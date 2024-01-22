/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
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
#define IS_WIN_MODE TRUE

#if defined(CLAN_SRV)
extern char SRV_PATH[];
extern char SRV_NAME[];
#endif

#define COMBOSP struct combo_speakers_list
struct combo_speakers_list {
	char *fname;
	char *sp;
	char *ID;
	char isFound;
	float spMatch;
	float spUtts;
	COMBOSP *next_sp;
} ;

#define PAT_ELEM struct pattern_elements
PAT_ELEM {
	char *pat;
	MORWDLST *pat_feat;
	char whatType;
	char neg;
	char wild;
	char ref;
	char isCheckedForDups;
	char isAllNeg;
	char isElemMat;
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

enum {
	SPK_TIER = 1,
	MOR_TIER,
	GRA_TIER
} ;

struct combo_utts_list {
	char *line;
	FNType *fname;
	long lineno;
	struct combo_utts_list *NextUtt;
} ;

extern char R4;
extern char puredata;
extern char isPostcliticUse;	/* if '~' found on %mor tier break it into two words */
extern char isPrecliticUse;		/* if '$' found on %mor tier break it into two words */
extern char linkDep2Other;
extern char Preserve_dir;
extern char GExt[];
extern char stin_override;
extern char OverWriteFile;
extern char isWOptUsed;

static int  maxRef;
static int  firstmatch;
static int  SpCluster;
static int  StrMatch;
static int	stackAndsI;
static char *expression;
static char *patgiven;
static char *lastTxtMatch;
static char trvsearch;
static char isSearchMOR, isSearchGRA;
static char mftime;
static char isMorGRAItem;
static char isIncludeTier;
static char isEchoFlatmac;
static char includeUtteranceDelims;
static char includeTierCodeName;
static char showDupMatches;
static char isContinueSearch;
static long SpClusterLineno;
static short combo_string;
static unsigned long *mat;
static PAT_ELEM *origmac;
static PAT_ELEM *flatmac;
static PAT_ELEM *stackAnds[200];
static struct cdepTiers *extTier;
static struct combo_utts_list *rootUtts;
static REFS refArray[MAXREFS];
static IEWORDS *duplicatesList;
static COMBOSP *combo_head;

/* ******************** combo prototypes ********************** */
/* *********************************************************** */

void usage() {
	printf("Usage: combo [bN gN s %s] filename(s)\n",mainflgs());
	puts("+bN: search a N number-cluster unit(s)");
	puts("+g1: do a string oriented search on a whole tier. (Default: word oriented)");
	puts("+g2: do a string oriented search on just one word.");
	puts("+g3: find only first match on an utterance.");
	puts("+g4: exclude utterance delimiters from the search.");
	puts("+g5: \"^\" symbol used as an \"and\" operator, instead of \"followed by\" operator.");
	puts("+g6: include tier's code name in the search..");
	puts("+g7: do not count duplicate matches.");
	puts("+sS: output tiers that match specified search pattern S");
	puts("-sS: output tiers that do not match specified search pattern S");
	puts("     \"^*^\" means match zero or more, \"^_^\" means match exactly anyone item");
	puts("+dv: display all parsed individual parts of search pattern");
	puts("+d : outputs each matched sentence in a simple legal CHAT format");
	puts("+d1: outputs legal CHAT format with line numbers and file names");
	puts("+d2: outputs file names once per file only");
	puts("+d3: outputs ONLY matched words in the same format as +d1");
	puts("+d4: if string match is found, add codes and tiers to data file");
	puts("+d5: outputs ONLY matched words with words bracketed by first and last matched word (+d1 format)");
	puts("+d6: outputs SPREADSHEET with number of matches, utterances and ratio");
	puts("+d7: search words linked between one dependent tier and speaker or another dependent tier");
	puts("+d8: outputs SPREADSHEET format with keywords and utterances");
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
	if (p->pat_feat != NULL)
		p->pat_feat = freeMorWords(p->pat_feat);
	if (p->parenElem != NULL)
		freeMac(p->parenElem, isRemoveOR);
	if (p->andElem != NULL)
		freeMac(p->andElem, isRemoveOR);
	if (isRemoveOR && p->orElem != NULL)
		freeMac(p->orElem, isRemoveOR);
	free(p);
}

static COMBOSP *combo_freeSpeakers(COMBOSP *p) {
	COMBOSP *t;

	while (p != NULL) {
		t = p;
		p = p->next_sp;
		if (t->fname != NULL)
			free(t->fname);
		if (t->sp != NULL)
			free(t->sp);
		if (t->ID)
			free(t->ID);
		free(t);
	}
	return(NULL);
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
	combo_head = combo_freeSpeakers(combo_head);
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
	t->pat_feat = NULL;
	t->whatType = 0;
	t->neg = 0;
	t->ref = 0;
	if (trvsearch)
		t->wild = 1;
	else
		t->wild = 0;
	t->isCheckedForDups = FALSE;
	t->isAllNeg = FALSE;
	t->isElemMat = FALSE;
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
	int  i, j;
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

static PAT_ELEM *addToPats(PAT_ELEM *elem, char *pat) {
	char *pat_st;

	elem->pat = NULL;
	elem->pat_feat = NULL;
	if (linkDep2Other) {
		if ((pat[0] == 'm' || pat[0] == 'M') && isMorPat(pat+1)) {
			pat_st = pat + 1;
			elem->whatType = MOR_TIER;
			isSearchMOR = TRUE;
		} else if ((pat[0] == 'g' || pat[0] == 'G') && (pat[1] == '|' || isGraPat(pat+1))) {
			elem->whatType = GRA_TIER;
			isSearchGRA = TRUE;
			pat_st = strrchr(pat, '|');
			if (pat_st != NULL)
				pat_st++;
			else
				pat_st = pat + 1;
		} else {
			pat_st = pat;
			elem->whatType = SPK_TIER;
		}
	} else {
		pat_st = pat;
		elem->whatType = SPK_TIER;
	}

	elem->pat = (char *)malloc((strlen(pat_st)+1));
	if (elem->pat == NULL) {
		fprintf(stderr, "combo: Out of memory\n");
		freeMem();
		cutt_exit(0);
	}
	strcpy(elem->pat,pat_st);
	return(elem);
}

static long storepat(PAT_ELEM *elem, char *exp, long pos) {
	int p1, p2, len;
	char t;

	p1 = pos;
	do {
		t = FALSE;
		for (; ((!uS.isRightChar(exp,pos,'(',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,'!',&dFnt,C_MBF) && 
				 !uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) && !uS.isRightChar(exp,pos,')',&dFnt,C_MBF) &&
				 !uS.isRightChar(exp,pos,'^',&dFnt,C_MBF)) ||
				(uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) && (pos == 0 || uS.isRightChar(exp,pos-1,'&',&dFnt,C_MBF))) ||
				(t && uS.isRightChar(exp,pos,'+',&dFnt,C_MBF) && exp[pos+1] == ' ') ||
				(t && exp[pos] == '^')) && exp[pos] != '\n'; pos++) {
			if (uS.isRightChar(exp,pos,'[',&dFnt,C_MBF))
				t = TRUE;
			else if (uS.isRightChar(exp,pos,']',&dFnt,C_MBF))
				t = FALSE;
		}
		if (pos-1 < p1 || !uS.isRightChar(exp, pos-1, '\\', &dFnt, C_MBF)) {
			break;
		} else if (exp[pos] == '\n' || exp[pos] == EOS) {
			if (uS.isRightChar(exp, pos-1, '\\', &dFnt, C_MBF) && !uS.isRightChar(exp, pos-2, '\\', &dFnt, C_MBF)) {
				err("1; Unexpected ending.",exp,pos);
				return(-1L);
			}
			break;
		} else if (uS.isRightChar(exp, pos-1, '\\', &dFnt, C_MBF) && exp[pos] == '!' && exp[pos+1] == '\n') {
			break;
		} else
			pos++;
	} while (1) ;
	if (uS.isRightChar(exp, pos-1, '\\', &dFnt, C_MBF) && exp[pos] == '!' && exp[pos+1] == '\n') {
		if (exp[p1] == '\\')
			p1++;
		if (exp[pos] == '!')
			pos++;
	} else if (uS.isRightChar(exp,pos,'!', &dFnt, C_MBF) || uS.isRightChar(exp,pos,'(', &dFnt, C_MBF)) {
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
		} else {
			if (p1 == p2) {
				elem->wild = 1;
			} else
				p2++;
		}
		t = exp[p2];
		exp[p2] = EOS;
		strcpy(templineC2, exp+p1);
		exp[p2] = t;
		elem = addToPats(elem, templineC2);
		len = strlen(elem->pat) - 1;
		if (elem->pat[0] == '[' && elem->pat[len] == ']') {
			strcpy(templineC, "+");
			if ((elem->pat[1] == '+' || elem->pat[1] == '-') && elem->pat[2] == ' ') {
				strcat(templineC, "<");
				strcat(templineC, elem->pat+1);
				addword('P','i',templineC);
			} else {
				strcat(templineC, elem->pat);
				addword('\0','i',templineC);
			}
		} else if (elem->pat[0] == '&' || (elem->pat[0] == '*' && elem->pat[1] == '&')) {
			strcpy(templineC, "+");
			strcat(templineC, elem->pat);
			addword('s','i',templineC);
		}
		if (elem->pat[0] == EOS) {
			if (combo_string > 0) {
				err("character '*' as an item is not allowed in string mode, +g1 or +g2 option.", elem->pat, 0);
				return(-1L);
			}
			if (trvsearch) {
				err("character '*' as an item is not allowed with +g5 option.", elem->pat,0);
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
		strcpy(templineC2, exp+p1);
		exp[p2] = t;
		elem = addToPats(elem, templineC2);
		len = strlen(elem->pat) - 1;
		if (elem->pat[0] == '[' && elem->pat[len] == ']') {
			strcpy(templineC, "+");
			if ((elem->pat[1] == '+' || elem->pat[1] == '-') && elem->pat[2] == ' ') {
				strcat(templineC, "<");
				strcat(templineC, elem->pat+1);
				addword('P','i',templineC);
			} else {
				strcat(templineC, elem->pat);
				addword('\0','i',templineC);
			}
		} else if (elem->pat[0] == '&' || (elem->pat[0] == '*' && elem->pat[1] == '&')) {
			strcpy(templineC, "+");
			strcat(templineC, elem->pat);
			addword('s','i',templineC);
		}
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
		} else if (uS.isRightChar(exp,pos,'\n',&dFnt,C_MBF)) {
			pos++;
		} else if (uS.isRightChar(exp,pos,'\\',&dFnt,C_MBF)) {
			pos++;
			pos = storepat(elem, exp, pos);
			if (pos == -1L)
				return(-1L);
		} else {
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

static char storeRef(int ref, char *s, char *fname) {
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
		return(TRUE);
	}
	return(FALSE);
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
	char wd[256], *s, qt, isOpenAdded, isExtFunction, isPlusFound;
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
		if (uS.isUTF8(wd) || uS.isInvisibleHeader(wd) || (*wd == '#' && *(wd+1) == ' ') || combo_isBlankLine(wd))
			continue;
		isExtFunction = FALSE;
		s = wd;
		if (*s == '"' || *s == '\'') {
			qt = *s;
			maxRef++;
			*exp++ = '(';
			*exp++ = '\001';
			*exp++ = maxRef;
			isPlusFound = FALSE;
			for (s++; *s != '\n' && *s != qt && *s; s++) {
				if (*s == '+')
					isPlusFound = TRUE;
				*exp++ = *s;
			}
			if (*s != qt) {
				fprintf(stderr,"Search string in file \"%s\" is missing closing '%c'",fname,qt);
				freeMem();
				cutt_exit(0);
			}
			isExtFunction = storeRef(maxRef-1, s+1, fname);
			if (*(exp-1) == '+' || *(exp-1) == '^')
				exp--;
			*exp++ = ')';
			if (onlydata == 5 && isExtFunction && isPlusFound) {
				fprintf(stderr, "\nPlease do not use OR (+) symbol in search line:\n");
				fprintf(stderr, "%s", wd);
				fprintf(stderr, "Instead place each \"OR\" (+), alternative, on its own separate line.\n");
				freeMem();
				cutt_exit(0);
			}
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
			if ((pat[1] == 's' || pat[1] == 'S') && pat[2] == ':') {
				*exp++ = *pat++;
			} else {
				pat++;
				beg = pat;
				while (*pat != '+' && *pat != '^' && *pat != ')' && *pat)
					pat++;
				t = *pat;
				*pat = EOS;
				exp = GetIncludeFile(exp, beg);
				*pat = t;
				isFileFound = TRUE;
			}
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
			elem->pat_feat = NULL;
			elem->whatType = cur->whatType;
			elem->wild = cur->wild;
			elem->neg = cur->neg;
			elem->ref = cur->ref;
			elem->isElemMat = cur->isElemMat;
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
			elem->pat_feat = NULL;
			elem->whatType = cur->whatType;
			elem->wild = cur->wild;
			elem->neg = cur->neg;
			elem->ref = cur->ref;
			elem->isElemMat = cur->isElemMat;
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
		elem->pat_feat = NULL;
		elem->whatType = flatp->whatType;
		elem->wild = flatp->wild;
		elem->neg = flatp->neg;
		elem->ref = flatp->ref;
		elem->isElemMat = flatp->isElemMat;
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

static void duplicateFlatMacs(char isResetDuplicates) {
	PAT_ELEM *elem, *elemRoot, *flatp, *lastElem;

	if (isResetDuplicates) {
		duplicatesList = freeIEWORDS(duplicatesList);
		for (flatp=flatmac; flatp != NULL; flatp=flatp->orElem) {
			flatp->isCheckedForDups = FALSE;
		}
	}
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

static void flatten_expr(PAT_ELEM *cur, char isResetDuplicates) {
	if (cur->parenElem != NULL) {
		addParenToLastFlatMacs(cur);
		addToStack(cur->parenElem);
		flatten_expr(cur->parenElem, FALSE);
		stackAndsI--;
	} else if (cur->pat != NULL) {
		addPatToLastFlatMacs(cur);
	}
	if (cur->andElem != NULL) {
		addAndToFlatMacs(cur);
		addToStack(cur->andElem);
		flatten_expr(cur->andElem, FALSE);
		stackAndsI--;
	}
	if (cur->orElem != NULL) {
		duplicateFlatMacs(isResetDuplicates);
		flatten_expr(cur->orElem, isResetDuplicates);
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
	int  i;
	char isNeg;

	if (first) {
		patgiven = NULL;
		origmac = NULL;
		flatmac = NULL;
		extTier = NULL;
		combo_head = NULL;
		isIncludeTier = TRUE;
		mftime = 1;
		isMorGRAItem = FALSE;
		trvsearch = FALSE;
		isSearchMOR = FALSE;
		isSearchGRA = FALSE;
		SpCluster = 0;
		combo_string = 0;
		isContinueSearch = TRUE;
		maxRef = 0;
		rootUtts = NULL;
		isEchoFlatmac = FALSE;
		includeUtteranceDelims = TRUE;
		includeTierCodeName = FALSE;
		showDupMatches = TRUE;
		isPostcliticUse = TRUE;
		isPrecliticUse = TRUE;
		addword('\0','\0',"+0*");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
		addword('\0','\0',"+cm|*");
		addword('\0','\0',"+bq|*"); //2019-04-29
		addword('\0','\0',"+eq|*"); //2019-04-29
		addword('\0','\0',"+bq2|*");
		addword('\0','\0',"+eq2|*");
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
		if (linkDep2Other) {
			if (combo_string > 0) {
				fprintf(stderr,"\nThe +d7 can't be used with +g1 or +g2 option\n");
				cutt_exit(0);
			}
			if (onlydata >= 5 && onlydata <= 6) {
				fprintf(stderr,"\nThe +d7 can not be used with +d4 or +d5 option\n");
				cutt_exit(0);
			}
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == ';' ||
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
		flatten_expr(origmac, TRUE);
		if (isEchoFlatmac)
			fprintf(stderr, "\n*** Individual search patterns:\n");
		removeDuplicateFlatMacs(isEchoFlatmac);
		if (isEchoFlatmac)
			fprintf(stderr, "\n");
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
			if (!R4) {
				isPostcliticUse = FALSE;
				isPrecliticUse = FALSE;
			}
		}
		if (linkDep2Other) {
			if (fDepTierName[0] == EOS && chatmode) {
				if (isSearchGRA && isSearchMOR) {
					strcpy(fDepTierName, "%mor:");
					strcpy(sDepTierName, "%gra:");
					isMorGRAItem = TRUE;
				} else if (isSearchGRA) {
					strcpy(fDepTierName, "%gra:");
				} else
					strcpy(fDepTierName, "%mor:");
			}
			if (fDepTierName[0] != EOS && !checktier(fDepTierName))
				maketierchoice(fDepTierName,'+',FALSE);
			if (sDepTierName[0] != EOS && !checktier(sDepTierName))
				maketierchoice(sDepTierName,'+',FALSE);
			FilterTier = 2;
		}
		if (!linkDep2Other) {
			fDepTierName[0] = EOS;
			sDepTierName[0] = EOS;
		}
		if (onlydata == 7 || onlydata == 9) {
			if (isWOptUsed) {
				fputs("+w option can't be used with +d6 or +d8 options.\n",stderr);
				cutt_exit(0);
			}
			maketierchoice("@ID:",'+',FALSE);
//			OverWriteFile = TRUE;
		}
	}
	if (!combinput || first)
		StrMatch = 0;
}

static void combo_FindSpeaker(char *sp, char *ID, int spUtts, int spMatch) {
	COMBOSP *ts;

	uS.remblanks(sp);
	if (combo_head == NULL) {
		combo_head = NEW(COMBOSP);
		ts = combo_head;
	} else {
		for (ts=combo_head; 1; ts=ts->next_sp) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE) && uS.mStricmp(ts->fname, oldfname) == 0) {
				ts->spUtts = ts->spUtts + (float)spUtts;
				ts->spMatch = ts->spMatch + (float)spMatch;
				if (ID == NULL)
					ts->isFound = TRUE;
				return;
			}
			if (ts->next_sp == NULL)
				break;
		}
		ts->next_sp = NEW(COMBOSP);
		ts = ts->next_sp;
	}
	if (ts == NULL)
		out_of_mem();
// ".xls"
	if ((ts->fname=(char *)malloc(strlen(oldfname)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(ts->fname, oldfname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(ts->sp, sp);
	if (ID == NULL) {
		ts->ID = NULL;
		ts->isFound = TRUE;
	} else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL) {
			out_of_mem();
		}
		strcpy(ts->ID, ID);
		ts->isFound = FALSE;
	}
	ts->spUtts = (float)spUtts;
	ts->spMatch = (float)spMatch;
	ts->next_sp = NULL;
}

static void outputExcel(void) {
	char tCombinput;
	COMBOSP *ts;
	FNType *fn, StatName[FNSize];
	FILE *StatFpOut = NULL;

	if (Preserve_dir)
		strcpy(StatName, wd_dir);
	else
		strcpy(StatName, od_dir);
	addFilename2Path(StatName, "stat");
	strcat(StatName, GExt);
	strcat(StatName, ".cex");
	AddCEXExtension = ".xls";
	parsfname(StatName, FileName1, "");
	tCombinput = combinput;
	combinput = FALSE;
	if (f_override)
		StatFpOut = stdout;
	else if ((StatFpOut=openwfile(StatName, FileName1, NULL)) == NULL) {
		fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",FileName1);
	}
	combinput = tCombinput;
	if (StatFpOut != NULL) {
		excelHeader(StatFpOut, newfname, 95);
		excelRow(StatFpOut, ExcelRowStart);
		excelCommasStrCell(StatFpOut, "File,Language,Corpus,Speaker,Age,Sex,Group,Race,SES,Role,Education,Custom field,# matches,# utts,matches/utts");
		excelRow(StatFpOut, ExcelRowEnd);
		for (ts=combo_head; ts != NULL; ts=ts->next_sp) {
			if (ts->isFound) {
				excelRow(StatFpOut, ExcelRowStart);
				excelStrCell(StatFpOut, ts->fname);
				if (ts->ID != NULL)
					excelOutputID(StatFpOut, ts->ID);
				else {
					excelCommasStrCell(StatFpOut, ".,.");
					excelCommasStrCell(StatFpOut, ts->sp);
					excelCommasStrCell(StatFpOut, ".,.,.,.,.,.,.,.");
				}
				excelNumCell(StatFpOut, "%.0f", ts->spMatch);
				excelNumCell(StatFpOut, "%.0f", ts->spUtts);
				excelNumCell(StatFpOut, "%.3f", ts->spMatch/ts->spUtts);
				excelRow(StatFpOut, ExcelRowEnd);
			}
		}
		excelFooter(StatFpOut);
		if (!f_override) {
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
		}
		fclose(StatFpOut);
	}
}

static struct combo_utts_list *combo_AddLinearUtts(struct combo_utts_list *root, long lineno, char *line) {
	struct combo_utts_list *u;

	if (root == NULL) {
		root = NEW(struct combo_utts_list);
		if (root == NULL)
			out_of_mem();
		u = root;
	} else {
		for (u=root; u->NextUtt != NULL; u=u->NextUtt) ;
		u->NextUtt = NEW(struct combo_utts_list);
		if (u->NextUtt == NULL) out_of_mem();
		u = u->NextUtt;
	}
	u->NextUtt = NULL;
	u->lineno = lineno;
	u->line = (char *)malloc(strlen(line)+1);
	if (u->line == NULL)
		out_of_mem();
	strcpy(u->line, line);
#if defined(CLAN_SRV)
	int  len;
	FNType *s;

	if (oldfname[0] == PATHDELIMCHR) {
		len = strlen(SRV_PATH);
		for (s=oldfname; *s != EOS; s++) {
			if (uS.mStrnicmp(SRV_PATH, s, len) == 0) {
				s = s + len;
				if (*s == PATHDELIMCHR)
					s++;
				break;
			}
		}
		if (s == EOS)
			s = oldfname;
	} else
		s = oldfname;
	u->fname = (FNType *)malloc((strlen(s)+1)*sizeof(FNType));
	if (u->fname == NULL)
		out_of_mem();
	strcpy(u->fname, s);
#else
	u->fname = (FNType *)malloc((strlen(oldfname)+1)*sizeof(FNType));
	if (u->fname == NULL)
		out_of_mem();
	strcpy(u->fname, oldfname);
#endif
	return(root);
}

static void excelCOMBOStrCell(FILE *fp, const char *st) {
	int  i, j;

	if (st != NULL) {
		j = 0;
		for (i=0; st[i] != EOS; i++) {
			if (st[i] == '\n' && st[i+1] == '%') {
				templineC4[j] = EOS;
				if (templineC4[0] != EOS) {
					excelTextToXML(templineC3, templineC4, templineC4+strlen(templineC4));
					fprintf(fp, "    <Cell><Data ss:Type=\"String\">%s</Data></Cell>\n", templineC3);
				}
				j = 0;
			} else
				templineC4[j++] = st[i];
		}
		templineC4[j] = EOS;
		if (templineC4[0] != EOS) {
			excelTextToXML(templineC3, templineC4, templineC4+strlen(templineC4));
			fprintf(fp, "    <Cell><Data ss:Type=\"String\">%s</Data></Cell>\n", templineC3);
		}
	}
}

static void combo_outputIDInfo(FILE *fp, char *sp, char *fname) {
	char *s;
	COMBOSP *ts;

	s = strrchr(fname, PATHDELIMCHR);
	if (s == NULL)
		s = fname;
	else
		s++;
	excelStrCell(fp, s);
	for (ts=combo_head; ts != NULL; ts=ts->next_sp) {
		if (uS.partcmp(ts->sp, sp, FALSE, FALSE) && uS.mStricmp(ts->fname, fname) == 0) {
			if (ts->ID != NULL)
				excelOutputID(fp, ts->ID);
			else {
				excelCommasStrCell(fp, ".,.");
				excelCommasStrCell(fp, ts->sp);
				excelCommasStrCell(fp, ".,.,.,.,.,.,.,.");
			}
			return;
		}
	}
	excelCommasStrCell(fp, ".,.");
	excelStrCell(fp, sp);
	excelCommasStrCell(fp, ".,.,.,.,.,.,.,.");
}

static void outputExcelUtts(struct combo_utts_list *u, char *keyword) {
	int  i;
	char tCombinput;
	struct combo_utts_list *t;
	FNType *fn, StatName[FNSize];
	FILE *StatFpOut = NULL;

	if (Preserve_dir)
		strcpy(StatName, wd_dir);
	else
		strcpy(StatName, od_dir);
	addFilename2Path(StatName, "stat");
	strcat(StatName, GExt);
	strcat(StatName, ".cex");
	AddCEXExtension = ".xls";
	parsfname(StatName, FileName1, "");
	tCombinput = combinput;
	combinput = FALSE;
	if (f_override)
		StatFpOut = stdout;
	else if ((StatFpOut=openwfile(StatName, FileName1, NULL)) == NULL) {
		fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",FileName1);
	}
	combinput = tCombinput;
	if (StatFpOut != NULL) {
		excelHeader(StatFpOut, newfname, 95);
		excelRow(StatFpOut, ExcelRowStart);
		excelCommasStrCell(StatFpOut, "File,Language,Corpus,Speaker,Age,Sex,Group,Race,SES,Role,Education,Custom field,Line_number, Utterance");
		excelRow(StatFpOut, ExcelRowEnd);
		while (u != NULL) {
			if (u->line[0] == '*') {
				for (i=0L; u->line[i] != EOS && u->line[i] != ':'; i++)
					spareTier1[i] = u->line[i];
				if (u->line[i] == ':')
					spareTier1[i] = EOS;
				else
					strcpy(spareTier1, "NO_SPEAKER_NAME");
			} else
				strcpy(spareTier1, "NO_SPEAKER_NAME");
			excelRow(StatFpOut, ExcelRowStart);
			combo_outputIDInfo(StatFpOut, spareTier1, u->fname);
			excelLongNumCell(StatFpOut, "%ld", u->lineno);
			remove_CRs_Tabs(u->line);
			excelCOMBOStrCell(StatFpOut, u->line);
			excelRow(StatFpOut, ExcelRowEnd);
			t = u;
			u = u->NextUtt;
			free(t->line);
			free(t->fname);
			free(t);
		}
		excelFooter(StatFpOut);
		if (!f_override) {
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
		}
		fclose(StatFpOut);
	}
}

static void combo_pr_result(void) {
	if (isIncludeTier == TRUE && onlydata != 7 && onlydata != 9) {
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
		OnlydataLimit = 8;
		UttlineEqUtterance = FALSE;
		bmain(argc,argv,combo_pr_result);
		if (onlydata == 7)
			outputExcel();
		else if (onlydata == 9)
			outputExcelUtts(rootUtts, NULL);
		freeMem();
	}
}

static void shiftLeftOneSpCluster(UTTER *sTier) {
	char c;
	long i;

	for (i=0L; sTier->line[i]; i++) {
		if (sTier->line[i] == '\n') {
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

static char isGRAFirst(char *s) {
	int i, vb;

	vb = 0;
	for (i=0; s[i] != EOS; i++) {
		if (s[i] != '|')
			vb++;
		else if (!isdigit(s[i]))
			break;
	}
	if (vb == 2)
		return(TRUE);
	return(FALSE);
}

static int morMatch(char *s, PAT_ELEM *tm, int i, char end) {
	int  j;
	char t, isMatchFound, word[BUFSIZ+1];

	if (tm->pat_feat == NULL) {
		tm->pat_feat = makeMorSeachList(tm->pat_feat, &t, tm->pat, 'i');
		if (tm->pat_feat == NULL) {
			fprintf(stderr,"*** Search pattern eror: %s.\n", tm->pat);
			fprintf(stderr,"*** PLEASE READ MESSAGE ABOVE PREVIOUS LINE ***\n");
			freeMem();
			cutt_exit(0);
		}
	}
	j = 0;
	if (isMorGRAItem) {
		if (isGRAFirst(s)) {
			while (s[i] != end && s[i] != EOS) {
				if (s[i] == (char)0xE2 && s[i+1] == (char)0x87 && s[i+2] == (char)0x94) {
					i += 3;
					break;
				}
				i++;
			}
			while (s[i] != end && s[i] != EOS) {
				word[j++] = s[i++];
			}
		} else {
			while (s[i] != end && s[i] != EOS) {
				if (s[i] == (char)0xE2 && s[i+1] == (char)0x87 && s[i+2] == (char)0x94)
					break;
				word[j++] = s[i++];
			}
		}
	} else {
		while (s[i] != end && s[i] != EOS) {
			word[j++] = s[i++];
		}
	}
	word[j] = EOS;
	if (strchr(word, '|') != NULL) {
		isMatchFound = isMorPatMatchedWord(tm->pat_feat, word);
		if (isMatchFound == 2) {
			freeMem();
			cutt_exit(0);
		}
		if (isMatchFound) {
			while (s[i] != end && s[i] != EOS)
				i++;
			return(i);
		}
	}
	return(-1);
}

static int graMatch(char *s, PAT_ELEM *tm, int i, char end) {
	int  j;
	char *gra_word, word[BUFSIZ+1];

	j = 0;
	if (isMorGRAItem) {
		if (isGRAFirst(s)) {
			while (s[i] != end && s[i] != EOS) {
				if (s[i] == (char)0xE2 && s[i+1] == (char)0x87 && s[i+2] == (char)0x94)
					break;
				word[j++] = s[i++];
			}
		} else {
			while (s[i] != end && s[i] != EOS) {
				if (s[i] == (char)0xE2 && s[i+1] == (char)0x87 && s[i+2] == (char)0x94) {
					i += 3;
					break;
				}
				i++;
			}
			while (s[i] != end && s[i] != EOS) {
				word[j++] = s[i++];
			}
		}
	} else {
		while (s[i] != end && s[i] != EOS) {
			word[j++] = s[i++];
		}
	}
	word[j] = EOS;
	gra_word = strrchr(word, '|');
	if (gra_word != NULL) {
		if (uS.patmat(gra_word+1, tm->pat)) {
			while (s[i] != end && s[i] != EOS)
				i++;
			return(i);
		}
	}
	return(-1);
}

static int strMatch(char *s, char *pat, int i, char end) {
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
						return(-1);
				}
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

static int subindex(char *s, PAT_ELEM *tm, int i, char end) {
	int res;

	if (tm->whatType == MOR_TIER) {
		res = morMatch(s, tm, i, end);
	} else if (tm->whatType == GRA_TIER) {
		res = graMatch(s, tm, i, end);
	} else {
		res = strMatch(s, tm->pat, i, end);
	}
	return(res);
}

static int CMatch(char *s, PAT_ELEM *tm, char wild, int *wildOffset) { /* return index of pat in s, -1 if no Match */
	register int i, j;

	*wildOffset = 0;
	i = 0;
	if (combo_string <= 0) {
		while (s[i] == ' ' || s[i] == '\n')
			i++;
		if (tm->pat[0] == EOS) {
			firstmatch = 0;
			return(0);
		} else if (!strcmp(tm->pat, "_")) {
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
				if ((j=subindex(s, tm, i, ' ')) != -1) {
					while (s[j] == ' ' || s[j] == '\n')
						j++;
					*wildOffset = i;
					return(j);
				}
				while (s[i] != ' ' && s[i] != 0)
					i++;
				while (s[i] == ' ' || s[i] == '\n')
					i++;
			} else {
				if ((j=subindex(s, tm, i, EOS)) != -1)
					return(j);
				i++;
			}
		}
		return(-1);
	} else {
		if (combo_string <= 0) {
			if ((j=subindex(s, tm, i, ' ')) != -1) {
				while (s[j] == ' ' || s[j] == '\n')
					j++;
				return(j);
			}
		} else {
			if ((j=subindex(s, tm, i, EOS)) != -1)
				return(j);
		}
		return(-1);
	}
}

static char *match(char *txt, PAT_ELEM *tm, char wild, char *isMatched) {
	int last = -1, wildOffset;
	char *tmp, negC1, negC2, *negSf, *negSl, *lastTxt, isNegFound;

	isNegFound = FALSE;
	negSf = NULL;
	negSl = NULL;
	negC1 = 0;
	negC2 = 0;
	if (tm->pat != NULL) {
		if ((last=CMatch(txt, tm, wild, &wildOffset)) != -1) {
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
					if (wildOffset != 0)
						tm->matchIndex = (int)((txt + wildOffset) - uttline);
					else
						tm->matchIndex = (int)((txt + firstmatch) - uttline);
					tm->isElemMat = TRUE;
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
	int last = -1, wildOffset;
	char *tmp;

	if (tm->pat != NULL) {
		if ((last=CMatch(txt, tm, wild, &wildOffset)) != -1) {
			if (tm->pat[0] != EOS) {
				tm->matchIndex = (int)((txt + firstmatch + wildOffset) - uttline);
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

static void noDupMatches(PAT_ELEM *cur, int matchnum, char *isDupMatch) {
	int i;
	unsigned long t;

	if (cur->parenElem != NULL) {
		if (!cur->neg)
			noDupMatches(cur->parenElem, matchnum, isDupMatch);
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
			if ((mat[i] & t) == FALSE)
				*isDupMatch = FALSE;
		}
	}
	if (*isDupMatch == FALSE)
		return;
	if (cur->andElem != NULL) {
		noDupMatches(cur->andElem, matchnum, isDupMatch);
	}
}

static long computeLineNumber(UTTER *sTier) {
/*
	long i, ln;

	ln = 0;
	for (i=0L; sTier->line[i]; i++) {
		if (sTier->line[i] == '\n') {
			ln++;
			if (sTier->line[i+1] == '*') {
				i++;
				break;
			}
		}
	}
	if (ln > lineno)
		return(ln);
	else
		ln = lineno - ln;
//	return(ln);
//	return(lineno);
*/
	if (SpCluster)
		return(SpClusterLineno);
	else
		return(sTier->tlineno);
}

static void isFlatNegMat(PAT_ELEM *cur, char isNegParan, char *isNegMatched) {
	if (cur->parenElem != NULL) {
		if (cur->neg) {
			isFlatNegMat(cur->parenElem, TRUE, isNegMatched);
		} else {
			isFlatNegMat(cur->parenElem, isNegParan, isNegMatched);
		}
	} else if (cur->pat != NULL) {
		if (isNegParan && cur->pat[0] != EOS && cur->isElemMat) {
			*isNegMatched = TRUE;
			return;
		}
	}
	if (*isNegMatched == TRUE)
		return;
	if (cur->andElem != NULL) {
		isFlatNegMat(cur->andElem, isNegParan, isNegMatched);
	}
}

static void resetFlatElemMat(PAT_ELEM *cur) {
	cur->isElemMat = FALSE;
	if (cur->parenElem != NULL) {
		resetFlatElemMat(cur->parenElem);
	}
	if (cur->andElem != NULL) {
		resetFlatElemMat(cur->andElem);
	}
}

static void morifyLine(char *line) {
	int i, j;
	char isMorFound;

	isMorFound = FALSE;
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '\n') {
			if (line[i+1] == '%' && line[i+2] == 'm' && line[i+3] == 'o' && line[i+4] == 'r') {
				isMorFound = TRUE;
				i += 5;
				line[i++] = ':';
				for (; !isSpace(line[i]) && line[i] != '\n'; i++)
					line[i] = ' ';
				break;
			}
		} 
		// E2 87 94
	}
	if (isMorFound == FALSE)
		return;
	
	while (line[i] != EOS) {
//		if (line[i] == '\n' || line[i] == '\t')
//			line[i] = ' ';
		if (line[i] == (char)0xE2 && line[i+1] == (char)0x87 && line[i+2] == (char)0x94) {
			for (j=i+3; isdigit(line[j]) || line[j] == '|'; j++) ;
			line[i++] = '@';
			while (!isSpace(line[j]) && line[j] != '\n' && line[j] != EOS) {
				line[i] = line[j];
				i++;
				j++;
			}
			for (; !isSpace(line[i]) && line[i] != '\n' && line[i] != EOS; i++)
				line[i] = ' ';
		}
		i++;
	}
}

static int findmatch(char *txt) {
	int  matchnum;
	long i, k;
	char *orgTxt, *wTxt, *tTxt, tChr1, tChr2;
	char *outLine;
	char wild, isMatched, isStop, isNegMatched = FALSE, isDupMatch;
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
			resetFlatElemMat(flatp);
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
							if (showDupMatches == FALSE && matchnum > 0) {
								isDupMatch = TRUE;
								noDupMatches(flatp, matchnum-1, &isDupMatch);
								if (isDupMatch == FALSE) {
									setmat(flatp, matchnum);
									matchnum++;
								}
							} else {
								setmat(flatp, matchnum);
								matchnum++;
							}
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
								if (showDupMatches == FALSE && matchnum > 0) {
									isDupMatch = TRUE;
									noDupMatches(flatp, matchnum-1, &isDupMatch);
									if (isDupMatch == FALSE) {
										setmat(flatp, matchnum);
										matchnum++;
									}
								} else {
									setmat(flatp, matchnum);
									matchnum++;
								}
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
							if (showDupMatches == FALSE && matchnum > 0) {
								isDupMatch = TRUE;
								noDupMatches(flatp, matchnum-1, &isDupMatch);
								if (isDupMatch == FALSE) {
									setmat(flatp, matchnum);
									matchnum++;
								}
							} else {
								setmat(flatp, matchnum);
								matchnum++;
							}
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
			fprintf(fpout,"line %ld.\n", computeLineNumber(utterance));
		else
			fprintf(fpout,"\n");
		fprintf(stderr,"WARNING: Number of matches has exceeded the limit of 32\n\n");
	}
*/

	isNegMatched = FALSE;
	for (flatp=flatmac; flatp != NULL; flatp=flatp->orElem) {
		isFlatNegMat(flatp, FALSE, &isNegMatched);
		if (isNegMatched == TRUE) {
			matchnum = 0;
			break;
		}
	}
	if ((matchnum > 0 && isIncludeTier == TRUE) || (matchnum == 0 && isIncludeTier == FALSE)) {
		if (onlydata == 7) {
		} else if (onlydata == 9) {
			strcpy(spareTier1, utterance->speaker);
			strcat(spareTier1, outLine);
			rootUtts = combo_AddLinearUtts(rootUtts, utterance->tlineno, spareTier1);
		} else if (onlydata == 2 || onlydata == 4 || onlydata == 6) {
			fputs("@Comment:\t-----------------------------------\n",fpout);
			fprintf(fpout,"@Comment:\t*** File \"%s\": ",oldfname);
			if (utterance->tlineno > 0) 
				fprintf(fpout,"line %ld;\n", computeLineNumber(utterance));
			else
				fprintf(fpout,"\n");
		} else if (onlydata == 0) {
			fputs("----------------------------------------\n",fpout);
			fprintf(fpout,"*** File \"%s\": ",oldfname);
			if (utterance->tlineno > 0)
				fprintf(fpout,"line %ld.\n", computeLineNumber(utterance));
			else
				fprintf(fpout,"\n");
		}
		befprintout(TRUE);
		if (onlydata == 7 || onlydata == 9) {
		} else if (onlydata == 0) {
			display(utterance->speaker,outLine,fpout);
		} else if (onlydata == 6) {
			long ei, matI;
			char sq, isMatUsed[32];
			unsigned long curMat, tMat;

			if (SpCluster == 0)
				printout(utterance->speaker,NULL,utterance->attSp,NULL,FALSE);
			for (i=0L; utterance->line[i]; i++) {
				templineC4[i] = 0;
			}
			for (i=0L; i < 32; i++) {
				isMatUsed[i] = FALSE;
			}
			for (k=0L; utterance->line[k]; k++) {
				if (mat[k] > 0L) {
					ei = k;
					tMat = mat[k];
					for (matI=0; matI < 32; matI++) {
						if (matI > 0) {
							tMat = tMat >> 1;
							if (tMat <= 0L)
								break;
						}
						if (tMat & 1L) {
							curMat = matI + 1;
							if (!isMatUsed[curMat]) {
								isMatUsed[curMat] = TRUE;
								for (i=k+1; utterance->line[i]; i++) {
									if (mat[i] > 0L) {
										tMat = mat[i];
										for (matI=0; matI < 32; matI++) {
											if (matI > 0) {
												tMat = tMat >> 1;
												if (tMat <= 0L)
													break;
											}
											if (tMat & 1L) {
												if (curMat == matI+1) {
													if (!uS.isskip(utterance->line,i,&dFnt,MBF) ||
														uS.isRightChar(utterance->line,i,'[',&dFnt,MBF) || uS.isRightChar(utterance->line,i,']',&dFnt,MBF)) {
														sq = (char)uS.isRightChar(utterance->line,i,'[',&dFnt,MBF);
														for (; (!uS.isskip(utterance->line,i,&dFnt,MBF)  || sq) && utterance->line[i]; i++) {
															if (uS.isRightChar(utterance->line,i,'[',&dFnt,MBF))
																sq = TRUE;
															else if (uS.isRightChar(utterance->line,i,']',&dFnt,MBF))
																sq = FALSE;
														}
														i--;
													}
													ei = i;
													break;
												}
											}
										}
									}
								}
								for (i=k; i <= ei; i++)
									templineC4[i] = 1;
							}
						}
					}
				}
			}
			for (k=0L; utterance->line[k]; k++) {
				if (templineC4[k] == 1) {
					putc(outLine[k],fpout);
				} else {
					if (utterance->line[k]=='\n' || utterance->line[k]=='\t')
						putc(outLine[k], fpout);
					else
						putc(' ',fpout);
//					mat[k] = 0L;
				}
			}
		} else if (onlydata == 4) {
			int  oi;
			char sq;

			oi = 0;
			templineC4[oi] = EOS;
			if (SpCluster == 0) {
				strcpy(templineC4, utterance->speaker);
				oi = strlen(templineC4);
			}
			if (linkDep2Other == TRUE)
				morifyLine(outLine);
			for (k=0L; outLine[k]; k++) {
				if (linkDep2Other == TRUE) {
					if (outLine[k] == '\n' && outLine[k+1] == '%' && outLine[k+2] == 'm' &&
						outLine[k+3] == 'o' && outLine[k+4] == 'r' && outLine[k+5] == ':') {
						templineC4[oi] = EOS;
						strcat(templineC4, "\n%mor:\t");
						oi = strlen(templineC4);
						k += 6;
					}
				}
				if (mat[k] > 0L) {
					if (!uS.isskip(utterance->line,k,&dFnt,MBF) || 
						uS.isRightChar(utterance->line,k,'[',&dFnt,MBF) || uS.isRightChar(utterance->line,k,']',&dFnt,MBF)) {
						sq = (char)uS.isRightChar(utterance->line,k,'[',&dFnt,MBF);
						for (; (!uS.isskip(utterance->line,k,&dFnt,MBF)  || sq) && utterance->line[k]; k++) {
							if (uS.isRightChar(utterance->line,k,'[',&dFnt,MBF))
								sq = TRUE;
							else if (uS.isRightChar(utterance->line,k,']',&dFnt,MBF))
								sq = FALSE;
							templineC4[oi++] = outLine[k];
//							mat[k] = 0L;
						}
						k--;
					} else {
						templineC4[oi++] = outLine[k];
//						mat[k] = 0L;
					}
				} else {
					if (utterance->line[k]=='\n' || utterance->line[k]=='\t')
						templineC4[oi++] = outLine[k];
					else
						templineC4[oi++] = ' ';
//					mat[k] = 0L;
				}
			}
			uS.remFrontAndBackBlanks(templineC4);
			removeExtraSpace(templineC4);
			fprintf(fpout, "%s\n", templineC4);
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
	return(matchnum);
}

void call() {
	long k;
	int  SpClusterCnt, res;
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
		SpClusterLineno = 1L;
	} else {
		sTier = NULL;
		SpClusterLineno = lineno;
	}
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (onlydata == 7 || onlydata == 9) {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC2, TRUE)) {
					uS.remblanks(utterance->line);
					combo_FindSpeaker(templineC2, utterance->line, 0, 0);
				}
				continue;
			}
		}
		if (SpCluster && *utterance->speaker == '@') {
			SpClusterCnt = 1;
			sTier->speaker[0] = EOS;
			sTier->line[0] = EOS;
			sTier->tuttline[0] = EOS;
		} else if (SpCluster && SpClusterCnt < SpCluster) {
			if (SpClusterCnt == 1) {
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
			if (linkDep2Other == TRUE) {
				if (strchr(uttline, dMarkChr) != NULL) {
					combineMainDepWords(utterance->line, FALSE);
					combineMainDepWords(uttline, FALSE);
				}
				if (onlydata == 4)
					strcpy(utterance->line, uttline);
				for (k=0; uttline[k] != EOS; k++) {
					if (!includeTierCodeName) {
						if (uttline[k] == '\n' && uttline[k+1] == '%') {
							while (uttline[k] != ':' && !isSpace(uttline[k]) && uttline[k] != EOS) {
								uttline[k++] = ' ';
							}
							if (uttline[k] == ':')
								uttline[k] = ' ';
						}
					}
					if (uttline[k] == '$' || uttline[k] == '~')
						uttline[k] = ' ';
				}
			}
			if (combo_string != 1) {
				for (k=0; uttline[k] != EOS; k++) {
					if (uttline[k] == '[' && uS.isskip(uttline, k, &dFnt, MBF)) {
						while (uttline[k] != ']' && uttline[k] != EOS)
							k++;
						if (uttline[k] == EOS)
							break;
					} else {
						if (uttline[k] == sMarkChr || uttline[k] == dMarkChr) {
						} else if (uS.isskip(uttline, k, &dFnt, MBF)) {
							uttline[k] = ' ';
						}
					}
				}
/* 2018-04-05
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
*/
			}
#ifdef DEBUG_CLAN
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
#endif
			if (*utterance->speaker=='@' && CheckOutTier(utterance->speaker) && utterance->nextutt==utterance) {
				if (onlydata == 2 || onlydata == 4 || onlydata == 6) {
					fputs("@Comment:\t-----------------------------------\n",fpout);
					fprintf(fpout,"@Comment:\t*** File \"%s\": ", oldfname);
					if (utterance->tlineno > 0) 
						fprintf(fpout,"line %ld.\n", computeLineNumber(utterance));
					else
						fprintf(fpout,"\n");
				} else if (onlydata == 0) {
					fputs("----------------------------------------\n",fpout);
					fprintf(fpout,"*** File \"%s\": ",oldfname);
					if (utterance->tlineno > 0) 
						fprintf(fpout,"line %ld.\n", computeLineNumber(utterance));
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
				res = findmatch(uttline);
				if (onlydata == 7 || onlydata == 9)
					combo_FindSpeaker(utterance->speaker, NULL, 1, res);
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
		if (SpCluster)
			SpClusterLineno = lineno;
	}
	if (sTier != NULL) {
		free(sTier);
	}
	if (!combinput)
		combo_pr_result();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'g':
				if (*f == '1' && *(f+1) == EOS)
					combo_string = 1;
				else if (*f == '2' && *(f+1) == EOS)
					combo_string = 2;
				else if (*f == '3' && *(f+1) == EOS)
					isContinueSearch = FALSE;
				else if (*f == '4' && *(f+1) == EOS)
					includeUtteranceDelims = FALSE;
				else if (*f == '5' && *(f+1) == EOS)
					trvsearch = TRUE;
				else if (*f == '6' && *(f+1) == EOS)
					includeTierCodeName = TRUE;
				else if (*f == '7' && *(f+1) == EOS)
					showDupMatches = FALSE;
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
				if (*f == 'v' || *f == 'V') {
					isEchoFlatmac = TRUE;
				} else if (*f == '8' && (*(f+1) == EOS || isSpace(*(f+1)))) {
					onlydata = 9;
					break;
				} else if (*f == '7' && (*(f+1) == EOS || isSpace(*(f+1)))) {
					linkDep2Other = TRUE;
					break;
				} else {
					if (*f != EOS && !isdigit(*f)) {
						fprintf(stderr, "Illegal character \"%c\" used with %s option.\n", *f, f-2);
						fprintf(stderr, "The only +d levels allowed are 0-8.\n");
						cutt_exit(0);
					}
					onlydata = (char)(atoi(getfarg(f,f1,i))+1);
					if (onlydata < 0 || onlydata > OnlydataLimit) {
						fprintf(stderr, "The only +d levels allowed are 0-8.\n");
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
		case 't':
				if (*(f-2) == '+') {
					if ((!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:")))
						isSearchMOR = TRUE;
					else if ((!uS.mStricmp(f, "%gra") || !uS.mStricmp(f, "%gra:")))
						isSearchGRA = TRUE;
				}
				maingetflag(f-2,f1,i);
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
	if ((onlydata == 4 || onlydata == 6) && combo_string > 0) {
		fprintf(stderr,"The use of +d3 or +d5 option is illegal with +g1 or +g2 options.\n");
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
