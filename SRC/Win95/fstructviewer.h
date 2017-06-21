#if !defined(AFX_CFSTRUCTVIEWER_H__0C90E9B9_F311_48C4_BCE3_9D27BCF2728A__INCLUDED_)
#define AFX_CFSTRUCTVIEWER_H__0C90E9B9_F311_48C4_BCE3_9D27BCF2728A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FStructViewer.h : header file
//

#include "Clan2Doc.h"

/////////////////////////////////////////////////////////////////////////////
// cFStructViewer dialog

class cFStructViewer : public CDialog
{
// Construction
public:
	cFStructViewer(CWnd* pParent = NULL);   // standard constructor
	CDC* textDC;
	CClan2Doc* textDoc;

// Dialog Data
	//{{AFX_DATA(cFStructViewer)
	enum { IDD = IDD_FSTRUCT_VIEWER };
	CTreeCtrl	m_tree;
	CString	m_DepTier;
	CString	m_SpTier;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cFStructViewer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
public:
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cFStructViewer)
	afx_msg void OnExpandAll();
	afx_msg void OnCollapseAll();
	afx_msg void OnBad();
	afx_msg void OnGood();
	afx_msg void OnSkipNext();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern cFStructViewer *FViewerDlg;
extern int BuildTree( CTreeCtrl& tree, CString& str );
extern char ShowFStruct(void);
extern void ExpandTree( CTreeCtrl* tree, HTREEITEM item, UINT code );

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CFSTRUCTVIEWER_H__0C90E9B9_F311_48C4_BCE3_9D27BCF2728A__INCLUDED_)
