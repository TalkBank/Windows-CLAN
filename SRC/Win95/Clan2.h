// Clan2.h : main header file for the CLAN2 application
//

#if !defined(AFX_CLAN2_H__EB2FA985_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
#define AFX_CLAN2_H__EB2FA985_D6AD_11D0_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#ifdef _DEBUG
#define isspace win95_isspace
extern int win95_isspace(char c);
#endif

#include "resource.h"       // main symbols

extern LOGFONT m_lfDefFont;
extern CFont m_font;
extern CDC* GlobalDC;
extern void cedMoveCaret(void);
extern SHORT bApKey;
extern SHORT TildKey;
extern SHORT ApKey;
extern SHORT HatKey;
extern SHORT AtKey;
extern SHORT ColKey;
extern SHORT SemiColKey;
extern SHORT ComaKey;
extern SHORT DotKey;
extern SHORT StarKey;
extern SHORT QuestKey;
extern SHORT AndKey;
extern SHORT SlashKey;
extern SHORT EqSignKey;
extern SHORT OpSQBKey;
extern SHORT ClSQBKey;
extern SHORT OpAGBKey;
extern SHORT ClAGBKey;


extern "C" {
extern void FinishMainLoop(void);
extern void AdjustWindowSize(RECT *r);
}
/////////////////////////////////////////////////////////////////////////////
// CClan2App:
// See Clan2.cpp for the implementation of this class
//

class CMPegDoc;

class CClan2App : public CWinApp
{
public:
	CClan2App();

// Notify when document created / destroyed
public:
	BOOL myPumpMessage();
protected:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClan2App)
	public:
	virtual BOOL InitInstance();
	virtual int  ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual int Run();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CClan2App)
	afx_msg void OnAppAbout();
	afx_msg void OnEditOptionsCedoptions();
	afx_msg void OnClanClanwindow();
	afx_msg void OnFileOpenMovie();
	afx_msg void OnFileNewMovie();
	afx_msg void myOnFileNew();
	afx_msg void OnEditUrlAdd();
	afx_msg void OnEditThumbnails();
	afx_msg void OnEditResetOptions();
	afx_msg void OnFuncShortcuts();
	afx_msg void OnFuncsShortcutsFile();
	afx_msg void OnClanCaChars();
	afx_msg void OnWebData();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLAN2_H__EB2FA985_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
