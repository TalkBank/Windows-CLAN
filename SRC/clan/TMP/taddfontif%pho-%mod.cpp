// Add Palfont to %pho: tier
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

void usage()			/* print proper usage and exit */
{
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage();
}

void call() {
	int  i;
	char stf = FALSE, ftime;

	stf = FALSE;
	ftime = TRUE;
    currentchar = (char)getc_cr(fpin);
    while (getwholeutter()) {
		if (ftime) {
			ftime = FALSE;
			fprintf(fpout, "@Font:\tMonaco:9\n");
			if (partwcmp(utterance->speaker, FONTHEADER))
				continue;
		}
		strcpy(templineC,utterance->speaker);
		uS.uppercasestr(templineC, MBF);
		if (partcmp(templineC,"%PHO:",FALSE) || partcmp(templineC,"%MOD:",FALSE)) {
			uS.lowercasestr(uttline, MBF);
			if (partwcmp(uttline, FONTMARKER)) {
				for (i=0; utterance->line[i] != ']' && utterance->line[i]; i++) ;
				if (utterance->line[i] == ']') {
					if (utterance->line[++i] == ' ')
						i++;
					strcpy(utterance->line, utterance->line+i);
				}
			}
			if (!stf) {
				stf = TRUE;
				strcpy(uttline, "[%fnt: IPAPhon:12] ");
				strcat(uttline, utterance->line);
				strcpy(utterance->line, uttline);
			}
			printout(utterance->speaker, utterance->line, utterance->tierNumber);
		} else {
			if (stf) {
				stf = FALSE;
				uS.remblanks(uttline);
				if (partwcmp(utterance->speaker, FONTHEADER)) {
					fprintf(fpout, "@Font:\tMonaco:9\n");
					continue;
				} else if (*uttline == EOS)
					fprintf(fpout, "@Font:\tMonaco:9\n");
				else {
					uS.lowercasestr(uttline, MBF);
					if (partwcmp(uttline, FONTMARKER)) {
						for (i=0; utterance->line[i] != ']' && utterance->line[i]; i++) ;
						if (utterance->line[i] == ']') {
							if (utterance->line[++i] == ' ')
								i++;
							strcpy(utterance->line, utterance->line+i);
						}
					}
					strcpy(uttline, "[%fnt: Monaco:9] ");
					strcat(uttline, utterance->line);
					strcpy(utterance->line, uttline);
					printout(utterance->speaker, utterance->line, utterance->tierNumber);
					continue;
				}
			}
			fputs(utterance->speaker,fpout);
			fputs(utterance->line,fpout);			
		}
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
