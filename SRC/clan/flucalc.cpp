/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main flucalc_main
#define call flucalc_call
#define getflag flucalc_getflag
#define init flucalc_init
#define usage flucalc_usage
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

enum {
	UNK = 0,
	ENG,
	FRA,
	ZHO
} ;

// isPOSMatch beg
struct flucalc_lines {
	char isUsed;
	char *loc;
	char *tier;
	flucalc_lines *nextLine;
} ;

struct flucalc_where {
	unsigned int count;
	struct flucalc_lines *line;
	struct flucalc_where *nextMatch;
} ;

struct flucalc_freqTnode {
	char *word;
	struct flucalc_where *where;
	unsigned int count;
	struct flucalc_freqTnode *left;
	struct flucalc_freqTnode *right;
};
// isPOSMatch end

struct flucalc_speakers {
	char isSpeakerFound;
	char isMORFound;
	char isPSDFound; // +/.

	char isSyllableLang;
	char *fname;
	char *sp;
	char *ID;
	char *utt;
	long uttCnt;

	// isPOSMatch beg
	struct flucalc_freqTnode *words;
	long total;				/* total number of words */
	long different;			/* number of different words */
	// isPOSMatch end


	float tm;
	float morUtt;
	float morWords;
	float morSyllables;
	float allWords;

	float prolongation;	// :
	float broken_word;	// ^
	float block;		// ≠
	float PWR;			// ↫ ↫ pairs//Part Word Repetition; Sequences
	float PWRRU;		// ↫ - - ↫  // iterations
	float phono_frag;	// &+       // Phonological_fragment
	float WWR;			// WWR:             how many times one word "the [/] the [/] the" occurs. In this case it is 2, before 2022-08-29 1
	float mWWR;			// mono-WWR:        how many times monosyllable "the [/] the [/] the" occurs. In this case it is 2, before 2022-08-29 1.
	float WWRRU;		// WWR-RU:      [/] Whole Word repetition iterations
	float mWWRRU;		// mono-WWR-RU: [/] Whole monosyllable Word repetition iterations
	float phrase_repets;// Phrase repetitions: <> [/]
	float word_revis;	// [//]
	float phrase_revis;	// <> [//]
	float pauses_cnti;	// (.) & (2.4) internal
	float pauses_dur;	// (2.4)
	float filled_pauses;// &-
	float openClassD, openClassA;
	float closedClassD, closedClassA;
	float allDisfluency;

	float IWDur;
	float UTTDur;
	float SwitchDur;
	float NumOfSwitch;
	float NoSwitchDur;
	float NumNoSwitch;

	struct flucalc_speakers *next_sp;
} ;

struct flucalc_tnode {
	char *word;
	int count;
	struct flucalc_tnode *left;
	struct flucalc_tnode *right;
};

static int  flucalc_SpecWords, DBGemNum;
static char *DBGems[NUMGEMITEMS];
static char isUttList, isSyllWordsList, isWordMode, sampleType, isMorTierFirst, isFilterUtts, isWorTier, isPuaseDurFound;
static char flucalc_ftime, isPOSMatch;
static char flucalc_BBS[5], flucalc_CBS[5];
static char flucalc_group, flucalc_n_option, isNOptionSet, GemMode, specialOptionUsed, langType, langPrecode;
static char isAddPhono_fragToTD;
static long sampleSize;
static struct flucalc_speakers *sp_head;
static struct flucalc_tnode *rootWords;
static flucalc_lines *rootLines;
static FILE *SyllWordsListFP;


void usage() {
	puts("FLUCALC creates a spreedsheet with a series of fluency measures.");
/*
#ifdef UNX
	printf("FluCalc REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cFluCalc REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
*/
	printf("Usage: flucalc [%s] filename(s)\n",mainflgs());
	puts("+a : get pauses time value from %wor tier (default: disregard %wor tier)");
	puts("+b : select word mode analyses (default: syllables mode)");
	puts("+b1: select word with repetition, retraces mode analyses");
	puts("+c : compute Pause_duration from pause notation like (2.4)");
	puts("+c1: add phonological fragment to SLD instead of TD (default: add it to TD instead)");
	puts("+c5: when combined with +p option it will reverse tier's priority");
	puts("+dN: specify sample size of N (s, w), +d100s means first 100 syllables");
	puts("+e1: create file with words and their syllable count");
	puts("+e2: create file with perfectly fluent utterances, both TD and SLD are zero");
	puts("+e3: create file with perfectly fluent utterances, but with TD is non-zero");
	puts("+e4: create file with perfectly fluent utterances, but with SLD is non-zero");
	puts("+e5: create file with disfluencies, both TD and SLD are non-zero");
	puts("+u : compute output for every utterance individually");
	puts("+pS: search for word S and match it to corresponding POS");
	puts("+g : Gem tier should contain all words specified by +gS");
	puts("+gS: select gems which are labeled by label S");
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	puts("+p@F: search for words in file F and match it to corresponding POS");
	mainusage(FALSE);
/*
#ifdef UNX
	printf("FluCalc REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cFluCalc REQUIRES THE PRESENCE OF THE \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO USE ONLY THE MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
*/
	cutt_exit(0);
}

// isPOSMatch beg
static flucalc_lines *flucal_freeLines(flucalc_lines *lines, char isJustOne) {
	flucalc_lines *t;

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

static flucalc_where *flucal_freeWhere(flucalc_where *where) {
	flucalc_where *t;

	while (where != NULL) {
		t = where;
		where = where->nextMatch;
		free(t);
	}
	return(NULL);
}

static void flucalc_freqFreetree(struct flucalc_freqTnode *p) {
	if (p != NULL) {
		flucalc_freqFreetree(p->left);
		flucalc_freqFreetree(p->right);
		free(p->word);
		p->where = flucal_freeWhere(p->where);
		free(p);
	}
}
// isPOSMatch end

static struct flucalc_speakers *freespeakers(struct flucalc_speakers *p) {
	struct flucalc_speakers *ts;

	while (p) {
		ts = p;
		p = p->next_sp;
		if (ts->fname != NULL)
			free(ts->fname);
		if (ts->sp != NULL)
			free(ts->sp);
		if (ts->ID != NULL)
			free(ts->ID);
		if (ts->utt != NULL)
			free(ts->utt);
		flucalc_freqFreetree(ts->words);
		free(ts);
	}
	return(NULL);
}

static void flucalc_freetree(struct flucalc_tnode *p) {
	if (p != NULL) {
		flucalc_freetree(p->left);
		flucalc_freetree(p->right);
		free(p->word);
		free(p);
	}
}

static void flucalc_error(char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	sp_head = freespeakers(sp_head);
	if (SyllWordsListFP != NULL)
		fclose(SyllWordsListFP);
	flucalc_freetree(rootWords);
	rootWords = NULL;
	rootLines = flucal_freeLines(rootLines, FALSE);
	cutt_exit(0);
}

// isPOSMatch beg
static flucalc_lines *flucalc_AddNewLine(flucalc_lines *lines, char *loc, char *tier) {
	flucalc_lines *p;

	if ((p=NEW(struct flucalc_lines)) == NULL)
		flucalc_error(TRUE);
	if (loc != NULL) {
		if ((p->loc=(char *)malloc(strlen(loc)+1)) == NULL)
			flucalc_error(TRUE);
		strcpy(p->loc, loc);
	} else
		p->loc = NULL;
	if ((p->tier=(char *)malloc(strlen(tier)+1)) == NULL)
		flucalc_error(TRUE);
	strcpy(p->tier, tier);
	p->nextLine = lines;
	p->isUsed = FALSE;
	lines = p;

	return(lines);
}

static void addDBGems(char *gem) {
	if (DBGemNum >= NUMGEMITEMS) {
		fprintf(stderr, "\nERROR: Too many keywords specified. The limit is %d\n", NUMGEMITEMS);
		flucalc_error(FALSE);
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

static char *flucalc_strsave(char *s) {
	char *p;

	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		flucalc_error(TRUE);
	return(p);
}

static struct flucalc_freqTnode *flucalc_talloc(char *word, unsigned int count) {
	struct flucalc_freqTnode *p;

	if ((p=NEW(struct flucalc_freqTnode)) == NULL)
		flucalc_error(TRUE);
	p->word = word;
	p->count = count;
	return(p);
}

static flucalc_where *flucalc_AddNewMatch(flucalc_where *where, flucalc_lines *line) {
	flucalc_where *p;

	if (where == NULL) {
		if ((p=NEW(struct flucalc_where)) == NULL)
			flucalc_error(TRUE);
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
		if ((p->nextMatch=NEW(struct flucalc_where)) == NULL)
			flucalc_error(TRUE);
		p = p->nextMatch;
	}
	p->nextMatch = NULL;
	p->count = 1;
	p->line = line;
	line->isUsed = TRUE;
	return(where);
}

static void flucalc_addlineno(struct flucalc_freqTnode *p, flucalc_lines *line) {
	p->count++;
	if (line != NULL) {
		p->where = flucalc_AddNewMatch(p->where, line);
	}
}

static struct flucalc_freqTnode *flucalc_freqTree(struct flucalc_freqTnode *p, flucalc_speakers *ts, char *w, flucalc_lines *line) {
	int cond;
	struct flucalc_freqTnode *t = p;

	if (p == NULL) {
		ts->different++;
		p = flucalc_talloc(flucalc_strsave(w), 0);
		p->where = NULL;
		flucalc_addlineno(p, line);
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->word)) == 0)
		flucalc_addlineno(p, line);
	else if (cond < 0)
		p->left = flucalc_freqTree(p->left, ts, w, line);
	else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0)
			flucalc_addlineno(p, line);
		else if (cond < 0)
			p->left = flucalc_freqTree(p->left, ts, w, line);
		else
			p->right = flucalc_freqTree(p->right, ts, w, line); // if cond > 0
		return(t);
	}
	return(p);
}
// isPOSMatch end

static void flucalc_initTSVars(struct flucalc_speakers *ts) {
	ts->isMORFound = FALSE;
	ts->isPSDFound = FALSE;

	if (langType == UNK)
		ts->isSyllableLang = FALSE;
	else
		ts->isSyllableLang = TRUE;
	// isPOSMatch beg
	ts->words = NULL;
	ts->total = 0L;		/* total number of words */
	ts->different = 0L;	/* number of different words */
	// isPOSMatch end

	ts->tm		  = 0.0;
	ts->morUtt	  = 0.0;
	ts->morWords  = 0.0;
	ts->morSyllables= 0.0;
	ts->allWords = 0.0;

	ts->prolongation	= 0.0;
	ts->broken_word		= 0.0;
	ts->block			= 0.0;
	ts->PWR				= 0.0;
	ts->PWRRU			= 0.0;
	ts->phono_frag		= 0.0;
	ts->WWR				= 0.0;
	ts->mWWR			= 0.0;
	ts->WWRRU			= 0.0;
	ts->mWWRRU			= 0.0;
	ts->phrase_repets	= 0.0;
	ts->word_revis	    = 0.0;
	ts->phrase_revis    = 0.0;
	ts->pauses_cnti		= 0.0;
	ts->pauses_dur		= 0.0;
	ts->filled_pauses   = 0.0;
	ts->openClassD		= 0.0;
	ts->openClassA		= 0.0;
	ts->closedClassD	= 0.0;
	ts->closedClassA	= 0.0;
	ts->allDisfluency	= 0.0;

	ts->IWDur			= 0.0;
	ts->UTTDur			= 0.0;
	ts->SwitchDur		= 0.0;
	ts->NumOfSwitch		= 0.0;
	ts->NoSwitchDur		= 0.0;
	ts->NumNoSwitch		= 0.0;
}

static struct flucalc_speakers *flucalc_FindSpeaker(char *fname, char *sp, char *ID, long uttCnt, char isSpeakerFound) {
	int i;
	struct flucalc_speakers *ts, *tsp;

	uS.remblanks(sp);
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->fname, fname) == 0) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
				ts->isSpeakerFound = isSpeakerFound;
				if (isUttList) {
					if (ts->uttCnt == uttCnt)
						return(ts);
					if (ID == NULL)
						ID = ts->ID;
				} else
					return(ts);
			}
		}
	}
	if ((ts=NEW(struct flucalc_speakers)) == NULL)
		flucalc_error(TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		free(ts);
		flucalc_error(TRUE);
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
		flucalc_error(TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			flucalc_error(TRUE);
		strcpy(ts->ID, ID);
	}
	if (isUttList) {
		strcpy(templineC1, utterance->line);
		for (i=0; templineC1[i] != EOS; i++) {
			if (templineC1[i] == '\n' || templineC1[i] == '\t')
				templineC1[i] = ' ';
			removeExtraSpace(templineC1);
			uS.remFrontAndBackBlanks(templineC1);
		}
		if ((ts->utt=(char *)malloc(strlen(templineC1)+1)) == NULL)
			flucalc_error(TRUE);
		strcpy(ts->utt, templineC1);
	} else
		ts->utt = NULL;
	ts->uttCnt = uttCnt;
	ts->isSpeakerFound = isSpeakerFound;
	flucalc_initTSVars(ts);
	return(ts);
}

void init(char f) {
	int i;
	FNType debugfile[FNSize];
//	struct tier *nt;
	IEWORDS *twd;

	if (f) {
		sp_head = NULL;
		rootLines = NULL;
		flucalc_ftime = TRUE;
		fatal_error_function = flucalc_error;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		mor_initwords();
		isAddPhono_fragToTD = TRUE;
		isSyllWordsList = FALSE;
		SyllWordsListFP = NULL;
		rootWords = NULL;
		isWordMode = 0;
		isPOSMatch = FALSE;
		sampleSize = 0L;
		sampleType = 0;
		isFilterUtts = 0;
		isPuaseDurFound = FALSE;
		isUttList = FALSE;
		isWorTier = FALSE;
		isMorTierFirst = TRUE;
		specialOptionUsed = FALSE;
		langPrecode = FALSE;
		GemMode = '\0';
		flucalc_SpecWords = 0;
		flucalc_group = FALSE;
		flucalc_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(flucalc_BBS, "@*&#");
		strcpy(flucalc_CBS, "@*&#");
		DBGemNum = 0;
	} else {
		if (flucalc_ftime) {
			flucalc_ftime = FALSE;
			if (chatmode) {
				if (isWorTier == TRUE) {
					LocalTierSelect = TRUE;
				} else if (GemMode != '\0' || isNOptionSet == TRUE) {
					LocalTierSelect = TRUE;
				} else {
					maketierchoice("@Languages:",'+',FALSE);
					if (isWorTier == TRUE)
						maketierchoice("%wor:",'+',FALSE);
					maketierchoice("@ID:",'+',FALSE);
					maketierchoice("%mor:",'+',FALSE);
				}
			} else {
				fprintf(stderr, "FluCalc can only run on CHAT data files\n\n");
				cutt_exit(0);
			}
/*
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
*/
			if (flucalc_group && GemMode == '\0') {
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
				if (sampleType != 0 && isFilterUtts != 0) {
					fprintf(stderr,"The +d option can not be used with +e option\n");
					cutt_exit(0);
				}
				if (!f_override)
					stout = FALSE;
				if (isFilterUtts == 0) {
					AddCEXExtension = ".xls";
					combinput = TRUE;
					linkMain2Mor = TRUE;
				} else if (isFilterUtts == 1) {
					strcpy(GExt, ".TD&SLD=0");
					maketierchoice("%gra:",'+',FALSE);
				} else if (isFilterUtts == 2) {
					strcpy(GExt, ".TD_non_0");
					maketierchoice("%gra:",'+',FALSE);
				} else if (isFilterUtts == 3) {
					strcpy(GExt, ".SLD_non_0");
					maketierchoice("%gra:",'+',FALSE);
				} else if (isFilterUtts == 4) {
					strcpy(GExt, ".TD&SLD_non_0");
					maketierchoice("%gra:",'+',FALSE);
				}
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
				if (isFilterUtts != 0) {
					fprintf(stderr,"The +e option can not be used with +p option\n");
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
			if (isSyllWordsList) {
				strcpy(debugfile, "word_syllables.cex");
				SyllWordsListFP = fopen(debugfile, "w");
				if (SyllWordsListFP == NULL) {
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
		case 'a':
			no_arg_option(f);
			isWorTier = TRUE;
			break;
		case 'b':
			OverWriteFile = FALSE;
			if (*f == EOS || *f == '0') {
				isWordMode = 1;
			} else if (*f == '1') {
				isWordMode = 2;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'c':
			if (*f == EOS || *f == '0')
				isPuaseDurFound = TRUE;
			else if (*f == '1')
				isAddPhono_fragToTD = FALSE;
			else if (*f == '5')
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
			if (*f == '1')
				isSyllWordsList = TRUE;
			else if (*f == '2') {
				isFilterUtts = 1;
			} else if (*f == '3') {
				isFilterUtts = 2;
			} else if (*f == '4') {
				isFilterUtts = 3;
			} else if (*f == '5') {
				isFilterUtts = 4;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'u':
			isUttList = TRUE;
			no_arg_option(f);
			break;
		case 'g':
			if (*f == EOS) {
				flucalc_group = TRUE;
				specialOptionUsed = TRUE;
			} else {
				GemMode = 'i';
				flucalc_SpecWords++;
				addDBGems(getfarg(f,f1,i));
			}
			break;
		case 'n':
			if (*(f-2) == '+') {
				flucalc_n_option = TRUE;
				strcpy(flucalc_BBS, "@G:");
				strcpy(flucalc_CBS, "@*&#");
				specialOptionUsed = TRUE;
			} else {
				strcpy(flucalc_BBS, "@BG:");
				strcpy(flucalc_CBS, "@EG:");
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
			if (*(f-2) == '+' && (*f == '*' || *f == '#')) {
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
static void flucalc_printlines(struct flucalc_freqTnode *p) {
	flucalc_where *w;

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

static void flucalc_freqTreeprint(struct flucalc_freqTnode *p) {
	if (p != NULL) {
		flucalc_freqTreeprint(p->left);
		do {
			fprintf(fpout,"%3u ",p->count);
			fprintf(fpout,"%-10s", p->word);
			flucalc_printlines(p);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				flucalc_freqTreeprint(p->right);
				break;
			}
			p = p->right;
		} while (1);
	}
}
// isPOSMatch end

static void flucalc_pr_result(void) {
	char  *sFName;
	float SLD, TD, X, devNum;
	struct flucalc_speakers *ts;

	if (sp_head == NULL) {
		if (flucalc_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n");
			fprintf(stderr,"OR No specified gems found for this speaker\n\n");
		} else
			fprintf(stderr, "\nERROR: No speaker matching +t option found\n\n");
	}
	if (isFilterUtts > 0) {
	} else if (isPOSMatch) {
		for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
			if (!ts->isSpeakerFound) {
				fprintf(stderr, "\nWARNING: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
				continue;
			}
			fprintf(fpout, "Speaker: %s\n", ts->sp);
			flucalc_freqTreeprint(ts->words);
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

		if (isUttList == TRUE)
			excelStrCell(fpout, "Utt_line");
		if (isUttList == FALSE)
			excelStrCell(fpout, "mor_Utts");
		excelStrCell(fpout, "mor_words");
		excelStrCell(fpout, "mor_syllables");
		excelStrCell(fpout, "all_words");
		if (isUttList == FALSE) {
			excelStrCell(fpout, "words_min");
			excelStrCell(fpout, "syllables_min");
			excelStrCell(fpout, "all_words_min");
		}

		excelStrCell(fpout, "#_Prolongation");
		excelStrCell(fpout, "%_Prolongation");
		excelStrCell(fpout, "#_Broken_word");
		excelStrCell(fpout, "%_Broken_word");
		excelStrCell(fpout, "#_Block");
		excelStrCell(fpout, "%_Block");
		excelStrCell(fpout, "#_PWR");
		excelStrCell(fpout, "%_PWR");
		excelStrCell(fpout, "#_PWR-RU");
		excelStrCell(fpout, "%_PWR-RU");
		excelStrCell(fpout, "#_WWR");
		excelStrCell(fpout, "%_WWR");
		excelStrCell(fpout, "#_mono-WWR");
		excelStrCell(fpout, "%_mono-WWR");
		excelStrCell(fpout, "#_WWR-RU");
		excelStrCell(fpout, "%_WWR-RU");
		excelStrCell(fpout, "#_mono-WWR-RU");
		excelStrCell(fpout, "%_mono-WWR-RU");
		excelStrCell(fpout, "Mean_RU");
		excelStrCell(fpout, "#_Phonological_fragment");
		excelStrCell(fpout, "%_Phonological_fragment");
		excelStrCell(fpout, "#_Phrase_repetitions");
		excelStrCell(fpout, "%_Phrase_repetitions");
		excelStrCell(fpout, "#_Word_revisions");
		excelStrCell(fpout, "%_Word_revisions");
		excelStrCell(fpout, "#_Phrase_revisions");
		excelStrCell(fpout, "%_Phrase_revisions");
		excelStrCell(fpout, "#_Pauses");
		excelStrCell(fpout, "%_Pauses");
		if (isPuaseDurFound == TRUE) {
			excelStrCell(fpout, "#_Pause_duration");
			excelStrCell(fpout, "%_Pause_duration");
		}
		excelStrCell(fpout, "#_Filled_pauses");
		excelStrCell(fpout, "%_Filled_pauses");

		excelStrCell(fpout, "#_TD");
		excelStrCell(fpout, "%_TD");
		excelStrCell(fpout, "#_SLD");
		excelStrCell(fpout, "%_SLD");

		excelStrCell(fpout, "#_Total_(SLD+TD)");
		excelStrCell(fpout, "%_Total_(SLD+TD)");

		if (isUttList == FALSE) {
			excelStrCell(fpout, "SLD_Ratio");
			
			excelStrCell(fpout, "Content_words_ratio");
			
			excelStrCell(fpout, "Function_words_ratio");
			
			excelStrCell(fpout, "Content_all_words_ratio");
			
			excelStrCell(fpout, "Function_all_words_ratio");
			
			excelStrCell(fpout, "Weighted_SLD");
		}
		if (isWorTier == TRUE) {
			excelStrCell(fpout, "IW_Dur");
			excelStrCell(fpout, "Utt_dur");
			excelStrCell(fpout, "IW_Dur/Utt_dur");
			excelStrCell(fpout, "Switch_Dur");
			excelStrCell(fpout, "#_Switch");
			excelStrCell(fpout, "Switch_Dur/#_Switch");
			excelStrCell(fpout, "No_Switch_Dur");
			excelStrCell(fpout, "#_No_Switch");
			excelStrCell(fpout, "No_Switch_Dur/#_No_Switch");
		}

		excelRow(fpout, ExcelRowEnd);

		for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
			if (!ts->isSpeakerFound) {
				if (flucalc_SpecWords) {
					fprintf(stderr,"\nWARNING: No specified gems found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
				} else
					fprintf(stderr, "\nWARNING: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
				continue;
			}
// ".xls"
			if (!ts->isMORFound && isUttList == TRUE)
				continue;
			
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

			if (isWordMode == 0) {
				devNum = ts->morSyllables;
			} else if (isWordMode == 1) {
				devNum = ts->morWords;
			} else if (isWordMode == 2) {
				devNum = ts->allWords;
			}
			
			if (!ts->isMORFound || devNum == 0.0) {
				fprintf(stderr, "\n*** File \"%s\": Speaker \"%s\"\n", sFName, ts->sp);
				fprintf(stderr, "WARNING: Speaker \"%s\" has no \"%s\" tiers.\n\n", ts->sp, "%mor:");
				excelNumCell(fpout, "%.0f", ts->morUtt);
				excelNumCell(fpout, "%.0f", ts->morWords);
				if (ts->isSyllableLang == TRUE)
					excelNumCell(fpout, "%.0f", ts->morSyllables);
				else
					excelStrCell(fpout, "NA");
				excelNumCell(fpout, "%.0f", ts->allWords);
				excelCommasStrCell(fpout, "NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA");
				if (isWorTier == TRUE)
					excelCommasStrCell(fpout, "NA,NA,NA,NA,NA,NA,NA,NA,NA");
			} else {
				if (isUttList) {
					excelStrCell(fpout, ts->utt);
				}
				if (isUttList == FALSE)
					excelNumCell(fpout, "%.0f", ts->morUtt);
				excelNumCell(fpout, "%.0f", ts->morWords);
				if (ts->isSyllableLang == TRUE)
					excelNumCell(fpout, "%.0f", ts->morSyllables);
				else
					excelStrCell(fpout, "NA");
				excelNumCell(fpout, "%.0f", ts->allWords);

				if (isUttList == FALSE) {
					if (ts->tm == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.3f", ts->morWords / (ts->tm / 60.0000));
					
					if (ts->tm == 0.0 || ts->isSyllableLang == FALSE)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.3f", ts->morSyllables / (ts->tm / 60.0000));
					
					if (ts->tm == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.3f", ts->allWords / (ts->tm / 60.0000));
				}
				excelNumCell(fpout, "%.3f", ts->prolongation);
				excelNumCell(fpout, "%.3f", (ts->prolongation/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->broken_word);
				excelNumCell(fpout, "%.3f", (ts->broken_word/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->block);
				excelNumCell(fpout, "%.3f", (ts->block/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->PWR);
				excelNumCell(fpout, "%.3f", (ts->PWR/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->PWRRU);
				excelNumCell(fpout, "%.3f", (ts->PWRRU/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->WWR);
				excelNumCell(fpout, "%.3f", (ts->WWR/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->mWWR);
				excelNumCell(fpout, "%.3f", (ts->mWWR/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->WWRRU);
				excelNumCell(fpout, "%.3f", (ts->WWRRU/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->mWWRRU);
				excelNumCell(fpout, "%.3f", (ts->mWWRRU/devNum)*100.0000);
				X = ts->PWR + ts->WWR;
				if (X <= 0.0)
					X = 0.0;
				else {
					X = (ts->PWRRU + ts->WWRRU) / X;
				}
				excelNumCell(fpout, "%.3f", X);
				excelNumCell(fpout, "%.3f", ts->phono_frag);
				excelNumCell(fpout, "%.3f", (ts->phono_frag/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->phrase_repets);
				excelNumCell(fpout, "%.3f", (ts->phrase_repets/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->word_revis);
				excelNumCell(fpout, "%.3f", (ts->word_revis/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->phrase_revis);
				excelNumCell(fpout, "%.3f", (ts->phrase_revis/devNum)*100.0000);
				excelNumCell(fpout, "%.3f", ts->pauses_cnti);
				excelNumCell(fpout, "%.3f", (ts->pauses_cnti/devNum)*100.0000);
				if (isPuaseDurFound == TRUE) {
					excelNumCell(fpout, "%.3f", ts->pauses_dur);
					excelNumCell(fpout, "%.3f", (ts->pauses_dur/devNum)*100.0000);
				}
				excelNumCell(fpout, "%.3f", ts->filled_pauses);
				excelNumCell(fpout, "%.3f", (ts->filled_pauses/devNum)*100.0000);

				TD = ts->phrase_repets+ts->word_revis+ts->phrase_revis+ts->pauses_cnti+ts->filled_pauses;
				if (isAddPhono_fragToTD == TRUE)
					TD += ts->phono_frag;
				excelNumCell(fpout, "%.3f", TD);
				excelNumCell(fpout, "%.3f", (TD/devNum)*100.0000);

				SLD = ts->prolongation+ts->broken_word+ts->block+ts->PWR+ts->mWWR;
				if (isAddPhono_fragToTD == FALSE)
					SLD += ts->phono_frag;
				excelNumCell(fpout, "%.3f", SLD);
				excelNumCell(fpout, "%.3f", (SLD/devNum)*100.0000);

				excelNumCell(fpout, "%.3f", SLD+TD);
				excelNumCell(fpout, "%.3f", ((SLD/devNum)*100.0000)+((TD/devNum)*100.0000));

				if (isUttList == FALSE) {
					if ((SLD+TD) == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.3f", SLD / (SLD+TD));
					
					if (ts->allDisfluency == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.3f", ts->openClassD/ts->allDisfluency);
					
					if (ts->allDisfluency == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.3f", ts->closedClassD/ts->allDisfluency);
					
					if (ts->openClassA == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.3f", ts->openClassD/ts->openClassA);
					
					if (ts->openClassA == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.3f", ts->closedClassD/ts->openClassA);
					
					if (ts->isSyllableLang == FALSE || ts->morSyllables == 0.0) {
						excelStrCell(fpout, "NA");
					} else {
						X = (ts->PWR * 100.0000 / ts->morSyllables) + (ts->mWWR * 100.0000 / ts->morSyllables);
						if (X > 0) {
							SLD = (X * ( ( (ts->PWRRU * 100.0000 / ts->morSyllables) + (ts->mWWRRU * 100.0000 / ts->morSyllables) ) / X) ) +
							(2 * ( (ts->prolongation * 100.0000 / ts->morSyllables) + (ts->block * 100.0000 / ts->morSyllables) ) );
						} else {
							SLD = (2 * ( (ts->prolongation * 100.0000 / ts->morSyllables) + (ts->block * 100.0000 / ts->morSyllables) ) );
						}
						excelNumCell(fpout, "%.3f", SLD);
					}
				}
				if (isWorTier == TRUE) {
					excelNumCell(fpout, "%.3f", ts->IWDur);
					excelNumCell(fpout, "%.3f", ts->UTTDur);
					if (ts->UTTDur == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.2f", ts->IWDur / ts->UTTDur);

					excelNumCell(fpout, "%.3f", ts->SwitchDur);
					excelNumCell(fpout, "%.3f", ts->NumOfSwitch);
					if (ts->NumOfSwitch == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.0f", ts->SwitchDur / ts->NumOfSwitch);

					excelNumCell(fpout, "%.3f", ts->NoSwitchDur);
					excelNumCell(fpout, "%.3f", ts->NumNoSwitch);
					if (ts->NumNoSwitch == 0.0)
						excelStrCell(fpout, "NA");
					else
						excelNumCell(fpout, "%.0f", ts->NoSwitchDur / ts->NumNoSwitch);
				}
			}
			excelRow(fpout, ExcelRowEnd);
		}
		excelRow(fpout, ExcelRowEmpty);
		excelRow(fpout, ExcelRowEmpty);
		excelRowOneStrCell(fpout, ExcelBlkCell, "Interpretation of weighted SLD score: Scores above 4.0 is highly suggestive of clinical diagnosis of stuttering in young children");
		excelRowOneStrCell(fpout, ExcelBlkCell, "Weighted score formula (from Ambrose & Yairi, 1999): ((PWR + mono-WWR) * ((PWR-RU + mono-WWR-RU)/(PWR + mono-WWR))) + (2 * (prologations + blocks))");
		if (isWordMode == 0) {
			excelRowOneStrCell(fpout, ExcelRedCell, "This spreadsheet was run in syllable mode");
		} else if (isWordMode == 1) {
			excelRowOneStrCell(fpout, ExcelRedCell, "This spreadsheet was run in %mor tier words mode");
		} else if (isWordMode == 2) {
			excelRowOneStrCell(fpout, ExcelRedCell, "This spreadsheet was run in all words mode");
		}
		excelFooter(fpout);
		sp_head = freespeakers(sp_head);
	}
}

/*

 "Mean RUs" = (PWRRU + WWRRU) / (PWR + WWR)


 TD = phrase_repets+word_revis+phrase_revis+pauses_cnti+phono_frag+filled_pauses;
 "#TD" = TD
 "%TD" = (TD/devNum)*100.0000


 SLD = prolongation+broken_word+block+PWR+WWR;
 "#SLD" = SLD
 "%SLD" = (SLD/devNum)*100.0000


 "#Total (SLD+TD)" = SLD + TD
 "%Total (SLD+TD)" = ((SLD/devNum)*100.0000) + ((TD/devNum)*100.0000)


 "SLD Ratio" = SLD / (SLD + TD)


 X = (PWR * 100.0000 / morSyllables) + (mWWR * 100.0000 / morSyllables)
 SLD = (X * ( ( (PWRRU * 100.0000 / morSyllables) + (mWWRRU * 100.0000 / morSyllables) ) / X) ) +
       (2 * ( (prolongation * 100.0000 / morSyllables) + (block * 100.0000 / morSyllables) ) )


 X = ( PWR + mWWR )
 If X is not equal to zero, then
	SLD = ( X * ( ( PWRRU + mWWRRU) / X  ) ) + ( 2 * ( prolongation + block ) )
 If X is equal to zero, then
	SLD = ( 2 * ( prolongation + block ) )


 SLD = ( (PWR + mWWR) * ( (PWRRU + mWWRRU) / (PWR + mWWR) ) ) + (2 * ( prolongation + block ) )
 "Weighted SLD" = SLD);
*/

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

static struct flucalc_tnode *flucalc_tree(struct flucalc_tnode *p, char *word, int count) {
	int cond;
	struct flucalc_tnode *t = p;

	if (p == NULL) {
		if ((p=NEW(struct flucalc_tnode)) == NULL)
			flucalc_error(TRUE);
		p->word = (char *)malloc(strlen(word)+1);
		if (p->word == NULL)
			flucalc_error(TRUE);
		strcpy(p->word, word);
		p->count = count;
		p->left = p->right = NULL;
	} else if ((cond=strcmp(word,p->word)) < 0)
		p->left = flucalc_tree(p->left, word, count);
	else if (cond > 0){
		for (; (cond=strcmp(word,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond < 0)
			p->left = flucalc_tree(p->left, word, count);
		else if (cond > 0)
			p->right = flucalc_tree(p->right, word, count); /* if cond > 0 */
		return(t);
	}
	return(p);
}

static char flucalc_isUttDel(char *line, int pos) {
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

static char isDisfluency(char *word) {
	int j;

	if (strcmp(word, "[/]") == 0)
		return(TRUE);
	for (j=0; word[j] != EOS; j++) {
		if (word[j] == ':')
			return(TRUE);
		else if (word[j] == '^')
			return(TRUE);
		else if (UTF8_IS_LEAD((unsigned char)word[j]) && word[j] == (char)0xE2) {
			if (word[j+1] == (char)0x89 && word[j+2] == (char)0xA0) // ≠
				return(TRUE);
			else if (word[j+1] == (char)0x86 && word[j+2] == (char)0xAB) // ↫ - - ↫
				return(TRUE);
		}
	}
	return(FALSE);
}

static void addToPOSList(struct flucalc_speakers *ts, char *ws, char *wm) {

	rootLines = flucalc_AddNewLine(rootLines, NULL, ws);
	ts->total++;
	ts->words = flucalc_freqTree(ts->words, ts, wm, rootLines);
	if (rootLines != NULL && !rootLines->isUsed)
		rootLines = flucal_freeLines(rootLines, TRUE);

}

#define set_WWR_to_1(x)		(char)(x | 1)
#define is_WWR(x)			(char)(x & 1)

#define set_mWWR_to_1(x)	(char)(x | 2)
#define is_mWWR(x)			(char)(x & 2)

#define set_WWRRU_to_1(x)	(char)(x | 4)
#define is_WWRRU(x)			(char)(x & 4)

#define set_mWWRRU_to_1(x)	(char)(x | 8)
#define is_mWWRRU(x)		(char)(x & 8)

#define set_wRev_to_1(x)	(char)(x | 16)
#define is_wRev(x)			(char)(x & 16)

#define set_phRep_to_1(x)	(char)(x | 32)
#define is_phRep(x)			(char)(x & 32)

#define set_phRev_to_1(x)	(char)(x | 64)
#define is_phRev(x)			(char)(x & 64)

#define WWRSQMAX 200
// #define DEBUGmWWR

static int isRightText(char *gem_word) {
	int i = 0;
	int found = 0;

	if (GemMode == '\0')
		return(TRUE);
	filterwords("@", uttline, excludeGemKeywords);
	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i)))
		found++;
	if (GemMode == 'i')
		return((flucalc_group == FALSE && found) || (flucalc_SpecWords == found));
	else
		return((flucalc_group == TRUE && flucalc_SpecWords > found) || (found == 0));
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
	char word[BUFSIZ+1], *ws, *wm, *s;
	char lastSp[SPEAKERLEN];
	char isRepSeg, isWWRFound, ismWWRFound, isRepeatFound, isInternalPause, isFilterUttsOutputDepTier;
	char isOutputGem, isSwitch, isMultiLang, isUDMor;
	long stime, etime, uttCnt;
	double tNum;
	float lphrase_repets, lword_revis, lphrase_revis, lpauses_cnti, lfilled_pauses, lTD;
	float lprolongation, lbroken_word, lblock, lPWR, lphono_frag, lmWWR, lSLD;
	float lastEndTime, curEndTime, curBegTime, wBegTime, wEndTime, wLastEndTime;
	struct flucalc_speakers *ts;
	MORFEATS word_feats, *compd, *feat;
	IEWORDS *twd;

	uttCnt = 0L;
	isMultiLang = FALSE;
	langType = UNK;
	if (isPOSMatch) {
		fprintf(stderr,"From file <%s>\n", oldfname);
	}
	if (isNOptionSet == FALSE) {
		strcpy(flucalc_BBS, "@*&#");
		strcpy(flucalc_CBS, "@*&#");
	}
	if (flucalc_SpecWords) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
	isFilterUttsOutputDepTier = FALSE;
	ts = NULL;
	lRightspeaker = FALSE;
	curBegTime = -1.0;
	curEndTime = -1.0;
	lastEndTime = -1.0;
	found = 0;
	lastSp[0] = EOS;
	isSwitch = TRUE;
	isUDMor = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
 if (lineno > tlineno) {
	tlineno = lineno + 200;
 }
*/
		if (isWorTier == TRUE && *utterance->speaker == '*') {
			if (uS.partcmp(utterance->speaker,lastSp,FALSE,FALSE))
				isSwitch = FALSE;
			else
				isSwitch = TRUE;
			strcpy(lastSp, utterance->speaker);
			uS.remblanks(lastSp);
			if (curEndTime > -1)
				lastEndTime = curEndTime;
			i = FindBulletTime(TRUE, utterance->line, 0, &curBegTime, &curEndTime);
		}
		if (LocalTierSelect == TRUE) {
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
		} else {
			if (*utterance->speaker == '*') {
				i = isPostCodeFound(utterance->speaker, utterance->line);
				if ((PostCodeMode == 'i' && i == 1) || (PostCodeMode == 'e' && i == 5)) {
					lRightspeaker = FALSE;
					continue;
				} else
					lRightspeaker = TRUE;
			} else if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (flucalc_SpecWords && !strcmp(flucalc_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				flucalc_n_option = FALSE;
				strcpy(flucalc_BBS, "@BG:");
				strcpy(flucalc_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				flucalc_n_option = TRUE;
				strcpy(flucalc_BBS, "@G:");
				strcpy(flucalc_CBS, "@*&#");
			}
		}
		lphrase_repets = 0.0;
		lword_revis = 0.0;
		lphrase_revis = 0.0;
		lpauses_cnti = 0.0;
		lphono_frag = 0.0;
		lfilled_pauses = 0.0;
//TD = lphrase_repets+lword_revis+lphrase_revis+lpauses_cnti+lphono_frag+lfilled_pauses;

		lprolongation = 0.0;
		lbroken_word = 0.0;
		lblock = 0.0;
		lPWR = 0.0;
		lmWWR = 0.0;
//SLD = lprolongation+lbroken_word+lblock+lPWR+lmWWR;

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
							fprintf(stderr,"UNIX FLUCALC can not count syllables for French language.\n");
							fprintf(stderr,"Try adding +b option to command line for word count.\n");
							cutt_exit(0);
#else
							langType = FRA;
#endif
						} else if (!uS.mStrnicmp(utterance->line+j, "zho", 3)) {
#ifdef UNX
							fprintf(stderr,"UNIX FLUCALC can not count syllables for Chinese language.\n");
							fprintf(stderr,"Try adding +b option to command line for word count.\n");
							cutt_exit(0);
#else
							langType = ZHO;
#endif
						} else if (!uS.mStrnicmp(utterance->line+j, "eng", 3)) {
							langType = ENG;
//						} else if (!uS.mStrnicmp(utterance->line+j, "deu", 3)) {
//							langType = ENG; // 2024-10-28 until German rules are added
						}
					}
				}
			}
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
					uS.remblanks(utterance->line);
					flucalc_FindSpeaker(oldfname, templineC, utterance->line, 0L, FALSE);
				}
				if (isFilterUtts > 0) {
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				}
			}
			if (uS.partcmp(utterance->speaker,flucalc_BBS,FALSE,FALSE)) {
				if (flucalc_n_option) {
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
			} else if (found > 0 && uS.partcmp(utterance->speaker,flucalc_CBS,FALSE,FALSE)) {
				if (flucalc_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (flucalc_SpecWords)
								isOutputGem = FALSE;
							else {
								isOutputGem = TRUE;
							}
						}
					}
				}
			}
		} else if (*utterance->speaker == '*' && isOutputGem) {
			uttCnt++;
			isFilterUttsOutputDepTier = FALSE;
			strcpy(templineC, utterance->speaker);
			ts = flucalc_FindSpeaker(oldfname, templineC, NULL, uttCnt, TRUE);
			if (isMultiLang == TRUE && isWordMode == 0) {
				if (utterance->line[0] == '[' && utterance->line[1] == '-' && isSpace(utterance->line[2])) {
					for (j=3; isSpace(utterance->line[j]); j++) ;
					if (!uS.mStrnicmp(utterance->line+j, "fra", 3)) {
#ifdef UNX
						fprintf(stderr,"UNIX FLUCALC can not count syllables for French language.\n");
						fprintf(stderr,"Try adding +b option to command line for word count.\n");
						cutt_exit(0);
#else
						ts->isSyllableLang = TRUE;
						langType = FRA;
#endif
					} else if (!uS.mStrnicmp(utterance->line+j, "zho", 3)) {
#ifdef UNX
						fprintf(stderr,"UNIX FLUCALC can not count syllables for Chinese language.\n");
						fprintf(stderr,"Try adding +b option to command line for word count.\n");
						cutt_exit(0);
#else
						ts->isSyllableLang = TRUE;
						langType = ZHO;
#endif
					} else if (!uS.mStrnicmp(utterance->line+j, "eng", 3)) {
						langType = ENG;
						ts->isSyllableLang = TRUE;
//					} else if (!uS.mStrnicmp(utterance->line+j, "deu", 3)) {
//						langType = ENG;  // 2024-10-28 until German rules are added
//						ts->isSyllableLang = TRUE;
					} else {
						langType = UNK;
						ts->isSyllableLang = FALSE;
					}
				}
			}
			if (langType == UNK && isWordMode == 0) {
#ifdef UNX
				fprintf(stderr,"FLUCALC can only count syllables for English or French languages.\n");
				fprintf(stderr,"Can't determine the language of datafile for syllables count.\n");
				fprintf(stderr,"Please make sure that this datafile has correct @Languages: header.\n\n");
				fprintf(stderr,"Try adding +b option to command line for other languages.\n");
#else
				fprintf(stderr,"%c%cFLUCALC can only count syllables for English, French and Chinese languages.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
				fprintf(stderr,"%c%cPlease make sure that this datafile has correct @Languages: header.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
				fprintf(stderr,"%c%cTry adding +b option to command line for other languages.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
				cutt_exit(0);
			}
			if (ts != NULL) {
				for (i=0; utterance->line[i] != EOS; i++) {

					if (utterance->line[i] == HIDEN_C && isdigit(utterance->line[i+1])) {
						if (getMediaTagInfo(utterance->line+i, &stime, &etime)) {
							tNum = etime;
							tNum = tNum - stime;
							tNum = tNum / 1000.0000;
							ts->tm = ts->tm + roundFloat(tNum);
						}
					}

					if ((i == 0 || uS.isskip(utterance->line,i-1,&dFnt,MBF)) && utterance->line[i] == '+' &&
						uS.isRightChar(utterance->line,i+1,',',&dFnt, MBF) && ts->isPSDFound) {
						if (ts->morUtt > 0.0)
							ts->morUtt--;
						ts->isPSDFound = FALSE;
					}
				}
				for (i=0; i < WWRSQMAX; i++) {
					wwrSq[i] = 0;
				}
				wwrSqCnt = 0;
				isWWRFound = FALSE;
				ismWWRFound = FALSE;
				isInternalPause = FALSE;
				i = 0;
				while ((i=getword(utterance->speaker, utterance->line, word, &wi, i))) {
					isRepSeg = FALSE;
					if (word[0] != '[' && word[0] != '0') {
						for (j=0; word[j] != EOS; j++) {
							if (word[j] == ':') {
								ts->prolongation++;
								lprolongation++;
							} else if (word[j] == '^') {
								ts->broken_word++;
								lbroken_word++;
							} else if (UTF8_IS_LEAD((unsigned char)word[j]) && word[j] == (char)0xE2) {
								if (word[j+1] == (char)0x89 && word[j+2] == (char)0xA0) { // ≠
									ts->block++;
									lblock++;
								} else if (word[j+1] == (char)0x86 && word[j+2] == (char)0xAB) { // ↫ - - ↫
									isRepSeg = !isRepSeg;
									if (isRepSeg) {
										ts->PWRRU++;
										ts->PWR++;
										lPWR++;
// fprintf(stdout, "PWR=%s\n", word); // lxs
									}
								}
							} else if (word[j] == '&' && word[j+1] == '+') {
								ts->phono_frag++;
								lphono_frag++;
							} else if (word[j] == '&' && word[j+1] == '-') {
								ts->filled_pauses++;
								lfilled_pauses++;
							}
							if (isRepSeg && word[j] == '-') // ↫ - - ↫
								ts->PWRRU++;
						}
						if (word[0] != '(')
							isInternalPause = TRUE;
					}
					if (!strcmp(word, "[/]")) {
						if (isOnlyOneWordPreCode(utterance->line, wi)) {
							if (isWWRFound == FALSE) {
								ts->WWR++;
								wwrSq[wwrSqCnt] = set_WWR_to_1(wwrSq[wwrSqCnt]);
// 2022-08-29 								isWWRFound = TRUE;
							}
							ts->WWRRU++;
							wwrSq[wwrSqCnt] = set_WWRRU_to_1(wwrSq[wwrSqCnt]);
							if (isMonoWordPreCode(utterance->line, wi)) {
								if (ismWWRFound == FALSE) {
#ifdef DEBUGmWWR
fprintf(stdout, "1 (%d) mWWR=%s\n", wwrSqCnt, utterance->line+wi); // lxs
#endif
									ts->mWWR++;
									lmWWR++;
									wwrSq[wwrSqCnt] = set_mWWR_to_1(wwrSq[wwrSqCnt]);
// 2022-08-29									ismWWRFound = TRUE;
								}
								ts->mWWRRU++;
								wwrSq[wwrSqCnt] = set_mWWRRU_to_1(wwrSq[wwrSqCnt]);
							}
						} else {
							wwrSq[wwrSqCnt] = set_phRep_to_1(wwrSq[wwrSqCnt]);
							ts->phrase_repets++;
							lphrase_repets++;
						}
						wwrSqCnt++;
					} else {
						if (!isPreviousItemWWR(utterance->line, wi)) {
							isWWRFound = FALSE;
							ismWWRFound = FALSE;
						}
						if (!strcmp(word, "[//]")) {
							if (isOnlyOneWordPreCode(utterance->line, wi)) {
								ts->word_revis++;
								lword_revis++;
								wwrSq[wwrSqCnt] = set_wRev_to_1(wwrSq[wwrSqCnt]);
							} else {
								ts->phrase_revis++;
								lphrase_revis++;
								wwrSq[wwrSqCnt] = set_phRev_to_1(wwrSq[wwrSqCnt]);
							}
							wwrSqCnt++;
						} else if (word[0] == '(' && uS.isPause(word, 0, NULL, &j)) {
							if (isInternalPause && !isEndTier(utterance->speaker, utterance->line, i)) {
								ts->pauses_cnti++;
								lpauses_cnti++;
								ts->pauses_dur = ts->pauses_dur + getPauseTimeDuration(word);
							}
						}
					}
				}
			}
			strcpy(spareTier1, utterance->line);
			lTD = lphrase_repets+lword_revis+lphrase_revis+lpauses_cnti+lphono_frag+lfilled_pauses;
			lSLD = lprolongation+lbroken_word+lblock+lPWR+lmWWR;
			if (isFilterUtts == 1) {
				if (lTD == 0.0 && lSLD == 0.0) {
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					isFilterUttsOutputDepTier = TRUE;
				}
			} else if (isFilterUtts == 2) {
				if (lTD > 0.0 && lSLD == 0.0) {
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					isFilterUttsOutputDepTier = TRUE;
				}
			} else if (isFilterUtts == 3) {
				if (lTD == 0.0 && lSLD > 0.0) {
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					isFilterUttsOutputDepTier = TRUE;
				}
			} else if (isFilterUtts == 4) {
				if (lTD > 0.0 && lSLD > 0.0) {
					printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					isFilterUttsOutputDepTier = TRUE;
				}
			}
		} else if (uS.partcmp(utterance->speaker,"%wor",FALSE,TRUE) && ts != NULL && isOutputGem) {
			if (isWorTier == TRUE) {
				ts->UTTDur = ts->UTTDur + (curEndTime - curBegTime);
				wLastEndTime = -1.0;
				i = 0;
				while (utterance->line[i] != EOS) {
					i = FindBulletTime(FALSE, utterance->line, i, &wBegTime, &wEndTime);
					if (wLastEndTime >= 0.0 && wBegTime > wLastEndTime)
						ts->IWDur = ts->IWDur + (wBegTime - wLastEndTime);
					wLastEndTime = wEndTime;
				}
				if (lastEndTime > 0.0 && curBegTime > lastEndTime) {
					if (isSwitch == TRUE) {
						ts->NumOfSwitch++;
						ts->SwitchDur = ts->SwitchDur + (curBegTime-lastEndTime);
					} else {
						ts->NumNoSwitch++;
						ts->NoSwitchDur = ts->NoSwitchDur + (curBegTime-lastEndTime);
					}
				}
			}
		} else if (*utterance->speaker == '%' && ts != NULL && isOutputGem) {
			ts->isMORFound = TRUE;
			if (isFilterUttsOutputDepTier) {
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			} else if (isPOSMatch) {
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
										if (is_WWRRU(wwrSq[wwrSqI])) {
											ws[wsLen] = EOS;
											strcat(ws, " {WWR-RU}");
											addToPOSList(ts, ws, wm);
											ws[wsLen] = EOS;
										}
										if (is_mWWRRU(wwrSq[wwrSqI])) {
											ws[wsLen] = EOS;
											strcat(ws, " {mono-WWR-RU}");
											addToPOSList(ts, ws, wm);
											ws[wsLen] = EOS;
										}
										if (is_phRep(wwrSq[wwrSqI])) {
											ws[wsLen] = EOS;
											strcat(ws, " {Phrase repetition}");
											addToPOSList(ts, ws, wm);
											ws[wsLen] = EOS;
										}
										if (is_wRev(wwrSq[wwrSqI])) {
											ws[wsLen] = EOS;
											strcat(ws, " {Word revision}");
											addToPOSList(ts, ws, wm);
											ws[wsLen] = EOS;
										}
										if (is_phRev(wwrSq[wwrSqI])) {
											ws[wsLen] = EOS;
											strcat(ws, " {Phrase revision}");
											addToPOSList(ts, ws, wm);
											ws[wsLen] = EOS;
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
			} else {
				i = 0;
				isRepeatFound = FALSE;
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
						if (strcmp(wm, "NO_POS") == 0 && strcmp(ws, "[/]") != 0)
							ts->allWords++;
						s = strchr(wm, '|');
						if (s != NULL) {
							if (strcmp(ws, "[/]") != 0) {
								ts->allWords++;
								ts->morWords++;
							}
							if (sampleType == 'w' && ts->morWords >= sampleSize)
								goto finish;
							strcpy(templineC2, wm);
							if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
								flucalc_error(FALSE);
							for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
								if (isEqual("noun", feat->pos) || isEqual("verb", feat->pos))
									isUDMor = TRUE;
								// counts open/closed BEG
								if (langType == ENG) {
									if (isEqual("adv:int", feat->pos)) {
										if (isDisfluency(ws))
											ts->closedClassD++;
//										else if (!isRepeatFound)
											ts->closedClassA++;
									} else if (isEqual("n", feat->pos) || isnEqual("n:", feat->pos, 2) || isAllv(feat) ||
											   isEqual("noun", feat->pos) || isEqual("verb", feat->pos) ||
											   isnEqual("cop", feat->pos, 3) || isEqual("adj", feat->pos)) {
										if (isDisfluency(ws))
											ts->openClassD++;
//										else if (!isRepeatFound)
											ts->openClassA++;
									} else if ((isEqual("adv", feat->pos) || isnEqual("adv:", feat->pos, 4)) &&
											   (isUDMor == TRUE || isEqualIxes("LY", feat->suffix, NUM_SUFF))) {
										if (isDisfluency(ws))
											ts->openClassD++;
//										else if (!isRepeatFound)
											ts->openClassA++;
									} else if (isEqual("co", feat->pos) == FALSE && isEqual("on", feat->pos) == FALSE) {
										if (isDisfluency(ws))
											ts->closedClassD++;
//										else if (!isRepeatFound)
											ts->closedClassA++;
									}
									if (isDisfluency(ws)) {
										if (!isRepeatFound)
											ts->allDisfluency++;
									}
								} else /* if (langType == FRA) */ {
									if (isEqual("noun", feat->pos) || isEqual("verb", feat->pos) ||
										isEqual("n", feat->pos)    || isnEqual("n:", feat->pos, 2) || 
										isEqual("v", feat->pos)    || isnEqual("v:", feat->pos, 2) || 
										isEqual("adj", feat->pos)  || isEqual("adv", feat->pos)) {
										if (isDisfluency(ws))
											ts->openClassD++;
//										else if (!isRepeatFound)
											ts->openClassA++;
									} else {
											ts->closedClassD++;
//										else if (!isRepeatFound)
											ts->closedClassA++;
									}
									if (isDisfluency(ws)) {
										if (!isRepeatFound)
											ts->allDisfluency++;
									}
								} 
								// counts open/closed END

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
/*
									else {
										if (feat->stem != NULL && feat->stem[0] != EOS) {
											syllCnt += countSyllables(feat->stem, wm);
										}
									}
*/
								}
							}
							freeUpFeats(&word_feats);
							if (strcmp(ws, "[/]") != 0) {
								if (syllCnt == 0) {
									syllCnt += countSyllables(ws, wm);
								}
								if (SyllWordsListFP != NULL) {
									rootWords = flucalc_tree(rootWords, ws, syllCnt);
								}
								ts->morSyllables += (float)syllCnt;
								if (sampleType == 's' && ts->morSyllables >= sampleSize) {
									ts->morSyllables = (float)sampleSize;
									goto finish;
								}
								isRepeatFound = FALSE;
							} else
								isRepeatFound = TRUE;
						} else if (!isPOSMatch && strcmp(ws, "[/]") != 0 && strcmp(wm, "NO_POS") != 0) {
							if (isTierContSymbol(wm, 0, FALSE))  //    +.  +/.  +/?  +//?  +...  +/.?   ===>   +,
								ts->isPSDFound = TRUE;
							for (j=0; wm[j] != EOS; j++) {
								if (flucalc_isUttDel(wm, j)) {
									ts->morUtt = ts->morUtt + (float)1.0;
									break;
								}
							}
						}
					}
				}
			}
/*
			i = 0;
			while ((i=getNextDepTierPair(utterance->line, word, templineC4, NULL, i)) != 0) {
				 if (strchr(word, '|') != NULL || strcmp(word, "NO_POS") == 0) {
					 wm = word;
					 ws = templineC4;
				 } else {
					 wm = templineC4;
					 ws = word;
				 }
				if (strchr(wm, '|') != NULL) {
					ts->allWords++;
					ts->morWords++;
// count Syllables beg
					strcpy(templineC2, wm);
					if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
						flucalc_error(FALSE);
					for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
						if (feat->compd != NULL) {
							for (compd=feat; compd != NULL; compd=compd->compd) {
								if (compd->stem != NULL && compd->stem[0] != EOS) {
									ts->morSyllables += countSyllables(compd->stem, wm);
								}
							}
						} else {
							if (feat->stem != NULL && feat->stem[0] != EOS) {
								ts->morSyllables += countSyllables(feat->stem, wm);
							}
						}
					}
					freeUpFeats(&word_feats);
// count Syllables end
				} else {
					if (isTierContSymbol(word, 0, FALSE))  //    +.  +/.  +/?  +//?  +...  +/.?   ===>   +,
						ts->isPSDFound = TRUE;
					for (j=0; word[j] != EOS; j++) {
						if (flucalc_isUttDel(word, j)) {
							ts->morUtt = ts->morUtt + (float)1.0;
							break;
						}
					}
				}
			}
*/
		}
	}
finish:
	if (!combinput) {
		flucalc_pr_result();
		rootLines = flucal_freeLines(rootLines, FALSE);
	}
}

static void flucalc_treeprint(struct flucalc_tnode *p) {
	if (p != NULL) {
		flucalc_treeprint(p->left);
		do {
			fprintf(SyllWordsListFP,"%s %d\n", p->word, p->count);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				flucalc_treeprint(p->right);
				break;
			}
			p = p->right;
		} while (1);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = FLUCALC;
	chatmode = CHAT_MODE;
	OnlydataLimit = 1;
	UttlineEqUtterance = FALSE;
	rootWords = NULL;
	isSyllWordsList = FALSE;
	SyllWordsListFP = NULL;
	bmain(argc,argv,flucalc_pr_result);
	rootLines = flucal_freeLines(rootLines, FALSE);
	if (SyllWordsListFP != NULL) {
		flucalc_treeprint(rootWords);
		fclose(SyllWordsListFP);
	}
	flucalc_freetree(rootWords);
	rootWords = NULL;
	sp_head = freespeakers(sp_head);
}
