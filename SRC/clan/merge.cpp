/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main merge_main
#define call merge_call
#define getflag merge_getflag
#define init merge_init
#define usage merge_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

extern char OverWriteFile;
extern struct tier *headtier;
extern struct tier *defheadtier;

void usage() {
	puts("merge *.cha file with *.txt file inthe same folder");
	printf("Usage: temp filename(s)\n");
	mainusage(TRUE);
}

void init(char first) {
	if (first) {
		stout = FALSE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		onlydata = 1;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

void call(void) {
	char *ext;
	char hebrewFName[FNSize];
	FILE *hebrewFp;

	strcpy(hebrewFName, wd_dir);
	addFilename2Path(hebrewFName, oldfname);
	ext = strrchr(hebrewFName, '.');
	if (ext != NULL)
		*ext = EOS;
	strcat(hebrewFName, ".txt");
	if ((hebrewFp=fopen(hebrewFName, "r")) == NULL) {
#ifdef UNX
		fprintf(stderr,"\NCAN'T OPEN HEBREW FILE \"%s\"\n\n", hebrewFName);
#else
		fprintf(stderr,"\n%c%CCAN'T OPEN HEBREW FILE \"%s\"%c%c\n\n", ATTMARKER, error_start, hebrewFName, ATTMARKER, error_end);
#endif
		return;
	}
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			if (fgets_cr(spareTier1, UTTLINELEN, hebrewFp)) {
				printout(utterance->speaker,spareTier1,utterance->attSp,NULL,FALSE);
			}
			printout("%ort:",utterance->line,NULL,utterance->attLine,FALSE);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
	fclose(hebrewFp);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = MERGE;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}
