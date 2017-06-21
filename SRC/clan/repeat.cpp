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
#define _main repeat_main
#define call repeat_call
#define getflag repeat_getflag
#define init repeat_init
#define usage repeat_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

extern struct tier *defheadtier;
extern long option_flags[];

char tsp[SPEAKERLEN];

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void usage() {
	printf("Usage: repeat [t %s] filename(s)\n",mainflgs());
	puts("+tS: specify target speaker to be S");
	mainusage(TRUE);
}

void init(char f) {
	int i;

	if (f) {
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+<\\% bch>");
		addword('\0','\0',"+uh");
		addword('\0','\0',"+um");
		addword('\0','\0',"+:*");
		addword('\0','\0',"+$*");
		maininitwords();
		mor_initwords();
		*tsp = EOS;
		stout = FALSE;
		onlydata = 1;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		option_flags[REPEAT]	= F_OPTION+K_OPTION+P_OPTION;
	} else {
		if (*tsp == EOS) {
			fprintf(stderr,"User +t option to specify target speaker\n");
			cutt_exit(0);
		}
		*spareTier1 = EOS;
		for (i=0; GlobalPunctuation[i]; ) {
			if (GlobalPunctuation[i] == '!' ||
				GlobalPunctuation[i] == '?' ||
				GlobalPunctuation[i] == '.') 
				strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
			else
				i++;
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = REPEAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 't':
			if (*tsp != EOS) {
				fprintf(stderr,"Please specify only one target speaker\n");
				cutt_exit(0);
			} else
				if (*f != '*' && *f != '@' && *f != '%')
					strcpy(tsp, "*");
				strcat(tsp, f);
				uS.uppercasestr(tsp, &dFnt, MBF);
				uS.remblanks(tsp);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void CleanUpLine(char *st) {
	int pos;

	for (pos=0; st[pos] != EOS; ) {
		if (uS.isskip(st, pos, &dFnt, MBF))
			strcpy(st+pos, st+pos+1);
		else
			pos++;
	}
}

void call() {
	char hasChanged;

	*spareTier1 = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		hasChanged = FALSE;
		if (utterance->speaker[0] == '*') {
			strcpy(templineC3, utterance->speaker);
			uS.uppercasestr(templineC3, &dFnt, MBF);
			uS.remblanks(templineC3);
			strcpy(templineC4, uttline);
			CleanUpLine(templineC4);
			if (uS.partcmp(templineC3, tsp, FALSE, FALSE)) {
				if (strcmp(spareTier1, templineC4) == 0) {
					att_cp(strlen(utterance->line), utterance->line, " [+ rep]", utterance->attLine, NULL);
					hasChanged = TRUE;
				}
			}
			strcpy(spareTier1, templineC4);
		}
		printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, hasChanged);
	}
}
