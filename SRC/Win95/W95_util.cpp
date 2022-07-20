#include "cu.h"
#include "ced.h"
#include "mmedia.h"
#include "c_clan.h"
#include "w95_commands.h"

extern void del_dir(FNType *pathO, char *wd_path);

static long  NumberOfReads = 0L;
static WPARAM lastKey;

short winEncod;

long UnicodeToUTF8(const unCH *UniStr, unsigned long actualStringLength, unsigned char *UTF8str, unsigned long *actualUT8Len, unsigned long MaxLen) {
	long uchars=WideCharToMultiByte(CP_UTF8,0,UniStr,actualStringLength,NULL,0,NULL,NULL);
	if (uchars > MaxLen) {
		if (actualUT8Len != NULL)
			*actualUT8Len = 0L;
		UTF8str[0] = '\0';
		return(1L);
	}
	WideCharToMultiByte(CP_UTF8,0,UniStr,actualStringLength,(char *)UTF8str,uchars,NULL,NULL);
	UTF8str[uchars] = '\0';
	if (actualUT8Len != NULL)
		*actualUT8Len = uchars;
	return(0L);
}

long UTF8ToUnicode(unsigned char *UTF8str, unsigned long actualUT8Len, unCH *UniStr, unsigned long *actualUnicodeLength, unsigned long MaxLen) {
	long wchars=MultiByteToWideChar(CP_UTF8,0,(const char*)UTF8str,actualUT8Len,NULL,0);
	if (wchars > MaxLen) {
		if (actualUnicodeLength != NULL)
			*actualUnicodeLength = 0L;
		UniStr[0] = 0;
		return(1L);
	}
	int err = MultiByteToWideChar(CP_UTF8,0/*MB_ERR_INVALID_CHARS*/,(const char*)UTF8str,actualUT8Len,UniStr,wchars);
	DWORD errnum;

	if (err == 0 && actualUT8Len != 0L)
		errnum = GetLastError();
	else
		errnum = 0L;

	UniStr[wchars] = 0;
	if (actualUnicodeLength != NULL)
		*actualUnicodeLength = wchars;
	return(errnum);
}

long UnicodeToANSI(unCH *UniStr, unsigned long actualStringLength, unsigned char *ANSIstr, unsigned long *actualANSILen, unsigned long MaxLen, short script) {
	long uchars=WideCharToMultiByte(script,0,UniStr,actualStringLength,NULL,0,NULL,NULL);
	if (uchars > MaxLen) {
		if (actualANSILen != NULL)
			*actualANSILen = 0L;
		ANSIstr[0] = '\0';
		return(1L);
	}
	WideCharToMultiByte(script,0,UniStr,actualStringLength,(char *)ANSIstr,uchars,NULL,NULL);
	ANSIstr[uchars] = '\0';
	if (actualANSILen != NULL)
		*actualANSILen = uchars;
	return(0L);
}

long ANSIToUnicode(unsigned char *ANSIstr, unsigned long actualANSILen, unCH *UniStr, unsigned long *actualStringLength, unsigned long MaxLen, short script) {
	unsigned long wchars=MultiByteToWideChar(script,0,(const char*)ANSIstr,actualANSILen,NULL,0);
	if (wchars > MaxLen) {
		if (actualStringLength != NULL)
			*actualStringLength = 0L;
		return(1L);
	}
	int err = MultiByteToWideChar(script,MB_ERR_INVALID_CHARS,(const char*)ANSIstr,actualANSILen,UniStr,wchars);
	DWORD errnum;

	if (err == 0 && wchars != 0L)
		errnum = GetLastError();
	else
		errnum = 0L;

	UniStr[wchars] = 0;
	if (actualStringLength != NULL)
		*actualStringLength = wchars;
	return(errnum);
}

long UTF8ToANSI(unsigned char *UTF8str, unsigned long actualUT8Len, unsigned char *ANSIstr, unsigned long *actualANSILen, unsigned long MaxLen, short script) {
	unsigned long wchars=MultiByteToWideChar(CP_UTF8,0,(const char*)UTF8str,actualUT8Len,NULL,0);
//	LPVOID hwText = LocalAlloc(LMEM_MOVEABLE, (wchars+1)*2);
//	LPWSTR lpwText = (LPWSTR)LocalLock(hwText);
//	int err = MultiByteToWideChar(CP_UTF8,0,(const char*)UTF8str,actualUT8Len,lpwText,wchars);
	int err = MultiByteToWideChar(CP_UTF8,0,(const char*)UTF8str,actualUT8Len,templineW1,wchars);
	DWORD errnum;

	if (err == 0 && actualUT8Len != 0L)
		errnum = GetLastError();
	else
		errnum = 0L;

	if (errnum == 0L) {
//		errnum = UnicodeToANSI(lpwText, wchars, ANSIstr, actualANSILen, MaxLen, script);
		errnum = UnicodeToANSI(templineW1, wchars, ANSIstr, actualANSILen, MaxLen, script);
	}

//	LocalUnlock(hwText);
//	HLOCAL LocalFree(hwText);
	return(errnum);
}

long ANSIToUTF8(unsigned char *ANSIstr, unsigned long actualANSILen, unsigned char *UTF8str, unsigned long *actualUT8Len, unsigned long MaxLen, short script) {
	unsigned long wchars=MultiByteToWideChar(script,0,(const char*)ANSIstr,actualANSILen,NULL,0);
//	LPVOID hwText = LocalAlloc(LMEM_MOVEABLE, (wchars+1)*2);
//	LPWSTR lpwText = (LPWSTR)LocalLock(hwText);
//	int err = MultiByteToWideChar(script,MB_ERR_INVALID_CHARS,(const char*)ANSIstr,actualANSILen,lpwText,wchars);
	int err = MultiByteToWideChar(script,MB_ERR_INVALID_CHARS,(const char*)ANSIstr,actualANSILen,templineW1,wchars);
	DWORD errnum;

	if (err == 0 && wchars != 0L)
		errnum = GetLastError();
	else
		errnum = 0L;

	if (errnum == 0L) {
//		errnum = UnicodeToUTF8(lpwText, wchars, UTF8str, actualUT8Len, MaxLen);
		errnum = UnicodeToUTF8(templineW1, wchars, UTF8str, actualUT8Len, MaxLen);
	}

//	LocalUnlock(hwText);
//	HLOCAL LocalFree(hwText);
	return(errnum);
}


/////////////////////////////////////////////////////////////////////////////
// DEL DIR BEG
static void recursive_rm(unCH *path) {
	int  res;
	int  dPos;
	BOOL notDone;
	unCH tpath[FNSize];
	CFileFind fileFind, dirFind;
	CString fname;

	dPos = strlen(path);
	res = _wchdir(path);
	if (res)
		return;

	notDone = fileFind.FindFile(_T("*.*"), 0);
	while (notDone) {
		notDone = fileFind.FindNextFile();
		if (fileFind.IsDirectory())
			continue;
		fname = fileFind.GetFileName();
		res = _wremove(fname);
	}
	fileFind.Close();

	notDone = dirFind.FindFile(cl_T("*.*"), 0);
	while (notDone) {
		notDone = dirFind.FindNextFile();
		if (!dirFind.IsDirectory())
			continue;
		fname = dirFind.GetFileName();
		if (!strcmp(fname, ".") || !strcmp(fname, ".."))
			continue;
		strcat(path, fname);
		strcat(path, PATHDELIMSTR);
		recursive_rm(path);
		strcpy(tpath, path);
		path[dPos] = EOS;
		res = _wchdir(path);
		res = _wrmdir(tpath);
	}
	dirFind.Close();
}

void del_dir(FNType *pathO, char *wd_path) {
	int res;
	unCH path[FNSize];

	u_strcpy(path, pathO, FNSize);
	recursive_rm(path);
	SetNewVol(wd_path);
	res = _wrmdir(path);
}
// DEL DIR END

void do_warning(const char *str, int delay) {
	if (GlobalDC)
		DrawCursor(1);
	u_strcpy(templineW, str, UTTLINELEN);
	AfxGetApp()->m_pMainWnd->MessageBox(templineW, NULL, MB_ICONWARNING);
	if (GlobalDC)
		DrawCursor(0);
}

void ProgExit(const char *err) {	
	if (err != NULL)
		do_warning(err, 0);
//	CloseAllWindows();
	exit(1);
}

char getFileDate(FNType *fn, UInt32 *date) {
	CTime rt;
	CFileFind fileFind;
	unCH wDirPathName[FNSize];

	u_strcpy(wDirPathName, fn, FNSize);
	if (fileFind.FindFile(wDirPathName, 0)) {
		fileFind.FindNextFile();
		if (fileFind.GetLastWriteTime(rt)) {
			*date = rt.GetTime();
			fileFind.Close();
			return(TRUE);
		} else
			*date = 0L;
		fileFind.Close();
	} else
		*date = 0L;
	
	return(FALSE);
}

void setFileDate(FNType *fn, UInt32 date) {
}

void cd(char *new_dir) {
	FNType tref[FNSize];
	extern void my_fprintf(FILE * file, const char * format, ...);

	SetNewVol(wd_dir);
	if (!chdir(new_dir)) {
		strcpy(tref, wd_dir);
		getcwd(wd_dir, FILENAME_MAX);
		if (wd_dir[strlen(wd_dir)-1] != PATHDELIMCHR)
			strcat(wd_dir, PATHDELIMSTR);
		fprintf(stderr,"directory set %s\n",wd_dir);
		if (pathcmp(od_dir, tref) == 0) {
			strcpy(od_dir, wd_dir);
		}

		if (clanDlg != NULL) {
			if (pathcmp(od_dir, wd_dir) == 0)
				clanDlg->m_OdSt = "";
			else {
				u_strcpy(clanDlg->t_st, od_dir, FNSize);
				AdjustName(clanDlg->od_st, clanDlg->t_st, 39);
				clanDlg->m_OdSt = clanDlg->od_st;
			}
			u_strcpy(clanDlg->t_st, wd_dir, FNSize);
			AdjustName(clanDlg->wd_st, clanDlg->t_st, 39);
			clanDlg->m_WdSt = clanDlg->wd_st;
			clanDlg->UpdateData(FALSE);
		}
		WriteCedPreference();
	} else
		fprintf(stderr,"change directory error %s\n", new_dir);
}

short my_FontToScript(char *fName, int CharSet) {
	if (winEncod != 0)
		return(winEncod);
	if (uS.partwcmp(fName,"Arial Unicode") && (CharSet == 134 || CharSet == 136))
		return(CSCRIPT);
	if (uS.partwcmp(fName,"Arial Unicode") && CharSet == 128)
		return(JSCRIPT);
	if (JP_FONTS(fName))
		return(JSCRIPT);
	if (CH_FONTS(fName))
		return(CSCRIPT);

	return(NSCRIPT);
}

short my_CharacterByteType(const char *org, short pos, NewFontInfo *finfo) {
	short i;
	short cType;

	if (finfo->isUTF) {
		cType = 2;
/*
		for (i=0; i <= pos; i++) {
			if (IsDBCSLeadByteEx(950,(BYTE)org[i]) == 0) {
				DWORD t = GetLastError();
				if (cType == -1) {
					cType = 1;
				} else
					cType = 0;
			} else
				cType = -1;
		}
*/
		if (UTF8_IS_SINGLE((unsigned char)org[pos]))
			cType = 0;
		else if (UTF8_IS_LEAD((unsigned char)org[pos]))
			cType = -1;
		else if (UTF8_IS_TRAIL((unsigned char)org[pos])) {
			if (!UTF8_IS_TRAIL((unsigned char)org[pos+1]))
				cType = 1;
			else
				cType = 2;
		}

		return(cType);
	} else
	if (finfo->Encod == JSCRIPT || finfo->Encod == CSCRIPT) {
		cType = 0;
		for (i=0; i <= pos; i++) {
			if (org[i] >= 0 && org[i] < 128 && cType == -1)
				cType = 1;
			else if (org[i] >= 0 && org[i] < 128)
				cType = 0;
			else if (cType == 0 || cType == 1)
				cType = -1;
			else
				cType = 1;
		}
		return(cType);
	}
	return(0);
}

short my_wCharacterByteType(short *org, short pos, short encod) {
	short i;
	short cType;

	if (encod == JSCRIPT || encod == CSCRIPT) {
		cType = 0;
		for (i=0; i <= pos; i++) {
			if (org[i] >= 0 && org[i] < 128 && cType == -1)
				cType = 1;
			else if (org[i] >= 0 && org[i] < 128)
				cType = 0;
			else if (cType == 0 || cType == 1)
				cType = -1;
			else
				cType = 1;
		}
		return(cType);
	}
	return(0);
}

void SysEventCheck(long reps) {
	myFInfo *tglobal_df;
	MSG msg;

    if (NumberOfReads < GetTickCount()) {
		NumberOfReads = GetTickCount() + reps;
		tglobal_df = global_df;
		if (PeekMessage(&msg,AfxGetApp()->m_pMainWnd->m_hWnd,0,0,PM_REMOVE)) {
			switch(msg.message) { 
				case WM_KEYDOWN:
					if (lastKey==17 && (msg.wParam==67 || msg.wParam==190)) {
						isKillProgram = 1;
					}
					lastKey = msg.wParam;
					break;
			} 
		}	
		global_df = tglobal_df;
    }
}
/*
void CheckForInterupt(void) {
	SysEventCheck(20);
}
*/
/* temp */

void Quit_CED(int num) {
	if (num != 1999)
		do_warning("Pausing for you to read the message, press return key and the program will exit.", 0);
	exit(0);
}

/* from sound.cpp */
void cpStrToTimeCode(char *st, DVTimeCode *tc) {
	tc->hours = 0;
	tc->minutes = 0;
	tc->seconds = 0;
	tc->frames  = 0;
}

void setTimeCodeToZero(DVTimeCode *dest) {
	dest->hours   = 0;
	dest->minutes = 0;
	dest->seconds = 0;
	dest->frames  = 0;
}

void setTimeCodeToTime(DVTimeCode *dest, UInt8 h, UInt8 m, UInt8 s, UInt8 f) {
	dest->hours   = h;
	dest->minutes = m;
	dest->seconds = s;
	dest->frames  = f;
}

char sameKeyPressed(int key) {
	return(FALSE);
}

char *sendMessage(char *mess) {
	return("This function is not working yet");
}

void AdjustName(unCH *toSt, unCH *fromSt, int WinLim) {
	int i, j, len;

	if (strlen(fromSt) <= WinLim)
		strcpy(toSt, fromSt);
	else {
		for (i=0; fromSt[i] != PATHDELIMCHR && fromSt[i] != EOS && i < WinLim; i++)
			toSt[i] = fromSt[i];
		if (fromSt[i] == PATHDELIMCHR && i < WinLim) {
			toSt[i] = fromSt[i];
			i++;
		}
		toSt[i] = EOS;
		if (i < WinLim-1) {
			len = strlen(fromSt);
			for (j=len; j >= i && fromSt[j] != PATHDELIMCHR; j--) ;
			if (i+len-j < WinLim-1) {
				j = len - WinLim + i + 3;
				strcat(toSt, "...");
			} else {
				j = len - WinLim + 3;
				strcpy(toSt, "...");
			}
		} else {
			j = strlen(fromSt) - WinLim + 3;
			strcpy(toSt, "...");
		}
		strcat(toSt, fromSt+j);
	}
}
