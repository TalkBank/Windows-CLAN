#include "ced.h"
#include "cu.h"
#include "CedDlgs.h"
#include "w95_commands.h"
#include "w95_cl_eval.h"

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

static char AnomicTp, GlobalTp, BrocaTp, WernickeTp, TranssenTp, TransmotTp,
			ConductionTp, ControlTp, NotAphByWab, MaleOnly, FemaleOnly, SpeechGm,
			StrokeGm, WindowGm, Impevent, Umbrella, Cat, Flood, Cinderella, Sandwich;
static char AgeRange[256];
static 	time_t GlobalTime;
static struct SpeakersListS *spRoot;

void InitEvalOptions(void) {
	AnomicTp = 0;
	GlobalTp = 0;
	BrocaTp = 0;
	WernickeTp = 0;
	TranssenTp = 0;
	TransmotTp = 0;
	ConductionTp = 0;
	ControlTp = 0;
	NotAphByWab = 0;
	MaleOnly = 0;
	FemaleOnly = 0;
	SpeechGm = 0;
	StrokeGm = 0;
	WindowGm = 0;
	Impevent = 0;
	Umbrella = 0;
	Cat = 0;
	Flood = 0;
	Cinderella = 0;
	Sandwich = 0;
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
// CClanEval dialog


CClanEval::CClanEval(CWnd* pParent /*=NULL*/)
	: CDialog(CClanEval::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClanEval)
	m_Reading = _T("");
	//}}AFX_DATA_INIT
}

void CClanEval::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanEval)
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
	DDX_Check(pDX, IDC_PORTF_ANOMIC, m_ANOMIC);
	DDX_Check(pDX, IDC_PORTF_GLOBAL, m_GLOBAL);
	DDX_Check(pDX, IDC_PORTF_CONTROL, m_CONTROL);
	DDX_Check(pDX, IDC_PORTF_BROCA, m_BROCA);
	DDX_Check(pDX, IDC_PORTF_WERNICKE, m_WERNICKE);
	DDX_Check(pDX, IDC_PORTF_CONDUCTION, m_CONDUCTION);
	DDX_Check(pDX, IDC_PORTF_TRANS, m_TRANS);
	DDX_Check(pDX, IDC_PORTF_TRANM, m_TRANM);
	DDX_Check(pDX, IDC_PORTF_NOTAPH, m_NOTAPH);
	DDX_Control(pDX, IDC_PORTF_AGE, m_AGE_RANGECTRL);
	DDX_Text(pDX, IDC_PORTF_AGE, m_AGE_RANGE);
	DDX_Check(pDX, IDC_PORTF_MALE, m_MALE);
	DDX_Check(pDX, IDC_PORTF_FEMALE, m_FEMALE);
	DDX_Check(pDX, IDC_PORTF_SPEECH, m_SPEECH);
	DDX_Check(pDX, IDC_PORTF_STROKE, m_STROKE);
	DDX_Check(pDX, IDC_PORTF_WINDOW, m_WINDOW);
	DDX_Check(pDX, IDC_PORTF_IMPEVENT, m_IMPEVENT);
	DDX_Check(pDX, IDC_PORTF_UMBRELLA, m_UMBRELLA);
	DDX_Check(pDX, IDC_PORTF_CAT, m_CAT);
	DDX_Check(pDX, IDC_PORTF_FLOOD, m_FLOOD);
	DDX_Check(pDX, IDC_PORTF_CINDERELLA, m_CINDERELLA);
	DDX_Check(pDX, IDC_PORTF_SANDWICH, m_SANDWICH);

	DDX_Control(pDX, IDC_SELECT_READING, m_ReadingCTRL);
	DDX_Text(pDX, IDC_SELECT_READING, m_Reading);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClanEval, CDialog)
	//{{AFX_MSG_MAP(CClanEval)
	ON_BN_CLICKED(IDC_PORTF_DDB, OnDDB)
	ON_BN_CLICKED(IDC_PORTF_UDB, OnUDB)
	ON_BN_CLICKED(IDC_PORTF_ANOMIC, OnAnomic)
	ON_BN_CLICKED(IDC_PORTF_GLOBAL, OnGlobal)
	ON_BN_CLICKED(IDC_PORTF_CONTROL, OnControl)
	ON_BN_CLICKED(IDC_PORTF_BROCA, OnBroca)
	ON_BN_CLICKED(IDC_PORTF_WERNICKE, OnWernicke)
	ON_BN_CLICKED(IDC_PORTF_CONDUCTION, OnConduction)
	ON_BN_CLICKED(IDC_PORTF_TRANS, OnTrans)
	ON_BN_CLICKED(IDC_PORTF_TRANM, OnTRANM)
	ON_BN_CLICKED(IDC_PORTF_NOTAPH, OnNotaph)
	ON_BN_CLICKED(IDC_PORTF_MALE, OnMale)
	ON_BN_CLICKED(IDC_PORTF_FEMALE, OnFemale)
	ON_BN_CLICKED(IDC_PORTF_FLUENT, OnFluent)
	ON_BN_CLICKED(IDC_PORTF_NONF, OnNonFluent)
	ON_BN_CLICKED(IDC_PORTF_ALL, OnAllAphasia)
	ON_BN_CLICKED(IDC_PORTF_GEMD, OnDGEM)
	ON_BN_CLICKED(IDC_PORTF_GEMS, OnSGEM)
	ON_BN_CLICKED(IDC_PORTF_SPEECH, OnSpeech)
	ON_BN_CLICKED(IDC_PORTF_STROKE, OnStroke)
	ON_BN_CLICKED(IDC_PORTF_WINDOW, OnWindow)
	ON_BN_CLICKED(IDC_PORTF_IMPEVENT, OnImpEvent)
	ON_BN_CLICKED(IDC_PORTF_UMBRELLA, OnUmbrella)
	ON_BN_CLICKED(IDC_PORTF_CAT, OnCat)
	ON_BN_CLICKED(IDC_PORTF_FLOOD, OnFlood)
	ON_BN_CLICKED(IDC_PORTF_CINDERELLA, OnCinderella)
	ON_BN_CLICKED(IDC_PORTF_SANDWICH, OnSandwich)

	ON_BN_CLICKED(IDC_SELECT_T1, OnSelectT1)
	ON_BN_CLICKED(IDC_SELECT_T2, OnSelectT2)
	ON_BN_CLICKED(IDC_SELECT_T3, OnSelectT3)
	ON_BN_CLICKED(IDC_SELECT_T4, OnSelectT4)
	ON_BN_CLICKED(IDC_SELECT_T5, OnSelectT5)
	ON_BN_CLICKED(IDC_SELECT_T6, OnSelectT6)
	ON_BN_CLICKED(IDC_SELECT_T7, OnSelectT7)
	ON_BN_CLICKED(IDC_SELECT_T8, OnSelectT8)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClanEval message handlers
void CClanEval::OnDDB() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_ANOMIC = 0;
	m_GLOBAL = 0;
	m_BROCA = 0;
	m_WERNICKE = 0;
	m_TRANS = 0;
	m_TRANM = 0;
	m_CONDUCTION = 0;
	m_CONTROL = 0;
	m_NOTAPH = 0;
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

void CClanEval::OnUDB() 
{
	int len;
	char isCancel;
	wchar_t URLPath[128];
	wchar_t fname[FNSize];
	CGetWebPasswd dlg;
	extern bool curlURLDownloadToFile(unCH *fulURLPath, unCH *fname, size_t isProgres);

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	u_strcpy(URLPath, "http://talkbank.org/data/eval_db.txt", 128);
	strcpy(FileName1, wd_dir);
	addFilename2Path(FileName1, "eval_db1.txt");
	u_strcpy(fname, FileName1, UTTLINELEN);
	isCancel = FALSE;
	if (curlURLDownloadToFile(URLPath, fname, 0L) == true) {
		while (isPassNeeded(fname)) {
			dlg.m_Passwd = cl_T("");
			dlg.m_Username = cl_T("");
			if (dlg.DoModal() == IDOK) {
				u_strcpy(URL_passwd, dlg.m_Username, 128);
				strcat(URL_passwd, ":");
				len = strlen(URL_passwd);
				u_strcpy(URL_passwd+len, dlg.m_Passwd, 128-len);
				if (curlURLDownloadToFile(URLPath, fname, 6500000L) != true) {
					do_warning("Error downloading data from the web (2)", 0);
					isCancel = TRUE;
					break;
				}
			} else {
				URL_passwd[0] = EOS;
				isCancel = TRUE;
				break;
			}
		} 
		if (!isCancel) {
			strcpy(FileName2, wd_dir);
			addFilename2Path(FileName2, "eval_db.txt");
			unlink(FileName2);
			if (rename(FileName1,FileName2)) {
			}
		}
	} else {
		do_warning("Error downloading data from the web (2)", 0);
		unlink(FileName1);
	}
	UpdateData(false);
	len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnAnomic() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_ANOMIC = !m_ANOMIC;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnGlobal() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_GLOBAL = !m_GLOBAL;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnControl() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_CONTROL = !m_CONTROL;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnBroca() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_BROCA = !m_BROCA;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnWernicke() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_WERNICKE = !m_WERNICKE;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnConduction() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_CONDUCTION = !m_CONDUCTION;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnTrans() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_TRANS = !m_TRANS;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnTRANM() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_TRANM = !m_TRANM;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnNotaph() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_NOTAPH = !m_NOTAPH;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnMale() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_MALE = !m_MALE;
	if (m_MALE == 1)
		m_FEMALE = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnFemale() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_FEMALE = !m_FEMALE;
	if (m_FEMALE == 1)
		m_MALE = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnFluent() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_ANOMIC = 1;
	m_GLOBAL = 0;
	m_BROCA = 0;
	m_WERNICKE = 1;
	m_TRANS = 1;
	m_TRANM = 0;
	m_CONDUCTION = 1;
	m_CONTROL = 0;
	m_NOTAPH = 1;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnNonFluent() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_ANOMIC = 0;
	m_GLOBAL = 1;
	m_BROCA = 1;
	m_WERNICKE = 0;
	m_TRANS = 0;
	m_TRANM = 1;
	m_CONDUCTION = 0;
	m_CONTROL = 0;
	m_NOTAPH = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnAllAphasia() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_ANOMIC = 1;
	m_GLOBAL = 1;
	m_BROCA = 1;
	m_WERNICKE = 1;
	m_TRANS = 1;
	m_TRANM = 1;
	m_CONDUCTION = 1;
	m_CONTROL = 0;
	m_NOTAPH = 1;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnDGEM() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_SPEECH = 0;
	m_STROKE = 0;
	m_WINDOW = 0;
	m_IMPEVENT = 0;
	m_UMBRELLA = 0;
	m_CAT = 0;
	m_FLOOD = 0;
	m_CINDERELLA = 0;
	m_SANDWICH = 0;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnSGEM() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_SPEECH = 1;
	m_STROKE = 1;
	m_WINDOW = 1;
	m_IMPEVENT = 1;
	m_UMBRELLA = 1;
	m_CAT = 1;
	m_FLOOD = 1;
	m_CINDERELLA = 1;
	m_SANDWICH = 1;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnSpeech() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_SPEECH = !m_SPEECH;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}
void CClanEval::OnStroke() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_STROKE = !m_STROKE;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}
void CClanEval::OnWindow() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_WINDOW = !m_WINDOW;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}
void CClanEval::OnImpEvent() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_IMPEVENT = !m_IMPEVENT;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}
void CClanEval::OnUmbrella() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_UMBRELLA = !m_UMBRELLA;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}
void CClanEval::OnCat() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_CAT = !m_CAT;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}
void CClanEval::OnFlood() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_FLOOD = !m_FLOOD;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}
void CClanEval::OnCinderella() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_CINDERELLA = !m_CINDERELLA;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}
void CClanEval::OnSandwich() 
{
	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_SANDWICH = !m_SANDWICH;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
}

void CClanEval::OnSelectT1() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_T1 = !m_T1;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
	for (p=spRoot; p != NULL; p=p->next_tier) {
		if (p->num == 1) {
			p->val = m_T1;
			return;
		}
	}
}

void CClanEval::OnSelectT2() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_T2 = !m_T2;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
	for (p=spRoot; p != NULL; p=p->next_tier) {
		if (p->num == 2) {
			p->val = m_T2;
			return;
		}
	}
}

void CClanEval::OnSelectT3() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_T3 = !m_T3;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
	for (p=spRoot; p != NULL; p=p->next_tier) {
		if (p->num == 3) {
			p->val = m_T3;
			return;
		}
	}
}

void CClanEval::OnSelectT4() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_T4 = !m_T4;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
	for (p=spRoot; p != NULL; p=p->next_tier) {
		if (p->num == 4) {
			p->val = m_T4;
			return;
		}
	}
}

void CClanEval::OnSelectT5() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_T5 = !m_T5;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
	for (p=spRoot; p != NULL; p=p->next_tier) {
		if (p->num == 5) {
			p->val = m_T5;
			return;
		}
	}
}

void CClanEval::OnSelectT6() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_T6 = !m_T6;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
	for (p=spRoot; p != NULL; p=p->next_tier) {
		if (p->num == 6) {
			p->val = m_T6;
			return;
		}
	}
}

void CClanEval::OnSelectT7() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_T7 = !m_T7;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
	for (p=spRoot; p != NULL; p=p->next_tier) {
		if (p->num == 7) {
			p->val = m_T7;
			return;
		}
	}
}

void CClanEval::OnSelectT8() 
{
	struct SpeakersListS *p;

	UpdateData(true);
	GotoDlgCtrl(GetDlgItem(IDC_PORTF_AGE));
	m_T8 = !m_T8;
	UpdateData(false);
	int len = strlen(m_AGE_RANGE);
	m_AGE_RANGECTRL.SetSel(len, len, false);
	for (p=spRoot; p != NULL; p=p->next_tier) {
		if (p->num == 8) {
			p->val = m_T8;
			return;
		}
	}
}

void CClanEval::getTierNamesFromFile(char *fname) {
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
				spRoot = addToSpArr(spRoot, templineC3);
				isFoundFile = 1;
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
						isFoundFile = 1;
					}
				}
			}
		}
	}
	fclose(fp);
}

void CClanEval::makeSpeakersList() {
	int		i, j, len;
	FILE	*fp;

	SetNewVol(wd_dir);
	for (i=1; i < cl_argc; i++) {
		if (cl_argv[i][0] == '-' || cl_argv[i][0] == '+') {
		} else {
			if (!strcmp(cl_argv[i], "@")) {
				for (j=1; j <= F_numfiles; j++) {
					get_selected_file(j, FileName1);
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
void CClanEval::setNameOfButton(struct SpeakersListS *p, int which, int nCmdShow, BOOL val) {
	if (p != NULL) {
		strcpy(templineC3, p->code);
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
	}
}

BOOL CClanEval::OnInitDialog() {
	int i;

	CDialog::OnInitDialog();
	for (i=1; i <= TOTAL_SP_NUMBER; i++)
		setNameOfButton(NULL, i, SW_HIDE, 0);
	ReadyToLoadState = 1;
	return 0;
}

LRESULT CClanEval::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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

static void selectEvalDialog() {
	char isGender;
	struct SpeakersListS *p;
	CClanEval dlg;

	spRoot = NULL;
	dlg.isFoundFile = 0;
	dlg.ReadyToLoadState = 0;
	u_strcpy(templineW1, AgeRange, UTTLINELEN);
	dlg.m_AGE_RANGE = templineW1;
	dlg.m_ANOMIC = AnomicTp;
	dlg.m_GLOBAL = GlobalTp;
	dlg.m_BROCA = BrocaTp;
	dlg.m_WERNICKE = WernickeTp;
	dlg.m_TRANS = TranssenTp;
	dlg.m_TRANM = TransmotTp;
	dlg.m_CONDUCTION = ConductionTp;
	dlg.m_CONTROL = ControlTp;
	dlg.m_NOTAPH = NotAphByWab;
	dlg.m_MALE = MaleOnly;
	dlg.m_FEMALE = FemaleOnly;
	dlg.m_SPEECH = SpeechGm;
	dlg.m_STROKE = StrokeGm;
	dlg.m_WINDOW = WindowGm;
	dlg.m_IMPEVENT = Impevent;
	dlg.m_UMBRELLA = Umbrella;
	dlg.m_CAT = Cat;
	dlg.m_FLOOD = Flood;
	dlg.m_CINDERELLA = Cinderella;
	dlg.m_SANDWICH = Sandwich;
	if (dlg.DoModal() == IDOK) {
		strcpy(templineC3, clan_name[EVAL]);
		strcat(templineC3, " @");
		for (p=spRoot; p != NULL; p=p->next_tier) {
			if (p->val == 1) {
				strcat(templineC3, " +t");
				strcat(templineC3, p->code);
				strcat(templineC3, ":");
			}
		}
		AnomicTp = dlg.m_ANOMIC;
		GlobalTp = dlg.m_GLOBAL;
		BrocaTp = dlg.m_BROCA;
		WernickeTp = dlg.m_WERNICKE;
		TranssenTp = dlg.m_TRANS;
		TransmotTp = dlg.m_TRANM;
		ConductionTp = dlg.m_CONDUCTION;
		ControlTp = dlg.m_CONTROL;
		NotAphByWab = dlg.m_NOTAPH;
		MaleOnly = dlg.m_MALE;
		FemaleOnly = dlg.m_FEMALE;
		SpeechGm = dlg.m_SPEECH;
		StrokeGm = dlg.m_STROKE;
		WindowGm = dlg.m_WINDOW;
		Impevent = dlg.m_IMPEVENT;
		Umbrella = dlg.m_UMBRELLA;
		Cat = dlg.m_CAT;
		Flood = dlg.m_FLOOD;
		Cinderella = dlg.m_CINDERELLA;
		Sandwich = dlg.m_SANDWICH;
		u_strcpy(AgeRange, dlg.m_AGE_RANGE, 256);
		if (SpeechGm && StrokeGm && WindowGm && Impevent && Umbrella && Cat && Flood && Cinderella && Sandwich) {
			SpeechGm = StrokeGm = WindowGm = Impevent = Umbrella = Cat = Flood = Cinderella = Sandwich = 0;
		}
		if (MaleOnly)
			isGender = 1;
		else if (FemaleOnly)
			isGender = 2;
		else 
			isGender = 0;
		if (AnomicTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Anomic");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (GlobalTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Global");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (BrocaTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Broca");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (WernickeTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Wernicke");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (TranssenTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "TransSensory");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (TransmotTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "TransMotor");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (ConductionTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "Conduction");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (ControlTp) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "control");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (NotAphByWab) {
			strcat(templineC3, " +d\"");
			strcat(templineC3, "NotAphasicByWAB");
			if (AgeRange[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, AgeRange);
			}
			if (isGender == 1) {
				strcat(templineC3, "|");
				strcat(templineC3, "male");
			} else if (isGender == 2) {
				strcat(templineC3, "|");
				strcat(templineC3, "female");
			}
			strcat(templineC3, "\"");
		}
		if (SpeechGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Speech");
			strcat(templineC3, "\"");
		}
		if (StrokeGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Stroke");
			strcat(templineC3, "\"");
		}
		if (WindowGm) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Window");
			strcat(templineC3, "\"");
		}
		if (Impevent) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Important_Event");
			strcat(templineC3, "\"");
		}
		if (Umbrella) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Umbrella");
			strcat(templineC3, "\"");
		}
		if (Cat) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Cat");
			strcat(templineC3, "\"");
		}
		if (Flood) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Flood");
			strcat(templineC3, "\"");
		}
		if (Cinderella) {
			strcat(templineC3, " +g\"");
			strcat(templineC3, "Cinderella");
			strcat(templineC3, "\"");
		}
		if (Sandwich) {
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

void EvalDialog(unCH *str) {
	CClanEval dlg;

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
	selectEvalDialog();
	if (templineC3[0] != EOS) {
		u_strcpy(templine3, templineC3, UTTLINELEN);
		strcpy(str, templine3);
	} else
		str[0] = EOS;
}
