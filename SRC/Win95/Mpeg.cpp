// MPEG.cpp : implementation file
//

#include "ced.h"
#include "MMedia.h"
#include "MPeg.h"
#include "MpegDlg.h"
#include "MpegEventDispatch.h"

/////////////////////////////////////////////////////////////////////////////
// CMpeg dialog

IMPLEMENT_DYNAMIC(CMpeg, CDialog)

CMpeg::CMpeg(CString pathName, CWnd* pParent /*=NULL*/)
	: CDialog(CMpeg::IDD, pParent)
{
	m_wmpPlayer = NULL;
	m_wmpControler = NULL;
	m_spConnectionPoint = NULL;

	isMovieDurationSet = FALSE;
	havemovie = FALSE;
	mcActionType = FALSE;
	isPlaying = 0;
	strcpy(filename, pathName);
	curTime = startTime = backTime = 0.0;
	duration = movieDuration = 0.0;
}

CMpeg::~CMpeg()
{
}

void CMpeg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMpeg, CDialog)
	//{{AFX_MSG_MAP(CQT)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define MAX_STRING  1024
static BOOL FAILMSG(HRESULT hr) {
	if (hr == S_OK)
		return FALSE;
	else
		return TRUE;
}

BOOL CMpeg::OnInitDialog() {
	CDialog::OnInitDialog();

	if (!AtlAxWinInit()) {
		return 0;
	}
	Open();
	MoveWindow(0, 0, MpegDlg->MvBounds.right-MpegDlg->MvBounds.left, MpegDlg->MvBounds.bottom-MpegDlg->MvBounds.top, TRUE);
	return TRUE;
}

BOOL CMpeg::Open() {
	IWMPSettings	*m_wmpSettings;
	CComPtr<IAxWinHostWindow>           spHost;
	CComPtr<IConnectionPointContainer>  spConnectionContainer;
	CComWMPEventDispatch                *pEventListener = NULL;
	CComPtr<IWMPEvents>                 spEventListener;
	HRESULT	hr;
	char	cStr[FNSize];
	unCH	wStr[FNSize];

	Close();
	if (!havemovie) {
		// load OCX in window
		startTime = 0.0;
		curTime = 0.0;
		duration = 0.0;
		movieDuration = duration;
		isPlaying = 0;
		m_wndView.Create(m_hWnd, MpegDlg->MvBounds, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
		if (NULL == m_wndView.m_hWnd)
			goto FAILURE;
		hr = m_wndView.QueryHost(&spHost);
		if (FAILMSG(hr))
			goto FAILURE;
		hr = spHost->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), m_wndView, 0);
		if (FAILMSG(hr))
			goto FAILURE;
		hr = m_wndView.QueryControl(&m_wmpPlayer);
		if (FAILMSG(hr))
			goto FAILURE;
		hr = m_wmpPlayer->get_settings(&m_wmpSettings);
		if (FAILMSG(hr))
			goto FAILURE;
		hr = m_wmpSettings->put_autoStart(FALSE);
		if (FAILMSG(hr))
			goto FAILURE;
		setVolume(0);
		// start listening to events
		hr = CComWMPEventDispatch::CreateInstance(&pEventListener);
		spEventListener = pEventListener;
		if (FAILMSG(hr))
			goto FAILURE;
		hr = m_wmpPlayer->QueryInterface(&spConnectionContainer);
		if (FAILMSG(hr))
			goto FAILURE;
		// See if OCX supports the IWMPEvents interface
		m_spConnectionPoint = NULL;
		hr = spConnectionContainer->FindConnectionPoint(__uuidof(IWMPEvents), &m_spConnectionPoint);
		if (FAILED(hr)) {
			// If not, try the _WMPOCXEvents interface, which will use IDispatch
			hr = spConnectionContainer->FindConnectionPoint(__uuidof(_WMPOCXEvents), &m_spConnectionPoint);
			if (FAILMSG(hr))
				goto FAILURE;
		}
		m_dwAdviseCookie = 0;
		hr = m_spConnectionPoint->Advise(spEventListener, &m_dwAdviseCookie);
		if (FAILMSG(hr))
			goto FAILURE;
		hr = m_wmpPlayer->put_URL(filename);
		if (FAILMSG(hr))
			goto FAILURE;
		hr = m_wmpPlayer->get_controls(&m_wmpControler);
		if (FAILMSG(hr))
			goto FAILURE;
		havemovie = TRUE;
	}
	return TRUE;
FAILURE:
	strcpy(cStr, "Error opening movie file: \"");
	u_strcpy(wStr, cStr, FNSize);
	strcat(wStr, filename);
	strcat(wStr, "\" \"");
	MessageBox(wStr, _T(""), MB_OK);
	return FALSE;
}

// CMpeg message handlers

static long conv_to_msec_wmp(double num) {
	long t;

	num = num * 1000.0000;
	t = (long)num;
	if (num > 0.0 && t < 0L)
		t = 0x7fffffff;
	return(t);
}

static double conv_from_msec_wmp(long num) {
	double t;

	t = (double)num;
	t = t / 1000.0000;
	if (num > 0 && t < 0L)
		t = 0x7fffffff;
	return(t);
}

void CMpeg::setVolume(long vol) {
	IWMPSettings	*m_wmpSettings;

	m_wmpPlayer->get_settings(&m_wmpSettings);
	if (m_wmpSettings) {
		m_wmpSettings->put_volume(vol);
	}
}
BOOL CMpeg::IsMpegMovieDone(void) {
	if (isPlaying <= 0)
		return TRUE;
	else
		return FALSE;
}

void CMpeg::SetStartTime(long start) {
	startTime = conv_from_msec_wmp(start);
}

long CMpeg::GetStartTime(void) {
	return conv_to_msec_wmp(startTime);
}

void CMpeg::SetDuration(long howlong) {
	duration = conv_from_msec_wmp(howlong);
	if (duration <= 0.0 && howlong > 0)
		duration = (double)howlong;
}

long CMpeg::GetDuration(void) {
	return conv_to_msec_wmp(duration);
}

void CMpeg::SetRawDuration(double dur) {
	duration = dur;
	movieDuration = dur;
}

long CMpeg::GetMpegMovieDuration() {
	return conv_to_msec_wmp(movieDuration);
}

long CMpeg::GetCurTime(void) {
	double num;
	HRESULT	hr;

	if (havemovie) {
		hr = m_wmpControler->get_currentPosition(&num);
		if (!FAILMSG(hr)) {
			return conv_to_msec_wmp(num);
		} else
			return 0;
	} else
		return 0;
}

void CMpeg::FakeMove(long start) {
	HRESULT	hr;
	double dStart;

	if (m_wmpControler != NULL) {
		dStart = conv_from_msec_wmp(0);
		hr = m_wmpControler->put_currentPosition(dStart);
		if (FAILMSG(hr)) {
		}
	}
}

char CMpeg::myWP(void) {
	HRESULT	hr;
	double num;
	LRESULT res = 0;

	if (havemovie) {
		hr = m_wmpControler->get_currentPosition(&num);
		if (!FAILMSG(hr)) {
			curTime = num;
			if (IsMpegMovieDone() && isPlaying == 1) {
				return(2);
			} else if (backTime != curTime && isPlaying == 1) {
				return(1);
				backTime = curTime;
			}
		}
	}
	return(0);
}

void CMpeg::Play(void) {
	HRESULT	hr;
	IWMPSettings	*m_wmpSettings;

	if (havemovie && m_wmpControler != NULL) {
		if (PBC.speed != 100) {
			double speed;

			hr = m_wmpPlayer->get_settings(&m_wmpSettings);
			if (FAILMSG(hr)) {
			}
			if (PBC.speed > 0 && PBC.speed < 100) {
				speed = (1.0000 * (double)PBC.speed) / 100.0000;
				hr = m_wmpSettings->put_rate(speed);
				if (FAILMSG(hr)) {
				}
			} else if (PBC.speed > 100) {
				speed = (double)PBC.speed / 100.0000;
				hr = m_wmpSettings->put_rate(speed);
				if (FAILMSG(hr)) {
				}
			} else {
				speed = 1.0;
				hr = m_wmpSettings->put_rate(speed);
				if (FAILMSG(hr)) {
				}
			}
		}
		if (isMovieDurationSet == TRUE) {
			hr = m_wmpControler->put_currentPosition(startTime);
			if (FAILMSG(hr)) {
			}
		}
		m_wmpControler->play();
	}
}

void CMpeg::StopMpegMovie(void) {
	HRESULT	hr;

	isPlaying = 0;
	if (m_wmpControler != NULL) {
		hr = m_wmpControler->pause();
		if (FAILMSG(hr)) {
		}
	}
}

void CMpeg::Close(void) {
	HRESULT	hr;

	if (m_spConnectionPoint != NULL) {
		if (m_dwAdviseCookie != 0)
			m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
		m_spConnectionPoint->Release();
		m_spConnectionPoint = NULL;
	}

	if (havemovie) {
		havemovie = FALSE;
		StopMpegMovie();
		hr = m_wmpPlayer->close();
		hr = m_wmpPlayer->Release();
		hr = m_wmpControler->Release();
	}
	isMovieDurationSet = FALSE;
	m_wmpPlayer = NULL;
	m_wmpControler = NULL;
}

BOOL CMpeg::DestroyWindow() {
	// TODO: Add your specialized code here and/or call the base class

	myFInfo *saveGlobal_df;

	if (((CMpegDlg*)GetParent())->textDoc && (((CMpegDlg*)GetParent())->textDoc)->FileInfo) {
		saveGlobal_df = global_df;
		global_df = (((CMpegDlg*)GetParent())->textDoc)->FileInfo;
		if (global_df->cpMvTr != NULL) {
//			GlobalDC = ((CMpegDlg*)GetParent())->textDC;
			GlobalDoc = ((CMpegDlg*)GetParent())->textDoc;
			if (((CMpegDlg*)GetParent())->doErase)
				CopyAndFreeMovieData(TRUE);
		}
		global_df = saveGlobal_df;
	}
	Close();
	return CDialog::DestroyWindow();
}

void CMpeg::PostNcDestroy() {
	// TODO: Add your specialized code here and/or call the base class

	delete this;
	CDialog::PostNcDestroy();
}

BOOL CMpeg::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) {
	// TODO: Add your specialized code here and/or call the base class

//	EventProc(message);
	return 1;//CDialog::OnChildNotify(message, wParam, lParam, pLResult);
}


void CMpeg::OnLButtonDown(UINT nFlags, CPoint point) {
	// TODO: Add your message handler code here and/or call default
	CDialog::OnLButtonDown(nFlags, point);
}

BOOL CMpeg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) {
	// TODO: Add your specialized code here and/or call the base class
//	EventProc(WM_NOTIFY);

	return 1;//CDialog::OnNotify(wParam, lParam, pResult);
}
