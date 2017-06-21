#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

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

char postCodes[UTTLINELEN];
char spChanged[20];
char spname[20][UTTLINELEN];
char line[20][UTTLINELEN];

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

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

static char alreadyThere(char *line, char *postCodes) {
	int i;

	strcpy(templineC2, line);
	uS.lowercasestr(templineC2, FALSE);
	if (*postCodes == ' ')
		postCodes++;
	strcpy(templineC3, postCodes);
	uS.lowercasestr(templineC3, FALSE);
	for (i=0; templineC2[i] != EOS; i++) {
		if (partwcmp(templineC2+i, templineC3))
			return(TRUE);
	}
	return(FALSE);
}

static void ifFoundThenCopy(int max) {
	int  i, j, orgJ, row;
	char *p2;
	char *trn;

	for (row=0; row < max; row++) {
		j = 0;
		if (partcmp(spname[row], "%mor:", FALSE) || partcmp(spname[row], "%trn:", FALSE)) {
			i = 0;
			trn = line[row];
			while (trn[i] != EOS) {
				if (trn[i] == '[' && trn[i+1] == '+' && trn[i+2] == ' ') {
					orgJ = j;
					postCodes[j++] = ' ';
					while (trn[i] != ']' && trn[i] != EOS)
						postCodes[j++] = trn[i++];
					if (trn[i] == ']')
						postCodes[j++] = ']';
					else
						j = orgJ;
				} else
					i++;
			}
			postCodes[j] = EOS;
		}
		if (j > 0) {
			if (partcmp(spname[row], "%mor:", FALSE)) {
				p2 = "%trn:";
			} else {
				p2 = "%mor:";
			}
			for (i=0; i < max; i++) {
				if (*spname[i] == '*') {
					if (!alreadyThere(line[i], postCodes)) {
						strcat(line[i], postCodes);
						spChanged[i] = TRUE;
					}
				}
			}
			for (i=0; i < max; i++) {
				if (partcmp(spname[i], p2, FALSE)) {
					if (!alreadyThere(line[i], postCodes)) {
						strcat(line[i], postCodes);
						spChanged[i] = TRUE;
					}
				}
			}
		}
	}
}

void call() {
	int i, max;

	max = 0;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			ifFoundThenCopy(max);

			for (i=0; i < max; i++) {
				printout(spname[i], line[i], NULL, NULL, 0, spChanged[i]);
			}
			max = 0;
		}
		if (max >= 20) {
			fputs("\nERROR: Internal error increase tier array size.\n\n",stderr);
			cutt_exit(0);
		}
		spChanged[max] = FALSE;
		strcpy(spname[max], utterance->speaker);
		strcpy(line[max], utterance->line);
		max++;
	}
	ifFoundThenCopy(max);
	for (i=0; i < max; i++) {
		printout(spname[i], line[i], NULL, NULL, 0, spChanged[i]);
	}
}
