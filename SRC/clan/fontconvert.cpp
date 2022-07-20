/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "common.h"
#include <ctype.h>
#include "c_clan.h"
#ifdef _WIN32
	#include "w95_QTReplace.h"
#endif

#ifdef UNX
#include "cu.h"
#endif
#include "fontconvert.h"
#include "stringparser.h"


extern "C" {
	char defUniFontName[256];
	long defUniFontSize;
	long stickyFontSize;
	short ArialUnicodeFOND;
	short SecondDefUniFOND;
}

#ifdef UNX
    #define kTextEncodingMacThai 21
    #define kTextEncodingWindowsLatin5 0x0504
#endif

/*	_WIN32
ANSI Code-Page Identifiers
874 Thai 
932 Japan 
936 Chinese (PRC, Singapore) 
949 Korean 
950 Chinese (Taiwan Region; Hong Kong SAR, PRC)  
1200 Unicode (BMP of ISO 10646) 
1250 Windows 3.1 Eastern European  
1251 Windows 3.1 Cyrillic 
1252 Windows 3.1 Latin 1 (US, Western Europe) 
1253 Windows 3.1 Greek 
1254 Windows 3.1 Turkish 
1255 Hebrew 
1256 Arabic 
1257 Baltic 

OEM Code-Page Identifiers
437 MS-DOS United States 
708 Arabic (ASMO 708) 
709 Arabic (ASMO 449+, BCON V4) 
710 Arabic (Transparent Arabic) 
720 Arabic (Transparent ASMO) 
737 Greek (formerly 437G) 
775 Baltic 
850 MS-DOS Multilingual (Latin I) 
852 MS-DOS Slavic (Latin II) 
855 IBM Cyrillic (primarily Russian) 
857 IBM Turkish 
860 MS-DOS Portuguese 
861 MS-DOS Icelandic 
862 Hebrew 
863 MS-DOS Canadian-French 
864 Arabic 
865 MS-DOS Nordic 
866 MS-DOS Russian (former USSR) 
869 IBM Modern Greek 
874 Thai 
932 Japan 
936 Chinese (PRC, Singapore) 
949 Korean 
950 Chinese (Taiwan Region; Hong Kong SAR, PRC)  
1361 Korean (Johab) 


Code-Page Identifiers
037 EBCDIC 
437 MS-DOS United States 
500 EBCDIC "500V1" 
708 Arabic (ASMO 708) 
709 Arabic (ASMO 449+, BCON V4) 
710 Arabic (Transparent Arabic) 
720 Arabic (Transparent ASMO) 
737 Greek (formerly 437G) 
775 Baltic 
850 MS-DOS Multilingual (Latin I) 
852 MS-DOS Slavic (Latin II) 
855 IBM Cyrillic (primarily Russian) 
857 IBM Turkish 
860 MS-DOS Portuguese 
861 MS-DOS Icelandic 
862 Hebrew 
863 MS-DOS Canadian-French 
864 Arabic 
865 MS-DOS Nordic 
866 MS-DOS Russian 
869 IBM Modern Greek 
874 Thai 
875 EBCDIC 
932 Japan 
936 Chinese (PRC, Singapore) 
949 Korean 
950 Chinese (Taiwan Region; Hong Kong SAR, PRC)  
1026 EBCDIC 
1200 Unicode (BMP of ISO 10646) 
1250 Windows 3.1 Eastern European  
1251 Windows 3.1 Cyrillic 
1252 Windows 3.1 US (ANSI) 
1253 Windows 3.1 Greek 
1254 Windows 3.1 Turkish 
1255 Hebrew 
1256 Arabic 
1257 Baltic 
1361 Korean (Johab) 
10000 Macintosh Roman 
10001 Macintosh Japanese 
10006 Macintosh Greek I 
10007 Macintosh Cyrillic 
10029 Macintosh Latin 2 
10079 Macintosh Icelandic 
10081 Macintosh Turkish 

// MAC
DOSLatinUS       = code page 437
DOSGreek         = code page 737 (formerly code page 437G)
DOSBalticRim     = code page 775
DOSLatin1        = code page 850, "Multilingual"
DOSGreek1        = code page 851
DOSLatin2        = code page 852, Slavic
DOSCyrillic      = code page 855, IBM Cyrillic
DOSTurkish       = code page 857, IBM Turkish
DOSPortuguese    = code page 860
DOSIcelandic     = code page 861
DOSHebrew        = code page 862
DOSCanadianFrench = code page 863
DOSArabic        = code page 864
DOSNordic        = code page 865
DOSRussian       = code page 866
DOSGreek2        = code page 869, IBM Modern Greek
DOSThai          = code page 874, also for Windows
DOSJapanese      = code page 932, also for Windows; Shift-JIS with additions
DOSChineseSimplif = code page 936, also for Windows; was EUC-CN, now GBK (EUC-CN extended)
DOSKorean        = code page 949, also for Windows; Unified Hangul Code (EUC-KR extended)
DOSChineseTrad   = code page 950, also for Windows; Big-5
WindowsLatin1    = code page 1252
WindowsANSI      = code page 1252 (alternate name)
WindowsLatin2    = code page 1250, Central Europe
WindowsCyrillic  = code page 1251, Slavic Cyrillic
WindowsGreek     = code page 1253
WindowsLatin5    = code page 1254, Turkish
WindowsHebrew    = code page 1255
WindowsArabic    = code page 1256
WindowsBalticRim = code page 1257
WindowsVietnamese = code page 1258
WindowsKoreanJohab = code page 1361, for Windows NT


*/
/************************************************************/
/*	 Find and set the current font of the tier			*/
short GetEncode(const char  *fontPref, const char *fontName, short fontType, int CharSet, char isShowErr) {
#if defined(_MAC_CODE)
	if (CharSet == 238)
		return(kTextEncodingWindowsLatin2);
	if (CharSet == 222)
		return(kTextEncodingDOSThai);
	if (CharSet == 204)
		return(kTextEncodingWindowsCyrillic);
	if (CharSet == 163)
		return(kTextEncodingWindowsVietnamese);
	if (CharSet == 162)
		return(kTextEncodingWindowsLatin5);
	if (CharSet == 136)
		return(kTextEncodingDOSChineseTrad);
	if (CharSet == 134)
		return(kTextEncodingDOSChineseSimplif);
	if (CharSet == 129)
		return(kTextEncodingDOSKorean);
	if (CharSet == 128)
		return(kTextEncodingDOSJapanese);

	if (fontType == WIN95cour)
		return(kTextEncodingWindowsLatin1);
	if (fontType == WIN95cain)
		return(kTextEncodingWindowsLatin1);
	if (fontType == WIN95courCE)
		return(kTextEncodingWindowsLatin2);
	if (fontType == WIN95Latin5)
		return(kTextEncodingWindowsLatin5);
	if (fontType == WINchn)
		return(kTextEncodingDOSChineseTrad);
	if (fontType == WINchn_GB)
		return(kTextEncodingDOSChineseSimplif);
	if (fontType == WINchn_B5)
		return(kTextEncodingDOSChineseTrad);
	if (fontType == WINjpn)
		return(kTextEncodingDOSJapanese);
	if (fontType == WINKor_HUNG)
		return(kTextEncodingDOSKorean);

	if (fontType == WIN95cain)
		return(kTextEncodingWindowsLatin1);
	if (fontType == WIN95ipa)
		return(kTextEncodingWindowsLatin1);

	if (fontType == WINCAFont)
		return(kTextEncodingWindowsLatin1);
	if (fontType == UNICODEDATA)
		return(kTextEncodingWindowsLatin1);
	if (fontType == WINFxdsys)
		return(kTextEncodingWindowsLatin1);
	return(kTextEncodingWindowsLatin1);
#elif defined(_WIN32)
	if (CharSet == 238)
		return(1250);
	if (CharSet == 222)
		return(874);
	if (CharSet == 204)
		return(1251);
	if (CharSet == 163)
		return(1258);
	if (CharSet == 162)
		return(1254);
	if (CharSet == 136)
		return(950);
	if (CharSet == 134)
		return(936);
	if (CharSet == 129)
		return(949);
	if (CharSet == 128)
		return(932);

	if (fontType == WIN95cour)
		return(1252);
	if (fontType == WIN95courCE)
		return(1250);
	if (fontType == WIN95Latin5)
		return(1254);
	if (fontType == WINchn)
		return(950);
	if (fontType == WINchn_GB)
		return(936);
	if (fontType == WINchn_B5)
		return(950);
	if (fontType == WINjpn)
		return(932);
	if (fontType == WINKor_HUNG)
		return(949);

	if (fontType == WIN95cain)
		return(1252);
	if (fontType == WIN95ipa)
		return(1252);

	if (fontType == UNICODEDATA)
		return(1252);
	if (fontType == WINCAFont)
		return(1252);
	if (fontType == WINFxdsys)
		return(1252);

	if (fontType == COURIER)
		return(CP_MACCP);
	if (fontType == MONACO)
		return(CP_MACCP);
	if (fontType == MACjpn)
		return(932);
	if (fontType == MACchn)
		return(950);
	if (fontType == MACchn_GB)
		return(936);
	if (fontType == MACchn_B5)
		return(950);
	if (fontType == MACKor_HUNG)
		return(949);


	if (fontType == MACipa)
		return(1252);
	if (fontType == MACcain)
		return(1252);

	if (fontType == MacCAFont)
		return(1252);
	if (fontType == MacFxdsys)
		return(1252);


/* 2012-05-03
	if (isShowErr) {
		sprintf(templineC3, "Unrecognized font \"%s%s\". Please install this font on your computer.",fontPref,fontName);
		do_warning(templineC3, 0);
	}
*/
	return(1252);
#else
	return(0);
#endif
}

#ifdef _MAC_CODE
char GetFontNumber(char *fname, short *font) {
#ifndef _COCOA_APP
	Str255 sysFName, pFName;
#endif
	Boolean res; 

	res = TRUE;
#ifndef _COCOA_APP
	c2pstrcpy(pFName, fname);
	*font = FMGetFontFamilyFromName(pFName);
//	GetFNum(pFName, font);
	if (*font == 0) {
		GetFontName(0, sysFName);
		res = EqualString(pFName, sysFName, FALSE, FALSE);
	}
#endif
	return(res);
}
#endif


short getFontType(const char *fontName, char isPC) {
	short type;

	if (!uS.mStricmp(fontName, "ARIAL UNICODE MS") || !uS.mStricmp(fontName, "ARIAL")) {
		type = ((isPC) ? WINArialUC : MACArialUC);
	} else if (!uS.mStricmp(fontName, "CAFONT")) {
		type = ((isPC) ? WINCAFont : MacCAFont);
	} else if (!uS.mStrnicmp(fontName, "FIXEDSYS EXCELSIOR", 18)) {
		type = ((isPC) ? WINFxdsys : MacFxdsys);
	} else {
		type = UNICODEDATA;
	}

	return(type);
}

static void	IDFont(char *tFontName, char isUTF, NewFontInfo *finfo) {
	finfo->fontTable = NULL;
	if (finfo->platform == WIN95DATA) {
		finfo->fontPref = "Win95:";
		if (!strcmp(tFontName, "ARIAL UNICODE MS") || !strcmp(tFontName, "ARIAL"))
			finfo->fontType = WINArialUC;
		else if (!strcmp(tFontName, "CAFONT"/*UNICODEFONT*/))
			finfo->fontType = WINCAFont;
		else if ((finfo->CharSet == 238) &&
			  (!strcmp(tFontName, "COURIER NEW") || !strcmp(tFontName, "ARIAL") ||
			   !strcmp(tFontName, "COURIER NEW CE") || !strcmp(tFontName, "ARIAL CE") ||
			   !strcmp(tFontName, "TIMES NEW ROMAN") || !strcmp(tFontName, "ARIAL UNICODE MS")))
			finfo->fontType = WIN95courCE;
		else if (finfo->CharSet == 222)
			finfo->fontType = WINThai;
		else if ((finfo->CharSet == 162) &&
			  (!strcmp(tFontName, "COURIER NEW") || !strcmp(tFontName, "ARIAL") ||
			   !strcmp(tFontName, "TIMES NEW ROMAN") || !strcmp(tFontName, "ARIAL UNICODE MS")))
			finfo->fontType = WIN95Latin5;
		else if (finfo->CharSet == 134)
			finfo->fontType = WINchn_GB;
		else if (finfo->CharSet == 136)
			finfo->fontType = WINchn_B5;
		else if (finfo->CharSet == 128)
			finfo->fontType = WINjpn;
		else if (finfo->CharSet == 129)
			finfo->fontType = WINKor_HUNG;
		else if (finfo->CharSet == -2 || (finfo->CharSet >= 0 && finfo->CharSet <= 2)) {
			if (!strcmp(tFontName, "COURIER") || !strcmp(tFontName, "COURIER NEW"))
				finfo->fontType = WIN95cour;
			else if (!strcmp(tFontName, "COURIER NEW CE") || !strcmp(tFontName, "ARIAL CE"))
				finfo->fontType = WIN95courCE;
			else if (!strcmp(tFontName, "IPAPHON"))
				finfo->fontType = WIN95ipa;
			else if (!strncmp(tFontName, "FIXEDSYS EXCELSIOR", 18))
				finfo->fontType = WINFxdsys;
			else if (!strcmp(tFontName, "CAFONT"))
				finfo->fontType = WINCAFont;
			else if (!strcmp(tFontName, "CA"))
				finfo->fontType = WIN95cain;
			else if (!strncmp(tFontName, "CHN ", 4))
				finfo->fontType = WINchn;
			else if (!strncmp(tFontName, "JPN ", 4))
				finfo->fontType = WINjpn;
			else
				finfo->fontType = NOCHANGE;
		} else
			finfo->fontType = NOCHANGE;
	} else if (!strcmp(tFontName, "REALDOSCODEPAGE")) {
		finfo->fontPref = "";
		finfo->platform = DOSDATA;
		if (finfo->fontSize == 850) finfo->fontType = DOS850;
		else if (finfo->fontSize == 860) finfo->fontType = DOS860;
		else if (finfo->fontSize == 437) finfo->fontType = DOS850;
		else finfo->fontType = NOCHANGE;
	} else if (!strcmp(tFontName, "REALUNIX")) {
		finfo->fontPref = "";
		finfo->platform = UNIXDATA;
		finfo->fontType = NOCHANGE;
	} else {
		finfo->fontPref = "";
		finfo->platform = MACDATA;
		if (/* isUTF &&  */!strcmp(tFontName, "ARIAL UNICODE MS"))
			finfo->fontType = MACArialUC;
		else if (/* isUTF && */!strcmp(tFontName, "CAFONT"/*UNICODEFONT*/))
			finfo->fontType = MacCAFont;
		else if  (!strcmp(tFontName, "IPAPHON"))
			finfo->fontType = MACipa;
		else if  (!strcmp(tFontName, "CA"))
			finfo->fontType = MACcain;
		else if  (!strcmp(tFontName, "COURIER CE") || !strcmp(tFontName, "GENEVA CE") ||
				  !strcmp(tFontName, "CHICAGO CE"))
			finfo->fontType = MACGenevaCE;
		else if (!strcmp(tFontName, "MONACO")  || !strcmp(tFontName, "GENEVA"))
			finfo->fontType = MONACO;
		else if (!strcmp(tFontName, "COURIER")  || !strcmp(tFontName, "CHICAGO") || 
				 !strcmp(tFontName, "TIMES")	|| !strcmp(tFontName, "CHARCOAL"))
			finfo->fontType = COURIER;
		else if  (!strcmp(tFontName, "OSAKA"))
			finfo->fontType = MACjpn;
		else if  (!strcmp(tFontName, "TAIPEI")  || !strcmp(tFontName, "APPLE LIGOTHIC MEDIUM") ||
				  !strcmp(tFontName, "BIAUKAI") || !strcmp(tFontName, "APPLE LISUNG LIGHT"))
			finfo->fontType = MACchn_B5;
		else if  (!strcmp(tFontName, "BEIJING") || !strcmp(tFontName, "FANG SONG") ||
				  !strcmp(tFontName, "SONG")    || !strcmp(tFontName, "HEI") ||
				  !strcmp(tFontName, "KAI"))
			finfo->fontType = MACchn_GB;
		else if  (!strcmp(tFontName, "APPLEMYUNGJO"))
			finfo->fontType = MACKor_HUNG;
		else if (!strncmp(tFontName, "FIXEDSYS EXCELSIOR", 18))
			finfo->fontType = MacFxdsys;
		else if (!strcmp(tFontName, "CAFONT"))
			finfo->fontType = MacCAFont;
		else if (finfo->CharSet == 0)
			finfo->fontType = COURIER;
		else if (finfo->CharSet == 21)
			finfo->fontType = MACThai;
		else if (finfo->CharSet == 1)
			finfo->fontType = MACjpn;
		else if (finfo->CharSet == 2)
			finfo->fontType = MACchn_B5;
		else  if (finfo->CharSet == 25)
			finfo->fontType = MACchn_GB;
		else  if (finfo->CharSet == 3)
			finfo->fontType = MACKor_HUNG;
		else
			finfo->fontType = NOCHANGE;
	}
	if (!strcmp(tFontName, "ARIAL UNICODE MS")/* && finfo->CharSet < 2*/) {
		if (finfo->platform == WIN95DATA)
			finfo->orgFType = WINArialUC;
		else
			finfo->orgFType = MACArialUC;
	} else if (!strcmp(tFontName,"CAFONT"/*UNICODEFONT*/)/* && finfo->CharSet < 2*/) {
		if (finfo->platform == WIN95DATA)
			finfo->orgFType = WINCAFont;
		else
			finfo->orgFType = MacCAFont;
	} else
		finfo->orgFType = finfo->fontType;
}

const char *GetDatasFont(const char *line, char ec, NewFontInfo *cfinfo, char isUTF, NewFontInfo *finfo) {
	int  i, j;
	char tFontName[256];
	char t[BUFSIZ+1];

	if (line == NULL)
		return(NULL);

	finfo->platform = NOCHANGE;
	for (i=0; isSpace(line[i]) || line[i] == '\n'; i++) ;

	for (j=0; j < BUFSIZ && line[i] && line[i] != ':' && line[i] != ec; i++) {
		t[j++] = line[i];
	}
	t[j] = EOS;
	if (j >= 255)
		return(NULL);
	strcpy(finfo->fontName, t);
	strcpy(tFontName, finfo->fontName);
	uS.uppercasestr(tFontName, cfinfo, FALSE);
	if (line[i] == EOS || line[i] == ec)
		return(NULL);
	if (!strcmp(tFontName, "WIN95"))
		finfo->platform = WIN95DATA;
	for (i++; isSpace(line[i]) || line[i] == '\n'; i++) ;
	if (line[i] == EOS || line[i] == ec)
		return(NULL);

	for (j=0; line[i] && line[i] != ':' && line[i] != ec; i++) {
		t[j++] = line[i];
	}
	t[j] = EOS;	
	if (finfo->platform == WIN95DATA) {
		if (j >= 255)
			return(NULL);
		strcpy(finfo->fontName, t);
		strcpy(tFontName, finfo->fontName);
		uS.uppercasestr(tFontName, cfinfo, FALSE);
		if (line[i] == EOS || line[i] == ec)
			return(NULL);
		for (i++; isSpace(line[i]) || line[i] == '\n'; i++) ;
		if (line[i] == EOS || line[i] == ec)
			return(NULL);
		for (j=0; line[i] && line[i] != ':' && line[i] != ec; i++) {
			t[j++] = line[i];
		}
		t[j] = EOS;			
	}

	finfo->fontSize = atol(t);
	if (finfo->fontSize == 0L)
		return(NULL);

#if defined(_WIN32)
	finfo->CharSet = 1;
#else
	finfo->CharSet = -2;
#endif
	if (line[i] == ':') {
		for (i++; isSpace(line[i]) || line[i] == '\n'; i++) ;
		if (line[i] == EOS || line[i] == ec || !isdigit(line[i])) 
			;
		else {
			for (j=0; line[i] && line[i] != ':' && line[i] != ec; i++) {
				t[j++] = line[i];
			}
			t[j] = EOS;						
			finfo->CharSet = atoi(t);
		}
	}

	if (line[i] != ec) {
		if (line[i] == ':') {
			for (; line[i] && line[i] != ec; i++) ;
			if (line[i] != ec)
				return(NULL);
		} else
			return(NULL);
	}
	if (line[i] != EOS)
		for (i++; isSpace(line[i]) || line[i] == '\n'; i++) ;

	IDFont(tFontName, isUTF, finfo);

	return(line+i);
}

extern "C"
{
extern long Dos850ToASCII(FONTARGS);
extern long Dos860ToASCII(FONTARGS);
extern long MonacoToASCII(FONTARGS);
extern long MonacoToWin95courier(FONTARGS);
extern long CourierToWin95courier(FONTARGS);
extern long Win95courierToCourier(FONTARGS);
extern long Win95courierToDos850(FONTARGS);
extern long Dos850ToMonaco(FONTARGS);
extern long Dos860ToCourier(FONTARGS);
extern long Dos860ToWin95courier(FONTARGS);
extern long Dos850ToWin95courier(FONTARGS);
extern long MonacoToDos850(FONTARGS);
extern long CourierToDos860(FONTARGS);
extern long MacCAToWin95CA(FONTARGS);
extern long Win95CAToMacCA(FONTARGS);
extern long MacCourierCEToWin95CourCE(FONTARGS);
extern long Win95CourCEToMacCourierCE(FONTARGS);
}

char FindTTable(NewFontInfo *finfo, short thisPlatform) {
	finfo->fontTable = NULL;
	if (thisPlatform == MACDATA) {
		if (finfo->platform == DOSDATA) {
			if (finfo->fontType == DOS850) {
				strcpy(finfo->fontName, "Monaco");
				finfo->fontPref = "";
				finfo->fontSize = 9;
				finfo->CharSet = 0;
				finfo->fontTable = Dos850ToMonaco;
				return(TRUE);
			} else if (finfo->fontType == DOS860) {
				strcpy(finfo->fontName, "Courier");
				finfo->fontPref = "";
				finfo->fontSize = 10;
				finfo->CharSet = 0;
				finfo->fontTable = Dos860ToCourier;
				return(TRUE);
			}
		} else if (finfo->platform == WIN95DATA) {
			if (finfo->fontType == WIN95cour) {
				strcpy(finfo->fontName, "Courier");
				finfo->fontPref = "";
				finfo->fontSize = 10;
				finfo->CharSet = 0;
				finfo->fontTable = Win95courierToCourier;
				return(TRUE);
			} else if (finfo->fontType == WIN95ipa) {
				strcpy(finfo->fontName, "IPAPhon");
				finfo->fontPref = "";
				finfo->fontSize = 12;
				finfo->CharSet = 0;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WIN95cain) {
				strcpy(finfo->fontName, "CA");
				finfo->fontPref = "";
				finfo->fontSize = 12;
				finfo->CharSet = 0;
				finfo->fontTable = Win95CAToMacCA;
				return(TRUE);
			} else if (finfo->fontType == WIN95courCE) {
				strcpy(finfo->fontName, "Courier CE");
				finfo->fontPref = "";
				finfo->fontSize = 10;
				finfo->CharSet = 0;
				finfo->fontTable = Win95CourCEToMacCourierCE;
				return(TRUE);
			} else if (finfo->fontType == WINFxdsys) {
				strcpy(finfo->fontName, "Fixedsys Excelsior 2.00");
				finfo->fontPref = "";
				finfo->fontSize = 15;
				finfo->CharSet = 0;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WINCAFont) {
				strcpy(finfo->fontName, "CAfont");
				finfo->fontPref = "";
				finfo->fontSize = 14;
				finfo->CharSet = 0;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WINArialUC) {
#if !defined(UNX)
				if (ArialUnicodeFOND) {
					strcpy(finfo->fontName, "Cambria");
					finfo->fontPref = "";
					finfo->fontSize = 14;
					finfo->CharSet = 0;
					finfo->fontTable = NULL;
				} else {
#endif
					strcpy(finfo->fontName, defUniFontName);
					finfo->fontPref = "";
					finfo->fontSize = defUniFontSize;
					finfo->CharSet = 0;
					finfo->fontTable = NULL;
#if !defined(UNX)
				}
#endif
				return(TRUE);
			} else if (finfo->fontType == WINThai) {
				finfo->orgEncod = finfo->CharSet;
				strcpy(finfo->fontName, "LxS SpEcIaL FoNt");
				finfo->fontPref = "";
				finfo->fontSize = 13;
				finfo->CharSet = kTextEncodingMacThai;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WIN95Latin5) {
				finfo->orgEncod = finfo->CharSet;
				strcpy(finfo->fontName, "LxS SpEcIaL FoNt");
				finfo->fontPref = "";
				finfo->fontSize = 14;
				finfo->CharSet = kTextEncodingWindowsLatin5;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WINchn) {
				strcpy(finfo->fontName, "Taipei");
				finfo->fontPref = "";
				finfo->fontSize = 12;
				finfo->CharSet = 2;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WINchn_GB) {
				strcpy(finfo->fontName, "Beijing");
				finfo->fontPref = "";
				finfo->fontSize = 12;
				finfo->CharSet = 25;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WINchn_B5) {
				strcpy(finfo->fontName, "Taipei");
				finfo->fontPref = "";
				finfo->fontSize = 12;
				finfo->CharSet = 2;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WINjpn) {
				int i;
				strcpy(finfo->fontName, "Osaka");
				i = strlen(finfo->fontName);
				finfo->fontName[i++] = 0x81;
				finfo->fontName[i++] = 0x7C;
				finfo->fontName[i++] = 0x93;
				finfo->fontName[i++] = 0x99;
				finfo->fontName[i++] = 0x95;
				finfo->fontName[i++] = 0x9D;
				finfo->fontName[i] = EOS;
				finfo->fontPref = "";
				if (finfo->fontSize >= -15)
					finfo->fontSize = 12;
				else if (finfo->fontSize <= -19)
					finfo->fontSize = 16;
				else
					finfo->fontSize = 14;
				finfo->CharSet = 1;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == WINKor_HUNG) {
				strcpy(finfo->fontName, "AppleMyungjo");
				finfo->fontPref = "";
				finfo->fontSize = 10;
				finfo->CharSet = 3;
				finfo->fontTable = NULL;
				return(TRUE);
			}
/*
		} else if (finfo->platform == UNICODEDATA) {
			if (finfo->fontType == UNICYRIL) {
				strcpy(finfo->fontName, "Latinskij");
				finfo->fontPref = "";
				finfo->fontSize = 14;
				finfo->CharSet = 0;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == UNIIPAPH) {
				strcpy(finfo->fontName, "IPAPhon");
				finfo->fontPref = "";
				finfo->fontSize = 12;
				finfo->CharSet = 0;
				finfo->fontTable = NULL;
				return(TRUE);
			}
*/
		}
	} else if (thisPlatform == WIN95DATA) {
		if (finfo->platform == MACDATA) {
			if (finfo->fontType == COURIER) {
				strcpy(finfo->fontName, "Courier");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -14;
				finfo->CharSet = 1;
				finfo->fontTable = CourierToWin95courier;
				return(TRUE);
			} else if (finfo->fontType == MONACO) {
				strcpy(finfo->fontName, "Courier");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -14;
				finfo->CharSet = 1;
				finfo->fontTable = MonacoToWin95courier;
				return(TRUE);
			} else if (finfo->fontType == MACipa) {
				strcpy(finfo->fontName, "IPAPhon");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -14;
				finfo->CharSet = 2;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == MACcain) {
				strcpy(finfo->fontName, "CA");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -12;
				finfo->CharSet = 1;
				finfo->fontTable = MacCAToWin95CA;
				return(TRUE);
			} else if (finfo->fontType == MacFxdsys) {
				strcpy(finfo->fontName, "Fixedsys Excelsior 2.00");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -16;
				finfo->CharSet = 0;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == MacCAFont) {
				strcpy(finfo->fontName, "CAfont");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -15;
				finfo->CharSet = 0;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == MACGenevaCE) {
				strcpy(finfo->fontName, "Courier New");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -13;
				finfo->CharSet = 238;
				finfo->fontTable = MacCourierCEToWin95CourCE;
				return(TRUE);
			} else if (finfo->fontType == MACjpn) {
				strcpy(finfo->fontName, "MS Mincho");
				finfo->fontPref = "Win95:";
				if (finfo->fontSize < 14)
					finfo->fontSize = -15;
				else if (finfo->fontSize > 14)
					finfo->fontSize = -19;
				else
					finfo->fontSize = -16;
				finfo->CharSet = 128;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == MACchn) {
				strcpy(finfo->fontName, "Chn System");
				finfo->fontPref = "Win95:";
				finfo->fontSize = defUniFontSize;
				finfo->CharSet = 1;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == MACchn_GB) {
				strcpy(finfo->fontName, defUniFontName);
				finfo->fontPref = "Win95:";
				finfo->fontSize = defUniFontSize;
				finfo->CharSet = 134;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == MACchn_B5) {
				strcpy(finfo->fontName, defUniFontName);
				finfo->fontPref = "Win95:";
				finfo->fontSize = defUniFontSize;
				finfo->CharSet = 136;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == MACKor_HUNG) {
				strcpy(finfo->fontName, defUniFontName);
				finfo->fontPref = "Win95:";
				finfo->fontSize = defUniFontSize;
				finfo->CharSet = 129;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == MACArialUC) {
#if !defined(UNX)
				if (ArialUnicodeFOND) {
					strcpy(finfo->fontName, "Arial Unicode MS");
					finfo->fontPref = "Win95:";
					finfo->fontSize = -13;
					finfo->CharSet = 0;
					finfo->fontTable = NULL;
				} else {
#endif
					strcpy(finfo->fontName, defUniFontName);
					finfo->fontPref = "Win95:";
					finfo->fontSize = defUniFontSize;
					finfo->CharSet = 0;
					finfo->fontTable = NULL;
#if !defined(UNX)
				}
#endif
				return(TRUE);
			}
/*
		} else if (finfo->platform == UNICODEDATA) {
			if (finfo->fontType == UNICYRIL) {
				strcpy(finfo->fontName, "Courier New");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -13;
				finfo->CharSet = 204;
				finfo->fontTable = NULL;
				return(TRUE);
			} else if (finfo->fontType == UNIIPAPH) {
				strcpy(finfo->fontName, "IPAPhon");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -14;
				finfo->CharSet = 2;
				finfo->fontTable = NULL;
				return(TRUE);
			}
*/
		} else if (finfo->platform == DOSDATA) {
			if (finfo->fontType == DOS850) {
				strcpy(finfo->fontName, "Courier");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -13;
				finfo->CharSet = 1;
				finfo->fontTable = Dos850ToWin95courier;
				return(TRUE);
			} else if (finfo->fontType == DOS860) {
				strcpy(finfo->fontName, "Courier");
				finfo->fontPref = "Win95:";
				finfo->fontSize = -13;
				finfo->CharSet = 1;
				finfo->fontTable = Dos860ToWin95courier;
				return(TRUE);
			}
		}
	} else if (thisPlatform == DOSDATA) {
		if (finfo->platform == MACDATA) {
			if (finfo->fontType == MONACO) {
				strcpy(finfo->fontName, "RealDosCodePage");
				finfo->fontPref = "";
				finfo->fontSize = 850;
				finfo->CharSet = 0;
				finfo->fontTable = MonacoToDos850;
				return(TRUE);
			} else if (finfo->fontType == COURIER) {
				strcpy(finfo->fontName, "RealDosCodePage");
				finfo->fontPref = "";
				finfo->fontSize = 860;
				finfo->CharSet = 0;
				finfo->fontTable = CourierToDos860;
				return(TRUE);
			}
		} else if (finfo->platform == WIN95DATA) {
			if (finfo->fontType == WIN95cour) {
				strcpy(finfo->fontName, "RealDosCodePage");
				finfo->fontPref = "";
				finfo->fontSize = 850;
				finfo->CharSet = 0;
				finfo->fontTable = Win95courierToDos850;
				return(TRUE);
			}
		}
	} else if (thisPlatform == UNIXDATA) {
		if (finfo->platform == MACDATA) {
			if (finfo->fontType == MONACO) {
				strcpy(finfo->fontName, "RealUNIX");
				finfo->fontPref = "";
				finfo->fontSize = 0;
				finfo->CharSet = 0;
				finfo->fontTable = MonacoToASCII;
				return(TRUE);
			}
		} else if (finfo->platform == DOSDATA) {
			if (finfo->fontType == DOS850) {
				strcpy(finfo->fontName, "RealUNIX");
				finfo->fontPref = "";
				finfo->fontSize = 0;
				finfo->CharSet = 0;
				finfo->fontTable = Dos850ToASCII;
				return(TRUE);
			} else if (finfo->fontType == DOS860) {
				strcpy(finfo->fontName, "RealUNIX");
				finfo->fontPref = "";
				finfo->fontSize = 0;
				finfo->CharSet = 0;
				finfo->fontTable = Dos860ToASCII;
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

#define AddStr(s) { strcpy(st, (s)); return(strlen(s)); }

long Dos850ToASCII(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\216') AddStr("^A\"")
	else if (c[*index] == '\217') AddStr("^A0")
	else if (c[*index] == '\200') AddStr("^Cc")
	else if (c[*index] == '\220') AddStr("^E'")
	else if (c[*index] == '\245') AddStr("^N~")
	else if (c[*index] == '\231') AddStr("^O\"")
	else if (c[*index] == '\232') AddStr("^U\"")
	else if (c[*index] == '\240') AddStr("^a'")
	else if (c[*index] == '\205') AddStr("^a`")
	else if (c[*index] == '\203') AddStr("^a^")
	else if (c[*index] == '\204') AddStr("^a\"")
	else if (c[*index] == '\306') AddStr("^a~")
	else if (c[*index] == '\206') AddStr("^a0")
	else if (c[*index] == '\207') AddStr("^cc")
	else if (c[*index] == '\202') AddStr("^e'")
	else if (c[*index] == '\212') AddStr("^e`")
	else if (c[*index] == '\210') AddStr("^e^")
	else if (c[*index] == '\211') AddStr("^e\"")
	else if (c[*index] == '\241') AddStr("^i'")
	else if (c[*index] == '\215') AddStr("^i`")
	else if (c[*index] == '\214') AddStr("^i^")
	else if (c[*index] == '\213') AddStr("^i\"")
	else if (c[*index] == '\244') AddStr("^n~")
	else if (c[*index] == '\242') AddStr("^o'")
	else if (c[*index] == '\225') AddStr("^o`")
	else if (c[*index] == '\223') AddStr("^o^")
	else if (c[*index] == '\224') AddStr("^o\"")
	else if (c[*index] == '\344') AddStr("^o~")
	else if (c[*index] == '\243') AddStr("^u'")
	else if (c[*index] == '\227') AddStr("^u`")
	else if (c[*index] == '\226') AddStr("^u^")
	else if (c[*index] == '\201') AddStr("^u\"")
	else if (c[*index] == '\341') AddStr("^ss")
	else if (c[*index] == '\222') AddStr("^AE")
	else if (c[*index] == '\235') AddStr("^O/")
	else if (c[*index] == '\221') AddStr("^ae")
	else if (c[*index] == '\233') AddStr("^o/")
	else if (c[*index] == '\267') AddStr("^A`")
	else if (c[*index] == '\307') AddStr("^A~")
	else if (c[*index] == '\345') AddStr("^O~")
	else if (c[*index] == '\230') AddStr("^y\"")

	else if (c[*index] == '\265') AddStr("^A'")
	else if (c[*index] == '\266') AddStr("^A^")
	else if (c[*index] == '\320') AddStr("^d\\")
	else if (c[*index] == '\321') AddStr("^D\\")
	else if (c[*index] == '\322') AddStr("^E^")
	else if (c[*index] == '\323') AddStr("^E\"")
	else if (c[*index] == '\324') AddStr("^E`")
	else if (c[*index] == '\326') AddStr("^I'")
	else if (c[*index] == '\327') AddStr("^I^")
	else if (c[*index] == '\330') AddStr("^I\"")
	else if (c[*index] == '\336') AddStr("^I`")
	else if (c[*index] == '\340') AddStr("^O'")
	else if (c[*index] == '\342') AddStr("^O^")
	else if (c[*index] == '\343') AddStr("^O`")
	else if (c[*index] == '\347') AddStr("^PD")
	else if (c[*index] == '\350') AddStr("^pd")
	else if (c[*index] == '\351') AddStr("^U'")
	else if (c[*index] == '\352') AddStr("^U^")
	else if (c[*index] == '\353') AddStr("^U`")
	else if (c[*index] == '\354') AddStr("^y'")
	else if (c[*index] == '\355') AddStr("^Y'")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Dos860ToASCII(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\240') AddStr("^a'")
	else if (c[*index] == '\205') AddStr("^a`")
	else if (c[*index] == '\203') AddStr("^a^")
	else if (c[*index] == '\204') AddStr("^a~")
	else if (c[*index] == '\207') AddStr("^c,")
	else if (c[*index] == '\202') AddStr("^e'")
	else if (c[*index] == '\212') AddStr("^e`")
	else if (c[*index] == '\210') AddStr("^e^")
	else if (c[*index] == '\241') AddStr("^i'")
	else if (c[*index] == '\215') AddStr("^i`")
	else if (c[*index] == '\244') AddStr("^n~")
	else if (c[*index] == '\242') AddStr("^o'")
	else if (c[*index] == '\225') AddStr("^o`")
	else if (c[*index] == '\223') AddStr("^o^")
	else if (c[*index] == '\224') AddStr("^o~")
	else if (c[*index] == '\341') AddStr("^ss")
	else if (c[*index] == '\201') AddStr("^u\"")
	else if (c[*index] == '\243') AddStr("^u'")
	else if (c[*index] == '\227') AddStr("^u`")
	else if (c[*index] == '\221') AddStr("^A`")
	else if (c[*index] == '\216') AddStr("^A~")
	else if (c[*index] == '\206') AddStr("^A'")
	else if (c[*index] == '\217') AddStr("^A^")
	else if (c[*index] == '\200') AddStr("^C,")
	else if (c[*index] == '\220') AddStr("^E'")
	else if (c[*index] == '\211') AddStr("^E^")
	else if (c[*index] == '\222') AddStr("^E`")
	else if (c[*index] == '\213') AddStr("^I'")
	else if (c[*index] == '\230') AddStr("^I`")
	else if (c[*index] == '\245') AddStr("^N~")
	else if (c[*index] == '\231') AddStr("^O~")
	else if (c[*index] == '\237') AddStr("^O'")
	else if (c[*index] == '\214') AddStr("^O^")
	else if (c[*index] == '\251') AddStr("^O`")
	else if (c[*index] == '\232') AddStr("^U\"")
	else if (c[*index] == '\226') AddStr("^U'")
	else if (c[*index] == '\235') AddStr("^U`")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long MonacoToASCII(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("^A\"")
	else if (c[*index] == '\201') AddStr("^A0")
	else if (c[*index] == '\202') AddStr("^Cc")
	else if (c[*index] == '\203') AddStr("^E'")
	else if (c[*index] == '\204') AddStr("^N~")
	else if (c[*index] == '\205') AddStr("^O\"")
	else if (c[*index] == '\206') AddStr("^U\"")
	else if (c[*index] == '\207') AddStr("^a'")
	else if (c[*index] == '\210') AddStr("^a`")
	else if (c[*index] == '\211') AddStr("^a^")
	else if (c[*index] == '\212') AddStr("^a\"")
	else if (c[*index] == '\213') AddStr("^a~")
	else if (c[*index] == '\214') AddStr("^a0")
	else if (c[*index] == '\215') AddStr("^cc")
	else if (c[*index] == '\216') AddStr("^e'")
	else if (c[*index] == '\217') AddStr("^e`")
	else if (c[*index] == '\220') AddStr("^e^")
	else if (c[*index] == '\221') AddStr("^e\"")
	else if (c[*index] == '\222') AddStr("^i'")
	else if (c[*index] == '\223') AddStr("^i`")
	else if (c[*index] == '\224') AddStr("^i^")
	else if (c[*index] == '\225') AddStr("^i\"")
	else if (c[*index] == '\226') AddStr("^n~")
	else if (c[*index] == '\227') AddStr("^o'")
	else if (c[*index] == '\230') AddStr("^o`")
	else if (c[*index] == '\231') AddStr("^o^")
	else if (c[*index] == '\232') AddStr("^o\"")
	else if (c[*index] == '\233') AddStr("^o~")
	else if (c[*index] == '\234') AddStr("^u'")
	else if (c[*index] == '\235') AddStr("^u`")
	else if (c[*index] == '\236') AddStr("^u^")
	else if (c[*index] == '\237') AddStr("^u\"")
	else if (c[*index] == '\247') AddStr("^ss")
	else if (c[*index] == '\256') AddStr("^AE")
	else if (c[*index] == '\257') AddStr("^O/")
	else if (c[*index] == '\276') AddStr("^ae")
	else if (c[*index] == '\277') AddStr("^o/")
	else if (c[*index] == '\313') AddStr("^A`")
	else if (c[*index] == '\314') AddStr("^A~")
	else if (c[*index] == '\315') AddStr("^O~")
	else if (c[*index] == '\316') AddStr("^OE")
	else if (c[*index] == '\317') AddStr("^oe")
	else if (c[*index] == '\330') AddStr("^y\"")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long MonacoToWin95courier(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("\304")
	else if (c[*index] == '\201') AddStr("\305")
	else if (c[*index] == '\202') AddStr("\307")
	else if (c[*index] == '\203') AddStr("\311")
	else if (c[*index] == '\204') AddStr("\321")
	else if (c[*index] == '\205') AddStr("\326")
	else if (c[*index] == '\206') AddStr("\334")
	else if (c[*index] == '\207') AddStr("\341")
	else if (c[*index] == '\210') AddStr("\340")
	else if (c[*index] == '\211') AddStr("\342")
	else if (c[*index] == '\212') AddStr("\344")
	else if (c[*index] == '\213') AddStr("\343")
	else if (c[*index] == '\214') AddStr("\345")
	else if (c[*index] == '\215') AddStr("\347")
	else if (c[*index] == '\216') AddStr("\351")
	else if (c[*index] == '\217') AddStr("\350")
	else if (c[*index] == '\220') AddStr("\352")
	else if (c[*index] == '\221') AddStr("\353")
	else if (c[*index] == '\222') AddStr("\355")
	else if (c[*index] == '\223') AddStr("\354")
	else if (c[*index] == '\224') AddStr("\356")
	else if (c[*index] == '\225') AddStr("\357")
	else if (c[*index] == '\226') AddStr("\361")
	else if (c[*index] == '\227') AddStr("\363")
	else if (c[*index] == '\230') AddStr("\362")
	else if (c[*index] == '\231') AddStr("\364")
	else if (c[*index] == '\232') AddStr("\366")
	else if (c[*index] == '\233') AddStr("\365")
	else if (c[*index] == '\234') AddStr("\372")
	else if (c[*index] == '\235') AddStr("\371")
	else if (c[*index] == '\236') AddStr("\373")
	else if (c[*index] == '\237') AddStr("\374")
	else if (c[*index] == '\244') AddStr("\247")
	else if (c[*index] == '\246') AddStr("\266")
	else if (c[*index] == '\247') AddStr("\337")
	else if (c[*index] == '\256') AddStr("\306")
	else if (c[*index] == '\257') AddStr("\330")
	else if (c[*index] == '\264') AddStr("\245")
	else if (c[*index] == '\266') AddStr("\360")
	else if (c[*index] == '\276') AddStr("\234")
	else if (c[*index] == '\277') AddStr("\370")
	else if (c[*index] == '\300') AddStr("\277")
	else if (c[*index] == '\313') AddStr("\300")
	else if (c[*index] == '\314') AddStr("\303")
	else if (c[*index] == '\315') AddStr("\325")
	else if (c[*index] == '\316') AddStr("\214")
	else if (c[*index] == '\317') AddStr("\346")
	else if (c[*index] == '\322') AddStr("\223")
	else if (c[*index] == '\323') AddStr("\224")
	else if (c[*index] == '\324') AddStr("\140")
	else if (c[*index] == '\325') AddStr("\221")
	else if (c[*index] == '\330') AddStr("\377")
	else if (c[*index] == '\331') AddStr("\237")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long MacCAToWin95CA(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("\304")
	else if (c[*index] == '\201') AddStr("\305")
	else if (c[*index] == '\202') AddStr("\307")
	else if (c[*index] == '\203') AddStr("\311")
	else if (c[*index] == '\204') AddStr("\321")
	else if (c[*index] == '\205') AddStr("\326")
	else if (c[*index] == '\206') AddStr("\334")
	else if (c[*index] == '\207') AddStr("\341")
	else if (c[*index] == '\210') AddStr("\340")
	else if (c[*index] == '\211') AddStr("\342")
	else if (c[*index] == '\212') AddStr("\344")
	else if (c[*index] == '\213') AddStr("\343")
	else if (c[*index] == '\214') AddStr("\345")
	else if (c[*index] == '\215') AddStr("\347")
	else if (c[*index] == '\216') AddStr("\351")
	else if (c[*index] == '\217') AddStr("\350")
	else if (c[*index] == '\220') AddStr("\352")
	else if (c[*index] == '\221') AddStr("\353")
	else if (c[*index] == '\222') AddStr("\355")
	else if (c[*index] == '\223') AddStr("\354")
	else if (c[*index] == '\224') AddStr("\356")
	else if (c[*index] == '\225') AddStr("\357")
	else if (c[*index] == '\226') AddStr("\361")
	else if (c[*index] == '\227') AddStr("\363")
	else if (c[*index] == '\230') AddStr("\362")
	else if (c[*index] == '\231') AddStr("\364")
	else if (c[*index] == '\232') AddStr("\366")
	else if (c[*index] == '\233') AddStr("\365")
	else if (c[*index] == '\234') AddStr("\372")
	else if (c[*index] == '\235') AddStr("\371")
	else if (c[*index] == '\236') AddStr("\373")
	else if (c[*index] == '\237') AddStr("\374")
	else if (c[*index] == '\240') AddStr("\206")
	else if (c[*index] == '\242') AddStr("\242")
	else if (c[*index] == '\243') AddStr("\243")
	else if (c[*index] == '\244') AddStr("\247")
	else if (c[*index] == '\246') AddStr("\266")
	else if (c[*index] == '\247') AddStr("\337")
	else if (c[*index] == '\250') AddStr("\256")
	else if (c[*index] == '\253') AddStr("\222")
	else if (c[*index] == '\254') AddStr("\230")
	else if (c[*index] == '\256') AddStr("\306")
	else if (c[*index] == '\257') AddStr("\330")
	else if (c[*index] == '\264') AddStr("\245")
	else if (c[*index] == '\266') AddStr("\360")
	else if (c[*index] == '\276') AddStr("\346")
	else if (c[*index] == '\277') AddStr("\370")
	else if (c[*index] == '\300') AddStr("\277")
	else if (c[*index] == '\301') AddStr("\241")
	else if (c[*index] == '\302') AddStr("\254")
	else if (c[*index] == '\304') AddStr("\203")
	else if (c[*index] == '\307') AddStr("\253")
	else if (c[*index] == '\310') AddStr("\273")
	else if (c[*index] == '\311') AddStr("\205")
	else if (c[*index] == '\312') AddStr("\240")
	else if (c[*index] == '\313') AddStr("\300")
	else if (c[*index] == '\314') AddStr("\303")
	else if (c[*index] == '\315') AddStr("\325")
	else if (c[*index] == '\316') AddStr("\214")
	else if (c[*index] == '\317') AddStr("\234")
	else if (c[*index] == '\322') AddStr("\245") // open up half sq
	else if (c[*index] == '\323') AddStr("\253") // open down half sq
	else if (c[*index] == '\324') AddStr("\244") // closed up half sq
	else if (c[*index] == '\325') AddStr("\252") // closed down half sq
	else if (c[*index] == '\326') AddStr("\367")
	else if (c[*index] == '\330') AddStr("\377")
	else if (c[*index] == '\331') AddStr("\237")
	else if (c[*index] == '\345') AddStr("\305")
	else if (c[*index] == '\346') AddStr("\312")
	else if (c[*index] == '\347') AddStr("\301")
	else if (c[*index] == '\350') AddStr("\313")
	else if (c[*index] == '\351') AddStr("\310")
	else if (c[*index] == '\352') AddStr("\315")
	else if (c[*index] == '\353') AddStr("\316")
	else if (c[*index] == '\354') AddStr("\317")
	else if (c[*index] == '\355') AddStr("\314")
	else if (c[*index] == '\356') AddStr("\323")
	else if (c[*index] == '\357') AddStr("\324")
	else if (c[*index] == '\361') AddStr("\322")
	else if (c[*index] == '\362') AddStr("\332")
	else if (c[*index] == '\363') AddStr("\333")
	else if (c[*index] == '\364') AddStr("\331")
	else if (c[*index] == '\274') AddStr("\272")
	else if (c[*index] == '\303') AddStr("\257")
	else if (c[*index] == '\245') AddStr("\225")
	else if (c[*index] == '\325') AddStr("\047")
	else if (c[*index] == '\241') AddStr("\272")
	else if (c[*index] == '\341') AddStr("\225")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Win95CAToMacCA(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\304') AddStr("\200")
	else if (c[*index] == '\305') AddStr("\201")
	else if (c[*index] == '\307') AddStr("\202")
	else if (c[*index] == '\311') AddStr("\203")
	else if (c[*index] == '\321') AddStr("\204")
	else if (c[*index] == '\326') AddStr("\205")
	else if (c[*index] == '\334') AddStr("\206")
	else if (c[*index] == '\341') AddStr("\207")
	else if (c[*index] == '\340') AddStr("\210")
	else if (c[*index] == '\342') AddStr("\211")
	else if (c[*index] == '\344') AddStr("\212")
	else if (c[*index] == '\343') AddStr("\213")
	else if (c[*index] == '\345') AddStr("\214")
	else if (c[*index] == '\347') AddStr("\215")
	else if (c[*index] == '\351') AddStr("\216")
	else if (c[*index] == '\350') AddStr("\217")
	else if (c[*index] == '\352') AddStr("\220")
	else if (c[*index] == '\353') AddStr("\221")
	else if (c[*index] == '\355') AddStr("\222")
	else if (c[*index] == '\354') AddStr("\223")
	else if (c[*index] == '\356') AddStr("\224")
	else if (c[*index] == '\357') AddStr("\225")
	else if (c[*index] == '\361') AddStr("\226")
	else if (c[*index] == '\363') AddStr("\227")
	else if (c[*index] == '\362') AddStr("\230")
	else if (c[*index] == '\364') AddStr("\231")
	else if (c[*index] == '\366') AddStr("\232")
	else if (c[*index] == '\365') AddStr("\233")
	else if (c[*index] == '\372') AddStr("\234")
	else if (c[*index] == '\371') AddStr("\235")
	else if (c[*index] == '\373') AddStr("\236")
	else if (c[*index] == '\374') AddStr("\237")
	else if (c[*index] == '\206') AddStr("\240")
	else if (c[*index] == '\272') AddStr("\274")
	else if (c[*index] == '\247') AddStr("\244")
	else if (c[*index] == '\257') AddStr("\303")
	else if (c[*index] == '\266') AddStr("\246")
	else if (c[*index] == '\337') AddStr("\247")
	else if (c[*index] == '\256') AddStr("\250")
	else if (c[*index] == '\222') AddStr("\253")
	else if (c[*index] == '\230') AddStr("\254")
	else if (c[*index] == '\306') AddStr("\256")
	else if (c[*index] == '\330') AddStr("\257")
	else if (c[*index] == '\360') AddStr("\266")
	else if (c[*index] == '\346') AddStr("\276")
	else if (c[*index] == '\370') AddStr("\277")
	else if (c[*index] == '\277') AddStr("\300")
	else if (c[*index] == '\241') AddStr("\301")
	else if (c[*index] == '\254') AddStr("\302")
	else if (c[*index] == '\203') AddStr("\304")
	else if (c[*index] == '\273') AddStr("\310")
	else if (c[*index] == '\205') AddStr("\311")
	else if (c[*index] == '\240') AddStr("\312")
	else if (c[*index] == '\300') AddStr("\313")
	else if (c[*index] == '\303') AddStr("\314")
	else if (c[*index] == '\325') AddStr("\315")
	else if (c[*index] == '\214') AddStr("\316")
	else if (c[*index] == '\234') AddStr("\317")
	else if (c[*index] == '\245') AddStr("\322") // open up half sq
	else if (c[*index] == '\253') AddStr("\323") // open down half sq
	else if (c[*index] == '\244') AddStr("\324") // closed up half sq
	else if (c[*index] == '\252') AddStr("\325") // closed down half sq
	else if (c[*index] == '\367') AddStr("\326")
	else if (c[*index] == '\377') AddStr("\330")
	else if (c[*index] == '\237') AddStr("\331")
	else if (c[*index] == '\305') AddStr("\345")
	else if (c[*index] == '\312') AddStr("\346")
	else if (c[*index] == '\301') AddStr("\347")
	else if (c[*index] == '\313') AddStr("\350")
	else if (c[*index] == '\310') AddStr("\351")
	else if (c[*index] == '\315') AddStr("\352")
	else if (c[*index] == '\316') AddStr("\353")
	else if (c[*index] == '\317') AddStr("\354")
	else if (c[*index] == '\314') AddStr("\355")
	else if (c[*index] == '\323') AddStr("\356")
	else if (c[*index] == '\324') AddStr("\357")
	else if (c[*index] == '\322') AddStr("\361")
	else if (c[*index] == '\332') AddStr("\362")
	else if (c[*index] == '\333') AddStr("\363")
	else if (c[*index] == '\331') AddStr("\364")
	else if (c[*index] == '\225') AddStr("\341")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Win95courierToCourier(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\304') AddStr("\200")
	else if (c[*index] == '\305') AddStr("\201")
	else if (c[*index] == '\307') AddStr("\202")
	else if (c[*index] == '\311') AddStr("\203")
	else if (c[*index] == '\321') AddStr("\204")
	else if (c[*index] == '\326') AddStr("\205")
	else if (c[*index] == '\334') AddStr("\206")
	else if (c[*index] == '\341') AddStr("\207")
	else if (c[*index] == '\340') AddStr("\210")
	else if (c[*index] == '\342') AddStr("\211")
	else if (c[*index] == '\344') AddStr("\212")
	else if (c[*index] == '\343') AddStr("\213")
	else if (c[*index] == '\345') AddStr("\214")
	else if (c[*index] == '\347') AddStr("\215")
	else if (c[*index] == '\351') AddStr("\216")
	else if (c[*index] == '\350') AddStr("\217")
	else if (c[*index] == '\352') AddStr("\220")
	else if (c[*index] == '\353') AddStr("\221")
	else if (c[*index] == '\355') AddStr("\222")
	else if (c[*index] == '\354') AddStr("\223")
	else if (c[*index] == '\356') AddStr("\224")
	else if (c[*index] == '\357') AddStr("\225")
	else if (c[*index] == '\361') AddStr("\226")
	else if (c[*index] == '\363') AddStr("\227")
	else if (c[*index] == '\362') AddStr("\230")
	else if (c[*index] == '\364') AddStr("\231")
	else if (c[*index] == '\366') AddStr("\232")
	else if (c[*index] == '\365') AddStr("\233")
	else if (c[*index] == '\372') AddStr("\234")
	else if (c[*index] == '\371') AddStr("\235")
	else if (c[*index] == '\373') AddStr("\236")
	else if (c[*index] == '\374') AddStr("\237")
	else if (c[*index] == '\206') AddStr("\240")
	else if (c[*index] == '\272') AddStr("\241")
	else if (c[*index] == '\247') AddStr("\244")
	else if (c[*index] == '\266') AddStr("\246")
	else if (c[*index] == '\337') AddStr("\247")
	else if (c[*index] == '\256') AddStr("\250")
	else if (c[*index] == '\222') AddStr("\253")
	else if (c[*index] == '\230') AddStr("\254")
	else if (c[*index] == '\306') AddStr("\256")
	else if (c[*index] == '\330') AddStr("\257")
	else if (c[*index] == '\245') AddStr("\264")
	else if (c[*index] == '\360') AddStr("\266")
	else if (c[*index] == '\346') AddStr("\276")
	else if (c[*index] == '\370') AddStr("\277")
	else if (c[*index] == '\277') AddStr("\300")
	else if (c[*index] == '\241') AddStr("\301")
	else if (c[*index] == '\254') AddStr("\302")
	else if (c[*index] == '\203') AddStr("\304")
	else if (c[*index] == '\253') AddStr("\307")
	else if (c[*index] == '\273') AddStr("\310")
	else if (c[*index] == '\205') AddStr("\311")
	else if (c[*index] == '\240') AddStr("\312")
	else if (c[*index] == '\300') AddStr("\313")
	else if (c[*index] == '\303') AddStr("\314")
	else if (c[*index] == '\325') AddStr("\315")
	else if (c[*index] == '\214') AddStr("\316")
	else if (c[*index] == '\234') AddStr("\317")
	else if (c[*index] == '\221') AddStr("\325")
	else if (c[*index] == '\264') AddStr("\325")
	else if (c[*index] == '\367') AddStr("\326")
	else if (c[*index] == '\377') AddStr("\330")
	else if (c[*index] == '\237') AddStr("\331")
	else if (c[*index] == '\305') AddStr("\345")
	else if (c[*index] == '\312') AddStr("\346")
	else if (c[*index] == '\301') AddStr("\347")
	else if (c[*index] == '\313') AddStr("\350")
	else if (c[*index] == '\310') AddStr("\351")
	else if (c[*index] == '\315') AddStr("\352")
	else if (c[*index] == '\316') AddStr("\353")
	else if (c[*index] == '\317') AddStr("\354")
	else if (c[*index] == '\314') AddStr("\355")
	else if (c[*index] == '\323') AddStr("\356")
	else if (c[*index] == '\324') AddStr("\357")
	else if (c[*index] == '\322') AddStr("\361")
	else if (c[*index] == '\332') AddStr("\362")
	else if (c[*index] == '\333') AddStr("\363")
	else if (c[*index] == '\331') AddStr("\364")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long CourierToWin95courier(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("\304")
	else if (c[*index] == '\201') AddStr("\305")
	else if (c[*index] == '\202') AddStr("\307")
	else if (c[*index] == '\203') AddStr("\311")
	else if (c[*index] == '\204') AddStr("\321")
	else if (c[*index] == '\205') AddStr("\326")
	else if (c[*index] == '\206') AddStr("\334")
	else if (c[*index] == '\207') AddStr("\341")
	else if (c[*index] == '\210') AddStr("\340")
	else if (c[*index] == '\211') AddStr("\342")
	else if (c[*index] == '\212') AddStr("\344")
	else if (c[*index] == '\213') AddStr("\343")
	else if (c[*index] == '\214') AddStr("\345")
	else if (c[*index] == '\215') AddStr("\347")
	else if (c[*index] == '\216') AddStr("\351")
	else if (c[*index] == '\217') AddStr("\350")
	else if (c[*index] == '\220') AddStr("\352")
	else if (c[*index] == '\221') AddStr("\353")
	else if (c[*index] == '\222') AddStr("\355")
	else if (c[*index] == '\223') AddStr("\354")
	else if (c[*index] == '\224') AddStr("\356")
	else if (c[*index] == '\225') AddStr("\357")
	else if (c[*index] == '\226') AddStr("\361")
	else if (c[*index] == '\227') AddStr("\363")
	else if (c[*index] == '\230') AddStr("\362")
	else if (c[*index] == '\231') AddStr("\364")
	else if (c[*index] == '\232') AddStr("\366")
	else if (c[*index] == '\233') AddStr("\365")
	else if (c[*index] == '\234') AddStr("\372")
	else if (c[*index] == '\235') AddStr("\371")
	else if (c[*index] == '\236') AddStr("\373")
	else if (c[*index] == '\237') AddStr("\374")
	else if (c[*index] == '\240') AddStr("\206")
	else if (c[*index] == '\241') AddStr("\272")
	else if (c[*index] == '\242') AddStr("\242")
	else if (c[*index] == '\243') AddStr("\243")
	else if (c[*index] == '\244') AddStr("\247")
	else if (c[*index] == '\246') AddStr("\266")
	else if (c[*index] == '\247') AddStr("\337")
	else if (c[*index] == '\250') AddStr("\256")
	else if (c[*index] == '\253') AddStr("\222")
	else if (c[*index] == '\254') AddStr("\230")
	else if (c[*index] == '\256') AddStr("\306")
	else if (c[*index] == '\257') AddStr("\330")
	else if (c[*index] == '\264') AddStr("\245")
	else if (c[*index] == '\266') AddStr("\360")
	else if (c[*index] == '\276') AddStr("\346")
	else if (c[*index] == '\277') AddStr("\370")
	else if (c[*index] == '\300') AddStr("\277")
	else if (c[*index] == '\301') AddStr("\241")
	else if (c[*index] == '\302') AddStr("\254")
	else if (c[*index] == '\304') AddStr("\203")
	else if (c[*index] == '\307') AddStr("\253")
	else if (c[*index] == '\310') AddStr("\273")
	else if (c[*index] == '\311') AddStr("\205")
	else if (c[*index] == '\312') AddStr("\240")
	else if (c[*index] == '\313') AddStr("\300")
	else if (c[*index] == '\314') AddStr("\303")
	else if (c[*index] == '\315') AddStr("\325")
	else if (c[*index] == '\316') AddStr("\214")
	else if (c[*index] == '\317') AddStr("\234")
	else if (c[*index] == '\324') AddStr("\140")
	else if (c[*index] == '\325') AddStr("\221")
	else if (c[*index] == '\326') AddStr("\367")
	else if (c[*index] == '\330') AddStr("\377")
	else if (c[*index] == '\331') AddStr("\237")
	else if (c[*index] == '\345') AddStr("\305")
	else if (c[*index] == '\346') AddStr("\312")
	else if (c[*index] == '\347') AddStr("\301")
	else if (c[*index] == '\350') AddStr("\313")
	else if (c[*index] == '\351') AddStr("\310")
	else if (c[*index] == '\352') AddStr("\315")
	else if (c[*index] == '\353') AddStr("\316")
	else if (c[*index] == '\354') AddStr("\317")
	else if (c[*index] == '\355') AddStr("\314")
	else if (c[*index] == '\356') AddStr("\323")
	else if (c[*index] == '\357') AddStr("\324")
	else if (c[*index] == '\361') AddStr("\322")
	else if (c[*index] == '\362') AddStr("\332")
	else if (c[*index] == '\363') AddStr("\333")
	else if (c[*index] == '\364') AddStr("\331")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long MacCourierCEToWin95CourCE(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("\304")
	else if (c[*index] == '\201') AddStr("\303")
	else if (c[*index] == '\202') AddStr("\343")
	else if (c[*index] == '\203') AddStr("\311")
	else if (c[*index] == '\204') AddStr("\245")
	else if (c[*index] == '\205') AddStr("\326")
	else if (c[*index] == '\206') AddStr("\334")
	else if (c[*index] == '\207') AddStr("\341")
	else if (c[*index] == '\210') AddStr("\271")
	else if (c[*index] == '\211') AddStr("\310")
	else if (c[*index] == '\212') AddStr("\344")
	else if (c[*index] == '\213') AddStr("\350")
	else if (c[*index] == '\214') AddStr("\306")
	else if (c[*index] == '\215') AddStr("\346")
	else if (c[*index] == '\216') AddStr("\351")
	else if (c[*index] == '\217') AddStr("\217")
	else if (c[*index] == '\220') AddStr("\237")
	else if (c[*index] == '\221') AddStr("\317")
	else if (c[*index] == '\222') AddStr("\355")
	else if (c[*index] == '\223') AddStr("\357")
//	else if (c[*index] == '\224') AddStr("\")
//	else if (c[*index] == '\225') AddStr("\")
//	else if (c[*index] == '\226') AddStr("\")
	else if (c[*index] == '\227') AddStr("\363")
//	else if (c[*index] == '\230') AddStr("\")
	else if (c[*index] == '\231') AddStr("\364")
	else if (c[*index] == '\232') AddStr("\366")
//	else if (c[*index] == '\233') AddStr("\")
	else if (c[*index] == '\234') AddStr("\372")
	else if (c[*index] == '\235') AddStr("\314")
	else if (c[*index] == '\236') AddStr("\354")
	else if (c[*index] == '\237') AddStr("\374")
	else if (c[*index] == '\240') AddStr("\206")
	else if (c[*index] == '\241') AddStr("\260")
	else if (c[*index] == '\242') AddStr("\312")
//	else if (c[*index] == '\243') AddStr("\")
	else if (c[*index] == '\244') AddStr("\247")
	else if (c[*index] == '\245') AddStr("\225")
	else if (c[*index] == '\246') AddStr("\266")
	else if (c[*index] == '\247') AddStr("\337")
	else if (c[*index] == '\250') AddStr("\256")
	else if (c[*index] == '\251') AddStr("\251")
	else if (c[*index] == '\252') AddStr("\231")
	else if (c[*index] == '\253') AddStr("\352")
	else if (c[*index] == '\254') AddStr("\250")
//	else if (c[*index] == '\255') AddStr("\")
//	else if (c[*index] == '\256') AddStr("\")
//	else if (c[*index] == '\257') AddStr("\")
//	else if (c[*index] == '\260') AddStr("\")
//	else if (c[*index] == '\261') AddStr("\")
	else if (c[*index] == '\262') AddStr("\253")
	else if (c[*index] == '\263') AddStr("\273")
//	else if (c[*index] == '\264') AddStr("\")
//	else if (c[*index] == '\265') AddStr("\")
//	else if (c[*index] == '\266') AddStr("\")
//	else if (c[*index] == '\267') AddStr("\")
	else if (c[*index] == '\270') AddStr("\263")
//	else if (c[*index] == '\271') AddStr("\")
//	else if (c[*index] == '\272') AddStr("\")
	else if (c[*index] == '\273') AddStr("\274")
	else if (c[*index] == '\274') AddStr("\276")
	else if (c[*index] == '\275') AddStr("\305")
	else if (c[*index] == '\276') AddStr("\345")
//	else if (c[*index] == '\277') AddStr("\")
//	else if (c[*index] == '\300') AddStr("\")
	else if (c[*index] == '\301') AddStr("\321")
	else if (c[*index] == '\302') AddStr("\254")
//	else if (c[*index] == '\303') AddStr("\")
	else if (c[*index] == '\304') AddStr("\361")
	else if (c[*index] == '\305') AddStr("\322")
//	else if (c[*index] == '\306') AddStr("\")
	else if (c[*index] == '\307') AddStr("\253")
	else if (c[*index] == '\310') AddStr("\273")
	else if (c[*index] == '\311') AddStr("\205")
	else if (c[*index] == '\312') AddStr("\240")
	else if (c[*index] == '\313') AddStr("\362")
	else if (c[*index] == '\314') AddStr("\325")
//	else if (c[*index] == '\315') AddStr("\")
	else if (c[*index] == '\316') AddStr("\365")
//	else if (c[*index] == '\317') AddStr("\")
	else if (c[*index] == '\320') AddStr("\226")
	else if (c[*index] == '\321') AddStr("\227")
	else if (c[*index] == '\322') AddStr("\223")
	else if (c[*index] == '\323') AddStr("\224")
	else if (c[*index] == '\324') AddStr("\221")
	else if (c[*index] == '\325') AddStr("\222")
	else if (c[*index] == '\326') AddStr("\367")
//	else if (c[*index] == '\327') AddStr("\")
//	else if (c[*index] == '\330') AddStr("\")
	else if (c[*index] == '\331') AddStr("\300")
	else if (c[*index] == '\332') AddStr("\340")
	else if (c[*index] == '\333') AddStr("\330")
	else if (c[*index] == '\334') AddStr("\213")
	else if (c[*index] == '\335') AddStr("\233")
	else if (c[*index] == '\336') AddStr("\370")
//	else if (c[*index] == '\337') AddStr("\")
//	else if (c[*index] == '\340') AddStr("\")
	else if (c[*index] == '\341') AddStr("\212")
//	else if (c[*index] == '\342') AddStr("\")
//	else if (c[*index] == '\343') AddStr("\")
	else if (c[*index] == '\344') AddStr("\232")
	else if (c[*index] == '\345') AddStr("\214")
	else if (c[*index] == '\346') AddStr("\234")
	else if (c[*index] == '\347') AddStr("\301")
	else if (c[*index] == '\350') AddStr("\215")
	else if (c[*index] == '\351') AddStr("\235")
	else if (c[*index] == '\352') AddStr("\345")
	else if (c[*index] == '\353') AddStr("\216")
	else if (c[*index] == '\354') AddStr("\236")
//	else if (c[*index] == '\355') AddStr("\")
	else if (c[*index] == '\356') AddStr("\323")
	else if (c[*index] == '\357') AddStr("\324")
//	else if (c[*index] == '\360') AddStr("\")
	else if (c[*index] == '\361') AddStr("\331")
	else if (c[*index] == '\362') AddStr("\332")
	else if (c[*index] == '\363') AddStr("\371")
	else if (c[*index] == '\364') AddStr("\333")
	else if (c[*index] == '\365') AddStr("\373")
//	else if (c[*index] == '\366') AddStr("\")
//	else if (c[*index] == '\367') AddStr("\")
	else if (c[*index] == '\370') AddStr("\335")
	else if (c[*index] == '\371') AddStr("\375")
//	else if (c[*index] == '\372') AddStr("\")
	else if (c[*index] == '\373') AddStr("\257")
	else if (c[*index] == '\374') AddStr("\243")
	else if (c[*index] == '\375') AddStr("\277")
//	else if (c[*index] == '\376') AddStr("\")
//	else if (c[*index] == '\377') AddStr("\")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Win95CourCEToMacCourierCE(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
//	if (c[*index] == '\200') AddStr("\")
//	else if (c[*index] == '\201') AddStr("\")
//	else if (c[*index] == '\202') AddStr("\")
//	else if (c[*index] == '\203') AddStr("\")
//	else if (c[*index] == '\204') AddStr("\")
	if (c[*index] == '\205') AddStr("\311")
	else if (c[*index] == '\206') AddStr("\240")
//	else if (c[*index] == '\207') AddStr("\")
//	else if (c[*index] == '\210') AddStr("\")
//	else if (c[*index] == '\211') AddStr("\")
	else if (c[*index] == '\212') AddStr("\341")
	else if (c[*index] == '\213') AddStr("\334")
	else if (c[*index] == '\214') AddStr("\345")
	else if (c[*index] == '\215') AddStr("\350")
	else if (c[*index] == '\216') AddStr("\353")
	else if (c[*index] == '\217') AddStr("\217")
//	else if (c[*index] == '\220') AddStr("\")
	else if (c[*index] == '\221') AddStr("\324")
	else if (c[*index] == '\222') AddStr("\325")
	else if (c[*index] == '\223') AddStr("\322")
	else if (c[*index] == '\224') AddStr("\323")
	else if (c[*index] == '\225') AddStr("\245")
	else if (c[*index] == '\226') AddStr("\320")
	else if (c[*index] == '\227') AddStr("\321")
//	else if (c[*index] == '\230') AddStr("\")
	else if (c[*index] == '\231') AddStr("\252")
	else if (c[*index] == '\232') AddStr("\344")
	else if (c[*index] == '\233') AddStr("\335")
	else if (c[*index] == '\234') AddStr("\346")
	else if (c[*index] == '\235') AddStr("\351")
	else if (c[*index] == '\236') AddStr("\354")
	else if (c[*index] == '\237') AddStr("\220")
	else if (c[*index] == '\240') AddStr("\312")
//	else if (c[*index] == '\241') AddStr("\")
//	else if (c[*index] == '\242') AddStr("\")
	else if (c[*index] == '\243') AddStr("\374")
//	else if (c[*index] == '\244') AddStr("\")
	else if (c[*index] == '\245') AddStr("\204")
//	else if (c[*index] == '\246') AddStr("\")
	else if (c[*index] == '\247') AddStr("\244")
	else if (c[*index] == '\250') AddStr("\254")
	else if (c[*index] == '\251') AddStr("\251")
//	else if (c[*index] == '\252') AddStr("\")
	else if (c[*index] == '\253') AddStr("\262")
	else if (c[*index] == '\253') AddStr("\307")
//	else if (c[*index] == '\255') AddStr("\")
	else if (c[*index] == '\256') AddStr("\250")
	else if (c[*index] == '\257') AddStr("\373")
	else if (c[*index] == '\260') AddStr("\241")
//	else if (c[*index] == '\261') AddStr("\")
//	else if (c[*index] == '\262') AddStr("\")
	else if (c[*index] == '\263') AddStr("\270")
//	else if (c[*index] == '\264') AddStr("\")
//	else if (c[*index] == '\265') AddStr("\")
	else if (c[*index] == '\266') AddStr("\246")
//	else if (c[*index] == '\267') AddStr("\")
//	else if (c[*index] == '\270') AddStr("\")
	else if (c[*index] == '\271') AddStr("\210")
//	else if (c[*index] == '\272') AddStr("\")
	else if (c[*index] == '\273') AddStr("\310")
	else if (c[*index] == '\274') AddStr("\273")
//	else if (c[*index] == '\275') AddStr("\")
	else if (c[*index] == '\276') AddStr("\274")
	else if (c[*index] == '\277') AddStr("\375")
	else if (c[*index] == '\300') AddStr("\331")
	else if (c[*index] == '\301') AddStr("\347")
//	else if (c[*index] == '\302') AddStr("\")
	else if (c[*index] == '\303') AddStr("\201")
	else if (c[*index] == '\304') AddStr("\200")
	else if (c[*index] == '\305') AddStr("\275")
	else if (c[*index] == '\306') AddStr("\214")
//	else if (c[*index] == '\307') AddStr("\")
	else if (c[*index] == '\310') AddStr("\211")
	else if (c[*index] == '\311') AddStr("\203")
	else if (c[*index] == '\312') AddStr("\242")
//	else if (c[*index] == '\313') AddStr("\")
	else if (c[*index] == '\314') AddStr("\235")
//	else if (c[*index] == '\315') AddStr("\")
//	else if (c[*index] == '\316') AddStr("\")
	else if (c[*index] == '\317') AddStr("\221")
//	else if (c[*index] == '\320') AddStr("\")
	else if (c[*index] == '\321') AddStr("\301")
	else if (c[*index] == '\322') AddStr("\305")
	else if (c[*index] == '\323') AddStr("\356")
	else if (c[*index] == '\324') AddStr("\357")
	else if (c[*index] == '\325') AddStr("\314")
	else if (c[*index] == '\326') AddStr("\205")
//	else if (c[*index] == '\327') AddStr("\")
	else if (c[*index] == '\330') AddStr("\333")
	else if (c[*index] == '\331') AddStr("\361")
	else if (c[*index] == '\332') AddStr("\362")
	else if (c[*index] == '\333') AddStr("\364")
	else if (c[*index] == '\334') AddStr("\206")
	else if (c[*index] == '\335') AddStr("\370")
//	else if (c[*index] == '\336') AddStr("\")
	else if (c[*index] == '\337') AddStr("\247")
	else if (c[*index] == '\340') AddStr("\332")
	else if (c[*index] == '\341') AddStr("\207")
//	else if (c[*index] == '\342') AddStr("\")
	else if (c[*index] == '\343') AddStr("\202")
	else if (c[*index] == '\344') AddStr("\212")
	else if (c[*index] == '\345') AddStr("\276")
	else if (c[*index] == '\346') AddStr("\215")
//	else if (c[*index] == '\347') AddStr("\")
	else if (c[*index] == '\350') AddStr("\213")
	else if (c[*index] == '\351') AddStr("\216")
	else if (c[*index] == '\352') AddStr("\253")
//	else if (c[*index] == '\353') AddStr("\")
	else if (c[*index] == '\354') AddStr("\236")
	else if (c[*index] == '\355') AddStr("\222")
//	else if (c[*index] == '\356') AddStr("\")
	else if (c[*index] == '\357') AddStr("\223")
//	else if (c[*index] == '\360') AddStr("\")
	else if (c[*index] == '\361') AddStr("\304")
	else if (c[*index] == '\362') AddStr("\313")
	else if (c[*index] == '\363') AddStr("\227")
	else if (c[*index] == '\364') AddStr("\231")
	else if (c[*index] == '\365') AddStr("\316")
	else if (c[*index] == '\366') AddStr("\232")
	else if (c[*index] == '\367') AddStr("\326")
	else if (c[*index] == '\370') AddStr("\336")
	else if (c[*index] == '\371') AddStr("\363")
	else if (c[*index] == '\372') AddStr("\234")
	else if (c[*index] == '\373') AddStr("\365")
	else if (c[*index] == '\374') AddStr("\237")
	else if (c[*index] == '\375') AddStr("\371")
//	else if (c[*index] == '\376') AddStr("\")
//	else if (c[*index] == '\377') AddStr("\")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Win95courierToDos850(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\304') AddStr("\216")
	else if (c[*index] == '\305') AddStr("\217")
	else if (c[*index] == '\307') AddStr("\200")
	else if (c[*index] == '\311') AddStr("\220")
	else if (c[*index] == '\321') AddStr("\245")
	else if (c[*index] == '\326') AddStr("\231")
	else if (c[*index] == '\334') AddStr("\232")
	else if (c[*index] == '\341') AddStr("\240")
	else if (c[*index] == '\340') AddStr("\205")
	else if (c[*index] == '\342') AddStr("\203")
	else if (c[*index] == '\344') AddStr("\204")
	else if (c[*index] == '\343') AddStr("\306")
	else if (c[*index] == '\345') AddStr("\206")
	else if (c[*index] == '\347') AddStr("\207")
	else if (c[*index] == '\351') AddStr("\202")
	else if (c[*index] == '\350') AddStr("\212")
	else if (c[*index] == '\352') AddStr("\210")
	else if (c[*index] == '\353') AddStr("\211")
	else if (c[*index] == '\355') AddStr("\241")
	else if (c[*index] == '\354') AddStr("\215")
	else if (c[*index] == '\356') AddStr("\214")
	else if (c[*index] == '\357') AddStr("\213")
	else if (c[*index] == '\361') AddStr("\244")
	else if (c[*index] == '\363') AddStr("\242")
	else if (c[*index] == '\362') AddStr("\225")
	else if (c[*index] == '\364') AddStr("\223")
	else if (c[*index] == '\366') AddStr("\224")
	else if (c[*index] == '\365') AddStr("\344")
	else if (c[*index] == '\372') AddStr("\243")
	else if (c[*index] == '\371') AddStr("\227")
	else if (c[*index] == '\373') AddStr("\226")
	else if (c[*index] == '\374') AddStr("\201")
	else if (c[*index] == '\337') AddStr("\341")
	else if (c[*index] == '\306') AddStr("\222")
	else if (c[*index] == '\330') AddStr("\235")
	else if (c[*index] == '\234') AddStr("\221")
	else if (c[*index] == '\370') AddStr("\233")
	else if (c[*index] == '\300') AddStr("\267")
	else if (c[*index] == '\303') AddStr("\307")
	else if (c[*index] == '\325') AddStr("\345")
	else if (c[*index] == '\377') AddStr("\230")
	else if (c[*index] == '\301') AddStr("\265")
	else if (c[*index] == '\302') AddStr("\266")
	else if (c[*index] == '\360') AddStr("\320")
	else if (c[*index] == '\320') AddStr("\321")
	else if (c[*index] == '\312') AddStr("\322")
	else if (c[*index] == '\313') AddStr("\323")
	else if (c[*index] == '\310') AddStr("\324")
	else if (c[*index] == '\315') AddStr("\326")
	else if (c[*index] == '\316') AddStr("\327")
	else if (c[*index] == '\317') AddStr("\330")
	else if (c[*index] == '\314') AddStr("\336")
	else if (c[*index] == '\323') AddStr("\340")
	else if (c[*index] == '\324') AddStr("\342")
	else if (c[*index] == '\322') AddStr("\343")
	else if (c[*index] == '\332') AddStr("\351")
	else if (c[*index] == '\333') AddStr("\352")
	else if (c[*index] == '\331') AddStr("\353")
	else if (c[*index] == '\375') AddStr("\354")
	else if (c[*index] == '\335') AddStr("\355")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Dos860ToWin95courier(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("\307")
	else if (c[*index] == '\201') AddStr("\374")
	else if (c[*index] == '\202') AddStr("\351")
	else if (c[*index] == '\203') AddStr("\342")
	else if (c[*index] == '\204') AddStr("\343")
	else if (c[*index] == '\205') AddStr("\352")
	else if (c[*index] == '\206') AddStr("\301")
	else if (c[*index] == '\207') AddStr("\347")
	else if (c[*index] == '\210') AddStr("\352")
	else if (c[*index] == '\211') AddStr("\312")
	else if (c[*index] == '\212') AddStr("\350")
	else if (c[*index] == '\213') AddStr("\315")
	else if (c[*index] == '\214') AddStr("\324")
	else if (c[*index] == '\215') AddStr("\223")
	else if (c[*index] == '\216') AddStr("\303")
	else if (c[*index] == '\217') AddStr("\305")
	else if (c[*index] == '\220') AddStr("\311")
	else if (c[*index] == '\221') AddStr("\300")
	else if (c[*index] == '\222') AddStr("\310")
	else if (c[*index] == '\223') AddStr("\364")
	else if (c[*index] == '\224') AddStr("\365")
	else if (c[*index] == '\225') AddStr("\362")
	else if (c[*index] == '\226') AddStr("\332")
	else if (c[*index] == '\227') AddStr("\371")
	else if (c[*index] == '\230') AddStr("\314")
	else if (c[*index] == '\231') AddStr("\325")
	else if (c[*index] == '\232') AddStr("\334")
	else if (c[*index] == '\233') AddStr("\242")
	else if (c[*index] == '\234') AddStr("\243")
	else if (c[*index] == '\235') AddStr("\331")
	else if (c[*index] == '\237') AddStr("\323")
	else if (c[*index] == '\240') AddStr("\341")
	else if (c[*index] == '\241') AddStr("\355")
	else if (c[*index] == '\242') AddStr("\363")
	else if (c[*index] == '\243') AddStr("\372")
	else if (c[*index] == '\244') AddStr("\361")
	else if (c[*index] == '\245') AddStr("\321")
	else if (c[*index] == '\251') AddStr("\322")
	else if (c[*index] == '\341') AddStr("\337")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Dos850ToWin95courier(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\216') AddStr("\304")
	else if (c[*index] == '\217') AddStr("\305")
	else if (c[*index] == '\200') AddStr("\307")
	else if (c[*index] == '\220') AddStr("\311")
	else if (c[*index] == '\245') AddStr("\321")
	else if (c[*index] == '\231') AddStr("\326")
	else if (c[*index] == '\232') AddStr("\334")
	else if (c[*index] == '\240') AddStr("\341")
	else if (c[*index] == '\205') AddStr("\340")
	else if (c[*index] == '\203') AddStr("\342")
	else if (c[*index] == '\204') AddStr("\344")
	else if (c[*index] == '\306') AddStr("\343")
	else if (c[*index] == '\206') AddStr("\345")
	else if (c[*index] == '\207') AddStr("\347")
	else if (c[*index] == '\202') AddStr("\351")
	else if (c[*index] == '\212') AddStr("\350")
	else if (c[*index] == '\210') AddStr("\352")
	else if (c[*index] == '\211') AddStr("\353")
	else if (c[*index] == '\241') AddStr("\355")
	else if (c[*index] == '\215') AddStr("\354")
	else if (c[*index] == '\214') AddStr("\356")
	else if (c[*index] == '\213') AddStr("\357")
	else if (c[*index] == '\244') AddStr("\361")
	else if (c[*index] == '\242') AddStr("\363")
	else if (c[*index] == '\225') AddStr("\362")
	else if (c[*index] == '\223') AddStr("\364")
	else if (c[*index] == '\224') AddStr("\366")
	else if (c[*index] == '\344') AddStr("\365")
	else if (c[*index] == '\243') AddStr("\372")
	else if (c[*index] == '\227') AddStr("\371")
	else if (c[*index] == '\226') AddStr("\373")
	else if (c[*index] == '\201') AddStr("\374")
	else if (c[*index] == '\341') AddStr("\337")
	else if (c[*index] == '\222') AddStr("\306")
	else if (c[*index] == '\235') AddStr("\330")
	else if (c[*index] == '\221') AddStr("\234")
	else if (c[*index] == '\233') AddStr("\370")
	else if (c[*index] == '\267') AddStr("\300")
	else if (c[*index] == '\307') AddStr("\303")
	else if (c[*index] == '\345') AddStr("\325")
	else if (c[*index] == '\230') AddStr("\377")
	else if (c[*index] == '\265') AddStr("\301")
	else if (c[*index] == '\266') AddStr("\302")
	else if (c[*index] == '\320') AddStr("\360")
	else if (c[*index] == '\321') AddStr("\320")
	else if (c[*index] == '\322') AddStr("\312")
	else if (c[*index] == '\323') AddStr("\313")
	else if (c[*index] == '\324') AddStr("\310")
	else if (c[*index] == '\326') AddStr("\315")
	else if (c[*index] == '\327') AddStr("\316")
	else if (c[*index] == '\330') AddStr("\317")
	else if (c[*index] == '\336') AddStr("\314")
	else if (c[*index] == '\340') AddStr("\323")
	else if (c[*index] == '\342') AddStr("\324")
	else if (c[*index] == '\343') AddStr("\322")
	else if (c[*index] == '\351') AddStr("\332")
	else if (c[*index] == '\352') AddStr("\333")
	else if (c[*index] == '\353') AddStr("\331")
	else if (c[*index] == '\354') AddStr("\375")
	else if (c[*index] == '\355') AddStr("\335")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Dos850ToMonaco(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\216') AddStr("\200")
	else if (c[*index] == '\217') AddStr("\201")
	else if (c[*index] == '\200') AddStr("\202")
	else if (c[*index] == '\220') AddStr("\203")
	else if (c[*index] == '\245') AddStr("\204")
	else if (c[*index] == '\231') AddStr("\205")
	else if (c[*index] == '\232') AddStr("\206")
	else if (c[*index] == '\240') AddStr("\207")
	else if (c[*index] == '\205') AddStr("\210")
	else if (c[*index] == '\203') AddStr("\211")
	else if (c[*index] == '\204') AddStr("\212")
	else if (c[*index] == '\306') AddStr("\213")
	else if (c[*index] == '\206') AddStr("\214")
	else if (c[*index] == '\207') AddStr("\215")
	else if (c[*index] == '\202') AddStr("\216")
	else if (c[*index] == '\212') AddStr("\217")
	else if (c[*index] == '\210') AddStr("\220")
	else if (c[*index] == '\211') AddStr("\221")
	else if (c[*index] == '\241') AddStr("\222")
	else if (c[*index] == '\215') AddStr("\223")
	else if (c[*index] == '\214') AddStr("\224")
	else if (c[*index] == '\213') AddStr("\225")
	else if (c[*index] == '\244') AddStr("\226")
	else if (c[*index] == '\242') AddStr("\227")
	else if (c[*index] == '\225') AddStr("\230")
	else if (c[*index] == '\223') AddStr("\231")
	else if (c[*index] == '\224') AddStr("\232")
	else if (c[*index] == '\344') AddStr("\233")
	else if (c[*index] == '\243') AddStr("\234")
	else if (c[*index] == '\227') AddStr("\235")
	else if (c[*index] == '\226') AddStr("\236")
	else if (c[*index] == '\201') AddStr("\237")
	else if (c[*index] == '\341') AddStr("\247")
	else if (c[*index] == '\222') AddStr("\256")
	else if (c[*index] == '\235') AddStr("\257")
	else if (c[*index] == '\221') AddStr("\276")
	else if (c[*index] == '\233') AddStr("\277")
	else if (c[*index] == '\267') AddStr("\313")
	else if (c[*index] == '\307') AddStr("\314")
	else if (c[*index] == '\345') AddStr("\315")
	else if (c[*index] == '\230') AddStr("\330")
	else if (c[*index] == '\265') AddStr("^A'")
	else if (c[*index] == '\266') AddStr("^A^")
	else if (c[*index] == '\320') AddStr("^d\\")
	else if (c[*index] == '\321') AddStr("^D\\")
	else if (c[*index] == '\322') AddStr("^E^")
	else if (c[*index] == '\323') AddStr("^E\"")
	else if (c[*index] == '\324') AddStr("^E`")
	else if (c[*index] == '\326') AddStr("^I'")
	else if (c[*index] == '\327') AddStr("^I^")
	else if (c[*index] == '\330') AddStr("^I\"")
	else if (c[*index] == '\336') AddStr("^I`")
	else if (c[*index] == '\340') AddStr("^O'")
	else if (c[*index] == '\342') AddStr("^O^")
	else if (c[*index] == '\343') AddStr("^O`")
	else if (c[*index] == '\347') AddStr("^PD")
	else if (c[*index] == '\350') AddStr("^pd")
	else if (c[*index] == '\351') AddStr("^U'")
	else if (c[*index] == '\352') AddStr("^U^")
	else if (c[*index] == '\353') AddStr("^U`")
	else if (c[*index] == '\354') AddStr("^y'")
	else if (c[*index] == '\355') AddStr("^Y'")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long Dos860ToCourier(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("\202")
	else if (c[*index] == '\201') AddStr("\237")
	else if (c[*index] == '\202') AddStr("\216")
	else if (c[*index] == '\203') AddStr("\211")
	else if (c[*index] == '\204') AddStr("\213")
	else if (c[*index] == '\205') AddStr("\210")
	else if (c[*index] == '\206') AddStr("\347")
	else if (c[*index] == '\207') AddStr("\215")
	else if (c[*index] == '\210') AddStr("\220")
	else if (c[*index] == '\211') AddStr("\346")
	else if (c[*index] == '\212') AddStr("\217")
	else if (c[*index] == '\213') AddStr("\352")
	else if (c[*index] == '\214') AddStr("^O^")
	else if (c[*index] == '\215') AddStr("\223")
	else if (c[*index] == '\216') AddStr("\314")
	else if (c[*index] == '\217') AddStr("\345")
	else if (c[*index] == '\220') AddStr("\203")
	else if (c[*index] == '\221') AddStr("\313")
	else if (c[*index] == '\222') AddStr("\351")
	else if (c[*index] == '\223') AddStr("\231")
	else if (c[*index] == '\224') AddStr("\233")
	else if (c[*index] == '\225') AddStr("\230")
	else if (c[*index] == '\226') AddStr("\362")
	else if (c[*index] == '\227') AddStr("\235")
	else if (c[*index] == '\230') AddStr("\355")
	else if (c[*index] == '\231') AddStr("\315")
	else if (c[*index] == '\232') AddStr("^U\"")
	else if (c[*index] == '\233') AddStr("\242")
	else if (c[*index] == '\234') AddStr("\243")
	else if (c[*index] == '\235') AddStr("\364")
	else if (c[*index] == '\237') AddStr("\356")
	else if (c[*index] == '\240') AddStr("\207")
	else if (c[*index] == '\241') AddStr("\222")
	else if (c[*index] == '\242') AddStr("\227")
	else if (c[*index] == '\243') AddStr("\234")
	else if (c[*index] == '\244') AddStr("\226")
	else if (c[*index] == '\245') AddStr("\204")
	else if (c[*index] == '\251') AddStr("\361")
	else if (c[*index] == '\341') AddStr("\247")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long MonacoToDos850(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("\216")
	else if (c[*index] == '\201') AddStr("\217")
	else if (c[*index] == '\202') AddStr("\200")
	else if (c[*index] == '\203') AddStr("\220")
	else if (c[*index] == '\204') AddStr("\245")
	else if (c[*index] == '\205') AddStr("\231")
	else if (c[*index] == '\206') AddStr("\232")
	else if (c[*index] == '\207') AddStr("\240")
	else if (c[*index] == '\210') AddStr("\205")
	else if (c[*index] == '\211') AddStr("\203")
	else if (c[*index] == '\212') AddStr("\204")
	else if (c[*index] == '\213') AddStr("\306")
	else if (c[*index] == '\214') AddStr("\206")
	else if (c[*index] == '\215') AddStr("\207")
	else if (c[*index] == '\216') AddStr("\202")
	else if (c[*index] == '\217') AddStr("\212")
	else if (c[*index] == '\220') AddStr("\210")
	else if (c[*index] == '\221') AddStr("\211")
	else if (c[*index] == '\222') AddStr("\241")
	else if (c[*index] == '\223') AddStr("\215")
	else if (c[*index] == '\224') AddStr("\214")
	else if (c[*index] == '\225') AddStr("\213")
	else if (c[*index] == '\226') AddStr("\244")
	else if (c[*index] == '\227') AddStr("\242")
	else if (c[*index] == '\230') AddStr("\225")
	else if (c[*index] == '\231') AddStr("\223")
	else if (c[*index] == '\232') AddStr("\224")
	else if (c[*index] == '\233') AddStr("\344")
	else if (c[*index] == '\234') AddStr("\243")
	else if (c[*index] == '\235') AddStr("\227")
	else if (c[*index] == '\236') AddStr("\226")
	else if (c[*index] == '\237') AddStr("\201")
	else if (c[*index] == '\247') AddStr("\341")
	else if (c[*index] == '\256') AddStr("\222")
	else if (c[*index] == '\257') AddStr("\235")
	else if (c[*index] == '\276') AddStr("\221")
	else if (c[*index] == '\277') AddStr("\233")
	else if (c[*index] == '\313') AddStr("\267")
	else if (c[*index] == '\314') AddStr("\307")
	else if (c[*index] == '\315') AddStr("\345")
	else if (c[*index] == '\316') AddStr("^OE")
	else if (c[*index] == '\317') AddStr("^oe")
	else if (c[*index] == '\330') AddStr("\230")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

long CourierToDos860(FONTARGS) {
#ifdef UNX
	*st = c[*index];
	return(1);
#else
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\207') AddStr("\240")
	else if (c[*index] == '\210') AddStr("\205")
	else if (c[*index] == '\211') AddStr("\203")
	else if (c[*index] == '\213') AddStr("\204")
	else if (c[*index] == '\215') AddStr("\207")
	else if (c[*index] == '\216') AddStr("\202")
	else if (c[*index] == '\217') AddStr("\212")
	else if (c[*index] == '\220') AddStr("\210")
	else if (c[*index] == '\222') AddStr("\241")
	else if (c[*index] == '\223') AddStr("\215")
	else if (c[*index] == '\226') AddStr("\244")
	else if (c[*index] == '\227') AddStr("\242")
	else if (c[*index] == '\230') AddStr("\225")
	else if (c[*index] == '\231') AddStr("\223")
	else if (c[*index] == '\233') AddStr("\224")
	else if (c[*index] == '\247') AddStr("\341")
	else if (c[*index] == '\237') AddStr("\201")
	else if (c[*index] == '\234') AddStr("\243")
	else if (c[*index] == '\235') AddStr("\227")
	else if (c[*index] == '\313') AddStr("\221")
	else if (c[*index] == '\314') AddStr("\216")
	else if (c[*index] == '\347') AddStr("\206")
	else if (c[*index] == '\345') AddStr("\217")
	else if (c[*index] == '\202') AddStr("\200")
	else if (c[*index] == '\203') AddStr("\220")
	else if (c[*index] == '\346') AddStr("\211")
	else if (c[*index] == '\351') AddStr("\222")
	else if (c[*index] == '\352') AddStr("\213")
	else if (c[*index] == '\355') AddStr("\230")
	else if (c[*index] == '\204') AddStr("\245")
	else if (c[*index] == '\315') AddStr("\231")
	else if (c[*index] == '\356') AddStr("\237")
	else if (c[*index] == '\361') AddStr("\251")
	else if (c[*index] == '\362') AddStr("\226")
	else if (c[*index] == '\364') AddStr("\235")
	else if (c[*index] == '\242') AddStr("\233")
	else if (c[*index] == '\243') AddStr("\234")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
#endif
}

/*
long (FONTARGS) {
	if (c[*index] >= 0 && c[*index] < 127) { *st = c[*index]; return(1); }
	if (cnt != NULL)
		(*cnt)++;
	if (c[*index] == '\200') AddStr("\")
	else if (c[*index] == '\201') AddStr("\")
	else if (c[*index] == '\202') AddStr("\")
	else if (c[*index] == '\203') AddStr("\")
	else if (c[*index] == '\204') AddStr("\")
	else if (c[*index] == '\205') AddStr("\")
	else if (c[*index] == '\206') AddStr("\")
	else if (c[*index] == '\207') AddStr("\")
	else if (c[*index] == '\210') AddStr("\")
	else if (c[*index] == '\211') AddStr("\")
	else if (c[*index] == '\212') AddStr("\")
	else if (c[*index] == '\213') AddStr("\")
	else if (c[*index] == '\214') AddStr("\")
	else if (c[*index] == '\215') AddStr("\")
	else if (c[*index] == '\216') AddStr("\")
	else if (c[*index] == '\217') AddStr("\")
	else if (c[*index] == '\220') AddStr("\")
	else if (c[*index] == '\221') AddStr("\")
	else if (c[*index] == '\222') AddStr("\")
	else if (c[*index] == '\223') AddStr("\")
	else if (c[*index] == '\224') AddStr("\")
	else if (c[*index] == '\225') AddStr("\")
	else if (c[*index] == '\226') AddStr("\")
	else if (c[*index] == '\227') AddStr("\")
	else if (c[*index] == '\230') AddStr("\")
	else if (c[*index] == '\231') AddStr("\")
	else if (c[*index] == '\232') AddStr("\")
	else if (c[*index] == '\233') AddStr("\")
	else if (c[*index] == '\234') AddStr("\")
	else if (c[*index] == '\235') AddStr("\")
	else if (c[*index] == '\236') AddStr("\")
	else if (c[*index] == '\237') AddStr("\")
	else if (c[*index] == '\240') AddStr("\")
	else if (c[*index] == '\241') AddStr("\")
	else if (c[*index] == '\242') AddStr("\")
	else if (c[*index] == '\243') AddStr("\")
	else if (c[*index] == '\244') AddStr("\")
	else if (c[*index] == '\245') AddStr("\")
	else if (c[*index] == '\246') AddStr("\")
	else if (c[*index] == '\247') AddStr("\")
	else if (c[*index] == '\250') AddStr("\")
	else if (c[*index] == '\251') AddStr("\")
	else if (c[*index] == '\252') AddStr("\")
	else if (c[*index] == '\253') AddStr("\")
	else if (c[*index] == '\254') AddStr("\")
	else if (c[*index] == '\255') AddStr("\")
	else if (c[*index] == '\256') AddStr("\")
	else if (c[*index] == '\257') AddStr("\")
	else if (c[*index] == '\260') AddStr("\")
	else if (c[*index] == '\261') AddStr("\")
	else if (c[*index] == '\262') AddStr("\")
	else if (c[*index] == '\263') AddStr("\")
	else if (c[*index] == '\264') AddStr("\")
	else if (c[*index] == '\265') AddStr("\")
	else if (c[*index] == '\266') AddStr("\")
	else if (c[*index] == '\267') AddStr("\")
	else if (c[*index] == '\270') AddStr("\")
	else if (c[*index] == '\271') AddStr("\")
	else if (c[*index] == '\272') AddStr("\")
	else if (c[*index] == '\273') AddStr("\")
	else if (c[*index] == '\274') AddStr("\")
	else if (c[*index] == '\275') AddStr("\")
	else if (c[*index] == '\276') AddStr("\")
	else if (c[*index] == '\277') AddStr("\")
	else if (c[*index] == '\300') AddStr("\")
	else if (c[*index] == '\301') AddStr("\")
	else if (c[*index] == '\302') AddStr("\")
	else if (c[*index] == '\303') AddStr("\")
	else if (c[*index] == '\304') AddStr("\")
	else if (c[*index] == '\305') AddStr("\")
	else if (c[*index] == '\306') AddStr("\")
	else if (c[*index] == '\307') AddStr("\")
	else if (c[*index] == '\310') AddStr("\")
	else if (c[*index] == '\311') AddStr("\")
	else if (c[*index] == '\312') AddStr("\")
	else if (c[*index] == '\313') AddStr("\")
	else if (c[*index] == '\314') AddStr("\")
	else if (c[*index] == '\315') AddStr("\")
	else if (c[*index] == '\316') AddStr("\")
	else if (c[*index] == '\317') AddStr("\")
	else if (c[*index] == '\320') AddStr("\")
	else if (c[*index] == '\321') AddStr("\")
	else if (c[*index] == '\322') AddStr("\")
	else if (c[*index] == '\323') AddStr("\")
	else if (c[*index] == '\324') AddStr("\")
	else if (c[*index] == '\325') AddStr("\")
	else if (c[*index] == '\326') AddStr("\")
	else if (c[*index] == '\327') AddStr("\")
	else if (c[*index] == '\330') AddStr("\")
	else if (c[*index] == '\331') AddStr("\")
	else if (c[*index] == '\332') AddStr("\")
	else if (c[*index] == '\333') AddStr("\")
	else if (c[*index] == '\334') AddStr("\")
	else if (c[*index] == '\335') AddStr("\")
	else if (c[*index] == '\336') AddStr("\")
	else if (c[*index] == '\337') AddStr("\")
	else if (c[*index] == '\340') AddStr("\")
	else if (c[*index] == '\341') AddStr("\")
	else if (c[*index] == '\342') AddStr("\")
	else if (c[*index] == '\343') AddStr("\")
	else if (c[*index] == '\344') AddStr("\")
	else if (c[*index] == '\345') AddStr("\")
	else if (c[*index] == '\346') AddStr("\")
	else if (c[*index] == '\347') AddStr("\")
	else if (c[*index] == '\350') AddStr("\")
	else if (c[*index] == '\351') AddStr("\")
	else if (c[*index] == '\352') AddStr("\")
	else if (c[*index] == '\353') AddStr("\")
	else if (c[*index] == '\354') AddStr("\")
	else if (c[*index] == '\355') AddStr("\")
	else if (c[*index] == '\356') AddStr("\")
	else if (c[*index] == '\357') AddStr("\")
	else if (c[*index] == '\360') AddStr("\")
	else if (c[*index] == '\361') AddStr("\")
	else if (c[*index] == '\362') AddStr("\")
	else if (c[*index] == '\363') AddStr("\")
	else if (c[*index] == '\364') AddStr("\")
	else if (c[*index] == '\365') AddStr("\")
	else if (c[*index] == '\366') AddStr("\")
	else if (c[*index] == '\367') AddStr("\")
	else if (c[*index] == '\370') AddStr("\")
	else if (c[*index] == '\371') AddStr("\")
	else if (c[*index] == '\372') AddStr("\")
	else if (c[*index] == '\373') AddStr("\")
	else if (c[*index] == '\374') AddStr("\")
	else if (c[*index] == '\375') AddStr("\")
	else if (c[*index] == '\376') AddStr("\")
	else if (c[*index] == '\377') AddStr("\")
	else {
		*st = c[*index];
		if (cnt != NULL)
			(*cnt)--;
		return(1);
	}
}
*/
