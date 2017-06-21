#include "ced.h"
#include <math.h>
#include <TextUtils.h>

static int h;
static int v;
static int row, col, lrow;
static char *tempSt = NULL;
static char where; /* 1, 2, 3, 4, 5, 6, 7, 8, 9 */
static long left_lim, right_lim;
static long last_row_win, last_col_win;
static WINDOW *curWin;
static POINT lastWhere = {0,0};
static DWORD lastWhen = 0;

char DoubleClickCount = 1;

static char isword(int c) {
	if (isalnum(c) ||  c < 0 || c > 127 || 
			c == (int)'-' || c == (int)'\'' || c == (int)'_')
		return(TRUE);
	else return(FALSE);
}

static void CpHead_rowToTempSt(void) {
	char *s;
	LINE *tl;

	if (tempSt == NULL) {
		tempSt = (char *)malloc(global_df->head_row_len+1);
		if (tempSt == NULL) {
			tempSt = "";
			mem_err(TRUE, global_df);
		}
	} else if (global_df->head_row_len >= (int)strlen(tempSt)) {
		free(tempSt);
		tempSt = (char *)malloc(global_df->head_row_len+1);
		if (tempSt == NULL) {
			tempSt = "";
			mem_err(TRUE, global_df);
		}
	}
	s = tempSt;
	tl = global_df->head_row->next_char;
	while (tl != global_df->tail_row) {
		*s++ = (char)tl->c;
		tl = tl->next_char;
	}
	*s = '\0';
}

static int FindRightColumn(int h, unCH *s, int row, WINDOW *t) {
	register int tcol;
	register int ccol;
	register int res;
	register int i;
	LOGFONT lfFont;
	CFont l_font;

	SetLogfont(&lfFont, t->RowFInfo[row], NULL);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = GlobalDC->SelectObject(&l_font);
	cedDFnt.isUTF = global_df->isUTF;
	cedDFnt.Encod = my_FontToScript(t->RowFInfo[row]->FName, t->RowFInfo[row]->CharSet);
	ccol = tcol = 0;
	h -= (LEFTMARGIN + t->textOffset);
	for (i=0; s[i] && i < t->num_cols; i++) {
		res = TextWidthInPix(s, 0, i+1, t->RowFInfo[row], 0);
		if (res > ccol) {
			ccol = res;
			tcol = (ccol - tcol) / 2;
		}
		if (h < ccol-tcol) {
			GlobalDC->SelectObject(pOldFont);
			l_font.DeleteObject();
			if (i == 0)
				return(global_df->LeftCol-1);
			i--;
			return(i+global_df->LeftCol);
		}
		if (res >= ccol)
			tcol = ccol;
	}
	if (h < tcol+(ccol/2)) i--;

	GlobalDC->SelectObject(pOldFont);
	l_font.DeleteObject();

	if (i < 0) return(global_df->LeftCol-1);
	cedDFnt.isUTF = global_df->isUTF;
	i -= 1;
	i++;
	return(i+global_df->LeftCol);
}

static void DelOldCur(long row_wint, long col_wint) {
	long rt, ct, ct2, rt2;

	rt  = global_df->row_win;  ct  = global_df->col_win;
	rt2 = global_df->row_win2; ct2 = global_df->col_win2;
	global_df->row_win = row_wint; global_df->col_win = col_wint;
	global_df->row_win2 = 0; global_df->col_win2 = -2;
	DrawCursor(0);
	global_df->row_win  = rt;  global_df->col_win  = ct;
	global_df->row_win2 = rt2; global_df->col_win2 = ct2;
}

static void SortOutPos(long row_wint, long col_wint, long col_chrt, char cnv) {
	long rt, ct;

	if ((global_df->row_win > row_wint) || (global_df->row_win == row_wint && global_df->col_win > col_wint)) {
		if ((global_df->row_win2 == row_wint && global_df->col_win2 == col_wint) ||
			(global_df->row_win2 == row_wint-1 && global_df->col_win2 == 0 && col_wint == 0))
			DelOldCur(row_wint, col_wint);
		else if (last_row_win==global_df->row_win2 && last_col_win==global_df->col_win2 && cnv== 0) {
			rt = global_df->row_win; ct = global_df->col_win;
			global_df->row_win = row_wint; global_df->col_win = col_wint; global_df->row_win2 -= global_df->row_win;
			DrawCursor(2);
			global_df->row_win2 = row_wint; global_df->col_win2 = col_wint; global_df->col_chr2 = col_chrt;
			global_df->row_win = rt; global_df->col_win = ct;
		}
			rt = global_df->row_win2; ct = global_df->col_win2;
			global_df->row_win2 = row_wint - global_df->row_win; global_df->col_win2 = col_wint;
			DrawCursor(2);
			global_df->row_win2 = rt; global_df->col_win2 = ct;
	} else if ((global_df->row_win<global_df->row_win2) || (global_df->row_win==global_df->row_win2 && global_df->col_win<global_df->col_win2)) {
		if (global_df->row_win2 == row_wint && global_df->col_win2 == col_wint)
			DelOldCur(row_wint, col_wint);
		else if (last_row_win==row_wint && last_col_win==col_wint && cnv== 0) {
			rt = global_df->row_win; ct = global_df->col_win;
			global_df->row_win = row_wint; global_df->col_win = col_wint; global_df->row_win2 -= global_df->row_win;
			DrawCursor(2);
			global_df->row_win2 += global_df->row_win;
			row_wint = global_df->row_win2; col_wint = global_df->col_win2; col_chrt = global_df->col_chr2;
			global_df->row_win = rt; global_df->col_win = ct;
		}
		global_df->row_win2 -= global_df->row_win;
		DrawCursor(2);
		global_df->row_win2 = row_wint; global_df->col_win2 = col_wint; global_df->col_chr2 = col_chrt;
	} else if ((global_df->row_win > global_df->row_win2 && global_df->row_win < row_wint) ||
				   (global_df->row_win==global_df->row_win2 && global_df->row_win< row_wint && global_df->col_win> global_df->col_win2) ||
				   (global_df->row_win> global_df->row_win2 && global_df->row_win==row_wint && global_df->col_win< col_wint) ||
				   (global_df->row_win == global_df->row_win2 && global_df->row_win == row_wint && 
					global_df->col_win > global_df->col_win2  && global_df->col_win <  col_wint)) {
		if (last_row_win == global_df->row_win2 && last_col_win == global_df->col_win2) {
			global_df->row_win2 -= global_df->row_win;
			DrawCursor(2);
			global_df->row_win2 = row_wint; global_df->col_win2 = col_wint; global_df->col_chr2 = col_chrt;
		} else {
			rt = global_df->row_win2; ct = global_df->col_win2;
			global_df->row_win2 = row_wint-global_df->row_win; global_df->col_win2 = col_wint;
			DrawCursor(2);
			global_df->row_win2 = rt; global_df->col_win2 = ct;
		}
	} else if (global_df->row_win == global_df->row_win2 && global_df->col_win == global_df->col_win2) {
		global_df->row_win2 = row_wint - global_df->row_win;
		global_df->col_win2 = col_wint;
		if (global_df->row_win2 == 0L && global_df->col_win == global_df->col_win2) global_df->col_win2 = -2;
		DrawCursor(1);
		global_df->row_win2 = row_wint;
		global_df->col_win2 = col_wint; global_df->col_chr2 = col_chrt;
	}
	last_row_win = global_df->row_win;
	last_col_win = global_df->col_win;
}

void matchCursorColToGivenCol(long col) {
	if (global_df->row_txt == global_df->cur_line) {
		global_df->col_win = ComColWin(FALSE, NULL, global_df->head_row_len);
		if (global_df->col_win < col)
			col = global_df->col_win;
		global_df->col_win = 0L;
		global_df->col_chr = 0L;
		global_df->col_txt = global_df->head_row->next_char;
		while (global_df->col_txt != global_df->tail_row && global_df->col_win < col) {
			global_df->col_chr++;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			global_df->col_txt = global_df->col_txt->next_char;
		}
		if (global_df->col_txt->prev_char != global_df->head_row &&
				global_df->col_txt->prev_char->c == NL_C && global_df->ShowParags != '\001') {
			global_df->col_chr--;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			global_df->col_txt = global_df->col_txt->prev_char;
		}
	} else {
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, strlen(global_df->row_txt->line));
		if (global_df->col_win < col)
				col = global_df->col_win;
		global_df->col_win = 0L;
		global_df->col_chr = 0L;
		while (global_df->row_txt->line[global_df->col_chr] != EOS && global_df->col_win < col) {
			global_df->col_chr++;
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		}
		if (global_df->col_chr > 0 && global_df->row_txt->line[global_df->col_chr-1] == NL_C &&
															global_df->ShowParags != '\001') {
			global_df->col_chr--;
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		}
	}
}

static char isAnyBulletsOnLine(void) {
	register int offset;
	long index, ti;
	char hc_found;
	ROWS *tr;
	LINE *tl, *ttl;
	
	offset = 0;
	tr = global_df->row_txt;
	if (global_df->row_txt == global_df->cur_line) {
		hc_found = FALSE;
		ttl = NULL;
		for (tl=global_df->col_txt; tl != global_df->tail_row; tl=tl->next_char) {
			if (tl->c == HIDEN_C) {
				hc_found = !hc_found;
				if (hc_found && ttl != NULL)
					return(FALSE);
				ttl = tl;
				if (!hc_found && ttl != NULL)
					break;
			}
		}
		if (ttl != NULL) {
			return(TRUE);
		}
	} else {
		hc_found = FALSE;
		ti = -1;
		for (index=global_df->col_chr; global_df->row_txt->line[index]; index++) {
			if (global_df->row_txt->line[index] == HIDEN_C) {
				hc_found = !hc_found;
				if (hc_found && ti != -1)
					return(FALSE);
				ti = index;
				if (!hc_found && ti != -1)
					break;
			}
		}
		if (ti != -1) {
			return(TRUE);
		}
	}
	return(FALSE);
}

static void UpdateCursorPos(int row, int col, char cnv, int extend) {
	long old_row_win = global_df->row_win;
	long row_wint, col_wint, col_chrt;

	if (cnv < 2) {
		long t;

		if (global_df->row_win2 == 0L && global_df->col_win2 == -2) {
			global_df->row_win2 = global_df->row_win;
			global_df->col_win2 = global_df->col_win;
			global_df->col_chr2 = global_df->col_chr;
		} else {
			global_df->row_win2 += global_df->row_win;
		}
		row_wint = global_df->row_win;
		col_wint = global_df->col_win;
		col_chrt = global_df->col_chr;
		if (row_wint < global_df->row_win2) {
			t = row_wint; row_wint = global_df->row_win2; global_df->row_win2 = t;
			t = col_wint; col_wint = global_df->col_win2; global_df->col_win2 = t;
			t = col_chrt; col_chrt = global_df->col_chr2; global_df->col_chr2 = t;
		} else if (row_wint == global_df->row_win2 && col_wint < global_df->col_win2) {
			t = col_wint; col_wint = global_df->col_win2; global_df->col_win2 = t;
			t = col_chrt; col_chrt = global_df->col_chr2; global_df->col_chr2 = t;
		}
	}
		
	global_df->row_win = 0;
	global_df->row_txt = global_df->top_win;
	while (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE) && row > 0) {
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
		row--;
		global_df->row_win++;
	}

	if (last_row_win != global_df->row_win || extend != 2)
		matchCursorColToGivenCol(col);

	if (cnv == 3) {
		GetCurCode();
		if ((col=uS.partcmp(sp, "%gra:", FALSE, FALSE))) {
			isPlayS = -8;
		} else if ((col=uS.partcmp(sp, "%grt:", FALSE, FALSE))) {
			isPlayS = -8;
		} else if (global_df->SoundWin && (GetCurHidenCode(TRUE, NULL) || isAnyBulletsOnLine())) {
			char isTopWinChanged;
			char tDataChanged = global_df->DataChanged;
			long old_col_win = global_df->col_win;
			long old_col_chr = global_df->col_chr;
			long old_row_win = global_df->row_win;
			long old_col_win2 = global_df->col_win2;
			long old_col_chr2 = global_df->col_chr2;
			long old_row_win2 = global_df->row_win2;
			long old_lineno  = global_df->lineno;
			long old_LeftCol = global_df->LeftCol;
			ROWS *old_row_txt = global_df->row_txt;
			ROWS *old_top_win = global_df->top_win;
			FindAnyBulletsOnLine();
			if (FindTextCodeLine(cl_T(SOUNDTIER), NULL)) {
				if ((col=uS.partcmp(sp, SOUNDTIER, FALSE, FALSE)))
					RSoundPlay(-2, FALSE);
			}
			isTopWinChanged = (global_df->top_win != old_top_win);
			global_df->col_win = old_col_win;
			global_df->col_chr = old_col_chr;
			global_df->row_win = old_row_win;
			global_df->col_win2 = old_col_win2;
			global_df->col_chr2 = old_col_chr2;
			global_df->row_win2 = old_row_win2;
			global_df->lineno  = old_lineno;
			global_df->LeftCol = old_LeftCol;
			global_df->row_txt = old_row_txt;
			global_df->top_win = old_top_win;
			if (global_df->row_txt == global_df->cur_line) {
				long j;
				for (global_df->col_txt=global_df->head_row->next_char, j=global_df->col_chr; j > 0; j--)
					global_df->col_txt = global_df->col_txt->next_char;
			}
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			if (isTopWinChanged)
				DisplayTextWindow(NULL, 1);
			global_df->DataChanged = tDataChanged;
		}
		global_df->col_win2 = 0L;
		global_df->col_chr2 = 0L;
		if (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
			global_df->row_win2++;
			global_df->col_win = 0L;
			global_df->col_chr = 0L;
		} else {
			if (global_df->row_txt == global_df->cur_line) {
				for (; global_df->col_txt->c != NL_C && global_df->col_txt != global_df->tail_row;
								global_df->col_chr++, global_df->col_txt=global_df->col_txt->next_char) ;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			} else {
				for (global_df->col_chr++; global_df->row_txt->line[global_df->col_chr] != NL_C &&
							global_df->row_txt->line[global_df->col_chr]; global_df->col_chr++) ;
				global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
			}
		}
		if (/*!global_df->ChatMode && */FindFileLine(TRUE, NULL)) {
			isPlayS = -5;
			InKey = (int)'v';
			global_df->LeaveHighliteOn = TRUE;
		}
	} else if (cnv == 2) {
		unCH *s;
		LINE *col_txt2;

		cedDFnt.isUTF = global_df->isUTF;
		if (global_df->row_txt == global_df->cur_line) {
			CpHead_rowToTempSt();
			if (global_df->col_txt != global_df->tail_row && isword(global_df->col_txt->c)) {
		 		for (; (isword(global_df->col_txt->c) || my_CharacterByteType(tempSt,global_df->col_chr,&cedDFnt)) && global_df->col_txt != global_df->tail_row; 
						global_df->col_chr++, global_df->col_txt=global_df->col_txt->next_char) ;
				global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		 		for (global_df->col_chr2=global_df->col_chr, col_txt2=global_df->col_txt->prev_char; 
						(isword(col_txt2->c) || my_CharacterByteType(tempSt,global_df->col_chr2,&cedDFnt)) && col_txt2 != global_df->head_row;
						global_df->col_chr2--, col_txt2=col_txt2->prev_char) ;
				global_df->col_win2 = ComColWin(FALSE, NULL, global_df->col_chr2);
			}
		} else {
			s = global_df->row_txt->line;
			if (isword(s[global_df->col_chr])) {
#ifdef _UNICODE
				for (global_df->col_chr++; isword(s[global_df->col_chr]); global_df->col_chr++) ;
#else
				for (global_df->col_chr++; isword(s[global_df->col_chr]) || my_CharacterByteType(s,global_df->col_chr,&cedDFnt); global_df->col_chr++) ;
#endif
				global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
#ifdef _UNICODE
				for (global_df->col_chr2=global_df->col_chr-1; isword(s[global_df->col_chr2]) && global_df->col_chr2 >= 0; global_df->col_chr2--) ;
#else
				for (global_df->col_chr2=global_df->col_chr-1; 
					 (isword(s[global_df->col_chr2]) || my_CharacterByteType(s,global_df->col_chr2,&cedDFnt)) && global_df->col_chr2 >= 0; 
					 global_df->col_chr2--) ;
#endif
				global_df->col_chr2++;
				global_df->col_win2 = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr2);
			}
		}
	} else if (cnv < 2) {
		SortOutPos(row_wint, col_wint, col_chrt, cnv);
		global_df->row_win2 -= global_df->row_win;
	}

	if (global_df->row_win2 == 0L && global_df->col_win == global_df->col_win2) {
		global_df->col_win2 = -2L;
		global_df->col_chr2 = -2L;
	}
	global_df->lineno += (global_df->row_win - old_row_win);
	global_df->window_rows_offset = 0;
	PosAndDispl();
}

void StartSelectCursorPosition(CPoint point, int extend) {
	long  tn;
	DWORD curTime;

	if (global_df == NULL)
		return;
	h = point.x;
	v = point.y;
	curWin = global_df->RootWindow;
	while (curWin != NULL) {
		if (curWin->LT_row <= v &&
			((curWin == global_df->w1 && v < global_df->TextWinSize) ||
			 (curWin == global_df->wm && v < curWin->LT_row+ComputeHeight(curWin,0,1)) ||
			 (curWin != global_df->wm && v < curWin->LT_row+ComputeHeight(curWin,0,ActualWinSize(curWin))))) {
			row = 0;
			v -= curWin->LT_row;
			do {
				v -= ComputeHeight(curWin,row,row+1);
				if (v <= 0) break;
				if (row > curWin->num_rows) break;
				row++;
			} while (TRUE) ;
			break;
		}
		curWin = curWin->NextWindow;
	}
	if (curWin == NULL) return;

	if (global_df->EditorMode && curWin == global_df->w2) {
		if (global_df->SoundWin == NULL && !global_df->NoCodes && global_df->cod_fname != NULL) {
			InKey = (int)'e';
			isPlayS = 4;
			global_df->LeaveHighliteOn = TRUE;
			return;
		}
	}

	if (global_df->ScrollBar == '\001' && curWin == global_df->w1) {
/* 30-4-99 && (global_df->SoundWin == NULL || extend != 2)*/
		where = 1;
		col = FindRightColumn(h, curWin->win_data[row], row, curWin);
		if (!global_df->EditorMode && extend != 2) {
			InKey = (int)'e';
			isPlayS = 4;
			global_df->LeaveHighliteOn = TRUE;
		} else {
			isPlayS = 0;
			InKey = 0;
		}
/* take care of the mouse */
		if (global_df->row_win2 == 0L && global_df->col_win2 == -2)
			DrawCursor(0);
		else if (extend == 1) {
			if (global_df->row_win2 > 0) {
				global_df->row_win2 += global_df->row_win;
				tn = global_df->row_win; global_df->row_win = global_df->row_win2; global_df->row_win2 = tn;
				global_df->row_win2 -= global_df->row_win;
				tn = global_df->col_win; global_df->col_win = global_df->col_win2; global_df->col_win2 = tn;
				tn = global_df->col_chr; global_df->col_chr = global_df->col_chr2; global_df->col_chr2 = tn;
			} else if (global_df->row_win2 == 0 && global_df->col_win < global_df->col_win2) {
				tn = global_df->col_win; global_df->col_win = global_df->col_win2; global_df->col_win2 = tn;
				tn = global_df->col_chr; global_df->col_chr = global_df->col_chr2; global_df->col_chr2 = tn;
			}
		} else {
			DrawCursor(0);
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
		}
		last_row_win = global_df->row_win;
		last_col_win = global_df->col_win;
		curTime = GetTickCount();
		if ((curTime - lastWhen < (DWORD)GetDoubleClickTime()) &&
			(labs(point.x - lastWhere.x) < 5) &&
			(labs(point.y - lastWhere.y) < 5) && DoubleClickCount < 3) {
			DoubleClickCount++;
			UpdateCursorPos(row, col+1, DoubleClickCount, extend);
			DrawCursor(2);
			where = 6;
		} else {
			DoubleClickCount = 1;
			SaveUndoState(FALSE);
			if (extend == 1) UpdateCursorPos(row, col+1, 1, extend);
			else UpdateCursorPos(row, col+1, 4, extend);
		}
		SetScrollControl();
		if (extend == 2)
			lastWhen  = 0;
		else
			lastWhen  = curTime;
		lastWhere = point;
	} else if (global_df->SoundWin && (curWin == global_df->SoundWin || curWin == global_df->w2 || extend == 2)) {
		RECT theRect;

		GlobalDC->GetWindow()->GetClientRect(&theRect);
		where = 2;
		right_lim = theRect.right - global_df->SnTr.SNDWccol;
		left_lim = global_df->SnTr.SNDWccol;
		if (global_df->row_win2 != 0L || global_df->col_win2 != -2L) {
			DrawCursor(0);
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
		}
		if (h >= left_lim && h <= right_lim) {
			if (global_df->SnTr.BegF == global_df->SnTr.EndF || global_df->SnTr.EndF == 0L) DrawSoundCursor(0);
			else if (!extend) {
				DrawSoundCursor(0);
				global_df->SnTr.EndF = 0L;
			}
		} else 
			DrawSoundCursor(0);
		v = row;
		row = 0;
		lrow = point.y - global_df->SoundWin->LT_row;
		do {
			if (row >= global_df->SoundWin->num_rows) {
				row = global_df->SoundWin->num_rows - 1;
				break;
			}
			lrow -= ComputeHeight(global_df->SoundWin,row,row+1);
			if (lrow <= 0) break;
			row++;
		} while (TRUE) ;

		curTime = GetTickCount();
		if (((curTime - lastWhen) <  (DWORD)GetDoubleClickTime()) &&
			(labs(point.x - lastWhere.x) < 5) &&
			(labs(point.y - lastWhere.y) < 5) &&
			(h >= left_lim && h <= right_lim)) {
			InKey = (int)'s';
			isPlayS = 50;
		}

		if (extend == 2)
			lastWhen  = 0;
		else
			lastWhen  = curTime;
		lastWhere = point;
		where = AdjustSound(row, point.x, extend, right_lim);
	} else if (!global_df->EditorMode && curWin == global_df->w2 && extend != 2) {
		where = 3;
		col = FindRightColumn(h, curWin->win_data[row], row, curWin);
		curTime = GetTickCount();
		if (((curTime - lastWhen) <  (DWORD)GetDoubleClickTime()) &&
			(labs(point.x - lastWhere.x) < 5) &&
			(labs(point.y - lastWhere.y) < 5)) {
			if (UpdateCodesCursorPos(row, col+1)) {
				DrawCursor(0);
				InKey = (int)'c';
				isPlayS = 39;
			}
		} else {
			UpdateCodesCursorPos(row, col+1);
		}
		if (extend == 2)
			lastWhen  = 0;
		else
			lastWhen  = curTime;
		lastWhere = point;
	} else if (curWin == global_df->wm) {
		where = 8;
		if (global_df->SoundWin) {
			DrawSoundCursor(0);
			global_df->SnTr.dtype = (++global_df->SnTr.dtype) % 3;
			PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
			if (global_df->SnTr.dtype == 0)
				PrintSoundWin(0,0,0L);
		} else {
			blinkCursorLine();
		}
	} else if (!global_df->EditorMode && curWin == global_df->w1 && extend == 2) {
		where = 9;
		SaveUndoState(FALSE);
		col = FindRightColumn(h, curWin->win_data[row], row, curWin);
		curTime = GetTickCount();
		UpdateCursorPos(row, col+1, 1, extend);
		PosAndDispl();
		DrawCursor(0);
		global_df->row_win2 = 0L;
		global_df->col_win2 = -2L;
		global_df->col_chr2 = -2L;
		InKey = (int)'r';
		isPlayS = -6;
		if (extend == 2)
			lastWhen  = 0;
		else
			lastWhen  = curTime;
		lastWhere = point;
	} else {
		where = 4;
		if (global_df->ScrollBar != '\001') {
			DrawCursor(0);
			DrawSoundCursor(0);
			return;
		}
		if (global_df->row_win2 != 0L || global_df->col_win2 != -2L) {
			DrawCursor(0);
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
		}
	}
}

static void MoveRegionDownAndAddFirstTier(WINDOW *w) {
	register long i;
	RECT	theRect;
	RECT 	winRect;
	CWnd*	win = GlobalDC->GetWindow();

	if (win == NULL)
		return;
	win->GetClientRect(&winRect);
	if (!AtTopEnd(global_df->top_win, global_df->head_text, FALSE)) {
		global_df->top_win = ToPrevRow(global_df->top_win, FALSE);
		global_df->row_win++;
		last_row_win++;
		global_df->WinChange = FALSE;
		DisplayTextWindow(NULL, 0);
		global_df->WinChange = TRUE;
	} else return;

	theRect.left = LEFTMARGIN;
	theRect.right = winRect.right;
	theRect.top = w->LT_row;
	theRect.bottom = theRect.top + global_df->TextWinSize;

 	win->ScrollWindow(0, ComputeHeight(w, 0, 1), &theRect, NULL);

	global_df->WinChange = FALSE;
	wUntouchWin(w, 0);
	wrefresh(w);
	global_df->WinChange = TRUE;
	if (global_df->row_win2 < 0) {
		i = global_df->col_win;
		global_df->col_win = 0L;
		DrawCursor(2);
		global_df->col_win = i;
	}
}

static void MoveRegionUpAndAddLastTier(WINDOW *w) {
	register long i;
	int		NumRows;
	ROWS*	tt;
	RECT	theRect;
	RECT 	winRect;
	CWnd*	win = GlobalDC->GetWindow();

	if (win == NULL)
		return;
	win->GetClientRect(&winRect);

	if (!AtBotEnd(global_df->top_win, global_df->tail_text, FALSE)) {
		for (i=ActualWinSize(w), tt=global_df->top_win; i > 0; i--) {
			if (!AtBotEnd(tt, global_df->tail_text, FALSE)) 
				tt = ToNextRow(tt, FALSE);
			else return;
		}
		NumRows = ActualWinSize(w);
		global_df->top_win = ToNextRow(global_df->top_win, FALSE);
		global_df->row_win--;
		last_row_win--;

		theRect.left = LEFTMARGIN;
		theRect.right = winRect.right;
		theRect.top = w->LT_row;
		theRect.bottom = theRect.top + global_df->TextWinSize - 1;

 		win->ScrollWindow(0, -(ComputeHeight(w, 0, 1)), &theRect, NULL);

		global_df->WinChange = FALSE;
		DisplayTextWindow(NULL, 0);
		global_df->WinChange = TRUE;
	} else
		return;

	global_df->WinChange = FALSE;
	if (NumRows > ActualWinSize(w)) 
		wUntouchWin(w, NumRows-1);
	else
		wUntouchWin(w, ActualWinSize(w)-1);
	wrefresh(w);
	global_df->WinChange = TRUE;
	if (global_df->row_win2 > 0) {
		i = global_df->col_win;
		global_df->row_win2--;
		global_df->row_win++; global_df->col_win = 0L;
		DrawCursor(2);
		global_df->row_win--; global_df->col_win = i;
		global_df->row_win2++;
	}
}

int MidSelectCursorPosition(CPoint point, int extend) {
	extern void AdjustSoundScroll(int col, Size right_lim);

	if (global_df == NULL || curWin == NULL)
		return(0);
	if (where == 1) {
		lrow = 0;
		v = point.y - curWin->LT_row;
		do {
			if (lrow >= global_df->EdWinSize) {
				lrow = global_df->EdWinSize - 1;
				break;
			}
			v -= ComputeHeight(curWin,lrow,lrow+1);
			if (v <= 0) break;
			lrow++;
		} while (TRUE) ;
		v = point.y - curWin->LT_row;
		if (v >= global_df->TextWinSize) {
			MoveRegionUpAndAddLastTier(curWin);
			row = global_df->EdWinSize - 1;
			col = FindRightColumn(point.x, curWin->win_data[row], row, curWin);
			SetScrollControl();
		} else if (v < 0) {
			MoveRegionDownAndAddFirstTier(curWin);
			row = 0;
			col = FindRightColumn(point.x, curWin->win_data[row], row, curWin);
			SetScrollControl();
		} else {
			row = lrow;
			col = FindRightColumn(point.x, curWin->win_data[row], row, curWin);
		} 
		UpdateCursorPos(row, col+1, 0, extend);
	} else if (where == 2) {
		row = 0;
		lrow = point.y - global_df->SoundWin->LT_row;
		do {
			if (row >= global_df->SoundWin->num_rows) {
				row = global_df->SoundWin->num_rows - 1;
				break;
			}
			lrow -= ComputeHeight(global_df->SoundWin,row,row+1);
			if (lrow <= 0) break;
			row++;
		} while (TRUE) ;
		if ((point.x >= left_lim && point.x <= right_lim &&
			 h >= left_lim && h <= right_lim) ||
			((point.x < left_lim || point.x > right_lim) &&
			 (h < left_lim || h > right_lim))) {
			where = AdjustSound(row, point.x, extend, right_lim);
		}
	} else if (where == 3) {
	} else if (where == 4) {
	} else if (where == 7) {
		AdjustSoundScroll(point.x, right_lim);
		return(1);
	} else if (where == 8) {
	} else if (where == 9) {
	}
	return(0);
}

void EndSelectCursorPosition(CPoint point, int extend) {
	long tn;

	if (global_df == NULL || curWin == NULL)
		return;

	if (where == 1 || where == 6) {
		GetCurCode();
		FindRightCode(1);
		if (isPlayS != -5 && isPlayS != -8 && isPlayS != 4) {
			isPlayS = 0;
			InKey = 0;
		}
		PosAndDispl();
		if (extend == 2) {
			DrawCursor(0);
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
			InKey = (int)'r';
			if (global_df->SoundWin == NULL)
				isPlayS = -4;
			else
				isPlayS = -7;
		}
	} else if (where == 2 || where == 5 || where == 7) {
		if (where == 7) {
			if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L) {
				SaveUndoState(FALSE);
				isPlayS = -3;
			}
		}
		if (where == 5) {
			if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L) 
				isPlayS = -3;
			else
				isPlayS = -2;
		}
		if (global_df->SnTr.EndF < global_df->SnTr.BegF && global_df->SnTr.EndF != 0L) {
			tn = global_df->SnTr.EndF; global_df->SnTr.EndF = global_df->SnTr.BegF; global_df->SnTr.BegF = tn;
		}
		if ((h < left_lim || h > right_lim) && (v == 1 || v == 6 || v == 5 || v == 10))
			DisplayEndF(TRUE);
/* 7-1-98
		if (v == 4 && (h < left_lim || h > right_lim)) {
			global_df->WinChange = FALSE;
			touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
			global_df->WinChange = TRUE;
		}
*/
		DrawCursor(0);
	} else if (where == 3) {
	} else if (where == 4) {
		DrawCursor(0);
	} else if (where == 8) {
	} else if (where == 9) {
	}
}
