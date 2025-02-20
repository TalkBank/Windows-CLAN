/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// 2011-04-07

/*
#define DEBUG_CLAN
*/

#define CHAT_MODE 4

#include "cu.h"
#include "check.h"
#ifndef UNX
	#include "ced.h"
#else
	#define RGBColor int
	#include "c_curses.h"
#endif

#ifdef _MAC_CODE
#undef isatty
#include <unistd.h>
#include <pwd.h>
#endif

#if !defined(UNX)
#define _main check_main
#define call check_call
#define getflag check_getflag
#define init check_init
#define usage check_usage
#else
#undef is_unCH_digit
#define is_unCH_digit isdigit
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

#define LAST_ERR_NUM 161

#define CHECK_CODES struct templates

#define MINFILESNUM 30 // 2019-04-18 TotalNumFiles
#define NUMLANGUAGESINTABLE 10

#define NUM_GEN_OPT 5   /* number of generic options */
#define D_D_SET_1(x)		(AttTYPE)(x | 1)
#define D_D_COUNT(x)		(AttTYPE)(x & 1)
#define D_D_SET_0(x)		(AttTYPE)(x & 0xfffe)

#define NAME_ROLE_SET_1(x)	(AttTYPE)(x | 2)
#define NAME_ROLE_COUNT(x)	(AttTYPE)(x & 2)
#define NAME_ROLE_SET_0(x)	(AttTYPE)(x & 0xfffd)

#define W_CHECK_SET_1(x)	(AttTYPE)(x | 4)
#define W_CHECK_COUNT(x)	(AttTYPE)(x & 4)
#define W_CHECK_SET_0(x)	(AttTYPE)(x & 0xfffb)

#define SET_CHECK_ID_1(x)	(AttTYPE)(x | 8)
#define IS_CHECK_ID(x)		(AttTYPE)(x & 8)
#define SET_CHECK_ID_0(x)	(AttTYPE)(x & 0xfff7)

#define SET_SPK_USED_1(x)	(AttTYPE)(x | 16)
#define IS_SPK_USED(x)		(AttTYPE)(x & 16)
#define SET_SPK_USED_0(x)	(AttTYPE)(x & 0xffef)

//#define SET_LANG_CHECK_1(x)	(AttTYPE)(x | 32)
//#define IS_LANG_CHECK(x)	(AttTYPE)(x & 32)
//#define SET_LANG_CHECK_0(x)	(AttTYPE)(x & 0xffdf)

#define check_trans_err(wh,sp,ep,ln)						 \
		check_err(wh,							 \
			 (int)((unsigned long)(sp)-(unsigned long)(uttline)),\
			 (int)((unsigned long)(ep)-(unsigned long)(uttline)),\
			 ln+lineno)

#if defined(UNX)
#define TPLATES struct templates
#define ERRLIST struct errlst
#define BE_TIERS struct templates
#define SPLIST struct speakers
#endif

TPLATES {
	char *pattern;
	TPLATES *nextone;
} ;

ERRLIST {
    char *st;
    short errnum;
    ERRLIST *nexterr;
} ;


struct UDErr {
	char *code;
	struct UDErr *nextUDerr;
} ;

struct IDsp {
	char *sp;
	struct IDsp *nextsp;
} ;

SPLIST {
	char *sp;
	char *role;
	char isUsed;
	long endTime;
	char hasID;
	SPLIST *nextsp;
} ;

#define CHECK_TIERS struct check_tiers
CHECK_TIERS {
    char *code;
    char col, 
    IGN, 					/* if TRUE, do not check words and such on that tier */
    UTD, 					/* if TRUE, check tiers for utterance delimiters */
    CSUFF,					/* if TRUE, check tiers' words' suffixes */
    WORDCHECK;				/* if TRUE, check every words on tier for bad characters */
    TPLATES *prefs;	/* items in place of '*' [UPREFS Mac] all pref Upper chars: MacDonald*/
    TPLATES *tplate;
    CHECK_TIERS *nexttier;
} ;

#define NUMUSEDTIERSONCE 50
#define USEDTIERSONCELEN 10
struct TiersOnceList {
	char spCode[USEDTIERSONCELEN+1];
	char fileNum;
	char freq;
} ;
const char *lHeaders[] = {"@Begin", "@Languages:", "@Participants:", ""};

/* *********************************************************** */

extern int  totalFilesNum; // 2019-04-18 TotalNumFiles
extern char tcs;			/* 1- if all non-specified speaker tier should be selected*/
extern char tch;
extern char tct;
extern char isRecursive;
extern char Parans;
extern char OverWriteFile;
extern struct tier *defheadtier;
extern struct tier *headtier;

static AttTYPE check_GenOpt;
static int    FileCount;
static int    check_curLanguage;
static double oPrc;
static char isCheckUDFeats; // 2024-02-16
static char isBulletsFound;
static char isMediaHeaderFound;
static char checkBullets;
static char check_err_found;
static char FTime;
static char isItemPrefix[256];
static char ErrorReport[LAST_ERR_NUM];
static char ErrorRepFlag;
static char check_all_err_found;
static char err_itm[512];
static char check_lastSpDelim[25];
static char check_isNumberShouldBeHere;
static char check_isCAFound;
static char check_isArabic;
static char check_isHebrew;
static char check_isGerman;
static char check_isMultiBullets;
static char check_isCheckBullets;
static char check_isUnlinkedFound;
static char check_applyG2Option;
static char check_check_mismatch;
static char check_SNDFname[FILENAME_MAX];
static char check_fav_lang[20];
static char check_LanguagesTable[NUMLANGUAGESINTABLE][9];
static long tDiff;
static long check_SNDBeg = 0L;
static long check_SNDEnd = 0L;
static long check_tierBegTime = 0L;
static TPLATES *check_WordExceptions;
static FNType check_localDir[FNSize];
static FNType oldDepfile[FNSize];
//2011-04-07 static struct TiersOnceList tiersUsedOnce[NUMUSEDTIERSONCE];

static ERRLIST *headerr;
static struct UDErr *check_UDerr;
static struct IDsp *check_idsp;
static SPLIST *headsp;
static CHECK_TIERS *headt, *maint, *codet;

void usage() {
	puts("CHECK.");
	printf("Usage: check [cN e gN %s] filename(s)\n",mainflgs());
	puts("+cN: check bullets consistency (1- check for missing bullets only)");
	puts("+eN: report only error(s) number N");
	puts("+e : list numbers for every error");
	puts("-eN: report all error(s) except number N");
	puts("+g1: recognize prosodic delimiter");
	puts("-g1: ignore prosodic delimiter (default)");
	puts("+g2: check for \"CHI Target_Child\"");
	puts("-g2: do not check for \"CHI Target_Child\" (default)");
	puts("+g3: check words for upper case letters, numbers and illegal apostrophes");
	puts("-g3: do not check words in more details (default)");
	puts("+g4: check file(s) for missing @ID tiers");
	puts("-g4: do not check file(s) for missing @ID tiers (default)");
	puts("+g5: check for Speakers specified on @Participants tier, but not used in the file");
	puts("-g5: do not check for Speakers not used in the file (default)");
//	puts("+g6: validate language codes against file ISO-639.cut");
//	puts("-g6: do not validate language codes against file ISO-639.cut (default)");
	puts("+u : validate UD features on %mor tier");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(TRUE);
}

static void check_overflow() {
	fprintf(stderr,"ERROR: no more core memory available.\n");
	cleanupLanguages();
	cutt_exit(1);
}

static void check_clean_headerr() {
	ERRLIST *t;

	while (headerr != NULL) {
		t = headerr;
		headerr = headerr->nexterr;
		free(t->st);
		free(t);
	}
}

static void check_clean_template(TPLATES *p) {
	struct templates *t;

	while (p != NULL) {
		t = p;
		p = p->nextone;
		if (t->pattern)
			free(t->pattern);
		free(t);
	}
}

static struct UDErr *check_clean_UDerr(struct UDErr *p) {
	struct UDErr *t;

	while (p != NULL) {
		t = p;
		p = p->nextUDerr;
		if (t->code)
			free(t->code);
		free(t);
	}
	return(NULL);
}

static void check_clean_IDsp(struct IDsp *p) {
	struct IDsp *t;

	while (p != NULL) {
		t = p;
		p = p->nextsp;
		if (t->sp)
			free(t->sp);
		free(t);
	}
}

static void check_clean_speaker(SPLIST *p) {
	SPLIST *t;

	while (p != NULL) {
		t = p;
		p = p->nextsp;
		if (t->sp)
			free(t->sp);
		if (t->role)
			free(t->role);
		free(t);
	}
}

static void check_clean__t(CHECK_TIERS *p) {
	CHECK_TIERS *t;

	while (p != NULL) {
		t = p;
		p = p->nexttier;
		free(t->code);
		check_clean_template(t->prefs);
		check_clean_template(t->tplate);
		free(t);
	}
}

static char *check_mkstring(char *s) {  /* allocate space for string of text */
	char *p;

	if ((p=(char *)malloc(strlen(s)+1)) == NULL)
		check_overflow();
	strcpy(p, s);
	return(p);
}

static void check_mktplate(CHECK_TIERS *ts, char *s, char AtTheEnd) {
	TPLATES *tp;

	if (uS.isSqCodes(s, templineC3, &dFnt, FALSE)) {
		if (strcmp(s, templineC3)) {
			free(s);
			s = check_mkstring(templineC3);
		}
	}
	if (AtTheEnd) {
		if (ts->tplate == NULL) {
			ts->tplate = NEW(TPLATES);
			tp = ts->tplate;
		} else {
			for (tp=ts->tplate; tp->nextone != NULL; tp = tp->nextone) ;
			tp->nextone = NEW(TPLATES);
			tp = tp->nextone;
		}
		tp->nextone = NULL;
	} else {
		tp = NEW(TPLATES);
		tp->nextone = ts->tplate;
		ts->tplate = tp;
	}
	tp->pattern = s;
}

static CHECK_TIERS *SameNameTier(const char *s, char col, CHECK_TIERS *head) {
	while (head != NULL) {
		if (col == head->col && !strcmp(s, head->code))
			return(head);
		head = head->nexttier;
	}
	return(NULL);
}

static CHECK_TIERS *mktier(const char *s, char col, char CSUFF, char UTD) {
	CHECK_TIERS *ts;

	if ((ts=NEW(CHECK_TIERS)) == NULL) check_overflow();
	if ((ts->code=(char *)malloc(strlen(s)+1)) == NULL) check_overflow();
	strcpy(ts->code, s);
	ts->tplate = NULL;
	ts->col = col;
	ts->CSUFF = CSUFF;
	ts->prefs = NULL;
	ts->WORDCHECK = TRUE;
	ts->UTD = UTD;
	ts->IGN = FALSE;
	return(ts);
}

static TPLATES *check_getPrefChars(TPLATES *prefs, char *st) {
	TPLATES *tp;

	uS.extractString(templineC3, st, "[UPREFS ", ']');
	st = check_mkstring(templineC3);
	if (st == NULL)
		return(prefs);
	if (prefs == NULL) {
		prefs = NEW(TPLATES);
		tp = prefs;
	} else {
		for (tp=prefs; tp->nextone != NULL; tp = tp->nextone) ;
		tp->nextone = NEW(TPLATES);
		tp = tp->nextone;
	}
	if (tp != NULL) {
		tp->nextone = NULL;
		tp->pattern = st;
	} else
		free(st);
	return(prefs);
}

/* 2019-04-23 do not check for upper case letters
static char check_isRightPref(TPLATES *prefs, char *w, int s) {
	int len;

	while (prefs != NULL) {
		if (prefs->pattern) {
			len = strlen(prefs->pattern);
			if (s >= len) {
				strncpy(templineC3, w+s-len, len);
				templineC3[len] = EOS;
//				uS.uppercasestr(templineC3, &dFnt, MBF);
				if (!strcmp(prefs->pattern, templineC3))
					return(TRUE);
			}
		}
		prefs = prefs->nextone;
	}
	return(FALSE);
}

static char check_isException(char *w, TPLATES *prefs) {
	char tParans;

	strcpy(templineC4, w);
	tParans = Parans;
	Parans = 1;
	HandleParans(templineC4, 0, 0);
	Parans = tParans;
	while (prefs != NULL) {
		if (prefs->pattern) {
			if (uS.patmat(templineC4, prefs->pattern))
				return(TRUE);
		}
		prefs = prefs->nextone;
	}
	return(FALSE);
}
*/

static char *check_convertRegExp(char *s) {
	char lc, sb;
	int i, j;

	j = 0;
	lc = 0;
	if (s[0] == '[')
		sb = TRUE;
	else
		sb = FALSE;
	for (i=0; s[i] != EOS; i++) {
		if (uS.mStrnicmp(s+i, "{ALNUM}", 7) == 0) {
			templineC[j++] = ALNUM;
			if (sb)
				templineC[j++] = OR_CH;
			i += 6;
		} else if (uS.mStrnicmp(s+i, "{ALPHA}", 7) == 0) {
			templineC[j++] = ALPHA;
			if (sb)
				templineC[j++] = OR_CH;
			i += 6;
		} else if (uS.mStrnicmp(s+i, "{DIGIT}", 7) == 0) {
			templineC[j++] = DIGIT;
			if (sb)
				templineC[j++] = OR_CH;
			i += 6;
		} else if (s[i] == '|') {
			templineC[j++] = OR_CH;
		} else if (s[i] == '(') {
			templineC[j++] = PARAO;
		} else if (s[i] == ')') {
			templineC[j++] = PARAC;
		} else if (s[i] == '+' && lc == ')') {
			templineC[j++] = ONE_T;
		} else if (s[i] == '*' && lc == ')') {
			templineC[j++] = ZEROT;
		} else if (s[i] == '[') {
			sb = TRUE;
		} else if (s[i] == ']') {
			if (j > 0 && templineC[j-1] == OR_CH)
				j--;
			sb = FALSE;
		} else if (s[i] == '\\' && s[i+1] != EOS) {
			i++;
			templineC[j++] = s[i];
			if (sb)
				templineC[j++] = OR_CH;
		} else if (!isSpace(s[i]) && s[i] != '\n') {
			templineC[j++] = s[i];
			if (sb)
				templineC[j++] = OR_CH;
		}

		if (!isSpace(s[i]) && s[i] != '\n')
			lc = s[i];
	}
	templineC[j] = EOS;
	return(templineC);
}

static void check_ParseTier(char *temp) {
	char *s, sb, regExp;
	CHECK_TIERS *ts, *tsWor;

	for (s=temp; *temp != ':' && *temp != '\n'; temp++) {
		if (*temp == '#') *temp = '*';
	}
	if (*temp == ':')
		sb = TRUE;
	else
		sb = FALSE;
	*temp = EOS;
//	uS.uppercasestr(s, &dFnt, C_MBF);
	for (temp++; *temp && (*temp == ' ' || *temp == '\t'); temp++) ;
	if (*s == '*') {
		if ((ts=SameNameTier(s, sb, maint)) == NULL) {
			ts = mktier(s, sb, TRUE, TRUE);
			ts->nexttier = maint;
			maint = ts;
		}
		if ((tsWor=SameNameTier("%wor", sb, maint)) == NULL) {
			tsWor = mktier("%wor", sb, TRUE, TRUE);
			tsWor->nexttier = codet;
			codet = tsWor;
		}		
	} else if (*s == '%') {
		if ((ts=SameNameTier(s, sb, codet)) == NULL) {
			ts = mktier(s, sb, FALSE, 0);
			ts->nexttier = codet;
			codet = ts;
		}
	} else {
/*
		if (nomap)
			uS.uppercasestr(temp, &dFnt, C_MBF);
*/
		if ((ts=SameNameTier(s, sb, headt)) == NULL) {
			ts = mktier(s, sb, FALSE, 0);
			ts->nexttier = headt;
			headt = ts;
		}
	}
	s = temp;
	if (*temp == '[')
		sb = TRUE;
	else
		sb = FALSE;
	if (temp[0] == '@' && temp[1] == 'r' && temp[2] == '<')
		regExp = TRUE;
	else
		regExp = FALSE;
	while (*s) {
		if (((*temp== ' ' || *temp== '\t' || *temp== '\n') && !sb && !regExp) || !*temp) {
			if (*temp) {
				*temp = EOS;
				for (temp++; *temp && (*temp== ' ' || *temp== '\t' || *temp== '\n'); temp++) ;
			}
			if (s[0] == '@' && s[1] == 'r' && s[2] == '<') {
				s = check_convertRegExp(s);
			}
			if (!strcmp(s,"*")) {
				check_mktplate(ts,check_mkstring(s),1);
				if (strcmp(ts->code, "*") == 0)
					check_mktplate(tsWor,check_mkstring(s),1);
			} else if (!strcmp(s,UTTDELSYM)) {
				ts->UTD   = 1;
				if (strcmp(ts->code, "*") == 0)
					tsWor->UTD   = 1;
			} else if (!strcmp(s,NUTTDELSYM)) {
				ts->UTD   = -1;
				if (strcmp(ts->code, "*") == 0)
					tsWor->UTD   = -1;
			} else if (!strcmp(s,UTTIGNORE)) {
				ts->IGN   = TRUE;
				if (strcmp(ts->code, "*") == 0)
					tsWor->IGN   = TRUE;
			} else if (!strcmp(s,SUFFIXSYM)) {
				ts->CSUFF = '\002';
				if (strcmp(ts->code, "*") == 0)
					tsWor->CSUFF = '\002';
			} else if (!strcmp(s,WORDCKECKSYM)) {
				ts->WORDCHECK = FALSE;
				if (strcmp(ts->code, "*") == 0)
					tsWor->WORDCHECK = FALSE;
			} else if (uS.fpatmat(s,UPREFSSYM)) {
				ts->prefs = check_getPrefChars(ts->prefs, s);
				if (strcmp(ts->code, "*") == 0)
					tsWor->prefs = check_getPrefChars(tsWor->prefs, s);
			} else {
				check_mktplate(ts,check_mkstring(s),0);
				if (strcmp(ts->code, "*") == 0)
					check_mktplate(tsWor,check_mkstring(s),0);
			}
			s = temp;
		} else
			temp++;
		if (*temp == '[')
			sb = TRUE;
		else if (*temp == ']')
			sb = FALSE;
		if (temp[0] == '@' && temp[1] == 'r' && temp[2] == '<')
			regExp = TRUE;
		else if (regExp && temp[0] == '>')
			regExp = FALSE;
	}
}

static void check_ReadDepFile(const FNType *depfile, char err) {
	FILE *fp;
	int i = 0;
	char lc, c;
	FNType mFileName[FNSize];

#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(wd_dir);
#endif
	if ((fp=checkOpenWdDepfile(depfile)) != NULL) { // 2024-08-22
	} else if ((fp=OpenGenLib(depfile,"r",FALSE,FALSE,mFileName)) == NULL) {
		if (err) {
#ifndef UNX
			fprintf(stderr, "Can't open dep-file:\n  \"%s\"\n", mFileName);
#else
			fprintf(stderr, "Can't open either one of the dep-files:\n  \"%s\", \"%s\"\n", depfile, mFileName);
#endif
			fprintf(stderr, "Check to see if lib directory is set correctly. It is located next to CLAN application.\n");
#ifdef _MAC_CODE
			fprintf(stderr, "\n   Lib directory can be set in \"Commands\" window with \"lib\" button to\n");
			fprintf(stderr, "   \"/Applications/CLAN/lib\" directory, for example.\n");
#endif
#ifdef _WIN32
			fprintf(stderr, "\n   Lib directory can be set in \"Commands\" window with \"lib\" button to\n");
			fprintf(stderr, "   \"C:\\TalkBank\\CLAN\\lib\\\" directory, for example.\n");
#endif
			cleanupLanguages();
			cutt_exit(0);
		} else
			return;
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
#endif
	if (!strcmp(oldDepfile, mFileName)) {
		fclose(fp);
		return;
	}
	strcpy(oldDepfile, mFileName);
	if (isRecursive) {
		check_clean_headerr();
		check_clean_IDsp(check_idsp);
		check_clean_speaker(headsp);
		check_clean__t(headt);
		check_clean__t(maint);
		check_clean__t(codet);
		headerr = NULL;
		check_idsp = NULL;
		headsp = NULL;
		headt = NULL;
		maint = NULL;
		codet = NULL;
	}
	fprintf(stderr, "Dep-file being used: \"%s\"\n", mFileName);

	i = 0;
	lc = '\n';
	uttline[0] = EOS;
	while (1) {
		c = (char)getc_cr(fp, NULL);
		if (lc == '\n' && c == '#') {
			while (!feof(fp) && c != '\n')
				c = (char)getc_cr(fp, NULL);
			if (feof(fp)) {
				uttline[i] = EOS;
				if (uttline[0] != EOS)
					check_ParseTier(uttline);
				break;
			}
		} else if (lc == '\n' && (c == '@' || c == '*' || c == '%')) {
			uttline[i] = EOS;
			if (uttline[0] != EOS)
				check_ParseTier(uttline);
			if (feof(fp))
				break;
			uttline[0] = c;
			i = 1;
		} else if (feof(fp)) {
			uttline[i] = EOS;
			if (uttline[0] != EOS)
				check_ParseTier(uttline);
			break;
		} else
			uttline[i++] = c;
		lc = c;
	}

	CHECK_TIERS *ts;
	TPLATES *tp;
	for (i=0; i < 256; i++)
		isItemPrefix[i] = FALSE;

	for (ts=maint; ts != NULL; ts = ts->nexttier) {
		if (*ts->code == '*') {
			for (tp=ts->tplate; tp != NULL; tp = tp->nextone) {
				if (*tp->pattern == '*' && *(tp->pattern+1) != EOS) {
					if (uS.ismorfchar(tp->pattern, 1, &dFnt, CHECK_MORPHS, MBF)) {
						isItemPrefix[(int)tp->pattern[1]] = FALSE;
					} else {
						i = strlen(tp->pattern) - 1;
						if (uS.ismorfchar(tp->pattern, i, &dFnt, CHECK_MORPHS, MBF))
							isItemPrefix[(int)tp->pattern[i]] = TRUE;
					}
				}
			}
		}
	}
	for (ts=codet; ts != NULL; ts = ts->nexttier) {
		if (strcmp(ts->code, "%wor") == 0) {
			for (tp=ts->tplate; tp != NULL; tp = tp->nextone) {
				if (*tp->pattern == '*' && *(tp->pattern+1) != EOS) {
					if (uS.ismorfchar(tp->pattern, 1, &dFnt, CHECK_MORPHS, MBF)) {
						isItemPrefix[(int)tp->pattern[1]] = FALSE;
					} else {
						i = strlen(tp->pattern) - 1;
						if (uS.ismorfchar(tp->pattern, i, &dFnt, CHECK_MORPHS, MBF))
							isItemPrefix[(int)tp->pattern[i]] = TRUE;
					}
				}
			}
			break;
		}
	}
	fclose(fp);
}

static char isNewDir(void) {
	int  i;
	char t;

	for (i=strlen(oldfname); i >= 0 && oldfname[i] != PATHDELIMCHR; i--) ;
	i++;
	t = oldfname[i];
	oldfname[i] = EOS;
	if (pathcmp(check_localDir, oldfname)) {
		strcpy(check_localDir, oldfname);
		oldfname[i] = t;
		return(TRUE);
	}
	oldfname[i] = t;
	return(FALSE);
}

void init(char f) {
	int n;
#ifdef DEBUG_CLAN
TPLATES *tp;
CHECK_TIERS *ts;
#endif /* DEBUG_CLAN */

	if (f) {
		FileCount = 0;
		check_all_err_found = FALSE;
		headt = NULL;
		maint = NULL;
		codet = NULL;
		FTime = TRUE;
		check_UDerr = NULL;
		check_idsp = NULL;
		headsp = NULL;
		headerr = NULL;
		check_isNumberShouldBeHere = FALSE;
		check_isCAFound = FALSE;
		check_isArabic = FALSE;
		check_isHebrew = FALSE;
		check_isGerman = FALSE;
		check_isMultiBullets = FALSE;
		check_isCheckBullets = TRUE;
		check_isUnlinkedFound = FALSE;
		check_applyG2Option = TRUE;
		check_check_mismatch = TRUE;
		isCheckUDFeats = FALSE;
		checkBullets = 0;
		check_WordExceptions = NULL;
		check_localDir[0] = EOS;
		oldDepfile[0] = EOS;
		ErrorRepFlag = FALSE;
		for (n=0; n < LAST_ERR_NUM; n++)
			ErrorReport[n] = TRUE;
		check_GenOpt = 0;
		check_GenOpt = D_D_SET_0(check_GenOpt);		/* set to 1 if '-?' is not a delimiter */
		check_GenOpt = NAME_ROLE_SET_0(check_GenOpt);	/* set to 1 if want to check for CHI-Target_Child */
		check_GenOpt = W_CHECK_SET_1(check_GenOpt);	/* set to 0 if WORDS should not be checked for <'> and digits */
		check_GenOpt = SET_CHECK_ID_1(check_GenOpt);	/* set to 0 if you do not want to check for missing @ID */
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		free(defheadtier);
		defheadtier = NULL;

/* 2011-04-07
		for (n=0; n < NUMUSEDTIERSONCE; n++) {
			tiersUsedOnce[n].spCode[0] = EOS;
			tiersUsedOnce[n].fileNum = 0;
			tiersUsedOnce[n].freq = 0;
		}
*/
	} else {
//		isCreateFakeMor = 2;
		check_SNDFname[0] = EOS;
		check_SNDBeg = 0L;
		check_SNDEnd = 0L;
		check_tierBegTime = 0L;
		check_isNumberShouldBeHere = FALSE;
		check_isCAFound = FALSE;
		check_isArabic = FALSE;
		check_isHebrew = FALSE;
		check_isGerman = FALSE;
		check_isMultiBullets = FALSE;
		check_isCheckBullets = TRUE;
		check_isUnlinkedFound = FALSE;
		check_clean_template(check_WordExceptions);
		check_WordExceptions = NULL;
		check_clean_IDsp(check_idsp);
		check_idsp = NULL;
		check_clean_speaker(headsp);
		headsp = NULL;
		check_clean_headerr();
		headerr = NULL;
		*utterance->speaker = EOS;
		for (n=0; n < NUMLANGUAGESINTABLE; n++)
			check_LanguagesTable[n][0] = EOS;
		if (FTime) {
			ReadLangsFile(FALSE);
			if (!isRecursive) {
				check_ReadDepFile(DEPFILE, TRUE);
			} else
				onlydata = 3;
			FTime = FALSE;
#ifdef DEBUG_CLAN
for (ts=headt; ts != NULL ; ts = ts->nexttier) {
	printf("t=%s%c ", ts->code, (ts->col ? ':' : ' '));
	for(tp=ts->tplate; tp!=NULL; tp=tp->nextone) printf("(%s) ",tp->pattern);
	putchar('\n');
}
for (ts=maint; ts != NULL ; ts = ts->nexttier) {
	printf("t=%s%c ", ts->code, (ts->col ? ':' : ' '));
	for(tp=ts->tplate; tp!=NULL; tp=tp->nextone) printf("(%s) ",tp->pattern);
	putchar('\n');
}
for (ts=codet; ts != NULL ; ts = ts->nexttier) {
	printf("t=%s%c ", ts->code, (ts->col ? ':' : ' '));
	for(tp=ts->tplate; tp!=NULL; tp=tp->nextone) printf("(%s) ",tp->pattern);
	putchar('\n');
}
#endif /* DEBUG_CLAN */
		}
		if (isRecursive) {
			if (isNewDir()) {
				check_ReadDepFile(DEPFILE, TRUE);
			}
		}
	}
}

static int check_adderror(int s, int e, int num, char *st) {
	char t;
	ERRLIST *te;

	if (onlydata == 0 || onlydata == 3) return(TRUE);
	if (s == e) {
		if (num != 4 && num != 36 && onlydata != 2) return(TRUE);
	}
	t = st[++e];
	st[e] = EOS;
	if (headerr == NULL) {
		if ((headerr=NEW(ERRLIST)) == NULL) check_overflow();
		te = headerr;
	} else {
		strcpy(templineC,st+s);
//		uS.uppercasestr(templineC, &dFnt, C_MBF);
		for (te=headerr; 1; te=te->nexterr) {
			if ((!strcmp(te->st,templineC) || onlydata==2) && te->errnum==num){
				st[e] = t;
				return(FALSE);
			} else if (te->errnum == 36 && num == 36) {
				st[e] = t;
				return(FALSE);
			}

			if (te->nexterr == NULL) {
				if ((te->nexterr=NEW(ERRLIST)) == NULL) check_overflow();
				te = te->nexterr;
				break;
			}
		}
	}
	te->nexterr = NULL;
	te->st = check_mkstring(st+s);
	te->errnum = num;
	st[e] = t;
	return(TRUE);
}

static int check_displ(int s, int e, long ln, int num) {
	int j, i;
	long ei;
	char isAttOff = TRUE, isCloseAttMarker;

	err_itm[0] = EOS;
	if (!check_adderror(s,e,num,uttline))
		return(FALSE);
	if (totalFilesNum > MINFILESNUM && !stin) { // 2019-04-18 TotalNumFiles
#ifndef UNX
		if (redirect_out.fp == NULL) // 2024-08-29
#endif
			fprintf(stderr, "\r    	     \r");
	}
	fprintf(fpout,"*** File \"%s\": ", oldfname);
	if (ln == -1L)
		fprintf(fpout,"end of file.\n");
	else
		fprintf(fpout,"line %ld.\n", ln);
	i  = 0;
	if (*uttline) {
		fputs(utterance->speaker, fpout);
		ei = 0L;
		isCloseAttMarker = FALSE;
		j = 0;
		templineC2[0] = EOS;
		while (uttline[i]) {
			if (i == s && s <= e && s > -1) {
				if (isCloseAttMarker) {
#ifndef UNX
					sprintf(templineC2+j, "%c%c", ATTMARKER, error_end);
					j += 2;
#endif
					isCloseAttMarker = FALSE;
				}
#ifndef UNX
				sprintf(templineC2+j, "%c%c", ATTMARKER, error_start);
				j += 2;
#endif
				isAttOff = FALSE;
			}
			if (i == e+1 && s <= e && s > -1) {
				isAttOff = TRUE;
				isCloseAttMarker = TRUE;
			}
			if (isCloseAttMarker) {
				if (UTF8_IS_LEAD(uttline[i]) || UTF8_IS_SINGLE(uttline[i])) {
					isCloseAttMarker = FALSE;
#ifndef UNX
					sprintf(templineC2+j, "%c%c", ATTMARKER, error_end);
					j += 2;
#endif
				}
			}
			if (!isAttOff && ei < 512) {
				if (uttline[i] == HIDEN_C) {
					strcpy(err_itm, "<bullet>");
					ei = 15;
				} else if (uttline[i] == ' ' && ei < 1) {
					strcpy(err_itm, "<space>");
					ei = 15;
				} else if (uttline[i] == '\t' && ei < 1) {
					strcpy(err_itm, "<tab>");
					ei = 15;
				} else {
					if (/*dFnt.isUTF && */(uttline[i] >= 127 || !UTF8_IS_SINGLE((unsigned char)uttline[i]))) {
						err_itm[ei++] = '\001';
					} else
						err_itm[ei++] = uttline[i];
				}
			}
			templineC2[j++] = uttline[i];
			i++;
		}
		if (isCloseAttMarker) {
#ifndef UNX
			sprintf(templineC2+j, "%c%c", ATTMARKER, error_end);
			j += 2;
#endif
		}
		for (ei--; ei >= 0 && err_itm[ei] == '\001'; ei--) ;
		err_itm[ei+1] = EOS;
#ifndef UNX
		if (!isAttOff) {
			sprintf(templineC2+j, "%c%c", ATTMARKER, error_end);
			j += 2;
		}
#endif
		if (uttline[i-1] != '\n')
			templineC2[j++] = '\n';
		templineC2[j] = EOS;
		fprintf(fpout, "%s", templineC2);
	}
	return(TRUE);
}

static void check_mess(int wh) {
	int i;

	check_err_found = TRUE;
	switch(wh) {
		case 1: fprintf(fpout,"Expected characters are: @ or %% or *.(%d)\n", wh);
				break;
		case 2: fprintf(fpout,"Missing ':' character and argument.(%d)\n", wh); break;
		case 3: fprintf(fpout,"Missing either TAB or SPACE character.(%d)\n", wh);
				break;
		case 4: if (onlydata == 0 || onlydata == 3) {
					fprintf(fpout,"Found a space character instead of TAB character after Tier name\n");
				} else {
					fprintf(fpout,"Found a space character......\n");
				}
				fprintf(fpout,"Please run \"chstring +q +1\" command on this file to fix this error. (%d)\n", wh);
				break;
		case 5: fprintf(fpout,"Colon (:) character is illegal.(%d)\n", wh); break;
		case 6: fprintf(fpout,"\"@Begin\" is missing at the beginning of the file.(%d)\n", wh);
				break;
		case 7: fprintf(fpout,"\"@End\" is missing at the end of the file.(%d)\n", wh); break;
		case 8: fprintf(fpout,"Expected characters are: @ %% * TAB.(%d)\n", wh);
				break;
		case 9: fprintf(fpout,"Tier name is longer than %d.(%d)\n",SPEAKERLEN,wh);
				break;
		case 10: fprintf(fpout,"Tier text is longer than %ld.(%d)\n",UTTLINELEN,wh);
				 break;
		case 11: fprintf(fpout,"Symbol is not declared in the depfile.(%d)\n", wh); break;
		case 12: fprintf(fpout,"Missing speaker name and/or role.(%d)\n", wh); break;
		case 13: fprintf(fpout,"Duplicate speaker declaration.(%d)\n", wh); break;
		case 14: fprintf(fpout,"Spaces before tier code.(%d)\n", wh); break;
		case 15: fprintf(fpout,"Illegal role. Please see \"depfile.cut\" for list of roles.(%d)\n", wh); break;
		case 16: fprintf(fpout,"Illegal use of extended characters in speaker names.(%d)\n", wh); break;
		case 17: fprintf(fpout,"Tier is not declared in depfile file.(%d)\n", wh);
				 break;
		case 18: fprintf(fpout,"Speaker ");
				 for (i=0; utterance->speaker[i] != ':' && utterance->speaker[i]; i++)
					 putc(utterance->speaker[i],fpout);
				 fprintf(fpout," is not specified in a participants list.(%d)\n", wh);
				 break;
		case 19: fprintf(fpout,"Illegal use of delimiter in a word.\n");
				 fprintf(fpout,"Or a SPACE should be added after it.(%d)\n", wh);
				 break;
		case 20: fprintf(fpout,"Undeclared suffix in depfile.(%d)\n", wh); break;
		case 21: fprintf(fpout,"Utterance delimiter expected.(%d)\n", wh); break;
		case 22: fprintf(fpout,"Unmatched [ found on the tier.(%d)\n", wh); break;
		case 23: fprintf(fpout,"Unmatched ] found on the tier.(%d)\n", wh); break;
		case 24: fprintf(fpout,"Unmatched < found on the tier.(%d)\n", wh); break;
		case 25: fprintf(fpout,"Unmatched > found on the tier.(%d)\n", wh); break;
		case 26: fprintf(fpout,"Unmatched { found on the tier.(%d)\n", wh); break;
		case 27: fprintf(fpout,"Unmatched } found on the tier.(%d)\n", wh); break;
		case 28: fprintf(fpout,"Unmatched ( found on the tier.(%d)\n", wh); break;
		case 29: fprintf(fpout,"Unmatched ) found on the tier.(%d)\n", wh); break;
		case 30: fprintf(fpout,"Text is illegal.(%d)\n", wh); break;
		case 31: fprintf(fpout,"Missing text after the colon.(%d)\n", wh); break;
		case 32: fprintf(fpout,"Code is not declared in depfile.(%d)\n", wh);break;
		case 33: fprintf(fpout,"Either illegal date or time or symbol is not declared in depfile.(%d)\n", wh);
				 break;
		case 34: fprintf(fpout,"Illegal date representation.(%d)\n", wh); break;
		case 35: fprintf(fpout,"Illegal time representation.(%d)\n", wh); break;
		case 36: fprintf(fpout,"Utterance delimiter must be at the end of the utterance.(%d)\n", wh);
/*   fprintf(fpout,"Use \"fixit\" program to break up this tier.(%d)\n", wh); */
				 break;
		case 37: fprintf(fpout,"Undeclared prefix.(%d)\n", wh); break;
		case 38: fprintf(fpout,"Numbers should be written out in words.(%d)\n", wh);
				 break;
		case 39: fprintf(fpout,"Code tier must NOT follow header tier.(%d)\n", wh);
				 break;
		case 40: fprintf(fpout,"Duplicate code tiers per one main tier are NOT allowed.(%d)\n", wh);
				 break;
		case 41: fprintf(fpout,"Parentheses around words are illegal.(%d)\n", wh);
				 break;
		case 42: fprintf(fpout,"Use either \"&\" or \"()\", but not both.(%d)\n", wh);
				 break;
		case 43: fprintf(fpout,"The file must start with \"@Begin\" tier.(%d)\n", wh);
				 break;
		case 44: fprintf(fpout,"The file must end with \"@End\" tier.(%d)\n", wh);
				 fprintf(fpout,"Possibly there are some blank lines at the end of the file.(%d)\n", wh);
				 break;
		case 45: fprintf(fpout,"There were more @Bg than @Eg tiers found.(%d)\n", wh);
				 break;
		case 46: fprintf(fpout,"This @Eg does not have matching @Bg.(%d)\n", wh);
				 break;
		case 47: fprintf(fpout,"Numbers are not allowed inside words.(%d)\n", wh);
				 break;
		case 48: if (err_itm[0] == EOS)
					fprintf(fpout,"Illegal character(s) found.(%d)\n", wh);
				 else
					fprintf(fpout,"Illegal character(s) '%s' found.(%d)\n", err_itm, wh);
				 break;
		case 49: fprintf(fpout,"Upper case letters are not allowed inside a word.(%d)\n", wh);
				 break;
		case 50: fprintf(fpout,"Redundant utterance delimiter.(%d)\n", wh); break;
		case 51: fprintf(fpout,"expected [ ]; < > should be followed by [ ]\n"); break;
		case 52: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must be preceded by text.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must be preceded by text.(%d)\n", err_itm, wh);
				 break;
		case 53: fprintf(fpout,"Only one \"@Begin\" can be in a file.(%d)\n", wh); break;
		case 54: fprintf(fpout,"Only one \"@End\" can be in a file.(%d)\n", wh); break;
				 break;
		case 55: fprintf(fpout,"Unmatched ( found in the word.(%d)\n", wh); break;
		case 56: fprintf(fpout,"Unmatched ) found in the word.(%d)\n", wh); break;
		case 57: if (err_itm[0] == EOS)
					fprintf(fpout,"Please add space between word and pause symbol.(%d)\n", wh);
				 else
					fprintf(fpout,"Please add space between word and pause symbol: '%s'.(%d)\n", err_itm, wh);
				 break;
		case 58: fprintf(fpout,"Tier name is longer than 8 characters.(%d)\n", wh); break;
		case 59: if (err_itm[0] == EOS)
#if defined(_MAC_CODE)
					fprintf(fpout,"Expected second %c character.(%d)\n", '\245', wh);
				else
#elif defined(_WIN32) // _MAC_CODE
					fprintf(fpout,"Expected second %c character.(%d)\n", '\225', wh);
				else
#endif
					fprintf(fpout,"Expected second %s character.(%d)\n", err_itm, wh);
				break;
		case 60: fprintf(fpout,"\"@ID:\" tier is missing in the file.\nPlease run \"insert\" in Commands window on this data file.(%d)\n", wh); break;
		case 61: fprintf(fpout,"\"@Participants:\" tier is expected here.(%d)\n", wh); break;
		case 62: fprintf(fpout,"Missing language information.(%d)\n", wh); break;
		case 63: fprintf(fpout,"Missing Corpus name.(%d)\n", wh); break;
		case 64: fprintf(fpout,"Wrong gender information (Choose: female or male).(%d)\n", wh); break;
		case 65: if (err_itm[0] == EOS)
					fprintf(fpout,"This item can not be followed by the next symbol.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' can not be followed by the next symbol.(%d)\n", err_itm, wh);
				 break;
		case 66: if (err_itm[0] == EOS)
					fprintf(fpout,"Illegal character in a word.\n");
				 else
					fprintf(fpout,"Illegal character '%s' in a word.\n", err_itm);
				 fprintf(fpout,"Or a SPACE should be added before it.(%d)\n", wh);
				 break;
		case 67: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must be followed by text, \n");
				 else
					fprintf(fpout,"Item '%s' must be followed by text, \n", err_itm);
				fprintf(fpout," preceded by SPACE or be removed.(%d)\n", wh);
				break;
		case 68: fprintf(fpout,"PARTICIPANTS TIER IS MISSING \"CHI Target_Child\".(%d)\n", wh);
				break;
		case 69: fprintf(fpout,"The UTF8 header is missing. If you edit and save the file, it will be inserted.(%d)\n", wh);
				break;
		case 70: fprintf(fpout,"Expected either text or \"0\" on this tier.(%d)\n", wh);
				break;
		case 71: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must be before pause (#).(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must be before pause (#).(%d)\n", err_itm, wh);
				 break;
		case 72: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must precede the utterance delimiter or CA delimiter.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must precede the utterance delimiter or CA delimiter.(%d)\n", err_itm, wh);
				 break;
		case 73: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must be preceded by text or '0'.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must be preceded by text or '0'.(%d)\n", err_itm, wh);
				 break;
		case 74: fprintf(fpout,"Only one tab after ':' is allowed.(%d)\n", wh); break;
		case 75: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must follow after utterance delimiter.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must follow after utterance delimiter.(%d)\n", err_itm, wh);
				 break;
		case 76: fprintf(fpout,"Only one letter is allowed with '@l'.(%d)\n", wh); break;
		case 77: fprintf(fpout,"\"@Languages:\" tier is expected here.(%d)\n", wh); break;
		case 78: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must be used at the beginning of tier.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must be used at the beginning of tier.(%d)\n", err_itm, wh);
				 break;
		case 79: fprintf(fpout,"Only one occurrence of | symbol per word is allowed.(%d)\n", wh); break;
		case 80: fprintf(fpout,"There must be at least one occurrence of '|'.(%d)\n", wh); break;
		case 81: fprintf(fpout,"Bullet must follow utterance delimiter or be followed by end-of-line.(%d)\n", wh);
				 break;

		case 82: fprintf(fpout,"BEG mark of bullet must be smaller than END mark.(%d)\n", wh);
				 break;
		case 83: fprintf(fpout,"Current BEG time is smaller than previous' tier BEG time(%d)\n", wh);
				 break;
		case 84: fprintf(fpout,"Current BEG time is smaller than previous' tier END time by %ld msec.(%d)\n", tDiff, wh);
				 break;
		case 85: fprintf(fpout,"Gap found between current BEG time and previous' tier END time.(%d)\n", wh);
				 break;
		case 86: fprintf(fpout,"Illegal character. Please re-enter it using Unicode standard.(%d)\n", wh);
				 break;
		case 87: fprintf(fpout,"Malformed structure.(%d)\n", wh); break;
		case 88: fprintf(fpout,"Illegal use of compounds and special form markers.(%d)\n", wh); break;
		case 89: fprintf(fpout,"Missing or extra or wrong characters found in bullet.(%d)\n", wh); break;
		case 90: fprintf(fpout,"Illegal time representation inside a bullet.(%d)\n", wh); break;
		case 91: fprintf(fpout,"Blank lines are not allowed.(%d)\n", wh); break;
		case 92: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must be followed by space or end-of-line.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must be followed by space or end-of-line.(%d)\n", err_itm, wh);
				 break;
		case 93: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must be preceded by SPACE.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must be preceded by SPACE.(%d)\n", err_itm, wh);
				break;
		case 94: fprintf(fpout,"Mismatch of speaker and %%mor: utterance delimiters.(%d)\n", wh); break;
		case 95: fprintf(fpout,"Illegal use of capitalized words in compounds.(%d)\n", wh); break;
		case 96: fprintf(fpout,"Word color is now illegal.(%d)\n", wh); break;
// 18-07-2008		case 96: fprintf(fpout,"Illegal color name. (blue, red, green, magenta).(%d)\n", wh); break;
		case 97: fprintf(fpout,"Illegal character inside parentheses.(%d)\n", wh); break;
		case 98: fprintf(fpout,"Space is not allow in media file name inside bullets.(%d)\n", wh); break;
		case 99: fprintf(fpout,"Extension is not allow at the end of media file name.(%d)\n", wh); break;
		case 100: fprintf(fpout,"Commas at the end of PARTICIPANTS tier are not allowed.(%d)\n", wh); break;
		case 101: if (err_itm[0] == EOS)
					fprintf(fpout,"This item must be followed or preceded by text.(%d)\n", wh);
				 else
					fprintf(fpout,"Item '%s' must be followed or preceded by text.(%d)\n", err_itm, wh);
				 break;
		case 102: fprintf(fpout,"Italic markers are no longer legal in CHAT.(%d)\n", wh); break;
		case 103: fprintf(fpout,"Illegal use of both CA and IPA on \"@Options:\" tier.(%d)\n", wh); break;
		case 104: fprintf(fpout,"Please select \"CAfont\" or \"Ascender Uni Duo\" font for CA file as per \"@Options:\" tier.(%d)\n", wh); break;
		case 105: fprintf(fpout,"Please select \"Charis SIL\" font for IPA file as per \"@Options:\" tier.(%d)\n", wh); break;
		case 106: fprintf(fpout,"The whole code must be on one line. Please run chstring +q on this file.(%d)\n", wh); break;
		case 107: fprintf(fpout,"Only single commas are allowed in tier.(%d)\n", wh); break;
		case 108: fprintf(fpout,"All postcodes must precede final bullet.(%d)\n", wh); break;
		case 109: fprintf(fpout,"Postcodes are not allowed on dependent tiers.(%d)\n", wh); break;
		case 110: fprintf(fpout,"No bullet found on this tier.(%d)\n", wh); break;
		case 111: if (err_itm[0] == EOS)
					fprintf(fpout,"Illegal pause format. Pause has to have '.'(%d)\n", wh);
				else
					fprintf(fpout,"Pause needs '.' in '%s' or this item is in wrong location.(%d)\n", err_itm, wh);
				break;
		case 112: fprintf(fpout,"Missing %s tier with media file name in headers section at the top of the file.(%d)\n", MEDIAHEADER, wh); break;
		case 113: fprintf(fpout,"Illegal keyword, use \"audio\", \"video\" or look in depfile.cut.(%d)\n", wh); break;
		case 114: fprintf(fpout,"Add \"audio\", \"video\" or look in depfile.cut for more keywords after the media file name on %s tier.(%d)\n", MEDIAHEADER, wh); break;
		case 115: fprintf(fpout,"Old bullets format found. Please run \"fixbullets\" program to fix this data. (%d)\n", wh); break;
		case 116: fprintf(fpout,"Specifying Font for individual lines is illegal. Please open this file and save it again. (%d)\n", wh); break;
		case 117: if (err_itm[0] == EOS)
					fprintf(fpout,"This character must be used in pairs. See if any are unmatched.(%d)\n", wh);
				else
					fprintf(fpout,"Character %s must be used in pairs. See if any are unmatched.(%d)\n", err_itm, wh);
				break;
		case 118: fprintf(fpout,"Utterance delimiter must precede final bullet.(%d)\n", wh); break;
		case 119: if (err_itm[0] == EOS)
					fprintf(fpout,"Missing word after code (%d)\n", wh);
		  		 else
					 fprintf(fpout,"Missing word after code \"%s\" (%d)\n", err_itm, wh);
				break;
//		case 119: fprintf(fpout,"Illegal to imbed code inside the scope of the same code.(%d)\n", wh); break;
		case 120: if (check_fav_lang[0] == EOS)
				fprintf(fpout,"Please use three letter language code.(%d)\n", wh);
			else {
				fprintf(fpout,"Please use \"%s\" language code instead.(%d)\n", check_fav_lang, wh); 
				fprintf(fpout,"    Or see if \"fixlang\" CLAN command in commands window can fix codes automaticaly.(%d)\n", wh);
			}
			break;
		case 121: if (err_itm[0] == EOS)
				fprintf(fpout,"Language code not found in CLAN/lib/fixes/ISO-639.cut file.(%d)\n", wh); 
			else
				fprintf(fpout,"Language code \"%s\" not found in \"CLAN/lib/fixes/ISO-639.cut\" file.(%d)\n", err_itm, wh); 
			fprintf(fpout,"    If it is a legal code, then please add it to \"CLAN/lib/fixes/ISO-639.cut\" file.\n"); 
			break;
		case 122: fprintf(fpout,"Language on @ID tier is not defined on \"@Languages:\" header tier.(%d)\n", wh); break;
		case 123: if (err_itm[0] == EOS)
					fprintf(fpout,"Illegal character found in tier text. If it CA, then add \"@Options: CA\"(%d)\n", wh);
				else
					fprintf(fpout,"Illegal character '%s' found in tier text. If it CA, then add \"@Options: CA\"(%d)\n", err_itm, wh);
			break;
		case 124: fprintf(fpout,"Please remove \"unlinked\" from @Media header.(%d)\n", wh); break;
		case 125: fprintf(fpout,"\"@Options\" header must immediately follow \"@Participants:\" header.(%d)\n", wh); break;
		case 126: fprintf(fpout,"\"@ID\" header must immediately follow \"@Participants:\" or \"@Options\" header.(%d)\n", wh); break;
		case 127: fprintf(fpout,"Header must follow \"@ID:\" or \"@Birth of\" or \"@Birthplace of\" or \"@L1 of\" header.(%d)\n", wh); break;
		case 128: fprintf(fpout,"Unmatched %c%c%c found on the tier.(%d)\n", 0xE2, 0x80, 0xB9, wh); break;
		case 129: fprintf(fpout,"Unmatched %c%c%c found on the tier.(%d)\n", 0xE2, 0x80, 0xBA, wh); break;
		case 130: fprintf(fpout,"Unmatched %c%c%c found on the tier.(%d)\n", 0xE3, 0x80, 0x94, wh); break;
		case 131: fprintf(fpout,"Unmatched %c%c%c found on the tier.(%d)\n", 0xE3, 0x80, 0x95, wh); break;
		case 132: fprintf(fpout,"Tabs should only be used to mark the beginning of lines.(%d)\n", wh); break;
		case 133: fprintf(fpout,"BEG time is smaller than same speaker's previous END time by %ld msec.(%d)\n", tDiff, wh); break;
		case 134: if (err_itm[0] == EOS)
				fprintf(fpout,"This item is illegal. Please run \"mor\" command on this data.(%d)\n", wh);
			else
				fprintf(fpout,"Item '%s' is illegal. Please run \"mor\" command on this data.(%d)\n", err_itm, wh);
			break;
		case 135: if (err_itm[0] == EOS)
				fprintf(fpout,"This item is illegal.(%d)\n", wh);
			else
				fprintf(fpout,"Item '%s' is illegal.(%d)\n", err_itm, wh);
			break;
		case 136: fprintf(fpout,"Unmatched %c%c%c found on the tier.(%d)\n", 0xE2, 0x80, 0x9C, wh); break;
		case 137: fprintf(fpout,"Unmatched %c%c%c found on the tier.(%d)\n", 0xE2, 0x80, 0x9D, wh); break;
		case 138: fprintf(fpout,"Special quote U2019 must be replaced by single quote (').(%d)\n", wh); break;
		case 139: fprintf(fpout,"Special quote U2018 must be replaced by single quote (').(%d)\n", wh); break;
		case 140: fprintf(fpout,"Tier \"%%MOR:\" does not link in size to its speaker tier.(%d)\n", wh); break;
		case 141: if (err_itm[0] == EOS)
				fprintf(fpout,"[: ...] has to be preceded by only one word and nothing else.(%d)\n", wh);
			else
				fprintf(fpout,"'%s' must be preceded by only one word and nothing else.(%d)\n", err_itm, wh);
			break;
		case 142: fprintf(fpout,"Speaker's role on @ID tier does not match role on @Participants: tier.(%d)\n", wh); break;
		case 143: fprintf(fpout,"The @ID line needs 10 fields.(%d)\n", wh); break;
		case 144: fprintf(fpout,"Either illegal SES field value or symbol is not declared in depfile.(%d)\n", wh); break;
		case 145: fprintf(fpout,"This intonational marker should be outside paired markers.(%d)\n", wh); break;
		case 146: fprintf(fpout,"The &= symbol must include some code after '=' character.(%d)\n", wh); break;
		case 147: fprintf(fpout,"Undeclared special form marker in depfile.(%d)\n", wh); break;
		case 148: fprintf(fpout,"Space character is not allowed before comma(,) character on \"@Media:\" header.(%d)\n", wh); break;
		case 149: if (err_itm[0] == EOS)
				fprintf(fpout,"Illegal character located between a word and [...] code.(%d)\n", wh);
			else
				fprintf(fpout,"Illegal character '%s' located between a word and [...] code.(%d)\n", err_itm, wh);
			break;
		case 150: fprintf(fpout,"Illegal item located between a word and [...] code.(%d)\n", wh); break;
		case 151: fprintf(fpout,"This word has only repetition segments.(%d)\n", wh); break;

		case 152: fprintf(fpout,"Language is not defined on \"@Languages:\" header tier.(%d)\n", wh); break;
		case 153: fprintf(fpout,"Age's month or day are missing initial zero.\nPlease run \"chstring +q +1\" command on this file to fix this error.(%d)\n", wh); break;
		case 154: fprintf(fpout,"Please add \"unlinked\" to @Media header.(%d)\n", wh); break;
		case 155: if (err_itm[0] == EOS)
				fprintf(fpout,"Please use \"0word\" instead of \"(word)\".(%d)\n", wh);
			else {
				i = strlen(err_itm) - 1;
				if (i > 0)
					err_itm[i] = EOS;
				fprintf(fpout,"Please use \"0%s\" instead of \"(%s)\".(%d)\n", err_itm+1, err_itm+1,wh);
			}
			break;
		case 156: fprintf(fpout,"Please replace ,, with F2-t (%c%c%c) character.(%d)\n", 0xE2, 0x80, 0x9E, wh); break;
		case 157: fprintf(fpout,"Media file name has to match datafile name.(%d)\n", wh); break;
		case 158: fprintf(fpout,"[: ...] has to have real word, not 0... or &... or xxx.(%d)\n", wh); break;
		case 159: fprintf(fpout,"Pause markers should appear after retrace markers.(%d)\n", wh); break;
		case 160: fprintf(fpout,"Space character is not allowed after '<' or before '>' character.(%d)\n", wh); break;
		case 161: fprintf(fpout,"Space character is required before '[' code item.(%d)\n", wh); break;

/* when adding new error change number in LAST_ERR_NUM */
		default: fprintf(fpout,"Internal ERROR. #%d is not defined!\n",wh);
				 break;
	}
}

static char check_err(int wh, int s, int e, long ln) {
	if (ErrorRepFlag && !ErrorReport[wh-1])
		return(FALSE);
	if (check_displ(s,e,ln,wh))
		check_mess(wh);
	return(TRUE);
}

static void check_CodeErr(int wh) {
	int i, j;

	if (ErrorRepFlag && !ErrorReport[wh-1])
		return;
	for (j=0; utterance->speaker[j] != ':' && utterance->speaker[j]; j++) ;
	if (!check_adderror(0,j,wh,utterance->speaker))
		return;
	if (totalFilesNum > MINFILESNUM && !stin) { // 2019-04-18 TotalNumFiles
		fprintf(stderr, "\r    	     \r");
	}
	fprintf(fpout,"*** File \"%s\": ", oldfname);
	fprintf(fpout,"line %ld.\n", lineno);
	fputs(utterance->speaker, fpout);
	for (i=0; uttline[i] != '\n' && uttline[i]; i++)
		putc(uttline[i],fpout);
	if (uttline[i]) i++;
	putc('\n', fpout);
	for (j=0; utterance->speaker[j] != ':' && utterance->speaker[j]; j++)
		putc('^',fpout);
	putc('\n', fpout);
	if (uttline[i]) {
		for (; uttline[i]; i++)
			putc(uttline[i],fpout);
		if (uttline[i-1] != '\n')
			putc('\n', fpout);
	}
	err_itm[0] = EOS;
	check_mess(wh);
}

static CHECK_TIERS *check_FindTier(CHECK_TIERS *ts, const char *c) {
	char tc[BUFSIZ+1];
	for (; ts != NULL; ts = ts->nexttier) {
		if (*c == *ts->code) {
			strncpy(tc, c, BUFSIZ);
			tc[BUFSIZ] = EOS;
			if (uS.patmat(tc+1,ts->code+1))
				return(ts);
			else if (*c == '*' && *ts->code == '*')
				return(ts);
		}
	}
	return(NULL);
}

static char check_correctMonth(char *s) {
	if (!strncmp(s, "JAN", 3))
		return(TRUE);
	else if (!strncmp(s, "FEB", 3))
		return(TRUE);
	else if (!strncmp(s, "MAR", 3))
		return(TRUE);
	else if (!strncmp(s, "APR", 3))
		return(TRUE);
	else if (!strncmp(s, "MAY", 3))
		return(TRUE);
	else if (!strncmp(s, "JUN", 3))
		return(TRUE);
	else if (!strncmp(s, "JUL", 3))
		return(TRUE);
	else if (!strncmp(s, "AUG", 3))
		return(TRUE);
	else if (!strncmp(s, "SEP", 3))
		return(TRUE);
	else if (!strncmp(s, "OCT", 3))
		return(TRUE);
	else if (!strncmp(s, "NOV", 3))
		return(TRUE);
	else if (!strncmp(s, "DEC", 3))
		return(TRUE);
	else
		return(FALSE);
}

static int check_SES_item(char *tmpl, char *s) {
	if (*tmpl == 's')
		tmpl++;
	if (*tmpl == '<')
		tmpl++;
	while (*tmpl != EOS && *s != EOS) {
		if (*tmpl != *s)
			break;
		tmpl++;
		s++;
	}
	if ((*tmpl == '>' || *tmpl == EOS) && *s == EOS)
		return(TRUE);
	else
		return(FALSE);
}

static int check_MatchDT(char *tmpl, char *s) {
	char time = FALSE;

	if (*tmpl == 't')
		time = TRUE;
	for (tmpl++; *tmpl; tmpl++, s++) {
		if (*tmpl == 'd' && isdigit((unsigned char)*s)) {
			if (*(tmpl-1) != 'd') {
				if (isdigit((unsigned char)*(s+1)) && *s >= '0' && *s <= '3')
					;
/* no 1-AUG-1980 allowed, only 01-AUG-1980
				else if (!isdigit((unsigned char)*(s+1)) && *s >= '0' && *s <= '9')
					tmpl++;
*/
				else if (!isdigit((unsigned char)*(s+1)) && *(tmpl+1) != 'd')
					;
				else
					return(FALSE);
			}
		} else if (*tmpl == 'h' && isdigit((unsigned char)*s)) {
			if (*(tmpl-1) != 'h') {
				if (isdigit((unsigned char)*(s+1)) && *s >= '0' && *s <= '2')
					;
				else if (!isdigit((unsigned char)*(s+1)))
					tmpl++;
				else
					return(FALSE);
			} else if (*(s-1) == '1' && (*s < '0' || *s > '9'))
				return(FALSE);
			else if (*(s-1) == '2' && (*s < '0' || *s > '4'))
				return(FALSE);
		} else if (*tmpl == 'm' && isdigit((unsigned char)*s)) {
			if (*(tmpl-1) != 'm') {
				if (isdigit((unsigned char)*(s+1)) && *s >= '0') {
					if ((*s > '1' && !time) || *s > '5')
						return(FALSE);
					if (*s == '1' && *(s+1) > '2' && !time)
						return(FALSE);
				} else if (!isdigit((unsigned char)*(s+1)))
					tmpl++;
				else
					return(FALSE);
			}
		} else if (*tmpl == 's' && isdigit((unsigned char)*s)) {
			if (*(tmpl-1) != 's') {
				if (isdigit((unsigned char)*(s+1)) && *s >= '0' && *s <= '5')
					;
				else if (!isdigit((unsigned char)*(s+1)))
					tmpl++;
				else
					return(FALSE);
			}
		} else if (*tmpl == 'l' && isalpha((unsigned char)*s)) {
			if (tmpl[1] == 'l' && tmpl[2] == 'l') {
				if (!check_correctMonth(s)) {
					return(FALSE);
				}
			}
		} else if (*tmpl == 'y' && isdigit((unsigned char)*s)) {
			if (*(tmpl-1) != 'y' && !isdigit((unsigned char)*(s+1)))
				tmpl++;
		} else if (*tmpl == '>' || *tmpl == '<')
			s--;
		else if (*tmpl != *s)
			return(FALSE);
	}
	if (isSpace(*s) || *s == '\n' || *s == EOS)
		return(TRUE);
	else
		return(FALSE);
}

static int check_matchnumber(TPLATES *tp, char *s) {
	for (; tp != NULL; tp = tp->nextone) {
		if (*tp->pattern != '*') {
			if (uS.patmat(s,tp->pattern)) return(FALSE);
		}
	}
	return(TRUE);
}

static int check_matchplate(TPLATES *tp, char *s, char charType) {
	char dt = FALSE, ses = FALSE;

	for (; tp != NULL; tp = tp->nextone) {
		if (*tp->pattern == '@') {
			if ((*(tp->pattern+1) == 'd' || *(tp->pattern+1) == 't') && charType == 'D') {
				dt = TRUE;
				if (check_MatchDT(tp->pattern+1,s))
					return(FALSE);
				else if (tp->nextone == NULL) {
					if (*(tp->pattern+1) == 'd')
						return(34);
					else
						return(35);
				}
			} else if ((*(tp->pattern+1) == 's' || *(tp->pattern+1) == 'e') && charType == 's') {
				ses = TRUE;
				if (check_SES_item(tp->pattern+2,s))
					return(FALSE);
			} else if (uS.patmat(s,tp->pattern))
				return(FALSE);
		} else if (*tp->pattern == '*' && *(tp->pattern+1) == EOS) {
			if (*s == '(')
				return(FALSE);
			if (dt) {
				if (isalpha((unsigned char)*s) || (*s < 1 && *s != EOS) || *s >= 127)
					return(FALSE);
			} else if (isalnum((unsigned char)*s) || (*s < 1 && *s != EOS) || *s >= 127) 
				return(FALSE);
			else if (*s == '^') {
				if (isalpha((unsigned char)*(s+1)) || (*(s+1) < 1 && *(s+1) != EOS) || *(s+1)>= 127)
					return(FALSE);
			}
		} else if (tp->pattern[0] == '[' && uS.isSqCodes(s, templineC3, &dFnt, FALSE)) {
			if (uS.patmat(templineC3,tp->pattern)) return(FALSE);
		} else if (*tp->pattern != '*') {
			if (uS.patmat(s,tp->pattern)) return(FALSE);
		}
	}
	if (dt) {
		return(33);
	} else if (ses) {
		return(144);
	} else if (*s == '$') {
		return(32);
	} else {
		return(11);
	}
}

static int check_matchIDsp(struct IDsp *tp, char *s) {
	for (; tp != NULL; tp = tp->nextsp) {
		if (uS.mStricmp(s,tp->sp) == 0) {
			return(TRUE);
		}
	}
	return(FALSE);
}


static SPLIST *check_matchSpeaker(SPLIST *tp, char *s, char isFromID) {
	for (; tp != NULL; tp = tp->nextsp) {
		if (uS.patmat(s,tp->sp)) {
			if (isFromID)
				tp->hasID = TRUE;
			return(tp);
		}
	}
	return(NULL);
}

static int check_matchRole(char *sp, SPLIST *tp, char *s) {
	for (; tp != NULL; tp = tp->nextsp) {
		if (uS.patmat(sp,tp->sp)) {
			if (uS.mStricmp(tp->role, s) == 0)
				return(FALSE);
			else
				return(TRUE);
		}
	}
	return(TRUE);
}

static char check_setLastTime(char *s, long t) {
	SPLIST *tp;

	if (*s == '*')
		s++;
	else if (*s == '%' || *s == '@')
		return(FALSE);
	for (tp=headsp; tp != NULL; tp=tp->nextsp) {
		if (uS.mStricmp(s,tp->sp) == 0) {
			tp->endTime = t;
			return(TRUE);
		}
	}
	return(FALSE);
}

static long check_getLatTime(char *s) {
	SPLIST *tp;

	if (*s == '*')
		s++;
	else if (*s == '%' || *s == '@')
		return(-2L);
	for (tp=headsp; tp != NULL; tp=tp->nextsp) {
		if (uS.mStricmp(s,tp->sp) == 0) {
			return(tp->endTime);
		}
	}
	return(-1L);
}

static char check_CheckFirstCh(long ln, char er) {
	if (!isSpeaker(*uttline) && *uttline != ' ' && *uttline != '\t' && *uttline != EOS) {
		if (check_err(8,0,0,ln))
			er = TRUE;
	}
	return(er);
}

static CHECK_TIERS *check_MatchTierName(CHECK_TIERS *ts, char *c, char  col) {
	CHECK_TIERS *ts_old;

	ts_old = NULL;
	for (; ts != NULL; ts = ts->nexttier) {
		if (uS.partcmp(c,ts->code,TRUE, TRUE)) {
			ts_old = ts;
			if (col == ts->col) return(ts);
		}
	}
	ts = ts_old;
	return(ts);
}

static char check_FoundText(int i) {
	for (; uttline[i]==' ' || uttline[i]=='\t' || uttline[i]== '\n'; i++) ;
	return(uttline[i]);
}

static char check_FoundAlNums(int i, char *word) {
	for (; (!isalnum((unsigned char)word[i]) || word[i] < 1 || word[i] >= 127) && word[i] != EOS; i++) ;
	return(word[i] == EOS);
}

static char isCATime(char *word) {
	register int i;

	for (i=0; isdigit((unsigned char)word[i]); i++) ;
	for (; isSpace(word[i]); i++) ;
	for (; isdigit((unsigned char)word[i]) || word[i] == '(' || word[i] == ')' ||
		word[i] == '.' || isSpace(word[i]); i++) ;
	return(word[i] == EOS);
}

static char check_Tabs(long ln, char er) {
	long i;

	for (i=0L; uttline[i] != EOS; i++) {
		if (uttline[0] != '\t') {
			if (check_isCAFound && uttline[i] == '\t' && i > 0 && uttline[i-1] != ':') {
				if (check_err(132,i,i,ln))
					er = TRUE;
			}
			if (!check_isCAFound && uttline[i] == '\t' && i > 0 && uttline[i-1] == ':' && uttline[i+1] == ' ') {
				if (check_err(123,i+1,i+1,ln))
					return(FALSE);
			}
			if (!check_isCAFound && uttline[i] == ' ' && i == 1 && uttline[i-1] == '\t') {
				if (check_err(123,i,i,ln))
					er = TRUE;
			}
			if (uttline[i] == '\t' && i > 0 && uttline[i-1] != ':') {
				if (check_err(132,i,i,ln))
					er = TRUE;
			}
		}
		if (uttline[i] == ' ' && i == 0) {
			if (check_err(4,i,i,ln))
				er = TRUE;
		}
	}
	return(er);
}

static char check_Colon(long ln, char er) {
	register int  i;
	register char blf;
	CHECK_TIERS *ts;

	if (isSpeaker(*uttline)) {
		i = 0;
		blf = (uttline[i] == HIDEN_C);
		for (; uttline[i] && (uttline[i] != ':' || blf); i++) {
			if (uttline[i] == HIDEN_C)
				blf = !blf;
		}
		strcpy(templineC,uttline);
		uS.remblanks(templineC);
//		uS.uppercasestr(templineC, &dFnt, MBF);
		if (*uttline == '@')
			ts = check_MatchTierName(headt,templineC,(uttline[i] ==':'));
		else {
			if (*uttline == '*')
				ts = check_MatchTierName(maint,templineC,(uttline[i] ==':'));
			else if (*uttline == '%')
				ts = check_MatchTierName(codet,templineC,(uttline[i] ==':'));
			else
				ts = NULL;
		}
		if (uttline[i] != ':') {
			if (ts != NULL) {
				if (ts->col) {
					if (*uttline == '@') {
						if (check_err(2,strlen(ts->code),strlen(ts->code),ln))
							er = TRUE;
					} else if (isSpace(uttline[4])) {
						if (check_err(2,4,4,ln))
							er = TRUE;
					} else {
						if (check_err(2,i,i,ln))
							er = TRUE;
					}
				} else if (check_FoundText(strlen(ts->code))) {
					if (check_err(30,strlen(ts->code),i,ln))
						er = TRUE;
				}
			} else if (*uttline == '*' || *uttline == '%') {
				if (isSpace(uttline[4])) {
					check_err(2,4,4,ln);
				} else
					check_err(2,i,i,ln);
			} else if (!isCATime(uttline)) {
				if (check_err(17,i,i,ln))
					er = TRUE;
			}
		} else {
			i++;
			if (ts != NULL) {
				if (!ts->col) {
					if (check_err(5,i-1,i-1,ln))
						er = TRUE;
					i = -1;
				} else if (!check_FoundText(i) && *uttline != '@'/* to always allow empty header tiers*/) {
					if (check_err(31,i,strlen(uttline),ln))
						er = TRUE;
					i = -1;
				}
			}
			if (i == -1) ;
			else if (!check_FoundText(i) && *uttline == '@'/* to always allow empty header tiers*/) ;
			else if (!isSpace(uttline[i])) {
				if (check_err(3,i,i,ln))
					er = TRUE; 
			} else if (1/* *uttline != '@' */) {
				if (uttline[i] == ' ') {
					if (check_err(4,i,i,ln)) er = TRUE;
				} else if (isSpace(uttline[i+1])) {
// 2004-04-27					if (check_err(74,i+1,i+1,ln)) er = TRUE;
				}
			}
		}
	}
	return(er);
}

static char check_CheckErr(long ln, char er) {
	int i, s, e;

	if (isSpace(*uttline)) {
		s = 0;
		for (i=0; uttline[i] == ' ' || uttline[i] == '\t'; i++) ;
		e = i - 1;
		if (isSpeaker(uttline[i])) {
			for (; uttline[i] && i < 79 && uttline[i] != ':'; i++) ;
			if (uttline[i] == ':' && *uttline != '\t')
				check_err(14,s,e,ln);
		}
	}
	return(er);
}

static char check_isMissingInitialZero(char *line, int s) { // lxs
	int i;

	for (i=s; line[i] != EOS && line[i] != '|'; i++) {
		if (line[i] == ';' && isdigit(line[i+1]) && line[i+2] == '.') {
			check_err(153,i,i+1,lineno);
			return(TRUE);
		} else if (line[i] == '.' && isdigit(line[i+1]) && line[i+2] == '|') {
			check_err(153,i,i+1,lineno);
			return(TRUE);
		}
	}
	return(FALSE);
}

static void check_Age(CHECK_TIERS *ts, char *sp, char *line, int offset, char eChar) {
	long ln = 0L;
	int r, s, e;
	char word[BUFSIZ], *st;

	s = offset;
	if (sp != NULL) {
		uS.extractString(templineC4, sp, "@Age of ", ':');
		if (check_matchSpeaker(headsp, templineC4, FALSE) == NULL) {
			check_CodeErr(18);
		}
	} else {
		if (check_isMissingInitialZero(line, s))
			return;
	}
	e = s;
	while (1) {
		st = word;
		while ((*st=line[s++]) == ' ' || *st == '\t' || *st == '\n') {
			if (*st == '\n')
				ln += 1;
		}
		if (*st == eChar)
			break;
		e = s;
		while ((*++st=line[e]) != eChar) {
			e++;
			if (*st == '\n' || *st == ' ' || *st == '\t')
				break;
		}
		*st = EOS;
		if ((r=check_matchplate(ts->tplate, word, 'D'))) {
			if (eChar == EOS)
				offset = e - 2;
			else
				offset = e - 1;
			check_err(r,s-1,offset,ln+lineno);
		}
		s = e;
	}
}

static void check_Sex(CHECK_TIERS *ts, char *sp, char *line) {
	char t;
	int s, e;

	if (sp != NULL) {
		uS.extractString(templineC4, sp, "@Sex of ", ':');
		if (check_matchSpeaker(headsp, templineC4, FALSE) == NULL) {
			check_CodeErr(18);
		}
	}
	for (s=0; isSpace(line[s]); s++) ;
	if (line[s] != EOS && line[s] != '\n') {
		for (e=s; line[e] != '\n' && line[e] != EOS; e++) ;
		t = line[e];
		line[e] = EOS;
		if (strcmp(line+s, "male") && strcmp(line+s, "female")) {
			line[e] = t;
			check_err(64,s,e,lineno);
		}
		line[e] = t;
	}
}

static void check_ses(CHECK_TIERS *ts, char *sp, char *line, int offset, char eChar) {
	long ln = 0L;
	int r, s, e;
	char word[BUFSIZ], *st;

	if (sp != NULL) {
		uS.extractString(templineC4, sp, "@Ses of ", ':');
		if (check_matchSpeaker(headsp, templineC4, FALSE) == NULL) {
			check_CodeErr(18);
		}
	}
	s = offset;
	e = s;
	while (1) {
		st = word;
		while ((*st=line[s++]) == ' ' || *st == '\t' || *st == ',' || *st == '\n') {
			if (*st == '\n') ln += 1;
		}
		if (*st == eChar)
			break;
		e = s;
		while ((*++st=line[e]) != eChar) {
			e++;
			if (*st == '\n' || *st == ',' || *st == ' ' || *st == '\t')
				break;
		}
		*st = EOS;
		if ((r=check_matchplate(ts->tplate, word, 's'))) {
			if (eChar == EOS)
				offset = e - 2;
			else
				offset = e - 1;
			check_err(r,s-1,offset,ln+lineno);
		}
		s = e;
	}
}

static void check_Birth(CHECK_TIERS *ts, char *sp, char *line) {
	long ln = 0L;
	int r, s, e;
	char word[BUFSIZ], *st;

	if (sp != NULL) {
		uS.extractString(templineC4, sp, "@Birth of ", ':');
		if (check_matchSpeaker(headsp, templineC4, FALSE) == NULL) {
			check_CodeErr(18);
		}
	}
	s = 0;
	e = s;
	while (1) {
		st = word;
		while ((*st=line[s++]) == ' ' || *st == '\t' || *st == '\n') {
			if (*st == '\n') ln += 1;
		}
		if (*st == EOS)
			break;
		e = s;
		while ((*++st=line[e]) != EOS) {
			e++;
			if (*st == '\n' || *st == ' ' || *st == '\t')
				break;
		}
		*st = EOS;
/*
		if (!nomap)
			uS.uppercasestr(word, &dFnt, MBF);
*/
		if ((r=check_matchplate(ts->tplate, word, 'D'))) {
			check_err(r,s-1,e - 2,ln+lineno);
		}
		s = e;
	}
}

static void check_Educ(CHECK_TIERS *ts, char *sp, char *line) {
	if (sp != NULL) {
		uS.extractString(templineC4, sp, "@Education of ", ':');
		if (check_matchSpeaker(headsp, templineC4, FALSE) == NULL) {
			check_CodeErr(18);
		}
	}
}

static void check_Group(CHECK_TIERS *ts, char *sp, char *line) {
	if (sp != NULL) {
		uS.extractString(templineC4, sp, "@Group of ", ':');
		if (check_matchSpeaker(headsp, templineC4, FALSE) == NULL) {
			check_CodeErr(18);
		}
	}
}

static int check_badrole(char *s, const char *c) {
	CHECK_TIERS *ts;

	strcpy(templineC,s);
//	uS.uppercasestr(templineC, &dFnt, MBF);
	if ((ts=check_FindTier(headt,c)) != NULL) {
		if (check_matchplate(ts->tplate,templineC, 'D') == FALSE)
			return(FALSE);
	} else
		check_CodeErr(17);
	return(TRUE);
}

static void check_addIDdsp(char *s) {
	struct IDsp *tp;

	if (check_idsp == NULL) {
		if ((check_idsp=NEW(struct IDsp)) == NULL) check_overflow();
		tp = check_idsp;
	} else {
		for (tp=check_idsp; 1; tp=tp->nextsp) {
			if (!uS.mStricmp(tp->sp,templineC)) {
			}
			if (tp->nextsp == NULL) {
				if ((tp->nextsp=NEW(struct IDsp)) == NULL)
					check_overflow();
				tp = tp->nextsp;
				break;
			}
		}
	}
	tp->nextsp = NULL;
	strcpy(templineC,s);
	tp->sp = check_mkstring(templineC);
}

static void check_addsp(char *s, char *role, char *line, long ln, char t) {
	SPLIST *tp;

	if (headsp == NULL) {
		if ((headsp=NEW(SPLIST)) == NULL) check_overflow();
		tp = headsp;
	} else {
		strcpy(templineC,s);
		for (tp=headsp; 1; tp=tp->nextsp) {
			if (!strcmp(tp->sp,templineC)) {
				if (role == NULL) {
					*line = t;
					check_trans_err(13,s,line-1,ln);
				} else if (tp->role == NULL) {
					strcpy(templineC,role);
					tp->role = check_mkstring(templineC);
				}
				return;
			}
			if (tp->nextsp == NULL) {
				if ((tp->nextsp=NEW(SPLIST)) == NULL)
					check_overflow();
				tp = tp->nextsp;
				break;
			}
		}
	}
	tp->nextsp = NULL;
	strcpy(templineC,s);
	tp->sp = check_mkstring(templineC);
	if (role == NULL)
		tp->role = NULL;
	else {
		strcpy(templineC,role);
		tp->role = check_mkstring(templineC);
	}
	tp->isUsed = FALSE;
	tp->endTime = 0L;
	tp->hasID = FALSE;
}

static void check_getpart(char *line) {
	int i;
	char sp[SPEAKERLEN];
	char *s, *e, t, wc, tchFound, isCommaFound;
	long ln = 0L;
	short cnt = 0;

	for (; *line && (*line == ' ' || *line == '\t'); line++) ;
	s = line;
	tchFound = FALSE;
	isCommaFound = FALSE;
	sp[0] = EOS;
	while (*s) {
		if (*line == ','  || *line == ' '  ||
			*line == '\t' || *line == '\n' || *line == EOS) {

			if (*line == ',')
				isCommaFound = TRUE;
			if (*line == ',' && !isSpace(*(line+1)) && *(line+1) != '\n')
				check_trans_err(92,line,line,ln);

			wc = ' ';
			e = line;
			for (; *line == ' ' || *line == '\t' || *line == '\n'; line++) {
				if (*line == '\n') ln += 1L;
			}
			if (*line != ',' && *line != EOS)
				line--;
			else
				wc = ',';
			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == ',') {
					if (NAME_ROLE_COUNT(check_GenOpt)) {
						if (!strcmp(sp, "CHI") && !strcmp(s, "Target_Child"))
							tchFound = TRUE;
					}
					if (check_badrole(s,PARTICIPANTS)) {
						*e = t;
						check_trans_err(15,s,e-1,ln);
					}
					check_addsp(sp,s,line,ln,t);
				} else if (cnt == 0) {
					for (i=0; s[i] > 32 && s[i] < 127 && s[i] != EOS; i++) ;
					if (s[i] != EOS) {
						check_trans_err(16,line-strlen(s),line-1,ln);
					} else {
						isCommaFound = FALSE;
						strcpy(sp, s);
						check_addsp(s,NULL,line,ln,t);
					}
				}
				*e = t;
				if (wc == ',') {
					if (cnt == 0)
						check_trans_err(12,line,line,ln);
					cnt = -1;
					sp[0] = EOS;
				}
				for (line++; *line==' ' || *line=='\t' || *line=='\n' || *line==','; line++) {
					if (*line == ',') {
						if (cnt == 0)
							check_trans_err(12,line,line,ln);
						cnt = -1;
						sp[0] = EOS;
					} else if (*line == '\n')
						ln += 1L;
				}
			} else {
				for (line=e; *line; line++) {
					if (*line == '\n')
						ln -= 1L;
				}
				if (cnt == 0)
					check_trans_err(12,e,e,ln);
				else {
					t = *e;
					*e = EOS;
					if (NAME_ROLE_COUNT(check_GenOpt)) {
						if (!strcmp(sp, "CHI") && !strcmp(s, "Target_Child"))
							tchFound = TRUE;
					}
					if (check_badrole(s,PARTICIPANTS))
						check_trans_err(15,s,e-1,ln);
					check_addsp(sp,s,line,ln,t);
					*e = t;
				}
				for (line=e; *line; line++) {
					if (*line == '\n')
						ln += 1L;
				}
			}
			if (cnt == 2) {
				cnt = 0;
				sp[0] = EOS;
			} else
				cnt++;
			s = line;
		} else
			line++;
	}
	if (isCommaFound) {
		check_err(100,-1,-1,ln);
	}
	if (NAME_ROLE_COUNT(check_GenOpt) && !tchFound && check_applyG2Option) {
		check_err(68,-1,-1,ln);
	}
/*
SPEAKERS *tp;
printf("partic=");
for (tp=headsp; tp != NULL; tp = tp->nextone)
printf("%s ", tp->pattern);
putchar('\n');
*/
}

static void check_isLangMatch(char *langs, long ln, int s, int wh, char isOneTime) {
	int i, j, langCnt;
	char t;

	i = 0;
	langCnt = 0;
	while (langs[i]) {
		for (; uS.isskip(langs,i,&dFnt,TRUE) || langs[i] == '\n'; i++) ;
		if (langs[i] == EOS)
			break;
		for (j=i; !uS.isskip(langs,j,&dFnt,TRUE) && langs[j] != '=' && langs[j] != '\n' && langs[j] != EOS; j++) ;
		t = langs[j];
		langs[j] = EOS;
		for (langCnt=0; langCnt < NUMLANGUAGESINTABLE; langCnt++) {
			if (strcmp(check_LanguagesTable[langCnt], langs+i) == 0)
				break;
		}
		langs[j] = t;
		if (langCnt >= NUMLANGUAGESINTABLE) {
			check_err(wh,s+i,s+j-1,ln+lineno);
		}
		if (isOneTime)
			break;
		if (langs[j] == '=')
			for (; !uS.isskip(langs,j,&dFnt,TRUE) && langs[j] != '\n' && langs[j] != EOS; j++) ;
		i = j;
	}
}

static char check_OverAll(void) {
	long linenum = 1L, tlinenum = 0L;
	char er = FALSE, UTF8SymCnt;
	char beg = FALSE, end = FALSE, font = FALSE, idt = FALSE;
	char  lSP[SPEAKERLEN+1];
	int  i = 1, cnt, j;

	UTF8SymCnt = 0;
	lSP[0] = EOS;
	do {
		*uttline = (char)getc_cr(fpin, NULL);
		if (*uttline == '\n')
			linenum += 1L;
		if (*uttline == (char)0xef)
			UTF8SymCnt++;
		if (*uttline == (char)0xbb)
			UTF8SymCnt++;
		if (*uttline == (char)0xbf)
			UTF8SymCnt++;
	} while (*uttline == ' ' || *uttline == '\t' || *uttline == '\n' || 
			 *uttline == (char)0xef || *uttline == (char)0xbb || *uttline == (char)0xbf) ;
	if (!feof(fpin) && !isSpeaker(*uttline)) {
		check_err(1,0,0,linenum);
		er = TRUE;
	}
	lineno = 1L;
	cnt = 0;
	while (!feof(fpin)) {
		uttline[i] = (char)getc_cr(fpin, NULL);
		if (uttline[i] == '\n' || feof(fpin)) {
			uttline[i] = EOS;
			if (uS.partcmp(uttline, FONTHEADER, FALSE, TRUE)) {
				if (font >= 0)
					font++;
				i = 0;
				continue;
			}
			if (uS.isUTF8(uttline)) {
				i = 0;
				if (font == 0)
					font++;
				continue;
			}
			if (uS.isInvisibleHeader(uttline)) {
				i = 0;
				continue;
			}
			lineno++;
			if (uS.partcmp(uttline, "@Options:",FALSE,FALSE) || uS.partcmp(uttline, "@Languages:",FALSE,FALSE)) {
				for (j=0; uttline[j] != EOS; j++) {
					if (!strncmp(uttline+j, "CA", 2) || !strncmp(uttline+j, "CA-Unicode", 10)) {
						check_isCAFound = TRUE;
					} else if (!strncmp(uttline+j, "notarget", 8)) {
						check_applyG2Option = FALSE;
					}
				}
			}
			if (end && isSpeaker(*uttline)) {
				check_err(44,-1,-1,linenum);
				er = TRUE;
				end = -1;
				break;
			}
			if (!stout) {
				if (linenum % 250 == 0) {
					if (totalFilesNum <= MINFILESNUM || stin)
						fprintf(stderr, "\r%ld ", linenum);
				}
			}
			er = check_CheckFirstCh(linenum,er);
			er = check_CheckErr(linenum,er);
			er = check_Tabs(linenum,er);
			er = check_Colon(linenum,er);
			i = 0;
			linenum += 1L;
			if (*uttline == '@') {
				strcpy(templineC,uttline);
				uS.remblanks(templineC);
//				uS.lowercasestr(templineC, &dFnt, MBF);
				if (!strcmp(templineC,"@Begin")) {
					if (beg >= 0)
						beg++;
					if (beg > 1) {
						if (check_err(53,-1,-1,linenum-1))
							er = TRUE;
					}
				} else if (!strcmp(templineC,"@End")) {
					if (end >= 0)
						end++;
					if (end > 1) {
						if (check_err(54,-1,-1,linenum-1))
							er = TRUE;
					}
				} else if (templineC[0] == '@' && templineC[1] == 'I' &&
						   templineC[2] == 'D' && templineC[3] == ':') {
					idt = TRUE;
				}
			}
			if (*uttline == EOS) ;
			else {
				if (linenum <= 1)
					tlinenum = linenum;
				else
					tlinenum = linenum - 1;
				if (lHeaders[cnt][0] != EOS && !uS.partcmp(templineC, lHeaders[cnt], FALSE, TRUE)) {
					if (cnt == 0) {
						if (check_err(43,-1,-1,tlinenum))
							er = TRUE;
						beg = -1;
					} else if (cnt == 1) {
						if (check_err(77,-1,-1,tlinenum))
							er = TRUE;
					} else if (cnt == 2) {
						if (check_err(61,-1,-1,tlinenum))
							er = TRUE;
					}
				}
				if (lHeaders[cnt][0] != EOS)
					cnt++;
			}
			if (uS.partcmp(uttline, "@Options", FALSE, FALSE)) {
				if (!uS.partcmp(lSP, "@Participants", FALSE, FALSE)) {
					if (check_err(125,0,strlen(uttline),tlinenum))
						er = TRUE;
				}
			} else if (uS.partcmp(uttline, "@ID:", FALSE, FALSE)) {
				if (!uS.partcmp(lSP,"@Participants",FALSE,FALSE) && !uS.partcmp(lSP,"@Options",FALSE,FALSE) &&
					!uS.partcmp(lSP,"@ID:",FALSE,FALSE)) {
					if (check_err(126,0,strlen(uttline),tlinenum))
						er = TRUE;
				}
			} else if (uS.partcmp(uttline,"@Birth of",FALSE,FALSE) || uS.partcmp(uttline,"@L1 of",FALSE,FALSE) ||
					   uS.partcmp(uttline,"@Birthplace of",FALSE,FALSE)) {
				if (!uS.partcmp(lSP,"@ID:",FALSE,FALSE) && !uS.partcmp(lSP,"@Birth of",FALSE,FALSE) &&
					!uS.partcmp(lSP,"@L1 of",FALSE,FALSE) && !uS.partcmp(lSP,"@Birthplace of",FALSE,FALSE)) {
					if (check_err(127,0,strlen(uttline),tlinenum))
						er = TRUE;
				}
			} 
			if (isSpeaker(uttline[0])) {
				strncpy(lSP, uttline, SPEAKERLEN);
				lSP[SPEAKERLEN] = EOS;
			}
		} else if (i >= UTTLINELEN) {
			uttline[i-1] = EOS;
			fprintf(stderr,"ERROR. Speaker turn is longer then ");
			fprintf(stderr,"%ld characters.\n",UTTLINELEN);
			for (i=0; uttline[i]; i++) putc(uttline[i],stderr);
			putc('\n',stderr);
			cleanupLanguages();
			cutt_exit(1);
		} else
			i++;
	}
	lineno = 0L;
	*uttline = EOS;
	if (!beg) {
		if (check_err(6,-1,-1,-1L))
			er = TRUE;
	}
	if (!idt && IS_CHECK_ID(check_GenOpt)) {
		if (check_err(60,-1,-1,-1L))
			er = TRUE;
	}
	if (!end) {
		if (check_err(7,-1,-1,-1L))
			er = TRUE;
	}
	if (!font) {
		if (check_err(69,-1,-1,-1L))
			er = TRUE;
	}
#if defined(UNX)
	if (!stin)
#endif
		rewind(fpin);
	if (!er) {
		linenum = 1L;
		do {
			*uttline = (char)getc_cr(fpin, NULL);
			if (*uttline == '\n')
				linenum += 1;
		} while (*uttline == ' ' || *uttline == '\t' || *uttline == '\n' || 
				 *uttline == (char)0xef || *uttline == (char)0xbb || *uttline == (char)0xbf) ;
		i = 1;
		beg = TRUE;
		end = TRUE;
		while (!feof(fpin)) {
			*uttline = (char)getc_cr(fpin, NULL);
			if (isSpeaker(*uttline)) {
				if (beg) { i = 0; end = TRUE; }
			} else if (*uttline == ':') {
				if (end) i = 0;
				end = FALSE;
			}
			if (*uttline == '\n') {
				if (!stout) {
					if (linenum % 250 == 0) {
						if (totalFilesNum <= MINFILESNUM || stin)
							fprintf(stderr, "\r%ld ", linenum);
					}
				}
				linenum += 1L;
				beg = TRUE;
			} else
				beg = FALSE;
			i++;
			if (end) {
				if (i >= SPEAKERLEN) {
					er = TRUE;
					*uttline = EOS;
					check_err(9,-1,-1,linenum);
					while (*uttline != ':')
						*uttline = (char)getc_cr(fpin, NULL);
					i = 0; end = FALSE;
				}
			} else if (i >= UTTLINELEN) {
				er = TRUE;
				*uttline = EOS;
				check_err(10,-1,-1,linenum);
				do {
					while (*uttline != '\n')
						*uttline = (char)getc_cr(fpin, NULL);
					linenum += 1L;
					*uttline = (char)getc_cr(fpin, NULL);
					if (feof(fpin) || isSpeaker(*uttline))
						break;
				} while (1) ;
				i = 0; end = TRUE;
			}
		}
#if defined(UNX)
		if (!stin)
#endif
			rewind(fpin);
	}
	if (UTF8SymCnt == 3) {
		getc(fpin); getc(fpin); getc(fpin);
	}
	return(!er);
}

static void check_getExceptions(char *line) {
	char *s, *e, t;
	TPLATES *tp = check_WordExceptions;

	for (; *line && isSpace(*line); line++) ;
	s = line;
	while (*s) {
		if (isSpace(*line) || *line == '\n' || *line == EOS) {
			e = line;
			for (; *line == ' ' || *line == '\t' || *line == '\n'; line++) ;
			t = *e;
			*e = EOS;
			if (tp == NULL) {
				check_WordExceptions = NEW(TPLATES);
				tp = check_WordExceptions;
			} else {
				tp->nextone = NEW(TPLATES);
				tp = tp->nextone;
			}
			if (tp != NULL) {
				tp->nextone = NULL;
				tp->pattern = check_mkstring(s);
			} else
				return;
			*e = t;
			s = line;
		} else
			line++;
	}
}

static char check_isOneLetter(char *word, char *bw, int *offset) {
	int num = 0;

	for (; word != bw && *word != EOS && *word != '@' && *word != '-'; word++) {
		if (word[0] == (char)0xE2 && word[1] == (char)0x86 && word[2] == (char)0xAB) {
			word += 3;
			while (*word != EOS) {
				if (word[0] == (char)0xE2 && word[1] == (char)0x86 && word[2] == (char)0xAB) {
					word += 3;
					break;
				} else
					word++;
			}
			if (*word == EOS)
				break;
		}
		if (*word != ':') {
			if (UTF8_IS_LEAD((unsigned char)*word) || UTF8_IS_SINGLE((unsigned char)*word))
				num++;
		}
	}
	if (num > 1) {
		*offset = 1;
		for (word--, num=0; num < 1; word--) {
			if (*word != ':') {
				if (UTF8_IS_LEAD((unsigned char)*word) || UTF8_IS_SINGLE((unsigned char)*word))
					num++;
			}
			*offset = *offset + 1;
		}
		return(FALSE);
	} else
		return(TRUE);
}
				/* sufix or prefix */
static char check_CheckFixes(CHECK_TIERS *ts, char *word, long ln, int s, int e) {
	int s1, offset, pos;
	char *bw, t, found, isPrefix, isCompFound;
	TPLATES *tp;

	isCompFound = FALSE;
	found = FALSE;
	s1 = s;
	bw = word;
	for (pos=0; word[pos]; pos++, s++) {
		if (uS.isRightChar(word, pos,'+',&dFnt,MBF))
			isCompFound = TRUE;

		if (word[pos] == '~') {
		} else if (uS.ismorfchar(word, pos, &dFnt, CHECK_MORPHS, MBF) || uS.isRightChar(word, pos, '@', &dFnt, MBF)) {
			if (!isItemPrefix[(int)word[pos]]) {
				isPrefix = FALSE;
				bw = word + pos;
				s1 = s;
				for (pos++, s++; word[pos] && !uS.isRightChar(word, pos, '@', &dFnt, MBF) &&
											!uS.ismorfchar(word, pos, &dFnt, CHECK_MORPHS, MBF) &&
											!uS.isRightChar(word, pos, '$', &dFnt, MBF); pos++, s++) ;
				pos--;
				s--;
			} else
				isPrefix = TRUE;
			t = word[pos+1];
			word[pos+1] = EOS;
			if (!strcmp(bw, "@l") && !check_isOneLetter(word, bw, &offset)) {
				if (s1-offset > 0 && !UTF8_IS_SINGLE((unsigned char)uttline[s1-offset]) && !UTF8_IS_LEAD((unsigned char)uttline[s1-offset])) {
					offset++;
				}
				check_err(76,s1-offset,s1-2,ln);
			}
			if (bw[0] == '@' && bw[1] == 's' && bw[2] == ':') {
				if (bw[3] == EOS)
					check_err(62,s1-1,s-1,ln);
			}
			for (tp=ts->tplate; tp != NULL; tp = tp->nextone) {
				if (isCompFound) {
					if (*tp->pattern == '+' && *(tp->pattern+1) != EOS) {
						if (uS.patmat(bw,tp->pattern+1))
							break;
					}
				} else {
					if (*tp->pattern == '*' && *(tp->pattern+1) != EOS) {
						if (uS.patmat(bw,tp->pattern+1))
							break;
					}
				}
			}
			word[pos+1] = t;
			if (tp == NULL) {
				if (word[pos] == '#' && check_isHebrew == TRUE) {
					found = TRUE;
				} else if (isPrefix) {
					check_err(37,s1-1,s-1,ln);
				} else if (uttline[s1-1] == '@')
					check_err(147,s1-1,s-1,ln);
				else
					check_err(20,s1-1,s-1,ln);
			} else
				found = TRUE;
			bw = word + pos + 1;
			s1 = s + 1;
		}
	}
	return(found);
}

// | & # - = + ~ $ ^
static char check_MorItem(char *item, int *sErr, int *eErr, int offset) {
	return(0);

/*
	char *c, *btc, *etc, t, *last, *org;

	org = item;
	if ((c=strchr(item,'^')) != NULL) {
		*c = EOS;
		getMorItem(item, lang, groupIndent, st);
		getMorItem(c+1, lang, groupIndent, st);
	} else {
		changeGroupIndent(groupIndent, 1);
		last = NULL;
		while (*item != EOS) {
			for (c=item; *c != '|' && *c != '#' &&  *c != '-' && 
						*c != '=' && *c != '~' *c != '$' && *c != EOS; c++) {
				if (!strncmp(c, "&amp;", 5))
					break;
			}
			t = *c;
			*c = EOS;
			if (last == NULL) {
				if (t != '|') {
					*c = t;
					sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(15);
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("<pos>\n", NULL);
				} else {
					strcat(st, groupIndent);
					strcat(st, "<pos>\n");
				}
				if ((etc=strchr(item, ':')) == NULL) {
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("    <c>", NULL);
						xml_output(NULL, item);
						xml_output("</c>\n", NULL);
					} else {
						strcat(st, groupIndent);
						strcat(st, "    <c>");
						strcat(st, item);
						strcat(st, "</c>\n");
					}
				} else {
					*etc = EOS;
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("    <c>", NULL);
						xml_output(NULL, item);
						xml_output("</c>\n", NULL);
					} else {
						strcat(st, groupIndent);
						strcat(st, "    <c>");
						strcat(st, item);
						strcat(st, "</c>\n");
					}
					*etc = ':';
					btc = etc + 1;
					while ((etc=strchr(btc, ':')) != NULL) {
						*etc = EOS;
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("    <s>", NULL);
							xml_output(NULL, btc);
							xml_output("</s>\n", NULL);
						} else {
							strcat(st, groupIndent);
							strcat(st, "    <s>");
							strcat(st, btc);
							strcat(st, "</s>\n");
						}
						*etc = ':';
						btc = etc + 1;
					}
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("    <s>", NULL);
						xml_output(NULL, btc);
						xml_output("</s>\n", NULL);
					} else {
						strcat(st, groupIndent);
						strcat(st, "    <s>");
						strcat(st, btc);
						strcat(st, "</s>\n");
					}
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("</pos>\n", NULL);
				} else {
					strcat(st, groupIndent);
					strcat(st, "</pos>\n");
				}
			} else if (*last == '|') {
				if ((etc=strchr(item, '+')) == NULL) {
					if (t == '|') {
						*c = t;
						sprintf(tempst2, ": %ls\nItem=%ls", c, org);
						freeAll(16);
					}
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("<stem>", NULL);
						xml_output(NULL, item);
						xml_output("</stem>\n", NULL);
					} else {
						strcat(st, groupIndent);
						strcat(st, "<stem>");
						strcat(st, item);
						strcat(st, "</stem>\n");
					}
				} else {
					if (t == '|') {
						*etc = EOS;
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<stem>", NULL);
							xml_output(NULL, item);
							xml_output("</stem>\n", NULL);
						} else {
							strcat(st, groupIndent);
							strcat(st, "<stem>");
							strcat(st, item);
							strcat(st, "</stem>\n");
						}
						*etc = '+';
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<wk type=\"cmp\"/>\n", NULL);
						} else {
							strcat(st, groupIndent);
							strcat(st, "<wk type=\"cmp\"/>\n");
						}
						last = NULL;
						*c = t;
						item = etc + 1;
						continue;
					} else {
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<stem>", NULL);
							xml_output(NULL, item);
							xml_output("</stem>\n", NULL);
						} else {
							strcat(st, groupIndent);
							strcat(st, "<stem>");
							strcat(st, item);
							strcat(st, "</stem>\n");
						}
					}
				}
			} else if (!strncmp(last, "&amp;", 5)) {
				if (t == '|') {
					*c = t;
					sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(17);
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("<mk type=\"suffix fusion\">", NULL);
					xml_output(NULL, item+4);
					xml_output("</mk>\n", NULL);
				} else {
					strcat(st, groupIndent);
					strcat(st, "<mk type=\"suffix fusion\">");
					strcat(st, item+4);
					strcat(st, "</mk>\n");
				}
			} else if (*last == '#') {
				if (t == '|') {
					*c = t;
					sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(18);
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("<mk type=\"prefix\">", NULL);
					xml_output(NULL, item);
					xml_output("</mk>\n", NULL);
				} else {
					strcat(st, groupIndent);
					strcat(st, "<mk type=\"prefix\">");
					strcat(st, item);
					strcat(st, "</mk>\n");
				}
			} else if (*last == '-') {
				if (t == '|') {
					*c = t;
					sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(19);
				}
				if (*item == '0') {
					if (item[1] == '*') {
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<mk type=\"incorrectly_omitted_affix\">", NULL);
							xml_output(NULL, item+2);
							xml_output("</mk>\n", NULL);
						} else {
							strcat(st, groupIndent);
							strcat(st, "<mk type=\"incorrectly_omitted_affix\">");
							strcat(st, item+2);
							strcat(st, "</mk>\n");
						}
					} else {
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<mk type=\"omitted_affix\">", NULL);
							xml_output(NULL, item+1);
							xml_output("</mk>\n", NULL);
						} else {
							strcat(st, groupIndent);
							strcat(st, "<mk type=\"omitted_affix\">");
							strcat(st, item+1);
							strcat(st, "</mk>\n");
						}
					}
				} else {
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("<mk type=\"suffix\">", NULL);
						xml_output(NULL, item);
						xml_output("</mk>\n", NULL);
					} else {
						strcat(st, groupIndent);
						strcat(st, "<mk type=\"suffix\">");
						strcat(st, item);
						strcat(st, "</mk>\n");
					}
				}
			} else if (*last == '=') {
				*c = t;
				if (t == '|') {
					sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(20);
				}
				sprintf(tempst2, "\nItem=%ls", org);
				freeAll(900);
			} else if (*last == '~' || *last == '$') {
				*c = t;
				if (t != '|') {
					sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(21);
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("<wk type=\"cli\"/>\n", NULL);
				} else {
					strcat(st, groupIndent);
					strcat(st, "<wk type=\"cli\"/>\n");
				}
				last = NULL;
				continue;
			}
			last = c;
			*c = t;
			if (t != EOS)
				item = c + 1;
			else
				break;
		}
		changeGroupIndent(groupIndent, -1);
		if (st == NULL) {
			xml_output(NULL, groupIndent);
			xml_output("</item>\n", NULL);
		} else {
			strcat(st, groupIndent);
			strcat(st, "</item>\n");
			priList = addToPriList(priList, 3, st, FALSE, FALSE);
		}
	}
*/
}

static void check_CheckBrakets(char *org, long pos, int *sb, int *pb, int *ab, int *cb, int *anb) {
	if (uS.isRightChar(org, pos, '(', &dFnt, MBF)) (*pb)++;
	else if (uS.isRightChar(org, pos, ')', &dFnt, MBF)) (*pb)--;
	else if (uS.isRightChar(org, pos, '[', &dFnt, MBF)) (*sb)++;
	else if (uS.isRightChar(org, pos, ']', &dFnt, MBF)) (*sb)--;
	else if (uS.isRightChar(org, pos, '{', &dFnt, MBF)) (*cb)++;
	else if (uS.isRightChar(org, pos, '}', &dFnt, MBF)) (*cb)--;
	else if (uS.isRightChar(org, pos, '<', &dFnt, MBF)) {
		if (!uS.isPlusMinusWord(org, pos))
			(*ab)++;
	} else if (uS.isRightChar(org, pos, '>', &dFnt, MBF)) {
		if (!uS.isPlusMinusWord(org, pos)) {
			(*ab)--;
			(*anb) = TRUE;
		}
	}
}

static char isIllegalASCII(char *w, int s) {
	if ((unsigned char)w[s] == 0xc2 && my_CharacterByteType(w,(short)s,&dFnt) == -1 && (unsigned char)w[s+1] == 0xb7)
		return(FALSE);
	else if ((unsigned char)w[s] >= 0xee && my_CharacterByteType(w,(short)s,&dFnt) == -1 && my_CharacterByteType(w,(short)s+1,&dFnt) > 0 && my_CharacterByteType(w,(short)s+2,&dFnt) > 0) {
		long num, t;

		num = (unsigned char)w[s];
		num = num << 16;
		t = (unsigned char)w[s+1];
		t = t << 8;
		num = num | t;
		t = (unsigned char)w[s+2];
		num = num | t;

		if ((num >= 0xef85b0 /*15697328L*/ && num <= 0xef89a4 /*15698340L*/) || 
			(num >= 0xefbc81 /* 15711361L */ && num <= 0xefbd9e /* 15711646 */))
			return(FALSE);
		else
			return(TRUE);
	}
	return(FALSE);
}

static int isBadQuotes(char *w, int s) {
	if ((unsigned char)w[s] == 0xe2 && my_CharacterByteType(w,(short)s,&dFnt) == -1 && (unsigned char)w[s+1] == 0x80) {
		if ((unsigned char)w[s+2] == 0x98)
			return(139);
		else if ((unsigned char)w[s+2] == 0x99)
			return(138);
	}
	return(0);
}

static char isLegalUseOfDigit(char *w, int s) {
	if (uS.isRightChar(w,s-1,'-',&dFnt,MBF) ||
		 (s >= 3 && (unsigned char)w[s-3] == 0xe2 && (unsigned char)w[s-2] == 0x8c && 
			((unsigned char)w[s-1] == 0x88 || (unsigned char)w[s-1] == 0x89 || 
			 (unsigned char)w[s-1] == 0x8a || (unsigned char)w[s-1] == 0x8b))
		)
		return(TRUE);
	else
		return(FALSE);
}
/* 2009-05-01
static char check_isCAchar(char *w, int pos) {
	int s, matchType;

	if (dFnt.isUTF) {
		s = uS.HandleCAChars(w+pos, &matchType);
		if (matchType == CA_APPLY_CLTOPSQ || matchType == CA_APPLY_CLBOTSQ) {
			if (w[pos+s] == EOS)
				return(TRUE);
		}
	}
	return(FALSE);
}
*/
static char check_isOnlyCAs(char *w, int pos) {
	int s;

	while (w[pos] != EOS) {
		s = uS.HandleCAChars(w+pos, NULL);
		if (s == 0)
			return(FALSE);
		else
			pos = pos + s;
	}
	return(TRUE);
}

static char check_isEndOfWord(char *w) {
	int i;

	for (i=strlen(w)-1; i >= 0; i--) {
		if (isalnum(w[i]))
			return(FALSE);
	}
	return(TRUE);
}

static void check_isThereStem(char *w, int pos, long ln) {
	int i, s, matchType;
	char isF;

	isF = FALSE;
	i = 0;
	while (w[i] != EOS) {
		s = uS.HandleCAChars(w+i, &matchType);
		if (s) {
			if (matchType == NOTCA_LEFT_ARROW_CIRCLE)
				isF = !isF;
			i = i + s;
		} else
			i++;
		if (!isF && w[i] != EOS)
			return;
	}
	check_err(151, pos-1, pos+i-2, ln);
}

static void check_CheckWords(char *w, int pos, long ln, CHECK_TIERS *ts) {
//	char capWErrFound, capWFound, isCompFound;
	char digFound, paransFound;
//	int CAOffset;
	int s, t, len, CAo, isATFound;

	len = strlen(w) - 1;
	if (ts->code[0] == '*' || uS.partcmp(ts->code, "%wor", FALSE, TRUE) || uS.partcmp(ts->code, "%xwor", FALSE, TRUE)) {
#ifndef UNX
		int matchType;

//		if (dFnt.isUTF) {
			s = uS.HandleCAChars(w, &matchType);
			if (s) {
				if (matchType == NOTCA_MISSINGWORD) {
					check_err(48, pos-1, pos+s-2, ln);
				} else if (matchType == CA_APPLY_CLTOPSQ || matchType == CA_APPLY_CLBOTSQ ||
					matchType == NOTCA_GROUP_END || matchType == NOTCA_SIGN_GROUP_END) {
					if (!check_isOnlyCAs(w, s))
						check_err(92, pos-1, pos+s-2, ln);
				} else if (matchType == CA_APPLY_SHIFT_TO_HIGH_PITCH || matchType == CA_APPLY_SHIFT_TO_LOW_PITCH || matchType == CA_APPLY_INHALATION ||
						   matchType == CA_APPLY_FASTER || matchType == CA_APPLY_SLOWER || matchType == CA_APPLY_CREAKY ||
						   matchType == CA_APPLY_UNSURE || matchType == CA_APPLY_SOFTER || matchType == CA_APPLY_LOUDER ||
						   matchType == CA_APPLY_LOW_PITCH || matchType == CA_APPLY_HIGH_PITCH || matchType == CA_APPLY_SMILE_VOICE ||
						   matchType == CA_APPLY_WHISPER || matchType == CA_APPLY_YAWN || matchType == CA_APPLY_SINGING || 
						   matchType == CA_APPLY_PRECISE || matchType == CA_APPLY_CONSTRICTION || matchType == CA_APPLY_PITCH_RESET || 
						   matchType == CA_APPLY_LAUGHINWORD || matchType == NOTCA_ARABIC_DOT_DIACRITIC || matchType == NOTCA_ARABIC_RAISED ||
						   matchType == NOTCA_STRESS || matchType == NOTCA_GLOTTAL_STOP || matchType == NOTCA_HEBREW_GLOTTAL ||
						   matchType == NOTCA_CARON || matchType == NOTCA_CROSSED_EQUAL || matchType == NOTCA_LEFT_ARROW_CIRCLE) {
					if (matchType == NOTCA_LEFT_ARROW_CIRCLE) {
						check_isThereStem(w, pos, ln);
					}
					if (w[s] == EOS || uS.isskip(w, s, &dFnt, MBF)) {
						check_err(101, pos-1, pos+s-2, ln);
					} else if (uS.HandleCAChars(w+s, &matchType)) {
						if (matchType == CA_APPLY_OPTOPSQ || matchType == CA_APPLY_OPBOTSQ) {
							check_err(65, pos-1, pos+s-2, ln);
						}
					}
				} else if (matchType == NOTCA_DOUBLE_COMMA || matchType == NOTCA_VOCATIVE) {
					if (w[s] != EOS) {
						check_err(92, pos-1, pos+s-2, ln);
					}
				} else if (matchType == CA_HURRIED_START) {
					if (w[s] == EOS || uS.isskip(w,s,&dFnt,TRUE))
						check_err(67, pos-1, pos+s-2, ln);
				} else if (matchType == CA_SUDDEN_STOP) {
					if (!check_isOnlyCAs(w, s)) {
						check_err(92, pos-1, pos+s-2, ln);
					} else if (s == 3) {
						check_err(73, pos-1, pos+s-2, ln);
					}
				}  else if (matchType == CA_HARDENING_OVERDOT) {
					check_err(73, pos-1, pos+s-2, ln);
				} else if (matchType == CA_APPLY_OPTOPSQ || matchType == CA_APPLY_OPBOTSQ ||
						   matchType == NOTCA_GROUP_START || matchType == NOTCA_SIGN_GROUP_START ||
						   matchType == NOTCA_OPEN_QUOTE)
					;
				else
					s = 0;
			}
//		} else
//			s = 0;
#else
		s = 0;
#endif
//		CAOffset = s;
		if (w[s] == HIDEN_C) ;
		else if (uS.isRightChar(w,s,'[',&dFnt,MBF) && uS.isRightChar(w,len,']',&dFnt,MBF)) ;
		else if (uS.isRightChar(w,s,'{',&dFnt,MBF) && uS.isRightChar(w,len,'}',&dFnt,MBF)) ;
		else if (uS.isRightChar(w,s,'+',&dFnt,MBF)) ;
		else if (uS.isRightChar(w,s,'-',&dFnt,MBF) && w[s+1] != EOS && !isalnum((unsigned char)w[s+1])) ;
		else if (uS.isRightChar(w,s,'(',&dFnt,MBF) && uS.isPause(w,s,NULL,&t)) {
			char isNumf, isColf, isDotf;

			isNumf = FALSE;
			isColf = FALSE;
			isDotf = 0;
			while (w[s] != EOS) {
				if (is_unCH_digit(w[s])) {
					if (isDotf > 1)
						check_err(111, pos+s-1, pos+s-1, ln);
					isNumf =  TRUE;
				}
				if (w[s] == ':') {
					if (isColf || isDotf)
						check_err(111, pos+s-1, pos+s-1, ln);
					isColf = TRUE;
				}
				if (w[s] == '.') {
					if ((isDotf >= 1 && (isColf || isNumf)) || isDotf >= 3)
						check_err(111, pos+s-1, pos+s-1, ln);
					isDotf++;
				}
				if (isSpace(w[s]))
					check_err(111, pos+s-1, pos+s-1, ln);
				s++;
			}
			if (!isDotf)
				check_err(111, pos-1, pos+strlen(w)-2, ln);
		} else if (uS.isRightChar(w,s,'&',&dFnt,MBF)) {
			s++;
			if (w[s] == EOS) {
				check_err(65, pos-1, pos-1, ln);
			}
			while (w[s] != EOS) {
				if (isalnum((unsigned char)w[s]) || w[s] == '=' || w[s] == '-' || w[s] == '_' || w[s] == '^' ||
						w[s] == '$' || w[s] == '%' || w[s] == '{' || w[s] == '}') ;
				else if (uS.isRightChar(w,s,'@',&dFnt,MBF) || uS.isRightChar(w,s,'\'',&dFnt,MBF) ||
						 uS.isRightChar(w,s,':',&dFnt,MBF) || uS.isRightChar(w,s,'&',&dFnt,MBF)  ||
						 uS.isRightChar(w,s,'+',&dFnt,MBF) || uS.isRightChar(w,s,'/',&dFnt,MBF)  ||
						 uS.isRightChar(w,s,'~',&dFnt,MBF) || uS.isRightChar(w,s,'*',&dFnt,MBF)) {
					if (w[s] == ':' && w[s+1] == ':' && (w[s+2] == '@' || w[s+2] == ':')) {
						if (uS.atUFound(w,s,&dFnt,MBF))
							break;
						check_err(48, pos+s-1, pos+s-1, ln);
						break;
					}
					if (w[s] == w[s+1] && w[s] != ':') {
						if (uS.atUFound(w,s,&dFnt,MBF))
							break;
						check_err(48, pos+s-1, pos+s-1, ln);
						break;
					}
				} else if (w[s] > 0 && w[s] < 127) {
					if (uS.atUFound(w,s,&dFnt,MBF))
						break;
					check_err(65, pos+s-2, pos+s-2, ln);
					break;
				}
				s++;
			}
		} else if (uS.isRightChar(w,s,'#',&dFnt,MBF))  {
			if (check_isHebrew == TRUE) {
			} else {
				s++;
				if (uS.isRightChar(w,s,'#',&dFnt,MBF))
					s++;
				if (uS.isRightChar(w,s,'#',&dFnt,MBF))
					s++;
				if (w[s] == '_') {
					check_err(48, pos+s-1, pos+s-1, ln);
				} 
				if (w[s] == '0' && w[s+1] != '_') {
					check_err(48, pos+s-1, pos+s-1, ln);
				} 
				while (w[s] != EOS) {
					if (!isdigit((unsigned char)w[s]) && !uS.isRightChar(w,s,':',&dFnt,MBF) && !uS.isRightChar(w,s,'_',&dFnt,MBF)) {
						check_err(48, pos+s-1, pos+s-1, ln);
						break;
					} else if (uS.isRightChar(w,s-1,'_',&dFnt,MBF) && w[s] == '0' && isdigit((unsigned char)w[s+1])) {
						check_err(48, pos+s-1, pos+s-1, ln);
						break;
					}
					s++;
				}
			}
		} else if (uS.mStricmp(w,"xx") == 0 || uS.mStricmp(w,"yy") == 0) {
			check_err(135, pos-1, pos+len-1, ln);
		} else {
			isATFound = 0;
//			capWFound = FALSE;
//			capWErrFound = FALSE;
			digFound = FALSE;
//			isCompFound = FALSE;
			paransFound = FALSE;
			if (uS.isRightChar(w,s,'0',&dFnt,MBF))
				s++;
			if (uS.isRightChar(w,s,'0',&dFnt,MBF) || uS.isRightChar(w,s,'*',&dFnt,MBF))
				s++;
			while (w[s] != EOS) {
				if (w[s] == (char)0xE2 && w[s+1] == (char)0x80 && w[s+2] == (char)0xA6) {
					check_err(48, pos+s-1, pos+s-1, ln);
				} else if (isdigit((unsigned char)w[s]) && my_CharacterByteType(w, (short)s, &dFnt) == 0) {
				   	if (isATFound && isdigit((unsigned char)w[s])) ;
					else if (!digFound && (s == 0 || !isLegalUseOfDigit(w, s))) {
						if (uS.atUFound(w,s,&dFnt,MBF)) {
							digFound = TRUE;
							check_err(47, pos+s-1, pos+s-1, ln);
						} else if (!check_isNumberShouldBeHere) {
							digFound = TRUE;
							check_err(47, pos+s-1, pos+s-1, ln);
						} else if (check_isNumberShouldBeHere) {
							if (s == 0) {
								digFound = TRUE;
								check_err(47, pos+s-1, pos+s-1, ln);
							}
						}
					}
				} else if (!isalpha((unsigned char)w[s])) {
#ifndef UNX
					t = uS.HandleCAChars(w+s, &matchType);

					if (t) {
						if (w[s+t] == EOS && (matchType == CA_APPLY_OPTOPSQ || matchType == CA_APPLY_OPBOTSQ ||
											  matchType == CA_HURRIED_START ||
											  matchType == NOTCA_GROUP_START || matchType == NOTCA_SIGN_GROUP_START)) {
							check_err(93, pos+s-1, pos+s+t-2, ln);
						}
						if (w[s+t] != EOS && (matchType == CA_APPLY_CONTINUATION || matchType == CA_APPLY_LATCHING ||
											  matchType == CA_SUDDEN_STOP ||
											  matchType == NOTCA_DOUBLE_COMMA || matchType == NOTCA_VOCATIVE)) {
							check_err(92, pos+s-1, pos+s+t-2, ln);
						}
						if (s != 0 && (matchType == NOTCA_DOUBLE_COMMA || matchType == NOTCA_VOCATIVE)) {
							check_err(93, pos+s-1, pos+s+t-2, ln);
						}
						s += (t - 1);
						if (matchType == NOTCA_LEFT_ARROW_CIRCLE && isupper(w[s+1]))
							s++;
					} else
#endif
					if ((t=isBadQuotes(w, s)) > 0) {
						check_err(t, pos+s-1, pos+s-1, ln);
						s += 3;
					} else if (isIllegalASCII(w, s)) {
						check_err(86, pos+s-1, pos+s-1, ln);
					} else if (uS.isRightChar(w,s,'/',&dFnt,MBF)) {
						if (uS.isRightChar(w,s+1,'/',&dFnt,MBF) && uS.isRightChar(w,s+2,'/',&dFnt,MBF) && 
							uS.isRightChar(w,s+3,'/',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						}
						if (check_FoundAlNums(0, w)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(48, pos+s-1, pos+s-1, ln);
						}
						if (w[s+1] == EOS) {
							check_err(48, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,':',&dFnt,MBF)) {
						if (uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'"',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(48, pos+s, pos+s, ln);
						}
					} else if (uS.isRightChar(w,s,'&',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'\'',&dFnt,MBF)|| uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'^',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'@',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'"',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						} else {
							check_err(66, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'(',&dFnt,MBF)) {
						paransFound = TRUE;
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'(',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'#',&dFnt,MBF) || uS.isRightChar(w,s+1,'-',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,')',&dFnt,MBF)) {
						paransFound = FALSE;
						if (w[s+1] != EOS && uS.isRightChar(w,s+1,')',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'`',&dFnt,MBF) && check_isArabic) {
					} else if (uS.isRightChar(w,s,'#',&dFnt,MBF)) {
						if (check_isEndOfWord(w+s+1) || check_isHebrew == TRUE) ;
						else {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(48, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'^',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || /* uS.isRightChar(w,s+1,'\'',&dFnt,MBF) || */
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'^',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'@',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'"',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'_',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'\'',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'@',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'"',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'~',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'@',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'"',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'$',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'@',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'"',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'+',&dFnt,MBF)) {
//						isCompFound = TRUE;
/*
						if (capWFound) {
							check_err(95, pos+s-1, pos+s-1, ln);
						} else
*/
						if (uS.isRightChar(w,s+1,'"',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'<',&dFnt,MBF) || uS.isRightChar(w,s+1,',',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'/',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'.',&dFnt,MBF) || uS.isRightChar(w,s+1,'!',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'?',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							if (!check_isNumberShouldBeHere || !uS.isRightChar(w,s+1,'+',&dFnt,MBF))
								check_err(66, pos+s-1, pos+s-1, ln);
							break;
						} else 
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'@',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(67, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'-',&dFnt,MBF)) {
						if (w[s+1] == EOS)
							;
						else if (uS.isRightChar(w,s+1,',',&dFnt,MBF) || uS.isRightChar(w,s+1,'\'',&dFnt,MBF) ||
								   uS.isRightChar(w,s+1,'.',&dFnt,MBF) || uS.isRightChar(w,s+1,'!',&dFnt,MBF)  ||
								   uS.isRightChar(w,s+1,'?',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(66, pos+s-1, pos+s-1, ln);
							break;
						} else if (uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
								   uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								   uS.isRightChar(w,s+1,'@',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF) ||
								   uS.isRightChar(w,s+1,'"',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							if (check_isHebrew == FALSE)
								check_err(67, pos+s-1, pos+s-1, ln);
						} else if (isATFound == 0 && w[s+1] == '@') {
//							if (!uS.isRightChar(w,s+1,'0',&dFnt,MBF)) {
//								if (uS.atUFound(w,s,&dFnt,MBF))
//									break;
								check_err(48, pos+s-1, pos+s-1, ln);
//							}
						}
					} else if (uS.isRightChar(w,s,'\'',&dFnt,MBF)) {
						if (w[s+1] != EOS && (uS.isRightChar(w,s+1,'\'',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'@',&dFnt,MBF) || uS.isRightChar(w,s+1,'"',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'_',&dFnt,MBF))) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							if (check_isHebrew == FALSE)
								check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'@',&dFnt,MBF)) {
						isATFound = s;
						if (paransFound) {
							check_err(97, pos+s-1, pos+s-1, ln);
						} else if (w[s+1] == 's' && w[s+2] == ':') {
#ifdef UNX
							for (t=s+3; isalpha(w[t]) || w[t] == '&' || w[t] == '+' || w[t] == '$' || w[t] == ':'; t++) ;
#else
//							if (dFnt.isUTF) {
								for (t=s+3; TRUE; t++) {
									if (!isalpha(w[t]) && w[t] != '&' && w[t] != '+' && w[t] != '$' && w[t] != ':') {
										if ((CAo=uS.HandleCAChars(w+t,NULL)) == 0)
											break;
										else {
											t = t + CAo - 1;
										}
									}
								}
//							} else {
//								for (t=s+3; isalpha(w[t]) || w[t] == '&' || w[t] == '+' || w[t] == '$' || w[t] == ':'; t++) ;
//							}
#endif
							if (w[t] != EOS) {
								check_err(65, pos+s-1, pos+t-2, ln);
							}
							s = t - 1;
						} else if (w[s+1] == 's') {
#ifdef UNX
							for (t=s+2; isalpha(w[t]) || uS.HandleCAChars(w+t, NULL) || w[t] == '$' || w[t] == ':'; t++) ;
#else
//							if (dFnt.isUTF) {
								for (t=s+2; TRUE; t++) {
									if (!isalpha(w[t]) && w[t] != '$' && w[t] != ':') {
										if ((CAo=uS.HandleCAChars(w+t,NULL)) == 0)
											break;
										else {
											t = t + CAo - 1;
										}
									}
								}
//							} else {
//								for (t=s+2; isalpha(w[t]) || uS.HandleCAChars(w+t, NULL) || w[t] == '$' || w[t] == ':'; t++) ;
//							}
#endif
							if (w[t] != EOS) {
								check_err(65, pos+s-1, pos+t-2, ln);
							}
							s = t - 1;
/* 2009-06-08
						} else if (isCompFound) {
							if (check_isSign)
								;
							else if (w[s+1] == 's' && (w[s+2] == ':' || w[s+2] == EOS))
								;
							else if ((w[s+1] == 'c' || w[s+2] == 'n') && w[s+2] == EOS)
								;
							else 
								check_err(88, pos-1, pos+len-1, ln);
							break;
*/
						} else if (w[s+1] == EOS || uS.isRightChar(w,s+1,'@',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'(',&dFnt,MBF) || uS.isRightChar(w,s+1,')',&dFnt,MBF)) {
							check_err(65, pos+s-1, pos+s-1, ln);
						} else if (uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'\'',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'"',&dFnt,MBF) || uS.isRightChar(w,s+1,'_',&dFnt,MBF)) {
							if (uS.atUFound(w,s,&dFnt,MBF))
								break;
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (my_CharacterByteType(w,(short)s,&dFnt)==0 && w[s]>0 && w[s]<127) {
						if (uS.atUFound(w,s,&dFnt,MBF))
							break;
						check_err(48, pos+s-1, pos+s-1, ln);
					}
				}
/* 2019-04-23 do not check for upper case letters
				else if (isupper((unsigned char)w[s]) && w[s]>0 && w[s]<127 && !check_isCAFound && !capWErrFound &&
						   (s == 0 || (w[s-1] != '-' && w[s-1] != '\'' && w[s-1] != '_' && w[s-1] != '0' && w[s-1] != '(' && s != CAOffset))) {
					if (s == 0 && check_isGerman)
						;
					else
						capWFound = TRUE;
					if (w[s-1] == '+' && check_isGerman)
						;
					else if (s > 0 && !check_isRightPref(ts->prefs, w, s) && !check_isException(w, check_WordExceptions)) {
						if (uS.atUFound(w,s,&dFnt,MBF))
							break;
						capWErrFound = TRUE;
						check_err(49, pos+s-1, pos+s-1, ln);
					}
				}
 */
				s++;
			}
		}
	} else if (uS.partcmp(ts->code, "@Languages:", FALSE, TRUE)) {
		s = 0;
		while (w[s] != EOS) {
			if (uS.isRightChar(w,s,'=',&dFnt,MBF)) {
/* 18-07-2008
				if (!strcmp(w+s+1, "blue") || !strcmp(w+s+1, "red") || !strcmp(w+s+1, "green") || !strcmp(w+s+1, "magenta"))
					break;
				else
*/					check_err(96, pos+s, pos+strlen(w)-2, ln);
			}
			s++;
		}
	} else if (uS.partcmp(ts->code, "%mor", FALSE, TRUE) || uS.partcmp(ts->code, "%xmor", FALSE, TRUE) ||
			   uS.partcmp(ts->code, "%trn", FALSE, TRUE)) {
		if (w[0] == (char)0xe2 && w[1] == (char)0x80 && w[2] == (char)0x9E && w[3] == EOS) {
		} else if (uS.isRightChar(w,0,'[',&dFnt,MBF) && uS.isRightChar(w,len,']',&dFnt,MBF)) {
			if (isPostCodeMark(w[1], w[2])) {
				check_err(109, pos-1, pos+len-1, ln);
			}
		} else if (uS.isRightChar(w,0,'+',&dFnt,MBF)) ;
		else if (uS.isRightChar(w,0, '-',&dFnt,MBF) && w[1] != EOS && !isalnum((unsigned char)w[1])) ;
		else if (uS.patmat(w,"unk|xxx") || uS.patmat(w,"unk|yyy") || uS.patmat(w,"*|www") /*|| uS.patmat(w,"cm|cm")*/) {
			check_err(134, pos-1, pos+len-1, ln);
		} else {
			int  e = 0, numOfCompounds;
			char isFoundBar, res;

			if ((res=check_MorItem(w, &s, &e, 0))) {
				check_err(65, pos+s-1, pos+s-1, ln);
				return;
			}
			if (w[0] == '^' && uS.partcmp(utterance->speaker, "%mor", FALSE, FALSE))
				check_err(48, pos+s-1, pos+s-1, ln);
			isFoundBar = 0;
			s = 0;
			numOfCompounds = -1;
			while (w[s] != EOS) {
				if (isalnum((unsigned char)w[s]) || my_CharacterByteType(w, (short)s, &dFnt) != 0) {
				} else {
					if ((t=isBadQuotes(w, s)) > 0) {
						check_err(t, pos+s-1, pos+s-1, ln);
						s += 3;
					} else if (isIllegalASCII(w, s)) {
						check_err(86, pos+s-1, pos+s-1, ln);
					} else if (w[s] == HIDEN_C) {
						check_err(65, pos+s-1, pos+s-1, ln);
					} else if (uS.isRightChar(w,s,'-',&dFnt,MBF) || uS.isRightChar(w,s,'&',&dFnt,MBF) ||
							uS.isRightChar(w,s,'*',&dFnt,MBF) || uS.isRightChar(w,s,'#',&dFnt,MBF) ||
							uS.isRightChar(w,s,'\'',&dFnt,MBF)|| uS.isRightChar(w,s,'/',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'=',&dFnt,MBF) || uS.isRightChar(w,s+1,'|',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'#',&dFnt,MBF) || uS.isRightChar(w,s+1,':',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'^',&dFnt,MBF) || uS.isRightChar(w,s+1,'~',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'*',&dFnt,MBF) || uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'\'',&dFnt,MBF)) {
							if (uS.isRightChar(w,s,'\'',&dFnt,MBF) == FALSE)
								check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'+',&dFnt,MBF) || uS.isRightChar(w,s,'~',&dFnt,MBF) || uS.isRightChar(w,s,'$',&dFnt,MBF)) {
						if (isFoundBar == 1) {
							isFoundBar = 0;
							e = s;
						} else {
							check_err(80, pos-1, pos+s-2, ln);
						}
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'=',&dFnt,MBF) || uS.isRightChar(w,s+1,'|',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'#',&dFnt,MBF) || uS.isRightChar(w,s+1,':',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'^',&dFnt,MBF) || uS.isRightChar(w,s+1,'~',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'*',&dFnt,MBF) || uS.isRightChar(w,s+1,'+',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'\'',&dFnt,MBF)) {
							check_err(65, pos+s-1, pos+s-1, ln);
						}
						if (uS.isRightChar(w,s,'+',&dFnt,MBF)) {
							if (uS.isRightChar(w,s-1,'|',&dFnt,MBF)) {
								if (numOfCompounds >= 0) {
									numOfCompounds = -2;
								} else
									numOfCompounds = 0;
							} else if (numOfCompounds == -1)
								numOfCompounds = -2;
							else if (numOfCompounds >= 0)
								numOfCompounds++;
						}
					} else if (uS.isRightChar(w,s,':',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'=',&dFnt,MBF) || uS.isRightChar(w,s+1,'|',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'#',&dFnt,MBF) || uS.isRightChar(w,s+1,':',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'^',&dFnt,MBF) || uS.isRightChar(w,s+1,'~',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'\'',&dFnt,MBF)) {
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'|',&dFnt,MBF)) {
						if (isFoundBar) {
							check_err(79, pos-1, pos+s-1, ln);
						} else
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'=',&dFnt,MBF) || uS.isRightChar(w,s+1,'|',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'#',&dFnt,MBF) || uS.isRightChar(w,s+1,':',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'^',&dFnt,MBF) || uS.isRightChar(w,s+1,'~',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'*',&dFnt,MBF)) {
							check_err(65, pos+s-1, pos+s-1, ln);
						}
						isFoundBar++;
					} else if (uS.isRightChar(w,s,'^',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'=',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,':',&dFnt,MBF) || uS.isRightChar(w,s+1,'^',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'~',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'\'',&dFnt,MBF)) {
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'?',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'=',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,':',&dFnt,MBF) || uS.isRightChar(w,s+1,'^',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'*',&dFnt,MBF) ||
								uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'\'',&dFnt,MBF)) {
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'=',&dFnt,MBF)) {
						for (t=s+1; isalpha((unsigned char)w[t]) || w[t] == '/' || w[t] == '_' || UTF8_IS_LEAD(w[t]) || UTF8_IS_TRAIL(w[t]); t++) ;
						if (w[t] != EOS && w[t] != '~' && w[t] != '+' && w[t] != '$' && w[t] != '%' && w[t] != '^') {
							check_err(65, pos+s-1, pos+t-2, ln);
						}
						s = t - 1;
					} else if (uS.isRightChar(w,s,'_',&dFnt,MBF)) {
					} else if (uS.isRightChar(w,s,'`',&dFnt,MBF) && check_isArabic) {
					} else if (my_CharacterByteType(w,(short)s,&dFnt)==0 && w[s]>0 && w[s]<127) {
						check_err(48, pos+s-1, pos+s-1, ln);
					}
				}
				s++;
			}
			if (isFoundBar < 1)
				check_err(80, pos+e-1, pos+len-1, ln);
			if (numOfCompounds == -2 || (numOfCompounds >= 0 && numOfCompounds < 1))
				check_err(87, pos-1, pos+len-1, ln);
		}
	} else if (uS.partcmp(ts->code, "%cnl", FALSE, TRUE) || uS.partcmp(ts->code, "%xcnl", FALSE, TRUE)) {
		if (w[0] == (char)0xe2 && w[1] == (char)0x80 && w[2] == (char)0x9E && w[3] == EOS) {
		} else if (uS.isRightChar(w,0,'[',&dFnt,MBF) && uS.isRightChar(w,len,']',&dFnt,MBF)) {
			if (isPostCodeMark(w[1], w[2])) {
				check_err(109, pos-1, pos+len-1, ln);
			}
		} else if (uS.isRightChar(w,0,'+',&dFnt,MBF)) ;
		else if (uS.isRightChar(w,0, '-',&dFnt,MBF) && w[1] != EOS && !isalnum((unsigned char)w[1])) ;
		else if (uS.patmat(w,"unk|xxx") || uS.patmat(w,"unk|yyy") || uS.patmat(w,"*|www") /*|| uS.patmat(w,"cm|cm")*/) {
			check_err(134, pos-1, pos+len-1, ln);
		} else {
			int numOfCompounds;

			s = 0;
			numOfCompounds = -1;
			while (w[s] != EOS) {
				if (isalnum((unsigned char)w[s]) || my_CharacterByteType(w, (short)s, &dFnt) != 0) {
					if ((s == 0 || isSpace(w[s-1])) && w[s] == 'g' && w[s+1] == 'e' && w[s+2] == 'n' &&
						(uS.isskip(w,s+3,&dFnt,TRUE) || w[s+3] == EOS)) {
						check_err(135, pos+s-1, pos+s+1, ln);
					}
				} else {
					if ((t=isBadQuotes(w, s)) > 0) {
						check_err(t, pos+s-1, pos+s-1, ln);
						s += 3;
					} else if (isIllegalASCII(w, s)) {
						check_err(86, pos+s-1, pos+s-1, ln);
					} else if (w[s] == HIDEN_C) {
						check_err(65, pos+s-1, pos+s-1, ln);
					} else if (uS.isRightChar(w,s,':',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'=',&dFnt,MBF) || uS.isRightChar(w,s+1,'|',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'#',&dFnt,MBF) || uS.isRightChar(w,s+1,':',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'^',&dFnt,MBF) || uS.isRightChar(w,s+1,'~',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'\'',&dFnt,MBF)) {
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					} else if (uS.isRightChar(w,s,'^',&dFnt,MBF)) {
						check_err(135, pos+s-1, pos+s-1, ln);
					} else if (uS.isRightChar(w,s,'?',&dFnt,MBF)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&dFnt,MBF) || uS.isRightChar(w,s+1,'&',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'=',&dFnt,MBF) || uS.isRightChar(w,s+1,'#',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,':',&dFnt,MBF) || uS.isRightChar(w,s+1,'^',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'~',&dFnt,MBF) || uS.isRightChar(w,s+1,'*',&dFnt,MBF) ||
							uS.isRightChar(w,s+1,'+',&dFnt,MBF) || uS.isRightChar(w,s+1,'\'',&dFnt,MBF)) {
							check_err(65, pos+s-1, pos+s-1, ln);
						}
					}
				}
				s++;
			}
			if (numOfCompounds == -2 || (numOfCompounds >= 0 && numOfCompounds < 1))
				check_err(87, pos-1, pos+len-1, ln);
		}
	} else if (uS.partcmp(ts->code, "%cod", FALSE, TRUE)) {
		if (uS.isRightChar(w,0,'[',&dFnt,MBF) && uS.isRightChar(w,len,']',&dFnt,MBF)) {
			if (isPostCodeMark(w[1], w[2])) {
				check_err(109, pos-1, pos+len-1, ln);
			}
		} else {
			s = 0;
			while (w[s] != EOS) {
				if ((t=isBadQuotes(w, s)) > 0) {
					check_err(t, pos+s-1, pos+s-1, ln);
					s += 3;
				} else if (isIllegalASCII(w, s)) {
					check_err(86, pos+s-1, pos+s-1, ln);
				} else if (w[s] == HIDEN_C) {
					check_err(65, pos+s-1, pos+s-1, ln);
				}
				s++;
			}
		}
	} else if (uS.partcmp(ts->code, "%ges", FALSE, TRUE)) {
		if (uS.isRightChar(w,0,'[',&dFnt,MBF) && uS.isRightChar(w,len,']',&dFnt,MBF)) {
			if (isPostCodeMark(w[1], w[2])) {
				check_err(109, pos-1, pos+len-1, ln);
			}
		}
	} else if (uS.partcmp(ts->code, "%pho", FALSE, TRUE)) {
		if (uS.isRightChar(w,0,'[',&dFnt,MBF) && uS.isRightChar(w,len,']',&dFnt,MBF)) {
			if (isPostCodeMark(w[1], w[2])) {
				check_err(109, pos-1, pos+len-1, ln);
			}
		} else {
			s = 0;
			while (w[s] != EOS) {
				if (w[s] == HIDEN_C) {
					check_err(65, pos+s-1, pos+s-1, ln);
				}
				s++;
			}
		}
	} else if (ts->code[0] == '%') {
		if (uS.isRightChar(w,0,'[',&dFnt,MBF) && uS.isRightChar(w,len,']',&dFnt,MBF)) {
			if (isPostCodeMark(w[1], w[2])) {
				check_err(109, pos-1, pos+len-1, ln);
			}
		} else {
			s = 0;
			while (w[s] != EOS) {
				if ((t=isBadQuotes(w, s)) > 0) {
					check_err(t, pos+s-1, pos+s-1, ln);
					s += 3;
				} else if (isIllegalASCII(w, s)) {
					check_err(86, pos+s-1, pos+s-1, ln);
				} else if (w[s] == HIDEN_C) {
					check_err(65, pos+s-1, pos+s-1, ln);
				} else if (w[s] == '$' && s > 0) {
					check_err(48, pos+s-1, pos+s-1, ln);
				}
				s++;
			}
		}
	}
}

static char check_isMorTier(void) {
	if (utterance->speaker[0] == '%' &&
		(utterance->speaker[1] == 'm' || utterance->speaker[1] == 'M') &&
		(utterance->speaker[2] == 'o' || utterance->speaker[2] == 'O') &&
		(utterance->speaker[3] == 'r' || utterance->speaker[3] == 'R') &&
		utterance->speaker[4] == ':')
		return(TRUE);
	else
		return(FALSE);
}

static char check_getMediaTagInfo(char *line, long *Beg, long *End) {
	int i;
	long beg = 0L, end = 0L;

	if (*line == HIDEN_C)
		line++;
	if (*line == EOS)
		return(FALSE);
	if (*line == '%' && (*(line+1) == 's' || *(line+1) == 'm'))
		return(6);
	if (*line == '%' && (*(line+1) == 'p' || *(line+1) == 't' || *(line+1) == 'n'))
		return(0);
	if (*line == '0' && isdigit((unsigned char)*(line+1)))
		return(3);
	for (i=0; *line && isdigit((unsigned char)*line); line++)
		templineC1[i++] = *line;
	templineC1[i] = EOS;
	beg = atol(templineC1);
	if (beg == 0)
		beg = 1;
	if (*line != '_')
		return(2);
	line++;
	if (*line == EOS)
		return(FALSE);
	if (*line == '0' && is_unCH_digit((int)*(line+1)))
		return(3);
	for (i=0; *line && isdigit((unsigned char)*line); line++)
		templineC1[i++] = *line;
	templineC1[i] = EOS;
	end = atol(templineC1);
	*Beg = beg;
	*End = end;
	if (*line == '-')
		line++;
	if (*line != HIDEN_C)
		return(2);
	return(TRUE);
}

static char check_getOLDMediaTagInfo(char *line, char *fname, long *Beg, long *End) {
	int i;
	long beg = 0L, end = 0L;

	if (*line == HIDEN_C)
		line++;
	if (*line == EOS)
		return(FALSE);
	if (isdigit(*line))
		return(6);
	for (i=0; *line && *line != ':'; line++)
		templineC1[i++] = *line;
	if (*line == EOS)
		return(FALSE);
	templineC1[i++] = *line;
	templineC1[i] = EOS;
	line++;
	if (!uS.partcmp(templineC1, SOUNDTIER, FALSE, FALSE) && !uS.partcmp(templineC1, REMOVEMOVIETAG, FALSE, FALSE))
		return(FALSE);

	if (*line != '"')
		return(2);
	line++;
	if (*line == EOS)
		return(FALSE);
	for (i=0; *line && *line != '"' && *line != HIDEN_C; line++) {
		templineC1[i++] = *line;
		if (isSpace(*line))
			return(4);
	}
	templineC1[i] = EOS;
	if (strrchr(templineC1, '.'))
		return(5);
	if (*line != '"')
		return(2);
	strcpy(fname, templineC1);
	line++;
	if (*line == EOS)
		return(FALSE);
	if (*line != '_')
		return(2);
	line++;
	if (*line == EOS)
		return(FALSE);
	if (*line == '0' && isdigit((unsigned char)*(line+1)))
		return(3);
	for (i=0; *line && isdigit((unsigned char)*line); line++)
		templineC1[i++] = *line;
	templineC1[i] = EOS;
	beg = atol(templineC1);
	if (beg == 0)
		beg = 1;
	if (*line != '_')
		return(2);
	line++;
	if (*line == EOS)
		return(FALSE);
	if (*line == '0' && is_unCH_digit((int)*(line+1)))
		return(3);
	for (i=0; *line && isdigit((unsigned char)*line); line++)
		templineC1[i++] = *line;
	templineC1[i] = EOS;
	end = atol(templineC1);
	*Beg = beg;
	*End = end;
	if (*line == '-')
		line++;
	if (*line != HIDEN_C)
		return(2);
	return(TRUE);
}

static void check_CleanTierNames(char *st) {
	register int i;

	i = strlen(st) - 1;
	while (i >= 0 && (st[i] == ' ' || st[i] == '\t' || st[i] == '\n')) i--;
	if (i >= 0 && st[i] == ':') i--;
	if (*st == '*') {
		int j = i;
		while (i >= 0 && st[i] != '-') i--;
		if (i < 0) i = j;
		else i--;
	}
	st[i+1] = EOS;
}

static int check_checkBulletsConsist(char *FirstBulletFound, char *word) {
	char res;
	char tFname[FILENAME_MAX];
	long tBegTime;
	long tEndTime;

	if (!check_isCheckBullets)
		return(0);
	if (isMediaHeaderFound) {
		isBulletsFound = TRUE;
		tBegTime = check_SNDBeg;
		tEndTime = check_SNDEnd;
		res = check_getMediaTagInfo(word, &check_SNDBeg, &check_SNDEnd);
		if (res == 2)
			return(89); // illegal character found
		if (res == 3)
			return(90); // illegal time rep.
		if (res == 4)
			return(98); // Space is not allow in media file name.
		if (res == 5)
			return(99); // Extension is not allow at the end of media file name.
		if (res == 6)
			return(115); // old bullets format
		if (res) {
			if (check_isUnlinkedFound) {
				check_isUnlinkedFound = FALSE;
				return(124); // remove "unlinked" from @Media header
			}
			check_CleanTierNames(templineC);
			if (check_SNDEnd <= check_SNDBeg)
				return(82); // BEG mark must be smaller than END mark
			if (tEndTime != 0L) {
				if (*FirstBulletFound == FALSE) {
					*FirstBulletFound = TRUE;
					if (check_SNDBeg < check_tierBegTime) {
						check_tierBegTime = check_SNDBeg;
						return(83); // Current BEG time is smaller than previous' tier BEG time
					} else
						check_tierBegTime = check_SNDBeg;
				} else {
					if (check_SNDBeg < tBegTime)
						return(83); // Current BEG time is smaller than previous' tier BEG time
				}
				tDiff = check_getLatTime(templineC) - check_SNDBeg;
				if (tDiff > 500L) {
					check_setLastTime(templineC, check_SNDEnd);
					return(133); // Current BEG time is smaller than same speaker's previous END time
				}
				if (checkBullets == 1) {
					if (check_SNDBeg < tEndTime) {
						tDiff = tEndTime - check_SNDBeg;
						check_setLastTime(templineC, check_SNDEnd);
						return(84); // Current BEG time is smaller than previous' tier END time
					}
					if (check_SNDBeg > tEndTime) {
						check_setLastTime(templineC, check_SNDEnd);
						return(85); // Gap found between current BEG time and previous' tier END time
					}
				}
			}
			check_setLastTime(templineC, check_SNDEnd);
		}
		return(0);
	}

	tBegTime = check_SNDBeg;
	tEndTime = check_SNDEnd;
	strcpy(tFname, check_SNDFname);
	res = check_getOLDMediaTagInfo(word, check_SNDFname, &check_SNDBeg, &check_SNDEnd);
	if (res == 2)
		return(89); // illegal character found
	if (res == 3)
		return(90); // illegal time rep.
	if (res == 4)
		return(98); // Space is not allow in media file name.
	if (res == 5)
		return(99); // Extension is not allow at the end of media file name.
	if (res == 6)
		return(112); // Missing @Media header.
	if (res) {
		if (check_isUnlinkedFound) {
			check_isUnlinkedFound = FALSE;
			return(124); // remove "unlinked" from @Media header
		}
		check_CleanTierNames(templineC);
		if (uS.mStricmp(tFname, check_SNDFname) == 0 && tEndTime != 0L) {
			if (check_SNDEnd <= check_SNDBeg)
				return(82); // BEG mark must be smaller than END mark
			if (*FirstBulletFound == FALSE) {
				*FirstBulletFound = TRUE;
				if (check_SNDBeg < check_tierBegTime) {
					check_tierBegTime = check_SNDBeg;
					return(83); // Current BEG time is smaller than previous' tier BEG time
				} else
					check_tierBegTime = check_SNDBeg;
			} else {
				if (check_SNDBeg < tBegTime)
					return(83); // Current BEG time is smaller than previous' tier BEG time
			}
			tDiff = check_getLatTime(templineC) - check_SNDBeg;
			if (tDiff > 500L) {
				check_setLastTime(templineC, check_SNDEnd);
				return(133); // Current BEG time is smaller than same speaker's previous END time
			}
			if (checkBullets == 1) {
				if (check_SNDBeg < tEndTime) {
					tDiff = tEndTime - check_SNDBeg;
					check_setLastTime(templineC, check_SNDEnd);
					return(84); // Current BEG time is smaller than previous' tier END time
				}
				if (check_SNDBeg > tEndTime)
					check_setLastTime(templineC, check_SNDEnd);
					return(85); // Gap found between current BEG time and previous' tier END time
			}
		}
		check_setLastTime(templineC, check_SNDEnd);
	}
	return(0);
}

static void check_removeCAChars(char *word) {
	int i, res;
	int matchedType;

	if (word[0] == '+' || word[0] == '-' || word[0] == '[' || word[0] == '&')
		return;

	i = 0;
	while (word[i]) {
		if ((res=uS.HandleCAChars(word+i, &matchedType)) != 0) {
			if (matchedType != NOTCA_LEFT_ARROW_CIRCLE)
				strcpy(word+i,word+i+res);
			else
				i++;
		} else
			i++;
	}
}

static char check_isBlankLine(char *line, int s) {
	while (isSpace(line[s]) && line[s] != '\n' && line[s] != EOS)
		s++;
	if (line[s] == '\n')
		return(TRUE);
	else
		return(FALSE);
}

static char check_lastBulletOnTier(char *st) {
	for (; *st != EOS; st++) {
		if (*st != '\n' && !isSpace(*st))
			return(FALSE);
	}
	return(TRUE);
}

static void check_ParseBlob(CHECK_TIERS *ts) {
	long ln = 0L;
	int s = 0, e = 0, res;
	char word[BUFSIZ], *st, hidenc;
	char isTextBetweenBulletsFound = FALSE, FirstBulletFound = FALSE, isLastItemBullet = FALSE;

	word[0] = EOS;
	if (ts != NULL && ts->IGN)
		return;
	while (1) {
		st = word;
		while ((*st=uttline[s++]) != EOS && uS.isskip(uttline, s-1, &dFnt, MBF)) ;

		if (*st == EOS)
			break;

		if (uttline[s-1] == HIDEN_C)
			hidenc = TRUE;
		else
			hidenc = FALSE;
		e = s;
		while ((*++st=uttline[e]) != EOS && (!uS.isskip(uttline,e,&dFnt,MBF) || hidenc)) {
			e++;
			if (e-s >= BUFSIZ) {
				fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, ln+lineno);
				fprintf(stderr,"%s\n", uttline+s);
				fprintf(stderr,"Word is too long.\n");
				check_clean_headerr(); check_clean_speaker(headsp); check_clean_IDsp(check_idsp);
				check_clean__t(headt); check_clean__t(maint);
				check_clean__t(codet);
				cleanupLanguages();
				cutt_exit(0);
			}
			if (hidenc && uttline[e-1] == HIDEN_C) { hidenc = FALSE; st++; break; }
			if (!hidenc && uttline[e] == HIDEN_C) { st++; break; }
		}
		*st = EOS;
		if (word[0] == HIDEN_C) {
			if (*utterance->speaker == '*') {
				if (!isTextBetweenBulletsFound && (!check_isMultiBullets || !isLastItemBullet || !check_lastBulletOnTier(uttline+e))) {
					check_err(73,s-1,e-1,ln+lineno);
				}
			}
			if (hidenc) {
				check_err(59,s-1,s-1,ln+lineno);
			}
			if (utterance->speaker[0] == '*') {
				if ((res=check_checkBulletsConsist(&FirstBulletFound, word)) != 0)
					check_err(res,s-1,s-1,ln+lineno);
			}
			isTextBetweenBulletsFound = FALSE;
			isLastItemBullet = TRUE;
		} else {
			isTextBetweenBulletsFound = TRUE;
			isLastItemBullet = TRUE;
		}
		s = e;
	}
}

#if defined(_MAC_CODE) || defined(_WIN32)
static char check_isRight2Left(char *line) {
	unCH *wline;

	if (!check_isHebrew && !check_isArabic)
		return(FALSE);
	u_strcpy(templineW, line, UTTLINELEN);
	for (wline=templineW; *wline != EOS; wline++) {
		if (*wline >= 0x0590 && *wline < 0x0780)
			return(TRUE);
	}
	return(FALSE);
}
#endif

static char check_isTwoLetterCode(char *code) {
	if (strlen(code) < 3)
		return(TRUE);
	if (code[2] == '-')
		return(TRUE);
	return(FALSE);
}

static char check_isReplacementError(char *line, int s) {
	for (s--; s >= 0 && (isSpace(line[s]) || line[s] == '\n' || line[s] == '\r'); s--) ;
	if (s < 0 || line[s] == ']' || line[s] == '>' || uS.isskip(line, s, &dFnt, MBF))
		return(TRUE);
	for (; s >= 0 && line[s] != '.' && !uS.isskip(line, s, &dFnt, MBF); s--) ;
	if (uS.isRightChar(line, s, '.', &dFnt, MBF) && uS.isPause(line, s, NULL, NULL))
		return(TRUE);
	s++;
	if (line[s] == '&' || line[s] == '+') // 2023-02-24 || line[s] == '0')
		return(TRUE);
	return(FALSE);
}

static char check_isNumbersLang(char *lang) {
	if (!strcmp(lang, "zh")  || !strncmp(lang, "zh-", 3)  ||
		!strcmp(lang, "chi") || !strncmp(lang, "chi-", 4) ||
		!strcmp(lang, "zho") || !strncmp(lang, "zho-", 4) ||
		!strcmp(lang, "cy")  || !strcmp(lang, "cym")      ||
		!strcmp(lang, "vi")  || !strcmp(lang, "vie")      ||
		!strcmp(lang, "th")  || !strcmp(lang, "tha")      ||
		!strcmp(lang, "nan") || !strcmp(lang, "yue")      ||
		!strcmp(lang, "min") || !strcmp(lang, "hak")) {
		return(TRUE);
	}
	return(FALSE);
}

static void check_CheckLanguageCodes(char *word, int s, long ln, char *isLcodeFound, char *isGlcodeFound) {
	int i, j, li;
	char t;
	char *st, isAtS;
	char old_code[20];

	if (word[0] == '[' && word[1] == '-') {
		j = 2;
		for (i=j; isSpace(word[i]); i++) ;
		if (word[i] == EOS || word[i] == ']')
			return;
		isAtS = FALSE;
	} else if ((st=strchr(word, '@')) != NULL) {
		if (st[1] != 's') {
			return;
		} else if (st[2] != ':') {
			if (check_curLanguage == 0) {
				li = 1;
			} else if (check_curLanguage == 1) {
				li = 0;
			} else {
				li = -1;
			}
			if (li == -1)
				check_isNumberShouldBeHere = FALSE;
			else if (check_isNumbersLang(check_LanguagesTable[li]))
				check_isNumberShouldBeHere = TRUE;
			else
				check_isNumberShouldBeHere = FALSE;
			*isLcodeFound = TRUE;
			return;
		}
		i = (st - word) + 3;
		if (word[i] == EOS)
			return;
		isAtS = TRUE;
	} else
		return;
// 2019-04-17	for (j=i; word[j] != EOS && word[j] != ']' && word[j] != '&' && word[j] != '+' && word[j] != '$' && word[j] != ':'; j++) ;
	for (j=i; (word[j] >= 'a' && word[j] <= 'z') || word[j] == '-'; j++) ;
	t = word[j];
	word[j] = EOS;
	if (check_isTwoLetterCode(word+i)) {
		strncpy(check_fav_lang, word+i, 19);
		check_fav_lang[19] = EOS;
		if (!getLanguageCodeAndName(check_fav_lang, TRUE, NULL))
			check_fav_lang[0] = EOS;
		word[j] = t;
		if (j-i < 2)
			i -= 2;
		check_err(120,s+i-1,s+j-2,ln);
	} else {
		strncpy(check_fav_lang, word+i, 19);
		check_fav_lang[19] = EOS;
		strcpy(old_code, check_fav_lang);
		if (!getLanguageCodeAndName(check_fav_lang, TRUE, NULL)) {
			word[j] = t;
			check_err(121,s+i-1,s+j-2,ln);
		} else {
			if (strcmp(old_code, check_fav_lang)) {
				word[j] = t;
				check_err(120,s+i-1,s+j-2,ln);
			} else {
				if (isAtS) { // 2019-01-06 @s:rus
					check_isNumberShouldBeHere = FALSE;
					do {
						if (check_isNumbersLang(word+i))
							check_isNumberShouldBeHere = TRUE;
						word[j] = t;
						if (word[j] == '+' || word[j] == '&') {
							i = j + 1;
							for (j=i; (word[j] >= 'a' && word[j] <= 'z') || word[j] == '-'; j++) ;
							t = word[j];
							word[j] = EOS;
						} else
							break;
					} while (word[i] != EOS) ;
					*isLcodeFound = TRUE;
				} else {
					check_isLangMatch(word+i, 0, s+i-1, 152, TRUE);
					if (check_isNumbersLang(word+i))
						check_isNumberShouldBeHere = TRUE;
					else
						check_isNumberShouldBeHere = FALSE;
					*isGlcodeFound = TRUE;
					for (li=0; li < NUMLANGUAGESINTABLE; li++) {
						if (strcmp(check_LanguagesTable[li], word+i) == 0) {
							check_curLanguage = li;
							break;
						}
					}

				}
				word[j] = t;
			}
		}
	}
	word[j] = t;
}

static char check_isGrouping(char *beg, char *end, int e) {

	if (beg[0] == (char)0xE2 && beg[1] == (char)0x80 && beg[2] == (char)0xB9 && beg[3] == '(' && e >= 3 &&
		end[e-3] == (char)0xE2 && end[e-2] == (char)0x80 && end[e-1] == (char)0xB9 && end[e] == '(')
		return(TRUE);
	return(FALSE);
}

static char check_isPlayBullet(char *word) {
	if (isdigit(word[1]) || (word[1] == '%' && (word[2] == 's' || word[2] == 'm')))
		return(TRUE);
	return(FALSE);
}

static void check_ParseWords(CHECK_TIERS *ts) {
	long ln = 0L;
	int j, r, s = 0, e = 0, te, res, isReplaceFound = -1;
	int matchType;
	char word[BUFSIZ], *st, sq, hidenc, t;
	char FirstWordFound = FALSE, FirstBulletFound = FALSE, isTextBetweenBulletsFound = FALSE,
		isBulletFound = FALSE, anyBulletFound, isTextFound, isPoundSymFound, isSpecialDelimFound, isR2L,
		CADelFound = FALSE, isPreLanguageCodeFound = FALSE, isLastItemBullet,
		isLcodeFound, tisNumberShouldBeHere, isGLcodeFound, isGNumberShouldBeHere, isPauseFound;
	int isFasterMatched, isSlowerMatched, isCreakyMatched, isUnsureMatched, isSofterMatched, 
		isLouderMatched, isLoPitchMatched, isHiPitchMatched, isSmileMatched, isWhisperMatched,
		isYawnMatched, isSingingMatched, isPreciseMatched, isBreathyMatched, isLeftArrowMatched;
	int sb = 0, pb = 0, ab = 0, cb = 0, gm = 0, sgm = 0, qt = 0, lpb = 0, anb = FALSE, DelFound = 0;
	int pauseBeg, pauseEnd;

	pauseBeg = 0;
	pauseEnd = 0;
	isPauseFound = FALSE;
	isFasterMatched = -1;
	isSlowerMatched = -1;
	isCreakyMatched = -1;
	isUnsureMatched = -1;
	isSofterMatched = -1;
	isLouderMatched = -1;
	isLoPitchMatched= -1;
	isHiPitchMatched= -1;
	isSmileMatched  = -1;
	isWhisperMatched= -1;
	isYawnMatched   = -1;
	isSingingMatched= -1;
	isPreciseMatched= -1;
	isBreathyMatched= -1;
	isLeftArrowMatched = -1;
	check_curLanguage = 0;
	isGLcodeFound = FALSE;
	isGNumberShouldBeHere = check_isNumberShouldBeHere;
	word[0] = EOS;
	if (ts->IGN)
		return;
	if (*utterance->speaker == '%' || *utterance->speaker == '@')
		isTextFound = TRUE;
	else
		isTextFound = FALSE;
	isPoundSymFound = FALSE;
	isSpecialDelimFound = FALSE;
	anyBulletFound = FALSE;
	isLastItemBullet = FALSE;
#if defined(_MAC_CODE) || defined(_WIN32)
	isR2L = check_isRight2Left(uttline);
#else
	isR2L = FALSE;
#endif
	while (1) {
		st = word;
		t = word[0];
		lpb = pb;
		while ((*st=uttline[s++]) != EOS && uS.isskip(uttline, s-1, &dFnt, MBF) && !uS.isRightChar(uttline, s-1, '[', &dFnt, MBF)) {
			if (*st == '>')
				isPoundSymFound = FALSE;
			if (is_italic(utterance->attLine[s-1])) {
				check_err(102,s-1,s-1,ln+lineno);
			}
			if (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) {
				if (uttline[s-1] == ';' ) {
					check_err(48,s-1,s-1,ln+lineno);
				} else if (uttline[s-1] == '<' && isSpace(uttline[s])) {
					check_err(160,s-1,s+1,ln+lineno);
				} else if (uttline[s] == '>' && isSpace(uttline[s-1])) {
					j = s - 2;
					if (j < 0)
						j = 0;
					check_err(160,j,j+2,ln+lineno);
				}
			}
			if (*st == ',' && *utterance->speaker == '*') {
				if (!uS.isskip(uttline, s, &dFnt, MBF) && uttline[s] != ',' && uttline[s] != EOS && uttline[s] != '\n') {
					if (uS.HandleCAChars(uttline+s, &matchType)) {
						if (matchType != NOTCA_GROUP_START && matchType != NOTCA_GROUP_END &&
							matchType != CA_SUDDEN_STOP && matchType != CA_HURRIED_START &&
							matchType != NOTCA_SIGN_GROUP_START && matchType != NOTCA_SIGN_GROUP_END &&
							matchType != CA_APPLY_OPTOPSQ && matchType != CA_APPLY_CLTOPSQ &&
							matchType != CA_APPLY_OPBOTSQ && matchType != CA_APPLY_CLBOTSQ) {
							check_err(92,s-1,s-1,ln+lineno);
						}
					} else
						check_err(92,s-1,s-1,ln+lineno);
				} else if (uttline[s] == ',' && uttline[s+1] == ',') {
					check_err(107,s-1,s+1,ln+lineno);
				} else if (uttline[s] == ',') {
					check_err(156,s-1,s,ln+lineno);
				}
			}
			if (*st == '\n') {
				isBulletFound = FALSE;
				ln += 1;
				if (check_isBlankLine(uttline, s)) {
					check_err(91,s,s,ln+lineno);
				}
			} else if (uttline[s-1] == '?' && check_isMorTier() && uttline[s] == '|')
				break;
			else if ((res=uS.IsUtteranceDel(uttline, s-1))) {
				if (!isSpecialDelimFound) {
					if (!strncmp(utterance->speaker, "%mor", 4) && check_lastSpDelim[0] != EOS && check_lastSpDelim[0] != uttline[s-1]) {
						check_err(94,s-1,s-1,ln+lineno);
					}
					if (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) {
						check_lastSpDelim[0] = uttline[s-1];
						check_lastSpDelim[1] = EOS;
					}
				}
				if (uS.atUFound(uttline,s-1,&dFnt,MBF))
					break;
				if (ts->UTD == -1) {
					check_err(48,s-1,s-1,ln+lineno);
				} else if (ts->UTD == 1) {
					if (DelFound == 1)
						check_err(50,s-1,s-1,ln+lineno);
					DelFound = res;
					if (!uS.isRightChar(uttline,s-2,'+',&dFnt,MBF) && uS.IsUtteranceDel(uttline, s))
						check_err(50,s,s,ln+lineno);
					while (uttline[s]=='.' || uttline[s]=='!' || uttline[s]=='?')
						s++;
					while (isSpace(uttline[s])) s++;
				}
			} else {
				if (uttline[s-1] == ']' && s-1 == isReplaceFound) {
				} else
					check_CheckBrakets(uttline,s-1,&sb,&pb,&ab,&cb,&anb);
			}
		}
		if ((*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) && uttline[s-1] == '[') {
			j = s - 2;
			if (j >= 0) {
				if (!isSpace(uttline[j])) {
					check_err(161,j,j+1,ln+lineno);
				}
			}
		}
		if (*st == EOS) {
			if ((*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) && anb) {
				check_err(51,s-1,s-1,ln+lineno);
			}
			break;
		}
		hidenc = FALSE;
		sq = FALSE;
		if (uttline[s-1] == HIDEN_C)
			hidenc = TRUE;
		else if (uS.isRightChar(uttline,s-1,'[',&dFnt,MBF))
			sq = TRUE;
		else if (uS.isRightChar(uttline,s-1,'#',&dFnt,MBF)) ; // Remove to diss-allow # after '.'
		else if (uS.isRightChar(uttline,s-1,'(',&dFnt,MBF) && uS.isPause(uttline,s-1,NULL,&r)) ; // Remove to diss-allow (.) pause after '.'
		else {
			if (DelFound && ts->UTD == 1) {
				if (DelFound == 1) {
					DelFound = 3;
					if (!isR2L) {
						if (s >= 3) {
							check_err(36,s-3,s-3,ln+lineno);
						} else {
							check_err(36,s-1,s-1,ln+lineno);
						}
					}
				} if (DelFound == 2)
					DelFound = 0;
			}
			sq = FALSE;
		}
		check_CheckBrakets(uttline,s-1,&sb,&pb,&ab,&cb,&anb);
		e = s;
		if (*st == '&' && (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE))) {
			if (uttline[e] == '+' && (uttline[e+1] < '!' || uttline[e+1] > '?')) {
				st++;
				*st = uttline[e];
				e++;
			}
			while ((*++st=uttline[e]) != EOS && !isSpace(*st) && *st != '\n' && *st != '>' && *st != '[' &&
					*st != ',' && *st != '+' && *st != '.' && *st != '?' && *st != '!') {
				if (is_italic(utterance->attLine[e])) {
					check_err(102,e-1,e-1,ln+lineno);
				}
				e++;
				if (e-s >= BUFSIZ) {
					fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, ln+lineno);
					fprintf(stderr,"%s\n", uttline+s);
					fprintf(stderr,"Word is too long.\n");
					check_clean_headerr(); check_clean_speaker(headsp); check_clean_IDsp(check_idsp);
					check_clean__t(headt); check_clean__t(maint);
					check_clean__t(codet);
					cleanupLanguages();
					cutt_exit(0);
				}
			}
		} else {
			while ((*++st=uttline[e]) != EOS &&
				   (!uS.isskip(uttline,e,&dFnt,MBF) || (uS.isRightChar(uttline, e, '<', &dFnt, MBF) && word[0] == '+') || sq || hidenc || (uS.isRightChar(uttline, e, '.', &dFnt, MBF) && ts->UTD == 0))) {
				if (is_italic(utterance->attLine[e])) {
					check_err(102,e-1,e-1,ln+lineno);
				}
				if (uS.isRightChar(uttline, e, '(', &dFnt, MBF) && uS.isPause(uttline, e, NULL, &r)) {
					if (!uS.isRightChar(word, 0, '[', &dFnt, MBF) && !check_isGrouping(word, uttline, e))
						check_err(57,e,e,ln+lineno);
				}
				if (uS.IsUtteranceDel(uttline, e) == 2)
					break;
				e++;
				if (e-s >= BUFSIZ) {
					fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, ln+lineno);
					fprintf(stderr,"%s\n", uttline+s);
					fprintf(stderr,"%s is too long.\n", (sq ? "Item in []" : "Word"));
					check_clean_headerr(); check_clean_speaker(headsp); check_clean_IDsp(check_idsp);
					check_clean__t(headt); check_clean__t(maint);
					check_clean__t(codet);
					cleanupLanguages();
					cutt_exit(0);
				}
				if (hidenc && uttline[e-1] == HIDEN_C) { hidenc = FALSE; st++; break; }
				if (!hidenc && uttline[e] == HIDEN_C) { st++; break; }
				if (uttline[e-1] == ']' && e-1 == isReplaceFound) {
					st--;
					break;
				} else if (uS.isRightChar(uttline, e-1, ']', &dFnt, MBF)) {
					sb--;
					st++;
					break;
				}
				if (!sq)
					check_CheckBrakets(uttline,e-1,&sb,&pb,&ab,&cb,&anb);
				else if (uS.isRightChar(uttline, e-1, '[', &dFnt, MBF)) 
					sb++;
			}
			if (uttline[e] == '>' && uttline[e+1] == '[') {
				check_err(93,e+1,e+1,ln+lineno);
			}
			if (uttline[e-1] == ']' && uttline[e] == '[') {
				check_err(93,e,e,ln+lineno);
			}
			if (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) {
				if (uS.IsUtteranceDel(uttline, e)) {
					if (uS.atUFound(uttline,e,&dFnt,MBF)) {
						e++;
						while ((*++st=uttline[e]) != EOS) {
							if (isSpace(*st) ||
								uS.isRightChar(uttline, e, '<', &dFnt, MBF) || uS.isRightChar(uttline, e, '>', &dFnt, MBF) || 
								uS.isRightChar(uttline, e, '[', &dFnt, MBF) || uS.isRightChar(uttline, e, ']', &dFnt, MBF))
								break;
							if (is_italic(utterance->attLine[e])) {
								check_err(102,e-1,e-1,ln+lineno);
							}
							e++;
						}
					}
				}
			} else if (!strcmp(ts->code, DATEOF)) {
				if (isSpace(*st) || *st == '\n')
					;
				else {
					while ((*++st=uttline[e]) != EOS) {
						if (isSpace(*st) || *st == '\n')
							break;
						if (is_italic(utterance->attLine[e])) {
							check_err(102,e-1,e-1,ln+lineno);
						}
						e++;
					}
				}
			}
		}
		te = e;
		isSpecialDelimFound = FALSE;
		if (*word == '+' || *word == '-') {
			if ((uS.IsUtteranceDel(uttline, e) || uttline[e] == ',' || uttline[e] == '<') &&
						!isalnum((unsigned char)uttline[e-1]) && uttline[e-1] > 0 && uttline[e-1] < 127) {
				for (te++; uS.IsUtteranceDel(uttline, te) || 
										uttline[te] == ',' || uttline[te] == '<'; te++) {
					*++st = uttline[te];
					if (is_italic(utterance->attLine[te])) {
						check_err(102,te-1,te-1,ln+lineno);
					}
				}
				st++;
			}
			*st = EOS;

			if (!strncmp(utterance->speaker, "%mor", 4) && uS.IsUtteranceDel(uttline, e) && check_lastSpDelim[0] != EOS && strcmp(check_lastSpDelim, word)) {
				check_err(94,s-1,te-1,ln+lineno);
			}
			if (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE))
				strcpy(check_lastSpDelim, word);
			isSpecialDelimFound = TRUE;
		} else
			*st = EOS;
		//detecting pause markers before retrace markers-BEG
		if (strcmp(word, "[/]") == 0 || strcmp(word, "[//]") == 0 || strcmp(word, "[///]") == 0 ||
			strcmp(word, "[/-]") == 0) {
			if (isPauseFound && pauseBeg != pauseEnd && e > pauseBeg) {
				check_err(159,pauseBeg-1,e-1,ln+lineno);
			}
		}
		pauseBeg = 0;
		pauseEnd = 0;
		isPauseFound = FALSE;
		if (uS.isRightChar(word,0,'(',&dFnt,MBF) && uS.isPause(word,0,NULL,&r)) {
			pauseBeg = s;
			pauseEnd = e;
			isPauseFound = TRUE;
		}
		//detecting pause markers before retrace markers-END
		if (word[0] == '[' && word[1] == ':' && word[2] == ' ') {
			isReplaceFound = e - 1;
			e = s + 1;
		}
		if (uS.IsUtteranceDel(word, 0) == 2) {
			if (ts->UTD == -1) {
				check_err(48,s-1,s-1,ln+lineno);
			} else if (ts->UTD == 1) {
				if (DelFound == 1)
					check_err(50,s-1,s-1,ln+lineno);
				DelFound = 2;
			}
		}
		if (!uS.isRightChar(word,0,'[',&dFnt,MBF) && !uS.isRightChar(word,0,'+',&dFnt,MBF) &&
			!uS.isRightChar(word,0,'-',&dFnt,MBF) && !uS.isRightChar(word,0,'#',&dFnt,MBF) &&
			!uS.IsUtteranceDel(word, 0))
			isTextFound = TRUE;
		else if (uS.isRightChar(word,0,'[',&dFnt,MBF) && uS.isRightChar(word,1,'^',&dFnt,MBF) &&
				 uS.isRightChar(word,2,' ',&dFnt,MBF))
			isTextFound = TRUE;

		if (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) {
			if (anb && !uS.isRightChar(word,0,'[',&dFnt,MBF)) {
				check_err(51,s-1,s-1,ln+lineno);
			}
			anb = FALSE;
			if (pb > lpb) check_err(55,s-1,te-1,ln+lineno);
			else if (pb < lpb) check_err(56,s-1,te-1,ln+lineno);
		}

		if (!isSpace(uttline[e]) && !uS.isskip(uttline, e, &dFnt, MBF)) {
			res = 0;
//			if (dFnt.isUTF) {
				res = uS.HandleCAChars(uttline+e, &matchType);
				if (res) {
					if (matchType != CA_APPLY_CLTOPSQ && matchType != CA_APPLY_CLBOTSQ && matchType != NOTCA_GROUP_END &&
						matchType != CA_SUDDEN_STOP && matchType != NOTCA_SIGN_GROUP_END && uS.IsUtteranceDel(uttline, e) != 2)
						res = 0;
				}
//			}
			if (res == 0) {
				if (uttline[e] && uttline[e+1] &&
					(*utterance->speaker == '*' /*(*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) */ ||
					 (uttline[e+1] != '\'' && uttline[e+1] != '"' && !isdigit((unsigned char)uttline[e+1])))) {
					if (uttline[e] == HIDEN_C && e > 0) {
						check_err(19,e-1,e-1,ln+lineno);
					} else {
						check_err(19,e,e,ln+lineno);
					}
				}
			}
		} 
//		if (dFnt.isUTF) {
			for (r=0; word[r]; ) {
				res = uS.HandleCAChars(word+r, &matchType); // up-arrow
				if (res) {
/* 2019-04-17 leave this to CHATTER
					if (matchType == CA_APPLY_RISETOHIGH || matchType == CA_APPLY_RISETOMID || matchType == CA_APPLY_LEVEL ||
						matchType == CA_APPLY_FALLTOMID || matchType == CA_APPLY_FALLTOLOW) {
						if (isFasterMatched != -1 || isSlowerMatched != -1 || isCreakyMatched != -1 || isUnsureMatched != -1 ||
							isSofterMatched != -1 || isLouderMatched != -1 || isLoPitchMatched!= -1 || isHiPitchMatched != -1||
							isSmileMatched != -1  || isWhisperMatched != -1|| isYawnMatched != -1   || isSingingMatched != -1||
							isPreciseMatched != -1|| isBreathyMatched != -1|| isLeftArrowMatched != -1) {
							if (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE))
								check_err(145,s+r-1,s+r+res-2,ln+lineno);
						}
					}
 */
					if (matchType == CA_APPLY_RISETOHIGH || matchType == CA_APPLY_RISETOMID || matchType == CA_APPLY_LEVEL ||
						matchType == CA_APPLY_FALLTOMID || matchType == CA_APPLY_FALLTOLOW || matchType == CA_APPLY_UNMARKEDENDING) {
						CADelFound = TRUE;
						if (isBulletFound) {
							check_err(118,s-1,e-1,ln+lineno);
						}
					} else if (matchType == CA_APPLY_FASTER) {
						if (isFasterMatched != -1)
							isFasterMatched = -1;
						else
							isFasterMatched = s+r;
					} else if (matchType == CA_APPLY_SLOWER) {
						if (isSlowerMatched != -1)
							isSlowerMatched = -1;
						else
							isSlowerMatched = s+r;
					} else if (matchType == CA_APPLY_CREAKY) {
						if (isCreakyMatched != -1)
							isCreakyMatched = -1;
						else
							isCreakyMatched = s+r;
					} else if (matchType == CA_APPLY_UNSURE) {
						if (isUnsureMatched != -1)
							isUnsureMatched = -1;
						else
							isUnsureMatched = s+r;
					} else if (matchType == CA_APPLY_SOFTER) {
						if (isSofterMatched != -1)
							isSofterMatched = -1;
						else
							isSofterMatched = s+r;
					} else if (matchType == CA_APPLY_LOUDER) {
						if (isLouderMatched != -1)
							isLouderMatched = -1;
						else
							isLouderMatched = s+r;
					} else if (matchType == CA_APPLY_LOW_PITCH) {
						if (isLoPitchMatched != -1)
							isLoPitchMatched = -1;
						else
							isLoPitchMatched = s+r;
					} else if (matchType == CA_APPLY_HIGH_PITCH) {
						if (isHiPitchMatched != -1)
							isHiPitchMatched = -1;
						else
							isHiPitchMatched = s+r;
					} else if (matchType == CA_APPLY_SMILE_VOICE) {
						if (isSmileMatched != -1)
							isSmileMatched = -1;
						else
							isSmileMatched = s+r;
					} else if (matchType == CA_APPLY_WHISPER) {
						if (isWhisperMatched != -1)
							isWhisperMatched = -1;
						else
							isWhisperMatched = s+r;
					} else if (matchType == CA_APPLY_YAWN) {
						if (isYawnMatched != -1)
							isYawnMatched = -1;
						else
							isYawnMatched = s+r;
					} else if (matchType == CA_APPLY_SINGING) {
						if (isSingingMatched != -1)
							isSingingMatched = -1;
						else
							isSingingMatched = s+r;
					} else if (matchType == CA_APPLY_PRECISE) {
						if (isPreciseMatched != -1)
							isPreciseMatched = -1;
						else
							isPreciseMatched = s+r;
					} else if (matchType == CA_BREATHY_VOICE) {
						if (isBreathyMatched != -1)
							isBreathyMatched = -1;
						else
							isBreathyMatched = s+r;
					} else if (matchType == NOTCA_GROUP_START) {
						gm++;
					} else if (matchType == NOTCA_GROUP_END) {
						gm--;
					} else if (matchType == NOTCA_SIGN_GROUP_START) {
						sgm++;
					} else if (matchType == NOTCA_SIGN_GROUP_END) {
						sgm--;
					} else if (matchType == NOTCA_OPEN_QUOTE) {
						qt++;
					} else if (matchType == NOTCA_CLOSE_QUOTE) {
						qt--;
					} else if (matchType == NOTCA_LEFT_ARROW_CIRCLE) {
						if (isLeftArrowMatched != -1)
							isLeftArrowMatched = -1;
						else
							isLeftArrowMatched = s+r;
					}
					r += res;
				} else
					r++;
			}
//		}


		isLcodeFound = FALSE;
		tisNumberShouldBeHere = check_isNumberShouldBeHere;
//		if (!nomap) uS.uppercasestr(word, &dFnt, MBF);
		if (!strcmp(word, "/")) {
			check_err(48,s-1,e-1,ln+lineno);
		}
		if (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) {
			j = strlen(word) - 1;
			if (!check_isCAFound && j > 0 && word[0] == '(' && word[j] == ')') {
				if (!uS.isPause(word, 1, NULL, NULL) && strchr(word+1, '(') == NULL)
					check_err(155,s-1,te-1,ln+lineno);
			}
			if (word[0] == '#')
				isPoundSymFound = TRUE;
			else {
				if (word[0] == '[' && word[1] != '+' && word[1] != '^' && isPoundSymFound) {
					check_err(71,s-1,e-1,ln+lineno);
				}
				isPoundSymFound = FALSE;
			}
			if (word[0] == '(' && uS.isPause(word,0,NULL,&r) && DelFound) {
				check_err(72,s-1,e-1,ln+lineno);
			}
			if (uS.isRightChar(word, 0, '[', &dFnt, MBF)) {
				if (word[1] != '+' && (DelFound || CADelFound)) {
					check_err(72,s-1,te-1,ln+lineno);
				}
				if (word[1] == '+' && !DelFound) {
					check_err(75,s-1,te-1,ln+lineno);
				}
				if (*utterance->speaker == '*' && word[1] == '+' && isBulletFound) {
					check_err(108,s-1,te-1,ln+lineno);
				}
			}
			if (word[0] == '[' && word[1] == '-')
				isPreLanguageCodeFound = TRUE;
			check_CheckLanguageCodes(word, s, ln+lineno, &isLcodeFound, &isGLcodeFound);
		}

		if (W_CHECK_COUNT(check_GenOpt) && ts->WORDCHECK && word[0] != HIDEN_C) {
			check_CheckWords(word, s, ln+lineno, ts);
		}

		if (*utterance->speaker == '*' || uS.partcmp(utterance->speaker,"%wor:",FALSE,FALSE)) {
			check_removeCAChars(word);
		}
		if (word[0]=='+' && (word[1] == '"' || word[1] == '^' || word[1] == '<' || word[1] == ',' || word[1] == '+') && word[2]==EOS) {
			if (FirstWordFound || isPreLanguageCodeFound) {
				check_err(78,s-1,e-1,ln+lineno);
			}
		}
		if (word[0] == HIDEN_C) {
			if (check_isMultiBullets) {
				if (DelFound)
					isBulletFound = TRUE;
			} else
				isBulletFound = TRUE;
			anyBulletFound = TRUE;
			if (*utterance->speaker == '*' && check_isPlayBullet(word)) {
				if (!isTextBetweenBulletsFound && (!check_isMultiBullets || !isLastItemBullet || !check_lastBulletOnTier(uttline+e))) {
					check_err(73,s-1,e-1,ln+lineno);
				}
				if (!DelFound && !check_isMultiBullets) {
					int k;
					for (k=e; uttline[k] != EOS && uS.isskip(uttline, k, &dFnt, MBF) && !uS.IsUtteranceDel(uttline, k); k++) {
						if (uttline[k] == '\n')
							break;
					}
					if (uttline[k] != '\n') {
						check_err(81,s-1,e-1,ln+lineno);
					}
				}
			}
			if (hidenc) {
				check_err(59,s-1,s-1,ln+lineno);
			}
			if (!check_isPlayBullet(word)) {
				isTextBetweenBulletsFound = TRUE;
			} else if ((utterance->speaker[0] == '*') && (!check_isMultiBullets || check_lastBulletOnTier(uttline+e))) {
				if ((res=check_checkBulletsConsist(&FirstBulletFound, word)) != 0)
					check_err(res,s-1,s-1,ln+lineno);
				isTextBetweenBulletsFound = FALSE;
			}
			isLastItemBullet = TRUE;
		} else if (isdigit((unsigned char)*word) && ts->CSUFF) {
			isLastItemBullet = FALSE;
			if (*word != '0' || isdigit((unsigned char)*(word+1))) {
				if (check_matchnumber(ts->tplate,word) && !check_isNumberShouldBeHere)
					check_err(38,s-1,e-1,ln+lineno);
				else {
					FirstWordFound = TRUE;
					isTextBetweenBulletsFound = TRUE;
				}
			} else {
				FirstWordFound = TRUE;
				isTextBetweenBulletsFound = TRUE;
			}
		} else {
			isLastItemBullet = FALSE;
			if (*word == '&') {
				for (r=1; word[r]; r++) {
					if (word[r]=='(') {check_err(42,s-1,e-1,ln+lineno);break;}
				}
				if (word[1] == '=' && word[2] == EOS) {
					check_err(146,s-1,e-1,ln+lineno);
				}
			}
			if (isalpha((unsigned char)*(word+1)) || *(word+1) < 0 || *(word+1) > 126)
				HandleParans(word,0,0);
			if (uS.isRightChar(word, 0, '[', &dFnt, MBF)) {
				for (r=1; word[r]; r++) {
					if (word[r]=='\n') {
						check_err(106,s-1,e-1,ln+lineno);
						break;
					}
				}
				if (*utterance->speaker == '*') {
					if (utterance->line[s-1] == '[' && utterance->line[s] != '+' && utterance->line[s] != '-') {
						for (r=s-2; r >= 0; r--) {
							if (utterance->line[r] == ')') {
								for (j=r; j >= 0 && utterance->line[j] != '(' && !uS.isskip(utterance->line, j, &dFnt, MBF); j--) ;
								if (j < 0)
									check_err(150,j,r,ln+lineno);
							}
							if (!uS.isskip(utterance->line, r, &dFnt, MBF) || utterance->line[r] == ']')
								break;
							if (utterance->line[r] == '\n' || utterance->line[r] == '<' || utterance->line[r] == '>') {
							} else if (utterance->line[r] == ',') {
//								check_err(149,r,r,ln+lineno);
							} else if (!isSpace(utterance->line[r])) {
								check_err(149,r,r,ln+lineno);
							}
						}
					}
					if (!FirstWordFound) {
						if (word[1] == '<' || word[1] == '>' || word[1] == ':' || word[1] == '/' || word[1] == '"') {
							check_err(52,s-1,te-1,ln+lineno);
						} else if (word[1] != '-' && word[1] != '^') {
							check_err(73,s-1,te-1,ln+lineno);
						}
					} else if (word[1] == ':') {
						if (check_isReplacementError(uttline, s-1)) {
							check_err(141,s-1,te-1,ln+lineno);
						}

						for (j=2; isSpace(word[j]); j++) ;
						uS.isSqCodes(word+j, templineC3, &dFnt, TRUE);
						if (templineC3[0] == '0' || uS.mStricmp(templineC3, "xxx]") == 0 || uS.mStricmp(templineC3, "xx]") == 0 ||
							uS.mStricmp(templineC3, "yyy]") == 0 || uS.mStricmp(templineC3, "yy]") == 0||
							uS.mStricmp(templineC3, "www]") == 0 || uS.mStricmp(templineC3, "ww]") == 0) {
							check_err(158,s-1,te-1,ln+lineno);
						}

					}
				}
				if (uS.patmat(word,"[\\%fnt: *:*]")) {
					check_err(116,s-1,te-1,ln+lineno);
				}
				if ((isalpha((unsigned char)*word) || *word < 0 || *word > 126) && ts->CSUFF)
					check_CheckFixes(ts,word,ln+lineno,s,e);
				if ((r=check_matchplate(ts->tplate, word, 'D')))
					check_err(r,s-1,te-1,ln+lineno);
			} else if (*word != EOS) {
				if (word[0] != '+') {
					FirstWordFound = TRUE;
					isTextBetweenBulletsFound = TRUE;
				}
				if (ts->CSUFF == '\002') {
					if (isalpha((unsigned char)*word) || *word < 0 || *word > 126) {
						if (!check_CheckFixes(ts,word,ln+lineno,s,e)) {
							if ((r=check_matchplate(ts->tplate, word, 'D')))
								check_err(r,s-1,te-1,ln+lineno);
						}
					} else {
						if ((r=check_matchplate(ts->tplate, word, 'D')))
							check_err(r,s-1,te-1,ln+lineno);
					}
				} else {
					if ((r=check_matchplate(ts->tplate, word, 'D')))
						check_err(r,s-1,te-1,ln+lineno);
					if ((isalpha((unsigned char)*word) || *word < 0 || *word > 126) && ts->CSUFF)
						check_CheckFixes(ts,word,ln+lineno,s,e);
				}
			}
		}
		if (isLcodeFound)
			check_isNumberShouldBeHere = tisNumberShouldBeHere;
		s = e;
	}
	if (*utterance->speaker == '*' && !anyBulletFound && checkBullets) {
		check_err(110,-1,-1,lineno);
	}
	if (isGLcodeFound)
		check_isNumberShouldBeHere = isGNumberShouldBeHere;
	if (!DelFound && ts->UTD == 1 && !check_isCAFound)
		check_err(21,s-2,s-2,ln+lineno-1L);
	if (*utterance->speaker == '*') {
		if (isFasterMatched != -1) {
			t = uS.HandleCAChars(uttline+isFasterMatched, NULL);
			check_err(117, isFasterMatched-1, isFasterMatched+t-1, lineno);
		}
		if (isSlowerMatched != -1) {
			t = uS.HandleCAChars(uttline+isSlowerMatched, NULL);
			check_err(117, isSlowerMatched-1, isSlowerMatched+t-1, lineno);
		}
		if (isCreakyMatched != -1) {
			t = uS.HandleCAChars(uttline+isCreakyMatched, NULL);
			check_err(117, isCreakyMatched-1, isCreakyMatched+t-1, lineno);
		}
		if (isUnsureMatched != -1) {
			t = uS.HandleCAChars(uttline+isUnsureMatched, NULL);
			check_err(117, isUnsureMatched-1, isUnsureMatched+t-1, lineno);
		}
		if (isSofterMatched != -1) {
			t = uS.HandleCAChars(uttline+isSofterMatched, NULL);
			check_err(117, isSofterMatched-1, isSofterMatched+t-1, lineno);
		}
		if (isLouderMatched != -1) {
			t = uS.HandleCAChars(uttline+isLouderMatched, NULL);
			check_err(117, isLouderMatched-1, isLouderMatched+t-1, lineno);
		}
		if (isLoPitchMatched != -1) {
			t = uS.HandleCAChars(uttline+isLoPitchMatched, NULL);
			check_err(117, isLoPitchMatched-1, isLoPitchMatched+t-1, lineno);
		}
		if (isHiPitchMatched != -1) {
			t = uS.HandleCAChars(uttline+isHiPitchMatched, NULL);
			check_err(117, isHiPitchMatched-1, isHiPitchMatched+t-1, lineno);
		}
		if (isSmileMatched != -1) {
			t = uS.HandleCAChars(uttline+isSmileMatched, NULL);
			check_err(117, isSmileMatched-1, isSmileMatched+t-1, lineno);
		}
		if (isWhisperMatched != -1) {
			t = uS.HandleCAChars(uttline+isWhisperMatched, NULL);
			check_err(117, isWhisperMatched-1, isWhisperMatched+t-1, lineno);
		}
		if (isYawnMatched != -1) {
			t = uS.HandleCAChars(uttline+isYawnMatched, NULL);
			check_err(117, isYawnMatched-1, isYawnMatched+t-1, lineno);
		}
		if (isSingingMatched != -1) {
			t = uS.HandleCAChars(uttline+isSingingMatched, NULL);
			check_err(117, isSingingMatched-1, isSingingMatched+t-1, lineno);
		}
		if (isPreciseMatched != -1) {
			t = uS.HandleCAChars(uttline+isPreciseMatched, NULL);
			check_err(117, isPreciseMatched-1, isPreciseMatched+t-1, lineno);
		}
		if (isBreathyMatched != -1) {
			t = uS.HandleCAChars(uttline+isBreathyMatched, NULL);
			check_err(117, isBreathyMatched-1, isBreathyMatched+t-1, lineno);
		}
		if (isLeftArrowMatched != -1) {
			t = uS.HandleCAChars(uttline+isLeftArrowMatched, NULL);
			check_err(117, isLeftArrowMatched-1, isLeftArrowMatched+t-1, lineno);
		}
	}
	if (!isTextFound) check_err(70,-1,-1,lineno);
	if (sb > 0) check_err(22,-1,-1,lineno);
	if (sb < 0) check_err(23,-1,-1,lineno);
	if (ab > 0) check_err(24,-1,-1,lineno);
	if (ab < 0) check_err(25,-1,-1,lineno);
	if (cb > 0) check_err(26,-1,-1,lineno);
	if (cb < 0) check_err(27,-1,-1,lineno);
/* 11-02-98
	if (pb > 0) check_err(28,-1,-1,lineno);
	if (pb < 0) check_err(29,-1,-1,lineno);
*/
	if (gm > 0) check_err(128,-1,-1,lineno);
	if (gm < 0) check_err(129,-1,-1,lineno);
	if (sgm > 0) check_err(130,-1,-1,lineno);
	if (sgm < 0) check_err(131,-1,-1,lineno);
	if (qt > 0) check_err(136,-1,-1,lineno);
	if (qt < 0) check_err(137,-1,-1,lineno);
}

static char check_ID(CHECK_TIERS *ts) {
	int r, s = 0, e = 0, cnt;
	long ln = 0L;
	char word[BUFSIZ], *st, spkr[BUFSIZ+1];
	CHECK_TIERS *tts;

	cnt = 0;
	word[0] = EOS;
	if (ts->IGN)
		return(TRUE);
	while (uttline[s] != EOS) {
		if (uttline[s] == '\n')
			ln += 1;
		if (!uS.isskip(uttline, s, &dFnt, MBF))
			break;
		s++;
	}
	if (uttline[s] == EOS)
		return(TRUE);
	if (uttline[s] == '|')
		check_err(62,s,s,ln+lineno);
	while (1) {
		st = word;
		if ((*st=uttline[s]) == '|') {
			e = s + 1;
		} else {
			while ((*st=uttline[s]) == '|' || uS.isskip(uttline, s, &dFnt, MBF)) {
				if (uttline[s] == '\n')
					ln += 1;
				if (uttline[s] == EOS)
					break;
				if (uttline[s] == '|')
					cnt++;
				s++;
			}
			if (*st == EOS)
				break;
			e = s + 1;
			while ((*++st=uttline[e]) != EOS) {
				e++;
				if (uttline[e-1] == '|')
					break;
			}
		}
		*st = EOS;
//		uS.uppercasestr(word, &dFnt, MBF);
		if (isalnum((unsigned char)*(word+1)) || (*(word+1) < 1 && *(word+1) != EOS) || *(word+1) > 126)
			HandleParans(word,0,0);
		if (cnt == 0) {
			check_isLangMatch(word, ln, s, 122, FALSE);
			if (word[0] == EOS)
				check_err(62,s,e-2,ln+lineno);
		} else if (cnt == 1) {
			if (word[0] == EOS)
				check_err(63,s,e-2,ln+lineno);
		} else if (cnt == 2) {
			if (word[0] == EOS)
				check_err(12,s,e-2,ln+lineno);
			else if (check_matchSpeaker(headsp, word, TRUE) == NULL) {
				check_err(18,s,e-2,ln+lineno);
			} else if (check_matchIDsp(check_idsp, word)) {
				check_err(13,s,e-2,ln+lineno);
			}
			check_addIDdsp(word);
			strcpy(spkr, word);
		} else if (cnt == 3) {
			if ((tts=check_FindTier(headt,IDOF)) != NULL) {
				check_Age(tts, NULL, uttline, s, '|');
			}
		} else if (cnt == 4) {
			if (word[0] != EOS) {
				if (strcmp(word, "male") && strcmp(word, "female"))
					check_err(64,s,e-2,ln+lineno);
			}
		} else if (cnt == 5) {
		} else if (cnt == 6) {
			if ((tts=check_FindTier(headt,IDOF)) != NULL) {
				check_ses(tts, NULL, uttline, s, '|');
			}
		} else if (cnt == 7) {
			if (word[0] == EOS)
				check_err(12,s,e-2,ln+lineno);
			else if ((tts=check_FindTier(headt,PARTICIPANTS)) != NULL) {
				if (check_matchplate(tts->tplate, word, 'D'))
					check_err(15,s,e-2,ln);
			}
			if (check_matchRole(spkr, headsp, word))
				check_err(142,s,e-2,ln+lineno);
		} else if (cnt == 8) {
		} else if (cnt == 9) {
		} else if ((r=check_matchplate(ts->tplate, word, 'D')))
			check_err(r,s-1,e-2,ln+lineno);

		s = e;
		cnt++;
	}
	if (cnt != 10) {
		check_err(143,s,s,ln+lineno);
	}
	return(TRUE);
}

static int check_checktier(char *s) {
	struct tier *p;

	if (!chatmode) return(TRUE);
	strcpy(templineC,s);
//	uS.uppercasestr(templineC, &dFnt, MBF);
	for (p=defheadtier; p != NULL; p=p->nexttier)
		if (uS.partcmp(templineC,p->tcode,p->pat_match, TRUE)) break;
	if (p == NULL) {
		for (p=headtier; p != NULL; p=p->nexttier) {
			if (uS.partcmp(templineC,p->tcode,p->pat_match, TRUE)) {
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
				if (j > i) strcpy(s+i, s+j);
			}
		} else {
			return(FALSE);
		}
	} else {
		if ((*s == '*' && tcs) || (*s == '@' && tch) || (*s == '%' && tct)) {
			return(FALSE);
		}
	}
	return(TRUE);
}
/*
static char check_isDoubleScope(char *line, int pos, char *code) {
	long ang, isword;
	long temp, sqCnt;

	if (line[pos] == '[') {
		ang = 0;
		isword = 0;
		for (pos--; pos >= 0; pos--) {
			if (line[pos] == ']') {
				sqCnt = 0;
				temp = pos;
				for (pos--; (!uS.isRightChar(line, pos, '[', &dFnt, MBF) || sqCnt > 0) && pos >= 0; pos--) {
					if (uS.isRightChar(line, pos, ']', &dFnt, MBF))
						sqCnt++;
					else if (uS.isRightChar(line, pos, '[', &dFnt, MBF))
						sqCnt--;
				}
				if (pos >= 0) {
					line[temp] = EOS;
					uS.isSqCodes(line+pos+1, templineC2, &dFnt, TRUE);
					line[temp] = ']';
					if (uS.mStricmp(code, templineC2) == 0) {
						check_err(???, pos, temp, lineno);
					}
				} else
					pos = temp;
			} else if (line[pos] == '>')
				ang++;
			else if (line[pos] == '<')
				ang--;
			else if (uS.isskip(line,pos,&dFnt,MBF) && ang <= 0 && isword > 0)
				break;
			else if (!uS.isskip(line,pos,&dFnt,MBF) && isword <= 0)
				isword++;
		}
	}
	return(FALSE);
}
*/
static char check_ParseScope(char *line) {
	int pos, temp, sqCnt;
	char sq, bullet, isWordFound;

	sq = FALSE;
//	if (strncmp(line, "put", 3) == 0)
//		bullet = FALSE;
	bullet = FALSE;
	isWordFound = FALSE;
	pos = strlen(line) - 1;
	while (pos >= 0) {
		if (line[pos] == ']')
			sq = TRUE;
		else if (line[pos] == '[')
			sq = FALSE;
		else if (line[pos] == HIDEN_C) {
			bullet = !bullet;
		}

		if (uS.isRightChar(line, pos, ']', &dFnt, MBF)) {
			sqCnt = 0;
			temp = pos;
			for (pos--; (!uS.isRightChar(line, pos, '[', &dFnt, MBF) || sqCnt > 0) && pos >= 0; pos--) {
				if (uS.isRightChar(line, pos, ']', &dFnt, MBF))
					sqCnt++;
				else if (uS.isRightChar(line, pos, '[', &dFnt, MBF))
					sqCnt--;
				if (line[pos] == '[')
					sq = FALSE;
			}
			if (line[pos] == '[')
				sq = FALSE;
			if (pos >= 0) {
				line[temp] = EOS;
				uS.isSqCodes(line+pos+1, templineC3, &dFnt, TRUE);
				line[temp] = ']';
				if (isWordFound == FALSE && templineC3[0] == '/') {
					temp = (temp - pos) + 1;
					check_err(119, pos, pos+temp-1, lineno);
				}
/* 2010-07-20
				if (check_isDoubleScope(line, pos, templineC3)) {
					return(FALSE);
				}
*/
			} else
				pos = temp;
		}
		if (sq == FALSE && bullet == FALSE && !uS.isskip(line,pos,&dFnt,MBF) &&
			line[pos] != HIDEN_C && line[pos] != '/' && !isUttDel(line+pos))
			isWordFound = TRUE;
		pos--;
	}
	return(TRUE);
}

static char check_SpecialDelim(char *ch, int index) {
	if (ch[index] == ';' && !uS.isskip(ch, index, &dFnt, MBF)) {
		return(TRUE);
	} else if (ch[index] == ',' && !uS.isskip(ch, index, &dFnt, MBF)) {
		return(TRUE);
	}
	return(FALSE);
}

static void check_cleanUpLine(const char *sp, char *ch) {
	int index;

	index = 0;
	while (ch[index] != EOS) {
		if (uS.isRightChar(ch, index, '[', &dFnt, MBF)) {
			for (index++; ch[index] != EOS && !uS.isRightChar(ch, index, ']', &dFnt, MBF); index++) ;
			if (ch[index] != EOS)
				index++;
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
				uS.shiftright(ch+index, 1);
				ch[index] = ' ';
				index += 3;
			} else
				index += 2;
		} else if (ch[index] == '.') {
			if (!uS.isPlusMinusWord(ch, index) && !uS.atUFound(ch, index, &dFnt, MBF) && !uS.isPause(ch,index,NULL,NULL) &&
				!isSpace(ch[index-1]) && (!isdigit(ch[index-1]) || (*sp == '*' && !isdigit(ch[index+1]))) && index > 0) {
				uS.shiftright(ch+index, 1);
				ch[index] = ' ';
				index += 2;
			} else
				index++;
			while (ch[index] == '.' || ch[index] == '?' || ch[index] == '!')
				index++;
		} else if (ch[index] == '?' || ch[index] == '!' || check_SpecialDelim(ch, index)) {
			if (!uS.isPlusMinusWord(ch, index) && !uS.atUFound(ch, index, &dFnt, MBF) &&
				!isSpace(ch[index-1]) && (!isdigit(ch[index-1]) || *sp == '*') && index > 0) {
				uS.shiftright(ch+index, 1);
				ch[index] = ' ';
				index += 2;
			} else
				index++;
			while (ch[index] == '.' || ch[index] == '?' || ch[index] == '!')
				index++;
		} else
			index++;
	}
}

static void check_isMissiingID(void) {
	SPLIST *tp;

	for (tp=headsp; tp != NULL; tp = tp->nextsp) {
		if (tp->hasID == FALSE) {
			if (totalFilesNum > MINFILESNUM && !stin) { // 2019-04-18 TotalNumFiles
				fprintf(stderr, "\r    	     \r");
			}
			fprintf(fpout,"*** File \"%s\": line 3.\n", oldfname);
			fprintf(fpout,"Missing @ID header for speaker *%s.\n", tp->sp);
		}
	}
}

static void check_addUDerr(char *s) {
	int i;

	struct UDErr *tp;

	if (*s == EOS)
		return;
	strcpy(templineC3, s);
	for (i=1; templineC3[i] != EOS; i++) {
		if (!isalnum(templineC3[i]))
			break;
	}
	templineC3[i] = EOS;
	if (check_UDerr == NULL) {
		if ((check_UDerr=NEW(struct UDErr)) == NULL) check_overflow();
		tp = check_UDerr;
	} else {
		for (tp=check_UDerr; tp->nextUDerr != NULL; tp=tp->nextUDerr) {
			if (!uS.mStricmp(tp->code, templineC3)) {
				return;
			}
		}
		if (!uS.mStricmp(tp->code, templineC3)) {
			return;
		}
		if (tp->nextUDerr == NULL) {
			if ((tp->nextUDerr=NEW(struct UDErr)) == NULL)
				check_overflow();
			tp = tp->nextUDerr;
		}
	}
	tp->nextUDerr = NULL;
	tp->code = check_mkstring(templineC3);
}

static char check_CheckRest(void) {
	int  i, j;
	char t, isSkip, *ext;
	BE_TIERS *head_bg, *tb;
	CHECK_CODES *headcodes, *tc;
	char CodeLegalPos;
	CHECK_TIERS *ts;
	SPLIST *tp;

	isBulletsFound = FALSE;
	isMediaHeaderFound = FALSE;
	check_lastSpDelim[0] = EOS;
	head_bg = NULL;
	headcodes = NULL;
	CodeLegalPos = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		isSkip = FALSE;
		if (!stout) {
			if (lineno % 100 == 0) {
				if (totalFilesNum <= MINFILESNUM || stin)
					fprintf(stderr, "\r%ld ", lineno);
			}
		}
		if (*utterance->speaker == '@') {
			if (uS.isUTF8(utterance->speaker) || uS.isInvisibleHeader(utterance->speaker))
				continue;
			strcpy(templineC, utterance->speaker);
//			uS.uppercasestr(templineC, &dFnt, MBF);
			check_CleanTierNames(templineC);
			if (uS.patmat(templineC, "@Bg")) {
				if ((tb=NEW(BE_TIERS)) == NULL) check_overflow();
				if ((tb->pattern=(char *)malloc(strlen(uttline)+1)) == NULL)
					check_overflow();
				strcpy(tb->pattern, uttline);
				tb->nextone = head_bg;
				head_bg = tb;
			} else if (uS.patmat(templineC, "@Eg")) {
				if (head_bg == NULL) {
					check_err(46,-1,-1,lineno);
				} else {
					BE_TIERS *to = NULL;
					for (tb=head_bg; tb != NULL; tb=tb->nextone) {
						if (!strcmp(tb->pattern, uttline))
							break;
						to = tb;
					}
					if (tb == NULL) {
						check_err(46,-1,-1,lineno);
					} else if (to != NULL) {
						if (tb == head_bg) head_bg = tb->nextone;
						else to->nextone = tb->nextone;
						free(tb->pattern);
						free(tb);
					} else if (tb == head_bg) {
						head_bg = tb->nextone;
						free(tb->pattern);
						free(tb);
					}
				}
			} else if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE)) {
				int langCnt;
				char isIPAFound = FALSE, isFirstLang;
				char old_code[20];

				isSkip = TRUE;
				i = 0;
				langCnt = 0;
				isFirstLang = TRUE;
				while (uttline[i]) {
					for (; uS.isskip(uttline, i, &dFnt, MBF) || uttline[i] == '\n'; i++) ;
					if (uttline[i] == EOS)
						break;
					for (j=i; !uS.isskip(uttline, j, &dFnt, MBF) && uttline[j] != '=' && uttline[j] != '\n' && uttline[j] != EOS; j++) ;
					t = uttline[j];
					uttline[j] = EOS;
					if (check_isNumbersLang(uttline+i)) {
						if (isFirstLang)
							check_isNumberShouldBeHere = TRUE;
					} else if (!strcmp(uttline+i, "ar") || !strcmp(uttline+i, "ara")) {
						check_isArabic = TRUE;
					} else if (!strcmp(uttline+i, "he") || !strcmp(uttline+i, "heb")) {
						check_isHebrew = TRUE;
					} else if (!strcmp(uttline+i, "de") || !strcmp(uttline+i, "deu")) {
						check_isGerman = TRUE;
					} else if (!strcmp(uttline+i, "CA") || !strcmp(uttline+i, "CA-Unicode")) {
						check_isCAFound = TRUE;
					} else if (!strcmp(uttline+i, "IPA")) {
						isIPAFound = TRUE;
					}
					isFirstLang = FALSE;
					if (langCnt < NUMLANGUAGESINTABLE) {
						strncpy(check_LanguagesTable[langCnt], uttline+i, 8);
						check_LanguagesTable[langCnt][8] = EOS;
						langCnt++;
					}
					if (check_isTwoLetterCode(uttline+i)) {
						strncpy(check_fav_lang, uttline+i, 19);
						check_fav_lang[19] = EOS;
						if (!getLanguageCodeAndName(check_fav_lang, TRUE, NULL))
							check_fav_lang[0] = EOS;
						uttline[j] = t;
						check_err(120,i,j-1,lineno);
						uttline[j] = EOS;
					} else {
						strncpy(check_fav_lang, uttline+i, 19);
						check_fav_lang[19] = EOS;
						strcpy(old_code, check_fav_lang);
						if (!getLanguageCodeAndName(check_fav_lang, TRUE, NULL)) {
							uttline[j] = t;
							check_err(121,i,j-1,lineno);
							uttline[j] = EOS;
						} else {
							if (strcmp(old_code, check_fav_lang)) {
								uttline[j] = t;
								check_err(120,i,j-1,lineno);
								uttline[j] = EOS;
							}
						}
					}
					uttline[j] = t;
					if (uttline[j] == '=')
						for (; !uS.isskip(uttline, j, &dFnt, MBF) && uttline[j] != '\n' && uttline[j] != EOS; j++) ;
					i = j;
				}
				if (check_isCAFound && isIPAFound)
					check_err(103,-1,-1,lineno);
				else {
/*
					if (check_isCAFound) {
						if (strcmp(dFnt.fontName, "CAfont") && strcmp(dFnt.fontName, "Ascender Uni Duo"))
							check_err(104,-1,-1,lineno);
					}
					if (isIPAFound) {
						if (strcmp(dFnt.fontName, "Charis SIL")) 
							check_err(105,-1,-1,lineno);
					}
*/
				}
			} else if (uS.partcmp(utterance->speaker,"@New Language:",FALSE,FALSE)) {
				isSkip = TRUE;
				i = 0;
				while (uttline[i]) {
					for (; uS.isskip(uttline, i, &dFnt, MBF) || uttline[i] == '\n'; i++) ;
					if (uttline[i] == EOS)
						break;
					for (j=i; !uS.isskip(uttline, j, &dFnt, MBF) && uttline[j] != '=' && uttline[j] != '\n' && uttline[j] != EOS; j++) ;
					t = uttline[j];
					uttline[j] = EOS;
					check_isLangMatch(uttline+i, 0, i, 152, TRUE);
					uttline[j] = t;
					i = j;
				}
			} else if (uS.partcmp(utterance->speaker, "@Options:",FALSE,FALSE)) {
				char isIPAFound = FALSE;

				i = 0;
				while (uttline[i]) {
					for (; uS.isskip(uttline, i, &dFnt, MBF) || uttline[i] == '\n'; i++) ;
					if (uttline[i] == EOS)
						break;
					for (j=i; !uS.isskip(uttline, j, &dFnt, MBF) && uttline[j] != '=' && uttline[j] != '\n' && uttline[j] != EOS; j++) ;
					t = uttline[j];
					uttline[j] = EOS;
					if (!strcmp(uttline+i, "multi")) {
						check_isMultiBullets = TRUE;
					} else if (!strcmp(uttline+i, "bullets")) {
						check_isCheckBullets = FALSE;
					} else if (!strcmp(uttline+i, "CA") || !strcmp(uttline+i, "CA-Unicode")) {
						check_isCAFound = TRUE;
					} else if (!strcmp(uttline+i, "IPA")) {
						isIPAFound = TRUE;
					} else if (!strcmp(uttline+i, "notarget")) {
						check_applyG2Option = FALSE;
					} else if (!strcmp(uttline+i, "mismatch")) {
						check_check_mismatch = FALSE;
					} else if (!strcmp(uttline+i, "dummy")) {
						return(FALSE);
					}
					uttline[j] = t;
					if (uttline[j] == '=')
						for (; !uS.isskip(uttline, j, &dFnt, MBF) && uttline[j] != '\n' && uttline[j] != EOS; j++) ;
					i = j;
				}
				if (check_isCAFound && isIPAFound)
					check_err(103,-1,-1,lineno);
				else {
/*
					if (check_isCAFound) {
						if (strcmp(dFnt.fontName, "CAfont") && strcmp(dFnt.fontName, "Ascender Uni Duo"))
							check_err(104,-1,-1,lineno);
					}
					if (isIPAFound) {
						if (strcmp(dFnt.fontName, "Charis SIL")) 
							check_err(105,-1,-1,lineno);
					}
*/
				}
			} else if (uS.partcmp(utterance->speaker,MEDIAHEADER,FALSE,FALSE)) {
				int cnt;
				char isAudioFound, isVideoFound, qm;

				isSkip = TRUE;
				if ((ts=check_FindTier(headt,templineC)) == NULL)
					check_CodeErr(17);
				isMediaHeaderFound = TRUE;
				cnt = 0;
				i = 0;
				qm = 0;
				isAudioFound = FALSE;
				isVideoFound = FALSE;
				check_isUnlinkedFound = FALSE;
				while (uttline[i]) {
					for (; uS.isskip(uttline, i, &dFnt, MBF) || uttline[i] == ',' || uttline[i] == '\n'; i++) {
						if (uttline[i] == ',' && i > 0 && isSpace(uttline[i-1])) {
							check_err(148,i-1,i-1,lineno);
						}
					}
					if (uttline[i] == EOS)
						break;
					for (j=i; uttline[j] != EOS; j++) {
						if (qm == 0 && (uttline[j] == '"' || uttline[j] == '\'')) {
							qm = uttline[j];
						} else if (qm == uttline[j]) {
							qm = 0;
						} else if (qm != 0) {
						} else if ((uS.isskip(uttline, j, &dFnt, MBF) && uttline[j] != '.') ||
								   uttline[j] == ',' || uttline[j] == '\n') {
							break;
						}
					}
					t = uttline[j];
					uttline[j] = EOS;
					cnt++;
					if (cnt > 1) {
						if (uttline[i-1] == '.' && cnt == 2) {
							uttline[j] = t;
							check_err(99,i,j-1,lineno);
						} else if (check_matchplate(ts->tplate,uttline+i, 'D')) {
							uttline[j] = t;
							check_err(113,i,j-1,lineno);
						} else if (!uS.mStricmp(uttline+i, "audio"))
							isAudioFound = TRUE;
						else if (!uS.mStricmp(uttline+i, "video"))
							isVideoFound = TRUE;
						else if (!uS.mStricmp(uttline+i, "unlinked"))
							check_isUnlinkedFound = TRUE;
					} else if (check_check_mismatch) {
						extractFileName(templineC3, oldfname);
						if ((ext=strrchr(templineC3, '.')) != NULL)
							*ext = EOS;
						if (uttline[i] == '"' || uttline[i] == '\'')
							i++;
						if (uS.mStricmp(uttline+i, templineC3) && uS.mStrnicmp(uttline+i, "http", 4) != 0) {
							if (ext != NULL && uS.mStricmp(ext+1, "cex") == 0) {
								if ((ext=strchr(templineC3, '.')) != NULL)
									*ext = EOS;
								if (uS.mStricmp(uttline+i, templineC3)) {
									check_err(157,i,j-1,lineno);
								}
							} else
								check_err(157,i,j-1,lineno);
						}
					}
					uttline[j] = t;
					i = j;
				}
				if (!isAudioFound && !isVideoFound) {
					check_err(114,-1,-1,lineno);
				}
			} else if (uS.partcmp(utterance->speaker,"@Exceptions",FALSE,FALSE)) {
				check_getExceptions(uttline);
			}
			CodeLegalPos = FALSE;
			if (uS.patmat(templineC,PARTICIPANTS))
				check_getpart(uttline);
			else if (check_checktier(utterance->speaker)) {
				check_CleanTierNames(templineC);
				if ((ts=check_FindTier(headt,templineC)) == NULL)
					check_CodeErr(17);
				else if (uS.patmat(templineC,AGEOF))
					check_Age(ts, templineC, uttline, 0, EOS);
				else if (uS.patmat(templineC,SEXOF))
					check_Sex(ts, templineC, uttline);
				else if (uS.patmat(templineC,SESOF))
					check_ses(ts, templineC, uttline, 0, EOS);
				else if (uS.patmat(templineC,BIRTHOF))
					check_Birth(ts, templineC, uttline);
				else if (uS.patmat(templineC,EDUCOF))
					check_Educ(ts, templineC, uttline);
				else if (uS.patmat(templineC,GROUPOF))
					check_Group(ts, templineC, uttline);
				else if (uS.patmat(templineC,IDOF))
					check_ID(ts);
				else if (!isSkip)
					check_ParseWords(ts);
			}
		} else if (*utterance->speaker=='*' || uS.partcmp(utterance->speaker, "%wor:",FALSE,FALSE)) {
			check_clean_template(headcodes);
			headcodes = NULL;
			CodeLegalPos = TRUE;
			if (check_checktier(utterance->speaker) && !nomain) {
				check_CleanTierNames(templineC);
				if (*utterance->speaker=='%') {
					if ((ts=check_FindTier(codet,templineC)) == NULL)
						check_CodeErr(17);
					else {
						check_ParseWords(ts);
						check_ParseScope(uttline);
					}
				} else if ((ts=check_FindTier(maint,templineC)) == NULL)
					check_CodeErr(17);
				else if ((tp=check_matchSpeaker(headsp,templineC+1, FALSE)) == NULL)
					check_CodeErr(18);
				else {
					if (tp != NULL)
						tp->isUsed = TRUE;
					check_ParseWords(ts);
					check_ParseScope(uttline);
				}
			}
		} else if (utterance->speaker[1] != 'x') /* if (*utterance->speaker == '%') */ {
			if (!CodeLegalPos)
				check_CodeErr(39);
			CodeLegalPos = TRUE;
			if (check_checktier(utterance->speaker)) {
				check_CleanTierNames(templineC);
				if (isCheckUDFeats && (uS.partcmp(utterance->speaker, "%mor:", FALSE, FALSE) ||
									   uS.partcmp(utterance->speaker, "%umor:", FALSE, FALSE))) {  // 2024-02-16
					for (i=0; utterance->line[i] != EOS; i++) {
						if (utterance->line[i] == '-' || utterance->line[i] == '&') {
							if (utterance->line[i] == '&') {
								check_all_err_found = TRUE;
								check_addUDerr(utterance->line+i);
							} else if (!isUDsCode(utterance->line+i, FALSE)) {
								check_all_err_found = TRUE;
								check_addUDerr(utterance->line+i);
//								fprintf(stderr, "\n*** File \"%s\": line %ld.\n", oldfname, lineno);
//								fprintf(stderr, "Code not found: %s\n", utterance->line+i);
							}
						}
					}
				}
				if ((ts=check_FindTier(codet,templineC)) == NULL)
					check_CodeErr(17);
				else
					check_ParseWords(ts);
				if (uS.partcmp(utterance->speaker, "%mor:", FALSE, FALSE)) {
					mor_link.error_found = FALSE;
					check_cleanUpLine(org_spName, org_spTier);
					strcpy(spareTier1, utterance->line);
//					filterwords(utterance->speaker,spareTier1,excludeMorItems);
					createMorUttline(templineC4, org_spTier, "%mor:", spareTier1, TRUE, TRUE);
					if (mor_link.error_found) {
						check_err(140, -1, -1, lineno);
					}
					mor_link.error_found = FALSE;
					mor_link.fname[0] = EOS;
					mor_link.lineno = 0L;		
				}
			} else {
				strcpy(templineC,utterance->speaker);
//				uS.uppercasestr(templineC, &dFnt, MBF);
				check_CleanTierNames(templineC);
			}
			for (tc=headcodes; tc != NULL; tc=tc->nextone) {
				if (!strcmp(tc->pattern, templineC)) {
					check_CodeErr(40);
					break;
				}
			}
			if (tc == NULL) {
				if ((tc=NEW(CHECK_CODES)) == NULL)
					check_overflow();
				if ((tc->pattern=(char *)malloc(strlen(templineC)+1)) == NULL)
					check_overflow();
				strcpy(tc->pattern, templineC);
				tc->nextone = headcodes;
				headcodes = tc;
			}
		} else { /* sp == '%x...: */
// Duplicate tiers: off 2019-07-22 on 2021-11-11
			for (tc=headcodes; tc != NULL; tc=tc->nextone) {
				if (!strcmp(tc->pattern, templineC)) {
					check_CodeErr(40);
					break;
				}
			}
			check_ParseBlob(NULL);

			if (tc == NULL) {
				if ((tc=NEW(CHECK_CODES)) == NULL) check_overflow();
				if ((tc->pattern=(char *)malloc(strlen(templineC)+1)) == NULL)
					check_overflow();
				strcpy(tc->pattern, templineC);
				tc->nextone = headcodes;
				headcodes = tc;
			}
		}
	}
/* in 2018-10-03 out 2018-10-07
	if (!check_isUnlinkedFound && !isBulletsFound) {
		check_err(154,-1,-1,-1L);
	}
*/
	if (head_bg != NULL) {
		check_err(45,-1,-1,lineno);
		for (tb=head_bg; tb != NULL; tb=tb->nextone) {
			uS.remblanks(tb->pattern);
			fprintf(fpout, "    @Bg:\t%s\n", tb->pattern);
		}
		putc('\n', fpout);
		check_clean_template(head_bg);
	}
	check_clean_template(headcodes);
	return(TRUE);
}
/* 2011-04-07
static char check_unUsedSpeakers(TPLATES *tp) {
	int  i;
	char err, found;

	err = FALSE;
	for (; tp != NULL; tp = tp->nextone) {
		if (tp->cnt == 0) {
			fprintf(fpout,"*** File \"%s\": line 0.\n", oldfname);
			fprintf(fpout, "Speaker \"*%s\" not used in this file\n", tp->pattern);
			err = TRUE;
		} else {
			found = FALSE;
			for (i=0; i < NUMUSEDTIERSONCE; i++) {
				if (tiersUsedOnce[i].spCode[0] != EOS) {
					if (strcmp(tiersUsedOnce[i].spCode, tp->pattern) == 0) {
						if (tiersUsedOnce[i].freq < 10) {
							if (tp->cnt < 10)
								tiersUsedOnce[i].freq += tp->cnt;
							else
								tiersUsedOnce[i].freq += 10;
						}
						if (tiersUsedOnce[i].fileNum < 10)
							tiersUsedOnce[i].fileNum += 1;
						found = TRUE;
						break;
					}
				}
			}
			if (found == FALSE) {
				for (i=0; i < NUMUSEDTIERSONCE; i++) {
					if (tiersUsedOnce[i].spCode[0] == EOS) {
						strncpy(tiersUsedOnce[i].spCode, tp->pattern, USEDTIERSONCELEN);
						tiersUsedOnce[i].spCode[USEDTIERSONCELEN] = EOS;
						tiersUsedOnce[i].fileNum = 1;
						if (tp->cnt < 10)
							tiersUsedOnce[i].freq += tp->cnt;
						else
							tiersUsedOnce[i].freq += 10;
						break;
					}
				}
			}
		}
	}
	return(err);
}
*/
void call() {
	double prc, ffc, ftf;
	SPLIST *tp;

	check_applyG2Option = TRUE;
	FileCount++;
	if (totalFilesNum > MINFILESNUM && !stin) { // 2019-04-18 TotalNumFiles
		ffc = (float)FileCount;
		ftf = (float)totalFilesNum;
		prc = (ffc * 100.0000) / ftf;
		if (prc >= oPrc) {
			fprintf(stderr, "\r%.1lf%% ", prc);
			oPrc = prc + 0.1;
		}
	}
	if (check_OverAll()) {
		check_err_found = FALSE;
/* 2017-04-11
		if (fpout != stdout)
			fprintf(stderr, "\rFirst pass DONE.\n");
		if (!isRecursive && onlydata != 3)
			fprintf(fpout, "First pass DONE.\n");
*/
		if (!check_CheckRest())
			return;
		check_isMissiingID();
/* 2017-04-11
		if (fpout != stdout)
			fprintf(stderr,"\rSecond pass DONE.\n");
		if (!isRecursive && onlydata != 3)
			fprintf(fpout, "Second pass DONE.\n");
*/
	} else {
		check_err_found = TRUE;
		if (fpout != stdout)
			fprintf(stderr,"\nWarning: BASIC SYNTAX ERROR - Second pass not attempted !\n");
		if (!isRecursive)
			fprintf(fpout,"\nWarning: BASIC SYNTAX ERROR - Second pass not attempted !\n");
	}
/* 2011-04-07
	if (check_unUsedSpeakers(headsp)) {
		check_err_found = TRUE;
	}
*/
	if (check_err_found) {
		check_all_err_found  = TRUE;
		if (fpout != stdout)
			fprintf(stderr,"Warning: Please repeat CHECK until no error messages are reported!\n\n");
/* 2017-04-11
		if (!isRecursive && onlydata != 3)
			fprintf(fpout,"Warning: Please repeat CHECK until no error messages are reported!\n\n");
*/
	} else if (ErrorRepFlag) {
		if (onlydata != 3) {
			if (ErrorRepFlag == '+') {
				if (fpout != stdout) fprintf(stderr,"No errors of type chosen with %ce option were found.\n\n", ErrorRepFlag);
				fprintf(fpout,"No errors of type chosen with %ce option were found.\n\n", ErrorRepFlag);
			} else {
				if (fpout != stdout) fprintf(stderr,"No errors other than excluded by %ce option were found.\n\n", ErrorRepFlag);
//2023-01-29				fprintf(fpout,"No errors other than excluded by %ce option were found.\n\n", ErrorRepFlag);
			}
		}
	} else {
/* 2017-04-11
		if (fpout != stdout)
			fprintf(stderr,"Success! No errors found.\n");
		else if (onlydata == 3 && isRecursive)
			fprintf(stderr,"    Success! No errors found.\n");

		if (onlydata != 3) {
			if (!isRecursive)
				fprintf(fpout,"Success! No errors found.\n\n");
			else
				fprintf(fpout,"    Success! No errors found.\n");
		}
*/
	}
	if (IS_SPK_USED(check_GenOpt)) {
		for (tp=headsp; tp != NULL; tp = tp->nextsp) {
			if (tp->isUsed == FALSE) {
				check_all_err_found  = TRUE;
				if (totalFilesNum > MINFILESNUM && !stin) { // 2019-04-18 TotalNumFiles
					fprintf(stderr, "\r    	     \r");
				}
				fprintf(fpout,"*** File \"%s\": line 3.\n", oldfname);
				fprintf(fpout,"Speaker \"%s\" is not used in this file.\n", tp->sp);
			}
		}
	}
}

void getflag(char *f, char *f1, int *i) {
		int n, j;

		f++;
		switch(*f++) {
			case 'c':
				j = atoi(f) + 1;
				if (j < 1 || j > 2) {
					fprintf(stderr,"The range of +cN is 0 to 1.\n");
					cleanupLanguages();
					cutt_exit(0);
				}
				checkBullets = j;
				break;
			case 'e':
				if (*f) {
					j = atoi(f);
					if (j < 1 || j > LAST_ERR_NUM) {
						fprintf(stderr,"The range of +eN is 1 to %d.\n",LAST_ERR_NUM);
						cleanupLanguages();
						cutt_exit(0);
					}
					if (*(f-2) == '-') {
						ErrorReport[j-1] = FALSE;
						ErrorRepFlag = '-';
					} else {
						if (!ErrorRepFlag) {
							for (n=0; n < LAST_ERR_NUM; n++) 
								ErrorReport[n] = FALSE;
						}
						ErrorReport[j-1] = TRUE;
						ErrorRepFlag = '+';
					}
				} else {
					fpout = stdout;
					for (n=1; n <= LAST_ERR_NUM; n++) {
						fprintf(fpout, "%2d: ", n);
						err_itm[0] = EOS;
						check_mess(n);
					}
					cleanupLanguages();
					cutt_exit(0);
				}
				break;
			case 'g':
				if ((n=atoi(f)) > NUM_GEN_OPT || n < 1) {
					fprintf(stderr,"The range of N in %cg is 1 to %d.\n", *(f-2), NUM_GEN_OPT);
					cleanupLanguages();
					cutt_exit(0);
				}
				if (n == 1) {
					if (*(f-2) == '+')
						check_GenOpt = D_D_SET_1(check_GenOpt);
					else
						check_GenOpt = D_D_SET_0(check_GenOpt);
				} else if (n == 2) {
					if (*(f-2) == '+')
						check_GenOpt = NAME_ROLE_SET_1(check_GenOpt);
					else
						check_GenOpt = NAME_ROLE_SET_0(check_GenOpt);
				} else if (n == 3) {
					if (*(f-2) == '+')
						check_GenOpt = W_CHECK_SET_1(check_GenOpt);
					else
						check_GenOpt = W_CHECK_SET_0(check_GenOpt);
				} else if (n == 4) {
					if (*(f-2) == '+')
						check_GenOpt = SET_CHECK_ID_1(check_GenOpt);
					else
						check_GenOpt = SET_CHECK_ID_0(check_GenOpt);
				} else if (n == 5) {
					if (*(f-2) == '+')
						check_GenOpt = SET_SPK_USED_1(check_GenOpt);
					else
						check_GenOpt = SET_SPK_USED_0(check_GenOpt);
				}
/*
				else if (n == 6) {
					if (*(f-2) == '+')
						check_GenOpt = SET_LANG_CHECK_1(check_GenOpt);
					else
						check_GenOpt = SET_LANG_CHECK_0(check_GenOpt);
				}
*/
				break;
			case 'u':
				isCheckUDFeats = TRUE;
				no_arg_option(f);
				break;
#ifdef UNX
			case 'L':
				strcpy(lib_dir, f);
				j = strlen(lib_dir);
				if (j > 0 && lib_dir[j-1] != '/')
					strcat(lib_dir, "/");
				break;
#endif
			default:
				maingetflag(f-2,f1,i);
				break;
		}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int i;
	struct UDErr *tp;

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CHECK_P;
// 2017-04-11	OnlydataLimit = 3;
	OnlydataLimit = 2;
	UttlineEqUtterance = TRUE;
	initLanguages();
	check_GenOpt = 0;
	check_UDerr = NULL;
#ifdef _MAC_CODE
//	struct passwd *pass;
//	pass = getpwuid(getuid());
#endif
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') {
			if (argv[i][1] == 'g') {
				if (i+1 < argc) {
					getflag(argv[i],argv[i+1],&i);
				} else {
					getflag(argv[i],NULL,&i);
				}
			}
		}
	}
	oPrc = 0.0;
	FileCount = 0;
	bmain(argc,argv,NULL);
	if (totalFilesNum > MINFILESNUM && !stin) { // 2019-04-18 TotalNumFiles
		fprintf(stderr, "\r    	     \r");
	}
	cleanupLanguages();
	if (FileCount > 0) { // 2019-04-17 changed from 1 to 0
/* 2011-04-07
		for (i=0; i < NUMUSEDTIERSONCE; i++) {
			if (tiersUsedOnce[i].spCode[0] != EOS) {
				if (tiersUsedOnce[i].freq < 5 && tiersUsedOnce[i].fileNum == 2)
					fprintf(stderr,"Warning: found speaker \"%s\" %d times in just %d file(s)\n",
						tiersUsedOnce[i].spCode, tiersUsedOnce[i].freq, tiersUsedOnce[i].fileNum);
				else if (tiersUsedOnce[i].freq < 3 && tiersUsedOnce[i].fileNum == 1)
					fprintf(stderr,"Warning: found speaker \"%s\" %d times in just %d file(s)\n",
							tiersUsedOnce[i].spCode, tiersUsedOnce[i].freq, tiersUsedOnce[i].fileNum);
			}
		}
*/
		if (!check_all_err_found) {
			if (ErrorRepFlag) {
				fprintf(fpout,"\nNo errors other than excluded by %ce option were found.\n", ErrorRepFlag);
				fprintf(stderr,"Please repeat CHECK without %ce option until no error are reported!\n\n", ErrorRepFlag);
			} else
				fprintf(stderr, "\nALL FILES CHECKED OUT OK!\n");
		} else
			fprintf(stderr, "\nTHERE WERE SOME ERROR(S) FOUND.\n");
	}
	if (check_UDerr != NULL) {
		fprintf(stderr, "Code(s) not found:\n");
		for (tp=check_UDerr; tp != NULL; tp=tp->nextUDerr) {
//			if (tp->code[0] == '-' || tp->code[0] == '&')
//				fprintf(stderr, "    %s\n", tp->code+1);
//			else
				fprintf(stderr, "    %s\n", tp->code);
		}
	}
	check_UDerr = check_clean_UDerr(check_UDerr);
	check_clean_headerr();
	check_clean_IDsp(check_idsp);
	check_clean_speaker(headsp);
	check_clean__t(headt);
	check_clean__t(maint);
	check_clean__t(codet);
}
