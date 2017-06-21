// w95_commands.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_W95_COMMANDS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
#define AFX_W95_COMMANDS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "c_clan.h"
#include "afxwin.h"
#include "afxcmn.h"

/////////////////////////////////////////////////////////////////////////////
// CClanWindow dialog

class CClanWindow : public CDialog
{
// Construction
public:
	CClanWindow(CWnd* pParent = NULL);   // standard constructor
	wchar_t t_st[FNSize], wd_st[45], od_st[45], lib_st[45], mor_lib_st[45];

// Dialog Data
	//{{AFX_DATA(CClanWindow)
	enum { IDD = IDD_CLAN };
	CEdit	m_CommandsControl;
	CString	m_LibSt;
	CString	m_OdSt;
	CString	m_WdSt;
	CString	m_Commands;
	CString	m_morLibSt;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClanWindow)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	char fbuffer[EXPANSION_SIZE + 2];
	unCH fbufferU[EXPANSION_SIZE + 2];
	CComboBox m_ProgsCtrl;
	CString m_ProgS;
	CButton optionsButtonCTRL;
	CButton searchButtonCTRL;
	CButton fileinButtonCTRL;
	CButton fileoutButtonCTRL;
	void AddToClan_commands(char *st);
	void RecallCommand(short type);
	void HideClanWinIcons(void);
	void SetClanWinIcons(void);
	void createPopupProgsMenu(void);

	// Generated message map functions
	//{{AFX_MSG(CClanWindow)
	afx_msg void OnUpdateClanCommands();
	afx_msg void OnClanProgs();
	afx_msg void OnClanProgsCancel();
	afx_msg void OnClanHelp();
	afx_msg void OnClanLib();
	afx_msg void OnClanOd();
	afx_msg void OnClanRun();
	afx_msg void OnClanWd();
	afx_msg void OnClanProg();
	afx_msg void OnClanFilein();
	afx_msg void OnClanFileout();
	afx_msg void OnClanOpt();
	afx_msg void OnClanSearch();
	afx_msg void OnChangeClanCommands();
	afx_msg void OnClanMorLib();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClanRecall();
};
/////////////////////////////////////////////////////////////////////////////
// CClanRecall dialog

class CClanRecall : public CDialog
{
	DECLARE_DYNAMIC(CClanRecall)

public:
	CClanRecall(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_CLAN_RECALL };
	CListBox	m_ClanRecallListControl;

protected:
	BOOL m_isInited;
	virtual void OnCancel();
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

// Implementation
protected:
	void ResizeRecallWindow(int cx, int cy);

	// Generated message map functions
	//{{AFX_MSG(CClanProgs)
	afx_msg void OnDblclkClanRecalls();
	afx_msg void OnSelchangeClanRecalls();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CClanWindow *clanDlg;
extern char isFindFile(int ProgNum);

#endif // !defined(AFX_W95_COMMANDS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
