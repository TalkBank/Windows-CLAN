#if !defined(AFX_W95_WEBDATA_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
#define AFX_W95_WEBDATA_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// W95_WebData.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWebData dialog

class CWebData : public CDialog
{
// Construction
public:
	CWebData(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWebData)
	enum { IDD = IDD_WEB_DATA };
	CListBox	m_WebDataListControl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWebData)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:

	long extractNames(char *st);
	void handleWebDirFile(unCH *fname);

	// Generated message map functions
	//{{AFX_MSG(CWebData)
	afx_msg void OnSelchangeWebData();
	afx_msg void OnDblclkWebData();
	afx_msg void OnDestroy();
	afx_msg void OnWebTalkbank();
	afx_msg void OnWebChildes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CWebData *webDataDlg;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W95_WEBDATA_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
