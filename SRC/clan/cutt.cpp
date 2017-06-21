/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*
 Server CLAN:

 g++ -O -DUNX -DCLAN_SRV -fwritable-strings freq.c -o freq

 freq +0 *.cha 2>&1
*/

#include "cu.h"
#include "mul.h"
#include "mllib.h"
#include "check.h"

#ifndef UNX
	#include "ced.h"
#endif
#include <time.h>
#ifdef _WIN32
	#include "stdafx.h"
	#include "Clan2.h"
	#include "w95_commands.h"
	#include <TextUtils.h>
#endif

#ifdef UNX
	#define RGBColor int
	#define WD_Not_Eq_OD (pathcmp(wd_dir, od_dir))

    #if defined(__linux__) || defined(ERRNO)
        #define USE_TERMIO
    #endif

	#ifdef USE_TERMIO
		#include <sys/errno.h>
		#if defined(ERRNO)
			#include <errno.h>
		#endif		
        #ifdef __linux__
            #include <termio.h>
        #else
			#include <sys/termio.h>
        #endif
	#else
		#include <sgtty.h>
	#endif
	#include <sys/stat.h>
	#include <dirent.h>

	#include <sys/ioctl.h>
	#include <ctype.h>
	#include "c_curses.h"
	#include "mul.h"

	typedef unsigned long  UInt32;

	NewFontInfo dFnt = {"", "", 0L, 0, 0, 0, 0, 0, 0, '\0', 0, 0, NULL};
	FNType wd_dir[FNSize];
	FNType od_dir[FNSize];
	FNType lib_dir[FNSize];
	FNType mor_lib_dir[FNSize];
	int isKillProgram = 0;
#endif
#if defined(_MAC_CODE)
	#include <sys/stat.h>
	#include <dirent.h>
#endif
#if !defined(UNX)
	#define usage (*clan_usage[CLAN_PROG_NUM])
	#define call (*clan_call[CLAN_PROG_NUM])
	#define getflag (*clan_getflag[CLAN_PROG_NUM])
	#define init (*clan_init[CLAN_PROG_NUM])

	extern char *MEMPROT;
#endif /* !UNX */

#define EXTENDEDARGVLEN 10000
#define NUMLANGUAGESINTABLE 10
#define OPTIONS_EXT_LEN 15

#define MORPHS "-#~+"

#define UTTLENWORDS struct UttLenWord
UTTLENWORDS {
	char *word;
	char inc;
	UTTLENWORDS *nextWdUttLen;
} ;

#define LANGPARTSLIST struct LangsPartList
#define LANGWDLST struct LangWordList
struct LangsPartList {
	char partType;
	char flag;
	char isLangMatch;
	char *pat;
    LANGPARTSLIST *nextPart;
} ;
struct LangWordList {
	char type;
    LANGPARTSLIST *rootPart;
    LANGWDLST *nextMatch;
} ;

#define LANGSTABLE struct LangsList
LANGSTABLE {
	char *iso639[4];
	char *name;
	LANGSTABLE *nextlang;
} ;

#define LANGPARTS struct LangParts
struct LangParts {
    char *wordpart;
	char *pos;
	char *lang0;
	char *lang1;
	char *lang2;
	char *lang3;
	char *suffix0;
	char *suffix1;
	char *suffix2;
} ;

#define ML_CLAUSE struct clauses
ML_CLAUSE {
	char *cl;
	ML_CLAUSE *nextcl;
} ;
static ML_CLAUSE *ml_root_clause;

struct mor_link_struct mor_link;

char isWinMode;	/* controls line counting and window features */

//extern void (*clan_main[]) (int argc, char *argv[]);
extern void (*clan_usage[]) (void);
extern void (*clan_getflag[]) (char *, char *, int *);
extern void (*clan_init[]) (char);
extern void (*clan_call[]) (void);

static char options_already_inited = 0;
static char fontErrorReported = FALSE;

void VersionNumber(char isshortfrmt, FILE *fp);
int  IsOutTTY(void);

#define ALL_OPTIONS F_OPTION+RE_OPTION+K_OPTION+P_OPTION+L_OPTION+R_OPTION+SP_OPTION+SM_OPTION+T_OPTION+UP_OPTION+Z_OPTION

#define TESTLIM 6
const char *MonthNames[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
			   "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" } ;
char GExt[32];
int  OnlydataLimit;
int  UttlineEqUtterance;
int  chatmode;				/* chatmode =0 - no y switch & errmargin =0;*/
							/*		=1 - y switch & errmargin =0;	   */
							/*		=2 - no y switch & errmargin =0;	*/
							/*		=3 - y switch & errmargin = TESTLIM */
							/*		=4 - no y switch & no testing	*/


/**************************************************************/
/*	 enviroment control				  */

char tcs = FALSE,			/* 1- if all non-specified speaker tier should be selected*/
	 tch = FALSE,			/* 1- if all non-specified header tier should be selected */
	 tct = FALSE;			/* 1- if all non-specified code tier should be selected   */
char otcs = TRUE,			/* 1- if all non-specified speaker tier should be selected*/
	 otch = TRUE,			/* 1- if all non-specified header tier should be selected */
	 otct = TRUE,			/* 1- if all non-specified code tier should be selected   */
	 otcdt = FALSE;			/* 1- if code tiers were specified by user with +o option */
char tcode = FALSE;			/* 1- if all files with keyword on @Keywords should be included */
static char *targs;				/* array used by bmain; 0 if arg is a file name; otherwise 1  */
static char LanguagesTable[NUMLANGUAGESINTABLE][9];
static char foundUttContinuation = FALSE;
static int  defLanguage;
static LANGSTABLE *langs_list;
char GlobalPunctuation[50];
static char morPunctuation[50];
static char cedPunctuation[50];
//2011-01-26 FNType punctFile[FNSize];
//2011-01-26 char punctMess[512];
char ProgDrive;
char OutputDrive;
char IncludeAllDefaultCodes;
char cutt_isMultiFound = FALSE;
char cutt_isCAFound = FALSE;
char cutt_isBlobFound = FALSE;
char cutt_depWord = FALSE;
char isRemoveCAChar[NUMSPCHARS+2];
struct tier *HeadOutTier;	/* points to the list of specified output tier   */
struct tier *headtier;		/* points to the list of specified tier	   */
struct tier *defheadtier;	/* default include/exclude tier list header	   */
struct IDtype *IDField;		/* used to specify speaker names through @ID tier*/
struct IDtype *CODEField;	/* used to filter files based on string on @Keywords tier */
struct IDtype *SPRole;		/* specifies speakers through role on @participants tier */
UTTER *utterance = NULL;
UTTER *lutter;				/* point to current tier in a set of window tiers   */
char *uttline;				/* points to a working version of the turn text	 */
char currentchar= '\0';		/* contains last input character read		   */
AttTYPE currentatt=  '\0';	/* contains attribute of the last input character read */
int aftwin = 0;				/* size of window after the target tier, in tiers   */
int IsModUttDel;
int CLAN_PROG_NUM;
int getc_cr_lc = '\0';
char fgets_cr_lc = '\0';
long MAXOUTCOL;
long option_flags[LAST_CLAN_PROG];

char MBF = FALSE;
char C_MBF = FALSE;
static char options_ext[LAST_CLAN_PROG][OPTIONS_EXT_LEN+1];
char *expandedArgv = NULL;
char org_spTier[UTTLINELEN+2];	/* used by createMorUttline function */
static char tempTier[UTTLINELEN+2];	/* used by createMorUttline, remove_main_tier_print, expandX functions */
char spareTier1[UTTLINELEN+2];
char spareTier2[UTTLINELEN+2];
char morTierName[SPEAKERLEN+1];	/* to store %mor tier name */ 
char rightUttLenC[UTTLINELEN+2];/* used in "rightUttLen" */
unCH templine [UTTLINELEN+2];		/* temporary variable to store code descriptors */ // found uttlinelen
unCH templine1[UTTLINELEN+2];		/* temporary variable to store code descriptors */
unCH templine2[UTTLINELEN+2];		/* temporary variable to store code descriptors */
unCH templine3[UTTLINELEN+2];		/* temporary variable to store code descriptors */
unCH templine4[UTTLINELEN+2];   	/* temporary variable to store code descriptors */
char templineC [UTTLINELEN+2];		/* used to be: templine  */
char templineC1[UTTLINELEN+2];		/* used to be: templine1 */
char templineC2[UTTLINELEN+2];		/* used to be: templine2 */
char templineC3[UTTLINELEN+2];		/* used to be: templine3 */
char templineC4[UTTLINELEN+2];		/* used to be: templine4 */
AttTYPE tempAtt[UTTLINELEN+2];		/* temporary variable to store code descriptors */
wchar_t templineW[UTTLINELEN+2];/* temporary variable to store code descriptors */
wchar_t templineW1[UTTLINELEN+2];/* temporary variable to store code descriptors */
FNType DirPathName[FNSize];			/* used for temp storage of path name */
FNType FileName1[FNSize];			/* used for temp storage of file name */
FNType FileName2[FNSize];			/* used for temp storage of file name */
FNType cMediaFileName[FILENAME_MAX];/* used by expend bullets */

int  postcodeRes;			/* reflects presence of postcodes 0-if non or 4,5,6-if found */
char rightspeaker = '\001';	/* 1 - speaker/code turn is selected by the user*/
char nomain = FALSE;			/* 1 - exclude text from main speaker tiers,   0*/
char getrestline = 1;		/* 0 - cutt_getline() should not get rest of text,  1*/
char skipgetline = FALSE;	/* 1 - cutt_getline() should NOT add "\n"		   0*/
char CntWUT;				/* 0- do not count words or utterances, dflt
								1- count words; 2- utterances; 3- turns;
								it is used to produce a range of words or utt*/
char CntFUttLen;			/* 0- do not count utterance length, dflt
								1- count number of words; 
								2- count number of morpheme; 
								3- count number of characters; 
								it is used to filter utts based on their length*/
char linkMain2Mor = FALSE;	/* connect words on speaker tier to corresponding items on %mor */
char linkMain2Sin = FALSE;	/* connect words on speaker tier to corresponding items on %sin */
char isMorWordSpec = 0;		/* if no word specified with +s, then 0, all are = 1, at least 1 not %mor word, = 2 */
char Preserve_dir = TRUE;	/* select directory for output files		   */
char FirstTime = TRUE;		/* 0 if the call() has been called once, dflt 1 */
char *Toldsp;				/* last speaker name, used  for +z#t range	   */
char *TSoldsp;				/* last speaker name, used  for -u range	   */
char Tspchanged;		/* iniciates speaker turn change		   */
char isExpendX = TRUE;			/* if = 1, then expend word [x 2] to word [/] word */
char LocalTierSelect;		/* 1 if tier selection is done by programs,dfl 1*/
char FilterTier;			/* 0 if tiers should NOT be filtered, dflt 1	   */
char Parans = 1;			/* get(s); 1-gets, 2-get(s), 3-get		   */
char f_override;			/* if user specified -f option, then = 1 */
char y_option;				/* 0 - line at a time; 1 - utterance at a time  */
char *rootmorf = NULL;		/* contains a list of morpheme characters	   */
char RemPercWildCard;		/* if =1 then chars matched by % are removed	   */
char contSpeaker[SPEAKERLEN+1];	/* if +/. found then set it and look for +, */
char OverWriteFile;			/* if =0 then keep output file versions		*/
char PreserveFileTypes;		/* if=1 then file date and type are copied */
const char *AddCEXExtension;	/* if =1 then add ".cex" to output file name*/
static char isPostcliticUse;	/* if '~' found on %mor tier break it into two words */
static char isPrecliticUse;	/* if '$' found on %mor tier break it into two words */
static char restoreXXX, restoreYYY, restoreWWW;
char filterUttLen_cmp;		/* = Equal, < les than, > greater than */
long filterUttLen;			/* filter utterances based on their length */
long FromWU,				/* contains the beginning range number	   */
	 ToWU,					/* contains the end range number		   */
	 WUCounter;				/* current word/utterance count number	   */
static UTTLENWORDS *wdUttLen;	/* contains words to included/excluded			*/

/*****************************************************************/
/*	 programs common options						 */
const char *delSkipedFile = NULL;/* if +t@id="" option used and file didn't match, then delete output file */
char isFileSkipped = FALSE;	/* TRUE - if +t@id="" option used and file didn't match */
char replaceFile = FALSE;	/* 1 - replace org data file with the same name file */
int  onlydata = '\0';		/* 1,2,3 - level of amount of data output, dflt 0  */
char puredata = 2;			/* FALSE - the +d output is NOT CHAT legal, dflt 1 */
char outputOnlyData = FALSE;	/* if FALSE, then no file, command line info outputed */
char combinput = '\0';		/* 1 - merge data from all specified file, dflt 0  */
char ByTurn = '\0';			/* if = 1 then each turn is afile set		   */
char nomap = FALSE;			/* 0 - convert all to lower case, dflt 0 	   */
char WordMode = '\0';		/* 'i' - means to include wdptr words, else exclude*/
char R5;					/* take care of [: text]. default leave it in
							   its a hack look for value 1001 by "isExcludeScope" */
char R5_1;					/* take care of [:: text]. default leave it in
							 its a hack look for value 1001 by "isExcludeScope" */
char isLanguageExplicit;	/* 1 - language tags added to every word */
char pauseFound,opauseFound;	/* 1 - if +s#* found on command line				*/
char *ScopWdPtr[MXWDS];		/* set of scop tokens (<one>[\\]) to be included
								or excluded */
char ScopMode = '\0';		/* 'i' - means to included scop tokens, else excl  */
int  ScopNWds = 1;			/* number of include/exclude scop tokens specified */
char PostCodeMode= '\0';	/* mode for post codes, the same setup as ScopMode */
char R4 = FALSE;			/* if true do not break words at '~' and '$' on %mor tier */
char R6 = TRUE;				/* if true then retraces </>,<//>,<///>,</->,</?> are included   */
char R6_override = FALSE;	/* includes all </>,<//>,<///>,</->,</?> regardless of other settings */
char R7Slash = TRUE;			/* if false do not remove '/' from words */
char R7Tilda = TRUE;			/* if false do not remove '~' from words */
char R7Caret = TRUE;			/* if false do not remove '^' from words */
char R7Colon = TRUE;			/* if false do not remove ':' from words */
char isSpecRepeatArrow = FALSE;	/* 1 if left arrow loop character is used in search */
char R8 = FALSE;			/* if true inlcude actual word spoken and/or error code in output from %mor: tier */
char isRecursive=FALSE;		/* if = 1 then run on all sub-directories too	  */
char stin_override=FALSE;		/* TRUE is +0 option used with CLAN_SRV			*/
char anyMultiOrder = FALSE;	/* if = 1 then match multi-words in any order found on a tier */
char onlySpecWsFound= FALSE;	/* if = 1 then match only if tier consists solely of all words in multi-word group */
long lineno = 0L;			/* current turn line number (dflt 0)		   */
long tlineno = 1L;			/* line number within the current turn		   */
long deflineno = 0L;		/* default line number				   */
IEWORDS *wdptr;				/* contains words to included/excluded			*/
IEMWORDS *mwdptr;			/* contains multi-words to included/excluded	*/
static MORWDLST *morfptr;	/* contains words to included/excluded			*/
static LANGWDLST *langptr;	/* contains words to included/excluded			*/
static IEWORDS *defwdptr;	/* contains default words to included/excluded	*/
static IEWORDS *CAptr;				/* contains words to included/excluded			*/

/**************************************************************/
/*	 input output file control 				  */
FILE *fpin;					/* file pointers to input stream		   */
FILE *fpout;				/* file pointers to output stream		   */
FNType *oldfname;
FNType newfname[FNSize];	/* the name of the currently opened file	   */
char stout = TRUE;			/* 0 - if the output is NOT stdout, dflt 1	   */
char stin  = FALSE;			/* 1 - if the input is stdin, dflt 0		   */

char maxwd_which;

#ifdef _WIN32
	static CFileFind fileFind;
#endif /* _WIN32 */
#if defined(UNX) || defined(_MAC_CODE)
	static DIR *cDIR;
#endif /* UNX */

#ifdef UNX
short my_CharacterByteType(const char *org, short pos, NewFontInfo *finfo) {
	short cType;

	if (finfo->isUTF) {
		cType = 2;
		if (UTF8_IS_SINGLE((unsigned char)org[pos]))
			cType = 0;
		else if (UTF8_IS_LEAD((unsigned char)org[pos]))
			cType = -1;
		else if (UTF8_IS_TRAIL((unsigned char)org[pos])) {
			if (!UTF8_IS_TRAIL((unsigned char)org[pos+1]))
				cType = 1;
			else
				cType = 2;
		}

		return(cType);
	} else
		return(0);
}
#endif // UNX


void VersionNumber(char isshortfrmt, FILE *fp) {
	long pnum = 0L, t;
	char *s;
	char pbuf[20];
	extern char VERSION[];

	s = VERSION;
	while (!isdigit(*s) && *s)
		s++;
	if (!*s)
		return;

	pnum = atoi(s);
	if (s[1] == '-') {
		pbuf[0] = ' ';
		pbuf[1] = *s;
	} else {
		pbuf[0] = *s;
		pbuf[1] = *(s+1);
	}
	while (isdigit(*s))
		s++;
	if (!*s)
		return;
	while (!isdigit(*s) && !isalpha(*s) && *s)
		s++;
	if (!*s)
		return;
	if (isdigit(*s)) {
		t = atoi(s);
		if (t-1 > 11)
			return;
		strcpy(pbuf+3, MonthNames[t-1]);
		pnum = pnum + (t * 100L);
		while (isdigit(*s))
			s++;
	} else {
		pbuf[3] = *s;
		pbuf[4] = *(s+1);
		pbuf[5] = *(s+2);
		while (isalpha(*s))
			s++;
		pnum = pnum + (12 * 100L);
	}

	if (!*s) return;
	while (!isdigit(*s) && *s) s++;
	if (!*s) return;
	pnum = pnum + ((long)atoi(s) * 10000L);
	pbuf[7] = *s; pbuf[8] = *(s+1);
	pbuf[2] = '-'; pbuf[6] = '-';
	if (isdigit(s[2])) {
		pbuf[9] = *(s+2); pbuf[10] = *(s+3);
		pbuf[11] = EOS;
	} else
		pbuf[9] = EOS;
	if (pbuf[0] == ' ')
		strcpy(pbuf, pbuf+1);

	if (isshortfrmt) {
		fprintf(fp, " (%s)", pbuf);
	} else {
		fprintf(fp, "CLAN version: %s; ", pbuf);
#if defined(_MAC_CODE)
		fprintf(fp, "MAC using XCode 3.1.2.\n");
#elif defined(_WIN32)
		fprintf(fp, "WinPC using MS Visual C++ 8.0.\n");
#else
		fprintf(fp, "Unix.\n");
#endif
	}
}

void out_of_mem() {
	fputs("ERROR: Out of memory.\n",stderr);
	cutt_exit(1);
}

int getc_cr(FILE *fp, AttTYPE *att) {
	register int c;

#if defined(_MAC_CODE) || defined(_WIN32)
	SysEventCheck(400L);
#endif

	if (ByTurn == 3) {
		if (att != NULL)
			*att = currentatt;
		return(currentchar);
	}
rep:
	if ((c=getc(fp)) == '\r') {
		if (getc_cr_lc == '\n')
			c = getc(fp);
		else {
			getc_cr_lc = '\r';
			return('\n');
		}
	} else if (c == '\n' && getc_cr_lc == '\r') {
		c = getc(fp);
		if (c == '\r') {
			getc_cr_lc = '\r';
			return('\n');
		}
	}
	getc_cr_lc = c;
	if (c == (int)ATTMARKER) {
		c = getc(fp);
		if (SetTextAtt(NULL, (unCH)c, att))
			goto rep;
		else {
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr,"Illegal text attribute marker '%c'(%d) found\n", c, (int)c);
			cutt_exit(0);
		}
	}
	return(c);
}

char *fgets_cr(char *beg, int size, FILE *fp) {
	register int i = 1;
	register char *buf;

#if defined(_MAC_CODE) || defined(_WIN32)
	SysEventCheck(200L);
#endif
	size--;
	buf = beg;
	*buf = (char)getc(fp);
	if (feof(fp))
		return(NULL);
	do {
		i++;
		if (*buf == '\r') {
			if (fgets_cr_lc != '\n') {
				*buf++ = '\n';
				fgets_cr_lc = '\r';
				break;
			} else
				i--;
		} else if (*buf == '\n') {
			if (fgets_cr_lc != '\r') {
				fgets_cr_lc = '\n';
				buf++;
				break;
			} else
				i--;
		} else
			buf++;
		fgets_cr_lc = '\0';
		if (i >= size)
			break;
		*buf = (char)getc(fp);
		if (feof(fp))
			break;
	} while (1) ;
	*buf = EOS;
	return(beg);
}

char *getCurrDirName(char *dir) {
	int i, t;

	i = strlen(dir) - 1;
	while ((isSpace(dir[i]) || dir[i] == '\n' || dir[i] == PATHDELIMCHR) && i >= 0)
		i--;
	if (i < 0)
		return(dir);
	t = i + 1;
	while (dir[i] != PATHDELIMCHR && i >= 0)
		i--;
	i++;
	t = t - i;
	strncpy(templineC4, dir+i, t);
	templineC4[t] = EOS;
	return(templineC4);
}

void displayOoption(void) {
	puts("     macl  - Mac Latin (German, Spanish ...)");
	puts("     macce - Mac Central Europe");
	puts("     pcl   - PC  Latin (German, Spanish ...)");
	puts("     pcce  - PC  Central Europe\n");

	puts("     macar - Mac Arabic");
	puts("     pcar  - PC  Arabic\n");

	puts("     maccs - Mac Chinese Simplified");
	puts("     macct - Mac Chinese Traditional");
	puts("     pccs  - PC  Chinese Simplified");
	puts("     pcct  - PC  Chinese Traditional\n");

	puts("     maccr - Mac Croatian");

	puts("     maccy - Mac Cyrillic");
	puts("     pccy  - PC  Cyrillic");

	puts("     machb - Mac Hebrew");
	puts("     pchb  - PC  Hebrew\n");

	puts("     macjp - Mac Japanese");
	puts("     pcjp  - PC  Japanese (Shift-JIS)\n");
	puts("     macj1 - Mac Japanese (JIS_X0201_76)");
	puts("     macj2 - Mac Japanese (JIS_X0208_83)");
	puts("     macj3 - Mac Japanese (JIS_X0208_90)");
	puts("     macj4 - Mac Japanese (JIS_X0212_90)");
	puts("     macj5 - Mac Japanese (JIS_C6226_78)");
	puts("     macj6 - Mac Japanese (Shift-JIS_X0213_00)");
	puts("     macj7 - Mac Japanese (ISO_2022_JP)");
	puts("     macj8 - Mac Japanese (ISO_2022_JP_1)");
	puts("     macj9 - Mac Japanese (ISO_2022_JP_2)");
	puts("     macj10- Mac Japanese (ISO_2022_JP_3)");
	puts("     macj11- Mac Japanese (EUC_JP)");
	puts("     macj12- Mac Japanese (Shift-JIS)\n");

	puts("     krn   - Mac and Windows Korean\n");
	puts("     pckr  - PC DOS Korean unified Hangul Code");
	puts("     pckrj - PC DOS Korean Johab");

	puts("     macth - Mac Thai");
	puts("     pcth  - PC  Thai\n");

	puts("     pcturk- PC  Turkish\n");

	puts("     macvt - Mac Vietnamese");
	puts("     pcvt  - PC  Vietnamese");
}

/**************************************************************/
//ML_ CLAUSE ml_ cluase begin
int ml_isclause(void) {
	return(ml_root_clause != NULL);
}

static int ml_IsClauseMarker(char *s) {
	ML_CLAUSE *tc;

	for (tc=ml_root_clause; tc; tc=tc->nextcl) {
		if (uS.partwcmp(s, tc->cl))
			return(TRUE);
	}
	return(FALSE);
}

int ml_UtterClause(char *line, int pos) {
	if (ml_root_clause == NULL) {
		if (*utterance->speaker == '%' && line[pos] == '?' && line[pos+1] == '|')
			;
		else if (uS.IsUtteranceDel(line, pos)) {
			if (!uS.atUFound(line, pos, &dFnt, MBF))
				return(TRUE);
		}
	} else {
		if (ml_IsClauseMarker(line+pos))
			return(TRUE);
	}
	return(FALSE);
} 

static void ml_free_clause(void) {
	ML_CLAUSE *t;

	while (ml_root_clause != NULL) {
		t = ml_root_clause;
		ml_root_clause = ml_root_clause->nextcl;
		if (t->cl != NULL)
			free(t->cl);
		free(t);
	}
}

void ml_AddClause(char opt, char *st) {
	char code[BUFSIZ], t[BUFSIZ];
	ML_CLAUSE *tc;

	if ((tc=NEW(ML_CLAUSE)) == NULL)
		out_of_mem();
	if (st[0] != '[') {
		strcpy(code, "[^");
		strcat(code, st);
		strcat(code, "]");
	} else
		strcpy(code, st);
	strcpy(t, "\001");
	strcat(t, code);
	if (t[1] == '[') {
		int i;
		t[1] = '<';
		for (i=0; t[i]; i++) {
			if (t[i] == ']') {
				t[i] = '>';
				break;
			}
		}
		addword(opt, '\0', t);
		t[1] = '[';
		for (i=0; t[i]; i++) {
			if (t[i] == '>') {
				t[i] = ']';
				break;
			}
		}
		addword(opt, 'i', t);
	}
	tc->cl = (char *)malloc(strlen(code)+1);
	if (tc->cl != NULL)
		strcpy(tc->cl, code);
	tc->nextcl = ml_root_clause;
	ml_root_clause = tc;
}

void ml_AddClauseFromFile(char opt, FNType *fname) {
	FILE *fp;
	char wd[BUFSIZ+1];
	FNType mFileName[FNSize];

	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of the delemeter files:\n\t\"%s\", \"%s\"\n", fname, mFileName);
		cutt_exit(0);
	}
	while (fgets_cr(wd, BUFSIZ, fp)) {
		if (uS.isUTF8(wd) || uS.partcmp(wd, FONTHEADER, FALSE, FALSE))
			continue;
		uS.remblanks(wd);
		ml_AddClause(opt, wd);
	}
	fclose(fp);
}
//ML_ CLAUSE ml_ cluase end
/**************************************************************/

/**************************************************************/
/*	BEGIN %mor tier elements parsing BEGIN */
#define isStringEmpty(x) (x == NULL || *x == EOS)

static void freeMorFeats(MORFEATURESLIST *p) {
	MORFEATURESLIST *t;

	while (p != NULL) {
		t = p;
		p = p->nextFeat;
		if (t->pat != NULL)
			free(t->pat);
		free(t);
	}
}

MORWDLST *freeMorWords(MORWDLST *p) {
	MORWDLST *t;

	while (p != NULL) {
		t = p;
		p = p->nextMatch;
		freeMorFeats(t->rootFeat);
		free(t);
	}
	return(p);
}

void freeUpFeats(MORFEATS *p) {
	MORFEATS *t;

	if (p->free_prefix)
		free(p->prefix);
	if (p->free_suffix0)
		free(p->suffix0);
	if (p->free_suffix1)
		free(p->suffix1);
	if (p->free_suffix2)
		free(p->suffix2);
	if (p->free_fusion0)
		free(p->fusion0);
	if (p->free_fusion1)
		free(p->fusion1);
	if (p->free_fusion2)
		free(p->fusion2);
	if (p->free_trans)
		free(p->trans);
	if (p->free_repls)
		free(p->repls);
	if (p->free_error0)
		free(p->error0);
	if (p->free_error1)
		free(p->error1);
	if (p->free_error2)
		free(p->error2);
	p = p->comp;
	while (p != NULL) {
		t = p;
		p = p->comp;
		if (t->free_prefix)
			free(t->prefix);
		if (t->free_suffix0)
			free(t->suffix0);
		if (t->free_suffix1)
			free(t->suffix1);
		if (t->free_suffix2)
			free(t->suffix2);
		if (t->free_fusion0)
			free(t->fusion0);
		if (t->free_fusion1)
			free(t->fusion1);
		if (t->free_fusion2)
			free(t->fusion2);
		if (t->free_trans)
			free(t->trans);
		if (t->free_repls)
			free(t->repls);
		if (t->free_error0)
			free(t->error0);
		if (t->free_error1)
			free(t->error1);
		if (t->free_error2)
			free(t->error2);
		free(t);
	}
}

static void morSearchSytaxUsage(char *f) {
	if (f != NULL) {
		printf("\n%c%c@ Followed by file name or morpho-syntax\n", f[0], f[1]);
		puts("Morpho-syntax markers specify the nature of the following string");
	}
	puts("    # prefix marker");
	puts("    | part-of-speech marker");
	puts("    r stem of the word marker, can also be ';' instead of 'r'");
	puts("    - suffix marker");
	puts("    & nonconcatenated morpheme marker");
	puts("    = English translation for the stem marker");
	puts("    @ replacement word preceding [: ...] code marker");
	puts("    * error code inside [* ...] code marker");
	puts("    o all other elements not specified by user");
	if (f != NULL) {
		puts("  followed by - or + and/or the following");
		puts("    * find any match");
		puts("    % erase any match");
		puts("    string find \"string\"");
		puts("  , the comma separates alternative elements ");
		puts("\nException:");
		puts("    ~ postclitic OR $ preclitic");
		puts("  followed by");
		puts("    Morpho-elements	find words with preclitic OR postclitic Morpho elements");
		puts("    %				if preclitic OR postclitic is present, then erase it");
		puts("\nFor example:");
		puts("    +t%mor -t* +s\"@r-*,o-%\"");
		puts("  find all stems and erase all other markers");
		puts("    +t%mor -t* +s\"@r*,|adv,o%\"");
		puts("  find all stems of all \"adv\" and erase all other markers");
		puts("    +t%mor -t* +s\"@r-be\"");
		puts("  find all forms of \"be\" verb");
		puts("    +t%mor -t* +s\"@r*,|*,o%\"");
		puts("  find all stems and parts-of-speech and erase other markers");
		puts("    +t%mor -t* +s\"@r*,|*,-*,o%\"");
		puts("  find only stems and parts-of-speech that have suffixes and erase other markers");
		puts("    +t%mor -t* +s\"@r-*,|-*,-+*,o-%\"");
		puts("  find all stems, parts-of-speech and distinguish those with suffix and erase other markers");
		puts("    +t%mor -t* -s@\"-*\" +s\"@r*,|*,o%\"");
		puts("  find only stems and parts-of-speech that do not have suffixes and erase other markers");
		puts("    +t%mor -t* +s@\"|n,|n:*,~|poss\"");
		puts("  find only noun words with \"poss\" parts-of-speech postclitic");
		puts("    +t%mor -t* +s@\"|n,|n:*,~o%\"");
		puts("  find only noun words with postclitics and remove postclitics");
		puts("    +t%mor -t* +s@\"|n,|n:*,~%\"");
		puts("  find all noun words and if they have postclitics then remove it");
		cutt_exit(0);
	} else {
		puts("    $ preclitic");
		puts("    ~ postclitic");
	}
}

char isMorSearchListGiven(void) {
	if (morfptr == NULL)
		return(FALSE);
	else
		return(TRUE);
}

static MORFEATURESLIST *makeMorFeatList(char *wd, char *orgWd, char *isClitic, char *res) {
	char *comma;
	size_t len;
	MORFEATURESLIST *tItem;

	if (*wd != '#' && *wd != '~' && *wd != '$' && *wd != '|' && *wd != 'r' && *wd != ';' && *wd != '-' &&
		*wd != '&' && *wd != '=' && *wd != '@' && *wd != '*' && *wd != 'o') {
		printf("\n  Illegal symbol \"%c\" used in option \"%s\".\n  Please use one the following symbols:\n", *wd, orgWd);
		morSearchSytaxUsage(NULL);
		if (res != NULL)
			*res = FALSE;
		return(NULL);
	}
	if ((tItem=NEW(MORFEATURESLIST)) == NULL) {
		return(NULL);
	} else {
		comma = strchr(wd, ',');
		if (comma != NULL)
			*comma = EOS;
		if (*wd == '~') {
			tItem->typeID = '~';
			isPostcliticUse = TRUE;
			*isClitic = TRUE;
			wd++;
		} else if (*wd == '$') {
			tItem->typeID = '$';
			isPrecliticUse = TRUE;
			*isClitic = TRUE;
			wd++;
		} else
			tItem->typeID = ' ';
		if ((tItem->typeID == '~' || tItem->typeID == '$') && *wd == '%') {
			tItem->featType = tItem->typeID;
			tItem->flag = '-';
			len = strlen(wd);
			tItem->pat = (char *)malloc(2);
			if (tItem->pat == NULL) {
				free(tItem);
				if (comma != NULL)
					*comma = ',';
				return(NULL);
			}
			strcpy(tItem->pat, "%");
		} else {
			if ((tItem->typeID == '~' || tItem->typeID == '$') && *wd == EOS) {
				fprintf(stderr, "\n  Please specify Morpho-elements or '%%' after symbol '%c' in option \"%s\".\n", tItem->typeID, orgWd);
				free(tItem);
				if (comma != NULL)
					*comma = ',';
				if (res != NULL)
					*res = FALSE;
				return(NULL);
			} else if (*wd == EOS) {
				fprintf(stderr, "\n  Please specify all Morpho-elements in option \"%s\".\n", orgWd);
				free(tItem);
				if (comma != NULL)
					*comma = ',';
				if (res != NULL)
					*res = FALSE;
				return(NULL);
			}
			tItem->featType = *wd++;
			if (tItem->featType == ';')
				tItem->featType = 'r';
			if (tItem->featType == '@' || tItem->featType == '*')
				R8 = TRUE;
			if (*wd == '-' || *wd == '+')
				tItem->flag = *wd++;
			else
				tItem->flag = '-';
			len = strlen(wd);
			if (len == 0) {
				fprintf(stderr, "\n  Please specify '*' or '%%' or string after symbol \"%c%c\" in option \"%s\".\n", tItem->typeID, tItem->featType, orgWd);
				free(tItem);
				if (comma != NULL)
					*comma = ',';
				if (res != NULL)
					*res = FALSE;
				return(NULL);
			}
			tItem->pat = (char *)malloc(len+1);
			if (tItem->pat == NULL) {
				free(tItem);
				if (comma != NULL)
					*comma = ',';
				return(NULL);
			}
			strcpy(tItem->pat, wd);
		}
		tItem->nextFeat = NULL;
		if (comma != NULL) {
			tItem->nextFeat = makeMorFeatList(comma+1, orgWd, isClitic, res);
			if (tItem->nextFeat == NULL) {
				if (tItem->pat != NULL)
					free(tItem->pat);
				free(tItem);
				if (comma != NULL)
					*comma = ',';
				return(NULL);
			}
		}
		if (comma != NULL)
			*comma = ',';
	}
	return(tItem);
}

MORWDLST *makeMorWordList(MORWDLST *root, char *res, char *wd, char ch) {
	char st[BUFSIZ], colaps[] = "o-%";
	MORWDLST *tItem, *p;
	MORFEATURESLIST *l, *m;

	if (res != NULL)
		*res = TRUE;
	if ((tItem=NEW(MORWDLST)) == NULL) {
		root = freeMorWords(root);
		return(NULL);
	}
	strncpy(st, wd, BUFSIZ-2);
	st[BUFSIZ-1] = EOS;
	tItem->type = ch;
	tItem->isClitic = FALSE;
	tItem->rootFeat = makeMorFeatList(wd, st, &tItem->isClitic, res);
	tItem->nextMatch = NULL;
	if (tItem->rootFeat == NULL) {
		root = freeMorWords(root);
		return(NULL);
	}
	for (l=tItem->rootFeat; l != NULL; l=l->nextFeat) {
		for (m=tItem->rootFeat; m != NULL; m=m->nextFeat) {
			if (l != m && l->featType == m->featType && m->featType != '|' && m->featType != 'r' && m->featType != '*') {
				printf("\nPlease do not use the same element twice \"%c-%s\" and \"%c-%s\" in option \"%s\".\n", l->featType, l->pat, m->featType, m->pat, st);
				root = freeMorWords(root);
				if (res != NULL)
					*res = FALSE;
				return(NULL);
			}
		}
	}
	if (tItem->type != 'i' && tItem->rootFeat != NULL) {
		for (l=tItem->rootFeat; l->nextFeat != NULL; l=l->nextFeat) {
			if (l->featType == 'o')
				break;
		}
		if (l->featType != 'o') {
			l->nextFeat = makeMorFeatList(colaps, st, &tItem->isClitic, res);
			if (l->nextFeat == NULL) {
				root = freeMorWords(root);
				return(NULL);
			}
		}
	}
	if (root == NULL)
		root = tItem;
	else {
		for (p=root; p->nextMatch != NULL; p=p->nextMatch) ;
		p->nextMatch = tItem;
	}
	return(root);
}

static char isPOSFound(char *line) {
	while (*line != EOS && *line != sMarkChr) {
		if (*line == '|')
			return(TRUE);
		line++;
	}
	return(FALSE);
}

void filterMorTier(char *morUtt, char *morLine, char isReplace) {
	int  i;
	char spaceOut, isDMark;

	if (linkMain2Mor) {
		if (isMorSearchListGiven()) {
			isDMark = TRUE;
			for (i=0; morUtt[i] != EOS; i++) {
				if (morUtt[i] == sMarkChr)
					isDMark = FALSE;
				else if (morUtt[i] == dMarkChr)
					isDMark = TRUE;
				if (isDMark && !isPostcliticUse && morUtt[i] == '~' && isPOSFound(morUtt+i+1))
					morUtt[i] = ' ';
				if (isDMark && !isPrecliticUse && morUtt[i] == '$' && isPOSFound(morUtt+i+1))
					morUtt[i] = ' ';
			}
		}
	} else if (isMorSearchListGiven()) {
		for (i=0; morUtt[i] != EOS; i++) {
			if (!isPostcliticUse && morUtt[i] == '~')
				morUtt[i] = ' ';
			if (!isPrecliticUse && morUtt[i] == '$')
				morUtt[i] = ' ';
			if (isReplace) {
				if (morUtt[i] == '\001')
					morUtt[i] = '@';
				else if (morUtt[i] == '\002')
					morUtt[i] = '*';
			}
		}
		if (morLine != NULL) {
			if (isReplace == 1) {
				spaceOut = FALSE;
				for (i=0; morLine[i] != EOS; i++) {
					if (uS.isskip(morLine,i,&dFnt,MBF))
						spaceOut = FALSE;
					if (morLine[i] == '\001' || morLine[i] == '\002')
						spaceOut = TRUE;
					if (spaceOut)
						morLine[i] = ' ';
				}
			} else if (isReplace == 2) {
				for (i=0; morLine[i] != EOS; i++) {
					if (morLine[i] == '\001')
						morLine[i] ='@';
					else if (morLine[i] == '\002')
						morLine[i] ='*';
				}
			}
		}
	}
}

static char isMorSearchOption(char *f, char ch) {
	if (ch != '-' && ch != '+')
		return(FALSE);
	if (*f != '@')
		return(FALSE);
	f++;
	if (*f == '+' || *f == '~')
		f++;
	if (*f == '#') // prefix
		return(TRUE);
	else if (*f == '~') // postclitic
		return(TRUE);
	else if (*f == '$') // preclitic
		return(TRUE);
	else if (*f == '|') // part of speech
		return(TRUE);
	else if (*f == 'r') {// stem
		f++;
		if (*f == '-' || *f == '+' || *f == '*' || *f == '%')
			return(TRUE);
	} else if (*f == ';') // stem
		return(TRUE);
	else if (*f == '-') // suffix
		return(TRUE);
	else if (*f == '&') // fusion
		return(TRUE);
	else if (*f == '=') // trans
		return(TRUE);
	else if (*f == '@') // repls
		return(TRUE);
	else if (*f == '*') // errors
		return(TRUE);
	else if (*f == 'o') {// other (default *)
		f++;
		if (*f == '-' || *f == '+' || *f == '*' || *f == '%')
			return(TRUE);
	}
	return(FALSE);
}

static char isMorWord(char c, char isErrReplFound) {
	if (isErrReplFound) {
		if (c != '@' && c != '*' && c != EOS)
			return(TRUE);			
	} else {
		if (c != '^' && c != '~' && c != '$' && c != '+' && c != '=' && c != '#' && c != '|' && c != '-' && c != '&' && c != '@' && c != '*' && c != EOS)
			return(TRUE);			
	}
	return(FALSE);
}

static void copyCompoundElems(MORFEATS *root) {
	MORFEATS *elem;

	for (elem=root->comp; elem != NULL; elem=elem->comp) {
		if (elem->type == NULL || elem->typeID != '+') 
			break;
		if (elem->prefix != NULL && root->prefix == NULL) {
			root->prefix = (char *)malloc(strlen(elem->prefix)+1);
			if (root->prefix != NULL) {
				root->free_prefix = TRUE;
				strcpy(root->prefix, elem->prefix);
			}
		}
		if (elem->suffix0 != NULL) {
			if (root->suffix0 == NULL) {
				root->suffix0 = (char *)malloc(strlen(elem->suffix0)+1);
				if (root->suffix0 != NULL) {
					root->free_suffix0 = TRUE;
					strcpy(root->suffix0, elem->suffix0);
				}
			} else if (root->suffix1 == NULL) {
				root->suffix1 = (char *)malloc(strlen(elem->suffix0)+1);
				if (root->suffix1 != NULL) {
					root->free_suffix1 = TRUE;
					strcpy(root->suffix1, elem->suffix0);
				}
			} else if (root->suffix2 == NULL) {
				root->suffix2 = (char *)malloc(strlen(elem->suffix0)+1);
				if (root->suffix2 != NULL) {
					root->free_suffix2 = TRUE;
					strcpy(root->suffix2, elem->suffix0);
				}
			}
		}
		if (elem->suffix1 != NULL) {
			if (root->suffix0 == NULL) {
				root->suffix0 = (char *)malloc(strlen(elem->suffix1)+1);
				if (root->suffix0 != NULL) {
					root->free_suffix0 = TRUE;
					strcpy(root->suffix0, elem->suffix1);
				}
			} else if (root->suffix1 == NULL) {
				root->suffix1 = (char *)malloc(strlen(elem->suffix1)+1);
				if (root->suffix1 != NULL) {
					root->free_suffix1 = TRUE;
					strcpy(root->suffix1, elem->suffix1);
				}
			} else if (root->suffix2 == NULL) {
				root->suffix2 = (char *)malloc(strlen(elem->suffix1)+1);
				if (root->suffix2 != NULL) {
					root->free_suffix2 = TRUE;
					strcpy(root->suffix2, elem->suffix1);
				}
			}
		}
		if (elem->suffix2 != NULL) {
			if (root->suffix0 == NULL) {
				root->suffix0 = (char *)malloc(strlen(elem->suffix2)+1);
				if (root->suffix0 != NULL) {
					root->free_suffix0 = TRUE;
					strcpy(root->suffix0, elem->suffix2);
				}
			} else if (root->suffix1 == NULL) {
				root->suffix1 = (char *)malloc(strlen(elem->suffix2)+1);
				if (root->suffix1 != NULL) {
					root->free_suffix1 = TRUE;
					strcpy(root->suffix1, elem->suffix2);
				}
			} else if (root->suffix2 == NULL) {
				root->suffix2 = (char *)malloc(strlen(elem->suffix2)+1);
				if (root->suffix2 != NULL) {
					root->free_suffix2 = TRUE;
					strcpy(root->suffix2, elem->suffix2);
				}
			}
		}
		if (elem->fusion0 != NULL) {
			if (root->fusion0 == NULL) {
				root->fusion0 = (char *)malloc(strlen(elem->fusion0)+1);
				if (root->fusion0 != NULL) {
					root->free_fusion0 = TRUE;
					strcpy(root->fusion0, elem->fusion0);
				}
			} else if (root->fusion1 == NULL) {
				root->fusion1 = (char *)malloc(strlen(elem->fusion0)+1);
				if (root->fusion1 != NULL) {
					root->free_fusion1 = TRUE;
					strcpy(root->fusion1, elem->fusion0);
				}
			} else if (root->fusion2 == NULL) {
				root->fusion2 = (char *)malloc(strlen(elem->fusion0)+1);
				if (root->fusion2 != NULL) {
					root->free_fusion2 = TRUE;
					strcpy(root->fusion2, elem->fusion0);
				}
			}
		}
		if (elem->fusion1 != NULL) {
			if (root->fusion0 == NULL) {
				root->fusion0 = (char *)malloc(strlen(elem->fusion1)+1);
				if (root->fusion0 != NULL) {
					root->free_fusion0 = TRUE;
					strcpy(root->fusion0, elem->fusion1);
				}
			} else if (root->fusion1 == NULL) {
				root->fusion1 = (char *)malloc(strlen(elem->fusion1)+1);
				if (root->fusion1 != NULL) {
					root->free_fusion1 = TRUE;
					strcpy(root->fusion1, elem->fusion1);
				}
			} else if (root->fusion2 == NULL) {
				root->fusion2 = (char *)malloc(strlen(elem->fusion1)+1);
				if (root->fusion2 != NULL) {
					root->free_fusion2 = TRUE;
					strcpy(root->fusion2, elem->fusion1);
				}
			}
		}
		if (elem->fusion2 != NULL) {
			if (root->fusion0 == NULL) {
				root->fusion0 = (char *)malloc(strlen(elem->fusion2)+1);
				if (root->fusion0 != NULL) {
					root->free_fusion0 = TRUE;
					strcpy(root->fusion0, elem->fusion2);
				}
			} else if (root->fusion1 == NULL) {
				root->fusion1 = (char *)malloc(strlen(elem->fusion2)+1);
				if (root->fusion1 != NULL) {
					root->free_fusion1 = TRUE;
					strcpy(root->fusion1, elem->fusion2);
				}
			} else if (root->fusion2 == NULL) {
				root->fusion2 = (char *)malloc(strlen(elem->fusion2)+1);
				if (root->fusion2 != NULL) {
					root->free_fusion2 = TRUE;
					strcpy(root->fusion2, elem->fusion2);
				}
			}
		}
		if (elem->trans != NULL && root->trans == NULL) {
			root->trans = (char *)malloc(strlen(elem->trans)+1);
			if (root->trans != NULL) {
				root->free_trans = TRUE;
				strcpy(root->trans, elem->trans);
			}
		}
		if (elem->repls != NULL && root->repls == NULL) {
			root->repls = (char *)malloc(strlen(elem->repls)+1);
			if (root->repls != NULL) {
				root->free_repls = TRUE;
				strcpy(root->repls, elem->repls);
			}
		}
		if (elem->error0 != NULL) {
			if (root->error0 == NULL) {
				root->error0 = (char *)malloc(strlen(elem->error0)+1);
				if (root->error0 != NULL) {
					root->free_error0 = TRUE;
					strcpy(root->error0, elem->error0);
				}
			} else if (root->error1 == NULL) {
				root->error1 = (char *)malloc(strlen(elem->error0)+1);
				if (root->error1 != NULL) {
					root->free_error1 = TRUE;
					strcpy(root->error1, elem->error0);
				}
			} else if (root->error2 == NULL) {
				root->error2 = (char *)malloc(strlen(elem->error0)+1);
				if (root->error2 != NULL) {
					root->free_error2 = TRUE;
					strcpy(root->error2, elem->error0);
				}
			}
		}
		if (elem->error1 != NULL) {
			if (root->error0 == NULL) {
				root->error0 = (char *)malloc(strlen(elem->error1)+1);
				if (root->error0 != NULL) {
					root->free_error0 = TRUE;
					strcpy(root->error0, elem->error1);
				}
			} else if (root->error1 == NULL) {
				root->error1 = (char *)malloc(strlen(elem->error1)+1);
				if (root->error1 != NULL) {
					root->free_error1 = TRUE;
					strcpy(root->error1, elem->error1);
				}
			} else if (root->error2 == NULL) {
				root->error2 = (char *)malloc(strlen(elem->error1)+1);
				if (root->error2 != NULL) {
					root->free_error2 = TRUE;
					strcpy(root->error2, elem->error1);
				}
			}
		}
		if (elem->error2 != NULL) {
			if (root->error0 == NULL) {
				root->error0 = (char *)malloc(strlen(elem->error2)+1);
				if (root->error0 != NULL) {
					root->free_error0 = TRUE;
					strcpy(root->error0, elem->error2);
				}
			} else if (root->error1 == NULL) {
				root->error1 = (char *)malloc(strlen(elem->error2)+1);
				if (root->error1 != NULL) {
					root->free_error1 = TRUE;
					strcpy(root->error1, elem->error2);
				}
			} else if (root->error2 == NULL) {
				root->error2 = (char *)malloc(strlen(elem->error2)+1);
				if (root->error2 != NULL) {
					root->free_error2 = TRUE;
					strcpy(root->error2, elem->error2);
				}
			}
		}
	}
}

char ParseWordIntoFeatures(char *item, MORFEATS *word_feats) {
	char *c, type, t, isErrReplFound;

	word_feats->free_prefix = FALSE;
	word_feats->prefix  = NULL;
	word_feats->pos     = NULL;
	word_feats->stem    = NULL;
	word_feats->free_suffix0 = FALSE;
	word_feats->suffix0 = NULL;
	word_feats->free_suffix1 = FALSE;
	word_feats->suffix1 = NULL;
	word_feats->free_suffix2 = FALSE;
	word_feats->suffix2 = NULL;
	word_feats->free_fusion0 = FALSE;
	word_feats->fusion0 = NULL;
	word_feats->free_fusion1 = FALSE;
	word_feats->fusion1 = NULL;
	word_feats->free_fusion2 = FALSE;
	word_feats->fusion2 = NULL;
	word_feats->free_trans   = FALSE;
	word_feats->trans   = NULL;
	word_feats->free_repls   = FALSE;
	word_feats->repls   = NULL;
	word_feats->free_error0  = FALSE;
	word_feats->error0  = NULL;
	word_feats->free_error1  = FALSE;
	word_feats->error1  = NULL;
	word_feats->free_error2  = FALSE;
	word_feats->error2  = NULL;
	word_feats->comp    = NULL;
	for (c=item; *c != EOS && *c != '+' && *c != '*' && *c != '@'; c++) ;
	if (*c == '+') {
		*c = EOS;
		word_feats->comp = NEW(MORFEATS);
		if (word_feats->comp != NULL) {
			word_feats->comp->type = c;
			word_feats->comp->typeID = '+';
			if (ParseWordIntoFeatures(c+1, word_feats->comp) == FALSE)
				return(FALSE);
		} else {
			return(FALSE);
		}
	}
	for (c=item; *c != EOS && *c != '^' && *c != '*' && *c != '@'; c++) ;
	if (*c == '^') {
		*c = EOS;
		word_feats->comp = NEW(MORFEATS);
		if (word_feats->comp != NULL) {
			word_feats->comp->type = c;
			word_feats->comp->typeID = '^';
			if (ParseWordIntoFeatures(c+1, word_feats->comp) == FALSE)
				return(FALSE);
		} else {
			return(FALSE);
		}
	}
	for (c=item; *c != EOS && *c != '~' && *c != '*' && *c != '@'; c++) ;
	if (*c == '~') {
		*c = EOS;
		word_feats->comp = NEW(MORFEATS);
		if (word_feats->comp != NULL) {
			word_feats->comp->type = c;
			word_feats->comp->typeID = '~';
			if (ParseWordIntoFeatures(c+1, word_feats->comp) == FALSE)
				return(FALSE);
		} else {
			return(FALSE);
		}
	}
	for (c=item; *c != EOS && *c != '$' && *c != '*' && *c != '@'; c++) ;
	if (*c == '$') {
		*c = EOS;
		word_feats->comp = NEW(MORFEATS);
		if (word_feats->comp != NULL) {
			word_feats->comp->type = c;
			word_feats->comp->typeID = '$';
			if (ParseWordIntoFeatures(c+1, word_feats->comp) == FALSE)
				return(FALSE);
		} else {
			return(FALSE);
		}
	}
	for (c=item; *c != EOS && *c != '#' && *c != '*' && *c != '@'; c++) ;
	if (*c == '#') {
		*c = EOS;
		word_feats->prefix = item;
		item = c + 1;
	}
	for (c=item; *c != EOS && *c != '|' && *c != '*' && *c != '@'; c++) ;
	if (*c == '|') {
		*c = EOS;
		word_feats->pos = item;
		item = c + 1;
	}
	isErrReplFound = FALSE;
	for (c=item; isMorWord(*c, isErrReplFound); c++) {
		if (*c == '*' || *c == '@')
			isErrReplFound = TRUE;
	}
	type = *c;
	*c = EOS;
	word_feats->stem = item;
	item = c + 1;

	while (type != EOS) {
		while (type == '\n' || isSpace(type))
			type = *item++;
		if (type == '*' || type == '@')
			isErrReplFound = TRUE;
		for (c=item; isMorWord(*c, isErrReplFound); c++) ;
		if (type == '@' && *c == '@') {
			for (c++; isMorWord(*c, isErrReplFound); c++) ;
		}
		t = *c;
		*c = EOS;
		if (type == '-') {
			if (word_feats->suffix0 == NULL)
				word_feats->suffix0 = item;
			else if (word_feats->suffix1 == NULL)
				word_feats->suffix1 = item;
			else
				word_feats->suffix2 = item;
		} else if (type == '&') {
			if (word_feats->fusion0 == NULL)
				word_feats->fusion0 = item;
			else if (word_feats->fusion1 == NULL)
				word_feats->fusion1 = item;
			else
				word_feats->fusion2 = item;
		} else if (type == '=') {
			word_feats->trans = item;
		} else if (type == '@') {
			word_feats->repls = item;
		} else if (type == '*') {
			if (word_feats->error0 == NULL)
				word_feats->error0 = item;
			else if (word_feats->error1 == NULL)
				word_feats->error1 = item;
			else
				word_feats->error2 = item;
		} else {
			for (; *item != EOS; item++)
				*item = ' ';
		}
		type = t;
		item = c + 1;
	}
	if (word_feats->typeID == 'R' && word_feats->stem != NULL && word_feats->comp != NULL) {
		if (word_feats->stem[0] == EOS && word_feats->comp->type != NULL && word_feats->comp->typeID == '+') {
			copyCompoundElems(word_feats);
		}
	}
	return(TRUE);
}

char matchToMorFeatures(MORWDLST *pat_rootFeat, MORFEATS *word_feats, char isSkipRestOfCompound, char isClean) {
	char *c, isMatched, isPreCMatched, isPostCMatched;
	char isWordHasTrans, isWordHasRepls, isWordHasError, isCliticPat, isTryMatch;
	char mPrefix,
		mPos,
		mStem,
		mSuffix0,
		mSuffix1,
		mSuffix2,
		mFusion0,
		mFusion1,
		mFusion2,
		mTrans,
		mRepls,
		mError0,
		mError1,
		mError2;
	MORFEATS *word_feat;
	MORFEATURESLIST *pat_feats, *pat_feat;

	pat_feats = pat_rootFeat->rootFeat;
	isCliticPat = pat_rootFeat->isClitic;
	isWordHasTrans = FALSE;
	isWordHasRepls = FALSE;
	isWordHasError = FALSE;
	for (word_feat=word_feats; word_feat != NULL; word_feat=word_feat->comp) {
		if (word_feat->trans != NULL)
			isWordHasTrans = TRUE;
		if (word_feat->repls != NULL)
			isWordHasRepls = TRUE;
		if (word_feat->error0 != NULL)
			isWordHasError = TRUE;
	}
	if (isCliticPat) {
		isMatched      = TRUE;
		isPreCMatched  = TRUE;
		isPostCMatched = TRUE;
	} else
		isMatched = FALSE;
	mTrans = 0;
	mRepls = 0;
	mError0 = 0;
	mError1 = 0;
	mError2 = 0;
	for (pat_feat=pat_feats; pat_feat != NULL; pat_feat=pat_feat->nextFeat) {
		if (isCliticPat) {
			if (pat_feat->typeID == '~') {
				if (pat_feat->featType != '~' || strcmp(pat_feat->pat, "%"))
					isPostCMatched = FALSE;
			} else if (pat_feat->typeID == '$') {
				if (pat_feat->featType != '$' || strcmp(pat_feat->pat, "%"))
					isPreCMatched = FALSE;
			} else {
				isMatched = FALSE;
			}
		}
		for (word_feat=word_feats; word_feat != NULL; word_feat=word_feat->comp) {
			if ((!R7Slash || !R7Tilda || !R7Caret || !R7Colon) && word_feat->typeID == '^')
				break;
			if (pat_feat->featType == '=') {
				if (!isWordHasTrans) {
					if (mTrans != 1 && pat_feat->flag != '+') {
//						if (word_feat->trans != NULL || strcmp(pat_feat->pat, "*"))
							mTrans = -1;
					}
					break;
				}
				if (word_feat->trans != NULL) {
					if (uS.patmat(word_feat->trans, pat_feat->pat)) {
						mTrans = 1;
					} else {
						mTrans = -1;
					}
				}
			} else if (pat_feat->featType == '@') {
				if (!isWordHasRepls) {
					if (mRepls != 1 && pat_feat->flag != '+')
						mRepls = -1;
					break;
				}
				if (word_feat->repls != NULL) {
					if (uS.patmat(word_feat->repls, pat_feat->pat)) {
						mRepls = 1;
					} else {
						mRepls = -1;
					}
				}
			} else if (pat_feat->featType == '*') {
				if (!isWordHasError) {
					if (pat_feat->flag != '+') {
						if (mError0 != 1)
							mError0 = -1;
						if (mError1 != 1)
							mError1 = -1;
						if (mError2 != 1)
							mError2 = -1;
					}
					break;
				}
				if (word_feat->error0 != NULL && uS.patmat(word_feat->error0, pat_feat->pat)) {
					mError0 = 1;
				} else if (mError0 != 1 && pat_feat->flag != '+') {
					mError0 = -1;
				}
				if (word_feat->error1 != NULL && uS.patmat(word_feat->error1, pat_feat->pat)) {
					mError1 = 1;
				} else if (mError1 != 1 && pat_feat->flag != '+') {
					mError1 = -1;
				}
				if (word_feat->error2 != NULL && uS.patmat(word_feat->error2, pat_feat->pat)) {
					mError2 = 1;
				} else if (mError2 != 1 && pat_feat->flag != '+') {
					mError2 = -1;
				}
			}
		}
	}
	for (word_feat=word_feats; word_feat != NULL; word_feat=word_feat->comp) {
		if ((!R7Slash || !R7Tilda || !R7Caret || !R7Colon) && word_feat->typeID == '^')
			break;
		if (isSkipRestOfCompound && word_feat->type != NULL && word_feat->typeID == '+')
			continue;
		mPrefix = 0;
		mPos = 0;
		mStem = 0;
		mSuffix0 = 0;
		mSuffix1 = 0;
		mSuffix2 = 0;
		mFusion0 = 0;
		mFusion1 = 0;
		mFusion2 = 0;
		for (pat_feat=pat_feats; pat_feat != NULL; pat_feat=pat_feat->nextFeat) {
			if (isCliticPat) {
				if ((word_feat->typeID == '~' || word_feat->typeID == '$') && word_feat->typeID == pat_feat->typeID)
					isTryMatch = TRUE;
				else if (word_feat->typeID != '~' && word_feat->typeID != '$' && pat_feat->typeID != '~' && pat_feat->typeID != '$')
					isTryMatch = TRUE;
				else
					isTryMatch = FALSE;
			} else
				isTryMatch = TRUE;
			if (pat_feat->featType == 'o') {
			} else if (pat_feat->featType == '#') {
				if (word_feat->prefix != NULL && uS.patmat(word_feat->prefix, pat_feat->pat)) {
					mPrefix = 1;
				} else if (word_feat->prefix != NULL || (mPrefix != 1 && pat_feat->flag != '+')) {
//					if (word_feat->prefix != NULL || strcmp(pat_feat->pat, "*"))
						mPrefix = -1;
				}
			} else if (pat_feat->featType == '|') {
				if (isTryMatch) {
					if (word_feat->type != NULL && word_feat->typeID == '+' && mPos == 0)
						mPos = -1;
					else if (word_feat->pos != NULL && uS.patmat(word_feat->pos, pat_feat->pat)) {
						mPos = 1;
					} else if (mPos != 1 && pat_feat->flag != '+') {
						mPos = -1;
					}
				}
			} else if (pat_feat->featType == 'r') {
				if (isTryMatch) {
					if (word_feat->stem != NULL &&
						  (uS.patmat(word_feat->stem,pat_feat->pat) || (word_feat->stem[0]==EOS && !strcmp(pat_feat->pat,"*")))) {
						mStem = 1;
					} else if (mStem != 1 && pat_feat->flag != '+') {
						mStem = -1;
					}
				}
			} else if (pat_feat->featType == '-') {
				if (word_feat->suffix0 != NULL && uS.patmat(word_feat->suffix0, pat_feat->pat)) {
					mSuffix0 = 1;
				} else if (word_feat->suffix0 != NULL || (mSuffix0 != 1 && pat_feat->flag != '+')) {
//					if (word_feat->suffix0 != NULL || strcmp(pat_feat->pat, "*"))
						mSuffix0 = -1;
				}
				if (word_feat->suffix1 != NULL && uS.patmat(word_feat->suffix1, pat_feat->pat)) {
					mSuffix1 = 1;
				} else if (word_feat->suffix1 != NULL || (mSuffix1 != 1 && pat_feat->flag != '+')) {
//					if (word_feat->suffix1 != NULL || strcmp(pat_feat->pat, "*"))
						mSuffix1 = -1;
				}
				if (word_feat->suffix2 != NULL && uS.patmat(word_feat->suffix2, pat_feat->pat)) {
					mSuffix2 = 1;
				} else if (word_feat->suffix2 != NULL || (mSuffix2 != 1 && pat_feat->flag != '+')) {
//					if (word_feat->suffix2 != NULL || strcmp(pat_feat->pat, "*"))
						mSuffix2 = -1;
				}
			} else if (pat_feat->featType == '&') {
				if (word_feat->fusion0 != NULL && uS.patmat(word_feat->fusion0, pat_feat->pat)) {
					mFusion0 = 1;
				} else if (word_feat->fusion0 != NULL || (mFusion0 != 1 && pat_feat->flag != '+')) {
//					if (word_feat->fusion0 != NULL || strcmp(pat_feat->pat, "*"))
						mFusion0 = -1;
				}
				if (word_feat->fusion1 != NULL && uS.patmat(word_feat->fusion1, pat_feat->pat)) {
					mFusion1 = 1;
				} else if (word_feat->fusion1 != NULL || (mFusion1 != 1 && pat_feat->flag != '+')) {
//					if (word_feat->fusion1 != NULL || strcmp(pat_feat->pat, "*"))
						mFusion1 = -1;
				}
				if (word_feat->fusion2 != NULL && uS.patmat(word_feat->fusion2, pat_feat->pat)) {
					mFusion2 = 1;
				} else if (word_feat->fusion2 != NULL || (mFusion2 != 1 && pat_feat->flag != '+')) {
//					if (word_feat->fusion2 != NULL || strcmp(pat_feat->pat, "*"))
						mFusion2 = -1;
				}
			}
		}
		if (isCliticPat) {
			for (pat_feat=pat_feats; pat_feat != NULL; pat_feat=pat_feat->nextFeat) {
				if (((pat_feat->featType == '~' && pat_feat->typeID == '~') && word_feat->typeID == '~') ||
					  ((pat_feat->featType == '$' && pat_feat->typeID == '$') && word_feat->typeID == '$')) {
					if (mPrefix == 0 && !isStringEmpty(word_feat->prefix)) {
						if (uS.patmat(word_feat->prefix, pat_feat->pat)) {
							mPrefix = 1;
						} else {
							mPrefix = -1;
						}
					}
					if (mPos == 0 && !isStringEmpty(word_feat->pos)) {
						if (uS.patmat(word_feat->pos, pat_feat->pat)) {
							mPos = 1;
						} else {
							mPos = -1;
						}
					}
					if (mStem == 0 && !isStringEmpty(word_feat->stem)) {
						if (uS.patmat(word_feat->stem, pat_feat->pat)) {
							mStem = 1;
						} else {
							mStem = -1;
						}
					}
					if (mSuffix0 != 1 && !isStringEmpty(word_feat->suffix0)) {
						if (uS.patmat(word_feat->suffix0, pat_feat->pat)) {
							if (mSuffix0 == 0) mSuffix0 = 1;
						} else {
							if (mSuffix0 == 0) mSuffix0 = -1;
						}
					}
					if (mSuffix1 != 1 && !isStringEmpty(word_feat->suffix1)) {
						if (uS.patmat(word_feat->suffix1, pat_feat->pat)) {
							if (mSuffix1 == 0) mSuffix1 = 1;
						} else {
							if (mSuffix1 == 0) mSuffix1 = -1;
						}
					}
					if (mSuffix2 != 1 && !isStringEmpty(word_feat->suffix2)) {
						if (uS.patmat(word_feat->suffix2, pat_feat->pat)) {
							if (mSuffix2 == 0) mSuffix2 = 1;
						} else {
							if (mSuffix2 == 0) mSuffix2 = -1;
						}
					}
					if (mFusion0 != 1 && !isStringEmpty(word_feat->fusion0)) {
						if (uS.patmat(word_feat->fusion0, pat_feat->pat)) {
							if (mFusion0 == 0) mFusion0 = 1;
						} else {
							if (mFusion0 == 0) mFusion0 = -1;
						}
					}
					if (mFusion1 != 1 && !isStringEmpty(word_feat->fusion1)) {
						if (uS.patmat(word_feat->fusion1, pat_feat->pat)) {
							if (mFusion1 == 0) mFusion1 = 1;
						} else {
							if (mFusion1 == 0) mFusion1 = -1;
						}
					}
					if (mFusion2 != 1 && !isStringEmpty(word_feat->fusion2)) {
						if (uS.patmat(word_feat->fusion2, pat_feat->pat)) {
							if (mFusion2 == 0) mFusion2 = 1;
						} else {
							if (mFusion2 == 0) mFusion2 = -1;
						}
					}
					if (mTrans == 0 && !isStringEmpty(word_feat->trans)) {
						if (uS.patmat(word_feat->trans, pat_feat->pat)) {
							mTrans = 1;
						} else {
							mTrans = -1;
						}
					}
					if (mRepls == 0 && !isStringEmpty(word_feat->repls)) {
						if (uS.patmat(word_feat->repls, pat_feat->pat)) {
							mRepls = 1;
						} else {
							mRepls = -1;
						}
					}
					if (mError0 == 0 && !isStringEmpty(word_feat->error0)) {
						if (uS.patmat(word_feat->error0, pat_feat->pat)) {
							mError0 = 1;
						} else {
							mError0 = -1;
						}
					}
					if (mError1 == 0 && !isStringEmpty(word_feat->error1)) {
						if (uS.patmat(word_feat->error1, pat_feat->pat)) {
							mError1 = 1;
						} else {
							mError1 = -1;
						}
					}
					if (mError2 == 0 && !isStringEmpty(word_feat->error2)) {
						if (uS.patmat(word_feat->error2, pat_feat->pat)) {
							mError2 = 1;
						} else {
							mError2 = -1;
						}
					}
				}
			}
		}
		for (pat_feat=pat_feats; pat_feat != NULL; pat_feat=pat_feat->nextFeat) {
			if (pat_feat->featType == 'o') {
				if (mPrefix == 0 && !isStringEmpty(word_feat->prefix)) {
					if (uS.patmat(word_feat->prefix, pat_feat->pat)) {
						mPrefix = 1;
					} else {
						mPrefix = -1;
					}
				}
				if (mPos == 0 && !isStringEmpty(word_feat->pos)) {
					if (uS.patmat(word_feat->pos, pat_feat->pat)) {
						mPos = 1;
					} else {
						mPos = -1;
					}
				}
				if (mStem == 0 && !isStringEmpty(word_feat->stem)) {
					if (uS.patmat(word_feat->stem, pat_feat->pat)) {
						mStem = 1;
					} else {
						mStem = -1;
					}
				}
				if (mSuffix0 != 1 && !isStringEmpty(word_feat->suffix0)) {
					if (uS.patmat(word_feat->suffix0, pat_feat->pat)) {
						if (mSuffix0 == 0) mSuffix0 = 1;
					} else {
						if (mSuffix0 == 0) mSuffix0 = -1;
					}
				}
				if (mSuffix1 != 1 && !isStringEmpty(word_feat->suffix1)) {
					if (uS.patmat(word_feat->suffix1, pat_feat->pat)) {
						if (mSuffix1 == 0) mSuffix1 = 1;
					} else {
						if (mSuffix1 == 0) mSuffix1 = -1;
					}
				}
				if (mSuffix2 != 1 && !isStringEmpty(word_feat->suffix2)) {
					if (uS.patmat(word_feat->suffix2, pat_feat->pat)) {
						if (mSuffix2 == 0) mSuffix2 = 1;
					} else {
						if (mSuffix2 == 0) mSuffix2 = -1;
					}
				}
				if (mFusion0 != 1 && !isStringEmpty(word_feat->fusion0)) {
					if (uS.patmat(word_feat->fusion0, pat_feat->pat)) {
						if (mFusion0 == 0) mFusion0 = 1;
					} else {
						if (mFusion0 == 0) mFusion0 = -1;
					}
				}
				if (mFusion1 != 1 && !isStringEmpty(word_feat->fusion1)) {
					if (uS.patmat(word_feat->fusion1, pat_feat->pat)) {
						if (mFusion1 == 0) mFusion1 = 1;
					} else {
						if (mFusion1 == 0) mFusion1 = -1;
					}
				}
				if (mFusion2 != 1 && !isStringEmpty(word_feat->fusion2)) {
					if (uS.patmat(word_feat->fusion2, pat_feat->pat)) {
						if (mFusion2 == 0) mFusion2 = 1;
					} else {
						if (mFusion2 == 0) mFusion2 = -1;
					}
				}
				if (mTrans == 0 && !isStringEmpty(word_feat->trans)) {
					if (uS.patmat(word_feat->trans, pat_feat->pat)) {
						mTrans = 1;
					} else {
						mTrans = -1;
					}
				}
				if (mRepls == 0 && !isStringEmpty(word_feat->repls)) {
					if (uS.patmat(word_feat->repls, pat_feat->pat)) {
						mRepls = 1;
					} else {
						mRepls = -1;
					}
				}
				if (mError0 == 0 && !isStringEmpty(word_feat->error0)) {
					if (uS.patmat(word_feat->error0, pat_feat->pat)) {
						mError0 = 1;
					} else {
						mError0 = -1;
					}
				}
				if (mError1 == 0 && !isStringEmpty(word_feat->error1)) {
					if (uS.patmat(word_feat->error1, pat_feat->pat)) {
						mError1 = 1;
					} else {
						mError1 = -1;
					}
				}
				if (mError2 == 0 && !isStringEmpty(word_feat->error2)) {
					if (uS.patmat(word_feat->error2, pat_feat->pat)) {
						mError2 = 1;
					} else {
						mError2 = -1;
					}
				}
			}
		}
		if (mPrefix != -1 && mPos != -1 && mStem != -1 && mTrans != -1 && mRepls != -1 && 
			(mSuffix0 != -1 || mSuffix1 != -1 || mSuffix2 != -1) &&
			(mFusion0 != -1 || mFusion1 != -1 || mFusion2 != -1) &&
			(mError0 != -1  || mError1 != -1  || mError2 != -1)) {
			if (isCliticPat) {
				if (word_feat->typeID == '~')
					isPostCMatched = TRUE;
				else if (word_feat->typeID == '$')
					isPreCMatched = TRUE;
				else
					isMatched = TRUE;
			} else
				isMatched = TRUE;
		} else if (isPrecliticUse || isPostcliticUse) {
		} else if (isClean && word_feat->typeID != '+' && (word_feat->comp == NULL || word_feat->comp->typeID != '+')) {
			if (word_feat->prefix != NULL) {
				for (c=word_feat->prefix; *c != EOS; c++)
					*c = ' ';
			}
			if (word_feat->pos != NULL) {
				for (c=word_feat->pos; *c != EOS; c++)
					*c = ' ';
			}
			if (word_feat->stem != NULL) {
				for (c=word_feat->stem; *c != EOS; c++)
					*c = ' ';
			}
			if (word_feat->suffix0 != NULL) {
				for (c=word_feat->suffix0; *c != EOS; c++)
					*c = ' ';
			}
			if (word_feat->suffix1 != NULL) {
				for (c=word_feat->suffix1; *c != EOS; c++)
					*c = ' ';
			}
			if (word_feat->suffix2 != NULL) {
				for (c=word_feat->suffix2; *c != EOS; c++)
					*c = ' ';
			}
			if (word_feat->fusion0 != NULL) {
				for (c=word_feat->fusion0; *c != EOS; c++)
					*c = ' ';
			}
			if (word_feat->fusion1 != NULL) {
				for (c=word_feat->fusion1; *c != EOS; c++)
					*c = ' ';
			}
			if (word_feat->fusion2 != NULL) {
				for (c=word_feat->fusion2; *c != EOS; c++)
					*c = ' ';
			}
			if (mTrans == -1 && word_feat->trans != NULL) {
				for (c=word_feat->trans; *c != EOS; c++)
					*c = ' ';
			}
			if (mRepls == -1 && word_feat->repls != NULL) {
				for (c=word_feat->repls; *c != EOS; c++)
					*c = ' ';
			}
			if (mError0 == -1 && word_feat->error0 != NULL) {
				for (c=word_feat->error0; *c != EOS; c++)
					*c = ' ';
			}
			if (mError1 == -1 && word_feat->error1 != NULL) {
				for (c=word_feat->error1; *c != EOS; c++)
					*c = ' ';
			}
			if (mError2 == -1 && word_feat->error2 != NULL) {
				for (c=word_feat->error2; *c != EOS; c++)
					*c = ' ';
			}
		}
	}
	if (isCliticPat) {
		if (isMatched && isPreCMatched && isPostCMatched)
			return(TRUE);
		else
			return(FALSE);
	} else
		return(isMatched);
}

static void cleanUpMorExcludeWord(char *word, char *matched) {
	char isFoundTransOrRepls, isErrReplFound;
	long i, j, c;

	isFoundTransOrRepls = FALSE;
	for (i=0; word[i] != EOS; i++) {
		if (!isSpace(matched[i]) && matched[i] != EOS) {
			for (j=i; j >= 0; j--) {
				if ((word[j] == '^' || word[j] == '~' || word[j] == '$') && !isFoundTransOrRepls)
					break;
				if (word[j] == '=' || word[j] == '@' || word[j] == '*')
					isFoundTransOrRepls = TRUE;
				word[j] = ' ';
			}
			for (; word[i] != EOS; i++) {
				if ((word[i]=='^' || word[i]=='~' || word[i]=='$' || word[i]=='=' || word[i]=='@' || word[i]=='*') && !isFoundTransOrRepls)
					break;
				word[i] = ' ';
			}
			if (word[i] == EOS)
				break;
		}
	}
	for (i=0; word[i] != EOS && isSpace(word[i]); i++) ;
	if ((word[i] == '@' || word[i] == '*') && (isSpace(matched[i]) || matched[i] == EOS) && isSpace(matched[i+1])) {
		for (; word[i] != EOS; i++) {
			if (isSpace(matched[i]) || matched[i] == EOS)
				word[i] = ' ';
		}
	}
	for (i=0; word[i] != EOS; ) {
		if (isSpace(word[i]))
			strcpy(word+i, word+i+1);
		else
			i++;
	}
	if (word[0] == '@' || word[0] == '*')
		isErrReplFound = TRUE;
	else
		isErrReplFound = FALSE;
	while (word[0] != EOS && !isMorWord(word[0], isErrReplFound)) {
		if (word[0] == '@' || word[0] == '*')
			isErrReplFound = TRUE;
		strcpy(word, word+1);
	}
	for (c=0; word[c] != EOS && word[c] != '@' && word[c] != '*'; c++) ;
	if (word[c] == '@' || word[c] == '*')
		isErrReplFound = TRUE;
	else
		isErrReplFound = FALSE;
	for (i=strlen(word)-1; i >= 0 && !isMorWord(word[i], isErrReplFound); i--) {
		if (c == i)
			isErrReplFound = FALSE;
		strcpy(word+i, word+i+1);
	}
	isErrReplFound = FALSE;
	for (i=0; word[i] != EOS; ) {
		if (word[i]=='@' || word[i]=='*')
			isErrReplFound = TRUE;
		if ((word[i]=='~' || word[i]=='$' || word[i]=='^' || word[i]=='+' || word[i]=='=' || word[i]=='@' || word[i]=='*') && !isMorWord(word[i+1], isErrReplFound))
			strcpy(word+i, word+i+1);
		else
			i++;
	}
	if (word[0] == '=' || word[0] == '@' || word[0] == '*') {
		while (word[0] != EOS)
			strcpy(word, word+1);
	}
}

static char isMorAllBeforeSpace(char *matched, char *word, long i) {
	while (i >= 0) {
		if (!isSpace(matched[i]) && matched[i] != EOS)
			return(FALSE);
		if (word[i] == '+' || word[i] == '^' || word[i] == '~' || word[i] == '$')
			break;
		i--;
	}
	return(TRUE);
}

static void cleanUpMorWord(char *word, char *matched) {
	char isErrReplFound;
	long i, c;

	isErrReplFound = FALSE;
	for (i=0; word[i] != EOS; i++) {
		if (word[i]=='@' || word[i]=='*')
			isErrReplFound = TRUE;
		if (isSpace(matched[i]) || matched[i] == EOS) {
			if (isMorWord(word[i], isErrReplFound))
				word[i] = ' ';
			else if ((word[i] == '#' || word[i] == '|') && isMorAllBeforeSpace(matched, word, i-1))
				word[i] = ' ';
			else if (matched[i] == EOS && word[i] == '|' && matched[i+1]==EOS && word[i+1] == '+')
				;
			else if (word[i]!='+' && word[i]!='^' && word[i]!='~' && word[i]!='$' && word[i]!='#' && (isSpace(matched[i+1]) || matched[i+1]==EOS))
				word[i] = ' ';
		}
	}
	for (i=0; word[i] != EOS; ) {
		if (isSpace(word[i]))
			strcpy(word+i, word+i+1);
		else
			i++;
	}
	if (word[0] == '@' || word[0] == '*')
		isErrReplFound = TRUE;
	else
		isErrReplFound = FALSE;
	while (word[0] != EOS && !isMorWord(word[0], isErrReplFound)) {
		if (word[0] == '@' || word[0] == '*')
			isErrReplFound = TRUE;
		strcpy(word, word+1);
	}
	for (c=0; word[c] != EOS && word[c] != '@' && word[c] != '*'; c++) ;
	if (word[c] == '@' || word[c] == '*')
		isErrReplFound = TRUE;
	else
		isErrReplFound = FALSE;
	for (i=strlen(word)-1; i >= 0 && !isMorWord(word[i], isErrReplFound); i--) {
		if (c == i)
			isErrReplFound = FALSE;
		strcpy(word+i, word+i+1);
	}
	isErrReplFound = FALSE;
	for (i=0; word[i] != EOS; ) {
		if (word[i]=='@' || word[i]=='*')
			isErrReplFound = TRUE;
		if ((word[i] == '~' || word[i] == '$' || word[i] == '^' || word[i] == '+') && !isMorWord(word[i+1], isErrReplFound))
			strcpy(word+i, word+i+1);
		else
			i++;
	}
}

void removeMainTierWords(char *line) {
	int b, e, t;
	char dataFound;

	if (strchr(line, dMarkChr) == NULL)
		return;
	for (b=0; isSpace(line[b]); b++) ;
	while (line[b] != EOS) {
		if (line[b] == dMarkChr) {
			for (e=b+1; line[e] != EOS && line[e] != sMarkChr; e++) ;
			if (line[e] == sMarkChr) {
				t = e;
				dataFound = FALSE;
				for (e++; line[e] != EOS && line[e] != dMarkChr; e++) {
					if (!isSpace(line[e]))
						dataFound = TRUE;
				}
				line[b] = ' ';
				if (!dataFound) {
					t = b + 1;
				}
				for (b=t; b < e; b++)
					line[b] = ' ';
			} else {
				line[b] = ' ';
				b = e;
			}
		} else
			b++;
	}
}

void removeDepTierItems(char *line) {
	int  b, e;
	char dataFound;

	if (strchr(line, dMarkChr) == NULL)
		return;
	for (b=0; isSpace(line[b]); b++) ;
	while (line[b] != EOS) {
		if (line[b] == dMarkChr) {
			dataFound = FALSE;
			for (e=b+1; line[e] != EOS && line[e] != sMarkChr; e++) {
				if (!isSpace(line[e]))
					dataFound = TRUE;
			}
			if (!dataFound) {
				for (; line[e] != EOS && line[e] != dMarkChr; e++) ;
			}
			if (line[e] == sMarkChr)
				e += 1;
			for (; b < e; b++)
				line[b] = ' ';
		} else
			b++;
	}
}

int getNextDepTierPair(char *line, char *morItem, char *spWord, int *wi, int i) {
	int j, k;

	morItem[0] = EOS;
	spWord[0] = EOS;
	for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
	if (line[i] == EOS)
		return(0);
	if (line[i] == dMarkChr) {
		i++;
		for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
	}
	if (line[i] == EOS)
		return(0);
	if (wi != NULL)
		*wi = i;
	k = 0;
	for (; line[i] != sMarkChr && !isSpace(line[i]) && line[i] != '\n' && line[i] != EOS; i++) {
		morItem[k++] = line[i];
	}
	morItem[k] = EOS;
	uS.remFrontAndBackBlanks(morItem);
	for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
	if (line[i] == EOS)
		return(i);
	j = i + 1;
	if (line[i] != sMarkChr) {
		for (; line[j] != sMarkChr && line[j] != EOS; j++) ;
		if (line[j] == sMarkChr)
			j++;
	}
	k = 0;
	for (; line[j] != dMarkChr && line[j] != EOS; j++) {
		spWord[k++] = line[j];
	}
	spWord[k] = EOS;
	uS.remFrontAndBackBlanks(spWord);
	if (line[i] == sMarkChr)
		i = j;
	return(i);
}

static int LinkQuoteMorScope(char *wline, int pos, char isFirst) {
	int tPos;
	char isLetFound;

	if (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) && pos >= 0) {
		while (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) && pos >= 0) {
			if (isFirst)
				wline[pos] = ' ';
			pos--;
		}
		if (uS.isRightChar(wline, pos, '[', &dFnt, MBF)) {
			if (isFirst)
				wline[pos] = ' ';
		}
	}
	if (pos < 0) {
		fprintf(stderr,"Missing '[' character in file %s\n",oldfname);
		fprintf(stderr,"In tier on line: %ld.\n", lineno);
		fprintf(stderr,"text=%s->%s\n",wline,wline+pos);
		return(-2);
	} else
		pos--;
	while (!uS.isRightChar(wline, pos, '>', &dFnt, MBF) && uS.isskip(wline,pos,&dFnt,MBF) && pos >= 0) {
		if (uS.isRightChar(wline, pos, ']', &dFnt, MBF) || uS.isRightChar(wline, pos, '<', &dFnt, MBF))
			break;
		else
			pos--;
	}
	if (uS.isRightChar(wline, pos, '>', &dFnt, MBF)) {
		isLetFound = FALSE;
		while (!uS.isRightChar(wline, pos, '<', &dFnt, MBF) && pos >= 0) {
			if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
				pos = LinkQuoteMorScope(wline,pos,FALSE);
			else {
				if (isFirst) {
					if (!uS.isskip(wline,pos,&dFnt,MBF))
						isLetFound = TRUE;
					if (isSpace(wline[pos]) && isLetFound)
						wline[pos] = fSpaceChr;
				}
				pos--;
			}
		}
		if (pos < 0) {
			fprintf(stderr,"Missing '<' character in file %s\n", oldfname);
			fprintf(stderr,"In tier on line: %ld.\n", lineno+tlineno);
			fprintf(stderr,"text=%s\n",wline);
			return(-2);
		} else {
			tPos = pos;
			if (wline[tPos] == '<')
				tPos++;
			for (; isSpace(wline[tPos]) || wline[tPos] == fSpaceChr; tPos++)
				wline[tPos] = ' ';
		}
	} else if (pos < 0) ;
	else if (uS.isRightChar(wline, pos, ']', &dFnt, MBF)) {
		pos = LinkQuoteMorScope(wline,pos,FALSE);
	} else {
		while (!uS.isskip(wline,pos,&dFnt,MBF) && pos >= 0) {
			if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
				pos = LinkQuoteMorScope(wline,pos,FALSE);
			else {
				pos--;
			}
		}
	}
	return(pos);
}

static char isQuotes(char *wline, int pos) {
	if (UTF8_IS_LEAD((unsigned char)wline[pos-2])) {
		if (wline[pos-2] == (char)0xE2 && wline[pos-1] == (char)0x80 && wline[pos] == (char)0x9C) {
			return(TRUE);
		} else if (wline[pos-2] == (char)0xE2 && wline[pos-1] == (char)0x80 && wline[pos] == (char)0x9D) {
			return(TRUE);
		} else if (wline[pos-2] == (char)0xE2 && wline[pos-1] == (char)0x80 && wline[pos] == (char)0x98) {
			return(TRUE);
		} else if (wline[pos-2] == (char)0xE2 && wline[pos-1] == (char)0x80 && wline[pos] == (char)0x99) {
			return(TRUE);
		} 
	}
	return(FALSE);
}

static int ExcludeMorScope(char *wline, int pos, char isblankit) {
	if (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) && pos >= 0) {
		while (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) && pos >= 0) {
			if (isblankit) {
				if (isQuotes(wline, pos)) {
					pos -= 2;
				} else
					wline[pos] = ' ';
			}
			pos--;
		}
		if (uS.isRightChar(wline, pos, '[', &dFnt, MBF)) {
			if (isblankit)
				wline[pos] = ' ';
		}
	}
	if (pos < 0) {
		fprintf(stderr,"Missing '[' character in file %s\n",oldfname);
		fprintf(stderr,"In tier on line: %ld.\n", lineno);
		fprintf(stderr,"text=%s->%s\n",wline,wline+pos);
		return(-2);
	} else
		pos--;
	while (!uS.isRightChar(wline, pos, '>', &dFnt, MBF) && uS.isskip(wline,pos,&dFnt,MBF) && pos >= 0) {
		if (uS.isRightChar(wline, pos, ']', &dFnt, MBF) || uS.isRightChar(wline, pos, '<', &dFnt, MBF))
			break;
		else
			pos--;
	}
	if (uS.isRightChar(wline, pos, '>', &dFnt, MBF)) {
		while (!uS.isRightChar(wline, pos, '<', &dFnt, MBF) && pos >= 0) {
			if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
				pos = ExcludeMorScope(wline,pos,isblankit);
			else {
				if (isblankit) {
					if (isQuotes(wline, pos)) {
						pos -= 2;
					} else
						wline[pos] = ' ';
				}
				pos--;
			}
		}
		if (pos < 0) {
			fprintf(stderr,"Missing '<' character in file %s\n", oldfname);
			fprintf(stderr,"In tier on line: %ld.\n", lineno+tlineno);
			fprintf(stderr,"text=%s\n",wline);
			return(-2);
		} else if (isblankit)
			wline[pos] = ' ';
	} else if (pos < 0) ;
	else if (uS.isRightChar(wline, pos, ']', &dFnt, MBF)) {
		pos = ExcludeMorScope(wline,pos,isblankit);
	} else {
		while (!uS.isskip(wline,pos,&dFnt,MBF) && pos >= 0) {
			if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
				pos = ExcludeMorScope(wline,pos,isblankit);
			else {
				if (isblankit) {
					if (isQuotes(wline, pos)) {
						pos -= 2;
					} else
						wline[pos] = ' ';
				}
				pos--;
			}
		}
	}
	return(pos);
}

static void getRepls(char *repls, char *errors, int beg, int end, char *wline) {
	int  ri, ei;

	ri = 0;
	repls[ri++] = ' ';
	repls[ri++] = '\001';
	ei = 0;
	errors[ei++] = ' ';
	while (beg < end) {
		for (; beg < end && isspace(wline[beg]); beg++) ;
		if (beg >= end)
			break;
		if (wline[beg] == '[' && wline[beg+1] == '*' && isSpace(wline[beg+2])) {
			errors[ei++] = '\002';
			for (beg=beg+2; beg < end && wline[beg] != ']'; beg++) {
				if (wline[beg] == (char)0xE2 && wline[beg+1] == (char)0x80 && 
					  (wline[beg+2] == (char)0x9C || wline[beg+2] == (char)0x9D ||
					   wline[beg+2] == (char)0x98 || wline[beg+2] == (char)0x99)) {
					beg += 2;
				} else if (wline[beg] == ',' && !isdigit(wline[beg+1])) {
				} else if (!isSpace(wline[beg]))
					errors[ei++] = wline[beg];
			}
			if (beg < end)
				beg++;
		} else if (wline[beg] == '[') {
			for (beg++; beg < end && wline[beg] != ']'; beg++) ;
			if (beg < end)
				beg++;
		} else if (wline[beg] == '&' || wline[beg] == '0' || wline[beg] == '+' || wline[beg] == '-') {
			for (; beg < end && !isspace(wline[beg]); beg++) ;
		} else if (!isspace(wline[beg]) && wline[beg] != '<' && wline[beg] != '>') {
			if (ri > 2) {
				repls[ri++] = 0xe2;
				repls[ri++] = 0x80;
				repls[ri++] = 0xaf;
			}
			for (; beg < end && !isspace(wline[beg]); beg++) {
				if (wline[beg] == (char)0xE2 && wline[beg+1] == (char)0x80 && 
					  (wline[beg+2] == (char)0x9C || wline[beg+2] == (char)0x9D ||
					   wline[beg+2] == (char)0x98 || wline[beg+2] == (char)0x99)) {
					beg += 2;
				} else if (wline[beg] == ',' && !isdigit(wline[beg+1])) {
				} else
					repls[ri++] = wline[beg];
			}
		} else
			beg++;
	}
	repls[ri] = EOS;
	errors[ei] = EOS;
}

static void addRepls(char *repls, int beg, int end, char *wline) {
	int i, j;

	while (beg < end) {
		for (; beg < end && isspace(wline[end]); end--) ;
		if (beg < end && !isspace(wline[end])) {
			uS.shiftright(wline+end+1, strlen(repls));
			for (i=end+1,j=0; repls[j] != EOS; i++, j++)
				wline[i] = repls[j];
			for (; beg < end && !isspace(wline[end]); end--) ;
		}
	}
}

static int countWords(char *wline, int beg, int end) {
	int  cnt;
	char sq, counted;

	cnt = 0;
	counted = FALSE;
	if (uS.isRightChar(wline, beg, '[', &dFnt, MBF))
		sq = TRUE;
	else
		sq = FALSE;
	for (; beg < end; beg++) {
		if (uS.isRightChar(wline, beg, '[', &dFnt, MBF))
			sq = TRUE;
		if (!uS.isskip(wline,beg,&dFnt,MBF) && !counted && !sq) {
			cnt++;
			counted = TRUE;
		}
		if (isSpace(wline[beg]) && !sq)
			counted = FALSE;
		if (uS.isRightChar(wline, beg, ']', &dFnt, MBF))
			sq = FALSE;
	}
	return(cnt);
}

static IEWORDS *InsertCASearch(IEWORDS *CAptr, char *str) {
	IEWORDS *t;

	if ((t=NEW(IEWORDS)) == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}			
	t->word = (char *)malloc(strlen(str)+1);
	if (t->word == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}
	strcpy(t->word, str);
	t->nextword = CAptr;
	CAptr = t;
	return(CAptr);
}

void IsSearchCA(char *wline) {
	char CAc[20];
	int cur, end, matchType;

	cur = 0;
	while (wline[cur]) {
		if ((end=uS.HandleCAChars(wline+cur, &matchType)) != 0) {
			if (matchType == NOTCA_LEFT_ARROW_CIRCLE)
				isSpecRepeatArrow = TRUE;
			strncpy(CAc, wline+cur, end);
			CAc[end] = EOS;
			CAptr = InsertCASearch(CAptr, CAc);
			cur += end;
		} else
			cur++;
	}
}

static int isCAsearched(char *word) {
	IEWORDS *twd;

	for (twd=CAptr; twd != NULL; twd = twd->nextword) {
		if (strcmp(word, twd->word) == 0) {
			return(TRUE);
		}
	}
	return(FALSE);
}

static void removeRepeatSegments(char *wline, int beg, int len) {
	int i;

	for (i=beg+1; wline[i] != EOS; i++) {
		if (wline[i] == (char)0xE2 && wline[i+1] == (char)0x86 && wline[i+2] == (char)0xAB)
			break;
	}
	if (wline[i] == EOS) {
		strcpy(wline+beg,wline+beg+len);
	} else {
		strcpy(wline+beg,wline+i+3);
	}
}

int isMorExcludePlus(char *w) {
	//	if (w[0] == '+') {
	if ((w[1] == '"' || w[1] == '^' || w[1] == ',' || w[1] == '+' || w[1] == '<') && w[2] == EOS)
		return(TRUE);
	if (w[1] == EOS)
		return(TRUE);
	//	}
	return(FALSE);
}

/* excludeMordef(word) determines if %mor: item only "word" is to be included in
 the analyses or not by compairing the "word" to the list of words in "wdptr". It
 returns 1 if the word should be included, and 0 otherwise.
 */
static int excludeMordef(char *word) {
	IEWORDS *twd;

	for (twd=defwdptr; twd != NULL; twd = twd->nextword) {
		if (strchr(twd->word, '|') != NULL && uS.patmat(word, twd->word)) {
			if (word[0] != '-')
				return(FALSE);
			else if (uS.isToneUnitMarker(word))
				return(FALSE);
		}
	}
	if (uS.mStricmp(word, "unk|xxx") == 0 || uS.mStricmp(word, "unk|yyy") == 0 || uS.patmat(word, "*|www") ||
		  uS.patmat(word, "*|xx") || uS.patmat(word, "*|yy")) {
		return(FALSE);
	} else if (word[0] == '+') {
		if (isMorExcludePlus(word))
			return(FALSE);
	}
	return(TRUE);
}

/* excludeSpMordef(word) determines if %mor: item only "word" is to be included in
 the analyses or not by compairing the "word" to the list of words in "wdptr". It
 returns 1 if the word should be included, and 0 otherwise.
 */
static int excludeSpMordef(char *word) {
	if (uS.mStricmp(word, "xx") == 0 || uS.mStricmp(word, "xxx") == 0 || strcmp(word, "0") == 0 ||
		  uS.mStricmp(word, "yy") == 0 || uS.mStricmp(word, "yyy") == 0 || uS.mStricmp(word, "www") == 0 ||
		  uS.mStrnicmp(word,"xxx@s",5) == 0 || uS.mStrnicmp(word,"yyy@s",5) == 0 || uS.mStrnicmp(word,"www@s",5) == 0 ||
		  word[0] == '&' || word[0] == '#' || word[0] == '-' || word[0] == '[') {
		return(FALSE);
	} else if (word[0] == '+') {
		if (isMorExcludePlus(word))
			return(FALSE);
	}
	return(TRUE);
}

static void filterMorscop(char *wline, char linkSp2Mor) {
	char repls[BUFSIZ+1], errors[BUFSIZ+1], CAc[20];
	int i, pos, lastNonSkip, cur, end, begRepls, sqCnt, matchType, org_word_cnt;

	cur = 0;
	while (wline[cur]) {
		if ((end=uS.HandleCAChars(wline+cur, &matchType)) != 0) {
			if (isRemoveCAChar[matchType] == TRUE &&
				matchType != NOTCA_DOUBLE_COMMA && matchType != NOTCA_VOCATIVE &&
				matchType != NOTCA_OPEN_QUOTE   && matchType != NOTCA_CLOSE_QUOTE   &&
				matchType != NOTCA_OPEN_S_QUOTE && matchType != NOTCA_CLOSE_S_QUOTE) {
				strncpy(CAc, wline+cur, end);
				CAc[end] = EOS;
				if (!isCAsearched(CAc)) {
					if (matchType == NOTCA_LEFT_ARROW_CIRCLE)
						removeRepeatSegments(wline, cur, end);
					else
						strcpy(wline+cur,wline+cur+end);
				} else
					cur += end;
			} else
				cur += end;
			continue;
		}
		cur++;
	}
	pos = strlen(wline) - 1;
	lastNonSkip = pos;
	while (pos >= 0) {
		if (wline[pos] == HIDEN_C) {
			end = pos;
			for (pos--; wline[pos] != HIDEN_C && pos >= 0; pos--) ;
			if (wline[pos] == HIDEN_C && pos >= 0) {
				for (; end >= pos; end--)
					wline[end] = ' ';
			}
		} else if (uS.isRightChar(wline, pos, ']', &dFnt, MBF)) {
			sqCnt = 0;
			end = pos;
			for (pos--; (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) || sqCnt > 0) && pos >= 0; pos--) {
				if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
					sqCnt++;
				else if (uS.isRightChar(wline, pos, '[', &dFnt, MBF))
					sqCnt--;
			}
			if (pos < 0) {
				if (chatmode) {
					if (!cutt_isBlobFound) {
						fprintf(stderr,"Missing '[' character in file %s\n",oldfname);
						fprintf(stderr,"In tier on line: %ld.\n", lineno+tlineno);
						fprintf(stderr,"text=%s\n",wline);
					}
					pos = end - 1;
					if (isRecursive || cutt_isCAFound || cutt_isBlobFound)
						continue;
					else
						cutt_exit(1);
				} else
					break;
			}
			wline[end] = EOS;
			uS.isSqCodes(wline+pos+1, templineC3, &dFnt, TRUE);
			wline[end] = ']';
			uS.lowercasestr(templineC3, &dFnt, MBF);
			if (!strcmp(templineC3, "/?") || !strcmp(templineC3, "/-") || !strcmp(templineC3, "///") || 
				!strcmp(templineC3, "//") || !strcmp(templineC3, "/")) {
				ExcludeMorScope(wline, pos, TRUE);
				for (; !uS.isRightChar(wline, end, '[', &dFnt, MBF); end--)
					wline[end] = ' ';
				wline[end] = ' ';
				for (; end < lastNonSkip; end++) {
					if (!isSpace(wline[end]) && wline[end] != '\n')
						wline[end] = ' ';
				}
			} else if (!strcmp(templineC3, "\"") ) {
				LinkQuoteMorScope(wline, pos, TRUE);
				for (; !uS.isRightChar(wline, end, '[', &dFnt, MBF); end--)
					wline[end] = ' ';
				wline[end] = ' ';
				for (; end < lastNonSkip; end++) {
					if (!isSpace(wline[end]) && wline[end] != '\n')
						wline[end] = ' ';
				}
			} else if (linkSp2Mor) {
				if (uS.patmat(templineC3,":: *")) {
					if (!R5_1) {
						for (; !uS.isRightChar(wline, end, '[', &dFnt, MBF); end--)
							wline[end] = ' ';
						wline[end] = ' ';
					} else {
						begRepls = ExcludeMorScope(wline, pos, FALSE);
						if (isSpace(wline[begRepls]))
							begRepls++;
						org_word_cnt = countWords(wline, begRepls, pos);
						for (cur=begRepls; cur < pos; cur++)
							wline[cur] = ' ';
						wline[end] = ' ';
						cur = pos;
						lastNonSkip = pos;
						wline[cur++] = ' ';
						if (wline[cur] == ':')
							wline[cur++] = ' ';
						if (wline[cur] == ':')
							wline[cur++] = ' ';
						for (cur=pos; cur < end && isSpace(wline[cur]); cur++) ;
						for (; cur < end && isSpace(wline[end]); end--) ;
						end++;
						i = 0;
						for (; cur < end; cur++) {
							if (isSpace(wline[cur]))
								wline[cur] = fSpaceChr;
							if (i < BUFSIZ)
								repls[i++] = wline[cur];
						}
						repls[i] = EOS;
						if (org_word_cnt > 1) {
							strcpy(wline+begRepls, wline+end);
							i = (org_word_cnt * (strlen(repls) + 1)) - 1;
							pos = begRepls;
							lastNonSkip = pos;
							cur = begRepls;
							uS.shiftright(wline+cur, i);
							for (; org_word_cnt > 0; org_word_cnt--) {
								for (i=0; repls[i] != EOS; i++) {
									wline[cur++] = repls[i];
								}
								if (org_word_cnt > 1)
									wline[cur++] = ' ';
							}
						}
					}
				} else if (uS.patmat(templineC3,": *") || uS.patmat(templineC3,":\\* *") || uS.patmat(templineC3,":=_ *")) {
					if (!R5) {
						begRepls = ExcludeMorScope(wline, pos, FALSE);
						if (isSpace(wline[begRepls]))
							begRepls++;
						wline[end] = ' ';
						cur = pos;
						wline[cur++] = ' ';
						if (wline[cur] == ':')
							wline[cur++] = ' ';
						if (wline[cur] == '*')
							wline[cur++] = ' ';
						if (wline[cur] == '=') {
							wline[cur++] = ' ';
							wline[cur++] = ' ';
						}
						lastNonSkip = begRepls;
						org_word_cnt = countWords(wline, pos, end);
						for (; cur < end; cur++)
							wline[cur] = ' ';
						for (cur=begRepls; cur < pos && isSpace(wline[cur]); cur++) ;
						for (; cur < pos && isSpace(wline[pos]); pos--) ;
						pos++;
						i = 0;
						for (; cur < pos; cur++) {
							if (isSpace(wline[cur]))
								wline[cur] = fSpaceChr;
							if (i < BUFSIZ)
								repls[i++] = wline[cur];
						}
						repls[i] = EOS;
						if (org_word_cnt > 1) {
							strcpy(wline+begRepls, wline+end);
							i = (org_word_cnt * (strlen(repls) + 1)) - 1;
							pos = begRepls;
							lastNonSkip = pos;
							cur = begRepls;
							uS.shiftright(wline+cur, i);
							for (; org_word_cnt > 0; org_word_cnt--) {
								for (i=0; repls[i] != EOS; i++) {
									wline[cur++] = repls[i];
								}
								if (org_word_cnt > 1)
									wline[cur++] = ' ';
							}
						}
					} else {
					ExcludeMorScope(wline, pos, TRUE);
					wline[end] = ' ';
						cur = pos;
						lastNonSkip = pos;
						wline[cur++] = ' ';
						if (wline[cur] == ':')
							wline[cur++] = ' ';
						if (wline[cur] == '*')
							wline[cur++] = ' ';
						if (wline[cur] == '=') {
							wline[cur++] = ' ';
							wline[cur++] = ' ';
						}
					}
				} else {
					for (; !uS.isRightChar(wline, end, '[', &dFnt, MBF); end--)
						wline[end] = ' ';
					wline[end] = ' ';
				}
			} else if (uS.patmat(templineC3,"\\* *-*")) {
				for (; !uS.isRightChar(wline, end, '[', &dFnt, MBF); end--)
					wline[end] = ' ';
				wline[end] = ' ';
			} else if (uS.patmat(templineC3,"\\* *")) {
				cur = end;
				for (; !uS.isRightChar(wline, cur, '[', &dFnt, MBF); cur--) ;
				if (pos <= cur) {
					wline[end] = ' ';
					wline[cur++] = ' ';
					while (cur < end) {
						if (wline[cur] == '*')
							wline[cur] = '\002';
						if (isspace(wline[cur])) {
							strcpy(wline+cur, wline+cur+1);
							end--;
						} else
							cur++;
					}
				}
			} else if (uS.patmat(templineC3,": *") || uS.patmat(templineC3,":: *") || uS.patmat(templineC3,":\\* *") || uS.patmat(templineC3,":=_ *")) {
				begRepls = ExcludeMorScope(wline,pos,FALSE);
				if (begRepls > -2) {
					if (begRepls < 0)
						begRepls = 0;
					cur = end;
					wline[cur] = ' ';
					for (; !uS.isRightChar(wline, cur, '[', &dFnt, MBF); cur--) ;
					lastNonSkip = cur;
					wline[cur] = ' ';
					wline[cur+1] = ' ';
					if (wline[cur+2] == ':')
						wline[cur+2] = ' ';
					if (wline[cur+2] == '*')
						wline[cur+2] = ' ';
					if (wline[cur+2] == '=' && !isSpace(wline[cur+3])) {
						wline[cur+2] = ' ';
						wline[cur+3] = ' ';
					}
					if (pos <= cur) {
						if (uS.patmat(templineC3,":: *")) {
							getRepls(repls, errors, cur, end, wline);
						} else
							getRepls(repls, errors, begRepls, cur, wline);
						templineC3[0] = EOS;
						if (repls[0] != EOS)
							strcat(templineC3, repls);
						if (errors[0] != EOS)
							strcat(templineC3, errors);
						if (templineC3[0] != EOS)
							addRepls(templineC3, cur, end, wline);
					}
					for (; begRepls < cur; begRepls++)
						wline[begRepls] = ' ';
				}
			} else if (uS.patmat(templineC3,"\"")) {
//				ExcludeMorScope(wline,pos, TRUE);
				for (; !uS.isRightChar(wline, end, '[', &dFnt, MBF); end--)
					wline[end] = ' ';
				wline[end] = ' ';
			} else {
				for (; !uS.isRightChar(wline, end, '[', &dFnt, MBF); end--)
					wline[end] = ' ';
				wline[end] = ' ';
			}
		} else {
			if (!uS.isskip(wline,pos,&dFnt,MBF) && !uS.isRightChar(wline,pos,'[',&dFnt,MBF) && !uS.isRightChar(wline,pos,']',&dFnt,MBF))
				lastNonSkip = pos;
			pos--;
		}
	}
}

static char isRightMorUttDelim(char *w, char linkSp2Mor) {
	int i;

	if (linkSp2Mor && w[0] == '+') {
		for (i=0; w[i] != EOS; i++) {
			if (uS.IsUtteranceDel(w, i))
				return(TRUE);
		}
	}
	return(FALSE);
}

static void ChangeBackSpace(char *s) {
	for (; *s != EOS; s++) {
		if (*s == fSpaceChr)
			*s = ' ';
	}
}

void createMorUttline(char *new_mor_tier, char *spTier, char *mor_tier, char linkSp2Mor) {
	int i, j, t;
	char spWord[BUFSIZ], MorWord[BUFSIZ], spTierDone, dMark[4], sMark[4], missMatch, isCMFound, res;

//strcpy(new_mor_tier, mor_tier);
//return;

/*
 addword('\0','\0',"+</?>");
 addword('\0','\0',"+</->");
 addword('\0','\0',"+<///>");
 addword('\0','\0',"+<//>");
 addword('\0','\0',"+</>");
 addword('\0','\0',"+<\">");
*/
	if (linkSp2Mor) {
		if (morPunctuation[0] == EOS) {
			strcpy(morPunctuation, GlobalPunctuation);
			for (i=0; morPunctuation[i]; ) {
				if (morPunctuation[i] == '!' ||
					  morPunctuation[i] == '?' ||
					  morPunctuation[i] == '.') 
					strcpy(morPunctuation+i,morPunctuation+i+1);
				else
					i++;
			}
		}
		punctuation = morPunctuation;
	}
	filterMorscop(spTier, linkSp2Mor);
	filterwords("*", spTier, excludeSpMordef);
	filterwords("%", mor_tier, excludeMordef);
/*
 addword('\0','\0',"+0");
 addword('\0','\0',"+&*");
 addword('\0','\0',"+-*");
 addword('\0','\0',"+#*");
 addword('\0','\0',"+(*.*)");
*/
	sMark[0] = ' ';
	sMark[1] = sMarkChr;
	sMark[2] = ' ';
	sMark[3] = EOS;
	dMark[0] = ' ';
	dMark[1] = dMarkChr;
	dMark[2] = ' ';
	dMark[3] = EOS;
	missMatch = FALSE;
	i = 0;
	j = 0;
	isCMFound = FALSE;
	new_mor_tier[0] = EOS;
	if (linkSp2Mor)
		spTierDone = FALSE;
	else
		spTierDone = TRUE;
	tempTier[0] = EOS;
	do {
		while ((j=getword("%", mor_tier, MorWord, NULL, j))) {
			if (!uS.mStrnicmp(MorWord,"tag|",4) && MorWord[4]== (char)0xE2 && MorWord[5]== (char)0x80 && MorWord[6]== (char)0x9E) {
			} else
				break;
		}
		if (j != 0) {
			if (linkSp2Mor)
				strcat(new_mor_tier, dMark);
			else
				strcat(new_mor_tier, " ");
			strcat(new_mor_tier, MorWord);
			if (!strcmp(MorWord, "cm|cm"))
				isCMFound = TRUE;
			if ((MorWord[0] != '+' || isRightMorUttDelim(MorWord, linkSp2Mor)) && MorWord[0] != '-' && !spTierDone) {
				while ((i=getword("*\001", spTier, spWord, NULL, i))) {
					if (uS.isRightChar(spWord, 0, '(', &dFnt, MBF) && uS.isPause(spWord, 0, NULL,  &t)) {
					} else if (!strcmp(spWord, ",") && uS.mStrnicmp(MorWord, "cm|", 3)) {
					} else if (isRightMorUttDelim(spWord, linkSp2Mor)) {
						break;
					} else if (spWord[0] != '\001' && spWord[0] != '\002') {
						if ((res=uS.HandleCAChars(spWord, NULL)) != 0) {
							if (spWord[res] != EOS)
								break;
						} else
							break;
					}
				}
				if (i == 0) {
					spTierDone = TRUE;
					missMatch = TRUE;
				} else {
					if (linkSp2Mor) {
						ChangeBackSpace(spWord);
						strcat(new_mor_tier, sMark);
						strcat(new_mor_tier, spWord);
					}
					t = i;
					while ((t=getword("*\001", spTier, spWord, NULL, t))) {
						ChangeBackSpace(spWord);
						if (spWord[0] == '\001') {
							if (R8)
								strcat(new_mor_tier, spWord);
						} else if (spWord[0] == '\002') {
							if (R8)
								strcat(new_mor_tier, spWord);
						} else if (spWord[0] != '[') {
							if ((res=uS.HandleCAChars(spWord, NULL)) != 0) {
								if (spWord[res] != EOS)
									break;
							} else
								break;
						} else if (spWord[0] == '[' && (spWord[1] == '+' || spWord[1] == '-')) {
							break;
						} else if (linkSp2Mor) {
							if (!ml_IsClauseMarker(spWord)) {
								strcat(new_mor_tier, " ");
								strcat(new_mor_tier, spWord);
							}
						}
					}
					if (ml_isclause()) {
						t = i;
						while ((t=getword("*\001", spTier, spWord, NULL, t))) {
							ChangeBackSpace(spWord);
							if (uS.isRightChar(spWord, 0, '(', &dFnt, MBF) && uS.isPause(spWord, 0, NULL,  &t)) {
							} else if (!strcmp(spWord, ",") && uS.mStrnicmp(MorWord, "cm|", 3)) {
							} else if (isRightMorUttDelim(spWord, linkSp2Mor)) {
								break;
							} else if (spWord[0] != '\001' && spWord[0] != '\002') {
								if ((res=uS.HandleCAChars(spWord, NULL)) != 0) {
									if (spWord[res] != EOS)
										break;
								} else
									break;
							} else if (linkSp2Mor) {
								if (ml_IsClauseMarker(spWord)) {
									strcat(tempTier, spWord);
									strcat(tempTier, " ");
								}
							}
						}
					}
				}
			} else if (spTierDone)
				missMatch = TRUE;
			if (tempTier[0] != EOS) {
				uS.remblanks(tempTier);
				strcat(new_mor_tier, dMark);
				strcat(new_mor_tier, tempTier);
				strcat(new_mor_tier, sMark);
				strcat(new_mor_tier, tempTier);
				tempTier[0] = EOS;
			}
		}
	} while (j != 0) ;
	if (tempTier[0] != EOS) {
		uS.remblanks(tempTier);
		strcat(new_mor_tier, dMark);
		strcat(new_mor_tier, tempTier);
		strcat(new_mor_tier, sMark);
		strcat(new_mor_tier, tempTier);
		tempTier[0] = EOS;
	}
	if (!missMatch && i != 0) {
		while ((i=getword("*", spTier, spWord, NULL, i))) {
			if (uS.isRightChar(spWord, 0, '(', &dFnt, MBF) && uS.isPause(spWord, 0, NULL,  &t)) {
			} else if (!strcmp(spWord, ",") && uS.mStrnicmp(MorWord, "cm|", 3)) {
			} else if (isRightMorUttDelim(spWord, linkSp2Mor)) {
				break;
			} else if (spWord[0] != '\001' && spWord[0] != '\002') {
				if ((res=uS.HandleCAChars(spWord, NULL)) != 0) {
					if (spWord[res] != EOS)
						break;
				} else
					break;
			}
		}
		if (i != 0)
			missMatch = TRUE;
	}
	if (missMatch && linkSp2Mor) {
/* 2012-11-1 2012-10-16 comma to %mor
		if (isCMFound)
			fprintf(stderr, "\nWARNING: found \"cm|cm\" on %%mor tier.\nPlease run \"mor\", \"post\" command on data file \"%s\"\n\n", oldfname);
*/
		mor_link.error_found = TRUE;
		strcpy(mor_link.fname, oldfname);
		mor_link.lineno = lineno;
		strcpy(new_mor_tier, mor_tier);
	}
	if (new_mor_tier[0] == ' ')
		strcpy(new_mor_tier, new_mor_tier+1);
	punctuation = GlobalPunctuation;
}

char isEqual(const char *st, const char *pat) {
	if (pat == NULL)
		return(FALSE);
	if (!uS.mStricmp(st, pat))
		return(TRUE);
	else
		return(FALSE);
}

char isnEqual(const char *st, const char *pat, int len) {
	if (pat == NULL)
		return(FALSE);
	if (!uS.mStrnicmp(st, pat, len))
		return(TRUE);
	else
		return(FALSE);
}

char isSuffix(const char *st, MORFEATS *feat) {
	if (feat->suffix0 != NULL) {
		if (!uS.mStricmp(st, feat->suffix0))
			return(TRUE);
	}
	if (feat->suffix1 != NULL) {
		if (!uS.mStricmp(st, feat->suffix1))
			return(TRUE);
	}
	if (feat->suffix2 != NULL) {
		if (!uS.mStricmp(st, feat->suffix2))
			return(TRUE);
	}
	return(FALSE);
}

char isFusion(const char *st, MORFEATS *feat) {
	if (feat->fusion0 != NULL) {
		if (!uS.mStricmp(st, feat->fusion0))
			return(TRUE);
	}
	if (feat->fusion1 != NULL) {
		if (!uS.mStricmp(st, feat->fusion1))
			return(TRUE);
	}
	if (feat->fusion2 != NULL) {
		if (!uS.mStricmp(st, feat->fusion2))
			return(TRUE);
	}
	return(FALSE);
}

char isAllv(MORFEATS *feat) {
	if (isEqual("v", feat->pos) || 
		  (isEqual("part",feat->pos) && (isSuffix("PASTP",feat) || isSuffix("PRESP",feat) || isFusion("PASTP",feat) || isFusion("PRESP",feat))))
		return(TRUE);
	return(FALSE);
}

/*	END %mor tier elements parsing END */
/**************************************************************/

/**************************************************************/
/*	BEGIN *@s: elements parsing BEGIN */
#define isLangPart(n)  ((n) != (unCH)':' && (n) != (unCH)'&' && (n) != (unCH)'+' && (n) != (unCH)'$' && (n) != (unCH)EOS)
static void freeLangParts(LANGPARTSLIST *p) {
	LANGPARTSLIST *t;

	while (p != NULL) {
		t = p;
		p = p->nextPart;
		free(t->pat);
		free(t);
	}
}

static LANGWDLST *freeLangWords(LANGWDLST *p) {
	LANGWDLST *t;

	while (p != NULL) {
		t = p;
		p = p->nextMatch;
		freeLangParts(t->rootPart);
		free(t);
	}
	return(p);
}

static void LangSearchSytaxUsage(char *f) {
	if (f != NULL) {
		printf("\n%c%c@s Followed by language \"@s:\" search pattern\n", f[0], f[1]);
		puts("Example of language code format: word@s:ita+eng$n also word@s:eng&spa, word@s:spa");
	}
	puts("    r word, can also be ';' instead of 'r'");
	puts("    & stem language marker");
	puts("    + suffix language marker");
	puts("    $ part-of-speech marker");
	puts("    o all other elements not specified by user");
	if (f != NULL) {
		puts("  followed by - or + and/or the following");
		puts("    * \t\t find any match");
		puts("    % \t\t erase any match");
		puts("    string \t find \"string\"");
		puts("\nFor example:");
		puts("    +s\"@s&-spa\"");
		puts("  find all words with only Spanish stems");
		puts("    +s\"@s&ita,$n\"");
		puts("  find all words with only Italian stems and part of speech tag \"n\"");
		puts("    +s\"@sr-*,&-eng,o-%\"");
		puts("  find all words with only English stems and erase all other markers");
		puts("    +s\"@s&ita,+eng\"");
		puts("  find all words with only Italian stems and English suffix");
		puts("    +s\"@s&eng\" +s\"@s&spa\"");
		puts("  find all words with only English stems or only Spanish stem");
		puts("    +s\"@s&+eng\"");
		puts("  find all words with English stem even if stem can be of other language too");
		puts("    +s\"@s&+eng,&+spa\"");
		puts("  find all words with either English or Spanish stems or both");
		puts("    +s\"@s&eng,&spa\"");
		puts("  find only words that have both English and Spanish stem");
	}
	cutt_exit(0);
}

char isLanguageSearchListGiven(void) {
	if (langptr == NULL)
		return(FALSE);
	else
		return(TRUE);
}

static LANGPARTSLIST *makeLangPartList(char *wd, char *orgWd) {
	char *comma;
	size_t len;
	LANGPARTSLIST *tItem;

	if (*wd != 'r' && *wd != ';' && *wd != '&' && *wd != ':' && *wd != '+' && *wd != '$' && *wd != 'o') {
		fprintf(stderr, "\n  Illegal symbol \"%c\" used in option \"%s\".\n  Please use one the following symbols:\n", *wd, orgWd);
		langptr = freeLangWords(langptr);
		LangSearchSytaxUsage(NULL);
	}
	if ((tItem=NEW(LANGPARTSLIST)) == NULL) {
		return(NULL);
	} else {
		comma = strchr(wd, ',');
		if (comma != NULL)
			*comma = EOS;
		tItem->partType = *wd++;
		if (tItem->partType == ';')
			tItem->partType = 'r';
		if (*wd == '-' || *wd == '+')
			tItem->flag = *wd++;
		else
			tItem->flag = '-';
		len = strlen(wd);
		if (len == 0) {
			fprintf(stderr, "\n  Please specify '*' or '%%' or string after symbol '%c' in option \"%s\".\n", tItem->partType, orgWd);
			langptr = freeLangWords(langptr);
			cutt_exit(0);
		}
		tItem->pat = (char *)malloc(len+1);
		if (tItem->pat == NULL) {
			if (comma != NULL)
				*comma = ',';
			return(NULL);
		}
		strcpy(tItem->pat, wd);
		tItem->nextPart = NULL;
		if (comma != NULL)
			tItem->nextPart = makeLangPartList(comma+1, orgWd);
		if (comma != NULL)
			*comma = ',';
	}
	return(tItem);
}

static char makeLangWordList(char *wd, char ch) {
	char st[BUFSIZ], colaps[] = "o-%";
	LANGWDLST *tItem, *p;
	LANGPARTSLIST *l, *m;

	if ((tItem=NEW(LANGWDLST)) == NULL) {
		return(FALSE);
	}
	wd++;
	if (*wd == ':' && (*(wd+1) == '&' || *(wd+1) == '+' || *(wd+1) == '$'))
		wd++;
	strncpy(st, wd, BUFSIZ-2);
	st[BUFSIZ-1] = EOS;
	tItem->type = ch;
	tItem->rootPart = makeLangPartList(wd, st);
	tItem->nextMatch = NULL;

	for (l=tItem->rootPart; l != NULL; l=l->nextPart) {
		for (m=tItem->rootPart; m != NULL; m=m->nextPart) {
			if (l != m && l->partType == m->partType && m->partType != 'r' && m->partType != '&') {
				fprintf(stderr, "\nPlease do not use the same element twice \"%c-%s\" and \"%c-%s\" in option \"%s\".\n", l->partType, l->pat, m->partType, m->pat, st);
				langptr = freeLangWords(langptr);
				cutt_exit(0);
			}
		}
	}
	if (tItem->type != 'i' && tItem->rootPart != NULL) {
		for (l=tItem->rootPart; l->nextPart != NULL; l=l->nextPart) {
			if (l->partType == 'o')
				break;
		}
		if (l->partType != 'o') {
			l->nextPart = makeLangPartList(colaps, st);
		}
	}
	if (langptr == NULL)
		langptr = tItem;
	else {
		for (p=langptr; p->nextMatch != NULL; p=p->nextMatch) ;
		p->nextMatch = tItem;
	}
	return(TRUE);
}

static char isLangSearchOption(char *f, char ch) {
	if (ch != '-' && ch != '+')
		return(FALSE);
	if (*f != '@')
		return(FALSE);
	f++;
	if (*f == '+' || *f == '~')
		f++;
	if (*f != 's')
		return(FALSE);
	f++;
	if (*f == 'r') {// word
		f++;
		if (*f == '-' || *f == '+' || *f == '*' || *f == '%')
			return(TRUE);
	} else if (*f == ';') // stem
		return(TRUE);
	else if (*f == '&') // stem
		return(TRUE);
	else if (*f == ':') // stem
		return(TRUE);
	else if (*f == '+') // suffix
		return(TRUE);
	else if (*f == '$') // suffix
		return(TRUE);
	else if (*f == 'o') {// other (default *)
		f++;
		if (*f == '-' || *f == '+' || *f == '*' || *f == '%')
			return(TRUE);
	}
	return(FALSE);
}

static void ParseWordIntoParts(char *item, LANGPARTS *word_parts) {
	char *c, type, t;

	word_parts->wordpart = NULL;
	word_parts->pos = NULL;
	word_parts->lang0 = NULL;
	word_parts->lang1 = NULL;
	word_parts->lang2 = NULL;
	word_parts->lang3 = NULL;
	word_parts->suffix0 = NULL;
	word_parts->suffix1 = NULL;
	word_parts->suffix2 = NULL;
	for (; *item == '\n' || isSpace(*item); item++) ;
	for (c=item; *c != EOS && *c != '@'; c++) ;
	type = *c;
	*c = EOS;
	word_parts->wordpart = item;
	item = c + 1;
	while (type == '@' || type == 's' || type == '\n' || isSpace(type))
		type = *item++;
	if (type == ':' && (*item == '&' || *item == '+' || *item == '$'))
		type = *item++;
	while (type != EOS) {
		while (type == '\n' || isSpace(type))
			type = *item++;
		for (c=item; isLangPart(*c); c++) ;
		t = *c;
		*c = EOS;
		if (type == '&' || type == ':') {
			if (word_parts->lang0 == NULL)
				word_parts->lang0 = item;
			else if (word_parts->lang1 == NULL)
				word_parts->lang1 = item;
			else if (word_parts->lang2 == NULL)
				word_parts->lang2 = item;
			else
				word_parts->lang3 = item;
		} else if (type == '+') {
			if (word_parts->suffix0 == NULL)
				word_parts->suffix0 = item;
			else if (word_parts->suffix1 == NULL)
				word_parts->suffix1 = item;
			else
				word_parts->suffix2 = item;
		} else if (type == '$') {
			word_parts->pos = item;
		} else {
			for (; *item != EOS; item++)
				*item = ' ';
		}
		type = t;
		item = c + 1;
	}
}

static int matchToLangParts(LANGPARTSLIST *pat_parts, LANGPARTS *word_parts) {
	char *c;
	char mWordpart,
		mPos,
		mLang0,
		mLang1,
		mLang2,
		mLang3,
		mSuffix0,
		mSuffix1,
		mSuffix2;
	LANGPARTSLIST *pat_part;

	mWordpart = 0;
	mPos = 0;
	mLang0 = 0;
	mLang1 = 0;
	mLang2 = 0;
	mLang3 = 0;
	mSuffix0 = 0;
	mSuffix1 = 0;
	mSuffix2 = 0;
	for (pat_part=pat_parts; pat_part != NULL; pat_part=pat_part->nextPart) {
		pat_part->isLangMatch = FALSE;
		if (pat_part->partType == 'o') {
		} else if (pat_part->partType == 'r') {
			if (word_parts->wordpart != NULL && uS.patmat(word_parts->wordpart, pat_part->pat)) {
				mWordpart = 1;
			} else if (mWordpart != 1 && pat_part->flag != '+') {
				mWordpart = -1;
			}
		} else if (pat_part->partType == '$') {
			if (word_parts->pos != NULL && uS.patmat(word_parts->pos, pat_part->pat)) {
				mPos = 1;
			} else if (word_parts->pos != NULL || (mPos != 1 && pat_part->flag != '+')) {
				mPos = -1;
			}
		} else if (pat_part->partType == '&' || pat_part->partType == ':') {
			if (word_parts->lang0 != NULL && uS.patmat(word_parts->lang0, pat_part->pat)) {
				pat_part->isLangMatch = TRUE;
				mLang0 = 1;
				if (pat_part->flag == '+') {
					mLang1 = 1;
					mLang2 = 1;
					mLang3 = 1;
				}
			} else if (word_parts->lang0 != NULL && mLang0 != 1) {
				mLang0 = -1;
			}
			if (word_parts->lang1 != NULL && uS.patmat(word_parts->lang1, pat_part->pat)) {
				pat_part->isLangMatch = TRUE;
				mLang1 = 1;
				if (pat_part->flag == '+') {
					mLang0 = 1;
					mLang2 = 1;
					mLang3 = 1;
				}
			} else if (word_parts->lang1 != NULL && mLang1 != 1) {
				mLang1 = -1;
			}
			if (word_parts->lang2 != NULL && uS.patmat(word_parts->lang2, pat_part->pat)) {
				pat_part->isLangMatch = TRUE;
				mLang2 = 1;
				if (pat_part->flag == '+') {
					mLang0 = 1;
					mLang1 = 1;
					mLang3 = 1;
				}
			} else if (word_parts->lang2 != NULL && mLang2 != 1) {
				mLang2 = -1;
			}
			if (word_parts->lang3 != NULL && uS.patmat(word_parts->lang3, pat_part->pat)) {
				pat_part->isLangMatch = TRUE;
				mLang3 = 1;
				if (pat_part->flag == '+') {
					mLang0 = 1;
					mLang1 = 1;
					mLang2 = 1;
				}
			} else if (word_parts->lang3 != NULL && mLang3 != 1) {
				mLang3 = -1;
			}
		} else if (pat_part->partType == '+') {
			if (word_parts->suffix0 != NULL && uS.patmat(word_parts->suffix0, pat_part->pat)) {
				mSuffix0 = 1;
			} else if (word_parts->suffix0 != NULL || (mSuffix0 != 1 && pat_part->flag != '+')) {
				mSuffix0 = -1;
			}
			if (word_parts->suffix1 != NULL && uS.patmat(word_parts->suffix1, pat_part->pat)) {
				mSuffix1 = 1;
			} else if (word_parts->suffix1 != NULL || (mSuffix1 != 1 && pat_part->flag != '+')) {
				mSuffix1 = -1;
			}
			if (word_parts->suffix2 != NULL && uS.patmat(word_parts->suffix2, pat_part->pat)) {
				mSuffix2 = 1;
			} else if (word_parts->suffix2 != NULL || (mSuffix2 != 1 && pat_part->flag != '+')) {
				mSuffix2 = -1;
			}
		}
	}
	for (pat_part=pat_parts; pat_part != NULL; pat_part=pat_part->nextPart) {
		if (pat_part->partType == 'o') {
			if (mWordpart != 1 && !isStringEmpty(word_parts->wordpart)) {
				if (uS.patmat(word_parts->wordpart, pat_part->pat)) {
					if (mWordpart == 0) mWordpart = 1;
				} else {
					if (mWordpart == 0) mWordpart = -1;
				}
			}
			if (mPos != 1 && !isStringEmpty(word_parts->pos)) {
				if (uS.patmat(word_parts->pos, pat_part->pat)) {
					if (mPos == 0) mPos = 1;
				} else {
					if (mPos == 0) mPos = -1;
				}
			}
			if (mLang0 != 1 && !isStringEmpty(word_parts->lang0)) {
				if (uS.patmat(word_parts->lang0, pat_part->pat)) {
					if (mLang0 == 0) mLang0 = 1;
				} else {
					if (mLang0 == 0) mLang0 = -1;
				}
			}
			if (mLang1 != 1 && !isStringEmpty(word_parts->lang1)) {
				if (uS.patmat(word_parts->lang1, pat_part->pat)) {
					if (mLang1 == 0) mLang1 = 1;
				} else {
					if (mLang1 == 0) mLang1 = -1;
				}
			}
			if (mLang2 != 1 && !isStringEmpty(word_parts->lang2)) {
				if (uS.patmat(word_parts->lang2, pat_part->pat)) {
					if (mLang2 == 0) mLang2 = 1;
				} else {
					if (mLang2 == 0) mLang2 = -1;
				}
			}
			if (mLang3 != 1 && !isStringEmpty(word_parts->lang3)) {
				if (uS.patmat(word_parts->lang3, pat_part->pat)) {
					if (mLang3 == 0) mLang3 = 1;
				} else {
					if (mLang3 == 0) mLang3 = -1;
				}
			}
			if (mSuffix0 != 1 && !isStringEmpty(word_parts->suffix0)) {
				if (uS.patmat(word_parts->suffix0, pat_part->pat)) {
					if (mSuffix0 == 0) mSuffix0 = 1;
				} else {
					if (mSuffix0 == 0) mSuffix0 = -1;
				}
			}
			if (mSuffix1 != 1 && !isStringEmpty(word_parts->suffix1)) {
				if (uS.patmat(word_parts->suffix1, pat_part->pat)) {
					if (mSuffix1 == 0) mSuffix1 = 1;
				} else {
					if (mSuffix1 == 0) mSuffix1 = -1;
				}
			}
			if (mSuffix2 != 1 && !isStringEmpty(word_parts->suffix2)) {
				if (uS.patmat(word_parts->suffix2, pat_part->pat)) {
					if (mSuffix2 == 0) mSuffix2 = 1;
				} else {
					if (mSuffix2 == 0) mSuffix2 = -1;
				}
			}
			break;
		}
	}
	for (pat_part=pat_parts; pat_part != NULL; pat_part=pat_part->nextPart) {
		if (pat_part->partType == '&' && pat_part->isLangMatch == FALSE && pat_part->flag != '+') {
			mLang0 = -1;
			break;
		}
	}
	if (mWordpart != -1 && mPos != -1 && mLang0 != -1 && mLang1 != -1 && mLang2 != -1 && mLang3 != -1 &&
				(mSuffix0 != -1 || mSuffix1 != -1 || mSuffix2 != -1)) {
		return(TRUE);
	}
	if (word_parts->wordpart != NULL) {
		for (c=word_parts->wordpart; *c != EOS; c++)
			*c = ' ';
	}
	if (word_parts->pos != NULL) {
		for (c=word_parts->pos; *c != EOS; c++)
			*c = ' ';
	}
	if (word_parts->lang0 != NULL) {
		for (c=word_parts->lang0; *c != EOS; c++)
			*c = ' ';
	}
	if (word_parts->lang1 != NULL) {
		for (c=word_parts->lang1; *c != EOS; c++)
			*c = ' ';
	}
	if (word_parts->lang2 != NULL) {
		for (c=word_parts->lang2; *c != EOS; c++)
			*c = ' ';
	}
	if (word_parts->lang3 != NULL) {
		for (c=word_parts->lang3; *c != EOS; c++)
			*c = ' ';
	}
	if (word_parts->suffix0 != NULL) {
		for (c=word_parts->suffix0; *c != EOS; c++)
			*c = ' ';
	}
	if (word_parts->suffix1 != NULL) {
		for (c=word_parts->suffix1; *c != EOS; c++)
			*c = ' ';
	}
	if (word_parts->suffix2 != NULL) {
		for (c=word_parts->suffix2; *c != EOS; c++)
			*c = ' ';
	}
	return(FALSE);
}

static char isLangAllBeforeSpace(char *matched, char *word, long i) {
	while (i >= 0) {
		if (!isSpace(matched[i]) && matched[i] != EOS)
			return(FALSE);
		if (i > 0 && word[i-1] == '@' && word[i] == 's')
			break;
		i--;
	}
	return(TRUE);
}

static void cleanUpLangWord(char *word, char *matched) {
	long i;

	for (i=0; word[i] != EOS; i++) {
		if (word[i] == '@' && word[i+1] == 's')
			i += 2;
		if (isSpace(matched[i]) || matched[i] == EOS) {
			if (isLangPart(word[i]))
				word[i] = ' ';
			else if ((word[i] == '&' || word[i] == '+') && isLangAllBeforeSpace(matched, word, i-1))
				word[i] = ' ';
			else if (word[i] != '&' && word[i] != ':' && (isSpace(matched[i+1]) || matched[i+1]==EOS))
				word[i] = ' ';
		}
	}
	for (i=0; word[i] != EOS; ) {
		if (isSpace(word[i]))
			strcpy(word+i, word+i+1);
		else
			i++;
	}
	while (word[0] != EOS && !isLangPart(word[0]))
		strcpy(word, word+1);
	for (i=strlen(word)-1; i >= 0 && !isLangPart(word[i]); i--)
		strcpy(word+i, word+i+1);
	for (i=0; word[i] != EOS; ) {
		if ((word[i] == '&' || word[i] == '+') && !isLangPart(word[i+1]))
			strcpy(word+i, word+i+1);
		else
			i++;
	}
}
/*	END *@s: elements parsing END */
/**************************************************************/

/**************************************************************/
/*	 keyword and punctuation files					*/
void freedefwdptr(char *st) {
	IEWORDS *t, *tt;

	t = defwdptr;
	tt = t;
	do {
		if (uS.patmat(st,tt->word)) {
			if (tt == defwdptr) {
				defwdptr = tt->nextword;
				t = defwdptr;
			} else
				t->nextword = tt->nextword;
			free(tt->word);
			free(tt);
			tt = t;
		} else if (strchr(st,(int)'%') == NULL) {
			if (uS.patmat(tt->word,st)) {
				if (tt == defwdptr) {
					defwdptr = tt->nextword;
					t = defwdptr;
				} else
					t->nextword = tt->nextword;
				free(tt->word);
				free(tt);
				tt = t;
			}
		}
		t = tt;
		tt = tt->nextword;
	} while (tt != NULL) ;
}

/* addword(opt,ch,wd) is used to specify the words and scop items to be excluded
   or include. If the first character of string "wd" is "+" then the string
   will be added to the existing list of words or scop tokens. "ch"
   character spesifies whether the string is to be included "i" or excluded
   "e". If the ScopMode or WordMode are equal to 0 then the items will be
   excluded. If the first or the first after "+" character is either "<" or
   "[" then the "wd" is a scop token, otherwise it is a word.
*/
IEWORDS *InsertWord(IEWORDS *tw, IEWORDS *wptr) {
	IEWORDS *t, *tt;

	if (wptr == NULL) {
		wptr = tw;
		wptr->nextword = NULL;
	} else if (strcmp(tw->word,wptr->word) > 0) {
		tw->nextword = wptr;
		wptr = tw;
	} else if (strcmp(tw->word,wptr->word) == 0) {
		free(tw->word);
		free(tw);
	} else {
		t = wptr;
		tt = wptr->nextword;
		while (tt != NULL) {
			if (strcmp(tw->word,tt->word) > 0) break;
			t = tt;
			tt = tt->nextword;
		}
		if (tt == NULL) {
			t->nextword = tw;
			tw->nextword = NULL;
		} else {
			tw->nextword = tt;
			t->nextword = tw;
		}
	}
	return(wptr);
}

void cleanupMultiWord(char *st) {
	char *s;

	s = st + strlen(st) + 1;
	*s = EOS;
	s = st;
	while ((s=strchr(s, ' ')) != NULL) {
		*s = EOS;
		s++;
	}
}

IEMWORDS *InsertMulti(IEMWORDS *mroot, char *st) {
	char *s;
	IEMWORDS *p;

	if (st[0] == EOS)
		return(mroot);
	if (mroot == NULL) {
		mroot = NEW(IEMWORDS);
		p = mroot;
	} else {
		for (p=mroot; p->nextword != NULL; p=p->nextword) ;
		p->nextword = NEW(IEMWORDS);
		p = p->nextword;
	}
	p->nextword = NULL;
	if (p == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(1);
	}
	s = st;
	p->total = 0;
	while ((s=strchr(st, ' ')) != NULL) {
		*s = EOS;
		p->word_arr[p->total] = (char *)malloc(strlen(st)+1);
		if(p->word_arr[p->total] == NULL) {
			fprintf(stderr,"No more space left in core.\n");
			cutt_exit(1);
		}
		strcpy(p->word_arr[p->total], st);
		p->context_arr[p->total][0] = EOS;
		p->isMatch[p->total] = FALSE;
		*s = ' ';
		s++;
		st = s;
		p->total++;
		if (p->total >= MULTIWORDMAX)
			break;
	}
	if (p->total < MULTIWORDMAX) {
		p->word_arr[p->total] = (char *)malloc(strlen(st)+1);
		if(p->word_arr[p->total] == NULL) {
			fprintf(stderr,"No more space left in core.\n");
			cutt_exit(1);
		}
		strcpy(p->word_arr[p->total], st);
		p->context_arr[p->total][0] = EOS;
		p->isMatch[p->total] = FALSE;
		p->total++;
	}
	p->cnt = 0;
	return(mroot);
}


static char isCodeSpecified(int num) {
	int  res;
	char code[SPEAKERLEN];
	IEWORDS *twd;

	for (twd=wdptr; twd != NULL; twd = twd->nextword) {
		if (uS.isSqCodes(twd->word, code, &dFnt, FALSE)) {
			if (code[0] == '[') {
				res = strlen(code) - 1;
				code[res] = EOS;
				if (!uS.mStricmp(ScopWdPtr[num]+3, code+1)) {
					ScopWdPtr[num][0] = 5;
					ScopWdPtr[num][1] = 2;
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}

void IsSearchR7(char *w) {
	for (; *w; w++) {
		if (*w == '@')
			break;
		if (*w == '/') {
			R7Slash = FALSE;
		} else if (*w == '~') {
			R7Tilda = FALSE;
		} else if (*w == '^') {
			R7Caret = FALSE;
		} else if (*w == ':') {
			R7Colon = FALSE;
		}
	}
}

static char SpecialPlus(char *wd, char opt, char ch) {
	if (wd[0] == '+') {
		if (opt == '\0' && ch == '\0')
			return(TRUE);
		else if (!strcmp(wd+1, "...") || !strcmp(wd+1, "..?") ||  !strcmp(wd+1, "!?")  || !strcmp(wd+1, "/.")  ||
			!strcmp(wd+1, "/?")  || !strcmp(wd+1, "//.") || !strcmp(wd+1, "//?")  || !strcmp(wd+1, ".")   ||
			!strcmp(wd+1, "+.")  || !strcmp(wd+1, "=.")  || !strcmp(wd+1, "\"/.") || !strcmp(wd+1, "\".") ||
			!strcmp(wd+1, "\"")  || !strcmp(wd+1, "^")   || !strcmp(wd+1, "<")    || !strcmp(wd+1, ",")   ||
			!strcmp(wd+1, "+"))
			return(FALSE);
		else
			return(TRUE);
	} else
		return(FALSE);
}

static int cutt_isPause(char *st, int posO) {
	int pos;

	for (pos=posO+1; isdigit(st[pos]) || st[pos]== '.' || st[pos]== ':' || st[pos]== '*' || st[pos]== '%' || st[pos]== '_'; pos++) ;
	if (!uS.isRightChar(st,pos,')',&dFnt,MBF))
		return(FALSE);
	return(TRUE);
}

static void cleanCode(char *code) {
	char *e;

	while (*code != EOS) {
		if ((*code == '<' || *code == '[') && (*(code+1) == '-' || *(code+1) == '+')) {
			code += 2;
			if (!isSpace(*code)) {
				if (*code != ']' && *code != '>') {
					uS.shiftright(code, 1);
					*code = ' ';
					code++;
				}
			} else {
				*code = ' ';
				code++;
				for (e=code; isSpace(*e); e++) ;
				if (code != e)
					strcpy(code, e);
			}
		} else if ((*code == '<' || *code == '[') && *(code+1) == '\\' && *(code+2) == '*') {
			code += 3;
			if (!isSpace(*code)) {
				if (*code != ']' && *code != '>') {
					uS.shiftright(code, 1);
					*code = ' ';
					code++;
				}
			} else {
				*code = ' ';
				code++;
				for (e=code; isSpace(*e); e++) ;
				if (code != e)
					strcpy(code, e);
			}
		} else
			code++;
	}
}

void addword(char opt, char ch, const char *wdO) {
	int i, j, offset, JustScop = 0;
	char wd[3200+2];
	IEWORDS *tempwd;

	if (strlen(wdO) > 3200)
		return;
	if (!strcmp(wdO, "<*>"))
		strcpy(wd, "<\\*>");
	else if (!strcmp(wdO, "[*]"))
		strcpy(wd, "[\\*]");
	else
		strcpy(wd, wdO);
	if (wd[0] == '[' || wd[0] == '<' || wd[1] == '[' || wd[1] == '<')
		cleanCode(wd);
	for (i=strlen(wd)-1; (wd[i]== ' ' || wd[i]== '\t' || wd[i]== '\n') && i >= 0; i--) ;
	if (i == -1)
		return;

	wd[++i] = EOS;
	if (SpecialPlus(wd, opt, ch)) {
		JustScop = -1;
		i = 1;
	} else if (wd[0] == '~') {
		JustScop = 1;
		i = 1;
	} else if (wd[0] == '\001') {
		i = 1;
	} else {
 		if (*wd == '\\' && *(wd+1) == '+')
 			strcpy(wd, wd+1);
 		i = 0;
	}
	for (; wd[i] == ' ' || wd[i] == '\t'; i++) ;
	if (wd[i] == EOS)
		return;

	if (uS.isRightChar(wd, i, '(', &dFnt, MBF) && cutt_isPause(wd, i)) {
		if (opt == 's' && ch == 'i')
			pauseFound = FALSE;
		else {
			pauseFound = TRUE;
			return;
		}
	}
	if (uS.isRightChar(wd, i, ',', &dFnt, C_MBF)) {
		for (j=0; GlobalPunctuation[j]; ) {
			if (GlobalPunctuation[j] == ',')
				strcpy(GlobalPunctuation+j,GlobalPunctuation+j+1);
			else
				j++;
		}
	}
	if (!nomap)
		uS.lowercasestr(wd+i, &dFnt, C_MBF);

	for (offset=i; wd[offset] == '\\'; offset++) ;
	if (uS.isRightChar(wd, offset, '<', &dFnt, C_MBF) || uS.isRightChar(wd, offset, '[', &dFnt, C_MBF)) {
		if (isPostCodeMark(wd[i+1], wd[i+2])) {
			if (!PostCodeMode) {
				PostCodeMode = ch;
				if (JustScop == -1 && uS.isRightChar(wd, offset, '[', &dFnt, C_MBF)) {
					if (ch == 'i')
						PostCodeMode = 'I';
				}
			} else if ((char)toupper((unsigned char)PostCodeMode) != (char)toupper((unsigned char)ch)) {
				if (opt == '\0')
					opt = ' ';
				fprintf(stderr,"-%c@ or -%c option can not be used together with either option +%c@ or +%c.\n",opt,opt,opt,opt);
				fprintf(stderr,"Offending word is: %s\n", wd+i);
				cutt_exit(0);
			}
			if (JustScop != 1 && uS.isRightChar(wd, offset, '[', &dFnt, C_MBF)) {
				wd[offset] = '<';
			} else if (uS.isRightChar(wd, offset, '<', &dFnt, C_MBF)) {
				wd[offset] = '[';
			}
			if (opt != 'P')
				JustScop = 0;
		} else {
			if (!ScopMode || (opt == '\0' && ch == '\0')) {
				if (wd[offset] == '<') {
					if (JustScop == 0 && ch == 'i')
						ScopMode = 'I';
				} else if ((opt != '\0' || ch != '\0') && wd[0] != '\001')
					ScopMode = ch;
			} else if (ScopMode != ch) {
				if (ScopMode != 'I' || ch != 'i') {
					if (opt == '\0')
						opt = ' ';
					fprintf(stderr,"-%c@ or -%c option can not be used together with either option +%c@ or +%c.\n",opt,opt,opt,opt);
					fprintf(stderr,"Offending word is: %s\n", wd+i);
					cutt_exit(0);
				}
			}
		}

		if (ScopNWds >= MXWDS) {
			fprintf(stderr,"Maximum of %d words allowed.\n",MXWDS);
			cutt_exit(0);
		} else {
			int  num, k;
			char *t, isCodeMatch;

			for (; wd[i] == '\\'; i++) ;
			i++;
			j = strlen(wd+i)-1+i;
			if ((wd[j] == ']' || wd[j] == '>') && j >= i) j--;
			for (; (wd[j] == ' ' || wd[j] == '\t') && j >= i; j--) ;
			wd[++j] = EOS;
			uS.isSqCodes(wd+i, templineC3, &dFnt, TRUE);
			uS.lowercasestr(templineC3, &dFnt, MBF);
			isCodeMatch = FALSE;
			for (num=1; num < ScopNWds; num++) {
				if (opt == '\0' && ch == '\0')
					k = (strcmp(ScopWdPtr[num]+3, templineC3) == 0);
				else
					k = uS.patmat(ScopWdPtr[num]+3, templineC3);
				if (k) {
					if (wd[i-1] == '<') {
						if (ch == 'i' && JustScop != 0) {
							if (!isCodeSpecified(num)) {
								t = ScopWdPtr[num];
								for (k=num,ScopNWds--; k < ScopNWds; k++)
									ScopWdPtr[k] = ScopWdPtr[k+1];
								free(t);
							}
						} else {
							if (ScopWdPtr[num][0] != 98 && ScopWdPtr[num][2] != 'i' && ScopWdPtr[num][2] != 'I') {
								ScopWdPtr[num][0] = 1;
								ScopWdPtr[num][2] = ch;
							}
						}
					} else {
						if (JustScop == 1 || wd[0] == '\001')
							ScopWdPtr[num][0] = 1;
						else
							ScopWdPtr[num][0] = 98;
						ScopWdPtr[num][1] = 2;
						ScopWdPtr[num][2] = ch;
					}
					isCodeMatch = TRUE;
				}
			}
			if (isCodeMatch)
				goto cont;
			if (isPostCodeMark(wd[i], wd[i+1])) ;
			else if (wd[i-1] == '<' && ch == 'i' && JustScop != 0)
				return;
			if (ScopWdPtr[0] != NULL) {
				if (!strcmp(templineC3,ScopWdPtr[0]+3)) {
					if (wd[i-1] == '<')
						ScopWdPtr[0][0] = 1;
					else if (ch == 'e' && ScopWdPtr[0][1] == 2) {
						if (ScopWdPtr[0][0] == 0) {
							free(ScopWdPtr[0]);
							ScopWdPtr[0] = NULL;
							goto cont;
						} else
							ScopWdPtr[0][1] = 0;
					} else
						ScopWdPtr[0][1] = 2;
					ScopWdPtr[0][2] = ch;
					goto cont;
				}
			}
			if (JustScop > 0) {
				JustScop = ScopNWds;
				ScopNWds = 0;
				if (ScopWdPtr[ScopNWds] != NULL) {
					free(ScopWdPtr[ScopNWds]);
				}
			}
			ScopWdPtr[ScopNWds] = (char *)malloc(strlen(templineC3)+4);
			if (ScopWdPtr[ScopNWds] == NULL)
				out_of_mem();
			if (wd[i-1] == '<') {
				ScopWdPtr[ScopNWds][0] = 1;
				ScopWdPtr[ScopNWds][1] = 0;
			} else {
				if ((!strcmp(templineC3, "/") || !strcmp(templineC3, "//") || !strcmp(templineC3, "///") ||
					 !strcmp(templineC3, "/-") || !strcmp(templineC3, "/?")) && R6) {
					ScopWdPtr[num][0] = 5;
					ScopWdPtr[num][1] = 2;
				} else {
					if (JustScop == -1)
						ScopWdPtr[ScopNWds][0] = 1;
					else
						ScopWdPtr[ScopNWds][0] = 0;
					ScopWdPtr[ScopNWds][1] = 2;
				}
			}
			ScopWdPtr[ScopNWds][2] = ch;
			strcpy(ScopWdPtr[ScopNWds]+3,templineC3);
			if (JustScop > 0)
				ScopNWds = JustScop;
			else
				ScopNWds++;
cont:
			i--;
			if (uS.isRightChar(wd, offset, '[', &dFnt, C_MBF))
				wd[j++] = ']';
			else
				wd[j++] = '>';
			wd[j] = EOS;
		}
	}
	if (wd[offset] != '<' && JustScop < 1 && wd[0] != '\001') {
		if (wd[i] != '\002' && wd[i] != '\003') {
			if (WordMode && WordMode != ch && JustScop != -1) {
				if (opt == 's' && ch =='e') {
					opt = '\0';
					ch  = '\0';
					JustScop = -1;
				} else {
					if (opt == '\0')
						opt = ' ';
					fprintf(stderr,"-%c@ or -%c option can not be used together with either option +%c@ or +%c.\n",opt,opt,opt,opt);
					fprintf(stderr,"Offending word is: %s\n", wd+i);
					cutt_exit(0);
				}
			}
			if (ch && defwdptr != NULL)
				freedefwdptr(wd+i);
			if (JustScop != -1)
				WordMode = ch;
			else if (ch == 'i')
				WordMode = 'I';
		} else {
			if (WordMode && WordMode != ch && ch == 'i') {
				fprintf(stderr," Please place all -%c@ or -%c options after any +%c@ or +%c option.\n",opt,opt,opt,opt);
				cutt_exit(0);
			} else if (ch == 'i') {
				if (JustScop == -1)
					WordMode = 'I';
				else
					WordMode = ch;
			}
		}
		if (wd[i] == '\002') {
			morfptr = makeMorWordList(morfptr, NULL, wd+i+1, ch);
			if (morfptr == NULL)
				cutt_exit(0);
		} else if (wd[i] == '\003') {
			if (makeLangWordList(wd+i+1, ch) == FALSE)
				out_of_mem();
		} else if ((tempwd=NEW(IEWORDS)) == NULL)
			out_of_mem();
		else {
			tempwd->word = (char *)malloc(strlen(wd+i)+1);
			if(tempwd->word == NULL) {
				fprintf(stderr,"No more space left in core.\n");
				cutt_exit(1);
			}
			strcpy(tempwd->word, wd+i);
			IsSearchR7(tempwd->word);
			IsSearchCA(tempwd->word);
			if (ch != '\0' || opt != '\0') {
				if (tempwd->word[0] != '[' && tempwd->word[0] != '<') {
					if (strchr(tempwd->word, '|') == NULL)
						isMorWordSpec = 2;
					else if (isMorWordSpec == 0)
						isMorWordSpec = 1;
				}
			}
			if ((JustScop == -1 && WordMode != 'I') || !ch) {
				tempwd->nextword = defwdptr;
				defwdptr = tempwd;
			} else {
				wdptr = InsertWord(tempwd,wdptr);
			}
		}
	}
}

static void removeExtraSpace(char *st) {
	int i;

	for (i=0; st[i] != EOS; ) {
		if (st[i]==' ' || st[i]=='\t' || (st[i]=='<' && (i==0 || st[i-1]==' ' || st[i-1]=='\t'))) {
			i++;
			while (st[i] == ' ' || st[i] == '\t')
				strcpy(st+i, st+i+1);
		} else
			i++;
	}
}


/* rdexclf(opt,ch,fname) opens file fname and send its content, one line at the
   time, to the addword function. "ch" character spesifies whether the string
   is to be included "i" or excluded "e".
*/
void rdexclf(char opt, char ch, const FNType *fname) {
	FILE *efp;
	char wd[1024];
	int  len;
	FNType mFileName[FNSize];

	if (*fname == '+') {
		*wd = '+';
		fname++;
	} else if (*fname == '~') {
		*wd = '~';
		fname++;
	} else
		*wd = ' ';

	if (*fname == EOS) {
		fprintf(stderr,	"No %s file specified.\n", ((ch=='i') ? "include" : "exclude"));
		cutt_exit(0);
	}

	if ((efp=OpenGenLib(fname,"r",TRUE,TRUE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of the %s-files:\n\t\"%s\", \"%s\"\n",
				((ch=='i') ? "include" : "exclude"), fname, mFileName);
		cutt_exit(0);
	}
	fprintf(stderr, "    Using search file: %s\n", mFileName);
//	if ((!stout || !IsOutTTY()) && (!onlydata || !puredata) && !outputOnlyData) {
//		fprintf(fpout, "Using search file: %s.\n", mFileName);
//	}
	while (fgets_cr(wd+1, 1024, efp)) {
		if (uS.isUTF8(wd+1) || uS.partcmp(wd+1, FONTHEADER, FALSE, FALSE))
			continue;
		for (len=strlen(wd)-1; (wd[len]== ' ' || wd[len]== '\t' || wd[len]== '\n') && len >= 0; len--) ;
		if (len < 0)
			continue;
		wd[len+1] = EOS;
		if (wd[1] == '@' && isMorSearchOption(wd+1, '+'))
			wd[1] = '\002';
		if (wd[1] == '@' && isLangSearchOption(wd+1, ch))
			wd[1] = '\003';
		if (wd[1] == '"' && wd[len] == '"') {
			wd[len] = EOS;
			strcpy(wd+1, wd+2);
		}
		if (wd[1] == '\'' && wd[len] == '\'') {
			wd[len] = EOS;
			strcpy(wd+1, wd+2);
		}
		removeExtraSpace(wd+1);
		uS.remFrontAndBackBlanks(wd+1);
		if (ch == 'e') {
			if (strchr(wd+1, '[') == NULL && strchr(wd+1, '<') == NULL && strchr(wd+1, ' ') != NULL) {
				fprintf(stderr,"Multi-words are not allowed with option: -%c\n", opt);
				fprintf(stderr,"    Multi-words are search patterns with space character(s) in-between.\n");
				fprintf(stderr,"    If you did not mean to have space characters in-between words,\n");
				fprintf(stderr,"        then please remove space characters and try again.\n");
				cutt_exit(0);
			} else
				addword(opt,ch,wd);
		} else {
			if (strchr(wd+1, '[') == NULL && strchr(wd+1, '<') == NULL && 
				  (strchr(wd+1, ' ') != NULL || anyMultiOrder || onlySpecWsFound)) {
				mwdptr = InsertMulti(mwdptr, wd+1);
			} else
				addword(opt,ch,wd);
		}
	}
	fclose(efp);
}

/**************************************************************/
/*	 UttLen manipulation routines 				  */
static UTTLENWORDS *freeUTTLENWORDS(UTTLENWORDS *ptr) {
	UTTLENWORDS *t;

	while (ptr != NULL) {
		t = ptr;
		ptr = ptr->nextWdUttLen;
		if (t->word)
			free(t->word);
		free(t);
	}
	return(ptr);
}

static UTTLENWORDS *InsertUttLenWord(UTTLENWORDS *tw, UTTLENWORDS *wptr) {
	UTTLENWORDS *t, *tt;

	if (wptr == NULL) {
		wptr = tw;
		wptr->nextWdUttLen = NULL;
	} else if (strcmp(tw->word,wptr->word) > 0) {
		tw->nextWdUttLen = wptr;
		wptr = tw;
	} else if (strcmp(tw->word,wptr->word) == 0) {
		free(tw->word);
		free(tw);
	} else {
		t = wptr;
		tt = wptr->nextWdUttLen;
		while (tt != NULL) {
			if (strcmp(tw->word,tt->word) > 0) break;
			t = tt;
			tt = tt->nextWdUttLen;
		}
		if (tt == NULL) {
			t->nextWdUttLen = tw;
			tw->nextWdUttLen = NULL;
		} else {
			tw->nextWdUttLen = tt;
			t->nextWdUttLen = tw;
		}
	}
	return(wptr);
}

static void addwordUttLen(char ch, char *wd) {
	int i;
	UTTLENWORDS *tempwd;

	for (i=strlen(wd)-1; (wd[i]== ' ' || wd[i]== '\t' || wd[i]== '\n') && i >= 0; i--) ;
	if (i == -1)
		return;
	wd[++i] = EOS;
	for (i=0; wd[i] == ' ' || wd[i] == '\t'; i++) ;
	if (wd[i] == EOS)
		return;
	if (!nomap)
		uS.lowercasestr(wd+i, &dFnt, C_MBF);
	if ((tempwd=NEW(UTTLENWORDS)) == NULL)
		out_of_mem();
	else {
		tempwd->word = (char *)malloc(strlen(wd+i)+1);
		if(tempwd->word == NULL) {
			fprintf(stderr,"No more space left in core.\n");
			cutt_exit(1);
		}
		tempwd->inc = ch;
		strcpy(tempwd->word, wd+i);
		wdUttLen = InsertUttLenWord(tempwd,wdUttLen);
	}
}

static void rdexclfUttLen(char ch, const FNType *fname) {
	FILE *efp;
	char wd[1024];
	int  len;
	FNType mFileName[FNSize];

	if (*fname == EOS) {
		fprintf(stderr,	"No %s file specified.\n", ((ch=='i') ? "include" : "exclude"));
		cutt_exit(0);
	}
	if ((efp=OpenGenLib(fname,"r",TRUE,TRUE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of the %s-files:\n\t\"%s\", \"%s\"\n",
				((ch=='i') ? "include" : "exclude"), fname, mFileName);
		cutt_exit(0);
	}
	fprintf(stderr, "    Using \"+/-x\" file: %s\n", mFileName);
	while (fgets_cr(wd, 1024, efp)) {
		if (uS.isUTF8(wd) || uS.partcmp(wd, FONTHEADER, FALSE, FALSE))
			continue;
		for (len=strlen(wd)-1; (wd[len]== ' ' || wd[len]== '\t' || wd[len]== '\n') && len >= 0; len--) ;
		if (len < 0)
			continue;
		wd[len+1] = EOS;
		if (wd[0] == '\'' && wd[len] == '\'') {
			wd[len] = EOS;
			strcpy(wd, wd+1);
		}
		uS.remFrontAndBackBlanks(wd);
		addwordUttLen(ch,wd);
	}
	fclose(efp);
}

static int excludeUttLen(char *word) {
	UTTLENWORDS *twd;

	for (twd=wdUttLen; twd != NULL; twd = twd->nextWdUttLen) {
		if (uS.patmat(word, twd->word)) {
			if (twd->inc == 'i')
				return(TRUE);
			else
				return(FALSE);
		}
	}
	return(TRUE);
}
/**************************************************************/
/*	 strings manipulation routines                            */
/*
#ifdef _MAC_CODE
int _stricmp(const char *s1, const char *s2) {
	strcpy(templineC, s1);
	uS.uppercasestr(templineC, &dFnt, C_MBF);
	strcpy(templineC3, s2);
	uS.uppercasestr(templineC3, &dFnt, C_MBF);
	return(strcmp(templineC, templineC3));
}

int _strnicmp(const char *s1, const char *s2, int n) {
	strcpy(templineC, s1);
	uS.uppercasestr(templineC, &dFnt, C_MBF);
	strcpy(templineC3, s2);
	uS.uppercasestr(templineC3, &dFnt, C_MBF);
	return(strncmp(templineC, templineC3, n));
}
#endif // _MAC_CODE
*/
/* String coping functions
*/
static void att_copy(long pos, char *desSt, const char *srcSt, AttTYPE *desAtt, AttTYPE *srcAtt) {
	long i;

	for (i=0; srcSt[i]; i++, pos++) {
		desSt[pos] = srcSt[i];
		desAtt[pos] = srcAtt[i];
	}
	desSt[pos] = EOS;
}

static void att_copy_same(long pos, char *desSt, const char *srcSt, AttTYPE *desAtt) {
	AttTYPE att;
	long i;

	if (pos > 0L)
		att = desAtt[pos-1];
	else
		att = 0;
	for (i=0; srcSt[i]; i++, pos++) {
		desSt[pos] = srcSt[i];
		desAtt[pos] = att;
	}
	desSt[pos] = EOS;
}

void att_cp(long pos, char *desSt, const char *srcSt, AttTYPE *desAtt, AttTYPE *srcAtt) {
	if (srcAtt != NULL)
		att_copy(pos, desSt, srcSt, desAtt, srcAtt);
	else
		att_copy_same(pos, desSt, srcSt, desAtt);
}

void att_shiftright(char *srcSt, AttTYPE *srcAtt, long num) {
	long i;

	for (i=strlen(srcSt); i >= 0L; i--) {
		srcSt[i+num] = srcSt[i];
		srcAtt[i+num] = srcAtt[i];
	}
}

/**************************************************************/
/* isExcludePostcode(str) determines if the code token in "str" is to be included
 in the analyses or not by compairing the "str" to the list of words in
 "ScopWdPtr".
 */
extern int isExcludePostcode(char *str);
int isExcludePostcode(char *str) {
	int i;

	for (i=1; i < ScopNWds; i++) {
		if (uS.patmat(str,ScopWdPtr[i]+3)) {
			if (isPostCodeMark(str[0], str[1])) {
				if (PostCodeMode == 'i' || PostCodeMode == 'I') {
					if (ScopWdPtr[i][1] == 2)
						return(6);
					else
						return(4);
				} else
					return(5);
			}
			if (ScopWdPtr[i][2] == 'i' || ScopWdPtr[i][2] == 'I')
				return(ScopWdPtr[i][0] + ScopWdPtr[i][1]);
			else
				return((ScopWdPtr[i][0] == 1) ? 0 : 1);
		}
	}
	if (ScopWdPtr[0] == NULL) {
		return(1);
	} else {
		if (!uS.patmat(str,ScopWdPtr[0]+3)) {
			if (ScopWdPtr[0][2] == 'i' || ScopWdPtr[0][2] == 'I') {
				if (isPostCodeMark(str[0], str[1]))
					return(1);
				else
					return(1);
			} else
				return(1);
		} else
			return(ScopWdPtr[0][0] + ScopWdPtr[0][1]);
	}
}

/**************************************************************/
/* isExcludeScope(str) determines if the scop token in "str" is to be included
 in the analyses or not by compairing the "str" to the list of words in
 "ScopWdPtr". It returns 1 if the scop token should be included, and 0
 otherwise except for is uS.patmat true.
 */
int isExcludeScope(char *str) {
	int i;

	if (R6_override) {
		if (uS.patmat(str, "/") || uS.patmat(str, "//") || uS.patmat(str, "///") || uS.patmat(str, "/-") || uS.patmat(str, "/?"))
			goto skip_check;
	}
	for (i=1; i < ScopNWds; i++) {
		if (uS.patmat(str, ScopWdPtr[i]+3)) {
			if (isPostCodeMark(str[0],str[1]) && isPostCodeMark(ScopWdPtr[i][3],ScopWdPtr[i][4])) {
				if (PostCodeMode == 'i' || PostCodeMode == 'I') {
					if (ScopWdPtr[i][1] == 2)
						return(6);
					else
						return(4);
				} else
					return(5);
			}
			if (ScopWdPtr[i][2] == 'i' || ScopWdPtr[i][2] == 'I')
				return(ScopWdPtr[i][0] + ScopWdPtr[i][1]);
			else if (CLAN_PROG_NUM == KWAL && ScopWdPtr[i][0] != 1 && ScopWdPtr[i][2] == 'e')
				return(2013);
			else
				return((ScopWdPtr[i][0] == 1) ? 0 : 1);
		}
	}
	if (IncludeAllDefaultCodes)
		return(2009);
	if (R5 == TRUE) {
		if (uS.patmat(str,": *"))
			return(1001);
		if (uS.patmat(str,":\\* *"))
			return(1001);
		if (uS.patmat(str,":=_ *"))
			return(1001);
	}
	if (R5_1 == TRUE) {
		if (uS.patmat(str,":: *"))
			return(1001);
	}
skip_check:
	if (ScopWdPtr[0] == NULL) {
		if (ScopMode == 'i' || ScopMode == 'I') {
			if (ScopMode == 'I')
				return(0);
			else if (isPostCodeMark(str[0], str[1]))
				return(1);
			else
				return(1);
		} else
			return(1);
	} else {
		if (!uS.patmat(str,ScopWdPtr[0]+3)) {
			if (ScopWdPtr[0][2] == 'i' || ScopWdPtr[0][2] == 'I') {
				if (isPostCodeMark(str[0], str[1]))
					return(1);
				else
					return(1);
			} else
				return(1);
		} else
			return(ScopWdPtr[0][0] + ScopWdPtr[0][1]);
	}
}

char isMorPatMatchedWord(MORWDLST *pats, char *word) {
//	char wordT[BUFSIZ];
	MORFEATS word_feats;
	MORWDLST *pat_rootFeat;

	if (pats == NULL)
		return(1);
	for (pat_rootFeat=pats; pat_rootFeat != NULL; pat_rootFeat=pat_rootFeat->nextMatch) {
		if (pat_rootFeat->type == 'i')
			continue;
		strcpy(templineC1, word);
		word_feats.type = NULL;
		word_feats.typeID = 'R';
		if (ParseWordIntoFeatures(templineC1, &word_feats) == FALSE)
			return(2);
		if (matchToMorFeatures(pat_rootFeat, &word_feats, TRUE, TRUE)) {
//			strcpy(wordT, word);
//			cleanUpMorExcludeWord(wordT, templineC1);
			freeUpFeats(&word_feats);
//			if (wordT[0] == EOS)
			return(0);
		}
		freeUpFeats(&word_feats);
	}
	for (pat_rootFeat=pats; pat_rootFeat != NULL; pat_rootFeat=pat_rootFeat->nextMatch) {
		if (pat_rootFeat->type != 'i')
			continue;
		strcpy(templineC1, word);
		word_feats.type = NULL;
		word_feats.typeID = 'R';
		if (ParseWordIntoFeatures(templineC1, &word_feats) == FALSE)
			return(2);
		if (matchToMorFeatures(pat_rootFeat, &word_feats, TRUE, TRUE)) {
			freeUpFeats(&word_feats);
			return(1);
		} 
		freeUpFeats(&word_feats);
	}
	return(0);
}

/* exclude(word) determines if the "word" is to be included in the analyses
   or not by compairing the "word" to the list of words in "wdptr". It returns
   1 if the word should be included, and 0 otherwise.
*/
int excludedef(char *word) {
	IEWORDS *twd;

	for (twd=defwdptr; twd != NULL; twd = twd->nextword) {
		if (uS.patmat(word, twd->word)) {
			if (word[0] != '-')
				return(FALSE);
			else if (uS.isToneUnitMarker(word))
				return(FALSE);
		}
	}
	return(TRUE);
}

int exclude(char *word) {
	int res;
	char code[SPEAKERLEN], *at, isOnlyCodeSearched, isSpecialWordFound, wType;
	IEWORDS *twd;

	if (linkMain2Mor) {
		if (strchr(word, '|') || cutt_depWord)
			wType = '|';
		else if (langptr != NULL && (at=strrchr(word, '@')) != NULL && at[1] == 's' && (at[2] == ':' || at[2] == '$'))
			wType = '@';
		else
			wType = 'w';
	}
	if (word[0] != '-' && word[0] != '+' && morfptr != NULL && strchr(word, '|')) {
		MORFEATS word_feats;
		MORWDLST *pat_rootFeat;

		for (pat_rootFeat=morfptr; pat_rootFeat != NULL; pat_rootFeat=pat_rootFeat->nextMatch) {
			if (pat_rootFeat->type == 'i')
				continue;
			strcpy(templineC1, word);
			word_feats.type = NULL;
			word_feats.typeID = 'R';
			if (ParseWordIntoFeatures(templineC1, &word_feats) == FALSE)
				out_of_mem();
			if (matchToMorFeatures(pat_rootFeat, &word_feats, FALSE, TRUE)) {
				cleanUpMorExcludeWord(word, templineC1);
				freeUpFeats(&word_feats);
				if (word[0] == EOS)
					return(FALSE);
			}
			freeUpFeats(&word_feats);
		}
		for (pat_rootFeat=morfptr; pat_rootFeat != NULL; pat_rootFeat=pat_rootFeat->nextMatch) {
			if (pat_rootFeat->type != 'i')
				continue;
			strcpy(templineC1, word);
			word_feats.type = NULL;
			word_feats.typeID = 'R';
			if (ParseWordIntoFeatures(templineC1, &word_feats) == FALSE)
				out_of_mem();
			if (matchToMorFeatures(pat_rootFeat, &word_feats, FALSE, TRUE)) {
				cleanUpMorWord(word, templineC1);
				freeUpFeats(&word_feats);
				return(TRUE);
			}
			freeUpFeats(&word_feats);
		}
	}
	if (word[0] != '-' && word[0] != '+' && langptr != NULL && (at=strchr(word, '@')) != NULL) {
		if (at[1] == 's' && (at[2] == ':' || at[2] == '$')) {
			LANGPARTS word_parts;
			LANGWDLST *pat_part;

			for (pat_part=langptr; pat_part != NULL; pat_part=pat_part->nextMatch) {
				if (pat_part->type == 'i')
					continue;
				strcpy(templineC1, word);
				ParseWordIntoParts(templineC1, &word_parts);
				if (matchToLangParts(pat_part->rootPart, &word_parts)) {
					return(FALSE);
				}
			}
			for (pat_part=langptr; pat_part != NULL; pat_part=pat_part->nextMatch) {
				if (pat_part->type != 'i')
					continue;
				strcpy(templineC1, word);
				ParseWordIntoParts(templineC1, &word_parts);
				if (matchToLangParts(pat_part->rootPart, &word_parts)) {
					cleanUpLangWord(word, templineC1);
					return(TRUE);
				}
			}
		}
	}
	if ((WordMode == 'i' || WordMode == 'I') && wdptr != NULL) {
		isOnlyCodeSearched = TRUE;
		for (twd=wdptr; twd != NULL; twd = twd->nextword) {
			if (uS.isSqCodes(twd->word, code, &dFnt, FALSE)) {
				res = strlen(code) - 1;
				code[res] = EOS;
				if (isExcludeScope(code+1) != 3) {
					isOnlyCodeSearched = FALSE;
					break;
				}
			} else {
				isOnlyCodeSearched = FALSE;
				break;
			}
		}
		if (isOnlyCodeSearched)
			return(TRUE);
	}
	if (word[0] == '0' || word[0] == '&' || word[0] == '+' || word[0] == '-' || word[0] == '#')
		isSpecialWordFound = word[0];
	else
		isSpecialWordFound = 0;
	for (twd=wdptr; twd != NULL; twd=twd->nextword) {
		if (uS.patmat(word, twd->word)) {
			if (WordMode == 'i' || WordMode == 'I') {
				res = strlen(twd->word) - 1;
				if ((twd->word[res-2] == '-' || twd->word[res-2] == '&' || twd->word[res-2] == '~') && 
					  twd->word[res-1] == '%' && twd->word[res] == '%') {
					for (twd=twd->nextword; twd != NULL; twd=twd->nextword) {
						res = strlen(twd->word) - 1;
						if ((twd->word[res-2] == '-' || twd->word[res-2] == '&' || twd->word[res-2] == '~') && 
							  twd->word[res-1] == '%' && twd->word[res] == '%')
							uS.patmat(word, twd->word);
					}
				}
				return(TRUE);
			} else
				return(FALSE);
		}
		if (isSpecialWordFound == twd->word[0])
			isSpecialWordFound = 1;
	}
	if (WordMode == 'i' || (WordMode == 'I' && isSpecialWordFound == 1)) {
		if (linkMain2Mor) {
			if (wType == 'w' && (wdptr == NULL || isMorWordSpec == 1))
				return(TRUE);
			else if (wType == '|' && morfptr == NULL && isMorWordSpec != 1)
				return(TRUE);
			else if (wType == '@' && langptr == NULL)
				return(TRUE);
		}
		return(FALSE);
	} else
		return(TRUE);
}

int isPostCodeFound(UTTER *utt) {
	long i, b, e;

	for (b=0L; utt->line[b] != EOS; b++) {
		if (uS.isRightChar(utt->line, b, '[', &dFnt, MBF)) {
			b++;
			for (e=b; !uS.isRightChar(utt->line,e,']',&dFnt,MBF) && !uS.isRightChar(utt->line,e,'[',&dFnt,MBF) && utt->line[e] != EOS; e++) ;
			if (uS.isRightChar(utt->line,e,']',&dFnt,MBF)) {
				utt->line[e] = EOS;
				uS.isSqCodes(utt->line+b, templineC3, &dFnt, TRUE);
				utt->line[e] = ']';
				if (isPostCodeMark(templineC3[0], templineC3[1])) {
					uS.lowercasestr(templineC3, &dFnt, MBF);
					if (uS.isRightChar(utt->line,e,']',&dFnt, MBF)) {
						for (i=1L; i < ScopNWds; i++) {
							if (uS.patmat(templineC3,ScopWdPtr[i]+3)) {
								if (PostCodeMode == 'i' || PostCodeMode == 'I') {
									if (ScopWdPtr[i][1] == 2)
										return(6);
									else
										return(4);
								} else
									return(5);
							}
						}
						b = e;
					}
				}
			}
		}
	}
	if (utt->speaker[0] == '%') {
		if (postcodeRes != 0)
			return(postcodeRes);
	}
	if (PostCodeMode == 'i' || PostCodeMode == 'I')
		return(1);
	else
		return(0);
}

void getMediaName(char *line, FNType *tMediaFileName, long size) {
	char qf, *s;
	int i;

	if (line != NULL) {
		strncpy(tMediaFileName, line, size-1);
		tMediaFileName[size-1] = EOS;
	}
	for (i=0; isSpace(tMediaFileName[i]); i++) ;
	if (i > 0)
		strcpy(tMediaFileName, tMediaFileName+i);
	qf = FALSE;
	for (i=0; tMediaFileName[i] != EOS; i++) {
		if (tMediaFileName[i] == '"')
			qf = !qf;
		if (tMediaFileName[i] == ',' || (!qf && isSpace(tMediaFileName[i]))) {
			tMediaFileName[i] = EOS;
			break;
		}
	}
	uS.remFrontAndBackBlanks(tMediaFileName);
	s = strrchr(tMediaFileName, '.');
	if (s != NULL) {
		i = strlen(tMediaFileName) - 1;
		if (tMediaFileName[i] == '"')
			*s++ = '"';
		*s = EOS;
	}
}

/* getMediaTagInfo(*line, *tag, *fname, *Beg, *End) get information in the bullets
   associated with tag name "tag". If tag == NULL, then get information for both "%snd:" and "%mov:".
*/
char getMediaTagInfo(char *line, long *Beg, long *End) {
	int i;
	long beg = 0L, end = 0L;
	char s[256+2];

	if (*line == HIDEN_C)
		line++;

	while (*line && (isSpace(*line) || *line == '_'))
		line++;
	if (*line == EOS)
		return(FALSE);

	while (*line && !isdigit(*line) && *line != HIDEN_C)
		line++;
	if (!isdigit(*line))
		return(FALSE);
	for (i=0; *line && isdigit(*line) && i < 256; line++)
		s[i++] = *line;
	s[i] = EOS;
	beg = atol(s);
	if (beg == 0)
		beg = 1;
	while (*line && !isdigit(*line) && *line != HIDEN_C)
		line++;
	if (!isdigit(*line))
		return(FALSE);
	for (i=0; *line && isdigit(*line) && i < 256; line++)
		s[i++] = *line;
	s[i] = EOS;
	end = atol(s);
	*Beg = beg;
	*End = end;
	return(TRUE);
}

/* getOLDMediaTagInfo(*line, *tag, *fname, *Beg, *End) same as above for older version of bullets
*/
char getOLDMediaTagInfo(char *line, const char *tag, FNType *fname, long *Beg, long *End) {
	int i;
	long beg = 0L, end = 0L;
	char s[256+2];

	if (*line == HIDEN_C)
		line++;
	for (i=0; *line && *line != ':' && *line != HIDEN_C && i < 256; line++)
		s[i++] = *line;
	if (*line != ':')
		return(FALSE);
	s[i++] = *line;
	s[i] = EOS;
	line++;
	if (tag == NULL) {
		if (!uS.partcmp(s, SOUNDTIER, FALSE, FALSE) && !uS.partcmp(s, REMOVEMOVIETAG, FALSE, FALSE))
			return(FALSE);
	} else {
		if (!uS.partcmp(s, tag, FALSE, FALSE))
			return(FALSE);
	}

	while (*line && (isSpace(*line) || *line == '_'))
		line++;
	if (*line != '"')
		return(FALSE);

	line++;
	if (*line == EOS)
		return(FALSE);
	for (i=0; *line && *line != '"' && *line != HIDEN_C  && i < 256; line++)
		s[i++] = *line;
	s[i] = EOS;
	if (fname != NULL)
		uS.str2FNType(fname, 0L, s);
	while (*line && !isdigit(*line) && *line != HIDEN_C)
		line++;
	if (!isdigit(*line))
		return(FALSE);
	for (i=0; *line && isdigit(*line) && i < 256; line++)
		s[i++] = *line;
	s[i] = EOS;
	beg = atol(s);
	if (beg == 0)
		beg = 1;
	while (*line && !isdigit(*line) && *line != HIDEN_C)
		line++;
	if (!isdigit(*line))
		return(FALSE);
	for (i=0; *line && isdigit(*line) && i < 256; line++)
		s[i++] = *line;
	s[i] = EOS;
	end = atol(s);
	*Beg = beg;
	*End = end;
	return(TRUE);
}

/* getword(sp, cleanLine, word, *wi, i) extracts a word from the "cleanLine" starting at "i"
   position and stores it into "word". It return "i", the first position
   after the word and "wi" first pos of word, or 0 if end of string. It removes "(", ")"
   characters from the word.
*/
int getword(const char *sp, register char *cleanLine, register char *orgWord, int *wi, int i) {
	register int  temp;
	register char sq;
	register char *word;

	word = orgWord;
	if (chatmode && *sp == '%') {
		if (cleanLine[i] == EOS)
			return(0);
		if (i > 0 && !uS.isRightChar(cleanLine, i, '(', &dFnt, MBF) && !uS.isRightChar(cleanLine,i,'[',&dFnt,MBF))
			i--;
		while ((*word=cleanLine[i]) != EOS && uS.isskip(cleanLine,i,&dFnt,MBF) && !uS.isRightChar(cleanLine,i,'[',&dFnt,MBF)) {
			if (cleanLine[i] == '?' && cleanLine[i+1] == '|')
				break;
			i++;
			if (*word == '<') {
				temp = i;
				for (i++; cleanLine[i] != '>' && cleanLine[i]; i++) {
					if (isdigit(cleanLine[i])) ;
					else if (cleanLine[i]== ' ' || cleanLine[i]== '\t' || cleanLine[i]== '\n') ;
					else if ((i-1 == temp+1 || !isalpha(cleanLine[i-1])) && cleanLine[i] == '-' && !isalpha(cleanLine[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(cleanLine[i-1])) && (cleanLine[i] == 'u' || cleanLine[i] == 'U') &&
							 !isalpha(cleanLine[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(cleanLine[i-1])) && (cleanLine[i] == 'w' || cleanLine[i] == 'W') &&
							 !isalpha(cleanLine[i+1])) ;
					else if ((i-1 == temp+1 || !isalpha(cleanLine[i-1])) && (cleanLine[i] == 's' || cleanLine[i] == 'S') &&
							 !isalpha(cleanLine[i+1])) ;
					else
						break;
				}
				if (cleanLine[i] == '>')
					i++;
				else
					i = temp;
			}
		}
	} else {
		while ((*word=cleanLine[i]) != EOS && uS.isskip(cleanLine,i,&dFnt,MBF) && !uS.isRightChar(cleanLine,i,'[',&dFnt,MBF)) {
// 2012-11-1 2012-10-16 comma to %mor
			if (*sp == '*' && sp[1] == '\001' && uS.isRightChar(cleanLine,i,',',&dFnt,MBF))
				break;
//
			 i++;
		}
	}
	if (*word == EOS)
		return(0);
	if (uS.isRightChar(cleanLine, i, '[',&dFnt,MBF)) {
		if (uS.isSqBracketItem(cleanLine, i+1, &dFnt, MBF))
			sq = TRUE;
		else
			sq = FALSE;
	} else
		sq = FALSE;
	if (wi != NULL)
		*wi = i;
getword_rep:

	if ((*word == '+' || *word == '-') && !sq) {
		while ((*++word=cleanLine[++i]) != EOS && (!uS.isskip(cleanLine,i,&dFnt,MBF) ||
			uS.isRightChar(cleanLine,i,'/',&dFnt,MBF) || uS.isRightChar(cleanLine,i,'<',&dFnt,MBF) ||
			uS.isRightChar(cleanLine,i,'.',&dFnt,MBF) || uS.isRightChar(cleanLine,i,'!',&dFnt,MBF) ||
			uS.isRightChar(cleanLine,i,'?',&dFnt,MBF) || uS.isRightChar(cleanLine,i,',',&dFnt,MBF))) {
			if (uS.isRightChar(cleanLine, i, ']', &dFnt, MBF)) {
				*++word = EOS;
				return(i+1);
			}
		}
	} else if (uS.atUFound(cleanLine, i, &dFnt, MBF)) {
		while ((*++word=cleanLine[++i]) != EOS && (!uS.isskip(cleanLine,i,&dFnt,MBF) ||
			uS.isRightChar(cleanLine,i,'.',&dFnt,MBF) || uS.isRightChar(cleanLine,i,'!',&dFnt,MBF) ||
			uS.isRightChar(cleanLine,i,'?',&dFnt,MBF) || uS.isRightChar(cleanLine,i,',',&dFnt,MBF))) {
			if (uS.isRightChar(cleanLine, i, ']', &dFnt, MBF)) {
				*++word = EOS;
				return(i+1);
			}
		}
	} else {
		if (UTF8_IS_LEAD((unsigned char)cleanLine[i])) {
			if (cleanLine[i] == (char)0xE2 && cleanLine[i+1] == (char)0x80 && cleanLine[i+2] == (char)0x9C) {
				*++word = (char)0x80;
				*++word = (char)0x9C;
				*++word = EOS;
				return(i+3);
			} else if (cleanLine[i] == (char)0xE2 && cleanLine[i+1] == (char)0x80 && cleanLine[i+2] == (char)0x9D) {
				*++word = (char)0x80;
				*++word = (char)0x9D;
				*++word = EOS;
				return(i+3);
			} else if (cleanLine[i] == (char)0xE2 && cleanLine[i+1] == (char)0x80 && cleanLine[i+2] == (char)0x98) {
				*++word = (char)0x80;
				*++word = (char)0x98;
				*++word = EOS;
				return(i+3);
			} else if (cleanLine[i] == (char)0xE2 && cleanLine[i+1] == (char)0x80 && cleanLine[i+2] == (char)0x99) {
				*++word = (char)0x80;
				*++word = (char)0x99;
				*++word = EOS;
				return(i+3);
			} 
		} else if (cleanLine[i] == ',') {
			*++word = EOS;
			return(i+1);
		}
		while ((*++word=cleanLine[++i]) != EOS && (!uS.isskip(cleanLine,i,&dFnt,MBF) || sq)) {
			if (uS.isRightChar(cleanLine, i, ']', &dFnt, MBF)) {
				*++word = EOS;
				if (*orgWord == '[')
					uS.cleanUpCodes(orgWord, &dFnt, MBF);
				return(i+1);
			} else if (uS.isRightChar(cleanLine, i, '(', &dFnt, MBF) && uS.isPause(cleanLine, i, NULL,  &temp)) {
				*word = EOS;
				return(i);
			} else if (uS.isRightChar(cleanLine, i, '.', &dFnt, MBF) && uS.isPause(cleanLine, i, NULL, NULL)) {
				while ((*++word=cleanLine[++i]) != EOS && !uS.isRightChar(cleanLine, i, ')', &dFnt, MBF)) ;
				*++word = EOS;
				return(i+1);
			} else if (uS.IsUtteranceDel(cleanLine, i) == 2) {
				*word = EOS;
				return(i);
			} else if (UTF8_IS_LEAD((unsigned char)cleanLine[i])) {
				if (i > 0 && cleanLine[i-1] == '|' && *sp == '%') {
				} else if (cleanLine[i] == (char)0xE2 && cleanLine[i+1] == (char)0x80 && cleanLine[i+2] == (char)0x9C) {
					*word = EOS;
					return(i);
				} else if (cleanLine[i] == (char)0xE2 && cleanLine[i+1] == (char)0x80 && cleanLine[i+2] == (char)0x9D) {
					*word = EOS;
					return(i);
				} else if (cleanLine[i] == (char)0xE2 && cleanLine[i+1] == (char)0x80 && cleanLine[i+2] == (char)0x98) {
					*word = EOS;
					return(i);
				} else if (cleanLine[i] == (char)0xE2 && cleanLine[i+1] == (char)0x80 && cleanLine[i+2] == (char)0x99) {
					*word = EOS;
					return(i);
				} 
			} else if (cleanLine[i] == ',' && !isdigit(cleanLine[i+1])) {
				*word = EOS;
				return(i);
			}
		}
	}
	if (uS.isRightChar(cleanLine, i, '[', &dFnt, MBF)) {
		if (!uS.isSqBracketItem(cleanLine, i+1, &dFnt, MBF))
			goto getword_rep;
		else
			i--;
	}
	*word = EOS;
	if (cleanLine[i] != EOS) {
// 2012-11-1 2012-10-16 comma to %mor
		if (*sp == '*' && sp[1] == '\001' && uS.isRightChar(cleanLine,i,',',&dFnt,MBF)) {
		} else
//
			i++;
	}
	return(i);
}

/**************************************************************/
/*	 filter whole tier					  */
/* isSelectedScope(char st, int pos) determines if the scop token was selected
 by a user with +s option.
 */
static char isSelectedScope(char *wline, int pos) {
//	long i;

	return(FALSE);
/* Should a code be excluded even if user is searching for it and it is encompassed by exclude code:

 *PAR:	&uh <a hat [: cat] [* p:w-ret]> [//] a cat [/] cat up there looking up

 Following source code will still include code [* p:w-ret] in output if +s"[\* p*]" option specified even
 if it within exclude code [//].
*/
/*
	if (ScopMode != 'i' && ScopMode != 'I')
		return(FALSE);
	i = pos;
	for (pos--; !uS.isRightChar(wline, pos, '[', &dFnt, MBF) && !uS.isRightChar(wline, pos, ']', &dFnt, MBF) && pos >= 0; pos--) ;
	wline[i] = EOS;
	uS.isSqCodes(wline+pos+1, templineC3, &dFnt, TRUE);
	wline[i] = ']';
	uS.lowercasestr(templineC3, &dFnt, MBF);
	for (i=1; i < ScopNWds; i++) {
		if (uS.patmat(templineC3,ScopWdPtr[i]+3)) {
			if (isPostCodeMark(templineC3[0], templineC3[1]))
				return(FALSE);
			if (ScopWdPtr[i][2] == 'i' || ScopWdPtr[i][2] == 'I') {
				return(TRUE);
			} else
				return(FALSE);
		}
	}
	return(FALSE);
 */
}

/* ExcludeScope(wline,pos,isblankit) replaces all the character of the scoped
 data with the space character. All the scoped data within the given one
 is left untouched, i.e. <one <two> [*] three> [//] <two> [*] is left
 untouched. It return the pointer to the first character of the given
 scoped data. "beg" is a begining of the string marker, "wline" is a
 pointer to a scoped data, "isblankit" if 1 means replace data with space
 character and if 0 means leave it untouched.
 */
int ExcludeScope(char *wline, int pos, char isblankit) {
	if (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) && pos >= 0) {
		while (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) && pos >= 0) {
			if (isblankit) {
				if (isQuotes(wline, pos)) {
					pos -= 2;
				} else
					wline[pos] = ' ';
			}
			pos--;
		}
		if (uS.isRightChar(wline, pos, '[', &dFnt, MBF)) {
			if (isblankit)
				wline[pos] = ' ';
		}
	}
	if (pos < 0) {
		fprintf(stderr,"Missing '[' character in file %s\n",oldfname);
		fprintf(stderr,"In tier on line: %ld.\n", lineno);
		fprintf(stderr,"text=%s->%s->%d\n",wline,wline+pos,isblankit);
		return(-1);
	} else
		pos--;
	while (!uS.isRightChar(wline, pos, '>', &dFnt, MBF) && uS.isskip(wline,pos,&dFnt,MBF) && pos >= 0) {
		if (uS.isRightChar(wline, pos, ']', &dFnt, MBF) || uS.isRightChar(wline, pos, '<', &dFnt, MBF))
			break;
		else
			pos--;
	}
	if (uS.isRightChar(wline, pos, '>', &dFnt, MBF)) {
		while (!uS.isRightChar(wline, pos, '<', &dFnt, MBF) && pos >= 0) {
			if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
				pos = ExcludeScope(wline,pos,!isSelectedScope(wline, pos));
			else if (isblankit) {
				if (isQuotes(wline, pos)) {
					pos -= 3;
				} else
					wline[pos--] = ' ';
			} else
				pos--;
		}
		if (pos < 0) {
			if (chatmode) {
				fprintf(stderr,"Missing '<' character in file %s\n", oldfname);
				fprintf(stderr,"In tier on line: %ld.\n", lineno+tlineno);
				fprintf(stderr,"text=%s\n",wline);
			}
			return(-1);
		} else if (isblankit) {
			wline[pos] = ' ';
		} else
			pos--;
	} else if (pos < 0) ;
	else if (uS.isRightChar(wline, pos, ']', &dFnt, MBF)) {
		pos = ExcludeScope(wline,pos,!isSelectedScope(wline, pos));
	} else {
		while (!uS.isskip(wline,pos,&dFnt,MBF) && pos >= 0) {
			if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
				pos = ExcludeScope(wline,pos,!isSelectedScope(wline, pos));
			else if (isblankit) {
				if (isQuotes(wline, pos)) {
					pos -= 3;
				} else
					wline[pos--] = ' ';
			} else
				pos--;
		}
	}
	return(pos);
}

void HandleParans(char *s, int beg, int end) {
	int temp, i;
	char buf[BUFSIZ+1], tParans, isXXXFound;

	temp = 0;
	for (i=beg; s[i] != EOS; i++) {
		if (temp >= BUFSIZ)
			break;
		if (!uS.isRightChar(s, i, '(', &dFnt, MBF) && !uS.isRightChar(s, i, ')', &dFnt, MBF)) {
			buf[temp++] = s[i];
		}
	}
	isXXXFound = FALSE;
	if (temp < BUFSIZ) {
		buf[temp] = EOS;
		if (uS.mStricmp(buf, "xxx") == 0 || uS.mStricmp(buf, "yyy") == 0 || uS.mStricmp(buf, "www") == 0) {
			tParans = Parans;
			Parans = 1;
			isXXXFound = TRUE;
		}
	}
	while (s[beg]) {
		if (uS.isRightChar(s, beg, '(', &dFnt, MBF) || uS.isRightChar(s, beg, ')', &dFnt, MBF)) {
			if (Parans == 1) {
				if (uS.isRightChar(s, beg, '(', &dFnt, MBF) && s[beg+1] == '*') {
					for (temp=beg; s[temp] && !uS.isRightChar(s, temp, ')', &dFnt, MBF); temp++) ;
					if (s[temp])
						strcpy(s+beg,s+temp+1);
					else
						beg++;
				} else
					strcpy(s+beg,s+beg+1);
			} else { /* if (Parans == 3) */
				for (temp=beg; s[temp] && !uS.isRightChar(s, temp, ')', &dFnt, MBF); temp++ );
				if (s[temp])
					strcpy(s+beg,s+temp+1);
				else
					beg++;
			}
		} else
			beg++;
	}
	if (isXXXFound)
		Parans = tParans;
	if (end != 0) {
		for (; beg < end; beg++)
			s[beg] = ' ';
	}
}

static void HandleMorCAs(char *s, int beg, int end, char isAlwaysRemove) {
	int  res, i;
	char CAc[20];
	int matchType;

	while (s[beg]) {
		if ((res=uS.HandleCAChars(s+beg, &matchType)) != 0) {
			if (isRemoveCAChar[matchType] == TRUE) {
				strncpy(CAc, s+beg, res);
				CAc[res] = EOS;
				if (!isCAsearched(CAc) || isAlwaysRemove) {
					if (matchType == NOTCA_LEFT_ARROW_CIRCLE)
						removeRepeatSegments(s, beg, res);
					else
						strcpy(s+beg,s+beg+res);
					i = beg - 1;
					if (s[i] == '|') {
						for (; i >= 0 && !uS.isskip(s,i,&dFnt,MBF); i--) {
							s[i] = ' ';
						}
					}
				} else
					beg += res;
			} else
				beg += res;
		} else
			beg++;
	}
	if (end != 0) {
		for (; beg < end; beg++)
			s[beg] = ' ';
	}
}

static void HandleSpCAs(char *s, int beg, int end, char isAlwaysRemove) {
	int  res;
	char CAc[20];
	int matchType;

	while (s[beg]) {
		if ((res=uS.HandleCAChars(s+beg, &matchType)) != 0) {
			if (isRemoveCAChar[matchType] == TRUE) {
				strncpy(CAc, s+beg, res);
				CAc[res] = EOS;
				if (!isCAsearched(CAc) || isAlwaysRemove) {
					if (matchType == NOTCA_LEFT_ARROW_CIRCLE)
						removeRepeatSegments(s, beg, res);
					else
						strcpy(s+beg,s+beg+res);
				} else
					beg += res;
			} else
				beg += res;
		} else
			beg++;
	}
	if (end != 0) {
		for (; beg < end; beg++)
			s[beg] = ' ';
	}
}

static void HandleSlash(char *s, int beg, int end) {
	int  res;
	char sq, isATFound, CAc[20];
	int matchType;

	sq = FALSE;
	isATFound = FALSE;
	if (s[beg] == '+' || s[beg] == '-')
		isATFound = TRUE;
	while (s[beg]) {
		if ((res=uS.HandleCAChars(s+beg, &matchType)) != 0) {
			if (isRemoveCAChar[matchType] == TRUE) {
				strncpy(CAc, s+beg, res);
				CAc[res] = EOS;
				if (!isCAsearched(CAc)) {
					if (matchType == NOTCA_LEFT_ARROW_CIRCLE)
						removeRepeatSegments(s, beg, res);
					else
						strcpy(s+beg,s+beg+res);
				} else
					beg += res;
			} else
				beg += res;
			continue;
		}
		if (uS.isRightChar(s, beg, '@', &dFnt, MBF) && !sq)
			isATFound = TRUE;
		if (uS.isRightChar(s, beg, '[', &dFnt, MBF))
			sq = TRUE;
		else if (uS.isRightChar(s, beg, ']', &dFnt, MBF))
			sq = FALSE;
		if (!sq && !isATFound) {
			if (uS.isRightChar(s,beg,'^',&dFnt,MBF) && R7Caret) {
				strcpy(s+beg,s+beg+1);
			} else if (uS.isRightChar(s,beg,':',&dFnt,MBF) && R7Colon) {
				strcpy(s+beg,s+beg+1);
			} else if (uS.isRightChar(s,beg,'~',&dFnt,MBF) && R7Tilda) {
				beg++;
//				strcpy(s+beg,s+beg+1);
			} else if (uS.isRightChar(s,beg,'/',&dFnt,MBF) && R7Slash) {
				if ((uS.isRightChar(s,beg-1,'+',&dFnt,MBF) && beg > 0) ||
					  (uS.isRightChar(s,beg-1,'/',&dFnt,MBF) && uS.isRightChar(s,beg-2,'+',&dFnt,MBF) && beg > 1) ||
					  (uS.isRightChar(s,beg-1,'"',&dFnt,MBF) && uS.isRightChar(s,beg-2,'+',&dFnt,MBF) && beg > 1))
					beg++;
				else
					strcpy(s+beg,s+beg+1);
			} else
				beg++;
		} else
			beg++;
	}
	if (end != 0) {
		for (; beg < end; beg++)
			s[beg] = ' ';
	}
}

void findWholeWord(int wi, char *word) {
	int i;

	i = 0;
	if (wi > 0 && !uS.isskip(utterance->line,wi-1,&dFnt,MBF) && (utterance->line[wi-1] == '~' || utterance->line[wi-1] == '$')) {
		for (; wi > 0 && utterance->line[wi] != ']' && !uS.isskip(utterance->line,wi,&dFnt,MBF); wi--) ;
		if (wi < 0)
			wi = 0;
		if (utterance->line[wi] == ']' || uS.isskip(utterance->line,wi,&dFnt,MBF))
			wi++;
	}
	while (utterance->line[wi] != EOS && utterance->line[wi] != '[' && !uS.isskip(utterance->line,wi,&dFnt,MBF)) {
		word[i++] = utterance->line[wi++];
	}
	word[i] = EOS;
}

void findWholeScope(int wi, char *word) {
	int sqb, ang, isword;
	int i, ti, eg;
	char tNOTCA_CROSSED_EQUAL, tNOTCA_LEFT_ARROW_CIRCLE;

	i = 0;
	word[i] = EOS;
	for (ti=wi; ti >= 0 && utterance->line[ti] != '[' && utterance->line[ti] != ']'; ti--) ;
	if (ti >= 0 && utterance->line[ti] == '[' && utterance->line[ti+1] == ':') {
		if (utterance->line[ti+2] == ':' && utterance->line[ti+3] == ' ') {
			wi = ti + 4;
		} else if (utterance->line[ti+2] == ' ') {
			wi = ti + 3;
		}
	}
	if ((wi > 3 && utterance->line[wi-4] == '[' && utterance->line[wi-3] == ':' && utterance->line[wi-2] == ':' && utterance->line[wi-1] == ' ') ||
		  (wi > 2 && utterance->line[wi-3] == '[' && utterance->line[wi-2] == ':' && utterance->line[wi-1] == ' ')) {
		for (; utterance->line[wi] != EOS && utterance->line[wi] != ']' && utterance->line[wi] != '['; wi++) ;
		if (utterance->line[wi] == EOS || utterance->line[wi] == '[') {
			word[0] = EOS;
			return;
		}
		eg = wi+1;
		sqb = 0;
		for (; utterance->line[eg] != EOS; eg++) {
			if (utterance->line[eg] == ']')
				sqb--;
			else if (utterance->line[eg] == '[') {
				if (sqb > 0)
					break;
				sqb++;
			} else if (utterance->line[eg] == '<' && utterance->line[eg-1] != '+' && sqb <= 0) {
				eg--;
				break;
			} else if (!uS.isskip(utterance->line,eg,&dFnt,MBF) && sqb <= 0) {
				eg--;
				break;
			}
		}
		if (utterance->line[eg] == '[') {
			word[0] = EOS;
			return;
		}
		sqb = 0;
		ang = 0;
		isword = 0;
		for (; wi >= 0; wi--) {
			if (utterance->line[wi] == ']')
				sqb++;
			else if (utterance->line[wi] == '[')
				sqb--;
			else if (utterance->line[wi] == '>' && utterance->line[wi-1] != '+' && sqb <= 0)
				ang++;
			else if (utterance->line[wi] == '<' && utterance->line[wi-1] != '+' && sqb <= 0)
				ang--;
			else if (uS.isskip(utterance->line,wi,&dFnt,MBF) && sqb <= 0 && ang <= 0 && isword > 0)
				break;
			else if (!uS.isskip(utterance->line,wi,&dFnt,MBF) && sqb <= 0 && isword <= 0)
				isword++;
		}
		if (wi < 0)
			wi = 0;
		while (utterance->line[wi] != EOS && wi <= eg && uS.isskip(utterance->line,wi,&dFnt,MBF))
			wi++;
		i = 0;
		if (wi <= eg) {
			for (; wi <= eg; wi++) {
				if (utterance->line[wi] == '[' && utterance->line[wi+1] == '/') {
					for (; wi <= eg && utterance->line[wi] != ']'; wi++) ;
				} else if (utterance->line[wi] != '<' && utterance->line[wi] != '>')
					word[i++] = utterance->line[wi];
			}
		}
		word[i] = EOS;
	} else if (utterance->line[wi] != '[') {
		sqb = 0;
		ang = 0;
		i = 0;
		if (!uS.isskip(utterance->line,wi,&dFnt,MBF)) {
			ti = wi;
			for (wi--; wi >= 0 && utterance->line[wi] != ']' && uS.isskip(utterance->line,wi,&dFnt,MBF); wi--) {
				if (utterance->line[wi] == ']')
					sqb++;
				else if (utterance->line[wi] == '[')
					sqb--;
				else if (utterance->line[wi] == '>' && utterance->line[wi-1] != '+' && sqb <= 0)
					ang++;
				else if (utterance->line[wi] == '<' && utterance->line[wi-1] != '+' && sqb <= 0)
					ang--;
			}
			if (ang != 0) {
				for (wi++; utterance->line[wi] != EOS && utterance->line[wi] != '<' && uS.isskip(utterance->line,wi,&dFnt,MBF); wi++) ;
				while (utterance->line[wi] != EOS && uS.isskip(utterance->line,wi,&dFnt,MBF)) {
					word[i++] = utterance->line[wi++];
				}
			} else
				wi = ti;
		}
		while (utterance->line[wi] != EOS && utterance->line[wi] != '[' && !uS.isskip(utterance->line,wi,&dFnt,MBF)) {
			word[i++] = utterance->line[wi++];
		}
		ti = i;
		while (utterance->line[wi] != EOS && uS.isskip(utterance->line,wi,&dFnt,MBF) && utterance->line[wi] != '[') {
			word[i++] = utterance->line[wi++];
		}
		if (utterance->line[wi] == '[' && utterance->line[wi+1] == '*') {
			word[i++] = utterance->line[wi++];
			while (utterance->line[wi] != EOS && utterance->line[wi] != ']' && utterance->line[wi] != '[') {
				word[i++] = utterance->line[wi++];
			}
			if (utterance->line[wi] == EOS || utterance->line[wi] == '[')
				i = ti;
			else
				word[i++] = utterance->line[wi++];
		} else
			i = ti;
		word[i] = EOS;
		if (ang != 0) {
			sqb = 0;
			ang = 0;
			for (i=0; word[i] != EOS; i++) {
				if (word[i] == ']')
					sqb++;
				else if (word[i] == '[')
					sqb--;
				else if (word[i] == '>' && sqb <= 0)
					ang++;
				else if (word[i] == '<' && sqb <= 0)
					ang--;
			}
			if (ang != 0) {
				for (i=0; word[i] != EOS; i++) {
					if (word[i] == '>' && sqb <= 0)
						word[i] = ' ';
					else if (word[i] == '<' && sqb <= 0)
						word[i] = ' ';
				}
			}
		}
	} else {
		for (wi++; utterance->line[wi] != EOS && utterance->line[wi] != ']' && utterance->line[wi] != '['; wi++) ;
		if (utterance->line[wi] == EOS || utterance->line[wi] == '[') {
			word[0] = EOS;
			return;
		}
		eg = wi+1;
		sqb = 0;
		for (; utterance->line[eg] != EOS; eg++) {
			if (utterance->line[eg] == ']')
				sqb--;
			else if (utterance->line[eg] == '[') {
				if (sqb > 0)
					break;
				sqb++;
			} else if (utterance->line[eg] == '<' && utterance->line[eg-1] != '+' && sqb <= 0) {
				eg--;
				break;
			} else if (!uS.isskip(utterance->line,eg,&dFnt,MBF) && sqb <= 0) {
				eg--;
				break;
			}
		}
		if (utterance->line[eg] == '[') {
			word[0] = EOS;
			return;
		}
		sqb = 0;
		ang = 0;
		isword = 0;
		for (; wi >= 0; wi--) {
			if (utterance->line[wi] == ']')
				sqb++;
			else if (utterance->line[wi] == '[')
				sqb--;
			else if (utterance->line[wi] == '>' && utterance->line[wi-1] != '+' && sqb <= 0)
				ang++;
			else if (utterance->line[wi] == '<' && utterance->line[wi-1] != '+' && sqb <= 0)
				ang--;
			else if (uS.isskip(utterance->line,wi,&dFnt,MBF) && sqb <= 0 && ang <= 0 && isword > 0)
				break;
			else if (!uS.isskip(utterance->line,wi,&dFnt,MBF) && sqb <= 0 && isword <= 0)
				isword++;
		}
		if (wi < 0)
			wi = 0;
		while (utterance->line[wi] != EOS && wi <= eg && uS.isskip(utterance->line,wi,&dFnt,MBF))
			wi++;
		i = 0;
		if (wi <= eg) {
			for (; wi <= eg; wi++) {
				if (utterance->line[wi] == '[' && utterance->line[wi+1] == '/') {
					for (; wi <= eg && utterance->line[wi] != ']'; wi++) ;
				} else if (utterance->line[wi] != '<' && utterance->line[wi] != '>')
					word[i++] = utterance->line[wi];
			}
		}
		word[i] = EOS;
	}
	tNOTCA_CROSSED_EQUAL = isRemoveCAChar[NOTCA_CROSSED_EQUAL];
	tNOTCA_LEFT_ARROW_CIRCLE = isRemoveCAChar[NOTCA_LEFT_ARROW_CIRCLE];
	isRemoveCAChar[NOTCA_CROSSED_EQUAL] = FALSE;
	isRemoveCAChar[NOTCA_LEFT_ARROW_CIRCLE] = FALSE;
	HandleSpCAs(word, 0, strlen(word), FALSE);
	isRemoveCAChar[NOTCA_CROSSED_EQUAL] = tNOTCA_CROSSED_EQUAL;
	isRemoveCAChar[NOTCA_LEFT_ARROW_CIRCLE] = tNOTCA_LEFT_ARROW_CIRCLE;
	uS.remFrontAndBackBlanks(word);
}

static void filterPause(char *wline, char opauseFound, char pauseFound) {
	int pos, temp, i;
	char res, pa;

	pos = 0;
	do {
		for (; uS.isskip(wline,pos,&dFnt,MBF) && wline[pos] && !uS.isRightChar(wline,pos,'[',&dFnt,MBF); pos++) ;
		pa = FALSE;
		if (wline[pos]) {
			temp = pos;
			if (uS.isRightChar(wline, pos, '[', &dFnt, MBF) && uS.isSqBracketItem(wline,pos+1,&dFnt,MBF)) {
					for (pos++; !uS.isRightChar(wline, pos, ']', &dFnt, MBF) && wline[pos]; pos++) ;
				if (wline[pos])
					pos++;
			} else if (uS.isRightChar(wline, pos, '(', &dFnt, MBF) && uS.isPause(wline, pos, NULL,  &i)) {
				pos = i + 1;
				pa = TRUE;
			} else {
				do {
					for (pos++; !uS.isskip(wline,pos,&dFnt,MBF) && wline[pos]; pos++) {
						if (uS.isRightChar(wline, pos, '.', &dFnt, MBF)) {
							if (uS.isPause(wline, pos, &i, NULL)) {
								pos = i;
								if (temp > pos)
									temp = pos;
								break;
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
			if (opauseFound && uS.patmat(wline+temp, "#*")) {
				for (; temp != pos; temp++)
					wline[temp] = ' ';
			} else if (pa && pauseFound) {
				for (; temp != pos; temp++)
					wline[temp] = ' ';
			}
			wline[pos] = res;
		}
	} while (wline[pos]) ;
}

static void filterscop(const char *sp, char *wline) {
	int pos, temp, t2, LastPos, res, sqCnt;

	pos = strlen(wline) - 1;
	LastPos = pos + 1;
	while (pos >= 0) {
		if (wline[pos] == HIDEN_C) {
			temp = pos;
			for (pos--; wline[pos] != HIDEN_C && pos >= 0; pos--) ;
			if (wline[pos] == HIDEN_C && pos >= 0) {
				for (; temp >= pos; temp--)
					wline[temp] = ' ';
			}
		} else if (uS.isRightChar(wline, pos, ']', &dFnt, MBF)) {
			sqCnt = 0;
			temp = pos;
			for (pos--; (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) || sqCnt > 0) && pos >= 0; pos--) {
				if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
					sqCnt++;
				else if (uS.isRightChar(wline, pos, '[', &dFnt, MBF))
					sqCnt--;
			}
			if (pos < 0) {
				if (chatmode) {
					if (!cutt_isBlobFound) {
						fprintf(stderr,"Missing '[' character in file %s\n",oldfname);
						fprintf(stderr,"In tier on line: %ld.\n", lineno+tlineno);
						fprintf(stderr,"text=%s\n",wline);
					}
					pos = temp - 1;
					if (isRecursive || cutt_isCAFound || cutt_isBlobFound)
						continue;
					else
						cutt_exit(1);
				} else
					break;
			}
			wline[temp] = EOS;
			uS.isSqCodes(wline+pos+1, templineC3, &dFnt, TRUE);
			wline[temp] = ']';
			uS.lowercasestr(templineC3, &dFnt, MBF);
			res = isExcludeScope(templineC3);
			if (res == 0 || res == 100/* || res == 2*/)
				ExcludeScope(wline,pos,(char)TRUE);
			if ((res == 1 || res == 3) && ScopMode == 'I') {
				for (t2=temp+1; t2 < LastPos; t2++)
					wline[t2] = ' ';
				if (uS.isRightChar(wline, pos, '[', &dFnt, MBF))
					pos--;
				while (pos >= 0 && (sqCnt > 0 || (uS.isskip(wline,pos,&dFnt,MBF) && !uS.isRightChar(wline,pos,'>',&dFnt,MBF)))) {
					if (uS.isRightChar(wline,pos,']',&dFnt,MBF))
						sqCnt++;
					else if (uS.isRightChar(wline,pos,'[',&dFnt,MBF))
						sqCnt--;

					pos--;
				}
				if (pos >= 0) {
					if (uS.isRightChar(wline, pos, '>', &dFnt, MBF)) {
						while (pos >= 0 && !uS.isRightChar(wline, pos, '<', &dFnt, MBF)) pos--;
						if (pos < 0) {
							if (chatmode) {
								fprintf(stderr,"Missing '<' character in file %s\n", oldfname);
								fprintf(stderr,"text=%s\n",wline);
								cutt_exit(1);
							}
						} else pos--;
					} else {
						while (pos >= 0 && !uS.isskip(wline,pos,&dFnt,MBF) && !uS.isRightChar(wline,pos,']',&dFnt,MBF))
							pos--;
					}
				}
				LastPos = pos + 1;
			}
			if (res == 2 && ScopMode == 'I') {
				for (t2=temp+1; t2 < LastPos; t2++)
					wline[t2] = ' ';
				LastPos = pos;
			} else if (res < 2) {
				for (; !uS.isRightChar(wline, temp, '[', &dFnt, MBF); temp--)
					wline[temp] = ' ';
				wline[temp] = ' ';
			} else if (res == 2013) {
				for (pos=0; wline[pos]; pos++)
					wline[pos] = ' ';
				return;
			} else if (res == 1001) {
				ExcludeScope(wline,pos,(char)TRUE);
				wline[temp] = ' ';
				for (; !uS.isRightChar(wline, temp, '[', &dFnt, MBF); temp--) ;
				wline[temp] = ' ';
				wline[temp+1] = ' ';
				if (wline[temp+2] == ':')
					wline[temp+2] = ' ';
				if (wline[temp+2] == '*')
					wline[temp+2] = ' ';
				if (wline[temp+2] == '=' && !isSpace(wline[temp+3])) {
					wline[temp+2] = ' ';
					wline[temp+3] = ' ';
				}
			} else if (res == 4) {
				if (postcodeRes != 5)
					postcodeRes = 4;
				for (; !uS.isRightChar(wline, temp, '[', &dFnt, MBF); temp--)
					wline[temp] = ' ';
				wline[temp] = ' ';
			} else if (res == 5) {
				postcodeRes = 5;
				for (pos=0; wline[pos]; pos++)
					wline[pos] = ' ';
				return;
			} else if (res == 6) {
				if (postcodeRes != 5)
					postcodeRes = 6;
			} else if (res == 7) {
//				both [...] and <...> are preserved and if +s[...] used, then it is searched for
			}
		} else
			pos--;
	}
	if (ScopMode == 'I') {
		for (pos=0; pos < LastPos; pos++)
			wline[pos] = ' ';
	}
	if (PostCodeMode == 'i') {
		if (postcodeRes == 0) {
			for (pos=0; wline[pos]; pos++)
				wline[pos] = ' ';
		}
	}
	if (*sp == '%' && postcodeRes == 5) {
		for (pos=0; wline[pos]; pos++)
			wline[pos] = ' ';
	}
}

void filterwords(const char *sp, char *wline, int (*excludeP)(char *)) {
	int pos, temp, i;
	char res, pa;

	if (*sp == '@') {
		if (uS.mStrnicmp(sp+1, "ID:", 3) == 0)
			return;
	}
	cutt_depWord = FALSE;
	pos = 0;
	do {
		if (sp[0] == '%' && uS.mStrnicmp(sp+1, "mor:", 4) == 0) {
			for (; uS.isskip(wline,pos,&dFnt,MBF) && (wline[pos] != '?' || wline[pos+1] != '|') && wline[pos] && !uS.isRightChar(wline,pos,'[',&dFnt,MBF); pos++) {
				if (wline[pos] == dMarkChr)
					cutt_depWord = TRUE;
			}
		} else {
			for (; uS.isskip(wline,pos,&dFnt,MBF) && wline[pos] && !uS.isRightChar(wline,pos,'[',&dFnt,MBF); pos++) ;
		}
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
							pos += 3;
							break;
						} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x9D) {
							pos += 3;
							break;
						} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x98) {
							pos += 3;
							break;
						} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x99) {
							pos += 3;
							break;
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
							if (pos > 0 && wline[pos-1] == '|' && *sp == '%') {
							} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x9C) {
								break;
							} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x9D) {
								break;
							} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x98) {
								break;
							} else if (wline[pos] == (char)0xE2 && wline[pos+1] == (char)0x80 && wline[pos+2] == (char)0x99) {
								break;
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
			if (!uS.isToneUnitMarker(wline+temp) && !pa) {
				if (isMainSpeaker(*sp)) {
					if (Parans != 2) {
						HandleParans(wline,temp,pos);
						if (pos > 0 && pos > temp && isSpace(wline[pos-1])) {
							wline[pos] = res;
							for (pos--; pos > temp-1 && isSpace(wline[pos]); pos--) ;
							pos++;
							res = wline[pos];
							wline[pos] = EOS;
						}
					}
					if (R7Slash || R7Tilda || R7Caret || R7Colon) {
						HandleSlash(wline,temp,pos);
						if (pos > 0 && pos > temp && isSpace(wline[pos-1])) {
							wline[pos] = res;
							for (pos--; pos > temp-1 && isSpace(wline[pos]); pos--) ;
							pos++;
							res = wline[pos];
							wline[pos] = EOS;
						}
					} else {
						HandleSpCAs(wline,temp,pos,FALSE);
						if (pos > 0 && pos > temp && isSpace(wline[pos-1])) {
							wline[pos] = res;
							for (pos--; pos > temp-1 && isSpace(wline[pos]); pos--) ;
							pos++;
							res = wline[pos];
							wline[pos] = EOS;
						}
					}
				} else {
					if (uS.partcmp(sp,"%mor:",FALSE, FALSE)) {
						HandleMorCAs(wline,temp,pos,FALSE);
						if (pos > 0 && pos > temp && isSpace(wline[pos-1])) {
							wline[pos] = res;
							for (pos--; pos > temp-1 && isSpace(wline[pos]); pos--) ;
							pos++;
							res = wline[pos];
							wline[pos] = EOS;
						}
					}
				}
			}
			if (excludeP != NULL) {
				if (!(*excludeP)(wline+temp)) {
					for (; temp < pos; temp++)
						wline[temp] = ' ';
				} else {
					for (; temp < pos; temp++) {
						if (wline[temp] == EOS)
							wline[temp] = ' ';
					}
				}
			}
			wline[pos] = res;
			cutt_depWord = FALSE;
		}
	} while (wline[pos]) ;
}

/* filterData(sp,wline) filters out all the wrong data from the string pointed
   to by "wline". "sp" is the first character in a speaker string.
*/
void filterData(const char *sp, char *wline) {
	if (*sp != '@') {
		if (opauseFound || pauseFound)
			filterPause(wline, opauseFound, pauseFound);
		filterscop(sp, wline);
	}
	filterwords(sp,wline,excludedef);
}

FILE *OpenGenLib(const FNType *fname, const char *mode, char checkWD, char checkSubDir, FNType *mFileName) {
	FILE *fp;
	int  t;
	FNType mDirPathName[FNSize];
#if defined(UNX) || defined(_MAC_CODE)
	struct dirent *dp;
	struct stat sb;
	DIR *cDIR;
#endif // _MAC_CODE
#ifdef _WIN32
	BOOL notDone;
	CString dirname;
	CFileFind fileFind;
	FNType tFileName[FILENAME_MAX];
#endif // _WIN32

	if (!isRefEQZero(fname)) {
		fp = fopen(fname, mode);
		strcpy(mFileName, fname);
	} else if (checkWD) {
		strcpy(mFileName, wd_dir);
		addFilename2Path(mFileName, fname);
		fp = fopen(mFileName, mode);
	} else {
		mFileName[0] = EOS;
		fp = NULL;
	}	
#if defined(UNX)
	if (!checkWD) {
		strcpy(mFileName, wd_dir);
		addFilename2Path(mFileName, fname);
		fp = fopen(mFileName, mode);
	}
#endif
	if (fp == NULL) {
#if defined(UNX)
		getcwd(mDirPathName, FNSize);
		strcpy(mFileName,lib_dir);
		t = strlen(mFileName);
		addFilename2Path(mFileName, fname);
		fp = fopen(mFileName, mode);
		if (checkSubDir && fp == NULL) {
			SetNewVol(lib_dir);
			if ((cDIR=opendir(".")) != NULL) {
				while ((dp=readdir(cDIR)) != NULL) {
					if (stat(dp->d_name, &sb) == 0) {
						if (!S_ISDIR(sb.st_mode)) {
							continue;
						}
					} else
						continue;
					if (dp->d_name[0] == '.')
						continue;
					mFileName[t] = EOS;
					addFilename2Path(mFileName, dp->d_name);
					addFilename2Path(mFileName, fname);
					fp = fopen(mFileName, mode);
					if (fp != NULL) {
						break;
					}
				}
				closedir(cDIR);
			}
		}
		SetNewVol(mDirPathName);
#else // defined(UNX)
		my_getcwd(mDirPathName, FNSize);
		if (isRefEQZero(lib_dir)) {/* no library directory yet */
			if (!LocateDir("Clan library directory",lib_dir,false))
				return(NULL);
			WriteCedPreference();
  #if defined(_MAC_CODE)
			UpdateWindowNamed(Commands_str);
  #endif // _MAC_CODE
  #ifdef _WIN32
			if (clanDlg != NULL) {
				u_strcpy(clanDlg->t_st, lib_dir, FNSize);
				AdjustName(clanDlg->lib_st, clanDlg->t_st, 39);
				clanDlg->m_LibSt = clanDlg->lib_st;
				clanDlg->UpdateData(FALSE);
			}
  #endif // _WIN32
		}
		if (!isRefEQZero(lib_dir)) {	/* we have a lib */
			strcpy(mFileName,lib_dir);
			t = strlen(mFileName);
			addFilename2Path(mFileName, fname);
			fp = fopen(mFileName, mode);
			if (checkSubDir && fp == NULL) {
  #if defined(_MAC_CODE)
				SetNewVol(lib_dir);
				if ((cDIR=opendir(".")) != NULL) {
					while ((dp=readdir(cDIR)) != NULL) {
						if (stat(dp->d_name, &sb) == 0) {
							if (!S_ISDIR(sb.st_mode)) {
								continue;
							}
						} else
							continue;
						if (dp->d_name[0] == '.')
							continue;
						mFileName[t] = EOS;
						addFilename2Path(mFileName, dp->d_name);
						addFilename2Path(mFileName, fname);
						fp = fopen(mFileName, mode);
						if (fp != NULL) {
							break;
						}
					}
					closedir(cDIR);
				}
				SetNewVol(mDirPathName);
  #endif // _MAC_CODE
  #ifdef _WIN32
				SetNewVol(lib_dir);
				if (!fileFind.FindFile(_T("*.*"), 0)) {
					fileFind.Close();
				} else {
					do {
						notDone = fileFind.FindNextFile();
						dirname = fileFind.GetFileName();
						if (!fileFind.IsDirectory())
							continue;
						dirname = fileFind.GetFileName();
						if (!strcmp(dirname, ".") || !strcmp(dirname, ".."))
							continue;
						mFileName[t] = EOS;
						u_strcpy(tFileName, dirname, FILENAME_MAX);
						addFilename2Path(mFileName, tFileName);
						addFilename2Path(mFileName, fname);
						fp = fopen(mFileName, mode);
						if (fp != NULL) {
							break;
						}
					} while (notDone) ;
					fileFind.Close();
				}
				SetNewVol(mDirPathName);
  #endif // _WIN32
			}
		}
#endif // !defined(UNX)
		if (fp == NULL) {
			strcpy(mFileName,lib_dir);
			addFilename2Path(mFileName, fname);
		}
	}
	return(fp);
}

FILE *OpenMorLib(const FNType *fname, const char *mode, char checkWD, char checkSubDir, FNType *mFileName) {
	FILE *fp;
	int  t;
#if defined(UNX) || defined(_MAC_CODE)
	struct dirent *dp;
	struct stat sb;
	DIR *cDIR;
#endif // _MAC_CODE
#ifdef _WIN32
	BOOL notDone;
	CString dirname;
	CFileFind fileFind;
	FNType tFileName[FILENAME_MAX];
#endif // _WIN32
	if (checkWD) {
		strcpy(mFileName, wd_dir);
		addFilename2Path(mFileName, fname);
		fp = fopen(mFileName, mode);
	} else {
		mFileName[0] = EOS;
		fp = NULL;
	}

	if (fp == NULL) {
#if defined(UNX)
		strcpy(mFileName, mor_lib_dir);
		t = strlen(mFileName);
		addFilename2Path(mFileName, fname);
		fp = fopen(mFileName, mode);
#else // defined(UNX)
		if (isRefEQZero(mor_lib_dir)) {/* no library directory yet */
			if (!LocateDir("Clan mor library directory",mor_lib_dir,false))
				return(NULL);
			WriteCedPreference();
  #if defined(_MAC_CODE)
			UpdateWindowNamed(Commands_str);
  #endif // _MAC_CODE
  #ifdef _WIN32
			if (clanDlg != NULL) {
				u_strcpy(clanDlg->t_st, mor_lib_dir, FNSize);
				AdjustName(clanDlg->mor_lib_st, clanDlg->t_st, 39);
				clanDlg->m_morLibSt = clanDlg->mor_lib_st;
				clanDlg->UpdateData(FALSE);
			}
  #endif // _WIN32
		}
		if (!isRefEQZero(mor_lib_dir)) {	/* we have a lib */
			strcpy(mFileName, mor_lib_dir);
			t = strlen(mFileName);
			addFilename2Path(mFileName, fname);
			fp = fopen(mFileName, mode);
			if (checkSubDir && fp == NULL) {
  #if defined(_MAC_CODE)
				SetNewVol(mor_lib_dir);
				if ((cDIR=opendir(".")) != NULL) {
					while ((dp=readdir(cDIR)) != NULL) {
						if (stat(dp->d_name, &sb) == 0) {
							if (!S_ISDIR(sb.st_mode)) {
								continue;
							}
						} else
							continue;
						if (dp->d_name[0] == '.')
							continue;
						mFileName[t] = EOS;
						addFilename2Path(mFileName, dp->d_name);
						addFilename2Path(mFileName, fname);
						fp = fopen(mFileName, mode);
						if (fp != NULL) {
							break;
						}
					}
					closedir(cDIR);
				}
				if (WD_Not_Eq_OD)
					SetNewVol(od_dir);
				else
					SetNewVol(wd_dir);
  #endif // _MAC_CODE
  #ifdef _WIN32
				SetNewVol(mor_lib_dir);
				if (!fileFind.FindFile(_T("*.*"), 0)) {
					fileFind.Close();
				} else {
					do {
						notDone = fileFind.FindNextFile();
						dirname = fileFind.GetFileName();
						if (!fileFind.IsDirectory())
							continue;
						dirname = fileFind.GetFileName();
						if (dirname[0] == '.')
							continue;
						mFileName[t] = EOS;
						u_strcpy(tFileName, dirname, FILENAME_MAX);
						addFilename2Path(mFileName, tFileName);
						addFilename2Path(mFileName, fname);
						fp = fopen(mFileName, mode);
						if (fp != NULL) {
							break;
						}
					} while (notDone) ;
					fileFind.Close();
				}
				if (WD_Not_Eq_OD)
					SetNewVol(od_dir);
				else
					SetNewVol(wd_dir);
  #endif // _WIN32
			}
		}
#endif // !defined(UNX)
		if (fp == NULL) {
			strcpy(mFileName,mor_lib_dir);
			addFilename2Path(mFileName, fname);
		}
	}
	return(fp);
}

void init_punct(char which) {
	if (which == 0)
		strcpy(cedPunctuation, PUNCTUATION_SET);
	else
		strcpy(cedPunctuation, PUNCT_PHO_MOD_SET);
	punctuation = cedPunctuation;
	morPunctuation[0] = EOS;
}

/* getpunct(pfile) opens pfile file and reads puctuation set. */
/* 2011-01-26 
static void getpunct(FNType *pfile) {
	FILE *pfp;
	char s[BUFSIZ];
	FNType mFileName[FNSize];

	if (FirstTime) {
		if (*pfile == EOS)
			uS.str2FNType(pfile, 0L, "punct.cut");
		if (WD_Not_Eq_OD)
			SetNewVol(wd_dir);
		if ((pfp=OpenGenLib(pfile,"r",TRUE,TRUE,mFileName)) != NULL) {
			if (strlen(mFileName) < 512-30) {
				sprintf(punctMess, "    Using punctuation file: %s.\n", mFileName);
			} else
				*punctMess = EOS;
			fprintf(stderr, "    Using punctuation file: %s.\n", mFileName);
			if ((!stout || !IsOutTTY()) && (!onlydata || !puredata) && !outputOnlyData) {
				fprintf(fpout, "    Using punctuation file: %s.\n", mFileName);
			}
			if (FirstTime) {
				do {
					if (fgets_cr(s, BUFSIZ, pfp)) {
						if (uS.isUTF8(s))
							continue;
						if (strlen(s) > 48) {
							fprintf(stderr,"Too many punctuation symbols in a file.\n");
							fprintf(stderr,"48 symbols is a top limit.\n");
							cutt_exit(0);
						}
						uS.remblanks(s);
						strcpy(GlobalPunctuation, s);
						punctuation = GlobalPunctuation;
					}
				} while (uS.isUTF8(s)) ;
			}
			fclose(pfp);
		} else
			*punctMess = EOS;
		if (WD_Not_Eq_OD)
			SetNewVol(od_dir);
		else
			SetNewVol(wd_dir);
	} else if (*punctMess != EOS) {
		if ((!stout || !IsOutTTY()) && (!onlydata || !puredata) && !outputOnlyData) {
			fputs(punctMess, fpout);
		}
	}
}
*/
/**************************************************************/
/*	 main initialization routines				  */
void mmaininit(void) {
	int i;

	for (i=0; i <= NUMSPCHARS; i++) // up-arrow
		isRemoveCAChar[i] = TRUE;
	isRemoveCAChar[NOTCA_STRESS] = FALSE;
	isRemoveCAChar[NOTCA_GLOTTAL_STOP] = FALSE;
	isRemoveCAChar[NOTCA_HEBREW_GLOTTAL] = FALSE;
	isRemoveCAChar[NOTCA_CARON] = FALSE;
	Parans = 1;
	nomain = FALSE;
	FromWU = 0;
	fpin = NULL;
	fpout = NULL;
	wdUttLen = NULL;
	wdptr = NULL;
	defwdptr = NULL;
	mwdptr = NULL;
	morfptr = NULL;
	langptr = NULL;
	CAptr = NULL;
	linkMain2Mor = FALSE;
	isExpendX = TRUE;
	isMorWordSpec = 0;
	anyMultiOrder = FALSE;
	onlySpecWsFound = FALSE;
	isPostcliticUse = FALSE;
	isPrecliticUse = FALSE;
	cutt_isMultiFound = FALSE;
	cutt_isCAFound = FALSE, 
	cutt_isBlobFound = FALSE;
	cutt_depWord = FALSE;
	ml_root_clause = NULL;
	ByTurn = 0;
	CntWUT = 0;
	CntFUttLen = 0;
	filterUttLen_cmp = 0;
	filterUttLen = 0L;
	restoreXXX = FALSE;
	restoreYYY = FALSE;
	restoreWWW = FALSE;
	mor_link.error_found = FALSE;
	mor_link.fname[0] = EOS;
	mor_link.lineno = 0L;
	y_option = 0;
#if !defined(CLAN_SRV)
	f_override = FALSE;
#else
	f_override = TRUE;
#endif
	Toldsp = NULL;
	TSoldsp = NULL;
	MAXOUTCOL = 76L;
	R4 = FALSE;
	if (CLAN_PROG_NUM == WDLEN)
		R5 = FALSE;
	else
		R5 = TRUE;
	R5_1 = FALSE;
	if (CLAN_PROG_NUM == MLT || CLAN_PROG_NUM == MLU || CLAN_PROG_NUM == MODREP || CLAN_PROG_NUM == REPEAT ||
		CLAN_PROG_NUM == FLO || CLAN_PROG_NUM == FREQ || CLAN_PROG_NUM == DSS || CLAN_PROG_NUM == RETRACE || CLAN_PROG_NUM == MOR_P ||
		CLAN_PROG_NUM == JOINITEMS || CLAN_PROG_NUM==SYNCODING || CLAN_PROG_NUM==IMDI_P || CLAN_PROG_NUM==LAB2CHAT ||
		CLAN_PROG_NUM == MORTABLE || CLAN_PROG_NUM == FIXLANG || CLAN_PROG_NUM == SUBTITLES || CLAN_PROG_NUM == IPSYN ||
		CLAN_PROG_NUM == EVAL || CLAN_PROG_NUM == KIDEVAL || CLAN_PROG_NUM == SCRIPT_P || CLAN_PROG_NUM == WDLEN)
		R6 = FALSE;
	else
		R6 = TRUE;
	R6_override = FALSE;
	R7Slash = TRUE;
	R7Tilda = TRUE;
	R7Caret = TRUE;
	R7Colon = TRUE;
	R8 = FALSE;
	if (CLAN_PROG_NUM == FREQ || CLAN_PROG_NUM == FREQMERGE || CLAN_PROG_NUM == DSS || CLAN_PROG_NUM == PHONFREQ ||
		  CLAN_PROG_NUM == RELY || CLAN_PROG_NUM == CP2UTF || CLAN_PROG_NUM == LOWCASE || CLAN_PROG_NUM == ORT ||
		  CLAN_PROG_NUM == MOR_P || CLAN_PROG_NUM == TRNFIX || CLAN_PROG_NUM == JOINITEMS || CLAN_PROG_NUM == SYNCODING ||
		  CLAN_PROG_NUM == IMDI_P || CLAN_PROG_NUM == LAB2CHAT || CLAN_PROG_NUM == MORTABLE || CLAN_PROG_NUM == FIXLANG ||
		  CLAN_PROG_NUM == SUBTITLES || CLAN_PROG_NUM == FLO || CLAN_PROG_NUM == IPSYN || CLAN_PROG_NUM == SCRIPT_P ||
		  CLAN_PROG_NUM == TIMEDUR || CLAN_PROG_NUM == EVAL || CLAN_PROG_NUM == KIDEVAL)
		nomap = TRUE;
	else
		nomap = FALSE;
	isSpecRepeatArrow = FALSE;
#ifndef UNX
	copyNewFontInfo(&dFnt, &oFnt);
	if (dFnt.Encod == 1 || dFnt.Encod == 2 || dFnt.Encod == 3) {
		MBF = TRUE;
	} else
#endif
		MBF = FALSE;
	WUCounter = 0L;
	contSpeaker[0] = EOS;
	OverWriteFile = FALSE;
	PreserveFileTypes = FALSE;
	AddCEXExtension = ".cex";
	fontErrorReported = FALSE;
	IncludeAllDefaultCodes = FALSE;
	FilterTier = 2;
	SPRole = NULL;
	IDField = NULL;
	CODEField = NULL;
	headtier = NULL;
	ProgDrive = '\0';
	OutputDrive = '\0';
	HeadOutTier = NULL;
	combinput = FALSE;
	isRecursive = FALSE;
	ScopWdPtr[0] = NULL;
#ifndef UNX
	if (WD_Not_Eq_OD)
		Preserve_dir = FALSE;
	else
#endif
		Preserve_dir = TRUE;
	isLanguageExplicit = FALSE;
	opauseFound = FALSE;
	pauseFound = FALSE;
	RemPercWildCard = TRUE;
	LocalTierSelect = FALSE;
	cMediaFileName[0] = EOS;
//2011-01-26	uS.str2FNType(punctFile, 0L, "punct.cut");
//2011-01-26	punctMess[0] = EOS;
	strcpy(GlobalPunctuation, PUNCTUATION_SET);
	punctuation = GlobalPunctuation;
	morPunctuation[0] = EOS;
	rootmorf = (char *)malloc(strlen(MORPHS)+1);
	if (rootmorf == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(0);
	}
	strcpy(rootmorf,MORPHS);
	utterance = NEW(UTTER);
	if (utterance == NULL) {
		fprintf(stderr,"No more space left in core.\n");
		cutt_exit(0);
	}
	*utterance->speaker	= EOS;
	*utterance->attSp	= EOS;
	*utterance->line	= EOS;
	*utterance->attLine	= EOS;
	utterance->nextutt = utterance;
	if (isWinMode) {
		lutter = utterance;
		uttline = utterance->tuttline;
	} else {
		if (UttlineEqUtterance)
			uttline = utterance->line;
		else
			uttline = utterance->tuttline;
	}
	if (!chatmode)
		defheadtier = NULL;
	else {
		defheadtier = NEW(struct tier);
		defheadtier->include = FALSE;
		defheadtier->pat_match = FALSE;
		strcpy(defheadtier->tcode,"@");
		if (chatmode != 2 && chatmode != 4) {
			defheadtier->nexttier = NEW(struct tier);
			strcpy(defheadtier->nexttier->tcode,"%");
			defheadtier->nexttier->include = FALSE;
			defheadtier->nexttier->pat_match = FALSE;
			defheadtier->nexttier->nexttier = NULL;
		} else
			defheadtier->nexttier = NULL;
	}
}

#if defined(_MAC_CODE) || defined(_WIN32)
/* global init */
void globinit(void) {
	int i;

	for (i=0; i <= NUMSPCHARS; i++) // up-arrow
		isRemoveCAChar[i] = TRUE;
	isRemoveCAChar[NOTCA_STRESS] = FALSE;
	isRemoveCAChar[NOTCA_GLOTTAL_STOP] = FALSE;
	isRemoveCAChar[NOTCA_HEBREW_GLOTTAL] = FALSE;
	isRemoveCAChar[NOTCA_CARON] = FALSE;
	tcs = FALSE;
	tch = FALSE;
	tct = FALSE;
	otcs = TRUE;
	otch = TRUE;
	otct = TRUE;
	otcdt = FALSE;
	tcode = FALSE;
	MBF = FALSE;
	IncludeAllDefaultCodes = FALSE;
	utterance = NULL;
	currentchar = 0;
	currentatt = 0;
	getc_cr_lc = '\0';
	fgets_cr_lc = '\0';
	aftwin = 0;
	rightspeaker = 1;
	mor_link.error_found = FALSE;
	mor_link.fname[0] = EOS;
	mor_link.lineno = 0L;
#if !defined(CLAN_SRV)
	f_override = FALSE;
#else
	f_override = TRUE;
#endif
	cutt_isMultiFound = FALSE;
	cutt_isCAFound = FALSE, 
	cutt_isBlobFound = FALSE;
	cutt_depWord = FALSE;
	ml_root_clause = NULL;
	if (CLAN_PROG_NUM == FREQ || CLAN_PROG_NUM == FREQMERGE || CLAN_PROG_NUM == DSS || CLAN_PROG_NUM == PHONFREQ ||
		CLAN_PROG_NUM == RELY || CLAN_PROG_NUM == CP2UTF || CLAN_PROG_NUM == LOWCASE || CLAN_PROG_NUM == ORT ||
		CLAN_PROG_NUM == MOR_P || CLAN_PROG_NUM == TRNFIX || CLAN_PROG_NUM == JOINITEMS || CLAN_PROG_NUM == SYNCODING ||
		CLAN_PROG_NUM == IMDI_P || CLAN_PROG_NUM == LAB2CHAT || CLAN_PROG_NUM == MORTABLE || CLAN_PROG_NUM == FIXLANG ||
		CLAN_PROG_NUM == SUBTITLES || CLAN_PROG_NUM == FLO || CLAN_PROG_NUM == IPSYN || CLAN_PROG_NUM == SCRIPT_P ||
		  CLAN_PROG_NUM == TIMEDUR)
		nomap = TRUE;
	else
		nomap = FALSE;
	isSpecRepeatArrow = FALSE;
	getrestline = 1;
	isExpendX = TRUE;
	skipgetline = FALSE;
	linkMain2Mor = FALSE;
	isMorWordSpec = 0;
	FirstTime = TRUE;
	delSkipedFile = NULL;
	isFileSkipped = FALSE;
	replaceFile = FALSE;
	restoreXXX = FALSE;
	restoreYYY = FALSE;
	restoreWWW = FALSE;
	onlydata = 0;
	puredata = 2;
	outputOnlyData = FALSE;
	WordMode = '\0';
	ScopMode = '\0';
	ScopNWds = 1;
	PostCodeMode = '\0';
	lineno = 0L;
	tlineno = 1L;
	deflineno = 0L;
	stout = TRUE;
	stin  = FALSE;
	targs = NULL;
}
#endif /* defined(_MAC_CODE) || defined(_WIN32)  */

IEWORDS *freeIEWORDS(IEWORDS *ptr) {
	IEWORDS *t;

	while (ptr != NULL) {
		t = ptr;
		ptr = ptr->nextword;
		if (t->word)
			free(t->word);
		free(t);
	}
	return(ptr);
}

IEMWORDS *freeIEMWORDS(IEMWORDS *ptr) {
	int i;
	IEMWORDS *t;

	while (ptr != NULL) {
		t = ptr;
		ptr = ptr->nextword;
		for (i=0; i < t->total; i++) {
			if (t->word_arr[i])
				free(t->word_arr[i]);
		}
		free(t);
	}
	return(ptr);
}

void clean_s_option(void) {
	int num;

	if (ScopWdPtr[0] != NULL)
		free(ScopWdPtr[0]);
	ScopWdPtr[0] = NULL;
	for (num=1; num < ScopNWds; num++)
		free(ScopWdPtr[num]);
	morfptr = freeMorWords(morfptr);
	langptr = freeLangWords(langptr);
	wdptr = freeIEWORDS(wdptr);
	mwdptr = freeIEMWORDS(mwdptr);
	defwdptr = freeIEWORDS(defwdptr);
	CAptr = freeIEWORDS(CAptr);
	ScopNWds = 1;
	pauseFound = FALSE;
	PostCodeMode = '\0';
	ScopMode = '\0';
	WordMode = '\0';
}

void main_cleanup(void) {
	struct tier *tt;
	struct IDtype *tID;
	UTTER *ttt;

	clean_s_option();
	wdUttLen = freeUTTLENWORDS(wdUttLen);
	if (targs)
		free(targs);
	if (rootmorf) {
		free(rootmorf);
		rootmorf = NULL;
	}
	ml_free_clause();
	if (Toldsp != NULL)
		free(Toldsp);
	if (TSoldsp != NULL)
		free(TSoldsp);
	while (defheadtier != NULL) {
		tt = defheadtier;
		defheadtier = defheadtier->nexttier;
		free(tt);
	}
	while (headtier != NULL) {
		tt = headtier;
		headtier = headtier->nexttier;
		free(tt);
	}
	while (IDField != NULL) {
		tID = IDField;
		IDField = IDField->next_ID;
		free(tID);
	}
	while (CODEField != NULL) {
		tID = CODEField;
		CODEField = CODEField->next_ID;
		free(tID);
	}
	while (SPRole != NULL) {
		tID = SPRole;
		SPRole = SPRole->next_ID;
		free(tID);
	}
	while (HeadOutTier != NULL) {
		tt = HeadOutTier;
		HeadOutTier = HeadOutTier->nexttier;
		free(tt);
	}
	if (utterance != NULL) {
		ttt = utterance->nextutt;
		utterance->nextutt = NULL;
		utterance = ttt;
		while (utterance != NULL) {
			ttt = utterance;
			utterance = utterance->nextutt;
			free(ttt);
		}
	}
}

void CleanUpTempIDSpeakers(void) {
	struct tier *t, *tt;

	t = headtier;
	tt = t;
	while (t != NULL) {
		if (t->used == '\002') {
			if (t == headtier) {
				headtier = t->nexttier;
				tt = headtier;
				free(t);
				t = headtier;
			} else {
				tt->nexttier = t->nexttier;
				free(t);
				t = tt->nexttier;
			}
		} else {
			tt = t;
			t = t->nexttier;
		}
	}
}

void cutt_exit(int i) {
	if (i != 1 && i != -1) {
		if (fpin != NULL && fpin != stdin) {
			fclose(fpin);
			fpin = NULL;
		}
		main_cleanup();
	}
	if (fpout != NULL && fpout != stderr && fpout != stdout) {
		if (i != -1) {
			fclose(fpout);
/*			unlink(newfname); */
			fpout = NULL;
		}
		if (CLAN_PROG_NUM != RELY)
			fprintf(stderr,"\nCURRENT OUTPUT FILE \"%s\" IS INCOMPLETE.\n",newfname);
	}
	exit(i);
}

static void AddToCode(const char *ID, char inc) {
	int j;
	struct IDtype *tID;

	if (CODEField == NULL) {
		tID = NEW(struct IDtype);
		if (tID == NULL)
			out_of_mem();
		CODEField = tID;
	} else {
		for (tID=CODEField; tID->next_ID != NULL; tID=tID->next_ID) {
//			if (tID->inc != inc) {
//			}
		}
//		if (tID->inc != inc) {
//		}
		tID->next_ID = NEW(struct IDtype);
		if (tID->next_ID == NULL)
			out_of_mem();
		tID = tID->next_ID;
	}
	tID->next_ID = NULL;
	for (j=0; ID[j] == ' ' || ID[j] == '\t'; j++) ;
	strcpy(tID->ID, ID+j);
	uS.remblanks(tID->ID);
	uS.lowercasestr(tID->ID, &dFnt, FALSE);
	tID->inc = inc;
	if (inc == '+')
		tcode = TRUE;
}

static char CheckCodeNumber(char *line) {
	int  i, j;
	char t, keyword[1025];
	struct IDtype *tID;

	strcpy(templineC, line);
	uS.remblanks(templineC);
	uS.lowercasestr(templineC, &dFnt, FALSE);

	i = 0;
	while (templineC[i]) {
		for (; uS.isskip(templineC, i, &dFnt, MBF) || templineC[i] == '\n' || templineC[i] == '.'; i++) ;
		if (templineC[i] == EOS)
			break;
		for (j=i; !uS.isskip(templineC, j, &dFnt, MBF) && templineC[j] != '.' && templineC[j] != '\n' && templineC[j] != EOS; j++) ;
		t = templineC[j];
		templineC[j] = EOS;
		strncpy(keyword, templineC+i, 1024);

		for (tID=CODEField; tID != NULL; tID=tID->next_ID) {
			if (uS.patmat(keyword, tID->ID)) {
				if (i > 0)
					strcpy(templineC, templineC+i);
				return(TRUE);
			}
		}

		templineC[j] = t;
		i = j;
	}

	return(FALSE);
}

static void AddToID(const char *ID, char inc) {
	int j;
	struct IDtype *tID;

	if (IDField == NULL)
		maketierchoice("*:", '+', '\001');
	tID = NEW(struct IDtype);
	if (tID == NULL)
		out_of_mem();
	tID->next_ID = IDField;
	IDField = tID;
	for (j=0; ID[j] == ' ' || ID[j] == '\t'; j++) ;
	strcpy(tID->ID, ID+j);
	uS.remblanks(tID->ID);
	uS.lowercasestr(tID->ID, &dFnt, FALSE);
	tID->inc = inc;
}

void SetIDTier(char *line) {
	int i, j;
	struct IDtype *tID;

	strcpy(templineC2, line);
	for (j=0; templineC2[j] != '|' && templineC2[j] != EOS; j++) ;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (j++; templineC2[j] != '|' && templineC2[j] != EOS; j++) ;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	uS.remblanks(templineC2);
	uS.lowercasestr(templineC2, &dFnt, FALSE);
	for (i=0,j++; templineC2[j] != '|' && templineC2[j] != EOS; j++, i++) {
		templineC3[i] = templineC2[j];
	}
	templineC3[i] = EOS;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (tID=IDField; tID != NULL; tID=tID->next_ID) {
		if (uS.patmat(templineC2, tID->ID)) {
			for (j=0; isSpace(templineC3[j]); j++) ;
			maketierchoice(templineC3+j, tID->inc, '\002');
		}
	}
}

char CheckIDNumber(char *line, char *copyST) {
	struct IDtype *tID;

	strcpy(copyST, line);
	uS.remblanks(copyST);
	uS.lowercasestr(copyST, &dFnt, FALSE);
	for (tID=IDField; tID != NULL; tID=tID->next_ID) {
		if (uS.patmat(copyST, tID->ID)) {
			strcpy(copyST, line);
			uS.remblanks(copyST);
			return(TRUE);
		}
	}
	return(FALSE);
}

static void AddToSPRole(const char *ID, char inc) {
	int j;
	struct IDtype *tID;

	tID = NEW(struct IDtype);
	if (tID == NULL)
		out_of_mem();
	tID->next_ID = SPRole;
	SPRole = tID;
	for (j=0; ID[j] == ' ' || ID[j] == '\t'; j++) ;
	strcpy(tID->ID, ID+j);
	uS.remblanks(tID->ID);
	uS.uppercasestr(tID->ID, &dFnt, FALSE);
	tID->inc = inc;
	if (inc == '+')
		tcs = TRUE;
}

void SetSPRoleIDs(char *line) {
	int i, j;
	char sp[SPEAKERLEN];
	struct IDtype *tID;

	strcpy(templineC2, line);
	for (j=0; templineC2[j] != '|' && templineC2[j] != EOS; j++) ;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (j++; templineC2[j] != '|' && templineC2[j] != EOS; j++) ;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (i=0,j++; templineC2[j] != '|' && templineC2[j] != EOS; j++, i++) {
		sp[i] = templineC2[j];
	}
	sp[i] = EOS;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (j++; templineC2[j] != '|' && templineC2[j] != EOS; j++) ;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (j++; templineC2[j] != '|' && templineC2[j] != EOS; j++) ;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (j++; templineC2[j] != '|' && templineC2[j] != EOS; j++) ;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (j++; templineC2[j] != '|' && templineC2[j] != EOS; j++) ;
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (i=0,j++; templineC2[j] != '|' && templineC2[j] != EOS; j++, i++) {
		templineC[i] = templineC2[j];
	}
	templineC[i] = EOS;
	uS.remblanks(templineC);
	uS.uppercasestr(templineC, &dFnt, C_MBF);
	if (templineC2[j] == EOS) {
		uS.remblanks(templineC2);
		fprintf(stderr,"Illegal @ID field format: %s. In file %s\n", templineC2, oldfname);
		cutt_exit(1);
	}
	for (tID=SPRole; tID != NULL; tID=tID->next_ID) {
		if (uS.patmat(templineC, tID->ID))
			maketierchoice(sp, tID->inc, '\002');
	}
}

void SetSPRoleParticipants(char *line) {
	int b, e, t;
	char sp[SPEAKERLEN];
	struct IDtype *tID;

	e = strlen(line) - 1;
	while (e >= 0) {
		for (; (isSpace(line[e]) || line[e] == '\n' || line[e] == ',') && e >= 0; e--) ;
		if (e < 0)
			break;
		for (b=e; !isSpace(line[b]) && line[b] != '\n' && line[b] != ',' && b >= 0; b--) ;
		strncpy(templineC, line+b+1, e-b);
		templineC[e-b] = EOS;
		uS.remblanks(templineC);
		uS.uppercasestr(templineC, &dFnt, C_MBF);
		for (; (isSpace(line[b]) || line[b] == '\n') && b >= 0; b--) ;
		if (line[b] == ',' || b < 0) {
			fprintf(stderr,"Corrupted \"@Participants\" tier in file %s\n", oldfname);
			cutt_exit(1);
		}
		e = b;
		for (; !isSpace(line[b]) && line[b] != '\n' && line[b] != ',' && b >= 0; b--) ;
		for (t=b; (isSpace(line[t]) || line[t] == '\n') && t >= 0; t--) ;
		if (line[t] == ',' || t < 0) {
			strncpy(sp, line+b+1, e-b);
			sp[e-b] = EOS;
			uS.remblanks(sp);
			uS.uppercasestr(sp, &dFnt, FALSE);
			e = t - 1;
		} else {
			e = t;
			b = t;
			for (; !isSpace(line[b]) && line[b] != '\n' && line[b] != ',' && b >= 0; b--) ;
			for (t=b; (isSpace(line[t]) || line[t] == '\n') && t >= 0; t--) ;
			if (line[t] != ',' && t >= 0) {
				fprintf(stderr,"Corrupted \"@Participants\" tier in file %s\n", oldfname);
				cutt_exit(1);
			}
			strncpy(sp, line+b+1, e-b);
			sp[e-b] = EOS;
			uS.remblanks(sp);
			uS.uppercasestr(sp, &dFnt, FALSE);
			e = t - 1;
		}
		for (tID=SPRole; tID != NULL; tID=tID->next_ID) {
			if (uS.patmat(templineC, tID->ID))
				maketierchoice(sp, tID->inc, '\002');
		}
	}
}

void InitOptions(void) {
	register int i;

	if (options_already_inited)
		return;
	options_already_inited = 1;
	for (i=0; i < LAST_CLAN_PROG; i++) {
		options_ext[i][0] = EOS;
		option_flags[i] = F_OPTION + T_OPTION;
	}
	option_flags[CHAINS]	= F_OPTION+RE_OPTION+K_OPTION+P_OPTION+SP_OPTION+SM_OPTION+T_OPTION+UP_OPTION+Z_OPTION+D_OPTION+L_OPTION;
	strncpy(options_ext[CHAINS], ".chain", OPTIONS_EXT_LEN); options_ext[CHAINS][OPTIONS_EXT_LEN] = EOS;
	option_flags[CHAT2CA] = F_OPTION+RE_OPTION+L_OPTION;
	strncpy(options_ext[CHAT2CA], ".c2ca", OPTIONS_EXT_LEN); options_ext[CHAT2CA][OPTIONS_EXT_LEN] = EOS;
	option_flags[CHIP] = F_OPTION+RE_OPTION+D_OPTION+K_OPTION+P_OPTION+R_OPTION+T_OPTION+UP_OPTION+UM_OPTION+L_OPTION;
	strncpy(options_ext[CHIP], ".chip", OPTIONS_EXT_LEN); options_ext[CHIP][OPTIONS_EXT_LEN] = EOS;
	option_flags[COMBO] = F_OPTION+RE_OPTION+K_OPTION+P_OPTION+R_OPTION+T_OPTION+UP_OPTION+D_OPTION+W_OPTION+Y_OPTION+Z_OPTION+O_OPTION+L_OPTION;
	strncpy(options_ext[COMBO], ".combo", OPTIONS_EXT_LEN); options_ext[COMBO][OPTIONS_EXT_LEN] = EOS;
	option_flags[COMPOUND] = 0L;
	strncpy(options_ext[COMPOUND], ".cmpnd", OPTIONS_EXT_LEN); options_ext[COMPOUND][OPTIONS_EXT_LEN] = EOS;
	option_flags[COOCCUR] = ALL_OPTIONS+D_OPTION+Y_OPTION;
	strncpy(options_ext[COOCCUR], ".coocr", OPTIONS_EXT_LEN); options_ext[COOCCUR][OPTIONS_EXT_LEN] = EOS;
	option_flags[DATES] = FR_OPTION;
	strncpy(options_ext[DATES], ".date", OPTIONS_EXT_LEN); options_ext[DATES][OPTIONS_EXT_LEN] = EOS;
	option_flags[DIST] = ALL_OPTIONS+UM_OPTION+D_OPTION;
	strncpy(options_ext[DIST], ".dist", OPTIONS_EXT_LEN); options_ext[DIST][OPTIONS_EXT_LEN] = EOS;
	option_flags[DSS] = F_OPTION+RE_OPTION+SP_OPTION+SM_OPTION+UP_OPTION+UM_OPTION;
	strncpy(options_ext[DSS], ".tbl", OPTIONS_EXT_LEN); // This has to be ".tbl" look at dss.cpp program!!!
		options_ext[DSS][OPTIONS_EXT_LEN] = EOS;
	option_flags[EVAL] = F_OPTION+RE_OPTION+R_OPTION+T_OPTION+UP_OPTION;
	strncpy(options_ext[EVAL], ".eval", OPTIONS_EXT_LEN); options_ext[EVAL][OPTIONS_EXT_LEN] = EOS;
	option_flags[FREQ] = ALL_OPTIONS+UM_OPTION+D_OPTION+Y_OPTION;
	strncpy(options_ext[FREQ], ".frq", OPTIONS_EXT_LEN); options_ext[FREQ][OPTIONS_EXT_LEN] = EOS;
	option_flags[FREQMERGE] = F_OPTION+RE_OPTION+K_OPTION+SP_OPTION+SM_OPTION+P_OPTION+L_OPTION;
	strncpy(options_ext[FREQMERGE], ".frqmrg", OPTIONS_EXT_LEN); options_ext[FREQMERGE][OPTIONS_EXT_LEN] = EOS;
	option_flags[FREQPOS] = F_OPTION+RE_OPTION+K_OPTION+P_OPTION+R_OPTION+SM_OPTION+T_OPTION+UP_OPTION+UM_OPTION+Z_OPTION+D_OPTION+Y_OPTION+L_OPTION;
	strncpy(options_ext[FREQPOS], ".frqpos", OPTIONS_EXT_LEN); options_ext[FREQPOS][OPTIONS_EXT_LEN] = EOS;
	option_flags[GEM] = ALL_OPTIONS+UM_OPTION+D_OPTION;
	strncpy(options_ext[GEM], ".gem", OPTIONS_EXT_LEN); options_ext[GEM][OPTIONS_EXT_LEN] = EOS;
	option_flags[GEMFREQ] = ALL_OPTIONS+UM_OPTION+D_OPTION;
	strncpy(options_ext[GEMFREQ], ".gemfrq", OPTIONS_EXT_LEN); options_ext[GEMFREQ][OPTIONS_EXT_LEN] = EOS;
	option_flags[GEMLIST] = F_OPTION+RE_OPTION+T_OPTION+D_OPTION+L_OPTION;
	strncpy(options_ext[GEMLIST], ".gemlst", OPTIONS_EXT_LEN); options_ext[GEMLIST][OPTIONS_EXT_LEN] = EOS;
	option_flags[IPSYN] = F_OPTION+RE_OPTION+FR_OPTION+T_OPTION+L_OPTION+UP_OPTION;
	strncpy(options_ext[IPSYN], ".ipsyn", OPTIONS_EXT_LEN); options_ext[IPSYN][OPTIONS_EXT_LEN] = EOS;
	option_flags[KEYMAP] = ALL_OPTIONS+UM_OPTION;
	strncpy(options_ext[KEYMAP], ".keymap", OPTIONS_EXT_LEN); options_ext[KEYMAP][OPTIONS_EXT_LEN] = EOS;
	option_flags[KIDEVAL] = F_OPTION+RE_OPTION+T_OPTION+UP_OPTION;
	strncpy(options_ext[KIDEVAL], ".kideval", OPTIONS_EXT_LEN); options_ext[KIDEVAL][OPTIONS_EXT_LEN] = EOS;
	option_flags[KWAL] = ALL_OPTIONS+D_OPTION+W_OPTION+Y_OPTION+O_OPTION+FR_OPTION;
	strncpy(options_ext[KWAL], ".kwal", OPTIONS_EXT_LEN); options_ext[KWAL][OPTIONS_EXT_LEN] = EOS;
	option_flags[MAXWD] = ALL_OPTIONS+UM_OPTION+D_OPTION+Y_OPTION+O_OPTION;
	strncpy(options_ext[MAXWD], ".mxwrd", OPTIONS_EXT_LEN); options_ext[MAXWD][OPTIONS_EXT_LEN] = EOS;
	option_flags[MEGRASP] = RE_OPTION+FR_OPTION;
	strncpy(options_ext[MEGRASP], ".mgrasp", OPTIONS_EXT_LEN); options_ext[MEGRASP][OPTIONS_EXT_LEN] = EOS;
	option_flags[MLT] = ALL_OPTIONS+UM_OPTION+D_OPTION;
	strncpy(options_ext[MLT], ".mlt", OPTIONS_EXT_LEN); options_ext[MLT][OPTIONS_EXT_LEN] = EOS;
	option_flags[MLU] = ALL_OPTIONS+UM_OPTION+D_OPTION+Y_OPTION;
	strncpy(options_ext[MLU], ".mlu", OPTIONS_EXT_LEN); options_ext[MLU][OPTIONS_EXT_LEN] = EOS;
	option_flags[MODREP] = F_OPTION+RE_OPTION+K_OPTION+P_OPTION+R_OPTION+T_OPTION+UP_OPTION+UM_OPTION+Z_OPTION+L_OPTION;
	strncpy(options_ext[MODREP], ".mdrep", OPTIONS_EXT_LEN); options_ext[MODREP][OPTIONS_EXT_LEN] = EOS;
	option_flags[MOR_P] = F_OPTION+RE_OPTION+T_OPTION+SP_OPTION+SM_OPTION+FR_OPTION;
	strncpy(options_ext[MOR_P],	".mor", OPTIONS_EXT_LEN); options_ext[MOR_P][OPTIONS_EXT_LEN] = EOS;
	option_flags[MORTABLE] = F_OPTION+RE_OPTION+T_OPTION+FR_OPTION+UP_OPTION;
	strncpy(options_ext[MORTABLE], ".mrtbl", OPTIONS_EXT_LEN); options_ext[MORTABLE][OPTIONS_EXT_LEN] = EOS;
	option_flags[PHONFREQ] = ALL_OPTIONS+UM_OPTION+D_OPTION;
	strncpy(options_ext[PHONFREQ], ".phofrq", OPTIONS_EXT_LEN); options_ext[PHONFREQ][OPTIONS_EXT_LEN] = EOS;
	option_flags[POST] = F_OPTION+RE_OPTION+SP_OPTION+SM_OPTION+T_OPTION+FR_OPTION;
	strncpy(options_ext[POST], ".pst", OPTIONS_EXT_LEN); options_ext[POST][OPTIONS_EXT_LEN] = EOS; // it is set in post program, ignored
	option_flags[POSTLIST] = 0L;
	strncpy(options_ext[POSTLIST], ".poslst", OPTIONS_EXT_LEN); options_ext[POSTLIST][OPTIONS_EXT_LEN] = EOS;
	option_flags[POSTMODRULES] = 0L;
	strncpy(options_ext[POSTMODRULES], ".pmr", OPTIONS_EXT_LEN); options_ext[POSTMODRULES][OPTIONS_EXT_LEN] = EOS;
	option_flags[POSTMORTEM]= FR_OPTION+RE_OPTION+T_OPTION;
	strncpy(options_ext[POSTMORTEM], ".pmortm", OPTIONS_EXT_LEN); options_ext[POSTMORTEM][OPTIONS_EXT_LEN] = EOS;
	option_flags[POSTTRAIN] = RE_OPTION+SP_OPTION+SM_OPTION+T_OPTION;
	strncpy(options_ext[POSTTRAIN], ".ptrain", OPTIONS_EXT_LEN); options_ext[POSTTRAIN][OPTIONS_EXT_LEN] = EOS;
	option_flags[RELY] = F_OPTION+K_OPTION+P_OPTION+RE_OPTION+SP_OPTION+T_OPTION+L_OPTION;
	strncpy(options_ext[RELY], ".rely", OPTIONS_EXT_LEN); options_ext[RELY][OPTIONS_EXT_LEN] = EOS;
	option_flags[SCRIPT_P] = F_OPTION+RE_OPTION+R_OPTION+T_OPTION+UP_OPTION;
	strncpy(options_ext[SCRIPT_P], ".script", OPTIONS_EXT_LEN); options_ext[SCRIPT_P][OPTIONS_EXT_LEN] = EOS;
	option_flags[TIMEDUR] = F_OPTION+RE_OPTION+K_OPTION+P_OPTION+R_OPTION+SP_OPTION+SM_OPTION+T_OPTION+UP_OPTION+Z_OPTION;
	strncpy(options_ext[TIMEDUR], ".timdur", OPTIONS_EXT_LEN); options_ext[TIMEDUR][OPTIONS_EXT_LEN] = EOS;
	option_flags[VOCD] = F_OPTION+K_OPTION+P_OPTION+R_OPTION+RE_OPTION+SP_OPTION+SM_OPTION+T_OPTION+UP_OPTION;
	strncpy(options_ext[VOCD], ".vocd", OPTIONS_EXT_LEN); options_ext[VOCD][OPTIONS_EXT_LEN] = EOS;
	option_flags[WDLEN] = ALL_OPTIONS+D_OPTION+UM_OPTION+Y_OPTION;
	strncpy(options_ext[WDLEN], ".wdlen", OPTIONS_EXT_LEN); options_ext[WDLEN][OPTIONS_EXT_LEN] = EOS;


	option_flags[CHSTRING] = F_OPTION+RE_OPTION+P_OPTION+T_OPTION+Y_OPTION+FR_OPTION;
	strncpy(options_ext[CHSTRING], ".chstr", OPTIONS_EXT_LEN); options_ext[CHSTRING][OPTIONS_EXT_LEN] = EOS;
	option_flags[ANVIL2CHAT]= F_OPTION+RE_OPTION+FR_OPTION;
	strncpy(options_ext[ANVIL2CHAT], ".anvil", OPTIONS_EXT_LEN); options_ext[ANVIL2CHAT][OPTIONS_EXT_LEN] = EOS;
	option_flags[CHAT2ANVIL]= F_OPTION+RE_OPTION+P_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[CHAT2ANVIL], ".c2anvl", OPTIONS_EXT_LEN); options_ext[CHAT2ANVIL][OPTIONS_EXT_LEN] = EOS;
	option_flags[CHAT2ELAN] = F_OPTION+RE_OPTION+P_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[CHAT2ELAN], ".c2elan", OPTIONS_EXT_LEN); options_ext[CHAT2ELAN][OPTIONS_EXT_LEN] = EOS;
	option_flags[CHAT2PRAAT]= F_OPTION+RE_OPTION+P_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[CHAT2PRAAT], ".c2praat", OPTIONS_EXT_LEN); options_ext[CHAT2PRAAT][OPTIONS_EXT_LEN] = EOS;
	option_flags[CHAT2XMAR] = F_OPTION+RE_OPTION+P_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[CHAT2XMAR], ".c2xmar", OPTIONS_EXT_LEN); options_ext[CHAT2XMAR][OPTIONS_EXT_LEN] = EOS;
	option_flags[CHECK] = F_OPTION+RE_OPTION+D_OPTION+P_OPTION+T_OPTION+L_OPTION;
	strncpy(options_ext[CHECK], ".chck", OPTIONS_EXT_LEN); options_ext[CHECK][OPTIONS_EXT_LEN] = EOS;
	option_flags[COMBTIER] = F_OPTION+RE_OPTION+K_OPTION+P_OPTION+L_OPTION;
	strncpy(options_ext[COMBTIER], ".cmbtr", OPTIONS_EXT_LEN); options_ext[COMBTIER][OPTIONS_EXT_LEN] = EOS;
	option_flags[CP2UTF] = F_OPTION+RE_OPTION+FR_OPTION+T_OPTION+L_OPTION+Y_OPTION;
	strncpy(options_ext[CP2UTF], ".cp2utf", OPTIONS_EXT_LEN); options_ext[CP2UTF][OPTIONS_EXT_LEN] = EOS;
	option_flags[DATACLEANUP]= F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+L_OPTION;
	strncpy(options_ext[DATACLEANUP], ".dtcl", OPTIONS_EXT_LEN); options_ext[DATACLEANUP][OPTIONS_EXT_LEN] = EOS;
	option_flags[DELIM] = F_OPTION+RE_OPTION+FR_OPTION+K_OPTION+P_OPTION+T_OPTION+L_OPTION;
	strncpy(options_ext[DELIM], ".delim", OPTIONS_EXT_LEN); options_ext[DELIM][OPTIONS_EXT_LEN] = EOS;
	option_flags[ELAN2CHAT] = F_OPTION+RE_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[ELAN2CHAT], ".elan", OPTIONS_EXT_LEN); options_ext[ELAN2CHAT][OPTIONS_EXT_LEN] = EOS;
	option_flags[FIXBULLETS]= F_OPTION+RE_OPTION+T_OPTION+FR_OPTION+L_OPTION+Y_OPTION;
	strncpy(options_ext[FIXBULLETS], ".fxblts", OPTIONS_EXT_LEN); options_ext[FIXBULLETS][OPTIONS_EXT_LEN] = EOS;
	option_flags[FIXIT] = F_OPTION+RE_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[FIXIT], ".fixit", OPTIONS_EXT_LEN); options_ext[FIXIT][OPTIONS_EXT_LEN] = EOS;
	option_flags[FIXLANG] = FR_OPTION+RE_OPTION;
	strncpy(options_ext[FIXLANG], ".fixlan", OPTIONS_EXT_LEN); options_ext[FIXLANG][OPTIONS_EXT_LEN] = EOS;
	option_flags[FIXMP3] = P_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[FIXMP3], ".fixmp3", OPTIONS_EXT_LEN); options_ext[FIXMP3][OPTIONS_EXT_LEN] = EOS;
	option_flags[FLO] = F_OPTION+RE_OPTION+D_OPTION+SP_OPTION+SM_OPTION+T_OPTION+FR_OPTION+L_OPTION+R_OPTION+Z_OPTION;
	strncpy(options_ext[FLO], ".flo", OPTIONS_EXT_LEN); options_ext[FLO][OPTIONS_EXT_LEN] = EOS;
	option_flags[IMDI_P] = RE_OPTION;
	strncpy(options_ext[IMDI_P], ".imdi", OPTIONS_EXT_LEN); options_ext[IMDI_P][OPTIONS_EXT_LEN] = EOS;
	option_flags[INDENT] = F_OPTION+RE_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[INDENT], ".indnt", OPTIONS_EXT_LEN); options_ext[INDENT][OPTIONS_EXT_LEN] = EOS;
	option_flags[INSERT] = F_OPTION+RE_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[INSERT], ".insert", OPTIONS_EXT_LEN); options_ext[INSERT][OPTIONS_EXT_LEN] = EOS;
	option_flags[JOINITEMS] = 0;
	strncpy(options_ext[JOINITEMS], ".jnt", OPTIONS_EXT_LEN); options_ext[JOINITEMS][OPTIONS_EXT_LEN] = EOS;
	option_flags[LAB2CHAT] = RE_OPTION;
	strncpy(options_ext[LAB2CHAT], ".lab", OPTIONS_EXT_LEN); options_ext[LAB2CHAT][OPTIONS_EXT_LEN] = EOS;
	option_flags[LIPP2CHAT] = F_OPTION+RE_OPTION+FR_OPTION;
	strncpy(options_ext[LIPP2CHAT], ".lipp", OPTIONS_EXT_LEN); options_ext[LIPP2CHAT][OPTIONS_EXT_LEN] = EOS;
	option_flags[LONGTIER] = F_OPTION+RE_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[LONGTIER], ".longtr", OPTIONS_EXT_LEN); options_ext[LONGTIER][OPTIONS_EXT_LEN] = EOS;
	option_flags[LOWCASE] = F_OPTION+RE_OPTION+FR_OPTION+P_OPTION+T_OPTION+L_OPTION;
	strncpy(options_ext[LOWCASE], ".lowcas", OPTIONS_EXT_LEN); options_ext[LOWCASE][OPTIONS_EXT_LEN] = EOS;
	option_flags[MAKEMOD] = F_OPTION+RE_OPTION+T_OPTION+L_OPTION;
	strncpy(options_ext[MAKEMOD], ".mkmod", OPTIONS_EXT_LEN); options_ext[MAKEMOD][OPTIONS_EXT_LEN] = EOS;
	option_flags[OLAC_P] = RE_OPTION;
	strncpy(options_ext[OLAC_P], ".xml", OPTIONS_EXT_LEN); options_ext[OLAC_P][OPTIONS_EXT_LEN] = EOS;
	option_flags[ORT] = 0L;
	strncpy(options_ext[ORT], ".ort", OPTIONS_EXT_LEN); options_ext[ORT][OPTIONS_EXT_LEN] = EOS;
	option_flags[PRAAT2CHAT]= F_OPTION+RE_OPTION+FR_OPTION;
	strncpy(options_ext[PRAAT2CHAT], ".praat", OPTIONS_EXT_LEN); options_ext[PRAAT2CHAT][OPTIONS_EXT_LEN] = EOS;
	option_flags[QUOTES] = F_OPTION+RE_OPTION+FR_OPTION+UP_OPTION+L_OPTION;
	strncpy(options_ext[QUOTES], ".qotes", OPTIONS_EXT_LEN); options_ext[QUOTES][OPTIONS_EXT_LEN] = EOS;
	option_flags[REPEAT]	= F_OPTION+RE_OPTION+P_OPTION+L_OPTION;
	strncpy(options_ext[REPEAT], ".rpeat", OPTIONS_EXT_LEN); options_ext[REPEAT][OPTIONS_EXT_LEN] = EOS;
	option_flags[RETRACE] = F_OPTION+RE_OPTION+K_OPTION+P_OPTION+L_OPTION;
	strncpy(options_ext[RETRACE], ".retrace", OPTIONS_EXT_LEN); options_ext[RETRACE][OPTIONS_EXT_LEN] = EOS;
	option_flags[RTFIN] = F_OPTION+RE_OPTION+K_OPTION+P_OPTION+L_OPTION;
	strncpy(options_ext[RTFIN], ".rtf", OPTIONS_EXT_LEN); options_ext[RTFIN][OPTIONS_EXT_LEN] = EOS;
	option_flags[SALTIN] = F_OPTION+RE_OPTION+K_OPTION+L_OPTION;
	strncpy(options_ext[SALTIN], ".sltin", OPTIONS_EXT_LEN); options_ext[SALTIN][OPTIONS_EXT_LEN] = EOS;
	option_flags[SILENCE_P] = RE_OPTION+K_OPTION+R_OPTION+SP_OPTION+SM_OPTION+T_OPTION;
	strncpy(options_ext[SILENCE_P], ".aif", OPTIONS_EXT_LEN); options_ext[SILENCE_P][OPTIONS_EXT_LEN] = EOS;
	option_flags[SPREADSH] = FR_OPTION+RE_OPTION;
	strncpy(options_ext[SPREADSH], ".spread", OPTIONS_EXT_LEN); options_ext[SPREADSH][OPTIONS_EXT_LEN] = EOS;
	option_flags[SUBTITLES] = RE_OPTION;
	strncpy(options_ext[SUBTITLES], ".subtl", OPTIONS_EXT_LEN); options_ext[SUBTITLES][OPTIONS_EXT_LEN] = EOS;
	option_flags[SYNCODING] = F_OPTION+RE_OPTION+FR_OPTION;
	strncpy(options_ext[SYNCODING], ".sync", OPTIONS_EXT_LEN); options_ext[SYNCODING][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEXTIN] = F_OPTION+RE_OPTION;
	strncpy(options_ext[TEXTIN], ".txtin", OPTIONS_EXT_LEN); options_ext[TEXTIN][OPTIONS_EXT_LEN] = EOS;
	option_flags[TIERORDER] = F_OPTION+RE_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TIERORDER], ".tordr", OPTIONS_EXT_LEN); options_ext[TIERORDER][OPTIONS_EXT_LEN] = EOS;
	option_flags[TRNFIX] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TRNFIX], ".trnfx", OPTIONS_EXT_LEN); options_ext[TRNFIX][OPTIONS_EXT_LEN] = EOS;
	option_flags[UNIQ] = F_OPTION+RE_OPTION+D_OPTION+K_OPTION+SP_OPTION+SM_OPTION+UP_OPTION+UM_OPTION+L_OPTION+Y_OPTION;
	strncpy(options_ext[UNIQ], ".uniq", OPTIONS_EXT_LEN); options_ext[UNIQ][OPTIONS_EXT_LEN] = EOS;


	option_flags[TEMP01] = RE_OPTION;
	strncpy(options_ext[TEMP01], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP01][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP02] = RE_OPTION+FR_OPTION;
	strncpy(options_ext[TEMP02], ".connl", OPTIONS_EXT_LEN); options_ext[TEMP02][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP03] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TEMP03], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP03][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP04] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TEMP04], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP04][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP05] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TEMP05], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP05][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP06] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TEMP06], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP06][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP07] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TEMP07], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP07][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP08] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TEMP08], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP08][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP09] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TEMP09], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP09][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP10] = F_OPTION+RE_OPTION+T_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[TEMP10], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP10][OPTIONS_EXT_LEN] = EOS;
	option_flags[TEMP] = F_OPTION+FR_OPTION;
	strncpy(options_ext[TEMP], ".tmp", OPTIONS_EXT_LEN); options_ext[TEMP][OPTIONS_EXT_LEN] = EOS;


	option_flags[DOS2UNIX]  = F_OPTION+RE_OPTION+P_OPTION+Y_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[DOS2UNIX], ".dos2unx", OPTIONS_EXT_LEN); options_ext[DOS2UNIX][OPTIONS_EXT_LEN] = EOS;
	option_flags[FIXCA] = F_OPTION+RE_OPTION+L_OPTION;
	strncpy(options_ext[FIXCA], ".fixca", OPTIONS_EXT_LEN); options_ext[FIXCA][OPTIONS_EXT_LEN] = EOS;
	option_flags[FIXOVLPSYM]= F_OPTION+RE_OPTION+FR_OPTION+L_OPTION;
	strncpy(options_ext[FIXOVLPSYM], ".fxovl", OPTIONS_EXT_LEN); options_ext[FIXOVLPSYM][OPTIONS_EXT_LEN] = EOS;
	option_flags[GPS] = RE_OPTION+FR_OPTION;
	strncpy(options_ext[GPS], ".gps", OPTIONS_EXT_LEN); options_ext[GPS][OPTIONS_EXT_LEN] = EOS;
	option_flags[LINES_P] = F_OPTION+RE_OPTION+Y_OPTION+L_OPTION;
	strncpy(options_ext[LINES_P], ".line", OPTIONS_EXT_LEN); options_ext[LINES_P][OPTIONS_EXT_LEN] = EOS;
	option_flags[PP] = 0L;
	strncpy(options_ext[PP], ".pp", OPTIONS_EXT_LEN); options_ext[PP][OPTIONS_EXT_LEN] = EOS;
}

void maininitwords() {
	addword('\0','\0',"+0*");
	addword('\0','\0',"+&*");
	addword('\0','\0',"++*");
	addword('\0','\0',"+-*");
	addword('\0','\0',"+#*");
	addword('\0','\0',"+(*.*)");
}

void mor_initwords() {
	addword('\0','\0',"+end|*");
	addword('\0','\0',"+beg|*");
	addword('\0','\0',"+cm|*");
	addword('\0','\0',"+bq|*");
	addword('\0','\0',"+eq|*");
}

/* getrange(f) get the range specified by the user using +z option.
*/
static char getrange(const char *f) {
	const char *t;

	for (t=f; *t; t++) {
		if (*t != 'u' && *t != 'w' && *t != 't' && *t != 'U' && *t != 'W' && *t != 'T' && *t != '-' && !isdigit(*t) && *t) {
			fprintf(stderr,"Illegal character '%c' found in +z argument\n", *t);
			return(FALSE);
		}
	}
	CntWUT = 0;
	FromWU = 0;
	ToWU = 0;
	for (; *f && *f != 'u' && *f != 'w' && *f != 't' && *f != 'U' && *f != 'W' && *f != 'T' && !isdigit(*f); f++) ;
	if (*f == 'w' || *f == 'W') {
		CntWUT = 1; /* word */
		for (f++; *f && !isdigit(*f); f++) ;
	} else if (*f == 'u' || *f == 'U') {
		CntWUT = 2; /* utterance */
		for (f++; *f && !isdigit(*f); f++) ;
	} else if (*f == 't' || *f == 'T') {
		CntWUT = 3; /* turn */
		for (f++; *f && !isdigit(*f); f++) ;
	}
	if (*f) {
		ToWU = atoi(f);
		for (; isdigit(*f); f++) ;
		if (*f == '-') { FromWU = ToWU; ToWU = 0; }
		for (; *f && *f != 'u' && *f != 'w' && *f != 't' && *f != 'U' && *f != 'W' && *f != 'T' && !isdigit(*f); f++) ;
		if (*f == 't' || *f == 'T') {
			if (CntWUT == 1) {
				fprintf(stderr,"Missmatch in range definition: +zw#T#\n");
				return(FALSE);
			} else if (CntWUT == 2) {
				fprintf(stderr,"Missmatch in range definition: +zu#T#\n");
				return(FALSE);
			} else CntWUT = 3; /* turn */
			for (f++; *f && !isdigit(*f); f++)
				if (*f == '-') { FromWU = ToWU; ToWU = 0; }
		} else if (*f == 'u' || *f == 'U') {
			if (CntWUT == 1) {
				fprintf(stderr,"Missmatch in range definition: +zw#U#\n");
				return(FALSE);
			} else if (CntWUT == 3) {
				fprintf(stderr,"Missmatch in range definition: +zt#U#\n");
				return(FALSE);
			} else CntWUT = 2; /* utterance */
			for (f++; *f && !isdigit(*f); f++)
				if (*f == '-') { FromWU = ToWU; ToWU = 0; }
		} else if (*f == 'w' || *f == 'W') {
			if (CntWUT == 2) {
				fprintf(stderr,"Missmatch in range definition: +zu#W#\n");
				return(FALSE);
			} else if (CntWUT == 3) {
				fprintf(stderr,"Missmatch in range definition: +zt#W#\n");
				return(FALSE);
			} else CntWUT = 1; /* word */
			for (f++; *f && !isdigit(*f); f++)
				if (*f == '-') { FromWU = ToWU; ToWU = 0; }
		} else if (*f == '-') { FromWU = ToWU; ToWU = 0; }

		if (*f) {
			if (!FromWU) FromWU = ToWU;
			ToWU = atoi(f);
			for (; isdigit(*f); f++) ;
			for (; *f && *f != 'u' && *f != 'w' && *f != 't' && *f != 'U' && *f != 'W' && *f != 'T'; f++) ;
			if (*f == 't' || *f == 'T') {
				if (CntWUT == 1) {
					fprintf(stderr,"Missmatch in range definition: +zw#T#\n");
					return(FALSE);
				} else if (CntWUT == 2) {
					fprintf(stderr,"Missmatch in range definition: +zu#T#\n");
					return(FALSE);
				} else CntWUT = 3; /* turn */
			} else if (*f == 'u' || *f == 'U') {
				if (CntWUT == 1) {
					fprintf(stderr,"Missmatch in range definition: +zw#U#\n");
					return(FALSE);
				} else if (CntWUT == 3) {
					fprintf(stderr,"Missmatch in range definition: +zt#U#\n");
					return(FALSE);
				} else CntWUT = 2; /* utterance */
			} else if (*f == 'w' || *f == 'W') {
				if (CntWUT == 2) {
					fprintf(stderr,"Missmatch in range definition: +zu#W#\n");
					return(FALSE);
				} else if (CntWUT == 3) {
					fprintf(stderr,"Missmatch in range definition: +zt#W#\n");
					return(FALSE);
				} else CntWUT = 1; /* word */
			}
		}
		if (!CntWUT) {
			fprintf(stderr,"Please specify range type with +z option\n");
			fprintf(stderr,"\tw - words, u - utterances or t - turns.\n");
			return(FALSE);
		}
	} else {
		fprintf(stderr,"Please specify range type with +z option\n");
		fprintf(stderr,"\tw - words, u - utterances or t - turns.\n");
		return(FALSE);
	}

/*
printf("FromWU=%ld; ToWU=%ld; CntWUT=%d;\n", FromWU, ToWU, CntWUT);
*/
	if (FromWU > ToWU && ToWU) {
		fprintf(stderr, "Error: Illegal range (+z): From %ld to %ld\n",
				FromWU, ToWU);
		return(FALSE);
	}
	return(TRUE);
}

/* getxrange(f) get the value specified by the user using +x option.
*/
static char getxrange(const char *f) {
	const char *t;

	for (t=f; *t; t++) {
		if (*t != 'm' && *t != 'w' && *t != 'c' && *t != 'M' && *t != 'W' && *t != 'C' && !isdigit(*t)) {
			fprintf(stderr,"Illegal character '%c' found in +x%c option\n", *t, filterUttLen_cmp);
			fprintf(stderr,"\tw - words, c - characters or m - morphemes.\n");
			fprintf(stderr,"\tFor example: +x%c2w\n", filterUttLen_cmp);
			return(FALSE);
		}
	}
	if (*f == EOS) {
		fprintf(stderr,"Please specify count type after +x%c option along with a number\n", filterUttLen_cmp);
		fprintf(stderr,"\tw - words, c - characters or m - morphemes.\n");
		fprintf(stderr,"\tFor example: +x%c2w\n", filterUttLen_cmp);
		return(FALSE);
	}
	CntFUttLen = 0;
	filterUttLen = 0L;
	for (; *f && *f != 'm' && *f != 'w' && *f != 'c' && *f != 'M' && *f != 'W' && *f != 'C' && !isdigit(*f); f++) ;
	if (*f == 'w' || *f == 'W') {
		CntFUttLen = 1; /* word */
		for (f++; *f && !isdigit(*f); f++) ;
	} else if (*f == 'm' || *f == 'M') {
		CntFUttLen = 2; /* morphemes */
		for (f++; *f && !isdigit(*f); f++) ;
	} else if (*f == 'c' || *f == 'C') {
		CntFUttLen = 3; /* characters */
		for (f++; *f && !isdigit(*f); f++) ;
	}
	if (*f) {
		filterUttLen = atol(f);
		for (; isdigit(*f); f++) ;
		for (; *f && *f != 'm' && *f != 'w' && *f != 'c' && *f != 'M' && *f != 'W' && *f != 'C' && !isdigit(*f); f++) ;
		if (*f == 'm' || *f == 'M') {
			CntFUttLen = 2; /* morphemes */
		} else if (*f == 'c' || *f == 'C') {
			CntFUttLen = 3; /* characters */
		} else if (*f == 'w' || *f == 'W') {
			CntFUttLen = 1; /* word */
		}
		if (CntFUttLen == 0 && CLAN_PROG_NUM != MAXWD) {
			fprintf(stderr,"Please specify count type after +x%c option along with a number\n", filterUttLen_cmp);
			fprintf(stderr,"\tw - words, c - characters or m - morphemes.\n");
			fprintf(stderr,"\tFor example: +x%c2w\n", filterUttLen_cmp);
			return(FALSE);
		}
	} else if (filterUttLen == 0L) {
		fprintf(stderr,"Please specify count type after +x%c option along with a number\n", filterUttLen_cmp);
		fprintf(stderr,"\tw - words, c - characters or m - morphemes.\n");
		fprintf(stderr,"\tFor example: +x%c2w\n", filterUttLen_cmp);
		return(FALSE);
	}
	if (CLAN_PROG_NUM == MAXWD && CntFUttLen == 0 && maxwd_which != 0) {
		if (maxwd_which == 1)
			CntFUttLen = 2;
		else if (maxwd_which == 2)
			CntFUttLen = 1;
		else if (maxwd_which == 3)
			CntFUttLen = 3;
	}
	if (CntFUttLen == 2)
		maketierchoice("%mor",'+',FALSE);
	return(TRUE);
}

/* makewind(w) increases the size of the utterance loop by "w" elements to
   accomodate the data for the window of the specified size.
*/
void makewind(int w) {
	UTTER *ttt;

	for (; w > 0; w--) {
		ttt = NEW(UTTER);
		if (ttt == NULL) out_of_mem();
		ttt->speaker[0]	= EOS;
		ttt->attSp[0]	= EOS;
		ttt->line[0]	= EOS;
		ttt->attLine[0]	= EOS;
		ttt->nextutt = utterance->nextutt;
		utterance->nextutt = ttt;
	}
}

char *mainflgs() {
	*templineC = EOS;
	if (option_flags[CLAN_PROG_NUM] & D_OPTION) {
		if (OnlydataLimit == 1) strcat(templineC,"d ");
		else if (OnlydataLimit > 1) strcat(templineC,"dN ");
	}
#if !defined(CLAN_SRV)
	if (option_flags[CLAN_PROG_NUM] & F_OPTION)  strcat(templineC,"fS ");
#endif
	if (option_flags[CLAN_PROG_NUM] & K_OPTION)  strcat(templineC,"k ");
	if (option_flags[CLAN_PROG_NUM] & L_OPTION)  strcat(templineC,"l ");
	if (option_flags[CLAN_PROG_NUM] & O_OPTION)  strcat(templineC,"oS ");
	if (option_flags[CLAN_PROG_NUM] & P_OPTION)  strcat(templineC,"pS ");
	if (option_flags[CLAN_PROG_NUM] & R_OPTION)  strcat(templineC,"rN ");
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	if (option_flags[CLAN_PROG_NUM] & RE_OPTION)  strcat(templineC,"re ");
#endif // defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	if (option_flags[CLAN_PROG_NUM] & SP_OPTION || option_flags[CLAN_PROG_NUM] & SM_OPTION) strcat(templineC,"sS ");
	if (option_flags[CLAN_PROG_NUM] & T_OPTION)  strcat(templineC,"tS ");
	if (option_flags[CLAN_PROG_NUM] & UP_OPTION || option_flags[CLAN_PROG_NUM] & UM_OPTION) strcat(templineC,"u ");
	if (option_flags[CLAN_PROG_NUM] & W_OPTION)  strcat(templineC,"wN ");
	if (option_flags[CLAN_PROG_NUM] & Z_OPTION)  strcat(templineC,"xN ");
	if (option_flags[CLAN_PROG_NUM] & Y_OPTION)  strcat(templineC,"yN ");
	if (option_flags[CLAN_PROG_NUM] & Z_OPTION)  strcat(templineC,"zN ");
	if (option_flags[CLAN_PROG_NUM] & FR_OPTION)  strcat(templineC,"1  ");
	if (option_flags[CLAN_PROG_NUM] & F_OPTION)  strcat(templineC,"2  ");
	return(templineC);
}

/* mainusage(char isQuit) displays the extended options list.
*/
void mainusage(char isQuit) {
	if (option_flags[CLAN_PROG_NUM] & D_OPTION) {
		if (CLAN_PROG_NUM == CHECK) {
			puts("+d : attempts to suppress repeated warnings of the same type");
			puts("+d1: suppress ALL repeated warnings of the same type");
			puts("+d2: display ONLY error messages");
		} else if (CLAN_PROG_NUM == CHIP) {
			puts("+d : outputs only coding tiers");
			puts("+d1: outputs only summary statistics");
		} else if (CLAN_PROG_NUM == COMBO) {
		} else if (CLAN_PROG_NUM == COOCCUR) {
			puts("+d : output matches without frequency count");
			puts("+d1: output results with line numbers and file names");
		} else if (CLAN_PROG_NUM == DIST)
			puts("+d:  Output sdata in a form suitable for statistical analysis.");
		else if (CLAN_PROG_NUM == FLO)
			puts("+d:   replaces the main tier with the simplified %flo tier in the output");
		else if (CLAN_PROG_NUM == FREQ) {
			puts("+dCN:output only words used by <, <=, =, => or > than N percent of speakers");
			puts("+d : outputs all selected words, corresponding frequencies, and line numbers");
			puts("+d1: outputs word with no frequency information. in KWAL or COMBO format");
			puts("+d2: outputs in SPREADSHEET format");
			puts("+d3: output only type/token information in SPREADSHEET format");
			puts("+d4: outputs only type/token information");
			puts("+d5: outputs all words selected with +s option, including the ones with 0 frequency count");
			puts("+d6: outputs words and frequencies with limited searched word surrounding context");
			puts("+d7: outputs words and frequencies with corresponding words from speaker tier");
		} else if (CLAN_PROG_NUM == GEM) {
			puts("+d : outputs legal CHAT format");
			puts("+d1: outputs legal CHAT format plus file names, line numbers, and @ID codes");
		} else if (CLAN_PROG_NUM == GEMLIST) {
			puts("+d : only the data between @Bg and @Eg is displayed");
		} else if (CLAN_PROG_NUM == KWAL) {
			puts("+d : outputs legal CHAT format");
			puts("+d1: outputs legal CHAT format plus file names and line numbers");
			puts("+d2: outputs file names once per file only");
			puts("+d3: outputs ONLY matched items");
			puts("+d30: outputs ONLY matched items without any defaults removed");
			puts("+d4: outputs in SPREADSHEET format");
			puts("+d40: outputs in SPREADSHEET format and repeat the same tier for every keyword match");
//			puts("+d41: outputs in SPREADSHEET format with each matched keyword listed one per column");
			puts("+d99: convert \"word [x 2]\" to \"word [/] word\" and so on");
		} else if (CLAN_PROG_NUM == MAXWD) {
			puts("+d : outputs one line for the length level and the next line for the word");
			puts("+d1: produces longest words, one per line, without any other information");
		} else if (CLAN_PROG_NUM == MLT) {
			puts("+d : output in SPREADSHEET format. Must include speaker specifications");
			puts("+d1: output data ONLY.");
		} else if (CLAN_PROG_NUM == MLU) {
			puts("+d : output in SPREADSHEET format. Must include speaker specifications");
			puts("+d1: output data ONLY.");
		} else if (CLAN_PROG_NUM == PHONFREQ) {
			puts("+d : actual words matched will be written to the output");
		} else if (CLAN_PROG_NUM == VOCD) {
		} else if (CLAN_PROG_NUM == WDLEN) {
			puts("+d : output in SPREADSHEET format");
		} else if (OnlydataLimit == 1) {
			puts("+d : output only level 0 data");
		} else if (OnlydataLimit > 1) {
			puts("+dN: output only level N data");
		}
	}
#if !defined(CLAN_SRV)
	if (option_flags[CLAN_PROG_NUM] & F_OPTION) {
		puts("+fS: send output to file (program will derive filename)");
		puts("-f : send output to the screen or pipe");
	}
#endif
	if (option_flags[CLAN_PROG_NUM] & K_OPTION) {
		if (!nomap)
			puts("+k : treat upper and lower case as different");
		else
			puts("+k : treat upper and lower case as the same");
	}
	if (option_flags[CLAN_PROG_NUM] & L_OPTION)
		puts("+l : add language tag to every word");
	if (option_flags[CLAN_PROG_NUM] & O_OPTION) {
		puts("+oS: include additional tier code S for output purposes ONLY");
		puts("-oS: exclude tier code S from an additional inclusion in an output");
	}
	if (option_flags[CLAN_PROG_NUM] & P_OPTION)
		puts("+pS: add S to word delimiters. (+p_ will break New_York into two words)");
	if (option_flags[CLAN_PROG_NUM] & R_OPTION) {
		puts("+rN: if N=1 then \"get(s)\" goes to \"gets\", N=2 to \"get(s)\", N=3 to \"get\" (default is N=1)");
		puts("     4- do not break words into two at post/pre clitics '~' and '$' on %mor tier");
		if (R5)
			printf("     5- no text replacement: [: *], ");
		else
			printf("     5- perform text replacement: [: *, ]");
		if (R5_1)
			puts("   50- no text replacement: [:: *]");
		else
			puts("   50- perform text replacement: [:: *]");
		if (R6)
			puts("     6- exclude repetitions: </>, <//>, <///>, </-> and </?>,");
		else
			puts("     6- include repetitions: </>, <//>, <///>, </-> and </?>,");
		puts("     7- do not remove prosodic symbols in words '/', '~', '^' and ':'");
		puts("     8- add word spoken, error code from speaker tier to output from %mor: tier");
	}
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	if (option_flags[CLAN_PROG_NUM] & RE_OPTION)
		puts("+re: run program recursively on all sub-directories.");
#endif // defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	if (option_flags[CLAN_PROG_NUM] & SP_OPTION) {
		if (CLAN_PROG_NUM == GEM || CLAN_PROG_NUM == GEMFREQ)
			puts("+sS: select gems which are labeled by either label S or labels in file @S");
		else {
			puts("+sS: search for word S or words in file @S in an input file (+s@ or +s@s for more info).");
			puts("    \"[+ ...]\" for data on postcode tier, \"<+ ...>\" for postcode itself");
		}
	}
	if (option_flags[CLAN_PROG_NUM] & SM_OPTION) {
		if (CLAN_PROG_NUM == GEM || CLAN_PROG_NUM == GEMFREQ)
			puts("-sS: select gems which are NOT labeled by either label S or labels in file @S");
		else
			puts("-sS: exclude word S or words in file @S from an input file (-s@ or -s@s for more info)");
	}
	if (option_flags[CLAN_PROG_NUM] & T_OPTION) {
		puts("+tS: include tier code S");
		puts("-tS: exclude tier code S");
		puts("    +/-t#Target_Child - select target child's tiers");
		puts("    +/-t@id=\"*|Mother|*\" - select mother's tiers");
	}
	if (!combinput && ByTurn == '\0') {
		if (option_flags[CLAN_PROG_NUM] & UP_OPTION) {
			if (CLAN_PROG_NUM == QUOTES || CLAN_PROG_NUM == VOCD || CLAN_PROG_NUM == EVAL || CLAN_PROG_NUM == MORTABLE ||
				CLAN_PROG_NUM == SCRIPT_P || CLAN_PROG_NUM == KIDEVAL || CLAN_PROG_NUM == TIMEDUR)
				puts("+u : send output to just one output file.");
			else
				puts("+u : merge all specified files together.");
		}
		if (option_flags[CLAN_PROG_NUM] & UM_OPTION)
			puts("-u : compute result for each turn separately.");
	}
	if (option_flags[CLAN_PROG_NUM] & W_OPTION) {
		puts("+wN: display N number of utterance AFTER the given one");
		puts("-wN: display N number of utterance BEFORE the given one");
	}
	if (option_flags[CLAN_PROG_NUM] & Z_OPTION) {
		puts("+xCN: include only utterances which are C (>, <, =) than N items (w, c, m), \"+x=0w\" for zero words");
		puts("+xS: specify items to include in above count (Example: +xxxx +xyyy)");
		puts("-xS: specify items to exclude from above count");
	}
	if (option_flags[CLAN_PROG_NUM] & Y_OPTION) {
		puts("+y : work on TEXT format files one line at the time");
		puts("+y1: work on TEXT format files one utterance at the time");
	}
	if (option_flags[CLAN_PROG_NUM] & Z_OPTION)
		puts("+zN: compute statistics on a specified range of input data");
	if (option_flags[CLAN_PROG_NUM] & FR_OPTION) {
		if (replaceFile)
			puts("-1 : do not replace original data file with new one");
		else
			puts("+1 : replace original data file with new one");
	}
	if (option_flags[CLAN_PROG_NUM] & F_OPTION)
		puts("+/-2: -2 do not create different versions of output file names / +2 create them");
	puts("\tFile names can be \"*.cha\" or a file of list of names \"@:filename\"");
	if (isQuit)
		cutt_exit(0);
}

char *getfarg(char *f, char *f1, int *i) {
	return(f);
/*
	if (*f) return(f);
	(*i)++;
	if (f1 == NULL) {
		fputs("Unexpected ending.\n",stderr); cutt_exit(0);
	} else if (*f1 == '-')
		fprintf(stderr, "*** WARNING: second string %s looks like a switch\n",f1);
	return(f1);
*/
}

void no_arg_option(char *f) {
	if (*f) {
		fprintf(stderr,"Option \"%s\" does not take any arguments.\n", f-2);
		cutt_exit(0);
	}
}

/* maingetflag(f,f1,i) executes apropriate commands assosiated with the
   specified option found in "f" variable.
*/
void maingetflag(char *f, char *f1, int *i) { /* sets up options */
	UTTER *temp;
	char *ts;
	int  tc;

	f++;
	if (*f == '0' || *f == 'v') {
		no_arg_option(++f);
	} else if (*f == 'd' && option_flags[CLAN_PROG_NUM] & D_OPTION) {
		f++;
		onlydata=(char)(atoi(getfarg(f,f1,i))+1);
		if (onlydata < 0 || onlydata > OnlydataLimit) {
			if (CLAN_PROG_NUM == FREQ) {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				fprintf(stderr,"Please specify <, <=, =, =>, > followed by percentage value\n    OR\n");
			}
			if (OnlydataLimit == 1)
				fprintf(stderr, "The only +d level allowed is 0.\n");
			else
				fprintf(stderr, "The only +d levels allowed are 0-%d.\n", OnlydataLimit-1);
			cutt_exit(0);
		}
		if (chatmode == 0 && (onlydata == 3 || onlydata == 4)) {
			fprintf(stderr, "+d%d option can't be used with +y option.\n", onlydata-1);
			cutt_exit(0);
		}
		if (CLAN_PROG_NUM == CHECK) {
			if (onlydata == 3)
				puredata = 2;
			else
				puredata = 0;
		} else if (CLAN_PROG_NUM == CHIP) {
			if (onlydata == 2)
				puredata = 0;
		} else if (CLAN_PROG_NUM == COOCCUR) {
		} else if (CLAN_PROG_NUM == DIST) {
		} else if (CLAN_PROG_NUM == FLO) {
		} else if (CLAN_PROG_NUM == FREQ) {
			if (onlydata == 5 || onlydata == 6) {
				puredata = 0;
			} else if (onlydata == 3 || onlydata == 4) {
				if (ByTurn != '\0') {
					fprintf(stderr, "+d%d option can't be used with -u option.\n", onlydata-1);
					cutt_exit(0);
				}
//				OverWriteFile = TRUE;
			} else if (onlydata == 1 || onlydata == 7) {
				if (onlydata == 7)
					OverWriteFile = TRUE;
#if defined(_MAC_CODE) || defined(_WIN32)
				if (redirect_out.fp != NULL)
					puredata = 0;
				else
#endif
					puredata = 1;
			} else if (onlydata == 8) {
#if defined(_MAC_CODE) || defined(_WIN32)
				if (redirect_out.fp != NULL)
					puredata = 0;
				else
#endif
					puredata = 1;
			}
		} else if (CLAN_PROG_NUM == GEM) {
		} else if (CLAN_PROG_NUM == GEMFREQ) {
		} else if (CLAN_PROG_NUM == KWAL) {
			if (onlydata == 3)
				puredata = 0;
		} else if (CLAN_PROG_NUM == MAXWD) {
			if (onlydata == 1)
				puredata = 0;
		} else if (CLAN_PROG_NUM == MLT) {
		} else if (CLAN_PROG_NUM == MLU) {
		} else if (CLAN_PROG_NUM == PHONFREQ) {
			puredata = 0;
		} else if (CLAN_PROG_NUM == VOCD) {
		} else if (CLAN_PROG_NUM == WDLEN) {
		}
	} else if (*f == 'k' && option_flags[CLAN_PROG_NUM] & K_OPTION) {
		no_arg_option(++f);
	} else if (*f == 'l' && option_flags[CLAN_PROG_NUM] & L_OPTION) {
		isLanguageExplicit = TRUE;
		no_arg_option(++f);
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	} else if (*f == 'r' && f[1] == 'e' && option_flags[CLAN_PROG_NUM] & RE_OPTION) {
		f += 2;
		no_arg_option(f);
		isRecursive = TRUE;
#endif // defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	} else if (*f == 'r' && option_flags[CLAN_PROG_NUM] & R_OPTION) {
		f++;
		tc = atoi(getfarg(f,f1,i));
		if (tc == 0 && !*f)
			tc = 1;
		else if (tc < 1 || tc > 3) {
			if (tc == 4) {
				R4 = TRUE;
				isPrecliticUse = TRUE;
				isPostcliticUse = TRUE;
			} else if (*f == '5') {
				if (*(f+1) == '0')
					R5_1 = !R5_1;
				else
					R5 = !R5;
			} else if (tc == 6) {
				R6 = !R6;
				if (R6) {
					addword('r','i',"+</?>");
					addword('r','i',"+</->");
					addword('r','i',"+<///>");
					addword('r','i',"+<//>");
					addword('r','i',"+</>");
				} else {
					addword('\0','\0',"+</?>");
					addword('\0','\0',"+</->");
					addword('\0','\0',"+<///>");
					addword('\0','\0',"+<//>");
					addword('\0','\0',"+</>");
				}
			} else if (tc == 7) {
				R7Slash = FALSE;
				R7Tilda = FALSE;
				R7Caret = FALSE;
				R7Colon = FALSE;
				isRemoveCAChar[NOTCA_CROSSED_EQUAL] = FALSE;
				isRemoveCAChar[NOTCA_LEFT_ARROW_CIRCLE] = FALSE;
			} else if (tc == 8) {
				R8 = TRUE;
			} else {
				fputs("Choose N for +r option to be between 1 - 7\n",stderr);
				cutt_exit(0);
			}
		} else
			Parans = (char)tc;
	} else if (*f == 's') {
		char wd[1024];
		if (*(f-1) == '+' && option_flags[CLAN_PROG_NUM] & SP_OPTION) {
			f++;
			if (*f) {
				if ((*f == '+' || *f == '~') &&  *(f+1) == '@') {
					*(f+1) = *f;
					*f = '@';
				}
				if (*f == '@' && isMorSearchOption(f, *(f-2))) {
					if (*(f+1) == '+' || *(f+1) == '~') {
						*f = *(f+1);
						*(f+1) = '\002';
					} else
						*f = '\002';
					addword('s','i',getfarg(f,f1,i));
					*f = '@';
				} else if (*f == '@' && isLangSearchOption(f, *(f-2))) {
					if (*(f+1) == '+' || *(f+1) == '~') {
						*f = *(f+1);
						*(f+1) = '\003';
					} else
						*f = '\003';
					addword('s','i',getfarg(f,f1,i));
					*f = '@';
				} else if (*f == '@') {
					f++;
					rdexclf('s','i',getfarg(f,f1,i));
				} else {
					if (*f == '\\' && *(f+1) == '@')
						f++;
					strcpy(wd, getfarg(f,f1,i));
					removeExtraSpace(wd);
					uS.remFrontAndBackBlanks(wd);
					if (strchr(wd, '[') == NULL && strchr(wd, '<') == NULL && 
						  (strchr(wd, ' ') != NULL || anyMultiOrder || onlySpecWsFound)) {
						if (CLAN_PROG_NUM != FREQ) {
							fprintf(stderr,"Multi-words are only allowed with FREQ command.\n");
							cutt_exit(0);
						} else
							mwdptr = InsertMulti(mwdptr, wd);
					} else
						addword('s','i',getfarg(f,f1,i));
				}
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
		} else if (option_flags[CLAN_PROG_NUM] & SM_OPTION) {
			f++;
			if (*f) {
				if ((*f == '+' || *f == '~') &&  *(f+1) == '@') {
					*(f+1) = *f;
					*f = '@';
				}
				if (*f == '@' && isMorSearchOption(f, *(f-2))) {
					if (*(f+1) == '+' || *(f+1) == '~') {
						*f = *(f+1);
						*(f+1) = '\002';
					} else
						*f = '\002';
					addword('s','e',getfarg(f,f1,i));
					*f = '@';
				} else if (*f == '@' && isLangSearchOption(f, *(f-2))) {
					if (*(f+1) == '+' || *(f+1) == '~') {
						*f = *(f+1);
						*(f+1) = '\003';
					} else
						*f = '\003';
					addword('s','e',getfarg(f,f1,i));
					*f = '@';
				} else if (*f == '@') {
					f++;
					rdexclf('s','e',getfarg(f,f1,i));
				} else {
					if (*f == '\\' && *(f+1) == '@')
						f++;
					strcpy(wd, getfarg(f,f1,i));
					removeExtraSpace(wd);
					uS.remFrontAndBackBlanks(wd);
					if (strchr(wd, '[') == NULL && strchr(wd, '<') == NULL && strchr(wd, ' ') != NULL) {
						fprintf(stderr,"Multi-words are not allowed with option: \"%s\"\n", f-2);
						fprintf(stderr,"    Multi-words are search patterns with space character(s) in-between.\n");
						fprintf(stderr,"    If you did not mean to have space characters in-between words,\n");
						fprintf(stderr,"        then please remove space characters and try again.\n");
						cutt_exit(0);
					} else
						addword('s','e',getfarg(f,f1,i));
				}
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
		} else {
			fprintf(stderr,"Invalid option: %s\n", f-1);
			cutt_exit(0);
		}
#if !defined(CLAN_SRV)
	} else  if (*f == 'f' && option_flags[CLAN_PROG_NUM] & F_OPTION) {
		if (*(f-1) == '+') {
			if (!IsOutTTY()) {
				fprintf(stderr,"Option \"%s\" can't be used with file redirect \">\".\n", f-1);
				cutt_exit(0);
			}
			f++;
			stout = FALSE;
			if (*f) {
				if (*f != '.')
					strcpy(GExt, ".");
				else
					GExt[0] = EOS;
				strncat(GExt, f, 31);
				GExt[31] = EOS;
//				AddCEXExtension = "";
			}
		} else {
			f_override = TRUE;
			stout = TRUE;
		}
#endif
	} else if (*f == 't' && option_flags[CLAN_PROG_NUM] & T_OPTION) {
		f++;
		if (chatmode) {
			if (*f == '@') {
				if ((*(f+1) == 'I' || *(f+1) == 'i') && (*(f+2) == 'D' || *(f+2) == 'd') && (*(f+3) == '=') && *(f+4) != EOS) {
					AddToID(getfarg(f,f1,i)+4, *(f-2));
				} else if ((*(f+1) == 'K' || *(f+1) == 'k') && (*(f+2) == 'E' || *(f+2) == 'e') && (*(f+3) == 'Y' || *(f+3) == 'y') && (*(f+4) == '=') && *(f+5) != EOS) {
					AddToCode(getfarg(f,f1,i)+5, *(f-2));
				} else
					maketierchoice(getfarg(f,f1,i), *(f-2), FALSE);
			} else if (*f == '#')
				AddToSPRole(getfarg(f,f1,i)+1, *(f-2));
			else
				maketierchoice(getfarg(f,f1,i), *(f-2), FALSE);
		} else {
			fputs("+/-t option is not allowed with TEXT format\n",stderr);
			cutt_exit(0);
		}
	} else if (*f == 'u') {
/* FREQ +d2 +d3
		if (onlydata == 3 || onlydata == 4) {
			fprintf(stderr, "+/-u option can't be used with +d%d option.\n", onlydata-1);
			cutt_exit(0);
		}
*/
		if ((option_flags[CLAN_PROG_NUM] & UP_OPTION) && *(f-1) == '+') {
			if (ByTurn) {
				fputs("+u option can't be used with -u option.\n",stderr);
				cutt_exit(0);
			}
			combinput = TRUE;
		} else if ((option_flags[CLAN_PROG_NUM] & UM_OPTION) && *(f-1)== '-') {
			if (chatmode == 0) {
				fputs("-u option can't be used in nonCHAT mode.\n",stderr);
				cutt_exit(0);
			}
			if (combinput) {
				fputs("-u option can't be used with +u option.\n",stderr);
				cutt_exit(0);
			}
			if (TSoldsp != NULL) {
				free(TSoldsp);
				TSoldsp = NULL;
			}
			if (TSoldsp == NULL) {
				TSoldsp = (char *) malloc(SPEAKERLEN+1);
				*TSoldsp = EOS;
			}
			ByTurn = 1;
		} else {
			fprintf(stderr,"Invalid option: %s\n", f-1);
			cutt_exit(0);
		}
		no_arg_option(++f);
	} else if (*f == 'p' && option_flags[CLAN_PROG_NUM] & P_OPTION) {
		int i, j;
		f++;
		if (*f == EOS) {
		   fputs("Please specify word delimiter characters with +p option.\n",stderr);
		   cutt_exit(0);
		}
		j = strlen(GlobalPunctuation);
		for (; *f != EOS; f++) {
			if (!isSpace(*f)) {
				for (i=0; GlobalPunctuation[i] != EOS; i++) {
					if (*f == GlobalPunctuation[i])
						break;
				}
				if (GlobalPunctuation[i] == EOS) {
					GlobalPunctuation[j++] = *f;
				}
			}
		}
		GlobalPunctuation[j] = EOS;
		punctuation = GlobalPunctuation;
//2011-01-26		uS.str2FNType(punctFile, 0L, getfarg(f,f1,i));
	} else if (*f == 'y' && option_flags[CLAN_PROG_NUM] & Y_OPTION) {
		f++;
		if (headtier != NULL || IDField != NULL || CODEField != NULL || SPRole != NULL) {
		   fputs("ERROR: +y option can't be used with +/-t option.\n",stderr);
		   cutt_exit(0);
		}
		if (ByTurn) {
		   fputs("ERROR: +y option can't be used with -u option.\n",stderr);
		   cutt_exit(0);
		}
		if (onlydata == 3 || onlydata == 4) {
		   fputs("+y option can't be used with +d2 or +d3 option.\n",stderr);
		   cutt_exit(0);
		}
		chatmode = 0;
		nomain = FALSE;
		temp = utterance;
		do {
			*temp->speaker = EOS;
			temp = temp->nextutt;
		} while (temp != utterance) ;
		for (ts=GlobalPunctuation; *ts; ) {
			if (*ts == ':')
				strcpy(ts,ts+1);
			else
				ts++;
		}
		if (*f == EOS)
			y_option = 0;
		else
			y_option = (char)(atoi(getfarg(f,f1,i)));
		if (y_option < 0 || y_option > 1) {
			fputs("Choose N for +y option to be between 0 - 1\n",stderr);
			cutt_exit(0);
		}
	} else if (*f == 'z' && option_flags[CLAN_PROG_NUM] & Z_OPTION) {
		f++;
		if (Toldsp == NULL) {
			Toldsp = (char *) malloc(SPEAKERLEN+1);
			*Toldsp = EOS;
		}
		if (!getrange(getfarg(f,f1,i))) {
			if (Toldsp != NULL) {
				free(Toldsp);
				Toldsp = NULL;
			}
			cutt_exit(0);
		}
		if (!isWinMode && UttlineEqUtterance) {
			UttlineEqUtterance = FALSE;
			uttline = utterance->tuttline;
		}
	} else if (*f == 'x' && option_flags[CLAN_PROG_NUM] & Z_OPTION) {
		f++;
		if (*f == '=' || *f == '<' || *f == '>') {
			filterUttLen_cmp = *f++;
			if (!getxrange(f)) {
				cutt_exit(0);
			}
		} else {
			if (*f == EOS) {
				fprintf(stderr, "\nPlease specify comparison type (=, <, >)\n");
				fprintf(stderr, "\tnumber and type (m, w, c) of items after \"+x\" option.\n");
				fprintf(stderr, "Or words to exclude from +x count.\n");
				fprintf(stderr,"\tFor example: +x=0w - select only utterances with zero words\n");
				fprintf(stderr,"\t\t+x>0w - select utterances with 1 or more words\n");
				fprintf(stderr,"\t\t+xword - count only this \"word\"\n");
				fprintf(stderr,"\t\t-xword - do not count this \"word\"\n");
				cutt_exit(0);
			}
			if (*(f-2) == '+') {
				if (uS.mStricmp(f, "xxx") == 0 || uS.mStricmp(f, "xx") == 0)
					restoreXXX = TRUE;
				else if (uS.mStricmp(f, "yyy") == 0 || uS.mStricmp(f, "yy") == 0)
					restoreYYY = TRUE;
				else if (uS.mStricmp(f, "www") == 0 || uS.mStricmp(f, "ww") == 0)
					restoreWWW = TRUE;
				else {
					if (*f == '@') {
						f++;
						rdexclfUttLen('i', f);
					} else {
						if (*f == '\\' && *(f+1) == '@')
							f++;
						addwordUttLen('i', f);
					}
				}
			} else {
				if (uS.mStricmp(f, "xxx") == 0 || uS.mStricmp(f, "xx") == 0)
					restoreXXX = FALSE;
				else if (uS.mStricmp(f, "yyy") == 0 || uS.mStricmp(f, "yy") == 0)
					restoreYYY = FALSE;
				else if (uS.mStricmp(f, "www") == 0 || uS.mStricmp(f, "ww") == 0)
					restoreWWW = FALSE;
				else {
					if (*f == '@') {
						f++;
						rdexclfUttLen('e', f);
					} else {
						if (*f == '\\' && *(f+1) == '@')
							f++;
						addwordUttLen('e', f);
					}
				}
			}
		}
		if (!isWinMode && UttlineEqUtterance) {
			UttlineEqUtterance = FALSE;
			uttline = utterance->tuttline;
		}
	} else if (*f == 'w' && option_flags[CLAN_PROG_NUM] & W_OPTION) {
		if (CLAN_PROG_NUM == KWAL && onlydata == 5) {
			fputs("+w option can't be used with +d4 option.\n",stderr);
			cutt_exit(0);
		}
		f++;
		if (*(f-2) == '-') makewind(atoi(getfarg(f,f1,i)));
		else makewind(aftwin=atoi(getfarg(f,f1,i)));
	} else if (*f == 'o' && option_flags[CLAN_PROG_NUM] & O_OPTION) {
		f++;
		MakeOutTierChoice(getfarg(f,f1,i), *(f-2));
#if !defined(CLAN_SRV)
	} else if (*f == '1' && option_flags[CLAN_PROG_NUM] & FR_OPTION) {
		f++;
		no_arg_option(f);
		if (replaceFile) {
			if (*(f-2) == '-')
				replaceFile = FALSE;
		} else {
			if (*(f-2) == '+')
				replaceFile = TRUE;
		}
	} else if (*f == '2') {
		f++;
		no_arg_option(f);
		if (*(f-2) == '+')
			OverWriteFile = FALSE;
		else
			OverWriteFile = TRUE;
#endif
	} else {
		fprintf(stderr,"Invalid option: %s\n", f-1);
		cutt_exit(0);
	}
}

// @Time Duration:	@t<hh:mm-hh:mm> @t<hh:mm:ss-hh:mm:ss> @t<hh:mm:ss>

float getTimeDuration(char *st) {
	char *col, *time1S, *time2S;
	float h, m, s, time1, time2;

	while (isSpace(*st))
		st++;
	col = strchr(st,'-');
	if (col != NULL) {
		*col = EOS;
		time1S = st;
		time2S = col + 1;
		while (isSpace(*time2S))
			time2S++;
	} else {
		time1S = st;
		time2S = NULL;
	}
	h = atof(time1S);
	col = strchr(time1S,':');
	if (col != NULL) {
		*col = EOS;
		col++;
		m = atof(col);
		col = strchr(col,':');
		if (col != NULL) {
			*col = EOS;
			col++;
			s = atof(col);
		} else
			s = 0.0;
	} else {
		m = 0.0;
		s = 0.0;
	}
	time1 = (h * 3600.0000) + (m * 60.0000) + s;
	if (time2S != NULL) {
		h = atof(time2S);
		col = strchr(time2S,':');
		if (col != NULL) {
			*col = EOS;
			col++;
			m = atof(col);
			col = strchr(col,':');
			if (col != NULL) {
				*col = EOS;
				col++;
				s = atof(col);
			} else
				s = 0.0;
		} else {
			m = 0.0;
			s = 0.0;
		}
		time2 = (h * 3600.0000) + (m * 60.0000) + s;
		time1 = time2 - time1;
	}
	return(time1);
}


void Secs2Str(float tm, char *st) {
	int   i;
	char  isNeg;
	float h, m, s;

	isNeg = FALSE;
	if (tm < 0.0)
		isNeg = TRUE;
	h = tm / 3600.0;
	i = h;
	h = i;
	tm = tm - (h * 3600.0);
	m = tm / 60;
	i = m;
	m = i;
	tm = tm - (m * 60.0);
	i = tm;
	s = i;
	st[0] = EOS;
	if (isNeg)
		strcat(st, "-");
	i = strlen(st);
	if (h >= 1.0)
		sprintf(st+i, "%.0f:", h);
	else
		sprintf(st+i, "00:");
	i = strlen(st);
	if (m < 10.00 && h >= 1.0)
		sprintf(st+i, "0%.0f:", m);
	else
		sprintf(st+i, "%.0f:", m);
	i = strlen(st);
	if (s < 10.00)
		sprintf(st+i, "0%.0f", s);
	else
		sprintf(st+i, "%.0f", s);
}

AttTYPE set_color_num(char num, AttTYPE att) {	// bits 8=128, 9=256, 10=512
	AttTYPE t;

	t = 0;
	t = (AttTYPE)num;
	t = t & (AttTYPE)0x7;
	t = t << 7;
	att = zero_color_num(att);
	att = att | t;
	return(att);
}

char get_color_num(AttTYPE att) {			// bits 8=128, 9=256, 10=512
	AttTYPE t;
	char num;

	t = att;
	t = t & (AttTYPE)0x380;
	t = t >> 7;
	num = (char)t;
	return(num);
}

char SetTextAtt(char *st, unCH c, AttTYPE *att) {
	char found = FALSE;

	if (st != NULL)
		c = st[1];
	if (c == underline_start) {
		if (att != NULL)
			*att = set_underline_to_1(*att);
		found = TRUE;
	} else if (c == underline_end) {
		if (att != NULL)
			*att = set_underline_to_0(*att);
		found = TRUE;
	} else if (c == italic_start) {
		if (att != NULL)
			*att = set_italic_to_1(*att);
		found = TRUE;
	} else if (c == italic_end) {
		if (att != NULL)
			*att = set_italic_to_0(*att);
		found = TRUE;
	} else if (c == bold_start) {
		if (att != NULL)
			*att = set_bold_to_1(*att);
		found = TRUE;
	} else if (c == bold_end) {
		if (att != NULL)
			*att = set_bold_to_0(*att);
		found = TRUE;
	} else if (c == error_start) {
		if (att != NULL)
			*att = set_error_to_1(*att);
		found = TRUE;
	} else if (c == error_end) {
		if (att != NULL)
			*att = set_error_to_0(*att);
		found = TRUE;
	} else if (c == blue_start) {
		if (att != NULL)
			*att = set_color_num(blue_color, *att);
		found = TRUE;
	} else if (c == red_start) {
		if (att != NULL)
			*att = set_color_num(red_color, *att);
		found = TRUE;
	} else if (c == green_start) {
		if (att != NULL)
			*att = set_color_num(green_color, *att);
		found = TRUE;
	} else if (c == magenta_start) {
		if (att != NULL)
			*att = set_color_num(magenta_color, *att);
		found = TRUE;
	} else if (c == color_end) {
		if (att != NULL)
			*att = zero_color_num(*att);
		found = TRUE;
	}

	if (found) {
		if (st != NULL)
			strcpy(st, st+2);
		return(TRUE);
	} else {
		return(FALSE);
	}
}

void copyFontInfo(FONTINFO *des, FONTINFO *src, char isUse) {
	SetFontName(des->FName, src->FName);
	des->FSize   = src->FSize;
	des->CharSet = src->CharSet;
	des->Encod = src->Encod;
	des->FHeight = src->FHeight;
#ifndef UNX
	if (isUse && global_df != NULL) {
		SetFontName(global_df->row_txt->Font.FName, des->FName);
		global_df->row_txt->Font.FSize   = des->FSize;
		global_df->row_txt->Font.CharSet = des->CharSet;
		global_df->row_txt->Font.FHeight = des->FHeight;
		global_df->row_txt->Font.Encod   = des->Encod;
	}
#endif
}

void copyNewToFontInfo(FONTINFO *des, NewFontInfo *src) {
#ifndef UNX
#ifdef _MAC_CODE
	des->FName = src->fontId;
#endif
#ifdef _WIN32
	strcpy(des->FName, src->fontName);
#endif
	des->FSize   = src->fontSize;
	des->CharSet = src->CharSet;
	des->FHeight = src->charHeight;
	des->Encod   = src->Encod;
#endif
}

void copyNewFontInfo(NewFontInfo *des, NewFontInfo *src) {
	strcpy(des->fontName, src->fontName);
	des->fontPref = src->fontPref;
	des->fontSize = src->fontSize;
	des->CharSet  = src->CharSet;
	des->charHeight  = src->charHeight;
	des->platform = src->platform;
	des->fontType = src->fontType;
	des->orgFType = src->orgFType;
	des->fontId = src->fontId;
	des->isUTF = src->isUTF;
	des->Encod = src->Encod;
	des->orgEncod = src->orgEncod;
	des->fontTable = src->fontTable;
}

/************************************************************/
/*	 Find and set the current font of the tier			*/
#if defined(_WIN32)
static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *font,NEWTEXTMETRICEX *t,int ft,LPARAM finfo) {
	NewFontInfo *fi = (NewFontInfo *)finfo;

	if (strcmp(font->elfLogFont.lfFaceName, fi->fontName) == 0 &&
		(fi->CharSet == DEFAULT_CHARSET || fi->CharSet == font->elfLogFont.lfCharSet)) {
		fi->CharSet = font->elfLogFont.lfCharSet;
		fi->Encod = GetEncode(fi->fontPref, fi->fontName, fi->fontType, fi->CharSet, FALSE);
		return(0);
	}
	return(1);
}

static int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *font,NEWTEXTMETRIC FAR *t,int ft,LPARAM finfo) {
	NewFontInfo *fi = (NewFontInfo *)finfo;

	if (strcmp(font->elfLogFont.lfFaceName, fi->fontName) == 0 &&
		(fi->CharSet == DEFAULT_CHARSET || fi->CharSet == font->elfLogFont.lfCharSet)) {
		fi->CharSet = (int)font->elfLogFont.lfCharSet;
		fi->Encod = GetEncode(fi->fontPref, fi->fontName, fi->fontType, fi->CharSet, FALSE);
		return(0);
	}
	return(1);
}
#endif /* _WIN32 */

void SetDefaultThaiFinfo(NewFontInfo *finfo) {
#ifdef _WIN32
	strcpy(finfo->fontName, "Arial Unicode MS");
	finfo->fontSize = -15;
	finfo->fontPref = "Win95:";
	finfo->fontId = DEFAULT_ID;
	finfo->fontType = getFontType(finfo->fontName, TRUE);
	finfo->orgFType = finfo->fontType;
	finfo->CharSet = 0;
	finfo->charHeight = GetFontHeight(NULL, finfo);
	finfo->fontTable = NULL;
#endif
#ifdef _MAC_CODE
	strcpy(finfo->fontName, "Ayuthaya");
	finfo->fontSize = 15;
	finfo->fontPref = "";
	finfo->fontType = getFontType(finfo->fontName, FALSE);
	finfo->orgFType = finfo->fontType;
	if (!GetFontNumber(finfo->fontName, &finfo->fontId)) {
		strcpy(finfo->fontName, defUniFontName);
		finfo->fontSize = defUniFontSize;
		finfo->fontId = DEFAULT_ID;
		finfo->fontType = getFontType(finfo->fontName, FALSE);
		finfo->orgFType = finfo->fontType;
		if (!GetFontNumber(finfo->fontName, &finfo->fontId)) {
			strcpy(finfo->fontName, DEFAULT_FONT);
			finfo->fontSize = DEFAULT_SIZE;
			finfo->fontId = DEFAULT_ID;
			finfo->fontType = getFontType(finfo->fontName, FALSE);
			finfo->orgFType = finfo->fontType;
		}
	}
	if (global_df != NULL)
		finfo->charHeight = GetFontHeight(NULL, finfo, global_df->wind);
	else
		finfo->charHeight = 12;
	finfo->fontTable = NULL;
	finfo->CharSet = my_FontToScript(finfo->fontId, 0);
#endif
#ifdef UNX
	strcpy(finfo->fontName, "Arial Unicode MS");
	finfo->fontSize = 12L;
	finfo->fontPref = "";
	finfo->fontType = getFontType(finfo->fontName, FALSE);
	finfo->orgFType = finfo->fontType;
	finfo->fontId = 0;
	finfo->charHeight = 12;
	finfo->fontTable = NULL;
	finfo->CharSet = 0;
#endif
}

void SetDefaultCAFinfo(NewFontInfo *finfo) {
#ifdef _WIN32
	if (!strcmp(finfo->fontName, "CAfont"))
		return;
	strcpy(finfo->fontName, "CAfont");
	finfo->fontSize = -15;
	finfo->fontPref = "Win95:";
	finfo->fontId = DEFAULT_ID;
	finfo->fontType = getFontType(finfo->fontName, TRUE);
	finfo->orgFType = finfo->fontType;
	finfo->CharSet = 0;
	finfo->charHeight = GetFontHeight(NULL, finfo);
	finfo->fontTable = NULL;
#endif
#ifdef _MAC_CODE
	if (!strcmp(finfo->fontName, "CAfont"))
		return;
	strcpy(finfo->fontName, "CAfont");
	finfo->fontSize = 13;
	finfo->fontPref = "";
	finfo->fontType = getFontType(finfo->fontName, FALSE);
	finfo->orgFType = finfo->fontType;
	if (!GetFontNumber(finfo->fontName, &finfo->fontId)) {
		strcpy(finfo->fontName, defUniFontName);
		finfo->fontSize = defUniFontSize;
		finfo->fontId = DEFAULT_ID;
		finfo->fontType = getFontType(finfo->fontName, FALSE);
		finfo->orgFType = finfo->fontType;
		if (!GetFontNumber(finfo->fontName, &finfo->fontId)) {
			strcpy(finfo->fontName, DEFAULT_FONT);
			finfo->fontSize = DEFAULT_SIZE;
			finfo->fontId = DEFAULT_ID;
			finfo->fontType = getFontType(finfo->fontName, FALSE);
			finfo->orgFType = finfo->fontType;
		}
	}
	if (global_df != NULL)
		finfo->charHeight = GetFontHeight(NULL, finfo, global_df->wind);
	else
		finfo->charHeight = 12;
	finfo->fontTable = NULL;
	finfo->CharSet = my_FontToScript(finfo->fontId, 0);
#endif
#ifdef UNX
	strcpy(finfo->fontName, "Arial Unicode MS");
	finfo->fontSize = 12L;
	finfo->fontPref = "";
	finfo->fontType = getFontType(finfo->fontName, FALSE);
	finfo->orgFType = finfo->fontType;
	finfo->fontId = 0;
	finfo->charHeight = 12;
	finfo->fontTable = NULL;
	finfo->CharSet = 0;
#endif
}

char SetDefaultUnicodeFinfo(NewFontInfo *finfo) {
// DO NOT SET finfo->Encod HERE
	short lOrgFType = finfo->orgFType;
#ifdef _WIN32
	strcpy(finfo->fontName, defUniFontName);
	finfo->fontSize = defUniFontSize;
	finfo->fontPref = "Win95:";
	finfo->fontId = DEFAULT_ID;
	finfo->fontType = getFontType(finfo->fontName, TRUE);
	finfo->orgFType = finfo->fontType;
	finfo->CharSet = 0;
	finfo->charHeight = GetFontHeight(NULL, finfo);
	finfo->fontTable = NULL;
	return(lOrgFType == WINArialUC || lOrgFType == WINCAFont);
#endif
#ifdef _MAC_CODE
	strcpy(finfo->fontName, defUniFontName);
	finfo->fontSize = defUniFontSize;
	finfo->fontPref = "";
	finfo->fontType = getFontType(finfo->fontName, FALSE);
	finfo->orgFType = finfo->fontType;
	if (!GetFontNumber(finfo->fontName, &finfo->fontId)) {
		strcpy(finfo->fontName, DEFAULT_FONT);
		finfo->fontSize = DEFAULT_SIZE;
		finfo->fontId = DEFAULT_ID;
		finfo->fontType = getFontType(finfo->fontName, FALSE);
		finfo->orgFType = finfo->fontType;
	}
	if (global_df != NULL)
		finfo->charHeight = GetFontHeight(NULL, finfo, global_df->wind);
	else
		finfo->charHeight = 12;
	finfo->fontTable = NULL;
	finfo->CharSet = my_FontToScript(finfo->fontId, 0);
	return(lOrgFType == MACArialUC || lOrgFType == MacCAFont);
#endif
#ifdef UNX
	strcpy(finfo->fontName, "Arial Unicode MS");
	finfo->fontSize = 12L;
	finfo->fontPref = "";
	finfo->fontType = getFontType("Arial Unicode MS", FALSE);
	finfo->orgFType = finfo->fontType;
	finfo->fontId = 0;
	finfo->charHeight = 12;
	finfo->fontTable = NULL;
	finfo->CharSet = 0;
	return(lOrgFType == MACArialUC);
#endif
}

char selectChoosenFont(NewFontInfo *finfo, char isForced) {
	char isUnicodeFont = FALSE;

	if (!stout && !isForced)
		return(TRUE);
#if defined(_MAC_CODE)
	if (CLAN_PROG_NUM == CP2UTF && strcmp(finfo->fontName, "LxS SpEcIaL FoNt") == 0) {
		dFnt.orgEncod = GetEncode("", "", finfo->fontType, finfo->orgEncod, FALSE);
		dFnt.CharSet = finfo->orgEncod;
		return(TRUE);
	} else if (!GetFontNumber(finfo->fontName, &finfo->fontId)) {
		fontErrorReported = TRUE;
		return(FALSE);
	}
	isUnicodeFont = (strcmp("Arial Unicode MS", dFnt.fontName) == 0 || strcmp("CAfont"/*UNICODEFONT*/, dFnt.fontName) == 0);
	strcpy(dFnt.fontName, finfo->fontName);
	dFnt.fontId = finfo->fontId;
	if (isUnicodeFont && strcmp("CAfont", dFnt.fontName) == 0)
		dFnt.fontSize = finfo->fontSize;
//31-03-03	dFnt.fontSize = finfo->fontSize;
	dFnt.CharSet = finfo->CharSet;
	if (global_df != NULL)
		dFnt.charHeight = GetFontHeight(NULL, &dFnt, global_df->wind);
	dFnt.Encod = my_FontToScript(finfo->fontId, finfo->CharSet);
	dFnt.orgEncod = dFnt.Encod;
	MBF = (dFnt.Encod == 1 || dFnt.Encod == 2 || dFnt.Encod == 3);
	return(TRUE);
#endif
#ifdef _WIN32
	LOGFONT t_lfDefFont;

	SetLogfont(&t_lfDefFont, NULL, finfo);
	if (EnumFontFamiliesEx(GlobalDC->GetSafeHdc(),&t_lfDefFont,(FONTENUMPROC)EnumFontFamExProc,(LPARAM)finfo,0) != 0) {
		if (EnumFontFamilies(GlobalDC->GetSafeHdc(),cl_T(finfo->fontName),(FONTENUMPROC)EnumFontFamProc,(LPARAM)finfo)) {
/*
			sprintf(templineC3, "Windows font \"%s\", script %d, is not found on this computer",finfo->fontName, finfo->CharSet);
			if (!fontErrorReported)
				do_warning(templineC3, 0);
*/
			fontErrorReported = TRUE;
			return(FALSE);
		}
	}
	isUnicodeFont = (strcmp("Arial Unicode MS", dFnt.fontName) == 0 || strcmp("CAfont"/*UNICODEFONT*/, dFnt.fontName) == 0);
	strcpy(dFnt.fontName, finfo->fontName);
	dFnt.Encod = my_FontToScript(finfo->fontName, finfo->CharSet);
	strcpy(dFnt.fontName, finfo->fontName);
	if (isUnicodeFont && strcmp("CAfont", dFnt.fontName) == 0)
		dFnt.fontSize = finfo->fontSize;
//31-03-03	dFnt.fontSize = finfo->fontSize;
	dFnt.CharSet = finfo->CharSet;
	if (global_df != NULL)
		dFnt.charHeight = GetFontHeight(NULL, &dFnt);
	MBF = (dFnt.Encod == 1 || dFnt.Encod == 2 || dFnt.Encod == 3);
	return(TRUE);
#endif /* _WIN32 */
	return(TRUE);
}

char cutt_SetNewFont(const char *st, char ec) {
	NewFontInfo finfo;
	int  i;
#if defined(_MAC_CODE) || defined(_WIN32)
	char oFontName[256];
	const char *oFontPref;
	short oFontType;
	int  oCharSet;
#endif

	if (ec != EOS) {
		for (i=0; st[i] && st[i] != ':' && st[i] != ec; i++) ;
		if (st[i] != ':')
			return(FALSE);
		i++;
	} else
		i = 0;

	if (GetDatasFont(st+i, ec, &dFnt, dFnt.isUTF, &finfo) == NULL)
		return(FALSE);

	dFnt.orgFType = finfo.orgFType;
#if defined(_MAC_CODE)
	if (finfo.platform == MACDATA) {
		return(selectChoosenFont(&finfo, FALSE));
	} else {
		strcpy(oFontName, finfo.fontName);
		oFontPref = finfo.fontPref;
		oCharSet = finfo.CharSet;
		oFontType = finfo.fontType;
		if (FindTTable(&finfo, MACDATA)) {
			dFnt.orgEncod = GetEncode(oFontPref, oFontName, oFontType, oCharSet, FALSE);
			if (finfo.fontTable == NULL) {
				return(selectChoosenFont(&finfo, FALSE));
			}
			return(TRUE);
		} else {
/* 2007-08-28
			if (!fontErrorReported)
				fprintf(stderr, "Font \"%s%s\" is not supported on this computer.\n", oFontPref, oFontName);
			fontErrorReported = TRUE;
*/
			dFnt.orgEncod = GetEncode(oFontPref, oFontName, oFontType, oCharSet, !dFnt.isUTF);
		}
	}
#endif
#ifdef _WIN32
	strcpy(oFontName, finfo.fontName);
	oFontPref = finfo.fontPref;
	oCharSet = finfo.CharSet;
	oFontType = finfo.fontType;
	if (finfo.platform == WIN95DATA) {
		dFnt.orgEncod = GetEncode(oFontPref, oFontName, oFontType, oCharSet, !dFnt.isUTF);
		return(selectChoosenFont(&finfo, FALSE));
	} else {
		if (FindTTable(&finfo, WIN95DATA)) {
			dFnt.orgEncod = GetEncode(oFontPref, oFontName, oFontType, oCharSet, !dFnt.isUTF);
			if (finfo.fontTable == NULL) {
				return(selectChoosenFont(&finfo, FALSE));
			}
			return(TRUE);
		} else {
/* 2007-08-28
			if (!fontErrorReported)
				fprintf(stderr, "Font \"%s%s\" is not supported on this computer.\n", oFontPref, oFontName);
			fontErrorReported = TRUE;
*/
			dFnt.orgEncod = GetEncode(oFontPref, oFontName, oFontType, oCharSet, !dFnt.isUTF);
		}
	}
#endif /* _WIN32 */
	return(FALSE);
}

// Languages begin
#define LANGSFILE "ISO-639.cut"
void initLanguages(void) {
	langs_list = NULL;
}

static LANGSTABLE *free_langs(LANGSTABLE *p) {
	int i;
	LANGSTABLE *t;

	while (p != NULL) {
		t = p;
		p = p->nextlang;
		for (i=0; i < 4; i++) {
			if (t->iso639[i] != NULL)
				free(t->iso639[i]);
		}
		if (t->name != NULL)
			free(t->name);
		free(t);
	}
	return(NULL);
}

void cleanupLanguages(void) {
	langs_list = free_langs(langs_list);
}

static void createLanguageTable(FILE *fp) {
	int  i, len;
	char *bg, *eg, buf[BUFSIZ], t, isName;
	LANGSTABLE *pt;

	while (fgets_cr(buf, BUFSIZ, fp)) {
		if (uS.isUTF8(buf) || uS.partcmp(buf,FONTHEADER,FALSE,FALSE) ||
			buf[0] == '%' || buf[0] == '#')
			continue;
		uS.remFrontAndBackBlanks(buf);
		if (*buf != EOS) {
			if ((pt=NEW(LANGSTABLE)) == NULL)
				return;
			pt->nextlang = langs_list;
			for (i=0; i < 4; i++)
				pt->iso639[i] = NULL;
			pt->name = NULL;
			langs_list = pt;
			i = 0;
			bg = buf;
			while (*bg != EOS) {
				for (; isSpace(*bg); bg++) ;
				if (*bg == EOS)
					break;
				isName = FALSE;
				for (eg=bg; *eg != EOS; eg++) {
					if (*eg == '\t')
						break;
					if (*eg == ' ' && !isName) {
						len = eg - bg;
						if (len < 4 && islower(*bg))
							break;
						else if (((bg[2] == '-' && len == 6) || (bg[3] == '-' && (len == 6 || len == 7))) && islower(*bg))
							break;
						else
							isName = TRUE;
					}
				}
				if (*eg == EOS) {
					isName = TRUE;
				}
				t = *eg;
				*eg = EOS;
				if (isName) {
					if (langs_list->name != NULL)
						free(langs_list->name);
					langs_list->name = (char *)malloc(strlen(bg)+1);
					if (langs_list->name != NULL)
						strcpy(langs_list->name, bg);
				} else {
					if (i < 4) {
						if (langs_list->iso639[i] != NULL)
							free(langs_list->iso639[i]);
						langs_list->iso639[i] = (char *)malloc(strlen(bg)+1);
						if (langs_list->iso639[i] != NULL) {
							strcpy(langs_list->iso639[i], bg);
							i++;
						}
					}
				}
				*eg = t;
				bg = eg;
			}
		}
	}
}

char ReadLangsFile(char isCED) {
	FILE *fp;
	FNType mFileName[FNSize];

#ifndef UNX
	if (!isCED) {
		if (WD_Not_Eq_OD)
			SetNewVol(wd_dir);
	}
#endif
	if ((fp=OpenGenLib(LANGSFILE,"r",TRUE,TRUE,mFileName)) == NULL) {
		if (!isCED) {
			fprintf(stderr, "Can't open either one of the language codes list files:\n  \"%s\", \"%s\"\n", LANGSFILE, mFileName);
			fprintf(stderr, "Check to see if lib directory is set correctly. It is located next to CLAN application.\n");
#ifdef _MAC_CODE
			fprintf(stderr, "\n   Lib directory can be set in \"Commands\" window with \"lib\" button to\n");
			fprintf(stderr, "   \"/Applications/CLAN/lib\" directory, for example.\n");
#endif
#ifdef _WIN32
			fprintf(stderr, "\n   Lib directory can be set in \"Commands\" window with \"lib\" button to\n");
			fprintf(stderr, "   \"C:\\TalkBank\\CLAN\\lib\\\" directory, for example.\n");
#endif
			cutt_exit(0);
		} else
			return(FALSE);
	}
	if (!isCED) {
#ifndef UNX
		if (WD_Not_Eq_OD)
			SetNewVol(od_dir);
#endif
		fprintf(stderr, "Language codes file being used: \"%s\"\n", mFileName);
	}
	createLanguageTable(fp);
	fclose(fp);
	return(TRUE);
}

char getLanguageCodeAndName(char *code, char isReplace, char *name) {
	int i, lastBest;
	char found;
	LANGSTABLE *pt;

/*
	if (strncmp(code, "sgn-", 4) == 0) {
		if (name != NULL)
			strcpy(name, "sign language");
		return(TRUE);
	}
*/
	if (code[0] == EOS)
		return(FALSE);
	found = FALSE;
	lastBest = 0;
	if (name != NULL)
		name[0] = EOS;
	for (pt=langs_list; pt != NULL; pt=pt->nextlang) {
		for (i=0; i < 4; i++) {
			if (pt->iso639[i] == NULL)
				break;
			if (strcmp(code, pt->iso639[i]) == 0)
				found = TRUE;
			if (strlen(pt->iso639[i]) == 3 || pt->iso639[i][2] == '-' || pt->iso639[i][3] == '-')
				lastBest = i;
		}
		if (found) {
			if (isReplace && strcmp(code, pt->iso639[lastBest]))
				strcpy(code, pt->iso639[lastBest]);
			if (name != NULL) {
				if (pt->name != NULL) 
					strcpy(name, pt->name);
				else
					return(FALSE);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

void InitLanguagesTable(void) {
	int langCnt;

	defLanguage = 0;
	for (langCnt=0; langCnt < NUMLANGUAGESINTABLE; langCnt++)
		LanguagesTable[langCnt][0] = EOS;
}

void addToLanguagesTable(char *line, char *sp) {
	int langCnt, cnt;
	char *s, *e, t;

	if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE)) {
		langCnt = 0;
		s = line;
		for (; *s == ',' || *s == ' ' || *s == '\t' || *s == '\n'; s++) ;
		e = s;
		while (*s) {
			if (*e == ',' || *e == ' ' || *e == '\t' || *e == '\n') {
				t = *e;
				*e = EOS;
				strncpy(LanguagesTable[langCnt], s, 8);
				LanguagesTable[langCnt][8] = EOS;
				langCnt++;
				if (langCnt >= NUMLANGUAGESINTABLE)
					break;
				*e = t;
				if (*e != EOS) {
					s = e;
					for (; *s == ',' || *s == ' ' || *s == '\t' || *s == '\n'; s++) ;
					e = s;
				} else
					break;
			} else
				e++;
		}
		for (; langCnt < NUMLANGUAGESINTABLE; langCnt++)
			LanguagesTable[langCnt][0] = EOS;
	} else {
		cnt = 0;
		s = line;
		for (; *s == ',' || *s == ' ' || *s == '\t' || *s == '\n'; s++) ;
		e = s;
		while (*s) {
			if (*e == ',' || *e == ' ' || *e == '\t' || *e == '\n') {
				if (cnt > 0) {
					fprintf(stderr,"*** File \"%s\": ", oldfname);
					fprintf(stderr,"line %ld.\n", lineno);
					fprintf(stderr,"%s%s", sp, line);
					if (line[strlen(line)-1] != '\n')
						fputc('\n', stderr);
					fprintf(stderr,"Only one language is allowed on \"@New Language:\" tier.\n");
					if (CLAN_PROG_NUM != CHECK)
#ifdef UNX
						exit(1);
#else
						isKillProgram = 2;
#endif
				}
				cnt++;
				t = *e;
				*e = EOS;
				if (*s == EOS)
					langCnt = NUMLANGUAGESINTABLE;
				else {
					for (langCnt=0; LanguagesTable[langCnt] != EOS && langCnt < NUMLANGUAGESINTABLE; langCnt++) {
						if (!uS.mStricmp(s, LanguagesTable[langCnt])) {
							defLanguage = langCnt;
							break;
						}
					}
				}
				if (langCnt >= NUMLANGUAGESINTABLE) {
					fprintf(stderr,"*** File \"%s\": ", oldfname);
					fprintf(stderr,"line %ld.\n", lineno);
					*e = t;
					fprintf(stderr,"%s%s", sp, line);
					if (line[strlen(line)-1] != '\n')
						fputc('\n', stderr);
					*e = EOS;
					fprintf(stderr,"Language \"%s\" is not defined in \"@Languages:\" header tier.\n", s);
					if (CLAN_PROG_NUM != CHECK)
#ifdef UNX
						exit(1);
#else
						isKillProgram = 2;
#endif
				}
				*e = t;
				if (*e != EOS) {
					s = e;
					for (; *s == ',' || *s == ' ' || *s == '\t' || *s == '\n'; s++) ;
					e = s;
				} else
					break;
			} else
				e++;
		}
	}
	if (defLanguage < 0 || defLanguage >= NUMLANGUAGESINTABLE || LanguagesTable[defLanguage][0] == EOS) {
		fprintf(stderr,"*** File \"%s\": ", oldfname);
		fprintf(stderr,"line %ld.\n", lineno);
		fprintf(stderr,"%s%s", sp, line);
		if (line[strlen(line)-1] != '\n')
			fputc('\n', stderr);
		fprintf(stderr,"No language defined in \"@Languages:\" header tier in %d position.\n", defLanguage);
		if (CLAN_PROG_NUM != CHECK)
#ifdef UNX
			exit(1);
#else
			isKillProgram = 2;
#endif
	}
}
// Languages end

/**************************************************************/
/*	 main loop, argv pars, fname pars					*/
/* chattest(InFname) test the first TESTLIM turns of the input data file to
   make sure that the file is in the CHAT format. If the first part is
   correct it is assumed that the rest of the data is in a good form.

*/
char isIDSpeakerSpecified(char *IDTier, char *code, char isReportErr) {
	char *s, *codeP, isSpSpecified;
	struct tier *p;

	if (IDTier != code)
		strcpy(code, IDTier);
	s = code;
	if ((s=strchr(s, '|')) != NULL) {
		s++;
		if ((s=strchr(s, '|')) != NULL) {
			s++;
			codeP = s;
			if ((s=strchr(s, '|')) != NULL) {
				*s = EOS;
				if (*codeP != '*') {
					code[0] = '*';
					strcpy(code+1, codeP);
				} else {
					strcpy(code, codeP);
				}
				strcat(code, ":");
				isSpSpecified = FALSE;
				for (p=headtier; p != NULL; p=p->nexttier) {
					if (*p->tcode == '*') {
						if (p->include && uS.mStrnicmp(p->tcode,code,strlen(p->tcode)) == 0)
							return(TRUE);
						isSpSpecified = TRUE;
					}
				}
				if (IDField == NULL && SPRole == NULL && isSpSpecified == FALSE)
					return(TRUE);
			} else if (isReportErr) {
				uS.remblanks(code);
				fprintf(stderr,"Malformed @ID tier format: %s. In file %s\n", code, oldfname);
				cutt_exit(1);
			}
		} else if (isReportErr) {
			uS.remblanks(code);
			fprintf(stderr,"Malformed @ID tier format: %s. In file %s\n", code, oldfname);
			cutt_exit(1);
		}
	} else if (isReportErr) {
		uS.remblanks(code);
		fprintf(stderr,"Malformed @ID tier format: %s. In file %s\n", code, oldfname);
		cutt_exit(1);
	}
	return(FALSE);
}

char chattest(FNType *InFname, char isJustCheck, char *isUTF8SymFound) {
					/* chatmode =1 - y switch & errmargin =0;	*/
					/*	   =2 - no y switch & errmargin =0; */
					/*	   =3 - y opt & errmargin = TESTLIM */
					/*	   =4 - no y switch & no testing	*/
	char isFTime, fdata = 0, data, ID_Code_MatchFound = 0;
	char *x;
	long ln, len;
	int i, err = 0, count = 0;

	isFTime = TRUE;
	*isUTF8SymFound = 0;
	ln = 1;
	len = 0;
	templineC4[0] = EOS;
	while ((x=fgets_cr(templineC3, UTTLINELEN, fpin)) != NULL) {
		if (uS.partcmp(templineC3,"@Languages:", FALSE, FALSE)) {
			NewFontInfo finfo;
			char isThaiFound = FALSE;

			for (i=0; templineC3[i] != ':' && templineC3[i] != EOS; i++) ;
			if (templineC3[i] == ':')
				i++;
			for (; isSpace(templineC3[i]); i++) ;
			for (; templineC3[i] != EOS; i++) {
				if (templineC3[i] == 't' && templineC3[i+1] == 'h' &&
					(isSpace(templineC3[i+2]) || templineC3[i+2]== ',' || templineC3[i+2]== EOS || templineC3[i+2]== '\n'))
					isThaiFound = TRUE;
			}
			if (isThaiFound) {
				SetDefaultThaiFinfo(&finfo);
				selectChoosenFont(&finfo, FALSE);
			}
		}
		if (isFTime) {
			if (templineC3[0] == (char)0xef && templineC3[1] == (char)0xbb && templineC3[2] == (char)0xbf) {
				*isUTF8SymFound = 1;
				strcpy(templineC3, templineC3+3);
			} else if ((templineC3[0] == (char)0xff && templineC3[1] == (char)0xfe) || 
					   (templineC3[0] == (char)0xfe && templineC3[1] == (char)0xff)) {
				*isUTF8SymFound = 2;
			}
		}
		isFTime = FALSE;
		for (i=0; templineC3[i]; i++) {
			if (templineC3[i] == ATTMARKER)
				strcpy(templineC3+i, templineC3+i+2);
		}
		i = strlen(templineC3) - 1;
		if (templineC3[i] == '\n') {
			i--;
			ln++;
		} else if (!feof(fpin))
			break;
		for (; i > -1 && isSpace(templineC3[i]); i--) ;
		templineC3[i+1] = EOS;
		data = 0;
		i = 0;

		for (; templineC3[i] != ':' && templineC3[i] != EOS; i++) {
			if (!isSpace(templineC3[i]))
				data = 1;
		}
		if (templineC3[i] == ':') {
			if (isSpace(*templineC3)) {
				if (!fdata)
					break;
			} else if (!isSpeaker(*templineC3) && (!isdigit(templineC3[i-1]) || i == 0))
				break;
			count++;
			fdata = 1;
			err = 0;
		} else if ((data && !fdata) || (data && *templineC3 != ' ' && *templineC3 != '\t')) {
//			uS.uppercasestr(templineC3, &dFnt, MBF);
			if (*templineC3 != '@') {
				if (chatmode != 3 || ++err == TESTLIM)
					break;
			} else
				count++;
			fdata = 1;
		}
		if (isSpeaker(*templineC3)) {
			if (IDField != NULL || CODEField != NULL) {
				if (IDField != NULL) {
					if (strncmp(templineC4, "@ID:", 4) == 0) {
						for (i=4; isSpace(templineC4[i]); i++) ;
						if (CheckIDNumber(templineC4+i, templineC))
							ID_Code_MatchFound = (ID_Code_MatchFound | 1);
						else if (isIDSpeakerSpecified(templineC4+i, templineC4+i, FALSE))
							ID_Code_MatchFound = (ID_Code_MatchFound | 1);
					}
				}
				if (CODEField != NULL) {
					if (strncmp(templineC4, "@Keywords:", 10) == 0) {
						for (i=6; isSpace(templineC4[i]); i++) ;
						if (CheckCodeNumber(templineC4+i))
							ID_Code_MatchFound = (ID_Code_MatchFound | 2);
					} 
				}
				strcpy(templineC4, templineC3);
				len = strlen(templineC3);
			}
			if (isMainSpeaker(templineC3[0])) {
				if (count > 0)
					count = TESTLIM;
				break;
			}
		} else {
			if (IDField != NULL || CODEField != NULL) {
				len = len + strlen(templineC3);
				if (len >= UTTLINELEN) {
					count = 0;
					err = TESTLIM;
					break;
				} else
					strcat(templineC4, templineC3);
			}
		}
	}
	if ((count < TESTLIM && x != NULL) || err >= TESTLIM) {
		if (!isJustCheck) {
			fprintf(stderr,"*** File \"%s\": line %ld.\n", InFname, ln);			
			fprintf(stderr, "ERROR: File is NOT in a proper CHAT format.\n%s\n", templineC3);
			if (option_flags[CLAN_PROG_NUM] & Y_OPTION) {
				fprintf(stderr, "Use +y option if the file is NOT supposed to be in CHAT format.\n");
				fprintf(stderr,"Otherwise fix the error!\n");
			}
			cutt_exit(0);
		} else
			return(FALSE);
	}
	if (!isJustCheck) {
		if (IDField != NULL && !(ID_Code_MatchFound & 1)) {
			return(FALSE);
		} else if (CODEField != NULL) {
			if (ID_Code_MatchFound & 2)
				return(tcode);
			else
				return(!tcode);
		}
	}
	return(TRUE);
}

#ifndef UNX
int wpathcmp(const wchar_t *path1, const wchar_t *path2) {
	for (; towupper(*path1) == towupper(*path2) && *path2 != EOS; path1++, path2++) ;
	if (*path1 == EOS && *path2 == PATHDELIMCHR && *(path2+1) == EOS)
		return(0);
	else if (*path1 == PATHDELIMCHR && *(path1+1) == EOS && *path2 == EOS)
		return(0);
	else if (*path1 == EOS && *path2 == EOS)
		return(0);
	else if (towupper(*path1) > towupper(*path2))
		return(1);
	else
		return(-1);	
}
#endif

int pathcmp(const FNType *path1, const FNType *path2) {
	for (; toupper(*path1) == toupper(*path2) && *path2 != EOS; path1++, path2++) ;
	if (*path1 == EOS && *path2 == PATHDELIMCHR && *(path2+1) == EOS)
		return(0);
	else if (*path1 == PATHDELIMCHR && *(path1+1) == EOS && *path2 == EOS)
		return(0);
	else if (*path1 == EOS && *path2 == EOS)
		return(0);
	else if (toupper(*path1) > toupper(*path2))
		return(1);
	else
		return(-1);	
}

void addFilename2Path(FNType *path, const FNType *filename) {
	int len;

	len = strlen(path);
	if (path[len-1] != PATHDELIMCHR)
		path[len++] = PATHDELIMCHR;
	if (filename[0] == PATHDELIMCHR)
		strcpy(path+len, filename+1);
	else
		strcpy(path+len, filename);
}

char extractPath(FNType *mDirPathName, FNType *file_path) {
	FNType *p;

	strcpy(mDirPathName, file_path);
	p = strrchr(mDirPathName, PATHDELIMCHR);
	if (p != NULL) {
		*(p+1) = EOS;
		return(TRUE);
	} else {
//		mDirPathName[0] = EOS;
		return(FALSE);
	}
}

void extractFileName(FNType *FileName, FNType *file_path) {
	FNType *p;

	p = strrchr(file_path, PATHDELIMCHR);
	if (p != NULL) {
		strcpy(FileName, p+1);
	} else
		strcpy(FileName, file_path);
}

/* parsfname(old,fname,fext) take the file name in "old" removes, if it
   exists, an extention from the end of it. Then it removes the full path, if
   it exists. Then it copies the remainder to the "fname" and concatanate the
   extention "ext" to the end of string in "fname".
*/
void parsfname(FNType *oldO, FNType *fname, const char *fext) {
	long i, ti, j;
	FNType *old, oldS[FNSize];

	if (strlen(oldO) < FNSize-1) {
		strcpy(oldS, oldO);
		old = oldS;
	} else
		old = oldO;
	i = strlen(old) - 1;
	while (i >= 0L && (old[i] == ' ' || old[i] == '\t'))
		i--;
	old[++i] = EOS;
	while (i > 0L && old[i] != '.' && old[i] != PATHDELIMCHR)
		i--;
	ti = 0L;
	if (old[i] == PATHDELIMCHR) {
		if (!Preserve_dir)
			ti = i+1;
		i = strlen(old);
	} else if (old[i] == '.') {
		if (old[i-1] != PATHDELIMCHR) {
			if (!Preserve_dir) {
				ti = i;
				while (ti > 0L && old[ti] != PATHDELIMCHR) ti--;
				if (old[ti] == PATHDELIMCHR) ti++;
			}
		} else {
			if (!Preserve_dir)
				ti = i;
			i = strlen(old);
		}
	} else {
		ti = 0L;
		i = strlen(old);
	}
	j = 0L;
#ifndef UNX
	if (WD_Not_Eq_OD) {
		strcpy(fname, od_dir);
		j = strlen(fname);
		if (j > 0 && fname[j-1] != PATHDELIMCHR)
			fname[j++] = PATHDELIMCHR;
	}
#endif
	while (ti < i)
		fname[j++] = old[ti++];
	fname[j] = EOS;
#ifndef UNX
	if (AddCEXExtension[0] == EOS) {
		for (i=strlen(fname)-1; i > 0L && fname[i] != '.' && fname[i] != PATHDELIMCHR; i--) ;
		if (fname[i] == '.' && isCHATFile(fname+i) && uS.mStricmp(fname+i, fext)) {
			fname[i] = EOS;
			j = i;
		}
	}
#endif
	uS.str2FNType(fname, j, fext);
}

/* openwfile(oldfname,fname,fp) opens file "fname" for writing while trying to
   maintain version numbers of output files. The limit is 10 versions as of
   now.
*/
FILE *openwfile(const FNType *oldfname, FNType *fname, FILE *fp) {
	register char res;
	register int i = 0;
	register int len;

	if (!FirstTime && combinput)
		return(fp);
	len = strlen(fname);
	if (AddCEXExtension[0] != EOS)
		uS.str2FNType(fname, strlen(fname), AddCEXExtension);
	if (!OverWriteFile) {
		for (i=0; i < 100 && !access(fname,0); i++) {
			sprintf(fname+len, "%d", i);
			if (AddCEXExtension[0] != EOS)
				uS.str2FNType(fname, strlen(fname), AddCEXExtension);
		}
		if (i >= 100) {
			strcpy(fname+len, "?: Too many versions.");
			return(NULL);
		}
	}
	if (!uS.mStricmp(oldfname, fname))
		uS.str2FNType(fname, strlen(fname), ".1");
	fp = fopen(fname,"w");
#ifdef _MAC_CODE
	res = isCHATFile(fname);
	if (isCEXFile(fname) || (res != 0 && res != '\005'))
		settyp(fname, 'TEXT', the_file_creator.out, FALSE);
#endif
	return(fp);
}

int IsOutTTY(void) {
#if defined(CLAN_SRV)
	if (stout)
		return(1);
#endif
#if defined(_MAC_CODE) || defined(_WIN32)
	return(isatty(stdout));
#endif
#ifdef UNX
#ifdef USE_TERMIO
	struct termio otty;
	if (ioctl(fileno(stdout), TCGETA, &otty) == -1) {
		if (errno == ENOTTY)
			return(0);
	}
#elif defined(APPLEUNX)
	struct sgttyb otty;
	return(ioctl(fileno(stdout), TIOCGETP, &otty) == 0);
#else
	struct sgttyb otty;
	return(gtty(fileno(stdout), &otty) == 0);
#endif
#endif
	return(1);
}

/* help function to CurTierSearch() function.
*/
char listtcs(char todo, char code) {
	struct tier *p;
	char f = TRUE;

	if (todo == '\0' || todo == '\001') {
		for (p=defheadtier; p != NULL; p=p->nexttier) {
			if ((p->include || todo) && *p->tcode == code && !p->tcode[1])
				return('\004');
			else if (*p->tcode == code)
				f = FALSE;
		}
		for (p=headtier; p != NULL; p=p->nexttier) {
			if ((p->include || todo) && *p->tcode == code && !p->tcode[1])
				return('\004');
			else if (*p->tcode == code)
				f = FALSE;
		}
		if (SPRole != NULL) {
			if (code == '*')
				f = FALSE;
		}
		if (todo == '\001' && f)
			return('\003');
	} else
	if (todo == '\004') {
		for (p=defheadtier; p != NULL; p=p->nexttier) {
			if (p->include == 0 && *p->tcode == code) {
				if (f) {
					f = FALSE;
					if (FirstTime)
						fputs(" EXCEPT the ones matching:",stderr);
					if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
						fputs(" EXCEPT the ones matching:", fpout);
				}
				if (FirstTime)
					fprintf(stderr," %s;",p->tcode);
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
					fprintf(fpout," %s;",p->tcode);
			}
		}
		for (p=headtier; p != NULL; p=p->nexttier) {
			if (p->include == 0 && *p->tcode == code) {
				if (f) {
					f = FALSE;
					if (FirstTime)
						fputs(" EXCEPT the ones matching:",stderr);
					if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
						fputs(" EXCEPT the ones matching:",fpout);
				}
				if (FirstTime)
					fprintf(stderr," %s;",p->tcode);
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
					fprintf(fpout," %s;",p->tcode);
			}
		}
	} else {
		for (p=defheadtier; p != NULL; p=p->nexttier) {
			if ((p->include || todo == '\003') && *p->tcode == code) {
				if (FirstTime)
					fprintf(stderr," %s;",p->tcode);
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
					fprintf(fpout," %s;",p->tcode);
			}
		}
		for (p=headtier; p != NULL; p=p->nexttier) {
			if ((p->include || todo == '\003') && *p->tcode == code) {
				if (FirstTime)
					fprintf(stderr," %s;",p->tcode);
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
					fprintf(fpout," %s;",p->tcode);
			}
		}
	}
	return('\002');
}

/* CurTierSearch(pn) displays the list of all tiers that would be
   included/excluded from the analyses. "pn" is a current program name.
*/
void CurTierSearch(char *pn) {
	int i;
	char t;
	time_t timer;
	struct IDtype *tID;

	if (onlydata >= 10)
		return;
	time(&timer);
	if (FirstTime) {
		fprintf(stderr,"%s", ctime(&timer));
	}
	if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData) {
		fprintf(fpout,"%s", ctime(&timer));
	}
	for (i=strlen(pn)-1; i >= 0 && pn[i] != ':' && pn[i] != PATHDELIMCHR; i--) ;
	if (FirstTime) {
		fprintf(stderr,"%s", pn+i+1);
		VersionNumber(TRUE, stderr);
		if (chatmode)
			fprintf(stderr," is conducting analyses on:\n");
		else
			fprintf(stderr,"\n");
	}
	if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData) {
		fprintf(fpout,"%s", pn+i+1);
		VersionNumber(TRUE, fpout);
		if (chatmode)
			fprintf(fpout," is conducting analyses on:\n");
		else
			fprintf(fpout,"\n");
	}
	if (!chatmode) {
		if (FirstTime)
			fputs("****************************************\n",stderr);
		if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
			fputs("****************************************\n",fpout);
		return;
	}
	i = FALSE;

	if (!nomain) {
		if (FirstTime)
			fputs("  ",stderr);
		if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
			fputs("  ", fpout);
		if (tcs) {
			if ((t=listtcs('\0','*')) == 4) {
				if (FirstTime) {
					if (IDField != NULL)
						fputs("ALL speaker main tiers whose IDs",stderr);
					else if (SPRole != NULL)
						fputs("ALL speaker main tiers whose role(s):",stderr);
					else
						fputs("ALL speaker main tiers",stderr);
				}
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData) {
					if (IDField != NULL)
						fputs("ALL speaker main tiers whose IDs",fpout);
					else if (SPRole != NULL)
						fputs("ALL speaker main tiers whose role(s):",fpout);
					else
						fputs("ALL speaker main tiers",fpout);
				}
			} else {
				if (FirstTime) {
					if (IDField != NULL)
						fputs("ONLY speaker main tiers with IDs matching:",stderr);
					else if (SPRole != NULL)
						fputs("ONLY speaker main tiers with role(s):",stderr);
					else
						fputs("ONLY speaker main tiers matching:",stderr);
				}
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData) {
					if (IDField != NULL)
						fputs("ONLY speaker main tiers with IDs matching:",fpout);
					else if (SPRole != NULL)
						fputs("ONLY speaker main tiers with role(s):",fpout);
					else
						fputs("ONLY speaker main tiers matching:",fpout);
				}
			}
			for (tID=SPRole; tID != NULL; tID=tID->next_ID) {
				if (FirstTime)
					fprintf(stderr," %s;", tID->ID);
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
					fprintf(fpout," %s;", tID->ID);
			}
			for (tID=IDField; tID != NULL; tID=tID->next_ID) {
				if (FirstTime)
					fprintf(stderr," %s;", tID->ID);
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
					fprintf(fpout," %s;", tID->ID);
			}
			listtcs(t,'*');
			i = TRUE;
		} else
		if ((t=listtcs('\001','*')) != 4) {
			if (t == 3) {
				if (FirstTime) {
					if (IDField != NULL)
						fputs("ALL speaker tiers with IDs",stderr);
					else if (SPRole != NULL)
						fputs("ALL speaker tiers with role(s):",stderr);
					else
						fputs("ALL speaker tiers",stderr);
				}
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData) {
					if (IDField != NULL)
						fputs("ALL speaker tiers with IDs",fpout);
					else if (SPRole != NULL)
						fputs("ALL speaker tiers with role(s):",fpout);
					else
						fputs("ALL speaker tiers",fpout);
				}
			} else {
				if (FirstTime) {
					if (IDField != NULL)
						fputs("ALL speaker main tiers EXCEPT the ones with IDs matching:",stderr);
					else if (SPRole != NULL)
						fputs("ALL speaker main tiers EXCEPT the ones with role(s):",stderr);
					else
						fputs("ALL speaker main tiers EXCEPT the ones matching:",stderr);
				}
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData) {
					if (IDField != NULL)
						fputs("ALL speaker main tiers EXCEPT the ones with IDs matching:",fpout);
					else if (SPRole != NULL)
						fputs("ALL speaker main tiers EXCEPT the ones with role(s):",fpout);
					else
						fputs("ALL speaker main tiers EXCEPT the ones matching:",fpout);
				}
				listtcs('\003','*');
			}
			for (tID=SPRole; tID != NULL; tID=tID->next_ID) {
				if (FirstTime)
					fprintf(stderr," %s;", tID->ID);
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
					fprintf(fpout," %s;", tID->ID);
			}
			for (tID=IDField; tID != NULL; tID=tID->next_ID) {
				if (FirstTime)
					fprintf(stderr," %s;", tID->ID);
				if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
					fprintf(fpout," %s;", tID->ID);
			}
			i = TRUE;
		}
	}

	if (tct) {
		if (nomain) {
			if (FirstTime) fputs("  ",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("  ",fpout);
		} else {
			if (FirstTime)
				fprintf(stderr,"\n	and those speakers' ");
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fprintf(fpout,"\n    and those speakers' ");
		}
		if ((t=listtcs('\0','%')) == 4) {
			if (FirstTime)
				fputs("ALL dependent tiers",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("ALL dependent tiers",fpout);
		} else {
			if (FirstTime)
				fputs("ONLY dependent tiers matching:",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("ONLY dependent tiers matching:",fpout);
		}
		listtcs(t,'%');
		i = TRUE;
	} else
	if ((t=listtcs('\001','%')) != 4) {
		if (nomain) {
			if (FirstTime)
				fputs("  ",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("  ",fpout);
		} else {
			if (FirstTime)
				fprintf(stderr,"\n    and those speakers' ");
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fprintf(fpout,"\n    and those speakers' ");
		}
		if (t == 3) {
			if (FirstTime)
				fputs("ALL dependent tiers",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("ALL dependent tiers",fpout);
		} else {
			if (FirstTime)
				fputs("ALL dependent tiers EXCEPT the ones matching:",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("ALL dependent tiers EXCEPT the ones matching:",fpout);
			listtcs('\003','%');
		}
		i = TRUE;
	}

	if (tch) {
		if (i) {
			if (FirstTime)
				fputs("\n  and ",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("\n  and ",fpout);
		}
		if ((t=listtcs('\0','@')) == 4) {
			if (FirstTime)
				fputs("ALL header tiers",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("ALL header tiers",fpout);
		} else {
			if (FirstTime)
				fputs("ONLY header tiers matching:",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("ONLY header tiers matching:",fpout);
		}
		listtcs(t,'@');
		i = TRUE;
	} else
	if ((t=listtcs('\001','@')) != 4) {
		if (i) {
			if (FirstTime)
				fputs("\n  and ",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				 fputs("\n  and ",fpout);
		}
		if (t == 3) {
			if (FirstTime)
				fputs("ALL header tiers",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("ALL header tiers",fpout);
		} else {
			if (FirstTime)
				fputs("ALL header tiers EXCEPT the ones matching:",stderr);
			if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
				fputs("ALL header tiers EXCEPT the ones matching:",fpout);
			listtcs('\003','@');
		}
		i = TRUE;
	}

	if (!i)
		fputs("NO DATA!!!",stderr);
	if (FirstTime)
		fputs("\n****************************************\n",stderr);
	if ((!stout || !IsOutTTY()) && (!onlydata || puredata < 2) && !outputOnlyData)
		fputs("\n****************************************\n",fpout);
}

void printArg(char *argv[], int argc, FILE *fp, char specialCase, FNType *fname) {
	int  i;
	char *st, isSpf, *tst, isDQf;

	for (i=strlen(argv[0])-1; i >= 0 && argv[0][i] != ':' && argv[0][i] != PATHDELIMCHR; i--) ;
	fprintf(fp,"%s", argv[0]+i+1);
	for (i=1; i < argc; i++) {
		isDQf = FALSE;
		isSpf = FALSE;
		st = argv[i];
		if (!specialCase || argv[i][0] == '+' || argv[i][0] == '-' || argv[i][0] == '"' || combinput || !uS.FNTypecmp(fname, argv[i], 0L)) {
			for (tst=st; *tst != EOS; tst++) {
				if (*tst == ' ' || *tst == '\t' || *tst == '|' || *tst == '<' || *tst == '>') {
					isSpf = TRUE;
				} else if ((*tst == '"' || *tst == '\'') && (tst == st || *(tst-1) != '\\'))
					isSpf = TRUE;
				if (*tst == '"')
					isDQf = TRUE;
			}
			if (isSpf) {
				fputc(' ', fp);
				if (*st == '+' || *st == '-') {
					fputc(*st, fp);
					st++;
					fputc(*st, fp);
					st++;
					if (*st == '+') {
						fputc(*st, fp);
						st++;
					}
					if (*st == '@') {
						fputc(*st, fp);
						st++;
					}
				}
				if (isDQf)
					fputc('\'', fp);
				else
					fputc('"', fp);
				for (; *st != EOS; st++)
					fputc(*st, fp);
				if (isDQf)
					fputc('\'', fp);
				else
					fputc('"', fp);
			} else
				fprintf(fp," %s", st);
		}
	}
	putc('\n', fp);
}

/* work(pn,st) opens apropriate input/output streams, figures out output
   file name, if any, and calls the program local "call()" function. It also
   calls functions to test the data format and to display list of user
   specified tier. It returns 2 if input file can not be opened and 1
   otherwise.
*/
static int work(char *argv[], int argc, FNType *fname) {
	FNType *showFName;
	char isUTF8SymFound;
	UInt32 dateValue = 0L;
#ifdef _MAC_CODE
	creator_type the_file_type, the_file_creator;
#endif

#ifndef UNX
	copyNewFontInfo(&dFnt, &oFnt);
	copyNewToFontInfo(&global_df->row_txt->Font, &dFnt);
#endif
	foundUttContinuation = FALSE;
	cMediaFileName[0] = EOS;
	oldfname = fname;
	if (utterance != NULL) {
		UTTER *ttt = utterance;

		do {
			ttt->speaker[0]	= EOS;
			ttt->attSp[0]	= EOS;
			ttt->line[0]	= EOS;
			ttt->attLine[0]	= EOS;
			ttt = ttt->nextutt;
		} while (ttt != utterance) ;
	}
	getc_cr_lc = '\0';
	fgets_cr_lc = '\0';
	contSpeaker[0] = EOS;
	Tspchanged = FALSE;

	if (isLanguageExplicit)
		InitLanguagesTable();

	cutt_isMultiFound = FALSE;
	cutt_isCAFound = FALSE, 
	cutt_isBlobFound = FALSE;
	cutt_depWord = FALSE;
	init('\0');
	if (TSoldsp != NULL)
		*TSoldsp = EOS;
	if (!combinput)
		WUCounter = 0L;
	else if (CLAN_PROG_NUM == QUOTES || CLAN_PROG_NUM == VOCD || CLAN_PROG_NUM == EVAL || CLAN_PROG_NUM == MORTABLE ||
			 CLAN_PROG_NUM == COMBO || CLAN_PROG_NUM == SCRIPT_P || CLAN_PROG_NUM == KIDEVAL || CLAN_PROG_NUM == TIMEDUR ||
			 (CLAN_PROG_NUM == MLT && onlydata == 1) || (CLAN_PROG_NUM == MLU && onlydata == 1))
		WUCounter = 0L;
	if (lineno > -1L) {
		tlineno = 1L;
		lineno = deflineno;
	}
	if (!stin) {
		if (access(fname,0)) {
		   fprintf(stderr, "Can't locate file \"%s\" in a specified directory.\n", fname);
		   fprintf(stderr,"Check spelling of file name.\n");
		   return(2);
		}
		if ((fpin=fopen(fname, "r")) == NULL) {
		   fprintf(stderr,"Can't open file %s.\n",fname);
		   return(2);
		}
	} else {
		fpin = stdin;
	}

#if defined(CLAN_SRV)
	stout = TRUE;
#endif
	if (stout) {
		fpout = stdout;

		if (FirstTime)
//			getpunct(punctFile);
#if !defined(CLAN_SRV)
		if (FirstTime) {
			printArg(argv, argc, stderr, FALSE, fname);
			if (!IsOutTTY() && (!onlydata || puredata < 2) && !outputOnlyData) {
				printArg(argv, argc, fpout, FALSE, fname);
			}
		}
#endif // !defined(CLAN_SRV)
#if defined(UNX)
		if (!stin)
#endif
		if (chatmode != 0 && chatmode != 4) {
			if (!chattest(fname, FALSE, &isUTF8SymFound)) {
				if (fpin != NULL && fpin != stdin) {
					fclose(fpin);
					fpin = NULL;
				}
				FirstTime = FALSE;
				return(0);
			}
			rewind(fpin);
			if (isUTF8SymFound == 1) {
				getc(fpin); getc(fpin); getc(fpin);
			} else if (isUTF8SymFound == 2) {
				fprintf(stderr, "\n\nCLAN can't read UTF-16 encoded files: %s.\n\n", fname);
				return(2);
			} else {
			}
		} else if (!stin) {
			int c;
			isUTF8SymFound = FALSE;
			c = getc(fpin);
			if (c == (int)0xef) {
				c = getc(fpin);
				if (c == (int)0xbb) {
					c = getc(fpin);
					if (c == (int)0xbf) {
						isUTF8SymFound = TRUE;
					}
				}
			} else if (c == (int)0xff) {
				c = getc(fpin);
				if (c == (int)0xfe) {
					fprintf(stderr, "\n\nCLAN can't read UTF-16 encoded files: %s.\n\n", fname);
					return(2);
				}
			} else if (c == (int)0xfe) {
				c = getc(fpin);
				if (c == (int)0xff) {
					fprintf(stderr, "\n\nCLAN can't read UTF-16 encoded files: %s.\n\n", fname);
					return(2);
				}
			}
			if (!isUTF8SymFound)
				rewind(fpin);
		}
		if (FirstTime)
			CurTierSearch(argv[0]);

		if (stin) {
			if (FirstTime) {
				fprintf(stderr,"From pipe input\n");
				if (!IsOutTTY() && (!onlydata || !puredata) && !outputOnlyData)
					fprintf(stdout,"From pipe input\n");
				}
			do {
				call();
				if (ByTurn < 2)
					break;
				if (isKillProgram)
					break;
				ByTurn = 3;
				init('\0');
				fprintf(fpout, "############### TURN BREAK ###############\n");
			} while (1) ;
		} else {
			if (CLAN_PROG_NUM != POST && CLAN_PROG_NUM != IMDI_P && CLAN_PROG_NUM != RELY) {
#if defined(CLAN_SRV)
				FNType *s;
				s = strrchr(fname, '/');
				if (s != NULL)
					s++;
				else
					s = fname;
				fprintf(stdout,"From file \"%s\"\n", s);
#else
				if (!stin_override) {
					if (CLAN_PROG_NUM != EVAL && CLAN_PROG_NUM != KIDEVAL) {
						fprintf(stderr,"From file <%s>\n",fname);
					}
				} else
					fprintf(stderr,"From file \"%s\"\n",fname);
				if (!IsOutTTY() && (!onlydata || !puredata) && !outputOnlyData) {
					if (!stin_override)
						fprintf(stdout,"From file <%s>\n",fname);
					else
						fprintf(stdout,"From file \"%s\"\n",fname);
				}
#endif
			}
			do {
				call();
				if (ByTurn < 2)
					break;
				if (isKillProgram)
					break;
				ByTurn = 3;
				init('\0');
				fprintf(fpout, "############### TURN BREAK ###############\n");
			} while (1) ;
			if (fpin != NULL && fpin != stdin) {
				fclose(fpin);
				fpin = NULL;
			}
		}
		FirstTime = FALSE;
	} else { /* else if (stout) { */
		if (FirstTime || !combinput)
			parsfname(fname,newfname,GExt);
		if ((fpout=openwfile(fname,newfname,fpout)) == NULL) {
			fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",newfname);
			if (fpin != NULL && fpin != stdin) {
				fclose(fpin);
				fpin = NULL;
			}
			return(2);
		} else {
//			if (FirstTime || !combinput)
//				getpunct(punctFile);
			if (!combinput && replaceFile)
				showFName = fname;
			else
				showFName = newfname;
#if !defined(CLAN_SRV)
			if (FirstTime) {
				printArg(argv, argc, stderr, FALSE, fname);
			}
#endif // !defined(CLAN_SRV)
			if (FirstTime || !combinput) {
				if ((!onlydata || puredata < 2) && !outputOnlyData) {
					printArg(argv, argc, fpout, TRUE, fname);
				}
#if defined(UNX)
				if (!stin)
#endif
				if (chatmode != 0 && chatmode != 4) {
					if (!chattest(fname, FALSE, &isUTF8SymFound)) {
						if (fpin != NULL && fpin != stdin) {
							fclose(fpin);
							fpin = NULL;
						}
						if (!combinput) {
							fclose(fpout);
							fpout = NULL;
						}
						if (delSkipedFile != NULL && !replaceFile && fpout != stdout && !combinput) {
							fprintf(stderr, "%s    \"%s\"", delSkipedFile, oldfname);
							if (unlink(newfname))
								fprintf(stderr, "Can't delete output file \"%s\".", newfname);
						}
						isFileSkipped = TRUE;
						FirstTime = FALSE;
						return(0);
					}
					rewind(fpin);
					if (isUTF8SymFound == 1) {
						getc(fpin); getc(fpin); getc(fpin);
					} else if (isUTF8SymFound == 2) {
						fprintf(stderr, "\n\nCLAN can't read UTF-16 encoded files: %s.\n\n", fname);
						return(2);
					} else {
					}
				} else {
					int c;

					isUTF8SymFound = FALSE;
					c = getc(fpin);
					if (c == (int)0xef) {
						c = getc(fpin);
						if (c == (int)0xbb) {
							c = getc(fpin);
							if (c == (int)0xbf) {
								isUTF8SymFound = TRUE;
							}
						}
					} else if (c == (int)0xff) {
						c = getc(fpin);
						if (c == (int)0xfe) {
							fprintf(stderr, "\n\nCLAN can't read UTF-16 encoded files: %s.\n\n", fname);
							return(2);
						}
					} else if (c == (int)0xfe) {
						c = getc(fpin);
						if (c == (int)0xff) {
							fprintf(stderr, "\n\nCLAN can't read UTF-16 encoded files: %s.\n\n", fname);
							return(2);
						}
					}
					if (!isUTF8SymFound)
						rewind(fpin);
				}
				CurTierSearch(argv[0]);
			} else {
#if defined(UNX)
				if (!stin)
#endif
				if (chatmode != 0 && chatmode != 4) {
					if (!chattest(fname, FALSE, &isUTF8SymFound)) {
						if (fpin != NULL && fpin != stdin) {
							fclose(fpin);
							fpin = NULL;
						}
						if (!combinput)
							fclose(fpout);
						FirstTime = FALSE;
						return(0);
					}
					rewind(fpin);
					if (isUTF8SymFound == 1) {
						getc(fpin); getc(fpin); getc(fpin);
					} else if (isUTF8SymFound == 2) {
						fprintf(stderr, "\n\nCLAN can't read UTF-16 encoded files: %s.\n\n", fname);
						return(2);
					}
				}
			}

			if (stin) {
				if ((!onlydata || !puredata) && !outputOnlyData) {
					fprintf(fpout,"From file <%s>\n",fname);
				}
				fprintf(stderr,"From file <%s>\n",fname);
				do {
					call();
					if (ByTurn < 2)
						break;
					if (isKillProgram)
						break;
					ByTurn = 3;
					init('\0');
					fprintf(fpout, "############### TURN BREAK ###############\n");
				} while (1) ;
				if (!combinput)
					fprintf(stderr,"Output file <%s>\n",showFName);
			} else {
				if ((!onlydata || !puredata) && !outputOnlyData) {
					if (!stin_override)
						fprintf(fpout,"From file <%s>\n",fname);
					else
						fprintf(fpout,"From file \"%s\"\n",fname);
				}
				if (CLAN_PROG_NUM != RELY && CLAN_PROG_NUM != EVAL && CLAN_PROG_NUM != KIDEVAL)
					fprintf(stderr,"From file <%s>\n",fname);
				do {
					call();
					if (ByTurn < 2)
						break;
					if (isKillProgram)
						break;
					ByTurn = 3;
					init('\0');
					fprintf(fpout, "############### TURN BREAK ###############\n");
				} while (1) ;
				if (!combinput && CLAN_PROG_NUM != RELY) {
					if (!stin_override)
						fprintf(stderr,"Output file <%s>\n",showFName);
					else
						fprintf(stderr,"Output file \"%s\"\n",showFName);
				}
				if (fpin != NULL && fpin != stdin) {
					fclose(fpin);
					fpin = NULL;
				}
			}
			FirstTime = FALSE;
			if (!combinput) {
				if (fpout)
					fclose(fpout);
				fpout = NULL;
				if ((option_flags[CLAN_PROG_NUM] & FR_OPTION) && replaceFile && isKillProgram == 0) {
					if (PreserveFileTypes) {
#ifdef _MAC_CODE
						gettyp(fname, &the_file_type.out, &the_file_creator.out);
#endif
#ifndef UNX
						getFileDate(fname,&dateValue);
#endif
					}
					if (unlink(fname)) {
#ifndef UNX
						sprintf(templineC3, "Can't delete original file \"%s\". Perhaps it is opened by some application.", fname);
					   	do_warning(templineC3, 0);
#else
						fprintf(stderr, "Can't delete original file \"%s\". Perhaps it is opened by some application.", fname);
#endif
					   	return(2);
					}

#ifndef UNX
					if (rename_each_file(newfname, fname, FALSE) == -1) {
						sprintf(templineC3, "Can't rename original file \"%s\". Perhaps it is opened by some application.", fname);
						do_warning(templineC3, 0);
						return(2);
					}
#else
					rename(fname, newfname);
#endif
					if (PreserveFileTypes) {
#ifdef _MAC_CODE
						settyp(fname, the_file_type.out, the_file_creator.out, TRUE);
#endif
#ifndef UNX
						setFileDate(fname,dateValue);
#endif
					}
				} else if (PreserveFileTypes) {
#ifdef _MAC_CODE
					gettyp(fname, &the_file_type.out, &the_file_creator.out);
#endif
#ifndef UNX
					getFileDate(fname,&dateValue);
#endif
#ifdef _MAC_CODE
					settyp(newfname, the_file_type.out, the_file_creator.out, TRUE);
#endif
#ifndef UNX
					setFileDate(newfname,dateValue);
#endif
				}
			}
		}
	}
	if (IDField != NULL || CODEField != NULL || SPRole != NULL)
		CleanUpTempIDSpeakers();
	return(0);
}

int Get_Dir(FNType *dirname, int index) {
#if defined(UNX) || defined(_MAC_CODE)
	struct dirent *dp;
	struct stat sb;

	if (index == 1) {
		if ((cDIR=opendir(".")) == NULL) {
			return(0);
		}
	}
	while ((dp=readdir(cDIR)) != NULL) {
		if (stat(dp->d_name, &sb) == 0) {
			if (!S_ISDIR(sb.st_mode)) {
				continue;
			}
		} else
			continue;
		strcpy(dirname, dp->d_name);
		if (!strcmp(dirname, ".") || !strcmp(dirname, ".."))
			continue;
		return(2);
	}
	closedir(cDIR);
	return(0);
#endif // defined(UNX) || defined(_MAC_CODE)
#ifdef _WIN32
	CString fname;

	if (index == 1) {
		if (!fileFind.FindFile(_T("*.*"), 0)) {
			fileFind.Close();
			return(0);
		}
	} else if (index == 3) {
		fileFind.Close();
		return(0);
	}

	while (fileFind.FindNextFile()) {
		if (!fileFind.IsDirectory())
			continue;
		fname = fileFind.GetFileName();
		u_strcpy(dirname, fname, FILENAME_MAX);
		if (!strcmp(dirname, ".") || !strcmp(dirname, ".."))
			continue;
		return(2);
	}
	if (fileFind.IsDirectory()) {
		fname = fileFind.GetFileName();
		u_strcpy(dirname, fname, FILENAME_MAX);
		if (strcmp(dirname, ".") && strcmp(dirname, ".."))
			return(3);
	}
	fileFind.Close();
	return(0);
#endif /* _WIN32 */
}

int Get_File(FNType *filename, int index) {
#if defined(UNX) || defined(_MAC_CODE)
	struct dirent *dp;
	struct stat sb;

	if (index == 1) {
		if ((cDIR=opendir(".")) == NULL) {
			return(0);
		}
	}
	while ((dp=readdir(cDIR)) != NULL) {
		if (stat(dp->d_name, &sb) == 0) {
			if (S_ISDIR(sb.st_mode)) {
				continue;
			}
		}
		if (dp->d_name[0] == '.')
			continue;
		strcpy(filename, dp->d_name);
		return(2);
	}
	closedir(cDIR);
	return(0);
#endif // defined(UNX) || defined(_MAC_CODE)
#ifdef _WIN32
	CString fname;

	if (index == 1) {
		if (!fileFind.FindFile(_T("*.*"), 0)) {
			fileFind.Close();
			return(0);
		}
	} else if (index == 3) {
		fileFind.Close();
		return(0);
	}
	while (fileFind.FindNextFile()) {
		if (fileFind.IsDirectory())
			continue;
		fname = fileFind.GetFileName();
		if (fname[0] == '.')
			continue;
		u_strcpy(filename, fname, FILENAME_MAX);
		return(2);
	}
	if (!fileFind.IsDirectory()) {
		fname = fileFind.GetFileName();
		if (fname[0] != '.') {
			u_strcpy(filename, fname, FILENAME_MAX);
			return(3);
		}
	}
	fileFind.Close();
	return(0);
#endif /* _WIN32 */
}

#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
/* find files recursively or otherwise */
struct cutt_FileList {
	FNType fname[FNSize];
	struct cutt_FileList *next_file;
} ;
typedef struct cutt_FileList cutt_FileList;

struct cutt_opts {
	FNType *path;
	FNType *targ;
	char **argv;
	int  argc;
	cutt_FileList *root_file;
} ;
typedef struct cutt_opts cutt_opts;

static void free_FileList(cutt_FileList *p) {
	cutt_FileList *t;

	while (p != NULL) {
		t = p;
		p = p->next_file;
		free(t);
	}
}

static char cutt_addToFileList(cutt_opts *args, FNType *fname) {
	cutt_FileList *tF, *t, *tt;

	tF = NEW(cutt_FileList);
	if (tF == NULL) {
#ifndef UNX
		if (MEMPROT)
			free(MEMPROT);
		do_warning("Out of memory", 0);
#else
		fprintf(stderr, "Out of memory");
#endif
		return(FALSE);
	}
	if (args->root_file == NULL) {
		args->root_file = tF;
		tF->next_file = NULL;
	} else if (strcmp(args->root_file->fname, fname) > 0) {
		tF->next_file = args->root_file;
		args->root_file = tF;
	} else {
		t = args->root_file;
		tt = args->root_file->next_file;
		while (tt != NULL) {
			if (strcmp(tt->fname, fname) > 0) break;
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
	return(TRUE);
}

static char cutt_dir(char *argvS, cutt_opts *args, char *isFF, int *worked, char *addToExpand) {
	int i, dPos, offset;
	FNType fname[FILENAME_MAX];
	cutt_FileList *tFile;
#if defined(UNX) || defined(_MAC_CODE)
	struct dirent *dp;
	struct stat sb;
	DIR *cDIR;
#endif // _MAC_CODE
#ifdef _WIN32
	BOOL notDone;
	CFileFind dirFind;
	CString fnameFound;
#endif // _WIN32

	dPos = strlen(args->path);
 	if (SetNewVol(args->path)) {
		fprintf(stderr,"\nCan't change to folder: %s.\n", args->path);
		fprintf(stderr,"    From original option: %s.\n", argvS);
		return(FALSE);
	}
	free_FileList(args->root_file);
	args->root_file = NULL;
	if (stin) {
		addFilename2Path(args->path, args->targ);
		if (isRecursive || WD_Not_Eq_OD)
			offset = 0;
		else if (args->path[dPos] == PATHDELIMCHR)
			offset = dPos + 1;
		else
			offset = dPos;
		if (addToExpand) {
			if (strlen(addToExpand)+strlen(args->path+offset)+3 >= EXTENDEDARGVLEN-1) {
				fprintf(stderr, "Too many files specified'\n");
				return(FALSE);
			}
			strcat(addToExpand, "\"");
			uS.FNType2str(addToExpand, strlen(addToExpand), args->path+offset);
			strcat(addToExpand, "\"");
			strcat(addToExpand, " ");
		} else if (!cutt_addToFileList(args, args->path+offset)) {
			return(FALSE);
		}
		args->path[dPos] = EOS;
	} else {
		i = 1;
		while ((i=Get_File(fname, i)) != 0) {
			if (uS.fIpatmat(fname, args->targ)) {
				addFilename2Path(args->path, fname);
				if (isRecursive || WD_Not_Eq_OD)
					offset = 0;
				else if (args->path[dPos] == PATHDELIMCHR)
					offset = dPos + 1;
				else
					offset = dPos;
				if (addToExpand) {
					if (strlen(addToExpand)+strlen(args->path+offset)+3 >= EXTENDEDARGVLEN-1) {
						fprintf(stderr, "Too many files specified'\n");
						return(FALSE);
					}
					strcat(addToExpand, "\"");
					uS.FNType2str(addToExpand, strlen(addToExpand), args->path+offset);
					strcat(addToExpand, "\"");
					strcat(addToExpand, " ");
				} else if (!cutt_addToFileList(args, args->path+offset)) {
					return(FALSE);
				}
				args->path[dPos] = EOS;
			}
		}
	}
	for (tFile=args->root_file; tFile != NULL; tFile=tFile->next_file) {
		if (WD_Not_Eq_OD)
			SetNewVol(od_dir);
		*worked = work(args->argv,args->argc,tFile->fname);
		if (WD_Not_Eq_OD)
			SetNewVol(args->path);
		*isFF = TRUE;
		if (*worked == 2)
			break;
		if (stin)
			break;
		if (isKillProgram)
			break;
	}

	if (!isRecursive)
		return(TRUE);

 	SetNewVol(args->path);
#if defined(UNX) || defined(_MAC_CODE)
	if ((cDIR=opendir(".")) != NULL) {
		while ((dp=readdir(cDIR)) != NULL) {
			if (stat(dp->d_name, &sb) == 0) {
				if (!S_ISDIR(sb.st_mode)) {
					continue;
				}
			} else
				continue;
			if (dp->d_name[0] == '.')
				continue;
			addFilename2Path(args->path, dp->d_name);
			uS.str2FNType(args->path, strlen(args->path), PATHDELIMSTR);
			if (!cutt_dir(argvS, args,isFF,worked,addToExpand)) {
				args->path[dPos] = EOS;
				closedir(cDIR);
				return(FALSE);
			}
			args->path[dPos] = EOS;
			SetNewVol(args->path);
		}
		closedir(cDIR);
	}
#endif // _MAC_CODE
#ifdef _WIN32
	notDone = dirFind.FindFile(_T("*.*"), 0);
	while (notDone) {
		if (isKillProgram)
			break;
		if (*worked == 2)
			break;
		notDone = dirFind.FindNextFile();
		if (!dirFind.IsDirectory())
			continue;
		fnameFound = dirFind.GetFileName();
		if (fnameFound[0] == '.')
			continue;
		u_strcpy(fname, fnameFound, FILENAME_MAX);
		addFilename2Path(args->path, fname);
		strcat(args->path, PATHDELIMSTR);
		if (!cutt_dir(argvS, args,isFF,worked,addToExpand)) {
			args->path[dPos] = EOS;
			dirFind.Close();
			return(FALSE);
		}
		args->path[dPos] = EOS;
	}
	dirFind.Close();
#endif // _WIN32
	return(TRUE);
}
#endif /* defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)*/
/*
// unix argv extender: *.* if bmain is not called
if ((argc=expandArgv(argc, argv)) == 0) {
	exit(0);
}

#if defined(_MAC_CODE) || defined(_WIN32)
static int WildStar(char *w) {
	for (; *w; w++) {
		if (*w == '*')
			return(TRUE);
	}
	return(FALSE);
}

static int WildAt(char *w) {
	if (!strcmp(w, "@") || (w[0] == '@' && w[1] == ':'))
		return(TRUE);
	else
		return(FALSE);
}

static int extendArgvVar(int oldArgc, char *argv[]) {
	register int  argc;
	register char *com;
	register char *endCom;

	SetNewVol(wd_dir);
	com = expandedArgv;
	argc = 1;
	while (*com != EOS) {
		for (; *com == ' ' || *com == '\t'; com++) ;
		if (*com == EOS)
			break;
		endCom = NextArg(com);
		if (endCom == NULL)
			return(FALSE);

		if (argc < oldArgc) {
			while (argv[argc] != NULL && argc < oldArgc)
				argc++;
		}
		if (argc >= MAX_ARGS) {
#ifndef UNX
			do_warning("out of memory; Too many arguments.", 0);
#else
			fprintf(stderr, "out of memory; Too many arguments.");
#endif
			return(0);
		}
		argv[argc++] = com;
		com = endCom;
	}
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
	if (argc < oldArgc) {
		while (argv[argc] != NULL && argc < oldArgc)
			argc++;
	}
	return(argc);
}

int expandArgv(int argc, char *argv[]) { // unix argv extender: *.* if bmain is not called
	int i, j;
	int cArgc = argc;
	int worked = 1;
	char isFileFound;
	char st[FNSize];
	FNType path[FNSize];
	FNType targ[FILENAME_MAX];
	cutt_opts args;
	FILE *fp;
	extern int F_numfiles;

	isRecursive = FALSE;
	expandedArgv = (char *)malloc(EXTENDEDARGVLEN);
	if (expandedArgv == NULL) {
		fprintf(stderr, "\nOut of memory\n");
		return(0);
	}
	*expandedArgv = EOS;
	for (i=1; i < cArgc; i++) {
		if (argv[i][0] == '-' || argv[i][0] == '+')
			continue;
		else if (!WildStar(argv[i]) && !WildAt(argv[i]))
			continue;

		if (!strcmp(argv[i], "@")) {
			if (F_numfiles <= 0) {
				fprintf(stderr, "No files were selected to go with '@', \"File In\" button\n");
				return(0);
			}
			for (j=1; j <= F_numfiles; j++) {
				get_selected_file(j, path);
				if (strlen(expandedArgv)+strlen(path)+3 >= EXTENDEDARGVLEN-1) {
					fprintf(stderr, "Too many files specified'\n");
					return(0);
				}
				strcat(expandedArgv, "\"");
				uS.FNType2str(expandedArgv, strlen(expandedArgv), path);
				strcat(expandedArgv, "\"");
				strcat(expandedArgv, " ");
			}
		} else if (argv[i][0] == '@' && argv[i][1] == ':') {
			SetNewVol(wd_dir);
			uS.str2FNType(path, 0L, argv[i]+2);
			fp = fopen(path, "r");
			if (fp == NULL) {
				fprintf(stderr, "Can't open input files list file: %s\n", argv[i]+2);
				return(0);
			}
			while (fgets_cr(st, 3072, fp)) {
				uS.remblanks(st);
				if (strlen(expandedArgv)+strlen(path)+3 >= EXTENDEDARGVLEN-1) {
					fprintf(stderr, "Too many files specified'\n");
					return(0);
				}
				strcat(expandedArgv, "\"");
				strcat(expandedArgv, st);
				strcat(expandedArgv, "\"");
				strcat(expandedArgv, " ");
			}
			fclose(fp);
		} else {
			strcpy(path, wd_dir);
			uS.strn2FNType(targ, 0L, argv[i], FILENAME_MAX-1);
			targ[FILENAME_MAX-1] = EOS;
			uS.uppercasestr(targ, &dFnt, C_MBF);
			SetNewVol(wd_dir);
			args.root_file = NULL;
			args.path = path;
			args.argv = argv;
			args.argc = argc;
			args.targ = targ;
			if (!cutt_dir(argv[i], &args,&isFileFound,&worked,expandedArgv)) {
				fprintf(stderr, "\nOut of memory\n");
				return(0);
			}
		}
		argv[i] = NULL;
	}
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
	argc = extendArgvVar(argc, argv);
	return(argc);
}
#endif // defined(_MAC_CODE) || defined(_WIN32)
*/

/* bmain(argc,argv,pr_result) calls getflag to pars given options and call
   work to perform work on a given input data. It returns 0 if no work was
   performed on any input data files, 2 if last input file couldn't have been
   opened, 1 otherwise. The options setup could be: -sword, or -s word.
   "argc" and "argv" are the same as in main(). "pr_result" says which
   function is used to print result. It is set to NULL if combined result
   need NOT be printed.
*/
void bmain(int argc, char *argv[], void (*pr_result)(void)) {
	int i;
	int worked = 1;
	char isCRPrinted, *ts;
	struct tier *tt;
	FNType path[FNSize];
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	int  j;
	char st[FNSize];
	FNType targ[FILENAME_MAX];
	char isFileFound;
	cutt_opts args;
	FILE *fp;
	extern int F_numfiles;
#endif /* defined(_MAC_CODE) || defined(_WIN32) || defined(UNX) */

#if defined(UNX)
#ifdef USE_TERMIO
	struct termio otty;
#else
	struct sgttyb  otty;
#endif
#endif

#ifdef UNX
	if (CLAN_PROG_NUM == EVAL) {
		char isCreateDB = 0;
		for (i=1; i < argc; i++) {
			if (*argv[i] == '+'  || *argv[i] == '-') {
				if (argv[i][1] == 'c') {
					if (argv[i][2] == '1')
						isCreateDB = 2;
				}
			}
		}
		if (isCreateDB == 2)
			strcpy(wd_dir, "/TalkBank/data-orig/AphasiaBank/English");
		else
			getcwd(wd_dir, FNSize);
	} else
		getcwd(wd_dir, FNSize);
	strcpy(od_dir, wd_dir);
  #if defined(CLAN_SRV)
	fp = fopen("/web/childes/browser/lib/depfile.cut", "r");
	if (fp != NULL) {
		fclose(fp);
		strcpy(lib_dir, "/web/childes/browser/lib/");
		strcpy(mor_lib_dir, "/web/childes/browser/lib/");
	} else {
		strcpy(lib_dir, "/TalkBank/browser/lib/");
		strcpy(mor_lib_dir, "/TalkBank/browser/lib/");
	}
  #else
	strcpy(lib_dir, DEPDIR);
	strcpy(mor_lib_dir, DEPDIR);
  #endif
#endif
	targs = (char *) malloc(argc);
	if (targs == NULL)
		out_of_mem();
	mmaininit();
	if (argc > 1) {
		if (argv[1][0] == '-' || argv[1][0] == '+') {
			if (argv[1][1] == 'v') {
				if (argv[1][2] == EOS) {
					VersionNumber(FALSE, stdout);
					cutt_exit(0);
				}
			}
		}
	}
	InitOptions();
	if (argc >= 2) {
		if (argv[1][0] == '+' && argv[1][1] == '0') {
			stin_override = TRUE;
			if (argc < 3) {
				usage();
			}
		}
	}
#if defined(_MAC_CODE) || defined(_WIN32)
	if (!isatty(stdin) && !stin_override)
		stin = TRUE;
#elif defined(UNX)
  #ifdef USE_TERMIO
	if (ioctl(fileno(stdin), TCGETA, &otty) == -1) {
		if (errno == ENOTTY && !stin_override)
			stin = TRUE;
	}
  #elif defined(APPLEUNX)
	if (ioctl(fileno(stdin), TIOCGETP, &otty) != 0 && !stin_override)
		stin = TRUE;
  #else
	if (gtty(fileno(stdin), &otty) != 0 && !stin_override)
		stin = TRUE;
  #endif
#endif
	if (argc < 2 && !stin) {
		usage();
	}
/* this MUST follow the mmaininit function call but preceed user args check */
	init('\001');

	strcpy(GExt,options_ext[CLAN_PROG_NUM]);
	for (i=1; i < argc; i++) {
		targs[i] = 1;
		if (*argv[i] == '+'  || *argv[i] == '-') {
			if (CLAN_PROG_NUM == FREQ) {
				if (argv[i][1] == 'c') {
					if (argv[i][2] == '3')
						anyMultiOrder = TRUE;
					else if (argv[i][2] == '4')
						onlySpecWsFound = TRUE;
				}
			}
			if (argv[i][1] == 'k') {
				if (!nomap)
					nomap = TRUE;
				else
					nomap = FALSE;
			} else if (argv[i][1] == 's' && argv[i][2] == '@' && argv[i][3] == EOS) {
				morSearchSytaxUsage(argv[i]);
			} else if (argv[i][1] == 's' && argv[i][2] == '@' && argv[i][3] == 's' && argv[i][4] == EOS) {
				LangSearchSytaxUsage(argv[i]);
			}
		}
	}
	for (; i < argc; i++)
		targs[i] = 1;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '-' || *argv[i] == '+') {
			if (i+1 < argc) {
				getflag(argv[i],argv[i+1],&i);
			} else {
				getflag(argv[i],NULL,&i);
			}
		} else if (*argv[i] != EOS)
			targs[i] = 0;
	}
	if (wdUttLen != NULL && filterUttLen_cmp == 0) {
		fprintf(stderr, "\nPlease specify comparison type (=, <, >)\n");
		fprintf(stderr, "\tnumber and type (m, w, c) of items after \"+x\" option.\n");
		fprintf(stderr, "along with \"+/-xword\" option.\n");
		fprintf(stderr,"\tFor example: +x=0w - select only utterances with zero words\n");
		fprintf(stderr,"\t\t+x>0w - select utterances with 1 or more words\n");
		fprintf(stderr,"\t\t+xword - count only this \"word\"\n");
		fprintf(stderr,"\t\t-xword - do not count this \"word\"\n");
		return;
	}
	for (i=1; i < argc; i++) {
		if (targs[i])
			continue;

#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
		isFileFound = FALSE;

   #if defined(_MAC_CODE) || defined(_WIN32)
		if (!strcmp(argv[i], "@")) {
			if (WD_Not_Eq_OD)
				SetNewVol(od_dir);
			if (F_numfiles <= 0) {
				fprintf(stderr, "No files were selected to go with '@', \"File In\" button\n");
				worked = 2;
				isFileFound = TRUE;
				break;
			}
			for (j=1; j <= F_numfiles; j++) {
				get_selected_file(j, path);
				worked = work(argv,argc,path);
				isFileFound = TRUE;
				if (worked == 2)
					break;
				if (stin)
					break;
				if (isKillProgram)
					break;
			}
		} else
   #endif /* defined(_MAC_CODE) || defined(_WIN32) */
		if (argv[i][0] == '@' && argv[i][1] == ':') {
			uS.str2FNType(path, 0L, argv[i]+2);
			fp = fopen(path, "r");
			if (fp == NULL) {
				fprintf(stderr, "Can't open input files list file: %s\n", argv[i]+2);
				break;
			}
			while (fgets_cr(st, 3072, fp)) {
				if (uS.isUTF8(st) || uS.partcmp(st,FONTHEADER,FALSE,FALSE) || st[0] == '#' || st[0] == '%')
					continue;
				uS.remblanks(st);
				uS.str2FNType(path, 0L, st);
				worked = work(argv,argc,path);
				isFileFound = TRUE;
				if (worked == 2)
					break;
				if (stin)
					break;
				if (isKillProgram)
					break;
			}
			fclose(fp);
		} else {
   #if defined(_MAC_CODE) || defined(UNX)
			strcpy(path, wd_dir);
   #else // #ifdef defined(_MAC_CODE) || defined(UNX)
			if (*argv[i] == '\\' || (argv[i][1] == ':' && argv[i][2] == '\\')) {
				_splitpath(argv[i], NULL, path, NULL, NULL );
				if (*argv[i] == '\\' && path[0] == '\\')
					strcpy(argv[i], argv[i]+strlen(path));
				else
					strcpy(argv[i], argv[i]+strlen(path)+2);
			} else if (*argv[i] == '.') {
				_splitpath(argv[i], NULL, templineC3, NULL, NULL );
				strcpy(path, wd_dir);
				if ( _fullpath(path, templineC3, _MAX_PATH ) != NULL )
					strcpy(argv[i], argv[i]+strlen(templineC3));
				else
					strcpy(path, wd_dir);
			} else
				strcpy(path, wd_dir);
   #endif // #else defined(_MAC_CODE) || defined(UNX)
			uS.strn2FNType(targ, 0L, argv[i], FILENAME_MAX-1);
			targ[FILENAME_MAX-1] = EOS;
   #if defined(_WIN32)
			uS.uppercasestr(targ, &dFnt, C_MBF);
   #endif
   #if defined(_MAC_CODE) || defined(UNX)
			if ((ts=strrchr(targ, PATHDELIMCHR)) != NULL) {
				*ts = EOS;
				if (targ[0] == '/') {
					DirPathName[0] = EOS;
					path[0] = EOS;
				} else
 					strcpy(DirPathName, wd_dir);
				addFilename2Path(DirPathName, targ);
				addFilename2Path(DirPathName, "");
				addFilename2Path(path, targ);
				addFilename2Path(path, "");
				strcpy(targ, ts+1);
				SetNewVol(DirPathName);
			} else
   #endif
				SetNewVol(wd_dir);
			args.root_file = NULL;
			args.path = path;
			args.argv = argv;
			args.argc = argc;
			args.targ = targ;
			if (!cutt_dir(argv[i],&args,&isFileFound,&worked,NULL))
				break;
			free_FileList(args.root_file);
			args.root_file = NULL;
		}
		if (isKillProgram)
			break;
		if (!isFileFound) {
			if (!stin) {
				fprintf(stderr,"\n**** WARNING: No file matching \"%s\" was found.\n\n", argv[i]);
			} else {
				uS.str2FNType(path, 0L, argv[i]);
				for (j=0; path[j]!='*' && path[j]!=EOS; j++) ;
				if (path[j] != '*')
					worked = work(argv,argc,path);
			}
			worked = 2;
		}
#else /* #if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX) */
		strcpy(path, argv[i]);
		worked = work(argv,argc,path);
		if (worked == 2)
			break;
		if (stin)
			break;
#endif /* #else defined(_MAC_CODE) || defined(_WIN32) || defined(UNX) */
	}

	if (worked == 1) {
		if (!stin)
			fprintf(stderr,"No input file given.\n");
		else {
			strcpy(path, "pipeout");
			worked = work(argv,argc,path);
		}
	}
	if (isKillProgram) {
		if (combinput && worked == 0 && !FirstTime && !stout) {
			if (fpout)
				fclose(fpout);
			fpout = NULL;
		}
		main_cleanup();
		return;
	}
	if (combinput && worked == 0 && !FirstTime) {
		if (!stout) {
			if (pr_result != NULL)
				(*pr_result)();
			fprintf(stderr,"Output file <%s>\n",newfname);
			fclose(fpout);
			fpout = NULL;
		} else
			if (pr_result != NULL)
				(*pr_result)();
	}
	if (worked == 0 && chatmode) {
		if (mor_link.error_found) {
#ifdef UNX
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", mor_link.fname, mor_link.lineno);
			fprintf(stderr, "WARNING: %%MOR: TIER DOES NOT LINK IN SIZE TO IT'S SPEAKER TIER.\n");
			fprintf(stderr, "THIS MAY EFFECT RESULTS OF THE ANALYSES.\n\n");
#else
			fprintf(stderr,"\n%c%c*** File \"%s\": line %ld.%c%c\n", ATTMARKER, error_start, mor_link.fname, mor_link.lineno, ATTMARKER, error_end);
			fprintf(stderr, "%c%cWARNING: %%MOR: TIER DOES NOT LINK IN SIZE TO IT'S SPEAKER TIER.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
			fprintf(stderr, "%c%cTHIS MAY EFFECT RESULTS OF THE ANALYSES.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
		}
		isCRPrinted = FALSE;
		for (tt=headtier; tt != NULL; tt=tt->nexttier) {
			if (tt->used == FALSE) {
				if (!isCRPrinted) {
					isCRPrinted = TRUE;
					fprintf(stderr, "\n\n");
				}
				if (IDField != NULL && uS.partcmp(tt->tcode,"@ID:",FALSE,FALSE)) {
					fprintf(stderr, "    TIER \"%s\" HASN'T BEEN FOUND IN THE INPUT DATA!\n", tt->tcode);
				} else {
					if (CntWUT)
						fprintf(stderr, "    TIER \"%s\", ASSOCIATED WITH A \"+z OPTION\" SELECTION,\n        HASN'T BEEN FOUND IN THE INPUT DATA!\n", tt->tcode);
					else
						fprintf(stderr, "    TIER \"%s\", ASSOCIATED WITH A SELECTED SPEAKER,\n        HASN'T BEEN FOUND IN THE INPUT DATA!\n", tt->tcode);
					if ((CLAN_PROG_NUM == MLU || CLAN_PROG_NUM == CHIP || CLAN_PROG_NUM == WDLEN) && uS.mStricmp(tt->tcode, "%MOR") == 0) {
						fprintf(stderr, "ADD -t%%mor TO YOUR COMMAND TO ANALYZE JUST THE MAIN LINE\n");
					}
				}
			}
		}
	}
/*
	for (tt=HeadOutTier; tt != NULL; tt=tt->nexttier) {
		if (tt->used == FALSE) {
			fprintf(stderr, "\n\nTIER \"%s\" HASN'T BEEN FOUND IN THE INPUT DATA!\n",
					tt->tcode);
		}
	}
*/
	main_cleanup();
}

/* output tiers handling
 */
int CheckOutTier(char *s) {
	struct tier *p;

	if (!chatmode)
		return(TRUE);
	for (p=HeadOutTier; p != NULL; p=p->nexttier) {
		if (uS.partcmp(s,p->tcode,p->pat_match,FALSE)) {
/*			if (p->used != '\002') p->used = '\001'; */
			break;
		}
	}
	if (p != NULL) {
		if (!p->include) {
			return(FALSE);
		}
	} else {
		if ((*s == '*' && otcs) || (*s == '@' && otch) || (*s == '%' && otct)) {
			return(FALSE);
		}
	}
	return(TRUE);
}

/* MakeOutTierChoice(ts,inc) creats a list tiers which should be included or
 excluded from KWAL and COMBO output. "ts" is a tier string, inc is either "+" or "-". 
 Procedure will remove apropriate tiers from the default list based on the tiers
 specified by the user.
 */
void MakeOutTierChoice(char *ts, char inc) {
	struct tier *nt, *tnt;
	int l1, l2;

	if (!*ts) {
		fprintf(stderr,"String expected after %co option.\n", '+');
		cutt_exit(0);
	}
	uS.uppercasestr(ts, &dFnt, C_MBF);
	if (HeadOutTier == NULL) {
		HeadOutTier = NEW(struct tier);
		nt = HeadOutTier;
		nt->nexttier = NULL;
	} else {
		l1 = strlen(ts);
		tnt= HeadOutTier;
		nt = HeadOutTier;
		while (1) {
			if (nt == NULL) break;
			else if (uS.partcmp(ts,nt->tcode,FALSE,TRUE)) {
				l2 = strlen(nt->tcode);
				if (l1 > l2) break;
				else return;
			}
			tnt = nt;
			nt = nt->nexttier;
		}
		if (nt == NULL) {
			tnt->nexttier = NEW(struct tier);
			nt = tnt->nexttier;
			nt->nexttier = NULL;
		} else if (nt == HeadOutTier) {
			HeadOutTier = NEW(struct tier);
			HeadOutTier->nexttier = nt;
			nt = HeadOutTier;
		} else {
			nt = NEW(struct tier);
			nt->nexttier = tnt->nexttier;
			tnt->nexttier = nt;
		}
	}
/*
	 if (*ts == '*' && *(ts+1) == ':')
		 nt->used = '\001';
	 else
		 nt->used = FALSE;
	 nt->include = TRUE;
*/
	if (*ts != '*' && *ts != '@' && *ts != '%')
		strcpy(nt->tcode,"*");
	else
		nt->tcode[0] = EOS;
	nt->include = (char)((inc == '-') ? FALSE : 1);
	nt->pat_match = FALSE;
	for (l1=0; ts[l1] != EOS; l1++) {
		if (ts[l1] == '#') {
			nt->pat_match = TRUE;
			ts[l1] = '*';
		}
	}
	if (ts[0] == '%')
		otcdt = TRUE;
	strcat(nt->tcode,ts);
	if (!nt->include) {
		if (*ts == '@') otch = FALSE;
		else if (*ts == '%') otct = FALSE;
		else otcs = FALSE;
	}
}

/**************************************************************/
/*	 tiers options, find the right tier and get tier line.  */

/* checktier(s) determines if the "s" tier should be selected or not.
*/
int checktier(char *s) {
	struct tier *p;

	if (!chatmode)
		return(TRUE);
	for (p=defheadtier; p != NULL; p=p->nexttier) {
		if (uS.partcmp(s,p->tcode,p->pat_match,FALSE))
			break;
	}
	if (p == NULL) {
		for (p=headtier; p != NULL; p=p->nexttier) {
			if (uS.partcmp(s,p->tcode,p->pat_match,FALSE)) {
				if (p->used != '\002')
					p->used = '\001';
				break;
			}
		}
	}
	if (p != NULL) {
		if (p->include) {
			if (p->include == 2) {
				register int i, j;
				i = strlen(p->tcode);
				for (j=i; s[j] != ':' && s[j]; j++) ;
				if (j > i)
					strcpy(s+i, s+j);
			}
		} else {
			if (ByTurn) {
				if (*s == '*')
					*TSoldsp = EOS;
			}
			return(FALSE);
		}
	} else {
		if ((*s == '*' && tcs) || (*s == '@' && tch) || (*s == '%' && tct)) {
			if (ByTurn) {
				if (*s == '*')
					*TSoldsp = EOS;
			}
			return(FALSE);
		}
	}
	return(TRUE);
}

/* maketierchoice(ts,inc) creats a list tiers which should be included or
 excluded. "ts" is a tier string, inc is either "+" or "-". Procedure will
 remove apropriate tiers from the default list based on the tiers
 specified by the user.
 */
void maketierchoice(const char *SpId, char inc, char istemp) {
	struct tier *nt, *tnt;
	register int l1;
	register int l2;
	int inc_type = 1;
	char ts[SPEAKERLEN];

	if (!*SpId) {
		fprintf(stderr,"String expected after %ct option.\n", inc);
		cutt_exit(0);
	}
	if (inc == '-' && !strcmp(SpId,"*")) {
		if (chatmode)
			nomain = TRUE;
		else {
			fprintf(stderr, "\"-t*\" option is not allowed with TEXT format\n");
			cutt_exit(0);
		}
		return;
	}
	if (*SpId != '*' && *SpId != '@' && *SpId != '%')
		strcpy(ts, "*");
	else
		*ts = EOS;
	strcat(ts, SpId);
	uS.uppercasestr(ts, &dFnt, C_MBF);
	nt = defheadtier;
	tnt = nt;
	while (nt != NULL) {
		if (*ts == *nt->tcode) {
			if (nt == tnt)
				defheadtier = nt->nexttier;
			else
				tnt->nexttier = nt->nexttier;
			free(nt);
			if (*ts == '@') tch = FALSE;
			else if (*ts == '%') tct = FALSE;
			else if (*ts == '*') tcs = FALSE;
			break;
		} else if (*ts != '@' && *ts != '%' && *nt->tcode == '*') {
			if (nt == tnt)
				defheadtier = nt->nexttier;
			else
				tnt->nexttier = nt->nexttier;
			free(nt);
			tcs = FALSE;
			break;
		}
		tnt = nt;
		nt = nt->nexttier;
	}
	l1 = strlen(ts);
	if (ts[l1-1] == '%' && l1 > 1) {
		ts[--l1] = EOS;
		inc_type = 2;
	}
	if (headtier == NULL) {
		headtier = NEW(struct tier);
		nt = headtier;
		nt->nexttier = NULL;
	} else {
		tnt= headtier;
		nt = headtier;
		while (1) {
			if (nt == NULL)
				break;
			else if (uS.partcmp(ts,nt->tcode,FALSE,FALSE)) {
				l2 = strlen(nt->tcode);
				if (l1 > l2) {
					if (nt->include != FALSE && inc == '-')
						nt->used = 1;
					break;
				} else {
					if (l1 == l2-1 && nt->tcode[l2-1] == ':' && l2 > 0)
						nt->tcode[l2-1] = EOS;
					if (istemp != '\002' || istemp == nt->used)
						return;
				}
			}
			tnt = nt;
			nt = nt->nexttier;
		}
		if (nt == NULL) {
			tnt->nexttier = NEW(struct tier);
			nt = tnt->nexttier;
			nt->nexttier = NULL;
		} else if (nt == headtier) {
			headtier = NEW(struct tier);
			headtier->nexttier = nt;
			nt = headtier;
		} else {
			nt = NEW(struct tier);
			nt->nexttier = tnt->nexttier;
			tnt->nexttier = nt;
		}
	}
	nt->used = istemp;
	nt->include = (char)((inc == '-') ? FALSE : inc_type);
	nt->pat_match = FALSE;
	for (l1=1; ts[l1] != EOS; l1++) {
		if (ts[l1] == '#' || ts[l1] == '*') {
			nt->pat_match = TRUE;
			ts[l1] = '*';
		}
	}
	strcpy(nt->tcode,ts);
	if (nt->include) {
		if (*ts == '@') tch = TRUE;
		else if (*ts == '%') tct = TRUE;
		else tcs = TRUE;
	}
}

static char SpecialDelim(char *ch, int index) {
	if (ch[index] == ';' && !uS.isskip(ch, index, &dFnt, MBF)) {
		return(TRUE);
	} else if (CLAN_PROG_NUM != MOR_P && ch[index] == ',' && !uS.isskip(ch, index, &dFnt, MBF)) {
		return(TRUE);
	}
	return(FALSE);
}


static void removeAngleBracketsFromLastElem(int lastElem, char *st) {
	int i, agb;

	agb = 0;
	for (i=strlen(st)-1; i >= lastElem && st[i] != ']'; i--) {
		if (st[i] == '<') {
			agb--;
			st[i] = ' ';
		} else if (st[i] == '>') {
			agb++;
			st[i] = ' ';
		}
	}	
	for (i=lastElem; st[i] != EOS && agb > 0; i--) {
		if (st[i] == '<') {
			agb--;
			st[i] = ' ';
		} else if (st[i] == '>') {
			break;
		}
	}	
}

static void removeExtraSpaceFromExpandX(char *st, AttTYPE *att) {
	int i;

	for (i=0; st[i] != EOS; ) {
		if (st[i]=='\n')
			st[i] = ' ';
		if (st[i]==' ' || st[i]=='\t' || (st[i]=='<' && (i==0 || st[i-1]==' ' || st[i-1]=='\t'))) {
			i++;
			while (st[i] == ' ' || st[i] == '\t' || st[i] == '\n')
				att_cp(0,st+i,st+i+1,att+i,att+i+1);
		} else
			i++;
	}
}

static int expandX(char *ch, AttTYPE *att, int x, int index) {
	int i, b, e, sqb, agb, n, cnt, lastElem;

	for (e=x-2; e >= 0 && (isSpace(ch[e]) || ch[e] == '\n'); e--) ;
	if (e < 0)
		return(index);
	sqb = 0;
	for (b=e; b >= 0; b--) {
		if (ch[b] == '[')
			sqb--;
		else if (ch[b] == ']')
			sqb++;
		else if (sqb == 0 && (ch[b] == '>' || !uS.isskip(ch,b,&dFnt,MBF)))
			break;
	}
	if (b < 0)
		return(index);
	if (ch[b] == '>') {
		agb = 0;
		sqb = 0;
		for (; b >= 0; b--) {
			if (ch[b] == '[')
				sqb--;
			else if (ch[b] == ']')
				sqb++;
			else if (sqb == 0) {
				if (ch[b] == '<') {
					agb--;
					if (agb <= 0)
						break;
				} else if (ch[b] == '>')
					agb++;
			}
		}
		if (b < 0)
			return(index);
	} else {
		for (; b >= 0; b--) {
			if (uS.isskip(ch,b,&dFnt,MBF) || ch[b] == '<' || ch[b] == '>' || ch[b] == '[' || ch[b] == ']') {
				break;
			}
		}
		b++;
	}
	for (n=x+1; isalpha(ch[n]) || isSpace(ch[n]); n++) ;
	cnt = atoi(ch+n);
	if (cnt < 2)
		return(index);
	att_cp(0,ch+x-1,ch+index,att+x-1,att+index);
	index = x - 1;
	i = 0;
	lastElem = i;
	while (cnt > 1) {
		cnt--;
		tempTier[i] = '[';
		tempAtt[i] = 0;
		i++;
		tempTier[i] = '/';
		tempAtt[i] = 0;
		i++;
		tempTier[i] = ']';
		tempAtt[i] = 0;
		i++;
		tempTier[i] = ' ';
		tempAtt[i] = 0;
		i++;
		lastElem = i;
		for (n=b; n <= e; n++) {
			tempTier[i] = ch[n];
			tempAtt[i] = att[n];
			i++;
		}
		tempTier[i] = ' ';
		tempAtt[i] = 0;
		i++;
	}
	tempTier[i] = EOS;
	removeAngleBracketsFromLastElem(lastElem, tempTier);
	removeExtraSpaceFromExpandX(tempTier, tempAtt);
	n = strlen(tempTier);
	att_shiftright(ch+index, att+index, n);
	for (i=0; tempTier[i] != EOS; i++) {
		ch[index] = tempTier[i];
		att[index] = tempAtt[i];
		index++;
	}
	return(index);
}

void cutt_cleanUpLine(const char *sp, char *ch, AttTYPE *att, int oIndex) {
	int x, index;

	if (uS.partcmp(sp,"%mor:",FALSE, FALSE)) {
		index = oIndex;
		while (ch[index] != EOS) {
			if (ch[index] == '.') {
				if (!uS.isPlusMinusWord(ch, index) && !isSpace(ch[index-1]) && 
					  (!isdigit(ch[index-1]) || !isdigit(ch[index+1])) && index > 0) {
					att_shiftright(ch+index, att+index, 1);
					ch[index] = ' ';
					index += 2;
				} else
					index++;
				while (ch[index] == '.' || ch[index] == '?' || ch[index] == '!')
					index++;
			} else if (ch[index] == '?' || ch[index] == '!' || SpecialDelim(ch, index)) {
				if (!uS.isPlusMinusWord(ch, index) && !isSpace(ch[index-1]) &&
					  (!isdigit(ch[index-1]) || *sp == '*') && index > 0) {
					att_shiftright(ch+index, att+index, 1);
					ch[index] = ' ';
					index += 2;
				} else
					index++;
				while (ch[index] == '.' || ch[index] == '?' || ch[index] == '!')
					index++;
			} else
				index++;
		}
		return;
	}

	index = oIndex;
	while (ch[index] != EOS) {
		if (uS.isRightChar(ch, index, '[', &dFnt, MBF)) {
			x = index + 1;
			for (index++; ch[index] != EOS && !uS.isRightChar(ch, index, ']', &dFnt, MBF); index++) ;
			if (ch[index] != EOS)
				index++;
			if ((ch[x] == 'x' || ch[x] == 'X') && isSpace(ch[x+1]) && isExpendX && CLAN_PROG_NUM < MEGRASP) {
				if (ch[index] != EOS)
					index = expandX(ch, att, x, index);
			}
		} else if (ch[index] == HIDEN_C) {
			for (index++; ch[index] != EOS && ch[index] != HIDEN_C; index++) ;
			if (ch[index] != EOS)
				index++;
		} else if (uS.isRightChar(ch, index, '&', &dFnt, MBF)) {
			for (index++; ch[index] != EOS; index++) {
				if (uS.isskip(ch, index, &dFnt, MBF) || ch[index] == '>' || ch[index] == '[' || ch[index] == ',' || 
					ch[index] == '+' || ch[index] == '.' || ch[index] == '?' || ch[index] == '!')
					break;
			}
		} else if (uS.isRightChar(ch, index, ',', &dFnt, MBF) && uS.isRightChar(ch, index+1, ',', &dFnt, MBF)) {
			if (!uS.isPlusMinusWord(ch, index) && !uS.atUFound(ch, index, &dFnt, MBF) && 
				  !isSpace(ch[index-1]) && (!isdigit(ch[index-1]) || *sp != '@') && index > 0) {
				att_shiftright(ch+index, att+index, 1);
				ch[index] = ' ';
				index += 3;
			} else
				index += 2;
		} else if (ch[index] == '.') {
			if (!uS.isPlusMinusWord(ch, index) && !uS.atUFound(ch, index, &dFnt, MBF) && !uS.isPause(ch,index,NULL,NULL) &&
				!isSpace(ch[index-1]) && (!isdigit(ch[index-1]) || (*sp == '*' && !isdigit(ch[index+1]))) && index > 0) {
				att_shiftright(ch+index, att+index, 1);
				ch[index] = ' ';
				index += 2;
			} else
				index++;
			while (ch[index] == '.' || ch[index] == '?' || ch[index] == '!')
				index++;
		} else if (ch[index] == '?' || ch[index] == '!' || SpecialDelim(ch, index)) {
			if (!uS.isPlusMinusWord(ch, index) && !uS.atUFound(ch, index, &dFnt, MBF) &&
				!isSpace(ch[index-1]) && (!isdigit(ch[index-1]) || *sp == '*') && index > 0) {
				att_shiftright(ch+index, att+index, 1);
				ch[index] = ' ';
				index += 2;
			} else
				index++;
			while (ch[index] == '.' || ch[index] == '?' || ch[index] == '!')
				index++;
		} else
			index++;
	}
//	[: ] [:* ] [:=_ ]
	if (isLanguageExplicit && chatmode && *sp == '*') {
		int  tDefLanguage;
		int  i, j, k, startIndex;
		char lang[9], *pLang, isAddLang, isPrecodeFound;

		lang[0] = EOS;
		startIndex = oIndex;
		index = oIndex;
		isPrecodeFound = FALSE;
		tDefLanguage = defLanguage;
		while (ch[index] != EOS) {
			while (ch[index] != EOS && uS.isskip(ch,index,&dFnt,MBF) && !uS.isRightChar(ch,index,'[',&dFnt,MBF))
				index++;

			if (ch[index] == EOS)
				break;

			if (uS.isRightChar(ch, index, '[', &dFnt, MBF)) {
				if (ch[index+1] == '-') {
					k = index;
					isPrecodeFound = TRUE;
					lang[0] = EOS;
					index += 2;
					for (; isSpace(ch[index]); index++) ;
					for (i=0; ch[index] != EOS && !uS.isRightChar(ch, index, ']', &dFnt, MBF) && i < 8; index++, i++)
						lang[i] = ch[index];
					lang[i] = EOS;
					if (i == 0)
						i = NUMLANGUAGESINTABLE;
					else {
						for (i=0; LanguagesTable[i] != EOS && i < NUMLANGUAGESINTABLE; i++) {
							if (!uS.mStricmp(lang, LanguagesTable[i])) {
								tDefLanguage = i;
								break;
							}
						}
					}
					if (i >= NUMLANGUAGESINTABLE) {
						fprintf(stderr,"*** File \"%s\": ", oldfname);
						fprintf(stderr,"line %ld.\n", lineno);
						fprintf(stderr,"%s%s", sp, ch);
						if (ch[strlen(ch)-1] != '\n')
							fputc('\n', stderr);
						fprintf(stderr,"Language \"%s\" is not defined in \"@Languages:\" header tier.\n", lang);
						if (CLAN_PROG_NUM != CHECK)
#ifdef UNX
							exit(1);
#else
							isKillProgram = 2;
#endif
						return;
					}
					for (j=index; ch[j] != EOS && !uS.isRightChar(ch, j, ']', &dFnt, MBF); j++) ;
					if (ch[j] != EOS) {
						j++;
						while (ch[j] != EOS && uS.isskip(ch,j,&dFnt,MBF) && !uS.isRightChar(ch,j,'[',&dFnt,MBF))
							j++;
						att_cp(0,ch+k,ch+j,att+k,att+j);
						index = k;
					}
				} else if (ch[index+1] == ':' && (isSpace(ch[index+2]) || ch[index+2] == '*' || ch[index+2] == '=')) {
					for (; !isSpace(ch[index]) && ch[index] != EOS && !uS.isRightChar(ch, index, ']', &dFnt, MBF); index++) ;
					if (uS.isRightChar(ch, index, ']', &dFnt, MBF))
						index++;
				} else {
					for (index++; ch[index] != EOS && !uS.isRightChar(ch, index, ']', &dFnt, MBF); index++) ;
					if (ch[index] != EOS)
						index++;
				}
			} else if (uS.isRightChar(ch, index, '+', &dFnt, MBF) || uS.isRightChar(ch, index, '-', &dFnt, MBF) || uS.IsUtteranceDel(ch, index)) {
				while (ch[++index] != EOS && (!uS.isskip(ch,index,&dFnt,MBF) ||
									uS.isRightChar(ch,index,'/',&dFnt,MBF) || uS.isRightChar(ch,index,'<',&dFnt,MBF) ||
									uS.isRightChar(ch,index,'.',&dFnt,MBF) || uS.isRightChar(ch,index,'!',&dFnt,MBF) ||
									uS.isRightChar(ch,index,'?',&dFnt,MBF) || uS.isRightChar(ch,index,',',&dFnt,MBF))) {
				}
			} else if (ch[index] == HIDEN_C) {
				for (index++; ch[index] != EOS && ch[index] != HIDEN_C; index++) ;
				if (ch[index] != EOS)
					index++;
			} else if (uS.isRightChar(ch, index, '&', &dFnt, MBF)) {
				for (index++; ch[index] != EOS && !uS.isskip(ch, index, &dFnt, MBF); index++) ;
			} else if (uS.isRightChar(ch, index, ',', &dFnt, MBF) && uS.isRightChar(ch, index+1, ',', &dFnt, MBF)) {
				index += 2;
			} else if (uS.isRightChar(ch, index, ':', &dFnt, MBF)) {
				index++;
			} else if (uS.isRightChar(ch, index, '(', &dFnt, MBF) && uS.isPause(ch, index, NULL,  &index)) {
				index++;
			} else if ((i=uS.HandleCAChars(ch+index, NULL))) {
				index += i;
			} else if (uS.atUFound(ch, index, &dFnt, MBF)) {
				while (ch[++index] != EOS && (!uS.isskip(ch,index,&dFnt,MBF) ||
									uS.isRightChar(ch,index,'.',&dFnt,MBF) || uS.isRightChar(ch,index,'!',&dFnt,MBF) ||
									uS.isRightChar(ch,index,'?',&dFnt,MBF) || uS.isRightChar(ch,index,',',&dFnt,MBF))) {
				}
			} else {
				oIndex = -1;
				isAddLang = (LanguagesTable[1][0] != EOS);
				for (; ch[index] != EOS && !uS.isskip(ch,index,&dFnt,MBF); index++) {
					if (uS.isRightChar(ch, index, '(', &dFnt, MBF) && uS.isPause(ch, index, NULL,  &i)) {
						break;
					} else if (uS.IsUtteranceDel(ch, index)) {
						break;
					} else if (ch[index] == HIDEN_C) {
						break;
					} else if ((i=uS.HandleCAChars(ch+index, NULL))) {
						if (oIndex < 0)
							oIndex = index;
						index = index + i - 1;
					} else if (ch[index] == '@' && ch[index+1] == 's' && (ch[index+2] == ':' || uS.isskip(ch,index+2,&dFnt,MBF))) {
						oIndex = -1;
						index += 2;
						isAddLang = FALSE;
						if (ch[index] != ':') {
							if (tDefLanguage == 0)
								pLang = LanguagesTable[1];
							else if (tDefLanguage == 1)
								pLang = LanguagesTable[0];
							else {
								fprintf(stderr,"*** File \"%s\": ", oldfname);
								fprintf(stderr,"line %ld.\n", lineno);
								fprintf(stderr,"%s%s", sp, ch);
								if (ch[strlen(ch)-1] != '\n')
									fputc('\n', stderr);
								fprintf(stderr,"Illegal use of \"@s\" suffix, default font \"%s\" is in wrong position on @Language: tier.\n", LanguagesTable[tDefLanguage]);
								if (CLAN_PROG_NUM != CHECK)
#ifdef UNX
									exit(1);
#else
									isKillProgram = 2;
#endif
								return;
							}
							if (pLang[0] == EOS) {
								fprintf(stderr,"*** File \"%s\": ", oldfname);
								fprintf(stderr,"line %ld.\n", lineno);
								fprintf(stderr,"%s%s", sp, ch);
								if (ch[strlen(ch)-1] != '\n')
									fputc('\n', stderr);
								fprintf(stderr,"Illegal use of \"@s\" suffix, no alternative font in position %d defined on @Language: tier.\n",((tDefLanguage == 1) ? 0 : 1));
								if (CLAN_PROG_NUM != CHECK)
#ifdef UNX
									exit(1);
#else
									isKillProgram = 2;
#endif
								return;
							}
							att_shiftright(ch+index, att+index, strlen(pLang)+1);
							ch[index++] = ':';
							for (i=0; pLang[i] != EOS; i++)
								ch[index++] = pLang[i];
							index--;
						} else {
							do {
								lang[0] = EOS;
								for (i=0, index++; ch[index] != EOS && isalpha(ch[index]) && i < 8; index++, i++)
									lang[i] = ch[index];
								lang[i] = EOS;
								if (i == 0)
									i = NUMLANGUAGESINTABLE;
								else {
									for (i=0; LanguagesTable[i] != EOS && i < NUMLANGUAGESINTABLE; i++) {
										if (!uS.mStricmp(lang, LanguagesTable[i]))
											break;
									}
								}
								if (i >= NUMLANGUAGESINTABLE) {
									fprintf(stderr,"*** File \"%s\": ", oldfname);
									fprintf(stderr,"line %ld.\n", lineno);
									fprintf(stderr,"%s%s", sp, ch);
									if (ch[strlen(ch)-1] != '\n')
										fputc('\n', stderr);
									fprintf(stderr,"Language \"@s:%s\" is not defined in \"@Languages:\" header tier.\n", lang);
									if (CLAN_PROG_NUM != CHECK)
#ifdef UNX
										exit(1);
#else
										isKillProgram = 2;
#endif
									return;
								}
								if (ch[index] == '&' || ch[index] == '+')
									;
								else {
									index--;
									break;
								}
							} while (1) ;
						}
					} else
						oIndex = -1;
				}
				if (isAddLang) {
					if (oIndex >= 0)
						j = oIndex;
					else
						j = index;
					if (tDefLanguage >= 0 && tDefLanguage < NUMLANGUAGESINTABLE && LanguagesTable[tDefLanguage][0] != EOS) {
						att_shiftright(ch+j, att+j, strlen(LanguagesTable[tDefLanguage])+3);
						att[j] = att[j-1];
						ch[j++] = '@';
						att[j] = att[j-1];
						ch[j++] = 's';
						att[j] = att[j-1];
						ch[j++] = ':';
						index += 3;
						for (i=0; LanguagesTable[tDefLanguage][i] != EOS; i++) {
							att[j] = att[j-1];
							ch[j++] = LanguagesTable[tDefLanguage][i];
							index++;
						}
					}
				}
			}
		}
/*
		if (!isPrecodeFound) {
			if (tDefLanguage >= 0 && tDefLanguage < NUMLANGUAGESINTABLE && LanguagesTable[tDefLanguage][0] != EOS) {
				j = startIndex;
				att_shiftright(ch+j, att+j, strlen(LanguagesTable[tDefLanguage])+5);
				att[j] = 0;
				ch[j++] = '[';
				att[j] = att[j-1];
				ch[j++] = '-';
				att[j] = att[j-1];
				ch[j++] = ' ';
				for (i=0; LanguagesTable[tDefLanguage][i] != EOS; i++) {
					att[j] = att[j-1];
					ch[j++] = LanguagesTable[tDefLanguage][i];
				}
				att[j] = att[j-1];
				ch[j++] = ']';
				att[j] = att[j-1];
				ch[j++] = ' ';
			}
		}
*/
	}
}

static char isSpeakerMarker(char ch) {
	if (isKillProgram)
		return(FALSE);
	else if (CLAN_PROG_NUM != CHECK && CLAN_PROG_NUM != CHSTRING) {
		if (ch != '\n' && ch != '\r' && !isSpace(ch))
			return(TRUE);
	} else {
		if (isSpeaker(ch))
			return(TRUE);
	}
	return(FALSE);
}

/* cutt_getline(ch, ccnt) reads in character by character until either end of file
   or new tier has been reached. The characters are read into string pointed
   to by "ch". At the and "currentchar" is set to the first character of a
   new tier or to EOF marker. The character count is kept to make sure that
   the function stays within allocated memory limits.
*/
void cutt_getline(const char *sp, char *ch, AttTYPE *att, register int index) {
	register int  lc;
	register int  s;

	if (!chatmode) {
		if (lineno > -1L)
			lineno = lineno + tlineno;
		tlineno = 0L;
	}
	if (getrestline) {
		s = index;
		lc = index;
		ch[index] = currentchar;
		att[index] = currentatt;
		index++;
		att[index] = att[index-1];
		while (1) {
			if (ch[lc] == '\n')
				tlineno++;
			ch[index] = (char)getc_cr(fpin, &att[index]);
			if (!chatmode) {
				if (feof(fpin))
					break;
				else if (y_option == 1) {
					if (uS.IsUtteranceDel(ch, index)) {
						do {
							index++;
							att[index] = att[index-1];
							ch[index] = (char)getc_cr(fpin, &att[index]);
						} while (uS.IsUtteranceDel(ch, index)) ;
						do {
							if (ch[index] == '\n') tlineno++;
							index++;
							att[index] = att[index-1];
							ch[index] = (char)getc_cr(fpin, &att[index]);
						} while (uS.isskip(ch, index, &dFnt, MBF) && !feof(fpin)) ;
						break;
					}
				} else {
					if (ch[lc] == '\n')
						break;
				}
			} else if (feof(fpin))
				break;
			else if (isSpeakerMarker(ch[index])) {
				if (ch[lc] == '\n')
					break;
			}
			lc = index;

			if (index >= UTTLINELEN) {
				int i;
				ch[index] = EOS;
				fprintf(stderr,"ERROR. Speaker turn is longer than ");
				fprintf(stderr,"%ld characters.\n",UTTLINELEN);
				fprintf(stderr,"On line: %ld.\n", lineno);
				for (i=0; ch[i]; i++) putc(ch[i],stderr);
				putc('\n',stderr);
				cutt_exit(-1);
				index--;
			} else {
				index++;
				att[index] = att[index-1];
			}
		}
		currentchar = ch[index];
		currentatt = att[index];
		ch[index] = EOS;
		if (IsModUttDel && chatmode)
			cutt_cleanUpLine(sp, ch, att, s);
	} else if (skipgetline == FALSE) {
		ch[index] = '\n';
		att[index] = 0;
		index++;
		ch[index] = EOS;
	} else
		skipgetline = FALSE;
	getrestline = 1;
}

/* killline(char *line, char *atts) skip all data in the input stream starting from the current
   character until either end of file or new tier has been reached.  At the
   and "currentchar" is set to the first character of a new tier or to EOF
   marker.
*/
void killline(char *line, AttTYPE *atts) {
	register char lc = 0;

	if (getrestline) {
		if (line == NULL) {
			char *chrs;
			AttTYPE att;

			att = currentatt;
			chrs = templineC3;
			*chrs = currentchar;
			while (1) {
				*++chrs = (char)getc_cr(fpin, &att);
				if (*chrs == '\n')
					tlineno++;
				if (feof(fpin))
					break;
				else if (isSpeakerMarker(*chrs)) {
					if (lc == '\n')
						break;
				}
				lc = *chrs;
			}
			currentchar = *chrs;
			currentatt = att;
			*chrs = EOS;
			if (uS.partwcmp(templineC3, FONTMARKER))
				cutt_SetNewFont(templineC3,']');
		} else {
			int pos;

			line[0] = currentchar;
			atts[0] = currentatt;
			pos = 1;
			atts[pos] = atts[pos-1];
			while (1) {
				line[pos] = (char)getc_cr(fpin, &atts[pos]);
				if (line[pos] == '\n')
					tlineno++;
				if (feof(fpin))
					break;
				else if (isSpeakerMarker(line[pos])) {
					if (lc == '\n')
						break;
				}
				lc = line[pos];
				if (pos >= UTTLINELEN) {
					int i;
					line[pos] = EOS;
					fprintf(stderr,"ERROR. Speaker turn is longer than ");
					fprintf(stderr,"%ld characters.\n",UTTLINELEN);
					fprintf(stderr,"On line: %ld.\n", lineno);
					for (i=0; line[i]; i++)
						putc(line[i],stderr);
					putc('\n',stderr);
					cutt_exit(-1);
					pos--;
				} else {
					pos++;
					atts[pos] = atts[pos-1];
				}
			}
			currentchar = line[pos];
			currentatt = atts[pos];
			line[pos] = EOS;
			if (uS.partwcmp(line, FONTMARKER))
				cutt_SetNewFont(line,']');
		}
	} else
		skipgetline = FALSE;
	getrestline = 1;
}

/**************************************************************/
/*	 Tier print routines for programs with windows   */
/* befprintout(char isChangeBullets) displays specified number of tiers before the current tier.
   -w option is used to specify number of tiers to be displayed before the
   target one.
*/
void befprintout(char isChangeBullets) {
	UTTER *temp;
	int w;

	w = 0;
	if (lutter != utterance) {
		temp = utterance;
		do {
			temp = temp->nextutt;
			w++;
		} while (temp != lutter) ;
	}
	for (temp=lutter->nextutt; w < aftwin; w++)
		temp = temp->nextutt;
	for (; temp != utterance; temp=temp->nextutt) {
		if (chatmode && *temp->speaker) {
			if (nomain)
				remove_main_tier_print(temp->speaker, temp->line, temp->attLine);
			else {
				if (cMediaFileName[0] != EOS)
					changeBullet(temp->line, temp->attLine);
				printout(temp->speaker,temp->line,temp->attSp,temp->attLine,FALSE);
			}
		} else if (!chatmode && *temp->line) {
			if (cMediaFileName[0] != EOS && isChangeBullets)
				changeBullet(temp->line, temp->attLine);
			printout(NULL,temp->line,NULL,temp->attLine,FALSE);
		}
	}
}

/* aftprintout() displays specified number of tiers after the current tier.
  +w option is used to specify number of tiers to be displayed after the
   target one.
*/
void aftprintout(char isChangeBullets) {
	UTTER *temp, *oldlutter;
	int w;

	w = 0;
	if (lutter != utterance) {
		temp = utterance;
		do {
			temp = temp->nextutt;
			w++;
			if (*temp->speaker) {
				if (nomain)
					remove_main_tier_print(temp->speaker, temp->line, temp->attLine);
				else {
					if (cMediaFileName[0] != EOS && isChangeBullets)
						changeBullet(temp->line, temp->attLine);
					printout(temp->speaker,temp->line,temp->attSp,temp->attLine,FALSE);
				}
			}
		} while (temp != lutter) ;
	}
	for (; w < aftwin; w++) {
		oldlutter = lutter;
		lutter = lutter->nextutt;
		if (chatmode) {
			postcodeRes = 0;
			if (!getmaincode()) {
				lutter = oldlutter;
				return;
			}
		} else {
			postcodeRes = 0;
			if (!gettextspeaker()) {
				lutter = oldlutter;
				return;
			}
			IsModUttDel = chatmode < 3;
			cutt_getline("*", lutter->line, lutter->attLine, 0);
			strcpy(lutter->tuttline,lutter->line);
			if (FilterTier > 0) {
				if (uS.isskip("[", 0, &dFnt, FALSE) || uS.isskip("]", 0, &dFnt, FALSE))
					filterData("*",lutter->tuttline);
				else
					filterwords("*",lutter->tuttline,excludedef);
			}
		}
		lutter->tlineno = lineno;
		lutter->uttLen = 0L;
		if (nomain)
			remove_main_tier_print(lutter->speaker, lutter->line, lutter->attLine);
		else {
			if (cMediaFileName[0] != EOS && isChangeBullets)
				changeBullet(lutter->line, lutter->attLine);
			printout(lutter->speaker,lutter->line,lutter->attSp,lutter->attLine,FALSE);
		}
	}
}

/**************************************************************/
/*	 printint in chat format								  */
long DealWithAtts_cutt(char *line, long i, AttTYPE att, AttTYPE oldAtt) {
	if (att != oldAtt) {
		if (is_underline(att) != is_underline(oldAtt)) {
			if (is_underline(att))
				sprintf(line+i, "%c%c", ATTMARKER, underline_start);
			else
				sprintf(line+i, "%c%c", ATTMARKER, underline_end);
			i += 2;
		}
		if (is_italic(att) != is_italic(oldAtt)) {
			if (is_italic(att))
				sprintf(line+i, "%c%c", ATTMARKER, italic_start);
			else
				sprintf(line+i, "%c%c", ATTMARKER, italic_end);
			i += 2;
		}
		if (is_bold(att) != is_bold(oldAtt)) {
			if (is_bold(att))
				sprintf(line+i, "%c%c", ATTMARKER, bold_start);
			else
				sprintf(line+i, "%c%c", ATTMARKER, bold_end);
			i += 2;
		}
		if (is_error(att) != is_error(oldAtt)) {
			if (is_error(att))
				sprintf(line+i, "%c%c", ATTMARKER, error_start);
			else
				sprintf(line+i, "%c%c", ATTMARKER, error_end);
			i += 2;
		}
		if (is_word_color(att) != is_word_color(oldAtt)) {
			char color;

			color = get_color_num(att);
			if (color) {
				if (color == blue_color)
					sprintf(line+i, "%c%c", ATTMARKER, blue_start);
				else if (color == red_color)
					sprintf(line+i, "%c%c", ATTMARKER, red_start);
				else if (color == green_color)
					sprintf(line+i, "%c%c", ATTMARKER, green_start);
				else // if (color == magenta_color)
					sprintf(line+i, "%c%c", ATTMARKER, magenta_start);
			} else
				sprintf(line+i, "%c%c", ATTMARKER, color_end);
			i += 2;
		}
	}
	return(i);
}

void printAtts(AttTYPE att, AttTYPE oldAtt, FILE *fp) {
	if (att != oldAtt) {
		if (is_underline(att) != is_underline(oldAtt)) {
			if (is_underline(att))
				fprintf(fp, "%c%c", ATTMARKER, underline_start);
			else
				fprintf(fp, "%c%c", ATTMARKER, underline_end);
		}
		if (is_italic(att) != is_italic(oldAtt)) {
			if (is_italic(att))
				fprintf(fp, "%c%c", ATTMARKER, italic_start);
			else
				fprintf(fp, "%c%c", ATTMARKER, italic_end);
		}
		if (is_bold(att) != is_bold(oldAtt)) {
			if (is_bold(att))
				fprintf(fp, "%c%c", ATTMARKER, bold_start);
			else
				fprintf(fp, "%c%c", ATTMARKER, bold_end);
		}
		if (is_error(att) != is_error(oldAtt)) {
			if (is_error(att))
				fprintf(fp, "%c%c", ATTMARKER, error_start);
			else
				fprintf(fp, "%c%c", ATTMARKER, error_end);
		}

		if (is_word_color(att) != is_word_color(oldAtt)) {
			char color;

			color = get_color_num(att);
			if (color) {
				if (color == blue_color)
					fprintf(fp, "%c%c", ATTMARKER, blue_start);
				else if (color == red_color)
					fprintf(fp, "%c%c", ATTMARKER, red_start);
				else if (color == green_color)
					fprintf(fp, "%c%c", ATTMARKER, green_start);
				else // if (color == magenta_color)
					fprintf(fp, "%c%c", ATTMARKER, magenta_start);
			} else
				fprintf(fp, "%c%c", ATTMARKER, color_end);
		}
	}
}

static void checkPrintFont(const char *st, char isChangeScreenOuput) {
	for (; *st == '\n'; st++) ;
	for (; *st != EOS && *st != '\n'; st++) {
		if (uS.partwcmp(st, FONTMARKER)) {
			cutt_SetNewFont(st,']');
#ifndef UNX
			if (isChangeScreenOuput && global_df != NULL)
				copyNewToFontInfo(&global_df->row_txt->Font, &dFnt);
#endif
			break;
		}
	}
}

void ActualPrint(const char *st, AttTYPE *att, AttTYPE *oldAtt, char justFirstAtt, char needCR, FILE *fp) {
	char lastCR = FALSE;

	while (*st) {
		if (att != NULL)
			printAtts(*att, *oldAtt, fp);
		else
			printAtts(0, *oldAtt, fp);
		if (*st == '\n') {
			checkPrintFont(st, FALSE);
			lastCR = TRUE;
		} else
			lastCR = FALSE;
		putc(*st,fp);
		st++;
		if (att != NULL) {
			*oldAtt = *att;
			if (!justFirstAtt)
				att++;
		} else
			*oldAtt = 0;
	}
	if (!lastCR && needCR) {
		printAtts(0, *oldAtt, fp);
		putc('\n',fp);
	}
}

/* printout(sp,line) displays tier properly indented according to the CHAT
   format. "sp" points to the code identification part, "line" points to
   the text line.
*/
void printout(const char *sp, char *line, AttTYPE *attSp, AttTYPE *attLine, char format) {
	register long colnum;
	register long splen = 0;
	register AttTYPE oldAtt;
	AttTYPE *posAtt, *tposAtt;
	char *pos, *tpos, *s, first = TRUE, sb = FALSE, isLineSpace, isBulletFound, BulletFound;

	oldAtt = 0;
	if (line != NULL)
		checkPrintFont(line, fpout == stdout);
	if (!format) {
		if (sp == NULL) {
			if (line != NULL)
				ActualPrint(line, attLine, &oldAtt, FALSE, TRUE, fpout);
		} else {
			if (*sp != EOS) {
				ActualPrint(sp, attSp, &oldAtt, FALSE, FALSE, fpout);
				splen = strlen(sp) - 1;
			} else
				splen = 0;

			if (sp[splen] == '\n' && *line == EOS) ;
			else if (line != NULL) {
				if (sp[0] == '@' && *line == EOS) ;
				else if (!isSpace(sp[splen]) && sp[0] != EOS && line[0] != '\n')
					putc('\t',fpout);
				ActualPrint(line, attLine, &oldAtt, FALSE, TRUE, fpout);
			}
		}
		return;
	}
	if (sp != NULL) {
		for (colnum=0; sp[colnum] != EOS; colnum++) {
			if (sp[colnum] == '\t')
				splen = ((splen / TabSize) + 1) * TabSize;
			else
				splen++;
		}
		ActualPrint(sp, attSp, &oldAtt, FALSE, FALSE, fpout);
		if (line != NULL) {
			if (colnum != 0 && !isSpace(sp[colnum-1]) && line[0] != EOS && line[0] != '\n') {
				putc('\t',fpout);
				splen = ((splen / TabSize) + 1) * TabSize;
			}
		}
	}
	if (line == NULL) {
		printAtts(0, oldAtt, fpout);
		putc('\n',fpout);
		return;
	}
	if (isSpace(*line))
		*line = ' ';
	colnum = splen;
	tpos = line;
	tposAtt = attLine;
	isLineSpace = *line;
	BulletFound = FALSE;
	while (*line != EOS) {
		pos = line;
		posAtt = attLine;
		colnum++;
		if (*line == '[')
			sb = TRUE;
		if (BulletFound) {
			BulletFound = FALSE;
			if (!cutt_isMultiFound)
				colnum = MAXOUTCOL + 1;
		}
		isBulletFound = FALSE;
		while (((*line != ' ' && *line != '\t' && *line != '\n') || sb || isBulletFound) && *line != EOS) {
			if (*line == HIDEN_C) {
				isBulletFound = !isBulletFound;
				BulletFound = TRUE;
			}
			line++;
			if (attLine != NULL)
				attLine++;
			if ((*line == '\n' && sb) || *line == '\t') *line = ' ';
			else if (*line == '[') sb = TRUE;
			else if (*line == ']') sb = FALSE;
			if (!isBulletFound && (UTF8_IS_SINGLE((unsigned char)*line) || UTF8_IS_LEAD((unsigned char)*line)))
				colnum++;
		}
		isLineSpace = *line;
		if (*line != EOS) {
			*line++ = EOS;
			if (attLine != NULL)
				attLine++;
		}
		if (colnum > MAXOUTCOL) {
			if (first)
				first = FALSE;
			else {
				if (tposAtt != NULL) {
					while (isSpace(*tpos)) {
						tpos++;
						printAtts(*tposAtt, oldAtt, fpout);
						oldAtt = *tposAtt;
						tposAtt++;
					}
					if (*tpos == '\n') {
						tpos++;
						tposAtt++;
					}
				}
				printAtts(0, oldAtt, fpout);
				oldAtt = 0;
				putc('\n',fpout);
				if (tposAtt != NULL) {
					while (isSpace(*tpos)) {
						tpos++;
						printAtts(*tposAtt, oldAtt, fpout);
						oldAtt = *tposAtt;
						tposAtt++;
					}
				}
				putc('\t',fpout);
				colnum = splen;
				isBulletFound = FALSE;
				for (s=pos; *s != EOS; s++) {
					if (*s == HIDEN_C)
						isBulletFound = !isBulletFound;
					if (!isBulletFound && (UTF8_IS_SINGLE((unsigned char)*s) || UTF8_IS_LEAD((unsigned char)*s)))
						colnum++;
				}
			}
		} else if (!first) {
			if (tposAtt != NULL) {
				while (isSpace(*tpos) || *tpos == '\n') {
					tpos++;
					printAtts(*tposAtt, oldAtt, fpout);
					oldAtt = *tposAtt;
					tposAtt++;
				}
			}
			putc(' ',fpout);
		} else
			first = FALSE;
		if (pos != line)
			ActualPrint(pos, posAtt, &oldAtt, FALSE, FALSE, fpout);
		if (isLineSpace != EOS) {
			line--;
			*line = isLineSpace;
			if (attLine != NULL)
				attLine--;
		}
		tpos = line;
		tposAtt = attLine;
		while (isSpace(*line) || *line == '\n') {
			line++;
			if (attLine != NULL)
				attLine++;
		}
	}
	printAtts(0, oldAtt, fpout);
	putc('\n',fpout);
}

static char isTierLabel(char *line, char isFirstChar) {
	if (isSpeaker(*line) && (*(line-1) =='\n' || *(line-1) == '\r' || isFirstChar)) {
		for (; *line != ':' && *line != '\n' && *line != '\r' && *line != EOS; line++) ;
		if (*line == ':')
			return(TRUE);
	}
	return(FALSE);
}

void changeBullet(char *line, AttTYPE *att) {
	long i, j;

	for (i=0; line[i] != EOS; i++) {
		if (line[i] == HIDEN_C && isdigit(line[i+1])) {
			i++;
			if (att == NULL)
				uS.shiftright(line+i, strlen(cMediaFileName)+strlen(SOUNDTIER)+3);
			else
				att_shiftright(line+i, att+i, strlen(cMediaFileName)+strlen(SOUNDTIER)+3);
			for (j=0L; SOUNDTIER[j] != EOS; j++)
				line[i++] = SOUNDTIER[j];
			line[i++] = '"';
			for (j=0L; cMediaFileName[j] != EOS; j++)
				line[i++] = cMediaFileName[j];
			line[i++] = '"';
			line[i++] = '_';
		}
	}
}

void remove_CRs_Tabs(char *line) {
	long i;

	for (i=0L; line[i] != EOS; ) {
		if (line[i] == '\n')
			strcpy(line+i, line+i+1);
		else if (line[i] == '\t')
			line[i] = ' ';
		else
			i++;
	}
}

void remove_main_tier_print(const char *sp, char *line, AttTYPE *att) {
	char isBulletFound;
	long i, j;

	i = 0L;
	j = 0L;
	isBulletFound = FALSE;
	if (*sp == '*' && (!isTierLabel(line+i, TRUE) || line[i] != '%')) {
		while (!isTierLabel(line+i, FALSE) && line[i] != EOS)
			i++;
	}

	for (; line[i] != EOS; i++) {
		if (line[i] == '*' && isTierLabel(line+i, FALSE)) {
			do {
				i++;
			} while (!isTierLabel(line+i, FALSE) && line[i] != EOS) ;
			if (line[i] == EOS)
				break;
		}

		if ((line[i-1] =='\n' || line[i-1] == '\r') && !isTierLabel(line+i, FALSE)) {
			if (line[i] == ' ')
				line[i] = '\t';
			else if (!isSpace(line[i])) {
				tempTier[j] = '\t';
				if (att != NULL)
					tempAtt[j] = att[i];
				else
					tempAtt[j] = 0;
				j++;
			}
		}
		if (line[i] == HIDEN_C)
			isBulletFound = TRUE;
		tempTier[j] = line[i];
		if (att != NULL)
			tempAtt[j] = att[i];
		else
			tempAtt[j] = 0;
		j++;
	}
	tempTier[j] = EOS;
	if (isBulletFound && cMediaFileName[0] != EOS)
		changeBullet(tempTier, tempAtt);
	if (CLAN_PROG_NUM == KWAL && onlydata == 5) {
		remove_CRs_Tabs(tempTier);
		fprintf(fpout, "%s", tempTier);
	} else
		printout(NULL,tempTier,NULL,tempAtt,FALSE);
}
/**************************************************************/
/*	 get whole tier and filterData it					  */
/* rightrange(sp,uttline) determines if the data in uttline is in the user
   selected range, specified by +z option. It returns 1 if the data is
   within the selected range and 0 otherwise.
*/
void cleanUttline(char *line) {
	int  k;
	char spfound;

	for (k=0; line[k] == ' ' || line[k] == '\t' || 
		   line[k] == '\n'; k++) ;
	if (k > 0)
		strcpy(line, line+k);
	for (k=0; line[k]; k++) {
		if (line[k] == '\n')
			line[k] = ' ';
		if (line[k] == ' ' || line[k] == '\t') {
			if (!spfound)
				spfound = TRUE; 
			else {
				strcpy(line+k, line+k+1);
				k--;
			}
		} else {
			spfound = FALSE; 
			if (line[k] == '<' || line[k] == '>') {
				strcpy(line+k, line+k+1);
				k--;
			}
		}
	}
}

static char hasSpeakerChanged(char *curSP, char *oldSP, char *curLine, char isChosen) {
	register char sq;
	int i, j;

	if (oldSP == NULL)
		return(FALSE);

	if (*curSP == '*' && (CntWUT || CntFUttLen)) {
		i = strlen(curSP) - 1;
		for (; i >= 0 && isSpace(curSP[i]); i--) ;
		curSP[i+1] = EOS;
		if (strcmp(oldSP,curSP) != 0) {
			sq = FALSE;
			for (j=0; curLine[j]; j++) {
				if (uS.isRightChar(curLine, j, '[', &dFnt, MBF))
					sq = TRUE;
				else if (uS.isRightChar(curLine, j, ']', &dFnt, MBF))
					sq = FALSE;
				if (!sq && !uS.isskip(curLine, j, &dFnt, MBF) && !uS.IsUtteranceDel(curLine, j)) {
					strcpy(oldSP,curSP);
					return(TRUE);
				}
			}
		}
		if (!isChosen)
			strcpy(oldSP,curSP);
	}
	return(FALSE);
}

// if (isTierContSymbol(utterance->line, pos, TRUE))
// if (isTierContSymbol(utterance->line, pos, FALSE))
char isTierContSymbol(char *line, int i, char isForced) {
	if (line[i] == '+' && (i == 0 || uS.isskip(line,i-1,&dFnt,MBF))) {
		if (isForced) {
			if (uS.isRightChar(line,i+1,'.',&dFnt,MBF) &&
				  (line[i+2] == EOS || (uS.isskip(line,i+2,&dFnt,MBF) && !uS.IsUtteranceDel(line,i+2))))
				return(TRUE);
		} else {
			if (line[i+1] == '/') {
				if (line[i+2] == '.' && (uS.isskip(line,i+3,&dFnt,MBF) || line[i+3] == EOS))
					return(TRUE);
				else if (line[i+2] == '/' && line[i+3] == '.' && (uS.isskip(line,i+4,&dFnt,MBF) || line[i+4] == EOS))
					return(TRUE);
				else if (line[i+2] == '?' && (uS.isskip(line,i+3,&dFnt,MBF) || line[i+3] == EOS))
					return(TRUE);
				else if (line[i+2] == '/' && line[i+3] == '?' && (uS.isskip(line,i+4,&dFnt,MBF) || line[i+4] == EOS))
					return(TRUE);
			} else if (line[i+1] == '.') {
				if (line[i+2] == '.' && line[i+3] == '.' && (uS.isskip(line,i+4,&dFnt,MBF) || line[i+4] == EOS))
					return(TRUE);
				else if (line[i+2] == '.' && line[i+3] == '?' && (uS.isskip(line,i+4,&dFnt,MBF) || line[i+4] == EOS))
					return(TRUE);
			}
		}
	}
	return(FALSE);
}

int rightrange(char sp, char *tLine, char *uttline) {
	const char *speaker;
	char *line, sq = FALSE, delf = TRUE;
	int pos;
	int fn = FALSE;

	if (sp == '@')
		return(TRUE);
	if (sp == '*') {
		foundUttContinuation = FALSE;
	}
	if (CntWUT == 2 && chatmode && sp == '*') {
		pos = 0;
		speaker = utterance->speaker;
		line = utterance->line;
		for (pos=0; tLine[pos] != EOS; pos++) {
			if (uS.isRightChar(line, pos, '[', &dFnt, MBF)) {
				while (!uS.isRightChar(line, pos, ']', &dFnt, MBF) && line[pos] != EOS)
					pos++;
				if (line[pos] == EOS)
					pos--;
			} else {
				if ((pos == 0 || uS.isskip(line,pos-1,&dFnt,MBF)) &&
					  line[pos] == '+' && uS.isRightChar(line,pos+1,',',&dFnt, MBF) &&
					  uS.partcmp(speaker, contSpeaker, FALSE, FALSE) && contSpeaker[0] != EOS) {
					foundUttContinuation = TRUE;
					contSpeaker[0] = EOS;
				} else if (isTierContSymbol(line, pos, TRUE)) {
					foundUttContinuation = TRUE;
					contSpeaker[0] = EOS;
					pos = pos + 1;
				} else if (isTierContSymbol(line, pos, FALSE)) {
					if (WUCounter > 0L) {
						strcpy(contSpeaker, speaker);
						uS.remblanks(contSpeaker);
					}
				}
			}
		}
	}
	if (CLAN_PROG_NUM == MLU || CLAN_PROG_NUM == MLT || CLAN_PROG_NUM == FREQ) {
		if ((sp != '*' && !nomain) || (sp == '*' && nomain))
			return(TRUE);
	} else
		if (sp != '*')
			return(TRUE);
	if (CntWUT == 3) { /* turn */
		if (chatmode) {
			if (hasSpeakerChanged(utterance->speaker, Toldsp, utterance->tuttline, TRUE)) {
				Tspchanged = TRUE;
			}
			for (pos=0; tLine[pos]; pos++) {
				if (!uS.isskip(tLine, pos, &dFnt, MBF)) {
					if (Tspchanged) {
						WUCounter = WUCounter + 1L;
					}
					Tspchanged = FALSE;
					if (WUCounter < FromWU || (WUCounter > ToWU && ToWU)) {
						for (; *uttline; uttline++)
							*uttline = ' ';
						return(FALSE);
					} else
						return(TRUE);
				}
			}
			return(TRUE);
		} else
			return(TRUE);
	} else if (CntWUT == 2) { /* utterance */
		pos = 0;
		line = utterance->line;
		delf = TRUE;
		if (chatmode)
			speaker = utterance->speaker;
		else
			speaker = "*";
		do {
			if (uS.isRightChar(tLine, pos, '[', &dFnt, MBF)) {
				if (WUCounter < FromWU || (WUCounter > ToWU && ToWU)) {
					for (; tLine[pos] && !uS.isRightChar(tLine, pos, ']', &dFnt, MBF); pos++) {
						uttline[pos] = ' ';
					}
				} else {
					for (; tLine[pos] && !uS.isRightChar(tLine, pos, ']', &dFnt, MBF); pos++) ;
				}
				if (tLine[pos]) {
					if (WUCounter < FromWU || (WUCounter > ToWU && ToWU)) {
						uttline[pos] = ' ';
					}
				} else
					break;
			}
			if (uS.isRightChar(line, pos, '[', &dFnt, MBF))
				sq = TRUE;
			else if (uS.isRightChar(line, pos, ']', &dFnt, MBF))
				sq = FALSE;
			if (!uS.isskip(tLine,pos,&dFnt,MBF) && tLine[pos] != EOS) {
				if (uS.IsUtteranceDel(line, pos) && !sq) {
					if (line[pos-2] == '+' && line[pos-1] == '/' && line[pos] == '.' && pos > 1) {
						if (WUCounter > 0L && !delf) {
							strcpy(contSpeaker, speaker);
							uS.remblanks(contSpeaker);
						}
					}
					delf = TRUE;
				} else if (delf) {
					delf = FALSE;
					WUCounter = WUCounter + 1L;
					if (foundUttContinuation && WUCounter > 0L)
						WUCounter = WUCounter - 1L;
					if (WUCounter >= FromWU && (WUCounter <= ToWU || !ToWU))
						fn = TRUE;
				}
			} else if (uS.IsUtteranceDel(line,pos) && !sq) {
				if (line[pos-2] == '+' && line[pos-1] == '/' && line[pos] == '.' && pos > 1) {
					if (WUCounter > 0L && !delf) {
						strcpy(contSpeaker, speaker);
						uS.remblanks(contSpeaker);
					}
				}
				delf = TRUE;
			}
			if (tLine[pos]) {
				if (!uS.isRightChar(tLine, pos, '[', &dFnt, MBF)) {
					if (WUCounter < FromWU || (WUCounter > ToWU && ToWU)) {
						if (tLine[pos] != sMarkChr && tLine[pos] != dMarkChr)
							uttline[pos] = ' ';
					}
					pos++;
				}
			} else
				break;
		} while (1) ;
	} else if (CntWUT == 1) { /* word */
		pos = 0;
		if (WUCounter >= FromWU && WUCounter <= ToWU && ToWU)
			fn= TRUE;
		do {
			if (uS.isRightChar(tLine, pos, '[', &dFnt, MBF)) {
				if (WUCounter < FromWU || (WUCounter > ToWU && ToWU)) {
					for (; tLine[pos] && *tLine!=']'; pos++) {
						uttline[pos] = ' ';
					}
				} else for (; tLine[pos] && !uS.isRightChar(tLine, pos, ']', &dFnt, MBF); pos++) ;
				if (tLine[pos]) {
					if (WUCounter < FromWU || (WUCounter > ToWU && ToWU)) {
						uttline[pos] = ' ';
					}
				} else
					break;
			}
			if (!uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != EOS) {
				if (uS.IsUtteranceDel(tLine,pos)) {
					for (; uS.IsUtteranceDel(tLine,pos) && tLine[pos]; pos++) ;
				} else {
					WUCounter = WUCounter + 1L;
					for (; !uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos]; pos++) {
						if (WUCounter < FromWU || (WUCounter > ToWU && ToWU)) {
							uttline[pos] = ' ';
						} else
							fn = TRUE;
					}
				}
			}
			if (tLine[pos]) {
				if (!uS.isRightChar(tLine, pos, '[', &dFnt, MBF)) {
					if (WUCounter < FromWU || (WUCounter > ToWU && ToWU)) {
						if (tLine[pos] != sMarkChr && tLine[pos] != dMarkChr)
							uttline[pos] = ' ';
					}
					pos++;
				}
			} else break;
		} while (1) ;
	}
	return(fn);
}

void checkOptions(char *st) {
	int t;
	NewFontInfo finfo;

	for (t=0; st[t] != EOS; t++) {
		if (!strncmp(st+t, "multi", 5))
			cutt_isMultiFound = TRUE;
		else if (!strncmp(st+t, "CA", 2))
			cutt_isCAFound = TRUE;
		else if (!strncmp(st+t, "heritage", 8))
			cutt_isBlobFound = TRUE;
	}
	if (cutt_isCAFound) {
		SetDefaultCAFinfo(&finfo);
		selectChoosenFont(&finfo, FALSE);
	}
}

static char extractRightTier(const char *sp, char *line, char *tierName, char *outTier) {
	int  i, j;
	char isFound;

	i = 0;
	j = 0;
	isFound = FALSE;
	if (!nomain) {
		strcpy(tierName, sp);
		uS.remblanks(tierName);
		for (j=0; line[i] != EOS; i++) {
			outTier[j++] = line[i];
			if (line[i] == '\n' && line[i+1] == '%')
				break;
		}
		isFound = TRUE;
	} else {
		do {
			for (; line[i] != EOS; i++) {
				if (line[i] == '\n' && line[i+1] == '%')
					break;
			}
			if (line[i] != EOS)
				i++;
			for (j=0; line[i] != EOS; i++) {
				tierName[j++] = line[i];
				if (line[i] == ':')
					break;
			}
			tierName[j] = EOS;
			j = 0;
			if (line[i] == EOS)
				break;
			if (checktier(tierName)) {
				isFound = TRUE;
				if (line[i] == ':')
					i++;
				while (isSpace(line[i]))
					i++;
				for (j=0; line[i] != EOS; i++) {
					outTier[j++] = line[i];
					if (line[i] == '\n' && line[i+1] == '%')
						break;
				}
				break;
			}
		} while (line[i] != EOS) ;
	}
	outTier[j] = EOS;
	return(isFound);
}

static void correctForXXXYYYWWW(char *line, char *outTier) {
	int i;

	if (!nomain) {
		for (i=0; outTier[i] != EOS; i++) {
			if (restoreXXX) {
				if (line[i] == 'x')
					outTier[i] = line[i];
				if (line[i+1] == 'x')
					outTier[i+1] = line[i+1];
				if (line[i+2] == 'x')
					outTier[i+2] = line[i+2];
				i += 2;
			} else {
				if (outTier[i] == 'x')
					outTier[i] = ' ';
				if (outTier[i+1] == 'x')
					outTier[i+1] = ' ';
				if (outTier[i+2] == 'x')
					outTier[i+2] = ' ';
				i += 2;
			}
			if (restoreYYY) {
				if (line[i] == 'y')
					outTier[i] = line[i];
				if (line[i+1] == 'y')
					outTier[i+1] = line[i+1];
				if (line[i+2] == 'y')
					outTier[i+2] = line[i+2];
				i += 2;
			} else {
				if (outTier[i] == 'y')
					outTier[i] = ' ';
				if (outTier[i+1] == 'y')
					outTier[i+1] = ' ';
				if (outTier[i+2] == 'y')
					outTier[i+2] = ' ';
				i += 2;
			}
			if (restoreWWW) {
				if (line[i] == 'w')
					outTier[i] = line[i];
				if (line[i+1] == 'w')
					outTier[i+1] = line[i+1];
				if (line[i+2] == 'w')
					outTier[i+2] = line[i+2];
				i += 2;
			} else {
				if (outTier[i] == 'w')
					outTier[i] = ' ';
				if (outTier[i+1] == 'w')
					outTier[i+1] = ' ';
				if (outTier[i+2] == 'w')
					outTier[i+2] = ' ';
				i += 2;
			}
		}
	}
}

/* rightUttLen() determines if the data in uttline is in the user
   selected utterance length, specified by +x option. It returns 1 if the data is
   of the right length and 0 otherwise.
*/
int rightUttLen(const char *spO, char *line, char *tLineO, long *cUttLen) {
	const char *sp;
	char tmp, isMorFound, *tLine, tierName[SPEAKERLEN];
	long uttLen;
	int  pos, eCA;

	if (CLAN_PROG_NUM == MLU || CLAN_PROG_NUM == MLT || CLAN_PROG_NUM == FREQ) {
		if ((spO[0] != '*' && !nomain) || (spO[0] != '%' && nomain))
			return(TRUE);
	} else
		if (spO[0] != '*')
			return(TRUE);
	if (isWinMode) {
		if (!extractRightTier(spO, tLineO, tierName, rightUttLenC))
			return(FALSE);
		sp = tierName;
		tLine = rightUttLenC;
	} else {
		sp = spO;
		tLine = tLineO;
	}
	if (wdUttLen != NULL)
		filterwords(sp,tLine,excludeUttLen);
//2013-07-22	if (restoreXXX || restoreYYY || restoreWWW)
		correctForXXXYYYWWW(line, tLine);
	if (CntFUttLen == 3) { /* characters */
		pos = 0;
		uttLen = 0L;
		if (isWinMode && nomain && tLine[pos] == '\n' && tLine[pos+1] == '%') {
			pos++;
			while (!checktier(tLine+pos)) {
				for (; tLine[pos] && (tLine[pos] != '\n' || tLine[pos+1] != '%'); pos++) ;
				if (tLine[pos] == '\n')
					pos++;
				if (tLine[pos] == EOS)
					break;
			}
			for (; tLine[pos] && tLine[pos] != ':'; pos++) ;
			if (tLine[pos] == ':')
				pos++;
		}
		do {
			do {
				if ((eCA=uS.HandleCAChars(tLine+pos, NULL)) != 0) {
					pos += eCA;
				} else if (uS.isRightChar(tLine, pos, '[', &dFnt, MBF)) {
					for (; tLine[pos] && !uS.isRightChar(tLine, pos, ']', &dFnt, MBF); pos++) ;
					if (tLine[pos] != EOS)
						pos++;
				} else if (tLine[pos] == HIDEN_C) {
					for (; tLine[pos] && tLine[pos] != HIDEN_C; pos++) ;
					if (tLine[pos] != EOS)
						pos++;				
				} else if (tLine[pos] == ',') {
					pos++;				
				} else if (uS.isRightChar(tLine, pos, '(', &dFnt, MBF) && uS.isPause(tLine, pos, NULL, &pos)) {
					pos++;
				} else if (tLine[pos] == '-' || tLine[pos] == '+' || tLine[pos] == '0' || tLine[pos] == '&' ||
						   tLine[pos] == '#' || tLine[pos] == '.' || tLine[pos] == '?' || tLine[pos] == '!') {
					for (; !uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != EOS && tLine[pos] != ','; pos++) ;
				} else
					break;
			} while (tLine[pos] != EOS) ;
			if (tLine[pos] == EOS)
				break;
			if (!uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != EOS) {
				for (; !uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != ',' && tLine[pos] != EOS; pos++) ;
				uttLen++;
			}
			if (tLine[pos]) {
				if (isWinMode && tLine[pos] == '\n' && tLine[pos+1] == '%') {
					if (nomain) {
						pos++;
						while (!checktier(tLine+pos)) {
							for (; tLine[pos] && (tLine[pos] != '\n' || tLine[pos+1] != '%'); pos++) ;
							if (tLine[pos] == '\n')
								pos++;
							if (tLine[pos] == EOS)
								break;
						}
						for (; tLine[pos] && tLine[pos] != ':'; pos++) ;
						if (tLine[pos] == EOS)
							break;
						pos++;
					} else
						break;
				} else
					pos++;
			} else 
				break;
		} while (1) ;
	} else if (CntFUttLen == 2) { /* morpheme */
		isMorFound = FALSE;
		for (pos=0; tLine[pos]; pos++) {
			if (tLine[pos] == '\n' && tLine[pos+1] == '%') {
				pos++;
				if (uS.partcmp(tLine+pos,"%mor:",0,FALSE)) {
					isMorFound = TRUE;
					for (; tLine[pos] && tLine[pos] != ':'; pos++) ;
					if (tLine[pos] == ':')
						pos++;
					break;
				}
			}
		}
		if (!isMorFound)
			pos = 0;
		uttLen = 0L;
		do {
			if (isWinMode && tLine[pos] == '\n' && tLine[pos+1] == '%') {
				if (nomain) {
					pos++;
					while (!checktier(tLine+pos)) {
						for (; tLine[pos] && (tLine[pos] != '\n' || tLine[pos+1] != '%'); pos++) ;
						if (tLine[pos] == '\n')
							pos++;
						if (tLine[pos] == EOS)
							break;
					}
					for (; tLine[pos] && tLine[pos] != ':'; pos++) ;
					if (tLine[pos] == EOS)
						break;
					pos++;
				} else
					break;
			}
			if (uS.isRightChar(tLine, pos, '[', &dFnt, MBF)) {
				for (; tLine[pos] && !uS.isRightChar(tLine, pos, ']', &dFnt, MBF); pos++) ;
				if (!tLine[pos])
					break;
			}
			if (!uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != EOS) {
				if (uS.IsUtteranceDel(tLine,pos)) {
					for (; uS.IsUtteranceDel(tLine,pos) && tLine[pos]; pos++) ;
				} else {
					tmp = TRUE;
					while (!uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos]) {
						if (isWinMode && tLine[pos] == '\n' && tLine[pos+1] == '%') {
							if (nomain) {
								pos++;
								while (!checktier(tLine+pos)) {
									for (; tLine[pos] && (tLine[pos] != '\n' || tLine[pos+1] != '%'); pos++) ;
									if (tLine[pos] == '\n')
										pos++;
									if (tLine[pos] == EOS)
										break;
								}
								for (; tLine[pos] && tLine[pos] != ':'; pos++) ;
								if (tLine[pos] == EOS)
									break;
								pos++;
							} else
								break;
						}
						if (!uS.ismorfchar(tLine, pos, &dFnt, rootmorf, MBF)) {
							if (tmp) {
								if (tLine[pos] != '0')
									uttLen++;
								tmp = FALSE;
							}
						} else
							tmp = TRUE;
						pos++;
					}
				}
			}
			if (tLine[pos]) {
				if (isWinMode && tLine[pos] == '\n' && tLine[pos+1] == '%') {
					if (nomain) {
						pos++;
						while (!checktier(tLine+pos)) {
							for (; tLine[pos] && (tLine[pos] != '\n' || tLine[pos+1] != '%'); pos++) ;
							if (tLine[pos] == '\n')
								pos++;
							if (tLine[pos] == EOS)
								break;
						}
						for (; tLine[pos] && tLine[pos] != ':'; pos++) ;
						if (tLine[pos] == EOS)
							break;
						pos++;
					} else
						break;
				}
				if (!uS.isRightChar(tLine, pos, '[', &dFnt, MBF)) {
					pos++;
				}
			} else 
				break;
		} while (1) ;
	} else if (CntFUttLen == 1) { /* word */
		pos = 0;
		uttLen = 0L;
		if (isWinMode && nomain && tLine[pos] == '\n' && tLine[pos+1] == '%') {
			pos++;
			while (!checktier(tLine+pos)) {
				for (; tLine[pos] && (tLine[pos] != '\n' || tLine[pos+1] != '%'); pos++) ;
				if (tLine[pos] == '\n')
					pos++;
				if (tLine[pos] == EOS)
					break;
			}
			for (; tLine[pos] && tLine[pos] != ':'; pos++) ;
			if (tLine[pos] == ':')
				pos++;
		}
		do {
			do {
				if ((eCA=uS.HandleCAChars(tLine+pos, NULL)) != 0) {
					pos += eCA;
				} else if (uS.isRightChar(tLine, pos, '[', &dFnt, MBF)) {
					for (; tLine[pos] && !uS.isRightChar(tLine, pos, ']', &dFnt, MBF); pos++) ;
					if (tLine[pos] != EOS)
						pos++;
				} else if (tLine[pos] == HIDEN_C) {
					for (; tLine[pos] && tLine[pos] != HIDEN_C; pos++) ;
					if (tLine[pos] != EOS)
						pos++;				
				} else if (tLine[pos] == ',') {
					pos++;				
				} else if (uS.isRightChar(tLine, pos, '(', &dFnt, MBF) && uS.isPause(tLine, pos, NULL, &pos)) {
					pos++;
				} else if (tLine[pos] == '-' || tLine[pos] == '+' || tLine[pos] == '0' || tLine[pos] == '&' ||
						   tLine[pos] == '#' || tLine[pos] == '.' || tLine[pos] == '?' || tLine[pos] == '!') {
					if (!uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != EOS && tLine[pos] != ',') {
						for (; !uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != EOS && tLine[pos] != ','; pos++) ;
					} else
						break;
				} else
					break;
			} while (tLine[pos] != EOS) ;
			if (tLine[pos] == EOS)
				break;
			if (!uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != EOS) {
				uttLen++;
				for (; !uS.isskip(tLine, pos, &dFnt, MBF) && tLine[pos] != ',' && tLine[pos] != EOS; pos++) ;
			}
			if (tLine[pos]) {
				if (isWinMode && tLine[pos] == '\n' && tLine[pos+1] == '%') {
					if (nomain) {
						pos++;
						while (!checktier(tLine+pos)) {
							for (; tLine[pos] && (tLine[pos] != '\n' || tLine[pos+1] != '%'); pos++) ;
							if (tLine[pos] == '\n')
								pos++;
							if (tLine[pos] == EOS)
								break;
						}
						for (; tLine[pos] && tLine[pos] != ':'; pos++) ;
						if (tLine[pos] == EOS)
							break;
						pos++;
					} else
						break;
				} else
					pos++;
			} else 
				break;
		} while (1) ;
	}
	*cUttLen = uttLen;
	if ((filterUttLen_cmp == '=' && filterUttLen == uttLen) ||
		  (filterUttLen_cmp == '<' && uttLen < filterUttLen) || (filterUttLen_cmp == '>' && uttLen > filterUttLen)) {
		return(TRUE);
	}
	return(FALSE);
}

/* gettextspeaker() is used in a chatmode = 0. It is a dummy function.
*/
int gettextspeaker() {
	if (feof(fpin)) return(FALSE);
	return(TRUE);
}

static void SpeakerNameError(char *name, int i, int lim) {
	register int j;

	fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
	fprintf(stderr,"Tier name is longer than %d characters.\n",lim);
	if (name[4] == ';')
		fprintf(stderr,"Possibly ';' character should be changed to ':'.\n");
	fprintf(stderr,"Maybe ':' character is missing or '-' is used.\n");
	j = 0;
	for (; j < i; j++)
		putc(name[j],stderr);
	putc('\n',stderr);
	cutt_exit(-1);
}

/* getspeaker(chrs) gets the code identifier part of the tier and stores it
   into memory pointed by "chrs". It returns 0 if end of file has been
   reached, 1 otherwise. If the tier does not have a line part, i.e. @begin,
   or the end of file has been reached then the "getrestline" is set to 0.
   "currentchar" is set to the last character read.
*/
int getspeaker(char *sp, AttTYPE *att, register int index) {
	register int orgIndex;
	register int i;
	register int j;

	if (feof(fpin)) {
		if (ByTurn)
			ByTurn = 1;
		return(FALSE);
	}

	if (chatmode && !isSpeaker(currentchar)) {
		fprintf(stderr,"\n\n*** File \"%s\": line %ld.\n", oldfname, lineno);
		if (currentchar <= 32) {
			fprintf(stderr,"Illegal speaker character found: %x.\n", currentchar);
			if (currentchar == '\r' || currentchar == '\n')
				fprintf(stderr,"It looks like a blank line. Blank lines are not allowed in CHAT\n");
		} else
			fprintf(stderr,"Illegal speaker character found: %c.\n", currentchar);
		cutt_exit(-1);
	}

	sp[index] = currentchar;
	att[index] = currentatt;
	orgIndex = index;

	for (i=0; sp[index] != ':' && sp[index] != '\n' && !feof(fpin); i++) {
		if (i >= SPEAKERLEN) {
			SpeakerNameError(sp+orgIndex, i, SPEAKERLEN);
			index--;
		}
		index++;
		att[index] = att[index-1];
		sp[index] = (char)getc_cr(fpin, &att[index]);
/*
		if (sp[index] == '-')
			break;
*/
	}

	j = 0;
	while (sp[index] != ':' && sp[index] != '\n' && !feof(fpin)) {
		j++;
		if (i+j >= SPEAKERLEN) {
			i += j;
			SpeakerNameError(sp+orgIndex, i, SPEAKERLEN);
			index--;
		}
		index++;
		att[index] = att[index-1];
		sp[index] = (char)getc_cr(fpin, &att[index]);
	}

	if (sp[orgIndex] == '*' || sp[orgIndex] == '%') {
		if (i > 7+1 && CLAN_PROG_NUM != CHSTRING) {
			if (sp[index] == ':') {
				index++;
				att[index] = att[index-1];
				sp[index] = EOS;
			} else
				sp[index] = EOS;
			SpeakerNameError(sp+orgIndex, strlen(sp+orgIndex), 7);
		}
	}

	if (ByTurn) {
		sp[index+1] = EOS;
		if (sp[orgIndex] == '*' && strcmp(TSoldsp,sp+orgIndex) != 0) {
			if (*TSoldsp != EOS)
				ByTurn = 2;
			strcpy(TSoldsp, sp+orgIndex);
		}
	}

	if (sp[index] == ':') {
		i += j+1;
		if (i >= SPEAKERLEN) {
			SpeakerNameError(sp+orgIndex, i, SPEAKERLEN);
			index--;
		}
		index++;
		att[index] = att[index-1];
		sp[index] = (char)getc_cr(fpin, &att[index]);
		if (sp[index] == '\t'/* && CLAN_PROG_NUM == CHSTRING*/ && cutt_isCAFound) {
			i++;
			if (i >= SPEAKERLEN) {
				SpeakerNameError(sp+orgIndex, i, SPEAKERLEN);
				index--;
			}
			index++;
			att[index] = att[index-1];
			sp[index] = (char)getc_cr(fpin, &att[index]);
		} else {
			while (sp[index] == ' ' || sp[index] == '\t') {
				i++;
				if (i >= SPEAKERLEN) {
					SpeakerNameError(sp+orgIndex, i, SPEAKERLEN);
					index--;
				}
				index++;
				att[index] = att[index-1];
				sp[index] = (char)getc_cr(fpin, &att[index]);
			}
		}
	} else {
		for (i=index; (isSpace(sp[i]) || sp[i] == '\n') && i >= 0; i--) ;
		if (i < index) {
			index = i + 1;
			sp[index] = '\n';
		}
	}

	if (sp[index] == '\n') {
		if (CLAN_PROG_NUM != CHECK && CLAN_PROG_NUM != CHSTRING) {
			 do {
				tlineno++;
				sp[index] = (char)getc_cr(fpin, &att[index]);
			} while (sp[index] == '\n') ;
			if (isSpeaker(sp[index]) || uS.isUTF8(sp))
				getrestline = 0;
		}
	}
	if (feof(fpin))
		getrestline = 0;
	currentchar = sp[index];
	currentatt = att[index];
	sp[index] = EOS;
	return(TRUE);
}

/* getmaincode() gets all the tiers that should be clastered together and
   stores them into "->line" field. It discards the tiers that were not
   selected by the user, and then it filters the selected data. It returns 0
   if getspeaker returned 0, and 1 otherwise.
*/
int getmaincode() {
	int pos, i;
	char tc, *tl, RightTier, RightSpeaker, isTierSelected;

	if (lineno > -1L)
		lineno = lineno + tlineno;
	tlineno = 0L;
	if (!getspeaker(lutter->speaker, lutter->attSp, 0))
		return(FALSE);
	lutter->tlineno = lineno;
	lutter->uttLen = 0L;
	if (*lutter->speaker == '@')
		IsModUttDel = FALSE;
	else if (*lutter->speaker == '*')
		IsModUttDel = chatmode < 3;
	else {
		strcpy(templineC,lutter->speaker);
		uS.uppercasestr(templineC, &dFnt, MBF);
		if (uS.partcmp(templineC,"%PHO:",FALSE, TRUE) ||
			uS.partcmp(templineC,"%MOD:",FALSE, TRUE)) {
			IsModUttDel = 0;
			punctuation = PUNCT_PHO_MOD_SET;
		} else {
			IsModUttDel = (chatmode < 3);
			punctuation = GlobalPunctuation;
		}
	}
	cutt_getline(lutter->speaker, lutter->line, lutter->attLine, 0);
	strcpy(org_spTier, lutter->line);
	if (*lutter->speaker == '@' && currentchar == '%') {
		fprintf(stderr, "Header tier should not be followed by the Coder Dependent tier.\n");
		fprintf(stderr, "Error found in file \"%s\" on line %ld\n", oldfname, lineno);
		cutt_exit(0);
	}
	if (*lutter->speaker == '*' && nomain) {
		postcodeRes = isPostCodeFound(lutter);
		for (i=0; lutter->line[i] != EOS; i++) lutter->tuttline[i] = ' ';
		if (i > 0) lutter->tuttline[i-1] = '\n';
		lutter->tuttline[i] = EOS;
		pos = i;
	} else {
		pos = strlen(lutter->line);
		strcpy(lutter->tuttline,lutter->line);
		if (FilterTier > 0)
			filterData(lutter->speaker,lutter->tuttline);
		if (FilterTier > 1 && WordMode != 'e' && !CntWUT && !CntFUttLen)
			filterwords(lutter->speaker,lutter->tuttline,exclude);
	}
	RightSpeaker = (char)checktier(lutter->speaker);
	while (!isMainSpeaker(currentchar) && currentchar != '@' && !feof(fpin)) {
		if (!getspeaker(lutter->line, lutter->attLine, pos))
			return(FALSE);
		if (lineno > -1L)
			utterance->tlineno = lineno;
		utterance->uttLen = 0L;
		isTierSelected = (char)checktier(lutter->line+pos);
		RightTier = (RightSpeaker && isTierSelected);
		if (otcdt && !RightTier && wdptr == NULL && isMorSearchListGiven() == FALSE && tct == FALSE && tcs)
			RightTier = (!RightSpeaker && CheckOutTier(lutter->line+pos));
		else if (otcdt && RightTier && RightSpeaker && wdptr == NULL && isMorSearchListGiven() == FALSE && tct == FALSE)
			RightTier = CheckOutTier(lutter->line+pos);
		else
			RightTier = (RightTier || CheckOutTier(lutter->line+pos));
		if (CLAN_PROG_NUM == KWAL)
			isTierSelected = RightTier;
		if (RightTier) {
			if (pos > 0 && lutter->tuttline[pos-1] != '\n' && lutter->tuttline[pos-1] != EOS)
				lutter->tuttline[pos-1] = '\n';
			tl = lutter->line+pos;
			if (RightTier)
				strcat(lutter->tuttline+pos,lutter->line+pos);
			else {
				for (i=pos; lutter->line[i]; i++) {
					if (lutter->line[i] == '\n')
						lutter->tuttline[i] = '\n';
					else
						lutter->tuttline[i] = ' ';
				}
				lutter->tuttline[i] = EOS;
			}
			templineC[0] = EOS;
			if (lutter->line[pos] == '@') IsModUttDel = FALSE;
			else if (lutter->line[pos] == '*') IsModUttDel = chatmode < 3;
			else {
				strcpy(templineC,lutter->line+pos);
				uS.uppercasestr(templineC, &dFnt, MBF);
				if (uS.partcmp(templineC,"%PHO:",FALSE, TRUE) ||
					uS.partcmp(templineC,"%MOD:",FALSE, TRUE)) {
					IsModUttDel = 0;
					punctuation = PUNCT_PHO_MOD_SET;
				} else {
					IsModUttDel = (chatmode < 3);
					punctuation = GlobalPunctuation;
				}
			}
			pos += strlen(lutter->line+pos);
			cutt_getline("%", lutter->line, lutter->attLine, pos);
			if (isTierSelected)
				strcat(lutter->tuttline+pos,lutter->line+pos);
			else {
				for (i=pos; lutter->line[i]; i++) {
					if (lutter->line[i] == '\n')
						lutter->tuttline[i] = '\n';
					else
						lutter->tuttline[i] = ' ';
				}
				lutter->tuttline[i] = EOS;
			}
			tc = *utterance->speaker;
			*utterance->speaker = *tl;
			if (isTierSelected) {
				if (uS.partcmp(templineC,"%mor:",FALSE, FALSE)) {
					if (isMorSearchListGiven() || linkMain2Mor) {
						createMorUttline(lutter->line+pos, org_spTier, lutter->tuttline+pos, linkMain2Mor);
						strcpy(lutter->tuttline+pos, lutter->line+pos);
						filterMorTier(lutter->tuttline+pos, lutter->line+pos, 1);
					} else
						filterMorTier(lutter->tuttline+pos, NULL, 0);
				}
				if (FilterTier > 0)
					filterData(tl,lutter->tuttline+pos);
				if (FilterTier > 1 && WordMode != 'e' && !CntWUT && !CntFUttLen)
					filterwords(tl,lutter->tuttline+pos,exclude);
			}
			*utterance->speaker = tc;
			pos += strlen(lutter->line+pos);
		} else {
			lutter->line[pos] = EOS;
			killline(NULL, NULL);
		}
	}
	punctuation = GlobalPunctuation;
	return(TRUE);
}
void blankoSelectedSpeakers(char *line) {
	int pos;

	pos = 0;
	while (line[pos] != EOS) {
		if (line[pos] == '\n' && line[pos+1] == '%') {
			pos++;
			while (!checktier(line+pos)) {
				for (; line[pos] && line[pos] != ':'; pos++) ;
				if (line[pos] == ':') {
					pos++;
					if (line[pos] == '\t')
						pos++;
				}
				for (; line[pos] && (line[pos] != '\n' || line[pos+1] != '%'); pos++) {
					line[pos] = ' ';
				}
				if (line[pos] == '\n')
					pos++;
				if (line[pos] == EOS)
					break;
			}
			for (; line[pos] && line[pos] != ':'; pos++) ;
			if (line[pos] == ':') {
				pos++;
				if (line[pos] == '\t')
					pos++;
			}
			if (line[pos] == EOS)
				break;
		}
		pos++;
	}
}

int getwholeutter() {
	if (isWinMode) {
/* if isWinMode getwholeutter() call getmaincode() to get cluster tiers make sure that
   the tiers are the selected ones and that the data in the cluster tiers is
   in the right range. It returns 0 if getmaincode() or gettextspeaker
   returned 0, and 1 otherwise.
*/
		char *chrs;

		do {
			if (chatmode) {
				do {
					if (lutter != utterance) {
						utterance = utterance->nextutt;
						chrs = utterance->speaker;
					} else {
						utterance = utterance->nextutt;
						lutter = lutter->nextutt;
						postcodeRes = 0;
						if (!getmaincode())
							return(FALSE);
						chrs = lutter->speaker;
					}
					if (*utterance->speaker == '@') {
						if (IDField != NULL) {
							if (uS.partcmp(utterance->speaker,"@ID:",FALSE, FALSE)) {
								SetIDTier(utterance->line);
							}
						}
						if (SPRole != NULL) {
							if (uS.partcmp(utterance->speaker,"@ID:",FALSE, FALSE)) {
								SetSPRoleIDs(utterance->line);
							} else if (uS.partcmp(utterance->speaker,PARTICIPANTS,FALSE,FALSE)) {
								SetSPRoleParticipants(utterance->line);
							}
						}
						if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE) || uS.partcmp(utterance->speaker,"@New Language:",FALSE,FALSE)) {
							if (isLanguageExplicit)
								addToLanguagesTable(utterance->line, utterance->speaker);
						} else if (uS.partcmp(utterance->speaker,"@Options:",FALSE,FALSE)) {
							checkOptions(utterance->line);
						} else if (uS.partcmp(utterance->speaker,MEDIAHEADER,FALSE,FALSE)) {
							getMediaName(utterance->line, cMediaFileName, FILENAME_MAX);
						} else if (uS.partcmp(utterance->speaker,CKEYWORDHEADER,FALSE,FALSE)) {
							tlineno--;
						} else if (uS.isUTF8(utterance->speaker)) {
							dFnt.isUTF = 1;
							if (isOrgUnicodeFont(dFnt.orgFType)) {
								NewFontInfo finfo;
								SetDefaultUnicodeFinfo(&finfo);
								selectChoosenFont(&finfo, FALSE);
							}
							tlineno--;
						} else if (uS.partcmp(utterance->speaker,FONTHEADER,FALSE,FALSE)) {
							cutt_SetNewFont(utterance->line, EOS);
							tlineno--;
						} else {
							if (uS.partwcmp(utterance->line, FONTMARKER))
								cutt_SetNewFont(utterance->line,']');
						}
						if (CntWUT || CntFUttLen) {
							strcpy(templineC,utterance->speaker);
							uS.uppercasestr(templineC, &dFnt, MBF);
							if (uS.partcmp(templineC, "@ENDTURN", FALSE, FALSE))
								Tspchanged = TRUE;
						}
						if (checktier(chrs) || CheckOutTier(chrs)) break;
					} else if (rightspeaker || *chrs == '*') {
						if (checktier(chrs)) {
							rightspeaker = TRUE;
							if (uS.partwcmp(utterance->line, FONTMARKER))
								cutt_SetNewFont(utterance->line,']');
							if (hasSpeakerChanged(utterance->speaker, Toldsp, utterance->tuttline, TRUE)) {
								Tspchanged = TRUE;
							}
							break;
						} else if (CheckOutTier(chrs)) {
							if (*utterance->speaker == '*')
								rightspeaker = FALSE;
							if (uS.partwcmp(utterance->line, FONTMARKER))
								cutt_SetNewFont(utterance->line,']');
							if (hasSpeakerChanged(utterance->speaker, Toldsp, utterance->tuttline, FALSE)) {
								Tspchanged = TRUE;
							}
							break;
						} else if (*utterance->speaker == '*') {
							rightspeaker = FALSE;
							if (uS.partwcmp(utterance->line, FONTMARKER))
								cutt_SetNewFont(utterance->line,']');
							if (hasSpeakerChanged(utterance->speaker, Toldsp, utterance->tuttline, FALSE)) {
								Tspchanged = TRUE;
							}
						} else {
							if (uS.partwcmp(utterance->line, FONTMARKER))
								cutt_SetNewFont(utterance->line,']');
						}
					} else {
						if (uS.partwcmp(utterance->line, FONTMARKER))
							cutt_SetNewFont(utterance->line,']');
					}
				} while (1) ;
				uttline = utterance->tuttline;
				if (CntWUT || CntFUttLen) {
					if (CntWUT && ToWU && WUCounter > ToWU)
						return(FALSE);
					if (checktier(utterance->speaker)) {
						if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
							break;
						}
						strcpy(templineC, uttline);
//						if (FilterTier == 1)
//							filterwords(utterance->speaker,templineC,exclude);
						if (FilterTier > 1 && WordMode != 'e')
							filterwords(utterance->speaker,uttline,exclude);
						if (!CntFUttLen) {
							if (rightrange(*utterance->speaker,templineC,uttline))
								break;
						} else if (rightUttLen(utterance->speaker,utterance->line,templineC,&utterance->uttLen)) {
							if (!CntWUT)
								break;
							else if (rightrange(*utterance->speaker,templineC,uttline))
								break;
						}
					} else if (CntWUT && WUCounter >= FromWU && CheckOutTier(utterance->speaker)) {
						int pos;
						for (pos=0; uttline[pos]; pos++)
							uttline[pos] = ' ';
						break;
					} else if (CntFUttLen && WUCounter >= FromWU && CheckOutTier(utterance->speaker)) {
						break;
					}
				} else
					break;
			} else {
				if (lutter != utterance) {
					utterance = utterance->nextutt;
				} else {
					utterance = utterance->nextutt;
					lutter = lutter->nextutt;
					if (!gettextspeaker())
						return(FALSE);
					postcodeRes = 0;
					IsModUttDel = chatmode < 3;
					cutt_getline("*", lutter->line, lutter->attLine, 0);
					lutter->tlineno = lineno;
					lutter->uttLen = 0L;
					strcpy(lutter->tuttline,lutter->line);
					if (uS.isskip("[", 0, &dFnt, FALSE) || uS.isskip("]", 0, &dFnt, FALSE))
						filterData("*",lutter->tuttline);
					else
						filterwords("*",lutter->tuttline,excludedef);
					if (WordMode != 'e')
						filterwords("*",lutter->tuttline,exclude);
				}
				uttline = utterance->tuttline;
				if (uS.partcmp(utterance->line,CKEYWORDHEADER,FALSE,FALSE)) {
					tlineno--;
				} else if (uS.isUTF8(utterance->line)) {
					tlineno--;
				} else if (uS.partcmp(utterance->line,FONTHEADER,FALSE,FALSE)) {
					int i = strlen(FONTHEADER);
					while (isSpace(utterance->line[i]))
						i++;
					cutt_SetNewFont(utterance->line+i, EOS);
					tlineno--;
				}
				if (CntWUT || CntFUttLen) {
					if (CntWUT && ToWU && WUCounter > ToWU)
						return(FALSE);
					if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
						break;
					}
					strcpy(templineC, uttline);
//					if (FilterTier == 1)
//						filterwords("*",templineC,exclude);
					if (FilterTier > 1 && WordMode != 'e')
						filterwords("*",uttline,exclude);
					if (!CntFUttLen) {
						if (rightrange('*',templineC,uttline))
							break;
					} else if (rightUttLen("*",utterance->line,templineC,&utterance->uttLen)) {
						if (!CntWUT)
							break;
						else if (rightrange('*',templineC,uttline))
							break;
					}
				} else
					break;
			}
		} while (1) ;
		return(TRUE);
	} else {
/* if !isWinMode getwholeutter()  gets next tier into utterance and determines if the tier
   has been selected by the user and if it is in the right range, if yes
   then it is filtered, other wise it gets the next tier from the input
   stream. It returns 0 if the getspeaker() or gettextspeaker() returns 0,
   and 1 otherwise.
*/
		if (ByTurn >= 2) {
			ByTurn = 1;
			return(TRUE);
		}
		do {
			if (chatmode) {
				if (LocalTierSelect) {
					do {
						if (lineno > -1L)
							lineno = lineno + tlineno;
						tlineno = 0L;
						if (!getspeaker(utterance->speaker, utterance->attSp, 0))
							return(FALSE);
						if (IDField != NULL) {
							if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
								IsModUttDel = FALSE;
								cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
								skipgetline = TRUE;
								getrestline = 0;
								SetIDTier(utterance->line);
							}
						}
						if (SPRole != NULL) {
							if (uS.partcmp(utterance->speaker,"@ID:",FALSE, FALSE)) {
								IsModUttDel = FALSE;
								cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
								skipgetline = TRUE;
								getrestline = 0;
								SetSPRoleIDs(utterance->line);
							} else if (uS.partcmp(utterance->speaker,PARTICIPANTS,FALSE,FALSE)) {
								IsModUttDel = FALSE;
								cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
								skipgetline = TRUE;
								getrestline = 0;
								SetSPRoleParticipants(utterance->line);
							}
						}
						if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE) || uS.partcmp(utterance->speaker,"@New Language:",FALSE,FALSE)) {
							if (isLanguageExplicit) {
								IsModUttDel = FALSE;
								cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
								skipgetline = TRUE;
								getrestline = 0;
								addToLanguagesTable(utterance->line, utterance->speaker);
							}
						} else if (uS.partcmp(utterance->speaker,"@Options:",FALSE,FALSE)) {
							IsModUttDel = FALSE;
							cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
							skipgetline = TRUE;
							getrestline = 0;
							checkOptions(utterance->line);
						} else if (uS.partcmp(utterance->speaker,MEDIAHEADER,FALSE,FALSE)) {
							IsModUttDel = FALSE;
							cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
							skipgetline = TRUE;
							getrestline = 0;
							getMediaName(utterance->line, cMediaFileName, FILENAME_MAX);
						} else if (uS.partcmp(utterance->speaker,CKEYWORDHEADER,FALSE,FALSE)) {
							tlineno--;
						} else if (uS.isUTF8(utterance->speaker)) {
							dFnt.isUTF = 1;
							if (isOrgUnicodeFont(dFnt.orgFType)) {
								NewFontInfo finfo;
								SetDefaultUnicodeFinfo(&finfo);
								selectChoosenFont(&finfo, FALSE);
							}
							tlineno--;
						} else if (uS.partcmp(utterance->speaker,FONTHEADER,FALSE,FALSE)) {
							IsModUttDel = FALSE;
							cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
							skipgetline = TRUE;
							getrestline = 0;
							cutt_SetNewFont(utterance->line, EOS);
							tlineno--;
						}
						if (CntWUT || CntFUttLen) {
							int i;

							i = strlen(utterance->speaker) - 1;
							for (; i >= 0 && (utterance->speaker[i] == ' ' ||
											  utterance->speaker[i] == '\t'); i--) ;
							utterance->speaker[i+1] = EOS;
							strcpy(templineC,utterance->speaker);
							uS.uppercasestr(templineC, &dFnt, MBF);
							if (*utterance->speaker == '@') {
								if (uS.partcmp(templineC, "@ENDTURN", FALSE, FALSE))
									Tspchanged = TRUE;
								break;
							} else if (rightspeaker || *utterance->speaker == '*') {
								if (checktier(utterance->speaker)) {
									rightspeaker = TRUE;
									if (*utterance->speaker != '*')
										IsModUttDel = (chatmode < 3);
									break;
								} else if (*utterance->speaker == '*') {
									rightspeaker = FALSE;
									IsModUttDel = (chatmode < 3);
									cutt_getline(utterance->speaker, uttline, utterance->attLine, 0);
									if (uS.partwcmp(uttline, FONTMARKER))
										cutt_SetNewFont(uttline,']');
									if (FilterTier > 0)
										filterData(utterance->speaker,uttline);
									if (hasSpeakerChanged(utterance->speaker, Toldsp, uttline, FALSE)) {
										Tspchanged = TRUE;
									}
								} else
									killline(utterance->line, utterance->attLine);
							} else
								killline(utterance->line, utterance->attLine);
						} else
							break;
					} while (1) ;
					if (*utterance->speaker == '*')
						postcodeRes = 0;
				} else {
					do {
						if (lineno > -1L)
							lineno = lineno + tlineno;
						tlineno = 0L;
						if (!getspeaker(utterance->speaker, utterance->attSp, 0))
							return(FALSE);
						if (*utterance->speaker == '*')
							postcodeRes = 0;
						if (*utterance->speaker == '@') {
							if (IDField != NULL) {
								if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
									IsModUttDel = FALSE;
									cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
									skipgetline = TRUE;
									getrestline = 0;
									SetIDTier(utterance->line);
								}
							}
							if (SPRole != NULL) {
								if (uS.partcmp(utterance->speaker,"@ID:",FALSE, FALSE)) {
									IsModUttDel = FALSE;
									cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
									skipgetline = TRUE;
									getrestline = 0;
									SetSPRoleIDs(utterance->line);
								} else if (uS.partcmp(utterance->speaker,PARTICIPANTS,FALSE,FALSE)) {
									IsModUttDel = FALSE;
									cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
									skipgetline = TRUE;
									getrestline = 0;
									SetSPRoleParticipants(utterance->line);
								}
							}
							if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE) || uS.partcmp(utterance->speaker,"@New Language:",FALSE,FALSE)) {
								if (isLanguageExplicit) {
									IsModUttDel = FALSE;
									cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
									skipgetline = TRUE;
									getrestline = 0;
									addToLanguagesTable(utterance->line, utterance->speaker);
								}
							} else if (uS.partcmp(utterance->speaker,"@Options:",FALSE,FALSE)) {
								IsModUttDel = FALSE;
								cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
								skipgetline = TRUE;
								getrestline = 0;
								checkOptions(utterance->line);
							} else if (uS.partcmp(utterance->speaker,MEDIAHEADER,FALSE,FALSE)) {
								IsModUttDel = FALSE;
								cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
								skipgetline = TRUE;
								getrestline = 0;
								getMediaName(utterance->line, cMediaFileName, FILENAME_MAX);
							} else if (uS.partcmp(utterance->speaker,CKEYWORDHEADER,FALSE,FALSE)) {
								tlineno--;
							} else if (uS.isUTF8(utterance->speaker)) {
								dFnt.isUTF = 1;
								if (isOrgUnicodeFont(dFnt.orgFType)) {
									NewFontInfo finfo;
									SetDefaultUnicodeFinfo(&finfo);
									selectChoosenFont(&finfo, FALSE);
								}
								tlineno--;
							} else if (uS.partcmp(utterance->speaker,FONTHEADER,FALSE,FALSE)) {
								IsModUttDel = FALSE;
								cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
								skipgetline = TRUE;
								getrestline = 0;
								cutt_SetNewFont(utterance->line, EOS);
								tlineno--;
							}
							if (CntWUT || CntFUttLen) {
								strcpy(templineC,utterance->speaker);
								uS.uppercasestr(templineC, &dFnt, MBF);
								if (uS.partcmp(templineC, "@ENDTURN", FALSE, FALSE))
									Tspchanged = TRUE;
							}
							if (checktier(utterance->speaker) || CheckOutTier(utterance->speaker))
								break;
							killline(utterance->line, utterance->attLine);
						} else if (rightspeaker || *utterance->speaker == '*') {
							if (checktier(utterance->speaker)) {
								rightspeaker = TRUE;
								if (*utterance->speaker != '*' && (CntWUT || CntFUttLen)) {
									IsModUttDel = (chatmode < 3);
									cutt_getline(utterance->speaker, uttline, utterance->attLine, 0);
									getrestline = FALSE;
									if (uS.partwcmp(uttline, FONTMARKER))
										cutt_SetNewFont(uttline,']');
									if (FilterTier > 0)
										filterData(utterance->speaker,uttline);
									if (hasSpeakerChanged(utterance->speaker, Toldsp, uttline, TRUE)) {
										Tspchanged = TRUE;
									}
								}
								if (*utterance->speaker != '*' || !nomain)
									break;
								killline(utterance->line, utterance->attLine);
								if (*utterance->speaker == '*')
									postcodeRes = isPostCodeFound(utterance);
							} else if (CheckOutTier(utterance->speaker)) {
								rightspeaker = TRUE;
								if (*utterance->speaker != '*' && (CntWUT || CntFUttLen)) {
									IsModUttDel = (chatmode < 3);
									cutt_getline(utterance->speaker, uttline, utterance->attLine, 0);
									strcpy(utterance->line, uttline);
									skipgetline = TRUE;
									getrestline = FALSE;
									if (uS.partwcmp(uttline, FONTMARKER))
										cutt_SetNewFont(uttline,']');
									if (FilterTier > 0)
										filterData(utterance->speaker,uttline);
									if (hasSpeakerChanged(utterance->speaker, Toldsp, uttline, FALSE)) {
										Tspchanged = TRUE;
									}
								}
								if (*utterance->speaker != '*' || !nomain)
									break;
								killline(utterance->line, utterance->attLine);
								if (*utterance->speaker == '*')
									postcodeRes = isPostCodeFound(utterance);
							} else if (*utterance->speaker == '*') {
								rightspeaker = FALSE;
								if (CntWUT || CntFUttLen) {
									IsModUttDel = (chatmode < 3);
									cutt_getline(utterance->speaker, uttline, utterance->attLine, 0);
									if (uS.partwcmp(uttline, FONTMARKER))
										cutt_SetNewFont(uttline,']');
									if (FilterTier > 0)
										filterData(utterance->speaker,uttline);
									if (hasSpeakerChanged(utterance->speaker, Toldsp, uttline, FALSE)) {
										Tspchanged = TRUE;
									}
								} else
									killline(utterance->line, utterance->attLine);
							} else
								killline(utterance->line, utterance->attLine);
							if (*utterance->speaker == '*') {
								if (linkMain2Mor)
									cutt_cleanUpLine(utterance->speaker, utterance->line, utterance->attLine, 0);
								strcpy(org_spTier, utterance->line);
							}
						} else
							killline(utterance->line, utterance->attLine);
					} while (1) ;
				}

				strcpy(templineC,utterance->speaker);
				uS.uppercasestr(templineC, &dFnt, MBF);
				if (uS.partcmp(templineC,"%PHO:",FALSE,FALSE) ||
					uS.partcmp(templineC,"%MOD:",FALSE,FALSE)) {
					IsModUttDel = 0;
					punctuation = PUNCT_PHO_MOD_SET;
					cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
					if (uS.partwcmp(utterance->line, FONTMARKER))
						cutt_SetNewFont(utterance->line,']');
					if (uttline != utterance->line)
						strcpy(uttline,utterance->line);
					if (FilterTier > 0) {
						filterscop("%", uttline);
					}
				} else {
					IsModUttDel = ((*templineC != '@') && (chatmode < 3));
					punctuation = GlobalPunctuation;
					cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
					if (*utterance->speaker == '*')
						strcpy(org_spTier, utterance->line);
					if (uS.partwcmp(utterance->line, FONTMARKER))
						cutt_SetNewFont(utterance->line,']');
					if (uttline != utterance->line)
						strcpy(uttline,utterance->line);
					if (uS.partcmp(templineC,"%mor:",FALSE, FALSE)) {
						if (isMorSearchListGiven() || linkMain2Mor) {
							createMorUttline(utterance->line, org_spTier, utterance->tuttline, linkMain2Mor);
							strcpy(utterance->tuttline, utterance->line);
							filterMorTier(uttline, utterance->line, 2);
						} else
							filterMorTier(uttline, NULL, 0);
					}
					if (FilterTier > 0)
						filterData(utterance->speaker,uttline);
				}
				if (CntWUT || CntFUttLen) {
/*
					if (LocalTierSelect && *utterance->speaker == '*' && nomain) {
						if (FilterTier > 1)
							filterwords(utterance->speaker,uttline,exclude);
						break;
					}
*/
					if (CntWUT && ToWU && WUCounter > ToWU)
						return(FALSE);
					if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
						break;
					}
					strcpy(templineC, uttline);
//					if (FilterTier == 1)
//						filterwords(utterance->speaker,templineC,exclude);
					if (FilterTier > 1)
						filterwords(utterance->speaker,uttline,exclude);
					if (!CntFUttLen) {
						if (rightrange(*utterance->speaker,templineC,uttline))
							break;
						else
							rightspeaker = FALSE;
					} else if (rightUttLen(utterance->speaker,utterance->line,templineC,&utterance->uttLen)) {
						if (!CntWUT)
							break;
						else if (rightrange(*utterance->speaker,templineC,uttline))
							break;
						else
							rightspeaker = FALSE;
					} else
						rightspeaker = FALSE;
				} else {
					if (FilterTier > 1)
						filterwords(utterance->speaker,uttline,exclude);
					break;
				}
			} else { /* if (chatmode) */
				postcodeRes = 0;
				if (!gettextspeaker()) return(FALSE);
				IsModUttDel = chatmode < 3;
				cutt_getline("*", utterance->line, utterance->attLine, 0);
				if (uS.partcmp(utterance->line,CKEYWORDHEADER,FALSE,FALSE)) {
					tlineno--;
				} else if (uS.isUTF8(utterance->line)) {
					tlineno--;
				} else if (uS.partcmp(utterance->line,FONTHEADER,FALSE,FALSE)) {
					int i = strlen(FONTHEADER);
					while (isSpace(utterance->line[i]))
						i++;
					cutt_SetNewFont(utterance->line+i, EOS);
					tlineno--;
				}
				if (uttline != utterance->line)
					strcpy(uttline,utterance->line);
				if (FilterTier > 0)
					filterData("*",uttline);
				if (CntWUT || CntFUttLen) {
					if (CntWUT && ToWU && WUCounter > ToWU)
						return(FALSE);
					if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
						break;
					}
					strcpy(templineC, uttline);
//					if (FilterTier == 1)
//						filterwords("*",templineC,exclude);
					if (FilterTier > 1)
						filterwords("*",uttline,exclude);
					if (!CntFUttLen) {
						if (rightrange('*',templineC,uttline))
							break;
					} else if (rightUttLen("*",utterance->line,templineC,&utterance->uttLen)) {
						if (!CntWUT)
							break;
						else if (rightrange('*',templineC,uttline))
							break;
					}
				} else {
					if (FilterTier > 1)
						filterwords("*",uttline,exclude);
					break;
				}
			}
		} while (1) ;
		if (ByTurn == 2)
			return(FALSE);
		else
			return(TRUE);
	}
}
