#include "stdafx.h"
#include "ced.h"
#include "MMedia.h"
#include "Clan2Doc.h"
#include "MVHelp.h"

CMyHelp *MVHelpDlg=NULL;

CMyHelp::CMyHelp(CWnd* pParent /*=NULL*/)
	: CDialog(CMyHelp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMyHelp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMyHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMyHelp)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMyHelp, CDialog)
	//{{AFX_MSG_MAP(CMyHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyHelp message handlers

BOOL CMyHelp::DestroyWindow() 
{
	BOOL res = CDialog::DestroyWindow();
	if (res) {
		delete this;
		MVHelpDlg=NULL;
	}
	return res;
}

void CMyHelp::OnCancel() {
	DestroyWindow();
}
