/**********************************************************************
	"Copyright 1990-2023 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *headtier;
extern char OverWriteFile;

#define PERSUADE_FIX_UTT struct persuade_fix_utterance /* main utterance structure             */
PERSUADE_FIX_UTT {
	char speaker[SPEAKERLEN];		/* code descriptor field of the turn	*/
	AttTYPE attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
	char line[UTTLINELEN+1];		/* text field of the turn		*/ // found uttlinelen
	AttTYPE attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
	PERSUADE_FIX_UTT *nextutt;
} ;

static PERSUADE_FIX_UTT *persuade_fix_freeUtts(PERSUADE_FIX_UTT *p) {
	PERSUADE_FIX_UTT *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextutt;
		free(t);
	}
	return(NULL);
}

static PERSUADE_FIX_UTT *persuade_fix_add2Utts(PERSUADE_FIX_UTT *root_utts) {
	PERSUADE_FIX_UTT *utt;
	
	if (root_utts == NULL) {
		utt = NEW(PERSUADE_FIX_UTT);
		if (utt == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = persuade_fix_freeUtts(root_utts);
			cutt_exit(0);
		}
		root_utts = utt;
	} else {
		for (utt=root_utts; utt->nextutt != NULL; utt=utt->nextutt) ;
		if ((utt->nextutt=NEW(PERSUADE_FIX_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = persuade_fix_freeUtts(root_utts);
			cutt_exit(0);
		}
		utt = utt->nextutt;
	}
	utt->nextutt = NULL;
	att_cp(0L, utt->speaker, utterance->speaker, utt->attSp, utterance->attSp);
	att_cp(0L, utt->line, utterance->line, utt->attLine, utterance->attLine);
	return(root_utts);
}

static void persuade_fix_outputUtts(PERSUADE_FIX_UTT *root_utts) {
	int  i, j, cnt;
	char isEll, isDis, isFoundNot;
	PERSUADE_FIX_UTT *utt;

	isEll = FALSE;
	isDis = FALSE;
	for (utt=root_utts; utt != NULL; utt=utt->nextutt) {
		if (uS.partcmp(utt->speaker, "@Comment:", FALSE, FALSE)) {
			if (uS.partcmp(utt->line, "ell_status", FALSE, FALSE)) {
				for (i=10; utt->line[i] != EOS; i++) {
					if (uS.mStrnicmp(utt->line+i, "Yes", 3) == 0) {
						isEll = TRUE;
						break;
					}
				}
			} else if (uS.partcmp(utt->line, "student_disability_status", FALSE, FALSE)) {
				isFoundNot = FALSE;
				for (i=10; utt->line[i] != EOS; i++) {
					if (uS.mStrnicmp(utt->line+i, "Not", 3) == 0) {
						isFoundNot = TRUE;
						break;
					}
				}
				if (isFoundNot == FALSE)
					isDis = TRUE;
			}
		}
	}
	for (utt=root_utts; utt != NULL; utt=utt->nextutt) {
		if (uS.partcmp(utt->speaker, "@ID:", FALSE, FALSE)) {
			if (isEll == TRUE || isDis == TRUE) {
				cnt = 0;
				for (i=0; utt->line[i] != EOS; i++) {
					if (utt->line[i] == '|') {
						cnt++;
						if (cnt == 5) {
							i++;
							strcpy(utt->line+i, utt->line+i+2);
							j = i;
							if (isEll == TRUE) {
								uS.shiftright(utt->line+j, 3);
								utt->line[j++] = 'E';
								utt->line[j++] = 'L';
								utt->line[j++] = 'L';
							}
							if (isDis == TRUE) {
								if (utt->line[j-1] == 'L') {
									uS.shiftright(utt->line+j, 4);
									utt->line[j++] = ',';
								} else {
									uS.shiftright(utt->line+j, 3);
								}
								utt->line[j++] = 'D';
								utt->line[j++] = 'I';
								utt->line[j++] = 'S';
							}
						}
					}
				}
			}
		}
		printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
	}
}

void init(char f) {

    if (f) {
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		onlydata = 1;
		OverWriteFile = TRUE;
		maketierchoice("*:", '+', '\001');
    }
}

void usage()			/* print proper usage and exit */
{
	puts("TEMP adds info about ell_status and disability_status to @ID header.");
    printf("Usage: temp [%s] filename(s)\n",mainflgs());
    mainusage(TRUE);
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

void call() {    
	PERSUADE_FIX_UTT *root_utts;

	root_utts = NULL;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		root_utts = persuade_fix_add2Utts(root_utts);
		if (utterance->speaker[0] == '*')
			break;
	}
	if (root_utts != NULL) {
		persuade_fix_outputUtts(root_utts);
		root_utts = persuade_fix_freeUtts(root_utts);
	}
	while (getwholeutter()) {
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
}
