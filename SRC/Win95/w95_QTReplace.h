// w95_QTReplace.h

#if !defined(AFX_W95_QTREPLACE_H__EB2FA987_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
#define AFX_W95_QTREPLACE_H__EB2FA987_D6AD_11D0_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/* // NO QT
#include "MacWindows.h"
#include <ConditionalMacros.h>
#include <QTML.h>
#include <Movies.h>
#include <scrap.h>
*/

enum {
	/* Mac OS encodings*/
	kTextEncodingMacRoman = 0,
	kTextEncodingMacJapanese = 1,
	kTextEncodingMacChineseTrad = 2,
	kTextEncodingMacKorean = 3,
	kTextEncodingMacArabic = 4,
	kTextEncodingMacHebrew = 5,
	kTextEncodingMacGreek = 6,
	kTextEncodingMacCyrillic = 7,
	kTextEncodingMacDevanagari = 9,
	kTextEncodingMacGurmukhi = 10,
	kTextEncodingMacGujarati = 11,
	kTextEncodingMacOriya = 12,
	kTextEncodingMacBengali = 13,
	kTextEncodingMacTamil = 14,
	kTextEncodingMacTelugu = 15,
	kTextEncodingMacKannada = 16,
	kTextEncodingMacMalayalam = 17,
	kTextEncodingMacSinhalese = 18,
	kTextEncodingMacBurmese = 19,
	kTextEncodingMacKhmer = 20,
	kTextEncodingMacThai = 21,
	kTextEncodingMacLaotian = 22,
	kTextEncodingMacGeorgian = 23,
	kTextEncodingMacArmenian = 24,
	kTextEncodingMacChineseSimp = 25,
	kTextEncodingMacTibetan = 26,
	kTextEncodingMacMongolian = 27,
	kTextEncodingMacEthiopic = 28,
	kTextEncodingMacCentralEurRoman = 29,
	kTextEncodingMacVietnamese = 30,
	kTextEncodingMacExtArabic = 31,   /* The following use script code 0, smRoman*/
	kTextEncodingMacSymbol = 33,
	kTextEncodingMacDingbats = 34,
	kTextEncodingMacTurkish = 35,
	kTextEncodingMacCroatian = 36,
	kTextEncodingMacIcelandic = 37,
	kTextEncodingMacRomanian = 38,
	kTextEncodingMacCeltic = 39,
	kTextEncodingMacGaelic = 40,
	kTextEncodingMacKeyboardGlyphs = 41
};

enum {
	kTextEncodingDOSLatinUS = 0x0400, /* code page 437*/
	kTextEncodingDOSGreek = 0x0405, /* code page 737 (formerly code page 437G)*/
	kTextEncodingDOSBalticRim = 0x0406, /* code page 775*/
	kTextEncodingDOSLatin1 = 0x0410, /* code page 850, "Multilingual"*/
	kTextEncodingDOSGreek1 = 0x0411, /* code page 851*/
	kTextEncodingDOSLatin2 = 0x0412, /* code page 852, Slavic*/
	kTextEncodingDOSCyrillic = 0x0413, /* code page 855, IBM Cyrillic*/
	kTextEncodingDOSTurkish = 0x0414, /* code page 857, IBM Turkish*/
	kTextEncodingDOSPortuguese = 0x0415, /* code page 860*/
	kTextEncodingDOSIcelandic = 0x0416, /* code page 861*/
	kTextEncodingDOSHebrew = 0x0417, /* code page 862*/
	kTextEncodingDOSCanadianFrench = 0x0418, /* code page 863*/
	kTextEncodingDOSArabic = 0x0419, /* code page 864*/
	kTextEncodingDOSNordic = 0x041A, /* code page 865*/
	kTextEncodingDOSRussian = 0x041B, /* code page 866*/
	kTextEncodingDOSGreek2 = 0x041C, /* code page 869, IBM Modern Greek*/
	kTextEncodingDOSThai = 0x041D, /* code page 874, also for Windows*/
	kTextEncodingDOSJapanese = 0x0420, /* code page 932, also for Windows; Shift-JIS with additions*/
	kTextEncodingDOSChineseSimplif = 0x0421, /* code page 936, also for Windows; was EUC-CN, now GBK (EUC-CN extended)*/
	kTextEncodingDOSKorean = 0x0422, /* code page 949, also for Windows; Unified Hangul Code (EUC-KR extended)*/
	kTextEncodingDOSChineseTrad = 0x0423, /* code page 950, also for Windows; Big-5*/
	kTextEncodingWindowsLatin1 = 0x0500, /* code page 1252*/
	kTextEncodingWindowsANSI = 0x0500, /* code page 1252 (alternate name)*/
	kTextEncodingWindowsLatin2 = 0x0501, /* code page 1250, Central Europe*/
	kTextEncodingWindowsCyrillic = 0x0502, /* code page 1251, Slavic Cyrillic*/
	kTextEncodingWindowsGreek = 0x0503, /* code page 1253*/
	kTextEncodingWindowsLatin5 = 0x0504, /* code page 1254, Turkish*/
	kTextEncodingWindowsHebrew = 0x0505, /* code page 1255*/
	kTextEncodingWindowsArabic = 0x0506, /* code page 1256*/
	kTextEncodingWindowsBalticRim = 0x0507, /* code page 1257*/
	kTextEncodingWindowsVietnamese = 0x0508, /* code page 1258*/
	kTextEncodingWindowsKoreanJohab = 0x0510 /* code page 1361, for Windows NT*/
};

enum {
	kULawCompression = 'ulaw', /*µLaw 2:1*/
	kALawCompression = 'alaw', /*aLaw 2:1*/
};

struct _extended80 {
	short exp;
	short man[4];
};
typedef _extended80    extended80;

//typedef SInt32			TimeValue;


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W95_QTREPLACE_H__EB2FA987_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
