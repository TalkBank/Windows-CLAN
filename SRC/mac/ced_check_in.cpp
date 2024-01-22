#include "ced.h"
#include "cu.h"
#include "check.h"
#include "my_ctype.h"
#include "MMedia.h"
#import "Document.h"
/*
#ifdef _MAC_CODE
#undef isatty
#include <unistd.h>
#include <pwd.h>
#endif
*/
extern void CheckC(NSTextView *textView, char *filePath, BOOL isMedia);

#define CED_HIDEN_C 0x2022

#define NUMLANGUAGESINTABLE 10

#define NUM_GEN_OPT 4   /* number of generic options */
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

//#define SET_LANG_CHECK_1(x)	(AttTYPE)(x | 16)
//#define IS_LANG_CHECK(x)	(AttTYPE)(x & 16)
//#define SET_LANG_CHECK_0(x)	(AttTYPE)(x & 0xffef)

TPLATES {
	unCH *pattern;
	TPLATES *nextone;
} ;

struct IDsp {
	unCH *sp;
	struct IDsp *nextsp;
} ;

SPLIST {
	unCH *sp;
	unCH *role;
	long endTime;
	char hasID;
	SPLIST *nextsp;
} ;

ERRLIST {
	unCH *st;
	short errnum;
	ERRLIST *nexterr;
} ;

CHECK_IN_TIERS {
	unCH *code;
	char col,
		IGN, 					/* if TRUE, do not check words and such on that tier */
		UTD, 					/* if TRUE, check tiers for utterance delimiters */
		CSUFF,					/* if TRUE, check tiers' words' suffixes */
		WORDCHECK;				/* if TRUE, check every words on tier for bad characters */
	TPLATES *prefs;	/* items in place of '*' [UPREFS Mac] all pref Upper chars: MacDonald*/
	TPLATES *tplate;
	CHECK_IN_TIERS *nexttier;
} ;

static class cedUtil
{
public:

	int check_patmat(unCH *s, const unCH *pat);
	int check_patmat(unCH *s, const char *pat);
	int check_partcmp(unCH *st1, const unCH *st2, char pat_match);
	int check_partcmp(unCH *st1, const char *st2, char pat_match);
	int check_patmat(char *s, const char *pat);
	int check_partcmp(char *st1, const char *st2, char pat_match);
} ced_u;

static int  ced_onlydata;
static int  ced_curLanguage;
static int  ced_CurSpLen;
static char ced_AtBeginFound;
static char ced_AtEndFound;
static char ced_AtIDFound;
static char ced_CodeLegalPos;
static char ced_isNumberShouldBeHere;
static char ced_isArabic;
static char	ced_isHebrew;
static char ced_isGerman;
static char ced_isBlob;
static char ced_isCAFound;
static char ced_isMultiBullets;
static char ced_isCheckBullets;
static char ced_isUnlinkedFound;
static char ced_applyG2Option;
static char ced_check_mismatch;
static char ced_Parans = '\001';		   /* get(s); 1-gets, 2-get(s), 3-get				   */
static char Check_level;
static char isItemPrefix[256];
static char dep_file[FILENAME_MAX];
static char checkBullets = 0;
static char ced_fav_lang[20];
static char err_item[512];
static unCH ced_LanguagesTable[NUMLANGUAGESINTABLE][9];
static unCH ced_lastSpDelim[25];
static unCH ced_SNDFname[FILENAME_MAX];
static unCH ced_sp[SPEAKERLEN];
static long tDiff;
static long ced_tierBegTime = 0L;
static long ced_SNDBeg = 0L;
static long ced_SNDEnd = 0L;
static NSUInteger textPos, begTPos;
//static UInt32 DepfileDate = 0L;
//static long DepAddDate = 0L;
static NewFontInfo tFnt;
static TPLATES *ced_WordExceptions;
static CHECK_IN_TIERS *ced_headt, *ced_maint, *ced_codet;
static struct IDsp *ced_idsp;
static SPLIST *ced_headsp;
static TCODES *ced_headcodes;
static BE_TIERS *ced_head_bg;
static ERRLIST *ced_headerr;

AttTYPE ced_GenOpt, old_GenOpt;
char isdone;
char ced_err_message[ERRMESSAGELEN];
NSRange ced_cursorRange;


static void ced_clean_headerr(void);
static void ced_clean_template(TPLATES *p);
static void ced_clean_IDsp(struct IDsp *p);
static void ced_clean_speakers(SPLIST *p);
static void ced_clean_t(CHECK_IN_TIERS *p);

void main_check_init(void) {
	checkBullets = 0;
	ced_onlydata = 0;
	ced_isNumberShouldBeHere = FALSE;
	ced_isArabic = FALSE;
	ced_isHebrew = FALSE;
	ced_isGerman = FALSE;
	ced_isBlob = FALSE;
	ced_isCAFound = FALSE;
	ced_isMultiBullets = FALSE;
	ced_isCheckBullets = TRUE;
	ced_WordExceptions = NULL;
	ced_isUnlinkedFound = FALSE;
	ced_applyG2Option = TRUE;
	ced_check_mismatch = TRUE;
	strcpy(dep_file, DEPFILE);
	ced_Parans = '\001';
	ced_GenOpt = 0;
	ced_GenOpt = D_D_SET_0(ced_GenOpt);			/* set to 1 if '-?' is not a delimiter */
	ced_GenOpt = NAME_ROLE_SET_0(ced_GenOpt);	/* set to 1 if want to check for CHI-Target_Child */
	ced_GenOpt = W_CHECK_SET_1(ced_GenOpt);		/* if 0 don't check WORDS for <'> & digits */
	ced_GenOpt = SET_CHECK_ID_1(ced_GenOpt);	/* set to 0 if you do not want to check for missing @ID */
	old_GenOpt = ced_GenOpt;
	ced_check_init(TRUE);
}

void ced_check_init(char first) {
	int i;

	if (!first) {
		ced_clean_t(ced_headt);
		ced_clean_t(ced_maint);
		ced_clean_t(ced_codet);
		ced_clean_IDsp(ced_idsp);
		ced_clean_speakers(ced_headsp);
		ced_clean_headerr();
		ced_clean_template(ced_headcodes);
		ced_clean_template(ced_head_bg);
		ced_clean_template(ced_WordExceptions);
	}
	ced_AtBeginFound = FALSE;
	ced_AtEndFound   = FALSE;
	ced_AtIDFound    = FALSE;
	ced_CodeLegalPos = FALSE;
	ced_CurSpLen = 0;
	ced_headt = NULL;
	ced_maint = NULL;
	ced_codet = NULL;
	ced_idsp = NULL;
	ced_headsp = NULL;
	ced_head_bg = NULL;
	ced_headerr = NULL;
	ced_headcodes = NULL;
	ced_isNumberShouldBeHere = FALSE;
	ced_isArabic = FALSE;
	ced_isHebrew = FALSE;
	ced_isGerman = FALSE;
	ced_isBlob = FALSE;
	ced_isCAFound = FALSE;
	ced_isMultiBullets = FALSE;
	ced_isCheckBullets = TRUE;
	ced_isUnlinkedFound = FALSE;
	ced_applyG2Option = TRUE;
	ced_check_mismatch = TRUE;
	ced_WordExceptions = NULL;
	ced_SNDFname[0] = EOS;
	ced_SNDBeg = 0L;
	ced_SNDEnd = 0L;
	ced_tierBegTime = 0L;
	templineW1[0] = EOS;
	for (i=0; i < NUMLANGUAGESINTABLE; i++)
		ced_LanguagesTable[i][0] = EOS;
}

static char ced_getc_cr(FILE *fp) {
    register char c;
    
    if ((c=(char)getc(fp)) == '\r')
    	return('\n');
    return(c);
}

short IsGSet(short which) {
	if (which == 1) return((D_D_COUNT(ced_GenOpt) ? TRUE : FALSE));
	else if (which == 2) return((NAME_ROLE_COUNT(ced_GenOpt) ? TRUE : FALSE));
	else if (which == 3) return((W_CHECK_COUNT(ced_GenOpt) ? TRUE : FALSE));
	else if (which == 4) return((IS_CHECK_ID(ced_GenOpt) ? TRUE : FALSE));
//	else if (which == 5) return((IS_LANG_CHECK(ced_GenOpt) ? TRUE : FALSE));
	return(0);
}

void SetGOption(short which, int val) {
	if (which == 1) {
		if (val) ced_GenOpt = D_D_SET_1(ced_GenOpt);
		else ced_GenOpt = D_D_SET_0(ced_GenOpt);
	} else if (which == 2) {
		if (val) ced_GenOpt = NAME_ROLE_SET_1(ced_GenOpt);
		else ced_GenOpt = NAME_ROLE_SET_0(ced_GenOpt);
	} else if (which == 3) {
		if (val) ced_GenOpt = W_CHECK_SET_1(ced_GenOpt);
		else ced_GenOpt = W_CHECK_SET_0(ced_GenOpt);
	} else if (which == 4) {
		if (val) ced_GenOpt = SET_CHECK_ID_1(ced_GenOpt);
		else ced_GenOpt = SET_CHECK_ID_0(ced_GenOpt);
	}
	if (old_GenOpt != ced_GenOpt) {
		old_GenOpt = ced_GenOpt;
//		resetAllTierCheckFlags(TRUE);
	}
}

/* cleanup of variables start */
static void ced_clean_headerr(void) {
	ERRLIST *t;

	while (ced_headerr != NULL) {
		t = ced_headerr;
		ced_headerr = ced_headerr->nexterr;
		free(t->st);
		free(t);
	}
}

static void ced_clean_IDsp(struct IDsp *p) {
	struct IDsp *t;

	while (p != NULL) {
		t = p;
		p = p->nextsp;
		if (t->sp)
			free(t->sp);
		free(t);
	}
}

static void ced_clean_speakers(SPLIST *p) {
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

static void ced_clean_template(TPLATES *p) {
	TPLATES *t;

	while (p != NULL) {
		t = p;
		p = p->nextone;
		if (t->pattern)
			free(t->pattern);
		free(t);
	}
}

static void ced_clean_t(CHECK_IN_TIERS *p) {
	CHECK_IN_TIERS *t;

	while (p != NULL) {
		t = p;
		p = p->nexttier;
		free(t->code);
		ced_clean_template(t->prefs);
		ced_clean_template(t->tplate);
		free(t);
	}
}
/* cleanup of variables end */

/* some string manipulation start */
static unCH *ced_mkstring(unCH *s) {  /* allocate space for string of text */
	unCH *p;

	if ((p=(unCH *)malloc((strlen(s)+1)*sizeof(unCH))) == NULL)
		return(NULL);
	strcpy(p, s);
//	uS.uppercasestr(p, &tFnt, TRUE);
	return(p);
}

/* check_patmat(s, pat) does pattern matching of pattern "pat" to string "s".
   "pat" may contain meta characters "_", "*", "%" and "\". "_" means match
   any one character, "*" means match zero or more characters, "%" means match
   zero or more characters and delete the matched part from string "s", "\" is
   used to specify meta characters as litteral characters. The value returned
   is 1 if there is a match, and 0 otherwise.
*/
int cedUtil::check_patmat(unCH *s, const unCH *pat) {
	register int j, k;
	int n, m, t, l;
	unCH *lf;

	if (s[0] == EOS) {
		return(pat[0] == s[0]);
	}
	l = strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if ((s[j] == '(' || s[j] == ')') && *ced_sp != '%') {
			if (pat[k] != s[j]) {
				k--;
				continue;
			}
		} else if (pat[k] == '\\') {
			if (s[j] != pat[++k]) break;
		} else if (pat[k] == '_') {
			if (iswspace(s[j]))
				return(FALSE);
			if (s[j] == EOS)
				return(FALSE);
			if (s[j+1] && pat[k+1])
				continue; /* any character */
			else {
				if (s[j+1] == EOS && pat[k+1] == '*' && pat[k+2] == EOS)
					return(TRUE);
				else if (pat[k+1] == s[j+1]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			}
		} else if (pat[k] == '*') {		  /* wildcard */
			k++; t = j;
			if (pat[k] == '\\') k++;
f1:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else {
					for (; t < j; t++) {
						if (uS.ismorfchar(s,t, &tFnt, CHECK_MORPHS, TRUE))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS && *ced_sp == '*') {
						s = s+l;
						if (lf != s) {
							while (lf != s)
								*lf++ = ' ';
						}
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && *ced_sp != '%')
					n--;
				else if (pat[n] == '*' || pat[n] == '%')
					break;
				else if (pat[n] == '_') {
					if (iswspace(s[m]) || s[m] == EOS) {
						j++;
						goto f1;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n])
						return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f1;
					}
				} else if (s[m] != pat[n]) {
					if (pat[n+1] == '%' && pat[n+2] == '%')
						break;
					j++;
					goto f1;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f1;
			}
		} else if (pat[k] == '%') {		  /* wildcard */
			m = j;
			if (pat[++k] == '%') {
				k++;
				if (pat[k] == '\\') k++;
				if ((t=j - 1) < 0) t = 0;
			} else {
				if (pat[k] == '\\') k++;
				t = j;
			}
f2:
/*			while (s[j] && s[j] != pat[k]) s[j++] = ' ';
*/			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					lf = uS.sp_cp(s+t,s+j);
					s = s+l;
					if (lf != s) {
						while (lf != s) *lf++ = ' ';
					}
					return(TRUE); 
				} else {
					for (; m < j; m++) {
						if (uS.ismorfchar(s, m, &tFnt, CHECK_MORPHS, TRUE))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS && *ced_sp == '*') {
						lf = uS.sp_cp(s+t,s+j);
						s = s+l;
						if (lf != s) {
							while (lf != s) *lf++ = ' ';
						}
						return(TRUE); 
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && *ced_sp != '%')
					n--;
				else if (pat[n] == '*' || pat[n] == '%') break;
				else if (pat[n] == '_') {
					if (iswspace(s[m])) {
						j++;
						goto f2;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n])
						return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f2;
					}
				} else if (s[m] != pat[n]) {
					j++;
					goto f2;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f2;
			}
			lf = uS.sp_cp(s+t,s+j);
			j = t;
		}

		if (s[j] != pat[k]) {
			if (pat[k+1] == '%' && pat[k+2] == '%') {
				if (s[j] == EOS && pat[k+3] == EOS && *ced_sp == '*') {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			} else
				return(FALSE);
		}
	}
	if (pat[k] == s[j]) {
		s = s+l;
		if (lf != s) {
			while (lf != s)
				*lf++ = ' ';
		}
		return(TRUE);
	} else
		return(FALSE);
}

int cedUtil::check_patmat(unCH *s, const char *pat) {
	register int j, k;
	int n, m, t, l;
	unCH *lf;

	if (s[0] == EOS) {
		return(pat[0] == s[0]);
	}
	l = strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if ((s[j] == '(' || s[j] == ')') && *ced_sp != '%') {
			if (pat[k] != s[j]) {
				k--;
				continue;
			}
		} else if (pat[k] == '\\') {
			if (s[j] != pat[++k]) break;
		} else if (pat[k] == '_') {
			if (iswspace(s[j]))
				return(FALSE);
			if (s[j] == EOS)
				return(FALSE);
			if (s[j+1] && pat[k+1])
				continue; /* any character */
			else {
				if (s[j+1] == EOS && pat[k+1] == '*' && pat[k+2] == EOS)
					return(TRUE);
				else if (pat[k+1] == s[j+1]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			}
		} else if (pat[k] == '*') {		  /* wildcard */
			k++; t = j;
			if (pat[k] == '\\') k++;
f1:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else {
					for (; t < j; t++) {
						if (uS.ismorfchar(s,t, &tFnt, CHECK_MORPHS, TRUE))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS && *ced_sp == '*') {
						s = s+l;
						if (lf != s) {
							while (lf != s)
								*lf++ = ' ';
						}
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && *ced_sp != '%')
					n--;
				else if (pat[n] == '*' || pat[n] == '%')
					break;
				else if (pat[n] == '_') {
					if (iswspace(s[m]) || s[m] == EOS) {
						j++;
						goto f1;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n])
						return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f1;
					}
				} else if (s[m] != pat[n]) {
					if (pat[n+1] == '%' && pat[n+2] == '%')
						break;
					j++;
					goto f1;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f1;
			}
		} else if (pat[k] == '%') {		  /* wildcard */
			m = j;
			if (pat[++k] == '%') {
				k++;
				if (pat[k] == '\\') k++;
				if ((t=j - 1) < 0) t = 0;
			} else {
				if (pat[k] == '\\') k++;
				t = j;
			}
f2:
/*			while (s[j] && s[j] != pat[k]) s[j++] = ' ';
*/			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					lf = uS.sp_cp(s+t,s+j);
					s = s+l;
					if (lf != s) {
						while (lf != s) *lf++ = ' ';
					}
					return(TRUE); 
				} else {
					for (; m < j; m++) {
						if (uS.ismorfchar(s, m, &tFnt, CHECK_MORPHS, TRUE))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS && *ced_sp == '*') {
						lf = uS.sp_cp(s+t,s+j);
						s = s+l;
						if (lf != s) {
							while (lf != s) *lf++ = ' ';
						}
						return(TRUE); 
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && *ced_sp != '%')
					n--;
				else if (pat[n] == '*' || pat[n] == '%') break;
				else if (pat[n] == '_') {
					if (iswspace(s[m])) {
						j++;
						goto f2;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n])
						return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f2;
					}
				} else if (s[m] != pat[n]) {
					j++;
					goto f2;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f2;
			}
			lf = uS.sp_cp(s+t,s+j);
			j = t;
		}

		if (s[j] != pat[k]) {
			if (pat[k+1] == '%' && pat[k+2] == '%') {
				if (s[j] == EOS && pat[k+3] == EOS && *ced_sp == '*') {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			} else
				return(FALSE);
		}
	}
	if (pat[k] == s[j]) {
		s = s+l;
		if (lf != s) {
			while (lf != s)
				*lf++ = ' ';
		}
		return(TRUE);
	} else
		return(FALSE);
}

int cedUtil::check_patmat(char *s, const char *pat) {
	register int j, k;
	int n, m, t, l;
	char *lf;

	if (s[0] == EOS) {
		return(pat[0] == s[0]);
	}
	l = strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if ((s[j] == '(' || s[j] == ')') && *ced_sp != '%') {
			if (pat[k] != s[j]) {
				k--;
				continue;
			}
		} else if (pat[k] == '\\') {
			if (s[j] != pat[++k]) break;
		} else if (pat[k] == '_') {
			if (isspace(s[j]))
				return(FALSE);
			if (s[j] == EOS)
				return(FALSE);
			if (s[j+1] && pat[k+1])
				continue; /* any character */
			else {
				if (s[j+1] == EOS && pat[k+1] == '*' && pat[k+2] == EOS)
					return(TRUE);
				else if (pat[k+1] == s[j+1]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			}
		} else if (pat[k] == '*') {		  /* wildcard */
			k++; t = j;
			if (pat[k] == '\\') k++;
f1:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else {
					for (; t < j; t++) {
						if (uS.ismorfchar(s, t, &tFnt, CHECK_MORPHS, TRUE))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS && *ced_sp == '*') {
						s = s+l;
						if (lf != s) {
							while (lf != s)
								*lf++ = ' ';
						}
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && *ced_sp != '%')
					n--;
				else if (pat[n] == '*' || pat[n] == '%')
					break;
				else if (pat[n] == '_') {
					if (isspace(s[m]) || s[m] == EOS) {
						j++;
						goto f1;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n])
						return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f1;
					}
				} else if (s[m] != pat[n]) {
					if (pat[n+1] == '%' && pat[n+2] == '%')
						break;
					j++;
					goto f1;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f1;
			}
		} else if (pat[k] == '%') {		  /* wildcard */
			m = j;
			if (pat[++k] == '%') {
				k++;
				if (pat[k] == '\\') k++;
				if ((t=j - 1) < 0) t = 0;
			} else {
				if (pat[k] == '\\') k++;
				t = j;
			}
f2:
/*			while (s[j] && s[j] != pat[k]) s[j++] = ' ';
*/			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					lf = uS.sp_cp(s+t,s+j);
					s = s+l;
					if (lf != s) {
						while (lf != s) *lf++ = ' ';
					}
					return(TRUE); 
				} else {
					for (; m < j; m++) {
						if (uS.ismorfchar(s, m, &tFnt, CHECK_MORPHS, TRUE))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS && *ced_sp == '*') {
						lf = uS.sp_cp(s+t,s+j);
						s = s+l;
						if (lf != s) {
							while (lf != s) *lf++ = ' ';
						}
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && *ced_sp != '%')
					n--;
				else if (pat[n] == '*' || pat[n] == '%') break;
				else if (pat[n] == '_') {
					if (isspace(s[m])) {
						j++;
						goto f2;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n]) return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f2;
					}
				} else if (s[m] != pat[n]) {
					j++;
					goto f2;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f2;
			}
			lf = uS.sp_cp(s+t,s+j);
			j = t;
		}

		if (s[j] != pat[k]) {
			if (pat[k+1] == '%' && pat[k+2] == '%') {
				if (s[j] == EOS && pat[k+3] == EOS && *ced_sp == '*') {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			} else
				return(FALSE);
		}
	}
	if (pat[k] == s[j]) {
		s = s+l;
		if (lf != s) {
			while (lf != s)
				*lf++ = ' ';
		}
		return(TRUE);
	} else
		return(FALSE);
}

int cedUtil::check_partcmp(unCH *st1, const unCH *st2, char pat_match) { /* st1- full, st2- part */
	if (pat_match) {
		int i;
		if (*st1 == *st2) {
			for (i=strlen(st1)-1; i >= 0 && (st1[i] == ' ' || st1[i] == '\t'); i--) ;
			if (st1[i] == ':') st1[i] = EOS;
			else st1[i+1] = EOS;
			return(ced_u.check_patmat(st1+1, st2+1));
		} else
			return(FALSE);
	}
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	if (*st2 == ':') return(!*st1);
	else return(!*st2);
}

int cedUtil::check_partcmp(unCH *st1, const char *st2, char pat_match) { /* st1- full, st2- part */
	if (pat_match) {
		int i;
		if (*st1 == (unCH)*st2) {
			for (i=strlen(st1)-1; i >= 0 && (st1[i] == ' ' || st1[i] == '\t'); i--) ;
			if (st1[i] == ':') st1[i] = EOS;
			else st1[i+1] = EOS;
			return(ced_u.check_patmat(st1+1, cl_T(st2+1)));
		} else
			return(FALSE);
	}
	for (; *st1 == (unCH)*st2 && *st2 != EOS; st1++, st2++) ;
	if (*st2 == ':') return(!*st1);
	else return(!*st2);
}

int cedUtil::check_partcmp(char *st1, const char *st2, char pat_match) { /* st1- full, st2- part */
	if (pat_match) {
		int i;
		if (*st1 == *st2) {
			for (i=strlen(st1)-1; i >= 0 && (st1[i] == ' ' || st1[i] == '\t'); i--) ;
			if (st1[i] == ':') st1[i] = EOS;
			else st1[i+1] = EOS;
			return(ced_u.check_patmat(st1+1, st2+1));
		} else
			return(FALSE);
	}
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	if (*st2 == ':') return(!*st1);
	else return(!*st2);
}
/* some string manipulation end */

/* reading in depfiles start*/
static CHECK_IN_TIERS *SameNameTier(unCH *s, char col, CHECK_IN_TIERS *head) {
	while (head != NULL) {
		if (col == head->col && !strcmp(s, head->code))
			return(head);
		head = head->nexttier;
	}
	return(NULL);
}

static CHECK_IN_TIERS *mktier(unCH *s, char col, char CSUFF, char UTD) {
	CHECK_IN_TIERS *ts;

	if ((ts=NEW(CHECK_IN_TIERS)) == NULL) return(NULL);
	if ((ts->code=(unCH *)malloc((strlen(s)+1)*sizeof(unCH))) == NULL) {
		free(ts);
		return(NULL);
	}
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

static char ced_mktplate(CHECK_IN_TIERS *ts, unCH *s, char AtTheEnd) {
	TPLATES *tp;

	if (uS.isSqCodes(s, templine3, &tFnt, FALSE)) {
		if (strcmp(s, templine3)) {
			free(s);
			s = ced_mkstring(templine3);
		}
	}
	if (s == NULL) return(FALSE);
	if (AtTheEnd) {
		if (ts->tplate == NULL) {
			ts->tplate = NEW(TPLATES);
			if (ts->tplate == NULL) return(FALSE);
			tp = ts->tplate;
		} else {
			for (tp=ts->tplate; tp->nextone != NULL; tp = tp->nextone) ;
			tp->nextone = NEW(TPLATES);
			if (tp->nextone == NULL) return(FALSE);
			tp = tp->nextone;
		}
		tp->nextone = NULL;
	} else {
		tp = NEW(TPLATES);
		if (tp == NULL) return(FALSE);
		tp->nextone = ts->tplate;
		ts->tplate = tp;
	}
	tp->pattern = s;
	return(TRUE);
}

static TPLATES *ced_getPrefChars(TPLATES *prefs, unCH *st) {
	TPLATES *tp;

	uS.extractString(templine3, st, "[UPREFS ", ']');
	st = ced_mkstring(templine3);
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

static void ced_HandleParans(unCH *beg, unCH *end) {
	unCH *temp;

	while (*beg) {
		if (*beg == '(' || *beg == ')') {
			if (ced_Parans == 1)	
				strcpy(beg,beg+1);
			else { /* if (ced_Parans == 3) */
				temp = beg;

				while (*temp && *temp != ')') temp++;
				if (*temp)
					strcpy(beg,temp+1);
				else
					beg++;
			}
		} else
			beg++;
	}
	if (end) {
		for (; beg < end; beg++)
			*beg = ' ';
	}
}

/* 2019-04-23 do not check for upper case letters
static char ced_isRightPref(TPLATES *prefs, unCH *w, int s) {
	int len;

	while (prefs != NULL) {
		if (prefs->pattern) {
			len = strlen(prefs->pattern);
			if (s >= len) {
				strncpy(templine3, w+s-len, len);
				templine3[len] = EOS;
				//				uS.uppercasestr(templine3, &tFnt, TRUE);
				if (!strcmp(prefs->pattern, templine3))
					return(TRUE);
			}
		}
		prefs = prefs->nextone;
	}
	return(FALSE);
}

static char ced_isException(unCH *w, TPLATES *prefs) {
	char tced_Parans;

	strcpy(templine4, w);
	tced_Parans = ced_Parans;
	ced_Parans = 1;
	ced_HandleParans(templine4, NULL);
	ced_Parans = tced_Parans;
	while (prefs != NULL) {
		if (prefs->pattern) {
			if (ced_u.check_patmat(templine4, prefs->pattern))
				return(TRUE);
		}
		prefs = prefs->nextone;
	}
	return(FALSE);
}
*/

static unCH *ced_convertRegExp(unCH *s) {
	strcpy(templine, s);

	return(templine);
}

static char ced_ParseTier(unCH *org) {
	unCH *s;
	char sb, regExp;
	unCH *temp = org;
	CHECK_IN_TIERS *ts, *tsWor;

	for (s=temp; *temp != ':' && *temp != '\n'; temp++) {
		if (*temp == '#')
			*temp = '*';
	}
	if (*temp == ':')
		sb = TRUE;
	else
		sb = FALSE;
	*temp = EOS;
//	uS.uppercasestr(s, &tFnt, TRUE);
	if (*s == '*') {
		if ((ts=SameNameTier(s, sb, ced_maint)) == NULL) {
			ts = mktier(s, sb, TRUE, 1);
			if (ts == NULL)
				return(FALSE);
			ts->nexttier = ced_maint;
			ced_maint = ts;
		}
		if ((tsWor=SameNameTier(cl_T("%wor"), sb, ced_codet)) == NULL) {
			tsWor = mktier(cl_T("%wor"), sb, TRUE, 1);
			if (tsWor == NULL)
				return(FALSE);
			tsWor->nexttier = ced_codet;
			ced_codet = tsWor;
		}		
	} else if (*s == '%') {
		if ((ts=SameNameTier(s, sb, ced_codet)) == NULL) {
			ts = mktier(s, sb, FALSE, 0);
			if (ts == NULL) return(FALSE);
			ts->nexttier = ced_codet;
			ced_codet = ts;
		}
	} else {
		if ((ts=SameNameTier(s, sb, ced_headt)) == NULL) {
			ts = mktier(s, sb, FALSE, 0);
			if (ts == NULL) return(FALSE);
			ts->nexttier = ced_headt;
			ced_headt = ts;
		}
	}
	for (temp++; *temp && isSpace(*temp); temp++) ;
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
		if (((isSpace(*temp) || *temp== '\n') && !sb && !regExp) || !*temp) {
			if (*temp) {
				*temp = EOS;
				for (temp++; *temp && (isSpace(*temp) || *temp== '\n'); temp++) ;
			}
			if (s[0] == '@' && s[1] == 'r' && s[2] == '<') {
				s = ced_convertRegExp(s);
			}
			if (!strcmp(s,"*")) {
				if (!ced_mktplate(ts,ced_mkstring(s),1))
					return(FALSE);
				if (strcmp(ts->code, "*") == 0) {
					if (!ced_mktplate(tsWor,ced_mkstring(s),1))
						return(FALSE);
				}
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
			} else if (uS.fpatmat(s, UPREFSSYM)) {
				ts->prefs = ced_getPrefChars(ts->prefs, s);
				if (strcmp(ts->code, "*") == 0)
					tsWor->prefs = ced_getPrefChars(tsWor->prefs, s);
			} else {
				if (!ced_mktplate(ts,ced_mkstring(s),0))
					return(FALSE);
				if (strcmp(ts->code, "*") == 0) {
					if (!ced_mktplate(tsWor,ced_mkstring(s),0))
						return(FALSE);
				}
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
	return(TRUE);
}

static char ced_ReadDepFile(char *depfile, char err) {
	int  i;
	char c, lc; 
	FILE *fp;
	FNType mDirPathName[FNSize];

	fp = NULL;
/* 2018-03-23
	if (!isRefEQZero(fileName)) {
		extractPath(mDirPathName, fileName);
		addFilename2Path(mDirPathName, depfile);
		fp = fopen(mDirPathName,"r");
		if (fp != NULL) {
			sprintf(ced_err_message, "-Using depfile \"%s\"", mDirPathName);
			draw_mid_wm();
		}
	}
*/
/*
	if (fp == NULL) {
		strcpy(mDirPathName, wd_dir);
		addFilename2Path(mDirPathName, depfile);
		fp = fopen(mDirPathName,"r");
	}
*/	
	if (fp == NULL) {
		strcpy(mDirPathName, lib_dir);
		addFilename2Path(mDirPathName, depfile);
		fp = fopen(mDirPathName,"r");
	}
	if (fp == NULL) {
		if (err) {
			strcpy(templineC4, "+Error: Can't open depfile \"");
			uS.FNType2str(templineC4, strlen(templineC4), mDirPathName);
			strcat(templineC4, "\".");
			strcpy(ced_err_message, templineC4);
			return(FALSE);
		} else {
			return(TRUE);
		}
	}
	i = 0;
	lc = '\n';
	*ced_lineC = EOS;
	while (1) {
		c = ced_getc_cr(fp);
		if (lc == '\n' && c == '#') {
			while (!feof(fp) && c != '\n')
				c = ced_getc_cr(fp);
			if (feof(fp)) {
				ced_lineC[i] = EOS;
				if (ced_lineC[0] != EOS) {
					u_strcpy(ced_line, ced_lineC, UTTLINELEN);
					if (!ced_ParseTier(ced_line)) {
						strcpy(ced_err_message, "+Error: Out of memory.");
						return(FALSE);
					}
				}
				break;
			}
		} else if (lc == '\n' && (c == '@' || c == '*' || c == '%')) {
			ced_lineC[i] = EOS;
			if (ced_lineC[0] != EOS) {
				u_strcpy(ced_line, ced_lineC, UTTLINELEN);
				if (!ced_ParseTier(ced_line)) {
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
			}
			if (feof(fp))
				break;
			ced_lineC[0] = c;
			i = 1;
		} else if (feof(fp)) {
			ced_lineC[i] = EOS;
			if (ced_lineC[0] != EOS) {
				u_strcpy(ced_line, ced_lineC, UTTLINELEN);
				if (!ced_ParseTier(ced_line)) {
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
			}
			break;
		} else
			ced_lineC[i++] = c;
		lc = c;
	}
	
	CHECK_IN_TIERS *ts;
	TPLATES *tp;
	for (i=0; i < 256; i++)
		isItemPrefix[i] = FALSE;

	for (ts=ced_maint; ts != NULL; ts = ts->nexttier) {
		if (*ts->code == '*') {
			for (tp=ts->tplate; tp != NULL; tp = tp->nextone) {
				if (*tp->pattern == '*' && *(tp->pattern+1) != EOS) {
					if (uS.ismorfchar(tp->pattern, 1, &tFnt, CHECK_MORPHS, TRUE)) {
						isItemPrefix[tp->pattern[1]] = FALSE;
					} else {
						i = strlen(tp->pattern) - 1;
						if (uS.ismorfchar(tp->pattern, i, &tFnt, CHECK_MORPHS, TRUE))
							isItemPrefix[tp->pattern[i]] = TRUE;
					}
				}
			}
		}
	}
	for (ts=ced_codet; ts != NULL; ts = ts->nexttier) {
		if (strcmp(ts->code, "%wor") == 0) {
			for (tp=ts->tplate; tp != NULL; tp = tp->nextone) {
				if (*tp->pattern == '*' && *(tp->pattern+1) != EOS) {
					if (uS.ismorfchar(tp->pattern, 1, &tFnt, CHECK_MORPHS, TRUE)) {
						isItemPrefix[tp->pattern[1]] = FALSE;
					} else {
						i = strlen(tp->pattern) - 1;
						if (uS.ismorfchar(tp->pattern, i, &tFnt, CHECK_MORPHS, TRUE))
							isItemPrefix[tp->pattern[i]] = TRUE;
					}
				}
			}
			break;
		}
	}
	fclose(fp);
	return(TRUE);
}
/* reading in depfiles end  */

/* displaying error message start */
static char ced_adderror(int s, int e, int num, unCH *st) {
	unCH t, *tst;
	ERRLIST *te;

	if (ced_onlydata == 0) return(1);
	if (s == e) {
		if (num != 4 && num != 36 && ced_onlydata != 2) return(1);
	}
	t = st[++e];
	st[e] = EOS;
	tst = ced_mkstring(st+s);
	if (tst == NULL) {
		strcpy(ced_err_message, "+Error: Out of memory.");
		return(2);
	}

	if (ced_headerr == NULL) {
		if ((ced_headerr=NEW(ERRLIST)) == NULL) {
			free(tst);
			strcpy(ced_err_message, "+Error: Out of memory.");
			return(2);
		}
		te = ced_headerr;
	} else {
		strcpy(ced_line,st+s);
//		uS.uppercasestr(ced_line, &tFnt, TRUE);
		for (te=ced_headerr; 1; te=te->nexterr) {
			if ((!strcmp(te->st,ced_line) || ced_onlydata==2) && te->errnum==num){
				st[e] = t;
				return(0);
			} else if (te->errnum == 36 && num == 36) {
				st[e] = t;
				return(0);
			}

			if (te->nexterr == NULL) {
				if ((te->nexterr=NEW(ERRLIST)) == NULL) {
					free(tst);
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(2);
				}
				te = te->nexterr;
				break;
			}
		}
	}
	te->nexterr = NULL;
	te->st = tst;
	te->errnum = num;
	st[e] = t;
	return(1);
}

static void ced_mess(short wh) {
	int i;

	switch(wh) {
		case 1: strcpy(ced_err_message, "-Expected characters are: @ or % or *.");
				break;
		case 2: strcpy(ced_err_message, "-Missing ':' character and argument."); break;
		case 3: strcpy(ced_err_message, "-Missing either TAB or SPACE character.");
				break;
		case 4: if (!ced_onlydata) {
					strcpy(ced_err_message, "-Use TAB character instead of space character after Tier name.");
				} else {
					strcpy(ced_err_message, "-Found a space character......");
				}
				break;
		case 5: strcpy(ced_err_message, "-Colon (:) character is illegal."); break;
		case 6: strcpy(ced_err_message, "-\"@Begin\" is missing at the beginning of the file.");
				break;
		case 7: strcpy(ced_err_message, "-\"@End\" is missing at the end of the file."); break;
		case 8: strcpy(ced_err_message, "-Expected characters are: @ % * TAB SPACE.");
				break;
		case 9: strcpy(ced_err_message,  "-Tier name is too long."); break;
		case 10: strcpy(ced_err_message, "-Tier text is too long"); break;
		case 11: strcpy(ced_err_message, "-Symbol is not declared in the depfile."); break;
		case 12: strcpy(ced_err_message, "-Missing speaker name and/or role."); break;
		case 13: strcpy(ced_err_message, "-Duplicate speaker declaration."); break;
		case 14: strcpy(ced_err_message, "-Spaces before tier code.");break;
		case 15: strcpy(ced_err_message, "-Illegal role. Please see \"depfile.cut\" for list of roles."); break;
		case 16: strcpy(ced_err_message, "-Illegal use of extended characters in speaker names."); break;
		case 17: strcpy(ced_err_message, "-Tier is not declared in depfile file.");
				 break;
		case 18: strcpy(ced_err_message, "-Speaker is not specified in a participants list.");
				 break;
		case 19: strcpy(ced_err_message, "-Illegal use of delimiter in a word. Try adding space before it.");
				 break;
		case 20: strcpy(ced_err_message, "-Undeclared suffix in depfile."); break;
		case 21: strcpy(ced_err_message, "-Utterance delimiter expected."); break;
		case 22: strcpy(ced_err_message, "-Unmatched [ found on the tier."); break;
		case 23: strcpy(ced_err_message, "-Unmatched ] found on the tier."); break;
		case 24: strcpy(ced_err_message, "-Unmatched < found on the tier."); break;
		case 25: strcpy(ced_err_message, "-Unmatched > found on the tier."); break;
		case 26: strcpy(ced_err_message, "-Unmatched { found on the tier."); break;
		case 27: strcpy(ced_err_message, "-Unmatched } found on the tier."); break;
		case 28: strcpy(ced_err_message, "-Unmatched ( found on the tier."); break;
		case 29: strcpy(ced_err_message, "-Unmatched ) found on the tier."); break;
		case 30: strcpy(ced_err_message, "-Text is illegal."); break;
		case 31: strcpy(ced_err_message, "-Missing text after the colon."); break;
		case 32: strcpy(ced_err_message, "-Code is not declared in depfile.");
				 break;
		case 33: strcpy(ced_err_message, "-Either illegal date or time or symbol is not declared in depfile.");
				 break;
		case 34: strcpy(ced_err_message, "-Illegal date representation."); break;
		case 35: strcpy(ced_err_message, "-Illegal time representation."); break;
		case 36: strcpy(ced_err_message, "-Utterance delimiter must be at the end of the utterance.");
				 break;
		case 37: strcpy(ced_err_message, "-Undeclared prefix."); break;
		case 38: strcpy(ced_err_message, "-Numbers should be written out in words.");
				 break;
		case 39: strcpy(ced_err_message, "-Code tier must NOT follow header tier.");
				 break;
		case 40: strcpy(ced_err_message, "-Duplicate code tiers per one main tier are NOT allowed.");
				 break;
		case 41: strcpy(ced_err_message, "-Parentheses around words are illegal.");
				 break;
		case 42: strcpy(ced_err_message, "-Use either \"&\" or \"()\", but not both.");
				 break;
		case 43: strcpy(ced_err_message, "-The file must start with \"@Begin\" tier.");
				 break;
		case 44: strcpy(ced_err_message, "-The file must end with \"@End\" tier. Trailing blank lines possible.");
				 break;
		case 45: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-There were more @Bg than @Eg tiers found.");
				 else
					sprintf(ced_err_message, "-There were more \"%s\" than @Eg tiers found.", err_item);
				 break;
		case 46: strcpy(ced_err_message, "-This @Eg does not have matching @Bg.");
				 break;
		case 47: strcpy(ced_err_message, "-Numbers are not allowed inside words.");
				 break;
		case 48: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-Illegal character(s) found.");
				 else
					sprintf(ced_err_message, "-Illegal character(s) '%s' found.", err_item);
				 break;
		case 49: strcpy(ced_err_message, "-Upper case letters are not allowed inside a word.");
				 break;
		case 50: strcpy(ced_err_message, "-Redundant utterance delimiter."); break;
		case 51: strcpy(ced_err_message, "-Expected [ ]; < > should be followed by [ ]."); break;
		case 52: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must be preceded by text.");
				 else
					sprintf(ced_err_message, "-Item '%s' must be preceded by text.", err_item);
				 break;
		case 53: strcpy(ced_err_message, "-Only one \"@Begin\" can be in a file."); break;
		case 54: strcpy(ced_err_message, "-Only one \"@End\" can be in a file."); break;
		case 55: strcpy(ced_err_message, "-Unmatched ( found in the word."); break;
		case 56: strcpy(ced_err_message, "-Unmatched ) found in the word."); break;
		case 57: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-Please add space between word and pause symbol.");
				 else
					sprintf(ced_err_message, "-Please add space between word and pause symbol: '%s'.", err_item);
				 break;
		case 58: strcpy(ced_err_message, "-Tier name is longer than 8 characters."); break;
		case 59: sprintf(ced_err_message, "-Expected second %c%c%c character.", 0xE2, 0x80, 0xA2); break;
		case 60: sprintf(ced_err_message, "+Please use \"Tiers->ID headers\" menu to add \"@ID:\" and \"@Participants:\" tiers."); break;
		case 61: strcpy(ced_err_message, "-\"@Participants:\" tier is expected here."); break;
		case 62: strcpy(ced_err_message, "-Missing language information."); break;
		case 63: strcpy(ced_err_message, "-Missing Corpus name."); break;
		case 64: strcpy(ced_err_message, "-Wrong gender information (Choose: female or male).");
				break;
		case 65: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item can not be followed by the next symbol.");
				 else
					sprintf(ced_err_message, "-Item '%s' can not be followed by the next symbol.", err_item);
				 break;
		case 66: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-Illegal character in a word. Maybe SPACE should be added before it.");
				 else
					sprintf(ced_err_message, "-Illegal character '%s' in a word. Maybe SPACE should be added before it.", err_item);
				 break;
		case 67: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must be followed by text, preceded by SPACE or be removed.");
				 else
					sprintf(ced_err_message, "-Item '%s' must be followed by text, preceded by SPACE or be removed.", err_item);
				 break;
		case 68: strcpy(ced_err_message, "-PARTICIPANTS TIER IS MISSING \"CHI Target_Child\".");
				 break;
		case 69: strcpy(ced_err_message, "-\"@Font:\" is missing at the beginning of the file.");
				break;
		case 70: strcpy(ced_err_message, "-Expected either text or \"0\" on this tier.");
				break;
		case 71: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must be before pause (#).");
				 else
					sprintf(ced_err_message, "-Item '%s' must be before pause (#).", err_item);
				break;
		case 72: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must precede the utterance delimiter or CA delimiter.");
				 else
					sprintf(ced_err_message, "-Item '%s' must precede the utterance delimiter or CA delimiter.", err_item);
				break;
		case 73: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must be preceded by text or '0'.");
				 else
					sprintf(ced_err_message, "-Item '%s' must be preceded by text or '0'.", err_item);
				break;
		case 74: strcpy(ced_err_message, "-Only one tab after ':' is allowed.");
				break;
		case 75: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must follow after utterance delimiter.");
				 else
					sprintf(ced_err_message, "-Item '%s' must follow after utterance delimiter.", err_item);
				break;
		case 76: strcpy(ced_err_message, "-Only one letter is allowed with '@l'.");
				break;
		case 77: strcpy(ced_err_message, "-\"@Languages:\" tier is expected here."); break;
		case 78: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must be used at the beginning of tier.");
				 else
					sprintf(ced_err_message, "-Item '%s' must be used at the beginning of tier.", err_item);
				 break;
		case 79: strcpy(ced_err_message, "-Only one occurrence of | symbol per word is allowed."); break;
		case 80: strcpy(ced_err_message, "-There must be at least one occurrence of '|'."); break;
		case 81: strcpy(ced_err_message, "-Bullet must follow utterance delimiter or be followed by end-of-line.");
				break;
		case 82: strcpy(ced_err_message, "-BEG mark of bullet must be smaller than END mark.");
				break;
		case 83: strcpy(ced_err_message, "-Current BEG time is smaller than previous' tier BEG time.");
				break;
		case 84: sprintf(ced_err_message, "-Current BEG time is smaller than previous' tier END time by %ld msec.", tDiff);
				break;
		case 85: strcpy(ced_err_message, "-Gap found between current BEG time and previous' tier END time.");
				break;
		case 86: strcpy(ced_err_message, "-Illegal character. Please re-enter it using Unicode standard.");
				break;
		case 87: strcpy(ced_err_message, "-Malformed structure.");
				break;
		case 88: strcpy(ced_err_message, "-Illegal use of compounds and special form markers.");
				break;
		case 89: strcpy(ced_err_message, "-Missing or extra or wrong characters found in bullet.");
				break;
		case 90: strcpy(ced_err_message, "-Illegal time representation inside a bullet.");
				break;
		case 91: strcpy(ced_err_message, "-Blank lines are not allowed.");
				 break;
		case 92: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must be followed by SPACE or end-of-line.");
				 else
					sprintf(ced_err_message, "-Item '%s' must be followed by SPACE or end-of-line.", err_item);
				 break;
		case 93: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must be preceded by SPACE.");
				 else
					sprintf(ced_err_message, "-Item '%s' must be preceded by SPACE.", err_item);
				 break;
		case 94: strcpy(ced_err_message, "-Mismatch of speaker and %mor: utterance delimiters.");
				 break;
		case 95: strcpy(ced_err_message, "-Illegal use of capitalized words in compounds.");
				 break;
		case 96: strcpy(ced_err_message, "-Word color is now illegal.");  break;
		case 97: strcpy(ced_err_message, "-Illegal character inside parentheses.");
				 break;
		case 98: strcpy(ced_err_message, "-Space is not allow in media file name inside bullets.");
				 break;
		case 99: strcpy(ced_err_message, "-Extension is not allow at the end of media file name.");
				 break;
		case 100: strcpy(ced_err_message, "-Commas at the end of PARTICIPANTS tier are not allowed.");
				 break;
		case 101: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This item must be followed or preceded by text.");
				 else
					sprintf(ced_err_message, "-Item '%s' must be followed or preceded by text.", err_item);
				 break;
		case 102: strcpy(ced_err_message, "-Italic markers are no longer legal in CHAT."); break;
		case 103: strcpy(ced_err_message, "-Illegal use of both CA and IPA on \"@Options:\" tier."); break;
		case 104: strcpy(ced_err_message, "-Please select \"CAfont\" or \"Ascender Uni Duo\" font for CA file as per \"@Options:\" tier."); break;
		case 105: strcpy(ced_err_message, "-Please select \"Charis SIL\" font for IPA file as per \"@Options:\" tier."); break;
		case 106: strcpy(ced_err_message, "-The whole code must be on one line. Please run chstring +q on this file."); break;
		case 107: strcpy(ced_err_message, "-Only single commas are allowed in tier."); break;
		case 108: strcpy(ced_err_message, "-All postcodes must precede final bullet."); break;
		case 109: strcpy(ced_err_message, "-Postcodes are not allowed on dependent tiers."); break;
		case 110: strcpy(ced_err_message, "-No bullet found on this tier."); break;
		case 111: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-Illegal pause format. Pause has to have '.'");
				else
					sprintf(ced_err_message, "-Pause needs '.' in '%s' or this item is in wrong location.", err_item);
				break;
		case 112: sprintf(ced_err_message, "-Missing %s tier with media file name in headers section at the top of the file.", MEDIAHEADER); break;
		case 113: sprintf(ced_err_message, "-Illegal keyword, use \"audio\", \"video\" or look in depfile.cut."); break;
		case 114: sprintf(ced_err_message, "-Add \"audio\", \"video\" or look in depfile.cut for more keywords after the media file name on %s tier.", MEDIAHEADER); break;
		case 115: strcpy(ced_err_message, "-Old bullets format found. Please run \"fixbullets\" program to fix this data."); break;
		case 116: strcpy(ced_err_message, "-Specifying Font for individual lines is illegal."); break;
		case 117: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-This character must be used in pairs. See if any are unmatched.");
				else
					sprintf(ced_err_message, "-Character %s must be used in pairs. See if any are unmatched.", err_item);
				break;
		case 118: strcpy(ced_err_message, "-Utterance delimiter must precede final bullet."); break;
		case 119: strcpy(ced_err_message, "-Illegal to imbed code inside the scope of the same code."); break;
		case 120: if (ced_fav_lang[0] == EOS)
					strcpy(ced_err_message, "-Please use three letter language code.");
				else
					sprintf(ced_err_message, "-Please use \"%s\" language code instead.", ced_fav_lang);
				break;
		case 121: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-Language code not found in CLAN/lib/fixes/ISO-639.cut file. If it is legal, then add it.");
				else
					sprintf(ced_err_message, "-Language code \"%s\" not found in CLAN/lib/fixes/ISO-639.cut file. If it is legal, then add it", err_item);
				break;
		case 122: strcpy(ced_err_message, "-Language on @ID tier is not defined on \"@Languages:\" header tier."); break;
		case 123: if (err_item[0] == EOS)
					strcpy(ced_err_message, "-Illegal character found in tier text. If it CA, then add \"@Options: CA\"");
				else
					sprintf(ced_err_message, "-Illegal character '%s' found in tier text. If it CA, then add \"@Options: CA\"", err_item);
			break;
		case 124: strcpy(ced_err_message, "-Please remove \"unlinked\" from @Media header."); break;
		case 125: strcpy(ced_err_message, "-\"@Options\" header must immediately follow \"@Participants:\" header."); break;
		case 126: strcpy(ced_err_message, "-\"@ID\" header must immediately follow \"@Participants:\" or \"@Options\" header."); break;
		case 127: strcpy(ced_err_message, "-This header must immediately follow \"@ID:\" header."); break;
		case 128:
			strcpy(ced_err_message, "-Unmatched     found on the tier.");
			ced_err_message[11] = 0xE2;
			ced_err_message[12] = 0x80;
			ced_err_message[13] = 0xB9;
			break;
		case 129:
			strcpy(ced_err_message, "-Unmatched     found on the tier.");
			ced_err_message[11] = 0xE2;
			ced_err_message[12] = 0x80;
			ced_err_message[13] = 0xBA;
			break;
		case 130:
			strcpy(ced_err_message, "-Unmatched     found on the tier.");
			ced_err_message[11] = 0xE3;
			ced_err_message[12] = 0x80;
			ced_err_message[13] = 0x94;
			break;
		case 131:
			strcpy(ced_err_message, "-Unmatched     found on the tier.");
			ced_err_message[11] = 0xE3;
			ced_err_message[12] = 0x80;
			ced_err_message[13] = 0x95;
			break;
		case 132: strcpy(ced_err_message, "-Tabs should only be used to mark the beginning of lines."); break;
		case 133: sprintf(ced_err_message, "-BEG time is smaller than same speaker's previous END time by %ld msec.", tDiff);
			break;
		case 134: if (err_item[0] == EOS)
				strcpy(ced_err_message, "-This item is illegal. Please run \"mor\" command on this data.");
			else
				sprintf(ced_err_message, "-Item '%s' is illegal. Please run \"mor\" command on this data.", err_item);
			break;
		case 135: if (err_item[0] == EOS)
				strcpy(ced_err_message, "-This item is illegal.");
			else
				sprintf(ced_err_message, "-Item '%s' is illegal.", err_item);
			break;
		case 136:
			strcpy(ced_err_message, "-Unmatched     found on the tier.");
			ced_err_message[11] = 0xE2;
			ced_err_message[12] = 0x80;
			ced_err_message[13] = 0x9C;
			break;
		case 137:
			strcpy(ced_err_message, "-Unmatched     found on the tier.");
			ced_err_message[11] = 0xE2;
			ced_err_message[12] = 0x80;
			ced_err_message[13] = 0x9D;
			break;
		case 138: strcpy(ced_err_message, "-Special quote U2019 must be replaced by single quote (')."); break;
		case 139: strcpy(ced_err_message, "-Special quote U2018 must be replaced by single quote (')."); break;
		case 140: sprintf(ced_err_message, "-%%MOR: tier does not link in size to its speaker tier.");
			break;
		case 141: if (err_item[0] == EOS)
				strcpy(ced_err_message, "-[: ...] has to be preceded by only one word and nothing else.");
			else
				sprintf(ced_err_message, "-'%s' must be preceded by only one word and nothing else.", err_item);
			break;
		case 142: strcpy(ced_err_message, "-Speaker's role on @ID tier does not match role on @Participants: tier."); break;
		case 143: strcpy(ced_err_message, "-The @ID line needs 10 fields."); break;
		case 144: strcpy(ced_err_message, "-Either illegal SES field value or symbol is not declared in depfile."); break;
		case 145: strcpy(ced_err_message, "-This intonational marker should be outside paired markers."); break;
		case 146: strcpy(ced_err_message, "-The &= symbol must include some code after '=' character."); break;
		case 147: strcpy(ced_err_message, "-Undeclared special form marker in depfile."); break;
		case 148: strcpy(ced_err_message, "-Space character is not allowed before comma(,) character on \"@Media:\" header."); break;
		case 149: if (err_item[0] == EOS)
				strcpy(ced_err_message, "-Illegal character located between a word and [...] code.");
			else
				sprintf(ced_err_message, "-Illegal character '%s' located between a word and [...] code.", err_item);
			break;
		case 150: strcpy(ced_err_message, "-Illegal item located between a word and [...] code."); break;
		case 151: strcpy(ced_err_message, "-This word has only repetition segments."); break;
		case 152: strcpy(ced_err_message, "-Language is not defined on \"@Languages:\" header tier."); break;
		case 153: strcpy(ced_err_message, "-Age's month or day are missing initial zero.\nPlease run \"chstring +q +1\" command on this file to fix this error."); break;
		case 154: strcpy(ced_err_message, "-Please add \"unlinked\" to @Media header."); break;
		case 155: if (err_item[0] == EOS)
				strcpy(ced_err_message, "-Please use \"0word\" instead of \"(word)\".");
			else {
				i = strlen(err_item) - 1;
				if (i > 0)
					err_item[i] = EOS;
				sprintf(ced_err_message, "-Please use \"0%s\" instead of \"(%s)\".", err_item+1, err_item+1);
			}
			break;
		case 156:
			strcpy(ced_err_message, "-Please replace ,, with F2-t (   ) character .");
			ced_err_message[30] = 0xE2;
			ced_err_message[31] = 0x80;
			ced_err_message[32] = 0x9D;
			break;
		case 157: strcpy(ced_err_message, "-Media file name has to match datafile name."); break;
		case 158: strcpy(ced_err_message, "-[: ...] has to have real word, not 0... or &... or xxx."); break;
		case 159: strcpy(ced_err_message, "-Pause markers should appear after retrace markers"); break;

		default: strcpy(ced_err_message, "+Internal ERROR. undefined error code!"); break;
	}
}

static char check_err(int num, int s, int e, unCH *templine) {
	long i, j, len;
	char res;
	unCH err_itemU[512];

	if (isdone == 3 && num != 45)
		return(FALSE);
	len = strlen(templine);
	if (s > len)
		s = len;
	if (e > len)
		e = len;
	res = ced_adderror(s,e,num,templine);
	if (res == 2)
		return(TRUE);
	else if (res) {
		if (Check_level == 1) {
			if (s > -1) {
				ced_cursorRange.location = begTPos + s + ced_CurSpLen;
				ced_cursorRange.length = (begTPos + e + 1 + ced_CurSpLen) - ced_cursorRange.location;
			} else {
				ced_cursorRange.location = begTPos;
				ced_cursorRange.length = 0;
			}
		} else {
			if (s > -1) {
				ced_cursorRange.location = begTPos + s;
				ced_cursorRange.length = (begTPos + e + 1) - ced_cursorRange.location;
				if (s == e && num != 48 && num != 123 && num != 132) {
//					global_dfC->col_win2 = -2L;
//					global_dfC->col_chr2 = -2L;
				}
			} else {
				ced_cursorRange.location = begTPos;
				ced_cursorRange.length = 0;
			}
		}
/*
		if (ced_cursorRange.location > begTPos + strlen(templine)) {
			ced_cursorRange.location = begTPos + strlen(templine);
			if (ced_cursorRange.location - begTPos > 0 && templine[ced_cursorRange.location - begTPos-1] == '\n') {
				ced_cursorRange.location--;
			}
		}
*/
/*
		if (global_dfC->col_win2 != -2) {
			if (global_dfC->col_win == global_dfC->col_win2 && //global_dfC->row_txt->line[global_dfC->col_chr2-1] == '\n' &&
				global_dfC->row_txt->line[global_dfC->col_chr2] == EOS) {
				global_dfC->col_chr2--;
				global_dfC->col_win2 = ComColWin(FALSE, global_dfC->row_txt->line, global_dfC->col_chr2);
				if (global_dfC->col_chr2 <= global_dfC->col_chr) {
					global_dfC->col_win2 = -2L;
					global_dfC->col_chr2 = -2L;
				}
			}
		}
*/
		if (num != 45) {
			j = 0L;
			if (ced_cursorRange.length > 0) {
				for (i=s; j < 15 && i <= e; i++) {
					if (templine[i] == '\n' || templine[i] == EOS)
						break;
					if (templine[i] == CED_HIDEN_C) {
						strcpy(err_itemU, "<bullet>");
						j = strlen(err_itemU);
						break;
					} else if (templine[i] == ' ' && j < 1) {
						strcpy(err_itemU+j, "Space");
						j += 5;
					} else if (templine[i] == '\t' && j < 1) {
						strcpy(err_itemU+j, "Tab");
						j += 3;
					} else {
						err_itemU[j++] = templine[i];
					}
				}
			}
			err_itemU[j] = EOS;
			u_strcpy(err_item, err_itemU, 510);
		}
		ced_mess(num);
		return(TRUE);
	} else
		return(FALSE);
}

static char ced_CodeErr(short wh) {
	long  j;
	char res;

	if (isdone == 3 && wh != 46)
		return(FALSE);
	for (j=0; ced_sp[j] != ':' && ced_sp[j]; j++) ;
	res = ced_adderror(0,j,wh,ced_sp);
	if (res == 2)
		return(TRUE);
	else if (res) {
		if (Check_level == 1) {
		}
		ced_cursorRange.location = begTPos;
		ced_cursorRange.length = j;
		if (ced_cursorRange.location > begTPos + strlen(templine)) {
			ced_cursorRange.location = begTPos + strlen(templine);
			if (ced_cursorRange.location - begTPos > 0 && templine[ced_cursorRange.location - begTPos-1] == '\n') {
				ced_cursorRange.location--;
			}
		}
		j = 0L;
		err_item[j] = EOS;
		ced_mess(wh);
		return(TRUE);
	} else
		return(FALSE);
}
/*
static void DisplayErrorRow(void) {
	FindMidWindow();
//	TRUE_CHECK_ID1(global_dfC->row_txt->flag);
//	TRUE_CHECK_ID2(global_dfC->row_txt->flag);
	DisplayTextWindow(NULL, 1);
	GetCurCode();
	FindRightCode(1);
}
*/
/* displaying error message end */

/* main check functions start */
static CHECK_IN_TIERS *ced_MatchTierName(CHECK_IN_TIERS *ts, unCH *c, unCH col) {
	CHECK_IN_TIERS *ts_old;

	ts_old = NULL;
	for (; ts != NULL; ts = ts->nexttier) {
		if (ced_u.check_partcmp(c,ts->code,TRUE)) {
			ts_old = ts;
			if (col == ts->col)
				return(ts);
		}
	}
	ts = ts_old;
	return(ts);
}

static char ced_FoundText(int i, unCH *word) {
	for (; isSpace(word[i]); i++) ;
	return(word[i] != EOS);
}

static char ced_FoundAlNums(int i, unCH *word) {
	for (; (!is_unCH_alnum(word[i]) || word[i] < 1 || word[i] > 127) && word[i] != EOS; i++) ;
	return(word[i] == EOS);
}

static char isCATime(unCH *word) {
	register int i;

	for (i=0; is_unCH_digit(word[i]); i++) ;
	for (; isSpace(word[i]); i++) ;
	for (; is_unCH_digit(word[i]) || word[i] == '(' || word[i] == ')' || 
		word[i] == '.' || isSpace(word[i]); i++) ;
	return(word[i] == EOS);
}

static char ced_Tabs(unCH *line) {
	long i;

	for (i=0L; line[i] != EOS; i++) {
		if (line[0] != '\t') {
			if (ced_isCAFound && line[i] == '\t' && i > 0 && line[i-1] != ':') {
				if (check_err(132,i,i,line))
					return(FALSE);
			}
			if (!ced_isCAFound && line[i] == '\t' && i > 0 && line[i-1] == ':' && line[i+1] == ' ') {
				if (check_err(123,i+1,i+1,line))
					return(FALSE);
			}
			if (!ced_isCAFound && line[i] == ' ' && i == 1 && line[i-1] == '\t') {
				if (check_err(123,i,i,line))
					return(FALSE);
			}
			if (line[i] == '\t' && i > 0 && line[i-1] != ':') {
				if (check_err(132,i,i,line))
					return(FALSE);
			}
		}
		if (line[i] == ' ' && i == 0) {
			if (check_err(4,i,i,line))
				return(FALSE);
		}
	}
	return(TRUE);
}

static char ced_Colon(unCH *line) {
	register int  i;
	register int  j;
	register char blf;
	CHECK_IN_TIERS *ts;

	if (isSpeaker(*line)) {
		i = 0;
		j = i;
		blf = (line[i] == CED_HIDEN_C);
		for (; line[i] && (line[i] != ':' || blf); i++) {
			if (line[i] == CED_HIDEN_C)
				blf = !blf;
		}
		strcpy(ced_line,line);
		uS.remblanks(ced_line);
//		uS.uppercasestr(ced_line, &tFnt, TRUE);
		if (*line == '@') 
			ts = ced_MatchTierName(ced_headt,ced_line,(line[i] ==':'));
		else if (*line == '*') 
			ts = ced_MatchTierName(ced_maint,ced_line,(line[i] ==':'));
		else if (*line == '%') 
			ts = ced_MatchTierName(ced_codet,ced_line,(line[i] ==':'));
		else
			ts = NULL;

		if (line[i] != ':') {
			if (ts != NULL) {
				if (ts->col) {
					if (*line == '@') {
						i = strlen(ts->code);
						while (!isSpace(line[i]) && line[i] != '\n' && line[i] != EOS)
							i++;
						if (check_err(2,i,i,line))
							return(FALSE);
					} else if (isSpace(line[4])) {
						if (check_err(2,4,4,line)) return(FALSE);
					} else if (check_err(2,i,i,line)) return(FALSE);
				} else if (ced_FoundText(strlen(ts->code), line)) {
					if (check_err(30,strlen(ts->code),i,line)) return(FALSE);
				}
			} else if (*line == '*' || *line == '%') {
				if (isSpace(line[4])) {
					if (check_err(2,4,4,line)) return(FALSE);
				} else 
					if (check_err(2,i,i,line)) return(FALSE);
			} else if (!isCATime(line)) {
				if (check_err(17,i,i,line)) return(FALSE);
			}
		} else {
			i++;
			if (ts != NULL) {
				if (!ts->col) {
					if (check_err(5,i-1,i-1,line)) return(FALSE);
					i = -1;
				} else if (!ced_FoundText(i, line) && *line != '@'/* to always allow empty header tiers*/) {
					if (check_err(31,i,strlen(line),line)) return(FALSE);
					i = -1;
				}
			}
			if (i == -1) ;
			else if (!ced_FoundText(i, line) && *line == '@'/* to always allow empty header tiers*/) ;
			else if (!isSpace(line[i])) { 
				if (check_err(3,i,i,line)) return(FALSE);
			} else if (1 /* *line != '@' */) {
				if (line[i] == ' ') { 
					if (check_err(4,i,i,line)) return(FALSE);
				} else if (isSpace(line[i+1])) {
// 2004-04-27					if (check_err(74,i+1,i+1,line)) return(FALSE);
				}
			}
		}
	}
	return(TRUE);
}

static char ced_CheckErr(unCH *line) {
	int i, s, e;

	if (isSpace(*line)) {
		s = 0;
		for (i=0; isSpace(line[i]); i++) ;
		e = i - 1;
		if (isSpeaker(line[i])) {
			for (; line[i] && i < 79 && line[i] != ':'; i++) ;
			if (line[i] == ':' && *line != '\t') {
				if (check_err(14,s,e,line)) return(FALSE);
			}
		}
	}
	return(TRUE);
}

/*
static void ced_checkFixAge(unCH *line, int i) {
	int cnt;

	cnt = 0;
	for (; line[i] != EOS; i++) {
		if (line[i] == '|') {
			cnt++;
			if (cnt == 3) {
				for (i++; line[i] != EOS && line[i] != '|'; i++) {
					if (line[i] == ';' && isdigit(line[i+1]) && line[i+2] == '.') {
						i++;
						ResetUndos();
						global_dfC->col_chr = i;
						ChangeCurLine();
						AddText(NULL, '0', TRUE, 1L);
						while (global_dfC->col_txt != global_dfC->tail_row && global_dfC->col_txt->c != '|') {
							if (global_dfC->col_txt->c == '.') {
								global_dfC->col_txt = global_dfC->col_txt->next_char;
								global_dfC->col_chr++;
								if (global_dfC->col_txt != global_dfC->tail_row && isdigit(global_dfC->col_txt->c)) {
									if (global_dfC->col_txt->next_char != global_dfC->tail_row && global_dfC->col_txt->next_char->c == '|') {
										AddText(NULL, '0', TRUE, 1L);
										return;
									}
								}
							}
							global_dfC->col_txt = global_dfC->col_txt->next_char;
							global_dfC->col_chr++;
						}
						return;
					} else if (line[i] == '.' && isdigit(line[i+1]) && line[i+2] == '|') {
						i++;
						ResetUndos();
						global_dfC->col_chr = i;
						ChangeCurLine();
						AddText(NULL, '0', TRUE, 1L);
						return;
					}
				}
			}
		}
	}
}
*/

static void fillup_templine(NSString *textSt, int len, unCH *line, BOOL isTier) {
	int j;
	unichar ch;

	begTPos = textPos;
	j = 0;
	while (textPos < len) {	// Run through the whole text in NSTextStorage *text
		ch = [textSt characterAtIndex:textPos];
		if (j < UTTLINELEN)
			line[j++] = ch;
		if (ch == '\n') {
			if (isTier == false) {
				textPos++;
				break;
			}
			if (textPos+1 < len) {
				ch = [textSt characterAtIndex:textPos+1];
				if (ch == '*' || ch == '%' || ch == '@') {
					textPos++;
					break;
				}
			}
		}
		textPos++;
	}
	line[j] = EOS;
//	uS.remblanks(line);
}

static char check_getwholeutter(NSString *textSt, int len) {
	int i;
	char isDashFound;
	char tCheck_level;

	*ced_sp = EOS;
	*templine = EOS;
	fillup_templine(textSt, len, templine4, true);
	if (isSpeaker(templine4[0])) {
		isDashFound = FALSE;
		for (i=0; templine4[i] != ':' && templine4[i]; i++) {
			if (i >= SPEAKERLEN) {
				*ced_sp = EOS;
				*templine = EOS;
				check_err(9,-1,-1,cl_T(""));
				return(-1);
			}
			ced_sp[i] = templine4[i];
			if (ced_sp[i] == '-') {
				isDashFound = TRUE;
				if (i > 7+1 && (ced_sp[0] == '*' || ced_sp[0] == '%')) {
					ced_sp[i] = EOS;
					*templine = EOS;
					tCheck_level = Check_level;
					Check_level = 0;
					isdone = 1;
					check_err(58,1,i-1,ced_sp);
					Check_level = tCheck_level;
					return(-1);
				}
			}
		}
		if (!isDashFound) {
			if (i > 7+1 && (ced_sp[0] == '*' || ced_sp[0] == '%')) {
				ced_sp[i] = EOS;
				*templine = EOS;
				tCheck_level = Check_level;
				Check_level = 0;
				isdone = 1;
				check_err(58,1,i-1,ced_sp);
				Check_level = tCheck_level;
				return(-1);
			}
		}
		ced_sp[i] = templine4[i];
		if (ced_sp[i]) {
			ced_sp[++i] = EOS;
			if ((len=strlen(templine+i)) >= UTTLINELEN) {
				*ced_sp = EOS;
				*templine = EOS;
				check_err(10,-1,-1,cl_T(""));
				return(-1);
			}
			strcpy(templine, templine4+i);
			if (ced_u.check_partcmp(ced_sp,IDOF,FALSE)) {
//				ced_checkFixAge(templine, i);
			}
		}
		ced_CurSpLen = i;
		uS.remblanks(ced_sp);
		strcpy(ced_line, ced_sp);
//		uS.uppercasestr(ced_line, &tFnt, TRUE);

		if (ced_u.check_partcmp(ced_line,"%pho:",FALSE) ||
			ced_u.check_partcmp(ced_line,"%mod:",FALSE)) {
			init_punct(1);
		} else {
			init_punct(0);
		}

//		uS.remblanks(templine);
	} else
		return(0);
/*
	len = strlen(templine);
	if (len == 0)
		strcat(templine, "\n");
	else if (templine[len-1] != '\n')
		strcat(templine, "\n");
*/
	return(2);
}

static void ced_CleanTierNames(unCH *st) {
	register int i;

	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == '\n')) i--;
	if (i >= 0 && st[i] == ':') i--;
	if (*st == '*') {
		int j = i;
		while (i >= 0 && st[i] != '-') i--;
		if (i < 0) i = j;
		else i--;
	}
	st[i+1] = EOS;
}

static char ced_correctMonth(unCH *s) {
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

static int ced_SES_item(unCH *tmpl, unCH *s) {
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

static int ced_MatchDT(unCH *tmpl, unCH *s) {
	char time = FALSE;

	if (*tmpl == 't')
		time = TRUE;
	for (tmpl++; *tmpl; tmpl++, s++) {
		if (*tmpl == 'd' && is_unCH_digit(*s)) {
			if (*(tmpl-1) != 'd') {
				if (is_unCH_digit(*(s+1)) && *s >= '0' && *s <= '3')
					;
/* no 1-AUG-1980 allowed, only 01-AUG-1980
				else if (!is_unCH_digit(*(s+1)) && *s >= '0' && *s <= '9')
					tmpl++;
*/
				else if (!is_unCH_digit(*(s+1)) && *(tmpl+1) != 'd')
					;
				else
					return(FALSE);
			}
		} else if (*tmpl == 'h' && is_unCH_digit(*s)) {
			if (*(tmpl-1) != 'h') {
				if (is_unCH_digit(*(s+1)) && *s >= '0' && *s <= '2')
					;
				else if (!is_unCH_digit(*(s+1)))
					tmpl++;
				else
					return(FALSE);
			} else if (*(s-1) == '1' && (*s < '0' || *s > '9'))
				return(FALSE);
			else if (*(s-1) == '2' && (*s < '0' || *s > '4'))
				return(FALSE);
		} else if (*tmpl == 'm' && is_unCH_digit(*s)) {
			if (*(tmpl-1) != 'm') {
				if (is_unCH_digit(*(s+1)) && *s >= '0') {
					if ((*s > '1' && !time) || *s > '5')
						return(FALSE);
					if (*s == '1' && *(s+1) > '2' && !time)
						return(FALSE);
				} else if (!is_unCH_digit(*(s+1)))
					tmpl++;
				else
					return(FALSE);
			}
		} else if (*tmpl == 's' && is_unCH_digit(*s)) {
			if (*(tmpl-1) != 's') {
				if (is_unCH_digit(*(s+1)) && *s >= '0' && *s <= '5')
					;
				else if (!is_unCH_digit(*(s+1)))
					tmpl++;
				else
					return(FALSE);
			}
		} else if (*tmpl == 'l' && is_unCH_alpha(*s)) {
			if (tmpl[1] == 'l' && tmpl[2] == 'l') {
				if (!ced_correctMonth(s)) {
					return(FALSE);
				}
			}
		} else if (*tmpl == 'y' && is_unCH_digit(*s)) {
			if (*(tmpl-1) != 'y' && !is_unCH_digit(*(s+1)))
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

static int ced_matchplate(TPLATES *tp, unCH *s, char charType) {
	char dt = FALSE, ses = FALSE;

	for (; tp != NULL; tp = tp->nextone) {
		if (*tp->pattern == '@') {
			if ((*(tp->pattern+1) == 'd' || *(tp->pattern+1) == 't') && charType == 'D') {
				dt = TRUE;
				if (ced_MatchDT(tp->pattern+1,s))
					return(FALSE);
				else if (tp->nextone == NULL) {
					if (*(tp->pattern+1) == 'd')
						return(34);
					else
						return(35);
				}
			} else if ((*(tp->pattern+1) == 's' || *(tp->pattern+1) == 'e') && charType == 's') {
				ses = TRUE;
				if (ced_SES_item(tp->pattern+2,s))
					return(FALSE);
			} else if (ced_u.check_patmat(s,tp->pattern))
				return(FALSE);
		} else if (*tp->pattern == '*' && *(tp->pattern+1) == EOS) {
			if (*s == '(')
				return(FALSE);
			if (dt) {
				if (is_unCH_alpha(*s) || (*s < 1 && *s != EOS) || *s > 127)
					return(FALSE);
			} else if (is_unCH_alnum(*s) || (*s < 1 && *s != EOS) || *s > 127)
				return(FALSE);
			else if (*s == '^') {
				if (is_unCH_alpha(*(s+1)) || (*(s+1) < 1 && *(s+1) != EOS) || *(s+1)> 127)
					return(FALSE);
			}
		} else if (tp->pattern[0] == '[' && uS.isSqCodes(s, templine3, &tFnt, FALSE)) {
			if (ced_u.check_patmat(templine3,tp->pattern))
				return(FALSE);
		} else if (*tp->pattern != '*') {
			if (ced_u.check_patmat(s,tp->pattern))
				return(FALSE);
		}
	}
	if (dt) {
		return(33);
	} else if (ses) {
		return(144);
	} else if (*s == '$') {
		if (ced_isBlob)
			return(FALSE);
		else
			return(32);
	} else {
		if (ced_isBlob)
			return(FALSE);
		else
			return(11);
	}
}

static int ced_matchIDsp(struct IDsp *tp, unCH *s) {
	for (; tp != NULL; tp = tp->nextsp) {
		if (uS.mStricmp(s,tp->sp) == 0) {
			return(TRUE);
		}
	}
	return(FALSE);
}

static int ced_matchSpeaker(SPLIST *tp, unCH *s, char isFromID) {
	for (; tp != NULL; tp = tp->nextsp) {
		if (ced_u.check_patmat(s,tp->sp)) {
			if (isFromID)
				tp->hasID = TRUE;
			return(FALSE);
		}
	}
	return(TRUE);
}

static int ced_matchRole(unCH *sp, SPLIST *tp, unCH *s) {
	for (; tp != NULL; tp = tp->nextsp) {
		if (ced_u.check_patmat(sp,tp->sp)) {
			if (uS.mStricmp(tp->role, s) == 0)
				return(FALSE);
			else
				return(TRUE);
		}
	}
	return(TRUE);
}

static char ced_setLastTime(unCH *s, long t) {
	SPLIST *tp;

	if (*s == '*')
		s++;
	else if (*s == '%' || *s == '@')
		return(FALSE);
	for (tp=ced_headsp; tp != NULL; tp=tp->nextsp) {
		if (uS.mStricmp(s,tp->sp) == 0) {
			tp->endTime = t;
			return(TRUE);
		}
	}
	return(FALSE);
}

static long ced_getLatTime(unCH *s) {
	SPLIST *tp;

	if (*s == '*')
		s++;
	else if (*s == '%' || *s == '@')
		return(-2L);
	for (tp=ced_headsp; tp != NULL; tp=tp->nextsp) {
		if (uS.mStricmp(s,tp->sp) == 0) {
			return(tp->endTime);
		}
	}
	return(-1L);
}

static CHECK_IN_TIERS *ced_FindTier(CHECK_IN_TIERS *ts, unCH *c) {
	for (; ts != NULL; ts = ts->nexttier) {
		if (*c == *ts->code) {
			if (ced_u.check_patmat(c+1,ts->code+1))
				return(ts);
			else if (*c == '*' && *ts->code == '*')
				return(ts);
		}
	}
	return(NULL);
}

static int ced_badrole(unCH *s, unCH *c) {
	CHECK_IN_TIERS *ts;

	strcpy(ced_line,s);
//	uS.uppercasestr(ced_line, &tFnt, TRUE);
	if ((ts=ced_FindTier(ced_headt,c)) != NULL) {
		if (ced_matchplate(ts->tplate, ced_line, 'D') == FALSE)
			return(FALSE);
	}
	return(TRUE);
}

static char ced_trans_err(int wh, unCH *sp, unCH *ep, unCH *line) {
	int s, e;

	s = (int)((unsigned long)(sp)-(unsigned long)(line));
	e = (int)((unsigned long)(ep)-(unsigned long)(line));
	s /= 2;
	e /= 2;
	return(check_err(wh, s, e, line));
}

static char ced_addIDdsp(unCH *s) {
	unCH *tst;
	struct IDsp *tp;

	strcpy(ced_line,s);
	tst = ced_mkstring(ced_line);
	if (tst == NULL) {
		strcpy(ced_err_message, "+Error: Out of memory.");
		return(FALSE);
	}
	if (ced_idsp == NULL) {
		if ((ced_idsp=NEW(struct IDsp)) == NULL) {
			strcpy(ced_err_message, "+Error: Out of memory.");
			free(tst);
			return(FALSE);
		}
		tp = ced_idsp;
	} else {
		for (tp=ced_idsp; 1; tp=tp->nextsp) {
			if (!uS.mStricmp(tp->sp,ced_line)) {
				free(tst);
				return(TRUE);
			}
			if (tp->nextsp == NULL) {
				if ((tp->nextsp=NEW(struct IDsp)) == NULL) {
					free(tst);
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
				tp = tp->nextsp;
				break;
			}
		}
	}
	tp->nextsp = NULL;
	tp->sp = tst;
	return(TRUE);
}

static char ced_addsp(unCH *s, unCH *role, unCH *line, unCH *templine, unCH t) {
	unCH *tst, *trole;
	SPLIST *tp;

	if (role == NULL)
		trole = NULL;
	else {
		strcpy(ced_line,role);
		trole = ced_mkstring(ced_line);
		if (trole == NULL) {
			strcpy(ced_err_message, "+Error: Out of memory.");
			*line = t;
			free(trole);
			return(FALSE);
		}
	}
	strcpy(ced_line,s);
	tst = ced_mkstring(ced_line);
	if (tst == NULL) {
		strcpy(ced_err_message, "+Error: Out of memory.");
		*line = t;
		return(FALSE);
	}
	if (ced_headsp == NULL) {
		if ((ced_headsp=NEW(SPLIST)) == NULL) {
			strcpy(ced_err_message, "+Error: Out of memory.");
			*line = t;
			free(tst);
			free(trole);
			return(FALSE);
		}
		tp = ced_headsp;
	} else {
		for (tp=ced_headsp; 1; tp=tp->nextsp) {
			if (!uS.mStricmp(tp->sp,ced_line)) {
				free(tst);
				if (role == NULL) {
					*line = t;
					if (ced_trans_err(13,s,line-1,templine)) {
						free(trole);
						return(FALSE);
					}
				} else if (tp->role == NULL)
					tp->role = trole;
				else
					free(trole);
				return(TRUE);
			}
			if (tp->nextsp == NULL) {
				if ((tp->nextsp=NEW(SPLIST)) == NULL) {
					*line = t;
					free(tst);
					free(trole);
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
				tp = tp->nextsp;
				break;
			}
		}
	}
	tp->nextsp = NULL;
	tp->sp = tst;
	tp->role = trole;
	tp->endTime = 0L;
	tp->hasID = FALSE;
	return(TRUE);
}

static char ced_getpart(unCH *line) {
	int i;
	unCH sp[SPEAKERLEN];
	unCH *rootline = line;
	unCH *s, *e, t;
	char wc, tchFound, isCommaFound;
	short cnt = 0;

	ced_clean_speakers(ced_headsp);
	ced_headsp = NULL;
	for (; *line && isSpace(*line); line++) ;
	s = line;
	tchFound = FALSE;
	isCommaFound = FALSE;
	sp[0] = EOS;
	while (*s) {
		if (*line == ','  || isSpace(*line)  || *line == '\n' || *line == EOS) {

			if (*line == ',')
				isCommaFound = TRUE;
			if (*line == ',' && !isSpace(*(line+1)) && *(line+1) != '\n') {
				if (ced_trans_err(92,line,line,rootline))
					return(FALSE);
			}

			wc = ' ';
			e = line;
			for (; isSpace(*line) || *line == '\n'; line++) ;
			if (*line != ',' && *line != EOS)
				line--;
			else
				wc = ',';
			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == ',') {
					if (NAME_ROLE_COUNT(ced_GenOpt)) {
						if (!strcmp(sp, "CHI") && !strcmp(s, "Target_Child"))
							tchFound = TRUE;
					}
					if (ced_badrole(s,cl_T(PARTICIPANTS))) {
						*e = t;
						if (ced_trans_err(15,s,e-1,rootline))
							return(FALSE);
					}
					if (!ced_addsp(sp,s,line,rootline,t))
						return(FALSE);
				} else if (cnt == 0) {
					for (i=0; s[i] > 32 && s[i] < 127 && s[i] != EOS; i++) ;
					if (s[i] != EOS) {
						if (ced_trans_err(16,line-strlen(s),line-1,rootline))
							return(FALSE);
					} else {
						isCommaFound = FALSE;
						strcpy(sp, s);
						if (!ced_addsp(s,NULL,line,rootline,t))
							return(FALSE);
					}
				}
				*e = t;
				if (wc == ',') {
					if (cnt == 0) {
						if (ced_trans_err(12,line,line,rootline))
							return(FALSE);
					}
					cnt = -1;
					sp[0] = EOS;
				}
				for (line++; isSpace(*line) || *line=='\n' || *line==','; line++) {
					if (*line == ',') {
						if (cnt == 0) {
							if (ced_trans_err(12,line,line,rootline))
								return(FALSE);
						}
						cnt = -1;
						sp[0] = EOS;
					}
				}
			} else {
				for (line=e; *line; line++) ;
				if (cnt == 0) {
					if (ced_trans_err(12,e,e,rootline))
						return(FALSE);
				} else {
					t = *e;
					*e = EOS;
					if (NAME_ROLE_COUNT(ced_GenOpt)) {
						if (!strcmp(sp, "CHI") && !strcmp(s, "Target_Child"))
							tchFound = TRUE;
					}

					if (ced_badrole(s,cl_T(PARTICIPANTS))) {
						if (ced_trans_err(15,s,e-1,rootline))
							return(FALSE);
					}
					if (!ced_addsp(sp,s,line,rootline,t))
						return(FALSE);
					*e = t;
				}
				for (line=e; *line; line++) ;
			}
			cnt++;
			s = line;
		} else
			line++;
	}
	if (isCommaFound) {
		if (check_err(100,-1,-1,rootline))
			return(FALSE);
	}
	if (NAME_ROLE_COUNT(ced_GenOpt) && !tchFound && ced_applyG2Option) {
		if (check_err(68,-1,-1,rootline))
			return(FALSE);
	}
	return(TRUE);
}

static void ced_getExceptions(unCH *line) {
	unCH *s, *e, t;
	TPLATES *tp = ced_WordExceptions;

	for (; *line && (*line == ' ' || *line == '\t'); line++) ;
	s = line;
	while (*s) {
		if (isSpace(*line) || *line == '\n' || *line == EOS) {
			e = line;
			for (; isSpace(*line) || *line == '\n'; line++) ;
			t = *e;
			*e = EOS;
			if (tp == NULL) {
				ced_WordExceptions = NEW(TPLATES);
				tp = ced_WordExceptions;
			} else {
				tp->nextone = NEW(TPLATES);
				tp = tp->nextone;
			}
			if (tp != NULL) {
				tp->nextone = NULL;
				tp->pattern = ced_mkstring(s);
			} else
				return;
			*e = t;
			s = line;
		} else
			line++;
	}
}

static char ced_Age(CHECK_IN_TIERS *ts, unCH *sp, unCH *templine, int offset, char eChar) {
	int r, s, e;
	unCH *st;

	if (sp != NULL) {
		uS.extractString(templine4, sp, "@Age of ", ':');
		if (ced_matchSpeaker(ced_headsp, templine4, FALSE)) {
			if (ced_CodeErr(18))
				return(FALSE);
		}
	}
	s = offset;
	e = s;
	while (1) {
		st = ced_line;
		while ((*st=templine[s++]) == '\n' || isSpace(*st)) ;
		if (*st == eChar)
			break;
		e = s;
		while ((*++st=templine[e]) != eChar) {
			e++;
			if (*st == '\n' || isSpace(*st))
				break;
		}
		*st = EOS;
		if ((r=ced_matchplate(ts->tplate, ced_line, 'D'))) {
			if (eChar == EOS)
				offset = e - 2;
			else
				offset = e - 1;
			if (check_err(r,s-1,offset,templine))
				return(FALSE);
		}
		s = e;
	}
	return(TRUE);
}

static char ced_Sex(CHECK_IN_TIERS *ts, unCH *sp, unCH *templine) {
	unCH t;
	int s, e;

	if (sp != NULL) {
		uS.extractString(templine4, sp, "@Sex of ", ':');
		if (ced_matchSpeaker(ced_headsp, templine4, FALSE)) {
			if (ced_CodeErr(18))
				return(FALSE);
		}
	}
	for (s=0; isSpace(templine[s]); s++) ;
	if (templine[s] != EOS && templine[s] != '\n') {
		for (e=s; templine[e] != '\n' && templine[e] != EOS; e++) ;
		t = templine[e];
		templine[e] = EOS;
		if (strcmp(templine+s, "male") && strcmp(templine+s, "female")) {
			templine[e] = t;
			if (check_err(64,s,e,templine)) return(FALSE);
		}
		templine[e] = t;
	}
	return(TRUE);
}

static char ced_ses(CHECK_IN_TIERS *ts, unCH *sp, unCH *templine, int offset, char eChar) {
	int r, s, e;
	unCH *st;

	if (sp != NULL) {
		uS.extractString(templine4, sp, "@Ses of ", ':');
		if (ced_matchSpeaker(ced_headsp, templine4, FALSE)) {
			if (ced_CodeErr(18))
				return(FALSE);
		}
	}
	s = offset;
	e = s;
	while (1) {
		st = ced_line;
		while ((*st=templine[s++]) == '\n' || *st == ',' || isSpace(*st)) ;
		if (*st == eChar)
			break;
		e = s;
		while ((*++st=templine[e]) != eChar) {
			e++;
			if (*st == '\n' || *st == ',' || isSpace(*st))
				break;
		}
		*st = EOS;
		if ((r=ced_matchplate(ts->tplate, ced_line, 's'))) {
			if (eChar == EOS)
				offset = e - 2;
			else
				offset = e - 1;
			if (check_err(r,s-1,offset,templine))
				return(FALSE);
		}
		s = e;
	}
	return(TRUE);
}

static char ced_Birth(CHECK_IN_TIERS *ts, unCH *sp, unCH *templine) {
	int r, s, e;
	unCH *st;

	if (sp != NULL) {
		uS.extractString(templine4, sp, "@Birth of ", ':');
		if (ced_matchSpeaker(ced_headsp, templine4, FALSE)) {
			if (ced_CodeErr(18))
				return(FALSE);
		}
	}
	s = 0;
	e = s;
	while (1) {
		st = ced_line;
		while ((*st=templine[s++]) == '\n' || isSpace(*st)) ;
		if (*st == EOS)
			break;
		e = s;
		while ((*++st=templine[e]) != EOS) {
			e++;
			if (*st == '\n' || isSpace(*st))
				break;
		}
		*st = EOS;
//		uS.uppercasestr(ced_line, &tFnt, TRUE);
		if ((r=ced_matchplate(ts->tplate, ced_line, 'D'))) {
			if (check_err(r,s-1,e - 2,templine))
				return(FALSE);
		}
		s = e;
	}
	return(TRUE);
}

static char ced_Educ(CHECK_IN_TIERS *ts, unCH *sp, unCH *templine) {
	if (sp != NULL) {
		uS.extractString(templine4, sp, "@Education of ", ':');
		if (ced_matchSpeaker(ced_headsp, templine4, FALSE)) {
			if (ced_CodeErr(18))
				return(FALSE);
		}
	}
	return(TRUE);
}

static char ced_Group(CHECK_IN_TIERS *ts, unCH *sp, unCH *templine) {
	if (sp != NULL) {
		uS.extractString(templine4, sp, "@Group of ", ':');
		if (ced_matchSpeaker(ced_headsp, templine4, FALSE)) {
			if (ced_CodeErr(18))
				return(FALSE);
		}
	}
	return(TRUE);
}

static int ced_matchnumber(TPLATES *tp, unCH *s) {
	for (; tp != NULL; tp = tp->nextone) {
		if (*tp->pattern != '*') {
			if (ced_u.check_patmat(s,tp->pattern)) return(FALSE);
		}
	}
	return(TRUE);
}

// | & # - = + ~ ^
static char ced_MorItem(unCH *item, int *sErr, int *eErr, int offset) {
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
						*c != '=' && *c != '~' && *c != '$' && *c != EOS; c++) {
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

static void ced_CheckBrakets(unCH *org, long pos, int *sb, int *pb, int *ab, int *cb, int *anb) {
	if (uS.isRightChar(org, pos, '(', &tFnt, TRUE)) (*pb)++;
	else if (uS.isRightChar(org, pos, ')', &tFnt, TRUE)) (*pb)--;
	else if (uS.isRightChar(org, pos, '[', &tFnt, TRUE)) (*sb)++;
	else if (uS.isRightChar(org, pos, ']', &tFnt, TRUE)) (*sb)--;
	else if (uS.isRightChar(org, pos, '{', &tFnt, TRUE)) (*cb)++;
	else if (uS.isRightChar(org, pos, '}', &tFnt, TRUE)) (*cb)--;
	else if (uS.isRightChar(org, pos, '<', &tFnt, TRUE)) {
		if (!uS.isPlusMinusWord(org, pos))
			(*ab)++;
	} else if (uS.isRightChar(org, pos, '>', &tFnt, TRUE)) {
		if (!uS.isPlusMinusWord(org, pos)) {
			(*ab)--;
			(*anb) = TRUE;
		}
	}
}

static char ced_atUFound(unCH *w, int s) {
	while (w[s] != EOS) {
		if (uS.isRightChar(w, s, '<', &tFnt, TRUE) || uS.isRightChar(w, s, '>', &tFnt, TRUE) || 
			uS.isRightChar(w, s, '[', &tFnt, TRUE) || uS.isRightChar(w, s, ']', &tFnt, TRUE))
			break;
		if (w[s] == '@') {
			if (w[s+1] != EOS && w[s+1] == 'u' && (w[s+2] == EOS || !is_unCH_alnum(w[s+2]))) 
				return(TRUE);
		}
		s++;
	}
	return(FALSE);
}

static char isIllegalASCII(unCH *w, int s) {
	if (w[s] == 0x00b7)
		return(TRUE);
	else if (w[s] >= 0xe000) {
		if ((w[s] >= 0xf170 && w[s] <= 0xf264) || (w[s] >= 0xff01 && w[s] <= 0xff5e))
			return(FALSE);
		else
			return(TRUE);
	}
	return(FALSE);
}

static int isBadQuotes(unCH *w, int s) {
	if (w[s] == 0x2018)
		return(139);
	else if (w[s] == 0x2019)
		return(138);
	return(FALSE);
}

static char isLegalUseOfDigit(unCH *w, int s) {
	if (uS.isRightChar(w,s-1,'-',&tFnt,TRUE) ||
		 (w[s-1] == 0x2308 || w[s-1] == 0x2309 || w[s-1] == 0x230a || w[s-1] == 0x230b)
		)
		return(TRUE);
	else
		return(FALSE);
}
/* 2009-05-01
static char ced_isCAchar(unCH *w, int pos) {
	int s, matchType;

	if (global_dfC->isUTF) {
		s = uS.HandleCAChars(w+pos, &matchType);
		if (matchType == CA_APPLY_CLTOPSQ || matchType == CA_APPLY_CLBOTSQ) {
			if (w[pos+s] == EOS)
				return(TRUE);
		}
	}
	return(FALSE);
}
*/
static char ced_isOnlyCAs(unCH *w, int pos) {
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

static char ced_isEndOfWord(unCH *w) {
	int i;

	for (i=strlen(w)-1; i >= 0; i--) {
		if (iswalnum(w[i]))
			return(FALSE);
	}
	return(TRUE);
}

static char ced_isThereStem(unCH *w, int pos) {
	int s, matchType;
	char isF;

	isF = FALSE;
	for (s=0; w[s] != EOS; s++) {
		if (uS.HandleCAChars(w+s, &matchType)) {
			if (matchType == NOTCA_LEFT_ARROW_CIRCLE)
				isF = !isF;
		}
		if (!isF && w[s+1] != EOS)
			return(TRUE);
	}
	if (check_err(151, pos-1, pos+s-2, templine))
		return(FALSE);
	else
		return(TRUE);
}

static char ced_CheckWords(unCH *w, int pos, unCH *templine, CHECK_IN_TIERS *ts) {
	char capWErrFound, capWFound, digFound, isCompFound, paransFound;
	int s, t, len, matchType, CAOffset, isATFound;

	len = strlen(w) - 1;
	if (ts->code[0] == '*' || 
		ced_u.check_partcmp(ts->code, "%wor", FALSE) || 
		ced_u.check_partcmp(ts->code, "%xwor", FALSE)) {
		s = uS.HandleCAChars(w, &matchType);
		if (s) {
			if (matchType == NOTCA_MISSINGWORD) {
				if (check_err(48, pos-1, pos+s-2, templine))
					return(FALSE);
			} else if (matchType == CA_APPLY_CLTOPSQ || matchType == CA_APPLY_CLBOTSQ ||
				matchType == NOTCA_GROUP_END || matchType == NOTCA_SIGN_GROUP_END) {
				if (!ced_isOnlyCAs(w, s)) {
					if (check_err(92, pos-1, pos+s-2, templine))
						return(FALSE);
				}
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
					if (!ced_isThereStem(w, pos))
						return(FALSE);
				}
				if (w[s] == EOS || uS.isskip(w,s,&tFnt,TRUE)) {
					if (check_err(101, pos-1, pos+s-2, templine))
						return(FALSE);
				} else if (uS.HandleCAChars(w+s, &matchType)) {
					if (matchType == CA_APPLY_OPTOPSQ || matchType == CA_APPLY_OPBOTSQ) {
						if (check_err(65, pos-1, pos+s-2, templine))
							return(FALSE);
					}
				}
			} else if (matchType == NOTCA_DOUBLE_COMMA || matchType == NOTCA_VOCATIVE) {
				if (w[s] != EOS) {
					if (check_err(92, pos-1, pos+s-2, templine))
						return(FALSE);
				}
			} else if (matchType == CA_APPLY_OPTOPSQ || matchType == CA_APPLY_OPBOTSQ ||
					   matchType == NOTCA_GROUP_START || matchType == NOTCA_SIGN_GROUP_START ||
					   matchType == NOTCA_OPEN_QUOTE)
				;
			else
				s = 0;
		}
		CAOffset = s;
		if (w[s] == CED_HIDEN_C) ;
		else if (uS.isRightChar(w,s,'[',&tFnt,TRUE) && uS.isRightChar(w,len,']',&tFnt,TRUE)) ;
		else if (uS.isRightChar(w,s,'{',&tFnt,TRUE) && uS.isRightChar(w,len,'}',&tFnt,TRUE)) ;
		else if (uS.isRightChar(w,s,'-',&tFnt,TRUE) && w[s+1] != EOS && !is_unCH_alnum(w[s+1])) ;
		else if (uS.isRightChar(w,s,'+',&tFnt,TRUE)) ;
		else if (uS.isRightChar(w,s,'(',&tFnt,TRUE) && uS.isPause(w,s,NULL,&t)) {
			char isNumf, isColf, isDotf;

			isNumf = FALSE;
			isColf = FALSE;
			isDotf = 0;
			while (w[s] != EOS) {
				if (is_unCH_digit(w[s])) {
					if (isDotf > 1) {
						if (check_err(111, pos+s-1, pos+s-1, templine))
							return(FALSE);
					}
					isNumf =  TRUE;
				}
				if (w[s] == ':') {
					if (isColf || isDotf) {
						if (check_err(111, pos+s-1, pos+s-1, templine))
							return(FALSE);
					}
					isColf = TRUE;
				}
				if (w[s] == '.') {
					if ((isDotf >= 1 && (isColf || isNumf)) || isDotf >= 3) {
						if (check_err(111, pos+s-1, pos+s-1, templine))
							return(FALSE);
					}
					isDotf++;
				}
				if (isSpace(w[s])) {
					if (check_err(111, pos+s-1, pos+s-1, templine))
						return(FALSE);
				}
				s++;
			}
			if (!isDotf) {
				if (check_err(111, pos-1, pos+strlen(w)-2, templine))
					return(FALSE);
			}
		} else if (uS.isRightChar(w,s,'&',&tFnt,TRUE)) {
			s++;	
			if (w[s] == EOS) {
				if (check_err(65, pos-1, pos-1, templine))
					return(FALSE);
			}
			while (w[s] != EOS) {
				if (is_unCH_alnum(w[s]) || w[s] == '=' || w[s] == '-' || w[s] == '_' || w[s] == '^' ||
						w[s] == '$' || w[s] == '%' || w[s] == '{' || w[s] == '}') ;
				else if (uS.isRightChar(w,s,'@',&tFnt,TRUE) || uS.isRightChar(w,s,'\'',&tFnt,TRUE) ||
						 uS.isRightChar(w,s,':',&tFnt,TRUE) || uS.isRightChar(w,s,'&',&tFnt,TRUE)  ||
						 uS.isRightChar(w,s,'+',&tFnt,TRUE) || uS.isRightChar(w,s,'/',&tFnt,TRUE)  ||
						 uS.isRightChar(w,s,'~',&tFnt,TRUE) || uS.isRightChar(w,s,'*',&tFnt,TRUE)) {

					if (w[s] == ':' && w[s+1] == ':' && (w[s+2] == '@' || w[s+2] == ':')) {
						if (ced_atUFound(w, s))
							break;
						if (check_err(48, pos+s-1, pos+s-1, templine))
							return(FALSE);
					}
					if (w[s] == w[s+1] && w[s] != ':') {
						if (ced_atUFound(w, s))
							break;
						if (check_err(48, pos+s-1, pos+s-1, templine))
							return(FALSE);
					}
				} else if (w[s] > 0 && w[s] <= 127) {
					if (ced_atUFound(w, s))
						break;
					if (check_err(65, pos+s-2, pos+s-2, templine))
						return(FALSE);
				}
				s++;
			}
		} else if (uS.isRightChar(w,s,'#',&tFnt,TRUE))  {
			s++;
			if (uS.isRightChar(w,s,'#',&tFnt,TRUE))
				s++;
			if (uS.isRightChar(w,s,'#',&tFnt,TRUE))
				s++;
			if (w[s] == '_') {
				if (check_err(48, pos+s-1, pos+s-1, templine))
					return(FALSE);
			}
			if (w[s] == '0' && w[s+1] != '_') {
				if (check_err(48, pos+s-1, pos+s-1, templine))
					return(FALSE);
			}
			while (w[s] != EOS) {
				if (!is_unCH_digit(w[s]) && !uS.isRightChar(w,s,':',&tFnt,TRUE) && !uS.isRightChar(w,s,'_',&tFnt,TRUE)) {
					if (check_err(48, pos+s-1, pos+s-1, templine))
						return(FALSE);
				} else if (uS.isRightChar(w,s-1,'_',&tFnt,TRUE) && w[s] == '0' && is_unCH_digit(w[s+1])) {
					if (check_err(48, pos+s-1, pos+s-1, templine))
						return(FALSE);
				}
				s++;
			}
		} else if (uS.mStricmp(w,"xx") == 0 || uS.mStricmp(w,"yy") == 0) {
				if (check_err(135, pos-1, pos+len-1, templine))
					return(FALSE);
		} else {
			isATFound = 0;
			capWFound = FALSE;
			capWErrFound = FALSE;
			digFound = FALSE;
			isCompFound = FALSE;
			paransFound = FALSE;
			if (uS.isRightChar(w,s,'0',&tFnt,TRUE))
				s++;
			if (uS.isRightChar(w,s,'0',&tFnt,TRUE) || uS.isRightChar(w,s,'*',&tFnt,TRUE))
				s++;
			while (w[s] != EOS) {
				if (w[s] == 0x2026) {
					if (check_err(48, pos+s-1, pos+s-1, templine))
						return(FALSE);
				} else if (is_unCH_digit(w[s])) {
				   	if (isATFound) ;
					else if (!digFound && (s == 0 || !isLegalUseOfDigit(w, s))) {
						if (ced_atUFound(w, s)) {
							digFound = TRUE;
							if (check_err(47, pos+s-1, pos+s-1, templine))
								return(FALSE);
						} else if (!ced_isNumberShouldBeHere) {
							digFound = TRUE;
							if (check_err(47, pos+s-1, pos+s-1, templine))
								return(FALSE);
						} else if (ced_isNumberShouldBeHere) {
							if (s == 0) {
								digFound = TRUE;
								if (check_err(47, pos+s-1, pos+s-1, templine))
									return(FALSE);
							}
						}
					}
				} else if (!is_unCH_alpha(w[s])) {
					t = uS.HandleCAChars(w+s, &matchType);
					if (t) {
						if (w[s+t] == EOS && (matchType == CA_APPLY_OPTOPSQ || matchType == CA_APPLY_OPBOTSQ ||
											  matchType == NOTCA_GROUP_START || matchType == NOTCA_SIGN_GROUP_START)) {
							if (check_err(93, pos+s-1, pos+s+t-2, templine))
								return(FALSE);
						}
						if (w[s+t] != EOS && (matchType == CA_APPLY_CONTINUATION || matchType == CA_APPLY_LATCHING ||
											  matchType == NOTCA_DOUBLE_COMMA || matchType == NOTCA_VOCATIVE)) {
							if (check_err(92, pos+s-1, pos+s+t-2, templine))
								return(FALSE);
						}
						if (s != 0 && (matchType == NOTCA_DOUBLE_COMMA || matchType == NOTCA_VOCATIVE)) {
							if (check_err(93, pos+s-1, pos+s+t-2, templine))
								return(FALSE);
						}
						s += (t - 1);
						if (matchType == NOTCA_LEFT_ARROW_CIRCLE && iswupper(w[s+1]))
							s++;
					} else if ((t=isBadQuotes(w, s)) > 0) {
						if (check_err(t, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (isIllegalASCII(w, s)) {
						if (check_err(86, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (uS.isRightChar(w,s,'/',&tFnt,TRUE)) {
						if (uS.isRightChar(w,s+1,'/',&tFnt,TRUE) && uS.isRightChar(w,s+2,'/',&tFnt,TRUE) && 
							uS.isRightChar(w,s+3,'/',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
						if (ced_FoundAlNums(0, w)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(48, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
						if (w[s+1] == EOS) {
							if (check_err(48, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,':',&tFnt,TRUE)) {
						if (uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'"',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(48, pos+s, pos+s, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'&',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'\'',&tFnt,TRUE)|| uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'^',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'@',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'"',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						} else {
							if (check_err(66, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'(',&tFnt,TRUE)) {
						paransFound = TRUE;
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'(',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'#',&tFnt,TRUE) || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,')',&tFnt,TRUE)) {
						paransFound = FALSE;
						if (w[s+1] != EOS && uS.isRightChar(w,s+1,')',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'`',&tFnt,TRUE) && ced_isArabic) {
					} else if (uS.isRightChar(w,s,'#',&tFnt,TRUE)) {
						if (ced_isEndOfWord(w+s+1)) ;
						else {
							if (ced_atUFound(w, s))
								break;
							if (check_err(48, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'^',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || /* uS.isRightChar(w,s+1,'\'',&tFnt,TRUE) || */
								uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'^',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'@',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'"',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'_',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'@',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'"',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'~',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'@',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'"',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'$',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'@',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'"',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'+',&tFnt,TRUE)) {
						isCompFound = TRUE;
/*
						if (capWFound) {
							if (check_err(95, pos+s-1, pos+s-1, templine))
								return(FALSE);
						} else
*/
						if (uS.isRightChar(w,s+1,'"',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'<',&tFnt,TRUE) || uS.isRightChar(w,s+1,',',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'/',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'.',&tFnt,TRUE) || uS.isRightChar(w,s+1,'!',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'?',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (!ced_isNumberShouldBeHere || !uS.isRightChar(w,s+1,'+',&tFnt,TRUE)) {
								if (check_err(66, pos+s-1, pos+s-1, templine))
									return(FALSE);
							}
						} else 
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'@',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(67, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'-',&tFnt,TRUE)) {
						if (w[s+1] == EOS)
							;
						else if (uS.isRightChar(w,s+1,',',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE) ||
								   uS.isRightChar(w,s+1,'.',&tFnt,TRUE) || uS.isRightChar(w,s+1,'!',&tFnt,TRUE)  ||
								   uS.isRightChar(w,s+1,'?',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(66, pos+s-1, pos+s-1, templine))
								return(FALSE);
						} else if (uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
								   uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
								   uS.isRightChar(w,s+1,'@',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE) ||
								   uS.isRightChar(w,s+1,'"',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(67, pos+s-1, pos+s-1, templine))
								return(FALSE);
						} else if (isATFound == 0 && w[s+1] == '@') {
//							if (!uS.isRightChar(w,s+1,'0',&tFnt,TRUE)) {
//								if (ced_atUFound(w, s))
//									break;
								if (check_err(48, pos+s-1, pos+s-1, templine))
									return(FALSE);
//							}
						}
					} else if (uS.isRightChar(w,s,'\'',&tFnt,TRUE)) {
						if (w[s+1] != EOS && (uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'@',&tFnt,TRUE) || uS.isRightChar(w,s+1,'"',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'_',&tFnt,TRUE))) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'@',&tFnt,TRUE)) {
						isATFound = s;
						if (paransFound) {
							if (check_err(97, pos+s-1, pos+s-1, templine))
								return(FALSE);
						} else if (w[s+1] == 's' && w[s+2] == ':') {
							for (t=s+3; iswalpha(w[t]) || uS.HandleCAChars(w+t, NULL) || w[t] == '&' || w[t] == '+' || w[t] == '$' || w[t] == ':'; t++) ;
							if (w[t] != EOS) {
								if (check_err(65, pos+s-1, pos+t-2, templine))
									return(FALSE);
							}
							s = t - 1;
						} else if (w[s+1] == 's') {
							for (t=s+2; iswalpha(w[t]) || uS.HandleCAChars(w+t, NULL) || w[t] == '$' || w[t] == ':'; t++) ;
							if (w[t] != EOS) {
								if (check_err(65, pos+s-1, pos+t-2, templine))
									return(FALSE);
							}
							s = t - 1;
/* 2009-06-08
						} else if (isCompFound) {
							if (ced_isSign)
								;
							else if (w[s+1] == 's' && (w[s+2] == ':' || w[s+2] == EOS))
								;
							else if ((w[s+1] == 'c' || w[s+2] == 'n') && w[s+2] == EOS)
								;
							else if (check_err(88, pos-1, pos+len-1, templine))
								return(FALSE);
*/
						} else if (w[s+1] == EOS || uS.isRightChar(w,s+1,'@',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'(',&tFnt,TRUE) || uS.isRightChar(w,s+1,')',&tFnt,TRUE)) {
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						} else if (uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'"',&tFnt,TRUE) || uS.isRightChar(w,s+1,'_',&tFnt,TRUE)) {
							if (ced_atUFound(w, s))
								break;
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (w[s]>0 && w[s]<=127) {
						if (ced_atUFound(w, s))
							break;
						if (check_err(48, pos+s-1, pos+s-1, templine))
							return(FALSE);
					}
				}
/* 2019-04-23 do not check for upper case letters
				else if (is_unCH_upper(w[s]) && w[s]>0 && w[s]<=127 && !ced_isCAFound && !capWErrFound &&
						   (s == 0 || (w[s-1] != '-' && w[s-1] != '\'' && w[s-1] != '_' && w[s-1] != '0' && w[s-1] != '(' && s != CAOffset))) {
					if (s == 0 && ced_isGerman)
						;
					else
						capWFound = TRUE;
					if (w[s-1] == '+' && ced_isGerman)
						;
					else if (s > 0 && !ced_isRightPref(ts->prefs, w, s) && !ced_isException(w, ced_WordExceptions)) {
						if (ced_atUFound(w, s))
							break;
						capWErrFound = TRUE;
						if (check_err(49, pos+s-1, pos+s-1, templine))
							return(FALSE);
					}
				}
*/
				s++;
			}
		}
	} else if (ced_u.check_partcmp(ts->code, "@Languages:", FALSE)) {
		s = 0;
		while (w[s] != EOS) {
			if (uS.isRightChar(w,s,'=',&tFnt,TRUE) ) {
/* 18-07-2008
				if (!strcmp(w+s+1, "blue") || !strcmp(w+s+1, "red") || !strcmp(w+s+1, "green") || !strcmp(w+s+1, "magenta"))
					break;
				else
*/
				if (check_err(96, pos+s, pos+strlen(w)-2, templine))
					return(FALSE);
			}
			s++;
		}
	} else if (ced_u.check_partcmp(ts->code, "%mor", FALSE) || ced_u.check_partcmp(ts->code, "%xmor", FALSE) ||
			   ced_u.check_partcmp(ts->code, "%trn", FALSE)) {
		if (w[0] == 0x201E && w[1] == EOS) {
		} else if (uS.isRightChar(w,0,'[',&tFnt,TRUE) && uS.isRightChar(w,len,']',&tFnt,TRUE)) {
			if (isPostCodeMark(w[1], w[2])) {
				if (check_err(109, pos-1, pos+len-1, templine))
					return(FALSE);
			}
		} else if (uS.isRightChar(w,0,'+',&tFnt,TRUE)) ;
		else if (uS.isRightChar(w,0,'-',&tFnt,TRUE) && w[1] != EOS && !is_unCH_alnum(w[1])) ;
		else if (ced_u.check_patmat(w,"unk|xxx") || ced_u.check_patmat(w,"*|xx") ||
				 ced_u.check_patmat(w,"unk|yyy") || ced_u.check_patmat(w,"*|yy") ||
				 ced_u.check_patmat(w,"*|www")   /* || ced_u.check_patmat(w,"cm|cm")*/) {
			if (check_err(134, pos-1, pos+len-1, templine))
				return(FALSE);
		} else {
			int  e = 0, numOfCompounds;
			char isFoundBar, res;

			if ((res=ced_MorItem(w, &s, &e, 0))) {
				if (check_err(65, pos+s-1, pos+s-1, templine))
					return(FALSE);
			}
			if (w[0] == '^' && uS.partcmp(ced_sp, "%mor", FALSE, FALSE)) {
				if (check_err(48, pos-1, pos-1, templine))
					return(FALSE);
			}
			isFoundBar = 0;
			s = 0;
			numOfCompounds = -1;
			while (w[s] != EOS) {
				if (is_unCH_alnum(w[s])) {
				} else {
					if ((t=isBadQuotes(w, s)) > 0) {
						if (check_err(t, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (isIllegalASCII(w, s)) {
						if (check_err(86, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (w[s] == CED_HIDEN_C) {
						if (check_err(65, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (uS.isRightChar(w,s,'-',&tFnt,TRUE) || uS.isRightChar(w,s,'&',&tFnt,TRUE) ||
							   uS.isRightChar(w,s,'*',&tFnt,TRUE) || uS.isRightChar(w,s,'#',&tFnt,TRUE) || 
							   uS.isRightChar(w,s,'\'',&tFnt,TRUE)|| uS.isRightChar(w,s,'/',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'=',&tFnt,TRUE) || uS.isRightChar(w,s+1,'|',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'#',&tFnt,TRUE) || uS.isRightChar(w,s+1,':',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'^',&tFnt,TRUE) || uS.isRightChar(w,s+1,'~',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'*',&tFnt,TRUE) || uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'\'',&tFnt,TRUE)) {
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'+',&tFnt,TRUE) || uS.isRightChar(w,s,'~',&tFnt,TRUE) || uS.isRightChar(w,s,'$',&tFnt,TRUE)) {
						if (isFoundBar == 1) {
							isFoundBar = 0;
							e = s;
						} else {
							if (check_err(80, pos-1, pos+s-2, templine))
								return(FALSE);
						}
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'=',&tFnt,TRUE) || uS.isRightChar(w,s+1,'|',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'#',&tFnt,TRUE) || uS.isRightChar(w,s+1,':',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'^',&tFnt,TRUE) || uS.isRightChar(w,s+1,'~',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'*',&tFnt,TRUE) || uS.isRightChar(w,s+1,'+',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'\'',&tFnt,TRUE)) {
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
						if (uS.isRightChar(w,s,'+',&tFnt,TRUE)) {
							if (uS.isRightChar(w,s-1,'|',&tFnt,TRUE)) {
								if (numOfCompounds >= 0) {
									numOfCompounds = -2;
								} else
									numOfCompounds = 0;
							} else if (numOfCompounds == -1)
								numOfCompounds = -2;
							else if (numOfCompounds >= 0)
								numOfCompounds++;
						}
					} else if (uS.isRightChar(w,s,':',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'=',&tFnt,TRUE) || uS.isRightChar(w,s+1,'|',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'#',&tFnt,TRUE) || uS.isRightChar(w,s+1,':',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'^',&tFnt,TRUE) || uS.isRightChar(w,s+1,'~',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE)) {
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'|',&tFnt,TRUE)) {
						if (isFoundBar) {
							if (check_err(79, pos-1, pos+s-1, templine))
								return(FALSE);
						} else
							if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'=',&tFnt,TRUE) || uS.isRightChar(w,s+1,'|',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'#',&tFnt,TRUE) || uS.isRightChar(w,s+1,':',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'^',&tFnt,TRUE) || uS.isRightChar(w,s+1,'~',&tFnt,TRUE) ||
								uS.isRightChar(w,s+1,'*',&tFnt,TRUE)) {
								if (check_err(65, pos+s-1, pos+s-1, templine))
									return(FALSE);
							}
						isFoundBar++;
					} else if (uS.isRightChar(w,s,'^',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'=',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,':',&tFnt,TRUE) || uS.isRightChar(w,s+1,'^',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'~',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE)) {
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'?',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'=',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,':',&tFnt,TRUE) || uS.isRightChar(w,s+1,'^',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'*',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE)) {
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'=',&tFnt,TRUE)) {
						for (t=s+1; iswalpha(w[t]) || w[t] == '/' || w[t] == '_' || w[t] == 0x2013; t++) ;
						if (w[t] != EOS && w[t] != '~' && w[t] != '+' && w[t] != '$' && w[t] != '%' && w[t] != '^') {
							if (check_err(65, pos+s-1, pos+t-2, templine))
								return(FALSE);
						}
						s = t - 1;
					} else if (uS.isRightChar(w,s,'_',&tFnt,TRUE)) {
					} else if (uS.isRightChar(w,s,'`',&tFnt,TRUE) && ced_isArabic) {
					} else if (w[s]>0 && w[s]<=127) {
						if (check_err(48, pos+s-1, pos+s-1, templine))
							return(FALSE);
					}
				}
				s++;
			}
			if (isFoundBar < 1) {
				if (check_err(80, pos+e-1, pos+len-1, templine))
					return(FALSE);
			}
			if (numOfCompounds == -2 || (numOfCompounds >= 0 && numOfCompounds < 1)) {
				if (check_err(87, pos-1, pos+len-1, templine))
					return(FALSE);
			}
		}
	} else if (ced_u.check_partcmp(ts->code, "%cnl", FALSE) || ced_u.check_partcmp(ts->code, "%xcnl", FALSE)) {
		if (w[0] == 0x201E && w[1] == EOS) {
		} else if (uS.isRightChar(w,0,'[',&tFnt,TRUE) && uS.isRightChar(w,len,']',&tFnt,TRUE)) {
			if (isPostCodeMark(w[1], w[2])) {
				if (check_err(109, pos-1, pos+len-1, templine))
					return(FALSE);
			}
		} else if (uS.isRightChar(w,0,'+',&tFnt,TRUE)) ;
		else if (uS.isRightChar(w,0,'-',&tFnt,TRUE) && w[1] != EOS && !is_unCH_alnum(w[1])) ;
		else if (ced_u.check_patmat(w,"unk|xxx") || ced_u.check_patmat(w,"*|xx") ||
				 ced_u.check_patmat(w,"unk|yyy") || ced_u.check_patmat(w,"*|yy") ||
				 ced_u.check_patmat(w,"*|www")   /* || ced_u.check_patmat(w,"cm|cm")*/) {
			if (check_err(134, pos-1, pos+len-1, templine))
				return(FALSE);
		} else {
			int numOfCompounds;

			s = 0;
			numOfCompounds = -1;
			while (w[s] != EOS) {
				if (is_unCH_alnum(w[s])) {
					if ((s == 0 || isSpace(w[s-1])) && w[s] == 'g' && w[s+1] == 'e' && w[s+2] == 'n' &&
						(uS.isskip(w,s+3,&tFnt,TRUE) || w[s+3] == EOS)) {
						if (check_err(135, pos+s-1, pos+s+1, templine))
							return(FALSE);
					}
				} else {
					if ((t=isBadQuotes(w, s)) > 0) {
						if (check_err(t, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (isIllegalASCII(w, s)) {
						if (check_err(86, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (w[s] == CED_HIDEN_C) {
						if (check_err(65, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (uS.isRightChar(w,s,':',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'=',&tFnt,TRUE) || uS.isRightChar(w,s+1,'|',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'#',&tFnt,TRUE) || uS.isRightChar(w,s+1,':',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'^',&tFnt,TRUE) || uS.isRightChar(w,s+1,'~',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE)) {
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					} else if (uS.isRightChar(w,s,'^',&tFnt,TRUE)) {
						if (check_err(135, pos+s-1, pos+s-1, templine))
							return(FALSE);
					} else if (uS.isRightChar(w,s,'?',&tFnt,TRUE)) {
						if (w[s+1] == EOS || uS.isRightChar(w,s+1,'-',&tFnt,TRUE) || uS.isRightChar(w,s+1,'&',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'=',&tFnt,TRUE) || uS.isRightChar(w,s+1,'#',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,':',&tFnt,TRUE) || uS.isRightChar(w,s+1,'^',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'~',&tFnt,TRUE) || uS.isRightChar(w,s+1,'*',&tFnt,TRUE) ||
							uS.isRightChar(w,s+1,'+',&tFnt,TRUE) || uS.isRightChar(w,s+1,'\'',&tFnt,TRUE)) {
							if (check_err(65, pos+s-1, pos+s-1, templine))
								return(FALSE);
						}
					}
				}
				s++;
			}
			if (numOfCompounds == -2 || (numOfCompounds >= 0 && numOfCompounds < 1)) {
				if (check_err(87, pos-1, pos+len-1, templine))
					return(FALSE);
			}
		}
	} else if (ced_u.check_partcmp(ts->code, "%cod", FALSE)) {
		if (uS.isRightChar(w,0,'[',&tFnt,TRUE) && uS.isRightChar(w,len,']',&tFnt,TRUE)) {
			if (isPostCodeMark(w[1], w[2])) {
				if (check_err(109, pos-1, pos+len-1, templine))
					return(FALSE);
			}
		} else {
			s = 0;
			while (w[s] != EOS) {
				if ((t=isBadQuotes(w, s)) > 0) {
					if (check_err(t, pos+s-1, pos+s-1, templine))
						return(FALSE);
				} else if (isIllegalASCII(w, s)) {
					if (check_err(86, pos+s-1, pos+s-1, templine))
						return(FALSE);
				} else if (w[s] == CED_HIDEN_C) {
					if (check_err(65, pos+s-1, pos+s-1, templine))
						return(FALSE);
				}
				s++;
			}
		}
	} else if (ced_u.check_partcmp(ts->code, "%ges", FALSE)) {
		if (uS.isRightChar(w,0,'[',&tFnt,TRUE) && uS.isRightChar(w,len,']',&tFnt,TRUE)) {
			if (isPostCodeMark(w[1], w[2])) {
				if (check_err(109, pos-1, pos+len-1, templine))
					return(FALSE);
			}
		}
	} else if (ced_u.check_partcmp(ts->code, "%pho", FALSE)) {
		if (uS.isRightChar(w,0,'[',&tFnt,TRUE) && uS.isRightChar(w,len,']',&tFnt,TRUE)) {
			if (isPostCodeMark(w[1], w[2])) {
				if (check_err(109, pos-1, pos+len-1, templine))
					return(FALSE);
			}
		} else {
			s = 0;
			while (w[s] != EOS) {
				if (w[s] == CED_HIDEN_C) {
					if (check_err(65, pos+s-1, pos+s-1, templine))
						return(FALSE);
				}
				s++;
			}
		}
	} else if (ts->code[0] == '%') {
		if (uS.isRightChar(w,0,'[',&tFnt,TRUE) && uS.isRightChar(w,len,']',&tFnt,TRUE)) {
			if (isPostCodeMark(w[1], w[2])) {
				if (check_err(109, pos-1, pos+len-1, templine))
					return(FALSE);
			}
		} else {
			s = 0;
			while (w[s] != EOS) {
				if ((t=isBadQuotes(w, s)) > 0) {
					if (check_err(t, pos+s-1, pos+s-1, templine))
						return(FALSE);
				} else if (isIllegalASCII(w, s)) {
					if (check_err(86, pos+s-1, pos+s-1, templine))
						return(FALSE);
				} else if (w[s] == CED_HIDEN_C) {
					if (check_err(65, pos+s-1, pos+s-1, templine))
						return(FALSE);
				} else if (w[s] == '$' && s > 0) {
					if (check_err(48, pos+s-1, pos+s-1, templine))
						return(FALSE);
				}
				s++;
			}
		}
	}
	return(TRUE);
}

static char ced_isLangMatch(unCH *langs, int s, int wh, char isOneTime) {
	int i, j, langCnt;
	unCH t;

	i = 0;
	langCnt = 0;
	while (langs[i]) {
		for (; uS.isskip(langs,i,&tFnt,TRUE) || langs[i] == '\n'; i++) ;
		if (langs[i] == EOS)
			break;
		for (j=i; !uS.isskip(langs,j,&tFnt,TRUE) && langs[j] != '=' && langs[j] != '\n' && langs[j] != EOS; j++) ;
		t = langs[j];
		langs[j] = EOS;
		for (langCnt=0; ced_LanguagesTable[langCnt] != EOS && langCnt < NUMLANGUAGESINTABLE; langCnt++) {
			if (strcmp(ced_LanguagesTable[langCnt], langs+i) == 0)
				break;
		}
		langs[j] = t;
		if (langCnt >= NUMLANGUAGESINTABLE) {
			if (check_err(wh,s+i,s+j-1,templine))
				return(FALSE);
		}
		if (isOneTime)
			break;
		if (langs[j] == '=')
			for (; !uS.isskip(langs,j,&tFnt,TRUE) && langs[j] != '\n' && langs[j] != EOS; j++) ;
		i = j;
	}
	return(TRUE);
}

static char ced_isOneLetter(unCH *word, unCH *bw) {
	int num = 0;

	for (; word != bw && *word != EOS && *word != '@' && *word != '-'; word++) {
		if (*word == 0x21AB) {
			while (*word != EOS) {
				word++;
				if (*word == 0x21AB)
					break;
			}
			if (*word == 0x21AB)
				word++;
			if (*word == EOS)
				break;
		}
		if (*word != ':')
			num++;
	}
	if (num > 1)
		return(FALSE);
	else
		return(TRUE);
}

static char ced_CheckFixes(CHECK_IN_TIERS *ts, unCH *word, unCH *templine, int s, int e, char *found) {
	int s1, pos;
	unCH *bw, t;
	char isPrefix, isCompFound;
	TPLATES *tp;

	isCompFound = FALSE;
	*found = FALSE;
	s1 = s;
	bw = word;
	for (pos=0; word[pos]; pos++, s++) {
		if (uS.isRightChar(word, pos, '+', &tFnt, TRUE))
			isCompFound = TRUE;

		if (uS.ismorfchar(word, pos, &tFnt, CHECK_MORPHS, TRUE) || uS.isRightChar(word, pos, '@', &tFnt, TRUE)) {
			if (!isItemPrefix[word[pos]]) {
				isPrefix = FALSE;
				bw = word + pos;
				s1 = s;
				for (pos++, s++; word[pos] && !uS.isRightChar(word, pos, '@', &tFnt, TRUE) &&
											!uS.ismorfchar(word,pos,&tFnt,CHECK_MORPHS,TRUE) &&
											!uS.isRightChar(word, pos, '$', &tFnt, TRUE); pos++, s++) ;
				pos--;
				s--;
			} else
				isPrefix = TRUE;
			t = word[pos+1];
			word[pos+1] = EOS;
			if (!strcmp(bw, "@l") && !ced_isOneLetter(word, bw)) {
				if (check_err(76,s1-2,s1-2,templine)) return(FALSE);
			}
			if (bw[0] == '@' && bw[1] == 's' && bw[2] == ':') {
				if (bw[3] == EOS) {
					if (check_err(62,s1-1,s-1,templine))
						return(FALSE);
				}
			}
			for (tp=ts->tplate; tp != NULL; tp = tp->nextone) {
				if (isCompFound) {
					if (*tp->pattern == '+' && *(tp->pattern+1) != EOS) {
						if (ced_u.check_patmat(bw,tp->pattern+1))
							break;
					}
				} else {
					if (*tp->pattern == '*' && *(tp->pattern+1) != EOS) {
						if (ced_u.check_patmat(bw,tp->pattern+1))
							break;
					}
				}
			}
			word[pos+1] = t;
			if (tp == NULL) {
				if (isPrefix) {
					if (check_err(37,s1-1,s-1,templine))
						return(FALSE);
				} else if (templine[s1-1] == '@') {
					if (check_err(147,s1-1,s-1,templine))
						return(FALSE);
				} else {
					if (check_err(20,s1-1,s-1,templine))
						return(FALSE);
				}
			} else
				*found = TRUE;
			bw = word + pos + 1;
			s1 = s + 1;
		}
	}
	return(TRUE);
}

static char ced_ID(CHECK_IN_TIERS *ts) {
	int r, s = 0, e = 0, cnt;
	unCH *st, spkr[BUFSIZ+1];
	CHECK_IN_TIERS *tts;

	cnt = 0;
	ced_line[0] = EOS;
	if (ts->IGN)
		return(TRUE);
	while (templine[s] != EOS) {
		if (!uS.isskip(templine,s,&tFnt,TRUE))
			break;
		s++;
	}
	if (templine[s] == EOS)
		return(TRUE);
	if (templine[s] == '|') {
		if (check_err(62,s,s,templine))
			return(FALSE);
	}
	while (1) {
		st = ced_line;
		if ((*st=templine[s]) == '|') {
			e = s + 1;
		} else {
			while ((*st=templine[s]) == '|' || uS.isskip(templine,s,&tFnt,TRUE)) {
				if (templine[s] == EOS)
					break;
				if (templine[s] == '|')
					cnt++;
				s++;
			}
			if (*st == EOS)
				break;
			e = s + 1;
			while ((*++st=templine[e]) != EOS) {
				e++;
				if (templine[e-1] == '|')
					break;
			}
		}
		*st = EOS;
//		uS.uppercasestr(ced_line, &tFnt, TRUE);
		if (is_unCH_alnum(*(ced_line+1)) || (*(ced_line+1) < 1 && *(ced_line+1) != EOS) || *(ced_line+1) > 127)
			ced_HandleParans(ced_line,NULL);
		if (cnt == 0) {
			if (!ced_isLangMatch(ced_line, s, 122, FALSE))
				return(FALSE);
			if (ced_line[0] == EOS) {
				if (check_err(62,s,e-2,templine))
					return(FALSE);
			}
		} else if (cnt == 1) {
			if (ced_line[0] == EOS) {
				if (check_err(63,s,e-2,templine))
					return(FALSE);
			}
		} else if (cnt == 2) {
			if (ced_line[0] == EOS) {
				if (check_err(12,s,e-2,templine))
					return(FALSE);
			} else if (ced_matchSpeaker(ced_headsp, ced_line, TRUE)) {
				if (check_err(18,s,e-2,templine))
					return(FALSE);
			} else if (ced_matchIDsp(ced_idsp, ced_line)) {
				if (check_err(13,s,e-2,templine))
					return(FALSE);
			}
			if (!ced_addIDdsp(ced_line))
				return(FALSE);
			strncpy(spkr, ced_line, BUFSIZ);
			spkr[BUFSIZ] = EOS;
		} else if (cnt == 3) {
			if ((tts=ced_FindTier(ced_headt,cl_T(IDOF))) != NULL) {
				if (!ced_Age(tts, NULL, templine, s, '|'))
					return(FALSE);
			}
		} else if (cnt == 4) {
			if (ced_line[0] != EOS) {
				if (strcmp(ced_line, "male") && strcmp(ced_line, "female")) {
					if (check_err(64,s,e-2,templine)) return(FALSE);
				}
			}
		} else if (cnt == 5) {
		} else if (cnt == 6) {
			if ((tts=ced_FindTier(ced_headt,cl_T(IDOF))) != NULL) {
				if (!ced_ses(tts, NULL, templine, s, '|'))
					return(FALSE);
			}
		} else if (cnt == 7) {
			if (ced_line[0] == EOS) {
				if (check_err(12,s,e-2,templine))
					return(FALSE);
			} else {
				if ((tts=ced_FindTier(ced_headt,cl_T(PARTICIPANTS))) != NULL) {
					if (ced_matchplate(tts->tplate, ced_line, 'D')) {
						if (check_err(15,s,e-2,templine)) return(FALSE);
					}
				}
				if (ced_matchRole(spkr, ced_headsp, ced_line)) {
					if (check_err(142,s,e-2,templine))
						return(FALSE);
				}
			}
		} else if (cnt == 8) {
		} else if (cnt == 9) {
		} else if ((r=ced_matchplate(ts->tplate, ced_line, 'D'))) {
			if (check_err(r,s-1,e-2,templine)) return(FALSE);
		}
		s = e;
		cnt++;
	}
	if (cnt != 10) {
		if (check_err(143,1,e,templine))
			return(FALSE);
	}
	return(TRUE);
}

static char ced_isMorTier(void) {
    if (ced_sp[0] == '%' && ced_sp[1] == 'm' && ced_sp[2] == 'o' && ced_sp[3] == 'r')
		return(TRUE);
    else
		return(FALSE);
}

static char ced_getMediaTagInfo(unCH *line, long *Beg, long *End) {
	int i;
	long beg = 0L, end = 0L;

	if (*line == CED_HIDEN_C)
		line++;
	if (*line == EOS)
		return(FALSE);
	if (*line == '%' && (*(line+1) == 's' || *(line+1) == 'm'))
		return(6);
	if (*line == '%' && (*(line+1) == 'p' || *(line+1) == 't' || *(line+1) == 'n'))
		return(0);
	if (*line == '0' && is_unCH_digit(*(line+1)))
		return(3);
	for (i=0; *line && is_unCH_digit(*line); line++)
		templine1[i++] = *line;
	templine1[i] = EOS;
	beg = uS.atol(templine1);
	if (beg == 0)
		beg = 1;
	if (*line != '_')
		return(2);
	line++;
	if (*line == EOS)
		return(FALSE);
	if (*line == '0' && is_unCH_digit(*(line+1)))
		return(3);
	for (i=0; *line && is_unCH_digit(*line); line++)
		templine1[i++] = *line;
	templine1[i] = EOS;
	end = uS.atol(templine1);
	*Beg = beg;
	*End = end;
	if (*line == '-')
		line++;
	if (*line != CED_HIDEN_C)
		return(2);
	return(1);
}

static char ced_getOLDMediaTagInfo(unCH *line, unCH *fname, long *Beg, long *End) {
	int i;
	long beg = 0L, end = 0L;

	if (*line == CED_HIDEN_C)
		line++;
	if (*line == EOS)
		return(FALSE);
	if (is_unCH_digit(*line))
		return(6);
	for (i=0; *line && *line != ':'; line++)
		templine1[i++] = *line;
	if (*line == EOS)
		return(FALSE);
	templine1[i++] = *line;
	templine1[i] = EOS;
	line++;
	if (!uS.partcmp(templine1, SOUNDTIER, FALSE, FALSE) && !uS.partcmp(templine1, REMOVEMOVIETAG, FALSE, FALSE))
		return(FALSE);

	if (*line != '"')
		return(2);
	line++;
	if (*line == EOS)
		return(FALSE);
	for (i=0; *line && *line != '"' && *line != CED_HIDEN_C; line++) {
		templine1[i++] = *line;
		if (isSpace(*line))
			return(4);
	}
	templine1[i] = EOS;
	if (strrchr(templine1, '.'))
		return(5);
	if (*line != '"')
		return(2);
	strcpy(fname, templine1);
	line++;
	if (*line == EOS)
		return(FALSE);
	if (*line != '_')
		return(2);
	line++;
	if (*line == EOS)
		return(FALSE);
	if (*line == '0' && is_unCH_digit(*(line+1)))
		return(3);
	for (i=0; *line && is_unCH_digit(*line); line++)
		templine1[i++] = *line;
	templine1[i] = EOS;
	beg = uS.atol(templine1);
	if (beg == 0)
		beg = 1;
	if (*line != '_')
		return(2);
	line++;
	if (*line == EOS)
		return(FALSE);
	if (*line == '0' && is_unCH_digit(*(line+1)))
		return(3);
	for (i=0; *line && is_unCH_digit(*line); line++)
		templine1[i++] = *line;
	templine1[i] = EOS;
	end = uS.atol(templine1);
	*Beg = beg;
	*End = end;
	if (*line == '-')
		line++;
	if (*line != CED_HIDEN_C)
		return(2);
	return(1);
}

static int ced_checkBulletsConsist(char *FirstBulletFound, unCH *word, BOOL isMedia) {
	char res;
	unCH tFname[FILENAME_MAX];
	long tBegTime;
	long tEndTime;

	if (!ced_isCheckBullets)
		return(0);
	if (isMedia) {
		tBegTime = ced_SNDBeg;
		tEndTime = ced_SNDEnd;
		res = ced_getMediaTagInfo(word, &ced_SNDBeg, &ced_SNDEnd);
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
			if (ced_isUnlinkedFound)
				return(124); // remove "unlinked" from @Media header
			if (ced_SNDEnd <= ced_SNDBeg) {
				ced_SNDBeg = tBegTime;
				ced_SNDEnd = tEndTime;
				return(82); // BEG mark must be smaller than END mark
			}
			if (tEndTime != 0L) {
				if (*FirstBulletFound == FALSE) {
					*FirstBulletFound = TRUE;
					if (ced_SNDBeg < ced_tierBegTime) {
						ced_SNDBeg = ced_tierBegTime;
						ced_SNDEnd = tEndTime;
						ced_tierBegTime = ced_SNDBeg;
						return(83); // Current BEG time is smaller than previous' tier BEG time
					} else
						ced_tierBegTime = ced_SNDBeg;
				} else {
					if (ced_SNDBeg < tBegTime) {
						ced_SNDBeg = tBegTime;
						ced_SNDEnd = tEndTime;
						return(83); // Current BEG time is smaller than previous' tier BEG time
					}
				}
				tDiff = ced_getLatTime(ced_sp) - ced_SNDBeg;
				if (tDiff > 500L) {
					ced_SNDBeg = tBegTime;
					ced_SNDEnd = tEndTime;
					ced_setLastTime(ced_sp, ced_SNDEnd);
					return(133); // Current BEG time is smaller than same speaker's previous END time
				}
				if (checkBullets == 1) {
					if (ced_SNDBeg < tEndTime) {
						ced_SNDBeg = tBegTime;
						ced_SNDEnd = tEndTime;
						tDiff = tEndTime - ced_SNDBeg;
						ced_setLastTime(ced_sp, ced_SNDEnd);
						return(84); // Current BEG time is smaller than previous' tier END time
					}
					if (ced_SNDBeg > tEndTime) {
						ced_SNDBeg = tBegTime;
						ced_SNDEnd = tEndTime;
						ced_setLastTime(ced_sp, ced_SNDEnd);
						return(85); // Gap found between current BEG time and previous' tier END time
					}
				}
			}
			ced_setLastTime(ced_sp, ced_SNDEnd);
		}
		return(0);
	}

	tBegTime = ced_SNDBeg;
	tEndTime = ced_SNDEnd;
	strcpy(tFname, ced_SNDFname);
	res = ced_getOLDMediaTagInfo(word, ced_SNDFname, &ced_SNDBeg, &ced_SNDEnd);
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
		if (ced_isUnlinkedFound)
			return(124); // remove "unlinked" from @Media header
		if (uS.mStricmp(tFname, ced_SNDFname) == 0 && tEndTime != 0L) {
			if (ced_SNDEnd <= ced_SNDBeg) {
				ced_SNDBeg = tBegTime;
				ced_SNDEnd = tEndTime;
				return(82); // BEG mark must be smaller than END mark
			}
			if (*FirstBulletFound == FALSE) {
				*FirstBulletFound = TRUE;
				if (ced_SNDBeg < ced_tierBegTime) {
					ced_SNDBeg = ced_tierBegTime;
					ced_SNDEnd = tEndTime;
					ced_tierBegTime = ced_SNDBeg;
					return(83); // Current BEG time is smaller than previous' tier BEG time
				} else
					ced_tierBegTime = ced_SNDBeg;
			} else {
				if (ced_SNDBeg < tBegTime) {
					ced_SNDBeg = tBegTime;
					ced_SNDEnd = tEndTime;
					return(83); // Current BEG time is smaller than previous' tier BEG time
				}
			}
			tDiff = ced_getLatTime(ced_sp) - ced_SNDBeg;
			if (tDiff > 500L) {
				ced_SNDBeg = tBegTime;
				ced_SNDEnd = tEndTime;
				ced_setLastTime(ced_sp, ced_SNDEnd);
				return(133); // Current BEG time is smaller than same speaker's previous END time
			}
			if (checkBullets == 1) {
				if (ced_SNDBeg < tEndTime) {
					ced_SNDBeg = tBegTime;
					ced_SNDEnd = tEndTime;
					tDiff = tEndTime - ced_SNDBeg;
					ced_setLastTime(ced_sp, ced_SNDEnd);
					return(84); // Current BEG time is smaller than previous' tier END time
				}
				if (ced_SNDBeg > tEndTime) {
					ced_SNDBeg = tBegTime;
					ced_SNDEnd = tEndTime;
					ced_setLastTime(ced_sp, ced_SNDEnd);
					return(85); // Gap found between current BEG time and previous' tier END time
				}
			}
		}
		ced_setLastTime(ced_sp, ced_SNDEnd);
	}
	return(0);
}

static void ced_removeCAChars(unCH *word) {
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

static char ced_lastBulletOnTier(unCH *st) {
	for (; *st != EOS; st++) {
		if (*st != '\n' && !isSpace(*st))
			return(FALSE);
	}
	return(TRUE);
}

static char ced_ParseBlob(CHECK_IN_TIERS *ts, BOOL isMedia) {
	int s = 0, e = 0, res;
	unCH *st;
	char hidenc;
	char isTextBetweenBulletsFound = FALSE, FirstBulletFound = FALSE, isLastItemBullet = FALSE;

	ced_line[0] = EOS;
	if (ts != NULL && ts->IGN)
		return(TRUE);
	while (1) {
		st = ced_line;
		while ((*st=templine[s++]) != EOS && uS.isskip(templine,s-1,&tFnt,TRUE)) ;

		if (*st == EOS)
			break;

		if (templine[s-1] == CED_HIDEN_C)
			hidenc = TRUE;
		else
			hidenc = FALSE;
		e = s;
		while ((*++st=templine[e]) != EOS && (!uS.isskip(templine,e,&tFnt,TRUE) || hidenc)) {
			e++;
			if (hidenc && templine[e-1] == CED_HIDEN_C) { hidenc = FALSE; st++; break; }
			if (!hidenc && templine[e] == CED_HIDEN_C) { st++; break; }
		}
		*st = EOS;
		if (ced_line[0] == CED_HIDEN_C) {
			if (*ced_sp == '*') {
				if (!isTextBetweenBulletsFound && (!ced_isMultiBullets || !isLastItemBullet || !ced_lastBulletOnTier(templine+e))) {
					if (check_err(73,s-1,e-1,templine)) return(FALSE);
				}
			}
			if (hidenc) {
				if (check_err(59,s-1,s-1,templine)) return(FALSE);
			}
			if (ced_sp[0] == '*' /* 2018-08-06 || ced_isBlob == FALSE */) {
				if ((res=ced_checkBulletsConsist(&FirstBulletFound, ced_line, isMedia)) != 0) {
					if (check_err(res,s-1,e-1,templine))
						return(FALSE);
				}
			}
			isTextBetweenBulletsFound = FALSE;
			isLastItemBullet = TRUE;
		} else {
			isTextBetweenBulletsFound = TRUE;
			isLastItemBullet = FALSE;
		}
		s = e;
	}
	return(TRUE);
}

static char ced_isBlankLine(unCH *line, int s) {
	while (isSpace(line[s]) && line[s] != '\n' && line[s] != EOS)
		s++;
	if (line[s] == '\n')
		return(TRUE);
	else
		return(FALSE);
}

static char ced_isRight2Left(unCH *line) {
	if (!ced_isHebrew && !ced_isArabic)
		return(FALSE);
	for (; *line != EOS; line++) {
		if (*line >= 0x0590 && *line < 0x0780)
			return(TRUE);
	}
	return(FALSE);
}

static char getwLanguageCodeAndName(unCH *code, char isReplace) {
	char buf[BUFSIZ], res;

	u_strcpy(buf, code, BUFSIZ);
	res = getLanguageCodeAndName(buf, isReplace, NULL);
	if (isReplace && res) {
		strncpy(ced_fav_lang, buf, 19);
		ced_fav_lang[19] = EOS;
	}
	return(res);
}

static char ced_isTwoLetterCode(unCH *code) {
	if (strlen(code) < 3)
		return(TRUE);
	if (code[2] == '-')
		return(TRUE);
	return(FALSE);
}

static char ced_isReplacementError(unCH *line, int s) {
	for (s--; s >= 0 && (isSpace(line[s]) || line[s] == '\n' || line[s] == '\r'); s--) ;
	if (s < 0 || line[s] == ']' || line[s] == '>' || uS.isskip(line, s, &tFnt, TRUE))
		return(TRUE);
	for (; s >= 0 && line[s] != '.' && !uS.isskip(line, s, &tFnt, TRUE); s--) ;
	if (uS.isRightChar(line, s, '.', &dFnt, MBF) && uS.isPause(line, s, NULL, NULL))
		return(TRUE);
	s++;
	if (line[s] == '&' || line[s] == '+' || line[s] == '0')
		return(TRUE);
	return(FALSE);
}

static char ced_isNumbersLang(unCH *lang) {
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

static char ced_CheckLanguageCodes(unCH *word, int s, char *isLcodeFound, char *isGlcodeFound) {
	int i, j, li;
	char old_code[20], isAtS;
	unCH t;
	unCH *st;

	if (word[0] == '[' && word[1] == '-') {
		for (i=2; isSpace(word[i]); i++) ;
		if (word[i] == EOS || word[i] == ']')
			return(TRUE);
		isAtS = FALSE;
	} else if ((st=strchr(word, '@')) != NULL) {
		if (st[1] != 's') {
			return(TRUE);
		} else if (st[2] != ':') {
			if (ced_curLanguage == 0) {
				li = 1;
			} else if (ced_curLanguage == 1) {
				li = 0;
			} else {
				li = -1;
			}
			if (li == -1)
				ced_isNumberShouldBeHere = FALSE;
			else if (ced_isNumbersLang(ced_LanguagesTable[li]))
				ced_isNumberShouldBeHere = TRUE;
			else
				ced_isNumberShouldBeHere = FALSE;
			*isLcodeFound = TRUE;
			return(TRUE);
		}
		i = (st - word) + 3;
		isAtS = TRUE;
	} else
		return(TRUE);
	// 2019-04-17	for (j=i; word[j] != EOS && word[j] != ']' && word[j] != '&' && word[j] != '+' && word[j] != '$' && word[j] != ':'; j++) ;
	for (j=i; (word[j] >= 'a' && word[j] <= 'z') || word[j] == '-'; j++) ;
	t = word[j];
	word[j] = EOS;
	if (ced_isTwoLetterCode(word+i)) {
		ced_fav_lang[0] = EOS;
		if (!getwLanguageCodeAndName(word+i, TRUE))
			ced_fav_lang[0] = EOS;
		word[j] = t;
		if (check_err(120,s+i-1,s+j-2,templine)) {
			word[j] = EOS;
 			return(FALSE);
		}
	} else {
		u_strcpy(old_code, word+i, 19);
		old_code[19] = EOS;
		strcpy(ced_fav_lang, old_code);
		if (!getwLanguageCodeAndName(word+i, TRUE)) {
			word[j] = t;
			if (check_err(121,s+i-1,s+j-2,templine)) {
				word[j] = EOS;
				return(FALSE);
			}
		} else {
			if (strcmp(old_code, ced_fav_lang)) {
				word[j] = t;
				if (check_err(120,s+i-1,s+j-2,templine)) {
					word[j] = EOS;
					return(FALSE);
				}
			} else if (isAtS) { // 2019-01-06 @s:rus
				ced_isNumberShouldBeHere = FALSE;
				do {
					if (ced_isNumbersLang(word+i))
						ced_isNumberShouldBeHere = TRUE;
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
				return(TRUE);
			} else if (!ced_isLangMatch(word+i, s+i-1, 152, TRUE)) {
				word[j] = t;
				return(FALSE);
			} else {
				if (ced_isNumbersLang(word+i))
					ced_isNumberShouldBeHere = TRUE;
				else
					ced_isNumberShouldBeHere = FALSE;
				*isGlcodeFound = TRUE;
				for (li=0; ced_LanguagesTable[li] != EOS && li < NUMLANGUAGESINTABLE; li++) {
					if (strcmp(ced_LanguagesTable[li], word+i) == 0) {
						ced_curLanguage = li;
						break;
					}
				}
			}
		}
	}
	word[j] = t;
	return(TRUE);
}

static char ced_isGrouping(unCH *beg, unCH *end, int e) {

	if (beg[0] == 0x2039 && beg[1] == '(' && e >= 1 &&
		end[e-1] == 0x2039 && end[e] == '(')
		return(TRUE);
	return(FALSE);
}

static char ced_isPlayBullet(unCH *word) {
	if (isdigit(word[1]) || (word[1] == '%' && (word[2] == 's' || word[2] == 'm')))
		return(TRUE);
	return(FALSE);
}


static char ced_ParseWords(CHECK_IN_TIERS *ts, BOOL isMedia) {
	int j, r, s, e, te, res;
	int matchType;
	unCH *st, t;
	char sq, hidenc;
	char FirstWordFound = FALSE, FirstBulletFound = FALSE, isTextBetweenBulletsFound = FALSE,
		isBulletFound = FALSE, isTextFound, isPoundSymFound, isSpecialDelimFound, isR2L, CADelFound = FALSE,
		isPreLanguageCodeFound = FALSE, isLastItemBullet,
		isLcodeFound, isLNumberShouldBeHere, isGLcodeFound, isGNumberShouldBeHere, isPauseFound;
	int isFasterMatched, isSlowerMatched, isCreakyMatched, isUnsureMatched, isSofterMatched, 
		isLouderMatched, isLoPitchMatched, isHiPitchMatched, isSmileMatched, isWhisperMatched,
		isYawnMatched, isSingingMatched, isPreciseMatched, isBreathyMatched, isLeftArrowMatched;
	int sb = 0, pb = 0, ab = 0, cb = 0, lpb = 0, gm = 0, sgm = 0, qt = 0, anb = FALSE, DelFound = 0;
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
	ced_curLanguage = 0;
	isGLcodeFound = FALSE;
	isGNumberShouldBeHere = ced_isNumberShouldBeHere;
	ced_line[0] = EOS;
	if (ts->IGN)
		return(TRUE);
	if (*ced_sp == '%' || *ced_sp == '@')
		isTextFound = TRUE;
	else
		isTextFound = FALSE;
	isPoundSymFound = FALSE;
	isSpecialDelimFound = FALSE;
	isR2L = ced_isRight2Left(templine);
	s = 0;
	e = 0;
	isLastItemBullet = FALSE;
	while (1) {
		st = ced_line;
		t = ced_line[0];
		lpb = pb;
		while ((*st=templine[s++]) != EOS && uS.isskip(templine,s-1,&tFnt,TRUE) && !uS.isRightChar(templine,s-1,'[',&tFnt,TRUE)) {
			if (*st == '>')
				isPoundSymFound = FALSE;
/*
			if (is_italic(tempAtt[s-1])) {
				if (check_err(102,s-1,s-1,templine)) return(FALSE);
			}
*/
			if (*st == ',') {
				if (!uS.isskip(templine,s,&tFnt,TRUE) && templine[s] != ',' && templine[s] != EOS && templine[s] != '\n') {
					if (uS.HandleCAChars(templine+s, &matchType)) {
						if (matchType != NOTCA_GROUP_START && matchType != NOTCA_GROUP_END &&
							matchType != NOTCA_SIGN_GROUP_START && matchType != NOTCA_SIGN_GROUP_END &&
							matchType != CA_APPLY_OPTOPSQ && matchType != CA_APPLY_CLTOPSQ &&
							matchType != CA_APPLY_OPBOTSQ && matchType != CA_APPLY_CLBOTSQ) {
							if (check_err(92,s-1,s-1,templine))
								return(FALSE);
						}
					} else if (check_err(92,s-1,s-1,templine))
						return(FALSE);
				} else if (templine[s] == ',' && templine[s+1] == ',') {
					if (check_err(107,s-1,s+1,templine)) return(FALSE);
				} else if (templine[s] == ',') {
					if (check_err(156,s-1,s,templine)) return(FALSE);
				}
			}
			if (*st == '\n') {
				isBulletFound = FALSE;
				if (ced_isBlankLine(templine, s)) {
					if (check_err(91,s,s,templine)) return(FALSE);
				}
	   		} else if (templine[s-1] == '?' && ced_isMorTier() && templine[s] == '|')
				break;
			else if ((res=uS.IsUtteranceDel(templine, s-1))) {
				if (!isSpecialDelimFound) {
					if (!strncmp(ced_sp, "%mor", 4) && ced_lastSpDelim[0] != EOS && ced_lastSpDelim[0] != templine[s-1]) {
						if (check_err(94,s-1,s-1,templine)) return(FALSE);
					}
					if (*ced_sp == '*') {
						ced_lastSpDelim[0] = templine[s-1];
						ced_lastSpDelim[1] = EOS;
					}
				}
				if (ced_atUFound(templine, s-1))
					break;
				if (ts->UTD == -1) {
					if (check_err(48,s-1,s-1,templine)) return(FALSE);
				} else if (ts->UTD == 1) {
					if (DelFound == 1) {
						if (check_err(50,s-1,s-1,templine)) return(FALSE);
					}
					DelFound = res;
					if (!uS.isRightChar(templine,s-2,'+',&tFnt,TRUE) && uS.IsUtteranceDel(templine, s)) {
						if (check_err(50,s,s,templine)) return(FALSE);
					}
					while (templine[s]=='.' || templine[s]=='!' || templine[s]=='?')
						s++;
					while (isSpace(templine[s])) s++;
				}
			} else {
				ced_CheckBrakets(templine,s-1,&sb,&pb,&ab,&cb,&anb);
			}
		}
		if (*st == EOS) {
			if (*ced_sp == '*' && anb) {
				if (check_err(51,s-1,s-1,templine)) return(FALSE);
			}
			break;
		}
		hidenc = FALSE;
		sq = FALSE;
		if (templine[s-1] == CED_HIDEN_C)
			hidenc = TRUE;
		else if (uS.isRightChar(templine,s-1,'[',&tFnt,TRUE))
			sq = TRUE;
		else if (uS.isRightChar(templine,s-1,'(',&tFnt,TRUE) && uS.isPause(templine,s-1,NULL,&r)) ; // Remove to diss-allow (.) pause after '.'
		else {
			if (DelFound && ts->UTD == 1) {
				if (DelFound == 1) {
					DelFound = 3;
					if (!isR2L) {
						if (s >= 3) {
							if (check_err(36,s-3,s-3,templine)) return(FALSE);
						} else {
							if (check_err(36,s-1,s-1,templine)) return(FALSE);
						}
					}
				} else if (DelFound == 2)
					DelFound = 0;
			}
		}
		ced_CheckBrakets(templine,s-1,&sb,&pb,&ab,&cb,&anb);
		e = s;
		if (*st == '&' && *ced_sp == '*') {
			if (templine[e] == '+' && (templine[e+1] < '!' || templine[e+1] > '?')) {
				st++;
				*st = templine[e];
				e++;
			}
			while ((*++st=templine[e]) != EOS && !isSpace(*st) && *st != '\n' && *st != '>' && *st != '[' &&
				   *st != ',' && *st != '+' && *st != '.' && *st != '?' && *st != '!') {
				if (is_italic(tempAtt[e])) {
					if (check_err(102,e-1,e-1,templine)) return(FALSE);
				}
				e++;
			}
		} else {
			while ((*++st=templine[e]) != EOS &&
				   (!uS.isskip(templine,e,&tFnt,TRUE) || (uS.isRightChar(templine, e, '<', &tFnt, TRUE) && ced_line[0] == '+') || sq || hidenc || (*st == '.' && ts->UTD == 0))) {
				if (is_italic(tempAtt[e])) {
					if (check_err(102,e-1,e-1,templine)) return(FALSE);
				}
				if (uS.isRightChar(templine, e, '(', &tFnt, TRUE) && uS.isPause(templine, e, NULL, &r)) {
					if (!uS.isRightChar(ced_line, 0, '[', &tFnt, TRUE) && !ced_isGrouping(ced_line, templine, e)) {
						if (check_err(57,e,e,templine)) return(FALSE);
					}
				}
				if (uS.IsUtteranceDel(templine, e) == 2)
					break;
				e++;
				if (hidenc && templine[e-1] == CED_HIDEN_C) { hidenc = FALSE; st++; break; }
				if (!hidenc && templine[e] == CED_HIDEN_C) { st++; break; }
				if (uS.isRightChar(templine, e-1, ']', &tFnt, TRUE)) { sb--; st++; break; }
				if (!sq)
					ced_CheckBrakets(templine,e-1,&sb,&pb,&ab,&cb,&anb);
				else if (uS.isRightChar(templine, e-1, '[', &tFnt, TRUE))
					sb++;
			}
			if (*ced_sp == '*') {
				if (uS.IsUtteranceDel(templine, e)) {
					if (ced_atUFound(templine, e)) {
						e++;
						while ((*++st=templine[e]) != EOS) {
							if (isSpace(*st) ||
								uS.isRightChar(templine, e, '<', &tFnt, TRUE) || uS.isRightChar(templine, e, '>', &tFnt, TRUE) || 
								uS.isRightChar(templine, e, '[', &tFnt, TRUE) || uS.isRightChar(templine, e, ']', &tFnt, TRUE))
								break;
							if (is_italic(tempAtt[e])) {
								if (check_err(102,e-1,e-1,templine)) return(FALSE);
							}
							e++;
						}
					}
				}
			} else if (!strcmp(ts->code, DATEOF)) {
				if (isSpace(*st) || *st == '\n')
					;
				else {
					while ((*++st=templine[e]) != EOS) {
						if (isSpace(*st) || *st == '\n')
							break;
						if (is_italic(tempAtt[e])) {
							if (check_err(102,e-1,e-1,templine)) return(FALSE);
						}
						e++;
					}
				}
			}
		}
		te = e;
		isSpecialDelimFound = FALSE;
		if (uS.isRightChar(ced_line,0,'+',&tFnt,TRUE) || uS.isRightChar(ced_line,0,'-',&tFnt,TRUE)) {
			if ((uS.IsUtteranceDel(templine, e) || templine[e]==',' || templine[e]=='<') && 
				   !is_unCH_alnum(templine[e-1]) && templine[e-1]>=1 && templine[e-1]<=127) {
				for (te++; uS.IsUtteranceDel(templine, te) || templine[te] == ',' || templine[te] == '<'; te++) {
					*++st = templine[te];
					if (is_italic(tempAtt[te])) {
						if (check_err(102,te-1,te-1,templine)) return(FALSE);
					}
				}
				st++;
			}

			*st = EOS;

			if (!strncmp(ced_sp, "%mor", 4) && uS.IsUtteranceDel(templine, e) && ced_lastSpDelim[0] != EOS && strcmp(ced_lastSpDelim, ced_line)) {
				if (check_err(94,s-1,te-1,templine))
					return(FALSE);
			}
			if (*ced_sp == '*')
				strcpy(ced_lastSpDelim, ced_line);
			isSpecialDelimFound = TRUE;
		} else
			*st = EOS;
		//detecting pause markers before retrace markers-BEG
		if (strcmp(ced_line, "[/]") == 0 || strcmp(ced_line, "[//]") == 0 || strcmp(ced_line, "[///]") == 0 ||
			strcmp(ced_line, "[/-]") == 0) {
			if (isPauseFound && pauseBeg != pauseEnd && e > pauseBeg) {
				if (check_err(159,pauseBeg-1,e-1,templine))
					return(FALSE);
			}
		}
		pauseBeg = 0;
		pauseEnd = 0;
		isPauseFound = FALSE;
		if (uS.isRightChar(ced_line,0,'(',&tFnt,TRUE) && uS.isPause(ced_line,0,NULL,&r)) {
			pauseBeg = s;
			pauseEnd = e;
			isPauseFound = TRUE;
		}
		//detecting pause markers before retrace markers-END
		if (ced_line[0] == '[' && ced_line[1] == ':' && ced_line[2] == ' ') {
			templine[s-1] = ' ';
			templine[s] = ' ';
			templine[e-1] = ' ';
			e = s + 1;
		}
		if (uS.IsUtteranceDel(ced_line, 0) == 2) {
			if (ts->UTD == -1) {
				if (check_err(48,s-1,s-1,templine)) return(FALSE);
			} else if (ts->UTD == 1) {
				if (DelFound == 1) {
					if (check_err(50,s-1,s-1,templine)) return(FALSE);
				}
				DelFound = 2;
			}
		}
		if (!uS.isRightChar(ced_line,0,'[',&tFnt,TRUE) && !uS.isRightChar(ced_line,0,'+',&tFnt,TRUE) &&
			!uS.isRightChar(ced_line,0,'-',&tFnt,TRUE) && !uS.isRightChar(ced_line,0,'#',&tFnt,TRUE) &&
			!uS.IsUtteranceDel(ced_line,0))
			isTextFound = TRUE;
		else if (uS.isRightChar(ced_line,0,'[',&tFnt,TRUE) && uS.isRightChar(ced_line,1,'^',&tFnt,TRUE) && 
				 uS.isRightChar(ced_line,2,' ',&tFnt,TRUE))
			isTextFound = TRUE;

		if (*ced_sp == '*') {
			if (anb && !uS.isRightChar(ced_line,0,'[',&tFnt,TRUE)) {
				if (check_err(51,s-1,s-1,templine)) return(FALSE);
			}
			anb = FALSE;
			if (pb > lpb) {
				if (check_err(55,s-1,te-1,templine)) return(FALSE);
			} else if (pb < lpb) {
				if (check_err(56,s-1,te-1,templine)) return(FALSE);
			}
		}
		if (!isSpace(templine[e]) && !uS.isskip(templine,e,&tFnt,TRUE)) {
			res = uS.HandleCAChars(templine+e, &matchType);
			if (res) {
				if (matchType != CA_APPLY_CLTOPSQ && matchType != CA_APPLY_CLBOTSQ && matchType != NOTCA_GROUP_END &&
					matchType != NOTCA_SIGN_GROUP_END && uS.IsUtteranceDel(templine, e) != 2)
					res = 0;
			}
			if (res == 0) {
				if (templine[e] && templine[e+1] && 
					(*ced_sp == '*' || (templine[e+1] != '\'' && templine[e+1] != '"' && !is_unCH_digit(templine[e+1])))) {
					if (templine[e] == CED_HIDEN_C && e > 0) {
						if (check_err(19,e,e-1,templine)) return(FALSE);
					} else {
						if (check_err(19,e,e,templine)) return(FALSE);
					}
				}
			}
		}
		for (r=0; ced_line[r]; ) {
			res = uS.HandleCAChars(ced_line+r, &matchType);// up-arrow
			if (res) {
/* 2019-04-17 leave this to CHATTER
				if (matchType == CA_APPLY_RISETOHIGH || matchType == CA_APPLY_RISETOMID || matchType == CA_APPLY_LEVEL ||
					matchType == CA_APPLY_FALLTOMID || matchType == CA_APPLY_FALLTOLOW) {
					if (isFasterMatched != -1 || isSlowerMatched != -1 || isCreakyMatched != -1 || isUnsureMatched != -1 ||
						isSofterMatched != -1 || isLouderMatched != -1 || isLoPitchMatched!= -1 || isHiPitchMatched != -1||
						isSmileMatched != -1  || isWhisperMatched != -1|| isYawnMatched != -1   || isSingingMatched != -1||
						isPreciseMatched != -1|| isBreathyMatched != -1|| isLeftArrowMatched != -1) {
						if (*ced_sp == '*') {
							if (check_err(145,s+r-1,s+r-1,templine)) return(FALSE);
						}
					}
				}
*/
				if (matchType == CA_APPLY_RISETOHIGH || matchType == CA_APPLY_RISETOMID || matchType == CA_APPLY_LEVEL ||
					matchType == CA_APPLY_FALLTOMID || matchType == CA_APPLY_FALLTOLOW || matchType == CA_APPLY_UNMARKEDENDING) {
					CADelFound = TRUE;
					if (isBulletFound) {
						if (check_err(118,s-1,e-1,templine)) return(FALSE);
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
			} else if (ced_line[r] == ATTMARKER) {
				if (ced_line[r+1] != EOS && ced_line[r+1] < 0x20) {
					strcpy(ced_line+r, ced_line+r+2);
				}
			} else
				r++;
		}

//		uS.uppercasestr(ced_line, &tFnt, TRUE);
		if (!strcmp(ced_line, "/")) {
			if (check_err(48,s-1,e-1,templine)) return(FALSE);
		}
		isLcodeFound = FALSE;
		isLNumberShouldBeHere = ced_isNumberShouldBeHere;
		if (*ced_sp == '*') {
			j = strlen(ced_line) - 1;
			if (!ced_isCAFound && j > 0 && ced_line[0] == '(' && ced_line[j] == ')') {
				if (!uS.isPause(ced_line, 1, NULL, NULL) && strchr(ced_line+1, '(') == NULL) {
					if (check_err(155,s-1,e-1,templine))
						return(FALSE);
				}
			}
			if (ced_line[0] == '#')
				isPoundSymFound = TRUE;
			else {
				if (ced_line[0] == '[' && ced_line[1] != '+' && ced_line[1] != '^' && isPoundSymFound) {
					if (check_err(71,s-1,e-1,templine)) return(FALSE);
				}
				isPoundSymFound = FALSE;
			}
			if (ced_line[0] == '(' && uS.isPause(ced_line,0,NULL,&r) && DelFound) {
				if (check_err(72,s-1,e-1,templine)) return(FALSE);
			}
			if (uS.isRightChar(ced_line, 0, '[', &tFnt, TRUE)) {
				if (ced_line[1] != '+' && (DelFound || CADelFound)) {
					if (check_err(72,s-1,te-1,templine)) return(FALSE);
				}
				if (ced_line[1] == '+' && !DelFound) {
					if (check_err(75,s-1,te-1,templine)) return(FALSE);
				}
				if (ced_line[1] == '+' && isBulletFound) {
					if (check_err(108,s-1,te-1,templine)) return(FALSE);
				}
			}
			if (ced_line[0] == '[' && ced_line[1] == '-')
				isPreLanguageCodeFound = TRUE;
			if (!ced_CheckLanguageCodes(ced_line, s, &isLcodeFound, &isGLcodeFound))
				return(FALSE);
		}

		if (W_CHECK_COUNT(ced_GenOpt) && ts->WORDCHECK && ced_line[0] != CED_HIDEN_C) {
			if (!ced_CheckWords(ced_line, s, templine, ts)) return(FALSE);
		}

		if (*ced_sp == '*') {
			ced_removeCAChars(ced_line);
		}
		if (ced_line[0]=='+' && (ced_line[1]=='"' || ced_line[1]=='^' || ced_line[1]=='<' || ced_line[1]==',' || ced_line[1]=='+') && ced_line[2]==EOS) {
			if (FirstWordFound || isPreLanguageCodeFound) {
				if (check_err(78,s-1,e-1,templine)) return(FALSE);
			}
		} 
		if (ced_line[0] == CED_HIDEN_C) {
			if (ced_isMultiBullets) {
				if (DelFound)
					isBulletFound = TRUE;
			} else
				isBulletFound = TRUE;
			if (*ced_sp == '*' && ced_isPlayBullet(ced_line)) {
				if (!isTextBetweenBulletsFound && (!ced_isMultiBullets || !isLastItemBullet || !ced_lastBulletOnTier(templine+e))) {
					if (check_err(73,s-1,e-1,templine)) return(FALSE);
				}
				if (!DelFound && !ced_isMultiBullets) {
					int k;
					for (k=e; templine[k] != EOS && uS.isskip(templine,k,&tFnt,TRUE) && !uS.IsUtteranceDel(templine,k); k++) {
						if (templine[k] == '\n')
							break;
					}
					if (templine[k] != '\n') {
						if (check_err(81,s-1,e-1,templine)) return(FALSE);
					}
				}
			}
			if (hidenc) {
				if (check_err(59,s-1,s-1,templine)) return(FALSE);
			}
			if (!ced_isPlayBullet(ced_line)) {
				isTextBetweenBulletsFound = TRUE;
			} else if ((ced_sp[0] == '*' /* 2018-08-06 || ced_isBlob == FALSE*/) && (!ced_isMultiBullets || ced_lastBulletOnTier(templine+e))) {
				if ((res=ced_checkBulletsConsist(&FirstBulletFound, ced_line, isMedia)) != 0) {
					if (check_err(res,s-1,e-1,templine))
						return(FALSE);
				}
				isTextBetweenBulletsFound = FALSE;
			}
			isLastItemBullet = TRUE;
		} else if (is_unCH_digit(*ced_line) && ts->CSUFF) {
			isLastItemBullet = FALSE;
			if (*ced_line != '0' || is_unCH_digit(*(ced_line+1))) {
				if (ced_matchnumber(ts->tplate,ced_line) && !ced_isNumberShouldBeHere) {
					if (check_err(38,s-1,e-1,templine))
						return(FALSE);
				} else {
					FirstWordFound = TRUE;
					isTextBetweenBulletsFound = TRUE;
				}
			} else {
				FirstWordFound = TRUE;
				isTextBetweenBulletsFound = TRUE;
			}
		} else {
			isLastItemBullet = FALSE;
			if (*ced_line == '&') {
				for (r=1; ced_line[r]; r++) {
					if (ced_line[r]=='(') {
						if (check_err(42,s-1,e-1,templine)) return(FALSE);
						break;
					}
				}
				if (ced_line[1] == '=' && ced_line[2] == EOS) {
					if (check_err(146,s-1,e-1,templine)) return(FALSE);
				}
			}
			if (is_unCH_alpha(*(ced_line+1)) || (*(ced_line+1) < 1 && *(ced_line+1) != EOS) || *(ced_line+1) > 127)
				ced_HandleParans(ced_line,NULL);
			if (uS.isRightChar(ced_line, 0, '[', &tFnt, TRUE)) {
				for (r=1; ced_line[r]; r++) {
					if (ced_line[r]=='\n') {
						if (check_err(106,s-1,e-1,templine)) return(FALSE);
						break;
					}
				}
				if (*ced_sp == '*') {
					if (templine[s-1] == '[' && templine[s] != '+' && templine[s] != '-') {
						for (r=s-2; r >= 0; r--) {
							if (templine[r] == ')') {
								for (j=r; j >= 0 && templine[j] != '(' && !uS.isskip(templine, j, &dFnt, MBF); j--) ;
								if (j < 0) {
									if (check_err(150,j,r,templine))
										return(FALSE);
								}
							}
							if (!uS.isskip(templine, r, &dFnt, MBF) || templine[r] == ']')
								break;
							if (templine[r] == '\n' || templine[r] == '<' || templine[r] == '>') {
							} else if (templine[r] == ',') {
//								if (check_err(149,r,r,templine))
//									return(FALSE);
							} else if (!isSpace(templine[r])) {
								if (check_err(149,r,r,templine))
									return(FALSE);
							}
						}
					}
					if (!FirstWordFound) {
						if (ced_line[1] == '<' || ced_line[1] == '>' || ced_line[1] == ':' || ced_line[1] == '/' || ced_line[1] == '"') {
							if (check_err(52,s-1,te-1,templine))
								return(FALSE);
						} else if (ced_line[1] != '-' && ced_line[1] != '^') {
							if (check_err(73,s-1,te-1,templine))
								return(FALSE);
						}
					} else if (ced_line[1] == ':') {
						if (ced_isReplacementError(templine, s-1)) {
							if (check_err(141,s-1,te-1,templine))
								return(FALSE);
						}
						for (j=2; isSpace(ced_line[j]); j++) ;
						uS.isSqCodes(ced_line+j, templine3, &tFnt, TRUE);
						if (templine3[0] == '0' || uS.mStricmp(templine3, "xxx]") == 0 || uS.mStricmp(templine3, "xx]") == 0 ||
							uS.mStricmp(templine3, "yyy]") == 0 || uS.mStricmp(templine3, "yy]") == 0||
							uS.mStricmp(templine3, "www]") == 0 || uS.mStricmp(templine3, "ww]") == 0) {
							if (check_err(158,s-1,te-1,templine))
								return(FALSE);
						}
					}
				}
				if (ced_u.check_patmat(ced_line,cl_T("[\\%fnt: *:*]"))) {
					if (check_err(116,s-1,te-1,templine))
						return(FALSE);
				}
				if ((is_unCH_alpha(*ced_line) || (*ced_line < 1 && *ced_line != EOS) || *ced_line > 127) && ts->CSUFF) {
					char found;
					if (!ced_CheckFixes(ts,ced_line,templine,s,e, &found))
						return(FALSE);
				}
				if ((r=ced_matchplate(ts->tplate, ced_line, 'D'))) {
					if (check_err(r,s-1,te-1,templine)) return(FALSE);
				}
			} else if (*ced_line != EOS) {
				if (ced_line[0] != '+') {
					FirstWordFound = TRUE;
					isTextBetweenBulletsFound = TRUE;
				}
				if (ts->CSUFF == '\002') {
					if (is_unCH_alpha(*ced_line) || (*ced_line < 1 && *ced_line != EOS) || *ced_line > 127) {
						char found;
						if (!ced_CheckFixes(ts,ced_line,templine,s,e,&found)) return(FALSE);
						if (!found) {
							if ((r=ced_matchplate(ts->tplate, ced_line, 'D'))) {
								if (check_err(r,s-1,te-1,templine)) return(FALSE);
							}
						}
					} else {
						if ((r=ced_matchplate(ts->tplate, ced_line, 'D'))) {
							if (check_err(r,s-1,te-1,templine)) return(FALSE);
						}
					}
				} else {
					if ((r=ced_matchplate(ts->tplate, ced_line, 'D'))) {
						if (check_err(r,s-1,te-1,templine)) return(FALSE);
					}
					if ((is_unCH_alpha(*ced_line) || (*ced_line < 1 && *ced_line != EOS) || *ced_line > 127) && ts->CSUFF) {
						char found;
						if (!ced_CheckFixes(ts,ced_line,templine,s,e,&found)) return(FALSE);
					}
				}
			}
		}
		if (isLcodeFound)
			ced_isNumberShouldBeHere = isLNumberShouldBeHere;
		s = e;
	}
	if (isGLcodeFound)
		ced_isNumberShouldBeHere = isGNumberShouldBeHere;
	if (!DelFound && ts->UTD == 1 && !ced_isCAFound) { 
		if (check_err(21,s-2,s-2,templine)) return(FALSE);
	}
	if (!isTextFound) { if (check_err(70,-1,-1,templine)) return(FALSE); }
	if (*ced_sp == '*') {
		if (isFasterMatched != -1) {
			t = uS.HandleCAChars(templine+isFasterMatched, NULL);
			if (check_err(117, isFasterMatched-1, isFasterMatched+t-1, templine))
				return(FALSE);
		}
		if (isSlowerMatched != -1) {
			t = uS.HandleCAChars(templine+isSlowerMatched, NULL);
			if (check_err(117, isSlowerMatched-1, isSlowerMatched+t-1, templine))
				return(FALSE);
		}
		if (isCreakyMatched != -1) {
			t = uS.HandleCAChars(templine+isCreakyMatched, NULL);
			if (check_err(117, isCreakyMatched-1, isCreakyMatched+t-1, templine))
				return(FALSE);
		}
		if (isUnsureMatched != -1) {
			t = uS.HandleCAChars(templine+isUnsureMatched, NULL);
			if (check_err(117, isUnsureMatched-1, isUnsureMatched+t-1, templine))
				return(FALSE);
		}
		if (isSofterMatched != -1) {
			t = uS.HandleCAChars(templine+isSofterMatched, NULL);
			if (check_err(117, isSofterMatched-1, isSofterMatched+t-1, templine))
				return(FALSE);
		}
		if (isLouderMatched != -1) {
			t = uS.HandleCAChars(templine+isLouderMatched, NULL);
			if (check_err(117, isLouderMatched-1, isLouderMatched+t-1, templine))
				return(FALSE);
		}
		if (isLoPitchMatched != -1) {
			t = uS.HandleCAChars(templine+isLoPitchMatched, NULL);
			if (check_err(117, isLoPitchMatched-1, isLoPitchMatched+t-1, templine))
				return(FALSE);
		}
		if (isHiPitchMatched != -1) {
			t = uS.HandleCAChars(templine+isHiPitchMatched, NULL);
			if (check_err(117, isHiPitchMatched-1, isHiPitchMatched+t-1, templine))
				return(FALSE);
		}
		if (isSmileMatched != -1) {
			t = uS.HandleCAChars(templine+isSmileMatched, NULL);
			if (check_err(117, isSmileMatched-1, isSmileMatched+t-1, templine))
				return(FALSE);
		}
		if (isWhisperMatched != -1) {
			t = uS.HandleCAChars(templine+isWhisperMatched, NULL);
			if (check_err(117, isWhisperMatched-1, isWhisperMatched+t-1, templine))
				return(FALSE);
		}
		if (isYawnMatched != -1) {
			t = uS.HandleCAChars(templine+isYawnMatched, NULL);
			if (check_err(117, isYawnMatched-1, isYawnMatched+t-1, templine))
				return(FALSE);
		}
		if (isSingingMatched != -1) {
			t = uS.HandleCAChars(templine+isSingingMatched, NULL);
			if (check_err(117, isSingingMatched-1, isSingingMatched+t-1, templine))
				return(FALSE);
		}
		if (isPreciseMatched != -1) {
			t = uS.HandleCAChars(templine+isPreciseMatched, NULL);
			if (check_err(117, isPreciseMatched-1, isPreciseMatched+t-1, templine))
				return(FALSE);
		}
		if (isBreathyMatched != -1) {
			t = uS.HandleCAChars(templine+isBreathyMatched, NULL);
			if (check_err(117, isBreathyMatched-1, isBreathyMatched+t-1, templine))
				return(FALSE);
		}
		if (isLeftArrowMatched != -1) {
			t = uS.HandleCAChars(templine+isLeftArrowMatched, NULL);
			if (check_err(117, isLeftArrowMatched-1, isLeftArrowMatched+t-1, templine))
				return(FALSE);
		}
	}
	if (sb > 0) { if (check_err(22,-1,-1,templine)) return(FALSE); }
	if (sb < 0) { if (check_err(23,-1,-1,templine)) return(FALSE); }
	if (ab > 0) { if (check_err(24,-1,-1,templine)) return(FALSE); }
	if (ab < 0) { if (check_err(25,-1,-1,templine)) return(FALSE); }
	if (cb > 0) { if (check_err(26,-1,-1,templine)) return(FALSE); }
	if (cb < 0) { if (check_err(27,-1,-1,templine)) return(FALSE); }
	if (pb > 0) { if (check_err(28,-1,-1,templine)) return(FALSE); }
	if (pb < 0) { if (check_err(29,-1,-1,templine)) return(FALSE); }
	if (gm > 0) { if (check_err(128,-1,-1,templine)) return(FALSE); }
	if (gm < 0) { if (check_err(129,-1,-1,templine)) return(FALSE); }
	if (sgm > 0) { if (check_err(130,-1,-1,templine)) return(FALSE); }
	if (sgm < 0) { if (check_err(131,-1,-1,templine)) return(FALSE); }
	if (qt > 0) { if (check_err(136,-1,-1,templine)) return(FALSE); }
	if (qt < 0) { if (check_err(137,-1,-1,templine)) return(FALSE); }
	return(TRUE);
}

static char ced_ParseScope(unCH *wline) {
	int pos, temp, sqCnt;

	pos = strlen(wline) - 1;
	while (pos >= 0) {
		if (uS.isRightChar(wline, pos, ']', &tFnt, TRUE)) {
			sqCnt = 0;
			temp = pos;
			for (pos--; (!uS.isRightChar(wline, pos, '[', &tFnt, TRUE) || sqCnt > 0) && pos >= 0; pos--) {
				if (uS.isRightChar(wline, pos, ']', &tFnt, TRUE))
					sqCnt++;
				else if (uS.isRightChar(wline, pos, '[', &tFnt, TRUE))
					sqCnt--;
			}
			if (pos >= 0) {
				wline[temp] = EOS;
				uS.isSqCodes(wline+pos+1, templine3, &tFnt, TRUE);
				wline[temp] = ']';
			} else
				pos = temp;
		}
		pos--;
	}
	return(TRUE);
}

static char ced_isMissiingID(void) {
	SPLIST *tp;

	for (tp=ced_headsp; tp != NULL; tp = tp->nextsp) {
		if (tp->hasID == FALSE) {
			u_strcpy(err_item, tp->sp, 510);
			err_item[500] = EOS;
			sprintf(ced_err_message, "-Missing @ID header for speaker *%s.", err_item);
			return(TRUE);
		}
	}
	return(FALSE);
}

static char ced_CheckRest(NSTextView *textView, char *filePath, BOOL isMedia) {
	char FTime, isSkip, *s;
	int  i, j, langCnt, len;
	unCH t;
	TCODES *tc = NULL;
	BE_TIERS *tb;
	CHECK_IN_TIERS *ts;
	NSTextStorage *text;
	NSString *textSt;

	FTime = TRUE;
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	textPos = 0;
	ced_clean_IDsp(ced_idsp);
	ced_idsp = NULL;
	while ((isdone=check_getwholeutter(textSt, len)) > 0) {
		isSkip = FALSE;
		if (*ced_sp == '@') {
			init_punct(0);
			if (uS.isUTF8(ced_sp) || ced_u.check_partcmp(ced_sp, PIDHEADER, FALSE) || ced_u.check_partcmp(ced_sp, CKEYWORDHEADER, FALSE) ||
				ced_u.check_partcmp(ced_sp, WINDOWSINFO, FALSE) || ced_u.check_partcmp(ced_sp, FONTHEADER, FALSE))
				continue;
//			uS.uppercasestr(ced_sp, &tFnt, TRUE);
			ced_CleanTierNames(ced_sp);
			if (ced_u.check_partcmp(ced_sp, "@Bg", FALSE)) {
				if ((tb=NEW(BE_TIERS)) == NULL) {
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
				if ((tb->pattern=(unCH *)malloc((strlen(templine)+1)*sizeof(unCH))) == NULL) {
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
				strcpy(tb->pattern, templine);
				tb->nextone = ced_head_bg;
				ced_head_bg = tb;
			} else if (ced_u.check_partcmp(ced_sp, "@Eg", FALSE)) {
				if (ced_head_bg == NULL) {
					if (ced_CodeErr(46)) return(FALSE);
				} else {
					BE_TIERS *to = NULL;
					for (tb=ced_head_bg; tb != NULL; tb=tb->nextone) {
						if (!strcmp(tb->pattern, templine))
							break;
						to = tb;
					}
					if (tb == NULL) {
						if (ced_CodeErr(46)) return(FALSE);
					} else if (to != NULL){
						if (tb == ced_head_bg) ced_head_bg = tb->nextone;
						else to->nextone = tb->nextone;
						free(tb->pattern);
						free(tb);
					} else if (tb == ced_head_bg) {
						ced_head_bg = tb->nextone;
						free(tb->pattern);
						free(tb);
					}
				}
			} else if (ced_u.check_partcmp(ced_sp, "@Languages:", FALSE)) {
				char isIPAFound = FALSE, isFirstLang;
				char old_code[20];

				isSkip = TRUE;
//				if (isdone != 3)
//					resetAllTierCheckFlags(FALSE);
				ced_isArabic = FALSE;
				ced_isHebrew = FALSE;
				ced_isGerman = FALSE;
				ced_isBlob= FALSE;
				ced_isCAFound = FALSE;
				ced_isMultiBullets = FALSE;
				ced_isCheckBullets = TRUE;
				ced_isNumberShouldBeHere = FALSE;
				i = 0;
				langCnt = 0;
				isFirstLang = TRUE;
				while (templine[i]) {
					for (; uS.isskip(templine,i,&tFnt,TRUE) || templine[i] == '\n'; i++) ;
					if (templine[i] == EOS)
						break;
					for (j=i; !uS.isskip(templine,j,&tFnt,TRUE) && templine[j] != '=' && templine[j] != '\n' && templine[j] != EOS; j++) ;
					t = templine[j];
					templine[j] = EOS;
					if (ced_isNumbersLang(templine+i)) {
						if (isFirstLang)
							ced_isNumberShouldBeHere = TRUE;
					} else if (!strcmp(templine+i, "ar") ||!strcmp(templine+i, "ara")) {
						ced_isArabic = TRUE;
					} else if (!strcmp(templine+i, "he") || !strcmp(templine+i, "heb")) {
						ced_isHebrew = TRUE;
					} else if (!strcmp(templine+i, "de") || !strcmp(templine+i, "deu")) {
						ced_isGerman = TRUE;
					} else if (!strcmp(templine+i, "CA") || !strcmp(templine+i, "CA-Unicode")) {
						ced_isCAFound = TRUE;
					} else if (!strcmp(templine+i, "IPA")) {
						isIPAFound = TRUE;
					}
					isFirstLang = FALSE;
					if (langCnt < NUMLANGUAGESINTABLE) {
						strncpy(ced_LanguagesTable[langCnt], templine+i, 8);
						ced_LanguagesTable[langCnt][8] = EOS;
						langCnt++;
					}
					if (ced_isTwoLetterCode(templine+i)) {
						ced_fav_lang[0] = EOS;
						if (!getwLanguageCodeAndName(templine+i, TRUE))
							ced_fav_lang[0] = EOS;
						templine[j] = t;
						if (check_err(120,i,j-1,templine))
							return(FALSE);
						templine[j] = EOS;
					} else {
						u_strcpy(old_code, templine+i, 19);
						strcpy(ced_fav_lang, old_code);
						if (!getwLanguageCodeAndName(templine+i, TRUE)) {
							templine[j] = t;
							if (check_err(121,i,j-1,templine))
								return(FALSE);
							templine[j] = EOS;
						} else {
							if (strcmp(old_code, ced_fav_lang)) {
								templine[j] = t;
								if (check_err(120,i,j-1,templine))
									return(FALSE);
								templine[j] = EOS;
							}
						}
					}
					templine[j] = t;
					if (templine[j] == '=')
						for (; !uS.isskip(templine,j,&tFnt,TRUE) && templine[j] != '\n' && templine[j] != EOS; j++) ;
					i = j;
				}
				if (ced_isCAFound && isIPAFound) {
					if (ced_CodeErr(103))
						return(FALSE);
				} else {
/*
					if (ced_isCAFound) {
						if (strcmp(tFnt.fontName, "CAfont") && strcmp(tFnt.fontName, "Ascender Uni Duo")) {
							if (ced_CodeErr(104)) return(FALSE);
						}
					}
					if (isIPAFound) {
						if (strcmp(tFnt.fontName, "Charis SIL")) {
							if (ced_CodeErr(105)) return(FALSE);
						}
					}
*/
				}
			} else if (ced_u.check_partcmp(ced_sp, "@New Language:", FALSE)) {
				isSkip = TRUE;
				i = 0;
				while (templine[i]) {
					for (; uS.isskip(templine,i,&tFnt,TRUE) || templine[i] == '\n'; i++) ;
					if (templine[i] == EOS)
						break;
					for (j=i; !uS.isskip(templine,j,&tFnt,TRUE) && templine[j] != '=' && templine[j] != '\n' && templine[j] != EOS; j++) ;
					t = templine[j];
					templine[j] = EOS;
					if (!ced_isLangMatch(templine+i, i, 152, TRUE))
						return(FALSE);
					templine[j] = t;
					i = j;
				}
			} else if (ced_u.check_partcmp(ced_sp, "@Options:", FALSE)) {
				char isIPAFound = FALSE;

				i = 0;
				while (templine[i]) {
					for (; uS.isskip(templine,i,&tFnt,TRUE) || templine[i] == '\n'; i++) ;
					if (templine[i] == EOS)
						break;
					for (j=i; !uS.isskip(templine,j,&tFnt,TRUE) && templine[j] != '=' && templine[j] != '\n' && templine[j] != EOS; j++) ;
					t = templine[j];
					templine[j] = EOS;
					if (!uS.mStricmp(templine+i, "heritage")) {
						ced_isBlob = TRUE;
					} else if (!strcmp(templine+i, "multi")) {
						ced_isMultiBullets = TRUE;
					} else if (!strcmp(templine+i, "bullets")) {
						ced_isCheckBullets = FALSE;
					} else if (!strcmp(templine+i, "CA") || !strcmp(templine+i, "CA-Unicode")) {
						ced_isCAFound = TRUE;
					} else if (!strcmp(templine+i, "IPA")) {
						isIPAFound = TRUE;
					} else if (!strcmp(templine+i, "notarget")) {
						ced_applyG2Option = FALSE;
					} else if (!strcmp(templine+i, "mismatch")) {
						ced_check_mismatch = FALSE;
					} else if (!strcmp(templine+i, "dummy")) {
						isdone = 1;
						isSkip = TRUE;
						break;
					}
					templine[j] = t;
					if (templine[j] == '=')
						for (; !uS.isskip(templine,j,&tFnt,TRUE) && templine[j] != '\n' && templine[j] != EOS; j++) ;
					i = j;
				}
				if (ced_isCAFound && isIPAFound) {
					if (ced_CodeErr(103)) return(FALSE);
				} else {
/*
					if (ced_isCAFound) {
						if (strcmp(tFnt.fontName, "CAfont") && strcmp(tFnt.fontName, "Ascender Uni Duo")) {
							if (ced_CodeErr(104)) return(FALSE);
						}
					}
					if (isIPAFound) {
						if (strcmp(tFnt.fontName, "Charis SIL")) {
							if (ced_CodeErr(105)) return(FALSE);
						}
					}
*/
				}
			} else if (ced_u.check_partcmp(ced_sp, MEDIAHEADER, FALSE)) {
				int cnt;
				char isAudioFound, isVideoFound;

				isSkip = TRUE;
				if ((ts=ced_FindTier(ced_headt,ced_sp)) == NULL) {
					if (ced_CodeErr(17))
						return(FALSE);
				}
				i = 0;
				cnt = 0;
				isAudioFound = FALSE;
				isVideoFound = FALSE;
				ced_isUnlinkedFound = FALSE;
				while (templine[i]) {
					for (; uS.isskip(templine,i,&tFnt,TRUE) || templine[i] == ',' || templine[i] == '\n'; i++) {
						if (templine[i] == ',' && i > 0 && isSpace(templine[i-1])) {
							if (check_err(148,i-1,i-1,templine))
								return(FALSE);
						}
					}
					if (templine[i] == EOS)
						break;
					for (j=i; (!uS.isskip(templine,j,&tFnt,TRUE) || templine[j] == '.') && templine[j] != ',' && templine[j] != '\n' && templine[j] != EOS; j++) ;
					t = templine[j];
					templine[j] = EOS;
					cnt++;
					if (cnt > 1) {
						if (templine[i-1] == '.' && cnt == 2) {
							templine[j] = t;
							if (check_err(99,i,j-1,templine))
								return(FALSE);
						} else if (ced_matchplate(ts->tplate, templine+i, 'D')) {
							templine[j] = t;
							if (check_err(113,i,j-1,templine))
								return(FALSE);
						} else if (!uS.mStricmp(templine+i, "audio"))
							isAudioFound = TRUE;
						else if (!uS.mStricmp(templine+i, "video"))
							isVideoFound = TRUE;
						else if (!uS.mStricmp(templine+i, "unlinked"))
							ced_isUnlinkedFound = TRUE;
					} else if (ced_check_mismatch) {
						extractFileName(templineC2, filePath);
						if ((s=strrchr(templineC2, '.')) != NULL)
							*s = EOS;
						u_strcpy(templine2, templineC2, UTTLINELEN);
						if (uS.mStricmp(templine+i, templine2)) {
							s = strrchr(templineC2, '.');
							if (s != NULL && (uS.mStricmp(s, ".elan") == 0 || uS.mStricmp(s, ".praat") == 0)) {
								*s = EOS;
								u_strcpy(templine2, templineC2, UTTLINELEN);
								if (uS.mStricmp(templine+i, templine2)) {
									if (check_err(157,i,j-1,templine))
										return(FALSE);
								}
							} else {
								if (check_err(157,i,j-1,templine))
									return(FALSE);
							}
						}
					}
					templine[j] = t;
					i = j;
				}
				if (!isAudioFound && !isVideoFound) {
					if (ced_CodeErr(114))
						return(FALSE);
				}
/* in 2018-10-03 out 2018-10-07
				if (!ced_isUnlinkedFound) {
					ROWS *tr;

					for (tr=global_dfC->head_text->next_row; tr != global_dfC->tail_text; tr=tr->next_row) {
						for (i=0; tr->line[i] != EOS; i++) {
							if (tr->line[i] == CED_HIDEN_C) {
								if (isdigit(tr->line[i+1]))
									break;
							}
						}
						if (tr->line[i] == CED_HIDEN_C) {
							break;
						}
					}
					if (tr == global_dfC->tail_text) {
						if (check_err(154,0,strlen(templine)-1,templine))
							return(FALSE);
					}
				}
*/
			} else if (ced_u.check_partcmp(ced_sp, "@Exceptions", FALSE)) {
				ced_getExceptions(templine);
			}
			ced_CodeLegalPos = FALSE;
			if (ced_u.check_partcmp(ced_sp,PARTICIPANTS,FALSE)) {
//				if (isdone != 3)
//					resetAllTierCheckFlags(FALSE);
				if (FTime) {
					ced_clean_speakers(ced_headsp);
					ced_headsp = NULL;
				}
				if (!ced_getpart(templine))
					return(FALSE);
			} else if (isdone != 3) {
				if ((ts=ced_FindTier(ced_headt,ced_sp)) == NULL) {
					if (ced_CodeErr(17))
						return(FALSE);
				} else if (ced_u.check_partcmp(ced_sp,AGEOF,TRUE)) {
					if (!ced_Age(ts, ced_sp, templine, 0, EOS))
						return(FALSE);
				} else if (ced_u.check_partcmp(ced_sp,SEXOF,TRUE)) {
					if (!ced_Sex(ts, ced_sp, templine))
						return(FALSE);
				} else if (ced_u.check_partcmp(ced_sp,SESOF,TRUE)) {
					if (!ced_ses(ts, ced_sp, templine, 0, EOS))
						return(FALSE);
				} else if (ced_u.check_partcmp(ced_sp,BIRTHOF,TRUE)) {
					if (!ced_Birth(ts, ced_sp, templine))
						return(FALSE);
				} else if (ced_u.check_partcmp(ced_sp,EDUCOF,TRUE)) {
					if (!ced_Educ(ts, ced_sp, templine))
						return(FALSE);
				} else if (ced_u.check_partcmp(ced_sp,GROUPOF,TRUE)) {
					if (!ced_Group(ts, ced_sp, templine))
						return(FALSE);
				} else if (ced_u.check_partcmp(ced_sp,IDOF,FALSE)) {
					if (!ced_ID(ts))
						return(FALSE);
				} else if (!isSkip) {
					if (!ced_ParseWords(ts, isMedia))
						return(FALSE);
				}
			}
		} else if (isMainSpeaker(*ced_sp)) {
			if (ced_isMissiingID())
				return(FALSE);
			init_punct(0);
			ced_clean_template(ced_headcodes);
			ced_headcodes = NULL;
			ced_CodeLegalPos = TRUE;
//			uS.uppercasestr(ced_sp, &tFnt, TRUE);
			ced_CleanTierNames(ced_sp);
			if (isdone == 3) ;
			else if ((ts=ced_FindTier(ced_maint,ced_sp)) == NULL) {
				if (ced_CodeErr(17))
					return(FALSE);
			} else if (ced_matchSpeaker(ced_headsp,ced_sp+1, FALSE)) {
				if (ced_CodeErr(18))
					return(FALSE);
			} else if (ced_isBlob) {
				if (!ced_ParseBlob(ts, isMedia))
					return(FALSE);
			} else {
				if (!ced_ParseWords(ts, isMedia))
					return(FALSE);
				if (!ced_ParseScope(templine))
					return(FALSE);
			}
		} else if (uS.partcmp(ced_sp, "%wor:",FALSE,FALSE)) {
			init_punct(0);
			ced_clean_template(ced_headcodes);
			ced_headcodes = NULL;
			ced_CodeLegalPos = TRUE;
//			uS.uppercasestr(ced_sp, &tFnt, TRUE);
			ced_CleanTierNames(ced_sp);
			if (isdone == 3) ;
			else if ((ts=ced_FindTier(ced_codet,ced_sp)) == NULL) {
				if (ced_CodeErr(17))
					return(FALSE);
			} else if (ced_isBlob) {
				if (!ced_ParseBlob(ts, isMedia))
					return(FALSE);
			} else {
				if (!ced_ParseWords(ts, isMedia))
					return(FALSE);
				if (!ced_ParseScope(templine))
					return(FALSE);
			}
		} else if (ced_sp[1] != 'x') /* if (*ced_sp == '%') */ {
			if (!ced_CodeLegalPos) {
				if (ced_CodeErr(39))
					return(FALSE);
			}
			ced_CodeLegalPos = TRUE;
//			uS.uppercasestr(ced_sp, &tFnt, TRUE);
			ced_CleanTierNames(ced_sp);
			if (ced_u.check_partcmp(ced_sp,"%pho",FALSE) || ced_u.check_partcmp(ced_sp,"%mod",FALSE))
				init_punct(1);
			else
				init_punct(0);
			if (isdone != 3) {
				if ((ts=ced_FindTier(ced_codet,ced_sp)) == NULL) {
					if (ced_CodeErr(17)) return(FALSE);
				} else if (ced_isBlob) {
					if (!ced_ParseWords(ts, isMedia))
						return(FALSE);
				} else {
					if (!ced_ParseWords(ts, isMedia))
						return(FALSE);
				}
				for (tc=ced_headcodes; tc != NULL; tc=tc->nextone) {
					if (!strcmp(tc->pattern, ced_sp)) {
						if (ced_CodeErr(40)) return(FALSE);
						break;
					}
				}
			}

			if (tc == NULL) {
				if ((tc=NEW(TCODES)) == NULL) {
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
				if ((tc->pattern=(unCH *)malloc((strlen(ced_sp)+1)*sizeof(unCH))) == NULL) {
					free(tc);
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
				strcpy(tc->pattern, ced_sp);
				tc->nextone = ced_headcodes;
				ced_headcodes = tc;
			}
		} else { /* ced_sp == '%x...: */
			if (isdone != 3) {
// Duplicate tiers: off 2019-07-22 on 2021-11-11
				for (tc=ced_headcodes; tc != NULL; tc=tc->nextone) {
					if (!strcmp(tc->pattern, ced_sp)) {
						if (ced_CodeErr(40))
							return(FALSE);
						break;
					}
				}
				if (!ced_ParseBlob(NULL, isMedia)) return(FALSE);
			}

			if (tc == NULL) {
				if ((tc=NEW(TCODES)) == NULL) {
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
				if ((tc->pattern=(unCH *)malloc((strlen(ced_sp)+1)*sizeof(unCH))) == NULL) {
					free(tc);
					strcpy(ced_err_message, "+Error: Out of memory.");
					return(FALSE);
				}
				strcpy(tc->pattern, ced_sp);
				tc->nextone = ced_headcodes;
				ced_headcodes = tc;
			}
		}
		if (isdone == 1)
			break;
		FTime = FALSE;
	}
	if (ced_head_bg != NULL) {
		u_strcpy(templineC, ced_head_bg->pattern, UTTLINELEN);
		strncpy(err_item, "%Bg: ", 511);
		strncat(err_item, templineC, 511);
		err_item[511] = EOS;
		uS.remblanks(err_item);
		if (check_err(45,-1,-1,templine))
			return(FALSE);
	}
	if (isdone < 0)
		return(FALSE);
	else
		return(TRUE);
}

static void addToID(unCH *sp, unCH *role, unCH *languages) {
	long i;

	strcpy(templine2, "@ID:	");
	strcat(templine2, languages);
	strcat(templine2, "|change_me_later|");
	strcat(templine2, sp);
	strcat(templine2, "|||||");
	strcat(templine2, role);
	strcat(templine2, "|||");
	i = strlen(templine2);
	templine2[i++] = '\n';
	templine2[i] = EOS;
	if (strlen(templineW1)+strlen(templine2) < UTTLINELEN)
		strcat(templineW1, templine2);
}

static void moveToParticipantsTier(NSString *textSt, NSUInteger len) {
	textPos = 0;
	sp[0] = EOS;
	ced_line[0] = EOS;
	while (1) {
		if (ced_u.check_partcmp(sp,PARTICIPANTS,FALSE))
			break;
		textPos = getUtts(textPos, len, textSt);
		uS.remblanks(ced_line);

		if (!isSpeaker(sp[0]))
			return;
	}
	textPos = getUtts(textPos, len, textSt);
	uS.remblanks(ced_line);
	if (!isSpeaker(sp[0]))
		return;
	while (1) {
		if (isMainSpeaker(sp[0]))
			break;
		textPos = getUtts(textPos, len, textSt);
		uS.remblanks(ced_line);

		if (!isSpeaker(sp[0]))
			break;
	}
}

static char createIDTier(NSString *textSt, NSUInteger len, unCH *line, unCH *languages) {
	int i;
	unCH sp[SPEAKERLEN];
	unCH *rootline = line;
	unCH *s, *e, t;
	char wc;
	short cnt = 0;

	isdone = 2;
	for (line=rootline; *line && isSpace(*line); line++) ;
	s = line;
	sp[0] = EOS;
	while (*s) {
		if (*line == ',' || isSpace(*line) || *line == '\n' || *line == EOS) {
			if (*line == ',' && !isSpace(*(line+1)) && *(line+1) != '\n') {
				moveToParticipantsTier(textSt, len);
				ced_trans_err(92,line,line,rootline);
				return(FALSE);
			}
			wc = ' ';
			e = line;
			for (; isSpace(*line) || *line == '\n'; line++) ;
			if (*line != ',' && *line != EOS)
				line--;
			else
				wc = ',';
			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == ',') {
					if (ced_badrole(s,cl_T(PARTICIPANTS))) {
						*e = t;
						moveToParticipantsTier(textSt, len);
						ced_trans_err(15,s,e-1,rootline);
						return(FALSE);
					}
					addToID(sp, s, languages);
				} else if (cnt == 0) {
					for (i=0; s[i] > 32 && s[i] < 127 && s[i] != EOS; i++) ;
					if (s[i] != EOS) {
						moveToParticipantsTier(textSt, len);
						ced_trans_err(16,line-strlen(s),line-1,rootline);
						return(FALSE);
					} else {
						strcpy(sp, s);
					}
				}
				*e = t;
				if (wc == ',') {
					if (cnt == 0) {
						moveToParticipantsTier(textSt, len);
						ced_trans_err(12,line,line,rootline);
						return(FALSE);
					}
					cnt = -1;
					sp[0] = EOS;
				}
				for (line++; isSpace(*line) || *line=='\n' || *line==','; line++) {
					if (*line == ',') {
						if (cnt == 0) {
							moveToParticipantsTier(textSt, len);
							ced_trans_err(12,line,line,rootline);
							return(FALSE);
						}
						cnt = -1;
						sp[0] = EOS;
					}
				}
			} else {
				for (line=e; *line; line++) ;
				if (cnt == 0) {
					moveToParticipantsTier(textSt, len);
					ced_trans_err(12,e,e,rootline);
					return(FALSE);
				} else {
					t = *e;
					*e = EOS;
					if (ced_badrole(s,cl_T(PARTICIPANTS))) {
						moveToParticipantsTier(textSt, len);
						ced_trans_err(15,s,e-1,rootline);
						return(FALSE);
					}
					addToID(sp, s, languages);
					*e = t;
				}
				for (line=e; *line; line++) ;
			}
			cnt++;
			s = line;
		} else
			line++;
	}
	return(TRUE);
}

static void addIDs(NSTextView *textView) {
	long i, j;
	NSUInteger len;
	char ParticipantsFound, delimFound;
	unCH languages[1024];
	NSTextStorage *text;
	NSString *textSt;

	ParticipantsFound = FALSE;
	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	textPos = 0;
	templine[0] = EOS;
	sp[0] = EOS;
	ced_line[0] = EOS;
	strcpy(languages, "change_me_later");
	while (1) {
		if (ParticipantsFound) {
			if (ced_u.check_partcmp(sp,"@Options:",FALSE)) {
			} else {
				if (!createIDTier(textSt, len, templine4, languages)) {
					return;
				}
				break;
			}
		}
		if (ced_u.check_partcmp(sp,"@Languages:",FALSE)) {
			delimFound = TRUE;
			i = 0L;
			for (j=0L; ced_line[i] != EOS; i++) {
				if (isSpace(ced_line[i]) || ced_line[i] == ',') {
					if (!delimFound) {
						languages[j++] = ',';
						languages[j++] = ' ';
						delimFound = TRUE;
					}
				} else {
					languages[j++] = ced_line[i];
					delimFound = FALSE;
				}
			}
			languages[j] = EOS;
			uS.remblanks(languages);
		} else if (ced_u.check_partcmp(sp,PARTICIPANTS,FALSE)) {
			strcpy(templine4, ced_line);
			ced_CurSpLen = strlen(sp);
			ParticipantsFound = TRUE;
		}
		begTPos = textPos;
		textPos = getUtts(textPos, len, textSt);
		uS.remblanks(ced_line);

		if (!isSpeaker(sp[0]))
			break;
		if (isMainSpeaker(sp[0]))
			break;
	}
	if (ParticipantsFound && templineW1[0] == EOS && (!isSpeaker(sp[0]) || isMainSpeaker(sp[0]))) {
		if (!createIDTier(textSt, len, templine4, languages)) {
			return;
		}
	}
	if (ParticipantsFound && templineW1[0] != EOS) {
		NSString *keys;
		NSRange charRange;

		charRange = NSMakeRange(begTPos, 0);
		keys = [NSString stringWithCharacters:templineW1 length:strlen(templineW1)];
		if ([textView shouldChangeTextInRange:charRange replacementString:keys]) {
			[text replaceCharactersInRange:charRange withString:keys];
			[textView didChangeText];
			[[textView undoManager] setActionName:@"IDs"];
		}
		strcpy(ced_err_message, "+Added ID tier(s).\nPlease use \"Tiers->ID headers\" menu to change them.");
	} else {
		if (check_err(60,-1,-1,templine4)) {
		}
	}
}

static char ced_OverAll(NSTextView *textView) {
	int cnt, j, len;
	unCH  lSP[SPEAKERLEN+1];
	NSTextStorage *text;
	NSString *textSt;

	text = [textView textStorage];
	textSt = [text string];
	len = [text length];
	textPos = 0;
	fillup_templine(textSt, len, templine, false);
	uS.remblanks(templine);
	strcpy(ced_sp, "*");
	if (!isSpeaker(templine[0])){
		if (check_err(1,0,0,templine)) return(FALSE);
	}
	lSP[0] = EOS;
	cnt = 0;
	do {
		if (uS.partcmp(templine, "@Options:",FALSE,FALSE) || uS.partcmp(templine, "@Languages:",FALSE,FALSE)) {
			for (j=0; templine[j] != EOS; j++) {
				if (!strncmp(templine+j, "CA", 2) || !strncmp(templine+j, "CA-Unicode", 10)) {
					ced_isCAFound = TRUE;
				} else if (!strncmp(templine+j, "notarget", 8)) {
					ced_applyG2Option = FALSE;
				}
			}
		}
		if (uS.isUTF8(templine) || ced_u.check_partcmp(templine, PIDHEADER, FALSE) || ced_u.check_partcmp(templine, CKEYWORDHEADER, FALSE) ||
			 ced_u.check_partcmp(templine, WINDOWSINFO, FALSE) || ced_u.check_partcmp(templine, FONTHEADER, FALSE)) ;
		else /* if (CMP_CHECK_ID1(global_dfC->row_txt->flag)) */ {
			if (ced_AtEndFound && *templine) {
				if (check_err(44,-1,-1,templine)) return(FALSE);
			}
			if (!isSpeaker(*templine) && !isSpace(*templine) && *templine != EOS) {
				if (check_err(8,0,0,templine)) return(FALSE);
			}
			if (!ced_CheckErr(templine)) return(FALSE);
			if (!ced_Tabs(templine)) return(FALSE);
			if (!ced_Colon(templine)) return(FALSE);

			if (*templine == '@') {
				strcpy(ced_line, templine);
				uS.remblanks(ced_line);
//				uS.lowercasestr(ced_line, &tFnt, FALSE);
				if (!strcmp(ced_line,"@Begin")) {
					if (ced_AtBeginFound) {
						if (check_err(53,-1,-1,templine)) return(FALSE);
					}
					ced_AtBeginFound = TRUE;
				} else if(!strcmp(ced_line,"@End")) {
					if (ced_AtEndFound) {
						if (check_err(54,-1,-1,templine)) return(FALSE);
					}
					ced_AtEndFound = TRUE;
				} else if (ced_line[0] == '@' && ced_line[1] == 'I' &&
						   ced_line[2] == 'D' && ced_line[3] == ':') {
					ced_AtIDFound = TRUE;
				}
			}

			if (lHeaders[cnt][0] != EOS && !ced_u.check_partcmp(ced_line, lHeaders[cnt], FALSE)) {
				if (cnt == 0) {
					if (check_err(43,-1,-1,templine)) return(FALSE);
					ced_AtBeginFound = TRUE;
				} else if (cnt == 1) {
					if (check_err(77,-1,-1,templine)) return(FALSE);
				} else if (cnt == 2) {
					if (check_err(61,-1,-1,templine)) return(FALSE);
				}
			}
			if (lHeaders[cnt][0] != EOS)
				cnt++;
//			FALSE_CHECK_ID1(global_dfC->row_txt->flag);
		}
/*
		else {
			if (*templine == '@') {
				strcpy(ced_line, templine);
				uS.remblanks(ced_line);
//				uS.lowercasestr(ced_line, &tFnt, FALSE);
				if (!strcmp(ced_line,"@Begin")) {
					ced_AtBeginFound = TRUE;
				} else if(!strcmp(ced_line,"@End")) {
					ced_AtEndFound = TRUE;
				} else if (ced_line[0] == '@' && ced_line[1] == 'I' &&
						   ced_line[2] == 'D' && ced_line[3] == ':') {
					ced_AtIDFound = TRUE;
				}
				if (lHeaders[cnt][0] != EOS)
					cnt++;
			}
		}
*/
		if (ced_u.check_partcmp(templine, "@Options", FALSE)) {
			if (!ced_u.check_partcmp(lSP, "@Participants", FALSE)) {
				if (check_err(125,0,strlen(templine),templine)) return(FALSE);
			}
		} else if (ced_u.check_partcmp(templine, "@ID:", FALSE)) {
			if (!ced_u.check_partcmp(lSP,"@Participants",FALSE) && !ced_u.check_partcmp(lSP,"@Options",FALSE) &&
				!ced_u.check_partcmp(lSP,"@ID:",FALSE)) {
				if (check_err(126,0,strlen(templine),templine)) return(FALSE);
			}
		} else if (ced_u.check_partcmp(templine,"@Birth of",FALSE) || ced_u.check_partcmp(templine,"@L1 of",FALSE) ||
				   ced_u.check_partcmp(templine,"@Birthplace of",FALSE)) {
			if (!ced_u.check_partcmp(lSP,"@ID:",FALSE) && !ced_u.check_partcmp(lSP,"@Birth of",FALSE) &&
				!ced_u.check_partcmp(lSP,"@L1 of",FALSE) && !ced_u.check_partcmp(lSP,"@Birthplace of",FALSE)) {
				if (check_err(127,0,strlen(templine),templine)) return(FALSE);
			}
		} 
		if (textPos >= len)
			break;
		if (isSpeaker(templine[0])) {
			strncpy(lSP, templine, SPEAKERLEN);
			lSP[SPEAKERLEN] = EOS;
		}
		fillup_templine(textSt, len, templine, false);
		uS.remblanks(templine);
	} while (1) ;
	return(TRUE);
}

void CheckC(NSTextView *textView, char *filePath, BOOL isMedia) {
	int  len;
	char isverytop;
	NSTextStorage *text;
	NSString *textSt;

	tFnt.isUTF = TRUE;
	tFnt.fontName[0] = EOS;
	ced_applyG2Option = TRUE;
	templineW1[0] = EOS;
	isverytop = TRUE;
	ced_check_init(FALSE);
/*
	if (!global_dfC->ChatMode) {
		strcpy(ced_err_message, "+Illegal in TEXT mode.");
		return;
	}
*/
	initLanguages();
	if (!ReadLangsFile(TRUE)) {
		strcpy(ced_err_message, "+Error: Can't open language codes file \"ISO-639\". Please make sure library directory in commands window is set correctly.");
		return;
	}
	init_punct(0);
	ced_lastSpDelim[0] = EOS;
	if (!ced_ReadDepFile(dep_file, TRUE)) {
		ced_check_init(FALSE);
		cleanupLanguages();
		return;
	}
	Check_level = 0;
	if (!ced_OverAll(textView)) {
//		DisplayErrorRow();
//		ResetUndos();
		ced_check_init(FALSE);
		cleanupLanguages();
		[textView setSelectedRange:ced_cursorRange];
		[textView scrollRangeToVisible:ced_cursorRange];
		return;
	}
	Check_level = 1;
	if (!ced_AtBeginFound) {
		text = [textView textStorage];
		textSt = [text string];
		len = [text length];
		textPos = 0;
		fillup_templine(textSt, len, templine, false);
		if (check_err(6,-1,-1, templine)) {
//			DisplayErrorRow();
//			ResetUndos();
			ced_check_init(FALSE);
			cleanupLanguages();
			[textView setSelectedRange:ced_cursorRange];
			[textView scrollRangeToVisible:ced_cursorRange];
			return;
		}
	}

	if (!ced_AtIDFound && IS_CHECK_ID(ced_GenOpt)) {
		addIDs(textView);
		ced_check_init(FALSE);
		cleanupLanguages();
		[textView setSelectedRange:ced_cursorRange];
		[textView scrollRangeToVisible:ced_cursorRange];
		return;
	}

	if (!ced_AtEndFound) {
		text = [textView textStorage];
		textSt = [text string];
		len = [text length];
		textPos = 0;
		fillup_templine(textSt, len, templine, false);
		if (check_err(7,-1,-1,templine)) {
//			DisplayErrorRow();
//			ResetUndos();
			ced_check_init(FALSE);
//			EndOfFile(-1);
			ced_check_init(FALSE);
			cleanupLanguages();
			[textView setSelectedRange:ced_cursorRange];
			[textView scrollRangeToVisible:ced_cursorRange];
			return;
		}
	}
	if (!ced_CheckRest(textView, filePath, isMedia)) {
//		DisplayErrorRow();
//		ResetUndos();
		ced_check_init(FALSE);
		cleanupLanguages();
		[textView setSelectedRange:ced_cursorRange];
		[textView scrollRangeToVisible:ced_cursorRange];
		return;
	}
	if (isverytop) {
		strcpy(ced_err_message, "-Success! No errors found.");
	}
	ced_check_init(FALSE);
	cleanupLanguages();
}
/* main check functions end */
