#include "ced.h"
#include "c_clan.h"
#include <process.h>

extern char *nameOverride, *pathOverride;
extern char *lineNumFname;
extern char isSpOverride;
extern long lineNumOverride;

char isAjustCursor = TRUE;

int VisitFile(int i) {
	isAjustCursor = TRUE;
	::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_OPEN, NULL);
	return(59);
}

static void lExtractFileName(unCH *line, FNType *fname, char term) {
	long i;
	FNType mFileName[FNSize];

	for (i=0L; line[i] != term && line[i] != EOS; i++) ;
	if (line[i] == term) {
		line[i] = EOS;
		if (!strcmp(line, "Stdin"))
			return;

		if (!isRefEQZero(line)) {
			u_strcpy(fname, line, FNSize);
		} else {
			u_strcpy(mFileName, line, FNSize);
			extractPath(fname, global_df->fileName);
			addFilename2Path(fname, mFileName);
			if (access(fname, 0)) {
				strcpy(fname, wd_dir);
				addFilename2Path(fname, mFileName);
			}
		}
		strcpy(line, line+i+1);
	}
}

char FindFileLine(char isTest, char *message) {
	char fname[FILENAME_MAX];
	char isZeroFound = FALSE, isSpeakerNumber;
	unCH *line;
	char *t;
	unCH u_fname[FNSize];
	long len;
	long ln = 0L;
	LINE *tcol;
	extern char tFileBuf[];
	extern char isDontAskDontTell;

	if (global_df->row_txt == global_df->cur_line) {
		tcol = global_df->head_row->next_char;
		for (len=0; tcol->c != NL_C && tcol != global_df->tail_row; tcol=tcol->next_char) {
			ced_line[len++] = tcol->c;
		}
		ced_line[len] = EOS;
	} else {
		strncpy(ced_line, global_df->row_txt->line, UTTLINELEN);
		ced_line[UTTLINELEN] = EOS;
	}
	if (ced_line[0] == '0') {
		isZeroFound = TRUE;
		strcpy(ced_line, ced_line+1);
	}
	len = (long)strlen("@Comment:	");
	if (strncmp(ced_line, "@Comment:	", (size_t)len) == 0)
		strcpy(ced_line, ced_line+len);
	line = ced_line;
	*fname = EOS;	
	if (strncmp(line, "From file <", (size_t)11) == 0) {
		strcpy(line, line+11);
		lExtractFileName(line, fname, '>');
	} else if (strncmp(line, "Output file <", (size_t)13) == 0) {
		strcpy(line, line+13);
		lExtractFileName(line, fname, '>');
	} else if (*line == '*' || *line == ' ') {
		for (len=0L; line[len] == '*'; len++) ;
		for (; line[len] == ' '; len++) ;
		strcpy(line, line+len);
		len = strlen("File \"");
		if (strncmp(line, "File \"", len) == 0) {
			strcpy(line, line+len);
			lExtractFileName(line, fname, '"');
		}
		if (*fname == EOS)
			return(FALSE);
		for (len=0L; line[len] == ':' || line[len] == ' '; len++) ;
		if (len > 0L)
			strcpy(line, line+len);
		len = strlen("line ");
		if (strncmp(line, "line ", len) == 0) {
			strcpy(line, line+len);
			ln = uS.atol(line);
			isSpeakerNumber = FALSE;
		} else {
			len = strlen("speaker ");
			if (strncmp(line, "speaker ", len) == 0) {
				strcpy(line, line + len);
				ln = uS.atol(line);
				isSpeakerNumber = TRUE;
			}
		}
	} else
		return(FALSE);

	if (*fname != EOS && !isTest) {
		if (access(fname, 0)) {
			sprintf(templineC, "Can't open file \"%s\".", fname);
			do_warning(templineC, 0);
			return(FALSE);
		}

		isAjustCursor = FALSE;
		t = strrchr(fname, '.');
		if (t != NULL && !strcmp(t, ".xls")) {
			strcpy(FileName2, "start \"Excel\" \"");
			strcat(FileName2, fname);
			strcat(FileName2, "\"");
			u_strcpy(u_fname, FileName2, FNSize);
			
//			if (_wspawnl(_P_NOWAITO, u_fname, u_fname, _T(""), NULL))
//			if (!_wsystem(u_fname))
//			if (!_wexecl(u_fname, u_fname, NULL))
			_wsystem(u_fname);
				return(TRUE);
		}

		DrawCursor(0);
		global_df->DrawCur = FALSE;
		strcpy(tFileBuf, fname);
		t = strrchr(tFileBuf,'\\');
		pathOverride = wd_dir;
		if (t != NULL) {
			*t = EOS;
			strcpy(templineC3, tFileBuf);
			*t = '\\';
			strcat(templineC3, "\\");
			pathOverride = templineC3;
			nameOverride = t+1;
		} else
			nameOverride = tFileBuf;
		lineNumFname = tFileBuf;
		lineNumOverride = ln;
		isSpOverride = isSpeakerNumber;
		if (isZeroFound)
			isDontAskDontTell = TRUE;
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW, NULL);
	}
	return(TRUE);
}

void RefreshOtherCursesWindows(char all) {
	WINDOW *t;

	if (global_df == NULL) return;
	if (global_df->RootWindow == NULL) return;
	if (all) RefreshScreen(-1);
	global_df->WinChange = FALSE;
	for (t=global_df->RootWindow; t != NULL; t=t->NextWindow) {
		if (t != global_df->w1 && t != global_df->w2 && t != global_df->wm) {
			touchwin(t);
			wrefresh(t);
		}
	}
    global_df->WinChange = TRUE;
}

/* Scroll begin */
void SetScrollControl(void) {
	int  i;
	long len, shiftLen, t;
	long colWin, newcol;
	double max, shiftMax, width;
	ROWS *win;
	LINE *tl;
	RECT theRect;
	CSize tw;
	CWnd *gWin;
	LOGFONT lfFont;
	CFont l_font;
	SCROLLINFO sb;

	if (global_df == NULL || GlobalDC == NULL)
		return;
	if (GlobalDC->GetWindow() == NULL)
		return;

	sb.cbSize = sizeof(SCROLLINFO);
	sb.fMask = SIF_DISABLENOSCROLL | SIF_RANGE;
	sb.nMin = 0;

// Vertical bar
	if (AtBotEnd(global_df->head_text, global_df->top_win, FALSE) && 
						global_df->numberOfRows <= global_df->EdWinSize) {
		if (global_df->VScrollBar) {
			sb.nMax = 0;
			GlobalDC->GetWindow()->SetScrollInfo(SB_VERT, &sb, TRUE);
			global_df->VScrollBar = FALSE;
		}
	} else {
		if (global_df->wLineno == 1)
			max = (double)(global_df->window_rows_offset * 100L) / global_df->numberOfRows;
		else
			max = (double)((global_df->wLineno + global_df->window_rows_offset) * 100L) / global_df->numberOfRows;
		GlobalDC->GetWindow()->SetScrollPos(SB_VERT, (int)max, (BOOL)global_df->VScrollBar);
		if (!global_df->VScrollBar) {
			sb.nMax = 100;
			GlobalDC->GetWindow()->SetScrollInfo(SB_VERT, &sb, TRUE);
			global_df->VScrollBar = TRUE;
		}
	}

	if (global_df->ScrollBar == '\255')
		return;

// Horizontal bar
	if (global_df->SoundWin) {
		max = (double)global_df->SnTr.SoundFileSize;
		if (max < (double)global_df->SnTr.EndF-global_df->SnTr.BegF) {
			if (global_df->HScrollBar) {
				sb.nMax = 0;
				GlobalDC->GetWindow()->SetScrollInfo(SB_HORZ, &sb, TRUE);
				global_df->HScrollBar = FALSE;
			}
		} else {
			max = (double)(global_df->SnTr.WBegF * 100L) / max;
			if (!global_df->HScrollBar) {
				sb.nMax = 100;
				GlobalDC->GetWindow()->SetScrollInfo(SB_HORZ, &sb, TRUE);
				global_df->HScrollBar = TRUE;
			}	
			GlobalDC->GetWindow()->SetScrollPos(SB_HORZ, (int)max, (BOOL)global_df->HScrollBar);
		}
	} else {
		max = 0.0000;
		shiftMax = 0.0000;
		if (global_df->top_win == global_df->head_text)
			win = ToNextRow(global_df->top_win, FALSE);
		else
			win = global_df->top_win;
		gWin = GlobalDC->GetWindow();
		if (!gWin)
			return;
		gWin->GetClientRect(&theRect);		
		if (global_df == NULL || global_df->w1 == NULL)
			width = (double)(theRect.right - theRect.left - LEFTMARGIN - getNumberOffset() - SCROLL_BAR_SIZE);
		else
			width = (double)(theRect.right - theRect.left - LEFTMARGIN - global_df->w1->textOffset - SCROLL_BAR_SIZE);
		for (i=0; i < global_df->EdWinSize && win!=global_df->tail_text; i++, win=ToNextRow(win,FALSE)) {
			SetLogfont(&lfFont, &win->Font, NULL);
			l_font.CreateFontIndirect(&lfFont);
			CFont* pOldFont = GlobalDC->SelectObject(&l_font);
			shiftLen = 0L;
			colWin = 0L;
			len = 0L;
			if (win == global_df->cur_line) {
				for (tl=global_df->head_row->next_char; 
						tl != global_df->tail_row && len < UTTLINELEN-1; tl=tl->next_char) {
					if (tl->c == '\t') {
						newcol = (((colWin / TabSize) + 1) * TabSize);
						for (; colWin < newcol; colWin++) 
							templine4[len++] = ' ';
					} else {
						templine4[len] = tl->c;
						colWin++;
						len++;
					}
				}
			} else {
				for (t=0L; win->line[t] != EOS && len < UTTLINELEN-1; t++) {
					if (win->line[t] == '\t') {
						newcol = (((colWin / TabSize) + 1) * TabSize);
						for (; colWin < newcol; colWin++) 
							templine4[len++] = ' ';
					} else {
						templine4[len++] = win->line[t];
						colWin++;
					}
				}
			}
			templine4[len] = EOS;
			if (global_df->LeftCol > 0L) {
				shiftLen = TextWidthInPix(templine4, 0, ((global_df->LeftCol >= len) ? len : global_df->LeftCol), &win->Font, 0);
			}
			len = TextWidthInPix(templine4, 0, len, &win->Font, 0);
			if (max < (double)len)
				max = (double)len;
			if (shiftMax < (double)shiftLen)
				shiftMax = (double)shiftLen;
			GlobalDC->SelectObject(pOldFont);
			l_font.DeleteObject();
		}
		if (global_df->LeftCol == 0 && max < width) {
			if (global_df->HScrollBar) {
				sb.nMax = 0;
				GlobalDC->GetWindow()->SetScrollInfo(SB_HORZ, &sb, TRUE);
				global_df->HScrollBar = FALSE;
			}
		} else {
			max = (shiftMax * 100.0000) / max;
			if (!global_df->HScrollBar) {
				sb.nMax = 100;
				GlobalDC->GetWindow()->SetScrollInfo(SB_HORZ, &sb, TRUE);
				global_df->HScrollBar = TRUE;
			}	
			GlobalDC->GetWindow()->SetScrollPos(SB_HORZ, (int)roundUp(max), (BOOL)global_df->HScrollBar);
		}
	}
}

static long Con_MoveLineUp(long orgCol_win) {
    if (!AtTopEnd(global_df->top_win, global_df->head_text, FALSE)) {
		global_df->row_win++;
		global_df->window_rows_offset--;
		global_df->top_win = ToPrevRow(global_df->top_win, FALSE);
		DisplayTextWindow(NULL, 1);
		if (global_df->row_win >= 0L && global_df->row_win < global_df->EdWinSize) 
			global_df->window_rows_offset = 0;
	} else if (global_df->row_txt != global_df->top_win && 
								global_df->row_win < (long)global_df->EdWinSize) {
		global_df->redisplay = 0;
		MoveUp(-1);
		global_df->redisplay = 1;
		orgCol_win = global_df->col_win;
	}
	return(orgCol_win);
}

static void Con_MoveLineDown(void) {
    if (!AtBotEnd(global_df->top_win, global_df->tail_text, FALSE)) {
		global_df->row_win--;
		global_df->window_rows_offset++;
		global_df->top_win = ToNextRow(global_df->top_win, FALSE);
		DisplayTextWindow(NULL, 1);
		if (global_df->row_win >= 0L && global_df->row_win < global_df->EdWinSize) 
			global_df->window_rows_offset = 0;
	}
}

static long Con_PrevPage(long orgCol_win) {
    register int num;

    if (!AtTopEnd(global_df->top_win, global_df->head_text, FALSE)) {
		num = global_df->TextWinSize - global_df->top_win->Font.FHeight;
		while (num > 0) {
			if (!AtTopEnd(global_df->top_win,global_df-> head_text, FALSE)) 
				global_df->top_win = ToPrevRow(global_df->top_win, FALSE);
			else {
				global_df->window_rows_offset = 0L - global_df->wLineno;
				break;
			}
	    	num -= global_df->top_win->Font.FHeight;
			global_df->row_win++;
			global_df->window_rows_offset--;
		}
		if (num < 0) {
			if (!AtBotEnd(global_df->top_win, global_df->tail_text, FALSE)) {
				global_df->top_win = ToNextRow(global_df->top_win, FALSE);
				global_df->row_win--;
				global_df->window_rows_offset++;
			}
		}
		if (AtTopEnd(global_df->top_win, global_df->head_text, FALSE))
			global_df->window_rows_offset = 0L - global_df->wLineno;
		if (global_df->row_win >= 0L && global_df->row_win < global_df->EdWinSize) 
			global_df->window_rows_offset = 0L;
		DisplayTextWindow(NULL, 1);
	} else {
		PrevPage(-1);
		orgCol_win = global_df->col_win;
	}
	return(orgCol_win);
}

static void Con_NextPage(void) {
	register int i;

    if (!AtBotEnd(global_df->top_win, global_df->tail_text, FALSE)) {
		for (i=1; i < global_df->EdWinSize; i++) {
			if (!AtBotEnd(global_df->top_win, global_df->tail_text, FALSE)) 
				global_df->top_win = ToNextRow(global_df->top_win, FALSE);
			else break;
			global_df->row_win--;
			global_df->window_rows_offset++;
		}
		if (global_df->row_win >= 0L && global_df->row_win < global_df->EdWinSize) 
			global_df->window_rows_offset = 0L;
		DisplayTextWindow(NULL, 1);
	}
}

static void Con_GotoLine(long num) {
	register long fp = global_df->wLineno + global_df->window_rows_offset;

	if (num < fp) {
		for (; num < fp; num++) {
			if (!AtTopEnd(global_df->top_win, global_df->head_text, FALSE)) 
				global_df->top_win = ToPrevRow(global_df->top_win, FALSE);
			else {
				global_df->window_rows_offset = 0L - global_df->wLineno;
				break;
			}
			global_df->row_win++;
			global_df->window_rows_offset--;
		}
		if (AtTopEnd(global_df->top_win, global_df->head_text, FALSE))
			global_df->window_rows_offset = 0L - global_df->wLineno;
	} else {
		for (; num > fp; num--) {
			if (!AtBotEnd(global_df->top_win, global_df->tail_text, FALSE)) 
				global_df->top_win = ToNextRow(global_df->top_win, FALSE);
			else break;
			global_df->row_win--;
			global_df->window_rows_offset++;
		}
	}
	if (global_df->row_win >= 0L && global_df->row_win < global_df->EdWinSize) 
		global_df->window_rows_offset = 0;
	DisplayTextWindow(NULL, 1);
}

void HandleVScrollBar(UINT nSBCode, UINT nPos, CWnd* win) {
	long val, orgCol_win;

	if (!global_df->ScrollBar) return;
	orgCol_win = global_df->col_win;
	global_df->col_win = global_df->LeftCol + 1;
	switch (nSBCode) {
		case SB_LINEUP:
			orgCol_win = Con_MoveLineUp(orgCol_win);
			PosAndDispl();
			break;

		case SB_LINEDOWN:
			Con_MoveLineDown();
			PosAndDispl();
			break;
	
		case SB_PAGEUP:
			if (global_df->ScrollBar == '\001') 
				orgCol_win = Con_PrevPage(orgCol_win);
			else
				orgCol_win = Con_MoveLineUp(orgCol_win);
			PosAndDispl();
			break;

		case SB_PAGEDOWN:
			if (global_df->ScrollBar == '\001') Con_NextPage();
			else Con_MoveLineDown();
			PosAndDispl();
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			val = (long)nPos;
			val = global_df->numberOfRows * val / 100L;
			Con_GotoLine(val);
			PosAndDispl();
			break;	
	}
	SetScrollControl();
	global_df->col_win = orgCol_win;
	wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
}

void HandleHScrollBar(UINT nSBCode, UINT nPos, CWnd* win) {
	int  i;
	long max, orgCol_win;
	double val;
	ROWS *row;

	if (!global_df->ScrollBar) return;
	orgCol_win = global_df->col_win;
	global_df->col_win = global_df->LeftCol + 1;
	if (global_df->SoundWin) {
		SetScrollControl();
		DrawSoundCursor(0);
	}
	switch (nSBCode) {
		case SB_LINEUP:
			if (global_df->SoundWin) {
				int rm, cm;
				Size t;

				GetSoundWinDim(&rm, &cm);
				t = ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample) / 30;
				if (global_df->SnTr.WBegF >= t) global_df->SnTr.WBegF -= t;
				else global_df->SnTr.WBegF = 0L;
				global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.WBegF, '+');
				global_df->WinChange = FALSE;
				touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
				global_df->WinChange = TRUE;
				delay_mach(1);
			} else {
				global_df->col_win = global_df->LeftCol + 1;
				if (global_df->LeftCol > 0L) {
					global_df->LeftCol--;
					DisplayTextWindow(NULL, 1);
				}
				PosAndDispl();
			}
			break;

		case SB_LINEDOWN:
			if (global_df->SoundWin) {
				int rm, cm;
				Size t, tf;

				GetSoundWinDim(&rm, &cm);
				tf = ((Size)cm * (Size)global_df->scale_row * (Size)global_df->SnTr.SNDsample);
				t = tf / 30;
				if (global_df->SnTr.WBegF+t+tf < global_df->SnTr.SoundFileSize)
					global_df->SnTr.WBegF += t;
				else
					global_df->SnTr.WBegF = global_df->SnTr.SoundFileSize - tf;
				global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.WBegF, '-');
				global_df->WinChange = FALSE;
				touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
				global_df->WinChange = TRUE;
				delay_mach(1);
			} else {
				long max, len;
				ROWS *win;
				max = 0L;
				win = global_df->top_win;
				for (i=0; i < global_df->EdWinSize && win!=global_df->tail_text; i++, win=ToNextRow(win,FALSE)) {
					if (win == global_df->cur_line) {
						len = ComColWin(FALSE, NULL, global_df->head_row_len);
						if (max < len)
							max = len;
					} else {
						len = ComColWin(FALSE, win->line, strlen(win->line));
						if (max < len)
							max = len;
					}
				}
				if (global_df->LeftCol < max+1)
					global_df->LeftCol++;
				global_df->col_win = global_df->LeftCol + 1;
				DisplayTextWindow(NULL, 1);
				PosAndDispl();
			}
			break;
	
		case SB_PAGEUP:
			if (global_df->SoundWin) {
				int rm, cm;
				Size t;
				GetSoundWinDim(&rm, &cm);
				t = global_df->SnTr.WEndF - global_df->SnTr.WBegF;
				if (global_df->SnTr.WBegF >= t) global_df->SnTr.WBegF -= t;
				else global_df->SnTr.WBegF = 0L;
				global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.WBegF, '+');
				global_df->WinChange = FALSE;
				touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
				global_df->WinChange = TRUE;
				delay_mach(1);
			} else {
				if (global_df->LeftCol > 0L) {
					global_df->LeftCol -= 10;
					if (global_df->LeftCol < 0L)
						global_df->LeftCol = 0L;
					global_df->col_win = global_df->LeftCol + 1;
					DisplayTextWindow(NULL, 1);
				}
				PosAndDispl();
			}
			break;

		case SB_PAGEDOWN:
			if (global_df->SoundWin) {
				int rm, cm;
				Size t;
				GetSoundWinDim(&rm, &cm);
				t = global_df->SnTr.WEndF - global_df->SnTr.WBegF;
				if (global_df->SnTr.WBegF+t < global_df->SnTr.SoundFileSize)
					global_df->SnTr.WBegF += t;
				else
					global_df->SnTr.WBegF = global_df->SnTr.SoundFileSize - t;
				global_df->SnTr.WBegF = AlignMultibyteMediaStream(global_df->SnTr.WBegF, '-');
				global_df->WinChange = FALSE;
				touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
				global_df->WinChange = TRUE;
				delay_mach(1);
			} else {
				global_df->LeftCol += 10;
				global_df->col_win = global_df->LeftCol + 1;
				DisplayTextWindow(NULL, 1);
				PosAndDispl();
			}
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			if (global_df->SoundWin) {
				max = global_df->SnTr.SoundFileSize;
				val = (double)nPos;
				val = (double)max * (val / 100.0000);
				global_df->SnTr.WBegF = AlignMultibyteMediaStream((long)val, '-');
				global_df->WinChange = FALSE;
				touchwin(global_df->SoundWin); wrefresh(global_df->SoundWin);
				global_df->WinChange = TRUE;
				delay_mach(1);
			} else {
				max = 0L;
				row = global_df->top_win;
				for (i=0; i < global_df->EdWinSize && row!=global_df->tail_text; i++, row=ToNextRow(row,FALSE)) {
					if (row == global_df->cur_line) {
						if (max < global_df->head_row_len)
							max = global_df->head_row_len;
					} else {
						if (max < strlen(row->line))
							max = strlen(row->line);
					}
				}
				if (global_df->ScrollBar != '\001') {
					strcpy(global_df->err_message, "+Finish coding current line!");
				} else {
					max++;
					val = (double)nPos;
					val = (double)max * (val / 100.0000);
					global_df->LeftCol = (long)val;
				}
				global_df->col_win = global_df->LeftCol + 1;
				DisplayTextWindow(NULL, 1);
				PosAndDispl();
			}
			break;	
	}
	if (global_df->SoundWin)
		DrawSoundCursor(1);
	SetScrollControl();
	global_df->col_win = orgCol_win;
	wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
}
/* Scroll End */
