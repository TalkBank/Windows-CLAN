/**********************************************************************
	"Copyright 1990-2017 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 2

#include "cu.h"

#if !defined(UNX)
#define _main modrep_main
#define call modrep_call
#define getflag modrep_getflag
#define init modrep_init
#define usage modrep_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define NONLINE '\0'
#define MODLINE '\001'
#define REPLINE '\002'
#define CURLINE '\003'
#define OTHLINE '\004'

#define REP struct rep_s
REP {
	char *word;
	unsigned int count;
	REP  *next_rep;
};

struct modrep_tnode {
	char *word;
	REP  *rep;
	unsigned int count;
	struct modrep_tnode *left;
	struct modrep_tnode *right;
} ;

extern struct IDtype *IDField;
extern struct IDtype *SPRole;

static char modrep_ftime, mor_specified;
static char isExcel;
static char isSort = FALSE;
static char ModWord[BUFSIZ], RepWord[BUFSIZ];
static char *OtherUtt, *OrgScopTier;
static char tf, *mainTierselected, ModLine[SPEAKERLEN], RepLine[SPEAKERLEN];
static char WordMode_b;
static char WordMode_c;
static struct modrep_tnode *modrep_root;
static IEWORDS *wdptr_b;
static IEWORDS *wdptr_c;
static EXCELCOL *rootexcel;

/*  *************** modrep prototypes **************** */

void modrep_overflow(void);
void modrep_treeprint(struct modrep_tnode *p);
void modrep_pr_result(void);
void AddRepWord(struct modrep_tnode *p, char *rw);
void ProcessLines(char WhichLine, char OtherSP, char zeroLine);
void ChoseProcessor(char WhichLine, char OtherSP, char zeroLine);
void getrefrange(int p1, char w);
void modrep_pars(char w, char *st, int p2, int p4);
void comppos(char, char *, int, int, int, int, int, int, int, int, int, int);
void maketree(char w, char *tu, char *ts, char *wd, int mid);
void modrep_shiftr(char *st, int num);
void setdot(char *s, int *b1, int *b2, char frstnum, char numf);
void free_modrep_tree(struct modrep_tnode *p);
void modrep_tree_cleanup(void);
char *modrep_MkStr(char *s);
char *getfirst(int p2, int p4, char *wd, char **kwd);
char *modrep_MkStrAndShift(char *s, int shift);
char *getnum(char *st, int *u, int *w, int *s);
int  modrep_exclude(char *word, IEWORDS *wdptr, char WordMode);
int  getneww(register char *word, register int i);
int  locword(char,char *,int,int,int,int,int,int,int,int,char *,int,int);
int  modrep_isskip(char *line, int pos, char MBC);
IEWORDS *modrep_addwdptr(char opt, char ch, char *wd, IEWORDS *wdptr, char *WordMode);
struct modrep_tnode *modrep_talloc(char *mw, char *rw);
struct modrep_tnode *modrep_tree(struct modrep_tnode *p, char *mw, char *rw);

/*  ********************************************************* */

void usage() {
	puts("MODREP matches words on the %mod line to replicas on the %pho line");
	printf("Usage: modrep [a bS cS nS oS sS %s] filename(s)\n", mainflgs());
	printf("+a : sort output by descending frequency\n");
	printf("+bS: set model tier name to S, i.e. +b*CHI or +b%%hes\n");
	printf("     S = \"*\", if you want to use \"+t@ID=\" option for speaker names\n");
	printf("+cS: set replica tier name to S, i.e. +c*CHI or +c%%hes\n");
	printf("     S = \"*\", if you want to use \"+t@ID=\" option for speaker names\n");
	printf("+d : outputs in SPREADSHEET format\n");
	printf("+nS: word S or in file @S included in an output assiciated with +c\n");
	printf("+oS: word S or in file @S included in an output assiciated with +b\n");
	printf("+sS: word S or in file @S included in an input\n");
	printf("-sS: word S or in file @S excluded from an input\n");
	mainusage(TRUE);
}

static void modrepError(void) {
	wdptr_b = freeIEWORDS(wdptr_b);
	wdptr_c = freeIEWORDS(wdptr_c);
	free(OtherUtt); free(OrgScopTier);
	modrep_tree_cleanup();
	cutt_exit(0);
}

void init(char first) {
	if (first) {
		isExcel = FALSE;
		isSort = FALSE;
		mor_specified = FALSE;
		modrep_ftime = TRUE;
		mainTierselected = NULL;
		*ModLine = EOS;
		*RepLine = EOS;
		rootexcel = NULL;
		modrep_root = NULL;
		wdptr_b = NULL;
		wdptr_c = NULL;
		WordMode_b = '\0';
		WordMode_c = '\0';
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
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
//		maininitwords();
		mor_initwords();
		addword('\0','\0',"+&*");
		addword('\0','\0',"++*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");		
		OtherUtt = (char *)malloc(UTTLINELEN+1);
		if (OtherUtt == NULL)
			out_of_mem();
		OrgScopTier = (char *)malloc(UTTLINELEN+1);
		if (OrgScopTier == NULL)
			out_of_mem();
	} else {
		if (modrep_ftime) {
			modrep_ftime = FALSE;
			if (!mor_specified)
				addword('\0','\0',"+0*");
			if (*ModLine == EOS) {
				fprintf(stderr, "Please specify model code tier with \"+b\" option.\n");
				modrepError();
			} else if (SPRole != NULL || IDField != NULL) {
				if (strcmp(ModLine, "*"))
					maketierchoice(ModLine,'+',FALSE);
			} else
				maketierchoice(ModLine,'+',FALSE);

			if (*RepLine == EOS) {
				fprintf(stderr, "Please specify replica code tier with \"+c\" option.\n");
				modrepError();
			} else if (SPRole != NULL || IDField != NULL) {
				if (strcmp(RepLine, "*"))
					maketierchoice(RepLine,'+',FALSE);
			} else
				maketierchoice(RepLine,'+',FALSE);
			if (isExcel) {
				stout = TRUE;
			}
		}
	}

	if (!combinput || first)
		modrep_root = NULL;
}

IEWORDS *modrep_addwdptr(char opt, char ch, char *wd, IEWORDS *wdptr, char *WordMode) {
	register int i;
	char plus = FALSE;
	IEWORDS *tempwd;

	for (i=strlen(wd)-1; wd[i]== ' ' || wd[i]== '\t' || wd[i]== '\n'; i--) ;
	wd[++i] = EOS;
	if (wd[0] == '+') {
		plus = TRUE;
		i = 1;
	} else
		i = 0;
	for (; wd[i] == ' ' || wd[i] == '\t'; i++) ;
	if (!*WordMode) {
		if (!plus)
			wdptr = freeIEWORDS(wdptr); 
		*WordMode = ch;
	} else if (*WordMode != ch) {
		if (opt == '\0')
			opt = ' ';
		fprintf(stderr, "-%c option can not be used with option +%c.\n", opt, opt);
		fprintf(stderr, "Offending word is: %s\n", wd+i);
		modrepError();
	}
	tempwd = NEW(IEWORDS);
	if (tempwd == NULL) {
		fprintf(stderr,"Memory ERROR. Use less include/exclude words.\n");
		cutt_exit(1);
	} else {
		tempwd->nextword = wdptr;
		wdptr = tempwd;
		wdptr->word = (char *)malloc(strlen(wd+i)+1);
		if(wdptr->word == NULL) {
			fprintf(stderr,"No more space left in core.\n");
			cutt_exit(1);
		}
		if (!nomap)
			uS.lowercasestr(wd+i, &dFnt, C_MBF);
		strcpy(wdptr->word, wd+i);
	}
	return(wdptr);
}

static IEWORDS *modrep_rdexclf(char opt, char ch, FNType *fname, IEWORDS *wdptr, char *WordMode) {
	FILE *efp;
	char wd[512];

	if (*fname == '+') {
		*wd = '+';
		fname++;
	} else
		*wd = ' ';

	if (*fname == EOS) {
		fprintf(stderr, "No file name for the +o option specified!\n");
		modrepError();
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(wd_dir);
#endif
	if ((efp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "Can not access %s file: %s\n", ((ch=='i') ? "include" : "exclude"), fname);
		modrepError();
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
#endif
	while (fgets_cr(wd+1, 511, efp)) {
		if (uS.isUTF8(wd+1) || uS.partcmp(wd+1, FONTHEADER, FALSE, FALSE))
			continue;
		if (wd[1] == ';' && wd[2] == '%' && wd[3] == '*' && wd[4] == ' ')
			continue;
		if (wd[1] == '#' && wd[2] == ' ')
			continue;
		wdptr = modrep_addwdptr(opt, ch, wd, wdptr, WordMode);
	}
	return(wdptr);
}

void free_modrep_tree(struct modrep_tnode *p) {
	struct modrep_tnode *t;
	REP *r, *tr;

	if (p != NULL) {
		free_modrep_tree(p->left);
		do {
			r = p->rep;
			while (r != NULL) {
				tr = r;
				r = r->next_rep;
				if (tr->word) free(tr->word);
				free(tr);
			}
			if (p->right == NULL) break;
			if (p->right->left != NULL) {
				free_modrep_tree(p->right);
				break;
			}
			t = p;
			p = p->right;
			if (t->word) free(t->word);
			free(t);
		} while (1);
		if (p->word) free(p->word);
		free(p);
	}
}

void modrep_tree_cleanup() {
	free_modrep_tree(modrep_root);
	modrep_root = NULL;
}

int modrep_exclude(char *word, IEWORDS *wdptr, char WordMode) {
	IEWORDS *twd;

	for (twd=wdptr; twd != NULL; twd = twd->nextword) {
		if (uS.patmat(word, twd->word)) {
			if (WordMode == 'i')
				return(TRUE);
			else
				return(FALSE);
		}
	}
	if (WordMode == 'i')
		return(FALSE);
	else
		return(TRUE);
}

			/* allocate space for string of text */
char *modrep_MkStr(char *s) {
	char *p;

	if ((p = (char *)malloc(strlen(s)+1)) != NULL) strcpy(p, s);
	else modrep_overflow();
	return(p);
}

void AddRepWord(struct modrep_tnode *p, char *rw) {
	REP *t;

	p->count++;
	t = p->rep;
	while (1) {
		if (!strcmp(t->word,rw)) {
			t->count++;
			return;
		}
		if (t->next_rep == NULL) break;
		t = t->next_rep;
	}
	if ((t->next_rep=NEW(REP)) == NULL) modrep_overflow();
	t = t->next_rep;
	t->word = modrep_MkStr(rw);
	t->count = 1;
	t->next_rep = NULL;
}

void modrep_overflow() {
	fprintf(stderr,"Modrep: no more core available.\n");
	cutt_exit(1);
}

struct modrep_tnode *modrep_talloc(char *mw, char *rw) {
	struct modrep_tnode *p;

	if ((p = NEW(struct modrep_tnode)) == NULL) modrep_overflow();
	p->word = mw;
	p->count = 1;
	if ((p->rep=NEW(REP)) == NULL) modrep_overflow();
	p->rep->word  = rw;
	p->rep->count = 1;
	p->rep->next_rep = NULL;
	return(p);
}

struct modrep_tnode *modrep_tree(struct modrep_tnode *p, char *mw, char *rw) {
	int cond;
	struct modrep_tnode *t = p;

	 if (p == NULL) {
		p = modrep_talloc(modrep_MkStr(mw),modrep_MkStr(rw));
		p->left = p->right = NULL;
	} else if ((cond=strcmp(mw,p->word)) == 0) AddRepWord(p,rw);
	else if (cond < 0) p->left = modrep_tree(p->left, mw, rw);
	else {
		while ((cond=strcmp(mw,p->word)) > 0 && p->right != NULL) p=p->right;
		if (cond == 0) AddRepWord(p,rw);
		else if (cond < 0) p->left = modrep_tree(p->left, mw, rw);
		else p->right = modrep_tree(p->right, mw, rw);
		return(t);
	}
	return(p);
}


static void addExcelSp_KeywordRows(char *fnameOrg, char *sp, char *keyword, int count) {
	char *fname;
	KEYWDROW *r;

	if (rootexcel == NULL) {
		rootexcel = NEW(EXCELCOL);
		if (rootexcel == NULL)
			out_of_mem();
		rootexcel->keyword = NULL;
		rootexcel->keyrows = NEW(KEYWDROW);
		rootexcel->rows = NULL;
		rootexcel->nextcol = NULL;
		r = rootexcel->keyrows;
	} else {
		for (r=rootexcel->keyrows; r->nextrow != NULL; r=r->nextrow) ;
		r->nextrow = NEW(KEYWDROW);
		r = r->nextrow;
	}
	if (r == NULL)
		out_of_mem();
	// ".xls"
	fname = strrchr(fnameOrg, PATHDELIMCHR);
	if (fname != NULL)
		fname = fname + 1;
	else
		fname = fnameOrg;
	r->fname = (char *)malloc((size_t) strlen(fname)+1);
	if (r->fname == NULL)
		out_of_mem();
	strcpy(r->fname, fname);
	r->sp = (char *)malloc((size_t) strlen(sp)+1);
	if (r->sp == NULL)
		out_of_mem();
	strcpy(r->sp, sp);
	r->keyword = (char *)malloc((size_t) strlen(keyword)+1);
	if (r->keyword == NULL)
		out_of_mem();
	strcpy(r->keyword, keyword);
	r->count = count;
	r->rowCnt = ExcelCnt;
	r->nextrow = NULL;
}

static EXCELROW *addExcelRows(EXCELROW *root, int count) {
	EXCELROW *p;

	if (root == NULL) {
		root = NEW(EXCELROW);
		p = root;
	} else {
		for (p=root; p->nextrow != NULL; p=p->nextrow) ;
		p->nextrow = NEW(EXCELROW);
		p = p->nextrow;
	}
	if (p == NULL)
		out_of_mem();
	p->count = count;
	p->rowCnt = ExcelCnt;
	p->nextrow = NULL;
	return(root);
}

static void addExcelCols_Rows(char *sp, char *keyword, int count) {
	EXCELCOL *tkey, *tkey2, *new_key;

	//	if (sp[0] == '*')
	//		sp++;
	strcpy(spareTier1, sp);
	strcat(spareTier1, ":");
	strcat(spareTier1, keyword);
	if (rootexcel->nextcol == NULL) {
		rootexcel->nextcol = NEW(EXCELCOL);
		if (rootexcel->nextcol == NULL)
			out_of_mem();
		tkey = rootexcel->nextcol;
		tkey->keyrows = NULL;
		tkey->rows = NULL;
		tkey->nextcol = NULL;
	} else {
		tkey = rootexcel->nextcol;
		tkey2 = NULL;
		while (1) {
			if (!strcmp(spareTier1, tkey->keyword)) {
				tkey->rows = addExcelRows(tkey->rows, count);
				return;
			}
			if (strcmp(spareTier1,tkey->keyword) < 0) {
				if ((new_key=NEW(EXCELCOL)) == NULL)
					out_of_mem();
				new_key->keyrows = NULL;
				new_key->rows = NULL;
				new_key->nextcol = tkey;
				if (tkey2 == NULL)
					rootexcel->nextcol = new_key;
				else
					tkey2->nextcol = new_key;
				tkey = new_key;
				break;
			}
			if (tkey->nextcol == NULL) {
				if ((tkey->nextcol=NEW(EXCELCOL)) == NULL)
					out_of_mem();
				tkey = tkey->nextcol;
				tkey->keyrows = NULL;
				tkey->rows = NULL;
				tkey->nextcol = NULL;
				break;
			}
			tkey2 = tkey;
			tkey = tkey->nextcol;
		}
	}
	tkey->keyword = (char *)malloc((size_t) strlen(spareTier1)+1);
	if (tkey->keyword == NULL)
		out_of_mem();
	strcpy(tkey->keyword, spareTier1);
	tkey->rows = addExcelRows(tkey->rows, count);
}

void modrep_treeprint(struct modrep_tnode *p) {
	struct modrep_tnode *t;
	REP *r, *tr;

	if (p != NULL) {
		modrep_treeprint(p->left);
		do {
			if (!isExcel)
				fprintf(fpout,"%3u %s\n", p->count, p->word);
			else
				addExcelSp_KeywordRows(oldfname, tsp1->sp, key1->mapkeyname, key1->count);
			r = p->rep;
			while (r != NULL) {
				if (!isExcel)
					fprintf(fpout,"    %3u %s\n", r->count, r->word);
				else
					addExcelCols_Rows(tsp2->sp, key2->mapkeyname, key2->count);
				tr = r;
				r = r->next_rep;
				if (tr->word) free(tr->word);
				free(tr);
			}
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				modrep_treeprint(p->right);
				break;
			}
			t = p;
			p = p->right;
			if (t->word)
				free(t->word);
			free(t);
		} while (1);
		if (p->word)
			free(p->word);
		free(p);
	}
}

static struct modrep_tnode *modrep_salloc(char *mw, unsigned int count, REP *rep) {
	struct modrep_tnode *p;

	if ((p = NEW(struct modrep_tnode)) == NULL) modrep_overflow();
	p->word = mw;
	p->count = count;
	p->rep = rep;
	return(p);
}

static struct modrep_tnode *modrep_stree(struct modrep_tnode *p, struct modrep_tnode *m) {
	struct modrep_tnode *t = p;

	if (p == NULL) {
		p = modrep_salloc(modrep_MkStr(m->word), m->count, m->rep);
		p->left = p->right = NULL;
	} else if (p->count < m->count)
		p->left = modrep_stree(p->left, m);
	else if (p->count == m->count) {
		for (; p->count>= m->count && p->right!= NULL; p=p->right) ;
		if (p->count < m->count)
			p->left = modrep_stree(p->left, m);
		else
			p->right = modrep_stree(p->right, m);
		return(t);
	} else
		p->right = modrep_stree(p->right, m);
	return(p);
}

static void modrep_sorttree(struct modrep_tnode *p) {
	if (p != NULL) {
		modrep_sorttree(p->left);
		modrep_root = modrep_stree(modrep_root,p);
		modrep_sorttree(p->right);
		free(p->word);
		free(p);
	}
}

void modrep_pr_result(void) {
	if (isSort) {
		struct modrep_tnode *p;
		p = modrep_root;
		modrep_root = NULL;
		modrep_sorttree(p);
		modrep_treeprint(modrep_root);
	} else
		modrep_treeprint(modrep_root);
}

int getneww(register char *word, register int i) {
	register char sq;

	while ((*word=OtherUtt[i++]) != EOS && uS.isskip(OtherUtt,i-1,&dFnt,MBF) && !uS.isRightChar(OtherUtt, i-1, '[', &dFnt, MBF)) ;
	if (*word == EOS) return(0);
	if (*word == '[') sq = TRUE;
	else sq = FALSE;
	while ((*++word=OtherUtt[i++]) != EOS && (!uS.isskip(OtherUtt,i-1,&dFnt,MBF) || sq)) {
		if (*word == ']') { *++word = EOS; return(i); }
	}
	if (!*word || *word == '[') i--;
	else *word = EOS;
	return(i);
}

void ProcessLines(char WhichLine, char OtherSP, char zeroLine) {
	const char *punc;
	int wi1 = 0L, wi2 = 0L;

	while (1) {
		if (WhichLine == MODLINE) {
			if (OtherSP == '*') {
				punc = punctuation;
				punctuation = GlobalPunctuation;
				wi1 = getneww(ModWord,wi1);
				punctuation = punc;
			} else
				wi1 = getneww(ModWord,wi1);
			wi2 = getword(utterance->speaker, uttline, RepWord, NULL, wi2);
			if (zeroLine != NONLINE) {
				if (zeroLine == OTHLINE && wi2 != 0 && wi1 == 0) {
					wi1 = strlen(OtherUtt);
					strcpy(ModWord, "0");
				} else if (zeroLine == CURLINE && wi1 != 0 && wi2 == 0) {
					wi2 = strlen(uttline);
					strcpy(RepWord, "0");
				}
			}
		} else {
			wi1 = getword(utterance->speaker, uttline, ModWord, NULL, wi1);
			if (OtherSP == '*') {
				punc = punctuation;
				punctuation = GlobalPunctuation;
				wi2 = getneww(RepWord,wi2);
				punctuation = punc;
			} else
				wi2 = getneww(RepWord,wi2);
			if (zeroLine != NONLINE) {
				if (zeroLine == OTHLINE && wi1 != 0 && wi2 == 0) {
					wi2 = strlen(OtherUtt);
					strcpy(RepWord, "0");
				} else if (zeroLine == CURLINE && wi2 != 0 && wi1 == 0) {
					wi1 = strlen(uttline);
					strcpy(ModWord, "0");
				}
			}
		}

		if (wi1 == 0 || wi2 == 0) {
			if (WhichLine == MODLINE) {
				if (wi1 != 0) {
					fprintf(stderr, "Model line:\n%s\nis longer than Rep line:\n%s\n", OtherUtt, uttline);
					fprintf(stderr, "In File \"%s\" in tier cluster around line %ld.\n", oldfname, lineno);
					modrepError();
				} else if (wi2 != 0) {
					fprintf(stderr, "Replica line:\n%s\nis longer than Model line:\n%s\n", uttline, OtherUtt);
					fprintf(stderr, "In File \"%s\" in tier cluster around line %ld.\n", oldfname, lineno);
					modrepError();
				} else
					break;
			} else {
				if (wi1 != 0) {
					fprintf(stderr, "Model line:\n%s\nis longer than Rep line:\n%s\n", uttline, OtherUtt);
					fprintf(stderr, "In File \"%s\" in tier cluster around line %ld.\n", oldfname, lineno);
					modrepError();
				} else if (wi2 != 0) {
					fprintf(stderr, "Replica line:\n%s\nis longer than Model line:\n%s\n", OtherUtt, uttline);
					fprintf(stderr, "In File \"%s\" in tier cluster around line %ld.\n", oldfname, lineno);
					modrepError();
				} else
					break;
			}
		}
		uS.remblanks(ModWord);
		uS.remblanks(RepWord);
		if (modrep_exclude(ModWord, wdptr_b, WordMode_b) && modrep_exclude(RepWord, wdptr_c, WordMode_c)) {
			modrep_root = modrep_tree(modrep_root, ModWord, RepWord);
		}
	}
}

void modrep_shiftr(char *st, int num) {
	register int i;
 
	for (i=strlen(st); i >= 0; i--) st[i+num] = st[i];
}

void maketree(char w, char *tu, char *ts, char *wd, int mid) {
	if (mid == -1)
		tu = modrep_MkStrAndShift(tu,0);
	else {
		tu = modrep_MkStrAndShift(tu,1);
		if (mid > (int)strlen(tu)) mid = strlen(tu);
		modrep_shiftr(tu+mid,1);
		tu[mid] = '|';
	}
	if (w == MODLINE) {
		uS.remblanks(wd);
		uS.remblanks(tu);
		if (modrep_exclude(wd, wdptr_b, WordMode_b) && modrep_exclude(tu, wdptr_c, WordMode_c)) {
			modrep_root = modrep_tree(modrep_root, wd, tu);
		}
	} else {
		uS.remblanks(tu);
		uS.remblanks(wd);
		if (modrep_exclude(tu, wdptr_b, WordMode_b) && modrep_exclude(wd, wdptr_c, WordMode_c)) {
			modrep_root = modrep_tree(modrep_root, tu, wd);
		}
	}
}
 
char *modrep_MkStrAndShift(char *s, int shift) {
	int i, col;
	char *p;
 
	if ((p = (char *)malloc(strlen(s)+1+shift)) != NULL) {
		i = 0;
		col = 0;
		while (*s != EOS) {
			if (*s == ' ' || *s == '\t') {
				if (*s == '\t') *s = ' ';
				if (col >= 60) { *s = '\n'; col = 0; }
				p[i++] = *s++;
			} else if (*s == '\n') {
				if (col < 60) { if (p[i-1] != ' ' && i > 0) p[i++]= ' '; }
				else { p[i++] = *s; col = 0; }
				for (s++; *s == ' ' || *s == '\t'; s++) ;
			} else p[i++] = *s++;
			col++;
		}
		for (i--; p[i] == ' ' && i >= 0; i--) ;
		p[++i] = *s;
	} else out_of_mem();
	return(p);
}
 
char *getfirst(int p2, int p4, char *wd, char **kwd) {
	int f1, l1, t;
	char *ts, s;
 
/*	for(f1=p2; uS.isskip(OrgScopTier,f1,&dFnt,MBF); f1++);
*/
	for (f1=p2; OrgScopTier[f1]==' ' || OrgScopTier[f1]=='\t'; f1++) ;
	p2 = f1 - 1;
	do {
		for (p2++; OrgScopTier[p2]!='$' && OrgScopTier[p2]!='\n' &&
				   OrgScopTier[p2] != '<' && p2 < p4; p2++) ;
	} while (OrgScopTier[p2] == '$' && p2 > 0 && 
			 !uS.isskip(OrgScopTier,p2-1,&dFnt,MBF) && OrgScopTier[p2-1]) ;
	if (*wd == '$' && !tf) {
		for (l1=p2-1; uS.isskip(OrgScopTier,l1,&dFnt,MBF) && l1 >= f1; l1--) ;
		l1++;
		if (f1 == l1) tf = 1;
		else {
			t = f1;
/*
			if (uS.partcmp(utterance->speaker,"%err",FALSE,FALSE)) {
				for (; OrgScopTier[t] != '>' && t < l1; t++) ;
				if (t < l1) {
					for (t++; uS.isskip(OrgScopTier,t,&dFnt,MBF) && t < l1; t++) ;
					if (t >= l1) t = f1;
					else for (l1=t+1; !uS.isskip(OrgScopTier,l1,&dFnt,MBF); l1++) ;
				} else t = f1;
			}
*/
			s = OrgScopTier[l1];
			OrgScopTier[l1] = EOS;
			*kwd = modrep_MkStrAndShift(OrgScopTier+t,0);
			OrgScopTier[l1] = s;
		}
	}
	if (OrgScopTier[p2] == '$' && p2 < p4) {
		for (p2++; !uS.isskip(OrgScopTier,p2,&dFnt,MBF) && OrgScopTier[p2]; p2++) ;
		while (1) {
			if (p2 >= p4 || OrgScopTier[p2] == '<') break;
			for (; uS.isskip(OrgScopTier,p2,&dFnt,MBF) && p2 < p4 &&
										OrgScopTier[p2] != '<'; p2++) ;
			if (p2 >= p4 || OrgScopTier[p2] == '<') break;
			if (OrgScopTier[p2] != '$') break;
			for (; !uS.isskip(OrgScopTier,p2,&dFnt,MBF) && OrgScopTier[p2]; p2++) ;
		}
	} else {
		t = f1;
		l1 = p2;
/*
		if (uS.partcmp(utterance->speaker,"%err",FALSE,FALSE)) {
			for (; OrgScopTier[t] != '>' && t < l1; t++) ;
			if (t < l1) {
				for (t++; uS.isskip(OrgScopTier,t,&dFnt,MBF) && t < l1; t++) ;
				if (t >= l1) t = f1;
				else for (l1=t+1; !uS.isskip(OrgScopTier,l1,&dFnt,MBF); l1++) ;
			} else t = f1;
		}
*/
		if (t == f1) {
			for (l1--; uS.isskip(OrgScopTier,l1,&dFnt,MBF) && l1 >= t; l1--) ;
			l1++;
		}
		s = OrgScopTier[l1];
		OrgScopTier[l1] = EOS;
		strcpy(wd,OrgScopTier+t);
		OrgScopTier[l1] = s;
	}
	for (p2--; uS.isskip(OrgScopTier,p2,&dFnt,MBF) && p2 >= f1; p2--) ;
	p2++;
	s = OrgScopTier[p2];
	OrgScopTier[p2] = EOS;
	ts = modrep_MkStrAndShift(OrgScopTier+f1,0);
	OrgScopTier[p2] = s;
	return(ts);
}

int modrep_isskip(char *line, int pos, char MBC) {
	if (uS.IsUtteranceDel(line, pos)) return(1);
	return(uS.isskip(line, pos, &dFnt, MBC));
}

int locword(char w, char *ts, 
			int u1, int w1, int s1, int b1, int u2, int w2, int s2, int b2, 
			char *wd, int p2, int p4) {
	int f1, l1, mid = -1;
	char s, *kwd, *kts, ww1;

/* get text between codes from code line */
	kwd = wd;
	if (ts == NULL) {
		ts = getfirst(p2,p4,wd,&kwd);
/*
		if (interturn) {
			InsertInterTurn(interturn,ts,wd);
			interturn = 0;
		}
*/
	}
 
/* find first position */
	*utterance->speaker = '*';
	f1 = 0;
	if (!u1 && !w1 && !s1 && !u2 && !w2 && !s2) {
		l1 = strlen(OtherUtt);
		for (l1--; OtherUtt[l1] == '\n' && l1 >= f1; l1--) ;
		l1++;
		goto nextcom;
	}
	if (b1 == 2 && !s1 && !w1) u1++;
	if (u1 != 0) {
		u1--;
		for (; u1 > 0; f1++) {
			if (uS.IsUtteranceDel(OtherUtt, f1)) u1--;
			else if (OtherUtt[f1] == EOS) {
				if (w1 || s1 || !b1 || u2 || w2 || s2) return(FALSE);
				else break;
			}
		}
		while (modrep_isskip(OtherUtt,f1,MBF) && OtherUtt[f1] != EOS) f1++;
		if (OtherUtt[f1] == EOS) {
			if (w1 || s1 || !b1 || u2 || w2 || s2) return(FALSE);
		}
	}
	if (!w1 && !s1 && !u2 && !w2 && !s2) {
		if (b1 && !u2 && !w2 && !s2 && !b2) l1 = f1;
		else {
			if (OtherUtt[f1] == EOS) l1 = f1;
			else {
				for (l1=f1+1; !uS.IsUtteranceDel(OtherUtt, l1) && 
					OtherUtt[l1] != EOS && OtherUtt[l1] != '\n'; l1++) ;
				for (l1--; modrep_isskip(OtherUtt,l1,MBF) && l1 >= f1; l1--) ;
				l1++;
			}
		}
		goto nextcom;
	}
	if (b1 == 2 && !s1 && w1) w1++;
	s = 0;
	if (w1) ww1 = (char)TRUE; else ww1 = (char)FALSE;
	while (1) {
		if (!modrep_isskip(OtherUtt,f1,MBF) && OtherUtt[f1]!=EOS) {
			if (!s) w1--;
			s = 1;
		} else
		if (OtherUtt[f1] == EOS) return(FALSE);
		else s = 0;
		if (w1 <= 0) break;
		else f1++;
	}
	if (b1 == 2 && s1) s1++;
	for(; s1>1 && !modrep_isskip(OtherUtt,f1,MBF); s1--) f1++;
	if (modrep_isskip(OtherUtt,f1,MBF)) return(FALSE);

/* find last position */
	if (b1 && !u2 && !w2 && !s2 && !b2) l1 = f1;
	else
	if (s1 != 0) {
		if (b2 == 1) s2--;
		for (l1=f1+1; s2 > 0 && !modrep_isskip(OtherUtt,f1,MBF); s2--) l1++;
	} else {
		l1 = f1;
		if (b1 != 2) for (; !modrep_isskip(OtherUtt,l1,MBF); l1++) ;
 
		if (u2 > 0 && (b1 != 2 || ww1)) {
			for (; !uS.IsUtteranceDel(OtherUtt, l1) && OtherUtt[l1]; l1++) ;
			if (OtherUtt[l1] == EOS) return(FALSE);
			l1++;
		}
		if (b2 == 1 || w2 || s2) u2--;
		for (; u2 > 0; l1++) {
			if (uS.IsUtteranceDel(OtherUtt, l1)) u2--;
			else if (OtherUtt[l1] == EOS) {
				u2--;
				if (u2 || w2 != 0 || s2 != 0) return(FALSE);
			}
		}
		s = 1;
		while (modrep_isskip(OtherUtt,l1,MBF) && OtherUtt[l1] != EOS) l1++;
		if (b2 == 1 && !s2 && w2) w2--;
		if (w2 && OtherUtt[l1] == EOS) return(FALSE);
		while (1) {
			if (!modrep_isskip(OtherUtt,l1,MBF) && OtherUtt[l1] != EOS) {
				if (!s) w2--;
				s = 1;
			} else
			if (OtherUtt[l1] == EOS) {
				if (w2-1 <= 0) break;
				else return(FALSE);
			} else s = 0;
			if (w2 <= 0) break;
			else l1++;
		}
		for (l1--; modrep_isskip(OtherUtt,l1,MBF) && l1 >= f1; l1--) ;
		l1++;
	}
nextcom:
/* get text from speaker line */
	if (f1 >= l1) {
		s = 0;
		mid = f1;
		l1 = f1+1;
		do {
			f1--;
			for (; !uS.IsUtteranceDel(OtherUtt, f1) && f1 >= 0; f1--) {
				if (!modrep_isskip(OtherUtt,f1,MBF)) s = 1;
			}
			if (!s && f1 < 0) s = 1;
		} while (!s) ;
		for (f1++; modrep_isskip(OtherUtt,f1,MBF) && f1 < l1; f1++) ;
		mid -= f1;
		for (; !uS.IsUtteranceDel(OtherUtt, l1) && OtherUtt[l1] && OtherUtt[l1] != '\n'; l1++) ;
	}
	*utterance->speaker = '%';
	s = OtherUtt[l1];
	OtherUtt[l1] = EOS;
	wd = modrep_MkStrAndShift(wd,0);
	if (*wd == '$' && !tf) {
		tf = 1;
		kts = modrep_MkStrAndShift(ts,0);
		maketree(w,OtherUtt+f1,ts,wd,mid);
		if (kwd != wd) maketree(w,OtherUtt+f1,kts,kwd,mid);
	} else maketree(w,OtherUtt+f1,ts,wd,mid);
	OtherUtt[l1] = s;
	return(TRUE);
}
 
static void modrep_err(char *s, const char *s1) {
	s--;
	if (s1 != NULL) fprintf(stderr,"%s in: %s\n",s1,s);
	else fprintf(stderr,"Illegal location representation: %s\n",s);
	wdptr_b = freeIEWORDS(wdptr_b);
	wdptr_c = freeIEWORDS(wdptr_c);
	free(OtherUtt); free(OrgScopTier);
	modrep_tree_cleanup();
	cutt_exit(0);
}
 
void comppos(char w, char *s, int u1, int w1, int s1, int b1, 
					int u2, int w2, int s2, int b2, int p2, int p4) {
	if (u2 != 0) {
		if (u1 == 0) u1 = 1;
		u2 -= u1;
	}
	if (u2 < 0) 
		modrep_err(s,"Second utterance must be located after the first one");
	if (u2 == 0) {
		if (w2 != 0) {
			if (w1 == 0) w1 = 1;
			w2 -= w1;
		}
		if (w2 < 0) 
			modrep_err(s,"Second word must be located after the first one");
	}
	if (s1 != 0 && (w2 != 0 || u2 != 0))
		modrep_err(s,"The second segment must be in the same word");
	if (s1 != 0 && s2 != 0) {
		s2 -= s1;
		if (s2<0) 
		   modrep_err(s,"Second segment must be located after the first one");
	}
/*
printf("*** s=%s",s-1);
printf("u1=%d; w1=%d; s1=%d; b1=%d; u2=%d; w2=%d; s2=%d; b2=%d;\n",
u1,w1,s1,b1,u2,w2,s2,b2);
printf("ModWord=%s; OrgScopTier+p2=%sOrgScopTier+p4=%s\n", 
	ModWord, OrgScopTier+p2, OrgScopTier+p4);
puts("*****");
*/
	if (!locword(w,NULL,u1,w1,s1,b1,u2,w2,s2,b2,ModWord,p2,p4)) {
		if (s[strlen(s)-1] == '\n')
			s[strlen(s)-1] = EOS;
		fprintf(stderr,"ERROR in scoping of: %s\n    to line: %s", s-1, OtherUtt);
		modrepError();
	}
}
 
char *getnum(char *st, int *u, int *w, int *s) {
	char *f, *l, t, *dot, sf = 0, uf = 0;
	int lev = 0;
 
	f = st;
	l = f;
	dot = NULL;
	while (*st != '>' && *st != ',' && *st != '-') {
		if (isdigit(*st)) {
			if (dot) return(NULL);
			lev++;
			f = st;
			for (; isdigit(*st); st++) ;
			l = st;
		} else {
			if (*st == '.')  dot = st;
			else
			if (*st == 'u') {
				if (dot) return(NULL);
				if (lev == 1 && !uf) {
					t = *l;
					*l = EOS;
					*u = atoi(f);
					*l = t;
					f = l;
				} else return(NULL);
				uf = 1;
			} else
			if (*st == 's') {
				if (dot) return(NULL);
				if (sf) return(NULL);
				if (lev == 1 || lev == 2) {
					t = *l;
					*l = EOS;
					*w = atoi(f);
					*l = t;
					f = l;
				} else return(NULL);
				sf = 1;
			}
			st++;
		}
	}
	if (lev < 1 || (lev > 2 && !sf) || lev > 3) return(NULL);
	if (f != l) {
		t = *l;
		*l = EOS;
		if (sf) *s = atoi(f);
		else *w = atoi(f);
		*l = t;
	}
	if (dot) return(dot);
	else return(st);
}
 
void setdot(char *s, int *b1, int *b2, char frstnum, char numf) {
	if (frstnum) {
		if (*b1) modrep_err(s,"Only one dot '.' allowed");
		if (!numf) *b1 = 1; else *b1 = 2;
	} else {
		if (*b2) modrep_err(s,"Only one dot '.' allowed");
		if (!numf) *b2 = 1; else *b2 = 2;
	}
}
 
void modrep_pars(char w, char *st, int p2, int p4) {
	char cf = 0, numf = 0, frstnum = 1;
	char *s;
	int u1, w1, s1, b1, u2, w2, s2, b2;
 
	u1 = 0; w1 = 0; s1 = 0; b1 = 0; u2 = 0; w2 = 0; s2 = 0; b2 = 0;
	if (*st == '<') {
		st++;
		s = st;
		while (*st != '>') {
			if (isdigit(*st)) {
				if (cf) 
					modrep_err(s,"Illegal characters found before numbers");
				numf = 1;
/*
				if (interturn) {
					interturn = atoi(st);
					for (; isdigit(*st); st++) ;
					for (; *st == ' ' || *st == '\t'; st++) ;
					if (*st != '>') modrep_err(s,"Illegal scoping");
				} else
*/
				if (frstnum) {
					if ((st=getnum(st,&u1,&w1,&s1)) == NULL) 
						modrep_err(s,NULL);
				} else {
					if ((st=getnum(st,&u2,&w2,&s2)) == NULL) 
						modrep_err(s,NULL);
				}
			} else {
				if (*st == '.') setdot(s,&b1,&b2,frstnum,numf);
				else
				if (*st == '-') {
					if (!numf) 
						modrep_err(s,"No position representation found");
					numf = 0;
					if (frstnum) frstnum = 0;
					else modrep_err(s,"Too many '-' symbols");
				} else
				if (*st == ',') {
					if (!numf) 
						modrep_err(s,"No position representation found");
					comppos(w,s,u1,w1,s1,b1,u2,w2,s2,b2,p2,p4);
					numf = 0; frstnum = 1;
					u1=0; w1=0; s1 = 0; b1 = 0; u2=0; w2 = 0; s2 = 0; b2 = 0;
				} else
				if (*st == 'a' && *(st+1) == 'f' && *(st+2) == 't') {
					if (numf) modrep_err(s,"<aft> can't be after any number");
					numf = 1;
					setdot(s,&b1,&b2,frstnum,numf);
					st += 2;
				} else
				if (*st == 'b' && *(st+1) == 'e' && *(st+2) == 'f') {
					setdot(s,&b1,&b2,frstnum,numf);
					numf = 1;
					st += 2;
				} else
				if (*st == '$' && *(st+1) == '/' && *(st+2) == '=') {
/* interturn = 1; */
					st += 2;
				} else
				if (*st != ' ' && *st != '\t' && *st != '\n') cf = 1;
				st++;
			}
		}
		if (!numf) modrep_err(s,"No position representation found");
	} else s = st;
	comppos(w,s,u1,w1,s1,b1,u2,w2,s2,b2,p2,p4);
}
 
void getrefrange(int p1, char w) {
	int i, p2, p3, p4;
	char wf = 0;

	tf = 0;
/*interturn = 0; */
	for (; OrgScopTier[p1]==' ' || OrgScopTier[p1]=='\t' || OrgScopTier[p1]=='\n'; p1++) ;
	if (OrgScopTier[p1] != EOS) {
		for (p4=p1+1; OrgScopTier[p4] != EOS && OrgScopTier[p4] != '<'; p4++) ;
		if (OrgScopTier[p1] == '<') {
			for (p2=p1+1; OrgScopTier[p2] != '>' && p2<p4; p2++) ;
			if (OrgScopTier[p2] == '>') p2++;
		} else p2 = p1;
		p3 = p2 - 1;
		do {
			for (p3++; uttline[p3] != '$' && p3 < p4; p3++) ;
		} while (!uS.isskip(uttline,p3-1,&dFnt,MBF) && uttline[p3]=='$') ;
		if (uttline[p3] == '$') {
			while (!uS.isskip(uttline,p3,&dFnt,MBF) && p3 < p4) p3++;
			while (uS.isskip(uttline,p3,&dFnt,MBF) && p3 < p4) p3++;
		}
		i = p2;
		while (1) {
/*
printf("p1=%d; p2=%d; p3=%d; p4=%d; i=%d; wf=%d;\n", p1, p2, p3, p4, i, wf);
*/
			if (i >= p2 && i < p4) {
				if ((i=getword(utterance->speaker, uttline, ModWord, NULL, i)) == 0) break;
				while (uS.isskip(OrgScopTier,i,&dFnt,MBF) && i < p3) i++;
/*
printf("st=%s; p3=%d; i=%d; word=%s;\n", OrgScopTier+p3, p3, i, word);
*/
				if (i <= p3) {
					if (*ModWord != '$') wf = 1;
					else {
						modrep_pars(w,OrgScopTier+p1,p2,p4);
						wf = 0;
					}
				}
				if (i >= p3) {
					if (wf) modrep_pars(w,OrgScopTier+p1,p2,p4);
					wf = 0;
					if (OrgScopTier[p3] != '$') {
						tf = 0;
						p2 = p3;
						p3--;
						do {
							for(p3++; uttline[p3]!='$' && p3<p4;p3++);
						} while (!uS.isskip(uttline,p3-1,&dFnt,MBF) && uttline[p3]=='$') ;
					}
					if (uttline[p3] == '$') {
						while (!uS.isskip(uttline,p3,&dFnt,MBF) && p3 < p4) p3++;
						while (uS.isskip(uttline,p3,&dFnt,MBF) && p3 < p4) p3++;
					}
				}
			} else {
				if (wf) modrep_pars(w,OrgScopTier+p1,p2,p4);
				if (OrgScopTier[p4] != EOS) getrefrange(p4, w);
				break;
			}
		}
	}
}
 
void ChoseProcessor(char WhichLine, char OtherSP, char zeroLine) {
	char *s, *ScopTier = NULL;
	char ab, sf, ScopFound;

	if (OtherSP != '*') {
		ab = FALSE;
		sf = FALSE;
		ScopFound = FALSE;
		for (s=OtherUtt; *s; s++) {
			if (*s == '<' && ab == FALSE) ab = TRUE;
			else if (*s == '>' && ab) {
				if (sf) ScopFound = TRUE;
				ab = FALSE;
				sf = FALSE;
			} else if (ab) {
				if (isdigit(*s)) sf = TRUE;
				else if (*s== 'a' && *(s+1)== 'f' && *(s+2)== 't') sf = TRUE;
				else if (*s== 'b' && *(s+1)== 'e' && *(s+2)== 'f') sf = TRUE;
			}
		}
		if (ScopFound)
			ScopTier = OtherUtt;
	}

	ab = FALSE;
	sf = FALSE;
	ScopFound = FALSE;
	for (s=uttline; *s; s++) {
		if (*s == '<' && ab == FALSE) ab = TRUE;
		else if (*s == '>' && ab) {
			if (sf) ScopFound = TRUE;
			ab = FALSE;
			sf = FALSE;
		} else if (ab) {
			if (isdigit(*s)) sf = TRUE;
			else if (*s== 'a' && *(s+1)== 'f' && *(s+2)== 't') sf = TRUE;
			else if (*s== 'b' && *(s+1)== 'e' && *(s+2)== 'f') sf = TRUE;
		}
	}

	if (ScopFound && ScopTier != NULL) {
		fprintf(stderr,"ERROR: both of the following tiers have scoping!\n");
		fputs(OrgScopTier, stderr);
		fputs(utterance->line,  stderr);
		modrepError();
	}

	if (ScopFound || ScopTier != NULL) {
		if (ScopTier == NULL) {
			strcpy(OrgScopTier, utterance->line);
			getrefrange(0, WhichLine);
		} else {
			ScopTier = uttline;
			uttline  = OtherUtt;
			OtherUtt = ScopTier;
			getrefrange(0, WhichLine);
			OtherUtt = uttline;
			uttline  = ScopTier;
		}
	} else
		ProcessLines(WhichLine, OtherSP, zeroLine);
}

void call() {
	char OtherSP = 0;
	char WhichLine = NONLINE, zeroLine = NONLINE;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (*utterance->speaker == '*') {
			WhichLine = NONLINE;
			zeroLine = NONLINE;
			*OtherUtt = EOS;
			*OrgScopTier = EOS;
		}
		strcpy(templineC,utterance->speaker);
		uS.uppercasestr(templineC, &dFnt, MBF);
		if (uS.partcmp(templineC,ModLine,FALSE,FALSE)) {
			if (WhichLine != NONLINE && WhichLine != MODLINE) {
				if (*utterance->speaker == '*' && *utterance->line == '0' && (utterance->line[1] == '.' || utterance->line[1] == EOS))
					zeroLine = CURLINE;
				ChoseProcessor(WhichLine, OtherSP, zeroLine);
			} else {
				WhichLine = MODLINE;
				OtherSP = *utterance->speaker;
				strcpy(OtherUtt, uttline);
				strcpy(OrgScopTier, utterance->line);
				if (OtherSP == '*' && *OrgScopTier == '0' && (OrgScopTier[1] == '.' || OrgScopTier[1] == EOS))
					zeroLine = OTHLINE;
			}
		} else if (uS.partcmp(templineC,RepLine,FALSE,FALSE)) {
			if (WhichLine != NONLINE && WhichLine != REPLINE) {
				if (*utterance->speaker == '*' && *utterance->line == '0' && (utterance->line[1] == '.' || utterance->line[1] == EOS))
					zeroLine = CURLINE;
				ChoseProcessor(WhichLine, OtherSP, zeroLine);
			} else {
				WhichLine = REPLINE;
				OtherSP = *utterance->speaker;
				strcpy(OtherUtt, uttline);
				strcpy(OrgScopTier, utterance->line);
				if (OtherSP == '*' && *OrgScopTier == '0' && (OrgScopTier[1] == '.' || OrgScopTier[1] == EOS))
					zeroLine = OTHLINE;
			}
		}
	}
	if (!combinput)
		modrep_pr_result();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'a':
			isSort = TRUE;
			no_arg_option(f);
			break;
		case 'b':
			if (!*f) {
				fprintf(stderr,"Tier name expected after %s option.\n", f-2);
				modrepError();
			}
			uS.uppercasestr(f, &dFnt, C_MBF);
			if (*f != '*' && *f != '%' && *f != '@') {
				strcpy(ModLine, "*");
				if (mainTierselected != NULL && mainTierselected[1] == 't') {
					fprintf(stderr,"Selecting more than one speaker tier is not allowed.\n");
					fprintf(stderr,"One speaker tier is already selected using %s option.\n", mainTierselected);
					modrepError();
				}
				mainTierselected = f-2;
			} else {
				*ModLine = EOS;
				if (*f == '*') {
					if (mainTierselected != NULL && mainTierselected[1] == 't') {
						fprintf(stderr,"Selecting more than one speaker tier is not allowed.\n");
						fprintf(stderr,"One speaker tier is already selected using %s option.\n", mainTierselected);
						modrepError();
					}
					mainTierselected = f-2;
				}
			}
			if (uS.mStrnicmp(f, "%mor", 4) == 0)
				mor_specified = TRUE;
			strcat(ModLine, f);
			break;
		case 'c':
			if (!*f) {
				fprintf(stderr,"Tier name expected after %s option.\n", f-2);
				modrepError();
			}
			uS.uppercasestr(f, &dFnt, C_MBF);
			if (*f != '*' && *f != '%' && *f != '@') {
				strcpy(RepLine, "*");
				if (mainTierselected != NULL && mainTierselected[1] == 't') {
					fprintf(stderr,"Selecting more than one speaker tier is not allowed.\n");
					fprintf(stderr,"One speaker tier is already selected using %s option.\n", mainTierselected);
					modrepError();
				}
				mainTierselected = f-2;
			} else {
				*RepLine = EOS;
				if (*f == '*') {
					if (mainTierselected != NULL && mainTierselected[1] == 't') {
						fprintf(stderr,"Selecting more than one speaker tier is not allowed.\n");
						fprintf(stderr,"One speaker tier is already selected using %s option.\n", mainTierselected);
						modrepError();
					}
					mainTierselected = f-2;
				}
			}
			if (uS.mStrnicmp(f, "%mor", 4) == 0)
				mor_specified = TRUE;
			strcat(RepLine, f);
			break;
		case 'd':
			no_arg_option(f);
			isExcel = TRUE;
			break;
		case 'n':
			if (*f) {
				if (*f == '@') {
					f++;
					wdptr_c = modrep_rdexclf('n', 'i', f, wdptr_c, &WordMode_c);
				} else {
					if (*f == '\\' && *(f+1) == '@')
						f++;
					wdptr_c = modrep_addwdptr('n', 'i', f, wdptr_c, &WordMode_c);
				}
			} else {
				fprintf(stderr,"String expected after %s option.\n", f-2);
				modrepError();
			}
			break;
		case 'o':
			if (*f) {
				if (*f == '@') {
					f++;
					wdptr_b = modrep_rdexclf('o', 'i', f, wdptr_b, &WordMode_b);
				} else {
					if (*f == '\\' && *(f+1) == '@')
						f++;
					wdptr_b = modrep_addwdptr('o', 'i', f, wdptr_b, &WordMode_b);
				}
			} else {
				fprintf(stderr,"String expected after %s option.\n", f-2);
				modrepError();
			}
			break;
		case 's':
			if (!*f) {
				fprintf(stderr,"String expected after \"%s\" option\n",f-2);
				modrepError();
			} else if (*(f-2) == '+') {
				if (*f == '@') {
					f++;
					rdexclf('q','i',getfarg(f,f1,i));
				} else {
					if (*f == '\\' && *(f+1) == '@')
						f++;
					addword('q','i',getfarg(f,f1,i));
				}
			} else {
				if (*f == '@') {
					f++;
					rdexclf('q','e',getfarg(f,f1,i));
				} else {
					if (*f == '\\' && *(f+1) == '@')
						f++;
					addword('q','e',getfarg(f,f1,i));
				}
			}
			break;
		default:
			if (*(f-1) == 't') {
				if (*f != '*' && *f != '%' && *f != '@') {
					if (mainTierselected != NULL && mainTierselected[1] != 't') {
						fprintf(stderr,"Selecting more than one speaker tier is not allowed.\n");
						fprintf(stderr,"One speaker tier is already selected using %s option.\n", mainTierselected);
						modrepError();
					}
					mainTierselected = f-2;
				} else {
					if (uS.mStrnicmp(f, "%mor", 4) == 0)
						mor_specified = TRUE;
					if (*f == '*' /* || *f == '#' || uS.mStrnicmp(f, "@ID", 3) == 0 */) {
						if (mainTierselected != NULL && mainTierselected[1] != 't') {
							fprintf(stderr,"Selecting more than one speaker tier is not allowed.\n");
							fprintf(stderr,"One speaker tier is already selected using %s option.\n", mainTierselected);
							modrepError();
						}
						mainTierselected = f-2;
					}
				}
			}
			maingetflag(f-2,f1,i);
			break;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = MODREP;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,modrep_pr_result);
	wdptr_b = freeIEWORDS(wdptr_b);
	wdptr_c = freeIEWORDS(wdptr_c);
	free(OtherUtt);
	free(OrgScopTier);
	if (rootexcel != NULL) {
		AddCEXExtension = ".xls";
		if (Preserve_dir)
			strcpy(StatName, wd_dir);
		else
			strcpy(StatName, od_dir);
		addFilename2Path(StatName, "stat.keymap");
		strcat(StatName, GExt);
		parsfname(StatName, FileName1, "");
		if ((StatFpOut=openwfile(StatName, FileName1, NULL)) == NULL) {
			fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",FileName1);
		}
		pr_Excel(StatFpOut, rootexcel);
		fclose(StatFpOut);
		if (Preserve_dir) {
			fn = strrchr(FileName1, PATHDELIMCHR);
			if (fn == NULL)
				fn = FileName1;
			else
				fn++;
		} else
			fn = FileName1;
		if (!stin_override)
			fprintf(stderr,"Output file <%s>\n", fn);
		else
			fprintf(stderr,"Output file \"%s\"\n", fn);
		keymap_cleanup_excel(rootexcel);
	}
}
