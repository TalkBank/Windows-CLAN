/**********************************************************************
	"Copyright 2010 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h"

#define IS_WIN_MODE FALSE
#define CHAT_MODE 1

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char GExt[];

void usage() {
	printf("Usage: sec [%s] filename(s)\n",mainflgs());
	mainusage();
}

void init(char first) {
	if (first) {
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
/*
		addword('\0','\0',"+.");
		addword('\0','\0',"+?");
		addword('\0','\0',"+!");
*/
		maininitwords();
		stout = FALSE;
		onlydata = 1;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		strcpy(GExt, ".sec");
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

static void replaceAtsWithSec(void) {
	int i;

	for (i=0; utterance->line[i] != EOS; i++) {\
		if (utterance->line[i] == '@' && utterance->line[i+1] == 's' && uS.isskip(utterance->line,i+2,&dFnt,MBF))
			att_cp(0, utterance->line+i, utterance->line+i+2, utterance->attLine+i, utterance->attLine+i+2);
	}
	i = strlen(utterance->line) - 1;
	while (i >= 0 && isspace(utterance->line[i]))
		i--;
	i++;
	if (i > 0 && utterance->line[i-1] == HIDEN_C) {
		i = i - 2;
		while (i >= 0 && utterance->line[i] != HIDEN_C)
			i--;
		if (i > 0 && utterance->line[i] == HIDEN_C) {
			i--;
		} else if (utterance->line[i] != HIDEN_C) {
			i++;
		}
	}
	att_shiftright(utterance->line+i, utterance->attLine+i, 8);
	utterance->line[i] = ' ';
	utterance->attLine[i] = 0;
	i++;
	utterance->line[i] = '[';
	utterance->attLine[i] = 0;
	i++;
	utterance->line[i] = '+';
	utterance->attLine[i] = 0;
	i++;
	utterance->line[i] = ' ';
	utterance->attLine[i] = 0;
	i++;
	utterance->line[i] = 's';
	utterance->attLine[i] = 0;
	i++;
	utterance->line[i] = 'e';
	utterance->attLine[i] = 0;
	i++;
	utterance->line[i] = 'c';
	utterance->attLine[i] = 0;
	i++;
	utterance->line[i] = ']';
	utterance->attLine[i] = 0;
}

void call() {
	int i;
	char areAllATs, *at, word[BUFSIZ];

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (lineno > tlineno) {
			tlineno = lineno + 200;
			fprintf(stderr,"\r%ld ",lineno);
		}
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/

		if (utterance->speaker[0] == '*') {
			filterwords(utterance->speaker,uttline,excludedef);
			filterwords(utterance->speaker,uttline,exclude);
			areAllATs = TRUE;
			i = 0;
			while ((i=getword(word,i))) {
				at = strchr(word, '@');
				if (word[0] == '[' || word[0] == '#' || word[0] == HIDEN_C)
					;
				else if (at != NULL) {
					if (strcmp(at, "@s") != 0) {
						areAllATs = FALSE;
						break;
					}
				} else {
					areAllATs = FALSE;
					break;
				}
			}
			if (areAllATs)
				replaceAtsWithSec();
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,TRUE);
		} else
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
	fprintf(stderr, "\r	  \r");
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = TEMP;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}
