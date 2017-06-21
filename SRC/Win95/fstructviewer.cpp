// cFStructViewer.cpp : implementation file
//

#include "ced.h"
#include "CedDlgs.h"
#include "FStructViewer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

cFStructViewer *FViewerDlg = NULL;

/////////////////////////////////////////////////////////////////////////////
// cFStructViewer dialog


cFStructViewer::cFStructViewer(CWnd* pParent /*=NULL*/)
	: CDialog(cFStructViewer::IDD, pParent)
{

	//{{AFX_DATA_INIT(cFStructViewer)
	m_DepTier = _T("");
	m_SpTier = _T("");
	//}}AFX_DATA_INIT
//	Create(IDD, pParent);
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

BOOL cFStructViewer::OnInitDialog() {
	CDialog::OnInitDialog();

	textDC = NULL;
	textDoc = NULL;
	UpdateData(FALSE);
	ShowWindow(SW_SHOWNORMAL );
	return TRUE;
}

void cFStructViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cFStructViewer)
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Text(pDX, IDC_DEP_TIER, m_DepTier);
	DDX_Text(pDX, IDC_SPEAKER_TIER, m_SpTier);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(cFStructViewer, CDialog)
	//{{AFX_MSG_MAP(cFStructViewer)
	ON_BN_CLICKED(IDC_EXPAND_ALL, OnExpandAll)
	ON_BN_CLICKED(IDC_COLLAPSE_ALL, OnCollapseAll)
	ON_BN_CLICKED(IDC_BAD, OnBad)
	ON_BN_CLICKED(IDC_GOOD, OnGood)
	ON_BN_CLICKED(IDC_SKIP_NEXT, OnSkipNext)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cFStructViewer message handlers

void cFStructViewer::OnExpandAll() 
{
	ExpandTree( &m_tree, m_tree.GetRootItem(), TVE_EXPAND );	
}

void cFStructViewer::OnCollapseAll() 
{
	ExpandTree( &m_tree, m_tree.GetRootItem(), TVE_COLLAPSE );	
}

void cFStructViewer::OnCancel() {
	delete FViewerDlg;
	FViewerDlg = NULL;
}

static void getTierText(unCH *st) {
	long i;

	st[0] = EOS;
	strcpy(st, global_df->row_txt->line);
	MoveDown(-1);
	while (!isSpeaker(global_df->row_txt->line[0]) && 
		!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
		strcat(st, global_df->row_txt->line);
		MoveDown(-1);
	}
	if (st[0]) {
		for (i=0; st[i] != EOS; i++) {
			if (st[i] == SNL_C || st[i] == NL_C || st[i] == '\t')
				st[i] = ' ';
		}
	}
}

char ShowFStruct(void) {
	char synEmpty;
	long i;
	CString synTier;

	synEmpty = FALSE;
	DrawCursor(0);
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;
	DestroyCaret();
	global_df->DrawCur = FALSE;
	if (FViewerDlg == NULL) {
		FViewerDlg = new cFStructViewer(AfxGetApp()->m_pMainWnd);
	} else {
		FViewerDlg->m_tree.DeleteAllItems();
		FViewerDlg->m_DepTier = _T("");
		FViewerDlg->m_SpTier = _T("");
	}

//	FViewerDlg->textDC = GlobalDC;
	FViewerDlg->textDoc = GlobalDoc;

	ChangeCurLineAlways(0);
	while (!isMainSpeaker(global_df->row_txt->line[0]) && 
					!AtTopEnd(global_df->row_txt, global_df->tail_text, FALSE))
		MoveUp(-1);
	if (isMainSpeaker(global_df->row_txt->line[0])) {
		do {
			getTierText(templine);
			if (uS.partcmp(templine,"%syn:",FALSE,FALSE)) {
				for (i=0; templine[i] != ':' && templine[i] != EOS; i++) ;
				if (templine[i] == ':')
					i++;
				for (; isSpace(templine[i]); i++) ;
				if (templine[i] == EOS)
					synEmpty = TRUE;
				if (i > 0)
					strcpy(templine, templine+i);
				synTier = templine;
				BuildTree(FViewerDlg->m_tree, synTier);
			} else if (templine[0] == '*') {
				FViewerDlg->m_SpTier = templine;
			} else if (uS.partcmp(templine,"%mor:",FALSE,FALSE) && FViewerDlg->m_DepTier.IsEmpty()) {
				FViewerDlg->m_DepTier = templine;
			} else if (uS.partcmp(templine,"%pos:",FALSE,FALSE)) {
				FViewerDlg->m_DepTier = templine;
			}
			if (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE) || 
								global_df->row_txt->line[0] == '@')
				break;
		} while (!isMainSpeaker(global_df->row_txt->line[0])) ;
	}
	if (!synEmpty) {
		FViewerDlg->UpdateData(FALSE);
		ExpandTree( &FViewerDlg->m_tree, FViewerDlg->m_tree.GetRootItem(), TVE_EXPAND );	
		return(1);
	} else {
		sprintf(GlobalDoc->isAddFStructComm, "%c%%syc:	not parsed", NL_C);
		return(0);
	}
}

void cFStructViewer::OnBad() 
{
	UpdateData(TRUE);
	CEnterComments dlg;
	dlg.m_Comments = _T("");
	if (dlg.DoModal() == IDOK) {
		if (dlg.m_Comments.IsEmpty())
			sprintf(textDoc->isAddFStructComm, "%c%%syc:	bad", NL_C);
		else
			sprintf(textDoc->isAddFStructComm, "%c%%syc:	bad, %s", NL_C, dlg.m_Comments);
		uS.remFrontAndBackBlanks(textDoc->isAddFStructComm);
		textDoc->UpdateAllViews(NULL, 0L, NULL);
	}
}

void cFStructViewer::OnGood() 
{
	UpdateData(TRUE);
	sprintf(textDoc->isAddFStructComm, "%c%%syc:	good", NL_C);
	uS.remFrontAndBackBlanks(textDoc->isAddFStructComm);
	textDoc->UpdateAllViews(NULL, 0L, NULL);
}

void cFStructViewer::OnSkipNext() 
{
	UpdateData(TRUE);
	textDoc->isAddFStructComm[0] = '\001';
	textDoc->isAddFStructComm[1] = EOS;
	textDoc->UpdateAllViews(NULL, 0L, NULL);	
}
