// w95_cl_eval.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_W95_KIDEVAL_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
#define AFX_W95_KIDEVAL_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxwin.h"
#include "afxcmn.h"

#define TOTAL_SP_NUMBER 8

/////////////////////////////////////////////////////////////////////////////
// CClanKideval dialog
/*
18-23	1;6 - 1;11
24-29	2;  - 2;5
30-35	2;6 - 2;11
36-41	3;  - 3;5
42-47	3;6 - 3;11
48-53	4;  - 4;5
54-59	4;6 - 4;11
60-65	5;  - 5;5
66-72	5;6 - 6;
*/

class CClanKideval : public CDialog
{
// Construction
public:
	CClanKideval(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClanKideval)
	enum { IDD = IDD_CLAN_KIDEVAL_SELECT };
	CString	m_Reading;
	int		ReadyToLoadState;
	char	isFoundFile;
	time_t	GlobalTime;

	BOOL	m_CmpDB;
	BOOL	m_NotCmpDB;

	BOOL	m_SP[TOTAL_SP_NUMBER];

	BOOL	m_ENG;
	BOOL	m_ENGU;
	BOOL	m_FRA;
	BOOL	m_SPA;
	BOOL	m_JPN;
	BOOL	m_YUE;
	BOOL	m_ZHO;

	int		m_AgeAuto;

	BOOL	m_IndyAge;

	BOOL	m_OneH;
	BOOL	m_Two;
	BOOL	m_TwoH;
	BOOL	m_Three;
	BOOL	m_ThreeH;
	BOOL	m_Four;
	BOOL	m_FourH;
	BOOL	m_Five;
	BOOL	m_FiveH;

	BOOL	m_MALE;
	BOOL	m_FEMALE;
	BOOL	m_BOTHGEN;

	BOOL	m_LinkAge;
	BOOL	m_DBEngToyplay;
	BOOL	m_DBEngNarrative;
	BOOL	m_DBEngUToyplay;
	BOOL	m_DBEngUNarrative;
	BOOL	m_DBZhoToyplay;
	BOOL	m_DBZhoNarrative;
	BOOL	m_DBNldToyplay;
	BOOL	m_DBFraToyplay;
	BOOL	m_DBFraNarrative;
	BOOL	m_DBJpnToyplay;
	BOOL	m_DBSpaToyplay;
	BOOL	m_DBSpaNarrative;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClanKideval)
	public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void setNameOfButton(int which, int nCmdShow, BOOL val);
	void kidevalGetTierNamesFromFile(char *fname);
	void kidevalMakeSpeakersList();
	void ResizeOptionsWindow();

	int  readFilesCnt;

	CEdit	m_ReadingCTRL;

	CEdit	m_ChooseOneCTRL;
	CEdit	m_SelSpCTRL;
	CEdit	m_SpCTRL;
	CEdit	m_AgeCTRL;
	CEdit	m_LangCTRL;
	CEdit	m_DbTypeCTRL;
	CEdit	m_ErrorMesOneCTRL;
	CEdit	m_ErrorMesTwoCTRL;
	CEdit	m_ErrorMesThrCTRL;

	CButton	m_SP_CTRL[TOTAL_SP_NUMBER];

	CButton m_ENG_CTRL;
	CButton m_ENGU_CTRL;
	CButton m_FRA_CTRL;
	CButton m_SPA_CTRL;
	CButton m_JPN_CTRL;
	CButton m_YUE_CTRL;
	CButton m_ZHO_CTRL;

	CButton m_IndyAge_CTRL;

	CButton m_OneH_CTRL;
	CButton m_Two_CTRL;
	CButton m_TwoH_CTRL;
	CButton m_Three_CTRL;
	CButton m_ThreeH_CTRL;
	CButton m_Four_CTRL;
	CButton m_FourH_CTRL;
	CButton m_Five_CTRL;
	CButton m_FiveH_CTRL;

	CButton m_MALE_CTRL;
	CButton m_FEMALE_CTRL;
	CButton m_BOTHGEN_CTRL;

	CButton	m_DBEngToyplay_CTRL;
	CButton	m_DBEngNarrative_CTRL;
	CButton	m_DBEngUToyplay_CTRL;
	CButton	m_DBEngUNarrative_CTRL;
	CButton	m_DBZhoToyplay_CTRL;
	CButton	m_DBZhoNarrative_CTRL;
	CButton	m_DBNldToyplay_CTRL;
	CButton	m_DBFraToyplay_CTRL;
	CButton	m_DBFraNarrative_CTRL;
	CButton	m_DBJpnToyplay_CTRL;
	CButton	m_DBSpaToyplay_CTRL;
	CButton	m_DBSpaNarrative_CTRL;

	CButton	m_OK_CTRL;
	CButton	m_CANCEL_CTRL;

	// Generated message map functions
	//{{AFX_MSG(CClanKideval)
	afx_msg void OnCmpDB();
	afx_msg void OnNotCmpDB();

	afx_msg void OnSelectSP1();
	afx_msg void OnSelectSP2();
	afx_msg void OnSelectSP3();
	afx_msg void OnSelectSP4();
	afx_msg void OnSelectSP5();
	afx_msg void OnSelectSP6();
	afx_msg void OnSelectSP7();
	afx_msg void OnSelectSP8();

	afx_msg void OnSelectENG();
	afx_msg void OnSelectENGU();
	afx_msg void OnSelectFRA();
	afx_msg void OnSelectSPA();
	afx_msg void OnSelectJPN();
	afx_msg void OnSelectYUE();
	afx_msg void OnSelectZHO();

	afx_msg void OnIndyAge();

	afx_msg void OnOneH();
	afx_msg void OnTwo();
	afx_msg void OnTwoH();
	afx_msg void OnThree();
	afx_msg void OnThreeH();
	afx_msg void OnFour();
	afx_msg void OnFourH();
	afx_msg void OnFive();
	afx_msg void OnFiveH();

	afx_msg void OnMale();
	afx_msg void OnFemale();
	afx_msg void OnBothGen();

	afx_msg void OnDBEngToyplay();
	afx_msg void OnDBEngNarrative();
	afx_msg void OnDBEngUToyplay();
	afx_msg void OnDBEngUNarrative();
	afx_msg void OnDBZhoToyplay();
	afx_msg void OnDBZhoNarrative();
	afx_msg void OnDBNldToyplay();
	afx_msg void OnDBFraToyplay();
	afx_msg void OnDBFraNarrative();
	afx_msg void OnDBJpnToyplay();
	afx_msg void OnDBSpaToyplay();
	afx_msg void OnDBSpaNarrative();

	afx_msg void OnOKClicked();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelectDbJpntoyplay();
};

extern void KidevalDialog(unCH *str);

#endif // !defined(AFX_W95_KIDEVAL_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
