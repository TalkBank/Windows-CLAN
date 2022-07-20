// MpegDlg.cpp : implementation file
//

//#define USETIMER

#include "ced.h"
#include "MMedia.h"
#include "Clan2Doc.h"
#include "MpegDlg.h"
#include "MVHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG

extern struct DefWin MovieWinSize;
extern char  rptMark;
extern int	 rptPTime;

CMpegDlg *MpegDlg = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMpegDlg dialog

#define __AFXCONV_H__
#include "afxpriv.h"


CMpegDlg::CMpegDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMpegDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMpegDlg)
#ifdef USETIMER
	timer = 0;
#endif
	m_from = 0;
	m_pos = 0;
	m_to = 0;
	m_rptstr = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	mpeg = NULL;
	textDoc=NULL;
	doErase = TRUE;
	isPlayRange = FALSE;
	tDelay = 0;
	keyDNcount = 0;
	m_inFocus = IDC_MPG_EDIT_FROM;
	m_PlayToEnd = FALSE;
	m_isUserResize = FALSE;
	if (textDoc != NULL)
		textDoc->isTranscribing = FALSE;
}

void CMpegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMpegDlg)
	DDX_Control(pDX, IDC_MPG_EDIT_TO, m_ToCtrl);
	DDX_Control(pDX, IDC_MPG_EDIT_POS, m_PosCtrl);
	DDX_Control(pDX, IDC_MPG_EDIT_FROM, m_FromCtrl);
	DDX_Control(pDX, IDC_MPG_EDIT_DELAY, m_DelayCtrl);
	DDX_Control(pDX, IDC_SPIN1, m_spin);
	DDX_Text(pDX, IDC_MPG_EDIT_FROM, m_from);
	DDX_Text(pDX, IDC_MPG_EDIT_POS, m_pos);
	DDX_Text(pDX, IDC_MPG_EDIT_TO, m_to);
	DDX_Text(pDX, IDC_MPG_EDIT_DELAY, m_rptstr);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMpegDlg, CDialog)
	//{{AFX_MSG_MAP(CMpegDlg)
	ON_BN_CLICKED(IDC_BEG_BUTTON, OnBegButton)
	ON_BN_CLICKED(IDC_END_BUTTON, OnEndButton)
	ON_BN_CLICKED(IDC_SAVE_BUTTON, OnSaveButton)
	ON_BN_CLICKED(IDC_PLAY_BUTTON, OnPlayButton)
	ON_BN_CLICKED(IDC_MPG_HELP, OnMpgHelp)
	ON_EN_UPDATE(IDC_MPG_EDIT_POS, OnUpdateEditPos)
	ON_EN_UPDATE(IDC_MPG_EDIT_FROM, OnUpdateEditFrom)
	ON_EN_UPDATE(IDC_MPG_EDIT_TO, OnUpdateEditTo)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	ON_WM_PAINT()
#ifdef USETIMER
	ON_WM_TIMER()
#endif
	ON_WM_LBUTTONDOWN()
	ON_EN_SETFOCUS(IDC_MPG_EDIT_DELAY, OnSetfocusMpgEditDelay)
	ON_EN_SETFOCUS(IDC_MPG_EDIT_FROM, OnSetfocusMpgEditFrom)
	ON_EN_SETFOCUS(IDC_MPG_EDIT_POS, OnSetfocusMpgEditPos)
	ON_EN_SETFOCUS(IDC_MPG_EDIT_TO, OnSetfocusMpgEditTo)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMpegDlg message handlers

BOOL CMpegDlg::OnInitDialog() {
	RECT client;

	UpdateData(FALSE);
	CDialog::OnInitDialog();

	myhelpButton.AutoLoad(IDC_MPG_HELP, this);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	textDC = NULL;
	textDoc = NULL;

	// TODO: Add extra initialization here
#ifdef USETIMER
	int i = 0;
	do{
		i++;
		timer=SetTimer(i,10,NULL);
	} while (timer==0);
#endif
	if (MovieWinSize.top || MovieWinSize.width || MovieWinSize.height || MovieWinSize.left) {
		client.top = MovieWinSize.top;
		client.bottom = MovieWinSize.height;
		client.left = MovieWinSize.left;
		client.right = MovieWinSize.width;

		MoveWindow(&client, TRUE);
	}
	GetClientRect(&MvBounds);
	RECT delta;
	CWnd *pw;
	pw = GetDlgItem(IDC_MPG_EDIT_POS);
	pw->GetWindowRect(&delta);
	CTRLoffset = (delta.bottom - delta.top) * 5L;
	MvBounds.bottom = MvBounds.bottom - CTRLoffset;
	mpeg = new CMpeg(pathname, this);
	if (mpeg->Create(IDD_MPEG, this)) {
		ResizeMpegDlgWindow(mpeg);
	} else {
		DestroyWindow();
	}

	return TRUE;
}

void CMpegDlg::PlaySetting(void) {
	long t_rptPTime, dur;

	dur = mpeg->GetMpegMovieDuration();
	m_to = dur;

	if (mvRec->MEnd > dur)
		mvRec->MEnd = dur;

	if (PlayingContMovie != '\003') {
		m_to = mvRec->MEnd;
		m_PlayToEnd = false;
	}
	m_from = mvRec->MBeg;
	m_pos = m_from;

	if (isJustOpen == TRUE) {
		m_from = 0;
		m_pos = m_from;
		m_to = 1;
	}
	real_to = 0;
	isPlayRange = TRUE;
	if (m_PlayToEnd) {
		m_PlayToEnd = FALSE;
		mpeg->SetStartTime(m_pos);
		mpeg->SetDuration(mpeg->GetMpegMovieDuration() - m_pos);
		real_to = mpeg->GetMpegMovieDuration();
	} else if (m_to == 0) {
		m_pos = m_from;
		m_to = mpeg->GetMpegMovieDuration();
		UpdateData(FALSE);
		mpeg->SetStartTime(1);
		mpeg->SetDuration(1);
	} else if (PlayingContMovie) {
		mpeg->SetStartTime(m_from);
		mpeg->SetDuration(m_to - m_from);
		real_to = m_to;
	} else if (*m_rptstr == 'e' || *m_rptstr == 'E') {
		t_rptPTime = uS.atol(m_rptstr.Mid(1));
		mpeg->SetStartTime(m_to - t_rptPTime);
		mpeg->SetDuration(t_rptPTime);
		real_to = m_to;
	} else if (*m_rptstr == 'b' || *m_rptstr == 'B') {
		t_rptPTime = uS.atol(m_rptstr.Mid(1));
		mpeg->SetStartTime(m_from);
		mpeg->SetDuration(t_rptPTime);
		real_to = m_from + t_rptPTime;
	} else if (*m_rptstr == '-') {
		t_rptPTime = uS.atol(m_rptstr.Mid(1));
		if (m_from - t_rptPTime < 0)
			t_rptPTime = 0;
		else
			t_rptPTime = m_from - t_rptPTime;
		mpeg->SetStartTime(t_rptPTime);
		mpeg->SetDuration(m_to - t_rptPTime);
		real_to = m_to;
	} else if (*m_rptstr == '+') {
		t_rptPTime = uS.atol(m_rptstr.Mid(1));
		if (m_to + t_rptPTime > mpeg->GetMpegMovieDuration())
			t_rptPTime = mpeg->GetMpegMovieDuration();
		else
			t_rptPTime = m_to + t_rptPTime;
		mpeg->SetStartTime(m_from);
		mpeg->SetDuration(t_rptPTime - m_from);
		real_to = t_rptPTime;
	} else {
		t_rptPTime = uS.atol(m_rptstr);
		if (t_rptPTime > 0) {
			if (m_to + t_rptPTime > mpeg->GetMpegMovieDuration())
				t_rptPTime = mpeg->GetMpegMovieDuration();
			else
				t_rptPTime = m_to + t_rptPTime;
			mpeg->SetStartTime(m_from);
			mpeg->SetDuration(t_rptPTime - m_from);
			real_to = t_rptPTime;
		} else {
			mpeg->SetStartTime(m_from);
			mpeg->SetDuration(m_to - m_from);
			real_to = m_to;
		}
	}
	last_m_pos = 0;
	same_m_posValue = 0;
}

void CMpegDlg::Play(){
	if (textDoc != NULL)
		textDoc->isTranscribing = (BOOL)(PlayingContMovie == '\003');
	if (mpeg) {
		if (textDoc != NULL)
			textDoc->isTranscribing = (BOOL)(PlayingContMovie == '\003');
		if (mpeg->isMovieDurationSet == TRUE) {
			PlaySetting();
			mpeg->isPlaying = 1;
		} else {
			mpeg->SetStartTime(0);
			mpeg->isPlaying = 3;
			m_pos = m_from;
			UpdateData(FALSE);
		}
		if (PlayingContMovie && PlayingContMovie != '\003') {
			if (m_pos)
				textDoc->ChangeCursorInfo = m_pos;
			else
				textDoc->ChangeCursorInfo = 1;
			textDoc->myUpdateAllViews(FALSE);
		}
		mpeg->Play();
	}
}

void CMpegDlg::OnSysCommand(UINT nID, LPARAM lParam)
{

	if (nID==SC_CLOSE)
		DestroyWindow();
	else if (nID==0xF032)
		;
//	else if (nID==SC_RESTORE)
//		i = 4;
//	else if (nID==SC_DEFAULT)
//		i = 3;
//	else if (nID==SC_MINIMIZE)
//		i = 2;
//	else if (nID==SC_SIZE)
//		i = 1;
//	else if (nID==SC_MAXIMIZE)
	else {
		if (nID >= 0xF001 && nID <= 0xF008) {
			if (nID == 0xF006 || nID == 0xF003)
				m_isUserResize = TRUE;
			else
				m_isUserResize = TRUE;
 		}
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CMpegDlg::OnSize(UINT nType, int cx, int cy) 
{
	CRect lpRect;

	CDialog::OnSize(nType, cx, cy);
	
	if (m_isUserResize == TRUE) {
//		m_isUserResize = FALSE;

		UpdateData(TRUE);
		GetWindowRect(&lpRect);
		MovieWinSize.top = lpRect.top;
		MovieWinSize.left = lpRect.left;
		MovieWinSize.width = lpRect.right;
		MovieWinSize.height = lpRect.bottom;
		WriteCedPreference();

		MvBounds.right = MvBounds.left + cx;
		MvBounds.bottom = MvBounds.top + cy;
		MvBounds.bottom = MvBounds.bottom - CTRLoffset;

		mpeg->m_wndView.MoveWindow(MvBounds.left, MvBounds.top, MvBounds.right, MvBounds.bottom);
		mpeg->MoveWindow(0, 0, MvBounds.right - MvBounds.left, MvBounds.bottom - MvBounds.top, TRUE);
		ResizeMpegDlgWindow(mpeg);
		UpdateData(FALSE);
	}
}

void CMpegDlg::ResizeMpegDlgWindow(CMpeg *mpeg) {
	RECT delta, client, parent;
	long editx, edity, tButx, tButy, offsetxl, offsetxr, offsety;
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

	mpeg->GetWindowRect(&parent);
	client = parent;
	pw = GetDlgItem(IDC_MPG_EDIT_POS);
	pw->GetWindowRect(&delta);
	editx = delta.right - delta.left;
	edity = delta.bottom - delta.top;
	parent.bottom += 5 * edity;
	if ((parent.right - parent.left)<3 * editx)
		parent.right = parent.left + 3 * editx;
	CalcWindowRect(&parent);
	offsetxl = client.left - parent.left;
	offsetxr = parent.right - client.right;
	MoveWindow(&parent, TRUE);

	GetClientRect(&parent);
	if (parent.top < 0) {
		parent.bottom = parent.bottom - parent.top;
		parent.top = 0;
	}
	if (parent.left < 0) {
		parent.right = parent.right - parent.left;
		parent.left = 0;
	}
	client = parent;

	pw = GetDlgItem(IDC_PLAY_BUTTON);
	client.top = client.bottom - edity;
	client.left = offsetxl;
	client.right = offsetxl + editx;
	pw->MoveWindow(&client, TRUE);

	pw = GetDlgItem(IDC_MPG_EDIT_DELAY);
	client.left += editx + 2;
	client.right = client.left + editx;
	pw->MoveWindow(&client, TRUE);

	pw = GetDlgItem(IDC_MPG_EDIT_MSEC);
	pw->GetWindowRect(&delta);
	client.left += editx + 2;
	client.right = client.left + (delta.right - delta.left);
	pw->MoveWindow(&client, TRUE);

	pw = GetDlgItem(IDC_MPG_EDIT_FROM);
	client.bottom -= edity;
	client.left = offsetxl;
	client.top = client.bottom - edity;
	client.right = offsetxl + editx;
	pw->MoveWindow(&client, TRUE);

	pw = GetDlgItem(IDC_MPG_EDIT_TO);
	client.left += editx + 2;
	client.right = client.left + editx;
	pw->MoveWindow(&client, TRUE);
	
	pw = GetDlgItem(IDC_MPG_EDIT_POS);
	client.left = .5*editx + offsetxl;
	client.right = 1.5*editx + offsetxl;
	client.top -= 2.5*edity;
	offsety = client.top;
	client.bottom -= 2.5*edity;
	pw->MoveWindow(&client, TRUE);

	delta = client;
	pw = GetDlgItem(IDC_BEG_BUTTON);
	pw->GetWindowRect(&client);
	tButx = client.right - client.left;
	tButy = client.bottom - client.top;
	client.top = delta.bottom;
	client.bottom = client.top + tButy;
	client.left = delta.left;
	client.right = client.left + tButx;
	client.top += tButy / 5;
	client.bottom += tButy / 5;
	pw->MoveWindow(&client, TRUE);

	pw = GetDlgItem(IDC_END_BUTTON);
	client.right = delta.right;
	client.left = client.right - tButx;
	pw->MoveWindow(&client);

	pw = GetDlgItem(IDC_SAVE_BUTTON);
	client.top = offsety;
	client.bottom = offsety + edity;
	client.right = parent.right - offsetxr;
	client.left = client.right - editx;
	pw->MoveWindow(&client, TRUE);

	pw = GetDlgItem(IDC_MPG_HELP);
	pw->GetWindowRect(&client);
	editx = client.right - client.left;
	edity = client.bottom - client.top;
	client.bottom = parent.bottom - offsetxr;
	client.top = client.bottom - edity;
	client.right = parent.right - offsetxr;
	client.left = client.right - editx;
	pw->MoveWindow(&client, TRUE);
	UpdateData(FALSE);
	GetParent()->GetClientRect(&parent);
	GetWindowRect(&client);
	delta = client;
	client.right = parent.right;
	client.bottom = parent.bottom;
	client.left = parent.right - (delta.right - delta.left);
	client.top = parent.bottom - (delta.bottom - delta.top);

	if (MovieWinSize.top || MovieWinSize.width || MovieWinSize.height || MovieWinSize.left) {
		long width, hight;

//		GetWindowRect(&client);
		width = client.right - client.left;
		hight = client.bottom - client.top;
		client.top = MovieWinSize.top<0 ? -2 * MovieWinSize.top : MovieWinSize.top;
		client.bottom = client.top + hight;
		client.left = MovieWinSize.left;
		client.right = client.left + width;
//		AdjustWindowSize(&client);
	}
	MoveWindow(&client, TRUE);
	EnableToolTips();
}

void CMpegDlg::OnPaint() 
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

HCURSOR CMpegDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMpegDlg::OnBegButton() 
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
		UpdateData(TRUE);
		m_from=m_pos;
		UpdateData(FALSE);
	} else if (m_inFocus == IDC_MPG_EDIT_POS) {
		UpdateData(TRUE);
		m_pos=m_from;
		UpdateData(FALSE);
	}

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CMpegDlg::OnEndButton() 
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
		UpdateData(TRUE);
		m_to=m_pos;
		UpdateData(FALSE);
	} else if (m_inFocus == IDC_MPG_EDIT_POS) {
		UpdateData(TRUE);
		m_pos=m_to;
		UpdateData(FALSE);
	}
	if (mpeg) {
		mpeg->StopMpegMovie();
		isPlayRange = FALSE;
		PBC.walk = 0;
		PBC.isPC = 0;
	}

	GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CMpegDlg::ExecuteCommNumber(int num)
{
	if (textDoc != NULL && textDoc->FileInfo != NULL) {
		textDoc->FileInfo->MvTr.MBeg = m_from;
		textDoc->FileInfo->MvTr.MEnd = m_to;
		textDoc->ExecuteCommNUM = num;
		textDoc->myUpdateAllViews(FALSE);
	}
}

void CMpegDlg::OnToggleMovieText() 
{
	if (textDoc != NULL && textDoc->FileInfo != NULL) {
		AfxGetApp()->m_pMainWnd->SetActiveWindow();
	}
}

#define VK_SHIFT_USED (GetAsyncKeyState(VK_SHIFT) & 0x8000)
#define VK_CTRL_USED  (GetAsyncKeyState(VK_CONTROL) & 0x8000)

BOOL CMpegDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_CHAR && pMsg->wParam == 17/*^W*/) {
		PlayingContMovie = FALSE;
		if (mpeg) {
			mpeg->StopMpegMovie();
			isPlayRange = FALSE;
			PBC.walk = 0;
			PBC.isPC = 0;
		}
		isPlayRange = FALSE;
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
		return TRUE;
	} else if (pMsg->message == WM_CHAR && pMsg->wParam == 9/*^I*/) {
		if (PlayingContMovie=='\003') {
			if (textDoc != NULL && textDoc->FileInfo != NULL && mpeg) {
				UINT cpos = mpeg->GetCurTime();
				if (cpos)
					textDoc->ChangeCursorInfo = cpos;
				else
					textDoc->ChangeCursorInfo = 1;
				textDoc->myUpdateAllViews(FALSE);
			}
		} else {
			SendMessage(WM_COMMAND, IDC_BUTTON_SAVE, 0);
		}
		return TRUE;
	} else if (pMsg->message == WM_KEYDOWN) {
		if (textDoc != NULL && textDoc->isTranscribing) {
			if ((pMsg->wParam == VK_F5 || pMsg->wParam == VK_F1 || pMsg->wParam == VK_F2 || pMsg->wParam == VK_F3) && F5_Offset != 0L) {
				if (textDoc != NULL && textDoc->FileInfo != NULL) {
					UINT cpos = mpeg->GetCurTime();
					if (cpos)
						textDoc->ChangeCursorInfo = cpos;
					else
						textDoc->ChangeCursorInfo = 1;
					textDoc->myUpdateAllViews(FALSE);
					if (mpeg && mpeg->isPlaying) {
						PlayingContMovie = FALSE;
						mpeg->StopMpegMovie();
						isPlayRange = FALSE;
						PBC.walk = 0;
						PBC.isPC = 0;
					}
				} else {
					PlayingContMovie = FALSE;
					if (mpeg) {
						mpeg->StopMpegMovie();
						isPlayRange = FALSE;
						PBC.walk = 0;
						PBC.isPC = 0;
					}
					isPlayRange = FALSE;
					do_warning("Internal error with movie to text link. Please quit CLAN and try again.", 0);
				}
				return TRUE;
			} else if (pMsg->wParam == VK_F5 && F5_Offset == 0L) {
				if (mpeg && mpeg->isPlaying) {
					PlayingContMovie = FALSE;
					mpeg->StopMpegMovie();
					isPlayRange = FALSE;
					PBC.walk = 0;
					PBC.isPC = 0;
				}
			} else if (pMsg->wParam == VK_SPACE) {
				if (textDoc != NULL && textDoc->FileInfo != NULL) {
					UINT cpos = mpeg->GetCurTime();
					if (cpos)
						textDoc->ChangeCursorInfo = cpos;
					else
						textDoc->ChangeCursorInfo = 1;
					textDoc->myUpdateAllViews(FALSE);
				} else {
					PlayingContMovie = FALSE;
					if (mpeg) {
						mpeg->StopMpegMovie();
						isPlayRange = FALSE;
						PBC.walk = 0;
						PBC.isPC = 0;
					}
					isPlayRange = FALSE;
					do_warning("Internal error with movie to text link. Please quit CLAN and try again.",0);
				}
				return TRUE;
			}
		} else if (pMsg->wParam == VK_RETURN) {
			m_PlayToEnd = FALSE;
			SendMessage(WM_COMMAND, IDC_PLAY_BUTTON, 0);
			return TRUE;
		} else if (pMsg->wParam == VK_SPACE) {
			m_PlayToEnd = TRUE;
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
		} else if (pMsg->wParam == 0xBF && VK_CTRL_USED) {
			OnToggleMovieText();
//			return TRUE;
		} else if (pMsg->wParam == VK_LEFT && VK_SHIFT_USED && VK_CTRL_USED) {
//			int count;
			if (GetFocus( ) != GetDlgItem(IDC_MPG_EDIT_DELAY)) {
				UpdateData(TRUE);
				if (GetTickCount()-tDelay < 400)
					keyDNcount++;
				else
					keyDNcount = 0;
				tDelay = GetTickCount();
//				count = (int)(pMsg->lParam & 0xFFFF);
				if (m_pos > 0)
					m_pos -= ((keyDNcount < 8) ? 1 : 50);
				if (m_pos < 0)
					m_pos = 0;
				UpdateData(FALSE);
				SetCurPos(0);
				return TRUE;
			}
		} else if (pMsg->wParam == VK_RIGHT && VK_SHIFT_USED && VK_CTRL_USED) {
//			int count;
			if (GetFocus( ) != GetDlgItem(IDC_MPG_EDIT_DELAY)) {
				UpdateData(TRUE);
				if (GetTickCount()-tDelay < 400)
					keyDNcount++;
				else
					keyDNcount = 0;
				tDelay = GetTickCount();
//				count = (int)(pMsg->lParam & 0xFFFF);
				m_pos += ((keyDNcount < 8) ? 1 : 50);
				if (m_pos > mpeg->GetMpegMovieDuration())
					m_pos = mpeg->GetMpegMovieDuration();
				UpdateData(FALSE);
				SetCurPos(0);
				return TRUE;
			}
		} else if (pMsg->wParam == VK_UP && VK_SHIFT_USED && VK_CTRL_USED) {
			if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_FROM)) {
				UpdateData(TRUE);
				m_pos = m_from;
				UpdateData(FALSE);
				SetCurPos(0);
				return TRUE;
			} else if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_TO)) {
				UpdateData(TRUE);
				m_pos = m_to;
				UpdateData(FALSE);
				SetCurPos(0);
				return TRUE;
			} else {
				return TRUE;
			}
		} else if (pMsg->wParam == VK_DOWN && VK_SHIFT_USED && VK_CTRL_USED) {
			if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_FROM)) {
				UpdateData(TRUE);
				m_from = m_pos;
				UpdateData(FALSE);
				return TRUE;
			} else if (GetFocus( ) == GetDlgItem(IDC_MPG_EDIT_TO)) {
				UpdateData(TRUE);
				m_to = m_pos;
				UpdateData(FALSE);
				return TRUE;
			} else {
				return TRUE;
			}
		} else if (pMsg->wParam == VK_LEFT && VK_SHIFT_USED) {
			UpdateData(TRUE);
			m_to -= MOVEINCREMENT;
			if (m_to < 0)
				m_to = 0;
			m_pos = m_to;
			UpdateData(FALSE);
			SetCurPos(0);
			ExecuteCommNumber(93);
			return TRUE;
		} else if (pMsg->wParam == VK_LEFT && VK_CTRL_USED) {
			UpdateData(TRUE);
			m_from -= MOVEINCREMENT;
			if (m_from < 0)
				m_from = 0;
			m_pos = m_from;
			UpdateData(FALSE);
			SetCurPos(0);
			ExecuteCommNumber(95);
			return TRUE;
		} else if (pMsg->wParam == VK_RIGHT && VK_SHIFT_USED) {
			UpdateData(TRUE);
			m_to += MOVEINCREMENT;
			if (m_to > mpeg->GetMpegMovieDuration())
				m_to = mpeg->GetMpegMovieDuration();
			m_pos = m_to;
			UpdateData(FALSE);
			SetCurPos(0);
			ExecuteCommNumber(92);
			return TRUE;
		} else if (pMsg->wParam == VK_RIGHT && VK_CTRL_USED) {
			UpdateData(TRUE);
			m_from += MOVEINCREMENT;
			if (m_from > mpeg->GetMpegMovieDuration())
				m_from = mpeg->GetMpegMovieDuration();
			m_pos = m_from;
			UpdateData(FALSE);
			SetCurPos(0);
			ExecuteCommNumber(94);
			return TRUE;
		} else
			return CDialog::PreTranslateMessage(pMsg);
	}

	if (pMsg->message == WM_MOUSEACTIVATE) {
		if (textDoc != NULL && textDoc->FileInfo != NULL) {
			textDoc->SetFakeHilight = 2;
			textDoc->myUpdateAllViews(FALSE);
		}
	}
	if (mpeg != NULL) {
		char res = 0;
		res = mpeg->myWP();
		if (res == 1) {
			SetCurPos(1);
		} else if (res == 2) {
			SetCurPos(2);
		}
		if (mpeg->mcActionType) {
			mpeg->mcActionType = FALSE;
			SetWaveTimeValue(mpeg->GetCurTime(), 0L);
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CMpegDlg::DestroyWindow() 
{
	CRect lpRect;

	MpegDlg = NULL;
	PlayingContMovie = FALSE;
	UpdateData(TRUE);
	GetWindowRect(&lpRect);
	MovieWinSize.top = lpRect.top;
	MovieWinSize.left = lpRect.left;
	MovieWinSize.width = lpRect.right;
	MovieWinSize.height = lpRect.bottom;
	if (*m_rptstr == 'e' || *m_rptstr == 'E') {
		rptMark = 'e';
		rptPTime = uS.atol(m_rptstr.Mid(1));
	} else if (*m_rptstr == 'b' || *m_rptstr == 'B') {
		rptMark = 'b';
		rptPTime = uS.atol(m_rptstr.Mid(1));
	} else if (*m_rptstr == '-') {
		rptMark = '-';
		rptPTime = uS.atol(m_rptstr.Mid(1));
	} else if (*m_rptstr == '+') {
		rptMark = '+';
		rptPTime = uS.atol(m_rptstr.Mid(1));
	} else {
		rptPTime = uS.atol(m_rptstr);
		if (rptPTime > 0)
			rptMark = '+';
		else
			rptMark = 0;
	}
	WriteCedPreference();
	if (mpeg)	{
		mpeg->DestroyWindow() ;
		mpeg=NULL;
	}
#ifdef USETIMER
	if (KillTimer(timer))
		timer = 0;
#endif
	return CDialog::DestroyWindow();
}

void CMpegDlg::OnCancel() {
	DestroyWindow();
}

void CMpegDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	MpegDlg = NULL;
	CDialog::PostNcDestroy();
}

void CMpegDlg::OnPlayButton() 
{
	DWORD	cPos;

	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_MPG_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_MPG_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_MPG_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_MPG_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	if (PBC.enable && PBC.isPC && PBC.walk) {
		if (mpeg && mpeg->isPlaying && mpeg->isPlaying != 3) {
			PlayingContMovie = FALSE;
			mpeg->StopMpegMovie();
			isPlayRange = FALSE;
			PBC.walk = 0;
			PBC.isPC = 0;
		}
		do_warning("To repeat use F7 or F8 command. This buttons stops playback in Walker Controller.", 0);
	} else if (mpeg && mpeg->isPlaying && mpeg->isPlaying != 3) {
		PlayingContMovie = FALSE;
		mpeg->StopMpegMovie();
		isPlayRange = FALSE;
		PBC.walk = 0;
		PBC.isPC = 0;
	} else if (mpeg) {
		if (!mpeg->IsMpegMovieDone()) {
			mpeg->isPlaying = 0;
			mpeg->m_wmpControler->pause();
		}
		Play();
	}

	if (!PlayingContMovie || (PBC.walk && PBC.enable))
		;
	else
		GotoDlgCtrl(GetDlgItem(m_inFocus));
	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   m_PosCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_FROM:  m_FromCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_TO:    m_ToCtrl.SetSel(cPos, FALSE); break;
		case IDC_MPG_EDIT_DELAY: m_DelayCtrl.SetSel(cPos, FALSE); break;
	}
}

void CMpegDlg::OnSaveButton() 
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
			UINT t = mpeg->GetCurTime();
			if (t)
				textDoc->ChangeCursorInfo=t;
			else
				textDoc->ChangeCursorInfo=1;
			textDoc->myUpdateAllViews(FALSE);
		}
	} else if (textDoc != NULL && textDoc->FileInfo != NULL) {
		UpdateData(TRUE);
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

BOOL CMpegDlg::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	return 1;// CDialog::OnChildNotify(message, wParam, lParam, pLResult);
}

#ifdef USETIMER
void CMpegDlg::OnTimer(UINT nIDEvent)
{
/*
	double pos;
	if ((pos=m_player.GetCurrentPosition())>-1){
		pos*=1000.;
		if (playing)
			m_pos=pos;
		else {
			UpdateData();
		}
		UpdateData(FALSE);
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
*/
	CDialog::OnTimer(nIDEvent);
}
#endif

BOOL CMpegDlg::ShowToolTip( UINT id, NMHDR * pTTTStruct, LRESULT * pResult )
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

void CMpegDlg::SetCurPos(char passive) {
	BOOL justStoped = FALSE;
	UINT pos = 0;
//	CDC* tGlobalDC;

	if (passive) {
		pos = mpeg->GetCurTime();
		if (m_pos != pos || passive == 2) {
//			UpdateData(TRUE);
			last_m_pos = m_pos;
			m_pos = pos;
			if (isPlayRange && mpeg->isPlaying) {
				if (last_m_pos == m_pos)
					same_m_posValue++;
				else
					same_m_posValue = 0;
			} else
				same_m_posValue = 0;
			if (PlayingContMovie && PlayingContMovie != '\003' && m_pos < real_to) {
				if (textDoc != NULL && textDoc->FileInfo != NULL) {
					if (m_pos > textDoc->FileInfo->MvTr.MEnd) {
						textDoc->ChangeCursorInfo = m_pos;
						textDoc->myUpdateAllViews(FALSE);
					}
				}
			}
			if (!mpeg->isPlaying && PBC.enable && !PBC.walk) {
				textDoc->FileInfo->MvTr.MBeg = m_pos;
				PBC.cur_pos = m_pos;
			}
			if (passive == 2) {
				m_pos = pos;
			}
			if ((m_pos >= real_to - 1 || (m_pos >= real_to - 10 && same_m_posValue > 6)) && isPlayRange && mpeg->isPlaying) {
				m_pos = real_to;
				if (!PlayingContMovie || PlayingContMovie == '\003' || PlayingContMovie == '\002') {
					if (mpeg != NULL) {
						mpeg->StopMpegMovie();
					}
					isPlayRange = FALSE;
					PlayingContMovie = FALSE;
					justStoped = TRUE;
				}
				if (textDoc && textDoc->FileInfo != NULL && (justStoped || mpeg->IsMpegMovieDone())) {
					mpeg->isPlaying = 0;
					if (textDoc->FileInfo->cpMvTr != NULL) {
						isPlayRange = FALSE;
						PlayingContMovie = FALSE;
						textDoc->RestorePlayMovieInfo = TRUE;
						textDoc->myUpdateAllViews(FALSE);
					} else if (PBC.enable && PBC.isPC) {
						isPlayRange = FALSE;
						PlayingContMovie = FALSE;
						textDoc->DoWalkerController = TRUE;
						textDoc->myUpdateAllViews(FALSE);
					}
				}
			} else if (passive != 2 && !isPlayRange && !mpeg->isPlaying && PBC.enable) {
				if (textDoc != NULL && textDoc->FileInfo != NULL)
					textDoc->FileInfo->MvTr.MBeg = pos;
			} else if (mpeg->IsMpegMovieDone() && same_m_posValue > 6 && isPlayRange && mpeg->isPlaying) {
				m_pos = real_to;
				if (!PlayingContMovie || PlayingContMovie == '\003' || PlayingContMovie == '\002') {
					if (mpeg != NULL) {
						mpeg->StopMpegMovie();
					}
					isPlayRange = FALSE;
					PlayingContMovie = FALSE;
					justStoped = TRUE;
//					strcpy(global_df->err_message, DASHES);
//					tGlobalDC = GlobalDC;
//					GlobalDC = textDC;
//					draw_mid_wm();
//					GlobalDC = tGlobalDC;
				}
			}
			UpdateData(FALSE);
		}
	} else {
		UpdateData(TRUE);
		if (mpeg) {
			if (!mpeg->isPlaying && PBC.enable && !PBC.walk) {
				textDoc->FileInfo->MvTr.MBeg = m_pos;
				PBC.cur_pos = m_pos;
			}
			if (mpeg != NULL) {
				mpeg->StopMpegMovie();
			}
			isPlayRange = FALSE;
//lxsc		MoviesTask(mpeg->theMovie, 0);
			mpeg->FakeMove(m_pos);
			PBC.walk = 0;
			PBC.isPC = 0;
		}
		UpdateData(FALSE);
	}
}

LRESULT CMpegDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	return CDialog::WindowProc(message, wParam, lParam);
}

void CMpegDlg::OnMpgHelp() {
	DWORD cPos;

	switch(m_inFocus) {
		case IDC_MPG_EDIT_POS:   cPos = m_PosCtrl.GetSel(); break;
		case IDC_MPG_EDIT_FROM:  cPos = m_FromCtrl.GetSel(); break;
		case IDC_MPG_EDIT_TO:    cPos = m_ToCtrl.GetSel(); break;
		case IDC_MPG_EDIT_DELAY: cPos = m_DelayCtrl.GetSel(); break;
		default: m_inFocus=IDC_MPG_EDIT_FROM; cPos = m_FromCtrl.GetSel(); break;
	}

	if (mpeg) {
		PlayingContMovie = FALSE;
		mpeg->StopMpegMovie();
		isPlayRange = FALSE;
		PBC.walk = 0;
		PBC.isPC = 0;
	}
	isPlayRange = FALSE;
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

void CMpegDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (mpeg) {
		PlayingContMovie = FALSE;
		mpeg->StopMpegMovie();
		isPlayRange = FALSE;
		PBC.walk = 0;
		PBC.isPC = 0;
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CMpegDlg::OnUpdateEditPos() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	// TODO: Add your control notification handler code here
	if (mpeg) {
		UpdateData(TRUE);
		SetWaveTimeValue(m_pos, 0L);
		SetCurPos(0);
	}
}

void CMpegDlg::OnUpdateEditFrom()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	if (mpeg) {
		UpdateData(TRUE);
		SetWaveTimeValue(m_from, m_to);
	}
}

void CMpegDlg::OnUpdateEditTo()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	if (mpeg) {
		UpdateData(TRUE);
		SetWaveTimeValue(m_from, m_to);
	}
}

void CMpegDlg::OnSetfocusMpgEditPos() 
{
	SetDlgItemText(IDC_BEG_BUTTON, _T("^"));
	SetDlgItemText(IDC_END_BUTTON, _T("^"));
	m_inFocus = IDC_MPG_EDIT_POS;
}

void CMpegDlg::OnSetfocusMpgEditTo() 
{
	SetDlgItemText(IDC_BEG_BUTTON, _T("v"));
	SetDlgItemText(IDC_END_BUTTON, _T("v"));
	m_inFocus = IDC_MPG_EDIT_TO;	
}

void CMpegDlg::OnSetfocusMpgEditFrom() 
{
	SetDlgItemText(IDC_BEG_BUTTON, _T("v"));
	SetDlgItemText(IDC_END_BUTTON, _T("v"));
	m_inFocus = IDC_MPG_EDIT_FROM;		
}

void CMpegDlg::OnSetfocusMpgEditDelay() 
{
	SetDlgItemText(IDC_BEG_BUTTON, _T("-"));
	SetDlgItemText(IDC_END_BUTTON, _T("-"));
	m_inFocus = IDC_MPG_EDIT_DELAY;
}

void CMpegDlg::SetWaveTimeValue(UINT timeBeg, UINT timeEnd) {
	long t;
	myFInfo *saveGlobal_df;

	if (textDoc != NULL && textDoc->FileInfo != NULL) {
		saveGlobal_df = global_df;
		global_df = textDoc->FileInfo;
		if (syncAudioAndVideo(global_df->MvTr.MovieFile)) {
//			DrawSoundCursor(0);
			t = AlignMultibyteMediaStream(conv_from_msec_rep(timeBeg), '+');
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

void CMpegDlg::SetCurPosToValue(long tTime){
	if (mpeg) {
		mpeg->StopMpegMovie();
		isPlayRange = FALSE;
		m_pos = tTime;
//lxsc	MoviesTask(mpeg->theMovie, 0);
		mpeg->FakeMove(m_pos);
		PBC.walk = 0;
		PBC.isPC = 0;
	}
	UpdateData(FALSE);
}
