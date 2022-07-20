#if !defined(AFX_SPCHARSDIALOG_H__18CD6110_3B02_4B0F_869A_ACC9ACC97E2F__INCLUDED_)
#define AFX_SPCHARSDIALOG_H__18CD6110_3B02_4B0F_869A_ACC9ACC97E2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpCharsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpCharsDialog dialog

class CSpCharsDialog : public CDialog
{
// Construction
public:
	CSpCharsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSpCharsDialog)
	enum { IDD = IDD_CLAN_SP_CHARS };
	CListBox	m_Sp_Chars_Control;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpCharsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ResizeSpCharWindow(int cx, int cy);

	// Generated message map functions
	//{{AFX_MSG(CSpCharsDialog)
	afx_msg void OnSelchangeClanSpChars();
	afx_msg void OnDblclkClanSpChars();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CSpCharsDialog *spCharsDlg;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPCHARSDIALOG_H__18CD6110_3B02_4B0F_869A_ACC9ACC97E2F__INCLUDED_)
