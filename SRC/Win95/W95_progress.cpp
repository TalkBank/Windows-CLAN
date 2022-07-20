//#define USETIMER

#include "ced.h"
#include "W95_progressClass.h"
#include "W95_progress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL hasUserCaceledProgressBar = FALSE;
static CProgressBar *progressDlg = NULL;
#ifdef USETIMER
static int perc, oldPerc;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressBar dialog

CProgressBar::CProgressBar(CString Label, CWnd* pParent /*=NULL*/)
	: CDialog(CProgressBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressBar)
	m_Label = Label;
#ifdef USETIMER
	timer = 0;
#endif
	hasUserCaceledProgressBar = FALSE;
	//}}AFX_DATA_INIT
	Create(IDD, pParent);
}
BOOL CProgressBar::OnInitDialog() {

	CDialog::OnInitDialog();
#ifdef USETIMER
	int i = 0;
	do {
		i++;
		timer = SetTimer(i, 10, NULL);
	} while (timer == 0);
#endif
	return TRUE;
}

void CProgressBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressBar)
	DDX_Text(pDX, IDC_LABEL, m_Label);
	DDX_Control(pDX, IDC_PROGRESS, m_ProgressBar);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProgressBar, CDialog)
	//{{AFX_MSG_MAP(CProgressBar)
	ON_BN_CLICKED(IDC_BOT_CANCEL, OnBotCancel)
#ifdef USETIMER
	ON_WM_TIMER()
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressBar message handlers

#ifdef USETIMER
void CProgressBar::OnTimer(UINT nIDEvent) {
	if (perc > oldPerc && progressDlg != NULL) {
		progressDlg->m_ProgressBar.SetPos(perc);
		progressDlg->UpdateData(FALSE);
		oldPerc = perc;
	}
	CDialog::OnTimer(nIDEvent);
}
#endif

void CProgressBar::OnBotCancel() 
{
#ifdef USETIMER
	if (KillTimer(timer))
		timer = 0;
#endif
	hasUserCaceledProgressBar = TRUE;
	progressDlg = NULL;
	EndDialog(IDCANCEL);
}

void CProgressBar::OnCancel() {
#ifdef USETIMER
	if (KillTimer(timer))
		timer = 0;
#endif
	progressDlg = NULL;
	EndDialog(IDCANCEL);
}

/////////////////////////////////////////////////////////////////////////////
// CProgressBar control functions

void initProgressBar(unCH *Label) {
	hasUserCaceledProgressBar = FALSE;
	progressDlg = new CProgressBar(Label, AfxGetApp()->m_pMainWnd);
	if (progressDlg != NULL) {
#ifdef USETIMER
		perc = 0;
		oldPerc = 0;
#endif
		progressDlg->m_ProgressBar.SetRange(0, 100);
	}
}

void setCurrentProgressBarValue(int perc) {
	if (progressDlg != NULL) {
		progressDlg->m_ProgressBar.SetPos(perc);
		progressDlg->UpdateData(FALSE);
	}
}

#ifdef USETIMER
void setProgressBarPercValue(int nperc) {
	if (progressDlg != NULL) {
		perc = nperc;
	}
}
#endif

void deleteProgressBar(void) {
	if (progressDlg != NULL)
		progressDlg->OnCancel();
}

BOOL isProgressBarCanceled(void) {
	return hasUserCaceledProgressBar;
}
