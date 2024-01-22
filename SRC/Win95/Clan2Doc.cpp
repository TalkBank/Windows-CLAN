// Clan2Doc.cpp : implementation of the CClan2Doc class
//

#include "ced.h"

#include "Clan2Doc.h"

#include "common.h"

#ifndef __MWERKS__
#include "msutil.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char *nameOverride, *pathOverride;
long rowLimitOverride;

/////////////////////////////////////////////////////////////////////////////
// CClan2Doc

IMPLEMENT_DYNCREATE(CClan2Doc, CDocument)

BEGIN_MESSAGE_MAP(CClan2Doc, CDocument)
	//{{AFX_MSG_MAP(CClan2Doc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClan2Doc construction/destruction

CClan2Doc::CClan2Doc()
{
	// TODO: add one-time construction code here
	FileInfo = NULL;
	ExecuteCommNUM = 0;
	SaveMovieInfo = 0;
	SetFakeHilight = FALSE;
	ChangeCursorInfo = 0L;
	WC_dummy = 0;
	isTranscribing = false;
	isAddFStructComm[0] = EOS;
	RestorePlayMovieInfo = FALSE;
	DoWalkerController = FALSE;
	re_wrap = FALSE;
	redo_soundwave = FALSE;
	RedisplayWindow = FALSE;
	InsertSpChar = 0;
	isNeedResize = FALSE;
	m_RowLimit = 0L;
	docFName[0] = EOS;
}

CClan2Doc::~CClan2Doc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPadView File Handling

BOOL CClan2Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	FNType st[FNSize];
	char *filePath;

	if (nameOverride != NULL) {
		strcpy(docFName, nameOverride);
		nameOverride = NULL;
		m_RowLimit = rowLimitOverride;
		rowLimitOverride = 0L;
		if (m_RowLimit != 0L && m_RowLimit < 10L)
			m_RowLimit = 10L;
	} else
		strcpy(docFName, NEWFILENAME);

	if (pathOverride != NULL) {
		strcpy(st, pathOverride);
		strcat(st, docFName);
		filePath = st;
	} else
		filePath = "";
	CWinApp* wApp = AfxGetApp();
	POSITION pos = wApp->GetFirstDocTemplatePosition();
	while (pos != NULL) {
		CDocTemplate* pTemplate = (CDocTemplate*)wApp->GetNextDocTemplate(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		POSITION pos2 = pTemplate->GetFirstDocPosition();
		while (pos2 != NULL) {
			CDocument* pDoc = pTemplate->GetNextDoc(pos2);
			if (AfxComparePath(pDoc->GetTitle(), cl_T(docFName)) &&
				AfxComparePath(pDoc->GetPathName(), cl_T(filePath))) {
				POSITION pos3 = pDoc->GetFirstViewPosition();
				while (pos3 != NULL) {
					CView* pView = pDoc->GetNextView(pos3);
					if (pView != NULL) {
						CFrameWnd* pFrame = pView->GetParentFrame();
						if (pFrame != NULL)
							pFrame->ActivateFrame();
						if (strcmp(docFName, NEWFILENAME))
							pDoc->UpdateAllViews(NULL, 0L, NULL);
						break;
					}
				}
				pathOverride = NULL;
				*docFName = EOS;
				return FALSE;
			}
		}
	}

	SetTitle(cl_T(docFName));
	if (pathOverride != NULL) {
		strcpy(docFName, st);
		pathOverride = NULL;
	}

	return TRUE;
}

BOOL CClan2Doc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	u_strcpy(docFName, lpszPathName, _MAX_PATH+_MAX_FNAME);
	return TRUE;
}

BOOL CClan2Doc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class

	global_df = FileInfo;
	if (global_df == NULL)
		return FALSE;
	u_strcpy(global_df->fileName, lpszPathName, _MAX_PATH+_MAX_FNAME);
	if (!SaveToFile(global_df->fileName, TRUE)) {
		return FALSE;
	}
	sprintf(global_df->err_message, "-File \"%s\" written.", global_df->fileName); 
	global_df->DataChanged = '\0';
	ResetUndos();
	SetModifiedFlag(FALSE);
	return TRUE;
//	return CDocument::OnSaveDocument(lpszPathName);
}

/////////////////////////////////////////////////////////////////////////////
// CClan2Doc serialization

void CClan2Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CClan2Doc diagnostics

#ifdef _DEBUG
void CClan2Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CClan2Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CClan2Doc commands


void CClan2Doc::myUpdateAllViews(BOOL isErase)
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL) {
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		pView->Invalidate(isErase);
		pView->SetActiveWindow();
	}
}
