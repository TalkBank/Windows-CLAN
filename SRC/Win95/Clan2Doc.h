// Clan2Doc.h : interface of the CClan2Doc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLAN2DOC_H__EB2FA98D_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
#define AFX_CLAN2DOC_H__EB2FA98D_D6AD_11D0_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CClan2Doc : public CDocument
{
protected: // create from serialization only
	CClan2Doc();
	DECLARE_DYNCREATE(CClan2Doc)

// Attributes
public:
	myFInfo	*FileInfo;
	long m_RowLimit;
	long ChangeCursorInfo;
	BOOL isTranscribing;
	unCH InsertSpChar;
	int  ExecuteCommNUM;
	char isAddFStructComm[2000];
	char RestorePlayMovieInfo;
	char DoWalkerController;
	char re_wrap;
	char redo_soundwave;
	char RedisplayWindow;
	char SaveMovieInfo;
	char SetFakeHilight;
	char isNeedResize;
	char docFName[_MAX_PATH+_MAX_FNAME];

// Operations
public:
	void myUpdateAllViews(BOOL isErase);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClan2Doc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CClan2Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CClan2Doc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CClan2Doc* GlobalDoc;
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLAN2DOC_H__EB2FA98D_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
