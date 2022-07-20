// ClanWindow.cpp : implementation file
//

#include "ced.h"
#include "cu.h"
#include "w95_commands.h"
#include "w95_cl_eval.h"
#include "w95_cl_kideval.h"
#include "w95_cl_tiers.h"
#include "w95_cl_search.h"

#ifdef __MWERKS__
#include <unistd.h>
#else
#include "msutil.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUM_COMMANDS 50
#define MAX_COM_LEN  512

static char  *clan_commands[NUM_COMMANDS];
static short curCommand;

short lastCommand;
long  ClanWinRowLim = 1000L;
long  lineNumOverride;
char  *lineNumFname;
char  isSpOverride;
char  *clanBuf, clanRun;
char  ClanAutoWrap = TRUE;
FNType wd_dir[FNSize], od_dir[FNSize];
CClanWindow *clanDlg = NULL;
FONTINFO cmdFnt; // 2016-03-29

extern int  F_numfiles;
extern int  cl_argc;
extern char *cl_argv[];
extern char *nameOverride, *pathOverride;
extern float scalingSize;
extern long option_flags[];
extern struct DefWin ClanWinSize;

extern "C"
{
	extern void CleanUpAll(char all);
}

bool isAtOnCommandLineFound(unCH *s) {
	int  i;
	unCH qf;

	if (F_numfiles <= 0)
		return(false);

	qf = FALSE;
	for (i = 0; s[i] != EOS; i++) {
		if (s[i] == '@') {
			if (!qf && i > 0) {
				if (isSpace(s[i - 1]) && (isSpace(s[i + 1]) || s[i + 1] == EOS))
					return(true);
			}
		} else if (s[i] == '\'' || s[i] == '"') {
			if (!qf) {
				qf = s[i];
			} else if (qf == s[i]) {
				qf = FALSE;
			}
		}
	}
	return(false);
}

char isFindFile(int ProgNum) {
	int		i, j, len;
	FILE	*fp, *fpT;

	SetNewVol(wd_dir);
	for (i=1; i < cl_argc; i++) {
		if (cl_argv[i][0] == '-' || cl_argv[i][0] == '+') {
			if ((cl_argv[i][1] == 's' || cl_argv[i][1] == 'S') && ProgNum == CHSTRING)
				i++;
		} else {
			if (!strcmp(cl_argv[i], "@")) {
				for (j=1; j <= F_numfiles; j++) {
					get_selected_file(j, FileName1, FNSize);
					fpT = fopen(FileName1, "r");
					if (fp != NULL) {
						fclose(fpT);
						return(TRUE);
					}
				}
			} else if (cl_argv[i][0] == '@' && cl_argv[i][1] == ':') {
				uS.str2FNType(FileName1, 0L, cl_argv[i]+2);
				fp = fopen(FileName1, "r");
				if (fp == NULL) {
					return(FALSE);
				}
				while (fgets_cr(FileName1, 3072, fp)) {
					uS.remFrontAndBackBlanks(FileName1);
					fpT = fopen(FileName1, "r");
					if (fp != NULL) {
						fclose(fpT);
						return(TRUE);
					}
				}
				fclose(fp);
			} else if (strchr(cl_argv[i], '*') == NULL) {
				fpT = fopen(cl_argv[i], "r");
				if (fp != NULL) {
					fclose(fpT);
					return(TRUE);
				}
			} else {
				strcpy(DirPathName, wd_dir);
				len = strlen(DirPathName);
				j = 1;
				while ((j=Get_File(FileName1, j)) != 0) {
					if (uS.fIpatmat(FileName1, cl_argv[i])) {
						addFilename2Path(DirPathName, FileName1);
						fpT = fopen(DirPathName, "r");
						if (fp != NULL) {
							fclose(fpT);
							return(TRUE);
						}
						DirPathName[len] = EOS;
					}
				}
			}
		}
	}
	return(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CClanWindow dialog
#define __AFXCONV_H__
#include "afxpriv.h"

CClanWindow::CClanWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CClanWindow::IDD, pParent)
{
	unCH ts[256];
	short size;
	float tf;

	//{{AFX_DATA_INIT(CClanWindow)
	u_strcpy(t_st, lib_dir, FNSize);
	AdjustName(lib_st, t_st, 39);
	m_LibSt = lib_st;
	u_strcpy(t_st, mor_lib_dir, FNSize);
	AdjustName(mor_lib_st, t_st, 39);
	m_morLibSt = mor_lib_st;
	if (pathcmp(od_dir, wd_dir) == 0)
		m_OdSt = _T("");
	else {
		u_strcpy(t_st, od_dir, FNSize);
		AdjustName(od_st, t_st, 39);
		m_OdSt = od_st;
	}
	u_strcpy(t_st, wd_dir, FNSize);
	AdjustName(wd_st, t_st, 39);
	m_WdSt = wd_st;
	m_Commands = _T("");
	//}}AFX_DATA_INIT
//	Create(IDD, pParent);
	LPCTSTR lpszTemplateName = MAKEINTRESOURCE(IDD);
	ASSERT(HIWORD(lpszTemplateName) == 0 ||
		AfxIsValidString(lpszTemplateName));
	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (HIWORD(m_lpszTemplateName) == 0 && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD)m_lpszTemplateName);
	HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	ASSERT(hTemplate != NULL);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
	CDialogTemplate dlgTemp(lpDialogTemplate);
// 2016-03-29
	if (!strcmp(cmdFnt.FName, "Courier") ||
		(!strcmp(cmdFnt.FName, "Courier New") && cmdFnt.CharSet <= 1)) {
		tf = (float)cmdFnt.FSize;
		tf = tf / scalingSize;
		size = (short)tf;
		dlgTemp.SetFont(_T("MS Sans Serif"), (WORD)size); // 8
	} else if (!strcmp(cmdFnt.FName, "CAfont") && cmdFnt.FSize > -16) {
		tf = -16;
		tf = tf / scalingSize;
		size = (short)tf;
		dlgTemp.SetFont(u_strcpy(ts, cmdFnt.FName, 256), (WORD)size);
	} else {
		tf = (float)cmdFnt.FSize;
		tf = tf / scalingSize;
		size = (short)tf;
		dlgTemp.SetFont(u_strcpy(ts, cmdFnt.FName, 256), (WORD)size); // lxslxslxs
	}
	HGLOBAL hTemplate2 = dlgTemp.Detach();
	if (hTemplate2 != NULL)
		lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate2);
	CreateIndirect(lpDialogTemplate,pParent,NULL,NULL/*hInst*/);
	if (hTemplate2 != NULL) {
		GlobalUnlock(hTemplate2);
		GlobalFree(hTemplate2);
	}
	UnlockResource(hTemplate);
	FreeResource(hTemplate);
}

void CClanWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanWindow)
	DDX_Control(pDX, IDC_CLAN_PROGM, m_ProgsCtrl);
	DDX_CBString(pDX, IDC_CLAN_PROGM, m_ProgS);
	DDX_Control(pDX, IDC_CLAN_OPT, optionsButtonCTRL);
	DDX_Control(pDX, IDC_CLAN_SEARCH, searchButtonCTRL);
	DDX_Control(pDX, IDC_CLAN_FILEIN, fileinButtonCTRL);
	DDX_Control(pDX, IDC_CLAN_FILEOUT, fileoutButtonCTRL);
	DDX_Control(pDX, IDC_CLAN_COMMANDS, m_CommandsControl);
	DDX_Text(pDX, IDC_CLAN_LIB_ST, m_LibSt);
	DDX_Text(pDX, IDC_CLAN_OD_ST, m_OdSt);
	DDX_Text(pDX, IDC_CLAN_WD_ST, m_WdSt);
	DDX_Text(pDX, IDC_CLAN_COMMANDS, m_Commands);
	DDX_Text(pDX, IDC_CLAN_MOR_LIB_ST, m_morLibSt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CClanWindow, CDialog)
	//{{AFX_MSG_MAP(CClanWindow)
	ON_EN_UPDATE(IDC_CLAN_COMMANDS, OnUpdateClanCommands)
	ON_CBN_SELCHANGE(IDC_CLAN_PROGM, OnClanProgs)
	ON_CBN_SELENDCANCEL(IDC_CLAN_PROGM, OnClanProgsCancel)
	ON_BN_CLICKED(IDC_CLAN_HELP, OnClanHelp)
	ON_BN_CLICKED(IDC_CLAN_LIB, OnClanLib)
	ON_BN_CLICKED(IDC_CLAN_OD, OnClanOd)
	ON_BN_CLICKED(IDC_CLAN_RUN, OnClanRun)
	ON_BN_CLICKED(IDC_CLAN_WD, OnClanWd)
	ON_BN_CLICKED(IDC_CLAN_FILEIN, OnClanFilein)
	ON_BN_CLICKED(IDC_CLAN_FILEOUT, OnClanFileout)
	ON_BN_CLICKED(IDC_CLAN_OPT, OnClanOpt)
	ON_BN_CLICKED(IDC_CLAN_SEARCH, OnClanSearch)
	ON_EN_CHANGE(IDC_CLAN_COMMANDS, OnChangeClanCommands)
	ON_BN_CLICKED(IDC_CLAN_MOR_LIB, OnClanMorLib)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CLAN_RECALL, &CClanWindow::OnClanRecall)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClanWindow message handlers
void CClanWindow::HideClanWinIcons(void) {
	optionsButtonCTRL.ShowWindow(SW_HIDE);
	searchButtonCTRL.ShowWindow(SW_HIDE);
	fileinButtonCTRL.ShowWindow(SW_HIDE);
	fileoutButtonCTRL.ShowWindow(SW_HIDE);
}

void CClanWindow::SetClanWinIcons(void) {
	int i, ProgNum;
	char *s, *com, progName[512+1];

	u_strcpy(fbuffer, m_Commands, EXPANSION_SIZE);
	for (i=0L; isSpace(fbuffer[i]); i++) ;
	if (i > 0L)
		strcpy(fbuffer, fbuffer+i);
	if (*fbuffer == EOS)
		HideClanWinIcons();
	else {
		com = fbuffer;
		s = strchr(com, ' ');
		if (s != NULL)
			*s = EOS;
		if (getAliasProgName(com, progName, 512)) {
			if (s != NULL)
				*s = ' ';
			s = NULL;
			com = progName;
		}
		if ((ProgNum=get_clan_prog_num(com, FALSE)) < 0) {
			if (strcmp(com, "bat") == 0 || strcmp(com, "batch") == 0) {
				SetDlgItemText(IDC_CLAN_FILEIN, cl_T("File In"));
				fileinButtonCTRL.ShowWindow(SW_SHOW);
				optionsButtonCTRL.ShowWindow(SW_HIDE);
				searchButtonCTRL.ShowWindow(SW_HIDE);
				fileoutButtonCTRL.ShowWindow(SW_HIDE);
			} else
				HideClanWinIcons();
		} else {
			InitOptions();
			if (ProgNum == EVAL) {
				SetDlgItemText(IDC_CLAN_FILEIN, cl_T("Option"));
			} else if (ProgNum == KIDEVAL) {
				SetDlgItemText(IDC_CLAN_FILEIN, cl_T("Option"));
			} else {
				SetDlgItemText(IDC_CLAN_FILEIN, cl_T("File In"));
			}
			fileinButtonCTRL.ShowWindow(SW_SHOW);
			if (option_flags[ProgNum] & T_OPTION && ProgNum != EVAL && ProgNum != KIDEVAL) {
				optionsButtonCTRL.ShowWindow(SW_SHOW);
			} else
				optionsButtonCTRL.ShowWindow(SW_HIDE);
			if (option_flags[ProgNum] & SP_OPTION || option_flags[ProgNum] & SM_OPTION)
				searchButtonCTRL.ShowWindow(SW_SHOW);
			else
				searchButtonCTRL.ShowWindow(SW_HIDE);
			fileoutButtonCTRL.ShowWindow(SW_HIDE);
		}
		if (s != NULL)
			*s = ' ';
	}
}

static int poup_insertSorted(const char *temp[], int k, const char *newName) {
	int i, j;

	for (i = 0; i < k; i++) {
		if (strcmp(temp[i], newName) == 0)
			return(k);
		else if (strcmp(temp[i], newName) > 0)
			break;
	}
	for (j = k; j > i; j--)
		temp[j] = temp[j - 1];
	temp[i] = newName;
	return(k+1);
}

void CClanWindow::createPopupProgMenu(void) {
	int err, pi, cnt;
	const char *temp[512];
	ALIASES_LIST *al;
	extern ALIASES_LIST *aliases;

	cnt = 0;
	for (al=aliases; al != NULL; al = al->next_alias) {
		if (al->isPullDownC == 1) {
			cnt = poup_insertSorted(temp, cnt, al->alias);
		}
	}
	for (pi=0; pi < MEGRASP; pi++) {
		if (clan_name[pi][0] != EOS)
			cnt = poup_insertSorted(temp, cnt, clan_name[pi]);
	}

	err = m_ProgsCtrl.InsertString(0, cl_T("Progs"));
	for (pi=0; pi < cnt; pi++) {
		err = m_ProgsCtrl.InsertString(-1, cl_T(temp[pi]));
	}
	m_ProgsCtrl.SetCurSel(0);
}

BOOL CClanWindow::OnInitDialog() {
	CRect lpRect;
	CDialog::OnInitDialog();

	HideClanWinIcons();

	func_init();
	readAliases(1);
	createPopupProgMenu();
	m_Commands = _T("");
	UpdateData(FALSE);
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
	m_CommandsControl.SetSel(0, 0, FALSE);
	this->GetWindowRect(&lpRect);
	if (ClanWinSize.top || ClanWinSize.width || ClanWinSize.height || ClanWinSize.left) {
		long width, hight;

		width = lpRect.right - lpRect.left;
		hight = lpRect.bottom - lpRect.top;
		lpRect.top = ClanWinSize.top;
		lpRect.bottom = lpRect.top + hight;
		lpRect.left = ClanWinSize.left;
		lpRect.right = lpRect.left + width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}
	if (lpRect.right == lpRect.left) {
		AfxGetApp()->m_pMainWnd->MessageBox(cl_T("Please use \"View->Set Commands Font\" menu to choose different font"), NULL, MB_ICONWARNING);
	}
	return 0;
}

BOOL CClanWindow::PreTranslateMessage(MSG* pMsg) 
{
	if (_AfxCompareClassName(pMsg->hwnd, _T("Edit"))) {
		if (pMsg->message == WM_KEYDOWN &&
			(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CANCEL)) {
			m_Commands = _T("");
			HideClanWinIcons();
			UpdateData(FALSE);
			return TRUE;
		} else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_UP) {
			RecallCommand(VK_UP);
			return TRUE;
		} else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DOWN) {
			RecallCommand(VK_DOWN);
			return TRUE;
		} else if (pMsg->message == WM_CHAR && pMsg->wParam == 0x17/*^W*/) {
			SendMessage(WM_COMMAND, IDCANCEL, 0);
			return TRUE;
		} else if (pMsg->message == WM_CHAR && pMsg->wParam == 0x11/*^Q*/) {
			::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

LRESULT CClanWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	return(CDialog ::WindowProc(message, wParam, lParam));
}

void CClanWindow::OnUpdateClanCommands() 
{
	UpdateData(TRUE);
	SetClanWinIcons();
}

void CClanWindow::RecallCommand(short type) {
	if (type == VK_UP) {
		type = curCommand;
		if (--curCommand < 0)
			curCommand = NUM_COMMANDS - 1;
		while ((clan_commands[curCommand] == NULL || *clan_commands[curCommand] == EOS) && curCommand != type) {
			if (--curCommand < 0)
				curCommand = NUM_COMMANDS - 1;
		}
	} else {
		type = curCommand;
		if (curCommand == lastCommand) {
			if (m_CommandsControl.LineLength(-1) < 1)
				curCommand--;
		}
		if (++curCommand >= NUM_COMMANDS)
			curCommand = 0;
		while ((clan_commands[curCommand] == NULL || *clan_commands[curCommand] == EOS) && curCommand != type) {
			if (++curCommand >= NUM_COMMANDS)
				curCommand = 0;
		}
	}

	if (clan_commands[curCommand] != NULL) {
		m_Commands = u_strcpy(fbufferU,clan_commands[curCommand],EXPANSION_SIZE);
		SetClanWinIcons();
		UpdateData(FALSE);
		int len = strlen(fbufferU);
		m_CommandsControl.SetSel(len, len, FALSE);
	}
}

void CClanWindow::OnCancel() {
	CRect lpRect;
	extern char isMORXiMode;

	isMORXiMode = FALSE;
	StdInWindow = NULL;
	StdInErrMessage = NULL;
	StdDoneMessage = NULL;
	CleanUpAll(TRUE);
	this->GetWindowRect(&lpRect);
	ClanWinSize.top = lpRect.top;
	ClanWinSize.left = lpRect.left;
	ClanWinSize.width = lpRect.right;
	ClanWinSize.height = lpRect.bottom;
	WriteCedPreference();
	DestroyWindow();
	if (clanDlg != NULL) {
		delete clanDlg;
		clanDlg = NULL;
	}
	free_aliases();
}

void CClanWindow::OnDestroy() 
{
	CRect lpRect;
	extern char isMORXiMode;

	isMORXiMode = FALSE;
	StdInWindow = NULL;
	StdInErrMessage = NULL;
	StdDoneMessage = NULL;
	CleanUpAll(TRUE);
	this->GetWindowRect(&lpRect);
	ClanWinSize.top = lpRect.top;
	ClanWinSize.left = lpRect.left;
	ClanWinSize.width = lpRect.right;
	ClanWinSize.height = lpRect.bottom;
	WriteCedPreference();
	CDialog::OnDestroy();
	if (clanDlg != NULL) {
		delete clanDlg;
		clanDlg = NULL;
	}
	free_aliases();
}

void CClanWindow::OnClanProgs() 
{
	int len;
	int item;

	item = m_ProgsCtrl.
		GetCurSel();
	m_ProgsCtrl.SetCurSel(0);
	UpdateData(TRUE);
	if (item > 0) {
		m_ProgsCtrl.GetLBText(item, ced_line);
		m_Commands = ced_line;
		m_Commands = m_Commands + " ";
		UpdateData(FALSE);
		SetClanWinIcons();
		len = strlen(m_Commands);
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
		m_CommandsControl.SetSel(len, len, FALSE);
	} else {
		DWORD cPos = m_CommandsControl.GetSel();
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
		m_CommandsControl.SetSel(cPos, FALSE);
	}
	m_ProgsCtrl.SetCurSel(0);
}

void CClanWindow::OnClanProgsCancel() 
{
	DWORD cPos = m_CommandsControl.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
	m_CommandsControl.SetSel(cPos, FALSE);
}

void CClanWindow::OnClanHelp() 
{
	char fname[FNSize];

	strcpy(fname, lib_dir);
	strcat(fname, "commands.cut");
	if (!access(fname, 0)) {
		nameOverride = "commands.cut";
		pathOverride = lib_dir;
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW, NULL);
	} else
		do_warning("Can't open help file: commands.cut. Check to see if lib directory is set correctly.", 0);

	DWORD cPos = m_CommandsControl.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
	m_CommandsControl.SetSel(cPos, FALSE);
}

char LocateDepfile(char *name) {
	myFInfo	*tGlobal_df;
	DWORD cPos;

	tGlobal_df = global_df;
	global_df = NULL;
	if (clanDlg != NULL)
		cPos = clanDlg->m_CommandsControl.GetSel();
	if (LocateDir("Please locate library directory with depfile", lib_dir, 0)) {
		WriteCedPreference();
		if (clanDlg != NULL) {
			u_strcpy(clanDlg->t_st, lib_dir, FNSize);
			AdjustName(clanDlg->lib_st, clanDlg->t_st, 39);
			clanDlg->m_LibSt = clanDlg->lib_st;
			clanDlg->UpdateData(FALSE);
		}
	} else
		return(FALSE);
	global_df = tGlobal_df;
	if (clanDlg != NULL) {
		clanDlg->GotoDlgCtrl(clanDlg->GetDlgItem(IDC_CLAN_COMMANDS));
		clanDlg->m_CommandsControl.SetSel(cPos, FALSE);
	}
	return(TRUE);
}

void CClanWindow::OnClanLib() 
{
	myFInfo	*tGlobal_df;

	tGlobal_df = global_df;
	global_df = NULL;
	DWORD cPos = m_CommandsControl.GetSel();
	if (LocateDir("Please locate library directory", lib_dir, 0)) {
		WriteCedPreference();
		u_strcpy(t_st, lib_dir, FNSize);
		AdjustName(lib_st, t_st, 39);
		m_LibSt = lib_st;
		UpdateData(FALSE);
		free_aliases();
		readAliases(1);
	}
	global_df = tGlobal_df;
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
	m_CommandsControl.SetSel(cPos, FALSE);
}

void CClanWindow::OnClanMorLib() 
{
	myFInfo	*tGlobal_df;
	tGlobal_df = global_df;
	global_df = NULL;
	DWORD cPos = m_CommandsControl.GetSel();
	if (LocateDir("Please locate mor library directory", mor_lib_dir, 0)) {
		WriteCedPreference();
		u_strcpy(t_st, mor_lib_dir, FNSize);
		AdjustName(mor_lib_st, t_st, 39);
		m_morLibSt = mor_lib_st;
		UpdateData(FALSE);
	}
	global_df = tGlobal_df;
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
	m_CommandsControl.SetSel(cPos, FALSE);
}

void CClanWindow::OnClanOd() 
{
	myFInfo	*tGlobal_df;
	tGlobal_df = global_df;
	global_df = NULL;
	DWORD cPos = m_CommandsControl.GetSel();
	if (LocateDir("Please locate output directory", od_dir, 0)) {
		WriteCedPreference();
		if ((pathcmp(od_dir, wd_dir)) == 0)
			m_OdSt = _T("");
		else {
			u_strcpy(t_st, od_dir, FNSize);
			AdjustName(od_st, t_st, 39);
			m_OdSt = od_st;
		}
		UpdateData(FALSE);
	}
	global_df = tGlobal_df;		
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
	m_CommandsControl.SetSel(cPos, FALSE);
}

void CClanWindow::OnClanWd() 
{
	myFInfo	*tGlobal_df;
	FNType tref[FNSize];

	tGlobal_df = global_df;
	global_df = NULL;
	strcpy(tref, wd_dir);
	DWORD cPos = m_CommandsControl.GetSel();
	if (LocateDir("Please locate working directory", wd_dir, 0)) {
		if (pathcmp(od_dir, tref) == 0) {
			strcpy(od_dir, wd_dir);
		}
		WriteCedPreference();
		if ((pathcmp(od_dir, wd_dir)) == 0)
			m_OdSt = _T("");
		else {
			u_strcpy(t_st, od_dir, FNSize);
			AdjustName(od_st, t_st, 39);
			m_OdSt = od_st;
		}
		u_strcpy(t_st, wd_dir, FNSize);
		AdjustName(wd_st, t_st, 39);
		m_WdSt = wd_st;
		UpdateData(FALSE);
	}
	global_df = tGlobal_df;	
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
	m_CommandsControl.SetSel(cPos, FALSE);
}

void CClanWindow::OnClanRun() 
{
	extern long rowLimitOverride;

	UpdateData(TRUE);
	u_strcpy(fbuffer, m_Commands, EXPANSION_SIZE);

	if (!init_clan())
		return;
	uS.remFrontAndBackBlanks(fbuffer);
	if (fbuffer[0] != EOS) {
		m_Commands = _T("");
		HideClanWinIcons();
		UpdateData(FALSE);
		AddToClan_commands(fbuffer);
		clanBuf = fbuffer;
		clanRun = TRUE;
		nameOverride = "CLAN Output";
		rowLimitOverride = ClanWinRowLim;
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW, NULL);
	}
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
}

void CClanWindow::OnClanRecall()
{
	CClanRecall dlg;

	UpdateData(TRUE);
	*ced_line = EOS;
	if (dlg.DoModal() == IDOK) {
		if (*ced_line != EOS) {
			m_Commands = ced_line;
			UpdateData(FALSE);
			SetClanWinIcons();
			int len = strlen(m_Commands);
			GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
			m_CommandsControl.SetSel(len, len, FALSE);
		}
	} else {
		DWORD cPos = m_CommandsControl.GetSel();
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
		m_CommandsControl.SetSel(cPos, FALSE);
	}
}

void CClanWindow::OnClanFilein() 
{
	int i, len, ProgNum;
	char *s, *com, progName[512+1];

	u_strcpy(fbuffer, m_Commands, EXPANSION_SIZE);
	for (i=0L; isSpace(fbuffer[i]); i++) ;
	if (i > 0L)
		strcpy(fbuffer, fbuffer+i);
	com = fbuffer;
	s = strchr(com, ' ');
	if (s != NULL)
		*s = EOS;

	if (getAliasProgName(com, progName, 512)) {
		if (s != NULL)
			*s = ' ';
		s = NULL;
		com = progName;
	}
	ProgNum = get_clan_prog_num(com, FALSE);
	UpdateData(TRUE);
	if (ProgNum == EVAL) {
		DWORD cPos = m_CommandsControl.GetSel();
		EvalDialog(templineW);
		if (templineW[0] != EOS) {
			strcpy(fbufferU, templineW);
			len = strlen(fbufferU);
			m_Commands = fbufferU;
			UpdateData(FALSE);
			SetClanWinIcons();
			GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
			m_CommandsControl.SetSel(len, len, FALSE);
		} else {
			GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
			m_CommandsControl.SetSel(cPos, FALSE);
		}
	} else if (ProgNum == KIDEVAL) {
		DWORD cPos = m_CommandsControl.GetSel();
		KidevalDialog(templineW);
		if (templineW[0] != EOS) {
			strcpy(fbufferU, templineW);
			len = strlen(fbufferU);
			m_Commands = fbufferU;
			UpdateData(FALSE);
			SetClanWinIcons();
			GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
			m_CommandsControl.SetSel(len, len, FALSE);
		} else {
			GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
			m_CommandsControl.SetSel(cPos, FALSE);
		}
	} else {
		myget();
		DWORD cPos = m_CommandsControl.GetSel();
		if (F_numfiles > 0) {
			strcpy(fbufferU, m_Commands);
			for (i=0L; isSpace(fbufferU[i]); i++) ;
			if (i > 0L)
				strcpy(fbufferU, fbufferU+i);
			len = strlen(fbufferU);
			if (!isAtOnCommandLineFound(fbufferU)) {
				strcat(fbufferU, " @");
				len += 2;
			}
			m_Commands = fbufferU;
			UpdateData(FALSE);
			SetClanWinIcons();
			GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
			m_CommandsControl.SetSel(len, len, FALSE);
		} else {
			GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
			m_CommandsControl.SetSel(cPos, FALSE);
		}
	}
}

void CClanWindow::OnClanFileout() 
{
	// TODO: Add your control notification handler code here
	
	DWORD cPos = m_CommandsControl.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
	m_CommandsControl.SetSel(cPos, FALSE);
}

void CClanWindow::OnClanOpt() 
{
	int i, len;
	// TODO: Add your control notification handler code here
	
	UpdateData(TRUE);
	DWORD cPos = m_CommandsControl.GetSel();
	strcpy(fbufferU, m_Commands);
	TiersDialog(templineW, fbufferU);
	if (templineW[0] != EOS) {
		strcpy(fbufferU, m_Commands);
		for (i=0L; isSpace(fbufferU[i]); i++) ;
		if (i > 0L)
			strcpy(fbufferU, fbufferU+i);
		strcat(fbufferU,templineW);
		len = strlen(fbufferU);
		m_Commands = fbufferU;
		UpdateData(FALSE);
		SetClanWinIcons();
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
		m_CommandsControl.SetSel(len, len, FALSE);
	} else {
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
		m_CommandsControl.SetSel(cPos, FALSE);
	}
}

void CClanWindow::OnClanSearch() 
{
	int i, len;
	// TODO: Add your control notification handler code here
	
	UpdateData(TRUE);
	DWORD cPos = m_CommandsControl.GetSel();
	SearchDialog(templineW);
	if (templineW[0] != EOS) {
		strcpy(fbufferU, m_Commands);
		for (i=0L; isSpace(fbufferU[i]); i++) ;
		if (i > 0L)
			strcpy(fbufferU, fbufferU+i);
		strcat(fbufferU,templineW);
		len = strlen(fbufferU);
		m_Commands = fbufferU;
		UpdateData(FALSE);
		SetClanWinIcons();
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
		m_CommandsControl.SetSel(len, len, FALSE);
	} else {
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_COMMANDS));
		m_CommandsControl.SetSel(cPos, FALSE);
	}
}

void CClanWindow::OnChangeClanCommands() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
}

/////////////////////////////////////////////////////////////////////////////
// CClanWindow Recall functions

void CClanWindow::AddToClan_commands(char *st) {
	register int i;

	if (strlen(st) >= MAX_COM_LEN)
		return;

	i = lastCommand - 1;
	if (i < 0)
		i = NUM_COMMANDS - 1;
	if (clan_commands[i] != NULL && !strcmp(clan_commands[i], st)) {
		curCommand = lastCommand;
		return;
	}
	i = lastCommand;
	if (clan_commands[lastCommand] == NULL) {
		for (lastCommand++; clan_commands[lastCommand] == NULL && lastCommand != i; lastCommand++) {
			if (lastCommand >= NUM_COMMANDS)
				lastCommand = 0;
		}
	}
	if (clan_commands[lastCommand] != NULL) {
		strncpy(clan_commands[lastCommand], st, MAX_COM_LEN);
		clan_commands[lastCommand][MAX_COM_LEN] = EOS;
		lastCommand++;
		if (lastCommand >= NUM_COMMANDS)
			lastCommand = 0;
		curCommand = lastCommand;
		WriteClanPreference();
	}
}

char *getRecallCommand(int i) {
	return(clan_commands[i]);
}
int getTotalRecallCommands() {
	return(NUM_COMMANDS);
}

int getCurRecallCommands() {
	short oc, cc;

	oc = curCommand;
	cc = curCommand;
	if (--cc < 0)
		cc = NUM_COMMANDS - 1;
	while ((clan_commands[cc] == NULL || *clan_commands[cc] == EOS) && cc != oc) {
		if (--cc < 0)
			cc = NUM_COMMANDS - 1;
	}
	return(cc);
}

void init_commands(void) {
	register int i;

	curCommand = 0;
	lastCommand = 0;
	for (i=0; i < NUM_COMMANDS; i++) {
		clan_commands[i] = (char *)malloc(MAX_COM_LEN+1);
		if (clan_commands[i] != NULL)
			*clan_commands[i] = EOS;
	}
}

void free_commands(void) {
	register int i;

	for (i=0; i < NUM_COMMANDS; i++) {
		if (clan_commands[i] != NULL)
			free(clan_commands[i]);
	}
}

void set_commands(char *text) {
	register int i;

	for (i=0; i < NUM_COMMANDS; i++) {
		if (clan_commands[i] != NULL && *clan_commands[i] == EOS) {
			strncpy(clan_commands[i], text, MAX_COM_LEN);
			clan_commands[i][MAX_COM_LEN] = EOS;
			break;
		}
	}
}

void set_lastCommand(short num) {
	lastCommand = num;
	if (lastCommand < 0 || lastCommand >= NUM_COMMANDS)
		lastCommand = 0;
	curCommand  = lastCommand;
}

void write_commands1977(FILE *fp) {
	register int i;

	for (i=0; i < NUM_COMMANDS; i++) {
		if (clan_commands[i] != NULL && *clan_commands[i] != EOS)
			fprintf(fp, "%d=%s\n", 1977, clan_commands[i]);
	}
}

void re_readAliases(void) {
	if (clanDlg != NULL) {
//		free_aliases();
//		readAliases(1);
	}
}
/////////////////////////////////////////////////////////////////////////////
// CClanRecall dialog
extern struct DefWin RecallWinSize;

 IMPLEMENT_DYNAMIC(CClanRecall, CDialog)

CClanRecall::CClanRecall(CWnd* pParent /*=NULL*/)
	: CDialog(CClanRecall::IDD, pParent)
{

}

void CClanRecall::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanRecall)
	DDX_Control(pDX, IDC_CLAN_RECALLS, m_ClanRecallListControl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CClanRecall, CDialog)
	//{{AFX_MSG_MAP(CClanProgs)
	ON_LBN_DBLCLK(IDC_CLAN_RECALLS, OnDblclkClanRecalls)
	ON_LBN_SELCHANGE(IDC_CLAN_RECALLS, OnSelchangeClanRecalls)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CClanRecall message handlers
BOOL CClanRecall::OnInitDialog() {
	int i, total_commands;
	char *command;
	CRect lpRect;
	extern int getTotalRecallCommands();
	extern int getCurRecallCommands();
	extern char *getRecallCommand(int i);

	CDialog::OnInitDialog();

	if (RecallWinSize.top || RecallWinSize.width || RecallWinSize.height || RecallWinSize.left) {
		this->GetWindowRect(&lpRect);
		lpRect.top = RecallWinSize.top;
		lpRect.bottom = RecallWinSize.height;
		lpRect.left = RecallWinSize.left;
		lpRect.right = RecallWinSize.width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}

	total_commands = getTotalRecallCommands();
	for (i=0; i < total_commands; i++) {
		command = getRecallCommand(i);
		if (command != NULL && *command != EOS) {
			u_strcpy(templineW, command, UTTLINELEN);
			m_ClanRecallListControl.AddString(templineW);
		}
	}
	i = getCurRecallCommands();
	m_ClanRecallListControl.SetCurSel(i);
	m_ClanRecallListControl.GetText(i, ced_line);
	return TRUE;
}

void CClanRecall::OnDblclkClanRecalls() 
{
	int i;
	CRect lpRect;

	i = m_ClanRecallListControl.GetCurSel();
	if (i != LB_ERR) {
		m_ClanRecallListControl.GetText(i, ced_line);
		set_lastCommand(i);
	}

	this->GetWindowRect(&lpRect);
	RecallWinSize.top = lpRect.top;
	RecallWinSize.left = lpRect.left;
	RecallWinSize.width = lpRect.right;
	RecallWinSize.height = lpRect.bottom;
	WriteCedPreference();
	EndDialog(IDOK);
}

void CClanRecall::OnSelchangeClanRecalls() 
{
	int i;

	i = m_ClanRecallListControl.GetCurSel();
	if (i != LB_ERR)
		m_ClanRecallListControl.GetText(i, ced_line);
}

void CClanRecall::OnCancel() {
	CRect lpRect;

	this->GetWindowRect(&lpRect);
	RecallWinSize.top = lpRect.top;
	RecallWinSize.left = lpRect.left;
	RecallWinSize.width = lpRect.right;
	RecallWinSize.height = lpRect.bottom;
	WriteCedPreference();
	EndDialog(IDCANCEL);
}

void CClanRecall::OnOK() {
	CRect lpRect;

	this->GetWindowRect(&lpRect);
	RecallWinSize.top = lpRect.top;
	RecallWinSize.left = lpRect.left;
	RecallWinSize.width = lpRect.right;
	RecallWinSize.height = lpRect.bottom;
	WriteCedPreference();
	EndDialog(IDOK);
}

void CClanRecall::ResizeRecallWindow(int cx, int cy) {
	RECT itemRect, winRect;
	UINT buttonsHight, buttonsWidth;
	CWnd *pw_OK, *pw_cancel, *pw_ListBox;

	pw_OK=this->GetDlgItem(IDOK);
	pw_cancel=this->GetDlgItem(IDCANCEL);
	pw_ListBox=this->GetDlgItem(IDC_CLAN_RECALLS);
	if (this == NULL || pw_OK == NULL || pw_cancel == NULL || pw_ListBox == NULL)
		return;
	this->GetClientRect(&winRect);

	pw_OK->GetWindowRect(&itemRect);
	buttonsHight = itemRect.bottom - itemRect.top;
	buttonsWidth = itemRect.right - itemRect.left;
	itemRect.top=winRect.bottom - buttonsHight - 10;
	itemRect.bottom=itemRect.top + buttonsHight;
	itemRect.left=winRect.left + 10;
	itemRect.right=itemRect.left + buttonsWidth;
	pw_OK->MoveWindow(&itemRect,true);

	pw_cancel->GetWindowRect(&itemRect);
	buttonsHight = itemRect.bottom - itemRect.top;
	buttonsWidth = itemRect.right - itemRect.left;
	itemRect.top=winRect.bottom - buttonsHight - 10;
	itemRect.bottom=itemRect.top + buttonsHight;
	itemRect.left=winRect.right - buttonsWidth - 10;
	itemRect.right=itemRect.left + buttonsWidth;
	pw_cancel->MoveWindow(&itemRect,true);

	pw_ListBox->GetWindowRect(&itemRect);
	itemRect.top=winRect.top + 10;
	itemRect.bottom=winRect.bottom - buttonsHight - 17;
	itemRect.left=winRect.left + 10;
	itemRect.right=winRect.right - 10;
	pw_ListBox->MoveWindow(&itemRect,true);
}

void CClanRecall::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	ResizeRecallWindow(cx, cy);
}
