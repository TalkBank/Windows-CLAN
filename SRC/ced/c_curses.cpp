#if defined(_MAC_CODE)
	#include "0global.h"
#endif

#ifdef _MAC_CODE
	#include <Script.h>
	#include <Quickdraw.h>

	long CaretTime;
#endif
#include "ced.h"
#include "cu.h"
#include "my_ctype.h"
#ifdef _WIN32
// NO QT	#include <TextUtils.h>
	#include "imm.h"
#endif

#ifndef NEW
#define NEW(type) ((type *)malloc((size_t) sizeof(type)))
#endif

#ifdef _WIN32
static char isDrawCursor = 1;
static long CaretHeight = 0L;
#endif

char showColorKeywords = TRUE;
NewFontInfo cedDFnt;
Boolean IsColorMonitor;

int SpecialWindowOffset(short id) {
	return(0);
}

WINDOW *newwin(int num_rows, int num_cols, int LT_row, int LT_col, int tOff) {
	WINDOW *w;
	register int i;
	register int j;
	short FHeight;
#ifdef _MAC_CODE
	FontInfo fi;
	GrafPtr oldPort;
#endif // _MAC_CODE

	if ((w=NEW(WINDOW)) == NULL)
		return(NULL);
	w->NextWindow = global_df->RootWindow;
#ifdef _MAC_CODE
	short FName;
	short FSize;

	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
	FName = DEFAULT_ID;
	FSize = DEFAULT_SIZE;
	TextFont(FName);
	TextSize(FSize);
	GetFontInfo(&fi);
	FHeight = fi.ascent+fi.descent+fi.leading+FLINESPACE;
	SetPort(oldPort);
	CaretTime = GetCaretTime();
#endif // _MAC_CODE
#ifdef _WIN32
	CSize CW;
	char FName[LF_FACESIZE];
	long FSize;

	strcpy(FName, DEFAULT_FONT);
	FSize = DEFAULT_SIZE;
	CFont* pOldFont = GlobalDC->SelectObject(&m_font);
	CW = GlobalDC->GetTextExtent(cl_T("IjWF115[E|CHAT]"), 15);
	GlobalDC->SelectObject(pOldFont);
	FHeight = CW.cy + FLINESPACE;
#endif /* _WIN32 */
	w->isUTF = 1;
	w->winPixelSize = num_rows * FHeight;
	w->reverse = '\0';
	w->textOffset = tOff;
	if (LT_row == 0) w->LT_row = SpecialWindowOffset(global_df->winID);
	else w->LT_row = (NumOfRowsOfDefaultWindow() - (global_df->total_num_rows-LT_row)) * FHeight;
#ifdef _WIN32
	w->LT_row += 2;
#endif /* _WIN32 */
	num_cols = num_cols * 4;
	w->LT_col = LT_col;
	w->cur_row = 0;
	w->cur_col = 0;
	w->num_rows = num_rows;
	w->num_cols = num_cols;
	w->lineno = (unsigned long *)malloc(sizeof(unsigned long) * num_rows);
	w->RowFInfo = (FONTINFO **)malloc(sizeof(FONTINFO *) * num_rows);
	if (w->RowFInfo == NULL) { free(w->lineno); free(w); return(NULL); }
	w->win_data = (unCH **)malloc(sizeof(unCH *) * num_rows);
	if (w->win_data == NULL) { free(w->lineno); free(w->RowFInfo); free(w); return(NULL); }
	w->win_atts = (AttTYPE **)malloc(sizeof(AttTYPE *) * num_rows);
	if (w->win_atts == NULL) { free(w->lineno); free(w->RowFInfo); free(w->win_data); free(w); return(NULL); }
	for (i=0; i < num_rows; i++) {
		if ((w->win_data[i]=(unCH *)malloc(num_cols*sizeof(unCH))) == NULL) {
			for (j=0; j < i; j++) {
				free(w->RowFInfo[j]); free(w->win_data[j]); free(w->win_atts[j]);
			}
			free(w->lineno); free(w->RowFInfo); free(w->win_data); free(w->win_atts);
			free(w); return(NULL);
		}

		if ((w->win_atts[i]=(AttTYPE *)malloc((num_cols)*sizeof(AttTYPE))) == NULL) {
			for (j=0; j < i; j++) {
				free(w->RowFInfo[j]); free(w->win_data[j]); free(w->win_atts[j]);
			}
			free(w->win_data[i]);
			free(w->lineno); free(w->RowFInfo); free(w->win_data); free(w->win_atts);
			free(w); return(NULL);
		}
		if ((w->RowFInfo[i] = NEW(FONTINFO)) == NULL) {
			for (j=0; j < i; j++) {
				free(w->RowFInfo[j]); free(w->win_data[j]); free(w->win_atts[j]);
			}
			free(w->win_data[i]); free(w->win_atts[i]);
			free(w->lineno); free(w->RowFInfo); free(w->win_data); free(w->win_atts);
			free(w); return(NULL);
		}
#ifdef _MAC_CODE
		w->RowFInfo[i]->FName = FName;
		w->RowFInfo[i]->CharSet = 0;
#endif
#ifdef _WIN32
		strcpy(w->RowFInfo[i]->FName, FName);
		w->RowFInfo[i]->CharSet = DEFAULT_CHARSET;
#endif
		w->RowFInfo[i]->FSize = FSize;
		w->RowFInfo[i]->FHeight = FHeight;
		w->lineno[i] = 0;
		for (j=0; j < num_cols; j++) {
			w->win_data[i][j] = 0;
			w->win_atts[i][j] = '\0';
		}
	}
	global_df->RootWindow = w;
	return(w);
}

short ComputeHeight(WINDOW *w, int st, int en) {
	register short sum = 0;

	if (w == NULL)
		return(0);
	if (st > w->num_rows)
		st = w->num_rows;
	if (en > w->num_rows)
		en = w->num_rows;
	for (; st < en; st++) {
		sum += w->RowFInfo[st]->FHeight;
	}
	return(sum);
}

short TextWidthInPix(unCH *st, long beg, long len, FONTINFO *fnt, int textOffset) {
	if (len == 0)
		return(textOffset);

#ifdef _MAC_CODE
	OSStatus		err;
	unsigned long	iRunLengths[1]; //numberOfRuns = 1
	ATSUTextLayout	iTextLayout;
	ATSUStyle		iStyles[1];
	Rect			oTextImageRect;

	st = st + beg;
	len = len - beg;
	// only 1 style thus only 1 run
	iRunLengths[0] = len;
//	iRunLengths[0] = kATSUToTextEnd;

	// and it's the default style
	ATSUStyle tempS;
	if ((err=ATSUCreateStyle(&tempS)) != noErr) {
		return(0);
	}

	ATSUFontID theFontID;

	err = ATSUFONDtoFontID(fnt->FName, 0, &theFontID);
	if (err == noErr) {
		unsigned long			theSize = sizeof(theFontID);
		ATSUAttributeValuePtr	thePtr = &theFontID;
		ATSUAttributeTag 		iTag = kATSUFontTag;
		err = ATSUSetAttributes(tempS, 1, &iTag, &theSize, &thePtr );

		Fixed atsuSize = Long2Fix(fnt->FSize);
		theSize = sizeof(atsuSize);
		thePtr = &atsuSize;
		iTag = kATSUSizeTag;
		err = ATSUSetAttributes(tempS, 1, &iTag, &theSize, &thePtr );
	}

	iStyles[0] = tempS;

	if ((err=ATSUCreateTextLayoutWithTextPtr(
					(ConstUniCharArrayPtr)st,
					0,// kATSUFromTextBeginning,
					len,// kATSUToTextEnd,
					len,
					1, //numberOfRuns,
					iRunLengths,
					iStyles,
					&iTextLayout)) != noErr) {
		return(0);
	}

	ATSUTextMeasurement oTextBefore, oTextAfter, oAscent, oDescent;

	err = ATSUMeasureText (
				iTextLayout, 
				0,// kATSUFromTextBeginning,
				len,// kATSUToTextEnd,
				&oTextBefore, 
				&oTextAfter, 
				&oAscent, 
				&oDescent
				);
	oTextImageRect.right = (short)Fix2Long(oTextAfter);
	oTextImageRect.left  = (short)Fix2Long(oTextBefore);
	err = ATSUDisposeTextLayout(iTextLayout);

	err = ATSUDisposeStyle(tempS);
	return((oTextImageRect.right-oTextImageRect.left) + textOffset);
#endif // _MAC_CODE

#ifdef _WIN32
	LOGFONT lfFont;
	CFont l_font;
	CSize cCharsWidth;
	SIZE  CharsWidth;
	short res;

	SetLogfont(&lfFont, fnt, NULL);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = GlobalDC->SelectObject(&l_font);
	if (GetTextExtentPointW(GlobalDC->m_hDC, st+beg, len-beg, &CharsWidth) != 0)
		res = (short)CharsWidth.cx + textOffset;
	else
		res = 0;
	GlobalDC->SelectObject(pOldFont);
	l_font.DeleteObject();
	return(res);
#endif // _WIN32
}

short FontTxtWidth(WINDOW *w, int row, unCH *s, int st, int en) {
	if (en < 0)
		en = 0;
	if (st < 0)
		st = 0;

	if (s == NULL) {
		if (row >= w->num_rows) row = w->num_rows - 1;
		if (en  >= w->num_cols) en  = w->num_cols - 1;
		s = w->win_data[row];
	}
#ifdef _MAC_CODE
	TextFont(w->RowFInfo[row]->FName);
	TextSize(w->RowFInfo[row]->FSize);
#endif
	return(TextWidthInPix(s, st, en, w->RowFInfo[row], w->textOffset));	
}

void ResetSelectFlag(char c) {
	global_df->TSelectFlag = c;
	global_df->SSelectFlag = c;
}

void blinkCursorLine(void) {
	long len, sc, ec;

	if (global_df == NULL)
		return;
	if (global_df->w1 == NULL)	
		return;
#ifdef _WIN32
	if (global_df->row_win2 == 0L && global_df->col_win2 == -2) {
		if (global_df->row_win < 0L || global_df->row_win >= (long)global_df->EdWinSize) {
			do_warning("Cursor is outside the window.", 0);
		} else {
			SaveUndoState(FALSE);
			DrawCursor(0);
			if (global_df->row_txt == global_df->cur_line)
				global_df->col_txt=global_df->head_row->next_char;
			global_df->col_win = 0L;
			global_df->col_chr = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
			global_df->row_win2 = 1L;
			global_df->LeaveHighliteOn = TRUE;
		}
	} else {
		if (global_df->row_win2 == 0L && global_df->col_win2 != -2) {
			if (global_df->col_win > global_df->col_win2) {
				sc = global_df->col_chr2;
				ec = global_df->col_chr;
			} else {
				sc = global_df->col_chr;
				ec = global_df->col_chr2;
			}
			if (ec - sc > 25)
				do_warning("Text is already selected. It maybe outside the window. Or select < 25 characters", 0);
			else {
				ChangeCurLineAlways(0);
				templine[0] = EOS;
				for (; sc < ec; sc++) {
					len = strlen(templine);
					strcat(templine+len, "| ");
					len +=2;
					templine[len++] = global_df->row_txt->line[sc];
					uS.sprintf(templine+len,cl_T("=0x%x"), global_df->row_txt->line[sc]);
				}
				strcat(templine, "|");
				u_strcpy(templineC, templine, UTTLINELEN);
				do_warning(templineC, 0);
			}
		} else
			do_warning("Text is already selected. It maybe outside the window.", 0);
	}
#endif /* _WIN32 */
#ifdef _MAC_CODE
	if (global_df->row_win2 == 0L && global_df->col_win2 == -2) {
		if (global_df->row_win < 0L || global_df->row_win >= (long)global_df->EdWinSize) {
			do_warning("Cursor is outside the window.", 0);
		} else {
			SaveUndoState(FALSE);
			DrawCursor(0);
			if (global_df->row_txt == global_df->cur_line)
				global_df->col_txt=global_df->head_row->next_char;
			global_df->col_win = 0L;
			global_df->col_chr = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
			global_df->row_win2 = 1L;
			global_df->LeaveHighliteOn = TRUE;
		}
	} else {
		if (global_df->row_win2 == 0L && global_df->col_win2 != -2) {
			if (global_df->col_win > global_df->col_win2) {
				sc = global_df->col_chr2;
				ec = global_df->col_chr;
			} else {
				sc = global_df->col_chr;
				ec = global_df->col_chr2;
			}
			if (ec - sc > 25)
				do_warning("Text is already selected. It maybe outside the window. Or select < 25 characters", 0);
			else {
				ChangeCurLineAlways(0);
				templine[0] = EOS;
				for (; sc < ec; sc++) {
					len = strlen(templine);
					strcat(templine+len, "| ");
					len +=2;
					templine[len++] = global_df->row_txt->line[sc];
					uS.sprintf(templine+len,cl_T("=0x%x"), global_df->row_txt->line[sc]);
				}
				strcat(templine, "|");
				u_strcpy(templineC, templine, UTTLINELEN);
				do_warning(templineC, 0);
			}
		} else
			do_warning("Text is already selected. It maybe outside the window.", 0);
	}
#endif // _MAC_CODE
}

void DrawFakeHilight(char TurnOn) {
	WINDOW *RW;
	char kill;

	if (global_df == NULL)
		return;
	if (global_df->RdW == NULL)	
		return;
	RW = global_df->RdW;
	if (TurnOn == -1) {
		TurnOn = 0;
		kill = true;
	} else
		kill = false;
#ifdef _WIN32
	RECT winRect;
	CWnd* win;

	if (GlobalDC == NULL)
		return;
	win = GlobalDC->GetWindow();
	if (win == NULL)
		return;
	win->GetClientRect(&winRect);
	if ((global_df->fake_row_win2 || global_df->fake_col_win2 != -2) && global_df->w1 == RW) {
		long sc, ec, sr, er;
		RECT theRect;

		if (global_df->fake_row_win2 < 0) {
			sr = global_df->fake_row_win + global_df->fake_row_win2;
			er = global_df->fake_row_win;
		} else if (global_df->fake_row_win2 >= 0) {
			sr = global_df->fake_row_win;
			er = global_df->fake_row_win + global_df->fake_row_win2;
		}
		if (sr == er) {
			if (global_df->fake_col_win2 < global_df->fake_col_win) {
				sc = global_df->fake_col_win2-global_df->LeftCol;
				ec = global_df->fake_col_win-global_df->LeftCol;
			} else {
				sc = global_df->fake_col_win-global_df->LeftCol;
				ec = global_df->fake_col_win2-global_df->LeftCol;
			}
			if (sc < 0)
				sc = 0;
			if (ec < 0)
				ec = 0;
			if (sr >= (long)ActualWinSize(RW) || sr < 0L)
				goto fake_DoneCur;
			if (sc < 0) theRect.left = LEFTMARGIN + RW->textOffset;
			else theRect.left = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, sc);
			theRect.right = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, ec);
			theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr));
			theRect.bottom = theRect.top + ComputeHeight(RW, sr, sr+1);
			if ((TurnOn && global_df->fake_TSelectFlag == 0) || (!TurnOn && global_df->fake_TSelectFlag == 1) || 
					 TurnOn == 2) 
				GlobalDC->InvertRect(&theRect);
		} else {
			if (global_df->fake_row_win2 < 0) {
				sc = global_df->fake_col_win2-global_df->LeftCol;
				ec = global_df->fake_col_win-global_df->LeftCol;
			} else {
				sc = global_df->fake_col_win-global_df->LeftCol;
				ec = global_df->fake_col_win2-global_df->LeftCol;
			}
			if (sc < 0)
				sc = 0;
			if (ec < 0)
				ec = 0;
			if (sr >= (long)ActualWinSize(RW))
				goto fake_DoneCur;
			if (sr < 0L) {
				if (er >= 0L) {sr = 0L; sc = 0L; }
				else goto fake_DoneCur;
			}
			if (er >= ActualWinSize(RW)) { er = ActualWinSize(RW) - 1; ec = -1; }
			if (sr != er || ec == -1) {
				if (sc < 0) theRect.left = LEFTMARGIN + RW->textOffset;
				else theRect.left = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, sc);
				theRect.right = winRect.right;
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr));
				theRect.bottom = theRect.top + ComputeHeight(RW, sr, sr+1);
				if ((TurnOn && global_df->fake_TSelectFlag == 0) || (!TurnOn && global_df->fake_TSelectFlag == 1) || 
					 TurnOn == 2)
					GlobalDC->InvertRect(&theRect);
			}

			if (er-sr > 1) {
				theRect.left  = LEFTMARGIN + RW->textOffset;
				theRect.right = winRect.right;
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr+1));
				theRect.bottom = theRect.top + ComputeHeight(RW, sr+1, er);
				if ((TurnOn && global_df->fake_TSelectFlag == 0) || (!TurnOn && global_df->fake_TSelectFlag == 1) || 
					 TurnOn == 2)
					GlobalDC->InvertRect(&theRect);
			}
			if (sr != er || ec != -1) {
				theRect.left  = LEFTMARGIN + RW->textOffset;
				if (ec == -1)
					theRect.right = winRect.right;
				else
					theRect.right = LEFTMARGIN + FontTxtWidth(RW, er, NULL, 0, ec);
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, er));
				theRect.bottom = theRect.top + ComputeHeight(RW, er, er+1);
				if ((TurnOn && global_df->fake_TSelectFlag == 0) || (!TurnOn && global_df->fake_TSelectFlag == 1) || 
					 TurnOn == 2)
					GlobalDC->InvertRect(&theRect);
			}
		}

fake_DoneCur:
		if (TurnOn)
			global_df->fake_TSelectFlag = 1;
		else
			global_df->fake_TSelectFlag = 0;
	}
#endif /* _WIN32 */
#ifdef _MAC_CODE
	GrafPtr  oldPort;

	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
	if ((global_df->fake_row_win2 || global_df->fake_col_win2 != -2) && global_df->w1 == RW) {
		long sc, ec, sr, er;
		Rect theRect;

		if (global_df->fake_row_win2 < 0) {
			sr = global_df->fake_row_win + global_df->fake_row_win2;
			er = global_df->fake_row_win;
		} else if (global_df->fake_row_win2 >= 0) {
			sr = global_df->fake_row_win;
			er = global_df->fake_row_win + global_df->fake_row_win2;
		} else {
			sr = global_df->fake_row_win;
			er = global_df->fake_row_win + global_df->fake_row_win2;
		}
		PenMode(((IsColorMonitor) ? 50 : notPatCopy));
		if (sr == er) {
			if (global_df->fake_col_win2 < global_df->fake_col_win) {
				sc = global_df->fake_col_win2-global_df->LeftCol;
				ec = global_df->fake_col_win-global_df->LeftCol;
			} else {
				sc = global_df->fake_col_win-global_df->LeftCol;
				ec = global_df->fake_col_win2-global_df->LeftCol;
			}
			if (sr >= (long)ActualWinSize(RW) || sr < 0L) goto fake_DoneCur;
			if (sc < 0) theRect.left = LEFTMARGIN + RW->textOffset;
			else theRect.left = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, sc);
			theRect.right = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, ec);
			theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr));
			theRect.bottom = theRect.top + ComputeHeight(RW, sr, sr+1);
			if ((TurnOn && global_df->fake_TSelectFlag == 0) || (!TurnOn && global_df->fake_TSelectFlag == 1) || 
					 TurnOn == 2) 
				PaintRect(&theRect);
		} else {
			Rect trect;
#if (TARGET_API_MAC_CARBON == 1)
			GetWindowPortBounds(global_df->wind, &trect);
#else
			trect = global_df->wind->portRect;
#endif
			if (global_df->fake_row_win2 < 0) {
				sc = global_df->fake_col_win2-global_df->LeftCol;
				ec = global_df->fake_col_win-global_df->LeftCol;
			} else {
				sc = global_df->fake_col_win-global_df->LeftCol;
				ec = global_df->fake_col_win2-global_df->LeftCol;
			}
			if (sr >= (long)ActualWinSize(RW)) goto fake_DoneCur;
			if (sr < 0L) {
				if (er >= 0L) {sr = 0L; sc = 0L; }
				else goto fake_DoneCur;
			}
			if (er >= ActualWinSize(RW)) { er = ActualWinSize(RW) - 1; ec = -1; }
			if (sr != er || ec == -1) {
				if (sc < 0) theRect.left = LEFTMARGIN + RW->textOffset;
				else theRect.left = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, sc);
				theRect.right = trect.right - SCROLL_BAR_SIZE;
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr));
				theRect.bottom = theRect.top + ComputeHeight(RW, sr, sr+1);
				if ((TurnOn && global_df->fake_TSelectFlag == 0) || (!TurnOn && global_df->fake_TSelectFlag == 1) || 
					 TurnOn == 2)
					PaintRect(&theRect);
			}

			if (er-sr > 1) {
				theRect.left  = LEFTMARGIN + RW->textOffset;
				theRect.right = trect.right - SCROLL_BAR_SIZE;
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr+1));
				theRect.bottom = theRect.top + ComputeHeight(RW, sr+1, er);
				if ((TurnOn && global_df->fake_TSelectFlag == 0) || (!TurnOn && global_df->fake_TSelectFlag == 1) || 
					 TurnOn == 2)
					PaintRect(&theRect);
			}
			if (sr != er || ec != -1) {
				theRect.left  = LEFTMARGIN + RW->textOffset;
				if (ec == -1)
					theRect.right = trect.right - SCROLL_BAR_SIZE;
				else
					theRect.right = LEFTMARGIN + FontTxtWidth(RW, er, NULL, 0, ec);
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, er));
				theRect.bottom = theRect.top + ComputeHeight(RW, er, er+1);
				if ((TurnOn && global_df->fake_TSelectFlag == 0) || (!TurnOn && global_df->fake_TSelectFlag == 1) || 
					 TurnOn == 2)
					PaintRect(&theRect);
			}
		}
fake_DoneCur:
		if (TurnOn)
			global_df->fake_TSelectFlag = 1;
		else
			global_df->fake_TSelectFlag = 0;
	}
	SetPort(oldPort);
#endif // _MAC_CODE
	if (kill) {
		global_df->fake_row_win2 = 0;
		global_df->fake_col_win2 = -2;
	}
}				

void DrawCursor(char TurnOn) {
	WINDOW *RW;

	if (global_df == NULL)
		return;
	if (global_df->RdW == NULL)	
		return;
	RW = global_df->RdW;
#ifdef _MAC_CODE
	register int print_row;
	register int ccol;
	register int RS;
	long 		 tik; // lll
	GrafPtr		 oldPort;
	Rect		 trect;
#if (TARGET_API_MAC_CARBON == 1)
	GetWindowPortBounds(global_df->wind, &trect);
#else
	trect = global_df->wind->portRect;
#endif
	tik = TickCount(); // lll
	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);

	if (global_df->fake_TSelectFlag && TurnOn)
		DrawFakeHilight(0);

	if ((global_df->row_win2 || global_df->col_win2 != -2) && global_df->w1 == RW) {
		long sc, ec, sr, er;
		Rect theRect;

		if (global_df->row_win2 < 0) {
			sr = global_df->row_win + global_df->row_win2;
			er = global_df->row_win;
		} else if (global_df->row_win2 >= 0) {
			sr = global_df->row_win;
			er = global_df->row_win + global_df->row_win2;
		} else {
			sr = global_df->row_win;
			er = global_df->row_win + global_df->row_win2;
		}
		PenMode(((IsColorMonitor) ? 50 : notPatCopy));
		if (sr == er) {
			if (global_df->col_win2 < global_df->col_win) {
				sc = global_df->col_win2-global_df->LeftCol;
				ec = global_df->col_win-global_df->LeftCol;
			} else {
				sc = global_df->col_win-global_df->LeftCol;
				ec = global_df->col_win2-global_df->LeftCol;
			}
			if (sr >= (long)ActualWinSize(RW) || sr < 0L)
				goto DoneCur;
			if (sc < 0)
				theRect.left = LEFTMARGIN + RW->textOffset;
			else
				theRect.left = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, sc);
			theRect.right = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, ec);
			theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr));
			theRect.bottom = theRect.top + ComputeHeight(RW, sr, sr+1);
			if ((TurnOn && global_df->TSelectFlag == 0) || (!TurnOn && global_df->TSelectFlag == 1) ||
				TurnOn == 2)
				PaintRect(&theRect);
		} else {
			if (global_df->row_win2 < 0) {
				sc = global_df->col_win2-global_df->LeftCol;
				ec = global_df->col_win-global_df->LeftCol;
			} else {
				sc = global_df->col_win-global_df->LeftCol;
				ec = global_df->col_win2-global_df->LeftCol;
			}
			if (sr >= (long)ActualWinSize(RW))
				goto DoneCur;
			if (sr < 0L) {
				if (er >= 0L) {
					sr = 0L;
					sc = 0L;
				} else
					goto DoneCur;
			}
			if (er >= ActualWinSize(RW)) { er = ActualWinSize(RW) - 1; ec = -1; }
			if (sr != er || ec == -1) {
				if (sc < 0) theRect.left = LEFTMARGIN + RW->textOffset;
				else theRect.left = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, sc);
				theRect.right = trect.right - SCROLL_BAR_SIZE;
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr));
				theRect.bottom = theRect.top + ComputeHeight(RW, sr, sr+1);
				if ((TurnOn && global_df->TSelectFlag == 0) || (!TurnOn && global_df->TSelectFlag == 1) ||
					TurnOn == 2)
					PaintRect(&theRect);
			}

			if (er-sr > 1) {
				theRect.left  = LEFTMARGIN + RW->textOffset;
				theRect.right = trect.right - SCROLL_BAR_SIZE;
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr+1));
				theRect.bottom = theRect.top + ComputeHeight(RW, sr+1, er);
				if ((TurnOn && global_df->TSelectFlag == 0) || (!TurnOn && global_df->TSelectFlag == 1) ||
					TurnOn == 2)
					PaintRect(&theRect);
			}
			if (sr != er || ec != -1) {
				theRect.left  = LEFTMARGIN + RW->textOffset;
				if (ec == -1)
					theRect.right = trect.right - SCROLL_BAR_SIZE;
				else
					theRect.right = LEFTMARGIN + FontTxtWidth(RW, er, NULL, 0, ec);
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, er));
				theRect.bottom = theRect.top + ComputeHeight(RW, er, er+1);
				if ((TurnOn && global_df->TSelectFlag == 0) || (!TurnOn && global_df->TSelectFlag == 1) ||
					TurnOn == 2)
					PaintRect(&theRect);
			}
		}
DoneCur:
		if (TurnOn)
			global_df->TSelectFlag = 1;
		else
			global_df->TSelectFlag = 0;
	} else if (TurnOn && tik-global_df->Ltik < CaretTime) { // lll
	} else if (RW->cur_row == -1 || RW->cur_col < 0 || RW->cur_col >= RW->num_cols) {
	} else if (FontTxtWidth(RW,RW->cur_row,NULL,0,RW->cur_col) <= trect.right-SCROLL_BAR_SIZE) {
		RS = ((ComputeHeight(RW, RW->cur_row, RW->cur_row+1) * 5) / 100);
		print_row = (RW->LT_row + ComputeHeight(RW, 0, RW->cur_row+1)) + RS - 1;
		TextFont(RW->RowFInfo[RW->cur_row]->FName);
		TextSize(RW->RowFInfo[RW->cur_row]->FSize);
		ccol = LEFTMARGIN+FontTxtWidth(RW,RW->cur_row,NULL,0,RW->cur_col) - 1;

		if (TurnOn) {
//lll			if (global_df->DrawCur) {
				PenMode(patXor);
				MoveTo(ccol, print_row);
				LineTo(ccol, print_row-(ComputeHeight(RW,RW->cur_row,RW->cur_row+1)-RS)+1);
				global_df->DrawCur = !global_df->DrawCur; // lll
				global_df->Ltik = tik; // lll
//lll				global_df->DrawCur = 0;
//lll			}
		} else {
			if (!global_df->DrawCur) {
				PenMode(patXor);
				MoveTo(ccol, print_row);
				LineTo(ccol, print_row-(ComputeHeight(RW,RW->cur_row,RW->cur_row+1)-RS)+1);
				global_df->DrawCur = 1;
			}
			global_df->Ltik = 0L; // lll
			global_df->TSelectFlag = 0;
		}
	}
	SetPort(oldPort);
#endif // _MAC_CODE
#ifdef _WIN32
	RECT winRect;
	CWnd* win;

	if (GlobalDC == NULL)
		return;
	win = GlobalDC->GetWindow();
	if (win == NULL)
		return;

	if (TurnOn == 3) {
		CaretHeight = 0L;
		isDrawCursor = TRUE;
		global_df->DrawCur = TRUE;
	}
	if (TurnOn == 4) {
		if (!isDrawCursor) {
			HideCaret(win->m_hWnd);
		}
	}
	if (!global_df->DrawCur)
		return;

	if (global_df->fake_TSelectFlag && TurnOn)
		DrawFakeHilight(0);

	win->GetClientRect(&winRect);
	if ((global_df->row_win2 || global_df->col_win2 != -2) && global_df->w1 == RW) {
		if (!isDrawCursor) {
			HideCaret(win->m_hWnd);
			isDrawCursor = TRUE;
		}
		long sc, ec, sr, er;
		RECT theRect;

		if (global_df->row_win2 < 0) {
			sr = global_df->row_win + global_df->row_win2;
			er = global_df->row_win;
		} else if (global_df->row_win2 >= 0) {
			sr = global_df->row_win;
			er = global_df->row_win + global_df->row_win2;
		}
		if (sr == er) {
			if (global_df->col_win2 < global_df->col_win) {
				sc = global_df->col_win2-global_df->LeftCol;
				ec = global_df->col_win-global_df->LeftCol;
			} else {
				sc = global_df->col_win-global_df->LeftCol;
				ec = global_df->col_win2-global_df->LeftCol;
			}
			if (sr >= (long)ActualWinSize(RW) || sr < 0L)
				goto DoneCur;
			if (sc < 0) theRect.left = LEFTMARGIN + RW->textOffset;
			else theRect.left = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, sc);
			theRect.right = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, ec);
			theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr));
			theRect.bottom = theRect.top + ComputeHeight(RW, sr, sr+1);
			if ((TurnOn && global_df->TSelectFlag == 0) || (!TurnOn && global_df->TSelectFlag == 1) || 
					 TurnOn == 2) 
				GlobalDC->InvertRect(&theRect);
		} else {
			if (global_df->row_win2 < 0) {
				sc = global_df->col_win2-global_df->LeftCol;
				ec = global_df->col_win-global_df->LeftCol;
			} else {
				sc = global_df->col_win-global_df->LeftCol;
				ec = global_df->col_win2-global_df->LeftCol;
			}
			if (sr >= (long)ActualWinSize(RW))
				goto DoneCur;
			if (sr < 0L) {
				if (er >= 0L) {sr = 0L; sc = 0L; }
				else goto DoneCur;
			}
			if (er >= ActualWinSize(RW)) { er = ActualWinSize(RW) - 1; ec = -1; }
			if (sr != er || ec == -1) {
				if (sc < 0) theRect.left = LEFTMARGIN + RW->textOffset;
				else theRect.left = LEFTMARGIN + FontTxtWidth(RW, sr, NULL, 0, sc);
				theRect.right = winRect.right;
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr));
				theRect.bottom = theRect.top + ComputeHeight(RW, sr, sr+1);
				if ((TurnOn && global_df->TSelectFlag == 0) || (!TurnOn && global_df->TSelectFlag == 1) || 
					 TurnOn == 2)
					GlobalDC->InvertRect(&theRect);
			}

			if (er-sr > 1) {
				theRect.left  = LEFTMARGIN + RW->textOffset;
				theRect.right = winRect.right;
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, sr+1));
				theRect.bottom = theRect.top + ComputeHeight(RW, sr+1, er);
				if ((TurnOn && global_df->TSelectFlag == 0) || (!TurnOn && global_df->TSelectFlag == 1) || 
					 TurnOn == 2)
					GlobalDC->InvertRect(&theRect);
			}
			if (sr != er || ec != -1) {
				theRect.left  = LEFTMARGIN + RW->textOffset;
				if (ec == -1)
					theRect.right = winRect.right;
				else
					theRect.right = LEFTMARGIN + FontTxtWidth(RW, er, NULL, 0, ec);
				theRect.top = (RW->LT_row + ComputeHeight(RW, 0, er));
				theRect.bottom = theRect.top + ComputeHeight(RW, er, er+1);
				if ((TurnOn && global_df->TSelectFlag == 0) || (!TurnOn && global_df->TSelectFlag == 1) || 
					 TurnOn == 2)
					GlobalDC->InvertRect(&theRect);
			}
		}

DoneCur:
		if (TurnOn) global_df->TSelectFlag = 1;
		else global_df->TSelectFlag = 0;
	} else if (RW->cur_row == -1 || RW->cur_col < 0 || 
			   RW->cur_col >= RW->num_cols) ;
	else if (FontTxtWidth(RW,RW->cur_row,NULL,0,RW->cur_col) <= winRect.right) {
		if (TurnOn) {
			if (isDrawCursor || TurnOn == 3 || TurnOn == 4) {
				CBitmap pBitmap;
				int x, y;
				COMPOSITIONFORM cpf; //IME window position set
				HIMC hIMC;			 //IME window position set

				if (CaretHeight != RW->RowFInfo[RW->cur_row]->FHeight) {
					if (TurnOn != 3 && TurnOn != 4)
						DestroyCaret();
					CaretHeight = RW->RowFInfo[RW->cur_row]->FHeight;
					CreateCaret(win->m_hWnd, NULL, 1, RW->RowFInfo[RW->cur_row]->FHeight);
				}

				x = LEFTMARGIN + FontTxtWidth(RW,RW->cur_row,NULL,0,RW->cur_col) - 1;
				y = RW->LT_row + ComputeHeight(RW,0,RW->cur_row) - FLINESPACE;
				SetCaretPos(x, y);
				ShowCaret(win->m_hWnd);
				isDrawCursor = FALSE;

				// IME window position set Begin
				hIMC = ImmGetContext(win->m_hWnd);
				if (hIMC) {
					cpf.dwStyle = CFS_POINT;
					cpf.ptCurrentPos.x = x;
					cpf.ptCurrentPos.y = y;
					ImmSetCompositionWindow(hIMC,&cpf);
					ImmReleaseContext(win->m_hWnd,hIMC);
				}
				// IME window position set End
			}
		} else {
			if (!isDrawCursor) {
				HideCaret(win->m_hWnd);
				isDrawCursor = TRUE;
			}
			global_df->TSelectFlag = 0;
		}
	}
#endif /* _WIN32 */
}

void DrawSoundCursor(char TurnOn) {
	int  SoundOffset;
	long tik;

	if (global_df == NULL)
		return;
	if (global_df->SoundWin == NULL || (!global_df->ScrollBar && TurnOn != 3 && TurnOn && !PlayingContSound))
		return;
	if (TurnOn == 3)
		TurnOn = 1;
#ifdef _WIN32
	RECT	winRect;
	CPen	pen, *pOldPen;
	CWnd*	win;
	int		print_row;
	int		ccol;

	if (GlobalDC == NULL)
		return;
	win = GlobalDC->GetWindow();
	if (win == NULL)
		return;
	tik = GetTickCount();
	win->GetClientRect(&winRect);
	if (global_df->SnTr.EndF > global_df->SnTr.SoundFileSize) {
		if (global_df->SnTr.BegF == global_df->SnTr.EndF) {
			global_df->SnTr.EndF = 0L;
			global_df->SnTr.BegF = global_df->SnTr.SoundFileSize;
		} else
			global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
	}
	SoundOffset = global_df->SnTr.SNDWccol; // FontTxtWidth(global_df->SoundWin,0,cl_T("+W"),0,2);
	if (global_df->SnTr.WEndF >= global_df->SnTr.SoundFileSize) 
		global_df->SnTr.WEndF  = global_df->SnTr.SoundFileSize;
	if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L) {
		long sc, ec;
		long Beg, End;
		RECT theRect;
		if (global_df->SoundWin && PlayingContSound == TRUE && global_df->SnTr.contPlayBeg != 0L && global_df->SnTr.contPlayEnd != 0L) {
			Beg = global_df->SnTr.contPlayBeg;
			End = global_df->SnTr.contPlayEnd;
		} else {
			Beg = global_df->SnTr.BegF;
			End = global_df->SnTr.EndF;
		}
		sc = Beg;
		ec = End;
		if (sc < global_df->SnTr.WBegF) 
			theRect.left = SoundOffset;
		else 
			theRect.left = SoundOffset + ((Beg - global_df->SnTr.WBegF) / global_df->scale_row / global_df->SnTr.SNDsample);
		if (ec > global_df->SnTr.WEndF)
			theRect.right = winRect.right - SoundOffset;
		else
			theRect.right = SoundOffset + ((End - global_df->SnTr.WBegF) / global_df->scale_row / global_df->SnTr.SNDsample);
		theRect.top = global_df->SoundWin->LT_row + ComputeHeight(global_df->SoundWin, 0, 1);
		theRect.bottom = theRect.top + ComputeHeight(global_df->SoundWin, 1, global_df->SoundWin->num_rows);
		if ((TurnOn && global_df->SSelectFlag==0) || (!TurnOn && global_df->SSelectFlag==1) || TurnOn==2) {
			if (theRect.left < theRect.right)
				GlobalDC->InvertRect(&theRect);
		}
		if (TurnOn) global_df->SSelectFlag = 1;
		else global_df->SSelectFlag = 0;
	} else {
		if (TurnOn && tik-global_df->SLtik < 480) ; 
		else if (global_df->SnTr.BegF >= global_df->SnTr.WBegF && global_df->SnTr.BegF <= global_df->SnTr.WEndF) {
			print_row = global_df->SoundWin->LT_row+ComputeHeight(global_df->SoundWin,0,global_df->SoundWin->num_rows);
			ccol = SoundOffset + ((global_df->SnTr.BegF - global_df->SnTr.WBegF) / global_df->scale_row / global_df->SnTr.SNDsample);
			if (TurnOn) {
				if (global_df->SnTr.SDrawCur) {
					if (!pen.CreatePen(PS_SOLID, 0, RGB(0,0,0)))
						return;
					global_df->SnTr.SSC_TOP = print_row - ComputeHeight(global_df->SoundWin,1,global_df->SoundWin->num_rows) + 1;
					for (global_df->SnTr.SEC_TOP=global_df->SnTr.SSC_TOP; 
											global_df->SnTr.SEC_TOP < print_row;
															global_df->SnTr.SEC_TOP++) {
						if (GlobalDC->GetPixel(ccol, global_df->SnTr.SEC_TOP) == RGB(0,0,0)) {
							global_df->SnTr.SEC_TOP -= 1;
							break;
						}
					}
					if (global_df->SnTr.SEC_TOP != print_row) {
						global_df->SnTr.SSC_MID = global_df->SnTr.SEC_TOP + 1;
						while (GlobalDC->GetPixel(ccol, global_df->SnTr.SSC_MID) == RGB(0,0,0) && global_df->SnTr.SSC_MID < print_row)
							global_df->SnTr.SSC_MID++;

						if (global_df->SnTr.SSC_MID == print_row)
							global_df->SnTr.SSC_MID = 0;
						else {
							for (global_df->SnTr.SEC_MID=global_df->SnTr.SSC_MID; 
													global_df->SnTr.SEC_MID < print_row;
																	global_df->SnTr.SEC_MID++) {
								if (GlobalDC->GetPixel(ccol, global_df->SnTr.SEC_MID) == RGB(0,0,0)) {
									global_df->SnTr.SEC_MID -= 1;
									break;
								}
							}
						}
						if (global_df->SnTr.SEC_MID != print_row) {
							global_df->SnTr.SSC_BOT = global_df->SnTr.SEC_MID + 1;
							while (GlobalDC->GetPixel(ccol, global_df->SnTr.SSC_BOT) && global_df->SnTr.SSC_BOT < print_row)
								global_df->SnTr.SSC_BOT++;

							if (global_df->SnTr.SSC_BOT == print_row)
								global_df->SnTr.SSC_BOT = 0;
							else {
								for (global_df->SnTr.SEC_BOT=global_df->SnTr.SSC_BOT; 
														global_df->SnTr.SEC_BOT < print_row;
																		global_df->SnTr.SEC_BOT++) {
									if (GlobalDC->GetPixel(ccol, global_df->SnTr.SEC_BOT) == RGB(0,0,0)) {
										global_df->SnTr.SEC_BOT -= 1;
										break;
									}
								}
							}
						} else
							global_df->SnTr.SSC_BOT = 0;

					} else {
						global_df->SnTr.SSC_MID = 0;
						global_df->SnTr.SSC_BOT = 0;
					}
				} else {
					if (!pen.CreatePen(PS_SOLID, 0, RGB(255,255,255)))
						return;
				}
				global_df->SnTr.SDrawCur = !global_df->SnTr.SDrawCur;
				global_df->SLtik = tik;
			} else {
				if (!pen.CreatePen(PS_SOLID, 0, RGB(255,255,255)))
					return;
				global_df->SLtik = 0L;
				global_df->SSelectFlag = 0;
				global_df->SnTr.SDrawCur = 1;
			}
			pOldPen = GlobalDC->SelectObject(&pen);

			if (global_df->SnTr.SSC_TOP) {
				GlobalDC->MoveTo(ccol, global_df->SnTr.SSC_TOP);
				GlobalDC->LineTo(ccol, global_df->SnTr.SEC_TOP);
			}
			if (global_df->SnTr.SSC_MID) {
				GlobalDC->MoveTo(ccol, global_df->SnTr.SSC_MID);
				GlobalDC->LineTo(ccol, global_df->SnTr.SEC_MID);
			}
			if (global_df->SnTr.SSC_BOT) {
				GlobalDC->MoveTo(ccol, global_df->SnTr.SSC_BOT);
				GlobalDC->LineTo(ccol, global_df->SnTr.SEC_BOT);
			}
			GlobalDC->SelectObject(pOldPen);
		}
	}
#endif /* _WIN32 */
#ifdef _MAC_CODE
	GrafPtr  oldPort;
	Rect trect;
	GetWindowPortBounds(global_df->wind, &trect);
	tik = TickCount();
	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
	if (global_df->SnTr.EndF > global_df->SnTr.SoundFileSize) {
		if (global_df->SnTr.BegF == global_df->SnTr.EndF) {
			global_df->SnTr.EndF = 0L;
			global_df->SnTr.BegF = global_df->SnTr.SoundFileSize;
		} else
			global_df->SnTr.EndF = global_df->SnTr.SoundFileSize;
	}
	SoundOffset = global_df->SnTr.SNDWccol; // FontTxtWidth(global_df->SoundWin,0,cl_T("\245S"),0,2);
	if (global_df->SnTr.WEndF >= global_df->SnTr.SoundFileSize) 
		global_df->SnTr.WEndF  = global_df->SnTr.SoundFileSize;
	if (global_df->SnTr.BegF != global_df->SnTr.EndF && global_df->SnTr.EndF != 0L) {
		long sc, ec;
		long Beg, End;
		Rect theRect;
		if (global_df->SoundWin && PlayingContSound == TRUE && global_df->SnTr.contPlayBeg != 0L && global_df->SnTr.contPlayEnd != 0L) {
			Beg = global_df->SnTr.contPlayBeg;
			End = global_df->SnTr.contPlayEnd;
		} else {
			Beg = global_df->SnTr.BegF;
			End = global_df->SnTr.EndF;
		}
		sc = Beg;
		ec = End;
		PenMode(((IsColorMonitor) ? 50 : notPatCopy));
		if (sc < global_df->SnTr.WBegF)
			theRect.left = SoundOffset;
		else
			theRect.left = SoundOffset + ((Beg - global_df->SnTr.WBegF) / global_df->scale_row / global_df->SnTr.SNDsample);
		if (ec > global_df->SnTr.WEndF)
			theRect.right = trect.right - SCROLL_BAR_SIZE - global_df->SnTr.SNDWccol + 1;
		else
			theRect.right = SoundOffset + ((End - global_df->SnTr.WBegF) / global_df->scale_row / global_df->SnTr.SNDsample);
		theRect.top = global_df->SoundWin->LT_row + ComputeHeight(global_df->SoundWin, 0, 1);
		theRect.bottom = theRect.top + ComputeHeight(global_df->SoundWin, 1, global_df->SoundWin->num_rows);
		if ((TurnOn && global_df->SSelectFlag==0) || (!TurnOn && global_df->SSelectFlag==1) || TurnOn==2) 
			PaintRect(&theRect);
		if (TurnOn)
			global_df->SSelectFlag = 1;
		else
			global_df->SSelectFlag = 0;
	} else {
		Rect theRect;
		if (TurnOn && tik-global_df->SLtik < CaretTime) ; 
		else if (global_df->SnTr.BegF >= global_df->SnTr.WBegF && global_df->SnTr.BegF <= global_df->SnTr.WEndF) {
			PenMode(hilitetransfermode); // hilitetransfermode
			theRect.left = SoundOffset + ((global_df->SnTr.BegF - global_df->SnTr.WBegF) / global_df->scale_row / global_df->SnTr.SNDsample);
			theRect.right = theRect.left + 2;
			theRect.top = global_df->SoundWin->LT_row; //  + ComputeHeight(global_df->SoundWin, 0, 1);
			theRect.bottom = theRect.top + (ComputeHeight(global_df->SoundWin, 0, 1) * (global_df->SoundWin->num_rows + 1)); // ComputeHeight(global_df->SoundWin, 0, global_df->SoundWin->num_rows) + ;
			if (TurnOn) {
				PaintRect(&theRect);
				global_df->SnTr.SDrawCur = !global_df->SnTr.SDrawCur;
				global_df->SLtik = tik;
			} else {
				if (global_df->SnTr.SDrawCur == 0) {
					PaintRect(&theRect);
					global_df->SnTr.SDrawCur = 1;
				}
				global_df->SLtik = 0L;
				global_df->SSelectFlag = 0;
			}
		}
	}
	SetPort(oldPort);
#endif // _MAC_CODE
}

void GetSoundWinDim(int *row, int *col) {
#ifdef _MAC_CODE
	GrafPtr oldPort;
	Rect trect;
#if (TARGET_API_MAC_CARBON == 1)
	GetWindowPortBounds(global_df->wind, &trect);
#else
	trect = global_df->wind->portRect;
#endif
	if (global_df == NULL)
		return;
	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
	if (doMixedSTWave)
		*row = ComputeHeight(global_df->SoundWin,1,global_df->SoundWin->num_rows);
	else
		*row = ComputeHeight(global_df->SoundWin,1,global_df->SoundWin->num_rows) / global_df->SnTr.SNDchan;
	*col = trect.right - SCROLL_BAR_SIZE - FontTxtWidth(global_df->SoundWin,0,cl_T("WW"),0,2) - global_df->SnTr.SNDWccol - 1;
	SetPort(oldPort);

// mary-wave *col = 1937;

#endif // _MAC_CODE
#ifdef _WIN32
	RECT cRect;

	if (global_df == NULL)
		return;
	GlobalDC->GetWindow()->GetClientRect(&cRect);
	if (doMixedSTWave)
		*row = ComputeHeight(global_df->SoundWin,1,global_df->SoundWin->num_rows);
	else
		*row = ComputeHeight(global_df->SoundWin,1,global_df->SoundWin->num_rows) / global_df->SnTr.SNDchan;
//	*col = cRect.right - SCROLL_BAR_SIZE - FontTxtWidth(global_df->SoundWin,0,cl_T("WW"),0,2) - global_df->SnTr.SNDWccol - 1;
	*col = cRect.right - FontTxtWidth(global_df->SoundWin,0,cl_T("+W+W"),0,4);
#endif /* _WIN32 */
}

void wdrawcontr(WINDOW *w, char on) {
	register int print_row;
	extern char leftChan;
	extern char rightChan;

#ifdef _WIN32
	unCH *s;
	RECT winRect;
	LOGFONT lfFont;
	CFont	l_font;
	COLORREF bkColor, txtColor;
	CWnd* win;

	if (GlobalDC == NULL)
		return;
	win = GlobalDC->GetWindow();
	if (win == NULL)
		return;
	win->GetClientRect(&winRect);
	SetLogfont(&lfFont, w->RowFInfo[0], NULL);
	l_font.CreateFontIndirect(&lfFont);
	CFont* pOldFont = GlobalDC->SelectObject(&l_font);
	print_row = w->LT_row + ComputeHeight(w,0,w->num_rows-1);
	if (on) {
		bkColor = GlobalDC->SetBkColor(RGB(0,0,0));
		txtColor = GlobalDC->SetTextColor(RGB(255,255,255));
	}

	if (on)
		s = cl_T("-H");
	else
		s = cl_T("  ");
	GlobalDC->TextOut(0, print_row, s, 2);

	if (on)
		s = cl_T("+H");
	else
		s = cl_T("  ");
	GlobalDC->TextOut(0, print_row-ComputeHeight(w,0,4), s, 2);

	if (on)
		s = cl_T(" S");
	else
		s = cl_T("  ");
	GlobalDC->TextOut(0, print_row-ComputeHeight(w,0,2), s, 2);

	if (on)	
		s = cl_T("-V");
	else
		s = cl_T("  ");
	GlobalDC->TextOut(winRect.right-FontTxtWidth(w,0,s,0,2), print_row, s, 2);

	if (on)
		s = cl_T("+V");
	else
		s = cl_T("  ");
	GlobalDC->TextOut(winRect.right-FontTxtWidth(w,0,s,0,2), print_row-ComputeHeight(w,0,4), s, 2);

	if (on) {
		if (leftChan)
			s = cl_T("*L");
		else
			s = cl_T(" L");
	} else s = cl_T("  ");
	GlobalDC->TextOut(winRect.right-FontTxtWidth(w,0,s,0,2), print_row-ComputeHeight(w,0,3), s, 2);

	if (on) {
		if (rightChan)
			s = cl_T("*R");
		else
			s = cl_T(" R");
	} else
		s = cl_T("  ");
	GlobalDC->TextOut(winRect.right-FontTxtWidth(w,0,s,0,2), print_row-ComputeHeight(w,0,1), s, 2);

	if (on) {
		GlobalDC->SetTextColor(txtColor);
		GlobalDC->SetBkColor(bkColor);
	}

	GlobalDC->SelectObject(pOldFont);
	l_font.DeleteObject();
#endif /* _WIN32 */
#ifdef _MAC_CODE 
	GrafPtr oldPort;
	Rect trect;
	if (w == NULL)
		return;
	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
	TextFont(w->RowFInfo[0]->FName);
	TextSize(w->RowFInfo[0]->FSize);
#if (TARGET_API_MAC_CARBON == 1)
	GetWindowPortBounds(global_df->wind, &trect);
#else
	trect = global_df->wind->portRect;
#endif
	print_row = w->LT_row + ComputeHeight(w,0,w->num_rows);
	if (on)
		TextMode(notSrcCopy);
	else
		TextMode(srcCopy);

	MoveTo(0, print_row);
	if (on)
		DrawText("-H", 0, 2);
	else
		DrawText("  ", 0, 2);

	MoveTo(0, print_row - ComputeHeight(w,0,4) - 1);
	if (on)
		DrawText("+H", 0, 2);
	else
		DrawText("  ", 0, 2);

	MoveTo(0, print_row - ComputeHeight(w,0,2) - 1);
	if (on)
		DrawText("\245S", 0, 2);
	else
		DrawText("  ", 0, 2);

	MoveTo(trect.right-SCROLL_BAR_SIZE-FontTxtWidth(w,0,cl_T("-V"),0,2), print_row);
	if (on)
		DrawText("-V", 0, 2);
	else
		DrawText("  ", 0, 2);

	MoveTo(trect.right-SCROLL_BAR_SIZE-FontTxtWidth(w,0,cl_T("+V"),0,2),print_row - ComputeHeight(w,0,4) - 1);
	if (on)
		DrawText("+V", 0, 2);
	else
		DrawText("  ", 0, 2);
/*
	MoveTo(trect.right-SCROLL_BAR_SIZE-FontTxtWidth(w,0,cl_T("*L"),0,2),print_row - ComputeHeight(w,0,3) - 1);
	if (on) {
		if (leftChan)
			DrawText("*L", 0, 2);
		else
			DrawText(" L", 0, 2);
	} else
		DrawText("  ", 0, 2);
	MoveTo(trect.right-SCROLL_BAR_SIZE-FontTxtWidth(w,0,cl_T("*R"),0,2),print_row - ComputeHeight(w,0,1) - 1);
	if (on) {
		if (rightChan)
			DrawText("*R", 0, 2);
		else
			DrawText(" R", 0, 2);
	} else
		DrawText("  ", 0, 2);
*/
	TextMode(srcCopy);
	DrawControls(global_df->wind);
	SetPort(oldPort);
#endif // _MAC_CODE
}

void MoveSoundWave(WINDOW *w, int dir, int left_lim, int right_lim) {
#ifdef _WIN32
	RECT	theRect;
	CWnd*	win;

	if (GlobalDC == NULL)
		return;
	win = GlobalDC->GetWindow();
	if (win == NULL)
		return;

	theRect.right = right_lim;
	theRect.left = left_lim;
	theRect.top = w->LT_row + 2;
	theRect.bottom = theRect.top + ComputeHeight(w,0,w->num_rows);

 	win->ScrollWindow(dir, 0, &theRect, NULL);
#endif /* _WIN32 */
#ifdef _MAC_CODE
	GrafPtr oldPort;
	Rect	theRect;
	RgnHandle tempRgn;

	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);

	theRect.right = right_lim;
	theRect.left = left_lim;
	theRect.top = w->LT_row + 2;
	theRect.bottom = theRect.top + ComputeHeight(w,0,w->num_rows);

	tempRgn = NewRgn();
	ScrollRect(&theRect, dir, 0, tempRgn);
	DisposeRgn(tempRgn);
	SetPort(oldPort);
#endif // _MAC_CODE
}

void wdrawdot(WINDOW *w, int hp1, int lp1, int hp2, int lp2, int col) {
	register int t;
#ifdef _WIN32
	CPen pen, *pOldPen;

	if (GlobalDC == NULL)
		return;
	if (w == NULL)
		return;
	t = global_df->SnTr.SNDWprint_row1 - w->LT_row - 2;
	hp1 += global_df->SnTr.SNDWHalfRow;
	if (hp1 > t) hp1 = t;
	lp1 += global_df->SnTr.SNDWHalfRow;
	if (lp1 < 0) lp1 = 0;
	t = col + global_df->SnTr.SNDWccol;
	if (!pen.CreatePen(PS_SOLID, 0, RGB(0,0,0)))
		return;
	pOldPen = GlobalDC->SelectObject(&pen);
	GlobalDC->MoveTo(t, global_df->SnTr.SNDWprint_row1-hp1);
	GlobalDC->LineTo(t, global_df->SnTr.SNDWprint_row1-lp1);
	if (global_df->SnTr.SNDchan == 2 && !doMixedSTWave) {
		t = global_df->SnTr.SNDWprint_row2 - w->LT_row - 2;
		hp2 += global_df->SnTr.SNDWHalfRow;
		if (hp2 > t) hp2 = t;
		lp2 += global_df->SnTr.SNDWHalfRow;
		if (lp2 < 0) lp2 = 0;
		t = col + global_df->SnTr.SNDWccol;
		GlobalDC->MoveTo(t, global_df->SnTr.SNDWprint_row2-hp2);
		GlobalDC->LineTo(t, global_df->SnTr.SNDWprint_row2-lp2);
	}
	GlobalDC->SelectObject(pOldPen);
#endif /* _WIN32 */
#ifdef _MAC_CODE
	GrafPtr oldPort;

	if (w == NULL)
		return;
	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
	t = global_df->SnTr.SNDWprint_row1 - w->LT_row - 2;
	hp1 += global_df->SnTr.SNDWHalfRow;
	if (hp1 > t)
		hp1 = t;
	lp1 += global_df->SnTr.SNDWHalfRow;
	if (lp1 < 0)
		lp1 = 0;
	t = col + global_df->SnTr.SNDWccol;
	PenMode(patCopy);
	MoveTo(t, global_df->SnTr.SNDWprint_row1-hp1);
	LineTo(t, global_df->SnTr.SNDWprint_row1-lp1);
	if (global_df->SnTr.SNDchan == 2 && !doMixedSTWave) {
		t = global_df->SnTr.SNDWprint_row2 - w->LT_row - 2;
		hp2 += global_df->SnTr.SNDWHalfRow;
		if (hp2 > t)
			hp2 = t;
		lp2 += global_df->SnTr.SNDWHalfRow;
		if (lp2 < 0)
			lp2 = 0;
		t = col + global_df->SnTr.SNDWccol;
		PenMode(patCopy);
		MoveTo(t, global_df->SnTr.SNDWprint_row2-hp2);
		LineTo(t, global_df->SnTr.SNDWprint_row2-lp2);
	}
	SetPort(oldPort);
#endif // _MAC_CODE
}

void SetRdWKeyScript(void) {
#ifdef _WIN32
	if (global_df->RdW != NULL) {
		if (global_df->RdW != global_df->w1) {
			cedDFnt.Encod = my_FontToScript(global_df->RdW->RowFInfo[0]->FName, global_df->RdW->RowFInfo[0]->CharSet);
		} else {
			cedDFnt.Encod = my_FontToScript(global_df->row_txt->Font.FName, global_df->row_txt->Font.CharSet);
		}
		cedDFnt.isUTF = global_df->isUTF;
	}
#endif /* _WIN32 */
#ifdef _MAC_CODE
	GrafPtr oldPort;

	if (global_df->RdW != NULL) {
		GetPort(&oldPort);
		SetPortWindowPort(global_df->wind);
		if (global_df->RdW != global_df->w1) {
			TextFont(global_df->RdW->RowFInfo[0]->FName);
			TextSize(global_df->RdW->RowFInfo[0]->FSize);
			cedDFnt.Encod = my_FontToScript(global_df->RdW->RowFInfo[0]->FName, global_df->RdW->RowFInfo[0]->CharSet);
		} else {
			TextFont(global_df->row_txt->Font.FName);
			TextSize(global_df->row_txt->Font.FSize);
			cedDFnt.Encod = my_FontToScript(global_df->row_txt->Font.FName, global_df->row_txt->Font.CharSet);
		}
		cedDFnt.isUTF = global_df->isUTF;
		if (AutoScriptSelect && !global_df->isUTF)
			KeyScript(cedDFnt.Encod);
		SetPort(oldPort);
	}
#endif // _MAC_CODE
}

#ifdef _MAC_CODE
void SetDefFontKeyScript(void) {
	WindowPtr		win;
	WindowProcRec	*rec;

	if ((win=FrontWindow()) != NULL) {
		if ((rec=WindowProcs(win))) {
			cedDFnt.Encod = my_FontToScript(dFnt.fontId, dFnt.CharSet);
			if (AutoScriptSelect && (rec->FileInfo == NULL || !rec->FileInfo->isUTF))
				KeyScript(cedDFnt.Encod);
		}
	}
}

int wgetch(WINDOW *w) {
	register int c;

	c = ced_getc();
	return(c);
}
#endif // _MAC_CODE

void PutCursorInWindow(WINDOW *w) {
	if (w != global_df->w1)
		return;
	global_df->window_rows_offset = 0L;
	global_df->WinChange = FALSE;
	FindMidWindow();
	DisplayTextWindow(NULL, 1);
	global_df->WinChange = TRUE;
}

void delOneWin(WINDOW *w) {
	register int i;

	if (w == NULL)
		return;
	if (w->win_data != NULL) {
		for (i=0; i < w->num_rows; i++) free(w->win_data[i]);
		free(w->win_data);
	}
	if (w->win_atts != NULL) {
		for (i=0; i < w->num_rows; i++) free(w->win_atts[i]);
		free(w->win_atts);
	}
	if (w->lineno != NULL)
		free(w->lineno);
	if (w->RowFInfo != NULL) {
		for (i=0; i < w->num_rows; i++) free(w->RowFInfo[i]);
		free(w->RowFInfo);
	}
	if (w == global_df->RdW)
		global_df->RdW = global_df->w1;
	free(w);
}

void delwin(WINDOW *w) {
	register int i;
	WINDOW *t = global_df->RootWindow;

	if (t == NULL)
		return;
	if (t == w) {
		global_df->RootWindow = t->NextWindow;
	} else {
		while (t->NextWindow != w && t->NextWindow != NULL) t = t->NextWindow;
		if (t->NextWindow != w) return;
		t->NextWindow = w->NextWindow;
	}
	if (w->win_data != NULL) {
		for (i=0; i < w->num_rows; i++) free(w->win_data[i]);
		free(w->win_data);
	}
	if (w->win_atts != NULL) {
		for (i=0; i < w->num_rows; i++) free(w->win_atts[i]);
		free(w->win_atts);
	}
	if (w->lineno != NULL)
		free(w->lineno);
	if (w->RowFInfo != NULL) {
		for (i=0; i < w->num_rows; i++) free(w->RowFInfo[i]);
		free(w->RowFInfo);
	}
	if (w == global_df->RdW)
		global_df->RdW = global_df->w1;
	free(w);
}

void endwin(void) {
	WINDOW *t;
	register int i;

	if (global_df == NULL) return;
	while (global_df->RootWindow != NULL) {
		t = global_df->RootWindow;
		global_df->RootWindow = global_df->RootWindow->NextWindow;
		if (t->win_data != NULL) {
			for (i=0; i < t->num_rows; i++) free(t->win_data[i]);
			free(t->win_data);
		}
		if (t->win_atts != NULL) {
			for (i=0; i < t->num_rows; i++) free(t->win_atts[i]);
			free(t->win_atts);
		}
		if (t->RowFInfo != NULL) {
			for (i=0; i < t->num_rows; i++) free(t->RowFInfo[i]);
			free(t->RowFInfo);
		}
		if (t->lineno != NULL)
			free(t->lineno);
		free(t);
	}
	global_df->w1 = global_df->RdW = NULL;
}

void clear(void) {
#ifdef _MAC_CODE
	Rect theRect;
	GrafPtr oldPort;
	Rect trect;
#if (TARGET_API_MAC_CARBON == 1)
	GetWindowPortBounds(global_df->wind, &trect);
#else
	trect = global_df->wind->portRect;
#endif
	if (global_df == NULL) return;
	if (global_df->wind == NULL) return;
	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
	theRect.bottom = trect.bottom - SCROLL_BAR_SIZE;
	theRect.top = trect.top;
	theRect.left = trect.left;
	theRect.right = trect.right - SCROLL_BAR_SIZE;
	EraseRect(&theRect);
	SetPort(oldPort);
#endif // _MAC_CODE
#ifdef _WIN32
	RECT cRect;
	CWnd* win;

	if (GlobalDC == NULL)
		return;
	win = GlobalDC->GetWindow();
	if (win == NULL)
		return;
	if (global_df == NULL || win == NULL) return;
	if (win) {
		win->GetClientRect(&cRect);
		GlobalDC->FillSolidRect(&cRect, RGB(255,255,255));
	}
#endif /* _WIN32 */
}

void touchwin(WINDOW *w) {
	AttTYPE *sAtts;
	register int row;

	if (w == NULL)
		return;
	for (row=0; row < w->num_rows; row++) {
		sAtts = w->win_atts[row];
		sAtts[0] = set_changed_to_1(sAtts[0]);
	}
}

void sp_touchwin(WINDOW *w) {
	unCH *sData;
	AttTYPE *sAtts;
	register int row;
	register int col;

	for (row=0; row < w->num_rows-1; row++) {
		sAtts = w->win_atts[row];
		sAtts[0] = set_changed_to_1(sAtts[0]);
	}
	if (row < w->num_rows) {
		sData = w->win_data[row];
		for (col=0; col < w->num_cols; col++) {
			if (sData[col] != 0)
				break;
		}
		if (col < w->num_cols) {
			sAtts = w->win_atts[row];
			sAtts[0] = set_changed_to_1(sAtts[0]);
		}
	}
}

void wsetlineno(WINDOW *w, int row, unsigned long lineno) {
	AttTYPE *sAtts;

	if (row < w->num_rows) {
		if (w->lineno[row] != lineno) {
			sAtts = w->win_atts[row];
			sAtts[0] = set_changed_to_1(sAtts[0]);
		}
		w->lineno[row] = lineno;
	}
}

void wmove(WINDOW *w, long row_win, long col_win) {
	unCH *sData;
	AttTYPE *sAtts;
	int col;

	if (row_win < 0 || row_win >= (long)ActualWinSize(w))
		w->cur_row = -1;
	else {
		w->cur_row = row_win;
		sAtts = w->win_atts[w->cur_row];
		sData = w->win_data[w->cur_row];
		for (col=strlen(sData); col < col_win; col++) {
			if (sData[col] == 0) {
				sAtts[0] = set_changed_to_1(sAtts[0]);
				sData[col] = ' ';
			}
		}
	}
	w->cur_col = col_win;
}

void mvwaddstr(WINDOW *w, int row, int col, unCH *s) {
	unCH *sData;
	AttTYPE *sAtts;

	if (row >= w->num_rows)
		return;
	sAtts = w->win_atts[row];
	sData = w->win_data[row];
	for (; *s; col++, s++) {
		if (col >= w->num_cols) {
			col = w->num_cols;
			break;
		}
		if (sData[col] != *s) {
			sAtts[0] = set_changed_to_1(sAtts[0]);
			sData[col] = *s;
		}
		if (w->reverse != is_reversed(sAtts[col])) {
			if (w->reverse == '\002')
				sAtts[col] = set_reversed_to_1(sAtts[col]);
			else
				sAtts[col] = set_reversed_to_0(sAtts[col]);
			sAtts[0] = set_changed_to_1(sAtts[0]);
		}
	}
	wmove(w, row, col);
}

void waddstr(WINDOW *w, unCH *s) {
	register int col;
	register int row;
	unCH *sData;
	AttTYPE *sAtts;

	if (w->cur_row == -1)
		PutCursorInWindow(w);
	row = w->cur_row;
	sAtts = w->win_atts[row];
	sData = w->win_data[row];
	for (col=w->cur_col; *s; col++, s++) {
		if (col >= w->num_cols) {
			col = w->num_cols;
			break;
		}
		if (sData[col] != *s) {
			sAtts[0] = set_changed_to_1(sAtts[0]);
			sData[col] = *s;
		}
		if (w->reverse != is_reversed(sAtts[col])) {
			if (w->reverse == '\002')
				sAtts[col] = set_reversed_to_1(sAtts[col]);
			else
				sAtts[col] = set_reversed_to_0(sAtts[col]);
			sAtts[0] = set_changed_to_1(sAtts[0]);
		}
	}
	wmove(w, row, col);
}

void waddch(WINDOW *w, unCH c, AttTYPE *att, long col_chr, AttTYPE att1) {
	register AttTYPE *sAtts;
	register int row;
	register int col;

	if (w->cur_row == -1) 
		PutCursorInWindow(w);
	row = w->cur_row; col = w->cur_col;
/* 05-06-96 for extra protection
	if (col >= w->num_cols || row >= w->num_rows)
		return;
*/
	sAtts = w->win_atts[row];
	if (w->win_data[row][col] != c) {
		sAtts[0] = set_changed_to_1(sAtts[0]);
		w->win_data[row][col] = c;
	}
	if (att != NULL) {
		if (get_text_att(sAtts[col]) != att[col_chr]) {
			sAtts[col] = set_text_att_to_0(sAtts[col]);
			sAtts[col] = (AttTYPE)(sAtts[col] | att[col_chr]);
			sAtts[0] = set_changed_to_1(sAtts[0]);
		}
	} else if (att1 != 0) {
		if (get_text_att(sAtts[col]) != att1) {
			sAtts[col] = set_text_att_to_0(sAtts[col]);
			sAtts[col] = (AttTYPE)(sAtts[col] | att1);
			sAtts[0] = set_changed_to_1(sAtts[0]);
		}
	} else if (get_text_att(sAtts[col])) {
		sAtts[col] = set_text_att_to_0(sAtts[col]);
		sAtts[0] = set_changed_to_1(sAtts[0]);
	}
	if (w->reverse != is_reversed(sAtts[col])) {
		if (w->reverse == '\002')
			sAtts[col] = set_reversed_to_1(sAtts[col]);
		else
			sAtts[col] = set_reversed_to_0(sAtts[col]);
		sAtts[0] = set_changed_to_1(sAtts[0]);
	}
	wmove(w, row, col + 1);
}

void wstandout(WINDOW *w) {
	w->reverse = '\002';
}

void wstandend(WINDOW *w) {
	w->reverse = '\0';
}

void wUntouchWin(WINDOW *w, int num) {
	AttTYPE *sAtts;
	register int row;

	for (row=0; row < w->num_rows; row++) {
		sAtts = w->win_atts[row];
		if (row != num) {
			sAtts[0] = set_changed_to_0(sAtts[0]);
		}
	}
}

void werase(WINDOW *w) {
	unCH *sData;
	AttTYPE *sAtts;
	register int row;
	register int col;

	for (row=0; row < w->num_rows; row++) {
		sAtts = w->win_atts[row];
		sData = w->win_data[row];
		for (col=0; col < w->num_cols; col++) {
			sAtts[col] = '\0';
			sData[col] = 0;
		}
		sAtts[0] = set_changed_to_1(sAtts[0]);
	}
}

void wclrtoeol(WINDOW *w, char isForce) {
	unCH *sData;
	AttTYPE *sAtts;
	register int col;

	if (w->cur_row == -1)
		PutCursorInWindow(w);
	sAtts = w->win_atts[w->cur_row];
	sData = w->win_data[w->cur_row];
	for (col=w->cur_col; col < w->num_cols; col++) {
		sAtts[col] = '\0';
		if (sData[col] != 0 || isForce) {
			sAtts[0] = set_changed_to_1(sAtts[0]);
		}
		sData[col] = 0;
	}
}

void wclrtobot(WINDOW *w) {
	unCH *sData;
	AttTYPE *sAtts;
	int  row, col;
	short top = w->LT_row;

	if (w->cur_row == -1) PutCursorInWindow(w);
	col = w->cur_col;
	top += ComputeHeight(w,0,w->cur_row);
	for (row=w->cur_row; row < ActualWinSize(w); row++) {
		top += w->RowFInfo[row]->FHeight;
		sAtts = w->win_atts[row];
		sData = w->win_data[row];
		for (; col < w->num_cols; col++) {
			sAtts[col] = '\0';
			sData[col] = 0;
		}
		sAtts[0] = set_changed_to_1(sAtts[0]);
		col = 0;
	}
	if (top < global_df->TextWinSize) {
#ifdef _MAC_CODE
		GrafPtr oldPort;
		Rect theRect;
		Rect trect;

		GetPort(&oldPort);
		SetPortWindowPort(global_df->wind);
#if (TARGET_API_MAC_CARBON == 1)
		GetWindowPortBounds(global_df->wind, &trect);
#else
		trect = global_df->wind->portRect;
#endif
		theRect.top = top;
		theRect.bottom = global_df->TextWinSize + 1;
		theRect.left = trect.left;
		theRect.right = trect.right - SCROLL_BAR_SIZE;
		EraseRect(&theRect);
		SetPort(oldPort);
#endif // _MAC_CODE
#ifdef _WIN32
		CWnd* win;
		RECT cRect;

		if (GlobalDC == NULL)
			return;
		win = GlobalDC->GetWindow();
		if (win) {
			win->GetClientRect(&cRect);
			cRect.top = top;
			cRect.bottom = global_df->TextWinSize + 1;
			GlobalDC->FillSolidRect(&cRect, RGB(255,255,255));
		}
#endif /* _WIN32 */
	}
	for (; row < w->num_rows && top < global_df->TextWinSize; row++) {
		top += w->RowFInfo[row]->FHeight;
		sAtts = w->win_atts[row];
		sData = w->win_data[row];
		for (col=0; col < w->num_cols; col++) {
			sAtts[col] = '\0';
			sData[col] = 0;
		}
		sAtts[0] = set_changed_to_1(sAtts[0]);
	}
}

static int curses_patmat(unCH *s, unCH *pat, unCH spC) {
	register int j, k;
	int n, m, t, l;
	unCH *lf;

	if (s[0] == EOS) {
		return(pat[0] == s[0]);
	}
	l = strlen(s);

	lf = s+l;
	for (j = 0, k = 0; pat[k]; j++, k++) {
		if ((s[j] == '(' || s[j] == ')') && (char)spC != '%') {
			if (s[j] == ')' && (pat[k] == '*' || pat[k] == '%') && pat[k+1] == ')')
				k++;
			else if (pat[k] != s[j]) {
				k--;
				continue;
			}
		} else if (pat[k] == '\\') {
			if (s[j] != pat[++k]) break;
		} else if (pat[k] == '_') {
			if (iswspace(s[j]))
				return(FALSE);
			if (s[j] == EOS)
				return(FALSE);
			if (s[j+1] && pat[k+1])
				continue; // any character
			else {
				if (s[j+1] == EOS && pat[k+1] == '*' && pat[k+2] == EOS)
					return(TRUE);
				else if (pat[k+1] == s[j+1]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			}
		} else if (pat[k] == '*') {		  // wildcard
			k++; t = j;
			if (pat[k] == '\\') k++;
f1:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else {
					if ((pat[k]=='-' || pat[k]=='&') && pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS) {
					} else {
						for (; t < j; t++) {
							if (uS.ismorfchar(s,t, &dFnt, rootmorf, MBF))
								return(FALSE);
						}
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS/* && (char)spC == '*'*/) {
						s = s+l;
						if (lf != s) {
							while (lf != s)
								*lf++ = ' ';
						}
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && (char)spC != '%') {
					if (s[m]== ')' && (pat[n] == '*' || pat[n] == '%') && pat[n+1] == ')')
						n++;
					else
						n--;
				} else if (pat[n] == '*' || pat[n] == '%')
					break;
				else if (pat[n] == '_') {
					if (iswspace(s[m]) || s[m] == EOS) {
						j++;
						goto f1;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n])
						return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f1;
					}
				} else if (s[m] != pat[n]) {
					if (pat[n+1] == '%' && pat[n+2] == '%')
						break;
					j++;
					goto f1;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f1;
			}
		} else if (pat[k] == '%') {		  // wildcard
			m = j;
			if (pat[++k] == '%') {
				k++;
				if (pat[k] == '\\') k++;
				if ((t=j - 1) < 0) t = 0;
			} else {
				if (pat[k] == '\\') k++;
				t = j;
			}
f2:
			while (s[j] && s[j] != pat[k]) j++;
			if (!s[j]) {
				if (!pat[k]) {
					if (RemPercWildCard) {
						lf = uS.sp_cp(s+t,s+j);
						s = s+l;
						if (lf != s) {
							while (lf != s)
								*lf++ = ' ';
						}
					} else
						uS.sp_mod(s+t,s+j);
					return(TRUE);
				} else {
					for (; m < j; m++) {
						if (uS.ismorfchar(s, m, &dFnt, rootmorf, MBF))
							return(FALSE);
					}
					if (pat[k+1]=='%' && pat[k+2]=='%' && pat[k+3]==EOS/* && (char)spC == '*'*/) {
						if (RemPercWildCard) {
							lf = uS.sp_cp(s+t,s+j);
							s = s+l;
							if (lf != s) {
								while (lf != s)
									*lf++ = ' ';
							}
						} else
							uS.sp_mod(s+t,s+j);
						return(TRUE);
					} else
						return(FALSE);
				}
			}
			for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
				if ((s[m]== '(' || s[m]== ')') && (char)spC != '%') {
					if (s[m]== ')' && (pat[n] == '*' || pat[n] == '%') && pat[n+1] == ')')
						n++;
					else
						n--;
				} else if (pat[n] == '*' || pat[n] == '%')
					break;
				else if (pat[n] == '_') {
					if (iswspace(s[m])) {
						j++;
						goto f2;
					}
				} else if (pat[n] == '\\') {
					if (!pat[++n])
						return(FALSE);
					else if (s[m] != pat[n]) {
						j++;
						goto f2;
					}
				} else if (s[m] != pat[n]) {
					j++;
					goto f2;
				}
			}
			if (s[m] && !pat[n]) {
				j++;
				goto f2;
			}
			if (RemPercWildCard) {
				lf = uS.sp_cp(s+t,s+j);
				j = t;
			} else
				uS.sp_mod(s+t,s+j);
		}

		if (s[j] != pat[k]) {
			if (pat[k+1] == '%' && pat[k+2] == '%') {
				if (s[j] == EOS && pat[k+3] == EOS/* && (char)spC == '*'*/) {
					s = s+l;
					if (lf != s) {
						while (lf != s)
							*lf++ = ' ';
					}
					return(TRUE);
				} else
					return(FALSE);
			} else
				return(FALSE);
		}
	}
	if (pat[k] == s[j]) {
		s = s+l;
		if (lf != s) {
			while (lf != s)
				*lf++ = ' ';
		}
		return(TRUE);
	} else
		return(FALSE);
}

void FreeColorWord(COLORWORDLIST *RootColorWord) {
	COLORWORDLIST *t;

	while (RootColorWord != NULL) {
		t = RootColorWord;
		RootColorWord = RootColorWord->nextCW;
		free(t);
	}
}

void FreeColorText(COLORTEXTLIST *lRootColorText) {
	COLORTEXTLIST *t;

	while (lRootColorText != NULL) {
		t = lRootColorText;
		lRootColorText = lRootColorText->nextCT;
		free(t->keyWord);
		free(t->fWord);
		free(t);
	}
}

void createColorWordsList(char *st) {
	int  i;
	char color;
	COLORWORDLIST *t = NULL;

	for (i=0; st[i] != ':' && st[i] != EOS; i++) ;
	if (st[i] == EOS)
		return;
	for (i++; isSpace(st[i]); i++) ;
	st = st + i;
	while ((st=strchr(st, '=')) != NULL) {
		st++;
		if (uS.partcmp(st, "blue", FALSE, TRUE))
			color = blue_color;
		else if (uS.partcmp(st, "red", FALSE, TRUE))
			color = red_color;
		else if (uS.partcmp(st, "green", FALSE, TRUE))
			color = green_color;
		else if (uS.partcmp(st, "magenta", FALSE, TRUE))
			color = magenta_color;
		else
			color = 0;

		if (global_df->RootColorWord == NULL) {
			global_df->RootColorWord = NEW(COLORWORDLIST);
			if (global_df->RootColorWord == NULL)
				return;
			t = global_df->RootColorWord;
		} else {
			t->nextCW = NEW(COLORWORDLIST);
			if (t->nextCW == NULL)
				return;
			t = t->nextCW;
		}
		t->nextCW = NULL;
		t->color = color;
	}
}

COLORTEXTLIST *createColorTextKeywordsList(COLORTEXTLIST *lRootColorText, char *st) {
	int i, wordLen, red, green, blue;
	char cWordFlag;
	unCH *tKeyWord, *tfWord;
	COLORTEXTLIST *t;

	t = lRootColorText;
	if (t != NULL) {
		for (; t->nextCT != NULL; t=t->nextCT) ;
	}
	i = strlen(CKEYWORDHEADER);
	for (; isSpace(st[i]); i++) ;
	while (st[i] != EOS) {
		for (wordLen=0; !isSpace(st[i]) && st[i] != EOS; i++, wordLen++)
			templineC3[wordLen] = st[i];
		templineC3[wordLen] = EOS;
		UTF8ToUnicode((unsigned char *)templineC3, wordLen, templine3, NULL, UTTLINELEN);
		for (; isSpace(st[i]); i++) ;
		if (!isdigit(st[i]))
			return(lRootColorText);
		cWordFlag = atoi(st+i);
		for (; isdigit(st[i]); i++) ;
		for (; isSpace(st[i]); i++) ;
		if (!isdigit(st[i]))
			return(lRootColorText);
		red = atoi(st+i);
		for (; isdigit(st[i]); i++) ;
		for (; isSpace(st[i]); i++) ;
		if (!isdigit(st[i]))
			return(lRootColorText);
		green = atoi(st+i);
		for (; isdigit(st[i]); i++) ;
		for (; isSpace(st[i]); i++) ;
		if (!isdigit(st[i]))
			return(lRootColorText);
		blue = atoi(st+i);
		for (; isdigit(st[i]); i++) ;
		for (; isSpace(st[i]); i++) ;

		tKeyWord = (unCH *)malloc((strlen(templine3)+1)*sizeof(unCH));
		if (tKeyWord == NULL)
			return(lRootColorText);
		strcpy(tKeyWord, templine3);
		tfWord = (unCH *)malloc((strlen(templine3)+1)*sizeof(unCH));
		if (tfWord == NULL) {
			free(tKeyWord);
			return(lRootColorText);
		}
		strcpy(tfWord, templine3);
		if (!is_case_word(cWordFlag)) {
			uS.uppercasestr(tfWord, &dFnt, C_MBF);
		}

		if (lRootColorText == NULL) {
			lRootColorText = NEW(COLORTEXTLIST);
			if (lRootColorText == NULL) {
				free(tKeyWord);
				free(tfWord);
				return(lRootColorText);
			}
			t = lRootColorText;
		} else {
			t->nextCT = NEW(COLORTEXTLIST);
			if (t->nextCT == NULL) {
				free(tKeyWord);
				free(tfWord);
				return(lRootColorText);
			}
			t = t->nextCT;
		}
		t->nextCT = NULL;
		t->keyWord = tKeyWord;
		t->fWord = tfWord;
		t->len = strlen(tfWord);
		t->cWordFlag = cWordFlag;
		t->red = red;
		t->green = green;
		t->blue = blue;
	}
	return(lRootColorText);
}

COLORTEXTLIST *FindColorKeywordsBounds(COLORTEXTLIST *lRootColorText, AttTYPE *sAtts,unCH *sData,int lnoff,int ecol,COLORTEXTLIST *cl) {
	int i, j, col;
	COLORTEXTLIST *t;

	if (!showColorKeywords)
		return(NULL);

	for (t=lRootColorText; t != NULL; t=t->nextCT) {
		for (i=0; i < NUMCOLORPOINTS; i++) {
			t->sCol[i] = -1;
			t->eCol[i] = -1;
		}
		t->index = 0;
	}

	init_punct(0);
	for (t=lRootColorText; t != NULL; t=t->nextCT) {
		if (is_match_word(t->cWordFlag)) {
			col = lnoff;
			while (col < ecol) {
				while ((uS.isskip(sData, col, &dFnt, TRUE)/* || sData[col] == '='*/)  && col < ecol)
					col++;
				for (j=col; !uS.isskip(sData, j, &dFnt, TRUE)/* && sData[j] != '=' */ && j < ecol; j++) {
					templine3[j-col] = sData[j];
				}
				templine3[j-col] = EOS;
				if (!is_case_word(t->cWordFlag))
					uS.uppercasestr(templine3, &dFnt, C_MBF);
				if (is_wild_card(t->cWordFlag))
					i = curses_patmat(templine3, t->fWord, *sData);
				else
					i = !strcmp(t->fWord, templine3);
				if (i) {
					if (is_all_line(t->cWordFlag)) {
						if (t->index < NUMCOLORPOINTS) {
							t->sCol[t->index] = 0;
							t->eCol[t->index] = ecol;
							t->index++;
							for (i=0; i < ecol; i++)
								sAtts[i] = set_color_to_1(sAtts[i]);
							if (cl)
								return(cl);
							else if (global_df != NULL && global_df->ChatMode)
								return(t);
							else
								return(NULL);
						}
					} else {
						if (t->index < NUMCOLORPOINTS) {
							t->sCol[t->index] = col;
							t->eCol[t->index] = j;
							t->index++;
							for (i=col; i < j; i++)
								sAtts[i] = set_color_to_1(sAtts[i]);
						}
					}
				}
				if (j == col)
					col++;
				else
					col = j;
			}
		} else {
			for (col=lnoff; col < ecol;) {
				if (col+t->len > ecol && !is_wild_card(t->cWordFlag))
					break;
				if (is_wild_card(t->cWordFlag)) {
					strncpy(templine3, sData+col, ecol-col);
					templine3[ecol-col] = EOS;
					j = ecol;
				} else {
					strncpy(templine3, sData+col, t->len);
					templine3[t->len] = EOS;
					j = col + t->len;
				}
				if (!is_case_word(t->cWordFlag))
					uS.uppercasestr(templine3, &dFnt, C_MBF);
				if (is_wild_card(t->cWordFlag))
					i = curses_patmat(templine3, t->fWord, *sData);
				else
					i = !strcmp(t->fWord, templine3);
				if (i) {
					if (is_all_line(t->cWordFlag)) {
						if (t->index < NUMCOLORPOINTS) {
							t->sCol[t->index] = 0;
							t->eCol[t->index] = ecol;
							t->index++;
							for (i=0; i < ecol; i++)
								sAtts[i] = set_color_to_1(sAtts[i]);
							if (cl)
								return(cl);
							else if (global_df != NULL && global_df->ChatMode)
								return(t);
							else
								return(NULL);
						}
					} else {
						if (t->index < NUMCOLORPOINTS) {
							t->sCol[t->index] = col;
							t->eCol[t->index] = j;
							t->index++;
							for (i=col; i < j; i++)
								sAtts[i] = set_color_to_1(sAtts[i]);
						}
					}
				}
				if (is_wild_card(t->cWordFlag)) {
					col = ecol;
				} else {
					col++;
				}
			}
		}
	}
	if (cl) {
		if (global_df != NULL && global_df->ChatMode && !isSpeaker(*(sData+lnoff))) {
			if (cl->index < NUMCOLORPOINTS) {
				cl->sCol[cl->index] = 0;
				cl->eCol[cl->index] = ecol;
				cl->index++;
				for (i=0; i < ecol; i++)
					sAtts[i] = set_color_to_1(sAtts[i]);
				return(cl);
			}
		}
	}
	return(NULL);
}

char SetKeywordsColor(COLORTEXTLIST *lRootColorText, int cCol, RGBColor *theColor) {
	int i;
	COLORTEXTLIST *t;
#ifdef _WIN32
	long tc;
#endif

	for (t=lRootColorText; t != NULL; t=t->nextCT) {
		for (i=0; i < t->index; i++) {
			if (cCol >= t->sCol[i] && cCol < t->eCol[i]) {
#ifdef _WIN32
				tc				= t->red;
				tc				= tc * 100 / 0xffff;
				theColor->red   = 0xff * tc / 100;
				tc				= t->green;
				tc				= tc * 100 / 0xffff;
				theColor->green = 0xff * tc / 100;
				tc				= t->blue;
				tc				= tc * 100 / 0xffff;
				theColor->blue  = 0xff * tc / 100;
#else
				theColor->red   = t->red;
				theColor->green = t->green;
				theColor->blue  = t->blue;
#endif
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

#if (TARGET_API_MAC_CARBON == 1)
Boolean DrawUTFontMac(unCH *st, long len, FONTINFO *font, AttTYPE style) {
	OSStatus				err;
	unsigned long			iRunLengths[1]; //numberOfRuns = 1
	ATSUTextLayout			iTextLayout;
	ATSUStyle				iStyles[1];		 //numberOfRuns = 1
	Boolean					StyleValue;
	unsigned long			theSize;
	ATSUAttributeValuePtr	thePtr;
	ATSUAttributeTag 		iTag;
	Point					where;

	// only 1 style thus only 1 run
	iRunLengths[0] = len;
//	iRunLengths[0] = kATSUToTextEnd;

	// and it's the default style
	ATSUStyle tempS;
	if ((err=ATSUCreateStyle(&tempS)) != noErr) {
		return(false);
	}

	ATSUFontID theFontID;

	err = ATSUFONDtoFontID(font->FName, 0, &theFontID);
	if (err == noErr) {
		theSize = sizeof(theFontID);
		thePtr = &theFontID;
		iTag = kATSUFontTag;
		err = ATSUSetAttributes(tempS, 1, &iTag, &theSize, &thePtr );

		Fixed atsuSize = Long2Fix(font->FSize);
		theSize = sizeof(atsuSize);
		thePtr = &atsuSize;
		iTag = kATSUSizeTag;
		err = ATSUSetAttributes(tempS, 1, &iTag, &theSize, &thePtr );
	}
	if (is_underline(style)) {
		StyleValue = true;
		theSize = sizeof(Boolean);
		thePtr = &StyleValue;
		iTag = kATSUQDUnderlineTag;
		err = ATSUSetAttributes(tempS, 1, &iTag, &theSize, &thePtr );
	}
	if (is_italic(style)) {
		StyleValue = true;
		theSize = sizeof(Boolean);
		thePtr = &StyleValue;
		iTag = kATSUQDItalicTag;
		err = ATSUSetAttributes(tempS, 1, &iTag, &theSize, &thePtr );
	}
	if (is_bold(style)) {
		StyleValue = true;
		theSize = sizeof(Boolean);
		thePtr = &StyleValue;
		iTag = kATSUQDBoldfaceTag;
		err = ATSUSetAttributes(tempS, 1, &iTag, &theSize, &thePtr );
	}
	iStyles[0] = tempS;

	if ((err=ATSUCreateTextLayoutWithTextPtr(
					(ConstUniCharArrayPtr)st,
					kATSUFromTextBeginning,
					kATSUToTextEnd,
					len,
					1, //numberOfRuns,
					iRunLengths,
					iStyles,
					&iTextLayout)) != noErr) {
		return(false);
	}

	GetPen(&where); 

	err = ATSUDrawText(iTextLayout, kATSUFromTextBeginning, kATSUToTextEnd, kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc);


	ATSUTextMeasurement oTextBefore, oTextAfter, oAscent, oDescent;
	Rect oTextImageRect;
	err = ATSUMeasureText(iTextLayout,0,len,&oTextBefore,&oTextAfter,&oAscent,&oDescent);
	oTextImageRect.right = (short)Fix2Long(oTextAfter);
	oTextImageRect.left  = (short)Fix2Long(oTextBefore);
	MoveTo(where.h+(oTextImageRect.right-oTextImageRect.left), where.v);


	err = ATSUDisposeTextLayout(iTextLayout);

	err = ATSUDisposeStyle(tempS);
	return(true);
}
#endif // (TARGET_API_MAC_CARBON == 1)


void wrefresh(WINDOW *w) {
	unCH *sData;
	AttTYPE *sAtts;
	register int col;
	register int ecol;
	register int row;
	register int erow;
	register int cCol;
	int     len;
	int		lastViewCol;
	int		tVar;
	char	pr, isShowLineNumbers;
	AttTYPE	RevChar = 0;
	AttTYPE	oldState;
	COLORTEXTLIST *tierColor = NULL;
	RGBColor theColor;
#ifdef _MAC_CODE
	int		print_row;
	int		print_cRow;
	Rect	theRect;
	Rect	trect;
	GrafPtr oldPort;
	short   face = 0;
	short   fsize = -1;
	short	fname = -1;
	RGBColor oldForgColor;

	if (w == NULL)
		return;
	if (global_df->WinChange)
		global_df->RdW = w;
	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
#if (TARGET_API_MAC_CARBON == 1)
	GetWindowPortBounds(global_df->wind, &trect);
#else
	trect = global_df->wind->portRect;
#endif
	GetForeColor(&oldForgColor);

	theRect.left = trect.left;
	theRect.right = trect.right - SCROLL_BAR_SIZE;
	ecol = w->num_cols;
	pr = 0;
	tVar = -1;
	erow = ActualWinSize(w);
	if (w == global_df->w1 && erow >= w->num_rows)
		erow = w->num_rows - 1;
	for (row=0; row < erow; row++) {
		sAtts = w->win_atts[row];
		if (is_changed(sAtts[0])) {
			pr++;
			if (tVar == -1)
				tVar = row;
		}
		if (pr > 1)
			break;
	}
	isShowLineNumbers = (isShowLineNums && w == global_df->w1);
	if (pr) {
		if (w == global_df->w1 && global_df->fake_TSelectFlag)
			DrawFakeHilight(0);
		row = tVar;
		if (pr == 1)
			erow = row + 1;
		if (w == global_df->w1 && erow >= w->num_rows)
			erow = w->num_rows - 1;
		theRect.top = (w->LT_row + ComputeHeight(w,0,row));
		theRect.bottom = theRect.top + ComputeHeight(w,row,erow);
		print_row = w->LT_row + ComputeHeight(w,0,row) - FLINESPACE;

		for (; row < erow; row++) {
			print_row += w->RowFInfo[row]->FHeight;
			if (fname != w->RowFInfo[row]->FName || fsize != w->RowFInfo[row]->FSize) {
				TextFont(w->RowFInfo[row]->FName);
				TextSize(w->RowFInfo[row]->FSize);
				fname = w->RowFInfo[row]->FName;
				fsize = w->RowFInfo[row]->FSize;
			}
			sAtts = w->win_atts[row];

//			if (!is_changed(sAtts[0]))
//				continue;

			if (global_df->w1 == w && row == w->cur_row)
				global_df->DrawCur = 1;
			theRect.top = (w->LT_row + ComputeHeight(w,0,row));
			theRect.bottom = theRect.top + ComputeHeight(w,row,row+1);
			EraseRect(&theRect);
			sAtts[0] = set_changed_to_0(sAtts[0]);

			sData = w->win_data[row];
			lastViewCol = ecol; //0;
			for (col=0; col < ecol; col++) {
				if (sData[col] == 0 && lastViewCol == ecol && (!isShowLineNumbers || col > 0)) {
					lastViewCol = col;
//					break;
				}
				if (is_colored(sAtts[col]))
					sAtts[col] = set_color_to_0(sAtts[col]);
			}
			if (lastViewCol == 0)
				continue;
			if (lastViewCol > ecol)
				lastViewCol = ecol;
			if (global_df->RootColorText != NULL && w == global_df->w1)
				tierColor = FindColorKeywordsBounds(global_df->RootColorText, sAtts, sData, 0, lastViewCol, tierColor);
			cCol = 0;
			oldState = sAtts[0];
			if (w->RowFInfo[row]->FHeight > 12)
				print_cRow = print_row - 3;
			else
				print_cRow = print_row - 1;
			if (isShowLineNumbers) {
				MoveTo(LEFTMARGIN, print_cRow);
				TextFace(0);
				TextMode(srcCopy);
				if (/*LineNumberingType == 0 || */w->lineno[row] != 0L) {
					sprintf(templineC4, "%ld", w->lineno[row]);
					len = 6 - strlen(templineC4);
					if (len > 0) {
						uS.shiftright(templineC4, len);
						for (len--; len >= 0; len--) {
							templineC4[len] = ' ';
						}
					}
				} else
					strcpy(templineC4, "      ");
				if (LineNumberDigitSize != 0) {
					if (strlen(templineC4) < LineNumberDigitSize) {
						strncat(templineC4, "                     ", LineNumberDigitSize);
					}
					templineC4[LineNumberDigitSize] = EOS;
				}
				DrawText(templineC4, 0, strlen(templineC4));
			}
			MoveTo(LEFTMARGIN+w->textOffset, print_cRow);
			for (col=0; col <= lastViewCol; col++) {
				if (oldState != sAtts[col] || col == lastViewCol) {
#if (TARGET_API_MAC_CARBON == 1)
					if (!w->isUTF) {
#endif
						face = 0;
						if (is_underline(sAtts[cCol]) || is_error(sAtts[cCol]))
							face = face | underline;
						if (is_italic(sAtts[cCol]))
							face = face | italic;
						if (is_bold(sAtts[cCol]))
							face = face | bold;
						TextFace(face);
						RevChar = is_reversed(sAtts[cCol]);
						if (RevChar)
							TextMode(notSrcCopy);
#if (TARGET_API_MAC_CARBON == 1)
					}
#endif
					if (is_colored(sAtts[cCol])) {
						if (SetKeywordsColor(global_df->RootColorText, cCol, &theColor))
							RGBForeColor(&theColor);
					}
					if (is_word_color(sAtts[cCol])) {
						char color;

						color = get_color_num(sAtts[cCol]);
						if (color == blue_color) {
							theColor.red = 0;
							theColor.green = 5;
							theColor.blue = 58715;
						} else if (color == red_color) {
							theColor.red = 58122;
							theColor.green = 0;
							theColor.blue = 0;
						} else if (color == green_color) {
							theColor.red = 0;
							theColor.green = 41219;
							theColor.blue = 3;
						} else /* if (color == magenta_color) */ {
							theColor.red = 56935;
							theColor.green = 0;
							theColor.blue = 56931;
						}
						RGBForeColor(&theColor);
					}
					if (is_error(sAtts[cCol])) {
						theColor.red = 57670;
						theColor.green = 1;
						theColor.blue = 1;
						RGBForeColor(&theColor);
					}
#if (TARGET_API_MAC_CARBON == 1)
					if (w->isUTF) {
						if (!DrawUTFontMac(sData+cCol, col-cCol, w->RowFInfo[row], sAtts[cCol])) {
//							SetPort(oldPort);
//							return;
						}
						if (is_reversed(sAtts[cCol])) {
							trect.left = LEFTMARGIN+w->textOffset+TextWidthInPix(sData, 0, cCol, w->RowFInfo[row], 0);
							trect.right = trect.left+TextWidthInPix(sData, cCol, col, w->RowFInfo[row], 0);
							trect.top = print_row - w->RowFInfo[row]->FHeight + 2;
							trect.bottom = print_row + 1;
//							trect.left += 3;
							trect.right += 3;
							InvertRect(&trect); 
						}
					} else
#endif
						DrawText(sData, cCol, col-cCol);
#if (TARGET_API_MAC_CARBON == 1)
					if (!w->isUTF) {
#endif
						if (RevChar)
							TextMode(srcCopy);
#if (TARGET_API_MAC_CARBON == 1)
					}
#endif
					if (is_colored(sAtts[cCol]) || is_error(sAtts[cCol]) || is_word_color(sAtts[cCol]))
						RGBForeColor(&oldForgColor);
					cCol = col;
					oldState = sAtts[col];
				}
			}
			RGBForeColor(&oldForgColor);
			TextFace(0);
			TextMode(srcCopy);
		}
#if (TARGET_API_MAC_CARBON == 1)
		DrawControls(global_df->wind); 		/* Draw all the controls */
#endif
	}
	if (global_df != NULL) {
		if (w == global_df->SoundWin) {
			PrintSoundWin(0,0,0L);
		}
	}
	SetPort(oldPort);
#endif // _MAC_CODE
#ifdef _WIN32
	long	print_row;
	long	fsize = -1;
	unsigned char charSet;
	char	attChanged;
	char	isRevText;
	unCH	fontName[LF_FACESIZE];
	int		leftPos;
	RECT	winRect;
	RECT	cRect;
	CSize	CW;
	SIZE	wCW;
	LOGFONT lfFont;
	CFont	l_font;
	CDC*	rDC;
	COLORREF bkColor, txtColor, oldTxtColor;
	CWnd* win;

	if (w == NULL)
		return;
	if (GlobalDC == NULL || GlobalDC->m_hDC == NULL)
		return;
	win = GlobalDC->GetWindow();
	if (win != NULL)
		win->GetClientRect(&winRect);
	else
		return;

	if (global_df->WinChange)
		global_df->RdW = w;

	oldTxtColor = GlobalDC->GetTextColor();
	fontName[0] = EOS;
	ecol = w->num_cols;
	pr = 0;
	tVar = -1;
	erow = ActualWinSize(w);
	if (w == global_df->w1 && erow >= w->num_rows)
		erow = w->num_rows - 1;
	for (row=0; row < erow; row++) {
		sAtts = w->win_atts[row];
		if (is_changed(sAtts[0])) {
			pr++;
			if (tVar == -1)
				tVar = row;
		}
		if (pr > 1)
			break;
	}
	isShowLineNumbers = (isShowLineNums && w == global_df->w1);
	if (pr) {
		row = tVar;
		if (pr == 1)
			erow = row + 1;
		if (w == global_df->w1 && erow >= w->num_rows)
			erow = w->num_rows - 1;
		print_row = w->LT_row + ComputeHeight(w,0,row);

		rDC = GlobalDC;

		SetLogfont(&lfFont, w->RowFInfo[row], NULL);
		l_font.CreateFontIndirect(&lfFont);
		CFont* pOldFont = rDC->SelectObject(&l_font);
		for (; row < erow; row++) {
			if (strcmp(fontName, w->RowFInfo[row]->FName) || 
						fsize != w->RowFInfo[row]->FSize  ||
						charSet != w->RowFInfo[row]->CharSet) {
				rDC->SelectObject(pOldFont);
				SetLogfont(&lfFont, w->RowFInfo[row], NULL);
				l_font.DeleteObject();
				l_font.CreateFontIndirect(&lfFont);
				rDC->SelectObject(&l_font);
				strcpy(fontName, w->RowFInfo[row]->FName);
				fsize = w->RowFInfo[row]->FSize;
				charSet = w->RowFInfo[row]->CharSet;
			}
			sAtts = w->win_atts[row];

			sData = w->win_data[row];
			isRevText = FALSE;
			lastViewCol = ecol; //0;
			for (col=0; col < ecol; col++) {
				if (sData[col] == 0 && lastViewCol == ecol && (!isShowLineNumbers || col > 0)) {
					lastViewCol = col;
//					break;
				}
				if (is_colored(sAtts[col]))
					sAtts[col] = set_color_to_0(sAtts[col]);
			}
			if (!is_changed(sAtts[0])) {
				if (lastViewCol == 0)
					goto contLoop;
			}
			sAtts[0] = set_changed_to_0(sAtts[0]);
			if (lastViewCol > ecol)
				lastViewCol = ecol;
			if (global_df->RootColorText != NULL && w == global_df->w1)
				tierColor = FindColorKeywordsBounds(global_df->RootColorText, sAtts, sData, 0, lastViewCol, tierColor);
			cCol = 0;
			oldState = sAtts[0];
			if (isShowLineNumbers) {
				leftPos = LEFTMARGIN;
				if (/*LineNumberingType == 0 || */w->lineno[row] != 0L) {
					wsprintf(templine4, cl_T("%ld"), w->lineno[row]);
					len = 6 - strlen(templine4);
					if (len > 0) {
						uS.shiftright(templine4, len);
						for (len--; len >= 0; len--) {
							templine4[len] = ' ';
						}
					}
				} else
					strcpy(templine4, "      ");
				if (LineNumberDigitSize != 0) {
					if (strlen(templine4) < LineNumberDigitSize) {
						strncat(templine4, "                     ", LineNumberDigitSize);
					}
					templine4[LineNumberDigitSize] = EOS;
				}
				cRect.top = print_row;
				cRect.bottom = print_row + w->RowFInfo[row]->FHeight;
				cRect.right = LEFTMARGIN+w->textOffset;
				cRect.left = LEFTMARGIN;
				GlobalDC->FillSolidRect(&cRect, RGB(255,255,255));
				rDC->TextOut(leftPos, print_row, templine4, strlen(templine4));
			}
			CW.cx = 0;
			CW.cy = 0;
			leftPos = LEFTMARGIN+w->textOffset;
			for (col=0; col <= lastViewCol; col++) {
				if (oldState != sAtts[col] || col == lastViewCol) {
					attChanged = FALSE;
					if (is_underline(sAtts[cCol])/* || is_error(sAtts[cCol])*/) {
						if (!lfFont.lfUnderline) {
							lfFont.lfUnderline = TRUE;
							attChanged = TRUE;
						}
					} else if (lfFont.lfUnderline) {
						lfFont.lfUnderline = FALSE;
						attChanged = TRUE;
					}
					if (is_italic(sAtts[cCol])) {
						if (!lfFont.lfItalic) {
							lfFont.lfItalic = TRUE;
							attChanged = TRUE;
						}
					} else if (lfFont.lfItalic) {
						lfFont.lfItalic = FALSE;
						attChanged = TRUE;
					}
					if (is_bold(sAtts[cCol])) {
						if (lfFont.lfWeight != FW_BOLD) {
							lfFont.lfWeight = FW_BOLD;
							attChanged = TRUE;
						}
					} else if (lfFont.lfWeight == FW_BOLD) {
						lfFont.lfWeight = FW_NORMAL;
						attChanged = TRUE;
					}

					if (attChanged) {
						rDC->SelectObject(pOldFont);
						l_font.DeleteObject();
						l_font.CreateFontIndirect(&lfFont);
						rDC->SelectObject(&l_font);
					}
					RevChar = is_reversed(sAtts[cCol]);
					if (RevChar) {
						bkColor = rDC->SetBkColor(RGB(0,0,0));
						txtColor = rDC->SetTextColor(RGB(255,255,255));
						isRevText = TRUE;
					}

					if (is_colored(sAtts[cCol])) {
						if (SetKeywordsColor(global_df->RootColorText, cCol, &theColor))
							rDC->SetTextColor(RGB(theColor.red,theColor.green,theColor.blue));
					}
					if (is_word_color(sAtts[cCol])) {
						char color;

						color = get_color_num(sAtts[cCol]);
						if (color == blue_color) {
							rDC->SetTextColor(RGB(0,0,240));
						} else if (color == red_color) {
							rDC->SetTextColor(RGB(240,0,0));
						} else if (color == green_color) {
							rDC->SetTextColor(RGB(0,192,0));
						} else /* if (color == magenta_color) */ {
							rDC->SetTextColor(RGB(192,0,192));
						}
					}
					if (is_error(sAtts[cCol])) {
						rDC->SetTextColor(RGB(240,0,0));
					}
					if (w->isUTF) {
						unsigned short *puText=NULL;
						long totalw=0;
						puText = sData+cCol;
						totalw = col-cCol;
						if (puText[0] != EOS) {
							TextOutW(rDC->m_hDC, leftPos, print_row, puText, totalw);
							if (GetTextExtentPointW(GlobalDC->m_hDC,puText,totalw,&wCW) != 0) {
								CW.cx = (short)wCW.cx;
								CW.cy = (short)wCW.cy;
							} else {
								CW.cx = 0;
								CW.cy = 0;
							}
						} else {
							CW.cx = 0;
							CW.cy = 0;
						}
					} else {
						rDC->TextOut(leftPos, print_row, sData+cCol, col-cCol);
						CW = rDC->GetTextExtent(sData+cCol, col-cCol);
					}
					if (RevChar) {
						rDC->SetTextColor(txtColor);
						rDC->SetBkColor(bkColor);
						isRevText = FALSE;
					}
					leftPos += CW.cx;
					if (is_colored(sAtts[cCol]) || is_error(sAtts[cCol]) || is_word_color(sAtts[cCol]))
						rDC->SetTextColor(oldTxtColor);
					cCol = col;
					oldState = sAtts[col];
				}
			}
contLoop:
			rDC->SelectObject(pOldFont);
			lfFont.lfUnderline = FALSE;
			lfFont.lfItalic = FALSE;
			lfFont.lfWeight = FW_NORMAL;
			l_font.DeleteObject();
			l_font.CreateFontIndirect(&lfFont);
			rDC->SelectObject(&l_font);
			if (isRevText) {
				rDC->SetTextColor(txtColor);
				rDC->SetBkColor(bkColor);
			}
			if (win != NULL) {
				cRect.top = print_row;
				cRect.bottom = print_row + w->RowFInfo[row]->FHeight;
				cRect.right = winRect.right;
				cRect.left = leftPos;
				GlobalDC->FillSolidRect(&cRect, RGB(255,255,255));
			}
			print_row += w->RowFInfo[row]->FHeight;
		}
		rDC->SelectObject(pOldFont);
		l_font.DeleteObject();
	}
	if (global_df != NULL) {
		if (w == global_df->SoundWin)
			PrintSoundWin(0,0,0L);
	}
#endif /* _WIN32 */
}
