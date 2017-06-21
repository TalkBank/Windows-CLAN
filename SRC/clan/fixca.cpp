/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main fixca_main
#define call fixca_call
#define getflag fixca_getflag
#define init fixca_init
#define usage fixca_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
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

void usage() {
	printf("Removes line numbers from old style of CA data files\n");
	printf("Usage: Fixca [%s] filename(s)\n",mainflgs());
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = FIXCA;
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

static char isSpName(char *st, long i) {
	int j;

	if (st[i] == '@' || st[i] == '%' || st[i] == '*')
		return(TRUE);

	for (j=0L; st[i] != ':' && st[i] != EOS && j < 10; i++, j++) ;
	if (st[i] == ':' && (isSpace(st[i+1]) || st[i+1] == '\n' || st[i+1] == EOS))
		return(TRUE);
	else
		return(FALSE);
}

void call() {
	long i, j;

	while (!feof(fpin) && fgets_cr(templineC, UTTLINELEN, fpin)) {
		i = 0L;
		if (templineC[i] == ATTMARKER && SetTextAtt(NULL,templineC[i+1], NULL)) {
			do {
				i += 2;
			} while (templineC[i] == ATTMARKER && SetTextAtt(NULL,templineC[i+1], NULL)) ;
		}
		if (isdigit(templineC[i])) {
			j = i;
			for (; isdigit(templineC[i]); i++) ;
			if (templineC[i] == ':' || templineC[i] == ')' || templineC[i] == '.')
				i++;
			for (; isSpace(templineC[i]); i++) ;
			if (!isSpName(templineC, i)) {
				if (i > 0L) {
					i--;
					templineC[i] = '\t';
				}
			}
			if (i > j)
				strcpy(templineC+j, templineC+i);
		} else {
			if (isSpName(templineC, i)) {
				if (templineC[i] == '\t')
					strcpy(templineC+i, templineC+i+1);
			} else if (templineC[i] != '\n' && templineC[i] != EOS) {
				if (!isSpace(templineC[i]))
					uS.shiftright(templineC+i, 1);
				if (templineC[i] == '\t' && isSpace(templineC[i+1]))
					strcpy(templineC+i, templineC+i+1);
				else
					templineC[i] = '\t';
			}
		}
		fputs(templineC, fpout);
	}
}
