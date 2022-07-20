// w95_cl_tiers.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_W95_TIERS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
#define AFX_W95_TIERS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxwin.h"
#include "afxcmn.h"

/////////////////////////////////////////////////////////////////////////////
// CClanTiers dialog

class CClanTiers : public CDialog
{
// Construction
public:
	CClanTiers(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClanTiers)
	enum { IDD = IDD_CLAN_TIERS };
	char	res;
	BOOL	m_ExcludeAllMain;
	BOOL	m_Include;
	BOOL	m_Exclude;
	BOOL	m_MainTier;
	BOOL	m_DepTier;
	BOOL	m_HeadTier;
	BOOL	m_RoleTier;
	BOOL	m_IDTier;
	CString	m_TierName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClanTiers)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	CButton	m_MainTierCTRL;
	CButton	m_DepTierCTRL;
	CButton	m_HeadTierCTRL;
	CButton	m_RoleTierCTRL;
	CButton	m_IDTierCTRL;
	CEdit	m_TierNameCTRL;

	// Generated message map functions
	//{{AFX_MSG(CClanTiers)
	afx_msg void OnExcludeAllMain();
	afx_msg void OnInclude();
	afx_msg void OnExclude();
	afx_msg void OnMainTier();
	afx_msg void OnDepTier();
	afx_msg void OnHeadTier();
	afx_msg void OnRoleTier();
	afx_msg void OnIDTier();
	afx_msg void OnMoreChoices();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CClanTiersSelectFile dialog

class CClanTiersSelectFile : public CDialog
{
// Construction
public:
	CClanTiersSelectFile(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClanTiersSelectFile)
	enum { IDD = IDD_CLAN_TIERS_SELECT_FILES };
	char	res;
	CString	m_FilePat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClanTiersSelectFile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CEdit m_FilePatCTRL;

	// Generated message map functions
	//{{AFX_MSG(CClanTiersSelectFile)
	afx_msg void OnFilePat();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CClanTiersSelect dialog

class CClanTiersSelect : public CDialog
{
// Construction
public:
	CClanTiersSelect(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClanTiersSelect)
	enum { IDD = IDD_CLAN_TIERS_SELECT };
	int		pageN;
	int		ProgNum;
	int		ReadyToLoadState;
	time_t	GlobalTime;
	BOOL	m_ExcludeSpeakerData;
	BOOL	m_IncludeHeaders;
	BOOL	m_SelectAllSpeaker;
	BOOL	m_SelectAllDep;
	BOOL	m_T1;
	BOOL	m_T2;
	BOOL	m_T3;
	BOOL	m_T4;
	BOOL	m_T5;
	BOOL	m_T6;
	BOOL	m_T7;
	BOOL	m_T8;
	BOOL	m_T9;
	BOOL	m_T10;
	BOOL	m_T11;
	BOOL	m_T12;
	BOOL	m_T13;
	BOOL	m_T14;
	BOOL	m_T15;
	BOOL	m_T16;
	BOOL	m_T17;
	BOOL	m_T18;
	BOOL	m_T19;
	BOOL	m_T20;
	BOOL	m_T21;
	BOOL	m_T22;
	BOOL	m_T23;
	BOOL	m_T24;
	BOOL	m_T25;
	BOOL	m_T26;
	BOOL	m_T27;
	BOOL	m_T28;
	BOOL	m_T29;
	BOOL	m_T30;
	CString	m_PageNum;
	CString	m_Reading;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClanTiersSelect)
	public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void setNameOfButton(struct TiersListS *p, int which, int nCmdShow, BOOL val);
	void getTierNamesFromFile(short pixelLen, char *fname);
	void makeTiersList(int ProgNum, short pixelLen);

	int  cnt;

	CButton	m_ExcludeSpeakerDataCTRL;
	CButton	m_IncludeHeadersCTRL;
	CButton	m_SelectAllSpeakerCTRL;
	CButton	m_SelectAllDepCTRL;
	CButton	m_SelectPrevPageCTRL;
	CButton	m_SelectNextPageCTRL;
	CButton	m_T1_CTRL;
	CButton	m_T2_CTRL;
	CButton	m_T3_CTRL;
	CButton	m_T4_CTRL;
	CButton	m_T5_CTRL;
	CButton	m_T6_CTRL;
	CButton	m_T7_CTRL;
	CButton	m_T8_CTRL;
	CButton	m_T9_CTRL;
	CButton	m_T10_CTRL;
	CButton	m_T11_CTRL;
	CButton	m_T12_CTRL;
	CButton	m_T13_CTRL;
	CButton	m_T14_CTRL;
	CButton	m_T15_CTRL;
	CButton	m_T16_CTRL;
	CButton	m_T17_CTRL;
	CButton	m_T18_CTRL;
	CButton	m_T19_CTRL;
	CButton	m_T20_CTRL;
	CButton	m_T21_CTRL;
	CButton	m_T22_CTRL;
	CButton	m_T23_CTRL;
	CButton	m_T24_CTRL;
	CButton	m_T25_CTRL;
	CButton	m_T26_CTRL;
	CButton	m_T27_CTRL;
	CButton	m_T28_CTRL;
	CButton	m_T29_CTRL;
	CButton	m_T30_CTRL;
	CEdit	m_ReadingCTRL;

	// Generated message map functions
	//{{AFX_MSG(CClanTiersSelect)
	afx_msg void OnExcludeSpeakerData();
	afx_msg void OnIncludeHeaders();
	afx_msg void OnSelectAllSpeaker();
	afx_msg void OnSelectAllDep();
	afx_msg void OnSelectPrevPage();
	afx_msg void OnSelectNextPage();
	afx_msg void OnSelectT1();
	afx_msg void OnSelectT2();
	afx_msg void OnSelectT3();
	afx_msg void OnSelectT4();
	afx_msg void OnSelectT5();
	afx_msg void OnSelectT6();
	afx_msg void OnSelectT7();
	afx_msg void OnSelectT8();
	afx_msg void OnSelectT9();
	afx_msg void OnSelectT10();
	afx_msg void OnSelectT11();
	afx_msg void OnSelectT12();
	afx_msg void OnSelectT13();
	afx_msg void OnSelectT14();
	afx_msg void OnSelectT15();
	afx_msg void OnSelectT16();
	afx_msg void OnSelectT17();
	afx_msg void OnSelectT18();
	afx_msg void OnSelectT19();
	afx_msg void OnSelectT20();
	afx_msg void OnSelectT21();
	afx_msg void OnSelectT22();
	afx_msg void OnSelectT23();
	afx_msg void OnSelectT24();
	afx_msg void OnSelectT25();
	afx_msg void OnSelectT26();
	afx_msg void OnSelectT27();
	afx_msg void OnSelectT28();
	afx_msg void OnSelectT29();
	afx_msg void OnSelectT30();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern void TiersDialog(unCH *str, unCH *fbufferU);

#endif // !defined(AFX_W95_TIERS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
