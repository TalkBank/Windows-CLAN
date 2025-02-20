// CedDlgs.cpp : implementation file
//

#include "ced.h"
#include "cu.h"
#include "search.h"
#include "ceddlgs.h"

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

/////////////////////////////////////////////////////////////////////////////
// CUpdateCLAN dialog

CUpdateCLAN::CUpdateCLAN(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateCLAN::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdateCLAN)
	m_Message= _T("");
	//}}AFX_DATA_INIT
}

void CUpdateCLAN::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateCLAN)
	DDX_Text(pDX, IDC_UPDATE_MESSAGE, m_Message);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUpdateCLAN, CDialog)
	//{{AFX_MSG_MAP(CUpdateCLAN)
	ON_BN_CLICKED(IDC_UPDATE_CLAN, OnUpdate)
	ON_BN_CLICKED(IDC_UPDATE_SKIP, OnSkip)
	ON_BN_CLICKED(IDC_UPDATE_NO_CHECK, OnDontCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateCLAN message handlers

void CUpdateCLAN::OnUpdate() 
{
	UpdateData(TRUE);
	res = 1;
	EndDialog(IDOK);
}

void CUpdateCLAN::OnSkip() 
{
	UpdateData(TRUE);
	res = 2;
	EndDialog(IDOK);
}

void CUpdateCLAN::OnDontCheck() 
{
	UpdateData(TRUE);
	res = 3;
	EndDialog(IDOK);
}
/////////////////////////////////////////////////////////////////////////////
// CCedDlgs dialog

CCedDlgs::CCedDlgs(CWnd* pParent /*=NULL*/)
	: CDialog(CCedDlgs::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCedDlgs)
	m_nCheckPointCnt = 0;
	m_ClanWindowLinesLimit = 0;
	m_CreateBackup = FALSE;
	m_OpenClan = FALSE;
	m_StartEditorMode = FALSE;
	m_WrapClanOutput = FALSE;
	m_WrapLine = FALSE;
	m_RestoreCursor = FALSE;
	m_Tier = _T("");
	m_Mixed_Stereo_Wave = FALSE;
	m_Update_Clan = FALSE;
	m_No_CheckMess = FALSE;
	//}}AFX_DATA_INIT
}


void CCedDlgs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCedDlgs)
	DDX_Text(pDX, IDC_CEDO_CHECKPOINT_E, m_nCheckPointCnt);
	DDV_MinMaxInt(pDX, m_nCheckPointCnt, 0, 16000);
	DDX_Text(pDX, IDC_CEDO_CLAN_LIMIT, m_ClanWindowLinesLimit);
	DDV_MinMaxLong(pDX, m_ClanWindowLinesLimit, 0, 16000);
	DDX_Control(pDX, IDC_CEDO_COLORLEMMA, m_ColorLemmaCtrl);
// Default color;Blue Color;Orange Color;Magenta Color;Purple Color;Brown Color
	if (m_LemmaColorNum >= 0 && m_LemmaColorNum <= 5)
		m_ColorLemmaCtrl.SetCurSel(m_LemmaColorNum);
	else
		m_ColorLemmaCtrl.SetCurSel(0);
	DDX_Check(pDX, IDC_CEDO_CREATE_BACKUP, m_CreateBackup);
	DDX_Check(pDX, IDC_CEDO_OPEN_CLAN, m_OpenClan);
	DDX_Check(pDX, IDC_CEDO_START_EDITOR, m_StartEditorMode);
	DDX_Check(pDX, IDC_CEDO_RESTORE_CURSOR, m_RestoreCursor);
	DDX_Check(pDX, IDC_CEDO_WRAP_CLAN_LINES, m_WrapClanOutput);
	DDX_Check(pDX, IDC_CEDO_WRAP_LINES, m_WrapLine);
	DDX_Text(pDX, IDC_CEDO_DISAMBIG, m_Tier);
	DDX_Check(pDX, IDC_CEDO_MIXED_STEREO, m_Mixed_Stereo_Wave);
	DDX_Check(pDX, IDC_CEDO_UPDATE_CLAN, m_Update_Clan);
	DDX_Check(pDX, IDC_CEDO_NO_CHECK_MESSAGE, m_No_CheckMess);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCedDlgs, CDialog)
	//{{AFX_MSG_MAP(CCedDlgs)
	ON_CBN_SELCHANGE(IDC_CEDO_COLORLEMMA, OnColorLemma)
	ON_CBN_SELENDCANCEL(IDC_CEDO_COLORLEMMA, OnColorLemmaCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCedDlgs::OnColorLemma()
{
	int len;

	m_LemmaColorNum = m_ColorLemmaCtrl.GetCurSel();
	UpdateData(TRUE);

//	m_ColorLemmaCtrl.SetCurSel(LemmasColorNumPtr);
}

void CCedDlgs::OnColorLemmaCancel()
{
	DWORD cPos = m_ColorLemmaCtrl.GetCurSel();
}

/////////////////////////////////////////////////////////////////////////////
// CCedFindString dialog


CCedFindString::CCedFindString(CWnd* pParent /*=NULL*/)
	: CDialog(CCedFindString::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCedFindString)
	m_SearchString = _T("");
	m_SearchBackwards = FALSE;
	m_CaseSensitive = FALSE;
	m_Wrap = FALSE;
	m_UseFileList = FALSE;
	m_SearchListFName = _T("");
	//}}AFX_DATA_INIT
}

void CCedFindString::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCedFindString)
	DDX_Control(pDX, IDC_FIND_STRING, m_SearchStrCtrl);
	DDX_Text(pDX, IDC_FIND_STRING, m_SearchString);
	DDX_Check(pDX, IDC_FIND_BACKWARD, m_SearchBackwards);
	DDX_Check(pDX, IDC_FIND_CASE_SEN, m_CaseSensitive);
	DDX_Check(pDX, IDC_FIND_WRAP, m_Wrap);
	DDX_Check(pDX, IDC_FIND_FILE_LIST, m_UseFileList);
	DDX_Text(pDX, IDC_FIND_LIST_FNAME, m_SearchListFName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCedFindString, CDialog)
	//{{AFX_MSG_MAP(CCedFindString)
	ON_BN_CLICKED(IDC_FIND_INS_RETURN, OnFindInsReturn)
	ON_BN_CLICKED(IDC_FIND_INS_TAB, OnFindInsTab)
	ON_BN_CLICKED(IDC_FIND_INS_BULLET, OnFindInsBullet)
	ON_BN_CLICKED(IDC_FIND_FROM_FILE, OnFindFromFile)
	ON_BN_CLICKED(IDC_FIND_FILE_LIST, OnFindFileList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCedFindString message handlers


BOOL CCedFindString::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1 &&
		_AfxCompareClassName(pMsg->hwnd, _T("Edit"))) {
		return TRUE;
	} else if (pMsg->message == WM_CHAR && pMsg->wParam == 0x17/*^W*/&&
		_AfxCompareClassName(pMsg->hwnd, _T("Edit"))) {
		return TRUE;
	}
/*
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
*/
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CCedFindString::OnFindInsReturn() 
{
	int  s, e;

	m_SearchStrCtrl.GetSel(s, e);
	m_SearchStrCtrl.ReplaceSel(cl_T("\n"), FALSE);
	GotoDlgCtrl(GetDlgItem(IDC_FIND_STRING));
	m_SearchStrCtrl.SetSel(s+1, s+1, FALSE);
}

void CCedFindString::OnFindInsTab() 
{
	int  s, e;

	m_SearchStrCtrl.GetSel(s, e);
	m_SearchStrCtrl.ReplaceSel(cl_T("\t"), FALSE);
	GotoDlgCtrl(GetDlgItem(IDC_FIND_STRING));
	m_SearchStrCtrl.SetSel(s+1, s+1, FALSE);
}

void CCedFindString::OnFindInsBullet() 
{
	int  s, e;

	templine3[0] = 0x2022;
	templine3[1] = EOS;
	m_SearchStrCtrl.GetSel(s, e);
	m_SearchStrCtrl.ReplaceSel(templine3, FALSE);
	GotoDlgCtrl(GetDlgItem(IDC_FIND_STRING));
	m_SearchStrCtrl.SetSel(s+1, s+1, FALSE);
}


void CCedFindString::OnFindFileList() 
{
	UpdateData(TRUE);
	if (!isThereSearchList())
		m_UseFileList = 0;
	UpdateData(FALSE);
}

void CCedFindString::OnFindFromFile() 
{
    unCH			*szFilter, *t;
	OPENFILENAME	ofn;
	unCH			wDirPathName[FNSize];

	szFilter = _T("Search File (*.cut)\0*.cut\0All files (*.*)\0*.*\0\0");
    strcpy(searchFName, "");
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = searchFName;
    ofn.nMaxFile = sizeof(searchFName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = u_strcpy(wDirPathName, wd_dir, FNSize);
    ofn.lpstrTitle = _T("Please locate search string file");
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;

	DrawCursor(1);
	if (GetOpenFileName(&ofn) == 0) {
		DrawCursor(0);
		searchFName[0] = EOS;
	} else {
		DrawCursor(0);
		if ((t=strrchr(searchFName,'\\')) != NULL)
			strcpy(searchFName, t+1);
		u_strcpy(searchListFileName, searchFName, _MAX_PATH+_MAX_FNAME);
		if (readSearchList(searchListFileName)) {
			if (!isThereSearchList()) {
				searchListFileName[0] = EOS;
			} else
				m_UseFileList = TRUE;
			m_SearchListFName = cl_T(searchListFileName);
			UpdateData(FALSE);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGotoLineNumber dialog

CGotoLineNumber::CGotoLineNumber(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoLineNumber::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGotoLineNumber)
	m_LineNumber = 0;
	//}}AFX_DATA_INIT
}


void CGotoLineNumber::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGotoLineNumber)
	DDX_Text(pDX, IDC_EDIT_GOTO_LINENUMBER, m_LineNumber);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGotoLineNumber, CDialog)
	//{{AFX_MSG_MAP(CGotoLineNumber)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGotoLineNumber message handlers
/////////////////////////////////////////////////////////////////////////////
// CReplaceString dialog

CReplaceString::CReplaceString(CWnd* pParent /*=NULL*/)
	: CDialog(CReplaceString::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReplaceString)
	m_ReplaceString = _T("");
	m_ReplaceWith = _T("");
	m_ReplaceListFName = _T("");
	m_CaseSensitive = FALSE;
	//}}AFX_DATA_INIT
	isRepStr = 0;
	ReplaceFromList = FALSE;
}

void CReplaceString::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplaceString)
	DDX_Control(pDX, IDC_EDIT_REPLACE_WITH, m_ReplaceWithCtrl);
	DDX_Control(pDX, IDC_EDIT_REPLACE_STRING, m_ReplaceStrCtrl);
	DDX_Text(pDX, IDC_EDIT_REPLACE_STRING, m_ReplaceString);
	DDX_Text(pDX, IDC_EDIT_REPLACE_WITH, m_ReplaceWith);
	DDX_Check(pDX, IDC_REPLACE_CASE_SEN, m_CaseSensitive);
	DDX_Text(pDX, IDC_REPLACE_LIST_FNAME, m_ReplaceListFName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReplaceString, CDialog)
	//{{AFX_MSG_MAP(CReplaceString)
	ON_BN_CLICKED(IDC_REPLACE_SKIP, OnReplaceSkip)
	ON_BN_CLICKED(IDC_REPLACE_THIS, OnReplaceThis)
	ON_BN_CLICKED(IDC_REPLACE_CASE_SEN, OnReplaceCaseSen)
	ON_BN_CLICKED(IDC_REPLACE_INS_RETURN, OnReplaceInsReturn)
	ON_BN_CLICKED(IDC_REPLACE_INS_TAB, OnReplaceInsTab)
	ON_BN_CLICKED(IDC_REPLACE_USE_FILE, OnReplaceUseFile)
	ON_BN_CLICKED(IDC_REPLACE_FROM_FILE, OnReplaceFromFile)
	ON_EN_SETFOCUS(IDC_EDIT_REPLACE_STRING, OnSetfocusEditReplaceString)
	ON_EN_SETFOCUS(IDC_EDIT_REPLACE_WITH, OnSetfocusEditReplaceWith)
	ON_BN_CLICKED(IDC_REPLACE_INS_BULLET, OnReplaceInsBullet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplaceString message handlers

void CReplaceString::OnReplaceSkip() 
{
	int s, e, res;

	UpdateData(TRUE);
	if (isRepStr == 1)
		m_ReplaceStrCtrl.GetSel(s, e);
	else if (isRepStr == 2)
		m_ReplaceWithCtrl.GetSel(s, e);

	strcpy(SearchString, m_ReplaceString);
	strcpy(ReplaceString, m_ReplaceWith);
	res = replaceAndFindNext(FALSE);
	if (res == 0)
		EndDialog(IDCANCEL);
	else if (res == 2) {
		replaceExecPos = 0;
		EndDialog(IDCANCEL);
	}
	if (isRepStr == 1) {
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_STRING));
		m_ReplaceStrCtrl.SetSel(s, e, FALSE);
	} else if (isRepStr == 2) {
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_WITH));
		m_ReplaceWithCtrl.SetSel(s, e, FALSE);
	}
}

void CReplaceString::OnReplaceThis() 
{
	int s, e, res;

	UpdateData(TRUE);
	if (isRepStr == 1)
		m_ReplaceStrCtrl.GetSel(s, e);
	else if (isRepStr == 2)
		m_ReplaceWithCtrl.GetSel(s, e);

	strcpy(SearchString, m_ReplaceString);
	strcpy(ReplaceString, m_ReplaceWith);
	res = replaceAndFindNext(TRUE);
	if (res == 0)
		EndDialog(IDCANCEL);
	else if (res == 2) {
		replaceExecPos = 0;
		EndDialog(IDCANCEL);
	}

	if (isRepStr == 1) {
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_STRING));
		m_ReplaceStrCtrl.SetSel(s, e, FALSE);
	} else if (isRepStr == 2) {
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_WITH));
		m_ReplaceWithCtrl.SetSel(s, e, FALSE);
	}
}

void CReplaceString::OnReplaceCaseSen() 
{
	int  s, e;

	UpdateData(TRUE);
	if (isRepStr == 1)
		m_ReplaceStrCtrl.GetSel(s, e);
	else if (isRepStr == 2)
		m_ReplaceWithCtrl.GetSel(s, e);

	CaseSensSearch = m_CaseSensitive;

	if (isRepStr == 1) {
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_STRING));
		m_ReplaceStrCtrl.SetSel(s, e, FALSE);
	} else if (isRepStr == 2) {
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_WITH));
		m_ReplaceWithCtrl.SetSel(s, e, FALSE);
	}
}

void CReplaceString::OnReplaceInsReturn() 
{
	int  s, e;

	if (isRepStr == 1) {
		m_ReplaceStrCtrl.GetSel(s, e);
		m_ReplaceStrCtrl.ReplaceSel(cl_T("\n"), FALSE);
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_STRING));
		m_ReplaceStrCtrl.SetSel(s+1, s+1, FALSE);
	} else if (isRepStr == 2) {
		m_ReplaceWithCtrl.GetSel(s, e);
		m_ReplaceWithCtrl.ReplaceSel(cl_T("\n"), FALSE);
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_WITH));
		m_ReplaceWithCtrl.SetSel(s+1, s+1, FALSE);
	}
}

void CReplaceString::OnReplaceInsTab() 
{
	int  s, e;

	if (isRepStr == 1) {
		m_ReplaceStrCtrl.GetSel(s, e);
		m_ReplaceStrCtrl.ReplaceSel(cl_T("\t"), FALSE);
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_STRING));
		m_ReplaceStrCtrl.SetSel(s+1, s+1, FALSE);
	} else if (isRepStr == 2) {
		m_ReplaceWithCtrl.GetSel(s, e);
		m_ReplaceWithCtrl.ReplaceSel(cl_T("\t"), FALSE);
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_WITH));
		m_ReplaceWithCtrl.SetSel(s+1, s+1, FALSE);
	}
}

void CReplaceString::OnReplaceInsBullet() 
{
	int  s, e;

	templine3[0] = 0x2022;
	templine3[1] = EOS;
	if (isRepStr == 1) {
		m_ReplaceStrCtrl.GetSel(s, e);
		m_ReplaceStrCtrl.ReplaceSel(templine3, FALSE);
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_STRING));
		m_ReplaceStrCtrl.SetSel(s+1, s+1, FALSE);
	} else if (isRepStr == 2) {
		m_ReplaceWithCtrl.GetSel(s, e);
		m_ReplaceWithCtrl.ReplaceSel(templine3, FALSE);
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_WITH));
		m_ReplaceWithCtrl.SetSel(s+1, s+1, FALSE);
	}
}

void CReplaceString::OnSetfocusEditReplaceString() 
{
	isRepStr = 1;
}

void CReplaceString::OnSetfocusEditReplaceWith() 
{
	isRepStr = 2;
}

void CReplaceString::OnReplaceUseFile() 
{
	int				s, e;
    unCH			*szFilter, *t;
	unCH			wDirPathName[FNSize];
	OPENFILENAME	ofn;

	replaceFName[0] = EOS;
	if (replaceListFileName[0] == EOS) {
		if (isRepStr == 1)
			m_ReplaceStrCtrl.GetSel(s, e);
		else if (isRepStr == 2)
			m_ReplaceWithCtrl.GetSel(s, e);
		szFilter = _T("Replace File (*.cut)\0*.cut\0All files (*.*)\0*.*\0\0");
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
		ofn.lpstrFilter = szFilter;
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0L;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = replaceFName;
		ofn.nMaxFile = sizeof(replaceFName);
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = u_strcpy(wDirPathName, wd_dir, FNSize);
		ofn.lpstrTitle = _T("Please locate replace string file");
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
		ofn.nFileOffset = 0;
		ofn.nFileExtension = 0;
		ofn.lpstrDefExt = NULL;

		DrawCursor(1);
		if (GetOpenFileName(&ofn) == 0) {
			DrawCursor(0);
			replaceFName[0] = EOS;
		} else {
			if ((t=strrchr(replaceFName,'\\')) != NULL)
				strcpy(replaceFName, t+1);
			u_strcpy(replaceListFileName, replaceFName, _MAX_PATH+_MAX_FNAME);
			m_ReplaceListFName = cl_T(replaceListFileName);
			ReplaceFromList = TRUE;
			DrawCursor(0);
			EndDialog(IDOK);
		}

		if (isRepStr == 1) {
			GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_STRING));
			m_ReplaceStrCtrl.SetSel(s, e, FALSE);
		} else if (isRepStr == 2) {
			GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_WITH));
			m_ReplaceWithCtrl.SetSel(s, e, FALSE);
		}
	} else {
		ReplaceFromList = TRUE;
		EndDialog(IDOK);
	}
}

void CReplaceString::OnReplaceFromFile() 
{
	int				s, e;
    unCH			*szFilter, *t;
	unCH			wDirPathName[FNSize];
	FNType			cDirPathName[FNSize];
	OPENFILENAME	ofn;

	if (isRepStr == 1)
		m_ReplaceStrCtrl.GetSel(s, e);
	else if (isRepStr == 2)
		m_ReplaceWithCtrl.GetSel(s, e);

	replaceFName[0] = EOS;
	szFilter = _T("Replace File (*.cut)\0*.cut\0All files (*.*)\0*.*\0\0");
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = replaceFName;
    ofn.nMaxFile = sizeof(replaceFName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
	if (replaceListFileName[0] == EOS)
	    ofn.lpstrInitialDir = u_strcpy(wDirPathName, wd_dir, FNSize);
	else {
		extractPath(cDirPathName, replaceListFileName);
	    ofn.lpstrInitialDir = u_strcpy(wDirPathName, cDirPathName, FNSize);
	}
    ofn.lpstrTitle = _T("Please locate replace string file");
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;

	DrawCursor(1);
	if (GetOpenFileName(&ofn) == 0) {
		DrawCursor(0);
		replaceFName[0] = EOS;
	} else {
		if ((t=strrchr(replaceFName,'\\')) != NULL)
			strcpy(replaceFName, t+1);
		u_strcpy(replaceListFileName, replaceFName, _MAX_PATH+_MAX_FNAME);
		m_ReplaceListFName = cl_T(replaceListFileName);
		ReplaceFromList = TRUE;
		DrawCursor(0);
		EndDialog(IDOK);
	}

	if (isRepStr == 1) {
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_STRING));
		m_ReplaceStrCtrl.SetSel(s, e, FALSE);
	} else if (isRepStr == 2) {
		GotoDlgCtrl(GetDlgItem(IDC_EDIT_REPLACE_WITH));
		m_ReplaceWithCtrl.SetSel(s, e, FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////
// CReplaceCont dialog


CReplaceCont::CReplaceCont(CWnd* pParent /*=NULL*/)
	: CDialog(CReplaceCont::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReplaceCont)
	m_ReplaceString = _T("");
	m_ReplaceWith = _T("");
	//}}AFX_DATA_INIT
	result = 2;
}

void CReplaceCont::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplaceCont)
	DDX_Text(pDX, IDC_EDIT_REPLACE_CONT_STRING, m_ReplaceString);
	DDX_Text(pDX, IDC_EDIT_REPLACE_CONT_WITH, m_ReplaceWith);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReplaceCont, CDialog)
	//{{AFX_MSG_MAP(CReplaceCont)
	ON_BN_CLICKED(IDC_REPLACE_CONT_SKIP_THIS, OnReplaceContSkipThis)
	ON_BN_CLICKED(IDC_REPLACE_CONT_THIS, OnReplaceContThis)
	ON_BN_CLICKED(IDC_REPLACE_SKIP, OnReplaceSkip)
	ON_BN_CLICKED(IDC_REPLACE_ALL, OnReplaceAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplaceCont message handlers

void CReplaceCont::OnReplaceContSkipThis() 
{
	register int res;

	UpdateData(TRUE);
	strcpy(SearchString, m_ReplaceString);
	res = replaceAndFindNext(FALSE);
	if (res == 0)
		EndDialog(IDCANCEL);
	else if (res == 2) {
		replaceExecPos = 0;
		result = 3;
		EndDialog(IDOK);
	}
}

void CReplaceCont::OnReplaceContThis() 
{
	register int res;

	UpdateData(TRUE);
	strcpy(SearchString, m_ReplaceString);
	strcpy(ReplaceString, m_ReplaceWith);
	res = replaceAndFindNext(TRUE);
	if (res == 0)
		EndDialog(IDCANCEL);
	else if (res == 2) {
		replaceExecPos = 0;
		result = 3;
		EndDialog(IDOK);
	}
}

void CReplaceCont::OnReplaceAll() 
{
	result = 2;
	EndDialog(IDOK);
}

void CReplaceCont::OnReplaceSkip() 
{
	result = 1;
	EndDialog(IDOK);
}
/////////////////////////////////////////////////////////////////////////////
// CHelpWindow dialog
CHelpWindow::CHelpWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CHelpWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHelpWindow)
	m_HelpInput = _T("");
	m_HelpOutput = _T("");
	//}}AFX_DATA_INIT
}


void CHelpWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHelpWindow)
	DDX_Control(pDX, IDC_HELP_SHOW, m_HelpOutputControl);
	DDX_Control(pDX, IDC_HELP_INPUT, m_HelpInputControl);
	DDX_Text(pDX, IDC_HELP_INPUT, m_HelpInput);
	DDX_Text(pDX, IDC_HELP_SHOW, m_HelpOutput);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHelpWindow, CDialog)
	//{{AFX_MSG_MAP(CHelpWindow)
	ON_EN_CHANGE(IDC_HELP_INPUT, OnChangeHelpInput)
	ON_NOTIFY(EN_SELCHANGE, IDC_HELP_SHOW, OnSelchangeHelpShow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CHelpWindow message handlers
BOOL CHelpWindow::OnInitDialog() {
	CDialog::OnInitDialog();
// |ENM_KEYEVENTS|ENM_MOUSEEVENTS
	m_HelpOutputControl.SetEventMask(ENM_NONE|ENM_SELCHANGE);
	return TRUE;
}

void CHelpWindow::OnSelchangeHelpShow(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);	
	unCH st[SPEAKERLEN];
	int   fline, len;
	long  ln, ln2, s, e;

	m_HelpOutputControl.GetSel(s, e);
	if (e != s) {
		len = m_HelpOutputControl.LineLength(s);
		ln = m_HelpOutputControl.LineFromChar(s);
		ln2 = m_HelpOutputControl.LineFromChar(e);
		if (ln == ln2 || (ln == ln2-1 && len == e-s-2)) {
			ln = m_HelpOutputControl.LineFromChar(-1);
			ln = m_HelpOutputControl.GetLine(ln, st, SPEAKERLEN);
			st[ln] = EOS;
			for (ln=0; st[ln] != ':' && st[ln] != EOS; ln++) ;
			st[ln] = EOS;
			fline = m_HelpOutputControl.GetFirstVisibleLine();
			m_HelpInput = st;
			UpdateData(FALSE);
			m_HelpOutputControl.SetSel(0, 0);
			m_HelpOutputControl.LineScroll(fline, 0);
			m_HelpOutputControl.SetSel(s, s);
		}
	}
	*pResult = 0;
}

void CHelpWindow::OnChangeHelpInput() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	int sl[256];
	long i, len;
	char st[SPEAKERLEN];

	UpdateData(TRUE);
	strncpy(templine, m_HelpInput, UTTLINELEN);
	templine[UTTLINELEN-1] = EOS;
	u_strcpy(st, templine, SPEAKERLEN);
	len = strlen(st) - 1;
	if (st[len] == ' ') {
		st[len] = EOS;
		if (m_doHelp) {
			DisplayCommands(NULL, st, '\002');
			m_HelpInput = u_strcpy(templine, st, FNSize);
			len = strlen(st);
		} else {
			SortCommands(sl,st,FALSE);
			m_HelpInput = u_strcpy(templine, st, FNSize);
			m_HelpOutput = cl_T("");
			for (i=0; sl[i] != -1; i++) {
				AddKeysToCommand(sl[i], st);
				strcat(st, "\r\n");
				m_HelpOutput = m_HelpOutput + u_strcpy(templine, st, FNSize);
			}
		}
		UpdateData(FALSE);
		m_HelpInputControl.SetSel(len, len, FALSE);
	} else if (st[len] == '?') {
		st[len] = EOS;
		SortCommands(sl,st,m_doHelp);
		m_HelpInput = u_strcpy(templine, st, FNSize);
		m_HelpOutput = cl_T("");
		for (i=0; sl[i] != -1; i++) {
			AddKeysToCommand(sl[i], st);
			strcat(st, "\r\n");
			m_HelpOutput = m_HelpOutput + u_strcpy(templine, st, FNSize);
		}
		UpdateData(FALSE);
		m_HelpInputControl.SetSel(len, len, FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////
// CDefineMacro dialog


CDefineMacro::CDefineMacro(CWnd* pParent /*=NULL*/)
	: CDialog(CDefineMacro::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDefineMacro)
	m_Number = 0;
	m_String = _T("");
	//}}AFX_DATA_INIT
}


void CDefineMacro::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDefineMacro)
	DDX_Text(pDX, IDC_EDIT_NUMBER, m_Number);
	DDV_MinMaxInt(pDX, m_Number, 0, 9);
	DDX_Text(pDX, IDC_EDIT_STRING, m_String);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDefineMacro, CDialog)
	//{{AFX_MSG_MAP(CDefineMacro)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefineMacro message handlers
/////////////////////////////////////////////////////////////////////////////
// CSoundSync dialog


CSoundSync::CSoundSync(CWnd* pParent /*=NULL*/)
	: CDialog(CSoundSync::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundSync)
	m_ShowWave = FALSE;
	m_Nothing = FALSE;
	m_ShowAndPlay = FALSE;
	//}}AFX_DATA_INIT
}


void CSoundSync::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundSync)
	DDX_Check(pDX, IDC_DISPLAY_WAVE, m_ShowWave);
	DDX_Check(pDX, IDC_DONOTHING, m_Nothing);
	DDX_Check(pDX, IDC_DISPLAY_AND_PLAY, m_ShowAndPlay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSoundSync, CDialog)
	//{{AFX_MSG_MAP(CSoundSync)
	ON_BN_CLICKED(IDC_DISPLAY_AND_PLAY, OnDisplayAndPlay)
	ON_BN_CLICKED(IDC_DISPLAY_WAVE, OnDisplayWave)
	ON_BN_CLICKED(IDC_DONOTHING, OnDonothing)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoundSync message handlers

void CSoundSync::OnDisplayAndPlay() 
{
	m_Nothing = FALSE;
	m_ShowWave = FALSE;
	m_ShowAndPlay = TRUE;
	UpdateData(FALSE);
}

void CSoundSync::OnDisplayWave() 
{
	m_Nothing = FALSE;
	m_ShowWave = TRUE;
	m_ShowAndPlay = FALSE;
	UpdateData(FALSE);
}

void CSoundSync::OnDonothing() 
{
	m_Nothing = TRUE;
	m_ShowWave = FALSE;
	m_ShowAndPlay = FALSE;
	UpdateData(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// CConstStringNumber dialog

CConstStringNumber::CConstStringNumber(CWnd* pParent /*=NULL*/)
	: CDialog(CConstStringNumber::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConstStringNumber)
	m_StringNumber = 1;
	//}}AFX_DATA_INIT
}


void CConstStringNumber::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConstStringNumber)
	DDX_Control(pDX, IDC_CONST_LIST, m_ConstStringListControl);
	DDX_Text(pDX, IDC_STRINGNUMBER, m_StringNumber);
	DDV_MinMaxInt(pDX, m_StringNumber, 0, 9);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConstStringNumber, CDialog)
	//{{AFX_MSG_MAP(CConstStringNumber)
	ON_LBN_DBLCLK(IDC_CONST_LIST, OnDblclkConstList)
	ON_LBN_SELCHANGE(IDC_CONST_LIST, OnSelchangeConstList)
	ON_EN_CHANGE(IDC_STRINGNUMBER, OnChangeStringnumber)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConstStringNumber message handlers
BOOL CConstStringNumber::OnInitDialog() {
	int i;

	CDialog::OnInitDialog();

	for (i=0; i < 10; i++) {
		if (ConstString[i] != NULL) {
			u_strcpy(templineW, ConstString[i], UTTLINELEN);
			m_ConstStringListControl.AddString(templineW);
		} else
			m_ConstStringListControl.AddString(cl_T(" "));
	}
	return TRUE;
}

void CConstStringNumber::OnDblclkConstList() 
{
	int i;

	i = m_ConstStringListControl.GetCurSel();
	if (i != LB_ERR)
		m_StringNumber = i + 1;
	UpdateData(FALSE);
	EndDialog(IDOK);
}

void CConstStringNumber::OnSelchangeConstList() 
{
	int i;

	i = m_ConstStringListControl.GetCurSel();
	if (i != LB_ERR)
		m_StringNumber = i + 1;
	UpdateData(FALSE);
}

void CConstStringNumber::OnChangeStringnumber() 
{
	UpdateData(TRUE);
	if (m_StringNumber == 0) {
		m_StringNumber = 10;
		UpdateData(FALSE);
	}
	if (m_StringNumber >= 1 && m_StringNumber <= 10)
		EndDialog(IDOK);
}
/////////////////////////////////////////////////////////////////////////////
// CGetAscii dialog
CGetAscii::CGetAscii(CWnd* pParent /*=NULL*/)
	: CDialog(CGetAscii::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetAscii)
	m_AsciiInput = _T("");
	m_AsciiOutput = _T("");
	//}}AFX_DATA_INIT
}

void CGetAscii::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetAscii)
	DDX_Control(pDX, IDC_ENTER_ASCII_SHOW, m_AsciiOuputControl);
	DDX_Control(pDX, IDC_ENTER_ASCII_INPUT, m_AsciiInputControl);
	DDX_Text(pDX, IDC_ENTER_ASCII_INPUT, m_AsciiInput);
	DDX_Text(pDX, IDC_ENTER_ASCII_SHOW, m_AsciiOutput);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGetAscii, CDialog)
	//{{AFX_MSG_MAP(CGetAscii)
	ON_EN_CHANGE(IDC_ENTER_ASCII_INPUT, OnChangeEnterAsciiInput)
	ON_NOTIFY(EN_SELCHANGE, IDC_ENTER_ASCII_SHOW, OnSelchangeEnterAsciiShow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetAscii message handlers
BOOL CGetAscii::OnInitDialog() 
{
	CDialog::OnInitDialog();
// |ENM_KEYEVENTS|ENM_MOUSEEVENTS
	m_AsciiOuputControl.SetEventMask(ENM_NONE|ENM_SELCHANGE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#define __AFXCONV_H__
#include "afxpriv.h"

int CGetAscii::DoModal() 
{
	int res;
	unCH wDirPathName[FNSize];

	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = AfxGetResourceHandle();
	if (m_lpszTemplateName != NULL)
	{
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	HGLOBAL hTemplate = NULL;
	if (global_df) {
		CDialogTemplate dlgTemp(lpDialogTemplate);
		dlgTemp.SetFont(u_strcpy(wDirPathName,global_df->row_txt->Font.FName,FNSize), 8);
		hTemplate = dlgTemp.Detach();
		if (hTemplate != NULL)
			lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);
	}
	LPCTSTR TemplateName = m_lpszTemplateName;
	HGLOBAL DialogTemplate = m_hDialogTemplate;
	m_lpszTemplateName = NULL;
	m_hDialogTemplate  = NULL;
	m_lpDialogTemplate = lpDialogTemplate;

	res = CDialog::DoModal();

	if (hTemplate != NULL)
	{
		GlobalUnlock(hTemplate);
		GlobalFree(hTemplate);
	}
	// unlock/free resources as necessary
	if (TemplateName != NULL || DialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (TemplateName != NULL)
		FreeResource(hDialogTemplate);

	return res;
}

void CGetAscii::OnSelchangeEnterAsciiShow(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);
	unCH st[SPEAKERLEN];
	int   fline, len;
	long  ln, ln2, s, e;

	m_AsciiOuputControl.GetSel(s, e);
	if (e != s) {
		len = m_AsciiOuputControl.LineLength(s);
		ln = m_AsciiOuputControl.LineFromChar(s);
		ln2 = m_AsciiOuputControl.LineFromChar(e);
		if (ln == ln2 || (ln == ln2-1 && len == e-s-2)) {
			ln = m_AsciiOuputControl.LineFromChar(-1);
			ln = m_AsciiOuputControl.GetLine(ln, st, SPEAKERLEN);
			st[ln] = EOS;
			for (ln=0; st[ln] != ':' && st[ln] != EOS; ln++) ;
			st[ln] = EOS;
			fline = m_AsciiOuputControl.GetFirstVisibleLine();
			m_AsciiInput = st;
			UpdateData(FALSE);
			m_AsciiOuputControl.SetSel(0, 0);
			m_AsciiOuputControl.LineScroll(fline, 0);
			m_AsciiOuputControl.SetSel(s, s);
			EndDialog(IDOK);
		}
	}
	*pResult = 0;
}

void CGetAscii::OnChangeEnterAsciiInput() 
{
	long i, len;
	unCH st[SPEAKERLEN];

	UpdateData(TRUE);
	strncpy(st, m_AsciiInput, SPEAKERLEN-1);
	st[SPEAKERLEN-1] = EOS;
	len = strlen(st) - 1;
	if (st[len] == '?') {
		st[len] = EOS;
		m_AsciiInput = st;
		m_AsciiOutput = cl_T("");
		for (i=0; i < 256; i++) {
			wsprintf(st, cl_T("%3d: %c; "), i, (char)i);
			strcat(st, "\r\n");
			m_AsciiOutput = m_AsciiOutput + st;
		}
		UpdateData(FALSE);
		m_AsciiInputControl.SetSel(len, len, FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////
// CNextTier dialog
CNextTier::CNextTier(CWnd* pParent /*=NULL*/)
	: CDialog(CNextTier::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNextTier)
	m_NextTier = _T("");
	//}}AFX_DATA_INIT
}

void CNextTier::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNextTier)
	DDX_Text(pDX, IDC_NEXT_TIER, m_NextTier);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNextTier, CDialog)
	//{{AFX_MSG_MAP(CNextTier)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CNextTier message handlers
/////////////////////////////////////////////////////////////////////////////
// CSelectTiers dialog
CSelectTiers::CSelectTiers(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectTiers::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectTiers)
	m_ExcludeTiers = FALSE;
	m_IncludeTiers = FALSE;
	m_ResetTiers = FALSE;
	m_Tiers = _T("");
	//}}AFX_DATA_INIT
}


void CSelectTiers::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectTiers)
	DDX_Check(pDX, IDC_TIER_EXCLUDE, m_ExcludeTiers);
	DDX_Check(pDX, IDC_TIER_INCLUDE, m_IncludeTiers);
	DDX_Check(pDX, IDC_TIER_RESET, m_ResetTiers);
	DDX_Text(pDX, IDC_TIER_LIST, m_Tiers);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectTiers, CDialog)
	//{{AFX_MSG_MAP(CSelectTiers)
	ON_BN_CLICKED(IDC_TIER_EXCLUDE, OnTierExclude)
	ON_BN_CLICKED(IDC_TIER_INCLUDE, OnTierInclude)
	ON_BN_CLICKED(IDC_TIER_RESET, OnTierReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectTiers message handlers

void CSelectTiers::OnTierExclude() 
{
	m_ResetTiers = FALSE;
	m_IncludeTiers = FALSE;
	m_ExcludeTiers = TRUE;
	UpdateData(FALSE);
}

void CSelectTiers::OnTierInclude() 
{
	m_ResetTiers = FALSE;
	m_IncludeTiers = TRUE;
	m_ExcludeTiers = FALSE;
	UpdateData(FALSE);
}

void CSelectTiers::OnTierReset() 
{
	m_ResetTiers = TRUE;
	m_IncludeTiers = FALSE;
	m_ExcludeTiers = FALSE;
	UpdateData(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// CStdIn dialog
extern struct DefWin defWinSize;

CStdIn::CStdIn(CWnd* pParent /*=NULL*/)
	: CDialog(CStdIn::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStdIn)
	m_StdInput = _T("");
	//}}AFX_DATA_INIT
}


void CStdIn::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStdIn)
	DDX_Text(pDX, IDC_STDIN_INPUT, m_StdInput);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStdIn, CDialog)
	//{{AFX_MSG_MAP(CStdIn)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStdIn message handlers
BOOL CStdIn::OnInitDialog() {
	CRect lpRect;

	CDialog::OnInitDialog();

	this->GetWindowRect(&lpRect);
	if (defWinSize.top || defWinSize.width || defWinSize.height || defWinSize.left) {
		lpRect.top = defWinSize.top;
		lpRect.bottom = defWinSize.height;
		lpRect.left = defWinSize.left;
		lpRect.right = defWinSize.width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}
	return TRUE;
}

BOOL CStdIn::PreTranslateMessage(MSG* pMsg) 
{
	UpdateData(true);
/*
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1 &&
		_AfxCompareClassName(pMsg->hwnd, _T("Edit"))) {
		return TRUE;
	} else if (pMsg->message == WM_CHAR && pMsg->wParam == 0x17 && // ^W
		_AfxCompareClassName(pMsg->hwnd, _T("Edit"))) {
		return TRUE;
	} else if (pMsg->message == WM_KEYDOWN &&
		_AfxCompareClassName(pMsg->hwnd, _T("Edit"))) {
		return TRUE;
	} else if (pMsg->message == WM_CHAR &&
		_AfxCompareClassName(pMsg->hwnd, _T("Edit"))) {
		return TRUE;
	}
*/
/*
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
*/
	return CDialog::PreTranslateMessage(pMsg);
}

void CStdIn::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	UpdateData(true);
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CStdIn::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	UpdateData(true);
	CDialog::OnChar(nChar, nRepCnt, nFlags);
}

void CStdIn::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	UpdateData(true);
	CDialog::OnClose();
	if (StdInErrMessage != NULL) {
		isKillProgram = 2;
	}
}

void CStdIn::OnCancel() {
	CRect lpRect;

	UpdateData(true);
	this->GetWindowRect(&lpRect);
	defWinSize.top = lpRect.top;
	defWinSize.left = lpRect.left;
	defWinSize.width = lpRect.right;
	defWinSize.height = lpRect.bottom;
	WriteCedPreference();
	EndDialog(IDCANCEL);
}

void CStdIn::OnOK() {
	CRect lpRect;

	UpdateData(true);
	this->GetWindowRect(&lpRect);
	defWinSize.top = lpRect.top;
	defWinSize.left = lpRect.left;
	defWinSize.width = lpRect.right;
	defWinSize.height = lpRect.bottom;
	WriteCedPreference();
	EndDialog(IDOK);
}

/////////////////////////////////////////////////////////////////////////////
// CDoString dialog


CDoString::CDoString(CWnd* pParent /*=NULL*/)
	: CDialog(CDoString::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDoString)
	m_StringVal = _T("");
	//}}AFX_DATA_INIT
}


void CDoString::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDoString)
	DDX_Text(pDX, IDC_EDIT_DO_STRING_VAL, m_StringVal);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDoString, CDialog)
	//{{AFX_MSG_MAP(CDoString)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDoString message handlers

/////////////////////////////////////////////////////////////////////////////
// CSetTab dialog


CSetTab::CSetTab(CWnd* pParent /*=NULL*/)
	: CDialog(CSetTab::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetTab)
	m_SetTab = 0;
	//}}AFX_DATA_INIT
}


void CSetTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetTab)
	DDX_Text(pDX, IDC_EDIT_GOTO_LINENUMBER, m_SetTab);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetTab, CDialog)
	//{{AFX_MSG_MAP(CSetTab)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetTab message handlers
/////////////////////////////////////////////////////////////////////////////
// CURLAddress dialog


CURLAddress::CURLAddress(CWnd* pParent /*=NULL*/)
	: CDialog(CURLAddress::IDD, pParent)
{
	//{{AFX_DATA_INIT(CURLAddress)
	m_Address = _T("");
	m_ProxyAddress = _T("");
	m_ProxyPort = _T("");
	//}}AFX_DATA_INIT
}


void CURLAddress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CURLAddress)
	DDX_Text(pDX, IDC_EDIT_URL_ADD_NAME, m_Address);
	DDX_Text(pDX, IDC_EDIT_PROXY_ADDRESS, m_ProxyAddress);
	DDX_Text(pDX, IDC_EDIT_PROXY_PORT, m_ProxyPort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CURLAddress, CDialog)
	//{{AFX_MSG_MAP(CURLAddress)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CURLAddress message handlers

/////////////////////////////////////////////////////////////////////////////
// CThumbnails dialog


CThumbnails::CThumbnails(CWnd* pParent /*=NULL*/)
	: CDialog(CThumbnails::IDD, pParent)
{
	//{{AFX_DATA_INIT(CThumbnails)
	m_Size = _T("");
	//}}AFX_DATA_INIT
}


void CThumbnails::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThumbnails)
	DDX_Text(pDX, IDC_EDIT_THUMBNAIL_SIZE, m_Size);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CThumbnails, CDialog)
	//{{AFX_MSG_MAP(CThumbnails)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThumbnails message handlers

/////////////////////////////////////////////////////////////////////////////
// CGetFilename dialog


CGetFilename::CGetFilename(CWnd* pParent /*=NULL*/)
	: CDialog(CGetFilename::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetFilename)
	m_FileName = _T("");
	//}}AFX_DATA_INIT
}


void CGetFilename::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetFilename)
	DDX_Text(pDX, IDC_GET_FILENAME, m_FileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetFilename, CDialog)
	//{{AFX_MSG_MAP(CGetFilename)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetFilename message handlers
/////////////////////////////////////////////////////////////////////////////
// CEnterComments dialog


CEnterComments::CEnterComments(CWnd* pParent /*=NULL*/)
	: CDialog(CEnterComments::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnterComments)
	m_Comments = _T("");
	//}}AFX_DATA_INIT
}


void CEnterComments::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnterComments)
	DDX_Text(pDX, IDC_COMMENTS, m_Comments);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnterComments, CDialog)
	//{{AFX_MSG_MAP(CEnterComments)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnterComments message handlers
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CLineNumSize dialog


CLineNumSize::CLineNumSize(CWnd* pParent /*=NULL*/)
	: CDialog(CLineNumSize::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLineNumSize)
	m_LineNumSize = 0;
	m_NumberEveryLine = FALSE;
	m_NumberMainTiersOnly = FALSE;
	//}}AFX_DATA_INIT
}


void CLineNumSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLineNumSize)
	DDX_Text(pDX, IDC_EDIT_LINENUM_SIZE, m_LineNumSize);
	DDX_Check(pDX, IDC_EDIT_LINENUM_EVERYLINE, m_NumberEveryLine);
	DDX_Check(pDX, IDC_EDIT_LINENUM_MAINTIER, m_NumberMainTiersOnly);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLineNumSize, CDialog)
	//{{AFX_MSG_MAP(CLineNumSize)
	ON_BN_CLICKED(IDC_EDIT_LINENUM_EVERYLINE, OnEditLinenumEveryline)
	ON_BN_CLICKED(IDC_EDIT_LINENUM_MAINTIER, OnEditLinenumMaintier)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLineNumSize message handlers

void CLineNumSize::OnEditLinenumEveryline() 
{
    m_NumberEveryLine = TRUE;
    m_NumberMainTiersOnly = FALSE;
	UpdateData(FALSE);
}

void CLineNumSize::OnEditLinenumMaintier() 
{
    m_NumberEveryLine = FALSE;
    m_NumberMainTiersOnly = TRUE;
	UpdateData(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// CSelectSoundAnalyzer dialog


CSelectSoundAnalyzer::CSelectSoundAnalyzer(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectSoundAnalyzer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectSoundAnalyzer)
	m_PitchWorks = FALSE;
	m_Praat = FALSE;
	//}}AFX_DATA_INIT
}


void CSelectSoundAnalyzer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectSoundAnalyzer)
	DDX_Check(pDX, IDC_PITCHWORKS, m_PitchWorks);
	DDX_Check(pDX, IDC_PRAAT, m_Praat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectSoundAnalyzer, CDialog)
	//{{AFX_MSG_MAP(CSelectSoundAnalyzer)
	ON_BN_CLICKED(IDC_PITCHWORKS, OnPitchworks)
	ON_BN_CLICKED(IDC_PRAAT, OnPraat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectSoundAnalyzer message handlers

void CSelectSoundAnalyzer::OnPitchworks() 
{
	m_PitchWorks = TRUE;
	m_Praat = FALSE;
	UpdateData(FALSE);	
}

void CSelectSoundAnalyzer::OnPraat() 
{
	m_PitchWorks = FALSE;
	m_Praat = TRUE;
	UpdateData(FALSE);	
}
/////////////////////////////////////////////////////////////////////////////
// CF5Options dialog


CF5Options::CF5Options(CWnd* pParent /*=NULL*/)
	: CDialog(CF5Options::IDD, pParent)
{
	//{{AFX_DATA_INIT(CF5Options)
	m_EveryTier = FALSE;
	m_EveryLine = FALSE;
	m_F5_Offset = 0;
	//}}AFX_DATA_INIT
}


void CF5Options::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CF5Options)
	DDX_Check(pDX, IDC_BET, m_EveryTier);
	DDX_Check(pDX, IDC_BEL, m_EveryLine);
	DDX_Text(pDX, IDC_F5_Offset, m_F5_Offset);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CF5Options, CDialog)
	//{{AFX_MSG_MAP(CF5Options)
	ON_BN_CLICKED(IDC_BEL, OnBel)
	ON_BN_CLICKED(IDC_BET, OnBet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CF5Options message handlers

void CF5Options::OnBel() 
{
	m_EveryLine = TRUE;
	m_EveryTier = FALSE;
	UpdateData(FALSE);	
}

void CF5Options::OnBet() 
{
	m_EveryLine = FALSE;
	m_EveryTier = TRUE;
	UpdateData(FALSE);	
}
/////////////////////////////////////////////////////////////////////////////
// CGetWebPasswd dialog


CGetWebPasswd::CGetWebPasswd(CWnd* pParent /*=NULL*/)
	: CDialog(CGetWebPasswd::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetWebPasswd)
	m_Passwd = _T("");
	m_Username = _T("");
	//}}AFX_DATA_INIT
}


void CGetWebPasswd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetWebPasswd)
	DDX_Text(pDX, IDC_WEB_PASSWD, m_Passwd);
	DDX_Text(pDX, IDC_WEB_USERNAME, m_Username);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetWebPasswd, CDialog)
	//{{AFX_MSG_MAP(CGetWebPasswd)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetWebPasswd message handlers


/////////////////////////////////////////////////////////////////////////////
// CKeywordColor dialog
CKeywordColor::CKeywordColor(CWnd* pParent /*=NULL*/)
	: CDialog(CKeywordColor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeywordColor)
	m_EditWord = _T("");
	m_ApplyColor = FALSE;
	m_Case = FALSE;
	m_ColorEntire = FALSE;
	m_MatchEntire = FALSE;
	m_TreatAs = FALSE;
	//}}AFX_DATA_INIT
}


void CKeywordColor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeywordColor)
	DDX_Control(pDX, IDC_COLOR_SWATCH, m_ColorSwatchControl);
	DDX_Control(pDX, IDC_CONST_LIST, m_KeyWordColorListControl);
	DDX_Text(pDX, IDC_EDIT_WORD, m_EditWord);
	DDX_Check(pDX, IDC_APPLY_COLOR, m_ApplyColor);
	DDX_Check(pDX, IDC_CASE, m_Case);
	DDX_Check(pDX, IDC_COLOR_ENTIRE, m_ColorEntire);
	DDX_Check(pDX, IDC_MATCH_ENTIRE, m_MatchEntire);
	DDX_Check(pDX, IDC_TREAT_AS, m_TreatAs);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeywordColor, CDialog)
	//{{AFX_MSG_MAP(CKeywordColor)
	ON_LBN_SELCHANGE(IDC_CONST_LIST, OnSelchangeConstList)
	ON_LBN_DBLCLK(IDC_CONST_LIST, OnDblclkConstList)
	ON_BN_CLICKED(IDC_DEL_ALL, OnDelAll)
	ON_BN_CLICKED(IDC_DEL_ONE, OnDelOne)
	ON_BN_CLICKED(IDC_EDIT_COLOR, OnEditColor)
	ON_BN_CLICKED(IDC_CASE, OnCase)
	ON_BN_CLICKED(IDC_COLOR_ENTIRE, OnColorEntire)
	ON_BN_CLICKED(IDC_MATCH_ENTIRE, OnMatchEntire)
	ON_BN_CLICKED(IDC_TREAT_AS, OnTreatAs)
	ON_BN_CLICKED(IDC_APPLY_COLOR, OnApplyColor)
	ON_BN_CLICKED(IDC_COLOR_SWATCH, OnColorSwatch)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeywordColor message handlers
BOOL CKeywordColor::OnInitDialog() {
	int  wordLen;
	char cWordFlag;
	unCH text[256];
	long sc, ec;
	COLORTEXTLIST *nt;
	RGBColor theColor;

	CDialog::OnInitDialog();

	if (global_df->row_win2 || global_df->col_win2 != -2L) {
		ChangeCurLineAlways(0);
		if (global_df->row_win2 == 0) {
			if (global_df->col_win > global_df->col_win2) {
				sc = global_df->col_chr2;
				ec = global_df->col_chr;
			} else {
				sc = global_df->col_chr;
				ec = global_df->col_chr2;
			}
			for (wordLen=0; sc < ec && wordLen < 255; sc++, wordLen++)
				text[wordLen] = global_df->row_txt->line[sc];
			text[wordLen] = EOS;
			m_EditWord = text;
		} else {
			m_EditWord = _T("");
		}
	} else
		m_EditWord = _T("");

	if (global_df->RootColorText != NULL) {
		for (nt=global_df->RootColorText; nt != NULL; nt=nt->nextCT)
			m_KeyWordColorListControl.AddString(nt->keyWord);

		m_KeyWordColorListControl.SetCurSel(0);
		nt = doKeywordColor(0, &theColor, FALSE);
		cWordFlag = nt->cWordFlag;
	} else {
		theColor.red   = 0;
		theColor.green = 0;
		theColor.blue  = 0;
		cWordFlag = 0;
	}

	m_Case = is_case_word(cWordFlag);
	m_ColorEntire = is_all_line(cWordFlag);
	m_MatchEntire = is_match_word(cWordFlag);
	m_TreatAs = is_wild_card(cWordFlag);
	UpdateData(FALSE);
	return TRUE;
}

COLORTEXTLIST *CKeywordColor::doKeywordColor(int num, RGBColor *theColor, char isSetColor) {	
	COLORTEXTLIST *t;

	for (t=global_df->RootColorText; t != NULL; t=t->nextCT, num--) {
		if (num == 0) {
			if (theColor != NULL) {
				if (isSetColor) {
					t->red = theColor->red;
					t->green = theColor->green;
					t->blue = theColor->blue;
				} else {
					theColor->red   = t->red;
					theColor->green = t->green;
					theColor->blue  = t->blue;
				}
			}
			return(t);
		}
	}
	return(global_df->RootColorText);
}

void CKeywordColor::FreeOneColorKeyword(int num) {
	COLORTEXTLIST *t, *tt;

	if (num == 0) {
		t = global_df->RootColorText;
		global_df->RootColorText = global_df->RootColorText->nextCT;
		free(t->keyWord);
		free(t->fWord);
		free(t);
	} else {
		tt = global_df->RootColorText;
		t  = global_df->RootColorText->nextCT;
		for (num--; t != NULL; num--) {
			if (num == 0) {
				tt->nextCT = t->nextCT;
				free(t->keyWord);
				free(t->fWord);
				free(t);
				break;
			}
			tt = t;
			t = t->nextCT;
		}	
	}
}

void CKeywordColor::OnSelchangeConstList() 
{
	int i;
	char cWordFlag;
	COLORTEXTLIST *nt;

	UpdateData(TRUE);
	i = m_KeyWordColorListControl.GetCurSel();
	if (i != LB_ERR) {
		nt = doKeywordColor(i, NULL, FALSE);
		cWordFlag = nt->cWordFlag;
		m_Case = is_case_word(cWordFlag);
		m_ColorEntire = is_all_line(cWordFlag);
		m_MatchEntire = is_match_word(cWordFlag);
		m_TreatAs = is_wild_card(cWordFlag);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT ); 
	}
	UpdateData(FALSE);
}

void CKeywordColor::OnDblclkConstList() 
{
	OnEditColor();	
}

void CKeywordColor::OnDelAll() 
{
	char cWordFlag;

	UpdateData(TRUE);
	FreeColorText(global_df->RootColorText);
	global_df->RootColorText = NULL;
	ColorChanged = TRUE;
	m_KeyWordColorListControl.ResetContent();
	cWordFlag = 0;
	m_Case = is_case_word(cWordFlag);
	m_ColorEntire = is_all_line(cWordFlag);
	m_MatchEntire = is_match_word(cWordFlag);
	m_TreatAs = is_wild_card(cWordFlag);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT ); 
	UpdateData(FALSE);
}

void CKeywordColor::OnDelOne() 
{
	int i, cnt;
	char cWordFlag;
	COLORTEXTLIST *nt;

	UpdateData(TRUE);
	i = m_KeyWordColorListControl.GetCurSel();
	if (i != LB_ERR) {
		ColorChanged = TRUE;
		FreeOneColorKeyword(i);
		m_KeyWordColorListControl.DeleteString(i);
		cnt = m_KeyWordColorListControl.GetCount();
		if (cnt <= i)
			i = cnt - 1;
		if (i >= 0) {
			m_KeyWordColorListControl.SetCurSel(i);
			nt = doKeywordColor(i, NULL, FALSE);
			cWordFlag = nt->cWordFlag;
		} else {
			cWordFlag = 0;
		}
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT ); 
	}
	UpdateData(FALSE);
}

void CKeywordColor::OnEditColor() 
{
	DWORD dwColor;
	static DWORD dwCustClrs [16];
	static BOOL areCustomInit = FALSE;
	BOOL fSetColor = FALSE;
	CHOOSECOLOR chsclr;
	COLORREF crColor;
	long tc;
	int i;
	RGBColor theColor;

	UpdateData(TRUE);
	if (!areCustomInit) {
		for (i = 0; i < 15; i++) {
			dwCustClrs [i] = RGB (255, 255, 255);
			areCustomInit = TRUE;
		}
	}
	i = m_KeyWordColorListControl.GetCurSel();
	if (i != LB_ERR) {
		doKeywordColor(i, &theColor, FALSE);

		tc				= theColor.red;
		tc				= tc * 100 / 0xffff;
		theColor.red	= 0xff * tc / 100;
		tc				= theColor.green;
		tc				= tc * 100 / 0xffff;
		theColor.green	= 0xff * tc / 100;
		tc				= theColor.blue;
		tc				= tc * 100 / 0xffff;
		theColor.blue	= 0xff * tc / 100;

		dwColor = RGB(theColor.red, theColor.green, theColor.blue);
	} else
		return;

	chsclr.lStructSize = sizeof (CHOOSECOLOR);
	chsclr.hwndOwner = m_hWnd;
	chsclr.hInstance = (HWND)NULL;
	chsclr.rgbResult = dwColor;
	chsclr.lpCustColors = (LPDWORD)dwCustClrs;
	chsclr.lCustData = 0L;
	chsclr.Flags = CC_FULLOPEN | CC_RGBINIT;
	chsclr.lpfnHook = (LPCCHOOKPROC)(FARPROC)NULL;
	chsclr.lpTemplateName = (LPTSTR)NULL;
	if (fSetColor = ChooseColor (&chsclr)) {
		crColor = chsclr.rgbResult;

/*
		tc				= t->red;
		tc				= tc * 100 / 0xffff;
		theColor->red   = 0xff * tc / 100;
*/

		tc				= (BYTE)(crColor & 0xff);
		tc				= tc * 100 / 0xff;
		theColor.red	= tc * 0xffff / 100;

		tc				= (BYTE)((crColor >> 8) & 0xff);
		tc				= tc * 100 / 0xff;
		theColor.green	= tc * 0xffff / 100;

		tc				= (BYTE)((crColor >> 16) & 0xff);
		tc				= tc * 100 / 0xff;
		theColor.blue	= tc * 0xffff / 100;

//		theColor.red   = (BYTE)(crColor & 0xff);
//		theColor.green = (BYTE)((crColor >> 8) & 0xff);
//		theColor.blue  = (BYTE)((crColor >> 16) & 0xff);
		doKeywordColor(i, &theColor, TRUE);
		ColorChanged = TRUE;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT ); 

	}	
	UpdateData(FALSE);
}

void CKeywordColor::OnOK() 
{
	int  last_cell, wordLen;
	char cWordFlag;
	COLORTEXTLIST *nt, *tnt;

	UpdateData(TRUE);
	strcpy(templine, m_EditWord);	
	wordLen = strlen(templine);
	if (wordLen > 0) {
		if (global_df->RootColorText == NULL) {
			global_df->RootColorText = NEW(COLORTEXTLIST);
			if (global_df->RootColorText == NULL)
				mem_err(TRUE, global_df);
			nt = global_df->RootColorText;
			nt->nextCT = NULL;
			last_cell = 0;
		} else {
			last_cell = 0;
			tnt= global_df->RootColorText;
			nt = global_df->RootColorText;
			strcpy(templine2, templine);
			uS.uppercasestr(templine2, &dFnt, C_MBF);
			while (1) {
				if (nt == NULL) {
					tnt->nextCT = NEW(COLORTEXTLIST);
					if (tnt->nextCT == NULL)
						mem_err(TRUE, global_df);
					nt = tnt->nextCT;
					nt->nextCT = NULL;
					break;
				} else {
					strcpy(templine3, nt->keyWord);
					uS.uppercasestr(templine3, &dFnt, C_MBF);
					if (strcmp(nt->keyWord, templine) == 0) {
						nt = NULL;
						break;
					} else if (strcmp(templine3, templine2) > 0) {
						if (nt == global_df->RootColorText) {
							global_df->RootColorText = NEW(COLORTEXTLIST);
							if (global_df->RootColorText == NULL)
								mem_err(TRUE, global_df);
							global_df->RootColorText->nextCT = nt;
							nt = global_df->RootColorText;
						} else {
							nt = NEW(COLORTEXTLIST);
							if (nt == NULL)
								mem_err(TRUE, global_df);
							nt->nextCT = tnt->nextCT;
							tnt->nextCT = nt;
						}
						break;
					}
				}
				tnt = nt;
				nt = nt->nextCT;
				last_cell++;
			}
		}
		if (nt != NULL) {
			nt->keyWord = (unCH *)malloc((wordLen+1)*sizeof(unCH));
			if (nt->keyWord == NULL)
				mem_err(TRUE, global_df);
			strcpy(nt->keyWord, templine);
			nt->fWord = (unCH *)malloc((wordLen+1)*sizeof(unCH));
			if (nt->fWord == NULL)
				mem_err(TRUE, global_df);
			strcpy(nt->fWord, templine);
			uS.uppercasestr(nt->fWord, &dFnt, C_MBF);
			nt->len = wordLen;
			nt->cWordFlag = 0;
			nt->red = 0;
			nt->green = 0;
			nt->blue = 0;
			cWordFlag = nt->cWordFlag;
			ColorChanged = TRUE;

			m_KeyWordColorListControl.ResetContent();
			
			for (nt=global_df->RootColorText; nt != NULL; nt=nt->nextCT)
				m_KeyWordColorListControl.AddString(nt->keyWord);
			m_KeyWordColorListControl.SetCurSel(last_cell);
			m_Case = is_case_word(cWordFlag);
			m_ColorEntire = is_all_line(cWordFlag);
			m_MatchEntire = is_match_word(cWordFlag);
			m_TreatAs = is_wild_card(cWordFlag);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT ); 
		}
	}
	m_EditWord = _T("");
	UpdateData(FALSE);	
	//	CDialog::OnOK();
}

void CKeywordColor::OnCase() 
{
	int i;
	COLORTEXTLIST *nt;

	UpdateData(TRUE);
	i = m_KeyWordColorListControl.GetCurSel();
	if (i != LB_ERR)
		nt = doKeywordColor(i, NULL, FALSE);
	else
		nt = NULL;
	if (nt != NULL) {
		if (m_Case) {
			nt->cWordFlag = set_case_word_to_1(nt->cWordFlag);
			strcpy(nt->fWord, nt->keyWord);
		} else {
			nt->cWordFlag = set_case_word_to_0(nt->cWordFlag);
			uS.uppercasestr(nt->fWord, &dFnt, C_MBF);
		}
		ColorChanged = TRUE;
	}
}

void CKeywordColor::OnColorEntire() 
{
	int i;
	COLORTEXTLIST *nt;

	UpdateData(TRUE);
	i = m_KeyWordColorListControl.GetCurSel();
	if (i != LB_ERR)
		nt = doKeywordColor(i, NULL, FALSE);
	else
		nt = NULL;
	if (nt != NULL) {
		if (m_ColorEntire)
			nt->cWordFlag = set_all_line_to_1(nt->cWordFlag);
		else
			nt->cWordFlag = set_all_line_to_0(nt->cWordFlag);
		ColorChanged = TRUE;
	}
}

void CKeywordColor::OnMatchEntire() 
{
	int i;
	COLORTEXTLIST *nt;

	UpdateData(TRUE);
	i = m_KeyWordColorListControl.GetCurSel();
	if (i != LB_ERR)
		nt = doKeywordColor(i, NULL, FALSE);
	else
		nt = NULL;
	if (nt != NULL) {
		if (m_MatchEntire)
			nt->cWordFlag = set_match_word_to_1(nt->cWordFlag);
		else
			nt->cWordFlag = set_match_word_to_0(nt->cWordFlag);
		ColorChanged = TRUE;
	}
}

void CKeywordColor::OnTreatAs() 
{
	int i;
	COLORTEXTLIST *nt;

	UpdateData(TRUE);
	i = m_KeyWordColorListControl.GetCurSel();
	if (i != LB_ERR)
		nt = doKeywordColor(i, NULL, FALSE);
	else
		nt = NULL;
	if (nt != NULL) {
		if (m_TreatAs)
			nt->cWordFlag = set_wild_card_to_1(nt->cWordFlag);
		else
			nt->cWordFlag = set_wild_card_to_0(nt->cWordFlag);
		ColorChanged = TRUE;
	}
}

void CKeywordColor::OnApplyColor() 
{
	UpdateData(TRUE);
	ColorChanged = TRUE;
}

void CKeywordColor::OnColorSwatch() 
{
	OnEditColor();	
}

void CKeywordColor::OnPaint() 
{
//	CPaintDC dc(this); // device context for painting
	int  i;
	long tc;
	RECT cRect;
	CWnd *pw;
	CDC* cDC;
	RGBColor theColor;

	CDialog::OnPaint();

	i = m_KeyWordColorListControl.GetCurSel();
	if (i != LB_ERR) {
		doKeywordColor(i, &theColor, FALSE);
	} else {
		theColor.red   = 0;
		theColor.green = 0;
		theColor.blue  = 0;
	}

	tc				= theColor.red;
	tc				= tc * 100 / 0xffff;
	theColor.red	= 0xff * tc / 100;
	tc				= theColor.green;
	tc				= tc * 100 / 0xffff;
	theColor.green	= 0xff * tc / 100;
	tc				= theColor.blue;
	tc				= tc * 100 / 0xffff;
	theColor.blue	= 0xff * tc / 100;

	pw=GetDlgItem(IDC_COLOR_SWATCH);
	pw->GetClientRect(&cRect);
	cDC = pw->GetDC();
	cDC->FillSolidRect(&cRect, RGB(theColor.red,theColor.green,theColor.blue));
	pw->ReleaseDC(cDC);
}
// C:\USR\ClanWin\SRC\Win95\ceddlgs.cpp : implementation file
//
