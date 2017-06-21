// W_Pict.cpp : implementation file
//

#include "ced.h"
#include "MMedia.h"
#include "W95_Pict.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern struct DefWin PictWinSize;

char pictName[_MAX_PATH+_MAX_FNAME];
char isShowPict = FALSE;
char isOverRidePicName = FALSE;
CPict *pictDial = NULL;
/////////////////////////////////////////////////////////////////////////////
// CPict dialog

CPict::CPict(CString pathName, CWnd* pParent /*=NULL*/)
	: CDialog(CPict::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPict)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	cgp = NULL;
	setSize = FALSE;
	gImporter = NULL;
	u_strcpy(fname, pathName, _MAX_PATH+_MAX_FNAME);
	Create(IDD, pParent);
}


void CPict::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPict)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPict, CDialog)
	//{{AFX_MSG_MAP(CPict)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPict::OnInitDialog() {

	CDialog::OnInitDialog();

	if (PictWinSize.top || PictWinSize.width || PictWinSize.height || PictWinSize.left) {
		CRect lpRect;
		long width, hight;

		this->GetWindowRect(&lpRect);
		width = lpRect.right - lpRect.left;
		hight = lpRect.bottom - lpRect.top;
		lpRect.top = PictWinSize.top;
		lpRect.bottom = lpRect.top + hight;
		lpRect.left = PictWinSize.left;
		lpRect.right = lpRect.left + width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}
	GrafPort *gp=CreatePortAssociation (m_hWnd, NULL,0);
	cgp=(CGrafPort *)GetWindowPort(gp);
	SetGWorld(cgp, NULL);
	isOverRidePicName = FALSE;
	if (PlayingContMovie)
		isShowPict = TRUE;
	return TRUE;
}

BOOL CPict::DestroyWindow() 
{
	if (gImporter != NULL)
		CloseComponent(gImporter);
	if (cgp != NULL) {
		DestroyPortAssociation (cgp);
		cgp = NULL;
	}
	isOverRidePicName = FALSE;
	isShowPict = FALSE;
	return CDialog::DestroyWindow();
}

void CPict::OnCancel() {
	CRect lpRect;

	this->GetWindowRect(&lpRect);
	DestroyWindow();
	if (pictDial != NULL) {
		delete pictDial;
		pictDial = NULL;
	}
	pictName[0] = EOS;
	isOverRidePicName = FALSE;
	isShowPict = FALSE;
	PictWinSize.top = lpRect.top;
	PictWinSize.left = lpRect.left;
	PictWinSize.width = lpRect.right;
	PictWinSize.height = lpRect.bottom;
	WriteCedPreference();
}

void CPict::OnDestroy() 
{
	CRect lpRect;

	if (gImporter != NULL)
		CloseComponent(gImporter);
	this->GetWindowRect(&lpRect);
	CDialog::OnDestroy();
	if (pictDial != NULL) {
		delete pictDial;
		pictDial = NULL;
	}
	pictName[0] = EOS;
	isOverRidePicName = FALSE;
	isShowPict = FALSE;
	PictWinSize.top = lpRect.top;
	PictWinSize.left = lpRect.left;
	PictWinSize.width = lpRect.right;
	PictWinSize.height = lpRect.bottom;
	WriteCedPreference();
}

void CPict::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	FSSpec  	  sfFile;
	double		  h, v, r;
	unsigned long myCount,
				  myIndex;
	Str255		  filename;
	ComponentResult isErr;
	CRect		  lpRect;
	Handle		  myHandle = NULL;
	Size		  mySize = 0;

	this->GetWindowRect(&lpRect);
	// TODO: Add extra initialization here

	if (!setSize) {
		if (gImporter != NULL)
			CloseComponent(gImporter);
		c2pstrcpy (filename, fname);                          
		FSMakeFSSpec(0, 0L, (const unsigned char*)filename, &sfFile);
		if (GetGraphicsImporterForFile(&sfFile, &gImporter) != noErr) {
			setSize = TRUE;
			return;
		}
	}
	if (gImporter == NULL)
		return;
	if (GraphicsImportGetImageCount(gImporter, &myCount) != noErr)
		return;
	if (myCount < 1 || myCount > 1)
		return;

	for (myIndex=1; myIndex <= myCount; myIndex++) {
		if (GraphicsImportSetImageIndex(gImporter, myIndex) != noErr)
			return;
		if (!setSize) {
			if (GraphicsImportGetNaturalBounds(gImporter, &myRect) != noErr)
				return;
			setSize = TRUE;
			h = myRect.right  - myRect.left;
			v = myRect.bottom - myRect.top;
			if (PictWinSize.width != 0) {
				if (h > PictWinSize.width) {
					r = (h / v);
					myRect.top = 0;
					myRect.right = PictWinSize.width - PictWinSize.left;
					myRect.left = 0;
					h = myRect.right;
					h /= r;
					myRect.bottom = (int)h;
				}
			} else if (v > lpRect.bottom-lpRect.top) {
				r = (h / v);
				myRect.top = 0;
				myRect.bottom = lpRect.bottom-lpRect.top;
				myRect.left = 0;
				v = myRect.bottom;
				v *= r;
				myRect.right = (int)v;
			}
			isErr = GraphicsImportSetBoundsRect(gImporter, &myRect);
			SizeWindow((WindowPtr)cgp, myRect.right, myRect.bottom,true);
		} else {
			isErr = GraphicsImportSetBoundsRect(gImporter, &myRect);
		}
		isErr = GraphicsImportSetGWorld(gImporter, cgp, NULL);		
		isErr = GraphicsImportDraw(gImporter);
	}
	CDialog::OnPaint();
}

void CPict::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	double	h, v, r;
	int oldH, oldV;

	if (gImporter == NULL)
		return;

	oldH = myRect.right  - myRect.left;
	oldV = myRect.bottom - myRect.top;
	if (GraphicsImportGetNaturalBounds(gImporter, &myRect) != noErr)
		return;		

	h = myRect.right  - myRect.left;
	v = myRect.bottom - myRect.top;
	r = (h / v);

	if (oldH == cx && oldV != cy) {
		h = cy;
		h /= r;
		cx = (int)h;
	}

	myRect.top = 0;
	myRect.right = cx;
	myRect.left = 0;
	h = myRect.right;
	h /= r;
	myRect.bottom = (int)h;

	GraphicsImportSetBoundsRect(gImporter, &myRect);
	SizeWindow((WindowPtr)cgp, myRect.right, myRect.bottom,true);
	GraphicsImportSetGWorld(gImporter, cgp, NULL);		
	GraphicsImportDraw(gImporter);

	PictWinSize.width  = PictWinSize.left + myRect.right;
	PictWinSize.height = PictWinSize.top + myRect.bottom;

	WriteCedPreference();
}

int DisplayPhoto(char *fname) {
	if (!strcmp(pictName, fname) && !isOverRidePicName)
		return(0);
	isOverRidePicName = FALSE;
	strcpy(pictName, fname);
	if (pictDial != NULL)
		pictDial->OnCancel();
	pictDial = new CPict(fname, AfxGetApp()->m_pMainWnd);
	return(0);
}
