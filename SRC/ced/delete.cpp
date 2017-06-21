#include "ced.h"
#ifdef _WIN32
	#include <TextUtils.h>
#endif

/* delete begin */
static char IncreaseKillBuffer(void) {
	register long i;
	unCH *t;

	t = global_df->KillBuff;
	global_df->KBSize += 200L;
	if ((global_df->KillBuff=(unCH *)malloc((global_df->KBSize+1)*sizeof(unCH))) == NULL) {
		global_df->KBSize -= 200L;
		return(FALSE);
	}
	if (t != NULL) {
		for (i=0; i < global_df->KBIndex; i++) global_df->KillBuff[i] = t[i];
		free(t);
	}
	return(TRUE);
}

int YankKilled(int i) {
	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return(34);
	}

	if (DeleteChank(1)) {
		SaveUndoState(FALSE);
		if (global_df->KBIndex > -1L) {
			global_df->FreqCount++;
			AddText(global_df->KillBuff, EOS, 1, global_df->KBIndex+1L);
		} else
			RemoveLastUndo();
	}
	return(34);
}

int KillLine(int i) {
	char NL_F = FALSE;

	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return(33);
	}

	if ((global_df->row_win2 != 0L || global_df->col_win2 != -2) && i > -1) {
		DeleteChank(1);
		return(33);
	}

	if (global_df->LastCommand != 33) global_df->KBIndex = 0L;
	else if (++(global_df->KBIndex) == global_df->KBSize) {
		if (!IncreaseKillBuffer()) {
			global_df->KBIndex--;
			ResetUndos();
			strcpy(global_df->err_message, "+Undo; Out of main memory.");
			return(33);
		}
	}

	ChangeCurLine();
	if (global_df->col_txt->c == NL_C && global_df->col_txt->next_char == global_df->tail_row) {
		global_df->FreqCount++;
		if (i == -1) global_df->redisplay = 0;
		DeleteNextChar(-1);
		if (i == -1) global_df->redisplay = 1;
		global_df->KillBuff[global_df->KBIndex] = NL_C;
		global_df->UndoList->key = DELTKEY;
		if ((global_df->UndoList->str=(unCH *)malloc(2*sizeof(unCH))) == NULL) {
			ResetUndos();
			return(33);
		} else {
			global_df->UndoList->str[0] = NL_C;
			global_df->UndoList->str[1] = EOS;
		}
		NL_F = TRUE;
	}
	if (global_df->col_txt == global_df->tail_row) {
		if (global_df->cur_line->next_row != global_df->tail_text) {
			if (!CMP_VIS_ID(global_df->cur_line->next_row->flag)) {
				if (!NL_F) {
					global_df->KBIndex--;
					RemoveLastUndo();
				}
				strcpy(global_df->err_message, "+Can't delete next non-selected tier.");
				return(33);
			}
			global_df->FreqCount++;
			if (i == -1) global_df->redisplay = 0;
			DeleteNextChar(-1);
			if (i == -1) global_df->redisplay = 1;
			if (!NL_F) {
				if (global_df->KBIndex == 0 || global_df->KillBuff[global_df->KBIndex-1] != NL_C) {
					global_df->KillBuff[global_df->KBIndex] = SNL_C;
					global_df->UndoList->key = DELTKEY;
					if ((global_df->UndoList->str=(unCH *)malloc(2*sizeof(unCH))) == NULL) {
						ResetUndos();
					} else {
						global_df->UndoList->str[0] = SNL_C;
						global_df->UndoList->str[1] = EOS;
					}
				} else {
					global_df->KBIndex--;
					RemoveLastUndo();
				}
			}
		} else if (!NL_F) {
			global_df->KBIndex--;
			RemoveLastUndo();
		}
	} else {
		long d, TKBIndex = global_df->KBIndex;

		global_df->FreqCount++;
		global_df->redisplay = 0;
		while (1) {
			if (global_df->col_txt->next_char == global_df->tail_row ||
				global_df->col_txt->next_char->c == NL_C) break;
			global_df->KillBuff[global_df->KBIndex] = global_df->col_txt->c;
			DeleteNextChar(-1);
			if (++(global_df->KBIndex) == global_df->KBSize) {
				if (!IncreaseKillBuffer()) {
					global_df->KBIndex--;
					ResetUndos();
					strcpy(global_df->err_message, "+Undo; Out of main memory.");
					return(33);
				}
			}
		}
		global_df->KillBuff[global_df->KBIndex] = global_df->col_txt->c;
		if (i != -1) global_df->redisplay = 1;
		DeleteNextChar(-1);
		if (i == -1) global_df->redisplay = 1;
		global_df->UndoList->key = DELTKEY;
		if ((global_df->UndoList->str =
					(unCH *)malloc(((global_df->KBIndex-TKBIndex)+2L)*sizeof(unCH))) == NULL) {
			ResetUndos();
		} else {
			for (d=0; TKBIndex <= global_df->KBIndex; d++, TKBIndex++)
				global_df->UndoList->str[d] = global_df->KillBuff[TKBIndex];
			global_df->UndoList->str[d] = EOS;
		}
	}
	return(33);
}

static void KillBuffer_shiftright(unCH *st, long len, int num) {
	for (; len >= 0; len--)
		st[len+num] = st[len];
}

int DeletePrevWord(int i) {
	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return(36);
	}

	if ((global_df->row_win2 != 0L || global_df->col_win2 != -2) && i > -1) {
		DeleteChank(1);
		return(36);
	}

	if (global_df->LastCommand != 35 && global_df->LastCommand != 36) global_df->KBIndex = 0L;
	else if (++(global_df->KBIndex) == global_df->KBSize) {
		if (!IncreaseKillBuffer()) {
			global_df->KBIndex--;
			ResetUndos();
			strcpy(global_df->err_message, "+Undo; Out of main memory.");
			return(36);
		}
	}

	ChangeCurLine();
	if (global_df->col_txt->prev_char == global_df->head_row) {
		if (global_df->cur_line->prev_row != global_df->head_text) {
			if (!CMP_VIS_ID(global_df->cur_line->prev_row->flag)) {
				global_df->KBIndex--;
				RemoveLastUndo();
				strcpy(global_df->err_message,"+Can't delete previous non-selected tier.");
				return(36);
			}
			global_df->FreqCount++;
			KillBuffer_shiftright(global_df->KillBuff,global_df->KBIndex-1,1);
			global_df->KillBuff[0] = SNL_C;
			DeletePrevChar(-1);
			if (i != -1) {
				global_df->UndoList->key = DELTKEY;
				if ((global_df->UndoList->str=(unCH *)malloc(2*sizeof(unCH))) == NULL) {
					ResetUndos();
				} else {
					global_df->UndoList->str[0] = SNL_C;
					global_df->UndoList->str[1] = EOS;
				}
			}
		} else {
			global_df->KBIndex--;
			RemoveLastUndo();
		}
	} else {
		long d, TKBIndex = global_df->KBIndex;
		char sp='\001', hc = FALSE;

		global_df->FreqCount++;
		if (global_df->KBIndex > 0 && global_df->KillBuff[0] == SNL_C && global_df->LastCommand == 36 && 
			global_df->col_txt->prev_char->c == NL_C && global_df->UndoList->NextUndo->key == DELTKEY) {
			RemoveLastUndo();
			if (global_df->UndoList->str) free(global_df->UndoList->str);
			global_df->UndoList->str = NULL;
		}
		global_df->redisplay = 0;
		while (global_df->col_txt->prev_char->prev_char != global_df->head_row) {
			if (global_df->col_txt->prev_char->c == HIDEN_C)
				hc = !hc;

			if (!hc && isSpace(global_df->col_txt->prev_char->c)) {
				if (sp == '\0') break;
			} else sp = '\0';
			KillBuffer_shiftright(global_df->KillBuff,global_df->KBIndex-1L,1);
			global_df->KillBuff[0] = global_df->col_txt->prev_char->c;
			DeletePrevChar(-1);
			if (++(global_df->KBIndex) == global_df->KBSize) {
				if (!IncreaseKillBuffer()) {
					global_df->KBIndex--;
					ResetUndos();
					strcpy(global_df->err_message, "+Undo; Out of main memory.");
					return(36);
				}
			}
		}
		KillBuffer_shiftright(global_df->KillBuff,global_df->KBIndex-1L,1);
		global_df->KillBuff[0] = global_df->col_txt->prev_char->c;
		global_df->redisplay = 1;
		DeletePrevChar(-1);
		if (i != -1) {
			global_df->UndoList->key = DELTKEY;
			if ((global_df->UndoList->str =
					(unCH *)malloc(((global_df->KBIndex-TKBIndex)+2L)*sizeof(unCH))) == NULL) {
				ResetUndos();
			} else {
				for (d=0; TKBIndex <= global_df->KBIndex; d++, TKBIndex++)
					global_df->UndoList->str[d] = global_df->KillBuff[d];
				global_df->UndoList->str[d] = EOS;
			}
		}
	}
	return(36);
}

int DeleteNextWord(int i) {
	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return(35);
	}

	if ((global_df->row_win2 != 0L || global_df->col_win2 != -2) && i > -1) {
		DeleteChank(1);
		return(35);
	}

repeat_DeleteNextWord:

	if (global_df->LastCommand != 35 && global_df->LastCommand != 36) global_df->KBIndex = 0L;
	else if (++(global_df->KBIndex) == global_df->KBSize) {
		if (!IncreaseKillBuffer()) {
			global_df->KBIndex--;
			ResetUndos();
			strcpy(global_df->err_message, "+Undo; Out of main memory.");
			return(35);
		}
	}

	ChangeCurLine();
	if (global_df->col_txt == global_df->tail_row) {
		if (global_df->cur_line->next_row != global_df->tail_text) {
			if (!CMP_VIS_ID(global_df->cur_line->next_row->flag)) {
				global_df->KBIndex--;
				RemoveLastUndo();
				strcpy(global_df->err_message, "+Can't delete next non-selected tier.");
				return(35);
			}
			global_df->FreqCount++;
			global_df->KillBuff[global_df->KBIndex] = SNL_C;
			DeleteNextChar(-1);
			if (global_df->KBIndex == 0 || global_df->KillBuff[global_df->KBIndex-1] != NL_C || 
																		global_df->ShowParags == '\001') {
				global_df->UndoList->key = DELTKEY;
				if ((global_df->UndoList->str=(unCH *)malloc(2*sizeof(unCH))) == NULL) {
					ResetUndos();
				} else {
					global_df->UndoList->str[0] = SNL_C;
					global_df->UndoList->str[1] = EOS;
				}
			} else {
				RemoveLastUndo();
			}
		} else {
			global_df->KBIndex--;
			RemoveLastUndo();
		}
	} else {
		long d, TKBIndex = global_df->KBIndex;
		char sp='\001', hc = FALSE;

		global_df->FreqCount++;
		global_df->redisplay = 0;
		while (global_df->col_txt->next_char != global_df->tail_row) {
			if (global_df->col_txt->c == HIDEN_C)
				hc = !hc;

			if (!isSpace(global_df->col_txt->c))
				sp = '\0';
			if ((isSpace(global_df->col_txt->next_char->c) ||
					 global_df->col_txt->next_char->c == NL_C) && !hc && sp == '\0') break;

			global_df->KillBuff[global_df->KBIndex] = global_df->col_txt->c;
			DeleteNextChar(-1);
			if (++(global_df->KBIndex) == global_df->KBSize) {
				if (!IncreaseKillBuffer()) {
					global_df->KBIndex--;
					ResetUndos();
					strcpy(global_df->err_message, "+Undo; Out of main memory.");
					return(35);
				}
			}
		}
		global_df->KillBuff[global_df->KBIndex] = global_df->col_txt->c;
		sp = (char)global_df->col_txt->c;
		if (sp != NL_C) {
			global_df->redisplay = 1;
		}
		DeleteNextChar(-1);
		global_df->UndoList->key = DELTKEY;
		if ((global_df->UndoList->str =
					(unCH *)malloc(((global_df->KBIndex-TKBIndex)+2L)*sizeof(unCH))) == NULL) {
			ResetUndos();
		} else {
			for (d=0; TKBIndex <= global_df->KBIndex; d++, TKBIndex++)
				global_df->UndoList->str[d] = global_df->KillBuff[TKBIndex];
			global_df->UndoList->str[d] = EOS;
		}
		if (sp == NL_C) {
			global_df->LastCommand = 35;
			SaveUndoState(FALSE);
			global_df->redisplay = 1;
			goto repeat_DeleteNextWord;
		}
	}
	return(35);
}

int DeletePrevChar(int i) {
	register char AlreadyChanged;
	register char DisplayAll = FALSE;

	cedDFnt.isUTF = global_df->isUTF;
	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return(23);
	}

	if ((global_df->row_win2 != 0L || global_df->col_win2 != -2) && i > -1) {
		DeleteChank(1);
		return(23);
	}

	AlreadyChanged = (char)(global_df->row_txt != global_df->cur_line);
	ChangeCurLine();
repeat_del_prev_char:
//	TRUE_CHECK_ID1(global_df->row_txt->flag);
//	TRUE_CHECK_ID2(global_df->row_txt->flag);
	if (global_df->col_txt->prev_char != global_df->head_row) {
		LINE *tl;

		if (i != -1) {
			char hc = FALSE;
			long num = 0L, j;
			short res;

			if (!AlreadyChanged)
				ChangeCurLineAlways(0);
			do {
				num++;
				if (global_df->row_txt->line[global_df->col_chr-num] == HIDEN_C && global_df->ShowParags != '\002')
					hc = !hc;

				if (hc)
					res = 1;
				else
#ifdef _UNICODE
					res = 0;
#else
					res = (int)my_CharacterByteType(global_df->row_txt->line, global_df->col_chr-num, &cedDFnt);
#endif
			} while (res != 0 && res != -1 && global_df->col_chr-num != 0) ;
			if (DisplayAll)
				global_df->UndoList->key = DELTRPT;
			else
				global_df->UndoList->key = DELTKEY;
			if ((global_df->UndoList->str=(unCH *)malloc((num+1L)*sizeof(unCH))) == NULL)
				mem_err(TRUE, global_df);
			for (j=num-1L; j >= 0L; j--) {
				tl = global_df->col_txt->prev_char;
				global_df->col_txt->prev_char = tl->prev_char;
				tl->prev_char->next_char = global_df->col_txt;
				global_df->UndoList->str[j] = tl->c;
				free(tl);
			}
			global_df->head_row_len -= num;
			global_df->col_chr -= num;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			global_df->UndoList->str[num] = EOS;
		} else {
			tl = global_df->col_txt->prev_char;
			global_df->col_txt->prev_char = tl->prev_char;
			tl->prev_char->next_char = global_df->col_txt;
			free(tl);
			global_df->head_row_len--;
			global_df->col_chr--;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		}
		if (global_df->redisplay) {
			if (DisplayAll) {
				if (global_df->ChatMode || global_df->AutoWrap) {
					if (isCallFixLine(0L, global_df->head_row_len, TRUE) && i != -1) 
						FixLine(TRUE);
				}
				DisplayTextWindow(NULL, 1);
			} else
				DisplayTextWindow(global_df->cur_line, 1);
		}
	} else if (global_df->cur_line->prev_row != global_df->head_text) {
		ROWS *tr;

		if (!CMP_VIS_ID(global_df->cur_line->prev_row->flag)) {
			RemoveLastUndo();
			strcpy(global_df->err_message, "+Can't delete previous non-selected tier.");
			return(23);
		}

		tr = global_df->row_txt;
		global_df->row_txt = global_df->row_txt->prev_row;
		global_df->lineno--;
		global_df->cur_line = global_df->row_txt;
		global_df->col_txt = global_df->head_row;
		CpCur_lineToHead_row(global_df->cur_line);
		global_df->col_txt = global_df->col_txt->next_char;
		global_df->row_txt->next_row = tr->next_row;
		tr->next_row->prev_row = global_df->row_txt;
		free(tr);
		if (global_df->numberOfRows)
			global_df->numberOfRows--;
		global_df->col_chr = strlen(global_df->row_txt->line);
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		if (--global_df->row_win < 0L) {
			global_df->row_win++;
			global_df->top_win = global_df->row_txt;
		}
		if (i != -1) {
			if (global_df->col_txt->prev_char != global_df->head_row &&
					global_df->col_txt->prev_char->c == NL_C && global_df->ShowParags != '\001') {
				DisplayAll = TRUE;
				goto repeat_del_prev_char;
			}
			global_df->UndoList->key = DELTKEY;
			if ((global_df->UndoList->str=(unCH *)malloc(2*sizeof(unCH))) == NULL)
				mem_err(TRUE, global_df);
			global_df->UndoList->str[0] = SNL_C;
			global_df->UndoList->str[1] = EOS;
		}
		if (global_df->ChatMode || global_df->AutoWrap) {
			if (isCallFixLine(0L, global_df->head_row_len, TRUE) && i != -1) 
				FixLine(TRUE);
		}
		if (global_df->redisplay)
			DisplayTextWindow(NULL, 1);
	} else if (i > -1)
		RemoveLastUndo();
    return(23);
}

int DeleteNextChar(int i) {
	register char AlreadyChanged;
	register char dc;
	LINE *tl;

	cedDFnt.isUTF = global_df->isUTF;
	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return(32);
	}

	if ((global_df->row_win2 != 0L || global_df->col_win2 != -2) && i > -1) {
		DeleteChank(1);
		return(32);
    }

	dc = 0;
	AlreadyChanged = (char)(global_df->row_txt != global_df->cur_line);
	ChangeCurLine();
//	TRUE_CHECK_ID1(global_df->row_txt->flag);
//	TRUE_CHECK_ID2(global_df->row_txt->flag);
repeat_del_next_char:
	if (global_df->col_txt != global_df->tail_row) {
		if (i != -1) {
			char hc = FALSE;
			long num = 0L;
			short res;

			dc = (char)global_df->col_txt->c;
			if (!AlreadyChanged)
				ChangeCurLineAlways(0);
			do {
				if (global_df->row_txt->line[global_df->col_chr+num] == HIDEN_C && global_df->ShowParags != '\002')
					hc = !hc;

				num++;
				if (hc)
					res = 1;
				else
#ifdef _UNICODE
					res = 0;
#else
					res = (int)my_CharacterByteType(global_df->row_txt->line, global_df->col_chr+num, &cedDFnt);
#endif
			} while (res != 0 && res != -1) ;
			global_df->UndoList->key = DELTKEY;
			if ((global_df->UndoList->str=(unCH *)malloc((num+1L)*sizeof(unCH))) == NULL)
				mem_err(TRUE, global_df);
			for (i=0; i < num; i++) {
				global_df->UndoList->str[i] = global_df->col_txt->c;
				tl = global_df->col_txt->prev_char;
				tl->next_char = global_df->col_txt->next_char;
				global_df->col_txt->next_char->prev_char = tl;
				free(global_df->col_txt);
				global_df->col_txt = tl->next_char;
			}
			global_df->head_row_len -= num;
			global_df->UndoList->str[i] = EOS;
			if (dc == NL_C && global_df->ShowParags != '\001') {
				goto repeat_del_next_char;
			}
		} else {
			tl = global_df->col_txt->prev_char;
			tl->next_char = global_df->col_txt->next_char;
			global_df->col_txt->next_char->prev_char = tl;
			free(global_df->col_txt);
			global_df->col_txt = tl->next_char;
			global_df->head_row_len--;
		}
		if (global_df->redisplay)
			DisplayTextWindow(global_df->cur_line, 1);
	} else if (global_df->cur_line->next_row != global_df->tail_text) {
		ROWS *tr;

		if (!CMP_VIS_ID(global_df->cur_line->next_row->flag)) {
			RemoveLastUndo();
			strcpy(global_df->err_message, "+Can't delete next non-selected tier.");
			return(32);
		}
		if (i != -1 && dc != NL_C && global_df->UndoList->NextUndo) {
			global_df->UndoList->key = DELTKEY;
			if ((global_df->UndoList->str=(unCH *)malloc(2*sizeof(unCH))) == NULL)
				mem_err(TRUE, global_df);
			global_df->UndoList->str[0] = SNL_C;
			global_df->UndoList->str[1] = EOS;
		}
		tr = global_df->row_txt->next_row;
		global_df->col_txt = global_df->col_txt->prev_char;
		tl = global_df->col_txt;
		CpCur_lineToHead_row(global_df->row_txt->next_row);
		global_df->col_txt = tl->next_char;	
		global_df->row_txt->next_row = tr->next_row;
		tr->next_row->prev_row = global_df->row_txt;
		if (global_df->col_txt->prev_char == global_df->head_row) {
			SetFontName(global_df->row_txt->Font.FName, tr->Font.FName);
			global_df->row_txt->Font.FSize = tr->Font.FSize;
			global_df->row_txt->Font.CharSet = tr->Font.CharSet;
			global_df->row_txt->Font.FHeight = tr->Font.FHeight;
		}
		free(tr);
		if (global_df->numberOfRows)
			global_df->numberOfRows--;
		if (global_df->ChatMode || global_df->AutoWrap) {
			if (isCallFixLine(0L, global_df->head_row_len, TRUE) && i != -1)
				FixLine(TRUE);
		}
		if (global_df->redisplay)
			DisplayTextWindow(NULL, 1);
	} else if (i > -1)
		RemoveLastUndo();
	return(32);
}

char DeleteChank(int d) {
	register long len;
	register long i;
	long tn;
	ROWS *st;

	if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode)) {
		strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		return(FALSE);
	}

	if (global_df->row_win2 == 0L && global_df->col_win2 == -2) {
		RemoveLastUndo();
		return(TRUE);
	}

	global_df->KBIndex = -1L;
	i = (long)global_df->row_win;
	if (global_df->row_win2 < 0) {
		global_df->lineno += global_df->row_win2;
		global_df->row_win2 += global_df->row_win;
		tn = (long)global_df->row_win;
		global_df->row_win = global_df->row_win2;
		global_df->row_win2 = tn;
		global_df->row_win2 -= global_df->row_win;
		tn = global_df->col_win;
		global_df->col_win = global_df->col_win2;
		global_df->col_win2 = tn;
		tn = global_df->col_chr;
		global_df->col_chr = global_df->col_chr2;
		global_df->col_chr2 = tn;
		for (tn=global_df->row_win2; tn && !AtTopEnd(global_df->row_txt,global_df->head_text,FALSE); tn--) {
			i--;
			global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
		}
	} else if (global_df->row_win2 == 0 && global_df->col_chr > global_df->col_chr2) {
		tn = global_df->col_win;
		global_df->col_win = global_df->col_win2;
		global_df->col_win2 = tn;
		tn = global_df->col_chr;
		global_df->col_chr = global_df->col_chr2;
		global_df->col_chr2 = tn;
	}

	ChangeCurLineAlways(0);
	global_df->row_win = i;
	if (i < 0) {
		global_df->row_win = i;
		FindMidWindow();
		DisplayTextWindow(NULL, 1);
	}

	if (global_df->row_win2 == 0) {
		len = (long)(global_df->col_chr2 - global_df->col_chr);
	} else {
		st = global_df->row_txt;
		len = (long)(strlen(st->line) - global_df->col_chr + 1);
		tn = global_df->row_win2 - 1;
		if (tn) {
			do {
				st = st->next_row;
				tn--;
				if (!CMP_VIS_ID(st->flag)) {
					RemoveLastUndo();
					strcpy(global_df->err_message, "+Can't delete non-selected tiers");
					global_df->row_win2 = 0L;
					global_df->col_win2 = -2L;
					global_df->col_chr2 = -2L;
					return(FALSE);
				}
				len += (long)(strlen(st->line) + 1);
			} while (tn) ;
		}
		len += global_df->col_chr2;
	}

	global_df->redisplay = 0;
	while (global_df->KBSize < len) {
		if (!IncreaseKillBuffer()) {
			ResetUndos();
			strcpy(global_df->err_message, "+Undo; Out of main memory.");
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
			return(FALSE);
		}
	}
	for (; len > 0; len--) {
		if (len == 1) {
			if (global_df->col_txt->c == NL_C && 
				global_df->col_txt->next_char == global_df->tail_row && 
				global_df->ShowParags != '\001') break;
		}
		global_df->KBIndex++;
		if (global_df->col_txt == global_df->tail_row) {
			if (global_df->KBIndex > 0 &&
						(global_df->KillBuff[global_df->KBIndex-1] == SNL_C ||
						 global_df->KillBuff[global_df->KBIndex-1] == NL_C))
				global_df->KBIndex--;
			else
				global_df->KillBuff[global_df->KBIndex] = SNL_C;
		} else if (global_df->col_txt->c == NL_C && global_df->ChatMode)
			global_df->KillBuff[global_df->KBIndex] = SNL_C;
		else
			global_df->KillBuff[global_df->KBIndex] = global_df->col_txt->c;
		DeleteNextChar(-1);
	}
	global_df->redisplay = 1;
	if (d != -1 && global_df->UndoList->NextUndo) {
		global_df->UndoList->key = DELTKEY;
		if ((global_df->UndoList->str=(unCH *)malloc((global_df->KBIndex+2L)*sizeof(unCH))) == NULL) {
			ResetUndos();
		} else {
			for (i=0; i <= global_df->KBIndex; i++) 
				global_df->UndoList->str[i] = global_df->KillBuff[i];
			global_df->UndoList->str[i] = EOS;
		}
	}
	if (global_df->ChatMode || global_df->AutoWrap) {
		if (isCallFixLine(0L, global_df->head_row_len, TRUE) && global_df->UndoList->key == INSTKEY) 
			FixLine(FALSE);
	}
	if (FreqCountLimit || global_df->FreqCountLimit)
		global_df->FreqCount++;
	DisplayTextWindow(NULL, 1);
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;
	return(TRUE);
}
/* delete end */
