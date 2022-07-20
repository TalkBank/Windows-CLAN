// MpegWindow.h : header file
//

#if !defined(AFX_MPEGWINDOW_H__249A8540_0862_11D3_847B_00A02481E866__INCLUDED_)
#define AFX_MPEGWINDOW_H__249A8540_0862_11D3_847B_00A02481E866__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPeg.h"
#include "Clan2Doc.h"
/////////////////////////////////////////////////////////////////////////////
// CMpegDlg dialog

class CMpegDlg : public CDialog
{

// Construction
public:
	CMpegDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMpegDlg)
	enum { IDD = IDD_MPEG_DIALOG };
	CEdit	m_ToCtrl;
	CEdit	m_PosCtrl;
	CEdit	m_FromCtrl;
	CEdit	m_DelayCtrl;
	CSpinButtonCtrl	m_spin;
	long	m_from,
			m_pos,
			m_to,
			last_m_pos,
			same_m_posValue;
	CString	m_rptstr;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMpegDlg)
public:
	virtual BOOL DestroyWindow();
	virtual void OnCancel();
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	CMpeg *mpeg;
	BOOL isPlayRange;
	BOOL m_PlayToEnd;
	BOOL isJustOpen;
	BOOL m_isUserResize;
	int MoviePlayLoopCnt;
	CString pathname;
	movInfo *mvRec;
	void Play();
	void SetCurPos(char passive);
	void ExecuteCommNumber(int num);
	CClan2Doc* textDoc;
	void Save(){OnSaveButton(); return;};
	void SetCurPosToValue(long tTime);
	void SetWaveTimeValue(UINT timeBeg, UINT timeEnd);
	void PlaySetting(void);
	BOOL doErase;
	CDC* textDC;
	DWORD tDelay;
	int keyDNcount;
	long real_to;
	long CTRLoffset;
	RECT MvBounds;

protected:
	UINT m_inFocus;
	BOOL ShowToolTip( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	void ResizeMpegDlgWindow(CMpeg *mpeg);
	void OnToggleMovieText();
	HICON m_hIcon;
	CBitmapButton myhelpButton;
#ifdef USETIMER
	UINT timer;
#endif

	// Generated message map functions
	//{{AFX_MSG(CMpegDlg)
	afx_msg void OnSaveButton();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPlayButton();
	afx_msg void OnBegButton();
	afx_msg void OnEndButton();
	afx_msg void OnUpdateEditPos();
	afx_msg void OnMpgHelp();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetfocusMpgEditPos();
	afx_msg void OnSetfocusMpgEditTo();
	afx_msg void OnSetfocusMpgEditFrom();
	afx_msg void OnSetfocusMpgEditDelay();
	afx_msg void OnSize(UINT nType, int cx, int cy);
#ifdef USETIMER
	afx_msg void OnTimer(UINT nIDEvent);
#endif
public:
	afx_msg void OnUpdateEditFrom();
	afx_msg void OnUpdateEditTo();

//	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CMpegDlg *MpegDlg;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MPEGWINDOW_H__249A8540_0862_11D3_847B_00A02481E866__INCLUDED_)
