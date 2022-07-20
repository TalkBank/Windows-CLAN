#if !defined(AFX_W95_PROGRESS_CLASS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
#define AFX_W95_PROGRESS_CLASS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// W95_progressClass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressBar dialog

class CProgressBar : public CDialog
{
// Construction
public:
	CProgressBar(CString Label, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_PROGRESS };
	//{{AFX_DATA(CProgressBar)
	CProgressCtrl m_ProgressBar; // progress control status bar
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressBar)
	public:
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation

protected:
	CString m_Label;
#ifdef USETIMER
	UINT timer;
#endif

	// Generated message map functions
	//{{AFX_MSG(CProgressBar)
	afx_msg void OnBotCancel();
#ifdef USETIMER
	afx_msg void OnTimer(UINT nIDEvent);
#endif
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W95_PROGRESS_CLASS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
