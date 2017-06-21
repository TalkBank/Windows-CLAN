// Clan2.cpp : Defines the class behaviors for the application.
//

#include "ced.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "Clan2Doc.h"
#include "Clan2View.h"
#include "CedDlgs.h"
#include "W95_commands.h"
#include "W95_SpCharsDialog.h"
#include "W95_WebData.h"
//#include "insertDlg.h"
#include "mmedia.h"
#include "Clan2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char isUpdateCLAN;

CWnd *isMouseButtonDn;
char firstTimeStart;
char tFileBuf[FILENAME_MAX];

static int ScreenX = 0, ScreenY = 0;

//extern CInsertDlg *insertDlg;

SHORT bApKey;
SHORT TildKey;
SHORT ApKey;
SHORT HatKey;
SHORT AtKey;
SHORT ColKey;
SHORT SemiColKey;
SHORT ComaKey;
SHORT DotKey;
SHORT StarKey;
SHORT QuestKey;
SHORT AndKey;
SHORT SlashKey;
SHORT EqSignKey;
SHORT OpSQBKey;
SHORT ClSQBKey;
SHORT OpAGBKey;
SHORT ClAGBKey;


void AdjustWindowSize(RECT *r) {
	if (ScreenX == 0 || ScreenY == 0) {
		// Get Screen Size in Pixels
		HDC     hDC;
		hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		ScreenX = GetDeviceCaps(hDC, HORZRES);
		ScreenY = GetDeviceCaps(hDC, VERTRES);
		DeleteDC(hDC);
	}

	if (ScreenX == 0 || ScreenY == 0)
		return;

	if (r->top >= ScreenY-30 || r->left >= ScreenX-30) {
		r->right = r->right - r->left;
		r->bottom = r->bottom - r->top;
		r->left = 0;
		r->top = 0;
	}
}
#ifdef _DEBUG
#undef isspace
int win95_isspace(char c) {
	return(isspace((unsigned char)c));
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CClan2App

BEGIN_MESSAGE_MAP(CClan2App, CWinApp)
	//{{AFX_MSG_MAP(CClan2App)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_EDIT_OPTIONS_CEDOPTIONS, OnEditOptionsCedoptions)
	ON_COMMAND(ID_CLAN_CLANWINDOW, OnClanClanwindow)
	ON_COMMAND(ID_FILE_NEW_MOVIE, OnFileNewMovie)
	ON_COMMAND(ID_FILE_NEW, myOnFileNew)
	ON_COMMAND(ID_EDIT_URL_ADD, OnEditUrlAdd)
	ON_COMMAND(ID_EDIT_THUMBNAILS, OnEditThumbnails)
	ON_COMMAND(ID_EDIT_RESETOPTIONS, OnEditResetOptions)
	ON_COMMAND(ID_FUNC_SHORTCUTS, OnFuncShortcuts)
	ON_COMMAND(ID_FUNCS_SHORTCUTS_FILE, OnFuncsShortcutsFile)
	ON_COMMAND(ID_CLAN_CA_CHARS, OnClanCaChars)
	ON_COMMAND(ID_WEB_DATA, OnWebData)
	//}}AFX_MSG_MAP
	// Standard file based document commands
//	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClan2App construction

CClan2App::CClan2App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	// Initialize QTML and QuickTime
	isMouseButtonDn = NULL;
	InitializeQTML (0);
	EnterMovies ();
}


/////////////////////////////////////////////////////////////////////////////
// The one and only CClan2App object

CClan2App theApp;
/////////////////////////////////////////////////////////////////////////////
// CClan2App initialization

BOOL CClan2App::InitInstance()
{
	extern char *nameOverride, *pathOverride, *lineNumFname,
		*clanBuf, clanRun, quickTranscribe;
	extern long rowLimitOverride, lineNumOverride;
	extern unCH *getWebMedia;

/*
    HWND      hwnd;

    // Win32 will always set hPrevInstance to NULL, so lets check
    // things a little closer. This is because we only want a single
    // version of this app to run at a time
    hwnd = FindWindow (szAppName, NULL);
    if (hwnd) {
        // We found another version of ourself. Lets defer to it:
        if (IsIconic(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }
        SetForegroundWindow (hwnd);

        return FALSE;
	}
*/
	AfxInitRichEdit();
    // Initialize the quartz library
    CoInitialize(NULL);

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
//	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	bApKey = VkKeyScan('`');
	TildKey = VkKeyScan('~');
	ApKey = VkKeyScan('\'');
	HatKey = VkKeyScan('^');
	AtKey = VkKeyScan('@');
	ColKey = VkKeyScan(':');
	SemiColKey = VkKeyScan(';');
	ComaKey = VkKeyScan(',');
	DotKey = VkKeyScan('.');
	StarKey = VkKeyScan('*');
	QuestKey = VkKeyScan('?');
	AndKey = VkKeyScan('&');
	SlashKey = VkKeyScan('/');
	EqSignKey = VkKeyScan('=');
	OpSQBKey = VkKeyScan('[');
	ClSQBKey = VkKeyScan(']');
	OpAGBKey = VkKeyScan('<');
	ClAGBKey = VkKeyScan('>');


	GlobalDC = NULL;
	nameOverride = NULL;
	pathOverride = NULL;
	lineNumFname = NULL;
	getWebMedia = NULL;
	clanBuf = NULL;
	clanRun = FALSE;
	quickTranscribe = FALSE;
	rowLimitOverride = 0L;
	lineNumOverride = -1L;

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Clan")); // use \windows\clan2.ini instead

	CChildFrame::Initialize();
//	SetDialogBkColor();
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_CLAN2TYPE,
		RUNTIME_CLASS(CClan2Doc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CClan2View));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	int nCmdShow;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
/*
	// DON'T display a new MDI child window during startup!!!
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
*/
	ParseCommandLine(cmdInfo);

	// setup main window
	nCmdShow = !cmdInfo.m_bRunEmbedded ? m_nCmdShow : SW_HIDE;
	m_nCmdShow = SW_HIDE;
	nCmdShow = ((CMainFrame*)m_pMainWnd)->InitialShowWindow(nCmdShow);


	RegisterShellFileTypes(TRUE);
	// do other initialization after (possibly) creating the splash window
	EnableShellOpen();
	m_pMainWnd->DragAcceptFiles();
	

	CClan2View::Initialize();
	LocalInit();
	firstTimeStart = TRUE;

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	m_nCmdShow = nCmdShow;
	m_pMainWnd->ShowWindow(m_nCmdShow);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

BOOL CClan2App::OnIdle(LONG lCount)
{
	// call base class idle first
	BOOL bResult = CWinApp::OnIdle(lCount);

/*
	// then do our work
	if (m_splash.m_hWnd != NULL)
	{
		if (::GetCurrentTime() - m_dwSplashTime > 2500)
		{
			// timeout expired, destroy the splash window
			m_splash.DestroyWindow();
			m_pMainWnd->UpdateWindow();

			// NOTE: don't set bResult to FALSE,
			//  CWinApp::OnIdle may have returned TRUE
		}
		else
		{
			// check again later...
			bResult = TRUE;
		}
	}
*/
	return bResult;
}


int CClan2App::ExitInstance()
{
	extern void freeArgs(void);

//	dlgPageSetup.Terminate();
	CClan2View::Terminate();
	CChildFrame::Terminate();
    CoUninitialize();
	ExitMovies();
	TerminateQTML();
	freeArgs();
	return CWinApp::ExitInstance();
}

void CClan2App::OnFileNewMovie()
{
	if (m_pDocManager != NULL) {
		POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
		CDocTemplate* pTemplate = NULL;
		ASSERT(pos != NULL);
		pTemplate = (CDocTemplate*)m_pDocManager->GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		ASSERT(pos != NULL);
		pTemplate = (CDocTemplate*)m_pDocManager->GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		pTemplate->OpenDocumentFile(NULL);
	}
}

void CClan2App::myOnFileNew()
{
	if (m_pDocManager != NULL) {
		POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
		CDocTemplate* pTemplate = NULL;
		ASSERT(pos != NULL);
		pTemplate = (CDocTemplate*)m_pDocManager->GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		pTemplate->OpenDocumentFile(NULL);

		// if returns NULL, the user has already been alerted
//		m_pDocManager->OnFileNew();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_Version;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_Version = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_VERSION, m_Version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CClan2App::OnAppAbout()
{
	extern char VERSION[];

	CAboutDlg aboutDlg;
	aboutDlg.m_Version = cl_T("");
	aboutDlg.m_Version += cl_T(VERSION);
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CClan2App commands

void CClan2App::OnEditOptionsCedoptions() 
{
	char ReWrapDiff;
	CCedDlgs dlg;
	BOOL tDoMixedSTWave;
	char tIsChatLineNums;
	long tClanWinRowLim;
	extern char DefClan;
	extern char ClanAutoWrap;
	extern char isCursorPosRestore;
	extern long ClanWinRowLim;

	ReWrapDiff = doReWrap;
	// Initialize dialog data
	dlg.m_nCheckPointCnt = FreqCountLimit;
	dlg.m_CreateBackup = !MakeBackupFile;
	dlg.m_StartEditorMode = !StartInEditorMode;
	dlg.m_WrapLine = DefAutoWrap;
	dlg.m_RestoreCursor = isCursorPosRestore;
	dlg.m_OpenClan = DefClan;
	dlg.m_WrapClanOutput = ClanAutoWrap;
	dlg.m_ClanWindowLinesLimit = ClanWinRowLim;
	dlg.m_Tier = cl_T(DisTier);
	dlg.m_Mixed_Stereo_Wave = doMixedSTWave;
	dlg.m_Update_Clan = isUpdateCLAN;
	tIsChatLineNums = isChatLineNums;
	tDoMixedSTWave = doMixedSTWave;
	tClanWinRowLim = ClanWinRowLim;

	// Invoke the dialog box
	if (dlg.DoModal() == IDOK) {
		// retrieve the dialog data
		FreqCountLimit = dlg.m_nCheckPointCnt;
		MakeBackupFile = !dlg.m_CreateBackup;
		StartInEditorMode = !dlg.m_StartEditorMode;
		DefAutoWrap = dlg.m_WrapLine;
		isCursorPosRestore = dlg.m_RestoreCursor;
		ReWrapDiff = ((ReWrapDiff != doReWrap) && doReWrap);
		DefClan = dlg.m_OpenClan;
		ClanAutoWrap = dlg.m_WrapClanOutput;
		ClanWinRowLim = dlg.m_ClanWindowLinesLimit;
		u_strcpy(DisTier, dlg.m_Tier, 51);
		uS.uppercasestr(DisTier, &dFnt, C_MBF);
		doMixedSTWave = dlg.m_Mixed_Stereo_Wave;
		isUpdateCLAN = dlg.m_Update_Clan;

	    WriteCedPreference();
	    if (ClanWinRowLim != tClanWinRowLim) {
			CWinApp* wApp = AfxGetApp();
			POSITION pos = wApp->GetFirstDocTemplatePosition();

			while (pos != NULL) {
				CDocTemplate* pTemplate = (CDocTemplate*)wApp->GetNextDocTemplate(pos);
				ASSERT_KINDOF(CDocTemplate, pTemplate);
				POSITION pos2 = pTemplate->GetFirstDocPosition();
				while (pos2 != NULL) {
					CClan2Doc* pDoc = (CClan2Doc *)pTemplate->GetNextDoc(pos2);
					if (pDoc->FileInfo != NULL) {
						if (pDoc->FileInfo->winID == 1964) {
							if (ClanWinRowLim != 0L && ClanWinRowLim < 10L)
								pDoc->FileInfo->RowLimit = 10L;
							else
								pDoc->FileInfo->RowLimit = ClanWinRowLim;
						}
					}
					pDoc->redo_soundwave = ClanWinRowLim;
				}
			}
		}
	    if (ReWrapDiff) {
			CWinApp* wApp = AfxGetApp();
			POSITION pos = wApp->GetFirstDocTemplatePosition();

			while (pos != NULL) {
				CDocTemplate* pTemplate = (CDocTemplate*)wApp->GetNextDocTemplate(pos);
				ASSERT_KINDOF(CDocTemplate, pTemplate);
				POSITION pos2 = pTemplate->GetFirstDocPosition();
				while (pos2 != NULL) {
					CClan2Doc* pDoc = (CClan2Doc *)pTemplate->GetNextDoc(pos2);
					pDoc->re_wrap = TRUE;
					pDoc->UpdateAllViews(NULL, 0L, NULL);
				}
			}
		}
	    if (doMixedSTWave != tDoMixedSTWave) {
			CWinApp* wApp = AfxGetApp();
			POSITION pos = wApp->GetFirstDocTemplatePosition();

			while (pos != NULL) {
				CDocTemplate* pTemplate = (CDocTemplate*)wApp->GetNextDocTemplate(pos);
				ASSERT_KINDOF(CDocTemplate, pTemplate);
				POSITION pos2 = pTemplate->GetFirstDocPosition();
				while (pos2 != NULL) {
					CClan2Doc* pDoc = (CClan2Doc *)pTemplate->GetNextDoc(pos2);
					pDoc->redo_soundwave = TRUE;
					pDoc->UpdateAllViews(NULL, 0L, NULL);
				}
			}
		}
		if (tIsChatLineNums != isChatLineNums) {
			CWinApp* wApp = AfxGetApp();
			POSITION pos = wApp->GetFirstDocTemplatePosition();

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
		}
//	    AfxGetMainWnd()->UpdateWindow();
	}
}

void CClan2App::OnClanClanwindow() 
{
	if (clanDlg == NULL) {
		clanDlg = new CClanWindow(AfxGetApp()->m_pMainWnd);
	}
}

void CClan2App::OnClanCaChars() 
{
	if (spCharsDlg == NULL) {
		spCharsDlg = new CSpCharsDialog(AfxGetApp()->m_pMainWnd);
	}	
}


void CClan2App::OnWebData() 
{
	if (webDataDlg == NULL) {
		webDataDlg = new CWebData(AfxGetApp()->m_pMainWnd);
	}	
}

/*
void CClan2App::OnInsertDialog() 
{
	if (insertDlg == NULL) {
		insertDlg = new CInsertDlg(AfxGetApp()->m_pMainWnd);
	}
}
*/

void CClan2App::OnEditUrlAdd() 
{
	int i;
	char *s;
	CURLAddress	dlg;
	extern char proxyAddPort[];

	dlg.m_Address = cl_T(URL_Address);
	strcpy(templineC1, proxyAddPort);
	s = strrchr(templineC1, ':');
	if (s != NULL) {
		*s = EOS;
		dlg.m_ProxyAddress = cl_T(templineC1);
		dlg.m_ProxyPort = cl_T(s+1);
	} else
		dlg.m_ProxyAddress = cl_T(templineC1);
	if (dlg.DoModal() == IDOK) {
		strcpy(templine1, dlg.m_Address);
		if (templine1[0] != EOS) {
			uS.lowercasestr(templine1, &dFnt, FALSE);
			if (strncmp(templine1, "http://", 7)) {
				strcpy(URL_Address, "http://");
				u_strcpy(URL_Address+7, dlg.m_Address, 256-7);
			} else
				u_strcpy(URL_Address, dlg.m_Address, 256);
			i = strlen(URL_Address);
			if (URL_Address[i-1] != '/')
				strcat(URL_Address, "/");
			for (i=0; isSpace(URL_Address[i]); i++) ;
			if (i > 0)
				strcpy(URL_Address, URL_Address+i);
			uS.remblanks(URL_Address);
		} else
			URL_Address[0] = EOS;
		strcpy(templine1, dlg.m_ProxyAddress);
		for (i=0; isSpace(templine1[i]); i++) ;
		if (i > 0)
			strcpy(templine1, templine1+i);
		uS.remblanks(templine1);
		if (templine1[0] != EOS) {
			u_strcpy(proxyAddPort, templine1, 256);
		} else
			proxyAddPort[0] = EOS;
		strcpy(templine1, dlg.m_ProxyPort);
		for (i=0; isSpace(templine1[i]); i++) ;
		if (i > 0)
			strcpy(templine1, templine1+i);
		uS.remblanks(templine1);
		if (templine1[0] != EOS && proxyAddPort[0] != EOS) {
			strcat(proxyAddPort, ":");
			i = strlen(proxyAddPort);
			u_strcpy(proxyAddPort+i, templine1, 256-i);
		}
	    WriteCedPreference();
	}
}

void CClan2App::OnEditThumbnails() 
{
	CThumbnails	dlg;

	sprintf(templineC1,"%ld", (long)ThumbnailsHight);
	dlg.m_Size = cl_T(templineC1);
	if (dlg.DoModal() == IDOK) {
		strcpy(templine1, dlg.m_Size);
		if (templine1[0] != EOS) {
			u_strcpy(templineC1, templine1, UTTLINELEN);
			ThumbnailsHight = atof(templineC1);
			if (ThumbnailsHight < 30.0000)
				ThumbnailsHight = 30.0000;
		} else
			ThumbnailsHight = 30.0000;
	    WriteCedPreference();
	}
}

void CClan2App::OnEditResetOptions() 
{
	resetOptions();
}

// CHECK FOR CLAN UPDATE
static void checkForUpdate(void) {
	int	 vn;
	char *s, res;
	wchar_t URLPath[128];
	char fname[FNSize];
	char buf[BUFSIZ];
	FILE *fp;
	time_t		timer;
	CUpdateCLAN dlgUpdate;
	extern int WebCLANVersion;
	extern time_t versionTime;
	extern FNType prefsDir[];
	extern bool curlURLDownloadToFile(unCH *fulURLPath, unCH *fname, size_t isProgres);

	time(&timer);
	if (versionTime > timer)
		return;
	if (!isUpdateCLAN)
		return;
// strcpy(templineC, ctime(&timer));
	versionTime = timer + 345600L; // this number = 4 days time
// strcpy(templineC, ctime((time_t *)&versionTime));
	u_strcpy(URLPath, "http://childes.talkbank.org/clan/CLAN_VERSION.txt", 128);
	strcpy(fname, prefsDir);
	strcat(fname, "CLAN_VERSION.txt");
	u_strcpy(templine, fname, UTTLINELEN);
	if (curlURLDownloadToFile(URLPath, templine, 0L) == true) {
		fp = fopen(fname, "r");
		if (fp == NULL) {
			WriteCedPreference();
			return;
		}
		templineC[0] = EOS;
		while (fgets(buf, BUFSIZ, fp)) {
			strcat(templineC, buf);
		}
		fclose(fp);
		unlink(fname);
		vn = atoi(templineC);
		if (vn == 0) {
			WriteCedPreference();
			return;
		}
		for (s=templineC; *s != EOS && *s != '\n' && *s != '\r'; s++) ;
		while (*s == '\n' || *s == '\r')
			s++;
		uS.remFrontAndBackBlanks(s);
		if (WebCLANVersion < vn) {
			u_strcpy(templine, s, UTTLINELEN);
			dlgUpdate.m_Message = templine;
			dlgUpdate.res = 0;
			if (dlgUpdate.DoModal() == IDOK) {
				res = dlgUpdate.res;
				if (res == 1) {
					u_strcpy(URLPath, "http://childes.talkbank.org/clan/clanwin.exe", 128);
					strcpy(fname, prefsDir);
					strcat(fname, "clanwin.exe");
					u_strcpy(templine, fname, UTTLINELEN);
					if (curlURLDownloadToFile(URLPath, templine, 2400000L /* 3072000L*/) == true) {
//						_wsystem(templine);
						_wexecl(templine, templine, NULL);
						exit(0);
					}
				} else if (res == 2) {
					WebCLANVersion = vn;
				} else if (res == 3) {
					isUpdateCLAN = FALSE;
				}
			}
		}
	}
	WriteCedPreference();
}
// CHECK FOR CLAN UPDATE

/*
BOOL CClan2App::myPumpMessage()
{
int k;
	ASSERT_VALID(this);

	if (!::GetMessage(&m_msgCur, NULL, NULL, NULL))
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceAppMsg)
			TRACE0("CWinThread::PumpMessage - Received WM_QUIT.\n");
		m_nDisablePumpCount++; // application must die
			// Note: prevents calling message loop things in 'ExitInstance'
			// will never be decremented
#endif
		return FALSE;
	}

if (m_msgCur.message != 0x200 &&
	m_msgCur.message != 0x00f &&
	m_msgCur.message != 0x36a &&
	(m_msgCur.message != 0x104 || cnt != 0) &&
	m_msgCur.message != 0x118)
	k = 1;

if (m_msgCur.message == 0x104)
	cnt++;
if (m_msgCur.message == 0x105)
	cnt = 0;

#ifdef _DEBUG
	if (m_nDisablePumpCount != 0)
	{
		TRACE0("Error: CWinThread::PumpMessage called when not permitted.\n");
		ASSERT(FALSE);
	}
#endif

	// process this message

	if (m_msgCur.message != 0x036A && !PreTranslateMessage(&m_msgCur))
	{
		::TranslateMessage(&m_msgCur);
		::DispatchMessage(&m_msgCur);
	}
	return TRUE;
}
*/
int CClan2App::Run() 
{   // Overridden to check for Graph events as well as messages
	if (m_pMainWnd == NULL && AfxOleGetUserCtrl())
	{
		// Not launched /Embedding or /Automation, but has no main window!
		TRACE0("Warning: m_pMainWnd is NULL in CWinApp::Run - quitting application.\n");
		AfxPostQuitMessage(0);
	}
	ASSERT_VALID(this);

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;
	DWORD tDelay;

	tDelay = GetTickCount();
	_AFX_THREAD_STATE *pstate = AfxGetThreadState();
	// acquire and dispatch messages until a WM_QUIT message is received.
	checkForUpdate();
	for (;;)
	{
		if (isMouseButtonDn != NULL) {
			if (GetTickCount() > tDelay) {
				tDelay = GetTickCount() + (1 * 9);
				if (isMouseButtonDn != NULL) {
					POINT mPos;
					if (GetCursorPos(&mPos)) {
						WPARAM wParam;
						LPARAM *lParam;
						isMouseButtonDn->ScreenToClient(&mPos);
						wParam = MK_LBUTTON;
						lParam = (LPARAM *)&mPos;
						isMouseButtonDn->SendMessage(WM_MOUSEMOVE, wParam, *lParam);
					}
				}
			}
		}

		// phase1: check to see if we can do idle work
		while (bIdle &&
			!::PeekMessage(&pstate->m_msgCur, NULL, NULL, NULL, PM_NOREMOVE))
		{
			// call OnIdle while in bIdle state
			if (!OnIdle(lIdleCount++))
				bIdle = FALSE; // assume "no idle" state
		}

		// phase2: pump messages while available
		do
		{
			// pump message, but quit on WM_QUIT
			if (!PumpMessage())
				return ExitInstance();

			// reset "no idle" state after pumping "normal" message
			if (IsIdleMessage(&pstate->m_msgCur))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}

		} while (::PeekMessage(&pstate->m_msgCur, NULL, NULL, NULL, PM_NOREMOVE));
	}

	ASSERT(FALSE);  // not reachable

 //	return CWinApp::Run();
}

void CClan2App::OnFuncShortcuts() 
{
	Apropos(-1);
}

void CClan2App::OnFuncsShortcutsFile() 
{
	AproposFile(-1);
}

BOOL CClan2App::PreTranslateMessage(MSG* pMsg)
{
	BOOL bResult;

	bResult = CWinApp::PreTranslateMessage(pMsg);
/*
	if (m_splash.m_hWnd != NULL &&
		(pMsg->message == WM_KEYDOWN ||
		 pMsg->message == WM_SYSKEYDOWN ||
		 pMsg->message == WM_LBUTTONDOWN ||
		 pMsg->message == WM_RBUTTONDOWN ||
		 pMsg->message == WM_MBUTTONDOWN ||
		 pMsg->message == WM_NCLBUTTONDOWN ||
		 pMsg->message == WM_NCRBUTTONDOWN ||
		 pMsg->message == WM_NCMBUTTONDOWN))
	{
		m_splash.DestroyWindow();
		m_pMainWnd->UpdateWindow();
	}
*/
/*
	static int b = 0;
	CWinThread* pWinThread = AfxGetThread();
	CWinApp* pWinApp = AfxGetApp();
	CWnd* win = AfxGetMainWnd();

	if (pWinThread->m_msgCur.message == WM_KICKIDLE) {
		CDC* pDC = win->GetDC();
		if (b == 0) b = 255;
		else b = 0;
		CPen penStroke;
		if (penStroke.CreatePen(PS_SOLID, 2, RGB(b,0,0))) {
			CPen* pOldPen = pDC->SelectObject( &penStroke );
			pDC->MoveTo(100, 100);
			pDC->LineTo(100, 150);
			pDC->SelectObject( pOldPen );
		}
	}
*/
	return bResult;
}
