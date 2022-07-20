#include "ced.h"
#include "prefs.h"
#include "cu.h"
#include "MMedia.h"

static char locateTopWinFromChar(myFInfo *g_df, long pos, long skip) {
	long i;
	long cPos, cSkip;
	long trow_win, twindow_rows_offset;
	ROWS *tr;

	trow_win = g_df->row_win;
	twindow_rows_offset = g_df->window_rows_offset;
	cPos = 0L;
	cSkip = 0L;
	tr = g_df->head_text->next_row;
	if (cPos == pos && skip == 0L) {
		g_df->top_win = tr;
		return(TRUE);
	}
	for (; tr != g_df->tail_text; tr = tr->next_row) {
		for (i=0; tr->line[i] != EOS; i++) {
			if (tr->line[i]!='\n' && !isSpace(tr->line[i]) && tr->line[i]!=NL_C && tr->line[i]!=SNL_C) {
				if (cPos == pos && skip != 0L) {
					g_df->top_win = tr;
					return(TRUE);
				}
				cSkip = 0L;
				cPos++;
			} else {
				cSkip++;
			}
			if (cPos == pos && skip == 0L) {
				g_df->top_win = tr;
				return(TRUE);
			}
		}
		g_df->row_win--;
		g_df->window_rows_offset++;
		if (g_df->row_win >= 0L && g_df->row_win < g_df->EdWinSize)
			g_df->window_rows_offset = 0;
	}
	g_df->row_win = trow_win;
	g_df->window_rows_offset = twindow_rows_offset;
	return(FALSE);
}

static char locateRowTxtFromChar(myFInfo *g_df, long pos, long skip) {
	long i, lastBullet;
	long cPos, cSkip;
	long trow_win, twindow_rows_offset, tlineno, twLineno;
	ROWS *tr;

	trow_win = g_df->row_win;
	twindow_rows_offset = g_df->window_rows_offset;
	tlineno = g_df->lineno;
	twLineno = g_df->wLineno;
	cPos = 0L;
	cSkip = 0L;
	tr = g_df->row_txt;
	if (cPos == pos && skip == 0L) {
		g_df->row_txt = tr;
		g_df->col_chr = 0L;
		return(TRUE);
	}
	for (; tr != g_df->tail_text; tr = tr->next_row) {
		lastBullet = -1L;
		for (i=0; tr->line[i] != EOS; i++) {
			if (tr->line[i] == HIDEN_C) {
				if (lastBullet == -1)
					lastBullet = i;
				else
					lastBullet = -1L;
			}
			if (tr->line[i]!='\n' && !isSpace(tr->line[i]) && tr->line[i]!=NL_C && tr->line[i]!=SNL_C) {
				if (cPos == pos && skip != 0L) {
					g_df->row_txt = tr;
					if (lastBullet == -1)
						g_df->col_chr = i;
					else
						g_df->col_chr = lastBullet;
					return(TRUE);
				}
				cSkip = 0L;
				cPos++;
			} else {
				cSkip++;
			}
			if (cPos == pos && skip == 0L) {
				g_df->row_txt = tr;
				if (lastBullet == -1)
					g_df->col_chr = i + 1;
				else
					g_df->col_chr = lastBullet;
				return(TRUE);
			}
		}
		if (isNL_CFound(tr))
			g_df->lineno++;
		g_df->wLineno++;
		g_df->row_win++;
		g_df->window_rows_offset--;
		if (g_df->row_win >= 0L && g_df->row_win < g_df->EdWinSize)
			g_df->window_rows_offset = 0;
	}
	g_df->row_win = trow_win;
	g_df->window_rows_offset = twindow_rows_offset;
	g_df->lineno  = tlineno;
	g_df->wLineno = twLineno;
	return(FALSE);
}

static ROWS *FindRowTxt(myFInfo *g_df, ROWS *orgTr) {
	ROWS *tr;

	for (tr=orgTr; tr != g_df->tail_text; tr = tr->next_row) {
		if (tr == g_df->row_txt)
			return(orgTr);
		g_df->row_win2--;
	}
	g_df->row_win2 = 0L;
	return(NULL);
}

static ROWS *locate2RowTxtFromChar(myFInfo *g_df, long pos, long skip) {
	char foundRowTxt;
	long i;
	long cPos, cSkip;
	ROWS *tr;

	cPos = 0L;
	cSkip = 0L;
	tr = g_df->head_text->next_row;
	if (tr == g_df->row_txt) {
		foundRowTxt = TRUE;
	} else
		foundRowTxt = FALSE;
	if (cPos == pos && skip == 0L) {
		g_df->col_chr2 = 0L;
		if (!foundRowTxt)
			return(FindRowTxt(g_df, tr));
		else
			return(tr);
	}
	for (; tr != g_df->tail_text; tr = tr->next_row) {
		if (tr == g_df->row_txt)
			foundRowTxt = TRUE;
		for (i=0; tr->line[i] != EOS; i++) {
			if (tr->line[i]!='\n' && !isSpace(tr->line[i]) && tr->line[i]!=NL_C && tr->line[i]!=SNL_C) {
				if (cPos == pos && skip != 0L) {
					g_df->col_chr2 = i;
					if (!foundRowTxt)
						return(FindRowTxt(g_df, tr));
					else
						return(tr);
				}
				cSkip = 0L;
				cPos++;
			} else {
				cSkip++;
			}
			if (cPos == pos && skip == 0L) {
				g_df->col_chr2 = i + 1;
				if (!foundRowTxt)
					return(FindRowTxt(g_df, tr));
				else
					return(tr);
			}
		}
		if (foundRowTxt)
			g_df->row_win2++;
	}
	g_df->row_win2 = 0L;
	g_df->col_win2 = -2L;
	g_df->col_chr2 = -2L;
	return(NULL);
}

void setTextCursor(myFInfo *g_df, WindowInfo *wi) {
	ROWS  *tr;
	extern char  DefWindowDims;

	if (isCursorPosRestore && isAjustCursor && (wi->topC != 0L || wi->pos1C != 0L ||  wi->pos2C != 0L)) {
		DrawCursor(0);
		DrawSoundCursor(0);
		g_df->row_win = 0L;
		g_df->col_win = 0L;
		g_df->col_chr = 0L;
		g_df->row_win2 = 0L;
		g_df->col_win2 = -2L;
		g_df->col_chr2 = -2L;
		g_df->LeaveHighliteOn = FALSE;
		g_df->lineno  = 1L;
		g_df->wLineno = 1L;
		g_df->row_txt = g_df->head_text->next_row;

		if (!locateTopWinFromChar(g_df, wi->topC, wi->skipTop)) {
			DisplayTextWindow(NULL, 1);
			FinishMainLoop();
			return;
		}
		if (!locateRowTxtFromChar(g_df, wi->pos1C, wi->skipP1)) {
			DisplayTextWindow(NULL, 1);
			FinishMainLoop();
			return;
		}
		if (g_df->row_txt == g_df->cur_line) {
			g_df->col_txt = g_df->head_row->next_char;
			wi->topC = 0L;
			for (; wi->topC < g_df->col_chr && g_df->col_txt != g_df->tail_row; wi->topC++)
				g_df->col_txt = g_df->col_txt->next_char;
			if (g_df->col_txt->prev_char->c == NL_C &&
				g_df->ShowParags != '\001' && g_df->col_chr > 0) {
				g_df->col_chr--;
				g_df->col_txt = g_df->col_txt->prev_char;
			}
			g_df->col_win = ComColWin(FALSE, NULL, g_df->col_chr);
		} else {
			if (g_df->row_txt->line[g_df->col_chr-1] == NL_C &&
				g_df->ShowParags != '\001' && g_df->col_chr > 0)
				g_df->col_chr--;
			g_df->col_win = ComColWin(FALSE, g_df->row_txt->line, g_df->col_chr);
		}
		if (wi->pos2C != wi->pos1C || wi->skipP2 != wi->skipP1) {
			if ((tr=locate2RowTxtFromChar(g_df, wi->pos2C, wi->skipP2)) != NULL) {
				if (tr->line[g_df->col_chr2-1] == NL_C &&
					g_df->ShowParags != '\001' && g_df->col_chr2 > 0)
					g_df->col_chr2--;
				g_df->col_win2 = ComColWin(FALSE, tr->line, g_df->col_chr2);
			}
		}
		DisplayTextWindow(NULL, 1);
		FinishMainLoop();
	}
}


#ifdef _MAC_CODE
myFInfo *InitFileInfo(FNType *fname, long lLen, WindowPtr wind, short id, char *isUTF8Header)
#endif
#ifdef _WIN32
myFInfo *InitFileInfo(FNType *fname, long lLen, short id, char *isUTF8Header)
#endif
{
	int i;
	myFInfo *FInf;
	FONTINFO fi;
	extern short DOSdata;

	if ((FInf=NEW(myFInfo)) == NULL)
		ProgExit("Out of memory");

	DOSdata = -1;
	global_df = FInf;
	if (fname == NULL)
		*FInf->fileName = EOS;
	else
		strcpy(FInf->fileName, fname);

	strcpy(FInf->SnTr.rSoundFile, FInf->fileName);
	FInf->mediaFileName[0] = EOS;
	FInf->isErrorFindPict = FALSE;
	fi.FSize = DEFAULT_SIZE;
#ifdef _MAC_CODE
	FInf->wind = wind;
	fi.FName = DEFAULT_ID;
	fi.CharSet = my_FontToScript(fi.FName, 0);
	FInf->MinFontSize = GetFontHeight(&fi, NULL, wind);
#endif
#ifdef _WIN32
	global_df = FInf;
	strcpy(fi.FName, DEFAULT_FONT);
	fi.CharSet = DEFAULT_CHARSET;
	FInf->MinFontSize = GetFontHeight(&fi, NULL);
#endif
	FInf->PIDs = NULL;
	FInf->winID = id;
	FInf->checkMessCnt = 1;
	FInf->CodeWinStart = 0;
	FInf->lineno  = 0L;
	FInf->wLineno = 0L;
	FInf->window_rows_offset = 0L;
	FInf->winWidthOffset = -1;
	FInf->FreqCount = 0;
	FInf->RootColorWord = NULL;
	FInf->RootColorText = NULL;
	FInf->RootWindow = NULL;
	FInf->RdW = NULL;
	FInf->w1 = NULL;
	FInf->ScrollBar = '\0';
	FInf->VScrollBar = FALSE;
	FInf->HScrollBar = FALSE;
	FInf->AutoWrap = DefAutoWrap;
	FInf->NoCodes = TRUE;
	FInf->isUTF = 0;
	FInf->dontAskDontTell = FALSE;
	FInf->WinChange = TRUE;
	FInf->SpecialTextFile = FALSE;
	FInf->isTempFile = 0;
	FInf->webMFiles = NULL;
	FInf->cursorCol = 0L;
	FInf->SnTr.SFileType = 0L;
	FInf->SnTr.SSource = 0;
	if (lLen != 0L && lLen < 10L)
		lLen = 10L;
	FInf->RowLimit = lLen;
	FInf->curRowLimit = NULL;
	FInf->isOutputScrolledOff = FALSE;
	FInf->DrawCur = 1;
	FInf->Ltik = 0L;
	FInf->SSelectFlag = 0;
#if defined(_MAC_CODE)
	FInf->crType = MacCrType;
#endif
#if defined(_WIN32)
	FInf->crType = PCCrType;
#endif
	FInf->KBIndex = -1L;
	FInf->KBSize = 200L;
	FInf->KillBuff = NULL;
	if ((FInf->KillBuff=(unCH *)malloc((FInf->KBSize+1L)*sizeof(unCH))) == NULL)
		mem_err(FALSE, global_df);

	FInf->EditorMode = StartInEditorMode;
	FInf->PriorityCodes = DefPriorityCodes;
	FInf->CodingScheme = DefCodingScheme;
	FInf->MakeBackupFile = MakeBackupFile;
	FInf->FreqCountLimit = FreqCountLimit;
	FInf->LastCommand = 0;
	FInf->LeaveHighliteOn = FALSE;
	FInf->isRedrawTextWindow = TRUE;
	FInf->RootCodesArr = NULL;
	FInf->winInfo.topC = 0L;
	FInf->winInfo.skipTop = 0L;
	FInf->winInfo.pos1C = 0L;
	FInf->winInfo.skipP1 = 0L;
	FInf->winInfo.pos2C = 0L;
	FInf->winInfo.skipP2 = 0L;
	FInf->SnTr.SoundFPtr = 0;
	FInf->SnTr.IsSoundOn = FALSE;
	FInf->TopP1 = NULL;
	FInf->BotP1 = NULL;
	FInf->TopP2 = NULL;
	FInf->BotP2 = NULL;
	FInf->SnTr.dtype = 0;
	FInf->SnTr.SoundFile[0] = EOS;
	FInf->MvTr.MovieFile[0] = EOS;
	FInf->MvTr.rMovieFile[0] = EOS;
	FInf->cpMvTr = NULL;
	FInf->SnTr.SoundFileSize = 0L;
	FInf->SnTr.errMess = NULL;
	FInf->PcTr.pictFName[0] = EOS;
	FInf->PcTr.pictChanged = FALSE;
	FInf->TxTr.textFName[0] = EOS;
	FInf->TxTr.textChanged = FALSE;
	FInf->w2 = NULL;
	FInf->wm = NULL;
	FInf->gAtt = 0;
	FInf->SoundWin = NULL;
	FInf->SnTr.AIFFOffset = 0L;
    FInf->tcs = FALSE;
    FInf->tch = FALSE;
    FInf->tct = FALSE;
    FInf->headtier = NULL;
	FInf->TSelectFlag = 0;
	FInf->fake_TSelectFlag = 0;
	FInf->SLtik = 0L;
	FInf->scale_row = 0L;
	FInf->mainVScale = 0;
	FInf->SnTr.SSC_TOP = 0;
	FInf->SnTr.SSC_MID = 0;
	FInf->SnTr.SSC_BOT = 0;
	FInf->SnTr.SDrawCur = 1;
	FInf->SnTr.isMP3 = FALSE;
#ifdef _MAC_CODE
	FInf->SnTr.mp3.theSoundMedia = NULL;
	FInf->SnTr.mp3.hSys7SoundData = NULL;
#endif 
	FInf->SnTr.DDsound = TRUE;
	FInf->SnTr.SNDchan = 1;
	FInf->SnTr.SNDsample = 1;
	FInf->SnTr.BegF = 0L;
	FInf->SnTr.EndF = 0L;
	FInf->MvTr.MBeg = 0L;
	FInf->MvTr.MEnd = 0L;
	FInf->SnTr.WBegF = 0L;
	FInf->SnTr.WEndF = 0L;
	FInf->SnTr.contPlayBeg = 0L;
	FInf->SnTr.contPlayEnd = 0L;
	FInf->attOutputToScreen = 0;
	FInf->EnteredCode = '\0';
	FInf->OldCode[0] = EOS;
	FInf->isExtend = 0;
	FInf->isSpelling = FALSE;
	FInf->err_message[0] = EOS;
	for (i=0; i < 10; i++)
		FInf->SpeakerNames[i] = NULL;
	for (i=0; i < 10; i++)
		FInf->VideosNameExts[i][0] = EOS;
	FInf->VideosNameIndex = -1;

	if (*FInf->fileName == EOS) {
		FInf->ChatMode = FALSE;
		FInf->total_num_rows = 0;
		FInf->cod_fname = NULL;
	} else {
		if (lLen)
			FInf->ChatMode = FALSE;
		else if (DefChatMode < 2)
			FInf->ChatMode = DefChatMode;
		else
			FInf->ChatMode = SetChatModeOfDatafileExt(FInf->fileName, FALSE);
		FInf->total_num_rows = 1;

		strcpy(FileName1, lib_dir);
		addFilename2Path(FileName1, CODEFNAME);
		if ((FInf->cod_fname=(FNType *)malloc(strlen(FileName1)+1)) == NULL)
			mem_err(FALSE, global_df);
		strcpy(FInf->cod_fname, FileName1);

		ced_check_init(TRUE);
		if (lLen) 
			init_codes("", "", NULL);
		else
			init_codes(FInf->cod_fname, CODEFNAME, NULL);
	}
#ifdef _MAC_CODE
	if (id == 1962 && isAjustCursor)
		getPrefCursor(fname);
#endif // _MAC_CODE
	init_text(isUTF8Header, id);
	checkForTiersToHide(FInf, FALSE);

#ifdef _MAC_CODE
	if (!init_windows(true, 0, false)) {
	    FreeUpText(FInf);
	    free(FInf);
	    return(NULL);
	}
	if (id == 1962 && !isRefEQZero(FInf->fileName))
		setTextCursor(FInf, &FInf->winInfo);
#endif // _MAC_CODE
	return(FInf);
}

static void FreeWebMedia(FNType *fname, webMFile *p) {
	int		i;
	FNType	path[FNSize];
	webMFile *t;

	if (p == NULL)
		return;

	extractPath(path, fname);
	i = strlen(path);
	while (p != NULL) {
		t = p;
		p = p->nextFile;
		if (t->name != NULL) {
			addFilename2Path(path, t->name);
			unlink(path);
			path[i] = EOS;
			free(t->name);
		}
		free(t);
	}
}

void FreeFileInfo(myFInfo *FileInfo) {
	int i;

	if (FileInfo == NULL)
		return;

#ifdef _MAC_CODE
	if (theMovie != NULL) {
		if (theMovie->df == FileInfo)
			theMovie->df = NULL;
	}
#endif

	global_df = FileInfo;
    if (global_df->SnTr.SoundFile[0] != EOS) {
		*global_df->SnTr.SoundFile = EOS;
    	if (global_df->SnTr.isMP3 == TRUE) {
			global_df->SnTr.isMP3 = FALSE;
#ifdef _MAC_CODE
			if (global_df->SnTr.mp3.hSys7SoundData)
				DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
			global_df->SnTr.mp3.theSoundMedia = NULL;
			global_df->SnTr.mp3.hSys7SoundData = NULL;
#endif
			if (global_df->SoundWin)
				DisposeOfSoundWin();
		} else if (global_df->SnTr.SoundFPtr != 0) {
#ifdef _WIN32
			fclose(global_df->SnTr.SoundFPtr);
			if (global_df->SoundWin)
				DisposeOfSoundWin();
#endif
#ifdef _MAC_CODE
			fclose(global_df->SnTr.SoundFPtr);
			if (global_df->SoundWin)
				DisposeOfSoundWin();
#endif // _MAC_CODE
		}
		global_df->SnTr.SoundFPtr = 0;
    }
	endwin();
	FreeCodesMem();
	FreeWebMedia(FileInfo->fileName, FileInfo->webMFiles);
	if (FileInfo->PIDs != NULL)
		free(FileInfo->PIDs);
	FreeColorWord(FileInfo->RootColorWord);
	FreeColorText(FileInfo->RootColorText);
	if (FileInfo->cod_fname != NULL)
		free(FileInfo->cod_fname);
	if (FileInfo->KillBuff != NULL)
		free(FileInfo->KillBuff);
	for (i=0; i < 10; i++) {
		if (FileInfo->SpeakerNames[i] != NULL)
			free(FileInfo->SpeakerNames[i]);
	}
	FreeUpText(FileInfo);
	if (FileInfo->isTempFile == 1)
		unlink(FileInfo->fileName);
	free(FileInfo);
}
