/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "ced.h"
#include "MMedia.h"
#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main silence_main
#define call silence_call
#define getflag silence_getflag
#define init silence_init
#define usage silence_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 1

extern struct tier *defheadtier;
extern char OverWriteFile;

#define SILENCE struct SilenceList
struct SilenceList {
	long beg;
	long end;
	SILENCE *next;
} ;

static char mediaFName[FILENAME_MAX+2];
static SILENCE *TimesList;

/* *********************************************************** */

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		AddCEXExtension = "";
		stout = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		mediaFName[0] = EOS;
		TimesList = NULL;
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+.");
		addword('\0','\0',"+?");
		addword('\0','\0',"+!");
		maininitwords();
		mor_initwords();
		FilterTier = 1;
	}
}

void usage() {
	printf("Usage: silence [%s] filename(s)\n", mainflgs());
	mainusage(TRUE);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SILENCE_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static SILENCE *free_SilenceList(SILENCE *p) {
	SILENCE *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		free(t);
	}
	return(NULL);
}

static void freeAllMemory(char isQuit, sndInfo *tempSound) {
	TimesList = free_SilenceList(TimesList);
	if (tempSound->isMP3 == TRUE) {
		tempSound->isMP3 = FALSE;
		if (tempSound->mp3.hSys7SoundData)
			DisposeHandle(tempSound->mp3.hSys7SoundData);
		tempSound->mp3.theSoundMedia = NULL;
		tempSound->mp3.hSys7SoundData = NULL;
	} else if (tempSound->SoundFPtr != NULL)
		fclose(tempSound->SoundFPtr);
	tempSound->SoundFPtr = NULL;
	if (isQuit)
		cutt_exit(0);
}

static SILENCE *AddToSilenceList(SILENCE *root, long mBeg, long mEnd, sndInfo *tempSound) {
	SILENCE *nt, *tnt;
	
	if (root == NULL) {
		root = NEW(SILENCE);
		nt = root;
		if (nt == NULL) {
			do_warning("Out of memory.", 0);
			freeAllMemory(TRUE, tempSound);
		}
		nt->next = NULL;
	} else {
		tnt= root;
		nt = root;
		while (1) {
			if (nt == NULL)
				break;
			else if (nt->beg >= mBeg) {
				if (mBeg <= nt->end) {
					if (nt->end < mEnd)
						nt->end = mEnd;
					return(root);
				} else
					break;
			}
			tnt = nt;
			nt = nt->next;
		}
		if (nt == NULL) {
			tnt->next = NEW(SILENCE);
			nt = tnt->next;
			if (nt == NULL) {
				do_warning("Out of memory.", 0);
				freeAllMemory(TRUE, tempSound);
			}
			nt->next = NULL;
		} else if (nt == root) {
			root = NEW(SILENCE);
			root->next = nt;
			nt = root;
			if (nt == NULL) {
				do_warning("Out of memory.", 0);
				freeAllMemory(TRUE, tempSound);
			}
		} else {
			nt = NEW(SILENCE);
			if (nt == NULL) {
				do_warning("Out of memory.", 0);
				freeAllMemory(TRUE, tempSound);
			}
			nt->next = tnt->next;
			tnt->next = nt;
		}
	}
	nt->beg = mBeg;
	nt->end = mEnd;
	return(root);
}

static void silence(char *buf, Size beg, Size end) {
	for (; beg < end; beg++)
		buf[beg] = '\0';
}

static long convert2msec(sndInfo *tempSound, long num) {
	double res, num2;
	
	res = (double)num;
	num2 = tempSound->SNDrate / (double)1000.0000;
	res = res / num2;
	num2 = (double)tempSound->SNDsample;
	res = res / num2;
	num2 = (double)tempSound->SNDchan;
	res = res / num2;
	
	//	}
	return(roundUp(res));
}

static char ProcessSound(sndInfo *tempSound, SILENCE *p) {
	Size cur, count;
	long i, BegM, EndM;
	short	sampleSize;
	double		framesD1;
	unsigned int frames;
	SILENCE *t;
	FILE	*fpout = NULL;
	FNType	toMediaFName[FNSize];
	char t0, t1, t2, *s;
	
	if (tempSound->SNDformat == kULawCompression || tempSound->SNDformat == kALawCompression) {
		fprintf(stderr, "\nError can not process compressed sound file \"%s\"\n\n", tempSound->rSoundFile);
		return(0);
	}
	strcpy(toMediaFName, tempSound->rSoundFile);
	s = strrchr(toMediaFName, '.');
	if (s != NULL)
		*s = EOS;
	strcat(toMediaFName, "-silent.aif");
	if ((fpout=fopen(toMediaFName, "wb")) == NULL) {
		fprintf(stderr, "\nError creating sound file \"%s\"\n\n", toMediaFName);
		return(0);
	}
#ifdef _MAC_CODE
	settyp(toMediaFName, 'AIFF', 'TVOD', FALSE);
#endif
	sampleSize = tempSound->SNDsample * 8;
	WriteAIFFHeader(fpout, 0L, 0, tempSound->SNDchan, sampleSize, tempSound->SNDrate);
	cur = 0L;
	if (fseek(tempSound->SoundFPtr, cur+tempSound->AIFFOffset, SEEK_SET) != 0) {
		fprintf(stderr, "\nError(1) reading sound file \"%s\"\n\n", tempSound->rSoundFile);
		fclose(fpout);
		return(0);
	}
	while (cur < tempSound->SoundFileSize) {
		count = ((Size)SNDBUFSIZE < tempSound->SoundFileSize-cur ? (Size)SNDBUFSIZE : tempSound->SoundFileSize-cur);
		if (fread(tempSound->SWaveBuf, 1, count, tempSound->SoundFPtr) != count) {
			fprintf(stderr, "\nError(2) reading sound file \"%s\"\n\n", tempSound->rSoundFile);
			fclose(fpout);
			return(0);
		}
#ifdef _MAC_CODE
		if (tempSound->SFileType == 'WAVE' && byteOrder == CFByteOrderLittleEndian) {
			if (tempSound->SNDsample == 2) {
				for (i=0; i < count; i+=2) {
					t0 = tempSound->SWaveBuf[i];
					t1 = tempSound->SWaveBuf[i+1];
					tempSound->SWaveBuf[i+1] = t0;
					tempSound->SWaveBuf[i] = t1;
				}
			} else if (tempSound->SNDsample == 3) {
				for (i=0; i < count; i+=3) {
					t0 = tempSound->SWaveBuf[i];
					t1 = tempSound->SWaveBuf[i+1];
					t2 = tempSound->SWaveBuf[i+2];
					tempSound->SWaveBuf[i+2] = t0;
					tempSound->SWaveBuf[i+1] = t1;
					tempSound->SWaveBuf[i] = t2;
				}
			}
		}
		if (!tempSound->DDsound && tempSound->SNDsample == 1) {
			for (i=0; i < count; i++)
				tempSound->SWaveBuf[i] += 128;
		}
#endif
#ifdef _WIN32
		if (tempSound->SFileType == 'AIFF') {
			if (tempSound->SNDsample == 1) {
				for (i=0; i < count; i++)
					tempSound->SWaveBuf[i] += (char)128;
			} else {
				if (tempSound->DDsound) {
					for (i=0; i < count; i+=2) {
						t0 = tempSound->SWaveBuf[i];
						t1 = tempSound->SWaveBuf[i+1];
						tempSound->SWaveBuf[i+1] = t0;
						tempSound->SWaveBuf[i] = t1;
					}
				}
			}
		} else if (tempSound->SFileType == 'WAVE') {
			if (tempSound->SNDsample == 2) {
				for (i=0; i < count; i+=2) {
					t0 = tempSound->SWaveBuf[i];
					t1 = tempSound->SWaveBuf[i+1];
					tempSound->SWaveBuf[i+1] = t0;
					tempSound->SWaveBuf[i] = t1;
				}
			} else if (tempSound->SNDsample == 3) {
				for (i=0; i < count; i+=3) {
					t0 = tempSound->SWaveBuf[i];
					t1 = tempSound->SWaveBuf[i+1];
					t2 = tempSound->SWaveBuf[i+2];
					tempSound->SWaveBuf[i+2] = t0;
					tempSound->SWaveBuf[i+1] = t1;
					tempSound->SWaveBuf[i] = t2;
				}
			}
		}
#endif
		BegM = convert2msec(tempSound, cur);
		EndM = convert2msec(tempSound, cur+count);
		for (t=p; t != NULL; t=t->next) {
			if (t->end < BegM || t->beg > EndM)
				;
			else if (t->beg <= BegM && t->end >= BegM && t->beg <= EndM && t->end >= EndM) {
				silence(tempSound->SWaveBuf, 0, count);
			} else if (t->beg >= BegM && t->end <= EndM) {
				silence(tempSound->SWaveBuf, t->beg - BegM, t->end - BegM);
			} else if (t->beg <= BegM && t->end >= BegM && t->end <= EndM) {
				silence(tempSound->SWaveBuf, 0, t->end - BegM);
			} else if (t->beg >= BegM && t->beg <= EndM && t->end >= EndM) {
				silence(tempSound->SWaveBuf, t->beg - BegM, count);
			}
		}
		if (fwrite(tempSound->SWaveBuf, 1, count, fpout) != count) {
			fprintf(stderr, "\nError writting sound file \"%s\"\n\n", toMediaFName);
			fclose(fpout);
			return(0);
		}
		cur += count;
	}
	count = tempSound->SoundFileSize;
	framesD1 = (double)convert2msec(tempSound, count) / 1000.0000;
	framesD1 = framesD1 * (double)tempSound->SNDrate;
	frames = (unsigned int)(framesD1);
	if (fseek(fpout, 0, SEEK_SET) != 0) {
		fprintf(stderr, "\nError setting sound file \"%s\" position\n\n", toMediaFName);
		fclose(fpout);
		return(0);
	}
	WriteAIFFHeader(fpout, count, frames, tempSound->SNDchan, sampleSize, tempSound->SNDrate);
	fclose(fpout);
	return(1);
}

void call() {
	char word[BUFSIZ];
	char mFName[FILENAME_MAX+2];
	long startI, endI, mBeg, mEnd;
	sndInfo tempSound;

	tempSound.isMP3 = FALSE;
	tempSound.SoundFPtr = NULL;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE)) {
			getMediaName(utterance->line, mediaFName, FILENAME_MAX);
			u_strcpy(tempSound.SoundFile, mediaFName, FILENAME_MAX);
			if (tempSound.SoundFPtr != NULL)
				fclose(tempSound.SoundFPtr);
			tempSound.SoundFPtr = NULL;
			tempSound.isMP3 = FALSE;
			templineC[0] = EOS;
			if (!findSoundFileInWd(&tempSound, templineC)) {
				fprintf(stderr, "\nError: Can't locate media file \"%s\"\n", mediaFName);
				if (tempSound.errMess[0] != EOS)
					fprintf(stderr, "%s\n", tempSound.errMess);
				fprintf(stderr, "\n", mediaFName);
				freeAllMemory(FALSE, &tempSound);
				return;
			} else if (tempSound.isMP3 == TRUE) {
				tempSound.isMP3 = FALSE;
				if (tempSound.mp3.hSys7SoundData)
					DisposeHandle(tempSound.mp3.hSys7SoundData);
				tempSound.mp3.theSoundMedia = NULL;
				tempSound.mp3.hSys7SoundData = NULL;
				fprintf(stderr, "\nError: Can't work with MP3 media file\n\n");
				freeAllMemory(FALSE, &tempSound);
				return;
			}
		} else {
			startI = 0L;
			for (endI=startI; utterance->line[endI]; endI++) {
				if (utterance->line[endI] == HIDEN_C && (utterance->line[endI+1] == '%' || isdigit(utterance->line[endI+1]))) {
					if (utterance->line[endI+1] == '%') {
						if (getOLDMediaTagInfo(utterance->line+endI, NULL, mFName, &mBeg, &mEnd)) {
							fprintf(stderr, "\nError: Media bullet(s) in this file are in old format.\n");
							fprintf(stderr, "Please run \"fixbullets\" program to fix this data.\n\n");
							freeAllMemory(TRUE, &tempSound);
						}
					} else if (getMediaTagInfo(utterance->line+endI, &mBeg, &mEnd)) {
						while (1) {
							while (uttline[startI] != EOS && uS.isskip(uttline,startI,&dFnt,MBF) && !uS.isRightChar(uttline,startI,'[',&dFnt,MBF))
								startI++;
							if (uttline[startI] == EOS || startI >= endI)
								break;
							startI = getword(utterance->speaker, uttline, word, NULL, startI);
							if (startI == 0)
								break;
							if (exclude(word)) {
								TimesList = AddToSilenceList(TimesList, mBeg, mEnd, &tempSound);
								break;
							}
						}
						for (startI=endI+1; utterance->line[startI] && utterance->line[startI] != HIDEN_C ; startI++) ;
						if (utterance->line[startI] == EOS)
							break;
					}
				}
			}
		}
	}
	if (!ProcessSound(&tempSound, TimesList)) {
		freeAllMemory(FALSE, &tempSound);
		return;
	}
	TimesList = free_SilenceList(TimesList);
}
