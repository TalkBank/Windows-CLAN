extern "C"
{
extern short *ChatError(int num, long *ln);
extern int   checkChat(short *chatS, char *fname, char UEncoding);
extern char  *transformChatToXML(short *chatS, char *fname, char UEncoding);

// for internal use or those who REALLY know what they are doing ONLY!
extern char *ChatToXML(short *chatS,
				char isCheck,
				char *oFname,
				char *lang,
				char UEncod,
				char *isWarn,
				FILE *warnFP);

}

// defs for UEncoding variable
#define UDEFAULT 0
#define UPC  1
#define UMAC 2
#define UNOT 3

#if defined(_MAC_CODE)
	#ifndef _MAX_FNAME
		#define _MAX_FNAME		256
	#endif
#endif

class cChatToXMLStr
{
public:
	int    strcmp(const short *, const short *);
	int    strncmp(const short *, const short *, size_t);
	size_t strlen(const short *);
	short *strcpy(short *, const short *);
	short *strncpy(short *, const short *, size_t);
	short *strcat(short *, const short *);
	short *strchr(short *, int);
	short *strrchr(short *, int);
	short *strpbrk(short *, const short *);
	int   atoi(const short *);
	long  atol(const short *);
	short *itoa(int num, short *st, int size);
	char  partcmp(short *st1, const short *st2, char pat_match);
	char  partwcmp(short *st1, short *st2);
	char  patmat(short *s, const short *pat);
	int   monthToNum(short *s);

	int    strcmp(const short *, const char *);
	int    strncmp(const short *, const char *, size_t);
	size_t strlen(const char *);
	short *strcpy(short *, const char *);
	short *strncpy(short *, const char *, size_t);
	short *strcat(short *, const char *);
	short *strpbrk(short *, const char *);
	char  ismorfchar(short *org, int pos, short Script, char *morfsList, char MBC);
	int   sprintf(short *, const char *, ...);
	char  partcmp(short *st1, const char *st2, char pat_match);
	char  partwcmp(short *st1, char *st2);
	char  patmat(short *s, const char *pat);

	short ismultibyte(short *org, short pos, short encod);
	short ismultibyte(char *org, short pos, short encod);
	char  isRightChar(short *org, long pos, const char chr, short Script, char MBC);
	char  isskip(short *org, int pos, short Script, char MBC);
	char  isSqBracketItem(short *s, int pos, short Script, char MBC);
	char  isSqCodes(short *word, short *tWord, short cScript, char isForce);
	char  isToneUnitMarker(short *word);
	void lowercasestr(short *, short, char);
	void uppercasestr(short *, short, char);
	void remFblanks(short *st);
	void remblanks(short *st);
	void cleanUpCodes(short *code, short Script, char MBC);
};
