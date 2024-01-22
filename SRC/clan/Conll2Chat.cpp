/**********************************************************************
 "Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
*/

#define CHAT_MODE 0
#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main Conll2Chat_main
#define call Conll2Chat_call
#define getflag Conll2Chat_getflag
#define init Conll2Chat_init
#define usage Conll2Chat_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define WLEN 512

extern char OverWriteFile;
extern char isRemoveCAChar[];

struct ROOT_POS {
	char *oPOS;
	char *POS;
	int len;
	struct ROOT_POS *next;
} ;

struct ROOT_GR {
	char *oGR;
	char *GR;
	struct ROOT_GR *next;
} ;

struct ROOT_MOR {
	char *oMOR;
	char *MOR;
	struct ROOT_MOR *next;
} ;

#define CONLL_UTT struct conll_utts
struct conll_utts {
	int  cnum;
	int  pnum;
	char GRA[WLEN];
	char word[WLEN];
	char stem[WLEN];
	char POS[WLEN];
	char MOR[WLEN];
	char changed;
	CONLL_UTT *next;
} ; 

struct conll_errList {
	char POS[WLEN+1];
	struct conll_errList *left;
	struct conll_errList *right;
};

#define CNL2CH_UTT struct conll2chat_utts
CNL2CH_UTT {
	long lineno;
    char speaker[SPEAKERLEN];		/* code descriptor field of the turn	*/
    AttTYPE attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
    char line[UTTLINELEN+1];		/* text field of the turn		*/ // found uttlinelen
    AttTYPE attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
    char tuttline[UTTLINELEN+1];	/* working copy of the text of the turn	*/
    CNL2CH_UTT *next;
} ;

struct chat_errList {
	char *conll;
	char *chat;
	struct chat_errList *next;
};

static char ftime;
static char coding;
static char isPOSFileSpecified, isMORFileSpecified, isGRFileSpecified;
static char isJoinFiles, isMisMatch, isFloFound;
static const char *posFName, *grFName, *morFName;
static long lastMisMatchNo;
static CONLL_UTT *conll_root_utt;
static CNL2CH_UTT  *chat_root_utt;
static struct ROOT_POS *rootPOS;
static struct ROOT_GR *rootGR;
static struct ROOT_MOR *rootMOR;
struct conll_errList *posErrList;
struct conll_errList *grErrList;
struct conll_errList *morErrList;
struct chat_errList  *chatErrList;
static FILE *fpChat;

void usage() {
	printf("Usage: conll2chat [cS m %s] filename(s)\n",mainflgs());
	puts("+cS: use special coding system (default: Depparse - MaltParser, TurboParser)");
	puts("    +cDe(pparse), +cMa(ltParser), +cTu(rboParser)");
	puts("    +cAn(CoraCorpus), +cCo(nnexor), +cCl(earparser)");
	puts("    +cPR(OIEL)");
	puts("+j : join specified CONLL file(s) with corresponding original CHAT file(s)");
	fprintf(stdout, "+gF: specify GR translation file (default: %s)\n", grFName);
	fprintf(stdout, "+mF: specify additional MOR elements translation file (default: %s)\n", morFName);
	fprintf(stdout, "+pF: specify Part-of-Speech translation file (default: %s)\n", posFName);
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(FALSE);
	puts("\nExample for AnCoraCorpus:");
	puts("    conll2chat +can  filename");
	puts("For Connexor:");
	puts("    conll2chat +cco +m filename");
	puts("For Depparse, MaltParser or TurboParser:");
	puts("    conll2chat  filename");
	cutt_exit(0);
}

static struct ROOT_POS *freePOSes(struct ROOT_POS *p) {
	struct ROOT_POS *t;
	
	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->oPOS != NULL)
			free(t->oPOS);
		if (t->POS != NULL)
			free(t->POS);
		free(t);
	}
	return(NULL);
}

static struct ROOT_GR *freeGRs(struct ROOT_GR *p) {
	struct ROOT_GR *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->oGR != NULL)
			free(t->oGR);
		if (t->GR != NULL)
			free(t->GR);
		free(t);
	}
	return(NULL);
}

static struct ROOT_MOR *freeMORs(struct ROOT_MOR *p) {
	struct ROOT_MOR *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->oMOR != NULL)
			free(t->oMOR);
		if (t->MOR != NULL)
			free(t->MOR);
		free(t);
	}
	return(NULL);
}

static struct conll_errList *freeConllErrList(struct conll_errList *p) {
	if (p != NULL) {
		freeConllErrList(p->left);
		freeConllErrList(p->right);
		free(p);
	}
	return(NULL);
}

static CONLL_UTT *freeConllUtts(CONLL_UTT *p) {
	CONLL_UTT *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		free(t);
	}
	return(NULL);
}

static CNL2CH_UTT *freeChatUtts(CNL2CH_UTT *p) {
	CNL2CH_UTT *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		free(t);
	}
	return(NULL);
}

static struct chat_errList *freeChatErrList(struct chat_errList *p) {
	struct chat_errList *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->conll != NULL)
			free(t->conll);
		if (t->chat != NULL)
			free(t->chat);
		free(t);
	}
	return(NULL);
}

static void conll2chat_error(void) {
	if (fpChat != NULL) {
		fclose(fpChat);
		fpChat = NULL;
	}
	conll_root_utt = freeConllUtts(conll_root_utt);
	chat_root_utt = freeChatUtts(chat_root_utt);
	rootPOS = freePOSes(rootPOS);
	rootGR = freeGRs(rootGR);
	rootMOR = freeMORs(rootMOR);
	posErrList = freeConllErrList(posErrList);
	morErrList = freeConllErrList(morErrList);
	grErrList =  freeConllErrList(grErrList);
	chatErrList = freeChatErrList(chatErrList);
	cutt_exit(0);
}

static struct ROOT_POS *add2POSConversion(struct ROOT_POS *root, char *oPOS, char *POS) {
	struct ROOT_POS *p;

	if (root == NULL) {
		root = NEW(struct ROOT_POS);
		p = root;
	} else {
		for (p=root; p->next != NULL; p=p->next) ;
		p->next = NEW(struct ROOT_POS);
		p = p->next;
	}
	if (p == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	p->next = NULL;
	p->oPOS = NULL;
	p->POS = NULL;
	if ((p->oPOS=(char *)malloc(strlen(oPOS)+1)) == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	strcpy(p->oPOS, oPOS);
	p->len = strlen(p->oPOS);
	if ((p->POS=(char *)malloc(strlen(POS)+1)) == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	strcpy(p->POS, POS);
	return(root);
}

static void readPOSFile(void) {
	char *oPOS, *POS;
	long ln;
	FILE *fp;
	FNType mFileName[FNSize];
	
	if ((fp=OpenGenLib(posFName,"r",TRUE,TRUE,mFileName)) == NULL) {
		if (isJoinFiles) {
			fprintf(stderr, "    NOT Using any POS file\n");
			return;
		} else {
			fputs("Can't open either one of the changes files:\n",stderr);
			fprintf(stderr,"\t\"%s\", \"%s\"\n", posFName, mFileName);
			fputs("Please use +pFilename option to specify different file name\n",stderr);
			conll2chat_error();
		}
	}
	isPOSFileSpecified = TRUE;
	fprintf(stderr, "    Using POS file: %s.\n", mFileName);
	ln = 0L;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC) || templineC[0] == '#')
			continue;
		ln++;
		uS.remblanks(templineC);
		if (templineC[0] != EOS) {
			for (oPOS=templineC; isSpace(*oPOS); oPOS++) ;
			POS = strchr(oPOS, '\t');
			if (POS == NULL)
				POS = strchr(oPOS, ' ');
			if (POS == NULL) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", mFileName, ln);
				fprintf(stderr, "Missing tab on line: %s\n", templineC);
				conll2chat_error();
			}
			*POS = EOS;
			for (POS++; isSpace(*POS); POS++);
			rootPOS = add2POSConversion(rootPOS, oPOS, POS);
		}
	}
	fclose(fp);
}

static struct ROOT_GR *add2GRConversion(struct ROOT_GR *root, char *oGR, char *GR) {
	struct ROOT_GR *p;

	if (root == NULL) {
		root = NEW(struct ROOT_GR);
		p = root;
	} else {
		for (p=root; p->next != NULL; p=p->next) ;
		p->next = NEW(struct ROOT_GR);
		p = p->next;
	}
	if (p == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	p->next = NULL;
	p->oGR = NULL;
	p->GR = NULL;
	if ((p->oGR=(char *)malloc(strlen(oGR)+1)) == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	strcpy(p->oGR, oGR);
	if ((p->GR=(char *)malloc(strlen(GR)+1)) == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	strcpy(p->GR, GR);
	return(root);
}

static void readGRFile(void) {
	char *oGR, *GR, *e;
	long ln;
	FILE *fp;
	FNType mFileName[FNSize];
	
	if ((fp=OpenGenLib(grFName,"r",TRUE,TRUE,mFileName)) == NULL) {
		if (isJoinFiles) {
			fprintf(stderr, "    NOT Using any GR file\n");
			return;
		} else {
			fputs("Can't open either one of the changes files:\n",stderr);
			fprintf(stderr,"\t\"%s\", \"%s\"\n", grFName, mFileName);
			fputs("Please use +gFilename option to specify different file name\n",stderr);
			conll2chat_error();
		}
	}
	isGRFileSpecified = TRUE;
	fprintf(stderr, "    Using GR file: %s.\n", mFileName);
	ln = 0L;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC) || templineC[0] == '#')
			continue;
		ln++;
		uS.remblanks(templineC);
		if (templineC[0] != EOS) {
			for (oGR=templineC; isSpace(*oGR); oGR++);
			GR = strchr(oGR, '\t');
			if (GR == NULL)
				GR = strchr(oGR, ' ');
			if (GR == NULL) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", mFileName, ln);
				fprintf(stderr, "Missing tab on line: %s\n", templineC);
				conll2chat_error();
			}
			*GR = EOS;
			for (GR++; isSpace(*GR); GR++);
			for (e=GR; !isSpace(*e) && *e != EOS; e++);
			*e = EOS;
			rootGR = add2GRConversion(rootGR, oGR, GR);
		}
	}
	fclose(fp);
}

static struct ROOT_MOR *add2MORConversion(struct ROOT_MOR *root, char *oMOR, char *MOR) {
	struct ROOT_MOR *p;

	if (root == NULL) {
		root = NEW(struct ROOT_MOR);
		p = root;
	} else {
		for (p=root; p->next != NULL; p=p->next) ;
		p->next = NEW(struct ROOT_MOR);
		p = p->next;
	}
	if (p == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	p->next = NULL;
	p->oMOR = NULL;
	p->MOR = NULL;
	if ((p->oMOR=(char *)malloc(strlen(oMOR)+1)) == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	strcpy(p->oMOR, oMOR);
	if ((p->MOR=(char *)malloc(strlen(MOR)+1)) == NULL) {
		fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
		conll2chat_error();
	}
	strcpy(p->MOR, MOR);
	return(root);
}

static void readMORFile(void) {
	char *oMOR, *MOR, *e, isFirst;
	long ln;
	FILE *fp;
	FNType mFileName[FNSize];

	if ((fp=OpenGenLib(morFName,"r",TRUE,TRUE,mFileName)) == NULL) {
		fprintf(stderr, "    NOT Using any MOR file\n");
		return;
	}
	isMORFileSpecified = TRUE;
	fprintf(stderr, "    Using MOR file: %s.\n", mFileName);
	ln = 0L;
	isFirst = TRUE;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC) || templineC[0] == '#')
			continue;
		ln++;
		uS.remblanks(templineC);
		if (templineC[0] != EOS) {
			strcpy(templineC1, templineC);
			for (oMOR=templineC; isSpace(*oMOR); oMOR++);
			MOR = strchr(oMOR, '\t');
			if (MOR == NULL)
				MOR = strchr(oMOR, ' ');
			if (MOR == NULL) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", mFileName, ln);
				fprintf(stderr, "Missing tab on line: %s\n", templineC1);
				conll2chat_error();
			}
			*MOR = EOS;
			for (MOR++; isSpace(*MOR); MOR++);
			for (e=MOR; !isSpace(*e) && *e != EOS; e++);
			*e = EOS;
			if (isFirst && (oMOR[1] != EOS || MOR[1] != EOS)) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", mFileName, ln);
				fprintf(stderr, "First line has to be one character for element's delimiter: %s\n", templineC1);
				conll2chat_error();
			}
			isFirst = FALSE;
			rootMOR = add2MORConversion(rootMOR, oMOR, MOR);
		}
	}
	fclose(fp);
}

static void errListPrint(struct conll_errList *p) {
	if (p != NULL) {
		errListPrint(p->left);
		errListPrint(p->right);
		fprintf(stderr, "     %s\n", p->POS);
	}
}

void init(char s) {
	if (s) {
		isMisMatch = FALSE;
		lastMisMatchNo = 0L;
		coding = 0;
		isJoinFiles = FALSE;
		isPOSFileSpecified = FALSE;
		isMORFileSpecified = FALSE;
		isGRFileSpecified  = FALSE;
		ftime = TRUE;
		fpChat = NULL;
		rootPOS = NULL;
		rootGR = NULL;
		rootMOR = NULL;
		conll_root_utt = NULL;
		chat_root_utt = NULL;
		posErrList = NULL;
		morErrList = NULL;
		grErrList  = NULL;
		chatErrList = NULL;
		stout = FALSE;
		onlydata = 3;
		OverWriteFile = TRUE;
		LocalTierSelect = TRUE;
		FilterTier = 0;
		AddCEXExtension = ".cha";
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+0");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
	} else {
		if (coding == 0) {
			fprintf(stderr, "Please specify parser name with +c option.\n");
			fprintf(stderr, "Please choose one of:\n");
			fprintf(stderr, "    +cDe(pparse)\n");
			fprintf(stderr, "    +cMa(ltParser)\n");
			fprintf(stderr, "    +cTu(rboParser)\n");
			fprintf(stderr, "    +cAn(CoraCorpus)\n");
			fprintf(stderr, "    +cCo(nnexor)\n");
			fprintf(stderr, "    +cCl(earparser)\n");
			fprintf(stderr, "    +cPR(OIEL)\n");
			cutt_exit(0);
		}
		if (ftime) {
			if (coding == 3) {
				maketierchoice("%flo:",'+','\001');
			}
			readPOSFile();
			readGRFile();
			readMORFile();
			ftime = FALSE;
		}
		isMisMatch = FALSE;
		lastMisMatchNo = 0L;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	fpChat = NULL;
	posFName = "0pos.cut";
	grFName = "0gr.cut";
	morFName = "0mor.cut";
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CONLL2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	posErrList = NULL;
	morErrList = NULL;
	grErrList  = NULL;
	bmain(argc,argv,NULL);
	rootPOS = freePOSes(rootPOS);
	rootGR = freeGRs(rootGR);
	rootMOR = freeMORs(rootMOR);
	if (isPOSFileSpecified && posErrList != NULL) {
		fprintf(stderr, "\n Undefined POS code(s):\n");
		errListPrint(posErrList);
	}
	if (isMORFileSpecified && morErrList != NULL) {
		fprintf(stderr, "\n Undefined MOR code(s):\n");
		errListPrint(morErrList);
	}
	if (isGRFileSpecified && grErrList != NULL) {
		fprintf(stderr, "\n Undefined GR code(s):\n");
		errListPrint(grErrList);
	}
	posErrList = freeConllErrList(posErrList);
	morErrList = freeConllErrList(morErrList);
	grErrList  = freeConllErrList(grErrList);
	chatErrList = freeChatErrList(chatErrList);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
			if ((*f == 'd' || *f == 'D') && (*(f+1) == 'e' || *(f+1) == 'E')) {
				coding = 1; // Depparse
			} else if ((*f == 'm' || *f == 'M') && (*(f+1) == 'a' || *(f+1) == 'A')) {
				coding = 1; // MaltParser
			} else if ((*f == 't' || *f == 'T') && (*(f+1) == 'u' || *(f+1) == 'U')) {
				coding = 1; // TurboParser
			} else if ((*f == 'a' || *f == 'A') && (*(f+1) == 'n' || *(f+1) == 'N')) {
				coding = 2; // AnCoraCorpus
			} else if ((*f == 'c' || *f == 'C') && (*(f+1) == 'o' || *(f+1) == 'O')) {
				coding = 3; // Connexor
			} else if ((*f == 'c' || *f == 'C') && (*(f+1) == 'l' || *(f+1) == 'L')) {
				coding = 4; // Clearparser
			} else if ((*f == 'p' || *f == 'P') && (*(f+1) == 'r' || *(f+1) == 'R')) {
				coding = 5; // Clearparser
			} else {
				fprintf(stderr, "Please choose one of:\n");
				fprintf(stderr, "    +cDe(pparse)\n");
				fprintf(stderr, "    +cMa(ltParser)\n");
				fprintf(stderr, "    +cTu(rboParser)\n");
				fprintf(stderr, "    +cAn(CoraCorpus)\n");
				fprintf(stderr, "    +cCo(nnexor)\n");
				fprintf(stderr, "    +cCl(earparser)\n");
				fprintf(stderr, "    +cPR(OIEL)\n");
				cutt_exit(0);
			}
			break;
		case 'j':
			isJoinFiles = TRUE;
			no_arg_option(f);
			break;
		case 'g':
			if (*f == EOS) {
				fprintf(stderr, "Please specify %%gra conversion file\n");
				cutt_exit(0);
			}
			grFName = f;
			break;
		case 'm':
			if (*f == EOS) {
				fprintf(stderr, "Please specify additional MOR elements conversion file\n");
				cutt_exit(0);
			}
			morFName = f;
			break;
		case 'p':
			if (*f == EOS) {
				fprintf(stderr, "Please specify \"Part of Speech\" conversion file\n");
				cutt_exit(0);
			}
			posFName = f;
			break;
#ifdef UNX
		case 'L':
			strcpy(lib_dir, f);
			len = strlen(lib_dir);
			if (len > 0 && lib_dir[len-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static CONLL_UTT *add2ConllUtt(CONLL_UTT *root, int cnum, int pnum,
					const char *GRA, const char *word, const char *stem, const char *POS, const char *MOR) {
	CONLL_UTT *p;

	if (root == NULL) {
		if ((p=NEW(CONLL_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			conll2chat_error();
		}
		root = p;
	} else {
		for (p=root; p->next != NULL; p=p->next) ;
		if ((p->next=NEW(CONLL_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			conll2chat_error();
		}
		p = p->next;
	}
	p->next = NULL;
	p->cnum = cnum;
	p->pnum = pnum;
	strcpy(p->GRA, GRA);
	strcpy(p->word, word);
	strcpy(p->stem, stem);
	strcpy(p->POS, POS);
	strcpy(p->MOR, MOR);
	p->changed = FALSE;
	return(root);
}

static char isConvertPunc(char *POS, char *stem) {
	if (stem[0] == (char)0xC2 && stem[1] == (char)0xA1 && stem[2] == EOS) {
		strcpy(POS, "punc");
		strcpy(stem, "sexc");
		return(TRUE);
	} else if (!strcmp(stem, ",")) {
		strcpy(POS, "cm");
		strcpy(stem, "cm");
		return(TRUE);
	} else if (!strcmp(stem, "[")) {
		strcpy(POS, "punc");
		strcpy(stem, "la");
		return(TRUE);
	} else if (!strcmp(stem, "]")) {
		strcpy(POS, "punc");
		strcpy(stem, "ra");
		return(TRUE);
	} else if (!strcmp(stem, ":")) {
		strcpy(POS, "punc");
		strcpy(stem, "colon");
		return(TRUE);
	} else if (!strcmp(stem, "\"")) {
		strcpy(POS, "bq");
		strcpy(stem, "bq");
		return(TRUE);
	} else if (!strcmp(stem, "-")) {
		strcpy(POS, "punc");
		strcpy(stem, "dash");
		return(TRUE);
	} else if (!strcmp(stem, "/")) {
		strcpy(POS, "punc");
		strcpy(stem, "sl");
		return(TRUE);
	} else if (stem[0] == (char)0xC2 && stem[1] == (char)0xBF && stem[2] == EOS) {
		strcpy(POS, "punc");
		strcpy(stem, "sq");
		return(TRUE);
	} else if (!strcmp(stem, "{")) {
		strcpy(POS, "punc");
		strcpy(stem, "lc");
		return(TRUE);
	} else if (!strcmp(stem, "}")) {
		strcpy(POS, "punc");
		strcpy(stem, "rc");
		return(TRUE);
	} else if (!strcmp(stem, "(")) {
		strcpy(POS, "punc");
		strcpy(stem, "lp");
		return(TRUE);
	} else if (!strcmp(stem, ")")) {
		strcpy(POS, "punc");
		strcpy(stem, "rp");
		return(TRUE);
	} else if (stem[0] == (char)0xC2 && stem[1] == (char)0xAB && stem[2] == EOS) {
		strcpy(POS, "punc");
		strcpy(stem, "ld");
		return(TRUE);
	} else if (stem[0] == (char)0xC2 && stem[1] == (char)0xBB && stem[2] == EOS) {
		strcpy(POS, "punc");
		strcpy(stem, "rd");
		return(TRUE);
	} else if (!strcmp(stem, "...")) {
		strcpy(POS, "punc");
		strcpy(stem, "gap");
		return(TRUE);
	} else if (!strcmp(stem, "%")) {
		strcpy(POS, "punc");
		strcpy(stem, "pc");
		return(TRUE);
	} else if (!strcmp(stem, ";")) {
		strcpy(POS, "punc");
		strcpy(stem, "semi");
		return(TRUE);
	} else if (!strcmp(stem, "_")) {
		strcpy(POS, "punc");
		strcpy(stem, "under");
		return(TRUE);
	} else if (!strcmp(stem, "+")) {
		strcpy(POS, "punc");
		strcpy(stem, "plus");
		return(TRUE);
	} else if (!strcmp(stem, "=")) {
		strcpy(POS, "punc");
		strcpy(stem, "eq");
		return(TRUE);
	}
	return(FALSE);
}

static char isPOSCodeMatched(char *oPOS, char *POS) {
	struct ROOT_POS *p;
	
	for (p=rootPOS; p != NULL; p=p->next) {
		if (strncmp(oPOS, p->oPOS, p->len) == 0) {
			strcpy(POS, p->POS);
			return(TRUE);
		}
	}
	return(FALSE);
}

static char isGRCodeMatched(char *oGR, char *GR) {
	struct ROOT_GR *p;

	for (p=rootGR; p != NULL; p=p->next) {
		if (strcmp(oGR, p->oGR) == 0) {
			strcpy(GR, p->GR);
			return(TRUE);
		}
	}
	return(FALSE);
}

static char isMORCodeMatched(char *oMOR, char *MOR) {
	struct ROOT_MOR *p;

	for (p=rootMOR; p != NULL; p=p->next) {
		if (strcmp(oMOR, p->oMOR) == 0) {
			strcpy(MOR, p->MOR);
			return(TRUE);
		}
	}
	return(FALSE);
}

static struct conll_errList *add2ConllErrList(struct conll_errList *p, char *w) {
	int cond;

	if (p == NULL) {
		p = NEW(struct conll_errList);
		if (p == NULL) {
			fprintf(stderr, "\n\nconll2chat: out of memory\n\n");
			conll2chat_error();
		}
		strncpy(p->POS, w, WLEN);
		p->POS[WLEN] = EOS;
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->POS)) == 0) {
	} else if (cond < 0) {
		p->left = add2ConllErrList(p->left, w);
	} else {
		p->right = add2ConllErrList(p->right, w);
	}
	return(p);
}
/*
static void replaceVBar(char *MOR, char c) {
	int i;

	i = 0;
	while (MOR[i] != EOS) {
		if (MOR[i] == '|')
			MOR[i] = c;
		else
			i++;
	}
}
*/
static void removeVBar(char *MOR) {
	int i;

	i = 0;
	while (MOR[i] != EOS) {
		if (MOR[i] == '|')
			strcpy(MOR+i, MOR+i+1);
		else
			i++;
	}
}

static char alphaStem(char *s) {
	for (; *s != EOS; s++) {
		if ((*s >= ' ' && *s <= '@') || (*s >= '[' && *s <= ']'))
			return(FALSE);
	}
	return(TRUE);
}

static char getTruePOS(char *oPOS, char *POS, char *stem) {
	char isStemChanged;

	isStemChanged = FALSE;
	if (isConvertPunc(POS, stem)) {
		isStemChanged = TRUE;
	} else if (!strcmp(stem, ".") || !strcmp(stem, "!") || !strcmp(stem, "?")) {
		POS[0] = EOS;
		isStemChanged = TRUE;
	} else if (!strcmp(oPOS, "n")) {
		strcpy(POS, oPOS);
	} else if (!strcmp(oPOS, "date")) {
		strcpy(POS, oPOS);
	} else if (!strcmp(oPOS, "num")) {
		strcpy(POS, oPOS);
	} else if (isPOSCodeMatched(oPOS, POS)) {
	} else {
		strcpy(POS, oPOS);
		if (isPOSFileSpecified && oPOS[0] != EOS)
			posErrList = add2ConllErrList(posErrList, oPOS);
	}
	if (!strcmp(POS, "date")) {
		strcpy(stem, "date");
		isStemChanged = TRUE;
	} else if (!strcmp(POS, "num") && !alphaStem(stem)) {
		strcpy(stem, "num");
		isStemChanged = TRUE;
	}
	return(!isStemChanged);
}

static void getTrueGR(char *oGR, char *GR) {
	if (isGRCodeMatched(oGR, GR)) {
	} else {
		strcpy(GR, oGR);
		if (isGRFileSpecified && oGR[0] != EOS && strcmp(oGR, "PUNCT") != 0)
			grErrList = add2ConllErrList(grErrList, oGR);
	}
}

static void getTrueMOR(char *oMOR, char *MOR) {
	char *b, *e, t, od;
	char tMOR[WLEN];

	od = rootMOR->oMOR[0];
	MOR[0] = EOS;
	b = oMOR;
	do {
		e = strchr(b, od);
		if (e != NULL) {
			t = *e;
			*e = EOS;
		}
// controls |stem- or |stem&
		if (MOR[0] != EOS)
			strcat(MOR, rootMOR->MOR);
		if (isMORCodeMatched(b, tMOR)) {
			strcat(MOR, tMOR);
		} else {
			strcat(MOR, b);
			if (isMORFileSpecified && b[0] != EOS)
				morErrList = add2ConllErrList(morErrList, b);
		}
		if (e != NULL) {
			*e = t;
			b = e + 1;
		}
	} while (e != NULL) ;
}

static void outputSp(CONLL_UTT *p, char *line) {
	line[0] = EOS;
	while (p != NULL) {
		if (p->cnum != 0) {
			if ((p->word[0] == '-' || p->word[0] == '$') && p->word[1] == '\'' && coding != 3)
				;
			else {
				if (line[0] != EOS)
					strcat(line, " ");
				strcat(line, p->word);
			}
		}
		p = p->next;
	}
}

static void outputMor(CONLL_UTT *p) {
	char sp[10];
	const char *sepSt;

	if (coding == 3) {
		strcpy(sp, "%xmor:");
	} else {
		strcpy(sp, "%mor:");
	}
	templineC1[0] = EOS;
	sepSt = " ";
	while (p != NULL) {
		if (p->cnum != 0) {
			if (getTruePOS(p->POS, templineC, p->stem)) {
// to output surface form instead of stem form
//				strcpy(p->stem, p->word);
//				if (isupper((unsigned char)p->stem[0]))
//					p->stem[0] = (char)tolower((unsigned char)p->stem[0]);
// to output surface form instead of stem form
			}
			if (templineC1[0] != EOS) {
				if ((p->word[0] == '-' && p->word[1] == '\'') || (p->word[0] == '-' && p->word[1] == 'n' && p->word[2] == '\''))
					strcat(templineC1, "~");
				else
					strcat(templineC1, sepSt);
			}
			if (templineC[0] != EOS) {
				strcat(templineC1, templineC);
				strcat(templineC1, "|");
			}
			strcat(templineC1, p->stem);
			if (p->MOR[0] != EOS && strcmp(p->MOR, "_")) {
				if (!isJoinFiles || (p->MOR[0] != '-' && p->MOR[0] != '&' && p->MOR[0] != '_')) {
					if (rootMOR != NULL) {
						getTrueMOR(p->MOR, templineC);
						if (templineC[0] != EOS) {
							strcpy(p->MOR, templineC);
							if (templineC[0] != rootMOR->MOR[0])
								strcat(templineC1, "-");
						} else
							strcat(templineC1, "-");
					} else
						strcat(templineC1, "-");
//					replaceVBar(p->MOR, '_');
				} else
					removeVBar(p->MOR);
				strcat(templineC1, p->MOR);
			}
			if (p->word[0] == '$' && p->word[1] == '\'')
				sepSt = "$";
			else
				sepSt = " ";
		}
		p = p->next;
	}
	if (templineC1[0] != EOS)
		printout(sp, templineC1, NULL, NULL, TRUE);
}

static void outputGra(CONLL_UTT *p) {
	char sp[10], numS[WLEN], isGRAFound, *s;

	if (coding == 3) {
		strcpy(sp, "%xgra:");
	} else {
		strcpy(sp, "%gra:");
	}
	templineC1[0] = EOS;
	if (isJoinFiles)
		isGRAFound = FALSE;
	else
		isGRAFound = TRUE;
	while (p != NULL) {
		if (p->cnum != 0) {
			if (templineC1[0] != EOS)
				strcat(templineC1, " ");
			sprintf(numS, "%d", p->cnum);
			strcat(templineC1, numS);
			strcat(templineC1, "|");
			getTrueGR(p->GRA, templineC);
			if (coding == 3) {
				if ((s=strchr(templineC, '>')) != NULL) {
					strcat(templineC1, s+1);
					strcat(templineC1, "|");
					*s = EOS;
				}
			} else {
				sprintf(numS, "%d", p->pnum);
				strcat(templineC1, numS);
				strcat(templineC1, "|");
			}
			strcat(templineC1, templineC);
			if (strcmp(templineC, "_") || coding == 3)
				isGRAFound = TRUE;
		}
		p = p->next;
	}
	if (templineC1[0] != EOS && isGRAFound)
		printout(sp, templineC1, NULL, NULL, TRUE);
}

static CNL2CH_UTT *add2ChatUtts(CNL2CH_UTT *root) {
	CNL2CH_UTT *utt;

	if (root == NULL) {
		utt = NEW(CNL2CH_UTT);
		if (utt == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			conll2chat_error();
		}
		root = utt;
	} else {
		for (utt=root; utt->next != NULL; utt=utt->next) ;
		if ((utt->next=NEW(CNL2CH_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			conll2chat_error();
		}
		utt = utt->next;
	}
	utt->next = NULL;
	utt->lineno = lineno;
	att_cp(0L, utt->speaker, utterance->speaker, utt->attSp, utterance->attSp);
	att_cp(0L, utt->line, utterance->line, utt->attLine, utterance->attLine);
	strcpy(utt->tuttline, utterance->tuttline);
	return(root);
}

static struct chat_errList *add2ChatErrList(struct chat_errList *root, const char *conll, const char *chat) {
	struct chat_errList *p;

	if (root == NULL) {
		if ((p=NEW(struct chat_errList)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			conll2chat_error();
		}
		root = p;
	} else {
		for (p=root; p->next != NULL; p=p->next) ;
		if ((p->next=NEW(struct chat_errList)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			conll2chat_error();
		}
		p = p->next;
	}
	p->next = NULL;
	p->conll = (char *)malloc(strlen(conll)+1);
	if (p->conll == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		conll2chat_error();
	}
	strcpy(p->conll, conll);
	p->chat = (char *)malloc(strlen(chat)+1);
	if (p->chat == NULL) {
		fprintf(stderr,"Error: out of memory\n");
		conll2chat_error();
	}
	strcpy(p->chat, chat);
	return(root);
}

static int excludeSpMorWords(char *word) {
	if (uS.mStricmp(word, "xx") == 0 || uS.mStricmp(word, "yy") == 0 || word[0] == '[') {
		return(FALSE);
	} else if (word[0] == '+') {
		if (isMorExcludePlus(word))
			return(FALSE);
	}
	return(TRUE);
}

static int removeAt(char *word) {
	char *at;

	at = strchr(word, '@');
	if (at != NULL) {
		for (; *at != EOS; at++)
			*at = ' ';
	}
	return(TRUE);
}

static char isEmptyLine(char *s) {
	for (; *s != EOS; s++) {
		if (*s == ',' || *s >= '@')
			return(FALSE);
	}
	return(TRUE);
}

static void remSQbANGb(char *line) {
	int i;
	char isSQ;

	isSQ = FALSE;
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '<' || line[i] == '>')
			line[i] = ' ';
		else if (line[i] == '[')
			isSQ = TRUE;
		else if (line[i] == ']') {
			line[i] = ' ';
			isSQ = FALSE;
		}
		if (isSQ)
			line[i] = ' ';
	}
}

static char isSpTierMatched(CNL2CH_UTT *root, char *isMOR, char *isGR) {
	int  i;
	char tBEG, tEND, isANGfound, isBracket;
//2019-04-29	char tOQ, tCQ;
	CNL2CH_UTT *spUtt, *floUtt, *grUtt, *morUtt;
	CNL2CH_UTT *utt;

	spUtt =  NULL;
	floUtt = NULL;
	morUtt = NULL;
	grUtt = NULL;
	*isMOR = FALSE;
	*isGR = FALSE;
	spUtt = NULL;
	isANGfound = 0;
	for (utt=root; utt != NULL; utt=utt->next) {
		if (utt->speaker[0] == '*') {
			spUtt = utt;
/* 2019-04-29
			tOQ  = isRemoveCAChar[NOTCA_OPEN_QUOTE];
			isRemoveCAChar[NOTCA_OPEN_QUOTE] = FALSE;
			tCQ  = isRemoveCAChar[NOTCA_CLOSE_QUOTE];
			isRemoveCAChar[NOTCA_CLOSE_QUOTE] = FALSE;
*/
			tBEG  = isRemoveCAChar[NOTCA_VOCATIVE];
			isRemoveCAChar[NOTCA_VOCATIVE] = FALSE;
			tEND  = isRemoveCAChar[NOTCA_DOUBLE_COMMA];
			isRemoveCAChar[NOTCA_DOUBLE_COMMA] = FALSE;
			filterData(spUtt->speaker,spUtt->tuttline);
			filterwords(spUtt->speaker,spUtt->tuttline,exclude);
			filterwords(spUtt->speaker, spUtt->tuttline, excludeSpMorWords);
			remSQbANGb(spUtt->tuttline);
//2019-04-29			isRemoveCAChar[NOTCA_OPEN_QUOTE]    = tOQ;
//2019-04-29			isRemoveCAChar[NOTCA_CLOSE_QUOTE]   = tCQ;
			isRemoveCAChar[NOTCA_VOCATIVE]      = tBEG;
			isRemoveCAChar[NOTCA_DOUBLE_COMMA]  = tEND;
		} else if (uS.partcmp(utt->speaker, "%mor:", FALSE, FALSE)) {
			*isMOR = TRUE;
			morUtt = utt;
		} else if (uS.partcmp(utt->speaker, "%gra:", FALSE, FALSE)) {
			*isGR = TRUE;
			grUtt = utt;
		} else if (uS.partcmp(utt->speaker, "%flo:", FALSE, FALSE)) {
			floUtt = utt;
			isBracket = FALSE;
			for (i=0; floUtt->line[i] != EOS; i++) {
				if (floUtt->line[i] == '[') {
					isBracket = TRUE;
				} else if (floUtt->line[i] == ']') {
					isBracket = FALSE;
				} else if (floUtt->line[i] == '<') {
					if (!isBracket && isANGfound == 0)
						isANGfound = 1;
				} else if (floUtt->line[i] == '>') {
					if (!isBracket && isANGfound == 1)
						isANGfound = 2;
				}
				floUtt->tuttline[i] = floUtt->line[i];
			}
			floUtt->tuttline[i] = EOS;
			filterData(floUtt->speaker,floUtt->tuttline);
			filterwords(floUtt->speaker,floUtt->tuttline,exclude);
			filterwords(floUtt->speaker,floUtt->tuttline,removeAt);
			remSQbANGb(floUtt->tuttline);
		}
	}
	if (spUtt == NULL) {
		for (utt=root; utt != NULL; utt=utt->next) {
			printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
		}
		spareTier2[0] = EOS;
		return(0);
	}
	if (coding == 3) {
		if (floUtt != NULL) {
			strcpy(spareTier2, floUtt->tuttline);
			removeAllSpaces(spareTier2);
			if (uS.mStricmp(spareTier2, spareTier1)) {
				if (isANGfound == 2) {
					isBracket = FALSE;
					for (i=0; floUtt->line[i] != EOS; i++) {
						if (floUtt->line[i] == '<') {
							isBracket = TRUE;
						} else if (floUtt->line[i] == '>') {
							isBracket = FALSE;
							floUtt->tuttline[i] = ' ';
						}
						if (isBracket)
							floUtt->tuttline[i] = ' ';
					}
					strcpy(spareTier2, floUtt->tuttline);
					removeAllSpaces(spareTier2);
					if (uS.mStricmp(spareTier2, spareTier1)) {
						for (utt=root; utt != NULL; utt=utt->next) {
							printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
						}
						return(0);
					}
				} else {
					for (utt=root; utt != NULL; utt=utt->next) {
						printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
					}
					return(0);
				}
			}
			isFloFound = TRUE;
		} else if (isFloFound) {
			for (utt=root; utt != NULL; utt=utt->next) {
				printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
			}
			return(2);
		} else {
			strcpy(spareTier2, spUtt->tuttline);
			removeAllSpaces(spareTier2);
			if (uS.mStricmp(spareTier2, spareTier1)) {
				for (utt=root; utt != NULL; utt=utt->next) {
					printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
				}
				return(0);
			}
		}
	} else {
		strcpy(spareTier2, spUtt->tuttline);
		removeAllSpaces(spareTier2);
		if (uS.mStricmp(spareTier2, spareTier1)) {
			for (utt=root; utt != NULL; utt=utt->next) {
				printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
			}
			return(0);
		}
	}
	for (utt=root; utt != NULL; utt=utt->next) {
		if (utt != grUtt)
			printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
	}
	return(1);
}

static void checkUttDelAndOutput(char *isMoreFile) {
	int  cnum, pnum, tchatmode;
	char isUttDelFound, isMOR, isGR, res, isOutputHeader;
	long ln;
	CONLL_UTT *p;
	FILE *tfp;

	if (conll_root_utt == NULL)
		return;
	cnum = 0;
	isUttDelFound = 0;
/* if there is more than one utterance per numbers run
	CONLL_UTT *pt;
	int  uttCnt;
	uttCnt = 0;
	for (p=conll_root_utt; p != NULL; p=p->next) {
		if (!strcmp(p->stem, ".") || !strcmp(p->stem, "!") || !strcmp(p->stem, "?")) {
			if (uttCnt > 0) {
				cnum = 0;
				for (pt=conll_root_utt; pt != NULL; pt=pt->next) {
					cnum++;
					pt->cnum = cnum;
				}
				renumber(conll_root_utt);
			}
			pt = p->next;
			p->next = NULL;
 			outputSp(conll_root_utt, spareTier1);
			if (spareTier1[0] != EOS)
				printout("*TXT:", spareTier1, NULL, NULL, TRUE);
			outputMor(conll_root_utt);
			outputGra(conll_root_utt);
			p->next = pt;
			conll_root_utt = p->next;
			uttCnt++;
			isUttDelFound = 1;
		} else
			isUttDelFound = 0;
	}
	if (!isUttDelFound) {
		if (uttCnt > 0) {
			cnum = 0;
			for (pt=conll_root_utt; pt != NULL; pt=pt->next) {
				cnum++;
				pt->cnum = cnum;
			}
			renumber(conll_root_utt);
		}
		pnum = 0;
		for (p=conll_root_utt; p->next != NULL; p=p->next) {
			if (!strcmp(p->GRA, "sentence"))
				pnum = p->cnum;
		}
		cnum = p->cnum + 1;
		conll_root_utt = add2ConllUtt(conll_root_utt, cnum, cnum, pnum, "PUNCT", ".", ".", "punc", "");
		outputSp(conll_root_utt, spareTier1);
		if (spareTier1[0] != EOS)
			printout("*TXT:", spareTier1, NULL, NULL, TRUE);
		outputMor(conll_root_utt);
		outputGra(conll_root_utt);
	}
*/
	for (p=conll_root_utt; p != NULL; p=p->next) {
		if (isUttDel(p->stem)) {
			isUttDelFound++;
		}
	}
	if (isUttDelFound == 0 && !isJoinFiles) {
		pnum = 0;
		for (p=conll_root_utt; p->next != NULL; p=p->next) {
			if (!strcmp(p->GRA, "sentence"))
				pnum = p->cnum;
		}
		cnum = p->cnum + 1;
		conll_root_utt = add2ConllUtt(conll_root_utt, cnum, pnum, "PUNCT", ".", ".", "punc", "");
		isUttDelFound++;
	}
	if (isUttDelFound == 1) {
		isMOR = FALSE;
		isOutputHeader = FALSE;
		outputSp(conll_root_utt, spareTier1);
		if (isJoinFiles) {
			remSQbANGb(spareTier1);
			removeAllSpaces(spareTier1);
			tfp = fpin;
			fpin = fpChat;
			tchatmode = chatmode;
			chatmode = 1;
			res = FALSE;
			if (*isMoreFile == TRUE) {
				while ((*isMoreFile=getwholeutter()) == TRUE) {
					if (utterance->speaker[0] == '*') {
						if (chat_root_utt != NULL) {
							res = isSpTierMatched(chat_root_utt, &isMOR, &isGR);
							ln = chat_root_utt->lineno;
							chat_root_utt = freeChatUtts(chat_root_utt);
							chat_root_utt = add2ChatUtts(chat_root_utt);
							if (res == 1) {
								lastMisMatchNo = 0L;
								break;
							} else {
								if (lastMisMatchNo == 0L) {
									if (res != 2 && !isEmptyLine(spareTier2)) {
										chatErrList = add2ChatErrList(chatErrList, spareTier1, spareTier2);
									}
									lastMisMatchNo = ln;
								}
							}
						} else
							chat_root_utt = add2ChatUtts(chat_root_utt);
					} else if (utterance->speaker[0] == '%') {
						chat_root_utt = add2ChatUtts(chat_root_utt);
					} else {
						if (chat_root_utt != NULL) {
							res = isSpTierMatched(chat_root_utt, &isMOR, &isGR);
							ln = chat_root_utt->lineno;
							chat_root_utt = freeChatUtts(chat_root_utt);
							if (res == 1) {
								lastMisMatchNo = 0L;
								isOutputHeader = TRUE;
								break;
							} else {
								isOutputHeader = FALSE;
								printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
								if (lastMisMatchNo == 0L) {
									if (res != 2 && !isEmptyLine(spareTier2)) {
										chatErrList = add2ChatErrList(chatErrList, spareTier1, spareTier2);
									}
									lastMisMatchNo = ln;
								}
							}
						} else {
							isOutputHeader = FALSE;
							printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
						}
					}
				}
			}
			if (chat_root_utt != NULL && !res) {
				res = isSpTierMatched(chat_root_utt, &isMOR, &isGR);
				ln = chat_root_utt->lineno;
				if (res == 1) {
					chat_root_utt = freeChatUtts(chat_root_utt);
					lastMisMatchNo = 0L;
				} else
					if (lastMisMatchNo == 0L) {
						if (res != 2 && !isEmptyLine(spareTier2)) {
							chatErrList = add2ChatErrList(chatErrList, spareTier1, spareTier2);
						}
						lastMisMatchNo = ln;
					}
			}
			if (!res) {
				isMisMatch = TRUE;
			}
			chatmode = tchatmode;
			fpin = tfp;
		} else {
			if (spareTier1[0] != EOS)
				printout("*TXT:", spareTier1, NULL, NULL, TRUE);
		}
		if (!isMOR)
			outputMor(conll_root_utt);
		outputGra(conll_root_utt);
		if (isOutputHeader)
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
}

static void getPOSMor(char *c5, char *POS, char *MOR) {
	int i, j;

	for (i=0; isSpace(c5[i]); i++) ;
	j = 0;
	if (c5[i] == '@') {
		for (; !isSpace(c5[i]) && c5[i] != EOS; i++) {
			POS[j++] = c5[i];
		}
		for (; isSpace(c5[i]); i++) ;
	}
	POS[j] = EOS;
	j = 0;
	for (; c5[i] != EOS; i++) {
		if (isSpace(c5[i]))
			MOR[j++] = '_';
		else
			MOR[j++] = c5[i];
	}
	MOR[j] = EOS;
}

static int countTabs(char *line) {
	int numTabs = 0;

	for (; *line != EOS; line++) {
		if (*line == '\t')
			numTabs++;
	}
	return(numTabs);
}

void call() {		/* this function is self-explanatory */
	int  col;
	int  cnum, len, tchatmode, numTabs;
	char c2[WLEN], c3[WLEN], c4[WLEN], c5[WLEN], c6[WLEN], c7[WLEN], c8[WLEN], c9[WLEN];
	char *b, *e, t, isMoreFile, resetCount;
	long conllLineNo;
	FILE *tfp;
	CNL2CH_UTT *utt;
	struct chat_errList *err;

	isFloFound = FALSE;
	if (isJoinFiles) {
		fpChat = NULL;
		strcpy(FileName1, oldfname);
		b = strrchr(FileName1, '.');
		if (b != NULL) {
			*b = EOS;
			col = strlen(FileName1);
			strcat(FileName1, ".cha");
			fpChat = fopen(FileName1, "r");
			if (fpChat == NULL) {
				FileName1[col] = EOS;
				b = strrchr(FileName1, '.');
				if (b != NULL) {
					*b = EOS;
					strcat(FileName1, ".cha");
					fpChat = fopen(FileName1, "r");
				}
			}
		}
		if (fpChat == NULL) {
			strcpy(FileName1, oldfname);
			strcat(FileName1, ".cha");
			fpChat = fopen(FileName1, "r");
		}
		if (fpChat == NULL) {
			fprintf(stderr, "\nWARNING: Can't open CHAT file corresponding to parser file \"%s\"\n\n", oldfname);
			return;
		}
		isMoreFile = TRUE;
		chat_root_utt = NULL;
		currentatt = 0;
		currentchar = (char)getc_cr(fpChat, &currentatt);
	} else {
		fprintf(fpout, "%s\n", UTF8HEADER);
		fprintf(fpout, "@Begin\n");
		fprintf(fpout, "@Languages:	eng\n");
		fprintf(fpout, "@Participants:\tTXT Text\n");
//		fprintf(fpout, "@Options:\theritage\n");
		fprintf(fpout, "@ID:	eng|text|TXT|||||Text|||\n");
		isMoreFile = FALSE;
	}
	cnum = 0;
	resetCount = FALSE;
	conllLineNo = 0L;
	while (fgets_cr(spareTier3, UTTLINELEN, fpin) != NULL) {
		conllLineNo++;
		if (uS.partwcmp(spareTier3, UTF8HEADER) || spareTier3[0] == '#') {
			continue;
		}
		if (coding != 3)
			cnum = 0;
		strcpy(c2, "_");
		strcpy(c3, "_");
		strcpy(c4, "_");
		strcpy(c5, "_");
		strcpy(c6, "_");
		strcpy(c7, "_");
		strcpy(c8, "_");
		strcpy(c9, "_");
		uS.remblanks(spareTier3);
		if (!isdigit(spareTier3[0])) {
			checkUttDelAndOutput(&isMoreFile);
			conll_root_utt = freeConllUtts(conll_root_utt);
			resetCount = FALSE;
			if (coding == 3)
				cnum = 0;
		} else {
			if (resetCount) {
				checkUttDelAndOutput(&isMoreFile);
				conll_root_utt = freeConllUtts(conll_root_utt);
				resetCount = FALSE;
				if (coding == 3)
					cnum = 0;
			} else if (spareTier3[0] == '1' && !isdigit(spareTier3[1])) {
				checkUttDelAndOutput(&isMoreFile);
				conll_root_utt = freeConllUtts(conll_root_utt);
				resetCount = FALSE;
				if (coding == 3)
					cnum = 0;
			}
			if (coding == 3) {
				numTabs = countTabs(spareTier3);
			} else
				numTabs = 0;
			col = 0;
			b = spareTier3;
			while (col < 9) {
				col++;
				if (numTabs == 4 && (col == 2 || col == 3))
					for (e=b; *e != '\t' && *e != EOS; e++) ;
				else
					for (e=b; !isSpace(*e) && *e != EOS; e++) ;
				t = *e;
				*e = EOS;
				if (col == 1) {
					if (coding == 3)
						cnum++;
					else
						cnum = atoi(b);
					if (cnum == 0) {
						fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, conllLineNo);
						fprintf(stderr, "cnum == 0\n");
						conll2chat_error();
					}
				} else if (col == 2) {
					strcpy(c2, b);
				} else if (col == 3) {
					strcpy(c3, b);
				} else if (col == 4) {
					if (coding == 3) {
						if (*b == '@' || isupper(*b)) {
							*e = t;
							strcpy(c5, b);
							break;
						} else
							strcpy(c4, b);
					} else
						strcpy(c4, b);
				} else if (col == 5) {
					if (coding == 3) {
						*e = t;
						strcpy(c5, b);
						break;
					} else
						strcpy(c5, b);
				} else if (col == 6) {
					strcpy(c6, b);
				} else if (col == 7) {
					strcpy(c7, b);
				} else if (col == 8) {
					strcpy(c8, b);
				} else if (col == 9) {
					strcpy(c9, b);
				}
				*e = t;
				for (; isSpace(*e); e++) ;
				if (*e == EOS)
					break;
				else
					b = e;
			}
			if (col < 2) {
				if (coding == 3) {
				} else {
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, conllLineNo);
					fprintf(stderr, "Line with only number found:\n");
					fprintf(stderr, "\t%s\n", spareTier3);
					conll2chat_error();
				}
			} else if (coding == 1) { // Depparse - MaltParser, TurboParser
				if (!isdigit(c7[0]) && c7[0] != '_') {
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, conllLineNo);
					fprintf(stderr, "HEAD == 0\n");
					conll2chat_error();
				}
				conll_root_utt = add2ConllUtt(conll_root_utt, cnum, atoi(c7), c8, c2, c3, c4, c6);
			} else if (coding == 2) { // AnCoraCorpus
				if (!isdigit(c5[0]) && c5[0] != '_') {
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, conllLineNo);
					fprintf(stderr, "HEAD == 0\n");
					conll2chat_error();
				}
				if (!strcmp(c2, "_") && !strcmp(c3, "_")) {
					strcpy(c2, "0sub");
					strcpy(c3, "0sub");
					strcpy(c4, "n");
					strcpy(c6, "suj");
					conll_root_utt = add2ConllUtt(conll_root_utt, cnum, atoi(c5), c6, c2, c3, c4, c8);
				} else
					conll_root_utt = add2ConllUtt(conll_root_utt, cnum, atoi(c5), c6, c2, c3, c4, c8);
			} else if (coding == 3) { // Connexor
				len = strlen(c2);
				if (c2[0] == '<' && c2[len-1] == '>')
					continue;
				getPOSMor(c5, c8, c9);
				conll_root_utt = add2ConllUtt(conll_root_utt, cnum, 0, c4, c2, c3, c8, c9);
				if (isUttDel(c2) && isUttDel(c3))
					resetCount = TRUE;
			} else if (coding == 4) { // Clearparser
			} else if (coding == 5) { // PROIEL
				if (!isdigit(c7[0]) && c7[0] != '_') {
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, conllLineNo);
					fprintf(stderr, "HEAD == 0\n");
					conll2chat_error();
				}
				b = strchr(c8, '(');
				if (b != NULL)
					*b = EOS;
				conll_root_utt = add2ConllUtt(conll_root_utt, cnum, atoi(c7), c8, c2, c3, c5, c6);
			}
//conll_root_utt = add2ConllUtt(conll_root_utt, cnum, pnum, GRA, word, stem, POS, MOR);
		}
	}
	if (conll_root_utt != NULL) {
		checkUttDelAndOutput(&isMoreFile);
		conll_root_utt = freeConllUtts(conll_root_utt);
		cnum = 0;
	}
	if (isJoinFiles && isMoreFile == TRUE) {
		tfp = fpin;
		fpin = fpChat;
		tchatmode = chatmode;
		chatmode = 1;
		for (utt=chat_root_utt; utt != NULL; utt=utt->next) {
			printout(utt->speaker,utt->line,utt->attSp,utt->attLine,FALSE);
		}
		while (getwholeutter()) {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
		chatmode = tchatmode;
		fpin = tfp;
	}
	if (fpChat != NULL) {
		fclose(fpChat);
		fpChat = NULL;
	}
	chat_root_utt = freeChatUtts(chat_root_utt);
	if (!isJoinFiles)
		fprintf(fpout, "@End\n");
	else if (isMisMatch) {
		fprintf(stderr,"\n*** File \"%s\": line %ld.\n", FileName1, lastMisMatchNo);
		fprintf(stderr, "    Missmatch found.\n");
		for (err=chatErrList; err != NULL; err=err->next) {
			fprintf(stderr, "CONLL=%s\n", err->conll);
			fprintf(stderr, "CHAT =%s\n", err->chat);
		}
		fprintf(stderr, "\n");
		chatErrList = freeChatErrList(chatErrList);
	}
}
