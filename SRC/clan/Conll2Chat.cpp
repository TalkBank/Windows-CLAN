/**********************************************************************
 "Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
*/

#define CHAT_MODE 0
#include "cu.h"

#if !defined(UNX)
#define _main Connl2Chat_main
#define call Connl2Chat_call
#define getflag Connl2Chat_getflag
#define init Connl2Chat_init
#define usage Connl2Chat_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define POSFNAME "0pos.cut"
#define GRFNAME  "0gr.cut"
#define WLEN 256

extern char OverWriteFile;

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

struct utt_s {
	int  cnum;
	int  anum;
	int  pnum;
	char GRA[WLEN];
	char word[WLEN];
	char stem[WLEN];
	char POS[WLEN];
	char MOR[WLEN];
	char changed;
	struct utt_s *next;
} ; 

struct errList_node {
	char POS[WLEN+1];
	struct errList_node *left;
	struct errList_node *right;
};

static char ftime;
static struct utt_s *root_utt;
static struct ROOT_POS *rootPOS;
static struct ROOT_GR *rootGR;
struct errList_node *errList;


void usage() {
	printf("Usage: connl2chat [%s] filename(s)\n",mainflgs());
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(TRUE);
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

static struct ROOT_POS *addToPOSConversion(struct ROOT_POS *root, char *oPOS, char *POS) {
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
		fprintf(stderr, "\n\nconnl2chat: out of memory\n\n");
		rootPOS = freePOSes(rootPOS);
		rootGR = freeGRs(rootGR);
		cutt_exit(0);
	}
	p->next = NULL;
	p->oPOS = NULL;
	p->POS = NULL;
	if ((p->oPOS=(char *)malloc(strlen(oPOS)+1)) == NULL) {
		fprintf(stderr, "\n\nconnl2chat: out of memory\n\n");
		rootPOS = freePOSes(rootPOS);
		rootGR = freeGRs(rootGR);
		cutt_exit(0);
	}
	strcpy(p->oPOS, oPOS);
	p->len = strlen(p->oPOS);
	if ((p->POS=(char *)malloc(strlen(POS)+1)) == NULL) {
		fprintf(stderr, "\n\nconnl2chat: out of memory\n\n");
		rootPOS = freePOSes(rootPOS);
		rootGR = freeGRs(rootGR);
		cutt_exit(0);
	}
	strcpy(p->POS, POS);
	return(root);
}

static void readPOSFile(void) {
	char *oPOS, *POS;
	FILE *fp;
	FNType mFileName[FNSize];
	
	if ((fp=OpenGenLib(POSFNAME,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the changes files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", POSFNAME, mFileName);
		cutt_exit(0);
	}
	fprintf(stderr, "    Using POS file: %s.\n", mFileName);
	lineno = 0L;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC,FONTHEADER,FALSE,FALSE) || templineC[0] == '#' || templineC[0] == '%')
			continue;
		lineno++;
		uS.remblanks(templineC);
		for (oPOS=templineC; isSpace(*oPOS); oPOS++) ;
		POS = strchr(oPOS, '\t');
		if (POS == NULL)
			POS = strchr(oPOS, ' ');
		if (POS == NULL) {
			fprintf(stderr,"*** File \"%s\": line %ld.\n", mFileName, lineno);
			fprintf(stderr, "Missing tab on line: %s\n", templineC);
			rootPOS = freePOSes(rootPOS);
			rootGR = freeGRs(rootGR);
			cutt_exit(0);
		}
		*POS = EOS;
		for (POS++; isSpace(*POS); POS++);
		rootPOS = addToPOSConversion(rootPOS, oPOS, POS);
	}
	fclose(fp);
}

static struct ROOT_GR *addToGRConversion(struct ROOT_GR *root, char *oGR, char *GR) {
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
		fprintf(stderr, "\n\nconnl2chat: out of memory\n\n");
		rootPOS = freePOSes(rootPOS);
		rootGR = freeGRs(rootGR);
		cutt_exit(0);
	}
	p->next = NULL;
	p->oGR = NULL;
	p->GR = NULL;
	if ((p->oGR=(char *)malloc(strlen(oGR)+1)) == NULL) {
		fprintf(stderr, "\n\nconnl2chat: out of memory\n\n");
		rootPOS = freePOSes(rootPOS);
		rootGR = freeGRs(rootGR);
		cutt_exit(0);
	}
	strcpy(p->oGR, oGR);
	if ((p->GR=(char *)malloc(strlen(GR)+1)) == NULL) {
		fprintf(stderr, "\n\nconnl2chat: out of memory\n\n");
		rootPOS = freePOSes(rootPOS);
		rootGR = freeGRs(rootGR);
		cutt_exit(0);
	}
	strcpy(p->GR, GR);
	return(root);
}

static void readGRFile(void) {
	char *oGR, *GR, *e;
	FILE *fp;
	FNType mFileName[FNSize];
	
	if ((fp=OpenGenLib(GRFNAME,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the changes files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", GRFNAME, mFileName);
		cutt_exit(0);
	}
	fprintf(stderr, "    Using GR file: %s.\n", mFileName);
	lineno = 0L;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC,FONTHEADER,FALSE,FALSE) || templineC[0] == '#' || templineC[0] == '%')
			continue;
		lineno++;
		uS.remblanks(templineC);
		for (oGR=templineC; isSpace(*oGR); oGR++);
		GR = strchr(oGR, '\t');
		if (GR == NULL)
			GR = strchr(oGR, ' ');
		if (GR == NULL) {
			fprintf(stderr,"*** File \"%s\": line %ld.\n", mFileName, lineno);
			fprintf(stderr, "Missing tab on line: %s\n", templineC);
			rootPOS = freePOSes(rootPOS);
			rootGR = freeGRs(rootGR);
			cutt_exit(0);
		}
		*GR = EOS;
		for (GR++; isSpace(*GR); GR++);
		for (e=GR; !isSpace(*e); e++);
		*e = EOS;
		rootGR = addToGRConversion(rootGR, oGR, GR);
	}
	fclose(fp);
}

static void errListPrint(struct errList_node *p) {
	if (p != NULL) {
		errListPrint(p->left);
		errListPrint(p->right);
		fprintf(stderr, "Unknown POS code: %s\n", p->POS);
	}
}

static struct errList_node *freeErrList(struct errList_node *p) {
	if (p != NULL) {
		freeErrList(p->left);
		freeErrList(p->right);
		free(p);
	}
	return(NULL);
}

void init(char s) {
	if (s) {
		ftime = TRUE;
		rootPOS = NULL;
		rootGR = NULL;
		root_utt = NULL;
		errList = NULL;
		stout = FALSE;
		onlydata = 3;
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
	} else {
		if (ftime) {
			readPOSFile();
			readGRFile();
			ftime = FALSE;
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP02;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	errList = NULL;
	bmain(argc,argv,NULL);
	rootPOS = freePOSes(rootPOS);
	rootGR = freeGRs(rootGR);
	errListPrint(errList);
	errList = freeErrList(errList);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
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

static struct utt_s *freeUtts(struct utt_s *p) {
	struct utt_s *t;
	
	while (p != NULL) {
		t = p;
		p = p->next;
		free(t);
	}
	return(NULL);
}

static struct utt_s *addUtt(struct utt_s *root, int cnum, int anum, int pnum,
							const char *GRA, const char *word, const char *stem, const char *POS, const char *MOR) {
	struct utt_s *p;

	if (root == NULL) {
		if ((p=NEW(struct utt_s)) == NULL)
			out_of_mem();
		root = p;
	} else {
		for (p=root; p->next != NULL; p=p->next) ;
		if ((p->next=NEW(struct utt_s)) == NULL)
			out_of_mem();
		p = p->next;
	}
	p->next = NULL;
	p->cnum = cnum;
	p->anum = anum;
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

static struct errList_node *addToErrList(struct errList_node *p, char *w) {
	int cond;

	if (p == NULL) {
		p = NEW(struct errList_node);
		if (p == NULL) {
			fprintf(stderr, "\n\nconnl2chat: out of memory\n\n");
			root_utt = freeUtts(root_utt);
			rootPOS = freePOSes(rootPOS);
			rootGR = freeGRs(rootGR);
			errList = freeErrList(errList);
			cutt_exit(0);
		}
		strncpy(p->POS, w, WLEN);
		p->POS[WLEN] = EOS;
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->POS)) == 0) {
	} else if (cond < 0) {
		p->left = addToErrList(p->left, w);
	} else {
		p->right = addToErrList(p->right, w);
	}
	return(p);
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
		errList = addToErrList(errList, oPOS);
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
	}
}

static void outputSp(struct utt_s *p) {
	strcpy(utterance->speaker, "*TXT:");
	utterance->line[0] = EOS;
	while (p != NULL) {
		if (p->cnum != 0) {
			if (utterance->line[0] != EOS)
				strcat(utterance->line, " ");
			strcat(utterance->line, p->word);
		}
		p = p->next;
	}
	if (utterance->line[0] != EOS)
		printout(utterance->speaker, utterance->line, NULL, NULL, TRUE);
}

static void outputMor(struct utt_s *p) {
	strcpy(utterance->speaker, "%mor:");
	utterance->line[0] = EOS;
	while (p != NULL) {
		if (p->cnum != 0) {
			if (utterance->line[0] != EOS)
				strcat(utterance->line, " ");
			if (getTruePOS(p->POS, templineC, p->stem)) {
// to output surface form instead of stem form
				strcpy(p->stem, p->word);
				if (isupper((unsigned char)p->stem[0]))
					p->stem[0] = (char)tolower((unsigned char)p->stem[0]);
// to output surface form instead of stem form
			}
			if (templineC[0] != EOS) {
				strcat(utterance->line, templineC);
				strcat(utterance->line, "|");
			}
			strcat(utterance->line, p->stem);
		}
		p = p->next;
	}
	if (utterance->line[0] != EOS)
		printout(utterance->speaker, utterance->line, NULL, NULL, TRUE);
}

static void outputGra(struct utt_s *p) {
	char numS[WLEN];

	strcpy(utterance->speaker, "%gra:");
	utterance->line[0] = EOS;
	while (p != NULL) {
		if (p->cnum != 0) {
			if (utterance->line[0] != EOS)
				strcat(utterance->line, " ");
			sprintf(numS, "%d", p->cnum);
			strcat(utterance->line, numS);
			strcat(utterance->line, "|");
			sprintf(numS, "%d", p->pnum);
			strcat(utterance->line, numS);
			strcat(utterance->line, "|");
			getTrueGR(p->GRA, templineC);
			strcat(utterance->line, templineC);
		}
		p = p->next;
	}
	if (utterance->line[0] != EOS)
		printout(utterance->speaker, utterance->line, NULL, NULL, TRUE);
}
/*
static void renumber(struct utt_s *root) {
	struct utt_s *p, *t;

	for (p=root; p != NULL; p=p->next) {
		for (t=root; t != NULL; t=t->next) {
			if (p->cnum == 0 && p->anum == t->pnum) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Line # %d is referenced by line # %d:\n", p->anum, t->anum);
				fprintf(stderr, "%d %s %s %d %s %s %s\n", p->anum, p->word, p->stem, p->pnum, p->GRA, p->POS, p->MOR);
				fprintf(stderr, "%d %s %s %d %s %s %s\n", t->anum, t->word, t->stem, t->pnum, t->GRA, t->POS, t->MOR);
				root_utt = freeUtts(root_utt);
				rootPOS = freePOSes(rootPOS);
				rootGR = freeGRs(rootGR);
				errList = freeErrList(errList);
				cutt_exit(0);
			} else if (p->cnum != p->anum && !t->changed && t->pnum == p->anum) {
				t->changed = TRUE;
				t->pnum = p->cnum;
			}
		}	
	}	
}
*/
static void checkUttDelAndOutput(struct utt_s *root) {
	struct utt_s *p;
	int  cnum, pnum;
	char isUttDelFound;

	cnum = 0;
	isUttDelFound = 0;
/*
	struct utt_s *pt;
	int  uttCnt;
	uttCnt = 0;
	for (p=root; p != NULL; p=p->next) {
		if (!strcmp(p->stem, ".") || !strcmp(p->stem, "!") || !strcmp(p->stem, "?")) {
			if (uttCnt > 0) {
				cnum = 0;
				for (pt=root; pt != NULL; pt=pt->next) {
					cnum++;
					pt->cnum = cnum;
				}
				renumber(root);
			}
			pt = p->next;
			p->next = NULL;
			outputSp(root);
			outputMor(root);
			outputGra(root);
			p->next = pt;
			root = p->next;
			uttCnt++;
			isUttDelFound = 1;
		} else
			isUttDelFound = 0;
	}
	if (!isUttDelFound) {
		if (uttCnt > 0) {
			cnum = 0;
			for (pt=root; pt != NULL; pt=pt->next) {
				cnum++;
				pt->cnum = cnum;
			}
			renumber(root);
		}
		pnum = 0;
		for (p=root; p->next != NULL; p=p->next) {
			if (!strcmp(p->GRA, "sentence"))
				pnum = p->cnum;
		}
		cnum = p->cnum + 1;
		root_utt = addUtt(root_utt, cnum, cnum, pnum, "f", ".", ".", "punc", "");
		outputSp(root);
		outputMor(root);
		outputGra(root);
	}
*/
	for (p=root; p != NULL; p=p->next) {
		if (!strcmp(p->stem, ".") || !strcmp(p->stem, "!") || !strcmp(p->stem, "?")) {
			isUttDelFound++;
		}
	}
	if (isUttDelFound == 0 && root != NULL) {
		pnum = 0;
		for (p=root; p->next != NULL; p=p->next) {
			if (!strcmp(p->GRA, "sentence"))
				pnum = p->cnum;
		}
		cnum = p->cnum + 1;
		root_utt = addUtt(root_utt, cnum, cnum, pnum, "f", ".", ".", "punc", "");
		isUttDelFound++;
	}
	if (isUttDelFound == 1) {
		outputSp(root);
		outputMor(root);
		outputGra(root);
	}
}

void call() {		/* this function is self-explanatory */
	int  col;
	int  cnum;
	int  anum;
	int  pnum;
	char GRA[WLEN];
	char word[WLEN];
	char stem[WLEN];
	char POS[WLEN];
	char MOR[WLEN];
	char *b, *e, t;
	
	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	eng\n");
	fprintf(fpout, "@Participants:\tTXT Text\n");
	fprintf(fpout, "@Options:\theritage\n");
	fprintf(fpout, "@ID:	eng|text|TXT|||||Text|||\n");
	lineno = 0L;
	cnum = 0;
	anum = 0;
	pnum = 0;
	GRA[0] = EOS;
	word[0] = EOS;
	stem[0] = EOS;
	POS[0] = EOS;
	MOR[0] = EOS;
	root_utt = NULL;
	while (fgets_cr(uttline, UTTLINELEN, fpin) != NULL) {
		lineno++;
		if (uS.partwcmp(uttline, UTF8HEADER) || uttline[0] == '#') {
			continue;
		}
		if (!isdigit(uttline[0])) {
			checkUttDelAndOutput(root_utt);
			root_utt = freeUtts(root_utt);
			cnum = 0;
		} else {
			cnum++;
			col = 0;
			b = uttline;
			while (TRUE) {
				col++;
				for (e=b; !isSpace(*e) && *e != '\n' && *e != EOS; e++) ;
				t = *e;
				*e = EOS;
				if (col == 1) {
					anum = atoi(b);
					if (anum == 0) {
						fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
						fprintf(stderr, "anum == 0\n");
						root_utt = freeUtts(root_utt);
						rootPOS = freePOSes(rootPOS);
						rootGR = freeGRs(rootGR);
						errList = freeErrList(errList);
						cutt_exit(0);
					}
				} else if (col == 2) {
					strcpy(word, b);
				} else if (col == 3) {
					strcpy(stem, b);
				} else if (col == 4) {
					strcpy(POS, b);
				} else if (col == 5) {
					pnum = atoi(b);
					if (!isdigit(*b)) {
						fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
						fprintf(stderr, "pnum == 0\n");
						root_utt = freeUtts(root_utt);
						rootPOS = freePOSes(rootPOS);
						rootGR = freeGRs(rootGR);
						errList = freeErrList(errList);
						cutt_exit(0);
					}
				} else if (col == 6) {
					strcpy(GRA, b);
				} else if (col == 7) {
				} else if (col == 8) {
					strcpy(MOR, b);
				}
				*e = t;
				if (isSpace(*e)) {
					for (; isSpace(*e); e++) ;
					b = e;
				} else
					break;
			}
			if (!strcmp(word, "_") && !strcmp(stem, "_")) {
				strcpy(word, "0sub");
				strcpy(stem, "0sub");
				strcpy(POS, "n");
				strcpy(GRA, "suj");
				root_utt = addUtt(root_utt, cnum, anum, pnum, GRA, word, stem, POS, MOR);
//				cnum--;
//				root_utt = addUtt(root_utt, 0, anum, pnum, GRA, word, stem, POS, MOR);
			} else
				root_utt = addUtt(root_utt, cnum, anum, pnum, GRA, word, stem, POS, MOR);
			anum = 0;
			pnum = 0;
			GRA[0] = EOS;
			word[0] = EOS;
			stem[0] = EOS;
			POS[0] = EOS;
			MOR[0] = EOS;
		}
	}
	if (root_utt != NULL) {
		checkUttDelAndOutput(root_utt);
		root_utt = freeUtts(root_utt);
		cnum = 0;
	}
	fprintf(fpout, "@End\n");
}
