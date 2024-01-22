/*
	File:		PlaySoundFillBuffer.c
	
	Description: Play Sound shows how to use the Sound Description Extention atom information with the
				 SoundConverterFillBuffer APIs to play VBR and Non-VBR MP3 files. It will also play
				 .aiff files using various encoding methods and .wav files.
				 NOTE: This Sample requires CarbonLib 1.1 or above.

	Author:		mc, era

	Copyright: 	© Copyright 2000 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first): <2> 10/31/00 modified to use SoundConverterFillBuffer APIs
										<1> 7/26/00 initial release as PlaySound.c
*/
#include "ced.h"
#include <FixMath.h>
#include "MMedia.h"
#include "my_vkf.h"
#include "mp3.h"
#if defined(_WIN32) || defined(__MACH__)
  #include "MP3ImporterPatch.h"
#endif
#if defined(_WIN32)
  #include "Clan2Doc.h"
#endif

static char					gBufferDone;
static UInt32				outputBytes = 0L,
							outputFrames,
							outputFlags,
							actualOutputBytes;
static UInt32				*pBufSize,
							pBuf0Size,
							pBuf1Size;
static SndChannelPtr		pSoundChannel;
static SndCommand			mySndCmd,
							theCallBackCmd;
static SndCommand			theVolumeCmd;
static SCFillBufferData 	scFillBufferData;// = {0};
static eBufferNumber   		whichBuffer;
static SndCommand	 		thePlayCmd0,
							thePlayCmd1,
							*pPlayCmd;
static Ptr					pDecomBuffer0,
							pDecomBuffer1;
static Ptr 					pDecomBuffer = NULL;			
static CmpSoundHeaderPtr 	pSndHeader = NULL;
static CmpSoundHeader		mySndHeader0,
							mySndHeader1;
static SoundConverter		mySoundConverter;
static SndCallBackUPP		theSoundCallBackUPP;
static AudioCompressionAtomPtr	theDecompressionAtom;
static SoundConverterFillBufferDataUPP theFillBufferDataUPP;

int  sndCom;
int  SndPlayLoopCnt;
char isMP3PatchLoaded = false;
char isWinSndPlayAborted = 0;
Boolean	isMP3SoundDone = true;
Boolean	isMP3Finishing = true;
/* 2009-10-13
static long convertCurMP3ToPos(long num) {
	return(conv_from_msec_rep(&global_df->SnTr, conv_to_msec_MP3(&global_df->SnTr, num)));
}
*/
static long convertCurPosToMP3(long num) {
	return(conv_from_msec_MP3(conv_to_msec_rep(num)));
}

// * ----------------------------
// MySoundCallBackFunction
//
// used to signal when a buffer is done playing
static pascal void MySoundCallBackFunction(SndChannelPtr theChannel, SndCommand *theCmd)
{
#pragma unused(theChannel)
	
	#ifndef TARGET_API_MAC_CARBON
		#if !GENERATINGCFM
			long oldA5;
			oldA5 = SetA5(theCmd->param2);
		#else
			#pragma unused(theCmd)
		#endif
	#else
		#pragma unused(theCmd)
	#endif // TARGET_API_MAC_CARBON

	gBufferDone = TRUE;

	#ifndef TARGET_API_MAC_CARBON
		#if !GENERATINGCFM
			oldA5 = SetA5(oldA5);
		#endif
	#endif // TARGET_API_MAC_CARBON
}

// * ----------------------------
// SoundConverterFillBufferDataProc
//
// the callback routine that provides the source data for conversion - it provides data by setting
// outData to a pointer to a properly filled out ExtendedSoundComponentData structure
static pascal Boolean SoundConverterFillBufferDataProc(SoundComponentDataPtr *outData, void *inRefCon)
{
	SCFillBufferDataPtr data = (SCFillBufferDataPtr)inRefCon;
	
	OSErr err = noErr;

	// if after getting the last chunk of data the total time is over the duration, we're done
	if (data->mediaTime >= data->srcEnd) {
		data->isThereMoreSource = false;
		data->compData.desc.buffer = NULL;
		data->compData.desc.sampleCount = 0;
		data->compData.bufferSize = 0;
	}
	
	if (data->isThereMoreSource) {
 // if use "GetMediaSample"
		long		sourceBytesReturned;
		long		numberOfSamples;
		TimeValue	newMediaTime;
		TimeValue	sourceReturnedTime, durationPerSample;
/* // if use "GetMediaSample2"
		ByteCount	sourceBytesReturned;
		ItemCount	numberOfSamples;
		TimeValue64	newMediaTime;
		TimeValue64	sourceReturnedTime, durationPerSample;
*/
		HUnlock(data->hSource);

//fprintf(data->fp, "2 mediaTime=%ld; srcEnd=%ld;\n", data->mediaTime, data->srcEnd);
		err = GetMediaSample(data->srcMedia,			// specifies the media for this operation
							 data->hSource,				// function returns the sample data into this handle
							 data->maxBufferSize,		// maximum number of bytes of sample data to be returned
							 &sourceBytesReturned,		// the number of bytes of sample data returned
							 data->mediaTime,			// starting time of the sample to be retrieved (must be in Media's TimeScale)
							 &sourceReturnedTime,		// indicates the actual time of the returned sample data
							 &durationPerSample,		// duration of each sample in the media
							 NULL,						// sample description corresponding to the returned sample data (NULL to ignore)
							 NULL,						// index value to the sample description that corresponds to the returned sample data (NULL to ignore)
							 data->maxSamples,			// maximum number of samples to be returned (0 to use a value that is appropriate for the media)
							 &numberOfSamples,			// number of samples it actually returned
							 NULL);						// flags that describe the sample (NULL to ignore)
/*
		err = GetMediaSample2(data->srcMedia,
                            (UInt8 *)*data->hSource,
                            data->maxBufferSize,
                            &sourceBytesReturned,
                            data->mediaTime,
                            &sourceReturnedTime,
                            &durationPerSample,
                            (TimeValue64 *)NULL,
                            (SampleDescriptionHandle)NULL,
                            (ItemCount *)NULL,
                            data->maxSamples,
                            &numberOfSamples,
                            (MediaSampleFlags *)NULL);
*/
		HLock(data->hSource);

		newMediaTime = sourceReturnedTime + (durationPerSample * numberOfSamples);
/*
fprintf(data->fp, "3 err=%d; mediaTime=%ld; newMediaTime=%ld; srcEnd=%ld;\n", err, data->mediaTime, newMediaTime, data->srcEnd);
fprintf(data->fp, "3.5 sourceReturnedTime=%ld; durationPerSample=%ld; numberOfSamples=%ld;\n", sourceReturnedTime, durationPerSample, numberOfSamples);
fprintf(data->fp, "3.6 sourceBytesReturned=%ld; data->compData.desc.sampleCount=%ld; data->maxBufferSize=%ld;\n",
		sourceBytesReturned, data->compData.desc.sampleCount, data->maxBufferSize);
*/
// The way to limit play on compressed stream  2008-03-12
/*
		if (data->maxSamples == 0L) {
			if ((newMediaTime-data->mediaTime) > (data->srcEnd - data->mediaTime)) {
				if (durationPerSample != 0L) {
					data->maxSamples = (data->srcEnd - data->mediaTime) / durationPerSample;
					if (data->maxSamples < 0)
						data->maxSamples = 0L;
				}
repeat_read:
				HUnlock(data->hSource);
				err = GetMediaSample(data->srcMedia,
									 data->hSource,
									 data->maxBufferSize,
									 &sourceBytesReturned,
									 data->mediaTime,
									 &sourceReturnedTime,
									 &durationPerSample,
									 NULL,
									 NULL,
									 data->maxSamples,
									 &numberOfSamples,
									 NULL);
									 
				HLock(data->hSource);
				newMediaTime = sourceReturnedTime + (durationPerSample * numberOfSamples);
				if (newMediaTime < data->srcEnd) {
					data->maxSamples++;
					goto repeat_read;
				}
			}
		}
*/
// The way to limit play on compressed stream
		if ((err != noErr) || (sourceBytesReturned == 0L)) {
			data->err = err;
			data->isThereMoreSource = false;
			data->compData.desc.buffer = NULL;
			data->compData.desc.sampleCount = 0;		
			data->mediaTime = 0L;
			data->maxSamples = 0L;
			data->compData.bufferSize = 0L;

			if ((err != noErr) && (sourceBytesReturned > 0L)) {
#if defined(_MAC_CODE)
				DebugStr("\pGetMediaSample - Failed in FillBufferDataProc");
#else
				DebugStr((unsigned char *)"GetMediaSample - Failed in FillBufferDataProc");
#endif
			}
		} else {
			data->mediaTime = newMediaTime;
// 2008-03-12	data->maxSamples = (data->srcEnd - data->mediaTime) / durationPerSample;
			data->maxSamples = 0L;
			if (data->maxSamples < 0)
				data->maxSamples = 0L;
			data->compData.bufferSize = sourceBytesReturned;
		}
	}

	// set outData to a properly filled out ExtendedSoundComponentData struct
	*outData = (SoundComponentDataPtr)&data->compData;
//fprintf(data->fp, "7 isThereMoreSource=%d; data->compData.bufferSize=%ld; \n", data->isThereMoreSource, data->compData.bufferSize);
	return (data->isThereMoreSource);
}


// * ----------------------------
// GetMovieMedia
//
// returns a Media identifier - if the file is a System 7 Sound a non-in-place import is done and
// a handle to the data is passed back to the caller who is responsible for disposing of it
OSErr GetMovieMedia(FNType *inFile, Media *outMedia, Handle *outHandle)
{
	unCH		wrSoundFile[FNSize];
	long 		ignoreFlags;
	FSSpec		fss;
	Movie		tMovie = 0;
	Track		theTrack;
	FInfo		fndrInfo;
	OSType		DRType;
	Handle		DR = NULL;
	short		RID = 0;
	CFStringRef	theStr = NULL;
	Handle 		hDataRef = NULL;
	Track		ignoreTrack;
	TimeValue 	ignoreDuration;		
	OSErr		err = noErr;
	MovieImportComponent theImporter = 0;

// mp3 mp3fix mp3patch mp3 fix mp3 patch old mp3 mp3 old fix old mp3 fix bad mp3 bad mp3

#if defined(_WIN32) || defined(__MACH__)
	if (!isMP3PatchLoaded) {
//		MP3ImporterPatch_DoRegister();
		isMP3PatchLoaded = true;
	}
#endif

#if defined(_WIN32)
	Str255 filename;

	c2pstrcpy (filename, inFile);
	err = FSMakeFSSpec(0, 0L, (const unsigned char*)filename, &fss);
#else // _WIN32
	FSRef  ref;

	err = FSPathMakeRef((UInt8 *)inFile, &ref, NULL);
	SkipErr(err);
	err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, &fss, NULL);
#endif // _MAC_CODE
	SkipErr(err);
	err = FSpGetFInfo(&fss, &fndrInfo);
	SkipErr(err);
	if (kQTFileTypeSystemSevenSound == fndrInfo.fdType) {
		// if this is an 'sfil' handle it appropriately
		// QuickTime can't import these files in place, but that's ok,
		// we just need a new place to put the data
		// create a new movie
		tMovie = NewMovie(newMovieActive);
		// allocate the data handle and create a data reference for this handle
		// the caller is responsible for disposing of the data handle once done with the sound
		*outHandle = NewHandle(0);
		err = PtrToHand(outHandle, &hDataRef, sizeof(Handle));
		if (noErr == err) {
			SetMovieDefaultDataRef(tMovie, hDataRef, HandleDataHandlerSubType);
			OpenADefaultComponent(MovieImportType, kQTFileTypeSystemSevenSound, &theImporter);
			if (theImporter) {
				err = MovieImportFile(theImporter, &fss, tMovie, 0, &ignoreTrack, 0, &ignoreDuration, 0, &ignoreFlags);
				CloseComponent(theImporter);
			}
		} else {
			if (*outHandle) {
				DisposeHandle(*outHandle);
				*outHandle = NULL;
			}
		}
		if (hDataRef) DisposeHandle(hDataRef);
		BailErr(err);
	} else {
skip_err:
		u_strcpy(wrSoundFile, inFile, FNSize);
		theStr = CFStringCreateWithCharacters(kCFAllocatorDefault, (UniChar *)wrSoundFile, strlen(wrSoundFile));
		err = QTNewDataReferenceFromFullPathCFString(theStr, kQTNativeDefaultPathStyle, 0, &DR, &DRType);
		CFRelease(theStr);
		BailErr(err);
		err = NewMovieFromDataRef(&tMovie, newMovieActive, &RID, DR, DRType);
		DisposeHandle(DR);
		BailErr(err);
	}
	// get the first sound track
	theTrack = GetMovieIndTrackType(tMovie, 1, SoundMediaType, movieTrackMediaType);
	if (NULL == theTrack ) BailErr(invalidTrack);
	// get and return the sound track media
	*outMedia = GetTrackMedia(theTrack);
	if (NULL == *outMedia) err = invalidMedia;

bail:
	return err;
}

// * ----------------------------
// MyGetSoundDescriptionExtension
//
// this function will extract the information needed to decompress the sound file, this includes 
// retrieving the sample description, the decompression atom and setting up the sound header
OSErr MyGetSoundDescriptionExtension(Media inMedia, AudioFormatAtomPtr *outAudioAtom, CmpSoundHeaderPtr outSoundHeader)
{
	OSErr err = noErr;
			
	Size size;
	Handle extension;
		
	// Version 1 of this record includes four extra fields to store information about compression ratios. It also defines
	// how other extensions are added to the SoundDescription record.
	// All other additions to the SoundDescription record are made using QT atoms. That means one or more
	// atoms can be appended to the end of the SoundDescription record using the standard [size, type]
	// mechanism used throughout the QuickTime movie resource architecture.
	// http://developer.apple.com/techpubs/quicktime/qtdevdocs/RM/frameset.htm
	SoundDescriptionV1Handle sourceSoundDescription = (SoundDescriptionV1Handle)NewHandle(0);
	
	// get the description of the sample data
	GetMediaSampleDescription(inMedia, 1, (SampleDescriptionHandle)sourceSoundDescription);
	if ((err=GetMoviesError()) != noErr) {
		if (sourceSoundDescription)
			DisposeHandle((Handle)sourceSoundDescription);
		return err;
	}

	if (outAudioAtom != NULL) {
		extension = NewHandle(0);

		// get the "magic" decompression atom
		// This extension to the SoundDescription information stores data specific to a given audio decompressor.
		// Some audio decompression algorithms require a set of out-of-stream values to configure the decompressor
		// which are stored in a siDecompressionParams atom. The contents of the siDecompressionParams atom are dependent
		// on the audio decompressor.
		err = GetSoundDescriptionExtension((SoundDescriptionHandle)sourceSoundDescription, &extension, siDecompressionParams);

		if (noErr == err) {
			size = GetHandleSize(extension);
			HLock(extension);
			*outAudioAtom = (AudioFormatAtom*)NewPtr(size);
			err = MemError();
			// copy the atom data to our buffer...
			BlockMoveData(*extension, *outAudioAtom, size);
			HUnlock(extension);
		} else {
			// if it doesn't have an atom, that's ok
			err = noErr;
		}
		DisposeHandle(extension);
	}
	
	// set up our sound header
	outSoundHeader->format = (*sourceSoundDescription)->desc.dataFormat;
	outSoundHeader->numChannels = (*sourceSoundDescription)->desc.numChannels;
	outSoundHeader->sampleSize = (*sourceSoundDescription)->desc.sampleSize;
	outSoundHeader->sampleRate = (*sourceSoundDescription)->desc.sampleRate;
	
	DisposeHandle((Handle)sourceSoundDescription);
	
	return err;
}
//cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
//return (char)(err == noErr);

static void cleanUpMp3(SoundConverterFillBufferDataUPP v1, 
					   SndChannelPtr v2, 
					   SndCallBackUPP v3, 
					   SCFillBufferData *v4,
					   SoundConverter v5,
					   Ptr v6,
					   Ptr v7,
					   AudioCompressionAtomPtr v8) {
	if (v1)
		DisposeSoundConverterFillBufferDataUPP(v1);
	if (v2)
		SndDisposeChannel(v2, false);		// wait until sounds stops playing before disposing of channel
	if (v3)
		DisposeSndCallBackUPP(v3);
	if (v4->hSource)
		DisposeHandle(v4->hSource);
	if (v5)
		SoundConverterClose(v5);
	if (v6)
		DisposePtr(v6);
	if (v7)
		DisposePtr(v7);
	if (v8 != NULL)
		DisposePtr((Ptr)v8);
}

// * ----------------------------
// PlayMP3Sound
//
// this function does the actual work of playing the sound file, it sets up the sound converter environment, allocates play buffers,
// creates the sound channel and sends the appropriate sound commands to play the converted sound data
char PlayMP3Sound(int com) {
	SoundComponentData		theInputFormat,
							theOutputFormat;
	OSErr 					err;
	extern char 			leftChan;
	extern char 			rightChan;

#if defined(_MAC_CODE)
repeat_playback:
#endif
	sndCom = com;
	outputBytes = 0;
	pSndHeader = NULL;
	pDecomBuffer = NULL;			
	whichBuffer = kFirstBuffer;
	pSoundChannel = NULL;
	theSoundCallBackUPP = NULL;
	theFillBufferDataUPP = NULL;
	scFillBufferData.hSource = NULL;
	theDecompressionAtom = NULL;
	mySoundConverter = NULL;
	pDecomBuffer0 = NULL;
	pDecomBuffer1 = NULL;
	isMP3SoundDone = false;
	isMP3Finishing = false;
	err = noErr;
	checkContinousSndPlayPossition(global_df->SnTr.BegF);
	if (com == (int)'p')
		global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
#if defined(_MAC_CODE)
	strcpy(global_df->err_message, DASHES);
#endif
	if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize) {
		strcpy(global_df->err_message, "+Starting point is beyond end of file.");
		return(2);
	}
	if (!leftChan && !rightChan) {
		strcpy(global_df->err_message, "+No channel was selected. Please goto \"sonic mode\" and click on 'L' and/or 'R'.");
		return(2);
	}
	err = MyGetSoundDescriptionExtension(global_df->SnTr.mp3.theSoundMedia, (AudioFormatAtomPtr *)&theDecompressionAtom, &mySndHeader0);
	if (err != noErr) {
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
		return (char)(err == noErr);
	}

	// setup input/output format for sound converter
	theInputFormat.flags = 0;
	theInputFormat.format = mySndHeader0.format;
	theInputFormat.numChannels = mySndHeader0.numChannels;
	theInputFormat.sampleSize = mySndHeader0.sampleSize;
	theInputFormat.sampleRate = mySndHeader0.sampleRate;
	theInputFormat.sampleCount = 0;
	theInputFormat.buffer = NULL;
	theInputFormat.reserved = 0;

	theOutputFormat.flags = 0;
	theOutputFormat.format = kSoundNotCompressed;
	theOutputFormat.numChannels = theInputFormat.numChannels;
	theOutputFormat.sampleSize = theInputFormat.sampleSize;
	theOutputFormat.sampleRate = theInputFormat.sampleRate;
	theOutputFormat.sampleCount = 0;
	theOutputFormat.buffer = NULL;
	theOutputFormat.reserved = 0;

	err = SoundConverterOpen(&theInputFormat, &theOutputFormat, &mySoundConverter);
	if (err != noErr) {
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
		return (char)(err == noErr);
	}

	// set up the sound converters decompresson 'environment' by passing in the 'magic' decompression atom
	err = SoundConverterSetInfo(mySoundConverter, siDecompressionParams, theDecompressionAtom);
	if (siUnknownInfoType == err) {
		// clear this error, the decompressor didn't
		// need the decompression atom and that's OK
		err = noErr;
	} else {
		if (err != noErr) {
			cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
			return (char)(err == noErr);
		}
	}

	UInt32	targetBytes = 65536L; // 32768L;
	// find out how much buffer space to alocate for our output buffers
	// The differences between SoundConverterConvertBuffer begin with how the buffering is done. SoundConverterFillBuffer will do as much or as
	// little work as is required to satisfy a given request. This means that you can pass in buffers of any size you like and expect that
	// the Sound Converter will never overflow the output buffer. SoundConverterFillBufferDataProc function will be called as many times as
	// necessary to fulfill a request. This means that the SoundConverterFillBufferDataProc routine is free to provide data in whatever chunk size
	// it likes. Of course with both sides, the buffer sizes will control how many times you need to request data and there is a certain amount of
	// overhead for each call. You will want to balance this against the performance you require. While a call to SoundConverterGetBufferSizes is
	// not required by the SoundConverterFillBuffer function, it is useful as a guide for non-VBR formats
	do {
		UInt32 inputFrames, inputBytes;
		targetBytes *= 2;
		err = SoundConverterGetBufferSizes(mySoundConverter, targetBytes, &inputFrames, &inputBytes, &outputBytes);
	} while (notEnoughBufferSpace == err  && targetBytes < (MaxBlock() / 4));
	// allocate play buffers
	pDecomBuffer0 = NewPtr(outputBytes);
	if ((err=MemError()) != noErr) {
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
		return (char)(err == noErr);
	}
	
	pDecomBuffer1 = NewPtr(outputBytes);
	if ((err=MemError()) != noErr) {
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
		return (char)(err == noErr);
	}

	err = SoundConverterBeginConversion(mySoundConverter);
	if (err != noErr) {
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
		return (char)(err == noErr);
	}

	// setup first header
	mySndHeader0.samplePtr = pDecomBuffer0;
	mySndHeader0.numChannels = theOutputFormat.numChannels;
	mySndHeader0.sampleRate = theOutputFormat.sampleRate;
	mySndHeader0.loopStart = 0;
	mySndHeader0.loopEnd = 0;
	mySndHeader0.encode = cmpSH;					// compressed sound header encode value
	mySndHeader0.baseFrequency = kMiddleC;
	// mySndHeader0.AIFFSampleRate;					// this is not used
	mySndHeader0.markerChunk = NULL;
	mySndHeader0.format = theOutputFormat.format;
	mySndHeader0.futureUse2 = 0;
	mySndHeader0.stateVars = NULL;
	mySndHeader0.leftOverSamples = NULL;
	mySndHeader0.compressionID = fixedCompression; //notCompressed;	// compression ID for fixed-sized compression, even uncompressed sounds use fixedCompression
	mySndHeader0.packetSize = 0;					// the Sound Manager will figure this out for us
	mySndHeader0.snthID = 0;
	mySndHeader0.sampleSize = theOutputFormat.sampleSize;
	mySndHeader0.sampleArea[0] = 0;					// no samples here because we use samplePtr to point to our buffer instead

	if (PBC.speed != 100) {
		double t1;
	 	t1 = (double)mySndHeader0.sampleRate;
	 	t1 *= (double)PBC.speed;
	 	t1 = t1 / 100.0000;
	 	mySndHeader0.sampleRate = (unsigned long)t1;
	}

	// setup second header, only the buffer ptr is different
	BlockMoveData(&mySndHeader0, &mySndHeader1, sizeof(mySndHeader0));
	mySndHeader1.samplePtr = pDecomBuffer1;
	
	// fill in struct that gets passed to SoundConverterFillBufferDataProc via the refcon
	// this includes the ExtendedSoundComponentData record	
	scFillBufferData.maxSamples = 0L;
	scFillBufferData.srcMedia = global_df->SnTr.mp3.theSoundMedia;
	scFillBufferData.mediaTime = convertCurPosToMP3(global_df->SnTr.BegF);
	scFillBufferData.sentToPlayer = conv_to_msec_rep(global_df->SnTr.BegF);
	scFillBufferData.srcEnd = convertCurPosToMP3(global_df->SnTr.SoundFileSize);
	scFillBufferData.curBegTime = global_df->SnTr.BegF;
	scFillBufferData.isThereMoreSource = true;
	scFillBufferData.maxBufferSize = kMaxInputBuffer;
	scFillBufferData.hSource = NewHandle((long)scFillBufferData.maxBufferSize);		// allocate source media buffer
	if ((err=MemError()) != noErr) {
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
		return (char)(err == noErr);
	}
	scFillBufferData.compData.desc = theInputFormat;
	scFillBufferData.compData.desc.buffer = (Byte *)*scFillBufferData.hSource;
	scFillBufferData.compData.desc.flags = kExtendedSoundData;
	scFillBufferData.compData.recordSize = sizeof(ExtendedSoundComponentData);
	scFillBufferData.compData.extendedFlags = kExtendedSoundSampleCountNotValid | kExtendedSoundBufferSizeValid;
	scFillBufferData.compData.bufferSize = 0;

	// setup the callback, create the sound channel and play the sound
	// we will continue to convert the sound data into the free (non playing) buffer
	theSoundCallBackUPP = NewSndCallBackUPP(MySoundCallBackFunction);
		 	
	unsigned short theLeftVol, theRightVol;
	theLeftVol = 256;
	theRightVol = 256;
	theVolumeCmd.cmd = volumeCmd;
	theVolumeCmd.param1 = 0;
	theVolumeCmd.param2 = (long)((theRightVol << 16) | theLeftVol);

	if (global_df->SnTr.SNDchan == 1)
		err = SndNewChannel(&pSoundChannel, sampledSynth, initMono, theSoundCallBackUPP);
	else if (!leftChan && rightChan)
		err = SndNewChannel(&pSoundChannel, sampledSynth, initChanRight, theSoundCallBackUPP);
	else if (leftChan && !rightChan)
		err = SndNewChannel(&pSoundChannel, sampledSynth, initChanLeft, theSoundCallBackUPP);
	else
		err = SndNewChannel(&pSoundChannel, sampledSynth, initStereo, theSoundCallBackUPP);
	if (err != noErr)
		strcpy(global_df->err_message, "+Can't allocate sound channel.");

	SndDoCommand(pSoundChannel, &theVolumeCmd, true);

	if (err != noErr) {
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
		return (char)(err == noErr);
	}

	theFillBufferDataUPP = NewSoundConverterFillBufferDataUPP(SoundConverterFillBufferDataProc);
	
	thePlayCmd0.cmd = bufferCmd;
	thePlayCmd0.param1 = 0;						// not used, but clear it out anyway just to be safe
	thePlayCmd0.param2 = (long)&mySndHeader0;

	thePlayCmd1.cmd = bufferCmd;
	thePlayCmd1.param1 = 0;						// not used, but clear it out anyway just to be safe
	thePlayCmd1.param2 = (long)&mySndHeader1;
				
	whichBuffer = kFirstBuffer;					// buffer 1 will be free when callback runs
	theCallBackCmd.cmd = callBackCmd;
#if defined(_MAC_CODE)
	theCallBackCmd.param2 = SetCurrentA5();
#endif
	gBufferDone = TRUE;
	if (PlayingContSound)
		isWinSndPlayAborted = 2;
	else
		isWinSndPlayAborted = 0;
	if (err != noErr) {
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
		return (char)(err == noErr);
	}

	scFillBufferData.lastBufSize = 0L;
	pBuf0Size = 0L;
	pBuf1Size = 0L;
#if defined(_MAC_CODE)
	int			key;
	char		res;
	long 		CurFP;
	long		msecs;
	SCStatus	status;
	double		timeOffset;

	scFillBufferData.lTime_mark = TickCount();
	PlayingSound = TRUE;
	do {
		timeOffset = (double)(TickCount() - scFillBufferData.lTime_mark);
		timeOffset *= 16.666666666666667;
		CurFP = conv_from_msec_rep((long)timeOffset) + scFillBufferData.curBegTime;
		if (CurFP > global_df->SnTr.EndF) {
			mySndCmd.cmd = quietCmd;
			err = SndDoImmediate(pSoundChannel, &mySndCmd);
			outputFrames = 0L;
			isMP3SoundDone = true;
			strcpy(global_df->err_message, DASHES);
			break;
		}
		checkContinousSndPlayPossition(CurFP);
		if (!PlayingContSound && PBC.enable && PBC.isPC) {
			strcpy(global_df->err_message, "-Playing, press any F5-F6 key to stop");
			draw_mid_wm();
			PrepWindow(0);
			NonModal = 0;
		}
		if ((key=ced_getc()) != -1) {
			if (!PlayingSound || global_df == NULL) {
				mySndCmd.cmd = quietCmd;
				err = SndDoImmediate(pSoundChannel, &mySndCmd);
				isWinSndPlayAborted = 1;
				break;
			}

			if (PlayingContSound == '\002') {
				mySndCmd.cmd = pauseCmd;
				err = SndDoImmediate(pSoundChannel, &mySndCmd);
				if (err)
					break;
				global_df->WinChange = FALSE;
				strcpy(global_df->err_message, "-Pausing ...");
				draw_mid_wm();
				while (StillDown() == TRUE) ;
				strcpy(global_df->err_message, "-Click mouse twice to stop, once to pause");
				draw_mid_wm();
				global_df->WinChange = TRUE;
				PlayingContSound = TRUE;
				mySndCmd.cmd = resumeCmd;
				err = SndDoImmediate(pSoundChannel, &mySndCmd);
				if (err)
					break;
			} else if (PlayingContSound != '\004') {
				if (!PlayingContSound) {
					if (isKeyEqualCommand(key, 91)) {
						isPlayS = 91;
					}
					if (((key > 0 && key < 256) || isPlayS != 0) && PBC.enable) {
						if (key != 1 || isPlayS != 0) {
							if (proccessKeys((unsigned int)key) == 1) {
								mySndCmd.cmd = quietCmd;
								err = SndDoImmediate(pSoundChannel, &mySndCmd);
								isWinSndPlayAborted = 1;
								strcpy(global_df->err_message, DASHES);
								break;
							}
						}
					} else {
						mySndCmd.cmd = quietCmd;
						err = SndDoImmediate(pSoundChannel, &mySndCmd);
						if (key == VK_F7) {
							isWinSndPlayAborted = 4; // F7
						} else if (key == VK_F9) {
							isWinSndPlayAborted = 5; // F9
						} else
							isWinSndPlayAborted = 1;
						strcpy(global_df->err_message, DASHES);
						break;
					}
				} else {
					PlayingContSound = '\003';
					mySndCmd.cmd = quietCmd;
					err = SndDoImmediate(pSoundChannel, &mySndCmd);
					isMP3SoundDone = true;
					isWinSndPlayAborted = 2;
					break;
				}
			}
		} else {
			if (!PlayingSound || global_df == NULL) {
				isWinSndPlayAborted = 1;
				break;
			}
		}
		if (PlayingContSound == '\004') {
			global_df->LeaveHighliteOn = TRUE;
			if ((key == VK_F5 || key == VK_F1 || key == VK_F2) && F5_Offset != 0L) {
				DrawCursor(0);
				DrawSoundCursor(2);
				ResetSelectFlag(0);
				if (global_df->SnTr.BegF != CurFP && CurFP != 0L) {
					global_df->row_win2 = 0L;
					global_df->col_win2 = -2L;
					global_df->col_chr2 = -2L;
					if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
						findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
					SaveUndoState(FALSE);
					addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, conv_to_msec_rep(global_df->SnTr.BegF), conv_to_msec_rep(CurFP));
					global_df->SnTr.BegF = AlignMultibyteMediaStream(CurFP, '-');
					SetPBCglobal_df(false, 0L);
					if (global_df->SnTr.IsSoundOn)
						DisplayEndF(FALSE);
				}
				selectNextSpeaker();
				strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
				draw_mid_wm();
				strcpy(global_df->err_message, DASHES);
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
				isMP3SoundDone = true;
			} else if (key == 'i' || key == 'I' || key == ' ') {
				DrawCursor(0);
				DrawSoundCursor(2);
				ResetSelectFlag(0);								
				if (global_df->SnTr.BegF != CurFP && CurFP != 0L) {
					global_df->row_win2 = 0L;
					global_df->col_win2 = -2L;
					global_df->col_chr2 = -2L;
					if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
						findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
					SaveUndoState(FALSE);
					addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, conv_to_msec_rep(global_df->SnTr.BegF), conv_to_msec_rep(CurFP));
					global_df->SnTr.BegF = AlignMultibyteMediaStream(CurFP, '-');
					SetPBCglobal_df(false, 0L);
					if (global_df->SnTr.IsSoundOn)
						DisplayEndF(FALSE);
				}
				selectNextSpeaker();
				strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
				draw_mid_wm();
				strcpy(global_df->err_message, DASHES);
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
			}
		}
		if (gBufferDone == TRUE && !isMP3SoundDone) {
			scFillBufferData.curBegTime += scFillBufferData.lastBufSize;
			if (kFirstBuffer == whichBuffer) {
				pBufSize = &pBuf0Size;
				pPlayCmd = &thePlayCmd0;
				pDecomBuffer = pDecomBuffer0;
				pSndHeader = &mySndHeader0;
				whichBuffer = kSecondBuffer;
				scFillBufferData.lastBufSize = pBuf1Size;
			} else {
				pBufSize = &pBuf1Size;
				pPlayCmd = &thePlayCmd1;
				pDecomBuffer = pDecomBuffer1;
				pSndHeader = &mySndHeader1;
				whichBuffer = kFirstBuffer;
				scFillBufferData.lastBufSize = pBuf0Size;
			}
			if (!isMP3Finishing) {
				err = SoundConverterFillBuffer(mySoundConverter,		// a sound converter
											   theFillBufferDataUPP,	// the callback UPP
											   &scFillBufferData,		// refCon passed to FillDataProc
											   pDecomBuffer,			// the decompressed data 'play' buffer
											   outputBytes,				// size of the 'play' buffer
											   &actualOutputBytes,		// number of output bytes
											   &outputFrames,			// number of output frames
											   &outputFlags);			// fillbuffer retured advisor flags
				if (err)
					break;
				*pBufSize = actualOutputBytes;
			} else {
				SoundConverterEndConversion(mySoundConverter, pDecomBuffer, &outputFrames, &outputBytes);
			}
			msecs = conv_to_msec_rep(actualOutputBytes);
			if (scFillBufferData.sentToPlayer + msecs > global_df->SnTr.EndF) {
				msecs = global_df->SnTr.EndF - scFillBufferData.sentToPlayer;
				actualOutputBytes = conv_from_msec_rep(msecs);
				outputFrames = actualOutputBytes / global_df->SnTr.SNDsample / global_df->SnTr.SNDchan;
			}
			scFillBufferData.sentToPlayer += msecs;

			pSndHeader->numFrames = outputFrames;

			gBufferDone = FALSE;
			if (!isMP3SoundDone && !isMP3Finishing)
				SndDoCommand(pSoundChannel, &theCallBackCmd, true);	// reuse callBackCmd
			
			SndDoCommand(pSoundChannel, pPlayCmd, true);			// play the next buffer
			scFillBufferData.lTime_mark = TickCount();
			if ((outputFlags & kSoundConverterHasLeftOverData) == false && scFillBufferData.isThereMoreSource == false)
				isMP3Finishing = true;
		}
		err = SndChannelStatus(pSoundChannel, sizeof(status), &status);
	} while (!isMP3SoundDone || status.scChannelBusy) ;
//8-5-02 fclose(fp);
	if (com == 'p' && global_df) {
		isWinSndPlayAborted = 2;
		global_df->SnTr.EndF = CurFP;
		if (global_df->SoundWin) {
			DisplayEndF(FALSE);
			if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L)
				SaveUndoState(FALSE);
		}
	}
	
	if (!isMP3Finishing)
		SoundConverterEndConversion(mySoundConverter, pDecomBuffer, &outputFrames, &outputBytes);
/*
	if (!isWinSndPlayAborted) {
		msecs = conv_to_msec_rep(outputBytes);
		if (scFillBufferData.sentToPlayer + msecs > global_df->SnTr.EndF) {
			msecs = global_df->SnTr.EndF - scFillBufferData.sentToPlayer;
			outputBytes = conv_from_msec_rep(msecs);
			outputFrames = outputBytes / global_df->SnTr.SNDsample / global_df->SnTr.SNDchan;
		}
		if (noErr == err && outputFrames && !isWinSndPlayAborted) {
			pSndHeader->numFrames = outputFrames;
			SndDoCommand(pSoundChannel, pPlayCmd, true);	// play the last buffer.
		}
	}
*/
	cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
	if (!isWinSndPlayAborted && PBC.enable && PBC.isPC && err == noErr && PBC.LoopCnt > SndPlayLoopCnt && com != (int)'p') {
		SndPlayLoopCnt++;
		goto repeat_playback;
	}
	if (/*!isWinSndPlayAborted && */PBC.walk && PBC.backspace != PBC.step_length && global_df) {
		long t;
		extern long sEF;

		SndPlayLoopCnt = 1;
		DrawSoundCursor(0);
		if (isWinSndPlayAborted) {
			if (isWinSndPlayAborted == 4) {
				t = conv_to_msec_rep(CurFP) - PBC.step_length;
				if (t < 0L)
					t = 0L;
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
					goto fin;
				t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
			} else if (isWinSndPlayAborted == 5) {
				t = conv_to_msec_rep(CurFP) + PBC.step_length;
				if (t < 0L)
					t = 0L;
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
					goto fin;
				t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
			} else {
				t = conv_to_msec_rep(CurFP);
				if (t < 0L)
					goto fin;
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
					goto fin;
				t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
			}
		} else if (PBC.backspace > PBC.step_length) {
			t = conv_to_msec_rep(global_df->SnTr.BegF) - (PBC.backspace - PBC.step_length);
			if (t < 0L)
				goto fin;
			global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
			if (PBC.walk == 2 && global_df->SnTr.BegF >= sEF)
				goto fin;
			if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
				goto fin;
			t = conv_to_msec_rep(global_df->SnTr.EndF) - (PBC.backspace - PBC.step_length);
			global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
			if (PBC.walk == 2 && global_df->SnTr.EndF > sEF)
				global_df->SnTr.EndF = sEF;
		} else {
			t = conv_to_msec_rep(global_df->SnTr.BegF) + (PBC.step_length - PBC.backspace);
			if (t < 0L)
				goto fin;
			global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
			if (PBC.walk == 2 && global_df->SnTr.BegF >= sEF)
				goto fin;
			if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
				goto fin;
			t = conv_to_msec_rep(global_df->SnTr.EndF) + (PBC.step_length - PBC.backspace);
			global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
			if (PBC.walk == 2 && global_df->SnTr.EndF > sEF)
				global_df->SnTr.EndF = sEF;
		}
		SetPBCglobal_df(false, 0L);
		DisplayEndF(TRUE);
		DrawSoundCursor(3);
		if (!isWinSndPlayAborted && PBC.pause_len > 0L && !PlayingContSound) {
			double d;
			Size t;

			d = (double)PBC.pause_len;
			t = TickCount() + (Size)(d / (double)16.666666666);
			do {  
				if (TickCount() > t) 
					break;

				if ((key=ced_getc()) != -1) {
					if (isKeyEqualCommand(key, 91)) {
						isPlayS = 91;
					}
					if (((key > 0 && key < 256) || isPlayS != 0) && PBC.enable) {
						if (key != 1 || isPlayS != 0) {
							if (proccessKeys((unsigned int)key) == 1) {
								isWinSndPlayAborted = 1;
								strcpy(global_df->err_message, DASHES);
								break;
							}
						}
					} else {
						if (key == VK_F7) {
							t = conv_to_msec_rep(CurFP) - PBC.step_length;
							if (t < 0L)
								t = 0L;
							global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
							if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
								goto fin;
							t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
							global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
							isWinSndPlayAborted = 4; // F7
						} else if (key == VK_F9) {
							t = conv_to_msec_rep(CurFP) + PBC.step_length;
							if (t < 0L)
								t = 0L;
							global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
							if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
								goto fin;
							t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
							global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
							isWinSndPlayAborted = 5; // F9
						} else
							isWinSndPlayAborted = 1;
						strcpy(global_df->err_message, DASHES);
						break;
					}
				} else if (!PlayingSound || isPlayS != 0) {
					isWinSndPlayAborted = 1;
					break;
				}
			} while (true) ;	
		}
		if (isWinSndPlayAborted != 1)
			goto repeat_playback;
	}
fin:
	if (global_df) {
		if (isWinSndPlayAborted == 1 && PBC.walk)
			strcpy(global_df->err_message, "-Aborted.");
		else if (!PlayingContSound && PBC.enable && PBC.isPC)
			strcpy(global_df->err_message, DASHES);
		else if (global_df->LastCommand == 49) {
			extern long gCurF;

			gCurF = conv_to_msec_rep(CurFP);
			if (gCurF >= 0L) {
				gCurF = AlignMultibyteMediaStream(conv_from_msec_rep(gCurF), '+');
			} else
				gCurF = global_df->SnTr.BegF;
		}
	}
	if (global_df)
		SetPBCglobal_df(false, 0L);
	PlayingSound = FALSE;
#else  // _MAC_CODE
	double	tickCount, lastTickCount;

	scFillBufferData.lTime_mark = GetTickCount();
	if (PlayingContSound || !PBC.isPC) {
		char t = 0;
		MSG msg;
		tickCount = (double)(GetTickCount());
		lastTickCount = tickCount;
		while (!isMP3SoundDone) {
			if (tickCount - lastTickCount > 15) {
				if (PeekMessage(&msg, AfxGetApp()->m_pMainWnd->m_hWnd, 0, 0, PM_REMOVE)) {
					t = checkMP3Sound(&msg, t, tickCount);
				} else {
					t = checkMP3Sound(NULL, t, tickCount);
				}
				lastTickCount = tickCount;
			}
			tickCount = (double)(GetTickCount());
		}
		PBC.walk = 0;
		PBC.isPC = 0;
	}
#endif // _WIN32
	return (char)(err == noErr);
}

#if defined(_WIN32)
void stopMP3SoundIfPlaying(void);

void stopMP3SoundIfPlaying(void) {
	if (PlayingContSound)
		PlayingContSound = '\003';
	mySndCmd.cmd = quietCmd;
	SndDoImmediate(pSoundChannel, &mySndCmd);
	isMP3SoundDone = true;
	isWinSndPlayAborted = 1;
	SoundConverterEndConversion(mySoundConverter, pDecomBuffer, &outputFrames, &outputBytes);
	cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);
	PlayingSound = FALSE;
}

char checkMP3Sound(MSG* pMsg, char skipOnChar, double tickCount) {
	OSErr 			err;
	long			CurFP;
	long			msecs;
	double			timeOffset;
	extern DWORD	walker_pause;

	timeOffset = (double)(tickCount - scFillBufferData.lTime_mark);
	CurFP = conv_from_msec_rep((long)timeOffset) + scFillBufferData.curBegTime;
	if (CurFP > global_df->SnTr.EndF) {
		mySndCmd.cmd = quietCmd;
		err = SndDoImmediate(pSoundChannel, &mySndCmd);
		outputFrames = 0L;
		isMP3SoundDone = true;
		strcpy(global_df->err_message, DASHES);
		goto continueMP3;
	}
	checkContinousSndPlayPossition(CurFP);
	if (!PlayingContSound && PBC.enable && PBC.isPC) {
		strcpy(global_df->err_message, "-Playing, press any F5-F6 key to stop");
		draw_mid_wm();
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
	}
	if (pMsg != NULL) {
		if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_RBUTTONDOWN) {
			if (PlayingContSound || !PBC.enable) {
				mySndCmd.cmd = quietCmd;
				err = SndDoImmediate(pSoundChannel, &mySndCmd);
				isMP3SoundDone = true;
				isWinSndPlayAborted = 1;
				if (global_df && PlayingContSound == '\004') {
					DrawSoundCursor(0);
					DrawCursor(0);
					strcpy(global_df->err_message, DASHES);
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
				}
				if (PlayingContSound)
					PlayingContSound = '\003';
				goto continueMP3;
			}
		} else if (pMsg->message == WM_KEYDOWN) {
			if (PlayingContSound == '\004') {
				global_df->LeaveHighliteOn = TRUE;
				if (pMsg->wParam == 'i' || pMsg->wParam == 'I' || pMsg->wParam == ' ') {
					char res;

					DrawCursor(2);
					DrawSoundCursor(2);
					ResetSelectFlag(0);
					if (global_df->SnTr.BegF != CurFP && CurFP != 0L) {
						global_df->row_win2 = 0L;
						global_df->col_win2 = -2L;
						global_df->col_chr2 = -2L;
						if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
							findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
						SaveUndoState(FALSE);
						addBulletsToText(SOUNDTIER, global_df->SnTr.SoundFile, conv_to_msec_rep(global_df->SnTr.BegF), conv_to_msec_rep(CurFP));
						global_df->SnTr.BegF = AlignMultibyteMediaStream(CurFP, '-');
						SetPBCglobal_df(false, 0L);
						if (global_df->SnTr.IsSoundOn)
							DisplayEndF(FALSE);
						if (GlobalDoc && global_df->DataChanged)
							GlobalDoc->SetModifiedFlag(TRUE);
					}
					selectNextSpeaker();
					strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
					draw_mid_wm();
					strcpy(global_df->err_message, DASHES);
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
					DrawSoundCursor(1);
					DrawCursor(1);
				}
			} else if (pMsg->wParam >= 32) {
				if (PlayingContSound) {
					PlayingContSound = '\003';
					mySndCmd.cmd = quietCmd;
					err = SndDoImmediate(pSoundChannel, &mySndCmd);
					isMP3SoundDone = true;
					isWinSndPlayAborted = 1;
					skipOnChar = TRUE;
					goto continueMP3;
				}
				if (!PBC.enable || (pMsg->wParam >= VK_F1 && pMsg->wParam <= VK_F12)) {
					mySndCmd.cmd = quietCmd;
					err = SndDoImmediate(pSoundChannel, &mySndCmd);
					isMP3SoundDone = true;
					if (pMsg->wParam == VK_F7 && PBC.enable) {
						isWinSndPlayAborted = 4; // F7
					} else if (pMsg->wParam == VK_F9 && PBC.enable) {
						isWinSndPlayAborted = 5; // F9
					} else
						isWinSndPlayAborted = 1;
					skipOnChar = TRUE;
					goto continueMP3;
				}
			}
		}
	}

	if (gBufferDone == TRUE) {
		scFillBufferData.curBegTime += scFillBufferData.lastBufSize;
		if (kFirstBuffer == whichBuffer) {
			pBufSize = &pBuf0Size;
			pPlayCmd = &thePlayCmd0;
			pDecomBuffer = pDecomBuffer0;
			pSndHeader = &mySndHeader0;
			whichBuffer = kSecondBuffer;
			scFillBufferData.lastBufSize = pBuf1Size;
		} else {
			pBufSize = &pBuf1Size;
			pPlayCmd = &thePlayCmd1;
			pDecomBuffer = pDecomBuffer1;
			pSndHeader = &mySndHeader1;
			whichBuffer = kFirstBuffer;
			scFillBufferData.lastBufSize = pBuf0Size;
		}

		if (!isMP3Finishing) {
			err = SoundConverterFillBuffer(mySoundConverter,		// a sound converter
										   theFillBufferDataUPP,	// the callback UPP
										   &scFillBufferData,		// refCon passed to FillDataProc
										   pDecomBuffer,			// the decompressed data 'play' buffer
										   outputBytes,				// size of the 'play' buffer
										   &actualOutputBytes,		// number of output bytes
										   &outputFrames,			// number of output frames
										   &outputFlags);			// fillbuffer retured advisor flags
			if (err) {
				isMP3SoundDone = true;
				goto fin;
			}
			*pBufSize = actualOutputBytes;
		} else {
			SoundConverterEndConversion(mySoundConverter, pDecomBuffer, &outputFrames, &outputBytes);
		}

		msecs = conv_to_msec_rep(actualOutputBytes);
		if (scFillBufferData.sentToPlayer + msecs > global_df->SnTr.EndF) {
			msecs = global_df->SnTr.EndF - scFillBufferData.sentToPlayer;
			actualOutputBytes = conv_from_msec_rep(msecs);
			outputFrames = actualOutputBytes / global_df->SnTr.SNDsample / global_df->SnTr.SNDchan;
		}
		scFillBufferData.sentToPlayer += msecs;

		pSndHeader->numFrames = outputFrames;

		gBufferDone = FALSE;
		if (!isMP3SoundDone && !isMP3Finishing)
			SndDoCommand(pSoundChannel, &theCallBackCmd, true);	// reuse callBackCmd
		
		SndDoCommand(pSoundChannel, pPlayCmd, true);			// play the next buffer
		scFillBufferData.lTime_mark = GetTickCount();
		if ((outputFlags & kSoundConverterHasLeftOverData) == false && scFillBufferData.isThereMoreSource == false)
			isMP3Finishing = true;
	} else if (!isMP3SoundDone) {
		SCStatus status;

		err = SndChannelStatus(pSoundChannel, sizeof(status), &status);
		if (err) {
			isMP3SoundDone = true;
			isWinSndPlayAborted = 1;
			goto fin;
		}
		if (!status.scChannelBusy) {
			isMP3SoundDone = true;
			if (global_df) {
				strcpy(global_df->err_message, DASHES);
				draw_mid_wm();
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
			}
		}
	}
continueMP3:
	if (isMP3SoundDone) {
		if (sndCom == 'p' && global_df && !isWinSndPlayAborted) {
			isWinSndPlayAborted = 2;
			global_df->SnTr.EndF = CurFP;
			if (global_df->SoundWin) {
				DisplayEndF(FALSE);
				if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L)
					SaveUndoState(FALSE);
			}
		}
		
		if (!isMP3Finishing)
			SoundConverterEndConversion(mySoundConverter, pDecomBuffer, &outputFrames, &outputBytes);
/*
		if (!isWinSndPlayAborted) {
			msecs = conv_to_msec_rep(outputBytes);
			if (scFillBufferData.sentToPlayer + msecs > global_df->SnTr.EndF) {
				msecs = global_df->SnTr.EndF - scFillBufferData.sentToPlayer;
				outputBytes = conv_from_msec_rep(msecs);
				outputFrames = outputBytes / global_df->SnTr.SNDsample / global_df->SnTr.SNDchan;
			}
			if (noErr == err && outputFrames) {
				pSndHeader->numFrames = outputFrames;
				SndDoCommand(pSoundChannel, pPlayCmd, true);	// play the last buffer.
			}
		}
*/
		cleanUpMp3(theFillBufferDataUPP, pSoundChannel, theSoundCallBackUPP, &scFillBufferData, mySoundConverter, pDecomBuffer0, pDecomBuffer1, theDecompressionAtom);


		if (!isWinSndPlayAborted && PBC.isPC && err == noErr && PBC.LoopCnt > SndPlayLoopCnt && sndCom != (int)'p') {
			SndPlayLoopCnt++;
			PlayBuffer(sndCom);
			return(skipOnChar);
		}

		if (/* !isWinSndPlayAborted && */PBC.walk && PBC.backspace != PBC.step_length && global_df) {
			long t;
			extern long sEF;

			SndPlayLoopCnt = 1;
			DrawSoundCursor(0);
			if (isWinSndPlayAborted) {
				if (isWinSndPlayAborted == 4) {
					t = conv_to_msec_rep(CurFP) - PBC.step_length;
					if (t < 0L)
						t = 0L;
					global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
					if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
						goto fin;
					t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				} else if (isWinSndPlayAborted == 5) {
					t = conv_to_msec_rep(CurFP) + PBC.step_length;
					if (t < 0L)
						t = 0L;
					global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
					if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
						goto fin;
					t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				} else {
					t = conv_to_msec_rep(CurFP);
					if (t < 0L)
						goto fin;
					global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
					if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
						goto fin;
					t = conv_to_msec_rep(global_df->SnTr.BegF) + PBC.step_length;
					global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				}
			} else if (PBC.backspace > PBC.step_length) {
				t = conv_to_msec_rep(global_df->SnTr.BegF) - (PBC.backspace - PBC.step_length);
				if (t < 0L)
					goto fin;
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
				if (PBC.walk == 2 && global_df->SnTr.BegF >= sEF)
					goto fin;
				if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
					goto fin;
				t = conv_to_msec_rep(global_df->SnTr.EndF) - (PBC.backspace - PBC.step_length);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '-');
				if (PBC.walk == 2 && global_df->SnTr.EndF > sEF)
					global_df->SnTr.EndF = sEF;
			} else {
				t = conv_to_msec_rep(global_df->SnTr.BegF) + (PBC.step_length - PBC.backspace);
				if (t < 0L)
					goto fin;
				global_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				if (PBC.walk == 2 && global_df->SnTr.BegF >= sEF)
					goto fin;
				if (global_df->SnTr.BegF >= global_df->SnTr.SoundFileSize)
					goto fin;
				t = conv_to_msec_rep(global_df->SnTr.EndF) + (PBC.step_length - PBC.backspace);
				global_df->SnTr.EndF = AlignMultibyteMediaStream(conv_from_msec_rep(t), '+');
				if (PBC.walk == 2 && global_df->SnTr.EndF > sEF)
					global_df->SnTr.EndF = sEF;
			}
			SetPBCglobal_df(false, 0L);
			DisplayEndF(TRUE);
			DrawSoundCursor(3);
			if (!isWinSndPlayAborted && PBC.pause_len > 0L && !PlayingContSound) {
				walker_pause = GetTickCount() + PBC.pause_len;
			} else if (isWinSndPlayAborted != 1) {
				PlayBuffer(sndCom);
			}
			return(skipOnChar);
		}
fin:
		if (global_df) {
			if (isWinSndPlayAborted == 1 && PBC.walk)
				strcpy(global_df->err_message, "-Aborted.");
			else if (!PlayingContSound && PBC.enable && PBC.isPC)
				strcpy(global_df->err_message, DASHES);
			else if (global_df->LastCommand == 49) {
				extern long gCurF;

				gCurF = conv_to_msec_rep(CurFP);
				if (gCurF >= 0L) {
					gCurF = AlignMultibyteMediaStream(conv_from_msec_rep(gCurF), '+');
				} else
					gCurF = global_df->SnTr.BegF;
			}
		}
		if (global_df)
			SetPBCglobal_df(false, 0L);
		PBC.walk = 0;
		PBC.isPC = 0;
		PlayingSound = FALSE;
	}
	return(skipOnChar);
}
#endif // _WIN32

char ReadMP3Sound(int col, int cm, Size cur) {
	SoundComponentData		theInputFormat,
							theOutputFormat;
	SCFillBufferData		READ_scFillBufferData;// = {0};
	SoundConverter			READ_mySoundConverter;
	AudioCompressionAtomPtr	READ_theDecompressionAtom;
	SoundConverterFillBufferDataUPP READ_FillBufferDataUPP;
	CmpSoundHeader			mySndHeader;
	Ptr 					READ_DecomBuffer = NULL;
	UInt32					READ_outputBytes = 0L,
							READ_outputFrames,
							READ_outputFlags,
							READ_actualOutputBytes;
	OSErr 					err = noErr;
	Boolean					READ_isMP3SoundDone = true, isFTime = true, isRepeat = false;
	int   row, hp1, lp1, hp2, lp2, scale;
	short *tbuf = NULL;
	long  i, scale_cnt;

	if (cm == 0) {
		GetSoundWinDim(&row, &cm);
		global_df->SnTr.WEndF = global_df->SnTr.WBegF + ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
	} else {
		GetSoundWinDim(&row, &hp1);
		global_df->SnTr.WEndF = global_df->SnTr.WBegF + ((Size)hp1 * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
	}
	if (cur == 0)
		cur = global_df->SnTr.WBegF;
	if (global_df->mainVScale == 0 || row == 0)
		scale = 0;
	else
		scale = (int)((0xFFFFL / (Size)global_df->mainVScale) / (Size)row) + 1;
	scale_cnt = 0L;
	hp1 = 0; lp1 = hp1 - 1;
	hp2 = 0; lp2 = hp2 - 1;
	wdrawcontr(global_df->SoundWin, TRUE);

repeatSoundConverter:
	READ_theDecompressionAtom = NULL;
	READ_mySoundConverter = NULL;
	READ_DecomBuffer = NULL;
	READ_scFillBufferData.hSource = NULL;
	READ_isMP3SoundDone = false;
	err = MyGetSoundDescriptionExtension(global_df->SnTr.mp3.theSoundMedia, (AudioFormatAtomPtr *)&READ_theDecompressionAtom, &mySndHeader);
	if (noErr == err) {	
		// setup input/output format for sound converter
		theInputFormat.flags = 0;
		theInputFormat.format = mySndHeader.format;
		theInputFormat.numChannels = mySndHeader.numChannels;
		theInputFormat.sampleSize = mySndHeader.sampleSize;
		theInputFormat.sampleRate = mySndHeader.sampleRate;
		theInputFormat.sampleCount = 0;
		theInputFormat.buffer = NULL;
		theInputFormat.reserved = 0;

		theOutputFormat.flags = 0;
		theOutputFormat.format = kSoundNotCompressed;
		theOutputFormat.numChannels = theInputFormat.numChannels;
		theOutputFormat.sampleSize = theInputFormat.sampleSize;
		theOutputFormat.sampleRate = theInputFormat.sampleRate;
		theOutputFormat.sampleCount = 0;
		theOutputFormat.buffer = NULL;
		theOutputFormat.reserved = 0;

		err = SoundConverterOpen(&theInputFormat, &theOutputFormat, &READ_mySoundConverter);
		BailErr(err);

		// set up the sound converters decompresson 'environment' by passing in the 'magic' decompression atom
		err = SoundConverterSetInfo(READ_mySoundConverter, siDecompressionParams, READ_theDecompressionAtom);
		if (siUnknownInfoType == err) {
			// clear this error, the decompressor didn't
			// need the decompression atom and that's OK
			err = noErr;
		} else BailErr(err);

		UInt32	targetBytes = 65536L, // 32768L;
				count;
		READ_outputBytes = 0;
		// find out how much buffer space to alocate for our output buffers
		// The differences between SoundConverterConvertBuffer begin with how the buffering is done. SoundConverterFillBuffer will do as much or as
		// little work as is required to satisfy a given request. This means that you can pass in buffers of any size you like and expect that
		// the Sound Converter will never overflow the output buffer. SoundConverterFillBufferDataProc function will be called as many times as
		// necessary to fulfill a request. This means that the SoundConverterFillBufferDataProc routine is free to provide data in whatever chunk size
		// it likes. Of course with both sides, the buffer sizes will control how many times you need to request data and there is a certain amount of
		// overhead for each call. You will want to balance this against the performance you require. While a call to SoundConverterGetBufferSizes is
		// not required by the SoundConverterFillBuffer function, it is useful as a guide for non-VBR formats
		do {
			UInt32 inputFrames, inputBytes;
			targetBytes *= 2;
			err = SoundConverterGetBufferSizes(READ_mySoundConverter, targetBytes, &inputFrames, &inputBytes, &READ_outputBytes);
		} while (notEnoughBufferSpace == err  && targetBytes < (MaxBlock() / 4));

		// allocate play buffers
		READ_DecomBuffer = NewPtr(READ_outputBytes);
		BailErr(MemError());
		err = SoundConverterBeginConversion(READ_mySoundConverter);
		BailErr(err);
		// fill in struct that gets passed to SoundConverterFillBufferDataProc via the refcon
		// this includes the ExtendedSoundComponentData record		
		READ_scFillBufferData.maxSamples = 0L;
		READ_scFillBufferData.srcMedia = global_df->SnTr.mp3.theSoundMedia;
//		cur = 58977218;// g 58982008; b 58979218; 22050
		if (isFTime == true)
			READ_scFillBufferData.mediaTime = convertCurPosToMP3(cur);
		READ_scFillBufferData.srcEnd = convertCurPosToMP3(global_df->SnTr.SoundFileSize);
		READ_scFillBufferData.isThereMoreSource = true;
		READ_scFillBufferData.maxBufferSize = kMaxInputBuffer;
		READ_scFillBufferData.err = noErr;
		READ_scFillBufferData.hSource = NewHandle((long)READ_scFillBufferData.maxBufferSize);		// allocate source media buffer
		BailErr(MemError());
		READ_scFillBufferData.compData.desc = theInputFormat;
		READ_scFillBufferData.compData.desc.buffer = (Byte *)*READ_scFillBufferData.hSource;
		READ_scFillBufferData.compData.desc.sampleCount = 0;
		READ_scFillBufferData.compData.desc.flags = kExtendedSoundData;
		READ_scFillBufferData.compData.recordSize = sizeof(ExtendedSoundComponentData);
		READ_scFillBufferData.compData.extendedFlags = kExtendedSoundSampleCountNotValid | kExtendedSoundBufferSizeValid;
		READ_scFillBufferData.compData.bufferSize = 0;
		if (err == noErr) {
			READ_FillBufferDataUPP = NewSoundConverterFillBufferDataUPP(SoundConverterFillBufferDataProc);
			if (noErr == err) {
/*
fprintf(fp, "BEFORE MAIN WHILE LOOP: cm=%d;\n", cm);
fprintf(fp, "BegF=%ld; EndF=%ld; WBegF=%ld; WEndF=%ld;\n",
global_df->SnTr.BegF, global_df->SnTr.EndF, global_df->SnTr.WBegF, global_df->SnTr.WEndF);
fprintf(fp, "cur=%ld; SoundFileSize=%ld; SNDsample=%d\n\n",
		cur, global_df->SnTr.SoundFileSize, global_df->SnTr.SNDsample);
fprintf(fp, "kMaxInputBuffer=%ld; mediaTime=%ld; srcEnd=%ld;\n", kMaxInputBuffer, READ_scFillBufferData.mediaTime, READ_scFillBufferData.srcEnd);
*/
				while (!READ_isMP3SoundDone && col < cm) {
					err = SoundConverterFillBuffer(READ_mySoundConverter,		// a sound converter
												   READ_FillBufferDataUPP,		// the callback UPP
												   &READ_scFillBufferData,		// refCon passed to FillDataProc
												   READ_DecomBuffer,			// the decompressed data 'play' buffer
												   READ_outputBytes,			// size of the 'play' buffer
												   &READ_actualOutputBytes,		// number of output bytes
												   &READ_outputFrames,			// number of output frames
												   &READ_outputFlags);			// fillbuffer retured advisor flags
					if (err) {
						break;
					}
					if (READ_scFillBufferData.err == invalidTime && isFTime == true) {
						cur -= 1000L;
						if (cur < 0L)
							cur = 0L;
						READ_scFillBufferData.mediaTime = convertCurPosToMP3(AlignMultibyteMediaStream(cur, '-'));
//fprintf(fp, "RESET: mediaTime=%ld;\n", READ_scFillBufferData.mediaTime);
						READ_scFillBufferData.isThereMoreSource = true;
						isFTime = false;
						isRepeat = true;
						break;
					} else
						isFTime = false;

					count = READ_actualOutputBytes;
					if (global_df->SnTr.SNDsample != 1) {
						tbuf = (short *)READ_DecomBuffer;
						count /= 2;
					}
//fprintf(fp, "READ_actualOutputBytes=%ld; count=%ld; cm=%d;\n", READ_actualOutputBytes, count, cm);
					if((READ_outputFlags & kSoundConverterHasLeftOverData) == false && READ_scFillBufferData.isThereMoreSource == false) {
						READ_isMP3SoundDone = true;
//fprintf(fp, "DONE READING: col=%d; cm=%d;\n", col, cm);
						break;
					}
					for (i=0; i < count && col < cm; i++) {
						if (scale == 0)
							row = 0;
						else if (global_df->SnTr.SNDsample == 1)
							row = (int)(READ_DecomBuffer[i]) / scale;
						else
							row = (int)(tbuf[i]) / scale;
						if (global_df->SnTr.SNDchan == 1 || doMixedSTWave) {
							if (row > hp1)
								hp1 = row;
							if (row < lp1)
								lp1 = row;
						} else {
							if ((i % 2) == 0) {
								if (row > hp1)
									hp1 = row;
								if (row < lp1)
									lp1 = row;
							} else {
								if (row > hp2)
									hp2 = row;
								if (row < lp2)
									lp2 = row;
							}
						}
						scale_cnt++;
//fprintf(fp, "i=%ld; scale_cnt=%ld; global_df->scale_row=%ld; \n", i, scale_cnt, global_df->scale_row);
						if (scale_cnt == global_df->scale_row) {
//fprintf(fp, "hp1=%d; lp1=%d; hp2=%d; lp2=%d; col=%d\n", hp1, lp1, hp2, lp2, col);
							if (global_df->TopP1 && global_df->BotP1) {
								global_df->TopP1[col] = hp1;
								global_df->BotP1[col] = lp1;
							}
							if (global_df->TopP2 && global_df->BotP2) {
								global_df->TopP2[col] = hp2;
								global_df->BotP2[col] = lp2;
							}
							wdrawdot(global_df->SoundWin, hp1, lp1, hp2, lp2, col);
							col++;
							scale_cnt = 0L;
			 				hp1 = 0; lp1 = hp1 - 1;
			 				hp2 = 0; lp2 = hp2 - 1;
						}
					}
				}
			}
			SoundConverterEndConversion(READ_mySoundConverter, READ_DecomBuffer, &READ_outputFrames, &READ_outputBytes);
			if (READ_FillBufferDataUPP)
				DisposeSoundConverterFillBufferDataUPP(READ_FillBufferDataUPP);
		}
	}

bail:
	global_df->SnTr.WBegFM = global_df->SnTr.WBegF;
	global_df->SnTr.WEndFM = global_df->SnTr.WEndF;
	if (READ_scFillBufferData.hSource)
		DisposeHandle(READ_scFillBufferData.hSource);
	if (READ_mySoundConverter)
		SoundConverterClose(READ_mySoundConverter);
	if (READ_DecomBuffer)
		DisposePtr(READ_DecomBuffer);
	if (READ_theDecompressionAtom != NULL)
		DisposePtr((Ptr)READ_theDecompressionAtom);
	if (global_df->TopP1 && global_df->BotP1 && cm != 1)
		global_df->TopP1[col] = -1;
	if (isRepeat == true) {
		isRepeat = false;
		goto repeatSoundConverter;
	}
	return (char)(err == noErr);
}
