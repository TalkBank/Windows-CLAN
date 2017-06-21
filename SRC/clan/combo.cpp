/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#include "cu.h"
#ifndef UNX
	#include "ced.h"
#else
	#define RGBColor int
	#include "c_curses.h"
#endif

#if !defined(UNX)
#define _main combo_main
#define call combo_call
#define getflag combo_getflag
#define init combo_init
#define usage combo_usage

#endif

#include "mul.h"
#define pr_result combo_pr_result
#define IS_WIN_MODE TRUE

#define PAT_ELEM struct pattern_elements
PAT_ELEM {
	char *pat;
	char neg;
	char wild;
	char ref;
	char isCheckedForDups;
	char isAllNeg;
	int  matchIndex;
	PAT_ELEM *parenElem, *refParenElem;
	PAT_ELEM *andElem, *refAndElem;
	PAT_ELEM *orElem;
} ;

struct cdepTiers {
	char tier[SPEAKERLEN];
	char line[UTTLINELEN]; // found uttlinelen
	struct cdepTiers *nextTier;
} ;

#define MAXREFS 25
#define REFS  struct refstruct
REFS {
	char *code;
	char *tier;
} ;

extern char puredata;

static int  maxRef;
static int  firstmatch;
static int  SpCluster;
static int  StrMatch;
static int	stackAndsI;
static char *expression;
static char *patgiven;
static char *lastTxtMatch;
static char trvsearch;
static char mftime;
static char isIncludeTier;
static char isEchoFlatmac;
static char includeUtteranceDelims;
static char isContinueSearch;
static short combo_string;
static unsigned long *mat;
static PAT_ELEM *origmac;
static PAT_ELEM *flatmac;
static PAT_ELEM *stackAnds[200];
static struct cdepTiers *extTier;
static REFS refArray[MAXREFS];
static IEWORDS *duplicatesList;

/* ******************** combo prototypes ********************** */
/* *********************************************************** */

void usage() {
	printf("Usage: combo [bN gN s %s] filename(s)\n",mainflgs());
	puts("+bN: search a N number-cluster unit(s)");
	puts("+g1: do a string oriented search on a whole tier. (Default: word oriented)");
	puts("+g2: do a string oriented search on just one word.");
	puts("+g3: find only first match on an utterance.");
	puts("+g4: exclude utterance delimiters from search.");
	puts("+g5: \"^\" symbol used as an \"and\" operator, instead of \"followed by\" operator.");
	puts("+sS: output tiers that match specified search pattern S");
	puts("-sS: output tiers that do not match specified search pattern S");
	puts("     \"^*^\" means match zero or more, \"^_^\" means match exactly anyone item");
	puts("+dv: display all parsed individual parts of search pattern");
	puts("+d : outputs each matched sentence in a simple legal CHAT format");
	puts("+d1: outputs legal CHAT format with line numbers and file names");
	puts("+d2: outputs file names once per file only");
	puts("+d3: outputs ONLY matched words in the same format as +d1");
	puts("+d4: if string match is found, add codes and tiers to data file");
	mainusage(TRUE);
}

static void tier_cleanup(void) {
	int i;
	struct cdepTiers *t;

	for (i=0; i < maxRef; i++) {
		if (refArray[i].code)
			free(refArray[i].code);
		if (refArray[i].tier)
			free(refArray[i].tier);
	}
	maxRef = 0;
	while (extTier != NULL) {
		t = extTier;
		extTier = extTier->nextTier;
		free(t);
	}
}

static void freeMac(PAT_ELEM *p, char isRemoveOR) {
	if (p == NULL)
		return;
	if (p->pat != NULL)
		free(p->pat);
	if (p->parenElem != NULL)
		freeMac(p->parenElem, isRemoveOR);
	if (p->andElem != NULL)
		freeMac(p->andElem, isRemoveOR);
	if (isRemoveOR && p->orElem != NULL)
		freeMac(p->orElem, isRemoveOR);
	free(p);
}

static void freeMem(void) {
	free(mat);
	mat = NULL;
	free(expression);
	expression = NULL;
	freeMac(origmac, TRUE);
	origmac = NULL;
	freeMac(flatmac, TRUE);
	flatmac = NULL;
	tier_cleanup();
	patgiven = NULL;
}

static PAT_ELEM *mkElem() {
	PAT_ELEM *t;

	t = NEW(PAT_ELEM);
	if (t == NULL) {
		fprintf(stderr, "combo: Out of memory\n");
		freeMem();
		cutt_exit(0);
	}
	t->pat = NULL;
	t->neg = 0;
	t->ref = 0;
	if (trvsearch)
		t->wild = 1;
	else
		t->wild = 0;
	t->isCheckedForDups = FALSE;
	t->isAllNeg = FALSE;
	t->matchIndex = -1;
	t->parenElem = NULL;
	t->refParenElem = NULL;
	t->andElem = NULL;
	t->refAndElem = NULL;
	t->orElem = NULL;
	return(t);
}

static void addTier(int ref) {
	struct cdepTiers *t;

	for (t=extTier; t != NULL; t=t->nextTier) {
		if (!strcmp(t->tier, refArray[ref].tier)) {
			if (t->line[0] != EOS)
				strcat(t->line, " ");
			strcat(t->line, refArray[ref].code);
			break;
		}
	}
}

static void echo_expr(PAT_ELEM *elem, char isEchoOr, char *str) {
	if (elem->parenElem != NULL) {
		if (elem->neg)
			strcat(str, "!");
		strcat(str, "(");
		echo_expr(elem->parenElem, isEchoOr, str);
		strcat(str, ")");
	} else if (elem->pat != NULL) {
		if (elem->neg)
			strcat(str, "!");
		if (*elem->pat == EOS)
			strcat(str, "{*}");
		else if (combo_string <= 0 && !strcmp(elem->pat, "_"))
			strcat(str, "{_}");
		else
			strcat(str, elem->pat);
	}
	if (elem->andElem != NULL) {
		strcat(str, "^");
		echo_expr(elem->andElem, isEchoOr, str);
	}
	if (isEchoOr && elem->orElem != NULL) {
		strcat(str, "+");
		echo_expr(elem->orElem, isEchoOr, str);
	}
}

static void resetMat(int i) {
	for (; i < UTTLINELEN; i++)
		mat[i] = 0L;
}

static void display(const char *sp, char *inst, FILE  *fp) {
	int i, j;
	char isAttShown;
	unsigned long t;
	AttTYPE oldAtt;
	AttTYPE *att;

	oldAtt = 0;
	if (*sp != EOS) {
		att = utterance->attSp;
		if (SpCluster == 0) {
			ActualPrint(sp, att, &oldAtt, FALSE, FALSE, fp);
			strcpy(templineC, sp);
		} else
			templineC[0] = EOS;
		att = utterance->attLine;
	} else {
		strcpy(templineC, sp);
		att = NULL;
	}
	for (i=0; inst[i]; i++) {
		if (att != NULL)
			printAtts(*att, oldAtt, fp);
		else
			printAtts(0, oldAtt, fp);
		isAttShown = FALSE;
		if (mat[i] > 0L) {
			t = mat[i];
			for (j=0; j < 32; j++) {
				if (j > 0) {
					t = t >> 1;
					if (t <= 0L)
						break;
				}
				if (t & 1L) {
					if (!isAttShown) {
						isAttShown = TRUE;
#ifndef UNX
						fprintf(fp, "%c%c", ATTMARKER, error_start);
#endif
					}
					fprintf(fp, "(%d)", j+1);
				}
			}
#ifndef UNX
			if (isAttShown)
				fprintf(fp, "%c%c", ATTMARKER, error_end);
#endif
//			mat[i] = 0L;
		}
		if (inst[i] != ' ' || isAttShown || (inst[i+1] != '\n' && inst[i+1] != EOS))
			putc(inst[i],fp);
		if (cMediaFileName[0] != EOS && inst[i] == HIDEN_C && isdigit(inst[i+1]))
			fprintf(fp, "%s\"%s\"_", SOUNDTIER, cMediaFileName);
		
		if (att != NULL) {
			oldAtt = *att;
			att++;
		} else
			oldAtt = 0;
	}
	printAtts(0, oldAtt, fp);
}

static void err(const char *s, char *exp, int pos) {
	if (exp[pos] == EOS)
		return;
	mat[pos] = 1L;
	display("",exp,stderr);
	fputs(s, stderr);
	fprintf(stderr, "\n");
	resetMat(0);
}

static long storepat(PAT_ELEM *elem, char *exp, long pos) {
	register int p1, p2;
	char t;

	p1 = pos;
	do {
		t = FALSE;
		for (; ((!uS.isRightChar(exp,pos,'(',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'!',&dFnt,C_MBF) && 
				 !uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,')',&dFnt,C_MBF) &&
				 !uS.isRightChar(exp,pos,'^',&dFnt,C_MBF)) ||
				(t && uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) && exp[pos+1] == ' ')) && exp[pos] != '\n'; pos++) {
			if (uS.isRightChar(exp,pos,'[',&dFnt,C_MBF))
				t = TRUE;
			else if (uS.isRightChar(exp,pos,']',&dFnt,C_MBF))
				t = FALSE;
		}
		if (pos-1 < p1 || !uS.isRightChar(exp, pos-1, '\\', &dFnt, C_MBF))
			break;
		else if (exp[pos] == '\n' || exp[pos] == EOS) {
			if (uS.isRightChar(exp, pos-1, '\\', &dFnt, C_MBF) && !uS.isRightChar(exp, pos-2, '\\', &dFnt, C_MBF)) {
				err("1; Unexpected ending.",exp,pos);
				return(-1L);
			}
			break;
		} else
			pos++;
	} while (1) ;
	if (uS.isRightChar(exp,pos,'!', &dFnt, C_MBF) || uS.isRightChar(exp,pos,'(', &dFnt, C_MBF)) {
		err("Illegal element found.",exp,pos);
		return(-1L);
	}
	p2 = pos;
	if (combo_string <= 0) {
		for(; p1!=p2 && uS.isskip(exp, p1, &dFnt, C_MBF) && !uS.isRightChar(exp, p1, '[', &dFnt, C_MBF) && 
			!uS.isRightChar(exp, p2, '[', &dFnt, C_MBF) && !uS.isRightChar(exp, p2, ']', &dFnt, C_MBF); p1++);
		if (p1 != p2) {
			for (p2--; p2 >= p1 && uS.isskip(exp, p2, &dFnt, C_MBF) && 
				 !uS.isRightChar(exp, p2, '[', &dFnt, C_MBF) && !uS.isRightChar(exp, p2, ']', &dFnt, C_MBF); p2--) ;
			p2++;
		}
	}
	if (uS.isRightChar(exp,p2-1,'*',&dFnt,C_MBF) && (!uS.isRightChar(exp,p2-2,'\\',&dFnt,C_MBF) || uS.isRightChar(exp,p2-3,'\\',&dFnt,C_MBF))) {
		p2--;
		for (; p2 >= p1 && uS.isRightChar(exp,p2,'*',&dFnt,C_MBF); p2--) ;
		if (uS.isRightChar(exp,p2,'\\',&dFnt,C_MBF) && (!uS.isRightChar(exp,p2-1,'\\',&dFnt,C_MBF) || uS.isRightChar(exp,p2-2,'\\',&dFnt,C_MBF)))
			p2 += 2;
		else
			p2++;
		if (combo_string > 0) {
			elem->wild = 1;
			t = exp[p2];
			exp[p2] = EOS;
		} else {
			if (p1 == p2) {
				elem->wild = 1;
			} else
				p2++;
			t = exp[p2];
			exp[p2] = EOS;
		}
		elem->pat = (char *)malloc((strlen(exp+p1)+1));
		if (elem->pat == NULL) {
			fprintf(stderr, "combo: Out of memory\n");
			freeMem();
			cutt_exit(0);
		}
		strcpy(elem->pat,exp+p1);
		if (uS.isRightChar(exp,p1,'[',&dFnt,C_MBF) && uS.isRightChar(exp,p1+strlen(exp+p1)-1,']',&dFnt,C_MBF)) {
			strcpy(templineC, "+");
			strcat(templineC, exp+p1);
			addword('\0','i',templineC);
		}
		exp[p2] = t;
		if (*elem->pat == 0) {
			if (combo_string > 0) {
				err("character '*' as an item is not allowed in string mode, +g1 or +g2 option.",exp,p1);
				return(-1L);
			}
			if (trvsearch) {
				err("character '*' as an item is not allowed with +g5 option.",exp,p1);
				return(-1L);
			}
		}
	} else {
		if (p1 == p2) {
			if (exp[p2] == '\n' && exp[p2+1] == EOS) {
				err("2; Unexpected ending.",exp,p1);
				return(-1L);
			} else {
				err("missing item or item is part of punctuation/delimiters set.",exp,p1);
				return(-1L);
			}
		}
		t = exp[p2];
		exp[p2] = EOS;
		elem->pat = (char *)malloc((size_t) (strlen(exp+p1)+1));
		if (elem->pat == NULL) {
			fprintf(stderr, "combo: Out of memory\n");
			freeMem();
			cutt_exit(0);
		}
		strcpy(elem->pat,exp+p1);
		if (uS.isRightChar(exp,p1,'[',&dFnt,C_MBF) && uS.isRightChar(exp,p1+strlen(exp+p1)-1,']',&dFnt,C_MBF)) {
			strcpy(templineC, "+");
			if (exp[p1+1] == '+' && exp[p1+2] == ' ') {
				strcat(templineC, "<");
				strcat(templineC, exp+p1+1);
				addword('P','i',templineC);
			} else {
				strcat(templineC, exp+p1);
				addword('\0','i',templineC);
			}
		}
		exp[p2] = t;
	}
	if (combo_string <= 0) {
		t = FALSE;
		for (; p1 < p2; p1++) {
			if (uS.isRightChar(exp,p1,'[',&dFnt,C_MBF))
				t = TRUE;
			else if (uS.isRightChar(exp,p1,']',&dFnt,C_MBF))
				t = FALSE;
			else if (!t && uS.isskip(exp, p1, &dFnt, C_MBF)) {
				err("Illegal character in a string.",exp,p1);
				return(-1L);
			}
		}
	}
	return(pos);
}

static long makeElems(char *exp, long pos, PAT_ELEM *elem) {
	if (uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) || uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
		err("Illegal element found.",exp,pos);
		return(-1L);
	}
	if (exp[pos] == '\001') {
		pos++;
		elem->ref = exp[pos++];
	}	
	while (exp[pos] != EOS && !uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
		if (uS.isRightChar(exp,pos,'(',&dFnt,C_MBF)) {
			pos++;
			elem->parenElem = mkElem();
			pos = makeElems(exp,pos,elem->parenElem);
			if (pos == -1L)
				return(-1L);
			if (!uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
				err("No matching ')' found.",exp,pos);
				return(-1L);
			}
			if (exp[pos])
				pos++;
			if (!uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) && 
				!uS.isRightChar(exp,pos,')',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'\n',&dFnt,C_MBF)) {
				err("Illegal element found.",exp,pos);
				return(-1L);
			}
		} else if (uS.isRightChar(exp,pos,'!',&dFnt,C_MBF)) { /* not */
			pos++;
			elem->neg = 1;
			if (uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) || 
				uS.isRightChar(exp,pos,')',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'!',&dFnt,C_MBF)) {
				err("Illegal element found.",exp,pos);
				return(-1L);
			} else if (!uS.isRightChar(exp,pos,'(',&dFnt,C_MBF)) {
				pos = storepat(elem, exp, pos);
				if (pos == -1L)
					return(-1L);
			}
		} else if (uS.isRightChar(exp,pos,'+',&dFnt,C_MBF)) { /* or */
			elem->orElem = mkElem();
			elem = elem->orElem;
			pos++;
			if (uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) || uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
				err("Illegal element found.",exp,pos);
				return(-1L);
			} else if (!uS.isRightChar(exp,pos,'(',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'!',&dFnt,C_MBF)) {
				pos = storepat(elem, exp, pos);
				if (pos == -1L)
					return(-1L);
			}
		} else if (uS.isRightChar(exp,pos,'^',&dFnt,C_MBF)) { /* and */
			elem->andElem = mkElem();
			elem = elem->andElem;
			pos++;
			if (uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) || uS.isRightChar(exp,pos,'^',&dFnt,C_MBF) || uS.isRightChar(exp,pos,')',&dFnt,C_MBF)) {
				err("Illegal element found.",exp,pos);
				return(-1L);
			} else if (!uS.isRightChar(exp,pos,'(',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'!',&dFnt,C_MBF)) {
				pos = storepat(elem, exp, pos);
				if (pos == -1L)
					return(-1L);
			}
		} else if (uS.isRightChar(exp,pos,'\n',&dFnt,C_MBF))
			pos++;
		else {
			pos = storepat(elem, exp, pos);
			if (pos == -1L)
				return(-1L);
		}
	}
	return(pos);
}

static void addNewCod(char *code) {
	struct cdepTiers *nextone;

	if (extTier == NULL) {
		extTier = NEW(struct cdepTiers);
		nextone = extTier;
	} else {
		nextone = extTier;
		while (nextone->nextTier != NULL)
			nextone = nextone->nextTier;
		nextone->nextTier = NEW(struct cdepTiers);
		nextone = nextone->nextTier;
	}
	if (nextone == NULL) {
		fprintf(stderr, "combo: Out of memory\n");
		freeMem();
		cutt_exit(0);
	}
	strncpy(nextone->tier, code, SPEAKERLEN-1);
	nextone->tier[SPEAKERLEN-1] = EOS;
	nextone->line[0] = EOS;
	nextone->nextTier = NULL;
}

static void storeRef(int ref, char *s, char *fname) {
	char *t, qt;

	if (ref < 0 || ref >= MAXREFS) {
		maxRef--;
		fprintf(stderr,"combo: Too many conditional code dependencies in search string file.\n");
		freeMem();
		cutt_exit(0);
	}
	refArray[ref].code = NULL;
	refArray[ref].tier = NULL;
	for (; *s != '"' && *s != '\'' && *s; s++) ;
	if (*s == '"' || *s == '\'') {
		qt = *s;
		t = templineC3;
		for (s++; *s != '\n' && *s != qt && *s; s++) {
			*t++ = *s;
		}
		*t = EOS;
		if (*s != qt) {
			fprintf(stderr,"Search string in file \"%s\" is missing closing '%c'",fname,qt);
			freeMem();
			cutt_exit(0);
		}
		s++;
		refArray[ref].code = (char *)malloc(strlen(templineC3));
		if (refArray[ref].code == NULL) {
			maxRef--;
			fprintf(stderr,"combo: Too many conditional code dependencies in search string file.\n");
			freeMem();
			cutt_exit(0);
		}
//		uS.uppercasestr(templineC3, &dFnt, MBF);
		strcpy(refArray[ref].code, templineC3);
	}
	for (; *s != '"' && *s != '\'' && *s; s++) ;
	if (*s == '"' || *s == '\'') {
		qt = *s;
		t = templineC3;
		for (s++; *s != '\n' && *s != qt && *s; s++) {
			*t++ = *s;
		}
		*t = EOS;
		if (*s != qt) {
			fprintf(stderr,"Search string in file \"%s\" is missing closing '%c'",fname,qt);
			freeMem();
			cutt_exit(0);
		}
		s++;
		refArray[ref].tier = (char *)malloc(strlen(templineC3));
		if (refArray[ref].tier == NULL) {
			maxRef--;
			fprintf(stderr,"combo: Too many conditional code dependencies in search string file.\n");
			freeMem();
			cutt_exit(0);
		}
		uS.remblanks(templineC3);
		strcat(templineC3, "\t");
		uS.lowercasestr(templineC3, &dFnt, C_MBF);
		strcpy(refArray[ref].tier, templineC3);
		addNewCod(templineC3);
	}
}

static char combo_isBlankLine(char *line) {
	for (; isSpace(*line) && *line != '\n' && *line != EOS; line++) ;
	if (*line == '\n' || *line == EOS)
		return(TRUE);
	else
		return(FALSE);
}

static char *GetIncludeFile(char *exp, char *fname) {
	FILE *fp;
	char wd[256], *s, qt, isOpenAdded;
	FNType mFileName[FNSize];

	if (*fname == EOS) {
		fprintf(stderr,"No include file specified!!\n");
		freeMem();
		cutt_exit(0);
	}
	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of the include files:\n\t\"%s\", \"%s\"\n", fname, mFileName);
		freeMem();
		cutt_exit(0);
	}
	*exp++ = '(';
	isOpenAdded = FALSE;
	while (fgets_cr(wd, 255, fp)) {
		if (uS.isUTF8(wd) || uS.partcmp(wd, FONTHEADER, FALSE, FALSE) || (*wd == '#' && *(wd+1) == ' ') || combo_isBlankLine(wd))
			continue;
		s = wd;
		if (*s == '"' || *s == '\'') {
			qt = *s;
			maxRef++;
			*exp++ = '(';
			*exp++ = '\001';
			*exp++ = maxRef;
			for (s++; *s != '\n' && *s != qt && *s; s++) {
				*exp++ = *s;
			}
			if (*s != qt) {
				fprintf(stderr,"Search string in file \"%s\" is missing closing '%c'",fname,qt);
				freeMem();
				cutt_exit(0);
			}
			storeRef(maxRef-1, s+1, fname);
			if (*(exp-1) == '+' || *(exp-1) == '^')
				exp--;
			*exp++ = ')';
		} else {
			if (!isOpenAdded)
				*exp++ = '(';
			isOpenAdded = FALSE;
			for (; *s != '\n' && *s; s++) {
				*exp++ = *s;
			}
		}
		isOpenAdded = FALSE;
		if (*(exp-1) != '+' && *(exp-1) != '^') {
			*exp++ = ')';
			*exp++ = '+';
			*exp++ = '(';
			isOpenAdded = TRUE;
		}
	}
	if (isOpenAdded && *(exp-1) == '(')
		exp--;
	if (*(exp-1) == '+' || *(exp-1) == '^')
		exp--;
	if (!isOpenAdded)
		*exp++ = ')';
	*exp++ = ')';
	fclose(fp);
	return(exp);
}

static char mycpy(char *exp, char *pat) {
	char t, *beg, isFileFound;

	isFileFound = FALSE;
	while (*pat) {
		if (*pat == '\\') {
			*exp++ = '\\';
			pat++;
			if (*pat)
				*exp++ = *pat++;
		} else if (*pat == '@') {
			pat++;
			beg = pat;
			while (*pat != '+' && *pat != '^' && *pat != ')' && *pat)
				pat++;
			t = *pat;
			*pat = EOS;
			exp = GetIncludeFile(exp, beg);
			*pat = t;
			isFileFound = TRUE;
		} else
			*exp++ = *pat++;
	}
	if (*(exp-1) != '\n')
		*exp++ = '\n';
	*exp = EOS;
	return(isFileFound);
}

static char getexpr() {
	if (patgiven != NULL) {
		while (mycpy(expression, patgiven)) {
			strcpy(templineC3, expression);
			patgiven = templineC3;
		}
	} else {
		fprintf(stderr, "Please specify search pattern with \"+s\" option.\n");
		return(FALSE);
	} 
	if (expression[1] == EOS)
		return(FALSE);
	if (!nomap)
		uS.lowercasestr(expression, &dFnt, C_MBF);
	IsSearchR7(expression);
	IsSearchCA(expression);
	origmac = mkElem();
	*utterance->speaker = EOS;
	if (makeElems(expression,0L,origmac) == -1) {
		return(FALSE);
	}
	return(TRUE);
}

static PAT_ELEM *getLastElem(PAT_ELEM *elem) {
	int i;

	for (i=0; i < stackAndsI; i++) {
		if (elem == NULL) {
			return(NULL);
		} else if (elem->andElem != NULL) {
			if (elem->refAndElem != stackAnds[i])
				return(NULL);
			else
				elem = elem->andElem;
		} else if (elem->parenElem != NULL) {
			if (elem->refParenElem != stackAnds[i])
				return(NULL);
			else
				elem = elem->parenElem;
		} else
			return(NULL);
	}
	return(elem);
}

static void addParenToLastFlatMacs(PAT_ELEM *cur) {
	PAT_ELEM *elem, *flatp;

	for (flatp=flatmac; flatp != NULL; flatp=flatp->orElem) {
		elem = getLastElem(flatp);
		if (elem != NULL && elem->parenElem == NULL && elem->pat == NULL) {
			elem->wild = cur->wild;
			elem->neg = cur->neg;
			elem->ref = cur->ref;
			elem->matchIndex = cur->matchIndex;
			elem->refParenElem = cur->parenElem;
			elem->parenElem = mkElem();
		}
	}
}

static void addPatToLastFlatMacs(PAT_ELEM *cur) {
	PAT_ELEM *elem, *flatp;

	for (flatp=flatmac; flatp != NULL; flatp=flatp->orElem) {
		elem = getLastElem(flatp);
		if (elem != NULL && elem->parenElem == NULL && elem->pat == NULL) {
			elem->wild = cur->wild;
			elem->neg = cur->neg;
			elem->ref = cur->ref;
			elem->matchIndex = cur->matchIndex;
			if (cur->pat == NULL)
				elem->pat = NULL;
			else {
				elem->pat = (char *)malloc(strlen(cur->pat)+1);
				if (elem->pat == NULL) {
					fprintf(stderr, "combo: Out of memory\n");
					freeMem();
					cutt_exit(0);
				}
				strcpy(elem->pat, cur->pat);
			}
		}
	}
}

static void addAndToFlatMacs(PAT_ELEM *cur) {
	PAT_ELEM *elem, *flatp;

	for (flatp=flatmac; flatp != NULL; flatp=flatp->orElem) {
		elem = getLastElem(flatp);
		if (elem != NULL && elem->andElem == NULL) {
			elem->refAndElem = cur->andElem;
			elem->andElem = mkElem();
		}
	}
}

static void copyMac(PAT_ELEM *elem, PAT_ELEM *lastElem, PAT_ELEM *flatp) {
	if (flatp != NULL && lastElem != flatp) {
		elem->wild = flatp->wild;
		elem->neg = flatp->neg;
		elem->ref = flatp->ref;
		elem->matchIndex = flatp->matchIndex;
		if (flatp->parenElem != NULL) {
			elem->refParenElem = flatp->refParenElem;
			elem->parenElem = mkElem();
			copyMac(elem->parenElem, lastElem, flatp->parenElem);
		} else if (flatp->pat != NULL) {
			elem->pat = (char *)malloc(strlen(flatp->pat)+1);
			if (elem->pat == NULL) {
				fprintf(stderr, "combo: Out of memory\n");
				freeMem();
				cutt_exit(0);
			}
			strcpy(elem->pat, flatp->pat);
		}
		if (flatp->andElem != NULL) {
			elem->refAndElem = flatp->refAndElem;
			elem->andElem = mkElem();
			copyMac(elem->andElem, lastElem, flatp->andElem);
		}
	}
}

static char isInLst(IEWORDS *twd, char *str) {	
	for (; twd != NULL; twd = twd->nextword) {
		if (!strcmp(str, twd->word))
			return(TRUE);
	}
	return(FALSE);
}

static IEWORDS *InsertFlatp(IEWORDS *lst, char *str) {
	IEWORDS *t;
	
	if ((t=NEW(IEWORDS)) == NULL) {
		fprintf(stderr, "combo: Out of memory\n");
		freeIEWORDS(lst);
		freeMem();
		cutt_exit(0);
	}			
	t->word = (char *)malloc(strlen(str)+1);
	if (t->word == NULL) {
		fprintf(stderr, "combo: Out of memory\n");
		free(t);
		freeIEWORDS(lst);
		freeMem();
		cutt_exit(0);
	}
	strcpy(t->word, str);
	t->nextword = lst;
	lst = t;
	return(lst);
}

static void removeDuplicateFlatMacs(char isEcho) {
	PAT_ELEM *flatp, *lflatp;
	
	lflatp = flatmac;
	for (flatp=flatmac; flatp != NULL; flatp=flatp->orElem) {
		if (!flatp->isCheckedForDups) {
			flatp->isCheckedForDups = TRUE;
			templineC1[0] = EOS;
			echo_expr(flatp, FALSE, templineC1);
			if (!isInLst(duplicatesList, templineC1)) {
				duplicatesList = InsertFlatp(duplicatesList, templineC1);
				if (isEcho)
					fprintf(stderr, "%s\n", templineC1);
			} else {
				lflatp->orElem = flatp->orElem;
				freeMac(flatp, FALSE);
				flatp = lflatp;
			}
		} else if (isEcho) {
			templineC1[0] = EOS;
			echo_expr(flatp, FALSE, templineC1);
			fprintf(stderr, "%s\n", templineC1);
		}
		lflatp = flatp;
	}
}

static void duplicateFlatMacs(void) {
	PAT_ELEM *elem, *elemRoot, *flatp, *lastElem;

	removeDuplicateFlatMacs(FALSE);
	elemRoot = NULL;
	for (flatp=flatmac; flatp != NULL; flatp=flatp->orElem) {
		lastElem = getLastElem(flatp);
		if (lastElem != NULL) {
			if (elemRoot == NULL) {
				elemRoot = mkElem();
				elem = elemRoot;
			} else {
				for (elem=elemRoot; elem->orElem != NULL; elem=elem->orElem) ;
				elem->orElem = mkElem();
				elem = elem->orElem;
			}
			copyMac(elem, lastElem, flatp);
		}
	}
	if (flatmac == NULL) {
		flatmac = elemRoot;
	} else {
		for (flatp=flatmac; flatp->orElem != NULL; flatp=flatp->orElem) ;
		flatp->orElem = elemRoot;
	}
}

static void addToStack(PAT_ELEM *elem) {
	if (stackAndsI >= 200) {
		fprintf(stderr, "combo: stack index exceeded 200 \"^\" elements\n");
		freeMem();
		cutt_exit(0);
	}
	stackAnds[stackAndsI] = elem;
	stackAndsI++;
}

static void flatten_expr(PAT_ELEM *cur) {
	if (cur->parenElem != NULL) {
		addParenToLastFlatMacs(cur);
		addToStack(cur->parenElem);
		flatten_expr(cur->parenElem);
		stackAndsI--;
	} else if (cur->pat != NULL) {
		addPatToLastFlatMacs(cur);
	}
	if (cur->andElem != NULL) {
		addAndToFlatMacs(cur);
		addToStack(cur->andElem);
		flatten_expr(cur->andElem);
		stackAndsI--;
	}
	if (cur->orElem != NULL) {
		duplicateFlatMacs();
		flatten_expr(cur->orElem);
	}
}

static void findAllNegFlats(PAT_ELEM *cur, char *isNeg) {
	char tNeg;

	if (cur->parenElem != NULL) {
		tNeg = TRUE;
		findAllNegFlats(cur->parenElem, &tNeg);
		if (cur->neg)
			tNeg = TRUE;
		if (!tNeg)
			*isNeg = FALSE;
	} else if (cur->pat != NULL) {
		if (!cur->neg && cur->pat[0] != EOS && strcmp(cur->pat, "_")) {
			*isNeg = FALSE;
		}
	}
	if (cur->andElem != NULL) {
		findAllNegFlats(cur->andElem, isNeg);
	}
	cur->isAllNeg = *isNeg;
	if (cur->orElem != NULL) {
		*isNeg = TRUE;
		findAllNegFlats(cur->orElem, isNeg);
	}	
}

void init(char first) {
	int i;
	char isNeg;

	if (first) {
		patgiven = NULL;
		origmac = NULL;
		flatmac = NULL;
		extTier = NULL;
		isIncludeTier = TRUE;
		mftime = 1;
		trvsearch = FALSE;
		SpCluster = 0;
		combo_string = 0;
		isContinueSearch = TRUE;
		maxRef = 0;
		isEchoFlatmac = FALSE;
		includeUtteranceDelims = TRUE;
	} else if (mftime) {
		mftime = 0;
		if (includeUtteranceDelims) {
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '!' ||
					GlobalPunctuation[i] == '?' ||
					GlobalPunctuation[i] == '.' ||
					GlobalPunctuation[i] == ',') 
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}
		}
		freeMac(origmac, TRUE);
		origmac = NULL;
		freeMac(flatmac, TRUE);
		flatmac = NULL;
		if (!getexpr()) {
			freeMem();
			cutt_exit(0);
		}
		if (onlydata == 0) {
			templineC1[0] = EOS;
			echo_expr(origmac, TRUE, templineC1);
			fprintf(stderr, "%s\n", templineC1);
		}
		duplicatesList = NULL;
		stackAndsI = 0;
		flatmac = mkElem();
		flatten_expr(origmac);
		removeDuplicateFlatMacs(isEchoFlatmac);
		duplicatesList = freeIEWORDS(duplicatesList);
		isNeg = TRUE;
		findAllNegFlats(flatmac, &isNeg);
		if (onlydata == 5) {
			char tc[3];
			strcpy(tc, "*");
			MakeOutTierChoice(tc, '+');
			strcpy(tc, "@");
			MakeOutTierChoice(tc, '+');
			strcpy(tc, "%");
			MakeOutTierChoice(tc, '+');
		}
	}
	if (!combinput || first)
		StrMatch = 0;
}

static void pr_result(void) {
	if (isIncludeTier == TRUE) {
		if (onlydata == 0)
			fprintf(fpout, "\n    Strings matched %d times\n\n", StrMatch);
		else
			fprintf(stderr, "\n    Strings matched %d times\n\n", StrMatch);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	mat = (unsigned long *)malloc((UTTLINELEN + 1)*sizeof(unsigned long));
	expression = (char *)malloc((UTTLINELEN + 1));
	if (mat == NULL || expression == NULL) {
		fprintf(stderr, "\ncombo: Out of memory\n");
	} else {
		resetMat(0);
		isWinMode = IS_WIN_MODE;
		CLAN_PROG_NUM = COMBO;
		chatmode = CHAT_MODE;
		OnlydataLimit = 5;
		UttlineEqUtterance = FALSE;
		bmain(argc,argv,pr_result);
		freeMem();
	}
}

static void shiftLeftOneSpCluster(UTTER *sTier) {
	char c;
	long i;

	for (i=0L; sTier->line[i]; i++) {
		if (sTier->line[i] == '\n') {
			sTier->tlineno++;
			if (sTier->line[i+1] == '*') {
				i++;
				break;
			}
		}
	}
	att_cp(0,sTier->line,sTier->line+i,sTier->attLine,sTier->attLine+i);
	strcpy(sTier->tuttline, sTier->tuttline+i);
	if (sTier->line[0] == '*') {
		for (i=0L; sTier->line[i] && sTier->line[i] != ':'; i++) ;
		if (sTier->line[i] == ':') {
			for (i++; isSpace(sTier->line[i]); i++) ;
		}
		c = sTier->line[i];
		sTier->line[i] = EOS;
		att_cp(0,sTier->speaker,sTier->line,sTier->attSp,sTier->attLine);
		sTier->line[i] = c;
	}
}

static int subindex(char *s, char *pat, int i, char end) {
	register int j, k, n, m;

	if (combo_string <= 0)
		firstmatch = i;
	else
		firstmatch = -1;
	for (j = i, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*' || pat[k] == '%') {
			for (k++; pat[k] == '*' || pat[k] == '%'; k++) ;
			if (pat[k] == '\\') k++;
forward:
			if (pat[0] == '[') {
				while (s[j] != EOS && s[j] != pat[k])
					j++;
				if (firstmatch == -1)
					firstmatch = j;
				if (s[j] == EOS) {
					if (!pat[k])
						return(j);
					else
						return(-1); }
			} else {
				while (s[j] != end && s[j] != EOS && s[j] != pat[k])
					j++;
				if (firstmatch == -1)
					firstmatch = j;
				if (s[j] == end || s[j] == EOS) { 
					if (!pat[k])
						return(j);
					else
						return(-1);
				}
			}
			for (m=j+1, n=k+1; s[m] != end && pat[n]; m++, n++) {
				if (pat[n]=='*' || pat[n]=='%' || pat[n]=='_' || pat[n]=='\\')
					break;
				else if (s[m] != pat[n]) {
					j++;
					goto forward;
				}
			}
			if (combo_string <= 0) {
				if (!pat[n] && s[m] != end) {
					j++;
					goto forward;
				}
			}
		} else {
			if (firstmatch == -1)
				firstmatch = j;
			if (pat[k] == '\\') {
				if (s[j] != pat[++k])
					break;
			} else if (pat[k] == '_')
				continue; /* any character */
		}
		if (s[j] != pat[k])
			break;
	}
	if (pat[k] == EOS) {
		if (combo_string > 0)
			return(j);
		else if (s[j] == end)
			return(j);
		else
			return(-1);
	} else
		return(-1);
}

static int CMatch(char *s, char *pat, char wild) { /* return index of pat in s, -1 if no Match */
	register int i, j;

	i = 0;
	if (combo_string <= 0) {
		while (s[i] == ' ' || s[i] == '\n')
			i++;
		if (*pat == EOS) {
			firstmatch = 0;
			return(0);
		} else if (!strcmp(pat, "_")) {
			firstmatch = i;
			while (s[i] != ' ' && s[i] != 0)
				i++;
			while (s[i] == ' ' || s[i] == '\n')
				i++;
			return(i);
		}
	}
	if (wild == 1) {
		while (s[i]) {
			if (combo_string <= 0) {
				if ((j=subindex(s, pat, i, ' ')) != -1) {
					while (s[j] == ' ' || s[j] == '\n')
						j++;
					return(j);
				}
				while (s[i] != ' ' && s[i] != 0)
					i++;
				while (s[i] == ' ' || s[i] == '\n')
					i++;
			} else {
				if ((j=subindex(s, pat, i, EOS)) != -1)
					return(j);
				i++;
			}
		}
		return(-1);
	} else {
		if (combo_string <= 0) {
			if ((j=subindex(s, pat, i, ' ')) != -1) {
				while (s[j] == ' ' || s[j] == '\n')
					j++;
				return(j);
			}
		} else {
			if ((j=subindex(s, pat, i, EOS)) != -1)
				return(j);
		}
		return(-1);
	}
}

static char *match(char *txt, PAT_ELEM *tm, char wild, char *isMatched) {
	int last = -1;
	char *tmp, negC1, negC2, *negSf, *negSl, *lastTxt, isNegFound;

	isNegFound = FALSE;
	negSf = NULL;
	negSl = NULL;
	negC1 = 0;
	negC2 = 0;
	if (tm->pat != NULL) {
		if ((last=CMatch(txt,tm->pat,wild)) != -1) {
			if (tm->neg) {
				if (trvsearch)
					return(NULL);
				negSf = txt + firstmatch;
				negC1 = negSf[0];
				negC2 = negSf[1];
				negSf[0] = '\n';
				negSf[1] = EOS;
				negSl = txt + last;
				isNegFound = TRUE;
			} else {
				if (tm->pat[0] != EOS) {
					tm->matchIndex = (int)((txt + firstmatch) - uttline);
				}
				tmp = txt + last;
				if (trvsearch) {
					if (lastTxtMatch < tmp)
						lastTxtMatch = tmp;
				} else
					txt = tmp;
			}
		} else {
			if (!tm->neg)
				return(NULL);
		}
	} else if (tm->parenElem != NULL) {
		if ((tmp=match(txt,tm->parenElem,wild,isMatched)) != NULL) {
			txt = tmp;
			if (tm->neg) {
				if (trvsearch)
					return(NULL);
				negSf = txt + firstmatch;
				negC1 = negSf[0];
				negC2 = negSf[1];
				negSf[0] = '\n';
				negSf[1] = EOS;
				negSl = txt;
				isNegFound = TRUE;
			}
		} else {
			if (!tm->neg)
				return(NULL);
		}
	}
	if (tm->andElem != NULL) {
		if (negSf != NULL) {
			lastTxt = txt;
			txt = match(txt,tm->andElem,tm->wild,isMatched);
			if (wild && txt == NULL && lastTxt < negSf) {
				txt = lastTxt;
				tmp = txt;
				while (txt < negSf) {
					txt = match(txt,tm->andElem,tm->wild,isMatched);
					if (*isMatched == FALSE)
						break;
					if (txt != NULL) {
						break;
					} else {
						if (combo_string <= 0) {
							while (*tmp != ' ' && *tmp != 0)
								tmp++;
							while (*tmp == ' ' || *tmp == '\n')
								tmp++;
						} else {
							tmp = tmp + 1;
						}
						if (tmp >= negSf)
							break;
						txt = tmp;
					}
				}
			}
			negSf[0] = negC1;
			negSf[1] = negC2;
			negSf = NULL;
			if (txt == NULL && *isMatched) {
				txt = negSl;
				txt = match(txt,tm->andElem,tm->wild,isMatched);
				if (tm->neg && isNegFound && txt != NULL)
					*isMatched = FALSE;
			}
			negSl = NULL;
		} else {
			txt = match(txt,tm->andElem,tm->wild,isMatched);
			if (tm->neg && isNegFound && txt != NULL)
				*isMatched = FALSE;
		}
	} else {
		if (negSf != NULL) {
			negSf[0] = negC1;
			negSf[1] = negC2;
			negSf = NULL;
			negSl = NULL;
		}
		if (isNegFound) {
			*isMatched = FALSE;
			return(NULL);
		}
	}
	if (tm->ref && onlydata == 5 && txt != NULL && *isMatched) {
		addTier(tm->ref-1);
	}
	return(txt);
}

static char *negMatch(char *txt, PAT_ELEM *tm, char wild) {
	int last = -1;
	char *tmp;

	if (tm->pat != NULL) {
		if ((last=CMatch(txt,tm->pat,wild)) != -1) {
			if (tm->pat[0] != EOS) {
				tm->matchIndex = (int)((txt + firstmatch) - uttline);
			}
			tmp = txt + last;
			if (trvsearch) {
				if (lastTxtMatch < tmp)
					lastTxtMatch = tmp;
			} else
				txt = tmp;
		} else {
			return(NULL);
		}
	} else if (tm->parenElem != NULL) {
		if ((tmp=negMatch(txt,tm->parenElem,wild)) != NULL) {
			txt = tmp;
		} else {
			return(NULL);
		}
	}
	if (tm->andElem != NULL) {
		txt = negMatch(txt,tm->andElem,tm->wild);
	}
	if (tm->ref && onlydata == 5 && txt != NULL) {
		addTier(tm->ref-1);
	}
	return(txt);
}

static void setmat(PAT_ELEM *cur, int matchnum) {
	int i;
	unsigned long t;

	if (cur->parenElem != NULL) {
		if (!cur->neg)
			setmat(cur->parenElem, matchnum);
	} else if (cur->pat != NULL) {
		if (cur->pat[0] != EOS && !cur->neg && cur->matchIndex > -1 && matchnum < 32) {
			t = 1L;
			if (matchnum > 0)
				t = t << matchnum;
			i = cur->matchIndex;
			if (i > 0 && !UTF8_IS_SINGLE((unsigned char)utterance->line[i]) && !UTF8_IS_LEAD((unsigned char)utterance->line[i])) {
				i--;
			}
			if (i > 0 && !UTF8_IS_SINGLE((unsigned char)utterance->line[i]) && !UTF8_IS_LEAD((unsigned char)utterance->line[i])) {
				i--;
			}
			if (i > 0 && !UTF8_IS_SINGLE((unsigned char)utterance->line[i]) && !UTF8_IS_LEAD((unsigned char)utterance->line[i])) {
				i--;
			}
			mat[i] = (mat[i] | t);
		}
	}
	if (cur->andElem != NULL) {
		setmat(cur->andElem, matchnum);
	}
}

static void resetElems(PAT_ELEM *cur) {
	cur->matchIndex = -1;
	if (cur->parenElem != NULL) {
		resetElems(cur->parenElem);
	}
	if (cur->andElem != NULL) {
		resetElems(cur->andElem);
	}
}

static void findmatch(char *txt) {
	int  matchnum;
	long i, k;
	char *orgTxt, *wTxt, *tTxt, tChr1, tChr2;
	char *outLine;
	char wild, isMatched, isStop, isNegMatched;
	struct cdepTiers *t;
	PAT_ELEM *flatp;

	isStop = FALSE;
	k = strlen(txt);
	if (txt[k-1L] != '\n') {
		txt[k++] = '\n';
		txt[k] = EOS;
	}
	if (!nomap)
		uS.lowercasestr(txt, &dFnt, MBF);
	matchnum = 0;
	if (combo_string != 1) {
		while (*txt == ' ' || *txt == '\n')
			txt++;
	}
	if (trvsearch)
		wild = 1;
	else
		wild = 0;
	orgTxt = txt;
	for (flatp=flatmac; flatp != NULL; flatp=flatp->orElem) {
		if (flatp->pat != NULL && flatp->pat[0] == EOS && flatp->wild && flatp->parenElem == NULL && flatp->andElem == NULL) {
			matchnum++;
		} else {
			if (flatp->isAllNeg)
				isNegMatched = TRUE;
			while (*txt != EOS) {
#ifdef DEBUG_CLAN
				printf("1; pat=%s;wild=%d;origmac->neg=%d;txt=%s", origmac->pat, origmac->wild, origmac->neg, txt);
#endif
				if (combo_string == 1) {
					resetElems(flatp);
					tTxt = txt;
					lastTxtMatch = txt;
					isMatched = TRUE;
					if (flatp->isAllNeg) {
						txt = negMatch(txt,flatp,wild);
						if (txt != NULL) {
							isNegMatched = FALSE;
							break;
						} else {
							txt = tTxt + 1;
						}
					} else {
						txt = match(txt,flatp,wild,&isMatched);
						if (txt != NULL && isMatched) {
							if (matchnum >= 32) {
								isStop = TRUE;
								break;
							}
							setmat(flatp, matchnum);
							matchnum++;
							if (trvsearch)
								txt = lastTxtMatch;
							else if (txt == tTxt)
								txt++;
						} else {
							if (trvsearch)
								break;
							if (matchnum && !isContinueSearch)
								break;
							if (txt == NULL)
								txt = tTxt + 1;
						}
					}
				} else if (combo_string == 2) {
					if (flatp->isAllNeg)
						isNegMatched = TRUE;
					wTxt = txt;
					while (*wTxt != ' ' && *wTxt !=  EOS)
						wTxt++;
					tChr1 = *wTxt;
					tChr2 = *(wTxt+1);
					*wTxt = '\n';
					*(wTxt+1) = EOS;
					while (*txt != EOS) {
						resetElems(flatp);
						tTxt = txt;
						lastTxtMatch = txt;
						isMatched = TRUE;
						if (flatp->isAllNeg) {
							txt = negMatch(txt,flatp,wild);
							if (txt != NULL) {
								isNegMatched = FALSE;
								break;
							} else {
								txt = tTxt + 1;
							}
						} else {
							txt = match(txt,flatp,wild,&isMatched);
							if (txt != NULL && isMatched) {
								if (matchnum >= 32) {
									isStop = TRUE;
									break;
								}
								setmat(flatp, matchnum);
								matchnum++;
								if (trvsearch)
									txt = lastTxtMatch;
								else if (txt == tTxt)
									txt++;
							} else {
								if (trvsearch)
									break;
								if (matchnum && !isContinueSearch)
									break;
								if (txt == NULL)
									txt = tTxt + 1;
							}
						}
					}
					if (flatp->isAllNeg) {
						if (isNegMatched) {
							if (matchnum >= 32) {
								isStop = TRUE;
								break;
							}
							matchnum++;
						}			
						isNegMatched = FALSE;
					}
					*wTxt = tChr1;
					*(wTxt+1) = tChr2;
					if (matchnum && !isContinueSearch)
						break;
					while (*wTxt == ' ' || *wTxt == '\n')
						wTxt++;
					txt = wTxt;
					if (isStop)
						break;
				} else {
					resetElems(flatp);
					tTxt = txt;
					lastTxtMatch = txt;
					isMatched = TRUE;
					if (flatp->isAllNeg) {
						txt = negMatch(txt,flatp,wild);
						if (txt != NULL) {
							isNegMatched = FALSE;
							break;
						} else {
							txt = tTxt;
							while (*txt != ' ' && *txt != 0)
								txt++;
							while (*txt == ' ' || *txt == '\n')
								txt++;
						}
					} else {
						txt = match(txt,flatp,wild,&isMatched);
						if (txt != NULL && isMatched) {
							if (matchnum >= 32) {
								isStop = TRUE;
								break;
							}
							setmat(flatp, matchnum);
							matchnum++;
							if (trvsearch)
								txt = lastTxtMatch;
							else if (txt == tTxt) {
								while (*txt != ' ' && *txt != 0)
									txt++;
							}
							while (*txt == ' ' || *txt == '\n')
								txt++;
						} else {
							if (trvsearch)
								break;
							if (matchnum && !isContinueSearch)
								break;
							if (txt == NULL) {
								txt = tTxt;
								while (*txt != ' ' && *txt != 0)
									txt++;
							}
							while (*txt == ' ' || *txt == '\n')
								txt++;
						}
					}
				}
			}
			if (flatp->isAllNeg && isNegMatched) {
				if (matchnum >= 32) {
					isStop = TRUE;
					break;
				}
				matchnum++;
			}			
			if (matchnum && !isContinueSearch)
				break;
			txt = orgTxt;
			if (isStop)
				break;
		}
	}
	if (CntWUT == 1) {
		outLine = uttline;
		for (i=0L; uttline[i]; i++) {
			if (!isSpace(uttline[i]))
				uttline[i] = utterance->line[i];
		}
	} else
		outLine = utterance->line;
/*
	if (matchnum >= 32) {
		fprintf(fpout,"\n*** File \"%s\": ",oldfname);
		if (utterance->tlineno > 0) 
			fprintf(fpout,"line %ld.\n",utterance->tlineno);
		else
			fprintf(fpout,"\n");
		fprintf(stderr,"WARNING: Number of matches has exceeded the limit of 32\n\n");
	}
*/
	if ((matchnum > 0 && isIncludeTier == TRUE) || (matchnum == 0 && isIncludeTier == FALSE)) {
		if (onlydata == 2 || onlydata == 4) {
			fputs("@Comment:\t-----------------------------------\n",fpout);
			fprintf(fpout,"@Comment:\t*** File \"%s\": ",oldfname);
			if (utterance->tlineno > 0) 
				fprintf(fpout,"line %ld;\n",utterance->tlineno);
			else
				fprintf(fpout,"\n");
		} else if (onlydata == 0) {
			fputs("----------------------------------------\n",fpout);
			fprintf(fpout,"*** File \"%s\": ",oldfname);
			if (utterance->tlineno > 0) 
				fprintf(fpout,"line %ld.\n",utterance->tlineno);
			else
				fprintf(fpout,"\n");
		}
		befprintout(TRUE);
		if (onlydata == 0) {
			display(utterance->speaker,outLine,fpout);
		} else if (onlydata == 4) {
			char sq;
			
			if (SpCluster == 0)
				printout(utterance->speaker,NULL,utterance->attSp,NULL,FALSE);
			for (k=0L; utterance->line[k]; k++) {
				if (mat[k] > 0L) {
					if (!uS.isskip(utterance->line,k,&dFnt,MBF) || 
						uS.isRightChar(utterance->line,k,'[',&dFnt,MBF) || uS.isRightChar(utterance->line,k,']',&dFnt,MBF)) {
						sq = (char)uS.isRightChar(utterance->line,k,'[',&dFnt,MBF);
						for (; (!uS.isskip(utterance->line,k,&dFnt,MBF)  || sq) && utterance->line[k]; k++) {
							if (uS.isRightChar(utterance->line,k,'[',&dFnt,MBF)) sq = TRUE;
							else if (uS.isRightChar(utterance->line,k,']',&dFnt,MBF)) sq = FALSE;
							putc(outLine[k],fpout);
//							mat[k] = 0L;
						}
						k--;
					} else {
						putc(outLine[k],fpout);
//						mat[k] = 0L;
					}
				} else {
					if (utterance->line[k]=='\n' || utterance->line[k]=='\t')
						putc(outLine[k], fpout);
					else
						putc(' ',fpout);
//					mat[k] = 0L;
				}
			}
		} else if (onlydata == 5) {
			if (SpCluster) {
				i = strlen(utterance->speaker);
				if (i > 0L)
					att_cp(0,outLine,outLine+i,utterance->attLine,utterance->attLine+i);
			}
			if (cMediaFileName[0] != EOS)
				changeBullet(outLine, utterance->attLine);
			printout(utterance->speaker,outLine,utterance->attSp,utterance->attLine,FALSE);
			for (t=extTier; t != NULL; t=t->nextTier) {
				for (i=0L; isSpace(t->line[i]); i++) ;
				if (t->line[i] != EOS) {
					printout(t->tier,t->line+i,NULL,NULL,TRUE);
				}
			}
		} else {
			if (SpCluster) {
				i = strlen(utterance->speaker);
				if (i > 0L)
					att_cp(0,outLine,outLine+i,utterance->attLine,utterance->attLine+i);
			}
			if (cMediaFileName[0] != EOS)
				changeBullet(outLine, utterance->attLine);
			printout(utterance->speaker,outLine,utterance->attSp,utterance->attLine,FALSE);
		}
		aftprintout(TRUE);
		StrMatch += matchnum;
	} else if (onlydata == 5 && utterance->nextutt==utterance) {
		befprintout(TRUE);
		if (cMediaFileName[0] != EOS)
			changeBullet(outLine, utterance->attLine);
		printout(utterance->speaker,outLine,utterance->attSp,utterance->attLine,FALSE);
		aftprintout(TRUE);
	}
	if (onlydata == 5) {
		for (t=extTier; t != NULL; t=t->nextTier) {
			if (t->line[0] != EOS)
				t->line[0] = EOS;
		}
	}
	resetMat(0);
}

void call() {
	register long k;
	int  SpClusterCnt;
	UTTER *sTier;

	if (nomain) {
		fprintf(stderr, "-t* option can not be used with \"combo\".\n");
		freeMem();
		cutt_exit(0);
	}
	SpClusterCnt = 1;
	if (SpCluster) {
		sTier = NEW(UTTER);
		if (sTier == NULL) {
			free(mat);
			free(expression);
			out_of_mem();
		}
		sTier->speaker[0] = EOS;
		sTier->line[0] = EOS;
		sTier->tuttline[0] = EOS;
	} else
		sTier = NULL;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (SpCluster && *utterance->speaker == '@') {
			SpClusterCnt = 1;
			sTier->speaker[0] = EOS;
			sTier->line[0] = EOS;
			sTier->tuttline[0] = EOS;
		} else if (SpCluster && SpClusterCnt < SpCluster) {
			if (SpClusterCnt == 1) {
				sTier->tlineno = utterance->tlineno;
				att_cp(0,sTier->speaker,utterance->speaker,sTier->attSp,utterance->attSp);
			}
			att_cp(strlen(sTier->line),sTier->line,utterance->speaker,sTier->attLine,utterance->attSp);
			att_cp(strlen(sTier->line),sTier->line,utterance->line,sTier->attLine,utterance->attLine);
			strcat(sTier->tuttline, utterance->speaker);
			strcat(sTier->tuttline, uttline);
			SpClusterCnt++;
		} else {
			if (SpCluster) {

				att_cp(strlen(sTier->line),sTier->line,utterance->speaker,sTier->attLine,utterance->attSp);
				att_cp(strlen(sTier->line),sTier->line,utterance->line,sTier->attLine,utterance->attLine);
				strcat(sTier->tuttline, utterance->speaker);
				strcat(sTier->tuttline, uttline);

				att_cp(0,utterance->speaker,sTier->speaker,utterance->attSp,sTier->attSp);
				att_cp(0,utterance->line,sTier->line,utterance->attLine,sTier->attLine);
				strcpy(uttline, sTier->tuttline);
				utterance->tlineno = sTier->tlineno;

				shiftLeftOneSpCluster(sTier);
/*
				att_cp(strlen(sTier->line),sTier->line,utterance->speaker,sTier->attLine,utterance->attSp);
				k = strlen(sTier->line);
				att_cp(k,sTier->line,utterance->line,sTier->attLine,utterance->attLine);
				att_cp(0,utterance->line,sTier->line,utterance->attLine,sTier->attLine);
				att_cp(0,sTier->line,sTier->line+k,sTier->attLine,sTier->attLine);

				strcat(sTier->tuttline, utterance->speaker);
				k = strlen(sTier->tuttline);
				strcat(sTier->tuttline, uttline);
				strcpy(uttline, sTier->tuttline);
				strcpy(sTier->tuttline, sTier->tuttline+k);

				att_cp(0,templineC,utterance->speaker,templineC3,utterance->attSp);
				att_cp(0,utterance->speaker,sTier->speaker,utterance->attSp,sTier->attSp);
				att_cp(0,sTier->speaker,templineC,sTier->attSp,templineC3);

				ttl = utterance->tlineno;
				utterance->tlineno = tl;
				tl = ttl;
*/
			}
			if (combo_string != 1) {
				for (k=0; uttline[k] != 0; k++) {
					if (uttline[k] == '[' && uS.isskip(uttline, k, &dFnt, MBF)) {
						while (uttline[k] != ']' && uttline[k] != EOS) k++;
						if (uttline[k] == EOS)
							k++;
					} else {
						if (uS.isskip(uttline, k, &dFnt, MBF))
							uttline[k] = ' ';
					}
				}
			}
#ifdef DEBUG_CLAN
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
#endif
			if (*utterance->speaker=='@' && CheckOutTier(utterance->speaker) && utterance->nextutt==utterance) {
				if (onlydata == 2 || onlydata == 4) {
					fputs("@Comment:\t-----------------------------------\n",fpout);
					fprintf(fpout,"@Comment:\t*** File \"%s\": ", oldfname);
					if (utterance->tlineno > 0) 
						fprintf(fpout,"line %ld.\n",utterance->tlineno);
					else
						fprintf(fpout,"\n");
				} else if (onlydata == 0) {
					fputs("----------------------------------------\n",fpout);
					fprintf(fpout,"*** File \"%s\": ",oldfname);
					if (utterance->tlineno > 0) 
						fprintf(fpout,"line %ld.\n",utterance->tlineno);
					else
						fprintf(fpout,"\n");
				}
				befprintout(TRUE);
				if (CntWUT == 1) {
					for (k=0L; uttline[k]; k++) {
						if (!isSpace(uttline[k]))
							uttline[k] = utterance->line[k];
					}
					if (cMediaFileName[0] != EOS)
						changeBullet(uttline, utterance->attLine);
					printout(utterance->speaker,uttline,utterance->attSp,utterance->attLine,FALSE);
				} else {
					if (cMediaFileName[0] != EOS)
						changeBullet(utterance->line, utterance->attLine);
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				}
				aftprintout(TRUE);
			} else if (checktier(utterance->speaker)) {
				findmatch(uttline);
			} else if (onlydata == 5 && utterance->nextutt==utterance) {
				befprintout(TRUE);
				if (CntWUT == 1) {
					for (k=0L; uttline[k]; k++) {
						if (!isSpace(uttline[k]))
							uttline[k] = utterance->line[k];
					}
					if (cMediaFileName[0] != EOS)
						changeBullet(uttline, utterance->attLine);
					printout(utterance->speaker,uttline,utterance->attSp,utterance->attLine,FALSE);
				} else {
					if (cMediaFileName[0] != EOS)
						changeBullet(utterance->line, utterance->attLine);
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				}
				aftprintout(TRUE);
			}
		}
	}
	if (sTier != NULL) {
		free(sTier);
	}
	if (!combinput)
		pr_result();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'g':
				if (*f == '1' && *(f+1) == EOS) combo_string = 1;
				else if (*f == '2' && *(f+1) == EOS) combo_string = 2;
				else if (*f == '3' && *(f+1) == EOS) isContinueSearch = FALSE;
				else if (*f == '4' && *(f+1) == EOS) includeUtteranceDelims = FALSE;
				else if (*f == '5' && *(f+1) == EOS) trvsearch = TRUE;
				else {
					fprintf(stderr,"Invalid argument for option: %s\n", f-2);
					fprintf(stderr,"Please choose +g1 to +g5\n");
					freeMem();
					cutt_exit(0);
				}
				break;
		case 'b':
				SpCluster = atoi(f);
				break;
		case 'd':
				if (*f == 'v' || *f == 'V')
					isEchoFlatmac = TRUE;
				else {
					onlydata=(char)(atoi(getfarg(f,f1,i))+1);
					if (onlydata < 0 || onlydata > OnlydataLimit) {
						fprintf(stderr, "The only +d levels allowed are 0-%d.\n", OnlydataLimit-1);
						cutt_exit(0);
					}
					if (onlydata == 3)
						puredata = 0;
				}
				break;
		case 's':
				if (patgiven != NULL) {
					fprintf(stderr, "COMBO: Only one \"s\" option is allowed!\n");
					freeMem();
					cutt_exit(0);
				}
				if (*(f-2) == '+')
					isIncludeTier = TRUE;
				else
					isIncludeTier = FALSE;
				patgiven = getfarg(f,f1,i);
				break;
		case 'z':
				addword('\0','\0',"+xxx");
				addword('\0','\0',"+yyy");
				addword('\0','\0',"+www");
				addword('\0','\0',"+*|xxx");
				addword('\0','\0',"+*|yyy");
				addword('\0','\0',"+*|www");
				maingetflag(f-2,f1,i);
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
	if (onlydata == 4 && combo_string > 0) {
		fprintf(stderr,"The use of +d3 option is illegal with +g option.\n");
		freeMem();
		cutt_exit(0);
	}
}

/*
combo +s@k1^@k2 c.cha
	*MOT:	text t(ex)t! more text to come. and so on?
	%syn:	( S V N ( S V O ) )
combo c.cha +s\! -r2
combo +t%syn c.cha +s\( -r2
combo -s *mot^*^e^*^f^i combo.txt
combo -s *mot^*^e^*^f^*^i combo.txt
combo -s d^*^!f^*^g combo.txt 
combo -s d^!f^*^g combo.txt
*/
