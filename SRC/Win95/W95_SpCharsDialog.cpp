// SpCharsDialog.cpp : implementation file
//

#include "ced.h"
#include "Clan2Doc.h"
#include "W95_SpCharsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSpCharsDialog *spCharsDlg = NULL;
/////////////////////////////////////////////////////////////////////////////
// CSpCharsDialog dialog


CSpCharsDialog::CSpCharsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSpCharsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpCharsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Create(IDD, pParent);
}


void CSpCharsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpCharsDialog)
	DDX_Control(pDX, IDC_CLAN_SP_CHARS, m_Sp_Chars_Control);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpCharsDialog, CDialog)
	//{{AFX_MSG_MAP(CSpCharsDialog)
	ON_LBN_SELCHANGE(IDC_CLAN_SP_CHARS, OnSelchangeClanSpChars)
	ON_LBN_DBLCLK(IDC_CLAN_SP_CHARS, OnDblclkClanSpChars)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpCharsDialog message handlers
#define isHex(x) (x == 'A' || x == 'B' || x == 'C' || x == 'D' || x == 'E' || x == 'F')

static void makeUnicodeString(wchar_t *dist, char *src) {
	long f, t, i;
	unsigned long uc;
	char hexStr[256];

	f=0L;
	t=0L;
	while (src[f] != EOS) {
		if (src[f] == '0' && src[f+1] == 'x') {
			i = 0L;
			hexStr[i++] = src[f++];
			hexStr[i++] = src[f++];
			for (; isdigit(src[f]) || isHex(src[f]); i++)
				hexStr[i] = src[f++];
			hexStr[i] = EOS;
			sscanf(hexStr, "%x", &uc);
			dist[t++] = uc;
		} else
			dist[t++] = src[f++];
	}
	dist[t] = EOS;
}

BOOL CSpCharsDialog::OnInitDialog() {
	CDialog::OnInitDialog();

#ifdef _UNICODE
	wchar_t wbuf[256]; // CA CHARS

	makeUnicodeString(wbuf, "0x2191 shift to high pitch; F1 up-arrow");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2193 shift to low pitch; F1 down-arrow");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x21D7 rise to high; F1 1");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2197 rise to mid; F1 2");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2192 level; F1 3");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2198 fall to mid; F1 4");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x21D8 fall to low; F1 5");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x221E unmarked ending; F1 6");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x224B 0x224Bcontinuation; F1 +");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2219 inhalation; F1 .");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2248 latching0x2248; F1 =");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2261 0x2261uptake; F1 u");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2308 top begin overlap; F1 [");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2309 top end overlap; F1 ]");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x230A bottom begin overlap; F1 {");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x230B bottom end overlap; F1 }");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2206 0x2206faster0x2206; F1 right-arrow");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2207 0x2207slower0x2207; F1 left-arrow");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x204E 0x204Ecreaky0x204E; F1 *");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2047 0x2047unsure0x2047; F1 /");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x00B0 0x00B0softer0x00B0; F1 0");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x25C9 0x25C9louder0x25C9; F1 )");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2581 0x2581low pitch0x2581; F1 d");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2594 0x2594high pitch0x2594; F1 h");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x263A 0x263Asmile voice0x263A; F1 l");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x264B 0x264Bbreathy voice0x264B marker; F1 b");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x222C 0x222Cwhisper0x222C; F1 w");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x03AB 0x03AByawn0x03AB; F1 y");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x222E 0x222Esinging0x222E; F1 s");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x00A7 0x00A7precise0x00A7; F1 p");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x223E constriction0x223E; F1 n");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x21BB 0x21BBpitch reset; F1 r");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x1F29 laugh in a word; F1 c");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x201E Tag/sentence final particle;F2 t");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2021 0x2021 Vocative or summons; F2 v");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, " 0x0323 Arabic dot diacritic; F2 ,");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x02B0 Arabic raised h; F2 h");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, " 0x0304 Stress; F2 -");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x0294 Glottal stop0x0294; F2 q");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x0295 Reverse glottal0x0295; F2 Q");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, " 0x030C Caron; F2 ;");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x02C8 raised0x02C8 stroke; F2 1");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x02CC lowered0x02CC stroke; F2 2");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2039 0x2039begin phono group0x203A marker; F2 <");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x203A 0x2039end phono group0x203A marker; F2 >");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x3014 0x3014begin sign group0x3015; F2 {");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x3015 0x3014end sign group0x3015; F2 }");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2026 %pho missing word; F2 m");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x0332 und0x0332e0x0332r0x0332line; F2 _");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x201C open 0x201Cquote0x201D; F2 '");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x201D close 0x201Cquote0x201D; F2 \"");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2018 open 0x2018quote0x2019; F2 '");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2019 close 0x2018quote0x2019; F2 \"");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x2260 0x2260row; F2 =");
	m_Sp_Chars_Control.AddString(wbuf);

	makeUnicodeString(wbuf, "0x21AB 0x21ABr-r0x21ABrabbit; F2 /");
	m_Sp_Chars_Control.AddString(wbuf);

#else // else _UNICODE
	m_Sp_Chars_Control.AddString("This ONLY works in Unicode CLAN");
#endif // else _UNICODE

	UpdateData(FALSE);
	ShowWindow(SW_SHOWNORMAL );
	return TRUE;
}

void CSpCharsDialog::OnSelchangeClanSpChars() 
{
#ifdef _UNICODE
/*
	int i;
	wchar_t wbuf[256];

	i = m_Sp_Chars_Control.GetCurSel();
	if (i != LB_ERR) {
		m_Sp_Chars_Control.GetText(i, wbuf);

		CWinApp* wApp = AfxGetApp();
		POSITION pos = wApp->GetFirstDocTemplatePosition();

		while (pos != NULL) {
			CDocTemplate* pTemplate = (CDocTemplate*)wApp->GetNextDocTemplate(pos);
			ASSERT_KINDOF(CDocTemplate, pTemplate);
			POSITION pos2 = pTemplate->GetFirstDocPosition();
			while (pos2 != NULL) {
				CClan2Doc* pDoc = (CClan2Doc *)pTemplate->GetNextDoc(pos2);
				if (pDoc == GlobalDoc) {
					pDoc->InsertSpChar = wbuf[0];
					pDoc->UpdateAllViews(NULL, 0L, NULL);
				}
			}
		}
	}
*/
#endif
}

void CSpCharsDialog::OnDblclkClanSpChars() 
{
#ifdef _UNICODE
	int i;
	wchar_t wbuf[256];

	i = m_Sp_Chars_Control.GetCurSel();
	if (i != LB_ERR) {
		m_Sp_Chars_Control.GetText(i, wbuf);

		CWinApp* wApp = AfxGetApp();
		POSITION pos = wApp->GetFirstDocTemplatePosition();

		while (pos != NULL) {
			CDocTemplate* pTemplate = (CDocTemplate*)wApp->GetNextDocTemplate(pos);
			ASSERT_KINDOF(CDocTemplate, pTemplate);
			POSITION pos2 = pTemplate->GetFirstDocPosition();
			while (pos2 != NULL) {
				CClan2Doc* pDoc = (CClan2Doc *)pTemplate->GetNextDoc(pos2);
				if (pDoc == GlobalDoc) {
					if (wbuf[0] == ' ')
						pDoc->InsertSpChar = wbuf[1];
					else
						pDoc->InsertSpChar = wbuf[0];
					pDoc->UpdateAllViews(NULL, 0L, NULL);
				}
			}
		}
	}
#endif	
}

void CSpCharsDialog::OnCancel() {
	DestroyWindow();
	delete spCharsDlg;
	spCharsDlg = NULL;
}

void CSpCharsDialog::OnDestroy() 
{
	CDialog::OnDestroy();
}
