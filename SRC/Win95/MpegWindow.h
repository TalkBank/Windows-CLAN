//{{AFX_INCLUDES()
#include "mediaplayer.h"
//}}AFX_INCLUDES
#if !defined(AFX_MPEGWINDOW_H__249A8540_0862_11D3_847B_00A02481E866__INCLUDED_)
#define AFX_MPEGWINDOW_H__249A8540_0862_11D3_847B_00A02481E866__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MpegWindow.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CMpegWindow dialog

class CMpegWindow : public CDialog
{

// Construction
public:
	CMpegWindow(CWnd* pParent = NULL);   // standard constructor
// Dialog Data
	//{{AFX_DATA(CMpegWindow)
	enum { IDD = IDD_MPEG };
	CEdit	m_ToCtrl;
	CEdit	m_PosCtrl;
	CEdit	m_FromCtrl;
	CEdit	m_DelayCtrl;
	CSpinButtonCtrl	m_spin;
	CMediaPlayer	m_player;
	CString	m_rptstr;
	UINT	m_from;
	UINT	m_pos;
	UINT	m_to;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMpegWindow)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	UINT movieDuration;
	BOOL isPlayRange;
	void ExecuteCommNumber(int num);
	DWORD tDelay;
	int keyDNcount;
	CDC* textDC;
	CClan2Doc* textDoc;
	void Save(){OnSaveButton(); return;};
	void Play();

protected:
	UINT m_inFocus;
	BOOL m_PlayToEnd;
	CBitmapButton myhelpButton;
	BOOL playing,opened;
	UINT timer;
	long m_isUserResize;
	Rect orgMvBounds;
	Rect MvBounds;
	BOOL ShowToolTip( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	void changeMovieWinSize(long DisplaySize, BOOL isSetOrgSize);
	void ResizeMovie(short w, short isRedo, BOOL isRefreshQT);
	
	// Generated message map functions
	//{{AFX_MSG(CMpegWindow)
	afx_msg void OnBegButton();
	afx_msg void OnEndButton();
	afx_msg void OnSaveButton();
	afx_msg void OnPlayButton();
	afx_msg void OnMpgHelp();
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPlayStateChangeMediaplayer1(long OldState, long NewState);
	afx_msg void OnPositionChangeMediaplayer1(double oldPosition, double newPosition);
	afx_msg void OnOpenStateChangeMediaplayer1(long OldState, long NewState);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetfocusMpgEditDelay();
	afx_msg void OnSetfocusMpgEditFrom();
	afx_msg void OnSetfocusMpgEditPos();
	afx_msg void OnSetfocusMpgEditTo();
	afx_msg void OnChangeCursorPos();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MPEGWINDOW_H__249A8540_0862_11D3_847B_00A02481E866__INCLUDED_)
