/////////////////////////////////////////////////////////////////////////////
// CMyHelp.h : header file
//

#if !defined(AFX_MVHELP_H__9A7F0482_A603_49F9_9FC2_3180C705CB9A__INCLUDED_)
#define AFX_MVHELP_H__9A7F0482_A603_49F9_9FC2_3180C705CB9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMyHelp : public CDialog
{
// Construction
public:
	CMyHelp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMyHelp)
	enum { IDD = IDD_MYHELP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyHelp)
	public:
	virtual BOOL DestroyWindow();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMyHelp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CMyHelp *MVHelpDlg;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_MVHELP_H__9A7F0482_A603_49F9_9FC2_3180C705CB9A__INCLUDED_)
