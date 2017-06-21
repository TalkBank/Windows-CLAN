#define VERSION "(16-07-97)"

#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define CHAT_MODE 3

extern struct tier *defheadtier;

void init(char f) {
	if (f) {
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void usage()			/* print proper usage and exit */
{
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage();
}


void call() {
    currentchar = (char)getc_cr(fpin);
	while (getwholeutter()) {
		if (partcmp(utterance->speaker,"@Font:",FALSE)) {
			continue;
		} else if (partcmp(utterance->speaker,"@Begin",FALSE)) {
			fputs("@Font:	Win95:Chn System:-13\n", fpout);
		}
		fputs(utterance->speaker, fpout);
		fputs(utterance->line, fpout);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	lversion = VERSION;
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
