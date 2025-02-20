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

#define GEM_FIX_UTT struct GEM_FIX_utterance /* main utterance structure             */
GEM_FIX_UTT {
	char speaker[SPEAKERLEN];		/* code descriptor field of the turn	*/
	AttTYPE attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
	char line[UTTLINELEN+1];		/* text field of the turn		*/ // found uttlinelen
	AttTYPE attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
	GEM_FIX_UTT *nextutt;
} ;

GEM_FIX_UTT *root_utts;

static GEM_FIX_UTT *GEM_FIX_freeUtts(GEM_FIX_UTT *p) {
	GEM_FIX_UTT *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextutt;
		free(t);
	}
	return(NULL);
}

static GEM_FIX_UTT *GEM_FIX_add2Utts(GEM_FIX_UTT *root) {
	GEM_FIX_UTT *utt;
	
	if (root == NULL) {
		utt = NEW(GEM_FIX_UTT);
		if (utt == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root = GEM_FIX_freeUtts(root);
			cutt_exit(0);
		}
		root = utt;
	} else {
		for (utt=root; utt->nextutt != NULL; utt=utt->nextutt) ;
		if ((utt->nextutt=NEW(GEM_FIX_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root = GEM_FIX_freeUtts(root);
			cutt_exit(0);
		}
		utt = utt->nextutt;
	}
	utt->nextutt = NULL;
	att_cp(0L, utt->speaker, utterance->speaker, utt->attSp, utterance->attSp);
	att_cp(0L, utt->line, utterance->line, utt->attLine, utterance->attLine);
	return(root);
}

static void GEM_FIX_outputUtts(GEM_FIX_UTT *root) {
	GEM_FIX_UTT *utt;

	for (utt=root; utt != NULL; utt=utt->nextutt) {
		printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
	}
}

void init(char f) {
    if (f) {
		root_utts = NULL;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		onlydata = 1;
		OverWriteFile = TRUE;
	} else {
	}
}

void usage()			/* print proper usage and exit */
{
	puts("TEMP flip GEM A with GEM B.");
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
	char isAGEM;

	isAGEM = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
			if (utterance->line[0] == 'A')
				isAGEM = TRUE;
			else if (utterance->line[0] == 'B')
				isAGEM = FALSE;
		}
		if (uS.partcmp(utterance->speaker,"@End:",FALSE,FALSE)) {
			GEM_FIX_outputUtts(root_utts);
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else if (isAGEM == TRUE) {
			if (uS.partcmp(utterance->speaker,"@Types:",FALSE,FALSE))
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			else
				root_utts = GEM_FIX_add2Utts(root_utts);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
	root_utts = GEM_FIX_freeUtts(root_utts);
}
