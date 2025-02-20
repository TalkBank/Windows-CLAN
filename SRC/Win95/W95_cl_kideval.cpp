#include "ced.h"
#include "cu.h"
#include "CedDlgs.h"
#include "w95_commands.h"
#include "w95_cl_kideval.h"

/*
18-23	1;6 - 1;11
24-29	2;  - 2;5
30-35	2;6 - 2;11
36-41	3;  - 3;5
42-47	3;6 - 3;11
48-53	4;  - 4;5
54-59	4;6 - 4;11
60-65	5;  - 5;5
66-72	5;6 - 6;
*/

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

#define CODESTRLEN  6

extern int  F_numfiles;
extern int  cl_argc;
extern char *cl_argv[];
extern char URL_passwd[];

static BOOL DBEngToyplayTp, DBEngNarrativeTp, DBEngUToyplayTp, DBEngUNarrativeTp, DBZhoToyplayTp, DBZhoNarrativeTp, DBNldToyplayTp,
	DBFraToyplayTp, DBFraNarrativeTp, DBJpnToyplayTp, DBSpaToyplayTp, DBSpaNarrativeTp;
static BOOL LinkAgeTp, CmpDBTp, NotCmpDBTp;
static BOOL IndyAgeTp, OneHTp, TwoTp, TwoHTp, ThreeTp, ThreeHTp, FourTp, FourHTp, FiveTp, FiveHTp, MaleOnly, FemaleOnly, BothGen;
static 	time_t GlobalTime;
char spCode[TOTAL_SP_NUMBER][CODESTRLEN + 1];
BOOL spSet[TOTAL_SP_NUMBER];

void InitKidevalOptions(void) {
	DBEngToyplayTp = TRUE;
	DBEngNarrativeTp = FALSE;
	DBEngUToyplayTp = FALSE;
	DBEngUNarrativeTp = FALSE;
	DBZhoToyplayTp = FALSE;
	DBZhoNarrativeTp = FALSE;
	DBNldToyplayTp = FALSE;
	DBFraToyplayTp = FALSE;
	DBFraNarrativeTp = FALSE;
	DBJpnToyplayTp = FALSE;
	DBSpaToyplayTp = FALSE;
	DBSpaNarrativeTp = FALSE;
	CmpDBTp = FALSE;
	NotCmpDBTp = FALSE;
	IndyAgeTp = FALSE;
	OneHTp = FALSE;
	TwoTp = FALSE;
	TwoHTp = FALSE;
	ThreeTp = FALSE;
	ThreeHTp = FALSE;
	FourTp = FALSE;
	FourHTp = FALSE;
	FiveTp = FALSE;
	FiveHTp = FALSE;
	MaleOnly = FALSE;
	FemaleOnly = FALSE;
	BothGen = TRUE;
	LinkAgeTp = TRUE;
}

static void addToSpArr(char *code) {
	int i;

	for (i=0; i < TOTAL_SP_NUMBER; i++) {
		if (spCode[i][0] == EOS)
			break;
		if (uS.mStricmp(spCode[i], code) == 0) {
			return;
		}
	}
	if (i < TOTAL_SP_NUMBER) {
		strncpy(spCode[i], code, CODESTRLEN);
		spCode[i][CODESTRLEN] = EOS;
	}
}
/////////////////////////////////////////////////////////////////////////////
// CClanKideval dialog


CClanKideval::CClanKideval(CWnd* pParent /*=NULL*/)
	: CDialog(CClanKideval::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClanKideval)
	m_Reading = _T("");
	//}}AFX_DATA_INIT
}

void CClanKideval::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanKideval)
	DDX_Control(pDX, IDC_SELECT_READING, m_ReadingCTRL);
	DDX_Text(pDX, IDC_SELECT_READING, m_Reading);

	DDX_Control(pDX, IDC_STAT_CHOOSE_ONE, m_ChooseOneCTRL);

	DDX_Check(pDX, IDC_SELECT_CMP_DB, m_CmpDB);
	DDX_Check(pDX, IDC_SELECT_NOT_CMP_DB, m_NotCmpDB);

	DDX_Control(pDX, IDC_STAT_SEL_SP, m_SelSpCTRL);
	DDX_Control(pDX, IDC_STAT_SP, m_SpCTRL);
	DDX_Control(pDX, IDC_STAT_AGE, m_AgeCTRL);
	DDX_Control(pDX, IDC_STAT_LANG, m_LangCTRL);
	DDX_Control(pDX, IDC_STAT_DBTYPE, m_DbTypeCTRL);

	DDX_Control(pDX, IDC_ERROR_MES_ONE, m_ErrorMesOneCTRL);
	DDX_Control(pDX, IDC_ERROR_MES_TWO, m_ErrorMesTwoCTRL);
	DDX_Control(pDX, IDC_ERROR_MES_THR, m_ErrorMesThrCTRL);

	DDX_Control(pDX, IDC_SELECT_SP1, m_SP_CTRL[0]);
	DDX_Check(pDX, IDC_SELECT_SP1, m_SP[0]);
	DDX_Control(pDX, IDC_SELECT_SP2, m_SP_CTRL[1]);
	DDX_Check(pDX, IDC_SELECT_SP2, m_SP[1]);
	DDX_Control(pDX, IDC_SELECT_SP3, m_SP_CTRL[2]);
	DDX_Check(pDX, IDC_SELECT_SP3, m_SP[2]);
	DDX_Control(pDX, IDC_SELECT_SP4, m_SP_CTRL[3]);
	DDX_Check(pDX, IDC_SELECT_SP4, m_SP[3]);
	DDX_Control(pDX, IDC_SELECT_SP5, m_SP_CTRL[4]);
	DDX_Check(pDX, IDC_SELECT_SP5, m_SP[4]);
	DDX_Control(pDX, IDC_SELECT_SP6, m_SP_CTRL[5]);
	DDX_Check(pDX, IDC_SELECT_SP6, m_SP[5]);
	DDX_Control(pDX, IDC_SELECT_SP7, m_SP_CTRL[6]);
	DDX_Check(pDX, IDC_SELECT_SP7, m_SP[6]);
	DDX_Control(pDX, IDC_SELECT_SP8, m_SP_CTRL[7]);
	DDX_Check(pDX, IDC_SELECT_SP8, m_SP[7]);

	DDX_Control(pDX, IDC_SELECT_ENG, m_ENG_CTRL);
	DDX_Check(pDX, IDC_SELECT_ENG, m_ENG);
	DDX_Control(pDX, IDC_SELECT_ENGU, m_ENGU_CTRL);
	DDX_Check(pDX, IDC_SELECT_ENGU, m_ENGU);
	DDX_Control(pDX, IDC_SELECT_FRA, m_FRA_CTRL);
	DDX_Check(pDX, IDC_SELECT_FRA, m_FRA);
	DDX_Control(pDX, IDC_SELECT_SPA, m_SPA_CTRL);
	DDX_Check(pDX, IDC_SELECT_SPA, m_SPA);
	DDX_Control(pDX, IDC_SELECT_JPN, m_JPN_CTRL);
	DDX_Check(pDX, IDC_SELECT_JPN, m_JPN);
	DDX_Control(pDX, IDC_SELECT_YUE, m_YUE_CTRL);
	DDX_Check(pDX, IDC_SELECT_YUE, m_YUE);
	DDX_Control(pDX, IDC_SELECT_ZHO, m_ZHO_CTRL);
	DDX_Check(pDX, IDC_SELECT_ZHO, m_ZHO);

	DDX_Control(pDX, IDC_SELECT_INDY_AGE, m_IndyAge_CTRL);
	DDX_Check(pDX, IDC_SELECT_INDY_AGE, m_IndyAge);

	DDX_Control(pDX, IDC_SELECT_OneH, m_OneH_CTRL);
	DDX_Check(pDX, IDC_SELECT_OneH, m_OneH);
	DDX_Control(pDX, IDC_SELECT_Two, m_Two_CTRL);
	DDX_Check(pDX, IDC_SELECT_Two, m_Two);
	DDX_Control(pDX, IDC_SELECT_TwoH, m_TwoH_CTRL);
	DDX_Check(pDX, IDC_SELECT_TwoH, m_TwoH);
	DDX_Control(pDX, IDC_SELECT_Three, m_Three_CTRL);
	DDX_Check(pDX, IDC_SELECT_Three, m_Three);
	DDX_Control(pDX, IDC_SELECT_ThreeH, m_ThreeH_CTRL);
	DDX_Check(pDX, IDC_SELECT_ThreeH, m_ThreeH);
	DDX_Control(pDX, IDC_SELECT_Four, m_Four_CTRL);
	DDX_Check(pDX, IDC_SELECT_Four, m_Four);
	DDX_Control(pDX, IDC_SELECT_FourH, m_FourH_CTRL);
	DDX_Check(pDX, IDC_SELECT_FourH, m_FourH);
	DDX_Control(pDX, IDC_SELECT_Five, m_Five_CTRL);
	DDX_Check(pDX, IDC_SELECT_Five, m_Five);
	DDX_Control(pDX, IDC_SELECT_FiveH, m_FiveH_CTRL);
	DDX_Check(pDX, IDC_SELECT_FiveH, m_FiveH);

	DDX_Control(pDX, IDC_SELECT_MALE, m_MALE_CTRL);
	DDX_Check(pDX, IDC_SELECT_MALE, m_MALE);
	DDX_Control(pDX, IDC_SELECT_FEMALE, m_FEMALE_CTRL);
	DDX_Check(pDX, IDC_SELECT_FEMALE, m_FEMALE);
	DDX_Control(pDX, IDC_SELECT_BOTHGEN, m_BOTHGEN_CTRL);
	DDX_Check(pDX, IDC_SELECT_BOTHGEN, m_BOTHGEN);


	DDX_Control(pDX, IDC_SELECT_ENGTOYPLAY, m_DBEngToyplay_CTRL);
	DDX_Check(pDX, IDC_SELECT_ENGTOYPLAY, m_DBEngToyplay);
	DDX_Control(pDX, IDC_SELECT_ENGNARRATIVE, m_DBEngNarrative_CTRL);
	DDX_Check(pDX, IDC_SELECT_ENGNARRATIVE, m_DBEngNarrative);

	DDX_Control(pDX, IDC_SELECT_ENGUTOYPLAY, m_DBEngUToyplay_CTRL);
	DDX_Check(pDX, IDC_SELECT_ENGUTOYPLAY, m_DBEngUToyplay);
	DDX_Control(pDX, IDC_SELECT_ENGUNARRATIVE, m_DBEngUNarrative_CTRL);
	DDX_Check(pDX, IDC_SELECT_ENGUNARRATIVE, m_DBEngUNarrative);

	DDX_Control(pDX, IDC_SELECT_ZHOTOYPLAY, m_DBZhoToyplay_CTRL);
	DDX_Check(pDX, IDC_SELECT_ZHOTOYPLAY, m_DBZhoToyplay);
	DDX_Control(pDX, IDC_SELECT_ZHONARRATIVE, m_DBZhoNarrative_CTRL);
	DDX_Check(pDX, IDC_SELECT_ZHONARRATIVE, m_DBZhoNarrative);

	DDX_Control(pDX, IDC_SELECT_NLDTOYPLAY, m_DBNldToyplay_CTRL);
	DDX_Check(pDX, IDC_SELECT_NLDTOYPLAY, m_DBNldToyplay);

	DDX_Control(pDX, IDC_SELECT_FRATOYPLAY, m_DBFraToyplay_CTRL);
	DDX_Check(pDX, IDC_SELECT_FRATOYPLAY, m_DBFraToyplay);
	DDX_Control(pDX, IDC_SELECT_FRANARRATIVE, m_DBFraNarrative_CTRL);
	DDX_Check(pDX, IDC_SELECT_FRANARRATIVE, m_DBFraNarrative);

	DDX_Control(pDX, IDC_SELECT_JPNTOYPLAY, m_DBJpnToyplay_CTRL);
	DDX_Check(pDX, IDC_SELECT_JPNTOYPLAY, m_DBJpnToyplay);

	DDX_Control(pDX, IDC_SELECT_SPATOYPLAY, m_DBSpaToyplay_CTRL);
	DDX_Check(pDX, IDC_SELECT_SPATOYPLAY, m_DBSpaToyplay);
	DDX_Control(pDX, IDC_SELECT_SPANARRATIVE, m_DBSpaNarrative_CTRL);
	DDX_Check(pDX, IDC_SELECT_SPANARRATIVE, m_DBSpaNarrative);

	DDX_Control(pDX, IDOK, m_OK_CTRL);
	DDX_Control(pDX, IDCANCEL, m_CANCEL_CTRL);

		//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClanKideval, CDialog)
	//{{AFX_MSG_MAP(CClanKideval)
	ON_BN_CLICKED(IDC_SELECT_CMP_DB, OnCmpDB)
	ON_BN_CLICKED(IDC_SELECT_NOT_CMP_DB, OnNotCmpDB)

	ON_BN_CLICKED(IDC_SELECT_SP1, OnSelectSP1)
	ON_BN_CLICKED(IDC_SELECT_SP2, OnSelectSP2)
	ON_BN_CLICKED(IDC_SELECT_SP3, OnSelectSP3)
	ON_BN_CLICKED(IDC_SELECT_SP4, OnSelectSP4)
	ON_BN_CLICKED(IDC_SELECT_SP5, OnSelectSP5)
	ON_BN_CLICKED(IDC_SELECT_SP6, OnSelectSP6)
	ON_BN_CLICKED(IDC_SELECT_SP7, OnSelectSP7)
	ON_BN_CLICKED(IDC_SELECT_SP8, OnSelectSP8)

	ON_BN_CLICKED(IDC_SELECT_ENG, OnSelectENG)
	ON_BN_CLICKED(IDC_SELECT_ENGU, OnSelectENGU)
	ON_BN_CLICKED(IDC_SELECT_FRA, OnSelectFRA)
	ON_BN_CLICKED(IDC_SELECT_SPA, OnSelectSPA)
	ON_BN_CLICKED(IDC_SELECT_JPN, OnSelectJPN)
	ON_BN_CLICKED(IDC_SELECT_YUE, OnSelectYUE)
	ON_BN_CLICKED(IDC_SELECT_ZHO, OnSelectZHO)
	
	ON_BN_CLICKED(IDC_SELECT_INDY_AGE, OnIndyAge)

	ON_BN_CLICKED(IDC_SELECT_OneH, OnOneH)
	ON_BN_CLICKED(IDC_SELECT_Two, OnTwo)
	ON_BN_CLICKED(IDC_SELECT_TwoH, OnTwoH)
	ON_BN_CLICKED(IDC_SELECT_Three, OnThree)
	ON_BN_CLICKED(IDC_SELECT_ThreeH, OnThreeH)
	ON_BN_CLICKED(IDC_SELECT_Four, OnFour)
	ON_BN_CLICKED(IDC_SELECT_FourH, OnFourH)
	ON_BN_CLICKED(IDC_SELECT_Five, OnFive)
	ON_BN_CLICKED(IDC_SELECT_FiveH, OnFiveH)

	ON_BN_CLICKED(IDC_SELECT_MALE, OnMale)
	ON_BN_CLICKED(IDC_SELECT_FEMALE, OnFemale)
	ON_BN_CLICKED(IDC_SELECT_BOTHGEN, OnBothGen)
	
	ON_BN_CLICKED(IDC_SELECT_ENGTOYPLAY, OnDBEngToyplay)
	ON_BN_CLICKED(IDC_SELECT_ENGNARRATIVE, OnDBEngNarrative)
	ON_BN_CLICKED(IDC_SELECT_ENGUTOYPLAY, OnDBEngUToyplay)
	ON_BN_CLICKED(IDC_SELECT_ENGUNARRATIVE, OnDBEngUNarrative)
	ON_BN_CLICKED(IDC_SELECT_ZHOTOYPLAY, OnDBZhoToyplay)
	ON_BN_CLICKED(IDC_SELECT_ZHONARRATIVE, OnDBZhoNarrative)
	ON_BN_CLICKED(IDC_SELECT_NLDTOYPLAY, OnDBNldToyplay)
	ON_BN_CLICKED(IDC_SELECT_FRATOYPLAY, OnDBFraToyplay)
	ON_BN_CLICKED(IDC_SELECT_FRANARRATIVE, OnDBFraNarrative)
	ON_BN_CLICKED(IDC_SELECT_JPNTOYPLAY, OnDBJpnToyplay)
	ON_BN_CLICKED(IDC_SELECT_SPATOYPLAY, OnDBSpaToyplay)
	ON_BN_CLICKED(IDC_SELECT_SPANARRATIVE, OnDBSpaNarrative)

	ON_BN_CLICKED(IDOK, OnOKClicked)


	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClanKideval message handlers

void CClanKideval::ResizeOptionsWindow() {
	int  i;
	RECT delta, client, parent;
	long wHeight, wLeft, parent_client_offsety;
	CWnd*pw;

//	UpdateData(TRUE);
	m_ReadingCTRL.ShowWindow(SW_HIDE);

	if (m_CmpDB || m_NotCmpDB)
		m_ChooseOneCTRL.ShowWindow(SW_HIDE);
	else
		m_ChooseOneCTRL.ShowWindow(SW_SHOW);

	GetClientRect(&client);
	if (client.top < 0) {
		client.bottom = client.bottom - client.top;
		client.top = 0;
	}
	if (client.left < 0) {
		client.right = client.right - client.left;
		client.left = 0;
	}
	GetWindowRect(&parent);
	parent_client_offsety = (parent.bottom - parent.top) - (client.bottom - client.top);
	if (parent_client_offsety < 0)
		parent_client_offsety = (client.bottom - client.top) - (parent.bottom - parent.top);

	wHeight = client.top + 5;

	if (m_CmpDB) {
		m_SelSpCTRL.ShowWindow(SW_HIDE);
		m_SpCTRL.ShowWindow(SW_HIDE);
		m_LangCTRL.ShowWindow(SW_HIDE);
		m_DbTypeCTRL.ShowWindow(SW_SHOW);

		m_ErrorMesOneCTRL.ShowWindow(SW_SHOW);
		m_ErrorMesTwoCTRL.ShowWindow(SW_SHOW);
		m_ErrorMesThrCTRL.ShowWindow(SW_SHOW);

		for (i = 0; i < TOTAL_SP_NUMBER; i++)
			m_SP_CTRL[i].ShowWindow(SW_HIDE);

		m_ENG_CTRL.ShowWindow(SW_HIDE);
		m_ENGU_CTRL.ShowWindow(SW_HIDE);
		m_FRA_CTRL.ShowWindow(SW_HIDE);
		m_SPA_CTRL.ShowWindow(SW_HIDE);
		m_JPN_CTRL.ShowWindow(SW_HIDE);
		m_YUE_CTRL.ShowWindow(SW_HIDE);
		m_ZHO_CTRL.ShowWindow(SW_HIDE);

		m_IndyAge_CTRL.ShowWindow(SW_SHOW);

		if (m_IndyAge) {
			m_AgeCTRL.ShowWindow(SW_SHOW);
			m_OneH_CTRL.ShowWindow(SW_SHOW);
			m_Two_CTRL.ShowWindow(SW_SHOW);
			m_TwoH_CTRL.ShowWindow(SW_SHOW);
			m_Three_CTRL.ShowWindow(SW_SHOW);
			m_ThreeH_CTRL.ShowWindow(SW_SHOW);
			m_Four_CTRL.ShowWindow(SW_SHOW);
			m_FourH_CTRL.ShowWindow(SW_SHOW);
			m_Five_CTRL.ShowWindow(SW_SHOW);
			m_FiveH_CTRL.ShowWindow(SW_SHOW);
		} else {
			m_AgeCTRL.ShowWindow(SW_HIDE);
			m_OneH_CTRL.ShowWindow(SW_HIDE);
			m_Two_CTRL.ShowWindow(SW_HIDE);
			m_TwoH_CTRL.ShowWindow(SW_HIDE);
			m_Three_CTRL.ShowWindow(SW_HIDE);
			m_ThreeH_CTRL.ShowWindow(SW_HIDE);
			m_Four_CTRL.ShowWindow(SW_HIDE);
			m_FourH_CTRL.ShowWindow(SW_HIDE);
			m_Five_CTRL.ShowWindow(SW_HIDE);
			m_FiveH_CTRL.ShowWindow(SW_HIDE);
		}

		m_MALE_CTRL.ShowWindow(SW_SHOW);
		m_FEMALE_CTRL.ShowWindow(SW_SHOW);
		m_BOTHGEN_CTRL.ShowWindow(SW_SHOW);

		m_DBEngToyplay_CTRL.ShowWindow(SW_SHOW);
		m_DBEngNarrative_CTRL.ShowWindow(SW_SHOW);
		m_DBEngUToyplay_CTRL.ShowWindow(SW_SHOW);
		m_DBEngUNarrative_CTRL.ShowWindow(SW_SHOW);
		m_DBZhoToyplay_CTRL.ShowWindow(SW_SHOW);
		m_DBZhoNarrative_CTRL.ShowWindow(SW_SHOW);
		m_DBNldToyplay_CTRL.ShowWindow(SW_SHOW);
		m_DBFraToyplay_CTRL.ShowWindow(SW_SHOW);
		m_DBFraNarrative_CTRL.ShowWindow(SW_SHOW);
		m_DBJpnToyplay_CTRL.ShowWindow(SW_SHOW);
		m_DBSpaToyplay_CTRL.ShowWindow(SW_SHOW);
		m_DBSpaNarrative_CTRL.ShowWindow(SW_SHOW);

		m_OK_CTRL.ShowWindow(SW_SHOW);

		pw = GetDlgItem(IDC_STAT_CHOOSE_ONE);
		pw->GetWindowRect(&delta);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;

		pw = GetDlgItem(IDC_SELECT_NOT_CMP_DB);
		pw->GetWindowRect(&delta);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;

		wHeight = wHeight + 10;

		wLeft = 8;
		pw = GetDlgItem(IDC_STAT_DBTYPE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 8;


		wLeft = 8;
		pw = GetDlgItem(IDC_ERROR_MES_ONE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top);

		wLeft = 8;
		pw = GetDlgItem(IDC_ERROR_MES_TWO);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top);

		wLeft = 8;
		pw = GetDlgItem(IDC_ERROR_MES_THR);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 8;

		wLeft = 29;
		pw = GetDlgItem(IDC_SELECT_ENGTOYPLAY);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);

		wLeft += client.right + 5;
		pw = GetDlgItem(IDC_SELECT_ENGNARRATIVE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 2;

		wLeft = 29;
		pw = GetDlgItem(IDC_SELECT_ENGUTOYPLAY);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);

		wLeft += client.right + 5;
		pw = GetDlgItem(IDC_SELECT_ENGUNARRATIVE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 2;

		wLeft = 29;
		pw = GetDlgItem(IDC_SELECT_ZHOTOYPLAY);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);

		wLeft += client.right + 5;
		pw = GetDlgItem(IDC_SELECT_ZHONARRATIVE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 2;

		wLeft = 29;
		pw = GetDlgItem(IDC_SELECT_NLDTOYPLAY);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 2;

		wLeft = 29;
		pw = GetDlgItem(IDC_SELECT_FRATOYPLAY);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);

		wLeft += client.right + 5;
		pw = GetDlgItem(IDC_SELECT_FRANARRATIVE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 2;

		wLeft = 29;
		pw = GetDlgItem(IDC_SELECT_JPNTOYPLAY);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 2;

		wLeft = 29;
		pw = GetDlgItem(IDC_SELECT_SPATOYPLAY);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);

		wLeft += client.right + 5;
		pw = GetDlgItem(IDC_SELECT_SPANARRATIVE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 10;

		wLeft = 18;
		pw = GetDlgItem(IDC_SELECT_MALE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_FEMALE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_BOTHGEN);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 10;

		wLeft = 8;
		pw = GetDlgItem(IDC_SELECT_INDY_AGE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 12;

		if (m_IndyAge) {
			wLeft = 35;
			pw = GetDlgItem(IDC_STAT_AGE);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wHeight = wHeight + (delta.bottom - delta.top) + 10;

			wLeft = 48;
			pw = GetDlgItem(IDC_SELECT_OneH);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wLeft = wLeft + (client.right - client.left) + 10;

			pw = GetDlgItem(IDC_SELECT_Three);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wLeft = wLeft + (client.right - client.left) + 10;

			pw = GetDlgItem(IDC_SELECT_FourH);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wHeight = wHeight + (delta.bottom - delta.top) + 10;

			wLeft = 48;
			pw = GetDlgItem(IDC_SELECT_Two);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wLeft = wLeft + (client.right - client.left) + 10;

			pw = GetDlgItem(IDC_SELECT_ThreeH);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wLeft = wLeft + (client.right - client.left) + 10;

			pw = GetDlgItem(IDC_SELECT_Five);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wHeight = wHeight + (delta.bottom - delta.top) + 10;

			wLeft = 48;
			pw = GetDlgItem(IDC_SELECT_TwoH);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wLeft = wLeft + (client.right - client.left) + 10;

			pw = GetDlgItem(IDC_SELECT_Four);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wLeft = wLeft + (client.right - client.left) + 10;

			pw = GetDlgItem(IDC_SELECT_FiveH);
			pw->GetWindowRect(&delta);
			client.top = wHeight;
			client.bottom = wHeight + (delta.bottom - delta.top);
			client.left = wLeft;
			client.right = client.left + (delta.right - delta.left);
			pw->MoveWindow(&client, TRUE);
			wHeight = wHeight + (delta.bottom - delta.top) + 10;
		}

		wLeft = 50;
		pw = GetDlgItem(IDOK);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = 50;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 100;

		pw = GetDlgItem(IDCANCEL);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;
	} else if (m_NotCmpDB) {
		m_SelSpCTRL.ShowWindow(SW_SHOW);
		m_SpCTRL.ShowWindow(SW_SHOW);
		m_AgeCTRL.ShowWindow(SW_HIDE);
		m_LangCTRL.ShowWindow(SW_SHOW);
		m_DbTypeCTRL.ShowWindow(SW_HIDE);

		m_ErrorMesOneCTRL.ShowWindow(SW_HIDE);
		m_ErrorMesTwoCTRL.ShowWindow(SW_HIDE);
		m_ErrorMesThrCTRL.ShowWindow(SW_HIDE);

		for (i = 0; i < TOTAL_SP_NUMBER; i++) {
			if (spCode[i][0] != EOS) {
				m_SP_CTRL[i].ShowWindow(SW_SHOW);
			} else {
				m_SP_CTRL[i].ShowWindow(SW_HIDE);
			}
		}

		m_ENG_CTRL.ShowWindow(SW_SHOW);
		m_ENGU_CTRL.ShowWindow(SW_SHOW);
		m_FRA_CTRL.ShowWindow(SW_SHOW);
		m_SPA_CTRL.ShowWindow(SW_SHOW);
		m_JPN_CTRL.ShowWindow(SW_SHOW);
		m_YUE_CTRL.ShowWindow(SW_SHOW);
		m_ZHO_CTRL.ShowWindow(SW_SHOW);

		m_IndyAge_CTRL.ShowWindow(SW_HIDE);

		m_OneH_CTRL.ShowWindow(SW_HIDE);
		m_Two_CTRL.ShowWindow(SW_HIDE);
		m_TwoH_CTRL.ShowWindow(SW_HIDE);
		m_Three_CTRL.ShowWindow(SW_HIDE);
		m_ThreeH_CTRL.ShowWindow(SW_HIDE);
		m_Four_CTRL.ShowWindow(SW_HIDE);
		m_FourH_CTRL.ShowWindow(SW_HIDE);
		m_Five_CTRL.ShowWindow(SW_HIDE);
		m_FiveH_CTRL.ShowWindow(SW_HIDE);

/*
// show gender in "no database"
		m_MALE_CTRL.ShowWindow(SW_SHOW);
		m_FEMALE_CTRL.ShowWindow(SW_SHOW);
		m_BOTHGEN_CTRL.ShowWindow(SW_SHOW);
// show gender in "no database"
*/
// no gender in "no database"
		m_MALE_CTRL.ShowWindow(SW_HIDE);
		m_FEMALE_CTRL.ShowWindow(SW_HIDE);
		m_BOTHGEN_CTRL.ShowWindow(SW_HIDE);
// no gender in "no database"

		m_DBEngToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBEngNarrative_CTRL.ShowWindow(SW_HIDE);
		m_DBEngUToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBEngUNarrative_CTRL.ShowWindow(SW_HIDE);
		m_DBZhoToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBZhoNarrative_CTRL.ShowWindow(SW_HIDE);
		m_DBNldToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBFraToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBFraNarrative_CTRL.ShowWindow(SW_HIDE);
		m_DBJpnToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBSpaToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBSpaNarrative_CTRL.ShowWindow(SW_HIDE);

		m_OK_CTRL.ShowWindow(SW_SHOW);

		pw = GetDlgItem(IDC_STAT_CHOOSE_ONE);
		pw->GetWindowRect(&delta);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;

		pw = GetDlgItem(IDC_SELECT_NOT_CMP_DB);
		pw->GetWindowRect(&delta);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;

		wHeight = wHeight + 10;

		wLeft = 8;
		pw = GetDlgItem(IDC_STAT_SEL_SP);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 7;

		wLeft = 20;
		pw = GetDlgItem(IDC_STAT_SP);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SP1);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SP2);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SP3);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SP4);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 7;

		wLeft = 20;
		pw = GetDlgItem(IDC_STAT_SP);
		pw->GetWindowRect(&delta);
		wLeft = wLeft + (delta.right - delta.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SP5);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SP6);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SP7);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SP8);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 7;

		wLeft = 20;
		pw = GetDlgItem(IDC_STAT_LANG);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_ENG);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_ENGU);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_FRA);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_SPA);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 7;

		wLeft = 20;
		pw = GetDlgItem(IDC_STAT_LANG);
		pw->GetWindowRect(&delta);
		wLeft = wLeft + (delta.right - delta.left) + 10;

		pw = GetDlgItem(IDC_SELECT_JPN);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_YUE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_ZHO);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 7;

/*
// show gender in "no database"
		wLeft = 18;
		pw = GetDlgItem(IDC_SELECT_MALE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_FEMALE);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 10;

		pw = GetDlgItem(IDC_SELECT_BOTHGEN);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 7;
// show gender in "no database"
*/

		wHeight = wHeight + 3;
		wLeft = 50;
		pw = GetDlgItem(IDOK);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wLeft = wLeft + (client.right - client.left) + 100;

		pw = GetDlgItem(IDCANCEL);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = wLeft;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;
	} else {
		m_SelSpCTRL.ShowWindow(SW_HIDE);
		m_SpCTRL.ShowWindow(SW_HIDE);
		m_AgeCTRL.ShowWindow(SW_HIDE);
		m_LangCTRL.ShowWindow(SW_HIDE);
		m_DbTypeCTRL.ShowWindow(SW_HIDE);

		m_ErrorMesOneCTRL.ShowWindow(SW_HIDE);
		m_ErrorMesTwoCTRL.ShowWindow(SW_HIDE);
		m_ErrorMesThrCTRL.ShowWindow(SW_HIDE);

		for (i = 0; i < TOTAL_SP_NUMBER; i++)
			m_SP_CTRL[i].ShowWindow(SW_HIDE);

		m_ENG_CTRL.ShowWindow(SW_HIDE);
		m_ENGU_CTRL.ShowWindow(SW_HIDE);
		m_FRA_CTRL.ShowWindow(SW_HIDE);
		m_SPA_CTRL.ShowWindow(SW_HIDE);
		m_JPN_CTRL.ShowWindow(SW_HIDE);
		m_YUE_CTRL.ShowWindow(SW_HIDE);
		m_ZHO_CTRL.ShowWindow(SW_HIDE);

		m_IndyAge_CTRL.ShowWindow(SW_HIDE);

		m_OneH_CTRL.ShowWindow(SW_HIDE);
		m_Two_CTRL.ShowWindow(SW_HIDE);
		m_TwoH_CTRL.ShowWindow(SW_HIDE);
		m_Three_CTRL.ShowWindow(SW_HIDE);
		m_ThreeH_CTRL.ShowWindow(SW_HIDE);
		m_Four_CTRL.ShowWindow(SW_HIDE);
		m_FourH_CTRL.ShowWindow(SW_HIDE);
		m_Five_CTRL.ShowWindow(SW_HIDE);
		m_FiveH_CTRL.ShowWindow(SW_HIDE);

		m_MALE_CTRL.ShowWindow(SW_HIDE);
		m_FEMALE_CTRL.ShowWindow(SW_HIDE);
		m_BOTHGEN_CTRL.ShowWindow(SW_HIDE);

		m_DBEngToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBEngNarrative_CTRL.ShowWindow(SW_HIDE);
		m_DBEngUToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBEngUNarrative_CTRL.ShowWindow(SW_HIDE);
		m_DBZhoToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBZhoNarrative_CTRL.ShowWindow(SW_HIDE);
		m_DBNldToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBFraToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBFraNarrative_CTRL.ShowWindow(SW_HIDE);
		m_DBJpnToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBSpaToyplay_CTRL.ShowWindow(SW_HIDE);
		m_DBSpaNarrative_CTRL.ShowWindow(SW_HIDE);

		m_OK_CTRL.ShowWindow(SW_HIDE);


		pw = GetDlgItem(IDC_STAT_CHOOSE_ONE);
		pw->GetWindowRect(&delta);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;

		pw = GetDlgItem(IDC_SELECT_NOT_CMP_DB);
		pw->GetWindowRect(&delta);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;

		wHeight = wHeight + 10;

		pw = GetDlgItem(IDCANCEL);
		pw->GetWindowRect(&delta);
		client.top = wHeight;
		client.bottom = wHeight + (delta.bottom - delta.top);
		client.left = 215;
		client.right = client.left + (delta.right - delta.left);
		pw->MoveWindow(&client, TRUE);
		wHeight = wHeight + (delta.bottom - delta.top) + 5;
	}

	GetWindowRect(&parent);
	wHeight = wHeight + parent_client_offsety;
	parent.bottom = parent.top + wHeight;
	MoveWindow(&parent, TRUE);
	EnableToolTips();
}

BOOL CClanKideval::OnInitDialog() {
	int i;

	CDialog::OnInitDialog();
	for (i=0; i < TOTAL_SP_NUMBER; i++)
		setNameOfButton(i, SW_HIDE, FALSE);
	ReadyToLoadState = 1;
	readFilesCnt = 0;

	m_ENG = FALSE;
	m_ENGU = FALSE;
	m_FRA = FALSE;
	m_SPA = FALSE;
	m_JPN = FALSE;
	m_YUE = FALSE;
	m_ZHO = FALSE;

	m_AgeAuto = 0;

	ResizeOptionsWindow();
	UpdateData(false);
	return 0;
}

void CClanKideval::OnCmpDB() {
	UpdateData(true);
	m_CmpDB = TRUE;
	m_NotCmpDB = FALSE;
	m_ChooseOneCTRL.ShowWindow(SW_HIDE);
	ResizeOptionsWindow();
	UpdateData(false);
}

void CClanKideval::OnNotCmpDB() {
	UpdateData(true);
	m_CmpDB = FALSE;
	m_NotCmpDB = TRUE;
	m_ChooseOneCTRL.ShowWindow(SW_HIDE);
	ResizeOptionsWindow();
	UpdateData(false);
}

void CClanKideval::OnDBEngToyplay() {
	UpdateData(true);
	m_DBEngToyplay = TRUE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBEngNarrative() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = TRUE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBEngUToyplay() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = TRUE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBEngUNarrative() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = TRUE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBZhoToyplay() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = TRUE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBZhoNarrative() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = TRUE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBNldToyplay() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = TRUE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBFraToyplay() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = TRUE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBFraNarrative() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = TRUE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBJpnToyplay() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = TRUE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBSpaToyplay() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = TRUE;
	m_DBSpaNarrative = FALSE;
	UpdateData(false);
}

void CClanKideval::OnDBSpaNarrative() {
	UpdateData(true);
	m_DBEngToyplay = FALSE;
	m_DBEngNarrative = FALSE;
	m_DBEngUToyplay = FALSE;
	m_DBEngUNarrative = FALSE;
	m_DBZhoToyplay = FALSE;
	m_DBZhoNarrative = FALSE;
	m_DBNldToyplay = FALSE;
	m_DBFraToyplay = FALSE;
	m_DBFraNarrative = FALSE;
	m_DBJpnToyplay = FALSE;
	m_DBSpaToyplay = FALSE;
	m_DBSpaNarrative = TRUE;
	UpdateData(false);
}



void CClanKideval::OnIndyAge() {
	UpdateData(true);
	m_IndyAge = !m_IndyAge;
	ResizeOptionsWindow();
	UpdateData(false);
}

void CClanKideval::OnOneH() 
{
	UpdateData(true);
	m_OneH = TRUE;
	m_Two = FALSE;
	m_TwoH = FALSE;
	m_Three = FALSE;
	m_ThreeH = FALSE;
	m_Four = FALSE;
	m_FourH = FALSE;
	m_Five = FALSE;
	m_FiveH = FALSE;
	UpdateData(false);
}

void CClanKideval::OnTwo() 
{
	UpdateData(true);
	m_OneH = FALSE;
	m_Two = TRUE;
	m_TwoH = FALSE;
	m_Three = FALSE;
	m_ThreeH = FALSE;
	m_Four = FALSE;
	m_FourH = FALSE;
	m_Five = FALSE;
	m_FiveH = FALSE;
	UpdateData(false);
}

void CClanKideval::OnTwoH() 
{
	UpdateData(true);
	m_OneH = FALSE;
	m_Two = FALSE;
	m_TwoH = TRUE;
	m_Three = FALSE;
	m_ThreeH = FALSE;
	m_Four = FALSE;
	m_FourH = FALSE;
	m_Five = FALSE;
	m_FiveH = FALSE;
	UpdateData(false);
}

void CClanKideval::OnThree() 
{
	UpdateData(true);
	m_OneH = FALSE;
	m_Two = FALSE;
	m_TwoH = FALSE;
	m_Three = TRUE;
	m_ThreeH = FALSE;
	m_Four = FALSE;
	m_FourH = FALSE;
	m_Five = FALSE;
	m_FiveH = FALSE;
	UpdateData(false);
}

void CClanKideval::OnThreeH() 
{
	UpdateData(true);
	m_OneH = FALSE;
	m_Two = FALSE;
	m_TwoH = FALSE;
	m_Three = FALSE;
	m_ThreeH = TRUE;
	m_Four = FALSE;
	m_FourH = FALSE;
	m_Five = FALSE;
	m_FiveH = FALSE;
	UpdateData(false);
}

void CClanKideval::OnFour() 
{
	UpdateData(true);
	m_OneH = FALSE;
	m_Two = FALSE;
	m_TwoH = FALSE;
	m_Three = FALSE;
	m_ThreeH = FALSE;
	m_Four = TRUE;
	m_FourH = FALSE;
	m_Five = FALSE;
	m_FiveH = FALSE;
	UpdateData(false);
}

void CClanKideval::OnFourH() 
{
	UpdateData(true);
	m_OneH = FALSE;
	m_Two = FALSE;
	m_TwoH = FALSE;
	m_Three = FALSE;
	m_ThreeH = FALSE;
	m_Four = FALSE;
	m_FourH = TRUE;
	m_Five = FALSE;
	m_FiveH = FALSE;
	UpdateData(false);
}

void CClanKideval::OnFive() 
{
	UpdateData(true);
	m_OneH = FALSE;
	m_Two = FALSE;
	m_TwoH = FALSE;
	m_Three = FALSE;
	m_ThreeH = FALSE;
	m_Four = FALSE;
	m_FourH = FALSE;
	m_Five = TRUE;
	m_FiveH = FALSE;
	UpdateData(false);
}

void CClanKideval::OnFiveH() 
{
	UpdateData(true);
	m_OneH = FALSE;
	m_Two = FALSE;
	m_TwoH = FALSE;
	m_Three = FALSE;
	m_ThreeH = FALSE;
	m_Four = FALSE;
	m_FourH = FALSE;
	m_Five = FALSE;
	m_FiveH = TRUE;
	UpdateData(false);
}

void CClanKideval::OnMale() 
{
	UpdateData(true);
	m_MALE = TRUE;
	m_FEMALE = FALSE;
	m_BOTHGEN = FALSE;
	UpdateData(false);
}

void CClanKideval::OnFemale() 
{
	UpdateData(true);
	m_MALE = FALSE;
	m_FEMALE = TRUE;
	m_BOTHGEN = FALSE;
	UpdateData(false);
}

void CClanKideval::OnBothGen() {
	UpdateData(true);
	m_MALE = FALSE;
	m_FEMALE = FALSE;
	m_BOTHGEN = TRUE;
	UpdateData(false);
}

void CClanKideval::OnSelectSP1() 
{
	int i;

	UpdateData(true);
	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spSet[i] = FALSE;
		m_SP[i] = FALSE;
	}
	m_SP[0] = TRUE;
	UpdateData(false);
	spSet[0] = TRUE;
}

void CClanKideval::OnSelectSP2() 
{
	int i;

	UpdateData(true);
	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spSet[i] = FALSE;
		m_SP[i] = FALSE;
	}
	m_SP[1] = TRUE;
	UpdateData(false);
	spSet[1] = TRUE;
}

void CClanKideval::OnSelectSP3() 
{
	int i;

	UpdateData(true);
	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spSet[i] = FALSE;
		m_SP[i] = FALSE;
	}
	m_SP[2] = TRUE;
	UpdateData(false);
	spSet[2] = TRUE;
}

void CClanKideval::OnSelectSP4() 
{
	int i;

	UpdateData(true);
	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spSet[i] = FALSE;
		m_SP[i] = FALSE;
	}
	m_SP[3] = TRUE;
	UpdateData(false);
	spSet[3] = TRUE;
}

void CClanKideval::OnSelectSP5() 
{
	int i;

	UpdateData(true);
	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spSet[i] = FALSE;
		m_SP[i] = FALSE;
	}
	m_SP[4] = TRUE;
	UpdateData(false);
	spSet[4] = TRUE;
}

void CClanKideval::OnSelectSP6() 
{
	int i;

	UpdateData(true);
	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spSet[i] = FALSE;
		m_SP[i] = FALSE;
	}
	m_SP[5] = TRUE;
	UpdateData(false);
	spSet[5] = TRUE;
}

void CClanKideval::OnSelectSP7() 
{
	int i;

	UpdateData(true);
	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spSet[i] = FALSE;
		m_SP[i] = FALSE;
	}
	m_SP[6] = TRUE;
	UpdateData(false);
	spSet[6] = TRUE;
}

void CClanKideval::OnSelectSP8() 
{
	int i;

	UpdateData(true);
	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spSet[i] = FALSE;
		m_SP[i] = FALSE;
	}
	m_SP[7] = TRUE;
	UpdateData(false);
	spSet[7] = TRUE;
}

void CClanKideval::OnSelectENG() {
	UpdateData(true);
	m_ENG = TRUE;
	m_ENGU = FALSE;
	m_FRA = FALSE;
	m_SPA = FALSE;
	m_JPN = FALSE;
	m_YUE = FALSE;
	m_ZHO = FALSE;
	UpdateData(false);
}

void CClanKideval::OnSelectENGU() {
	UpdateData(true);
	m_ENG = FALSE;
	m_ENGU = TRUE;
	m_FRA = FALSE;
	m_SPA = FALSE;
	m_JPN = FALSE;
	m_YUE = FALSE;
	m_ZHO = FALSE;
	UpdateData(false);
}

void CClanKideval::OnSelectFRA() {
	UpdateData(true);
	m_ENG = FALSE;
	m_ENGU = FALSE;
	m_FRA = TRUE;
	m_SPA = FALSE;
	m_JPN = FALSE;
	m_YUE = FALSE;
	m_ZHO = FALSE;
	UpdateData(false);
}

void CClanKideval::OnSelectSPA() {
	UpdateData(true);
	m_ENG = FALSE;
	m_ENGU = FALSE;
	m_FRA = FALSE;
	m_SPA = TRUE;
	m_JPN = FALSE;
	m_YUE = FALSE;
	m_ZHO = FALSE;
	UpdateData(false);
}

void CClanKideval::OnSelectJPN() {
	UpdateData(true);
	m_ENG = FALSE;
	m_ENGU = FALSE;
	m_FRA = FALSE;
	m_SPA = FALSE;
	m_JPN = TRUE;
	m_YUE = FALSE;
	m_ZHO = FALSE;
	UpdateData(false);
}

void CClanKideval::OnSelectYUE() {
	UpdateData(true);
	m_ENG = FALSE;
	m_ENGU = FALSE;
	m_FRA = FALSE;
	m_SPA = FALSE;
	m_JPN = FALSE;
	m_YUE = TRUE;
	m_ZHO = FALSE;
	UpdateData(false);
}

void CClanKideval::OnSelectZHO() {
	UpdateData(true);
	m_ENG = FALSE;
	m_ENGU = FALSE;
	m_FRA = FALSE;
	m_SPA = FALSE;
	m_JPN = FALSE;
	m_YUE = FALSE;
	m_ZHO = TRUE;
	UpdateData(false);
}

void CClanKideval::OnOKClicked() {
	UpdateData(TRUE);
//	if (saveIDFields())
		OnOK();
//	UpdateData(FALSE);
}



void CClanKideval::kidevalGetTierNamesFromFile(char *fname) {
	int agef, aget;
	char	*s, *e, *code, *age;
	time_t	timer;
	FILE	*fp;

	fp = fopen(fname, "r");
	if (fp == NULL)
		return;
//	UpdateData(true);
	time(&timer);
	if (timer > GlobalTime) {
		readFilesCnt++;
		sprintf(templineC3, "Reading file %d", readFilesCnt);
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
				addToSpArr(templineC3);
				isFoundFile = TRUE;
			}
		} else if (strncmp(templineC3, "@Languages:",11) == 0) {
		} else if (strncmp(templineC3, "@ID:", 4) == 0) {
			s = templineC3 + 4;
			while (isSpace(*s))
				s++;
			code = s;
			s = strchr(templineC3, '|');
			if (s != NULL) {
				if (m_ENG == FALSE && m_ENGU == FALSE && m_FRA == FALSE && m_SPA == FALSE &&
					m_JPN == FALSE && m_YUE == FALSE && m_ZHO == FALSE) {
					*s = EOS;
					e = strchr(code, ',');
					if (e == NULL) {
						e = strchr(code, ' ');
						if (e != NULL)
							*e = EOS;
					} else
						*e = EOS;
					if (uS.mStricmp(code, "eng") == 0) {
						m_ENG = TRUE;
					} else if (uS.mStricmp(code, "engu") == 0) {
						m_ENGU = TRUE;
					} else if (uS.mStricmp(code, "fra") == 0) {
						m_FRA = TRUE;
					} else if (uS.mStricmp(code, "spa") == 0) {
						m_SPA = TRUE;
					} else if (uS.mStricmp(code, "jpn") == 0) {
						m_JPN = TRUE;
					} else if (uS.mStricmp(code, "yue") == 0) {
						m_YUE = TRUE;
					} else if (uS.mStricmp(code, "zho") == 0) {
						m_ZHO = TRUE;
					}
				}
				s++;
				code = strchr(s, '|');
				if (code != NULL) {
					*code = '*';
					s = strchr(code, '|');
					if (s != NULL) {
						isFoundFile = TRUE;
						*s = EOS;
						uS.remblanks(code);
						addToSpArr(code);
						age = s + 1;
						s = strchr(age, '|');
						if (s != NULL) {
							*s = EOS;
							uS.remblanks(age);
							if (age[0] != EOS && uS.mStricmp(code, "*CHI") == 0 && isAge(age, &agef, &aget)) {
								if (m_AgeAuto == 0) {
									m_AgeAuto = agef;
								}
							}
						}
					}
				}
			}
		}
	}
	fclose(fp);
}

void CClanKideval::kidevalMakeSpeakersList() {
	int		i, j, len;
	FILE	*fp;

	SetNewVol(wd_dir);
	for (i=1; i < cl_argc; i++) {
		if (cl_argv[i][0] == '-' || cl_argv[i][0] == '+') {
		} else {
			if (!strcmp(cl_argv[i], "@")) {
				for (j=1; j <= F_numfiles && j < 5; j++) {
					get_selected_file(j, FileName1, FNSize);
					kidevalGetTierNamesFromFile(FileName1);
				}
			} else if (cl_argv[i][0] == '@' && cl_argv[i][1] == ':') {
				uS.str2FNType(FileName1, 0L, cl_argv[i]+2);
				fp = fopen(FileName1, "r");
				if (fp == NULL) {
					return;
				}
				j = 0;
				while (fgets_cr(FileName1, 3072, fp)) {
					j++;
					if (j >= 4)
						break;
					uS.remFrontAndBackBlanks(FileName1);
					kidevalGetTierNamesFromFile(FileName1);
				}
				fclose(fp);
			} else if (strchr(cl_argv[i], '*') == NULL) {
				kidevalGetTierNamesFromFile(cl_argv[i]);
			} else {
				strcpy(DirPathName, wd_dir);
				len = strlen(DirPathName);
				j = 1;
				while ((j=Get_File(FileName1, j)) != 0) {
					if (uS.fIpatmat(FileName1, cl_argv[i])) {
						if (j > 4)
							break;
						addFilename2Path(DirPathName, FileName1);
						kidevalGetTierNamesFromFile(DirPathName);
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
//m_SP1_CTRL.GetTextMargin(&pmargin);
void CClanKideval::setNameOfButton(int i, int nCmdShow, BOOL val) {
	m_SP_CTRL[i].ShowWindow(nCmdShow);
	if (spCode[i][0] != EOS) {
		strcpy(templineC3, spCode[i]);
		u_strcpy(templine3, templineC3, UTTLINELEN);
		if (i == 0) {
			SetDlgItemText(IDC_SELECT_SP1, templine3);
		} else if (i == 1) {
			SetDlgItemText(IDC_SELECT_SP2, templine3);
		} else if (i == 2) {
			SetDlgItemText(IDC_SELECT_SP3, templine3);
		} else if (i == 3) {
			SetDlgItemText(IDC_SELECT_SP4, templine3);
		} else if (i == 4) {
			SetDlgItemText(IDC_SELECT_SP5, templine3);
		} else if (i == 5) {
			SetDlgItemText(IDC_SELECT_SP6, templine3);
		} else if (i == 6) {
			SetDlgItemText(IDC_SELECT_SP7, templine3);
		} else if (i == 7) {
			SetDlgItemText(IDC_SELECT_SP8, templine3);
		}
	}
	m_SP[i] = val;

}

LRESULT CClanKideval::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	int i;
	LRESULT res = 0;

	if (message == WM_PAINT && ReadyToLoadState == 1) {
		ReadyToLoadState = 2;
		m_ReadingCTRL.ShowWindow(SW_SHOW);
		GlobalTime = 0;
		kidevalMakeSpeakersList();
		if (m_ENG == FALSE && m_ENGU == FALSE && m_FRA == FALSE && m_SPA == FALSE &&
			m_JPN == FALSE && m_YUE == FALSE && m_ZHO == FALSE) {
			m_ENG = TRUE;
		}
		m_ReadingCTRL.ShowWindow(SW_HIDE);
		for (i=0; i < TOTAL_SP_NUMBER; i++) {
			if (spCode[i][0] == EOS)
				break;
			if (uS.mStricmp(spCode[i], "*CHI") == 0) {
				spSet[i] = TRUE;
			} else {
				spSet[i] = FALSE;
			}
			setNameOfButton(i, SW_HIDE, spSet[i]);
		}
		ResizeOptionsWindow();
		UpdateData(false);	
	}
	res = CDialog ::WindowProc(message, wParam, lParam);
	return(res);
}

static void selectKidevalDialog(void) {
	int  i;
	const char *dbSt, *ageSt, *langSt, *genSt;
	CClanKideval dlg;

	for (i = 0; i < TOTAL_SP_NUMBER; i++) {
		spCode[i][0] = EOS;
		spSet[i] = FALSE;
	}

	dlg.isFoundFile = FALSE;
	dlg.ReadyToLoadState = 0;
	dlg.m_CmpDB = CmpDBTp;
	dlg.m_NotCmpDB = NotCmpDBTp;

	dlg.m_AgeAuto = 0;

	dlg.m_IndyAge = FALSE; // IndyAgeTp;

	dlg.m_OneH = OneHTp;
	dlg.m_Two = TwoTp;
	dlg.m_TwoH = TwoHTp;
	dlg.m_Three = ThreeTp;
	dlg.m_ThreeH = ThreeHTp;
	dlg.m_Four = FourTp;
	dlg.m_FourH = FourHTp;
	dlg.m_Five = FiveTp;
	dlg.m_FiveH = FiveHTp;

	dlg.m_MALE = MaleOnly;
	dlg.m_FEMALE = FemaleOnly;
	dlg.m_BOTHGEN = BothGen;
	dlg.m_LinkAge = LinkAgeTp;
	dlg.m_DBEngToyplay = DBEngToyplayTp;
	dlg.m_DBEngNarrative = DBEngNarrativeTp;
	dlg.m_DBEngUToyplay = DBEngUToyplayTp;
	dlg.m_DBEngUNarrative = DBEngUNarrativeTp;
	dlg.m_DBZhoToyplay = DBZhoToyplayTp;
	dlg.m_DBZhoNarrative = DBZhoNarrativeTp;
	dlg.m_DBNldToyplay = DBNldToyplayTp;
	dlg.m_DBFraToyplay = DBFraToyplayTp;
	dlg.m_DBFraNarrative = DBFraNarrativeTp;
	dlg.m_DBJpnToyplay = DBJpnToyplayTp;
	dlg.m_DBSpaToyplay = DBSpaToyplayTp;
	dlg.m_DBSpaNarrative = DBSpaNarrativeTp;

repeat:
	if (dlg.DoModal() == IDOK) {
		CmpDBTp = dlg.m_CmpDB;
		NotCmpDBTp = dlg.m_NotCmpDB;
		IndyAgeTp = dlg.m_IndyAge;
		OneHTp = dlg.m_OneH;
		TwoTp = dlg.m_Two;
		TwoHTp = dlg.m_TwoH;
		ThreeTp = dlg.m_Three;
		ThreeHTp = dlg.m_ThreeH;
		FourTp = dlg.m_Four;
		FourHTp = dlg.m_FourH;
		FiveTp = dlg.m_Five;
		FiveHTp = dlg.m_FiveH;
		MaleOnly = dlg.m_MALE;
		FemaleOnly = dlg.m_FEMALE;
		BothGen = dlg.m_BOTHGEN;
		LinkAgeTp = dlg.m_LinkAge;
		DBEngToyplayTp = dlg.m_DBEngToyplay;
		DBEngNarrativeTp = dlg.m_DBEngNarrative;
		DBEngUToyplayTp = dlg.m_DBEngUToyplay;
		DBEngUNarrativeTp = dlg.m_DBEngUNarrative;
		DBZhoToyplayTp = dlg.m_DBZhoToyplay;
		DBZhoNarrativeTp = dlg.m_DBZhoNarrative;
		DBNldToyplayTp = dlg.m_DBNldToyplay;
		DBFraToyplayTp = dlg.m_DBFraToyplay;
		DBFraNarrativeTp = dlg.m_DBFraNarrative;
		DBJpnToyplayTp = dlg.m_DBJpnToyplay;
		DBSpaToyplayTp = dlg.m_DBSpaToyplay;
		DBSpaNarrativeTp = dlg.m_DBSpaNarrative;

		if (MaleOnly)
			genSt = "male";
		else if (FemaleOnly)
			genSt = "female";
		else
			genSt = "";

		if (CmpDBTp) {
			if (DBEngToyplayTp) {
				dbSt = "toyplay";
				langSt = "eng";
			} else if (DBEngNarrativeTp) {
				dbSt = "narrative";
				langSt = "eng";
			} else if (DBEngUToyplayTp) {
				dbSt = "toyplay";
				langSt = "engu";
			} else if (DBEngUNarrativeTp) {
				dbSt = "narrative";
				langSt = "engu";
			} else if (DBZhoToyplayTp) {
				dbSt = "toyplay";
				langSt = "zho";
			} else if (DBZhoNarrativeTp) {
				dbSt = "narrative";
				langSt = "zho";
			} else if (DBFraToyplayTp) {
				dbSt = "toyplay";
				langSt = "fra";
			} else if (DBFraNarrativeTp) {
				dbSt = "narrative";
				langSt = "fra";
			} else if (DBNldToyplayTp) {
				dbSt = "toyplay";
				langSt = "nld";
			} else if (DBJpnToyplayTp) {
				dbSt = "toyplay";
				langSt = "jpn";
			} else if (DBSpaToyplayTp) {
				dbSt = "toyplay";
				langSt = "spa";
			} else if (DBSpaNarrativeTp) {
				dbSt = "narrative";
				langSt = "spa";
			} else {
				do_warning("Please select one database", 0);
				goto repeat;
			}
			if (IndyAgeTp == FALSE) {
				if (dlg.m_AgeAuto >= 18 && dlg.m_AgeAuto <= 23) {
					ageSt = "1;6-1;11";
				} else if (dlg.m_AgeAuto >= 24 && dlg.m_AgeAuto <= 29) {
					ageSt = "2;-2;5";
				} else if (dlg.m_AgeAuto >= 30 && dlg.m_AgeAuto <= 35) {
					ageSt = "2;6-2;11";
				} else if (dlg.m_AgeAuto >= 36 && dlg.m_AgeAuto <= 41) {
					ageSt = "3;-3;5";
				} else if (dlg.m_AgeAuto >= 42 && dlg.m_AgeAuto <= 47) {
					ageSt = "3;6-3;11";
				} else if (dlg.m_AgeAuto >= 48 && dlg.m_AgeAuto <= 53) {
					ageSt = "4;-4;5";
				} else if (dlg.m_AgeAuto >= 54 && dlg.m_AgeAuto <= 59) {
					ageSt = "4;6-4;11";
				} else if (dlg.m_AgeAuto >= 60 && dlg.m_AgeAuto <= 65) {
					ageSt = "5;-5;5";
				} else if (dlg.m_AgeAuto >= 66 && dlg.m_AgeAuto <= 72) {
					ageSt = "5;6-6;";
				} else {
					do_warning("Can't find @ID tier for *CHI speaker that provides child's age. Please, double check input file(s).", 0);
					templineC3[0] = EOS;
					return;
				}
			} else {
				if (OneHTp) {
					ageSt = "1;6-1;11";
				} else if (TwoTp) {
					ageSt = "2;-2;5";
				} else if (TwoHTp) {
					ageSt = "2;6-2;11";
				} else if (ThreeTp) {
					ageSt = "3;-3;5";
				} else if (ThreeHTp) {
					ageSt = "3;6-3;11";
				} else if (FourTp) {
					ageSt = "4;-4;5";
				} else if (FourHTp) {
					ageSt = "4;6-4;11";
				} else if (FiveTp) {
					ageSt = "5;-5;5";
				} else if (FiveHTp) {
					ageSt = "5;6-6;";
				} else {
					do_warning("Please select one Age Range", 0);
					goto repeat;
				}
			}

			strcpy(templineC3, clan_name[KIDEVAL]);
			strcat(templineC3, " @");
			if (IndyAgeTp == FALSE) {
				strcat(templineC3, " +LinkAge");
			}
			strcat(templineC3, " +l");
			strcat(templineC3, langSt);
			strcat(templineC3, " +t*CHI:");
			strcat(templineC3, " +d");
			strcat(templineC3, dbSt);
			strcat(templineC3, "~\"");
			strcat(templineC3, ageSt);
			if (genSt[0] != EOS) {
				strcat(templineC3, "|");
				strcat(templineC3, genSt);
			}
			strcat(templineC3, "\"");

		} else if (NotCmpDBTp) {
			genSt = ""; // no gender in "no database"

			for (i = 0; i < TOTAL_SP_NUMBER; i++) {
				if (spCode[i][0] != EOS && spSet[i] == TRUE)
					break;
			}
			if (i >= TOTAL_SP_NUMBER) {
				do_warning("Please select one Speaker", 0);
				goto repeat;
			}

			if (dlg.m_ENG == TRUE) {
				langSt = "eng";
			} else if (dlg.m_ENGU == TRUE) {
				langSt = "engu";
			} else if (dlg.m_FRA == TRUE) {
				langSt = "fra";
			} else if (dlg.m_SPA == TRUE) {
				langSt = "spa";
			} else if (dlg.m_JPN == TRUE) {
				langSt = "jpn";
			} else if (dlg.m_YUE == TRUE) {
				langSt = "yue";
			} else if (dlg.m_ZHO == TRUE) {
				langSt = "zho";
			} else {
				do_warning("Please select one Language", 0);
				goto repeat;
			}
			strcpy(templineC3, clan_name[KIDEVAL]);
			strcat(templineC3, " @");
			strcat(templineC3, " +l");
			strcat(templineC3, langSt);
			for (i = 0; i < TOTAL_SP_NUMBER; i++) {
				if (spSet[i] == TRUE && spCode[i][0] != EOS) {
					if (genSt[0] != EOS) {
						strcat(templineC3, " +t@ID=\"*|*|");
						if (spCode[i][0] == '*')
							strcat(templineC3, spCode[i] + 1);
						else
							strcat(templineC3, spCode[i]);
						strcat(templineC3, "|*|");
						strcat(templineC3, genSt);
						strcat(templineC3, "|*\"");
					} else {
						strcat(templineC3, " +t");
						strcat(templineC3, spCode[i]);
						strcat(templineC3, ":");
					}
				}
			}
		} else {
			templineC3[0] = EOS;
		}
	} else {
		templineC3[0] = EOS;
	}
}

/*
18-23	1;6 - 1;11
24-29	2;  - 2;5
30-35	2;6 - 2;11
36-41	3;  - 3;5
42-47	3;6 - 3;11
48-53	4;  - 4;5
54-59	4;6 - 4;11
60-65	5;  - 5;5
66-72	5;6 - 6;
*/

void KidevalDialog(unCH *str) {
	CClanKideval dlg;

	myget();
	if (F_numfiles <= 0) {
		do_warning("ERROR KidEVAL. Please specify input data files first", 0);
		str[0] = EOS;
		return;
	}
	cl_argc = 1;
	strcpy(templineC2, " @");
	if (!MakeArgs(templineC2)) {
		str[0] = EOS;
		return;
	}
	selectKidevalDialog();
	if (templineC3[0] != EOS) {
		u_strcpy(templine3, templineC3, UTTLINELEN);
		strcpy(str, templine3);
	} else
		str[0] = EOS;
}
