#include "cu.h"
#include "ced.h"
#include "MMedia.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#if defined(_MAC_CODE)
ClanProgInfo gToolInfo = {
	_main,
	usage,
	getflag,
	init,
	call,
	NULL
};
#endif /* _MAC_CODE */

extern struct tier *defheadtier;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */
char fnames[300][FILENAME_MAX];
long names[300];
int  nIndex;

static long CWtoMsec(char *from) {
	int t;

	t = atoi(from) * 3600;
	while (*from != ':' && *from != '-')
		from++;
	from++;
	t += (atoi(from) * 60);
	while (*from != ':' && *from != '-')
		from++;
	from++;
	t += atoi(from);
	while (*from != ':' && *from != '-')
		from++;
	from++;
	t *= 1000;
	t += (atoi(from) * 33);
	return(t);
}

static void readInMovieFNames() {
#ifdef _MAC_CODE
	FileParam pb2;
#endif /* _MAC_CODE */
#ifdef _WIN32
	CString fname;
#endif /* _WIN32 */
	int  index;
	char *t;
	char filename[FILENAME_MAX];

	index = 1;
	nIndex = 0;
	SetNewVol(wd_ref, wd_st_full);
#ifdef _MAC_CODE
	CtoPstr(filename);
	pb2.ioFDirIndex = index;
	pb2.ioNamePtr	= (StringPtr)filename;
	pb2.ioVRefNum	= 0;
	pb2.ioFVersNum	= 0;
	while(PBGetFInfoSync((ParmBlkPtr)&pb2) != fnfErr) {
 		PtoCstr((unsigned char *)filename);
		if ((t=strrchr(filename, '.')) != NULL) {
			strcpy(templineC, t);
			uS.uppercasestr(templineC, defScript, FALSE);
			if (!strcmp(templineC, ".MOV")) {
				strcpy(fnames[nIndex], filename);
				names[nIndex++] = CWtoMsec(filename);
			}
		}
		CtoPstr(filename);
		pb2.ioFDirIndex = ++index;
	}
#endif /* _MAC_CODE */
#ifdef _WIN32
	if (!fileFind.FindFile("*.*", 0)) {
		fileFind.Close();
		return;
	}
	while (fileFind.FindNextFile()) {
		if (fileFind.IsDirectory())
			continue;
		fname = fileFind.GetFileName();
		strcpy(filename, fname);
		if ((t=strrchr(filename, ".")) != NULL) {
			strcpy(templineC, t);
			uS.uppercasestr(templineC, defScript, FALSE);
			if (!strcmp(templineC, ".MOV")) {
				strcpy(fnames[nIndex], filename);
				names[nIndex++] = CWtoMsec(filename);
			}
		}
	}
	if (!fileFind.IsDirectory()) {
		fname = fileFind.GetFileName();
		strcpy(filename, fname);
		if ((t=strrchr(filename, '.')) != NULL) {
			strcpy(templineC, t);
			uS.uppercasestr(templineC, defScript, FALSE);
			if (!strcmp(templineC, ".MOV")) {
				strcpy(fnames[nIndex], filename);
				names[nIndex++] = CWtoMsec(filename);
			}
		}
		fileFind.Close();
	}
#endif /* _WIN32 */
}

void init(char f) {
	if (f) {
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		readInMovieFNames();
	}
}

void usage() {
	printf("Usage: movePostCodes [%s] filename(s)\n",mainflgs());
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
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

static TimeValue getMovieDuration(char *fname) {
	OSErr 	err;
	short 	movieResRefNum;
	short 	actualResId;
	FSSpec  sfFile;
	Movie	MvMovie;
	TimeValue val;
	TimeScale ts;

	CtoPstr(fname);
	FSMakeFSSpec((int)wd_ref, 0, (StringPtr)fname, &sfFile);
	PtoCstr((StringPtr)fname);
	if (moviesEnabled == 0) {
		if (EnterMovies()) {
			return 0L;
		}
	}
	moviesEnabled++;
	err = OpenMovieFile(&sfFile, &movieResRefNum, fsRdPerm);
	if (err != noErr) {
		return 0L;
	}
	actualResId = DoTheRightThing;
	err = NewMovieFromFile(&MvMovie, movieResRefNum, &actualResId, NULL, newMovieActive, NULL);
	if (err != noErr) {
		return 0L;
	}
	err = CloseMovieFile(movieResRefNum);
	if (err != noErr) {
		return 0L;
	}
	ts = GetMovieTimeScale(MvMovie);
	val = GetMovieDuration(MvMovie);
	val = conv_to_msec_mov(val, ts);
	DisposeMovie(MvMovie);
	if (moviesEnabled == 1)
		ExitMovies();
	moviesEnabled--;
	return(val);
}

static void findFileOffset(char *st, long fromMsec, long toMsec) {
	int i, maxFrom, maxTo;
	long cMax;
	TimeValue FileLen;

	cMax = 0L;
	maxFrom = maxTo = -1;
	for (i=0; i < nIndex; i++) {
		if (fromMsec >= names[i] && names[i] >= cMax) {
			maxFrom = i;
			cMax = names[i];
		}
	}
	cMax = 0L;
	for (i=0; i < nIndex; i++) {
		if (toMsec >= names[i] && names[i] >= cMax) {
			maxTo = i;
			cMax = names[i];
		}
	}
	if (maxFrom == -1) {
		fprintf(stderr, "Can't find segment that beginning time \"%ld\" is in\n", fromMsec);
		cutt_exit(0);
	}
	if (maxTo == -1) {
		fprintf(stderr, "Can't find segment that ending time \"%ld\" is in\n", toMsec);
		cutt_exit(0);
	}
	if (maxFrom != maxTo) {
/*
		fprintf(stderr, "%s%s", utterance->speaker,utterance->line);
		fprintf(stderr, "Starting time \"%ld\" and ending time \"%ld\" are in different movie segments\n", fromMsec, toMsec);
		cutt_exit(0);
*/
		FileLen = getMovieDuration(fnames[maxFrom]);
		sprintf(st, "\"%s\"_%ld_%ld%c %c%%mov:\"%s\"_%ld_%ld",
					fnames[maxFrom],fromMsec-names[maxFrom],FileLen, HIDEN_C,
					HIDEN_C, fnames[maxTo],0L,toMsec-names[maxTo]);
	} else
		sprintf(st, "\"%s\"_%ld_%ld", fnames[maxFrom], fromMsec-names[maxFrom], toMsec-names[maxFrom]);
}

static char *convFwToMsec(char *from, char *to, long *i) {
	long pos = *i;
	long fromMsec, toMsec;

	to[pos++] = *from;
	from++;
	if (!strncmp(from, "%fwdv:", 6)) {
		strncpy(to+pos, "%mov:", 5);
		pos += 5;
		from += 6;
		while (*from != '"')
			to[pos++] = *from++;
		from++;
		while (*from != '"')
			from++;
		from++;
		from++;
		fromMsec = CWtoMsec(from);
		while (*from != '_')
			from++;
		from++;
		toMsec = CWtoMsec(from);
		while (*from != HIDEN_C)
			from++;
		findFileOffset(to+pos, fromMsec, toMsec);
		pos += strlen(to+pos);
		to[pos++] = *from++;
	}
	*i = pos;
	return(from);
}

static void ConvertFWToMSecs(char *from, char *to) {
	long i;
	
	i = 0;
	for (; *from != EOS; from++) {
		if (*from == HIDEN_C) {
			from = convFwToMsec(from, to, &i);
		}
		to[i++] = *from;
	}
	to[i] = EOS;
}

void call() {
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			ConvertFWToMSecs(utterance->line, templineC3);
			printout(utterance->speaker,templineC3,NULL,NULL,utterance->tierNumber,TRUE);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,utterance->tierNumber,FALSE);
		}
	}
}
