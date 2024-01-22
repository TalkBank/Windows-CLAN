/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
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

#define AGE_FIX_UTT struct age_fix_utterance /* main utterance structure             */
AGE_FIX_UTT {
	char speaker[SPEAKERLEN];		/* code descriptor field of the turn	*/
	AttTYPE attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
	char line[UTTLINELEN+1];		/* text field of the turn		*/ // found uttlinelen
	AttTYPE attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
	AGE_FIX_UTT *nextutt;
} ;

static AGE_FIX_UTT *age_fix_freeUtts(AGE_FIX_UTT *p) {
	AGE_FIX_UTT *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextutt;
		free(t);
	}
	return(NULL);
}

static AGE_FIX_UTT *age_fix_add2Utts(AGE_FIX_UTT *root_utts) {
	AGE_FIX_UTT *utt;
	
	if (root_utts == NULL) {
		utt = NEW(AGE_FIX_UTT);
		if (utt == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = age_fix_freeUtts(root_utts);
			cutt_exit(0);
		}
		root_utts = utt;
	} else {
		for (utt=root_utts; utt->nextutt != NULL; utt=utt->nextutt) ;
		if ((utt->nextutt=NEW(AGE_FIX_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = age_fix_freeUtts(root_utts);
			cutt_exit(0);
		}
		utt = utt->nextutt;
	}
	utt->nextutt = NULL;
	att_cp(0L, utt->speaker, utterance->speaker, utt->attSp, utterance->attSp);
	att_cp(0L, utt->line, utterance->line, utt->attLine, utterance->attLine);
	return(root_utts);
}

static void age_fix_outputUtts(AGE_FIX_UTT *root_utts) {
	int  i, j, cnt;
	char isEll, isDis, isFoundNot;
	AGE_FIX_UTT *utt;

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
    }
}

void usage()			/* print proper usage and exit */
{
	puts("TEMP copy @ID header from orig file.");
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

static char readIDFromOrig(FNType* oldfname) {
	int  len;
	char isIDsFound;
	FNType origFname[FNSize];
	FILE *fp;

	strcpy(origFname, wd_dir);
	len = strlen(origFname);
	if (origFname[len-1] == '/')
		origFname[len-1] = EOS;
	strcat(origFname, "-orig");
	strcat(origFname, oldfname+len);
	
	if ((fp=fopen(origFname,"r")) == NULL) {
		fprintf(stderr, "***ERROR: Can't open orig file:\n\t%s\n", origFname);
		return(FALSE);
	}
	fprintf(stderr, "Reading orig file: %s\n", origFname);
	isIDsFound = FALSE;
	while (fgets_cr(spareTier1, UTTLINELEN, fp)) {
		if (uS.partcmp(spareTier1,"@ID:",FALSE,FALSE)) {
			fprintf(fpout, "%s", spareTier1);
			isIDsFound = TRUE;
		} else if (isIDsFound == TRUE)
			break;
	}
	fclose(fp);
	if (isIDsFound == FALSE) {
		fprintf(stderr, "***ERROR: Can't find any IDs in orig file\n");
		return(FALSE);
	} else
		return(TRUE);
}

void call() {   
	char isReadOrigIDs, isWroteNewIDs;

	isWroteNewIDs = TRUE;
	isReadOrigIDs = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
			if (isReadOrigIDs == FALSE) {
				isReadOrigIDs = TRUE;
				if (readIDFromOrig(oldfname) == FALSE)
					isWroteNewIDs = FALSE;
			}
			if (isWroteNewIDs == FALSE)
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
}
