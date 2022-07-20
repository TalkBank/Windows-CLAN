// Clan2View.h : interface of the CClan2View class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLAN2VIEW_H__EB2FA98F_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
#define AFX_CLAN2VIEW_H__EB2FA98F_D6AD_11D0_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CClan2View : public CView
{
protected: // create from serialization only
	CClan2View();
	DECLARE_DYNCREATE(CClan2View)
	char isExtendedKey(UINT nChar, UINT nRepCnt, UINT nFlags);
	void call_win95_call(UINT nChar, UINT nRepCnt, UINT nFlags, UINT extended);

// Attributes
public:
	CClan2Doc* GetDocument();
	// static init/term...
	static void Initialize();
	static void Terminate();

protected:
	int cursorCnt;
	char skipOnChar;
	unsigned char isExtKeyFound; // 1- `, 2- ~, 3- ', 4- ^, 5- @, 6- :, 7- ,, 8- &, 9- /
								 // 1- `, 2- ~, 4- ', 8- ^, 16- @, 32- :, 64- ,, 128- &
	char text_isFKeyPressed;
	UINT whichInput;

// Operations
public:
	UINT m_lp;
	UINT hPrintScale, vPrintScale;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClan2View)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CClan2View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:

	// Generated message map functions
protected:
	//{{AFX_MSG(CClan2View)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnChooseFont();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelectMediaFile();

	afx_msg void OnGetEnglishGrammar();
	afx_msg void OnGetCantoneseGrammar();
	afx_msg void OnGetChineseGrammar();
	afx_msg void OnGetDanishGrammar();
	afx_msg void OnGetDutchGrammar();
	afx_msg void OnGetFrenchGrammar();
	afx_msg void OnGetGermanGrammar();
	afx_msg void OnGetHebrewGrammar();
	afx_msg void OnGetItalianGrammar();
	afx_msg void OnGetJapaneseGrammar();
	afx_msg void OnGetSpanishGrammar();

	afx_msg void OnGetEnglishKevalDB();
	afx_msg void OnGetChineseKevalDB();

	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnChooseDefaultFont();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnSelectAll();
	afx_msg void OnEditFind();
	afx_msg void OnEditEnterSelection();
	afx_msg void OnEditFindSame();
	afx_msg void OnEditGoto();
	afx_msg void OnEditReplace();
	afx_msg void OnEditToLower();
	afx_msg void OnEditToUpper();
	afx_msg void OnModeChatmode();
	afx_msg void OnModeCheckfile();
	afx_msg void OnModeCoder();
	afx_msg void OnModeLinksnd();
	afx_msg void OnModePlaycont();
	afx_msg void OnModeSoundmode();
	afx_msg void OnMacros1();
	afx_msg void OnMacros10();
	afx_msg void OnMacros2();
	afx_msg void OnMacros3();
	afx_msg void OnMacros4();
	afx_msg void OnMacros5();
	afx_msg void OnMacros6();
	afx_msg void OnMacros8();
	afx_msg void OnMacros9();
	afx_msg void OnSpeakers1();
	afx_msg void OnSpeakers10();
	afx_msg void OnSpeakers2();
	afx_msg void OnSpeakers3();
	afx_msg void OnSpeakers4();
	afx_msg void OnSpeakers5();
	afx_msg void OnSpeakers6();
	afx_msg void OnSpeakers7();
	afx_msg void OnSpeakers8();
	afx_msg void OnSpeakers9();
	afx_msg void OnSpeakersUpdate();
	afx_msg void OnMacros7();
	afx_msg void OnModeDisambig();
	afx_msg void OnModeTierExclusion();
	afx_msg void OnModePlaysound();
	afx_msg void OnModeSoundtotextsync();
	afx_msg void OnModeHideTiers();
	afx_msg void OnModeExpendBullets();
	afx_msg void OnEditRedo();
	afx_msg void OnUnderline();
	afx_msg void OnEditReplacefind();
	afx_msg void OnItalic();
	afx_msg void OnModeCamode();
	afx_msg void OnSetTab();
	afx_msg void OnChooseCommandFont();
	afx_msg void OnModeTransmedia();
	afx_msg void OnModeSendToPraat();
	afx_msg void OnFileSaveClip();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLinenumberSize();
	afx_msg void OnSoundAnalizerApp();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnPlaybackControl();
	afx_msg void OnModeUtf8();
	afx_msg void OnF5Option();
	afx_msg void OnPlain();
	afx_msg void OnBold();
	afx_msg void OnColorWords();
	afx_msg void OnColorKeywords();
	afx_msg void OnToggleMovieText();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTiersIdheaders();
public:
	afx_msg void OnModePlaycontskip();
};

#ifndef _DEBUG  // debug version in Clan2View.cpp
inline CClan2Doc* CClan2View::GetDocument()
   { return (CClan2Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLAN2VIEW_H__EB2FA98F_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
