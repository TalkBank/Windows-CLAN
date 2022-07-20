// CedDlgs.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CEDDLGS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
#define AFX_CEDDLGS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "c_clan.h"
#include "afxwin.h"
#include "afxcmn.h"


#define isEqChar(x,y) (x == LOBYTE(y))
#define isEqShift(x) (((HIBYTE(x)&1) && (GetAsyncKeyState(VK_SHIFT) & 0x8000)) || \
					  (!(HIBYTE(x)&1) && !(GetAsyncKeyState(VK_SHIFT) & 0x8000)))


class CCedDlgs : public CDialog
{
// Construction
public:
	CCedDlgs(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCedDlgs)
	enum { IDD = IDD_CED_OPTIONS };
	int		m_nCheckPointCnt;
	long	m_ClanWindowLinesLimit;
	BOOL	m_CreateBackup;
	BOOL	m_OpenClan;
	BOOL	m_StartEditorMode;
	BOOL	m_WrapClanOutput;
	BOOL	m_WrapLine;
	BOOL	m_RestoreCursor;
	CString	m_Tier;
	BOOL	m_Mixed_Stereo_Wave;
	BOOL	m_Update_Clan;
	BOOL	m_No_CheckMess;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCedDlgs)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCedDlgs)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

/////////////////////////////////////////////////////////////////////////////
// CUpdateCLAN dialog

class CUpdateCLAN : public CDialog
{
// Construction
public:
	CUpdateCLAN(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_UPDATE_OPTION };
	//{{AFX_DATA(CUpdateCLAN)
	CString	m_Message;
	//}}AFX_DATA
	char res;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateCLAN)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpdateCLAN)
	afx_msg void OnUpdate();
	afx_msg void OnSkip();
	afx_msg void OnDontCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CCedFindString dialog

class CCedFindString : public CDialog
{
// Construction
public:
	CCedFindString(CWnd* pParent = NULL);   // standard constructor
	unCH searchFName[FNSize];

// Dialog Data
	enum { IDD = IDD_FIND_STRING_U };
	//{{AFX_DATA(CCedFindString)
	CEdit	m_SearchStrCtrl;
	CString	m_SearchString;
	BOOL	m_SearchBackwards;
	BOOL	m_CaseSensitive;
	BOOL	m_Wrap;
	BOOL	m_UseFileList;
	CString	m_SearchListFName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCedFindString)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCedFindString)
	afx_msg void OnFindInsReturn();
	afx_msg void OnFindInsTab();
	afx_msg void OnFindInsBullet();
	afx_msg void OnFindFromFile();
	afx_msg void OnFindFileList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CGotoLineNumber dialog

class CGotoLineNumber : public CDialog
{
// Construction
public:
	CGotoLineNumber(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGotoLineNumber)
	enum { IDD = IDD_EDIT_GOTO_LINE };
	long	m_LineNumber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGotoLineNumber)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGotoLineNumber)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CReplaceString dialog

class CReplaceString : public CDialog
{
// Construction
public:
	CReplaceString(CWnd* pParent = NULL);   // standard constructor

	char isRepStr;
	unCH replaceFName[FNSize];
// Dialog Data
	enum { IDD = IDD_REPLACE_STRING_U };
	//{{AFX_DATA(CReplaceString)
	CEdit	m_ReplaceWithCtrl;
	CEdit	m_ReplaceStrCtrl;
	CString	m_ReplaceString;
	CString	m_ReplaceWith;
	BOOL	m_CaseSensitive;
	CString	m_ReplaceListFName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReplaceString)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReplaceString)
	afx_msg void OnReplaceSkip();
	afx_msg void OnReplaceThis();
	afx_msg void OnReplaceCaseSen();
	afx_msg void OnReplaceInsReturn();
	afx_msg void OnReplaceInsTab();
	afx_msg void OnReplaceUseFile();
	afx_msg void OnReplaceFromFile();
	afx_msg void OnSetfocusEditReplaceString();
	afx_msg void OnSetfocusEditReplaceWith();
	afx_msg void OnReplaceInsBullet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CHelpWindow dialog

class CHelpWindow : public CDialog
{
// Construction
public:
	CHelpWindow(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHelpWindow)
	enum { IDD = IDD_HELP_WINDOW };
	CRichEditCtrl	m_HelpOutputControl;
	CEdit	m_HelpInputControl;
	CString	m_HelpInput;
	CString	m_HelpOutput;
	BOOL m_doHelp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelpWindow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHelpWindow)
	afx_msg void OnChangeHelpInput();
	afx_msg void OnSelchangeHelpShow(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDefineMacro dialog

class CDefineMacro : public CDialog
{
// Construction
public:
	CDefineMacro(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_EDIT_MACRO_U };
	//{{AFX_DATA(CDefineMacro)
	int		m_Number;
	CString	m_String;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefineMacro)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDefineMacro)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CSoundSync dialog

class CSoundSync : public CDialog
{
// Construction
public:
	CSoundSync(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSoundSync)
	enum { IDD = IDD_EDIT_SNDSYNC };
	BOOL	m_ShowWave;
	BOOL	m_Nothing;
	BOOL	m_ShowAndPlay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundSync)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSoundSync)
	afx_msg void OnDisplayAndPlay();
	afx_msg void OnDisplayWave();
	afx_msg void OnDonothing();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CConstStringNumber dialog

class CConstStringNumber : public CDialog
{
// Construction
public:
	CConstStringNumber(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_CONST_ST_NUMBER_U };
	//{{AFX_DATA(CConstStringNumber)
	CListBox	m_ConstStringListControl;
	int		m_StringNumber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConstStringNumber)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConstStringNumber)
	afx_msg void OnDblclkConstList();
	afx_msg void OnSelchangeConstList();
	afx_msg void OnChangeStringnumber();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CGetAscii dialog

class CGetAscii : public CDialog
{
// Construction
public:
	CGetAscii(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGetAscii)
	enum { IDD = IDD_ENTER_ASCII };
	CRichEditCtrl	m_AsciiOuputControl;
	CEdit	m_AsciiInputControl;
	CString	m_AsciiInput;
	CString	m_AsciiOutput;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetAscii)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGetAscii)
	afx_msg void OnChangeEnterAsciiInput();
	afx_msg void OnSelchangeEnterAsciiShow(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CNextTier dialog

class CNextTier : public CDialog
{
// Construction
public:
	CNextTier(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNextTier)
	enum { IDD = IDD_NEXT_TIER };
	CString	m_NextTier;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNextTier)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNextTier)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CSelectTiers dialog

class CSelectTiers : public CDialog
{
// Construction
public:
	CSelectTiers(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectTiers)
	enum { IDD = IDD_TIER_SELECT };
	BOOL	m_ExcludeTiers;
	BOOL	m_IncludeTiers;
	BOOL	m_ResetTiers;
	CString	m_Tiers;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectTiers)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectTiers)
	afx_msg void OnTierExclude();
	afx_msg void OnTierInclude();
	afx_msg void OnTierReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CStdIn dialog

class CStdIn : public CDialog
{
// Construction
public:
	CStdIn(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_STDIN_U };
	//{{AFX_DATA(CStdIn)
	CString	m_StdInput;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStdIn)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnCancel();
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	char text_isFKeyPressed;

	// Generated message map functions
	//{{AFX_MSG(CStdIn)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CDoString dialog

class CDoString : public CDialog
{
// Construction
public:
	CDoString(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_EDIT_DO_STRING_U };
	//{{AFX_DATA(CDoString)
	CString	m_StringVal;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDoString)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDoString)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CReplaceCont dialog

class CReplaceCont : public CDialog
{
// Construction
public:
	CReplaceCont(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_REPLACE_CONT_U };
	//{{AFX_DATA(CReplaceCont)
	CString	m_ReplaceString;
	CString	m_ReplaceWith;
	//}}AFX_DATA
	char result;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReplaceCont)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReplaceCont)
	afx_msg void OnReplaceContSkipThis();
	afx_msg void OnReplaceContThis();
	afx_msg void OnReplaceSkip();
	afx_msg void OnReplaceAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CSetTab dialog

class CSetTab : public CDialog
{
// Construction
public:
	CSetTab(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetTab)
	enum { IDD = IDD_VIEW_SET_TAB };
	short	m_SetTab;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetTab)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetTab)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CURLAddress dialog

class CURLAddress : public CDialog
{
// Construction
public:
	CURLAddress(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CURLAddress)
	enum { IDD = IDD_EDIT_URL_ADDRESS };
	CString	m_Address;
	CString	m_ProxyAddress;
	CString	m_ProxyPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CURLAddress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CURLAddress)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CThumbnails dialog

class CThumbnails : public CDialog
{
// Construction
public:
	CThumbnails(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CThumbnails)
	enum { IDD = IDD_EDIT_THUMBNAIL_SIZE };
	CString	m_Size;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThumbnails)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CThumbnails)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CGetFilename dialog

class CGetFilename : public CDialog
{
// Construction
public:
	CGetFilename(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGetFilename)
	enum { IDD = IDD_GET_FILENAME };
	CString	m_FileName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetFilename)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGetFilename)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CEnterComments dialog

class CEnterComments : public CDialog
{
// Construction
public:
	CEnterComments(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_ENTER_COMMENTS_U };
	//{{AFX_DATA(CEnterComments)
	CString	m_Comments;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEnterComments)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEnterComments)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CLineNumSize dialog

class CLineNumSize : public CDialog
{
// Construction
public:
	CLineNumSize(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLineNumSize)
	enum { IDD = IDD_EDIT_LINENUM_SIZE };
	long	m_LineNumSize;
	BOOL	m_NumberEveryLine;
	BOOL	m_NumberMainTiersOnly;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLineNumSize)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLineNumSize)
	afx_msg void OnEditLinenumEveryline();
	afx_msg void OnEditLinenumMaintier();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CSelectSoundAnalyzer dialog

class CSelectSoundAnalyzer : public CDialog
{
// Construction
public:
	CSelectSoundAnalyzer(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectSoundAnalyzer)
	enum { IDD = IDD_SOUND_ANA_APP };
	BOOL	m_PitchWorks;
	BOOL	m_Praat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectSoundAnalyzer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectSoundAnalyzer)
	afx_msg void OnPitchworks();
	afx_msg void OnPraat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CPlaybackControler dialog

class CPlaybackControler : public CDialog
{
// Construction
public:
	CPlaybackControler(CWnd* pParent = NULL);   // standard constructor
	void UpdateDialogWindow();

// Dialog Data
	BOOL mySetSlider;
	//{{AFX_DATA(CPlaybackControler)
	enum { IDD = IDD_PLAYBACK_CONTROL };
	CEdit	m_TotalLenControl;
	CEdit	m_CursorPosControl;
	CSliderCtrl	m_SliderControl;
	CString	m_loop;
	CString	m_shift;
	CString	m_speed;
	CString	m_seglen;
	CString	m_CursorPos;
	CString	m_PauseLen;
	CString	m_TotalLen;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlaybackControler)
	public:
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPlaybackControler)
	afx_msg void OnChangePlaybackShiftRate();
	afx_msg void OnChangePlaybackSpeed();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSegmentLength();
	afx_msg void OnDestroy();
	afx_msg void OnChangePlaybackLoopNumber();
	afx_msg void OnChangePlaybackCursorPos();
	afx_msg void OnChangePlaybackPauseLen();
	afx_msg void OnPaint();
	afx_msg void OnCustomdrawPlaybackSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPlaybackOpenmedia();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CPlaybackControler *PBCDlg;
/////////////////////////////////////////////////////////////////////////////
// cTextBullet dialog

class cTextBullet : public CDialog
{
// Construction
public:
	cTextBullet(CString pathName, CWnd* pParent = NULL);   // standard constructor
	char fname[_MAX_PATH+_MAX_FNAME];

// Dialog Data
	//{{AFX_DATA(cTextBullet)
	enum { IDD = IDD_TEXT };
	CEdit	m_TextControl;
	CString	m_text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cTextBullet)
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cTextBullet)
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CF5Options dialog

class CF5Options : public CDialog
{
// Construction
public:
	CF5Options(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CF5Options)
	enum { IDD = IDD_EDIT_F5_OPTION };
	BOOL	m_EveryTier;
	BOOL	m_EveryLine;
	long	m_F5_Offset;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CF5Options)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CF5Options)
	afx_msg void OnBel();
	afx_msg void OnBet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CGetWebPasswd dialog

class CGetWebPasswd : public CDialog
{
// Construction
public:
	CGetWebPasswd(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGetWebPasswd)
	enum { IDD = IDD_WEB_GETPASSWD };
	CString	m_Passwd;
	CString	m_Username;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetWebPasswd)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGetWebPasswd)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CKeywordColor dialog

class CKeywordColor : public CDialog
{
// Construction
public:
	CKeywordColor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeywordColor)
	enum { IDD = IDD_KEYWORD_COLOR_U };
	CStatic	m_ColorSwatchControl;
	CListBox	m_KeyWordColorListControl;
	CString	m_EditWord;
	BOOL	m_ApplyColor;
	BOOL	m_Case;
	BOOL	m_ColorEntire;
	BOOL	m_MatchEntire;
	BOOL	m_TreatAs;
	//}}AFX_DATA

	BOOL ColorChanged;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeywordColor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL


// Implementation
protected:

	COLORTEXTLIST *doKeywordColor(int num, RGBColor *theColor, char isSetColor);
	void FreeOneColorKeyword(int num);

	// Generated message map functions
	//{{AFX_MSG(CKeywordColor)
	afx_msg void OnSelchangeConstList();
	afx_msg void OnDblclkConstList();
	afx_msg void OnDelAll();
	afx_msg void OnDelOne();
	afx_msg void OnEditColor();
	virtual void OnOK();
	afx_msg void OnCase();
	afx_msg void OnColorEntire();
	afx_msg void OnMatchEntire();
	afx_msg void OnTreatAs();
	afx_msg void OnApplyColor();
	afx_msg void OnColorSwatch();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_CEDDLGS_H__F3C36341_3B3C_11D1_8D74_00A02481E866__INCLUDED_)
