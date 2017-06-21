// QTDlg.cpp : implementation file
//
#include "stdafx.h"
#include "ced.h"
#include "MMedia.h"
#include "Clan2Doc.h"
#include "QTDlg.h"
#include "MVHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern struct DefWin MovieWinSize;
extern char  rptMark;
extern int	 rptPTime;

CQTDlg *MovDlg=NULL;
/////////////////////////////////////////////////////////////////////////////
// CQTDlg dialog


CQTDlg::CQTDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQTDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQTDlg)
	m_from = 0;
	m_pos = 0;
	m_to = 0;
	m_rptstr = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	qt=NULL;
	textDoc=NULL;
	doErase = true;
	isPlayRange = false;
	tDelay = 0;
	keyDNcount = 0;
	m_inFocus = IDC_EDIT_FROM;
	m_PlayToEnd = false;
	m_isUserResize = 0L;
	if (textDoc != NULL)
		textDoc->isTranscribing = false;
}

void CQTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQTDlg)
	DDX_Control(pDX, IDC_EDIT_TO, m_ToCtrl);
	DDX_Control(pDX, IDC_EDIT_POS, m_PosCtrl);
	DDX_Control(pDX, IDC_EDIT_FROM, m_FromCtrl);
	DDX_Control(pDX, IDC_EDIT_DELAY, m_DelayCtrl);
	DDX_Control(pDX, IDC_SPIN1, m_spin);
	DDX_Text(pDX, IDC_EDIT_FROM, m_from);
	DDX_Text(pDX, IDC_EDIT_POS, m_pos);
	DDX_Text(pDX, IDC_EDIT_TO, m_to);
	DDX_Text(pDX, IDC_EDIT_DELAY, m_rptstr);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CQTDlg, CDialog)
	//{{AFX_MSG_MAP(CQTDlg)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_SET_BEGIN, OnSetBegin)
	ON_BN_CLICKED(IDC_SET_END, OnSetEnd)
	ON_EN_UPDATE(IDC_EDIT_POS, OnUpdateEditPos)
	ON_BN_CLICKED(IDC_QD_HELP, OnQdHelp)
	ON_WM_LBUTTONDOWN()
	ON_EN_SETFOCUS(IDC_EDIT_POS, OnSetfocusEditPos)
	ON_EN_SETFOCUS(IDC_EDIT_TO, OnSetfocusEditTo)
	ON_EN_SETFOCUS(IDC_EDIT_FROM, OnSetfocusEditFrom)
	ON_EN_SETFOCUS(IDC_EDIT_DELAY, OnSetfocusEditDelay)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_EN_UPDATE(IDC_EDIT_FROM, &CQTDlg::OnUpdateEditFrom)
	ON_EN_UPDATE(IDC_EDIT_TO, &CQTDlg::OnUpdateEditTo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQTDlg message handlers

BOOL CQTDlg::OnInitDialog()
{
	UpdateData(false);
	CDialog::OnInitDialog();

	myhelpButton.AutoLoad(IDC_QD_HELP, this);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	qt=new CQT(pathname,this,true);
	if (qt->Create(IDD_QT,this)){
		ResizeQTDlgWindow(qt);
		if (MVWinWidth != 0)
			ResizeMovie(MVWinWidth, 0, false);
		else if (MVWinZoom != 1)
			ResizeMovie(0, 0, false);
	}else{
		DestroyWindow();
	}
	return TRUE; // return TRUE  unless you set the focus to a control
}

void CQTDlg::Play(){

	if (textDoc != NULL)
		textDoc->isTranscribing = (BOOL)(PlayingContMovie == '\003');
	m_pos = m_from;
	UpdateData(false);
	if (PlayingContMovie && PlayingContMovie!='\003') {
		if (m_pos) textDoc->ChangeCursorInfo=m_pos;
		else textDoc->ChangeCursorInfo=1;
		textDoc->myUpdateAllViews(FALSE);
	}
	if (qt) {
		TimeValue m_rptPTime;

		if (textDoc != NULL)
			textDoc->isTranscribing = (BOOL)(PlayingContMovie == '\003');
		real_to = 0;
		isPlayRange = true;
		if (m_PlayToEnd) {
			m_PlayToEnd = false;
			qt->SetStartTime((TimeValue) m_pos);
			qt->SetDuration((TimeValue) (qt->GetQTMovieDuration()-m_pos));
			real_to = qt->GetQTMovieDuration();
		} else if (m_to == 0) {
			m_pos = m_from;
			m_to = qt->GetQTMovieDuration();
			UpdateData(false);
			qt->SetStartTime((TimeValue) 1);
			qt->SetDuration((TimeValue) 1);
		} else if (PlayingContMovie) {
			qt->SetStartTime((TimeValue) m_from);
			qt->SetDuration((TimeValue) (m_to-m_from));
			real_to = m_to;
		} else if (*m_rptstr == 'e' || *m_rptstr == 'E') {
			m_rptPTime = atol(m_rptstr.Mid(1));
			qt->SetStartTime((TimeValue) m_to-m_rptPTime);
			qt->SetDuration((TimeValue) m_rptPTime);
			real_to = m_to;
		} else if (*m_rptstr == 'b' || *m_rptstr == 'B') {
			m_rptPTime = atol(m_rptstr.Mid(1));
			qt->SetStartTime((TimeValue) m_from);
			qt->SetDuration((TimeValue) m_rptPTime);
			real_to = m_from + m_rptPTime;
		} else if (*m_rptstr == '-') {
			m_rptPTime = atol(m_rptstr.Mid(1));
			if (m_from-m_rptPTime < 0)
				m_rptPTime = 0;
			else
				m_rptPTime = m_from - m_rptPTime;
			qt->SetStartTime((TimeValue) m_rptPTime);
			qt->SetDuration((TimeValue) (m_to-m_rptPTime));
			real_to = m_to;
		} else if (*m_rptstr == '+') {
			m_rptPTime = atol(m_rptstr.Mid(1));
			if (m_to+m_rptPTime > qt->GetQTMovieDuration())
				m_rptPTime = qt->GetQTMovieDuration();
			else
				m_rptPTime = m_to + m_rptPTime;
			qt->SetStartTime((TimeValue) m_from);
			qt->SetDuration((TimeValue) (m_rptPTime-m_from));
			real_to = m_rptPTime;
		} else {
			m_rptPTime = atol(m_rptstr);
			if (m_rptPTime > 0) {
				if (m_to+m_rptPTime > qt->GetQTMovieDuration())
					m_rptPTime = qt->GetQTMovieDuration();
				else
					m_rptPTime = m_to + m_rptPTime;
				qt->SetStartTime((TimeValue) m_from);
				qt->SetDuration((TimeValue) (m_rptPTime-m_from));
				real_to = m_rptPTime;
			} else {
				qt->SetStartTime((TimeValue) m_from);
				qt->SetDuration((TimeValue) (m_to-m_from));
				real_to = m_to;
			}
		}
		last_m_pos = 0;
		same_m_posValue  = 0;
		qt->Play();
	}
}

void CQTDlg::ResizeQTDlgWindow(CQT *qt) {
	RECT delta,client,parent;
	UINT editx, edity, tButx,  tButy, offsetxl, offsetxr, offsety;
	CWnd*pw;

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
	m_from=qt->GetStartTime();
	m_pos=m_from;
	m_to=m_from + qt->GetDuration();
	m_spin.SetRange32(0,m_to);

	qt->GetWindowRect(&parent);
	client=parent;
	pw=GetDlgItem(IDC_EDIT_POS);
	pw->GetWindowRect(&delta);
	editx = delta.right-delta.left;
	edity = delta.bottom-delta.top;
	parent.bottom+=5*edity;
	if ((parent.right-parent.left)<3*editx)
		parent.right=parent.left+3*editx;
	CalcWindowRect(&parent);
	offsetxl=client.left-parent.left;
	offsetxr=parent.right-client.right;
	MoveWindow(&parent,true);

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

	pw=GetDlgItem(IDC_BUTTON_PLAY);
	client.top=client.bottom-edity;
	client.left=offsetxl;
	client.right=offsetxl+editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_EDIT_DELAY);
	client.left += editx + 2;
	client.right = client.left + editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_EDIT_MSEC);
	pw->GetWindowRect(&delta);
	client.left += editx + 2;
	client.right = client.left + (delta.right-delta.left);
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_EDIT_FROM);
	client.bottom -= edity;
	client.left=offsetxl;
	client.top=client.bottom-edity;
	client.right=offsetxl+editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_EDIT_TO);
	client.left += editx + 2;
	client.right = client.left + editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_EDIT_POS);
	client.left=.5*editx+offsetxl;
	client.right=1.5*editx+offsetxl;
	client.top-=2.5*edity;
	offsety=client.top;
	client.bottom-=2.5*edity;
	pw->MoveWindow(&client,true);

	delta=client;
	pw=GetDlgItem(IDC_SET_BEGIN);
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

	pw=GetDlgItem(IDC_SET_END);
	client.right=delta.right;
	client.left=client.right-tButx;
	pw->MoveWindow(&client);

	pw=GetDlgItem(IDC_BUTTON_SAVE);
	client.top=offsety;
	client.bottom=offsety+edity;
	client.right=parent.right-offsetxr;
	client.left=client.right-editx;
	pw->MoveWindow(&client,true);

	pw=GetDlgItem(IDC_QD_HELP);
	pw->GetWindowRect(&client);
	editx = client.right - client.left;
	edity = client.bottom - client.top;
	client.bottom=parent.bottom-offsetxr;
	client.top=client.bottom-edity;
	client.right=parent.right-offsetxr;
	client.left=client.right-editx;
	pw->MoveWindow(&client,true);
	UpdateData(false);
	GetParent()->GetClientRect(&parent);
	GetWindowRect(&client);
	delta=client;
	client.right=parent.right;
	client.bottom=parent.bottom;
	client.left=parent.right-(delta.right-delta.left);
	client.top=parent.bottom-(delta.bottom-delta.top);
	if (MovieWinSize.top || MovieWinSize.width || MovieWinSize.height || MovieWinSize.left) {
		long width, hight;

		GetWindowRect(&client);
		width = client.right - client.left;
		hight = client.bottom - client.top;
		client.top = MovieWinSize.top<0?-2*MovieWinSize.top:MovieWinSize.top;
		client.bottom = client.top + hight;
		client.left = MovieWinSize.left;
		client.right = client.left + width;
		AdjustWindowSize(&client);
	}
	MoveWindow(&client,true);
	EnableToolTips();
}

void CQTDlg::ResizeQTDlgWindowItems(void) {
	double ratio, th, tw;
	TimeValue tm_from,tm_pos,tm_to;
	short h, w, isRedo;
	RECT MvBounds,orgMvBounds;

	if (MVWinWidth != 0 || MVWinZoom != 1) {
		isRedo = 0;
		if (MVWinZoom != 1)
			w = 0;
		else
			w = MVWinWidth;

		orgMvBounds.top = qt->orgMvBounds.top;
		orgMvBounds.bottom = qt->orgMvBounds.bottom;
		orgMvBounds.left = qt->orgMvBounds.left;
		orgMvBounds.right = qt->orgMvBounds.right;

		MvBounds.top = qt->MvBounds.top;
		MvBounds.bottom = qt->MvBounds.bottom;
		MvBounds.left = qt->MvBounds.left;
		MvBounds.right = qt->MvBounds.right;

		tm_from = m_from;
		tm_pos = m_pos;
		tm_to = m_to;

		if (w == 0) {
			do {
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
				if (/*isInsideTotalScreen(&MvBounds) && */w > 170 && h > 140)
					break;
				else if (isRedo == 0)
					isRedo = 1;
			} while (1) ;
		} else {
			th = (double)(orgMvBounds.bottom - orgMvBounds.top);
			tw = (double)(orgMvBounds.right - orgMvBounds.left);
			ratio = th / tw;
			tw = w;
			MvBounds.right = MvBounds.left + w;
			MvBounds.bottom = MvBounds.top + (short)(tw * ratio);
			MVWinWidth = w;
		}

		qt->orgMvBounds.top = orgMvBounds.top;
		qt->orgMvBounds.bottom = orgMvBounds.bottom;
		qt->orgMvBounds.left = orgMvBounds.left;
		qt->orgMvBounds.right = orgMvBounds.right;

		qt->MvBounds.top = MvBounds.top;
		qt->MvBounds.bottom = MvBounds.bottom;
		qt->MvBounds.left = MvBounds.left;
		qt->MvBounds.right = MvBounds.right;


	}
	qt->MoveWindow(0,0,qt->MvBounds.right-qt->MvBounds.left,qt->MvBounds.bottom-qt->MvBounds.top,true);
	ResizeQTDlgWindow(qt);
}

void CQTDlg::ResizeMovie(short w, short isRedo, BOOL isRefreshQT) {
	double ratio, th, tw;
	TimeValue tm_from,tm_pos,tm_to;
	short			h;
	RECT MvBounds,orgMvBounds;

	orgMvBounds.top = qt->orgMvBounds.top;
	orgMvBounds.bottom = qt->orgMvBounds.bottom;
	orgMvBounds.left = qt->orgMvBounds.left;
	orgMvBounds.right = qt->orgMvBounds.right;

	MvBounds.top = qt->MvBounds.top;
	MvBounds.bottom = qt->MvBounds.bottom;
	MvBounds.left = qt->MvBounds.left;
	MvBounds.right = qt->MvBounds.right;

	tm_from = m_from;
	tm_pos = m_pos;
	tm_to = m_to;

	if (w == 0) {
		do {
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
			if (/*isInsideTotalScreen(&MvBounds) && */w > 170 && h > 140)
				break;
			else if (isRedo == 0)
				isRedo = 1;
		} while (1) ;
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
	
	
	if (qt)	{
		qt->DestroyWindow() ;
		qt=NULL;
	}
	
	qt=new CQT(pathname,this,false);

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
		if (isRefreshQT) {
			qt->SetStartTime((TimeValue) m_from);
			qt->SetDuration((TimeValue) (1));
			qt->Movie_Load(tm_from, FALSE);
			qt->Play();
		}
		m_from = tm_from;
		m_pos = tm_pos;
		m_to = tm_to;
		UpdateData(false);
	}else{
		DestroyWindow();
	}
}


void CQTDlg::OnSysCommand(UINT nID, LPARAM lParam)
{

	if (nID==SC_CLOSE)
		DestroyWindow();
	else if (nID==0xF032)
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
	else if (nID==SC_MAXIMIZE)
		ResizeMovie(0, 1, true);
	else {
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

void CQTDlg::OnSize(UINT nType, int cx, int cy) 
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
		ResizeMovie(cx, cy, true);
	}
	// TODO: Add your message handler code here
	
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CQTDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CQTDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CQTDlg::DestroyWindow() 
{
	CRect lpRect;

	PlayingContMovie = FALSE;
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
	WriteCedPreference();
	if (qt)	{
		qt->DestroyWindow() ;
		qt=NULL;
	}
	
	return CDialog::DestroyWindow();
}

void CQTDlg::OnButtonPlay() 
{
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	// TODO: Add your control notification handler code here
	UpdateData(true);

	if (PBC.enable && PBC.isPC && PBC.walk) {
		if (qt && qt->isPlaying && qt->isPlaying != 3) {
			PlayingContMovie = false;
			StopMovie(qt->theMovie);
			qt->isPlaying = 0;
			isPlayRange = false;
			PBC.walk = 0;
			PBC.isPC = 0;
		}
		do_warning("To repeat use F7 or F8 command. This buttons stops playback in Walker Controller.", 0);
	} else if (qt && qt->isPlaying && qt->isPlaying != 3) {
		PlayingContMovie = false;
		StopMovie(qt->theMovie);
		qt->isPlaying = 0;
		isPlayRange = false;
		PBC.walk = 0;
		PBC.isPC = 0;
	} else if (qt) {
		qt->isPlaying = 3;
		Play();
	}

	if (!PlayingContMovie || (PBC.walk && PBC.enable))
		;
	else
		GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CQTDlg::OnButtonSave() 
{
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (PlayingContMovie=='\003') {
		if (textDoc != NULL && textDoc->FileInfo != NULL) {
			TimeValue cpos = qt->GetCurTime();
			if (cpos) textDoc->ChangeCursorInfo=cpos;
			else textDoc->ChangeCursorInfo=1;
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
		case IDC_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CQTDlg::OnCancel()
{
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

void CQTDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	delete this;
	MovDlg=NULL;
	CDialog::PostNcDestroy();
}

void CQTDlg::SetCurPosToValue(long tTime){
	if (qt) {
		StopMovie(qt->theMovie);
		qt->isPlaying = 0;
		isPlayRange = false;
		m_pos = tTime;
		MoviesTask(qt->theMovie, 0);
		qt->FakeMove(m_pos);
		PBC.walk = 0;
		PBC.isPC = 0;
	}
	UpdateData(false);
}

void CQTDlg::SetCurPos(char passive){
	BOOL justStoped = false;
	TimeValue pos=0;
	if (passive){
		pos = qt->GetCurTime();
		if (m_pos!=pos || passive == 2){
			last_m_pos = m_pos;
			m_pos=pos;
			if (isPlayRange && qt->isPlaying) {
				if (last_m_pos == m_pos)
					same_m_posValue++;
				else
					same_m_posValue = 0;
			} else
				same_m_posValue = 0;
			if (PlayingContMovie && PlayingContMovie!='\003' && m_pos < real_to) {
				if (textDoc != NULL && textDoc->FileInfo != NULL) {
					if (m_pos > textDoc->FileInfo->MvTr.MEnd) {
						textDoc->ChangeCursorInfo=m_pos;
						textDoc->myUpdateAllViews(FALSE);
					}
				}
			}
			if (!qt->isPlaying && PBC.enable && !PBC.walk) {
				textDoc->FileInfo->MvTr.MBeg = m_pos;
				PBC.cur_pos = m_pos;
			}
/*
			if (passive == 2) {
				if (!PlayingContMovie || PlayingContMovie=='\003') {
					StopMovie(qt->theMovie);
					qt->isPlaying = 0;
					isPlayRange = false;
					PlayingContMovie = false;
				}
				if (qt && textDoc && textDoc->FileInfo != NULL) {
					qt->isPlaying = 0;
					if (textDoc->FileInfo->cpMvTr != NULL) {
						isPlayRange = false;
						PlayingContMovie=false;
						textDoc->RestorePlayMovieInfo=TRUE;
						textDoc->myUpdateAllViews(FALSE);
					} else if (PBC.enable && PBC.isPC) {
						isPlayRange = false;
						PlayingContMovie=false;
						textDoc->DoWalkerController=TRUE;
						textDoc->myUpdateAllViews(FALSE);
					}
				}
			} else 
*/

			if (passive == 2) {
				m_pos=pos;
			}
			if ((m_pos>=real_to-1 || (m_pos>=real_to-10 && same_m_posValue > 6)) && isPlayRange && qt->isPlaying) {
				m_pos = real_to;
				if (!PlayingContMovie || PlayingContMovie=='\003' || PlayingContMovie=='\002') {
					StopMovie(qt->theMovie);
					qt->isPlaying = 0;
					isPlayRange = false;
					PlayingContMovie = false;
					justStoped = true;
				}
				if (qt && textDoc && textDoc->FileInfo != NULL && (justStoped || qt->IsQTMovieDone())) {
					qt->isPlaying = 0;
					if (textDoc->FileInfo->cpMvTr != NULL) {
						isPlayRange = false;
						PlayingContMovie=false;
						textDoc->RestorePlayMovieInfo=TRUE;
						textDoc->myUpdateAllViews(FALSE);
					} else if (PBC.enable && PBC.isPC) {
						isPlayRange = false;
						PlayingContMovie=false;
						textDoc->DoWalkerController=TRUE;
						textDoc->myUpdateAllViews(FALSE);
					}
				}
			} else if (passive != 2 && !isPlayRange && !qt->isPlaying && PBC.enable) {
				if (textDoc != NULL && textDoc->FileInfo != NULL)
					textDoc->FileInfo->MvTr.MBeg = pos;
			}
			UpdateData(false);
		}
	} else{
		UpdateData(true);
		if (qt) {
			if (!qt->isPlaying && PBC.enable && !PBC.walk) {
				textDoc->FileInfo->MvTr.MBeg = m_pos;
				PBC.cur_pos = m_pos;
			}
			StopMovie(qt->theMovie);
			qt->isPlaying = 0;
			isPlayRange = false;
			MoviesTask(qt->theMovie, 0);
			qt->FakeMove(m_pos);
			PBC.walk = 0;
			PBC.isPC = 0;
		}
		UpdateData(false);
	}
}

void CQTDlg::OnSetBegin() 
{
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (m_inFocus == IDC_EDIT_FROM || m_inFocus == IDC_EDIT_TO) {
		UpdateData(true);
		m_from=m_pos;
		UpdateData(false);
	} else if (m_inFocus == IDC_EDIT_POS) {
		UpdateData(true);
		m_pos=m_from;
		UpdateData(false);
	}

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CQTDlg::OnSetEnd() 
{
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (m_inFocus == IDC_EDIT_FROM || m_inFocus == IDC_EDIT_TO) {
		UpdateData(true);
		m_to=m_pos;
		UpdateData(false);
	} else if (m_inFocus == IDC_EDIT_POS) {
		UpdateData(true);
		m_pos=m_to;
		UpdateData(false);
	}
	if (qt) {
		StopMovie(qt->theMovie);
		qt->isPlaying = 0;
		isPlayRange = false;
		PBC.walk = 0;
		PBC.isPC = 0;
	}

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CQTDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (qt) {
		PlayingContMovie = false;
		StopMovie(qt->theMovie);
		qt->isPlaying = 0;
		isPlayRange = false;
		PBC.walk = 0;
		PBC.isPC = 0;
	}
	CDialog::OnLButtonDown(nFlags, point);
}

BOOL CQTDlg::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	// TODO: Add your specialized code here and/or call the base class
 	return 1;//CDialog::OnChildNotify(message, wParam, lParam, pLResult);
}

void CQTDlg::SetWaveTimeValue(long timeBeg, long timeEnd) {
	long t;
	myFInfo *saveGlobal_df;

	if (textDoc != NULL && textDoc->FileInfo != NULL) {
		saveGlobal_df = global_df;
		global_df = textDoc->FileInfo;
		if (syncAudioAndVideo(global_df->MvTr.MovieFile)) {
//			DrawSoundCursor(0);
			t =  AlignMultibyteMediaStream(conv_from_msec_rep(timeBeg), '+');
			if (t != global_df->SnTr.BegF)
				global_df->SnTr.BegF = t;
			if (timeEnd == 0L)
				global_df->SnTr.EndF = 0L;
			else {
				t = AlignMultibyteMediaStream(conv_from_msec_rep(timeEnd), '+');
				if (t == global_df->SnTr.BegF)
					global_df->SnTr.EndF = 0L;
				else if (t > global_df->SnTr.BegF && t != global_df->SnTr.EndF)
					global_df->SnTr.EndF = t;
			}
			DisplayEndF(TRUE);
			textDoc->UpdateAllViews(NULL, 0L, NULL);
//			DrawSoundCursor(1);
		}
		global_df = saveGlobal_df;
	}
}

void CQTDlg::OnUpdateEditPos() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	// TODO: Add your control notification handler code here
	if (qt) {
		UpdateData(true);
		SetWaveTimeValue(m_pos, 0L);
		SetCurPos(0);
	}
}

void CQTDlg::OnUpdateEditFrom()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	if (qt) {
		UpdateData(true);
		SetWaveTimeValue(m_from, m_to);
	}
}

void CQTDlg::OnUpdateEditTo()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	if (qt) {
		UpdateData(true);
		SetWaveTimeValue(m_from, m_to);
	}
}

void CQTDlg::OnSetfocusEditPos() 
{
	SetDlgItemText(IDC_SET_BEGIN, _T("^"));
	SetDlgItemText(IDC_SET_END, _T("^"));
	m_inFocus = IDC_EDIT_POS;
}

void CQTDlg::OnSetfocusEditTo() 
{
	SetDlgItemText(IDC_SET_BEGIN, _T("v"));
	SetDlgItemText(IDC_SET_END, _T("v"));
	m_inFocus = IDC_EDIT_TO;
}

void CQTDlg::OnSetfocusEditFrom() 
{
	SetDlgItemText(IDC_SET_BEGIN, _T("v"));
	SetDlgItemText(IDC_SET_END, _T("v"));
	m_inFocus = IDC_EDIT_FROM;
}

void CQTDlg::OnSetfocusEditDelay() 
{
	SetDlgItemText(IDC_SET_BEGIN, _T("-"));
	SetDlgItemText(IDC_SET_END, _T("-"));
	m_inFocus = IDC_EDIT_DELAY;
}

BOOL CQTDlg::ShowToolTip( UINT id, NMHDR * pTTTStruct, LRESULT * pResult )
{
	NMTTDISPINFO* p=(NMTTDISPINFO*)pTTTStruct;
	UINT nID =pTTTStruct->idFrom;
	if (p->uFlags & TTF_IDISHWND){
		nID = ::GetDlgCtrlID((HWND)nID);
	}
	switch (nID){
	case IDC_BUTTON_PLAY:
		p->lpszText= _T("I play the selection specified") ;
		break;
	case IDC_BUTTON_SAVE:
		p->lpszText= _T("I save the selection specified in the current text file") ;
		break;
	case IDC_EDIT_FROM:
		p->lpszText= _T("I display/set the starting point of selection") ;
		break;
	case IDC_EDIT_TO:
		p->lpszText= _T("I display/set the ending point of selection") ;
		break;
	case IDC_EDIT_POS:
		p->lpszText= _T("I follow/set the current position in the movie") ;
		break;
	case IDC_SET_BEGIN:
//		p->lpszText= _T("I align the beginning of selection (below me) to the current position in the movie (above me)") ;
		break;
	case IDC_SET_END:
//		p->lpszText= _T("I align the end of selection (below me) to the current position in the movie (above me)") ;
		break;
	case IDC_EDIT_DELAY:
		p->lpszText= _T("I control the size of clip played, negative number mean from the beginning and positive - from the end") ;
		break;
	}
	return 1;
}

void CQTDlg::ExecuteCommNumber(int num)
{
	if (textDoc != NULL && textDoc->FileInfo != NULL) {
		textDoc->FileInfo->MvTr.MBeg = m_from;
		textDoc->FileInfo->MvTr.MEnd = m_to;
		textDoc->ExecuteCommNUM = num;
		textDoc->myUpdateAllViews(FALSE);
	}
}

void CQTDlg::OnToggleMovieText() 
{
	if (textDoc != NULL && textDoc->FileInfo != NULL) {
		AfxGetApp()->m_pMainWnd->SetActiveWindow();
	}
}

#define VK_SHIFT_USED (GetAsyncKeyState(VK_SHIFT) & 0x8000)
#define VK_CTRL_USED  (GetAsyncKeyState(VK_CONTROL) & 0x8000)

BOOL CQTDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_CHAR && pMsg->wParam == 17/*^W*/) {
		PlayingContMovie = false;
		if (qt) {
			StopMovie(qt->theMovie);
			qt->isPlaying = 0;
			isPlayRange = false;
			PBC.walk = 0;
			PBC.isPC = 0;
		}
		isPlayRange = false;
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
		return TRUE;
	} else if (pMsg->message == WM_CHAR && pMsg->wParam == 9/*^I*/) {
		if (PlayingContMovie=='\003') {
			if (textDoc != NULL && textDoc->FileInfo != NULL) {
				TimeValue cpos = qt->GetCurTime();
				if (cpos) textDoc->ChangeCursorInfo=cpos;
				else textDoc->ChangeCursorInfo=1;
				textDoc->myUpdateAllViews(FALSE);
			}
		} else {
			SendMessage(WM_COMMAND, IDC_BUTTON_SAVE, 0);
		}
		return TRUE;
	} else if (pMsg->message == WM_KEYDOWN) {
		if (textDoc != NULL && textDoc->isTranscribing) {
			if (pMsg->wParam == VK_F5 && F5_Offset == 0L) {
				if (qt && qt->isPlaying) {
					PlayingContMovie = false;
					StopMovie(qt->theMovie);
					qt->isPlaying = 0;
					isPlayRange = false;
					PBC.walk = 0;
					PBC.isPC = 0;
				}
			} else if (pMsg->wParam == VK_SPACE) {
				if (textDoc != NULL && textDoc->FileInfo != NULL) {
					TimeValue cpos = qt->GetCurTime();
					if (cpos) textDoc->ChangeCursorInfo=cpos;
					else textDoc->ChangeCursorInfo=1;
					textDoc->myUpdateAllViews(FALSE);
				} else {
					PlayingContMovie = false;
					if (qt) {
						StopMovie(qt->theMovie);
						qt->isPlaying = 0;
						isPlayRange = false;
						PBC.walk = 0;
						PBC.isPC = 0;
					}
					isPlayRange = false;
					do_warning("Internal error with movie to text link. Please quit CLAN and try again.",0);
				}
				return TRUE;
			}
		} else if (pMsg->wParam == VK_RETURN) {
			m_PlayToEnd = false;
			SendMessage(WM_COMMAND, IDC_BUTTON_PLAY, 0);
			return TRUE;
		} else if (pMsg->wParam == VK_SPACE) {
			m_PlayToEnd = true;
			SendMessage(WM_COMMAND, IDC_BUTTON_PLAY, 0);
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
		} else if (pMsg->wParam == 0xBF && VK_CTRL_USED) {
			OnToggleMovieText();
//			return TRUE;
		} else if (pMsg->wParam == VK_LEFT && VK_SHIFT_USED && VK_CTRL_USED) {
		//		int count;
			if (GetFocus( ) != GetDlgItem(IDC_EDIT_DELAY)) {
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
				SetCurPos(0);
				return TRUE;
			}
		} else if (pMsg->wParam == VK_RIGHT && VK_SHIFT_USED && VK_CTRL_USED) {
		//		int count;
			if (GetFocus( ) != GetDlgItem(IDC_EDIT_DELAY)) {
				UpdateData(true);
				if (GetTickCount()-tDelay < 400)
					keyDNcount++;
				else
					keyDNcount = 0;
				tDelay = GetTickCount();
		//			count = (int)(pMsg->lParam & 0xFFFF);
				m_pos += ((keyDNcount < 8) ? 1 : 50);
				if (m_pos > qt->GetQTMovieDuration())
					m_pos = qt->GetQTMovieDuration();
				UpdateData(false);
				SetCurPos(0);
				return TRUE;
			}
		} else if (pMsg->wParam == VK_UP && VK_SHIFT_USED && VK_CTRL_USED) {
			if (GetFocus( ) == GetDlgItem(IDC_EDIT_FROM)) {
				UpdateData(true);
				m_pos = m_from;
				UpdateData(false);
				SetCurPos(0);
				return TRUE;
			} else if (GetFocus( ) == GetDlgItem(IDC_EDIT_TO)) {
				UpdateData(true);
				m_pos = m_to;
				UpdateData(false);
				SetCurPos(0);
				return TRUE;
			} else {
				return TRUE;
			}
		} else if (pMsg->wParam == VK_DOWN && VK_SHIFT_USED && VK_CTRL_USED) {
			if (GetFocus( ) == GetDlgItem(IDC_EDIT_FROM)) {
				UpdateData(true);
				m_from = m_pos;
				UpdateData(false);
				return TRUE;
			} else if (GetFocus( ) == GetDlgItem(IDC_EDIT_TO)) {
				UpdateData(true);
				m_to = m_pos;
				UpdateData(false);
				return TRUE;
			} else {
				return TRUE;
			}
		} else if (pMsg->wParam == VK_LEFT && VK_SHIFT_USED) {
			UpdateData(true);
			m_to -= MOVEINCREMENT;
			if (m_to < 0)
				m_to = 0;
			m_pos = m_to;
			UpdateData(false);
			SetCurPos(0);
			ExecuteCommNumber(93);
			return TRUE;
		} else if (pMsg->wParam == VK_LEFT && VK_CTRL_USED) {
			UpdateData(true);
			m_from -= MOVEINCREMENT;
			if (m_from < 0)
				m_from = 0;
			m_pos = m_from;
			UpdateData(false);
			SetCurPos(0);
			ExecuteCommNumber(95);
			return TRUE;
		} else if (pMsg->wParam == VK_RIGHT && VK_SHIFT_USED) {
			UpdateData(true);
			m_to += MOVEINCREMENT;
			if (m_to > qt->GetQTMovieDuration())
				m_to = qt->GetQTMovieDuration();
			m_pos = m_to;
			UpdateData(false);
			SetCurPos(0);
			ExecuteCommNumber(92);
			return TRUE;
		} else if (pMsg->wParam == VK_RIGHT && VK_CTRL_USED) {
			UpdateData(true);
			m_from += MOVEINCREMENT;
			if (m_from > qt->GetQTMovieDuration())
				m_from = qt->GetQTMovieDuration();
			m_pos = m_from;
			UpdateData(false);
			SetCurPos(0);
			ExecuteCommNumber(94);
			return TRUE;
		} else
			return CDialog::PreTranslateMessage(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

LRESULT CQTDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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

void CQTDlg::OnQdHelp()
{
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (qt) {
		PlayingContMovie = false;
		StopMovie(qt->theMovie);
		qt->isPlaying = 0;
		isPlayRange = false;
		PBC.walk = 0;
		PBC.isPC = 0;
	}
	isPlayRange = false;
	if (MVHelpDlg == NULL) {
		MVHelpDlg=new CMyHelp(AfxGetApp()->m_pMainWnd);
		MVHelpDlg->Create(IDD_MYHELP);
	} else
		MVHelpDlg->SetActiveWindow();

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}
