// W_Pict.cpp : implementation file
//

#include "ced.h"
#include "MMedia.h"
#include "W95_Pict.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

extern struct DefWin PictWinSize;

char pictName[_MAX_PATH+_MAX_FNAME];
char isShowPict = FALSE;
char isOverRidePicName = FALSE;
CPict *pictDial = NULL;


/////////////////////////////////////////////////////////////////////////////
// CPict dialog

CPict::CPict(CString pathName, CWnd* pParent /*=NULL*/)
	: CDialog(CPict::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPict)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	myPict = NULL;
	filename = pathName;
	m_isPictureLoadFailed = TRUE;
	m_isFirstPaint = TRUE;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	Create(IDD, pParent);
}


void CPict::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPict)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPict, CDialog)
	//{{AFX_MSG_MAP(CPict)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPict::OnInitDialog() {
	HRESULT	hr;

	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	if (PictWinSize.top || PictWinSize.width || PictWinSize.height || PictWinSize.left) {
		CRect lpRect;

		long width, hight;

		GetWindowRect(&lpRect);
		width = lpRect.right - lpRect.left;
		hight = lpRect.bottom - lpRect.top;
		lpRect.top = PictWinSize.top;
		lpRect.bottom = lpRect.top + hight;
		lpRect.left = PictWinSize.left;
		lpRect.right = lpRect.left + width;
		AdjustWindowSize(&lpRect);
		MoveWindow(&lpRect, TRUE);

/*
		lpRect.top = PictWinSize.top;
		lpRect.bottom = PictWinSize.height;
		lpRect.left = PictWinSize.left;
		lpRect.right = PictWinSize.width;
		MoveWindow(&lpRect, TRUE);
*/
	}
	isOverRidePicName = FALSE;
	if (PlayingContMovie)
		isShowPict = TRUE;
	myPict = new CImage();
	if (myPict != NULL) {
		hr = myPict->Load(filename);
		if (hr == S_OK)
			m_isPictureLoadFailed = FALSE;
		else
			m_isPictureLoadFailed = TRUE;
	}
	Invalidate(TRUE);
	return TRUE;
}

BOOL CPict::DestroyWindow() 
{
	if (myPict != NULL) {
		delete myPict;
		myPict = NULL;
	}
	isOverRidePicName = FALSE;
	isShowPict = FALSE;
	return CDialog::DestroyWindow();
}

void CPict::OnCancel() {
	CRect lpRect;

	this->GetWindowRect(&lpRect);
	DestroyWindow();
}

void CPict::OnDestroy() 
{
	CRect lpRect;

	this->GetWindowRect(&lpRect);
	CDialog::OnDestroy();
	if (pictDial != NULL) {
		delete pictDial;
		pictDial = NULL;
	}
	pictName[0] = EOS;
	isOverRidePicName = FALSE;
	isShowPict = FALSE;
	PictWinSize.top = lpRect.top;
	PictWinSize.left = lpRect.left;
	PictWinSize.width = lpRect.right;
	PictWinSize.height = lpRect.bottom;
	WriteCedPreference();
}

void CPict::OnPaint() 
{
//	CPaintDC	dc(this); // device context for painting
	long		heightDiff, widthDiff;
	CRect		cRect, wRect;
	double		ratio, iWidth, iHeight, wWidth, wHeight;
	CDC*		pDC;
	HRESULT		hr;

	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	} else if (m_isPictureLoadFailed == FALSE) {
		pDC = CWnd::GetDC();
		GetWindowRect(&wRect);
		GetClientRect(&cRect);
		heightDiff = (wRect.bottom - wRect.top) - (cRect.bottom - cRect.top);
		widthDiff = (wRect.right - wRect.left) - (cRect.right - cRect.left);
		if (m_isFirstPaint == TRUE) {
			cRect.right = myPict->GetWidth();
			cRect.bottom = myPict->GetHeight();
			m_isFirstPaint = FALSE;
		} else {
			iWidth = (double)myPict->GetWidth();
			iHeight = (double)myPict->GetHeight();
			ratio = iWidth / iHeight;
			wWidth = (double)cRect.right;
			wHeight = wWidth / ratio;
//			if (wHeight <= (double)iRect.bottom) {
			cRect.bottom = (long)wHeight;
//			} else {
//				wHeight = (double)iRect.bottom;
//				wWidth = wHeight * ratio;
//				iRect.right = (long)wWidth;
//			}
		}
		wRect.bottom = wRect.top + cRect.bottom + heightDiff;
		wRect.right = wRect.left + cRect.right + widthDiff;
		//	CalcWindowRect(&wRect);
		MoveWindow(&wRect, FALSE);
		hr = myPict->Draw(pDC->m_hDC, CRect(&cRect));
		CDialog::OnPaint();
	} else
		CDialog::OnPaint();
}

void CPict::OnSize(UINT nType, int cx, int cy) 
{
	CRect lpRect;

	CDialog::OnSize(nType, cx, cy);
//	UpdateData(FALSE);
/*
	if (m_isUserResize == TRUE) {
		UpdateData(TRUE);
		GetWindowRect(&lpRect);
		PictWinSize.top = lpRect.top;
		PictWinSize.left = lpRect.left;
		PictWinSize.width = lpRect.right;
		PictWinSize.height = lpRect.bottom;
		WriteCedPreference();
		UpdateData(FALSE);
	}
*/
}

int DisplayPhoto(char *fname) {
	if (!strcmp(pictName, fname) && !isOverRidePicName)
		return(0);
	isOverRidePicName = FALSE;
	strcpy(pictName, fname);
	if (pictDial != NULL)
		pictDial->OnCancel();
	pictDial = new CPict(fname, AfxGetApp()->m_pMainWnd);
	return(0);
}
