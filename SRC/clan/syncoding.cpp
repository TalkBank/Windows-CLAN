/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#include "cu.h"
#include "check.h"
#ifndef UNX
#include "ced.h"
#else
#define RGBColor int
#include "c_curses.h"
#endif
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main syncoding_main
#define call syncoding_call
#define getflag syncoding_getflag
#define init syncoding_init
#define usage syncoding_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define to_1_BIT1(x)		(AttTYPE)(x | 1)
#define is_BIT1(x)			(AttTYPE)(x & 1)
#define to_0_BIT1(x)		(AttTYPE)(x & 0xfffe)

#define to_1_BIT2(x)		(AttTYPE)(x | 2)
#define is_BIT2(x)			(AttTYPE)(x & 2)
#define to_0_BIT2(x)		(AttTYPE)(x & 0xfffd)

#define to_1_BIT3(x)		(AttTYPE)(x | 4)
#define is_BIT3(x)			(AttTYPE)(x & 4)
#define to_0_BIT3(x)		(AttTYPE)(x & 0xfffb)

#define to_1_BIT4(x)		(AttTYPE)(x | 8)
#define is_BIT4(x)			(AttTYPE)(x & 8)
#define to_0_BIT4(x)		(AttTYPE)(x & 0xfff7)

#define to_1_BIT5(x)		(AttTYPE)(x | 16)
#define is_BIT5(x)			(AttTYPE)(x & 16)
#define to_0_BIT5(x)		(AttTYPE)(x & 0xffef)

#define to_1_BIT6(x)		(AttTYPE)(x | 32)
#define is_BIT6(x)			(AttTYPE)(x & 32)
#define to_0_BIT6(x)		(AttTYPE)(x & 0xffdf)

#define to_1_BIT7(x)		(AttTYPE)(x | 64)
#define is_BIT7(x)			(AttTYPE)(x & 64)
#define to_0_BIT7(x)		(AttTYPE)(x & 0xffbf)

#define MORTIERFEATS struct syncoding_morfeats /* main utterance structure             */
MORTIERFEATS {
	char *word;
	char *tword;
    MORFEATS *feats;
    MORTIERFEATS *nextfeats;
} ;
#define SYNUTT struct syncoding_utterance /* main utterance structure             */
SYNUTT {
    char speaker[SPEAKERLEN];		/* code descriptor field of the turn	*/
    AttTYPE attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
    char line[UTTLINELEN+1];		/* text field of the turn		*/ // found uttlinelen
    AttTYPE attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
	MORTIERFEATS *root_feats;
    SYNUTT *nextutt;
} ;

extern char OverWriteFile;

static int fileID;
static char syncoding_ftime;

void init(char f) {
	int i;

	if (f) {
		fileID = 0;
		syncoding_ftime = TRUE;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		onlydata = 1;
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+(*.*)");
		addword('\0','\0',"+<\">");
		mor_initwords();
	} else {
		if (syncoding_ftime) {
			syncoding_ftime = FALSE;
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
}

void usage() {
	printf("Creates coding tier %%syn: with Syntactic codings derived from existing data\n");
	printf("Usage: syncoding [%s] filename(s)\n", mainflgs());
	mainusage(TRUE);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void freeUpAllFeats(MORFEATS *p) {
	MORFEATS *t;

	while (p != NULL) {
		t = p;
		p = p->comp;
		if (t->free_prefix)
			free(t->prefix);
		if (t->free_suffix0)
			free(t->suffix0);
		if (t->free_suffix1)
			free(t->suffix1);
		if (t->free_suffix2)
			free(t->suffix2);
		if (t->free_fusion0)
			free(t->fusion0);
		if (t->free_fusion1)
			free(t->fusion1);
		if (t->free_fusion2)
			free(t->fusion2);
		if (t->free_trans)
			free(t->trans);
		if (t->free_repls)
			free(t->repls);
		if (t->free_error0)
			free(t->error0);
		if (t->free_error1)
			free(t->error1);
		if (t->free_error2)
			free(t->error2);
		free(t);
	}
}

static void freeRootFeats(MORTIERFEATS *p) {
	MORTIERFEATS *t;

	while (p != NULL) {
		t = p;
		p = p->nextfeats;
		if (t->word != NULL)
			free(t->word);
		if (t->tword != NULL)
			free(t->tword);
		freeUpAllFeats(t->feats);
		free(t);
	}
}

static SYNUTT *freeUtts(SYNUTT *p) {
	SYNUTT *t;

	while (p != NULL) {
		t = p;
		p = p->nextutt;
		freeRootFeats(t->root_feats);
		free(t);
	}
	return(NULL);
}

static SYNUTT *add2Utts(SYNUTT *root_utts) {
	SYNUTT *utt;

	if (root_utts == NULL) {
		utt = NEW(SYNUTT);
		if (utt == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = freeUtts(root_utts);
			cutt_exit(0);
		}
		root_utts = utt;
	} else {
		for (utt=root_utts; utt->nextutt != NULL; utt=utt->nextutt) ;
		if ((utt->nextutt=NEW(SYNUTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = freeUtts(root_utts);
			cutt_exit(0);
		}
		utt = utt->nextutt;
	}
	utt->nextutt = NULL;
	utt->root_feats = NULL;
	att_cp(0L, utt->speaker, utterance->speaker, utt->attSp, utterance->attSp);
	att_cp(0L, utt->line, utterance->line, utt->attLine, utterance->attLine);
	return(root_utts);
}

static MORTIERFEATS *addFeats(MORTIERFEATS *root_feats, char *word, SYNUTT *root_utts) {
	MORTIERFEATS *cfeats;

	if (root_feats == NULL) {
		cfeats = NEW(MORTIERFEATS);
		if (cfeats == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = freeUtts(root_utts);
			cutt_exit(0);
		}
		root_feats = cfeats;
	} else {
		for (cfeats=root_feats; cfeats->nextfeats != NULL; cfeats=cfeats->nextfeats) ;
		if ((cfeats->nextfeats=NEW(MORTIERFEATS)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = freeUtts(root_utts);
			cutt_exit(0);
		}
		cfeats = cfeats->nextfeats;
	}
	cfeats->nextfeats = NULL;
	cfeats->word = NULL;
	if ((cfeats->feats=NEW(MORFEATS)) == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		root_utts = freeUtts(root_utts);
		cutt_exit(0);
	}
	if ((cfeats->word=(char *)malloc(strlen(word)+1)) == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		root_utts = freeUtts(root_utts);
		cutt_exit(0);
	}
	strcpy(cfeats->word, word);
	if ((cfeats->tword=(char *)malloc(strlen(word)+1)) == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		root_utts = freeUtts(root_utts);
		cutt_exit(0);
	}
	strcpy(cfeats->tword, word);
	cfeats->feats->type = NULL;
	cfeats->feats->typeID = 'R';
	if (ParseWordIntoFeatures(cfeats->tword, cfeats->feats) == FALSE) {
		fprintf(stderr,"Error: out of memory\n");
		root_utts = freeUtts(root_utts);
		cutt_exit(0);
	}
	return(root_feats);
}

static void convertMor2Feats(SYNUTT *morUtt, SYNUTT *root_utts) {
	int  i;
	char word[BUFSIZ];

	i = 0;
	while ((i=getword(morUtt->speaker, morUtt->line, word, NULL, i))) {
		uS.remblanks(word);
		if (word[0] != '[' && strchr(word, '|') != NULL)
			morUtt->root_feats = addFeats(morUtt->root_feats, word, root_utts);
	}
}

static void outputUtts(SYNUTT *utt, char *synTier) {
	for (; utt != NULL; utt=utt->nextutt) {
		printout(utt->speaker, utt->line, utt->attSp, utt->attLine, FALSE);
	}
	uS.remblanks(synTier);
	if (synTier[0] != EOS)
		printout("%stx:", synTier, NULL, NULL, TRUE);
}

static void createSynCoding(SYNUTT *root_utts, char *synTier, char *tutt) {
	int  i;
	char word[BUFSIZ];
	AttTYPE isS, isSG, isSM, isNS, isUU, isAS;
//	AttTYPE isSS, isCS;
	SYNUTT *utt, *spUtt, *morUtt, *mgrsUtt;
	MORFEATS *feat;
	MORTIERFEATS *tierFeats;

	spUtt = NULL;
	morUtt = NULL;
	mgrsUtt = NULL;
	tutt[0] = EOS;
	for (utt=root_utts; utt != NULL; utt=utt->nextutt) {
		if (utt->speaker[0] == '*') {
			spUtt = utt;
			strcpy(tutt, spUtt->line);
			filterData(spUtt->speaker, tutt);
		} else if (uS.partcmp(utt->speaker, "%mor:", FALSE, FALSE)) {
			morUtt = utt;
			convertMor2Feats(morUtt, root_utts);
		} else if (uS.partcmp(utt->speaker, "%gra:", FALSE, FALSE))
			mgrsUtt = utt;
	}
	i = 0;
	isS = 0;
	isS = to_1_BIT1(isS);
	isSG = 0;
	isSM = 0;
	isNS = 0;
	isNS = to_1_BIT1(isNS);
	isNS = to_1_BIT2(isNS);
	isUU = 0;
//	isSS = 0;
//	isCS = 0;
	isAS = 0;
	while ((i=getword(spUtt->speaker, tutt, word, NULL, i))) {
		if (!strcmp(word, ".") || !strcmp(word, "?") || !strcmp(word, "!") || !strcmp(word, "+\"/.") || !strcmp(word, "+\"/?")) {
			isS = to_1_BIT2(isS);
			isSM = to_1_BIT2(isSM);
		}

		if (!strcmp(word, "+...") || !strcmp(word, "+..?") || !strcmp(word, "+/.") || !strcmp(word, "+//.") || !strcmp(word, "+/?") || !strcmp(word, "+//?")) {
			isSG = to_1_BIT2(isSG);
			isNS = to_0_BIT1(isNS);
		}

		if (!strcmp(word, "[+ gram]")) {
			isS = to_0_BIT1(isS);
			isSG = to_1_BIT2(isSG);
		}

		if (!strcmp(word, "[* agr]") || !strcmp(word, "[* pos]"))
			isSG = to_1_BIT2(isSG);

		if (!strcmp(word, "[* s]") || !strcmp(word, "[* per]") || !strcmp(word, "[* wu]") ||
			!strcmp(word, "[+ jar]") || uS.patmat(word, "*@n"))
			isSM = to_1_BIT1(isSM);

		if (!strcmp(word, "0art") || !strcmp(word, "0v") || !strcmp(word, "0aux") || !strcmp(word, "0subj") ||
			!strcmp(word, "0pobj"))
			isSG = to_1_BIT2(isSG);

		if (!strcmp(word, "xxx"))
			isUU = to_1_BIT1(isUU);
		else if (word[0] != '[' && word[0] != '+' && word[0] != '-' &&  word[0] != '0' &&
				 word[0] != '&' && word[0] != ',' && word[0] != '.' && word[0] != '!' && word[0] != '?')
			isUU = to_0_BIT1(isUU);
	}
	if (morUtt != NULL) {
		for (tierFeats=morUtt->root_feats; tierFeats != NULL; tierFeats=tierFeats->nextfeats) {
			for (feat=tierFeats->feats; feat != NULL; feat=feat->comp) {
				if (isAllv(feat) || isEqual("aux", feat->pos) || isEqual("cop", feat->pos)) {
					isS = to_1_BIT3(isS);
					isSG = to_1_BIT1(isSG);
					isNS = to_0_BIT2(isNS);
				}
				if ((isEqual("n", feat->pos) || isEqual("pro", feat->pos)) && !is_BIT3(isAS) && !is_BIT4(isAS))
					isAS = to_1_BIT2(isAS);
				else if ((isAllv(feat) || isEqual("aux", feat->pos)) && is_BIT2(isAS) && !is_BIT4(isAS))
					isAS = to_1_BIT3(isAS);
				else if ((isEqual("n", feat->pos) || isEqual("pro", feat->pos) || isEqual("n:prop", feat->pos)) && is_BIT2(isAS) && is_BIT3(isAS))
					isAS = to_1_BIT4(isAS);
			}
		}
	}
	if (mgrsUtt != NULL) {
	}
	if (is_BIT1(isS) && is_BIT2(isS) && is_BIT3(isS)) {
		strcat(synTier, "$s ");
		isAS = to_1_BIT1(isAS);
	}

	if (is_BIT1(isSG) && is_BIT2(isSG)) {
		strcat(synTier, "$*s:g ");
		isAS = to_1_BIT1(isAS);
	}

	if (is_BIT1(isSM) && is_BIT2(isSM)) {
		strcat(synTier, "$*s:m ");
		isAS = to_1_BIT1(isAS);
	}

	if (is_BIT1(isSG) && is_BIT2(isSG) && is_BIT1(isSM) && is_BIT2(isSM))
		strcat(synTier, "$*s:g:m ");

	if (is_BIT1(isNS) && is_BIT2(isNS))
		strcat(synTier, "$ns ");

	if (is_BIT1(isUU))
		strcat(synTier, "$uu ");

	if (is_BIT1(isAS) && is_BIT2(isAS) && is_BIT3(isAS) && is_BIT4(isAS))
		strcat(synTier, "$as ");

	outputUtts(root_utts, synTier);
}

/*
 char *type;		char typeID;
 char *prefix;	//	char mPrefix;  #
 char *pos;		//	char mPos;     |
 char *stem;	//	char mStem;    r
 char *suffix0;	//	char mSuffix0; -
 char *suffix1;	//	char mSuffix1; -
 char *suffix2;	//	char mSuffix2; -
 char *fusion0;	//	char mFusion0; &
 char *fusion1;	//	char mFusion1; &
 char *fusion2;	//	char mFusion2; &
 char *trans;   //	char mTrans;   =
 char *repls;   //	char mRepls;   @
 char *error;   //	char mError;   *
*/
void call() {
	char *synTier, *tutt;
	SYNUTT *root_utts;

	fileID++;
	root_utts = NULL;
	synTier = spareTier1;
	tutt = spareTier2;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			if (root_utts != NULL) {
				synTier[0] = EOS;
				createSynCoding(root_utts, synTier, tutt);
				root_utts = freeUtts(root_utts);
			}
			root_utts = add2Utts(root_utts);
		} else if (utterance->speaker[0] == '%') {
			root_utts = add2Utts(root_utts);
		} else {
			if (root_utts != NULL) {
				synTier[0] = EOS;
				createSynCoding(root_utts, synTier, tutt);
				root_utts = freeUtts(root_utts);
			}
			printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
		}
	}
	if (root_utts != NULL) {
		synTier[0] = EOS;
		createSynCoding(root_utts, synTier, tutt);
		root_utts = freeUtts(root_utts);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SYNCODING;
	OnlydataLimit = 1;
	UttlineEqUtterance = FALSE;
#ifdef _CLAN_DEBUG
	bmain(argc,argv,NULL);
#else
	fprintf(stderr, "\nThis program is not functional yet.\n\n");
#endif
}
