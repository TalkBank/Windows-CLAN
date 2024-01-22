/*
	File:		MP3Player.h

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
*/

#ifndef __MP3PLAYER__
	#define __MP3PLAYER__

#if __APPLE_CC__
	// build for carbon Mac OS X
	#define TARGET_API_MAC_CARBON 1
//	#define TARGET_API_MAC_OSX 1

	#include <Carbon/Carbon.h>
	#include <QuickTime/QuickTime.h>
#else
	// build for carbon Mac OS 9

	#include <QuickTimeComponents.h>
#endif // __APPLE_CC__
#include <time.h>

#define BailErr(x) {err = x; if(err != noErr) goto bail;}
#define SkipErr(x) {err = x; if(err != noErr) goto skip_err;}

// makes our buffers a little bigger so that if the decompressors overflow
// we don't corrupt memory which would suck
const UInt32 kMaxInputBuffer = 64 * 1024; // max size of input buffer

enum eBufferNumber { kFirstBuffer, kSecondBuffer };

typedef struct {
	long	atomSize;			// how big this structure is (big endian)
	long	atomType;			// atom type
	char	waveData[28];
} AtomQDMCWaveFormatEx;

typedef struct {
	AudioFormatAtom			formatData;
	AtomQDMCWaveFormatEx	endianData;
	AudioTerminatorAtom		terminatorData;
} AudioCompressionAtom, *AudioCompressionAtomPtr, **AudioCompressionAtomHandle;

typedef struct {
	ExtendedSoundComponentData	compData;
	Handle						hSource;		// source media buffer
	Media						srcMedia;		// sound media identifier
 // if use "GetMediaSample"
	long						maxSamples;
	TimeValue					mediaTime;
	TimeValue					srcEnd;
	TimeValue					curBegTime;
/* // if use "GetMediaSample2"
	ItemCount					maxSamples;
	TimeValue64					mediaTime;
	TimeValue64					srcEnd;
	TimeValue64					curBegTime;
*/
	unsigned long				sentToPlayer;
	unsigned long				lastBufSize;
	unsigned long				lTime_mark;
	Boolean						isThereMoreSource;
	UInt32						maxBufferSize;
} SCFillBufferData, *SCFillBufferDataPtr;

extern char isMP3PatchLoaded;

// functions
extern "C"
{
extern char PlayMP3Sound(int com);
extern char ReadMP3Sound(int col, int cm, Size cur);
extern OSErr MyGetSoundDescriptionExtension(Media inMedia, AudioFormatAtomPtr *outAudioAtom, CmpSoundHeaderPtr outSoundHeader);
extern OSErr GetMovieMedia(FNType *inFile, Media *outMedia, Handle *outHandle);

#if defined(_WIN32)
  extern char checkMP3Sound(MSG* pMsg, char skipOnChar, double tickCount);
#endif
}
#endif // __MP3PLAYER__


