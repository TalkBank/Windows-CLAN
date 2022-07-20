/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#ifndef COMMONDEF
#define COMMONDEF

#if defined(_MAC_CODE)
#ifdef _COCOA_APP
	#include <MacTypes.h>
#endif
	#include "0global.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32)
	#include "stdafx.h"
	#include "Clan2.h"
#endif

#include "wstring.h"

extern "C"
{

#ifdef UNX
	#define TRUE 1
	#define FALSE 0
	#define LF_FACESIZE 256
	#define DefChatMode 2
	#define TabSize 8
	#define FSSpec void
	#define SetFontName(x, y) strcpy(x, y);
	#define isFontEqual(x, y) (strcmp(x, y) == 0)
	typedef unsigned char Str255[256];

//	#define INSIDE_STRINGPARSER
#endif

#ifndef NEW
#if defined(_MAC_CODE)
	#define NEW(type) ((type *)malloc((size_t) sizeof(type)))
#endif
#if defined(_WIN32)
	#define NEW new
#endif
#endif /* NEW */

#ifndef EOS			/* End Of String marker			 */
	#define EOS 0 // '\0'
#endif
#ifndef TRUE
	#define TRUE  1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

typedef union creator_type {
	long out;
	char in[4];
} creator_type;

struct redirects {
	FILE *fp;
	FNType fn[FNSize];
	char all;
} ;

#include "fontconvert.h"
#include "stringparser.h"
#if defined(_MAC_CODE) || defined(_WIN32)
#include "my_unix.h"
#endif

#define UTF8	1
#define UTF16	2

#define SPECIALTEXTFILESTR "lXs Special Text file saves all fonts LxS"
#define SPECIALTEXTFILESTRLEN 41
#define ERRMESSAGELEN	2048
#define SPEAKERLEN		1024		 /* max len of the code part of the turn */
#define UTTLINELEN		18000L		 /* max len of the line part of the turn */
#define TEMPWLEN		30000L
#define UTF8HEADER		"@UTF8"
#define PIDHEADER		"@PID:"
#define CKEYWORDHEADER	"@Color words:"
#define WINDOWSINFO		"@Window:"
#define FONTHEADER		"@Font:"
#define FONTMARKER		"[%fnt: "
#define MEDIAHEADER		"@Media:"
#define SOUNDTIER		"%snd:"
#define PICTTIER		"%pic:"
#define TEXTTIER		"%txt:"
#define CMDIPIDHEADER	"CMDI_PID"
#ifdef _COCOA_APP
#define UTF8HEADERLEN	5
#define PIDHEADERLEN	5
#define CKEYWORDHEADERLEN 13
#define WINDOWSINFOLEN	8
#define FONTHEADERLEN	6
#define MEDIAHEADERLEN	7
#define SOUNDTIERLEN	5
#define PICTTIERLEN		5
#define TEXTTIERLEN		5
#define CMDIPIDHEADERLEN 8
#endif


#define REMOVEMOVIETAG	"%mov:"

#define NL_C  '\024'
#define SNL_C '\020'
#define HIDEN_C  '\025'
#define AttTYPE unsigned short

#define isSpace(c)	 ((c) == (unCH)' ' || (c) == (unCH)'\t')
#define isSpeaker(c) ((c) == (unCH)'@' || (c) == (unCH)'%' || (c) == (unCH)'*')
#define isMainSpeaker(c) ((c) == (unCH)'*')
#define NSCRIPT		0
#define JSCRIPT		1
#define CSCRIPT		2
#define KSCRIPT		3

#if defined(_WIN32)
	#define SetNewVol(name) my_chdir(name)
	#define WD_Not_Eq_OD (pathcmp(wd_dir, od_dir))
	#define isRefEQZero(ref) ((ref[0] != '\\') && ((ref[1] != ':') || (ref[2] != '\\')))
	#define CR_CHR '\n'
	#define PATHDELIMCHR '\\'		/* Path delimiter on IBM 		 */
	#define PATHDELIMSTR "\\"		/* Path delimiter on IBM 		 */
	#define chdir  my_chdir
	#define access my_access
	#define unlink my_unlink
	#define rename my_rename
	#define getcwd my_getcwd
	#define fopen my_fopen

	extern short winEncod;

	extern short my_FontToScript(char *fName, int charSet);
#elif defined(_MAC_CODE)
	#define SetNewVol(name) my_chdir(name)
	#define WD_Not_Eq_OD (pathcmp(wd_dir, od_dir))
	#define isRefEQZero(ref) (ref[0] != '/')
	#define CR_CHR '\r'
	#define PATHDELIMCHR '/'		/* Path delimiter on Mac 		 */
	#define PATHDELIMSTR "/"		/* Path delimiter on Mac 		 */
	#define chdir  my_chdir
	#define access my_access
	#define unlink my_unlink
	#define rename my_rename
	#define getcwd my_getcwd
	#define fopen my_fopen

	extern creator_type the_file_creator;

	extern int OpenAnyFile(const FNType *fn, int id, char winShift);
	extern short my_FontToScript(short fName, int charSet);
	extern void gettyp(const FNType *fn, long *type, long *creator);
	extern void settyp(const FNType *fn, long type, long creator, char isForce);
	extern void my_CFStringGetBytes(CFStringRef theString, char *buf, CFIndex maxBufLen);
#ifdef _COCOA_APP
	extern void do_warning_sheet(const char *str, NSWindow *window); // 2019-12-05
#endif
	extern CFStringRef my_CFStringCreateWithBytes(const char *bytes);
	extern OSStatus my_FSPathMakeRef(const char *path, FSRef *ref);
	extern OSStatus my_FSRefMakePath(const FSRef *ref, char *path, UInt32 maxPathSize);
#else
	#define SetNewVol(name) chdir(name)
	#define isRefEQZero(ref) (ref[0] != '/')
	#ifndef DEPDIR
		#define DEPDIR  "../lib/"
	#endif
	#define PATHDELIMCHR '/'		/* Path delimiter on Unix 		 */
	#define PATHDELIMSTR "/"		/* Path delimiter on Unix 		 */
#endif

extern char spareTier1[];
extern char spareTier2[];
extern char spareTier3[];
extern unCH templine[];
extern unCH templine1[];
extern unCH templine2[];
extern unCH templine3[];
extern unCH templine4[];
extern unCH templineW[];
extern unCH templineW1[];
extern char templineC[];
extern char templineC1[];
extern char templineC2[];
extern char templineC3[];
extern char templineC4[];
extern AttTYPE tempAtt[];
extern FNType DirPathName[];
extern FNType FileName1[];
extern FNType FileName2[];
extern FNType lib_dir[];
extern FNType mor_lib_dir[];
extern NewFontInfo dFnt, oFnt;
extern struct redirects *pipe_in;
extern struct redirects *pipe_out;
extern struct redirects redirect_in;
extern struct redirects redirect_out;	

extern int  LocateDir(const char *prompt, FNType *currentDir, char noDefault);
extern int  QueryDialog(const char *st, short id);
extern int  wpathcmp(const unCH *path1, const unCH *path2);
extern int  pathcmp(const FNType *path1, const FNType *path2);
extern int  my_isatty(FILE *fp);
extern int  my_feof(FILE *fp);
extern int  my_getc(FILE *fp);
//extern char *my_gets(char *st);
extern char SetTextAtt(char *st, unCH c, AttTYPE *att);
extern void addFilename2Path(FNType *path, const FNType *filename);
extern char extractPath(FNType *DirPathName, FNType *file_path);
extern void extractFileName(FNType *FileName, FNType *file_path);
extern void LocalInit(void);
extern void do_warning(const char *str, int delay);
extern void WriteCedPreference(void);
extern void WriteClanPreference(void);
extern void OutputToScreen(unCH *s);
extern void my_flush_chr(void);
extern void my_fprintf(FILE * file, const char * format, ...);
extern void my_printf(const char * format, ...);
extern void my_fputs(const char * format, FILE * file);
extern void my_puts(const char * format);
extern void my_putc(const char format, FILE * file);
extern void my_putchar(const char format);
extern void my_rewind(FILE *fp);

}
#endif /* COMMONDEF */
