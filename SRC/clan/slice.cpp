/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "ced.h"
#include "cu.h"
#ifdef _MAC_CODE 
#import "AVController.h"
#elif _WIN32 
#endif

#if !defined(UNX)
#define _main slice_main
#define call slice_call
#define getflag slice_getflag
#define init slice_init
#define usage slice_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 1

extern struct tier *defheadtier;
extern char OverWriteFile;

#define slice_AVinfo struct SLIAVINFO
struct SLIAVINFO {
	char mediaInFPath[CS_MAX_PATH+FILENAME_MAX];
	char mediaOutFPath[CS_MAX_PATH+FILENAME_MAX];
	char mediaFName[CS_MAX_PATH+FILENAME_MAX];
	char isWhatType; // isAudio isVideo
} ;

/* *********************************************************** */

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		AddCEXExtension = "";
		LocalTierSelect = TRUE;
		stout = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		FilterTier = 0;
	} else {
	}
}

void usage() {
	puts("SLICE creates a media file clip with beg and end time range of the data file");
	printf("Usage: slice [%s] filename(s)\n", mainflgs());
	mainusage(FALSE);
	puts("Example:");
	puts("       slice *.cha");
	puts("    or");
	puts("       gem +sflood +n *.cha +d1");
	puts("       slice *.gem.cex");
	puts("       fixbullets +g *.gem.cex");
	cutt_exit(0);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SLICE_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
#ifndef _CLAN_DEBUG
//	fprintf(stderr, "\nSLICE command is not work on the Mac yet\n\n");
//	return;
#endif
#ifdef _WIN32 
	fprintf(stderr, "\nSLICE command does not work on Windows PC\n\n");
#else
	bmain(argc,argv,NULL);
#endif
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static BOOL slice_findFullMediaName(slice_AVinfo *AVinfo) {
	int pathLen, nameLen;
	char *rFileName;
	BOOL isFirstTimeAround;

	isFirstTimeAround = true;
	rFileName = AVinfo->mediaInFPath;
	pathLen = strlen(rFileName);
tryAgain:
	strcat(rFileName, AVinfo->mediaFName);
	nameLen = strlen(rFileName);
	strcat(rFileName, ".mov");
	AVinfo->isWhatType = isVideo;
	if (access(rFileName, 0)) {
		rFileName[nameLen] = EOS;
		uS.str2FNType(rFileName, strlen(rFileName), ".mp4");
		AVinfo->isWhatType = isVideo;
		if (access(rFileName, 0)) {
			rFileName[nameLen] = EOS;
			uS.str2FNType(rFileName, strlen(rFileName), ".mp3");
			AVinfo->isWhatType = isAudio;
			if (access(rFileName, 0)) {
				rFileName[nameLen] = EOS;
				uS.str2FNType(rFileName, strlen(rFileName), ".wav");
				AVinfo->isWhatType = isAudio;
				if (access(rFileName, 0)) {
					rFileName[nameLen] = EOS;
					uS.str2FNType(rFileName, strlen(rFileName), ".m4v");
					AVinfo->isWhatType = isVideo;
					if (access(rFileName, 0)) {
						rFileName[nameLen] = EOS;
						uS.str2FNType(rFileName, strlen(rFileName), ".aif");
						AVinfo->isWhatType = isAudio;
						if (access(rFileName, 0)) {
							rFileName[nameLen] = EOS;
							uS.str2FNType(rFileName, strlen(rFileName), ".avi");
							AVinfo->isWhatType = isVideo;
							if (access(rFileName, 0)) {
								rFileName[nameLen] = EOS;
								uS.str2FNType(rFileName, strlen(rFileName), ".wmv");
								AVinfo->isWhatType = isVideo;
								if (access(rFileName, 0)) {
									rFileName[nameLen] = EOS;
									uS.str2FNType(rFileName, strlen(rFileName), ".mpg");
									AVinfo->isWhatType = isVideo;
									if (access(rFileName, 0)) {
										rFileName[nameLen] = EOS;
										uS.str2FNType(rFileName, strlen(rFileName), ".aiff");
										AVinfo->isWhatType = isAudio;
										if (access(rFileName, 0)) {
											rFileName[nameLen] = EOS;
											uS.str2FNType(rFileName, strlen(rFileName), ".dv");
											AVinfo->isWhatType = isVideo;
											if (access(rFileName, 0)) {
												if (isFirstTimeAround) {
													isFirstTimeAround = false;
													rFileName[pathLen] = EOS;
													strcat(rFileName, "media/");
													goto tryAgain;
												}
												rFileName[nameLen] = EOS;
												AVinfo->isWhatType = 0;
												return(false);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return(true);
}

void call() {
	char err_mess[ERRMESSAGELEN], isBegTimeFound;
	char tFName[FILENAME_MAX+2], *s;
	long i, mBeg, mEnd;
	long begTime;
	long endTime;
	int64_t startTime, durationTime;
	slice_AVinfo AVinfo;
	AVAsset *mediaAsset;
	NSURL *mediaURL_in, *mediaURL_out;
	NSString *mediaOutPath;

	AVinfo.mediaInFPath[0] = EOS;
	AVinfo.mediaOutFPath[0] = EOS;
	AVinfo.mediaFName[0] = EOS;
	AVinfo.isWhatType = 0; // isAudio isVideo

	isBegTimeFound = FALSE;
	begTime = 0L;
	endTime = 0L;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE)) {
			getMediaName(utterance->line, AVinfo.mediaFName, FILENAME_MAX);
			templineC[0] = EOS;
			strcpy(err_mess, "Can't locate media filename:\n");
			strcat(err_mess, AVinfo.mediaInFPath);
			addFilename2Path(err_mess, AVinfo.mediaFName);
			strcat(err_mess, "\n or:\n");
			if (!slice_findFullMediaName(&AVinfo)) {
				strcat(err_mess, AVinfo.mediaInFPath);
				fprintf(stderr, "%s\n", err_mess);
				return;
			}

			if (AVinfo.isWhatType == isVideo) {
				fprintf(stderr, "\nError: Slice command does not work on video media: %s\n", AVinfo.mediaInFPath);
			} else
				;
			mediaURL_in = [NSURL fileURLWithPath:[NSString stringWithUTF8String:AVinfo.mediaInFPath]];
			mediaAsset = [AVAsset assetWithURL:mediaURL_in];
		} else {
			for (i=0L; utterance->line[i]; i++) {
				if (utterance->line[i] == HIDEN_C && (utterance->line[i+1] == '%' || isdigit(utterance->line[i+1]))) {
					if (utterance->line[i+1] == '%') {
						if (getOLDMediaTagInfo(utterance->line+i, NULL, tFName, &mBeg, &mEnd)) {
							fprintf(stderr, "\nError: Media bullet(s) in this file are in old format.\n");
							fprintf(stderr, "Please run \"fixbullets\" program to fix this data.\n\n");
							cutt_exit(0);
						}
					} else if (getMediaTagInfo(utterance->line+i, &mBeg, &mEnd)) {
						if (isBegTimeFound == FALSE)
							begTime = mBeg;
						endTime = mEnd;
						isBegTimeFound = TRUE;
					}
				}
			}
		}
	}

	if (AVinfo.mediaFName[0] == EOS) {
#ifdef UNX
		fprintf(stderr, "\nError: @Media: header is missing in this file.\n\n");
#else
		fprintf(stderr, "\n%c%cError: @Media: header is missing in this file.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
		return;
	}
	strcpy(AVinfo.mediaOutFPath, oldfname);
	s = strrchr(AVinfo.mediaOutFPath, '.');
	if (s != NULL) {
		*s = EOS;
		s = strrchr(AVinfo.mediaOutFPath, '.');
		if (s != NULL) {
			if (uS.mStricmp(s, ".gem") == 0)
				*s = EOS;
		}
	}
	strcat(AVinfo.mediaOutFPath, ".slice.wav");

	mediaOutPath = [NSString stringWithUTF8String:AVinfo.mediaOutFPath];
	mediaURL_out = [NSURL fileURLWithPath:mediaOutPath];
	AVAssetExportSession* assetExport = [[AVAssetExportSession alloc] initWithAsset:mediaAsset presetName:AVAssetExportPresetPassthrough];
	if ([[NSFileManager defaultManager] fileExistsAtPath:mediaOutPath])
	{
		[[NSFileManager defaultManager] removeItemAtPath:mediaOutPath error:nil];
	}

	[assetExport setOutputURL:mediaURL_out];
	[assetExport setOutputFileType:AVFileTypeWAVE];
//	[assetExport setShouldOptimizeForNetworkUse:YES];

	CMTimeRange timeRange;
	startTime = begTime; // / 1000.000;
	durationTime = (endTime - begTime); // / 1000.000;
	timeRange = CMTimeRangeMake(CMTimeMake(startTime, 1000), 
								CMTimeMake(durationTime, 1000));
	[assetExport setTimeRange:timeRange];	
	[assetExport exportAsynchronouslyWithCompletionHandler:
	 ^(void ) {
		 if (AVAssetExportSessionStatusCompleted == assetExport.status) {
		 } else if (AVAssetExportSessionStatusFailed == assetExport.status) {
			 NSLog(@"AVAssetExportSessionStatusFailed with error--%@", assetExport.error);
		 }
	 }
	 ];	
}
