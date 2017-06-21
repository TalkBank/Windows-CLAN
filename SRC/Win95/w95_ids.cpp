// w95_ids.cpp : implementation file
//

#include "ced.h"
#include "w95_ids.h"

/////////////////////////////////////////////////////////////////////////////
// CSetIDs dialog
IMPLEMENT_DYNAMIC(CSetIDs, CDialog)

CSetIDs::CSetIDs(CWnd* pParent /*=NULL*/)
	: CDialog(CSetIDs::IDD, pParent)
//	, m_SpeakerIDs(_T(""))
	, m_RoleS(_T(""))
	, m_LanguageS(_T(""))
	, m_CorpusS(_T(""))
	, m_TierS(_T(""))
	, m_Y1S(_T(""))
	, m_M1S(_T(""))
	, m_D1S(_T(""))
	, m_Y2S(_T(""))
	, m_M2S(_T(""))
	, m_D2S(_T(""))
	, m_About(FALSE)
	, m_Unknown(FALSE)
	, m_Male(FALSE)
	, m_Female(FALSE)
	, m_GroupS(_T(""))
	, m_SESS(_T(""))
	, m_EducationS(_T(""))
	, m_UFIDS(_T(""))
	, m_SpNameS(_T(""))
{
	spItem = 0;
}

CSetIDs::~CSetIDs()
{
}

void CSetIDs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IDS_SPEAKERID, m_SpeakerIDC);
//	DDX_CBString(pDX, IDC_IDS_SPEAKERID, m_SpeakerIDs);
	DDX_Control(pDX, IDC_IDS_ROLE, m_RoleCtrl);
	DDX_CBString(pDX, IDC_IDS_ROLE, m_RoleS);
	DDX_Control(pDX, IDC_IDS_LAN, m_LanguageC);
	DDX_Text(pDX, IDC_IDS_LAN, m_LanguageS);
	DDX_Control(pDX, IDC_IDS_CORPUS, m_CorpusC);
	DDX_Text(pDX, IDC_IDS_CORPUS, m_CorpusS);
	DDX_Control(pDX, IDC_IDS_CODE, m_TierC);
	DDX_Text(pDX, IDC_IDS_CODE, m_TierS);
	DDX_Control(pDX, IDC_IDS_Y1, m_Y1C);
	DDX_Text(pDX, IDC_IDS_Y1, m_Y1S);
	DDV_MaxChars(pDX, m_Y1S, 3);
	DDX_Control(pDX, IDC_IDS_M1, m_M1C);
	DDX_Text(pDX, IDC_IDS_M1, m_M1S);
	DDV_MaxChars(pDX, m_M1S, 2);
	DDX_Control(pDX, IDC_IDS_D1, m_D1C);
	DDX_Text(pDX, IDC_IDS_D1, m_D1S);
	DDV_MaxChars(pDX, m_D1S, 2);
	DDX_Control(pDX, IDC_IDS_Y2, m_Y2C);
	DDX_Text(pDX, IDC_IDS_Y2, m_Y2S);
	DDV_MaxChars(pDX, m_Y2S, 3);
	DDX_Control(pDX, IDC_IDS_M2, m_M2C);
	DDX_Text(pDX, IDC_IDS_M2, m_M2S);
	DDV_MaxChars(pDX, m_M2S, 2);
	DDX_Control(pDX, IDC_IDS_D2, m_D2C);
	DDX_Text(pDX, IDC_IDS_D2, m_D2S);
	DDV_MaxChars(pDX, m_D2S, 2);
	DDX_Check(pDX, IDC_IDS_ABOUT, m_About);
	DDX_Check(pDX, IDC_IDS_UNK, m_Unknown);
	DDX_Check(pDX, IDC_IDS_MALE, m_Male);
	DDX_Check(pDX, IDC_IDS_FEMALE, m_Female);
	DDX_Control(pDX, IDC_IDS_GROUP, m_GroupC);
	DDX_Text(pDX, IDC_IDS_GROUP, m_GroupS);
	DDX_Control(pDX, IDC_IDS_SES, m_SESC);
	DDX_Text(pDX, IDC_IDS_SES, m_SESS);
	DDX_Control(pDX, IDC_IDS_EDUCATION, m_EducationC);
	DDX_Text(pDX, IDC_IDS_EDUCATION, m_EducationS);
	DDX_Control(pDX, IDC_IDS_UFID, m_UFIDC);
	DDX_Text(pDX, IDC_IDS_UFID, m_UFIDS);
	DDX_Control(pDX, IDC_IDS_SPNAME, m_SpNameC);
	DDX_Text(pDX, IDC_IDS_SPNAME, m_SpNameS);
}


BEGIN_MESSAGE_MAP(CSetIDs, CDialog)
	ON_BN_CLICKED(IDC_IDS_COPY, &CSetIDs::OnIdsCopy)
	ON_BN_CLICKED(IDC_IDS_CREATE, &CSetIDs::OnIdsCreate)
	ON_BN_CLICKED(IDC_IDS_DEL, &CSetIDs::OnIdsDelete)
	ON_BN_CLICKED(IDCANCEL, &CSetIDs::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CSetIDs::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_IDS_SPEAKERID, &CSetIDs::OnCbnSelchangeIdsSpeakerid)
	ON_BN_CLICKED(IDC_IDS_UNK, &CSetIDs::OnBnClickedIdsUnk)
	ON_BN_CLICKED(IDC_IDS_MALE, &CSetIDs::OnBnClickedIdsMale)
	ON_BN_CLICKED(IDC_IDS_FEMALE, &CSetIDs::OnBnClickedIdsFemale)
	ON_EN_CHANGE(IDC_IDS_CODE, &CSetIDs::OnEnChangeIdsCode)
//	ON_EN_UPDATE(IDC_IDS_LAN, &CSetIDs::OnEnUpdateIdsLan)
END_MESSAGE_MAP()


// CSetIDs message handlers

void CSetIDs::createPopupRoleMenu() {
	int err;
	ROLESTYPE *t;

	err = m_RoleCtrl.InsertString(0, cl_T("Choose one role"));
	for (t=rootRoles; t != NULL; t=t->nextrole) {
		err = m_RoleCtrl.InsertString(-1, t->role);
	}
	m_RoleCtrl.SetCurSel(0);
}

void CSetIDs::createPopupSpeakerMenu() {
	int			i, err;
	IDSTYPE		*p;

	for (i=m_SpeakerIDC.GetCount()-1; i >= 0; i--)
	   m_SpeakerIDC.DeleteString(i);

	i = 0;
	for (p=rootIDs; p != NULL; p=p->next_id) {
		err = m_SpeakerIDC.InsertString(-1, p->code);
	}
	if (spItem < 0 || spItem >= m_SpeakerIDC.GetCount())
		spItem = 0;
}

void CSetIDs::fillInIDFields() {
	int		item, err;
	wchar_t	str[IDFIELSSIZE+1];
	IDSTYPE *p;

	if (spItem < 0 || spItem >= m_SpeakerIDC.GetCount())
		return;
	item = spItem;
	for (p=rootIDs; p != NULL; p=p->next_id) {
		item--;
		if (item < 0)
			 break;
	}
	if (p != NULL) {
		m_LanguageS = p->language;
		m_CorpusS = p->corpus;
		m_TierS = p->code;
		m_About = p->ageAbout;
		if (p->age1y != -1) uS.sprintf(str, cl_T("%d"), p->age1y);
		else str[0] = EOS;
		m_Y1S = str;
		if (p->age1m != -1) uS.sprintf(str, cl_T("%d"), p->age1m);
		else str[0] = EOS;
		m_M1S = str;
		if (p->age1d != -1) uS.sprintf(str, cl_T("%d"), p->age1d);
		else str[0] = EOS;
		m_D1S = str;
		if (p->age2y != -1) uS.sprintf(str, cl_T("%d"), p->age2y);
		else str[0] = EOS;
		m_Y2S = str;
		if (p->age2m != -1) uS.sprintf(str, cl_T("%d"), p->age2m);
		else str[0] = EOS;
		m_M2S = str;
		if (p->age2d != -1) uS.sprintf(str, cl_T("%d"), p->age2d);
		else str[0] = EOS;
		m_D2S = str;
		if (p->sex == 'm' || p->sex == 'M') {
			m_Male = true;
			m_Female = false;
			m_Unknown = false;
		} else if (p->sex == 'f' || p->sex == 'F') {
			m_Male = false;
			m_Female = true;
			m_Unknown = false;
		} else {
			m_Male = false;
			m_Female = false;
			m_Unknown = true;
		}
		m_GroupS = p->group;
		m_SESS = p->SES;
		m_EducationS = p->education;
		m_UFIDS = p->custom_field;
		m_SpNameS = p->spname;
		UpdateData(FALSE);

		if (p->role[0] == EOS)
			m_RoleCtrl.SetCurSel(0);
		else {
			for (item=0; item < m_RoleCtrl.GetCount(); item++) {
				err = m_RoleCtrl.GetLBText(item, str);
				if (err == CB_ERR)
					return;
				if (uS.mStricmp(str, p->role) == 0) {
					m_RoleCtrl.SetCurSel(item);
					break;
				}
			}
		}
	}
}

BOOL CSetIDs::OnInitDialog() {

	CDialog::OnInitDialog();

	UpdateData(TRUE);
	spItem = 0;
	createPopupRoleMenu();
	createPopupSpeakerMenu();
	fillInIDFields();
	m_SpeakerIDC.SetCurSel(spItem);
//	UpdateData(FALSE);

	return TRUE;
}

BOOL CSetIDs::saveIDFields() {
	int		item, err;
	wchar_t	str[IDFIELSSIZE+1];
	char	err_mess[512];
	IDSTYPE *p;

	if (spItem < 0 || spItem >= m_SpeakerIDC.GetCount())
		return(TRUE);
	item = spItem;
	for (p=rootIDs; p != NULL; p=p->next_id) {
		item--;
		if (item < 0)
			 break;
	}
	if (p != NULL) {
		strncpy(p->language, m_LanguageS, IDFIELSSIZE);
		p->language[IDFIELSSIZE] = EOS;
		strncpy(p->corpus, m_CorpusS, IDFIELSSIZE);
		p->corpus[IDFIELSSIZE] = EOS;
		strncpy(p->code, m_TierS, CODESIZE);
		p->code[IDFIELSSIZE] = EOS;
		uS.uppercasestr(p->code, NULL, 0);
		p->ageAbout = m_About;
		strncpy(str, m_Y1S, IDFIELSSIZE);
		str[IDFIELSSIZE] = EOS;
		if (str[0] == EOS) p->age1y = -1;
		else p->age1y = atoi(str);
		strncpy(str, m_M1S, IDFIELSSIZE);
		str[IDFIELSSIZE] = EOS;
		if (str[0] == EOS) p->age1m = -1;
		else p->age1m = atoi(str);
		strncpy(str, m_D1S, IDFIELSSIZE);
		str[IDFIELSSIZE] = EOS;
		if (str[0] == EOS) p->age1d = -1;
		else p->age1d = atoi(str);
		strncpy(str, m_Y2S, IDFIELSSIZE);
		str[IDFIELSSIZE] = EOS;
		if (str[0] == EOS) p->age2y = -1;
		else p->age2y = atoi(str);
		strncpy(str, m_M2S, IDFIELSSIZE);
		str[IDFIELSSIZE] = EOS;
		if (str[0] == EOS) p->age2m = -1;
		else p->age2m = atoi(str);
		strncpy(str, m_D2S, IDFIELSSIZE);
		str[IDFIELSSIZE] = EOS;
		if (str[0] == EOS) p->age2d = -1;
		else p->age2d = atoi(str);
		if (m_Male)
			p->sex = 'm';
		else if (m_Female)
			p->sex = 'f';
		else
			p->sex = 0;
		strncpy(p->group, m_GroupS, IDFIELSSIZE);
		p->group[IDFIELSSIZE] = EOS;
		strncpy(p->SES, m_SESS, IDFIELSSIZE);
		p->SES[IDFIELSSIZE] = EOS;
		item = m_RoleCtrl.GetCurSel();
		if (item == 0) {
			p->role[0] = EOS;
		} else {
			err = m_RoleCtrl.GetLBText(item, str);
			if (err == CB_ERR)
				p->role[0] = EOS;
			else {
				strncpy(p->role, str, IDFIELSSIZE);
				p->role[IDFIELSSIZE] = EOS;
			}
		}
		strncpy(p->education, m_EducationS, IDFIELSSIZE);
		p->education[IDFIELSSIZE] = EOS;
		strncpy(p->custom_field, m_UFIDS, IDFIELSSIZE);
		p->custom_field[IDFIELSSIZE] = EOS;
		strncpy(p->spname, m_SpNameS, IDFIELSSIZE);
		p->spname[IDFIELSSIZE] = EOS;

		if (p->language[0] == EOS) {
			sprintf(err_mess, "Please fill-in \"Language\" field");
			do_warning(err_mess, -1);
			return(FALSE);
		}
		if (p->code[0] == EOS) {
			sprintf(err_mess, "Please fill-in \"Name code\" field");
			do_warning(err_mess, -1);
			return(FALSE);
		}
		if (p->role[0] == EOS) {
			sprintf(err_mess, "Please select a \"Role\"");
			do_warning(err_mess, -1);
			return(FALSE);
		}
		if (p->ageAbout && (p->age1m != -1 || p->age1d != -1 || p->age2m != -1 || p->age2d != -1)) {
			do_warning("if '~' is used, then only year number is allowed", -1);
			return(FALSE);
		}
		if (p->age1m != -1) {
			if (p->age1m < 0 || p->age1m >= 12) {
				sprintf(err_mess, "Illegal month number: %d, please choose 0 - 11", p->age1m);
				do_warning(err_mess, -1);
				return(FALSE);
			}
			if (p->age1y == -1) {
				sprintf(err_mess, "Please specify a year for age", p->age1m);
				do_warning(err_mess, -1);
				return(FALSE);
			}
		}
		if (p->age1d != -1) {
			if (p->age1d < 0 || p->age1d > 31) {
				sprintf(err_mess, "Illegal date number: %d, please choose 1 - 31", p->age1d);
				do_warning(err_mess, -1);
				return(FALSE);
			}
			if (p->age1y == -1) {
				sprintf(err_mess, "Please specify a year for age", p->age1d);
				do_warning(err_mess, -1);
				return(FALSE);
			}
			if (p->age1m == -1) {
				sprintf(err_mess, "Please specify a month for age", p->age1d);
				do_warning(err_mess, -1);
				return(FALSE);
			}
		}
		if (p->age2m != -1) {
			if (p->age2m < 0 || p->age2m >= 12) {
				sprintf(err_mess, "Illegal second month number: %d, please choose 0 - 11", p->age2m);
				do_warning(err_mess, -1);
				return(FALSE);
			}
			if (p->age2y == -1) {
				sprintf(err_mess, "Please specify a second year for age", p->age2m);
				do_warning(err_mess, -1);
				return(FALSE);
			}
		}
		if (p->age2d != -1) {
			if (p->age2d < 0 || p->age2d > 31) {
				sprintf(err_mess, "Illegal second date number: %d, please choose 1 - 31", p->age2d);
				do_warning(err_mess, -1);
				return(FALSE);
			}
			if (p->age2y == -1) {
				sprintf(err_mess, "Please specify a second year for age", p->age2d);
				do_warning(err_mess, -1);
				return(FALSE);
			}
			if (p->age2m == -1) {
				sprintf(err_mess, "Please specify a second month for age", p->age2d);
				do_warning(err_mess, -1);
				return(FALSE);
			}
		}
	}
	return(TRUE);
}

char IDDialog(IDSTYPE **rootIDs, ROLESTYPE *rootRoles) {
	CSetIDs dlg;

	dlg.rootIDs = *rootIDs;
	dlg.rootRoles = rootRoles;
	if (dlg.DoModal() == IDOK) {
		*rootIDs = dlg.rootIDs;
		return(TRUE);
	} else
		return(FALSE);
}
 
void CSetIDs::OnIdsCopy()
{
	UpdateData(TRUE);
	if (saveIDFields()) {
		rootIDs = createNewId(rootIDs, spItem+1);
		spItem = 0;
		createPopupSpeakerMenu();
		fillInIDFields();
		m_SpeakerIDC.SetCurSel(spItem);
	}
//	UpdateData(FALSE);
}

void CSetIDs::OnIdsCreate()
{
	UpdateData(TRUE);
	if (saveIDFields()) {
		rootIDs = createNewId(rootIDs, 0);
		spItem = 0;
		createPopupSpeakerMenu();
		fillInIDFields();
		m_SpeakerIDC.SetCurSel(spItem);
	}
//	UpdateData(FALSE);
}

void CSetIDs::OnIdsDelete()
{
	int i;

	UpdateData(TRUE);
	i = m_SpeakerIDC.GetCount();
	spItem = m_SpeakerIDC.GetCurSel();
	if (i > 1) {
		rootIDs = deleteID(rootIDs, spItem+1);
		spItem = 0;
		createPopupSpeakerMenu();
		fillInIDFields();
		m_SpeakerIDC.SetCurSel(spItem);
	} else {
		saveIDFields();
	}
//	UpdateData(FALSE);
}

void CSetIDs::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CSetIDs::OnBnClickedOk()
{
	UpdateData(TRUE);
	if (saveIDFields())
		OnOK();
//	UpdateData(FALSE);
}


void CSetIDs::OnCbnSelchangeIdsSpeakerid()
{
	int item;

	UpdateData(TRUE);
	item = m_SpeakerIDC.GetCurSel();
	if (item >= 0 && spItem != item) {
		if (saveIDFields()) {
			spItem = item;
			fillInIDFields();
		} else
			m_SpeakerIDC.SetCurSel(spItem);
	}
//	UpdateData(FALSE);
}

void CSetIDs::OnBnClickedIdsUnk()
{
	UpdateData(TRUE);
	m_Male = false;
	m_Female = false;
	UpdateData(FALSE);
}

void CSetIDs::OnBnClickedIdsMale()
{
	UpdateData(TRUE);
	m_Female = false;
	m_Unknown = false;
	UpdateData(FALSE);
}

void CSetIDs::OnBnClickedIdsFemale()
{
	UpdateData(TRUE);
	m_Male = false;
	m_Unknown = false;
	UpdateData(FALSE);
}

void CSetIDs::OnEnChangeIdsCode()
{
	int item;
	wchar_t	str[IDFIELSSIZE+1];
	IDSTYPE *p;

	UpdateData(TRUE);
	strncpy(str, m_TierS, IDFIELSSIZE);
	str[IDFIELSSIZE] = EOS;
	uS.uppercasestr(str, NULL, 0);
	if (spItem < 0 || spItem >= m_SpeakerIDC.GetCount())
		return;
	item = spItem;
	for (p=rootIDs; p != NULL; p=p->next_id) {
		item--;
		if (item < 0)
			 break;
	}
//	UpdateData(FALSE);
	if (p != NULL) {
		strcpy(p->code, str);
		createPopupSpeakerMenu();
		m_SpeakerIDC.SetCurSel(spItem);
	}
}


BOOL CSetIDs::PreTranslateMessage(MSG* pMsg) 
{
	int nStartChar, nEndChar;
	wchar_t str[IDFIELSSIZE+1];

	if (pMsg->message == WM_CHAR && pMsg->wParam == 17/*^W*/) {
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
		return TRUE;
	} else if (pMsg->message == WM_CHAR) {
		if (GetFocus( ) == GetDlgItem(IDC_IDS_LAN)) {
			if (iswalpha(pMsg->wParam) || pMsg->wParam == ',' || pMsg->wParam == '-' || pMsg->wParam < 0x20) {
			} else
				return TRUE;
		} else if (GetFocus( ) == GetDlgItem(IDC_IDS_CODE)) {
			if (iswalpha(pMsg->wParam) || iswdigit(pMsg->wParam) || pMsg->wParam == '-' || pMsg->wParam < 0x20) {
			} else {
				if (pMsg->wParam == '*' || pMsg->wParam == ':')
					do_warning("Please do not include either '*' or ':' character", -1);
				return TRUE;
			}
		} else if (GetFocus() == GetDlgItem(IDC_IDS_Y1)) {
			if (iswdigit(pMsg->wParam) || pMsg->wParam < 0x20) {
				UpdateData(TRUE);
				m_Y1C.GetSel(nStartChar,nEndChar);
				if (nStartChar == nEndChar) {
					strncpy(str, m_Y1S, IDFIELSSIZE);
					str[IDFIELSSIZE] = EOS;
					if (pMsg->wParam >= 0x20 && strlen(str) >= 2)
						return TRUE;
				}
			} else {
				return TRUE;
			}
		} else if (GetFocus() == GetDlgItem(IDC_IDS_M1)) {
			if (iswdigit(pMsg->wParam) || pMsg->wParam < 0x20) {
				UpdateData(TRUE);
				m_M1C.GetSel(nStartChar,nEndChar);
				if (nStartChar == nEndChar) {
					strncpy(str, m_M1S, IDFIELSSIZE);
					str[IDFIELSSIZE] = EOS;
					if (pMsg->wParam >= 0x20 && strlen(str) >= 2)
						return TRUE;
				}
			} else {
				return TRUE;
			}
		} else if (GetFocus() == GetDlgItem(IDC_IDS_D1)) {
			if (iswdigit(pMsg->wParam) || pMsg->wParam < 0x20) {
				UpdateData(TRUE);
				m_D1C.GetSel(nStartChar,nEndChar);
				if (nStartChar == nEndChar) {
					strncpy(str, m_D1S, IDFIELSSIZE);
					str[IDFIELSSIZE] = EOS;
					if (pMsg->wParam >= 0x20 && strlen(str) >= 2)
						return TRUE;
				}
			} else {
				return TRUE;
			}
		} else if (GetFocus() == GetDlgItem(IDC_IDS_Y2)) {
			if (iswdigit(pMsg->wParam) || pMsg->wParam < 0x20) {
				UpdateData(TRUE);
				m_Y2C.GetSel(nStartChar,nEndChar);
				if (nStartChar == nEndChar) {
					strncpy(str, m_Y2S, IDFIELSSIZE);
					str[IDFIELSSIZE] = EOS;
					if (pMsg->wParam >= 0x20 && strlen(str) >= 2)
						return TRUE;
				}
			} else {
				return TRUE;
			}
		} else if (GetFocus() == GetDlgItem(IDC_IDS_M2)) {
			if (iswdigit(pMsg->wParam) || pMsg->wParam < 0x20) {
				UpdateData(TRUE);
				m_M2C.GetSel(nStartChar,nEndChar);
				if (nStartChar == nEndChar) {
					strncpy(str, m_M2S, IDFIELSSIZE);
					str[IDFIELSSIZE] = EOS;
					if (pMsg->wParam >= 0x20 && strlen(str) >= 2)
						return TRUE;
				}
			} else {
				return TRUE;
			}
		} else if (GetFocus() == GetDlgItem(IDC_IDS_D2)) {
			if (iswdigit(pMsg->wParam) || pMsg->wParam < 0x20) {
				UpdateData(TRUE);
				m_D2C.GetSel(nStartChar,nEndChar);
				if (nStartChar == nEndChar) {
					strncpy(str, m_D2S, IDFIELSSIZE);
					str[IDFIELSSIZE] = EOS;
					if (pMsg->wParam >= 0x20 && strlen(str) >= 2)
						return TRUE;
				}
			} else {
				return TRUE;
			}
		} 
	}
	return CDialog::PreTranslateMessage(pMsg);
}
