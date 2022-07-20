#include "ced.h"
#include "c_clan.h"

#define CF_MCED 100
#define CF_UCED 101

static AttTYPE *CHECKATT(AttTYPE *att, long *cnt, AttTYPE *oldAtt) {
	if (att != NULL) {
		*cnt = DealWithAtts(NULL, *cnt, *att, *oldAtt, TRUE);
		*oldAtt = *att;
		att++;
	} else {
		*cnt = DealWithAtts(NULL, *cnt, 0, *oldAtt, TRUE);
		*oldAtt = 0;
	}
	return(att);
}

static AttTYPE *HANDLEATT(unCH *pt, AttTYPE *att, long *cnt, AttTYPE *oldAtt) {
	if (att != NULL) {
		*cnt = DealWithAtts(pt, *cnt, *att, *oldAtt, FALSE);
		*oldAtt = *att;
		att++;
	} else {
		*cnt = DealWithAtts(pt, *cnt, 0, *oldAtt, FALSE);
		oldAtt = 0;
	}
	return(att);
}

static void CHECKDATA(unCH *src, long *cnt, char *NL_F) {
	if (*src == NL_C) {
		(*cnt) += 1;
		*NL_F = TRUE;
	} else {
		(*cnt) += 1;
	}
}

static void COPYDATA(unCH *pt, unCH *src, long *cnt, char *NL_F) {
	if (*src == NL_C) {
		pt[*cnt] = '\n';
		(*cnt) += 1;
		*NL_F = TRUE;
	} else {
		pt[*cnt] = *src;
		(*cnt) += 1;
	}
}

static void CHECKLINETERM(long *cnt, char NL_F) {
	if (doReWrap && global_df->ChatMode) {
	} else if (global_df->ChatMode && !NL_F) {
		(*cnt) += 1;
	} else if (!NL_F) {
		(*cnt) += 1;
	}
}

static void LINETERMINATOR(unCH *pt, long *cnt, char NL_F) {
	if (doReWrap && global_df->ChatMode) {
	} else if (global_df->ChatMode && !NL_F) {
		pt[*cnt] = '\n';
		(*cnt) += 1;
	} else if (!NL_F) {
		pt[*cnt] = ' ';
		(*cnt) += 1;
	}
}

static long createLineNumberStr(LPTSTR pt, long cnt, ROWS *st, unsigned long lineno) {
	long nDigs, len;

	len = 0;
	if (LineNumberingType == 0 || isMainSpeaker(st->line[0])) {
		uS.sprintf(templine4, cl_T("%ld"), lineno);
		if (LineNumberDigitSize < 0)
			nDigs = 6L;
		else if (LineNumberDigitSize > 0)
			nDigs = (unsigned long)LineNumberDigitSize;
		else
			nDigs = 6;
		if (nDigs > 20)
			nDigs = 20;
		len = strlen(templine4);
		while (len < nDigs) {
			strcat(templine4, " ");
			len++;
		}
	}
	if (pt != NULL)
		strcpy(pt+cnt, templine4);
	return(cnt+len);
}

static ROWS *copyToNextRow(ROWS *p, unsigned long *lineno) {
	if (!global_df->ChatMode) {
		return(p->next_row);
	} else {
		while (p != global_df->tail_text) {
			p = p->next_row;
			if (CMP_VIS_ID(p->flag))
				return(p);
			else
				(*lineno)++;
		}
		return(p);
	}
}

char doCopy(void) {
	LPTSTR  lptstrCopy1, lptstrCopy2; 
	HGLOBAL hglbCopy1, hglbCopy2;
	unCH *src;
	char NL_F, prevNL_F;
    AttTYPE *att, oldAtt;
	long cnt, oCnt;
	long sc, ec, tc;
	unsigned long len;
	unsigned long lineno;
	ROWS *tst, *st, *et;

	if (!GlobalDC->GetWindow()->OpenClipboard())
		return(FALSE); 
	if (global_df->ScrollBar && (global_df->row_win2 || global_df->col_win2 != -2L)) {
		EmptyClipboard(); 
		ChangeCurLineAlways(0);
		if (global_df->row_win2 == 0) {
// for CLAN
			if (global_df->col_win > global_df->col_win2) {
				sc = global_df->col_chr2;
				ec = global_df->col_chr;
			} else {
				sc = global_df->col_chr;
				ec = global_df->col_chr2;
			}
			if (global_df->row_txt->att != NULL)
				att = global_df->row_txt->att + sc;
			else
				att = NULL;
			oldAtt = 0;
			cnt = 0L;
			for (tc=sc; tc < ec; tc++) {
				att = CHECKATT(att, &cnt, &oldAtt);
				cnt++;
			}
			if ((hglbCopy1=GlobalAlloc(GMEM_DDESHARE, (cnt+1)*sizeof(unCH))) == NULL) { 
				CloseClipboard(); 
				strcpy(global_df->err_message, "+Cut/Copy; out of memory!");
				PosAndDispl();
				return(FALSE); 
			} 
			lptstrCopy1 = (LPTSTR)GlobalLock(hglbCopy1); 

			src = global_df->row_txt->line;
			if (global_df->row_txt->att != NULL)
				att = global_df->row_txt->att + sc;
			else
				att = NULL;
			oldAtt = 0;
			cnt = 0L;
			while (sc < ec) {
				att = HANDLEATT(lptstrCopy1, att, &cnt, &oldAtt);
				lptstrCopy1[cnt++] = src[sc++];
			}
			lptstrCopy1[cnt] = (TCHAR) 0;    // null character 
			SetClipboardData(CF_UCED, hglbCopy1);
			GlobalUnlock(hglbCopy1); 
//			GlobalFree(hglbCopy1);

// for other APPs
			if (FALSE/*isShowLineNums*/) {
				lineno = 1L;
				for (tst=global_df->head_text->next_row; tst != global_df->tail_text; tst=tst->next_row) {
					if (tst == global_df->row_txt)
						break;
					if (LineNumberingType == 0 || isMainSpeaker(tst->line[0]))
						lineno++;
				}
			}
			if (global_df->col_win > global_df->col_win2) {
				sc = global_df->col_chr2;
				ec = global_df->col_chr;
			} else {
				sc = global_df->col_chr;
				ec = global_df->col_chr2;
			}
			cnt = 0L;
			if (FALSE/*isShowLineNums*/)
				cnt = createLineNumberStr(NULL, cnt, global_df->row_txt, lineno);
			for (tc=sc; tc < ec; tc++) {
				cnt++;
			}
			if ((hglbCopy2=GlobalAlloc(GMEM_DDESHARE, (cnt+1)*sizeof(unCH))) == NULL) { 
				CloseClipboard(); 
				strcpy(global_df->err_message, "+Cut/Copy; out of memory!");
				PosAndDispl();
				return(FALSE); 
			} 
			lptstrCopy2 = (LPTSTR)GlobalLock(hglbCopy2);

			src = global_df->row_txt->line;
			cnt = 0L;
			if (FALSE/*isShowLineNums*/)
				cnt = createLineNumberStr(lptstrCopy2, cnt, global_df->row_txt, lineno);
			while (sc < ec) {
				if (src[sc] == HIDEN_C && global_df->ShowParags != '\002') {
					sc++;
					if (sc >= ec)
						break;
					while (sc < ec && src[sc] != HIDEN_C)
						sc++;
				} else
					lptstrCopy2[cnt++] = src[sc];
				sc++;
			}
			lptstrCopy2[cnt] = (TCHAR) 0;    // null character 

			for (cnt=0L; lptstrCopy2[cnt]; ) {
				if (lptstrCopy2[cnt] == HIDEN_C) {
					lptstrCopy2[cnt] = 0x2022;
					cnt++;
				} else if (lptstrCopy2[cnt] == ATTMARKER)
					strcpy(lptstrCopy2+cnt, lptstrCopy2+cnt+2);
				else
					cnt++;
			}

			if (global_df->isUTF) {
			    SetClipboardData(CF_UNICODETEXT, hglbCopy2);
			} else
				SetClipboardData(CF_TEXT, hglbCopy2);

			GlobalUnlock(hglbCopy2); 
//			GlobalFree(hglbCopy2);
			CloseClipboard(); 
		} else {
// for CLAN
		if (global_df->row_win2 < 0L) {
				et = global_df->row_txt;
				for (cnt=global_df->row_win2, st=global_df->row_txt; cnt && !AtTopEnd(st,global_df->head_text,FALSE); 
								cnt++, st=ToPrevRow(st, FALSE)) ;
				sc = global_df->col_chr2; ec = global_df->col_chr;
			} else if (global_df->row_win2 > 0L) {
				st = global_df->row_txt;
				for (cnt=global_df->row_win2, et=global_df->row_txt; cnt && !AtBotEnd(et,global_df->tail_text,FALSE);
								cnt--, et=ToNextRow(et, FALSE)) ;
				sc = global_df->col_chr; ec = global_df->col_chr2;
			}
			tst = st;
    		cnt = 0L;
    		NL_F = FALSE;
			if (st->att != NULL)
				att = st->att + sc;
			else
				att = NULL;
			oldAtt = 0;
    		for (src=st->line+sc; *src; src++) {
				att = CHECKATT(att, &cnt, &oldAtt);
				CHECKDATA(src, &cnt, &NL_F);
   			}
   			CHECKLINETERM(&cnt, NL_F);
			if (!AtBotEnd(st,et,FALSE)) {
				do {
					st = ToNextRow(st, FALSE);
  			  		NL_F = FALSE;
					att = st->att;
  			  		for (src=st->line; *src; src++) {
  			  			att = CHECKATT(att, &cnt, &oldAtt);
  			  			CHECKDATA(src, &cnt, &NL_F);
   					}
   					CHECKLINETERM(&cnt, NL_F);
				} while (!AtBotEnd(st,et,FALSE)) ;
			}
			st = ToNextRow(st, FALSE);
			att = st->att;
			if (ec == strlen(st->line))
 	 			NL_F = FALSE;
			for (src=st->line, tc = ec; tc && *src; tc--, src++) {
				att = CHECKATT(att, &cnt, &oldAtt);
				CHECKDATA(src, &cnt, &NL_F);
			}
			if (ec == strlen(st->line)) {
   				CHECKLINETERM(&cnt, NL_F);
   			}

			if ((hglbCopy1=GlobalAlloc(GMEM_DDESHARE, (cnt+1)*sizeof(unCH))) == NULL) { 
				CloseClipboard(); 
				strcpy(global_df->err_message, "+Cut/Copy; out of memory!");
				PosAndDispl();
				return(FALSE); 
			} 
			lptstrCopy1 = (LPTSTR)GlobalLock(hglbCopy1); 

			st = tst;
    		cnt = 0L;
    		NL_F = FALSE;
			if (st->att != NULL)
				att = st->att + sc;
			else
				att = NULL;
			oldAtt = 0;
			prevNL_F = TRUE;
			if (st != global_df->head_text->next_row && st != global_df->head_text && sc == 0) {
				len = strlen(st->prev_row->line);
				if (len > 0) {
					if (st->prev_row->line[len-1] != NL_C)
						prevNL_F = FALSE;
				}
			}
			oCnt = cnt;
			for (src = st->line + sc; *src; src++) {
				att = HANDLEATT(lptstrCopy1, att, &cnt, &oldAtt);
				COPYDATA(lptstrCopy1, src, &cnt, &NL_F);
   			}
			if (prevNL_F == FALSE && global_df->ChatMode && doReWrap) {
				if (lptstrCopy1[oCnt] == '\t') {
					lptstrCopy1[oCnt] = ' ';
				}
			}
			prevNL_F = NL_F;
			LINETERMINATOR(lptstrCopy1, &cnt, NL_F);
			if (!AtBotEnd(st,et,FALSE)) {
				do {
					st = ToNextRow(st, FALSE);
  			  		NL_F = FALSE;
					att = st->att;
					oCnt = cnt;
					for (src = st->line; *src; src++) {
  			  			att = HANDLEATT(lptstrCopy1, att, &cnt, &oldAtt);
  			  			COPYDATA(lptstrCopy1, src, &cnt, &NL_F);
   					}
					if (prevNL_F == FALSE && global_df->ChatMode && doReWrap) {
						if (lptstrCopy1[oCnt] == '\t') {
							lptstrCopy1[oCnt] = ' ';
						}
					}
					prevNL_F = NL_F;
					LINETERMINATOR(lptstrCopy1, &cnt, NL_F);
				} while (!AtBotEnd(st,et,FALSE)) ;
			}
			st = ToNextRow(st, FALSE);
			att = st->att;
			if (ec == strlen(st->line))
 	 			NL_F = FALSE;
			oCnt = cnt;
			for (src = st->line, tc = ec; tc && *src; tc--, src++) {
				att = HANDLEATT(lptstrCopy1, att, &cnt, &oldAtt);
				COPYDATA(lptstrCopy1, src, &cnt, &NL_F);
			}
			if (prevNL_F == FALSE && global_df->ChatMode && doReWrap) {
				if (lptstrCopy1[oCnt] == '\t') {
					lptstrCopy1[oCnt] = ' ';
				}
			}
			prevNL_F = NL_F;
			if (ec == strlen(st->line)) {
   				LINETERMINATOR(lptstrCopy1, &cnt, NL_F);
   			}
			lptstrCopy1[cnt] = (TCHAR) 0;    // null character 
			SetClipboardData(CF_UCED, hglbCopy1);
			GlobalUnlock(hglbCopy1); 
//			GlobalFree(hglbCopy1);

// for other APPs
			if (global_df->row_win2 < 0L) {
				et = global_df->row_txt;
				for (cnt=global_df->row_win2, st=global_df->row_txt; cnt && !AtTopEnd(st,global_df->head_text,FALSE); 
								cnt++, st=ToPrevRow(st, FALSE)) ;
				sc = global_df->col_chr2; ec = global_df->col_chr;
			} else if (global_df->row_win2 > 0L) {
				st = global_df->row_txt;
				for (cnt=global_df->row_win2, et=global_df->row_txt; cnt && !AtBotEnd(et,global_df->tail_text,FALSE);
								cnt--, et=ToNextRow(et, FALSE)) ;
				sc = global_df->col_chr; ec = global_df->col_chr2;
			}
			if (FALSE/*isShowLineNums*/) {
				lineno = 1L;
				for (tst=global_df->head_text->next_row; tst != global_df->tail_text; tst=tst->next_row) {
					if (tst == st)
						break;
					if (LineNumberingType == 0 || isMainSpeaker(tst->line[0]))
						lineno++;
				}
			}
			tst = st;
    		cnt = 0L;
    		NL_F = FALSE;
			if (FALSE/*isShowLineNums*/)
				cnt = createLineNumberStr(NULL, cnt, st, lineno);
    		for (src=st->line+sc; *src; src++) {
				CHECKDATA(src, &cnt, &NL_F);
   			}
   			CHECKLINETERM(&cnt, NL_F);
			if (!AtBotEnd(st,et,FALSE)) {
				do {
					st = ToNextRow(st, FALSE);
  			  		NL_F = FALSE;
					if (FALSE/*isShowLineNums*/)
						cnt = createLineNumberStr(NULL, cnt, st, lineno);
  			  		for (src=st->line; *src; src++) {
  			  			CHECKDATA(src, &cnt, &NL_F);
   					}
   					CHECKLINETERM(&cnt, NL_F);
				} while (!AtBotEnd(st,et,FALSE)) ;
			}
			st = ToNextRow(st, FALSE);
			if (ec == strlen(st->line))
 	 			NL_F = FALSE;
			if (FALSE/*isShowLineNums*/)
				cnt = createLineNumberStr(NULL, cnt, st, lineno);
			for (src=st->line, tc = ec; tc && *src; tc--, src++) {
				CHECKDATA(src, &cnt, &NL_F);
			}
			if (ec == strlen(st->line)) {
   				CHECKLINETERM(&cnt, NL_F);
   			}

			if ((hglbCopy2=GlobalAlloc(GMEM_DDESHARE, (cnt+1)*sizeof(unCH))) == NULL) { 
				CloseClipboard(); 
				strcpy(global_df->err_message, "+Cut/Copy; out of memory!");
				PosAndDispl();
				return(FALSE); 
			} 
			lptstrCopy2 = (LPTSTR)GlobalLock(hglbCopy2);

			st = tst;
    		cnt = 0L;
    		NL_F = FALSE;
			if (FALSE/*isShowLineNums*/) {
				cnt = createLineNumberStr(lptstrCopy2, cnt, st, lineno);
				lineno++;
			}
			prevNL_F = TRUE;
			if (st != global_df->head_text->next_row && st != global_df->head_text && sc == 0) {
				len = strlen(st->prev_row->line);
				if (len > 0) {
					if (st->prev_row->line[len - 1] != NL_C)
						prevNL_F = FALSE;
				}
			}
			oCnt = cnt;
			for (src = st->line + sc; *src; src++) {
				if (*src == HIDEN_C && global_df->ShowParags != '\002') {
					src++;
					if (*src == EOS)
						break;
					while (*src != EOS && *src != HIDEN_C)
						src++;
					if (*src == EOS)
						break;
				} else
					COPYDATA(lptstrCopy2, src, &cnt, &NL_F);
   			}
			if (prevNL_F == FALSE && global_df->ChatMode && doReWrap) {
				if (lptstrCopy2[oCnt] == '\t') {
					lptstrCopy2[oCnt] = ' ';
				}
			}
			prevNL_F = NL_F;
			LINETERMINATOR(lptstrCopy2, &cnt, NL_F);
			if (!AtBotEnd(st,et,FALSE)) {
				do {
					NL_F = FALSE;
					oCnt = cnt;
					if (FALSE/*isShowLineNums*/) {
						st = copyToNextRow(st, &lineno);
						cnt = createLineNumberStr(lptstrCopy2, cnt, st, lineno);
						lineno++;
					} else
						st = ToNextRow(st, FALSE);
  			  		for (src=st->line; *src; src++) {
						if (*src == HIDEN_C && global_df->ShowParags != '\002') {
							src++;
							if (*src == EOS)
								break;
							while (*src != EOS && *src != HIDEN_C)
								src++;
							if (*src == EOS)
								break;
						} else
	  			  			COPYDATA(lptstrCopy2, src, &cnt, &NL_F);
   					}
					if (prevNL_F == FALSE && global_df->ChatMode && doReWrap) {
						if (lptstrCopy2[oCnt] == '\t') {
							lptstrCopy2[oCnt] = ' ';
						}
					}
					prevNL_F = NL_F;
					LINETERMINATOR(lptstrCopy2, &cnt, NL_F);
				} while (!AtBotEnd(st,et,FALSE)) ;
			}
			if (FALSE/*isShowLineNums*/)
				st = copyToNextRow(st, &lineno);
			else
				st = ToNextRow(st, FALSE);
			if (ec == strlen(st->line))
 	 			NL_F = FALSE;
			oCnt = cnt;
			if (FALSE/*isShowLineNums*/) {
				cnt = createLineNumberStr(lptstrCopy2, cnt, st, lineno);
				lineno++;
			}
			for (src=st->line, tc = ec; tc && *src; tc--, src++) {
				if (*src == HIDEN_C && global_df->ShowParags != '\002') {
					src++;
					tc--;
					if (!tc || *src == EOS)
						break;
					while (tc && *src != EOS && *src != HIDEN_C) {
						src++;
						tc--;
					}
					if (!tc || *src == EOS)
						break;
				} else
					COPYDATA(lptstrCopy2, src, &cnt, &NL_F);
			}
			if (prevNL_F == FALSE && global_df->ChatMode && doReWrap) {
				if (lptstrCopy2[oCnt] == '\t') {
					lptstrCopy2[oCnt] = ' ';
				}
			}
			prevNL_F = NL_F;
			if (ec == strlen(st->line)) {
   				LINETERMINATOR(lptstrCopy2, &cnt, NL_F);
   			}
			lptstrCopy2[cnt] = (TCHAR) 0;    // null character 

			for (cnt=0L; lptstrCopy2[cnt]; ) {
				if (lptstrCopy2[cnt] == HIDEN_C) {
					lptstrCopy2[cnt] = 0x2022;
					cnt++;
				} else if (lptstrCopy2[cnt] == ATTMARKER)
					strcpy(lptstrCopy2+cnt, lptstrCopy2+cnt+2);
				else
					cnt++;
			}

			if (global_df->isUTF) {
			    SetClipboardData(CF_UNICODETEXT, hglbCopy2);
			} else
				SetClipboardData(CF_TEXT, hglbCopy2);

			GlobalUnlock(hglbCopy2); 
//			GlobalFree(hglbCopy2);
			CloseClipboard(); 
		}
	}
	return(TRUE);
}

void doPaste(void) {
    HGLOBAL   hglb; 
    LPTSTR    lptstr;
	unCH *s;
	long sc = 0L, len;
	UINT	  ClipBoardFormat;

	ClipBoardFormat = CF_UCED;
	if (IsClipboardFormatAvailable(ClipBoardFormat)) {
		if (GlobalDC->GetWindow()->OpenClipboard()) {
			if ((hglb=GetClipboardData(ClipBoardFormat)) != NULL) {
				SaveUndoState(FALSE);
				if (DeleteChank(1)) {
					if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
						PutCursorInWindow(global_df->w1);
					global_df->row_win2 = 0L;
					global_df->col_win2 = -2L;
					global_df->col_chr2 = -2L;

					lptstr = (LPTSTR)GlobalLock(hglb); 
					if (lptstr != NULL) {
						len = strlen(lptstr);
						for (s=lptstr; *s; s++) {
							if (*s == '\n') {
//								if (global_df->ChatMode)
//									*s = SNL_C;
//								else
									*s = NL_C;
							}
						}
						if (global_df->RdW == global_df->w1) {
							global_df->WinChange = FALSE;
							if (len > 1625)
								ResetUndos();
							SaveUndoState(FALSE);
							AddText(lptstr, '\0', 0, len);
							GlobalUnlock(hglb);
							DisplayTextWindow(NULL, 1);
							global_df->WinChange = TRUE;
							PosAndDispl();
						} else {
							GlobalUnlock(hglb);
						}
					}
				}
			}
			CloseClipboard();
		}
		return;
	}

	ClipBoardFormat = CF_UNICODETEXT;
	if (IsClipboardFormatAvailable(ClipBoardFormat)) {
		if (GlobalDC->GetWindow()->OpenClipboard()) {
			if ((hglb=GetClipboardData(ClipBoardFormat)) != NULL) {
				SaveUndoState(FALSE);
				if (DeleteChank(1)) {
					if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
						PutCursorInWindow(global_df->w1);
					global_df->row_win2 = 0L;
					global_df->col_win2 = -2L;
					global_df->col_chr2 = -2L;

					lptstr = (LPTSTR)GlobalLock(hglb); 
					if (lptstr != NULL) {
						len = strlen(lptstr);
						for (s=lptstr; *s; s++) {
							if (*s == 0x2022)
								*s = HIDEN_C;
							else if (*s == '\n') {
//								if (global_df->ChatMode)
//									*s = SNL_C;
//								else
									*s = NL_C;
							}
						}
						if (global_df->RdW == global_df->w1) {
							global_df->WinChange = FALSE;
							if (len > 1625)
								ResetUndos();
							SaveUndoState(FALSE);
							AddText(lptstr, '\0', 0, len);
							GlobalUnlock(hglb);
							DisplayTextWindow(NULL, 1);
							global_df->WinChange = TRUE;
							PosAndDispl();
						} else {
							GlobalUnlock(hglb);
						}
					}
				}
			}
			CloseClipboard();
		}
	}
}
