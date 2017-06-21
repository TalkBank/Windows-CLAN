#include "cu.h"
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

extern struct tier *defheadtier;
extern char OverWriteFile;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void usage() {
	printf("if ALL words have @s, then replace them with [+ sec]\n");
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
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

static void fixAtS(char *line, AttTYPE *attLine) {
	char isAtSFound;
	long i, oldI, j;

	isAtSFound = FALSE;
	i = 0L;
	while ((i=getword(templineC,i))) {
		if (!uS.patmat(templineC, "*@s"))
			return;
		else
			isAtSFound = TRUE;
	}
	if (!isAtSFound)
		return;
	i = 0L;
	oldI = i;
	while ((i=getword(templineC,i))) {
		if (!uS.patmat(templineC, "*@s"))
			return;
		else {
			for (j=oldI; j < i; j++) {
				if (line[j] == '@' && line[j+1] == 's') {
					line[j] = ' '; line[j+1] = ' ';
					attLine[j] = 0; attLine[j+1] = 0; 
					break;
				}
			}
			isAtSFound = TRUE;
		}
		oldI = i;
	}
	for (i=0L; line[i] != EOS; ) {
		if (line[i] == ' ' && line[i+1] == ' ') {
			att_cp(0L, line+i, line+i+1, attLine+i, attLine+i+1);
		} else
			i++;
	}
	
}

void call() {
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			fixAtS(utterance->line, utterance->attLine);
		}
		printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
	}
}
