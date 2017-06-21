// QT.cpp : implementation file
//
#include "ced.h"
#include "MMedia.h"
#include "QT.h"
#include "QTDlg.h"
#include <QTSMovie.h>
#include <fcntl.h>
#include <io.h>
#include "FixMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FloatToFixed(a) ((Fixed)((float)(a) * fixed1))

static Boolean controlChangingBounds = false;
static FNType cFileName[FNSize];

/////////////////////////////////////////////////////////////////////////////
// CQT dialog

CQT::CQT(CString pathName,CWnd* pParent /*=NULL*/,BOOL SetOrg)
	: CDialog(CQT::IDD, pParent) 
{
	//{{AFX_DATA_INIT(CQT)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	cgp = NULL;
	havemovie=havemc=false;
	mcActionType = false;
	isPlaying = 0;
	isFirstLoad = 0;
	filename=LPCTSTR(pathName);
	curTime=startTime=duration=backTime=movieDuration=0;
	isSetOrg = SetOrg;
}

void CQT::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQT)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQT, CDialog)
	//{{AFX_MSG_MAP(CQT)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQT message handlers
BOOL CQT::Open(){
	OSType   myDataRefType;
	Handle   myDataRef = NULL;
	short    myResID = 0;
	CFStringRef	theStr = NULL;
	OSErr		err = noErr;

	Close();
	if (!havemovie){
		// create the data reference
		theStr = CFStringCreateWithCharacters(kCFAllocatorDefault, (UniChar *)filename, strlen(filename));
		err = QTNewDataReferenceFromFullPathCFString(theStr,
						kQTNativeDefaultPathStyle, 0, &myDataRef, &myDataRefType);
		CFRelease(theStr);
		if (err != noErr) {
 			MessageBox (_T("Error opening movie file"),   _T(""), MB_OK);
			return FALSE;
		} 
		// get the Movie
		err = NewMovieFromDataRef(&theMovie, newMovieActive, &myResID, myDataRef, myDataRefType);
		if (err != noErr) {
 			MessageBox (_T("Error opening movie file"),   _T(""), MB_OK);
			return FALSE;
		} 
		// dispose the data reference handle - we no longer need it
		DisposeHandle(myDataRef);
		havemovie=true;
		if (isSetOrg) {
			GetMovieBox(theMovie,&orgMvBounds);
			MvBounds.top = orgMvBounds.top;
			MvBounds.bottom = orgMvBounds.bottom;
			MvBounds.left = orgMvBounds.left;
			MvBounds.right = orgMvBounds.right;
		} else
			SetMovieBox(theMovie, &MvBounds);
		timescale=GetMovieTimeScale(theMovie);
//		startTime=curTime=GetMovieTime(theMovie,NULL);
		startTime=curTime=0;
		duration=GetMovieDuration(theMovie);
		movieDuration = duration;
	}
	
//	GoToBeginningOfMovie(theMovie);
//	LoadMovieIntoRam(theMovie, 0L/*GetMovieTime(Movie->MvMovie, 0L)*/, GetMovieDuration(theMovie), unkeepInRam);
//	UpdateMovie(theMovie);
	
	return TRUE;
}

static long conv_to_msec_mov(double num, double ts) {	
	long t;

	num /= ts;
	num *= 1000.0000;
	t = (long)num;
	if (num > 0.0 && t < 0L)
		t = 0x7fffffff;
	return(t);
}

static long conv_from_msec_mov(double num, double ts) {
	long t;

	num /= 1000.0000;
	num *= ts;
	t = (long)num;
	if (num > 0.0 && t < 0L)
		t = 0x7fffffff;
	return(t);
}

static Boolean myTestAndSet(Boolean *p) {
	Boolean val;
	
	val = *p;
	*p = true;
	return(val);
}

static Boolean myTestAndClear(Boolean *p) {
	Boolean val;
	
	val = *p;
	*p = false;
	return(!val);
}

static Boolean ActionNotificationCallback(MovieController inMC, short inAction, void *params, UInt32 inFlags, UInt32 *outFlags, void *inRefCon) {
#pragma unused(inParams)
	RECT r;
	Rect mrect;
    CQT	*qt = NULL;
	extern CQTDlg *MovDlg;

    qt = (CQT *)inRefCon;
    if (qt == NULL) { *outFlags = kQTMCActionNotifyCancelled; return 0; }
    if (qt->theMovie == NULL) { *outFlags = kQTMCActionNotifyCancelled; return 0; }
	if (inFlags & kQTMCActionNotifyCancelled) return 0;
    if (inFlags & kQTMCActionNotifyAfter) {
        switch (inAction) {
			case mcActionControllerSizeChanged:
			{
				if (myTestAndSet(&controlChangingBounds) == false) {

					GetMovieNaturalBoundsRect(qt->theMovie, &qt->MvBounds);
//					OffsetRect(&qt->MvBounds, -qt->MvBounds.left, -qt->MvBounds.top);
//					SetMovieBox(qt->theMovie, &qt->MvBounds);

					if ((qt->MvBounds.right-qt->MvBounds.left) < 200)
						qt->MvBounds.right = qt->MvBounds.left + 200;			

					if ((qt->MvBounds.bottom-qt->MvBounds.top) < 16)
						qt->MvBounds.bottom = qt->MvBounds.top + 16;			

					qt->orgMvBounds.top    = qt->MvBounds.top;
					qt->orgMvBounds.bottom = qt->MvBounds.bottom;
					qt->orgMvBounds.left   = qt->MvBounds.left;
					qt->orgMvBounds.right  = qt->MvBounds.right;

					if (MovDlg != NULL && MovDlg->qt == qt)
						MovDlg->ResizeQTDlgWindowItems();
					else
						qt->MoveWindow(0,0,qt->MvBounds.right-qt->MvBounds.left,qt->MvBounds.bottom-qt->MvBounds.top,true);

					qt->GetClientRect(&r);
					mrect.top=(short)r.top;
					mrect.left=(short)r.left;
					mrect.right=(short)r.right;
					mrect.bottom=(short)r.bottom;

					MCSetControllerBoundsRect(qt->mc, &mrect);

					myTestAndClear(&controlChangingBounds);
				}
				break;
			}
			default:
				break;
        }
    }
	
    return 0;
    
}   // ActionNotificationCallback

char isMovieStreaming(Movie MvMovie) 
{
    if (MvMovie == NULL)
		return(FALSE);
    return(GetMovieIndTrackType(MvMovie, 1, kQTSStreamMediaType, movieTrackMediaType | movieTrackEnabledOnly) != NULL);
}

static void MyPPRollProc(Movie theMovie, OSErr prerollErr, void *refcon) {
	char *isPlaying;

	isPlaying = (char *)refcon;
	*isPlaying = 3;
}

static pascal Boolean MCActionFilter(MovieController mc, short action, void *params, long ref) {
	extern CQTDlg *MovDlg;

	if (MovDlg!= NULL && MovDlg->qt!= NULL) {
		if (action == mcActionGoToTime) {
			(MovDlg->qt)->mcActionType = true;
		} else if (action == mcActionStep) {
			(MovDlg->qt)->mcActionType = true;
		}
	}
	return(false);
}

void CQT::Movie_Load(long start, char isPreLoad) {
	RECT r;
	Rect mrect;
	OSErr theError;
	Fixed playRate;
	TimeValue timeNow;
    QTMCActionNotificationRecord actionNotifier;
    MCActionFilterWithRefConUPP mFD(MCActionFilter);

	if (!havemc && havemovie) {
		GetClientRect(&r);
		mrect.top=(short)r.top;
		mrect.left=(short)r.left;
		mrect.right=(short)r.right;
		mrect.bottom=(short)r.bottom;
		mc=NewMovieController(theMovie, &mrect, mcWithFrame) ;
		MCEnableEditing(mc, false);
		MCDoAction(mc, mcActionSetUseBadge, (void *)false);

		if (isPreLoad && isMovieStreaming(theMovie)) {
			actionNotifier.returnSignature	= 0;                            // set to zero when passed to to the Movie Controller, on return will be set to 'noti' if mcActionAddActionNotification is implemented
			actionNotifier.notifyAction		= ActionNotificationCallback;   // function to be called at action time
			actionNotifier.refcon			= this;						 // something to pass to the action function
			actionNotifier.flags			= kQTMCActionNotifyAfter;	// option flags
			MCDoAction(mc, mcActionAddActionNotification, (void *)&actionNotifier);
			SetMoviePlayHints(theMovie, hintsAllowDynamicResize, hintsAllowDynamicResize);
		}
		if (isPreLoad) {
			MCSetActionFilterWithRefCon(mc, mFD, (long)theMovie);
			isFirstLoad = 1;
			isPlaying = 0;
			timeNow = conv_from_msec_mov((double)start, (double)timescale);
			playRate = GetMoviePreferredRate(theMovie);
			theError = PrePrerollMovie(theMovie,timeNow,playRate,NewMoviePrePrerollCompleteProc(MyPPRollProc),&isPlaying);
		} else
			UpdateMovie(theMovie);
		havemc=true;
	} else
		isPlaying = 0;
}

void CQT::Movie_PreLoad(long start) {
	OSErr theError;
	Fixed playRate;
	TimeValue timeNow;

	if (havemc && havemovie) {
		isFirstLoad = 0;
		isPlaying = 0;
		timeNow = conv_from_msec_mov((double)start, (double)timescale);
		playRate = GetMoviePreferredRate(theMovie);
		theError = PrePrerollMovie(theMovie,timeNow,playRate,NewMoviePrePrerollCompleteProc(MyPPRollProc),&isPlaying);
	} else
		isPlaying = 0;
}

void CQT::Play(){
//	CWaitCursor wait;
	OSErr err = noErr;
	Fixed playRate;
	long myPoints;

	if (isPlaying == 3) {
		isPlaying = 2;
		myPoints = GetMovieLoadState(theMovie);
		if (myPoints <= kMovieLoadStateError)
			isPlaying = 0;
		if (isPlaying > 0 && myPoints >= kMovieLoadStateComplete) {
			playRate = GetMoviePreferredRate(theMovie);
			err = PrerollMovie (theMovie, startTime, playRate);
			if (err != noErr)
				isPlaying = 0;
			else {
				if (isFirstLoad) {
					GoToBeginningOfMovie(theMovie);
					LoadMovieIntoRam(theMovie, startTime, duration, unkeepInRam);
					UpdateMovie(theMovie);
				}
			}
			SetMovieActiveSegment(theMovie, startTime, duration);
			GoToBeginningOfMovie(theMovie);
			SetMoviePlayHints(theMovie, hintsScrubMode, hintsScrubMode);
			if (PBC.speed != 100) {
				float speed;
				if (PBC.speed > 0 && PBC.speed < 100) {
					speed = (1.0000 * (float)PBC.speed) / 100.0000;
					SetMovieRate(theMovie, FloatToFixed(speed));
				} else if (PBC.speed > 100) {
					speed = (float)PBC.speed / 100.0000;
					SetMovieRate(theMovie, FloatToFixed(speed));
				} else
					StartMovie(theMovie);
			} else
				StartMovie(theMovie);
			isPlaying = 1;
		}
	}
}

BOOL CQT::EventProc(UINT message, WPARAM wParam, LPARAM lParam){
	TimeValue tv;
	long myPoints = GetMessagePos();
	Fixed playRate;
	MSG myMsg;
	EventRecord qtmlEvt;
	OSErr err = noErr;

	if (message != WM_COMMAND) {
		myMsg.hwnd = m_hWnd;
		myMsg.message = message;
		myMsg.wParam = wParam;
		myMsg.lParam = lParam;
		myMsg.time = GetMessageTime();
		myMsg.pt.x = LOWORD(myPoints);
		myMsg.pt.y = HIWORD(myPoints);
		WinEventToMacEvent(&myMsg, &qtmlEvt);

		if (havemc) {
			if (!IsIconic()) {
				if (isPlaying == 3 || isPlaying == 2) {
					tv = 0L;
					if (isPlaying == 3)
						isPlaying = 2;
					if (isPlaying > 0) {
						myPoints = GetMovieLoadState(theMovie);
						if (myPoints <= kMovieLoadStateError)
							isPlaying = 0;
						if (tv > startTime || myPoints >= kMovieLoadStateComplete) {
							playRate = GetMoviePreferredRate(theMovie);
							err = PrerollMovie (theMovie, startTime, playRate);
							if (err != noErr)
								isPlaying = 0;
							else {
								if (isFirstLoad) {
									GoToBeginningOfMovie(theMovie);
									LoadMovieIntoRam(theMovie, startTime, duration, unkeepInRam);
									UpdateMovie(theMovie);
								}
							}
							SetMovieActiveSegment(theMovie, startTime, duration);
							GoToBeginningOfMovie(theMovie);
							SetMoviePlayHints(theMovie, hintsScrubMode, hintsScrubMode);
							if (PBC.speed != 100) {
								float speed;
								if (PBC.speed > 0 && PBC.speed < 100) {
									speed = (1.0000 * (float)PBC.speed) / 100.0000;
									SetMovieRate(theMovie, FloatToFixed(speed));
								} else if (PBC.speed > 100) {
									speed = (float)PBC.speed / 100.0000;
									SetMovieRate(theMovie, FloatToFixed(speed));
								} else
									StartMovie(theMovie);
							} else
								StartMovie(theMovie);
							isPlaying = 1;
						}
					}
				}
				return(MCIsPlayerEvent(mc, (EventRecord *)&qtmlEvt));
			}
		}
	}
	return(false);
}

void CQT::Close(){
	if (havemc){
		MCSetActionFilterWithRefCon(mc, NULL, NULL);
		DisposeMovieController(mc);
		havemc=false;
	}
	if (havemovie){
		DisposeMovie(theMovie);
//		thisQT = NULL;
		havemovie=false;
	}
}

void CQT::SetStartTime(TimeValue start){
//	startTime=roundUp(conv_from_msec_mov((double)start, (double)timescale));
	startTime=conv_from_msec_mov((double)start, (double)timescale);
}

void CQT::SetDuration(TimeValue howlong){
//	duration=roundUp(conv_from_msec_mov((double)howlong, (double)timescale));
	duration=conv_from_msec_mov((double)howlong, (double)timescale);
	if (duration <= 0 && howlong > 0)
		duration = howlong;
}

TimeValue CQT::GetStartTime(){
//	return roundUp(conv_to_msec_mov((double)startTime, (double)timescale));
	return conv_to_msec_mov((double)startTime, (double)timescale);
}

TimeValue CQT::GetDuration( ){
//	return roundUp(conv_to_msec_mov((double)duration, (double)timescale));
	return conv_to_msec_mov((double)duration, (double)timescale);
}

TimeValue CQT::GetCurTime()
{
	TimeValue ts;

	if (havemovie) {
//		return roundUp(conv_to_msec_mov((double)GetMovieTime(theMovie,NULL), (double)timescale));
		if (havemc)
			return conv_to_msec_mov((double)MCGetCurrentTime(mc, &ts), (double)timescale);
		else
			return conv_to_msec_mov((double)GetMovieTime(theMovie,NULL), (double)timescale);
	} else
		return 0;
}

TimeValue CQT::GetQTMovieDuration( ){
//	return roundUp(conv_to_msec_mov((double)movieDuration, (double)timescale));
	return conv_to_msec_mov((double)movieDuration, (double)timescale);
}

void CQT::FakeMove(TimeValue start) {
//	start=roundUp(conv_from_msec_mov((double)start, (double)timescale));
	start=conv_from_msec_mov((double)start, (double)timescale);
	SetMovieActiveSegment (theMovie,  start, start+1);
	GoToBeginningOfMovie (theMovie);
	SetMovieActiveSegment (theMovie,  0, movieDuration);
}

BOOL CQT::IsQTMovieDone() {
	return(IsMovieDone(theMovie));
}


LRESULT CQT::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT res = 0;
	TimeValue tc;
	TimeValue ts;

	if(message == WM_ERASEBKGND){
//		res = CDialog ::WindowProc(message, wParam, lParam);//result;
		if (havemovie){
			EventProc(message, wParam, lParam);
			if (havemc)
				curTime=MCGetCurrentTime(mc, &ts);
			else
				curTime=GetMovieTime(theMovie,NULL);
			if (backTime!=curTime || isPlaying == 1) {
				if (IsMovieDone(theMovie) && isPlaying == 1) {
					((CQTDlg*)GetParent())->SetCurPos(2);
				} else if (!IsMovieDone(theMovie))
					((CQTDlg*)GetParent())->SetCurPos(1);

				backTime=curTime;
			}
		}
	}else{
		if (havemovie && havemc){
			EventProc(message, wParam, lParam);
			curTime=MCGetCurrentTime(mc, &ts);
			if (IsMovieDone(theMovie) && isPlaying == 1) {
				((CQTDlg*)GetParent())->SetCurPos(2);
			} else if (backTime!=curTime /* || isPlaying */){
				((CQTDlg*)GetParent())->SetCurPos(1);
				backTime=curTime;
			}
			if (mcActionType) {
				mcActionType = false;
				tc = conv_to_msec_mov((double)curTime, (double)ts);
				((CQTDlg*)GetParent())->SetWaveTimeValue(tc, 0L);
			}
		}
		res = CDialog ::WindowProc(message, wParam, lParam);
	}
	return(res);
}

BOOL CQT::OnInitDialog() 
{
	CDialog::OnInitDialog();

//	thisQT = this;
	// TODO: Add extra initialization here
	GrafPort *gp=CreatePortAssociation (m_hWnd, NULL,0);
	cgp=(CGrafPort *)GetWindowPort(gp);
	SetGWorld(cgp, NULL);
	Open();
	MoveWindow(0,0,MvBounds.right-MvBounds.left,MvBounds.bottom-MvBounds.top,true);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CQT::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	myFInfo *saveGlobal_df;

	if (((CQTDlg*)GetParent())->textDoc && (((CQTDlg*)GetParent())->textDoc)->FileInfo) {
		saveGlobal_df = global_df;
		global_df = (((CQTDlg*)GetParent())->textDoc)->FileInfo;
		if (global_df->cpMvTr != NULL) {
//			GlobalDC = ((CQTDlg*)GetParent())->textDC;
			GlobalDoc = ((CQTDlg*)GetParent())->textDoc;
			if (((CQTDlg*)GetParent())->doErase)
				CopyAndFreeMovieData(TRUE);
		}
		global_df = saveGlobal_df;
	}
	Close();
	if (cgp != NULL) {
		DestroyPortAssociation (cgp);
		cgp = NULL;
	}
	return CDialog::DestroyWindow();
}


void CQT::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	delete this;
	CDialog::PostNcDestroy();
}



BOOL CQT::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	EventProc(message, wParam, lParam);
	return 1;//CDialog::OnChildNotify(message, wParam, lParam, pLResult);
}


void CQT::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CDialog::OnLButtonDown(nFlags, point);
}

BOOL CQT::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	EventProc(WM_NOTIFY, wParam, lParam);

	return 1;//CDialog::OnNotify(wParam, lParam, pResult);
}
