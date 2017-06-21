#include "ced.h"
#include "cu.h"
#include "MMedia.h"

myFInfo *InitFileInfo(FNType *fname, long lLen, WindowPtr wind, short id, char *isUTF8Header) {
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
	strcpy(fi.FName, DEFAULT_FONT);
	fi.CharSet = DEFAULT_CHARSET;
	FInf->MinFontSize = GetFontHeight(&fi, NULL);
#endif
	FInf->winID = id;
	FInf->CodeWinStart = 0;
	FInf->lineno = 0L;
	FInf->window_rows_offset = 0L;
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
	if (lLen > 0L && lLen < 10L)
		lLen = 10L;
	FInf->SnTr.SFileType = 0L;
	FInf->SnTr.SSource = 0;
	FInf->RowLimit = lLen;
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
	FInf->SnTr.mp3.theSoundMedia = NULL;
	FInf->SnTr.mp3.hSys7SoundData = NULL;
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
			init_codes("", "");
		else
			init_codes(FInf->cod_fname, CODEFNAME);
	}
	init_text(isUTF8Header, id);
	checkForTiersToHide(FInf, FALSE);

#ifdef _MAC_CODE
	if (!init_windows(true, 0, false)) {
	    FreeUpText(FInf);
	    free(FInf);
	    return(NULL);
	}
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
			if (global_df->SnTr.mp3.hSys7SoundData)
				DisposeHandle(global_df->SnTr.mp3.hSys7SoundData);
			global_df->SnTr.mp3.theSoundMedia = NULL;
			global_df->SnTr.mp3.hSys7SoundData = NULL;
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
