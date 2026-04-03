// w95_cl_datasets.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_W95_EVAL_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
#define AFX_W95_EVAL_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxwin.h"
#include "afxcmn.h"

/////////////////////////////////////////////////////////////////////////////
// CClanDataset dialog

class CClanDataset: public CDialog
{
	DECLARE_DYNAMIC(CClanDataset)

public:
	CClanDataset(CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
	enum { IDD = IDD_CLAN_DATASET };
	CListBox m_ClanDatasetListControl;
	char datasetArg[64];

protected:
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	// Implementation
protected:
	void listDatasets(char *f);
	void ResizeDatasetWindow(int cx, int cy);

	// Generated message map functions
	//{{AFX_MSG(CClanDataset)
	afx_msg void OnDblclkClanDatasets();
	afx_msg void OnSelchangeClanDatasets();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_W95_EVAL_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
