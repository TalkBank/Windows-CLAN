/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/
/*
 MATTR:
 
 *CHA: 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25

 Number of frames/windows counted will be 16 consisting of 10 items each:

 01 02 03 04 05 06 07 08 09 10
 02 03 04 05 06 07 08 09 10 11
 03 04 05 06 07 08 09 10 11 12
 04 05 06 07 08 09 10 11 12 13
 05 06 07 08 09 10 11 12 13 14
 06 07 08 09 10 11 12 13 14 15
 07 08 09 10 11 12 13 14 15 16
 08 09 10 11 12 13 14 15 16 17
 09 10 11 12 13 14 15 16 17 18
 10 11 12 13 14 15 16 17 18 19
 11 12 13 14 15 16 17 18 19 20
 12 13 14 15 16 17 18 19 20 21
 13 14 15 16 17 18 19 20 21 22
 14 15 16 17 18 19 20 21 22 23
 15 16 17 18 19 20 21 22 23 24
 16 17 18 19 20 21 22 23 24 25

*/

#define CHAT_MODE 1
#include "cu.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main freq_main
#define call freq_call
#define getflag freq_getflag
#define init freq_init
#define usage freq_usage
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

#if defined(CLAN_SRV)
extern char SRV_PATH[];
extern char SRV_NAME[];
#endif

extern char R8;
extern char GExt[];
extern char R5_1;
extern char R6_freq;
extern char tct;
extern char y_option;
extern char Preserve_dir;
extern char stin_override;
extern char OverWriteFile;
extern char outputOnlyData;
extern char isLanguageExplicit;
extern char anyMultiOrder;
extern char onlySpecWsFound;
extern char isSpecRepeatArrow;
extern char linkDep2Other;
extern char isPostcliticUse;	/* if '~' found on %mor tier break it into two words */
extern char isPrecliticUse;		/* if '$' found on %mor tier break it into two words */
extern char puredata;
extern char cutt_isSearchForQuotedItems;
extern char morWordParse[];
extern struct tier *headtier;
extern struct IDtype *IDField;

#define ColLabels struct freq_ColList
struct freq_ColList {
	unsigned int totalCol;
	char *colName;
	unsigned int colRes;
	ColLabels *nextCol;
} ;

struct freq_lines {
	char isUsed;
	char *loc;
	char *tier;
	struct freq_lines *nextLine;
} ;

struct freq_where {
	unsigned int count;
	struct freq_lines *line;
	struct freq_where *nextMatch;
} ;

struct freq_tnode {
	char *word;
	struct freq_where *where;
	unsigned int count;
	struct freq_tnode *left;
	struct freq_tnode *right;
};

#define FREQSP struct freq_speakers_list
struct freq_speakers_list {
	char isSpeakerFound;
	char *fname;
	char *sp;
	char *ID;
	struct freq_tnode *words;
	char **list;
	struct freq_tnode *MATTRwords;
	long  MATTRdiff;
	float MATTR;
	int   NMATTRs;
	long total;				/* total number of words */
	long different;			/* number of different words */
	ColLabels *colList;
	FREQSP *next_sp;
} ;

static int  percent;
static int  frame_size;
static int  isCrossTabulation, isCTSpreadsheet;
static char percentC;
static char isChange_nomain;
static char isR6Legal;
static char revconc = 0;
static char isSort = FALSE;
static char capwd = 0;
static char isSearchForID = FALSE;
static char isHeaderSeleted = FALSE;
static char isSearchForCode;
static char zeroMatch = FALSE;
static char isSpreadsheetOnePerRow;
static char isSearchForSpeaker[64];
static char isCombineSpeakers = FALSE;
static char isMorTierFirst, isMorUsed;
static char isFoundTier;
static char isMultiWordsActual;
static char freq_FTime;
static char isLookAtSpeakerWord;
static char isRepeatSegment;
static struct freq_lines *rootLines;
static FREQSP *freq_head;
static FNType StatName[FNSize];
static FILE *StatFp;


void usage() {
	puts("FREQ creates a frequency word count");
	printf("Usage: freq [bN cN oN %s] filename(s)\n",mainflgs());
	puts("+bN: compute MATTR for frame size N");
	puts("+c : find capitalized words only");
	puts("+c1: find words with upper case letters in the middle only");
	puts("+c2: find matches for every string specified by +s option (default: only first match is counted)");
	puts("+c3: find multi-word groups anywhere and in any order on a tier");
	puts("+c4: find match only if tier consists solely of all words in multi-word group, even a single word");
	puts("+c5: when combined with +d7 option it will reverse tier's priority");
	fprintf(stdout, "+c6: count only repeat segments when searching for words with %c%c%c\n", 0xe2, 0x86, 0xAB);
	puts("+c7: for multi-word groups searches output actual words matched");
	puts("     default: display exactly what was specified in +s option, i.e. do not expand wildcards");
	puts("+o : sort output by descending frequency");
	puts("+o1: sort output by reverse concordance");
	puts("+o2: sort by reverse concordance of first word; non-CHAT, preserve the whole line");
	puts("+o3: combine selected speakers from each file into one results list for that file");
	mainusage(FALSE);
#ifdef UNX
	printf("DATA ASSOCIATED WITH CODES [/], [//], [///], [/-], [/?] IS EXCLUDED BY DEFAULT.\n");
	printf("TO INCLUDE THIS DATA PLEASE USE \"+r6\" OPTION.\n");
#else
	printf("%c%cDATA ASSOCIATED WITH CODES [/], [//], [///], [/-], [/?] IS EXCLUDED BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO INCLUDE THIS DATA PLEASE USE \"+r6\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	cutt_exit(0);
}

static struct freq_lines *freeLines(struct freq_lines *lines, char isJustOne) {
	struct freq_lines *t;

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

static struct freq_where *freeWhere(struct freq_where *where) {
	struct freq_where *t;

	while (where != NULL) {
		t = where;
		where = where->nextMatch;
		free(t);
	}
	return(NULL);
}

static void freq_freetree(struct freq_tnode *p) {
	if (p != NULL) {
		freq_freetree(p->left);
		freq_freetree(p->right);
		free(p->word);
		p->where = freeWhere(p->where);
		free(p);
	}
}

static ColLabels *freq_freeColList(ColLabels *p) {
	ColLabels *t;

	while (p != NULL) {
		t = p;
		p = p->nextCol;
		if (t->colName != NULL)
			free(t->colName);
		free(t);
	}
	return(NULL);
}

static FREQSP *freq_freeSpeakers(FREQSP *p) {
	int i;
	FREQSP *t;

	while (p != NULL) {
		t = p;
		p = p->next_sp;
		if (t->fname != NULL)
			free(t->fname);
		if (t->sp != NULL)
			free(t->sp);
		if (t->ID)
			free(t->ID);
		if (t->list != NULL) {
			for (i=0; i < frame_size; i++) {
				if (t->list[i] != NULL)
					free(t->list[i]);
			}
			free(t->list);
		}
		freq_freetree(t->words);
		freq_freeColList(t->colList);
		free(t);
	}
	return(NULL);
}

static void freq_overflow() {
	fprintf(stderr,"Freq: no more memory available.\n");
	freq_head = freq_freeSpeakers(freq_head);
	cutt_exit(0);
}

static struct freq_tnode *freq_talloc(char *word, unsigned int count) {
	struct freq_tnode *p;

	if ((p=NEW(struct freq_tnode)) == NULL)
		freq_overflow();
	p->word = word;
	p->count = count;
	return(p);
}

static struct freq_lines *AddNewLine(struct freq_lines *lines, char *loc, char *tier) {
	struct freq_lines *p;

	if ((p=NEW(struct freq_lines)) == NULL)
		freq_overflow();
	if (loc != NULL) {
		if ((p->loc=(char *)malloc(strlen(loc)+1)) == NULL)
			freq_overflow();
		strcpy(p->loc, loc);
	} else
		p->loc = NULL;
	if ((p->tier=(char *)malloc(strlen(tier)+1)) == NULL)
		freq_overflow();
	strcpy(p->tier, tier);
	p->nextLine = lines;
	p->isUsed = FALSE;
	lines = p;

	return(lines);
}

static int isFoundWildCard(char isFix) {
	char *w;
	IEWORDS *twd;

	for (twd=wdptr; twd != NULL; twd = twd->nextword) {
		for (w=twd->word; *w; w++) {
			if (*w == '\\' && *(w+1) != EOS) {
				if (isFix)
					strcpy(w, w+1);
				else
					w++;
			} else if (*w == '*' || *w == '%' || *w == '_')
				return(TRUE);
		}
	}
	return(FALSE);
}

static const char *percentToStr(char percentC) {
	if (percentC == 1)
		return("<");
	else if (percentC == 2)
		return("<=");
	else if (percentC == 3)
		return("=");
	else if (percentC == 4)
		return("=>");
	else if (percentC == 5)
		return(">");
	return("");
}

void init(char first) {
	long i;
	char ts[BUFSIZ];
	IEWORDS *twd;

	if (first) {
		isSort = FALSE;
		capwd = 0;
		revconc = 0;
		isSearchForID = FALSE;
		isHeaderSeleted = FALSE;
		zeroMatch = FALSE;
		isMorUsed = FALSE;
		isMorTierFirst = TRUE;
		isCombineSpeakers = FALSE;
		isSearchForSpeaker[0] = EOS;
		isSearchForCode = FALSE;
		isR6Legal = TRUE;
		isChange_nomain = TRUE;
		frame_size = 0;
		percent = 0;
		percentC = 0;
		StatFp = NULL;
		freq_FTime = TRUE;
		isCrossTabulation = 0;
		isSpreadsheetOnePerRow = FALSE;
		isCTSpreadsheet = FALSE;
		isMultiWordsActual = FALSE;
		rootLines = NULL;
		freq_head = NULL;
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
		addword('\0','\0',"+.");
		addword('\0','\0',"+?");
		addword('\0','\0',"+!");
		maininitwords();
		mor_initwords();
		gra_initwords();
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		LocalTierSelect = TRUE;
		FilterTier = 1;
		isLookAtSpeakerWord = FALSE;
		isRepeatSegment = FALSE;
	} else {
		if (freq_FTime) {
			freq_FTime = FALSE;
			if (onlySpecWsFound && mwdptr == NULL && wdptr == NULL) {
				fprintf(stderr,"For +c4 option to work correctly -\n");
				fprintf(stderr,"    please use +s option to specify a multi-word list (for example: +s\"word1 word2\")\n");
				cutt_exit(0);
			}
			if (anyMultiOrder && mwdptr == NULL) {
				fprintf(stderr,"For +c3 option to work correctly -\n");
				fprintf(stderr,"    please use +s option to specify a multi-word list (for example: +s\"word1 word2\")\n");
				cutt_exit(0);
			}
			if (isRepeatSegment && !isSpecRepeatArrow) {
				fprintf(stderr,"+c6 option only works if +s option with %c%c%c character specified\n", 0xe2, 0x86, 0xAB);
				cutt_exit(0);
			}
			if (capwd == 3) {
				if (wdptr == NULL || !isFoundWildCard(FALSE)) {
					fprintf(stderr,"For +c2 option to work correctly -\n");
					fprintf(stderr,"    please use +s option to specify a list of string with wild cards (* %% _)\n");
					cutt_exit(0);
				}
			}
			if (zeroMatch) {
				if (wdptr != NULL && isFoundWildCard(TRUE)) {
					fprintf(stderr,"Can't use +d5 option if a word or words specified with +s option have wild cards (* %% _) or duplicates\n");
					cutt_exit(0);
				}
				if (wdptr == NULL && !isMORSearch() && !isGRASearch() && !isLangSearch()) {
					fprintf(stderr,"For +d5 option to work correctly - please use +s option to specify a list of words\n");
					fprintf(stderr,"If your search does not involve looking for word explicitly spelled out, then do not use +d5 option\n");
					cutt_exit(0);
				}
			}
			if (mwdptr != NULL) {
				if (capwd == 3) {
					fprintf(stderr,"The +c2 option can't be used with multi-word search specified with +s option\n");
					cutt_exit(0);
				}
				if (revconc) {
					fprintf(stderr,"The +o1 or +o2 option can't be used with multi-word search specified with +s option\n");
					cutt_exit(0);
				}
			}
			if (onlydata == 8) {
				if (revconc == 2) {
					fprintf(stderr,"The +d7 can't be used with +o2 option\n");
					cutt_exit(0);
				}
				if (!chatmode) {
					fprintf(stderr,"The +d7 can't be used with +y or +y1 option\n");
					cutt_exit(0);
				}
				if (mwdptr != NULL) {
					fprintf(stderr,"The +d7 can't be used with multi-word search specified with +s option\n");
					cutt_exit(0);
				}
				linkDep2Other = TRUE;
				if (isGRASearch() && !isMORSearch()) {
					strcpy(ts, "m|*");
					ts[0] = '\002';
					addword('s','i',ts);
				}
				if (fDepTierName[0] == EOS && chatmode) {
					if (isGRASearch()) {
						strcpy(fDepTierName, "%mor:");
						strcpy(sDepTierName, "%gra:");
					} else
						strcpy(fDepTierName, "%mor:");
				}
				if (fDepTierName[0] != EOS)
					maketierchoice(fDepTierName,'+',FALSE);
				if (sDepTierName[0] != EOS)
					maketierchoice(sDepTierName,'+',FALSE);
				FilterTier = 2;
				nomain = TRUE;
			} else if (isCrossTabulation > 0) {
				if (percent > 0) {
					fprintf(stderr,"The +d8 can't be used with +d%s%d option\n", percentToStr(percentC), percent);
					cutt_exit(0);
				}
				if (zeroMatch) {
					fprintf(stderr, "The +d8 can't be used with +d5 option.\n");
					cutt_exit(0);
				}
				if (onlydata == 3) {
					isCTSpreadsheet = TRUE;
				} else if (onlydata > 0) {
					fprintf(stderr,"The +d8 can't be used with any other +d option\n");
					cutt_exit(0);
				}
				if (capwd == 3) {
					fprintf(stderr,"The +d8 can't be used with +c2 option\n");
					cutt_exit(0);
				}
				if (revconc) {
					fprintf(stderr,"The +d8 can't be used with +o1 or +o2 option\n");
					cutt_exit(0);
				}
				if (!chatmode) {
					fprintf(stderr,"The +d8 can't be used with +y or +y1 option\n");
					cutt_exit(0);
				}
				if (mwdptr != NULL) {
					fprintf(stderr,"The +d8 can't be used with multi-word search specified with +s option\n");
					cutt_exit(0);
				}
				if (fDepTierName[0] == EOS || sDepTierName[0] == EOS) {
					fprintf(stderr,"Please specify two dependent tier for +d8 option to work.\n");
					cutt_exit(0);
				}
				onlydata = 9;
				if (isCTSpreadsheet) {
					AddCEXExtension = ".xls";
					stout = FALSE;
					OverWriteFile = TRUE;
					outputOnlyData = TRUE;
				} else
					puredata = 1;
				uS.shiftright(fDepTierName, 1);
				uS.shiftright(sDepTierName, 1);
				fDepTierName[0] = EOS;
				sDepTierName[0] = EOS;
				FilterTier = 2;
				nomain = TRUE;
			} else {
				if (!isMorTierFirst) {
					fprintf(stderr,"The +c5 option can only be used with +d7 option\n");
					cutt_exit(0);
				}
				if ((isMORSearch() || isMultiMorSearch) && chatmode) {
					nomain = TRUE;
					maketierchoice("%mor",'+',FALSE);
				} else if (R8) {
					nomain = TRUE;
					maketierchoice("%mor",'+',FALSE);
				}
				if ((isGRASearch() || isMultiGraSearch) && chatmode) {
					nomain = TRUE;
					maketierchoice("%gra",'+',FALSE);
				}
			}
			if (isLanguageExplicit == 1 && chatmode) {
				strcpy(ts, "%mor:");
				if (checktier(ts)) {
					if (!R8)
						linkMain2Mor = TRUE;
					FilterTier = 2;
					nomain = TRUE;
				}
			}
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '!' ||
					GlobalPunctuation[i] == '?' ||
					GlobalPunctuation[i] == '.') 
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}
			if (onlydata == 7)
				FilterTier = 2;
			if (!linkDep2Other) {
				fDepTierName[0] = EOS;
				sDepTierName[0] = EOS;
			}
			if (nomain || isMORSearch() || isGRASearch() || isMultiMorSearch || isMultiGraSearch)
				isR6Legal = FALSE;
			if (revconc)
				nomap = TRUE;
			if (isMORSearch())
				isMorUsed = TRUE;
			if (zeroMatch && isSearchForSpeaker[0] != EOS) {
				maketierchoice(isSearchForSpeaker,'+',FALSE);
			}
			if (R6 && isSearchForCode)
				R6_freq = TRUE;
			if (nomain && isHeaderSeleted) {
				char tst[3];
				strcpy(tst, "0*");
				freedefwdptr(tst);
				strcpy(tst, "&*");
				freedefwdptr(tst);
				strcpy(tst, "+*");
				freedefwdptr(tst);
				strcpy(tst, "-*");
				freedefwdptr(tst);
				strcpy(tst, "#*");
				freedefwdptr(tst);
			}
		}
		if (onlydata == 3 || onlydata == 4) {
			if (StatFp == NULL) {
				maketierchoice("@ID:",'+',FALSE);
				if (Preserve_dir)
					strcpy(StatName, wd_dir);
				else
					strcpy(StatName, od_dir);
				addFilename2Path(StatName, "stat");
				strcat(StatName, GExt);
				if ((StatFp=openwfile("", StatName, NULL)) == NULL) {
					fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",StatName);
					freq_head = freq_freeSpeakers(freq_head);
					cutt_exit(0);
				}
			}
		}
	}

	if (!combinput || first) {
		if (onlydata == 3 || onlydata == 4) {
			combinput = FALSE;
			stout = TRUE;
		}
		freq_head = NULL;
		if (!chatmode) {
			isLookAtSpeakerWord = TRUE;
			 if (WordMode == 'i' || WordMode == 'I') {
				for (twd=wdptr; twd != NULL; twd = twd->nextword) {
					strcpy(ts, "*:");
					if (uS.patmat(ts, twd->word)) {
						isLookAtSpeakerWord = FALSE;
						break;
					}
				}
			}
		} else
			isLookAtSpeakerWord = FALSE;
	}
}

void getflag(char *f, char *f1, int *i) {
	char *t, tp;
	UTTER *temp;

	f++;
	switch(*f++) {
		case 'b':
			if (*f == EOS) {
				fprintf(stderr,"Please specify the frame size with +b option.\n");
				cutt_exit(0);
			}
			frame_size = atoi(f);
			if (frame_size <= 0) {
				fprintf(stderr,"Please specify the frame size greater than zero with +b option.\n");
				cutt_exit(0);
			}
			break;
		case 'c':
			if (*f == EOS)
				capwd = 1;
			else if (*f == '0')
				capwd = 1;
			else if (*f == '1')
				capwd = 2;
			else if (*f == '2')
				capwd = 3;
			else if (*f == '3')
				anyMultiOrder = TRUE;
			else if (*f == '4')
				onlySpecWsFound = TRUE;
			else if (*f == '5')
				isMorTierFirst = FALSE;
			else if (*f == '6')
				isRepeatSegment = TRUE;
			else if (*f == '7')
				isMultiWordsActual = TRUE;
			else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			nomap = TRUE;
			break;
		case 'o':
			if (*f == EOS || *f == '0')
				isSort = TRUE;
			else if (*f == '1')
				revconc = 1;
			else if (*f == '2') {
				FilterTier = 0;
				revconc = 2;
				y_option = 0;
				chatmode = 0;
				nomain = 0;
				temp = utterance;
				do {
					*temp->speaker = EOS;
					temp = temp->nextutt;
				} while (temp != utterance) ;
			} else if (*f == '3') {
				isCombineSpeakers = TRUE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'd':
			t = f;
			tp = FALSE;
			if ((*t == '<' && *(t+1) == '=') || (*t == '=' && *(t+1) == '<')) {
				percentC = 2;
				t += 2;
				tp = TRUE;
			} else if ((*t == '>' && *(t+1) == '=') || (*t == '=' && *(t+1) == '>')) {
				percentC = 4;
				t += 2;
				tp = TRUE;
			} else if (*t == '<') {
				percentC = 1;
				t++;
				tp = TRUE;
			} else if (*t == '=') {
				percentC = 3;
				t++;
				tp = TRUE;
			} else if (*t == '>') {
				percentC = 5;
				t++;
				tp = TRUE;
			}
			if (tp == TRUE) {
				if (zeroMatch) {
					fprintf(stderr, "+d%s%d option can't be used with +d5 option.\n", percentToStr(percentC), percent);
					cutt_exit(0);
				}
				if (*t == EOS || !isdigit(*t)) {
					fprintf(stderr,"Invalid argument for option: %s\n", f-2);
					fprintf(stderr,"Please specify percentage value\n");
					cutt_exit(0);
				}
				percent = atoi(t);
				if (onlydata != 3)
					onlydata = 4;
			} else {
				if (*f == '2' && *(f+1) == '0') {
					isSpreadsheetOnePerRow = TRUE;
					onlydata = 3;
				} else if (*f == '5') {
					if (percent > 0) {
						fprintf(stderr, "+d5 option can't be used with +d%s%d option.\n", percentToStr(percentC), percent);
						cutt_exit(0);
					}
					zeroMatch = TRUE;
				} else if (*f == '8') {
					isCrossTabulation = 1;
				} else {
					if (*f == '1')
						isCombineSpeakers = TRUE;
					if (*f == '6' && mwdptr == NULL)
						R5_1 = TRUE;
					if (*f == '7') {
						linkDep2Other = TRUE;
					} else
						linkDep2Other = FALSE;
					maingetflag(f-2,f1,i);
					if (percent > 0 && onlydata != 3 && onlydata != 4) {
						percent = 0;
						percentC = 0;
					}
				}
			}
			break;
		case 't':
			if (*(f-2)=='+' && *f=='@' && (*(f+1)=='I' || *(f+1)=='i') && (*(f+2)=='D' || *(f+2)=='d'))
				isSearchForID = TRUE;
			if (*(f-2)=='+' && *f=='@')
				isHeaderSeleted = TRUE;
			if (*(f-2) == '+' && *f == '%') {
				if (isChange_nomain)
					nomain = TRUE;
				if (uS.mStrnicmp(f, "%mor", 4) == 0) {
					isMorUsed = TRUE;
				}
				if (fDepTierName[0] == EOS) {
					strcpy(fDepTierName, f);
				} else if (sDepTierName[0] == EOS) {
					strcpy(sDepTierName, f);
				}
				maingetflag(f-2,f1,i);
			} else if (*(f-2) == '+' && *f == '*' && *(f+1) == EOS) {
				isChange_nomain = FALSE;
				nomain = FALSE;
			} else
				maingetflag(f-2,f1,i);
			break;
		case 's':
			if (*(f-2) == '+' && *f == '\\' && *(f+1) == '*' && (*(f+2) == '*' || isalpha(*(f+2)))) {
				strncpy(isSearchForSpeaker, f+1, 63);
				isSearchForSpeaker[63] = EOS;
			}
			if (*(f-2) == '+' && (*f == '[' || *f == '<'))
				isSearchForCode = TRUE;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char *RevWd(char *s) {
	register int i;
	register int j;
	register char c;

	if (revconc == 2) {
		for (j=0; !isSpace(s[j]) && s[j] != EOS; j++) ;
		if (j > 0)
			j--;
	} else {
		j = strlen(s) - 1;
	}
	for (i=0; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
	return(s);
}

static char *freq_strsave(char *s) {
	char *p;

	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		freq_overflow();
	return(p);
}

static void CleanUpLine(char *st) {
	int pos;

	for (pos=0; st[pos] != EOS; ) {
		if (st[pos] == '\n' || st[pos] == '\t')
			st[pos] = ' ';
		if ((isSpace(st[pos]) || st[pos] == '\n') && (isSpace(st[pos+1]) || st[pos+1] == EOS))
			strcpy(st+pos, st+pos+1);
		else
			pos++;
	}
}

static struct freq_where *freq_AddNewMatch(struct freq_where *where, struct freq_lines *line) {
	struct freq_where *p;

	if (where == NULL) {
		if ((p=NEW(struct freq_where)) == NULL)
			freq_overflow();
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
		if ((p->nextMatch=NEW(struct freq_where)) == NULL)
			freq_overflow();
		p = p->nextMatch;
	}
	p->nextMatch = NULL;
	p->count = 1;
	p->line = line;
	line->isUsed = TRUE;
	return(where);
}

static void freq_addlineno(struct freq_tnode *p, struct freq_lines *line) {
	p->count++;
	if (isCrossTabulation > 0 && line != NULL) {
		for (; isCrossTabulation > 1 && line != NULL; line=line->nextLine) {
			p->where = freq_AddNewMatch(p->where, line);
			isCrossTabulation--;
		}
		isCrossTabulation = 1;
	} else if ((onlydata == 1 || onlydata == 7 || onlydata == 8) && line != NULL) {
		p->where = freq_AddNewMatch(p->where, line);
	}
}

static char isSpeakerWord(char *w) {
	if (isLookAtSpeakerWord && w[0] == '*' && w[strlen(w)-1] == ':')
		return(TRUE);
	return(FALSE);
}

static int freq_strcmp(const char *st1, const char *st2) {
	if (revconc) {
		return(uS.mStricmp(st1,st2));
	} else {
		return(strcmp(st1,st2));
	}
}

static struct freq_tnode *freq_tree(struct freq_tnode *p, FREQSP *ts, char *w, struct freq_lines *line) {
	int cond;
	struct freq_tnode *t = p;

	if (p == NULL) {
		if (!isSpeakerWord(w))
			ts->different++;
		p = freq_talloc(freq_strsave(w), 0);
		p->where = NULL;
		freq_addlineno(p, line);
		p->left = p->right = NULL;
	} else if ((cond=freq_strcmp(w,p->word)) == 0)
		freq_addlineno(p, line);
	else if (cond < 0)
		p->left = freq_tree(p->left, ts, w, line);
	else {
		for (; (cond=freq_strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0)
			freq_addlineno(p, line);
		else if (cond < 0)
			p->left = freq_tree(p->left, ts, w, line);
		else
			p->right = freq_tree(p->right, ts, w, line); /* if cond > 0 */
		return(t);
	}
	return(p);
}

static struct freq_tnode *freq_stree(struct freq_tnode *p, struct freq_tnode *m) {
	struct freq_tnode *t = p;

	if (p == NULL) {
		p = freq_talloc(freq_strsave(m->word), m->count);
		if (onlydata == 1 || onlydata == 7 || onlydata == 8 || isCrossTabulation) {
			p->where = m->where;
		} else
			p->where = NULL;
		p->left = p->right = NULL;
	} else if (p->count < m->count)
		p->left = freq_stree(p->left, m);
	else if (p->count == m->count) {
		for (; p->count>= m->count && p->right!= NULL; p=p->right) ;
		if (p->count < m->count)
			p->left = freq_stree(p->left, m);
		else
			p->right = freq_stree(p->right, m);
		return(t);
	} else
		p->right = freq_stree(p->right, m);
	return(p);
}

static struct freq_tnode *freq_MATTR_tree(struct freq_tnode *p, FREQSP *ts, char *w) {
	int cond;

	if (p == NULL) {
		ts->MATTRdiff++;
		p = freq_talloc(freq_strsave(w), 1);
		p->where = NULL;
		p->left = p->right = NULL;
	} else if ((cond=freq_strcmp(w,p->word)) == 0)
		p->count++;
	else if (cond < 0)
		p->left = freq_MATTR_tree(p->left, ts, w);
	else
		p->right = freq_MATTR_tree(p->right, ts, w);
	return(p);
}

static void freq_printlines(struct freq_tnode *p) {
	struct freq_where *w;

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

static void freq_treeprint(struct freq_tnode *p) {
	if (p != NULL) {
		freq_treeprint(p->left);
		do {
			if (revconc)
				RevWd(p->word);
			if (onlydata < 2 || onlydata == 7 || onlydata == 8 || isCrossTabulation)
				fprintf(fpout,"%3u ",p->count);
			else if (onlydata == 3 || percent > 0)
				fprintf(StatFp,"%u ",p->count);

			if (onlydata == 1 || onlydata == 7 || onlydata == 8 || isCrossTabulation) {
				fprintf(fpout,"%-10s", p->word);
				freq_printlines(p);
			} else if (onlydata == 3 || percent > 0)
				fprintf(StatFp,"%s\n", p->word);
			else if (onlydata < 3 || onlydata > 5)
				fprintf(fpout,"%s\n", p->word);

			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				freq_treeprint(p->right);
				break;
			}
			p = p->right;
		} while (1);
	}
}

static void freq_sorttree(FREQSP *ts, struct freq_tnode *p) {
	if (p != NULL) {
		freq_sorttree(ts, p->left);
		ts->words = freq_stree(ts->words, p);
		freq_sorttree(ts, p->right);
		free(p->word);
		free(p);
	}
}

static char isRightUpper(char *word) {
	char *w;

	if (uS.isRightChar(word,0,'[',&dFnt,MBF))
		return(FALSE);
	w = strchr(word, '|');
	if (w != NULL)
		word = w;
	if (capwd == 1 && isupper((unsigned char)*word))
		return(TRUE);
	else if (capwd == 2) {
		w = strchr(word, '=');
		for (; *word != EOS && word != w; word++) {
			if (isupper((unsigned char)*word) && *(word-1) != '+')
				return(TRUE);
		}
		return(FALSE);
	} else
		return(FALSE);
}

static struct freq_tnode *freq_tree_add_zeros(struct freq_tnode *p, char *w) {
	int cond;
	struct freq_tnode *t = p;

	if (p == NULL) {
		p = freq_talloc(freq_strsave(w),0);
		p->where = NULL;
		p->left = p->right = NULL;
	} else if ((cond=freq_strcmp(w,p->word)) == 0) {
	} else if (cond < 0)
		p->left = freq_tree_add_zeros(p->left, w);
	else {
		for (; (cond=freq_strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
		} else if (cond < 0)
			p->left = freq_tree_add_zeros(p->left, w);
		else
			p->right = freq_tree_add_zeros(p->right, w); /* if cond > 0 */
		return(t);
	}
	return(p);
}

static void printCT(struct freq_tnode *p, ColLabels *colList) {
	float t;
	ColLabels *cl;
	struct freq_where *w;

	if (p != NULL) {
		printCT(p->left, colList);
		do {
			excelStrCell(fpout, p->word);
			t = (float)p->count;
			excelNumCell(fpout, "%.0f", t);
			for (cl=colList; cl != NULL; cl=cl->nextCol) {
				cl->colRes = 0;
			}
			for (w=p->where; w != NULL; w=w->nextMatch) {
				for (cl=colList; cl != NULL; cl=cl->nextCol) {
					if (strcmp(cl->colName, w->line->tier) == 0) {
						cl->colRes = w->count;
					}
				}
			}
			for (cl=colList; cl != NULL; cl=cl->nextCol) {
				t = (float)cl->colRes;
				excelNumCell(fpout, "%.0f", t);
			}
			excelRow(fpout, ExcelRowEnd);
			excelRow(fpout, ExcelRowStart);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				printCT(p->right, colList);
				break;
			}
			p = p->right;
		} while (1);
	}
}

static void freq_pr_result(void) {
	int   i;
	float t, t1;
	IEWORDS *twd;
	IEMWORDS *tmwd;
	FREQSP *ts;
	ColLabels *cl;

#if defined(CLAN_SRV)
	if (!combinput) {
		FNType *s;
		char isDataFound;

		isDataFound = FALSE;
		for (ts=freq_head; ts != NULL; ts=ts->next_sp) {
			if (ts->total > 0L) {
				isDataFound = TRUE;
				break;
			}
		}
		if (isDataFound) {
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
			fprintf(stdout,"<a href=\"http://%s/index.php?url=%s/%s\">From file \"%s\"</a>\n", SRV_NAME, SRV_PATH, s, s);
		}
	}
#endif // CLAN_SRV
	if (isCTSpreadsheet) {
		excelHeader(fpout, newfname, 80);
		for (ts=freq_head; ts != NULL; ts=ts->next_sp) {
			if (!ts->isSpeakerFound)
				continue;
			if (ts->total <= 0)
				continue;
			excelRow(fpout, ExcelRowStart);
			sprintf(templineC4, "Speaker: %s", ts->sp);
			excelStrCell(fpout, templineC4);
			excelRow(fpout, ExcelRowEnd);
			excelRow(fpout, ExcelRowStart);
			if (isMorTierFirst) {
				excelStrCell(fpout, fDepTierName+1);
				sprintf(templineC4, "Total %s", fDepTierName+1);
				excelStrCell(fpout, templineC4);
			} else {
				excelStrCell(fpout, sDepTierName+1);
				sprintf(templineC4, "Total %s", sDepTierName+1);
				excelStrCell(fpout, templineC4);
			}
			for (cl=ts->colList; cl != NULL; cl=cl->nextCol) {
				excelStrCell(fpout, cl->colName);
			}
			excelRow(fpout, ExcelRowEnd);
			excelRow(fpout, ExcelRowStart);
			printCT(ts->words, ts->colList);
			excelStrCell(fpout, "Total Cols");
			excelStrCell(fpout, "");
			for (cl=ts->colList; cl != NULL; cl=cl->nextCol) {
				t = (float)cl->totalCol;
				excelNumCell(fpout, "%.0f", t);
			}
			excelRow(fpout, ExcelRowEnd);
			excelRow(fpout, ExcelRowEmpty);
		}
		excelFooter(fpout);
	} else {
		for (ts=freq_head; ts != NULL; ts=ts->next_sp) {
			if (!ts->isSpeakerFound) {
				if (!zeroMatch || isSearchForSpeaker[0] == EOS)
					continue;
			}
#if defined(CLAN_SRV)
			if (ts->total > 0) {
				if (onlydata == 3 || onlydata == 4) {
					fprintf(StatFp,"@FILENAME:\n");
					fprintf(StatFp,"%s\n", ts->fname);
					fprintf(StatFp,"@ID:\n");
					if (ts->ID) {
						uS.remblanks(ts->ID);
						fprintf(StatFp, "%s\n", ts->ID);
					} else
						fprintf(StatFp,".|.|%s|.|.|.|.|.|.|.|\n", ts->sp);
					if (ts->isSpeakerFound)
						fprintf(StatFp,"@SPEAKER_FOUND@\n");
				} else if (!isCombineSpeakers && chatmode /* && freq_head->next_sp != NULL */) {
					fprintf(fpout, "Speaker: %s\n", ts->sp);
				} else if (onlydata == 2 && chatmode)
					fprintf(fpout, ";%%* Combined Speakers output:\n");
			}
#else // CLAN_SRV
			if (onlydata == 3 || onlydata == 4) {
				fprintf(StatFp,"@FILENAME:\n");
				fprintf(StatFp,"%s\n", ts->fname);
				fprintf(StatFp,"@ID:\n");
				if (ts->ID) {
					uS.remblanks(ts->ID);
					fprintf(StatFp, "%s\n", ts->ID);
				} else
					fprintf(StatFp,".|.|%s|.|.|.|.|.|.|.|\n", ts->sp);
				if (ts->isSpeakerFound)
					fprintf(StatFp,"@SPEAKER_FOUND@\n");
			} else if (!isCombineSpeakers && chatmode /* && freq_head->next_sp != NULL */) {
				fprintf(fpout, "Speaker: %s\n", ts->sp);
			} else if (onlydata == 2 && chatmode)
				fprintf(fpout, ";%%* Combined Speakers output:\n");
#endif // !CLAN_SRV
			if (zeroMatch) {
				for (twd=wdptr; twd != NULL; twd=twd->nextword) {
					if (revconc) {
						strcpy(templineC, twd->word);
						RevWd(templineC);
						ts->words = freq_tree_add_zeros(ts->words, templineC);
					} else
						ts->words = freq_tree_add_zeros(ts->words, twd->word);
				}
				for (tmwd=mwdptr; tmwd != NULL; tmwd=tmwd->nextword) {
					templineC[0] = EOS;
					for (i=0; i < tmwd->total && i < MULTIWORDMAX; i++) {
						if (templineC[0] != EOS)
							strcat(templineC, " ");
						strcat(templineC, tmwd->word_arr[i]);
					}
					ts->words = freq_tree_add_zeros(ts->words, templineC);
				}
			}
			if (isSort) {
				struct freq_tnode *p;
				p = ts->words;
				ts->words = NULL;
				freq_sorttree(ts, p);
				freq_treeprint(ts->words);
			} else
				freq_treeprint(ts->words);

			if (onlydata < 2 || onlydata >= 5) {
#if defined(CLAN_SRV)
				if (ts->total > 0) {
					fprintf(fpout,"------------------------------\n");
					fprintf(fpout,"%5ld  Total number of different item types used\n", ts->different);
					fprintf(fpout,"%5ld  Total number of items (tokens)\n", ts->total);
					if (ts->total > 0) {
						t1 = (float)ts->total; t = (float)ts->different;
						fprintf(fpout,"%5.3f  Type/Token ratio\n", t/t1);
						if (!isMorUsed && chatmode != 0) {
							fprintf(fpout, "    This TTR number was not calculated on the basis of %%mor line forms.\n");
							fprintf(fpout, "    If you want a TTR based on lemmas, run FREQ on the %%mor line\n");
							fprintf(fpout, "    with option: +sm;*,o%%\n");
						}
					}
					if (frame_size > 0) {
						if (ts->NMATTRs > 0)
							fprintf(fpout,"%5.3f  MATTR\n", ts->MATTR/ts->NMATTRs);
						else
							fprintf(fpout,"-      MATTR\n");
					}
					fprintf(fpout, "\n");
				}
#else // CLAN_SRV
				fprintf(fpout,"------------------------------\n");
				fprintf(fpout,"%5ld  Total number of different item types used\n", ts->different);
				fprintf(fpout,"%5ld  Total number of items (tokens)\n", ts->total);
				if (ts->total > 0) {
					t1 = (float)ts->total; t = (float)ts->different;
					fprintf(fpout,"%5.3f  Type/Token ratio\n", t/t1);
					if (!isMorUsed && chatmode != 0) {
						fprintf(fpout, "    This TTR number was not calculated on the basis of %%mor line forms.\n");
						fprintf(fpout, "    If you want a TTR based on lemmas, run FREQ on the %%mor line\n");
						fprintf(fpout, "    with option: +sm;*,o%%\n");
					}
				}
				if (frame_size > 0) {
					if (ts->NMATTRs > 0)
						fprintf(fpout,"%5.3f  MATTR\n", ts->MATTR/ts->NMATTRs);
					else
						fprintf(fpout,"-      MATTR\n");
				}
				fprintf(fpout, "\n");
#endif // !CLAN_SRV
			} else if (onlydata == 3 || onlydata == 4) {
				fputs("@ST:\n", StatFp);
				fprintf(StatFp,"%ld %ld ", ts->different, ts->total);
				if (ts->total != 0) {
					t1 = (float)ts->total; t = (float)ts->different;
					fprintf(StatFp,"%.3f ", t/t1);
				} else
					fprintf(StatFp,"- ");
				if (frame_size > 0) {
					if (ts->NMATTRs > 0)
						fprintf(StatFp,"%.3f", ts->MATTR/ts->NMATTRs);
					else
						fprintf(StatFp,"-");
				}
				fprintf(StatFp,"\n");

			} else {
				if (ts->next_sp != NULL && !isCombineSpeakers)
					fprintf(fpout, "\n");
			}
		}
	}
	freq_head = freq_freeSpeakers(freq_head);
}

static void dealWithDiscontinuousWord(char *word, int i) {
	if (word[strlen(word)-1] == '-') {
		while ((i=getword(utterance->speaker, uttline, templineC4, NULL, i))) {
			if (templineC4[0] == '-' && !uS.isToneUnitMarker(templineC4)) {
				strcat(word, templineC4+1);
				break;
			}
		}
	}
}

static void addFName(FREQSP *ts) {
	char *s;

// ".xls"
	strcpy(FileName2, oldfname);
	s = strrchr(FileName2, PATHDELIMCHR);
	if (s == NULL) {
		s = strrchr(FileName2, '.');
		if (s != NULL)
			*s = EOS;
	} else {
		s++;
		*s = EOS;
	}
	s = strrchr(ts->fname, ',');
	if (s == NULL)
		s = ts->fname;
	else
		s++;
	if (uS.mStricmp(s, FileName2) != 0) {
		strcpy(templineC, ts->fname);
		strcat(templineC, ",");
		strcat(templineC, FileName2);
		free(ts->fname);
		if ((ts->fname=(char *)malloc(strlen(templineC)+1)) == NULL)
			freq_overflow();
		strcpy(ts->fname, templineC);
	}
}

static FREQSP *freq_FindSpeaker(char *sp, char *ID, char isSpeakerFound) {
	int  i;
	char *s;
	FREQSP *ts;

	uS.remblanks(sp);
	if (freq_head == NULL) {
		freq_head = NEW(FREQSP);
		ts = freq_head;
	} else {
		for (ts=freq_head; 1; ts=ts->next_sp) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
				ts->isSpeakerFound = isSpeakerFound;
				if (onlydata == 3 || onlydata == 4) {
					addFName(ts);
				}
				return(ts);
			}
			if (ts->next_sp == NULL)
				break;
		}
		ts->next_sp = NEW(FREQSP);
		ts = ts->next_sp;
	}
	if (ts == NULL)
		freq_overflow();
	ts->isSpeakerFound = isSpeakerFound;
	if (onlydata == 3 || onlydata == 4) {
		s = strrchr(oldfname, PATHDELIMCHR);
		if (s == NULL)
			s = oldfname;
		else
			s++;
		if ((ts->fname=(char *)malloc(strlen(s)+1)) == NULL)
			freq_overflow();
		strcpy(ts->fname, s);
		s = strrchr(ts->fname, '.');
		if (s != NULL)
			*s = EOS;
	} else
		ts->fname = NULL;
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL)
		freq_overflow();
	strcpy(ts->sp, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			freq_overflow();
		strcpy(ts->ID, ID);
	}
	if (frame_size > 0) {
		ts->list = (char **)malloc(frame_size * sizeof(char *));
		if (ts->list == NULL)
			freq_overflow();
		for (i=0; i < frame_size; i++) {
			ts->list[i] = NULL;
		}
	} else
		ts->list = NULL;
	ts->MATTRwords = NULL;
	ts->MATTRdiff = 0L;
	ts->MATTR = 0.0;
	ts->NMATTRs = 0;
	ts->words = NULL;
	ts->colList = NULL;
	ts->total = 0L;
	ts->different = 0L;
	ts->next_sp = NULL;
	return(ts);
}

static void comute_MATTR(FREQSP *ts, char *w) {
	int MATTRi;
	float tf;

	if (ts->list == NULL || isSpeakerWord(w))
		return;
	for (MATTRi=0; MATTRi < frame_size; MATTRi++) {
		if (ts->list[MATTRi] == NULL)
			break;
	}
	if (MATTRi == frame_size) {
		if (ts->list[0] != NULL) {
			free(ts->list[0]);
			ts->list[0] = NULL;
		}
		for (MATTRi=0; MATTRi < frame_size-1; MATTRi++) {
			ts->list[MATTRi] = ts->list[MATTRi+1];
		}
		ts->list[MATTRi] = (char *)malloc(strlen(w)+1);
		if (ts->list[MATTRi] == NULL)
			freq_overflow();
		strcpy(ts->list[MATTRi], w);
		MATTRi++;
	} else if (MATTRi < frame_size) {
		ts->list[MATTRi] = (char *)malloc(strlen(w)+1);
		if (ts->list[MATTRi] == NULL)
			freq_overflow();
		strcpy(ts->list[MATTRi], w);
		MATTRi++;
	}
	if (MATTRi == frame_size) {
		ts->MATTRdiff = 0L;
		ts->MATTRwords = NULL;
		for (MATTRi=0; MATTRi < frame_size; MATTRi++) {
			ts->MATTRwords = freq_MATTR_tree(ts->MATTRwords, ts, ts->list[MATTRi]);
		}
		freq_freetree(ts->MATTRwords);
		tf = (float)frame_size;
		ts->MATTR = ts->MATTR + (ts->MATTRdiff / tf);
		ts->NMATTRs++;
	}
}

static void defaultProcessWords(FREQSP *ts, char isLineNull) {
	register int i, wi;
	char  word[BUFSIZ];
	IEWORDS *twd;

	i = 0;
	while ((i=getword(utterance->speaker, uttline, word, &wi, i))) {
		if (word[0] == '-' && !uS.isToneUnitMarker(word) && !exclude(word))
			continue;
		if (onlydata == 7) {
			if (utterance->speaker[0] == '%' && isMORSearch()) {
				findWholeWord(utterance->line, wi, templineC4);
				uS.remFrontAndBackBlanks(templineC4);
				rootLines = AddNewLine(rootLines, NULL, templineC4);
			} else {
				findWholeScope(utterance->line, wi, templineC4);
				uS.remFrontAndBackBlanks(templineC4);
				CleanUpLine(templineC4);
				rootLines = AddNewLine(rootLines, NULL, templineC4);
			}
		}
		dealWithDiscontinuousWord(word, i);
		if (capwd == 3 && (WordMode == 'i' || WordMode == 'I')) {
			for (twd=wdptr; twd != NULL; twd = twd->nextword) {
				strcpy(templineC2, word);
				if (uS.patmat(templineC2, twd->word)) {
					uS.remblanks(templineC2);
					if (!isSpeakerWord(templineC2))
						ts->total++;
					if (revconc)
						RevWd(templineC2);
					if (isLineNull)
						ts->words = freq_tree(ts->words, ts, templineC2, NULL);
					else
						ts->words = freq_tree(ts->words, ts, templineC2, rootLines);
					if (frame_size > 0)
						comute_MATTR(ts, templineC2);
				}
			}						
		} else {
			if (onlydata == 7 || exclude(word) || (isRepeatSegment && *utterance->speaker == '*')) {
				uS.remblanks(word);
				if (capwd == 0 || isRightUpper(word)) {
					if (!isSpeakerWord(word))
						ts->total++;
					if (revconc)
						RevWd(word);
					if (isLineNull)
						ts->words = freq_tree(ts->words, ts, word, NULL);
					else
						ts->words = freq_tree(ts->words, ts, word, rootLines);
					if (frame_size > 0)
						comute_MATTR(ts, word);
				}
			}
		}
		if (onlydata == 7 && rootLines != NULL && !rootLines->isUsed)
			rootLines = freeLines(rootLines, TRUE);
	}
}

static void separateRepeatSegments(char *wline, int beg, int end) {
	int  i;
	char isArrowFound;

	for (i=beg; i < end; i++) {
		if (UTF8_IS_LEAD((unsigned char)wline[i]) && wline[i] == (char)0xE2 && wline[i+1] == (char)0x86 && wline[i+2] == (char)0xAB)
			break;
	}
	if (wline[i] != EOS) {
		isArrowFound = FALSE;
		for (i=beg; i < end; i++) {
			if (UTF8_IS_LEAD((unsigned char)wline[i]) && wline[i] == (char)0xE2 && wline[i+1] == (char)0x86 && wline[i+2] == (char)0xAB) {
				wline[i] = ' ';
				wline[i+1] = ' ';
				wline[i+2] = ' ';
				if (isArrowFound)
					isArrowFound = FALSE;
				else
					isArrowFound = TRUE;
			}
			if (wline[i] == '-' || isArrowFound == FALSE)
				wline[i] = ' ';
		}
	}
}

static void filterRepeatSegs(char *wline) {
	int pos, temp, i;
	char res, pa;

	pos = 0;
	do {
		for (; uS.isskip(wline,pos,&dFnt,MBF) && wline[pos] && !uS.isRightChar(wline,pos,'[',&dFnt,MBF); pos++) ;
		pa = FALSE;
		if (wline[pos]) {
			temp = pos;
			if (!nomap) {
				if (MBF) {
					if (my_CharacterByteType(wline, (short)pos, &dFnt) == 0)
						wline[pos] = (char)tolower((unsigned char)wline[pos]);
				} else {
					wline[pos] = (char)tolower((unsigned char)wline[pos]);
				}
			}
			if (uS.isRightChar(wline, pos, '[', &dFnt, MBF) && uS.isSqBracketItem(wline,pos+1,&dFnt,MBF)) {
				if (MBF) {
					for (pos++; !uS.isRightChar(wline, pos, ']', &dFnt, MBF) && wline[pos]; pos++) {
						if (!nomap && my_CharacterByteType(wline, (short)pos, &dFnt) == 0)
							wline[pos] = (char)tolower((unsigned char)wline[pos]);
					}
				} else {
					for (pos++; !uS.isRightChar(wline, pos, ']', &dFnt, MBF) && wline[pos]; pos++) {
						if (!nomap)
							wline[pos] = (char)tolower((unsigned char)wline[pos]);
					}
				}
				if (wline[pos])
					pos++;
			} else if (uS.isRightChar(wline, pos, '+', &dFnt, MBF) || uS.isRightChar(wline, pos, '-', &dFnt, MBF)) {
				do {
					for (pos++; wline[pos] != EOS && (!uS.isskip(wline,pos,&dFnt,MBF) ||
													  uS.isRightChar(wline,pos,'/',&dFnt,MBF) || uS.isRightChar(wline,pos,'<',&dFnt,MBF) ||
													  uS.isRightChar(wline,pos,'.',&dFnt,MBF) || uS.isRightChar(wline,pos,'!',&dFnt,MBF) ||
													  uS.isRightChar(wline,pos,'?',&dFnt,MBF) || uS.isRightChar(wline,pos,',',&dFnt,MBF)); pos++) {
					}
					if (uS.isRightChar(wline, pos, '[', &dFnt, MBF)) {
						if (uS.isSqBracketItem(wline, pos+1, &dFnt, MBF))
							break;
					} else
						break;
				} while (1) ;
			} else if (uS.isRightChar(wline, pos, '(', &dFnt, MBF) && uS.isPause(wline, pos, NULL,  &i)) {
				pos = i + 1;
				pa = TRUE;
			} else {
				do {
					if (UTF8_IS_LEAD((unsigned char)wline[pos])) {
						if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x9C) {
							if (!cutt_isSearchForQuotedItems) { // 2019-07-17
								pos += 3;
								break;
							}
						} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x9D) {
							if (!cutt_isSearchForQuotedItems) { // 2019-07-17
								pos += 3;
								break;
							}
						}
					} else if (wline[pos] == ',') {
						pos += 1;
						break;
					}
					for (pos++; !uS.isskip(wline,pos,&dFnt,MBF) && wline[pos]; pos++) {
						if (uS.isRightChar(wline, pos, '.', &dFnt, MBF) && uS.isPause(wline, pos, &i, NULL)) {
							pos = i;
							if (temp > pos)
								temp = pos;
							break;
						} else if (uS.IsUtteranceDel(wline, pos) == 2) {
							if (temp > pos)
								temp = pos;
							break;
						} else if (UTF8_IS_LEAD((unsigned char)wline[pos])) {
							if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x9C) {
								if (!cutt_isSearchForQuotedItems) { // 2019-07-17
									break;
								}
							} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x9D) {
								if (!cutt_isSearchForQuotedItems) { // 2019-07-17
									break;
								}
							}
						} else if (wline[pos] == ',' && !isdigit(wline[pos+1])) {
							break;
						}
						if (!nomap) {
							if (MBF) {
								if (my_CharacterByteType(wline, (short)pos, &dFnt) == 0)
									wline[pos] = (char)tolower((unsigned char)wline[pos]);
							} else {
								wline[pos] = (char)tolower((unsigned char)wline[pos]);
							}
						}
					}
					if (uS.isRightChar(wline, pos, '[', &dFnt, MBF)) {
						if (uS.isSqBracketItem(wline, pos+1, &dFnt, MBF))
							break;
					} else
						break;
				} while (1) ;
			}
			res = wline[pos];
			wline[pos] = EOS;
			if (!exclude(wline+temp)) {
				for (; temp < pos; temp++)
					wline[temp] = ' ';
			} else {
				separateRepeatSegments(wline, temp, pos);
				for (; temp < pos; temp++) {
					if (wline[temp] == EOS)
						wline[temp] = ' ';
				}
			}
			wline[pos] = res;
		}
	} while (wline[pos]) ;
}

static char isDuplicateStr(char *org, char *word) {
	char *s, *c;

	s = strrchr(org, ' ');
	if (s != NULL) {
		for (; isSpace(*s) || *s == '\n'; s++) ;
	} else
		s = org;
	if (!isPostcliticUse) {
		c = strrchr(s, '~');
		if (c != NULL) {
			if (uS.mStricmp(c+1, word) == 0)
				return(TRUE);
		}
	}
	if (!isPrecliticUse) {
		c = strrchr(s, '$');
		if (c != NULL) {
			if (uS.mStricmp(c+1, word) == 0)
				return(TRUE);
		}
	}

	return(FALSE);
}

static ColLabels *addToColArr(ColLabels *root, char *st) {
	ColLabels *nt, *tnt;

	if (root == NULL) {
		root = NEW(ColLabels);
		nt = root;
		nt->nextCol = NULL;
	} else {
		tnt= root;
		nt = root;
		while (1) {
			if (nt == NULL)
				break;
			if (strcmp(st,nt->colName) == 0) {
				nt->totalCol++;
				return(root);
			}
			if (strcmp(nt->colName, st) > 0)
				break;
			tnt = nt;
			nt = nt->nextCol;
		}
		if (nt == NULL) {
			tnt->nextCol = NEW(ColLabels);
			nt = tnt->nextCol;
			nt->nextCol = NULL;
		} else if (nt == root) {
			root = NEW(ColLabels);
			root->nextCol = nt;
			nt = root;
		} else {
			nt = NEW(ColLabels);
			nt->nextCol = tnt->nextCol;
			tnt->nextCol = nt;
		}
	}
	if ((nt->colName=(char *)malloc(strlen(st)+1)) == NULL) {
		fprintf(stderr,"Freq: no more memory available.\n");
		cutt_exit(1);
	}
	strcpy(nt->colName, st);
	nt->totalCol = 1;
	return(root);
}

void call() {
	register int i, wi, cntItems, res;
	char word[BUFSIZ], isRightSpeaker, *wF, *wS;
	char tisLookAtSpeakerWord, isAddContextArr;
	long tlineno = 0;
	IEWORDS *twd;
	IEMWORDS *tm;
	FREQSP *ts = NULL;
	MORFEATS word_feats;

	spareTier1[0] = EOS;
	spareTier2[0] = EOS;
	isRightSpeaker = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (isCrossTabulation && *utterance->speaker == '*' && ts != NULL) {
			i = 0;
			wi = 0;
			if (isMorTierFirst) {
				if (isCTSpreadsheet) {
					while ((wi=getword("%", spareTier2, templineC2, NULL, wi))) {
						ts->colList = addToColArr(ts->colList, templineC2);
					}
					wi = 0;
				}
				while ((i=getword("%", spareTier1, word, NULL, i))) {
					if (word[0] != EOS) {
						while ((wi=getword("%", spareTier2, templineC2, NULL, wi))) {
							isCrossTabulation++;
							rootLines = AddNewLine(rootLines, NULL, templineC2);
						}
						if (!isSpeakerWord(word))
							ts->total++;
						ts->words = freq_tree(ts->words, ts, word, rootLines);
						if (frame_size > 0)
							comute_MATTR(ts, word);
						if (rootLines != NULL && !rootLines->isUsed)
							rootLines = freeLines(rootLines, TRUE);
					}
				}
			} else {
				if (isCTSpreadsheet) {
					while ((wi=getword("%", spareTier1, templineC2, NULL, wi))) {
						ts->colList = addToColArr(ts->colList, templineC2);
					}
					wi = 0;
				}
				while ((i=getword("%", spareTier2, word, NULL, i))) {
					if (word[0] != EOS) {
						while ((wi=getword("%", spareTier1, templineC2, NULL, wi))) {
							isCrossTabulation++;
							rootLines = AddNewLine(rootLines, NULL, templineC2);
						}
						if (!isSpeakerWord(word))
							ts->total++;
						ts->words = freq_tree(ts->words, ts, word, rootLines);
						if (frame_size > 0)
							comute_MATTR(ts, word);
						if (rootLines != NULL && !rootLines->isUsed)
							rootLines = freeLines(rootLines, TRUE);
					}
				}
			}
			spareTier1[0] = EOS;
			spareTier2[0] = EOS;
		}

		if (!chatmode) {
			if (strncmp(uttline, ";%* ", 4) == 0 || uS.isUTF8(uttline) || uS.isInvisibleHeader(uttline))
				continue;
		} else if (*utterance->speaker == '@') {
			if (!checktier(utterance->speaker))
				continue;
		} else if (isRightSpeaker || *utterance->speaker == '*') {
			if (checktier(utterance->speaker)) {
				isRightSpeaker = TRUE;
			} else if (*utterance->speaker == '*') {
				isRightSpeaker = FALSE;
				continue;
			} else
				continue;
		} else
			continue;
		if (lineno > tlineno) {
			tlineno = lineno + 200;
#if !defined(CLAN_SRV)
			fprintf(stderr,"\r%ld ",lineno);
#endif
		}
		if (onlydata == 1) {
			strcpy(templineC4, utterance->speaker);
			strcat(templineC4, utterance->line);
			CleanUpLine(templineC4);
			sprintf(templineC3, "File \"%s\": line %ld.", oldfname, lineno);
			rootLines = AddNewLine(rootLines, templineC3, templineC4);
		}
		if (isCombineSpeakers) {
			strcpy(word, "*COMBINED*");
			ts = freq_FindSpeaker(word, NULL, TRUE);
		} else if (!chatmode) {
			strcpy(word, "GENERIC");
			ts = freq_FindSpeaker(word, NULL, TRUE);
		} else if (*utterance->speaker == '*') {
			strcpy(templineC2, utterance->speaker);
//			freq_checktier(templineC2);
			ts = freq_FindSpeaker(templineC2, NULL, (nomain == FALSE || tct == TRUE || isSearchForID == TRUE));
		}

		if (onlydata == 3 || onlydata == 4) {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC2, TRUE)) {
					uS.remblanks(utterance->line);
					ts = freq_FindSpeaker(templineC2, utterance->line, FALSE);
				}
				continue;
			} else if (*utterance->speaker == '*' && !isPostCodeFound(utterance->speaker, utterance->line)) {
				strcpy(templineC2, utterance->speaker);
				uS.remblanks(templineC2);
				tisLookAtSpeakerWord = isLookAtSpeakerWord;
				isLookAtSpeakerWord = TRUE;
				if (capwd == 0 || isRightUpper(templineC2)) {
					if (revconc)
						RevWd(templineC2);
					ts->words = freq_tree(ts->words, ts, templineC2, NULL);
					if (frame_size > 0)
						comute_MATTR(ts, templineC2);
				}
				isLookAtSpeakerWord = tisLookAtSpeakerWord;
			}
		} else if (!isCombineSpeakers && chatmode) {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC2, TRUE)) {
					uS.remblanks(utterance->line);
					ts = freq_FindSpeaker(templineC2, utterance->line, TRUE);
				}
			} else if (utterance->speaker[0] == '@') {
				strcpy(word, "*HEADER TIER(S)*");
				ts = freq_FindSpeaker(word, NULL, TRUE);
			}
		}
		if (*utterance->speaker == '*' && nomain)
			continue;
		if (ts == NULL)
			continue;
		isFoundTier = TRUE;
		if (isRepeatSegment && *utterance->speaker == '*') {
			filterRepeatSegs(uttline);
		}
		i = 0;
		if ((wdptr == NULL || WordMode == 'e') && mwdptr != NULL) {
			if (linkMain2Mor && strchr(uttline, dMarkChr) != NULL && uS.partcmp(utterance->speaker, "%mor:", FALSE, TRUE)) {
				removeMainTierWords(uttline);
			} else if (linkDep2Other && strchr(uttline, dMarkChr) != NULL) {
				if ((fDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,fDepTierName,FALSE,TRUE)) ||
					(sDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,sDepTierName,FALSE,TRUE))) {
					removeMainTierWords(uttline);
				}
			}
		} else if (revconc == 2) {
			if ((i=getword(utterance->speaker, uttline, word, &wi, i)) != 0) {
				if (onlydata == 7) {
					if (utterance->speaker[0] == '%' && isMORSearch()) {
						findWholeWord(utterance->line, wi, templineC4);
						uS.remFrontAndBackBlanks(templineC4);
						rootLines = AddNewLine(rootLines, NULL, templineC4);
					} else {
						findWholeScope(utterance->line, wi, templineC4);
						uS.remFrontAndBackBlanks(templineC4);
						CleanUpLine(templineC4);
						rootLines = AddNewLine(rootLines, NULL, templineC4);
					}
				}
				if (onlydata == 7 || exclude(word) || (isRepeatSegment && *utterance->speaker == '*')) {
					if (!isSpeakerWord(utterance->line))
						ts->total++;
					uS.remblanks(utterance->line);
					RevWd(utterance->line);
					ts->words = freq_tree(ts->words, ts, utterance->line, rootLines);
					if (frame_size > 0)
						comute_MATTR(ts, utterance->line);
				}
				if (onlydata == 7 && rootLines != NULL && !rootLines->isUsed)
					rootLines = freeLines(rootLines, TRUE);
			}
		} else if (onlydata == 8 && (uS.partcmp(utterance->speaker,fDepTierName,FALSE,TRUE) ||
									 (sDepTierName[0]!=EOS && uS.partcmp(utterance->speaker,sDepTierName,FALSE,TRUE)))) {
			if (strchr(uttline, dMarkChr) == NULL) {
				defaultProcessWords(ts, TRUE);
			} else {
				while ((i=getNextDepTierPair(uttline, word, templineC4, NULL, i)) != 0) {
					if (word[0] != EOS && templineC4[0] != EOS) {
						if (isMorTierFirst) {
							wF = word;
							wS = templineC4;
						} else {
							wF = templineC4;
							wS = word;
						}
						rootLines = AddNewLine(rootLines, NULL, wS);
						if (capwd == 3 && (WordMode == 'i' || WordMode == 'I')) {
							for (twd=wdptr; twd != NULL; twd = twd->nextword) {
								strcpy(templineC2, wF);
								if (uS.patmat(templineC2, twd->word)) {
									uS.remblanks(templineC2);
									if (!isSpeakerWord(templineC2))
										ts->total++;
									if (revconc)
										RevWd(templineC2);
									ts->words = freq_tree(ts->words, ts, templineC2, rootLines);
									if (frame_size > 0)
										comute_MATTR(ts, templineC2);
								}
							}
						} else {
							if (capwd == 0 || isRightUpper(wF)) {
								if (!isSpeakerWord(wF))
									ts->total++;
								if (revconc)
									RevWd(wF);
								ts->words = freq_tree(ts->words, ts, wF, rootLines);
								if (frame_size > 0)
									comute_MATTR(ts, wF);
							}
						}
						if (rootLines != NULL && !rootLines->isUsed)
							rootLines = freeLines(rootLines, TRUE);
					}
				}
			}
		} else if (isCrossTabulation) {
			if (fDepTierName[1] != EOS && uS.partcmp(utterance->speaker,fDepTierName+1,FALSE,TRUE)) {
				strcpy(spareTier1, uttline);
			}
			if (sDepTierName[1] != EOS && uS.partcmp(utterance->speaker,sDepTierName+1,FALSE,TRUE)) {
				strcpy(spareTier2, uttline);
			}
		} else if (linkMain2Mor && uS.partcmp(utterance->speaker, "%mor:", FALSE, TRUE)){
			if (strchr(uttline, dMarkChr) == NULL) {
				defaultProcessWords(ts, TRUE);
			} else {
				while ((i=getNextDepTierPair(uttline, word, templineC4, &wi, i)) != 0) {
					if (word[0] == '-' && !uS.isToneUnitMarker(word) && !exclude(word))
						continue;
					else if (word[0] != EOS && templineC4[0] != EOS) {
						if (onlydata == 7) {
							if (utterance->speaker[0] == '%' && isMORSearch()) {
								findWholeWord(utterance->line, wi, templineC4);
								uS.remFrontAndBackBlanks(templineC4);
								rootLines = AddNewLine(rootLines, NULL, templineC4);
							}
						}
						if (capwd == 3 && (WordMode == 'i' || WordMode == 'I')) {
							for (twd=wdptr; twd != NULL; twd = twd->nextword) {
								strcpy(templineC2, word);
								if (uS.patmat(templineC2, twd->word)) {
									uS.remblanks(templineC2);
									if (!isSpeakerWord(templineC2))
										ts->total++;
									if (revconc)
										RevWd(templineC2);
									ts->words = freq_tree(ts->words, ts, templineC2, rootLines);
									if (frame_size > 0)
										comute_MATTR(ts, templineC2);
								}
							}						
						} else {
							if (capwd == 0 || isRightUpper(word)) {
								if (!isSpeakerWord(word))
									ts->total++;
								if (revconc)
									RevWd(word);
								ts->words = freq_tree(ts->words, ts, word, rootLines);
								if (frame_size > 0)
									comute_MATTR(ts, word);
							}
						}
						if (onlydata == 7 && rootLines != NULL && !rootLines->isUsed)
							rootLines = freeLines(rootLines, TRUE);
					}
				}
			}
		} else {
			defaultProcessWords(ts, FALSE);
		}
		if (mwdptr != NULL) {
			for (tm=mwdptr; tm != NULL; tm=tm->nextword) {
				if (anyMultiOrder) {
					for (tm->cnt=0; tm->cnt < tm->total; tm->cnt++) {
						tm->isMatch[tm->cnt] = FALSE;
					}
				}
				tm->cnt = 0;
			}	
			cntItems = -1;
			if (onlySpecWsFound) {
				cntItems = 0;
				i = 0;
				while ((i=getword(utterance->speaker, uttline, word, &wi, i))) {
					if (wdptr == NULL || exclude(word) || (isRepeatSegment && *utterance->speaker == '*'))
						cntItems++;
				}
			}
			if (anyMultiOrder) {
				for (tm=mwdptr; tm != NULL; tm=tm->nextword) {
					i = 0;
					while ((i=getword(utterance->speaker, uttline, word, &wi, i))) {
						if (onlydata == 7) {
							if (utterance->speaker[0] == '%' && isMORSearch()) {
								findWholeWord(utterance->line, wi, templineC4);
								uS.remFrontAndBackBlanks(templineC4);
							} else {
								findWholeScope(utterance->line, wi, templineC4);
								uS.remFrontAndBackBlanks(templineC4);
								CleanUpLine(templineC4);
							}
						}
						for (tm->cnt=0; tm->cnt < tm->total; tm->cnt++) {
							if (word[0] != '-' && word[0] != '+' && tm->morwords_arr[tm->cnt] != NULL && isWordFromMORTier(word)) {
								strcpy(morWordParse, word);
								if (ParseWordMorElems(morWordParse, &word_feats) == FALSE)
									out_of_mem();
								if (matchToMorElems(tm->morwords_arr[tm->cnt], &word_feats, FALSE, FALSE, TRUE))
									res = 1;
								else
									res = 0;
								freeUpFeats(&word_feats);
							} else if (word[0] != '-' && word[0] != '+' && tm->grafptr_arr[tm->cnt] != NULL && isWordFromGRATier(word)) {
								strcpy(templineC1, word);
								cleanupGRAWord(templineC1);
								if (uS.patmat(templineC1, tm->grafptr_arr[tm->cnt]->word))
									res = 1;
								else
									res = 0;
							} else {
								res = uS.patmat(word, tm->word_arr[tm->cnt]);
							}
							if (res && tm->isMatch[tm->cnt] == FALSE) {
								strncpy(tm->matched_word_arr[tm->cnt], word, MULTIWORDMATMAX);
								tm->matched_word_arr[tm->cnt][MULTIWORDMATMAX] = EOS;
								tm->isMatch[tm->cnt] = TRUE;
								strncpy(tm->context_arr[tm->cnt], templineC4, MULTIWORDCONTMAX);
								tm->context_arr[tm->cnt][MULTIWORDCONTMAX] = EOS;
								break;
							}
						}
						for (tm->cnt=0; tm->cnt < tm->total; tm->cnt++) {
							if (tm->isMatch[tm->cnt] == FALSE)
								break;
						}
						if (tm->cnt == tm->total && (cntItems == -1 || tm->total == cntItems)) {
							templineC[0] = EOS;
							templineC4[0] = EOS;
							for (tm->cnt=0; tm->cnt < tm->total; tm->cnt++) {
								if (tm->cnt != 0) {
									strcat(templineC, " ");
									strcat(templineC4, " ");
								}
								if (isMultiWordsActual)
									strcat(templineC, tm->matched_word_arr[tm->cnt]);
								else
									strcat(templineC, tm->word_arr[tm->cnt]);
								strcat(templineC4, tm->context_arr[tm->cnt]);
							}
							if (onlydata == 7)
								rootLines = AddNewLine(rootLines, NULL, templineC4);
							ts->total++;
							ts->words = freq_tree(ts->words, ts, templineC, rootLines);
							if (frame_size > 0)
								comute_MATTR(ts, templineC);
							if (onlydata == 7 && rootLines != NULL && !rootLines->isUsed)
								rootLines = freeLines(rootLines, TRUE);
							for (tm->cnt=0; tm->cnt < tm->total; tm->cnt++) {
								tm->isMatch[tm->cnt] = FALSE;
							}
							tm->cnt = 0;
						}
					}
				}
			} else {
				for (tm=mwdptr; tm != NULL; tm=tm->nextword) {
					i = 0;
					while ((i=getword(utterance->speaker, uttline, word, &wi, i))) {
						if (onlydata == 7) {
							if (utterance->speaker[0] == '%' && isMORSearch()) {
								findWholeWord(utterance->line, wi, templineC4);
								uS.remFrontAndBackBlanks(templineC4);
							} else {
								findWholeScope(utterance->line, wi, templineC4);
								uS.remFrontAndBackBlanks(templineC4);
								CleanUpLine(templineC4);
							}
						}
						if (tm->cnt == 0)
							tm->i = i;
						if (word[0] != '-' && word[0] != '+' && tm->morwords_arr[tm->cnt] != NULL && isWordFromMORTier(word)) {
							strcpy(morWordParse, word);
							if (ParseWordMorElems(morWordParse, &word_feats) == FALSE)
								out_of_mem();
							if (matchToMorElems(tm->morwords_arr[tm->cnt], &word_feats, FALSE, FALSE, TRUE)) {
								res = 1;
								cleanUpMorWord(word, morWordParse);
							} else
								res = 0;
							freeUpFeats(&word_feats);
						} else if (word[0] != '-' && word[0] != '+' && tm->grafptr_arr[tm->cnt] != NULL && isWordFromGRATier(word)) {
							strcpy(templineC1, word);
							cleanupGRAWord(templineC1);
							if (uS.patmat(templineC1, tm->grafptr_arr[tm->cnt]->word))
								res = 1;
							else
								res = 0;
						} else {
							res = uS.patmat(word, tm->word_arr[tm->cnt]);
						}
						if (res) {
							strncpy(tm->matched_word_arr[tm->cnt], word, MULTIWORDMATMAX);
							tm->matched_word_arr[tm->cnt][MULTIWORDMATMAX] = EOS;
							tm->cnt++;
							strncpy(tm->context_arr[tm->cnt], templineC4, MULTIWORDCONTMAX);
							tm->context_arr[tm->cnt][MULTIWORDCONTMAX] = EOS;
							if (tm->cnt > tm->total) {
								i = tm->i;
								tm->cnt = 0;
								break;
							} else if (tm->cnt == tm->total && (cntItems == -1 || tm->total == cntItems)) {
								templineC[0] = EOS;
								templineC4[0] = EOS;
								for (tm->cnt=0; tm->cnt < tm->total; tm->cnt++) {
									if (tm->cnt != 0) {
										strcat(templineC, " ");
										if (isDuplicateStr(templineC4, tm->context_arr[tm->cnt+1]) == FALSE)
											isAddContextArr = TRUE;
										else
											isAddContextArr = FALSE;
										if (isAddContextArr)
											strcat(templineC4, " ");
									} else
										isAddContextArr = TRUE;
									if (isMultiWordsActual)
										strcat(templineC, tm->matched_word_arr[tm->cnt]);
									else
										strcat(templineC, tm->word_arr[tm->cnt]);
									if (isAddContextArr)
										strcat(templineC4, tm->context_arr[tm->cnt+1]);
								}
								tm->cnt = 0;
								if (onlydata == 7)
									rootLines = AddNewLine(rootLines, NULL, templineC4);
								ts->total++;
								ts->words = freq_tree(ts->words, ts, templineC, rootLines);
								if (frame_size > 0)
									comute_MATTR(ts, templineC);
								if (onlydata == 7 && rootLines != NULL && !rootLines->isUsed)
									rootLines = freeLines(rootLines, TRUE);
							}
						} else {
							i = tm->i;
							tm->cnt = 0;
						}
					}
				}
			}
		}
		if (onlydata == 1 && rootLines != NULL && !rootLines->isUsed)
			rootLines = freeLines(rootLines, TRUE);
	}

	if (isCrossTabulation && ts != NULL && spareTier1[0] != EOS && spareTier2[0] != EOS) {
		i = 0;
		wi = 0;
		if (isMorTierFirst) {
			while ((wi=getword("%", spareTier2, templineC2, NULL, wi))) {
				if (isCTSpreadsheet) {
					ts->colList = addToColArr(ts->colList, templineC2);
				}
			}
			wi = 0;
			while ((i=getword("%", spareTier1, word, NULL, i))) {
				if (word[0] != EOS) {
					while ((wi=getword("%", spareTier2, templineC2, NULL, wi))) {
						isCrossTabulation++;
						rootLines = AddNewLine(rootLines, NULL, templineC2);
					}
					if (!isSpeakerWord(word))
						ts->total++;
					ts->words = freq_tree(ts->words, ts, word, rootLines);
					if (frame_size > 0)
						comute_MATTR(ts, word);
					if (rootLines != NULL && !rootLines->isUsed)
						rootLines = freeLines(rootLines, TRUE);
				}
			}
		} else {
			while ((wi=getword("%", spareTier1, templineC2, NULL, wi))) {
				if (isCTSpreadsheet) {
					ts->colList = addToColArr(ts->colList, templineC2);
				}
			}
			wi = 0;
			while ((i=getword("%", spareTier2, word, NULL, i))) {
				if (word[0] != EOS) {
					while ((wi=getword("%", spareTier1, templineC2, NULL, wi))) {
						isCrossTabulation++;
						rootLines = AddNewLine(rootLines, NULL, templineC2);
					}
					if (!isSpeakerWord(word))
						ts->total++;
					ts->words = freq_tree(ts->words, ts, word, rootLines);
					if (frame_size > 0)
						comute_MATTR(ts, word);
					if (rootLines != NULL && !rootLines->isUsed)
						rootLines = freeLines(rootLines, TRUE);
				}
			}
		}
		spareTier1[0] = EOS;
		spareTier2[0] = EOS;
	}

#if !defined(CLAN_SRV)
	fprintf(stderr, "\r	  \r");
#endif
	if (!combinput) {
		freq_pr_result();
		rootLines = freeLines(rootLines, FALSE);
	}
}

/* STAFREQ BEGIN */
struct statfreq_sp_list {
	char *fname;
	char *ID;
	struct statfreq_sp_list *next_sp;
} ;

struct statfreq_word_sp_list {
	statfreq_sp_list *spID;
	int  num;
	struct statfreq_word_sp_list *next_word_sp;
} ;

struct W_Struct {
	char *word;
	int count;
	struct statfreq_word_sp_list *speakers;
	struct W_Struct *left;
	struct W_Struct *right;
};

static struct W_Struct *WordsHead;
static struct statfreq_sp_list *statfreq_head;

static struct statfreq_sp_list *statfreq_freeSpeakers(struct statfreq_sp_list *p) {
	struct statfreq_sp_list *t;

	while (p != NULL) {
		t = p;
		p = p->next_sp;
		if (t->fname != NULL)
			free(t->fname);
		if (t->ID)
			free(t->ID);
		free(t);
	}
	return(NULL);
}

static void statfreq_freeWordSps(struct statfreq_word_sp_list *p) {
	struct statfreq_word_sp_list *t;

	while (p != NULL) {
		t = p;
		p = p->next_word_sp;
		free(t);
	}
}

static int findPercentNum(statfreq_sp_list *ts) {
	int num;
	float n, p;

	num = 0;
	while (ts != NULL) {
		num++;
		ts=ts->next_sp;
	}
	n = (float)num;
	p = (float)percent;
	n = (n * p) / 100.0000;
	num = (int)n;
	return(num);
}

static statfreq_sp_list *statfreq_FindSpeaker(char isMissingSpeaker, char *ID, char *fname) {
	statfreq_sp_list *ts;

	if (statfreq_head == NULL) {
		statfreq_head = NEW(statfreq_sp_list);
		ts = statfreq_head;
	} else {
		for (ts=statfreq_head; 1; ts=ts->next_sp) {
			if (uS.partcmp(ts->ID, ID, FALSE, FALSE))
				return(ts);
			if (ts->next_sp == NULL)
				break;
		}
		ts->next_sp = NEW(statfreq_sp_list);
		ts = ts->next_sp;
	}
	if (ts == NULL)
		freq_overflow();
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL)
		freq_overflow();
	strcpy(ts->fname, fname);
	if (ID[0] == EOS) {
		if (isMissingSpeaker)
			strcpy(ID, "SPEAKER NOT FOUND|.|.|.|.|.|.|.|.|.|");
		else
			strcpy(ID, "@ID NOT FOUND|.|.|.|.|.|.|.|.|.|");
	}	
	if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
		freq_overflow();
	strcpy(ts->ID, ID);
	ts->next_sp = NULL;
	return(ts);
}

static statfreq_word_sp_list *addToSpList(statfreq_word_sp_list *root, statfreq_sp_list *cSp, int count) {
	statfreq_word_sp_list *ts;

	if (root == NULL) {
		root = NEW(statfreq_word_sp_list);
		ts = root;
	} else {
		for (ts=root; 1; ts=ts->next_word_sp) {
			if (ts->spID == cSp)
				return(root);
			if (ts->next_word_sp == NULL)
				break;
		}
		ts->next_word_sp = NEW(statfreq_word_sp_list);
		ts = ts->next_word_sp;
	}
	if (ts == NULL)
		freq_overflow();
	ts->spID = cSp;
	ts->num = count;
	ts->next_word_sp = NULL;
	return(root);
}

static char isInList(statfreq_word_sp_list *speakers, statfreq_sp_list *cSp) {
	while (speakers != NULL) {
		if (speakers->spID == cSp)
			return(TRUE);
		speakers = speakers->next_word_sp;
	}
	return(FALSE);
}

static struct W_Struct *statfreq_AddWords(struct W_Struct *p, statfreq_sp_list *cSp, char *w, int count) {
	int cond;

	if (p == NULL) {
		if ((p = NEW(struct W_Struct)) == NULL)
			freq_overflow();
		if ((p->word = (char *)malloc(strlen(w)+1)) == NULL)
			freq_overflow();
		strcpy(p->word, w);
		p->count = 1;
		p->speakers = addToSpList(NULL, cSp, count);
		p->left = p->right = NULL;
	} else if ((cond = strcmp(w, p->word)) == 0) {
		if (!isInList(p->speakers, cSp)) {
			p->count++;
			p->speakers = addToSpList(p->speakers, cSp, count);
		}
	} else if (cond < 0)
		p->left = statfreq_AddWords(p->left, cSp, w, count);
	else
		p->right = statfreq_AddWords(p->right, cSp, w, count); /* if cond > 0 */
	return(p);
}

static struct W_Struct *statfreq_StoreWords(char isOutput, struct W_Struct *p, char *w, int count) {
	int cond;

	if (p == NULL) {
		if ((p = NEW(struct W_Struct)) == NULL)
			freq_overflow();
		if ((p->word = (char *)malloc(strlen(w)+1)) == NULL)
			freq_overflow();
		strcpy(p->word, w);
		p->count = 0;
		p->speakers = NULL;
		p->left = p->right = NULL;
	} else if ((cond = strcmp(w, p->word)) == 0) {
		if (isOutput)
			p->count = count;	
	} else if (cond < 0)
		p->left = statfreq_StoreWords(isOutput, p->left, w, count);
	else
		p->right = statfreq_StoreWords(isOutput, p->right, w, count); /* if cond > 0 */
	return(p);
}

static void statfreq_print_percent_row(statfreq_sp_list *cSp, struct W_Struct *p, int percentNum, FILE *fp, float *diff, float *total) {	
	float tf;
	statfreq_word_sp_list *sp;

	if (p != NULL) {
		statfreq_print_percent_row(cSp, p->left, percentNum, fp, diff, total);
		do {
			if (isSpeakerWord(p->word)) {
			} else if ((percentC == 1 && p->count <  percentNum) ||
				(percentC == 2 && p->count <= percentNum) ||
				(percentC == 3 && p->count == percentNum) ||
				(percentC == 4 && p->count >= percentNum) ||
				(percentC == 5 && p->count >  percentNum)) {
				if (onlydata == 3 || onlydata == 4) {
					if (cSp == NULL) {
						if (onlydata == 3)
							excelStrCell(fp, p->word);
					} else {
						for (sp=p->speakers; sp != NULL; sp=sp->next_word_sp) {
							if (sp->spID == cSp)
								break;
						}
						if (sp != NULL) {
							if (diff != NULL && total != NULL) {
								*total = *total + sp->num;
								*diff = *diff + 1;
							}
							if (onlydata == 3) {
								tf = (float)sp->num;
								excelNumCell(fp, "%.0f", tf);
							}
						} else if (onlydata == 3)
							excelStrCell(fp, "0");
					}
				} else
					fprintf(fp,"%s\n", p->word);
			}
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				statfreq_print_percent_row(cSp, p->right, percentNum, fp, diff, total);
				break;
			}
			p = p->right;
		} while (1);
	}
}

static void statfreq_percent_result(int percentNum, statfreq_sp_list *sp, FILE *fp) {
	char tisLookAtSpeakerWord;
	float diff, total;

	tisLookAtSpeakerWord = isLookAtSpeakerWord;
	isLookAtSpeakerWord = TRUE;
	if (onlydata == 3 || onlydata == 4) {
		if (newfname[0] == EOS) {
			strcpy(newfname, "FREQ");
			if (onlydata == 3)
				strcat(newfname, " +d2");
			else if (onlydata == 4)
				strcat(newfname, " +d2");
			strcat(newfname, " +d");
			if (percentC == 1)
				strcat(newfname, "<");
			else if (percentC == 2)
				strcat(newfname, "<=");
			else if (percentC == 3)
				strcat(newfname, "=");
			else if (percentC == 4)
				strcat(newfname, "=>");
			else if (percentC == 2)
				strcat(newfname, ">");
			sprintf(newfname+strlen(newfname), "%d", percent);
		}
		excelHeader(fp, newfname, 95);
		if (!isMorUsed && chatmode != 0) {
			excelRowOneStrCell(fp, ExcelRedCell, "This TTR number was not calculated on the basis of %%mor line forms.");
			excelRowOneStrCell(fp, ExcelRedCell, "If you want a TTR based on lemmas, run FREQ on the %%mor line");
			excelRowOneStrCell(fp, ExcelRedCell, "with option: +sm;*,o%");
		}
		excelRow(fp, ExcelRowStart);
		excelCommasStrCell(fp, "File,Language,Corpus,Speaker,Age,Sex,Group,Race,SES,Role,Education,Custom field");
		statfreq_print_percent_row(NULL, WordsHead, percentNum, fp, NULL, NULL);
		excelCommasStrCell(fp, "Types,Token,TTR");
		excelRow(fp, ExcelRowEnd);
		while (sp != NULL) {
			excelRow(fp, ExcelRowStart);
			excelStrCell(fp, sp->fname);
			excelOutputID(fp, sp->ID);
			total = 0.0;
			diff = 0.0;
			statfreq_print_percent_row(sp, WordsHead, percentNum, fp, &diff, &total);
			excelNumCell(fp, "%.0f", diff);
			excelNumCell(fp, "%.0f", total);
			if (total != 0)
				excelNumCell(fp, "%.3f", diff/total);
			else
				excelStrCell(fp,"-");
			excelRow(fp, ExcelRowEnd);
			sp = sp->next_sp;
		}
		excelFooter(fp);
	} else {
		statfreq_print_percent_row(NULL, WordsHead, percentNum, fp, NULL, NULL);
	}
	isLookAtSpeakerWord = tisLookAtSpeakerWord;
}

static void statfreq_readpercentfile(void) {
	int i, cnt;
	char *word;
	char isMissingSpeaker;
	char fname[BUFSIZ];
	char id[BUFSIZ];
	char isDataFound;
	statfreq_sp_list *cSp;

	*fname = EOS;
	*id = EOS;
	isDataFound = FALSE;
	isMissingSpeaker = TRUE;
	while (!feof(StatFp)) {
		if (fgets_cr(templineC, UTTLINELEN, StatFp) != NULL) {
			if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
				continue;
			templineC[strlen(templineC)-1] = EOS;
			if (uS.partcmp(templineC,"@FILENAME:",FALSE,FALSE)) {
				if (isDataFound) {
					*fname = EOS;
					*id = EOS;
					isMissingSpeaker = TRUE;
					isMissingSpeaker = TRUE;
				}
				fgets_cr(fname, BUFSIZ, StatFp);
				fname[strlen(fname)-1] = EOS;
			} else if (uS.partcmp(templineC,"@ID:",FALSE,FALSE)) {
				fgets_cr(id, BUFSIZ, StatFp);
				id[strlen(id)-1] = EOS;
				cnt = 0;
				for (i=0; id[i]; i++) {
					if (id[i] =='\t')
						id[i] = '_';
					else if (id[i] == '|') {
						cnt++;
					}
				}
				if (cnt > 0 && cnt < 10)
					strcat(id, ".|");
			} else if (uS.partcmp(templineC,"@ST:",FALSE,FALSE)) {
				fgets_cr(templineC, BUFSIZ, StatFp);
				isDataFound = TRUE;
			} else if (uS.partcmp(templineC,"@SPEAKER_FOUND@",FALSE,FALSE)) {
				isMissingSpeaker = FALSE;
			} else {
				cSp = statfreq_FindSpeaker(isMissingSpeaker, id, fname);
				for (word=templineC; *word != ' ' && *word; word++) ;
				for (; *word == ' '; word++) ;
				if (*word) {
					for (i=0; templineC[i]; i++) {
						if (templineC[i] == '\t')
							templineC[i] = ' ';
					}
					WordsHead = statfreq_AddWords(WordsHead, cSp, word, atoi(templineC));
				}
			}
		}
	}
}

static void statfreq_print_row(struct W_Struct *p, char prname, FILE *fp) {
//	float tf;

	if (p != NULL) {
		statfreq_print_row(p->left, prname, fp);
		do {
			if (prname) {
				excelStrCell(fp, p->word);
			} else {
//				tf = (float)p->count;
				excelNumCell(fp, "%.0f",p->count);
			}
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				statfreq_print_row(p->right, prname, fp);
				break;
			}
			p->count = 0;
			p = p->right;
		} while (1);
		p->count = 0;
	}
}

static void statfreq_pr_result(char *fname, char *id, char *stats, FILE *fp) {
	excelStrCell(fp, fname);
	excelOutputID(fp, id);
	statfreq_print_row(WordsHead, FALSE, fp);
	excelCommasStrCell(fp, stats);
	excelRow(fp, ExcelRowEnd);
	excelRow(fp, ExcelRowStart);
}

static void statfreq_cleanup(struct W_Struct *p) {
	struct W_Struct *t;

	if (p != NULL) {
		statfreq_cleanup(p->left);
		do {
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				statfreq_cleanup(p->right);
				break;
			}
			t = p;
			p = p->right;
			free(t->word);
			statfreq_freeWordSps(t->speakers);
			free(t);
		} while (1);
		free(p->word);
		free(p);
	}
}

static void statfreq_readinfile_writefile(FILE *StatFpOut) {
	int i, cnt;
	float tf;
	char *s;
	char *word;
	char isMissingSpeaker;
	char fname[BUFSIZ];
	char id[BUFSIZ];
	char stats[BUFSIZ];

	*fname = EOS;
	*id = EOS;
	*stats = EOS;
	strcpy(id, "SPEAKER NOT FOUND");
	while (!feof(StatFp)) {
		if (fgets_cr(templineC, UTTLINELEN, StatFp) != NULL) {
			if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
				continue;
			templineC[strlen(templineC)-1] = EOS;
			if (uS.partcmp(templineC,"@FILENAME:",FALSE,FALSE)) {
				fgets_cr(fname, BUFSIZ, StatFp);
				fname[strlen(fname)-1] = EOS;
			} else if (uS.partcmp(templineC,"@ID:",FALSE,FALSE)) {
				fgets_cr(id, BUFSIZ, StatFp);
				id[strlen(id)-1] = EOS;
				cnt = 0;
				for (i=0; id[i]; i++) {
					if (id[i] == '|') {
						cnt++;
						if (cnt == 2) {
							strcpy(id, id+i+1);
							s = strchr(id, '|');
							if (s != NULL)
								*s = EOS;
						}
					}
				}
				if (id[0] == '*')
					strcpy(id, id+1);
				s = strrchr(id, ':');
				if (s != NULL)
					*s = EOS;
			} else if (uS.partcmp(templineC,"@ST:",FALSE,FALSE)) {
				fgets_cr(stats, BUFSIZ, StatFp);
			} else if (uS.partcmp(templineC,"@SPEAKER_FOUND@",FALSE,FALSE)) {
				isMissingSpeaker = FALSE;
			} else {
				for (word=templineC; *word != ' ' && *word; word++) ;
				for (; *word == ' '; word++) ;
				if (*word) {
					for (i=0; templineC[i]; i++) {
						if (templineC[i] == '\t')
							templineC[i] = ' ';
					}
					if (word[0] == '*') {
					} else {
						excelRow(StatFpOut, ExcelRowStart);
						excelStrCell(StatFpOut, fname);
						excelStrCell(StatFpOut, id);
						excelStrCell(StatFpOut, word);
						tf = (float)atoi(templineC);
						excelNumCell(StatFpOut, "%.0f",tf);
						excelRow(StatFpOut, ExcelRowEnd);
					}
				}
			}
		}
	}
	if (id[0] == EOS) {
		if (isMissingSpeaker)
			strcpy(id, "SPEAKER NOT FOUND,.,.,.,.,.,.,.,.,.,");
		else
			strcpy(id, "@ID NOT FOUND,.,.,.,.,.,.,.,.,.,");
	}
}

static void statfreq_readinfile(char isOutput, FILE *fp) {
	int i, cnt;
	char *word;
	char isMissingSpeaker;
	char fname[BUFSIZ];
	char id[BUFSIZ];
	char stats[BUFSIZ];

	*fname = EOS;
	*id = EOS;
	*stats = EOS;
	isMissingSpeaker = TRUE;
	while (!feof(StatFp)) {
		if (fgets_cr(templineC, UTTLINELEN, StatFp) != NULL) {
			if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
				continue;
			templineC[strlen(templineC)-1] = EOS;
			if (uS.partcmp(templineC,"@FILENAME:",FALSE,FALSE)) {
				if (isOutput && stats[0] != EOS) {
					if (id[0] == EOS) {
						if (isMissingSpeaker)
							strcpy(id, "SPEAKER NOT FOUND,.,.,.,.,.,.,.,.,.,");
						else
							strcpy(id, "@ID NOT FOUND,.,.,.,.,.,.,.,.,.,");
					}
					statfreq_pr_result(fname, id, stats, fp);
					*fname = EOS;
					*id = EOS;
					*stats = EOS;
					isMissingSpeaker = TRUE;
				}
				fgets_cr(fname, BUFSIZ, StatFp);
				fname[strlen(fname)-1] = EOS;
			} else if (uS.partcmp(templineC,"@ID:",FALSE,FALSE)) {
				fgets_cr(id, BUFSIZ, StatFp);
				id[strlen(id)-1] = EOS;
				cnt = 0;
				for (i=0; id[i]; i++) {
					if (id[i] =='\t')
						id[i] = '_';
					else if (id[i] == '|') {
						cnt++;
					}
				}
				if (cnt > 0 && cnt < 10)
					strcat(id, ".|");
			} else if (uS.partcmp(templineC,"@ST:",FALSE,FALSE)) {
				fgets_cr(stats, BUFSIZ, StatFp);
				stats[strlen(stats)-1] = EOS;
				for (i=0; isSpace(stats[i]); i++) ;
				if (i > 0)
					strcpy(stats, stats+i);
				for (i=0; stats[i]; i++) {
					if (isSpace(stats[i])) {
						stats[i] = ',';
					} else if (stats[i] == '-')
						stats[i] = '.';
				}
			} else if (uS.partcmp(templineC,"@SPEAKER_FOUND@",FALSE,FALSE)) {
				isMissingSpeaker = FALSE;
			} else {
				for (word=templineC; *word != ' ' && *word; word++) ;
				for (; *word == ' '; word++) ;
				if (*word) {
					for (i=0; templineC[i]; i++) {
						if (templineC[i] == '\t')
							templineC[i] = ' ';
					}
					WordsHead = statfreq_StoreWords(isOutput, WordsHead, word, atoi(templineC));
				}
			}
		}
	}
	if (isOutput && stats[0] != EOS) {
		if (id[0] == EOS) {
			if (isMissingSpeaker)
				strcpy(id, "SPEAKER NOT FOUND,.,.,.,.,.,.,.,.,.,");
			else
				strcpy(id, "@ID NOT FOUND,.,.,.,.,.,.,.,.,.,");
		}
		statfreq_pr_result(fname, id, stats, fp);
	}
}
/* STATFREQ ENDS */

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int  percentNum;
	char tCombinput;
	FNType *fn;
	FILE *StatFpOut = NULL;

	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = FREQ;
	chatmode = CHAT_MODE;
	OnlydataLimit = 9;
	UttlineEqUtterance = FALSE;
	isFoundTier = FALSE;
	bmain(argc,argv,freq_pr_result);
	rootLines = freeLines(rootLines, FALSE);
	if (percent > 0) {
		statfreq_head = NULL;
		if (StatFp != NULL)
			fclose(StatFp);
		StatFp = NULL;
		if (!isFoundTier) {
			fprintf(stderr,"\nCAN'T FIND ANY DATA TIERS IN ANY OF INPUT FILES\n");
			fprintf(stderr,"PLEASE PROVIDE A SPECIFIC SPEAKER WITH +t OPTION\n\n");
		} else {
			StatFp = fopen(StatName, "r");
			if (StatFp == NULL) {
				fprintf(stderr,"Can't open file \"%s\" to read.\n", StatName);
			} else {
//				OverWriteFile = FALSE;
				if (onlydata == 3 || onlydata == 4)
					AddCEXExtension = ".xls";
				extractPath(FileName2, StatName);
				strcat(FileName2, "words.cha");
				parsfname(FileName2, FileName1, GExt);
				tCombinput = combinput;
				combinput = FALSE;
				if (f_override)
					StatFpOut = stdout;
				else if ((StatFpOut=openwfile(FileName2, FileName1, NULL)) == NULL) {
					fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",FileName1);
				}
				combinput = tCombinput;
				if (StatFpOut != NULL) {
					WordsHead = NULL;
					statfreq_readinfile(FALSE, StatFpOut);
					rewind(StatFp);
					statfreq_readpercentfile();
					fclose(StatFp);
					percentNum = findPercentNum(statfreq_head);
					statfreq_percent_result(percentNum, statfreq_head, StatFpOut);
					if (!f_override && StatFpOut != stdout)
						fclose(StatFpOut);
					StatFp = NULL;
					unlink(StatName);
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
					statfreq_cleanup(WordsHead);
				}
			}
		}
		statfreq_head = statfreq_freeSpeakers(statfreq_head);
	} else if (onlydata == 3 || onlydata == 4) {
		if (StatFp != NULL)
			fclose(StatFp);
		StatFp = NULL;
		if (!isFoundTier) {
			fprintf(stderr,"\nCAN'T FIND ANY DATA TIERS IN ANY OF INPUT FILES\n");
			fprintf(stderr,"PLEASE PROVIDE A SPECIFIC SPEAKER WITH +t OPTION\n\n");
		} else {
			StatFp = fopen(StatName, "r");
			if (StatFp == NULL) {
				fprintf(stderr,"Can't open file \"%s\" to read.\n", StatName);
			} else {
//				OverWriteFile = FALSE;
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
					WordsHead = NULL;
					if (isSpreadsheetOnePerRow) {
						excelHeader(StatFpOut, newfname, 95);
						excelRow(StatFpOut, ExcelRowStart);
						excelCommasStrCell(StatFpOut, "File,Code,Word,Count");
						excelRow(StatFpOut, ExcelRowEnd);
						statfreq_readinfile_writefile(StatFpOut);
						excelFooter(StatFpOut);
					} else {
						statfreq_readinfile(FALSE, StatFpOut);
						rewind(StatFp);
						excelHeader(StatFpOut, newfname, 95);
						if (!isMorUsed && chatmode != 0) {
							excelRowOneStrCell(StatFpOut, ExcelRedCell,
								"This TTR number was not calculated on the basis of %%mor line forms.");
							excelRowOneStrCell(StatFpOut, ExcelRedCell,
								"If you want a TTR based on lemmas, run FREQ on the %%mor line");
							excelRowOneStrCell(StatFpOut, ExcelRedCell,
								"with option: +sm;*,o%");
						}
						excelRow(StatFpOut, ExcelRowStart);
						excelCommasStrCell(StatFpOut,
							"File,Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom field");
						statfreq_print_row(WordsHead, TRUE, StatFpOut);
						if (frame_size > 0)
							excelCommasStrCell(StatFpOut, "Types,Token,TTR,MATTR");
						else
							excelCommasStrCell(StatFpOut, "Types,Token,TTR");
						excelRow(StatFpOut, ExcelRowEnd);
						excelRow(StatFpOut, ExcelRowStart);
						statfreq_readinfile(TRUE, StatFpOut);
						statfreq_cleanup(WordsHead);
						excelRow(StatFpOut, ExcelRowEnd);
//						printArg(argv, argc, StatFpOut, FALSE, "");
						excelFooter(StatFpOut);
					}
					fclose(StatFp);
					if (!f_override && StatFpOut != stdout)
						fclose(StatFpOut);
					StatFp = NULL;
					unlink(StatName);

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
				}
			}
		}
	}
/*
	if (isR6Legal) {
#ifdef UNX
		fprintf(stderr, "\nDATA ASSOCIATED WITH CODES [/], [//], [///], [/-], [/?] IS EXCLUDED BY DEFAULT.\n");
		fprintf(stderr, "TO INCLUDE THIS DATA PLEASE USE \"+r6\" OPTION.\n");
#else
		fprintf(stderr, "\n%c%cDATA ASSOCIATED WITH CODES [/], [//], [///], [/-], [/?] IS EXCLUDED BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
		fprintf(stderr, "%c%cTO INCLUDE THIS DATA PLEASE USE \"+r6\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	}
*/
}
