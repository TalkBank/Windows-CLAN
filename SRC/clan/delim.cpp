/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3

#include "cu.h"

#if !defined(UNX)
#define _main delim_main
#define call delim_call
#define getflag delim_getflag
#define init delim_init
#define usage delim_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *headtier;
extern char OverWriteFile;

static const char *defDelim;


void init(char f) {

    if (f) {
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		onlydata = 1;
		OverWriteFile = TRUE;
		defDelim = ".";
		maketierchoice("*:", '+', '\001');
    }
}

void usage()			/* print proper usage and exit */
{
    printf("Usage: delim [+a %s] filename(s)\n",mainflgs());
	puts("+a : change default delimiter from \".\" to \"++.\"");
    mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    chatmode = CHAT_MODE;
    CLAN_PROG_NUM = DELIM;
    OnlydataLimit = 0;
    UttlineEqUtterance = TRUE;
    bmain(argc,argv,NULL);
}
	
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'a':
			defDelim = "++.";
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char isPostCode(char *uttline, int *i) {
	int t = *i;

	if (uttline[t] != ']')
		return(FALSE);
	for (t--; uttline[t] != '[' && t >= 0; t--) ;
	if (t < 0)
		return(FALSE);
	if (uttline[t+1] == '+' && uttline[t+2] == ' ') {
		*i = t - 1;
		return(TRUE);
	}
	return(FALSE);
}

static void delim_anal(void) {
    register int i, j;
    char sb = FALSE, hid = FALSE;

    i = strlen(uttline) - 1;
    while (i >= 0) {
		if (!isSpace(uttline[i]) && uttline[i] != '\n') {
		    if (uttline[i] == HIDEN_C)
		    	hid = !hid;
		    else if (uttline[i] == ']')
		    	sb = TRUE;
		    else if (uttline[i] == '[')
		    	sb = FALSE;
		    else if (!sb && !hid) {
				if (!uS.IsUtteranceDel(uttline, i)) {
				    if (uttline[i] == '-' && uttline[i+1] == ' ')
				    	uttline[i+1] = '.';
				    else {
						i = strlen(uttline) - 1;
						do {
							for (; i >= 0 && (isSpace(uttline[i]) || uttline[i] == '\n'); i--) ;
							if ( i < 0)
								break;
							if (uttline[i] == HIDEN_C) {
								for (i--; uttline[i] != HIDEN_C && i >= 0; i--) ;
								if (i >= 0)
									i--;
							} else if (isPostCode(uttline, &i))
								;
							else
								break;
						} while (i >= 0) ;
						i++;
						att_shiftright(uttline+i, utterance->attLine+i, strlen(defDelim)+1);
						uttline[i++] = ' ';
						for (j=0; defDelim[j] != EOS; j++)
							uttline[i++] = defDelim[j];
				    }
				}
				break;
		    }
		}
		i--;
    }
    if (i < 0) {
		i++;
		att_shiftright(uttline+i, utterance->attLine+i, 3);
		uttline[i++] = '0';
		uttline[i++] = '.';
		uttline[i++] = ' ';
    }
    for (i=0; uttline[i] == ' ' || uttline[i] == '\t'; i++) ;
    if (i > 0)
    	strcpy(uttline, uttline+i);
    if (*uttline == EOS)
    	strcpy(uttline, " 0.");
}

void call() {
	if (headtier == NULL) {
		fprintf(stderr, "    Please specify tier(s) using \"+t\" option\n");
 		cutt_exit(0);
	} else if (headtier->nexttier == NULL && 
		headtier->tcode[0] == '*' && headtier->tcode[1] == ':') {
		fprintf(stderr, "    Please specify tier(s) using \"+t\" option\n");
		cutt_exit(0);
    }
    
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (checktier(utterance->speaker)) {
			delim_anal();
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
}
