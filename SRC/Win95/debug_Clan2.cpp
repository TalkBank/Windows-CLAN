// Clan2.cpp : Defines the class behaviors for the application.
//

#include "ced.h"
#include <direct.h>
#include "MainFrm.h"
#include "ChildFrm.h"
#include "Clan2Doc.h"
#include "Clan2View.h"
#include "CedDlgs.h"
#include "SpCharsDialog.h"
#include "W95_WebData.h"
//#include "insertDlg.h"
#include "mmedia.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWnd *isMouseButtonDn;
char firstTimeStart;
char tFileBuf[FILENAME_MAX];
char cwd[_MAX_PATH];

static int ScreenX = 0, ScreenY = 0;

//extern CInsertDlg *insertDlg;

#define DEBUG_CLAN
#ifdef DEBUG_CLAN
FILE *dcFP = NULL;
#endif

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


/////////////////////////////////////////////////////////////////////////////
// CClan2App

BEGIN_MESSAGE_MAP(CClan2App, CWinApp)
	//{{AFX_MSG_MAP(CClan2App)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_EDIT_OPTIONS_CEDOPTIONS, OnEditOptionsCedoptions)
	ON_COMMAND(ID_CLAN_CLANWINDOW, OnClanClanwindow)
	ON_COMMAND(ID_EDIT_OPTIONS_SETDEPFILE, OnEditOptionsSetdepfile)
	ON_COMMAND(ID_EDIT_MOR_OPTIONS, OnEditMorOptions)
	ON_COMMAND(ID_FILE_NEW_MOVIE, OnFileNewMovie)
	ON_COMMAND(ID_FILE_NEW, myOnFileNew)
	ON_COMMAND(ID_EDIT_URL_ADD, OnEditUrlAdd)
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
#ifdef DEBUG_CLAN
	dcFP = fopen("c:\\debug_clan.txt", "w");
#endif
	isMouseButtonDn = NULL;
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "1 in CClan2App::CClan2App()\n");
		fflush(dcFP);
	}
#endif
	InitializeQTML (0);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "2 in CClan2App::CClan2App()\n");
		fflush(dcFP);
	}
#endif
	EnterMovies ();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "3 in CClan2App::CClan2App()\n");
		fflush(dcFP);
	}
#endif
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
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "1 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	AfxInitRichEdit();
    // Initialize the quartz library
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "2 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
    CoInitialize(NULL);

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "3 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	AfxEnableControlContainer();

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "4 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "5 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
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

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "6 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	CChildFrame::Initialize();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "7 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	SetDialogBkColor();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "8 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "9 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_CLAN2TYPE,
		RUNTIME_CLASS(CClan2Doc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CClan2View));
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "10 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "11 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	CMainFrame* pMainFrame = new CMainFrame;
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "12 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
#ifdef _UNICODE
	if (!pMainFrame->LoadFrame(IDR_UMAINFRAME))
		return FALSE;
#else
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
#endif
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "13 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
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
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "14 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	nCmdShow = !cmdInfo.m_bRunEmbedded ? m_nCmdShow : SW_HIDE;
	m_nCmdShow = SW_HIDE;
	nCmdShow = ((CMainFrame*)m_pMainWnd)->InitialShowWindow(nCmdShow);


#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "15 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	RegisterShellFileTypes(TRUE);
	// do other initialization after (possibly) creating the splash window
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "16 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	EnableShellOpen();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "17 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	m_pMainWnd->DragAcceptFiles();
	
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "18 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	CClan2View::Initialize();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "19 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	LocalInit();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "20 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	firstTimeStart = TRUE;

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "21 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	if (_getcwd(cwd, _MAX_PATH) == NULL)
		cwd[0] = EOS;
	else
		strcat(cwd, "\\");

	// The main window has been initialized, so show and update it.
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "22 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	m_nCmdShow = nCmdShow;
	m_pMainWnd->ShowWindow(m_nCmdShow);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "23 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
	m_pMainWnd->UpdateWindow();

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "24 in BOOL CClan2App::InitInstance()\n");
		fflush(dcFP);
	}
#endif
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

BOOL CClan2App::PreTranslateMessage(MSG* pMsg)
{
	BOOL bResult = CWinApp::PreTranslateMessage(pMsg);
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

//	if (pWinThread->m_msgCur.message == WM_KICKIDLE) {
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
//	}
*/
	return bResult;
}

int CClan2App::ExitInstance()
{
	extern void freeArgs(void);

//	dlgPageSetup.Terminate();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "1 in int CClan2App::ExitInstance()\n");
		fflush(dcFP);
	}
#endif
	CClan2View::Terminate();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "2 in int CClan2App::ExitInstance()\n");
		fflush(dcFP);
	}
#endif
	CChildFrame::Terminate();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "3 in int CClan2App::ExitInstance()\n");
		fflush(dcFP);
	}
#endif
    CoUninitialize();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "4 in int CClan2App::ExitInstance()\n");
		fflush(dcFP);
	}
#endif
	ExitMovies();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "5 in int CClan2App::ExitInstance()\n");
		fflush(dcFP);
	}
#endif
	TerminateQTML();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "6 in int CClan2App::ExitInstance()\n");
		fflush(dcFP);
	}
#endif
	freeArgs();
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "7 in int CClan2App::ExitInstance()\n");
		fflush(dcFP);
	}
#endif
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
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "1 in void CClan2App::myOnFileNew()\n");
		fflush(dcFP);
	}
#endif
		POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
		CDocTemplate* pTemplate = NULL;
		ASSERT(pos != NULL);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "2 in void CClan2App::myOnFileNew()\n");
		fflush(dcFP);
	}
#endif
		pTemplate = (CDocTemplate*)m_pDocManager->GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "3 in void CClan2App::myOnFileNew()\n");
		fflush(dcFP);
	}
#endif
		pTemplate->OpenDocumentFile(NULL);

#ifdef DEBUG_CLAN
	if (dcFP != NULL) {
		fprintf(dcFP, "4 in void CClan2App::myOnFileNew()\n");
		fflush(dcFP);
	}
#endif
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
	extern char *VERSION;

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
	BOOL oldDoMixedSTWave;
	char tIsChatLineNums;
	extern char DefClan;
	extern char defIsCainMode;
	extern char ClanAutoWrap;
	extern long ClanWinRowLim;

	ReWrapDiff = doReWrap;
	// Initialize dialog data
	dlg.m_nCheckPointCnt = FreqCountLimit;
	if (strcmp(CODEFNAME, CODES_FILE)) {
		strcpy(CodesFName, CODEFNAME);
		dlg.m_nCodesFname = cl_T(CodesFName);
	} else {
		*CodesFName = EOS;
		dlg.m_nCodesFname = cl_T("No file selected");
	}
	if (strcmp(STATEFNAME, KEYS_BIND_FILE)) {
		strcpy(KeysFName, STATEFNAME);
		dlg.m_KeyBindFname = cl_T(KeysFName);
	} else {
		*KeysFName = EOS;
		dlg.m_KeyBindFname = cl_T("No file selected");
	}
	dlg.m_CreateBackup = !MakeBackupFile;
	dlg.m_StartEditorMode = !StartInEditorMode;
	dlg.m_CursorPerc = ShowPercentOfFile;
	dlg.m_WrapLine = DefAutoWrap;
	dlg.m_WrapChat = doReWrap;
	dlg.m_OpenClan = DefClan;
	dlg.m_isCainMode = defIsCainMode;
	dlg.m_WrapClanOutput = ClanAutoWrap;
	dlg.m_ClanWindowLinesLimit = ClanWinRowLim;
	dlg.m_Tier = cl_T(DisTier);
	dlg.m_Delims = (IsGSet(1) ? TRUE : FALSE);
	dlg.m_Chars = (IsGSet(3) ? TRUE : FALSE);
	dlg.m_IDTier = (IsGSet(2) ? TRUE : FALSE);
	dlg.m_Mixed_Stereo_Wave = doMixedSTWave;
	dlg.m_ChatLinenum = isChatLineNums;
	dlg.m_InputRawData = rawTextInput;
	dlg.m_SPCKeyShortcuts = isUseSPCKeyShortcuts;
	tIsChatLineNums = isChatLineNums;
	oldDoMixedSTWave = doMixedSTWave;

    if (DefChatMode == 2) dlg.m_ChatModeAuto = TRUE;	/* no +/- w */
    else if (DefChatMode) dlg.m_ChatModeAlways = TRUE;	/* +w */
    else dlg.m_ChatModeNever = TRUE;					/* -w */

	// Invoke the dialog box
	if (dlg.DoModal() == IDOK) {
		// retrieve the dialog data
		FreqCountLimit = dlg.m_nCheckPointCnt;
		MakeBackupFile = !dlg.m_CreateBackup;
		StartInEditorMode = !dlg.m_StartEditorMode;
		ShowPercentOfFile = dlg.m_CursorPerc;
		DefAutoWrap = dlg.m_WrapLine;
		doReWrap = dlg.m_WrapChat;
		ReWrapDiff = ((ReWrapDiff != doReWrap) && doReWrap);
		DefClan = dlg.m_OpenClan;
		defIsCainMode = dlg.m_isCainMode;
		ClanAutoWrap = dlg.m_WrapClanOutput;
		ClanWinRowLim = dlg.m_ClanWindowLinesLimit;
		convA_strcpy(CodesFName, dlg.m_nCodesFname, FILENAME_MAX);
		convA_strcpy(KeysFName, dlg.m_KeyBindFname, FILENAME_MAX);
		convU_strcpy(DisTier, dlg.m_Tier, 51);
		uS.uppercasestr(DisTier, &dFnt, C_MBF);
		SetGOption(1, dlg.m_Delims);
		SetGOption(3, dlg.m_Chars);
		SetGOption(2, dlg.m_IDTier);
		doMixedSTWave = dlg.m_Mixed_Stereo_Wave;
		isChatLineNums = dlg.m_ChatLinenum;
		rawTextInput = dlg.m_InputRawData;
		isUseSPCKeyShortcuts = dlg.m_SPCKeyShortcuts;

	    if (dlg.m_ChatModeAuto) DefChatMode = 2;
    	else if (dlg.m_ChatModeAlways) DefChatMode = 1;
    	else if (dlg.m_ChatModeNever) DefChatMode = 0;

	    WriteCedPreference();
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
	    if (doMixedSTWave != oldDoMixedSTWave) {
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
void CClan2App::OnEditMorOptions() 
{
	CMorOptions dlg;

	dlg.m_Grammar = cl_T(mor_grammar_name);
	dlg.m_Lexicon = cl_T(mor_lexicon_name);
	if (dlg.DoModal() == IDOK) {
		convA_strcpy(mor_grammar_name, dlg.m_Grammar, FILENAME_MAX);
		convA_strcpy(mor_lexicon_name, dlg.m_Lexicon, FILENAME_MAX);
	    WriteCedPreference();
	}
}

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
				convA_strcpy(URL_Address+7, dlg.m_Address, 256-7);
			} else
				convA_strcpy(URL_Address, dlg.m_Address, 256);
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
			convA_strcpy(proxyAddPort, templine1, 256);
		} else
			proxyAddPort[0] = EOS;
		strcpy(templine1, dlg.m_ProxyPort);
		for (i=0; isSpace(templine1[i]); i++) ;
		if (i > 0)
			strcpy(templine1, templine1+i);
		uS.remblanks(templine1);
		if (templine1[0] != EOS && proxyAddPort[0] != EOS) {
			strcat(proxyAddPort, ":");
			convA_strcat(proxyAddPort, templine1, 256-strlen(proxyAddPort));
		}
	    WriteCedPreference();
	}
}

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
	// acquire and dispatch messages until a WM_QUIT message is received.
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
			!::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE))
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
			if (IsIdleMessage(&m_msgCur))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}

		} while (::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE));
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
