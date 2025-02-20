// Clan2View.cpp : implementation of the CClan2View class
//
#include "cu.h"
#include "ced.h"
#include "c_clan.h"
#include "search.h"
#include "MMedia.h"
#include "ids.h"

#include "Clan2Doc.h"
#include "Clan2View.h"
#include "CedDlgs.h"
#include "W95_commands.h"
#include "W95_Unzip.h"
#include "MpegDlg.h"
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
extern float scalingSize;
extern FONTINFO cmdFnt;

extern void stopMovieIfPlaying(void);

char tComWinDataChanged;
CFont m_font;
CDC* GlobalDC;
CClan2Doc* GlobalDoc;
LOGFONT m_lfDefFont;
static LOGFONT m_lfComDefFont;
DWORD	walker_pause = 0L;

/////////////////////////////////////////////////////////////////////////////
// CClan2View

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
	ON_COMMAND(ID_FILE_SELECTMEDIAFILE, OnSelectMediaFile)

	ON_COMMAND(ID_GETMORGRAMMAR_ENGLISH, OnGetEnglishGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_CANTONESE, OnGetCantoneseGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_CHINESE, OnGetChineseGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_DANISH, OnGetDanishGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_DUTCH, OnGetDutchGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_FRENCH, OnGetFrenchGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_GERMAN, OnGetGermanGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_HEBREW, OnGetHebrewGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_ITALIAN, OnGetItalianGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_JAPANESE, OnGetJapaneseGrammar)
	ON_COMMAND(ID_GETMORGRAMMAR_SPANISH, OnGetSpanishGrammar)

	ON_COMMAND(ID_GETKIDEVALDATABASE_ENGLISH, OnGetEnglishKevalDB)
	ON_COMMAND(ID_GETKIDEVALDATABASE_CHINESE, OnGetChineseKevalDB)
	
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
	ON_COMMAND(ID_MODE_HIDE_TIERS, OnModeHideTiers)
	ON_COMMAND(ID_MODE_EXPEND_BULLETS, OnModeExpendBullets)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_UNDERLINE, OnUnderline)
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
// 18-07-2008	ON_COMMAND(ID_COLOR_WORDS, OnColorWords)
	ON_COMMAND(ID_COLOR_KEYWORDS, OnColorKeywords)
	ON_COMMAND(ID_TOGGLE_MOVIE_TEXT, OnToggleMovieText)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(ID_TIERS_IDHEADERS, &CClan2View::OnTiersIdheaders)
	ON_COMMAND(ID_MODE_PLAYCONTSKIP, &CClan2View::OnModePlaycontskip)
	//}}AFX_MSG_MAP
	// Standard printing commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Static initialization/termination

//static TCHAR szSettings[] = _T("Settings");
//static TCHAR szTabStops[] = _T("TabStops");
static TCHAR szFont[] = _T("Font");
static TCHAR szComandFont[] = _T("CommandFont");

static TCHAR szHeight[] = _T("Height");
static TCHAR szWeight[] = _T("Weight");
static TCHAR szItalic[] = _T("Italic");
static TCHAR szUnderline[] = _T("Underline");
static TCHAR szPitchAndFamily[] = _T("PitchAndFamily");
static TCHAR szCharSet[] = _T("CharSet");
static TCHAR szFaceName[] = _T("FaceName");
static TCHAR szSystem[] = _T("System");
//static TCHAR szWordWrap[] = _T("WordWrap");

static BOOL GetProfileFont(LOGFONT* plf, LOGFONT* plCf) {
	int FNS;
	CString strFont, strCFont;

//	return(FALSE);  // lxslxslxs

	CWinApp* pApp = AfxGetApp();
	plf->lfHeight = pApp->GetProfileInt(szFont, szHeight, 0);
	if (plf->lfHeight != 0) {
		plf->lfWeight = pApp->GetProfileInt(szFont, szWeight, 0);
		plf->lfItalic = (BYTE)pApp->GetProfileInt(szFont, szItalic, 0);
		plf->lfUnderline = (BYTE)pApp->GetProfileInt(szFont, szUnderline, 0);
		plf->lfPitchAndFamily = (BYTE)pApp->GetProfileInt(szFont, szPitchAndFamily, 0);
		plf->lfCharSet = (BYTE)pApp->GetProfileInt(szFont, szCharSet, DEFAULT_CHARSET);
		strFont = pApp->GetProfileString(szFont, szFaceName, szSystem);
		FNS = sizeof(plf->lfFaceName);
		lstrcpyn(plf->lfFaceName, strFont, FNS);
		plf->lfFaceName[FNS-1] = 0;

		plCf->lfHeight = pApp->GetProfileInt(szComandFont, szHeight, 0);
		if (plCf->lfHeight != 0) {
			plCf->lfWeight = pApp->GetProfileInt(szComandFont, szWeight, 0);
			plCf->lfItalic = (BYTE)pApp->GetProfileInt(szComandFont, szItalic, 0);
			plCf->lfUnderline = (BYTE)pApp->GetProfileInt(szComandFont, szUnderline, 0);
			plCf->lfPitchAndFamily = (BYTE)pApp->GetProfileInt(szComandFont, szPitchAndFamily, 0);
			plCf->lfCharSet = (BYTE)pApp->GetProfileInt(szComandFont, szCharSet, DEFAULT_CHARSET);
			strCFont = pApp->GetProfileString(szComandFont, szFaceName, szSystem);
			FNS = sizeof(plCf->lfFaceName);
			lstrcpyn(plCf->lfFaceName, strCFont, FNS);
			plCf->lfFaceName[FNS - 1] = 0;
		} else {
			plCf->lfHeight = plf->lfHeight;
			plCf->lfWeight = plf->lfWeight;
			plCf->lfItalic = plf->lfItalic;
			plCf->lfUnderline = plf->lfUnderline;
			plCf->lfPitchAndFamily = plf->lfPitchAndFamily;
			plCf->lfCharSet = plf->lfCharSet;
			lstrcpyn(plCf->lfFaceName, strFont, FNS);
			plCf->lfFaceName[FNS - 1] = 0;
		}
		return(TRUE);
	} else
		return(FALSE);
}

static void WriteProfileFont(const LOGFONT* plf, LOGFONT* plCf)
{
	CWinApp* pApp = AfxGetApp();

	if (plf->lfHeight != 0) {
		pApp->WriteProfileInt(szFont, szHeight, plf->lfHeight);
		pApp->WriteProfileInt(szFont, szWeight, plf->lfWeight);
		pApp->WriteProfileInt(szFont, szItalic, plf->lfItalic);
		pApp->WriteProfileInt(szFont, szUnderline, plf->lfUnderline);
		pApp->WriteProfileInt(szFont, szPitchAndFamily, plf->lfPitchAndFamily);
		pApp->WriteProfileInt(szFont, szCharSet, plf->lfCharSet);
		pApp->WriteProfileString(szFont, szFaceName, (LPCTSTR)plf->lfFaceName);
	}
	if (plCf->lfHeight != 0) {
		pApp->WriteProfileInt(szComandFont, szHeight, plCf->lfHeight);
		pApp->WriteProfileInt(szComandFont, szWeight, plCf->lfWeight);
		pApp->WriteProfileInt(szComandFont, szItalic, plCf->lfItalic);
		pApp->WriteProfileInt(szComandFont, szUnderline, plCf->lfUnderline);
		pApp->WriteProfileInt(szComandFont, szPitchAndFamily, plCf->lfPitchAndFamily);
		pApp->WriteProfileInt(szComandFont, szCharSet, plCf->lfCharSet);
		pApp->WriteProfileString(szComandFont, szFaceName, (LPCTSTR)plCf->lfFaceName);
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
		u_strcpy(lfFont->lfFaceName, fontInfo->FName, LF_FACESIZE);
	} else if (finfo != NULL) {
		lfFont->lfHeight = finfo->fontSize;
		u_strcpy(lfFont->lfFaceName, finfo->fontName, LF_FACESIZE);
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
	extern short ArialUnicodeFOND, SecondDefUniFOND;
	float tf;
	long size;
	CDC* tGlobalDC = GlobalDC;

	LOGFONT lfFont;
	unCH fontNameU[256];
	NewFontInfo finfo;

	if (!firstFontTest)
		return;
	firstFontTest = FALSE;
	strcpy(defUniFontName, "Arial Unicode MS"); // lxslxslxs
	tf = -14;
	tf = tf * scalingSize;
	size = (long)tf;

	defUniFontSize = size;
	memset(&lfFont, 0, sizeof(LOGFONT));
	lfFont.lfCharSet = (unsigned char)0;
	lfFont.lfOutPrecision = OUT_TT_PRECIS;
	lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfFont.lfQuality = PROOF_QUALITY;
	lfFont.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH; //FF_SWISS | VARIABLE_PITCH;
	lfFont.lfHeight = defUniFontSize;
	u_strcpy(lfFont.lfFaceName, defUniFontName, LF_FACESIZE);
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
		strcpy(defUniFontName, "Cambria");
		memset(&lfFont, 0, sizeof(LOGFONT));
		lfFont.lfCharSet = (unsigned char)0;
		lfFont.lfOutPrecision = OUT_TT_PRECIS;
		lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lfFont.lfQuality = PROOF_QUALITY;
		lfFont.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH; //FF_SWISS | VARIABLE_PITCH;
		lfFont.lfHeight = defUniFontSize;
		u_strcpy(lfFont.lfFaceName, defUniFontName, LF_FACESIZE);
		lfFont.lfWeight = FW_NORMAL;

		strcpy(finfo.fontName, defUniFontName);
		finfo.fontSize = defUniFontSize;
		finfo.fontId = DEFAULT_ID;
		finfo.fontPref = "Win95:";
		finfo.orgFType = WINArialUC;
		finfo.fontType = WINArialUC;
		finfo.CharSet = 0;
		if (EnumFontFamiliesEx(GlobalDC->GetSafeHdc(), &lfFont, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&finfo, 0) != 0) {
			u_strcpy(fontNameU, defUniFontName, 256);
			if (EnumFontFamilies(GlobalDC->GetSafeHdc(), fontNameU, (FONTENUMPROC)EnumFontFamProc, (LPARAM)&finfo) != 0) {
				ArialUnicodeFOND = 0;
			}
		}
	}

	if (ArialUnicodeFOND == 0) {
		strcpy(defUniFontName, "CAfont"/*UNICODEFONT*/); // lxs font lxslxs
		defUniFontSize = size;
		memset(&lfFont, 0, sizeof(LOGFONT));
		lfFont.lfCharSet = (unsigned char)0;
		lfFont.lfOutPrecision = OUT_TT_PRECIS;
		lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lfFont.lfQuality = PROOF_QUALITY;
		lfFont.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH; //FF_SWISS | VARIABLE_PITCH;
		lfFont.lfHeight = defUniFontSize;
		u_strcpy(lfFont.lfFaceName, defUniFontName, LF_FACESIZE);
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
			u_strcpy(fontNameU, defUniFontName, 256);
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
		lfFont.lfHeight = size;
		u_strcpy(lfFont.lfFaceName, "CAfont", LF_FACESIZE);
		lfFont.lfWeight = FW_NORMAL;

		strcpy(finfo.fontName, "CAfont");
		finfo.fontSize = size;
		finfo.fontId = DEFAULT_ID;
		finfo.fontPref = "Win95:";
		finfo.orgFType = WINCAFont;
		finfo.fontType = WINCAFont;
		finfo.CharSet = 0;
		
		SecondDefUniFOND = 1;
		if (EnumFontFamiliesEx(GlobalDC->GetSafeHdc(),&lfFont,(FONTENUMPROC)EnumFontFamExProc,(LPARAM)&finfo,0) != 0) {
			u_strcpy(fontNameU, defUniFontName, 256);
			if (EnumFontFamilies(GlobalDC->GetSafeHdc(),fontNameU,(FONTENUMPROC)EnumFontFamProc,(LPARAM)&finfo) != 0) {
				SecondDefUniFOND = 0;
			}
		}
	}
	GlobalDC = tGlobalDC;
	if (ArialUnicodeFOND == 0 && SecondDefUniFOND == 0)
		do_warning("Can't find either \"Cambria\" or \"TITUS Cyberbit Basic\" font. Please look on web page \"http://talkbank.org/\" for more information.", 0);
	else {
		SetDefaultUnicodeFinfo(&finfo);
		if (strcmp(m_lfDefFont.lfFaceName, "Arial Unicode MS") && strcmp(m_lfDefFont.lfFaceName, "Cambria") && strcmp(m_lfDefFont.lfFaceName, "CAfont"))
			SetLogfont(&m_lfDefFont, NULL, &finfo);
		else if ((!strcmp(m_lfDefFont.lfFaceName, "Arial Unicode MS") || !strcmp(m_lfDefFont.lfFaceName, "Cambria")) &&
			defUniFontName[0] != 'A' && defUniFontName[0] != 'C')
			SetLogfont(&m_lfDefFont, NULL, &finfo);
		else if (!strcmp(m_lfDefFont.lfFaceName, "CAfont") && defUniFontName[0] == 'A')
			SetLogfont(&m_lfDefFont, NULL, &finfo);

		m_font.DeleteObject();
		if (m_font.CreateFontIndirect(&m_lfDefFont) == 0) {
			::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_lfDefFont);
			m_lfDefFont.lfWeight = FW_NORMAL;
			m_font.CreateFontIndirect(&m_lfDefFont);
		}
		u_strcpy(dFnt.fontName, m_lfDefFont.lfFaceName, 250);
		dFnt.isUTF = 1;
		dFnt.fontSize = m_lfDefFont.lfHeight;
		dFnt.orgFType = getFontType(dFnt.fontName, TRUE);
		dFnt.fontType = dFnt.orgFType;
		dFnt.CharSet = DEFAULT_CHARSET;
		dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
	}
}

/*
static void GetFileVersion(DWORD *major, DWORD *minor) {
	DWORD               dwSize = 0;
	BYTE                *pVersionInfo = NULL;
	VS_FIXEDFILEINFO    *fInfo = NULL;
	UINT                pLenFileInfo = 0;
	unCH				inFName[FNSize];

	*major = 0;
	*minor = 0;
	u_strcpy(inFName, "kernel32.dll", FNSize);
	// getting the file version info size 
	dwSize = GetFileVersionInfoSize(inFName, NULL);
	if (dwSize == 0) {
		return;
	}
	pVersionInfo = new BYTE[dwSize]; //allocation of space for the verison size
	if (!GetFileVersionInfo(inFName, 0, dwSize, pVersionInfo))  {
		delete[] pVersionInfo;
		return;
	}
	if (!VerQueryValue(pVersionInfo, TEXT("\\"), (LPVOID*)&fInfo, &pLenFileInfo)) {
		delete[] pVersionInfo;
		return;
	}
	*major = (fInfo->dwFileVersionMS >> 16) & 0xffff;
	*minor = (fInfo->dwFileVersionMS) & 0xffff;
	sprintf(templineC, "major=%d, minor=%d, %d.%d", *major, *minor, (fInfo->dwFileVersionLS >> 16) & 0xffff, (fInfo->dwFileVersionLS >> 0) & 0xffff);
	do_warning(templineC, 0);
	sprintf(templineC, "File Version: %d.%d.%d.%d\n",
		(fInfo->dwFileVersionLS >> 24) & 0xff,
		(fInfo->dwFileVersionLS >> 16) & 0xff,
		(fInfo->dwFileVersionLS >> 8) & 0xff,
		(fInfo->dwFileVersionLS >> 0) & 0xff
		);
	do_warning(templineC, 0);
	sprintf(templineC, "Product Version: %d.%d.%d.%d\n",
		(fInfo->dwProductVersionLS >> 24) & 0xff,
		(fInfo->dwProductVersionLS >> 16) & 0xff,
		(fInfo->dwProductVersionLS >> 8) & 0xff,
		(fInfo->dwProductVersionLS >> 0) & 0xff
		);
	do_warning(templineC, 0);
	delete[] pVersionInfo;
}
*/

void CClan2View::Initialize()
{
	float tf;
	long size;
	unCH fontNameU[256];
	NewFontInfo finfo;
	HDC     hDC;
	BOOL	isFontSet;

//	GetFileVersion(&major, &minor); {// lxslxslxs

	isFontSet = FALSE;
	hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	if (GetProfileFont(&m_lfDefFont, &m_lfComDefFont)) {
		if (hDC != NULL) {
			u_strcpy(finfo.fontName, m_lfDefFont.lfFaceName, 250);
			finfo.fontSize = m_lfDefFont.lfHeight;
			finfo.fontId = DEFAULT_ID;
			finfo.fontPref = "Win95:";
			finfo.orgFType = WINArialUC;
			finfo.fontType = WINArialUC;
			finfo.CharSet = 0;
			u_strcpy(fontNameU, finfo.fontName, 256);
			if (EnumFontFamilies(hDC, fontNameU, (FONTENUMPROC)EnumFontFamProc, (LPARAM)&finfo) == 0) {
				isFontSet = TRUE;
				SetLogfont(&m_lfDefFont, NULL, &finfo);
				firstFontTest = FALSE;
			}
			u_strcpy(finfo.fontName, m_lfComDefFont.lfFaceName, 250);
			finfo.fontSize = m_lfComDefFont.lfHeight;
			finfo.fontId = DEFAULT_ID;
			finfo.fontPref = "Win95:";
			finfo.orgFType = WINArialUC;
			finfo.fontType = WINArialUC;
			finfo.CharSet = 0;
			u_strcpy(fontNameU, finfo.fontName, 256);
			if (EnumFontFamilies(hDC, fontNameU, (FONTENUMPROC)EnumFontFamProc, (LPARAM)&finfo) == 0) {
				isFontSet = TRUE;
				SetLogfont(&m_lfComDefFont, NULL, &finfo);
			}
		}
	}

	if (!isFontSet) {
		tf = -14;
		tf = tf * scalingSize;
		size = (long)tf;

		if (hDC != NULL) {
			firstFontTest = FALSE;
			strcpy(finfo.fontName, "Arial Unicode MS");
			finfo.fontSize = size;
			finfo.fontId = DEFAULT_ID;
			finfo.fontPref = "Win95:";
			finfo.orgFType = WINArialUC;
			finfo.fontType = WINArialUC;
			finfo.CharSet = 0;
			u_strcpy(fontNameU, finfo.fontName, 256);
			if (EnumFontFamilies(hDC, fontNameU, (FONTENUMPROC)EnumFontFamProc, (LPARAM)&finfo) != 0) {
				strcpy(finfo.fontName, "Cambria");
				u_strcpy(fontNameU, "Cambria", 256);
				if (EnumFontFamilies(hDC, fontNameU, (FONTENUMPROC)EnumFontFamProc, (LPARAM)&finfo) != 0) {
					::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_lfDefFont);
					m_lfDefFont.lfWeight = FW_NORMAL;
				} else {
					SetLogfont(&m_lfDefFont, NULL, &finfo);
					SetLogfont(&m_lfComDefFont, NULL, &finfo);
				}
			} else {
				SetLogfont(&m_lfDefFont, NULL, &finfo);
				SetLogfont(&m_lfComDefFont, NULL, &finfo);
			}
		} else {
			firstFontTest = FALSE;
			strcpy(finfo.fontName, "Cambria");
			finfo.fontSize = size;
			finfo.fontId = DEFAULT_ID;
			finfo.fontPref = "Win95:";
			finfo.orgFType = WINArialUC;
			finfo.fontType = WINArialUC;
			finfo.CharSet = 0;
			SetLogfont(&m_lfDefFont, NULL, &finfo);
			SetLogfont(&m_lfComDefFont, NULL, &finfo);
		}
	}
	u_strcpy(defUniFontName, m_lfDefFont.lfFaceName, 250);
	defUniFontSize = m_lfDefFont.lfHeight;
	stickyFontSize = defUniFontSize;
	if (strcmp(m_lfDefFont.lfFaceName, "Arial Unicode MS") && strcmp(m_lfDefFont.lfFaceName, "Cambria") && strcmp(m_lfDefFont.lfFaceName, "CAfont")) {
		NewFontInfo finfo;

		SetDefaultUnicodeFinfo(&finfo);
		SetLogfont(&m_lfDefFont, NULL, &finfo);
		if (!isFontSet) {
			SetLogfont(&m_lfComDefFont, NULL, &finfo);
		}
		if (m_font.CreateFontIndirect(&m_lfDefFont) != 0) {
			goto FinInit;
		} else {
			::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_lfDefFont);
			m_lfDefFont.lfWeight = FW_NORMAL;
			if (!isFontSet) {
				::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_lfComDefFont);
				m_lfComDefFont.lfWeight = FW_NORMAL;
			}
		}
	}

	m_font.CreateFontIndirect(&m_lfDefFont);
FinInit:
	if (hDC != NULL)
		DeleteDC(hDC);
	u_strcpy(dFnt.fontName, m_lfDefFont.lfFaceName, 250);
	dFnt.isUTF = 1;
	dFnt.fontSize = m_lfDefFont.lfHeight;
	dFnt.orgFType = getFontType(dFnt.fontName, TRUE);
	dFnt.fontType = dFnt.orgFType;
	dFnt.CharSet = DEFAULT_CHARSET;
	dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
// 2016-03-29
	u_strcpy(cmdFnt.FName, m_lfComDefFont.lfFaceName, LF_FACESIZE);
	cmdFnt.FSize = m_lfComDefFont.lfHeight;
	cmdFnt.CharSet = DEFAULT_CHARSET;
	cmdFnt.Encod = my_FontToScript(cmdFnt.FName, cmdFnt.CharSet);
}

void CClan2View::Terminate()
{
	WriteProfileFont(&m_lfDefFont, &m_lfComDefFont);
}

/////////////////////////////////////////////////////////////////////////////
// CClan2View construction/destruction

CClan2View::CClan2View()
{
	isExtKeyFound = 0; 	// 1- `, 2- ~, 3- ', 4- ^, 5- @, 6- :, 7- ,, 8- &, 9- /
	text_isFKeyPressed = 0;
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

static webMFile *FindMediaFileName(unCH *line, int *oIndex, char *mvFName, char *path, unCH *URL, webMFile *webMFiles, FNType *tMediaFName) {
	int i, orgIndex, index;
	char *s, isPict;
	char buf[FILENAME_MAX];
	unCH wFilePathName[FNSize];
	unCH	fname[FNSize];
	webMFile *p;
	extern bool curlURLDownloadToFile(unCH *fulURLPath, unCH *fname, size_t isProgres);

	index = *oIndex;
	orgIndex = index;
	isPict = FALSE;
	for (i=0, index=orgIndex+1; line[index]; i++, index++) {
		if (line[index] != PICTTIER[i])
			break;
	}
	if (PICTTIER[i] != EOS) {
		if (tMediaFName[0] == EOS) {
			for (i=0, index=orgIndex+1; line[index]; i++, index++) {
				if (line[index] != SOUNDTIER[i])
					break;
			}
			if (SOUNDTIER[i] != EOS) {
				for (i=0, index=orgIndex+1; line[index]; i++, index++) {
					if (line[index] != REMOVEMOVIETAG[i])
						break;
				}
				if (REMOVEMOVIETAG[i] != EOS) {
					*oIndex = orgIndex;
					return(webMFiles);
				}
			}
		}
	} else
		isPict = TRUE;

	if (isPict || tMediaFName[0] == EOS) {
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
			wFilePathName[i++] = line[index];
		wFilePathName[i] = EOS;
		u_strcpy(buf, wFilePathName, FILENAME_MAX);
	} else if (tMediaFName[0] != EOS) {
		strcpy(buf, tMediaFName);
		u_strcpy(wFilePathName, buf, FNSize);
	}

	if (uS.mStricmp(buf, mvFName) != 0) {
		i = strlen(URL);
		strcat(URL, "media/");
		s = strrchr(buf, '.');
		strcat(URL, wFilePathName);
		if (s == NULL) {
			if (isPict)
				strcat(URL, ".jpg");
			else
				strcat(URL, ".mov");
		}
		u_strcpy(fname, path, FNSize);
		strcat(fname, wFilePathName);
		if (s == NULL) {
			if (isPict)
				strcat(fname, ".jpg");
			else
				strcat(fname, ".mov");
		}

		if (curlURLDownloadToFile(URL, fname, 0L) != true)
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
				if (s == NULL)
					p->name = (FNType *)malloc((strlen(buf)+5)*sizeof(FNType));
				else
					p->name = (FNType *)malloc((strlen(buf)+1)*sizeof(FNType));
				if (p->name != NULL) {
					uS.str2FNType(p->name, 0L, buf);
					if (s == NULL) {
						if (isPict)
							strcat(p->name, ".jpg");
						else
							strcat(p->name, ".mov");
					}
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
	FNType tMediaFName[FILENAME_MAX];
	ROWS *curRow;
	
	tMediaFName[0] = EOS;
	mvFName[0] = EOS;
	curRow = df->head_text->next_row;
	while (!AtBotEnd(curRow, df->tail_text, FALSE)) {
		if (uS.partcmp(curRow->line, MEDIAHEADER, FALSE, FALSE)) {
			u_strcpy(tMediaFName, curRow->line+strlen(MEDIAHEADER), FILENAME_MAX-1);
			tMediaFName[FILENAME_MAX-1] = EOS;
			getMediaName(NULL, tMediaFName, FILENAME_MAX);
		}
		for (i=0; curRow->line[i]; i++) {
			if (curRow->line[i] == HIDEN_C) {
				webMFiles = FindMediaFileName(curRow->line, &i, mvFName, path, URL, webMFiles, tMediaFName);
			}
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
	int  i, len;
	int  num;
	char qt = 0, t;

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
			len = strlen(bArgv[num]);
			uS.shiftright(com + i, len);
			t = com[i+len];
			strncpy(com + i, bArgv[num], strlen(bArgv[num]));
			com[i+len] = t;
			i = i + strlen(bArgv[num]) - 1;
		} else if (qt == 0 && com[i] == '-' && com[i + 1] == '%' && isdigit(com[i + 2]) && !isdigit(com[i + 3])) {
			num = atoi(com + i + 2) - 1;
			if (num < 0 || num > 9) {
				sprintf(com, "Argument %%%d was not specified with batch file \"%s\".", num + 1, fname);
				do_warning(com, 0);
				return(FALSE);
			}
			strcpy(com + i, com + i + 3);
			if (bArgv[num] != NULL) {
				len = strlen(bArgv[num]);
				uS.shiftright(com + i, len);
				t = com[i + len];
				strncpy(com + i, bArgv[num], strlen(bArgv[num]));
				com[i + len] = t;
				i = i + strlen(bArgv[num]) - 1;
			}
			else
				i--;
		}
	}
	uS.remblanks(com);
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
		if (ext != NULL && true /*(strcmp(ext, ".mpg") == 0  ||
								   strcmp(ext, ".mpeg") == 0 ||
								   strcmp(ext, ".aiff") == 0 ||
								   strcmp(ext, ".aif") == 0  ||
								   strcmp(ext, ".avi") == 0  ||
								   strcmp(ext, ".mov") == 0  ||
								   strcmp(ext, ".m4v") == 0  ||
								   strcmp(ext, ".wmv") == 0  ||
								   strcmp(ext, ".mp4") == 0)*/) {
			if (MpegDlg != NULL) {
				MpegDlg->SetActiveWindow();
				pDoc->SetFakeHilight = 1;
				pDoc->myUpdateAllViews(FALSE);
			}
/* // NO QT
			if (MovDlg != NULL) {
				MovDlg->SetActiveWindow();
				pDoc->SetFakeHilight = 1;
				pDoc->myUpdateAllViews(FALSE);
			}
*/
		}
		global_df = tGlobal_df;
	}
}

static void finishWalk(myFInfo *df) {
	SInt32 pos = 0;

	if (df != NULL) {
/* // NO QT
		if (MovDlg != NULL)
			pos = (MovDlg->qt)->GetCurTime();
		else
*/
		if (MpegDlg != NULL)
			pos = (MpegDlg->mpeg)->GetCurTime();
		else
			return;
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
	char newlyCreated;
	CRect cRect;
	CDC* tGlobalDC;
	myFInfo	*tGlobal_df;
	CClan2Doc* pDoc = GetDocument();
	char commands_win_name[512];
	extern char ced_version[];
	extern char DefClan;
	extern char firstTimeStart;
	extern char *clanBuf, clanRun, quickTranscribe;
	extern char *lineNumFname;
	extern char isSpOverride;
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
	testPresenceOfUnicodeFont(pDC);
	if (pDoc->re_colorLemmas && pDoc->FileInfo != NULL) {
		pDoc->re_colorLemmas = FALSE;
		global_df = pDoc->FileInfo;
		global_df->re_colorLemmas = TRUE;
		global_df->WinChange = FALSE;
		DisplayTextWindow(NULL, 0);
		touchwin(global_df->w1);
		wrefresh(global_df->w1);
		global_df->WinChange = TRUE;
		global_df->re_colorLemmas = FALSE;
	}
	if (pDoc->re_wrap && pDoc->FileInfo != NULL) {
		global_df = pDoc->FileInfo;
		Re_WrapLines(AddLineToRow, 0L, TRUE, NULL);
		pDoc->re_wrap = FALSE;
	}
	if (pDoc->redo_soundwave && pDoc->FileInfo != NULL) {
		global_df = pDoc->FileInfo;
		if (global_df->SnTr.IsSoundOn) {
			cChan = -1;
			soundwindow(0);
			global_df->SnTr.IsSoundOn = (char)(soundwindow(1) == 0);
			if (global_df->SnTr.IsSoundOn)
				cChan = global_df->SnTr.SNDchan;
		}
		pDoc->redo_soundwave = FALSE;
	}
	
	if (pDoc->InsertSpChar && pDoc->FileInfo != NULL) {
		global_df = pDoc->FileInfo;
		call_win95_call(pDoc->InsertSpChar, 0, 0, 0);
		pDoc->InsertSpChar = 0;
	}

	if (pDoc->RedisplayWindow && pDoc->FileInfo != NULL) {
		global_df = pDoc->FileInfo;
		DrawCursor(0);
		if (global_df->SoundWin) {
			if (doMixedSTWave)
				global_df->total_num_rows = 5;
			else
				global_df->total_num_rows = global_df->SnTr.SNDchan * 5;
			global_df->total_num_rows += 1;
			global_df->CodeWinStart = 0;
		} else {
			global_df->total_num_rows = 1;
			global_df->CodeWinStart = 0;
		}
		init_windows(true, 1, true);
		if (doReWrap)
			Re_WrapLines(AddLineToRow, 0L, FALSE, NULL);
		RefreshAllTextWindows(TRUE);
		DrawCursor(1);
		pDoc->RedisplayWindow = FALSE;
	}

	if (pDoc->docFName[0] != EOS) {
		char isUTF8Header;
		short id;

		if (AfxComparePath(pDoc->GetPathName(), cl_T("")) && 
				isalpha(pDoc->docFName[0]) && 
				pDoc->docFName[1] == ':' && pDoc->docFName[2] == '\\')
			pDoc->SetPathName(cl_T(pDoc->docFName), FALSE);
		newlyCreated = TRUE;
		if (!strcmp(pDoc->docFName, "CLAN Output"))
			id = 1964;
		else
			id = 1962;
		isUTF8Header = FALSE;
		pDoc->FileInfo = InitFileInfo(pDoc->docFName, pDoc->m_RowLimit, id, &isUTF8Header);
		isAjustCursor = TRUE;
		pDoc->docFName[0] = EOS;
		global_df = pDoc->FileInfo;
		if (pDoc->FileInfo == NULL) {
			GlobalDC = tGlobalDC;
			global_df = tGlobal_df;
			return;
		}
		if (pDoc->FileInfo->DataChanged)
			pDoc->SetModifiedFlag(TRUE);
		if (!init_windows(true, 1, false)) {
	    	FreeUpText(pDoc->FileInfo);
	    	free(pDoc->FileInfo);
	    	pDoc->FileInfo = NULL;
	    	GlobalDC = tGlobalDC;
	    	global_df = tGlobal_df;
	    	return;
		}
		if (id == 1962 && !isRefEQZero(global_df->fileName))
			setTextCursor(global_df, &global_df->winInfo);
		if (!global_df->EditorMode && global_df->CurCode != global_df->RootCodes) {
			if (global_df->ScrollBar != '\002')
				ControlAllControls(2, 0);
		} else {
			if (global_df->ScrollBar != '\001')
				ControlAllControls(1, 0);
		}
		SetRdWKeyScript();
		if (lineNumOverride < 0L || strcmp(lineNumFname, global_df->fileName)) {
			DestroyCaret();
			DrawCursor(3);
		}
		SCROLLINFO sb;
		sb.cbSize = sizeof(SCROLLINFO);
		sb.fMask = SIF_DISABLENOSCROLL | SIF_RANGE;
		sb.nMin = 0;
		sb.nMax = 0;
		GlobalDC->GetWindow()->ShowScrollBar(SB_VERT, TRUE);
		GlobalDC->GetWindow()->SetScrollInfo(SB_VERT, &sb, TRUE);
		GlobalDC->GetWindow()->ShowScrollBar(SB_HORZ, TRUE);
		GlobalDC->GetWindow()->SetScrollInfo(SB_HORZ, &sb, TRUE);
		SetScrollControl();
		if (DOSdata != 0) {
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

			if (*ced_lineC != EOS)
				do_warning(ced_lineC, 0);	
		}	
		DOSdata = -1;
		if (firstTimeStart) {
			firstTimeStart = FALSE;	
			if (AfxComparePath(pDoc->GetPathName(), cl_T("")) && DefClan && clanDlg == NULL) {
				DrawCursor(0);
				DrawFakeHilight(0);
				global_df->DrawCur = FALSE;
				clanDlg = new CClanWindow(AfxGetApp()->m_pMainWnd);
				strcpy(commands_win_name, "Commands - ");
				strcat(commands_win_name, ced_version);
				clanDlg->SetWindowText(cl_T(commands_win_name));
			}
		}
		SetPBCglobal_df(false, 0L);
	} else if (pDoc->SaveMovieInfo) {
		pDoc->SaveMovieInfo = FALSE;
		global_df = pDoc->FileInfo;
		if (global_df && !PlayingContMovie) {
			strcpy(global_df->err_message, DASHES);
			DrawCursor(0);
			DrawFakeHilight(0);
			addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, global_df->MvTr.MEnd);
			PosAndDispl();
			DrawCursor(1);
			DrawFakeHilight(1);
			SetScrollControl();
			if (global_df->DataChanged)
				pDoc->SetModifiedFlag(TRUE);
		}
	} else if (pDoc->SetFakeHilight == 1 && (/* // NO QT MovDlg != NULL || */ MpegDlg != NULL)) {
		pDoc->SetFakeHilight = 0;
		global_df = pDoc->FileInfo;
		if (global_df && !PlayingContMovie) {
			DrawFakeHilight(0);
			FakeSelectWholeTier(F5Option == EVERY_LINE);
			DrawFakeHilight(1);
		}
	} else if (pDoc->ExecuteCommNUM) {
		int num = pDoc->ExecuteCommNUM;
		isPlayS = pDoc->ExecuteCommNUM;
		pDoc->ExecuteCommNUM = 0;
		global_df = pDoc->FileInfo;
		if (global_df) {
			DrawFakeHilight(0);
//			call_win95_call(0, 0, 0, 0);
			addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, global_df->MvTr.MEnd);
			if (num != 89 && num != 91 && num != 96) {
				FakeSelectWholeTier(F5Option == EVERY_LINE);
				DrawFakeHilight(1);
			}
			if (global_df->DataChanged)
				pDoc->SetModifiedFlag(TRUE);
		} else
			isPlayS = 0;
	} else if (pDoc->WC_dummy != 0L) { /* lxs 2022-10-21 Walker Controller */
		global_df = pDoc->FileInfo;
		DrawCursor(0);
		DrawCursor(1);
		pDoc->WC_dummy = 0;
		GlobalDC = tGlobalDC;
		global_df = tGlobal_df;
		return;
	} else if (pDoc->ChangeCursorInfo != 0L) {
		global_df = pDoc->FileInfo;
		if (global_df) {
			if (MpegDlg != NULL && MpegDlg->mpeg != NULL && MpegDlg->mpeg->isMovieDurationSet != TRUE) {
			} else if (PlayingContMovie == '\003' || pDoc->isTranscribing) {
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
					addBulletsToText(SOUNDTIER, global_df->MvTr.MovieFile, global_df->MvTr.MBeg, CurFP);
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
				HighlightNextTier(pDoc->ChangeCursorInfo, global_df->MvTr.MovieFile, TRUE, FALSE);
				PosAndDispl();
				DrawCursor(1);
				DrawFakeHilight(1);
				PlayingContMovie = '\002';
			}
		}
		pDoc->ChangeCursorInfo = 0L;
	} else if (pDoc->RestorePlayMovieInfo == TRUE) {
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
		global_df = pDoc->FileInfo;
		if (global_df && PBC.enable && PBC.isPC) {
/* // NO QT
			if (MovDlg && MovDlg->qt) {
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
							goto finmov;
						}
						global_df->MvTr.MBeg = tLong;

						if (PBC.walk == 2 && global_df->MvTr.MBeg >= sEF) {
							finishWalk(global_df);
							goto finmov;
						}
						if (global_df->MvTr.MBeg >= (MovDlg->qt)->GetQTMovieDuration()) {
							finishWalk(global_df);
							goto finmov;
						}

						global_df->MvTr.MEnd = global_df->MvTr.MEnd - (PBC.backspace - PBC.step_length);
						if (PBC.walk == 2 && global_df->MvTr.MEnd > sEF)
							global_df->MvTr.MEnd = sEF;
					} else {
						tLong = global_df->MvTr.MBeg + (PBC.step_length - PBC.backspace);
						if (tLong < 0L) {
							finishWalk(global_df);
							goto finmov;
						}
						global_df->MvTr.MBeg = tLong;

						if (PBC.walk == 2 && global_df->MvTr.MBeg >= sEF) {
							finishWalk(global_df);
							goto finmov;
						}
						if (global_df->MvTr.MBeg >= (MovDlg->qt)->GetQTMovieDuration()) {
							finishWalk(global_df);
							goto finmov;
						}

						global_df->MvTr.MEnd = global_df->MvTr.MEnd + (PBC.step_length - PBC.backspace);
						if (PBC.walk == 2 && global_df->MvTr.MEnd > sEF)
							global_df->MvTr.MEnd = sEF;
					}
					MovDlg->qt->Movie_PreLoad(global_df->MvTr.MBeg);
					MovDlg->m_from = global_df->MvTr.MBeg;
					MovDlg->m_pos = MovDlg->m_from;
					MovDlg->m_to = global_df->MvTr.MEnd;
					MovDlg->Play();
finmov:
					pDoc->DoWalkerController = FALSE;
				} else {
					finishWalk(global_df);
				}
			} else
*/
			if (MpegDlg && MpegDlg->mpeg) {
				if (PBC.enable && PBC.isPC && PBC.LoopCnt > MpegDlg->MoviePlayLoopCnt) {
					MpegDlg->MoviePlayLoopCnt++;
					MpegDlg->m_pos = MpegDlg->m_from;
					MpegDlg->Play();
				} else if (global_df != NULL && PBC.walk && PBC.isPC && PBC.backspace != PBC.step_length) {
					MpegDlg->MoviePlayLoopCnt = 1;
					if (PBC.backspace > PBC.step_length) {
						tLong = global_df->MvTr.MBeg - (PBC.backspace - PBC.step_length);
						if (tLong < 0L) {
							finishWalk(global_df);
							goto finmpeg;
						}
						global_df->MvTr.MBeg = tLong;

						if (PBC.walk == 2 && global_df->MvTr.MBeg >= sEF) {
							finishWalk(global_df);
							goto finmpeg;
						}
						if (global_df->MvTr.MBeg >= (MpegDlg->mpeg)->GetMpegMovieDuration()) {
							finishWalk(global_df);
							goto finmpeg;
						}

						global_df->MvTr.MEnd = global_df->MvTr.MEnd - (PBC.backspace - PBC.step_length);
						if (PBC.walk == 2 && global_df->MvTr.MEnd > sEF)
							global_df->MvTr.MEnd = sEF;
					} else {
						tLong = global_df->MvTr.MBeg + (PBC.step_length - PBC.backspace);
						if (tLong < 0L) {
							finishWalk(global_df);
							goto finmpeg;
						}
						global_df->MvTr.MBeg = tLong;

						if (PBC.walk == 2 && global_df->MvTr.MBeg >= sEF) {
							finishWalk(global_df);
							goto finmpeg;
						}
						if (global_df->MvTr.MBeg >= (MpegDlg->mpeg)->GetMpegMovieDuration()) {
							finishWalk(global_df);
							goto finmpeg;
						}

						global_df->MvTr.MEnd = global_df->MvTr.MEnd + (PBC.step_length - PBC.backspace);
						if (PBC.walk == 2 && global_df->MvTr.MEnd > sEF)
							global_df->MvTr.MEnd = sEF;
					}
					MpegDlg->m_from = global_df->MvTr.MBeg;
					MpegDlg->m_pos = MpegDlg->m_from;
					MpegDlg->m_to = global_df->MvTr.MEnd;
					MpegDlg->Play();
finmpeg:
					pDoc->DoWalkerController = FALSE;
				} else {
					finishWalk(global_df);
				}
			}
		}
	} else if (pDoc->isAddFStructComm[0] != EOS) {
skipSyn:
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
				if (SaveToFile(global_df->fileName, TRUE)) {
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
		if (pDoc->isNeedResize) {
			long t;

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
			char tc;
	    	WINDOW *t;

			DrawCursor(0);
			if (/* // NO QT GetActiveWindow( ) == MovDlg || */ GetActiveWindow( ) == MpegDlg) {
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
//			ts = global_df->SSelectFlag;
			DrawCursor(2);
//			DrawSoundCursor(2);
			global_df->TSelectFlag = tc;
//			global_df->SSelectFlag = ts;
			if (/* // NO QT GetActiveWindow( ) == MovDlg || */ GetActiveWindow( ) == MpegDlg) {
				DrawSoundCursor(0);
				DrawFakeHilight(1);
			}
			DrawCursor(1);
		}
		if (pDoc->SetFakeHilight == 2 && (/* // NO QT MovDlg != NULL || */ MpegDlg != NULL)) {
			pDoc->SetFakeHilight = 0;
			if (global_df && !PlayingContMovie) {
				DrawFakeHilight(0);
				FakeSelectWholeTier(F5Option == EVERY_LINE);
				DrawSoundCursor(0);
				DrawFakeHilight(1);
			}
		}
	}


	if (getWebMedia != NULL) {
		char *s;

		strcpy(ced_lineC, global_df->fileName);
		s = strrchr(ced_lineC, '\\');
		if (s != NULL)
			*(s+1) = EOS;
		global_df->webMFiles = getMediaFiles(ced_lineC, getWebMedia, global_df, NULL);
		global_df->isTempFile = 1;
		getWebMedia = NULL;
	}

	if (lineNumOverride >= 0L && !strcmp(lineNumFname, global_df->fileName)) {
		if (global_df != NULL) {
			strcpy(global_df->err_message, DASHES);
			DrawCursor(0);
			SaveUndoState(FALSE);
			global_df->row_win2 = 0L;
			global_df->col_win2 = 0L;
			global_df->col_chr2 = 0L;
			if (isSpOverride)
				MoveToSpeaker(lineNumOverride, 1);
			else
				MoveToLine(lineNumOverride, 1);
			isSpOverride = FALSE;
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
		char isSpeakerFound;
		extern char SelectWholeTier(char isCAMode);

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

			if (isMainSpeaker(*global_df->row_txt->line))
				isSpeakerFound = TRUE;
			else
				isSpeakerFound = FALSE;
			if ((res=findStartMediaTag(TRUE, F5Option == EVERY_LINE)) != TRUE)
				findEndOfSpeakerTier(res, F5Option == EVERY_LINE);
			if (global_df->row_txt->line[0] == '\t' && (global_df->row_txt->line[1] == EOS || global_df->row_txt->line[1] == NL_C)) { 
				SaveUndoState(FALSE);
				global_df->redisplay = 0;
				DeletePrevChar(1);
				global_df->redisplay = 1;
			}
			if (!isSpeakerFound) {
				if (!isMainSpeaker(*global_df->row_txt->line) && (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE) || FoundEndHTier(global_df->row_txt, FALSE))) {
					if (global_df->row_txt->line[0] != EOS && global_df->row_txt->line[0] != NL_C) { 
						SaveUndoState(FALSE);
						if (FoundEndHTier(global_df->row_txt, FALSE)) {
							MoveUp(1);
						}
						EndOfLine(-1);
						NewLine(-1);
					}
				}
				if (global_df->row_txt->line[0] == EOS || global_df->row_txt->line[0] == NL_C) { 
					SaveUndoState(FALSE);
					AddText(cl_T("*:\t"), EOS, 1, strlen("*:\t"));
				}
			} else if (global_df->row_txt->line[0] == EOS || global_df->row_txt->line[0] == NL_C) { 
				MoveUp(1);
			}
			SelectWholeTier(F5Option == EVERY_LINE);
			if (type == isAudio) {
				DrawSoundCursor(1);
				DrawCursor(1);
				global_df->SnTr.SSource = 1;
				if (!PlaySound(&global_df->SnTr, (int)'r')) {
					PlayingContSound = FALSE;
					global_df->row_win2 = 0L; global_df->col_win2 = -2L; global_df->col_chr2 = -2L;
				}
			} else
				PlayMovie(&global_df->MvTr, global_df, FALSE);
		} else if (type == isPict) { // picture
			DisplayPhoto(global_df->PcTr.pictFName);
		} else if (type == isText) { // text
			DisplayText(global_df->TxTr.textFName);
		} else {
			strcpy(global_df->err_message, "+Unsupported media type.");
		}
	}

	if (clanRun && !newlyCreated && global_df && !strcmp(global_df->fileName, "CLAN Output")) {
		FILE  *fp;
		char mDirPathName[FNSize];
		char nowref[_MAX_PATH+1];
		extern char ClanAutoWrap;
		extern char isMORXiMode;

		if (StdInWindow != NULL && global_df != NULL && !strcmp(StdInWindow, global_df->fileName)) {
			do_warning(StdDoneMessage, 0);
		} else {
			isKillProgram = 0;
			clanRun = FALSE;
			if (global_df->RowLimit) {
				global_df->curRowLimit = global_df->row_txt;
				global_df->isOutputScrolledOff = FALSE;
			}
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
			tComWinDataChanged = global_df->DataChanged;
			EndOfFile(-1);
			SetScrollControl();
			ResetUndos();
			copyNewToFontInfo(&global_df->row_txt->Font, &dFnt);
			global_df->attOutputToScreen = 0;
			if (global_df->wLineno == 1)
				OutputToScreen(cl_T("> "));
			my_getcwd(nowref, _MAX_PATH);
			dFnt.isUTF = isUTFData;

			if (!uS.mStrnicmp(clanBuf, "batch ", 6) || !uS.mStrnicmp(clanBuf, "bat ", 4)) {
				char *com, *eCom;

				SetNewVol(wd_dir);
				uS.remFrontAndBackBlanks(clanBuf + 1);
				OutputToScreen(cl_T(clanBuf));
				for (com = clanBuf; *com != EOS && *com != ' ' && *com != '\t'; com++);
				for (; *com == ' ' || *com == '\t'; com++);
				eCom = com;
				if (*com != EOS) {
					for (; *eCom != EOS && *eCom != ' ' && *eCom != '\t'; eCom++);
					if (*eCom != EOS) {
						*eCom = EOS;
						eCom++;
					}
				}
				fp = OpenGenLib(com, "r", TRUE, TRUE, mDirPathName);
				if (fp == NULL) {
					sprintf(ced_lineC, "Can't find batch file \"%s\" in either working or library directory: %s", com, mDirPathName);
					do_warning(ced_lineC, 0);
				}
				else {
					sprintf(ced_lineC, "\nRunning batch file: %s\n", mDirPathName);
					UTF8ToUnicode((unsigned char *)ced_lineC, strlen(ced_lineC), templine4, NULL, UTTLINELEN);
					OutputToScreen(templine4);
				}
				if (fp != NULL) {
					if ((eCom = getBatchArgs(eCom)) != NULL) {
						eCom++;
						while (fgets(eCom, 1024, fp)) {
							uS.remFrontAndBackBlanks(eCom);
							if (uS.isUTF8(eCom) || uS.partcmp(eCom, FONTHEADER, FALSE, FALSE) ||
								eCom[0] == '%' || eCom[0] == '#' || eCom[0] == EOS)
								continue;
							if (!fixArgs(com, eCom))
								break;
							if (*eCom == EOS ||
								(toupper(eCom[0]) == 'T' && toupper(eCom[1]) == 'Y') ||
								(toupper(eCom[0]) == 'P' && toupper(eCom[1]) == 'A'));
							else {
								OutputToScreen(cl_T("\nBATCH> "));
								execute(eCom, tComWinDataChanged);
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
			}
			else
				execute(clanBuf, tComWinDataChanged);

			copyNewFontInfo(&dFnt, &oFnt);
			if (isKillProgram)
				isKillProgram = 0;
			if (global_df != NULL && !strcmp(global_df->fileName, "CLAN Output")) {
				if (global_df->RowLimit && global_df->isOutputScrolledOff) {
					fprintf(stderr, "\n%c%c*** PART OF OUTPUT HAS SCROLLED OFF \"CLAN Output\" WINDOW%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
					fprintf(stderr, "%c%c*** YOU CAN INCREASE \"CLAN Output\" WINDOW SIZE IN \"CLAN Options\" MENU%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
					fprintf(stderr, "%c%c*** OR USE +f OPTION WITH THE COMMAND TO SAVE OUTPUT TO THE FILE%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
				}
				if (!isMORXiMode)
					OutputToScreen(cl_T("\n> "));
				global_df->DataChanged = tComWinDataChanged;
				PosAndDispl();
				SetScrollControl();
			}
			SetNewVol(nowref);
			clanBuf = NULL;
			CWnd* win = GlobalDC->GetWindow();
			if (win == NULL || win == win->GetFocus())
				DrawCursor(3);
			GlobalDC = tGlobalDC;
			global_df = tGlobal_df;
			//		if (clanDlg != NULL)
			//			clanDlg->SetActiveWindow();
		}
	} else {
		GlobalDC = tGlobalDC;
		global_df = tGlobal_df;
	}
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
	long	print_row, len, i;
	size_t	j;
	long	fsize = -1;
	long	newcol, colWin;
	unsigned long	ln;
	char	isFindFirstPage;
	AttTYPE	oldState;
	char	attChanged;
	char	fontName[LF_FACESIZE];
	long	FHeight;
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
			if (LineNumberingType == 0 || isMainSpeaker(rt->line[0]))
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
				if (LineNumberingType == 0 || isMainSpeaker(rt->line[0])) {
					uS.sprintf(ced_line, cl_T("%-5ld "), ln);
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
								ced_line[j] = (unCH)0x2022;
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
						puText = ced_line+j;
						totalw = i-j;
						TabbedTextOutW(pDC->m_hDC, leftPos, print_row, puText, totalw,0,NULL,0);

						if (GetTextExtentPointW(pDC->m_hDC,puText,totalw,&wCW) != 0) {
							CW.cx = (short)wCW.cx;
							CW.cy = (short)wCW.cy;
						} else
							CW.cx = 0;
						
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
	extern char isShowPict;

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
		if (/*PlayingContMovie != '\003' && */ PlayingContMovie && !isShowPict) {
/* // NO QT
			if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->isPlaying && !(MovDlg->qt)->IsQTMovieDone()) {
				StopMovie((MovDlg->qt)->theMovie);
				(MovDlg->qt)->isPlaying = 0;
				PlayingContMovie = false;
				if(pDoc != NULL)
					pDoc->isTranscribing = false;
			}
*/
			if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->isPlaying && !(MpegDlg->mpeg)->IsMpegMovieDone()) {
				(MpegDlg->mpeg)->StopMpegMovie();
				PlayingContMovie = false;
				if(pDoc != NULL)
					pDoc->isTranscribing = false;
			}
		}
		isShowPict = FALSE;
	}
 }

/////////////////////////////////////////////////////////////////////////////
// CPadView Font Handling
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
	if (!global_df->ChatMode) {
		DrawCursor(0);
		StyleItems(2, 0);
		DrawCursor(1);
	} else
		do_warning("\"Italic\" function is illegal in CHAT.", 0);
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
	if (!global_df->ChatMode) {
		DrawCursor(0);
		StyleItems(3, 0);
		DrawCursor(1);
	} else
		do_warning("\"Bold\" function is illegal in CHAT.", 0);
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
		extern long w95_fontSize;
		if (!strcmp(m_lfDefFont.lfFaceName, dFnt.fontName)) {
			dFnt.fontSize = m_lfDefFont.lfHeight;
//			WriteCedPreference();
		}
		w95_fontSize = 0L;
		u_strcpy(dFnt.fontName, lf.lfFaceName, 256);
		dFnt.fontSize = lf.lfHeight;
		dFnt.CharSet = (int)lf.lfCharSet;
		dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
		dFnt.orgEncod = dFnt.Encod;
		dFnt.orgFType = NOCHANGE;
		stickyFontSize = dFnt.fontSize;
		dFnt.isUTF = TRUE;
		copyNewFontInfo(&oFnt, &dFnt);
		C_MBF = uS.partwcmp(dFnt.fontName,"Jpn ") || uS.partwcmp(dFnt.fontName,"Chn ");
		WriteCedPreference();

		m_font.DeleteObject();
		m_font.CreateFontIndirect(&lf);
	}
}


void CClan2View::OnChooseCommandFont() {
	LOGFONT lf;
	char commands_win_name[512];
	extern char ced_version[];

	// 2016-03-29
	SetLogfont(&lf, &cmdFnt, NULL);
	CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT);
	if (dlg.DoModal() == IDOK) {  // lxslxslxs
		m_lfComDefFont = lf;
		u_strcpy(cmdFnt.FName, lf.lfFaceName, LF_FACESIZE);
		cmdFnt.FSize = lf.lfHeight;
		cmdFnt.CharSet = (int)lf.lfCharSet;
		cmdFnt.Encod = my_FontToScript(cmdFnt.FName, cmdFnt.CharSet);
//		WriteCedPreference();
		if (clanDlg != NULL) {
			clanDlg->OnCancel();
			clanDlg = new CClanWindow(AfxGetApp()->m_pMainWnd);
			strcpy(commands_win_name, "Commands - ");
			strcat(commands_win_name, ced_version);
			clanDlg->SetWindowText(cl_T(commands_win_name));
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

	CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST);
	if (dlg.DoModal() == IDOK) {  // lxslxslxs
		DrawCursor(0);
		DrawSoundCursor(0);

		m_lfDefFont = lf;
		extern long w95_fontSize;
		if (!strcmp(m_lfDefFont.lfFaceName, dFnt.fontName)) {
			dFnt.fontSize = m_lfDefFont.lfHeight;
//			WriteCedPreference();
		}
		w95_fontSize = 0L;
		u_strcpy(dFnt.fontName, lf.lfFaceName, 256);
		dFnt.fontSize = lf.lfHeight;
		dFnt.CharSet = (int)lf.lfCharSet;
		dFnt.Encod = my_FontToScript(dFnt.fontName, dFnt.CharSet);
		dFnt.orgEncod = dFnt.Encod;
		dFnt.orgFType = NOCHANGE;
		stickyFontSize = dFnt.fontSize;
		dFnt.isUTF = TRUE;
		copyNewFontInfo(&oFnt, &dFnt);
		C_MBF = uS.partwcmp(dFnt.fontName, "Jpn ") || uS.partwcmp(dFnt.fontName, "Chn ");
		WriteCedPreference();

		m_font.DeleteObject();
		m_font.CreateFontIndirect(&lf);

		u_strcpy(fi.FName, lf.lfFaceName, LF_FACESIZE);
		fi.FSize = lf.lfHeight;
		fi.CharSet = (int)lf.lfCharSet;
		fontHeight = GetFontHeight(&fi, NULL);
		if (global_df->MinFontSize > fontHeight || global_df->MinFontSize == 0)
			global_df->MinFontSize = fontHeight;
		u_strcpy(FName, lf.lfFaceName, LF_FACESIZE);
		cedDFnt.Encod = my_FontToScript(FName, lf.lfCharSet);
		cedDFnt.isUTF = global_df->isUTF;
		ChangeCurLineAlways(0);
		if (global_df->row_win2 == 0L && global_df->col_win2 == -2L) {
			for (rt = global_df->head_text->next_row; rt != global_df->tail_text; rt = ToNextRow(rt, FALSE)) {
				strcpy(rt->Font.FName, FName);
				rt->Font.FSize = lf.lfHeight;
				rt->Font.CharSet = (int)lf.lfCharSet;
				rt->Font.FHeight = fontHeight;
			}
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
					for (; cnt <= 0 && rt != global_df->head_text; cnt++, rt = ToPrevRow(rt, FALSE)) {
						strcpy(rt->Font.FName, FName);
						rt->Font.FSize = lf.lfHeight;
						rt->Font.CharSet = (int)lf.lfCharSet;
						rt->Font.FHeight = fontHeight;
					}
				} else if (global_df->row_win2 > 0L) {
					if (global_df->col_win2 <= 0) cnt--;
					for (rt = global_df->row_txt; cnt >= 0 && rt != global_df->tail_text;
						 cnt--, rt = ToNextRow(rt, FALSE)) {
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
		global_df->winWidthOffset = -1;
		if (!init_windows(true, !doReWrap, false))
			mem_err(TRUE, global_df);
		if (doReWrap) {
			Re_WrapLines(AddLineToRow, 0L, FALSE, NULL);
			DisplayTextWindow(NULL, 1);
		}
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
	if (global_df->DataChanged)
		GlobalDoc->SetModifiedFlag(TRUE);
	DrawSoundCursor(1);
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

/* // NO QT
	if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->isPlaying && !(MovDlg->qt)->IsQTMovieDone()) {
		if (PlayingContMovie || !PBC.walk || !PBC.enable) {
			stopMoviePlaying();
			PlayingContMovie = FALSE;
		}
	}
*/
	if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->isPlaying && !(MpegDlg->mpeg)->IsMpegMovieDone()) {
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
	extern char DoubleClickCount;
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	GlobalDoc = pDoc;
	GlobalDC = &dc;
	global_df = pDoc->FileInfo;
	if (cursorCnt < 0)
		cursorCnt = ShowCursor(TRUE);

/* // NO QT
	if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->isPlaying && !(MovDlg->qt)->IsQTMovieDone()) {
		if (PlayingContMovie || !PBC.walk || !PBC.enable) {
			stopMoviePlaying();
			PlayingContMovie = FALSE;
		}
	}
*/
	if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->isPlaying && !(MpegDlg->mpeg)->IsMpegMovieDone()) {
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
	if (DoubleClickCount == 3) {
		DoubleClickCount = 1;
		OnLButtonUp(nFlags, point);
	}
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

	while (isPlayS) {
		win95_call(0, 0, 0, 0);
	}
	DrawSoundCursor(1);
	if (global_df != NULL) {
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->col_txt->prev_char != global_df->head_row)
				global_df->gAtt = global_df->col_txt->prev_char->att;
			else
				global_df->gAtt = 0;
		}
		else if (global_df->row_txt->att != NULL) {
			if (global_df->col_chr > 0)
				global_df->gAtt = global_df->row_txt->att[global_df->col_chr - 1];
			else
				global_df->gAtt = 0;
		}
		else
			global_df->gAtt = 0;
	}
	GlobalDC = tGlobalDC;
						// the beginning of the mouse drag.

//	CView::OnLButtonUp(nFlags, point);
}

BOOL CClan2View::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	
	BOOL isScroll;
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	CWnd* win = GlobalDC->GetWindow();
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	isScroll = true;
	if (global_df) {
		if (!global_df->VScrollBar)
			isScroll = false;
		if (global_df->SoundWin && pt.y > global_df->SoundWin->LT_row) {
			isScroll = false;
		}
	}
//	lxs
	if (isScroll) {
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

void CClan2View::OnSelectMediaFile()
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
		SelectMediaFile();
		FinishMainLoop();
		if (global_df->DataChanged)
			GlobalDoc->SetModifiedFlag(TRUE);
	}
	GlobalDC = tGlobalDC;
}

// MORGrammar get MOR Grammar BEG
void CClan2View::OnGetEnglishGrammar() {
	GetMORGrammar("eng", 603000L);
}

void CClan2View::OnGetCantoneseGrammar() {
	GetMORGrammar("yue",363000L);
}

void CClan2View::OnGetChineseGrammar() {
	GetMORGrammar("zho", 416000L);
}

void CClan2View::OnGetDanishGrammar() {
	GetMORGrammar("dan", 589000L);
}

void CClan2View::OnGetDutchGrammar() {
	GetMORGrammar("nld", 100000L);
}

void CClan2View::OnGetFrenchGrammar() {
	GetMORGrammar("fra", 346000L);
}

void CClan2View::OnGetGermanGrammar() {
	GetMORGrammar("deu", 1700000L);
}

void CClan2View::OnGetHebrewGrammar() {
	GetMORGrammar("heb",1100000L);
}

void CClan2View::OnGetItalianGrammar() {
	GetMORGrammar("ita", 282000L);
}

void CClan2View::OnGetJapaneseGrammar() {
	GetMORGrammar("jpn",1000000L);
}

void CClan2View::OnGetSpanishGrammar() {
	GetMORGrammar("spa", 399000L);
}
// MORGrammar get MOR Grammar END

// KIDEVAL Get KIDEVAL Database BEG
void CClan2View::OnGetEnglishKevalDB() {
	GetKidevalDB("eng_fp_kideval_db.cut", 127000L);
}
void CClan2View::OnGetChineseKevalDB() {
	GetKidevalDB("zho_md_kideval_db.cut", 5000L);
}
// KIDEVAL Get KIDEVAL Database END

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
	BOOL isScroll;
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	CWnd* win = GlobalDC->GetWindow();
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	isScroll = true;
	if (global_df) {
		if (!global_df->VScrollBar)
			isScroll = false;
//		if (global_df->SoundWin && pt.y > global_df->SoundWin->LT_row) {
//			isScroll = false;
//		}
	}
	if (isScroll)
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
	BOOL isScroll;
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	CWnd* win = GlobalDC->GetWindow();
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	DrawCursor(0);
	isScroll = true;
	if (global_df) {
		if (global_df->SoundWin) {
		} else if (!global_df->HScrollBar)
			isScroll = false;
	}
//	lxs
	if (isScroll)
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
		DrawSoundCursor(0);
		SaveUndoState(TRUE);
		Undo(1);
		PosAndDispl();
		DrawCursor(1);
		DrawSoundCursor(1);
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
		DrawSoundCursor(0);
		SaveUndoState(TRUE);
		Redo(1);
		PosAndDispl();
		DrawCursor(1);
		DrawSoundCursor(1);
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

void RefreshAllTextWindows(char dummy) {
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
		RefreshAllTextWindows(FALSE);
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
		if (global_df->SoundWin) {
			if (doMixedSTWave)
				global_df->total_num_rows = 5;
			else
				global_df->total_num_rows = global_df->SnTr.SNDchan * 5;
			global_df->total_num_rows += 1;
			global_df->CodeWinStart = 0;
		} else {
			global_df->total_num_rows = 1;
			global_df->CodeWinStart = 0;
		}
		init_windows(true, 1, true);
		RefreshAllTextWindows(TRUE);
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

void CClan2View::OnModeHideTiers() 
{
	CClientDC dc(this);
	CDC* tGlobalDC = GlobalDC;
	CClan2Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	if (global_df != NULL) {
		DrawCursor(0);
		strcpy(global_df->err_message, DASHES);
		if (global_df->headtier != NULL)
			ShowAllTiers(1);
		else
			checkForTiersToHide(global_df, TRUE);
		DisplayTextWindow(NULL, 1);
		ResetUndos();
		FinishMainLoop();
		DrawCursor(1);
	}
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
	extern char isPlayAudioFirst, isPlayAudioMenuSet;

	isPlayAudioFirst = !isPlayAudioFirst;
	isPlayAudioMenuSet = 1;
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
	FNType mFileName[FILENAME_MAX];
	unCH wFileName[FILENAME_MAX];

	GlobalDC = &dc;
	GlobalDoc = pDoc;
	global_df = pDoc->FileInfo;
	stopSoundIfPlaying();
	if (global_df != NULL)
		strcpy(global_df->err_message, DASHES);
	if (MpegDlg)
		MpegDlg->Save();
/* // NO QT
	else if (MovDlg)
		MovDlg->Save();
*/
	else if (global_df->SoundWin) {
		SetCurrSoundTier();
	} else {
		char ret;
		if ((ret = GetNewMediaFile(TRUE, isPictText))) {
			if (ret == isAudio) { // sound
			} else if (ret == isVideo) { // movie
			} else if (ret == isPict) { // picture
				DrawCursor(0);
				extractFileName(mFileName, global_df->PcTr.pictFName);
				u_strcpy(wFileName, mFileName, FILENAME_MAX);
				addBulletsToText(PICTTIER, wFileName, 0L, 0L);
			} else if (ret == isText) { // text
				DrawCursor(0);
				extractFileName(mFileName, global_df->TxTr.textFName);
				u_strcpy(wFileName, mFileName, FILENAME_MAX);
				addBulletsToText(TEXTTIER, wFileName, 0L, 0L);
			} 
		}
	}
	FinishMainLoop();
	DrawSoundCursor(1);
	DrawCursor(1);
	if (pDoc && global_df && global_df->DataChanged)
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

void CClan2View::OnModePlaycontskip()
{
	isPlayS = 97;
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

void CClan2View::OnTiersIdheaders()
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
		setIDs(1);
		FinishMainLoop();
		if (global_df->DataChanged)
			GlobalDoc->SetModifiedFlag(TRUE);
	}
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


static BOOL Handletext_FKeyPress(UINT nChar, BOOL isVK, unCH *CAChar, char num) {
	// CA CHARS
	if (num == 1) {
		if (nChar == VK_UP && isVK) { // up-arrow
			*CAChar = 0x2191;
			return(true);
		} else if (nChar == VK_DOWN && isVK) { // down-arrow
			*CAChar = 0x2193;
			return(true);
		} else if (nChar == '1' && !isVK) { // rise to high
			*CAChar = 0x21D7;
			return(true);
		} else if (nChar == '2' && !isVK) { // rise to mid
			*CAChar = 0x2197;
			return(true);
		} else if (nChar == '3' && !isVK) { // level
			*CAChar = 0x2192;
			return(true);
		} else if (nChar == '4' && !isVK) { // fall to mid
			*CAChar = 0x2198;
			return(true);
		} else if (nChar == '5' && !isVK) { // fall to low
			*CAChar = 0x21D8;
			return(true);
		} else if (nChar == '6' && !isVK) { // unmarked ending
			*CAChar = 0x221E;
			return(true);
		} else if (nChar == '+' && !isVK) { // continuation - wavy triple-line equals sign
			*CAChar = 0x224B;
			return(true);
		} else if (nChar == '.' && !isVK) { // inhalation
			*CAChar = 0x2219;
			return(true);
		} else if (nChar == '=' && !isVK) { // latching
			*CAChar = 0x2248;
			return(true);
		} else if ((nChar == 'u' || nChar == 'U') && !isVK) { // uptake
			*CAChar = 0x2261;
			return(true);
		} else if (isEqChar(nChar, OpSQBKey) && isEqShift(OpSQBKey) && isVK) { // raised [
			*CAChar = 0x2308;
			return(true);
		} else if (isEqChar(nChar, ClSQBKey) && isEqShift(ClSQBKey) && isVK) { // raised ]
			*CAChar = 0x2309;
			return(true);
		} else if (isEqChar(nChar, OpSQBKey) && isVK) { // lowered [
			*CAChar = 0x230A;
			return(true);
		} else if (isEqChar(nChar, ClSQBKey) && isVK) { // lowered ]
			*CAChar = 0x230B;
			return(true);
		} else if (nChar == VK_RIGHT && isEqShift(VK_RIGHT) && isVK) { // faster
			*CAChar = 0x2206;
			return(true);
		} else if (nChar == VK_LEFT && isEqShift(VK_LEFT) && isVK) { // slower
			*CAChar = 0x2207;
			return(true);
		} else if (nChar == '*' && !isVK) { // creaky
			*CAChar = 0x204E;
			return(true);
		} else if ((nChar == '/') && !isVK) { // unsure
			*CAChar = 0x2047;
			return(true);
		} else if (nChar == '0' && !isVK) { // softer
			*CAChar = 0x00B0;
			return(true);
		} else if (nChar == ')' && !isVK) { // louder
			*CAChar = 0x25C9;
			return(true);
		} else if ((nChar == 'd') && !isVK) { // low pitch - low bar
			*CAChar = 0x2581;
			return(true);
		} else if ((nChar == 'h') && !isVK) { // high pitch - high bar
			*CAChar = 0x2594;
			return(true);
		} else if ((nChar == 'l' || nChar == 'L') && !isVK) { // smile voice
			*CAChar = 0x263A;
			return(true);
		} else if ((nChar == 'b') && !isVK) { // breathy-voice
			*CAChar = 0x264B;
			return(true);
		} else if ((nChar == 'w' || nChar == 'W') && !isVK) { // whisper
			*CAChar = 0x222C;
			return(true);
		} else if ((nChar == 'y' || nChar == 'Y') && !isVK) { // yawn
			*CAChar = 0x03AB;
			return(true);
		} else if ((nChar == 's' || nChar == 'S') && !isVK) { // singing
			*CAChar = 0x222E;
			return(true);
		} else if ((nChar == 'p' || nChar == 'P') && !isVK) { // precise
			*CAChar = 0x00A7;
			return(true);
		} else if ((nChar == 'n' || nChar == 'N') && !isVK) { // constriction
			*CAChar = 0x223E;
			return(true);
		} else if ((nChar == 'r' || nChar == 'R') && !isVK) { // pitch reset
			*CAChar = 0x21BB;
			return(true);
		} else if ((nChar == 'c' || nChar == 'C') && !isVK) { // laugh in a word
			*CAChar = 0x1F29;
			return(true);
		}
	}
	if (num == 2) {
		if (nChar == 'h' && !isVK) { // raised h
			*CAChar = 0x02B0;
			return(true);
		} else if (nChar == ',' && !isVK) { // dot diacritic
			*CAChar = 0x0323;
			return(true);
		} else if ((nChar == '<') && !isVK) { // Group start marker - NOT CA
			*CAChar = 0x2039;
			return(true);
		} else if ((nChar == '>') && !isVK) { // Group end marker - NOT CA
			*CAChar = 0x203A;
			return(true);
		} else if ((nChar == 't' || nChar == 'T') && !isVK) { // Tag or sentence final particle; „ - NOT CA
			*CAChar = 0x201E;
			return(true);
		} else if ((nChar == 'v' || nChar == 'V') && !isVK) { // Vocative or summons - NOT CA
			*CAChar = 0x2021;
			return(true);
		} else if ((nChar == '-') && !isVK) { // Stress - NOT CA
			*CAChar = 0x0304;
			return(true);
		} else if ((nChar == 'q') && !isVK) { // Glottal stop - NOT CA
			*CAChar = 0x0294;
			return(true);
		} else if ((nChar == 'Q') && !isVK) { // Hebrew glottal - NOT CA
			*CAChar = 0x0295;
			return(true);
		} else if ((nChar == ';') && !isVK) { // caron - NOT CA
			*CAChar = 0x030C;
			return(true);
		} else if ((nChar == '1') && !isVK) { // raised stroke - NOT CA
			*CAChar = 0x02C8; // cb 88
			return(true);
		} else if ((nChar == '2') && !isVK) { // lowered stroke - NOT CA
			*CAChar = 0x02CC; // cb 8c
			return(true);
		} else if ((nChar == '{') && !isVK) { // sign group start marker - NOT CA
			*CAChar = 0x3014; // 0xe3 80 94
			return(true);
		} else if ((nChar == '}') && !isVK) { // sign group end marker - NOT CA
			*CAChar = 0x3015; // 0xe3 80 95
			return(true);
		} else if ((nChar == 'm') && !isVK) { // %pho missing word - NOT CA
			*CAChar = 0x2026;
			return(true);
		} else if ((nChar == '_') && !isVK) { // Uderline NOT CA
			*CAChar = 0x0332;
			return(true);
		} else if ((nChar == '\'') && !isVK) { // open quote “ - NOT CA
			*CAChar = 0x201C; // 0xe2 80 9c - NOTCA_OPEN_QUOTE
			return(true);
		} else if ((nChar == '"') && !isVK) { // close quote ” - NOT CA
			*CAChar = 0x201D; // 0xe2 80 a6 - NOTCA_CLOSE_QUOTE
			return(true);
		} else if (nChar == '=' && !isVK) { // crossed equal
			*CAChar = 0x2260; // 0xe2 89 a0 - NOTCA_CROSSED_EQUAL
			return(true);
		} else if (nChar == '/' && !isVK) { // left arrow with circle
			*CAChar = 0x21AB; // 0xe2 86 ab - NOTCA_LEFT_ARROW_CIRCLE
			return(true);
		} else if (nChar == ':' && !isVK) { // colon for long vowels
			*CAChar = 0x02D0; // 0xe2 CB 90 - NOTCA_VOWELS_COLON
			return(true);
		}
	}
	return(false);
}

void CClan2View::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL isHideCursor;
	CClan2Doc* pDoc = GetDocument();

	isHideCursor = (isKeyEqualCommand(nChar, 1) || isKeyEqualCommand(nChar, 5)) && whichInput == 0;
	if (text_isFKeyPressed) {
		unCH CAChar;
		if (text_isFKeyPressed == 3) {
			SetUpVideos();
			getVideosExt(nChar);
			skipOnChar = TRUE;
		} else if (Handletext_FKeyPress(nChar, false, &CAChar, text_isFKeyPressed)) {
			call_win95_call(CAChar, nRepCnt, nFlags, 0);
			skipOnChar = TRUE;
		}
		text_isFKeyPressed = 0;
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
			call_win95_call(nChar, nRepCnt, nFlags, 0);
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

	if (text_isFKeyPressed) {
		unCH CAChar;
		if (text_isFKeyPressed == 3) {
			SetUpVideos();
			getVideosExt(nChar);
			skipOnChar = TRUE;
		} else if (Handletext_FKeyPress(nChar, true, &CAChar, text_isFKeyPressed)) {
			call_win95_call(CAChar, nRepCnt, nFlags, 0);
			skipOnChar = TRUE;
			text_isFKeyPressed = 0;
		}
		return;
	} else
		text_isFKeyPressed = 0;

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

/* // NO QT
	if (MovDlg != NULL && MovDlg->qt != NULL && (MovDlg->qt)->isPlaying && !(MovDlg->qt)->IsQTMovieDone() &&
			nChar != VK_CONTROL && nChar != VK_SHIFT) {
		if (PlayingContMovie || !PBC.walk || !PBC.enable) {
			stopMoviePlaying();
			PlayingContMovie = FALSE;
			skipOnChar = TRUE;
			return;
		}
	}
*/
	if (MpegDlg != NULL && MpegDlg->mpeg != NULL && (MpegDlg->mpeg)->isPlaying && !(MpegDlg->mpeg)->IsMpegMovieDone()) {
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
			BOOL EditorMode;

			GlobalDC = &dc;
			GlobalDoc = pDoc;
			global_df = pDoc->FileInfo;
			if (global_df != NULL)
				EditorMode = global_df->EditorMode;
			else
				EditorMode = FALSE;

			if (nChar == VK_F7 || nChar == VK_F8 || nChar == VK_F9)
				;
			else if (/* // NO QT MovDlg == NULL && */MpegDlg == NULL) {
				if (global_df->SnTr.isMP3 == TRUE)
					stopMovieIfPlaying();
				else
					stopSoundIfPlaying();
			}
			if (global_df) {
				if (nChar == VK_F1 && isUseSPCKeyShortcuts && EditorMode) {
					text_isFKeyPressed = 1;
				} else if (nChar == VK_F2 && isUseSPCKeyShortcuts && EditorMode) {
					text_isFKeyPressed = 2;
				} else if (nChar != VK_F7 && nChar != VK_F8 && nChar != VK_F9) {
					strcpy(global_df->err_message, "-Aborted.");
					draw_mid_wm();
					wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
					wrefresh(global_df->w1);
					if (MpegDlg != NULL && MpegDlg->mpeg != NULL) {
						if ((nChar == VK_F5 || nChar == VK_F6) && MpegDlg != NULL && MpegDlg->mpeg != NULL &&
							(MpegDlg->mpeg)->isPlaying && PBC.walk && PBC.enable) {
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
			if (nChar == VK_F7 || nChar == VK_F8 || nChar == VK_F9) {
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
				text_isFKeyPressed = 1;
			} else {
				F_key = nChar - VK_F1 + 1;
				nChar = 1;
				if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
					nChar += 12;
				if (!skipOnChar)
					call_win95_call(nChar, nRepCnt, nFlags, 0);
				F_key = 0;
			}
			GlobalDC = tGlobalDC;
		} else if (nChar == VK_F2 && isUseSPCKeyShortcuts) {
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
				text_isFKeyPressed = 2;
			} else {
				F_key = nChar - VK_F2 + 1;
				nChar = 1;
				if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
					nChar += 12;
				if (!skipOnChar)
					call_win95_call(nChar, nRepCnt, nFlags, 0);
				F_key = 0;
			}
			GlobalDC = tGlobalDC;
		} else if (nChar == VK_F3 && isUseSPCKeyShortcuts) {
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
				text_isFKeyPressed = 3;
			} else {
				F_key = nChar - VK_F3 + 1;
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
	if (!PlayingContSound && PBC.enable && /* // NO QT MovDlg == NULL && */ MpegDlg == NULL) {
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

			skipOnChar = checkPCMSound(pMsg, skipOnChar, GetTickCount());
			checkPCMPlaybackBuffer();

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
	if (global_df == NULL)
		return;
	DrawCursor(0);
	stopSoundIfPlaying();
	getTextAndSendToSoundAnalyzer();
	SetScrollControl();
	DrawCursor(1);
	global_df->WinChange = FALSE;
	draw_mid_wm();
	GlobalDC = tGlobalDC;
}
