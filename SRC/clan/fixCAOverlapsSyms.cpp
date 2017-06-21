/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

#if !defined(UNX)
#define _main fixoverlapsyms_main
#define call fixoverlapsyms_call
#define getflag fixoverlapsyms_getflag
#define init fixoverlapsyms_init
#define usage fixoverlapsyms_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

extern struct tier *defheadtier;
extern char OverWriteFile;


void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void usage() {
	printf("changes old CA overlap symbols '[]' to new ones\n");
	printf("Usage: fixoverlapsyms [%s] filename(s)\n", mainflgs());
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = FIXOVLPSYM;
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

static char FindOVs(char *isOpenOV, char *line, AttTYPE *attLine) {
	char isFoundOVS, isOnlyDown;
	long i;

	isOnlyDown = FALSE;
	isFoundOVS = FALSE;
	for (i=0L; line[i]; i++) {
		if (line[i] == '[' && line[i+1] != '%') {
			isFoundOVS = TRUE;
			if (*isOpenOV == 0)
				isOnlyDown = TRUE;
			if (*isOpenOV == 0 || isOnlyDown) {
				att_shiftright(line+i, attLine+i, 2);
				attLine[i] = attLine[i+2];
				attLine[i+1] = attLine[i+2];
				line[i++] = 0xe2;
				line[i++] = 0x8c;
				line[i++] = 0x88;
			} else {
				att_shiftright(line+i, attLine+i, 2);
				attLine[i] = attLine[i+2];
				attLine[i+1] = attLine[i+2];
				line[i++] = 0xe2;
				line[i++] = 0x8c;
				line[i++] = 0x8a;
			}
			while (line[i] != ']' && line[i] != EOS)
				i++;
			if (line[i] == EOS) {
				fprintf(stderr,"*** File \"%s\": ", oldfname);
				fprintf(stderr,"line %ld.\n", lineno);
				fprintf(stderr, "Missing ']'.\n");
				cutt_exit(0);
			}
			if (*isOpenOV == 0 || isOnlyDown) {
				att_shiftright(line+i, attLine+i, 2);
				attLine[i] = attLine[i+2];
				attLine[i+1] = attLine[i+2];
				line[i++] = 0xe2;
				line[i++] = 0x8c;
				line[i++] = 0x89;
				*isOpenOV += 1;
			} else {
				att_shiftright(line+i, attLine+i, 2);
				attLine[i] = attLine[i+2];
				attLine[i+1] = attLine[i+2];
				line[i++] = 0xe2;
				line[i++] = 0x8c;
				line[i++] = 0x8b;
				*isOpenOV -= 1;
			}
		}
	}
	return(isFoundOVS);
}

void call() {
	char isOpenOV, isFoundOVS, spname[SPEAKERLEN];

	isOpenOV = 0;
	spname[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			isFoundOVS = FindOVs(&isOpenOV, utterance->line, utterance->attLine);
			strcpy(templineC, utterance->speaker);
			uS.remblanks(templineC);
			if (!strcmp(spname, templineC)) {
				if (!isFoundOVS && isOpenOV) {
					fprintf(stderr,"*** File \"%s\": ", oldfname);
					fprintf(stderr,"line %ld.\n", lineno);
					fprintf(stderr, "Possibly missing closing overlap marker.\n");
					fprintf(stderr, "Or problem with overlap marker few line above.\n");
					isOpenOV = 0;
				}
			}
			strcpy(spname, templineC);
		}
		printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
	}
}
