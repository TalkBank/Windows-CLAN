/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"

#if !defined(UNX)
#define _main longtier_main
#define call longtier_call
#define getflag longtier_getflag
#define init longtier_init
#define usage longtier_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define CHAT_MODE 3

extern struct tier *defheadtier;
extern char OverWriteFile;

void usage()	{		/* print proper usage and exit */
    puts("LONGTIER");
    printf("Usage: longtier [o %s] filename(s)\n",mainflgs());
    mainusage(TRUE);
}

void init(char f) {
    if (f) {
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL) free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		onlydata = 1;
		OverWriteFile = TRUE;
    }
}

void call() {
    int i;
    char isSpaceFound;

	currentatt = 0;
    currentchar = (char)getc_cr(fpin, &currentatt);
    while (getwholeutter()) {
		isSpaceFound = FALSE;
		for (i=0; uttline[i] != EOS; i++) {
		    if (uttline[i] == '\n') {
		    	if (!isSpaceFound) {
		    		uttline[i] = ' ';
		    	} else if (uttline[i+1] != EOS) {
		    		att_cp(0, utterance->line+i, utterance->line+i+1, utterance->attLine+i, utterance->attLine+i+1);
		    		i--;
				}
			}
			if (i >= 0) {
				if (uttline[i] == ' ')
					isSpaceFound = TRUE;
				else
					isSpaceFound = FALSE;
			}
		    if (uttline[i] == '\t') {
		    	att_cp(0, utterance->line+i, utterance->line+i+1, utterance->attLine+i, utterance->attLine+i+1);
		    	i--;
		    }
		}
		if (uttline[i-1] == ' ')
			uttline[i-1] = '\n';
		fputs(utterance->speaker, fpout);
		fputs(uttline, fpout);
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

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    chatmode = CHAT_MODE;
    CLAN_PROG_NUM = LONGTIER;
    OnlydataLimit = 0;
    UttlineEqUtterance = TRUE;
    bmain(argc,argv,NULL);
}
