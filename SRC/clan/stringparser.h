/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#ifndef STRINGPARSERDEF
#define STRINGPARSERDEF

#include "wstring.h"

extern "C"
{
#ifndef EOS			/* End Of String marker			 */
	#define EOS 0 // '\0'
#endif
#ifndef TRUE
	#define TRUE  1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#define PUNCTUATION_SET ",[]<>;.?!"
#define PUNCT_PHO_MOD_SET ""


extern const char *punctuation;/* contains main text line punctuation set	  */
	
class cUStr
{
public:

#ifndef UNX
	int sprintf(wchar_t *st, const wchar_t *format, ...);
	int sprintf(char *st, const char *format, ...);
	int strcmp(const wchar_t *st1, const wchar_t *st2);
	int strcmp(const wchar_t *st1, const char *st2);
	int strcmp(const char *st1, const char *st2);
	int strncmp(const wchar_t *st1, const wchar_t *st2, size_t len);
	int strncmp(const wchar_t *st1, const char *st2, size_t len);
	int strncmp(const char *st1, const char *st2, size_t len);
	int mStricmp(const wchar_t *st1, const wchar_t *st2);
	int mStricmp(const wchar_t *st1, const char *st2);
	int mStrnicmp(const wchar_t *st1, const wchar_t *st2, size_t len);
	int mStrnicmp(const wchar_t *st1, const char *st2, size_t len);
	size_t strlen(const wchar_t *st);
	size_t strlen(const char *st);
	wchar_t *strcpy(wchar_t *des, const wchar_t *src);
	wchar_t *strcpy(wchar_t *des, const char *src);
	char    *strcpy(char *des, const char *src);
	wchar_t *strncpy(wchar_t *des, const wchar_t *src, size_t num);
	wchar_t *strncpy(wchar_t *des, const char *src, size_t num);
	char    *strncpy(char *des, const char *src, size_t num);
	wchar_t *strcat(wchar_t *des, const wchar_t *src);
	wchar_t *strcat(wchar_t *des, const char *src);
	char    *strcat(char *des, const char *src);
	wchar_t *strncat(wchar_t *des, const wchar_t *src, size_t num);
	wchar_t *strncat(wchar_t *des, const char *src, size_t num);
	char    *strncat(char *des, const char *src, size_t num);
	wchar_t *strchr(wchar_t *src, int c);
	char    *strchr(char *src, int c);
	wchar_t *strrchr(wchar_t *src, int c);
	char    *strrchr(char *src, int c);
	wchar_t *strpbrk(wchar_t *src, const wchar_t *cs);
	wchar_t *strpbrk(wchar_t *src, const char *cs);
	char    *strpbrk(char *src, const char *cs);
	int  atoi(const wchar_t *s);
	int  atoi(const char *s);
	long atol(const wchar_t *s);
	long atol(const char *s);

	char    *u_strcpy(char *des, const wchar_t *src, unsigned long MaxLen);
	wchar_t *u_strcpy(wchar_t *des, const char *src, unsigned long MaxLen);

	char isskip(const wchar_t *org, int pos, NewFontInfo *finfo, char MBC);
	char ismorfchar(const wchar_t *org, int pos, NewFontInfo *finfo, const char *morfsList, char MBC);
	char isCharInMorf(char c, wchar_t *morf);
	char atUFound(const wchar_t *w, int s, NewFontInfo *finfo, char MBC);
	char isRightChar(const wchar_t *org, long pos, register char chr, NewFontInfo *finfo, char MBC);
	char isUpperChar(wchar_t *org, int pos, NewFontInfo *finfo, char MBC);
	char isSqBracketItem(const wchar_t *s, int pos, NewFontInfo *finfo, char MBC);
	char isSqCodes(const wchar_t *word, wchar_t *tWord, NewFontInfo *finfo, char isForce);
	void remblanks(wchar_t *st);
	void remFrontAndBackBlanks(wchar_t *st);
	void shiftright(wchar_t *st, int num);
	void cleanUpCodes(wchar_t *code, NewFontInfo *finfo, char MBC);
	void extractString(wchar_t *out, const wchar_t *line, const char *type, wchar_t endC);
	int  isToneUnitMarker(wchar_t *word);
	int  IsCAUtteranceDel(wchar_t *st, int pos);
	int  IsCharUtteranceDel(wchar_t *st, int pos);
	int  IsUtteranceDel(const wchar_t *st, int pos);
	int  isPause(const wchar_t *st, int pos, int *beg, int *end);
	long lowercasestr(wchar_t *str, NewFontInfo *finfo, char MBC);
	long uppercasestr(wchar_t *str, NewFontInfo *finfo, char MBC);
	char fpatmat(const wchar_t *s, const wchar_t *pat);
	char fpatmat(const wchar_t *s, const char *pat);
	char fIpatmat(const wchar_t *s, const wchar_t *pat);
	char fIpatmat(const wchar_t *s, const char *pat);
	int  partcmp(const wchar_t *st1, const wchar_t *st2, char pat_match, char isCaseSenc);
	int  partcmp(const wchar_t *st1, const char *st2, char pat_match, char isCaseSenc);
	int  partwcmp(const wchar_t *st1, const wchar_t *st2);
	int  partwcmp(const wchar_t *st1, const char *st2);
	int  patmat(wchar_t *s, const wchar_t *pat);
	int  isPlusMinusWord(const wchar_t *ch, int pos);
	wchar_t *sp_cp(wchar_t *s1, wchar_t *s2);
	void sp_mod(wchar_t *s1, wchar_t *s2);
	char isUTF8(const wchar_t *str);
	char HandleCAChars(wchar_t *w, int *matchedType);

	char *strtok(char *, const char *);
	char *strstr(char *, const char *);
	size_t strspn(char *, const char *);
	void *memset(void * , int , size_t );
	void *memchr(const void * , int , size_t );
	int	  memcmp(const void * , const void * , size_t );
	void *memcpy (void * , const void * , size_t );
	void *memmove(void * , const void * , size_t );
#endif // UNX

	FNType  *str2FNType(FNType *des, long offset, const char *src);
	FNType  *strn2FNType(FNType *des, long offset, const char *src, long len);
	char    *FNType2str(char *des, long offset, const FNType *src);
	int		FNTypecmp(const FNType *st1, const char *st2, long len);
	int		FNTypeicmp(const FNType *st1, const char *st2, long len);


	int  isToneUnitMarker(char *word);
	int  IsCAUtteranceDel(char *st, int pos);
	int  IsCharUtteranceDel(char *st, int pos);
	int  IsUtteranceDel(const char *st, int pos);
	int  isPause(const char *st, int pos, int *beg, int *end);
	int  partcmp(const char *st1, const char *st2, char pat_match, char isCaseSenc);
	int  partwcmp(const char *st1, const char *st2);
	int  patmat(char *s, const char *pat);
	int  isPlusMinusWord(const char *ch, int pos);
	int  mStricmp(const char *st1, const char *st2);
	int  mStrnicmp(const char *st1, const char *st2, size_t len);
	long lowercasestr(char *str, NewFontInfo *finfo, char MBC);
	long uppercasestr(char *str, NewFontInfo *finfo, char MBC);
	char isskip(const char *org, int pos, NewFontInfo *finfo, char MBC);
	char ismorfchar(const char *org, int pos, NewFontInfo *finfo, const char *morfsList, char MBC);
	char isCharInMorf(char c, char *morf);
	char atUFound(const char *w, int s, NewFontInfo *finfo, char MBC);
	char isRightChar(const char *org, long pos, register char chr, NewFontInfo *finfo, char MBC);
	char isUpperChar(char *org, int pos, NewFontInfo *finfo, char MBC);
	char isSqBracketItem(const char *s, int pos, NewFontInfo *finfo, char MBC);
	char isSqCodes(const char *word, char *tWord, NewFontInfo *finfo, char isForce);
	char *sp_cp(char *s1, char *s2);
	char isUTF8(const char *str);
	char HandleCAChars(char *w, int *matchedType);
	char fpatmat(const char *s, const char *pat);
	char fIpatmat(const char *s, const char *pat);
	void sp_mod(char *s1, char *s2);
	void remblanks(char *st);
	void remFrontAndBackBlanks(char *st);
	void shiftright(char *st, int num);
	void cleanUpCodes(char *code, NewFontInfo *finfo, char MBC);
	void extractString(char *out, const char *line, const char *type, char endC);
};

extern cUStr uS;

#ifndef UNX
	#ifndef INSIDE_STRINGPARSER

		#define strcmp uS.strcmp
		#define strncmp uS.strncmp
		#define strlen uS.strlen
		#define strcpy uS.strcpy

		#define u_strcpy uS.u_strcpy
		#define strncpy uS.strncpy
		#define strcat uS.strcat
		#define strncat uS.strncat
		#define strchr uS.strchr
		#define strrchr uS.strrchr
		#define strpbrk uS.strpbrk
		#define strtok  uS.strtok
		#define strstr uS.strstr
		#define strspn uS.strspn
		#define memset uS.memset
		#define memcmp uS.memcmp
		#define memcpy uS.memcpy
		#define memmove uS.memmove
		#define atoi uS.atoi
		#define atol uS.atol

	#else

		#include <string.h> //lxs string

	#endif
#else // UNX
	#include <string.h> //lxs string
#endif // UNX
}

#endif /* STRINGPARSERDEF */
