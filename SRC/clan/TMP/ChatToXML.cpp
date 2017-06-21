#define _IS_UNICODE

#include <stdio.h>
#include <wctype.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <wchar.h>
#include <stdarg.h>

#define _INC_WSTRING
#undef char
#define char short
#ifndef _IS_UNICODE
	#include "fontconvert.h"
	#if defined(_WIN32)
		#include "MSUtil.h"
	#endif
#else
	#if defined(_WIN32)
		#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
		#include <afxwin.h>         // MFC core and standard components
	#endif
	struct tdFnt {
		short Encod;
	} ;
	static struct tdFnt dFnt = {0};
#endif
#undef char
#define char char

#include "ChatToXML.h"
#ifdef _WIN32 
	#define watoi _wtoi
	#define watol _wtol
#endif

#ifndef NEW
#if defined(_MAC_CODE)
	#define NEW(type) ((type *)malloc((size_t) sizeof(type)))
#endif
#if defined(_WIN32)
	#define NEW new
#endif
#endif /* NEW */

#ifndef EOS			// End Of String marker
	#define EOS 0
#endif
#ifndef TRUE
	#define TRUE  1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#define UTTLINELEN 6000		 /* max len of the line part of the turn */
#define SPEAKERLEN 256		 /* max len of the code part of the turn */
#define FONTMARKER		"[%fnt: "
#define FONTHEADER		"@Font:"
#define CKEYWORDHEADER	"@Color words:"
#define PARTICIP  		"@PARTICIPANTS"
#define MORPHS "-#~"
#define PUNCTUATION_SET ",.;?![]<>"
#define PUNCT_PHO_MOD_SET ""

#define isSpeaker(c,d) ((c) == '@' || (c) == '%' || (!(d) && ((c) == '*')) || ((d) && !isSpace((c))))
#define isMainSpeaker(c,d) ((!(d) && (c) == '*') || ((d) && !isSpace((c)) && (c) != '%' && (c) != '@'))

#define HIDEN_C			'\025'
#define ATTMARKER		'\002'

#define set_changed_to_1(x)		(x | 1)
#define is_changed(x)			(x & 1)
#define set_changed_to_0(x)		(x & 0xfe)

#define set_reversed_to_1(x)	(x | 2)
#define is_reversed(x)			(x & 2)
#define set_reversed_to_0(x)	(x & 0xfd)

#define underline_start			'\001'
#define underline_end			'\002'
#define set_underline_to_1(x)	(x | 4)
#define is_underline(x)			(x & 4)
#define set_underline_to_0(x)	(x & 0xfb)

#define italic_start			'\003'
#define italic_end				'\004'
#define set_italic_to_1(x)		(x | 8)
#define is_italic(x)			(x & 8)
#define set_italic_to_0(x)		(x & 0xf7)

#define error_start				'\005'
#define error_end				'\006'
#define set_error_to_1(x)		(x | 16)
#define is_error(x)				(x & 16)
#define set_error_to_0(x)		(x & 0xef)

#define set_color_to_1(x)		(x | 32)
#define is_colored(x)			(x & 32)
#define set_color_to_0(x)		(x & 0xdf)

/*
#define set_X_to_1(x)		(x | 64)
#define is_X(x)				(x & 64)
#define set_X_to_0(x)		(x & 0xbf)
*/

#define isSpace(c)	 ((c) == ' ' || (c) == '\t')

struct WCHARMASK {
	char c1, c2;
} *wc;

enum {
	UP,
	DN
} ;

enum {
	GROUP,
	EVENT,
	CODE,
	WORDT
} ;
enum {
	MORTIER,
	PHOTIER,
	MODTIER,
	ADDTIER,
	OTHTIER
} ;

#define CODESMAX 6
struct items {
	short word[1024];
	int   codesCnt;
	struct items *code[CODESMAX];
	struct items *replacement;
	struct items *morWord;
	struct items *phoWord;
	struct items *modWord;
	struct items *addWord;
	short  lang[5];
	char   omited;
	char   morOmited;
	char   isChange;
	char   sndMark;
	struct items *NextItem;
	struct items *PrevItem;
} ;

struct tiers {
	short sp[SPEAKERLEN];
	short orgLine[UTTLINELEN];
	short line[UTTLINELEN];
	struct tiers *NextUtt;
} ;

struct priolst {
	short priority;
	short *st;
	char  isInsideWord;
	char  isUsed;
	char  isPad;
	struct priolst *next;
} ;

struct overlap {
//  short ov_sp[SPEAKERLEN];
//  short ov_line[UTTLINELEN];
//  long lineno;
	int	 cId;
	int  with;
	int  num;
	char isUp, isDn;
	struct overlap *NextOverlap;
} ;

struct participants {
	short sp[SPEAKERLEN];
	short line[UTTLINELEN];
	struct participants *NextPart;
} ;

#define WUTTER struct wutterance /* main utterance structure             */
WUTTER {
    short speaker[SPEAKERLEN];	/* code descriptor field of the turn	*/
    short attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
    short line[UTTLINELEN+1];	/* text field of the turn		*/
    short attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
} ;				/* there is only 1 utterance, i.e. no	*/
				/* windows, then if points to itself	*/

static struct overlap *overlaps, *cOverlap;
static struct participants *speakers;
static struct items *words, *mor, *pho, *mod, *add;
static struct tiers *headers, *spTier;
static struct priolst *priList;
static short tempst1[UTTLINELEN], tempst2[UTTLINELEN], tempst3[UTTLINELEN],
			 tempst4[UTTLINELEN], tempst5[UTTLINELEN], tempst6[UTTLINELEN];
static short uLanguage[5], gLanguage[5], lLanguage[5], gScript[5];
static char  tempchar[UTTLINELEN];
static char  UEncoding;
static char  isJustCheck;
static char  getrestline = 1;
static char  skipgetline = FALSE;
static char  isWarning;
static char  MBF = false;
static char  isCainFile = false;
static int   errNum;
static int   getc_cr_lc = '\0';
static char  *oldfname;
static short currentchar;
static short currentatt;
static short punctuation[50];
static short *chatSt;
static char  *xmlSt;
static size_t xmlStLen;
static size_t xmlStMax;
static long  lineno;
static long  tlineno;
static cChatToXMLStr mStr;
static WUTTER wutterance;
static FILE *warningFP;
static jmp_buf my_jmp_buf;

#ifndef _IS_UNICODE
	static FONTINFO dFnt;
#endif


static void CToW(const char *c, short *s, int cnt) {
	for (; *c != EOS && cnt > 1; c++, s++, cnt--)
		*s = (short)*c;
	*s = (short)EOS;
}

static char *WToC(short *s, char *c, int cnt) {
	long i;

	for (i=0L; s[i] != (short)EOS && cnt > 1; i++, cnt--)
		c[i] = (char)s[i];
	c[i] = EOS;
	return(c);
}

static struct overlap *freeOverlaps(struct overlap *p) {
	struct overlap *t;

	while (p != NULL) {
		t = p;
		p = p->NextOverlap;
		free(t);
	}
	return(NULL);
}

static struct tiers *freeHeaders(struct tiers *p) {
	struct tiers *t;

	while (p != NULL) {
		t = p;
		p = p->NextUtt;
		free(t);
	}
	return(NULL);
}

static struct items *freeItems(struct items *p);

static void freeItem(struct items *t) {
	int i;

	for (i=0; i < t->codesCnt; i++) {
		freeItems(t->code[i]->replacement);
		free(t->code[i]);
	}
	if (t->morWord != NULL)
		free(t->morWord);
	if (t->phoWord != NULL)
		free(t->phoWord);
	if (t->modWord != NULL)
		free(t->modWord);
	if (t->addWord != NULL)
		free(t->addWord);
	freeItems(t->replacement);
	free(t);
}

static struct items *freeItems(struct items *p) {
	struct items *t;

	while (p != NULL) {
		t = p;
		p = p->NextItem;
		freeItem(t);
	}
	return(NULL);
}

static void freeSpeakers(void) {
	struct participants *t;

	while (speakers != NULL) {
		t = speakers;
		speakers = speakers->NextPart;
		free(t);
	}
}

static struct priolst *freePriList(struct priolst *p) {
	struct priolst *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->st != NULL)
			free(t->st);
		free(t);
	}
	return(NULL);
}

static void freeAll(int num) {
	headers = freeHeaders(headers);
	words = freeItems(words);
	mor = freeItems(mor);
	pho = freeItems(pho);
	mod = freeItems(mod);
	add = freeItems(add);	
	overlaps = freeOverlaps(overlaps);
	freeSpeakers();
	if (num) {
		if (xmlSt != NULL)
			free(xmlSt);
		errNum = num;
		longjmp(my_jmp_buf, 1);
	}
}

static void shiftCodeArray(struct items *w, int codesCnt) {
	for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
		w->code[codesCnt] = w->code[codesCnt+1];
	}
	w->codesCnt--;
}

/* write chat data start */
static void toChousenOutput(char c) {
	if (xmlStLen >= xmlStMax) {
		char *t;

		t = xmlSt;
		xmlSt = (char *)malloc(xmlStLen+xmlStMax);
		if (xmlSt == NULL)
			freeAll(1);
		memcpy(xmlSt, t, xmlStLen);
		free(t);
		xmlStMax = xmlStLen + xmlStMax;
	}
	xmlSt[xmlStLen] = c;
	xmlStLen++;
}

/* convert to UNICODE begin */
#ifdef _WIN32 
#include <mbstring.h>

static void AsciiToUnicode(char *lpszText) {
	long total;

	total = mStr.strlen((char *)lpszText);
	// Replace the editing edit buffer with the newly loaded data
	//long chars=_mbslen(lpszText);
	//long wchars=mbstowcs(NULL,(const char*)lpszText,chars);
	long wchars=MultiByteToWideChar(CP_ACP/*1251*/,0,(const char*)lpszText,total,NULL,0);
	LPVOID hwText = LocalAlloc(LMEM_MOVEABLE, (wchars+1)*2);
	LPWSTR lpwText = (LPWSTR)LocalLock(hwText);
	MultiByteToWideChar(CP_ACP/*1251*/,0,(const char*)lpszText,total,lpwText,wchars);

	for (total=0L; total < wchars; total++) {
		if (lpwText[total] == (short)'\n') {
			toChousenOutput('\r');
			toChousenOutput(0);
		}
		wc = (struct WCHARMASK *)&lpwText[total];
		toChousenOutput(wc->c1);
		toChousenOutput(wc->c2);
	}

	LocalUnlock(hwText);
	HLOCAL LocalFree(hwText);
}
#endif /* _WIN32 */
/* convert to UNICODE begin */

static void xml_output(char *st, short *ust) {
	long i;

	if (isJustCheck)
		return;
	if (st != NULL) {
		for (i=0L; st[i] != 0; i++) {
			if (UEncoding == UNOT) {
#if defined(_WIN32)
/*
				AsciiToUnicode(st);
				break;
*/
if (st[i] == '\n') {
toChousenOutput(0x0d);
toChousenOutput('\0');
toChousenOutput(0x0a);
toChousenOutput('\0');
} else {
toChousenOutput(st[i]);
toChousenOutput('\0');
}
#else
				toChousenOutput('\0');
				if (st[i] == '\n') {
					toChousenOutput(0x0d);
				} else {
					toChousenOutput(st[i]);
				}
#endif
			} else if (UEncoding == UPC) {
				if (st[i] == '\n') {
					toChousenOutput(0x0d);
					toChousenOutput('\0');
					toChousenOutput(0x0a);
					toChousenOutput('\0');
				} else {
					toChousenOutput(st[i]);
					toChousenOutput('\0');
				}
			} else if (UEncoding == UMAC) {
				toChousenOutput('\0');
				if (st[i] == '\n')
					toChousenOutput(0x0d);
				else
					toChousenOutput(st[i]);
			}
		}
	}
	if (ust != NULL) {
		for (i=0L; ust[i] != 0; i++) {
			if (UEncoding == UNOT) {
#if defined(_WIN32)
/*
				WToC(ust, tempchar, UTTLINELEN+UTTLINELEN);
				AsciiToUnicode(tempchar);
				break;
*/
if (ust[i] == '\n') {
toChousenOutput(0x0d);
toChousenOutput('\0');
toChousenOutput(0x0a);
toChousenOutput('\0');
} else {
wc = (struct WCHARMASK *)&ust[i];
toChousenOutput(wc->c1);
toChousenOutput('\0');
}
#else
				toChousenOutput('\0');
				if (ust[i] == '\n')
					toChousenOutput(0x0d);
				else {
					wc = (struct WCHARMASK *)&ust[i];
					toChousenOutput(wc->c2);
				}
#endif
			} else if (UEncoding == UPC) {
				if (ust[i] == '\n') {
					toChousenOutput(0x0d);
					toChousenOutput('\0');
					toChousenOutput(0x0a);
					toChousenOutput('\0');
				} else {
					wc = (struct WCHARMASK *)&ust[i];
#if defined(_WIN32)
					toChousenOutput(wc->c1);
					toChousenOutput(wc->c2);
#else
					toChousenOutput(wc->c2);
					toChousenOutput(wc->c1);
#endif
				}
			} else if (UEncoding == UMAC) {
				if (ust[i] == '\n') {
					toChousenOutput('\0');
					toChousenOutput(0x0d);
				} else {
					wc = (struct WCHARMASK *)&ust[i];
#if defined(_WIN32)
					toChousenOutput(wc->c2);
					toChousenOutput(wc->c1);
#else
					toChousenOutput(wc->c1);
					toChousenOutput(wc->c2);
#endif
				}
			}
		}
	}
}
/* write chat data end */

/* Handle Text Attributes begin */
static char SetTextAtt(short *st, short c, short *att, char isRawData) {
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
	} else if (c == error_start) {
		if (att != NULL)
			*att = set_error_to_1(*att);
		found = TRUE;
	} else if (c == error_end) {
		if (att != NULL)
			*att = set_error_to_0(*att);
		found = TRUE;
	}

	if (found && !isRawData) {
		if (st != NULL)
			mStr.strcpy(st, st+2);
		return(TRUE);
	} else {
		return(FALSE);
	}
}
/* Handle Text Attributes end */

#ifndef _IS_UNICODE
/*	 Find and set the current font of the tier begin */
#if defined(_WIN32)
static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *font,NEWTEXTMETRICEX *t,int ft,LPARAM finfo) {
	NewFontInfo *fi = (NewFontInfo *)finfo;

	if (mStr.strcmp(fi->fontName, font->elfLogFont.lfFaceName) == 0 &&
		(fi->CharSet == DEFAULT_CHARSET || fi->CharSet == font->elfLogFont.lfCharSet)) {
		fi->CharSet = font->elfLogFont.lfCharSet;
		return(0);
	}
	return(1);
}

static int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *font,NEWTEXTMETRIC FAR *t,int ft,LPARAM finfo) {
	
	NewFontInfo *fi = (NewFontInfo *)finfo;

	if (mStr.strcmp(fi->fontName, font->elfLogFont.lfFaceName) == 0 &&
		(fi->CharSet == DEFAULT_CHARSET || fi->CharSet == font->elfLogFont.lfCharSet)) {
		fi->CharSet = (int)font->elfLogFont.lfCharSet;
		return(0);
	}
	return(1);
}
#endif /* _WIN32 */

static char selectChoosenFont(NewFontInfo *finfo) {
#if defined(_MAC_CODE)
	if (!GetFontNumber((unsigned char *)finfo->fontName, &finfo->fontId)) {
		mStr.sprintf(tempst2, "Macintosh font \"%ls\" is not found on this computer",finfo->fontName);
		freeAll(47);
	}
	dFnt.FName = finfo->fontId;
	dFnt.FSize = finfo->fontSize;
	dFnt.CharSet = finfo->CharSet;
	dFnt.Encod = my_FontToScript(finfo->fontId, finfo->CharSet);
	if (dFnt.Encod == 1 || dFnt.Encod == 2 || dFnt.Encod == 3) {
		MBF = TRUE;
	} else
		MBF = FALSE;
	return(TRUE);
#endif
#ifdef _WIN32
	LOGFONT t_lfDefFont;

	SetLogfont(&t_lfDefFont, NULL, finfo); 
	if (EnumFontFamiliesEx(GlobalDC->GetSafeHdc(),&t_lfDefFont,(FONTENUMPROC)EnumFontFamExProc,(LPARAM)finfo,0) != 0) {
		WToC(finfo->fontName, tempchar, UTTLINELEN+UTTLINELEN);
		if (EnumFontFamilies(GlobalDC->GetSafeHdc(),tempchar,(FONTENUMPROC)EnumFontFamProc,(LPARAM)finfo)) {
			mStr.sprintf(tempst2, "Windows font \"%ls\" is not found on this computer",finfo->fontName);
			freeAll(47);
		}
	}
	if (mStr.partwcmp(finfo->fontName,"Jpn ") || 
		mStr.partwcmp(finfo->fontName,"Chn ") ||
		mStr.strcmp(finfo->fontName, "DFPHKStdKai") == 0) {
		MBF = TRUE;
	} else
		MBF = FALSE;
	mStr.strcpy(dFnt.FName, finfo->fontName);
	dFnt.FSize = finfo->fontSize;
	dFnt.CharSet = finfo->CharSet;
	dFnt.Encod = my_FontToScript(finfo->fontName, finfo->CharSet);
	return(TRUE);
#endif /* _WIN32 */
}

static char chattoxml_SetNewFont(short *st, short ec) {
	NewFontInfo finfo;
	short fontName[256];
	short *fontPref;
	int  i;

	if (ec != EOS) {
		for (i=0; st[i] && st[i] != ':' && st[i] != ec; i++) ;
		if (st[i] != ':')
			return(FALSE);
		i++;
	} else
		i = 0;

	if (GetDatasFont(st+i, (char)ec, dFnt.Encod, TRUE, &finfo) == NULL)
		return(FALSE);

	if (finfo.fontType == MACcain || finfo.fontType == WIN95cain) 
		isCainFile = TRUE;
#if defined(_MAC_CODE)
	if (finfo.platform == MACDATA) {
		return(selectChoosenFont(&finfo));
	} else {
		mStr.strcpy(fontName, finfo.fontName);
		fontPref = finfo.fontPref;
		if (FindTTable(&finfo, MACDATA)) {
			if (finfo.fontTable == NULL) {
				return(selectChoosenFont(&finfo));
			} else {
				mStr.sprintf(tempst2, "Font \"%ls%ls\" suggests that data file \"%ls\" should be converted with \"makedata\" program first.",
					fontPref, fontName, oldfname);
				freeAll(47);
			}
		} else {
			mStr.sprintf(tempst2, "Font \"%ls%ls\" is not supported on this computer.", fontPref, fontName);
			freeAll(47);
		}
	}
#endif
#ifdef _WIN32
	if (finfo.platform == WIN95DATA) {
		return(selectChoosenFont(&finfo));
	} else {
		mStr.strcpy(fontName, finfo.fontName);
		fontPref = finfo.fontPref;
		if (FindTTable(&finfo, WIN95DATA)) {
			if (finfo.fontTable == NULL) {
				return(selectChoosenFont(&finfo));
			} else {
				mStr.sprintf(tempst2, "Font \"%ls%ls\" suggests that data file \"%ls\" should be converted with \"makedata\" program first.",
					fontPref, fontName, oldfname);
				freeAll(47);
			}
		} else {
			mStr.sprintf(tempst2, "Font \"%ls%ls\" is not supported on this computer.", fontPref, fontName);
			freeAll(47);
		}
	}
#endif /* _WIN32 */
	return(FALSE);
}
/*	 Find and set the current font of the tier end */
#endif // _IS_UNICODE

/* read chat data start */
static int mFeof(void) {
	return(*chatSt == 0);
}

static void flipShort(short *num) {
	char *s, t0, t1;

#if defined(_MAC_CODE)
	if (UEncoding == UMAC)
#endif
#if defined(_WIN32)
	if (UEncoding == UPC)
#endif
		return;
	s = (char *)num;
	t0 = s[0];
	t1 = s[1];
	s[1] = t0;
	s[0] = t1;
}

static short lgetc(short *att) {
	register short c;

rep:
	c = *chatSt++;
	flipShort(&c);
	if (c == '\n' && getc_cr_lc == '\r') {
		c = *chatSt++;
		flipShort(&c);
		if (c == 0)
			chatSt--;
	}
	getc_cr_lc = c;
	if (c == '\r') {
		tlineno++;
		return('\n');
	}
	if (c == (int)ATTMARKER) {
		c = *chatSt++;
		flipShort(&c);
		if (SetTextAtt(NULL, c, att, false))
			goto rep;
		else {
			freeAll(44);
		}
	}
	if (c == '\n')
		tlineno++;
	return(c);
}
/* read chat data end */

/* parse CHAT tiers data begin */
/* cutt_getline(ch, ccnt) reads in character by character until either end of file 
   or new tier has been reached. The characters are read into string pointed
   to by "ch". At the and "currentchar" is set to the first character of a
   new tier or to EOF marker. The character count is kept to make sure that
   the function stays within allocated memory limits.
*/
static void cutt_getline(short *ch, short *att, register int index) {
	register char sq;
	register int  lc;

	if (getrestline) {
		lc = index;
		ch[index] = currentchar;
		att[index] = currentatt;
		sq = false;
		index++;
		att[index] = att[index-1];
		while (1) {
			ch[index] = lgetc(&att[index]);
			if (mStr.uS.isRightChar(ch, index, '[', dFnt.Encod, MBF))
				sq = true;
			else if (iswalnum(ch[index]) || mStr.uS.isRightChar(ch, index, ']', dFnt.Encod, MBF))
				sq = false;
			if (isSpeaker(ch[index],isCainFile)) {
				if (ch[lc] == '\n')
					break;
			} else if (mFeof())
				break;

			lc = index;
			if (index >= UTTLINELEN) {
				mStr.sprintf(tempst2, "%d characters.\n",UTTLINELEN);
				freeAll(45);
			} else {
				index++;
				att[index] = att[index-1];
			}
		}
		currentchar = ch[index];
		currentatt = att[index];
		ch[index] = EOS;
	} else if (skipgetline == FALSE) {
		ch[index] = '\n';
		att[index] = 0;
		index++;
		ch[index] = EOS;
	} else
		skipgetline = FALSE;
	getrestline = 1;
}

static void SpeakerNameError(short *name, int i, int lim) {
	mStr.sprintf(tempst2, "%d characters.\nMaybe ':' character is missing or ';' character should be changed to ':'",lim);
	freeAll(46);
}

/* getspeaker(chrs) gets the code identifier part of the tier and stores it
   into memory pointed by "chrs". It returns 0 if end of file has been 
   reached, 1 otherwise. If the tier does not have a line part, i.e. @begin,
   or the end of file has been reached then the "getrestline" is set to 0.
   "currentchar" is set to the last character read. 
*/
static int getspeaker(short *sp, short *att, register int index) {
	register int orgIndex;
	register int i;
	register int j;

	if (mFeof()) {
		return(FALSE);
	}
	sp[index] = currentchar;
	att[index] = currentatt;
	orgIndex = index;
	if (isCainFile) {
		if (sp[index] != ':' && sp[index] != '@' && sp[index] != '%' && sp[index] != '\n' && !mFeof()) {
			sp[index+1] = sp[index];
			sp[index] = '*';
			index++;
			att[index] = att[index-1];
		}
	}
	if (!isCainFile || sp[index] != '(') {
		for (i=0; sp[index] != ':' && sp[index] != '\n' && !mFeof(); i++) {
			if (i >= SPEAKERLEN) {
				SpeakerNameError(sp+orgIndex, i, SPEAKERLEN);
				index--;
			}
			index++;
			att[index] = att[index-1];
			sp[index] = lgetc(&att[index]);
			if (sp[index] == '-')
				break;
		}
		j = 0;
		while (sp[index] != ':' && sp[index] != '\n' && !mFeof()) {
			j++;
			if (i+j >= SPEAKERLEN) {
				i += j;
				SpeakerNameError(sp+orgIndex, i, SPEAKERLEN);
				index--;
			}
			index++;
			att[index] = att[index-1];
			sp[index] = lgetc(&att[index]);
		}
		if (sp[orgIndex] == '*' || sp[orgIndex] == '%') {
			if (i > 4) {
				if (sp[index] == ':') {
					index++;
					att[index] = att[index-1];
					sp[index] = EOS;
				} else
					sp[index] = EOS;
				SpeakerNameError(sp+orgIndex, mStr.strlen(sp+orgIndex), 3);
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
			sp[index] = lgetc(&att[index]);
			while (sp[index] == ' ' || sp[index] == '\t') {
				i++;
				if (i >= SPEAKERLEN) {
					SpeakerNameError(sp+orgIndex, i, SPEAKERLEN);
					index--;
				}		
				index++;
				att[index] = att[index-1];
				sp[index] = lgetc(&att[index]);
			}
		} else {
			for (i=index; (isSpace(sp[i]) || sp[i] == '\n') && i >= 0; i--) ;
			if (i < index) {
				index = i + 1;
				sp[index] = '\n';
			}
		}
	} else {
		sp[index+1] = sp[index];
		sp[index] = ':';
		index++;
		att[index] = att[index-1];
	}

	if (mFeof())
		getrestline = 0;
	currentchar = sp[index];
	currentatt = att[index];
	sp[index] = EOS;
	return(TRUE);
}

static int getutterance() {
	lineno = lineno + tlineno;
	tlineno = 0L;
	if (!getspeaker(wutterance.speaker, wutterance.attSp, 0))
		return(false);
	if (mStr.partcmp(wutterance.speaker,CKEYWORDHEADER,false)) {
		tlineno--;
	} else if (mStr.partcmp(wutterance.speaker,FONTHEADER,false)) {
		cutt_getline(wutterance.line, wutterance.attLine, 0);
		skipgetline = 1;
		getrestline = 0;
#ifndef _IS_UNICODE
		if (chattoxml_SetNewFont(wutterance.line, EOS))
			tlineno--;
#endif
	}
	cutt_getline(wutterance.line, wutterance.attLine, 0);
#ifndef _IS_UNICODE
	if (mStr.partwcmp(wutterance.line, FONTMARKER))
		chattoxml_SetNewFont(wutterance.line,']');
#endif
	return(true);
}
/* parse CHAT tiers data end */

static struct overlap *addOverlap(int ovID, int num, char isDn, char isUp) {
	struct overlap *t;

	if (overlaps == NULL) {
		t = NEW(struct overlap);
		if (t == NULL) {
			freeAll(1);
		}
		t->NextOverlap = NULL;
		overlaps = t;
	} else {
		for (t=overlaps; t->NextOverlap != NULL; t=t->NextOverlap) ;
		t->NextOverlap = NEW(struct overlap);
		if (t->NextOverlap == NULL) {
			freeAll(1);
		}
		t = t->NextOverlap;
	}
	t->NextOverlap = NULL;
	t->cId = ovID;
	t->with = 0;
	t->num = num;
	t->isUp = isUp;
	t->isDn = isDn;
	if (isDn && cOverlap == NULL)
		cOverlap = t;
	return(t);
}

static void insertMId(struct overlap *des, struct overlap *src) {
	des->with = src->cId;
}

static void makeOVMatch(struct overlap *tOverlap) {
	struct overlap *p;

	if (cOverlap == NULL) {
		mStr.sprintf(tempst2, "\n%ls", wutterance.line);
		freeAll(3);
	}
	if (tOverlap == NULL) {
		for (p=cOverlap; p != NULL; p=p->NextOverlap) {
			if (p->with == 0) {
				for (tOverlap=cOverlap; tOverlap != NULL; tOverlap=tOverlap->NextOverlap) {
					if (p->num == tOverlap->num) {
						insertMId(p, tOverlap);
						break;
					}
				}
			}
		}
	} else {
		for (p=cOverlap; p != NULL; p=p->NextOverlap) {
			if (p != tOverlap) {
				if (p->num == tOverlap->num && p->with == 0) {
					insertMId(p, tOverlap);
					break;
				}
			}
		}
	}
}

static long getNum(short *line, long i, int *num) {	
	for (; isSpace(line[i]); i++) ;
	if (iswdigit(line[i])) {
		*num = mStr.atoi(line+i);
		for (; iswdigit(line[i]); i++) ;
		if (line[i] != ']')
			*num = 0;
	}
	return(i);
}

static void createOverlapMap(void) {
	int  ovID=1, num, cnt;
	int  trend;
	char ovFound;
	long i, o;
	struct overlap *tOverlap;

	cnt = 0;
	trend = UP;
	currentatt = 0;
	currentchar = lgetc(&currentatt);
	while (getutterance()) {
		if (wutterance.speaker[0] == '*') {
			num = 0;
			ovFound = false;
			for (i=0L; wutterance.line[i] != EOS; i++) {
				if (mStr.partcmp(wutterance.line+i,"[<]",false)) {
					ovFound = true;
					trend = UP;
					i += 2;
					tOverlap = addOverlap(ovID, num, false, true);
					makeOVMatch(tOverlap);
					ovID += 2;
/*
mStr.strcpy(tOverlap->ov_sp,   wutterance.speaker);
mStr.strcpy(tOverlap->ov_line, wutterance.line);
tOverlap->lineno = lineno;
*/
				} else if (mStr.partcmp(wutterance.line+i,"[>]",false)) {
					ovFound = true;
					if (trend == UP) {
						if (cOverlap != NULL)
							makeOVMatch(NULL);
						cOverlap = NULL;
					}
					trend = DN;
					i += 2;
					tOverlap = addOverlap(ovID, num, true, false);
					ovID += 2;
/*
mStr.strcpy(tOverlap->ov_sp,   wutterance.speaker);
mStr.strcpy(tOverlap->ov_line, wutterance.line);
tOverlap->lineno = lineno;
*/
				} else if (mStr.partcmp(wutterance.line+i,"[<>]",false)) {
					ovFound = true;
					i += 3;
					tOverlap = addOverlap(ovID, num, true, true);
					makeOVMatch(tOverlap);
					ovID += 2;
/*
mStr.strcpy(tOverlap->ov_sp,   wutterance.speaker);
mStr.strcpy(tOverlap->ov_line, wutterance.line);
tOverlap->lineno = lineno;
*/
				} else if (mStr.partcmp(wutterance.line+i,"[<",false)) {
					ovFound = true;
					trend = UP;
					o = i;
					i += 2;
					i = getNum(wutterance.line, i, &num);
					if (num < 1) {
						mStr.sprintf(tempst2, ":%ls\n%ls", wutterance.line+o, wutterance.line);
						freeAll(4);
					}
					tOverlap = addOverlap(ovID, num, false, true);
					makeOVMatch(tOverlap);
					ovID += 2;
/*
mStr.strcpy(tOverlap->ov_sp,   wutterance.speaker);
mStr.strcpy(tOverlap->ov_line, wutterance.line);
tOverlap->lineno = lineno;
*/
				} else if (mStr.partcmp(wutterance.line+i,"[>",false)) {
					ovFound = true;
					if (trend == UP) {
						if (cOverlap != NULL)
							makeOVMatch(NULL);
						cOverlap = NULL;
					}
					trend = DN;
					o = i;
					i += 2;
					i = getNum(wutterance.line, i, &num);
					if (num < 1) {
						mStr.sprintf(tempst2, ":%ls\n%ls", wutterance.line+o, wutterance.line);
						freeAll(4);
					}
					tOverlap = addOverlap(ovID, num, true, false);
					ovID += 2;
/*
mStr.strcpy(tOverlap->ov_sp,   wutterance.speaker);
mStr.strcpy(tOverlap->ov_line, wutterance.line);
tOverlap->lineno = lineno;
*/
				} else if (mStr.partcmp(wutterance.line+i,"[<>",false)) {
					mStr.sprintf(tempst2, ":%ls\n%ls", wutterance.line+i, wutterance.line);
					freeAll(5);
				}
			}
			if (ovFound)
				cnt = 0;
			else {
				if (cnt < 10)
					cnt++;
				if (cnt >= 6 && trend == DN) {
					mStr.sprintf(tempst2, "\nSomewhere before tier: %ls", wutterance.line);
					freeAll(6);
				}
			}
		}
	}
	if (cOverlap != NULL)
		makeOVMatch(NULL);
/*
for (tOverlap=overlaps; tOverlap != NULL; tOverlap=tOverlap->NextOverlap) {
	fprintf(stderr, "************ %ld\n", tOverlap->lineno);
	fprintf(stderr, "%ls%ls", tOverlap->ov_sp, tOverlap->ov_line);
	fprintf(stderr, "cId=%d, num=%d, isUp=%d, isDn=%d\n    with=", 
					tOverlap->cId, tOverlap->num, tOverlap->isUp, tOverlap->isDn);
	fprintf(stderr, "%d, ", tOverlap->with);
	fprintf(stderr, "\n");
}
*/
}

static void CHATtoXML_CleanTierNames(short *st) {
	register int i;

	i = mStr.strlen(st) - 1;
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
/*
static void convertToOrg(short *dest, short *src) {
	long di, si;

	si = mStr.strlen(src);
	di = si;
	while (si >= 0) {
		if (src[si] == ';') {
			if (!strncmp(src+si-4, "&amp;", 5)) {
				si -= 5;
				dest[di--] = '&';
			} else if (!strncmp(src+si-5, "&quot;", 6)) {
				si -= 6;
				dest[di--] = '"';
			} else if (!strncmp(src+si-5, "&apos;", 6)) {
				si -= 6;
				dest[di--] = '\'';
			} else if (!strncmp(src+si-3, "&lt;", 4)) {
				si -= 3;
				while (di > si)
					dest[di--] = ' ';
				dest[di--] = '<';
				si--;
			} else if (!strncmp(src+si-3, "&gt;", 4)) {
				dest[di--] = '>';
				si -= 3;
				while (di > si)
					dest[di--] = ' ';
				si--;
			} else
				dest[di--] = src[si--];
		} else if (mStr.uS.isskip(src, si, dFnt.Encod, true) || src[si] == '\n') {
			while (di > si)
				dest[di--] = ' ';
			dest[di--] = src[si--];
		} else
			dest[di--] = src[si--];
	}
	while (di > si)
		dest[di--] = ' ';
}
*/
static struct priolst *addToPriList(struct priolst *root, short prior, short *st, char isInsideWord, char isPad) {
	struct priolst *nt, *tnt, *t;

	if (root == NULL) {
		root = NEW(struct priolst);
		if (root == NULL)
			freeAll(1);
		nt = root;
		nt->next = NULL;
	} else {
		tnt= root;
		nt = root;
		while (1) {
			if (nt == NULL)
				break;
			else if (prior < nt->priority)
				break;
			tnt = nt;
			nt = nt->next;
		}
		if (nt == NULL) {
			t = tnt->next;
			tnt->next = NEW(struct priolst);
			if (tnt->next == NULL) {
				tnt->next = t;
				freePriList(root);
				freeAll(1);
			}
			nt = tnt->next;
			nt->next = NULL;
		} else if (nt == root) {
			root = NEW(struct priolst);
			if (root == NULL) {
				freePriList(nt);
				freeAll(1);
			}
			root->next = nt;
			nt = root;
		} else {
			nt = NEW(struct priolst);
			if (nt == NULL) {
				freePriList(root);
				freeAll(1);
			}
			nt->next = tnt->next;
			tnt->next = nt;
		}
	}
	
	nt->priority = prior;
	nt->st = (short *)malloc((mStr.strlen(st)+1)*2);
	if (nt->st == NULL) {
		freePriList(root);
		freeAll(1);
	}
	mStr.strcpy(nt->st, st);
	nt->isInsideWord = isInsideWord;
	nt->isUsed = FALSE;
	nt->isPad = isPad;
	return(root);
}

static struct tiers *addToHeaders(struct tiers *p, short *sp, short *line) {
	long i, j;
	struct tiers *u;
	
	if ((u=p) == NULL) {
		p = NEW(struct tiers);
		if (p == NULL) {
			freeAll(1);
		}
		u = p;
	} else {
		while (u->NextUtt != NULL) {
			if (mStr.strcmp(sp, u->sp) == 0 && !mStr.strcmp(sp, "ID")) {
				mStr.sprintf(tempst2, "\n%ls%ls", sp, u->line);
				freeAll(7);
			}
			u = u->NextUtt;
		}
		if (mStr.strcmp(sp, u->sp) == 0 && !mStr.strcmp(sp, "ID")) {
			mStr.sprintf(tempst2, "\n%ls%ls", sp, u->line);
			freeAll(7);
		}
		u->NextUtt = NEW(struct tiers);
		if (u->NextUtt == NULL) {
			freeAll(1);
		}
		u = u->NextUtt;
	}
	u->NextUtt = NULL;
	j = 0L;
	for (i=0L; line[i] != EOS; i++) {
		if (line[i] == '&') {
			mStr.strcpy(tempst3+j, "&amp;");
			j = mStr.strlen(tempst3);
		} else if (line[i] == '"') {
			mStr.strcpy(tempst3+j, "&quot;");
			j = mStr.strlen(tempst3);
		} else if (line[i] == '\'') {
			mStr.strcpy(tempst3+j, "&apos;");
			j = mStr.strlen(tempst3);
		} else if (line[i] == '<') {
			mStr.strcpy(tempst3+j, "&lt;");
			j = mStr.strlen(tempst3);
		} else if (line[i] == '>') {
			mStr.strcpy(tempst3+j, "&gt;");
			j = mStr.strlen(tempst3);
		} else
			tempst3[j++] = line[i];
	}
	tempst3[j] = EOS;
	mStr.strcpy(u->sp, sp);
	mStr.strcpy(u->orgLine, line);
	mStr.strcpy(u->line, tempst3);
	return(p);
}

static struct items *addToItems(struct items *p, short *word, struct items **w) {
	struct items *u;
	
	if ((u=p) == NULL) {
		p = NEW(struct items);
		if (p == NULL) {
			freeAll(1);
		}
		u = p;
		u->PrevItem = NULL;
	} else {
		while (u->NextItem != NULL)
			u = u->NextItem;
		u->NextItem = NEW(struct items);
		if (u->NextItem == NULL) {
			freeAll(1);
		}
		u->NextItem->PrevItem = u;
		u = u->NextItem;
	}
	mStr.strcpy(u->word, word);
	u->replacement = NULL;
	u->morWord = NULL;
	u->phoWord = NULL;
	u->modWord = NULL;
	u->addWord = NULL;
	u->codesCnt = 0;
	mStr.strcpy(u->lang, lLanguage);
	u->omited = false;
	u->morOmited = false;
	u->isChange = false;
	u->sndMark = 0;
	u->NextItem = NULL;
	if (w != NULL)
		*w = u;
	return(p);
}

static int isExcludeWord(short *word) {
	char isMatched;
	
	isMatched = false;
	if (mStr.patmat(word, "xxx"))
		isMatched = true;
	else if (mStr.patmat(word, "yyy"))
		isMatched = true;
	else if (mStr.patmat(word, "xx"))
		isMatched = true;
	else if (mStr.patmat(word, "yy"))
		isMatched = true;
	else if (mStr.patmat(word, "www"))
		isMatched = true;
	else if (mStr.patmat(word, "0*"))
		isMatched = true;
	else if (mStr.patmat(word, "&*"))
		isMatched = true;
	else if (mStr.patmat(word, "-*"))
		isMatched = true;
	else if (mStr.patmat(word, "#*"))
		isMatched = true;

	if (isMatched) {
		if (word[0] != '-')
			return(true);
		else if (mStr.uS.isToneUnitMarker(word))
			return(true);
	}
	return(false);
}

static char spSymbols(short *word, long j) {
	return(!(mStr.strncmp(word+j,"&amp;",5) && mStr.strncmp(word+j,"&quot;",6) &&
				mStr.strncmp(word+j,"&apos;",6) && 
				mStr.strncmp(word+j,"&lt;",4) && mStr.strncmp(word+j,"&gt;",4)));
}

static void addToSpeaker(short *sp, char *type, short *att) {
	short  tst[512];
	struct participants *u;

	if (att[0] == EOS)
		return;
	if (sp[0] == EOS) {
		freeAll(8);
	}
	CToW(type, tst, 512);
	if ((u=speakers) == NULL) {
		speakers = NEW(struct participants);
		if (speakers == NULL)  {
			freeAll(1);
		}
		u = speakers;
	} else {
		while (u->NextPart != NULL) {
			if (!mStr.strcmp(sp, u->sp)) {
				mStr.sprintf(u->line+mStr.strlen(u->line), " %ls=\"%ls\"", tst, att);
				return;
			}
			u = u->NextPart;
		}
		if (!mStr.strcmp(sp, u->sp)) {
			mStr.sprintf(u->line+mStr.strlen(u->line), " %ls=\"%ls\"", tst, att);
			return;
		}
		u->NextPart = NEW(struct participants);
		if (u->NextPart == NULL) {
			freeAll(1);
		}
		u = u->NextPart;
	}
	u->NextPart = NULL;
	mStr.strcpy(u->sp, sp);
	mStr.sprintf(u->line, " %ls=\"%ls\"", tst, att);
}

static short *CHATtoXML_isskip(short *word, short *line, long *j, char *isSkip) {
	long i;

	i = *j;
	*isSkip = false;
	if (line[i] == '&' && line[i+1] == 'l' && line[i+2] == 't' && line[i+3] == ';') {
		*isSkip = true;
	} else if (line[i] == '&' && line[i+1] == 'g' && line[i+2] == 't' && line[i+3] == ';') {
		*isSkip = true;
	} else if (line[i] == '&' && line[i+1] == 'a' && line[i+2] == 'm' && line[i+3] == 'p' && line[i+4] == ';') {
		*++word=line[++i]; *++word=line[++i]; *++word=line[++i]; *++word=line[++i]; *++word=line[++i];
		*j = i;
	} else if (line[i] == '&' && line[i+1] == 'a' && line[i+2] == 'p' && line[i+3] == 'o' && line[i+4] == 's' && line[i+5] == ';') {
		*++word=line[++i]; *++word=line[++i]; *++word=line[++i]; *++word=line[++i]; *++word=line[++i]; *++word=line[++i];
		*j = i;
	} else if (line[i] == '&' && line[i+1] == 'q' && line[i+2] == 'u' && line[i+3] == 'o' && line[i+4] == 't' && line[i+5] == ';') {
		*++word=line[++i]; *++word=line[++i]; *++word=line[++i]; *++word=line[++i]; *++word=line[++i]; *++word=line[++i];
		*j = i;
	}
	if (*isSkip == false)
		*isSkip = mStr.uS.isskip(line, i, dFnt.Encod, MBF);
	return(word);
}

static char isThisScoping(short *line, long i) {
	if (line[i] != '&' || line[i+1] != 'l' || line[i+2] != 't' || line[i+3] != ';')
		return(false);
	for (; line[i] != ';'; i++) ;
	for (i++; line[i] != EOS; i++) {
		if (line[i] != '-' && !isSpace(line[i]) && !iswdigit(line[i]))
			break;
	}
	if (line[i] == '&' && line[i+1] == 'g' && line[i+2] == 't' && line[i+3] == ';')
		return(true);
	else
		return(false);
}

static long getScopedItem(register short *word, short *line, long i) {
	register char bl;

	while ((*word=line[i]) != EOS && (isSpace(line[i]) || line[i] == '\n'))
		 i++;
	if (*word == EOS)
		return(0);
	bl = (*word == HIDEN_C);
	if (isThisScoping(line, i)) {
		while ((*++word=line[++i]) != ';') ;
		while ((*++word=line[++i]) != '&') ;
		while ((*++word=line[++i]) != ';') ;
		word++;
		i++;
	} else {
		while ((*++word=line[++i]) != EOS) {
			if (*word == '&' && line[i+1] == 'l' && line[i+2] == 't' && line[i+3] == ';')
				break;
			if (line[i] == HIDEN_C) {
				*++word = EOS;
				return(i+1);
			}
			if (*word == '.' || *word == '!' || *word == '?')
				word--;
		}
	}
	*word = EOS;
	return(i);
}

static long getitem(register short *word, short *line, long i, int tier) {
	register char isSkip;
	register char sq;
	register char bl;

	while ((*word=line[i]) != EOS && (isSpace(line[i]) || line[i] == '\n'))
		 i++;
	if (*word == EOS)
		return(0);
	if (mStr.uS.isRightChar(line, i, '[',dFnt.Encod,MBF)) {
		if (mStr.uS.isSqBracketItem(line, i+1, dFnt.Encod, MBF))
			sq = true;
		else 
			sq = false;
	} else
		sq = false;
	bl = (*word == HIDEN_C);
	if (tier == MORTIER &&
			mStr.uS.isRightChar(line,i,'?',dFnt.Encod,MBF) && mStr.uS.isRightChar(line,i+1,'|',dFnt.Encod,MBF)) {
		while ((*++word=line[++i])!=EOS && !isSpace(*word) && *word!='\n') {
			if (mStr.uS.isRightChar(line, i, ']', dFnt.Encod, MBF)) { *++word = EOS; return(i+1); }
			if (mStr.uS.isRightChar(line, i, '[', dFnt.Encod, MBF)) { *word = EOS; return(i); }
		}
	} else if ((mStr.uS.isskip(word,0,dFnt.Encod,MBF) || mStr.uS.isRightChar(word,0,',',dFnt.Encod,MBF)) && !sq) {
		while ((*++word=line[++i])!=EOS && mStr.uS.isskip(line,i,dFnt.Encod,MBF) && !isSpace(*word) && *word!='\n') {
			if (mStr.uS.isRightChar(line, i, ']', dFnt.Encod, MBF)) { *++word = EOS; return(i+1); }
			if (mStr.uS.isRightChar(line, i, '[', dFnt.Encod, MBF)) { *word = EOS; return(i); }
		}
	} else if (mStr.uS.isRightChar(word,0,'-',dFnt.Encod,MBF) && !sq) {
		while ((*++word=line[++i]) != EOS && !isSpace(line[i]) && line[i] != '\n') {
			if (line[i] == '&' && line[i+1] == 'a' && line[i+2] == 'p' && 
					line[i+3] == 'o' && line[i+4] == 's' && line[i+5] == ';') {
				*++word = 'a'; *++word = 'p'; *++word = 'o'; *++word = 's'; *++word = ';';
				i += 5;
			} else if (iswalnum(line[i]))
				break;
			if (mStr.uS.isRightChar(line, i, ']', dFnt.Encod, MBF)) { *++word = EOS; return(i+1); }
			if (mStr.uS.isRightChar(line, i, '[', dFnt.Encod, MBF)) { *word = EOS; return(i); }
		}
	} else if (mStr.uS.isRightChar(word,0,'+',dFnt.Encod,MBF) && !sq) {
		while ((*++word=line[++i]) != EOS && !isSpace(line[i]) && line[i] != '\n') {
			if (line[i] == '&' && line[i+1] == 'l' && line[i+2] == 't' && line[i+3] == ';') {
				*++word = 'l'; *++word = 't'; *++word = ';';
				i += 3;
			} else if (line[i] == '&' && line[i+1] == 'q' && line[i+2] == 'u' && 
					line[i+3] == 'o' && line[i+4] == 't' && line[i+5] == ';') {
				*++word = 'q'; *++word = 'u'; *++word = 'o'; *++word = 't'; *++word = ';';
				i += 5;
			} else if (iswalnum(line[i]))
				break;
			if (mStr.uS.isRightChar(line, i, ']', dFnt.Encod, MBF)) { *++word = EOS; return(i+1); }
			if (mStr.uS.isRightChar(line, i, '[', dFnt.Encod, MBF)) { *word = EOS; return(i); }
		}
	} else if (*word == '&' && (line[i+1] == 'l' || line[i+1] == 'g') && line[i+2] == 't' && line[i+3] == ';') {
		if ((tier == PHOTIER || tier == MODTIER || tier == ADDTIER) && 
						line[i+1] == 'l' && isThisScoping(line, i)) {
			while ((*++word=line[++i]) != ';') ;
			while ((*++word=line[++i]) != '&') ;
			while ((*++word=line[++i]) != ';') ;
			word++;
			i++;
		} else {
			while ((*++word=line[++i]) != ';') ;
			word++;
			i++;
		}
	} else {
		word = CHATtoXML_isskip(word, line, &i, &isSkip);
		while ((*++word=line[++i]) != EOS) {
			word = CHATtoXML_isskip(word, line, &i, &isSkip);
			if (mStr.uS.isRightChar(line, i, ']', dFnt.Encod, MBF)) {
				*++word = EOS;
				return(i+1);
			}
			if (isSkip && !sq && !bl)
				break;
			if (*word == EOS)
				break;
			if (line[i] == HIDEN_C) {
				if (!bl)
					break;
				else {
					*++word = EOS;
					return(i+1);
				}
			}
		}
	}
	*word = EOS;
	return(i);
}

static void uS.extractString(short *sp, short *line, char *type, char endC) {
	int i, j;

	for (i=mStr.strlen(type); isSpace(line[i]); i++) ;
	for (j=0; line[i] != endC && line[i] != EOS; i++, j++)
		sp[j] = line[i];
	sp[j] = EOS;
	mStr.uS.remblanks(sp);
}

#define isQuote(x, i) (x[i] == '&' && x[i+1] == 'q' && x[i+2] == 'u' && x[i+3] == 'o' && x[i+4] == 't' && x[i+5] == ';')
static void writeMediaInfo(short *st) {
	register int i;
	register int j;
	char *mdtype;
	double t;

	for (i=1, j=0; st[i] && st[i] != ':'; i++, j++) {
		tempst1[j] = st[i];
	}
	tempst1[j] = EOS;
	if (st[i] != ':') {
		xml_output(" unknown_type=\"", NULL);
		xml_output(NULL, st);
		xml_output("\"", NULL);
		return;
	}
	i++;
	tempst3[0] = EOS;
	mStr.sprintf(tempst3, "<media");
	if (!mStr.strcmp(tempst1, "%snd"))
		mdtype = " type=\"audio\"";
	else if (!mStr.strcmp(tempst1, "%mov"))
		mdtype = " type=\"video\"";
	else if (!mStr.strcmp(tempst1, "%pic"))
		mdtype = " type=\"image\"";
	else if (!mStr.strcmp(tempst1, "%txt"))
		mdtype = " type=\"text\"";
	else {
		xml_output(" unknown_type=\"", NULL);
		xml_output(NULL, st);
		xml_output("\"", NULL);
		return;
	}
	for (; st[i] && (isSpace(st[i]) || st[i] == '_'); i++) ;
	if (!isQuote(st, i)) {
		xml_output(" unknown_type=\"", NULL);
		xml_output(NULL, st);
		xml_output("\"", NULL);
		return;
	}
	i += 6;
	if (st[i] == EOS) {
		xml_output(" unknown_type=\"", NULL);
		xml_output(NULL, st);
		xml_output("\"", NULL);
		return;
	}
	for (j=0; st[i] && !isQuote(st, i); i++)
		tempst1[j++] = st[i];
	tempst1[j] = EOS;
	if (st[i] == EOS) {
		xml_output(" unknown_type=\"", NULL);
		xml_output(NULL, st);
		xml_output("\"", NULL);
		return;
	}
	mStr.sprintf(tempst3+mStr.strlen(tempst3), " href=\"%ls\"", tempst1);
	mStr.strcat(tempst3, mdtype);
	for (; st[i] && !iswdigit(st[i]); i++) ;
	if (st[i] == EOS) {
		xml_output(" unknown_type=\"", NULL);
		xml_output(NULL, st);
		xml_output("\"", NULL);
		return;
	}
	for (j=0; st[i] && iswdigit(st[i]); i++)
		tempst1[j++] = st[i];
	tempst1[j] = EOS;
	t = (double)mStr.atol(tempst1);
	mStr.sprintf(tempst3+mStr.strlen(tempst3), " start=\"%.3lf\"", t/1000.000);

	for (; st[i] && !iswdigit(st[i]); i++) ;
	if (st[i] == EOS) {
		xml_output(" unknown_type=\"", NULL);
		xml_output(NULL, st);
		xml_output("\"", NULL);
		return;
	}
	for (j=0; st[i] && iswdigit(st[i]); i++)
		tempst1[j++] = st[i];
	tempst1[j] = EOS;
	t = (double)mStr.atol(tempst1);
	mStr.sprintf(tempst3+mStr.strlen(tempst3), " end=\"%.3lf\"", t/1000.000);
	xml_output(NULL, tempst3);
	xml_output("/>\n", NULL);
}

static short *convertName(short *st) {
	if (mStr.strlen(st) == 2)
		return(st);
	if (!mStr.strcmp(st, "ABKHAZIAN")) return((short *)L"AB");
	if (!mStr.strcmp(st, "AFAN") || !mStr.strcmp(st, "OROMO")) return((short *)L"OM");
	if (!mStr.strcmp(st, "AFAR")) return((short *)L"AA");
	if (!mStr.strcmp(st, "AFRIKAANS")) return((short *)L"AF");
	if (!mStr.strcmp(st, "ALBANIAN")) return((short *)L"SQ");
	if (!mStr.strcmp(st, "AMHARIC")) return((short *)L"AM");
	if (!mStr.strcmp(st, "ARABIC")) return((short *)L"AR");
	if (!mStr.strcmp(st, "ARMENIAN")) return((short *)L"HY");
	if (!mStr.strcmp(st, "ASSAMESE")) return((short *)L"AS");
	if (!mStr.strcmp(st, "AYMARA")) return((short *)L"AY");
	if (!mStr.strcmp(st, "AZERBAIJANI")) return((short *)L"AZ");
	if (!mStr.strcmp(st, "BASHKIR")) return((short *)L"BA");
	if (!mStr.strcmp(st, "BASQUE")) return((short *)L"EU");
	if (!mStr.strcmp(st, "BENGALI") || !mStr.strcmp(st, "BANGLA")) return((short *)L"BN");
	if (!mStr.strcmp(st, "BHUTANI")) return((short *)L"DZ");
	if (!mStr.strcmp(st, "BIHARI")) return((short *)L"BH");
	if (!mStr.strcmp(st, "BISLAMA")) return((short *)L"BI");
	if (!mStr.strcmp(st, "BRETON")) return((short *)L"BR");
	if (!mStr.strcmp(st, "BULGARIAN")) return((short *)L"BG");
	if (!mStr.strcmp(st, "BURMESE")) return((short *)L"MY");
	if (!mStr.strcmp(st, "BYELORUSSIAN")) return((short *)L"BE");
	if (!mStr.strcmp(st, "CAMBODIAN")) return((short *)L"KM");
	if (!mStr.strcmp(st, "CATALAN")) return((short *)L"CA");
	if (!mStr.strcmp(st, "CHINESE")) return((short *)L"ZH");
	if (!mStr.strcmp(st, "CORSICAN")) return((short *)L"CO");
	if (!mStr.strcmp(st, "CROATIAN")) return((short *)L"HR");
	if (!mStr.strcmp(st, "CZECH")) return((short *)L"CS");
	if (!mStr.strcmp(st, "DANISH")) return((short *)L"DA");
	if (!mStr.strcmp(st, "DUTCH")) return((short *)L"NL");
	if (!mStr.strcmp(st, "ENGLISH")) return((short *)L"EN");
	if (!mStr.strcmp(st, "ESPERANTO")) return((short *)L"EO");
	if (!mStr.strcmp(st, "ESTONIAN")) return((short *)L"ET");
	if (!mStr.strcmp(st, "FAROESE")) return((short *)L"FO");
	if (!mStr.strcmp(st, "FIJI")) return((short *)L"FJ");
	if (!mStr.strcmp(st, "FINNISH")) return((short *)L"FI");
	if (!mStr.strcmp(st, "FRENCH")) return((short *)L"FR");
	if (!mStr.strcmp(st, "FRISIAN")) return((short *)L"FY");
	if (!mStr.strcmp(st, "GALICIAN")) return((short *)L"GL");
	if (!mStr.strcmp(st, "GEORGIAN")) return((short *)L"KA");
	if (!mStr.strcmp(st, "GERMAN")) return((short *)L"DE");
	if (!mStr.strcmp(st, "GREEK")) return((short *)L"EL");
	if (!mStr.strcmp(st, "GREENLANDIC")) return((short *)L"KL");
	if (!mStr.strcmp(st, "GUARANI")) return((short *)L"GN");
	if (!mStr.strcmp(st, "GUJARATI")) return((short *)L"GU");
	if (!mStr.strcmp(st, "HAUSA")) return((short *)L"HA");
	if (!mStr.strcmp(st, "HEBREW")) return((short *)L"HE");
	if (!mStr.strcmp(st, "HINDI")) return((short *)L"HI");
	if (!mStr.strcmp(st, "ICELANDIC")) return((short *)L"IS");
	if (!mStr.strcmp(st, "INDONESIAN")) return((short *)L"ID");
	if (!mStr.strcmp(st, "INTERLINGUA")) return((short *)L"IA");
	if (!mStr.strcmp(st, "INTERLINGUE")) return((short *)L"IE");
	if (!mStr.strcmp(st, "INUKTITUT")) return((short *)L"IU");
	if (!mStr.strcmp(st, "INUPIAK")) return((short *)L"IK");
	if (!mStr.strcmp(st, "IRISH")) return((short *)L"GA");
	if (!mStr.strcmp(st, "ITALIAN")) return((short *)L"IT");
	if (!mStr.strcmp(st, "JAPANESE")) return((short *)L"JA");
	if (!mStr.strcmp(st, "JAVANESE")) return((short *)L"JV");
	if (!mStr.strcmp(st, "KANNADA")) return((short *)L"KN");
	if (!mStr.strcmp(st, "KASHMIRI")) return((short *)L"KS");
	if (!mStr.strcmp(st, "KAZAKH")) return((short *)L"KK");
	if (!mStr.strcmp(st, "KINYARWANDA")) return((short *)L"RW");
	if (!mStr.strcmp(st, "KIRGHIZ")) return((short *)L"KY");
	if (!mStr.strcmp(st, "KURUNDI")) return((short *)L"RN");
	if (!mStr.strcmp(st, "KOREAN")) return((short *)L"KO");
	if (!mStr.strcmp(st, "KURDISH")) return((short *)L"KU");
	if (!mStr.strcmp(st, "LAOTHIAN")) return((short *)L"LO");
	if (!mStr.strcmp(st, "LATIN")) return((short *)L"LA");
	if (!mStr.strcmp(st, "LATVIAN") || !mStr.strcmp(st, "LETTISH")) return((short *)L"LV");
	if (!mStr.strcmp(st, "LINGALA")) return((short *)L"LN");
	if (!mStr.strcmp(st, "LITHUANIAN")) return((short *)L"LT");
	if (!mStr.strcmp(st, "MACEDONIAN")) return((short *)L"MK");
	if (!mStr.strcmp(st, "MALAGASY")) return((short *)L"MG");
	if (!mStr.strcmp(st, "MALAY")) return((short *)L"MS");
	if (!mStr.strcmp(st, "MALAYALAM")) return((short *)L"ML");
	if (!mStr.strcmp(st, "MALTESE")) return((short *)L"MT");
	if (!mStr.strcmp(st, "MAORI")) return((short *)L"MI");
	if (!mStr.strcmp(st, "MARATHI")) return((short *)L"MR");
	if (!mStr.strcmp(st, "MOLDAVIAN")) return((short *)L"MO");
	if (!mStr.strcmp(st, "MONGOLIAN")) return((short *)L"MN");
	if (!mStr.strcmp(st, "NAURU")) return((short *)L"NA");
	if (!mStr.strcmp(st, "NEPALI")) return((short *)L"NE");
	if (!mStr.strcmp(st, "NORWEGIAN")) return((short *)L"NO");
	if (!mStr.strcmp(st, "OCCITAN")) return((short *)L"OC");
	if (!mStr.strcmp(st, "ORIYA")) return((short *)L"OR");
	if (!mStr.strcmp(st, "PASHTO") || !mStr.strcmp(st, "PUSHTO")) return((short *)L"PS");
	if (!mStr.strcmp(st, "PERSIAN")) return((short *)L"FA");
	if (!mStr.strcmp(st, "POLISH")) return((short *)L"PL");
	if (!mStr.strcmp(st, "PORTUGUESE")) return((short *)L"PT");
	if (!mStr.strcmp(st, "PUNJABI")) return((short *)L"PA");
	if (!mStr.strcmp(st, "QUECHUA")) return((short *)L"QU");
	if (!mStr.strcmp(st, "RHAETO-ROMANCE")) return((short *)L"RM");
	if (!mStr.strcmp(st, "ROMANIAN")) return((short *)L"RO");
	if (!mStr.strcmp(st, "RUSSIAN")) return((short *)L"RU");
	if (!mStr.strcmp(st, "SAMOAN")) return((short *)L"SM");
	if (!mStr.strcmp(st, "SANGHO")) return((short *)L"SG");
	if (!mStr.strcmp(st, "SANSKRIT")) return((short *)L"SA");
	if (!mStr.strcmp(st, "SCOTS GAELIC")) return((short *)L"GD");
	if (!mStr.strcmp(st, "SERBIAN")) return((short *)L"SR");
	if (!mStr.strcmp(st, "SERBO-CROATIAN")) return((short *)L"SH");
	if (!mStr.strcmp(st, "SESOTHO")) return((short *)L"ST");
	if (!mStr.strcmp(st, "SETSWANA")) return((short *)L"TN");
	if (!mStr.strcmp(st, "SHONA")) return((short *)L"SN");
	if (!mStr.strcmp(st, "SINDHI")) return((short *)L"SD");
	if (!mStr.strcmp(st, "SINGHALESE")) return((short *)L"SI");
	if (!mStr.strcmp(st, "SISWATI")) return((short *)L"SS");
	if (!mStr.strcmp(st, "SLOVAK")) return((short *)L"SK");
	if (!mStr.strcmp(st, "SLOVENIAN")) return((short *)L"SL");
	if (!mStr.strcmp(st, "SOMALI")) return((short *)L"SO");
	if (!mStr.strcmp(st, "SPANISH")) return((short *)L"ES");
	if (!mStr.strcmp(st, "SUNDANESE")) return((short *)L"SU");
	if (!mStr.strcmp(st, "SWAHILI")) return((short *)L"SW");
	if (!mStr.strcmp(st, "SWEDISH")) return((short *)L"SV");
	if (!mStr.strcmp(st, "TAGALOG")) return((short *)L"TL");
	if (!mStr.strcmp(st, "TAJIK")) return((short *)L"TG");
	if (!mStr.strcmp(st, "TAMIL")) return((short *)L"TA");
	if (!mStr.strcmp(st, "TATAR")) return((short *)L"TT");
	if (!mStr.strcmp(st, "TELUGU")) return((short *)L"TE");
	if (!mStr.strcmp(st, "THAI")) return((short *)L"TH");
	if (!mStr.strcmp(st, "TIBETAN")) return((short *)L"BO");
	if (!mStr.strcmp(st, "TIGRINYA")) return((short *)L"TI");
	if (!mStr.strcmp(st, "TONGA")) return((short *)L"TO");
	if (!mStr.strcmp(st, "TSONGA")) return((short *)L"TS");
	if (!mStr.strcmp(st, "TURKISH")) return((short *)L"TR");
	if (!mStr.strcmp(st, "TURKMEN")) return((short *)L"TK");
	if (!mStr.strcmp(st, "TWI")) return((short *)L"TW");
	if (!mStr.strcmp(st, "UIGUR")) return((short *)L"UG");
	if (!mStr.strcmp(st, "UKRAINIAN")) return((short *)L"UK");
	if (!mStr.strcmp(st, "URDU")) return((short *)L"UR");
	if (!mStr.strcmp(st, "UZBEK")) return((short *)L"UZ");
	if (!mStr.strcmp(st, "VIETNAMESE")) return((short *)L"VI");
	if (!mStr.strcmp(st, "VOLAPUK")) return((short *)L"VO");
	if (!mStr.strcmp(st, "WELSH")) return((short *)L"CY");
	if (!mStr.strcmp(st, "WOLOF")) return((short *)L"WO");
	if (!mStr.strcmp(st, "XHOSA")) return((short *)L"XH");
	if (!mStr.strcmp(st, "YIDDISH")) return((short *)L"YI");
	if (!mStr.strcmp(st, "YORUBA")) return((short *)L"YO");
	if (!mStr.strcmp(st, "ZHUANG")) return((short *)L"ZA");
	if (!mStr.strcmp(st, "ZULU")) return((short *)L"ZU");
	if (!mStr.strcmp(st, "IPAPHON")) return((short *)L"IPA");
	return((short *)L"");
}

static void rightLanguage(short *lang) {
	mStr.strcpy(tempst1, lang);
	mStr.uS.uppercasestr(tempst1, dFnt.Encod, MBF);
	if (mStr.strcmp(tempst1, "AB") &&
		mStr.strcmp(tempst1, "OM") &&
		mStr.strcmp(tempst1, "AA") &&
		mStr.strcmp(tempst1, "AF") &&
		mStr.strcmp(tempst1, "SQ") &&
		mStr.strcmp(tempst1, "AM") &&
		mStr.strcmp(tempst1, "AR") &&
		mStr.strcmp(tempst1, "HY") &&
		mStr.strcmp(tempst1, "AS") &&
		mStr.strcmp(tempst1, "AY") &&
		mStr.strcmp(tempst1, "AZ") &&
		mStr.strcmp(tempst1, "BA") &&
		mStr.strcmp(tempst1, "EU") &&
		mStr.strcmp(tempst1, "BN") &&
		mStr.strcmp(tempst1, "DZ") &&
		mStr.strcmp(tempst1, "BH") &&
		mStr.strcmp(tempst1, "BI") &&
		mStr.strcmp(tempst1, "BR") &&
		mStr.strcmp(tempst1, "BG") &&
		mStr.strcmp(tempst1, "MY") &&
		mStr.strcmp(tempst1, "BE") &&
		mStr.strcmp(tempst1, "KM") &&
		mStr.strcmp(tempst1, "CA") &&
		mStr.strcmp(tempst1, "ZH") &&
		mStr.strcmp(tempst1, "CO") &&
		mStr.strcmp(tempst1, "HR") &&
		mStr.strcmp(tempst1, "CS") &&
		mStr.strcmp(tempst1, "DA") &&
		mStr.strcmp(tempst1, "NL") &&
		mStr.strcmp(tempst1, "EN") &&
		mStr.strcmp(tempst1, "EO") &&
		mStr.strcmp(tempst1, "ET") &&
		mStr.strcmp(tempst1, "FO") &&
		mStr.strcmp(tempst1, "FJ") &&
		mStr.strcmp(tempst1, "FI") &&
		mStr.strcmp(tempst1, "FR") &&
		mStr.strcmp(tempst1, "FY") &&
		mStr.strcmp(tempst1, "GL") &&
		mStr.strcmp(tempst1, "KA") &&
		mStr.strcmp(tempst1, "DE") &&
		mStr.strcmp(tempst1, "EL") &&
		mStr.strcmp(tempst1, "KL") &&
		mStr.strcmp(tempst1, "GN") &&
		mStr.strcmp(tempst1, "GU") &&
		mStr.strcmp(tempst1, "HA") &&
		mStr.strcmp(tempst1, "HE") &&
		mStr.strcmp(tempst1, "HI") &&
		mStr.strcmp(tempst1, "IS") &&
		mStr.strcmp(tempst1, "ID") &&
		mStr.strcmp(tempst1, "IA") &&
		mStr.strcmp(tempst1, "IE") &&
		mStr.strcmp(tempst1, "IU") &&
		mStr.strcmp(tempst1, "IK") &&
		mStr.strcmp(tempst1, "GA") &&
		mStr.strcmp(tempst1, "IT") &&
		mStr.strcmp(tempst1, "JA") &&
		mStr.strcmp(tempst1, "JV") &&
		mStr.strcmp(tempst1, "KN") &&
		mStr.strcmp(tempst1, "KS") &&
		mStr.strcmp(tempst1, "KK") &&
		mStr.strcmp(tempst1, "RW") &&
		mStr.strcmp(tempst1, "KY") &&
		mStr.strcmp(tempst1, "RN") &&
		mStr.strcmp(tempst1, "KO") &&
		mStr.strcmp(tempst1, "KU") &&
		mStr.strcmp(tempst1, "LO") &&
		mStr.strcmp(tempst1, "LA") &&
		mStr.strcmp(tempst1, "LV") &&
		mStr.strcmp(tempst1, "LN") &&
		mStr.strcmp(tempst1, "LT") &&
		mStr.strcmp(tempst1, "MK") &&
		mStr.strcmp(tempst1, "MG") &&
		mStr.strcmp(tempst1, "MS") &&
		mStr.strcmp(tempst1, "ML") &&
		mStr.strcmp(tempst1, "MT") &&
		mStr.strcmp(tempst1, "MI") &&
		mStr.strcmp(tempst1, "MR") &&
		mStr.strcmp(tempst1, "MO") &&
		mStr.strcmp(tempst1, "MN") &&
		mStr.strcmp(tempst1, "NA") &&
		mStr.strcmp(tempst1, "NE") &&
		mStr.strcmp(tempst1, "NO") &&
		mStr.strcmp(tempst1, "OC") &&
		mStr.strcmp(tempst1, "OR") &&
		mStr.strcmp(tempst1, "PS") &&
		mStr.strcmp(tempst1, "FA") &&
		mStr.strcmp(tempst1, "PL") &&
		mStr.strcmp(tempst1, "PT") &&
		mStr.strcmp(tempst1, "PA") &&
		mStr.strcmp(tempst1, "QU") &&
		mStr.strcmp(tempst1, "RM") &&
		mStr.strcmp(tempst1, "RO") &&
		mStr.strcmp(tempst1, "RU") &&
		mStr.strcmp(tempst1, "SM") &&
		mStr.strcmp(tempst1, "SG") &&
		mStr.strcmp(tempst1, "SA") &&
		mStr.strcmp(tempst1, "GD") &&
		mStr.strcmp(tempst1, "SR") &&
		mStr.strcmp(tempst1, "SH") &&
		mStr.strcmp(tempst1, "ST") &&
		mStr.strcmp(tempst1, "TN") &&
		mStr.strcmp(tempst1, "SN") &&
		mStr.strcmp(tempst1, "SD") &&
		mStr.strcmp(tempst1, "SI") &&
		mStr.strcmp(tempst1, "SS") &&
		mStr.strcmp(tempst1, "SK") &&
		mStr.strcmp(tempst1, "SL") &&
		mStr.strcmp(tempst1, "SO") &&
		mStr.strcmp(tempst1, "ES") &&
		mStr.strcmp(tempst1, "SU") &&
		mStr.strcmp(tempst1, "SW") &&
		mStr.strcmp(tempst1, "SV") &&
		mStr.strcmp(tempst1, "TL") &&
		mStr.strcmp(tempst1, "TG") &&
		mStr.strcmp(tempst1, "TA") &&
		mStr.strcmp(tempst1, "TT") &&
		mStr.strcmp(tempst1, "TE") &&
		mStr.strcmp(tempst1, "TH") &&
		mStr.strcmp(tempst1, "BO") &&
		mStr.strcmp(tempst1, "TI") &&
		mStr.strcmp(tempst1, "TO") &&
		mStr.strcmp(tempst1, "TS") &&
		mStr.strcmp(tempst1, "TR") &&
		mStr.strcmp(tempst1, "TK") &&
		mStr.strcmp(tempst1, "TW") &&
		mStr.strcmp(tempst1, "UG") &&
		mStr.strcmp(tempst1, "UK") &&
		mStr.strcmp(tempst1, "UR") &&
		mStr.strcmp(tempst1, "UZ") &&
		mStr.strcmp(tempst1, "VI") &&
		mStr.strcmp(tempst1, "VO") &&
		mStr.strcmp(tempst1, "CY") &&
		mStr.strcmp(tempst1, "WO") &&
		mStr.strcmp(tempst1, "XH") &&
		mStr.strcmp(tempst1, "YI") &&
		mStr.strcmp(tempst1, "YO") &&
		mStr.strcmp(tempst1, "ZA") &&
		mStr.strcmp(tempst1, "ZU") ) {

		mStr.sprintf(tempst2, ":%ls", tempst1);
		freeAll(9);
	}
}

static void makeLanguage(short *oSt, char isLang) {
	short res[5], *t;

	mStr.strcpy(tempst2, oSt);
	mStr.uS.uppercasestr(tempst2, dFnt.Encod, MBF);
	if (isLang) {
		mStr.strcpy(res, convertName(tempst2));
		if (res[0] != EOS) {
			mStr.strcpy(lLanguage, res);
			if (gLanguage[0] == EOS && mStr.strcmp(res, "IPA"))
				mStr.strcpy(gLanguage, res);
		} else {
			mStr.sprintf(tempst2, ":%ls", oSt);
			freeAll(10);
		}
	} else {
		t = mStr.strchr(tempst2, ':');
		if (t != NULL) {
			*t = EOS;
			if (!mStr.strcmp(tempst2, "WIN95")) {
				mStr.strcpy(tempst2, t+1);
				t = mStr.strchr(tempst2, ':');
			}
		}
		if (t != NULL) {
			*t = EOS;
			mStr.strcpy(res, convertName(tempst2));
			if (res[0] != EOS) {
				mStr.strcpy(lLanguage, res);
				if (gLanguage[0] == EOS && mStr.strcmp(res, "IPA"))
					mStr.strcpy(gLanguage, res);
			} else {
				mStr.strcpy(lLanguage, gLanguage);
			}
		} else {
			mStr.sprintf(tempst2, ":%ls", oSt);
			freeAll(11);
		}
	}
}

static void changeGroupIndent(short *groupIndent, int d) {
	int i;

	if (d > 0)
		mStr.strcat(groupIndent, "    ");
	else {
		i = mStr.strlen(groupIndent) - 4;
		if (i < 0)
			i = 0;
		groupIndent[i] = EOS;
	}
}

static char isTerminator(short *st, short *terminatorType, short *groupIndent) {
	if (!mStr.strcmp(st, ".")) {
		if (terminatorType != NULL) {
			if (terminatorType[0] != EOS) {
				if (spTier != NULL)
					mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
				else
					tempst2[0] = EOS;
				freeAll(12);
			}
			mStr.strcpy(terminatorType, "<t type=\"period\"/>");
		}
		return(true);
	} else if (!mStr.strcmp(st, "?")) {
		if (terminatorType != NULL) {
			if (terminatorType[0] != EOS) {
				if (spTier != NULL)
					mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
				else
					tempst2[0] = EOS;
				freeAll(12);
			}
			mStr.strcpy(terminatorType, "<t type=\"question\"/>");
		}
		return(true);
	} else if (!mStr.strcmp(st, "!")) {
		if (terminatorType != NULL) {
			if (terminatorType[0] != EOS) {
				if (spTier != NULL)
					mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
				else
					tempst2[0] = EOS;
				freeAll(12);
			}
			mStr.strcpy(terminatorType, "<t type=\"exclamation\"/>");
		}
		return(true);
	} else if (st[0] == '+') {
		if (terminatorType != NULL) {
			if (terminatorType[0] != EOS) {
				if (spTier != NULL)
					mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
				else
					tempst2[0] = EOS;
				freeAll(12);
			}
		}
		if (!mStr.strcmp(st+1, "...")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"trail_off\"/>");
		} else if (!mStr.strcmp(st+1, "..?")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"trail_off_question\"/>");
		} else if (!mStr.strcmp(st+1, "!?")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"question_exclamation\"/>");
		} else if (!mStr.strcmp(st+1, "/.")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"interruption\"/>");
		} else if (!mStr.strcmp(st+1, "/?")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"interruption_question\"/>");
		} else if (!mStr.strcmp(st+1, "//.")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"self_interruption\"/>");
		} else if (!mStr.strcmp(st+1, "//?")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"self_interruption_question\"/>");
		} else if (!mStr.strcmp(st+1, "&quot;/.")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"quotation_next_line\"/>");
		} else if (!mStr.strcmp(st+1, "&quot;.")) {
			if (terminatorType != NULL) mStr.strcpy(terminatorType, "<t type=\"quotation_precedes\"/>");
		} else if (!mStr.strcmp(st+1, "&quot;")) {
			if (terminatorType != NULL) {
				xml_output(NULL, groupIndent);
				xml_output("<linker type=\"quoted_utterance_next\"/>\n", NULL);
			}
		} else if (!mStr.strcmp(st+1, "^")) {
			if (terminatorType != NULL) {
				xml_output(NULL, groupIndent);
				xml_output("<linker type=\"quick_uptake\"/>\n", NULL);
			}
		} else if (!mStr.strcmp(st+1, "&lt;")) {
			if (terminatorType != NULL) {
				xml_output(NULL, groupIndent);
				xml_output("<linker type=\"lazy_overlap_mark\"/>\n", NULL);
			}
		} else if (!mStr.strcmp(st+1, ",")) {
			if (terminatorType != NULL) {
				xml_output(NULL, groupIndent);
				xml_output("<linker type=\"self_completion\"/>\n", NULL);
			}
		} else if (!mStr.strcmp(st+1, "+")) {
			if (terminatorType != NULL) {
				xml_output(NULL, groupIndent);
				xml_output("<linker type=\"other_completion\"/>\n", NULL);
			}
		} else {
			if (terminatorType != NULL) {
				if (spTier != NULL)
					mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
				else
					tempst2[0] = EOS;
				freeAll(13);
			} else
				return(false);
		}
		return(true);
	} else
		return(false);
}

static void SymboLize(short *st) {
	for (; *st != EOS; st++) {
		if (isSpace(*st))
			*st = '_';
	}
}

static char isSpecialWord(short *word) {
	if (!mStr.strcmp(word, "xxx") || !mStr.strcmp(word, "yyy") || !mStr.strcmp(word, "www") || 
		!mStr.strcmp(word, "0")) {
		return(true);
	} else
		return(false);
}

static char isXYW(short *word) {
	if (!mStr.strcmp(word, "xxx") || !mStr.strcmp(word, "yyy") || !mStr.strcmp(word, "www") ||
		!mStr.strcmp(word, "xx")  || !mStr.strcmp(word, "yy")) {
		return(true);
	} else
		return(false);
}

static char isMarkScope(short *code) {
	if (/*mStr.patmat(code, "[=! *]") || mStr.patmat(code, "[$*]") || */mStr.patmat(code, "[c]") ||
		mStr.patmat(code, "[0 *]") || mStr.patmat(code, "[00 *]") || mStr.patmat(code, "[0\\* *]") ||
		mStr.patmat(code, "[\\%fnt: *]") || mStr.patmat(code, "[+ *]"))
		return(false);
	else
		return(true);
}

static short *getWordType(short *word, short *groupIndent, char isComposit) {
	short *c, *ts;
	long i, j;

	if ((c=mStr.strrchr(word,'@')) != NULL) {
		*c = EOS;
//		changeGroupIndent(groupIndent, 1);
		ts = getWordType(word, groupIndent, isComposit);
//		changeGroupIndent(groupIndent, -1);
		tempst2[0] = EOS;
		if (!mStr.strcmp(c+1, "b"))
			mStr.strcpy(tempst2, "babbling");
		else if (!mStr.strcmp(c+1, "c"))
			mStr.strcpy(tempst2, "child-invented");
		else if (!mStr.strcmp(c+1, "d"))
			mStr.strcpy(tempst2, "dialect");
		else if (!mStr.strcmp(c+1, "f"))
			mStr.strcpy(tempst2, "family-specific");
		else if (!mStr.strcmp(c+1, "fp"))
			mStr.strcpy(tempst2, "filled_pause");
		else if (!mStr.strcmp(c+1, "i"))
			mStr.strcpy(tempst2, "interjection");
		else if (!mStr.strcmp(c+1, "l"))
			mStr.strcpy(tempst2, "letter");
		else if (!mStr.strcmp(c+1, "n"))
			mStr.strcpy(tempst2, "neologism");
		else if (!mStr.strcmp(c+1, "o"))
			mStr.strcpy(tempst2, "onomatopoeia");
		else if (!mStr.strcmp(c+1, "p"))
			mStr.strcpy(tempst2, "phonology_consistent");
		else if (!mStr.strcmp(c+1, "pr"))
			mStr.strcpy(tempst2, "phrasal_repetition");
		else if (!mStr.strcmp(c+1, "s"))
			mStr.strcpy(tempst2, "second_language");
		else if (!mStr.strcmp(c+1, "sc"))
			mStr.strcpy(tempst2, "schwa");
		else if (!mStr.strcmp(c+1, "sl"))
			mStr.strcpy(tempst2, "signed_language");
		else if (!mStr.strcmp(c+1, "sas"))
			mStr.strcpy(tempst2, "sign_speech");
		else if (!mStr.strcmp(c+1, "t"))
			mStr.strcpy(tempst2, "test_word");
		else if (!mStr.strcmp(c+1, "u"))
			mStr.strcpy(tempst2, "UNIBET_transcription");
		else if (!mStr.strcmp(c+1, "x"))
			mStr.strcpy(tempst2, "words_to_be_excluded");
		else if (!mStr.strcmp(c+1, "wp"))
			mStr.strcpy(tempst2, "word_play");
		else if (!mStr.strcmp(c+1, "g"))
			mStr.strcpy(tempst2, "allfail");
		if (isComposit)
			changeGroupIndent(groupIndent, -1);
		if (tempst2[0] == EOS) {
			i = 1L;
			if (*(c+i) == '@')
				i++;
			SymboLize(c+i);
			mStr.sprintf(tempst4, "%ls<f type=\"other\"><symbol>@%ls</symbol></f>\n", groupIndent, c+i);
		} else {
			mStr.sprintf(tempst4, "%ls<f type=\"%ls\"/>\n", groupIndent, tempst2);
		}
		if (isComposit)
			changeGroupIndent(groupIndent, 1);
		priList = addToPriList(priList, 2, tempst4, FALSE, FALSE);
	} else {
		tempst4[0] = EOS;
		if (!mStr.strcmp(word, "xx")) {
			i = 0;
			mStr.sprintf(tempst4+i, "%ls<f type=\"unintelligible\"/>\n", groupIndent);
			priList = addToPriList(priList, 2, tempst4+i, TRUE, FALSE);
			tempst4[i] = EOS;
		} else if (!mStr.strcmp(word, "yy")) {
			i = 0;
			mStr.sprintf(tempst4+i, "%ls<f type=\"unintelligible\"/>\n", groupIndent);
			priList = addToPriList(priList, 2, tempst4+i, TRUE, FALSE);
			tempst4[i] = EOS;
		} else if (mStr.partwcmp(word, "&amp;")) {
			i = 0;
			mStr.sprintf(tempst4+i, "%ls<f type=\"phonological\">%ls</f>\n", groupIndent, word+5);
			priList = addToPriList(priList, 2, tempst4+i, TRUE, FALSE);
			tempst4[i] = EOS;
		} else if (word[0] == '0') {
			i = 0;
			if (word[1] == '*') {
				mStr.sprintf(tempst4+i, "%ls<f type=\"incorrect omission\">%ls</f>\n", groupIndent, word+2);
				priList = addToPriList(priList, 2, tempst4+i, TRUE, FALSE);
			} else if (word[1] == '0') {
				mStr.sprintf(tempst4+i, "%ls<f type=\"ellipsis\">%ls</f>\n", groupIndent, word+2);
				priList = addToPriList(priList, 2, tempst4+i, TRUE, FALSE);
			} else {
				mStr.sprintf(tempst4+i, "%ls<f type=\"omission\">%ls</f>\n", groupIndent, word+1);
				priList = addToPriList(priList, 2, tempst4+i, TRUE, FALSE);
			}
			tempst4[i] = EOS;
		} else {
			char tc;
			long k, offset;
			long isLastCr;

			isLastCr = mStr.strlen(tempst4);
			mStr.strcat(tempst4, groupIndent);
			i = mStr.strlen(tempst4);
			offset = 0L;
			for (j=0; word[j] != EOS;) {
				if (word[j] == '(') {
					j++;
					if (word[j] == '*') {
						j++;
						mStr.sprintf(tempst2, "%ls<k type=\"repetition\">", groupIndent);
						k = mStr.strlen(tempst2);
						while (word[j] != ')' && word[j] != EOS)
							tempst2[k++] = word[j++];
						tempst2[k] = EOS;
						mStr.strcat(tempst2, "</k>\n");
						priList = addToPriList(priList, 7, tempst2, TRUE, FALSE);
					} else if (!mStr.strncmp(word+j, "&amp;", 5)) {
						j += 5;
						mStr.sprintf(tempst2, "%ls<k type=\"repetition c\">", groupIndent);
						k = mStr.strlen(tempst2);
						while (word[j] != ')' && word[j] != EOS)
							tempst2[k++] = word[j++];
						tempst2[k] = EOS;
						mStr.strcat(tempst2, "</k>\n");
						priList = addToPriList(priList, 7, tempst2, TRUE, FALSE);
					} else {
						mStr.sprintf(tempst2, "%ls<f offset=\"%d\" type=\"completion\">", groupIndent, offset);
						k = mStr.strlen(tempst2);
						while (word[j] != ')' && word[j] != EOS) {
							tempst4[i++] = word[j];
							offset++;
							tempst2[k++] = word[j++];
						}
						tempst2[k] = EOS;
						mStr.strcat(tempst2, "</f>\n");
						priList = addToPriList(priList, 2, tempst2, TRUE, FALSE);
					}
					if (word[j] == ')')
						j++;
				} else if (word[j] == ':' && word[j+1] == ':') {
					mStr.sprintf(tempst2, "%ls<p offset=\"%d\" type=\"syllable pause\"/>\n", groupIndent, offset);
					priList = addToPriList(priList, 1, tempst2, TRUE, FALSE);
					j += 2;
				} else if (word[j] == ':') {
					if (isLastCr != -1) {
						mStr.sprintf(tempst2, ": %ls", word);
						freeAll(14);
					}
					tc = tempst4[i-1];
					mStr.sprintf(tempst2, "%ls<p offset=\"%d\" extent=\"%d\" type=\"lengthened syllable\"/>\n", groupIndent, offset, 1);
					priList = addToPriList(priList, 1, tempst2, TRUE, FALSE);
					j++;
				} else if (word[j] == '^') {
					j++;
					mStr.sprintf(tempst2, "%ls<p offset=\"%d\" type=\"blocking\"/>\n", groupIndent, offset);
					priList = addToPriList(priList, 1, tempst2, TRUE, FALSE);
				} else if (word[j] == '/') {
					j++;
					if (word[j] == '/' && word[j+1] == '/') {
						mStr.sprintf(tempst2, "%ls<p offset=\"%d\" type=\"contrastive stress\"/>\n", groupIndent, offset);
						priList = addToPriList(priList, 1, tempst2, TRUE, FALSE);
						j += 2;
					} else if (word[j] == '/') {
						mStr.sprintf(tempst2, "%ls<p offset=\"%d\" type=\"accented nucleus\"/>\n", groupIndent, offset);
						priList = addToPriList(priList, 1, tempst2, TRUE, FALSE);
						j++;
					} else {
						mStr.sprintf(tempst2, "%ls<p offset=\"%d\" type=\"stress\"/>\n", groupIndent, offset);
						priList = addToPriList(priList, 1, tempst2, TRUE, FALSE);
					}
				} else if (!mStr.strncmp(word+j,"&amp;",5)) {
					mStr.strncpy(tempst6, word, j);
					tempst6[j] = EOS;
					mStr.sprintf(tempst2, "%ls<mor>\n    %ls<pos><c>?</c></pos> <stem>%ls</stem>\n%ls    <mk offset=\"%d\" type=\"suffix fusion\">%ls</mk>\n%ls</mor>\n", 
						groupIndent, groupIndent, tempst6, groupIndent, offset, word+j+5, groupIndent);
					priList = addToPriList(priList, 3, tempst2, TRUE, FALSE);
					j += 5;
				} else if (word[j] == '-') {
					short type[512];

					j++;
					if (word[j] == '0') {
						j++;
						if (word[j+2] == '*') {
							j++;
							CToW("incorrectly_omitted_affix", type, 512);
						} else
							CToW("omitted_affix", type, 512);
					} else
						CToW("suffix", type, 512);
					mStr.sprintf(tempst2, "%ls<mor>\n    %ls<pos><c>?</c></pos> <stem>?</stem>\n%ls    <mk offset=\"%d\" type=\"%ls\">%ls</mk>\n%ls</mor>\n", 
									groupIndent, groupIndent, groupIndent, offset, type, word+j, groupIndent);
					priList = addToPriList(priList, 3, tempst2, TRUE, FALSE);
				} else if (word[j] == '#') {
					tc = word[j];
					word[j] = EOS;
					mStr.sprintf(tempst2, "%ls<mor>\n    %ls<pos><c>?</c></pos> <stem>?</stem>\n%ls    <mk offset=\"%d\" type=\"prefix\">%ls</mk>\n%ls</mor>\n", 
									groupIndent, groupIndent, groupIndent, offset, word, groupIndent);
					priList = addToPriList(priList, 3, tempst2, TRUE, FALSE);
					word[j] = tc;
					j++;
				} else {
					tempst4[i++] = word[j++];
					offset++;
					isLastCr = -1;
				}
			}
			if (isLastCr != -1)
				tempst4[isLastCr] = EOS;
			else {
				tempst4[i++] = '\n';
			}
			tempst4[i] = EOS;
		}
		ts = (short *)malloc((mStr.strlen(tempst4)+1)*2);
		if (ts == NULL)
			freeAll(1);
		mStr.strcpy(ts, tempst4);
	}
	return(ts);
}
// | & # - = + ~ ^
static void getMorItem(short *mor, short *lang, short *groupIndent, short *st) {
	short *c, *btc, *etc, t, *last, *org;

	if (isTerminator(mor, NULL, (short *)L""))
		return;
	org = mor;
	if ((c=mStr.strchr(mor,'^')) != NULL) {
		*c = EOS;
		getMorItem(mor, lang, groupIndent, st);
		getMorItem(c+1, lang, groupIndent, st);
	} else {
		if (st == NULL) {
			xml_output(NULL, groupIndent);
			xml_output("<mor", NULL);
			if (mStr.strcmp(gLanguage, lang)) {
				if (!mStr.strcmp(lang, "IPA")) {
					xml_output(" lang=\"", NULL);
					xml_output(NULL, gLanguage);
					xml_output("-", NULL);
					xml_output(NULL, lang);
					xml_output("\"", NULL);
				} else {
					xml_output(" lang=\"", NULL);
					xml_output(NULL, lang);
					xml_output("\"", NULL);
				}
			}
			xml_output(">\n", NULL);
		} else {
			mStr.strcpy(st, groupIndent);
			mStr.strcat(st, "<mor");
			if (mStr.strcmp(gLanguage, lang)) {
				if (!mStr.strcmp(lang, "IPA")) {
					mStr.strcat(st, " lang=\"");
					mStr.strcat(st, gLanguage);
					mStr.strcat(st, "-");
					mStr.strcat(st, lang);
					mStr.strcat(st, "\"");
				} else {
					mStr.strcat(st, " lang=\"");
					mStr.strcat(st, lang);
					mStr.strcat(st, "\"");
				}
			}
			mStr.strcat(st, ">\n");
		}
		changeGroupIndent(groupIndent, 1);
		last = NULL;
		while (*mor != EOS) {
			for (c=mor; *c != '|' && *c != '#' &&  *c != '-' && 
						*c != '=' && *c != '~' && *c != EOS; c++) {
				if (!mStr.strncmp(c, "&amp;", 5))
					break;
			}
			t = *c;
			*c = EOS;
			if (last == NULL) {
				if (t != '|') {
					*c = t;
					mStr.sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(15);
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("<pos>\n", NULL);
				} else {
					mStr.strcat(st, groupIndent);
					mStr.strcat(st, "<pos>\n");
				}
				if ((etc=mStr.strchr(mor, ':')) == NULL) {
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("    <c>", NULL);
						xml_output(NULL, mor);
						xml_output("</c>\n", NULL);
					} else {
						mStr.strcat(st, groupIndent);
						mStr.strcat(st, "    <c>");
						mStr.strcat(st, mor);
						mStr.strcat(st, "</c>\n");
					}
				} else {
					*etc = EOS;
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("    <c>", NULL);
						xml_output(NULL, mor);
						xml_output("</c>\n", NULL);
					} else {
						mStr.strcat(st, groupIndent);
						mStr.strcat(st, "    <c>");
						mStr.strcat(st, mor);
						mStr.strcat(st, "</c>\n");
					}
					*etc = ':';
					btc = etc + 1;
					while ((etc=mStr.strchr(btc, ':')) != NULL) {
						*etc = EOS;
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("    <s>", NULL);
							xml_output(NULL, btc);
							xml_output("</s>\n", NULL);
						} else {
							mStr.strcat(st, groupIndent);
							mStr.strcat(st, "    <s>");
							mStr.strcat(st, btc);
							mStr.strcat(st, "</s>\n");
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
						mStr.strcat(st, groupIndent);
						mStr.strcat(st, "    <s>");
						mStr.strcat(st, btc);
						mStr.strcat(st, "</s>\n");
					}
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("</pos>\n", NULL);
				} else {
					mStr.strcat(st, groupIndent);
					mStr.strcat(st, "</pos>\n");
				}
			} else if (*last == '|') {
				if ((etc=mStr.strchr(mor, '+')) == NULL) {
					if (t == '|') {
						*c = t;
						mStr.sprintf(tempst2, ": %ls\nItem=%ls", c, org);
						freeAll(16);
					}
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("<stem>", NULL);
						xml_output(NULL, mor);
						xml_output("</stem>\n", NULL);
					} else {
						mStr.strcat(st, groupIndent);
						mStr.strcat(st, "<stem>");
						mStr.strcat(st, mor);
						mStr.strcat(st, "</stem>\n");
					}
				} else {
					if (t == '|') {
						*etc = EOS;
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<stem>", NULL);
							xml_output(NULL, mor);
							xml_output("</stem>\n", NULL);
						} else {
							mStr.strcat(st, groupIndent);
							mStr.strcat(st, "<stem>");
							mStr.strcat(st, mor);
							mStr.strcat(st, "</stem>\n");
						}
						*etc = '+';
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<wk type=\"cmp\"/>\n", NULL);
						} else {
							mStr.strcat(st, groupIndent);
							mStr.strcat(st, "<wk type=\"cmp\"/>\n");
						}
						last = NULL;
						*c = t;
						mor = etc + 1;
						continue;
					} else {
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<stem>", NULL);
							xml_output(NULL, mor);
							xml_output("</stem>\n", NULL);
						} else {
							mStr.strcat(st, groupIndent);
							mStr.strcat(st, "<stem>");
							mStr.strcat(st, mor);
							mStr.strcat(st, "</stem>\n");
						}
					}
				}
			} else if (!mStr.strncmp(last, "&amp;", 5)) {
				if (t == '|') {
					*c = t;
					mStr.sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(17);
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("<mk type=\"suffix fusion\">", NULL);
					xml_output(NULL, mor+4);
					xml_output("</mk>\n", NULL);
				} else {
					mStr.strcat(st, groupIndent);
					mStr.strcat(st, "<mk type=\"suffix fusion\">");
					mStr.strcat(st, mor+4);
					mStr.strcat(st, "</mk>\n");
				}
			} else if (*last == '#') {
				if (t == '|') {
					*c = t;
					mStr.sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(18);
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("<mk type=\"prefix\">", NULL);
					xml_output(NULL, mor);
					xml_output("</mk>\n", NULL);
				} else {
					mStr.strcat(st, groupIndent);
					mStr.strcat(st, "<mk type=\"prefix\">");
					mStr.strcat(st, mor);
					mStr.strcat(st, "</mk>\n");
				}
			} else if (*last == '-') {
				if (t == '|') {
					*c = t;
					mStr.sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(19);
				}
				if (*mor == '0') {
					if (mor[1] == '*') {
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<mk type=\"incorrectly_omitted_affix\">", NULL);
							xml_output(NULL, mor+2);
							xml_output("</mk>\n", NULL);
						} else {
							mStr.strcat(st, groupIndent);
							mStr.strcat(st, "<mk type=\"incorrectly_omitted_affix\">");
							mStr.strcat(st, mor+2);
							mStr.strcat(st, "</mk>\n");
						}
					} else {
						if (st == NULL) {
							xml_output(NULL, groupIndent);
							xml_output("<mk type=\"omitted_affix\">", NULL);
							xml_output(NULL, mor+1);
							xml_output("</mk>\n", NULL);
						} else {
							mStr.strcat(st, groupIndent);
							mStr.strcat(st, "<mk type=\"omitted_affix\">");
							mStr.strcat(st, mor+1);
							mStr.strcat(st, "</mk>\n");
						}
					}
				} else {
					if (st == NULL) {
						xml_output(NULL, groupIndent);
						xml_output("<mk type=\"suffix\">", NULL);
						xml_output(NULL, mor);
						xml_output("</mk>\n", NULL);
					} else {
						mStr.strcat(st, groupIndent);
						mStr.strcat(st, "<mk type=\"suffix\">");
						mStr.strcat(st, mor);
						mStr.strcat(st, "</mk>\n");
					}
				}
			} else if (*last == '=') {
				*c = t;
				if (t == '|') {
					mStr.sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(20);
				}
				mStr.sprintf(tempst2, "\nItem=%ls", org);
				freeAll(900);
			} else if (*last == '~') {
				*c = t;
				if (t != '|') {
					mStr.sprintf(tempst2, ": %ls\nItem=%ls", c, org);
					freeAll(21);
				}
				if (st == NULL) {
					xml_output(NULL, groupIndent);
					xml_output("<wk type=\"cli\"/>\n", NULL);
				} else {
					mStr.strcat(st, groupIndent);
					mStr.strcat(st, "<wk type=\"cli\"/>\n");
				}
				last = NULL;
				continue;
			}

			last = c;
			*c = t;
			if (t != EOS)
				mor = c + 1;
			else
				break;
		}
		changeGroupIndent(groupIndent, -1);
		if (st == NULL) {
			xml_output(NULL, groupIndent);
			xml_output("</mor>\n", NULL);
		} else {
			mStr.strcat(st, groupIndent);
			mStr.strcat(st, "</mor>\n");
			priList = addToPriList(priList, 3, st, FALSE, FALSE);
		}
	}
}

static void getPhoItem(struct items *i, short *groupIndent, short *st) {
	if (st == NULL) {
		xml_output(NULL, groupIndent);
		xml_output("<pho", NULL);
		if (mStr.strcmp(gLanguage, i->lang)) {
			if (!mStr.strcmp(i->lang, "IPA")) {
				xml_output(" lang=\"", NULL);
				xml_output(NULL, gLanguage);
				xml_output("-", NULL);
				xml_output(NULL, i->lang);
				xml_output("\"", NULL);
			} else {
				xml_output(" lang=\"", NULL);
				xml_output(NULL, i->lang);
				xml_output("\"", NULL);
			}
		}
		xml_output(">", NULL);
		xml_output(NULL, i->word);
		xml_output("</pho>\n", NULL);
	} else {
		tempst6[0] = EOS;
		if (mStr.strcmp(gLanguage, i->lang)) {
			if (!mStr.strcmp(i->lang, "IPA")) {
				mStr.strcpy(tempst6, " lang=\"");
				mStr.strcat(tempst6, gLanguage);
				mStr.strcat(tempst6, "-");
				mStr.strcat(tempst6, i->lang);
				mStr.strcat(tempst6, "\"");
			} else {
				mStr.strcpy(tempst6, " lang=\"");
				mStr.strcat(tempst6, i->lang);
				mStr.strcat(tempst6, "\"");
			}
		}
		mStr.sprintf(st, "%ls<pho%ls>%ls</pho>\n", groupIndent, tempst6, i->word);
	}
}

static void getModItem(struct items *i, short *groupIndent, short *st) {
	if (st == NULL) {
		xml_output(NULL, groupIndent);
		xml_output("<target", NULL);
		if (mStr.strcmp(gLanguage, i->lang)) {
			if (!mStr.strcmp(i->lang, "IPA")) {
				xml_output(" lang=\"", NULL);
				xml_output(NULL, gLanguage);
				xml_output("-", NULL);
				xml_output(NULL, i->lang);
				xml_output("\"", NULL);
			} else {
				xml_output(" lang=\"", NULL);
				xml_output(NULL, i->lang);
				xml_output("\"", NULL);
			}
		}
		xml_output(">", NULL);
		xml_output(NULL, i->word);
		xml_output("</target>\n", NULL);
	} else {
		tempst6[0] = EOS;
		if (mStr.strcmp(gLanguage, i->lang)) {
			if (!mStr.strcmp(i->lang, "IPA")) {
				mStr.strcpy(tempst6, " lang=\"");
				mStr.strcat(tempst6, gLanguage);
				mStr.strcat(tempst6, "-");
				mStr.strcat(tempst6, i->lang);
				mStr.strcat(tempst6, "\"");
			} else {
				mStr.strcpy(tempst6, " lang=\"");
				mStr.strcat(tempst6, i->lang);
				mStr.strcat(tempst6, "\"");
			}
		}
		mStr.sprintf(st, "%ls<target%ls>%ls</target>\n", groupIndent, tempst6, i->word);
	}
}

static char closingAddressee(struct items *w, char isCkeckCurrent) {
	int cnt = 0, codesCnt;

	if (isCkeckCurrent) {
		for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
			if (mStr.patmat(w->code[codesCnt]->word,"[\\%add: *]")) {
				uS.extractString(tempst3, w->code[codesCnt]->word, "[%add: ", ']');
				return(true);
			}
		}
	} else {
		for (; w != NULL; w=w->NextItem) {
			if (mStr.strcmp(w->word, "&lt;") == 0)
				cnt++;
			else if (mStr.strcmp(w->word, "&gt;") == 0) {
				cnt--;
				if (cnt == 0) {
					for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
						if (mStr.patmat(w->code[codesCnt]->word,"[\\%add: *]")) {
							uS.extractString(tempst5, w->code[codesCnt]->word, "[%add: ", ']');
							return(true);
						}
					}
					break;
				}
			}
		}
	}
	return(false);
}

static char closingOverlap(struct items *w, char isCkeckCurrent) {
	int cnt = 0, codesCnt;
	struct items *tw, *code;

	if (isCkeckCurrent) {
		for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
			if (mStr.patmat(w->code[codesCnt]->word,"[&gt;*]") || 
					mStr.patmat(w->code[codesCnt]->word,"[&lt;*]") || 
					!mStr.strcmp(w->code[codesCnt]->word,"[&lt;&gt;]")) {
				if (cOverlap == NULL) {
					freeAll(22);
				}
				return(true);
			}
		}
	} else {
		for (tw=w; w != NULL; w=w->NextItem) {
			if (mStr.strcmp(w->word, "&lt;") == 0)
				cnt++;
			else if (mStr.strcmp(w->word, "&gt;") == 0) {
				cnt--;
				if (cnt == 0) {
					for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
						if (mStr.patmat(w->code[codesCnt]->word,"[&gt;*]") || 
								mStr.patmat(w->code[codesCnt]->word,"[&lt;*]") || 
								!mStr.strcmp(w->code[codesCnt]->word,"[&lt;&gt;]")) {
							if (cOverlap == NULL) {
								freeAll(22);
							}
							code = addToItems(NULL, w->code[codesCnt]->word, NULL);
							while (tw != NULL &&
									(mStr.patmat(tw->word,"[&gt;*]")  || 
									 mStr.patmat(tw->word,"[&lt;*]") || 
									 !mStr.strcmp(tw->word,"[&lt;&gt;]")))
								tw = tw->NextItem;
							if (tw == NULL)
								return(false);
							if (tw->codesCnt >= CODESMAX) {
								freeAll(30);
							}
							tw->code[tw->codesCnt] = code;
							tw->codesCnt++;

							tw = w;
							while (tw != NULL &&
									(mStr.patmat(tw->word,"[&gt;*]")  || 
									 mStr.patmat(tw->word,"[&lt;*]") || 
									 !mStr.strcmp(tw->word,"[&lt;&gt;]")))
								tw = tw->PrevItem;
							if (tw == NULL)
								return(false);
							if (tw->codesCnt >= CODESMAX) {
								freeAll(30);
							}
							tw->code[tw->codesCnt] = w->code[codesCnt];
							tw->codesCnt++;
							
							shiftCodeArray(w, codesCnt);
							return(false);
						}
					}
					break;
				}
			}
		}
	}
	return(false);
}
/*
#if defined(_MAC_CODE)
static char *itoa(int num, char *st, int size) {
	mStr.sprintf(st, "%d", num);
	return(st);
}
#endif
*/
static void addOverlapTags(short *st) {
	char  isFirst, isOverlapMatched;
	short tst[34];

	isOverlapMatched = false;
	isFirst = true;
	if (st == NULL) {
		xml_output(" id=\"_", NULL);
		xml_output(NULL, mStr.itoa(cOverlap->cId, tst,10));
		xml_output("\"", NULL);
		xml_output(" ovl=\"", NULL);
		xml_output("_", NULL);
		xml_output(NULL, mStr.itoa(cOverlap->with, tst,10));
		isOverlapMatched = true;
		xml_output("\"", NULL);
	} else {
		mStr.sprintf(st+mStr.strlen(st), " id=\"_%d\" ovl=\"", cOverlap->cId);
		mStr.sprintf(st+mStr.strlen(st), "_%d", cOverlap->with);
		isOverlapMatched = true;
		mStr.sprintf(st+mStr.strlen(st), "\"");
	}
	if (!isOverlapMatched) {
		if (spTier != NULL)
			mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
		else
			tempst2[0] = EOS;
		freeAll(27);
	}
}

static void outputPriorities(short *groupIndent, char isInsideOnly) {
	struct priolst *tp;
	char morphFirst = 0;

	for (tp=priList; tp != NULL; tp=tp->next) {
		if (!tp->isUsed && (!isInsideOnly || tp->isInsideWord)) {
			tp->isUsed = TRUE;
			if (tp->priority == 3 && morphFirst == 0) {
				if (tp->next != NULL && tp->next->priority == 3) {
					morphFirst = 2;
					xml_output(NULL, groupIndent);
					xml_output("<gm>\n", NULL);
				} else
					morphFirst = 1;
			}
			if (tp->priority > 3 && morphFirst == 2) {
				xml_output(NULL, groupIndent);
				xml_output("</gm>\n", NULL);
				morphFirst = 1;
			}
			if (tp->isPad)
				xml_output(NULL, groupIndent);
			xml_output(NULL, tp->st);
			if (tp->isPad)
				xml_output("\n", NULL);
		}
	}
	if (morphFirst == 2) {
		xml_output(NULL, groupIndent);
		xml_output("</gm>\n", NULL);
	}
}


static char isWordComposit(struct items *w, short *groupIndent) {
	short *c, *s, *e, t;
	short *ts;
	
	c = mStr.strrchr(w->word,'+');
	if (c == NULL)
		c = mStr.strrchr(w->word,'~');
	else if (mStr.strrchr(w->word,'~') != NULL) {
		mStr.sprintf(tempst2, ": %ls", w->word);
		freeAll(23);
	}
	if (c == NULL) {
		xml_output(NULL, groupIndent);
		xml_output("<w", NULL);
		if (closingOverlap(w, true))
			addOverlapTags(NULL);
		if (closingAddressee(w, true)) {
			if (tempst3[0] != EOS) {
				xml_output(" addressee=\"", NULL);
				xml_output(NULL, tempst3);
				xml_output("\"", NULL);
			}
		} else if (w->addWord != NULL) {
			mStr.remFblanks(w->addWord->word);
			mStr.uS.remblanks(w->addWord->word);
			if (w->addWord->word[0] != EOS) {
				xml_output(" addressee=\"", NULL);
				xml_output(NULL, w->addWord->word);
				xml_output("\"", NULL);
			}
		}
		xml_output(">\n", NULL);
		changeGroupIndent(groupIndent, 1);
		ts = getWordType(w->word, groupIndent, FALSE);
		xml_output(NULL, ts);
		free(ts);
	} else {
		xml_output(NULL, groupIndent);
		xml_output("<wn", NULL);
		if (closingOverlap(w, true))
			addOverlapTags(NULL);
		if (closingAddressee(w, true)) {
			if (tempst3[0] != EOS) {
				xml_output(" addressee=\"", NULL);
				xml_output(NULL, tempst3);
				xml_output("\"", NULL);
			}
		} else if (w->addWord != NULL) {
			mStr.remFblanks(w->addWord->word);
			mStr.uS.remblanks(w->addWord->word);
			if (w->addWord->word[0] != EOS) {
				xml_output(" addressee=\"", NULL);
				xml_output(NULL, w->addWord->word);
				xml_output("\"", NULL);
			}
		}
		xml_output(">\n", NULL);
		changeGroupIndent(groupIndent, 1);
		s = w->word;
		while (1) {
			e = s;
			while (*e != *c && *e != EOS) e++;
			t = *e;
			*e = EOS;
			xml_output(NULL, groupIndent);
			xml_output("<w>\n", NULL);
			changeGroupIndent(groupIndent, 1);
			ts = getWordType(s, groupIndent, TRUE);
			xml_output(NULL, ts);
			free(ts);
			outputPriorities(groupIndent, TRUE);
			changeGroupIndent(groupIndent, -1);
			xml_output(NULL, groupIndent);
			xml_output("</w>\n", NULL);
			*e = t;
			s = e;
			if (*s != EOS)
				s++;
			else
				break;
			if (*c == '+') {
				xml_output(NULL, groupIndent);
				xml_output("<wk type=\"cmp\"/>\n", NULL);
			} else if (*c == '~') {
				xml_output(NULL, groupIndent);
				xml_output("<wk type=\"cli\"/>\n", NULL);
			}
		}
	}
	if (w->morWord != NULL) {
		getMorItem(w->morWord->word, w->morWord->lang, groupIndent, tempst4);
	}
	if (w->modWord != NULL) {
		getModItem(w->modWord, groupIndent, tempst4);
		priList = addToPriList(priList, 4, tempst4, FALSE, FALSE);
	}
	if (w->phoWord != NULL) {
		getPhoItem(w->phoWord, groupIndent, tempst4);
		priList = addToPriList(priList, 5, tempst4, FALSE, FALSE);
	}

	if (c == NULL)
		return(false);
	else
		return(true);
}
/*
"p"				1
"f"				2
"gm"			3
"mor"			3
"target"		4
"pho"			5
"a"				6
"code"			7
"tone"			8
"translation"	9
"replacement"	10
*/

static short getCodes(struct items *code, char *oClosing, int type, short *groupIndent) {
	short priority = 0, *s;
	long  i;
	short closing[128];

	CToW(oClosing, closing, 128);
	if (mStr.patmat(code->word, "[\\%*: *]")) {
		if (mStr.patmat(code->word, "[\\%fnt: *]")) {
			tempst5[0] = EOS;
		} else if (mStr.patmat(code->word, "[\\%act: *]")) {
			uS.extractString(tempst3, code->word, "[%act: ", ']');
			mStr.sprintf(tempst5, "<a type=\"actions\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%add: *]")) {
			tempst5[0] = EOS;
		} else if (mStr.patmat(code->word, "[\\%alt: *]")) {
			uS.extractString(tempst3, code->word, "[%alt: ", ']');
			mStr.sprintf(tempst5, "<a type=\"alternative\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%cod: *]")) {
			uS.extractString(tempst3, code->word, "[%cod: ", ']');
			mStr.sprintf(tempst5, "<a type=\"gpcoding\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%coh: *]")) {
			uS.extractString(tempst3, code->word, "[%coh: ", ']');
			mStr.sprintf(tempst5, "<a type=\"cohesion\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%com: *]")) {
			uS.extractString(tempst3, code->word, "[%com: ", ']');
			mStr.sprintf(tempst5, "<a type=\"comments\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%def: *]")) {
			uS.extractString(tempst3, code->word, "[%def: ", ']');
			mStr.sprintf(tempst5, "<a type=\"SALT\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%eng: *]")) {
			uS.extractString(tempst3, code->word, "[%eng: ", ']');
			mStr.sprintf(tempst5, "<a type=\"english_translation\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%err: *]")) {
			uS.extractString(tempst3, code->word, "[%err: ", ']');
			mStr.sprintf(tempst5, "<a type=\"errcoding\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%exp: *]")) {
			uS.extractString(tempst3, code->word, "[%exp: ", ']');
			mStr.sprintf(tempst5, "<a type=\"explanation\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%fac: *]")) {
			uS.extractString(tempst3, code->word, "[%fac: ", ']');
			mStr.sprintf(tempst5, "<a type=\"facial\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%flo: *]")) {
			uS.extractString(tempst3, code->word, "[%flo: ", ']');
			mStr.sprintf(tempst5, "<a type=\"flow\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%gls: *]")) {
			uS.extractString(tempst3, code->word, "[%gls: ", ']');
			mStr.sprintf(tempst5, "<a type=\"target_gloss\">%ls</a>", tempst3);
		} else if (mStr.patmat(code->word, "[\\%gpx: *]")) {
			uS.extractString(tempst3, code->word, "[%gpx: ", ']');
			mStr.sprintf(tempst5, "<a type=\"gesture\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%int: *]")) {
			uS.extractString(tempst3, code->word, "[%int: ", ']');
			mStr.sprintf(tempst5, "<a type=\"intonation\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%lan: *]")) {
			uS.extractString(tempst3, code->word, "[%lan: ", ']');
			mStr.sprintf(tempst5, "<a type=\"language\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%par: *]")) {
			uS.extractString(tempst3, code->word, "[%par: ", ']');
			mStr.sprintf(tempst5, "<a type=\"paralinguistics\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%pho: *]")) {
			uS.extractString(tempst3, code->word, "[%pho: ", ']');
			mStr.sprintf(tempst5, "<pho>%ls</pho>", tempst3);
			priority = 5;
		} else if (mStr.patmat(code->word, "[\\%sit: *]")) {
			uS.extractString(tempst3, code->word, "[%sit: ", ']');
			mStr.sprintf(tempst5, "<a type=\"situation\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%spa: *]")) {
			uS.extractString(tempst3, code->word, "[%spa: ", ']');
			mStr.sprintf(tempst5, "<a type=\"speech_act\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%syn: *]")) {
			uS.extractString(tempst3, code->word, "[%syn: ", ']');
			mStr.sprintf(tempst5, "<a type=\"syntactic_structure\">%ls</a>", tempst3);
			priority = 6;
		} else if (mStr.patmat(code->word, "[\\%tim: *]")) {
			uS.extractString(tempst3, code->word, "[%tim: ", ']');
			mStr.sprintf(tempst5, "<a type=\"time_stamp\">%ls</a>", tempst3);
			priority = 6;
		} else {
			mStr.sprintf(tempst2, ": %ls", code->word);
			freeAll(24);
		}
	} else if (mStr.patmat(code->word, "[=! *]")) {
		uS.extractString(tempst3, code->word, "[=! ", ']');
		mStr.sprintf(tempst5, "<a type=\"paralinguistics\">%ls</a>", tempst3);
		priority = 6;
		if (warningFP != NULL) {
			fprintf(warningFP, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(warningFP,"WARNING: found paralinguistics\n");
			if (spTier != NULL)
				fprintf(warningFP, "%ls%ls\n", spTier->sp, spTier->orgLine);
		}
	} else if (mStr.patmat(code->word, "[$*]")) {
		uS.extractString(tempst3, code->word, "[$", ']');
		mStr.sprintf(tempst5, "<a type=\"SALT\">%ls</a>", tempst3);
		priority = 6;
		if (warningFP != NULL) {
			fprintf(warningFP, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(warningFP,"WARNING: found SALT\n");
			if (spTier != NULL)
				fprintf(warningFP, "%ls%ls\n", spTier->sp, spTier->orgLine);
		}
	} else if (mStr.patmat(code->word, "[\\% *]")) {
		uS.extractString(tempst3, code->word, "[% ", ']');
		mStr.sprintf(tempst5, "<a type=\"comments\">%ls</a>", tempst3);
		priority = 6;
		if (warningFP != NULL) {
			fprintf(warningFP, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(warningFP,"WARNING: found comments\n");
			if (spTier != NULL)
				fprintf(warningFP, "%ls%ls\n", spTier->sp, spTier->orgLine);
		}
	} else if (!mStr.strcmp(code->word, "[/]")) {
		mStr.sprintf(tempst5, "<k type=\"retracing no correction\"/>");
		priority = 7;
	} else if (!mStr.strcmp(code->word, "[//]")) {
		mStr.sprintf(tempst5, "<k type=\"retracing with correction\"/>");
		priority = 7;
	} else if (!mStr.strcmp(code->word, "[///]")) {
		mStr.sprintf(tempst5, "<k type=\"retracing reformulation\"/>");
		priority = 7;
	} else if (!mStr.strcmp(code->word, "[/-]")) {
		mStr.sprintf(tempst5, "<k type=\"false start no retracing\"/>");
		priority = 7;
	}  else if (!mStr.strcmp(code->word, "[/?]")) {
		mStr.sprintf(tempst5, "<k type=\"retracing unclear\"/>");
		priority = 7;
	} else if (!mStr.strcmp(code->word, "[?]")) {
		mStr.sprintf(tempst5, "<k type=\"best guess\"/>");
		priority = 7;
	} else if (!mStr.strcmp(code->word, "[!]")) {
		mStr.sprintf(tempst5, "<k type=\"stressing\"/>");
		priority = 7;
	} else if (!mStr.strcmp(code->word, "[!!]")) {
		mStr.sprintf(tempst5, "<k type=\"contrastive stressing\"/>");
		priority = 7;
	} else if (!mStr.strcmp(code->word, "[*]")) {
		mStr.sprintf(tempst5, "<k type=\"error\"/>");
		priority = 7;
	} else if (!mStr.strcmp(code->word, "[&quot;]")) {
		mStr.sprintf(tempst5, "<k type=\"quotation\"/>");
		priority = 7;
	} else if (mStr.patmat(code->word, "[= *]")) {
		uS.extractString(tempst3, code->word, "[= ", ']');
		mStr.sprintf(tempst5, "<a type=\"explanation\">%ls</a>", tempst3);
		priority = 6;
		if (warningFP != NULL) {
			fprintf(warningFP, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(warningFP,"WARNING: found explanation\n");
			if (spTier != NULL)
				fprintf(warningFP, "%ls%ls\n", spTier->sp, spTier->orgLine);
		}
	} else if (mStr.patmat(code->word, "[: *]")) {
		struct priolst *tPriList;

		uS.extractString(tempst3, code->word, "[: ", ']');
		xml_output(NULL, groupIndent);
		xml_output("<replacement>\n", NULL);
		priority = 10;
		changeGroupIndent(groupIndent, 1);
		tPriList = priList;
		for (code=code->replacement; code != NULL; code=code->NextItem) {
			priList = NULL;
			if (isWordComposit(code, groupIndent)) {
				outputPriorities(groupIndent, FALSE);
				priList = freePriList(priList);
				changeGroupIndent(groupIndent, -1);
				xml_output(NULL, groupIndent);
				xml_output("</wn>\n", NULL);
			} else {
				outputPriorities(groupIndent, FALSE);
				priList = freePriList(priList);
				changeGroupIndent(groupIndent, -1);
				xml_output(NULL, groupIndent);
				xml_output("</w>\n", NULL);
			}
		}
		priList = tPriList;
		changeGroupIndent(groupIndent, -1);
		xml_output(NULL, groupIndent);
		xml_output("</replacement>\n", NULL);
		tempst5[0] = EOS;
	} else if (mStr.patmat(code->word, "[=? *]")) {
		uS.extractString(tempst3, code->word, "[=? ", ']');
		mStr.sprintf(tempst5, "<k type=\"alt_transcription\">%ls</k>", tempst3);
		priority = 7;
	} else if (mStr.patmat(code->word, "[00 *]")) {
		uS.extractString(tempst3, code->word, "[00 ", ']');
		mStr.sprintf(tempst5, "<w><f type=\"ellipsis\">%ls</f></w>", tempst3);
	} else if (mStr.patmat(code->word, "[0\\* *]")) {
		uS.extractString(tempst3, code->word, "[0* ", ']');
		mStr.sprintf(tempst5, "<w><f type=\"incorrect_omission\">%ls</f></w>", tempst3);
	} else if (mStr.patmat(code->word, "[0 *]")) {
		uS.extractString(tempst3, code->word, "[0 ", ']');
		mStr.sprintf(tempst5, "<w><f type=\"omission\">%ls</f></w>", tempst3);
	} else if (mStr.patmat(code->word, "[:=* *]")) {
		uS.extractString(tempst3, code->word, "[:=", ']');
		for (i=0; !isSpace(tempst3[i]) && tempst3[i] != EOS; i++) ;
		if (tempst3[i] == EOS) {
			mStr.sprintf(tempst2, ": %ls", code->word);
			freeAll(25);
		}
		tempst3[i] = EOS;
		for (i++; isSpace(tempst3[i]); i++) ;
		s = mStr.strchr(tempst3, '-');
		if (s == NULL) {
			rightLanguage(tempst3);
			mStr.sprintf(tempst5, "<translation lang=\"%ls\">%ls</translation>", 
					tempst3, tempst3+i);
		} else {
			*s = EOS;
			s++;
			rightLanguage(tempst3);
			rightLanguage(s);
			mStr.sprintf(tempst5, "<translation lang=\"%ls\" script=\"%ls\">%ls</translation>", 
					tempst3, s, tempst3+i);
		}
		priority = 9;
	} else if (!mStr.strcmp(code->word, "[c]")) {
		mStr.sprintf(tempst5, "<separator type=\"clause_delimiter\"/>");
	} else if (!mStr.strcmp(code->word, "0")) {
		mStr.sprintf(tempst5, "<e type=\"action\"");
		if (closingOverlap(code, true))
			addOverlapTags(tempst5+mStr.strlen(tempst5));
		if (closingAddressee(code, true) && tempst3[0] != EOS)
			mStr.sprintf(tempst5+mStr.strlen(tempst5), " addressee=\"%ls\"", tempst3);
		mStr.sprintf(tempst5+mStr.strlen(tempst5), "%ls>", closing);
	} else if (!mStr.strcmp(code->word, "www")) {
		mStr.sprintf(tempst5, "<e type=\"untranscribed\"");
		if (closingOverlap(code, true))
			addOverlapTags(tempst5+mStr.strlen(tempst5));
		if (closingAddressee(code, true) && tempst3[0] != EOS)
			mStr.sprintf(tempst5+mStr.strlen(tempst5), " addressee=\"%ls\"", tempst3);
		mStr.sprintf(tempst5+mStr.strlen(tempst5), "%ls>", closing);
	} else if (!mStr.strcmp(code->word, "xxx")) {
		mStr.sprintf(tempst5, "<e type=\"unintelligible\"");
		if (closingOverlap(code, true))
			addOverlapTags(tempst5+mStr.strlen(tempst5));
		if (closingAddressee(code, true) && tempst3[0] != EOS)
			mStr.sprintf(tempst5+mStr.strlen(tempst5), " addressee=\"%ls\"", tempst3);
		mStr.sprintf(tempst5+mStr.strlen(tempst5), "%ls>", closing);
	} else if (!mStr.strcmp(code->word, "yyy")) {
		mStr.sprintf(tempst5, "<e type=\"unintelligible\"");
		if (closingOverlap(code, true))
			addOverlapTags(tempst5+mStr.strlen(tempst5));
		if (closingAddressee(code, true) && tempst3[0] != EOS)
			mStr.sprintf(tempst5+mStr.strlen(tempst5), " addressee=\"%ls\"", tempst3);
		mStr.sprintf(tempst5+mStr.strlen(tempst5), "%ls>", closing);
	} else if (mStr.patmat(code->word,"[&gt;*]") || mStr.patmat(code->word,"[&lt;*]") || !mStr.strcmp(code->word,"[&lt;&gt;]")) {
		tempst5[0] = EOS;
		if (type != GROUP && type != WORDT && type != EVENT) {
			if (spTier != NULL)
				mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
			else
				tempst2[0] = EOS;
			freeAll(26);
		}
		if (cOverlap == NULL) {
			freeAll(22);
		}
/*
fprintf(stderr, "************** %ld\n", lineno);
convertToOrg(tempst3, spLine);
fprintf(stderr, "%ls%ls\n", spName, tempst3);
fprintf(stderr, "cId=%d, num=%d, isUp=%d, isDn=%d\n    with=", 
				cOverlap->cId, cOverlap->num, cOverlap->isUp, cOverlap->isDn);
fprintf(stderr, "%d, ", cOverlap->with);
fprintf(stderr, "\n");
*/
		cOverlap = cOverlap->NextOverlap;
	} else {
		mStr.sprintf(tempst2, ": %ls", code->word);
		freeAll(28);
	}
	return(priority);
}

static char isSeparatorOrToneMarker(short *st, short *groupIndent) {
	if (mStr.strcmp(st, ";") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<separator type=\"semicolon\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, ",") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"syntactic juncture\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, ",,") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"tag question\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-?") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"rising final\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-!") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"final exclamation\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-.") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"falling final\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-&apos;.") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"rise fall\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-,.") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"fall rise\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-,") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"level nonfinal\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-_") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"falling nonfinal\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"low level\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-&apos;") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"rising nonfinal\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-:") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"prev word lenghtened\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "-?") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<tone type=\"rising final\"/>\n", NULL);
		return(true);
	} else if (mStr.strcmp(st, "#") == 0) {
		xml_output(NULL, groupIndent);
		xml_output("<pause/>\n", NULL);
		return(true);
	} else if (st[0] == '#') {
		xml_output(NULL, groupIndent);
		xml_output("<pause length=\"", NULL);
		xml_output(NULL, st+1);
		xml_output("\"/>\n", NULL);
		return(true);
	} else
		return(false);
}

static struct items *markScope(struct items *end, struct items *code, char isMark) {
	char morOmited;

	if (mStr.patmat(code->word, "[: *]") || !mStr.strcmp(code->word, "[&quot;]") || 
			!mStr.strcmp(code->word, "[/]") || !mStr.strcmp(code->word, "[//]"))
		morOmited = true;
	else
		morOmited = false;
	if (end->PrevItem == NULL) {
		mStr.sprintf(tempst2, ": %ls", end->word);
		if (spTier != NULL)
			mStr.sprintf(tempst2+mStr.strlen(tempst2), "\n%ls%ls", spTier->sp, spTier->orgLine);
		freeAll(29);
	} else 
		end = end->PrevItem;
	if (end->morOmited == false)
		end->morOmited = morOmited;
	while (mStr.strcmp(end->word, "&gt;") && 
			(mStr.uS.isskip(end->word,0,dFnt.Encod,MBF) || end->word[0] == HIDEN_C || end->word[0] == '#')) {
		if ((end->word[0] == '[' && end->word[mStr.strlen(end->word)-1] == ']') || mStr.strcmp(end->word, "&lt;") == 0)
			break;
		else
			end = end->PrevItem;
		if (end->morOmited == false)
			end->morOmited = morOmited;
	}
	if (mStr.strcmp(end->word, "&gt;") == 0) {
		if (isMark) {
			if (end->codesCnt >= CODESMAX) {
				freeAll(30);
			}
			end->code[end->codesCnt] = code;
			end->codesCnt++;
		}
		while (mStr.strcmp(end->word, "&lt;") && end != NULL) {
			if (end->morOmited == false)
				end->morOmited = morOmited;
			if (end->word[0] == '[' && end->word[mStr.strlen(end->word)-1] == ']') 
				end = markScope(end, code, false);
			else
				end = end->PrevItem;
		}
		if (end == NULL) {
			if (spTier != NULL)
				mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
			else
				tempst2[0] = EOS;
			freeAll(31);
		} else {
			if (end->morOmited == false)
				end->morOmited = morOmited;
			end = end->PrevItem;
		}
	} else if (end == NULL) ;
	else if (end->word[0] == '[' && end->word[mStr.strlen(end->word)-1] == ']') {
		if (end->morOmited == false)
			end->morOmited = morOmited;
		end = markScope(end, code, isMark);
	} else {
		if (isMark) {
			if (end->codesCnt >= CODESMAX) {
				freeAll(30);
			}
			end->code[end->codesCnt] = code;
			end->codesCnt++;
		}
		end = end->PrevItem;
	}
	return(end);
}

static void parseScope(struct items *end) {
	if (isCainFile)
		return;
	if (end == NULL)
		return;
	for (; end->NextItem != NULL; end=end->NextItem) ;

	while (end != NULL) {
		if (end->word[0] == '[' && end->word[mStr.strlen(end->word)-1] == ']') {
			mStr.uS.isSqCodes(end->word, tempst3, dFnt.Encod, true);
			if (isMarkScope(tempst3)) {
				markScope(end, end, true);
				if (end->PrevItem != NULL)
					end->PrevItem->NextItem = end->NextItem;
				if (end->NextItem != NULL)
					end->NextItem->PrevItem = end->PrevItem;
				end = end->PrevItem;
			} else {
				end->isChange = true;
				end = end->PrevItem;
			}
		} else
			end = end->PrevItem;
	}
}

static void getScopeNums(short *st, long *beg, long *end, struct tiers *h) {
	short *f;

	st += 4;
	*beg = -1;
	*end = -1;
	do {
		if (*st == '-')
			st++;
		f = NULL;
		while (*st != '&' && *st != '-') {
			if (iswdigit(*st)) {
				f = st;
				for (; iswdigit(*st); st++) ;
			} else
				st++;
		}
		if (f == NULL) {
			mStr.sprintf(tempst2, "\n%ls%ls", h->sp, h->orgLine);
			freeAll(32);
		} else {
			if (*beg == -1)
				*beg = mStr.atol(f);
			else
				*end = mStr.atol(f);
		}
	} while (*st == '-') ;
}

static struct items *matchDepTiers(struct items **orgW, struct items *orgD, int tierType, struct tiers *h, int lev) {
	int  codesCnt;
	long beg, end, cur;
	struct items *u;
	struct items *w;
	struct items *d;

	w = *orgW;
	d = orgD;
	if (tierType == MORTIER) {
		while (d != NULL && w != NULL) {
			if (w->replacement != NULL) {
				d = matchDepTiers(&w->replacement, d, tierType, h, lev+1);
			} else if (!w->omited && !w->morOmited) {
				w->morWord = d;
				d = d->NextItem;
			} else {
				for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
					if (w->code[codesCnt]->replacement != NULL)
						d = matchDepTiers(&w->code[codesCnt]->replacement, d, tierType, h, lev+1);
					if (!mStr.strcmp(w->code[codesCnt]->word, "[&quot;]")) {
						w->morWord = d;
						d = d->NextItem;
					}
				}
			}
			w = w->NextItem;
		}
		if (lev == 0) {
			while (w != NULL) {
				if (!w->omited && !w->morOmited)
					break;
				w = w->NextItem;
			}		
			if (d != NULL || w != NULL) {
				if (d == NULL)
					d = w;
				w = *orgW;
				while (w != NULL) {
					w->morWord = NULL;
					w = w->NextItem;
				}
			}
		}
	} else if (tierType == PHOTIER || tierType == MODTIER || tierType == ADDTIER) {
		if (isThisScoping(d->word, 0)) {
			while (d != NULL) {
				getScopeNums(d->word, &beg, &end, h);
				d = d->NextItem;
				if (d == NULL) {
					mStr.sprintf(tempst2, "\n%ls%ls", h->sp, h->orgLine);
					freeAll(33);
				}
				if (isThisScoping(d->word, 0)) {
					mStr.sprintf(tempst2, "\n%ls%ls", h->sp, h->orgLine);
					freeAll(33);
				}
				if (beg == 0) {
					mStr.sprintf(tempst2, "\n%ls%ls", h->sp, h->orgLine);
					freeAll(32);
				}
				cur = beg;
				for (w=*orgW; w != NULL; w = w->NextItem) {
					if (!w->omited || isXYW(w->word)) {
						if (cur == 1) {
							if (end == -1) {
								if (tierType == PHOTIER)
									w->phoWord = d;
								else if (tierType == ADDTIER)
									w->addWord = d;
								else
									w->modWord = d;
							} else {
								u = NEW(struct items);
								if (u == NULL) {
									freeAll(1);
								}
								mStr.strcpy(u->word, "&lt;");
								u->replacement = NULL;
								u->morWord = NULL;
								u->phoWord = NULL;
								u->modWord = NULL;
								u->addWord = NULL;
								u->codesCnt = 0;
								mStr.strcpy(u->lang, lLanguage);
								u->omited = true;
								u->morOmited = false;
								u->isChange = false;
								u->sndMark = 0;
								u->PrevItem = w->PrevItem;
								w->PrevItem = u;
								u->NextItem = w;
								if (u->PrevItem != NULL)
									u->PrevItem->NextItem = u;
								else if (*orgW == w)
									*orgW = u;
							}
							break;
						}
						cur--;
					}
				}
				if (cur > 1) {
					if (spTier != NULL)
						mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
					else
						tempst2[0] = EOS;
					mStr.sprintf(tempst2+mStr.strlen(tempst2), "\n%ls%ls", h->sp, h->orgLine);
					freeAll(34);
				}
				if (end != -1) {
					cur = end;
					for (w=*orgW; w != NULL; w = w->NextItem) {
						if (!w->omited || isXYW(w->word)) {
							if (cur == 1) {
								u = NEW(struct items);
								if (u == NULL) {
									freeAll(1);
								}
								mStr.strcpy(tempst3, d->word);
								if (tierType == PHOTIER)
									mStr.sprintf(d->word, "[%%pho: %ls]", tempst3);
								else if (tierType == ADDTIER)
									mStr.sprintf(d->word, "[%%add: %ls]", tempst3);
								else
									mStr.sprintf(d->word, "[%%mod: %ls]", tempst3);
								mStr.strcpy(u->word, "&gt;");
								u->code[0] = d;
								u->codesCnt = 1;
								u->replacement = NULL;
								u->morWord = NULL;
								u->phoWord = NULL;
								u->modWord = NULL;
								u->addWord = NULL;
								mStr.strcpy(u->lang, lLanguage);
								u->omited = true;
								u->morOmited = false;
								u->isChange = false;
								u->sndMark = 0;
								u->NextItem = w->NextItem;
								w->NextItem = u;
								u->PrevItem = w;
								if (u->NextItem != NULL)
									u->NextItem->PrevItem = u;
								break;
							}
							cur--;
						}
					}
					if (cur > 1) {
						if (spTier != NULL)
							mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
						else
							tempst2[0] = EOS;
						mStr.sprintf(tempst2+mStr.strlen(tempst2), "\n%ls%ls", h->sp, h->orgLine);
						freeAll(34);
					}
				}
				do {
					d = d->NextItem;
				} while (d != NULL && isTerminator(d->word, NULL, (short *)L"")) ;
				if (d != NULL && !isThisScoping(d->word, 0)) {
					mStr.sprintf(tempst2, "\n%ls%ls", h->sp, h->orgLine);
					freeAll(35);
				}
			}
		} else {
			if (tierType == ADDTIER) {
				mStr.sprintf(tempst2, "\n%ls%ls", h->sp, h->orgLine);
				freeAll(36);
			}
			while (d != NULL && w != NULL) {
				if (!w->omited || isXYW(w->word)) {
					if (tierType == PHOTIER)
						w->phoWord = d;
					else
						w->modWord = d;
					d = d->NextItem;
				}
				w = w->NextItem;
			}
			while (w != NULL) {
				if (!w->omited)
					break;
				w = w->NextItem;
			}		
			if (d != NULL || w != NULL) {
				if (d == NULL)
					d = w;
				w = *orgW;
				while (w != NULL) {
					if (tierType == PHOTIER)
						w->phoWord = NULL;
					else
						w->modWord = NULL;
					w = w->NextItem;
				}
			}
		}
	}
	return(d);
}

static char isCodeWordNonEmpty(struct items *w) {
	int codesCnt;

	for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
		if (w->code[codesCnt]->word[0] != EOS)
			return(true);
	}
	return(false);
}

static struct items *addItemToEnd(struct items *root, struct items *items) {
	struct items *t;

	if (items == NULL)
		return(root);

	if (root == NULL)
		root = items;
	else {
		for (t=root; t->NextItem != NULL; t=t->NextItem) ;
		t->NextItem = items;
	}
	return(root);
}

static struct items *cleanupGroups(struct items *item, struct items **words) {
	int i, itemsFound = 0;
	struct items *w, *pw;

	w = item->NextItem;
	if (w == NULL)
		return(w);

	for (; w != NULL; w=w->NextItem) {
		if (mStr.strcmp(w->word, "&lt;") == 0) {
			w = cleanupGroups(w, words);
			if (w == NULL)
				break;
		} else if (mStr.strcmp(w->word, "&gt;") == 0) {
			pw = w->PrevItem;
			if (itemsFound == 1 && pw != NULL) {
				for (i=0; i < w->codesCnt; i++) {
					if (pw->codesCnt >= CODESMAX) {
						freeAll(30);
					}
					pw->code[pw->codesCnt] = w->code[i];
					pw->codesCnt++;
				}
				pw->replacement = addItemToEnd(pw->replacement, w->replacement);
				pw->morWord = addItemToEnd(pw->morWord, w->morWord);
				pw->phoWord = addItemToEnd(pw->phoWord, w->phoWord);
				pw->modWord = addItemToEnd(pw->modWord, w->modWord);
				pw->addWord = addItemToEnd(pw->addWord, w->addWord);
				if (w->sndMark == 2 || w->sndMark == 3) {
					pw->sndMark = w->sndMark;
				} else if (pw->sndMark == 0)
					pw->sndMark = w->sndMark;
				if (w->isChange)
					pw->isChange = w->isChange;
				if (item == *words) {
					*words = item->NextItem;
					(*words)->PrevItem = NULL;
				} else {
					if (item->PrevItem != NULL)
						item->PrevItem->NextItem = item->NextItem;
					if (item->NextItem != NULL)
						item->NextItem->PrevItem = item->PrevItem;
				}
				freeItem(item);
				item = w;
				w = w->NextItem;
				if (item->PrevItem != NULL)
					item->PrevItem->NextItem = item->NextItem;
				if (item->NextItem != NULL)
					item->NextItem->PrevItem = item->PrevItem;
				free(item);
			}
			return(w);
		}
		itemsFound++;
	}
	return(w);
}

static void writeUttCluster(void) {
	int  cnt;
	int  codesCnt;
	long i;
	short *ts;
	short st[1024], groupIndent[256], terminatorType[256];
	short lang[5];
	short priority;
	struct tiers *h;
	struct items *w;

	if (headers == NULL)
		return;
	spTier = NULL;
	xml_output("<u", NULL);
	for (h=headers; h != NULL; h=h->NextUtt) {
		CToW(PUNCTUATION_SET, punctuation, 50);
		if (h->sp[0] == '*') {
			spTier = h;
			uS.extractString(st, h->sp, "*", ':');
			mStr.uS.uppercasestr(st, dFnt.Encod, MBF);
			xml_output(" who=\"", NULL);
			xml_output(NULL, st);
			xml_output("\"", NULL);
			i = 0L;
			while ((i=getitem(st, h->line, i, OTHTIER))) {
				if (mStr.uS.isRightChar(st, 0, '[',dFnt.Encod,MBF)) {
					if (mStr.uS.isSqBracketItem(st, 1, dFnt.Encod, MBF)) {
						mStr.uS.cleanUpCodes(st, dFnt.Encod, MBF);
					}
				}
				words = addToItems(words, st, &w);
				if (mStr.patmat(st, "[: *]")) {
					uS.extractString(tempst3, st, "[: ", ']');
					cnt = 0;
					while ((cnt=getitem(st, tempst3, cnt, OTHTIER))) {
						w->replacement = addToItems(w->replacement, st, NULL);
					}
				}
			}
			parseScope(words);
			for (w=words; w != NULL; w=w->NextItem) {
				if (mStr.strcmp(w->word, "&lt;") == 0) {
					w = cleanupGroups(w, &words);
					if (w == NULL)
						break;
				}
			}
			for (w=words; w != NULL; w=w->NextItem) {
				if (isExcludeWord(w->word))
					w->omited = true;
				else if (mStr.patmat(w->word, "#*"))
					w->omited = true;
				else if (mStr.uS.isskip(w->word,0,dFnt.Encod,MBF))
					w->omited = true;
				else if (isTerminator(w->word, NULL, (short *)L""))
					w->omited = true;
				else if (mStr.patmat(w->word, "[*]"))
					w->omited = true;
			}
		} else if (!mStr.strncmp(h->sp, "%mor:", 5)) { 
			mStr.strcpy(lang, lLanguage);
			for (i=0L; h->line[i] != EOS; i++) {
				if (mStr.partwcmp(h->line+i, FONTMARKER)) {
					long o=i;

					for (; h->line[i] != ']' && h->line[i] != EOS; i++) ;
					if (h->line[i] == ']') {
						uS.extractString(tempst3, h->line+o, "[%fnt: ", ']');
						makeLanguage(tempst3, false);
					}
				}
			}
			i = 0L;
			while (i=getitem(st, h->line, i, MORTIER)) {
				if (!mStr.patmat(st, "[*]") && !isTerminator(st, NULL, (short *)L""))
					mor = addToItems(mor, st, NULL);
			}
			mStr.strcpy(lLanguage, lang);
			if (matchDepTiers(&words, mor, MORTIER, h, 0) == NULL)
				mor = NULL;
			else {
				isWarning = true;
				if (warningFP == NULL) {
					if (spTier != NULL)
						mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
					else
						tempst2[0] = EOS;
					mStr.sprintf(tempst2+mStr.strlen(tempst2), "\n%ls%ls", h->sp, h->orgLine);
					freeAll(37);
				}
				if (warningFP != NULL) {
					fprintf(warningFP, "*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(warningFP,"Not all %%mor: items matched the main tier\n");
					if (spTier != NULL)
						fprintf(warningFP, "%ls%ls\n", spTier->sp, spTier->orgLine);
					fprintf(warningFP,"%ls%ls\n", h->sp, h->orgLine);
				}
			}
		} else if (!mStr.strncmp(h->sp, "%pho:", 5)) { 
			CToW(PUNCT_PHO_MOD_SET, punctuation, 50);
			mStr.strcpy(lang, lLanguage);
			for (i=0L; h->line[i] != EOS; i++) {
				if (mStr.partwcmp(h->line+i, FONTMARKER)) {
					long o=i;

					for (; h->line[i] != ']' && h->line[i] != EOS; i++) ;
					if (h->line[i] == ']') {
						uS.extractString(tempst3, h->line+o, "[%fnt: ", ']');
						makeLanguage(tempst3, false);
					}
				}
			}
			i = 0L;
			while ((i=getitem(st, h->line, i, PHOTIER))) {
				if (!mStr.patmat(st, "[*]"))
					break;
			}
			if (i != 0) {
				if (isThisScoping(st, 0)) {
					do {
						mStr.uS.remblanks(st);
						if (!mStr.patmat(st, "[*]") && !isTerminator(st, NULL, (short *)L""))
							pho = addToItems(pho, st, NULL);
					} while ((i=getScopedItem(st, h->line, i))) ;
				} else {
					do {
						if (!mStr.patmat(st, "[*]") && !isTerminator(st, NULL, (short *)L""))
							pho = addToItems(pho, st, NULL);
					} while ((i=getitem(st, h->line, i, PHOTIER))) ;
				}
			}
			mStr.strcpy(lLanguage, lang);
			if (matchDepTiers(&words, pho, PHOTIER, h, 0) == NULL)
				pho = NULL;
			else {
				isWarning = true;
				if (warningFP == NULL) {
					if (spTier != NULL)
						mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
					else
						tempst2[0] = EOS;
					mStr.sprintf(tempst2+mStr.strlen(tempst2), "\n%ls%ls", h->sp, h->orgLine);
					freeAll(38);
				}
				if (warningFP != NULL) {
					fprintf(warningFP, "*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(warningFP,"Not all %%pho: items matched the main tier\n");
					if (spTier != NULL)
						fprintf(warningFP, "%ls%ls\n", spTier->sp, spTier->orgLine);
					fprintf(warningFP,"%ls%ls\n", h->sp, h->orgLine);
				}
			}
		} else if (!mStr.strncmp(h->sp, "%mod:", 5)) { 
			CToW(PUNCT_PHO_MOD_SET, punctuation, 50);
			mStr.strcpy(lang, lLanguage);
			for (i=0L; h->line[i] != EOS; i++) {
				if (mStr.partwcmp(h->line+i, FONTMARKER)) {
					long o=i;

					if (h->line[i] == ']') {
						uS.extractString(tempst3, h->line+o, "[%fnt: ", ']');
						makeLanguage(tempst3, false);
					}
				}
			}
			i = 0L;
			while ((i=getitem(st, h->line, i, MODTIER))) {
				if (!mStr.patmat(st, "[*]"))
					break;
			}
			if (i != 0) {
				if (isThisScoping(st, 0)) {
					do {
						mStr.uS.remblanks(st);
						if (!mStr.patmat(st, "[*]") && !isTerminator(st, NULL, (short *)L""))
							mod = addToItems(mod, st, NULL);
					} while ((i=getScopedItem(st, h->line, i))) ;
				} else {
					do {
						if (!mStr.patmat(st, "[*]") && !isTerminator(st, NULL, (short *)L""))
							mod = addToItems(mod, st, NULL);
					} while ((i=getitem(st, h->line, i, MODTIER))) ;
				}
			}
			mStr.strcpy(lLanguage, lang);
			if (matchDepTiers(&words, mod, MODTIER, h, 0) == NULL)
				mod = NULL;
			else {
				isWarning = true;
				if (warningFP == NULL) {
					if (spTier != NULL)
						mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
					else
						tempst2[0] = EOS;
					mStr.sprintf(tempst2+mStr.strlen(tempst2), "\n%ls%ls", h->sp, h->orgLine);
					freeAll(39);
				}
				if (warningFP != NULL) {
					fprintf(warningFP, "*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(warningFP,"Not all %%mod: items matched the main tier\n");
					if (spTier != NULL)
						fprintf(warningFP, "%ls%ls\n", spTier->sp, spTier->orgLine);
					fprintf(warningFP,"%ls%ls\n", h->sp, h->orgLine);
				}
			}

		} else if (!mStr.strncmp(h->sp, "%add:", 5)) {
			for (i=0L; h->line[i] != EOS; i++) {
				if (mStr.partwcmp(h->line+i, FONTMARKER)) {
					if (h->line[i] == ']') {
						break;
					}
				}
			}
			if (h->line[i] == ']')
				for (i++; isSpace(h->line[i]); i++) ;
			else
				i = 0L;
			while ((i=getitem(st, h->line, i, ADDTIER))) {
				if (!mStr.patmat(st, "[*]"))
					break;
			}
			if (i != 0) {
				if (isThisScoping(st, 0)) {
					do {
						mStr.uS.remblanks(st);
						if (!mStr.patmat(st, "[*]") && !isTerminator(st, NULL, (short *)L"")) {
							if (!isThisScoping(st, 0)) {
								while ((ts=mStr.strpbrk(st, ",.:?!")) != NULL) {
									mStr.strcpy(ts, ts+1);
								}
								mStr.uS.uppercasestr(st, dFnt.Encod, MBF);
							}
							add = addToItems(add, st, NULL);
						}
					} while ((i=getScopedItem(st, h->line, i))) ;
					matchDepTiers(&words, add, ADDTIER, h, 0);
					add = NULL;
				} else {
					mStr.uS.uppercasestr(h->line+i, dFnt.Encod, MBF);
					while ((ts=mStr.strpbrk(h->line+i, ",.:?!")) != NULL) {
						mStr.strcpy(ts, ts+1);
					}
					mStr.remFblanks(h->line+i);
					mStr.uS.remblanks(h->line+i);
					if (h->line[i] != EOS) {
						xml_output(" addressee=\"", NULL);
						xml_output(NULL, h->line+i);
						xml_output("\"", NULL);
					}
				}
			} else {
				mStr.uS.uppercasestr(h->line+i, dFnt.Encod, MBF);
				while ((ts=mStr.strpbrk(h->line+i, ",.:?!")) != NULL) {
					mStr.strcpy(ts, ts+1);
				}
				mStr.remFblanks(h->line+i);
				mStr.uS.remblanks(h->line+i);
				if (h->line[i] != EOS) {
					xml_output(" addressee=\"", NULL);
					xml_output(NULL, h->line+i);
					xml_output("\"", NULL);
				}
			}
		}
	}
	CToW(PUNCTUATION_SET, punctuation, 50);
	cnt = 0;
	groupIndent[0] = EOS;
	changeGroupIndent(groupIndent, 1);
	if (words != NULL) {
		for (w=words; w->NextItem != NULL; w=w->NextItem) ;
		tempst3[0] = EOS;
		for (; w->PrevItem != NULL; w=w->PrevItem) {
			if (w->word[0] == HIDEN_C) {
				cnt++;
				if (tempst3[0] != 0)
					w->sndMark = 2;
				else
					w->sndMark = 1;
				mStr.strcpy(tempst3, w->word);
			}
		}
		if (tempst3[0] != EOS) {
			w->sndMark = 3;
		}
	}
	if (cnt == 1) {
		for (w=words; w != NULL; w=w->NextItem) {
			w->sndMark = 0;
		}
	}

	for (w=words; w != NULL; w=w->NextItem) {
		if (mStr.patmat(w->word, "[\\%fnt: *]")) {
			uS.extractString(tempst3, w->word, "[%fnt: ", ']');
			makeLanguage(tempst3, false);
		} else if (!mStr.strcmp(w->word, "[+ trn]")) {
			xml_output(" included=\"true\"", NULL);
			w->word[0] = EOS;
		} else if (!mStr.strcmp(w->word, "[+ bck]") || !mStr.strcmp(w->word, "[+ bch]")) {
			xml_output(" channel=\"1\"", NULL);
			w->word[0] = EOS;
		}
	}
	if (mStr.strcmp(gLanguage, lLanguage)) {
		if (!mStr.strcmp(lLanguage, "IPA")) {
			xml_output(" lang=\"", NULL);
			xml_output(NULL, gLanguage);
			xml_output("-", NULL);
			xml_output(NULL, lLanguage);
			xml_output("\"", NULL);
		} else {
			xml_output(" lang=\"", NULL);
			xml_output(NULL, lLanguage);
			xml_output("\"", NULL);
		}
	}
	xml_output(">\n", NULL);
	terminatorType[0] = EOS;
	for (w=words; w != NULL; w=w->NextItem) {
		if (w->sndMark == 3) {
			xml_output(NULL, groupIndent);
			xml_output("<g>\n", NULL);
			changeGroupIndent(groupIndent, 1);
		}

		if (w->word[0] == EOS) ;
		else if (w->word[0] == HIDEN_C) {
			if (w->sndMark == 1 || w->sndMark == 2) {
				xml_output(NULL, groupIndent);
				writeMediaInfo(w->word);
				changeGroupIndent(groupIndent, -1);
				xml_output(NULL, groupIndent);
				xml_output("</g>\n", NULL);
				if (w->sndMark == 2) {
					xml_output(NULL, groupIndent);
					xml_output("<g>\n", NULL);
					changeGroupIndent(groupIndent, 1);
				}
			}
		} else if (isTerminator(w->word, terminatorType, groupIndent)) {
		} else if (isSeparatorOrToneMarker(w->word, groupIndent)) {
		} else if (mStr.strcmp(w->word, "&lt;") == 0) {
			xml_output(NULL, groupIndent);
			xml_output("<g", NULL);
			if (closingOverlap(w, false))
				addOverlapTags(NULL);
			if (closingAddressee(w, false) && tempst5[0] != EOS) {
				xml_output(" addressee=\"", NULL);
				xml_output(NULL, tempst5);
				xml_output("\"", NULL);
			}
			xml_output(">\n", NULL);
			changeGroupIndent(groupIndent, 1);
		} else if (mStr.strcmp(w->word, "&gt;") == 0) {
			priList = NULL;
			if (w->morWord != NULL) {
				getMorItem(w->morWord->word, w->morWord->lang, groupIndent, tempst4);
			}
			for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
				if (w->code[codesCnt]->word[0] != EOS) {
					priority = getCodes(w->code[codesCnt],"/", GROUP, groupIndent);
					if (tempst5[0] != EOS)
						priList = addToPriList(priList, priority, tempst5, FALSE, TRUE);
				}
			}
			outputPriorities(groupIndent, FALSE);
			priList = freePriList(priList);
			changeGroupIndent(groupIndent, -1);
			xml_output(NULL, groupIndent);
			xml_output("</g>\n", NULL);
		} else if (isSpecialWord(w->word) && isCodeWordNonEmpty(w)) {
			struct priolst *tp;

			getCodes(w, "", EVENT, groupIndent);
			if (tempst5[0] != EOS) {
				xml_output(NULL, groupIndent);
				xml_output(NULL, tempst5);
			}
			priList = NULL;
			for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
				if (w->code[codesCnt]->word[0] != EOS) {
					priority = getCodes(w->code[codesCnt], "/", EVENT, groupIndent);
					if (tempst5[0] != EOS)
						priList = addToPriList(priList, priority, tempst5, FALSE, TRUE);
				}
			}
			for (tp=priList; tp != NULL; tp=tp->next) {
				if (!tp->isUsed) {
					tp->isUsed = TRUE;
					xml_output(NULL, tp->st);
				}
			}
			priList = freePriList(priList);
			xml_output("</e>\n", NULL);
		} else if (isSpecialWord(w->word) && 
						(w->modWord != NULL || w->phoWord != NULL || w->addWord != NULL)) {
			getCodes(w, "", EVENT, groupIndent);
			xml_output(NULL, groupIndent);
			xml_output(NULL, tempst5);
			xml_output("\n", NULL);
			changeGroupIndent(groupIndent, 1);
			if (w->modWord != NULL)
				getModItem(w->modWord, groupIndent, NULL);
			if (w->phoWord != NULL)
				getPhoItem(w->phoWord, groupIndent, NULL);
			changeGroupIndent(groupIndent, -1);
			xml_output(NULL, groupIndent);
			xml_output("</e>\n", NULL);
		} else if (isCodeWordNonEmpty(w)) {
			priList = NULL;
			if (isWordComposit(w, groupIndent)) {
				for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
					priority = getCodes(w->code[codesCnt], "/", WORDT, groupIndent);
					if (tempst5[0] != EOS)
						priList = addToPriList(priList, priority, tempst5, FALSE, TRUE);
				}
				outputPriorities(groupIndent, FALSE);
				priList = freePriList(priList);
				changeGroupIndent(groupIndent, -1);
				xml_output(NULL, groupIndent);
				xml_output("</wn>\n", NULL);
			} else {
				for (codesCnt=0; codesCnt < w->codesCnt; codesCnt++) {
					priority = getCodes(w->code[codesCnt], "/", WORDT, groupIndent);
					if (tempst5[0] != EOS)
						priList = addToPriList(priList, priority, tempst5, FALSE, TRUE);
				}
				outputPriorities(groupIndent, FALSE);
				priList = freePriList(priList);
				changeGroupIndent(groupIndent, -1);
				xml_output(NULL, groupIndent);
				xml_output("</w>\n", NULL);
			}
		} else if (w->isChange || isSpecialWord(w->word)) {
			if (mStr.patmat(w->word, "[+ *]")) ;
			else if (!mStr.patmat(w->word, "[\\%fnt: *]")) {
				getCodes(w, "/", CODE, groupIndent);
				if (tempst5[0] != EOS) {
					xml_output(NULL, groupIndent);
					xml_output(NULL, tempst5);
					xml_output("\n", NULL);
				}
			}
		} else {
			priList = NULL;
			if (isWordComposit(w, groupIndent)) {
				outputPriorities(groupIndent, FALSE);
				priList = freePriList(priList);
				changeGroupIndent(groupIndent, -1);
				xml_output(NULL, groupIndent);
				xml_output("</wn>\n", NULL);
			} else {
				outputPriorities(groupIndent, FALSE);
				priList = freePriList(priList);
				changeGroupIndent(groupIndent, -1);
				xml_output(NULL, groupIndent);
				xml_output("</w>\n", NULL);
			}
		}
	}
	if (words != NULL) {
		if (terminatorType[0] == EOS) {
			if (spTier != NULL)
				mStr.sprintf(tempst2, "\n%ls%ls", spTier->sp, spTier->orgLine);
			else
				mStr.strcpy(tempst2, "Internal Error.");
			freeAll(40);
		} else {
			xml_output(NULL, groupIndent);
			xml_output(NULL, terminatorType);
			xml_output("\n", NULL);
		}
	}
	for (w=pho; w != NULL; w=w->NextItem) {
		getPhoItem(w, groupIndent, NULL);
	}
	for (w=mod; w != NULL; w=w->NextItem) {
		getModItem(w, groupIndent, NULL);
	}
	for (w=mor; w != NULL; w=w->NextItem) {
		if (mStr.strchr(w->word,'^') != NULL) {
			xml_output(NULL, groupIndent);
			xml_output("<gm>\n", NULL);
			changeGroupIndent(groupIndent, 1);
			getMorItem(w->word, w->lang, groupIndent, NULL);
			changeGroupIndent(groupIndent, -1);
			xml_output(NULL, groupIndent);
			xml_output("</gm>\n", NULL);
		} else
			getMorItem(w->word, w->lang, groupIndent, NULL);
	}

	for (w=words; w != NULL; w=w->NextItem) {
		if (mStr.patmat(w->word, "[+ *]")) {
			uS.extractString(st, w->word, "[+ ", ']');
			xml_output(NULL, groupIndent);
			xml_output("<k type=\"other\">", NULL);
			xml_output(NULL, st);
			xml_output("</k>\n", NULL);
		}
	}
	for (h=headers; h != NULL; h=h->NextUtt) {
		if (h->sp[0] == '%') {
			uS.extractString(st, h->sp, "%", ':');
			if (!mStr.strcmp(st, "add") || !mStr.strcmp(st, "mor") || !mStr.strcmp(st, "pho") ||
				!mStr.strcmp(st, "mod")) 
				;
			else {
				for (i=0L; h->line[i] != EOS; i++) {
					if (mStr.partwcmp(h->line+i, FONTMARKER)) {
						long o=i;

						for (; h->line[i] != ']' && h->line[i] != EOS; i++) ;
						if (h->line[i] == ']') {
							uS.extractString(tempst3, h->line+o, "[%fnt: ", ']');
							makeLanguage(tempst3, false);
							for (i++; isSpace(h->line[i]); i++) ;
							if (o != i)
								mStr.strcpy(h->line+o, h->line+i);
						}
					}
				}
				xml_output(NULL, groupIndent);
				xml_output("<a", NULL);
				tempst6[0] = 0;
				for (ts=h->line; *ts != 0; ts++) {
					if (*ts == HIDEN_C) {
						short *ts2;

						ts2 = mStr.strchr(ts+1, HIDEN_C);
						if (ts2 == NULL)
							break;
						mStr.strncpy(tempst6, ts, ts2-ts+1);
						tempst6[ts2-ts+1] = 0;
						mStr.strcpy(ts, ts2+1);
						break;
					}
				}

				if (mStr.strcmp(gLanguage, lLanguage)) {
					if (!mStr.strcmp(lLanguage, "IPA")) {
						xml_output(" lang=\"", NULL);
						xml_output(NULL, gLanguage);
						xml_output("-", NULL);
						xml_output(NULL, lLanguage);
						xml_output("\"", NULL);
					} else {
						xml_output(" lang=\"", NULL);
						xml_output(NULL, lLanguage);
						xml_output("\"", NULL);
					}
				}
				if (!mStr.strcmp(st, "act")) {
					xml_output(" type=\"actions\">", NULL);
				} else if (!mStr.strcmp(st, "alt")) {
					xml_output(" type=\"alternative\">", NULL);
				} else if (!mStr.strcmp(st, "cod")) {
					xml_output(" type=\"gpcoding\">", NULL);
				} else if (!mStr.strcmp(st, "coh")) {
					xml_output(" type=\"cohesion\">", NULL);
				} else if (!mStr.strcmp(st, "com")) {
					xml_output(" type=\"comments\">", NULL);
				} else if (!mStr.strcmp(st, "def")) {
					xml_output(" type=\"SALT\">", NULL);
				} else if (!mStr.strcmp(st, "eng")) {
					xml_output(" type=\"english_translation\">", NULL);
				} else if (!mStr.strcmp(st, "err")) {
					xml_output(" type=\"errcoding\">", NULL);
				} else if (!mStr.strcmp(st, "exp")) {
					xml_output(" type=\"explanation\">", NULL);
				} else if (!mStr.strcmp(st, "fac")) {
					xml_output(" type=\"facial\">", NULL);
				} else if (!mStr.strcmp(st, "flo")) {
					xml_output(" type=\"flow\">", NULL);
				} else if (!mStr.strcmp(st, "gls")) {
					xml_output(" type=\"target_gloss\">", NULL);
				} else if (!mStr.strcmp(st, "gpx")) {
					xml_output(" type=\"gesture\">", NULL);
				} else if (!mStr.strcmp(st, "int")) {
					xml_output(" type=\"intonation\">", NULL);
				} else if (!mStr.strcmp(st, "lan")) {
					xml_output(" type=\"language\">", NULL);
				} else if (!mStr.strcmp(st, "par")) {
					xml_output(" type=\"paralinguistics\">", NULL);
				} else if (!mStr.strcmp(st, "sit")) {
					xml_output(" type=\"situation\">", NULL);
				} else if (!mStr.strcmp(st, "spa")) {
					xml_output(" type=\"speech_act\">", NULL);
				} else if (!mStr.strcmp(st, "syn")) {
					xml_output(" type=\"syntactic_structure\">", NULL);
				} else if (!mStr.strcmp(st, "tim")) {
					xml_output(" type=\"time_stamp\">", NULL);
				} else {
					SymboLize(st);
					xml_output(" type=\"other\"><symbol>", NULL);
					xml_output(NULL, st);
					xml_output("</symbol>", NULL);
				}
				xml_output(NULL, h->line);
				if (tempst6[0] != 0)
					writeMediaInfo(tempst6);
				xml_output("</a>\n", NULL);
			}
		}
	}
	for (w=words; w != NULL; w=w->NextItem) {
		if (w->word[0] == HIDEN_C) {
			xml_output(NULL, groupIndent);
			writeMediaInfo(w->word);
		}
	}

	xml_output("</u>\n", NULL);
	for (h=headers; h != NULL; h=h->NextUtt) {
		if (h->sp[0] == '@') {
			if (mStr.partwcmp(h->sp, FONTHEADER)) {
				mStr.strcpy(tempst3, h->line);
				makeLanguage(tempst3, false);
			} else {
				mStr.uS.remblanks(h->sp);
				SymboLize(h->sp);
				i = mStr.strlen(h->sp);
				if (h->sp[i-1] == ':')
					h->sp[i-1] = EOS;
				xml_output("<comment>\n", NULL);
				xml_output("    ", NULL);
				xml_output(NULL, h->sp+1);
				xml_output(": ", NULL);
				xml_output(NULL, h->line);
				xml_output("\n", NULL);
				xml_output("</comment>\n", NULL);
			}
		}
	}
	words = freeItems(words);
	mor = freeItems(mor);
	pho = freeItems(pho);
	mod = freeItems(mod);
	add = freeItems(add);
}

static char xmlDate(short *st) {
	long i;

	for (i=0L; isSpace(st[i]) || st[i] == '\n'; i++) ;
	if (iswdigit(st[i]) && iswdigit(st[i+1]) && iswdigit(st[i+2]) && iswdigit(st[i+3]))
		return(true);
	else
		return(false);
}

static void makeXMLDate(short *oSt) {
	int date, month, year;
	char dash;
	short *t, *st, dFiller[2], mFiller[2], yFiller[3];

	dash = false;
	st = oSt;
	do {
		t = mStr.strchr(st, '-');
		if (t == NULL) {
			mStr.sprintf(tempst2, ": %ls", oSt);
			freeAll(41);
		}
		*t = EOS;
		date = mStr.atoi(st);
		*t = '-';
		st = t + 1;
		t = mStr.strchr(st, '-');
		if (t == NULL) {
			mStr.sprintf(tempst2, ": %ls", oSt);
			freeAll(41);
		}
		*t = EOS;
		month = mStr.monthToNum(st);
		*t = '-';
		st = t + 1;
		year = mStr.atoi(st);
		if ((date < 1 || date > 31) || (month < 1 || month > 12) || (year < 1)) {
			mStr.sprintf(tempst2, ": %ls", oSt);
			freeAll(41);
		}
		if (year < 100)
			mStr.strcpy(yFiller, "19");
		else
			yFiller[0] = EOS;
		if (month < 10)
			mStr.strcpy(mFiller, "0");
		else
			mFiller[0] = EOS;
		if (date < 10)
			mStr.strcpy(dFiller, "0");
		else
			dFiller[0] = EOS;
		mStr.sprintf(tempst5, "%ls%d-%ls%d-%ls%d", yFiller, year, mFiller, month, dFiller, date);
		if (dash) {
			xml_output(" DateEnd=\"", NULL);
			xml_output(NULL, tempst5);
			xml_output("\"", NULL);
		} else {
			xml_output(" DateStart=\"", NULL);
			xml_output(NULL, tempst5);
			xml_output("\"", NULL);
		}
		for (; iswdigit(*st); st++) ;
		for (; isSpace(*st) || *st == '\n'; st++) ;
		if (*st == '-') {
			dash = true;
			st++;
		} else if ((st[0] == 't' || st[0] == 'T') && (st[1] == 'o' || st[1] == 'O')) {
			dash = true;
			st += 2;
		} else
			break;
		for (; isSpace(*st) || *st == '\n'; st++) ;
	} while (*st != EOS) ;
}

static char xmlAge(short *st) {
	long i;

	for (i=0L; st[i] != EOS; i++) {
		if (st[i] == ';')
			return(false);
	}
	return(true);
}

static void makeXMLAge(short *sp, short *st, long i, char mc, char ec) {
	long s;
	int y, m, d;
	char isY, isM, isD, dash, t;

	isY = false;
	isM = false;
	isD = false;
	dash = false;
	y = 0;
	m = 0;
	d = 0;
	while (st[i] != ec) {
		while (isSpace(st[i]) || st[i] == '\n') i++;
		if (st[i] == ec)
			break;
		s = i;
		while (iswdigit(st[i])) i++;
		if (st[i] == ';') {
			isY = true;
			st[i] = EOS;
			y = mStr.atoi(st+s);
			st[i] = ';';
			i++;
		} else if (st[i] == mc && isM == false) {
			isM = true;
			st[i] = EOS;
			m = mStr.atoi(st+s);
			st[i] = mc;
			i++;
		} else if (isY && isM) {
			isD = true;
			t = st[i];
			st[i] = EOS;
			d = mStr.atoi(st+s);
			st[i] = t;
		}
		if (st[i] == '-') {
			if (m == 0 && d == 0 && !isM && !isD)
				mStr.sprintf(tempst5, "P%dY", y);
			else if (d == 0 && !isD)
				mStr.sprintf(tempst5, "P%dY%dM", y, m);
			else
				mStr.sprintf(tempst5, "P%dY%dM%dD", y, m, d);
			addToSpeaker(sp, "age", tempst5);
			dash = true;
			isY = false; isM = false; isD = false;
			y = 0; m = 0; d = 0;
			i++;
		}
		if (st[i] == ec)
			break;
		if ((st[i] == 't' || st[i] == 'T') && (st[i+1] == 'o' || st[i+1] == 'O')) {
			if (m == 0 && d == 0 && !isM && !isD)
				mStr.sprintf(tempst5, "P%dY", y);
			else if (d == 0 && !isD)
				mStr.sprintf(tempst5, "P%dY%dM", y, m);
			else
				mStr.sprintf(tempst5, "P%dY%dM%dD", y, m, d);
			addToSpeaker(sp, "age", tempst5);
			dash = true;
			isY = false; isM = false; isD = false;
			y = 0; m = 0; d = 0;
			st[i] = '-';
			i++;
			mStr.strcpy(st+i, st+i+1);
		} else if (!isSpace(st[i]) && !iswdigit(st[i]) && st[i] != '\n') {
			mStr.sprintf(tempst2, ": %ls", st);
			freeAll(42);
		}
	}
	if (!isY && !isM && !isD && dash) {
		mStr.sprintf(tempst2, ": %ls", st);
		freeAll(42);
	}
	if (m == 0 && d == 0 && !isM && !isD)
		mStr.sprintf(tempst5, "P%dY", y);
	else if (d == 0 && !isD)
		mStr.sprintf(tempst5, "P%dY%dM", y, m);
	else
		mStr.sprintf(tempst5, "P%dY%dM%dD", y, m, d);
	if (dash) 
		addToSpeaker(sp, "ageUntil", tempst5);
	else
		addToSpeaker(sp, "age", tempst5);
}

static void finishTitle(void) {
	char  alreadyID;
	short sp[SPEAKERLEN], *s, *e;
	struct tiers *h;

	tempst1[0] = EOS;
	alreadyID = false;
	for (h=headers; h != NULL; h=h->NextUtt) {
		mStr.strcpy(tempst2, h->sp);
		mStr.uS.uppercasestr(tempst2, dFnt.Encod, MBF);
		CHATtoXML_CleanTierNames(tempst2);
		if (mStr.partcmp(tempst2,"@AGE OF ",false)) {
			uS.extractString(sp, tempst2, "@AGE OF ", ':');
			if (!xmlAge(h->line))
				makeXMLAge(sp, h->line, 0, '.', EOS);
			else
				addToSpeaker(sp, "age", h->line);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@BIRTH OF ",false)) {
			uS.extractString(sp, tempst2, "@BIRTH OF ", ':');
			addToSpeaker(sp, "birth", h->line);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@EDUCATION OF ",false)) {
			uS.extractString(sp, tempst2, "@EDUCATION OF ", ':');
			addToSpeaker(sp, "education", h->line);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@GROUP OF ",false)) {
			uS.extractString(sp, tempst2, "@GROUP OF ", ':');
			addToSpeaker(sp, "group", h->line);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@LANGUAGE OF ",false)) {
			uS.extractString(sp, tempst2, "@LANGUAGE OF ", ':');
			addToSpeaker(sp, "language", h->line);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@SES OF ",false)) {
			uS.extractString(sp, tempst2, "@SES OF ", ':');
			addToSpeaker(sp, "SES", h->line);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@SEX OF ",false)) {
			uS.extractString(sp, tempst2, "@SEX OF ", ':');
			mStr.uS.lowercasestr(h->line, dFnt.Encod, MBF);
			addToSpeaker(sp, "sex", h->line);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@SCRIPT:",false)) {
			mStr.strcpy(tempst2, h->line);
			mStr.uS.uppercasestr(tempst2, dFnt.Encod, MBF);
			mStr.strcpy(gScript, convertName(tempst2));
			if (gScript[0] == EOS) {
				mStr.sprintf(tempst2, ": %ls", h->line);
				freeAll(10);
			}
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@LANGUAGE:",false)) {
			makeLanguage(h->line, true);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@ID:",false)) {
			if (alreadyID) ;
			else {
				alreadyID = true;
				s = h->line;
				if ((e=mStr.strchr(s, '|')) != NULL) {
					*e = EOS;
					makeLanguage(s, true);
					s = e + 1;
					if ((e=mStr.strchr(s, '|')) != NULL) {
						*e = EOS;
						xml_output(" Corpus=\"", NULL);
						xml_output(NULL, s);
						xml_output("\"", NULL);
					}
				}
			}
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@COMMENT:",false)) {
			mStr.strcat(tempst1,  h->line);
			mStr.strcat(tempst1,  " ");
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@DATE:",false)) {
			if (!xmlDate(h->line))
				makeXMLDate(h->line);
			else {
				xml_output(" DateStart=\"", NULL);
				xml_output(NULL, h->line);
				xml_output("\"", NULL);
			}
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@SITUATION:",false)) {
			xml_output(" Situation=\"", NULL);
			xml_output(NULL, h->line);
			xml_output("\"", NULL);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@CODER:",false)) {
			xml_output(" Coder=\"", NULL);
			xml_output(NULL, h->line);
			xml_output("\"", NULL);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@FONT:",false)) {
			makeLanguage(h->line, false);
			h->sp[0] = EOS;
		} else if (mStr.partcmp(tempst2,"@WARNING:",false)) {
			xml_output(" Warning=\"", NULL);
			xml_output(NULL, h->line);
			xml_output("\"", NULL);
			h->sp[0] = EOS;
		}
	}
	if (tempst1[0] != EOS) {
		xml_output(" Comment=\"", NULL);
		xml_output(NULL, tempst1);
		xml_output("\"", NULL);
	}
	if (gLanguage[0] == EOS) {
		if (uLanguage[0] != EOS) {
			mStr.strcpy(gLanguage, uLanguage);
			mStr.strcpy(lLanguage, uLanguage);
		} else {
#ifndef _IS_UNICODE
			freeAll(43);
#else
			lLanguage[0] = EOS;
#endif
		}
	}
	if (!mStr.strcmp(lLanguage, "IPA")) {
		xml_output(" Lang=\"", NULL);
		xml_output(NULL, gLanguage);
		xml_output("-", NULL);
		xml_output(NULL, lLanguage);
		xml_output("\"", NULL);
	} else {
		xml_output(" Lang=\"", NULL);
		xml_output(NULL, lLanguage);
		xml_output("\"", NULL);
	}
	if (gScript[0] != EOS) {
		xml_output(" Script=\"", NULL);
		xml_output(NULL, gScript);
		xml_output("\"", NULL);
	} else if (dFnt.Encod == 1)
		xml_output(" Script=\"ja\"", NULL);
	else if (dFnt.Encod == 2)
		xml_output(" Script=\"zh\"", NULL);
	else if (dFnt.Encod == 3)
		xml_output(" Script=\"ko\"", NULL);
	else 
		xml_output(" Script=\"en\"", NULL);
	xml_output(">\n", NULL);
}

static void finishParticipants(void) {
	struct participants *t;

	xml_output("<Participants>\n", NULL);

	for (t=speakers; t != NULL; t=t->NextPart) {
		xml_output("    <participant", NULL);
		xml_output(NULL, t->line);
		xml_output("/>\n", NULL);
	}

	xml_output("</Participants>\n", NULL);
}

static void finishHeaders(void) {
	long i;
	struct tiers *h;

	for (h=headers; h != NULL; h=h->NextUtt) {
		if (h->sp[0] != EOS) {
			mStr.uS.remblanks(h->sp);
			SymboLize(h->sp);
			i = mStr.strlen(h->sp);
			if (h->sp[i-1] == ':')
				h->sp[i-1] = EOS;
			xml_output("<comment>\n", NULL);
			xml_output("    ", NULL);
			xml_output(NULL, h->sp+1);
			xml_output(": ", NULL);
			xml_output(NULL, h->line);
			xml_output("\n", NULL);
			xml_output("</comment>\n", NULL);
		}
	}
}

static void CHATtoXML_getpart(short *line) {
	short *s, *e, t, wc;
	short sp[SPEAKERLEN];
	long ln = 0L;
	short cnt = 0;

	for (; *line && (*line == ' ' || *line == '\t'); line++) ;
	s = line;
	sp[0] = EOS;
	while (*s) {
		if (*line == (short)','  || *line == (short)' '  ||
			*line == (short)'\t' || *line == (short)'\n' || *line == EOS) {
			wc = (short)' ';
			e = line;
			for (; *line == (short)' ' || *line == (short)'\t' || *line == (short)'\n'; line++) {
				if (*line == (short)'\n') ln += 1L;
			}
			if (*line != (short)',' && *line != EOS) line--;
			else wc = (short)',';
			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == (short)',') {
					addToSpeaker(sp, "role", s);
				} else if (cnt == 1) {
					addToSpeaker(sp, "name", s);
				} else if (cnt == 0) {
					mStr.strcpy(sp, s);
					mStr.uS.uppercasestr(sp, dFnt.Encod, false);
					addToSpeaker(sp, "id", sp);
				}
				*e = t;
				if (wc == (short)',') {
					if (cnt == 0) {
						freeAll(8);
					}
					cnt = -1;
				}
				for (line++; *line==' ' || *line=='\t' || *line=='\n' || *line==','; line++) {
					if (*line == ',') {
						if (cnt == 0) {
							freeAll(8);
						}
						cnt = -1;
					} else if (*line == '\n') ln += 1L;
				}
			} else {
				for (line=e; *line; line++) {
					if (*line == '\n') ln -= 1L;
				}
				if (cnt == 0) {
					freeAll(8);
				} else /* if (cnt == 2) */ {
					t = *e;
					*e = EOS;
					addToSpeaker(sp, "role", s);
					*e = t;
				}
				for (line=e; *line; line++) {
					if (*line == '\n') ln += 1L;
				}
			}
			if (cnt == 2) cnt = 0;
			else cnt++;
			s = line;
		} else line++;
	}
}

static void init(char *lang) {
	UEncoding = UNOT;
	headers = NULL;
	xmlSt = NULL;
	xmlStLen = 0L;
	xmlStMax = 20000L;
	spTier = NULL;
	words = NULL;
	mor = NULL;
	pho = NULL;
	mod = NULL;
	add = NULL;
	overlaps = NULL;
	cOverlap = NULL;
	speakers = NULL;
	lineno = 1L;
	tlineno = 0L;
	isWarning = false;
	MBF = false;
	isCainFile = false;
	getrestline = 1;
	skipgetline = FALSE;
	if (lang == NULL)
		uLanguage[0] = EOS;
	else
		mStr.strcpy(uLanguage, lang);
}

char *ChatToXML(short *chatS,
				char isCheck,
				char *oFname,
				char *lang,
				char UEncod,
				char *isWarn,
				FILE *warnFP) {
	short *c, t;
	char isTitleMade;
	char isFirstSpeaker;
	unsigned short wc;
	static int jmp_return;

	errNum = 0;
	jmp_return = setjmp(my_jmp_buf);
	if (jmp_return != 0) {
		return(NULL);
	}
	init(lang);
	isJustCheck = isCheck;
	xmlSt = (char *)malloc(xmlStMax);
	if (xmlSt == NULL)
		freeAll(1);
	chatSt = chatS;
	warningFP = warnFP;
	if (oFname == NULL)
		oldfname = "";
	else
		oldfname = oFname;
	UEncoding = UEncod;
	spTier = NULL;
	if (chatS == NULL) {
		return(NULL);
	} else {
		t = 0;
		wc = lgetc(&t);
	}
	createOverlapMap();
	cOverlap = overlaps;
	chatSt = (short *)chatS;
	xml_output("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n", NULL);
//	xml_output("<?xml-stylesheet server-config=\"/sheet.xml\" href=\"chat.xsl\" type=\"text/xsl\" ?>\n", NULL);
	xml_output("<CHAT", NULL);
	mStr.strcpy(tempst2, oldfname);
	c = mStr.strrchr(tempst2,'.');
	if (c != NULL)
		*c = 0;
	xml_output(" xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"", NULL);
	xml_output("talkbank.xsd\"", NULL);
	xml_output(" Title=\"", NULL);
	xml_output(NULL, tempst2);
	xml_output("\"", NULL);
	gLanguage[0] = EOS;
	lLanguage[0] = EOS;
	gScript[0] = EOS;
	isTitleMade = false;
	isFirstSpeaker = false;
	lineno = 1L;
	tlineno = 0L;
	currentatt = 0;
	currentchar = (char)lgetc(&currentatt);
	while (getutterance()) {
		mStr.strcpy(tempst2,wutterance.speaker);
		mStr.uS.uppercasestr(tempst2, dFnt.Encod, MBF);
		CHATtoXML_CleanTierNames(tempst2);
		mStr.uS.remblanks(wutterance.line);
		if (mStr.partcmp(wutterance.speaker,"*",false)) {
			isFirstSpeaker = true;
		} else if (!isFirstSpeaker && mStr.partcmp(tempst2,PARTICIP,false))
			CHATtoXML_getpart(wutterance.line);
		if (!isTitleMade) {
			if (isFirstSpeaker) {
				finishTitle();
				finishParticipants();
				finishHeaders();
				headers = freeHeaders(headers);
				freeSpeakers();
				isTitleMade = true;
			} else if (!mStr.partcmp(tempst2,PARTICIP,false) && !mStr.partcmp(tempst2,"@BEGIN",false) &&
						!mStr.partcmp(tempst2,"@END",false) && !mStr.partcmp(tempst2,"@FILENAME:",false) &&
						!mStr.partcmp(tempst2,"@COLOR WORDS:",false)) {
				headers = addToHeaders(headers, wutterance.speaker, wutterance.line);
				if (headers == NULL)
					return(NULL);
			}
		}
		if (isFirstSpeaker) {
			if (mStr.partcmp(wutterance.speaker,"*",false)) {
				writeUttCluster();
				headers = freeHeaders(headers);
				mStr.uS.uppercasestr(wutterance.speaker, dFnt.Encod, MBF);
				headers = addToHeaders(headers, wutterance.speaker, wutterance.line);
				if (headers == NULL)
					return(NULL);
			} else if (!mStr.partcmp(tempst2,PARTICIP,false) && !mStr.partcmp(tempst2,"@BEGIN",false) &&
						!mStr.partcmp(tempst2,"@END",false) && !mStr.partcmp(tempst2,"@FILENAME:",false) &&
						!mStr.partcmp(tempst2,"@COLOR WORDS:",false)) {
				headers = addToHeaders(headers, wutterance.speaker, wutterance.line);
				if (headers == NULL)
					return(NULL);
			}
		}
	}
	writeUttCluster();
	xml_output("</CHAT>\n", NULL);
	freeAll(0);

	if (isWarn != NULL)
		*isWarn = isWarning;
	if (xmlStLen >= xmlStMax-2) {
		char *t;
		t = xmlSt;
		xmlSt = (char *)malloc(xmlStLen+4);
		if (xmlSt == NULL)
			freeAll(1);
		memcpy(xmlSt, t, xmlStLen);
		free(t);
	}
	xmlSt[xmlStLen++] = 0;
	xmlSt[xmlStLen++] = 0;
	return(xmlSt);
}

/*	 strings manipulation routines begin   */
short cChatToXMLStr::ismultibyte(char *org, short pos, short encod) {
#ifndef _IS_UNICODE
	return(my_CharacterByteType(org, pos, encod));
#else
	return(0);
#endif
}

short cChatToXMLStr::ismultibyte(short *org, short pos, short encod) {
#ifndef _IS_UNICODE
	#if defined(_MAC_CODE)
		WToC(org, tempchar, UTTLINELEN);
		return(CharacterByteType(tempchar, pos, encod));
	#elif defined(_WIN32)
		return(my_wCharacterByteType(org, pos, encod));
	#endif
#else
		return(0);
#endif
}

int cChatToXMLStr::strcmp(const short *st1, const short *st2) {
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (*st1 > *st2)
		return(1);
	else
		return(-1);	
}

int cChatToXMLStr::strncmp(const short *st1, const short *st2, size_t len) {
	for (; *st1 == *st2 && *st2 != EOS && len > 0; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0)
		return(0);
	else if (*st1 > *st2)
		return(1);
	else
		return(-1);	
}

size_t cChatToXMLStr::strlen(const short *st) {
	size_t len = 0;

	for (; *st != EOS; st++)
		len++;
	return(len);
}

short *cChatToXMLStr::strcpy(short *des, const short *src) {
	short *org = des;

	while (*src != EOS) {
		*des++ = *src++;
	}
	*des = EOS;
	return(org);
}

short *cChatToXMLStr::strncpy(short *des, const short *src, size_t num) {
	long i = 0L;

	while (i < num && src[i] != EOS) {
		des[i] = src[i];
		i++;
	}
	des[i] = EOS;
	return(des);
}

short *cChatToXMLStr::strcat(short *des, const short *src) {
	long i;

	i = cChatToXMLStr::strlen(des);
	while (*src != EOS) {
		des[i] = *src++;
		i++;
	}
	des[i] = EOS;
	return(des);
}

short *cChatToXMLStr::strchr(short *src, int c) {
	for (; *src != c && *src != EOS; src++) ;
	if (*src == EOS)
		return(NULL);
	else
		return(src);
}

short *cChatToXMLStr::strrchr(short *src, int c) {
	long i;
	
	i = cChatToXMLStr::strlen(src);
	for (; i >= 0 && src[i] != c; i--) ;
	if (src[i] == c)
		return(src+i);
	else
		return(NULL);
}

short *cChatToXMLStr::strpbrk(short *src, const short *cs) {
	long i, j;

	for (i=0L; src[i] != EOS; i++) {
		for (j=0L; cs[j] != EOS; j++) {
			if (src[i] == cs[j])
				return(src+i);
		}
	}
	return(NULL);
}

int cChatToXMLStr::atoi(const short *s) {
	return(watoi((wchar_t*)s));
}

long cChatToXMLStr::atol(const short *s) {
	return(watol((wchar_t*)s));
}

short *cChatToXMLStr::itoa(int num, short *st, int size) {
	mStr.sprintf(st, "%d", num);
	return(st);
}

/* partcmp(st1,st2) does a partial comparisone of string "st2" to string 
   "st1". The match is sucessful if the string "st2" completely matches the
   beginning part of string "st1". i.e. st2 = "first", and st1 = "first part".

   WARNING: This function is for tier names ONLY!!!!
			Use "partwcmp" function to compaire strings in general
*/
char cChatToXMLStr::partcmp(short *st1, const short *st2, char pat_match) {// st1- full, st2- part
	if (pat_match) {
		int i;
		if (*st1 == *st2) {
			for (i=mStr.strlen(st1)-1; i >= 0 && (st1[i] == (short)' ' || st1[i] == (short)'\t'); i--) ;
			if (st1[i] == (short)':') st1[i] = EOS;
			else st1[i+1] = EOS;
			return(mStr.patmat(st1+1, st2+1));
		} else
			return(FALSE);
	}
	for (; *st1 == *st2 && *st2 != (short)EOS; st1++, st2++) ;
	if (*st2 == ':') return((char)!*st1);
	else return((char)!*st2);
}

/* partwcmp(st1,st2) does a partial comparisone of string "st2" to string 
   "st1". The match is sucessful if the string "st2" completely matches the
   beginning part of string "st1". i.e. st2 = "first", and st1 = "first part".
*/
char cChatToXMLStr::partwcmp(short *st1, short *st2) {
	for (; *st1 == *st2 && *st2 != (short)EOS; st1++, st2++) ;
	return((char)!*st2);
}

/* patmat(s, pat) does pattern matching of pattern "pat" to string "s".
   "pat" may contain meta characters "_", "*" and "\". "_" means match
   any one character, "*" means match zero or more characters, "\" is
   used to specify meta characters as litteral characters. The value returned
   is 1 if there is a match, and 0 otherwise.
*/
char cChatToXMLStr::patmat(short *s, const short *pat) {
	register int j, k;
	int n, m, t, l;
	short *lf;

	if (s[0] == (short)EOS) return(pat[0] == s[0]);
	l = mStr.strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if ((s[j] == (short)'(' || s[j] == (short)')') && *wutterance.speaker != (short)'%') {
			if (pat[k] != s[j]) {
				k--;
				continue;
			}
		} else if (pat[k] == (short)'\\') {
			if (s[j] != pat[++k]) break;
		} else if (pat[k] == (short)'_') {
			if (iswspace(s[j]))
				return(FALSE);
			if (s[j+1] && pat[k+1])
				continue; 					// any character
			else {
				if (pat[k+1] == s[j+1]) {
					s = s+l;
					if (lf != s) {
						while (lf != s) *lf++ = (short)' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			}
		} else if (pat[k] == (short)'*') {			// wildcard
			k++; t = j;
			if (pat[k] == (short)'\\') k++;
f1:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					s = s+l;
					if (lf != s) while (lf != s) *lf++ = (short)' ';
					return(TRUE);
				} else {
					for (; t < j; t++) {
						if (mStr.uS.ismorfchar(s,t, dFnt.Encod, MORPHS, MBF))
							return(FALSE);
					}
					return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== (short)'(' || s[m]== (short)')') && *wutterance.speaker != (short)'%')
					n--;
				else
				if (pat[n] == (short)'*') break;
				else
				if (pat[n] == (short)'_') {
					if (iswspace(s[m])) {
						j++;
						goto f1;
					}
				} else if (pat[n] == (short)'\\') {
					if (!pat[++n]) return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f1;
					}
				} else if (s[m] != pat[n]) {
					j++;
					goto f1;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f1;
			}
		}

		if (s[j] != pat[k]) {
			return(FALSE);
		}
	}
	if (pat[k] == s[j]) {
		s = s+l;
		if (lf != s) while (lf != s) *lf++ = (short)' ';
		return(TRUE);
	} else return(FALSE);
}

int cChatToXMLStr::monthToNum(short *s) {
	if (!mStr.strcmp(s,"JAN") || !mStr.strcmp(s,"jan") || !mStr.strcmp(s,"Jan")) return(1);
	else
	if (!mStr.strcmp(s,"FEB") || !mStr.strcmp(s,"feb") || !mStr.strcmp(s,"Feb")) return(2);
	else
	if (!mStr.strcmp(s,"MAR") || !mStr.strcmp(s,"mar") || !mStr.strcmp(s,"Mar")) return(3);
	else
	if (!mStr.strcmp(s,"APR") || !mStr.strcmp(s,"apr") || !mStr.strcmp(s,"Apr")) return(4);
	else
	if (!mStr.strcmp(s,"MAY") || !mStr.strcmp(s,"may") || !mStr.strcmp(s,"May")) return(5);
	else
	if (!mStr.strcmp(s,"JUN") || !mStr.strcmp(s,"jun") || !mStr.strcmp(s,"Jun")) return(6);
	else
	if (!mStr.strcmp(s,"JUL") || !mStr.strcmp(s,"jul") || !mStr.strcmp(s,"Jul")) return(7);
	else
	if (!mStr.strcmp(s,"AUG") || !mStr.strcmp(s,"aug") || !mStr.strcmp(s,"Aug")) return(8);
	else
	if (!mStr.strcmp(s,"SEP") || !mStr.strcmp(s,"sep") || !mStr.strcmp(s,"Sep")) return(9);
	else
	if (!mStr.strcmp(s,"OCT") || !mStr.strcmp(s,"oct") || !mStr.strcmp(s,"Oct")) return(10);
	else
	if (!mStr.strcmp(s,"NOV") || !mStr.strcmp(s,"nov") || !mStr.strcmp(s,"Nov")) return(11);
	else
	if (!mStr.strcmp(s,"DEC") || !mStr.strcmp(s,"dec") || !mStr.strcmp(s,"Dec")) return(12);
	else return(0);
}

int cChatToXMLStr::strcmp(const short *st1, const char *st2) {
	for (; *st1 == (short)*st2 && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (*st1 > (short)*st2)
		return(1);
	else
		return(-1);	
}

int cChatToXMLStr::strncmp(const short *st1, const char *st2, size_t len) {
	for (; *st1 == (short)*st2 && *st2 != EOS && len > 0; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0)
		return(0);
	else if (*st1 > (short)*st2)
		return(1);
	else
		return(-1);	
}

size_t cChatToXMLStr::strlen(const char *st) {
	size_t len = 0;

	for (; *st != EOS; st++)
		len++;
	return(len);
}

short *cChatToXMLStr::strcpy(short *des, const char *src) {
	short *org = des;

	while (*src != EOS) {
		*des++ = (short)*src++;
	}
	*des = EOS;
	return(org);
}

short *cChatToXMLStr::strncpy(short *des, const char *src, size_t num) {
	long i = 0L;

	while (i < num && src[i] != EOS) {
		des[i] = (short)src[i];
		i++;
	}
	des[i] = EOS;
	return(des);
}

short *cChatToXMLStr::strcat(short *des, const char *src) {
	long i;

	i = cChatToXMLStr::strlen(des);
	while (*src != EOS) {
		des[i] = (short)*src++;
		i++;
	}
	des[i] = EOS;
	return(des);
}

short *cChatToXMLStr::strpbrk(short *src, const char *cs) {
	long i, j;

	for (i=0L; src[i] != EOS; i++) {
		for (j=0L; cs[j] != EOS; j++) {
			if (src[i] == (short)cs[j])
				return(src+i);
		}
	}
	return(NULL);
}

char cChatToXMLStr::ismorfchar(short *org, int pos, short Script, char *morfsList, char MBC) {
	register char *morf;
	register char chr, nextChr;

	chr = org[pos];
	nextChr = org[pos+1];
	if (MBC) {
		if (ismultibyte(org, (short)pos, Script) != 0)
			return(FALSE);
	}
	for (morf=morfsList; *morf; morf++) {
		if (*morf == chr && nextChr != '0')
			return(TRUE);
	}
	return(FALSE);
}

int cChatToXMLStr::sprintf(short *st, const char *cFormat, ...) {
	short format[256];
	va_list args;

	CToW(cFormat, format, 256);
	va_start(args, cFormat); 	// prepare the arguments
#ifdef _WIN32 
	int t = vswprintf((wchar_t *)st, (wchar_t *)format, args);
#else
	int t = vswprintf((wchar_t *)st, (size_t)256, (wchar_t *)format, args);
#endif
	va_end(args);				// clean the stack
	return(t);
}

char cChatToXMLStr::partcmp(short *st1, const char *st2, char pat_match) {
	if (pat_match) {
		int i;
		if (*st1 == (short)*st2) {
			for (i=mStr.strlen(st1)-1; i >= 0 && (st1[i] == (short)' ' || st1[i] == (short)'\t'); i--) ;
			if (st1[i] == (short)':') st1[i] = EOS;
			else st1[i+1] = EOS;
			return(mStr.patmat(st1+1, st2+1));
		} else
			return(FALSE);
	}
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	if (*st2 == ':') return((char)!*st1);
	else return((char)!*st2);
}

char cChatToXMLStr::partwcmp(short *st1, char *st2) { // st1- full, st2- part 
	for (; *st1 == (short)*st2 && *st2 != EOS; st1++, st2++) ;
	return(!*st2);
}

char cChatToXMLStr::patmat(short *s, const char *pat) {
	register int j, k;
	int n, m, t, l;
	short *lf;

	if (s[0] == (short)EOS) return(s[0] == (short)pat[0]);
	l = mStr.strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if ((s[j] == (short)'(' || s[j] == (short)')') && *wutterance.speaker != (short)'%') {
			if (s[j] != (short)pat[k]) {
				k--;
				continue;
			}
		} else if (pat[k] == '\\') {
			if (s[j] != (short)pat[++k]) break;
		} else if (pat[k] == '_') {
			if (iswspace(s[j]))
				return(FALSE);
			if (s[j+1] && pat[k+1])
				continue; 					// any character
			else {
				if (s[j+1] == (short)pat[k+1]) {
					s = s+l;
					if (lf != s) {
						while (lf != s) *lf++ = (short)' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			}
		} else if (pat[k] == '*') {			// wildcard
			k++; t = j;
			if (pat[k] == '\\') k++;
f1:
			while (s[j] && s[j] != (short)pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					s = s+l;
					if (lf != s) while (lf != s) *lf++ = (short)' ';
					return(TRUE);
				} else {
					for (; t < j; t++) {
						if (mStr.uS.ismorfchar(s,t, dFnt.Encod, MORPHS, MBF))
							return(FALSE);
					}
					return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== (short)'(' || s[m]== (short)')') && *wutterance.speaker != (short)'%')
					n--;
				else
				if (pat[n] == '*') break;
				else
				if (pat[n] == '_') {
					if (iswspace(s[m])) {
						j++;
						goto f1;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n]) return(FALSE);
					else if (s[m] != (short)pat[n]) {
						j++;
						goto f1;
					}
				} else if (s[m] != (short)pat[n]) {
					j++;
					goto f1;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f1;
			}
		}

		if (s[j] != (short)pat[k]) {
			return(FALSE);
		}
	}
	if (s[j] == (short)pat[k]) {
		s = s+l;
		if (lf != s) while (lf != s) *lf++ = (short)' ';
		return(TRUE);
	} else return(FALSE);
}

char cChatToXMLStr::isRightChar(short *org, long pos, const char chr, short Script, char MBC) {
	if (MBC) {
		if (ismultibyte(org, (short)pos, Script) != 0)
			return(FALSE);
	}
	if (org[pos] == (short)chr)
		return(TRUE);
	return(FALSE);
}

char cChatToXMLStr::isskip(short *org, int pos, short Script, char MBC) {
	register short chr;
	register short *TempPunctPtr;

	if (MBC) {
		if (ismultibyte(org, (short)pos, Script) != 0)
			return(FALSE);
	}
	chr = org[pos];
	if (iswalnum(chr)) return(FALSE);
	if (iswspace(chr)) return(TRUE);
	TempPunctPtr = punctuation;
	while (*TempPunctPtr != chr && *TempPunctPtr != EOS)
		TempPunctPtr++;
	return(*TempPunctPtr != EOS);
}

char cChatToXMLStr::isSqBracketItem(short *s, int pos, short Script, char MBC) {
	for (; s[pos] && !uS.isRightChar(s, pos, '[', Script, MBC) && !uS.isRightChar(s, pos, ']', Script, MBC); pos++) ;
	if (uS.isRightChar(s, pos, ']', Script, MBC)) return(TRUE);
	else return(FALSE);
}

char cChatToXMLStr::isSqCodes(short *word, short *tWord, short cScript, char isForce) {
	int len;

	if (!isForce) {
		len = strlen(word) - 1;
		if (!uS.isRightChar(word, 0, '[', cScript, TRUE) ||
			!uS.isRightChar(word, len, ']', cScript, TRUE))
			return(FALSE);
	}
	while (*word != EOS) {
		if (iswspace(*word)) {
			*tWord++ = (short)' ';
			for (word++; iswspace(*word) && *word; word++) ;
		} else {
			*tWord++ = *word++;
		}
	}
	*tWord = EOS;
	return(TRUE);
}

char cChatToXMLStr::isToneUnitMarker(short *word) {
	if (!strcmp(word, "-?")  || !strcmp(word, "-!")  || !strcmp(word, "-.") || 
		!strcmp(word, "-'.") || !strcmp(word, "-,.") || !strcmp(word, "-,") || 
		!strcmp(word, "-'")  || !strcmp(word, "-_")  || !strcmp(word, "-:") ||
		!strcmp(word, "-"))
		return(TRUE);
	else
		return(FALSE);
}

void cChatToXMLStr::lowercasestr(short *str, short Script, char MBC) {
	register short pos;

	if (MBC) {
#if defined(_MAC_CODE)
		WToC(str, tempchar, UTTLINELEN);
#endif
		for (pos=0; str[pos] != EOS; pos++) {
#if defined(_MAC_CODE)
			if (ismultibyte(tempchar, pos, Script) == 0)
#elif defined(_WIN32)
			if (ismultibyte(str, pos, Script) == 0)
#endif
				str[pos] = towlower(str[pos]);
		}
	} else {
		for (; *str != EOS; str++) {
			*str = towlower(*str);
		}
	}
}

void cChatToXMLStr::uppercasestr(short *str, short Script, char MBC) {
	register short pos;

	if (MBC) {
#if defined(_MAC_CODE)
		WToC(str, tempchar, UTTLINELEN);
#endif
		for (pos=0; str[pos] != EOS; pos++) {
#if defined(_MAC_CODE)
			if (ismultibyte(tempchar, pos, Script) == 0)
#elif defined(_WIN32)
			if (ismultibyte(str, pos, Script) == 0)
#endif
				str[pos] = towupper(str[pos]);
		}
	} else {
		for (; *str != EOS; str++) {
			*str = towupper(*str);
		}
	}
}

void cChatToXMLStr::remFblanks(short *st) {
	register int i;

	for (i=0; isSpace(st[i]); i++) ;
	if (i > 0)
		mStr.strcpy(st, st+i);
}

void cChatToXMLStr::remblanks(short *st) {
	register int i;

	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == (short)'\n')) i--;
	st[i+1] = EOS;
}

void cChatToXMLStr::cleanUpCodes(short *code, short Script, char MBC) {
	long i;
	
	for (i=0L; code[i] != EOS; i++) {
		if (uS.isRightChar(code, i, '\t', Script, MBC) || 
			uS.isRightChar(code, i, '\n', Script, MBC)) {
			code[i] = (short)' ';
		}
	}
}
/*	 strings manipulation routines end   */

short *ChatError(int num, long *ln) {
	short *mess;

	if (num != -1)
		errNum = num;
	switch (errNum) {
		case 0:
			mStr.strcpy(tempst1, "Internal Error!");
			break;
		case 1:
			mStr.strcpy(tempst1, "Out of memory");
			break;
		case 2:
			mStr.strcpy(tempst1, "Internal error; Increase value of \"NUMOVMATCHES\"");
			break;
		case 3:
			mStr.strcpy(tempst1, "Closing overlap found before openning one");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 4:
			mStr.strcpy(tempst1, "Number expected in an overlap");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 5:
			mStr.strcpy(tempst1, "Found unmanageable overlap");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 6:
			mStr.strcpy(tempst1, "Missing closing overlap \"[<]\"");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 7:
			mStr.strcpy(tempst1, "Duplicate tiers found");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 8:
			mStr.strcpy(tempst1, "Participants tier is corrupt");
			break;
		case 9:
			mStr.strcpy(tempst1, "Illegal language code");
			mStr.strcat(tempst1, tempst2);
			break;
		case 10:
			mStr.strcpy(tempst1, "Unknown language");
			mStr.strcat(tempst1, tempst2);
			break;
		case 11:
			mStr.strcpy(tempst1, "Unrecognized font");
			mStr.strcat(tempst1, tempst2);
			break;
		case 12:
			mStr.strcpy(tempst1, "Multiple utterance terminators found on tier");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 13:
			mStr.strcpy(tempst1, "Unknown special utterance terminators found on tier");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 14:
			mStr.strcpy(tempst1, "wrong use of lenghtened_syllable ':' in a word");
			mStr.strcat(tempst1, tempst2);
			break;
		case 15:
			mStr.strcpy(tempst1, "Expected character '|' at current position");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 16:
			mStr.strcpy(tempst1, "Unexpected character '|' after '|' at position");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 17:
			mStr.strcpy(tempst1, "Unexpected character '|' after '&' at position");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 18:
			mStr.strcpy(tempst1, "Unexpected character '|' after '#' at position");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 19:
			mStr.strcpy(tempst1, "Unexpected character '|' after '-' at position");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 20:
			mStr.strcpy(tempst1, "Unexpected character '|' after '=' at position");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 21:
			mStr.strcpy(tempst1, "Expected character '|'at position");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 22:
			mStr.strcpy(tempst1, "Internal error undetected/unaccounted overlap");
			break;
		case 23:
			mStr.strcpy(tempst1, "Too complicated: word has both clitics and compounds");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 24:
			mStr.strcpy(tempst1, "Unknown tier code");
			mStr.strcat(tempst1, tempst2);
			break;
		case 25:
			mStr.strcpy(tempst1, "Illegal [:=* *] code");
			mStr.strcat(tempst1, tempst2);
			break;
		case 26:
			mStr.strcpy(tempst1, "overlaped word is not inside <>");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 27:
			mStr.strcpy(tempst1, "Unmatched overlap, perhaps next overlap should be \"<\" instead of \">\"");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 28:
			mStr.strcpy(tempst1, "Unknown code or special word");
			mStr.strcat(tempst1, tempst2);
			break;
		case 29:
			mStr.strcpy(tempst1, "Missing text before code");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 30:
			mStr.strcpy(tempst1, "Internal error; Increase value of \"CODESMAX\"");
			break;
		case 31:
			mStr.strcpy(tempst1, "Missing '<' character");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 32:
			mStr.strcpy(tempst1, "Bad scoping information");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 33:
			mStr.strcpy(tempst1, "Expected a word");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 34:
			mStr.strcpy(tempst1, "Scoping doesn't match to speaker tier");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 35:
			mStr.strcpy(tempst1, "Expected scoping information, such as <2>");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 36:
			mStr.strcpy(tempst1, "Internal error: Add tier found in non-scopping region");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 37:
			mStr.strcpy(tempst1, "Not all %%mor: items matched the main tier");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 38:
			mStr.strcpy(tempst1, "Not all %%pho: items matched the main tier");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 39:
			mStr.strcpy(tempst1, "Not all %%mod: items matched the main tier");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 40:
			mStr.strcpy(tempst1, "Terminator is missing on tier");
			if (num == -1)
				mStr.strcat(tempst1, tempst2);
			break;
		case 41:
			mStr.strcpy(tempst1, "Date is malformed");
			mStr.strcat(tempst1, tempst2);
			break;
		case 42:
			mStr.strcpy(tempst1, "Age is malformed");
			mStr.strcat(tempst1, tempst2);
			break;
		case 43:
			mStr.strcpy(tempst1, "Can't find appropriate Language information on @ID tier");
			break;
		case 44:
			mStr.strcpy(tempst1, "Illegal text attribute marker '\\002' found");
			break;
		case 45:
			mStr.strcpy(tempst1, "Speaker turn is longer than ");
			mStr.strcat(tempst1, tempst2);
			break;
		case 46:
			mStr.strcpy(tempst1, "Tier name is longer than ");
			mStr.strcat(tempst1, tempst2);
			break;
		case 47:
			mStr.strcpy(tempst1, "");
			mStr.strcat(tempst1, tempst2);
			break;
		case 900:
			mStr.strcpy(tempst1, "Romeo doesn't want to deal with '='");
			mStr.strcat(tempst1, tempst2);
			break;
		default:
			break;
	}
	if ((mess=(short *)malloc((mStr.strlen(tempst1)+1)*2)) == NULL) {
		if (ln)
			*ln = 0;
		return((short *)L"Out of memory");
	}
	mStr.strcpy(mess, tempst1);
	if (ln)
		*ln = lineno;
	return(mess);
}

int checkChat(short *chatS, char *fname, char UEncoding) {
	if (UEncoding == UDEFAULT)
#ifdef _WIN32 
		UEncoding = UPC;
#else
		UEncoding = UMAC;
#endif
	if (ChatToXML(chatS, true, fname, NULL, UEncoding, NULL, NULL) == NULL)
		return(errNum);
	else
		return(0);
}

char *transformChatToXML(short *chatS, char *fname, char UEncoding) {
	if (UEncoding == UDEFAULT)
#ifdef _WIN32 
		UEncoding = UPC;
#else
		UEncoding = UMAC;
#endif
	return(ChatToXML(chatS, false, fname, NULL, UEncoding, NULL, NULL));
}
