// QTDlg.h : header file
//

#if !defined(AFX_QTDLG_H__9A7F0482_A603_49F9_9FC2_3180C705CB9A__INCLUDED_)
#define AFX_QTDLG_H__9A7F0482_A603_49F9_9FC2_3180C705CB9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QT.h"
#include "Clan2Doc.h"
/////////////////////////////////////////////////////////////////////////////
// CQTDlg dialog

class CQTDlg : public CDialog
{

// Construction
public:
	CQTDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CQTDlg)
	enum { IDD = IDD_QT_DIALOG };
	CEdit	m_ToCtrl;
	CEdit	m_PosCtrl;
	CEdit	m_FromCtrl;
	CEdit	m_DelayCtrl;
	CSpinButtonCtrl	m_spin;
	TimeValue	m_from;
	TimeValue	m_pos;
	TimeValue	m_to;
	TimeValue	last_m_pos;
	int			same_m_posValue;
	CString	m_rptstr;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQTDlg)
	public:
	virtual BOOL DestroyWindow();
	virtual void OnCancel();
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void PostNcDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL isPlayRange;
	BOOL m_PlayToEnd;
	int MoviePlayLoopCnt;
	CString pathname;
	void Play();
	void SetCurPos(char passive);
	void ExecuteCommNumber(int num);
	CClan2Doc* textDoc;
	void Save(){OnButtonSave(); return;};
	void ResizeQTDlgWindowItems(void);
	void SetCurPosToValue(long tTime);
	void SetWaveTimeValue(long timeBeg, long timeEnd);
	char doErase;
	CQT *qt;
	CDC* textDC;
	DWORD tDelay;
	int keyDNcount;
	TimeValue real_to;

protected:
	UINT m_inFocus;
	long m_isUserResize;
	BOOL ShowToolTip( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	void ResizeQTDlgWindow(CQT *qt);
	void ResizeMovie(short w, short isRedo, BOOL isRefreshQT);
	void OnToggleMovieText();
	HICON m_hIcon;
	CBitmapButton myhelpButton;

	// Generated message map functions
	//{{AFX_MSG(CQTDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSave();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonPlay();
	afx_msg void OnSetBegin();
	afx_msg void OnSetEnd();
	afx_msg void OnUpdateEditPos();
	afx_msg void OnQdHelp();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetfocusEditPos();
	afx_msg void OnSetfocusEditTo();
	afx_msg void OnSetfocusEditFrom();
	afx_msg void OnSetfocusEditDelay();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateEditFrom();
public:
	afx_msg void OnUpdateEditTo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QTDLG_H__9A7F0482_A603_49F9_9FC2_3180C705CB9A__INCLUDED_)
