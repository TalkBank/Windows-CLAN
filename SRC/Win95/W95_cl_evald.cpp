#include "ced.h"
#include "cu.h"
#include "CedDlgs.h"
#include "w95_commands.h"
#include "w95_cl_evald.h"

#define DESELECT_DATABASE 30
#define ANOMIC_TYPE		  31
#define GLOBAL_TYPE		  32
#define BROCA_TYPE		  33
#define WERNICKE_TYPE	  34
#define TRANSSEN_TYPE	  35
#define TRANSMOT_TYPE	  36
#define CONDUCTION_TYPE	  37
#define CONTROL_TYPE	  38
#define NOT_APH_BY_WAB	  39
#define FLUENT_TYPE		  40
#define NONFLUENT_TYPE	  41
#define ALLAPHASIA_TYPE	  42
#define AGE_RANGE		  43
#define MALE_ONLY		  44
#define FEMALE_ONLY		  45

#define SPEECH_GEM		  60
#define STROKE_GEM		  61
#define WINDOW_GEM		  62
#define IMPEVENT_GEM	  63
#define UMBRELLA_GEM	  64
#define CAT_GEM			  65
#define FLOOD_GEM		  66
#define CINDERELLA_GEM	  67
#define SANDWICH_GEM	  68
#define COMMUNICATION_GEM 72
#define ILLNESS_GEM		  73
#define DESELECT_GEMS	  69
#define SELECT_ALL_GEMS	  70
#define UPDATE_DB		  71

#define TOTAL_SP_NUMBER 8

#define CODESTRLEN  6
struct SpeakersListS {
	char code[CODESTRLEN+1];
	int  num;
	short val;
	struct SpeakersListS *next_tier;
} ;

extern int  F_numfiles;
extern int  cl_argc;
extern char *cl_argv[];
extern char URL_passwd[];

static char ControlTp, MCITp, MemoryTp, PossibleADTp, ProbableADTp, VascularTp,
			MaleOnly, FemaleOnly,
			CatGm, CookieGm, HometownGm, RockwellGm, CinderellaGm, SandwichGm,
			Communication, Illness;
static char AgeRange[256];
static 	time_t GlobalTime;
static struct SpeakersListS *spRoot;

void InitEvaldOptions(void) {
	ControlTp = 0;
	MCITp = 0;
	MemoryTp = 0;
	PossibleADTp = 0;
	ProbableADTp = 0;
	VascularTp = 0;
	MaleOnly = 0;
	FemaleOnly = 0;
	CatGm = 0;
	CookieGm = 0;
	HometownGm = 0;
	RockwellGm = 0;
	CinderellaGm = 0;
	SandwichGm = 0;
	AgeRange[0] = EOS;
}

static struct SpeakersListS *cleanSpeakersList(struct SpeakersListS *p) {
	struct SpeakersListS *t;

	while (p != NULL) {
		t = p;
		p = p->next_tier;
		free(t);
	}
	return(NULL);
}
static struct SpeakersListS *addToSpArr(struct SpeakersListS *root, char *code) {
	struct SpeakersListS *nt;

	if (root == NULL) {
		root = NEW(struct SpeakersListS);
		if (root == NULL)
			return(root);
		nt = root;
	} else {
		for (nt=root; nt->next_tier != NULL; nt=nt->next_tier) {
			if (uS.mStricmp(nt->code, code) == 0) {
				nt->num++;
				return(root);
			}
		}
		if (uS.mStricmp(nt->code, code) == 0) {
			nt->num++;
			return(root);
		}
		nt->next_tier = NEW(struct SpeakersListS);
		nt = nt->next_tier;
		if (nt == NULL)
			return(root);
	}
	strncpy(nt->code, code, CODESTRLEN);
	nt->code[CODESTRLEN] = EOS;
	nt->num = 1;
	nt->next_tier = NULL;
	return(root);
}
/////////////////////////////////////////////////////////////////////////////
// CClanEvald dialog


CClanEvald::CClanEvald(CWnd* pParent /*=NULL*/)
	: CDialog(CClanEvald::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClanEvald)
	m_Reading = _T("");
	//}}AFX_DATA_INIT
}

void CClanEvald::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanEvald)
	DDX_Control(pDX, IDC_SELECT_TT1, m_T1_CTRL);
	DDX_Check(pDX, IDC_SELECT_TT1, m_T1);
	DDX_Control(pDX, IDC_SELECT_TT2, m_T2_CTRL);
	DDX_Check(pDX, IDC_SELECT_TT2, m_T2);
	DDX_Control(pDX, IDC_SELECT_TT3, m_T3_CTRL);
	DDX_Check(pDX, IDC_SELECT_TT3, m_T3);
	DDX_Control(pDX, IDC_SELECT_TT4, m_T4_CTRL);
	DDX_Check(pDX, IDC_SELECT_TT4, m_T4);
	DDX_Control(pDX, IDC_SELECT_TT5, m_T5_CTRL);
	DDX_Check(pDX, IDC_SELECT_TT5, m_T5);
	DDX_Control(pDX, IDC_SELECT_TT6, m_T6_CTRL);
	DDX_Check(pDX, IDC_SELECT_TT6, m_T6);
	DDX_Control(pDX, IDC_SELECT_TT7, m_T7_CTRL);
	DDX_Check(pDX, IDC_SELECT_TT7, m_T7);
	DDX_Control(pDX, IDC_SELECT_TT8, m_T8_CTRL);
	DDX_Check(pDX, IDC_SELECT_TT8, m_T8);
	DDX_Check(pDX, IDC_EVALD_CONTROL, m_CONTROL);
	DDX_Check(pDX, IDC_EVALD_MCI, m_MCI);
	DDX_Check(pDX, IDC_EVALD_MEMORY, m_MEMORY);
	DDX_Check(pDX, IDC_EVALD_POSSIBLEAD, m_POSSIBLEAD);
	DDX_Check(pDX, IDC_EVALD_PROBABLEAD, m_PROBABLEAD);
	DDX_Check(pDX, IDC_EVALD_VASCULAR, m_VASCULAR);

	DDX_Control(pDX, IDC_EVALD_AGE, m_AGE_RANGECTRL);
	DDX_Text(pDX, IDC_EVALD_AGE, m_AGE_RANGE);
	DDX_Check(pDX, IDC_EVALD_MALE, m_MALE);
	DDX_Check(pDX, IDC_EVALD_FEMALE, m_FEMALE);

	DDX_Check(pDX, IDC_EVALD_CAT, m_CAT);
	DDX_Check(pDX, IDC_EVALD_COOKIE, m_COOKIE);
	DDX_Check(pDX, IDC_EVALD_HOMETOWN, m_HOMETOWN);
	DDX_Check(pDX, IDC_EVALD_ROCKWELL, m_ROCKWELL);
	DDX_Check(pDX, IDC_EVALD_CINDERELLA, m_CINDERELLA);
	DDX_Check(pDX, IDC_EVALD_SANDWICH, m_SANDWICH);

	DDX_Control(pDX, IDC_SELECT_READING, m_ReadingCTRL);
	DDX_Text(pDX, IDC_SELECT_READING, m_Reading);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClanEvald, CDialog)
	//{{AFX_MSG_MAP(CClanEvald)
	ON_BN_CLICKED(IDC_EVALD_DDB, OnDDB)
	ON_BN_CLICKED(IDC_EVALD_CONTROL, OnControl)
	ON_BN_CLICKED(IDC_EVALD_MCI, OnMCI)
	ON_BN_CLICKED(IDC_EVALD_MEMORY, OnMemory)
	ON_BN_CLICKED(IDC_EVALD_POSSIBLEAD, OnPossibleAD)
	ON_BN_CLICKED(IDC_EVALD_PROBABLEAD, OnProbableAD)
	ON_BN_CLICKED(IDC_EVALD_VASCULAR, OnVascular)

	ON_BN_CLICKED(IDC_EVALD_MALE, OnMale)
	ON_BN_CLICKED(IDC_EVALD_FEMALE, OnFemale)

	ON_BN_CLICKED(IDC_EVALD_GEMD, OnDGEM)
	ON_BN_CLICKED(IDC_EVALD_GEMS, OnSGEM)

	ON_BN_CLICKED(IDC_EVALD_CAT, OnCat)
	ON_BN_CLICKED(IDC_EVALD_COOKIE, OnCookie)
	ON_BN_CLICKED(IDC_EVALD_HOMETOWN, OnHometown)
	ON_BN_CLICKED(IDC_EVALD_ROCKWELL, OnRockwell)
	ON_BN_CLICKED(IDC_EVALD_CINDERELLA, OnCinderella)
	ON_BN_CLICKED(IDC_EVALD_SANDWICH, OnSandwich)

	ON_BN_CLICKED(IDC_SELECT_TT1, OnSelectT1)
	ON_BN_CLICKED(IDC_SELECT_TT2, OnSelectT2)
	ON_BN_CLICKED(IDC_SELECT_TT3, OnSelectT3)
	ON_BN_CLICKED(IDC_SELECT_TT4, OnSelectT4)
	ON_BN_CLICKED(IDC_SELECT_TT5, OnSelectT5)
	ON_BN_CLICKED(IDC_SELECT_TT6, OnSelectT6)
	ON_BN_CLICKED(IDC_SELECT_TT7, OnSelectT7)
	ON_BN_CLICKED(IDC_SELECT_TT8, OnSelectT8)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EVALD_AGE, &CClanEvald::OnEnChangeEvaldAge)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClanEvald message handlers
void CClanEvald::OnDDB() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CONTROL = 0;
	m_MCI = 0;
	m_MEMORY = 0;
	m_POSSIBLEAD = 0;
	m_PROBABLEAD = 0;
	m_VASCULAR = 0;
	m_AGE_RANGE = _T("");
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

static char isPassNeeded(unCH *fname) {
	long  i, count;
	FILE *fp;

	if ((fp=_wfopen(fname, cl_T("rb"))) != NULL) {
		count = fread(templineC2, sizeof(char), UTTLINELEN, fp);
		templineC2[count] = EOS;
		if (!ferror(fp)) {
			for (i=0; templineC2[i] != EOS; i++) {
				if (uS.mStrnicmp(templineC2+i, "<TITLE>", 7) == 0) {
					for (i=i+7; isSpace(templineC2[i]); i++) ;
					if (uS.mStrnicmp(templineC2+i, "401 Authorization", 17) == 0) {
						fclose(fp);
						_wunlink(fname);
						return(TRUE);
					}
				}
			}
		}
		fclose(fp);
	}
	return(FALSE);
}


void CClanEvald::OnControl()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CONTROL = !m_CONTROL;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnMCI()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_MCI = !m_MCI;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnMemory()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_MEMORY = !m_MEMORY;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnPossibleAD()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_POSSIBLEAD = !m_POSSIBLEAD;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnProbableAD()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_PROBABLEAD = !m_PROBABLEAD;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnVascular()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_VASCULAR = !m_VASCULAR;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}


void CClanEvald::OnMale() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_MALE = !m_MALE;
	if (m_MALE == 1)
		m_FEMALE = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnFemale() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_FEMALE = !m_FEMALE;
	if (m_FEMALE == 1)
		m_MALE = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnDGEM() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CAT = 0;
	m_COOKIE = 0;
	m_HOMETOWN = 0;
	m_ROCKWELL = 0;
	m_CINDERELLA = 0;
	m_SANDWICH = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSGEM() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CAT = 1;
	m_COOKIE = 1;
	m_HOMETOWN = 1;
	m_ROCKWELL = 1;
	m_CINDERELLA = 1;
	m_SANDWICH = 1;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnCat()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CAT = 1;
	m_COOKIE = 0;
	m_HOMETOWN = 0;
	m_ROCKWELL = 0;
	m_CINDERELLA = 0;
	m_SANDWICH = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnCookie()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CAT = 0;
	m_COOKIE = 1;
	m_HOMETOWN = 0;
	m_ROCKWELL = 0;
	m_CINDERELLA = 0;
	m_SANDWICH = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnHometown()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CAT = 0;
	m_COOKIE = 0;
	m_HOMETOWN = 1;
	m_ROCKWELL = 0;
	m_CINDERELLA = 0;
	m_SANDWICH = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnRockwell()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CAT = 0;
	m_COOKIE = 0;
	m_HOMETOWN = 0;
	m_ROCKWELL = 1;
	m_CINDERELLA = 0;
	m_SANDWICH = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnCinderella()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CAT = 0;
	m_COOKIE = 0;
	m_HOMETOWN = 0;
	m_ROCKWELL = 0;
	m_CINDERELLA = 1;
	m_SANDWICH = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSandwich()
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_CAT = 0;
	m_COOKIE = 0;
	m_HOMETOWN = 0;
	m_ROCKWELL = 0;
	m_CINDERELLA = 0;
	m_SANDWICH = 1;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSelectT1()
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_T1 = 0;
	m_T2 = 0;
	m_T3 = 0;
	m_T4 = 0;
	m_T5 = 0;
	m_T6 = 0;
	m_T7 = 0;
	m_T8 = 0;
	for (p = spRoot; p != NULL; p = p->next_tier) {
		p->val = 0;
	}
	for (p = spRoot; p != NULL; p = p->next_tier) {
		if (p->num == 1) {
			m_T1 = 1;
			p->val = m_T1;
			break;
		}
	}
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSelectT2() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_T1 = 0;
	m_T2 = 0;
	m_T3 = 0;
	m_T4 = 0;
	m_T5 = 0;
	m_T6 = 0;
	m_T7 = 0;
	m_T8 = 0;
	for (p = spRoot; p != NULL; p = p->next_tier) {
		p->val = 0;
	}
	for (p = spRoot; p != NULL; p = p->next_tier) {
		if (p->num == 2) {
			m_T2 = 1;
			p->val = m_T2;
			break;
		}
	}
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSelectT3() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_T1 = 0;
	m_T2 = 0;
	m_T3 = 0;
	m_T4 = 0;
	m_T5 = 0;
	m_T6 = 0;
	m_T7 = 0;
	m_T8 = 0;
	for (p = spRoot; p != NULL; p = p->next_tier) {
		p->val = 0;
	}
	for (p = spRoot; p != NULL; p = p->next_tier) {
		if (p->num == 3) {
			m_T3 = 1;
			p->val = m_T3;
			break;
		}
	}
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSelectT4() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_T1 = 0;
	m_T2 = 0;
	m_T3 = 0;
	m_T4 = 0;
	m_T5 = 0;
	m_T6 = 0;
	m_T7 = 0;
	m_T8 = 0;
	for (p = spRoot; p != NULL; p = p->next_tier) {
		p->val = 0;
	}
	for (p = spRoot; p != NULL; p = p->next_tier) {
		if (p->num == 4) {
			m_T4 = 1;
			p->val = m_T4;
			break;
		}
	}
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSelectT5() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_T1 = 0;
	m_T2 = 0;
	m_T3 = 0;
	m_T4 = 0;
	m_T5 = 0;
	m_T6 = 0;
	m_T7 = 0;
	m_T8 = 0;
	for (p = spRoot; p != NULL; p = p->next_tier) {
		p->val = 0;
	}
	for (p = spRoot; p != NULL; p = p->next_tier) {
		if (p->num == 5) {
			m_T5 = 1;
			p->val = m_T5;
			break;
		}
	}
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSelectT6() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_T1 = 0;
	m_T2 = 0;
	m_T3 = 0;
	m_T4 = 0;
	m_T5 = 0;
	m_T6 = 0;
	m_T7 = 0;
	m_T8 = 0;
	for (p = spRoot; p != NULL; p = p->next_tier) {
		p->val = 0;
	}
	for (p = spRoot; p != NULL; p = p->next_tier) {
		if (p->num == 6) {
			m_T6 = 1;
			p->val = m_T6;
			break;
		}
	}
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSelectT7() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_T1 = 0;
	m_T2 = 0;
	m_T3 = 0;
	m_T4 = 0;
	m_T5 = 0;
	m_T6 = 0;
	m_T7 = 0;
	m_T8 = 0;
	for (p = spRoot; p != NULL; p = p->next_tier) {
		p->val = 0;
	}
	for (p = spRoot; p != NULL; p = p->next_tier) {
		if (p->num == 7) {
			m_T7 = 1;
			p->val = m_T7;
			break;
		}
	}
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::OnSelectT8() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_EVALD_AGE));
	m_T1 = 0;
	m_T2 = 0;
	m_T3 = 0;
	m_T4 = 0;
	m_T5 = 0;
	m_T6 = 0;
	m_T7 = 0;
	m_T8 = 0;
	for (p = spRoot; p != NULL; p = p->next_tier) {
		p->val = 0;
	}
	for (p = spRoot; p != NULL; p = p->next_tier) {
		if (p->num == 8) {
			m_T8 = 1;
			p->val = m_T8;
			break;
		}
	}
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEvald::getTierNamesFromFile(char *fname) {
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
			if (isFoundFile)
				break;
			s = strchr(templineC3, ':');
			if (s != NULL) {
				*s = EOS;
				spRoot = addToSpArr(spRoot, templineC3);
				isFoundFile = TRUE;
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
						spRoot = addToSpArr(spRoot, code);
						isFoundFile = TRUE;
					}
				}
			}
		}
	}
	fclose(fp);
}

void CClanEvald::makeSpeakersList() {
	int		i, j, len;
	FILE	*fp;

	SetNewVol(wd_dir);
	for (i=1; i < cl_argc; i++) {
		if (cl_argv[i][0] == '-' || cl_argv[i][0] == '+') {
		} else {
			if (!strcmp(cl_argv[i], "@")) {
				for (j=1; j <= F_numfiles; j++) {
					get_selected_file(j, FileName1, FNSize);
					getTierNamesFromFile(FileName1);
				}
			} else if (cl_argv[i][0] == '@' && cl_argv[i][1] == ':') {
				uS.str2FNType(FileName1, 0L, cl_argv[i]+2);
				fp = fopen(FileName1, "r");
				if (fp == NULL) {
					return;
				}
				while (fgets_cr(FileName1, 3072, fp)) {
					uS.remFrontAndBackBlanks(FileName1);
					getTierNamesFromFile(FileName1);
				}
				fclose(fp);
			} else if (strchr(cl_argv[i], '*') == NULL) {
				getTierNamesFromFile(cl_argv[i]);
			} else {
				strcpy(DirPathName, wd_dir);
				len = strlen(DirPathName);
				j = 1;
				while ((j=Get_File(FileName1, j)) != 0) {
					if (uS.fIpatmat(FileName1, cl_argv[i])) {
						addFilename2Path(DirPathName, FileName1);
						getTierNamesFromFile(DirPathName);
						DirPathName[len] = EOS;
					}
				}
			}
		}
	}
}

// SW_SHOW
// SW_HIDE
//CButton
//RECT pmargin;
//m_T1_CTRL.GetTextMargin(&pmargin);
void CClanEvald::setNameOfButton(struct SpeakersListS *p, int which, int nCmdShow, BOOL val) {
	if (p != NULL) {
		strcpy(templineC3, p->code);
		u_strcpy(templine3, templineC3, UTTLINELEN);
	}
	if (which == 1) {
		m_T1_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_TT1, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T1 = val;
		}
	} else if (which == 2) {
		m_T2_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_TT2, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T2 = val;
		}
	} else if (which == 3) {
		m_T3_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_TT3, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T3 = val;
		}
	} else if (which == 4) {
		m_T4_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_TT4, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T4 = val;
		}
	} else if (which == 5) {
		m_T5_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_TT5, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T5 = val;
		}
	} else if (which == 6) {
		m_T6_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_TT6, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T6 = val;
		}
	} else if (which == 7) {
		m_T7_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_TT7, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T7 = val;
		}
	} else if (which == 8) {
		m_T8_CTRL.ShowWindow(nCmdShow);
		if (p != NULL) {
			SetDlgItemText(IDC_SELECT_TT8, templine3);
		}
		if (nCmdShow == SW_SHOW) {
			m_T8 = val;
		}
	}
}

BOOL CClanEvald::OnInitDialog() {
	int i;

	CDialog::OnInitDialog();
	for (i=1; i <= TOTAL_SP_NUMBER; i++)
		setNameOfButton(NULL, i, SW_HIDE, 0);
	ReadyToLoadState = 1;
	return 0;
}

LRESULT CClanEvald::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	int i;
	struct SpeakersListS *p;
	LRESULT res = 0;

	if (message == WM_PAINT && ReadyToLoadState == 1) {
		ReadyToLoadState = 2;
		m_ReadingCTRL.ShowWindow(SW_SHOW);
		GlobalTime = 0;
		makeSpeakersList();
		m_ReadingCTRL.ShowWindow(SW_HIDE);
		i = 1;
		for (p=spRoot; p != NULL; p=p->next_tier) {
			if (i <= TOTAL_SP_NUMBER) {
				if (uS.mStricmp(p->code, "*PAR") == 0) {
					p->val = 1;
				} else {
					p->val = 0;
				}
				setNameOfButton(p, i, SW_SHOW, p->val);
				p->num = i;
			} else
				p->num = 0;
			i++;
		}
		UpdateData(false);	
	}
	res = CDialog ::WindowProc(message, wParam, lParam);
	return(res);
}

static void selectEvaldDialog() {
	char isGender;
	struct SpeakersListS *p;
	CClanEvald dlg;

	spRoot = NULL;
	dlg.isFoundFile = FALSE;
	dlg.ReadyToLoadState = 0;
	u_strcpy(templineW1, AgeRange, UTTLINELEN);
	dlg.m_AGE_RANGE = templineW1;

	dlg.m_CONTROL = ControlTp;
	dlg.m_MCI = MCITp;
	dlg.m_MEMORY = MemoryTp;
	dlg.m_POSSIBLEAD = PossibleADTp;
	dlg.m_PROBABLEAD = ProbableADTp;
	dlg.m_VASCULAR = VascularTp;

	dlg.m_MALE = MaleOnly;
	dlg.m_FEMALE = FemaleOnly;

	dlg.m_CAT = CatGm;
	dlg.m_COOKIE = CookieGm;
	dlg.m_HOMETOWN = HometownGm;
	dlg.m_ROCKWELL = RockwellGm;
	dlg.m_CINDERELLA = CinderellaGm;
	dlg.m_SANDWICH = SandwichGm;

	if (dlg.DoModal() == IDOK) {
		strcpy(templineC3, clan_name[EVALD]);
		strcat(templineC3, " @");
		for (p=spRoot; p != NULL; p=p->next_tier) {
			if (p->val == 1) {
				strcat(templineC3, " +t");
				strcat(templineC3, p->code);
				strcat(templineC3, ":");
			}
		}
		ControlTp = dlg.m_CONTROL;
		MCITp = dlg.m_MCI;
		MemoryTp = dlg.m_MEMORY;
		PossibleADTp = dlg.m_POSSIBLEAD;
		ProbableADTp = dlg.m_PROBABLEAD;
		VascularTp = dlg.m_VASCULAR;

		MaleOnly = dlg.m_MALE;
		FemaleOnly = dlg.m_FEMALE;

		CatGm = dlg.m_CAT;
		CookieGm = dlg.m_COOKIE;
		HometownGm = dlg.m_HOMETOWN;
		RockwellGm = dlg.m_ROCKWELL;
		CinderellaGm = dlg.m_CINDERELLA;
		SandwichGm = dlg.m_SANDWICH;

		u_strcpy(AgeRange, dlg.m_AGE_RANGE, 256);
		if (CatGm && CookieGm && HometownGm && RockwellGm && CinderellaGm && SandwichGm) {
			CatGm = CookieGm = HometownGm = RockwellGm = CinderellaGm = SandwichGm = 0;
			Communication = Illness = 0;
		}
		if (MaleOnly)
			isGender = 1;
		else if (FemaleOnly)
			isGender = 2;
		else 
			isGender = 0;

		if (ControlTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Control");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1)
				strcat(templineC3, "|male");
			else if (isGender == 2)
				strcat(templineC3, "|female");
			strcat(templineC3, "\"");
		}
		if (MCITp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "MCI");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1)
				strcat(templineC3, "|male");
			else if (isGender == 2)
				strcat(templineC3, "|female");
			strcat(templineC3, "\"");
		}
		if (MemoryTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Memory");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1)
				strcat(templineC3, "|male");
			else if (isGender == 2)
				strcat(templineC3, "|female");
			strcat(templineC3, "\"");
		}
		if (PossibleADTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "PossibleAD");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1)
				strcat(templineC3, "|male");
			else if (isGender == 2)
				strcat(templineC3, "|female");
			strcat(templineC3, "\"");
		}
		if (ProbableADTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "ProbableAD");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1)
				strcat(templineC3, "|male");
			else if (isGender == 2)
				strcat(templineC3, "|female");
			strcat(templineC3, "\"");
		}
		if (VascularTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Vascular");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1)
				strcat(templineC3, "|male");
			else if (isGender == 2)
				strcat(templineC3, "|female");
			strcat(templineC3, "\"");
		}

		if (CatGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Cat");
			strcat(templineC3, "\"");
		}
		if (CookieGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Cookie");
			strcat(templineC3, "\"");
		}
		if (HometownGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Hometown");
			strcat(templineC3, "\"");
		}
		if (RockwellGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Rockwell");
			strcat(templineC3, "\"");
		}
		if (CinderellaGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Cinderella");
			strcat(templineC3, "\"");
		}
		if (SandwichGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Sandwich");
			strcat(templineC3, "\"");
		}

		strcat(templineC3, " +u");
	} else {
		templineC3[0] = EOS;
	}
	spRoot = cleanSpeakersList(spRoot);
}

void EvaldDialog(unCH *str) {
	CClanEvald dlg;

	spRoot = NULL;
	myget();
	if (F_numfiles <= 0) {
		do_warning("ERROR eval. Please specify input data files first", 0);
		str[0] = EOS;
		return;
	}
	cl_argc = 1;
	strcpy(templineC2, " @");
	if (!MakeArgs(templineC2)) {
		str[0] = EOS;
		return;
	}
	selectEvaldDialog();
	if (templineC3[0] != EOS) {
		u_strcpy(templine3, templineC3, UTTLINELEN);
		strcpy(str, templine3);
	} else
		str[0] = EOS;
}


void CClanEvald::OnEnChangeEvaldAge()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
