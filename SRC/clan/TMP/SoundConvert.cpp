#include "cu.h"
#include "ced.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "MMedia.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main sndconv_main
#define call sndconv_call
#define getflag sndconv_getflag
#define init sndconv_init
#define usage sndconv_usage
#endif

#include "mul.h" 

void init(char f) {
}

void usage() {
	printf("Usage: soundconv filename(s)\n");
}

#if defined(_MAC_CODE)
static char LocateAndOpenSoundFile(void) {
	char  isOpened;
	myMacDirRef tsvRefNum;
	Str255 fName;
	FSSpec sfFile;

	isOpened = FALSE;
	global_df->SnTr.SoundFPtr = 0;
	SetNewVol(lib_ref, nil);
	tsvRefNum.vRefNum = wd_ref.vRefNum;
	tsvRefNum.parID = wd_ref.parID;

	strcpy((char *)fName, global_df->SnTr.SoundFile);
	CtoPstr((char *)fName);
	if (HOpenDF(tsvRefNum.vRefNum,tsvRefNum.parID,fName,fsRdPerm,&global_df->SnTr.SoundFPtr) != noErr) {
		if (!GetNewSoundFile(global_df->SnTr.SoundFile))
			return(0);
	} else {
		getFileType((StringPtr)fName, &tsvRefNum, nil, &global_df->SnTr.SFileType);
		if (global_df->SnTr.SFileType == 'MP3!' && !global_df->SnTr.isMp3) {
			fprintf(stderr, "MP3 files are not supported\n");
			return(0);
		}
		if (global_df->SnTr.SFileType != 'AIFF' && global_df->SnTr.SFileType != 'AIFC' && 
			global_df->SnTr.SFileType != 'WAVE' && global_df->SnTr.SFileType != 'MP3!') {
			FSMakeFSSpec(tsvRefNum.vRefNum, tsvRefNum.parID, (StringPtr)fName, &sfFile);
			if ((global_df->ResRefNum=HOpenResFile(sfFile.vRefNum, sfFile.parID, sfFile.name,fsRdPerm)) == -1) {
				global_df->SnTr.SoundFile[0] = EOS;
				FSClose(global_df->SnTr.SoundFPtr);
				global_df->SnTr.SoundFPtr = 0;
				fprintf(stderr, "Can't open resources of sound file: %s", global_df->SnTr.SoundFile);
				return(0);
			}
		}
	}
	if (global_df->SnTr.SoundFPtr == 0 && !global_df->SnTr.isMp3) {
		fprintf(stderr, "Can't open sound file: %s", global_df->SnTr.SoundFile);
		global_df->SnTr.SoundFile[0] = EOS;
		return(0);
	}
	if (!CheckRateChan()) {
		global_df->SnTr.SoundFile[0] = EOS;
		if (global_df->SnTr.isMp3) {
			global_df->SnTr.isMp3 = FALSE;
			if (global_df->SnTr.mp3.hSys7SoundData)
				DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
			global_df->SnTr.mp3.theSoundMedia = NULL;
			global_df->SnTr.mp3.hSys7SoundData = NULL;
		} else {
			FSClose(global_df->SnTr.SoundFPtr);
			global_df->SnTr.SoundFPtr = 0;
			if (global_df->SnTr.SFileType != 'AIFF' && global_df->SnTr.SFileType != 'AIFC' && 
				global_df->SnTr.SFileType != 'WAVE' && global_df->SnTr.SFileType != 'MP3!')
				CloseResFile(global_df->ResRefNum);
		}
		global_df->SnTr.SoundFPtr = 0;
		fprintf(stderr, "Can't understand format of sound file: %s", global_df->SnTr.SoundFile);
		return(0);
	}
	return(1);
}
#endif
#if defined(_WIN32)
static char LocateAndOpenSoundFile(void) {
	char fName[FILENAME_MAX];

	global_df->SnTr.SoundFPtr = NULL;
	SetNewVol(0, wd_st_full);
	strcpy(global_df->SnTr.rSoundFile, wd_st_full);
	strcpy(fName, global_df->SnTr.SoundFile); 
	if ((global_df->SnTr.SoundFPtr=fopen(fName,"rb")) == NULL) {
		if (!GetNewSoundFile(global_df->SnTr.SoundFile)) {
			global_df->SnTr.SoundFile[0] = EOS;
			return(0);
		}
	} else {
		strcat(global_df->SnTr.rSoundFile, fName);
		getFileType(global_df->SnTr.rSoundFile, &global_df->SnTr.SFileType);
	}

	if (global_df->SnTr.SFileType == 'MP3!' && !global_df->SnTr.isMp3) {
		fprintf(stderr, "MP3 files are not supported\n");
		return(0);
	}

	if (global_df->SnTr.SoundFPtr == NULL && !global_df->SnTr.isMp3) {
		fprintf(stderr, "Can't open sound file: %s", global_df->SnTr.SoundFile);
		global_df->SnTr.SoundFile[0] = EOS;
		return(0);
	}
	if (!CheckRateChan()) {
		global_df->SnTr.SoundFile[0] = EOS;
		if (global_df->SnTr.isMp3) {
			global_df->SnTr.isMp3 = FALSE;
			if (global_df->SnTr.mp3.hSys7SoundData)
				DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
			global_df->SnTr.mp3.theSoundMedia = NULL;
			global_df->SnTr.mp3.hSys7SoundData = NULL;
		} else {
			if (global_df->SnTr.SoundFPtr != NULL)
				fclose(global_df->SnTr.SoundFPtr);
			global_df->SnTr.SoundFPtr = NULL;
		}
		fprintf(stderr, "Can't understand format of sound file: %s", global_df->SnTr.SoundFile);
		return(0);
	}
	return(1);
}
#endif

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int i;

#if defined(_MAC_CODE) || defined(_WIN32)
	extern char AcceptAllFiles;

	CLAN_PROG_NUM = SNDCONV;
	AcceptAllFiles = TRUE;

	// unix argv extender: *.* if bmain is not called
	if ((argc=expandArgv(argc, argv)) == 0) {
		return;
	}
	if (argc == 1) {
		usage();
		return;
	}
	for (i=1; i < argc; i++) {
		fprintf(stderr, "Converting file <%s>\n", argv[i]);
		strcpy(global_df->SnTr.SoundFile, argv[i]);
		strcpy(global_df->SnTr.rSoundFile, argv[i]);
		if (!LocateAndOpenSoundFile())
			return;
		global_df->SnTr.BegF = 0L;
		global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
		global_df->err_message[0] = EOS;
		SaveSoundClip(&global_df->SnTr, FALSE);
		if (global_df->err_message[0] != EOS) {
			fputs(global_df->err_message+1, stderr);
			global_df->err_message[0] = EOS;
			return;
		}
	}
#endif
}
		
void getflag(char *f, char *f1, int *i) {
}

void call() {
}
