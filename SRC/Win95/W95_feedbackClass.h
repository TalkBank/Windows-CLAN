#if !defined(AFX_W95_FEEDBACK_CLASS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
#define AFX_W95_FEEDBACK_CLASS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// W95_feedbackClass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFeedback dialog

class CFeedback: public CDialog {
	// Construction
public:
	CFeedback(CString Label, CString message, CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
	enum { IDD = IDD_USER_FEEDBACK };
	//{{AFX_DATA(CFeedback)
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFeedback)
public:
	virtual void OnCancel();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

	// Implementation

public:
	CString m_message;

protected:
	CString m_Label;

	// Generated message map functions
	//{{AFX_MSG(CFeedback)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W95_FEEDBACK_CLASS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
