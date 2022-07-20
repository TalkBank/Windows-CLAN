#include "ced.h"
#include "W95_feedbackClass.h"
#include "W95_feedback.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CFeedback *feedbackDlg = NULL;

/////////////////////////////////////////////////////////////////////////////
// CFeedback dialog

CFeedback::CFeedback(CString Label, CString message, CWnd* pParent /*=NULL*/)
	: CDialog(CFeedback::IDD, pParent) {
	//{{AFX_DATA_INIT(CFeedback)
	m_Label = Label;
	m_message = message;
	//}}AFX_DATA_INIT
	Create(IDD, pParent);
}

BOOL CFeedback::OnInitDialog() {

	CDialog::OnInitDialog();
	return TRUE;
}

void CFeedback::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFeedback)
	DDX_Text(pDX, IDC_FEEDBACK_LABEL, m_Label);
	DDX_Text(pDX, IDC_FEEDBACK_MESSAGE, m_message);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFeedback, CDialog)
	//{{AFX_MSG_MAP(CFeedback)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFeedback message handlers

void CFeedback::OnCancel() {

	feedbackDlg = NULL;
	EndDialog(IDCANCEL);
}

/////////////////////////////////////////////////////////////////////////////
// CFeedback control functions

void initFeedback(unCH *Label, unCH *message) {
	if (Label == NULL)
		feedbackDlg = NULL;
	else
		feedbackDlg = new CFeedback(Label, message, AfxGetApp()->m_pMainWnd);
}

void setCurrentFeedbackMessage(int perc) {
	char    messageC[512];
	unCH messageW[512];

	if (feedbackDlg != NULL) {
		sprintf(messageC, "%d%%                ", perc);
		u_strcpy(messageW, messageC, 512);
		feedbackDlg->m_message = messageW;
//		feedbackDlg->UpdateData(FALSE);
	}
}

void deleteFeedback(void) {
	if (feedbackDlg != NULL)
		feedbackDlg->OnCancel();
}
