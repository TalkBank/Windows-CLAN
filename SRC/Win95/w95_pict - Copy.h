#if !defined(AFX_W_PICT_H__72723264_43E5_4530_9FEC_EB0DB05E468D__INCLUDED_)
#define AFX_W_PICT_H__72723264_43E5_4530_9FEC_EB0DB05E468D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// W_Pict.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPict dialog

class CPict : public CDialog
{
// Construction
public:
	CPict(CString pathName, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPict)
	enum { IDD = IDD_PICT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPict)
	public:
	virtual BOOL DestroyWindow();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	char fname[_MAX_PATH+_MAX_FNAME];
	char setSize;
	Rect myRect;
	CGrafPtr cgp;
	GraphicsImportComponent	gImporter;

protected:

	// Generated message map functions
	//{{AFX_MSG(CPict)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W_PICT_H__72723264_43E5_4530_9FEC_EB0DB05E468D__INCLUDED_)
