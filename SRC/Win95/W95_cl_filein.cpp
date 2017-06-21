/*
	for (i=1; i <= F_numfiles; i++)
		get_a_file(i, line, nil, FileList);
*/
#include "cu.h"
#include "ced.h"
#include "CedDlgs.h"

struct f_list {
	char *fname;
	Boolean isDir;
	struct f_list *nextFile;
} ;

extern char accept_all_flag;

static int  ALLFLAG;
static int  lastfn;
static int  firstSelectFile;
static long lastWhen;
static Boolean dbClick;
static struct f_list *FileList = NULL;

int  F_numfiles = 0;
char isAllFile;

void InitFileDialog(void) {
	ALLFLAG = false;
	FileList = NULL;
	firstSelectFile = true;
	lastfn = 0;
	F_numfiles = 0;
	isAllFile = FALSE;
	dbClick = false;
	lastWhen = 0L;
}

static struct f_list *CleanupFileList(struct f_list *fl) {
	struct f_list *t;

	while (fl != NULL) {
		t = fl;
		fl = fl->nextFile;
		free(t->fname);
		free(t);
	}
	return(NULL);
}

static void get_a_file(int fnum, char *s, Boolean *isDir, struct f_list *fl) {	
	struct f_list *t;

	*s = EOS;
	if (fnum < 1 || fl == NULL)
		return;

	for (t=fl; t != NULL; fnum--, t=t->nextFile) {
		if (fnum == 1) {
			strncpy(s, t->fname, 1024-1);
			s[1024-1] = EOS;
			if (isDir != nil)
				*isDir = t->isDir;
			break;
		}
	}	
}

static struct f_list *add_a_file(char *s, Boolean isDir, struct f_list *fl, int *fn) {
	struct f_list *tw, *t, *tt;

	tw = NEW(struct f_list);
	if (tw == NULL)
		ProgExit("Out of memory");
	if (fl == NULL) {
		fl = tw;
		tw->nextFile = NULL;
	} else if (strcmp(fl->fname, s) > 0 || (!fl->isDir && isDir)) {
		tw->nextFile = fl;
		fl = tw;
	} else {
		t = fl;
		tt = fl->nextFile;
		if (tt != NULL) {
			if (!isDir && tt->isDir) {
				while (tt != NULL) {
					if (!tt->isDir)
						break;
					t = tt;
					tt = tt->nextFile;
				}
			}
		}
		while (tt != NULL) {
			if (strcmp(tt->fname, s) > 0 || (isDir && !tt->isDir))
				break; 
			t = tt;
			tt = tt->nextFile;
		}
		if (tt == NULL) {
			t->nextFile = tw;
			tw->nextFile = NULL;
		} else {
			tw->nextFile = tt;
			t->nextFile = tw;
		}
    }
	tw->fname = (char *)malloc((strlen(s)+1)*sizeof(char));
	if (tw->fname == NULL)
		ProgExit("Out of memory");
	strcpy(tw->fname, s);
	tw->isDir = isDir;
	(*fn)++;
	return(fl);
}

static struct f_list *remove_a_file(int fnum, struct f_list *fl, int *fn) {
	struct f_list *t, *tt;

	if (fl == NULL) ;
	else if (fnum == 1) {
		t = fl;
		fl = fl->nextFile;
		free(t->fname);
		free(t);
		(*fn)--;
	} else {
		tt = fl;
		t  = fl->nextFile;
		for (fnum--; t != NULL; fnum--) {
			if (fnum == 1) {
				tt->nextFile = t->nextFile;
				free(t->fname);
				free(t);
				(*fn)--;
			}
			tt = t;
			t = t->nextFile;
		}	
	}
	return(fl);
}

static int isDuplicate_File(char *fs, struct f_list *fl) {
	struct f_list *t;

	for (t=fl; t != NULL; t=t->nextFile) {
		if (!strcmp(t->fname, fs))
			return(TRUE);
	}
	return(FALSE);
}

void get_selected_file(int fnum, char *s) {
	get_a_file(fnum, s, nil, FileList);
}

static void getPath(CWnd *win, unCH *pathU, int max) {
	int i, len, cnt;
	unCH fnameU[FILENAME_MAX];

//	int plen = win->GetDlgItemText(1088, pathU, 1024);
//	pathU[plen] = EOS;
    pathU[0] = EOS;
	cnt = win->SendDlgItemMessage(1121, LB_GETCURSEL, 0, 0);
//	cnt = win->SendDlgItemMessage(1121, LB_GETCOUNT, 0, 0);
	for (i=0; i <= cnt; i++) {
		win->SendDlgItemMessage(1121, LB_GETTEXT, (WPARAM)i, (LPARAM)fnameU);
		len = strlen(pathU);
		if (len+strlen(fnameU)+1 >= max-1)
			return;
		if (pathU[0] != EOS && pathU[len-1] != PATHDELIMCHR)
			strcat(pathU, PATHDELIMSTR);
		strcat(pathU, fnameU);
	}
	return;
}

static UINT APIENTRY OFNHookProcOldStyle(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam) {
//	static LPOPENFILENAME   lpOpenFn;
//	static DWORD            dwFlags;
//	lpOpenFn = (LPOPENFILENAME)lParam;
//	dwFlags = lpOpenFn->lCustData; 

	int i;
	char *s, 
		 fname[FILENAME_MAX],
		 path[1024];
	wchar_t fnameU[FILENAME_MAX],
			pathU[1024];
	CWnd tw, *win;

	switch (uiMsg) {
		case WM_INITDIALOG:
			for (i=1; i <= F_numfiles; i++) {
				get_a_file(i, path, nil, FileList);
				s = strrchr(path,PATHDELIMCHR);
				if (!s)
					s = path;
				else
					s++;
				u_strcpy(pathU, s, 1024);
				SendDlgItemMessage(hdlg, IDC_LIST_OF_DATA, LB_ADDSTRING, 0, (LPARAM)pathU);
			}
			return(TRUE);
        case WM_COMMAND:
            switch (wParam) {
            	case IDCANCEL:
                	break;
            	case IDOK:
            		win = tw.FromHandle(hdlg);
            		if (win) {
						i = win->SendDlgItemMessage(1120, LB_GETCURSEL, 0, 0);
            			if (i != LB_ERR && !win->DlgDirSelect(fnameU, FILENAME_MAX, 1120)) {
							getPath(win, pathU, 1024);
							u_strcpy(path, pathU, 1024);
							u_strcpy(fname, fnameU, FILENAME_MAX);
							addFilename2Path(path, fname);
 							if (!isDuplicate_File(path, FileList)) {
								FileList = add_a_file(path, 0, FileList, &F_numfiles);
								win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_RESETCONTENT, 0, 0);
								for (i=1; i <= F_numfiles; i++) {
									get_a_file(i, path, nil, FileList);
									s = strrchr(path,PATHDELIMCHR);
									if (!s)
										s = path;
									else
										s++;
									u_strcpy(pathU, s, 1024);
									SendDlgItemMessage(hdlg, IDC_LIST_OF_DATA, LB_ADDSTRING, 0, (LPARAM)pathU);
								}
							}
            			}
            		}
            		return(TRUE);
            	case IDC_REMOVE:
            		win = tw.FromHandle(hdlg);
            		if (win) {
						i = win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_GETCURSEL, 0, 0);
						if (i != LB_ERR)
							FileList = remove_a_file(i+1, FileList, &F_numfiles);
						win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_RESETCONTENT, 0, 0);
						for (i=1; i <= F_numfiles; i++) {
							get_a_file(i, path, nil, FileList);
							s = strrchr(path,PATHDELIMCHR);
							if (!s)
								s = path;
							else
								s++;
							u_strcpy(pathU, s, 1024);
							SendDlgItemMessage(hdlg, IDC_LIST_OF_DATA, LB_ADDSTRING, 0, (LPARAM)pathU);
						}
						win->SetDlgItemText(IDC_SELECTED_FILENAME, cl_T(""));
 					}
 					break; 
            	case IDC_CLEAR:
            		win = tw.FromHandle(hdlg);
            		if (win) {
						FileList = CleanupFileList(FileList);
						F_numfiles = 0;
						win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_RESETCONTENT, 0, 0);
						win->SetDlgItemText(IDC_SELECTED_FILENAME, cl_T(""));
 					}
 					break; 
            	case IDC_DESKTOP:
					TCHAR szPath[MAX_PATH], tszPath[MAX_PATH], *tp, *ts, tc;
if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, szPath))) 
{
int err, cnt;
cnt = 0;
strcpy(tszPath, szPath);
ts = szPath;
win = tw.FromHandle(hdlg);
//win->UpdateData(TRUE);
//win->SendDlgItemMessage(1121, LB_RESETCONTENT, 0, 0);
//err = win->DlgDirList(szPath, 1121, 0, DDL_DIRECTORY | DDL_READWRITE);
//win->UpdateData(FALSE);
win->SendDlgItemMessage(1121, LB_RESETCONTENT, 0, 0);
do {
	tp = strchr(ts, PATHDELIMCHR);
	if (cnt == 0) {
		if (iswupper(ts[0]))
			ts[0] = towlower(ts[0]);
		if (tp != NULL) {
			tc = tp[1];
			tp[1] = EOS;
			win->SendDlgItemMessage(1121, LB_INSERTSTRING, cnt, (LPARAM)ts);
			tp[1] = tc;
		} else
			win->SendDlgItemMessage(1121, LB_INSERTSTRING, cnt, (LPARAM)ts);
	} else {
		if (tp != NULL)
			*tp = EOS;
		win->SendDlgItemMessage(1121, LB_INSERTSTRING, cnt, (LPARAM)ts);
	}
	if (tp != NULL)
		ts = tp + 1;
	else
		break;
	cnt++;
} while (1) ;
err = win->SendDlgItemMessage(1121, LB_DIR, (WPARAM)0, (LPARAM)tszPath);
//err = win->SendDlgItemMessage(1121, LB_SETCURSEL, cnt, 0);
//err = win->SendDlgItemMessage(1121, LB_SELECTSTRING, cnt, (LPARAM)ts);
return(TRUE);
}

 					break; 
            	case IDC_ADD_ALL:
            		win = tw.FromHandle(hdlg);
            		if (win) {
						int cnt, len, plen;
						getPath(win, pathU, 1024);
						u_strcpy(path, pathU, 1024);
						plen = strlen(path);
						if (*path != EOS) {
            				strcat(path, PATHDELIMSTR);
							plen++;
						}
						cnt = win->SendDlgItemMessage(1120, LB_GETCOUNT, 0, 0);
						if (cnt != LB_ERR) {
							for (i=0; i < cnt; i++) {
								len = win->SendDlgItemMessage(1120, LB_GETTEXT, i, (LPARAM)fnameU);
								u_strcpy(fname, fnameU, FILENAME_MAX);
								if (len != LB_ERR) {
									path[plen] = EOS;
            						strcat(path, fname);
									if (!isDuplicate_File(path, FileList)) {
										FileList = add_a_file(path, 0, FileList, &F_numfiles);
									}
								}
							}
							win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_RESETCONTENT, 0, 0);
							for (i=1; i <= F_numfiles; i++) {
								get_a_file(i, path, nil, FileList);
								s = strrchr(path,PATHDELIMCHR);
								if (!s)
									s = path;
								else
									s++;
								u_strcpy(pathU, s, 1024);
								win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_ADDSTRING, 0, (LPARAM)pathU);
							}
							win->SetDlgItemText(IDC_SELECTED_FILENAME, cl_T(""));
						}
 					}
 					break; 

		        default:
					if (LOWORD(wParam) != IDC_LIST_OF_DATA) {
						if (LOWORD(wParam) == 1120 && HIWORD(wParam) == LBN_SELCHANGE) {
		            		win = tw.FromHandle(hdlg);
		            		if (win) {
            					if (!win->DlgDirSelect(fnameU, 1120)) {
									getPath(win, pathU, 1024);
            						if (*pathU != EOS)
            							strcat(pathU, PATHDELIMSTR);
            						strcat(pathU, fnameU);
									win->SetDlgItemText(IDC_SELECTED_FILENAME, pathU);
            					}
		 					}
						} else if (LOWORD(wParam) == 1121 && HIWORD(wParam) == LBN_SELCHANGE) {
		            		win = tw.FromHandle(hdlg);
						}
					} else
					switch (HIWORD(wParam)) {
						case LBN_SELCHANGE:
		            		win = tw.FromHandle(hdlg);
		            		if (win) {
								i = win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_GETCURSEL, 0, 0);
								if (i != LB_ERR) {
									get_a_file(i+1, path, nil, FileList);
									u_strcpy(pathU, path, 1024);
									win->SetDlgItemText(IDC_SELECTED_FILENAME, pathU);
								}	
		 					}
							break;
						case LBN_DBLCLK:
		            		win = tw.FromHandle(hdlg);
		            		if (win) {
								i = win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_GETCURSEL, 0, 0);
								if (i != LB_ERR)
									FileList = remove_a_file(i+1, FileList, &F_numfiles);
								win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_RESETCONTENT, 0, 0);
								for (i=1; i <= F_numfiles; i++) {
									get_a_file(i, path, nil, FileList);
									s = strrchr(path,PATHDELIMCHR);
									if (!s)
										s = path;
									else
										s++;
									u_strcpy(pathU, s, 1024);
									win->SendDlgItemMessage(IDC_LIST_OF_DATA, LB_ADDSTRING, 0, (LPARAM)pathU);
								}
								win->SetDlgItemText(IDC_SELECTED_FILENAME, cl_T(""));
		 					}
							break;
						case LBN_SELCANCEL:
		            		win = tw.FromHandle(hdlg);
		            		if (win) {
								win->SetDlgItemText(IDC_SELECTED_FILENAME, cl_T(""));
		 					}
							break;
					}
		            break;
         	}
			break;
        default:
            break;
    }
	return(FALSE);
}

void myget(void) {
    OPENFILENAME	ofn;
    unCH			szFile[1024];
	char			tDir[1024];
    unCH			*szFilter;
	wchar_t			wDirPathName[FNSize];

	strcpy(tDir, wd_dir);
	if (tDir[strlen(tDir)-1] == PATHDELIMCHR)
		tDir[strlen(tDir)-1] = EOS;
	szFilter = _T("CHAT files (*.cha,*.cex)\0*.cha;*.cex\0All files (*.*)\0*.*\0\0");
    strcpy(szFile, cl_T(""));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = 1024;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = u_strcpy(wDirPathName, tDir, UTTLINELEN);
    ofn.lpstrTitle = cl_T("Please select files");
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lpfnHook = OFNHookProcOldStyle;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENDIR);
	ofn.hInstance = AfxGetInstanceHandle();

// Place our flags in the lCustData parameter
//	DWORD          dwLocalOpt;
//	of.lCustData = dwLocalOpt;

	GetOpenFileName(&ofn);
}
/*
LPITEMIDLIST WINAPI SHBrowseForFolder(
  LPBROWSEINFO lpbi
);

SHBrowseForFolder

CFileDialog
CDN_FOLDERCHANGE
*/