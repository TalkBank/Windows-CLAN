// w95_cl_eval.h : header file
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
// CClanEval dialog

class CClanEval : public CDialog
{
// Construction
public:
	CClanEval(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClanEval)
	enum { IDD = IDD_CLAN_EVAL_SELECT };
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
	BOOL	m_ANOMIC;
	BOOL	m_GLOBAL;
	BOOL	m_BROCA;
	BOOL	m_WERNICKE;
	BOOL	m_TRANS;
	BOOL	m_TRANM;
	BOOL	m_CONDUCTION;
	BOOL	m_CONTROL;
	BOOL	m_NOTAPH;
	CString	m_AGE_RANGE;
	BOOL	m_MALE;
	BOOL	m_FEMALE;
	BOOL	m_SPEECH;
	BOOL	m_STROKE;
	BOOL	m_WINDOW;
	BOOL	m_IMPEVENT;
	BOOL	m_UMBRELLA;
	BOOL	m_CAT;
	BOOL	m_FLOOD;
	BOOL	m_CINDERELLA;
	BOOL	m_SANDWICH;
	BOOL	m_COMMUNICATION;
	BOOL	m_ILLNESS;
	CString	m_Reading;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClanEval)
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
	//{{AFX_MSG(CClanEval)
	afx_msg void OnDDB();
	afx_msg void OnUDB();
	afx_msg void OnAnomic();
	afx_msg void OnGlobal();
	afx_msg void OnControl();
	afx_msg void OnBroca();
	afx_msg void OnWernicke();
	afx_msg void OnConduction();
	afx_msg void OnTrans();
	afx_msg void OnTRANM();
	afx_msg void OnNotaph();
	afx_msg void OnMale();
	afx_msg void OnFemale();
	afx_msg void OnFluent();
	afx_msg void OnNonFluent();
	afx_msg void OnAllAphasia();
	afx_msg void OnDGEM();
	afx_msg void OnSGEM();
	afx_msg void OnSpeech();
	afx_msg void OnStroke();
	afx_msg void OnWindow();
	afx_msg void OnImpEvent();
	afx_msg void OnUmbrella();
	afx_msg void OnCat();
	afx_msg void OnFlood();
	afx_msg void OnCinderella();
	afx_msg void OnSandwich();
	afx_msg void OnCommunication();
	afx_msg void OnIllness();
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
};

extern void EvalDialog(unCH *str);

#endif // !defined(AFX_W95_EVAL_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
