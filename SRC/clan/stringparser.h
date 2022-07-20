/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
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
#define PUNCT_ID_SET ","


extern const char *punctuation;/* contains main text line punctuation set	  */
	
class cUStr
{
public:

#ifndef UNX
	int sprintf(unCH *st, const unCH *format, ...);
	int sprintf(char *st, const char *format, ...);
	int strcmp(const unCH *st1, const unCH *st2);
	int strcmp(const unCH *st1, const char *st2);
	int strncmp(const unCH *st1, const unCH *st2, size_t len);
	int strncmp(const unCH *st1, const char *st2, size_t len);
	int mStricmp(const unCH *st1, const unCH *st2);
	int mStricmp(const unCH *st1, const char *st2);
	int mStrnicmp(const unCH *st1, const unCH *st2, size_t len);
	int mStrnicmp(const unCH *st1, const char *st2, size_t len);
	size_t strlen(const unCH *st);
	unCH *strcpy(unCH *des, const unCH *src);
	unCH *strcpy(unCH *des, const char *src);
	unCH *strncpy(unCH *des, const unCH *src, size_t num);
	unCH *strncpy(unCH *des, const char *src, size_t num);
	unCH *strcat(unCH *des, const unCH *src);
	unCH *strcat(unCH *des, const char *src);
	unCH *strncat(unCH *des, const unCH *src, size_t num);
	unCH *strncat(unCH *des, const char *src, size_t num);
	unCH *strchr(unCH *src, int c);
	unCH *strrchr(unCH *src, int c);
	unCH *strpbrk(unCH *src, const unCH *cs);
	unCH *strpbrk(unCH *src, const char *cs);
	int  atoi(const unCH *s);
	long atol(const unCH *s);

	char *u_strcpy(char *des, const unCH *src, unsigned long MaxLen);
	unCH *u_strcpy(unCH *des, const char *src, unsigned long MaxLen);

	char isskip(const unCH *org, int pos, NewFontInfo *finfo, char MBC);
	char ismorfchar(const unCH *org, int pos, NewFontInfo *finfo, const char *morfsList, char MBC);
	char isCharInMorf(char c, unCH *morf);
	char atUFound(const unCH *w, int s, NewFontInfo *finfo, char MBC);
	char isRightChar(const unCH *org, long pos, register char chr, NewFontInfo *finfo, char MBC);
	char isUpperChar(unCH *org, int pos, NewFontInfo *finfo, char MBC);
	char isSqBracketItem(const unCH *s, int pos, NewFontInfo *finfo, char MBC);
	char isSqCodes(const unCH *word, unCH *tWord, NewFontInfo *finfo, char isForce);
	void remblanks(unCH *st);
	void remFrontAndBackBlanks(unCH *st);
	void shiftright(unCH *st, int num);
	void cleanUpCodes(unCH *code, NewFontInfo *finfo, char MBC);
	void extractString(unCH *out, const unCH *line, const char *type, unCH endC);
	int  isToneUnitMarker(unCH *word);
	int  IsCAUtteranceDel(unCH *st, int pos);
	int  IsCharUtteranceDel(unCH *st, int pos);
	int  IsUtteranceDel(const unCH *st, int pos);
	int  isPause(const unCH *st, int pos, int *beg, int *end);
	long lowercasestr(unCH *str, NewFontInfo *finfo, char MBC);
	long uppercasestr(unCH *str, NewFontInfo *finfo, char MBC);
	char fpatmat(const unCH *s, const unCH *pat);
	char fpatmat(const unCH *s, const char *pat);
	char fIpatmat(const unCH *s, const unCH *pat);
	char fIpatmat(const unCH *s, const char *pat);
	int  partcmp(const unCH *st1, const unCH *st2, char pat_match, char isCaseSenc);
	int  partcmp(const unCH *st1, const char *st2, char pat_match, char isCaseSenc);
	int  partwcmp(const unCH *st1, const unCH *st2);
	int  partwcmp(const unCH *st1, const char *st2);
	int  patmat(unCH *s, const unCH *pat);
	int  isPlusMinusWord(const unCH *ch, int pos);
	int  HandleCAChars(unCH *w, int *matchedType);
	unCH *sp_cp(unCH *s1, unCH *s2);
	void sp_mod(unCH *s1, unCH *s2);
	char isUTF8(const unCH *str);
	char isInvisibleHeader(const unCH *str);

	char isConsonant(const unCH *st);
	char isVowel(const unCH *st);

#endif // UNX

	int strcmp(const char *st1, const char *st2);
	int strncmp(const char *st1, const char *st2, size_t len);
	char *strchr(const char *src, int c);
	char *strrchr(const char *src, int c);
	char *strpbrk(char *src, const char *cs);
	int  atoi(const char *s);
	long atol(const char *s);

	size_t strlen(const char *st);
	char *strcpy(char *des, const char *src);
	char *strncpy(char *des, const char *src, size_t num);
	char *strcat(char *des, const char *src);
	char *strncat(char *des, const char *src, size_t num);

	char *strtok(char *, const char *);
	char *strstr(char *, const char *);
	size_t strspn(char *, const char *);
	void *memset(void * , int , size_t );
	void *memchr(const void * , int , size_t );
	int	 memcmp(const void * , const void * , size_t );
	void *memcpy (void * , const void * , size_t );
	void *memmove(void * , const void * , size_t );

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
	int  HandleCAChars(char *w, int *matchedType);
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
	char isInvisibleHeader(const char *str);
	char fpatmat(const char *s, const char *pat);
	char fIpatmat(const char *s, const char *pat);
	void sp_mod(char *s1, char *s2);
	void remblanks(char *st);
	void remFrontAndBackBlanks(char *st);
	void shiftright(char *st, int num);
	void cleanUpCodes(char *code, NewFontInfo *finfo, char MBC);
	void extractString(char *out, const char *line, const char *type, char endC);

	char isConsonant(const char *st);
	char isVowel(const char *st);
};

extern void removeExtraSpace(char *st);
extern void removeAllSpaces(char *st);

extern cUStr uS;

#ifndef UNX
	#ifndef INSIDE_STRINGPARSER

		#define strlen uS.strlen
		#define strcpy uS.strcpy
		#define strncpy uS.strncpy
		#define strcat uS.strcat
		#define strncat uS.strncat

		#define strcmp uS.strcmp
		#define strncmp uS.strncmp

		#define u_strcpy uS.u_strcpy
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
//		#define atoi uS.atoi
//		#define atol uS.atol
//	#else

//		#include <string.h> //lxs string

	#endif
#else // UNX
	#ifndef INSIDE_STRINGPARSER
		#define strlen uS.strlen
		#define strcpy uS.strcpy
		#define strncpy uS.strncpy
		#define strcat uS.strcat
		#define strncat uS.strncat

		#define strcmp uS.strcmp
		#define strncmp uS.strncmp

		#define u_strcpy uS.u_strcpy
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
	#endif
//	#include <string.h> //lxs string
#endif // UNX
}

#endif /* STRINGPARSERDEF */
