/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*
1. make sure that MP3 media file is located in the same directory as data files.
2. in "Commands" window type "fixmp3s fname.cha". Result: "Please run fixmp3s again"
3. type "fixmp3s fname.cha" again. Result files with ext: .nmp.cex.
4. test those files with .wav media.
5. if it works OK, then type "ren -f *.nmp.cex *.cha"

*/

#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main fixMP3s_main
#define call fixMP3s_call
#define getflag fixMP3s_getflag
#define init fixMP3s_init
#define usage fixMP3s_usage
#endif

#include "mul.h" 
#include "mp3.h"
#if defined(_WIN32) || defined(__MACH__)
  #include "MP3ImporterPatch.h"
#endif

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#define NAMESTIMEFILE "mp3.kill"
extern struct tier *defheadtier;
extern char OverWriteFile;
extern char fgets_cr_lc;

extern "C"
{
extern long roundUp(double num);
}

#define NAMES_TIMES struct times
struct times {
	char *name;
	long time;
	NAMES_TIMES *nextNameTime;
} *rootNameTime;
static char areNamesTimesFound;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

static NAMES_TIMES *CleanupNamesTimes(NAMES_TIMES *p) {
	NAMES_TIMES *t;

	while (p != NULL) {
		t = p;
		p = p->nextNameTime;
		if (t->name != NULL)
			free(t->name);
		free(t);
	}
	return(NULL);
}

static long findTimeForName(char *name) {
	NAMES_TIMES *p = rootNameTime;

	while (p != NULL) {
		if (!strcmp(p->name, name)) {
			return(p->time);
		}
		p = p->nextNameTime;
	}
	return(0L);
}

static void addNameTiime(char *name, long time) {
	NAMES_TIMES *p;

	if (rootNameTime == NULL) {
		if ((p = NEW(NAMES_TIMES)) == NULL)
			out_of_mem();
		rootNameTime = p;
	} else {
		for (p=rootNameTime; p->nextNameTime != NULL; p=p->nextNameTime) {
			if (!strcmp(p->name, name))
				return;
		}
		if (!strcmp(p->name, name))
			return;
		if ((p->nextNameTime = NEW(NAMES_TIMES)) == NULL)
			out_of_mem();
		p = p->nextNameTime;
	}
	p->nextNameTime = NULL;
	p->name = (char *)malloc(strlen(name)+1);
	if (p == NULL) {
		CleanupNamesTimes(rootNameTime);
		out_of_mem();
	}
	strcpy(p->name, name);
	p->time = time;
}

static void writeOutNamesTimes(NAMES_TIMES *p) {
	FILE *fp;

	fp = fopen(NAMESTIMEFILE,"w");
	if (fp == NULL)
		return;

	if (areNamesTimesFound)
		fprintf(fp, "*\n");
	else if (p == NULL) {
			fprintf(fp, "\n\n    NO MP3 FILES WERE FOUND!!!\n");
	} else {
		while (p != NULL) {
			fprintf(fp, "%s %ld\n", p->name, p->time);
			p = p->nextNameTime;
		}
		fprintf(stderr, "\n\nPlease run fixmp3s again\n");
	}

	fclose(fp);
	if (areNamesTimesFound)
		unlink(NAMESTIMEFILE);
}

static void readNamesTimes(void) {
	FILE *fp;
	char text[1024];
	long i;

	fp = fopen(NAMESTIMEFILE,"r");
	if (fp == NULL)
		return;

	fgets_cr_lc = '\0';
	while (fgets_cr(text, 256, fp)) {
		uS.remFrontAndBackBlanks(text);
		if (!strcmp(text, "*")) {
			rootNameTime = CleanupNamesTimes(rootNameTime);
			break;
		}
		if (*text != EOS) {
			for (i=strlen(text)-1; i >= 0L && isdigit(text[i]); i--) ;
			if (i >= 0L) {
				text[i] = EOS;
				uS.remFrontAndBackBlanks(text);
				i++;
				addNameTiime(text, atol(text+i));
			}
		}
	}
	fclose(fp);
}

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		rootNameTime = NULL;

		readNamesTimes();
		if (rootNameTime == NULL) {
			areNamesTimesFound = FALSE;
			stout = TRUE;
		} else {
			areNamesTimesFound = TRUE;
			stout = FALSE;
		}
		if (!areNamesTimesFound && isMP3PatchLoaded) {
			fprintf(stderr, "\n\nPlease quit CLAN, re-start it, then run fixmp3s again\n");
			cutt_exit(0);
		}

	}
}

void usage() {
	printf("fixes bullet times\n");
	printf("Usage: fixmp3s [%s] filename\n", mainflgs());
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = FIXMP3;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
	writeOutNamesTimes(rootNameTime);
	CleanupNamesTimes(rootNameTime);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static long fixmp3s_conv_to_msec_MP3(long num, double ts) {
	double res, num2;

	res = (double)num;
	num2 = ts;
	res /= num2;
	res *= 1000.0000;
	return(roundUp(res));
}

static long fixmp3s_conv_from_msec_MP3(long num, double ts) {
	double res, num2;

	res = (double)num;
	res /= 1000.0000;
	num2 = ts;
	res *= num2;
	return(roundUp(res));
}

static void getSampleRate(FNType *FileName, double *oldSNDrate, double *newSNDrate) {
	char oldIsMP3PatchLoaded;
	Media  theSoundMedia;
	Handle hSys7SoundData;
	double res, num;
	OSErr err = noErr;
	FNType mFileName[FNSize];

	strcpy(mFileName, wd_dir);
	addFilename2Path(mFileName, FileName);
	if (strrchr(FileName, '.') == NULL)
		strcat(mFileName, ".mp3");

#if defined(_WIN32) || defined(__MACH__)
	if (areNamesTimesFound && !isMP3PatchLoaded)
		MP3ImporterPatch_DoRegister();
#endif

	oldIsMP3PatchLoaded = isMP3PatchLoaded;
	isMP3PatchLoaded = true;
	hSys7SoundData = NULL;
	theSoundMedia = NULL;
	if ((err=GetMovieMedia(mFileName,&theSoundMedia,&hSys7SoundData)) != noErr) {
		fprintf(stderr, "\n\nCan not read mp3 file (%d): %s.\n", err, FileName);
		CleanupNamesTimes(rootNameTime);
		isMP3PatchLoaded = oldIsMP3PatchLoaded;
		cutt_exit(0);
	}
	isMP3PatchLoaded = oldIsMP3PatchLoaded;

	if (!areNamesTimesFound) {
		addNameTiime(mFileName, GetMediaDuration(theSoundMedia));
	} else {
		*oldSNDrate = (double)GetMediaTimeScale(theSoundMedia);
		res = (double)findTimeForName(mFileName);
		num = (double)fixmp3s_conv_to_msec_MP3(GetMediaDuration(theSoundMedia), *oldSNDrate);
		*newSNDrate =  res / num * 1000.0000;
	}

	if (hSys7SoundData)
		DisposeHandle(hSys7SoundData);
	return;
}

static char getNewTime(double oldSNDrate, double newSNDrate, long *Beg, long *End) {
	long dur, oBeg, oEnd;

	oBeg = *Beg;
	dur = fixmp3s_conv_from_msec_MP3(*Beg, oldSNDrate);
	*Beg = fixmp3s_conv_to_msec_MP3(dur, newSNDrate);
	oEnd = *End;
	dur = fixmp3s_conv_from_msec_MP3(*End, oldSNDrate);
	*End = fixmp3s_conv_to_msec_MP3(dur, newSNDrate);
	if (oBeg != *Beg || oEnd != *End)
		return(TRUE);
	else
		return(FALSE);
}

static void replaceOldStTimes(UTTER *utt, long i, long Beg, long End) {
	long j;
	char sBeg[256], sEnd[256];

	if (utt->line[i] == HIDEN_C)
		i++;
	for (; utt->line[i] && utt->line[i] != ':' && !isdigit(utt->line[i]); i++) ;
	if (!isdigit(utt->line[i]))
		i++;
	while (utt->line[i] && (isSpace(utt->line[i]) || utt->line[i] == '_'))
		i++;
	if (!isdigit(utt->line[i]))
		i++;
	for (; utt->line[i] && utt->line[i] != '"' && !isdigit(utt->line[i]); i++) ;
	while (utt->line[i] && !isdigit(utt->line[i]))
		i++;

	sprintf(sBeg, "%ld", Beg);
	sprintf(sEnd, "%ld", End);
	for (j=i; utt->line[j] && isdigit(utt->line[j]); j++) ;
	while (utt->line[j] && !isdigit(utt->line[j]))
		j++;
	for (; utt->line[j] && isdigit(utt->line[j]); j++) ;
	att_cp(0, utt->line+i, utt->line+j, utt->attLine+i, utt->attLine+j);
	att_shiftright(utt->line+i, utt->attLine+i, strlen(sBeg)+strlen(sEnd)+1);
	for (j=0; sBeg[j] != EOS; j++) {
		utt->line[i] = sBeg[j];
		utt->attLine[i] = 0;
		i++;
	}
	utt->line[i] = '_';
	utt->attLine[i] = 0;
	i++;
	for (j=0; sEnd[j] != EOS; j++) {
		utt->line[i] = sEnd[j];
		utt->attLine[i] = 0;
		i++;
	}
}

void call() {
	char isMediaHeaderFound, mediaRes;
	FNType oldFname[FNSize], Fname[FNSize];
	long Beg, End;
	long bs, es;
	double oldSNDrate, newSNDrate;

	isMediaHeaderFound = FALSE;
	Beg = End = 0L;
	oldFname[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE)) {
			getMediaName(utterance->line, Fname, FNSize);
			isMediaHeaderFound = TRUE;
		}
		bs = 0L;
		for (es=bs; utterance->line[es] != EOS; es++) {
			if (utterance->line[es] == HIDEN_C) {
				if (isMediaHeaderFound)
					mediaRes = getMediaTagInfo(utterance->line+es, &Beg, &End);
				else
					mediaRes = getOLDMediaTagInfo(utterance->line+es, SOUNDTIER, Fname, &Beg, &End);
				if (mediaRes) {
					if (strcmp(oldFname, Fname)) {
						getSampleRate(Fname, &oldSNDrate, &newSNDrate);
						strcpy(oldFname, Fname);
					}

					if (areNamesTimesFound) {
						if (getNewTime(oldSNDrate, newSNDrate, &Beg, &End))
							replaceOldStTimes(utterance, es, Beg, End);
					}
				}
				for (bs=es+1; utterance->line[bs] != HIDEN_C && utterance->line[bs] != EOS; bs++) ;
				if (utterance->line[bs] == HIDEN_C)
					bs++;
			}
		}
		if (areNamesTimesFound)
			printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
	}
}
