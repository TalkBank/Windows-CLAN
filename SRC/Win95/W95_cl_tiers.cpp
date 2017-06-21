#include "ced.h"
#include "cu.h"
#include "w95_commands.h"
#include "w95_cl_tiers.h"

#define TIERNAMELEN 128

enum {
	HEADT,
	MAINT,
	DEPT,
	ROLE,
	IDS
};
enum {
	EXCLUDEALL,
	INCLUDE1,
	EXCLUDE1
};

enum {
	CANCEL,
	FILES_PAT,
	FILES_IN
};

struct SelectedTiers {
	char type;
	wchar_t name[TIERNAMELEN];
	char what2do;
	wchar_t role[TIERNAMELEN+1];
} ;

#define DIAL_SPEAKER_FILED_LEN 30
#define DIAL_NUMBER_TIERS 31

#define CODESTRLEN  6
struct TiersListS {
	char code[CODESTRLEN+1];
	char *ID;
	int  pageN;
	int  num;
	short val;
	struct TiersListS *next_tier;
} ;

static struct SelectedTiers STier;
static wchar_t FilePattern[256+1];
static struct TiersListS *spTiersRoot;
static struct TiersListS *depTiersRoot;

extern int  F_numfiles;
extern int  cl_argc;
extern char *cl_argv[];

void InitSelectedTiers(void) {
	STier.type = MAINT;
	STier.name[0] = EOS;
	STier.what2do = INCLUDE1;
	STier.role[0] = EOS;
	strcpy(FilePattern, cl_T("*.cha"));
}

/////////////////////////////////////////////////////////////////////////////
// CClanTiers dialog


CClanTiers::CClanTiers(CWnd* pParent /*=NULL*/)
	: CDialog(CClanTiers::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClanTiers)
	m_ExcludeAllMain = false;
	m_Include = false;
	m_Exclude = false;
	m_MainTier = false;
	m_DepTier = false;
	m_HeadTier = false;
	m_RoleTier = false;
//	m_IDTier = false;
	m_TierName = _T("");
	//}}AFX_DATA_INIT
}

void CClanTiers::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanTiers)
	DDX_Check(pDX, IDC_CLAN_EXCLUDE_ALL_MAIN, m_ExcludeAllMain);
	DDX_Check(pDX, IDC_CLAN_INCLUDE, m_Include);
	DDX_Check(pDX, IDC_CLAN_EXCLUDE, m_Exclude);
	DDX_Control(pDX, IDC_CLAN_M_TIER, m_MainTierCTRL);
	DDX_Check(pDX, IDC_CLAN_M_TIER, m_MainTier);
	DDX_Control(pDX, IDC_CLAN_D_TIER, m_DepTierCTRL);
	DDX_Check(pDX, IDC_CLAN_D_TIER, m_DepTier);
	DDX_Control(pDX, IDC_CLAN_H_TIER, m_HeadTierCTRL);
	DDX_Check(pDX, IDC_CLAN_H_TIER, m_HeadTier);
	DDX_Control(pDX, IDC_CLAN_R_TIER, m_RoleTierCTRL);
	DDX_Check(pDX, IDC_CLAN_R_TIER, m_RoleTier);
//	DDX_Control(pDX, IDC_CLAN_ID_TIER, m_IDTierCTRL);
//	DDX_Check(pDX, IDC_CLAN_ID_TIER, m_IDTier);
	DDX_Control(pDX, IDC_CLAN_TIER_TEXT, m_TierNameCTRL);
	DDX_Text(pDX, IDC_CLAN_TIER_TEXT, m_TierName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClanTiers, CDialog)
	//{{AFX_MSG_MAP(CClanTiers)
	ON_BN_CLICKED(IDC_CLAN_EXCLUDE_ALL_MAIN, OnExcludeAllMain)
	ON_BN_CLICKED(IDC_CLAN_INCLUDE, OnInclude)
	ON_BN_CLICKED(IDC_CLAN_EXCLUDE, OnExclude)
	ON_BN_CLICKED(IDC_CLAN_M_TIER, OnMainTier)
	ON_BN_CLICKED(IDC_CLAN_D_TIER, OnDepTier)
	ON_BN_CLICKED(IDC_CLAN_H_TIER, OnHeadTier)
	ON_BN_CLICKED(IDC_CLAN_R_TIER, OnRoleTier)
//	ON_BN_CLICKED(IDC_CLAN_ID_TIER, OnIDTier)
	ON_BN_CLICKED(IDC_TIER_MORE_CHOICES, OnMoreChoices)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CClanTiers::OnInitDialog() {
	int len;
	CDialog::OnInitDialog();
	res = FALSE;
	len = strlen(m_TierName);
	if (m_ExcludeAllMain) {
		m_MainTierCTRL.ShowWindow(SW_HIDE);
		m_DepTierCTRL.ShowWindow(SW_HIDE);
		m_HeadTierCTRL.ShowWindow(SW_HIDE);
		m_RoleTierCTRL.ShowWindow(SW_HIDE);
//		m_IDTierCTRL.ShowWindow(SW_HIDE);
		m_TierNameCTRL.ShowWindow(SW_HIDE);
	} else if (m_Include) {
		m_MainTierCTRL.ShowWindow(SW_SHOW);
		m_DepTierCTRL.ShowWindow(SW_SHOW);
		m_HeadTierCTRL.ShowWindow(SW_SHOW);
		m_RoleTierCTRL.ShowWindow(SW_SHOW);
//		m_IDTierCTRL.ShowWindow(SW_SHOW);
		m_TierNameCTRL.ShowWindow(SW_SHOW);
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));
		m_TierNameCTRL.SetSel(len, len, false);
	} if (m_Exclude) {
		m_MainTierCTRL.ShowWindow(SW_SHOW);
		m_DepTierCTRL.ShowWindow(SW_SHOW);
		m_HeadTierCTRL.ShowWindow(SW_SHOW);
		m_RoleTierCTRL.ShowWindow(SW_SHOW);
//		m_IDTierCTRL.ShowWindow(SW_SHOW);
		m_TierNameCTRL.ShowWindow(SW_SHOW);
		GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));
		m_TierNameCTRL.SetSel(len, len, false);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CClanTiers message handlers

void CClanTiers::OnExcludeAllMain() 
{
	m_MainTierCTRL.ShowWindow(SW_HIDE);
	m_DepTierCTRL.ShowWindow(SW_HIDE);
	m_HeadTierCTRL.ShowWindow(SW_HIDE);
	m_RoleTierCTRL.ShowWindow(SW_HIDE);
//	m_IDTierCTRL.ShowWindow(SW_HIDE);
	m_TierNameCTRL.ShowWindow(SW_HIDE);
	m_ExcludeAllMain = true;
	m_Include = false;
	m_Exclude = false;

	UpdateData(false);
}

void CClanTiers::OnInclude() 
{
	BOOL tm=m_MainTier, td=m_DepTier, th=m_HeadTier, tr=m_RoleTier;//, tid=m_IDTier;
	UpdateData(true);
	DWORD cPos = m_TierNameCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));

	m_MainTierCTRL.ShowWindow(SW_SHOW);
	m_DepTierCTRL.ShowWindow(SW_SHOW);
	m_HeadTierCTRL.ShowWindow(SW_SHOW);
	m_RoleTierCTRL.ShowWindow(SW_SHOW);
//	m_IDTierCTRL.ShowWindow(SW_SHOW);
	m_TierNameCTRL.ShowWindow(SW_SHOW);
	m_ExcludeAllMain = false;
	m_Include = true;
	m_Exclude = false;
	m_MainTier = tm;
	m_DepTier = td;
	m_HeadTier = th;
	m_RoleTier = tr;
//	m_IDTier = tid;
	UpdateData(false);	

	m_TierNameCTRL.SetSel(cPos, false);
}

void CClanTiers::OnExclude() 
{
	BOOL tm=m_MainTier, td=m_DepTier, th=m_HeadTier, tr=m_RoleTier;//, tid=m_IDTier;
	UpdateData(true);	
	DWORD cPos = m_TierNameCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));

	m_MainTierCTRL.ShowWindow(SW_SHOW);
	m_DepTierCTRL.ShowWindow(SW_SHOW);
	m_HeadTierCTRL.ShowWindow(SW_SHOW);
	m_RoleTierCTRL.ShowWindow(SW_SHOW);
//	m_IDTierCTRL.ShowWindow(SW_SHOW);
	m_TierNameCTRL.ShowWindow(SW_SHOW);
	m_ExcludeAllMain = false;
	m_Include = false;
	m_Exclude = true;
	m_MainTier = tm;
	m_DepTier = td;
	m_HeadTier = th;
	m_RoleTier = tr;
//	m_IDTier = tid;
	UpdateData(false);	

	m_TierNameCTRL.SetSel(cPos, false);
}

void CClanTiers::OnMainTier() 
{
	int len;
	BOOL ti = m_Include, te = m_Exclude;
	UpdateData(true);	
//	DWORD cPos = m_TierNameCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));

	m_TierName = _T("*");
	m_MainTier = true;
	m_DepTier = false;
	m_HeadTier = false;
	m_RoleTier = false;
//	m_IDTier = false;
	m_Include = ti;
	m_Exclude = te;
	UpdateData(false);	

	len = strlen(m_TierName);
	m_TierNameCTRL.SetSel(len, len, false);
//	m_TierNameCTRL.SetSel(cPos, false);
}

void CClanTiers::OnDepTier() 
{
	int len;
	BOOL ti = m_Include, te = m_Exclude;
	UpdateData(true);	
//	DWORD cPos = m_TierNameCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));

	m_TierName = _T("%");
	m_MainTier = false;
	m_DepTier = true;
	m_HeadTier = false;
	m_RoleTier = false;
//	m_IDTier = false;
	m_Include = ti;
	m_Exclude = te;
	UpdateData(false);	

	len = strlen(m_TierName);
	m_TierNameCTRL.SetSel(len, len, false);
//	m_TierNameCTRL.SetSel(cPos, false);
}

void CClanTiers::OnHeadTier() 
{
	int len;
	BOOL ti = m_Include, te = m_Exclude;
	UpdateData(true);	
//	DWORD cPos = m_TierNameCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));

	m_TierName = _T("@");
	m_MainTier = false;
	m_DepTier = false;
	m_HeadTier = true;
	m_RoleTier = false;
//	m_IDTier = false;
	m_Include = ti;
	m_Exclude = te;
	UpdateData(false);	

	len = strlen(m_TierName);
	m_TierNameCTRL.SetSel(len, len, false);
//	m_TierNameCTRL.SetSel(cPos, false);
}

void CClanTiers::OnRoleTier() 
{
	int len;
	BOOL ti = m_Include, te = m_Exclude;
	UpdateData(true);	
//	DWORD cPos = m_TierNameCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));

	m_TierName = STier.role;
	m_MainTier = false;
	m_DepTier = false;
	m_HeadTier = false;
	m_RoleTier = true;
//	m_IDTier = false;
	m_Include = ti;
	m_Exclude = te;
	UpdateData(false);	

	len = strlen(m_TierName);
	m_TierNameCTRL.SetSel(len, len, false);
//	m_TierNameCTRL.SetSel(cPos, false);
}
/*
void CClanTiers::OnIDTier() 
{
	int len;
	BOOL ti = m_Include, te = m_Exclude;
	UpdateData(true);	
	DWORD cPos = m_TierNameCTRL.GetSel();
	GotoDlgCtrl(GetDlgItem(IDC_CLAN_TIER_TEXT));

	m_MainTier = false;
	m_DepTier = false;
	m_HeadTier = false;
	m_RoleTier = false;
	m_IDTier = true;
	m_Include = ti;
	m_Exclude = te;
	UpdateData(false);	

	len = strlen(m_TierName);
	m_TierNameCTRL.SetSel(len, len, false);
//	m_TierNameCTRL.SetSel(cPos, false);
}
*/

void CClanTiers::OnMoreChoices() 
{
	res = TRUE;
	EndDialog(IDCANCEL);
}

/////////////////////////////////////////////////////////////////////////////
// CClanTiersSelectFile dialog


CClanTiersSelectFile::CClanTiersSelectFile(CWnd* pParent /*=NULL*/)
	: CDialog(CClanTiersSelectFile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClanTiersSelectFile)
	m_FilePat = _T("");
	//}}AFX_DATA_INIT
}

void CClanTiersSelectFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanTiersSelectFile)
	DDX_Control(pDX, IDC_TIER_SELECT_PATS, m_FilePatCTRL);
	DDX_Text(pDX, IDC_TIER_SELECT_PATS, m_FilePat);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClanTiersSelectFile, CDialog)
	//{{AFX_MSG_MAP(CClanTiersSelectFile)
	ON_BN_CLICKED(IDC_TIER_SELECT_PAT, OnFilePat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CClanTiersSelectFile::OnInitDialog() {
	int len;
	CDialog::OnInitDialog();
	res = FALSE;
	len = strlen(m_FilePat);
	GotoDlgCtrl(GetDlgItem(IDC_TIER_SELECT_PATS));
	m_FilePatCTRL.SetSel(len, len, false);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CClanTiersSelectFile message handlers
void CClanTiersSelectFile::OnFilePat() 
{
	res = TRUE;
	EndDialog(IDCANCEL);
}

int SelectFilesDialog(wchar_t *additions) {
	int	res;
	CClanTiersSelectFile dlg;

	dlg.res = FALSE;
	dlg.m_FilePat = FilePattern;
	if (dlg.DoModal() == IDOK) {
		res = FILES_IN;
		strcpy(additions, cl_T(" @"));
	} else if (dlg.res == TRUE) {
		res = FILES_PAT;
		strcpy(additions, dlg.m_FilePat);
		uS.remFrontAndBackBlanks(additions);
		strncpy(FilePattern, additions, 256);
		FilePattern[256] = EOS;
	} else {
		res = CANCEL;
		additions[0] = EOS;
	}
	return(res);
}

/////////////////////////////////////////////////////////////////////////////
// CClanTiersSelect dialog


CClanTiersSelect::CClanTiersSelect(CWnd* pParent /*=NULL*/)
	: CDialog(CClanTiersSelect::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClanTiersSelect)
	m_PageNum = _T("page 1");
	m_Reading = _T("");
	//}}AFX_DATA_INIT
}

void CClanTiersSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanTiersSelect)
	DDX_Control(pDX, IDC_EXCLUDE_ALL_MAIN, m_ExcludeSpeakerDataCTRL);
	DDX_Check(pDX, IDC_EXCLUDE_ALL_MAIN, m_ExcludeSpeakerData);
	DDX_Control(pDX, IDC_INCLUDE_ALL_HEADER, m_IncludeHeadersCTRL);
	DDX_Check(pDX, IDC_INCLUDE_ALL_HEADER, m_IncludeHeaders);
	DDX_Control(pDX, IDC_SELECT_ALL_SPEAKER, m_SelectAllSpeakerCTRL);
	DDX_Check(pDX, IDC_SELECT_ALL_SPEAKER, m_SelectAllSpeaker);
	DDX_Control(pDX, IDC_SELECT_ALL_DEP, m_SelectAllDepCTRL);
	DDX_Check(pDX, IDC_SELECT_ALL_DEP, m_SelectAllDep);
	DDX_Control(pDX, IDC_SELECT_PAGE_PREV, m_SelectPrevPageCTRL);
	DDX_Control(pDX, IDC_SELECT_PAGE_NEXT, m_SelectNextPageCTRL);
	DDX_Control(pDX, IDC_SELECT_T1, m_T1_CTRL);
	DDX_Check(pDX, IDC_SELECT_T1, m_T1);
	DDX_Control(pDX, IDC_SELECT_T2, m_T2_CTRL);
	DDX_Check(pDX, IDC_SELECT_T2, m_T2);
	DDX_Control(pDX, IDC_SELECT_T3, m_T3_CTRL);
	DDX_Check(pDX, IDC_SELECT_T3, m_T3);
	DDX_Control(pDX, IDC_SELECT_T4, m_T4_CTRL);
	DDX_Check(pDX, IDC_SELECT_T4, m_T4);
	DDX_Control(pDX, IDC_SELECT_T5, m_T5_CTRL);
	DDX_Check(pDX, IDC_SELECT_T5, m_T5);
	DDX_Control(pDX, IDC_SELECT_T6, m_T6_CTRL);
	DDX_Check(pDX, IDC_SELECT_T6, m_T6);
	DDX_Control(pDX, IDC_SELECT_T7, m_T7_CTRL);
	DDX_Check(pDX, IDC_SELECT_T7, m_T7);
	DDX_Control(pDX, IDC_SELECT_T8, m_T8_CTRL);
	DDX_Check(pDX, IDC_SELECT_T8, m_T8);
	DDX_Control(pDX, IDC_SELECT_T9, m_T9_CTRL);
	DDX_Check(pDX, IDC_SELECT_T9, m_T9);
	DDX_Control(pDX, IDC_SELECT_T10, m_T10_CTRL);
	DDX_Check(pDX, IDC_SELECT_T10, m_T10);
	DDX_Control(pDX, IDC_SELECT_T11, m_T11_CTRL);
	DDX_Check(pDX, IDC_SELECT_T11, m_T11);
	DDX_Control(pDX, IDC_SELECT_T12, m_T12_CTRL);
	DDX_Check(pDX, IDC_SELECT_T12, m_T12);
	DDX_Control(pDX, IDC_SELECT_T13, m_T13_CTRL);
	DDX_Check(pDX, IDC_SELECT_T13, m_T13);
	DDX_Control(pDX, IDC_SELECT_T14, m_T14_CTRL);
	DDX_Check(pDX, IDC_SELECT_T14, m_T14);
	DDX_Control(pDX, IDC_SELECT_T15, m_T15_CTRL);
	DDX_Check(pDX, IDC_SELECT_T15, m_T15);
	DDX_Control(pDX, IDC_SELECT_T16, m_T16_CTRL);
	DDX_Check(pDX, IDC_SELECT_T16, m_T16);
	DDX_Control(pDX, IDC_SELECT_T17, m_T17_CTRL);
	DDX_Check(pDX, IDC_SELECT_T17, m_T17);
	DDX_Control(pDX, IDC_SELECT_T18, m_T18_CTRL);
	DDX_Check(pDX, IDC_SELECT_T18, m_T18);
	DDX_Control(pDX, IDC_SELECT_T19, m_T19_CTRL);
	DDX_Check(pDX, IDC_SELECT_T19, m_T19);
	DDX_Control(pDX, IDC_SELECT_T20, m_T20_CTRL);
	DDX_Check(pDX, IDC_SELECT_T20, m_T20);
	DDX_Control(pDX, IDC_SELECT_T21, m_T21_CTRL);
	DDX_Check(pDX, IDC_SELECT_T21, m_T21);
	DDX_Control(pDX, IDC_SELECT_T22, m_T22_CTRL);
	DDX_Check(pDX, IDC_SELECT_T22, m_T22);
	DDX_Control(pDX, IDC_SELECT_T23, m_T23_CTRL);
	DDX_Check(pDX, IDC_SELECT_T23, m_T23);
	DDX_Control(pDX, IDC_SELECT_T24, m_T24_CTRL);
	DDX_Check(pDX, IDC_SELECT_T24, m_T24);
	DDX_Control(pDX, IDC_SELECT_T25, m_T25_CTRL);
	DDX_Check(pDX, IDC_SELECT_T25, m_T25);
	DDX_Control(pDX, IDC_SELECT_T26, m_T26_CTRL);
	DDX_Check(pDX, IDC_SELECT_T26, m_T26);
	DDX_Control(pDX, IDC_SELECT_T27, m_T27_CTRL);
	DDX_Check(pDX, IDC_SELECT_T27, m_T27);
	DDX_Control(pDX, IDC_SELECT_T28, m_T28_CTRL);
	DDX_Check(pDX, IDC_SELECT_T28, m_T28);
	DDX_Control(pDX, IDC_SELECT_T29, m_T29_CTRL);
	DDX_Check(pDX, IDC_SELECT_T29, m_T29);
	DDX_Control(pDX, IDC_SELECT_T30, m_T30_CTRL);
	DDX_Check(pDX, IDC_SELECT_T30, m_T30);

	DDX_Text(pDX, IDC_SELECT_PAGE, m_PageNum);
	DDX_Control(pDX, IDC_SELECT_READING, m_ReadingCTRL);
	DDX_Text(pDX, IDC_SELECT_READING, m_Reading);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClanTiersSelect, CDialog)
	//{{AFX_MSG_MAP(CClanTiersSelect)
	ON_BN_CLICKED(IDC_EXCLUDE_ALL_MAIN, OnExcludeSpeakerData)
	ON_BN_CLICKED(IDC_INCLUDE_ALL_HEADER, OnIncludeHeaders)
	ON_BN_CLICKED(IDC_SELECT_ALL_SPEAKER, OnSelectAllSpeaker)
	ON_BN_CLICKED(IDC_SELECT_ALL_DEP, OnSelectAllDep)
	ON_BN_CLICKED(IDC_SELECT_PAGE_PREV, OnSelectPrevPage)
	ON_BN_CLICKED(IDC_SELECT_PAGE_NEXT, OnSelectNextPage)
	ON_BN_CLICKED(IDC_SELECT_T1, OnSelectT1)
	ON_BN_CLICKED(IDC_SELECT_T2, OnSelectT2)
	ON_BN_CLICKED(IDC_SELECT_T3, OnSelectT3)
	ON_BN_CLICKED(IDC_SELECT_T4, OnSelectT4)
	ON_BN_CLICKED(IDC_SELECT_T5, OnSelectT5)
	ON_BN_CLICKED(IDC_SELECT_T6, OnSelectT6)
	ON_BN_CLICKED(IDC_SELECT_T7, OnSelectT7)
	ON_BN_CLICKED(IDC_SELECT_T8, OnSelectT8)
	ON_BN_CLICKED(IDC_SELECT_T9, OnSelectT9)
	ON_BN_CLICKED(IDC_SELECT_T10, OnSelectT10)
	ON_BN_CLICKED(IDC_SELECT_T11, OnSelectT11)
	ON_BN_CLICKED(IDC_SELECT_T12, OnSelectT12)
	ON_BN_CLICKED(IDC_SELECT_T13, OnSelectT13)
	ON_BN_CLICKED(IDC_SELECT_T14, OnSelectT14)
	ON_BN_CLICKED(IDC_SELECT_T15, OnSelectT15)
	ON_BN_CLICKED(IDC_SELECT_T16, OnSelectT16)
	ON_BN_CLICKED(IDC_SELECT_T17, OnSelectT17)
	ON_BN_CLICKED(IDC_SELECT_T18, OnSelectT18)
	ON_BN_CLICKED(IDC_SELECT_T19, OnSelectT19)
	ON_BN_CLICKED(IDC_SELECT_T20, OnSelectT20)
	ON_BN_CLICKED(IDC_SELECT_T21, OnSelectT21)
	ON_BN_CLICKED(IDC_SELECT_T22, OnSelectT22)
	ON_BN_CLICKED(IDC_SELECT_T23, OnSelectT23)
	ON_BN_CLICKED(IDC_SELECT_T24, OnSelectT24)
	ON_BN_CLICKED(IDC_SELECT_T25, OnSelectT25)
	ON_BN_CLICKED(IDC_SELECT_T26, OnSelectT26)
	ON_BN_CLICKED(IDC_SELECT_T27, OnSelectT27)
	ON_BN_CLICKED(IDC_SELECT_T28, OnSelectT28)
	ON_BN_CLICKED(IDC_SELECT_T29, OnSelectT29)
	ON_BN_CLICKED(IDC_SELECT_T30, OnSelectT30)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClanTiersSelect message handlers
void CClanTiersSelect::OnExcludeSpeakerData() 
{
	UpdateData(true);
	m_ExcludeSpeakerData = !m_ExcludeSpeakerData;
	UpdateData(false);
}

void CClanTiersSelect::OnIncludeHeaders() 
{
	UpdateData(true);
	m_IncludeHeaders = !m_IncludeHeaders;
	UpdateData(false);
}

void CClanTiersSelect::OnSelectAllSpeaker() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_SelectAllSpeaker = !m_SelectAllSpeaker;
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN)
			setNameOfButton(NULL, p->num, SW_SHOW, m_SelectAllSpeaker);
		p->val = m_SelectAllSpeaker;
	}
	UpdateData(false);
}

void CClanTiersSelect::OnSelectAllDep() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_SelectAllDep = !m_SelectAllDep;
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN)
			setNameOfButton(NULL, p->num, SW_SHOW, m_SelectAllDep);
		p->val = m_SelectAllDep;
	}
	UpdateData(false);
}

void CClanTiersSelect::OnSelectPrevPage() 
{
	int i;
	struct TiersListS *p;

	UpdateData(true);
	pageN--;
	i = 1;
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN) {
			i = p->num;
			setNameOfButton(p, i, SW_SHOW, p->val);
		} else if (p->pageN > pageN)
			break;
	}
	if (i < 16) {
		for (i++; i < 16; i++)
			setNameOfButton(NULL, i, SW_HIDE, 0);
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN) {
			i = p->num;
			setNameOfButton(p, i, SW_SHOW, p->val);
		} else if (p->pageN > pageN)
			break;
	}
	if (pageN == 1)
		m_SelectPrevPageCTRL.ShowWindow(SW_HIDE);
	m_SelectNextPageCTRL.ShowWindow(SW_SHOW);
	sprintf(templineC3, "page %d", pageN);
	u_strcpy(templine3, templineC3, UTTLINELEN);
	m_PageNum = templine3;
	for (i++; i < DIAL_NUMBER_TIERS; i++)
		setNameOfButton(NULL, i, SW_HIDE, 0);
	UpdateData(false);
}

void CClanTiersSelect::OnSelectNextPage() 
{
	int i;
	struct TiersListS *p;

	UpdateData(true);
	pageN++;
	i = 1;
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN) {
			i = p->num;
			setNameOfButton(p, i, SW_SHOW, p->val);
		} else if (p->pageN > pageN)
			break;
	}
	if (i < 16) {
		for (i++; i < 16; i++)
			setNameOfButton(NULL, i, SW_HIDE, 0);
	}
	if (p != NULL && p->pageN > pageN) {
		m_SelectNextPageCTRL.ShowWindow(SW_SHOW);
	} else {
		for (p=depTiersRoot; p != NULL; p=p->next_tier) {
			if (p->pageN == pageN) {
				i = p->num;
				setNameOfButton(p, i, SW_SHOW, p->val);
			} else if (p->pageN > pageN)
				break;
		}
		if (p != NULL && p->pageN > pageN)
			m_SelectNextPageCTRL.ShowWindow(SW_SHOW);
	}
	m_SelectPrevPageCTRL.ShowWindow(SW_SHOW);
	if (p == NULL)
		m_SelectNextPageCTRL.ShowWindow(SW_HIDE);
	sprintf(templineC3, "page %d", pageN);
	u_strcpy(templine3, templineC3, UTTLINELEN);
	m_PageNum = templine3;
	for (i++; i < DIAL_NUMBER_TIERS; i++)
		setNameOfButton(NULL, i, SW_HIDE, 0);
	UpdateData(false);
}

void CClanTiersSelect::OnSelectT1() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T1 = !m_T1;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 1) {
			p->val = m_T1;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 1) {
			p->val = m_T1;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT2() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T2 = !m_T2;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 2) {
			p->val = m_T2;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 2) {
			p->val = m_T2;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT3() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T3 = !m_T3;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 3) {
			p->val = m_T3;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 3) {
			p->val = m_T3;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT4() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T4 = !m_T4;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 4) {
			p->val = m_T4;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 4) {
			p->val = m_T4;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT5() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T5 = !m_T5;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 5) {
			p->val = m_T5;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 5) {
			p->val = m_T5;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT6() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T6 = !m_T6;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 6) {
			p->val = m_T6;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 6) {
			p->val = m_T6;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT7() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T7 = !m_T7;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 7) {
			p->val = m_T7;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 7) {
			p->val = m_T7;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT8() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T8 = !m_T8;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 8) {
			p->val = m_T8;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 8) {
			p->val = m_T8;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT9() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T9 = !m_T9;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 9) {
			p->val = m_T9;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 9) {
			p->val = m_T9;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT10() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T10 = !m_T10;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 10) {
			p->val = m_T10;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 10) {
			p->val = m_T10;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT11() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T11 = !m_T11;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 11) {
			p->val = m_T11;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 11) {
			p->val = m_T11;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT12() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T12 = !m_T12;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 12) {
			p->val = m_T12;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 12) {
			p->val = m_T12;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT13() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T13 = !m_T13;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 13) {
			p->val = m_T13;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 13) {
			p->val = m_T13;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT14() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T14 = !m_T14;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 14) {
			p->val = m_T14;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 14) {
			p->val = m_T14;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT15() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T15 = !m_T15;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 15) {
			p->val = m_T15;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 15) {
			p->val = m_T15;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT16() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T16 = !m_T16;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 16) {
			p->val = m_T16;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 16) {
			p->val = m_T16;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT17() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T17 = !m_T17;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 17) {
			p->val = m_T17;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 17) {
			p->val = m_T17;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT18() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T18 = !m_T18;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 18) {
			p->val = m_T18;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 18) {
			p->val = m_T18;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT19() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T19 = !m_T19;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 19) {
			p->val = m_T19;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 19) {
			p->val = m_T19;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT20() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T20 = !m_T20;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 20) {
			p->val = m_T20;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 20) {
			p->val = m_T20;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT21() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T21 = !m_T21;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 21) {
			p->val = m_T21;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 21) {
			p->val = m_T21;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT22() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T22 = !m_T22;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 22) {
			p->val = m_T22;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 22) {
			p->val = m_T22;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT23() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T23 = !m_T23;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 23) {
			p->val = m_T23;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 23) {
			p->val = m_T23;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT24() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T24 = !m_T24;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 24) {
			p->val = m_T24;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 24) {
			p->val = m_T24;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT25() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T25 = !m_T25;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 25) {
			p->val = m_T25;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 25) {
			p->val = m_T25;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT26() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T26 = !m_T26;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 26) {
			p->val = m_T26;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 26) {
			p->val = m_T26;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT27() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T27 = !m_T27;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 27) {
			p->val = m_T27;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 27) {
			p->val = m_T27;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT28() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T28 = !m_T28;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 28) {
			p->val = m_T28;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 28) {
			p->val = m_T28;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT29() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T29 = !m_T29;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 29) {
			p->val = m_T29;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 29) {
			p->val = m_T29;
			break;
		}
	}
}

void CClanTiersSelect::OnSelectT30() 
{
	struct TiersListS *p;

	UpdateData(true);
	m_T30 = !m_T30;
	UpdateData(false);
	for (p=spTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 30) {
			p->val = m_T30;
			return;
		}
	}
	for (p=depTiersRoot; p != NULL; p=p->next_tier) {
		if (p->pageN == pageN && p->num == 30) {
			p->val = m_T30;
			break;
		}
	}
}

static struct TiersListS *cleanTiersList(struct TiersListS *p) {
	struct TiersListS *t;

	while (p != NULL) {
		t = p;
		p = p->next_tier;
		if (t->ID != NULL) {
			free(t->ID);
		}
		free(t);
	}
	return(NULL);
}

static struct TiersListS *addToTiersArr(struct TiersListS *root, short pixelLen, char *code, char *ID) {
	int IDlen, len;
	char *s;
	struct TiersListS *nt, *tnt;

	len = strlen(code);
	if (ID != NULL) {
		IDlen = strlen(ID);
//		if (ID[0] != '|')
//			IDlen++;
//		pixelLen = pixelLen - TextWidth(code, 0, len);
//		while (TextWidth(ID, 0, IDlen) > pixelLen) {
		while (len+IDlen >= DIAL_SPEAKER_FILED_LEN) {
			if (ID[0] == '|')
				ID++;
			s = strchr(ID, '|');
			if (s == NULL) {
				ID = NULL;
				IDlen = 0;
				break;
			}
			ID = s;
			IDlen = strlen(ID);
		}
		len = len + IDlen;
	}
	if (root == NULL) {
		root = NEW(struct TiersListS);
		if (root == NULL)
			return(root);
		nt = root;
		nt->next_tier = NULL;
	} else {
		for (nt=root; nt != NULL; nt=nt->next_tier) {
			if (uS.mStricmp(nt->code, code) == 0)
				return(root);
		}
		tnt = root;
		nt = root;
		while (1) {
			if (nt == NULL)
				break;
			else if (nt->num < len)
				break;
			tnt = nt;
			nt = nt->next_tier;
		}
		if (nt == NULL) {
			nt = NEW(struct TiersListS);
			if (nt == NULL)
				return(root);
			tnt->next_tier = nt;
			nt->next_tier = NULL;
		} else if (nt == root) {
			tnt = NEW(struct TiersListS);
			if (tnt == NULL)
				return(root);
			root = tnt;
			root->next_tier = nt;
			nt = root;
		} else {
			nt = NEW(struct TiersListS);
			if (nt == NULL)
				return(root);
			nt->next_tier = tnt->next_tier;
			tnt->next_tier = nt;
		}
	}
	strncpy(nt->code, code, CODESTRLEN);
	nt->code[CODESTRLEN] = EOS;
	if (ID != NULL) {
		nt->ID = (char *)malloc(strlen(ID)+1);
		if (nt->ID != NULL)
			strcpy(nt->ID, ID);
	} else
		nt->ID = NULL;
	nt->num = len;
	return(root);
}

void CClanTiersSelect::getTierNamesFromFile(short pixelLen, char *fname) {
	char	*s, *e, *code;
	time_t	timer;
	FILE	*fp;

	fp = fopen(fname, "r");
	if (fp == NULL)
		return;
//	UpdateData(true);
	time(&timer);
	if (timer > GlobalTime) {
		cnt = cnt + 1;
		sprintf(templineC3, "Reading file(s) %d", cnt);
		u_strcpy(templine3, templineC3, UTTLINELEN);
		m_Reading = templine3;
		GlobalTime = timer + 1;
	}
	UpdateData(false);
	while (fgets_cr(templineC3, 3072, fp)) {
		if (templineC3[0] == '*') {
			s = strchr(templineC3, ':');
			if (s != NULL) {
				*s = EOS;
				spTiersRoot = addToTiersArr(spTiersRoot, pixelLen, templineC3, NULL);
			}
		} else if (templineC3[0] == '%') {
			s = strchr(templineC3, ':');
			if (s != NULL) {
				*s = EOS;
				depTiersRoot = addToTiersArr(depTiersRoot, pixelLen, templineC3, NULL);
			}
		} else if (strncmp(templineC3, "@ID:", 4) == 0) {
			s = strchr(templineC3, '|');
			if (s != NULL) {
				s++;
				code = strchr(s, '|');
				if (code != NULL) {
					*code = '*';
					s = strchr(code, '|');
					if (s != NULL) {
						uS.remblanks(code);
						*s = EOS;
						s++;
						for (e=s; *e != EOS; ) {
							if (*e == '|' && *(e+1) == '|')
								strcpy(e, e+1);
							else
								e++;
						}
						spTiersRoot = addToTiersArr(spTiersRoot, pixelLen, code, s);
					}
				}
			}
		}
	}
	fclose(fp);
}

void CClanTiersSelect::makeTiersList(int ProgNum, short pixelLen) {
	int		i, j, len;
	FILE	*fp;

	SetNewVol(wd_dir);
	cnt = 0;
	for (i=1; i < cl_argc; i++) {
		if (cl_argv[i][0] == '-' || cl_argv[i][0] == '+') {
			if ((cl_argv[i][1] == 's' || cl_argv[i][1] == 'S') && ProgNum == CHSTRING)
				i++;
		} else {
			if (!strcmp(cl_argv[i], "@")) {
				for (j=1; j <= F_numfiles; j++) {
					get_selected_file(j, FileName1);
					getTierNamesFromFile(pixelLen, FileName1);
				}
			} else if (cl_argv[i][0] == '@' && cl_argv[i][1] == ':') {
				uS.str2FNType(FileName1, 0L, cl_argv[i]+2);
				fp = fopen(FileName1, "r");
				if (fp == NULL) {
					return;
				}
				while (fgets_cr(FileName1, 3072, fp)) {
					uS.remFrontAndBackBlanks(FileName1);
					getTierNamesFromFile(pixelLen, FileName1);
				}
				fclose(fp);
			} else if (strchr(cl_argv[i], '*') == NULL) {
				getTierNamesFromFile(pixelLen, cl_argv[i]);
			} else {
				strcpy(DirPathName, wd_dir);
				len = strlen(DirPathName);
				j = 1;
				while ((j=Get_File(FileName1, j)) != 0) {
					if (uS.fIpatmat(FileName1, cl_argv[i])) {
						addFilename2Path(DirPathName, FileName1);
						getTierNamesFromFile(pixelLen, DirPathName);
						DirPathName[len] = EOS;
					}
				}
			}
		}
	}
}

static char isExcludeHeadFromProg(int ProgNum) {
	if (ProgNum == FREQMERGE || ProgNum == TEXTIN   || ProgNum == ANVIL2CHAT||
		ProgNum == COMBTIER  || ProgNum == ELAN2CHAT || ProgNum == LAB2CHAT || ProgNum == LIPP2CHAT ||
		ProgNum == OLAC_P    || ProgNum == PRAAT2CHAT|| ProgNum == RTFIN    || ProgNum == SALTIN    ||
		ProgNum == SUBTITLES || ProgNum == UNIQ      || ProgNum == COMPOUND || ProgNum == DOS2UNIX  ||
		ProgNum == CHECK     || ProgNum == RELY      || ProgNum == CP2UTF   || ProgNum == FIXIT     ||
		ProgNum == REPEAT    || ProgNum == RETRACE   || ProgNum == MOR_P    || ProgNum == MEGRASP   ||
		ProgNum == FIXCA) 
		return(FALSE);
	return(TRUE);
}

static char isExcludeDepFromProg(int ProgNum) {
	if (ProgNum == CHAINS || ProgNum == FLO    || ProgNum == KEYMAP || ProgNum == MODREP  ||
		ProgNum == CHECK  || ProgNum == CP2UTF || ProgNum == IMDI_P || ProgNum == LOWCASE || 
		ProgNum == RETRACE|| ProgNum == RELY   || ProgNum == FIXIT  || ProgNum == REPEAT  ||
		ProgNum == RETRACE|| ProgNum == MOR_P  || ProgNum == MEGRASP|| ProgNum == FIXCA) 
		return(FALSE);
	return(TRUE);
}
// SW_SHOW
// SW_HIDE
//CButton
//RECT pmargin;
//m_T1_CTRL.GetTextMargin(&pmargin);
void CClanTiersSelect::setNameOfButton(struct TiersListS *p, int which, int nCmdShow, BOOL val) {
	int len;

	if (p != NULL) {
		strcpy(templineC3, p->code);
		if (p->ID != NULL) {
			len = strlen(p->code);
			if (p->ID[0] != '|') {
				templineC3[len] = '|';
				len++;
			}
			strcpy(templineC3+len, p->ID);
		}
		u_strcpy(templine3, templineC3, UTTLINELEN);
	}
	if (which == 1) {
		m_T1_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T1, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T1 = val;
		}
	} else if (which == 2) {
		m_T2_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T2, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T2 = val;
		}
	} else if (which == 3) {
		m_T3_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T3, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T3 = val;
		}
	} else if (which == 4) {
		m_T4_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T4, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T4 = val;
		}
	} else if (which == 5) {
		m_T5_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T5, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T5 = val;
		}
	} else if (which == 6) {
		m_T6_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T6, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T6 = val;
		}
	} else if (which == 7) {
		m_T7_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T7, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T7 = val;
		}
	} else if (which == 8) {
		m_T8_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T8, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T8 = val;
		}
	} else if (which == 9) {
		m_T9_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T9, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T9 = val;
		}
	} else if (which == 10) {
		m_T10_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T10, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T10 = val;
		}
	} else if (which == 11) {
		m_T11_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T11, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T11 = val;
		}
	} else if (which == 12) {
		m_T12_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T12, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T12 = val;
		}
	} else if (which == 13) {
		m_T13_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T13, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T13 = val;
		}
	} else if (which == 14) {
		m_T14_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T14, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T14 = val;
		}
	} else if (which == 15) {
		m_T15_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T15, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T15 = val;
		}
	} else if (which == 16) {
		m_T16_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T16, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T16 = val;
		}
	} else if (which == 17) {
		m_T17_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T17, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T17 = val;
		}
	} else if (which == 18) {
		m_T18_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T18, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T18 = val;
		}
	} else if (which == 19) {
		m_T19_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T19, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T19 = val;
		}
	} else if (which == 20) {
		m_T20_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T20, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T20 = val;
		}
	} else if (which == 21) {
		m_T21_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T21, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T21 = val;
		}
	} else if (which == 22) {
		m_T22_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T22, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T22 = val;
		}
	} else if (which == 23) {
		m_T23_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T23, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T23 = val;
		}
	} else if (which == 24) {
		m_T24_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T24, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T24 = val;
		}
	} else if (which == 25) {
		m_T25_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T25, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T25 = val;
		}
	} else if (which == 26) {
		m_T26_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T26, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T26 = val;
		}
	} else if (which == 27) {
		m_T27_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T27, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T27 = val;
		}
	} else if (which == 28) {
		m_T28_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T28, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T28 = val;
		}
	} else if (which == 29) {
		m_T29_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T29, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T29 = val;
		}
	} else if (which == 30) {
		m_T30_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_T30, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T30 = val;
		}
	} 
}

BOOL CClanTiersSelect::OnInitDialog() {
	int i;

	CDialog::OnInitDialog();
	for (i=1; i < DIAL_NUMBER_TIERS; i++)
		setNameOfButton(NULL, i, SW_HIDE, 0);
	m_ExcludeSpeakerDataCTRL.ShowWindow(SW_HIDE);
	m_IncludeHeadersCTRL.ShowWindow(SW_HIDE);
	m_SelectAllSpeakerCTRL.ShowWindow(SW_HIDE);
	m_SelectAllDepCTRL.ShowWindow(SW_HIDE);
	m_SelectPrevPageCTRL.ShowWindow(SW_HIDE);
	m_SelectNextPageCTRL.ShowWindow(SW_HIDE);
	ReadyToLoadState = 1;
	return 0;
}

/*
BOOL CClanTiersSelect::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_CHAR && pMsg->wParam == 17) { //^W
		PlayingContMovie = false;
		if (qt) {
			StopMovie(qt->theMovie);
			qt->isPlaying = 0;
			isPlayRange = false;
			PBC.walk = 0;
			PBC.isPC = 0;
		}
		isPlayRange = false;
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
*/
LRESULT CClanTiersSelect::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	int i, len;
	struct TiersListS *p;
	LRESULT res = 0;

	if (message == WM_PAINT && ReadyToLoadState == 1) {
		ReadyToLoadState = 2;
		m_ReadingCTRL.ShowWindow(SW_SHOW);
		GlobalTime = 0;
		makeTiersList(ProgNum, 30);
		m_ReadingCTRL.ShowWindow(SW_HIDE);
		m_ExcludeSpeakerDataCTRL.ShowWindow(SW_SHOW);
		m_ExcludeSpeakerData = false;
		m_IncludeHeadersCTRL.ShowWindow(SW_SHOW);
		if (isExcludeHeadFromProg(ProgNum))
			m_IncludeHeaders = false;
		else
			m_IncludeHeaders = true;
		pageN = 1;
		i = 1;
		for (p=spTiersRoot; p != NULL; p=p->next_tier) {
			if (pageN == 1 && i < DIAL_NUMBER_TIERS) {
				setNameOfButton(p, i, SW_SHOW, 1);
			} else if (i >= DIAL_NUMBER_TIERS) {
				pageN++;
				i = 1;
			}
			p->pageN = pageN;
			p->num = i;
			p->val = 1;
			i++;
		}
		m_SelectAllSpeaker = true;
		m_SelectAllSpeakerCTRL.ShowWindow(SW_SHOW);
		if (pageN == 1 && i < 16) {
			len = 16;
			for (p=depTiersRoot; p != NULL; p=p->next_tier)
				len++;
			if (len <= DIAL_NUMBER_TIERS) {
				for (; i < 16; i++)
					setNameOfButton(NULL, i, SW_HIDE, 0);
				i = 16;
			}
		}
		if (isExcludeDepFromProg(ProgNum)) {
			for (p=depTiersRoot; p != NULL; p=p->next_tier) {
				if (pageN == 1 && i < DIAL_NUMBER_TIERS) {
					if (strcmp(p->code, "%mor") == 0) {
						if (ProgNum == MLU || ProgNum == WDLEN  || ProgNum == MORTABLE || ProgNum == POSTMORTEM)
							setNameOfButton(p, i, SW_SHOW, 1);
						else
							setNameOfButton(p, i, SW_SHOW, 0);
					} else
						setNameOfButton(p, i, SW_SHOW, 0);
				} else if (i >= DIAL_NUMBER_TIERS) {
					pageN++;
					i = 1;
				}
				p->pageN = pageN;
				p->num = i;
				if (strcmp(p->code, "%mor") == 0) {
					if (ProgNum == MLU || ProgNum == WDLEN  || ProgNum == MORTABLE || ProgNum == POSTMORTEM)
						p->val = 1;
					else
						p->val = 0;
				} else
					p->val = 0;
				i++;
			}
			m_SelectAllDep = false;
		} else {
			for (p=depTiersRoot; p != NULL; p=p->next_tier) {
				if (pageN == 1 && i < DIAL_NUMBER_TIERS) {
					setNameOfButton(p, i, SW_SHOW, 0);
				} else if (i >= DIAL_NUMBER_TIERS) {
					pageN++;
					i = 1;
				}
				p->pageN = pageN;
				p->num = i;
				p->val = 1;
				i++;
			}
			m_SelectAllDep = true;
		}
		m_SelectAllDepCTRL.ShowWindow(SW_SHOW);
		if (pageN == 1) {
			for (; i < DIAL_NUMBER_TIERS; i++)
				setNameOfButton(NULL, i, SW_HIDE, 0);
		} else
			m_SelectNextPageCTRL.ShowWindow(SW_SHOW);
		pageN = 1;
		UpdateData(false);	
	}
	res = CDialog ::WindowProc(message, wParam, lParam);
	return(res);
}

static void selectTiersDialog(int ProgNum) {
	int	i, len, oldHead;
	struct TiersListS *p;
	CClanTiersSelect dlg;

	spTiersRoot = NULL;
	depTiersRoot = NULL;
	dlg.ReadyToLoadState = 0;
	dlg.pageN = 1;
	dlg.ProgNum = ProgNum;
	dlg.m_PageNum = _T("page 1");
	dlg.m_ExcludeSpeakerData = false;
	if (isExcludeHeadFromProg(ProgNum)) {
		oldHead = 0;
		dlg.m_IncludeHeaders = false;
	} else {
		oldHead = 1;
		dlg.m_IncludeHeaders = true;
	}
	if (dlg.DoModal() == IDOK) {
		templineC3[0] = EOS;
		if (dlg.m_ExcludeSpeakerData)
			strcat(templineC3, " -t*");
		if (dlg.m_IncludeHeaders) {
			if (oldHead != 1) {
				strcat(templineC3, " +t@");
			}
		} else {
			if (oldHead != 0) {
				strcat(templineC3, " -t@");
			}
		}
		i = 0;
		len = 0;
		for (p=spTiersRoot; p != NULL; p=p->next_tier) {
			len++;
			if (p->val == 1)
				i++;
		}
		if (i != len) {
			len = len / 2;
			if (i > len) {
				for (p=spTiersRoot; p != NULL; p=p->next_tier) {
					if (p->val == 0) {
						strcat(templineC3, " -t");
						strcat(templineC3, p->code);
						strcat(templineC3, ":");
					}
				}
			} else if (i <= len) {
				for (p=spTiersRoot; p != NULL; p=p->next_tier) {
					if (p->val == 1) {
						strcat(templineC3, " +t");
						strcat(templineC3, p->code);
						strcat(templineC3, ":");
					}
				}
			}
		}
		i = 0;
		len = 0;
		for (p=depTiersRoot; p != NULL; p=p->next_tier) {
			len++;
			if (p->val == 1)
				i++;
		}	
		if (i != len) {
			len = len / 2;
			if (i > len) {
				for (p=depTiersRoot; p != NULL; p=p->next_tier) {
					if (p->val == 0) {
						strcat(templineC3, " -t");
						strcat(templineC3, p->code);
					}
				}
			} else if (i <= len) {
				for (p=depTiersRoot; p != NULL; p=p->next_tier) {
					if (p->val == 1) {
						if (strcmp(p->code, "%mor") == 0) {
							if (ProgNum == MLU || ProgNum == WDLEN  || ProgNum == MORTABLE || ProgNum == POSTMORTEM) {
							} else {
								strcat(templineC3, " +t");
								strcat(templineC3, p->code);
								strcat(templineC3, ":");
							}
						} else {
							strcat(templineC3, " +t");
							strcat(templineC3, p->code);
							strcat(templineC3, ":");
						}
					} else if (strcmp(p->code, "%mor") == 0) {
						if (ProgNum == MLU || ProgNum == WDLEN  || ProgNum == MORTABLE || ProgNum == POSTMORTEM) {
							strcat(templineC3, " -t");
							strcat(templineC3, p->code);
							strcat(templineC3, ":");
						}
					}
				}
			}
		}
	} else {
		templineC3[0] = EOS;
	}
	spTiersRoot = cleanTiersList(spTiersRoot);
	depTiersRoot = cleanTiersList(depTiersRoot);
}

static char isFilesSpecified(int *ProgNum) {
	int  i;
	char *com, progName[512+1];

	com = cl_argv[0];
	if (getAliasProgName(com, progName, 512)) {
		com = progName;
	}
	*ProgNum = get_clan_prog_num(com, FALSE);
	if (cl_argc < 2)
		return(FALSE);
	for (i=1; i < cl_argc; i++) {
		if (cl_argv[i][0] == '-' || cl_argv[i][0] == '+') {
			if ((cl_argv[i][1] == 's' || cl_argv[i][1] == 'S') && *ProgNum == CHSTRING)
				i++;
		} else
			return(TRUE);
	}
	return(FALSE);
}

void TiersDialog(unCH *str, unCH *fbufferU) {
	int  i;
	char isFound;
	unCH *isSpCharFound;
	CClanTiers dlg;

	str[0] = EOS;
	STier.name[0] = EOS;
	dlg.res = false;
	dlg.m_TierName = STier.name;
	if (STier.what2do == EXCLUDEALL) {
		dlg.m_ExcludeAllMain = true;
		dlg.m_Include = false;
		dlg.m_Exclude = false;
	} else if (STier.what2do == INCLUDE1) {
		dlg.m_ExcludeAllMain = false;
		dlg.m_Include = true;
		dlg.m_Exclude = false;
	} if (STier.what2do == EXCLUDE1) {
		dlg.m_ExcludeAllMain = false;
		dlg.m_Include = false;
		dlg.m_Exclude = true;
	}
	if (STier.type == HEADT) {
		dlg.m_MainTier = false;
		dlg.m_DepTier = false;
		dlg.m_HeadTier = true;
		dlg.m_RoleTier = false;
//		dlg.m_IDTier = false;
		dlg.m_TierName = _T("@");
	} else if (STier.type == MAINT) {
		dlg.m_MainTier = true;
		dlg.m_DepTier = false;
		dlg.m_HeadTier = false;
		dlg.m_RoleTier = false;
//		dlg.m_IDTier = false;
		dlg.m_TierName = _T("*");
	} else if (STier.type == DEPT) {
		dlg.m_MainTier = false;
		dlg.m_DepTier = true;
		dlg.m_HeadTier = false;
		dlg.m_RoleTier = false;
//		dlg.m_IDTier = false;
		dlg.m_TierName = _T("%");
	} else if (STier.type == ROLE) {
		dlg.m_MainTier = false;
		dlg.m_DepTier = false;
		dlg.m_HeadTier = false;
		dlg.m_RoleTier = true;
//		dlg.m_IDTier = false;
		dlg.m_TierName = STier.role;
	}
/*
	else if (STier.type == IDS) {
		dlg.m_MainTier = false;
		dlg.m_DepTier = false;
		dlg.m_HeadTier = false;
		dlg.m_RoleTier = false;
//		dlg.m_IDTier = true;
	}
*/
	if (dlg.DoModal() == IDOK) {
		strcpy(STier.name, dlg.m_TierName);
		uS.remFrontAndBackBlanks(STier.name);
		if (dlg.m_ExcludeAllMain) {
			STier.what2do = EXCLUDEALL;
		} else {
			if (dlg.m_Include) {
				STier.what2do = INCLUDE1;
			} else if (dlg.m_Exclude) {
				STier.what2do = EXCLUDE1;
			}
			if (dlg.m_MainTier) {
				STier.type = MAINT;
			} else if (dlg.m_DepTier) {
				STier.type = DEPT;
			} else if (dlg.m_HeadTier) {
				STier.type = HEADT;
			} else if (dlg.m_RoleTier) {
				STier.type = ROLE;
			}
/*
			else if (dlg.m_IDTier) {
				STier.type = IDS;
			}
*/
		}
		isFound = FALSE;
		if (STier.what2do == EXCLUDEALL) {
			strcat(str, " -t*");
			isFound = TRUE;
		} else {
			strcat(str, " ");
			if (STier.what2do == INCLUDE1) {
				strcat(str, "+t");
				isFound = TRUE;
			} else if (STier.what2do == EXCLUDE1) {
				strcat(str, "-t");
				isFound = TRUE;
			}
			isSpCharFound = strchr(STier.name, '|');
			if (isSpCharFound != NULL || STier.type == IDS)
				strcat(str, "\"");
			if (!uS.mStrnicmp(STier.name, "@ID=", 4) || !uS.mStrnicmp(STier.name, "ID=", 3))
				STier.type = HEADT;
			if (STier.name[0] == '#') {
			} else if (STier.type == MAINT) {
				strcat(str, "*");
				uS.uppercasestr(STier.name, NULL, FALSE);
			} else if (STier.type == DEPT) {
				strcat(str, "%");
				uS.lowercasestr(STier.name, NULL, FALSE);
			} else if (STier.type == HEADT) {
				strcat(str, "@");
			} else if (STier.type == ROLE) {
				strcat(str, "#");
				strcpy(STier.role, dlg.m_TierName);
			} else if (STier.type == IDS) {
				strcat(str, "@ID=");
				if (STier.name[0] != '*')
					strcat(str, "*|");
			} else
				isFound = FALSE;
			if (STier.type != IDS && STier.type != ROLE) {
				for (i=0; STier.name[i] != EOS; i++) {
					if (STier.name[i] != '*' && STier.name[i] != '%' && STier.name[i] != '@' && !isSpace(STier.name[i]))
						break;
				}
			} else
				i = 0;
			strcat(str, STier.name+i);
			if (STier.type == IDS && STier.name[0] != '*')
				strcat(str, "|*");
			if (isSpCharFound != NULL || STier.type == IDS)
				strcat(str, "\"");
		}
		if (!isFound)
			str[0] = EOS;
	} else if (dlg.res == TRUE) {
		int  ProgNum, res;
		char *com;
		u_strcpy(templineC1, fbufferU, UTTLINELEN);
		com = templineC1;
		cl_argc = 0;
		cl_argv[cl_argc++] = com;
		for (; *com != EOS && *com != ' ' && *com != '\t'; com++) {
			*com = (char)tolower((unsigned char)*com);
		}
		if (*com != EOS) {
			*com = EOS;
			com++;
			if (!MakeArgs(com))
				return;
		}
		spTiersRoot = NULL;
		depTiersRoot = NULL;
		ProgNum = -1;
		if (!isFilesSpecified(&ProgNum)) {
			res = SelectFilesDialog(templine2);
			if (res == CANCEL)
				return;
			if (res == FILES_PAT) {
				strcat(str, cl_T(" "));
				strcat(str, templine2);
			} else if (res == FILES_IN) {
				myget();
				if (F_numfiles > 0) {
					for (i=0; i < cl_argc && strcmp(cl_argv[i], "@"); i++) ;
					if (i >= cl_argc) {
						strcat(str, templine2);
					}
				}
			}
			u_strcpy(templineC2, templine2, UTTLINELEN);
			cl_argc = 1;
			if (!MakeArgs(templineC2))
				return;
			if (isFindFile(ProgNum) == FALSE) {
				do_warning("Can't open any of specified files. Please check for specified files in working directory", 0);
				templineW[0] = EOS;
			} else {
				selectTiersDialog(ProgNum);
				if (templineC3[0] != EOS) {
					u_strcpy(templine3, templineC3, UTTLINELEN);
					strcat(str, templine3);
				}
			}
		} else {
			if (isFindFile(ProgNum) == FALSE) {
				do_warning("Can't open any of specified files. Please check for specified files in working directory", 0);
				templineW[0] = EOS;
			} else {
				selectTiersDialog(ProgNum);
				if (templineC3[0] != EOS) {
					u_strcpy(templine3, templineC3, UTTLINELEN);
					strcat(str, templine3);
				}
			}
		}
	}
}
