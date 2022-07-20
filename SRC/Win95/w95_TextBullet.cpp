// w95_TextBullet.cpp : implementation file
//

#include "ced.h"
#include "MMedia.h"
#include "ceddlgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern struct DefWin TextWinSize;

static cTextBullet *textDial = NULL;
static char textName[_MAX_PATH+_MAX_FNAME];

/////////////////////////////////////////////////////////////////////////////
// cTextBullet dialog


cTextBullet::cTextBullet(CString pathName, CWnd* pParent /*=NULL*/)
	: CDialog(cTextBullet::IDD, pParent)
{

	//{{AFX_DATA_INIT(cTextBullet)
	m_text = _T("");
	//}}AFX_DATA_INIT
	u_strcpy(fname, pathName, _MAX_PATH+_MAX_FNAME);
	Create(IDD, pParent);
}


void cTextBullet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cTextBullet)
	DDX_Control(pDX, IDC_TEXTBULLET, m_TextControl);
	DDX_Text(pDX, IDC_TEXTBULLET, m_text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(cTextBullet, CDialog)
	//{{AFX_MSG_MAP(cTextBullet)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cTextBullet message handlers

BOOL cTextBullet::OnInitDialog() {
	FILE *fp;
	unsigned long cnt;
	RECT client;
	CWnd* pw;

	CDialog::OnInitDialog();

	if (TextWinSize.top || TextWinSize.width || TextWinSize.height || TextWinSize.left) {
		CRect lpRect;
		long width, hight;

		this->GetWindowRect(&lpRect);
		width = lpRect.right - lpRect.left;
		hight = lpRect.bottom - lpRect.top;
		lpRect.top = TextWinSize.top;
		lpRect.bottom = lpRect.top + hight;
		lpRect.left = TextWinSize.left;
		lpRect.right = lpRect.left + width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}
	GetClientRect(&client);
	if (client.top < 0) {
		client.bottom = client.bottom - client.top;
		client.top = 0;
	}
	if (client.left < 0) {
		client.right = client.right - client.left;
		client.left = 0;
	}
	pw=GetDlgItem(IDC_TEXTBULLET);
	if (pw != NULL)
		pw->MoveWindow(&client,true);

	if ((fp=fopen(fname, "r")) == NULL)
		return TRUE;
	m_text = _T("");
	last_cr_char = '\0';
	while (fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt)) {
		uS.remblanks(ced_lineC);
		m_text = m_text + u_strcpy(ced_line, ced_lineC, UTTLINELEN);
		m_text = m_text + "\r\n";
	}
	fclose(fp);
	UpdateData(FALSE);
	return TRUE;
}

void cTextBullet::OnCancel() {
	CRect lpRect;

	this->GetWindowRect(&lpRect);
	TextWinSize.top = lpRect.top;
	TextWinSize.left = lpRect.left;
	TextWinSize.width = lpRect.right;
	TextWinSize.height = lpRect.bottom;
	WriteCedPreference();
	DestroyWindow();
	if (textDial != NULL) {
		delete textDial;
		textDial = NULL;
	}
	textName[0] = EOS;
}

void cTextBullet::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void cTextBullet::OnPaint() 
{
	int s, e;

	CPaintDC dc(this); // device context for painting
	
	CDialog::OnPaint();
	UpdateData(TRUE);
	m_TextControl.GetSel(s, e);
	m_TextControl.SetSel(s, s, FALSE);
}

void cTextBullet::OnSize(UINT nType, int cx, int cy) 
{
	RECT client;
	CWnd* pw;

	CDialog::OnSize(nType, cx, cy);
	GetClientRect(&client);
	if (client.top < 0) {
		client.bottom = client.bottom - client.top;
		client.top = 0;
	}
	if (client.left < 0) {
		client.right = client.right - client.left;
		client.left = 0;
	}
	pw=GetDlgItem(IDC_TEXTBULLET);
	if (pw != NULL)
		pw->MoveWindow(&client,true);
}


// change this to "DisplayText"
// and change "DisplayText" in file w95_TxtBullet.cpp to "DisplayFileText"
int DisplayFileText(char *justFname) {
	if (!strcmp(textName, justFname))
		return(0);

	strcpy(textName, justFname);
	if (textDial != NULL)
		textDial->OnCancel();
	textDial = new cTextBullet(justFname, AfxGetApp()->m_pMainWnd);
	return(0);
}

