#include "ced.h"

#ifdef _UNICODE
#define DEFFSIZE -13L

static char fontName[256];
long w95_fontSize = 0L;

// extern LOGFONT m_lfDefFont;

char *defFontName(void) {
	if (w95_fontSize == 0L) {
//		if (!strcmp(m_lfDefFont.lfFaceName, "Arial Unicode MS") || !strcmp(m_lfDefFont.lfFaceName, "CAfont"/*UNICODEFONT*/)) {
			w95_fontSize = m_lfDefFont.lfHeight;
//		} else {
//			w95_fontSize = DEFFSIZE;
//		}
		strcpy(fontName, defUniFontName);
	}
	return(fontName);
}

long defFontSize(void) {
	if (w95_fontSize == 0L) {
		if (!strcmp(m_lfDefFont.lfFaceName, "Arial Unicode MS") || !strcmp(m_lfDefFont.lfFaceName, "CAfont"/*UNICODEFONT*/)) {
			w95_fontSize = m_lfDefFont.lfHeight;
		} else {
			w95_fontSize = DEFFSIZE;
		}
		strcpy(fontName, defUniFontName);
	}
	return(w95_fontSize);
}
#endif

short GetFontHeight(FONTINFO *fontInfo, NewFontInfo *finfo) {
	LOGFONT lfFont;
	CFont l_font;
	CSize CharsWidth;

	if (GlobalDC == NULL)
		return(0);

	SetLogfont(&lfFont, fontInfo, finfo);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = GlobalDC->SelectObject(&l_font);
	CharsWidth = GlobalDC->GetTextExtent(_T("IjWfFg"), 6);
	GlobalDC->SelectObject(pOldFont);
	l_font.DeleteObject();
	return(CharsWidth.cy + FLINESPACE);	
}

int getNumberOffset(void) {
    FONTINFO *font;
	LOGFONT lfFont;
    int nDigs;
	CFont l_font;
	CSize CharsWidth;

	if (!isShowLineNums)
		return(0);
	if (GlobalDC == NULL)
		return(0);

	font = GetTextFont(global_df);
	if (font == NULL)
		return(0);
	SetLogfont(&lfFont, font, NULL);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = GlobalDC->SelectObject(&l_font);
	if (LineNumberDigitSize != 0)
		nDigs = LineNumberDigitSize;
	else
		nDigs = 6;
	if (nDigs > 20)
		nDigs = 20;
	if (nDigs < 0)
		nDigs = 6;
	CharsWidth = GlobalDC->GetTextExtent(_T("555555555555555555555"), nDigs);
	GlobalDC->SelectObject(pOldFont);
	l_font.DeleteObject();
	return((int)CharsWidth.cx);	
}

int WindowPageWidth(void) {
	int check_pgwid;	
	RECT theRect;
	CSize CharsWidth;
	CWnd*	win;

	if (GlobalDC == NULL)
		return(0);

	win = GlobalDC->GetWindow();
	if (!win)
		return(0);
	win->GetClientRect(&theRect);
	CFont* pOldFont = GlobalDC->SelectObject(&m_font);
	CharsWidth = GlobalDC->GetTextExtent(_T("l"), 1);
	GlobalDC->SelectObject(pOldFont);
	check_pgwid = (theRect.right-theRect.left-LEFTMARGIN)/CharsWidth.cx;	
	return(check_pgwid);	
}

int WindowPageLength(int *size) {
	int char_height, check_pglen = 0;
	RECT theRect;
	CSize CharsWidth;
	CWnd* win;
	CFont t_font;
	LOGFONT lf;
	BOOL res;
	if (GlobalDC == NULL)
		return(0);

	win = GlobalDC->GetWindow();
	if (!win)
		return(0);
	win->GetClientRect(&theRect);

	m_font.GetObject(sizeof(LOGFONT), &lf);
//	lf.lfHeight = -11;
	res = t_font.CreateFontIndirect(&lf);
	t_font.GetObject(sizeof(LOGFONT), &lf);

	CFont* pOldFont = GlobalDC->SelectObject(&t_font);
	CharsWidth = GlobalDC->GetTextExtent(_T("IjWfFg"), 6);
	GlobalDC->SelectObject(pOldFont);

	char_height = CharsWidth.cy + FLINESPACE;	
	check_pglen = (theRect.bottom - theRect.top) / char_height;	
	check_pglen = (check_pglen * char_height) - (char_height * (*size));	
	if (check_pglen < global_df->MinFontSize) {	
		check_pglen = (theRect.bottom-theRect.top) / char_height;	
		check_pglen = (check_pglen * char_height) - (char_height * 2);	
		*size = 2;	
	}	
	global_df->TextWinSize = check_pglen;	
	check_pglen = check_pglen / global_df->MinFontSize;
	check_pglen = check_pglen + (check_pglen / 3) + (*size);
	return(check_pglen);
}

int NumOfRowsOfDefaultWindow(void) {
	int char_height, check_pglen = 0;
	RECT theRect;
	CSize CharsWidth;
	CWnd*	win;
	CFont t_font;
	LOGFONT lf;


	if (GlobalDC == NULL)
		return(0);

	win = GlobalDC->GetWindow();
	if (!win)
		return(0);
	win->GetClientRect(&theRect);

//	m_font.GetObject(sizeof(LOGFONT), &lf);
//	lf.lfHeight = -10;
//	t_font.CreateFontIndirect(&lf);

	CFont* pOldFont = GlobalDC->SelectObject(&m_font);
	CharsWidth = GlobalDC->GetTextExtent(_T("IjWfFg"), 6);
	GlobalDC->SelectObject(pOldFont);

	char_height = CharsWidth.cy + FLINESPACE;	
	check_pglen = (theRect.bottom - theRect.top) / char_height;	
	return(check_pglen);
}
