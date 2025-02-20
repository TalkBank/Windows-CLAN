/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* ************************************************* */
/* mor.c -   morphological analyzer for CHAT files   */
/* produce %mor tier from main speaker tier		*/
/* ************************************************* */

// LOOK FOR LEX FILE: wd_dir/0add-to-mor-lex.cut
// LOOK FOR ignore folder FILE: 0nomor.cdc

/* PREPOST
 *CHI:	he is only now trying to leave .
 %xmor:	pro:sub|he cop|be&3S^aux|be&3S adv|only^qn|only adv|now
 	n:gerund|try-PRESP^part|try-PRESP inf|to^prep|to v|leave .

 # "aux|*" and any number of other ambiguity choices "^*"
 aux|*^* (adv|*^*) part|*-PRESP^* => aux|* (adv|*^*) part|*-PRESP

 # any number of any "*^*" elements in between
 aux|*^cop|* *^* part|*-PRESP^* => aux|* part|*-PRESP

 # $b next to first element or last element
 $b pro:sub|* aux|*^cop|* *^* part|*-PRESP^* => $b pro:sub|* aux|* part|*-PRESP

 # $e next to last element
 aux|*^cop|* *^* part|*-PRESP^* inf|*^* v|* $e => aux|* part|*-PRESP $- $- $e

 # "!aux|*" means match only if no choice is "aux|*"
 # this will not work for utterance above
 pro:sub|he^* cop|*^!aux|*^* => $-n|he cop|*

 # "$-str" means change the "to" element to some literal string
 pro:sub|he^* cop|*^* => $-n|he cop|*

 # "$-" means do no change that one element on replacement side at all
 aux|*^cop|* adv|*^* adv|*^* part|*-PRESP^* => aux|* $- $- part|*-PRESP

 # (adv|*^*) means any number of "adv|*^*" elements in between
 aux|*^cop|* (adv|*^*) part|*-PRESP^* => aux|* (adv|*^*) part|*-PRESP

 # skip exactly one element
 # this will not work for utterance above
 aux|*^cop|* * part|*-PRESP^* => aux|* $- part|*-PRESP

 # skip exactly two elements
 aux|*^cop|* * * part|*-PRESP^* => aux|* $- $- part|*-PRESP

 # as long as there is no "qn|*" anywhere in between
 aux|*^cop|* *^* !qn|* *^* part|*-PRESP^* => aux|* *^* part|*-PRESP

 # as long as there is no "qn|*" anywhere in between
 # this will not work for utterance above
 aux|*^cop|* *^* !adv|*^qn|* *^* part|*-PRESP^* => aux|* *^* part|*-PRESP
*/

// #define SORT_LEX_FILES // sorted lex files
#define CHAT_MODE 1
#ifndef UNX
	#include "ced.h"
#endif
#include "cu.h"
#include "check.h"
#include "morph.h"
#if defined(_MAC_CODE)
	#include <sys/stat.h>
	#include <dirent.h>
#endif

#define doDefComp FALSE /* lxs lxslxs *+* */

#if !defined(UNX)
#define _main mor_main
#define call mor_call
#define getflag mor_getflag
#define init mor_init
#define usage mor_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

struct mor_add {
//	unsigned int count;
	char *str;
	struct mor_add *nextAdd;
} ;

struct mor_X_option {
	char *line;
	struct mor_add *add;
	struct mor_X_option *left;
	struct mor_X_option *right;
} *rootXopt;

#define SFRULES struct SpecialForms
struct SpecialForms {
	char *pat;
	char *cat;
	struct SpecialForms *nextSF;
} ;

#define OUTPUTRULES struct OutputForms
struct OutputForms {
	char *cat;
	char delim;
	char isFeatureAdded;
	struct OutputForms *nextOut;
} ;

#define XWORDLIST struct XWordsList
XWORDLIST {
    char *word;
    char *plainW;
	char *fname;
	long lineno;
    XWORDLIST *matchW;
    XWORDLIST *nextW;
} ;

// Prepost beg
/*
 # test repeated (adv|very) mean match "adv|very" zero or more times
 # cop|*^aux|* (adv|very) adj|* => aux|* (adv|very) adj|*

 # I am happy that he is coming to the party .
 # test *^* mean skip zero or more words
 # cop|*^aux|* *^* n:gerund|*-PRESP^part|*-PRESP => aux|* *^* part|*-PRESP
 # cop|*^aux|* *^* !cop|*^aux|* *^* n:gerund|*-PRESP^part|*-PRESP => aux|* *^* part|*-PRESP

 # I am happy home, he is walking home .
 # pro:sub|* cop|*^aux|* !adj|* => pro:sub|* aux|*
 # cop|*^aux|* !adj|* n|*^adv|* => aux|* adv|*

 $b - begining
 $e - end
 $- - do not change word in that position
*/
#define PREPOST_DEFS
#define AMBIGS 10
struct mor_PATS {
	char *fromPats[AMBIGS];
	char *toLiteral;
	struct mor_PATS *negs;
	char isWild;
	char isNeg;
	char isMultRep;
	int  patCnt;
	int  toIndex;
	int  si, li;
	struct mor_PATS *nextPat;
} ;

struct prepost_rules {
	struct mor_PATS *from;
	struct mor_PATS *fromI;
	char        isMatch;
	struct prepost_rules *nextRule;
} ;
// Prepost end


extern char isRecursive;
extern char GExt[];
extern char isRemoveCAChar[];
extern char OverWriteFile;
extern long option_flags[];
extern struct tier *defheadtier;

extern char tComWinDataChanged;
extern char morXiExec(void);

static char *tutt;
static char mor_ftime;
char DEBUGWORD;
static char ReLoadRules;
static int  oldDEBUG;

static BOOL isConll;

static BOOL TEST;
static BOOL isPinyin;
static BOOL CHECK_LEXICON;
static BOOL isCompoundsTest;
static BOOL isAmbiguousTest;
static BOOL isLookAtLexFiles;
BOOL doCapitals;
static BOOL doLocations;
static BOOL isLookAtDep;
static BOOL isRunPrepost;
static BOOL isRunPost;
static BOOL isRunPostmortem;
static BOOL isRunMegrasp;
static long totalWords;
static long missingWords;


extern FEAT_PTR *feat_codes;
extern STRING **match_stack;
extern SEG_REC *word_segments;
extern LEX_PTR lex_root;
extern LEXF_PTR lex_frow;
extern ARULE_FILE_PTR ar_files_list;
extern CRULE_PTR crule_list;
extern RULEPACK_PTR startrules_list;
extern RULEPACK_PTR endrules_list;
extern DRULE_PTR drule_list;
extern int crules_linenum;
extern int drules_linenum;
extern TRIE_PTR trie_root;
extern long trie_node_ctr;
extern long letter_ctr;
extern long trie_entry_ctr;
extern FEAT_PTR features;
extern int feat_node_ctr;
extern int val_node_ctr;
extern VAR_REC *var_tab;
extern RESULT_REC *result_list;
extern int result_index;
extern long rt_entry_ctr;
extern long mem_ctr;


static SFRULES *sf_rules, *ex_rules;
static OUTPUTRULES *out_rules;
static struct prepost_rules *prepostRules;
// static int num_prev, num_cur, num_dis;

FNType debugName[FNSize];

static char w_temp[MAXLINE];
char w_temp_flag[MAX_WORD];

static FNType mor_arules_fname[FNSize];
static FNType mor_crules_fname[FNSize];
static FNType drules_fname[FNSize];
static FNType mor_output_fname[FNSize];
static FNType mor_sfrules_fname[FNSize];
static FNType mor_exrules_fname[FNSize];
static FNType prepost_fname[FNSize];
static FNType Xoption_fname[FNSize];

static char *e;
static char *w;
static char *c;

STRING estr[2];
static STRING eparse[2];
static STRING etrans[2];
static STRING ecomp[2];

static int lines_read;

static XWORDLIST *rootXTest;

static FEATTYPE empty_cat[1];
static FEATTYPE tmp_feats_comp[32];
static FEATTYPE err_f_comp[8];

char isMORXiMode = FALSE;
char morTestCom[UTTLINELEN+2];	/* used to be: templine4 */

FILE *crules = NULL, *drules = NULL, *other_rules = NULL, *prepostFP = NULL;

static void ShowHelp(int i) {
	/* 24-5-99
	 if (i == 1) {
		 printf("\ncommands:\n");
		 printf("\tword - analyze this word\n");
		 printf("\t:q  quit- exit program\n");
		 printf("\t:dN set debug level to N\n");
		 printf("\t\t0 - turn off debugging.\n");
	 }
	 printf("\t\t1 - display application of all the rules.\n");
	 printf("\t\t2 - display application of a rules.\n");
	 printf("\t\t3 - display application of c rules.\n");
	 printf("\t\t4 - display application of d rules.\n");
	 printf("\t\t5 - display memory usage.\n");
	 printf("\t\t6 - display d rules.\n");
	 printf("\t\t7 - display c rules.\n");
	 printf("\t\t8 - display a rules.\n");
	 if (i == 1) {
		 printf("\t:c  print out current set of c-rules\n");
		 printf("\t:f  store debugging info in file: debug.cdc\n");
		 printf("\t:l  re-load rules and lexicon files\n");
		 printf("\t:h  help - print this message\n");
	 }
	 */
	if (i == 1) {
		printf("\ncommands:\n");
		printf("\tword - analyze this word\n");
		printf("\t:q  quit- exit program\n");
		printf("\t:c  print out current set of c-rules\n");
		printf("\t:d  display application of a rules.\n");
		printf("\t:l  re-load rules and lexicon files\n");
		printf("\t:h  help - print this message\n");
	}
}

static OUTPUTRULES *AddToOutputRules(OUTPUTRULES *root, char *st, char isFeatureAdded) {
	char delim;
	OUTPUTRULES *t;

	if (st[0] == '-') {
		delim = '-';
		st++;
	} else if (st[0] == '&') {
		delim = '&';
		st++;
	} else {
		delim = '&';
	}
	uS.remblanks(st);
	if (root == NULL) {
		t = NEW(OUTPUTRULES);
		if (t == NULL)
			out_of_mem();
		root = t;
	} else {
		for (t=root; t->nextOut != NULL; t=t->nextOut) {
			if (!strcmp(t->cat, st))
				return(root);
		}
		t->nextOut = NEW(OUTPUTRULES);
		t = t->nextOut;
		if (t == NULL)
			out_of_mem();
	}
	t->nextOut = NULL;
	t->delim = delim;
	if ((t->cat=(char *)malloc(strlen(st)+1)) == NULL)
		out_of_mem();
	strcpy(t->cat, st);
	t->isFeatureAdded = isFeatureAdded;
	return(root);
}

static SFRULES *AddToSFRules(SFRULES *root, char *st, int type) {
	SFRULES *t;

	uS.remblanks(st);
	if (root == NULL || type == 0) {
		t = NEW(SFRULES);
		if (t == NULL) out_of_mem();
		t->nextSF = root;
		t->cat = NULL;
		if ((t->pat=(char *)malloc(strlen(st)+1)) == NULL)  out_of_mem();
		strcpy(t->pat, st);
		root = t;
	} else {
		if ((root->cat=(char *)malloc(strlen(st)+1)) == NULL)  out_of_mem();
		strcpy(root->cat, st);
	}
	return(root);
}

static char OpenAr(FNType *mFileName, int *isOpenOne, int isReadARcut) {
	int  index, t, len;
	FNType tFName[FNSize];
	FILE *arules;

	index = 1;
	t = strlen(mFileName);
	if (!isReadARcut) {
		addFilename2Path(mFileName, mor_arules_fname);
		if ((arules=fopen(mFileName, "r"))) {
			fprintf(stderr,"\rUsing a-rules: %s.\n", mFileName);
			if (read_arules(arules, mFileName) == FAIL) {
				fclose(arules);
				return(FAIL);
			}
			fclose(arules);
			*isOpenOne = TRUE;
		}
		mFileName[t] = EOS;
	}
	while ((index=Get_File(tFName, index)) != 0) {
		if (tFName[0] != '.' && uS.mStricmp(tFName,mor_arules_fname)) {
			len = strlen(tFName) - 4;
			if (len >= 0 && uS.FNTypeicmp(tFName+len, ".cut", 0) == 0) {
				addFilename2Path(mFileName, tFName);
				if ((arules=fopen(mFileName, "r"))) {
					fprintf(stderr,"\rUsing a-rules: %s.\n", mFileName);
					if (read_arules(arules, mFileName) == FAIL) {
						fclose(arules);
						return(FAIL);
					}
					fclose(arules);
					*isOpenOne = TRUE;
				}
				mFileName[t] = EOS;
			}
		}
	}
	return(SUCCEED);
}

static char FindAndOpenAr(void) {
	int isOpenOne = FALSE, isReadARcut = FALSE;
	FNType arst[5], mFileName[FNSize];
	FILE *arules;

	if ((arules=OpenMorLib(mor_arules_fname, "r", FALSE, FALSE, mFileName)) != NULL) {
		fprintf(stderr, "\rUsing a-rules: %s.\n", mFileName);
		isOpenOne = TRUE;
		if (read_arules(arules, mFileName) == FAIL) {
			fclose(arules);
			SetNewVol(wd_dir);
			return(FAIL);
		}
		fclose(arules);
		isReadARcut = TRUE;
	}
	uS.str2FNType(arst, 0L, "ar");
	uS.str2FNType(arst, strlen(arst), PATHDELIMSTR);
	strcpy(mFileName,mor_lib_dir);
	addFilename2Path(mFileName, arst);
	if (!SetNewVol(mFileName)) {
		if (OpenAr(mFileName, &isOpenOne, isReadARcut) == FAIL) {
			SetNewVol(wd_dir);
			return(FAIL);
		}
	}
	if (markPostARules()  == FAIL) {
		SetNewVol(wd_dir);
		return(FAIL);
	}
	SetNewVol(wd_dir);
	if (!isOpenOne) {
		fprintf(stderr,"Can't open either \"%s\" file or any file in folder \"%s\".\n",mor_arules_fname,mFileName);
#ifdef UNX
		fprintf(stderr,"Please use -l option to specify lexicon libary path.\n");
#else
		fprintf(stderr,"Please make sure that the \"mor lib\" in the Commands window is set to the folder in which the ar.cut file is located.\n");
#endif
		return(FAIL);
	} else
		return(SUCCEED);
}

static XWORDLIST *addToAmbiguous(XWORDLIST *root, char *e) {
	int res;
	char *s1;
	XWORDLIST *t, *tt, *tw;

	if ((tw=NEW(XWORDLIST)) == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}
	tw->word = (char *)malloc((unsigned)(strlen(e)+1));
	if (tw->word == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}
	strcpy(tw->word, e);
	tw->plainW = NULL;
	tw->fname = NULL;
	tw->lineno = 0L;
	tw->matchW = NULL;
	tw->nextW = NULL;
	for (s1=tw->word; !isSpace(*s1) && *s1 != EOS; s1++) ;
	if (*s1 == EOS)
		*(s1+1) = EOS;
	else
		*s1 = EOS;
	if (root == NULL) {
		root = tw;
	} else if ((res=strcmp(tw->word,root->word)) <= 0) {
		if (res == 0) {
			tw->matchW = root->matchW;
			root->matchW = tw;
		} else {
			tw->nextW = root;
			root = tw;
		}
	} else {
		t = root;
		tt = root->nextW;
		while (tt != NULL) {
			if ((res=strcmp(tw->word,tt->word)) <= 0)
				break;
			t = tt;
			tt = tt->nextW;
		}
		if (tt == NULL) {
			t->nextW = tw;
			tw->nextW = NULL;
		} else {
			if (res == 0) {
				tw->matchW = tt->matchW;
				tt->matchW = tw;
			} else {
				tw->nextW = tt;
				t->nextW = tw;
			}
		}
	}
	return(root);
}

static char isAddMatchW(XWORDLIST *t, XWORDLIST *tw) {
	int lt, ltw;
	XWORDLIST *tt;

	if (tw->plainW != NULL) {
		if (strcmp(t->word, tw->word) == 0) {
			lt = strlen(t->word);
			ltw = strlen(tw->word);
			if (t->word[lt+1] == EOS || tw->word[ltw+1] == EOS || strcmp(t->word+lt+1, tw->word+ltw+1))
				return(FALSE);
		}
		for (tt=t->matchW; tt != NULL; tt=tt->matchW) {
			if (strcmp(tt->word, tw->word) == 0) {
				lt = strlen(tt->word);
				ltw = strlen(tw->word);
				if (tt->word[lt+1] == EOS || tw->word[ltw+1] == EOS || strcmp(tt->word+lt+1, tw->word+ltw+1))
					return(FALSE);
			}
		}
	}
	tw->matchW = t->matchW;
	t->matchW = tw;
	return(TRUE);
}

static XWORDLIST *addToCompounds(XWORDLIST *root, char *e, char *fname, long ln) {
	int i;
	char *s1, *s2, isUsed;
	XWORDLIST *t, *tw;

	if ((tw=NEW(XWORDLIST)) == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}
	tw->word = (char *)malloc((unsigned)(strlen(e)+1));
	if (tw->word == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}
	strcpy(tw->word, e);
	tw->fname = (char *)malloc((unsigned)(strlen(fname)+1));
	if (tw->fname == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}
	strcpy(tw->fname, fname);
	tw->plainW = NULL;
	tw->lineno = ln;
	tw->matchW = NULL;
	tw->nextW = NULL;
	for (s1=tw->word; !isSpace(*s1) && *s1 != EOS; s1++) ;
	if (*s1 == EOS)
		*(s1+1) = EOS;
	else
		*s1 = EOS;
	if (tw->word[0] == '+' && strchr(tw->word+1, '+') == NULL) {
	} else if (strchr(tw->word, '+') != NULL) {
		tw->plainW = (char *)malloc((unsigned)(strlen(tw->word)+1));
		if (tw->plainW == NULL) {
			fprintf(stderr,"No more space left in core.\n");
			cutt_exit(1);
		}
		strcpy(tw->plainW, tw->word);
		i = 0;
		while (tw->plainW[i] != EOS) {
			if (tw->plainW[i] == '+')
				strcpy(tw->plainW+i, tw->plainW+i+1);
			else
				i++;
		}
	}
	if (root == NULL) {
		root = tw;
	} else {
		if (tw->plainW != NULL)
			s1 = tw->plainW;
		else
			s1 = tw->word;
		for (t=root; t->nextW != NULL; t=t->nextW) {
			if (t->plainW != NULL)
				s2 = t->plainW;
			else
				s2 = t->word;
			if (strcmp(s1, s2) == 0) {
				isUsed = FALSE;
				if (t->matchW != NULL || t->plainW != NULL || tw->plainW != NULL) {
					if (isAddMatchW(t, tw))
						isUsed = TRUE;
				}
				if (!isUsed) {
					if (tw->word)
						free(tw->word);
					if (tw->plainW)
						free(tw->plainW);
					if (tw->fname)
						free(tw->fname);
					free(tw);
				}
				return(root);
			}
		}
		if (strcmp(s1, t->word) == 0) {
			isUsed = FALSE;
			if (t->matchW != NULL || t->plainW != NULL || tw->plainW != NULL) {
				if (isAddMatchW(t, tw))
					isUsed = TRUE;
			}
			if (!isUsed) {
				if (tw->word)
					free(tw->word);
				if (tw->plainW)
					free(tw->plainW);
				if (tw->fname)
					free(tw->fname);
				free(tw);
			}
			return(root);
		}
		t->nextW = tw;
	}
	return(root);
}

static void changeForm(char *e) {
	char *e_ptr, cat[MAXCAT], stem[MAX_WORD];
	long k;

	e_ptr = get_lex_word(e,templineC4);
	e_ptr = skip_blanks(e_ptr);
	if (*e_ptr == EOS)
		return;
	e_ptr = get_cat(e_ptr,cat);
	if (e_ptr == NULL)
		return;
	e_ptr = skip_blanks(e_ptr);
	if (*e_ptr == EOS)
		return;
	if (*e_ptr != '"') {
		fprintf(stderr,"\rERROR: Can't find third column, may be wrong \" used:\n\t%s\n", e);
		return;
	}
	k = 0L;
	e_ptr++;
	if (*e_ptr != '"') {
		if (*e_ptr == ':')
			e_ptr++;
		do {
			stem[k++] = *e_ptr++;
		} while ((*e_ptr != '"' || *(e_ptr-1) == '\\') && *e_ptr != EOS) ;
		if (stem[k-1] == ':')
			k--;
	}
	stem[k] = EOS;
	if (*e_ptr != EOS)
		e_ptr++;
	if (strlen(stem) == 0) {
		fprintf(stderr,"\rERROR: Third column is missing the data:\n\t%s\n", e);
		return;
	}
	strcpy(templineC4, stem);
	strcat(templineC4, "\t");
	strcat(templineC4, cat);
	if (*e_ptr != '\t')
		strcat(templineC4, "\t");
	strcat(templineC4, e_ptr);
	strcpy(e, templineC4);
}

/* find files recursively or otherwise */
#ifdef SORT_LEX_FILES // sorted lex files BEG
struct lex_FileList {
	FNType fname[FNSize];
	struct lex_FileList *next_file;
} ;

static struct lex_FileList *free_FileList(struct lex_FileList *p) {
	struct lex_FileList *t;

	while (p != NULL) {
		t = p;
		p = p->next_file;
		free(t);
	}
	return(NULL);
}

static struct lex_FileList *cutt_addToFileList(struct lex_FileList *root, FNType *fname) {
	struct lex_FileList *tF, *t, *tt;

	tF = NEW(struct lex_FileList);
	if (tF == NULL) {
#ifndef UNX
		if (MEMPROT)
			free(MEMPROT);
#endif
		fprintf(stderr, "\nOut of memory\n");
		return(NULL);
	}
	if (root == NULL) {
		root = tF;
		tF->next_file = NULL;
	} else if (strcmp(root->fname, fname) > 0) {
		tF->next_file = root;
		root = tF;
	} else {
		t = root;
		tt = root->next_file;
		while (tt != NULL) {
			if (strcmp(tt->fname, fname) > 0)
				break;
			t = tt;
			tt = tt->next_file;
		}
		if (tt == NULL) {
			t->next_file = tF;
			tF->next_file = NULL;
		} else {
			tF->next_file = tt;
			t->next_file = tF;
		}
	}
	strcpy(tF->fname, fname);
	return(root);
}
#endif // SORT_LEX_FILES sorted lex files END

// LOOK FOR LEX FILE: wd_dir/0add-to-mor-lex.cut
static void OpenLex(FNType *mFileName, int *isOpenOne) {
	int  i, index, t, len;
	char isCompFound, isWord, isTrans;
	long cnt, tcnt, ln;
	FNType tFName[FNSize];
	FILE *lex;

	cnt = 0L;
	tcnt = 0L;
	index = 1;
	t = strlen(mFileName);

#ifdef SORT_LEX_FILES // sorted lex files BEG
	struct lex_FileList *tf, *root_file;

	root_file = NULL;
	while ((index=Get_File(tFName, index)) != 0) {
		len = strlen(tFName) - 4;
		if (tFName[0] != '.' && len >= 0 && uS.FNTypeicmp(tFName+len, ".cut", 0) == 0) {
			root_file = cutt_addToFileList(root_file, tFName);
			if (root_file == NULL) {
				root_file = free_FileList(root_file);
				return;
			}
		}
	}
	for (tf=root_file; tf != NULL; tf=tf->next_file) {
		addFilename2Path(mFileName, tf->fname);
		if ((lex=fopen(mFileName, "r"))) {
			fprintf(stderr,"\rUsing lexicon: %s.\n", mFileName);
			ln = 0L;
			while (get_line(e,MAXENTRY,lex,&lines_read) == 0) {
				ln += lines_read;
				if (strlen(e) > 0) {

// 2019-07-30 change dash to 0x2013
					isWord = TRUE;
					isTrans = FALSE;
					for (i=0; e[i] != EOS; i++) {
						if (e[i] == ' ') {
							isWord = FALSE;
						} else if (e[i] == '=') {
							if (isTrans)
								isTrans = FALSE;
							else if (i > 0 && isSpace(e[i-1]))
								isTrans = TRUE;
						} else if (e[i] == '-' && (isWord || isTrans)) {
//fprintf(stderr, "%s\n", e);
							uS.shiftright(e+i, 2);
							e[i++] = 0xE2;
							e[i++] = 0x80;
							e[i++] = 0x93;
						}
					}

					if (isCompoundsTest) {
						if (cnt >= tcnt) {
							tcnt = cnt + 200;
							fprintf(stderr,"\r%ld ",cnt);
						}
						cnt++;
						rootXTest = addToCompounds(rootXTest, e, tFName, ln);
					} else if (isAmbiguousTest) {
						if (cnt >= tcnt) {
							tcnt = cnt + 200;
							fprintf(stderr,"\r%ld ",cnt);
						}
						cnt++;
						rootXTest = addToAmbiguous(rootXTest, e);
					} else {
						if (isPinyin)
							changeForm(e);
						isCompFound = FALSE;
						for (i=0; isSpace(e[i]); i++) ;
						for (; !isSpace(e[i]) && e[i] != EOS; i++) {
							if (e[i] == '+') {
								isCompFound = TRUE;
								break;
							}
						}
						if (isCompFound)
							process_lex_entry(e, TRUE, mFileName, ln);
						process_lex_entry(e, FALSE, mFileName, ln);
					}
				}
			}
			fclose(lex);
			lex = NULL;
			*isOpenOne = TRUE;
		}
		mFileName[t] = EOS;
	}
	root_file = free_FileList(root_file);
#else // SORT_LEX_FILES  unsorted lex files BEG
	while ((index=Get_File(tFName, index)) != 0) {
		len = strlen(tFName) - 4;
		if (tFName[0] != '.' && len >= 0 && uS.FNTypeicmp(tFName+len, ".cut", 0) == 0) {
			addFilename2Path(mFileName, tFName);
			if ((lex=fopen(mFileName, "r"))) {
				fprintf(stderr,"\rUsing lexicon: %s.\n", mFileName);
				ln = 0L;
				while (get_line(e,MAXENTRY,lex,&lines_read) == 0) {
					ln += lines_read;
					if (strlen(e) > 0) {

// 2019-07-30 change dash to 0x2013
						isWord = TRUE;
						isTrans = FALSE;
						for (i=0; e[i] != EOS; i++) {
							if (e[i] == ' ') {
								isWord = FALSE;
							} else if (e[i] == '=') {
								if (isTrans)
									isTrans = FALSE;
								else if (i > 0 && isSpace(e[i-1]))
									isTrans = TRUE;
							} else if (e[i] == '-' && (isWord || isTrans)) {
//fprintf(stderr, "%s\n", e);
								uS.shiftright(e+i, 2);
								e[i++] = 0xE2;
								e[i++] = 0x80;
								e[i++] = 0x93;
							}
						}

						if (isCompoundsTest) {
							if (cnt >= tcnt) {
								tcnt = cnt + 200;
								fprintf(stderr,"\r%ld ",cnt);
							}
							cnt++;
							rootXTest = addToCompounds(rootXTest, e, tFName, ln);
						} else if (isAmbiguousTest) {
							if (cnt >= tcnt) {
								tcnt = cnt + 200;
								fprintf(stderr,"\r%ld ",cnt);
							}
							cnt++;
							rootXTest = addToAmbiguous(rootXTest, e);
						} else {
							if (isPinyin)
								changeForm(e);
							isCompFound = FALSE;
							for (i=0; isSpace(e[i]); i++) ;
							for (; !isSpace(e[i]) && e[i] != EOS; i++) {
								if (e[i] == '+') {
									isCompFound = TRUE;
									break;
								}
							}
							if (isCompFound)
								process_lex_entry(e, TRUE, mFileName, ln);
							process_lex_entry(e, FALSE, mFileName, ln);
						}
					}
				}
				fclose(lex);
				lex = NULL;
				*isOpenOne = TRUE;
			}
			mFileName[t] = EOS;
		}
	}
#endif // !SORT_LEX_FILES END

	strcpy(mFileName, wd_dir);
	addFilename2Path(mFileName, "0add-to-mor-lex.cut");
	if ((lex=fopen(mFileName, "r"))) {
		fprintf(stderr,"\rUsing lexicon: %s.\n", mFileName);
		ln = 0L;
		while (get_line(e,MAXENTRY,lex,&lines_read) == 0) {
			ln += lines_read;
			if (strlen(e) > 0) {

// 2019-07-30 change dash to 0x2013
				isWord = TRUE;
				isTrans = FALSE;
				for (i=0; e[i] != EOS; i++) {
					if (e[i] == ' ') {
						isWord = FALSE;
					} else if (e[i] == '=') {
						if (isTrans)
							isTrans = FALSE;
						else if (i > 0 && isSpace(e[i-1]))
							isTrans = TRUE;
					} else if (e[i] == '-' && (isWord || isTrans)) {
//fprintf(stderr, "%s\n", e);
						uS.shiftright(e+i, 2);
						e[i++] = 0xE2;
						e[i++] = 0x80;
						e[i++] = 0x93;
					}
				}

				if (isPinyin)
					changeForm(e);
				isCompFound = FALSE;
				for (i=0; isSpace(e[i]); i++) ;
				for (; !isSpace(e[i]) && e[i] != EOS; i++) {
					if (e[i] == '+') {
						isCompFound = TRUE;
						break;
					}
				}
				if (isCompFound)
					process_lex_entry(e, TRUE, mFileName, ln);
				process_lex_entry(e, FALSE, mFileName, ln);
			}
		}
		fclose(lex);
		lex = NULL;
	}
}

// LOOK FOR LEX FILE: wd_dir/0add-to-mor-lex.cut
static char FindAndOpenLex(void) {
	int  isOpenOne = FALSE;
	FNType lexst[5], mFileName[FNSize];

	uS.str2FNType(lexst, 0L, "lex");
	uS.str2FNType(lexst, strlen(lexst), PATHDELIMSTR);
	if (!isOpenOne) {
		strcpy(mFileName,mor_lib_dir);
		addFilename2Path(mFileName, lexst);
		if (!SetNewVol(mFileName)) {
			OpenLex(mFileName, &isOpenOne);
		}
	}
	SetNewVol(wd_dir);
	if (!isOpenOne) {
		return(FAIL);
	} else
		return(SUCCEED);
}

#define PrepostPARSE
/*
#*************************************
# Instructions:
# '~'	in the rule means clitic in the utterance is not treated as space, otherwise it is
# !	used before element refers to the whole element
# ,!	used within element after "," refers only to that choice
# $-	means copy whatever search pattern matched in full and unchanged to the output
# (...)	means match choices within zero or any number of times; the "to" part must be ()
# [...]	means match choices within zero or only one time; the "to" part must be []
# pro:*|*,n|*,!pro:dem|*	means match "pro:*|" or "n|" once, but fail at "pro:dem|";
# 				the "to" part must be $-
# !part|*P*P	means match only if part|*P*P not found; the "to" part must be !-
# \	means the rule continues on the next line
#*************************************
*/
// Prepost beg
//   parse rules
static struct mor_PATS *mor_makeFromPats(char *st) {
	char *s, *e, isWild, isRootNeg;
	int  lastc;
	long i, patsNum;
	struct mor_PATS *root, *rootNeg, *p = NULL;

	i = 0L;
	root = NULL;
	rootNeg = NULL;
	isWild = FALSE;
	while (1) {
		if (isSpace(*st) || *st == EOS) {
			templineC1[i] = EOS;
			if (!nomap)
				uS.lowercasestr(templineC1, &dFnt, MBF);
			if (i > 0) {
				if (strcmp(templineC1, "*^*") == 0) {
					isWild = TRUE;
				} else {
					if (isWild && templineC1[0] == '!') {
						if (rootNeg == NULL) {
							rootNeg = NEW(struct mor_PATS);
							p = rootNeg;
						} else if (p != NULL) {
							for (p=rootNeg; p->nextPat != NULL; p=p->nextPat) {
							}
							p->nextPat = NEW(struct mor_PATS);
							p = p->nextPat;
						}
						isRootNeg = TRUE;
					} else {
						if (root == NULL) {
							root = NEW(struct mor_PATS);
							p = root;
						} else if (p != NULL) {
							for (p=root; p->nextPat != NULL; p=p->nextPat) {
							}
							p->nextPat = NEW(struct mor_PATS);
							p = p->nextPat;
						}
						isRootNeg = FALSE;
					}
					if (p == NULL) {
						fprintf(stderr,"No more space left in core.\n");
						cutt_exit(1);
					}
					for (patsNum=0; patsNum < AMBIGS; patsNum++)
						p->fromPats[patsNum] = NULL;
					p->toLiteral = NULL;
					if (isRootNeg) {
						p->negs = NULL;
					} else {
						p->negs = rootNeg;
						rootNeg = NULL;
					}
					if (p->negs != NULL) {
						p->isWild = TRUE;
					} else {
						p->isWild = isWild;
					}
					p->isNeg = FALSE;
					p->isMultRep = FALSE;
					p->patCnt = 0;
					p->toIndex = 0;
					p->nextPat = NULL;
					lastc = strlen(templineC1) - 1;
					if (templineC1[0] == '(' && templineC1[lastc] == ')') {
						p->isMultRep = TRUE;
						templineC1[lastc] = EOS;
						strcpy(templineC1, templineC1+1);
					} else if (templineC1[0] == '!') {
						p->isNeg = TRUE;
						strcpy(templineC1, templineC1+1);
					}
					s = templineC1;
					patsNum = 0;
					p->patCnt = 0;
					do {
						e = strchr(s, '^');
						if (e != NULL)
							*e = EOS;
						strcpy(spareTier3, s);
						if (spareTier3[0] != EOS) {
							if (patsNum >= AMBIGS) {
								fprintf(stderr,"Internal error. patsNum limit exceeded.\n");
								cutt_exit(1);
							}
							p->fromPats[patsNum] = (char *)malloc(strlen(spareTier3)+1);
							if (p->fromPats[patsNum] == NULL) {
								fprintf(stderr,"No more space left in core.\n");
								cutt_exit(1);
							}
							strcpy(p->fromPats[patsNum], spareTier3);
							if (strcmp(p->fromPats[patsNum], "*") != 0 && p->fromPats[patsNum][0] != '!')
								p->patCnt++;
							patsNum++;
						}
						if (e != NULL) {
							*e = '^';
							s = e + 1;
						}
					} while (e != NULL) ;
					isWild = FALSE;
				}
			}
			i = 0L;
		} else
			templineC1[i++] = *st;
		if (*st == EOS)
			break;
		st++;
	}
	return(root);
}

static int findFromPatIndex(struct mor_PATS *from, char *to, char *fname, long ln) {
	int patsNum;
	for (patsNum=0; patsNum < AMBIGS && from->fromPats[patsNum] != NULL; patsNum++) {
		if (uS.mStricmp(from->fromPats[patsNum], to) == 0)
			return(patsNum);
	}
	fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
	fprintf(stderr,"\"to\" string does not match any \"from\" choices.\n");
	CleanUpAll(TRUE);
	cutt_exit(0);
	return(-1);
}

static void mor_addTos(struct mor_PATS *from, char *st, char *fname, long ln) {
	char isSkip;
	int  lastc;
	long i;

	i = 0L;
	while (1) {
		if (isSpace(*st) || *st == EOS) {
			templineC1[i] = EOS;
//			if (!nomap)
//				uS.lowercasestr(templineC1, &dFnt, MBF);
			if (i > 0) {
				isSkip = FALSE;
				lastc = strlen(templineC1) - 1;
				if (templineC1[0] == '(' && templineC1[lastc] == ')') {
					isSkip = TRUE;
				} else if (templineC1[0] == '!') {
					isSkip = TRUE;
				} else if (strcmp(templineC1, "*^*") == 0) {
					isSkip = TRUE;
				}
				if (!isSkip) {
					while (from->isMultRep || from->isNeg) {
						if (from->isNeg) {
							from->toIndex = -1;
						}
						from = from->nextPat;
						if (from == NULL)
							break;
					}
					if (from == NULL) {
						fprintf(stderr,"\n**** Unknown error in the prepost rules file\n");
						CleanUpAll(TRUE);
						cutt_exit(0);
					}
					if (templineC1[0] == '$' && templineC1[1] == '-') {
						from->toIndex = -1;
						if (templineC1[2] != EOS) {
							from->toLiteral = (char *)malloc(strlen(templineC1+2)+1);
							if (from->toLiteral == NULL) {
								fprintf(stderr,"No more space left in core.\n");
								cutt_exit(1);
							}
							strcpy(from->toLiteral, templineC1+2);
						}
					} else {
						from->toIndex = findFromPatIndex(from, templineC1, fname, ln);
					}
					from = from->nextPat;
				}
			}
			i = 0L;
		} else
			templineC1[i++] = *st;
		if (*st == EOS)
			break;
		st++;
	}
}

static void make_new_prepost_rule(char *fname, long ln, char *rule) {
	short offset;
	long secondPart;
	struct prepost_rules *t;

	if (prepostRules == NULL) {
		prepostRules = NEW(struct prepost_rules);
		t = prepostRules;
	} else {
		t = prepostRules;
		while (t->nextRule != NULL) t = t->nextRule;
		t->nextRule = NEW(struct prepost_rules);
		t = t->nextRule;
	}
	if (t == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}
	t->from = NULL;
	t->fromI = NULL;
	t->isMatch = FALSE;
	t->nextRule = NULL;

	offset = 0;
	for (secondPart=0L; rule[secondPart] != EOS; secondPart++) {
		if (rule[secondPart] == '=' && rule[secondPart+1] == '>') {
			offset = 2;
			break;
		} else if (rule[secondPart] == '=' && rule[secondPart+1] == '=' && rule[secondPart+2] == '>') {
			offset = 3;
			break;
		} else if (rule[secondPart] == '-' && rule[secondPart+1] == '>') {
			offset = 2;
			break;
		} else if (rule[secondPart] == '-' && rule[secondPart+1] == '-' && rule[secondPart+2] == '>') {
			offset = 3;
			break;
		}
	}
	if (rule[secondPart] == EOS) {
		CleanUpAll(TRUE);
		fprintf(stderr,"\n*** File \"%s\": line %ld.\n", fname, ln);
		fprintf(stderr,"Error: missing 'to' part in rule:\n\t%s\n", rule);
		cutt_exit(0);
	}
	rule[secondPart] = EOS;
	t->from = mor_makeFromPats(rule);
	t->fromI = t->from;
	t->isMatch = FALSE;
	mor_addTos(t->from, rule+secondPart+offset, fname, ln);
}

static void readPrepost(void) {
	long i, ln;

	fprintf(stderr,"Using PrePost rules file: %s\n", prepost_fname);
	ln = 0L;
	while (fgets_cr(templineC, UTTLINELEN, prepostFP)) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
			continue;
		ln++;
		for (i=0L; isSpace(templineC[i]); i++) ;
		if (i > 0L)
			strcpy(templineC, templineC+i);
		uS.remblanks(templineC);
		if (templineC[0] == EOS || templineC[0] == '%' || templineC[0] == '#')
			continue;
		make_new_prepost_rule(prepost_fname, ln, templineC);
	}
	fclose(prepostFP);
	prepostFP = NULL;
}

#define PrepostMATCH

// Prepost match rules
static void addRightStr(struct mor_PATS *from, char *line, char *out) {
	char c;
	char *s, *e, *t;

	c = line[from->li];
	line[from->li] = EOS;

	for (s=line+from->si; isSpace(*s); s++) ;
	if (from->toIndex == -1) {
		if (from->toLiteral == NULL)
			strcat(out, s);
		else
			strcat(out, from->toLiteral);
// 2019-03-26
	} else if (from->isNeg) {
		strcat(out, s);
//
	} else {
		do {
			e = strchr(s, '^');
			if (e != NULL)
				*e = EOS;
			if (s[0] != EOS) {
				if (from->fromPats[from->toIndex] != NULL) {
					t = strchr(s, '#');
					if (t != NULL)
						t++;
					else
						t = s;
					if (uS.patmat(t, from->fromPats[from->toIndex])) {
						strcat(out, s);
						if (e != NULL) {
							*e = '^';
						}
						line[from->li] = c;
						return;
					}
				}
			}
			if (e != NULL) {
				*e = '^';
				s = e + 1;
			}
		} while (e != NULL) ;
	}

	line[from->li] = c;
}

static int mor_replaceItems(struct prepost_rules *t, char *line, int i) {
	int j, lI, next;
	char frSPCFound;
	struct mor_PATS *fr;

	i = 0;
	j = 0;
	lI = 0;
	for (fr=t->from; fr != NULL; fr=fr->nextPat) {
		if (fr->isMultRep) {
		} else {
			for (i=lI; i < fr->si; i++) {
				templineC1[j] = line[i];
				j++;
			}
			frSPCFound = !strcmp(fr->fromPats[0], "$b") || !strcmp(fr->fromPats[0], "$e");
			if (!frSPCFound) {
				templineC1[j] = EOS;
				addRightStr(fr, line, templineC1);
				j = strlen(templineC1);
			}
			lI = fr->li;
		}
	}
	next = j;
	for (i=lI; line[i] != EOS; i++) {
		templineC1[j] = line[i];
		j++;
	}
	templineC1[j] = EOS;
	strcpy(line, templineC1);
	return(next);
}

static char mor_isEndOfTier(char *line, int i) {
	if (line[i] == EOS || uS.IsUtteranceDel(line, i))
		return(TRUE);
	else if (line[i] == '+') {
		for (i++; !uS.isskip(line,i,&dFnt,MBF) && line[i] != EOS; i++) {
			if (uS.IsUtteranceDel(line, i))
				return(TRUE);
		}
	}
	return(FALSE);
}

static char isAllAmbgMatch(char *w, mor_PATS *from) {
	int patsNum, wCnt, matchCnt;
	char *s, *e, *t, *comma, isFound, isPatMatched[AMBIGS];

	for (patsNum=0; patsNum < AMBIGS; patsNum++) {
		isPatMatched[patsNum] = FALSE;
	}
	s = w;
	wCnt = 0;
	matchCnt = 0;
	patsNum = 0;
	do {
		e = strchr(s, '^');
		if (e != NULL)
			*e = EOS;
		if (s[0] != EOS) {
			t = strchr(s, '#');
			if (t != NULL)
				s = t + 1;
			wCnt++;
			for (patsNum=0; patsNum < AMBIGS && from->fromPats[patsNum] != NULL; patsNum++) {
				if ((comma=strchr(from->fromPats[patsNum], ',')) != NULL) {
					t = from->fromPats[patsNum];
					isFound = FALSE;
					do {
						if (comma != NULL)
							*comma = EOS;
						if (uS.patmat(s, t)) {
							if (strcmp(t, "*") == 0) {
								wCnt--;
							} else if (!isPatMatched[patsNum]) {
								matchCnt++;
								isPatMatched[patsNum] = TRUE;
								isFound = TRUE;
							}
						} else if (*t == '!') {
							if (uS.patmat(s, t+1)) {
								if (e != NULL) {
									*e = '^';
									s = e + 1;
								}
								return(FALSE);
							}
						}
						if (comma != NULL) {
							*comma = ',';
							if (isFound)
								t = NULL;
							else {
								t = comma + 1;
								comma = strchr(t, ',');
							}
						} else
							t = NULL;
					} while (t != NULL) ;
					if (isFound)
						break;
				} else {
					if (uS.patmat(s, from->fromPats[patsNum])) {
						if (strcmp(from->fromPats[patsNum], "*") == 0) {
							wCnt--;
						} else if (!isPatMatched[patsNum]) {
							matchCnt++;
							isPatMatched[patsNum] = TRUE;
						}
						break;
					} else if (from->fromPats[patsNum][0] == '!') {
						if (uS.patmat(s, from->fromPats[patsNum]+1)) {
							if (e != NULL) {
								*e = '^';
								s = e + 1;
							}
							return(FALSE);
						}
					}
				}
			}
		}
		if (e != NULL) {
			*e = '^';
			s = e + 1;
		}
	} while (e != NULL) ;

	if (matchCnt == wCnt && wCnt == from->patCnt)
		return(TRUE);
	return(FALSE);
}

static void mor_MatchedAndReplaced(char *out) {
	char isBeg;
	char w[BUFSIZ];
	int  i, sI, lI;
	struct prepost_rules *t;

	for (t=prepostRules; t != NULL; t=t->nextRule) {
		if (t->fromI == NULL)
			continue;
		t->fromI = t->from;
		t->isMatch = FALSE;
		isBeg = TRUE;
		i = 0;
		while (out[i] != EOS) {
			for (; isSpace(out[i]) || out[i] == '\n'; i++) ;
			sI = i;
			i = getword("%mor", out, w, NULL, i);
			if (uS.isskip(out,i-1,&dFnt,MBF) && !uS.isRightChar(out, i-1, ']', &dFnt, MBF))
				lI = i - 1;
			else
				lI = i;
			if (i == 0)
				break;
			if (!strcmp(t->fromI->fromPats[0], "$b")) {
				if (isBeg) {
					if (t->fromI == t->from)
						t->isMatch = TRUE;
					t->fromI->si = sI;
					t->fromI->li = lI;
					t->fromI = t->fromI->nextPat;
				} else {
					t->fromI = t->from;
					t->isMatch = FALSE;
				}
			}
			if (t->fromI != NULL) {
				if (isAllAmbgMatch(w, t->fromI)) {
					if (t->fromI->isNeg) {
						t->fromI = t->from;
						t->isMatch = FALSE;
					} else if (!t->fromI->isMultRep) {
						if (t->fromI == t->from)
							t->isMatch = TRUE;
						t->fromI->si = sI;
						t->fromI->li = lI;
						t->fromI = t->fromI->nextPat;
					}
				} else if (t->fromI->isMultRep && t->fromI->nextPat != NULL) {
					t->fromI = t->fromI->nextPat;
					if (isAllAmbgMatch(w, t->fromI)) {
						if (t->fromI == t->from)
							t->isMatch = TRUE;
						t->fromI->si = sI;
						t->fromI->li = lI;
						t->fromI = t->fromI->nextPat;
// 2019-03-26
 					} else if (t->fromI->isNeg) {
						if (t->fromI == t->from)
							t->isMatch = TRUE;
						t->fromI->si = sI;
						t->fromI->li = lI;
						t->fromI = t->fromI->nextPat;
//
					} else {
						t->fromI = t->from;
						t->isMatch = FALSE;
					}
				} else if (t->fromI->isNeg) {
					if (t->fromI == t->from)
						t->isMatch = TRUE;
					t->fromI->si = sI;
					t->fromI->li = lI;
					t->fromI = t->fromI->nextPat;
				} else if (t->fromI->isWild) {
					if (t->fromI->negs != NULL) {
						if (isAllAmbgMatch(w, t->fromI->negs)) {
							t->fromI = t->from;
							t->isMatch = FALSE;
							if (isAllAmbgMatch(w, t->fromI)) {
								if (t->fromI == t->from)
									t->isMatch = TRUE;
								t->fromI->si = sI;
								t->fromI->li = lI;
								t->fromI = t->fromI->nextPat;
							}
						}
					}
				} else {
//					if (t->fromI != NULL && t->fromI != t->from)
//						i = t->from->li;
					t->fromI = t->from;
					t->isMatch = FALSE;
				}
				if (t->fromI != NULL) {
					if (!strcmp(t->fromI->fromPats[0], "$e") && !isBeg && mor_isEndOfTier(out, i)) {
						t->fromI->si = sI;
						t->fromI->li = lI;
						t->fromI = t->fromI->nextPat;
						if (t->fromI == t->from)
							t->isMatch = TRUE;
					}
				}
			}
			if (t->fromI == NULL && t->isMatch) {
				i = mor_replaceItems(t, out, i);
				if (isKillProgram)
					return;
				t->fromI = t->from;
				t->isMatch = FALSE;
			}
			isBeg = FALSE;
		}
	}
}
// Prepost end
#define PrepostEND

static void init_lex(char f) {
	register int i;
	FNType mFileName[FNSize];

	if (f) {
		sf_rules = NULL;
		ex_rules = NULL;
		out_rules = NULL;
		lex_root = NULL;
		lex_frow = NULL;
		tutt = NULL;
		rootXTest = NULL;
		empty_cat[0] = 0;
		isPinyin = FALSE;
		isCompoundsTest = FALSE;
		isAmbiguousTest = FALSE;
		isLookAtLexFiles = FALSE;
		doCapitals = TRUE;
		doLocations = FALSE;
		isLookAtDep = false;

		crules = NULL;
		drules = NULL;
		other_rules = NULL;
		mem_ctr = 0;
		trie_root = NULL;
		trie_node_ctr = 0;
		letter_ctr = 0;
		trie_entry_ctr = 0;
		features = NULL;
		feat_node_ctr = 0;
		val_node_ctr = 0;
		rt_entry_ctr = 0L;
		ar_files_list = NULL;
		crule_list = NULL;
		drule_list = NULL;
		drules_linenum = 0;
		crules_linenum = 0;
		startrules_list = NULL;
		endrules_list = NULL;
		e = NULL;
		w = NULL;
		c = NULL;
		eparse[0] = EOS;
		etrans[0] = EOS;
		ecomp[0]  = EOS;
		estr[0]   = EOS;
		feat_codes = NULL;
		match_stack = NULL;
		word_segments = NULL;
		var_tab = NULL;
		result_list = NULL;
		strcpy(mor_output_fname, "output.cut");
		strcpy(mor_sfrules_fname, "sf.cut");
		strcpy(mor_exrules_fname, "ex.cut");
		strcpy(mor_arules_fname, "ar.cut");
		strcpy(mor_crules_fname, "cr.cut");
		strcpy(drules_fname, "dr.cut");
	} else {
// 2012-11-01 2012-10-16 comma to %mor
		for (i=0; GlobalPunctuation[i]; ) {
			if (GlobalPunctuation[i] == ',')
				strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
			else
				i++;
		}
//
//2019-04-29		isRemoveCAChar[NOTCA_OPEN_QUOTE] = FALSE;
//2019-04-29		isRemoveCAChar[NOTCA_CLOSE_QUOTE] = FALSE;
		isRemoveCAChar[NOTCA_DOUBLE_COMMA] = FALSE;
		isRemoveCAChar[NOTCA_VOCATIVE] = FALSE;
		SetNewVol(wd_dir);
		/* open output file */
		if ((other_rules=OpenMorLib(mor_output_fname, "r", FALSE, FALSE, mFileName)) != NULL) {
			fprintf(stderr, "\rUsing output-rule: %s.\n", mFileName);
			while (fgets_cr(w_temp, MAXLINE, other_rules)) {
				uS.remFrontAndBackBlanks(w_temp);
				if (uS.isUTF8(w_temp) || uS.isInvisibleHeader(w_temp))
					continue;
				if (w_temp[0] == '%')
					continue;
				if (w_temp[0] != EOS) {
					i = strlen(w_temp) - 1;
					if (w_temp[i] == ':') {
						w_temp[i] = EOS;
						out_rules = AddToOutputRules(out_rules, w_temp, TRUE);
					} else
						out_rules = AddToOutputRules(out_rules, w_temp, FALSE);
				}
			}
			fclose(other_rules);
		}

		/* open sfrules file */
		if ((other_rules=OpenMorLib(mor_sfrules_fname, "r", FALSE, FALSE, mFileName)) != NULL) {
			fprintf(stderr, "\rUsing sf-rule: %s.\n", mFileName);
			while (fgets_cr(w_temp, MAXLINE, other_rules)) {
				uS.remblanks(w_temp);
				if (uS.isUTF8(w_temp) || uS.isInvisibleHeader(w_temp))
					continue;
				if (w_temp[0] == '%')
					continue;
				for (i=0; isSpace(w_temp[i]); i++) ;
				if (i > 0)
					strcpy(w_temp, w_temp+i);
				i = 0;
				if (w_temp[i] != EOS) {
					int j=i;
					for (; !isSpace(w_temp[i]) && w_temp[i] != EOS; i++) ;
					if (w_temp[j] == '\\' && i-j < 3) {
						j = i;
						for (; isSpace(w_temp[i]); i++) ;
						strcpy(w_temp+j, w_temp+i);
						for (i=j; !isSpace(w_temp[i]) && w_temp[i] != EOS; i++) ;
					}
					if (w_temp[i] != EOS) {
						w_temp[i] = EOS;
						sf_rules = AddToSFRules(sf_rules, w_temp, 0);
						for (i++; isSpace(w_temp[i]); i++) ;
						if (w_temp[i] != EOS) {
							sf_rules = AddToSFRules(sf_rules, w_temp+i, 1);
						}
					}
				}
			}
			fclose(other_rules);
		}

		/* open exrules file */
		if ((other_rules=OpenMorLib(mor_exrules_fname, "r", FALSE, FALSE, mFileName)) != NULL) {
			fprintf(stderr, "\rUsing ex-rule: %s.\n", mFileName);
			while (fgets_cr(w_temp, MAXLINE, other_rules)) {
				uS.remblanks(w_temp);
				if (uS.isUTF8(w_temp) || uS.isInvisibleHeader(w_temp))
					continue;
				if (w_temp[0] == '%')
					continue;
				i = strlen(w_temp) - 1;
				if (i >= 0 && w_temp[i] == '"')
					w_temp[i] = EOS;
				for (i=0; isSpace(w_temp[i]); i++) ;
				if (i > 0)
					strcpy(w_temp, w_temp+i);
				i = 0;
				if (w_temp[i] != EOS) {
					for (; !isSpace(w_temp[i]) && w_temp[i] != EOS; i++) ;
					if (isSpace(w_temp[i])) {
						w_temp[i] = EOS;
						ex_rules = AddToSFRules(ex_rules, w_temp, 0);
						for (i++; isSpace(w_temp[i]) || w_temp[i] == '"'; i++) ;
						if (w_temp[i] != EOS)
							ex_rules = AddToSFRules(ex_rules, w_temp+i, 1);
					}
				}
			}
			fclose(other_rules);
		}

		hello_arrays();

		for (i = 0; i < MAX_NUMVAR; i++) {
			strcpy(var_tab[i].var_name,"");
			strcpy(var_tab[i].var_pat,"");
			strcpy(var_tab[i].var_inst,"");
			var_tab[i].var_index = -1;
		}

		/* read in arules */
		if (FindAndOpenAr() == SUCCEED) {
			if (DEBUG_CLAN & 128)
				print_arules(ar_files_list);

			/*  process all lexicons; LOOK FOR LEX FILE: wd_dir/0add-to-mor-lex.cut */
			if (FindAndOpenLex() == FAIL) {
				fprintf(stderr,"Can't open lexicon file(s) or \"lex\" folder.\n");
				CloseFiles();
				cutt_exit(0);
			}
			fprintf(stderr, "\rLoaded lexicon: %ld   \n", rt_entry_ctr);
		} else {
			CleanUpAll(TRUE);
			CloseFiles();
			cutt_exit(0);
		}
		/* free up space as soon as we can */
		if (ar_files_list) { free_ar_files(ar_files_list); ar_files_list = NULL; }

		if (DEBUG_CLAN & 16) mem_usage();
		// mem_usage(); /* debug */

		/* open crules file */
		if ((crules=OpenMorLib(mor_crules_fname, "r", FALSE, FALSE, mFileName)) == NULL) {
			fprintf(stderr,"can't open either \"%s\" or \"%s\" file.\n",mor_crules_fname, mFileName);
#ifdef UNX
			fprintf(stderr,"Please use -l option to specify lexicon libary path.\n");
#else
			fprintf(stderr,"Please make sure that the \"mor lib\" in the Commands window is set to the folder in which the ar.cut file is located.\n");
#endif
			CloseFiles();
			cutt_exit(0);
		}
		fprintf(stderr,"\rUsing c-rules: %s.\n", mFileName);

		/* read c-rules */
		if (read_crules(crules) != SUCCEED) {
			/* free everything we've malloc'd */
			CleanUpAll(TRUE);
			CloseFiles();
			cutt_exit(0);
		}
		if (DEBUG_CLAN & 64)
			print_crules(crule_list);

		/* open drules file, if using them */
		if (DEBUG_CLAN & 32){
			print_drules(drule_list);
		}
		for (i=0; GlobalPunctuation[i]; ) {
			if (GlobalPunctuation[i] == '!' ||
				GlobalPunctuation[i] == '?' ||
				GlobalPunctuation[i] == '.')
				strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
			else
				i++;
		}
		if (crules) {
			fclose(crules);
			crules = NULL;
		}
		if (drules) {
			fclose(drules);
			drules = NULL;
		}

		if (isRunPrepost && prepostFP != NULL && prepostRules == NULL)
			readPrepost();

#ifndef UNX
		if (WD_Not_Eq_OD)
			SetNewVol(od_dir);
#endif
	}
}

#ifndef MOR_LIB
void usage() {
	printf("Usage: mor [eS p xi xl xb xc xd xa zN %s] filename(s)\n", mainflgs());
//	puts("+c : enable conll mode, output word's surface instead of stem");
//  puts("+dN: set level of diagnostic detail to N");
	puts("+d : do not run POST, POSTMORTEM and MEGRASP commands automatically");
	puts("+d1: do not run MEGRASP command automatically");
	puts("+eS: show result of a-rules on either a stem S or stems in file @S");
	ShowHelp(2);
//  puts("+gF: alternative grammar = F");
//  puts("     MOR expects set of files Far.cut Fcr.cut (Fdr.cut)");
#ifdef UNX
	puts("+LF: specify full path of the lexicon in F");
#endif
	puts("+p : use pinyin lexicon format");
	puts("+xi: interactive test mode");
	puts("+xl: check lexicon mode");
	puts("+xb: check lexicon mode, include word location in data files");
	puts("+xa: check lexicon for ambiguous entries");
	puts("+xc: check lexicon mode, including capitalized words");
	puts("+xd: check lexicon for compound words conflicting with plain words");
	puts("+xp: check lexicon mode, including words with prosodic symbols");
	puts("+xy: analyze words in lex files");
/* puts("+zN: N=0 - run comp. lex, N=1 - make comp. lex, N=2 - add to comp. lex"); */
	mainusage(TRUE);
}

void getflag(char *f, char *f1, int *i) {
	char *s;

	f++;
	switch(*f++) {
		case 'c':
			isConll = TRUE;
			no_arg_option(f);
			break;
		case 'e':
			isRunPrepost = FALSE;
			isRunPost = FALSE;
			isRunPostmortem = FALSE;
			isRunMegrasp = FALSE;
			DEBUGWORD = TRUE;
			for (s=f; *s != EOS; s++) {
				if (isalpha(*s))
					break;
			}
			if (isalpha(*s) || UTF8_IS_SINGLE((unsigned char)*f) || UTF8_IS_LEAD((unsigned char)*f) || *f == '@') {
				if (option_flags[CLAN_PROG_NUM] & SP_OPTION) {
					f -= 2;
					f[0] = '+'; f[1] = 's';
					maingetflag(f,f1,i);
				} else {
					long tmp;

					tmp = option_flags[MOR_P];
					option_flags[MOR_P] = option_flags[MOR_P]+SP_OPTION;
					f -= 2;
					f[0] = '+'; f[1] = 's';
					maingetflag(f,f1,i);
		 			option_flags[MOR_P] = tmp;
				}
			}
			stin = TRUE; stout = TRUE;
			chatmode = 4;
			break;
		case 'd':
			if (*f == EOS) {
				isRunPost = FALSE;
				isRunPostmortem = FALSE;
				isRunMegrasp = FALSE;
			} else if (*f == '1' && *(f+1) == EOS) {
				isRunMegrasp = FALSE;
			} else {
				fprintf(stderr, "Illegal argument used with option %s.\n", f-2);
				cutt_exit(0);
			}
			break;
/* 24-5-99
		case 'd':
			int j;
 			isRunPrepost = FALSE;
 			isRunPost = FALSE;
			isRunPostmortem = FALSE;
			isRunMegrasp = FALSE;
			j = atoi(f);
			if (j == 1) DEBUG_CLAN = DEBUG_CLAN | 2 | 4 | 8;
			else if (j == 2) DEBUG_CLAN = DEBUG_CLAN | 2;
			else if (j == 3) DEBUG_CLAN = DEBUG_CLAN | 4;
			else if (j == 4) DEBUG_CLAN = DEBUG_CLAN | 8;
			else if (j == 5) DEBUG_CLAN = DEBUG_CLAN | 16;
			else if (j == 6) DEBUG_CLAN = DEBUG_CLAN | 32;
			else if (j == 7) DEBUG_CLAN = DEBUG_CLAN | 64;
			else if (j == 8) DEBUG_CLAN = DEBUG_CLAN | 128;
			break;
*/
#ifdef UNX
		case 'L':
			int j;

			if (*f == '/')
				strcpy(mor_lib_dir, f);
			else {
				strcpy(mor_lib_dir, wd_dir);
				addFilename2Path(mor_lib_dir, f);
			}
			j = strlen(mor_lib_dir);
			if (j > 0 && mor_lib_dir[j-1] != '/')
				strcat(mor_lib_dir, "/");
			break;
#endif
		case 'p':
			if (*f != '1') {
				isPinyin = TRUE;
				no_arg_option(f);
			}
			break;
		case 'x':
			isRunPrepost = FALSE;
			isRunPost = FALSE;
			isRunPostmortem = FALSE;
			isRunMegrasp = FALSE;
			if (*f == 'a') {
				TEST = TRUE;
				isAmbiguousTest = TRUE;
				chatmode = 4;
				stin = TRUE;
				stout = TRUE;
			} else if (*f == 'b') {
				doLocations = TRUE;
				CHECK_LEXICON = TRUE;
				combinput = TRUE;
				stout = TRUE;
			} else if (*f == 'c') {
				doCapitals = FALSE;
				CHECK_LEXICON = TRUE;
				combinput = TRUE;
				stout = TRUE;
			} else if (*f == 'd') {
				TEST = TRUE;
				chatmode = 4;
				stin = TRUE;
				stout = TRUE;
				strcpy(GExt, ".ulx");
				isCompoundsTest = TRUE;
			} else if (*f == 'i') {
				TEST = TRUE;
				chatmode = 4;
				stin = TRUE;
				stout = TRUE;
			} else if (*f == 'l') {
				CHECK_LEXICON = TRUE;
				combinput = TRUE;
				stout = TRUE;
			} else if (*f == 'y') {
				isLookAtLexFiles = TRUE;
				strcpy(GExt, ".ambig");
				chatmode = 0;
			} else {
				fprintf(stderr, "Illegal argument used with option %s.\n", f-2);
				cutt_exit(0);
			}
			break;

		case 't':
			if (*f == '%')
				isLookAtDep = true;
			maingetflag(f-2,f1,i);
			break;
		case 's':
			if (*f == '[' && *(f+1) == '-') {
				maingetflag(f-2,f1,i);
/* enabled 2017-12-04, disabled 2018-04-04
			} else if (*f == '[' && *(f+1) == '+') {
				maingetflag(f-2,f1,i);
*/
			} else {
				fprintf(stderr, "Please specify only language codes, \"[- ...]\", with +/-s option.\n");
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

void init(char f) {
	if (f) {
		stout = FALSE;
		mor_ftime = TRUE;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		CHECK_LEXICON = FALSE;
		DEBUGWORD = FALSE;
		TEST = FALSE;
		ReLoadRules = FALSE;
		isConll = FALSE;
		addword('\0','\0',"+<e>");
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+[- *]");
		addword('\0','\0',"+0");
		addword('\0','\0',"+0*");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+xx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+yy");
		addword('\0','\0',"+www");
		free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
// defined in "mmaininit" and "globinit" - nomap = TRUE;

		init_lex(f);
	} else {
		if (mor_ftime) {
			mor_ftime = FALSE;
			init_lex(f);
			if (isLookAtLexFiles)
				replaceFile = FALSE;
			OverWriteFile = TRUE;
		}
	}
}
#endif // MOR_LIB

static void free_sfrules(SFRULES *p) {
	SFRULES *t;

	while (p != NULL) {
		t = p;
		p = p->nextSF;
		if (t->pat)
			free(t->pat);
		if (t->cat)
			free(t->cat);
		free(t);
	}
}

static void free_outputrules(OUTPUTRULES *p) {
	OUTPUTRULES *t;

	while (p != NULL) {
		t = p;
		p = p->nextOut;
		if (t->cat) free(t->cat);
		free(t);
	}
}

void CloseFiles(void) {
	if (crules) {
		fclose(crules);
		crules = NULL;
	}
	if (drules) {
		fclose(drules);
		drules = NULL;
	}
	if (debug_fp != stdout) {
		fclose(debug_fp);
		debug_fp = stdout;
	}
}

static struct mor_PATS *freeMorPATS(struct mor_PATS *p) {
	int patsNum;
	struct mor_PATS *t;

	while (p != NULL) {
		t = p;
		p = p->nextPat;
		if (t->negs != NULL)
			t->negs = freeMorPATS(t->negs);
		for (patsNum=0; patsNum < AMBIGS; patsNum++) {
			if (t->fromPats[patsNum] != NULL)
				free(t->fromPats[patsNum]);
		}
		if (t->toLiteral != NULL)
			free(t->toLiteral);
		free(t);
	}
	return(p);
}

static struct prepost_rules *prepost_cleanup(struct prepost_rules *p) {
	struct prepost_rules *t;

	while (p != NULL) {
		t = p;
		p = p->nextRule;
		t->from = freeMorPATS(t->from);
		free(t);
	}
	return(NULL);
}

void CleanUpAll(char all) {
	prepostRules = prepost_cleanup(prepostRules);
	if (trie_root)  { free_trie(trie_root); trie_root = NULL; }
	if (lex_root) {
		free_lex_lett(lex_root[0].next_lett, lex_root[0].num_letters);
		free(lex_root);
		lex_root = NULL;
	}
	if (lex_frow)   { free(lex_frow); lex_frow = NULL; }
	if (features)   { free_feats(features); features = NULL; }
	if (ar_files_list) { free_ar_files(ar_files_list); ar_files_list = NULL; }
	if (crule_list) { free_crules(crule_list); crule_list = NULL; }
	if (drule_list) { free_drules(drule_list); drule_list = NULL; }
	if (startrules_list) { free_rulepack(startrules_list); startrules_list = NULL; }
	if (endrules_list)   { free_rulepack(endrules_list); endrules_list = NULL; }
	if (sf_rules)	{ free_sfrules(sf_rules); sf_rules = NULL; }
	if (out_rules)	{ free_outputrules(out_rules); out_rules = NULL; }
	if (ex_rules)	{ free_sfrules(ex_rules); ex_rules = NULL; }
	bye_arrays();
}

static void freeAdd(struct mor_add *p) {
	struct mor_add *t;

	while (p != NULL) {
		t = p;
		p = p->nextAdd;
		free(t);
	}
}

static void mor_pr_result(struct mor_X_option *p, FILE *fp) {
	struct mor_add *tadd;

	if (p != NULL) {
		mor_pr_result(p->left, fp);
		fprintf(fpout,"%s\n", p->line);
		if (p->add != NULL) {
			for (tadd=p->add; tadd != NULL; tadd=tadd->nextAdd) {
				fprintf(fpout,"        %s\n", tadd->str);
//				fprintf(fpout,"\t%u %s\n", tadd->count, tadd->str);
			}
		}
		mor_pr_result(p->right, fp);
		free(p->line);
		freeAdd(p->add);
		free(p);
	}
}

static struct mor_add *addToAdd(struct mor_add *root_add, char *str) {
	struct mor_add *p;

	if (root_add == NULL) {
		if ((p=NEW(struct mor_add)) == NULL)
			out_of_mem();
		root_add = p;
	} else {
		for (p=root_add; p->nextAdd != NULL; p=p->nextAdd) {
			if (!doLocations) {
				if (!strcmp(p->str, str)) {
//					p->count++;
					return(root_add);
				}
			}
		}
		if (!doLocations) {
			if (!strcmp(p->str, str)) {
//				p->count++;
				return(root_add);
			}
		}
		if ((p->nextAdd=NEW(struct mor_add)) == NULL)
			out_of_mem();
		p = p->nextAdd;
	}
	p->nextAdd = NULL;
//	p->count = 1;
	p->str = (char *)malloc(strlen(str)+1);
	if (p->str == NULL) out_of_mem();
	strcpy(p->str, str);
	return(root_add);
}

static struct mor_X_option *insert(struct mor_X_option *p, char *line) {
	int cond;
	char *div;

	div = strchr(line, '~');
	if (div != NULL)
		*div = EOS;
	
	if (p == NULL) {
		if ((p=NEW(struct mor_X_option)) == NULL)
			out_of_mem();

// 2019-07-30 change dash to 0x2013
		for (cond=0; line[cond] != EOS; cond++) {
			if (line[cond] == (char)0xE2 && line[cond+1] == (char)0x80 && line[cond+2] == (char)0x93) {
				strcpy(line+cond, line+cond+2);
				line[cond] = '-';
			}
		}

		p->line = (char *)malloc(strlen(line)+1);
		if (p->line == NULL)
			out_of_mem();
		strcpy(p->line, line);
		p->add = NULL;
		if (div != NULL) {
			*div = '~';
			p->add = addToAdd(p->add, div+1);
		}
		p->left = p->right = NULL;
	} else if ((cond=strcmp(line, p->line)) < 0) {
		if (div != NULL)
			*div = '~';
		p->left= insert(p->left,line);
	} else if (cond > 0) {
		if (div != NULL)
			*div = '~';
		p->right = insert(p->right,line);
	} else if (div != NULL) {
		*div = '~';
		p->add = addToAdd(p->add, div+1);
	}
	if (div != NULL)
		*div = '~';
	return(p);
}

static void Uniq_results(void) {
	FNType mFileName[FNSize];

	fprintf(stderr, "Creating unique list of lexicon entries.\n");
	parsfname(Xoption_fname, mFileName, ".ulx");
	strcat(mFileName, ".cex");
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
#endif
#if defined(_MAC_CODE)
	settyp(mFileName, 'TEXT', the_file_creator.out, FALSE);
#endif
	if ((fpout=fopen(mFileName, "w")) == NULL) {
		fprintf(stderr,"Can't write file %s.\n", mFileName);
		return;
	}
	mor_pr_result(rootXopt, fpout);
	rootXopt = NULL;
	fclose(fpout);
	fprintf(stderr,"Output file <%s>\n", mFileName);
}


#ifndef MOR_LIB
CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int  i;
#ifndef UNX
	char comName[128], *tComName;
// char dashOne[3]; // lxs -1 lxs-1 2019-03-15
	extern void (*clan_main[LAST_CLAN_PROG]) (int argc, char *argv[]);
#endif //UNX
	char isAborted;

	isRunPrepost = TRUE;
	prepostFP = NULL;
	prepost_fname[0] = EOS;
	prepostRules = NULL;
#ifdef UNX
	for (i=0; i < argc; i++) {
		if (argv[i][0] == '+' || argv[i][0] == '-') {
			if (argv[i][1] == 'L') {
				int j;
				char *f;
				
				f = argv[i]+2;
				if (*f == '/')
					strcpy(mor_lib_dir, f);
				else {
					strcpy(mor_lib_dir, wd_dir);
					addFilename2Path(mor_lib_dir, f);
				}
				j = strlen(mor_lib_dir);
				if (j > 0 && mor_lib_dir[j-1] != '/')
					strcat(mor_lib_dir, "/");
				break;
			}
		}
		printf(" %s", argv[i]);
	}
#endif
	strcpy(prepost_fname, wd_dir);
	addFilename2Path(prepost_fname, "prepost.cut");
	prepostFP = fopen(prepost_fname, "r");
	if (prepostFP == NULL) {
		strcpy(prepost_fname, mor_lib_dir);
		addFilename2Path(prepost_fname, "prepost.cut");
		prepostFP = fopen(prepost_fname, "r");
	}

	rootXopt = NULL;
	Xoption_fname[0] = EOS;
	replaceFile = TRUE;
	totalWords = 0L;
	missingWords = 0L;
#ifdef UNX
	getcwd(wd_dir, FNSize);
	isRunPost = FALSE;
	isRunPostmortem = FALSE;
	isRunMegrasp = FALSE;
#else //UNX
	if (WD_Not_Eq_OD) {
		fprintf(stderr, "MOR can't run if \"working\" and \"output\" are set to different directories.\n");
		fprintf(stderr, "Please, set \"working\" and \"output\" to the same directory in \"Commands\" window.\n");
		cutt_exit(0);
	}
	isRunPost = TRUE;
	strcpy(DirPathName, mor_lib_dir);
	addFilename2Path(DirPathName, "postmortem.cut");
	if (!access(DirPathName,0))
		isRunPostmortem = TRUE;
	else
		isRunPostmortem = FALSE;
	strcpy(DirPathName, mor_lib_dir);
	addFilename2Path(DirPathName, "megrasp.mod");
	if (!access(DirPathName,0))
		isRunMegrasp = TRUE;
	else
		isRunMegrasp = FALSE;
/*
	if (uS.partwcmp(wd_dir, mor_lib_dir)) {
		for (i=strlen(wd_dir)-1; isSpace(wd_dir[i]) || wd_dir[i] == PATHDELIMCHR; i--) ;
		if (wd_dir[i] == PATHDELIMCHR)
			i--;
		for (; i >= 0 && wd_dir[i] != PATHDELIMCHR; i--) {
			if (uS.mStrnicmp(wd_dir+i, "train", 5) == 0) {
				isRunPost = FALSE;
				isRunPostmortem = FALSE;
				isRunMegrasp = FALSE;
				break;
			}
		}
	}
*/
#endif //else UNX
	TEST = FALSE;
	oldDEBUG = 0;
	debug_fp = stdout;
	do {
		DEBUG_CLAN = oldDEBUG;
		isWinMode = IS_WIN_MODE;
		chatmode = CHAT_MODE;
		CLAN_PROG_NUM = MOR_P;
		OnlydataLimit = 0;
		onlydata = TRUE;
		UttlineEqUtterance = FALSE;
/*
		if (NULL != 0) {
			fprintf(stderr, "Internal error: 'NULL' is not properly define.\n");
			if (debug_fp != stdout) {
				fclose(debug_fp);
				debug_fp = stdout;
			}
			cutt_exit(1);
		}
*/
		isAborted = FALSE;
		if (!bmain(argc,argv,NULL)) {
			isRunPrepost = FALSE;
			isRunPost = FALSE;
			isRunPostmortem = FALSE;
			isRunMegrasp = FALSE;
			isAborted = TRUE;
		}
		/* free everything we've malloc'd */
		if (!isMORXiMode || ReLoadRules)
			CleanUpAll(FALSE);
		if (CHECK_LEXICON) {
			if (rootXopt != NULL)
				Uniq_results();
			else if (!isAborted)
				fprintf(stderr, "\n    All words were found in the lexicon.\n");
		}
		if (ReLoadRules) {
#if defined(_MAC_CODE) || defined(_WIN32) 
			globinit();
#endif
		}
	} while (ReLoadRules) ;

	if (prepostFP != NULL) {
		fclose(prepostFP);
		prepostFP = NULL;
	}
	prepostRules = prepost_cleanup(prepostRules);
	if (isRunPost && !isKillProgram) {
#ifndef UNX
		globinit();
		my_flush_chr();
		replaceFile = TRUE;
		tComName = argv[0];
// RUN POST
		strcpy(comName, "post");
		CLAN_PROG_NUM = POST;
		printf("    %c%cAUTOMATICALLY RUNNING \"POST\" COMMAND.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
		printf(">");

		argv[0] = comName;
		for (i=0; i < argc; i++) {
			if (argv[i][0] == '+' || argv[i][0] == '-') {
				if (argv[i][1] == 't' && argv[i][2] == '%')
					argv[i][0] = EOS;
			}
			printf(" %s", argv[i]);
		}
		printf("\n");
		(*clan_main[CLAN_PROG_NUM])(argc, argv);
// RUN POSTMORTEM
		if (isRunPostmortem && !isKillProgram) {
			globinit();
			my_flush_chr();
			replaceFile = TRUE;
			strcpy(comName, "postmortem");
			CLAN_PROG_NUM = POSTMORTEM;
			printf("    %c%cAUTOMATICALLY RUNNING \"POSTMORTEM\" COMMAND.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
			printf(">");
			argv[0] = comName;
			for (i=0; i < argc; i++) {
				printf(" %s", argv[i]);
			}
			printf("\n");
			(*clan_main[CLAN_PROG_NUM])(argc, argv);
// RUN MEGRASP
			if (isRunMegrasp && !isKillProgram) {
				globinit();
				my_flush_chr();
				replaceFile = TRUE;
				strcpy(comName, "megrasp");
				CLAN_PROG_NUM = MEGRASP;
				printf("    %c%cAUTOMATICALLY RUNNING \"MEGRASP\" COMMAND.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
				printf(">");
				argv[0] = comName;
//				strcpy(dashOne, "-1");
//				argv[argc++] = dashOne;
				for (i=0; i < argc; i++) {
					printf(" %s", argv[i]);
				}
				printf("\n");
				(*clan_main[CLAN_PROG_NUM])(argc, argv);
			}
		}
// RESET BACK
		argv[0] = tComName;
		CLAN_PROG_NUM = MOR_P;
#endif //UNX
	}
	if (missingWords > 0L && !TEST && isRunPost && !isKillProgram) {
#ifdef UNX
		printf("\nYour transcript(s) have %ld word(s) with %ld word(s) that MOR does not recognize.\n", totalWords, missingWords);
		printf("These words are listed in a file labeled with the name of your input file that ends in \".ulx.cex\";\n");
		printf("you will find this file in the same directory containing the input file.\n");
		printf("Open the \".ulx.cex\" file and triple-click to go to the place of each error.\n");
		printf("After fixing the errors, please run MOR again.\n");
		printf("If you choose to work with incomplete data, you can skip all these steps.\n\n");
#else
		printf("\n%c%cYour transcript(s) have %ld word(s) with %ld word(s) that MOR does not recognize.\n", ATTMARKER, error_start, totalWords, missingWords, ATTMARKER, error_end);
		printf("%c%cThese words are listed in a file labeled with the name of your input file that ends in \".ulx.cex\";%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
		printf("%c%cyou will find this file in the same directory containing the input file.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
		printf("%c%cOpen the \".ulx.cex\" file and triple-click to go to the place of each error.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
		printf("%c%cAfter fixing the errors, please run MOR again.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
		printf("%c%cIf you choose to work with incomplete data, you can skip all these steps.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
		if (rootXopt != NULL)
			Uniq_results();
	}
/* debug *  fprintf(debug_fp,"mem_ctr: %ld\n",mem_ctr); */
/* debug *  mem_usage(); */
}
#endif // MOR_LIB

static XWORDLIST *outputXTests(const char *fname, const char *errmess, XWORDLIST *ptr) {
	long len, dlen;
	char isErrorFound, isFtime;
	XWORDLIST *t, *tptr;
	FILE *fp;

	isFtime = TRUE;
	fp = NULL;
	isErrorFound = FALSE;
	strcpy(DirPathName, mor_lib_dir);
	addFilename2Path(DirPathName, "lex");
	dlen = strlen(DirPathName);
	while (ptr != NULL) {
		if (ptr->matchW != NULL) {
			if (isFtime) {
				isFtime = FALSE;
				strcpy(FileName1, mor_lib_dir);
				addFilename2Path(FileName1, fname);
				fp = fopen(FileName1, "w");
				if (fp == NULL) {
					fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", FileName1);
				}
#ifdef _MAC_CODE
				else
					settyp(FileName1, 'TEXT', the_file_creator.out, FALSE);
#endif
			}
			isErrorFound = TRUE;
			if (fp != NULL) {
				fprintf(fp, "*********************************************\n");
				len = strlen(ptr->word);
				if (ptr->word[len+1] != EOS)
					ptr->word[len] = '\t';
				if (ptr->fname) {
					addFilename2Path(DirPathName, ptr->fname);
					fprintf(fp,"    File \"%s\": line %ld.\n", DirPathName, ptr->lineno);
					DirPathName[dlen] = EOS;
				}
				fprintf(fp, "%s\n", ptr->word);
			}
			tptr = ptr->matchW;
			while (tptr != NULL) {
				if (fp != NULL) {
					len = strlen(tptr->word);
					if (tptr->word[len+1] != EOS)
						tptr->word[len] = '\t';
					if (tptr->fname) {
						addFilename2Path(DirPathName, tptr->fname);
						fprintf(fp, "    File \"%s\": line %ld.\n", DirPathName, tptr->lineno);
						DirPathName[dlen] = EOS;
					}
					fprintf(fp, "%s\n", tptr->word);
				}
				t = tptr;
				tptr = tptr->matchW;
				if (t->word)
					free(t->word);
				if (t->plainW)
					free(t->plainW);
				if (t->fname)
					free(t->fname);
				free(t);
			}
		}
		t = ptr;
		ptr = ptr->nextW;
		if (t->word)
			free(t->word);
		if (t->plainW)
			free(t->plainW);
		if (t->fname)
			free(t->fname);
		free(t);
	}
	if (fp != NULL) {
		fclose(fp);
		if (isErrorFound) {
			fprintf(stderr,"\n*** File \"%s\".\n", FileName1);
			fprintf(stderr, "Found %s in lexicon file.\n\n", errmess);
		}
	}
	return(ptr);
}

static char justBlank(char *line) {
	register int i;

	i = 0;
	while ((i=getword(utterance->speaker, line, w, NULL, i))) {
		if (w[0] != '+' && strcmp(w, ".") != 0 && strcmp(w, "!") != 0 && strcmp(w, "?") != 0)
			return(FALSE);
	}
	return(TRUE);
}

/* preprocess_word - mm7c 1/22/93 */
/* look for proper nouns, and annotations */
static char preprocess_word(STRING *word, char *isZeroWord) {
	SFRULES *t;
	int  i;
	char isSCOLONFound, isMatchFound, ISenter_wordCalled;
	char *at, *dollar, *dash, *capWord, res, isSkip, scat[BUFSIZ], tScat[256], isL2;
	char *hyphenI;

	*isZeroWord = FALSE;
	ISenter_wordCalled = FALSE;
	if (word[0] == '0') {//2019-05-01
		result_index = 0;
		eparse[0] = EOS;
		etrans[0] = EOS;
		ecomp[0]  = EOS;
		analyze_word(word,empty_cat,eparse,etrans,ecomp,word,startrules_list,0,FALSE);
		if (result_index == 0) {//2019-05-01
			*isZeroWord = TRUE;
			strcpy(word, word+1);
		}
	}
	isMatchFound = FALSE;
	res = 0;
	dash = NULL;
	*w_temp = EOS;
	at = strrchr(word,'@');
	if (at != NULL)
		dollar = strchr(at,'$');
	else
		dollar = NULL;
	for (t=sf_rules; t != NULL; t=t->nextSF) {
repeat_preprocess_word:
		if (dollar != NULL)
			*dollar = EOS;
		if (*t->pat == '\\') {
			i = uS.fpatmat(word,t->pat+2);
			if (t->pat[1] == 'c' && !uS.isUpperChar(word, 0, &dFnt, MBF))
				i = 0;
			else if (t->pat[1] == 'l' && uS.isUpperChar(word, 0, &dFnt, MBF))
				i = 0;
		} else
			i = uS.fpatmat(word,t->pat);

		if (dollar != NULL)
			*dollar = '$';
		if (i == 0) {
			if (dash != NULL) {
				*dash = '-';
				dash = NULL;
			} else if ((dash=strrchr(word,'-')) != NULL) {
				if (strrchr(dash,'@') != NULL) {
					dash = NULL;
				} else {
					*dash = EOS;
					goto repeat_preprocess_word;
				}
			}
		}
// in sf.cut file add cat "delete" to delete wor from mor tier and "skip" to ignore the "@___" suffix.
// for example add lines:
// *@del	delete
// *@co	skip
// *@nc	skip
		if (i) {
			isSkip = FALSE;
  			if (at != NULL) { 
				isSCOLONFound = (at[1] == 's');
				if (!uS.mStricmp(t->cat,"del") || !uS.mStricmp(t->cat,"delete")) {
					word[0] = EOS;
				} else if (uS.mStricmp(t->cat,"skip") != 0) {
					if (dash != NULL)
						res = 1; // exception for *@*-s
					else
						res = 2;
					strncpy(w_temp,word,at-word);
					w_temp[at-word] = EOS;

// 2019-07-30 change dash to 0x2013
					hyphenI = strchr(w_temp, '-');
					while (hyphenI != NULL) {
//fprintf(stderr, "%s\n", w_temp);
						uS.shiftright(hyphenI, 2);
						*hyphenI++ = 0xE2;
						*hyphenI++ = 0x80;
						*hyphenI   = 0x93;
						hyphenI = strchr(w_temp, '-');
					}

					if (dollar != NULL && dollar[1] != EOS) {
						isL2 = FALSE;
						strcpy(tScat, t->cat);
						if (isSCOLONFound) {
							for (i=0; tScat[i] != EOS; i++) {
								if (uS.mStrnicmp(tScat+i, "scat", 2) == 0) {
									for (i=i+4; isSpace(tScat[i]); i++) ;
									strcpy(tScat, tScat+i);
									for (i=0; !isSpace(tScat[i]) && tScat[i]!=']' && tScat[i]!='}' && tScat[i]!=EOS; i++) ;
									tScat[i] = EOS;
									isL2 = TRUE;
									break;
								}
							}
						}
						if (isL2) {
							if (tScat[0] == EOS)
								sprintf(scat, "{[scat %s] [@S-L2 L2]}", dollar+1);
							else
								sprintf(scat, "{[scat %s] [@S-L2 %s]}", dollar+1, tScat);
						} else
							sprintf(scat, "{[scat %s]}", dollar+1);
						fs_comp(scat,tmp_feats_comp);
					} else
						fs_comp(t->cat,tmp_feats_comp);
					enter_word(w_temp,tmp_feats_comp,"","","",TRUE);
					ISenter_wordCalled = TRUE;
				} else {
					if (dollar != NULL && dollar[1] != EOS) {
						strncpy(w_temp,word,at-word);
						w_temp[at-word] = EOS;
						sprintf(scat, "{[scat %s]}", dollar+1);
						fs_comp(scat,tmp_feats_comp);
						enter_word(w_temp,tmp_feats_comp,"","","",TRUE);
						ISenter_wordCalled = TRUE;
						res = 2;
					} else {
						isSkip = TRUE;
					}
				}
				if (dash != NULL) {
					*dash = '-';
					dash = NULL;
				}
				if (isSCOLONFound) {
					for (i=1; at[i] != EOS; i++) ;
				} else {
					for (i=1; !uS.isskip(at,i,&dFnt,MBF) && at[i] != '+' && at[i] != '-' && at[i] != EOS; i++) ;
				}
				strcpy(at, at+i);
			}
			isMatchFound = TRUE;
			if (!isSkip)
				return(res);
		}
	}
	
	if (!isMatchFound) {
		dollar = strchr(word,'$');
		if (dollar != NULL && dollar[1] != EOS) {
			strncpy(w_temp,word,dollar-word);
			w_temp[dollar-word] = EOS;
			if (w_temp[0] != EOS) {
				sprintf(scat, "{[scat %s]}", dollar+1);
				fs_comp(scat,tmp_feats_comp);

// 2019-08-03 $ override
				result_index = 0;
				eparse[0] = EOS;
				etrans[0] = EOS;
				ecomp[0]  = EOS;
				analyze_word(w_temp,empty_cat,eparse,etrans,ecomp,w_temp,startrules_list,0,FALSE);
// char tcat[256], cat[MAX_WORD];
				for (i=result_index; i > 0; i--) {
/*
					strcpy(tcat, "[scat]");
					get_vname(tmp_feats_comp,tcat,cat);
					strcpy(tcat, "[scat]");
					get_vname(result_list[i].cat,tcat,cat);
*/
					if (featncmp(result_list[i].cat,tmp_feats_comp,featlen(tmp_feats_comp)) == 0) {
						res = 3;
						break;
					}
				}

				if (i == 0) {
					enter_word(w_temp,tmp_feats_comp,"","","",TRUE);
					res = 2;
				}

				for (i=1; !uS.isskip(dollar,i,&dFnt,MBF) && dollar[i] != '+' && dollar[i] != '-' && dollar[i] != EOS; i++) ;
				strcpy(dollar, dollar+i);
				return(res);
			}
		}
	}

/* old version 2019-08-03 $ override
	if (!isMatchFound) {
		dollar = strchr(word,'$');
		if (dollar != NULL && dollar[1] != EOS) {
			strncpy(w_temp,word,dollar-word);
			w_temp[dollar-word] = EOS;
			sprintf(scat, "{[scat %s]}", dollar+1);
			fs_comp(scat,tmp_feats_comp);
			enter_word(w_temp,tmp_feats_comp,"","","",TRUE);
			for (i=1; !uS.isskip(dollar,i,&dFnt,MBF) && dollar[i] != '+' && dollar[i] != '-' && dollar[i] != EOS; i++) ;
			strcpy(dollar, dollar+i);
			res = 2;
			return(res);
		}
	}
*/

	*w_temp = EOS;
	if ((word[0] == 'd' || word[0] == 'D' || word[0] == 'l' || word[0] == 'L') && word[1] == '\'')
		capWord = word + 2;
	else
		capWord = NULL;
	if (ISenter_wordCalled == TRUE) {
		capWord = NULL;
	} else if (capWord != NULL && uS.isUpperChar(capWord, 0, &dFnt, MBF) && (*(capWord+1) == '_' || isalpha(*(capWord+1)))) {
		/* try and put uppercase words in dict as proper nouns */
		result_index = 0;
		eparse[0] = EOS;
		etrans[0] = EOS;
		ecomp[0]  = EOS;
		analyze_word(capWord,empty_cat,eparse,etrans,ecomp,capWord,startrules_list,0,FALSE);
		if ((result_index == 0) && (*capWord != EOS) && doCapitals) {
			at = strchr(capWord,'-');
			if (at == NULL) {
				at = strchr(capWord,'\'');
			}
			if (at == NULL){
				strcpy(w_temp,capWord);
				strcpy(c,"{[scat n:prop] [allo n0]}");
			} else {
				strncpy(w_temp,capWord,at-capWord);
				w_temp[at-capWord]=EOS;
				strcpy(c,"{[scat n:prop] [allo n0]}");
			}
			fs_comp(c,tmp_feats_comp);
			enter_word(w_temp,tmp_feats_comp,"","","",TRUE);
			res = 1;
		}
	} else if (uS.isUpperChar(word, 0, &dFnt, MBF) && (*word != 'I' || word[1] == '_' || isalpha(word[1]))/* && !uS.isUpperChar(word, 1, &dFnt, MBF) */) {
		/* try and put uppercase words in dict as proper nouns */
		result_index = 0;
		eparse[0] = EOS;
		etrans[0] = EOS;
		ecomp[0]  = EOS;
		analyze_word(word,empty_cat,eparse,etrans,ecomp,word,startrules_list,0,FALSE);
		if ((result_index == 0) && (word[0] != EOS) && doCapitals) {
			at = strchr(word,'-');
			if (at == NULL) {
				at = strchr(word,'\'');
				if (at != NULL) {  /* make sure we've got a valid clitic */
					if (*(at+1) == EOS) ;
					else if (strcmp(at,"'s") == 0) ;
					else if (strcmp(at,"'ll") == 0) ;
					else if (strcmp(at,"'d") == 0) ;
					else at = NULL;
				}
			}
			if (at == NULL){
				strcpy(w_temp,word);
				strcpy(c,"{[scat n:prop] [allo n0]}");
			} else {
				strncpy(w_temp,word,at-word);
				w_temp[at-word]=EOS;
				strcpy(c,"{[scat n:prop] [allo n0]}");
			}
			fs_comp(c,tmp_feats_comp);
			enter_word(w_temp,tmp_feats_comp,"","","",TRUE);
			res = 1;
		}
	} else if (doDefComp && uS.patmat(word,"_*+_*")) { /* lxs lxslxs *+* */
		at = strchr(word,'-');
		if (at != NULL) {
			strncpy(w_temp,word,at-word);
			w_temp[at-word]=EOS;
			strcpy(c,"{[scat n+n+n]}");
			fs_comp(c,tmp_feats_comp);
			enter_word(w_temp,tmp_feats_comp,"","","",TRUE);
			res = 1;
		}
	}
	return(res);
}

static void FilterStem(char *s) {
	long i, j;
	
	for (i=0; s[i] != EOS;) {
		if (s[i] == '-' && s[i+1] == ':') {
			for (j=i+2; s[j] != ':' && s[j] != EOS; j++) ;
			if (s[j] == ':')
				strcpy(s+i, s+j+1);
			else
				i++;
		} else if (s[i] == '-' && s[i+1] == '#') { // grand&f-#n|maman -> grand#n|maman&f
			strcpy(s+i, s+i+1);
		} else
			i++;
	}
}

static char alreadyThere(const char *full, const char *out_cat, const char *cur) {
	int f, c;

	c = strlen(out_cat);
	f = strlen(cur);
	for (; *full != EOS; full++) {
		if (strncmp(full, out_cat, c) == 0) {
			if (!strncmp(full+c, cur, f))
				return(TRUE);
		}
	}
	return(FALSE);
}
/*
static void removeDuplicates(char *stem) {
	int i, j, max;
	char *e[20], c[20], *s, isRemoved;

	isRemoved = FALSE;
	max = 0;
	for (i=0; stem[i] != EOS; i++) {
		if (stem[i] == '-' || stem[i] == '&' || stem[i] == '$' || stem[i] == '~' || stem[i] == '#') {
			e[max] = stem+i+1;
			c[max] = stem[i];
			stem[i] = EOS;
			max++;
		}
		if (max == 20)
			break;
	}
	for (i=0; i < max; i++) {
		for (j=i+1; j < max; j++) {
			if (uS.mStricmp(e[i], e[j]) == 0) {
				isRemoved = TRUE;
				c[j] = ' ';
				for (s=e[j]; *s != EOS; s++) {
					*s = ' ';
				}
			}
		}
	}
	for (i=0; i < max; i++) {
		s = e[i];
		s--;
		*s = c[i];
	}
	if (isRemoved) {
		removeAllSpaces(stem);
	}
}
*/
static void infuseTransIntoStem(char *trans, char *stem) {
	long i, j, k;
	long len;
	
	i = 0L;
	j = 0L;
repeat:
	for (; stem[i] != EOS && stem[i] != '~' && stem[i] != '+' && stem[i] != '$' && 
		 stem[i] != '%' && stem[i] != '^' && stem[i] != '='; i++) ;
	
	for (len=0, k=j; trans[k]; k++,len++) {
		if (trans[k] == '+' && stem[i] == '+')
			break;
	}
	
	if (trans[k] == EOS)
		i = strlen(stem);
	else
		uS.shiftright(stem+i, len+1);
	stem[i++] = '=';
	for (; j < k; j++)
		stem[i++] = trans[j];
	if (trans[k] == EOS)
		stem[i] = EOS;
	if (trans[j] == '+' && stem[i] == '+') {
		j++;
		i++;
		goto repeat;
	}
}

static void breakUpComp(char *item, char *comp, char *output) {
	int  i;
	char *pc, *vb, *ua;
	char *cat, *cc, *stem, *cs;

	ua = strchr(item, '^');
	if (ua != NULL)
		*ua = EOS;
	vb = strchr(item, '$');
	pc = vb;
	while (vb != NULL) {
		pc = vb;
		vb = strchr(vb+1, '$');
	}
	if (pc == NULL) {
		vb = strchr(item, '|');
	} else {
		vb = strchr(pc, '|');
	}
	if (vb == NULL)
		return;
	*vb = EOS;
	cat = item;
	stem = vb + 1;

// 2023-03-28 change to dash from 0x2013
	for (i=0; stem[i] != EOS; i++) {
		if (stem[i] == (char)0xE2 && stem[i+1] == (char)0x80 && stem[i+2] == (char)0x93) {
			strcpy(stem+i+1, stem+i+3);
			stem[i] = '-';
		}
	}
	strcat(output, cat);
	strcat(output, "|+");
	cat = comp;
	cc = strchr(cat,  '+');
	cs = strchr(stem, '+');
	if (cs == NULL)
		cs = strchr(stem, '_');
	if (cs == NULL)
		cs = strchr(stem, '-');
	while (cc && cs) {
		*cc = EOS;
		*cs = EOS;
		strcat(output, cat);
		strcat(output, "|");
		strcat(output, stem);
		strcat(output, "+");
		cat = cc  + 1;
		stem = cs + 1;
		cc = strchr(cat,  '+');
		cs = strchr(stem, '+');
		if (cs == NULL)
			cs = strchr(stem, '_');
		if (cs == NULL)
			cs = strchr(stem, '-');
	}
	strcat(output, cat);
	strcat(output, "|");
	strcat(output, stem);	
	if (ua != NULL) {
		strcat(output, "^");
		strcat(output, ua+1);
	}
}

static char isExcludeForm(char *utt_out, long out_index, char *word) {
	SFRULES *t;
	
	if (utt_out[out_index] == '^')
		out_index++;
	for (t=ex_rules; t != NULL; t=t->nextSF) {
		if (!uS.mStricmp(t->pat, utt_out+out_index) && !uS.mStricmp(t->cat, word))
			return(true);
	}
	return(false);
}

static void integrateCompAndStem(char *stem, char *comp) {
	int  i, j, len;
	char plain[MAX_WORD], newStem[MAX_WORD];

	j = 0;
	for (i=0; comp[i] != EOS; i++) {
		if (comp[i] != '+')
			plain[j++] = comp[i];
	}
	plain[j] = EOS;
	len = j;
	j = 0;
	for (i=0; stem[i] != EOS && strncmp(plain, stem+i, len); i++) {
		newStem[j++] = stem[i];
	}
	if (stem[i] == EOS)
		strcpy(stem, comp);
	else {
		newStem[j] = EOS;
		strcat(newStem, comp);
		strcat(newStem, stem+i+len);
		strcpy(stem, newStem);
	}
}

static void integrateIntoBeforeSuffix(char *stem, char *feat) {
	char *s;

	if (feat[0] == EOS)
		return;
	templineC4[0] = EOS;
	s = strchr(stem, '~');
	if (s != NULL) {
		strcpy(templineC4, s);
		*s = EOS;
	}
	s = strchr(stem, '-');
	if (s == NULL) {
		strcat(stem, feat);
		strcat(stem, templineC4);
		return;
	}
	strcat(feat, s);
	*s = EOS;
	strcat(stem, feat);
	strcat(stem, templineC4);
}

//static void movePOS(char *w, char *c) {
//}

// squirm+worm row+boat
static void printword(STRING *word, RESULT_REC_PTR word_list, int num_words, STRING *utt_out, char isTempItem, char isZeroWord) {
	int  i;
	long out_index;
	char l2[4];
	char let[4];
	char tcat[256], comp[MAX_WORD], out_cat[MAX_WORD];
	char *prefix;
	char isCompFound;
	OUTPUTRULES *tor;
	
	/* mm: 1/22/93  got rid of loop for interactive coding */
	
	totalWords++;
	if ((num_words == 0) && (word[0] != EOS)) {
		/* word not recognized */
		strcat(utt_out,"\?|");
		if (isZeroWord) {//2019-05-01
			strcat(utt_out,"0");
		}
		strcat(utt_out,word);
		missingWords++;
	} else {
		for (i=num_words; i > 0; i--) {
			/* build up category string */
//			removeDuplicates(word_list[i].stem);
			if (isTempItem == 2 && featcmp(word_list[i].cat, tmp_feats_comp)) {
				if (DEBUG_CLAN & 4) {
					fprintf(debug_fp,"\nComparing Special Form marker to analysis category.\n");
					fs_decomp(tmp_feats_comp, out_cat);
					fprintf(debug_fp,"  Special Form = %s\n", out_cat);
					fs_decomp(word_list[i].cat, out_cat);
					fprintf(debug_fp,"  Analysis = %s\n", out_cat);
					fprintf(debug_fp,"  full match failed\n");
				}
				continue;
			} else if (isTempItem == 3) { // 2019-08-03 $ override
				if (featncmp(result_list[i].cat,tmp_feats_comp,featlen(tmp_feats_comp)) != 0) {
					continue;
				} else if (DEBUG_CLAN & 4) {
					fprintf(debug_fp,"\nComparing Special Form marker to analysis category.\n");
					fs_decomp(tmp_feats_comp, out_cat);
					fprintf(debug_fp,"  Special Form = %s\n", out_cat);
					fs_decomp(word_list[i].cat, out_cat);
					fprintf(debug_fp,"  Analysis = %s\n", out_cat);
					fprintf(debug_fp,"  full match failed\n");
				}
			}
			if (isZeroWord && i == num_words) {//2019-05-01
				strcat(utt_out,"0");
			}
			if (word_list[i].comp[0] != EOS)
				integrateCompAndStem(word_list[i].stem, word_list[i].comp);
			FilterStem(word_list[i].stem);
			strcpy(tcat, "[scat]");
			get_vname(word_list[i].cat,tcat,c);
			strcpy(tcat, "[comp]");
			get_vname(word_list[i].cat,tcat,comp);
			isCompFound = (comp[0] != EOS);
			strcpy(tcat, "[L2]");
			get_vname(word_list[i].cat,tcat,l2);
			if (!(*l2 == EOS))
				strcat(c,":L2");
			strcpy(tcat, "[letter]");
			get_vname(word_list[i].cat,tcat,let);
			if (!(*let == EOS))
				strcat(c,":let");
			/* alternative forms separated by "^" */
			out_index = strlen(utt_out);
			if (i != num_words && out_index > 0 && !isSpace(utt_out[out_index-1])) {
				strcat(utt_out,"^");
				if (isZeroWord)//2022-08-03
					strcat(utt_out,"0");
			}
			if (out_rules != NULL) {
				templineC3[0] = EOS;
				for (tor=out_rules; tor != NULL; tor=tor->nextOut) {
					if (tor->cat != NULL) {
						sprintf(out_cat, "[%s]", tor->cat);
						get_vname(word_list[i].cat,out_cat,templineC4);
						if (*templineC4 != EOS) {
							if (tor->isFeatureAdded)
								sprintf(out_cat, "%c%s:", tor->delim, tor->cat);
//							else if (templineC4[0] == '-' || templineC4[0] == '+')
//								out_cat[0] = EOS;
							else {
								out_cat[0] = tor->delim;
								out_cat[1] = EOS;
							}
							if (!alreadyThere(word_list[i].stem, out_cat, templineC4)) {
								strcat(templineC3, out_cat);
								strcat(templineC3, templineC4);
							}
						}
					}
				}
				integrateIntoBeforeSuffix(word_list[i].stem, templineC3);
			}
			strcpy(tcat, "[proc]");
			get_vname(word_list[i].cat,tcat,templineC4);
			if (*templineC4 != EOS) {
				uS.uppercasestr(templineC4, &dFnt, C_MBF);
				if (!alreadyThere(word_list[i].stem, "-", templineC4)) {
					strcat(word_list[i].stem, "-");
					strcat(word_list[i].stem, templineC4);
				}
			}
			templineC4[0] = EOS;
			if (word_list[i].trans[0])
				infuseTransIntoStem(word_list[i].trans, word_list[i].stem);
			prefix = strrchr(word_list[i].stem, '#');
			if (prefix == NULL)
				prefix = strrchr(word_list[i].stem, '$');
			/* errors preceeded by "*" */
			if (check_fvp(word_list[i].cat,err_f_comp)) {
				strcat(templineC4,"*");
				if (prefix != NULL)
					strncat(templineC4, word_list[i].stem, prefix-word_list[i].stem+1);
				strcat(templineC4, c);
			} else {
				if (prefix != NULL)
					strncat(templineC4, word_list[i].stem, prefix-word_list[i].stem+1);
				strcat(templineC4, c);
			}
			strcat(templineC4, "|");
			if (prefix != NULL)
				strcat(templineC4, prefix+1);
			else if (isConll)
				strcat(templineC4, word_list[i].surface);
			else
				strcat(templineC4, word_list[i].stem);
//			if (strchr(templineC4, '$') != NULL)
//				movePOS(templineC4, c);
			if (isCompFound && !isConll) {
				templineC3[0] = EOS;
				breakUpComp(templineC4, comp, templineC3);
				strcat(utt_out, templineC3);
			} else
				strcat(utt_out, templineC4);
			if (isExcludeForm(utt_out, out_index, word)) {
				utt_out[out_index] = EOS;
			}
		}
	}
}

static void mor_process_strings() {
	register int k;
	register int i;
	register int j;
	int  cnt;
	char isTempItem = 0, isZeroWord, tcat[256];
	
	k = 0;
	cnt = 0;
	*tutt = EOS;
	while ((k=getword("*", uttline, w, NULL, k))) {
		cnt++;
		isTempItem = preprocess_word(w, &isZeroWord);
		if (w[0] == EOS)
			continue;
		result_index = 0;
		if (!stin)
			fprintf(fpout,"%s:",w);
		eparse[0] = EOS;
		etrans[0] = EOS;
		ecomp[0]  = EOS;
		analyze_word(w,empty_cat,eparse,etrans,ecomp,w,startrules_list,0,doDefComp);
		if (!stin) {
			*spareTier1 = EOS;
			for (i = result_index, j=1; 0 < i; i--, j++) {
				if (isTempItem == 2 && featcmp(result_list[i].cat, tmp_feats_comp))
					continue;
				strcpy(tcat, "[scat]");
				get_vname(result_list[i].cat,tcat,c);
				fprintf(fpout,"\t");
				if (check_fvp(result_list[i].cat,err_f_comp))
					fprintf(fpout,"*");
				if (result_list[i].comp[0])
					fprintf(fpout,"%s\t%s", c, result_list[i].comp);
				else
					fprintf(fpout,"%s\t%s", c, result_list[i].stem);
				if (result_list[i].trans[0])
					fprintf(fpout,"=%s\n", result_list[i].trans);
				else
					fprintf(fpout,"\n");
			}
			printword(w,result_list,result_index,spareTier1,isTempItem,isZeroWord);
			fprintf(fpout,"\nResult: %s\n", spareTier1);
			strcat(tutt, " ");
			strcat(tutt, spareTier1);
		} else {
			*spareTier1 = EOS;
			for (i = result_index, j=1; 0 < i; i--, j++) {
				if (isTempItem == 2 && featcmp(result_list[i].cat, tmp_feats_comp))
					continue;
				fs_decomp(result_list[i].cat,c);
				if (debug_fp != stdout) {
					fprintf(debug_fp,"parse %d:\n",j); /* verbose output */
					fprintf(debug_fp,"\tlex info: %s\n",c);
					fprintf(debug_fp,"\tmorphemes (surface/stem): %s\n",result_list[i].stem);
					fprintf(debug_fp,"\tcompound: %s\n",result_list[i].comp);
					fprintf(debug_fp,"\ttranslation: %s\n",result_list[i].trans);
				}
				printf("parse %d:\n",j); /* verbose output */
				printf("\tlex info: %s\n",c);
				printf("\tmorphemes (surface/stem): %s\n",result_list[i].stem);
				printf("\tcompound: %s\n",result_list[i].comp);
				printf("\ttranslation: %s\n",result_list[i].trans);
			}
			printword(w,result_list,result_index,spareTier1,isTempItem,isZeroWord);
			if (debug_fp != stdout)
				fprintf(debug_fp,"\nResult: %s\n", spareTier1);
			printf("\nResult: %s\n", spareTier1);
			strcat(tutt, " ");
			strcat(tutt, spareTier1);
		}
		if (isTempItem) {
			if (isTempItem != 3)
				delete_word(w_temp,tmp_feats_comp);
			isTempItem = 0;
		}
	}
	if (cnt > 1) {
		if (tutt[0] == ' ')
			strcpy(tutt, tutt+1);
		if (tutt[0] != EOS)
			printf("\n%%mor:\t%s\n", tutt);
	}
}

static char call_test_mode(char isFirst) {
	char isQuit;
#ifndef UNX
	isQuit = FALSE;
	if (isFirst) {
		DEBUG_CLAN = 4 | 8; // disable :d after :l
		ShowHelp(1);
		if (tComWinDataChanged)
			isMORXiMode = 2;
		else
			isMORXiMode = 1;
#ifdef _COCOA_APP
		StdInWindow = NULL; // global_df->fileName;
#else
		StdInWindow = global_df->fileName;
#endif
		StdInErrMessage = "Please finish \"mor\" test mode first.";
		StdDoneMessage  = "Please finish \"mor\" test mode first. Type :q.";
	} else {
		stin = TRUE;
		stout = TRUE;
		fpout = stdout;
		printf("\n");
		uttline = morTestCom;
		/* word or command? commands prefaced with ":" */
		if (*uttline == ':') {
			if (uttline[1] == 'q' || uttline[1] == 'Q') {
				printf("> ");
				my_flush_chr();
				if (isMORXiMode == 2) {
#ifndef _COCOA_APP
					global_df->DataChanged = TRUE;
#endif
				} else if (isMORXiMode == 1) {
#ifndef _COCOA_APP
					global_df->DataChanged = FALSE;
#endif
				}
				isMORXiMode = FALSE;
				StdInWindow = NULL;
				StdInErrMessage = NULL;
				StdDoneMessage  = NULL;
				CleanUpAll(TRUE);
				isQuit = TRUE;
			} else if (uttline[1] == 'l' || uttline[1] == 'L') {
				CleanUpAll(FALSE);
//				globinit();
//				mmaininit();
				init_lex('\001');
				init_lex('\0');
				fpout = stdout;
				oldDEBUG = DEBUG_CLAN;
			} else if (uttline[1] == 'h' || uttline[1] == 'H') {
				ShowHelp(1);
			} else if (uttline[1] == 'c' || uttline[1] == 'C') {
				print_crules(crule_list);
				fprintf(debug_fp,"startrules list = ");
				print_rulepacks(startrules_list);
				fprintf(debug_fp,"\nendrules list = ");
				print_rulepacks(endrules_list);
			} else if (uttline[1] == 'd' || uttline[1] == 'D') {
				DEBUG_CLAN = DEBUG_CLAN | 2;
				printf("Now please re-load rules with \":l\"\n");
				printf("Output will be stored into file \"debug.cdc\"\n");
			}
		} else if (uttline[0] != EOS) { /* assume it's a word- process it */
			if ((debug_fp=fopen(debugName,"w")) == NULL) {
				fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", debugName);
				debug_fp = stdout;
			} else {
#ifdef _MAC_CODE
				settyp(debugName, 'TEXT', the_file_creator.out, FALSE);
#endif
				printf("*** File \"%s\"\n", debugName);
				fprintf(debug_fp,"%s\n", UTF8HEADER);
			}
			mor_process_strings();
			if (debug_fp != stdout) {
				fclose(debug_fp);
				debug_fp = stdout;
			}
		}
	}
	if (isKillProgram) {
		if (isKillProgram != 2) {
			isKillProgram = 0;
			StdInWindow = NULL;
			StdInErrMessage = NULL;
			StdDoneMessage  = NULL;
			CleanUpAll(TRUE);
			fprintf(stderr, "\n\nOutput is aborted by the user.\n");
			printf("> ");
			my_flush_chr();
			if (isMORXiMode == 2) {
#ifndef _COCOA_APP
				global_df->DataChanged = TRUE;
#endif
			} else if (isMORXiMode == 1) {
#ifndef _COCOA_APP
				global_df->DataChanged = FALSE;
#endif
			}
			isMORXiMode = FALSE;
			isQuit = TRUE;
		}
	} else if (isMORXiMode) {
		printf("\nmor (:h help)> ");
		my_flush_chr();
	}
#else // UNX
	isQuit = TRUE;
	DEBUG_CLAN = 4 | 8; // disable :d after :l
	ShowHelp(1);
	while(1) {
		printf("\nmor (:h help)> ");
		if (fgets_cr(uttline,UTTLINELEN,stdin) == NULL) {
			printf("\n");
			break;
		}
		/* word or command? commands prefaced with ":" */
		if (*uttline == ':') {
			if (uttline[1] == 'q' || uttline[1] == 'Q')
				break;
			else if (uttline[1] == 'l' || uttline[1] == 'L') {
				ReLoadRules = TRUE;
				oldDEBUG = DEBUG_CLAN;
				break;
			} else if (uttline[1] == 'h' || uttline[1] == 'H') {
				ShowHelp(1);
			} else if (uttline[1] == 'c' || uttline[1] == 'C') {
				print_crules(crule_list);
				fprintf(debug_fp,"startrules list = ");
				print_rulepacks(startrules_list);
				fprintf(debug_fp,"\nendrules list = ");
				print_rulepacks(endrules_list);
			} else if (uttline[1] == 'd' || uttline[1] == 'D') {
				DEBUG_CLAN = DEBUG_CLAN | 2;
				printf("Now please re-load rules with \":l\"\n");
				printf("Output will be stored into file \"debug.cdc\"\n");
			}
		} else { /* assume it's a word- process it */
			if ((debug_fp=fopen(debugName,"w")) == NULL) {
				fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", debugName);
				debug_fp = stdout;
			} else {
				printf("*** File \"%s\"\n", debugName);
				fprintf(debug_fp,"%s\n", UTF8HEADER);
			}
			mor_process_strings();
			if (debug_fp != stdout) {
				fclose(debug_fp);
				debug_fp = stdout;
			}
		}
	}  /* bottom of interactive loop */
#endif // UNX
	return(isQuit);
}

static void call_analyze_lex() {
	unsigned int ln = 0;
	int  wi;
	char isTempItem = 0, isZeroWord;

	if (!stout)
		fprintf(stderr,"\r                                     \r");
	else
		fprintf(stderr,"\n");
	while (fgets_cr(uttline, UTTLINELEN, fpin)) {
		if (uS.isUTF8(uttline) || uS.isInvisibleHeader(uttline))
			continue;
		uS.remblanks(uttline);
		ln++;
		if (ln % 500 == 0 && !stout) {
//			fprintf(stderr,"\r%u ",ln);
		}
		if (uttline[0] == '%' || uttline[0] == '#')
			continue;
		for (wi=0; isSpace(uttline[wi]); wi++) ;
		if (wi > 0)
			strcpy(uttline, uttline+wi);
		for (wi=0; !isSpace(uttline[wi]) && uttline[wi] != '\n' && uttline[wi] != EOS; wi++) ;
		uttline[wi] = EOS;
		strcpy(w, uttline);
		if (w[0] == EOS)
			continue;
/*
		printf("w=%s", w);
		if (w[strlen(w)-1] != '\n')
			putchar('\n');
*/
		*tutt = EOS;
//		if (isdigit(w[0]) && (w[0] != '0' || isdigit(w[1]) || ((w[1] == '.' || w[1] == ',') && isdigit(w[2])))) {
//			strcat(tutt, "num|number ");
//		} else {
			isTempItem = preprocess_word(w, &isZeroWord);
			if (w[0] != EOS) {
				result_index = 0;
				eparse[0] = EOS;
				etrans[0] = EOS;
				ecomp[0]  = EOS;
				analyze_word(w,empty_cat,eparse,etrans,ecomp,w,startrules_list,0,doDefComp);
				printword(w,result_list,result_index,tutt,isTempItem,isZeroWord);
			}
			if (isTempItem) {
				if (isTempItem != 3)
					delete_word(w_temp,tmp_feats_comp);
				isTempItem = 0;
			}
//		}
		fprintf(fpout, "%s: %s\n", w, tutt);
	}
//	if (!stout)	fprintf(stderr,"\n");
}

/* 2007-08-14
static void AddPostCodes(char *line, char *utt) {
	register long i = 0L, j;

	if (*utterance->speaker != '*')
		return;

	j = strlen(utt);
	while (line[i]) {
		if (line[i] == '[' && line[i+1] == '+' && line[i+2] == ' ') {
			utt[j++] = ' ';
			while (line[i] != EOS && line[i] != ']')
				utt[j++] = line[i++];
			if (line[i] == ']')
				utt[j++] = line[i++];
		} else
			i++;
	}
	utt[j] = EOS;
}
*/

static void getOrgWord(int pos, char *str) {
	int i;

	for (i=0; utterance->line[pos] != EOS && !uS.isskip(utterance->line,pos,&dFnt,MBF); pos++) {
		str[i++] = utterance->line[pos];
	}
	str[i] = EOS;
}

static void call_mor() {
	unsigned int ln = 0, spCnt;
//	int  last_wi;
	int  wi, wpos, offset;
	int  postRes;
	char isPostCodeExclude;
	char tword[MAX_WORD];
	BOOL isMorOutput;
	char isTempItem = 0, isZeroWord;

	if (Xoption_fname[0] == EOS)
		strcpy(Xoption_fname, oldfname);

	spCnt = 0;
	isMorOutput = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		ln++;
		if (ln % 1000 == 0 && !stout) {
// lxs 2019-03-15 if (!isRecursive)
				fprintf(stderr,"\r%u ",ln);
		}
/*
		printf("sp=%s; uttline=%s", utterance->speaker, uttline);
		if (uttline[strlen(uttline)-1] != '\n')
			putchar('\n');
*/
		if (*utterance->speaker == '*') {
			spCnt++;
			isMorOutput = FALSE;
			if (isLookAtDep)
				checktier(utterance->speaker);
		}
		if (chatmode == 0 || (((*utterance->speaker == '*' && !isLookAtDep) || (*utterance->speaker == '%' && isLookAtDep)) && checktier(utterance->speaker))) {
			if (!CHECK_LEXICON)
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			*tutt = EOS;
			wi = 0;
			/* printf("%s%s",utterance->speaker,utterance->line); */
			postRes = isPostCodeFound(utterance->speaker, utterance->line);
			isPostCodeExclude = (postRes == 5 || postRes == 1);
			while (isSpace(utterance->line[wi]))
				wi++;
//			last_wi = wi;
			while ((wi=getword(utterance->speaker, uttline, w, &wpos, wi))) {
//				if (isdigit(w[0]) && (w[0] != '0' || isdigit(w[1]) || ((w[1] == '.' || w[1] == ',') && isdigit(w[2])))) {
//					strcat(tutt, "num|number ");
//					continue;
//				}
				while (uttline[wi] != EOS && uS.isskip(uttline,wi,&dFnt,MBF) && !uS.isRightChar(uttline,wi,'[',&dFnt,MBF))
					 wi++;
				if (uS.isRightChar(w,0,'[',&dFnt,MBF))
					continue;
				if (w[0] == '+') {
					if (isMorExcludePlus(w))
						continue;
				}
				/* special for dss - break words in two */
				if ((strcmp(w,".") == 0) || (strcmp(w,"?") == 0) || (strcmp(w,"!") == 0) || (*w == '+') || (*w == '-')) {
					strcat(tutt,w);
				} else {
					isTempItem = preprocess_word(w, &isZeroWord);
					if (w[0] == EOS)
						continue;
					result_index = 0;
					eparse[0] = EOS;
					etrans[0] = EOS;
					ecomp[0]  = EOS;
					analyze_word(w,empty_cat,eparse,etrans,ecomp,w,startrules_list,0,doDefComp);
					printword(w,result_list,result_index,tutt,isTempItem,isZeroWord);
					if (CHECK_LEXICON || isRunPost) {
						if ((result_index == 0) && (w[0] != EOS)) {
							if (isZeroWord) {
								offset = 1;
								strcpy(templineC4, "0");
							} else
								offset = 0;
							if (isRunPost) {
								sprintf(templineC4+offset, "%s {[scat \?]}~File \"%s\": speaker %d", w, oldfname, spCnt);
							} else if (doLocations) {
								sprintf(templineC4+offset, "%s {[scat \?]}~File \"%s\": line %ld", w, oldfname, lineno);
							} else {
								getOrgWord(wpos, tword);
								if (uS.mStricmp(w, tword) != 0)
									sprintf(templineC4+offset, "%s {[scat \?]}~%s", w, tword);
								else
									sprintf(templineC4+offset, "%s {[scat \?]}", w);
							}
							rootXopt = insert(rootXopt, templineC4);
						}
					}
				}
				if (!CHECK_LEXICON)
					strcat(tutt, " ");
/*
				if (!isPostCodeExclude) {
					for (; last_wi <= wi; last_wi++) {
						if (utterance->line[last_wi] == ',' && utterance->line[last_wi+1] == ',') {
							int spc;
							spc = strlen(tutt);
							tutt[spc++] = 0xe2;
							tutt[spc++] = 0x80;
							tutt[spc++] = 0x9E;
							tutt[spc++] = ' ';
							tutt[spc] = EOS;
						}
					}
				}
				last_wi = wi;
*/
				if (isTempItem) {
					if (isTempItem != 3)
						delete_word(w_temp,tmp_feats_comp);
					isTempItem = 0;
				}
			}
			if (!CHECK_LEXICON) {
				if (!justBlank(tutt)) {
//2007-08-14		AddPostCodes(utterance->line, tutt);

					if (prepostRules != NULL)
						mor_MatchedAndReplaced(tutt);

					printout("%mor:", tutt, NULL, NULL, TRUE);
					isMorOutput = TRUE;
				} else if (!isPostCodeExclude)
					isMorOutput = TRUE;
			}
		} else if (!CHECK_LEXICON) {
			if (!uS.partcmp(utterance->speaker, "%mor:", FALSE, FALSE) || (!isMorOutput && !isLookAtDep))
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
	if (!stout) {
// lxs 2019-03-15 if (!isRecursive)
		fprintf(stderr,"\r                                     \r");
	}
}

#ifndef MOR_LIB
char morXiExec(void) {
	return(call_test_mode(FALSE));
}

void call() {
	char tcat[256];

#ifdef _MAC_CODE
	*debugName = EOS;
	if (!isRefEQZero(mor_lib_dir)) {		/* we have a lib */
		strcpy(debugName, mor_lib_dir);
	}
#elif defined(_WIN32)
	strcpy(debugName, mor_lib_dir);
#endif
	addFilename2Path(debugName, "debug.cdc");
	if (DEBUG_CLAN) {
		if (DEBUG_CLAN & 128)
			;
		else if ((debug_fp=fopen(debugName,"w")) == NULL) {
			fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", debugName);
			debug_fp = stdout;
		} else {
#ifdef _MAC_CODE
			settyp(debugName, 'TEXT', the_file_creator.out, FALSE);
#endif
			printf("*** File \"%s\"\n", debugName);
			fprintf(debug_fp,"%s\n", UTF8HEADER);
			
		}
	}

	/* set up temp fs for "[err +]" */
	strcpy(tcat, "[err +]");
	get_feats(tcat,err_f_comp);

	if (debug_fp != stdout) {
		fclose(debug_fp);
		debug_fp = stdout;
	}
	if (DEBUGWORD) {
	} else if (TEST) {
		if (isCompoundsTest) {
			rootXTest = outputXTests("compounds.xd.cex", "conflicting compound words", rootXTest);
		} else if (isAmbiguousTest)
			rootXTest = outputXTests("ambiguous.xd.cex", "ambiguous definitions of words", rootXTest);
		else
			call_test_mode(TRUE);
	} else if (isLookAtLexFiles) {
		call_analyze_lex();
	} else
		call_mor();

}
#endif // MOR_LIB

void hello_arrays() {
	extern FEAT_PTR *feat_codes;
	extern STRING **match_stack;
	extern SEG_REC *word_segments;
	extern RESULT_REC *result_list;

	e = (char *) malloc((size_t) MAXENTRY);
	if (e == NULL) out_of_mem();
	w = (char *) malloc((size_t) MAX_WORD);
	if (w == NULL) { free(e); out_of_mem(); }
	c = (char *) malloc((size_t) MAX_WORD);
	if (c == NULL) { free(e); free(w); out_of_mem(); }
	tutt = (char *) malloc((size_t) UTTLINELEN);
	if (tutt == NULL) { free(e); free(w); free(c); out_of_mem(); }
	
	feat_codes = (FEAT_PTR *) malloc((size_t) (MAX_FEATS * (size_t) sizeof(FEAT_PTR)));
	if (feat_codes == NULL) {
		free(e); free(w); free(c); free(tutt); out_of_mem();
	}
	match_stack = (STRING **) malloc((size_t) (MAX_MATCHES * (size_t) sizeof(STRING *)));
	if (match_stack == NULL) {
		free(e); free(w); free(c); free(tutt); free(feat_codes);
		out_of_mem();
	}
	word_segments = (SEG_REC *) malloc((size_t) (MAX_SEGS * (size_t) sizeof(SEG_REC)));
	if (word_segments == NULL) {
		free(e); free(w); free(c); free(tutt); free(feat_codes);
		free(match_stack); out_of_mem();
	}
	var_tab = (VAR_REC *) malloc((size_t) (MAX_NUMVAR * (size_t) sizeof(VAR_REC)));
	if (var_tab == NULL) {
		free(e); free(w); free(c); free(tutt); free(feat_codes);
		free(match_stack); free(word_segments); out_of_mem();
	}
	result_list = (RESULT_REC *) malloc((size_t) (MAX_PARSES * (size_t) sizeof(RESULT_REC)));
	if (result_list == NULL) {
		free(e); free(w); free(c); free(tutt); free(feat_codes);
		free(match_stack); free(word_segments); free(var_tab);
		out_of_mem();
	}			
}

void bye_arrays()
{
	extern FEAT_PTR *feat_codes;
	extern STRING **match_stack;
	extern SEG_REC *word_segments;
	extern RESULT_REC *result_list;

	if(e) { free(e);e = NULL;}
	if(w) { free(w);w = NULL;}
	if(c) { free(c);c = NULL;}
	if(tutt) { free(tutt);tutt = NULL;}

	if(feat_codes) { free(feat_codes);feat_codes = NULL;}
	if(match_stack) { free(match_stack);match_stack = NULL;}
	if(word_segments) { free(word_segments);word_segments = NULL;}
	if(var_tab) { free(var_tab);var_tab = NULL;}
	if(result_list) { free(result_list);result_list = NULL;}
}
