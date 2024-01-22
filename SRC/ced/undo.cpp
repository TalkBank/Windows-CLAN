#include "ced.h"

#if defined(_MAC_CODE) || defined(_WIN32)
extern void RecompEdWinSize(void);
#endif
void SaveVarUndoState(UNDO *p);
void ResetVarStatus(UNDO *p);

/* one undo add/remove begin */
static void CopyVarState(UNDO *p) {
	global_df->UndoList->CurCodeArr = p->CurCodeArr;
	global_df->UndoList->FirstCode = p->FirstCode;
	global_df->UndoList->StartCodeArr = p->StartCodeArr;
	global_df->UndoList->CursorCodeArr = p->CursorCodeArr;
	global_df->UndoList->EditorMode = p->EditorMode;
	global_df->UndoList->LeftCol = p->LeftCol;
	global_df->UndoList->top_win_pos = p->top_win_pos;
	global_df->UndoList->row_win = p->row_win;
	global_df->UndoList->col_win = p->col_win;
	global_df->UndoList->col_chr = p->col_chr;
	global_df->UndoList->row_win2 = p->row_win2;
	global_df->UndoList->col_win2 = p->col_win2;
	global_df->UndoList->col_chr2 = p->col_chr2;
	global_df->UndoList->lineno   = p->lineno;
	global_df->UndoList->wLineno  = p->wLineno;

	global_df->UndoList->ShowParags = p->ShowParags;
}

void SaveVarUndoState(UNDO *p) {
#if defined(_MAC_CODE) || defined(_WIN32)
	if (global_df->SoundWin) {
		p->CurCodeArr	= (CODES *)global_df->SnTr.EndF;
		p->FirstCode	= (CODES *)global_df->SnTr.BegF;
	} else {
#endif /* defined(_MAC_CODE) || defined(_WIN32) */
		p->CurCodeArr	= global_df->RootCodesArr[0];
		p->FirstCode	= global_df->FirstCodeOfPrevLevel;
#if defined(_MAC_CODE) || defined(_WIN32)
	}
#endif /* defined(_MAC_CODE) || defined(_WIN32) */
	p->StartCodeArr	= global_df->StartCodeArr;
	p->CursorCodeArr	= global_df->CursorCodeArr;
	p->EditorMode	= global_df->EditorMode;
	p->LeftCol		= global_df->LeftCol;
	p->top_win_pos	= global_df->wLineno - global_df->row_win;
	p->row_win		= global_df->row_win;
	p->col_win		= global_df->col_win;
	p->col_chr		= global_df->col_chr;
	p->row_win2		= global_df->row_win2;
	p->col_win2		= global_df->col_win2;
	p->col_chr2		= global_df->col_chr2;
	p->lineno		= global_df->lineno;
	p->wLineno		= global_df->wLineno;
	p->ShowParags	= global_df->ShowParags;
	p->key			= MOVERPT;
	p->str			= NULL;
	p->NextUndo		= NULL;
	p->PrevUndo		= NULL;
}

void ResetVarStatus(UNDO *p) {
	register long i;

#if defined(_MAC_CODE) || defined(_WIN32)
	if (global_df->SoundWin) {
		DrawSoundCursor(0);
		global_df->SnTr.BegF = (long)p->FirstCode;
		global_df->SnTr.EndF = (long)p->CurCodeArr;
		SetPBCglobal_df(false, 0L);
		DisplayEndF(FALSE);
#if defined(_WIN32)
		DrawSoundCursor(1);
#endif
	} else
#endif /* defined(_MAC_CODE) || defined(_WIN32) */
	if (p->CurCodeArr != global_df->RootCodesArr[0]) {
		global_df->FirstCodeOfPrevLevel = p->FirstCode;
		MapArrToList(p->CurCodeArr);
	}

	if (global_df->RootCodes == global_df->RootCodesArr[0]) {
		global_df->CurFirstCodeOfPrevLevel = global_df->RootCodes;
		global_df->CurCode = global_df->RootCodes;
	}
	global_df->StartCodeArr		= p->StartCodeArr;
	global_df->CursorCodeArr	= p->CursorCodeArr;
	global_df->EditorMode		= p->EditorMode;
	global_df->ShowParags		= p->ShowParags;
	global_df->LeftCol			= p->LeftCol;
	global_df->lineno			= p->lineno;
	global_df->wLineno			= p->wLineno;
	i = p->top_win_pos;
	for (global_df->top_win=global_df->head_text; i> 0L && !AtBotEnd(global_df->top_win,global_df->tail_text,FALSE); i--) {
		global_df->top_win = ToNextRow(global_df->top_win, FALSE);
	}
	i = global_df->row_win = p->row_win;
	for (global_df->row_txt=global_df->top_win; i> 0L && !AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE); i--) {
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
	}
	global_df->row_win -= i;
	global_df->col_chr = p->col_chr;
	global_df->col_win = p->col_win;
	global_df->col_chr2 = p->col_chr2;
	global_df->col_win2 = p->col_win2;
	global_df->row_win2 = p->row_win2;
	if (global_df->row_txt == global_df->cur_line) {
		int j;
		global_df->col_txt = global_df->head_row->next_char;
		for (j=0; j < global_df->col_chr && global_df->col_txt != global_df->tail_row; j++) 
			global_df->col_txt = global_df->col_txt->next_char;
		global_df->col_chr = j;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
	} else if (global_df->col_chr > (int)strlen(global_df->row_txt->line)) {
		global_df->col_chr = strlen(global_df->row_txt->line);
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
	}
#if defined(_MAC_CODE) || defined(_WIN32)
	RecompEdWinSize();
#endif
}
/* one undo add/remove end */

/* undo begin */
static void ResetCurStatus() {
	register long i;

#if defined(_MAC_CODE) || defined(_WIN32)
	if (global_df->SoundWin) {
		DrawSoundCursor(0);
		global_df->SnTr.BegF = (long)global_df->UndoList->FirstCode;
		global_df->SnTr.EndF = (long)global_df->UndoList->CurCodeArr;
		SetPBCglobal_df(false, 0L);
		DisplayEndF(FALSE);
	} else
#endif
		if (global_df->UndoList->CurCodeArr != global_df->RootCodesArr[0]) {
			global_df->FirstCodeOfPrevLevel = global_df->UndoList->FirstCode;
			MapArrToList(global_df->UndoList->CurCodeArr);
		}

	if (global_df->RootCodes == global_df->RootCodesArr[0]) {
		global_df->CurFirstCodeOfPrevLevel = global_df->RootCodes;
		global_df->CurCode = global_df->RootCodes;
	}
	global_df->ShowParags		= global_df->UndoList->ShowParags;
	global_df->StartCodeArr		= global_df->UndoList->StartCodeArr;
	global_df->CursorCodeArr	= global_df->UndoList->CursorCodeArr;
	global_df->EditorMode		= global_df->UndoList->EditorMode;
	global_df->LeftCol			= global_df->UndoList->LeftCol;
	global_df->lineno			= global_df->UndoList->lineno;
	global_df->wLineno			= global_df->UndoList->wLineno;

	i = global_df->UndoList->top_win_pos;
	for (global_df->top_win=global_df->head_text; i> 0L && !AtBotEnd(global_df->top_win,global_df->tail_text,FALSE); i--) {
		global_df->top_win = ToNextRow(global_df->top_win, FALSE);
	}
	i = global_df->row_win = global_df->UndoList->row_win;	
	if (i < 0) {
		for (global_df->row_txt=global_df->top_win; i<0L && !AtTopEnd(global_df->row_txt,global_df->tail_text,FALSE); i++){
			global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
		}
	} else {
		for (global_df->row_txt=global_df->top_win; i>0L && !AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE); i--){
			global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
		}
	}

	global_df->row_win -= i;
	global_df->col_win = global_df->UndoList->col_win;
	global_df->col_chr = global_df->UndoList->col_chr;
	global_df->col_win2 = global_df->UndoList->col_win2;
	global_df->col_chr2 = global_df->UndoList->col_chr2;
	global_df->row_win2 = global_df->UndoList->row_win2;	
	if (global_df->row_txt == global_df->cur_line) {
		int j;
		global_df->col_txt = global_df->head_row->next_char;
		for (j=0; j < global_df->col_chr && global_df->col_txt != global_df->tail_row; j++) 
			global_df->col_txt = global_df->col_txt->next_char;
		global_df->col_chr = j;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
	} else if (global_df->col_chr > (int)strlen(global_df->row_txt->line)) {
		global_df->col_chr = strlen(global_df->row_txt->line);
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
	}
#if defined(_MAC_CODE) || defined(_WIN32)
	RecompEdWinSize();
#endif
}

static void UndoInsert(UNDO *t) {
	register int i;
	register int num;
	register long len;
	ROWS *trow_txt;

	ChangeCurLine();
	num = (int)((t->top_win_pos+t->row_win) - (global_df->UndoList->top_win_pos+global_df->row_win));
	if (num) {
		num--;
		trow_txt = global_df->row_txt;
		len = t->col_chr + 1L;
		trow_txt = trow_txt->next_row;
		while (num > 0) {
			len += strlen(trow_txt->line) + 1L;
			trow_txt = trow_txt->next_row;
			num--;
		}
		len = len + (global_df->head_row_len - global_df->col_chr);
	} else len = t->col_chr - global_df->col_chr;
	global_df->col_txt = global_df->head_row->next_char;
	for (i=0; i < global_df->col_chr; i++)
		global_df->col_txt = global_df->col_txt->next_char;

	global_df->redisplay = 0;
	if (global_df->UndoList->str)
		free(global_df->UndoList->str);
	global_df->UndoList->str = (unCH *)malloc((len+1)*sizeof(unCH));
	for (i=0; len > 0L; len--, i++) {
		if (global_df->UndoList->str) {
			if (global_df->col_txt == global_df->tail_row)
				global_df->UndoList->str[i] = SNL_C;
			else
				global_df->UndoList->str[i] = global_df->col_txt->c;
		}
		DeleteNextChar(-1);
	}
	if (global_df->UndoList->str)
		global_df->UndoList->str[i] = EOS;
	global_df->redisplay = 1;
}

static void UndoDelete(UNDO *t) {
	ChangeCurLine();
	global_df->col_txt = global_df->col_txt->prev_char;
	AddString(global_df->UndoList->str, (long)strlen(global_df->UndoList->str), FALSE);
	global_df->col_txt = global_df->col_txt->next_char;
}

static void UndoReplace() {
	int len;
	UNDO t;

	while (global_df->UndoList->NextUndo != NULL) {
		ChangeCurLine();
		len = (int)global_df->UndoList->str[0];
		global_df->redisplay = 0;
		ced_line[len+1] = EOS;
		for (len--; len >= 0 ; len--) {
			ced_line[len+1] = global_df->col_txt->prev_char->c;
			DeletePrevChar(-1);
		}
		ced_line[0] = strlen(global_df->UndoList->str+1);
		global_df->redisplay = 1;
		if (global_df->UndoList->str[1] != EOS) {
			global_df->col_txt = global_df->col_txt->prev_char;
			AddString(global_df->UndoList->str+1, (long)strlen(global_df->UndoList->str+1), FALSE);
			global_df->col_txt = global_df->col_txt->next_char;
		}
		SaveVarUndoState(&t);
		ResetCurStatus();
		CopyVarState(&t);
		if (global_df->UndoList->str)
			free(global_df->UndoList->str);
		global_df->UndoList->str = (unCH *)malloc((strlen(ced_line+1)+2)*sizeof(unCH));
		if (global_df->UndoList->str) {
			global_df->UndoList->str[0] = ced_line[0];
			strcpy(global_df->UndoList->str+1, ced_line+1);
		}
		if (global_df->UndoList->key == LASTKEY)
			break;
		global_df->UndoList = global_df->UndoList->NextUndo;
		global_df->UndoCounter--;
	}
}

int Undo(int i) {
	UNDO *t;

	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;
	t = global_df->UndoList;
	global_df->UndoList = global_df->UndoList->NextUndo;
	if (global_df->UndoList->NextUndo) {
		if (global_df->UndoList->key == LASTKEY || global_df->UndoList->key == REPLKEY) {
			UndoReplace();
			if (global_df->UndoList->NextUndo == NULL) {
				FindMidWindow();
				DisplayTextWindow(NULL, 1);
				if (i > -1)
					strcpy(global_df->err_message, DASHES);
				return(46);
			}
		} else {
			while ((global_df->UndoList->key == MOVERPT || global_df->UndoList->key == INSTRPT ||
				   global_df->UndoList->key == DELTRPT) && global_df->UndoList->NextUndo != NULL) {
				if (global_df->UndoList->key == DELTRPT)
					UndoDelete(t);
				ResetCurStatus();
				if (global_df->UndoList->key == INSTRPT)
					UndoInsert(t);
				global_df->UndoCounter--;
//				if (t->str)
//					free(t->str);
//				free(t);
				t = global_df->UndoList;
				global_df->UndoList = global_df->UndoList->NextUndo;
			}
			if (global_df->UndoList->key == DELTKEY)
				UndoDelete(t);
			ResetCurStatus();
			if (global_df->UndoList->key == INSTKEY)
				UndoInsert(t);
		}
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
		if (global_df->StartCodeArr+global_df->CursorCodeArr > global_df->EndCodeArr) {
			global_df->StartCodeArr = global_df->StartCodeArr + global_df->CursorCodeArr;
			global_df->CursorCodeArr = 0;
			global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
		}
		FindMidWindow();
		DisplayTextWindow(NULL, 1);
		global_df->UndoCounter--;
//		if (t->str)
//			free(t->str);
//		free(t);
		t = global_df->UndoList;
		global_df->UndoList = global_df->UndoList->NextUndo;
		if (i > -1)
			strcpy(global_df->err_message, DASHES);
	} else {
		strcpy(global_df->err_message, "+Nothing more to undo!");
	}
	global_df->UndoCounter--;
//	if (t->str)
//		free(t->str);
//	free(t);
	return(46);
}
/* undo end */

/* redo begin */
static void RedoReplace(void) {
	int len;
	UNDO t;

	while (global_df->UndoList->PrevUndo != NULL) {
		SaveVarUndoState(&t);
		ResetCurStatus();
		CopyVarState(&t);
		ChangeCurLine();
		len = (int)global_df->UndoList->str[0];
		global_df->redisplay = 0;
		ced_line[len+1] = EOS;
		for (len--; len >= 0 ; len--) {
			ced_line[len+1] = global_df->col_txt->prev_char->c;
			DeletePrevChar(-1);
		}
		ced_line[0] = strlen(global_df->UndoList->str+1);
		global_df->redisplay = 1;
		if (global_df->UndoList->str[1] != EOS) {
			global_df->col_txt = global_df->col_txt->prev_char;
			AddString(global_df->UndoList->str+1, (long)strlen(global_df->UndoList->str+1), FALSE);
			global_df->col_txt = global_df->col_txt->next_char;
		}
		if (global_df->UndoList->str)
			free(global_df->UndoList->str);
		global_df->UndoList->str = (unCH *)malloc((strlen(ced_line+1)+2)*sizeof(unCH));
		if (global_df->UndoList->str) {
			global_df->UndoList->str[0] = ced_line[0];
			strcpy(global_df->UndoList->str+1, ced_line+1);
		}
		global_df->UndoList = global_df->UndoList->PrevUndo;
		global_df->UndoCounter++;
		if (global_df->UndoList->key != REPLKEY)
			break;
	}
}

static void RedoInsert(UNDO *t) {
	if (t->str != NULL) {
		ChangeCurLine();
		global_df->col_txt = global_df->col_txt->prev_char;
		AddString(t->str, (long)strlen(t->str), FALSE);
		global_df->col_txt = global_df->col_txt->next_char;
	}
}

static void RedoDelete(UNDO *t) {
	register int i;
	register int num;
	register long len;
	ROWS *trow_txt;

	ChangeCurLine();
	num = (int)((t->top_win_pos+t->row_win) - (global_df->UndoList->top_win_pos+global_df->row_win));
	if (num) {
		num--;
		trow_txt = global_df->row_txt;
		len = t->col_chr + 1L;
		trow_txt = trow_txt->next_row;
		while (num > 0) {
			len += strlen(trow_txt->line) + 1L;
			trow_txt = trow_txt->next_row;
			num--;
		}
		len = len + (global_df->head_row_len - global_df->col_chr);
	} else len = t->col_chr - global_df->col_chr;
	global_df->col_txt = global_df->head_row->next_char;
	for (i=0; i < global_df->col_chr; i++)
		global_df->col_txt = global_df->col_txt->next_char;

	if (len == 0L && t->str != NULL)
		len = strlen(t->str);
	global_df->redisplay = 0;
	for (i=0; len > 0L; len--, i++)
		DeleteNextChar(-1);
	global_df->redisplay = 1;
}

int Redo(int i) {
	UNDO *t;

	if (global_df->UndoList->PrevUndo == NULL) {
		strcpy(global_df->err_message, "+Nothing more to redo!");
		return(84);
	}
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;
	if (global_df->UndoList->key == LASTKEY || global_df->UndoList->key == REPLKEY) {
		RedoReplace();
		if (global_df->UndoList->PrevUndo == NULL) {
			FindMidWindow();
			DisplayTextWindow(NULL, 1);
			if (i > -1)
				strcpy(global_df->err_message, DASHES);
			return(84);
		}
	} else {
		t = global_df->UndoList;
		global_df->UndoList = global_df->UndoList->PrevUndo;
		if (t->key == INSTKEY)
			RedoInsert(t);
		ResetCurStatus();
		if (t->key == DELTKEY)
			RedoDelete(t);
		while ((global_df->UndoList->key == MOVERPT || global_df->UndoList->key == INSTRPT ||
			   global_df->UndoList->key == DELTRPT) && global_df->UndoList->PrevUndo != NULL) {
			global_df->UndoCounter++;
			t = global_df->UndoList;
			global_df->UndoList = global_df->UndoList->PrevUndo;
			if (t->key == INSTRPT)
				RedoInsert(t);
			ResetCurStatus();
			if (t->key == DELTRPT)
				RedoDelete(t);
		}
	}
	global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	if (global_df->StartCodeArr+global_df->CursorCodeArr > global_df->EndCodeArr) {
		global_df->StartCodeArr = global_df->StartCodeArr + global_df->CursorCodeArr;
		global_df->CursorCodeArr = 0;
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	}
	FindMidWindow();
	DisplayTextWindow(NULL, 1);
/*
	global_df->UndoCounter++;
	t = global_df->UndoList;
	global_df->UndoList = global_df->UndoList->PrevUndo;
*/
	if (i > -1)
		strcpy(global_df->err_message, DASHES);
	global_df->UndoCounter++;
	return(84);
}
/* redo end */

/* add/remove undo begin */
void RemoveLastUndo(void) {
	UNDO *p;

	p = global_df->UndoList;
	if (p->NextUndo != NULL) {
		global_df->UndoCounter--;
		global_df->UndoList = global_df->UndoList->NextUndo;
		global_df->UndoList->PrevUndo = NULL;
		if (p->str) free(p->str);
		free(p);
	}
}

static UNDO *RearangeUndoList(void) {
	UNDO *p = global_df->UndoList, *t;

	while (p->NextUndo != NULL)
		p = p->NextUndo;
	
	t = p->PrevUndo;
	p->PrevUndo = t->PrevUndo;
	t->PrevUndo->NextUndo = p;
	if (t->str != NULL)
		free(t->str);
	t->str = NULL;
	t->PrevUndo = NULL;
	t->NextUndo = NULL;
	return(t);
}

void ResetUndos(void) {
	UNDO *tu;

	while (global_df->UndoList->NextUndo != NULL) {
		tu = global_df->UndoList;
		global_df->UndoList = global_df->UndoList->NextUndo;
		if (tu->str) free(tu->str);
		free(tu);
	}
	global_df->UndoList->PrevUndo = NULL;
	global_df->UndoCounter = 0;
}

void freeUndo(UNDO *p) {
	UNDO *t;

	if (p != NULL) {
		t = p;
		p = p->NextUndo;
		if (t->str)
			free(t->str);
		free(t);
	}
}

static void RemoveTrailingUndos(UNDO *p) {
	UNDO *t;

	if (p != NULL) {
		t = p;
		p = p->PrevUndo;
		if (t->str) free(t->str);
		free(t);
	}
}

void SaveUndoState(char isDummy) {
	UNDO *p;

	if (global_df == NULL)
		return;
	if (global_df->UndoList->PrevUndo != NULL) {
		if (isDummy == TRUE) {
			if (global_df->LastCommand != 84) {
				global_df->UndoList = global_df->UndoList->PrevUndo;
				global_df->UndoCounter++;
			}
			return;
		} else {
			RemoveTrailingUndos(global_df->UndoList->PrevUndo);
			global_df->UndoList->PrevUndo = NULL;
		}
	}

	if (global_df->UndoCounter < UNDOLISTNUM) {
		if ((p=NEW(UNDO)) == NULL) {
			if (global_df->UndoCounter > 1)
				p = RearangeUndoList();
			else {
				ResetUndos();
				return;
			}
		} else
			global_df->UndoCounter++;
	} else
		p = RearangeUndoList();

#if defined(_MAC_CODE) || defined(_WIN32)
	if (global_df->SoundWin) {
		p->CurCodeArr	= (CODES *)global_df->SnTr.EndF;
		p->FirstCode	= (CODES *)global_df->SnTr.BegF;
	} else {
#endif /* defined(_MAC_CODE) || defined(_WIN32) */
		p->CurCodeArr	= global_df->RootCodesArr[0];
		p->FirstCode	= global_df->FirstCodeOfPrevLevel;
#if defined(_MAC_CODE) || defined(_WIN32)
	}
#endif /* defined(_MAC_CODE) || defined(_WIN32) */
	p->StartCodeArr	= global_df->StartCodeArr;
	p->CursorCodeArr= global_df->CursorCodeArr;
	p->EditorMode	= global_df->EditorMode;
	p->LeftCol		= global_df->LeftCol;
	p->top_win_pos	= global_df->wLineno - global_df->row_win;
	p->row_win		= global_df->row_win;
	p->col_win		= global_df->col_win;
	p->col_chr		= global_df->col_chr;
	p->row_win2		= global_df->row_win2;
	p->col_win2		= global_df->col_win2;
	p->col_chr2		= global_df->col_chr2;
	p->lineno		= global_df->lineno;
	p->wLineno		= global_df->wLineno;
	p->ShowParags	= global_df->ShowParags;
	p->key			= MOVEKEY;
	p->str			= NULL;
	p->NextUndo		= global_df->UndoList;
	global_df->UndoList->PrevUndo = p;
	p->PrevUndo		= NULL;
	global_df->UndoList = p;
}
/* add/remove undo end */
