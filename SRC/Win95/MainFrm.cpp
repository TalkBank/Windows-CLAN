// MainFrm.cpp : implementation of the CMainFrame class
//

#include "ced.h"
#include "c_clan.h"

#include "MainFrm.h"
#include "Clan2Doc.h"
#include "Clan2View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char isPlayAudioFirst;
extern float scalingSize;

int CMainFrame::m_nDefCmdShow = SW_SHOWMAXIMIZED;

static TCHAR BASED_CODE szSec[] = _T("Settings");
static TCHAR BASED_CODE szShowCmd[] = _T("MainShowCmd");

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_MESSAGE(WM_APP, CMainFrame::myOnOpenMessage)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_INITMENUPOPUP()
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CMDIFrameWnd::OnCreateClient(lpcs, pContext);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) || !m_wndToolBar.LoadToolBar(IDR_MAINFRAME)) {
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
/* 13-01-99
	(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_FILE_SAVE, TRUE);
	(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_FILE_PRINT, TRUE);
*/

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

/* lxslxslxs
	float tf;
	long size;

	tf = 15;
	tf = tf * scalingSize;
	size = (long)tf;
	SIZE sizeButton, sizeImage;
	sizeImage.cy = size;
	tf = 16;
	tf = tf * scalingSize;
	size = (long)tf;
	sizeImage.cx = size;
	sizeButton.cy = sizeImage.cy + 6;
	sizeButton.cx = sizeImage.cx + 7;
	m_wndToolBar.SetSizes(sizeButton, sizeImage);
*/
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

#define MAIN_WINDOW_CLASS _T("AfxClanAppClassName")

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if (!CMDIFrameWnd::PreCreateWindow(cs))
		return FALSE;

	WNDCLASS wndcls;
	HINSTANCE hResInst = AfxFindResourceHandle(ATL_MAKEINTRESOURCE(IDR_MAINFRAME), ATL_RT_GROUP_ICON);
	HICON hIcon = ::LoadIcon(hResInst, ATL_MAKEINTRESOURCE(IDR_MAINFRAME));
	HINSTANCE hInst = AfxGetInstanceHandle();

	// see if the class already exists
	if (!GetClassInfo /*AfxCtxGetClassInfo*/(hInst, MAIN_WINDOW_CLASS, &wndcls)) {
		GetClassInfo /*AfxCtxGetClassInfo*/(hInst, cs.lpszClass, &wndcls);
		// otherwise we need to register a new class
		wndcls.lpszClassName = MAIN_WINDOW_CLASS;
		wndcls.hIcon = hIcon;
		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	cs.lpszClass = MAIN_WINDOW_CLASS;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Helpers for saving/restoring window state

static TCHAR BASED_CODE szSection[] = _T("Settings");
static TCHAR BASED_CODE szWindowPos[] = _T("WindowPos");
static TCHAR szFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");

static BOOL PASCAL NEAR ReadWindowPlacement(LPWINDOWPLACEMENT pwp)
{
	CString strBuffer = AfxGetApp()->GetProfileString(szSection, szWindowPos);
	if (strBuffer.IsEmpty())
		return FALSE;

	WINDOWPLACEMENT wp;
	int nRead = _stscanf(strBuffer, szFormat,
		&wp.flags, &wp.showCmd,
		&wp.ptMinPosition.x, &wp.ptMinPosition.y,
		&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
		&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
		&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

	if (nRead != 10)
		return FALSE;

	wp.length = sizeof wp;
	*pwp = wp;
	return TRUE;
}

static void PASCAL NEAR WriteWindowPlacement(LPWINDOWPLACEMENT pwp)
	// write a window placement to settings section of app's ini file
{
	TCHAR szBuffer[sizeof("-32767")*8 + sizeof("65535")*2];

	wsprintf(szBuffer, szFormat,
		pwp->flags, pwp->showCmd,
		pwp->ptMinPosition.x, pwp->ptMinPosition.y,
		pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
		pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
		pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);
	AfxGetApp()->WriteProfileString(szSection, szWindowPos, szBuffer);
}

/////////////////////////////////////////////////////////////////////////////

int CMainFrame::InitialShowWindow(UINT nCmdShow)
{
	WINDOWPLACEMENT wp;

	nCmdShow = AfxGetApp()->GetProfileInt(szSec, szShowCmd, nCmdShow);
	m_nDefCmdShow = nCmdShow;
	if (!ReadWindowPlacement(&wp))
	{
		ShowWindow(nCmdShow);
		return(nCmdShow);
	}
	if (nCmdShow != SW_SHOWNORMAL)
		wp.showCmd = nCmdShow;
	SetWindowPlacement(&wp);
	ShowWindow(wp.showCmd);
	return(nCmdShow);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnClose() 
{
	WINDOWPLACEMENT wp;

	AfxGetApp()->WriteProfileInt(szSec, szShowCmd, m_nDefCmdShow);
	wp.length = sizeof wp;
	if (GetWindowPlacement(&wp))
	{
		wp.flags = 0;
		if (IsZoomed())
			wp.flags |= WPF_RESTORETOMAXIMIZED;
		// and write it to the .INI file
		WriteWindowPlacement(&wp);
	}

	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIFrameWnd::OnSize(nType, cx, cy);
	
	if (!IsWindowVisible())
		return;

	switch (nType)
	{
	case SIZE_MAXIMIZED:
		m_nDefCmdShow = SW_SHOWMAXIMIZED;
		break;
	case SIZE_RESTORED:
		m_nDefCmdShow = SW_SHOWNORMAL;
		break;
	}	
	
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CMDIFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	if (pPopupMenu) {
		UINT ID;

		ID = pPopupMenu->GetMenuItemID(0);
		if (ID == ID_MODE_CODER) {
			// Mode menu
			CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
			// Get the active MDI child window.
			CMDIChildWnd *pChild = pFrame->MDIGetActive();
			// Get the active view attached to the active MDI child window.
			CClan2View *pView = (CClan2View *) pChild->GetActiveView();
			CClan2Doc* pDoc = pView->GetDocument();
			myFInfo	*tGlobal_df = global_df;

			global_df = pDoc->FileInfo;
			if (global_df == NULL) {
	 			pPopupMenu->ModifyMenu(ID_MODE_CODER, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_CODER, _T("N/A"));
			    pPopupMenu->ModifyMenu(ID_MODE_CHATMODE, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_CHATMODE, _T("N/A"));
	 			if (isChatLineNums)
					pPopupMenu->ModifyMenu(ID_MODE_CAMODE, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_MODE_CAMODE, _T("Show line numbers"));
				else
					pPopupMenu->ModifyMenu(ID_MODE_CAMODE, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_CAMODE, _T("Show line numbers"));
	 			pPopupMenu->ModifyMenu(ID_MODE_SOUNDMODE, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_SOUNDMODE, _T("N/A"));
				pPopupMenu->ModifyMenu(ID_MODE_HIDE_TIERS, MF_BYCOMMAND|MF_STRING, ID_MODE_HIDE_TIERS, _T("Hide tiers in \"0hide.cut\""));
				if (isPlayAudioFirst) {
		 			pPopupMenu->ModifyMenu(ID_MODE_UTF8, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_MODE_UTF8, _T("Play &audio media first\tCtrl+M"));
			    } else {
		 			pPopupMenu->ModifyMenu(ID_MODE_UTF8, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_UTF8, _T("Play &audio media first\tCtrl+M"));
			    }
			} else {
			    if (global_df->EditorMode) {
		 			pPopupMenu->ModifyMenu(ID_MODE_CODER, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_CODER, _T("Coder &mode\tEsc-e"));
			    } else {
		 			pPopupMenu->ModifyMenu(ID_MODE_CODER, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_MODE_CODER, _T("Coder &mode\tEsc-e"));
			    }
				if (global_df->ChatMode)
			 		pPopupMenu->ModifyMenu(ID_MODE_CHATMODE, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_MODE_CHATMODE, _T("&Chat mode\tEsc-m"));
				else
			 		pPopupMenu->ModifyMenu(ID_MODE_CHATMODE, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_CHATMODE, _T("&Chat mode\tEsc-m"));
	 			if (isChatLineNums)
					pPopupMenu->ModifyMenu(ID_MODE_CAMODE, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_MODE_CAMODE, _T("Show line numbers"));
				else
					pPopupMenu->ModifyMenu(ID_MODE_CAMODE, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_CAMODE, _T("Show line numbers"));
			    if (global_df->SoundWin) {
		 			pPopupMenu->ModifyMenu(ID_MODE_SOUNDMODE, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_MODE_SOUNDMODE, _T("&Sonic mode\tEsc-0"));
			    } else {
		 			pPopupMenu->ModifyMenu(ID_MODE_SOUNDMODE, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_SOUNDMODE, _T("&Sonic mode\tEsc-0"));
			    }
				if (isPlayAudioFirst) {
		 			pPopupMenu->ModifyMenu(ID_MODE_UTF8, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_MODE_UTF8, _T("Play &audio media first\tCtrl+M"));
			    } else {
		 			pPopupMenu->ModifyMenu(ID_MODE_UTF8, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_MODE_UTF8, _T("Play &audio media first\tCtrl+M"));
			    }
				if (global_df->headtier != NULL)
				 	pPopupMenu->ModifyMenu(ID_MODE_HIDE_TIERS, MF_BYCOMMAND|MF_STRING, ID_MODE_HIDE_TIERS, _T("Show all tiers"));
				else
					pPopupMenu->ModifyMenu(ID_MODE_HIDE_TIERS, MF_BYCOMMAND|MF_STRING, ID_MODE_HIDE_TIERS, _T("Hide tiers in \"0hide.cut\""));
			    if (global_df->ShowParags != '\002')
				 	pPopupMenu->ModifyMenu(ID_MODE_EXPEND_BULLETS, MF_BYCOMMAND|MF_STRING, ID_MODE_EXPEND_BULLETS, _T("Expand bullets\tEsc-a"));
				else
				 	pPopupMenu->ModifyMenu(ID_MODE_EXPEND_BULLETS, MF_BYCOMMAND|MF_STRING, ID_MODE_EXPEND_BULLETS, _T("Hide bullets\tEsc-a"));
			}
		    DrawMenuBar();
			global_df = tGlobal_df;
		} else if (ID == ID_MACROS_1) {
			// Macros menu
			int i;

			for (i=0; i < 10; i++) {
				if (ConstString[i] != NULL) {
					u_strcpy(templineW, ConstString[i], UTTLINELEN);
	 				pPopupMenu->ModifyMenu(i, MF_BYPOSITION|MF_STRING, ID_MACROS_1+i, templineW);
				} else
	 				pPopupMenu->ModifyMenu(i, MF_BYPOSITION|MF_STRING, ID_MACROS_1+i, _T(" "));
			}
		    DrawMenuBar();
		} else if (ID == ID_VIEW_TOOLBAR) {
			CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
			// Get the active MDI child window.
			CMDIChildWnd *pChild = pFrame->MDIGetActive();
			// Get the active view attached to the active MDI child window.
			CClan2View *pView = (CClan2View *) pChild->GetActiveView();
			CClan2Doc* pDoc = pView->GetDocument();
			myFInfo	*tGlobal_df = global_df;

			global_df = pDoc->FileInfo;
			if (global_df != NULL) {
				if (is_underline(global_df->gAtt)) {
					pPopupMenu->ModifyMenu(ID_UNDERLINE, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_UNDERLINE, _T("&Underline\tCtrl+U"));
				} else {
			 		pPopupMenu->ModifyMenu(ID_UNDERLINE, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_UNDERLINE, _T("&Underline\tCtrl+U"));
				}
				if (is_italic(global_df->gAtt)) {
					pPopupMenu->ModifyMenu(ID_ITALIC, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_ITALIC, _T("&Italic"));
				} else {
					pPopupMenu->ModifyMenu(ID_ITALIC, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_ITALIC, _T("&Italic"));
				}
				if (is_bold(global_df->gAtt)) {
					pPopupMenu->ModifyMenu(ID_BOLD, MF_BYCOMMAND|MF_STRING|MF_CHECKED, ID_BOLD, _T("&Bold"));
				} else {
					pPopupMenu->ModifyMenu(ID_BOLD, MF_BYCOMMAND|MF_STRING|MF_UNCHECKED, ID_BOLD, _T("&Bold"));
				}
			}
		    DrawMenuBar();
		} else if (ID == ID_SPEAKERS_1) {
			// Speakers menu
			int  i, j;
			unCH buf[255];
			// Mode menu
			CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
			// Get the active MDI child window.
			CMDIChildWnd *pChild = pFrame->MDIGetActive();
			// Get the active view attached to the active MDI child window.
			CClan2View *pView = (CClan2View *) pChild->GetActiveView();
			if (pView->IsKindOf(RUNTIME_CLASS(CClan2View)) == 0)
				return;
			CClan2Doc* pDoc = pView->GetDocument();
			myFInfo	*tGlobal_df = global_df;

			global_df = pDoc->FileInfo;
			for (i=1; i <= 10; i++) {
				if (i == 10) {
					if (!GetSpeakerNames(buf, 0, 254))
						strcpy(buf, " ");
					else {
						for (j=0; buf[j]; j++) {
							if (buf[j] == '\t')
								buf[j] = ' ';
						}
					}
					strcat(buf, "\t");
		 			pPopupMenu->ModifyMenu(9, MF_BYPOSITION|MF_STRING, ID_SPEAKERS_10, buf);
				} else {
					if (!GetSpeakerNames(buf, i, 254))
						strcpy(buf, " ");
					else {
						for (j=0; buf[j]; j++) {
							if (buf[j] == '\t')
								buf[j] = ' ';
						}
					}
					uS.sprintf(buf+strlen(buf), cl_T("\tCtrl+%d"), i);
		 			pPopupMenu->ModifyMenu(i-1, MF_BYPOSITION|MF_STRING, ID_SPEAKERS_1+i-1, buf);
				}
			}
		    DrawMenuBar();
			global_df = tGlobal_df;
		}

	}
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

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
	
	// Do not call CMDIFrameWnd::OnPaint() for painting messages
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

LRESULT CMainFrame::myOnOpenMessage(WPARAM w, LPARAM l) {
	char homeDirectory[256], messageFileName[256], text[2048];
	char fname[2048], *ts;
	long len;
	FILE *messageFile;
	extern char tFileBuf[];
	extern char *nameOverride, *pathOverride;
	extern char *lineNumFname;
	extern long lineNumOverride;

	if (GetEnvironmentVariableA ("USERPROFILE", homeDirectory, 255)) {
		;   /* Ready. */
	} else if (GetEnvironmentVariableA ("HOMEDRIVE", homeDirectory, 255)) {
		GetEnvironmentVariableA ("HOMEPATH", homeDirectory + strlen (homeDirectory), 255);
	} else {
		GetWindowsDirectoryA (homeDirectory, 255);
	}
	sprintf (messageFileName, "%s\\CLAN_Message.txt", homeDirectory);
	if ((messageFile = fopen (messageFileName, "r")) == NULL)
		return(true);

	text[0] = EOS;
	if (!feof(messageFile))
		fgets(text, 2048, messageFile);
	fclose (messageFile);
	if (text[0] == EOS)
		return(true);

	uS.remblanks(text);
	for (len=0L; text[len] == '*'; len++) ;
	for (; text[len] == ' '; len++) ;
	strcpy(text, text+len);
	len = strlen("File \"");
	if (strncmp(text, "File \"", len) != 0) 
		return(true);
	strcpy(text, text+len);

	for (len=0L; text[len] != '"' && text[len] != EOS; len++) ;
	if (text[len] != '"')
		return(true);
	text[len] = EOS;
	strcpy(fname, text);
	strcpy(text, text+len+1);
	if (*fname == EOS || access(fname, 0))
		return(true);

	for (len=0L; text[len] == ':' || text[len] == ' '; len++) ;
	if (len > 0L)
		strcpy(text, text+len);
	len = strlen("line ");
	if (strncmp(text, "line ", len) == 0) {
		strcpy(text, text+len);
		len = atol(text);
	} else
		len = 0L;

	strcpy(tFileBuf, fname);
	ts = strrchr(tFileBuf,'\\');
	pathOverride = wd_dir;
	if (ts != NULL) {
		*ts = EOS;
		strcpy(templineC3, tFileBuf);
		*ts = '\\';
		strcat(templineC3, "\\");
		pathOverride = templineC3;
		nameOverride = ts+1;
	} else
		nameOverride = tFileBuf;
	lineNumFname = tFileBuf;
	lineNumOverride = len;
	::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW, NULL);


	return(false); // must return false for success; true for error
}
