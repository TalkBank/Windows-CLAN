#include "ced.h"
#ifdef _WIN32 
#include "CedDlgs.h"
// NO QT #include <TextUtils.h>
#endif

/* cursor movement begin */
int PrevPage(int i) {
	if (global_df->row_win < 0L || global_df->row_win >= (long)global_df->EdWinSize) {
#if defined(_MAC_CODE)// || defined(_WIN32)
		long orgCol_win;

		orgCol_win = global_df->col_win;
		global_df->col_win = global_df->LeftCol + 1;
		orgCol_win = Con_PrevPage(orgCol_win);
		global_df->col_win = orgCol_win;
		return(31);
#endif
	}
#if !defined(_MAC_CODE) && !defined(_WIN32)
	if (global_df->row_win2 || global_df->col_win2 != -2)
		global_df->row_win2 += global_df->row_win;
#endif
	BeginningOfWindow(-1);
	if (!AtTopEnd(global_df->row_txt, global_df->head_text, FALSE)) {
#if defined(_MAC_CODE) || defined(_WIN32)
		register int num;
		ROWS *Trow_txt = global_df->row_txt;

		global_df->EdWinSize = 1;
		num = global_df->TextWinSize - Trow_txt->Font.FHeight;
		while (!AtTopEnd(Trow_txt, global_df->head_text, FALSE) && num > 0 && 
									global_df->EdWinSize <= global_df->w1->num_rows) {
			Trow_txt = ToPrevRow(Trow_txt, FALSE);
			num -= Trow_txt->Font.FHeight;
			global_df->EdWinSize++;
		}
		if (num < 0 || global_df->EdWinSize > global_df->w1->num_rows)
			global_df->EdWinSize--;
		else {
			Trow_txt = global_df->row_txt;
			while (!AtBotEnd(Trow_txt, global_df->tail_text, FALSE) && num > 0 &&
									global_df->EdWinSize <= global_df->w1->num_rows) {
				Trow_txt = ToNextRow(Trow_txt, FALSE);
				num -= Trow_txt->Font.FHeight;
				global_df->EdWinSize++;
			}
			if (num < 0 || global_df->EdWinSize > global_df->w1->num_rows)
				global_df->EdWinSize--;
		}
#endif
		global_df->redisplay = 0;
		for (i=2; i < global_df->EdWinSize; i++) {
			if (!AtTopEnd(global_df->row_txt->prev_row, global_df->head_text, FALSE)) {
#if !defined(_MAC_CODE) && !defined(_WIN32)
				if (global_df->row_win2 || global_df->col_win2 != -2) global_df->row_win2 += 1;
#endif
				MoveUp(-1);
			} else break;
		}
		global_df->redisplay = 1;
#if !defined(_MAC_CODE) && !defined(_WIN32)
		if (!AtTopEnd(global_df->row_txt->prev_row, global_df->head_text, FALSE)) 
			if (global_df->row_win2 || global_df->col_win2 != -2) global_df->row_win2 += 1;
#endif
		MoveUp(-1);
		global_df->window_rows_offset = 0L;
	}
	return(31);
}

int NextPage(int i) {
	if (global_df->row_win < 0L || global_df->row_win >= (long)global_df->EdWinSize) {
#if defined(_MAC_CODE)// || defined(_WIN32)
		Con_NextPage();
		return(30);
#endif
	}
	EndOfWindow(-1);
	if (global_df->EdWinSize < 3) {
#if !defined(_MAC_CODE) && !defined(_WIN32)
		if (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
			if (global_df->row_win2 || global_df->col_win2 != -2) global_df->row_win2 -= 1;
		}
#endif
		MoveDown(-1);
	} else {
		if (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
			global_df->redisplay = 0;
#if !defined(_MAC_CODE) && !defined(_WIN32)
			if (global_df->row_win2 || global_df->col_win2 != -2) global_df->row_win2 -= 1;
#endif
			MoveDown(-1);
			global_df->redisplay = 1;
		}
#if !defined(_MAC_CODE) && !defined(_WIN32)
		if (global_df->row_win2 || global_df->col_win2 != -2) global_df->row_win2 += global_df->row_win;
#endif
		global_df->top_win = global_df->row_txt;
		global_df->row_win = 0L;
#if !defined(_MAC_CODE) && !defined(_WIN32)
		if (!AtTopEnd(global_df->row_txt->prev_row, global_df->head_text, FALSE)) {
			if (global_df->row_win2 || global_df->col_win2 != -2) global_df->row_win2 += 1;
		}
#endif
		MoveUp(-1);
	}
	global_df->window_rows_offset = 0L;
	return(30);
}

int BeginningOfLine(int i) {
	if (global_df->row_txt == global_df->cur_line)
		global_df->col_txt=global_df->head_row->next_char;
	global_df->col_win = 0L;
	global_df->col_chr = 0L;
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	return(28);
}

static void EndOfLineInWindow(void) {
	if (global_df->row_txt == global_df->cur_line) {
		global_df->col_win = 0L;
		global_df->col_chr = 0L;
		for (global_df->col_txt=global_df->head_row->next_char; 
							(global_df->col_win=ComColWin(FALSE, NULL,global_df->col_chr)) < global_df->LeftCol+global_df->num_of_cols-1 && 
											global_df->col_txt != global_df->tail_row; global_df->col_chr++)
			global_df->col_txt = global_df->col_txt->next_char;
		if (global_df->col_txt->prev_char->c == NL_C && global_df->col_txt->prev_char != global_df->head_row &&
										global_df->col_txt == global_df->tail_row && global_df->ShowParags != '\001') {
			global_df->col_chr--;
			global_df->col_txt = global_df->col_txt->prev_char;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		}
	} else {
		global_df->col_chr = strlen(global_df->row_txt->line);
		if (global_df->row_txt->line[global_df->col_chr-1] == NL_C && 
						global_df->col_chr > 0 && global_df->ShowParags != '\001') {
			global_df->col_chr--;
		}
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		if (global_df->col_win >= global_df->LeftCol+global_df->num_of_cols-1) {
			global_df->col_win = global_df->LeftCol+global_df->num_of_cols-1;
			global_df->col_chr = ComColChr(global_df->row_txt->line, global_df->col_win);
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		}
	}
}

int EndOfLine(int i) {
	if (global_df->row_txt == global_df->cur_line) {
		global_df->col_win = 0L;
		global_df->col_chr = 0L;
		for (global_df->col_txt=global_df->head_row->next_char; global_df->col_txt != global_df->tail_row; global_df->col_chr++)
			global_df->col_txt = global_df->col_txt->next_char;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		if (global_df->col_txt->prev_char->c == NL_C && 
					global_df->col_txt->prev_char!=global_df->head_row && 
					global_df->ShowParags != '\001') {
			global_df->col_chr--;
			global_df->col_txt = global_df->col_txt->prev_char;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		}
	} else {
		global_df->col_chr = strlen(global_df->row_txt->line);
		if (global_df->row_txt->line[global_df->col_chr-1] == NL_C && 
						global_df->col_chr > 0 && global_df->ShowParags != '\001') {
			global_df->col_chr--;
		}
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
	}
	if (i > 0) {
		if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
			PutCursorInWindow(global_df->w1);
	}
	return(29);
}

int BeginningOfWindow(int i) {
	while (global_df->row_txt != global_df->top_win) {
		global_df->row_txt = global_df->row_txt->prev_row;
		if (CMP_VIS_ID(global_df->row_txt->flag)) {
			if (isNL_CFound(global_df->row_txt))
				global_df->lineno--;
			global_df->wLineno--;
		}
	}
	global_df->row_win = 0L;
	BeginningOfLine(-1);
	if (i > -1) {
		GetCurCode();
		FindRightCode(1);
	}
	global_df->window_rows_offset = 0L;
	return(26);
}

int EndOfWindow(int i) {
	char isAddLineno;

	while (1) {
		if (global_df->row_win == (long)(global_df->EdWinSize-1)) break;
		else if (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) break;
#if !defined(_MAC_CODE) && !defined(_WIN32)
		if (i == -1) {
			if (global_df->row_win2 || global_df->col_win2 != -2) global_df->row_win2 -= 1;
		}
#endif
		if (isNL_CFound(global_df->row_txt))
			isAddLineno = TRUE;
		else
			isAddLineno = FALSE;
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
		if (isAddLineno)
			global_df->lineno++;
		global_df->wLineno++;
		global_df->row_win++;
	}

	EndOfLineInWindow();

	if (i > -1) {
		GetCurCode();
		FindRightCode(1);
	}
	global_df->window_rows_offset = 0L;
	return(27);
}

int BeginningOfFile(int i) {
	global_df->row_txt = ToNextRow(global_df->head_text, FALSE);
	global_df->lineno  = 1L;
	global_df->wLineno = 1L;
	global_df->row_win = 0L;
	global_df->col_win = 0L;
	global_df->col_chr = 0L;

	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;

	if (global_df->row_txt == global_df->cur_line)
		global_df->col_txt = global_df->head_row->next_char;
	if (global_df->top_win != global_df->row_txt) {
		global_df->top_win = global_df->row_txt;
		DisplayTextWindow(NULL, 1);
	} else if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	global_df->window_rows_offset = 0L;
	return(44);
}

int EndOfFile(int i) {
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	while (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
		global_df->row_win++;
		if (isNL_CFound(global_df->row_txt))
			global_df->lineno++;
		global_df->wLineno++;
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
	}
	if (global_df->row_txt == global_df->cur_line)
		global_df->col_txt = global_df->head_row->next_char;
	global_df->col_win = 0L;
	global_df->col_chr = 0L;

	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;

	if (global_df->row_win >= (long)global_df->EdWinSize) {
		FindMidWindow();
		DisplayTextWindow(NULL, 1);
	}
	EndOfLine(i);
	global_df->window_rows_offset = 0L;
	return(45);
}

int MoveUp(int i) {
	if (!AtTopEnd(global_df->row_txt, global_df->head_text, FALSE)) {
		global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
		if (isNL_CFound(global_df->row_txt))
			global_df->lineno--;
		global_df->wLineno--;
		global_df->row_win--;

		if (i != -1 && global_df->isExtend == 1) {
			if (global_df->row_win2 == 0L && global_df->col_win2 == -2L) {
				global_df->col_win2 = global_df->col_win;
				global_df->col_chr2 = global_df->col_chr;
			}
			global_df->row_win2++;
			if (global_df->row_win2 == 0L && global_df->col_win2 == global_df->col_win) {
				global_df->col_win2 = -2;
				global_df->col_chr2 = -2;
			}
		} else
			global_df->LeaveHighliteOn = FALSE;

		matchCursorColToGivenCol(global_df->cursorCol);
/* 6-3-02
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->col_chr > global_df->head_row_len)
				EndOfLineInWindow();
			else if (global_df->col_chr < global_df->head_row_len/2) {
				global_df->col_win = 0L;
				global_df->col_chr = 0L;
				global_df->col_txt = global_df->head_row->next_char;
			} else
				EndOfLineInWindow();
		} else {
			if (global_df->col_chr > (int)strlen(global_df->row_txt->line))
				EndOfLineInWindow();
			else if (global_df->col_win < global_df->LeftCol+global_df->num_of_cols-1 / 2) {
					global_df->col_win = 0L;
					global_df->col_chr = 0L;
			} else
				EndOfLineInWindow();
		}
*/
	}
	if (global_df->row_win < 0L) {
		if (i != -1) {
			FindMidWindow();
		} else {
			global_df->row_win++;
			global_df->top_win = ToPrevRow(global_df->top_win, FALSE);
		}
		if (global_df->redisplay) {
			DisplayTextWindow(NULL, 1);
			GetCurCode();
			FindRightCode(1);
		}
	} else if (global_df->redisplay) {
		if (global_df->row_win >= (long)global_df->EdWinSize)
			PutCursorInWindow(global_df->w1);
		GetCurCode();
		FindRightCode(1);
	} else if (global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	return(17);
}

int MoveDown(int i) {
	char isAddLineno;

	if (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
		if (isNL_CFound(global_df->row_txt))
			isAddLineno = TRUE;
		else
			isAddLineno = FALSE;
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
		if (isAddLineno)
			global_df->lineno++;
		global_df->wLineno++;
		global_df->row_win++;

		if (i != -1 && global_df->isExtend == 1) {
			if (global_df->row_win2 == 0L && global_df->col_win2 == -2L) {
				global_df->col_win2 = global_df->col_win;
				global_df->col_chr2 = global_df->col_chr;
			}
			global_df->row_win2--;
			if (global_df->row_win2 == 0L && global_df->col_win2 == global_df->col_win) {
				global_df->col_win2 = -2;
				global_df->col_chr2 = -2;
			}
		} else
			global_df->LeaveHighliteOn = FALSE;

		matchCursorColToGivenCol(global_df->cursorCol);
/* 6-3-02
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->col_chr > global_df->head_row_len)
				EndOfLineInWindow();
			else if (global_df->col_chr < global_df->head_row_len / 2) {
				global_df->col_win = 0L;
				global_df->col_chr = 0L;
				global_df->col_txt = global_df->head_row->next_char;
			} else
				EndOfLineInWindow();
		} else {
			if (global_df->col_chr > (int)strlen(global_df->row_txt->line))
				EndOfLineInWindow();
			else if (global_df->col_win < global_df->LeftCol+global_df->num_of_cols-1 / 2) {
				global_df->col_win = 0L;
				global_df->col_chr = 0L;
			} else
				EndOfLineInWindow();
		}
*/
	}
	if (global_df->row_win >= (long)global_df->EdWinSize) {
		if (i != -1) {
			FindMidWindow();
		} else {
			global_df->row_win--;
			global_df->top_win = ToNextRow(global_df->top_win, FALSE);
		}
		if (global_df->redisplay) {
			DisplayTextWindow(NULL, 1);
			GetCurCode();
			FindRightCode(1);
		}
	} else if (global_df->redisplay) {
		if (global_df->row_win < 0)
			PutCursorInWindow(global_df->w1);
		GetCurCode();
		FindRightCode(1);
	} else if (global_df->row_win < 0)
		PutCursorInWindow(global_df->w1);
	return(16);
}

int MoveLineUp(int i) {
	if (!AtTopEnd(global_df->top_win, global_df->head_text, FALSE)) {
		global_df->row_win++;
		if (!global_df->EditorMode && global_df->CurCode!= global_df->RootCodes && global_df->row_win >= (long)global_df->EdWinSize) {
			global_df->row_win--;
			IllegalFunction(-1);
			strcpy(global_df->err_message, "+Finish coding current line!");
			return(52);
		}
		global_df->top_win = ToPrevRow(global_df->top_win, FALSE);
		if (global_df->row_win >= (long)global_df->EdWinSize) {
			global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
			if (isNL_CFound(global_df->row_txt))
				global_df->lineno--;
			global_df->wLineno--;
			global_df->row_win--;
			global_df->col_win = 0L;
			global_df->col_chr = 0L;
			if (global_df->row_txt == global_df->cur_line) global_df->col_txt = global_df->head_row->next_char;
#if !defined(_MAC_CODE) && !defined(_WIN32)
			if (global_df->row_win2 || global_df->col_win2 != -2) global_df->row_win2 += 1;
#endif
		}
		DisplayTextWindow(NULL, (char)i);
		GetCurCode();
		FindRightCode(1);
	}
	return(52);
}

int MoveLineDown(int i) {
	char isAddLineno;

	if (!AtBotEnd(global_df->top_win, global_df->tail_text, FALSE)) {
		global_df->row_win--;
		if (!global_df->EditorMode && global_df->CurCode != global_df->RootCodes && global_df->row_win < 0L) {
			global_df->row_win++;
			IllegalFunction(-1);
			strcpy(global_df->err_message, "+Finish coding current line!");
			return(53);
		}
		global_df->top_win = ToNextRow(global_df->top_win, FALSE);
		if (global_df->row_win < 0L) {
			if (isNL_CFound(global_df->row_txt))
				isAddLineno = TRUE;
			else
				isAddLineno = FALSE;
			global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
			if (isAddLineno)
				global_df->lineno++;
			global_df->wLineno++;
			global_df->row_win++;
			global_df->col_win = 0L;
			global_df->col_chr = 0L;
			if (global_df->row_txt == global_df->cur_line) global_df->col_txt = global_df->head_row->next_char;
#if !defined(_MAC_CODE) && !defined(_WIN32)
			if (global_df->row_win2 || global_df->col_win2 != -2)
				global_df->row_win2 -= 1;
#endif
		}
		DisplayTextWindow(NULL, (char)i);
		GetCurCode();
		FindRightCode(1);
	}
	return(53);
}

int MoveRightWord(int i) {
	int space, c;

	ChangeCurLineAlways(0);
	do {
		space = global_df->row_txt->line[global_df->col_chr];
		MoveRight(-1);
		c = global_df->row_txt->line[global_df->col_chr];
		if ((c == EOS && space != NL_C) || c == NL_C) break;
	} while (isSpace(space) || space == NL_C || !isSpace(c)) ;
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	ChangeCurLineAlways(0);
	return(24);
}

int MoveRight(int d) {
//	register short i;
	register long  len;

	cedDFnt.isUTF = global_df->isUTF;
	if (d > -1) {
		ChangeCurLineAlways(0);
		if (global_df->isExtend == 1) {
			if (global_df->row_win2 == 0L && global_df->col_win2 == -2L) {
				global_df->col_win2 = global_df->col_win;
				global_df->col_chr2 = global_df->col_chr;
			}
		} else if (global_df->row_win2 || global_df->col_win2 != -2L) {
			if (global_df->row_win2 == 0L) {
				if (global_df->col_chr < global_df->col_chr2) {
					global_df->col_chr = global_df->col_chr2;
					global_df->col_win = global_df->col_win2;
					if (global_df->row_txt == global_df->cur_line) {
						for (global_df->col_txt=global_df->head_row->next_char, d=global_df->col_chr; d > 0L; d--)
							global_df->col_txt = global_df->col_txt->next_char;
					}
				}
			} else if (global_df->row_win2 > 0L) {
				d = (int)global_df->redisplay;
				global_df->redisplay = 0;
				for (; global_df->row_win2 > 0L; global_df->row_win2--)
					MoveDown(-1);
				global_df->redisplay = (char)d;
				global_df->col_chr = global_df->col_chr2;
				global_df->col_win = global_df->col_win2;
				if (global_df->row_txt == global_df->cur_line) {
					for (global_df->col_txt=global_df->head_row->next_char, d=global_df->col_chr; d > 0L; d--)
						global_df->col_txt = global_df->col_txt->next_char;
				}
			}
			global_df->LeaveHighliteOn = FALSE;
			return(18);
		}
	}
	len = (long)strlen(global_df->row_txt->line);
//	do {
		if (global_df->row_txt->line[global_df->col_chr] == NL_C && global_df->ShowParags != '\001' && d > -1) {
			global_df->col_chr++;
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		} else if (global_df->ShowParags != '\002' && global_df->row_txt->line[global_df->col_chr] == HIDEN_C) {
			if (++global_df->col_chr > len)
				goto finMoveRight;
			while (global_df->col_chr <= len && global_df->row_txt->line[global_df->col_chr] != HIDEN_C)
				global_df->col_chr++;
			if (global_df->col_chr > len)
				goto finMoveRight;
		}
		if (++global_df->col_chr > len) {
finMoveRight:
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
			if (!AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
				global_df->col_win = 0L;
				global_df->col_chr = 0L;
				MoveDown(1);
			} else {
				global_df->col_chr--;
				if (global_df->row_txt->line[global_df->col_chr-1] == NL_C && global_df->ShowParags != '\001' &&
					d > -1 && global_df->col_chr > 0) global_df->col_chr--;
				global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
			}
			return(18);
		}
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
//		break;
//	} while (i != 0 && i != -1) ;
	if (d > -1) {
		if (global_df->isExtend == 1) {
			if (global_df->row_win2 == 0L && global_df->col_win2 == global_df->col_win) {
				global_df->col_win2 = -2;
				global_df->col_chr2 = -2;
			}
		}
		if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
			PutCursorInWindow(global_df->w1);
		ChangeCurLineAlways(0);
	}
	return(18);
}

int MoveLeftWord(int i) {
	int space, c;

	ChangeCurLineAlways(0);
	do {
		space = global_df->row_txt->line[global_df->col_chr];
		MoveLeft(-1);
		if (global_df->col_chr == 0L) break;
		c = global_df->row_txt->line[global_df->col_chr];
	} while (isSpace(space) || space == NL_C || !isSpace(c)) ;
	if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
		PutCursorInWindow(global_df->w1);
	ChangeCurLineAlways(0);
	return(25);
}

int MoveLeft(int d) {
	register int i;

	cedDFnt.isUTF = global_df->isUTF;
	if (d > -1) {
		ChangeCurLineAlways(0);
		if (global_df->isExtend == 1) {
			if (global_df->row_win2 == 0L && global_df->col_win2 == -2L) {
				global_df->col_win2 = global_df->col_win;
				global_df->col_chr2 = global_df->col_chr;
			}
		} else if (global_df->row_win2 || global_df->col_win2 != -2L) {
			if (global_df->row_win2 == 0L) {
				if (global_df->col_chr > global_df->col_chr2) {
					global_df->col_chr = global_df->col_chr2;
					global_df->col_win = global_df->col_win2;
					if (global_df->row_txt == global_df->cur_line) {
						for (global_df->col_txt=global_df->head_row->next_char, d=global_df->col_chr; d > 0L; d--)
							global_df->col_txt = global_df->col_txt->next_char;
					}
				}
			} else if (global_df->row_win2 < 0L) {
				d = (int)global_df->redisplay;
				global_df->redisplay = 0;
				for (; global_df->row_win2 < 0L; global_df->row_win2++)
					MoveUp(-1);
				global_df->redisplay = (char)d;
				global_df->col_chr = global_df->col_chr2;
				global_df->col_win = global_df->col_win2;
				if (global_df->row_txt == global_df->cur_line) {
					for (global_df->col_txt=global_df->head_row->next_char, d=global_df->col_chr; d > 0L; d--)
						global_df->col_txt = global_df->col_txt->next_char;
				}
			}
			global_df->LeaveHighliteOn = FALSE;
			return(19);
		}
	}
	do {
		if (--global_df->col_chr < 0) {
finMoveLeft:
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
			if (!AtTopEnd(global_df->row_txt, global_df->head_text, FALSE)) {
				global_df->col_chr = 0xFFFFL;
				global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
				MoveUp(0);
				EndOfLine(-1);
				if ((global_df->col_chr > 0 && global_df->row_txt->line[global_df->col_chr-1] != NL_C) ||
													global_df->ShowParags == '\001')
					return(19);
				else if (d > -1) 
					i = 0;
				else
					i = 1;
			} else {
				global_df->col_chr++;
				global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
				return(19);
			}
		} else {
			if (global_df->ShowParags != '\002' && global_df->row_txt->line[global_df->col_chr] == HIDEN_C) {
				if (--global_df->col_chr < 0)
					goto finMoveLeft;
				while (global_df->col_chr >= 0 && global_df->row_txt->line[global_df->col_chr] != HIDEN_C)
					global_df->col_chr--;
				if (global_df->col_chr < 0)
					goto finMoveLeft;
			}
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
			break;
		}
	} while (i != 0 && i != -1) ;
	if (d > -1) {
		if (global_df->isExtend == 1) {
			if (global_df->row_win2 == 0L && global_df->col_win2 == global_df->col_win) {
				global_df->col_win2 = -2;
				global_df->col_chr2 = -2;
			}
		}
		if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
			PutCursorInWindow(global_df->w1);
		ChangeCurLineAlways(0);
	}
	return(19);
}
/* cursor movement end */

int CursorsCommand(int i) {
#ifndef _WIN32
	i = wgetch(global_df->w1);
#ifdef VT200
	i = VT200key(i,global_df->w1);
#endif
	if (i == (int)'A') {
		  if (global_df->CurCode == global_df->RootCodes || global_df->EditorMode) return(MoveUp(i));
		  else return(MoveCodeCursorUp(i));
	} else if (i == (int)'B') {
		  if (global_df->CurCode == global_df->RootCodes || global_df->EditorMode) return(MoveDown(i));
		  else return(MoveCodeCursorDown(i));
	} else if (i == (int)'C') {
		  if (global_df->CurCode == global_df->RootCodes || global_df->EditorMode) return(MoveRight(i));
		  else return(IllegalFunction(-1));
	} else if (i == (int)'D') {
		  if (global_df->CurCode == global_df->RootCodes || global_df->EditorMode) return(MoveLeft(i));
		  else return(IllegalFunction(-1));
	} else return(IllegalFunction(-1));
#else
	return(0);
#endif
}

void MoveToSpeaker(long num, char refresh) {
	int count = -1;
	char isAddLineno;
	long ln;
	ROWS *tr = global_df->head_text->next_row;

	global_df->lineno  = 1L;
	global_df->wLineno = 1L;
	ln = 0L;
	if (tr->line[0] == '*')
		ln++;
	while (tr->next_row != global_df->tail_text && ln < num) {
		if (tr == global_df->top_win)
			count = 0;
		if (count > -1 && CMP_VIS_ID(tr->flag))
			count++;
		if (isNL_CFound(tr))
			isAddLineno = TRUE;
		else
			isAddLineno = FALSE;
		tr = tr->next_row;
		if (CMP_VIS_ID(tr->flag)) {
			if (isAddLineno)
				global_df->lineno++;
			global_df->wLineno++;
		}
		if (tr->line[0] == '*')
			ln++;
	}
	if (tr == global_df->top_win)
		count = 0;
	if (!CMP_VIS_ID(tr->flag)) {
		strcpy(global_df->err_message, "-Showing previous selected tier");
		if (!AtTopEnd(tr, global_df->head_text, FALSE)) {
			tr = ToPrevRow(tr, FALSE);
		} else {
			strcpy(global_df->err_message, "-Showing next selected tier");
			if (!AtBotEnd(tr, global_df->tail_text, FALSE)) {
				tr = ToNextRow(tr, FALSE);
				if (count > -1 && CMP_VIS_ID(tr->flag))
					count++;
			} else
				strcpy(global_df->err_message, "+Can't find any selected tier");
		}
	} else
		strcpy(global_df->err_message, DASHES);

	global_df->col_win = 0L;
	global_df->col_chr = 0L;
	global_df->row_txt = tr;
	if (global_df->row_txt == global_df->cur_line)
		global_df->col_txt = global_df->head_row->next_char;
	global_df->row_win = (long)count;
	if (count < 0 || count >= global_df->EdWinSize) {
		FindMidWindow();
		DisplayTextWindow(NULL, refresh);
	}

	*sp = EOS;
	GetCurCode();
	FindRightCode(1);
}

void MoveToLine(long num, char refresh) {
	int count = -1;
	char isAddLineno;
	ROWS *tr = global_df->head_text->next_row;

	global_df->lineno  = 1L;
	global_df->wLineno = 1L;
	while (tr->next_row != global_df->tail_text && global_df->lineno < num) {
		if (tr == global_df->top_win)
			count = 0;
		if (count > -1 && CMP_VIS_ID(tr->flag))
			count++;

		if (isNL_CFound(tr))
			isAddLineno = TRUE;
		else
			isAddLineno = FALSE;
		tr = tr->next_row;
		if (CMP_VIS_ID(tr->flag)) {
			if (isAddLineno)
				global_df->lineno++;
			global_df->wLineno++;
		}
	}
	if (tr == global_df->top_win)
		count = 0;
	if (!CMP_VIS_ID(tr->flag)) {
		strcpy(global_df->err_message, "-Showing previous selected tier");
		if (!AtTopEnd(tr, global_df->head_text, FALSE)) {
			tr = ToPrevRow(tr, FALSE);
		} else {
			strcpy(global_df->err_message, "-Showing next selected tier");
			if (!AtBotEnd(tr, global_df->tail_text, FALSE)) {
				tr = ToNextRow(tr, FALSE);
				if (count > -1 && CMP_VIS_ID(tr->flag))
					count++;
			} else
				strcpy(global_df->err_message, "+Can't find any selected tier");
		}
	} else
		strcpy(global_df->err_message, DASHES);

	global_df->col_win = 0L;
	global_df->col_chr = 0L;
	global_df->row_txt = tr;
	if (global_df->row_txt == global_df->cur_line)
		global_df->col_txt = global_df->head_row->next_char;
	global_df->row_win = (long)count;
	if (count < 0 || count >= global_df->EdWinSize) {
		FindMidWindow();
		DisplayTextWindow(NULL, refresh);
	}
	
	*sp = EOS;
	GetCurCode();
	FindRightCode(1);
}

int GotoLine(int i) {
	long num;
#ifdef _MAC_CODE
	DrawSoundCursor(0);
	if (i > -1)
		SetDefKeyScript();
#endif
	if (i > -1) {
#if defined(_MAC_CODE)
		num = DoLineNumber(global_df->lineno);
#elif defined(_WIN32) // _MAC_CODE
		CGotoLineNumber dlg;
	
		if (global_df == NULL)
			return(12);
		DrawCursor(1);
		dlg.m_LineNumber = global_df->lineno;
		if (dlg.DoModal() == IDOK) {
			DrawCursor(0);
			num = dlg.m_LineNumber;
		} else {
			DrawCursor(0);
			RemoveLastUndo();
			strcpy(global_df->err_message,DASHES);
			return(12);
		}
#else /* _WIN32 */
		int len;

		strcpy(sp,"Line number: ");
		len = strlen(sp);
		if (!new_getstr(sp,len,NULL)) {
			strcpy(global_df->err_message,DASHES);
			*sp=EOS;
			return(12);
		}
		if (sp[len] == EOS) {strcpy(global_df->err_message,DASHES); *sp= EOS; return(12);}
		num = atol(sp+len);
		if (num < 1L) {
			strcpy(global_df->err_message, "+Please specify line number.");
			*sp = EOS;
			return(12);
		}
#endif /* else !_MAC_CODE || !_WIN32 */
		MoveToLine(num, 1);
		global_df->LeaveHighliteOn = FALSE;
		global_df->row_win2 = 0L;
		global_df->col_win2 = -2L;
		global_df->col_chr2 = -2L;
	}
	return(12);
}

void PositionCursor(void) {
	if (global_df->row_txt == global_df->cur_line) {
		LINE *tl=global_df->head_row->next_char;

		if (tl->c == '%') {
			for (; tl->c != ':' && tl != global_df->tail_row; tl=tl->next_char) {
				if (tl == global_df->col_txt) {
					global_df->col_txt = global_df->col_txt->next_char;
					global_df->col_chr++;
				}
			}
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			if (tl->c == ':') {
				if (tl== global_df->col_txt) {
					global_df->col_chr++;
					global_df->col_txt = global_df->col_txt->next_char;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				}
				tl = tl->next_char;
				if (isSpace(tl->c) && tl != global_df->tail_row) {
					while (isSpace(tl->c) && tl != global_df->tail_row) {
						if (tl == global_df->col_txt) {
							global_df->col_chr++;
							global_df->col_txt = global_df->col_txt->next_char;
						}
						tl = tl->next_char;
					}
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
					if (tl != global_df->tail_row && tl->prev_char->c != '\t') {
						if (tl == global_df->col_txt) {
							global_df->col_chr--;
							global_df->col_txt = global_df->col_txt->prev_char;
							global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
						}
						tl = tl->prev_char;
					}
				}
			}
		}
		if (tl != global_df->col_txt) {
			while (!isSpace(global_df->col_txt->c) && global_df->col_txt != global_df->tail_row) {
				global_df->col_chr++;
				global_df->col_txt = global_df->col_txt->next_char;
			}
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		}
	} else {
		int i = 0;

		if (*global_df->row_txt->line == '%') {
			for (i=0; global_df->row_txt->line[i] != ':' && global_df->row_txt->line[i]; i++) {
				if (i == global_df->col_chr)
					global_df->col_chr++;
			}
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
			if (global_df->row_txt->line[i] == ':') {
				if (i == global_df->col_chr) {
					global_df->col_chr++;
					global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
				}
				i++;
				if (isSpace(global_df->row_txt->line[i])) {
					for (; isSpace(global_df->row_txt->line[i]); i++) {
						if (i == global_df->col_chr)
							global_df->col_chr++;
					}
					global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
					if (global_df->row_txt->line[i] && global_df->row_txt->line[i-1] != '\t') {
						if (i == global_df->col_chr) {
							global_df->col_chr--;
							global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
						}
						i--;
					}
				}
			}
		}
		if (i != global_df->col_chr) {
			while (!isSpace(global_df->row_txt->line[global_df->col_chr]) && global_df->row_txt->line[global_df->col_chr]) {
				global_df->col_chr++;
			}
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		}
	}
}
