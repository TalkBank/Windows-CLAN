// MpegWindow.cpp : implementation file
//

#include "ced.h"
#include "MMedia.h"
#include "Clan2Doc.h"
#include "MpegWindow.h"
#include "MVHelp.h"

#ifdef __MWERKS__
#include <unistd.h>
#else
#include "msutil.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_PLAYTIME_EVENT   0x21122112

extern struct DefWin MovieWinSize;
extern char  rptMark;
extern int	 rptPTime;

CMpegWindow *MPegDlg = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMpegWindow dialog

#define __AFXCONV_H__
#include "afxpriv.h"


CMpegWindow::CMpegWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CMpegWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMpegWindow)
	m_rptstr = _T("");
	m_from = 0;
	m_pos = 0;
	m_to = 0;
	//}}AFX_DATA_INIT
	playing=opened=false;timer=0;
	MPegDlg=this;
	textDoc=NULL;
	isPlayRange = false;
	m_isUserResize = 0L;
	tDelay = 0;
	keyDNcount = 0;
	m_inFocus = IDC_MPG_EDIT_FROM;
	m_PlayToEnd = false;
	if (textDoc != NULL)
		textDoc->isTranscribing = false;
}

void CMpegWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMpegWindow)
	DDX_Control(pDX, IDC_MPG_EDIT_TO, m_ToCtrl);
	DDX_Control(pDX, IDC_MPG_EDIT_POS, m_PosCtrl);
	DDX_Control(pDX, IDC_MPG_EDIT_FROM, m_FromCtrl);
	DDX_Control(pDX, IDC_MPG_EDIT_DELAY, m_DelayCtrl);
	DDX_Control(pDX, IDC_SPIN1, m_spin);
	DDX_Control(pDX, IDC_MEDIAPLAYER1, m_player);
	DDX_Text(pDX, IDC_MPG_EDIT_DELAY, m_rptstr);
	DDX_Text(pDX, IDC_MPG_EDIT_FROM, m_from);
	DDX_Text(pDX, IDC_MPG_EDIT_POS, m_pos);
	DDX_Text(pDX, IDC_MPG_EDIT_TO, m_to);
	//}}AFX_DATA_MAP
}

void CMpegWindow::OnCancel() {
	CRect lpRect;

	UpdateData(true);
	GetWindowRect(&lpRect);
	MovieWinSize.top = lpRect.top;
	MovieWinSize.left = lpRect.left;
	MovieWinSize.width = lpRect.right;
	MovieWinSize.height = lpRect.bottom;
	if (*m_rptstr == 'e' || *m_rptstr == 'E') {
		rptMark = 'e';
		rptPTime = atol(m_rptstr.Mid(1));
	} else if (*m_rptstr == 'b' || *m_rptstr == 'B') {
		rptMark = 'b';
		rptPTime = atol(m_rptstr.Mid(1));
	} else if (*m_rptstr == '-') {
		rptMark = '-';
		rptPTime = atol(m_rptstr.Mid(1));
	} else if (*m_rptstr == '+') {
		rptMark = '+';
		rptPTime = atol(m_rptstr.Mid(1));
	} else {
		rptPTime = atol(m_rptstr);
		if (rptPTime > 0)
			rptMark = '+';
		else
			rptMark = 0;
	}
	DestroyWindow();
	WriteCedPreference();
}

BEGIN_MESSAGE_MAP(CMpegWindow, CDialog)
	//{{AFX_MSG_MAP(CMpegWindow)
	ON_BN_CLICKED(IDC_BEG_BUTTON, OnBegButton)
	ON_BN_CLICKED(IDC_END_BUTTON, OnEndButton)
	ON_BN_CLICKED(IDC_SAVE_BUTTON, OnSaveButton)
	ON_BN_CLICKED(IDC_PLAY_BUTTON, OnPlayButton)
	ON_BN_CLICKED(IDC_MPG_HELP, OnMpgHelp)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_EN_SETFOCUS(IDC_MPG_EDIT_DELAY, OnSetfocusMpgEditDelay)
	ON_EN_SETFOCUS(IDC_MPG_EDIT_FROM, OnSetfocusMpgEditFrom)
	ON_EN_SETFOCUS(IDC_MPG_EDIT_POS, OnSetfocusMpgEditPos)
	ON_EN_SETFOCUS(IDC_MPG_EDIT_TO, OnSetfocusMpgEditTo)
	ON_EN_CHANGE(IDC_MPG_EDIT_POS, OnChangeCursorPos)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CMpegWindow, CDialog)
    //{{AFX_EVENTSINK_MAP(CMpegWindow)
	ON_EVENT(CMpegWindow, IDC_MEDIAPLAYER1, 3012 /* PlayStateChange */, OnPlayStateChangeMediaplayer1, VTS_I4 VTS_I4)
	ON_EVENT(CMpegWindow, IDC_MEDIAPLAYER1, 2 /* PositionChange */, OnPositionChangeMediaplayer1, VTS_R8 VTS_R8)
	ON_EVENT(CMpegWindow, IDC_MEDIAPLAYER1, 3011 /* OpenStateChange */, OnOpenStateChangeMediaplayer1, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMpegWindow message handlers

BOOL CMpegWindow::OnInitDialog() {
	ShowWindow(SW_HIDE);
	m_pos = 0L;
	m_from = 0L;
	m_to = 0L;
	textDC = NULL;
	textDoc = NULL;
    // Get MediaPlayer object and set initial properties.

	CDialog::OnInitDialog();
	myhelpButton.AutoLoad(IDC_MPG_HELP, this);
	if (MovieWinSize.top || MovieWinSize.width || MovieWinSize.height || MovieWinSize.left) {
		CRect lpRect;
		long width, hight;

		GetWindowRect(&lpRect);
		width = lpRect.right - lpRect.left;
		hight = lpRect.bottom - lpRect.top;
		lpRect.top = MovieWinSize.top<0?-2*MovieWinSize.top:MovieWinSize.top;
		lpRect.bottom = lpRect.top + hight;
		lpRect.left = MovieWinSize.left;
		lpRect.right = lpRect.left + width;
		AdjustWindowSize(&lpRect);
		MoveWindow(&lpRect, FALSE);
	}
	if (rptMark == 'b')
		uS.sprintf(templine1, cl_T("b%ld"), rptPTime);
	else if (rptMark == 'e')
		uS.sprintf(templine1, cl_T("e%ld"), rptPTime);
	else if (rptMark == '-')
		uS.sprintf(templine1, cl_T("-%ld"), rptPTime);
	else if (rptMark == '+')
		uS.sprintf(templine1, cl_T("+%ld"), rptPTime);
	else
		uS.sprintf(templine1, cl_T("%ld"), rptPTime);
	m_rptstr = templine1;
	EnableToolTips();
	int i=0;
	do{
		i++;
		timer=SetTimer(i,10,NULL);
	} while (timer==0);
	return 1;
}



void CMpegWindow::OnPositionChangeMediaplayer1(double oldPosition, double newPosition) 
{
	if (!playing){
		long cp;
		cp = (long)(newPosition * (double)1000.000 +.5);
		m_pos=cp;
		UpdateData(false);
	}
}


void CMpegWindow ::changeMovieWinSize(long DisplaySize, BOOL isSetOrgSize) {
	RECT delta,client,parent;
	long winWidth, winHight, editx, edity, tButx,  tButy, offsetxl, offsetxr, offsety;

	//(m_player).SetDisplaySize(0);
	m_player.SetDisplaySize(DisplaySize);
	m_player.SetShowControls(true);
	m_player.SetAutoSize(true);
//	(m_player).SetAllowChangeDisplaySize(false);
	m_player.SetAutoStart(false);
	m_player.SetInvokeURLs(true);
//	(m_player).SetEnableTracker(true);
//	(m_player).SetShowTracker(true);
//	(m_player).SetEnablePositionControls(true);
//	(m_player).SetShowPositionControls(true);
//	(m_player).SetEnabled(true);

	if (isSetOrgSize) {
		m_player.GetWindowRect(&client);
		winHight = m_player.GetImageSourceHeight();
		winWidth = m_player.GetImageSourceWidth();
		if (!winHight && !winWidth)
			winWidth=client.right-client.left;

		orgMvBounds.top    = client.top;
		orgMvBounds.bottom = client.bottom;
		orgMvBounds.left   = client.left;
		orgMvBounds.right  = client.right;

		MvBounds.top = orgMvBounds.top;
		MvBounds.bottom = orgMvBounds.bottom;
		MvBounds.left = orgMvBounds.left;
		MvBounds.right = orgMvBounds.right;
	} else {
		m_player.GetWindowRect(&client);
		winHight = client.bottom-client.top;
		winWidth = client.right-client.left;

		MvBounds.top    = client.top;
		MvBounds.bottom = client.bottom;
		MvBounds.left   = client.left;
		MvBounds.right  = client.right;
	}

	GetWindowRect(&parent);
	parent.right=parent.left+winWidth;
	parent.bottom=parent.top+winHight;
	client=parent;

	CWnd *pw=GetDlgItem(IDC_MPG_EDIT_POS);
	pw->GetWindowRect(&delta);
	editx = delta.right-delta.left;
	edity = delta.bottom-delta.top;
	parent.bottom+=7*edity;
	if ((parent.right-parent.left)<3*editx)
		parent.right=parent.left+3*editx;
	CalcWindowRect(&parent);
	offsetxl=client.left-parent.left;
	offsetxr=parent.right-client.right;
	MoveWindow(&parent,true);
	(m_player).MoveWindow(0,0,winWidth,winHight);

	GetClientRect(&parent);
	if (parent.top < 0) {
		parent.bottom = parent.bottom - parent.top;
		parent.top = 0;
	}
	if (parent.left < 0) {
		parent.right = parent.right - parent.left;
		parent.left = 0;
	}
	client=parent;

	pw=GetDlgItem(IDC_PLAY_BUTTON);
	client.top=client.bottom-edity;
	client.left=offsetxl;
	client.right=offsetxl+editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_MPG_EDIT_DELAY);
	client.left += editx + 2;
	client.right = client.left + editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_MPG_EDIT_MSEC);
	pw->GetWindowRect(&delta);
	client.left += editx + 2;
	client.right = client.left + (delta.right-delta.left);
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_MPG_EDIT_FROM);
	client.bottom -= edity;
	client.left=offsetxl;
	client.top=client.bottom-edity;
	client.right=offsetxl+editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_MPG_EDIT_TO);
	client.left += editx + 2;
	client.right = client.left + editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_MPG_EDIT_POS);
	client.left=.5*editx+offsetxl;
	client.right=1.5*editx+offsetxl;
	client.top-=2.5*edity;
	offsety=client.top;
	client.bottom-=2.5*edity;
	pw->MoveWindow(&client,true);

	delta=client;
	pw=GetDlgItem(IDC_BEG_BUTTON);
	pw->GetWindowRect(&client);
	pw->GetWindowRect(&client);
	tButx=client.right-client.left;
	tButy=client.bottom-client.top;
	client.top=delta.bottom;
	client.bottom=client.top+tButy;
	client.left=delta.left;
	client.right=client.left+tButx;
	client.top+=tButy/5;
	client.bottom+=tButy/5;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_END_BUTTON);
	client.right=delta.right;
	client.left=client.right-tButx;
	pw->MoveWindow(&client);

	pw=GetDlgItem(IDC_SAVE_BUTTON);
	client.top=offsety;
	client.bottom=offsety+edity;
	client.right=parent.right-offsetxr;
	client.left=client.right-editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_MPG_HELP);
	pw->GetWindowRect(&client);
	editx = client.right - client.left;
	edity = client.bottom - client.top;
	client.bottom=parent.bottom-offsetxr;
	client.top=client.bottom-edity;
	client.right=parent.right-offsetxr;
	client.left=client.right-editx;
	pw->MoveWindow(&client,true);

	UpdateData(false);
	ShowWindow(SW_SHOWNORMAL );
}

void CMpegWindow::OnOpenStateChangeMediaplayer1(long OldState, long NewState) 
{
	if (NewState == 5 /*nsOpening*/ || NewState == 6 /*nsOpen*/) {
		changeMovieWinSize(4, true);
		if (MVWinWidth != 0)
			ResizeMovie(MVWinWidth, 0, false);
		else if (MVWinZoom != 1)
			ResizeMovie(0, 0, false);
		m_spin.SetRange32(0,(long)(m_player.GetDuration() * 1000.));
		opened=true;
	}
}

void CMpegWindow::ResizeMovie(short w, short isRedo, BOOL isRefreshMPG) {
	double ratio, th, tw;
	TimeValue tm_from,tm_pos,tm_to;
	short			h;

	tm_from = m_from;
	tm_pos = m_pos;
	tm_to = m_to;

	if (w == 0) {
		if (MVWinWidth != 0) {
			MVWinZoom = 1;
			MVWinWidth = 0;
		} else if (isRedo != 0) {
			MVWinZoom++;
			if (MVWinZoom >= 3)
				MVWinZoom = -1;
		}
		MvBounds.top = orgMvBounds.top;
		MvBounds.left = orgMvBounds.left;
		if (MVWinZoom <= 0) {
			if (MVWinZoom == -1) {
				MvBounds.right = MvBounds.left + ((orgMvBounds.right-orgMvBounds.left) / 4);
				MvBounds.bottom = MvBounds.top + ((orgMvBounds.bottom-orgMvBounds.top) / 4);
			} else {
				MvBounds.right = MvBounds.left + ((orgMvBounds.right-orgMvBounds.left) / 2);
				MvBounds.bottom = MvBounds.top + ((orgMvBounds.bottom-orgMvBounds.top) / 2);
			}
		} else {
			MvBounds.right = MvBounds.left + ((orgMvBounds.right-orgMvBounds.left) * MVWinZoom);
			MvBounds.bottom = MvBounds.top + ((orgMvBounds.bottom-orgMvBounds.top) * MVWinZoom);
		}
		w = MvBounds.right - MvBounds.left;
		h = MvBounds.bottom - MvBounds.top;
	} else {
		th = (double)(orgMvBounds.bottom - orgMvBounds.top);
		tw = (double)(orgMvBounds.right - orgMvBounds.left);
		ratio = th / tw;
		tw = w;
		MvBounds.right = MvBounds.left + w;
		MvBounds.bottom = MvBounds.top + (short)(tw * ratio);
		MVWinWidth = w;
	}
	
	WriteCedPreference();
	
	changeMovieWinSize(4+MVWinZoom, false);

/*	
	
	if (qt)	{
		qt->DestroyWindow() ;
		qt=NULL;
	}
	
	qt=new CQT(pathname,this,isUseURL,false);

	qt->orgMvBounds.top = orgMvBounds.top;
	qt->orgMvBounds.bottom = orgMvBounds.bottom;
	qt->orgMvBounds.left = orgMvBounds.left;
	qt->orgMvBounds.right = orgMvBounds.right;

	qt->MvBounds.top = MvBounds.top;
	qt->MvBounds.bottom = MvBounds.bottom;
	qt->MvBounds.left = MvBounds.left;
	qt->MvBounds.right = MvBounds.right;

	if (qt->Create(IDD_QT,this)){
		ResizeQTDlgWindow(qt);
		if (isRefreshMPG) {
			qt->SetStartTime((TimeValue) m_from);
			qt->SetDuration((TimeValue) (1));
			qt->Play();
		}
		m_from = tm_from;
		m_pos = tm_pos;
		m_to = tm_to;
		UpdateData(false);
	}else{
		DestroyWindow();
	}
*/
}

void CMpegWindow::OnSysCommand(UINT nID, LPARAM lParam)
{

	if (nID==0xF032)
		;
/*
	else if (nID==SC_RESTORE)
		i = 4;
	else if (nID==SC_DEFAULT)
		i = 3;
	else if (nID==SC_MINIMIZE)
		i = 2;
	else if (nID==SC_SIZE)
		i = 1;
*/
	else if (nID==SC_MAXIMIZE) {
	} else {
		if (nID >= 0xF001 && nID <= 0xF008) {
			if (nID == 0xF006 || nID == 0xF003) {
				RECT client;
				GetWindowRect(&client);
				m_isUserResize = client.bottom - client.top;
			} else
				m_isUserResize = -1;
 		}
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CMpegWindow::OnSize(UINT nType, int cx, int cy) 
{	
	CDialog::OnSize(nType, cx, cy);
	
	if (m_isUserResize != 0L) {
		double tOldC, tC;
		if (m_isUserResize > 0L) {
			tOldC = m_isUserResize;
			tC = cy;
			tC = (tC * 100.0000) / tOldC;
			tOldC = cx;
			tC = (tOldC * tC) / 100.0000;
			cx = tC;
		}
		m_isUserResize = 0L;
	}
	
}

void CMpegWindow::OnPlayStateChangeMediaplayer1(long OldState, long NewState) 
{
	int i=0;
	if (NewState==2 && OldState==0  ){
		playing=true;
		 UpdateData(false);
		 if (PlayingContMovie && PlayingContMovie!='\003') {
			 if (m_pos)
				 textDoc->ChangeCursorInfo=m_pos;
			 else
				 textDoc->ChangeCursorInfo=1;
			 textDoc->myUpdateAllViews(FALSE);
		 }
		 isPlayRange = true;
	}
	if (NewState==0 && OldState==2) {
		UpdateData(false);
		playing=false;
		PlayingContMovie=false;
		textDoc->RestorePlayMovieInfo=TRUE;
		textDoc->myUpdateAllViews(FALSE);
	}
}

void CMpegWindow::OnBegButton() 
{
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_MPG_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_MPG_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_MPG_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_MPG_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (m_inFocus == IDC_MPG_EDIT_FROM || m_inFocus == IDC_MPG_EDIT_TO) {
		UpdateData(true);
		m_from=m_pos;
		UpdateData(false);
	} else if (m_inFocus == IDC_MPG_EDIT_POS) {
		UpdateData(true);
		m_pos=m_from;
		UpdateData(false);
	}

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CMpegWindow::OnEndButton() 
{
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_MPG_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_MPG_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_MPG_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_MPG_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (m_inFocus == IDC_MPG_EDIT_FROM || m_inFocus == IDC_MPG_EDIT_TO) {
		UpdateData(true);
		m_to=m_pos;
		UpdateData(false);
	} else if (m_inFocus == IDC_MPG_EDIT_POS) {
		UpdateData(true);
		m_pos=m_to;
		UpdateData(false);
	}
	if (m_player.GetPlayState()==2) {
		m_player.Stop();
		PlayingContMovie = false;
	}

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CMpegWindow::OnSaveButton() 
{	
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_MPG_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_MPG_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_MPG_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_MPG_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (PlayingContMovie=='\003') {
		if (textDoc != NULL && textDoc->FileInfo != NULL) {
			if (m_pos)
				textDoc->ChangeCursorInfo=m_pos;
			else
				textDoc->ChangeCursorInfo=1;
			textDoc->myUpdateAllViews(FALSE);
		}
	} else if (textDoc != NULL && textDoc->FileInfo != NULL) {
		UpdateData(true);
		textDoc->FileInfo->MvTr.MBeg = m_from;
		textDoc->FileInfo->MvTr.MEnd = m_to;
		textDoc->SaveMovieInfo = TRUE;
		textDoc->myUpdateAllViews(FALSE);
	}

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CMpegWindow::ExecuteCommNumber(int num)
{
	if (textDoc != NULL && textDoc->FileInfo != NULL) {
		textDoc->FileInfo->MvTr.MBeg = m_from;
		textDoc->FileInfo->MvTr.MEnd = m_to;
		textDoc->ExecuteCommNUM = num;
		textDoc->myUpdateAllViews(FALSE);
	}
}

BOOL CMpegWindow::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_CHAR && pMsg->wParam == 17/*^W*/) {
		PlayingContMovie = false;
		if (m_player.GetPlayState()==2) 
			m_player.Stop();
		isPlayRange = false;
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
		return TRUE;
	} else if (pMsg->message == WM_CHAR && pMsg->wParam == 9/*^I*/) {
		if (PlayingContMovie=='\003') {
			if (textDoc != NULL && textDoc->FileInfo != NULL) {
				if (m_pos)
					textDoc->ChangeCursorInfo=m_pos;
				else
					textDoc->ChangeCursorInfo=1;
				textDoc->myUpdateAllViews(FALSE);
			}
		} else {
			SendMessage(WM_COMMAND, IDC_SAVE_BUTTON, 0);
		}
		return TRUE;
	} else if (pMsg->message == WM_KEYDOWN) {
		if (textDoc != NULL && textDoc->isTranscribing) {
			if (pMsg->wParam == VK_SPACE) {
				if (textDoc != NULL && textDoc->FileInfo != NULL) {
					if (m_pos)
						textDoc->ChangeCursorInfo=m_pos;
					else
						textDoc->ChangeCursorInfo=1;
					textDoc->myUpdateAllViews(FALSE);
				} else {
					PlayingContMovie = false;
					if (m_player.GetPlayState()==2) 
						m_player.Stop();
					isPlayRange = false;
					do_warning("Internal error with movie to text link. Please quit CLAN and try again.",0);
				}
				return TRUE;
			}
/*
			PlayingContMovie = false;
			if (m_player.GetPlayState()==2) 
				m_player.Stop();
			isPlayRange = false;
			return TRUE;
*/
		} else if (pMsg->wParam == VK_RETURN) {
			m_PlayToEnd = false;
			SendMessage(WM_COMMAND, IDC_PLAY_BUTTON, 0);
			return TRUE;
		} else if (pMsg->wParam == VK_SPACE) {
			m_PlayToEnd = true;
			SendMessage(WM_COMMAND, IDC_PLAY_BUTTON, 0);
			return TRUE;
		} else if (pMsg->wParam == VK_TAB) {
		} else if (pMsg->wParam == VK_F5) {
			ExecuteCommNumber(89);
			return TRUE;
		} else if (pMsg->wParam == VK_F6) {
			ExecuteCommNumber(91);
			return TRUE;
		} else if (pMsg->wParam == VK_F7) {
			ExecuteCommNumber(96);
			return TRUE;
		} else if (pMsg->wParam == VK_LEFT && (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
			UpdateData(true);
			m_to -= MOVEINCREMENT;
			if (m_to < 0)
				m_to = 0;
			m_pos = m_to;
			UpdateData(false);
			m_player.SetCurrentPosition((double)m_pos/1000.);
			ExecuteCommNumber(93);
			return TRUE;
		} else if (pMsg->wParam == VK_LEFT && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			UpdateData(true);
			m_from -= MOVEINCREMENT;
			if (m_from < 0)
				m_from = 0;
			m_pos = m_from;
			UpdateData(false);
			m_player.SetCurrentPosition((double)m_pos/1000.);
			ExecuteCommNumber(95);
			return TRUE;
		} else if (pMsg->wParam == VK_RIGHT && (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
			UpdateData(true);
			m_to += MOVEINCREMENT;
			if (m_to > movieDuration)
				m_to = movieDuration;
			m_pos = m_to;
			UpdateData(false);
			m_player.SetCurrentPosition((double)m_pos/1000.);
			ExecuteCommNumber(92);
			return TRUE;
		} else if (pMsg->wParam == VK_RIGHT && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			UpdateData(true);
			m_from += MOVEINCREMENT;
			if (m_from > movieDuration)
				m_from = movieDuration;
			m_pos = m_from;
			UpdateData(false);
			m_player.SetCurrentPosition((double)m_pos/1000.);
			ExecuteCommNumber(94);
			return TRUE;
		} else if (pMsg->wParam == VK_LEFT) {
		//		int count;
			if (GetFocus( ) != GetDlgItem(IDC_MPG_EDIT_DELAY)) {
				UpdateData(true);
				if (GetTickCount()-tDelay < 400)
					keyDNcount++;
				else
					keyDNcount = 0;
				tDelay = GetTickCount();
		//			count = (int)(pMsg->lParam & 0xFFFF);
				if (m_pos > 0)
					m_pos -= ((keyDNcount < 8) ? 1 : 50);
				if (m_pos < 0)
					m_pos = 0;
				UpdateData(false);
				m_player.SetCurrentPosition((double)m_pos/1000.);
				return TRUE;
			}
		} else if (pMsg->wParam == VK_RIGHT) {
		//		int count;
			if (GetFocus( ) != GetDlgItem(IDC_MPG_EDIT_DELAY)) {
				UpdateData(true);
				if (GetTickCount()-tDelay < 400)
					keyDNcount++;
				else
					keyDNcount = 0;
				tDelay = GetTickCount();
		//			count = (int)(pMsg->lParam & 0xFFFF);
				m_pos += ((keyDNcount < 8) ? 1 : 50);
				if (m_pos > movieDuration)
					m_pos = movieDuration;
				UpdateData(false);
				m_player.SetCurrentPosition((double)m_pos/1000.);
				return TRUE;
			}
		} else if (pMsg->wParam == VK_UP) {
			if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_FROM)) {
				UpdateData(true);
				m_pos = m_from;
				UpdateData(false);
				m_player.SetCurrentPosition((double)m_pos/1000.);
				return TRUE;
			} else if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_TO)) {
				UpdateData(true);
				m_pos = m_to;
				UpdateData(false);
				m_player.SetCurrentPosition((double)m_pos/1000.);
				return TRUE;
			} else {
				return TRUE;
			}
		} else if (pMsg->wParam == VK_DOWN) {
			if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_FROM)) {
				UpdateData(true);
				m_from = m_pos;
				UpdateData(false);
				return TRUE;
			} else if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_TO)) {
				UpdateData(true);
				m_to = m_pos;
				UpdateData(false);
				return TRUE;
			} else {
				return TRUE;
			}
		} else
			return CDialog::PreTranslateMessage(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CMpegWindow::DestroyWindow() 
{
	myFInfo *saveGlobal_df;

	PlayingContMovie = FALSE;
	saveGlobal_df = global_df;
	global_df = textDoc->FileInfo;
	if (global_df->cpMvTr != NULL) {
//		GlobalDC = textDC;
		GlobalDoc = textDoc;
		CopyAndFreeMovieData(TRUE);
	}
	global_df = saveGlobal_df;
	if (KillTimer(timer))
		timer=0;
	m_player.DestroyWindow();
	return CDialog::DestroyWindow();
}

void CMpegWindow::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	MPegDlg = NULL;
	CDialog::PostNcDestroy();
}

void CMpegWindow::OnPlayButton() 
{
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_MPG_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_MPG_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_MPG_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_MPG_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (textDoc != NULL)
		textDoc->isTranscribing = (BOOL)(PlayingContMovie == '\003');
	if (m_player.GetPlayState()==2) {
		m_player.Stop();
		PlayingContMovie = false;
		isPlayRange = false;
		if (m_to == 0) {
			m_to = movieDuration;
			UpdateData(false);
		}
		return;
	}
	UpdateData(true);
	isPlayRange = true;
	if (m_PlayToEnd) {
		m_PlayToEnd = false;
		m_player.SetSelectionStart((double)m_pos/1000.);
		m_player.SetCurrentPosition((double)m_pos/1000.);
		m_player.SetSelectionEnd((double)movieDuration/1000.);
		m_player.Play();
	} else if (m_to == 0) {
		m_to = movieDuration;
		UpdateData(false);
	} else if (m_to <= m_from)
		do_warning("Selection end value must be greater than start value", 0);
	else {
		long m_rptPTime;
		if (PlayingContMovie) {
			m_player.SetSelectionStart((double)m_from/1000.);
			m_player.SetCurrentPosition((double)m_from/1000.);
			m_player.SetSelectionEnd((double)m_to/1000.);
		} else if (*m_rptstr == 'e' || *m_rptstr == 'E') {
			m_rptPTime = atol(m_rptstr.Mid(1));
			m_player.SetSelectionStart((double)(m_to-m_rptPTime)/1000.);
			m_player.SetCurrentPosition((double)(m_to-m_rptPTime)/1000.);
			m_player.SetSelectionEnd((double)m_to/1000.);
		} else if (*m_rptstr == 'b' || *m_rptstr == 'B') {
			m_rptPTime = atol(m_rptstr.Mid(1));
			m_player.SetSelectionStart((double)m_from/1000.);
			m_player.SetCurrentPosition((double)m_from/1000.);
			m_player.SetSelectionEnd((double)(m_from+m_rptPTime)/1000.);
		} else if (*m_rptstr == '-') {
			m_rptPTime = atol(m_rptstr.Mid(1));
			if (m_from-m_rptPTime < 0)
				m_rptPTime = 0;
			else
				m_rptPTime = m_from - m_rptPTime;
			m_player.SetSelectionStart((double)m_rptPTime/1000.);
			m_player.SetCurrentPosition((double)m_rptPTime/1000.);
			m_player.SetSelectionEnd((double)m_to/1000.);
		} else if (*m_rptstr == '+') {
			m_rptPTime = atol(m_rptstr.Mid(1));
			if (m_to+m_rptPTime > movieDuration)
				m_rptPTime = movieDuration;
			else
				m_rptPTime = m_to + m_rptPTime;
			m_player.SetSelectionStart((double)m_from/1000.);
			m_player.SetCurrentPosition((double)m_from/1000.);
			m_player.SetSelectionEnd((double)m_rptPTime/1000.);
		} else {
			m_rptPTime = atol(m_rptstr);
			if (m_rptPTime > 0) {
				if (m_to+m_rptPTime > movieDuration)
					m_rptPTime = movieDuration;
				else
					m_rptPTime = m_to + m_rptPTime;
				m_player.SetSelectionStart((double)m_from/1000.);
				m_player.SetCurrentPosition((double)m_from/1000.);
				m_player.SetSelectionEnd((double)m_rptPTime/1000.);
			} else {
				m_player.SetSelectionStart((double)m_from/1000.);
				m_player.SetCurrentPosition((double)m_from/1000.);
				m_player.SetSelectionEnd((double)m_to/1000.);
			}
		}
		m_player.Play();
	}

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

BOOL CMpegWindow::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	return 1;// CDialog::OnChildNotify(message, wParam, lParam, pLResult);
}

void CMpegWindow::OnChangeCursorPos() 
{
	if (opened){
		if (!playing)
			m_player.SetCurrentPosition((double)m_pos/1000.);
		else {
			m_player.Stop();
			PlayingContMovie = false;
		}
	}
}

void CMpegWindow::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	CDialog::OnClose();
}

BOOL CMpegWindow::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	if (wParam == IDC_MPG_EDIT_POS) {
		TCHAR a=(TCHAR) wParam;
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CMpegWindow::OnTimer(UINT nIDEvent) 
{
	double pos;
	if ((pos=m_player.GetCurrentPosition())>-1){
		pos*=1000.;
		if (playing)
			m_pos=pos;
		else {
			UpdateData();
		}
		UpdateData(false);
		if (textDoc != NULL && textDoc->FileInfo != NULL) {
			if (PlayingContMovie && PlayingContMovie!='\003' && m_pos > textDoc->FileInfo->MvTr.MEnd) {
				if (m_pos)
					textDoc->ChangeCursorInfo=m_pos;
				else
					textDoc->ChangeCursorInfo=1;
				textDoc->myUpdateAllViews(FALSE);
			}
		}
	}

	CDialog::OnTimer(nIDEvent);
}


BOOL CMpegWindow::ShowToolTip( UINT id, NMHDR * pTTTStruct, LRESULT * pResult )
{
	NMTTDISPINFO* p=(NMTTDISPINFO*)pTTTStruct;
	UINT nID =pTTTStruct->idFrom;
	if (p->uFlags & TTF_IDISHWND){
		nID = ::GetDlgCtrlID((HWND)nID);
	}
	switch (nID){
	case IDC_PLAY_BUTTON:
		p->lpszText= _T("I play the selection specified") ;
		break;
	case IDC_SAVE_BUTTON:
		p->lpszText= _T("I save the selection specified in the current text file") ;
		break;
	case IDC_MPG_EDIT_FROM:
		p->lpszText= _T("I display/set the starting point of selection") ;
		break;
	case IDC_MPG_EDIT_TO:
		p->lpszText= _T("I display/set the ending point of selection") ;
		break;
	case IDC_MPG_EDIT_POS:
		p->lpszText= _T("I follow/set the current position in the movie") ;
		break;
	case IDC_BEG_BUTTON:
		p->lpszText= _T("I align the beginning of selection (below me) to the current position in the movie (above me)") ;
		break;
	case IDC_END_BUTTON:
		p->lpszText= _T("I align the end of selection (below me) to the current position in the movie (above me)") ;
		break;
	case IDC_MPG_EDIT_DELAY:
		p->lpszText= _T("I control the size of clip played, negative number mean from the beginning and positive - from the end") ;
		break;
	}
	return 1;
}

void CMpegWindow::Play(){
	OnPlayButton();
}

LRESULT CMpegWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message == WM_MOUSEACTIVATE) {
		if (textDoc != NULL && textDoc->FileInfo != NULL) {
			textDoc->SetFakeHilight = 2;
			textDoc->myUpdateAllViews(FALSE);
		}
	}
	return CDialog::WindowProc(message, wParam, lParam);
}


void CMpegWindow::OnMpgHelp() {
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_MPG_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_MPG_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_MPG_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_MPG_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	PlayingContMovie = false;
	if (m_player.GetPlayState()==2) 
		m_player.Stop();
	isPlayRange = false;
	if (MVHelpDlg == NULL) {
		MVHelpDlg=new CMyHelp(AfxGetApp()->m_pMainWnd);
		MVHelpDlg->Create(IDD_MYHELP);
	} else
		MVHelpDlg->SetActiveWindow();

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CMpegWindow::OnLButtonDown(UINT nFlags, CPoint point) 
{
	PlayingContMovie = false;
	if (m_player.GetPlayState()==2) 
		m_player.Stop();
	isPlayRange = false;
	CDialog::OnLButtonDown(nFlags, point);
}

void CMpegWindow::OnSetfocusMpgEditDelay() 
{
	SetDlgItemText(IDC_BEG_BUTTON, _T("-"));
	SetDlgItemText(IDC_END_BUTTON, _T("-"));
	m_inFocus = IDC_MPG_EDIT_DELAY;
}

void CMpegWindow::OnSetfocusMpgEditFrom() 
{
	SetDlgItemText(IDC_BEG_BUTTON, _T("v"));
	SetDlgItemText(IDC_END_BUTTON, _T("v"));
	m_inFocus = IDC_MPG_EDIT_FROM;		
}

void CMpegWindow::OnSetfocusMpgEditPos() 
{
	SetDlgItemText(IDC_BEG_BUTTON, _T("^"));
	SetDlgItemText(IDC_END_BUTTON, _T("^"));
	m_inFocus = IDC_MPG_EDIT_POS;
}

void CMpegWindow::OnSetfocusMpgEditTo() 
{
	SetDlgItemText(IDC_BEG_BUTTON, _T("v"));
	SetDlgItemText(IDC_END_BUTTON, _T("v"));
	m_inFocus = IDC_MPG_EDIT_TO;	
}
