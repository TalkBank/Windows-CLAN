#include "ced.h"
#include "c_clan.h"
#include "CedDlgs.h"

void DoDefContStrings(void) {
	CDefineMacro dlg;

	dlg.m_Number = 1;
	dlg.m_String = _T("");
	if (dlg.DoModal() == IDOK) {
		if (dlg.m_Number == 0)
			dlg.m_Number = 10;
		if (dlg.m_Number > 0 && dlg.m_Number <= 10) {
			u_strcpy(ced_lineC, dlg.m_String, UTTLINELEN);
			AllocConstString(ced_lineC, dlg.m_Number-1);
		}
	}
}

int QueryDialog(const char *st, short id) {
	int			res;

	u_strcpy(templine4, st, UTTLINELEN);
	if (id == 140)
		res = AfxMessageBox(templine4, MB_YESNOCANCEL, 0);
	else if (id == 147)
		res = AfxMessageBox(templine4, MB_YESNO, 0);
	else
		return(0);
	if (res == IDNO)
		res = -1;
	else if (res == IDYES)
		res = 1;
	else
		res = 0;
	return(res);
}

/////////////////////////////////////////////////////////////////////////////
// Locate Directory dialog
int LocateDir(const char *prompt, char *currentDir, char noDefault) {
    OPENFILENAME	ofn;
    unCH			szFile[FILENAME_MAX];
	char			tDir[FILENAME_MAX];
    unCH			*szFilter;
	wchar_t			wDirPathName[FNSize];

	strcpy(tDir, currentDir);
	if (tDir[strlen(tDir)-1] == PATHDELIMCHR)
		tDir[strlen(tDir)-1] = EOS;
	szFilter = _T("All files (*.*)\0*.*\0\0");
    strcpy(szFile, "Some");
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = u_strcpy(wDirPathName, tDir, UTTLINELEN);
    ofn.lpstrTitle = u_strcpy(templine1, prompt, UTTLINELEN);
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_ENABLETEMPLATE;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_LOCATE_DIR);
	ofn.hInstance = AfxGetInstanceHandle();

// Place our flags in the lCustData parameter
//	DWORD          dwLocalOpt;
//	of.lCustData = dwLocalOpt;

	if (GetSaveFileName(&ofn) == 0) {
		return(FALSE);
	}
	szFile[ofn.nFileOffset] = EOS;
	u_strcpy(currentDir, szFile, FILENAME_MAX);
	if (currentDir[strlen(currentDir)-1] != PATHDELIMCHR)
		strcat(currentDir, PATHDELIMSTR);
	return(TRUE);
}
