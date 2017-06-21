#if !defined(AFX_W_TXT_H__72723264_43E5_4530_9FEC_EB0DB05E468D__INCLUDED_)
#define AFX_W_TXT_H__72723264_43E5_4530_9FEC_EB0DB05E468D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// W95_TxtBullet.h : header file
//
#include <TextUtils.h>

#define TEXT_BULLETS_PICTS

/////////////////////////////////////////////////////////////////////////////
// cTxtBullet dialog

class cTxtBullet : public CDialog
{
// Construction
public:
	cTxtBullet(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(cTxtBullet)
	enum { IDD = IDD_TXT_FILE };
	CGrafPtr cgp;
	CBrush m_brush;
//	CTxtBulletView m_TxtControl;
//	CString	m_txt;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cTxtBullet)
	public:
	virtual void OnCancel();
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
//	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	MovieTXTInfo *SelectTextPosition(CPoint point);
	void SetTextScrollControl(void);
	void TxtHandleVScroll(UINT nSBCode, UINT nPos, CWnd* win);

	// Generated message map functions
	//{{AFX_MSG(cTxtBullet)
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
//	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
//	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
//	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W_TXT_H__72723264_43E5_4530_9FEC_EB0DB05E468D__INCLUDED_)
