// w95_cl_evald.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_W95_EVALD_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
#define AFX_W95_EVALD_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxwin.h"
#include "afxcmn.h"

/////////////////////////////////////////////////////////////////////////////
// CClanEvald dialog

class CClanEvald : public CDialog
{
// Construction
public:
	CClanEvald(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClanEvald)
	enum { IDD = IDD_CLAN_EVALD_SELECT };
	int		ReadyToLoadState;
	char	isFoundFile;
	time_t	GlobalTime;
	BOOL	m_T1;
	BOOL	m_T2;
	BOOL	m_T3;
	BOOL	m_T4;
	BOOL	m_T5;
	BOOL	m_T6;
	BOOL	m_T7;
	BOOL	m_T8;
	BOOL	m_CONTROL;
	BOOL	m_MCI;
	BOOL	m_MEMORY;
	BOOL	m_POSSIBLEAD;
	BOOL	m_PROBABLEAD;
	BOOL	m_VASCULAR;
	CString	m_AGE_RANGE;
	BOOL	m_MALE;
	BOOL	m_FEMALE;
	BOOL	m_CAT;
	BOOL	m_COOKIE;
	BOOL	m_HOMETOWN;
	BOOL	m_ROCKWELL;
	BOOL	m_CINDERELLA;
	BOOL	m_SANDWICH;
	CString	m_Reading;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClanEvald)
	public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void setNameOfButton(struct SpeakersListS *p, int which, int nCmdShow, BOOL val);
	void getTierNamesFromFile(char *fname);
	void makeSpeakersList();

	int  cnt;

	CButton	m_T1_CTRL;
	CButton	m_T2_CTRL;
	CButton	m_T3_CTRL;
	CButton	m_T4_CTRL;
	CButton	m_T5_CTRL;
	CButton	m_T6_CTRL;
	CButton	m_T7_CTRL;
	CButton	m_T8_CTRL;
	CEdit	m_AGE_RANGECTRL;
	CEdit	m_ReadingCTRL;

	// Generated message map functions
	//{{AFX_MSG(CClanEvald)
	afx_msg void OnDDB();
	afx_msg void OnControl();
	afx_msg void OnMCI();
	afx_msg void OnMemory();
	afx_msg void OnPossibleAD();
	afx_msg void OnProbableAD();
	afx_msg void OnVascular();
	afx_msg void OnMale();
	afx_msg void OnFemale();
	afx_msg void OnDGEM();
	afx_msg void OnSGEM();
	afx_msg void OnCat();
	afx_msg void OnCinderella();
	afx_msg void OnSandwich();
	afx_msg void OnCookie();
	afx_msg void OnHometown();
	afx_msg void OnRockwell();
	afx_msg void OnSelectT1();
	afx_msg void OnSelectT2();
	afx_msg void OnSelectT3();
	afx_msg void OnSelectT4();
	afx_msg void OnSelectT5();
	afx_msg void OnSelectT6();
	afx_msg void OnSelectT7();
	afx_msg void OnSelectT8();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEvaldAge();
};

extern void EvaldDialog(unCH *str);

#endif // !defined(AFX_W95_EVAL_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
