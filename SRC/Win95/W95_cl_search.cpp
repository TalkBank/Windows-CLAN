#include "ced.h"
#include "w95_commands.h"
#include "w95_cl_search.h"
#include "cl_search.h"

enum {
	INCLUDE,
	EXCLUDE
};

enum {
	FILE_B,
	WORD_B,
	CODE_B,
	PCODE_B,
	LCODE_B,
	MOR_B
};

#define MORSTLEN 100

static char col1 = INCLUDE, col2 = WORD_B;
static BOOL sto;
static unCH st21[MORSTLEN+1], st22[MORSTLEN+1], st23[MORSTLEN+1],
			st24[MORSTLEN+1], st25[MORSTLEN+1], st26[MORSTLEN+1], st09[MORSTLEN+1];
static FNType searchFileName[FNSize];

void InitSelectedSearch(void) {
	col1 = INCLUDE;
	col2 = WORD_B;
	searchFileName[0] = EOS;
	st21[0] = EOS;
	st22[0] = EOS;
	st23[0] = EOS;
	st24[0] = EOS;
	st25[0] = EOS;
	st26[0] = EOS;
	sto = false;
}

/////////////////////////////////////////////////////////////////////////////
// CMorHelp dialog used for App About

class CMorHelp : public CDialog
{
	public:
		CMorHelp();

// Dialog Data
	//{{AFX_DATA(CMorHelp)
		enum { IDD = IDD_SEARCH_MORHELP };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMorHelp)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	protected:
	//{{AFX_MSG(CMorHelp)
	//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};

CMorHelp::CMorHelp() : CDialog(CMorHelp::IDD)
{
	//{{AFX_DATA_INIT(CMorHelp)
	//}}AFX_DATA_INIT
}

void CMorHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMorHelp)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMorHelp, CDialog)
	//{{AFX_MSG_MAP(CMorHelp)
ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClanSearch dialog


CClanSearch::CClanSearch(CWnd* pParent /*=NULL*/)
	: CDialog(CClanSearch::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClanSearch)
	m_Include = false;
	m_Exclude = false;
	m_File = false;
	m_FName = _T("");
	m_Word = false;
	m_Code = false;
	m_Postcode = false;
	m_LangCode = false;
	m_CodeItself = false;
	m_CodeData = false;
	m_WordStr = _T("");
	m_Mor = false;
	m_Mor_pos_e = _T("");
	m_Mor_stem_e = _T("");
	m_Mor_pref_e = _T("");
	m_Mor_suf_e = _T("");
	m_Mor_fus_e = _T("");
	m_Mor_tran_e = _T("");
	m_Mor_o = false;
	//}}AFX_DATA_INIT
	spItem = 0;
	numberPresets = 0;
}

void CClanSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanSearch)
	DDX_Check(pDX, IDC_SEARCH_INCLUDE, m_Include);
	DDX_Check(pDX, IDC_SEARCH_EXCLUDE, m_Exclude);
	DDX_Control(pDX, IDC_SEARCH_LIST_FNAME, m_FNameCTRL);
	DDX_Text(pDX, IDC_SEARCH_LIST_FNAME, m_FName);
	DDX_Check(pDX, IDC_SEARCH_FILE, m_File);
	DDX_Check(pDX, IDC_SEARCH_WORD, m_Word);
	DDX_Check(pDX, IDC_SEARCH_CODE, m_Code);
	DDX_Check(pDX, IDC_SEARCH_POSTCODE, m_Postcode);
	DDX_Check(pDX, IDC_SEARCH_LANGCODE, m_LangCode);
	DDX_Control(pDX, IDC_SEARCH_CODE_ITSELF, m_CodeItselfCTRL);
	DDX_Check(pDX, IDC_SEARCH_CODE_ITSELF, m_CodeItself);
	DDX_Control(pDX, IDC_SEARCH_CODE_DATA, m_CodeDataCTRL);
	DDX_Check(pDX, IDC_SEARCH_CODE_DATA, m_CodeData);
	DDX_Control(pDX, IDC_SEARCH_TEXT, m_WordStrCTRL);
	DDX_Text(pDX, IDC_SEARCH_TEXT, m_WordStr);
	DDX_Check(pDX, IDC_SEARCH_MOR, m_Mor);
	DDX_Control(pDX, IDC_SEARCH_PRESETS, m_PresetsCTRL);
	DDX_Control(pDX, IDC_SEARCH_POS_S, m_Mor_pos_sCTRL);
	DDX_Control(pDX, IDC_SEARCH_POS_E, m_Mor_pos_eCTRL);
	DDX_Text(pDX, IDC_SEARCH_POS_E, m_Mor_pos_e);
	DDX_Control(pDX, IDC_SEARCH_STEM_S, m_Mor_stem_sCTRL);
	DDX_Control(pDX, IDC_SEARCH_STEM_E, m_Mor_stem_eCTRL);
	DDX_Text(pDX, IDC_SEARCH_STEM_E, m_Mor_stem_e);
	DDX_Control(pDX, IDC_SEARCH_PREFIX_S, m_Mor_pref_sCTRL);
	DDX_Control(pDX, IDC_SEARCH_PREFIX_E, m_Mor_pref_eCTRL);
	DDX_Text(pDX, IDC_SEARCH_PREFIX_E, m_Mor_pref_e);
	DDX_Control(pDX, IDC_SEARCH_SUFFIX_S, m_Mor_suf_sCTRL);
	DDX_Control(pDX, IDC_SEARCH_SUFFIX_E, m_Mor_suf_eCTRL);
	DDX_Text(pDX, IDC_SEARCH_SUFFIX_E, m_Mor_suf_e);
	DDX_Control(pDX, IDC_SEARCH_FUSION_S, m_Mor_fus_sCTRL);
	DDX_Control(pDX, IDC_SEARCH_FUSION_E, m_Mor_fus_eCTRL);
	DDX_Text(pDX, IDC_SEARCH_FUSION_E, m_Mor_fus_e);
	DDX_Control(pDX, IDC_SEARCH_TRANS_S, m_Mor_tran_sCTRL);
	DDX_Control(pDX, IDC_SEARCH_TRANS_E, m_Mor_tran_eCTRL);
	DDX_Text(pDX, IDC_SEARCH_TRANS_E, m_Mor_tran_e);
	DDX_Control(pDX, IDC_SEARCH_MOR_O, m_Mor_oCTRL);
	DDX_Check(pDX, IDC_SEARCH_MOR_O, m_Mor_o);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClanSearch, CDialog)
	//{{AFX_MSG_MAP(CClanSearch)
	ON_BN_CLICKED(IDC_SEARCH_INCLUDE, OnInclude)
	ON_BN_CLICKED(IDC_SEARCH_EXCLUDE, OnExclude)
	ON_BN_CLICKED(IDC_SEARCH_FROM_FILE, OnFileOpen)
	ON_BN_CLICKED(IDC_SEARCH_MORHELP, OnMoreHelp)
	ON_BN_CLICKED(IDC_SEARCH_FILE, OnFile)
	ON_BN_CLICKED(IDC_SEARCH_WORD, OnWord)
	ON_BN_CLICKED(IDC_SEARCH_CODE, OnCode)
	ON_BN_CLICKED(IDC_SEARCH_POSTCODE, OnPostcode)
	ON_BN_CLICKED(IDC_SEARCH_LANGCODE, OnLangCode)
	ON_BN_CLICKED(IDC_SEARCH_CODE_ITSELF, OnCodeItself)
	ON_BN_CLICKED(IDC_SEARCH_CODE_DATA, OnCodeData)
	ON_BN_CLICKED(IDC_SEARCH_MOR, OnMor)
	ON_CBN_SELCHANGE(IDC_SEARCH_PRESETS, &CClanSearch::OnPresets)
//	ON_CBN_DROPDOWN(IDC_SEARCH_PRESETS, &CClanSearch::OnPresetsFocus)
	ON_CBN_SETFOCUS(IDC_SEARCH_PRESETS, &CClanSearch::OnPresetsFocus)
	ON_BN_CLICKED(IDC_SEARCH_MOR_O, OnMor_o)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CClanSearch::ctrlAllMor(int nCmdShow) {
	m_Mor_pos_sCTRL.ShowWindow(nCmdShow);
	m_Mor_pos_eCTRL.ShowWindow(nCmdShow);
	m_Mor_stem_sCTRL.ShowWindow(nCmdShow);
	m_Mor_stem_eCTRL.ShowWindow(nCmdShow);
	m_Mor_pref_sCTRL.ShowWindow(nCmdShow);
	m_Mor_pref_eCTRL.ShowWindow(nCmdShow);
	m_Mor_suf_sCTRL.ShowWindow(nCmdShow);
	m_Mor_suf_eCTRL.ShowWindow(nCmdShow);
	m_Mor_fus_sCTRL.ShowWindow(nCmdShow);
	m_Mor_fus_eCTRL.ShowWindow(nCmdShow);
	m_Mor_tran_sCTRL.ShowWindow(nCmdShow);
	m_Mor_tran_eCTRL.ShowWindow(nCmdShow);
	m_Mor_oCTRL.ShowWindow(nCmdShow);
}

void CClanSearch::createPopupPresetsMenu() {
	int i;

	for (i=m_PresetsCTRL.GetCount()-1; i >= 0; i--)
	   m_PresetsCTRL.DeleteString(i);
	m_PresetsCTRL.InsertString(-1, cl_T("Choose preset examples"));
	for (i=0; presets[i].label != NULL; i++)
		m_PresetsCTRL.InsertString(-1, cl_T(presets[i].label));
	numberPresets = i;
	if (spItem < 0 || spItem >= m_PresetsCTRL.GetCount())
		spItem = 0;
}

BOOL CClanSearch::OnInitDialog() {
	CDialog::OnInitDialog();
	if (m_File) {
		m_FNameCTRL.ShowWindow(SW_SHOW);
		m_CodeItselfCTRL.ShowWindow(SW_HIDE);
		m_CodeDataCTRL.ShowWindow(SW_HIDE);
		m_WordStrCTRL.ShowWindow(SW_HIDE);
		ctrlAllMor(SW_HIDE);
	} else if (m_Word) {
		m_FNameCTRL.ShowWindow(SW_HIDE);
		m_CodeItselfCTRL.ShowWindow(SW_HIDE);
		m_CodeDataCTRL.ShowWindow(SW_HIDE);
		m_WordStrCTRL.ShowWindow(SW_SHOW);
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));
		ctrlAllMor(SW_HIDE);
	} else if (m_Code) {
		m_FNameCTRL.ShowWindow(SW_HIDE);
		m_CodeItselfCTRL.ShowWindow(SW_SHOW);
		m_CodeDataCTRL.ShowWindow(SW_SHOW);
		m_WordStrCTRL.ShowWindow(SW_SHOW);
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));
		ctrlAllMor(SW_HIDE);
	} else if (m_Postcode) {
		m_FNameCTRL.ShowWindow(SW_HIDE);
		m_CodeItselfCTRL.ShowWindow(SW_SHOW);
		m_CodeDataCTRL.ShowWindow(SW_SHOW);
		m_WordStrCTRL.ShowWindow(SW_SHOW);
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));
		ctrlAllMor(SW_HIDE);
	} else if (m_LangCode) {
		m_FNameCTRL.ShowWindow(SW_HIDE);
		m_CodeItselfCTRL.ShowWindow(SW_SHOW);
		m_CodeDataCTRL.ShowWindow(SW_SHOW);
		m_WordStrCTRL.ShowWindow(SW_SHOW);
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));
		ctrlAllMor(SW_HIDE);
	} else if (m_Mor) {
		m_FNameCTRL.ShowWindow(SW_HIDE);
		m_CodeItselfCTRL.ShowWindow(SW_HIDE);
		m_CodeDataCTRL.ShowWindow(SW_HIDE);
		m_WordStrCTRL.ShowWindow(SW_SHOW);
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));
		ctrlAllMor(SW_SHOW);
	}
	spItem = 0;
	createPopupPresetsMenu();
	m_PresetsCTRL.SetCurSel(spItem);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CClanSearch message handlers

void CClanSearch::OnMoreHelp() 
{
	DWORD cPos;
	CMorHelp morDlg;

	UpdateData(true);	
	if (m_Word || m_Code || m_Postcode || m_LangCode) {
		cPos = m_WordStrCTRL.GetSel();
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));
	} else if (m_Mor) {
		cPos = m_Mor_pos_eCTRL.GetSel();
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_POS_E));
	}

	morDlg.DoModal();
	UpdateData(false);	

	if (m_Word || m_Code || m_Postcode || m_LangCode)
		m_WordStrCTRL.SetSel(cPos, false);
	else if (m_Mor)
		m_Mor_pos_eCTRL.SetSel(cPos, false);
}

void CClanSearch::OnInclude() 
{
	DWORD cPos;
	UpdateData(true);	
	if (m_Word || m_Code || m_Postcode || m_LangCode) {
		cPos = m_WordStrCTRL.GetSel();
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));
	} else if (m_Mor) {
		cPos = m_Mor_pos_eCTRL.GetSel();
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_POS_E));
	}

	m_Include = true;
	m_Exclude = false;
	UpdateData(false);	

	if (m_Word || m_Code || m_Postcode || m_LangCode)
		m_WordStrCTRL.SetSel(cPos, false);
	else if (m_Mor)
		m_Mor_pos_eCTRL.SetSel(cPos, false);
}

void CClanSearch::OnExclude() 
{
	DWORD cPos;
	UpdateData(true);	
	if (m_Word || m_Code || m_Postcode || m_LangCode) {
		cPos = m_WordStrCTRL.GetSel();
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));
	} else if (m_Mor) {
		cPos = m_Mor_pos_eCTRL.GetSel();
		GotoDlgCtrl(GetDlgItem(IDC_SEARCH_POS_E));
	}

	m_Include = false;
	m_Exclude = true;
	UpdateData(false);	

	if (m_Word || m_Code || m_Postcode || m_LangCode)
		m_WordStrCTRL.SetSel(cPos, false);
	else if (m_Mor)
		m_Mor_pos_eCTRL.SetSel(cPos, false);
}

void CClanSearch::OnFile() 
{
    unCH			*szFilter, *t;
	OPENFILENAME	ofn;
	unCH			wDirPathName[FNSize];

	m_File = true;
	m_Word = false;
	m_Code = false;
	m_Postcode = false;
	m_LangCode = false;
	m_CodeItself = false;
	m_CodeData = false;
	m_Mor = false;
	m_FNameCTRL.ShowWindow(SW_SHOW);
	m_CodeItselfCTRL.ShowWindow(SW_HIDE);
	m_CodeDataCTRL.ShowWindow(SW_HIDE);
	m_WordStrCTRL.ShowWindow(SW_HIDE);
	ctrlAllMor(SW_HIDE);
	UpdateData(false);
	if (searchFileName[0] == EOS) {
		szFilter = _T("Search File (*.cut)\0*.cut\0All files (*.*)\0*.*\0\0");
		strcpy(fname, "");
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
		ofn.lpstrFilter = szFilter;
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0L;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = fname;
		ofn.nMaxFile = sizeof(fname);
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = u_strcpy(wDirPathName, wd_dir, FNSize);
		ofn.lpstrTitle = _T("Please locate search string file");
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
		ofn.nFileOffset = 0;
		ofn.nFileExtension = 0;
		ofn.lpstrDefExt = NULL;
		if (GetOpenFileName(&ofn) != 0) {
			u_strcpy(searchFileName, fname, FNSize);
			if ((t=strrchr(fname,'\\')) != NULL)
				strcpy(fname, t+1);
			m_FName = fname;
			UpdateData(false);	
		}
	}
}

void CClanSearch::OnFileOpen() 
{
    unCH			*szFilter, *t;
	OPENFILENAME	ofn;
	unCH			wDirPathName[FNSize];

	m_File = true;
	m_Word = false;
	m_Code = false;
	m_Postcode = false;
	m_LangCode = false;
	m_CodeItself = false;
	m_CodeData = false;
	m_Mor = false;
	m_FNameCTRL.ShowWindow(SW_SHOW);
	m_CodeItselfCTRL.ShowWindow(SW_HIDE);
	m_CodeDataCTRL.ShowWindow(SW_HIDE);
	m_WordStrCTRL.ShowWindow(SW_HIDE);
	ctrlAllMor(SW_HIDE);
	UpdateData(false);	

	szFilter = _T("Search File (*.cut)\0*.cut\0All files (*.*)\0*.*\0\0");
    strcpy(fname, "");
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = fname;
    ofn.nMaxFile = sizeof(fname);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = u_strcpy(wDirPathName, wd_dir, FNSize);
    ofn.lpstrTitle = _T("Please locate search string file");
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
	if (GetOpenFileName(&ofn) != 0) {
		u_strcpy(searchFileName, fname, FNSize);
		if ((t=strrchr(fname,'\\')) != NULL)
			strcpy(fname, t+1);
		m_FName = fname;
		UpdateData(false);	
	}
}

void CClanSearch::OnWord() 
{
	DWORD cPos = m_WordStrCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));

	UpdateData(true);
	m_File = false;
	m_Word = true;
	m_Code = false;
	m_Postcode = false;
	m_LangCode = false;
	m_CodeItself = false;
	m_CodeData = false;
	m_Mor = false;
	m_FNameCTRL.ShowWindow(SW_HIDE);
	m_CodeItselfCTRL.ShowWindow(SW_HIDE);
	m_CodeDataCTRL.ShowWindow(SW_HIDE);
	m_WordStrCTRL.ShowWindow(SW_SHOW);
	ctrlAllMor(SW_HIDE);
	UpdateData(false);	

	m_WordStrCTRL.SetSel(cPos, false);
}

void CClanSearch::OnCode() 
{
	DWORD cPos = m_WordStrCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));

	UpdateData(true);
	m_File = false;
	m_Word = false;
	m_Code = true;
	m_Postcode = false;
	m_LangCode = false;
	m_CodeItself = true;
	m_CodeData = false;
	m_Mor = false;
	m_FNameCTRL.ShowWindow(SW_HIDE);
	m_CodeItselfCTRL.ShowWindow(SW_SHOW);
	m_CodeDataCTRL.ShowWindow(SW_SHOW);
	m_WordStrCTRL.ShowWindow(SW_SHOW);
	ctrlAllMor(SW_HIDE);
	UpdateData(false);	

	m_WordStrCTRL.SetSel(cPos, false);
}

void CClanSearch::OnPostcode() 
{
	DWORD cPos = m_WordStrCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));

	UpdateData(true);
	m_File = false;
	m_Word = false;
	m_Code = false;
	m_Postcode = true;
	m_LangCode = false;
	m_CodeItself = false;
	m_CodeData = true;
	m_Mor = false;
	m_FNameCTRL.ShowWindow(SW_HIDE);
	m_CodeItselfCTRL.ShowWindow(SW_SHOW);
	m_CodeDataCTRL.ShowWindow(SW_SHOW);
	m_WordStrCTRL.ShowWindow(SW_SHOW);
	ctrlAllMor(SW_HIDE);
	UpdateData(false);	

	m_WordStrCTRL.SetSel(cPos, false);
}

void CClanSearch::OnLangCode() 
{
	DWORD cPos = m_WordStrCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));

	UpdateData(true);
	m_File = false;
	m_Word = false;
	m_Code = false;
	m_Postcode = false;
	m_LangCode = true;
	m_CodeItself = false;
	m_CodeData = true;
	m_Mor = false;
	m_FNameCTRL.ShowWindow(SW_HIDE);
	m_CodeItselfCTRL.ShowWindow(SW_SHOW);
	m_CodeDataCTRL.ShowWindow(SW_SHOW);
	m_WordStrCTRL.ShowWindow(SW_SHOW);
	ctrlAllMor(SW_HIDE);
	UpdateData(false);	

	m_WordStrCTRL.SetSel(cPos, false);
}

void CClanSearch::OnCodeItself() 
{
	DWORD cPos = m_WordStrCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));

	UpdateData(true);
	m_CodeItself = true;
	m_CodeData = false;
	UpdateData(false);	

	m_WordStrCTRL.SetSel(cPos, false);
}

void CClanSearch::OnCodeData() 
{
	DWORD cPos = m_WordStrCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_SEARCH_TEXT));

	UpdateData(true);
	m_CodeItself = false;
	m_CodeData = true;
	UpdateData(false);	

	m_WordStrCTRL.SetSel(cPos, false);
}

void CClanSearch::OnMor() 
{
	DWORD cPos = m_Mor_pos_eCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_SEARCH_POS_E));

	UpdateData(true);
	m_File = false;
	m_Word = false;
	m_Code = false;
	m_Postcode = false;
	m_LangCode = false;
	m_CodeItself = false;
	m_CodeData = false;
	m_Mor = true;
	m_FNameCTRL.ShowWindow(SW_HIDE);
	m_CodeItselfCTRL.ShowWindow(SW_HIDE);
	m_CodeDataCTRL.ShowWindow(SW_HIDE);
	m_WordStrCTRL.ShowWindow(SW_HIDE);
	ctrlAllMor(SW_SHOW);
	UpdateData(false);

	m_Mor_pos_eCTRL.SetSel(cPos, false);
}

void CClanSearch::OnPresetsFocus()
{
	m_File = false;
	m_Word = false;
	m_Code = false;
	m_Postcode = false;
	m_LangCode = false;
	m_CodeItself = false;
	m_CodeData = false;
	m_Mor = true;
	m_FNameCTRL.ShowWindow(SW_HIDE);
	m_CodeItselfCTRL.ShowWindow(SW_HIDE);
	m_CodeDataCTRL.ShowWindow(SW_HIDE);
	m_WordStrCTRL.ShowWindow(SW_HIDE);
	ctrlAllMor(SW_SHOW);
	UpdateData(false);
}

void CClanSearch::OnPresets()
{
	int item;
	DWORD cPos = m_Mor_pos_eCTRL.GetSel();

	item = m_PresetsCTRL.GetCurSel();
	item--;
	if (item >= 0 && item < numberPresets) {
		m_Include = presets[item].include;
		m_Exclude = presets[item].exclude;
		m_Mor_pos_e = cl_T(presets[item].POS);
		m_Mor_stem_e = cl_T(presets[item].stem);
		m_Mor_pref_e = cl_T(presets[item].prefix);
		m_Mor_suf_e = cl_T(presets[item].suffix);
		m_Mor_fus_e = cl_T(presets[item].fusion);
		m_Mor_tran_e = cl_T(presets[item].trans);
		m_Mor_o = presets[item].oOption;
	}
	GotoDlgCtrl(GetDlgItem(IDC_SEARCH_POS_E));
	m_Mor_pos_eCTRL.SetSel(cPos, false);
	UpdateData(false);
}

void CClanSearch::OnMor_o() 
{
	UpdateData(true);
	m_Mor_o = !m_Mor_o;
	UpdateData(false);
}

static void removeBrackets(char chr, unCH *str) {
	int i;

	i = 0;
	if (str[i] == '[' || str[i] == '<') {
		i++;
		if (str[i] == chr)
			i++;
	}
	while (isSpace(str[i]))
		i++;
	if (i > 0)
		strcpy(str, str+i);
	i = strlen(str) - 1;
	if (str[i] == ']' || str[i] == '>')
		str[i] = EOS;
	uS.remblanks(str);
}

static void createWord(BOOL isInclude, char obr, char cbr, char code, unCH *toStr, unCH *fromStr) {
	int		i;
	unCH *wqt;

	if (isInclude) {
		col1 = INCLUDE;
		strcat(toStr, " +s");
	} else {
		col1 = EXCLUDE;
		strcat(toStr, " -s");
	}
	wqt = strrchr(fromStr, '"');
	if (wqt == NULL)
		strcat(toStr, "\"");
	else
		strcat(toStr, "'");
	i = strlen(toStr);
	if (obr != '\0') {
		toStr[i++] = obr;
		if (code != '\0') {
			toStr[i++] = code;
			toStr[i++] = ' ';
		}
	}
	strcpy(toStr+i, fromStr);
	i = strlen(toStr);
	if (cbr != '\0') {
		toStr[i++] = cbr;
		toStr[i] = EOS;
	}
	if (wqt == NULL)
		strcat(toStr, "\"");
	else
		strcat(toStr, "'");
}

static char createMor(char isFnd, char code, CString fSt, unCH *toSt, unCH *stn, char n) {
	int		i;
	unCH	*bg, *eg, div;

	strcpy(templineW1, fSt);
	uS.remFrontAndBackBlanks(templineW1);
	if (templineW1[0] == EOS) {
		if (n == 0)
			strcpy(stn, templineW1);
		return(isFnd);
	}
	if (n == 0) {
		strncpy(stn, templineW1, MORSTLEN);
		stn[MORSTLEN] = EOS;
		eg = strchr(templineW1, '|');
		if (eg != NULL)
			*eg = EOS;
		bg = templineW1;
	} else {
		bg = templineW1;
		for (i=0; i < n; i++) {
			bg = strchr(bg, '|');
			if (bg == NULL)
				return(isFnd);
			bg++;
		}
		eg = strchr(bg, '|');
		if (eg != NULL)
			*eg = EOS;
	}
	do {
		eg = strchr(bg, ',');
		if (eg != NULL)
			*eg = EOS;
		uS.remFrontAndBackBlanks(bg);
		if (*bg == '+') {
			div = '+';
			bg++;
		} else
			div = '-';
		if (*bg != EOS) {
			if (isFnd)
				strcat(toSt, ",");
			i = strlen(toSt);
			if (*bg == '@' && (*(bg+1) == '+' || *(bg+1) == '-')) {
			} else {
				toSt[i++] = code;
				toSt[i++] = div;
			}
			strcpy(toSt+i, bg);
			isFnd = TRUE;
		}
		if (eg != NULL)
			bg = eg + 1;
		else {
			bg = NULL;
			break;
		}
	} while (bg != NULL) ;
	return(isFnd);
}	

void SearchDialog(unCH *str) {
	int	 i;
	unCH *t;
	char isFound, isFound2, *qt;
	CClanSearch dlg;

	str[0] = EOS;
	if (col1 == EXCLUDE) {
		dlg.m_Include = false;
		dlg.m_Exclude = true;
	} else {
		dlg.m_Include = true;
		dlg.m_Exclude = false;
	}
	u_strcpy(dlg.fname, searchFileName, FNSize);
	if ((t=strrchr(dlg.fname,'\\')) != NULL)
		strcpy(dlg.fname, t+1);
	dlg.m_FName = dlg.fname;
	dlg.m_WordStr = st09;
	dlg.m_Mor_pos_e = st21;
	dlg.m_Mor_stem_e = st22;
	dlg.m_Mor_pref_e = st23;
	dlg.m_Mor_suf_e = st24;
	dlg.m_Mor_fus_e = st25;
	dlg.m_Mor_tran_e = st26;
	dlg.m_Mor_o = sto;
	if (col2 == FILE_B) {
		dlg.m_File = true;
		dlg.m_Word = false;
		dlg.m_Code = false;
		dlg.m_Postcode = false;
		dlg.m_LangCode = false;
		dlg.m_CodeItself = false;
		dlg.m_CodeData = false;
		dlg.m_Mor = false;
	} else if (col2 == MOR_B) {
		dlg.m_File = false;
		dlg.m_Word = false;
		dlg.m_Code = false;
		dlg.m_Postcode = false;
		dlg.m_LangCode = false;
		dlg.m_CodeItself = false;
		dlg.m_CodeData = false;
		dlg.m_Mor = true;
	} else if (col2 == CODE_B) {
		dlg.m_File = false;
		dlg.m_Word = false;
		dlg.m_Code = true;
		dlg.m_Postcode = false;
		dlg.m_LangCode = false;
		dlg.m_CodeItself = true;
		dlg.m_CodeData = false;
		dlg.m_Mor = false;
	} else if (col2 == PCODE_B) {
		dlg.m_File = false;
		dlg.m_Word = false;
		dlg.m_Code = false;
		dlg.m_Postcode = true;
		dlg.m_LangCode = false;
		dlg.m_CodeItself = false;
		dlg.m_CodeData = true;
		dlg.m_Mor = false;
	} else if (col2 == LCODE_B) {
		dlg.m_File = false;
		dlg.m_Word = false;
		dlg.m_Code = false;
		dlg.m_Postcode = false;
		dlg.m_LangCode = true;
		dlg.m_CodeItself = false;
		dlg.m_CodeData = true;
		dlg.m_Mor = false;
	} else {
		dlg.m_File = false;
		dlg.m_Word = true;
		dlg.m_Code = false;
		dlg.m_Postcode = false;
		dlg.m_LangCode = false;
		dlg.m_CodeItself = false;
		dlg.m_CodeData = false;
		dlg.m_Mor = false;
	}
	templineW1[0] = EOS;
	if (dlg.DoModal() == IDOK) {
		if (dlg.m_Word) {
			col2 = WORD_B;
			strcpy(templineW1, dlg.m_WordStr);
			uS.remFrontAndBackBlanks(templineW1);
			if (templineW1[0] != EOS) {
				strncpy(st09, templineW1, MORSTLEN);
				st09[MORSTLEN] = EOS;
				if (templineW1[0] == '@' && isalpha(templineW1[1])) {
					uS.shiftright(templineW1, 1);
					templineW1[0] = '*';
				}
				createWord(dlg.m_Include,'\0', '\0', '\0', str, templineW1);
				isFound = TRUE;
			}
		} else if (dlg.m_Code) {
			col2 = CODE_B;
			strcpy(templineW1, dlg.m_WordStr);
			uS.remFrontAndBackBlanks(templineW1);
			if (templineW1[0] == '@' && isalpha(templineW1[1])) {
				strncpy(st09, templineW1, MORSTLEN);
				st09[MORSTLEN] = EOS;
				uS.shiftright(templineW1, 1);
				templineW1[0] = '*';
				createWord(dlg.m_Include, '\0', '\0', '\0', str, templineW1);
				isFound = TRUE;
			} else {
				removeBrackets('\0', templineW1);
				if (templineW1[0] != EOS) {
					strncpy(st09, templineW1, MORSTLEN);
					st09[MORSTLEN] = EOS;
					if (dlg.m_CodeItself) {
						createWord(dlg.m_Include, '[', ']', '\0', str, templineW1);
						isFound = TRUE;
					}
					if (dlg.m_CodeData) {
						createWord(dlg.m_Include, '<', '>', '\0', str, templineW1);
						isFound = TRUE;
					}
				}
			}
		} else if (dlg.m_Postcode) {
			col2 = PCODE_B;
			strcpy(templineW1, dlg.m_WordStr);
			uS.remFrontAndBackBlanks(templineW1);
			removeBrackets('+', templineW1);
			if (templineW1[0] != EOS) {
				strncpy(st09, templineW1, MORSTLEN);
				st09[MORSTLEN] = EOS;
				if (dlg.m_CodeItself) {
					createWord(dlg.m_Include, '<', '>', '+', str, templineW1);
					isFound = TRUE;
				}
				if (dlg.m_CodeData) {
					createWord(dlg.m_Include, '[', ']', '+', str, templineW1);
					isFound = TRUE;
				}
			}
		} else if (dlg.m_LangCode) {
			col2 = LCODE_B;
			strcpy(templineW1, dlg.m_WordStr);
			uS.remFrontAndBackBlanks(templineW1);
			removeBrackets('-', templineW1);
			if (templineW1[0] != EOS) {
				strncpy(st09, templineW1, MORSTLEN);
				st09[MORSTLEN] = EOS;
				if (dlg.m_CodeItself) {
					createWord(dlg.m_Include, '<', '>', '-', str, templineW1);
					isFound = TRUE;
				}
				if (dlg.m_CodeData) {
					createWord(dlg.m_Include, '[', ']', '-', str, templineW1);
					isFound = TRUE;
				}
			}
		} else if (dlg.m_Mor) {
			col2 = MOR_B;
			for (i=0; i < 4; i++) {
				isFound2 = FALSE;
				if (dlg.m_Include) {
					col1 = INCLUDE;
					strcpy(templine2, " +s\"m");
				} else if (dlg.m_Exclude) {
					col1 = EXCLUDE;
					strcpy(templine2, " -s\"m");
				} else
					break;
				isFound2 = createMor(isFound2,'|',dlg.m_Mor_pos_e,templine2,st21,i);
				isFound2 = createMor(isFound2, ';', dlg.m_Mor_stem_e, templine2, st22, i);
				isFound2 = createMor(isFound2, '#', dlg.m_Mor_pref_e, templine2, st23, i);
				isFound2 = createMor(isFound2, '-', dlg.m_Mor_suf_e, templine2, st24, i);
				isFound2 = createMor(isFound2, '&', dlg.m_Mor_fus_e, templine2, st25, i);
				isFound2 = createMor(isFound2, '=', dlg.m_Mor_tran_e, templine2, st26, i);
				if (dlg.m_Mor_o)
					strcat(templine2, ",o%");
				if (strchr(templine2+1, ' ') == NULL)
					strcpy(templine2+3, templine2+4);
				else
					strcat(templine2, "\"");
				if (isFound2) {
					strcat(str, templine2);
					isFound = TRUE;
				}
			}
			if (isFound) {
				sto = dlg.m_Mor_o;
			}
		} else if (dlg.m_File) {
			col2 = FILE_B;
			if (dlg.m_Include) {
				col1 = INCLUDE;
				strcat(str, " +s");
			} else if (dlg.m_Exclude) {
				col1 = EXCLUDE;
				strcat(str, " -s");
			}
			uS.remFrontAndBackBlanks(searchFileName);
			qt = strrchr(searchFileName, '"');
			if (qt == NULL)
				strcat(str, "\"@");
			else
				strcat(str, "'@");
			strcat(str, searchFileName);
			if (qt == NULL)
				strcat(str, "\"");
			else
				strcat(str, "'");
			if (searchFileName[0] != EOS)
				isFound = TRUE;
		}
		if (!isFound)
			str[0] = EOS;
	}
}
