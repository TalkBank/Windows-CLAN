/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#ifdef _WIN32 
	#define watoi _wtoi
	#define watol _wtol
//	#define iswupper _iswupper
//	#define iswlower _iswlower
//	#define iswdigit _iswdigit
//	#define iswspace _iswspace
//	#define iswalnum _iswalnum
#endif

/*
#ifdef isFNTYPEUNIQUE
	int sprintf(FNType *st, const char *cFormat, ...);
	int strcmp(const FNType *st1, const FNType *st2);
	int mStricmp(const FNType *st1, const FNType *st2);
	int mStrnicmp(const FNType *st1, const FNType *st2, size_t len);
	size_t strlen(const FNType *st);
	FNType  *strcpy(FNType *des, const FNType *src);
	FNType  *strncpy(FNType *des, const FNType *src, size_t num);
	FNType  *strcat(FNType *des, const FNType *src);
	FNType  *strchr(FNType *src, int c);
	FNType  *strrchr(FNType *src, int c);
	long lowercasestr(FNType *str, NewFontInfo *finfo, char MBC);
	long uppercasestr(FNType *str, NewFontInfo *finfo, char MBC);
	char fpatmat(FNType *s, FNType *pat);
#endif // isFNTYPEUNIQUE
*/

#define INSIDE_STRINGPARSER

#ifndef UNX
	#include "ced.h"
	#include <wchar.h>
#else // !UNX
	#define isSpace(c)	 ((c) == (unCH)' ' || (c) == (unCH)'\t')
	#define LF_FACESIZE 256

	#include <string.h> //lxs string
#endif // UNX

#include "cu.h"
#include "check.h"

extern char Parans;

const char *punctuation;
cUStr uS;

#if defined(_MAC_CODE)
static long lbuf[UTTLINELEN], lformat[256];
#endif

#ifndef UNX
/*	 strings manipulation routines begin   */

// UTF16
// -fshort-wchar
int cUStr::sprintf(unCH *st, const unCH *format, ...) {
	int t;
	va_list args;

	va_start(args, format); 	// prepare the arguments
#ifdef _WIN32
	t = vswprintf(st, UTTLINELEN, format, args);
#else
	for (t=0; format[t] != 0; t++)
		lformat[t] = format[t];
	lformat[t] = 0;
	t = vswprintf((wchar_t *)lbuf, UTTLINELEN, (wchar_t *)lformat, args);
	if (t >= 0) {
		for (t=0; lbuf[t] != 0; t++)
			st[t] = lbuf[t];
	} else
		t = 0;
	st[t] = 0;
#endif
	va_end(args);				// clean the stack
	return(t);
}

int cUStr::strcmp(const unCH *st1, const unCH *st2) {
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (*st1 > *st2)
		return(1);
	else
		return(-1);	
}

int cUStr::strcmp(const unCH *st1, const char *st2) {
	for (; *st1 == (unCH)*st2 && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (*st1 > (unCH)*st2)
		return(1);
	else
		return(-1);	
}

int cUStr::strncmp(const unCH *st1, const unCH *st2, size_t len) {
	for (; *st1 == *st2 && *st2 != EOS && len > 0; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0)
		return(0);
	else if (*st1 > *st2)
		return(1);
	else
		return(-1);	
}

int cUStr::strncmp(const unCH *st1, const char *st2, size_t len) {
	for (; *st1 == (unCH)*st2 && *st2 != EOS && len > 0; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0)
		return(0);
	else if (*st1 > (unCH)*st2)
		return(1);
	else
		return(-1);	
}

int cUStr::mStricmp(const unCH *st1, const unCH *st2) {
	for (; towupper(*st1) == towupper(*st2) && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (towupper(*st1) > towupper(*st2))
		return(1);
	else
		return(-1);	
}

int cUStr::mStricmp(const unCH *st1, const char *st2) {
	for (; towupper(*st1) == toupper((unsigned char)*st2) && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (towupper(*st1) > toupper((unsigned char)*st2))
		return(1);
	else
		return(-1);	
}

int cUStr::mStrnicmp(const unCH *st1, const unCH *st2, size_t len) {
	for (; towupper(*st1) == towupper(*st2) && *st2 != EOS && len > 0; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0)
		return(0);
	else if (towupper(*st1) > towupper(*st2))
		return(1);
	else
		return(-1);	
}

int cUStr::mStrnicmp(const unCH *st1, const char *st2, size_t len) {
	for (; towupper(*st1) == toupper((unsigned char)*st2) && *st2 != EOS && len > 0; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0)
		return(0);
	else if (towupper(*st1) > toupper((unsigned char)*st2))
		return(1);
	else
		return(-1);	
}

size_t cUStr::strlen(const unCH *st) {
	size_t len = 0;

	for (; *st != EOS; st++)
		len++;
	return(len);
}

unCH *cUStr::strcpy(unCH *des, const unCH *src) {
	unCH *org = des;

	while (*src != EOS) {
		*des++ = *src++;
	}
	*des = EOS;
	return(org);
}

unCH *cUStr::strcpy(unCH *des, const char *src) {
	unCH *org = des;

	while (*src != EOS) {
		*des = (unCH)*src++;
		*des &= 0x00FF;
		des++;
	}
	*des = EOS;
	return(org);
}

char *cUStr::u_strcpy(char *des, const unCH *src, unsigned long MaxLen) {
	UnicodeToUTF8(src, cUStr::strlen(src), (unsigned char *)des, NULL, MaxLen);
	return(des);
}

unCH *cUStr::u_strcpy(unCH *des, const char *src, unsigned long MaxLen) {
	UTF8ToUnicode((unsigned char *)src, cUStr::strlen(src), des, NULL, MaxLen);
	return(des);
}

unCH *cUStr::strncpy(unCH *des, const unCH *src, size_t num) {
	long i = 0L;

	while (i < num && src[i] != EOS) {
		des[i] = src[i];
		i++;
	}
	des[i] = EOS;
	return(des);
}

unCH *cUStr::strncpy(unCH *des, const char *src, size_t num) {
	long i = 0L;

	while (i < num && src[i] != EOS) {
		des[i] = (unCH)src[i];
		des[i] &= 0x00FF;
		i++;
	}
	des[i] = EOS;
	return(des);
}

unCH *cUStr::strcat(unCH *des, const unCH *src) {
	long i;

	i = cUStr::strlen(des);
	while (*src != EOS) {
		des[i] = *src++;
		i++;
	}
	des[i] = EOS;
	return(des);
}

unCH *cUStr::strcat(unCH *des, const char *src) {
	long i;

	i = cUStr::strlen(des);
	while (*src != EOS) {
		des[i] = (unCH)*src++;
		des[i] &= 0x00FF;
		i++;
	}
	des[i] = EOS;
	return(des);
}

unCH *cUStr::strncat(unCH *des, const unCH *src, size_t num) {
	long i, j = 0L;

	i = cUStr::strlen(des);
	while (j < num && *src != EOS) {
		des[i] = *src++;
		i++;
		j++;
	}
	des[i] = EOS;
	return(des);
}

unCH *cUStr::strncat(unCH *des, const char *src, size_t num) {
	long i, j = 0L;

	i = cUStr::strlen(des);
	while (j < num && *src != EOS) {
		des[i] = (unCH)*src++;
		des[i] &= 0x00FF;
		i++;
		j++;
	}
	des[i] = EOS;
	return(des);
}

unCH *cUStr::strchr(unCH *src, int c) {
	for (; *src != c && *src != EOS; src++) ;
	if (*src == EOS)
		return(NULL);
	else
		return(src);
}

unCH *cUStr::strrchr(unCH *src, int c) {
	long i;
	
	i = cUStr::strlen(src);
	for (; i >= 0 && src[i] != c; i--) ;
	if (src[i] == c)
		return(src+i);
	else
		return(NULL);
}

unCH *cUStr::strpbrk(unCH *src, const unCH *cs) {
	long i, j;

	for (i=0L; src[i] != EOS; i++) {
		for (j=0L; cs[j] != EOS; j++) {
			if (src[i] == cs[j])
				return(src+i);
		}
	}
	return(NULL);
}

unCH *cUStr::strpbrk(unCH *src, const char *cs) {
	long i, j;

	for (i=0L; src[i] != EOS; i++) {
		for (j=0L; cs[j] != EOS; j++) {
			if (src[i] == (unCH)cs[j])
				return(src+i);
		}
	}
	return(NULL);
}

int cUStr::atoi(const unCH *s) {
#if (__MACH__)
	int i;
	char buf[256];

	for (i=0; iswdigit(*s) && i < 255; i++, s++)
		buf[i] = (char)*s;
	buf[i] = EOS;
	return(atoi(buf));
#else
	return(watoi(s));
#endif
}

long cUStr::atol(const unCH *s) {
#if (__MACH__)
	int i;
	char buf[256];

	for (i=0; iswdigit(*s) && i < 255; i++, s++)
		buf[i] = (char)*s;
	buf[i] = EOS;
	return(atol(buf));
#else
	return(watol(s));
#endif
}

static char isskip_wchar(unCH chr, unCH *wPunctuation) {
	register unCH *w;

	if (iswalnum(chr)) return(FALSE);
	if (iswspace(chr)) return(TRUE);
	w = wPunctuation;
	while (*w != chr && *w != EOS)
		w++;
	return(*w != EOS);
}

char cUStr::isskip(const unCH *org, int pos, NewFontInfo *finfo, char MBC) {
	register unCH chr;
	register unCH *w;
	unCH wPunctuation[50];
	const char *c;

	for (w=wPunctuation,c=punctuation; *c != EOS; c++, w++) {
		*w = (unCH)*c;
		*w &= 0x00FF;
	}
	*w = EOS;
	
	chr = org[pos];
	if (chr == sMarkChr || chr == dMarkChr) return(TRUE);
	if (iswalnum(chr)) return(FALSE);
	if (iswspace(chr) || chr == (unCH)'\n' || chr == (unCH)'\r') return(TRUE);
	if (chr == '.') {
		if (uS.isPause(org, pos, NULL, NULL))
			return(FALSE);
	}
	w = wPunctuation;
	while (*w != chr && *w != EOS)
		w++;
	if (*w != EOS && chr == '+') {
		if (pos > 0 && isskip_wchar(w[pos-1], wPunctuation))
			return(FALSE);
	}
	return(*w != EOS);
}

char cUStr::ismorfchar(const unCH *org, int pos, NewFontInfo *finfo, const char *morfsList, char MBC) {
	register unCH chr, nextChr, prevChr;
	const char *morf;

	if (morfsList == NULL)
		return(FALSE);
	if (org[pos] >= 126 || org[pos] < 1)
		return(FALSE);

	chr = org[pos];
	if (pos > 0)
		prevChr = org[pos-1];
	else
		prevChr = 0;
	nextChr = org[pos+1];
	if (MBC) {
	}
	for (morf=morfsList; *morf; morf++) {
		if (*morf == (char)chr && nextChr != '0')  {
			if (chr != '+' || prevChr != '|')
				return(TRUE);
		}
	}
	return(FALSE);
}

char cUStr::isCharInMorf(char c, unCH *morf) {
	for (; *morf; morf++) 
		if ((unCH)c == *morf) return(TRUE);
	return(FALSE);
}

char cUStr::atUFound(const unCH *w, int s, NewFontInfo *finfo, char MBC) {
	while (w[s] != EOS) {
		if (uS.isRightChar(w, s, '<', finfo, MBC) || uS.isRightChar(w, s, '>', finfo, MBC) || 
			uS.isRightChar(w, s, '[', finfo, MBC) || uS.isRightChar(w, s, ']', finfo, MBC) ||
			isSpace(w[s]) || w[s] == '\n')
			break;
		if (w[s] == '@') {
			if (w[s+1] != EOS && w[s+1] == 'u' && (w[s+2] == EOS || !iswalnum(w[s+2]))) 
				return(TRUE);
		}
		s++;
	}
	return(FALSE);
}

char cUStr::isRightChar(const unCH *org, long pos, register char chr, NewFontInfo *finfo, char MBC) {
	if (MBC) {
	}
	if (org[pos] == (unCH)chr)
		return(TRUE);
	return(FALSE);
}

#define MyUpperChars(c) ((c) >= (unCH)0xC0 && (c) <= (unCH)0xDD)

char cUStr::isUpperChar(unCH *org, int pos, NewFontInfo *finfo, char MBC) {
	if (MBC) {
	}
	if (MyUpperChars(org[pos]))
		return(TRUE);
	else
		return(iswupper(org[pos]) != 0);
}

char cUStr::isSqBracketItem(const unCH *s, int pos, NewFontInfo *finfo, char MBC) {
	for (; s[pos] && !uS.isRightChar(s, pos, '[', finfo, MBC) && !uS.isRightChar(s, pos, ']', finfo, MBC); pos++) ;
	if (uS.isRightChar(s, pos, ']', finfo, MBC))
		return(TRUE);
	else
		return(FALSE);
}

char cUStr::isSqCodes(const unCH *word, unCH *tWord, NewFontInfo *finfo, char isForce) {
	int len;

	if (!isForce && word[0] != EOS) {
		len = cUStr::strlen(word) - 1;
		if (!uS.isRightChar(word, 0, '[', finfo, TRUE) ||
			!uS.isRightChar(word, len, ']', finfo, TRUE))
			return(FALSE);
	}
	while (*word != EOS) {
		if (iswspace(*word)) {
			*tWord++ = (unCH)' ';
			for (word++; iswspace(*word) && *word; word++) ;
		} else {
			*tWord++ = *word++;
		}
	}
	*tWord = EOS;
	return(TRUE);
}

void cUStr::remblanks(unCH *st) {
	register int i;

	i = cUStr::strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == '\r'))
		i--;
	if (st[i+1] != EOS)
		st[i+1] = EOS;
}

void cUStr::remFrontAndBackBlanks(unCH *st) {
	register int i;

	for (i=0; isSpace(st[i]) || st[i] == '\n' || st[i] == '\r'; i++) ;
	if (i > 0)
		cUStr::strcpy(st, st+i);
	i = cUStr::strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' ||  st[i] == '\r' || st[i] == NL_C || st[i] == SNL_C))
		i--;
	st[i+1] = EOS;
}

void cUStr::shiftright(unCH *st, int num) {
	register int i;

	for (i=cUStr::strlen(st); i >= 0; i--)
		st[i+num] = st[i];
}

void cUStr::cleanUpCodes(unCH *code, NewFontInfo *finfo, char MBC) {
	long i;
	
	for (i=0L; code[i] != EOS; i++) {
		if (uS.isRightChar(code, i, '\t', finfo, MBC) || 
			uS.isRightChar(code, i, '\n', finfo, MBC)) {
			code[i] = (unCH)' ';
		}
	}
}

// uS.extractString(tempst3, line, "[%add: ", ']');
void cUStr::extractString(unCH *out, const unCH *line, const char *type, unCH endC) {
	int i, j;

	for (i=cUStr::strlen(type); isSpace(line[i]); i++) ;
	for (j=0; line[i] != endC && line[i] != EOS; i++, j++)
		out[j] = line[i];
	out[j] = EOS;
	uS.remblanks(out);
}

int cUStr::isToneUnitMarker(unCH *word) {
/*
	if (!cUStr::strcmp(word, "-?")  || !cUStr::strcmp(word, "-!")  || !cUStr::strcmp(word, "-.") || 
		!cUStr::strcmp(word, "-'.") || !cUStr::strcmp(word, "-,.") || !cUStr::strcmp(word, "-,") || 
		!cUStr::strcmp(word, "-'")  || !cUStr::strcmp(word, "-_")  || !cUStr::strcmp(word, "-:") ||
		!cUStr::strcmp(word, "-"))
		return(TRUE);
	else
*/
		return(FALSE);
}

int cUStr::IsCAUtteranceDel(unCH *st, int pos) {
	if (st[pos] == 0x21D7 || st[pos] == 0x2197 || st[pos] == 0x2192 || st[pos] == 0x2198 ||
		  st[pos] == 0x21D8 || st[pos] == 0x221E)
		return(1);
	return(0);
}

int cUStr::IsCharUtteranceDel(unCH *st, int pos) {
	if (st[pos] == (unCH)'.' || st[pos] == (unCH)'?' || st[pos] == (unCH)'!' ||
		st[pos] == (unCH)'+' || st[pos] == (unCH)'/' || st[pos] == (unCH)'"') {
		return(1);
	}
	return(0);
}

int cUStr::IsUtteranceDel(const unCH *st, int pos) {
	if (st[pos] == (unCH)'.') {
		if (uS.isPause(st, pos, NULL, NULL))
			return(0);
		else {
			for (; pos >= 0 && !isSpace(st[pos]) && st[pos] != (unCH)'[' && st[pos] != (unCH)']'; pos--) ;
			if (pos >= 0 && st[pos] == (unCH)'[')
				return(0);
			else
				return(1);
		}
	} else if (st[pos] == (unCH)'?' || st[pos] == (unCH)'!') {
		for (; pos >= 0 && !isSpace(st[pos]) && st[pos] != (unCH)'[' && st[pos] != (unCH)']'; pos--) ;
		if (pos >= 0 && st[pos] == (unCH)'[')
			return(0);
		else
			return(1);
	}
	return(0);
}

int cUStr::isPause(const unCH *st, int posO, int *beg, int *end) {
	int pos;

	if (end != NULL) {
		for (pos=posO+1; iswdigit(st[pos]) || st[pos] == '.' || st[pos] == ':'; pos++) ;
		if (!uS.isRightChar(st,pos,')',&dFnt,MBF))
			return(FALSE);
		*end = pos;
	} else {
		for (pos=posO-1; pos >= 0 && (iswdigit(st[pos]) || st[pos] == '.' || st[pos] == ':'); pos--) ;
		if (pos < 0 || !uS.isRightChar(st,pos,'(',&dFnt,MBF))
			return(FALSE);
		if (beg != NULL)
			*beg = pos;
		for (pos=posO; iswdigit(st[pos]) || st[pos] == '.' || st[pos] == ':'; pos++) ;
		if (!uS.isRightChar(st,pos,')',&dFnt,MBF))
			return(FALSE);
	}
	return(TRUE);
}

long cUStr::lowercasestr(unCH *str, NewFontInfo *finfo, char MBC) {
	long count;

	count = 0L;
	for (; *str != EOS; str++) {
		if (iswupper(*str)) {
			count++;
			*str = towlower(*str);
		}
	}
	return(count);
}

long cUStr::uppercasestr(unCH *str, NewFontInfo *finfo, char MBC) {
	long count;

	count = 0L;
	for (; *str != EOS; str++) {
		if (iswlower(*str)) {
			count++;
			*str = towupper(*str);
		}
	}
	return(count);
}

unCH *cUStr::sp_cp(unCH *s1, unCH *s2) {
	if (s1 != s2) {
		while (*s2)
			*s1++ = *s2++;
		*s1 = EOS;
		return(s1);
	} else
		return(s1+cUStr::strlen(s2));
}

void cUStr::sp_mod(unCH *s1, unCH *s2) {
	if (s1 != s2) {
		while (s1 != s2)
			*s1++ = '\001';
	}
}

char cUStr::isUTF8(const unCH *str) {
	if (str[0] == (unCH)0xef && str[1] == (unCH)0xbb && str[2] == (unCH)0xbf &&
		str[3] == (unCH)'@' && str[4] == (unCH)'U' && str[5] == (unCH)'T' && str[6] == (unCH)'F' && str[7] == (unCH)'8')
		return(TRUE);
	else if (str[0] == (unCH)'@' && str[1] == (unCH)'U' && str[2] == (unCH)'T' && str[3] == (unCH)'F' && str[4] == (unCH)'8')
		return(TRUE);
	else
		return(FALSE);
}

char cUStr::isInvisibleHeader(const unCH *str) {
	if (uS.partcmp(str, PIDHEADER, FALSE, FALSE) || uS.partcmp(str, CKEYWORDHEADER, FALSE, FALSE) ||
		uS.partcmp(str, WINDOWSINFO, FALSE, FALSE) || uS.partcmp(str, FONTHEADER, FALSE, FALSE))
		return(TRUE);
	else
		return(FALSE);
}

int cUStr::HandleCAChars(unCH *w, int *matchedType) { // CA CHARS
	int offset;

	if (matchedType != NULL)
		*matchedType = CA_NOT_ALL;
	if (*w == 0x2191) {	// up-arrow
		if (matchedType != NULL)
			*matchedType = CA_APPLY_SHIFT_TO_HIGH_PITCH;
		return(1);
	} else if (*w == 0x2193) { // down-arrow
		if (matchedType != NULL)
			*matchedType = CA_APPLY_SHIFT_TO_LOW_PITCH;
		return(1);
	} else if (*w == 0x21D7) { // rise to high
		if (matchedType != NULL)
			*matchedType = CA_APPLY_RISETOHIGH;
		return(1);
	} else if (*w == 0x2197) { // rise to mid
		if (matchedType != NULL)
			*matchedType = CA_APPLY_RISETOMID;
		return(1);
	} else if (*w == 0x2192) { // level
		if (matchedType != NULL)
			*matchedType = CA_APPLY_LEVEL;
		return(1);
	} else if (*w == 0x2198) { // fall to mid
		if (matchedType != NULL)
			*matchedType = CA_APPLY_FALLTOMID;
		return(1);
	} else if (*w == 0x21D8) { // fall to low
		if (matchedType != NULL)
			*matchedType = CA_APPLY_FALLTOLOW;
		return(1);
	} else if (*w == 0x221E) { // unmarked ending
		if (matchedType != NULL)
			*matchedType = CA_APPLY_UNMARKEDENDING;
		return(1);
	} else if (*w == 0x224B) { // continuation - wavy triple-line equals sign
		if (matchedType != NULL)
			*matchedType = CA_APPLY_CONTINUATION;
		return(1);
	} else if (*w == 0x2219) { // inhalation - raised period
		if (matchedType != NULL)
			*matchedType = CA_APPLY_INHALATION;
		return(1);
	} else if (*w == 0x2248) { // latching
		if (matchedType != NULL)
			*matchedType = CA_APPLY_LATCHING;
		return(1);
	} else if (*w == 0x2261) { // uptake
		if (matchedType != NULL)
			*matchedType = CA_APPLY_UPTAKE;
		return(1);
	} else if (*w == 0x2308) { // raised [
		for (offset=1; iswdigit(w[offset]); offset++) ;
		if (matchedType != NULL)
			*matchedType = CA_APPLY_OPTOPSQ;
		return(offset);
	} else if (*w == 0x2309) { // raised ]
		for (offset=1; iswdigit(w[offset]); offset++) ;
		if (matchedType != NULL)
			*matchedType = CA_APPLY_CLTOPSQ;
		return(offset);
	} else if (*w == 0x230A) { // lowered [
		for (offset=1; iswdigit(w[offset]); offset++) ;
		if (matchedType != NULL)
			*matchedType = CA_APPLY_OPBOTSQ;
		return(offset);
	} else if (*w == 0x230B) { // lowered ]
		for (offset=1; iswdigit(w[offset]); offset++) ;
		if (matchedType != NULL)
			*matchedType = CA_APPLY_CLBOTSQ;
		return(offset);
	} else if (*w == 0x2206) { // faster
		if (matchedType != NULL)
			*matchedType = CA_APPLY_FASTER;
		return(1);
	} else if (*w == 0x2207) { // slower
		if (matchedType != NULL)
			*matchedType = CA_APPLY_SLOWER;
		return(1);
	} else if (*w == 0x204E) { // creaky - big asterisk
		if (matchedType != NULL)
			*matchedType = CA_APPLY_CREAKY;
		return(1);
	} else if (*w == 0x2047) { // unsure
		if (matchedType != NULL)
			*matchedType = CA_APPLY_UNSURE;
		return(1);
	} else if (*w == 0x00B0) { // softer
		if (matchedType != NULL)
			*matchedType = CA_APPLY_SOFTER;
		return(1);
	} else if (*w == 0x25C9) { // louder
		if (matchedType != NULL)
			*matchedType = CA_APPLY_LOUDER;
		return(1);
	} if (*w == 0x2581) { // low pitch - low bar
		if (matchedType != NULL)
			*matchedType = CA_APPLY_LOW_PITCH;
		return(1);
	} else if (*w == 0x2594) { // high pitch - high bar
		if (matchedType != NULL)
			*matchedType = CA_APPLY_HIGH_PITCH;
		return(1);
	} else if (*w == 0x263A) { // smile voice
		if (matchedType != NULL)
			*matchedType = CA_APPLY_SMILE_VOICE;
		return(1);
	} else if (*w == 0x222C) { // whisper
		if (matchedType != NULL)
			*matchedType = CA_APPLY_WHISPER;
		return(1);
	} else if (*w == 0x03AB) { // yawn
		if (matchedType != NULL)
			*matchedType = CA_APPLY_YAWN;
		return(1);
	} else if (*w == 0x222E) { // singing
		if (matchedType != NULL)
			*matchedType = CA_APPLY_SINGING;
		return(1);
	} else if (*w == 0x00A7) { // precise
		if (matchedType != NULL)
			*matchedType = CA_APPLY_SINGING;
		return(1);
	} else if (*w == 0x223E) { // constriction
		if (matchedType != NULL)
			*matchedType = CA_APPLY_CONSTRICTION;
		return(1);
	} else if (*w == 0x21BB) { // pitch reset
		if (matchedType != NULL)
			*matchedType = CA_APPLY_PITCH_RESET;
		return(1);
	} else if (*w == 0x1F29) { // laugh in a word
		if (matchedType != NULL)
			*matchedType = CA_APPLY_LAUGHINWORD;
		return(1);
	} else if (*w == 0x2039) { // Group start marker - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_GROUP_START;
		return(1);
	} else if (*w == 0x203A) { // Group end marker - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_GROUP_END;
		return(1);
	} else if (*w == 0x201E) { // Tag or sentence final particle; „ - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_DOUBLE_COMMA;
		return(1);
	} else if (*w == 0x2021) { // Vocative or summons - ‡ - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_VOCATIVE;
		return(1);
	} else if (*w == 0x0304) { // Stress - ̄ - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_STRESS;
		return(1);
	} else if (*w == 0x0294) { // Glottal stop - ʔ - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_GLOTTAL_STOP;
		return(1);
	} else if (*w == 0x0295) { // Hebrew glottal - ʕ - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_HEBREW_GLOTTAL;
		return(1);
	} else if (*w == 0x030C) { // caron - ̌- NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_CARON;
		return(1);
	} else if (*w == 0x0332) { // underline - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_UNDERLINE;
		return(1);
	} else if (*w == 0x2026) { // %pho missing word -…- NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_MISSINGWORD;
		return(1);
	} else if (*w == 0x264b) { // breathy-voice -♋- NOT CA
		if (matchedType != NULL)
			*matchedType = CA_BREATHY_VOICE;
		return(1);
	} else if (*w == 0x02C8) { // raised stroke - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_RAISED_STROKE;
		return(1);
	} else if (*w == 0x02CC) { // lowered stroke - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_LOWERED_STROKE;
		return(1);
	} else if (*w == 0x3014) { // sign group start marker - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_SIGN_GROUP_START;
		return(1);
	} else if (*w == 0x3015) { // sign group end marker - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_SIGN_GROUP_END;
		return(1);
	} else if (*w == 0x201C) { // open quote “ - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_OPEN_QUOTE;
		return(1);
	} else if (*w == 0x201D) { // close quote ” - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_CLOSE_QUOTE;
		return(1);
	} else if (*w == 0x2260) { // crossed equal ≠ - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_CROSSED_EQUAL;
		return(1);
	} else if (*w == 0x21AB) { // left arrow with circle ↫ - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_LEFT_ARROW_CIRCLE;
		return(1);
	} else if (*w == 0x02D0) { // left arrow with circle ː - NOT CA
		if (matchedType != NULL)
			*matchedType = NOTCA_VOWELS_COLON;
		return(1);
	}
	return(0);
}

/* partcmp(st1,st2) does a partial comparisone of string "st2" to string
   "st1". The match is sucessful if the string "st2" completely matches the
   beginning part of string "st1". i.e. st2 = "first", and st1 = "first part".

   WARNING: This function is for tier names ONLY!!!!
			Use "partwcmp" function to compaire strings in general
*/
int cUStr::partcmp(const unCH *st1, const unCH *st2, char pat_match, char isCaseSenc) { // st1- full, st2- part
	if (pat_match) {
		int i, j, res;
		unCH t[SPEAKERLEN+1];
		if (*st1 == *st2) {
			for (i=cUStr::strlen(st1)-1; i >= 0 && (st1[i] == ' ' || st1[i] == '\t'); i--) ;
			for (j=cUStr::strlen(st2)-1; j >= 0 && (st2[j] == ' ' || st2[j] == '\t'); j--) ;
			if (st1[i] != ':' || st2[j] == ':' || i < 0)
				i++;
			if (i < SPEAKERLEN) {
				cUStr::strncpy(t, st1, i);
				t[i] = EOS;
				res = (int)uS.fIpatmat(t+1, st2+1);
				return(res);
			} else
				return(FALSE);
		} else
			return(FALSE);
	}
	if (isCaseSenc) {
		for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	} else {
		for (; towupper(*st1) == towupper(*st2) && *st2 != EOS; st1++, st2++) ;
	}
	if (*st2 == ':') return(!*st1);
	else return(!*st2);
}
int cUStr::partcmp(const unCH *st1, const char *st2, char pat_match, char isCaseSenc) { // st1- full, st2- part
	if (pat_match) {
		int i, j, res;
		unCH t[SPEAKERLEN+1];
		if (*st1 == *st2) {
			for (i=cUStr::strlen(st1)-1; i >= 0 && (st1[i] == ' ' || st1[i] == '\t'); i--) ;
			for (j=cUStr::strlen(st2)-1; j >= 0 && (st2[j] == ' ' || st2[j] == '\t'); j--) ;
			if (st1[i] != ':' || st2[j] == ':' || i < 0)
				i++;
			if (i < SPEAKERLEN) {
				cUStr::strncpy(t, st1, i);
				t[i] = EOS;
				res = (int)uS.fIpatmat(t+1, st2+1);
				return(res);
			} else
				return(FALSE);
		} else
			return(FALSE);
	}
	if (isCaseSenc) {
		for (; *st1 == (unCH)*st2 && *st2 != EOS; st1++, st2++) ;
	} else {
		for (; towupper(*st1) == toupper((unsigned char)*st2) && *st2 != EOS; st1++, st2++) ;
	}
	if (*st2 == ':') return(!*st1);
	else return(!*st2);
}

/* partwcmp(st1,st2) does a partial comparisone of string "st2" to string
   "st1". The match is sucessful if the string "st2" completely matches the
   beginning part of string "st1". i.e. st2 = "first", and st1 = "first part".
*/
int cUStr::partwcmp(const unCH *st1, const unCH *st2) { // st1- full, st2- part
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	return(!*st2);
}
int cUStr::partwcmp(const unCH *st1, const char *st2) { // st1- full, st2- part
	for (; *st1 == (unCH)*st2 && *st2 != EOS; st1++, st2++) ;
	return(!*st2);
}

/* fpatmat(s, pat) does pattern matching of pattern "pat" to string "s".
   "pat" may contain meta characters "_" and "*". "_" means match
   any one character, "*" means match zero or more characters. The value returned
   is 1 if there is a match, and 0 otherwise.
*/
char cUStr::fpatmat(const unCH *s, const unCH *pat) {
	register int j, k;
	int n, m;

	if (s[0] == EOS)
		return(pat[0] == s[0]);
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {
			k++;
f0:
			while (s[j] && s[j] != pat[k])
				j++;
			if (!s[j]) {
				if (!pat[k]) return(1);
				else return(0);
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*') break;
				if (s[m] != pat[n]) {
					j++;
					goto f0;
				}
			}
		}
		if (s[j] != pat[k]) {
			if (pat[k] == '*') {
				k--;
				j--;
			} else
				break;
		}
	}
	if (pat[k] == s[j])
		return(1);
	else
		return(0);
}
char cUStr::fpatmat(const unCH *s, const char *pat) {
	register int j, k;
	int n, m;

	if (s[0] == EOS)
		return(pat[0] == s[0]);
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {
			k++;
f0:
			while (s[j] && s[j] != pat[k])
				j++;
			if (!s[j]) {
				if (!pat[k]) return(1);
				else return(0);
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*') break;
				if (s[m] != pat[n]) {
					j++;
					goto f0;
				}
			}
		}
		if (s[j] != pat[k]) {
			if (pat[k] == '*') {
				k--;
				j--;
			} else
				break;
		}
	}
	if (pat[k] == s[j])
		return(1);
	else
		return(0);
}

/* fIpatmat(s, pat) does case insensitive pattern matching of pattern "pat" to string "s".
 "pat" may contain meta character "*". "*" means match zero or more characters. The value returned
 is 1 if there is a match, and 0 otherwise.
 */
char cUStr::fIpatmat(const unCH *s, const unCH *pat) {
	register int j, k;
	int n, m;

	if (s[0] == EOS)
		return(pat[0] == s[0]);
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {
			k++;
f0:
			while (s[j] && towupper(s[j]) != towupper(pat[k]))
				j++;
			if (!s[j]) {
				if (!pat[k]) return(1);
				else return(0);
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*') break;
				if (towupper(s[m]) != towupper(pat[n])) {
					j++;
					goto f0;
				}
			}
		}
		if (towupper(s[j]) != towupper(pat[k])) {
			if (pat[k] == '*') {
				k--;
				j--;
			} else
				break;
		}
	}
	if (towupper(s[j]) == towupper(pat[k]))
		return(1);
	else
		return(0);
}
char cUStr::fIpatmat(const unCH *s, const char *pat) {
	register int j, k;
	int n, m;

	if (s[0] == EOS)
		return(pat[0] == s[0]);
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {
			k++;
f0:
			while (s[j] && towupper(s[j]) != toupper((unsigned char)pat[k]))
				j++;
			if (!s[j]) {
				if (!pat[k]) return(1);
				else return(0);
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*') break;
				if (towupper(s[m]) != toupper((unsigned char)pat[n])) {
					j++;
					goto f0;
				}
			}
		}
		if (towupper(s[j]) != toupper((unsigned char)pat[k])) {
			if (pat[k] == '*') {
				k--;
				j--;
			} else
				break;
		}
	}
	if (towupper(s[j]) == toupper((unsigned char)pat[k]))
		return(1);
	else
		return(0);
}

/* patmat(s, pat) does pattern matching of pattern "pat" to string "s".
   "pat" may contain meta characters "_", "*", "%" and "\". "_" means match
   any one character, "*" means match zero or more characters, "%" means match
   zero or more characters and delete the matched part from string "s", "\" is
   used to specify meta characters as litteral characters. The value returned
   is 1 if there is a match, and 0 otherwise.
*/
int cUStr::patmat(unCH *s, const unCH *pat) {
	register int j, k;
	int n, m, t, l;
	unCH *lf;

	if (s[0] == EOS) {
		return(pat[0] == s[0]);
	}
	l = cUStr::strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if ((s[j] == '(' || s[j] == ')') && *utterance->speaker != '%') {
			if (s[j] == ')' && (pat[k] == '*' || pat[k] == '%') && pat[k+1] == ')')
				k++;
			else if (pat[k] != s[j]) {
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
				continue; // any character
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
		} else if (pat[k] == '*') {		  // wildcard
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
					if ((pat[k]=='-' || pat[k]=='&') && pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS) {
					} else {
						for (; t < j; t++) {
							if (uS.ismorfchar(s,t, &dFnt, rootmorf, MBF))
								return(FALSE);
						}
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS/* && *utterance->speaker == '*'*/) {
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
				if ((s[m]== '(' || s[m]== ')') && *utterance->speaker != '%') {
					if (s[m]== ')' && (pat[n] == '*' || pat[n] == '%') && pat[n+1] == ')')
						n++;
					else
						n--;
				} else if (pat[n] == '*' || pat[n] == '%')
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
		} else if (pat[k] == '%') {		  // wildcard
			m = j;
			if (pat[++k] == '%') {
				k++;
				if (pat[k] == '\\')
					k++;
				if ((t=j - 1) < 0)
					t = 0;
			} else {
				if (pat[k] == '\\')
					k++;
				t = j;
			}
f2:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					if (RemPercWildCard) {
						lf = uS.sp_cp(s+t,s+j);
						s = s+l;
						if (lf != s) {
							while (lf != s)
								*lf++ = ' ';
						}
					} else
						uS.sp_mod(s+t,s+j);
					return(TRUE);
				} else {
					for (; m < j; m++) {
						if (uS.ismorfchar(s, m, &dFnt, rootmorf, MBF))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS/* && *utterance->speaker == '*'*/) {
						if (RemPercWildCard) {
							lf = uS.sp_cp(s+t,s+j);
							s = s+l;
							if (lf != s) {
								while (lf != s)
									*lf++ = ' ';
							}
						} else
							uS.sp_mod(s+t,s+j);
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && *utterance->speaker != '%') {
					if (s[m]== ')' && (pat[n] == '*' || pat[n] == '%') && pat[n+1] == ')')
						n++;
					else
						n--;
				} else if (pat[n] == '*' || pat[n] == '%')
					break;
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
			if (RemPercWildCard) {
				lf = uS.sp_cp(s+t,s+j);
				j = t;
			} else
				uS.sp_mod(s+t,s+j);
		}

		if (s[j] != pat[k]) {
			if (pat[k+1] == '%' && pat[k+2] == '%') {
				if (s[j] == EOS && pat[k+3] == EOS/* && *utterance->speaker == '*'*/) {
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

int cUStr::isPlusMinusWord(const unCH *ch, int pos) {
	while (pos >= 0 && 
		   (uS.isRightChar(ch,pos,'/',&dFnt,MBF) || uS.isRightChar(ch,pos,'<',&dFnt,MBF) ||
			uS.isRightChar(ch,pos,'.',&dFnt,MBF) || uS.isRightChar(ch,pos,'!',&dFnt,MBF) ||
			uS.isRightChar(ch,pos,'?',&dFnt,MBF) || uS.isRightChar(ch,pos,',',&dFnt,MBF) ||
			uS.isRightChar(ch,pos,'"',&dFnt,MBF) || uS.isRightChar(ch,pos,'^',&dFnt,MBF) ||
			uS.isRightChar(ch,pos,'+',&dFnt,MBF) || uS.isRightChar(ch,pos,'=',&dFnt,MBF)))
		pos--;
	pos++;
	if (uS.isRightChar(ch, pos, '-', &dFnt, MBF) || uS.isRightChar(ch, pos, '+', &dFnt, MBF))
		return(TRUE);
	else
		return(FALSE);
}

char cUStr::isConsonant(const unCH *st) {
	if (uS.mStrnicmp(st,"b",1)==0 || uS.mStrnicmp(st,"c",1)==0 ||
		uS.mStrnicmp(st,"d",1)==0 || uS.mStrnicmp(st,"f",1)==0 ||
		uS.mStrnicmp(st,"g",1)==0 || uS.mStrnicmp(st,"h",1)==0 ||
		uS.mStrnicmp(st,"k",1)==0 || uS.mStrnicmp(st,"l",1)==0 ||
		uS.mStrnicmp(st,"m",1)==0 || uS.mStrnicmp(st,"n",1)==0 ||
		uS.mStrnicmp(st,"p",1)==0 || uS.mStrnicmp(st,"r",1)==0 ||
		uS.mStrnicmp(st,"s",1)==0 || uS.mStrnicmp(st,"t",1)==0 ||
		uS.mStrnicmp(st,"w",1)==0 || uS.mStrnicmp(st,"z",1)==0)
		return(TRUE);
	return(FALSE);
}

char cUStr::isVowel(const unCH *st) {
	if (uS.mStrnicmp(st,"a",1)==0 || uS.mStrnicmp(st,"e",1)==0 ||
		uS.mStrnicmp(st,"i",1)==0 || uS.mStrnicmp(st,"o",1)==0 ||
		uS.mStrnicmp(st,"u",1)==0 || uS.mStrnicmp(st,"y",1)==0)
		return(TRUE);
	return(FALSE);
}

// Non UTF16

int cUStr::sprintf(char *st, const char *format, ...) {
	int t;
	va_list args;

	va_start(args, format); 	// prepare the arguments
	t = vsprintf(st, format, args);
	va_end(args);				// clean the stack
	return(t);
}

#endif // !UNX


size_t cUStr::strlen(const char *st) {
	size_t len = 0;

	for (; *st != EOS; st++)
		len++;
	return(len);
}

char *cUStr::strcpy(char *des, const char *src) {
	char *org = des;

	while (*src != EOS) {
		*des++ = *src++;
	}
	*des = EOS;
	return(org);
}

char *cUStr::strncpy(char *des, const char *src, size_t num) {
	long i = 0L;

	while (i < num && src[i] != EOS) {
		des[i] = src[i];
		i++;
	}
	des[i] = EOS;
	return(des);
}

char *cUStr::strcat(char *des, const char *src) {
	long i;

	i = cUStr::strlen(des);
	while (*src != EOS) {
		des[i] = *src++;
		i++;
	}
	des[i] = EOS;
	return(des);
}

char *cUStr::strncat(char *des, const char *src, size_t num) {
	long i, j = 0L;

	i = cUStr::strlen(des);
	while (j < num && *src != EOS) {
		des[i] = *src++;
		i++;
		j++;
	}
	des[i] = EOS;
	return(des);
}

int cUStr::strcmp(const char *st1, const char *st2) {
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (*st1 > *st2)
		return(1);
	else
		return(-1);
	//	return(::strcmp(st1, st2));
}

int cUStr::strncmp(const char *st1, const char *st2, size_t len) {
	return(::strncmp(st1, st2, len));
}

char *cUStr::strchr(const char *src, int c) {
	return((char *)::strchr(src, c));
}

char *cUStr::strrchr(const char *src, int c) {
	return((char *)::strrchr(src, c));
}

char *cUStr::strpbrk(char *src, const char *cs) {
	return(::strpbrk(src, cs));
}

char *cUStr::strtok(char *src, const char *cs) {
	return(::strtok(src, cs));
}

char *cUStr::strstr(char *src, const char *cs) {
	return(::strstr(src, cs));
}

size_t cUStr::strspn(char *src, const char *cs) {
	return(::strspn(src, cs));
}

void *cUStr::memset(void *ptr, int val, size_t num) {
	return(::memset(ptr, val, num));
}

void *cUStr::memchr(const void *ptr, int val, size_t num) {
	return((void *)::memchr(ptr, val, num));
}

int cUStr::memcmp(const void *ptr1, const void *ptr2, size_t num) {
	return(::memcmp(ptr1, ptr2, num));
}

void *cUStr::memcpy(void *ptr1, const void *ptr2, size_t num) {
	return(::memcpy(ptr1, ptr2, num));
}

void *cUStr::memmove(void *ptr1, const void *ptr2, size_t num) {
	return(::memmove(ptr1, ptr2, num));
}

int cUStr::atoi(const char *s) {
	return(::atoi(s));
}

long cUStr::atol(const char *s) {
	return(::atol(s));
}

FNType *cUStr::str2FNType(FNType *des, long offset, const char *src) {
	FNType *org = des;

	if (offset > 0L)
		des = des + offset;

	while (*src != EOS) {
		*des = (FNType)*src++;
		des++;
	}
	*des = EOS;

	return(org);
}

FNType *cUStr::strn2FNType(FNType *des, long offset, const char *src, long len) {
	FNType *org = des;

	if (offset > 0L)
		des = des + offset;

	while (*src != EOS && len > 0) {
		*des = (FNType)*src++;
		des++;
		len--;
	}
	*des = EOS;

	return(org);
}

char *cUStr::FNType2str(char *des, long offset, const FNType *src) {
	char *org = des;

	if (offset > 0L)
		des = des + offset;

	while (*src != EOS) {
		*des = (char)*src++;
		des++;
	}
	*des = EOS;

	return(org);
}

int cUStr::FNTypecmp(const FNType *st1, const char *st2, long len) {
	if (len <= 0L) {
		for (; *st1 == (FNType)*st2 && *st2 != EOS; st1++, st2++) ;
		if (*st1 == EOS && *st2 == EOS)
			return(0);
		else if (*st1 > (FNType)*st2)
			return(1);
		else
			return(-1);
	} else {
		for (; *st1 == (FNType)*st2 && *st2 != EOS && len > 0; st1++, st2++, len--) ;
		if ((*st1 == EOS && *st2 == EOS) || len <= 0)
			return(0);
		else if (*st1 > (FNType)*st2)
			return(1);
		else
			return(-1);	
	}
}

int cUStr::FNTypeicmp(const FNType *st1, const char *st2, long len) {
	if (len <= 0L) {
		for (; toupper(*st1) == toupper(*st2) && *st2 != EOS; st1++, st2++) ;
		if (*st1 == EOS && *st2 == EOS)
			return(0);
		else if (toupper(*st1) > toupper(*st2))
			return(1);
		else
			return(-1);
	} else {
		for (; toupper(*st1) == toupper(*st2) && *st2 != EOS && len > 0; st1++, st2++, len--) ;
		if ((*st1 == EOS && *st2 == EOS) || len <= 0)
			return(0);
		else if (toupper(*st1) > toupper(*st2))
			return(1);
		else
			return(-1);	
	}
}

/*
#ifdef isFNTYPEUNIQUE
int cUStr::sprintf(FNType *st, const char *format, ...) {
	int t;
	va_list args;

	va_start(args, format); 	// prepare the arguments
	t = vsprintf((char *)st, format, args);
	va_end(args);				// clean the stack
	return(t);
}

int cUStr::strcmp(const FNType *st1, const FNType *st2) {
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (*st1 > *st2)
		return(1);
	else
		return(-1);	
}

int cUStr::mStricmp(const FNType *st1, const FNType *st2) {
	for (; toupper(*st1) == toupper(*st2) && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (toupper(*st1) > toupper(*st2))
		return(1);
	else
		return(-1);	
}

int cUStr::mStrnicmp(const FNType *st1, const FNType *st2, size_t len) {
	for (; toupper(*st1) == toupper(*st2) && *st2 != EOS && len > 0; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0)
		return(0);
	else if (toupper(*st1) > toupper(*st2))
		return(1);
	else
		return(-1);	
}

size_t cUStr::strlen(const FNType *st) {
	size_t len = 0;

	for (; *st != EOS; st++)
		len++;
	return(len);
}

FNType *cUStr::strcpy(FNType *des, const FNType *src) {
	FNType *org = des;

	while (*src != EOS) {
		*des++ = *src++;
	}
	*des = EOS;
	return(org);
}

FNType *cUStr::strncpy(FNType *des, const FNType *src, size_t num) {
	long i = 0L;

	while (i < num && src[i] != EOS) {
		des[i] = src[i];
		i++;
	}
	des[i] = EOS;
	return(des);
}

FNType *cUStr::strcat(FNType *des, const FNType *src) {
	long i;

	i = cUStr::strlen(des);
	while (*src != EOS) {
		des[i] = *src++;
		i++;
	}
	des[i] = EOS;
	return(des);
}

FNType *cUStr::strchr(FNType *src, int c) {
	for (; *src != c && *src != EOS; src++) ;
	if (*src == EOS)
		return(NULL);
	else
		return(src);
}

FNType *cUStr::strrchr(FNType *src, int c) {
	long i;
	
	i = cUStr::strlen(src);
	for (; i >= 0 && src[i] != c; i--) ;
	if (src[i] == c)
		return(src+i);
	else
		return(NULL);
}

long cUStr::lowercasestr(FNType *str, NewFontInfo *finfo, char MBC) {
	long count;

	count = 0L;
	for (; *str != EOS; str++) {
		if (iswupper(*str)) {
			count++;
			*str = towlower(*str);
		}
	}
	return(count);
}

long cUStr::uppercasestr(FNType *str, NewFontInfo *finfo, char MBC) {
	long count;

	count = 0L;
	for (; *str != EOS; str++) {
		if (iswlower(*str)) {
			count++;
			*str = towupper(*str);
		}
	}
	return(count);
}
char cUStr::fpatmat(const FNType *s, const FNType *pat) {
	register int j, k;
	int n, m;

	if (s[0] == EOS)
		return(pat[0] == s[0]);
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {
			k++;
f0:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) return(1);
				else return(0);
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*') break;
				if (s[m] != pat[n]) {
					j++;
					goto f0;
				}
			}
		}
		if (s[j] != pat[k]) {
			if (pat[k] == '*') {
				k--;
				j--;
			} else
				break;
		}
	}
	if (pat[k] == s[j]) return(1);
	else return(0);
}

#endif // isFNTYPEUNIQUE
*/

int cUStr::mStricmp(const char *st1, const char *st2) {
	for (; toupper((unsigned char)*st1) == toupper((unsigned char)*st2) && *st2 != EOS; st1++, st2++) ;
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (toupper((unsigned char)*st1) > toupper((unsigned char)*st2))
		return(1);
	else
		return(-1);	
}

int cUStr::mStrnicmp(const char *st1, const char *st2, size_t len) {
	for (; toupper((unsigned char)*st1) == toupper((unsigned char)*st2) && *st2 != EOS && len > 0L; st1++, st2++, len--) ;
	if ((*st1 == EOS && *st2 == EOS) || len <= 0L)
		return(0);
	else if (toupper((unsigned char)*st1) > toupper((unsigned char)*st2))
		return(1);
	else
		return(-1);	
}

static char isskip_char(const char chr, const char *TempPunctPtr) {
	if (isalnum((unsigned char)chr)) return(FALSE);
	if (isspace((unsigned char)chr)) return(TRUE);
	while (*TempPunctPtr != chr && *TempPunctPtr != EOS)
		TempPunctPtr++;
	return(*TempPunctPtr != EOS);
}

char cUStr::isskip(const char *org, int pos, NewFontInfo *finfo, char MBC) {
	register char chr;
	register const char *TempPunctPtr;

	if (MBC) {
		if (my_CharacterByteType(org, (short)pos, finfo) != 0)
			return(FALSE);
	}
	chr = org[pos];
	if (chr == sMarkChr || chr == dMarkChr) return(TRUE);
	if (isalnum((unsigned char)chr)) return(FALSE);
	if (isspace((unsigned char)chr) || chr == '\n' || chr == '\r') return(TRUE);
	if (chr == '.') {
		if (uS.isPause(org, pos, NULL, NULL))
			return(FALSE);
	}
	TempPunctPtr = punctuation;
	while (*TempPunctPtr != chr && *TempPunctPtr != EOS)
		TempPunctPtr++;
	if (*TempPunctPtr != EOS && chr == '+') {
		if (pos > 0 && isskip_char(org[pos-1], punctuation))
			return(FALSE);
	}
	return(*TempPunctPtr != EOS);
}

char cUStr::ismorfchar(const char *org, int pos, NewFontInfo *finfo, const char *morfsList, char MBC) {
	register char chr, nextChr, prevChr;
	const char *morf;

	if (morfsList == NULL)
		return(FALSE);
	chr = org[pos];
	if (pos > 0)
		prevChr = org[pos-1];
	else
		prevChr = 0;
	nextChr = org[pos+1];
	if (MBC) {
		if (my_CharacterByteType(org, (short)pos, finfo) != 0)
			return(FALSE);
	}
	for (morf=morfsList; *morf; morf++) {
		if (*morf == chr && nextChr != '0') {
			if (chr != '+' || prevChr != '|')
				return(TRUE);
		}
	}
	return(FALSE);
}

char cUStr::isCharInMorf(char c, char *morf) {
	for (; *morf; morf++) 
		if (c == *morf) return(TRUE);
	return(FALSE);
}

char cUStr::atUFound(const char *w, int s, NewFontInfo *finfo, char MBC) {
	while (w[s] != EOS) {
		if (uS.isRightChar(w, s, '<', finfo, MBC) || uS.isRightChar(w, s, '>', finfo, MBC) || 
			uS.isRightChar(w, s, '[', finfo, MBC) || uS.isRightChar(w, s, ']', finfo, MBC) ||
			isSpace(w[s]) || w[s] == '\n')
			break;
		if (w[s] == '@') {
			if (w[s+1] != EOS && w[s+1] == 'u' && (w[s+2] == EOS || !isalnum((unsigned char)w[s+2]))) 
				return(TRUE);
		}
		s++;
	}
	return(FALSE);
}

char cUStr::isRightChar(const char *org, long pos, register char chr, NewFontInfo *finfo, char MBC) {
	if (MBC) {
		if (my_CharacterByteType(org, (short)pos, finfo) != 0)
			return(FALSE);
	}
	if (org[pos] == chr)
		return(TRUE);
	return(FALSE);
}

char cUStr::isUpperChar(char *org, int pos, NewFontInfo *finfo, char MBC) {
#ifndef UNX
	if (finfo->isUTF && pos == 0 && !UTF8_IS_SINGLE((unsigned char)org[0])) {
		UTF8ToUnicode((unsigned char *)org, cUStr::strlen(org), templineW, NULL, UTTLINELEN);
		if (MyUpperChars(templineW[0]))
			return(TRUE);
		else
			return(iswupper(templineW[0]) != 0);
	} else {
#endif
		if (MBC) {
			if (my_CharacterByteType(org, (short)pos, finfo) != 0)
				return(FALSE);
		}
		return(isupper((unsigned char)org[pos]) != 0);
#ifndef UNX
	}
#endif
}

char cUStr::isSqBracketItem(const char *s, int pos, NewFontInfo *finfo, char MBC) {
	for (; s[pos] && !uS.isRightChar(s, pos, '[', finfo, MBC) && !uS.isRightChar(s, pos, ']', finfo, MBC); pos++) ;
	if (uS.isRightChar(s, pos, ']', finfo, MBC))
		return(TRUE);
	else
		return(FALSE);
}

char cUStr::isSqCodes(const char *word, char *tWord, NewFontInfo *finfo, char isForce) {
	int len;

	if (!isForce && word[0] != EOS) {
		len = cUStr::strlen(word) - 1;
		if (!uS.isRightChar(word, 0, '[', finfo, TRUE) ||
			!uS.isRightChar(word, len, ']', finfo, TRUE))
			return(FALSE);
	}
	while (*word != EOS) {
		if (isspace(*word)) {
			*tWord++ = ' ';
			for (word++; isspace(*word) && *word; word++) ;
		} else {
			*tWord++ = *word++;
		}
	}
	*tWord = EOS;
	return(TRUE);
}

void removeAllSpaces(char *st) {
	while (*st != EOS) {
		if (*st == '\t' || *st == ' ' || *st == '\n')
			uS.strcpy(st, st+1);
		else
			st++;
	}
}

void removeExtraSpace(char *st) {
	int i;

	for (i=0; st[i] != EOS; ) {
		if (st[i]==' ' || st[i]=='\t' || (st[i]=='<' && (i==0 || st[i-1]==' ' || st[i-1]=='\t'))) {
			i++;
			while (st[i] == ' ' || st[i] == '\t')
				uS.strcpy(st+i, st+i+1);
		} else
			i++;
	}
}

void cUStr::remblanks(char *st) {
	register int i;

	i = cUStr::strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == '\r'))
		i--;
	if (st[i+1] != EOS)
		st[i+1] = EOS;
}

void cUStr::remFrontAndBackBlanks(char *st) {
	register int i;

	for (i=0; isSpace(st[i]) || st[i] == '\n' || st[i] == '\r'; i++) ;
	if (i > 0)
		cUStr::strcpy(st, st+i);
	i = cUStr::strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == '\r' || st[i] == NL_C || st[i] == SNL_C))
		i--;
	st[i+1] = EOS;
}

void cUStr::shiftright(char *st, int num) {
	register int i;

	for (i=cUStr::strlen(st); i >= 0; i--)
		st[i+num] = st[i];
}

void cUStr::cleanUpCodes(char *code, NewFontInfo *finfo, char MBC) {
	long i;
	
	for (i=0L; code[i] != EOS; i++) {
		if (uS.isRightChar(code, i, '\t', finfo, MBC) || 
			uS.isRightChar(code, i, '\n', finfo, MBC)) {
			code[i] = ' ';
		}
	}
}

// uS.extractString(tempst3, line, "[%add: ", ']');
void cUStr::extractString(char *out, const char *line, const char *type, char endC) {
	int i, j;

	for (i=cUStr::strlen(type); isSpace(line[i]); i++) ;
	for (j=0; line[i] != endC && line[i] != EOS; i++, j++)
		out[j] = line[i];
	out[j] = EOS;
	uS.remblanks(out);
}

int cUStr::isToneUnitMarker(char *word) {
/*
	if (!::strcmp(word, "-?")  || !::strcmp(word, "-!")  || !::strcmp(word, "-.") || 
		!::strcmp(word, "-'.") || !::strcmp(word, "-,.") || !::strcmp(word, "-,") || 
		!::strcmp(word, "-'")  || !::strcmp(word, "-_")  || !::strcmp(word, "-:") ||
		!::strcmp(word, "-"))
		return(TRUE);
	else
*/
		return(FALSE);
}

int cUStr::IsCAUtteranceDel(char *st, int pos) {
	if (UTF8_IS_LEAD((unsigned char)st[pos]) && st[pos] == (char)0xE2) {
		if (st[pos+1] == (char)0x86) {
			if (st[pos+2] == (char)0x92) { // level
				return(1);
			} else if (st[pos+2] == (char)0x97) { // rise to mid
				return(1);
			} else if (st[pos+2] == (char)0x98) { // fall to mid
				return(1);
			}
		} else if (st[pos+1] == (char)0x87) {
			if (st[pos+2] == (char)0x97) { // rise to high
				return(1);
			} else if (st[pos+2] == (char)0x98) { // fall to low
				return(1);
			}
		} else if (st[pos+1] == (char)0x88) {
			if (st[pos+2] == (char)0x9E) { // unmarked ending
				return(1);
			}
		}
	}
	return(0);
}

int cUStr::IsCharUtteranceDel(char *st, int pos) {
	if (st[pos] == '.' || st[pos] == '?' || st[pos] == '!' ||
		st[pos] == '+' || st[pos] == '/' || st[pos] == '"') {
		return(1);
	}
	return(0);
}

int cUStr::IsUtteranceDel(const char *st, int pos) {
	if (st[pos] == '.') {
		if (uS.isPause(st, pos, NULL, NULL))
			return(0);
		else {
			for (; pos >= 0 && !isSpace(st[pos]) && st[pos] != '[' && st[pos] != ']'; pos--) ;
			if (pos >= 0 && st[pos] == '[')
				return(0);
			else
				return(1);
		}
	} else if (st[pos] == '?' || st[pos] == '!') {
		for (; pos >= 0 && !isSpace(st[pos]) && st[pos] != '[' && st[pos] != ']'; pos--) ;
		if (pos >= 0 && st[pos] == '[')
			return(0);
		else
			return(1);
	}
	return(0);
}

int cUStr::isPause(const char *st, int posO, int *beg, int *end) {
	int pos;

	if (end != NULL) {
		for (pos=posO+1; isdigit(st[pos]) || st[pos] == '.' || st[pos] == ':'; pos++) ;
		if (!uS.isRightChar(st,pos,')',&dFnt,MBF))
			return(FALSE);
		*end = pos;
	} else {
		for (pos=posO-1; pos >= 0 && (isdigit(st[pos]) || st[pos] == '.' || st[pos] == ':'); pos--) ;
		if (pos < 0 || !uS.isRightChar(st,pos,'(',&dFnt,MBF))
			return(FALSE);
		if (beg != NULL)
			*beg = pos;
		for (pos=posO; isdigit(st[pos]) || st[pos] == '.' || st[pos] == ':'; pos++) ;
		if (!uS.isRightChar(st,pos,')',&dFnt,MBF))
			return(FALSE);
	}
	return(TRUE);
}

long cUStr::lowercasestr(char *str, NewFontInfo *finfo, char MBC) {
	long count;

	count = 0L;
	if (MBC) {
		register int pos;

		for (pos=0; str[pos] != EOS; pos++) {
			if (my_CharacterByteType(str, (short)pos, finfo) == 0) {
				if (isupper((unsigned char)str[pos])) {
					count++;
					str[pos] = (char)tolower((unsigned char)str[pos]);
				}
			}
		}
	} else {
		for (; *str != EOS; str++) {
			if (isupper((unsigned char)*str)) {
				count++;
				*str = (char)tolower((unsigned char)*str);
			}
		}
	}
	return(count);
}

long cUStr::uppercasestr(char *str, NewFontInfo *finfo, char MBC) {
	long count;

	count = 0L;
	if (MBC) {
		register int pos;

		for (pos=0; str[pos] != EOS; pos++) {
			if (my_CharacterByteType(str, (short)pos, finfo) == 0) {
				if (islower((unsigned char)str[pos])) {
					count++;
					str[pos] = (char)toupper((unsigned char)str[pos]);
				}
			}
		}
	} else {
		for (; *str != EOS; str++) {
			if (islower((unsigned char)*str)) {
				count++;
				*str = (char)toupper((unsigned char)*str);
			}
		}
	}
	return(count);
}

char *cUStr::sp_cp(char *s1, char *s2) {
	if (s1 != s2) {
		while (*s2)
			*s1++ = *s2++;
		*s1 = EOS;
		return(s1);
	} else
		return(s1+cUStr::strlen(s2));
}

void cUStr::sp_mod(char *s1, char *s2) {
	if (s1 != s2) {
		while (s1 != s2)
			*s1++ = '\001';
	}
}

char cUStr::isUTF8(const char *str) {
	if (str[0] == (char)0xef && str[1] == (char)0xbb && str[2] == (char)0xbf &&
		str[3] == '@' && str[4] == 'U' && str[5] == 'T' && str[6] == 'F' && str[7] == '8')
		return(TRUE);
	else if (str[0] == '@' && str[1] == 'U' && str[2] == 'T' && str[3] == 'F' && str[4] == '8')
		return(TRUE);
	else if (!chatmode && str[0] == '@' && str[1] == 'u' && str[2] == 't' && str[3] == 'f' && str[4] == '8')
		return(TRUE);
	else
		return(FALSE);
}

char cUStr::isInvisibleHeader(const char *str) {
	if (uS.partcmp(str, PIDHEADER, FALSE, FALSE) || uS.partcmp(str, CKEYWORDHEADER, FALSE, FALSE) ||
		uS.partcmp(str, WINDOWSINFO, FALSE, FALSE) || uS.partcmp(str, FONTHEADER, FALSE, FALSE))
		return(TRUE);
	else
		return(FALSE);
}

int cUStr::HandleCAChars(char *w, int *matchedType) { // CA CHARS
	int offset;

	if (matchedType != NULL)
		*matchedType = CA_NOT_ALL;

	if (UTF8_IS_LEAD((unsigned char)*w) && *w == (char)0xE1) {
		if (*(w+1) == (char)0xBC) {
			if (*(w+2) == (char)0xA9) { // laugh in a word
				if (matchedType != NULL)
					*matchedType = CA_APPLY_LAUGHINWORD;
				return(3);
			}
		}
	} else if (UTF8_IS_LEAD((unsigned char)*w) && *w == (char)0xE2) {
		if (*(w+1) == (char)0x80) {
			if (*(w+2) == (char)0x9C) { // open quote “ - NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_OPEN_QUOTE;
				return(3);
			} else if (*(w+2) == (char)0x9D) { // close quote ” - NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_CLOSE_QUOTE;
				return(3);
			} else if (*(w+2) == (char)0x9E) { // Tag or sentence final particle; „ - NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_DOUBLE_COMMA;
				return(3);
			} else if (*(w+2) == (char)0xA1) { // Vocative or summons - ‡ - NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_VOCATIVE;
				return(3);
			} else if (*(w+2) == (char)0xA6) { // %pho missing word -…- NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_MISSINGWORD;
				return(3);
			} else if (*(w+2) == (char)0xB9) { // Group start marker - NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_GROUP_START;
				return(3);
			} else if (*(w+2) == (char)0xBA) { // Group end marker - NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_GROUP_END;
				return(3);
			}
		} else if (*(w+1) == (char)0x81) {
			if (*(w+2) == (char)0x8E) { // creaky - big asterisk
				if (matchedType != NULL)
					*matchedType = CA_APPLY_CREAKY;
				return(3);
			} else if (*(w+2) == (char)0x87) { // unsure
				if (matchedType != NULL)
					*matchedType = CA_APPLY_UNSURE;
				return(3);
			}
		} else if (*(w+1) == (char)0x86) {
			if (*(w+2) == (char)0x91) { // up-arrow
				if (matchedType != NULL)
					*matchedType = CA_APPLY_SHIFT_TO_HIGH_PITCH;
				return(3);
			} else if (*(w+2) == (char)0x92) { // level
				if (matchedType != NULL)
					*matchedType = CA_APPLY_LEVEL;
				return(3);
			} else if (*(w+2) == (char)0x93) { // down-arrow
				if (matchedType != NULL)
					*matchedType = CA_APPLY_SHIFT_TO_LOW_PITCH;
				return(3);
			} else if (*(w+2) == (char)0x97) { // rise to mid
				if (matchedType != NULL)
					*matchedType = CA_APPLY_RISETOMID;
				return(3);
			} else if (*(w+2) == (char)0x98) { // fall to mid
				if (matchedType != NULL)
					*matchedType = CA_APPLY_FALLTOMID;
				return(3);
			} else if (*(w+2) == (char)0xAB) { // left arrow with circle
				if (matchedType != NULL)
					*matchedType = NOTCA_LEFT_ARROW_CIRCLE;
				return(3);
			} else if (*(w+2) == (char)0xBB) { // pitch reset
				if (matchedType != NULL)
					*matchedType = CA_APPLY_PITCH_RESET;
				return(3);
			}
		} else if (*(w+1) == (char)0x87) {
			if (*(w+2) == (char)0x97) { // rise to high
				if (matchedType != NULL)
					*matchedType = CA_APPLY_RISETOHIGH;
				return(3);
			} else if (*(w+2) == (char)0x98) { // fall to low
				if (matchedType != NULL)
					*matchedType = CA_APPLY_FALLTOLOW;
				return(3);
			}
		} else if (*(w+1) == (char)0x88) {
			if (*(w+2) == (char)0x86) { // faster
				if (matchedType != NULL)
					*matchedType = CA_APPLY_FASTER;
				return(3);
			} else if (*(w+2) == (char)0x87) { // slower
				if (matchedType != NULL)
					*matchedType = CA_APPLY_SLOWER;
				return(3);
			} else if (*(w+2) == (char)0x99) { // inhalation - raised period
				if (matchedType != NULL)
					*matchedType = CA_APPLY_INHALATION;
				return(3);
			} else if (*(w+2) == (char)0x9E) { // unmarked ending
				if (matchedType != NULL)
					*matchedType = CA_APPLY_UNMARKEDENDING;
				return(3);
			} else if (*(w+2) == (char)0xAC) { // whisper
				if (matchedType != NULL)
					*matchedType = CA_APPLY_WHISPER;
				return(3);
			} else if (*(w+2) == (char)0xAE) { // singing
				if (matchedType != NULL)
					*matchedType = CA_APPLY_SINGING;
				return(3);
			} else if (*(w+2) == (char)0xBE) { // constriction
				if (matchedType != NULL)
					*matchedType = CA_APPLY_CONSTRICTION;
				return(3);
			}
		} else if (*(w+1) == (char)0x89) {
			if (*(w+2) == (char)0x88) { // latching
				if (matchedType != NULL)
					*matchedType = CA_APPLY_LATCHING;
				return(3);
			} else if (*(w+2) == (char)0x8B) { // continuation
				if (matchedType != NULL)
					*matchedType = CA_APPLY_CONTINUATION;
				return(3);
			} else if (*(w+2) == (char)0xA0) { // crossed equal
				if (matchedType != NULL)
					*matchedType = NOTCA_CROSSED_EQUAL;
				return(3);
			} else if (*(w+2) == (char)0xA1) { // uptake
				if (matchedType != NULL)
					*matchedType = CA_APPLY_UPTAKE;
				return(3);
			}
		} else if (*(w+1) == (char)0x8C) {
			if (*(w+2) == (char)0x88) { // raised [
				for (offset=3; isdigit(w[offset]); offset++) ;
				if (matchedType != NULL)
					*matchedType = CA_APPLY_OPTOPSQ;
				return(offset);
			} else if (*(w+2) == (char)0x89) { // raised ]
				for (offset=3; isdigit(w[offset]); offset++) ;
				if (matchedType != NULL)
					*matchedType = CA_APPLY_CLTOPSQ;
				return(offset);
			} else if (*(w+2) == (char)0x8A) { // lowered [
				for (offset=3; isdigit(w[offset]); offset++) ;
				if (matchedType != NULL)
					*matchedType = CA_APPLY_OPBOTSQ;
				return(offset);
			} else if (*(w+2) == (char)0x8B) { // lowered ]
				for (offset=3; isdigit(w[offset]); offset++) ;
				if (matchedType != NULL)
					*matchedType = CA_APPLY_CLBOTSQ;
				return(offset);
			}
		} else if (*(w+1) == (char)0x96) {
			if (*(w+2) == (char)0x81) { // low pitch - low bar
				if (matchedType != NULL)
					*matchedType = CA_APPLY_LOW_PITCH;
				return(3);
			} else if (*(w+2) == (char)0x94) { // high pitch - high bar
				if (matchedType != NULL)
					*matchedType = CA_APPLY_HIGH_PITCH;
				return(3);
			}
		} else if (*(w+1) == (char)0x97) {
			if (*(w+2) == (char)0x89) { // louder
				if (matchedType != NULL)
					*matchedType = CA_APPLY_LOUDER;
				return(3);
			}
		} else if (*(w+1) == (char)0x98) {
			if (*(w+2) == (char)0xBA) { // smile voice
				if (matchedType != NULL)
					*matchedType = CA_APPLY_SMILE_VOICE;
				return(3);
			}
		} else if (*(w+1) == (char)0x99) {
			if (*(w+2) == (char)0x8B) { // breathy-voice -♋- NOT CA
				if (matchedType != NULL)
					*matchedType = CA_BREATHY_VOICE;
				return(3);
			}
		}
	} else if (UTF8_IS_LEAD((unsigned char)*w) && *w == (char)0xE3) {
		if (*(w+1) == (char)0x80) {
			if (*(w+2) == (char)0x94) { // sign group start marker - NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_SIGN_GROUP_START;
				return(3);
			} else if (*(w+2) == (char)0x95) { // sign group end marker - NOT CA
				if (matchedType != NULL)
					*matchedType = NOTCA_SIGN_GROUP_END;
				return(3);
			}
		}
	} else if (UTF8_IS_LEAD((unsigned char)*w) && *w == (char)0xC2) {
		if (*(w+1) == (char)0xA7) { // precise
			if (matchedType != NULL)
				*matchedType = CA_APPLY_PRECISE;
			return(2);
		} else if (*(w+1) == (char)0xB0) { // softer
			if (matchedType != NULL)
				*matchedType = CA_APPLY_SOFTER;
			return(2);
		}
	} else if (UTF8_IS_LEAD((unsigned char)*w) && *w == (char)0xCA) {
		if (*(w+1) == (char)0x94) { // Glottal stop - ʔ - NOT CA
			if (matchedType != NULL)
				*matchedType = NOTCA_GLOTTAL_STOP;
			return(2);
		} else if (*(w+1) == (char)0x95) { // Hebrew glottal - ʕ - NOT CA
			if (matchedType != NULL)
				*matchedType = NOTCA_HEBREW_GLOTTAL;
			return(2);
		}
	} else if (UTF8_IS_LEAD((unsigned char)*w) && *w == (char)0xCB) {
		if (*(w+1) == (char)0x88) { // raised stroke - NOT CA
			if (matchedType != NULL)
				*matchedType = NOTCA_RAISED_STROKE;
			return(2);
		} else if (*(w+1) == (char)0x90) { // colon for long vowels ː - NOT CA
			if (matchedType != NULL)
				*matchedType = NOTCA_VOWELS_COLON;
			return(2);
		} else if (*(w+1) == (char)0x8C) { // lowered stroke - NOT CA
			if (matchedType != NULL)
				*matchedType = NOTCA_LOWERED_STROKE;
			return(2);
		}
	} else if (UTF8_IS_LEAD((unsigned char)*w) && *w == (char)0xCC) {
		if (*(w+1) == (char)0x84) { // Stress - ̄ - NOT CA
			if (matchedType != NULL)
				*matchedType = NOTCA_STRESS;
			return(2);
		} else if (*(w+1) == (char)0x8C) { // caron - ̌- NOT CA
			if (matchedType != NULL)
				*matchedType = NOTCA_CARON;
			return(2);
		} else if (*(w+1) == (char)0xB2) { // underline - NOT CA
			if (matchedType != NULL)
				*matchedType = NOTCA_UNDERLINE;
			return(2);
		}
	} else if (UTF8_IS_LEAD((unsigned char)*w) && *w == (char)0xCE) {
		if (*(w+1) == (char)0xAB) { // yawn
			if (matchedType != NULL)
				*matchedType = CA_APPLY_YAWN;
			return(2);
		}
	}
	return(0);
}

int cUStr::partcmp(const char *st1, const char *st2, char pat_match, char isCaseSenc) { // st1- full, st2- part
	if (pat_match) {
		int i, j, res;
		char t[SPEAKERLEN+1];
		if (*st1 == *st2) {
			for (i=cUStr::strlen(st1)-1; i >= 0 && (st1[i] == ' ' || st1[i] == '\t'); i--) ;
			for (j=cUStr::strlen(st2)-1; j >= 0 && (st2[j] == ' ' || st2[j] == '\t'); j--) ;
			if (st1[i] != ':' || st2[j] == ':' || i < 0)
				i++;
			if (i < SPEAKERLEN) {
				cUStr::strncpy(t, st1, i);
				t[i] = EOS;
				res = (int)uS.fIpatmat(t+1, st2+1);
				return(res);
			} else
				return(FALSE);
		} else
			return(FALSE);
	}
	if (isCaseSenc) {
		for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	} else {
		for (; toupper((unsigned char)*st1) == toupper((unsigned char)*st2) && *st2 != EOS; st1++, st2++) ;
	}
	if (*st2 == ':') return(!*st1);
	else return(!*st2);
}

int cUStr::partwcmp(const char *st1, const char *st2) { // st1- full, st2- part
	for (; *st1 == *st2 && *st2 != EOS; st1++, st2++) ;
	return(!*st2);
}

char cUStr::fpatmat(const char *s, const char *pat) {
	register int j, k;
	int n, m;

	if (s[0] == EOS)
		return(pat[0] == s[0]);
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {
			k++;
f0:
			while (s[j] && s[j] != pat[k])
				j++;
			if (!s[j]) {
				if (!pat[k]) return(1);
				else return(0);
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*') break;
				if (s[m] != pat[n]) {
					j++;
					goto f0;
				}
			}
		}
		if (s[j] != pat[k]) {
			if (pat[k] == '*') {
				k--;
				j--;
			} else
				break;
		}
	}
	if (pat[k] == s[j])
		return(1);
	else
		return(0);
}

char cUStr::fIpatmat(const char *s, const char *pat) {
	register int j, k;
	int n, m;

	if (s[0] == EOS)
		return(pat[0] == s[0]);
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {
			k++;
f0:
			while (s[j] && toupper((unsigned char)s[j]) != toupper((unsigned char)pat[k]))
				j++;
			if (!s[j]) {
				if (!pat[k]) return(1);
				else return(0);
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if (pat[n] == '*') break;
				if (toupper((unsigned char)s[m]) != toupper((unsigned char)pat[n])) {
					j++;
					goto f0;
				}
			}
		}
		if (toupper((unsigned char)s[j]) != toupper((unsigned char)pat[k])) {
			if (pat[k] == '*') {
				k--;
				j--;
			} else
				break;
		}
	}
	if (toupper((unsigned char)s[j]) == toupper((unsigned char)pat[k]))
		return(1);
	else
		return(0);
}

int cUStr::patmat(char *s, const char *pat) {
	register int j, k;
	int n, m, t, l;
	char *lf;

	if (s[0] == EOS) {
		return(pat[0] == s[0]);
	}
	l = cUStr::strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if ((s[j] == '(' || s[j] == ')') && *utterance->speaker != '%' && Parans == 21) {
			if (s[j] == ')' && (pat[k] == '*' || pat[k] == '%') && pat[k+1] == ')')
				k++;
			else if (pat[k] != s[j]) {
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
				continue; // any character
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
		} else if (pat[k] == '*') {	// wildcard
			k++; t = j;
			if (pat[k] == '\\')
				k++;
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
					if ((pat[k]=='-' || pat[k]=='&') && pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS) {
					} else {
						for (; t < j; t++) {
							if (uS.ismorfchar(s,t, &dFnt, rootmorf, MBF))
								return(FALSE);
						}
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS/* && *utterance->speaker == '*'*/) {
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
				if ((s[m]== '(' || s[m]== ')') && *utterance->speaker != '%') {
					if (s[m]== ')' && (pat[n] == '*' || pat[n] == '%') && pat[n+1] == ')')
						n++;
					else
						n--;
				} else if (pat[n] == '*' || pat[n] == '%')
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
		} else if (pat[k] == '%') {		  // wildcard
			m = j;
			if (pat[++k] == '%') {
				k++;
				if (pat[k] == '\\')
					k++;
				if ((t=j - 1) < 0)
					t = 0;
			} else {
				if (pat[k] == '\\')
					k++;
				t = j;
			}
f2:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					if (RemPercWildCard) {
						lf = uS.sp_cp(s+t,s+j);
						s = s+l;
						if (lf != s) {
							while (lf != s)
								*lf++ = ' ';
						}
					} else
						uS.sp_mod(s+t,s+j);
					return(TRUE);
				} else {
					for (; m < j; m++) {
						if (uS.ismorfchar(s, m, &dFnt, rootmorf, MBF))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS/* && *utterance->speaker == '*'*/) {
						if (RemPercWildCard) {
							lf = uS.sp_cp(s+t,s+j);
							s = s+l;
							if (lf != s) {
								while (lf != s)
									*lf++ = ' ';
							}
						} else
							uS.sp_mod(s+t,s+j);
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && *utterance->speaker != '%') {
					if (s[m]== ')' && (pat[n] == '*' || pat[n] == '%') && pat[n+1] == ')')
						n++;
					else
						n--;
				} else if (pat[n] == '*' || pat[n] == '%')
					break;
				else if (pat[n] == '_') {
					if (isspace(s[m])) {
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
			if (RemPercWildCard) {
				lf = uS.sp_cp(s+t,s+j);
				j = t;
			} else
				uS.sp_mod(s+t,s+j);
		}

		if (s[j] != pat[k]) {
			if (pat[k+1] == '%' && pat[k+2] == '%') {
				if (s[j] == EOS && pat[k+3] == EOS/* && *utterance->speaker == '*'*/) {
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

int cUStr::isPlusMinusWord(const char *ch, int pos) {
	while (pos >= 0 && 
		   (uS.isRightChar(ch,pos,'/',&dFnt,MBF) || uS.isRightChar(ch,pos,'<',&dFnt,MBF) ||
			uS.isRightChar(ch,pos,'.',&dFnt,MBF) || uS.isRightChar(ch,pos,'!',&dFnt,MBF) ||
			uS.isRightChar(ch,pos,'?',&dFnt,MBF) || uS.isRightChar(ch,pos,',',&dFnt,MBF) ||
			uS.isRightChar(ch,pos,'"',&dFnt,MBF) || uS.isRightChar(ch,pos,'^',&dFnt,MBF) ||
			uS.isRightChar(ch,pos,'+',&dFnt,MBF) || uS.isRightChar(ch,pos,'=',&dFnt,MBF)))
		pos--;
	pos++;
	if (uS.isRightChar(ch, pos, '-', &dFnt, MBF) || uS.isRightChar(ch, pos, '+', &dFnt, MBF))
		return(TRUE);
	else
		return(FALSE);
}

char cUStr::isConsonant(const char *st) {
	if (uS.mStrnicmp(st,"b",1)==0 || uS.mStrnicmp(st,"c",1)==0 ||
		uS.mStrnicmp(st,"d",1)==0 || uS.mStrnicmp(st,"f",1)==0 ||
		uS.mStrnicmp(st,"g",1)==0 || uS.mStrnicmp(st,"h",1)==0 ||
		uS.mStrnicmp(st,"k",1)==0 || uS.mStrnicmp(st,"l",1)==0 ||
		uS.mStrnicmp(st,"m",1)==0 || uS.mStrnicmp(st,"n",1)==0 ||
		uS.mStrnicmp(st,"p",1)==0 || uS.mStrnicmp(st,"r",1)==0 ||
		uS.mStrnicmp(st,"s",1)==0 || uS.mStrnicmp(st,"t",1)==0 ||
		uS.mStrnicmp(st,"w",1)==0 || uS.mStrnicmp(st,"z",1)==0)
		return(TRUE);
	return(FALSE);
}

char cUStr::isVowel(const char *st) {
	if (uS.mStrnicmp(st,"a",1)==0 || uS.mStrnicmp(st,"e",1)==0 ||
		uS.mStrnicmp(st,"i",1)==0 || uS.mStrnicmp(st,"o",1)==0 ||
		uS.mStrnicmp(st,"u",1)==0 || uS.mStrnicmp(st,"y",1)==0)
		return(TRUE);
	return(FALSE);
}
/*	 strings manipulation routines end   */
