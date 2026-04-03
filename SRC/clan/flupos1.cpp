/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main flupos_main
#define call flupos_call
#define getflag flupos_getflag
#define init flupos_init
#define usage flupos_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define NUMGEMITEMS 10

extern char GExt[];
extern char OverWriteFile;
extern char outputOnlyData;
extern char isLanguageExplicit;
extern char linkMain2Mor;
extern char PostCodeMode;
extern char R7Colon;
extern char R7Caret;
extern void (*fatal_error_function)(char);
extern struct tier *defheadtier;
extern struct tier *headtier;

// build utt line beg
enum {
	ISWORD = 0,
	ISREPEAT,
	ISRETRACE,
	ISOTHER
} ;

struct items {
	char *item;
	char code;
	char isPhrase;
	struct items *nextItem;
} ;

struct utt_elems {
	struct items *item;
	struct utt_elems *nextElem;
} ;
// build utt line end

enum {
	UNK = 0,
	ENG,
	FRA,
	ZHO
} ;

// isPOSMatch beg
struct flp_line {
	char isUsed;
	char *loc;
	char *tier;
	flp_line *nextLine;
} ;

struct flp_where {
	unsigned int count;
	struct flp_line *line;
	struct flp_where *nextMatch;
} ;
// isPOSMatch end

struct wds_tnode {
	char *word;
	struct flp_where *where; // isPOSMatch
	unsigned int count;
	float prolongation;	// :
	float broken_word;	// ^
	float block;		// ≠
	float PWR;			// ↫ ↫ pairs//Part Word Repetition; Sequences
	float WWR;			// WWR: how many times one word "the [/] the [/] the" occurs. It is 2
	float mWWR;			// mono-WWR: how many times monosyllable "the [/] the [/] the" occurs. It is 2
	float openClassD; 	// Content
	float closedClassD; // Function
	struct wds_tnode *left;
	struct wds_tnode *right;
};

struct cWds {
	struct wds_tnode *wc;
};

struct flp_sp {
	char isSpeakerFound;
	char isMORFound;

	char isSyllableLang;
	char *fname;
	char *sp;
	char *ID;

	float morWords;
	float morSyllables;

	struct wds_tnode *words;
// isPOSMatch beg
	long total;				/* total number of words */
	long different;			/* number of different words */
// isPOSMatch end

	struct flp_sp *next_sp;
} ;

struct syll_tnode {
	char *word;
	int count;
	struct syll_tnode *left;
	struct syll_tnode *right;
};

static int  flp_SpecWords, DBGemNum;
static char *DBGems[NUMGEMITEMS];
static char flp_ftime, isSyllList, isPOSMatch, sampleType, isMorTierFirst;
static char flp_BBS[5], flp_CBS[5], flp_group, flp_n_option, isNOptionSet, GemMode, specialOptionUsed, langType, langPrecode;
static long sampleSize;
static struct flp_sp *sp_head;
static flp_line *rootLines;
static syll_tnode *rootSyll;
static FILE *SyllListFP;


void usage() {
	puts("FLUPOS creates a spreedsheet with a SLD word's position and fluency types.");
/*
#ifdef UNX
	printf("FluPos REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cFluPos REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
*/
	printf("Usage: flupos [%s] filename(s)\n",mainflgs());
	puts("+c5: when combined with +p option it will reverse tier's priority");
	puts("+dN: specify sample size of N (s, w), +d100s means first 100 syllables");
	puts("+e1: create file with words and their syllable count");
	puts("+pS: search for word S and match it to corresponding POS");
	puts("+g : Gem tier should contain all words specified by +gS");
	puts("+gS: select gems which are labeled by label S");
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	puts("+p@F: search for words in file F and match it to corresponding POS");
	mainusage(FALSE);
/*
#ifdef UNX
	printf("FluPos REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cFluPos REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
*/
	cutt_exit(0);
}

// isPOSMatch beg
static flp_line *flucal_freeLines(flp_line *lines, char isJustOne) {
	flp_line *t;

	while (lines != NULL) {
		if (lines->loc != NULL)
			free(lines->loc);
		if (lines->tier != NULL)
			free(lines->tier);
		t = lines;
		lines = lines->nextLine;
		free(t);
		if (isJustOne)
			break;
	}
	return(lines);
}

static flp_where *flucal_freeWhere(flp_where *where) {
	flp_where *t;

	while (where != NULL) {
		t = where;
		where = where->nextMatch;
		free(t);
	}
	return(NULL);
}

static void flp_freqFreetree(struct wds_tnode *p) {
	if (p != NULL) {
		flp_freqFreetree(p->left);
		flp_freqFreetree(p->right);
		free(p->word);
		p->where = flucal_freeWhere(p->where);
		free(p);
	}
}
// isPOSMatch end

static struct flp_sp *freespeakers(struct flp_sp *p) {
	struct flp_sp *ts;

	while (p) {
		ts = p;
		p = p->next_sp;
		if (ts->fname != NULL)
			free(ts->fname);
		if (ts->sp != NULL)
			free(ts->sp);
		if (ts->ID != NULL)
			free(ts->ID);
		flp_freqFreetree(ts->words);
		free(ts);
	}
	return(NULL);
}

static void syll_freetree(struct syll_tnode *p) {
	if (p != NULL) {
		syll_freetree(p->left);
		syll_freetree(p->right);
		free(p->word);
		free(p);
	}
}

static void flp_error(char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	sp_head = freespeakers(sp_head);
	if (SyllListFP != NULL)
		fclose(SyllListFP);
	syll_freetree(rootSyll);
	rootSyll = NULL;
	rootLines = flucal_freeLines(rootLines, FALSE);
	cutt_exit(0);
}

#define BUILD_UTT_LINE_BEG
// BUILD UTT LINE BEG
static struct items *freeUttItems(struct items *p) {
	struct items *ts;

	while (p) {
		ts = p;
		p = p->nextItem;
		if (ts->item != NULL)
			free(ts->item);
		free(ts);
	}
	return(NULL);
}

static struct utt_elems *freeUttElems(struct utt_elems *p) {
	struct utt_elems *ts;

	while (p) {
		ts = p;
		p = p->nextElem;
		ts->item = freeUttItems(ts->item);
		free(ts);
	}
	return(NULL);
}

static struct items *AddItem(struct items *root_items, char *item, char code) {
	struct items *p;

	if (root_items == NULL) {
		if ((p=NEW(struct items)) == NULL)
			flp_error(TRUE);
		root_items = p;
	} else {
		for (p=root_items; p->nextItem != NULL; p=p->nextItem) ;
		if ((p->nextItem=NEW(struct items)) == NULL)
			flp_error(TRUE);
		p = p->nextItem;
	}
	p->nextItem = NULL;
	p->code = code;
	p->isPhrase = 0;
	if ((p->item=(char *)malloc(strlen(item)+1)) == NULL)
		flp_error(TRUE);
	strcpy(p->item, item);
	return(root_items);
}

static struct utt_elems *AddToUttElems(struct utt_elems *root_elem, char *item, char code) {
	struct utt_elems *p;

	if (root_elem == NULL) {
		if ((p=NEW(struct utt_elems)) == NULL)
			flp_error(TRUE);
		root_elem = p;
	} else {
		for (p=root_elem; p->nextElem != NULL; p=p->nextElem) ;
		if (p->item != NULL && code != ISWORD) {
			p->item = AddItem(p->item, item, code);
			return(root_elem);
		}
		if ((p->nextElem=NEW(struct utt_elems)) == NULL)
			flp_error(TRUE);
		p = p->nextElem;
	}
	p->nextElem = NULL;
	p->item = AddItem(p->item, item, code);
	return(root_elem);
}

static int getitem(const char *sp, register char *line, register char *orgWord, int i) {
	int  si, temp;
	char sq;
	char *word;

	word = orgWord;
	while ((*word=line[i]) != EOS && isSpace(line[i])) {
		i++;
	}
	if (*word == EOS)
		return(0);
	si = i;

	if (uS.isRightChar(line, i, '[',&dFnt,MBF)) {
		if (uS.isSqBracketItem(line, i+1, &dFnt, MBF))
			sq = TRUE;
		else
			sq = FALSE;
	} else
		sq = FALSE;

getword_rep:

	while ((*++word=line[++i]) != EOS && (!uS.isskip(line,i,&dFnt,MBF) || line[i] == '>' || sq)) {
		if (i-si >= BUFSIZ-1) {
			fprintf(fpout,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(fpout,"ERROR: One word on the tier below has exceded 1024 characters in lenght\n");
			fprintf(fpout,"%s\n", line);
			cutt_exit(0);
		}
		if (uS.isRightChar(line, i, ']', &dFnt, MBF)) {
			if (i-si >= BUFSIZ-1) {
				fprintf(fpout,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(fpout,"ERROR: One word on the tier below has exceded 1024 characters in lenght\n");
				fprintf(fpout,"%s\n", line);
				cutt_exit(0);
			}
			*++word = EOS;
			if (*orgWord == '[')
				uS.cleanUpCodes(orgWord, &dFnt, MBF);
			return(i+1);
		} else if (uS.isRightChar(line, i, '(', &dFnt, MBF) && uS.isPause(line, i, NULL,  &temp)) {
			*word = EOS;
			return(i);
		} else if (uS.isRightChar(line, i, '.', &dFnt, MBF) && uS.isPause(line, i, NULL, NULL)) {
			while ((*++word=line[++i]) != EOS && !uS.isRightChar(line, i, ')', &dFnt, MBF)) {
				if (i-si >= BUFSIZ-1) {
					fprintf(fpout,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(fpout,"ERROR: One word on the tier below has exceded 1024 characters in lenght\n");
					fprintf(fpout,"%s\n", line);
					cutt_exit(0);
				}
			}
			if (i-si >= BUFSIZ-1) {
				fprintf(fpout,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(fpout,"ERROR: One word on the tier below has exceded 1024 characters in lenght\n");
				fprintf(fpout,"%s\n", line);
				cutt_exit(0);
			}
			*++word = EOS;
			return(i+1);
		} else if (uS.IsUtteranceDel(line, i) == 2) {
			*word = EOS;
			return(i);
		} else if (line[i] == ',' && !isdigit(line[i+1])) {
			*word = EOS;
			return(i);
		}
	}

	if (uS.isRightChar(line, i, '[', &dFnt, MBF)) {
		if (!uS.isSqBracketItem(line, i+1, &dFnt, MBF))
			goto getword_rep;
		else
			i--;
	}
	*word = EOS;

	if (line[i] != EOS)
		i++;
	return(i);
}

static void process_utt(struct utt_elems *root_elem) {
	struct utt_elems *pe;
	struct items *pi;

	if (root_elem != NULL) {
		for (pe=root_elem; pe != NULL; pe=pe->nextElem) {
			for (pi=pe->item; pi != NULL; pi=pi->nextItem) {
				fprintf(stderr, "%s(%d,%d) ", pi->item, pi->code, pi->isPhrase);
			}
		}
		fprintf(stderr, "\n");
	}
}

// build utt line end
#define BUILD_UTT_LINE_END


// isPOSMatch beg
static flp_line *flp_AddNewLine(flp_line *lines, char *loc, char *tier) {
	flp_line *p;

	if ((p=NEW(struct flp_line)) == NULL)
		flp_error(TRUE);
	if (loc != NULL) {
		if ((p->loc=(char *)malloc(strlen(loc)+1)) == NULL)
			flp_error(TRUE);
		strcpy(p->loc, loc);
	} else
		p->loc = NULL;
	if ((p->tier=(char *)malloc(strlen(tier)+1)) == NULL)
		flp_error(TRUE);
	strcpy(p->tier, tier);
	p->nextLine = lines;
	p->isUsed = FALSE;
	lines = p;

	return(lines);
}

static void addDBGems(char *gem) {
	if (DBGemNum >= NUMGEMITEMS) {
		fprintf(stderr, "\nERROR: Too many keywords specified. The limit is %d\n", NUMGEMITEMS);
		flp_error(FALSE);
	}
	DBGems[DBGemNum] = gem;
	DBGemNum++;
}

static int excludeGemKeywords(char *word) {
	int i;

	if (word[0] == '+' || strcmp(word, "!") == 0 || strcmp(word, "?") == 0 || strcmp(word, ".") == 0)
		return(FALSE);
	for (i=0; i < DBGemNum; i++) {
		if (uS.mStricmp(DBGems[i], word) == 0)
			return(TRUE);
	}
	return(FALSE);
}

static char *flp_strsave(char *s) {
	char *p;

	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		flp_error(TRUE);
	return(p);
}

static struct wds_tnode *flp_talloc(char *word) {
	struct wds_tnode *p;

	if ((p=NEW(struct wds_tnode)) == NULL)
		flp_error(TRUE);
	p->word = word;
	p->count = 0;
	p->prolongation	= 0.0;
	p->broken_word	= 0.0;
	p->block		= 0.0;
	p->PWR			= 0.0;
	p->WWR			= 0.0;
	p->mWWR			= 0.0;
	p->openClassD	= 0.0;
	p->closedClassD	= 0.0;
	return(p);
}

static flp_where *flp_AddNewMatch(flp_where *where, flp_line *line) {
	flp_where *p;

	if (where == NULL) {
		if ((p=NEW(struct flp_where)) == NULL)
			flp_error(TRUE);
		where = p;
	} else {
		for (p=where; p->nextMatch != NULL; p=p->nextMatch) {
			if (line->loc == NULL) {
				if (!strcmp(p->line->tier, line->tier)) {
					p->count++;
					return(where);
				}
			}
		}
		if (line->loc == NULL) {
			if (!strcmp(p->line->tier, line->tier)) {
				p->count++;
				return(where);
			}
		}
		if ((p->nextMatch=NEW(struct flp_where)) == NULL)
			flp_error(TRUE);
		p = p->nextMatch;
	}
	p->nextMatch = NULL;
	p->count = 1;
	p->line = line;
	line->isUsed = TRUE;
	return(where);
}

static void flp_addlineno(struct wds_tnode *p, struct cWds *tw, flp_line *line) {
	p->count++;
	if (tw != NULL)
		tw->wc = p;
	if (line != NULL) {
		p->where = flp_AddNewMatch(p->where, line);
	}
}

static struct wds_tnode *flp_freqTree(struct wds_tnode *p, flp_sp *ts, char *w, struct cWds *tw, flp_line *line) {
	int cond;

	if (p == NULL) {
		ts->different++;
		p = flp_talloc(flp_strsave(w));
		p->where = NULL;
		flp_addlineno(p, tw, line);
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->word)) == 0) {
		flp_addlineno(p, tw, line);
	} else if (cond < 0) {
		p->left = flp_freqTree(p->left, ts, w, tw, line);
	} else {
		p->right = flp_freqTree(p->right, ts, w, tw, line); // if cond > 0
	}
	return(p);
}
// isPOSMatch end

static void flp_initTSVars(struct flp_sp *ts) {
	ts->isMORFound = FALSE;
	
	if (langType == UNK)
		ts->isSyllableLang = FALSE;
	else
		ts->isSyllableLang = TRUE;
	// isPOSMatch beg
	ts->words = NULL;
	ts->total = 0L;		/* total number of words */
	ts->different = 0L;	/* number of different words */
	// isPOSMatch end

	ts->morWords  = 0.0;
	ts->morSyllables= 0.0;
}

static struct flp_sp *flp_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound) {
	struct flp_sp *ts, *tsp;

	uS.remblanks(sp);
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->fname, fname) == 0) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
				ts->isSpeakerFound = isSpeakerFound;
				return(ts);
			}
		}
	}
	if ((ts=NEW(struct flp_sp)) == NULL)
		flp_error(TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		free(ts);
		flp_error(TRUE);
	}
	if (sp_head == NULL) {
		sp_head = ts;
	} else {
		for (tsp=sp_head; tsp->next_sp != NULL; tsp=tsp->next_sp) ;
		tsp->next_sp = ts;
	}
	ts->next_sp = NULL;
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		flp_error(TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			flp_error(TRUE);
		strcpy(ts->ID, ID);
	}
	ts->isSpeakerFound = isSpeakerFound;
	flp_initTSVars(ts);
	return(ts);
}

void init(char f) {
	int i;
	FNType debugfile[FNSize];
	struct tier *nt;
	IEWORDS *twd;

	if (f) {
		sp_head = NULL;
		rootLines = NULL;
		flp_ftime = TRUE;
		fatal_error_function = flp_error;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		mor_initwords();
		isSyllList = FALSE;
		SyllListFP = NULL;
		rootSyll = NULL;
		isPOSMatch = FALSE;
		sampleSize = 0L;
		sampleType = 0;
		isMorTierFirst = TRUE;
		specialOptionUsed = FALSE;
		langPrecode = FALSE;
		GemMode = '\0';
		flp_SpecWords = 0;
		flp_group = FALSE;
		flp_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(flp_BBS, "@*&#");
		strcpy(flp_CBS, "@*&#");
		DBGemNum = 0;
	} else {
		if (flp_ftime) {
			flp_ftime = FALSE;
			if (chatmode) {
				if (GemMode != '\0' || isNOptionSet == TRUE) {
				} else {
					maketierchoice("@Languages:",'+',FALSE);
					maketierchoice("@ID:",'+',FALSE);
					maketierchoice("%mor:",'+',FALSE);
				}
			} else {
				fprintf(stderr, "FluPos can only run on CHAT data files\n\n");
				cutt_exit(0);
			}
			i = 0;
			for (nt=headtier; nt != NULL; nt=nt->nexttier) {
				if (nt->tcode[0] == '*') {
					i++;
				}
			}
			if (i != 1) {
				fprintf(stderr, "\nPlease specify only one speaker tier code with \"+t\" option.\n");
				cutt_exit(0);
			}
			if (flp_group && GemMode == '\0') {
				fprintf(stderr, "\nThe \"+g\" option has to used with \"+gS\" option.\n");
				cutt_exit(0);
			}
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '!' ||
					GlobalPunctuation[i] == '?' ||
					GlobalPunctuation[i] == '.') 
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}

			if (!isPOSMatch) {
				if (!isMorTierFirst) {
					fprintf(stderr,"The +c5 option can only be used with +p option\n");
					cutt_exit(0);
				}
				if (!f_override)
					stout = FALSE;
				AddCEXExtension = ".xls";
				combinput = TRUE;
				linkMain2Mor = TRUE;
				outputOnlyData = TRUE;
				OverWriteFile = TRUE;
				isCreateFakeMor = 2;
				R7Colon = FALSE;
				R7Caret = FALSE;
			} else {
				if (sampleType != 0) {
					fprintf(stderr,"The +d option can not be used with +p option\n");
					cutt_exit(0);
				}
				for (twd=wdptr; twd != NULL; twd=twd->nextword) {
					if (twd->word[0] == '&') {
						break;
					}
				}
				addword('\0','\0',"+0*");
				if (twd == NULL) {
					addword('\0','\0',"+&*");
					addword('\0','\0',"+(*.*)");
				}
				addword('\0','\0',"++*");
				addword('\0','\0',"+-*");
				addword('\0','\0',"+#*");
				FilterTier = 1;
				isCreateFakeMor = 1;
				linkMain2Mor = TRUE;
			}
#if !defined(CLAN_SRV)
			if (isSyllList) {
				strcpy(debugfile, "word_syllables.cex");
				SyllListFP = fopen(debugfile, "w");
				if (SyllListFP == NULL) {
					fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", debugfile);
				}
#ifdef _MAC_CODE
				else
					settyp(debugfile, 'TEXT', the_file_creator.out, FALSE);
#endif
			}
#endif // !defined(CLAN_SRV)
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	char wd[1024+2];

	f++;
	switch(*f++) {
		case 'c':
			if (*f == '5')
				isMorTierFirst = FALSE;
			else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'd':
			sampleSize = atol(f);
			for (f1=f; isdigit(*f1); f1++);
			sampleType = tolower(*f1);
			if (*f1 == EOS || sampleSize <= 0L || (sampleType != 's' && sampleType != 'w')) {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				fprintf(stderr,"+dN: specify sample size of N (s, w), +d100s means first 100 syllables\n");
				cutt_exit(0);
			}
			break;
		case 'e':
			if (*f == '1') {
				isSyllList = TRUE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'g':
			if (*f == EOS) {
				flp_group = TRUE;
				specialOptionUsed = TRUE;
			} else {
				GemMode = 'i';
				flp_SpecWords++;
				addDBGems(getfarg(f,f1,i));
			}
			break;
		case 'n':
			if (*(f-2) == '+') {
				flp_n_option = TRUE;
				strcpy(flp_BBS, "@G:");
				strcpy(flp_CBS, "@*&#");
				specialOptionUsed = TRUE;
			} else {
				strcpy(flp_BBS, "@BG:");
				strcpy(flp_CBS, "@EG:");
			}
			isNOptionSet = TRUE;
			no_arg_option(f);
			break;
		case 'p':
//			if (*(f-2) == '+') {
				if (*f) {
					strncpy(wd, f, 1024);
					wd[1024] = EOS;
					removeExtraSpace(wd);
					uS.remFrontAndBackBlanks(wd);
					if (wd[0] == '+' || wd[0] == '~') {
						if (wd[1] == '@') {
							wd[1] = wd[0];
							wd[0] = *(f+1);
						}
					}
					if (wd[0] == '@') {
						rdexclf('s','i',wd+1);
					} else {
						if (wd[0] == '\\' && wd[1] == '@')
							strcpy(wd, wd+1);
						if ((wd[0] == '+' || wd[0] == '~') && wd[1] == '\\' && wd[2] == '@')
							strcpy(wd+1, wd+2);
						addword('s','i',wd);
					}
					isPOSMatch = TRUE;
				} else {
					fprintf(stderr,"Missing argument to option: %s\n", f-2);
					cutt_exit(0);
				}
//			}
			break;
		case 's':
			specialOptionUsed = TRUE;
			if (*f == '[' && *(f+1) == '-') {
				langPrecode = TRUE;
				if (*(f-2) == '+')
					isLanguageExplicit = 2;
				maingetflag(f-2,f1,i);
			} else if ((*f == '[' && *(f+1) == '+') || ((*f == '+' || *f == '~') && *(f+1) == '[' && *(f+2) == '+')) {
				maingetflag(f-2,f1,i);
			} else {
				fprintf(stderr, "Please specify only postcodes, \"[+ ...]\", or precodes \"[- ...]\" with +/-s option.\n");
				cutt_exit(0);
			}
			break;
		case 't':
			if (*(f-2) == '+' && *f == '*') {
				maingetflag(f-2,f1,i);
			} else {
				fprintf(stderr, "\nPlease specify only one speaker tier code with \"+t\" option.\n");
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

// isPOSMatch beg
static void word_printline(struct wds_tnode *p) {
	flp_where *w;

	fprintf(fpout,"\n");
	for (w=p->where; w != NULL; w=w->nextMatch) {
		if (w->line->loc != NULL) {
			if (w->line->loc[0] != EOS)
				fprintf(fpout,"        %s\n", w->line->loc);
			fprintf(fpout,"      %s\n", w->line->tier);
		} else
			fprintf(fpout,"    %3u %s\n", w->count, w->line->tier);
	}
}
// isPOSMatch end

static void wordsTreeprint(struct wds_tnode *p) {
	if (p != NULL) {
		wordsTreeprint(p->left);
		do {
			if (isPOSMatch) {
				fprintf(fpout,"%3u ",p->count);
				fprintf(fpout,"%-10s", p->word);
				word_printline(p);
			} else {
				excelNumCell(fpout, "%.3f", p->prolongation);
				excelNumCell(fpout, "%.3f", p->broken_word);
				excelNumCell(fpout, "%.3f", p->block);
				excelNumCell(fpout, "%.3f", p->PWR);
				excelNumCell(fpout, "%.3f", p->WWR);
				excelNumCell(fpout, "%.3f", p->mWWR);
				excelNumCell(fpout, "%.3f", p->openClassD);
				excelNumCell(fpout, "%.3f", p->closedClassD);
			}
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				wordsTreeprint(p->right);
				break;
			}
			p = p->right;
		} while (1);
	}
}

static void flp_pr_result(void) {
	char  *sFName;
	struct flp_sp *ts;

	if (sp_head == NULL) {
		if (flp_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n");
			fprintf(stderr,"OR No specified gems found for this speaker\n\n");
		} else
			fprintf(stderr, "\nERROR: No speaker matching +t option found\n\n");
	}
	if (isPOSMatch) {
		for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
			if (!ts->isSpeakerFound) {
				fprintf(stderr, "\nWARNING: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
				continue;
			}
			fprintf(fpout, "Speaker: %s\n", ts->sp);
			wordsTreeprint(ts->words);
			fprintf(fpout,"------------------------------\n");
			fprintf(fpout,"%5ld  Total number of different item types used\n", ts->different);
			fprintf(fpout,"%5ld  Total number of items (tokens)\n", ts->total);
			fprintf(fpout, "\n");
		}
		sp_head = freespeakers(sp_head);
	} else {
		excelHeader(fpout, newfname, 105);
		excelRow(fpout, ExcelRowStart);
		excelStrCell(fpout, "File");
		excelCommasStrCell(fpout, "Language,Corpus,Code,Age(Month),Sex,Group,Race,SES,Role,Education,Custom_field");

		excelStrCell(fpout, "mor_Utts");
		excelStrCell(fpout, "mor_words");
		excelStrCell(fpout, "mor_syllables");
		excelStrCell(fpout, "all_words");

		excelStrCell(fpout, "#_Prolongation");
		excelStrCell(fpout, "#_Broken_word");
		excelStrCell(fpout, "#_Block");
		excelStrCell(fpout, "#_PWR");
		excelStrCell(fpout, "#_WWR");
		excelStrCell(fpout, "#_mono-WWR");

		excelStrCell(fpout, "Content");

		excelStrCell(fpout, "Function");

		excelRow(fpout, ExcelRowEnd);

		for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
			if (!ts->isSpeakerFound) {
				if (flp_SpecWords) {
					fprintf(stderr,"\nWARNING: No specified gems found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
				} else
					fprintf(stderr, "\nWARNING: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
				continue;
			}
// ".xls"
			sFName = strrchr(ts->fname, PATHDELIMCHR);
			if (sFName != NULL)
				sFName = sFName + 1;
			else
				sFName = ts->fname;
			excelRow(fpout, ExcelRowStart);
			excelStrCell(fpout, sFName);
			if (ts->ID) {
				excelOutputID(fpout, ts->ID);
			} else {
				excelCommasStrCell(fpout, ".,.");
				excelStrCell(fpout, ts->sp);
				excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
			}
			
			if (!ts->isMORFound) {
				fprintf(stderr, "\n*** File \"%s\": Speaker \"%s\"\n", sFName, ts->sp);
				fprintf(stderr, "WARNING: Speaker \"%s\" has no \"%s\" tiers.\n\n", ts->sp, "%mor:");
				excelCommasStrCell(fpout, "NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA");
			} else {
				wordsTreeprint(ts->words);
			}
			excelRow(fpout, ExcelRowEnd);
		}
		excelFooter(fpout);
		sp_head = freespeakers(sp_head);
	}
}

static char isMor(char *mor, const char *pat) {
	int i, plen;

	plen = strlen(pat);
	for (i=strlen(mor)-plen; i >= 0; i--) {
		if (uS.mStrnicmp(mor+i,pat, plen) == 0)
			return(TRUE);
	}
	return(FALSE);
}

static int countSyllables(char *word, char *mor) {
	int i, j, startIndex, vCnt;
	char isRepSeg, isBullet;

	vCnt = 0;
	if (langType == ENG) {
		char tWord[BUFSIZ+1];

		if (word[0] == '[' || word[0] == '(' || word[0] == EOS)
			return(0);
		isRepSeg = FALSE;
		isBullet = FALSE;
		j = 0;
		for (i=0; word[i] != EOS; i++) {
			if (UTF8_IS_LEAD((unsigned char)word[i]) && word[i] == (char)0xE2) {
				if (word[i+1] == (char)0x86 && word[i+2] == (char)0xAB) {
					isRepSeg = !isRepSeg;
				}
			} else if (word[i] == HIDEN_C) {
				isBullet = !isBullet;
			} else if (word[i] == '@')
				   break;
			if (!isRepSeg && !isBullet && isalnum(word[i]))
				tWord[j++] = word[i];
		}
		tWord[j] = EOS;
		if (tWord[0] == EOS)
			return(0);
	//	if (uS.mStricmp(tWord, "something") == 0)
	//		return(2);
		if (uS.mStricmp(tWord, "maybe") == 0)
			return(2);
		if (uS.mStrnicmp(tWord,"ice", 3) == 0) {
			vCnt = 1;
			startIndex = 3;
		} else if (uS.mStrnicmp(tWord,"some", 4) == 0 || uS.mStrnicmp(tWord,"fire", 4) == 0 ||
				   uS.mStrnicmp(tWord,"bake", 4) == 0 || uS.mStrnicmp(tWord,"base", 4) == 0 ||
				   uS.mStrnicmp(tWord,"bone", 4) == 0 || uS.mStrnicmp(tWord,"cake", 4) == 0 ||
				   uS.mStrnicmp(tWord,"care", 4) == 0 || uS.mStrnicmp(tWord,"dare", 4) == 0 ||
				   uS.mStrnicmp(tWord,"fire", 4) == 0 || uS.mStrnicmp(tWord,"fuse", 4) == 0 ||
				   uS.mStrnicmp(tWord,"game", 4) == 0 || uS.mStrnicmp(tWord,"home", 4) == 0 ||
				   uS.mStrnicmp(tWord,"juke", 4) == 0 || uS.mStrnicmp(tWord,"lake", 4) == 0 ||
				   uS.mStrnicmp(tWord,"life", 4) == 0 || uS.mStrnicmp(tWord,"mine", 4) == 0 ||
				   uS.mStrnicmp(tWord,"mole", 4) == 0 || uS.mStrnicmp(tWord,"name", 4) == 0 ||
				   uS.mStrnicmp(tWord,"nose", 4) == 0 || uS.mStrnicmp(tWord,"note", 4) == 0 ||
				   uS.mStrnicmp(tWord,"race", 4) == 0 || uS.mStrnicmp(tWord,"rice", 4) == 0 ||
				   uS.mStrnicmp(tWord,"side", 4) == 0 || uS.mStrnicmp(tWord,"take", 4) == 0 ||
				   uS.mStrnicmp(tWord,"tape", 4) == 0 || uS.mStrnicmp(tWord,"time", 4) == 0 ||
				   uS.mStrnicmp(tWord,"wine", 4) == 0 || uS.mStrnicmp(tWord,"wipe", 4) == 0) {
			vCnt = 1;
			startIndex = 4;
		} else if (uS.mStrnicmp(tWord,"goose", 5) == 0 || uS.mStrnicmp(tWord,"grade", 5) == 0 ||
				   uS.mStrnicmp(tWord,"grape", 5) == 0 || uS.mStrnicmp(tWord,"horse", 5) == 0 ||
				   uS.mStrnicmp(tWord,"house", 5) == 0 || uS.mStrnicmp(tWord,"phone", 5) == 0 ||
				   uS.mStrnicmp(tWord,"snake", 5) == 0 || uS.mStrnicmp(tWord,"space", 5) == 0 ||
				   uS.mStrnicmp(tWord,"store", 5) == 0 || uS.mStrnicmp(tWord,"stove", 5) == 0 ||
				   uS.mStrnicmp(tWord,"voice", 5) == 0 || uS.mStrnicmp(tWord,"waste", 5) == 0) {
			vCnt = 1;
			startIndex = 5;
		} else if (uS.mStrnicmp(tWord,"cheese", 6) == 0) {
			vCnt = 1;
			startIndex = 6;
		} else if (uS.mStrnicmp(tWord,"police", 6) == 0) {
			vCnt = 2;
			startIndex = 6;
		} else {
			vCnt = 0;
			startIndex = 0;
		}
		for (i=startIndex; tWord[i] != EOS; i++) {
			if ((i == startIndex || !uS.isVowel(tWord+i-1)) && uS.isVowel(tWord+i)) {
				if ((tWord[i] == 'e' || tWord[i] == 'E') && tWord[i+1] == EOS) {
					if (vCnt == 0)
						vCnt++;
					else if (i > 1 && !uS.isVowel(tWord+i-2) && (tWord[i-1] == 'l' || tWord[i-1] == 'L'))
						vCnt++;
				} else
					vCnt++;
			}
		}
#ifndef UNX
	} else if (langType == FRA) {
		unCH tWord[BUFSIZ+1], uWord[BUFSIZ+1];

		u_strcpy(tWord, word, BUFSIZ);
		if (tWord[0] == '[' || tWord[0] == '(' || tWord[0] == EOS)
			return(0);
		isRepSeg = FALSE;
		isBullet = FALSE;
		j = 0;
		for (i=0; tWord[i] != EOS; i++) {
			if (tWord[i] == 0x21ab) {
				isRepSeg = !isRepSeg;
			} else if (tWord[i] == HIDEN_C) {
				isBullet = !isBullet;
			} else if (tWord[i] == '@')
				break;
			if (!isRepSeg && !isBullet && (uS.my_isalpha(tWord+i) == 1 || iswdigit(tWord[i]) || tWord[i] == '\''))
				uWord[j++] = tWord[i];
		}
		uWord[j] = EOS;
		if (uWord[0] == EOS)
			return(0);
		for (i=0; uWord[i] != EOS; i++) {
			if ((i == 0 || !uS.isVowel(uWord+i-1)) && uS.isVowel(uWord+i)) {
				if (uS.mStrnicmp(uWord+i, "ient", 4) == 0 && uWord[i+3] == EOS) {
					vCnt++;
				} else if (uS.mStrnicmp(uWord+i, "ent", 3) == 0 && uWord[i+3] == EOS) {
					if (mor == NULL)
						vCnt++;
					else if (isMor(mor, "&3p") == TRUE)
						i += 2;
					else
						vCnt++;
				} else if (uS.mStrnicmp(uWord+i, "ouer", 4) == 0) {
					vCnt += 2;
					i += 3;
				} else if (uS.mStrnicmp(uWord+i, "ouet", 4) == 0) {
					vCnt += 2;
					i += 3;
				} else if (i > 0 && (uWord[i] == 'u' || uWord[i] == 'U') && (uWord[i-1] == 'q' || uWord[i-1] == 'Q')) {
					uWord[i] = '\002';
				} else if ((uWord[i] == 'o' || uWord[i] == 'O') && (uWord[i+1] == 'o' || uWord[i+1] == 'O')) {
					vCnt += 2;
					i++;
				} else if ((uWord[i] == 'u' || uWord[i] == 'U') && (uWord[i+1] == 'a' || uWord[i+1] == 'A')) {
					vCnt += 2;
					i++;
				} else if ((uWord[i] == 'e' || uWord[i] == 'E') && uWord[i+1] == EOS) {
				} else if (uWord[i] == 'e' || uWord[i] == 'E') {
					if ((uWord[i+1] == 's' || uWord[i+1] == 'S') && uWord[i+2] == EOS) {
					} else if (!uS.isVowel(uWord+i+1)) {
						if ((uWord[i+1] == 'n' || uWord[i+1] == 'N' || uWord[i+1] == 'c' || uWord[i+1] == 'C' ||
							 uWord[i+1] == 't' || uWord[i+1] == 'T' || uWord[i+1] == 'm' || uWord[i+1] == 'M' ||
							 uWord[i+1] == 'r' || uWord[i+1] == 'R' || uWord[i+1] == 'l' || uWord[i+1] == 'L' ||
							 uWord[i+1] == 's' || uWord[i+1] == 'S' ||
							 uWord[i+1] == 'b' || uWord[i+1] == 'B' || uWord[i+1] == 'd' || uWord[i+1] == 'D' ||
							 uWord[i+1] == 'f' || uWord[i+1] == 'F' || uWord[i+1] == 'g' || uWord[i+1] == 'G' ||
							 uWord[i+1] == 'k' || uWord[i+1] == 'K' || uWord[i+1] == 'p' || uWord[i+1] == 'P' ||
							 uWord[i+1] == 'v' || uWord[i+1] == 'V' || uWord[i+1] == 'z' || uWord[i+1] == 'Z') && 
							(i < 2 || !uS.isVowel(uWord+i+2))) {
							vCnt++;
						} else if (uWord[i+1] == 'x' || uWord[i+1] == 'X' || uWord[i+1] == 'q' || uWord[i+1] == 'Q' ||
								   uWord[i+1] == 'j' || uWord[i+1] == 'J' || uWord[i+1] == 'h' || uWord[i+1] == 'H') {
							vCnt++;
						} else if (i > 0 && (uWord[i-1] == '\'' || uWord[i-1] == '\001'))
							vCnt++;
					} else
						vCnt++;
				} else if (i > 0 && (uWord[i] == 0x00E9 || uWord[i] == 0x00C9) && uS.isVowel(uWord+i+1)) {
					if ((uWord[i+1] == 'e' || uWord[i+1] == 'E') && uWord[i+2] == EOS) {
						vCnt++;
						i++;
					} else if ((uWord[i+1] == 'e' || uWord[i+1] == 'E') && 
							   (uWord[i+2] == 's' || uWord[i+2] == 'S') && uWord[i+3] == EOS) {
						vCnt++;
						i += 2;
					} else {
						vCnt += 2;
						i++;
					}
				} else
					vCnt++;
			} else if (i > 0 && (uWord[i] == 0x00E9 || uWord[i] == 0x00C9) && uS.isVowel(uWord+i-1)) {
				if (uWord[i-1] == 'i' || uWord[i-1] == 'I') {
					if (i >= 3 && !uS.isVowel(uWord+i-2) && !uS.isVowel(uWord+i-3) && 
						uWord[i-3] != 'n' && uWord[i-3] != 'N' && uWord[i-3] != 'm' && uWord[i-3] != 'M')
						vCnt++;
				} else
					vCnt++;
			} else if (i > 0 && (uWord[i] == 'y' || uWord[i] == 'Y') && uS.isVowel(uWord+i-1) && uS.isVowel(uWord+i+1)) {
				uWord[i] = '\001';
			}
		}
		if (vCnt == 0)
			vCnt++;
	} else if (langType == ZHO) {
		unCH tWord[BUFSIZ+1], uWord[BUFSIZ+1];
		
		u_strcpy(tWord, word, BUFSIZ);
		if (tWord[0] == '[' || tWord[0] == '(' || tWord[0] == EOS)
			return(0);
		isRepSeg = FALSE;
		isBullet = FALSE;
		j = 0;
		for (i=0; tWord[i] != EOS; i++) {
			if (tWord[i] == 0x21ab) {
				isRepSeg = !isRepSeg;
			} else if (tWord[i] == HIDEN_C) {
				isBullet = !isBullet;
			} else if (tWord[i] == '@')
				break;
			if (!isRepSeg && !isBullet)
				uWord[j++] = tWord[i];
		}
		uWord[j] = EOS;
		if (uWord[0] == EOS)
			return(0);
		vCnt = strlen(uWord);
		if (vCnt == 0)
			vCnt++;
#endif // !UNX
	}
	return(vCnt);
}

static struct syll_tnode *syll_tree(struct syll_tnode *p, char *word, int count) {
	int cond;
	struct syll_tnode *t = p;

	if (p == NULL) {
		if ((p=NEW(struct syll_tnode)) == NULL)
			flp_error(TRUE);
		p->word = (char *)malloc(strlen(word)+1);
		if (p->word == NULL)
			flp_error(TRUE);
		strcpy(p->word, word);
		p->count = count;
		p->left = p->right = NULL;
	} else if ((cond=strcmp(word,p->word)) < 0)
		p->left = syll_tree(p->left, word, count);
	else if (cond > 0){
		for (; (cond=strcmp(word,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond < 0)
			p->left = syll_tree(p->left, word, count);
		else if (cond > 0)
			p->right = syll_tree(p->right, word, count); /* if cond > 0 */
		return(t);
	}
	return(p);
}

static char flp_isUttDel(char *line, int pos) {
	if (line[pos] == '?' && line[pos+1] == '|')
		;
	else if (uS.IsUtteranceDel(line, pos)) {
		if (!uS.atUFound(line, pos, &dFnt, MBF))
			return(TRUE);
	}
	return(FALSE);
}

static char isOnlyOneWordPreCode(char *line, int wi) {
	int  i, wCnt;
	char word[BUFSIZ+1];

	findWholeScope(line, wi, templineC);
	uS.remFrontAndBackBlanks(templineC);
	if (templineC[0] == EOS)
		return(TRUE);
	wCnt = 0;
	i = 0;
	while ((i=getword(utterance->speaker, templineC, word, NULL, i))) {
		if (word[0] != '[')
			wCnt++;
	}
	if (wCnt <= 1)
		return(TRUE);
	else
		return(FALSE);
}

static char isMonoWordPreCode(char *line, int wi) {
	int  i, wCnt, syllCnt;
	char word[BUFSIZ+1];

	findWholeScope(line, wi, templineC);
	uS.remFrontAndBackBlanks(templineC);
	if (templineC[0] == EOS)
		return(TRUE);
	wCnt = 0;
	syllCnt = 0;
	i = 0;
	while ((i=getword("*", templineC, word, NULL, i))) {
		if (word[0] != '[') {
			wCnt++;
			syllCnt += countSyllables(word, NULL); // 2024-05-01 in French language the result might be off by 1
		}
	}
	if (wCnt <= 1) {
		if (syllCnt == 1)
			return(TRUE);
	}
	return(FALSE); 
	// 2024-05-01 in French language the result might be off by 1
}

static char isPreviousItemWWR(char *line, int wi) {

	for (wi--; wi >= 0 && (isSpace(line[wi]) || line[wi] == '\n'); wi--) ;
	if (wi < 3)
		return(FALSE);
	if ((isSpace(line[wi-3]) || line[wi-3] == '\n') && line[wi-2] == '[' && line[wi-1] == '/' && line[wi] == ']')
		return(TRUE);
	return(FALSE);
}

static float roundFloat(double num) {
	long t;

	t = (long)num;
	num = num - t;
	if (num > 0.5)
		t++;
	return(t);
}

static int countMatchedWordSyms(char *word, char *nPat) {
	int i, len, cnt;

	cnt = 0;
	len = strlen(nPat);
	for (i=0; word[i] != EOS; i++) {
		if (strncmp(word+i, nPat, len) == 0)
			cnt++;
	}
	return(cnt);
}

static int countMatchedSyms(char *word, char *pat) {
	char nPat[4];

	if (pat[0] == '*' || pat[0] == '%') {
		if (pat[1] == ':' || pat[1] == '^') {
			if (pat[2] == '*' || pat[2] == '%') {
				nPat[0] = pat[1];
				nPat[1] = EOS;
				return(countMatchedWordSyms(word, nPat));
			}
		} else if (UTF8_IS_LEAD((unsigned char)pat[1]) && pat[1] == (char)0xE2) {
			if (pat[2] == (char)0x89 && pat[3] == (char)0xA0) { // ≠
				if (pat[4] == '*' || pat[4] == '%') {
					nPat[0] = pat[1];
					nPat[1] = pat[2];
					nPat[2] = pat[3];
					nPat[3] = EOS;
					return(countMatchedWordSyms(word, nPat));
				}
			} else if (pat[2] == (char)0x86 && pat[3] == (char)0xAB) { // ↫ - - ↫
				if (pat[4] == '*' || pat[4] == '%') {
					nPat[0] = pat[1];
					nPat[1] = pat[2];
					nPat[2] = pat[3];
					nPat[3] = EOS;
					return(countMatchedWordSyms(word, nPat)/2);
				}
			}
		}
	}
	return(0);
}

static void addToPOSList(struct flp_sp *ts, char *ws, char *wm) {

	rootLines = flp_AddNewLine(rootLines, NULL, ws);
	ts->total++;
	ts->words = flp_freqTree(ts->words, ts, wm, NULL, rootLines);
	if (rootLines != NULL && !rootLines->isUsed)
		rootLines = flucal_freeLines(rootLines, TRUE);

}

#define set_WWR_to_1(x)		(char)(x | 1)
#define is_WWR(x)			(char)(x & 1)

#define set_mWWR_to_1(x)	(char)(x | 2)
#define is_mWWR(x)			(char)(x & 2)

#define WWRSQMAX 200
// #define DEBUGmWWR

static void addToWordList(struct flp_sp *ts, char *wm, struct cWds *tw) {
	ts->total++;
	ts->words = flp_freqTree(ts->words, ts, wm, tw, NULL);
}

static int isRightText(char *gem_word) {
	int i = 0;
	int found = 0;

	if (GemMode == '\0')
		return(TRUE);
	filterwords("@", uttline, excludeGemKeywords);
	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i)))
		found++;
	if (GemMode == 'i')
		return((flp_group == FALSE && found) || (flp_SpecWords == found));
	else
		return((flp_group == TRUE && flp_SpecWords > found) || (found == 0));
}

static char isEndTier(char *sp, char *line, int i) {
	char word[BUFSIZ+1];

	while ((i=getword(sp, line, word, NULL, i))) {
		if (word[0] != '[' && word[0] != '(' && word[0] != '0' && word[0] != '+' && !uS.IsUtteranceDel(word, 0)) {
			return(FALSE);
		}
	}
	return(TRUE);
}

static int FindBulletTime(char isLastBullet, char *line, int i, float *cBeg, float *cEnd) {
	long Beg = 0L;
	long End = 0L;
	FNType Fname[FILENAME_MAX];

	*cBeg = 0.0;
	*cEnd = 0.0;
	for (; line[i] != EOS; i++) {
		if (line[i] == HIDEN_C) {
			if (isdigit(line[i+1])) {
				if (getMediaTagInfo(line+i, &Beg, &End)) {
					*cBeg = (float)(Beg);
					*cEnd = (float)(End);
				}
			} else {
				if (getOLDMediaTagInfo(line+i, SOUNDTIER, Fname, &Beg, &End)) {
					*cBeg = (float)(Beg);
					*cEnd = (float)(End);
				} else if (getOLDMediaTagInfo(line+i, REMOVEMOVIETAG, Fname, &Beg, &End)) {
					*cBeg = (float)(Beg);
					*cEnd = (float)(End);
				}
			}
			for (i++; line[i] != HIDEN_C && line[i] != EOS; i++) ;
			if (line[i] == HIDEN_C)
				i++;
			if (isLastBullet == FALSE)
				break;
		}
	}
	return(i);
}

void call()	{		/* tabulate array of word lengths */
	int  i, j, wi, wsLen, syllCnt, chrCnt, wwrSqI, wwrSqCnt, found;
	char lRightspeaker;
	unsigned char wwrSq[WWRSQMAX];
	char word[BUFSIZ+1], *ws, *wm, *s, code;
	char lastSp[SPEAKERLEN];
	char isRepSeg;
	char isOutputGem, isMultiLang;
	struct cWds tw;
	struct flp_sp *ts;
	struct utt_elems *root_elem;
	MORFEATS word_feats, *compd, *feat;
	IEWORDS *twd;

	isMultiLang = FALSE;
	langType = UNK;
	if (isPOSMatch) {
		fprintf(stderr,"From file <%s>\n", oldfname);
	}
	if (isNOptionSet == FALSE) {
		strcpy(flp_BBS, "@*&#");
		strcpy(flp_CBS, "@*&#");
	}
	if (flp_SpecWords) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
	ts = NULL;
	lRightspeaker = FALSE;
	found = 0;
	lastSp[0] = EOS;
	root_elem = NULL;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
 if (lineno > tlineno) {
	tlineno = lineno + 200;
 }
*/
		if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*')
				lRightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*') {
				i = isPostCodeFound(utterance->speaker, utterance->line);
				if ((PostCodeMode == 'i' && i == 1) || (PostCodeMode == 'e' && i == 5))
					lRightspeaker = FALSE;
				else
					lRightspeaker = TRUE;
			}
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (flp_SpecWords && !strcmp(flp_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				flp_n_option = FALSE;
				strcpy(flp_BBS, "@BG:");
				strcpy(flp_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				flp_n_option = TRUE;
				strcpy(flp_BBS, "@G:");
				strcpy(flp_CBS, "@*&#");
			}
		}

		if (*utterance->speaker == '@') {
			if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE)) {
				uS.remFrontAndBackBlanks(utterance->line);
				if (strchr(utterance->line, ',') != NULL || strchr(utterance->line, ' ') != NULL) {
					isMultiLang = TRUE;
					langType = UNK;
					if (isLanguageExplicit != 2) {
						isLanguageExplicit = 2;
						InitLanguagesTable();
						addToLanguagesTable(utterance->line, utterance->speaker);
					}
					if (langPrecode == FALSE) {
#ifdef UNX
						fprintf(stderr,"@Languages: header seems to have more than one language defined.\n");
						fprintf(stderr,"Please use +/-s\"[- code]\" option to specify which language code you want to analyze.\n");
#else
						fprintf(stderr,"%c%c@Languages: header seems to have more than one language defined.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
						fprintf(stderr,"%c%cPlease use +/-s\"[- code]\" option to specify which language code you want to analyze.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
						cutt_exit(0);
					}
				} else {
					for (j=0; utterance->line[j] != EOS; j++) {
						if (!uS.mStrnicmp(utterance->line+j, "fra", 3)) {
#ifdef UNX
							fprintf(stderr,"UNIX FLUPOS can not count syllables for French language.\n");
							fprintf(stderr,"Try adding +b option to command line for word count.\n");
							cutt_exit(0);
#else
							langType = FRA;
#endif
						} else if (!uS.mStrnicmp(utterance->line+j, "zho", 3)) {
#ifdef UNX
							fprintf(stderr,"UNIX FLUPOS can not count syllables for Chinese language.\n");
							fprintf(stderr,"Try adding +b option to command line for word count.\n");
							cutt_exit(0);
#else
							langType = ZHO;
#endif
						} else if (!uS.mStrnicmp(utterance->line+j, "eng", 3)) {
							langType = ENG;
						}
					}
				}
			}
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
					uS.remblanks(utterance->line);
					flp_FindSpeaker(oldfname, templineC, utterance->line, FALSE);
				}
			}
			if (uS.partcmp(utterance->speaker,flp_BBS,FALSE,FALSE)) {
				if (flp_n_option) {
					if (isRightText(word)) {
						isOutputGem = TRUE;
					} else
						isOutputGem = FALSE;
				} else {
					if (isRightText(word)) {
						found++;
						if (found == 1 || GemMode != '\0') {
							isOutputGem = TRUE;
						}
					}
				}
			} else if (found > 0 && uS.partcmp(utterance->speaker,flp_CBS,FALSE,FALSE)) {
				if (flp_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (flp_SpecWords)
								isOutputGem = FALSE;
							else {
								isOutputGem = TRUE;
							}
						}
					}
				}
			}
		} else if (*utterance->speaker == '*' && isOutputGem) {
			if (root_elem != NULL) {
				process_utt(root_elem);
			}
			strcpy(templineC, utterance->speaker);
			ts = flp_FindSpeaker(oldfname, templineC, NULL, TRUE);
			if (isMultiLang == TRUE) {
				if (utterance->line[0] == '[' && utterance->line[1] == '-' && isSpace(utterance->line[2])) {
					for (j=3; isSpace(utterance->line[j]); j++) ;
					if (!uS.mStrnicmp(utterance->line+j, "fra", 3)) {
#ifdef UNX
						fprintf(stderr,"UNIX FLUPOS can not count syllables for French language.\n");
						fprintf(stderr,"Try adding +b option to command line for word count.\n");
						cutt_exit(0);
#else
						ts->isSyllableLang = TRUE;
						langType = FRA;
#endif
					} else if (!uS.mStrnicmp(utterance->line+j, "zho", 3)) {
#ifdef UNX
						fprintf(stderr,"UNIX FLUPOS can not count syllables for Chinese language.\n");
						fprintf(stderr,"Try adding +b option to command line for word count.\n");
						cutt_exit(0);
#else
						ts->isSyllableLang = TRUE;
						langType = ZHO;
#endif
					} else if (!uS.mStrnicmp(utterance->line+j, "eng", 3)) {
						langType = ENG;
						ts->isSyllableLang = TRUE;
					} else {
						langType = UNK;
						ts->isSyllableLang = FALSE;
					}
				}
			}
			if (langType == UNK) {
#ifdef UNX
				fprintf(stderr,"FLUPOS can only count syllables for English or French languages.\n");
				fprintf(stderr,"Can't determine the language of datafile for syllables count.\n");
				fprintf(stderr,"Please make sure that this datafile has correct @Languages: header.\n\n");
				fprintf(stderr,"Try adding +b option to command line for other languages.\n");
#else
				fprintf(stderr,"%c%cFLUPOS can only count syllables for English, French and Chinese languages.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
				fprintf(stderr,"%c%cPlease make sure that this datafile has correct @Languages: header.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
				fprintf(stderr,"%c%cTry adding +b option to command line for other languages.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
				cutt_exit(0);
			}
			if (isPOSMatch) {
				for (i=0; i < WWRSQMAX; i++) {
					wwrSq[i] = 0;
				}
				wwrSqCnt = 0;
				i = 0;
				while ((i=getword(utterance->speaker, utterance->line, word, &wi, i))) {
					if (!strcmp(word, "[/]")) {
						if (isOnlyOneWordPreCode(utterance->line, wi)) {
							wwrSq[wwrSqCnt] = set_WWR_to_1(wwrSq[wwrSqCnt]);
							if (isMonoWordPreCode(utterance->line, wi)) {
#ifdef DEBUGmWWR
								fprintf(stdout, "1 (%d) mWWR=%s\n", wwrSqCnt, utterance->line+wi); // lxs
#endif
								wwrSq[wwrSqCnt] = set_mWWR_to_1(wwrSq[wwrSqCnt]);
							}
						}
						wwrSqCnt++;
					}
				}
			} else {
				i = 0;
				while ((i=getitem(utterance->speaker, utterance->line, word, i))) {
					code = ISWORD;
					root_elem = AddToUttElems(root_elem, word, code);
				}
				process_utt(root_elem);
				root_elem = freeUttElems(root_elem);
			}
			strcpy(spareTier1, utterance->line);
		} else if (*utterance->speaker == '%' && ts != NULL && isOutputGem) {
			ts->isMORFound = TRUE;
			if (isPOSMatch) {
				wwrSqI = 0;
#ifdef DEBUGmWWR
strcpy(spareTier2, utterance->line); // lxs
removeDepTierItems(spareTier2); // lxs
#endif // DEBUGmWWR
				while ((i=getNextDepTierPair(utterance->line, word, templineC4, &wi, i)) != 0) {
#ifdef DEBUGmWWR
for (; isSpace(spareTier2[wi]); wi++) ; // lxs
#endif // DEBUGmWWR
					strcpy(templineC2, templineC4);
					if (!exclude(templineC2)) {
					} else if (word[0] != EOS && templineC4[0] != EOS) {
						if (isMorTierFirst) {
							wm = word;
							ws = templineC2;
						} else {
							wm = templineC2;
							ws = word;
						}
						for (twd=wdptr; twd != NULL; twd=twd->nextword) {
							strcpy(templineC2, templineC4);
							if (uS.patmat(templineC2, twd->word)) {
								wsLen = strlen(ws);
								if (!strcmp(templineC4, "[/]") || !strcmp(templineC4, "[//]")) {
									if (wwrSqI < wwrSqCnt) {
										if (is_WWR(wwrSq[wwrSqI])) {
											ws[wsLen] = EOS;
											strcat(ws, " {WWR}");
											addToPOSList(ts, ws, wm);
											ws[wsLen] = EOS;
										}
										if (is_mWWR(wwrSq[wwrSqI])) {
											ws[wsLen] = EOS;
											strcat(ws, " {mono-WWR}");
											addToPOSList(ts, ws, wm);
											ws[wsLen] = EOS;
#ifdef DEBUGmWWR
fprintf(stdout, "2 (%d) mWWR=%s\n", wwrSqI, spareTier2+wi); // lxs
#endif
										}
									}
								} else {
									chrCnt = countMatchedSyms(templineC4, twd->word);
									do {
										uS.remblanks(templineC2);
										ws[wsLen] = EOS;
										strcat(ws, " {");
										strcat(ws, twd->word);
										strcat(ws, "}");
										addToPOSList(ts, ws, wm);
										ws[wsLen] = EOS;
										chrCnt--;
									} while (chrCnt > 0) ;
								}
							}
						}
					}
					if (!strcmp(templineC4, "[/]") || !strcmp(templineC4, "[//]")) {
						wwrSqI++;
					}
				}
			} else { // else if (isPOSMatch)
				i = 0;
				while ((i=getNextDepTierPair(uttline, word, templineC4, NULL, i)) != 0) {
					if (word[0] != EOS && templineC4[0] != EOS) {
						if (strchr(word, '|') != NULL || strcmp(word, "NO_POS") == 0) {
							wm = word;
							ws = templineC4;
						} else {
							wm = templineC4;
							ws = word;
						}
						syllCnt = 0;
						s = strchr(wm, '|');
						if (s != NULL) {
							if (strcmp(ws, "[/]") != 0) {
								ts->morWords++;
							}
							if (sampleType == 'w' && ts->morWords >= sampleSize)
								goto finish;
							strcpy(templineC2, wm);
							if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
								flp_error(FALSE);
							for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
								// counts open/closed BEG
								if (langType == ENG) {
									if (isEqual("adv:int", feat->pos)) {
										addToWordList(ts, feat->stem, &tw);
										tw.wc->closedClassD++;
									} else if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2) || isAllv(feat) ||
											   isnEqual("cop", feat->pos, 3) || isEqual("adj", feat->pos)) {
										addToWordList(ts, feat->stem, &tw);
										tw.wc->openClassD++;
									} else if ((isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) &&
											   isEqualIxes("LY", feat->suffix, NUM_SUFF)) {
										addToWordList(ts, feat->stem, &tw);
										tw.wc->openClassD++;
									} else if (isEqual("co", feat->pos) == FALSE && isEqual("on", feat->pos) == FALSE) {
										addToWordList(ts, feat->stem, &tw);
										tw.wc->closedClassD++;
									}
								} else { // if (langType == FRA)
									if (isEqual("noun", feat->pos) || isEqual("verb", feat->pos) ||
										isEqual("n", feat->pos)    || isnEqual("n:", feat->pos, 2) || 
										isEqual("v", feat->pos)    || isnEqual("v:", feat->pos, 2) || 
										isEqual("adj", feat->pos)  || isEqual("adv", feat->pos)) {
										addToWordList(ts, feat->stem, &tw);
										tw.wc->openClassD++;
									} else {
										addToWordList(ts, feat->stem, &tw);
										tw.wc->closedClassD++;
									}
								} 
								// counts open/closed END

								isRepSeg = FALSE;
								if (word[0] != '[' && word[0] != '0') {
									for (j=0; word[j] != EOS; j++) {
										if (word[j] == ':') {
											addToWordList(ts, feat->stem, &tw);
											tw.wc->prolongation++;
										} else if (word[j] == '^') {
											addToWordList(ts, feat->stem, &tw);
											tw.wc->broken_word++;
										} else if (UTF8_IS_LEAD((unsigned char)word[j]) && word[j] == (char)0xE2) {
											if (word[j+1] == (char)0x89 && word[j+2] == (char)0xA0) { // ≠
												addToWordList(ts, feat->stem, &tw);
												tw.wc->block++;
											} else if (word[j+1] == (char)0x86 && word[j+2] == (char)0xAB) { // ↫ - - ↫
												isRepSeg = !isRepSeg;
												if (isRepSeg) {
													addToWordList(ts, feat->stem, &tw);
													tw.wc->PWR++;
												}
											}
										}
									}
								}
								if (!strcmp(word, "[/]")) {
									if (isOnlyOneWordPreCode(utterance->line, wi)) {
										addToWordList(ts, feat->stem, &tw);
										tw.wc->WWR++;
										if (isMonoWordPreCode(utterance->line, wi)) {
											addToWordList(ts, feat->stem, &tw);
											tw.wc->mWWR++;
										}
									}
								}

								if (*(s+1) == '+' && strcmp(ws, "[/]") != 0) {
									if (feat->compd != NULL) {
										for (compd=feat; compd != NULL; compd=compd->compd) {
											if (compd->stem != NULL && compd->stem[0] != EOS) {
												syllCnt += countSyllables(compd->stem, wm);
												syllCnt += countSyllables(ws+strlen(compd->stem), wm);
												break;
											}
										}
										break;
									}
								}
							}
							freeUpFeats(&word_feats);
							if (strcmp(ws, "[/]") != 0) {
								if (syllCnt == 0) {
									syllCnt += countSyllables(ws, wm);
								}
								if (SyllListFP != NULL) {
									rootSyll = syll_tree(rootSyll, ws, syllCnt);
								}
								ts->morSyllables += (float)syllCnt;
								if (sampleType == 's' && ts->morSyllables >= sampleSize) {
									ts->morSyllables = (float)sampleSize;
									goto finish;
								}
							}
						}
					}
				}
			}
		}
	}
finish:
	if (!combinput) {
		flp_pr_result();
		rootLines = flucal_freeLines(rootLines, FALSE);
	}
}

static void SyllPrint(struct syll_tnode *p) {
	if (p != NULL) {
		SyllPrint(p->left);
		do {
			fprintf(SyllListFP,"%s %d\n", p->word, p->count);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				SyllPrint(p->right);
				break;
			}
			p = p->right;
		} while (1);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = FLUPOS;
	chatmode = CHAT_MODE;
	OnlydataLimit = 1;
	UttlineEqUtterance = FALSE;
	rootSyll = NULL;
	isSyllList = FALSE;
	SyllListFP = NULL;
#if !defined(_CLAN_DEBUG)
	fprintf(stderr, "This command is not fully implemented yet.\n\n");
	return;
#endif
	bmain(argc,argv,flp_pr_result);
	rootLines = flucal_freeLines(rootLines, FALSE);
	if (SyllListFP != NULL) {
		SyllPrint(rootSyll);
		fclose(SyllListFP);
	}
	syll_freetree(rootSyll);
	rootSyll = NULL;
	sp_head = freespeakers(sp_head);
}
