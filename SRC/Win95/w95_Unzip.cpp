// Unzip.cpp : implementation file
//

#include "ced.h"
#include "cu.h"
#include "W95_commands.h"
#include "W95_Unzip.h"

extern FNType home_dir[];
extern FNType prefsDir[];

extern void del_dir(FNType *pathO, char *wd_path);
extern bool curlURLDownloadToFile(unCH *fulURLPath, unCH *fname, size_t isProgres);

static CUnzip *UnzipDlg = NULL;


bool UnZipFolder(LPCTSTR zipFile, LPCTSTR destination) {
	HRESULT	hResult;
	IShellDispatch *pISD;
	Folder	*pToFolder = NULL;
	Folder	*pFromFolder = NULL;
	FolderItems *pFolderItems = NULL;
	FolderItem *pItem = NULL;

	VARIANT   vDir, vFile, vOpt;
	BSTR  strptr1, strptr2;

	bool bReturn = false;

	if (UnzipDlg == NULL) {
		UnzipDlg = new CUnzip(AfxGetApp()->m_pMainWnd);
		if (UnzipDlg != NULL)
			UnzipDlg->Create(IDD_UNZIP);
	} else
		UnzipDlg->SetActiveWindow();

	hResult = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD);

	if (FAILED(hResult)) {
		if (UnzipDlg != NULL)
			UnzipDlg->OnCancel();
		return bReturn;
	}

	CString strZipFile(zipFile);
	CString strDestination(destination);
	strptr1 = strZipFile.AllocSysString();
	strptr2 = strDestination.AllocSysString();

	VariantInit(&vFile);
	vFile.vt = VT_BSTR;
	vFile.bstrVal = strptr1;
	hResult = pISD->NameSpace(vFile, &pFromFolder);

	VariantInit(&vDir);
	vDir.vt = VT_BSTR;
	vDir.bstrVal = strptr2;

	hResult = pISD->NameSpace(vDir, &pToFolder);

	if (S_OK == hResult) {
		hResult = pFromFolder->Items(&pFolderItems);
		if (SUCCEEDED(hResult)) {
			long lCount = 0;
			pFolderItems->get_Count(&lCount);
			IDispatch* pDispatch = NULL;
			pFolderItems->QueryInterface(IID_IDispatch, (void**)&pDispatch);
			VARIANT vtDispatch;
			VariantInit(&vtDispatch);
			vtDispatch.vt = VT_DISPATCH;
			vtDispatch.pdispVal = pDispatch;

			VariantInit(&vOpt);
			vOpt.vt = VT_I4;
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb787866(v=vs.85).aspx
			vOpt.lVal = 16 | 4; // 16 + 4;     // Do not display a progress dialog box ~ This will not work properly!
			//cout << "Extracting files ...\n";
			hResult = pToFolder->CopyHere(vtDispatch, vOpt);
			if (hResult != S_OK) {
				if (UnzipDlg != NULL)
					UnzipDlg->OnCancel();
				return false;
			}

			//Cross check and wait until all files are unzipped!
			FolderItems* pToFolderItems;
			hResult = pToFolder->Items(&pToFolderItems);

			if (S_OK == hResult) {
				long lCount2 = 0;

				hResult = pToFolderItems->get_Count(&lCount2);
				if (S_OK != hResult) {
					pFolderItems->Release();
					pToFolderItems->Release();
					SysFreeString(strptr1);
					SysFreeString(strptr2);
					pISD->Release();
					if (UnzipDlg != NULL)
						UnzipDlg->OnCancel();
					return false;
				}
				//Use this code in a loop if you want to cross-check the items unzipped.
/*
				if(lCount2 != lCount) {
					pFolderItems->Release();
					pToFolderItems->Release();
					SysFreeString(strptr1);
					SysFreeString(strptr2);
					pISD->Release();
//					CoUninitialize();
					if (UnzipDlg != NULL)
						UnzipDlg->OnCancel();
					return false;
				}
*/
				bReturn = true;
			}

			pFolderItems->Release();
			pToFolderItems->Release();
		}

		pToFolder->Release();
		pFromFolder->Release();
	}

	//cout << "Over!\n";
	SysFreeString(strptr1);
	SysFreeString(strptr2);
	pISD->Release();

//	CoUninitialize();
	if (UnzipDlg != NULL)
		UnzipDlg->OnCancel();
	return bReturn;
}

//////////////////////////////////////////////////////////////
// CUnzip dialog

CUnzip::CUnzip(CWnd* pParent /*=NULL*/)
	: CDialog(CUnzip::IDD, pParent)
{

}

CUnzip::~CUnzip()
{
}

void CUnzip::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUnzip, CDialog)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUnzip message handlers

BOOL CUnzip::DestroyWindow() {
	BOOL res = CDialog::DestroyWindow();
	if (res) {
		delete this;
		UnzipDlg = NULL;
	}
	return res;
}

void CUnzip::OnCancel() {
	DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// MOR Grammar BEG
static bool DownloadGrammar(char *grName, char *destName, size_t fileSize) {
	int len, i;
	char *s;
	unCH URLPath[512];
	unCH fname[FNSize];
	FILE *fp;

	//CSIDL_COMMON_DESKTOPDIRECTORY
	//CSIDL_DESKTOPDIRECTORY
	HWND m_hWnd = AfxGetApp()->m_pMainWnd->m_hWnd;
	destName[0] = EOS;
	if (SHGetSpecialFolderPath(m_hWnd, fname, CSIDL_DESKTOPDIRECTORY, FALSE)) {
		u_strcpy(destName, fname, FNSize);
	}
	if (destName[0] != EOS) {
		if (chdir(destName)) {
			destName[0] = EOS;
		} else {
			addFilename2Path(destName, "MOR");
			if (chdir(destName)) {
				if (my_mkdir(destName, 0)) {
					destName[0] = EOS;
				}
			}
		}
	}
	if (destName[0] == EOS) {
		if (SHGetSpecialFolderPath(m_hWnd, fname, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE)) {
			u_strcpy(destName, fname, FNSize);
		}
		if (destName[0] != EOS) {
			if (chdir(destName)) {
				destName[0] = EOS;
			} else {
				addFilename2Path(destName, "MOR");
				if (chdir(destName)) {
					if (my_mkdir(destName, 0)) {
						destName[0] = EOS;
					}
				}
			}
		}
	}
	if (destName[0] == EOS) {
		strcpy(destName, prefsDir);
	}

//	strcpy(destName, home_dir);
//	strcpy(destName, prefsDir);
	len = strlen(destName);
	addFilename2Path(destName, grName);
	strcat(destName, ".zip");
	fp = fopen(destName, "w");
	if (fp == NULL) {
		strcpy(destName, mor_lib_dir);
		i = strlen(destName) - 1;
		if (destName[i] == PATHDELIMCHR)
			destName[i] = EOS;
		s = strrchr(destName, PATHDELIMCHR);
		if (s != NULL) {
			*(s + 1) = EOS;
			len = strlen(destName);
			addFilename2Path(destName, grName);
			strcat(destName, ".zip");
			fp = fopen(destName, "w");
		}
	}
	if (fp != NULL) {
		fclose(fp);
		unlink(destName);
		u_strcpy(URLPath, "https://talkbank.org/morgrams/", 512);
		strcat(URLPath, grName);
		strcat(URLPath, ".zip");
		u_strcpy(fname, destName, UTTLINELEN);
		if (curlURLDownloadToFile(URLPath, fname, fileSize) == true) {
			destName[len] = EOS;
			return(true);
		} else {
			unlink(destName);
		}
	}
	destName[len] = EOS;
	return(false);
}

void GetMORGrammar(char *grammar, size_t fileSize) {
	int  i;
	unCH zipFName[FNSize], unzipFolder[FNSize];

	if (DownloadGrammar(grammar, DirPathName, fileSize) == true) {
// delete old grammar location first
		strcpy(FileName1, home_dir);
		addFilename2Path(FileName1, grammar);
		strcat(FileName1, ".zip");
		unlink(FileName1);
		strcpy(FileName1, home_dir);
		addFilename2Path(FileName1, grammar);
		strcat(FileName1, PATHDELIMSTR);
		del_dir(FileName1, wd_dir);
// delete old grammar location first

		i = strlen(DirPathName) - 1;
/*
		strcpy(FileName1, "rmdir /s /q \"");
		strcat(FileName1, DirPathName);
		addFilename2Path(FileName1, grammar);
		strcat(FileName1, "\"");
		system(FileName1);

*/
// BEG delete current MOR directory
		strcpy(FileName1, DirPathName);
		addFilename2Path(FileName1, grammar);
		strcat(FileName1, PATHDELIMSTR);
		del_dir(FileName1, wd_dir);
// END delete current MOR directory

		u_strcpy(zipFName, DirPathName, FNSize);
		if (DirPathName[i] != '\\')
			strcat(zipFName, "\\");
		strcat(zipFName, grammar);
		strcat(zipFName, ".zip");
		u_strcpy(unzipFolder, DirPathName, FNSize);
		if (UnZipFolder(zipFName, unzipFolder) == true) {
			strcpy(FileName1, DirPathName);
			addFilename2Path(FileName1, grammar);
			strcat(FileName1, ".zip");
			unlink(FileName1);
			strcpy(mor_lib_dir, DirPathName);
			addFilename2Path(mor_lib_dir, grammar);
			strcat(mor_lib_dir, PATHDELIMSTR);
			if (clanDlg != NULL) {
				u_strcpy(clanDlg->t_st, mor_lib_dir, FNSize);
				AdjustName(clanDlg->mor_lib_st, clanDlg->t_st, 39);
				clanDlg->m_morLibSt = clanDlg->mor_lib_st;
				clanDlg->UpdateData(FALSE);
			}
		} else {
			do_warning("Can't unzip grammar archive. Please try to download it from URL \"https://talkbank.org/morgrams/\" and install it by hand.", 0);
		}
	} else {
		do_warning("Can't download MOR grammar. Please try to download it from URL \"https://talkbank.org/morgrams/\" and install it by hand.", 0);
	}
}
// MOR Grammar END

// KIDEVAL DB BEG
static bool getPreinstalledDatabase(const char *database, char *dirPath, unCH *zipFName, unCH *unzipFolder) {
	char isFoundZip;

	isFoundZip = FALSE;
	strcpy(dirPath, home_dir);
	addFilename2Path(dirPath, "lib\\kideval");
	addFilename2Path(dirPath, database);
	strcat(dirPath, ".zip");
	if (access(dirPath, 0) == 0)
		isFoundZip = TRUE;
	else {
		strcpy(dirPath, lib_dir);
		addFilename2Path(dirPath, "kideval");
		addFilename2Path(dirPath, database);
		strcat(dirPath, ".zip");
		if (access(dirPath, 0) == 0)
			isFoundZip = TRUE;
	}
	if (isFoundZip) {
		strcpy(FileName1, prefsDir);
		addFilename2Path(FileName1, database);
		unlink(FileName1);
		strcpy(FileName1, prefsDir);
		u_strcpy(zipFName, dirPath, FNSize);
		u_strcpy(unzipFolder, FileName1, FNSize);
		if (UnZipFolder(zipFName, unzipFolder)) {
//			unlink(dirPath);
/* DOWNLOAD DB
			strcpy(dirPath, prefsDir);
			addFilename2Path(dirPath, database);
			strcat(dirPath, ".1");
			unlink(dirPath);
			addFilename2Path(FileName1, database);
			if (rename(FileName1, dirPath) == 0)
				return(TRUE);
*/
		} else {
			do_warning("Can't unzip grammar archive. Please try to download and install it by hand.", 0);
		}
	}
	return(FALSE);
}

/* DOWNLOAD DB
static void compareDatesAndChooseOne(const char *database) {
	char dateOne[256], *s;
	FILE *fp;

	dateOne[0] = EOS;
	strcpy(FileName1, prefsDir);
	addFilename2Path(FileName1, database);
	strcat(FileName1, ".1");
	if ((fp = fopen(FileName1, "r")) != NULL) {
		if (fgets_cr(templineC, UTTLINELEN, fp)) {
			strncpy(dateOne, templineC, 256);
			dateOne[255] = EOS;
			s = strchr(dateOne, ',');
			if (s != NULL)
				*s = EOS;
		}
		fclose(fp);
	}
	if (dateOne[0] != EOS) {
		strcpy(FileName2, prefsDir);
		addFilename2Path(FileName2, database);
		templineC[0] = EOS;
		if ((fp = fopen(FileName2, "r")) != NULL) {
			if (fgets_cr(templineC, UTTLINELEN, fp)) {
				s = strchr(templineC, ',');
				if (s != NULL)
					*s = EOS;
			}
			fclose(fp);
		}
		if (templineC[0] != EOS) {
			if (strcmp(dateOne, templineC) > 0) {
				unlink(FileName2);
				rename(FileName1, FileName2);
			} else {
				unlink(FileName1);
			}
		}
	}
}

static bool DownloadDatabase(const char *database, char *destName, size_t fileSize) {
	int len;
	unCH URLPath[512];
	unCH fname[FNSize];
	FILE *fp;

	strcpy(destName, prefsDir);
	len = strlen(destName);
	addFilename2Path(destName, database);
	strcat(destName, ".zip");
	fp = fopen(destName, "w");
	if (fp != NULL) {
		fclose(fp);
		unlink(destName);
		u_strcpy(URLPath, "https://talkbank.org/clan/kideval-db/", 512);
		strcat(URLPath, database);
		strcat(URLPath, ".zip");
		u_strcpy(fname, destName, UTTLINELEN);
		if (curlURLDownloadToFile(URLPath, fname, fileSize) == true) {
			destName[len] = EOS;
			return(true);
		} else {
			unlink(destName);
		}
	}
	destName[len] = EOS;
	return(false);
}
*/
void GetKidevalDB(const char *database, size_t fileSize) {
//	char res;
//	int i;
//	unCH zipFName[FNSize], unzipFolder[FNSize];

	do_warning("Done.", 0);
//	res = getPreinstalledDatabase(database, DirPathName, zipFName, unzipFolder);

/* DOWNLOAD DB
	if (DownloadDatabase(database, DirPathName, fileSize) == true) {
		i = strlen(DirPathName) - 1;
		strcpy(FileName1, lib_dir);
		addFilename2Path(FileName1, "kideval");
		addFilename2Path(FileName1, database);
		unlink(FileName1);
		strcpy(FileName1, DirPathName);
		addFilename2Path(FileName1, "eng_kideval_db.cut");
		unlink(FileName1);
		strcpy(FileName1, DirPathName);
		addFilename2Path(FileName1, database);
		unlink(FileName1);
		u_strcpy(zipFName, DirPathName, FNSize);
		if (DirPathName[i] != '\\')
			strcat(zipFName, "\\");
		strcat(zipFName, database);
		strcat(zipFName, ".zip");
		u_strcpy(unzipFolder, DirPathName, FNSize);
		if (UnZipFolder(zipFName, unzipFolder) == true) {
			strcpy(FileName1, DirPathName);
			addFilename2Path(FileName1, database);
			strcat(FileName1, ".zip");
			unlink(FileName1);
			if (res)
				compareDatesAndChooseOne(database);
		} else {
			do_warning("Can't unzip grammar archive. Please try to download and install it by hand.", 0);
		}
	} else {
		do_warning("Can't download KIDEVAL database. Please try later.", 0);
	}
*/
}
// KIDEVAL DB END
