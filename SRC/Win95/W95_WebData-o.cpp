// W95_WebData.cpp : implementation file
//

#include "ced.h"
#include "c_clan.h"
#include "CedDlgs.h"
#include "W95_WebData.h"
#include "MMedia.h"

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

#define PREVDIRLABLE ":Previous Directory:"

#define CHILDES_URL    "http://childes.talkbank.org/data-orig/"
#define TALKBANK_URL   "http://www.talkbank.org/data-orig/"


extern struct DefWin WebItemsWinSize;

extern char *nameOverride, *pathOverride;
extern char tFileBuf[];
extern char proxyAddPort[];
extern FNType prefsDir[];

unCH *getWebMedia;
CWebData *webDataDlg = NULL;
static unCH fulURLPath[2000];
char URL_passwd[129];
static char  *ROOTWEBDIR = NULL;


/////////////////////////////////////////////////////////////////////////////
// CWebData dialog


CWebData::CWebData(CWnd* pParent /*=NULL*/)
	: CDialog(CWebData::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWebData)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Create(IDD, pParent);
}


void CWebData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWebData)
	DDX_Control(pDX, IDC_WEB_DATA, m_WebDataListControl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWebData, CDialog)
	//{{AFX_MSG_MAP(CWebData)
	ON_LBN_SELCHANGE(IDC_WEB_DATA, OnSelchangeWebData)
	ON_LBN_DBLCLK(IDC_WEB_DATA, OnDblclkWebData)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_WEB_TALKBANK, OnWebTalkbank)
	ON_BN_CLICKED(IDC_WEB_CHILDES, OnWebChildes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWebData main
/*
 * This is an example showing how to get a single file from an FTP server.
 * It delays the actual destination file creation until the first write
 * callback so that it won't create an empty file in case the remote file
 * doesn't exist or something else fails.
 */

#include "curl.h"
#include "curl_easy.h"
#include "W95_progress.h"

static size_t totalBytesRead;
static size_t totalsize = 2400000L /* 3072000L*/;

struct FtpFile {
  char *filename;
  FILE *stream;
};

static int my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
	int perc;
	unsigned long percl; 
	struct FtpFile *out=(struct FtpFile *)stream;
	if(out && !out->stream) {
		/* open file for writing */
		out->stream=fopen(out->filename, "wb");
		if(!out->stream)
			return -1; /* failure, can't open file to write */
	}
	totalBytesRead += nmemb;
	percl = (100L * totalBytesRead) / totalsize;
	perc = (int)percl;
	setCurrentProgressBarValue(perc);
	return fwrite(buffer, size, nmemb, out->stream);
}

bool curlURLDownloadToFile(unCH *fulURLPath, unCH *fname, size_t isProgres) {
	CURL *curl;
	CURLcode res;
	bool ret;
	struct FtpFile ftpfile;

	u_strcpy(templineC, fname, UTTLINELEN);
	ftpfile.filename = templineC;
	ftpfile.stream = NULL;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		totalBytesRead = 0L;
		if (isProgres > 0) {
			totalsize = isProgres;
			initProgressBar(_T("Downloading File..."));
		}
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
		if (proxyAddPort[0] != EOS)
			curl_easy_setopt(curl, CURLOPT_PROXY, proxyAddPort);
		/* Get curl 7.9.2 from sunet.se's FTP site: */
		u_strcpy(templineC1, fulURLPath, UTTLINELEN);
		curl_easy_setopt(curl, CURLOPT_URL, templineC1);
		/* Define our callback to get called when there's data to be written */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
		/* Set a pointer to our struct to pass to the callback */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
		curl_easy_setopt(curl, CURLOPT_USERPWD, URL_passwd);
/* Switch on full protocol/debug output */
//curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
		res = curl_easy_perform(curl);
		/* always cleanup */
		curl_easy_cleanup(curl);
		deleteProgressBar();
		if (res != CURLE_OK) {
			ret = false;
		} else
			ret = true;
		if (ftpfile.stream)
			fclose(ftpfile.stream); /* close the local file */
	} else
		ret = false;
	curl_global_cleanup();
	return(ret);
}

long CWebData::extractNames(char *st) {
	long i, beg;

	for (i=0; st[i] != EOS; i++) {
		if (uS.mStrnicmp(st+i, "<TITLE>", 7) == 0) {
			for (i=i+7; isSpace(st[i]); i++) ;
			if (uS.mStrnicmp(st+i, "401 Authorization", 17) == 0) {
				return(-1L);
			}
		} else
		if (uS.mStrnicmp(st+i, "<A HREF=\"", 9) == 0) {
			beg = i;
			for (; st[i] != EOS && st[i] != '"'; i++) ;
			if (st[i] == '"')
				i++;
			if (st[i] == EOS) {
				strcpy(st, st+beg-1);
				beg = strlen(st);
				return(beg);
			}
			beg = i;
			for (; st[i] != '"' && st[i] != EOS; i++) ;
			if (st[i] == '"') {
				st[i] = EOS;
				if (st[beg] != '?') {
					if (st[beg] == '/')
						m_WebDataListControl.AddString(cl_T(PREVDIRLABLE));
					else if (st[i-1] == '/' || strchr(st+beg, '.') == NULL || uS.fpatmat(st+beg, "*.cha") || uS.fpatmat(st+beg, "*.cdc"))
						m_WebDataListControl.AddString(cl_T(st+beg));
				}
				st[i] = '"';
			} else if (st[i] == EOS) {
				strcpy(st, st+beg-1);
				beg = strlen(st);
				return(beg);
			}
		}
	}
	return(0L);
}

void CWebData::handleWebDirFile(unCH *fname) {
	long  count, offset;
	bool  isPasswdNeeded;
	FILE *fp;

repeat_read:
	isPasswdNeeded = false;
	if ((fp=_wfopen(fname, cl_T("rb"))) != NULL) {
		offset = 0L;
		m_WebDataListControl.ResetContent();
		while (!feof(fp)) {
			count = fread(templineC+offset, sizeof(char), UTTLINELEN-offset, fp);
			if (ferror(fp))
				break;
			templineC[offset+count] = EOS;
			offset = extractNames(templineC);
			if (offset == -1L) {
				isPasswdNeeded = true;
				break;
			}
		}
		fclose(fp);
		_wunlink(fname);
	}

	if (isPasswdNeeded) {
		CGetWebPasswd dlg;
		int len;

		dlg.m_Passwd = cl_T("");
		dlg.m_Username = cl_T("");
		DrawCursor(1);
		if (dlg.DoModal() == IDOK) {
			DrawCursor(0);
			u_strcpy(URL_passwd, dlg.m_Username, 128);
			strcat(URL_passwd, ":");
			len = strlen(URL_passwd);
			u_strcpy(URL_passwd+len, dlg.m_Passwd, 128-len);
			if (curlURLDownloadToFile(fulURLPath, fname, 0L) == true)
				goto repeat_read;
			else {
				do_warning("Error downloading data from the web (2)", 0);
				m_WebDataListControl.AddString(cl_T(PREVDIRLABLE));
			}
		} else {
			URL_passwd[0] = EOS;
			m_WebDataListControl.AddString(cl_T(PREVDIRLABLE));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWebData message handlers
BOOL CWebData::OnInitDialog() {
	int  i;
	unCH fname[FNSize];

	CDialog::OnInitDialog();

	URL_passwd[0] = EOS;
	if (URL_Address[0] !=EOS)
		ROOTWEBDIR = URL_Address;
	else
		ROOTWEBDIR = CHILDES_URL;
	u_strcpy(fulURLPath, ROOTWEBDIR, 2000);
	u_strcpy(fname, prefsDir, FNSize);
	i = strlen(fulURLPath) - 1;
	if (fulURLPath[i] == '/')
		i--;
	for (; i >= 0 && fulURLPath[i] != '/'; i--) ;
	i++;
	strcat(fname, fulURLPath+i);
	i = strlen(fname) - 1;
	if (fname[i] == '/')
		fname[i] = EOS;
	if (curlURLDownloadToFile(fulURLPath, fname, 0L) == true) {
//	if (URLDownloadToFile(NULL, fulURLPath, fname, 0, NULL) == S_OK) {
		handleWebDirFile(fname);
	} else
		do_warning("Error downloading data from the web (3)", 0);

	UpdateData(FALSE);
	if (WebItemsWinSize.top || WebItemsWinSize.width || WebItemsWinSize.height || WebItemsWinSize.left) {
		CRect lpRect;
		long width, hight;

		this->GetWindowRect(&lpRect);
		width = lpRect.right - lpRect.left;
		hight = lpRect.bottom - lpRect.top;
		lpRect.top = WebItemsWinSize.top;
		lpRect.bottom = lpRect.top + hight;
		lpRect.left = WebItemsWinSize.left;
		lpRect.right = lpRect.left + width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}
	ShowWindow(SW_SHOWNORMAL );
	return TRUE;
}

void CWebData::OnWebTalkbank() 
{
	int  i;
	unCH fname[FNSize];

	ROOTWEBDIR = TALKBANK_URL;
	u_strcpy(fulURLPath, ROOTWEBDIR, 2000);
	u_strcpy(fname, prefsDir, FNSize);
	i = strlen(fulURLPath) - 1;
	if (fulURLPath[i] == '/')
		i--;
	for (; i >= 0 && fulURLPath[i] != '/'; i--) ;
	i++;
	strcat(fname, fulURLPath+i);
	i = strlen(fname) - 1;
	if (fname[i] == '/')
		fname[i] = EOS;
	if (curlURLDownloadToFile(fulURLPath, fname, 0L) == true) {

//	if (URLDownloadToFile(NULL, fulURLPath, fname, 0, NULL) == S_OK) {
		handleWebDirFile(fname);
	} else
		do_warning("Error downloading data from the web (4)", 0);

	UpdateData(FALSE);	
}

void CWebData::OnWebChildes() 
{
	int  i;
	unCH fname[FNSize];

	if (URL_Address[0] !=EOS)
		ROOTWEBDIR = URL_Address;
	else
		ROOTWEBDIR = CHILDES_URL;
	u_strcpy(fulURLPath, ROOTWEBDIR, 2000);
	u_strcpy(fname, prefsDir, FNSize);
	i = strlen(fulURLPath) - 1;
	if (fulURLPath[i] == '/')
		i--;
	for (; i >= 0 && fulURLPath[i] != '/'; i--) ;
	i++;
	strcat(fname, fulURLPath+i);
	i = strlen(fname) - 1;
	if (fname[i] == '/')
		fname[i] = EOS;
	if (curlURLDownloadToFile(fulURLPath, fname, 0L) == true) {

//	if (URLDownloadToFile(NULL, fulURLPath, fname, 0, NULL) == S_OK) {
		handleWebDirFile(fname);
	} else
		do_warning("Error downloading data from the web (5)", 0);

	UpdateData(FALSE);	
}

BOOL CWebData::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE  || pMsg->wParam == VK_CANCEL)) {
		return TRUE;
	} else if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_EXECUTE || pMsg->wParam == VK_RETURN)) {
		return TRUE;
	} else if (pMsg->message == WM_CHAR && pMsg->wParam == 0x17/*^W*/) {
		SendMessage(WM_COMMAND, IDCANCEL, 0);
		return TRUE;
	} else if (pMsg->message == WM_CHAR && pMsg->wParam == 0x11/*^Q*/) {
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CWebData::OnSelchangeWebData() 
{
}

void CWebData::OnDblclkWebData() 
{
	int  i, t;
	char *s;
	unCH tchar;
	unCH wbuf[256];
	unCH fname[FNSize];
	Boolean isDirectory;

	i = m_WebDataListControl.GetCurSel();
	if (i != LB_ERR) {
		m_WebDataListControl.GetText(i, wbuf);
		i = strlen(wbuf);
		if (wbuf[i-1] == '/' || uS.mStricmp(wbuf, PREVDIRLABLE) == 0)
			isDirectory = true;
		else
			isDirectory = false;

		if (uS.mStricmp(wbuf, PREVDIRLABLE)) {
			t = strlen(fulURLPath);
			tchar = 0;
			strcat(fulURLPath, wbuf);
		} else if (wpathcmp(fulURLPath, cl_T(ROOTWEBDIR))) {
			tchar = 0;
			t = strlen(fulURLPath) - 1;
			for (t--; t > 0 && fulURLPath[t] != '/'; t--) ;
			if (t > 0) {
				tchar = fulURLPath[t+1];
				fulURLPath[t+1] = EOS;
			} else
				t = strlen(fulURLPath);
		}

		u_strcpy(fname, prefsDir, FNSize);
		i = strlen(fulURLPath) - 1;
		if (fulURLPath[i] == '/')
			i--;
		for (; i >= 0 && fulURLPath[i] != '/'; i--) ;
		i++;
		strcat(fname, fulURLPath+i);
		i = strlen(fname) - 1;
		if (fname[i] == '/')
			fname[i] = EOS;

		if (curlURLDownloadToFile(fulURLPath, fname, 0L) == true) {
//		if (URLDownloadToFile(NULL, fulURLPath, fname, 0, NULL) == S_OK) {
			if (isDirectory)
				handleWebDirFile(fname);
			else {
				fulURLPath[t] = EOS;
				u_strcpy(templineC, fname, _MAX_PATH+_MAX_FNAME);
				s = strrchr(templineC, '\\');
				if (s != NULL) {
					strcpy(tFileBuf, s+1);
					nameOverride = tFileBuf;
					*(s+1) = EOS;
					pathOverride = templineC;
					getWebMedia = fulURLPath;
					::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW, NULL);
				}
			}
		} else {
			if (tchar != 0)
				fulURLPath[t+1] = tchar;
			else
				fulURLPath[t] = EOS;
			do_warning("Error downloading data from the web (6)", 0);
		}
	}
}

void CWebData::OnCancel() {
	CRect lpRect;

	ROOTWEBDIR = NULL;
	this->GetWindowRect(&lpRect);
	WebItemsWinSize.top = lpRect.top;
	WebItemsWinSize.left = lpRect.left;
	WebItemsWinSize.width = lpRect.right;
	WebItemsWinSize.height = lpRect.bottom;
	DestroyWindow();
	if (webDataDlg != NULL) {
		delete webDataDlg;
		webDataDlg = NULL;
	}
	URL_passwd[0] = EOS;
	WriteCedPreference();
}


void CWebData::OnDestroy() 
{
	CRect lpRect;

	ROOTWEBDIR = NULL;
	this->GetWindowRect(&lpRect);
	WebItemsWinSize.top = lpRect.top;
	WebItemsWinSize.left = lpRect.left;
	WebItemsWinSize.width = lpRect.right;
	WebItemsWinSize.height = lpRect.bottom;
	WriteCedPreference();
	CDialog::OnDestroy();	
	if (webDataDlg != NULL) {
		delete webDataDlg;
		webDataDlg = NULL;
	}
	URL_passwd[0] = EOS;
}
