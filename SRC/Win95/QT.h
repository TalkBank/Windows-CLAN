#if !defined(AFX_QT_H__A9FF1061_57CA_43A9_BEF4_2192F5459580__INCLUDED_)
#define AFX_QT_H__A9FF1061_57CA_43A9_BEF4_2192F5459580__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QT.h : header file
//
#include "stdafx.h"
#include <TextUtils.h>

#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CQT dialog
class CQT : public CDialog
{
// Construction
public:
	CQT(CString pathName,CWnd* pParent = NULL,BOOL SetOrg=true);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQT)
	enum { IDD = IDD_QT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQT)
	public:
	virtual BOOL DestroyWindow();
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
	public:
	BOOL mcActionType;
	BOOL isSetOrg;
	BOOL Open();
	BOOL IsQTMovieDone();
	BOOL EventProc(UINT message, WPARAM wParam, LPARAM lParam);
	void Play();
	void Close();
	void SetStartTime(TimeValue);
	void SetDuration(TimeValue);
	void FakeMove(TimeValue);
	void Movie_Load(long start, char isPreLoad);
	void Movie_PreLoad(long start);
	TimeValue GetStartTime();
	TimeValue GetDuration();
	TimeValue GetCurTime();
	TimeValue GetQTMovieDuration();
	CGrafPtr cgp;
	Movie theMovie;
	Rect orgMvBounds;
	Rect MvBounds;
	char isPlaying;
	char isFirstLoad;
	MovieController mc;

	protected:
	LPCTSTR filename;
	FSSpec fss;
	FSRef  fref;
	short frefnum;
	BOOL havemc, havemovie;
	TimeValue curTime,startTime, endTime, duration, 
		movieDuration, backTime;
	TimeScale timescale;


	// Generated message map functions
	//{{AFX_MSG(CQT)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QT_H__A9FF1061_57CA_43A9_BEF4_2192F5459580__INCLUDED_)
