/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#if !defined(UNX)
#include "ced.h"
#endif
#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include <stddef.h>
#include "c_curses.h"

#if !defined(UNX)
#define _main rtf2chat_main
#define call rtf2chat_call
#define getflag rtf2chat_getflag
#define init rtf2chat_init
#define usage rtf2chat_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern int  getc_cr_lc;
extern char OverWriteFile;
extern struct tier *defheadtier;

static char isPrintStruct;
static char isPostProcess;
static char lEncodeSet;
static short lEncode;
#ifdef _MAC_CODE
static TextEncodingVariant lVariant;
#endif

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
		lEncodeSet = 0;
		lEncode = 0;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		defheadtier = NULL;
		isPrintStruct = FALSE;
		isPostProcess = FALSE;
	} else {
		if (lEncodeSet == 0) {
			fprintf(stderr, "Please specify text encoding with +o option.\n");
			puts("For example: rtf2chat -opcl *.rtf");
			cutt_exit(0);
		}
	}
}

void usage() {
	printf("Usage: rtf2chat [a b oS re %s] filename(s)\n",mainflgs());
	puts("+a : pretty print RTF structure.");
	puts("+b : post process legal CHAT output file.");
	puts("+re: run program recursively on all sub-directories.");
	puts("+oS: Specify code page. Please type \"+o?\" for full listing of codes");
	puts("     macl  - Mac Latin (German, Spanish ...)");
	puts("     pcl   - PC  Latin (German, Spanish ...)");
	puts("\nExample: rtf2chat -opcl *.rtf");
	cutt_exit(0);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = RTF2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'a':
			isPrintStruct = TRUE;
			no_arg_option(f);
			break;
		case 'b':
			isPostProcess = TRUE;
			no_arg_option(f);
			break;
		case 'o':			
			lEncodeSet = UTF8;

#ifdef _MAC_CODE
			if (!uS.mStricmp(f, "macl"))
				lEncode = kTextEncodingMacRoman;
			else if (!uS.mStricmp(f, "macce"))
				lEncode = kTextEncodingMacCentralEurRoman;
			else if (!uS.mStricmp(f, "pcl"))
				lEncode = kTextEncodingWindowsLatin1;
			else if (!uS.mStricmp(f, "pcce"))
				lEncode = kTextEncodingWindowsLatin2;

			else if (!uS.mStricmp(f, "macar"))
				lEncode = kTextEncodingMacArabic;
			else if (!uS.mStricmp(f, "pcar"))
				lEncode = kTextEncodingDOSArabic;

			else if (!uS.mStricmp(f, "maccs"))
				lEncode = kTextEncodingMacChineseSimp;
			else if (!uS.mStricmp(f, "macct"))
				lEncode = kTextEncodingMacChineseTrad;
			else if (!uS.mStricmp(f, "pccs"))
				lEncode = kTextEncodingDOSChineseSimplif;
			else if (!uS.mStricmp(f, "pcct"))
				lEncode = kTextEncodingDOSChineseTrad;

			else if (!uS.mStricmp(f, "maccr"))
				lEncode = kTextEncodingMacCroatian;

			else if (!uS.mStricmp(f, "maccy"))
				lEncode = kTextEncodingMacCyrillic;
			else if (!uS.mStricmp(f, "pccy"))
				lEncode = kTextEncodingWindowsCyrillic;

			else if (!uS.mStricmp(f, "machb"))
				lEncode = kTextEncodingMacHebrew;
			else if (!uS.mStricmp(f, "pchb"))
				lEncode = kTextEncodingDOSHebrew;

			else if (!uS.mStricmp(f, "macjp"))
				lEncode = kTextEncodingMacJapanese;
			else if (!uS.mStricmp(f, "pcjp"))
				lEncode = kTextEncodingDOSJapanese;
			else if (!uS.mStricmp(f, "macj1"))
				lEncode = kTextEncodingJIS_X0201_76;
			else if (!uS.mStricmp(f, "macj2"))
				lEncode = kTextEncodingJIS_X0208_83;
			else if (!uS.mStricmp(f, "macj3"))
				lEncode = kTextEncodingJIS_X0208_90;
			else if (!uS.mStricmp(f, "macj4"))
				lEncode = kTextEncodingJIS_X0212_90;
			else if (!uS.mStricmp(f, "macj5"))
				lEncode = kTextEncodingJIS_C6226_78;
			else if (!uS.mStricmp(f, "macj6"))
				lEncode = kTextEncodingShiftJIS_X0213_00;
			else if (!uS.mStricmp(f, "macj7"))
				lEncode = kTextEncodingISO_2022_JP;
			else if (!uS.mStricmp(f, "macj8"))
				lEncode = kTextEncodingISO_2022_JP_1;
			else if (!uS.mStricmp(f, "macj9"))
				lEncode = kTextEncodingISO_2022_JP_2;
			else if (!uS.mStricmp(f, "macj10"))
				lEncode = kTextEncodingISO_2022_JP_3;
			else if (!uS.mStricmp(f, "macj11"))
				lEncode = kTextEncodingEUC_JP;
			else if (!uS.mStricmp(f, "macj12"))
				lEncode = kTextEncodingShiftJIS;

			else if (!uS.mStricmp(f, "krn"))
				lEncode = kTextEncodingMacKorean;
			else if (!uS.mStricmp(f, "pckr"))
				lEncode = kTextEncodingDOSKorean;
			else if (!uS.mStricmp(f, "pckrj"))
				lEncode = kTextEncodingWindowsKoreanJohab;

			else if (!uS.mStricmp(f, "macth"))
				lEncode = kTextEncodingMacThai;
			else if (!uS.mStricmp(f, "pcth"))
				lEncode = kTextEncodingDOSThai;

			else if (!uS.mStricmp(f, "pcturk"))
				lEncode = kTextEncodingWindowsLatin5; // kTextEncodingDOSTurkish

			else if (!uS.mStricmp(f, "macvt"))
				lEncode = kTextEncodingMacVietnamese;
			else if (!uS.mStricmp(f, "pcvt"))
				lEncode = kTextEncodingWindowsVietnamese;
			else {
				if (*f != '?')
					fprintf(stderr,"Unrecognized font option \"%s\". Please use:\n", f);
				displayOoption();
				cutt_exit(0);
			}
#endif
#ifdef _WIN32 
			if (!uS.mStricmp(f, "pcl"))
				lEncode = 1252;
			else if (!uS.mStricmp(f, "pcce"))
				lEncode = 1250;

			else if (!uS.mStricmp(f, "pcar"))
				lEncode = 1256;

			else if (!uS.mStricmp(f, "pccs"))
				lEncode = 936;
			else if (!uS.mStricmp(f, "pcct"))
				lEncode = 950;

			else if (!uS.mStricmp(f, "pccy"))
				lEncode = 1251;

			else if (!uS.mStricmp(f, "pchb"))
				lEncode = 1255;

			else if (!uS.mStricmp(f, "pcjp"))
				lEncode = 932;

			else if (!uS.mStricmp(f, "krn"))
				lEncode = 949;
			else if (!uS.mStricmp(f, "pckr"))
				lEncode = 949;
			else if (!uS.mStricmp(f, "pckrj"))
				lEncode = 1361;

			else if (!uS.mStricmp(f, "pcth"))
				lEncode = 874;

			else if (!uS.mStricmp(f, "pcturk"))
				lEncode = 1254; // 857

			else if (!uS.mStricmp(f, "pcvt"))
				lEncode = 1258;
			else {
				if (*f != '?')
					fprintf(stderr,"Unrecognized font option \"%s\". Please use:\n", f);
				displayOoption();
				cutt_exit(0);
			}
#endif
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static int rtf_getc_cr(FILE *fp) {
	register int c;

#if defined(_MAC_CODE) || defined(_WIN32)
#ifndef _COCOA_APP
	SysEventCheck(250L);
#endif
#endif
	if ((c=getc(fp)) == '\r') {
		if (getc_cr_lc == '\n')
			c = getc(fp);
		else
			return('\n');
	}
	getc_cr_lc = c;
	return(c);
}

// RTF variable declarations

#define ecOK 0                      // Everything's fine!
#define ecStackUnderflow    1       // Unmatched '}'
#define ecStackOverflow     2       // Too many '{' -- memory exhausted
#define ecUnmatchedBrace    3       // RTF ended during an open group.
#define ecInvalidHex        4       // invalid hex character found in data
#define ecBadTable          5       // RTF table (sym or prop) invalid
#define ecAssertion         6       // Assertion failure
#define ecEndOfFile         7       // End of file reached while reading RTF
#define ecOutOfMemory       8       // ran out of memory

typedef struct char_prop
{
    char fBold;
    char fUnderline;
    char fItalic;
} CHP;                  // CHaracter Properties

typedef enum {justL, justR, justC, justF } JUST;
typedef struct para_prop
{
    int xaLeft;                 // left indent in twips
    int xaRight;                // right indent in twips
    int xaFirst;                // first line indent in twips
    JUST just;                  // justification
} PAP;                  // PAragraph Properties

typedef enum {sbkNon, sbkCol, sbkEvn, sbkOdd, sbkPg} SBK;
typedef enum {pgDec, pgURom, pgLRom, pgULtr, pgLLtr} PGN;
typedef struct sect_prop
{
    int cCols;                  // number of columns
    SBK sbk;                    // section break type
    int xaPgn;                  // x position of page number in twips
    int yaPgn;                  // y position of page number in twips
    PGN pgnFormat;              // how the page number is formatted
} SEP;                  // SEction Properties

typedef struct doc_prop
{
    int xaPage;                 // page width in twips
    int yaPage;                 // page height in twips
    int xaLeft;                 // left margin in twips
    int yaTop;                  // top margin in twips
    int xaRight;                // right margin in twips
    int yaBottom;               // bottom margin in twips
    int pgnStart;               // starting page number in twips
    char fFacingp;              // facing pages enabled?
    char fLandscape;            // landscape or portrait??
} DOP;                  // DOcument Properties

typedef enum { rdsNorm, rdsSkip, rdsGetFontName, rdsGetSymbol } RDS; // Rtf Destination State
typedef enum { risNorm, risBin, risBinSkip, risHex, risHexSkip } RIS;          // Rtf Internal State

typedef struct save             // property save structure
{
    struct save *pNext;         // next save
    CHP chp;
    PAP pap;
    SEP sep;
    DOP dop;
    RDS rds;
    RIS ris;
    int fnt;
    int fntSize;
    int fntCharSet;
} SAVE;

// What types of properties are there?
typedef enum {ipropBold, ipropItalic, ipropUnderline, ipropLeftInd,
              ipropRightInd, ipropFirstInd, ipropCols, ipropPgnX,
              ipropPgnY, ipropXaPage, ipropYaPage, ipropXaLeft,
              ipropXaRight, ipropYaTop, ipropYaBottom, ipropPgnStart,
              ipropSbk, ipropPgnFormat, ipropFacingp, ipropLandscape,
              ipropJust, ipropPard, ipropPlain, ipropSectd,
              ipropMax } IPROP;

typedef enum {actnSpec, actnByte, actnWord} ACTN;
typedef enum {propChp, propPap, propSep, propDop} PROPTYPE;

typedef struct propmod
{
    ACTN actn;              // size of value
    PROPTYPE prop;          // structure containing value
    int  offset;            // offset of value from base of structure
} PROP;

typedef enum {ipfnBin, ipfnHex, ipfnSkipDest } IPFN;
typedef enum {idestPict, idestSkip, idestFontNum, idestFontSize, idestFontChange, 
				idestSymbol, idestFootNote } IDEST;
typedef enum {kwdChar, kwdUTFChar, kwdDest, kwdProp, kwdSpec} KWD;

typedef struct symbol
{
    const char *szKeyword;  // RTF keyword
    int  dflt;              // default value to use
    bool fPassDflt;         // true to use default value from this table
    KWD  kwd;               // base action to take
    int  idx;               // index into property table if kwd == kwdProp
                            // index into destination table if kwd == kwdDest
                            // character to print if kwd == kwdChar
} SYM;

// RTF parser tables

// Property descriptions
static PROP rgprop [ipropMax] = {
	{ actnByte,   propChp,    offsetof(CHP, fBold) } ,       // ipropBold
	{ actnByte,   propChp,    offsetof(CHP, fItalic) } ,     // ipropItalic
	{ actnByte,   propChp,    offsetof(CHP, fUnderline) } ,  // ipropUnderline
	{ actnWord,   propPap,    offsetof(PAP, xaLeft) } ,      // ipropLeftInd
	{ actnWord,   propPap,    offsetof(PAP, xaRight) } ,     // ipropRightInd
	{ actnWord,   propPap,    offsetof(PAP, xaFirst) } ,     // ipropFirstInd
	{ actnWord,   propSep,    offsetof(SEP, cCols) } ,       // ipropCols
	{ actnWord,   propSep,    offsetof(SEP, xaPgn) } ,       // ipropPgnX
	{ actnWord,   propSep,    offsetof(SEP, yaPgn) } ,       // ipropPgnY
	{ actnWord,   propDop,    offsetof(DOP, xaPage) } ,      // ipropXaPage
	{ actnWord,   propDop,    offsetof(DOP, yaPage) } ,      // ipropYaPage
	{ actnWord,   propDop,    offsetof(DOP, xaLeft) } ,      // ipropXaLeft
	{ actnWord,   propDop,    offsetof(DOP, xaRight) } ,     // ipropXaRight
	{ actnWord,   propDop,    offsetof(DOP, yaTop) } ,       // ipropYaTop
	{ actnWord,   propDop,    offsetof(DOP, yaBottom) } ,    // ipropYaBottom
	{ actnWord,   propDop,    offsetof(DOP, pgnStart) } ,    // ipropPgnStart
	{ actnByte,   propSep,    offsetof(SEP, sbk) } ,         // ipropSbk
	{ actnByte,   propSep,    offsetof(SEP, pgnFormat) } ,   // ipropPgnFormat
	{ actnByte,   propDop,    offsetof(DOP, fFacingp) } ,    // ipropFacingp
	{ actnByte,   propDop,    offsetof(DOP, fLandscape) } ,  // ipropLandscape
	{ actnByte,   propPap,    offsetof(PAP, just) } ,        // ipropJust
	{ actnSpec,   propPap,    0 } ,                          // ipropPard
	{ actnSpec,   propChp,    0 } ,                          // ipropPlain
	{ actnSpec,   propSep,    0 } ,                          // ipropSectd
};

// Keyword descriptions
static SYM rgsymRtf[] = {
//  szKeyword   dflt    fPassDflt   kwd         idx
	{ "b",        1,      false,     kwdProp,    ipropBold } ,
	{ "ul",       1,      false,     kwdProp,    ipropUnderline } ,
	{ "ulnone",   0,      false,     kwdProp,    ipropUnderline } ,
	{ "i",        1,      false,     kwdProp,    ipropItalic } ,
	{ "li",       0,      false,     kwdProp,    ipropLeftInd } ,
	{ "ri",       0,      false,     kwdProp,    ipropRightInd } ,
	{ "fi",       0,      false,     kwdProp,    ipropFirstInd } ,
	{ "cols",     1,      false,     kwdProp,    ipropCols } ,
	{ "sbknone",  sbkNon, true,      kwdProp,    ipropSbk } ,
	{ "sbkcol",   sbkCol, true,      kwdProp,    ipropSbk } ,
	{ "sbkeven",  sbkEvn, true,      kwdProp,    ipropSbk } ,
	{ "sbkodd",   sbkOdd, true,      kwdProp,    ipropSbk } ,
	{ "sbkpage",  sbkPg,  true,      kwdProp,    ipropSbk } ,
	{ "pgnx",     0,      false,     kwdProp,    ipropPgnX } ,
	{ "pgny",     0,      false,     kwdProp,    ipropPgnY } ,
	{ "pgndec",   pgDec,  true,      kwdProp,    ipropPgnFormat } ,
	{ "pgnucrm",  pgURom, true,      kwdProp,    ipropPgnFormat } ,
	{ "pgnlcrm",  pgLRom, true,      kwdProp,    ipropPgnFormat } ,
	{ "pgnucltr", pgULtr, true,      kwdProp,    ipropPgnFormat } ,
	{ "pgnlcltr", pgLLtr, true,      kwdProp,    ipropPgnFormat } ,
	{ "qc",       justC,  true,      kwdProp,    ipropJust } ,
	{ "ql",       justL,  true,      kwdProp,    ipropJust } ,
	{ "qr",       justR,  true,      kwdProp,    ipropJust } ,
	{ "qj",       justF,  true,      kwdProp,    ipropJust } ,
	{ "paperw",   12240,  false,     kwdProp,    ipropXaPage } ,
	{ "paperh",   15480,  false,     kwdProp,    ipropYaPage } ,
	{ "margl",    1800,   false,     kwdProp,    ipropXaLeft } ,
	{ "margr",    1800,   false,     kwdProp,    ipropXaRight } ,
	{ "margt",    1440,   false,     kwdProp,    ipropYaTop } ,
	{ "margb",    1440,   false,     kwdProp,    ipropYaBottom } ,
	{ "pgnstart", 1,      true,      kwdProp,    ipropPgnStart } ,
	{ "facingp",  1,      true,      kwdProp,    ipropFacingp } ,
	{ "landscape",1,      true,      kwdProp,    ipropLandscape } ,
#ifdef _WIN32 
	{ "rquote",   0,      false,     kwdChar,    0x92 } ,
#endif
#ifdef _MAC_CODE 
	{ "rquote",   0,      false,     kwdChar,    0xab } ,
#endif
	{ "lquote",   0,      false,     kwdChar,    '\'' } ,
	{ "par",      0,      false,     kwdChar,    0x0a } ,
	{ "\0x0a",    0,      false,     kwdChar,    0x0a } ,
	{ "\0x0d",    0,      false,     kwdChar,    0x0a } ,
	{ "tab",      0,      false,     kwdChar,    0x09 } ,
	{ "\t",       0,      false,     kwdChar,    0x09 } ,
	{ "ldblquote",0,      false,     kwdChar,    '"' } ,
	{ "rdblquote",0,      false,     kwdChar,    '"' } ,
	{ "u",        0,      false,     kwdUTFChar, '\0' } ,
	{ "bin",      0,      false,     kwdSpec,    ipfnBin } ,
	{ "*",        0,      false,     kwdSpec,    ipfnSkipDest } ,
	{ "'",        0,      false,     kwdSpec,    ipfnHex } ,
	{ "fldinst",  0,      false,     kwdDest,    idestSymbol } ,
	{ "f",        0,      false,     kwdDest,    idestFontNum } ,
	{ "fs",       0,      false,     kwdDest,    idestFontSize } ,
	{ "author",   0,      false,     kwdDest,    idestSkip } ,
	{ "buptim",   0,      false,     kwdDest,    idestSkip } ,
	{ "colortbl", 0,      false,     kwdDest,    idestSkip } ,
	{ "comment",  0,      false,     kwdDest,    idestSkip } ,
	{ "creatim",  0,      false,     kwdDest,    idestSkip } ,
	{ "doccomm",  0,      false,     kwdDest,    idestSkip } ,
	{ "fonttbl",  0,      false,     kwdDest,    idestFontChange } ,
	{ "footer",   0,      false,     kwdDest,    idestSkip } ,
	{ "footerf",  0,      false,     kwdDest,    idestSkip } ,
	{ "footerl",  0,      false,     kwdDest,    idestSkip } ,
	{ "footerr",  0,      false,     kwdDest,    idestSkip } ,
	{ "footnote", 0,      false,     kwdDest,    idestFootNote } ,
	{ "ftncn",    0,      false,     kwdDest,    idestSkip } ,
	{ "ftnsep",   0,      false,     kwdDest,    idestSkip } ,
	{ "ftnsepc",  0,      false,     kwdDest,    idestSkip } ,
	{ "header",   0,      false,     kwdDest,    idestSkip } ,
	{ "headerf",  0,      false,     kwdDest,    idestSkip } ,
	{ "headerl",  0,      false,     kwdDest,    idestSkip } ,
	{ "headerr",  0,      false,     kwdDest,    idestSkip } ,
	{ "info",     0,      false,     kwdDest,    idestSkip } ,
	{ "keywords", 0,      false,     kwdDest,    idestSkip } ,
	{ "operator", 0,      false,     kwdDest,    idestSkip } ,
	{ "pict",     0,      false,     kwdDest,    idestSkip } ,
	{ "printim",  0,      false,     kwdDest,    idestSkip } ,
	{ "private1", 0,      false,     kwdDest,    idestSkip } ,
	{ "revtim",   0,      false,     kwdDest,    idestSkip } ,
	{ "rxe",      0,      false,     kwdDest,    idestSkip } ,
	{ "stylesheet",0,     false,     kwdDest,    idestSkip } ,
	{ "subject",  0,      false,     kwdDest,    idestSkip } ,
	{ "tc",       0,      false,     kwdDest,    idestSkip } ,
	{ "title",    0,      false,     kwdDest,    idestSkip } ,
	{ "txe",      0,      false,     kwdDest,    idestSkip } ,
	{ "xe",       0,      false,     kwdDest,    idestSkip } ,
	{ "{",        0,      false,     kwdChar,    '{' } ,
	{ "}",        0,      false,     kwdChar,    '}' } ,
	{ "\\",       0,      false,     kwdChar,    '\\' } ,
	{ "\n",       0,      false,     kwdChar,    '\n' } ,
	{ "\r",       0,      false,     kwdChar,    '\n'}
};

static int  tGroup;
static bool fSkipDestIfUnk;
static long cbBin;
static long lParam;
static bool symFound;

static RDS rds;
static RIS ris;

static int fnt;
static int fntSize;
static int fntCharSet;
static int oldFontNum;
static int findx;
#define FONTTABLESIZE 1024
static char *fontTable[FONTTABLESIZE];
static CHP chp;
static CHP oldAtt;
static PAP pap;
static SEP sep;
static DOP dop;

static SAVE *psave;

static int isymMax = sizeof(rgsymRtf) / sizeof(SYM);

// RTF parser declarations

static int ecParseRtfKeyword(FILE *fp, bool *isSkipNextHex);
static int ecTranslateKeyword(char *szKeyword, int param, bool fParam, bool *isSkipNextHex);

//
// %%Function: checkAtt
//
// Check chr attribute values and send results to output.
//
static void checkAtt(char ch) {
	char fBold, fUnderline, fItalic;
	if (ch == '\n') {
		fBold = false;
		fUnderline = false;
		fItalic = false;
	} else {
		fBold = chp.fBold;
		fUnderline = chp.fUnderline;
		fItalic = chp.fItalic;
	}
	if (fUnderline != oldAtt.fUnderline) {
		if (fUnderline)
			fprintf(fpout, "%c%c", ATTMARKER, underline_start);
		else
			fprintf(fpout, "%c%c", ATTMARKER, underline_end);
	}
	if (fItalic != oldAtt.fItalic) {
		if (fItalic)
			fprintf(fpout, "%c%c", ATTMARKER, italic_start);
		else
			fprintf(fpout, "%c%c", ATTMARKER, italic_end);
	}
	if (fBold != oldAtt.fBold) {
		if (fBold)
			fprintf(fpout, "%c%c", ATTMARKER, bold_start);
		else
			fprintf(fpout, "%c%c", ATTMARKER, bold_end);
	}
	oldAtt.fBold = fBold;
	oldAtt.fUnderline = fUnderline;
	oldAtt.fItalic = fItalic;
}
/* 2009-10-13
//
// %%Function: printFontInfo
//
// Send font information to the output file.
//
static void printFontInfo(char *fontName) {
	strcpy(templineC, FONTHEADER);
	strcat(templineC, "\t");
#ifdef _MAC_CODE
	strcat(templineC, fontName);
#endif
#ifdef _WIN32
	strcat(templineC, "Win95:");
	strcat(templineC, fontName);
#endif
	strcat(templineC, ":");
	sprintf(templineC+strlen(templineC), "%d", fntSize);
#ifdef _WIN32
	strcat(templineC, ":");
	uS.sprintf(templineC+strlen(templineC), "%d", fntCharSet);
#endif
	strcat(templineC, "\n");
	fprintf(fpout, "%s", templineC);
}
*/
#ifdef _WIN32 
#include <mbstring.h>
static void AsciiToUnicodeToUTF8(char *src, char *line) {
	long UTF8Len;
	long total = strlen(src);
	long wchars=MultiByteToWideChar(lEncode,0,(const char*)src,total,NULL,0);

	MultiByteToWideChar(lEncode,0,(const char*)src,total,templineW,wchars);
	UnicodeToUTF8(templineW, wchars, (unsigned char *)line, (unsigned long *)&UTF8Len, UTTLINELEN);
	if (UTF8Len == 0 && wchars > 0) {
		line[0] = EOS;
		return;
	}
}
#endif

#ifdef _MAC_CODE
static void AsciiToUnicodeToUTF8(char *src, char *line) {
	OSStatus err;
	long len;
	TECObjectRef ec;
	TextEncoding utf8Encoding;
	TextEncoding MacRomanEncoding;
	unsigned long ail, aol;

	MacRomanEncoding = CreateTextEncoding( (long)lEncode, lVariant, kTextEncodingDefaultFormat );
	utf8Encoding = CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicodeUTF8Format );
	if ((err=TECCreateConverter(&ec, MacRomanEncoding, utf8Encoding)) != noErr) {
		line[0] = EOS;
		return;
	}

	len = strlen(src);
	if ((err=TECConvertText(ec, (ConstTextPtr)src, len, &ail, (TextPtr)line, UTTLINELEN, &aol)) != noErr) {
		line[0] = EOS;
		return;
	}
	err = TECDisposeConverter(ec);
	if (ail < len) {
		line[0] = EOS;
		return;
	}
	line[aol] = EOS;
}
#endif

//
// %%Function: ecPrintChar
//
// Send a character to the output file.
//

static int ecPrintChar(int ch, char isUTF) {
	int i;
	unCH bufW[2];
	char bufUTF[25], bufANSII[25];

	checkAtt(ch);
	if (oldFontNum != fnt && fnt >= 0 && fnt < FONTTABLESIZE) {
		if (fontTable[fnt] != NULL) {
//			printFontInfo(fontTable[fnt]);
			oldFontNum = fnt;
		}
	}
// unfortunately, we don't do a whole lot here as far as layout goes...
	if (isUTF) {
		bufW[0] = ch;
		bufW[1] = EOS;
		UnicodeToUTF8(bufW, strlen(bufW), (unsigned char *)bufUTF, NULL, 24);
		for (i=0; bufUTF[i] != EOS; i++)
			putc(bufUTF[i], fpout);
	} else if (ch >= 128) {
		if (lEncode != 0) {
			bufANSII[0] = ch;
			bufANSII[1] = EOS;
			AsciiToUnicodeToUTF8(bufANSII, bufUTF);
		} else {
			bufW[0] = ch;
			bufW[1] = EOS;
			UnicodeToUTF8(bufW, strlen(bufW), (unsigned char *)bufUTF, NULL, 24);
		}
		if (bufUTF[0] != EOS) {
			for (i=0; bufUTF[i] != EOS; i++)
				putc(bufUTF[i], fpout);
		} else
			fprintf(fpout, "(0x%x)", ch);
	} else
		putc(ch, fpout);
    return ecOK;
}

//
// %%Function: ecEndGroupAction
//
// The destination specified by rds is coming to a close.
// If there's any cleanup that needs to be done, do it now.
//

static int ecEndGroupAction(RDS rds) {
    return ecOK;
}

//
// %%Function: ecPushRtfState
//
// Save relevant info on a linked list of SAVE structures.
//

static int ecPushRtfState(void) {
    SAVE *psaveNew = (SAVE *)malloc(sizeof(SAVE));
    if (!psaveNew)
        return ecStackOverflow;

    psaveNew -> pNext = psave;
    psaveNew -> chp = chp;
    psaveNew -> pap = pap;
    psaveNew -> sep = sep;
    psaveNew -> dop = dop;
    psaveNew -> rds = rds;
    psaveNew -> ris = ris;
    psaveNew -> fnt = fnt;
    psaveNew -> fntSize = fntSize;
    psaveNew -> fntCharSet = fntCharSet;
    ris = risNorm;
    psave = psaveNew;
    tGroup++;
    return ecOK;
}

//
// %%Function: ecPopRtfState
//
// If we're ending a destination (that is, the destination is changing),
// call ecEndGroupAction.
// Always restore relevant info from the top of the SAVE list.
//

static int ecPopRtfState(void) {
    SAVE *psaveOld;
    int ec;

    if (!psave)
        return ecStackUnderflow;

    if (rds != psave->rds)
    {
        if ((ec = ecEndGroupAction(rds)) != ecOK)
            return ec;
    }
    chp = psave->chp;
    pap = psave->pap;
    sep = psave->sep;
    dop = psave->dop;
    rds = psave->rds;
    ris = psave->ris;
	fnt = psave->fnt;
	fntSize = psave->fntSize;
	fntCharSet = psave->fntCharSet;
    psaveOld = psave;
    psave = psave->pNext;
    tGroup--;
    free(psaveOld);
    return ecOK;
}

//
// %%Function: putFontIntoFontTable
//
// stores font name into font table under fnt.
//

static int putFontIntoFontTable(void) {
	if (findx > 0) {
		templineC1[findx] = EOS;
		if (fnt >= 0 && fnt < FONTTABLESIZE && fontTable[fnt] == NULL) {
			fontTable[fnt] = (char *)malloc(strlen(templineC1)+1);
			if (fontTable[fnt] == NULL) {
				fprintf(stderr, "Out of memory.\n");
				return ecOutOfMemory;
			}
			strcpy(fontTable[fnt], templineC1);
		}
		findx = 0;
	}
	return ecOK;
}

//
// %%Function: putSymbol
//
// converts SYMBOL to right char.
//

static int putSymbol(void) {
	int ch;

	if (findx > 0) {
		templineC1[findx] = EOS;
		ch = atoi(templineC1);
	
//		ch = 0;

		if (ch != 0)
			ecPrintChar(ch, FALSE);
		rds = rdsSkip;
		findx = 0;
		symFound = FALSE;
	}
	return ecOK;
}

//
// %%Function: ecParseChar
//
// Route the character to the appropriate destination stream.
//

static int ecParseChar(int ch) {
	if (ris == risBinSkip)
		return ecOK;
	else if (ris == risBin && --cbBin <= 0)
        ris = risNorm;
    switch (rds)
    {
    case rdsGetFontName:
	    if (ch == ';')
	     	putFontIntoFontTable();
    	else if (findx < UTTLINELEN-2)
	        templineC1[findx++] = ch;
        return ecOK;
    case rdsGetSymbol:
    	if (findx < UTTLINELEN-2 && isalpha(ch))
	        templineC1[findx++] = ch;
    	else if (!isdigit(ch)) {
			templineC1[findx] = EOS;
    		if (uS.mStricmp(templineC1, "SYMBOL") == 0) {
    			symFound = TRUE;
				findx = 0;
			} else
		     	putSymbol();
    	} else if (findx < UTTLINELEN-2 && isdigit(ch))
	        templineC1[findx++] = ch;
        return ecOK;
    case rdsSkip:
        // Toss this character.
        return ecOK;
    case rdsNorm:
        // Output a character. Properties are valid at this point.
        return ecPrintChar(ch, FALSE);
    default:
    // handle other destinations....
        return ecOK;
    }
}

//
// %%Function: ecRtfParse
//
// Step 1:
// Isolate RTF keywords and send them to ecParseRtfKeyword;
// Push and pop state at the start and end of RTF groups;
// Send text to ecParseChar for further processing.
//

static int ecRtfParse(FILE *fp) {
    int ch;
    int ec;
    int cNibble = 2;
    int b = 0;
	bool isSkipNextHex = FALSE;

    while ((ch = rtf_getc_cr(fp)) != EOF)
    {
		if (ch != '\\' && ch != '\n')
			isSkipNextHex = FALSE;
        if (tGroup < 0)
            return ecStackUnderflow;
        if (ris == risBin)                      // if we're parsing binary data, handle it directly
        {
            if ((ec = ecParseChar(ch)) != ecOK)
                return ec;
        }
        else
        {
            switch (ch)
            {
            case '{':
            	if (rds == rdsGetFontName)
		        	putFontIntoFontTable();
		        else if (rds == rdsGetSymbol)
		        	putSymbol();
                if ((ec = ecPushRtfState()) != ecOK)
                    return ec;
                break;
            case '}':
            	if (rds == rdsGetFontName)
		        	putFontIntoFontTable();
		        else if (rds == rdsGetSymbol)
		        	putSymbol();
                if ((ec = ecPopRtfState()) != ecOK)
                    return ec;
                break;
            case '\\':
            	if (rds == rdsGetFontName)
		        	putFontIntoFontTable();
		        else if (rds == rdsGetSymbol)
		        	putSymbol();
                if ((ec = ecParseRtfKeyword(fp, &isSkipNextHex)) != ecOK)
                    return ec;
                break;
            case 0x0d:
            case 0x0a:          // cr and lf are noise characters...
                break;
            default:
                if (ris == risNorm) {
                    if ((ec = ecParseChar(ch)) != ecOK)
                        return ec;
                } else if (ris == risHex || ris == risHexSkip) {               // parsing hex data
                    b = b * 16;
                    if (isdigit(ch))
                        b += (char) ch - '0';
                    else {
                        if (islower((unsigned char)ch)) {
                            if (ch < 'a' || ch > 'f')
                                return ecInvalidHex;
                            b = b + ((int)((char)ch - 'a') + 10);
                        } else {
                            if (ch < 'A' || ch > 'F')
                                return ecInvalidHex;
                            b = b + ((int)((char)ch - 'A') + 10);
                        }
                    }
                    cNibble--;
                    if (!cNibble) {
						if (ris == risHex) {
							if ((ec = ecParseChar(b)) != ecOK)
								return ec;
						}
                        cNibble = 2;
                        b = 0;
						ris = risNorm;
                    }
                } else
					return ecAssertion;
                break;
            }       // switch
        }           // else (ris != risBin)
    }               // while
    if (tGroup < 0)
        return ecStackUnderflow;
    if (tGroup > 0)
        return ecUnmatchedBrace;
    return ecOK;
}

//
// %%Function: ecParseRtfKeyword
//
// Step 2:
// get a control word (and its associated value) and
// call ecTranslateKeyword to dispatch the control.
//

static int
ecParseRtfKeyword(FILE *fp, bool *isSkipNextHex)
{
    int ch;
    bool fParam = false;
    bool fNeg = false;
    int param = 0;
    char *pch;
    char szKeyword[30];
    char szParameter[20];

    szKeyword[0] = '\0';
    szParameter[0] = '\0';
    if ((ch = rtf_getc_cr(fp)) == EOF)
        return ecEndOfFile;
    if (!isalpha(ch))           // a control symbol; no delimiter.
    {
        szKeyword[0] = (char) ch;
        szKeyword[1] = '\0';
        return ecTranslateKeyword(szKeyword, 0, fParam, isSkipNextHex);
    }
    for (pch = szKeyword; isalpha(ch); ch = rtf_getc_cr(fp))
        *pch++ = (char) ch;
    *pch = '\0';
    if (ch == '-')
    {
        fNeg  = true;
        if ((ch = rtf_getc_cr(fp)) == EOF)
            return ecEndOfFile;
    }
    if (isdigit(ch))
    {
        fParam = true;         // a digit after the control means we have a parameter
        for (pch = szParameter; isdigit(ch); ch = rtf_getc_cr(fp))
            *pch++ = (char) ch;
        *pch = '\0';
        param = atoi(szParameter);
        if (fNeg)
            param = -param;
        lParam = atol(szParameter);
        if (fNeg)
            param = -param;
    }
    if (ch != ' ')
        ungetc(ch, fp);
    return ecTranslateKeyword(szKeyword, param, fParam, isSkipNextHex);
}

//
// %%Function: ecParseSpecialProperty
//
// Set a property that requires code to evaluate.
//

static int
ecParseSpecialProperty(IPROP iprop, int val)
{
    switch (iprop)
    {
    case ipropPard:
        memset(&pap, 0, sizeof(pap));
        return ecOK;
    case ipropPlain:
        memset(&chp, 0, sizeof(chp));
        return ecOK;
    case ipropSectd:
        memset(&sep, 0, sizeof(sep));
        return ecOK;
    default:
        return ecBadTable;
    }
    return ecBadTable;
}

//
// %%Function: ecApplyPropChange
//
// Set the property identified by _iprop_ to the value _val_.
//
//

static int
ecApplyPropChange(IPROP iprop, int val)
{
    char *pb;

    if (rds == rdsSkip)                 // If we're skipping text,
        return ecOK;                    // don't do anything.

    switch (rgprop[iprop].prop)
    {
    case propDop:
        pb = (char *)&dop;
        break;
    case propSep:
        pb = (char *)&sep;
        break;
    case propPap:
        pb = (char *)&pap;
        break;
    case propChp:
        pb = (char *)&chp;
        break;
    default:
        if (rgprop[iprop].actn != actnSpec)
            return ecBadTable;
        break;
    }
    switch (rgprop[iprop].actn)
    {
    case actnByte:
        pb[rgprop[iprop].offset] = (unsigned char) val;
        break;
    case actnWord:
        (*(int *) (pb+rgprop[iprop].offset)) = val;
        break;
    case actnSpec:
        return ecParseSpecialProperty(iprop, val);
        break;
    default:
        return ecBadTable;
    }
    return ecOK;
}

//
// %%Function: ecParseSpecialKeyword
//
// Evaluate an RTF control that needs special processing.
//

static int
ecParseSpecialKeyword(IPFN ipfn, bool *isSkipNextHex)
{
    if (rds == rdsSkip && ipfn != ipfnBin)  // if we're skipping, and it's not
        return ecOK;                        // the \bin keyword, ignore it.
    switch (ipfn)
    {
    case ipfnBin:
		if (*isSkipNextHex)
			ris = risBinSkip;
		else
			ris = risBin;
        cbBin = lParam;
        break;
    case ipfnSkipDest:
        fSkipDestIfUnk = true;
        break;
    case ipfnHex:
		if (*isSkipNextHex)
			ris = risHexSkip;
		else
			ris = risHex;
		break;
    default:
        return ecBadTable;
    }
    return ecOK;
}

//
// %%Function: ecChangeDest
//
// Change to the destination specified by idest.
// There's usually more to do here than this...
//

static int
ecChangeDest(IDEST idest, int param)
{
//    if (rds == rdsSkip)             // if we're skipping text,
//        return ecOK;                // don't do anything

    switch (idest)
    {
    case idestFontNum:
        fnt = param;
        break;
    case idestFontSize:
        fntSize = param / 2;
        if (fntSize > 2)
        	fntSize -= 2;
        break;
    case idestFontChange:
    	rds = rdsGetFontName;
        findx = 0;
        break;
    case idestSymbol:
    	rds = rdsGetSymbol;
        findx = 0;
        symFound = FALSE;
        break;
    case idestFootNote:
         rds = rdsSkip;
        break;
   default:
        rds = rdsSkip;              // when in doubt, skip it...
        break;
    }
    return ecOK;
}

//
// %%Function: ecTranslateKeyword.
//
// Step 3.
// Search rgsymRtf for szKeyword and evaluate it appropriately.
//
// Inputs:
// szKeyword:   The RTF control to evaluate.
// param:       The parameter of the RTF control.
// fParam:      true if the control had a parameter; (that is, if param is valid)
//              false if it did not.
//

static int ecTranslateKeyword(char *szKeyword, int param, bool fParam, bool *isSkipNextHex) {
    int isym;

    // search for szKeyword in rgsymRtf

    for (isym = 0; isym < isymMax; isym++)
        if (strcmp(szKeyword, rgsymRtf[isym].szKeyword) == 0)
            break;
    if (isym == isymMax)            // control word not found
    {
        if (fSkipDestIfUnk)         // if this is a new destination
            rds = rdsSkip;          // skip the destination
                                    // else just discard it
        fSkipDestIfUnk = false;
        return ecOK;
    }

    // found it!  use kwd and idx to determine what to do with it.

    fSkipDestIfUnk = false;
    switch (rgsymRtf[isym].kwd)
    {
    case kwdProp:
        if (rgsymRtf[isym].fPassDflt || !fParam)
            param = rgsymRtf[isym].dflt;
        return ecApplyPropChange((IPROP)rgsymRtf[isym].idx, param);
    case kwdChar:
        return ecParseChar(rgsymRtf[isym].idx);
    case kwdUTFChar:
		if (rds == rdsNorm) {
			*isSkipNextHex = TRUE;
			return ecPrintChar(param, TRUE);
		} else
			return ecParseChar(param);
    case kwdDest:
        return ecChangeDest((IDEST)rgsymRtf[isym].idx, param);
    case kwdSpec:
        return ecParseSpecialKeyword((IPFN)rgsymRtf[isym].idx, isSkipNextHex);
    default:
        return ecBadTable;
    }
    return ecBadTable;
}

static void printTabs(int tabNum) {
	for (; tabNum > 0; tabNum--)
		putc(' ', fpout);
}

#define TABOFFSET 2

static void process(int tabNum) {
	char c;

    c = (char)rtf_getc_cr(fpin);
	while (!feof(fpin)) {
		if (c == '{') {
			putc('\n', fpout);
			printTabs(tabNum);
			putc(c, fpout);
			process(tabNum+TABOFFSET);
			c = (char)rtf_getc_cr(fpin);
			while (c == '\n')
				c = (char)rtf_getc_cr(fpin);
			if (c != '{' && c != '}') {
				putc('\n', fpout);
				printTabs(tabNum);
			}
		} else if (c == '}') {
			putc('\n', fpout);
			printTabs(tabNum-TABOFFSET);
			putc(c, fpout);
			break;
		} else if (c == '\n') {
			putc(' ', fpout);
			c = (char)rtf_getc_cr(fpin);
		} else {
			putc(c, fpout);
			c = (char)rtf_getc_cr(fpin);
		}
	}
}

static void postProcess(void) {
	long i;
	FNType tmp_name[FILENAME_MAX], *tf;
	FILE *tfpin;

	i = 0L;
	do {
		uS.sprintf(tmp_name, "T%ld", i);
		i++;
	} while (!access(tmp_name,0) && i < 32000L) ;
	if (i >= 32000L) {
		return;
	}
	fclose(fpout);
	fpout = NULL;
	if (rename(newfname,tmp_name)) {
		return;
	}
	tfpin = fpin;
	if ((fpin=fopen(tmp_name, "r")) == NULL) {
		fpin = tfpin;
		return;
	}
	if ((fpout=fopen(newfname, "w")) == NULL) {
		fclose(fpin);
		fpin = tfpin;
		return;
	}
#ifdef _MAC_CODE
	settyp(newfname, 'TEXT', the_file_creator.out, FALSE);
#endif
	fprintf(stderr, "Post processing....\n");
	tf = oldfname;
	oldfname = tmp_name;
	chatmode = 3;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,TRUE);
	}
	fclose(fpin);
	fpin = tfpin;
	unlink(tmp_name);
	oldfname = tf;
	fprintf(stderr, "Post processing done.\n");
}

void call() {	
    int ec;

	if (isPrintStruct) {
		process(0);
	} else {
		fprintf(fpout, "%s\n", UTF8HEADER);
		for (ec=0; ec < FONTTABLESIZE; ec++)
			fontTable[ec] = NULL;
		fnt = -1;
		fntSize = 12;
		fntCharSet = 1;
		oldFontNum = -1;
		memset(&oldAtt, 0, sizeof(oldAtt));
	    if ((ec = ecRtfParse(fpin)) != ecOK)
	        fprintf(stderr, "error %d parsing rtf\n", ec);
	    else if (isPostProcess && !stout)
	    	postProcess();
		for (ec=0; ec < FONTTABLESIZE; ec++) {
			if (fontTable[ec] != NULL)
				free(fontTable[ec]);
		}
	}
}
