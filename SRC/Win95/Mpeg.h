#if !defined(AFX_MPEG_H__A9FF1061_57CA_43A9_BEF4_2192F5459580__INCLUDED_)
#define AFX_MPEG_H__A9FF1061_57CA_43A9_BEF4_2192F5459580__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// CMpeg dialog

class CMpeg : public CDialog
{
	DECLARE_DYNAMIC(CMpeg)

public:
	CMpeg(CString pathName, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMpeg();

// Dialog Data
	enum { IDD = IDD_MPEG };

protected:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMpeg)
public:
	virtual BOOL DestroyWindow();
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	CAxWindow		m_wndView;
	IWMPPlayer		*m_wmpPlayer;
	IWMPControls	*m_wmpControler;
	IConnectionPoint *m_spConnectionPoint;
	DWORD			m_dwAdviseCookie;
	BOOL isMovieDurationSet;
	BOOL mcActionType;
	BOOL Open(void);
	BOOL IsMpegMovieDone(void);
//	BOOL EventProc(UINT message);
	void setVolume(long vol);
	void Play(void);
	void StopMpegMovie(void);
	void Close(void);
	void SetStartTime(long start);
	void SetDuration(long howlong);
	void SetRawDuration(double dur);
	void FakeMove(long start);
	char myWP(void);
	long GetStartTime(void);
	long GetCurTime(void);
	long GetDuration(void);
	long GetMpegMovieDuration(void);
	char isPlaying;

protected:
	WCHAR filename[FNSize];
	BOOL  havemovie;
	double curTime, startTime, backTime, duration, movieDuration;

	// Generated message map functions
	//{{AFX_MSG(CMpeg)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_MPEG_H__A9FF1061_57CA_43A9_BEF4_2192F5459580__INCLUDED_)
