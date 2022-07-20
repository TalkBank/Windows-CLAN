// w95_cl_search.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_W95_SEARCH_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
#define AFX_W95_SEARCH_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxwin.h"
#include "afxcmn.h"

/////////////////////////////////////////////////////////////////////////////
// CClanSearch dialog

class CClanSearch : public CDialog
{
// Construction
public:
	CClanSearch(CWnd* pParent = NULL);   // standard constructor
	int spItem;
	int numberPresets;
	unCH fname[FNSize];

// Dialog Data
	//{{AFX_DATA(CClanSearch)
	enum { IDD = IDD_CLAN_SEARCH };
	BOOL	m_Include;
	BOOL	m_Exclude;
	BOOL	m_File;
	CString	m_FName;
	BOOL	m_Word;
	BOOL	m_Code;
	BOOL	m_Postcode;
	BOOL	m_LangCode;
	BOOL	m_CodeItself;
	BOOL	m_CodeData;
	CString	m_WordStr;
	BOOL	m_Mor;
	CString	m_Mor_pos_e;
	CString	m_Mor_stem_e;
	CString	m_Mor_pref_e;
	CString	m_Mor_suf_e;
	CString	m_Mor_fus_e;
	CString	m_Mor_tran_e;
	BOOL	m_Mor_o;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClanSearch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	CEdit	m_FNameCTRL;
	CButton	m_CodeItselfCTRL;
	CButton	m_CodeDataCTRL;
	CEdit	m_WordStrCTRL;
	CComboBox m_PresetsCTRL;
	CString m_Presets;
	CStatic m_Mor_pos_sCTRL;
	CEdit	m_Mor_pos_eCTRL;
	CStatic m_Mor_stem_sCTRL;
	CEdit	m_Mor_stem_eCTRL;
	CStatic m_Mor_pref_sCTRL;
	CEdit	m_Mor_pref_eCTRL;
	CStatic m_Mor_suf_sCTRL;
	CEdit	m_Mor_suf_eCTRL;
	CStatic m_Mor_fus_sCTRL;
	CEdit	m_Mor_fus_eCTRL;
	CStatic m_Mor_tran_sCTRL;
	CEdit	m_Mor_tran_eCTRL;
	CButton	m_Mor_oCTRL;

	// Generated message map functions
	//{{AFX_MSG(CClanSearch)
	afx_msg void OnMoreHelp();
	afx_msg void OnInclude();
	afx_msg void OnExclude();
	afx_msg void OnFileOpen();
	afx_msg void OnFile();
	afx_msg void OnWord();
	afx_msg void OnCode();
	afx_msg void OnPostcode();
	afx_msg void OnLangCode();
	afx_msg void OnCodeItself();
	afx_msg void OnCodeData();
	afx_msg void OnMor();
	afx_msg void OnPresets();
	afx_msg void OnPresetsFocus();
	afx_msg void OnMor_o();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void ctrlAllMor(int nCmdShow);
	void createPopupPresetsMenu();
};

extern void SearchDialog(unCH *str);

#endif // !defined(AFX_W95_SEARCH_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
