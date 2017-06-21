/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main tierorder_main
#define call tierorder_call
#define getflag tierorder_getflag
#define init tierorder_init
#define usage tierorder_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#define ERROR_FILE "tierorder_errors.cut"
#define TIERSNAME "tierorder.cut"

#define TIERS struct tierOrderT
struct tierOrderT {
    char speaker[SPEAKERLEN];
    AttTYPE attSp[SPEAKERLEN];
    char line[UTTLINELEN+1];
    AttTYPE attLine[UTTLINELEN+1];
	TIERS *NextTier;
} ;

#define TIERORSER struct tiersOrder
struct tiersOrder  {
    char tier[25];
    TIERORSER *nextTierOrder;
} ;

extern struct tier *defheadtier;
extern char R7Slash;
extern char R7Tilda;
extern char R7Caret;
extern char R7Colon;
extern char isRemoveCAChar[];
extern char Parans;
extern char OverWriteFile;

static FNType tiername[FNSize];
static char tierTier[SPEAKERLEN];
static char isLeaveBullets;
static char isTierOderFound;
static char isCOptionUsed;
static char isSortHeaderTiers;
static char tierorder_FirstTime;
static TIERS *RootTier, *TierBody;
static TIERORSER *RootTiersOrder;
static FILE *errFp;

void usage() {
	printf("Usage: tierorder [b c q tS %s] filename(s)\n",mainflgs());
	puts("+b : Do not move bullets to a new speaker tier");
	printf("+cF: dictionary file. (Default %s)\n", TIERSNAME);
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	puts("+q : ONLY sort header tiers");
	puts("+tS: switch S tier with main speaker tier");
	mainusage(TRUE);
}

static TIERS *freeDepTiers(TIERS *p) {
	TIERS *t;

	while (p != NULL) {
		t = p;
		p = p->NextTier;
		free(t);
	}
	return(NULL);
}

static TIERORSER *freeTiersOrder(TIERORSER *p) {
	TIERORSER *t;

	while (p != NULL) {
		t = p;
		p = p->nextTierOrder;
		free(t);
	}
	return(NULL);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TIERORDER;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	errFp = NULL;
	bmain(argc,argv,NULL);
	RootTiersOrder = freeTiersOrder(RootTiersOrder);
	if (errFp != NULL && errFp != stderr) {
		fprintf(stderr, "\n\nErrors were detected and are stored in file \"%s\".\n", ERROR_FILE);
		fclose(errFp);
	}
}
		
void getflag(char *f, char *f1, int *i) {
	int len;

	f++;
	switch(*f++) {
		case 'b':
			isLeaveBullets = TRUE;
			no_arg_option(f);
			break;
		case 'c':
			if (*f) {
				uS.str2FNType(tiername, 0L, getfarg(f,f1,i));
				isCOptionUsed  = TRUE;
			}
			break;
#ifdef UNX
		case 'L':
			strcpy(lib_dir, f);
			len = strlen(lib_dir);
			if (len > 0 && lib_dir[len-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		case 'q':
			isSortHeaderTiers = TRUE;
			no_arg_option(f);
			break;
		case 't':
			if (*f != '%')
				strcpy(tierTier, "%");
			else
				tierTier[0] = EOS;
			strcat(tierTier, f);
			uS.lowercasestr(tierTier, &dFnt, C_MBF);
			len = strlen(tierTier);
			if (len > 0) {
				len--;
				if (tierTier[len] != ':')
					strcat(tierTier, ":");
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void OutputError(const char *mess) {
	if (errFp == NULL) {
		errFp = fopen(ERROR_FILE, "w");
		if (errFp == NULL)
			errFp = stderr;
		else
			fprintf(errFp, "%s\n", UTF8HEADER);
	}
	if (errFp == stderr)
		putc('\n', errFp);
	fputs("----------------------------------------\n", errFp);
	fprintf(errFp,"*** File \"%s\": line %ld.\n", newfname, lineno);
	fprintf(errFp,"Can't move %s on this line.\n", mess);
	fprintf(errFp,"Please move it by hand to tier \"%s\" in file \"%s\"\n", tierTier, newfname);
	if (errFp == stderr)
		putc('\n', errFp);
}

static TIERORSER * add2TierOrder(TIERORSER *root, char *tier) {
	TIERORSER *nt;


	nt = NEW(TIERORSER);
	if (nt == NULL) {
		RootTier = freeDepTiers(RootTier);
		TierBody = freeDepTiers(TierBody);
		RootTiersOrder = freeTiersOrder(RootTiersOrder);
		fprintf(stderr,"Out of memory.\n");
		cutt_exit(0);
	}
	nt->nextTierOrder = root;
	root = nt;
/*
	if (root == NULL) {
		root = NEW(TIERORSER);
		if (root == NULL) {
			RootTier = freeDepTiers(RootTier);
			TierBody = freeDepTiers(TierBody);
			RootTiersOrder = freeTiersOrder(RootTiersOrder);
			fprintf(stderr,"Out of memory.\n");
			cutt_exit(0);
		}
		nt = root;
	} else {
		for (nt = root; nt->nextTierOrder != NULL; nt = nt->nextTierOrder) ;
		nt->nextTierOrder = NEW(TIERORSER);
		if (nt->nextTierOrder == NULL) {
			RootTier = freeDepTiers(RootTier);
			TierBody = freeDepTiers(TierBody);
			RootTiersOrder = freeTiersOrder(RootTiersOrder);
			fprintf(stderr,"Out of memory.\n");
			cutt_exit(0);
		}
		nt = nt->nextTierOrder;
	}
	nt->nextTierOrder = NULL;
*/
	strcpy(nt->tier, tier);
	return(root);
}

static void readtiers(void) {
	FILE *fdic;
	register int t;
	FNType mFileName[FNSize];

	if (tiername[0] == EOS)
		uS.str2FNType(tiername, 0L, TIERSNAME);

	if ((fdic=OpenGenLib(tiername,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the tiers order files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", tiername, mFileName);
		if (isCOptionUsed)
			cutt_exit(0);
		else
			return;
	}

	isTierOderFound = TRUE;
	while (fgets_cr(templineC, 255, fdic)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE))
			continue;
		uS.remblanks(templineC);
		if (templineC[0] == EOS)
			continue;
		t = strlen(templineC) - 1;
		if (templineC[t] != ':')
			strcat(templineC, ":");
		RootTiersOrder = add2TierOrder(RootTiersOrder, templineC);
	}

	fclose(fdic);
}

void init(char f) {
	if (f) {
		RootTier = NULL;
		TierBody = NULL;
		RootTiersOrder = NULL;
		isTierOderFound = FALSE;
		isCOptionUsed = FALSE;
		isSortHeaderTiers = FALSE;
		*tiername = EOS;
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		tierorder_FirstTime = TRUE;
		OverWriteFile = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		R7Slash = FALSE;
		R7Tilda = FALSE;
		R7Caret = FALSE;
		R7Colon = FALSE;
		isRemoveCAChar[NOTCA_CROSSED_EQUAL] = FALSE;
		isRemoveCAChar[NOTCA_LEFT_ARROW_CIRCLE] = FALSE;
		Parans = 2;
		*tierTier = EOS;
		isLeaveBullets = FALSE;
	} else if (tierorder_FirstTime) {
		readtiers();
		tierorder_FirstTime = FALSE;
	}
}

static char isRightPlace(char *nextSp, char *thisSp) {
	char isNextSPFound, isCurrSPFound;
	TIERORSER *nt;

	if (nextSp[0] == '%')
		isNextSPFound = FALSE;
	else
		isNextSPFound = TRUE;
	isCurrSPFound = FALSE;
	if (uS.partcmp(thisSp, nextSp, FALSE, FALSE))
		return(TRUE);

	for (nt=RootTiersOrder; nt != NULL; nt=nt->nextTierOrder) {
		if (uS.partcmp(nextSp, nt->tier, FALSE, FALSE))
			isNextSPFound = TRUE;
		else if (uS.partcmp(thisSp, nt->tier, FALSE, FALSE)) {
			isCurrSPFound = TRUE;
			break;
		}
	}

	if (!isCurrSPFound) {
		RootTier = freeDepTiers(RootTier);
		TierBody = freeDepTiers(TierBody);
		RootTiersOrder = freeTiersOrder(RootTiersOrder);
		fprintf(stderr,"Tier \"%s\" is not specified in file: %s.\n", thisSp, tiername);
		cutt_exit(0);
	}

	if (!isNextSPFound)
		return(FALSE);
	else
		return(TRUE);
}

static TIERS *addAndSortDepTiers(TIERS *root, UTTER *utt) {
	TIERS *nt, *tnt, *t;

	if (root == NULL) {
		root = NEW(TIERS);
		if (root == NULL) {
			RootTier = freeDepTiers(RootTier);
			TierBody = freeDepTiers(TierBody);
			RootTiersOrder = freeTiersOrder(RootTiersOrder);
			fprintf(stderr,"Out of memory.\n");
			cutt_exit(0);
		}
		nt = root;
		nt->NextTier = NULL;
	} else {
		tnt= root;
		nt = root;
		while (1) {
			if (nt == NULL)
				break;
			else if (isTierOderFound) {
				if (isRightPlace(nt->speaker, utt->speaker))
					break;
			} else if (strcmp(nt->speaker, utt->speaker) > 0)
				break;
			tnt = nt;
			nt = nt->NextTier;
		}
		if (nt == NULL) {
			t = tnt->NextTier;
			tnt->NextTier = NEW(TIERS);
			if (tnt->NextTier == NULL) {
				tnt->NextTier = t;
				RootTier = freeDepTiers(RootTier);
				TierBody = freeDepTiers(TierBody);
				RootTiersOrder = freeTiersOrder(RootTiersOrder);
				fprintf(stderr,"Out of memory.\n");
				cutt_exit(0);
			}
			nt = tnt->NextTier;
			nt->NextTier = NULL;
		} else if (nt == root) {
			root = NEW(TIERS);
			if (root == NULL) {
				RootTier = freeDepTiers(RootTier);
				TierBody = freeDepTiers(TierBody);
				RootTiersOrder = freeTiersOrder(RootTiersOrder);
				fprintf(stderr,"Out of memory.\n");
				cutt_exit(0);
			}
			root->NextTier = nt;
			nt = root;
		} else {
			nt = NEW(TIERS);
			if (nt == NULL) {
				RootTier = freeDepTiers(RootTier);
				TierBody = freeDepTiers(TierBody);
				RootTiersOrder = freeTiersOrder(RootTiersOrder);
				fprintf(stderr,"Out of memory.\n");
				cutt_exit(0);
			}
			nt->NextTier = tnt->NextTier;
			tnt->NextTier = nt;
		}
	}
	
	att_cp(0, nt->speaker, utt->speaker, nt->attSp, utt->attSp);
	att_cp(0, nt->line, utt->line, nt->attLine, utt->attLine);
	return(root);
}

static TIERS *addHeadTiers(TIERS *root, UTTER *utt) {
	TIERS *t;

	if (root == NULL) {
		root = NEW(TIERS);
		t = root;
	} else {
		for (t=root; t->NextTier != NULL; t=t->NextTier) ;
		t->NextTier = NEW(TIERS);
		t = t->NextTier;
	}
	if (t == NULL) {
		RootTier = freeDepTiers(RootTier);
		TierBody = freeDepTiers(TierBody);
		RootTiersOrder = freeTiersOrder(RootTiersOrder);
		fprintf(stderr,"Out of memory.\n");
		cutt_exit(0);
	}
	t->NextTier = NULL;
	att_cp(0, t->speaker, utt->speaker, t->attSp, utt->attSp);
	att_cp(0, t->line, utt->line, t->attLine, utt->attLine);
	return(root);
}

static void PrintOutHeaderTier(TIERS *root) {
	TIERS *p;

	for (; root != NULL && !uS.partcmp(root->speaker, "@Begin", FALSE, FALSE) && 
		 !uS.partcmp(root->speaker, "@Languages", FALSE, FALSE) &&
		 !uS.partcmp(root->speaker, "@Participants", FALSE, FALSE) &&
		 !uS.partcmp(root->speaker, "@ID:", FALSE, FALSE); root=root->NextTier) {
		printout(root->speaker,root->line,root->attSp,root->attLine,FALSE);
		root->speaker[0] = EOS;
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "@Begin", FALSE, FALSE)) {
			if (p->speaker[0] != EOS)
				printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
			p->speaker[0] = EOS;
		}
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "@Languages", FALSE, FALSE)) {
			if (p->speaker[0] != EOS)
				printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
			p->speaker[0] = EOS;
		}
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "@Participants", FALSE, FALSE)) {
			if (p->speaker[0] != EOS)
				printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
			p->speaker[0] = EOS;
		}
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "@Options", FALSE, FALSE)) {
			if (p->speaker[0] != EOS)
				printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
			p->speaker[0] = EOS;
		}
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "@ID:", FALSE, FALSE)) {
			if (p->speaker[0] != EOS)
				printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
			p->speaker[0] = EOS;
		}
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "@Birth of", FALSE, FALSE)) {
			if (p->speaker[0] != EOS)
				printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
			p->speaker[0] = EOS;
		}
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "@Birthplace of", FALSE, FALSE)) {
			if (p->speaker[0] != EOS)
				printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
			p->speaker[0] = EOS;
		}
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "@L1 of", FALSE, FALSE)) {
			if (p->speaker[0] != EOS)
				printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
			p->speaker[0] = EOS;
		}
	}
	for (p=root; p != NULL; p=p->NextTier) {
		if (p->speaker[0] != EOS)
			printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
		p->speaker[0] = EOS;
	}
}

static void removeBullets(TIERS *p) {
	long b, e;

	b = 0L;
	while (p->line[b] != EOS) {
		if (p->line[b] == HIDEN_C) {
			for (e=b+1; p->line[e] != HIDEN_C && p->line[e] != EOS; e++) ;
			if (p->line[e] == HIDEN_C) {
				for (e++; isSpace(p->line[e]); e++) ;
				att_cp(0, p->line+b, p->line+e, p->attLine+b, p->attLine+e);
			} else
				b++;
		} else
			b++;
	}
}

static void PrintOutDepTier(char isDelBullets, char isReplacementTierFound, char *mainTier) {
	TIERS *p;

	if (tierTier[0] != EOS) {
		for (p=RootTier; p != NULL; p=p->NextTier) {
			if (uS.partcmp(p->speaker, tierTier, FALSE, TRUE)) {
				if (!isReplacementTierFound)
					strcpy(p->speaker, mainTier);
				else if (!isLeaveBullets && isDelBullets)
					removeBullets(p);
			}
		}
	}

	for (p=RootTier; p != NULL; p=p->NextTier) {
		if (p->speaker[0] == '*')
			printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
	}
	for (p=RootTier; p != NULL; p=p->NextTier) {
		if (p->speaker[0] != '*')
			printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
	}
	RootTier = freeDepTiers(RootTier);
	TierBody = freeDepTiers(TierBody);
}

static int IsUtteranceDelChar(char *line, int pos) {
	if (uS.IsUtteranceDel(line, pos))
		return(TRUE);

	if (uS.isRightChar(line, pos, '<', &dFnt, MBF) || uS.isRightChar(line, pos, ',', &dFnt, MBF) ||
		uS.isRightChar(line, pos, '/', &dFnt, MBF) || uS.isRightChar(line, pos, '^', &dFnt, MBF) ||
		uS.isRightChar(line, pos, '=', &dFnt, MBF) || uS.isRightChar(line, pos, '+', &dFnt, MBF) ||
		uS.isRightChar(line, pos, '"', &dFnt, MBF))
		return(TRUE);
	return(FALSE);
}

static int IsEndUtteranceDelChar(char *line, int pos) {
	if (uS.IsUtteranceDel(line, pos))
		return(TRUE);

	if ((uS.isRightChar(line, pos, '<', &dFnt, MBF) || uS.isRightChar(line, pos, ',', &dFnt, MBF) ||
		 uS.isRightChar(line, pos, '^', &dFnt, MBF) || uS.isRightChar(line, pos, '+', &dFnt, MBF) ||
		 uS.isRightChar(line, pos, '"', &dFnt, MBF)) && pos > 0 && uS.isRightChar(line, pos-1, '+', &dFnt, MBF) &&
		(isSpace(line[pos+1]) || line[pos+1] == '\n' || line[pos+1] == EOS))
		return(TRUE);
	return(FALSE);
}

static void addItem2UttLoc(UTTER *utt, long p, char *line, long b, long e) {
	char isAddSpaceBef, isAddSpaceAft;

	isAddSpaceBef = isAddSpaceAft = FALSE;
	if (p > 0 && !isSpace(utt->line[p-1]))
		isAddSpaceBef = TRUE;
	if (!isSpace(utt->line[p]) && utt->line[p] != '\n' && utt->line[p] != EOS)
		isAddSpaceAft = TRUE;
	if (isAddSpaceBef && isAddSpaceAft)
		att_shiftright(utt->line+p, utt->attLine+p, e-b+3);
	else if (isAddSpaceBef || isAddSpaceAft)
		att_shiftright(utt->line+p, utt->attLine+p, e-b+2);
	else
		att_shiftright(utt->line+p, utt->attLine+p, e-b+1);
	if (isAddSpaceBef)
		utt->line[p++] = ' ';
	for (; b <= e; b++)
		utt->line[p++] = line[b];
	if (isAddSpaceAft)
		utt->line[p++] = ' ';
}

static void changeUtteranceDelimiter(UTTER *utt, char *line, long b, long e) {
	long ob, oe;

	for (oe=strlen(utt->line)-1L; oe >= 0; oe--) {
		if (IsEndUtteranceDelChar(utt->line, oe)) {
			for (ob=oe-1; IsUtteranceDelChar(utt->line, ob) && ob >= 0; ob--) ;
			ob++;
			if (uS.IsUtteranceDel(utt->line, ob) || uS.isRightChar(utt->line, ob, '+', &dFnt, MBF)) {
				oe++;
				att_cp(0, utt->line+ob, utt->line+oe, utt->attLine+ob, utt->attLine+oe);
				addItem2UttLoc(utt, ob, line, b, e);
				return;
			}
		}
	}
	for (oe=strlen(utt->line)-1L; (isSpace(utt->line[oe]) || utt->line[oe] == '\n') && oe >= 0; oe--) ;
	oe++;
	addItem2UttLoc(utt, oe, line, b, e);
}

static void addPostcode2Utterance(UTTER *utt, int CRsFound, char *line, long b, long e) {
	long p;

	for (p=strlen(utt->line)-1L; p >= 0; p--) {
		if (utt->line[p] == '\n') {
			CRsFound--;
			if (CRsFound == 0) {
				addItem2UttLoc(utt, p, line, b, e);
				return;
			}
		}
	}
	if (CRsFound > 0)
		OutputError("postcode");
}

static char addBullet2Utterance(UTTER *utt, int CRsFound, char *line, long b, long e) {
	long p;

	for (p=strlen(utt->line)-1L; p >= 0; p--) {
		if (utt->line[p] == '\n') {
			CRsFound--;
			if (CRsFound == 0) {
				addItem2UttLoc(utt, p, line, b, e);
				return(TRUE);
			}
		}
	}
	if (p < 0)
		p++;
	addItem2UttLoc(utt, p, line, b, e);
	OutputError("bullet");
	return(FALSE);
}

static void addAllItems(UTTER *utt, TIERS *TB, char *isDelBullets) {
	int  CRsFound;
	long b, e;
	char isCRFound, isUttDelFound;

	CRsFound = 0;
	isCRFound = FALSE;
	isUttDelFound = FALSE;
	for (e=strlen(TB->line)-1L; e > 0; e--) {
		if (TB->line[e] == '\n') {
			if (!isCRFound)
				CRsFound++;
			isCRFound = TRUE;
		} else if (uS.isRightChar(TB->line, e, ']', &dFnt, MBF)) {
			for (b=e-1; TB->line[b] != '[' && b > 0; b--) ;
			if (TB->line[b] == '[') {
				if (uS.isRightChar(TB->line, b+1, '+', &dFnt, MBF))
					addPostcode2Utterance(utt, CRsFound, TB->line, b, e);
				e = b;
			}
		} else if (IsEndUtteranceDelChar(TB->line, e) && !isUttDelFound) {
			for (b=e-1; IsUtteranceDelChar(TB->line, b) && b >= 0; b--) ;
			b++;
			if (uS.IsUtteranceDel(TB->line, b) || uS.isRightChar(TB->line, b, '+', &dFnt, MBF)) {
				changeUtteranceDelimiter(utt, TB->line, b, e);
				e = b;
				isUttDelFound = TRUE;
			}
		} else if (TB->line[e] == HIDEN_C && !isLeaveBullets) {
			for (b=e-1; TB->line[b] != HIDEN_C && b > 0; b--) ;
			if (b >= 0 && TB->line[b] == HIDEN_C) {
				if (!isCRFound)
					OutputError("bullet");
				else {
					if (!addBullet2Utterance(utt, CRsFound, TB->line, b, e))
						*isDelBullets = FALSE;
				}
				e = b;
			}
		} else if (!isSpace(TB->line[e]))
			isCRFound = FALSE;
	}
}

void call() {
	char isDelBullets, isReplacementTierFound, mainTier[SPEAKERLEN], isEndHeaders;

	isEndHeaders = FALSE;
	isDelBullets = FALSE;
	isReplacementTierFound = FALSE;
	mainTier[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (lineno % 200 == 0)
			fprintf(stderr,"\r%ld ", lineno);

		if (isSortHeaderTiers) {
			if (*utterance->speaker == '@' && !isEndHeaders) {
				TierBody = addHeadTiers(TierBody, utterance);
			} else {
				PrintOutHeaderTier(TierBody);
				TierBody = freeDepTiers(TierBody);
				isEndHeaders = TRUE;
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			}
		} else if (*utterance->speaker == '@') {
			PrintOutDepTier(isDelBullets, isReplacementTierFound, mainTier);
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			if (*utterance->speaker == '*')
				PrintOutDepTier(isDelBullets, isReplacementTierFound, mainTier);
			if (tierTier[0] != EOS) {
				if (*utterance->speaker == '*') {
					isReplacementTierFound = FALSE;
					TierBody = addAndSortDepTiers(TierBody, utterance);
					if (!isLeaveBullets)
						isDelBullets = TRUE;
					strcpy(mainTier, utterance->speaker);
					strcpy(utterance->speaker, tierTier);
				} else if (uS.partcmp(utterance->speaker, tierTier, FALSE, TRUE)) {
					isReplacementTierFound = TRUE;
					strcpy(utterance->speaker, mainTier);
					addAllItems(utterance, TierBody, &isDelBullets);
				}
			}
			RootTier = addAndSortDepTiers(RootTier, utterance);
		}
	}
	if (!isSortHeaderTiers)
		PrintOutDepTier(isDelBullets, isReplacementTierFound, mainTier);
	fprintf(stderr, "\r	     \r");
}
