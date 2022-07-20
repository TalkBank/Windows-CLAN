// w95_TextBullet.cpp : implementation file
//

//#define TEXT_BULLETS_PICTS

#include "ced.h"
#ifdef TEXT_BULLETS_PICTS
#include "cu.h"
#include "MMedia.h"
#include "w95_TxtBullet.h"
#include "W95_progress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char fgets_cr_lc;
extern struct DefWin TextWinSize;

static int topRow;
static int maxRows;
static int max_num_rows;
static cTxtBullet *txtDial = NULL;
static unCH txtMovieFile[FILENAME_MAX];
static FNType txtrMovieFile[FNSize];
static MovieTXTInfo *txtPicts = NULL;
static COLORTEXTLIST *text_bullets_RootColorText = NULL;

/////////////////////////////////////////////////////////////////////////////
// cTxtBullet dialog

cTxtBullet::cTxtBullet(CWnd* pParent /*=NULL*/)
	: CDialog(cTxtBullet::IDD, pParent)
{

	Create(IDD, pParent);
	//{{AFX_DATA_INIT(cTxtBullet)
	cgp = NULL;
	m_brush.CreateSolidBrush(RGB(255,255,255));
	//}}AFX_DATA_INIT
}


void cTxtBullet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cTxtBullet)
//	DDX_Control(pDX, IDC_TXTBULLET, m_TxtControl);
//	DDX_Text(pDX, IDC_TXTBULLET, m_txt);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(cTxtBullet, CDialog)
	//{{AFX_MSG_MAP(cTxtBullet)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
//	ON_WM_LBUTTONDBLCLK()
//	ON_WM_MOUSEMOVE()
//	ON_WM_KEYDOWN()
//	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cTxtBullet message handlers

BOOL cTxtBullet::OnInitDialog() {
	CDialog::OnInitDialog();

	if (TextWinSize.top || TextWinSize.width || TextWinSize.height || TextWinSize.left) {
		CRect lpRect;

		lpRect.top = TextWinSize.top;
		lpRect.bottom = TextWinSize.height;
		lpRect.left = TextWinSize.left;
		lpRect.right = TextWinSize.width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}
	GrafPort *gp=CreatePortAssociation (m_hWnd, NULL,0);
	cgp=(CGrafPort *)GetWindowPort(gp);
	SetGWorld(cgp, NULL);
	UpdateData(FALSE);
	ShowWindow(SW_SHOWNORMAL); //SW_SHOW);
	Invalidate(TRUE);
	return TRUE;
}

static MovieTXTInfo *freeText(MovieTXTInfo *p) {
	MovieTXTInfo *t;

	while (p != NULL) {
		t = p;
		p = p->nextPict;
		if (t->text)
			free(t->text);
		if (t->atts)
			free(t->atts);
		if (t->pict != NULL)
			KillPicture(t->pict);
		free(t);
	}
	FreeColorText(text_bullets_RootColorText);
	text_bullets_RootColorText = NULL;
	return(NULL);
}

BOOL cTxtBullet::DestroyWindow() 
{
	if (cgp != NULL) {
		DestroyPortAssociation (cgp);
		cgp = NULL;
	}
	return CDialog::DestroyWindow();
}

void cTxtBullet::OnCancel() {
	CRect lpRect;

	this->GetWindowRect(&lpRect);
	TextWinSize.top = lpRect.top;
	TextWinSize.left = lpRect.left;
	TextWinSize.width = lpRect.right;
	TextWinSize.height = lpRect.bottom;
	WriteCedPreference();
	DestroyWindow();
	if (txtDial != NULL) {
		delete txtDial;
		txtDial = NULL;
	}
	topRow = 0;
	maxRows = 0;
	txtPicts = freeText(txtPicts);
}

void cTxtBullet::OnDestroy() 
{
	CRect lpRect;

	CDialog::OnDestroy();
	this->GetWindowRect(&lpRect);
	TextWinSize.top = lpRect.top;
	TextWinSize.left = lpRect.left;
	TextWinSize.width = lpRect.right;
	TextWinSize.height = lpRect.bottom;
	WriteCedPreference();
	CDialog::OnDestroy();
	if (txtDial != NULL) {
		delete txtDial;
		txtDial = NULL;
	}
	topRow = 0;
	maxRows = 0;
	txtPicts = freeText(txtPicts);
}

static short lTextWidthInPix(CDC *rDC, unCH *st, long len, FONTINFO *fnt) {
	LOGFONT lfFont;
	CFont l_font;
	CSize cCharsWidth;
	SIZE  CharsWidth;
	short res;

	if (len == 0)
		return(0);
	SetLogfont(&lfFont, fnt, NULL);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = rDC->SelectObject(&l_font);
	if (GetTextExtentPointW(rDC->m_hDC, st, len, &CharsWidth) != 0)
		res = (short)CharsWidth.cx;
	else
		res = 0;
	rDC->SelectObject(pOldFont);
	l_font.DeleteObject();
	return(res);
}

static long lGetFontHeight(CDC *rDC, FONTINFO *fontInfo) {
	LOGFONT lfFont;
	CFont l_font;
	CSize CharsWidth;

	if (rDC == NULL)
		return(0);
	SetLogfont(&lfFont, fontInfo, NULL);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = rDC->SelectObject(&l_font);
	CharsWidth = rDC->GetTextExtent(_T("IjWfFg"), 6);
	rDC->SelectObject(pOldFont);
	l_font.DeleteObject();
	return(CharsWidth.cy + FLINESPACE);	
}

MovieTXTInfo *cTxtBullet::SelectTextPosition(CPoint point) {
	int				h;
	int				v;
	int				offset;
	MovieTXTInfo	*Picts;
	RECT			rect;

	if (txtPicts == NULL)
		return(NULL);

	h = point.x;
	v = point.y;
	GetClientRect(&rect);
	if (v < 0 || h > rect.right - rect.left - SCROLL_BAR_SIZE)
		return(NULL);
	Picts = txtPicts;
	for (; Picts != NULL; Picts=Picts->nextPict) {
		if (topRow == Picts->row)
			break;
	}
	if (Picts == NULL)
		return(NULL);
	offset = Picts->dstRect.top;
	for (; Picts != NULL; Picts=Picts->nextPict) {
		if ((v > (Picts->dstRect.top-offset) && v < (Picts->dstRect.bottom-offset)) &&
			(h > Picts->dstRect.left && h < Picts->dstRect.right))
			break;
	}
	return(Picts);
}

HBRUSH cTxtBullet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_DLG) {
		hbr = m_brush;
	}
	return hbr;
}

void cTxtBullet::SetTextScrollControl(void) {
	SCROLLINFO sb;
	CWnd* win;
	CDC* tDC;

	tDC = CWnd::GetDC();
	if (tDC == NULL)
		return;
	win = tDC->GetWindow();
	if (win == NULL)
		return;
	sb.cbSize = sizeof(SCROLLINFO);
	sb.fMask = SIF_DISABLENOSCROLL | SIF_RANGE;
	sb.nMin = 0;
// Vertical bar
	if (topRow == 0 && max_num_rows == maxRows) {
		sb.nMax = 0;
		win->SetScrollInfo(SB_VERT, &sb, true);
	} else {
		sb.nMax = maxRows-1;
		win->SetScrollInfo(SB_VERT, &sb, true);
		win->SetScrollPos(SB_VERT, (int)topRow, true);
	}
}

void cTxtBullet::TxtHandleVScroll(UINT nSBCode, UINT nPos, CWnd* win) {
	switch (nSBCode) {
		case SB_LINEUP:
			if (topRow > 0) {
				topRow--;
				Invalidate(TRUE);
			}
			break;

		case SB_LINEDOWN:
			if (topRow < maxRows - 1) {
				topRow++;
				Invalidate(TRUE);
			}
			break;
	
		case SB_PAGEUP:
			topRow -= (max_num_rows / 2);
			if (topRow < 0)
				topRow = 0;
			Invalidate(TRUE);
			break;

		case SB_PAGEDOWN:
			topRow += (max_num_rows / 2);
			if (topRow >= maxRows) 
				topRow = maxRows - 1;
			Invalidate(TRUE);
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			topRow = nPos;
			if (topRow >= maxRows)
				topRow = maxRows - 1;
			Invalidate(TRUE);
			break;	
	}
	SetTextScrollControl();
}

void cTxtBullet::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	CDC* tDC;

	tDC = CWnd::GetDC();
	CWnd* win = tDC->GetWindow();
	TxtHandleVScroll(nSBCode, nPos, win);
//	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}


void cTxtBullet::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	CDC* tDC;

	tDC = CWnd::GetDC();
	CWnd* win = tDC->GetWindow();
//	HandleHScrollBar(nSBCode, nPos, win);
//	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL cTxtBullet::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	CDC* tDC;

	tDC = CWnd::GetDC();
	CWnd* win = tDC->GetWindow();
	if (zDelta > 0) {
		TxtHandleVScroll(SB_LINEUP, 0, win);
		TxtHandleVScroll(SB_LINEUP, 0, win);
		TxtHandleVScroll(SB_LINEUP, 0, win);
		TxtHandleVScroll(SB_LINEUP, 0, win);
	} else if (zDelta < 0) {
		TxtHandleVScroll(SB_LINEDOWN, 0, win);
		TxtHandleVScroll(SB_LINEDOWN, 0, win);
		TxtHandleVScroll(SB_LINEDOWN, 0, win);
		TxtHandleVScroll(SB_LINEDOWN, 0, win);
	}
	//return CView::OnMouseWheel(nFlags, zDelta, pt);
	return true;
}
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
void cTxtBullet::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (nFlags & MK_CONTROL) {
	} else if (nFlags & MK_SHIFT) {
	} else {
	}
	SetCapture();       // Capture the mouse until button up.
//	CView::OnLButtonDown(nFlags, point);
}

void cTxtBullet::OnLButtonUp(UINT nFlags, CPoint point) 
{
	MovieTXTInfo *Pict;
	movInfo			mvRec;

	if (GetCapture() != this)
		return; // If this window (view) didn't capture the mouse,

	if (nFlags & MK_CONTROL) {
		Pict = SelectTextPosition(point);
		if (Pict != NULL && (Pict->MBeg != 0L || Pict->MEnd != 0L)) {
/* 2011-12-08 Creates a problem
			if (global_df != NULL) {
				DrawCursor(0);
				SaveUndoState(FALSE);
				move_cursor(Pict->MBeg+1, txtMovieFile, TRUE, FALSE);
				global_df->LeaveHighliteOn = TRUE;
				DrawCursor(1);
			}
*/
			strcpy(mvRec.MovieFile, txtMovieFile);
			strcpy(mvRec.rMovieFile, txtrMovieFile);
			mvRec.MBeg = Pict->MBeg;
			mvRec.MEnd = Pict->MEnd;
			PlayMovie(&mvRec, NULL, FALSE);
		}
	} else if (nFlags & MK_SHIFT) {
	} else {
	}
	ReleaseCapture();   // Release the mouse capture established at

//	CView::OnLButtonUp(nFlags, point);
}
/*
void cTxtBullet::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	CDC* tDC;

	tDC = CWnd::GetDC();
	CWnd* win = tDC->GetWindow();

	if (nFlags & MK_CONTROL) {
//		StartSelectCursorPosition(point, 2);
	} else if (nFlags & MK_SHIFT) {
//		StartSelectCursorPosition(point, 1);
	} else {
//		StartSelectCursorPosition(point, 0);
	}

	SetCapture();       // Capture the mouse until button up.
//	CView::OnLButtonDblClk(nFlags, point);
}

void cTxtBullet::OnMouseMove(UINT nFlags, CPoint point) 
{
	RECT  wDim;
	POINT mPos;

	if (GetCursorPos(&mPos)) {
		GetWindowRect(&wDim);
		if (mPos.y < wDim.top+wDim.bottom)
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
	} else 
		SetCursor(LoadCursor(NULL, IDC_IBEAM));

	if (GetCapture() != this)
		return; // If this window (view) didn't capture the mouse,
				// then the user isn't drawing in this window.

	CDC* tDC;

	tDC = CWnd::GetDC();
	CWnd* win = tDC->GetWindow();

	if (nFlags & MK_CONTROL) {
//		MidSelectCursorPosition(point, 2);
	} else if (nFlags & MK_SHIFT) {
//		MidSelectCursorPosition(point, 1);
	} else {
//		if (MidSelectCursorPosition(point, 0) == 1)
//			isMouseButtonDn = GlobalDC->GetWindow();
	}

//	CView::OnMouseMove(nFlags, point);
}

void cTxtBullet::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_UP) {
//		if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_FROM)) {
			if (topRow > 0) {
				topRow--;
				Invalidate(TRUE);
			}
//		}
	} else if (nChar == VK_DOWN) {
		if (topRow < maxRows - 1) {
			topRow++;
			Invalidate(TRUE);
		}
	}
}

void cTxtBullet::OnKillFocus(CWnd* pNewWnd) 
{
	CDC* tDC;

	tDC = CWnd::GetDC();
	CWnd* win = tDC->GetWindow();
}
*/
BOOL cTxtBullet::PreTranslateMessage(MSG* pMsg) {
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_UP) {
			if (topRow > 0) {
				topRow--;
				Invalidate(TRUE);
			}
		} else if (pMsg->wParam == VK_DOWN) {
			if (topRow < maxRows - 1) {
				topRow++;
				Invalidate(TRUE);
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
/*
LRESULT cTxtBullet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	int i;
	if (message == WM_KEYDOWN && wParam == VK_DOWN) {
		i = 5;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
*/
void cTxtBullet::OnPaint() 
{
	int				i, len, col, cCol, num_rows;
	int				top, left, sLeft, hight, width;
	char			attChanged;
	int				row, maxRowHight;
	double			ThumbnailsWidth;
	unCH			*sData;
	RECT			rect;
	AttTYPE			*sAtts;
	AttTYPE			oldState;
	LOGFONT			lfFont;
	CFont			l_font;
	CDC				*rDC;
	CSize			CW;
	SIZE			wCW;
	RGBColor		theColor;
	COLORTEXTLIST	*tierColor = NULL;
	COLORREF		oldTxtColor;
	MovieTXTInfo	*Picts, *tPicts;

	CClientDC dc(this);
	CDialog::OnPaint();
	UpdateData(TRUE);
	if (txtPicts == NULL) {
		UpdateData(FALSE);
		return;
	}
	rDC = CWnd::GetDC();
	GetClientRect(&rect);
	hight = rect.bottom - rect.top;// - SCROLL_BAR_SIZE;
	width = rect.right - rect.left - SCROLL_BAR_SIZE;
	maxRows = 0;
	row = 0;
	for (Picts=txtPicts; Picts != NULL; Picts=Picts->nextPict) {
		Picts->row = row;
		if (Picts->isNewLine)
			row++;		
	}
	maxRows = row;
	if (topRow >= maxRows)
		topRow = maxRows - 1;
	oldTxtColor = rDC->GetTextColor();
	Picts = txtPicts;
	for (; Picts != NULL; Picts=Picts->nextPict) {
		if (topRow == Picts->row)
			break;
	}
	if (Picts == NULL) {
		UpdateData(FALSE);
		return;
	}
	SetLogfont(&lfFont, &Picts->Font, NULL);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = rDC->SelectObject(&l_font);
	num_rows = 0;
	for (top=0; top < hight && Picts != NULL; ) {
		maxRowHight = 0;
		for (tPicts=Picts; tPicts != NULL; tPicts=tPicts->nextPict) {
			if (tPicts->text != NULL) {
				tPicts->Font.FHeight = (short)lGetFontHeight(rDC, &tPicts->Font);
				if (maxRowHight < tPicts->Font.FHeight)
					maxRowHight = tPicts->Font.FHeight;
			}
			if (tPicts->pict != NULL) {
				if (maxRowHight < (int)(ThumbnailsHight))
					maxRowHight = (int)(ThumbnailsHight);
			}
			if (tPicts->isNewLine)
				break;
		}
		left = 0;
		do {
			if (Picts->text != NULL) {
				sLeft = left;
				cCol = 0;
				sData = Picts->text;
				sAtts = Picts->atts;
				len = strlen(sData);
				for (i=0; i < len; i++) {
					if (is_colored(sAtts[i]))
						sAtts[i] = set_color_to_0(sAtts[i]);
				}
				if (text_bullets_RootColorText != NULL)
					tierColor = FindColorKeywordsBounds(text_bullets_RootColorText,sAtts,sData,0,len,tierColor);
				oldState = sAtts[0];
				for (col=0; col <= len; col++) {
					if (oldState != sAtts[col] || col == len) {
						attChanged = FALSE;
						if (is_underline(sAtts[cCol])) {
							if (!lfFont.lfUnderline) {
								lfFont.lfUnderline = TRUE;
								attChanged = TRUE;
							}
						} else if (lfFont.lfUnderline) {
							lfFont.lfUnderline = FALSE;
							attChanged = TRUE;
						}
						if (is_italic(sAtts[cCol])) {
							if (!lfFont.lfItalic) {
								lfFont.lfItalic = TRUE;
								attChanged = TRUE;
							}
						} else if (lfFont.lfItalic) {
							lfFont.lfItalic = FALSE;
							attChanged = TRUE;
						}
						if (is_bold(sAtts[cCol])) {
							if (lfFont.lfWeight != FW_BOLD) {
								lfFont.lfWeight = FW_BOLD;
								attChanged = TRUE;
							}
						} else if (lfFont.lfWeight == FW_BOLD) {
							lfFont.lfWeight = FW_NORMAL;
							attChanged = TRUE;
						}
						if (attChanged) {
							rDC->SelectObject(pOldFont);
							l_font.DeleteObject();
							l_font.CreateFontIndirect(&lfFont);
							rDC->SelectObject(&l_font);
						}
						if (is_colored(sAtts[cCol])) {
							if (SetKeywordsColor(text_bullets_RootColorText, cCol, &theColor))
								rDC->SetTextColor(RGB(theColor.red,theColor.green,theColor.blue));
						}
						if (is_word_color(sAtts[cCol])) {
							char color;

							color = get_color_num(sAtts[cCol]);
							if (color == blue_color) {
								rDC->SetTextColor(RGB(0,0,240));
							} else if (color == red_color) {
								rDC->SetTextColor(RGB(240,0,0));
							} else if (color == green_color) {
								rDC->SetTextColor(RGB(0,192,0));
							} else { // if (color == magenta_color) {
								rDC->SetTextColor(RGB(192,0,192));
							}
						}
						if (sData[cCol] != EOS) {
							TextOutW(rDC->m_hDC, left, top+maxRowHight-Picts->Font.FHeight, sData+cCol, col-cCol);
							if (GetTextExtentPointW(rDC->m_hDC,sData+cCol,col-cCol,&wCW) != 0) {
								CW.cx = (short)wCW.cx;
								CW.cy = (short)wCW.cy;
							} else
								CW.cx = 0;
						} else
							CW.cx = 0;
						left += CW.cx;
						if (is_colored(sAtts[cCol]) || is_error(sAtts[cCol]) || is_word_color(sAtts[cCol]))
							rDC->SetTextColor(oldTxtColor);
						cCol = col;
						oldState = sAtts[col];
					}
				}
				if (Picts->pict == NULL) {
					Picts->dstRect.top		= top;
					Picts->dstRect.bottom	= top + maxRowHight;
					Picts->dstRect.left		= sLeft;
					Picts->dstRect.right	= left;
				}
			}
			if (Picts->pict != NULL) {
				Picts->dstRect.top		= top;
				Picts->dstRect.bottom	= top + maxRowHight;
				Picts->dstRect.left		= left;
				ThumbnailsWidth = (double)(maxRowHight) * Picts->aspectRetio;
				Picts->dstRect.right	= left + (int)(ThumbnailsWidth);
				left = Picts->dstRect.right + 2;
				DrawPicture(Picts->pict, &Picts->dstRect);
			}
			if (Picts->isNewLine) {
				Picts = Picts->nextPict;
				break;
			} else
				Picts = Picts->nextPict;
		} while (left < width && Picts != NULL) ;
		top += maxRowHight + 2;
		num_rows++;
	}
	if (topRow == 0 || top >= hight || num_rows > max_num_rows)
		max_num_rows = num_rows;
	rDC->SelectObject(pOldFont);
	l_font.DeleteObject();
	UpdateData(FALSE);
	SetTextScrollControl();
}

void cTxtBullet::OnSize(UINT nType, int cx, int cy) 
{
	RECT client;

	CDialog::OnSize(nType, cx, cy);
	GetClientRect(&client);
	if (client.top < 0) {
		client.bottom = client.bottom - client.top;
		client.top = 0;
	}
	if (client.left < 0) {
		client.right = client.right - client.left;
		client.left = 0;
	}
}

Movie txtMovieOpen(){
	unCH		wrMovieFile[FNSize];
	OSType		DRType;
	Handle		DR = NULL;
	short		RID = 0;
	CFStringRef	theStr = NULL;
	Movie		theMovie;
	OSErr		err = noErr;

	u_strcpy(wrMovieFile, txtrMovieFile, FNSize);
	theStr = CFStringCreateWithCharacters(kCFAllocatorDefault, (UniChar *)wrMovieFile, strlen(wrMovieFile));
	err = QTNewDataReferenceFromFullPathCFString(theStr, kQTNativeDefaultPathStyle, 0, &DR, &DRType);
	CFRelease(theStr);
	if (err != noErr) {
		return NULL;
	}
	err = NewMovieFromDataRef(&theMovie, newMovieActive, &RID, DR, DRType);
	if (err != noErr) {
		DisposeHandle(DR);
		return NULL;
	}
	return theMovie;
}

static long txt_conv_from_msec_mov(double num, double ts) {
	long t;

	num /= 1000.0000;
	num *= ts;
	t = (long)num;
	if (num > 0.0 && t < 0L)
		t = 0x7fffffff;
	return(t);
}

static int ShowText(FNType *name, long totalPicts) {
	int				perc;
	long			cPict;
	unsigned long	percl; 
	float			aspectRetio, t1, t2;
	TimeScale		ts;
	TimeValue		pos;
	Rect			MvBounds;
	Movie			thisMovie;
	MovieTXTInfo	*Picts;
	FNType			oldfName[FNSize];

	thisMovie = NULL;
	aspectRetio = 0.0;
/*
	if (!isMovieAvialable) {
		do_warning("QuickTime failure!", 0);
		txtPicts = freeText(txtPicts);
		return(1);
	}
*/
	if (!PlayingSound && !PlayingContSound && !PlayingContMovie) {
		if (totalPicts > 0) {
			sprintf(templineC, "Loading %ld movie clips...", totalPicts);
			initProgressBar(cl_T(templineC));
		}
		cPict = 0L;
		oldfName[0] = EOS;
		for (Picts=txtPicts; Picts != NULL; Picts=Picts->nextPict) {
			if ((Picts->MBeg != 0L || Picts->MEnd != 0L) && Picts->isShowPict) {
				if (strcmp(oldfName, txtrMovieFile)) {
					if (thisMovie != NULL)
						DisposeMovie(thisMovie);
					thisMovie = txtMovieOpen();
					if (thisMovie == NULL) {
						sprintf(templineC3, "Error opening Movie: %s", txtrMovieFile);
						do_warning(templineC3, 0);
						txtPicts = freeText(txtPicts);
						return(1);
					}
					GetMovieBox(thisMovie, &MvBounds);
					t1 = (float)(MvBounds.right - MvBounds.left);
					t2 = (float)(MvBounds.bottom - MvBounds.top);
					aspectRetio = (t1 / t2);
				}
				strcpy(oldfName, txtrMovieFile);
				ts = GetMovieTimeScale(thisMovie);
				Picts->aspectRetio = aspectRetio;
				pos = txt_conv_from_msec_mov(Picts->MBeg, ts);
				Picts->pict = GetMoviePict(thisMovie, pos);
				cPict++;
				percl = (100L * cPict) / totalPicts;
				perc = (int)percl;
				setCurrentProgressBarValue(perc);
			}
		}
		deleteProgressBar();
		if (thisMovie != NULL)
			DisposeMovie(thisMovie);
	}
	topRow = 0;
	maxRows = 0;
	return(1);
}

static char FindMovieInfo(unCH *line, int index, movInfo *tempMovie) {
	register int i;
	long beg, end;
	unCH buf[FILENAME_MAX];

	index++;
	if (is_unCH_digit(line[index])) {
		if (tempMovie->MovieFile[0] == EOS)
			return(-2);
	} else if (!uS.partwcmp(line+index, SOUNDTIER) && !uS.partwcmp(line+index, REMOVEMOVIETAG))
		return(0);
	else
		return(-1);
	if (line[index] == EOS || line[index] == HIDEN_C)
		return(0);
	if (mOpenMovieFile(tempMovie) != 0)
		return(-3);
	for (; line[index] && !is_unCH_digit(line[index]) && line[index] != HIDEN_C; index++) ;
	if (!is_unCH_digit(line[index]))
		return(-1);
	for (i=0; line[index] && is_unCH_digit(line[index]); index++)
		buf[i++] = line[index];
	buf[i] = EOS;
	beg = atol(buf);
	for (; line[index] && !is_unCH_digit(line[index]) && line[index] != HIDEN_C; index++) ;
	if (!is_unCH_digit(line[index]))
		return(-1);
	for (i=0; line[index] && is_unCH_digit(line[index]); index++)
		buf[i++] = line[index];
	buf[i] = EOS;
	end = atol(buf);
	tempMovie->MBeg = beg;
	tempMovie->MEnd = end;
	tempMovie->nMovie = NULL;
	return(1);
}

static void extractMediaName(unCH *line) {
	char qf;
	unCH *s;
	int i;
	
	strcpy(line, line+strlen(MEDIAHEADER));
	for (i=0; isSpace(line[i]); i++) ;
	if (i > 0)
		strcpy(line, line+i);
	qf = FALSE;
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '"')
			qf = !qf;
		if (line[i] == ',' || (!qf && isSpace(line[i]))) {
			line[i] = EOS;
			break;
		}
	}
	uS.remblanks(line);
	s = strrchr(line, '.');
	if (s != NULL)
		*s = EOS;
}

static char SetTextAttW(unCH *st, AttTYPE *att) {
	char c;
	char found = FALSE;

	c = (char)st[1];
	if (c == underline_start) {
		*att = set_underline_to_1(*att);
		found = TRUE;
	} else if (c == underline_end) {
		*att = set_underline_to_0(*att);
		found = TRUE;
	} else if (c == italic_start) {
		*att = set_italic_to_1(*att);
		found = TRUE;
	} else if (c == italic_end) {
		*att = set_italic_to_0(*att);
		found = TRUE;
	} else if (c == bold_start) {
		*att = set_bold_to_1(*att);
		found = TRUE;
	} else if (c == bold_end) {
		*att = set_bold_to_0(*att);
		found = TRUE;
	} else if (c == error_start) {
		*att = set_error_to_1(*att);
		found = TRUE;
	} else if (c == error_end) {
		*att = set_error_to_0(*att);
		found = TRUE;
	} else if (c == blue_start) {
		*att = set_color_num(blue_color, *att);
		found = TRUE;
	} else if (c == red_start) {
		*att = set_color_num(red_color, *att);
		found = TRUE;
	} else if (c == green_start) {
		*att = set_color_num(green_color, *att);
		found = TRUE;
	} else if (c == magenta_start) {
		*att = set_color_num(magenta_color, *att);
		found = TRUE;
	} else if (c == color_end) {
		*att = zero_color_num(*att);
		found = TRUE;
	}
	if (found) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

static void convertTabs(unCH *templine, unCH *templine1) {
	register long i;
	register long lineChr;
	register long colWin;
	long	newcol;
	AttTYPE att;

	i = 0L;
	att = 0;
	colWin = 0L;
	lineChr = 0L;
	while (templine1[lineChr]) {
		if (templine1[lineChr] == '\t') {
			newcol = (((colWin / TabSize) + 1) * TabSize);
			for (; colWin < newcol; colWin++) {
				templine[i] = ' ';
				tempAtt[i] = att;
				i++;
			}
		} else {
			colWin++;
			if (templine1[lineChr] == ATTMARKER && SetTextAttW(templine1+lineChr, &att)) {
				lineChr++;
			} else {
				templine[i] = templine1[lineChr];
				tempAtt[i] = att;
				i++;
			}
		}
		lineChr++;
	}
	templine[i] = EOS;
}
static void DrawMouseCursor(int i) {
}

int DisplayText(char *fname) {
	int			i, bc, ec, len;
	char		res, isCAFontSet;
	char		txtMovieFileC[FILENAME_MAX];
	long		ln, totalPicts;
	FILE		*fp;
	myFInfo 	*saveGlobal_df;
	movInfo		tempMovie;
	MovieTXTInfo *cPict, *tPicts;
	FNType mDirPathName[FNSize];
	FNType mFileName[FNSize];
	NewFontInfo finfo;
	NewFontInfo tFInfo;
	txtPicts = freeText(txtPicts);
	if (txtDial != NULL)
		txtDial->OnCancel();
	extractPath(mDirPathName, fname);
	extractFileName(mFileName, fname);
	if ((fp = fopen(fname, "r")) == NULL) {
		DrawMouseCursor(1);
		sprintf(templineC, "Can't open file \"%s\".", fname);
		do_warning(templineC, 0);
		return(0);
	}
	strcpy(finfo.fontName, DEFAULT_FONT);
	finfo.fontSize = DEFAULT_SIZE;
	finfo.fontId = DEFAULT_ID;
	finfo.orgFType = getFontType(finfo.fontName, TRUE);
	finfo.CharSet = NSCRIPT;
	finfo.Encod = finfo.orgEncod = 1252;
	finfo.fontTable = NULL;
	DrawMouseCursor(2);
	DrawSoundCursor(0);
	isCAFontSet = FALSE;
	totalPicts = 0L;
	txtMovieFile[0] = EOS;
	txtMovieFileC[0] = EOS;
	txtrMovieFile[0] = EOS;
	fgets_cr_lc = '\0';
	ln = 0;
	while (fgets_cr(templineC, 256, fp)) {
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE))
			continue;
		if (uS.partcmp(templineC, CKEYWORDHEADER, FALSE, FALSE)) {
			FreeColorText(text_bullets_RootColorText);
			text_bullets_RootColorText = NULL;
			text_bullets_RootColorText = createColorTextKeywordsList(text_bullets_RootColorText, templineC);
			continue;
		}
		if (uS.partcmp(templineC, FONTHEADER, FALSE, FALSE)) {
			strcpy(templineC, templineC + strlen(FONTHEADER));
			for (i = 0; isSpace(templineC[i]); i++);
			if (SetNewFont(templineC + i, EOS, &tFInfo))
				copyNewFontInfo(&finfo, &tFInfo);
			continue;
		}
		ln++;
		u_strcpy(templine1, templineC, UTTLINELEN);
		if (uS.partcmp(templine1, MEDIAHEADER, FALSE, FALSE)) {
			extractMediaName(templine1);
			strcpy(txtMovieFile, templine1);
			u_strcpy(txtMovieFileC, txtMovieFile, FILENAME_MAX);
			continue;
		}
		uS.remblanks(templine1);
		bc = 0;
		ec = 0;
		convertTabs(templine, templine1);
		if (!isCAFontSet) {
			if (!uS.partcmp(templine, "%pic:", FALSE, FALSE)) {
				isCAFontSet = TRUE;
				strcpy(finfo.fontName, "CAfont");
				finfo.fontSize = -13;
				finfo.fontId = 7;
				finfo.orgFType = getFontType(finfo.fontName, TRUE);
				finfo.CharSet = 0;
				finfo.Encod = finfo.orgEncod = 1252;
				finfo.fontTable = NULL;
				for (tPicts = txtPicts; tPicts != NULL; tPicts = tPicts->nextPict) {
					strcpy(tPicts->Font.FName, finfo.fontName);
					tPicts->Font.FSize = finfo.fontSize;
					tPicts->Font.CharSet = finfo.CharSet;
					tPicts->Font.FHeight = finfo.charHeight;
					tPicts->Font.Encod = finfo.Encod;
				}
			}
		}
		while (templine[ec]) {
			if (templine[ec] == HIDEN_C) {
				strcpy(tempMovie.MovieFile, txtMovieFile);
				strcpy(tempMovie.rMovieFile, mDirPathName);
				saveGlobal_df = global_df;
				global_df = NULL;
				res = FindMovieInfo(templine, ec, &tempMovie);
				global_df = saveGlobal_df;
				if (res == -1) {
					DrawMouseCursor(1);
					sprintf(templineC, "Media bullet is corrupted or old format on line %ld.", ln);
					do_warning(templineC, 0);
					return(0);
				} else if (res == -2) {
					DrawMouseCursor(1);
					sprintf(templineC, "Can't find \"%s\" header tier in %%txt: bullet file.", MEDIAHEADER);
					do_warning(templineC, 0);
					return(0);
				} else if (res == -3) {
					DrawMouseCursor(1);
					sprintf(templineC, "Can't locate media file \"%s\".", txtMovieFileC);
					do_warning(templineC, 0);
					return(0);
				} else if (res == 1) {
					if (txtPicts == NULL) {
						txtPicts = NEW(MovieTXTInfo);
						cPict = txtPicts;
					} else {
						cPict->nextPict = NEW(MovieTXTInfo);
						cPict = cPict->nextPict;
					}
					if (cPict == NULL) {
						txtPicts = freeText(txtPicts);
						DrawMouseCursor(1);
						sprintf(templineC, "Out of memory.");
						do_warning(templineC, 0);
						return(0);
					}
					cPict->text = NULL;
					cPict->isShowPict = FALSE;
					cPict->isNewLine = FALSE;
					cPict->atts = NULL;
					cPict->MBeg = tempMovie.MBeg;
					cPict->MEnd = tempMovie.MEnd;
					cPict->row = 0;
					cPict->pict = NULL;
					copyNewToFontInfo(&cPict->Font, &finfo);
					cPict->nextPict = NULL;
					strcpy(txtrMovieFile, tempMovie.rMovieFile);
					len = ec - bc + 1;
					if (!uS.partcmp(templine, "%pic:", FALSE, FALSE)) {
						len++;
					}
					cPict->text = (unCH *)malloc((len)*sizeof(unCH));
					if (cPict->text == NULL) {
						txtPicts = freeText(txtPicts);
						DrawMouseCursor(1);
						sprintf(templineC, "Out of memory.");
						do_warning(templineC, 0);
						return(0);
					}
					cPict->text[0] = EOS;
					cPict->atts = (AttTYPE *)malloc((len)*sizeof(AttTYPE));
					if (cPict->atts == NULL) {
						txtPicts = freeText(txtPicts);
						DrawMouseCursor(1);
						sprintf(templineC, "Out of memory.");
						do_warning(templineC, 0);
						return(0);
					}
					for (i = 0; bc < ec; bc++) {
						cPict->text[i] = templine[bc];
						cPict->atts[i] = tempAtt[bc];
						i++;
					}
					if (!uS.partcmp(templine, "%pic:", FALSE, FALSE)) {
						cPict->text[i] = 0x2022;
						cPict->atts[i] = 0;
						i++;
						cPict->isShowPict = FALSE;
					} else if (PlayingSound || PlayingContSound || PlayingContMovie) {
						cPict->text[i] = 0x2022;
						cPict->atts[i] = 0;
						i++;
						cPict->isShowPict = FALSE;
					} else {
						totalPicts++;
						cPict->isShowPict = TRUE;
					}
					cPict->text[i] = EOS;
				}
				bc = ec + 1;
				for (; templine[bc] != HIDEN_C && templine[bc] != EOS; bc++);
				if (templine[bc] == EOS) {
					DrawMouseCursor(1);
					sprintf(templineC, "Unmatched bullets found in %%txt: bullet file.");
					do_warning(templineC, 0);
					return(0);
				}
				bc++;
				ec = bc;
			} else
				ec++;
		}
		if (bc != ec || bc == 0) {
			ec = strlen(templine);
			if (txtPicts == NULL) {
				txtPicts = NEW(MovieTXTInfo);
				cPict = txtPicts;
			} else {
				cPict->nextPict = NEW(MovieTXTInfo);
				cPict = cPict->nextPict;
			}
			if (cPict == NULL) {
				txtPicts = freeText(txtPicts);
				DrawMouseCursor(1);
				sprintf(templineC, "Out of memory.");
				do_warning(templineC, 0);
				return(0);
			}
			cPict->text = NULL;
			cPict->isShowPict = FALSE;
			cPict->isNewLine = TRUE;
			cPict->MBeg = 0L;
			cPict->MEnd = 0L;
			cPict->row = 0;
			cPict->pict = NULL;
			copyNewToFontInfo(&cPict->Font, &finfo);
			cPict->nextPict = NULL;
			cPict->atts = NULL;
			len = ec - bc + 1;
			cPict->text = (unCH *)malloc((len)*sizeof(unCH));
			if (cPict->text == NULL) {
				txtPicts = freeText(txtPicts);
				DrawMouseCursor(1);
				sprintf(templineC, "Out of memory.");
				do_warning(templineC, 0);
				return(0);
			}
			cPict->text[0] = EOS;
			cPict->atts = (AttTYPE *)malloc((len)*sizeof(AttTYPE));
			if (cPict->atts == NULL) {
				txtPicts = freeText(txtPicts);
				DrawMouseCursor(1);
				sprintf(templineC, "Out of memory.");
				do_warning(templineC, 0);
				return(0);
			}
			for (i = 0; bc < ec; bc++) {
				cPict->text[i] = templine[bc];
				cPict->atts[i] = tempAtt[bc];
				i++;
			}
			cPict->text[i] = EOS;
		} else if (cPict != NULL) {
			cPict->isNewLine = TRUE;
		}
	}
	DrawMouseCursor(1);
	if (1 || txtPicts != NULL) {
		ShowText(mFileName, totalPicts);
		txtDial = new cTxtBullet(AfxGetApp()->m_pMainWnd);
	}
	return(0);
}

#else // TEXT_BULLETS_PICTS

int DisplayText(char *fname) {
	DisplayFileText(fname);
	return(0);
}

#endif // else TEXT_BULLETS_PICTS
