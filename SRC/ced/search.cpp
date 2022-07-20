#include "ced.h"
#include "cu.h"
#include "my_ctype.h"
#include "search.h"
#ifdef _WIN32 
#include "CedDlgs.h"
#endif

static struct uIEWords {
    unCH *word;
    struct uIEWords *nextword;
} *searchList = NULL;

char CaseSensSearch;
char SearchWrap;
char SearchFromList;
char ReplaceFromList;
char replaceExecPos;
unCH SearchString[SPEAKERLEN], ReplaceString[SPEAKERLEN];
FNType searchListFileName[FNSize];
FNType replaceListFileName[FNSize];

void init_Search() {
	*SearchString = EOS;
	*ReplaceString = EOS;
	CaseSensSearch = FALSE;
	SearchWrap = FALSE;
	searchList = NULL;
	SearchFromList = FALSE;
	ReplaceFromList = FALSE;
	searchListFileName[0] = EOS;
	replaceListFileName[0] = EOS;
}

short isThereSearchList(void) {
	return(searchList != NULL);
}

static void freeSearchList(void) {
	struct uIEWords *t;

	while (searchList != NULL) {
		t = searchList;
		searchList = searchList->nextword;
		if (t->word)
			free(t->word);
		free(t);
	}
}

static char AddWordTosearchList(unCH *word) {
	unCH *t;
	register int l1;
	register int l2;
	struct uIEWords *nt, *tnt;

	t = (unCH *)malloc((strlen(word)+1)*sizeof(unCH));
	if (t == NULL) {
		do_warning("Out of memory.", 0);
		return(FALSE);
	}
	l1 = strlen(word);
	if (searchList == NULL) {
		searchList = NEW(struct uIEWords);
		nt = searchList;
		if (nt == NULL) {
			do_warning("Out of memory.", 0);
			free(t);
			return(FALSE);
		}
		nt->nextword = NULL;
	} else {
		tnt= searchList;
		nt = searchList;
		while (1) {
			if (nt == NULL)
				break;
			else if (uS.partcmp(word,nt->word,FALSE,TRUE)) {
				l2 = strlen(nt->word);
				if (l1 > l2)
					break;
				else {
					free(t);
					return(TRUE);
				}
			}
			tnt = nt;
			nt = nt->nextword;
		}
		if (nt == NULL) {
			tnt->nextword = NEW(struct uIEWords);
			nt = tnt->nextword;
			if (nt == NULL) {
				do_warning("Out of memory.", 0);
				free(t);
				return(FALSE);
			}
			nt->nextword = NULL;
		} else if (nt == searchList) {
			searchList = NEW(struct uIEWords);
			searchList->nextword = nt;
			nt = searchList;
			if (nt == NULL) {
				do_warning("Out of memory.", 0);
				free(t);
				return(FALSE);
			}
		} else {
			nt = NEW(struct uIEWords);
			if (nt == NULL) {
				do_warning("Out of memory.", 0);
				free(t);
				return(FALSE);
			}
			nt->nextword = tnt->nextword;
			tnt->nextword = nt;
		}
	}
	nt->word = t;
	strcpy(nt->word, word);
	return(TRUE);
}

char readSearchList(FNType *fname) {
	int i;
	unCH *sPtr;
	FILE *fp;

	if ((fp=fopen(fname, "r")) == NULL) {
		do_warning("Can't open search list file", 0);
		return(FALSE);
	}
	freeSearchList();
	while (fgets_cr(templineC, UTTLINELEN, fp) != NULL) {
		if ((templineC[0] == '%' || templineC[0] == '#') && isSpace(templineC[1]))
			continue;
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
			continue;
		if (!strcmp(templineC,"\n"))
			continue;
		for (i=0; templineC[i];) {
			if (templineC[i] == '\\') {
				if (templineC[i+1] == 'r' || templineC[i+1] == 'n')
					templineC[i+1] = CR_CHR;
				else if (templineC[i+1] == 't')
					templineC[i+1] = '\t';
				strcpy(templineC+i, templineC+i+1);
			} else
				i++;
		}	
		if (i > 0 && templineC[i-1] == '\n')
			templineC[i-1] = EOS;

		UTF8ToUnicode((unsigned char *)templineC, strlen(templineC), templine, NULL, UTTLINELEN);
		sPtr = templine;

		if (!CaseSensSearch)
			uS.uppercasestr(sPtr, &dFnt, C_MBF);
		if (!AddWordTosearchList(sPtr)) 
			return(FALSE);
	}
	fclose(fp);
	return(TRUE);
}

static char isMatchPat(unCH *pat, unCH *s, LINE *tl, char isForward) {
	register unCH tc, *s1, *s2;
	LINE *l;

	if (global_df->row_txt == global_df->cur_line) {
		for (s=pat, l=tl; *s && l != global_df->tail_row; s++, l=l->next_char) {
			if (!CaseSensSearch)
				tc = (unCH)to_unCH_upper(l->c);
			else
				tc = l->c;
			if (*s != tc) {
				if (l->c == NL_C && global_df->ShowParags != '\001' && *s == '\r') ;
				else if (*s == '\r' && l == global_df->head_row->next_char)
					l = l->prev_char;
				else break;
			}
		}
		if (*s == '\r' && l == global_df->tail_row)
			s++;

		if (*s == EOS) {
			if (isForward)
				global_df->col_txt = l;
			else
				global_df->col_txt = tl;
			return(1);
		}
	} else {
		for (s1=pat, s2=s; *s1 != EOS; s1++, s2++) {
			if (!CaseSensSearch)
				tc = (unCH)to_unCH_upper(*s2);
			else
				tc = *s2;
			if (*s1 != tc) {
				if (*s2 == NL_C && global_df->ShowParags != '\001' && *s1 == '\r') ;
				else if (*s1 == '\r' && s2 == global_df->row_txt->line) s2--;
				else break;
			}
		}
		if (*s1 == '\r' && *s2 == EOS)
			s1++;

		if (*s1 == EOS) {
			return(1);
		}
	}
	return(0);
}

static void finishFoundForward(unCH *pat, int offset, int len, char isBeg) {
	if (global_df->row_txt == global_df->cur_line) {
		if (*pat == '\r' && *(pat+1) != EOS)
			len--;
		if (isBeg)
			global_df->col_chr = offset + len;
		else 
			global_df->col_chr = global_df->col_chr + offset + len;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		global_df->col_chr2 = global_df->col_chr - len;
		global_df->col_win2 = ComColWin(FALSE, NULL, global_df->col_chr2);
	} else {
		if (*pat == '\r' && *(pat+1) != EOS)
			len--;
		if (isBeg)
			global_df->col_chr = offset + len;
		else
			global_df->col_chr = global_df->col_chr + offset + len;
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		global_df->col_chr2 = global_df->col_chr - len;
		global_df->col_win2 = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr2);
	}
}

static int StringMatched(unCH *pat, char isBeg, char isSearchFromList) {
	unCH *s;
	int offset = 0, len;
	struct uIEWords *t;

	if ((len=strlen(pat)) == 0)
		return(0);
	if (global_df->row_txt == global_df->cur_line) {
		LINE *tl;
		if (isBeg)
			tl = global_df->head_row->next_char;
		else
			tl = global_df->col_txt;
		if (tl == NULL)
			return(0);
		if (tl->c == HIDEN_C && tl != global_df->tail_row && global_df->ShowParags != '\002') {
			for (tl=tl->next_char,offset++; tl->c!=HIDEN_C && tl!=global_df->tail_row; tl=tl->next_char,offset++) ;
		}
		while (tl != global_df->tail_row) {
			if (isSearchFromList && searchList != NULL) {
				for (t=searchList; t != NULL; t=t->nextword) {
					len = strlen(t->word);
					if (isMatchPat(t->word, NULL, tl, TRUE)) {
						finishFoundForward(pat, offset, len, isBeg);
						return(1);
					}
				}
			} else {
				if (isMatchPat(pat, NULL, tl, TRUE)) {
					finishFoundForward(pat, offset, len, isBeg);
					return(1);
				}
			}
			tl = tl->next_char;
			offset++;
			if (tl->c == HIDEN_C && tl != global_df->tail_row && global_df->ShowParags != '\002') {
				for (tl=tl->next_char,offset++; tl->c!=HIDEN_C && tl!=global_df->tail_row; tl=tl->next_char,offset++) ;
			}
		}
	} else {
		if (isBeg)
			s = global_df->row_txt->line;
		else
			s = global_df->row_txt->line+global_df->col_chr;
		if (s == NULL)
			return(0);
		if (*s == HIDEN_C && *s != EOS && global_df->ShowParags != '\002') {
			for (s++,offset++; *s != HIDEN_C && *s != EOS; s++,offset++) ;
		}
		while (*s != EOS) {
			if (isSearchFromList && searchList != NULL) {
				for (t=searchList; t != NULL; t=t->nextword) {
					len = strlen(t->word);
					if (isMatchPat(t->word, s, NULL, TRUE)) {
						finishFoundForward(pat, offset, len, isBeg);
						return(1);
					}
				}
			} else {
				if (isMatchPat(pat, s, NULL, TRUE)) {
					finishFoundForward(pat, offset, len, isBeg);
					return(1);
				}
			}
			s++;
			offset++;
			if (*s == HIDEN_C && *s != EOS && global_df->ShowParags != '\002') {
				for (s++,offset++; *s != HIDEN_C && *s != EOS; s++,offset++) ;
			}
		}
	}
	return(0);
}

int FindString(unCH *st, char updateScreen, char isWrap, char isSearchFromList) {
	long old_row_win = global_df->row_win;
	long old_lineno  = global_df->lineno;
	long old_wLineno = global_df->wLineno;
	ROWS *old_row_txt = global_df->row_txt;

	if (CMP_VIS_ID(global_df->row_txt->flag) && StringMatched(st, FALSE, isSearchFromList)) {
		global_df->lineno = countLines(global_df->row_txt);
		if (updateScreen)
			DisplayRow(TRUE);
		return(1);
	}
	global_df->row_txt = global_df->row_txt->next_row;
	if (CMP_VIS_ID(global_df->row_txt->flag)) {
		if (isNL_CFound(global_df->row_txt))
			global_df->lineno++;
		global_df->wLineno++;
	}
	if (global_df->row_win < (long)global_df->EdWinSize)
		global_df->row_win++;
	while (global_df->row_txt != global_df->tail_text) {
		if (CMP_VIS_ID(global_df->row_txt->flag) && StringMatched(st, TRUE, isSearchFromList)) {
			global_df->lineno = countLines(global_df->row_txt);
			if (updateScreen)
				DisplayRow(TRUE);
			return(1);
		}
		global_df->row_txt = global_df->row_txt->next_row;
		if (CMP_VIS_ID(global_df->row_txt->flag)) {
			if (isNL_CFound(global_df->row_txt))
				global_df->lineno++;
			global_df->wLineno++;
		}
		if (global_df->row_win < (long)global_df->EdWinSize)
			global_df->row_win++;
	}
	if (isWrap && global_df->head_text->next_row != global_df->tail_text) {
		global_df->row_txt = global_df->head_text->next_row;
		global_df->lineno  = 1L;
		global_df->wLineno = 1L;
		if (global_df->top_win == global_df->row_txt)
			global_df->row_win = 0L;
		while (global_df->row_txt != old_row_txt->next_row) {
			if (CMP_VIS_ID(global_df->row_txt->flag) && StringMatched(st, TRUE, isSearchFromList)) {
				global_df->lineno = countLines(global_df->row_txt);
				if (updateScreen)
					DisplayRow(TRUE);
				return(1);
			}
			global_df->row_txt = global_df->row_txt->next_row;
			if (CMP_VIS_ID(global_df->row_txt->flag)) {
				if (isNL_CFound(global_df->row_txt))
					global_df->lineno++;
				global_df->wLineno++;
			}
			if (global_df->row_win < (long)global_df->EdWinSize)
				global_df->row_win++;
		}
	}
	global_df->row_txt = old_row_txt;
	global_df->row_win = old_row_win;
	global_df->lineno  = old_lineno;
	global_df->wLineno = old_wLineno;
	return(0);
}

int SearchForward(int i) {
	global_df->LeaveHighliteOn = TRUE;
#if defined(_MAC_CODE)
	DrawSoundCursor(0);
	if (i > 0) {
		SearchFFlag = TRUE;
		if (!FindDialog(SearchString, 255)) {
			RemoveLastUndo();
			strcpy(global_df->err_message, DASHES);
			*sp = EOS;
			return(41);
		}
	}
#else
	int				ti;
	CCedFindString dlg;

	if (global_df == NULL)
		return(41);
	if (i > 0) {
		DrawCursor(1);
		SearchFFlag = TRUE;
		strcpy(templineW, SearchString);
		for (ti=0; templineW[ti] != EOS; ti++) {
			if (templineW[ti] == HIDEN_C)
				templineW[ti] = 0x2022;
		}
		dlg.m_SearchString = templineW;
		dlg.m_SearchBackwards = !SearchFFlag;
		dlg.m_Wrap = SearchWrap;
		dlg.m_CaseSensitive = CaseSensSearch;
		dlg.m_UseFileList = SearchFromList;
		dlg.m_SearchListFName = searchListFileName;
		// Invoke the dialog box
		if (dlg.DoModal() == IDOK) {
			DrawCursor(0);
			strcpy(SearchString, dlg.m_SearchString);
			for (ti=0; SearchString[ti] != EOS; ti++) {
				if (SearchString[ti] == 0x2022)
					SearchString[ti] = HIDEN_C;
			}
			SearchWrap = dlg.m_Wrap;
			CaseSensSearch = dlg.m_CaseSensitive;
			SearchFromList = dlg.m_UseFileList;
			if (!isThereSearchList())
				SearchFromList = FALSE;
			if (!CaseSensSearch)
				uS.uppercasestr(SearchString, &dFnt, C_MBF);
		} else {
			RemoveLastUndo();
			strcpy(global_df->err_message, DASHES);
			*sp = EOS;
			return(41);
		}
	}
#endif
	if (!FindString(SearchString, TRUE, SearchWrap, SearchFromList)) {
		RemoveLastUndo();
		global_df->LeaveHighliteOn = FALSE;
		strcpy(global_df->err_message, "-No match found.");
	}
	*sp = EOS;
	return(41);
}

static void finishFoundReverse(unCH *pat, unsigned int len, long pos) {
	LINE *tl;

	if (global_df->row_txt == global_df->cur_line) {
		tl = global_df->head_row->next_char;
		for (global_df->col_chr=0; tl != global_df->col_txt; tl=tl->next_char)
			global_df->col_chr++;
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		if (*pat == '\r' && *(pat+1) != EOS)
			global_df->col_chr2 = global_df->col_chr + len - 1;
		else
			global_df->col_chr2 = global_df->col_chr + len;
		global_df->col_win2 = ComColWin(FALSE, NULL, global_df->col_chr2);
	} else {
		global_df->col_chr = pos;
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
		if (*pat == '\r' && *(pat+1) != EOS)
			global_df->col_chr2 = global_df->col_chr + len - 1;
		else
			global_df->col_chr2 = global_df->col_chr + len;
		global_df->col_win2 = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr2);
	}
}

static int RevStringMatched(unCH *pat, int end, char isSearchFromList) {
	unsigned int i, len;
	unCH *s, *beg;
	struct uIEWords *t;

	if ((len=(unsigned int)strlen(pat)) == 0)
		return(0);
	if (global_df->row_txt == global_df->cur_line) {
		LINE *tl;
		if (end) {
			i = 0xffff;
			tl = global_df->tail_row->prev_char;
		} else {
			i = 0;
			tl = global_df->col_txt;
			if (tl != global_df->head_row) {
				i++;
				tl = tl->prev_char;
			}
			if (tl == global_df->head_row)
				return(0);
		}
		if (tl->c == HIDEN_C && tl != global_df->head_row && global_df->ShowParags != '\002') {
			for (tl=tl->prev_char; tl->c!=HIDEN_C && tl!=global_df->head_row; tl=tl->prev_char) ;
		}
		while (tl != global_df->head_row) {
			if (isSearchFromList && searchList != NULL) {
				for (t=searchList; t != NULL; t=t->nextword) {
					len = (unsigned int)strlen(t->word);
					if (len > i)
						continue;
					if (isMatchPat(t->word, NULL, tl, FALSE)) {
						finishFoundReverse(pat, len, 0L);
						return(1);
					}
				}
			} else {
				if (isMatchPat(pat, NULL, tl, FALSE)) {
					finishFoundReverse(pat, len, 0L);
					return(1);
				}
			}
			i++;
			tl = tl->prev_char;
			if (tl->c == HIDEN_C && tl != global_df->head_row && global_df->ShowParags != '\002') {
				for (tl=tl->prev_char; tl->c!=HIDEN_C && tl!=global_df->head_row; tl=tl->prev_char) ;
			}
		}
	} else {
		beg = global_df->row_txt->line;
		if (beg == NULL)
			return(0);
		if (end) {
			i = (unsigned int)strlen(global_df->row_txt->line);
			s = beg+i-1;
		} else {
			i = 0;
			s = global_df->row_txt->line + global_df->col_chr;
			if (s != beg) {
				i++;
				s--;
			}
			if (s == beg)
				return(0);
		}
		if (s == NULL)
			return(0);
		if (*s == HIDEN_C && s >= beg && global_df->ShowParags != '\002') {
			for (s--; *s != HIDEN_C && s >= beg; s--) ;
		}
		while (s >= beg) {
			if (isSearchFromList && searchList != NULL) {
				for (t=searchList; t != NULL; t=t->nextword) {
					len = (unsigned int)strlen(t->word);
					if (len > i)
						continue;
					if (isMatchPat(t->word, s, NULL, FALSE)) {
						finishFoundReverse(pat, len, (long)(s-beg));
						return(1);
					}
				}
			} else {
				if (isMatchPat(pat, s, NULL, FALSE)) {
					finishFoundReverse(pat, len, (long)(s-beg));
					return(1);
				}
			}
			i++;
			s--;
			if (*s == HIDEN_C && s >= beg && global_df->ShowParags != '\002') {
				for (s--; *s != HIDEN_C && s >= beg; s--) ;
			}
		}
	}
	return(0);
}

static int RevFindString(unCH *st) {
	long old_row_win = global_df->row_win;
	long old_lineno  = global_df->lineno;
	long old_wLineno = global_df->wLineno;
	ROWS *old_row_txt = global_df->row_txt, *tr;

	if (CMP_VIS_ID(global_df->row_txt->flag) && RevStringMatched(st,0,SearchFromList)) {
		global_df->lineno = countLines(global_df->row_txt);
		DisplayRow(TRUE);
		return(1);
	}
	global_df->row_txt = global_df->row_txt->prev_row;
	if (CMP_VIS_ID(global_df->row_txt->flag)) {
		if (isNL_CFound(global_df->row_txt))
			global_df->lineno--;
		global_df->wLineno--;
		if (global_df->row_win >= 0L)
			global_df->row_win--;
	}
	while (global_df->row_txt != global_df->head_text) {
		if (CMP_VIS_ID(global_df->row_txt->flag) && RevStringMatched(st,1,SearchFromList)) {
			global_df->lineno = countLines(global_df->row_txt);
			DisplayRow(TRUE);
			return(1);
		}
		global_df->row_txt = global_df->row_txt->prev_row;
		if (CMP_VIS_ID(global_df->row_txt->flag)) {
			if (isNL_CFound(global_df->row_txt))
				global_df->lineno--;
			global_df->wLineno--;
			if (global_df->row_win >= 0L)
				global_df->row_win--;
		}
    }
	if (SearchWrap && global_df->tail_text->prev_row != global_df->head_text) {
		global_df->row_txt = global_df->tail_text->prev_row;
		global_df->lineno  = 0L;
		global_df->wLineno = 0L;
		global_df->row_win = -1L;
		for (tr=global_df->head_text->next_row; tr != global_df->tail_text; tr=tr->next_row) {
			if (isNL_CFound(tr))
				global_df->lineno++;
			global_df->wLineno++;
		}
		while (global_df->row_txt != old_row_txt->prev_row) {
			if (CMP_VIS_ID(global_df->row_txt->flag) && RevStringMatched(st,1,SearchFromList)) {
				global_df->lineno = countLines(global_df->row_txt);
				DisplayRow(TRUE);
				return(1);
			}
			global_df->row_txt = global_df->row_txt->prev_row;
			if (CMP_VIS_ID(global_df->row_txt->flag)) {
				if (isNL_CFound(global_df->row_txt))
					global_df->lineno--;
				global_df->wLineno--;
				if (global_df->row_win >= 0L)
					global_df->row_win--;
			}
	    }
	}
	global_df->row_txt = old_row_txt;
	global_df->row_win = old_row_win;
	global_df->lineno  = old_lineno;
	global_df->wLineno = old_wLineno;
	return(0);
}

int SearchReverse(int i) {
	global_df->LeaveHighliteOn = TRUE;
#if defined(_MAC_CODE)
	DrawSoundCursor(0);
	if (i > 0) {
		SearchFFlag = FALSE;
		if (!FindDialog(SearchString, 255)) {
			RemoveLastUndo();
			strcpy(global_df->err_message, DASHES);
			*sp = EOS;
			return(51);
		}
	}
#else
	int				ti;
	CCedFindString dlg;

	if (global_df == NULL)
		return(51);
	if (i > 0) {

		DrawCursor(1);
		SearchFFlag = FALSE;
		strcpy(templineW, SearchString);
		for (ti=0; templineW[ti] != EOS; ti++) {
			if (templineW[ti] == HIDEN_C)
				templineW[ti] = 0x2022;
		}
		dlg.m_SearchString = templineW;
		dlg.m_SearchBackwards = !SearchFFlag;
		dlg.m_Wrap = SearchWrap;
		dlg.m_CaseSensitive = CaseSensSearch;
		dlg.m_UseFileList = SearchFromList;
		dlg.m_SearchListFName = searchListFileName;
		// Invoke the dialog box
		if (dlg.DoModal() == IDOK) {
			DrawCursor(0);
			strcpy(SearchString, dlg.m_SearchString);
			for (ti=0; SearchString[ti] != EOS; ti++) {
				if (SearchString[ti] == 0x2022)
					SearchString[ti] = HIDEN_C;
			}
			SearchWrap = dlg.m_Wrap;
			CaseSensSearch = dlg.m_CaseSensitive;
			SearchFromList = dlg.m_UseFileList;
			if (!isThereSearchList())
				SearchFromList = FALSE;
			if (!CaseSensSearch)
				uS.uppercasestr(SearchString, &dFnt, C_MBF);
		} else {
			RemoveLastUndo();
			strcpy(global_df->err_message, DASHES);
			*sp = EOS;
			return(51);
		}
	}
#endif
	if (!RevFindString(SearchString)) {
		RemoveLastUndo();
		global_df->LeaveHighliteOn = FALSE;
		strcpy(global_df->err_message, "-No match found.");
	}
	*sp = EOS;
	return(51);
}

int replaceAndFindNext(char isReplaceAndFind) {
	int i, SrchStrLen, ReplStrLen;

	if (!CaseSensSearch)
		uS.uppercasestr(SearchString, &dFnt, C_MBF);
	for (i=0; SearchString[i]; i++) {
		if (SearchString[i] == 0x2022)
			SearchString[i] = HIDEN_C;
		if ((SearchString[i] == CR_CHR) && SearchString[i+1] != EOS) {
			strcpy(global_df->err_message, "+Can't search for more than one line");
			*SearchString = EOS;
			return(0);
		}
	}
	if (isReplaceAndFind) {
		for (i=0; ReplaceString[i]; i++) {
			if (ReplaceString[i] == 0x2022)
				ReplaceString[i] = HIDEN_C;
			if (ReplaceString[i] == CR_CHR) {
				if (global_df->ChatMode) ReplaceString[i] = SNL_C;
				else ReplaceString[i] = NL_C;
			}
		}
	}
	DrawCursor(0);
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2;
	global_df->col_chr2 = -2L;
	if (isReplaceAndFind) {
		SrchStrLen = strlen(SearchString);
		ReplStrLen = strlen(ReplaceString);
		if (replaceExecPos != 0) {
			if (replaceOne(TRUE, SrchStrLen, ReplStrLen) == FALSE)
				return(0);
		}
	}
	if (FindString(SearchString, TRUE, FALSE, FALSE)) {
		if (replaceExecPos == 0)
			global_df->UndoList->key = LASTKEY;
		replaceExecPos = 1;
		global_df->WinChange = FALSE;
		draw_mid_wm();
		global_df->WinChange = TRUE;
		if (CheckLeftCol(global_df->col_win))
			DisplayTextWindow(NULL, 1);
		wmove(global_df->w1,global_df->row_win,global_df->col_win-global_df->LeftCol);
		global_df->LeaveHighliteOn = TRUE;
		global_df->DrawCur = TRUE;
		DrawCursor(1);
		return(1);
	} else {
		return(2);
	}
}

char replaceOne(char isDisplayText, int SrchStrLen, int ReplStrLen) {
	int  i;
	LINE *tl;
	ROWS *tr;

	ChangeCurLine();
//	TRUE_CHECK_ID1(global_df->row_txt->flag);
//	TRUE_CHECK_ID2(global_df->row_txt->flag);
	i = SrchStrLen - 1;
	if (SearchString[i] == CR_CHR &&
		(global_df->col_txt->prev_char->c == NL_C || global_df->col_txt == global_df->tail_row)) {
		if ((global_df->UndoList->str=(unCH *)malloc((SrchStrLen+2)*sizeof(unCH))) == NULL) 
			mem_err(TRUE, global_df);
		global_df->UndoList->str[0] = (char)ReplStrLen;
		global_df->UndoList->str[SrchStrLen+1] = EOS;
		if (global_df->col_txt->prev_char->c == NL_C) {
			global_df->col_txt = global_df->col_txt->prev_char;
			global_df->col_chr--;
		}
		if (global_df->cur_line->next_row != global_df->tail_text) {
			if (!CMP_VIS_ID(global_df->cur_line->next_row->flag)) {
				strcpy(global_df->err_message, "+Can't delete next non-selected tier.");
				return(FALSE);
			}
			global_df->UndoList->str[SrchStrLen] = SNL_C;
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
		}
		i--;
	} else if (global_df->col_txt->prev_char != global_df->head_row && 
			   global_df->col_txt->prev_char->c == HIDEN_C && global_df->ShowParags != '\002') {
		tl = global_df->col_txt->prev_char->prev_char;
		for (i=SrchStrLen; tl->c!=HIDEN_C && tl!=global_df->head_row; tl=tl->prev_char,i++) ;
		if ((global_df->UndoList->str=(unCH *)malloc((i+3)*sizeof(unCH))) == NULL) 
			mem_err(TRUE, global_df);
		global_df->UndoList->str[0] = (char)ReplStrLen;
		global_df->UndoList->str[i+2] = EOS;
	} else {
		if ((global_df->UndoList->str=(unCH *)malloc((SrchStrLen+2)*sizeof(unCH))) == NULL) 
			mem_err(TRUE, global_df);
		global_df->UndoList->str[0] = (char)ReplStrLen;
		global_df->UndoList->str[SrchStrLen+1] = EOS;
	}
	if (global_df->col_txt->prev_char != global_df->head_row && global_df->col_txt->prev_char->c == NL_C) {
		global_df->col_txt = global_df->col_txt->prev_char;
		global_df->col_chr--;
	}
	for (; i >= 0 && global_df->col_txt->prev_char != global_df->head_row; i--) {
		tl = global_df->col_txt->prev_char;
		global_df->col_txt->prev_char = tl->prev_char;
		tl->prev_char->next_char = global_df->col_txt;
		global_df->UndoList->str[i+1] = tl->c;
		free(tl);
		global_df->head_row_len--;
		global_df->col_chr--;
	}
	global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
	if (i >= 0)
		strcpy(global_df->UndoList->str+1, global_df->UndoList->str+i+2);
	if (ReplStrLen > 0) {
		global_df->col_txt = global_df->col_txt->prev_char;
		AddString(ReplaceString, (long)ReplStrLen, TRUE);
		global_df->col_txt = global_df->col_txt->next_char;
	} else {
		global_df->row_win2 = 0L;
		global_df->col_win2 = -2;
		global_df->col_chr2 = -2L;
	}
/* 07-09-00
	 if (global_df->ChatMode || global_df->AutoWrap) {
		 if (isCallFixLine(0L, global_df->head_row_len, TRUE))
			FixLine(TRUE);
	}
*/
	if (isDisplayText)
		DisplayTextWindow(NULL, 1);
	SaveUndoState(FALSE);
	if (isDisplayText)
		global_df->UndoList->key = LASTKEY;
	else
		global_df->UndoList->key = REPLKEY;
	return(TRUE);
}

static int doReplace(char isOneTime) {
	int  i, SrchStrLen, ReplStrLen, res;
	char rightFind = TRUE;

	SrchStrLen = strlen(SearchString);
	ReplStrLen = strlen(ReplaceString);
	if (global_df->row_win2 == 0L && global_df->col_win2 != -2) {
		if (isOneTime) {
			if (global_df->col_chr2 > global_df->col_chr)
				i = global_df->col_chr2 - global_df->col_chr;
			else
				i = global_df->col_chr - global_df->col_chr2;
			rightFind = (i == SrchStrLen);
		}
		global_df->col_win = global_df->col_win2;
		global_df->col_chr = global_df->col_chr2;
		if (global_df->row_txt == global_df->cur_line) {
			for (global_df->col_txt=global_df->head_row->next_char, i=global_df->col_chr; i > 0L; i--)
				global_df->col_txt = global_df->col_txt->next_char;
		}
	} else if (isOneTime)
		rightFind = FALSE;

	global_df->row_win2 = 0L;
	global_df->col_win2 = -2;
	global_df->col_chr2 = -2L;
	if ((res=FindString(SearchString, TRUE, FALSE, FALSE))) {
		global_df->UndoList->key = LASTKEY;
		if (!rightFind) {
			global_df->UndoList->key = MOVEKEY;
			SaveUndoState(FALSE);
		} else {
			do {
				if (CheckLeftCol(global_df->col_win))
					DisplayTextWindow(NULL, 1);
				if (replaceOne(FALSE, SrchStrLen, ReplStrLen) == FALSE)
					return(0);
				if (isOneTime) {
					res = FindString(SearchString, TRUE, FALSE, FALSE);
					if (res) {
						global_df->UndoList->key = MOVEKEY;
						SaveUndoState(FALSE);
					}
					break;
				}
			} while ((res=FindString(SearchString, FALSE, FALSE, FALSE))) ;
		}
		DisplayRow(TRUE);
		draw_mid_wm();
#if defined(_MAC_CODE)
		SetScrollControl();
#endif
		wmove(global_df->w1,global_df->row_win,global_df->col_win-global_df->LeftCol);
		RemoveLastUndo();
	} else
		RemoveLastUndo();
	return(res);
}

int Replace(int i) {
	int  ti;
	char res;
	unCH *s;
	FILE *fp;

	replaceExecPos = 0;
#if defined(_MAC_CODE)
	DrawSoundCursor(0);
	if (ReplaceDialog(SearchString, ReplaceString, replaceListFileName)) {
		DrawCursor(0);
		for (ti=0; SearchString[ti] != EOS; ti++) {
			if (SearchString[ti] == 0x2022)
				SearchString[ti] = HIDEN_C;
		}
		for (ti=0; ReplaceString[ti] != EOS; ti++) {
			if (ReplaceString[ti] == 0x2022)
				ReplaceString[ti] = HIDEN_C;
		}
		if (!CaseSensSearch)
			uS.uppercasestr(SearchString, &dFnt, C_MBF);
	} else {
		DrawCursor(0);
		RemoveLastUndo();
		if (replaceExecPos)
			global_df->LeaveHighliteOn = FALSE;
		return(7);
	}
#else
	CReplaceString dlgReplace;
	CReplaceCont   dlgCont;

	if (global_df == NULL)
		return(7);
	strcpy(global_df->err_message,DASHES);
	global_df->DrawCur = TRUE;
	if (global_df->row_win2 || global_df->col_win2 != -2)
		DrawCursor(1);
	strcpy(templineW, SearchString);
	for (ti=0; templineW[ti] != EOS; ti++) {
		if (templineW[ti] == HIDEN_C)
			templineW[ti] = 0x2022;
	}
	dlgReplace.m_ReplaceString = templineW;
	strcpy(templineW1, ReplaceString);
	for (ti=0; templineW1[ti] != EOS; ti++) {
		if (templineW1[ti] == HIDEN_C)
			templineW1[ti] = 0x2022;
	}
	dlgReplace.m_ReplaceWith = templineW1;
	dlgReplace.m_CaseSensitive = CaseSensSearch;
	dlgReplace.m_ReplaceListFName = replaceListFileName;
	if (dlgReplace.DoModal() == IDOK) {
		DrawCursor(0);
		strcpy(SearchString, dlgReplace.m_ReplaceString);
		for (ti=0; SearchString[ti] != EOS; ti++) {
			if (SearchString[ti] == 0x2022)
				SearchString[ti] = HIDEN_C;
		}
		strcpy(ReplaceString, dlgReplace.m_ReplaceWith);
		for (ti=0; ReplaceString[ti] != EOS; ti++) {
			if (ReplaceString[ti] == 0x2022)
				ReplaceString[ti] = HIDEN_C;
		}
		u_strcpy(replaceListFileName, dlgReplace.m_ReplaceListFName, FNSize);
		CaseSensSearch = dlgReplace.m_CaseSensitive;
		if (!CaseSensSearch)
			uS.uppercasestr(SearchString, &dFnt, C_MBF);
	} else {
		DrawCursor(0);
		RemoveLastUndo();
		if (replaceExecPos)
			global_df->LeaveHighliteOn = FALSE;
		return(7);
	}
#endif
	fp = NULL;
	if (ReplaceFromList) {
		if ((fp=fopen(replaceListFileName,"r")) == NULL) {
			sprintf(global_df->err_message, "+Can't open file %s.", replaceListFileName);
			return(7);
		}
	}
	do {
		if (fp != NULL) {
repeatR:
			if (fgets_cr(templineC, SPEAKERLEN, fp) == NULL) {
				*SearchString  = EOS;
				*ReplaceString = EOS;
				break;
			}
			if (templineC[0] == '#' && isSpace(templineC[1]))
				goto repeatR;
			if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
				goto repeatR;
			if (!strcmp(templineC,"\n"))
				goto repeatR;
			UTF8ToUnicode((unsigned char *)templineC, strlen(templineC), SearchString, NULL, SPEAKERLEN);
			for (i=0; SearchString[i];) {
				if (SearchString[i] == '\\') {
					if (SearchString[i+1] == 'r' || SearchString[i+1] == 'n')
						SearchString[i+1] = CR_CHR;
					else if (SearchString[i+1] == 't')
						SearchString[i+1] = '\t';
					strcpy(SearchString+i, SearchString+i+1);
				} else
					i++;
			}	
			if (i > 0 && SearchString[i-1] == '\n')
				SearchString[i-1] = EOS;
			if (!CaseSensSearch)
				uS.uppercasestr(SearchString, &dFnt, C_MBF);

			if (fgets_cr(templineC, SPEAKERLEN, fp) == NULL) {
				sprintf(global_df->err_message, "+Missing replace part of search-replace pair in file %s", replaceListFileName);
				*SearchString = EOS;
				fclose(fp);
				return(7);
			}
			UTF8ToUnicode((unsigned char *)templineC, strlen(templineC), ReplaceString, NULL, SPEAKERLEN);
			for (i=0; ReplaceString[i];) {
				if (ReplaceString[i] == '\\') {
					if (ReplaceString[i+1] == 'r' || ReplaceString[i+1] == 'n')
						ReplaceString[i+1] = CR_CHR;
					else if (ReplaceString[i+1] == 't')
						ReplaceString[i+1] = '\t';
					strcpy(ReplaceString+i, ReplaceString+i+1);
				} else
					i++;
			}	
			if (i > 0 && ReplaceString[i-1] == '\n')
				ReplaceString[i-1] = EOS;
			SaveUndoState(FALSE);
			BeginningOfFile(-1);
			SaveUndoState(FALSE);
#if defined(_MAC_CODE)
			res = ContReplaceDialog(SearchString, ReplaceString);
			if (res == 2) {
				if (!CaseSensSearch)
					uS.uppercasestr(SearchString, &dFnt, C_MBF);
			} else if (res == 0) {
				DrawCursor(0);
				RemoveLastUndo();
				if (replaceExecPos)
					global_df->LeaveHighliteOn = FALSE;
			}
#else
			if (global_df->row_win2 || global_df->col_win2 != -2)
				DrawCursor(1);
			dlgCont.m_ReplaceString = SearchString;
			dlgCont.m_ReplaceWith = ReplaceString;
			if (dlgCont.DoModal() == IDOK) {
				DrawCursor(0);
				res = dlgCont.result;
				if (res != 3) {
					strcpy(SearchString, dlgCont.m_ReplaceString);
					strcpy(ReplaceString, dlgCont.m_ReplaceWith);
					if (!CaseSensSearch)
						uS.uppercasestr(SearchString, &dFnt, C_MBF);
				}
			} else {
				DrawCursor(0);
				RemoveLastUndo();
				if (replaceExecPos)
					global_df->LeaveHighliteOn = FALSE;
				res = 0;
			}
#endif
			if (res == 0)
				break;
			else if (res == 1)
				continue;
			if (res == 3)
				continue;
		} else {
			SaveUndoState(FALSE);
			BeginningOfFile(-1);
			SaveUndoState(FALSE);
		}

		for (i=0; SearchString[i]; i++) {
			if ((SearchString[i] == CR_CHR) && SearchString[i+1] != EOS) {
				strcpy(global_df->err_message, "+Can't search for more than one line");
				*SearchString = EOS;
				if (fp != NULL)
					fclose(fp);
				return(7);
			}
		}
		for (s=ReplaceString; *s; s++) {
			if (*s == CR_CHR) {
				if (global_df->ChatMode)
					*s = SNL_C;
				else
					*s = NL_C;
			}
		}	
		if (doReplace(FALSE))
			global_df->LeaveHighliteOn = TRUE;
		else
			global_df->LeaveHighliteOn = FALSE;
	} while (ReplaceFromList) ;
	strcpy(global_df->err_message, DASHES);
	if (fp != NULL)
		fclose(fp);
	return(7);
}
