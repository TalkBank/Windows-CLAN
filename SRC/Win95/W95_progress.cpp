#include "ced.h"
#include "W95_progress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CProgressBar *progressDlg = NULL;

/////////////////////////////////////////////////////////////////////////////
// CProgressBar dialog

CProgressBar::CProgressBar(CString Label, CWnd* pParent /*=NULL*/)
	: CDialog(CProgressBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressBar)
	m_Label = Label;
	//}}AFX_DATA_INIT
	Create(IDD, pParent);
}
BOOL CProgressBar::OnInitDialog() {

	CDialog::OnInitDialog();
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
	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressBar message handlers


void CProgressBar::OnBotCancel() 
{
	progressDlg = NULL;
	EndDialog(IDCANCEL);
}

void CProgressBar::OnCancel() {
	progressDlg = NULL;
	EndDialog(IDCANCEL);
}
