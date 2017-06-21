#include "ced.h"
#include "CedDlgs.h"
#include "MMedia.h"


#ifdef __MWERKS__
#include <unistd.h>
#else
#include "msutil.h"
#endif

CPlaybackControler *PBCDlg;
extern struct DefWin PBCWinSize;

struct PBCs PBC = {0, 0, 100, 0};
static myFInfo *PBCglobal_df = NULL;

/////////////////////////////////////////////////////////////////////////////
// CPlaybackControler dialog


CPlaybackControler::CPlaybackControler(CWnd* pParent /*=NULL*/)
	: CDialog(CPlaybackControler::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlaybackControler)
	m_loop = _T("");
	m_shift = _T("");
	m_speed = _T("");
	m_seglen = _T("");
	m_CursorPos = _T("");
	m_PauseLen = _T("");
	m_TotalLen = _T("");
	//}}AFX_DATA_INIT
	mySetSlider = false;
	LPCTSTR lpszTemplateName = MAKEINTRESOURCE(IDD);
	ASSERT(HIWORD(lpszTemplateName) == 0 ||
		AfxIsValidString(lpszTemplateName));
	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (HIWORD(m_lpszTemplateName) == 0 && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD)m_lpszTemplateName);
	HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	ASSERT(hTemplate != NULL);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
	CreateIndirect(lpDialogTemplate,pParent,NULL,NULL/*hInst*/);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);
}

BOOL CPlaybackControler::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (PBCWinSize.top || PBCWinSize.width || PBCWinSize.height || PBCWinSize.left) {
		CRect lpRect;
		long width, hight;

		this->GetWindowRect(&lpRect);
		width = lpRect.right - lpRect.left;
		hight = lpRect.bottom - lpRect.top;
		lpRect.top = PBCWinSize.top;
		lpRect.bottom = lpRect.top + hight;
		lpRect.left = PBCWinSize.left;
		lpRect.right = lpRect.left + width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}

	PBC.enable = true;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlaybackControler::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaybackControler)
	DDX_Control(pDX, IDC_PLAYBACK_TOTAL_LEN, m_TotalLenControl);
	DDX_Control(pDX, IDC_PLAYBACK_CURSOR_POS, m_CursorPosControl);
	DDX_Control(pDX, IDC_PLAYBACK_SLIDER, m_SliderControl);
	DDX_Text(pDX, IDC_PLAYBACK_LOOP_NUMBER, m_loop);
	DDX_Text(pDX, IDC_PLAYBACK_SHIFT_RATE, m_shift);
	DDX_Text(pDX, IDC_PLAYBACK_SPEED, m_speed);
	DDX_Text(pDX, IDC_SEGMENT_LENGTH, m_seglen);
	DDX_Text(pDX, IDC_PLAYBACK_CURSOR_POS, m_CursorPos);
	DDX_Text(pDX, IDC_PLAYBACK_PAUSE_LEN, m_PauseLen);
	DDX_Text(pDX, IDC_PLAYBACK_TOTAL_LEN, m_TotalLen);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlaybackControler, CDialog)
	//{{AFX_MSG_MAP(CPlaybackControler)
	ON_EN_CHANGE(IDC_PLAYBACK_SHIFT_RATE, OnChangePlaybackShiftRate)
	ON_EN_CHANGE(IDC_PLAYBACK_SPEED, OnChangePlaybackSpeed)
	ON_EN_CHANGE(IDC_SEGMENT_LENGTH, OnChangeSegmentLength)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_PLAYBACK_LOOP_NUMBER, OnChangePlaybackLoopNumber)
	ON_EN_CHANGE(IDC_PLAYBACK_CURSOR_POS, OnChangePlaybackCursorPos)
	ON_EN_CHANGE(IDC_PLAYBACK_PAUSE_LEN, OnChangePlaybackPauseLen)
	ON_WM_PAINT()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PLAYBACK_SLIDER, OnCustomdrawPlaybackSlider)
	ON_BN_CLICKED(IDC_PLAYBACK_OPENMEDIA, OnPlaybackOpenmedia)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaybackControler message handlers

void CPlaybackControler::OnCancel() {
	CRect lpRect;

	UpdateData(TRUE);
	PBC.enable = false;
	PBC.LoopCnt = atoi(m_loop);
	if (PBC.LoopCnt < 1)
		PBC.LoopCnt = 1;
	PBC.backspace = atol(m_shift);
	if (PBC.backspace < 0L)
		PBC.backspace = -PBC.backspace;
	PBC.speed = atol(m_speed);
	if (PBC.speed < 0L)
		PBC.speed = 100L;
	PBC.step_length = atol(m_seglen);
	if (PBC.step_length < 0L)
		PBC.step_length = 0L;
	PBC.pause_len = atoi(m_PauseLen);
	if (PBC.pause_len < 0)
		PBC.pause_len = 0;
	PBC.cur_pos = atoi(m_CursorPos);
	if (PBC.cur_pos < 0)
		PBC.cur_pos = 0;
	if (PBC.cur_pos > PBC.total_len)
		PBC.cur_pos = PBC.total_len;

	this->GetWindowRect(&lpRect);
	PBCWinSize.top = lpRect.top;
	PBCWinSize.left = lpRect.left;
	PBCWinSize.width = lpRect.right;
	PBCWinSize.height = lpRect.bottom;
	DestroyWindow();
	if (PBCDlg != NULL) {
		delete PBCDlg;
		PBCDlg = NULL;
	}
	WriteCedPreference();
	PBCglobal_df = NULL;
}


void CPlaybackControler::OnDestroy() 
{
	CRect lpRect;

	UpdateData(TRUE);
	PBC.enable = false;
	PBC.LoopCnt = atoi(m_loop);
	if (PBC.LoopCnt < 1)
		PBC.LoopCnt = 1;
	PBC.backspace = atol(m_shift);
	if (PBC.backspace < 0L)
		PBC.backspace = -PBC.backspace;
	PBC.speed = atol(m_speed);
	if (PBC.speed < 0L)
		PBC.speed = 100L;
	PBC.step_length = atol(m_seglen);
	if (PBC.step_length < 0L)
		PBC.step_length = 0L;
	PBC.pause_len = atoi(m_PauseLen);
	if (PBC.pause_len < 0)
		PBC.pause_len = 0;
	PBC.cur_pos = atoi(m_CursorPos);
	if (PBC.cur_pos < 0)
		PBC.cur_pos = 0;
	if (PBC.cur_pos > PBC.total_len)
		PBC.cur_pos = PBC.total_len;

	this->GetWindowRect(&lpRect);
	PBCWinSize.top = lpRect.top;
	PBCWinSize.left = lpRect.left;
	PBCWinSize.width = lpRect.right;
	PBCWinSize.height = lpRect.bottom;
	WriteCedPreference();
	CDialog::OnDestroy();
	if (PBCDlg != NULL) {
		delete PBCDlg;
		PBCDlg = NULL;
	}
	PBCglobal_df = NULL;
}

void CPlaybackControler::OnChangePlaybackShiftRate() 
{
	UpdateData(TRUE);
	PBC.backspace = atol(m_shift);
	if (PBC.backspace < 0L)
		PBC.backspace = -PBC.backspace;
}

void CPlaybackControler::OnChangePlaybackSpeed() 
{
	UpdateData(TRUE);
	PBC.speed = atol(m_speed);
	if (PBC.speed < 0L)
		PBC.speed = 100L;
}

void CPlaybackControler::OnChangeSegmentLength() 
{
	UpdateData(TRUE);
	PBC.step_length = atol(m_seglen);
	if (PBC.step_length < 0L)
		PBC.step_length = 0L;
}

void CPlaybackControler::OnChangePlaybackLoopNumber() 
{
	UpdateData(TRUE);
	PBC.LoopCnt = atoi(m_loop);
	if (PBC.LoopCnt < 1)
		PBC.LoopCnt = 1;
}

void CPlaybackControler::OnChangePlaybackCursorPos() 
{
	UpdateData(TRUE);
	PBC.cur_pos = atoi(m_CursorPos);
	if (PBC.cur_pos < 0)
		PBC.cur_pos = 0;
	if (PBC.cur_pos > PBC.total_len)
		PBC.cur_pos = PBC.total_len;

	if (PBCglobal_df != NULL && !mySetSlider) {
		myFInfo	*tGlobal_df;

		tGlobal_df = global_df;
		global_df = PBCglobal_df;
		PBCglobal_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(PBC.cur_pos),'+');
		global_df = tGlobal_df;
	}
	UpdateDialogWindow();
}

void CPlaybackControler::OnChangePlaybackPauseLen() 
{
	UpdateData(TRUE);
	PBC.pause_len = atoi(m_PauseLen);
	if (PBC.pause_len < 0)
		PBC.pause_len = 0;
}

void CPlaybackControler::OnPlaybackOpenmedia() 
{
	char ret;

	if (PBCglobal_df != NULL) {
		if ((ret=GetNewMediaFile(FALSE, 3))) {
			if (ret == isAudio) { /* sound */
			} else if (ret == isVideo) { /* movie */
				if (global_df->SnTr.SoundFile[0] != EOS) {
					global_df->SnTr.SoundFile[0] = EOS;
					if (global_df->SnTr.isMP3 == TRUE) {
						global_df->SnTr.isMP3 = FALSE;
						if (global_df->SnTr.mp3.hSys7SoundData)
							DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
						global_df->SnTr.mp3.theSoundMedia = NULL;
						global_df->SnTr.mp3.hSys7SoundData = NULL;
					} else {
						if (global_df->SnTr.SoundFPtr != NULL)
							fclose(global_df->SnTr.SoundFPtr);
						global_df->SnTr.SoundFPtr = NULL;
					}
					if (global_df->SoundWin)
						DisposeOfSoundWin();
				}
				if (!findMediaTiers()) {
					sprintf(global_df->err_message, "+Please add \"%s\" tier with media file name to headers section at the top of the file.", MEDIAHEADER);
				}
			} else {
				strcpy(global_df->err_message, "+Unsupported media type.");
			}
		}
		SetPBCglobal_df(false, 0L);
		if (ret == isVideo) { // movie
			if (PBCglobal_df != NULL) {
				PlayMovie(&PBCglobal_df->MvTr, PBCglobal_df, TRUE);
			}
		}
	}
}

void SetPBCglobal_df(char isReset, long cur_pos) {
	if (isReset) {
		if (PBCglobal_df == global_df) {
			PBC.cur_pos = 0L;
			PBC.total_len = 0L;
			PBCglobal_df = NULL;
		}
	} else {
		PBCglobal_df = global_df;
		if (PBCglobal_df != NULL) {
			if (cur_pos == 0L)
				PBC.cur_pos = conv_to_msec_rep(PBCglobal_df->SnTr.BegF);
			else
				PBC.cur_pos = conv_to_msec_rep(cur_pos);
			PBC.total_len = conv_to_msec_rep(PBCglobal_df->SnTr.SoundFileSize);
		} else {
			PBC.cur_pos = 0L;
			PBC.total_len = 0L;
		}
	}
	if (PBCDlg != NULL) {
		PBCDlg->mySetSlider = true;
		sprintf(templineC3, "%ld", PBC.cur_pos);
		PBCDlg->m_CursorPos = cl_T(templineC3);
		PBCDlg->m_CursorPosControl.SetSel(0, -1, FALSE);
		PBCDlg->m_CursorPosControl.ReplaceSel(PBCDlg->m_CursorPos, FALSE);
		sprintf(templineC3, "%ld", PBC.total_len);
		PBCDlg->m_TotalLen = cl_T(templineC3);
		PBCDlg->m_TotalLenControl.SetSel(0, -1, FALSE);
		PBCDlg->m_TotalLenControl.ReplaceSel(PBCDlg->m_TotalLen, FALSE);
		PBCDlg->mySetSlider = false;
		PBCDlg->UpdateDialogWindow();
	}
}

void CPlaybackControler::UpdateDialogWindow() {
	sprintf(templineC3, "%d", PBC.LoopCnt);
	m_loop = cl_T(templineC3);
	sprintf(templineC3, "%ld", PBC.backspace);
	m_shift = cl_T(templineC3);
	sprintf(templineC3, "%ld", PBC.speed);
	m_speed = cl_T(templineC3);
	sprintf(templineC3, "%ld", PBC.step_length);
	m_seglen = cl_T(templineC3);
	sprintf(templineC3, "%ld", PBC.pause_len);
	m_PauseLen = cl_T(templineC3);
	sprintf(templineC3, "%ld", PBC.cur_pos);
	m_CursorPos = cl_T(templineC3);
/*
	if (PBCDlg != NULL) {
		m_CursorPosControl.SetSel(0, -1, FALSE);
		m_CursorPosControl.ReplaceSel(templineC3, FALSE);
	}
*/
	sprintf(templineC3, "%ld", PBC.total_len);
	m_TotalLen = cl_T(templineC3);
	m_SliderControl.SetRangeMin(0, FALSE);
	m_SliderControl.SetRangeMax(100, FALSE);
	UpdateData(FALSE);
	if (!mySetSlider) {
		mySetSlider = true;
		if (PBC.total_len == 0L) {
			m_SliderControl.SetRangeMax(0, TRUE);
			m_SliderControl.SetPos(0);
		} else {
			short tVal;
			double max;

			m_SliderControl.SetRangeMax(100, TRUE);
			max = 100.0000 / (double)PBC.total_len;
			max = (double)PBC.cur_pos * max;
			tVal = (short)roundUp(max);
			m_SliderControl.SetPos(tVal);
		}
		mySetSlider = false;
	}
}

void CPlaybackControler::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	UpdateDialogWindow();
}

/*
	double	val;

	val = (double)m_SliderControl.GetPos();
	val = (double)PBC.total_len * (val / 100.0000);
	PBC.cur_pos = (long)val;
	if (PBC.cur_pos < 0L)
		PBC.cur_pos = -PBC.cur_pos;
	UpdateWindow();
*/

void CPlaybackControler::OnCustomdrawPlaybackSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	double	val;

	if (!mySetSlider) {
		mySetSlider = true;
		val = (double)m_SliderControl.GetPos();
		val = (double)PBC.total_len * (val / 100.0000);
		PBC.cur_pos = (long)val;
		if (PBC.cur_pos < 0L)
			PBC.cur_pos = -PBC.cur_pos;
		if (PBCglobal_df != NULL) {
			myFInfo	*tGlobal_df;

			tGlobal_df = global_df;
			global_df = PBCglobal_df;
			PBCglobal_df->SnTr.BegF = AlignMultibyteMediaStream(conv_from_msec_rep(PBC.cur_pos),'+');
			global_df = tGlobal_df;
		}
		sprintf(templineC3, "%ld", PBC.cur_pos);
		m_CursorPos = cl_T(templineC3);
		m_CursorPosControl.SetSel(0, -1, FALSE);
		m_CursorPosControl.ReplaceSel(m_CursorPos, FALSE);
		sprintf(templineC3, "%ld", PBC.total_len);
		m_TotalLen = cl_T(templineC3);
		m_TotalLenControl.SetSel(0, -1, FALSE);
		m_TotalLenControl.ReplaceSel(m_TotalLen, FALSE);
		mySetSlider = false;
	}

	*pResult = 0;
}
