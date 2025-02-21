// Clan2View.cpp : implementation of the CClan2View class
//
#include "ced.h"
#include "c_clan.h"
#include "MMedia.h"
#include "mp3.h"
#include <direct.h>
#include "TextUtils.h"

#include "Clan2Doc.h"
#include "Clan2View.h"
#include "CedDlgs.h"
#include "QTDlg.h"
#include "MpegWindow.h"
#include "FStructViewer.h"

#ifndef __MWERKS__
#include "msutil.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL firstFontTest = TRUE;
static char *bArgv[10];
static COLORWORDLIST *cColor = NULL;
extern CWnd *isMouseButtonDn;
extern char isWinSndPlayAborted;
extern char *ced_punctuation;
extern void stopMovieIfPlaying(void);
extern void stopMP3SoundIfPlaying(void);

CFont m_font;
CDC* GlobalDC;
CClan2Doc* GlobalDoc;
LOGFONT m_lfDefFont;
DWORD	walker_pause = 0L;

extern CMpegWindow *MPegDlg;
extern CQTDlg *MovDlg;
/////////////////////////////////////////////////////////////////////////////
// CClan2View
#define DEBUG_CLAN
#ifdef DEBUG_CLAN
extern FILE *dcFP;
#endif

IMPLEMENT_DYNCREATE(CClan2View, CView)

BEGIN_MESSAGE_MAP(CClan2View, CView)
	//{{AFX_MSG_MAP(CClan2View)
	ON_WM_CREATE()
	ON_COMMAND(ID_CHOOSE_FONT, OnChooseFont)
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_CHOOSE_DEFAULT_FONT, OnChooseDefaultFont)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_SELECT_ALL, OnSelectAll)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_EDIT_ENTER_SELECTION, OnEditEnterSelection)
	ON_COMMAND(ID_EDIT_FIND_SAME, OnEditFindSame)
	ON_COMMAND(ID_EDIT_GOTO, OnEditGoto)
	ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
	ON_COMMAND(ID_EDIT_TO_LOWER, OnEditToLower)
	ON_COMMAND(ID_EDIT_TO_UPPER, OnEditToUpper)
	ON_COMMAND(ID_MODE_CHATMODE, OnModeChatmode)
	ON_COMMAND(ID_MODE_CHECKFILE, OnModeCheckfile)
	ON_COMMAND(ID_MODE_CODER, OnModeCoder)
	ON_COMMAND(ID_MODE_LINKSND, OnModeLinksnd)
	ON_COMMAND(ID_MODE_PLAYCONT, OnModePlaycont)
	ON_COMMAND(ID_MODE_SOUNDMODE, OnModeSoundmode)
	ON_COMMAND(ID_MACROS_DEFINE, OnMacrosDefine)
	ON_COMMAND(ID_MACROS_1, OnMacros1)
	ON_COMMAND(ID_MACROS_10, OnMacros10)
	ON_COMMAND(ID_MACROS_2, OnMacros2)
	ON_COMMAND(ID_MACROS_3, OnMacros3)
	ON_COMMAND(ID_MACROS_4, OnMacros4)
	ON_COMMAND(ID_MACROS_5, OnMacros5)
	ON_COMMAND(ID_MACROS_6, OnMacros6)
	ON_COMMAND(ID_MACROS_8, OnMacros8)
	ON_COMMAND(ID_MACROS_9, OnMacros9)
	ON_COMMAND(ID_SPEAKERS_1, OnSpeakers1)
	ON_COMMAND(ID_SPEAKERS_10, OnSpeakers10)
	ON_COMMAND(ID_SPEAKERS_2, OnSpeakers2)
	ON_COMMAND(ID_SPEAKERS_3, OnSpeakers3)
	ON_COMMAND(ID_SPEAKERS_4, OnSpeakers4)
	ON_COMMAND(ID_SPEAKERS_5, OnSpeakers5)
	ON_COMMAND(ID_SPEAKERS_6, OnSpeakers6)
	ON_COMMAND(ID_SPEAKERS_7, OnSpeakers7)
	ON_COMMAND(ID_SPEAKERS_8, OnSpeakers8)
	ON_COMMAND(ID_SPEAKERS_9, OnSpeakers9)
	ON_COMMAND(ID_SPEAKERS_UPDATE, OnSpeakersUpdate)
	ON_COMMAND(ID_MACROS_7, OnMacros7)
	ON_COMMAND(ID_MODE_DISAMBIG, OnModeDisambig)
	ON_COMMAND(ID_MODE_TIER_EXCLUSION, OnModeTierExclusion)
	ON_COMMAND(ID_MODE_PLAYSOUND, OnModePlaysound)
	ON_COMMAND(ID_MODE_SOUNDTOTEXTSYNC, OnModeSoundtotextsync)
	ON_COMMAND(ID_MODE_EXPEND_BULLETS, OnModeExpendBullets)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_UNDERLINE, OnUnderline)
	ON_COMMAND(ID_EDIT_REPLACEFIND, OnEditReplacefind)
	ON_COMMAND(ID_ITALIC, OnItalic)
	ON_COMMAND(ID_MODE_CAMODE, OnModeCamode)
	ON_COMMAND(ID_SET_TAB, OnSetTab)
	ON_COMMAND(ID_CHOOSE_COMMAND_FONT, OnChooseCommandFont)
	ON_COMMAND(ID_MODE_TRANSMEDIA, OnModeTransmedia)
	ON_COMMAND(ID_MODE_SEND_TO_PRAAT, OnModeSendToPraat)
	ON_COMMAND(ID_FILE_SAVE_CLIP, OnFileSaveClip)
	ON_WM_KILLFOCUS()
	ON_COMMAND(ID_LINENUMBER_SIZE, OnLinenumberSize)
	ON_COMMAND(ID_SOUND_ANALIZER_APP, OnSoundAnalizerApp)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_PLAYBACK_CONTROL, OnPlaybackControl)
	ON_COMMAND(ID_MODE_UTF8, OnModeUtf8)
	ON_COMMAND(ID_F5_OPTION, OnF5Option)
	ON_COMMAND(ID_PLAIN, OnPlain)
	ON_COMMAND(ID_BOLD, OnBold)
	ON_COMMAND(ID_COLOR_WORDS, OnColorWords)
	ON_COMMAND(ID_COLOR_KEYWORDS, OnColorKeywords)
	ON_COMMAND(ID_TOGGLE_MOVIE_TEXT, OnToggleMovieText)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Static initialization/termination

static TCHAR BASED_CODE szSettings[] = _T("Settings");
static TCHAR BASED_CODE szTabStops[] = _T("TabStops");
static TCHAR BASED_CODE szFont[] = _T("Font");
static TCHAR BASED_CODE szPrintFont[] = _T("PrintFont");
static TCHAR BASED_CODE szHeight[] = _T("Height");
static TCHAR BASED_CODE szWeight[] = _T("Weight");
static TCHAR BASED_CODE szItalic[] = _T("Italic");
static TCHAR BASED_CODE szUnderline[] = _T("Underline");
static TCHAR BASED_CODE szPitchAndFamily[] = _T("PitchAndFamily");
static TCHAR BASED_CODE szCharSet[] = _T("CharSet");
static TCHAR BASED_CODE szFaceName[] = _T("FaceName");
static TCHAR BASED_CODE szSystem[] = _T("System");
static TCHAR BASED_CODE szWordWrap[] = _T("WordWrap");

static BOOL GetProfileFont(LPCTSTR szSec, LOGFONT* plf)
{
	CWinApp* pApp = AfxGetApp();
	plf->lfHeight = pApp->GetProfileInt(szSec, szHeight, 0);
	if (plf->lfHeight != 0)
	{
		plf->lfWeight = pApp->GetProfileInt(szSec, szWeight, 0);
		plf->lfItalic = (BYTE)pApp->GetProfileInt(szSec, szItalic, 0);
		plf->lfUnderline = (BYTE)pApp->GetProfileInt(szSec, szUnderline, 0);
		plf->lfPitchAndFamily = (BYTE)pApp->GetProfileInt(szSec, szPitchAndFamily, 0);
		plf->lfCharSet = (BYTE)pApp->GetProfileInt(szSec, szCharSet, DEFAULT_CHARSET);
		CString strFont = pApp->GetProfileString(szSec, szFaceName, szSystem);
		lstrcpyn((TCHAR*)plf->lfFaceName, strFont, sizeof plf->lfFaceName);
		plf->lfFaceName[sizeof plf->lfFaceName-1] = 0;
#ifdef _UNICODE
		if (!strcmp(plf->lfFaceName, dFnt.fontName)) {
			dFnt.fontSize = plf->lfHeight;
			WriteCedPreference();
		}
#endif
		return(TRUE);
	} else
		return(FALSE);
}

static void WriteProfileFont(LPCTSTR szSec, const LOGFONT* plf)
{
	CWinApp* pApp = AfxGetApp();

	if (plf->lfHeight != 0)
	{
		pApp->WriteProfileInt(szSec, szHeight, plf->lfHeight);
		pApp->WriteProfileInt(szSec, szWeight, plf->lfWeight);
		pApp->WriteProfileInt(szSec, szItalic, plf->lfItalic);
		pApp->WriteProfileInt(szSec, szUnderline, plf->lfUnderline);
		pApp->WriteProfileInt(szSec, szPitchAndFamily, plf->lfPitchAndFamily);
		pApp->WriteProfileInt(szSec, szCharSet, plf->lfCharSet);
		pApp->WriteProfileString(szSec, szFaceName, (LPCTSTR)plf->lfFaceName);
	}
}

static BOOL isIPAPhon(char *font) {
	char tFont[LF_FACESIZE];
	
	strcpy(tFont, font);
	uS.uppercasestr(tFont, &dFnt, FALSE);
	return(!strcmp(tFont, "IPAPHON"));
}

void SetLogfont(LOGFONT *lfFont, FONTINFO *fontInfo, NewFontInfo *finfo) {
	char *lfName;

	if (fontInfo != NULL)
		lfName = fontInfo->FName;
	else if (finfo != NULL)
		lfName = finfo->fontName;

	memset(lfFont, 0, sizeof(LOGFONT));
	if (isIPAPhon(lfName)) {
		lfFont->lfCharSet = '\002';
		lfFont->lfOutPrecision = '\003';
		lfFont->lfClipPrecision = '\002';
		lfFont->lfQuality = '\001';
		lfFont->lfPitchAndFamily = '\002';
	} else {
		if (fontInfo != NULL)
			lfFont->lfCharSet = (unsigned char)fontInfo->CharSet;
		else if (finfo != NULL)
			lfFont->lfCharSet = (unsigned char)finfo->CharSet;
		lfFont->lfOutPrecision = OUT_TT_PRECIS;
		lfFont->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lfFont->lfQuality = PROOF_QUALITY;
//		lfFont->lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
		lfFont->lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;
	}
	if (fontInfo != NULL) {
		lfFont->lfHeight = fontInfo->FSize;
		convA_strcpy(lfFont->lfFaceName, fontInfo->FName, LF_FACESIZE);
	} else if (finfo != NULL) {
		lfFont->lfHeight = finfo->fontSize;
		convA_strcpy(lfFont->lfFaceName, finfo->fontName, LF_FACESIZE);
	}
	lfFont->lfWeight = FW_NORMAL;
}

void PosAndDispl(void) {
	if (global_df == NULL)
		return;
	global_df->WinChange = FALSE;
	if (CheckLeftCol(global_df->col_win))
		DisplayTextWindow(NULL, 1);
	wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
	if (!global_df->err_message[0] && global_df->SoundWin != NULL) 
		PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
	else
		draw_mid_wm();
	global_df->WinChange = TRUE;
}

void FinishMainLoop(void) {
	extern void PutSoundStats(Size ws);

	if (global_df == NULL)
		return;
    global_df->WinChange = FALSE;
	if (!global_df->err_message[0] && global_df->SoundWin != NULL) 
		PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
	else draw_mid_wm();
/*	if (!global_df->ChatMode) */
		if (CheckLeftCol(global_df->col_win)) 
			DisplayTextWindow(NULL, 1);

    global_df->WinChange = TRUE;
	wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
	wrefresh(global_df->w1);
}

void ControlAllControls(int on, short id) {
	if (id == 501) {
/*
 		DisableItem(SpeakersMenu, 0);
 		DisableItem(MacrosMenu, 0);
 		DisableItem(ModeMenu, 0);
 		EnableItem(EditMenu, 0);
 		DisableItem(EditMenu, 1);
  		EnableItem(EditMenu, 3);
 		EnableItem(EditMenu, 4);
 		EnableItem(EditMenu, 5);
		DisableItem(EditMenu, 6);
 		DisableItem(EditMenu, 8);
 		DisableItem(EditMenu, 9);
 		DisableItem(EditMenu, 10);
 		DisableItem(EditMenu, 11);
 		DisableItem(EditMenu, 12);
 		EnableItem(EditMenu, 14);
 		EnableItem(FontMenu, 0);
 		EnableItem(FileMenu, 0);
 		EnableItem(ClanMenu, 0);
 		EnableItem(WindowMenu, 0);
 		DrawMenuBar();
*/
 		return;
	}
	if (global_df == NULL) {
/*
 		EnableItem(EditMenu, 0);
 		DisableItem(EditMenu, 1);
  		DisableItem(EditMenu, 3);
 		DisableItem(EditMenu, 4);
 		DisableItem(EditMenu, 5);
		DisableItem(EditMenu, 6);
 		DisableItem(EditMenu, 8);
 		DisableItem(EditMenu, 9);
 		DisableItem(EditMenu, 10);
 		DisableItem(EditMenu, 11);
 		DisableItem(EditMenu, 12);
 		EnableItem(EditMenu, 14);
		EnableItem(FontMenu, 0);
 		DisableItem(SpeakersMenu, 0);
 		DisableItem(MacrosMenu, 0);
 		DisableItem(ModeMenu, 0);
 		EnableItem(FileMenu, 0);
 		EnableItem(ClanMenu, 0);
 		if (!NonModal)
 			DisableItem(WindowMenu, 0);
 		else
 			EnableItem(WindowMenu, 0);
 		DrawMenuBar();
*/
 		return;
	}
 	global_df->ScrollBar = (char)on;
 	if (global_df->ScrollBar == '\001') {
/*
		EnableItem(FileMenu, 0);
 		EnableItem(ClanMenu, 0);
		EnableItem(EditMenu, 0);
	 	EnableItem(EditMenu, 1);
 		EnableItem(EditMenu, 3);
 		EnableItem(EditMenu, 4);
 		EnableItem(EditMenu, 5);
 		EnableItem(EditMenu, 6);
 		EnableItem(EditMenu, 8);
 		EnableItem(EditMenu, 9);
 		EnableItem(EditMenu, 10);
 		EnableItem(EditMenu, 11);
 		EnableItem(EditMenu, 12);
	 	EnableItem(EditMenu, 14);
		EnableItem(FontMenu, 0);
 		EnableItem(SpeakersMenu, 0);
 		EnableItem(MacrosMenu, 0);
 		EnableItem(ModeMenu, 0);
 		EnableItem(WindowMenu, 0);
		DrawMenuBar();
*/
 	} else {
/*
 		EnableItem(FileMenu, 0);
 		EnableItem(ClanMenu, 0);
		if (global_df->UndoList && (id == -1 || on == 2)) {
 			EnableItem(EditMenu, 0);
	 		EnableItem(EditMenu, 1);
	  		DisableItem(EditMenu, 3);
	 		DisableItem(EditMenu, 4);
	 		DisableItem(EditMenu, 5);
			DisableItem(EditMenu, 6);
	 		DisableItem(EditMenu, 8);
 			DisableItem(EditMenu, 9);
	 		DisableItem(EditMenu, 10);
 			DisableItem(EditMenu, 11);
	 		DisableItem(EditMenu, 12);
	 		DisableItem(EditMenu, 14);
		} else
			DisableItem(EditMenu, 0);
 		DisableItem(FontMenu, 0);
 		DisableItem(SpeakersMenu, 0);
 		DisableItem(MacrosMenu, 0);
 		DisableItem(ModeMenu, 0);
 		DisableItem(WindowMenu, 0);
*/
 	}
}

static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *font,NEWTEXTMETRICEX *t,int ft,LPARAM finfo) {
	NewFontInfo *fi = (NewFontInfo *)finfo;

	if (strcmp(font->elfLogFont.lfFaceName, fi->fontName) == 0 &&
		(fi->CharSet == DEFAULT_CHARSET || fi->CharSet == (int)font->elfLogFont.lfCharSet)) {
		fi->CharSet = (int)font->elfLogFont.lfCharSet;
		fi->Encod = GetEncode(fi->fontPref, fi->fontName, fi->fontType, fi->CharSet, FALSE);
		return(0);
	}
	return(1);
}

static int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *font,NEWTEXTMETRIC FAR *t,int ft,LPARAM finfo) {
	NewFontInfo *fi = (NewFontInfo *)finfo;

	if (strcmp(font->elfLogFont.lfFaceName, fi->fontName) == 0 &&
		(fi->CharSet == DEFAULT_CHARSET || fi->CharSet == (int)font->elfLogFont.lfCharSet)) {
		fi->CharSet = (int)font->elfLogFont.lfCharSet;
		fi->Encod = GetEncode(fi->fontPref, fi->fontName, fi->fontType, fi->CharSet, FALSE);
		return(0);
	}
	return(1);
}

static void testPresenceOfUnicodeFont(CDC* dc) {
#ifdef _UNICODE
	extern short ArialUnicodeFOND, SecondDefUniFOND;
	CDC* tGlobalDC = GlobalDC;

	LOGFONT lfFont;
	unCH fontNameU[256];
	NewFontInfo finfo;

	if (!firstFontTest)
		return;
	firstFontTest = FALSE;
	strcpy(defUniFontName, "Arial Unicode MS"); // lxs font lxslxs
	defUniFontSize = -13;
	memset(&lfFont, 0, sizeof(LOGFONT));
	lfFont.lfCharSet = (unsigned char)0;
	lfFont.lfOutPrecision = OUT_TT_PRECIS;
	lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfFont.lfQuality = PROOF_QUALITY;
	lfFont.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH; //FF_SWISS | VARIABLE_PITCH;
	lfFont.lfHeight = defUniFontSize;
	convA_strcpy(lfFont.lfFaceName, defUniFontName, LF_FACESIZE);
	lfFont.lfWeight = FW_NORMAL;

	strcpy(finfo.fontName, defUniFontName);
	finfo.fontSize = defUniFontSize;
	finfo.fontId = DEFAULT_ID;
	finfo.fontPref = "Win95:";
	finfo.orgFType = WINArialUC;
	finfo.fontType = WINArialUC;
	finfo.CharSet = 0;

	ArialUnicodeFOND = 1;
	if (EnumFontFamiliesEx(GlobalDC->GetSafeHdc(),&lfFont,(FONTENUMPROC)EnumFontFamExProc,(LPARAM)&finfo,0) != 0) {
		convA_strcpy(fontNameU, defUniFontName, 256);
		if (EnumFontFamilies(GlobalDC->GetSafeHdc(),fontNameU,(FONTENUMPROC)EnumFontFamProc,(LPARAM)&finfo) != 0) {
			ArialUnicodeFOND = 0;
		}
	}

	if (ArialUnicodeFOND == 0) {
		strcpy(defUniFontName, "CAFont"/*UNICODEFONT*/); // lxs font lxslxs
		defUniFontSize = -13;
		memset(&lfFont, 0, sizeof(LOGFONT));
		lfFont.lfCharSet = (unsigned char)0;
		lfFont.lfOutPrecision = OUT_TT_PRECIS;
		lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lfFont.lfQuality = PROOF_QUALITY;
		lfFont.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH; //FF_SWISS | VARIABLE_PITCH;
		lfFont.lfHeight = defUniFontSize;
		convA_strcpy(lfFont.lfFaceName, defUniFontName, LF_FACESIZE);
		lfFont.lfWeight = FW_NORMAL;

		strcpy(finfo.fontName, defUniFontName);
		finfo.fontSize = defUniFontSize;
		finfo.fontId = DEFAULT_ID;
		finfo.fontPref = "Win95:";
		finfo.orgFType = WINCAFont;
		finfo.fontType = WINCAFont;
		finfo.CharSet = 0;
	
		SecondDefUniFOND = 1;
		if (EnumFontFamiliesEx(GlobalDC->GetSafeHdc(),&lfFont,(FONTENUMPROC)EnumFontFamExProc,(LPARAM)&finfo,0) != 0) {
			convA_strcpy(fontNameU, defUniFontName, 256);
			if (EnumFontFamilies(GlobalDC->GetSafeHdc(),fontNameU,(FONTENUMPROC)EnumFontFamProc,(LPARAM)&finfo) != 0) {
				SecondDefUniFOND = 0;
			}
		}
	} else {
		memset(&lfFont, 0, sizeof(LOGFONT));
		lfFont.lfCharSet = (unsigned char)0;
		lfFont.lfOutPrecision = OUT_TT_PRECIS;
		lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lfFont.lfQuality = PROOF_QUALITY;
		lfFont.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH; //FF_SWISS | VARIABLE_PITCH;
		lfFont.lfHeight = -13;
		convA_strcpy(lfFont.lfFaceName, "CAFont"/*UNICODEFONT*/, LF_FACESIZE);
		lfFont.lfWeight = FW_NORMAL;

		strcpy(finfo.fontName, "CAFont"/*UNICODEFONT*/);
		finfo.fontSize = -13;
		finfo.fontId = DEFAULT_ID;
		finfo.fontPref = "Win95:";
		finfo.orgFType = WINCAFont;
		finfo.fontType = WINCAFont;
		finfo.CharSet = 0;
		
		SecondDefUniFOND = 1;
		if (EnumFontFamiliesEx(GlobalDC->GetSafeHdc(),&lfFont,(FONTENUMPROC)EnumFontFamExProc,(LPARAM)&finfo,0) != 0) {
			convA_strcpy(fontNameU, defUniFontName, 256);
			if (EnumFontFamilies(GlobalDC->GetSafeHdc(),fontNameU,(FONTENUMPROC)EnumFontFamProc,(LPARAM)&finfo) != 0) {
				SecondDefUniFOND = 0;
			}
		}
	}
	GlobalDC = tGlobalDC;
	if (ArialUnicodeFOND == 0 && SecondDefUniFOND == 0)
		do_warning("Can't find either \"Arial Unicode MS\" or \"TITUS Cyberbit Basic\" font. Please look on web page \"http://childes.psy.cmu.edu/clan/\" for more information.", 0);
	else {
		SetDefaultUnicodeFinfo(&finfo);

		if (strcmp(m_lfDefFont.lfFaceName, "Arial Unicode MS") && strcmp(m_lfDefFont.lfFaceName, "CAFont"/*UNICODEFONT*/))
			SetLogfont(&m_lfDefFont, NULL, &finfo);
		else if (!strcmp(m_lfDefFont.lfFaceName, "Arial Unicode MS") && defUniFontName[0] != 'A')
			SetLogfont(&m_lfDefFont, NULL, &finfo);
		else if (!strcmp(m_lfDefFont.lfFaceName, "CAFont"/*UNICODEFONT*/) && defUniFontName[0] == 'A')
			SetLogfont(&m_lfDefFont, NULL, &finfo);

		m_font.DeleteObject();
		if (m_font.CreateFontIndirect(&m_lfDefFont) == 0) {
			::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_lfDefFont);
			m_lfDefFont.lfWeight = FW_NORMAL;
			m_font.CreateFontIndirect(&m_lfDefFont);
		}
#ifdef _UNICODE
		convA_strcpy(dFnt.fontName, m_lfDefFont.lfFaceName, 250);
		dFnt.isUTF = 1;
#else
		strcpy(dFnt.fontName, (char *)m_lfDefFont.lfFaceName);
		dFnt.isUTF = 0;
#endif
		dFnt.fontSize = m_lfDefFont.lfHeight;
		dFnt.orgFType = getFontType(dFnt.fontName, TRUE);
		dFnt.fontType = dFnt.orgFType;
		dFnt.CharSet = DEFAULT_CHARSET;
		dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
	}
#endif	
}

void CClan2View::Initialize()
{
	FONTINFO fi;

	firstFontTest = FALSE;
	if (!GetProfileFont(szFont, &m_lfDefFont)) {
#ifdef _UNICODE
		strcpy(fi.FName, "Arial Unicode MS");
		fi.FSize = -13;
		fi.CharSet = 0;
#else
		strcpy(fi.FName, "Courier");
		fi.FSize = -13;
		fi.CharSet = 1;
#endif
		SetLogfont(&m_lfDefFont, &fi, NULL);
		if (m_font.CreateFontIndirect(&m_lfDefFont) != 0) {
			return;
		} else {
			::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_lfDefFont);
			m_lfDefFont.lfWeight = FW_NORMAL;
		}
	}
#ifdef _UNICODE
	convA_strcpy(defUniFontName, m_lfDefFont.lfFaceName, 250);
#else
	strcpy(defUniFontName, m_lfDefFont.lfFaceName);
#endif
	defUniFontSize = m_lfDefFont.lfHeight;
#ifdef _UNICODE
	if (strcmp(m_lfDefFont.lfFaceName, "Arial Unicode MS") && strcmp(m_lfDefFont.lfFaceName, "CAFont"/*UNICODEFONT*/)) {
		NewFontInfo finfo;

		SetDefaultUnicodeFinfo(&finfo);
		SetLogfont(&m_lfDefFont, NULL, &finfo);
		if (m_font.CreateFontIndirect(&m_lfDefFont) != 0) {
			goto FinInit;
		} else {
			::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_lfDefFont);
			m_lfDefFont.lfWeight = FW_NORMAL;
		}
	}
#endif	
	m_font.CreateFontIndirect(&m_lfDefFont);
FinInit:
#ifdef _UNICODE
	convA_strcpy(dFnt.fontName, m_lfDefFont.lfFaceName, 250);
	dFnt.isUTF = 1;
#else
	strcpy(dFnt.fontName, (char *)m_lfDefFont.lfFaceName);
	dFnt.isUTF = 0;
#endif
	dFnt.fontSize = m_lfDefFont.lfHeight;
	dFnt.orgFType = getFontType(dFnt.fontName, TRUE);
	dFnt.fontType = dFnt.orgFType;
	dFnt.CharSet = DEFAULT_CHARSET;
	dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
}

void CClan2View::Terminate()
{
	WriteProfileFont(szFont, &m_lfDefFont);
}

/////////////////////////////////////////////////////////////////////////////
// CClan2View construction/destruction

CClan2View::CClan2View()
{
	isExtKeyFound = 0; 	// 1- `, 2- ~, 3- ', 4- ^, 5- @, 6- :, 7- ,, 8- &, 9- /
	CAKeyShortcuts = 0;
	skipOnChar = FALSE;
	whichInput = 0;
	cursorCnt = 0;
}

CClan2View::~CClan2View()
{
}

BOOL CClan2View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

//IsWindowUnicode(HWND hWnd)

	return CView::PreCreateWindow(cs);
}

int CClan2View::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{

	stopSoundIfPlaying();
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CClan2View drawing

static webMFile *FindMediaFileName(unCH *line, int *oIndex, char *mvFName, char *path, unCH *URL, webMFile *webMFiles) {
	int i, orgIndex, index;
	char *s, isPict;
	char buf[FILENAME_MAX];
	unCH fname[_MAX_PATH+_MAX_FNAME];
	webMFile *p;
	extern bool curlURLDownloadToFile(unCH *fulURLPath, unCH *fname);

	index = *oIndex;
	orgIndex = index;
	if (line[index] != HIDEN_C) {
		*oIndex = orgIndex;
		return(webMFiles);
	}

	
	for (i=0, index++; line[index]; i++, index++) {
		if (line[index] != SOUNDTIER[i])
			break;
	}
	isPict = FALSE;
	if (SOUNDTIER[i] != EOS) {
		for (i=0, index=orgIndex+1; line[index]; i++, index++) {
			if (line[index] != REMOVEMOVIETAG[i])
				break;
		}
		if (REMOVEMOVIETAG[i] != EOS) {
			for (i=0, index=orgIndex+1; line[index]; i++, index++) {
				if (line[index] != PICTTIER[i])
					break;
			}
			if (PICTTIER[i] != EOS) {
				*oIndex = orgIndex;
				return(webMFiles);
			} else
				isPict = TRUE;
		}
	}
	for (; line[index] && (isSpace(line[index]) || line[index] == '_'); index++) ;
	if (line[index] != '"') {
		*oIndex = orgIndex;
		return(webMFiles);
	}
	index++;
	if (line[index] == EOS) {
		*oIndex = orgIndex;
		return(webMFiles);
	}
	for (i=0; line[index] && line[index] != '"'; index++)
		buf[i++] = line[index];
	buf[i] = EOS;

	if (uS.mStricmp(buf, mvFName) != 0) {
		i = strlen(URL);
		strcat(URL, "media/");
		s = strrchr(buf, '.');
		strcat(URL, buf);
		if (s == NULL) {
			if (isPict)
				strcat(URL, ".jpg");
			else
				strcat(URL, ".mov");
		}
		strcpy(fname, path);
		strcat(fname, buf);
		if (s == NULL) {
			if (isPict)
				strcat(fname, ".jpg");
			else
				strcat(fname, ".mov");
		}

		if (curlURLDownloadToFile(URL, fname) != true)
//		if (URLDownloadToFile(NULL, URL, fname, 0, NULL) != S_OK)
			do_warning("Error downloading data from the web (1)", 0);
		else {
			if (webMFiles == NULL) {
				webMFiles = NEW(webMFile);
				p = webMFiles;
			} else {
				for (p=webMFiles; p->nextFile != NULL; p=p->nextFile) ;
				p->nextFile = NEW(webMFile);
				p = p->nextFile;
			}
			if (p != NULL) {
				strcpy(p->name, buf);
				if (s == NULL) {
					if (isPict)
						strcat(p->name, ".jpg");
					else
						strcat(p->name, ".mov");
				}
				p->nextFile = NULL;
			}
		}
		URL[i] = EOS;
		strcpy(mvFName, buf);
	}
	*oIndex = index;
	return(webMFiles);
}

static webMFile *getMediaFiles(char *path, unCH *URL, myFInfo *df, webMFile *webMFiles) {
	int  i;
	char mvFName[_MAX_PATH+_MAX_FNAME+FILENAME_MAX];
	ROWS *curRow;
	
	mvFName[0] = EOS;
	curRow = df->head_text->next_row;
	while (!AtBotEnd(curRow, df->tail_text, FALSE)) {
		for (i=0; curRow->line[i]; i++) {
			webMFiles = FindMediaFileName(curRow->line, &i, mvFName, path, URL, webMFiles);
		}
		curRow = ToNextRow(curRow, FALSE);
	}
	return(webMFiles);
}

static char *getBatchArgs(char *com) {
	register int i;
	register char *endCom;
	extern char *NextArg(char *s);

	for (i=0; i < 10; i++)
		bArgv[i] = NULL;

	i = 0;
	while (*com != EOS) {
		for (; *com == ' ' || *com == '\t'; com++) ;
		if (*com == EOS)
			break;
		endCom = NextArg(com);
		if (endCom == NULL)
			return(NULL);

		if (i >= 10) {
			do_warning("out of memory; Too many arguments.", 0);
			return(NULL);
		}
		bArgv[i++] = com;
		com = endCom;
	}
	return(com);
}

static char fixArgs(char *fname, char *com) {
	register int  i;
	register int  num;
	register char qt = 0;

	for (i=0; com[i] != EOS; i++) {
		if (com[i] == '\'' /*'*/ || com[i] == '"') {
			if (qt == 0)
				qt = com[i];
			else if (qt == com[i])
				qt = 0;
		} else if (qt == 0 && com[i] == '%' && isdigit(com[i+1]) && !isdigit(com[i+2])) {
			num = atoi(com+i+1) - 1;
			if (num < 0 || num > 9 || bArgv[num] == NULL) {
				sprintf(com, "Argument %%%d was not specified with batch file \"%s\".", 
																	num+1, fname);
				do_warning(com, 0);
				return(FALSE);
			}
			strcpy(com+i, com+i+2);
			uS.shiftright(com+i,(int)strlen(bArgv[num]));
			strncpy(com+i,bArgv[num],strlen(bArgv[num]));
			i = i + strlen(bArgv[num]) - 1;
		}
	}
	return(TRUE);
}

void CClan2View::OnToggleMovieText() 
{
	char MovieFileName[_MAX_PATH+FILENAME_MAX];
	char *ext;
	myFInfo	*tGlobal_df;
	CClan2Doc* pDoc = GetDocument();

	if (pDoc->FileInfo != NULL) {
		tGlobal_df = global_df;
		global_df = pDoc->FileInfo;
		strcpy(MovieFileName, global_df->MvTr.rMovieFile);
		uS.lowercasestr(MovieFileName, &dFnt, FALSE);
		ext = strrchr(MovieFileName,'.');
		if (ext == NULL) {
			strcat(MovieFileName, ".mov");
			ext = strrchr(MovieFileName,'.');
		}
		if (ext != NULL && (strcmp(ext, ".dat") == 0)) {
			if (MPegDlg != NULL){
				MPegDlg->SetActiveWindow();
				pDoc->SetFakeHilight = 1;
				pDoc->myUpdateAllViews(FALSE);
			}
		} else if (ext != NULL && true /*(strcmp(ext, ".mpg") == 0  ||
								   strcmp(ext, ".mpeg") == 0 ||
								   strcmp(ext, ".aiff") == 0 ||
								   strcmp(ext, ".aif") == 0  ||
								   strcmp(ext, ".avi") == 0  ||
								   strcmp(ext, ".mov") == 0  ||
								   strcmp(ext, ".mp4") == 0)*/) {
			if (MovDlg != NULL){			
				MovDlg->SetActiveWindow();
				pDoc->SetFakeHilight = 1;
				pDoc->myUpdateAllViews(FALSE);
			}
		}
		global_df = tGlobal_df;
	}
}

static void finishWalk(myFInfo *df) {
	TimeValue pos=0;

	if (df != NULL) {
		pos = (MovDlg->qt)->GetCurTime();
		if (pos >= df->MvTr.MEnd)
			df->MvTr.MBeg = df->MvTr.MEnd - 1;
		else
			df->MvTr.MBeg = pos;
		strcpy(global_df->err_message, DASHES);
		draw_mid_wm();
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
	}
	PBC.walk = 0;
	PBC.isPC = 0;
}

void CClan2View::OnDraw(CDC* pDC)
{
	int	index;
	char newlyCreated;
	char *fNamePath;
	CRect cRect;
	CDC* tGlobalDC;
	myFInfo	*tGlobal_df;
	CClan2Doc* pDoc = GetDocument();
	extern char DefClan;
	extern char firstTimeStart;
	extern char *clanBuf, clanRun, quickTranscribe;
	extern char *lineNumFname;
	extern char DOSdataErr[];
	extern long lineNumOverride;
	extern short DOSdata;
	extern unCH *getWebMedia;

	ASSERT_VALID(pDoc);
	GlobalDoc = pDoc;
	tGlobalDC = GlobalDC;
	tGlobal_df = global_df;
	GlobalDC = pDC;
	newlyCreated = FALSE;
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "1 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
	testPresenceOfUnicodeFont(pDC);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "2 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
	if (pDoc->re_wrap && pDoc->FileInfo != NULL) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "2 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		global_df = pDoc->FileInfo;
		Re_WrapLines(AddLineToRow, 0L, TRUE, NULL);
		pDoc->re_wrap = FALSE;
	}
	if (pDoc->redo_soundwave && pDoc->FileInfo != NULL) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "3 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		global_df = pDoc->FileInfo;
		if (global_df->SnTr.IsSoundOn) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "4 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			cChan = -1;
			soundwindow(0);
			global_df->SnTr.IsSoundOn = (char)(soundwindow(1) == 0);
			if (global_df->SnTr.IsSoundOn)
				cChan = global_df->SnTr.SNDchan;
		}
		pDoc->redo_soundwave = FALSE;
	}
	
	if (pDoc->InsertSpChar && pDoc->FileInfo != NULL) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "5 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		global_df = pDoc->FileInfo;
		call_win95_call(pDoc->InsertSpChar, 0, 0, 0);
		pDoc->InsertSpChar = 0;
	}

	if (pDoc->RedisplayWindow && pDoc->FileInfo != NULL) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "6 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		global_df = pDoc->FileInfo;
		DrawCursor(0);
		global_df->total_num_rows = 1;
		global_df->CodeWinStart = 0;
		init_windows(true, 1, true);
		RefreshAllTextWindows();
		DrawCursor(1);
		pDoc->RedisplayWindow = FALSE;
	}

	if (pDoc->docFName[0] != EOS) {
		char isUTF8Header;

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "7 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		myMacDirRef dref;
		if (AfxComparePath(pDoc->GetPathName(), cl_T("")) && 
				isalpha(pDoc->docFName[0]) && 
				pDoc->docFName[1] == ':' && pDoc->docFName[2] == '\\')
			pDoc->SetPathName(cl_T(pDoc->docFName), FALSE);
		fNamePath = strrchr(pDoc->docFName, '\\');
		if (fNamePath != NULL)
			index = (int)(fNamePath - pDoc->docFName);
		else
			index = 0;
		newlyCreated = TRUE;
		dref.parID = index;
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "8 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		pDoc->FileInfo = InitFileInfo(pDoc->docFName, &dref, pDoc->m_RowLimit, NULL, 1962, &isUTF8Header);
		pDoc->docFName[0] = EOS;
		global_df = pDoc->FileInfo;
		if (pDoc->FileInfo == NULL) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "9 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			GlobalDC = tGlobalDC;
			global_df = tGlobal_df;
			return;
		}
		if (pDoc->FileInfo->DataChanged)
			pDoc->SetModifiedFlag(TRUE);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "10 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		if (!init_windows(true, 1, false)) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "11 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
	    	FreeUpText(pDoc->FileInfo);
	    	free(pDoc->FileInfo);
	    	pDoc->FileInfo = NULL;
	    	GlobalDC = tGlobalDC;
	    	global_df = tGlobal_df;
	    	return;
		}
		if (!global_df->EditorMode && global_df->CurCode != global_df->RootCodes) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "12 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			if (global_df->ScrollBar != '\002')
				ControlAllControls(2, 0);
		} else {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "13 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			if (global_df->ScrollBar != '\001')
				ControlAllControls(1, 0);
		}
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "14 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		SetRdWKeyScript();
		if (lineNumOverride < 0L || strcmp(lineNumFname, global_df->fileName)) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "15 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			DestroyCaret();
			DrawCursor(3);
		}
		SCROLLINFO sb;
		sb.cbSize = sizeof(SCROLLINFO);
		sb.fMask = SIF_DISABLENOSCROLL | SIF_RANGE;
		sb.nMin = 0;
		sb.nMax = 0;
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "16 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		GlobalDC->GetWindow()->ShowScrollBar(SB_VERT, TRUE);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "17 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		GlobalDC->GetWindow()->SetScrollInfo(SB_VERT, &sb, TRUE);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "18 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		GlobalDC->GetWindow()->ShowScrollBar(SB_HORZ, TRUE);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "19 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		GlobalDC->GetWindow()->SetScrollInfo(SB_HORZ, &sb, TRUE);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "20 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		SetScrollControl();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "21 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		if (DOSdata != 0) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "22 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			if (DOSdata == -1) {
/*21-11-02
				if (global_df->ChatMode)
					sprintf(ced_lineC, "Missing font header. Clan doesn't know which platform file \"%s\" came from.", global_df->fileName);	
				else
*/
					*ced_lineC = EOS;
			} else if (DOSdata == 1)
				sprintf(ced_lineC, "File \"%s\" is in DOS format", global_df->fileName);	
			else if (DOSdata == 2)
*ced_lineC = EOS;//				sprintf(ced_lineC, "File \"%s\" is in Macintosh format", global_df->fileName);	
			else if (DOSdata == 3)
				sprintf(ced_lineC, "File \"%s\" is in Windows 95 format", global_df->fileName);	
			else if (DOSdata == 4)
				sprintf(ced_lineC, "File \"%s\" is in UNICODE format", global_df->fileName);	
			else if (DOSdata == 5)
*ced_lineC = EOS;//				sprintf(ced_lineC, "File \"%s\" has Macintosh style line returns", global_df->fileName);
			else if (DOSdata == 6)
				strcpy(ced_lineC, DOSdataErr);
			else if (DOSdata == 200)
				strcpy(ced_lineC, DOSdataErr);

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "23 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			if (*ced_lineC != EOS)
				do_warning(ced_lineC, 0);	
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "24 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		}	
		DOSdata = -1;
		if (firstTimeStart) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "25 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			firstTimeStart = FALSE;	
			if (AfxComparePath(pDoc->GetPathName(), cl_T("")) && DefClan && clanDlg == NULL) {
				DrawCursor(0);
				DrawFakeHilight(0);
				global_df->DrawCur = FALSE;
				clanDlg = new CClanWindow(AfxGetApp()->m_pMainWnd);
			}
		}
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "26 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		SetPBCglobal_df(false, 0L);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "27 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
	} else if (pDoc->SaveMovieInfo) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "28 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif

		pDoc->SaveMovieInfo = FALSE;
		global_df = pDoc->FileInfo;
		if (global_df && !PlayingContMovie) {
			strcpy(global_df->err_message, DASHES);
			DrawCursor(0);
			DrawFakeHilight(0);
			addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, global_df->MvTr.MEnd, 0);
			PosAndDispl();
			DrawCursor(1);
			DrawFakeHilight(1);
			SetScrollControl();
			if (global_df->DataChanged)
				pDoc->SetModifiedFlag(TRUE);
		}
	} else if (pDoc->SetFakeHilight == 1 && (MovDlg != NULL || MPegDlg != NULL)) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "29 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif

		pDoc->SetFakeHilight = 0;
		global_df = pDoc->FileInfo;
		if (global_df && !PlayingContMovie) {
			DrawFakeHilight(0);
			FakeSelectWholeTier(F5Option == EVERY_LINE);
			DrawFakeHilight(1);
		}
	} else if (pDoc->ExecuteCommNUM) {
		int num = pDoc->ExecuteCommNUM;
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "30 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif

		isPlayS = pDoc->ExecuteCommNUM;
		pDoc->ExecuteCommNUM = 0;
		global_df = pDoc->FileInfo;
		if (global_df) {
			DrawFakeHilight(0);
//			call_win95_call(0, 0, 0, 0);
			addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, global_df->MvTr.MEnd, 0);
			if (num != 89 && num != 91 && num != 96) {
				FakeSelectWholeTier(F5Option == EVERY_LINE);
				DrawFakeHilight(1);
			}
			if (global_df->DataChanged)
				pDoc->SetModifiedFlag(TRUE);
		} else
			isPlayS = 0;
	} else if (pDoc->ChangeCursorInfo != 0L) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "31 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		global_df = pDoc->FileInfo;
		if (global_df) {
  			if (PlayingContMovie == '\003' || pDoc->isTranscribing) {
				long CurFP;
				char res;

//				DrawCursor(2);
//				ResetSelectFlag(0);
				DrawFakeHilight(0);
				CurFP = pDoc->ChangeCursorInfo;
				if (global_df->MvTr.MBeg != CurFP && CurFP != 0L) {
					global_df->row_win2 = 0L;
					global_df->col_win2 = -2L;
					global_df->col_chr2 = -2L;
					if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
						findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
					SaveUndoState(FALSE);
					addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, CurFP, 0);
					global_df->MvTr.MBeg = CurFP;
					if (global_df->DataChanged)
						pDoc->SetModifiedFlag(TRUE);
				}
				selectNextSpeaker();
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
				wrefresh(global_df->w1);
//				DrawCursor(1);
				DrawFakeHilight(1);
				if (PlayingContMovie != '\003')
					pDoc->isTranscribing = false;
			} else if (PlayingContMovie) {
				DrawCursor(0);
				DrawFakeHilight(0);
				SaveUndoState(FALSE);
				move_cursor(pDoc->ChangeCursorInfo, global_df->MvTr.MovieFile, TRUE, FALSE);
				PosAndDispl();
				DrawCursor(1);
				DrawFakeHilight(1);
				PlayingContMovie = '\002';
			}
		}
		pDoc->ChangeCursorInfo = 0L;
	} else if (pDoc->RestorePlayMovieInfo == TRUE) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "32 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		pDoc->RestorePlayMovieInfo = FALSE;
		global_df = pDoc->FileInfo;
		if (global_df) {
	    	WINDOW *t;
			DrawCursor(0);
			DrawFakeHilight(0);
			global_df->WinChange = FALSE;
			DisplayTextWindow(NULL, 0);
			touchwin(global_df->w1);
			wmove(global_df->w1, global_df->row_win, global_df->col_win - global_df->LeftCol);
			wrefresh(global_df->w1);
			if (global_df->wm != NULL) {
				touchwin(global_df->wm);
				touchwin(global_df->w2);
				draw_mid_wm();
				wrefresh(global_df->w2);
				for (t=global_df->RootWindow; t != NULL; t=t->NextWindow) {
					if (t != global_df->w1 && t != global_df->w2 && t != global_df->wm) {
						touchwin(t);
						wrefresh(t);
					}
				}
			}
			global_df->WinChange = TRUE;
			DrawFakeHilight(1);
			if (global_df->cpMvTr != NULL) {
				CopyAndFreeMovieData(FALSE);
				PlayingContMovie = '\001';
				PlayMovie(&global_df->MvTr, global_df, FALSE);
			}
		}
	} else if (pDoc->DoWalkerController == TRUE) {
		long tLong;
		extern long sEF;

		pDoc->DoWalkerController = FALSE;
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "33 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		global_df = pDoc->FileInfo;
		if (global_df && PBC.enable && PBC.isPC && MovDlg && MovDlg->qt) {
			if (PBC.enable && PBC.isPC && PBC.LoopCnt > MovDlg->MoviePlayLoopCnt) {
				MovDlg->MoviePlayLoopCnt++;
				MovDlg->qt->Movie_PreLoad(MovDlg->m_from);
				MovDlg->m_pos = MovDlg->m_from;
				MovDlg->Play();
			} else if (global_df != NULL && PBC.walk && PBC.isPC && PBC.backspace != PBC.step_length) {
				MovDlg->MoviePlayLoopCnt = 1;
				if (PBC.backspace > PBC.step_length) {
					tLong = global_df->MvTr.MBeg - (PBC.backspace - PBC.step_length);
					if (tLong < 0L) {
						finishWalk(global_df);
						goto fin;
					}
					global_df->MvTr.MBeg = tLong;

					if (PBC.walk == 2 && global_df->MvTr.MBeg >= sEF) {
						finishWalk(global_df);
						goto fin;
					}
					if (global_df->MvTr.MBeg >= (MovDlg->qt)->GetQTMovieDuration()) {
						finishWalk(global_df);
						goto fin;
					}

					global_df->MvTr.MEnd = global_df->MvTr.MEnd - (PBC.backspace - PBC.step_length);
					if (PBC.walk == 2 && global_df->MvTr.MEnd > sEF)
						global_df->MvTr.MEnd = sEF;
				} else {
					tLong = global_df->MvTr.MBeg + (PBC.step_length - PBC.backspace);
					if (tLong < 0L) {
						finishWalk(global_df);
						goto fin;
					}
					global_df->MvTr.MBeg = tLong;

					if (PBC.walk == 2 && global_df->MvTr.MBeg >= sEF) {
						finishWalk(global_df);
						goto fin;
					}
					if (global_df->MvTr.MBeg >= (MovDlg->qt)->GetQTMovieDuration()) {
						finishWalk(global_df);
						goto fin;
					}

					global_df->MvTr.MEnd = global_df->MvTr.MEnd + (PBC.step_length - PBC.backspace);
					if (PBC.walk == 2 && global_df->MvTr.MEnd > sEF)
						global_df->MvTr.MEnd = sEF;
				}
				MovDlg->qt->Movie_PreLoad(global_df->MvTr.MBeg);
				MovDlg->m_from=global_df->MvTr.MBeg;
				MovDlg->m_pos = MovDlg->m_from;
				MovDlg->m_to=global_df->MvTr.MEnd;
				MovDlg->Play();
fin:
				pDoc->DoWalkerController = FALSE;
			} else {
				finishWalk(global_df);
			}
		}
	} else if (pDoc->isAddFStructComm[0] != EOS) {
skipSyn:
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "34 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		global_df = pDoc->FileInfo;
		if (global_df) {
	    	WINDOW *t;
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
			DrawCursor(0);
			DrawFakeHilight(0);
			if (global_df->row_txt->line[0] == '*' || global_df->row_txt->line[0] == '@') {
				MoveUp(-1);
				EndOfLine(-1);
			} else {
//				while (!AtBotEnd(tr, global_df->tail_text, FALSE))
			}
			if (pDoc->isAddFStructComm[0] != '\001') {
				AddText(cl_T(pDoc->isAddFStructComm), '\0',0, (long)strlen(pDoc->isAddFStructComm));
				if (SaveToFile(global_df->fileName)) {
					sprintf(global_df->err_message, "-File \"%s\" written.", global_df->fileName); 
					global_df->DataChanged = '\0';
					ResetUndos();
					pDoc->SetModifiedFlag(FALSE);
				}
			}
			while (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
				if (uS.partcmp(global_df->row_txt->line,"%syn:",FALSE,FALSE))
					break;
				MoveDown(-1);
			}
			
			global_df->WinChange = FALSE;
			DisplayTextWindow(NULL, 0);
			touchwin(global_df->w1);
			wmove(global_df->w1, global_df->row_win, global_df->col_win - global_df->LeftCol);
			wrefresh(global_df->w1);
			if (global_df->wm != NULL) {
				touchwin(global_df->wm);
				touchwin(global_df->w2);
				draw_mid_wm();
				wrefresh(global_df->w2);
				for (t=global_df->RootWindow; t != NULL; t=t->NextWindow) {
					if (t != global_df->w1 && t != global_df->w2 && t != global_df->wm) {
						touchwin(t);
						wrefresh(t);
					}
				}
			}
			global_df->WinChange = TRUE;
			if (global_df->DataChanged)
				pDoc->SetModifiedFlag(TRUE);
			DrawCursor(1);
			DrawFakeHilight(1);
			if (uS.partcmp(global_df->row_txt->line,"%syn:",FALSE,FALSE)) {
				if (!ShowFStruct())
					goto skipSyn;
				pDoc->isAddFStructComm[0] = EOS;
			} else {
				pDoc->isAddFStructComm[0] = EOS;
				if (FViewerDlg != NULL)
					FViewerDlg->OnCancel();
				do_warning("No more \"%syn:\" tiers found.", 0);
			}
		}
	} else if (pDoc->FileInfo != NULL) {
		global_df = pDoc->FileInfo;
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "35 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		if (pDoc->isNeedResize) {
			long t;

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "35.1 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			pDoc->isNeedResize = FALSE;
			DrawCursor(0);
			DrawFakeHilight(0);
			t = global_df->row_win;
			global_df->row_win = 0L;
			if (!init_windows(true, 1, false))
				mem_err(TRUE, global_df);
			global_df->row_win = t;
			wmove(global_df->w1, global_df->row_win, global_df->col_win - global_df->LeftCol);
			if (global_df->row_win < 0L || global_df->row_win >= (long)global_df->EdWinSize) 
				global_df->w1->cur_row = -1;
			SetRdWKeyScript();
			DrawSoundCursor(0);
			RefreshOtherCursesWindows(FALSE);
			SetScrollControl();
			DrawFakeHilight(1);
			DrawCursor(1);
			DrawSoundCursor(1);
		} else {
			char tc, ts;
	    	WINDOW *t;

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "35.2 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			if (GetActiveWindow( ) == MovDlg || 
				GetActiveWindow( ) == MPegDlg) {
				DrawFakeHilight(0);
			}
			global_df->WinChange = FALSE;
			DisplayTextWindow(NULL, 0);
			touchwin(global_df->w1);
			wrefresh(global_df->w1);
			if (global_df->wm != NULL) {
				touchwin(global_df->wm);
				touchwin(global_df->w2);
				draw_mid_wm();
				wrefresh(global_df->w2);
			    for (t=global_df->RootWindow; t != NULL; t=t->NextWindow) {
					if (t != global_df->w1 && t != global_df->w2 && t != global_df->wm) {
						touchwin(t);
						wrefresh(t);
					}
				}
			}
			global_df->WinChange = TRUE;
			SetRdWKeyScript();
			tc = global_df->TSelectFlag;
			ts = global_df->SSelectFlag;
			DrawCursor(2);
			DrawSoundCursor(2);
			global_df->TSelectFlag = tc;
			global_df->SSelectFlag = ts;
			if (GetActiveWindow( ) == MovDlg || 
				GetActiveWindow( ) == MPegDlg) {
				DrawFakeHilight(1);
			}
		}
		if (pDoc->SetFakeHilight == 2 && (MovDlg != NULL || MPegDlg != NULL)) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "35.3 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
			pDoc->SetFakeHilight = 0;
			if (global_df && !PlayingContMovie) {
				DrawFakeHilight(0);
				FakeSelectWholeTier(F5Option == EVERY_LINE);
				DrawFakeHilight(1);
			}
		}
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "35.4 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
	}

	if (getWebMedia != NULL) {
		char *s;

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "36 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		strcpy(ced_lineC, global_df->fileName);
		s = strrchr(ced_lineC, '\\');
		if (s != NULL)
			*(s+1) = EOS;
		global_df->webMFiles = getMediaFiles(ced_lineC, getWebMedia, global_df, NULL);
		global_df->isTempFile = TRUE;
		getWebMedia = NULL;
	}

	if (lineNumOverride >= 0L && !strcmp(lineNumFname, global_df->fileName)) {
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "37 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		if (global_df != NULL) {
			strcpy(global_df->err_message, DASHES);
			DrawCursor(0);
			SaveUndoState(FALSE);
			global_df->row_win2 = 0L;
			global_df->col_win2 = 0L;
			global_df->col_chr2 = 0L;
			MoveToLine(lineNumOverride, 1);
			global_df->row_win2 = 1L;
			global_df->LeaveHighliteOn = TRUE;
			FinishMainLoop();
			SetScrollControl();
			SetRdWKeyScript();
			CWnd* win = GlobalDC->GetWindow();
			if (win == NULL || win == win->GetFocus()) {
				DestroyCaret();
				DrawCursor(3);
				DrawSoundCursor(1);
			}
		}
		lineNumOverride = -1L;
		lineNumFname = NULL;
	}

	if (quickTranscribe) {
		char type, res;
		extern char SelectWholeTier(char isCAMode);

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "38 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		type = quickTranscribe;
		quickTranscribe = FALSE;
		if (type == isAudio || type == isVideo) { // sound
			if (type == isAudio) {
				PlayingContSound = '\004';
				global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
				strcpy(global_df->err_message, "-Transcribing, click mouse to stop");
				draw_mid_wm();
				strcpy(global_df->err_message, DASHES);
			} else
				PlayingContMovie = '\003';

			if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
				findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
			if (global_df->row_txt->line[0] == '\t' && (global_df->row_txt->line[1] == EOS || global_df->row_txt->line[1] == NL_C)) { 
				SaveUndoState(FALSE);
				global_df->redisplay = 0;
				DeletePrevChar(1);
				global_df->redisplay = 1;
			}
			if (!isMainSpeaker(*global_df->row_txt->line, global_df->IsCainMode) && AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
				SaveUndoState(FALSE);
				NewLine(-1);
			}
			if (global_df->row_txt->line[0] == EOS || global_df->row_txt->line[0] == NL_C) { 
				SaveUndoState(FALSE);
				if (global_df->IsCainMode)
					AddText(cl_T(":\t"), EOS, 1, strlen(":\t"));
				else
					AddText(cl_T("*:\t"), EOS, 1, strlen("*:\t"));
			}
			SelectWholeTier(F5Option == EVERY_LINE);
			if (type == isAudio) {
				DrawSoundCursor(1);
				DrawCursor(1);
				global_df->SnTr.SSource = 1;
				if (!PlaySound(global_df->SnTr.SoundFile, &global_df->SnTr, (int)'r')) {
					PlayingContSound = FALSE;
					global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
				}
			} else
				PlayMovie(&global_df->MvTr, global_df, FALSE);
		} else if (type == isPict) { // picture
			DisplayPhoto(global_df->PcTr.pictFName, &global_df->PcTr.pictDref);
		} else if (type == isText) { // text
			DisplayText(global_df->TxTr.textFName, &global_df->TxTr.textDref);
		} else {
			strcpy(global_df->err_message, "+Unsupported media type.");
		}
	}

	if (clanRun && !newlyCreated && global_df && !strcmp(global_df->fileName, "CLAN Output")) {
		char  tDataChanged;
		FILE  *fp;
		char nowref[_MAX_PATH+1];
		extern char ClanAutoWrap;

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "39 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
		isKillProgram = 0;
		clanRun = FALSE;
		global_df->AutoWrap = ClanAutoWrap;
		if (isUTFData && !dFnt.isUTF) {
			SetDefaultUnicodeFinfo(&dFnt);
			dFnt.fontTable = NULL;
			dFnt.isUTF = 1;
		}
		dFnt.charHeight = GetFontHeight(NULL, &dFnt);
		dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
		dFnt.orgEncod = dFnt.Encod;
		copyNewFontInfo(&oFnt, &dFnt);
		tDataChanged = global_df->DataChanged;
		EndOfFile(-1);
		SetScrollControl();
		ResetUndos();
		copyNewToFontInfo(&global_df->row_txt->Font, &dFnt);
		global_df->attOutputToScreen = 0;
		if (global_df->wLineno == 1) 
			OutputToScreen(cl_T("> "));
		_getcwd(nowref, _MAX_PATH);
		dFnt.isUTF = isUTFData;

		if (!strncmp(clanBuf, "BAT ", 4) || !strncmp(clanBuf, "bat ", 4) || 
			!strncmp(clanBuf, "BATCH ", 6) || !strncmp(clanBuf, "batch ", 6)) {
			char *com, *eCom;

			SetNewVol(0, wd_st_full);
			uS.remFrontAndBackBlanks(clanBuf+1);
			OutputToScreen(cl_T(clanBuf));

			for (com=clanBuf; *com != EOS && *com != ' ' && *com != '\t'; com++) ;
			for (; *com == ' ' || *com == '\t'; com++) ;
			eCom = com;
			if (*com != EOS) {
				for (; *eCom != EOS && *eCom != ' ' && *eCom != '\t'; eCom++) ;
				if (*eCom != EOS) {
					*eCom = EOS;
					eCom++;
				}
			}

			if ((fp=fopen(com, "r")) == NULL) {
				SetNewVol(0, lib_dir);
				if ((fp=fopen(com, "r")) == NULL) {
					sprintf(ced_lineC, "Can't find batch file \"%s\" in either working or library directories", com);
					do_warning(ced_lineC, 0);
				}
				SetNewVol(0, wd_st_full);
			}

			if (fp != NULL) {
				if ((eCom=getBatchArgs(eCom)) != NULL) {
					eCom++;
					while (fgets(eCom, 1024, fp)) {
						uS.remFrontAndBackBlanks(eCom);
						if (!fixArgs(com, eCom))
							break;
						if (*eCom == EOS ||
							(toupper(eCom[0]) == 'T' && toupper(eCom[1]) == 'Y') ||
							(toupper(eCom[0]) == 'P' && toupper(eCom[1]) == 'A')) ;
						else {
							OutputToScreen(cl_T("\nBATCH> "));
							execute(eCom, tDataChanged);
							if (isKillProgram) {
								if (isKillProgram != 2)
									OutputToScreen(cl_T("\n    BATCH FILE ABORTED\n"));
								break;
							}
						}
					}
				}
				fclose(fp);
			}
		} else
			execute(clanBuf, tDataChanged);

		copyNewFontInfo(&dFnt, &oFnt);
		if (isKillProgram)
			isKillProgram = 0;
		if (global_df != NULL && !strcmp(global_df->fileName, "CLAN Output")) {
			OutputToScreen(cl_T("\n> "));
			global_df->DataChanged = tDataChanged;
			PosAndDispl();
			SetScrollControl();
		}
		SetNewVol(0, nowref);
		clanBuf = NULL;
		CWnd* win = GlobalDC->GetWindow();
		if (win == NULL || win == win->GetFocus())
			DrawCursor(3);
	}
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "40 in void CClan2View::OnDraw(CDC* pDC)\n");
		fflush(dcFP);
	}
#endif
	GlobalDC = tGlobalDC;
	global_df = tGlobal_df;
}

/////////////////////////////////////////////////////////////////////////////
// CClan2View printing

BOOL CClan2View::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void CClan2View::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: add extra initialization before printing
/*
	int hor, ver;
	float t;

	hor = pDC->GetDeviceCaps(HORZRES);
	ver = pDC->GetDeviceCaps(VERTRES);
	if (hor > ver) {
		t = (float)hor;
	} else {
		t = (float)ver;
	}
	t = t / (float)937.000;
	t = t * (float)100.000;
	vPrintScale = (UINT)t;
	if (vPrintScale == 0)
		vPrintScale = 1;
	hPrintScale = vPrintScale;
//
	t = (float)pDC->GetDeviceCaps(HORZRES);
	t = t / 720.00;
	t = t * 100.00;
	hPrintScale = (UINT)t;
	if (hPrintScale == 0)
		hPrintScale = 1;
	t = (float)pDC->GetDeviceCaps(VERTRES);
	t = t / 937.00;
	t = t * 100.00;
	vPrintScale = (UINT)t;
	if (vPrintScale == 0)
		vPrintScale = 1;
*/
	m_lp = 0;
	pInfo->m_lpUserData = NULL;
}

void CClan2View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CClan2View::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CView::OnPrepareDC(pDC, pInfo);
	if (pInfo) {
//		pDC->SetMapMode(MM_ISOTROPIC);
//		pDC->SetMapMode(MM_ANISOTROPIC); 
//		pDC->SetMapMode(MM_LOMETRIC);
//		pDC->SetMapMode(MM_TEXT);
//		ret = pDC->ScaleViewportExt(10, 1, 10, 1);
//		ret = pDC->SetWindowExt(100, 100);
//		ret = pDC->SetViewportExt(hPrintScale, vPrintScale);
		pDC->SetBkMode(TRANSPARENT);
		if (m_lp > 0 && pInfo->m_nCurPage > m_lp)
			pInfo->m_bContinuePrinting = FALSE;
		else
			pInfo->m_bContinuePrinting = TRUE;
	} else {
//		ret = pDC->ScaleViewportExt(1000, 1, 1000, 1);
//		pDC->SetMapMode(MM_ANISOTROPIC); 
//		ret = pDC->SetWindowExt(100, 100);
	}
}

static void convertWinFontToPrinterFont(CDC* pDC, LOGFONT *lf) {
	HDC hDCFrom = ::GetDC(NULL);
	lf->lfHeight = ::MulDiv(lf->lfHeight, pDC->GetDeviceCaps(LOGPIXELSY),
		::GetDeviceCaps(hDCFrom, LOGPIXELSY));
	lf->lfWidth = ::MulDiv(lf->lfWidth, pDC->GetDeviceCaps(LOGPIXELSX),
		::GetDeviceCaps(hDCFrom, LOGPIXELSX));
	::ReleaseDC(NULL, hDCFrom);
}

static void formatHeader(unCH *st, char *fname, CDC* pDC, UINT pn, long right) {
	int		i;
	int		tabCnt = 0;
	time_t	lctime;
	CSize	CW;
	struct tm *now;

	lctime = time(NULL);
	now = localtime(&lctime);
	strftime(templineC, UTTLINELEN, "%d-%b-%y %H:%M", now);
	sprintf(templineC3, "Page %u", pn);

	do {
		tabCnt++;
		if (tabCnt == 300)
			break;
		strcpy(st, templineC);
		for (i=0; i < tabCnt; i++)
			strcat(st, " ");
		strcat(st, fname);
		for (i=0; i < tabCnt; i++)
			strcat(st, " ");
		strcat(st, templineC3);
		CW = pDC->GetTextExtent(st, strlen(st));
	} while (CW.cx < right) ;
	tabCnt--;
	if (tabCnt == 0) {
		strcpy(st, templineC);
		strcat(st, " \"");
		strcat(st, fname);
		strcat(st, "\" ");
		strcat(st, templineC3);
	} else {
		strcpy(st, templineC);
		for (i=0; i < tabCnt; i++)
			strcat(st, " ");
		strcat(st, fname);
		for (i=0; i < tabCnt; i++)
			strcat(st, " ");
		strcat(st, templineC3);
	}
}

#define PAD 1

void CClan2View::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	ROWS	*rt, *t;
	int		leftPos;
	long	print_row, len, i, j;
	long	fsize = -1;
	long	newcol, colWin;
	long	ln;
	char	isFindFirstPage;
	AttTYPE	oldState;
	char	attChanged;
	char	fontName[LF_FACESIZE];
	short	FHeight;
	int     charSet;
	unsigned int pageNum;
	LOGFONT lfFont;
	CFont	l_font;
	CSize	CW;
	SIZE	wCW;
	CDC* 	tGlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	tGlobalDC = GlobalDC;
	GlobalDC = pDC;
	global_df = pDoc->FileInfo;
	GlobalDoc = pDoc;
	isFindFirstPage = FALSE;
	pageNum = 1;
FindFirstPageLoop:
	if (pInfo->m_nCurPage == 1 || pInfo->m_bPreview) { // page no. 1 is the title page
		ChangeCurLineAlways(0);
		rt = global_df->head_text->next_row;
	} else if (pInfo->m_lpUserData == NULL) {
		isFindFirstPage = TRUE;
		rt = global_df->head_text->next_row;
	} else {
		if (pageNum == pInfo->m_nCurPage)
			isFindFirstPage = FALSE;
		rt = (ROWS *)pInfo->m_lpUserData;
	}

	SetLogfont(&lfFont, &rt->Font, NULL);
	convertWinFontToPrinterFont(pDC, &lfFont);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = pDC->SelectObject(&l_font);
	CW = pDC->GetTextExtent(cl_T("IjWfFg"), 6);
	FHeight = CW.cy + FLINESPACE;

	if (pInfo->m_bPreview) {
		len = 1;
		print_row = pInfo->m_rectDraw.top;
		for (; rt != global_df->tail_text; rt=rt->next_row) {
			if (print_row > pInfo->m_rectDraw.bottom) {
				len++;
				print_row = pInfo->m_rectDraw.top;
			}
			if (len == (long)pInfo->m_nCurPage)
				break;
			print_row += FHeight + PAD;
		}
	}
	print_row = pInfo->m_rectDraw.top;
	strcpy(fontName, rt->Font.FName);
	fsize = rt->Font.FSize;
	charSet = rt->Font.CharSet;

	formatHeader(ced_line, global_df->fileName, pDC, pInfo->m_nCurPage, pInfo->m_rectDraw.right);
	if (*templineC3 != EOS) {
		if (!isFindFirstPage)
			pDC->TabbedTextOut(LEFTMARGIN,print_row,ced_line,strlen(ced_line),0,NULL,0);
		print_row += FHeight + PAD;
		pDC->MoveTo(pInfo->m_rectDraw.left, print_row);
		pDC->LineTo(pInfo->m_rectDraw.right, print_row);
		print_row += FHeight + PAD;
	}

	if (isShowLineNums) {
		ln = 1L;
		for (t=global_df->head_text->next_row; t != global_df->tail_text && t != rt; t=t->next_row) {
			if (LineNumberingType == 0 || isMainSpeaker(rt->line[0], global_df->IsCainMode))
				ln++;
		}
	}

	for (; rt != global_df->tail_text; rt=rt->next_row) {
		if (print_row+FHeight > pInfo->m_rectDraw.bottom)
			break;

		if (strcmp(fontName, rt->Font.FName) ||
					fsize != rt->Font.FSize ||
					charSet != rt->Font.CharSet) {
			pDC->SelectObject(pOldFont);
			SetLogfont(&lfFont, &rt->Font, NULL);
			convertWinFontToPrinterFont(pDC, &lfFont);
			l_font.DeleteObject();
			l_font.CreateFontIndirect(&lfFont);
			pDC->SelectObject(&l_font);
			CW = pDC->GetTextExtent(cl_T("IjWfFg"), 6);
			FHeight = CW.cy + FLINESPACE;
			strcpy(fontName, rt->Font.FName);
			fsize = rt->Font.FSize;
			charSet = rt->Font.CharSet;
		}
		for (len=strlen(rt->line)-1; rt->line[len] == NL_C; len--) ;

		len++;
		if (isFindFirstPage) ;
		else if (strlen(rt->line) >= UTTLINELEN)
			pDC->TabbedTextOut(LEFTMARGIN,print_row,rt->line,len,0,NULL,0);
		else {
			j = 0;
			if (isShowLineNums) {
				if (LineNumberingType == 0 || isMainSpeaker(rt->line[0], global_df->IsCainMode)) {
					uS.sprintf(ced_line, "%-5ld ", ln);
					ln++;
				} else
					strcpy(ced_line, "      ");
				for (; j < strlen(ced_line); j++)
					tempAtt[j] = 0;
			}
			leftPos = LEFTMARGIN;
			colWin = 0L;
			for (i=0; i < len; i++) {
				if (rt->line[i] != NL_C) {
					if ((unsigned char)rt->line[i] == '\t') {
						newcol = (((colWin / TabSize) + 1) * TabSize);
						for (; colWin < newcol; colWin++) {
							ced_line[j] = ' ';
							if (rt->att == NULL)
								tempAtt[j] = 0;
							else
								tempAtt[j] = rt->att[i];
							j++;
						}
					} else {
						colWin++;
						if (rt->line[i] == HIDEN_C) {
							if (global_df->ShowParags != '\002') {
								for (i++; rt->line[i] != HIDEN_C && rt->line[i]; i++) ;
								if (rt->line[i] == EOS)
									i--;
							} else {
								ced_line[j] = '\225';
								if (rt->att == NULL)
									tempAtt[j] = 0;
								else
									tempAtt[j] = rt->att[i];
								j++;
							}
						} else {
							ced_line[j] = rt->line[i];
							if (rt->att == NULL)
								tempAtt[j] = 0;
							else
								tempAtt[j] = rt->att[i];
							j++;
						}
					}
				}
			}
			len = j;

			j = 0;
			oldState = tempAtt[0];
			for (i=0; i <= len; i++) {
				if (oldState != tempAtt[i] || i == len) {
					attChanged = FALSE;
					if (is_underline(tempAtt[j]) || is_error(tempAtt[j])) {
						if (!lfFont.lfUnderline) {
							lfFont.lfUnderline = TRUE;
							attChanged = TRUE;
						}
					} else if (lfFont.lfUnderline) {
						lfFont.lfUnderline = FALSE;
						attChanged = TRUE;
					}
					if (is_italic(tempAtt[j])) {
						if (!lfFont.lfItalic) {
							lfFont.lfItalic = TRUE;
							attChanged = TRUE;
						}
					} else if (lfFont.lfItalic) {
						lfFont.lfItalic = FALSE;
						attChanged = TRUE;
					}
					if (is_bold(tempAtt[j])) {
						if (lfFont.lfWeight != FW_BOLD) {
							lfFont.lfWeight = FW_BOLD;
							attChanged = TRUE;
						}
					} else if (lfFont.lfWeight == FW_BOLD) {
						lfFont.lfWeight = FW_NORMAL;
						attChanged = TRUE;
					}
					if (attChanged) {
						pDC->SelectObject(pOldFont);
						l_font.DeleteObject();
						l_font.CreateFontIndirect(&lfFont);
						pDC->SelectObject(&l_font);
					}
 

					if (global_df->isUTF) {
						unsigned short *puText=NULL;
						long totalw=0;
#ifdef _UNICODE
						puText = ced_line+j;
						totalw = i-j;
#else
						LPVOID hwText;
						totalw=MultiByteToWideChar(CP_UTF8,0,ced_line+j,i-j,NULL,0);
						if ((totalw+1) * 2 > TEMPWLEN) {
							hwText = LocalAlloc(LMEM_MOVEABLE, (totalw+1L)*2);
							puText = (unsigned short *)LocalLock(hwText);
						} else
							puText = (unsigned short *)tempW;

						MultiByteToWideChar(CP_UTF8,0,ced_line+j,i-j,puText,totalw);
#endif
						TabbedTextOutW(pDC->m_hDC, leftPos, print_row, puText, totalw,0,NULL,0);

						if (GetTextExtentPointW(pDC->m_hDC,puText,totalw,&wCW) != 0) {
							CW.cx = (short)wCW.cx;
							CW.cy = (short)wCW.cy;
						} else
							CW.cx = 0;
						
#ifndef _UNICODE
						if ((totalw+1) * 2 > TEMPWLEN) {
							LocalUnlock(hwText);
							LocalFree(hwText);
						}
#endif
					} else {
						pDC->TabbedTextOut(leftPos,print_row,ced_line+j,i-j,0,NULL,0);
						CW = pDC->GetTextExtent(ced_line+j, i-j);
					}

					
					
					leftPos += CW.cx;
					j = i;
					oldState = tempAtt[i];
				}
			}
		}
		pDC->SelectObject(pOldFont);
		lfFont.lfUnderline = FALSE;
		lfFont.lfItalic = FALSE;
		lfFont.lfWeight = FW_NORMAL;
		l_font.DeleteObject();
		l_font.CreateFontIndirect(&lfFont);
		pDC->SelectObject(&l_font);
		print_row += FHeight + PAD;
	}
	pDC->SelectObject(pOldFont);
	l_font.DeleteObject();
	if (rt == global_df->tail_text)
		m_lp = pInfo->m_nCurPage;
	else
		pInfo->m_lpUserData = (LPVOID)rt;
	pageNum++;
	if (isFindFirstPage && rt != global_df->tail_text)
		goto FindFirstPageLoop;
	GlobalDC = tGlobalDC;
}
/////////////////////////////////////////////////////////////////////////////
// CClan2View diagnostics

#ifdef _DEBUG
void CClan2View::AssertValid() const
{
	CView::AssertValid();
}

void CClan2View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CClan2Doc* CClan2View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClan2Doc)));
	return (CClan2Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CClan2View message handlers

void CClan2View::OnKillFocus(CWnd* pNewWnd) 
{
	CClientDC dc(this);
	CClan2Doc* pDoc = GetDocument();
	if (!PlayingContSound)
		stopSoundIfPlaying();
	GlobalDoc = pDoc;
	if (pDoc && (global_df=pDoc->FileInfo) != NULL) {
		CDC* tGlobalDC = GlobalDC;
		GlobalDC = &dc;
		DrawCursor(0);
		DrawSoundCursor(0);
		GlobalDC = tGlobalDC;
		global_df->DrawCur = FALSE;
	}
	DestroyCaret();
	CView::OnKillFocus(pNewWnd);
	if (cursorCnt < 0)
		cursorCnt = ShowCursor(TRUE);
}

void CClan2View::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if (cursorCnt < 0)
		cursorCnt = ShowCursor(TRUE);
	if (!bActivate && pActivateView == pDeactiveView) {
		CClientDC dc(pActivateView);
		CClan2Doc* pDoc = GetDocument();
		if (!PlayingContSound)
			stopSoundIfPlaying();
		GlobalDoc = pDoc;
		if (pDoc && (global_df=pDoc->FileInfo) != NULL) {
			CDC* tGlobalDC = GlobalDC;
			GlobalDC = &dc;
			DrawCursor(0);
			DrawSoundCursor(0);
			GlobalDC = tGlobalDC;
			global_df->DrawCur = FALSE;
		}
		DestroyCaret();
	}

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (bActivate && pActivateView == pDeactiveView) {
		CClientDC dc(pActivateView);
		CClan2Doc* pDoc = GetDocument();
		GlobalDoc = pDoc;
		ASSERT_VALID(pDoc);
		CDC* tGlobalDC = GlobalDC;
		GlobalDC = &dc;
		global_df = pDoc->FileInfo;
		if (global_df == NULL) {
			cedDFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
		} else {
			if (global_df->RdW != global_df->w1) {
				cedDFnt.Encod = my_FontToScript(global_df->RdW->RowFInfo[0]->FName, global_df->RdW->RowFInfo[0]->CharSet);
			} else {
				cedDFnt.Encod = my_FontToScript(global_df->row_txt->Font.FName, global_df->row_txt->Font.CharSet);
				wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			}
			cedDFnt.isUTF = global_df->isUTF;
			if (FreqCountLimit > 0 && global_df->FreqCount >= FreqCountLimit) {
				SaveCKPFile(TRUE);
				global_df->FreqCount = 0;
			}
		}
		DrawFakeHilight(0);
		DrawCursor(3);
		DrawSoundCursor(1);
		SetPBCglobal_df(false, 0L);
		GlobalDC = tGlobalDC;
		if (/*PlayingContMovie != '\003' && */PlayingContMovie) {
			if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->isPlaying) {
				StopMovie((MovDlg->qt)->theMovie);
				(MovDlg->qt)->isPlaying = 0;
				PlayingContMovie = false;
				if(pDoc != NULL)
					pDoc->isTranscribing = false;
			}
		}
	}
 }

/////////////////////////////////////////////////////////////////////////////
// CPadView Font Handling

static char changeStyleOfCurrentLine(char which, char color, char setAtt, ROWS *st, long sc, long ec) {
	char isOKtoChange, isDoAll, sb;
	long cnt, t;
	extern int ced_isskip(unCH *org, long pos);

	cnt = 0;
	isDoAll = (which == 0) || (!global_df->ChatMode) || (global_df->IsCainMode);
	sb = FALSE;
	if (!isDoAll) {
		isOKtoChange = FALSE;
		if (isSpeaker(st->line[cnt], FALSE)) {
			for (; st->line[cnt] != ':' && st->line[cnt] != EOS; cnt++) ;
			if (st->line[cnt] == ':')
				cnt++;
		}
		do {
			if (st->line[cnt] == '[')
				sb = TRUE;
			else if (st->line[cnt] == ']')
				sb = FALSE;
			if (isSpace(st->line[cnt])) {
				for (; cnt < ec && isSpace(st->line[cnt]); cnt++) ;
			} else if (st->line[cnt] == '[' && st->line[cnt+1] == '-') {
				for (; cnt < ec && st->line[cnt] != ']' && st->line[cnt] != EOS; cnt++) ;
				if (st->line[cnt] == ']')
					cnt++;
			} else if (st->line[cnt] == '+') {
				for (; cnt < ec && !isSpace(st->line[cnt]) && st->line[cnt] != '[' && st->line[cnt] != EOS; cnt++) ;
			} else
				isOKtoChange = TRUE;
		} while (!isOKtoChange && st->line[cnt] != EOS && cnt < ec) ;
		if (st->line[cnt] == EOS)
			return(FALSE);
	} else
		isOKtoChange = TRUE;
	if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll)
		return(FALSE);
	for (; cnt < sc; cnt++) {
		if (st->line[cnt] == '[')
			sb = TRUE;
		else if (st->line[cnt] == ']')
			sb = FALSE;
		if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll)
			return(FALSE);
	}
	if (which == 4 && isOKtoChange && !sb) {
		for (t=cnt-1; t >= 0 && !ced_isskip(st->line,t) && st->line[t] != ']'; t--) {
			st->att[t] = set_color_num(color, st->att[t]);
		}
	}
	for (; cnt < ec; cnt++) {
		if (st->line[cnt] == '[')
			sb = TRUE;
		else if (st->line[cnt] == ']')
			sb = FALSE;
		if (uS.IsUtteranceDel(st->line, cnt) && !sb && !isDoAll) {
			ec = cnt;
			for (cnt--; cnt > 0; cnt--) {
				if (st->line[cnt] == '+' || (ced_isskip(st->line,cnt) && !uS.IsUtteranceDel(st->line, cnt)) || is_unCH_alnum(st->line[cnt]))
					break;
			}
			if (st->line[cnt] == '+') {
				t = cnt;
				for (; cnt < ec; cnt++) {
					st->att[cnt] = set_underline_to_0(st->att[cnt]);
					st->att[cnt] = set_italic_to_0(st->att[cnt]);
					st->att[cnt] = set_bold_to_0(st->att[cnt]);
				}
				ec = t;
			}
			break;
		}

		if (!isOKtoChange) ;
		else if (which == 0) {
			st->att[cnt] = set_underline_to_0(st->att[cnt]);
			st->att[cnt] = set_italic_to_0(st->att[cnt]);
			st->att[cnt] = set_bold_to_0(st->att[cnt]);
		} else if (which == 1) {
			if (setAtt)
				st->att[cnt] = set_underline_to_1(st->att[cnt]);
			else
				st->att[cnt] = set_underline_to_0(st->att[cnt]);
		} else if (which == 2) {
			if (setAtt)
				st->att[cnt] = set_italic_to_1(st->att[cnt]);
			else
				st->att[cnt] = set_italic_to_0(st->att[cnt]);
		} else if (which == 3) {
			if (setAtt)
				st->att[cnt] = set_bold_to_1(st->att[cnt]);
			else
				st->att[cnt] = set_bold_to_0(st->att[cnt]);
		} else if (which == 4) {
			if (!ced_isskip(st->line,cnt) && !sb)
				st->att[cnt] = set_color_num(color, st->att[cnt]);
		}
	}
	if (isOKtoChange) {
		if (which == 4 && !ced_isskip(st->line,ec-1) && !sb) {
			for (cnt=ec; st->line[cnt] && !ced_isskip(st->line,cnt) && st->line[cnt] != '['; cnt++) {
				st->att[cnt] = set_color_num(color, st->att[cnt]);
			}
		} else
		for (cnt=ec-1; cnt > sc && isSpace(st->line[cnt]); cnt--) {
			if (which == 0) {
				st->att[cnt] = set_underline_to_0(st->att[cnt]);
				st->att[cnt] = set_italic_to_0(st->att[cnt]);
				st->att[cnt] = set_bold_to_0(st->att[cnt]);
			} else if (which == 1) {
				st->att[cnt] = set_underline_to_0(st->att[cnt]);
			} else if (which == 2) {
				st->att[cnt] = set_italic_to_0(st->att[cnt]);
			} else if (which == 3) {
				st->att[cnt] = set_bold_to_0(st->att[cnt]);
			} else if (which == 4) {
				st->att[cnt] = zero__color_num(st->att[cnt]);
			}
		}
	}
	return(TRUE);
}

static void StyleItems(char which, char color) {
    char setAtt, isRightTier;
	long cnt;
	long sc, ec;
	ROWS *st, *et, *tt;

	ced_punctuation = PUNCTUATION_SET;
	if (global_df->ScrollBar && (global_df->row_win2 || global_df->col_win2 != -2L)) {
		ChangeCurLineAlways(0);
		if (global_df->row_win2 == 0) {
			for (tt=global_df->row_txt; !isSpeaker(tt->line[0], FALSE) && !AtTopEnd(tt,global_df->head_text,FALSE); tt=ToPrevRow(tt, FALSE)) ;
			if (tt->line[0] == '*' || global_df->IsCainMode)
				isRightTier = TRUE;
			else
				return;
			if (global_df->row_txt->att == NULL) {
				global_df->row_txt->att = (AttTYPE *)malloc((strlen(global_df->row_txt->line)+1)*sizeof(AttTYPE));
				if (global_df->row_txt->att == NULL)
					mem_err(TRUE, global_df);
				for (cnt=0; global_df->row_txt->line[cnt]; cnt++)
					global_df->row_txt->att[cnt] = 0;
			}
			if (global_df->col_win > global_df->col_win2) {
				sc = global_df->col_chr2;
				ec = global_df->col_chr;
			} else {
				sc = global_df->col_chr;
				ec = global_df->col_chr2;
			}
			if (which == 0) {
				setAtt = FALSE;
			} else if (which == 1) {
				if (is_underline(global_df->row_txt->att[sc]))
					setAtt = FALSE;
				else
					setAtt = TRUE;
			} else if (which == 2) {
				if (is_italic(global_df->row_txt->att[sc]))
					setAtt = FALSE;
				else
					setAtt = TRUE;
			} else if (which == 3) {
				if (is_bold(global_df->row_txt->att[sc]))
					setAtt = FALSE;
				else
					setAtt = TRUE;
			} else if (which == 4) {
				setAtt = TRUE;
			}
			if (changeStyleOfCurrentLine(which, color, setAtt, global_df->row_txt, sc, ec))
				CpCur_lineAttToHead_rowAtt(global_df->cur_line);
		} else {
			if (global_df->row_win2 < 0) {
				et = global_df->row_txt;
				for (cnt=global_df->row_win2, st=global_df->row_txt; cnt && !AtTopEnd(st,global_df->head_text,FALSE); 
								cnt++, st=ToPrevRow(st, FALSE)) ;
				sc = global_df->col_chr2; ec = global_df->col_chr;
			} else if (global_df->row_win2 > 0L) {
				st = global_df->row_txt;
				for (cnt=global_df->row_win2, et=global_df->row_txt; cnt && !AtBotEnd(et,global_df->tail_text,FALSE);
								cnt--, et=ToNextRow(et, FALSE)) ;
				sc = global_df->col_chr; ec = global_df->col_chr2;
			}

			for (tt=st; !isSpeaker(tt->line[0], FALSE) && !AtTopEnd(tt,global_df->head_text,FALSE); tt=ToPrevRow(tt, FALSE)) ;
			if (tt->line[0] == '*' || global_df->IsCainMode)
				isRightTier = TRUE;
			else
				isRightTier = FALSE;

			if (isRightTier) {
				if (st->att == NULL) {
					st->att = (AttTYPE *)malloc((strlen(st->line)+1)*sizeof(AttTYPE));
					if (st->att == NULL)
						mem_err(TRUE, global_df);
					for (cnt=0; st->line[cnt]; cnt++)
						st->att[cnt] = 0;
				}
				if (which == 0) {
					setAtt = FALSE;
				} else if (which == 1) {
					if (is_underline(st->att[sc]))
						setAtt = FALSE;
					else
						setAtt = TRUE;
				} else if (which == 2) {
					if (is_italic(st->att[sc]))
						setAtt = FALSE;
					else
						setAtt = TRUE;
				} else if (which == 3) {
					if (is_bold(st->att[sc]))
						setAtt = FALSE;
					else
						setAtt = TRUE;
				} else if (which == 4) {
					setAtt = TRUE;
				}
				changeStyleOfCurrentLine(which, color, setAtt, st, sc, strlen(st->line));
			} else {
				if (which == 0)
					setAtt = FALSE;
				else
					setAtt = TRUE;
			}
				
			if (!AtBotEnd(st,et,FALSE)) {
				do {
					st = ToNextRow(st, FALSE);
					if (st->line[0] == '*' || global_df->IsCainMode)
						isRightTier = TRUE;
					else if (st->line[0] == '@' || st->line[0] == '%')
						isRightTier = FALSE;
					if (isRightTier) {
						if (st->att == NULL) {
							st->att = (AttTYPE *)malloc((strlen(st->line)+1)*sizeof(AttTYPE));
							if (st->att == NULL)
								mem_err(TRUE, global_df);
							for (cnt=0; st->line[cnt]; cnt++)
								st->att[cnt] = 0;
						}
						changeStyleOfCurrentLine(which, color, setAtt, st, 0, strlen(st->line));
					}
				} while (!AtBotEnd(st,et,FALSE)) ;
			}
			st = ToNextRow(st, FALSE);
			if (st->line[0] == '*' || global_df->IsCainMode)
				isRightTier = TRUE;
			else if (st->line[0] == '@' || st->line[0] == '%')
				isRightTier = FALSE;
			if (isRightTier) {
				if (st->att == NULL) {
					st->att = (AttTYPE *)malloc((strlen(st->line)+1)*sizeof(AttTYPE));
					if (st->att == NULL)
						mem_err(TRUE, global_df);
					for (cnt=0; st->line[cnt]; cnt++)
						st->att[cnt] = 0;
				}
				changeStyleOfCurrentLine(which, color, setAtt, st, 0, ec);
			}
			CpCur_lineAttToHead_rowAtt(global_df->cur_line);
		}
		DisplayTextWindow(NULL, 1);
		PosAndDispl();
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->col_txt->prev_char != global_df->head_row)
				global_df->gAtt = global_df->col_txt->prev_char->att;
			else
				global_df->gAtt = 0;
		} else if (global_df->row_txt->att != NULL) {
			if (global_df->col_chr > 0)
				global_df->gAtt = global_df->row_txt->att[global_df->col_chr-1];
			else
				global_df->gAtt = 0;
		} else
			global_df->gAtt = 0;
	} else {
		if (which == 0) {
			global_df->gAtt = set_underline_to_0(global_df->gAtt);
			global_df->gAtt = set_italic_to_0(global_df->gAtt);
			global_df->gAtt = set_bold_to_0(global_df->gAtt);
		} else if (which == 1) {
			if (!is_underline(global_df->gAtt))
				global_df->gAtt = set_underline_to_1(global_df->gAtt);
			else
				global_df->gAtt = set_underline_to_0(global_df->gAtt);
		} else if (which == 2) {
			if (!is_italic(global_df->gAtt))
				global_df->gAtt = set_italic_to_1(global_df->gAtt);
			else
				global_df->gAtt = set_italic_to_0(global_df->gAtt);
		} else if (which == 3) {
			if (!is_bold(global_df->gAtt))
				global_df->gAtt = set_bold_to_1(global_df->gAtt);
			else
				global_df->gAtt = set_bold_to_0(global_df->gAtt);
		} else if (which == 4) {
			global_df->gAtt = set_color_num(color, global_df->gAtt);
		}
	}
}

void CClan2View::OnPlain() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	StyleItems(0, 0);
	DrawCursor(1);
	global_df = NULL;
	GlobalDC = tGlobalDC;	
}

void CClan2View::OnUnderline() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	StyleItems(1, 0);
	DrawCursor(1);
	global_df = NULL;
	GlobalDC = tGlobalDC;
}

void CClan2View::OnItalic() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	StyleItems(2, 0);
	DrawCursor(1);
	global_df = NULL;
	GlobalDC = tGlobalDC;
}

void CClan2View::OnBold() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	StyleItems(3, 0);
	DrawCursor(1);
	global_df = NULL;
	GlobalDC = tGlobalDC;
}

int SelectWordColor(void) { // F7
	char color;
	char SB;
	COLORWORDLIST *t;

	if (cColor == NULL)
		cColor = global_df->RootColorWord;
	else {
		for (t=global_df->RootColorWord; cColor != t && t != NULL; t=t->nextCW) ;
		if (cColor == t && cColor != NULL) {
			cColor = cColor->nextCW;
		} else
			cColor = global_df->RootColorWord;
	}
	if (cColor != NULL)
		color = cColor->color;
	else
		color = 0;

	SB = global_df->ScrollBar;
	global_df->ScrollBar = 1;
	StyleItems(4, color);
    global_df->ScrollBar = SB;

	global_df->LeaveHighliteOn = TRUE;
	return(1);
}


void CClan2View::OnColorWords() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	SelectWordColor();
	DrawCursor(1);
	global_df = NULL;
	GlobalDC = tGlobalDC;
}

// lxs lxs lxslxs
void CClan2View::OnColorKeywords() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	CKeywordColor dlg;
	extern char showColorKeywords;

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);

	dlg.ColorChanged = FALSE;
	dlg.m_ApplyColor = showColorKeywords;
	dlg.DoModal();
	showColorKeywords = dlg.m_ApplyColor;

	if (dlg.ColorChanged) {
		global_df->DataChanged = TRUE;
		touchwin(global_df->w1);
		wrefresh(global_df->w1);
		draw_mid_wm();
		GlobalDoc->SetModifiedFlag(TRUE);
	}

	DrawCursor(1);
	global_df = NULL;
	GlobalDC = tGlobalDC;
}

void CClan2View::OnChooseDefaultFont() 
{
	LOGFONT lf;

	if (m_lfDefFont.lfHeight != 0)
		m_font.GetObject(sizeof(LOGFONT), &lf);
	else {
		::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
		lf.lfWeight = FW_NORMAL;
		m_font.CreateFontIndirect(&lf);
	}
	CFontDialog dlg(&lf, CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_FORCEFONTEXIST);
	if (dlg.DoModal() == IDOK) {
		m_lfDefFont = lf;
#ifdef _UNICODE
		extern long w95_fontSize;
		if (!strcmp(m_lfDefFont.lfFaceName, dFnt.fontName)) {
			dFnt.fontSize = m_lfDefFont.lfHeight;
//			WriteCedPreference();
		}
		w95_fontSize = 0L;
#endif
		convA_strcpy(dFnt.fontName, lf.lfFaceName, 256);
		dFnt.fontSize = lf.lfHeight;
		dFnt.CharSet = (int)lf.lfCharSet;
		dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
		dFnt.orgEncod = dFnt.Encod;
		dFnt.orgFType = NOCHANGE;
		if (!strcmp(dFnt.fontName, "Arial Unicode MS") || !strcmp(dFnt.fontName, "CAFont"/*UNICODEFONT*/))
			dFnt.isUTF = TRUE;
		copyNewFontInfo(&oFnt, &dFnt);
		C_MBF = uS.partwcmp(dFnt.fontName,"Jpn ") || uS.partwcmp(dFnt.fontName,"Chn ");
		WriteCedPreference();

		m_font.DeleteObject();
		m_font.CreateFontIndirect(&lf);
	}
}

void CClan2View::OnChooseCommandFont() 
{
	LOGFONT lf;
	
	SetLogfont(&lf, NULL, &dFnt);
	CFontDialog dlg(&lf, CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT);
	if (dlg.DoModal() == IDOK) {
#ifdef _UNICODE
		if (!strcmp(lf.lfFaceName, m_lfDefFont.lfFaceName)) {
			extern long w95_fontSize;
			m_lfDefFont.lfHeight = lf.lfHeight;
			w95_fontSize = 0L;
		}
#endif
		convA_strcpy(dFnt.fontName, lf.lfFaceName, 256);
		dFnt.fontSize = lf.lfHeight;
		dFnt.CharSet = (int)lf.lfCharSet;
		dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
		dFnt.orgEncod = dFnt.Encod;
		dFnt.orgFType = NOCHANGE;
		if (!strcmp(dFnt.fontName, "Arial Unicode MS") || !strcmp(dFnt.fontName, "CAFont"/*UNICODEFONT*/))
			dFnt.isUTF = TRUE;
		copyNewFontInfo(&oFnt, &dFnt);
		C_MBF = uS.partwcmp(dFnt.fontName,"Jpn ") || uS.partwcmp(dFnt.fontName,"Chn ");
		WriteCedPreference();
		if (clanDlg != NULL) {
			clanDlg->OnCancel();
			clanDlg = new CClanWindow(AfxGetApp()->m_pMainWnd);
		}
	}
}

void CClan2View::OnChooseFont() {
	LOGFONT lf;
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	char FName[LF_FACESIZE];
    short fontHeight;
    ROWS *rt;
	FONTINFO fi;
	extern char clanRun;

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL) {
//		OnChooseCommandFont();
		return;
	}
//	else if (!strcmp(global_df->fileName, "CLAN Output")) {
//		OnChooseCommandFont();
//	}
	GlobalDoc = pDoc;

	stopSoundIfPlaying();
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize) {
		PutCursorInWindow(global_df->w1);
		SetScrollControl();
	}
	if (global_df->row_win2 == 0L && global_df->col_win2 == -2L) {
		SetLogfont(&lf, &global_df->row_txt->Font, NULL);
	} else {
		if (global_df->col_win <= 0L && global_df->row_win2 < 0L)
			rt = ToPrevRow(global_df->row_txt, FALSE);
		else
			rt = global_df->row_txt;
		SetLogfont(&lf, &rt->Font, NULL);
	}

	CFontDialog dlg(&lf, CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_FORCEFONTEXIST);
	if (dlg.DoModal() == IDOK) {
		DrawCursor(0);
		DrawSoundCursor(0);
		if (!strcmp(global_df->fileName, "CLAN Output")) {
			convA_strcpy(dFnt.fontName, lf.lfFaceName, 256);
			dFnt.fontSize = lf.lfHeight;
			dFnt.CharSet = (int)lf.lfCharSet;
			dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
			dFnt.isUTF = global_df->isUTF;
			dFnt.orgEncod = dFnt.Encod;
			dFnt.orgFType = NOCHANGE;
			if (!strcmp(dFnt.fontName, "Arial Unicode MS") || !strcmp(dFnt.fontName, "CAFont"/*UNICODEFONT*/))
				dFnt.isUTF = TRUE;
			C_MBF = uS.partwcmp(dFnt.fontName,"Jpn ") || uS.partwcmp(dFnt.fontName,"Chn ");
			WriteCedPreference();
		}
		convA_strcpy(fi.FName, lf.lfFaceName, LF_FACESIZE);
		fi.FSize = lf.lfHeight;
		fi.CharSet = (int)lf.lfCharSet;
		fontHeight = GetFontHeight(&fi, NULL);
		if (global_df->MinFontSize > fontHeight || global_df->MinFontSize == 0)
			global_df->MinFontSize = fontHeight;
		convA_strcpy(FName, lf.lfFaceName, LF_FACESIZE);
		cedDFnt.Encod = my_FontToScript(FName, lf.lfCharSet);
		cedDFnt.isUTF = global_df->isUTF;
		ChangeCurLineAlways(0);
		if (global_df->row_win2 == 0L && global_df->col_win2 == -2L) {
			strcpy(global_df->row_txt->Font.FName, FName);
			global_df->row_txt->Font.FSize = lf.lfHeight;
			global_df->row_txt->Font.CharSet = (int)lf.lfCharSet;
			global_df->row_txt->Font.FHeight = fontHeight;
		} else {
			long cnt;

			if (global_df->row_win2 == 0) {
				strcpy(global_df->row_txt->Font.FName, FName);
				global_df->row_txt->Font.FSize = lf.lfHeight;
				global_df->row_txt->Font.CharSet = (int)lf.lfCharSet;
				global_df->row_txt->Font.FHeight = fontHeight;
			} else {
				cnt = global_df->row_win2;
				if (global_df->row_win2 < 0L) {
					if (global_df->col_win > 0)
						rt = global_df->row_txt;
					else {
						cnt++;
						rt = ToPrevRow(global_df->row_txt, FALSE);
					}
					for (; cnt<=0 && rt!=global_df->head_text; cnt++, rt=ToPrevRow(rt,FALSE)) {
						strcpy(rt->Font.FName, FName);
						rt->Font.FSize = lf.lfHeight;
						rt->Font.CharSet = (int)lf.lfCharSet;
						rt->Font.FHeight = fontHeight;
					}
				} else if (global_df->row_win2 > 0L) {
					if (global_df->col_win2 <= 0) cnt--;
					for (rt=global_df->row_txt; cnt >= 0 && rt != global_df->tail_text;
											cnt--, rt=ToNextRow(rt, FALSE)) {
						strcpy(rt->Font.FName, FName);
						rt->Font.FSize = lf.lfHeight;
						rt->Font.CharSet = (int)lf.lfCharSet;
						rt->Font.FHeight = fontHeight;
					}
				}
			}
		}
		DestroyCaret();
		clear();
		global_df->total_num_rows = 1;
		global_df->CodeWinStart = 0;
		if (!init_windows(true, 1, false))
	    	mem_err(TRUE, global_df);
		if (global_df->DataChanged == '\0') {
			global_df->DataChanged = '\001';
			pDoc->SetModifiedFlag(TRUE);
		}
		global_df->WinChange = FALSE;
		fontHeight = ::MulDiv(-lf.lfHeight, 72, dc.GetDeviceCaps(LOGPIXELSY));
		sprintf(global_df->err_message, "-Font: %s %ld", FName, fontHeight);
		draw_mid_wm();
		global_df->WinChange = TRUE;
		strcpy(global_df->err_message, DASHES);
		DrawCursor(3);
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnDestroy() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	stopMovieIfPlaying();
	SetPBCglobal_df(true, 0L);
	FreeFileInfo(pDoc->FileInfo);
	global_df = NULL;
	GlobalDC = tGlobalDC;
	CView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CPadView Input Handling
void CClan2View::call_win95_call(UINT nChar, UINT nRepCnt, UINT nFlags, UINT extended) {
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (extended)
		global_df->isExtend = (char)extended;
	whichInput = win95_call(nChar, nRepCnt, nFlags, whichInput);
	global_df->isExtend = 0;
	while (isPlayS) {
		win95_call(0, 0, 0, 0);
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDoc = pDoc;
	GlobalDC = &dc;
	global_df = pDoc->FileInfo;
	if (cursorCnt < 0)
		cursorCnt = ShowCursor(TRUE);

	if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->isPlaying) {
		if (PlayingContMovie || !PBC.walk || !PBC.enable) {
			stopMoviePlaying();
			PlayingContMovie = FALSE;
		}
	}

	if (nFlags & MK_CONTROL)
		StartSelectCursorPosition(point, 2);
	else if (nFlags & MK_SHIFT)
		StartSelectCursorPosition(point, 1);
	else
		StartSelectCursorPosition(point, 0);

	GlobalDC = tGlobalDC;

	SetCapture();       // Capture the mouse until button up.
//	CView::OnLButtonDblClk(nFlags, point);
}

void CClan2View::OnLButtonDown(UINT nFlags, CPoint point) 
{
/*
nFlags	Indicates whether various virtual keys are down. 
		This parameter can be any combination of the following values:
	MK_CONTROL	Set if the CTRL key is down.
	MK_LBUTTON	Set if the left mouse button is down.
	MK_MBUTTON	Set if the middle mouse button is down.
	MK_RBUTTON	Set if the right mouse button is down.
	MK_SHIFT	Set if the SHIFT key is down.
point	Specifies the x- and y-coordinate of the cursor. 
		These coordinates are always relative to the upper-left corner of 
		the window.
*/
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDoc = pDoc;
	GlobalDC = &dc;
	global_df = pDoc->FileInfo;
	if (cursorCnt < 0)
		cursorCnt = ShowCursor(TRUE);

	if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->isPlaying) {
		if (PlayingContMovie || !PBC.walk || !PBC.enable) {
			stopMoviePlaying();
			PlayingContMovie = FALSE;
		}
	}

	if (nFlags & MK_CONTROL)
		StartSelectCursorPosition(point, 2);
	else if (nFlags & MK_SHIFT)
		StartSelectCursorPosition(point, 1);
	else {
		StartSelectCursorPosition(point, 0);
	}
	if (global_df) {
		global_df->cursorCol = 0L;
		strcpy(global_df->err_message, DASHES);
	}
	GlobalDC = tGlobalDC;

	SetCapture();       // Capture the mouse until button up.
//	CView::OnLButtonDown(nFlags, point);
}

void CClan2View::OnMouseMove(UINT nFlags, CPoint point) 
{
	RECT  wDim;
	POINT mPos;
	CClan2Doc* pDoc = GetDocument();

	if (pDoc != NULL && pDoc->FileInfo != NULL) {
		if (GetCursorPos(&mPos)) {
			GetWindowRect(&wDim);
			if (mPos.y < wDim.top+pDoc->FileInfo->TextWinSize)
				SetCursor(LoadCursor(NULL, IDC_IBEAM));
			else
				SetCursor(LoadCursor(NULL, IDC_ARROW));
		} else 
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
	}
	if (cursorCnt < 0)
		cursorCnt = ShowCursor(TRUE);
	if (GetCapture() != this)
		return; // If this window (view) didn't capture the mouse,
				// then the user isn't drawing in this window.

	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	ASSERT_VALID(pDoc);
	GlobalDoc = pDoc;
	GlobalDC = &dc;
	global_df = pDoc->FileInfo;

	if (nFlags & MK_CONTROL)
		MidSelectCursorPosition(point, 2);
	else if (nFlags & MK_SHIFT)
		MidSelectCursorPosition(point, 1);
	else {
		if (MidSelectCursorPosition(point, 0) == 1)
			isMouseButtonDn = GlobalDC->GetWindow();
	}
	GlobalDC = tGlobalDC;

//	CView::OnMouseMove(nFlags, point);
}

void CClan2View::OnLButtonUp(UINT nFlags, CPoint point) 
{
	extern char *lineNumFname;
	extern long lineNumOverride;

	if (GetCapture() != this)
		return; // If this window (view) didn't capture the mouse,

	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDoc = pDoc;
	GlobalDC = &dc;
	global_df = pDoc->FileInfo;

	isMouseButtonDn = NULL;
	if (nFlags & MK_CONTROL)
		EndSelectCursorPosition(point, 2);
	else if (nFlags & MK_SHIFT)
		EndSelectCursorPosition(point, 1);
	else
		EndSelectCursorPosition(point, 0);

	if (lineNumFname == NULL) {
		DestroyCaret();
		DrawCursor(3);
	}
	if (lineNumOverride < 0L && lineNumFname != NULL)
		lineNumFname = NULL;
		
	ReleaseCapture();   // Release the mouse capture established at

	if (isPlayS == -8) {
		ShowFStruct();
		isPlayS = 0;
	}
	while (isPlayS) {
		win95_call(0, 0, 0, 0);
	}
	DrawSoundCursor(1);
	if (global_df->row_txt == global_df->cur_line) {
		if (global_df->col_txt->prev_char != global_df->head_row)
			global_df->gAtt = global_df->col_txt->prev_char->att;
		else
			global_df->gAtt = 0;
	} else if (global_df->row_txt->att != NULL) {
		if (global_df->col_chr > 0)
			global_df->gAtt = global_df->row_txt->att[global_df->col_chr-1];
		else
			global_df->gAtt = 0;
	} else
		global_df->gAtt = 0;
	GlobalDC = tGlobalDC;
						// the beginning of the mouse drag.

//	CView::OnLButtonUp(nFlags, point);
}

BOOL CClan2View::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	CWnd* win = GlobalDC->GetWindow();
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	if (zDelta > 0) {
		HandleVScrollBar(SB_LINEUP, 0, win);
		HandleVScrollBar(SB_LINEUP, 0, win);
		HandleVScrollBar(SB_LINEUP, 0, win);
		HandleVScrollBar(SB_LINEUP, 0, win);
	} else if (zDelta < 0) {
		HandleVScrollBar(SB_LINEDOWN, 0, win);
		HandleVScrollBar(SB_LINEDOWN, 0, win);
		HandleVScrollBar(SB_LINEDOWN, 0, win);
		HandleVScrollBar(SB_LINEDOWN, 0, win);
	}
	DrawCursor(1);
	if (!global_df)
		strcpy(global_df->err_message, DASHES);
	GlobalDC = tGlobalDC;

	//return CView::OnMouseWheel(nFlags, zDelta, pt);
	return true;
}

void CClan2View::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDoc = pDoc;
	pDoc->isNeedResize = TRUE;
}

void CClan2View::OnFileSave() 
{
	// TODO: Add your command handler code here
	
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;

	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (!global_df)
		return;
	DrawCursor(0);
	global_df->LastCommand = SaveCurrentFile(-1);
	pDoc->SetModifiedFlag(global_df->DataChanged);
	FinishMainLoop();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnFileSaveAs() 
{
	// TODO: Add your command handler code here
	
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;

	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (!global_df)
		return;
	DrawCursor(0);
	global_df->LastCommand = clWriteFile(-1);
	pDoc->SetModifiedFlag(global_df->DataChanged);
	FinishMainLoop();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	CWnd* win = GlobalDC->GetWindow();
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	HandleVScrollBar(nSBCode, nPos, win);
	DrawCursor(1);
	if (!global_df)
		strcpy(global_df->err_message, DASHES);
	GlobalDC = tGlobalDC;
//	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CClan2View::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	CWnd* win = GlobalDC->GetWindow();
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	HandleHScrollBar(nSBCode, nPos, win);
	DrawCursor(1);
	if (!global_df)
		strcpy(global_df->err_message, DASHES);
	GlobalDC = tGlobalDC;
//	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CClan2View::OnEditCopy() {
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df != NULL) {
		strcpy(global_df->err_message, DASHES);
		doCopy();
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnEditCut() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL) {
		GlobalDC = tGlobalDC;
		return;
	}
	if (global_df->ScrollBar && (global_df->row_win2 || global_df->col_win2 != -2L)) {
		strcpy(global_df->err_message, DASHES);
		DrawCursor(0);
		if (global_df->row_win2 < -25 || global_df->row_win2 > 25)
			ResetUndos();
		SaveUndoState(FALSE);
		doCopy();
		DeleteChank(1);
		PosAndDispl();
		global_df->row_win2 = 0L;
		global_df->col_win2 = -2L;
		global_df->col_chr2 = -2L;
		DrawCursor(1);
		SetScrollControl();
		if (global_df->DataChanged)
			GlobalDoc->SetModifiedFlag(TRUE);
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->col_txt->prev_char != global_df->head_row)
				global_df->gAtt = global_df->col_txt->prev_char->att;
			else
				global_df->gAtt = 0;
		} else if (global_df->row_txt->att != NULL) {
			if (global_df->col_chr > 0)
				global_df->gAtt = global_df->row_txt->att[global_df->col_chr-1];
			else
				global_df->gAtt = 0;
		} else
			global_df->gAtt = 0;
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnEditPaste() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df != NULL) {
		strcpy(global_df->err_message, DASHES);
		DrawCursor(0);
		doPaste();
		DrawCursor(1);
		SetScrollControl();
		if (global_df->DataChanged)
			GlobalDoc->SetModifiedFlag(TRUE);
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnEditUndo() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df) {
		strcpy(global_df->err_message, DASHES);
		DrawCursor(0);
		SaveUndoState(TRUE);
		Undo(1);
		PosAndDispl();
		DrawCursor(1);
		SetScrollControl();
		if (global_df->DataChanged)
			GlobalDoc->SetModifiedFlag(TRUE);
	}
	GlobalDC = tGlobalDC;
}


void CClan2View::OnEditRedo() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df) {
		strcpy(global_df->err_message, DASHES);
		DrawCursor(0);
		SaveUndoState(TRUE);
		Redo(1);
		PosAndDispl();
		DrawCursor(1);
		SetScrollControl();
		if (global_df->DataChanged)
			GlobalDoc->SetModifiedFlag(TRUE);
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSelectAll() 
{
	isPlayS = 75;
	call_win95_call(0, 0, 0, 0);
}


void CClan2View::OnEditFind() 
{
	int  i;
	CHAR oldSearchWrap;
	CCedFindString dlg;
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	extern char CaseSensSearch;
	extern char SearchWrap;
	extern char searchListFileName[];
	extern char SearchFromList;

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	strcpy(global_df->err_message, DASHES);
	// Initialize dialog data
	strcpy(templineW, SearchString);
	for (i=0; templineW[i] != EOS; i++) {
		if (templineW[i] == HIDEN_C)
			templineW[i] = 0x2022;
	}
	dlg.m_SearchString = templineW;
	dlg.m_SearchBackwards = !SearchFFlag;
	dlg.m_Wrap = SearchWrap;
	dlg.m_CaseSensitive = CaseSensSearch;
	dlg.m_UseFileList = SearchFromList;
	dlg.m_SearchListFName = cl_T(searchListFileName);

	oldSearchWrap = SearchWrap;
	// Invoke the dialog box
	if (dlg.DoModal() == IDOK) {
		// retrieve the dialog data
		DrawCursor(0);
		strcpy(SearchString, dlg.m_SearchString);
		for (i=0; SearchString[i] != EOS; i++) {
			if (SearchString[i] == 0x2022)
				SearchString[i] = HIDEN_C;
		}
		SearchFFlag = !dlg.m_SearchBackwards;
		SearchWrap = dlg.m_Wrap;
		CaseSensSearch = dlg.m_CaseSensitive;
		SearchFromList = dlg.m_UseFileList;
		if (!isThereSearchList())
			SearchFromList = FALSE;
		if (!CaseSensSearch)
			uS.uppercasestr(SearchString, &dFnt, C_MBF);
		global_df->row_win2 = 0L;
		global_df->col_win2 = -2L;
		global_df->col_chr2 = -2L;
		global_df->LeaveHighliteOn = FALSE;
		SaveUndoState(FALSE);
		if (SearchFFlag)
			global_df->LastCommand = SearchForward(-1);
		else
			global_df->LastCommand = SearchReverse(-1);
		FinishMainLoop();
		SetScrollControl();
		DrawCursor(1);
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->col_txt->prev_char != global_df->head_row)
				global_df->gAtt = global_df->col_txt->prev_char->att;
			else
				global_df->gAtt = 0;
		} else if (global_df->row_txt->att != NULL) {
			if (global_df->col_chr > 0)
				global_df->gAtt = global_df->row_txt->att[global_df->col_chr-1];
			else
				global_df->gAtt = 0;
		} else
			global_df->gAtt = 0;
		if (oldSearchWrap != SearchWrap)
			WriteCedPreference();
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnEditEnterSelection() 
{
    ROWS *rt;
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	extern char CaseSensSearch;

	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win2 == 0 && global_df->col_win2 != -2L) {
		ChangeCurLineAlways(0);
		if (global_df->col_win > global_df->col_win2) {
			if (global_df->col_chr-global_df->col_chr2 < SPEAKERLEN) {
				strncpy(SearchString, global_df->row_txt->line+global_df->col_chr2, global_df->col_chr-global_df->col_chr2);
				SearchString[global_df->col_chr-global_df->col_chr2] = EOS;
			}
		} else {
			if (global_df->col_chr2-global_df->col_chr < SPEAKERLEN) {
				strncpy(SearchString, global_df->row_txt->line+global_df->col_chr, global_df->col_chr2-global_df->col_chr);
				SearchString[global_df->col_chr2-global_df->col_chr] = EOS;
			}
		}
		if (!CaseSensSearch)
			uS.uppercasestr(SearchString, &dFnt, C_MBF);
	} else if (global_df->row_win2 == 1 || global_df->row_win2 == -1) {
		long i;
		ChangeCurLineAlways(0);
		if (global_df->row_win2 == 1) {
			i = strlen(global_df->row_txt->line) - global_df->col_chr;
			if (i < SPEAKERLEN) {
				strncpy(SearchString, global_df->row_txt->line+global_df->col_chr, i);
				SearchString[i] = EOS;
			}
		} else {
			rt = ToPrevRow(global_df->row_txt, FALSE);
			if (rt != global_df->head_text) {
				i = strlen(rt->line) - global_df->col_chr2;
				if (i < SPEAKERLEN) {
					strncpy(SearchString, rt->line+global_df->col_chr2, i);
					SearchString[i] = EOS;
				}
			}
		}
		for (i=0; SearchString[i];) {
			if (SearchString[i] == NL_C)
				strcpy(SearchString+i, SearchString+i+1);
			else i++;
		}
		if (SearchString[i-1] != '\r')
			strcat(SearchString, "\r");
		if (!CaseSensSearch)
			uS.uppercasestr(SearchString, &dFnt, C_MBF);
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnEditFindSame() 
{
	isPlayS = 76;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnEditReplace() 
{
	isPlayS = 7;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnEditReplacefind() 
{
	isPlayS = 86;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnEditGoto() 
{
	isPlayS = 12;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnEditToUpper() 
{
	isPlayS = 10;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnEditToLower() 
{
	isPlayS = 11;
	call_win95_call(0, 0, 0, 0);
}

void RefreshAllTextWindows(void) {
	myFInfo *tGlobal_df = global_df;
	CWinApp* wApp = AfxGetApp();
	POSITION pos = wApp->GetFirstDocTemplatePosition();
	while (pos != NULL) {
		CDocTemplate* pTemplate = (CDocTemplate*)wApp->GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		POSITION pos2 = pTemplate->GetFirstDocPosition();
		while (pos2 != NULL) {
			CDocument* pDoc = pTemplate->GetNextDoc(pos2);
			POSITION pos3 = pDoc->GetFirstViewPosition();
			while (pos3 != NULL) {
				CView* pView = pDoc->GetNextView(pos3);
				if (pView != NULL) {
					pDoc->UpdateAllViews(NULL, 0L, NULL);
				}
			}
		}
	}
	global_df = tGlobal_df;
}

void CClan2View::OnSetTab() 
{
	CSetTab dlg;
	
//	if (global_df != NULL)
//		DrawCursor(1);
	dlg.m_SetTab = TabSize;
	if (dlg.DoModal() == IDOK) {
		TabSize = dlg.m_SetTab;
		if (TabSize <= 0 || TabSize > 40)
			TabSize = 8;
		WriteCedPreference();
		RefreshAllTextWindows();
	}
//	if (global_df != NULL)
//		DrawCursor(0);
}

void CClan2View::OnLinenumberSize() 
{
	CLineNumSize dlg;
	
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
//	if (global_df != NULL)
//		DrawCursor(1);
	dlg.m_LineNumSize = LineNumberDigitSize;
    dlg.m_NumberEveryLine = FALSE;
    dlg.m_NumberMainTiersOnly = FALSE;
	if (LineNumberingType == 0)
    	dlg.m_NumberEveryLine = TRUE;
    else if (LineNumberingType == 1)
    	dlg.m_NumberMainTiersOnly = TRUE;
	if (dlg.DoModal() == IDOK) {
		LineNumberDigitSize = dlg.m_LineNumSize;
		if (LineNumberDigitSize > 20)
			LineNumberDigitSize = 20;
		if (dlg.m_NumberEveryLine)
			LineNumberingType = 0;
		else if (dlg.m_NumberMainTiersOnly)
			LineNumberingType = 1;

		WriteCedPreference();
		global_df->total_num_rows = 1;
		global_df->CodeWinStart = 0;
		init_windows(true, 1, true);
		RefreshAllTextWindows();
	}
//	if (global_df != NULL)
//		DrawCursor(0);
	GlobalDC = tGlobalDC;
}


void CClan2View::OnSoundAnalizerApp() 
{
	CSelectSoundAnalyzer dlg; 

	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df != NULL)
		DrawCursor(1);
 	dlg.m_Praat = FALSE;
  	dlg.m_PitchWorks = FALSE;
	if (sendMessageTargetApp == PRAAT)
 		dlg.m_Praat = TRUE;
	else if (sendMessageTargetApp == PITCHWORKS)
 		dlg.m_PitchWorks = TRUE;
	if (dlg.DoModal() == IDOK) {
		if (dlg.m_PitchWorks)
			sendMessageTargetApp = PITCHWORKS;
		else if (dlg.m_Praat)
			sendMessageTargetApp = PRAAT;
		WriteCedPreference();
	}
	if (global_df != NULL)
		DrawCursor(0);
	GlobalDC = tGlobalDC;	
}

void CClan2View::OnF5Option() 
{
	CF5Options dlg; 

	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df != NULL)
		DrawCursor(1);
 	dlg.m_EveryTier = FALSE;
  	dlg.m_EveryLine = FALSE;
	dlg.m_F5_Offset = F5_Offset;
	if (F5Option == EVERY_LINE)
 		dlg.m_EveryLine = TRUE;
	else if (F5Option == EVERY_TIER)
 		dlg.m_EveryTier = TRUE;
	if (dlg.DoModal() == IDOK) {
		if (dlg.m_EveryLine)
			F5Option = EVERY_LINE;
		else if (dlg.m_EveryTier)
			F5Option = EVERY_TIER;
		F5_Offset = dlg.m_F5_Offset;
		WriteCedPreference();
	}
	if (global_df != NULL)
		DrawCursor(0);
	GlobalDC = tGlobalDC;	
}

void CClan2View::OnModeExpendBullets() 
{
	isPlayS = 73;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnModeChatmode() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	ChangeCAToCHATModes(0);
	isPlayS = 58;
	whichInput = win95_call(0, 0, 0, whichInput);
	while (isPlayS) {
		win95_call(0, 0, 0, 0);
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnModeCamode() 
{
	CWinApp* wApp = AfxGetApp();
	POSITION pos = wApp->GetFirstDocTemplatePosition();

	isChatLineNums = ((isChatLineNums == TRUE) ? FALSE : TRUE);
	while (pos != NULL) {
		CDocTemplate* pTemplate = (CDocTemplate*)wApp->GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		POSITION pos2 = pTemplate->GetFirstDocPosition();
		while (pos2 != NULL) {
			CClan2Doc* pDoc = (CClan2Doc *)pTemplate->GetNextDoc(pos2);
			pDoc->RedisplayWindow = TRUE;
			pDoc->UpdateAllViews(NULL, 0L, NULL);
		}
	}
	WriteCedPreference();
}


void CClan2View::OnModeUtf8() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	isPlayS = 81;
	whichInput = win95_call(0, 0, 0, whichInput);
	while (isPlayS) {
		win95_call(0, 0, 0, 0);
	}
	GlobalDC = tGlobalDC;
}

void CClan2View::OnModeCheckfile() 
{
	isPlayS = 72;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnModeCoder() 
{
	isPlayS = 4;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnModeLinksnd() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df != NULL)
		strcpy(global_df->err_message, DASHES);
	if (MPegDlg)
		MPegDlg->Save();
	else if (MovDlg)
		MovDlg->Save();
	else if (global_df->SoundWin) {
		SetCurrSoundTier();
	} else {
		char ret;
		if ((ret=GetNewMediaFile(NULL, TRUE, 0))) {
			if (ret == isAudio) { // sound
			} else if (ret == isVideo) { // movie
			} else if (ret == isPict) { // picture
				DrawCursor(0);
				addBulletsToText(PICTTIER, global_df->PcTr.pictFName, 0L, 0L, 1);
			} else if (ret == isText) { // text
				DrawCursor(0);
				addBulletsToText(TEXTTIER, global_df->TxTr.textFName, 0L, 0L, 1);
			} 
		}
	}
	FinishMainLoop();
	DrawSoundCursor(1);
	DrawCursor(1);
	if (global_df && global_df->DataChanged)
		pDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	GlobalDC = tGlobalDC;
}

void CClan2View::OnModeSoundtotextsync() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	isPlayS = 50;
	whichInput = win95_call(0, 0, 0, whichInput);
	while (isPlayS) {
		win95_call(0, 0, 0, 0);
	}
	DrawSoundCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnModePlaysound() 
{
	isPlayS = 49;
	call_win95_call(0, 0, 0, 0);
}


void CClan2View::OnModeTransmedia() 
{
	isPlayS = 89;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnModePlaycont() 
{
	isPlayS = 71;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnModeSoundmode() 
{
	isPlayS = 63;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnModeDisambig() 
{
	isPlayS = 69;
	InKey = 0;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnModeTierExclusion() 
{
	isPlayS = 65;
	call_win95_call(0, 0, 0, 0);
}

void CClan2View::OnMacrosDefine() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df != NULL)
		strcpy(global_df->err_message, DASHES);
	DoDefContStrings();
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros1() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(0);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros2() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(1);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros3() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(2);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros4() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(3);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros5() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(4);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros6() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(5);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros7() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(6);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros8() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(7);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros9() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(8);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnMacros10() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddConstString(9);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakersUpdate() 
{
	int i;
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df != NULL)
		strcpy(global_df->err_message, DASHES);
	for (i=0; i < 10; i++)
		AllocSpeakerNames(cl_T(""), i);
	SetUpParticipants();
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers1() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(1);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers2() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(2);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers3() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(3);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers4() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(4);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers5() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(5);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers6() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(6);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers7() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(7);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers8() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(8);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers9() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(9);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnSpeakers10() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df == NULL)
		return;
	DrawCursor(0);
	strcpy(global_df->err_message, DASHES);
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	SaveUndoState(FALSE);
	AddSpeakerNames(0);
	FinishMainLoop();
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

void CClan2View::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL isHideCursor;
	CClan2Doc* pDoc = GetDocument();

	isHideCursor = (isKeyEqualCommand(nChar, 1) || isKeyEqualCommand(nChar, 5)) && whichInput == 0;
	if (CAKeyShortcuts) {
		wchar_t CAChar;
		if (HandleCAKeyShortcuts(nChar, false, &CAChar)) {
			call_win95_call(CAChar, nRepCnt, nFlags, 0);
			skipOnChar = TRUE;
		}
		CAKeyShortcuts = 0;
	}
	if (!skipOnChar) {
		if (cursorCnt >= 0) {
			POINT mPos;
			if (GetCursorPos(&mPos)) {
				RECT  wDim;
				RECT  cDim;

				GetWindowRect(&wDim);
				GetClientRect(&cDim);
				wDim.right = wDim.left + cDim.right;
				wDim.bottom = wDim.top + cDim.bottom;
				if ((wDim.left < mPos.x && wDim.right > mPos.x) &&
					(wDim.top < mPos.y && wDim.bottom > mPos.y)) {
					if (!isPlayS && isHideCursor)
						cursorCnt = ShowCursor(FALSE);

				}
			}
		} else if (!isHideCursor)
			cursorCnt = ShowCursor(TRUE);
		if (pDoc->FileInfo->isUTF) {
#ifdef _UNICODE
			call_win95_call(nChar, nRepCnt, nFlags, 0);
#else
			short orgEncod;
			int  i;
			unsigned long len;
			UINT toW[BUFSIZ];
			unsigned char toC[BUFSIZ];
			myFInfo *gdf = pDoc->FileInfo;

			orgEncod = GetEncode("Win95:", gdf->row_txt->Font.FName, WIN95cour, gdf->row_txt->Font.CharSet, FALSE);
			ANSIToUTF8((unsigned char *)&nChar, 1L, toC, &len, BUFSIZ, orgEncod);
			for (i=0L; i < len; i++)
				toW[i] = (UINT)toC[i];
			toW[i] = 0;
			for (i=0; toW[i] != 0; i++)
				call_win95_call(toW[i], nRepCnt, nFlags, 0);
#endif
		} else
			call_win95_call(nChar, nRepCnt, nFlags, 0);
	}
//	CView::OnChar(nChar, nRepCnt, nFlags);
}

char CClan2View::isExtendedKey(UINT nChar, UINT nRepCnt, UINT nFlags) {
	CClan2Doc* pDoc = GetDocument();

	// 1- `, 2- ~, 3- ', 4- ^, 5- @, 6- :, 7- ,, 8- &, 9- /
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
		if (isEqChar(nChar, bApKey) && isEqShift(bApKey)) {
			isExtKeyFound = 1;
		} else if (isEqChar(nChar, TildKey) && isEqShift(TildKey)) {
			isExtKeyFound = 2;
		} else if (isEqChar(nChar, ApKey) && isEqShift(ApKey)) {
			isExtKeyFound = 3;
		} else if (isEqChar(nChar, HatKey) && isEqShift(HatKey)) {
			isExtKeyFound = 4;
		} else if (isEqChar(nChar, AtKey) && isEqShift(AtKey)) {
			isExtKeyFound = 5;
		} else if (isEqChar(nChar, ColKey) && isEqShift(ColKey)) {
			isExtKeyFound = 6;
		} else if (isEqChar(nChar, ComaKey) && isEqShift(ComaKey)) {
			isExtKeyFound = 7;
		} else if (isEqChar(nChar, AndKey) && isEqShift(AndKey)) {
			isExtKeyFound = 8;
		}/* else if (isEqChar(nChar, SlashKey) && isEqShift(SlashKey)) {
			isExtKeyFound = 9;
		}*/
	}
	return(isExtKeyFound);
}

static UINT MakeExtendedKey(UINT nChar, char isExtKeyFound) {
	// 1- `, 2- ~, 3- ', 4- ^, 5- @, 6- :, 7- ,, 8- &, 9- /
	if (nChar == 'A') {
		if (isExtKeyFound == 1) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(192);
			else return(224);
		} else if (isExtKeyFound == 2) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(195);
			else return(227);
		} else if (isExtKeyFound == 3) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(193);
			else return(225);
		} else if (isExtKeyFound == 4) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(194);
			else return(226);
		} else if (isExtKeyFound == 5) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(197);
			else return(229);
		} else if (isExtKeyFound == 6) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(196);
			else return(228);
		} else if (isExtKeyFound == 8) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(198);
			else return(230);
		}
	} else if (nChar == 'C') {
		if (isExtKeyFound == 7) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(199);
			else return(231);
		}
	} else if (nChar == 'E') {
		if (isExtKeyFound == 1) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(200);
			else return(232);
		} else if (isExtKeyFound == 3) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(201);
			else return(233);
		} else if (isExtKeyFound == 4) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(202);
			else return(234);
		} else if (isExtKeyFound == 6) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(203);
			else return(235);
		}
	} else if (nChar == 'I') {
		if (isExtKeyFound == 1) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(204);
			else return(236);
		} else if (isExtKeyFound == 3) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(205);
			else return(237);
		} else if (isExtKeyFound == 4) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(206);
			else return(238);
		} else if (isExtKeyFound == 6) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(207);
			else return(239);
		}
	} else if (nChar == 'N') {
		if (isExtKeyFound == 2) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(209);
			else return(241);
		}
	} else if (nChar == 'O') {
		if (isExtKeyFound == 1) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(210);
			else return(242);
		} else if (isExtKeyFound == 2) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(213);
			else return(245);
		} else if (isExtKeyFound == 3) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(211);
			else return(243);
		} else if (isExtKeyFound == 4) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(212);
			else return(244);
		} else if (isExtKeyFound == 6) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(214);
			else return(246);
		}/* else if (isExtKeyFound == 9) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(216);
			else return(248);
		}*/
	} else if (nChar == 'S') {
		if (isExtKeyFound == 8) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(0);
			else return(223);
		}
	} else if (nChar == 'U') {
		if (isExtKeyFound == 1) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(217);
			else return(249);
		} else if (isExtKeyFound == 3) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(218);
			else return(250);
		} else if (isExtKeyFound == 4) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(219);
			else return(251);
		} else if (isExtKeyFound == 6) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(220);
			else return(252);
		}
	} else if (nChar == 'Y') {
		if (isExtKeyFound == 3) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(221);
			else return(253);
		} else if (isExtKeyFound == 6) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return(159);
			else return(255);
		}
	}
	return(0);
}

void CClan2View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{

	if (CAKeyShortcuts == 1) {
		wchar_t CAChar;

		if (GetAsyncKeyState(VK_CONTROL) || !(GetAsyncKeyState(VK_SHIFT) & 0x8000))
 			CAKeyShortcuts = 2;
		if (HandleCAKeyShortcuts(nChar, true, &CAChar)) {
			call_win95_call(CAChar, nRepCnt, nFlags, 0);
			skipOnChar = TRUE;
			CAKeyShortcuts = 0;
		}
		return;
	} else
		CAKeyShortcuts = 0;

	if (isExtKeyFound && nChar != VK_SHIFT && nChar != VK_CONTROL) {
		if ((nChar=MakeExtendedKey(nChar, isExtKeyFound))) {
			skipOnChar = TRUE;
			call_win95_call(nChar, nRepCnt, nFlags, 0);
		}
		isExtKeyFound = 0; // 1- `, 2- ~, 3- ', 4- ^, 5- @, 6- :, 7- ,, 8- &, 9- /
		return;
	} else if (isExtendedKey(nChar, nRepCnt, nFlags)) {
		return;
	}

	if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->isPlaying && 
			nChar != VK_CONTROL && nChar != VK_SHIFT) {
		if (PlayingContMovie || !PBC.walk || !PBC.enable) {
			stopMoviePlaying();
			PlayingContMovie = FALSE;
			skipOnChar = TRUE;
			return;
		}
	}

	if (nChar >= VK_F1 && nChar <= VK_F12) {
		walker_pause = 0L;
		if (PBC.walk) {
			CClientDC dc(this);
			CDC* tGlobalDC = GlobalDC;
			CClan2Doc* pDoc = GetDocument();
			ASSERT_VALID(pDoc);

			GlobalDC = &dc;
			GlobalDoc = pDoc;
			global_df = pDoc->FileInfo;

			if (nChar == VK_F7 || nChar == VK_F9)
				;
			else if (MovDlg == NULL && MPegDlg == NULL) {
				if (global_df->SnTr.isMP3 == TRUE)
					stopMP3SoundIfPlaying();
				else
					stopSoundIfPlaying();
			}
			if (global_df) {
				if (nChar != VK_F7 && nChar != VK_F9) {
					strcpy(global_df->err_message, "-Aborted.");
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
					if (MovDlg != NULL && MovDlg->qt != NULL) {
						if ((nChar == VK_F5 || nChar == VK_F6) &&
							(MovDlg->qt)->isPlaying && PBC.walk && PBC.enable) {
							stopMoviePlaying();
							PlayingContMovie = FALSE;
							skipOnChar = TRUE;
							finishWalk(global_df);
							return;
						} else
							finishWalk(global_df);
					} else {
						PBC.walk = 0;
						PBC.isPC = 0;
					}

				}
				SetPBCglobal_df(false, 0L);
			}

			GlobalDC = tGlobalDC;
			if (nChar == VK_F7 || nChar == VK_F9) {
				F_key = nChar-VK_F1+1;
				nChar = 1;
				if (!skipOnChar)
					call_win95_call(nChar, nRepCnt, nFlags, 0);
				F_key = 0;
			}
		} else if (nChar == VK_F1 && isUseSPCKeyShortcuts) {
			CClientDC dc(this);
			CDC* tGlobalDC = GlobalDC;
			CClan2Doc* pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			BOOL EditorMode;

			GlobalDC = &dc;
			GlobalDoc = pDoc;
			global_df = pDoc->FileInfo;
			if (global_df != NULL)
				EditorMode = global_df->EditorMode;
			else
				EditorMode = FALSE;

			if (EditorMode) {
				CAKeyShortcuts = 1;
			} else {
				F_key = nChar-VK_F1+1;
				nChar = 1;
				if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
					nChar += 12;
				if (!skipOnChar)
					call_win95_call(nChar, nRepCnt, nFlags, 0);
				F_key = 0;
			}
			GlobalDC = tGlobalDC;
		} else {
			F_key = nChar-VK_F1+1;
			nChar = 1;
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				nChar += 12;
			if (!skipOnChar)
				call_win95_call(nChar, nRepCnt, nFlags, 0);
			F_key = 0;
		}
	} else {
		UINT extended = 0;

		if (nChar == 0xBF && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			OnToggleMovieText();
			return;
		} else if (nChar == VK_LEFT) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				extended = 1;
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				if (extended)
					isPlayS = 93;
				else
					isPlayS = 95;	
				extended = 0;
			} else
				isPlayS = 19;	
		} else if (nChar == VK_RIGHT) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				extended = 1;
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				if (extended)
					isPlayS = 92;
				else
					isPlayS = 94;	
				extended = 0;
			} else
				isPlayS = 18;
		} else if (nChar == VK_UP) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				extended = 1;
			isPlayS = 17;
		} else if (nChar == VK_DOWN) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				extended = 1;
			isPlayS = 16;
		} else if (nChar == VK_END) {
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				isPlayS = 45;
			else
				isPlayS = 29;
		} else if (nChar == VK_HOME) {
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				isPlayS = 44;
			else
				isPlayS = 28;
		} else if (nChar == VK_PRIOR)
			isPlayS = 31;
		else if (nChar == VK_NEXT)
			isPlayS = 30;
		else if (nChar == VK_INSERT)
			nChar = 15;
		else if (nChar == VK_DELETE)
			isPlayS = 32;
		else if (nChar == VK_CONTROL || nChar == VK_SHIFT) {
			isPlayS = 0;
			return;
		} else
			isPlayS = 0;

		if (isPlayS != 0) {
			call_win95_call(nChar, nRepCnt, nFlags, extended);
		} else
			CView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}


void CClan2View::OnPlaybackControl() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;

	if (global_df)
		SetPBCglobal_df(false, 0L);
	if (PBCDlg == NULL) {
		PBCDlg = new CPlaybackControler(AfxGetApp()->m_pMainWnd);
	}

	GlobalDC = tGlobalDC;
}

void CClan2View::OnFileSaveClip() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	SaveSoundClip(&global_df->SnTr, TRUE);
	SetScrollControl();
	DrawCursor(1);
	GlobalDC = tGlobalDC;
}

#include <mmsystem.h>
#include <mmreg.h>

BOOL CClan2View::PreTranslateMessage(MSG* pMsg) 
{
	extern int		sndCom;
	extern Boolean	isMP3SoundDone;
	extern HWAVEOUT hWaveOut;

	if (pMsg->message == WM_KEYDOWN) {
		if (skipOnChar)
			skipOnChar = FALSE;
	}
	if (!PlayingContSound && PBC.enable && MovDlg == NULL && MPegDlg == NULL) {
		if (walker_pause != 0L) {
			DWORD t;
			if ((t=GetTickCount()) > walker_pause) {
				walker_pause = 0L;
				PlayBuffer(sndCom);
			}
		}
		if (hWaveOut != NULL) {
			CClientDC dc(this);
			CDC* tGlobalDC = GlobalDC;
			CClan2Doc* pDoc = GetDocument();
			ASSERT_VALID(pDoc);

			GlobalDC = &dc;
			GlobalDoc = pDoc;
			global_df = pDoc->FileInfo;

			skipOnChar = checkPCMSound(pMsg, skipOnChar);
			checkPCMPlaybackBuffer();

			GlobalDC = tGlobalDC;
		}

		if (!isMP3SoundDone) {
			CClientDC dc(this);
			CDC* tGlobalDC = GlobalDC;
			CClan2Doc* pDoc = GetDocument();
			ASSERT_VALID(pDoc);

			GlobalDC = &dc;
			GlobalDoc = pDoc;
			global_df = pDoc->FileInfo;

			skipOnChar = checkMP3Sound(pMsg, skipOnChar);

			GlobalDC = tGlobalDC;
		}
	}
	return CView::PreTranslateMessage(pMsg);
}

LRESULT CClan2View::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
/*
	TCHAR chCharCode; 
	UINT lKeyData;

	if (message == WM_SYSKEYDOWN) {  // ALT-key
		chCharCode = (TCHAR) wParam; // virtual-key code 
		lKeyData = lParam;			 // key data 
// if chCharCode == 18  then ALT key was pressed
// if chCharCode == 121 then F10 key was pressed
	}
	if (message == WM_SYSCHAR) { // doesn't seem to work
		chCharCode = (TCHAR) wParam; // virtual-key code 
		lKeyData = lParam;			 // key data 
	}
	if (message == WM_KEYDOWN) {
		chCharCode = (TCHAR) wParam;
		lKeyData = lParam;
	}
*/
	return CView::WindowProc(message, wParam, lParam);
}

void CClan2View::OnModeSendToPraat() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	getTextAndSendToSoundAnalyzer();
	SetScrollControl();
	DrawCursor(1);
	draw_mid_wm();
	GlobalDC = tGlobalDC;
}
