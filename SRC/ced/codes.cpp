#include "ced.h"
#include "my_ctype.h"
#include "MMedia.h"
#ifdef _WIN32 
#include "CedDlgs.h"
#endif

#define WO 200 /* maximum size of the code in a code file */

static CODES *FillInCodesString(long , long (*GetString)(long, unCH *), int , int *, int *);

/* The elements of RootCodes tree point to the memory allocated for			*/
/* coresponding code within this list. It is used to save space, i.e. space,*/
/* for each code string is allocated only once.								*/
#define CODESLIST struct code_slist
CODESLIST
{
	unCH *cod;
	CODESLIST *next_code;
} *RootCodeList;

char isLastCode;
char DisTier[51];

int   MaxNumOfCodes;

#if defined(_MAC_CODE) || defined(_WIN32)
char UpdateCodesCursorPos(int c_row, int c_col) {
	int index = 0, s_index, row, col, len, max;

	for (max=0,col=0; col < global_df->num_of_cols; col += max+1,max=0) {
		s_index = index;
		for (row=0; row < COM_WIN_SIZE; row++, index++) {
			if (global_df->StartCodeArr[index] == NULL)
				break;
			if ((len=strlen(global_df->StartCodeArr[index]->code)) > max)
				max = len;
			if (max+col+1 > global_df->num_of_cols)
				break;
		}
		for (row=0; s_index < index ; row++, s_index++) {
			if (row == c_row && c_col >= col && c_col <= max+col) {
				if (s_index != global_df->CursorCodeArr) {
#if defined(_MAC_CODE) || defined(_WIN32)
					global_df->WinChange = FALSE;
#endif
					SaveUndoState(FALSE);
					global_df->redisplay = 0; global_df->EditorMode = !global_df->EditorMode;
					DisplayCursor(COM_WIN_SIZE);
					global_df->redisplay = 1; global_df->EditorMode = !global_df->EditorMode;
					global_df->CursorCodeArr = s_index;
					DisplayCursor(COM_WIN_SIZE);
					if (global_df->CurCode==global_df->RootCodes) 
						strcpy(global_df->OldCode,global_df->StartCodeArr[global_df->CursorCodeArr]->code);
#if defined(_MAC_CODE) || defined(_WIN32)
					global_df->WinChange = TRUE;
#endif
				}
				return(TRUE);
			}
		}
		if (global_df->StartCodeArr[index] == NULL)
			return(FALSE);
		if (max+col+1 > global_df->num_of_cols)
			return(FALSE);
	}
	return(FALSE);
}
#endif /* defined(_MAC_CODE) || defined(_WIN32) */

static void FreeCL(void) {
	CODESLIST *t;

	while (RootCodeList != NULL) {
		t = RootCodeList;
		RootCodeList = RootCodeList->next_code;
		free(t->cod);
		free(t);
	}
}

static void FreeRC(void) {
	CODES *trcn, *trc;

	while (global_df->RootCodes != NULL) {
		trcn = global_df->RootCodes->NextCode;
		while (trcn != NULL) {
			trc = trcn;
			trcn = trcn->NextCode;
			free(trc);
		}
		trc = global_df->RootCodes;
		global_df->RootCodes = global_df->RootCodes->subcodes;
		free(trc);
	}
}

void FreeCodesMem(void) {
	if (global_df->RootCodesArr)
		free(global_df->RootCodesArr);
	FreeCL();
	FreeRC();
	global_df->RootCodes = NULL;
	global_df->RootCodesArr = NULL;
	RootCodeList = NULL;
}

static char isUNDCode(unCH *code) {
	if (strncmp(code, "?|", 2) == 0) {
		strcpy(templine3, code);
#if defined(_MAC_CODE)
		DoString(templine3);
#endif // _MAC_CODE
#if defined(_WIN32)
		CDoString dlg;
	
		DrawCursor(1);
		dlg.m_StringVal = code;
		if (dlg.DoModal() == IDOK) {
			strcpy(templine3, dlg.m_StringVal);
		}
		DrawCursor(0);
#endif /* _WIN32 */
		AddCodeTier(templine3, TRUE);
		return(TRUE);
	}
	return(FALSE);
}

int GetCursorCode(int i) {
	int  cd = -1;
	char isSpaceFound = FALSE;
	unCH *s;
   

	if (global_df->EditorMode && i != -1) {
		IllegalFunction(i);
		*sp = EOS;
		return(39);
	}

	if (global_df->CurCode != global_df->RootCodes || !global_df->ChatMode || global_df->cod_fname == NULL) {
		if (DeleteChank(1)) {
			if (global_df->row_txt == global_df->cur_line) {
				isSpaceFound = isSpace(global_df->col_txt->c);
			} else {
				isSpaceFound = isSpace(global_df->row_txt->line[global_df->col_chr]);
			}
			if (!isSpaceFound) {
				SaveUndoState(FALSE);
				AddText(NULL, ' ', 0, 1L);
				SaveUndoState(FALSE);
				MoveLeft(1);
				SaveUndoState(FALSE);
				s = global_df->StartCodeArr[global_df->CursorCodeArr]->code;
				if (*s == ' ') {
					if (!isUNDCode(s+1))
						AddCodeTier(s+1, TRUE);
				} else {
					if (!isUNDCode(s))
		   				AddCodeTier(s, TRUE);
		   		}
			} else {
				SaveUndoState(FALSE);
				if (!isUNDCode(global_df->StartCodeArr[global_df->CursorCodeArr]->code))
					AddCodeTier(global_df->StartCodeArr[global_df->CursorCodeArr]->code, TRUE);
			}
			if (global_df->cod_fname == NULL) {
				isPlayS = 69;
				InKey = 1;
			}
		}
	} else {
		GetCurCode();
		if (!uS.partcmp(global_df->StartCodeArr[global_df->CursorCodeArr]->code, sp, FALSE, TRUE) || *sp == EOS) {
			strcpy(templine1, global_df->StartCodeArr[global_df->CursorCodeArr]->code);
			uS.remFrontAndBackBlanks(templine1);
			if (*sp == '@') {
				global_df->redisplay = 0;
				while (1) {
					MoveDown(1);
					EndOfLine(-1);
					if (global_df->row_txt == global_df->cur_line) {
						if (isMainSpeaker(global_df->head_row->next_char->c)) { 
							break;
						}
					} else {
						if (isMainSpeaker(*global_df->row_txt->line)) { 
							break;
						}
					}
					if (FoundEndHTier(global_df->row_txt, TRUE) || AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
						strcpy(global_df->err_message, "-Can't find any speaker tiers.");
						*sp = EOS;
						return(39);
					}
				}
				global_df->redisplay = 1;
				DisplayTextWindow(NULL, 1);
			}
			if (!FindTextCodeLine(templine1, NULL)) {
				EndOfLine(-1);
				SaveUndoState(FALSE);
				AddCodeTier(global_df->StartCodeArr[global_df->CursorCodeArr]->code, TRUE);
				cd = 0;
			} else {
				global_df->EnteredCode = '\001';
				u_strcpy(templineC1, templine1, UTTLINELEN);
				sprintf(global_df->err_message, "-Please put text cursor at desired place on %s tier.", templineC1);
			}
		} else {
			PositionCursor();
			MoveToSubcode();
		}
	}
	*sp = EOS;
	return(39);
}

int MoveCodeCursorUp(int i) {
	if (global_df->EditorMode) {
		SelfInsert(i);
		return(42);
	}
	global_df->redisplay = 0;
	global_df->EditorMode = !global_df->EditorMode;
	DisplayCursor(COM_WIN_SIZE);
	global_df->redisplay = 1;
	global_df->EditorMode = !global_df->EditorMode;
	if (global_df->StartCodeArr+global_df->CursorCodeArr == global_df->RootCodesArr + 1) {
		for (; *global_df->StartCodeArr != NULL; global_df->StartCodeArr++) ;
		FindLastPage(COM_WIN_SIZE);
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	} else if (global_df->CursorCodeArr == 0) {
		FindLastPage(COM_WIN_SIZE);
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	} else {
		global_df->CursorCodeArr--;
		DisplayCursor(COM_WIN_SIZE);
	}
	if (global_df->CurCode == global_df->RootCodes)
		strcpy(global_df->OldCode,global_df->StartCodeArr[global_df->CursorCodeArr]->code);
	*sp = EOS;
	return(42);
}

int MoveCodeCursorDown(int i) {
	if (global_df->EditorMode) {
		SelfInsert(i);
		return(43);
	}
	global_df->redisplay = 0;
	global_df->EditorMode = !global_df->EditorMode;
	DisplayCursor(COM_WIN_SIZE);
	global_df->redisplay = 1;
	global_df->EditorMode = !global_df->EditorMode;
	global_df->CursorCodeArr++;
	if (global_df->StartCodeArr[global_df->CursorCodeArr] == global_df->EndCodeArr[0]) {
		if (global_df->EndCodeArr[0] == NULL)
			global_df->StartCodeArr = global_df->RootCodesArr + 1;
		else
			global_df->StartCodeArr = global_df->EndCodeArr;
		global_df->CursorCodeArr = 0;
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	} else
		DisplayCursor(COM_WIN_SIZE);
	if (global_df->CurCode == global_df->RootCodes)
		strcpy(global_df->OldCode,global_df->StartCodeArr[global_df->CursorCodeArr]->code);
	*sp = EOS;
	return(43);
}

int MoveCodeCursorLeft(int i) {
	int j, wsize;

	if (global_df->EditorMode) {
		SelfInsert(i);
		return(43);
	}
	wsize = COM_WIN_SIZE;
	global_df->redisplay = 0;
	global_df->EditorMode = !global_df->EditorMode;
	DisplayCursor(COM_WIN_SIZE);
	global_df->redisplay = 1;
	global_df->EditorMode = !global_df->EditorMode;
	if (global_df->CursorCodeArr < wsize) {
		i = global_df->CursorCodeArr;
		j = 0;
		do {
			i = i + wsize;
			for (; global_df->StartCodeArr[j] != global_df->EndCodeArr[0] && j < i; j++) ;
			if (global_df->StartCodeArr[j] != global_df->EndCodeArr[0])
				global_df->CursorCodeArr = j;
			if (global_df->StartCodeArr[j] == global_df->EndCodeArr[0])
				break;
		} while (1) ;
	} else {
		global_df->CursorCodeArr = global_df->CursorCodeArr - wsize;
	}
	if (global_df->StartCodeArr[global_df->CursorCodeArr] == global_df->EndCodeArr[0]) {
		if (global_df->EndCodeArr[0] == NULL)
			global_df->StartCodeArr = global_df->RootCodesArr + 1;
		else
			global_df->StartCodeArr = global_df->EndCodeArr;
		global_df->CursorCodeArr = 0;
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	} else
		DisplayCursor(COM_WIN_SIZE);
	if (global_df->CurCode == global_df->RootCodes)
		strcpy(global_df->OldCode,global_df->StartCodeArr[global_df->CursorCodeArr]->code);
	*sp = EOS;
	return(70);
}

int MoveCodeCursorRight(int i) {
	int wsize;

	if (global_df->EditorMode) {
		SelfInsert(i);
		return(43);
	}
	wsize = COM_WIN_SIZE;
	global_df->redisplay = 0;
	global_df->EditorMode = !global_df->EditorMode;
	DisplayCursor(COM_WIN_SIZE);
	global_df->redisplay = 1;
	global_df->EditorMode = !global_df->EditorMode;
	global_df->CursorCodeArr = global_df->CursorCodeArr + wsize;
	for (i=0; global_df->StartCodeArr[i] != global_df->EndCodeArr[0] && i < global_df->CursorCodeArr; i++) ;
	if (i < global_df->CursorCodeArr || global_df->StartCodeArr[i] == global_df->EndCodeArr[0]) {
		i = global_df->CursorCodeArr;
		global_df->CursorCodeArr = i - ((i / wsize) * wsize);
	}
	if (global_df->StartCodeArr[global_df->CursorCodeArr] == global_df->EndCodeArr[0]) {
		if (global_df->EndCodeArr[0] == NULL)
			global_df->StartCodeArr = global_df->RootCodesArr + 1;
		else
			global_df->StartCodeArr = global_df->EndCodeArr;
		global_df->CursorCodeArr = 0;
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	} else
		DisplayCursor(COM_WIN_SIZE);
	if (global_df->CurCode == global_df->RootCodes)
		strcpy(global_df->OldCode,global_df->StartCodeArr[global_df->CursorCodeArr]->code);
	*sp = EOS;
	return(77);
}

int GetFakeCursorCode(int i) {
	if (global_df->EditorMode && i != -1) {
		IllegalFunction(i);
		*sp = EOS;
		return(40);
	}
	if (global_df->CurCode == global_df->RootCodes) {
		GetCurCode();
		if (!FindRightCode(0)) return(40);
		PositionCursor();
	}
	if (global_df->StartCodeArr[global_df->CursorCodeArr]->subcodes != NULL) {
		if (global_df->CurCode == global_df->RootCodes) {
			global_df->CurFirstCodeOfPrevLevel = global_df->RootCodesArr[0]->subcodes;
			global_df->CurCode = global_df->StartCodeArr[global_df->CursorCodeArr];
		}
		global_df->FirstCodeOfPrevLevel = global_df->RootCodesArr[0]->subcodes;
		MapArrToList(global_df->StartCodeArr[global_df->CursorCodeArr]);
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	} else if (global_df->RootCodesArr[0] != global_df->CurCode) {
		global_df->FirstCodeOfPrevLevel = global_df->CurFirstCodeOfPrevLevel;
		MapArrToList(global_df->CurCode);
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	}
	*sp = EOS;
	return(40);
}

static unCH *AddCodeToList(unCH *s, int *index) {
	short cnt = 2;
	CODESLIST *nc;

	if (RootCodeList == NULL) {
		nc = NEW(CODESLIST);
		if (nc == NULL)
			mem_err(TRUE, global_df);
		RootCodeList = nc;
	} else {
		for (nc=RootCodeList; 1; nc=nc->next_code) {
			if (strcmp(nc->cod,s) == 0)
				return(nc->cod);
			cnt++;
			if (nc->next_code == NULL) {
				nc->next_code = NEW(CODESLIST);
				if (nc->next_code == NULL)
					mem_err(TRUE, global_df);
				nc = nc->next_code;
				break;
			}
		}
	}
	nc->cod = (unCH *)malloc((strlen(s)+1)*sizeof(unCH));
	if (nc->cod == NULL)
		mem_err(TRUE, global_df);
	strcpy(nc->cod,s);
	nc->next_code = NULL;
	return(nc->cod);
}

static int init_codes_string(long (*GetString)(long, unCH *)) {
	int end, index;
	long s_index;

	RootCodeList = NULL;
	if ((global_df->RootCodes=NEW(CODES)) == NULL) {
		strcpy(global_df->err_message, "+Out of memory.");
		global_df->NoCodes = TRUE;
		return(0);
	}
	global_df->RootCodes->code = NULL;
	global_df->RootCodes->NextCode = NULL;
	s_index = (*GetString)(-1, ced_line);
	
	index = WO;
	if (s_index == -1) {
		strcpy(ced_line,"    ");
		global_df->NoCodes = TRUE;
		MaxNumOfCodes = 1;
		global_df->RootCodes->subcodes = NEW(CODES);
		if (global_df->RootCodes->subcodes == NULL) {
			strcpy(global_df->err_message, "+Out of memory.");
			global_df->NoCodes = TRUE;
			return(0);
		}
		global_df->RootCodes->subcodes->code = AddCodeToList(ced_line,&index);
		global_df->RootCodes->subcodes->subcodes = NULL;
		global_df->RootCodes->subcodes->NextCode = NULL;
		MaxNumOfCodes++;
		global_df->CurFirstCodeOfPrevLevel = global_df->RootCodes;
		global_df->CurCode = global_df->RootCodes;
		global_df->EditorMode = TRUE;
	} else {
		global_df->NoCodes = FALSE;
		MaxNumOfCodes = 0;
		end = FALSE;
		global_df->RootCodes->subcodes = FillInCodesString(s_index,GetString,0,&end,&index);
		if (global_df->RootCodes->subcodes == NULL) global_df->NoCodes = TRUE;
		MaxNumOfCodes++;
		global_df->CurFirstCodeOfPrevLevel = global_df->RootCodes;
		global_df->CurCode = global_df->RootCodes;
	}
	MaxNumOfCodes++;
	global_df->RootCodesArr = (CODES **)malloc(sizeof(CODES *) * MaxNumOfCodes);
	if (global_df->RootCodesArr == NULL) {
		strcpy(global_df->err_message, "+Out of memory.");
		global_df->NoCodes = TRUE;
		return(0);
	}
	global_df->FirstCodeOfPrevLevel = global_df->RootCodes;
	MapArrToList(global_df->RootCodes);
	strcpy(global_df->OldCode,global_df->StartCodeArr[global_df->CursorCodeArr]->code);
	return(1);
}

static void HiLightMatch(void) {
	long t_col_chr;
	
	global_df->row_win2 = 0L;
	t_col_chr = global_df->col_chr;
	if (global_df->row_txt == global_df->cur_line) {
		LINE *tc;
		tc = global_df->col_txt;
		while (tc->prev_char != global_df->head_row) {
			if (isSpace(tc->c))
				break;
			tc = tc->prev_char;
			t_col_chr--;
		}
		global_df->col_chr2 = t_col_chr + 1;
		global_df->col_win2 = ComColWin(FALSE, NULL, global_df->col_chr2);
		while (global_df->col_txt != global_df->tail_row) {
			if (isSpace(global_df->col_txt->c))
				break;
			global_df->col_txt = global_df->col_txt->next_char;
			global_df->col_chr++;
		}
		global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
	} else {
		while (t_col_chr >= 0) {
			if (isSpace(global_df->row_txt->line[t_col_chr]))
				break;
			t_col_chr--;
		}
		global_df->col_chr2 = t_col_chr + 1;
		global_df->col_win2 = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr2);
		while (global_df->row_txt->line[global_df->col_chr] != EOS) {
			if (isSpace(global_df->row_txt->line[global_df->col_chr]))
				break;
			global_df->col_chr++;
		}
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
	}
	global_df->LeaveHighliteOn = TRUE;
}

static CODES *FillInCodesString(long s_index, long (*GetString)(long, unCH *), int LastLevel, int *end, int *index) {
	int ThisLevel, offset, LocMaxNumOfCodes = 1;
	CODES *RootCode, *CurCode, *LastSubcode;

	LastSubcode = NULL;
	ThisLevel = LastLevel;
	RootCode = NEW(CODES);
	if (RootCode == NULL) {
		strcpy(global_df->err_message, "+Out of memory.");
		return(NULL);
	}
	if (ced_line[ThisLevel] == '"')
		offset = 1;
	else
		offset = 0;
	RootCode->code = AddCodeToList(ced_line+ThisLevel+offset,index);
	RootCode->subcodes = NULL;
	RootCode->NextCode = NULL;
	CurCode = RootCode;
	while ((s_index=(*GetString)(s_index,ced_line)) != -1) {
		do {
			for (offset=0; isSpace(ced_line[offset]) || ced_line[offset] == '\n'; offset++) ;
			if (ced_line[offset] != EOS)
				break;
			if ((s_index=(*GetString)(s_index,ced_line)) == -1)
				goto contfillstr;
		} while (1) ;
		offset = strlen(ced_line);
		if (offset >= WO) {
			strcpy(global_df->err_message, "+One of the codes is too long.");
			return(NULL);
		}
		for (ThisLevel=0; isSpace(ced_line[ThisLevel]); ThisLevel++) ;
		if (ThisLevel > LastLevel) {
			if (ThisLevel > LastLevel+1) {
				strcpy(global_df->err_message,
					   "+More than one space indentation between two codes");
				return(NULL);
			}
			CurCode->subcodes = FillInCodesString(s_index,GetString,ThisLevel,end,index);
			if (CurCode->subcodes == NULL)
				return(NULL);
			LastSubcode = CurCode->subcodes;
			if (*end)
				break;
			for (ThisLevel=0; isSpace(ced_line[ThisLevel]); ThisLevel++) ;
		} else if (*end)
			break;

		if (ThisLevel < LastLevel) {
			if (LocMaxNumOfCodes>MaxNumOfCodes)
				MaxNumOfCodes=LocMaxNumOfCodes;
			return(RootCode);
		}
		LocMaxNumOfCodes++;
		CurCode->NextCode = NEW(CODES);
		if (CurCode->NextCode == NULL) {
			strcpy(global_df->err_message, "+Out of memory.");
			return(NULL);
		}
		CurCode = CurCode->NextCode;
		if (ced_line[ThisLevel] == '"')
			offset = 1;
		else
			offset = 0;
		CurCode->code = AddCodeToList(ced_line+ThisLevel+offset,index);
		if (global_df->CodingScheme == '\001')
			CurCode->subcodes = LastSubcode;
		else
			CurCode->subcodes = NULL;
		CurCode->NextCode = NULL;
	}
contfillstr:
	*end = TRUE;
	if (LocMaxNumOfCodes > MaxNumOfCodes)
		MaxNumOfCodes = LocMaxNumOfCodes;
	return(RootCode);
}

static long GetNextPostmortemString(long index, unCH *st) {
	int i = 0, j;

	if (isLastCode)
		return(-1);

	if (index == -1) {
		index = global_df->col_chr;
		while (index >= 0) {
			if (isSpace(global_df->row_txt->line[index]))
				break;
			index--;
		}
		index++;
	}
	st[i] = EOS;
	if (global_df->row_txt->line[index] == EOS || isSpace(global_df->row_txt->line[index])) {
		isLastCode = TRUE;
		return(index);
	}
	if (global_df->row_txt->line[index] == '^')
		index++;
	while (global_df->row_txt->line[index] != EOS) {
		if (global_df->row_txt->line[index] == '^' || isSpace(global_df->row_txt->line[index]))
			break;
		st[i++] = global_df->row_txt->line[index];
		index++;
	}
	if (st[i-1] == NL_C)
		i--;
	st[i] = EOS;
	if (*templine3 == EOS) {
		for (i=0; st[i] && st[i] != '|'; i++) ;
		if (st[i])
			i++;
		for (j=0; st[i] && is_unCH_alpha(st[i]); i++, j++) {
			templine3[j] = st[i];
		}
		templine3[j] = EOS;
	} 
	return(index);
}

static long GetNextMorString(long index, unCH *st) {
	int i = 0, j;

	if (isLastCode)
		return(-1);

	if (index == -1) {
		index = global_df->col_chr;
		while (index >= 0) {
			if (isSpace(global_df->row_txt->line[index]))
				break;
			index--;
		}
		index++;
	}
	st[i] = EOS;
	if (global_df->row_txt->line[index] == EOS || isSpace(global_df->row_txt->line[index])) {
		isLastCode = TRUE;
		strcpy(st, "?|");
		if (*templine3 != EOS)
			strcat(st, templine3);
		return(index);
	}
	if (global_df->row_txt->line[index] == '^')
		index++;
	while (global_df->row_txt->line[index] != EOS) {
		if (global_df->row_txt->line[index] == '^' || isSpace(global_df->row_txt->line[index]))
			break;
		st[i++] = global_df->row_txt->line[index];
		index++;
	}
	if (st[i-1] == NL_C)
		i--;
	st[i] = EOS;
	if (*templine3 == EOS) {
		for (i=0; st[i] && st[i] != '|'; i++) ;
		if (st[i])
			i++;
		for (j=0; st[i] && is_unCH_alpha(st[i]); i++, j++) {
			templine3[j] = st[i];
		}
		templine3[j] = EOS;
	} 
	return(index);
}

static void getFileInfo(ROWS *row_txt) {
}

static void FindNextGivenTier(char *st) {
	LINE *tc;
	unCH *l;
	char isAddLineno;
	const char *p;

	global_df->redisplay = 0;
	while (1) {
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->head_row->next_char->c == '*') { 
				tc = global_df->head_row->next_char->next_char;
				p = "** File \"";
				for (; tc!= global_df->tail_row && *p != EOS; tc=tc->next_char, p++) {
					if (tc->c != *p)
						break;
				}
				if (*p == EOS) {
					getFileInfo(global_df->row_txt);
				}
			}
			if (global_df->head_row->next_char->c == st[0]) { 
				if (st[0] == EOS)
					break;
				tc = global_df->head_row->next_char->next_char;
				p = st + 1;
				for (; tc!= global_df->tail_row && *p != EOS; tc=tc->next_char, p++) {
					if (tc->c >= 'a' && tc->c <= 'z') {
						if (to_unCH_upper(tc->c) != (unCH)*p)
							break;
					} else if (tc->c != *p)
						break;
				}
				if (*p == ':') {
					if (tc == global_df->tail_row)
						break;
				} else if (*p == EOS)
					break;
			}
		} else {
			if (global_df->row_txt->line[0] == '*') { 
				l = global_df->row_txt->line+1;
				p = "** File \"";
				for (; *p != EOS; l++, p++) {
					if (*l != *p)
						break;
				}
				if (*p == EOS) {
					getFileInfo(global_df->row_txt);
				}
			}
			if (global_df->row_txt->line[0] == st[0]) { 
				if (st[0] == EOS)
					break;
				l = global_df->row_txt->line+1;
				p = st + 1;
				for (; *p != EOS; l++, p++) {
					if (*l >= 'a' && *l <= 'z') {
						if (to_unCH_upper(*l) != (unCH)*p)
							break;
					} else if (*l != *p)
						break;
				}
				if (*p == ':') {
					if (*l == EOS)
						break;
				} else if (*p == EOS)
					break;
			}
		}
		if (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE))
			break;
		if (isNL_CFound(global_df->row_txt))
			isAddLineno = TRUE;
		else
			isAddLineno = FALSE;
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
		if (isAddLineno)
			global_df->lineno++;
		global_df->wLineno++;
		if (global_df->row_win < (long)global_df->EdWinSize)
			global_df->row_win++;
	}
	global_df->redisplay = 1;
}

static char isMatchCaret(unCH *s, LINE *tl, int offset, char isBeg) {
	if (global_df->row_txt == global_df->cur_line) {
		if (tl->c == '^' && tl->next_char != global_df->tail_row && is_unCH_alnum(tl->next_char->c)) {
			global_df->col_txt = tl;
			if (isBeg)
				global_df->col_chr = offset + 1;
			else 
				global_df->col_chr = global_df->col_chr + offset + 1;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
			global_df->col_chr2 = global_df->col_chr - 1;
			global_df->col_win2 = ComColWin(FALSE, NULL, global_df->col_chr2);
			return(1);
		}
	} else {
		if (*s == '^' && *(s+1) != EOS && is_unCH_alnum(*(s+1))) {
			if (isBeg)
				global_df->col_chr = offset + 1;
			else
				global_df->col_chr = global_df->col_chr + offset + 1;
			global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
			global_df->col_chr2 = global_df->col_chr - 1;
			global_df->col_win2 = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr2);
			return(1);
		}
	}
	return(0);
}

static int CaretMatched(char isBeg) {
	unCH *s;
	int offset = 0;

	if (global_df->row_txt == global_df->cur_line) {
		LINE *tl;
		if (isBeg)
			tl = global_df->head_row->next_char;
		else
			tl = global_df->col_txt;
		if (tl->c == HIDEN_C && tl != global_df->tail_row && global_df->ShowParags != '\002') {
			for (tl=tl->next_char,offset++; tl->c!=HIDEN_C && tl!=global_df->tail_row; tl=tl->next_char,offset++) ;
		}
		while (tl != global_df->tail_row) {
			if (isMatchCaret(NULL, tl, offset, isBeg))
				return(1);
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
		if (*s == HIDEN_C && *s != EOS && global_df->ShowParags != '\002') {
			for (s++,offset++; *s != HIDEN_C && *s != EOS; s++,offset++) ;
		}
		while (*s != EOS) {
			if (isMatchCaret(s, NULL, offset, isBeg))
				return(1);
			s++;
			offset++;
			if (*s == HIDEN_C && *s != EOS && global_df->ShowParags != '\002') {
				for (s++,offset++; *s != HIDEN_C && *s != EOS; s++,offset++) ;
			}
		}
	}
	return(0);
}

static int FindCaretOnMor(void) {
	char isAddLineno;
	long old_row_win = global_df->row_win;
	long old_lineno  = global_df->lineno;
	long old_wLineno = global_df->wLineno;
	ROWS *old_row_txt = global_df->row_txt;

	GetCurCode();
	uS.uppercasestr(sp, &dFnt, FALSE);
	if (uS.partcmp(sp, DisTier, FALSE, FALSE)) {
		if (CaretMatched(FALSE)) {
			DisplayRow(FALSE);
			return(1);
		} else {
			if (isNL_CFound(global_df->row_txt))
				isAddLineno = TRUE;
			else
				isAddLineno = FALSE;
			global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
			if (isAddLineno)
				global_df->lineno++;
			global_df->wLineno++;
			if (global_df->row_win < (long)global_df->EdWinSize)
				global_df->row_win++;
		}
	}
	if (global_df->row_txt != global_df->tail_text) {
		if (global_df->row_txt == global_df->cur_line) {
			if (isSpeaker(global_df->head_row->next_char->c))
				FindNextGivenTier(DisTier);
		} else {
			if (isSpeaker(*global_df->row_txt->line))
				FindNextGivenTier(DisTier);
		}
	}
	while (global_df->row_txt != global_df->tail_text) {
		if (CaretMatched(TRUE)) {
			DisplayRow(FALSE);
			return(1);
		}
		if (isNL_CFound(global_df->row_txt))
			isAddLineno = TRUE;
		else
			isAddLineno = FALSE;
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
		if (global_df->row_txt == global_df->tail_text)
			break;
		if (isAddLineno)
			global_df->lineno++;
		global_df->wLineno++;
		if (global_df->row_win < (long)global_df->EdWinSize)
			global_df->row_win++;
		if (global_df->row_txt == global_df->cur_line) {
			if (isSpeaker(global_df->head_row->next_char->c))
				FindNextGivenTier(DisTier);
		} else {
			if (isSpeaker(*global_df->row_txt->line))
				FindNextGivenTier(DisTier);
		}
	}
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;
	global_df->row_txt = old_row_txt;
	global_df->row_win = old_row_win;
	global_df->lineno  = old_lineno;
	global_df->wLineno = old_wLineno;
	return(0);
}

static void getSpTierText(ROWS *tr, int max) {
	int i, j;
	char hcf;
	LINE *tc;

	j = 0;
	templine2[0] = EOS;
	while (!AtTopEnd(tr, global_df->head_text, FALSE)) {
		if (tr == global_df->cur_line) {
			if (isMainSpeaker(global_df->head_row->next_char->c))
				break;
		} else {
			if (isMainSpeaker(tr->line[0]))
				break;
		}
		tr = ToPrevRow(tr, FALSE);
	}
	if (tr == global_df->cur_line) {
		if (!isMainSpeaker(global_df->head_row->next_char->c))
			return;
	} else {
		if (!isMainSpeaker(*tr->line))
			return;
	}
	while (1) {
		hcf = FALSE;
		if (tr == global_df->cur_line) {
			for (tc=global_df->head_row->next_char; tc != global_df->tail_row; tc=tc->next_char) {
				if (tc->c == HIDEN_C || hcf) {
					if (tc->c == HIDEN_C)
						hcf = !hcf;
				} else if (tc->c == '\t')
					templine2[j++] = ' ';
				else if (tc->c != NL_C && tc->c != SNL_C && tc->c != '\n')
					templine2[j++] = tc->c;
			}
		} else {
			for (i=0; tr->line[i] != EOS; i++) {
				if (tr->line[i] == HIDEN_C || hcf) {
					if (tr->line[i] == HIDEN_C)
						hcf = !hcf;
				} else if (tr->line[i] == '\t')
					templine2[j++] = ' ';
				else if (tr->line[i] != NL_C && tr->line[i] != SNL_C && tr->line[i] != '\n')
					templine2[j++] = tr->line[i];
			}
		}
		if (AtBotEnd(tr, global_df->tail_text, FALSE))
			break;
		tr = ToNextRow(tr, FALSE);
		if (tr == global_df->cur_line) {
			if (isSpeaker(global_df->head_row->next_char->c))
				break;
		} else {
			if (isSpeaker(*tr->line))
				break;
		}		
	}
	templine2[j] = EOS;
}

static char findPostmortemTag(void) {
/*
	if (global_df->head_text->next_row != global_df->tail_text) { 
		if (uS.partcmp(global_df->head_text->next_row->line, POSTMORTEMTAG, FALSE, FALSE))
			return(TRUE);
	}
*/
	return(FALSE);
}

int MorDisambiguate(int i) {
	char isPostmortem;
	long (*GetString)(long, unCH *);

#ifdef _MAC_CODE
	DrawCursor(0);
	DrawSoundCursor(0);
#endif
	if (!FindCaretOnMor()) {
		RemoveLastUndo();
		if (!global_df->EditorMode)
			EditMode(-1);
		strcpy(global_df->err_message, "-Done.");
	} else {
		ChangeCurLineAlways(0);
		isPostmortem = findPostmortemTag();
		*ced_line = EOS;
		if (global_df->cod_fname)
			free(global_df->cod_fname);
		global_df->cod_fname = NULL;
		ResetUndos();
		FreeCodesMem();
		isLastCode = FALSE;
		*templine3 = EOS;
		if (isPostmortem)
			GetString = GetNextPostmortemString;
		else
			GetString = GetNextMorString;
		if (init_codes_string(GetString) && COM_WIN_SIZE < 2) {
			global_df->EdWinSize = 0;
			global_df->CodeWinStart = 0;
			global_df->total_num_rows = 4;
			if (!init_windows(true, 1, false))
				mem_err(TRUE, global_df);
		} else
			global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
		if (global_df->EditorMode) {
			EditMode(-1);
			strcpy(global_df->err_message, DASHES);
		}
		HiLightMatch();
		getSpTierText(global_df->row_txt, ERRMESSAGELEN-4);
		if (templine2[0] != EOS) {
			strcpy(global_df->err_message, "-");
			u_strcpy(global_df->err_message+1, templine2, ERRMESSAGELEN-5);
		}
	}
	return(69);
}

int FindRightCode(int disp) {
	register int i;
	if (global_df->EditorMode)
		return(0);
	if (global_df->NoCodes)
		return(0);
	if (global_df->CurCode != global_df->RootCodes)
		return(0);
	if (*sp == EOS) {
		if (disp)
			strcpy(sp, global_df->OldCode);
		else {
			strcpy(global_df->err_message, "-Not pointing to a code tier!");
			return(0);
		}
	} else if (*sp == '*' || *sp == '@') {
		*sp = EOS;
		if (disp)
			strcpy(global_df->err_message, DASHES);
		else
			strcpy(global_df->err_message, "-Not pointing to a code tier!");
		return(0);
	}
	for (i=1; global_df->RootCodesArr[i] != NULL; i++) {
		if (uS.partcmp(global_df->RootCodesArr[i]->code, sp, FALSE, TRUE)) {
			if (global_df->RootCodesArr[i] != global_df->StartCodeArr[global_df->CursorCodeArr]) {
				if (global_df->StartCodeArr <= global_df->RootCodesArr+i && global_df->RootCodesArr+i <= global_df->EndCodeArr) {
					if (disp && !global_df->EditorMode) {
						global_df->redisplay = 0;
						global_df->EditorMode = !global_df->EditorMode;
						DisplayCursor(COM_WIN_SIZE);
						global_df->CursorCodeArr = i - (int)(global_df->StartCodeArr-global_df->RootCodesArr);
						global_df->redisplay = 1;
						global_df->EditorMode = !global_df->EditorMode;
						DisplayCursor(COM_WIN_SIZE);
					} else
						global_df->CursorCodeArr = i-(int)(global_df->StartCodeArr-global_df->RootCodesArr);
				} else {
					global_df->StartCodeArr = global_df->RootCodesArr + i;
					global_df->CursorCodeArr = 0;
					if (disp)
						global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
				}
			}
			*sp = EOS;
			return(1);
		}
	}
	strcpy(global_df->err_message, "-Undefined code tier!");
	*sp = EOS;
	return(0);
}

void DisplayCursor(int wsize) {
	int index = 0, row, col, len, max;

	for (max=0,col=0; col < global_df->num_of_cols; col += max+1,max=0) {
		for (row=0; row < wsize; row++) {
			if (global_df->StartCodeArr[index] == NULL) {
				if (global_df->redisplay)
					wrefresh(global_df->w2);
				return;
			}
			if ((len=strlen(global_df->StartCodeArr[index]->code)) > max)
				max = len;
			if (index == global_df->CursorCodeArr) {
				mvwaddstr(global_df->w2,row,col,cl_T(" "));
				if (!global_df->EditorMode)
					wstandout(global_df->w2);
				waddstr(global_df->w2,global_df->StartCodeArr[index++]->code);
				wstandend(global_df->w2);
				if (global_df->redisplay)
					wrefresh(global_df->w2);
				return;
			} else
				index++;
		}
	}
}

CODES **DisplayCodes(int wsize) {
	int index = 0, row, col, len, max;
	unCH code[2048];

#if defined(_MAC_CODE) || defined(_WIN32)
	if (global_df->SoundWin != NULL)
		return(global_df->EndCodeArr);
#endif
	werase(global_df->w2);
	for (max=0,col=0; col < global_df->num_of_cols; col += max+1,max=0) {
		for (row=0; row < wsize; row++) {
			if (global_df->StartCodeArr[index] == NULL) {
				wrefresh(global_df->w2);
				return(global_df->StartCodeArr+index);
			}
			if ((len=strlen(global_df->StartCodeArr[index]->code)) > max)
				max = len;
			if (max+col+1 > global_df->num_of_cols) {
				wrefresh(global_df->w2);
				return(global_df->StartCodeArr+index);
			}
			if (global_df->EditorMode)
				index++;
			else {
				mvwaddstr(global_df->w2,row,col,cl_T(" "));
				strcpy(code, global_df->StartCodeArr[index]->code);
				uS.remblanks(code);
				if (index == global_df->CursorCodeArr && !global_df->EditorMode) {
					wstandout(global_df->w2);
					waddstr(global_df->w2,code);
					wstandend(global_df->w2);
				} else
					waddstr(global_df->w2,code);
				index++;
			}
		}
	}
	wrefresh(global_df->w2);
	return(global_df->StartCodeArr+index);
}

void MoveToSubcode(void) {
	CODES *tc, *OldSubCode;

	if (global_df->PriorityCodes == '\001') {
		if (global_df->StartCodeArr+global_df->CursorCodeArr != global_df->RootCodesArr+1) {
			(*(global_df->StartCodeArr+(global_df->CursorCodeArr-1)))->NextCode = 
											global_df->StartCodeArr[global_df->CursorCodeArr]->NextCode;
			global_df->StartCodeArr[global_df->CursorCodeArr]->NextCode = global_df->RootCodesArr[0]->subcodes;
			OldSubCode = global_df->RootCodesArr[0]->subcodes;
			global_df->RootCodesArr[0]->subcodes = global_df->StartCodeArr[global_df->CursorCodeArr];
			for (tc=global_df->FirstCodeOfPrevLevel; tc; tc=tc->NextCode) {
				if (tc->subcodes == OldSubCode)
					tc->subcodes = global_df->StartCodeArr[global_df->CursorCodeArr];
			}
		}
	} else if (global_df->PriorityCodes == '\002') {
		if (global_df->StartCodeArr+global_df->CursorCodeArr != global_df->RootCodesArr+1) {
			(*(global_df->StartCodeArr+global_df->CursorCodeArr-1))->NextCode = 
											global_df->StartCodeArr[global_df->CursorCodeArr]->NextCode;
			if (global_df->StartCodeArr+global_df->CursorCodeArr-1 == global_df->RootCodesArr+1) {
				global_df->StartCodeArr[global_df->CursorCodeArr]->NextCode =
												global_df->RootCodesArr[0]->subcodes;
				OldSubCode = global_df->RootCodesArr[0]->subcodes;
				global_df->RootCodesArr[0]->subcodes = global_df->StartCodeArr[global_df->CursorCodeArr];
				for (tc=global_df->FirstCodeOfPrevLevel; tc; tc=tc->NextCode) {
					if (tc->subcodes == OldSubCode)
						tc->subcodes = global_df->StartCodeArr[global_df->CursorCodeArr];
				}
			} else {
				global_df->StartCodeArr[global_df->CursorCodeArr]->NextCode =
												(*(global_df->StartCodeArr+global_df->CursorCodeArr-2))->NextCode;
				(*(global_df->StartCodeArr+global_df->CursorCodeArr-2))->NextCode = 
												global_df->StartCodeArr[global_df->CursorCodeArr];
			}
		}
	}
	if (global_df->StartCodeArr[global_df->CursorCodeArr]->subcodes != NULL) {
		if (global_df->CurCode == global_df->RootCodes) {
			global_df->CurFirstCodeOfPrevLevel = global_df->RootCodesArr[0]->subcodes;
			global_df->CurCode = global_df->StartCodeArr[global_df->CursorCodeArr];
		}
		global_df->FirstCodeOfPrevLevel = global_df->RootCodesArr[0]->subcodes;
		MapArrToList(global_df->StartCodeArr[global_df->CursorCodeArr]);
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	} else if (global_df->RootCodesArr[0] != global_df->CurCode || global_df->PriorityCodes) {
		global_df->FirstCodeOfPrevLevel = global_df->CurFirstCodeOfPrevLevel;
		MapArrToList(global_df->CurCode);
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
	}
}

void FindLastPage(int wsize) {
	int index = 0, row, col, len, max;

	if (global_df->StartCodeArr == global_df->RootCodesArr + 1) {
		global_df->CursorCodeArr = 0;
		return;
	}
	global_df->StartCodeArr--;
	for (max=0,col=0; 1; col += max+1,max=0) {
		for (row=0; row < wsize; row++) {
			if (global_df->StartCodeArr == global_df->RootCodesArr + 1) {
				global_df->CursorCodeArr = index;
				return;
			}
			if ((len=strlen(global_df->StartCodeArr[0]->code)) > max)
				max = len;
			if (max+col+1 > global_df->num_of_cols) {
				global_df->StartCodeArr++;
				global_df->CursorCodeArr = index - 1;
				return;
			}
			index++;
			global_df->StartCodeArr--;
		}
	}
}

static void replaceSpaces(unCH *code, int ThisLevel) {
	int  i, end;

	i = ThisLevel;
	if (code[i] == '"') {
		for (i++; isSpace(code[i]); i++) ;
	}
	if ((ThisLevel == 1 && code[i] == '$') || (ThisLevel > 1 && code[i] == ':' && i == ThisLevel)) {
		end = strlen(code) - 1;
		while (end >= 0 && (isSpace(code[end]) || code[end] == '\n' || code[end] == '\r'))
			end--;
		end++;
		for (; code[i] != EOS && i < end; i++) {
			if (isSpace(code[i]))
				code[i] = '_';
		}
	}
}

static CODES *FillInCodes(FILE *fp, int LastLevel, int *end, int *index, long *ln) {
	unsigned long cnt;
	int ThisLevel, offset, LocMaxNumOfCodes = 1;
	CODES *RootCode, *CurCode, *LastSubcode;

	LastSubcode = NULL;
	ThisLevel = LastLevel;
	RootCode = NEW(CODES);
	if (RootCode == NULL)
		mem_err(TRUE, global_df);

	if (ced_line[ThisLevel] == '"')
		offset = 1;
	else
		offset = 0;
	RootCode->code = AddCodeToList(ced_line+ThisLevel+offset,index);
	RootCode->subcodes = NULL;
	RootCode->NextCode = NULL;
	CurCode = RootCode;
	while (fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt)) {
		for (ThisLevel=0; isSpace(ced_lineC[ThisLevel]); ThisLevel++) {
			if (ced_lineC[ThisLevel] == '\t') {
				sprintf(global_df->err_message, "+ERROR: Tab character is found on line %ld in codes file.", *ln);
				return(NULL);
			}
		}
		if (ThisLevel == 1 && ced_lineC[ThisLevel] == '"' && ced_lineC[ThisLevel+1] == '$') {
			uS.shiftright(ced_lineC+ThisLevel+1, 1);
			ced_lineC[ThisLevel+1] = ' ';
			cnt++;
		}
		UTF8ToUnicode((unsigned char *)ced_lineC, cnt, ced_line, NULL, UTTLINELEN);
		(*ln)++;
		do {
			for (offset=0; isSpace(ced_line[offset]) || ced_line[offset] == '\n'; offset++) ;
			if (ced_line[offset] != EOS)
				break;
			(*ln)++;
			if (!fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt))
				goto contfill;
			UTF8ToUnicode((unsigned char *)ced_lineC, cnt, ced_line, NULL, UTTLINELEN);
		} while (1) ;
		offset = strlen(ced_line) - 1;
		if (offset >= WO) {
			u_strcpy(ced_lineC, ced_line, UTTLINELEN);
			fprintf(stderr,"Input buffer too small for code %s.",ced_lineC);
			exit(1);
		}
		ced_line[offset] = EOS;
		for (ThisLevel=0; isSpace(ced_line[ThisLevel]); ThisLevel++) {
			if (ced_line[ThisLevel] == '\t') {
				sprintf(global_df->err_message, "+ERROR: Tab character is found on line %ld in codes file.", *ln);
				return(NULL);
			}
		}
		replaceSpaces(ced_line, ThisLevel);
/*
printf("ced_line=%s; ThisLevel=%d;LastLevel=%d\n",ced_line,ThisLevel,LastLevel);
*/
		if (ThisLevel > LastLevel) {
			if (ThisLevel > LastLevel+1) {
				sprintf(global_df->err_message, "+ERROR: Line %ld is indented by more than one space relative to previous line in codes file.", *ln);
				return(NULL);
			}
			if ((CurCode->subcodes=FillInCodes(fp,ThisLevel,end,index,ln))== NULL)
				return(NULL);
			LastSubcode = CurCode->subcodes;
			if (*end)
				break;
			for (ThisLevel=0; isSpace(ced_line[ThisLevel]); ThisLevel++) {
				if (ced_line[ThisLevel] == '\t') {
					sprintf(global_df->err_message, "+ERROR: Tab character is found on line %ld in codes file.", *ln);
					return(NULL);
				}
			}
		} else if (*end)
			break;

		if (ThisLevel < LastLevel) {
			if(LocMaxNumOfCodes>MaxNumOfCodes) MaxNumOfCodes=LocMaxNumOfCodes;
			return(RootCode);
		}
		LocMaxNumOfCodes++;
		CurCode->NextCode = NEW(CODES);
		if (CurCode->NextCode == NULL)
			mem_err(TRUE, global_df);
		CurCode = CurCode->NextCode;
		if (ced_line[ThisLevel] == '"')
			offset = 1;
		else
			offset = 0;
		CurCode->code = AddCodeToList(ced_line+ThisLevel+offset,index);
		if (global_df->CodingScheme == '\001')
			CurCode->subcodes = LastSubcode;
		else
			CurCode->subcodes = NULL;
		CurCode->NextCode = NULL;
	}
contfill:
	*end = TRUE;
	if (LocMaxNumOfCodes > MaxNumOfCodes)
		MaxNumOfCodes = LocMaxNumOfCodes;
	return(RootCode);
}

void MapArrToList(CODES *CurCode) {
	int i = 0;

	global_df->RootCodesArr[i++] = CurCode;
	if (CurCode != NULL) {
		for (CurCode=CurCode->subcodes; CurCode!= NULL; CurCode=CurCode->NextCode)
			global_df->RootCodesArr[i++] = CurCode;
	}
	global_df->RootCodesArr[i] = NULL;
	global_df->StartCodeArr = global_df->RootCodesArr + 1;
	global_df->CursorCodeArr = 0;
}

/*
+bN: set number of commands and word before auto-save
-d : do NOT create backup file
+lN: re-order codes (1 = advance to top, 2 = advance one step)
+sN: progam will make identical copies of codes across branches
+tS: set next speaker name to S
+xS: disambiguate tier S (default %s)", DisTier - "%MOR:"

*/
int init_codes(const FNType *fname, const FNType *cname, char *fontName) {
	int end, index;
	int tFreqCountLimit;
	unsigned long cnt;
	char tMakeBackupFile;
	char tPriorityCodes, tCodingScheme, *res;
	long ln;
	FILE *fp;

	RootCodeList = NULL;
	if ((global_df->RootCodes=NEW(CODES)) == NULL)
		mem_err(TRUE, global_df);
	global_df->RootCodes->code = NULL;
	global_df->RootCodes->NextCode = NULL;

	fp = NULL;
	if (*cname && (fp=fopen(cname,"rb")) != NULL)
		fname = cname;
	else if (fname != NULL && *fname)
		fp = fopen(fname,"r");
	else
		fname = NULL;
	tPriorityCodes = DefPriorityCodes;
	tCodingScheme = DefCodingScheme;
	if (fp != NULL) {
		last_cr_char = 0;
		while ((res=fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt)) != NULL) {
			if (!uS.isUTF8(ced_lineC) && !uS.isInvisibleHeader(ced_lineC) &&
				strncmp(ced_lineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) != 0 &&
				ced_lineC[0] != ';' && ced_lineC[0] != '#' && ced_lineC[0] != '\n' && ced_lineC[0] != EOS)
				break;
		}
		if (res != NULL) {
			if (*ced_lineC == '\\') {
				end = 1;
				tMakeBackupFile = MakeBackupFile;
				tFreqCountLimit = FreqCountLimit;
				do {
					while (isSpace(ced_lineC[end]))
						end++;
					if (ced_lineC[end] == '-' || ced_lineC[end] == '+') {
						if (fontName != NULL) {
							if (ced_lineC[end+1] == 'f') {
								strcpy(fontName, ced_lineC+end+2);
								uS.remblanks(fontName);
							}
						}
						ced_getflag(ced_lineC+end);
					}
					while (!isSpace(ced_lineC[end]) && ced_lineC[end])
						end++;
				} while (ced_lineC[end]) ;
				global_df->MakeBackupFile = MakeBackupFile;
				global_df->FreqCountLimit = FreqCountLimit;
				MakeBackupFile = tMakeBackupFile;
				FreqCountLimit = tFreqCountLimit;
			}
		}
		fclose(fp);
	}
	global_df->PriorityCodes = DefPriorityCodes;
	global_df->CodingScheme = DefCodingScheme;
	DefPriorityCodes = tPriorityCodes;
	DefCodingScheme = tCodingScheme;
	index = WO;
	if (fname == NULL || (fp=fopen(fname,"rb")) == NULL) {
		if (global_df->w1 != NULL)
			strcpy(global_df->err_message, "+Can't open codes file");
NoCodesFound:
		strcpy(ced_line,"	");
		global_df->NoCodes = TRUE;
		MaxNumOfCodes = 1;
		global_df->RootCodes->subcodes = NEW(CODES);
		if (global_df->RootCodes->subcodes == NULL)
			mem_err(TRUE, global_df);
		global_df->RootCodes->subcodes->code = AddCodeToList(ced_line,&index);
		global_df->RootCodes->subcodes->subcodes = NULL;
		global_df->RootCodes->subcodes->NextCode = NULL;
		MaxNumOfCodes++;
		global_df->CurFirstCodeOfPrevLevel = global_df->RootCodes;
		global_df->CurCode = global_df->RootCodes;
		global_df->EditorMode = TRUE;
		goto codes_fin;
	} else
		global_df->NoCodes = FALSE;

	MaxNumOfCodes = 0;
	last_cr_char = 0;
	ln = 0;
	while ((res=fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt)) != NULL) {
		ln++;
		if (!uS.isUTF8(ced_lineC) && !uS.isInvisibleHeader(ced_lineC) &&
			strncmp(ced_lineC, SPECIALTEXTFILESTR, SPECIALTEXTFILESTRLEN) != 0 &&
			ced_lineC[0] != ';' && ced_lineC[0] != '#' && ced_lineC[0] != '\n' && ced_lineC[0] != EOS)
			break;
	}
	if (res != NULL) {
		UTF8ToUnicode((unsigned char *)ced_lineC, cnt, ced_line, NULL, UTTLINELEN);
		if (*ced_line == '\\') {
			while (1) {
				if (fgets_ced(ced_lineC, UTTLINELEN, fp, &cnt) == NULL) {
					fclose(fp);
					goto NoCodesFound;
				} else {
					if (ced_lineC[0] != ';' && ced_lineC[0] != '#') {
						UTF8ToUnicode((unsigned char *)ced_lineC, cnt, ced_line, NULL, UTTLINELEN);
						for (end=0; isSpace(ced_line[end]); end++) ;
						if (ced_line[end] != EOS && ced_line[end] != '\n')
							break;
					}
				}
			}
		}
		end = strlen(ced_line) - 1;
		ced_line[end] = EOS;
		if (end > 0) {
			if (ced_line[end-1] != '\t')
				strcat(ced_line, "\t");
		}
		for (end=0; isSpace(ced_line[end]); end++) ;
		if (end != 0) {
			strcpy(global_df->err_message, "+Wrong code format - top line.");
			fclose(fp);
			goto NoCodesFound;
		}
		end = FALSE;
		if ((global_df->RootCodes->subcodes=FillInCodes(fp,0,&end,&index,&ln)) == NULL) {
			index = WO;
			fclose(fp);
			goto NoCodesFound;
		}
	} else
		global_df->RootCodes->subcodes = NULL;

	MaxNumOfCodes++;
	global_df->CurFirstCodeOfPrevLevel = global_df->RootCodes;
	global_df->CurCode = global_df->RootCodes;
	fclose(fp);
codes_fin:
	global_df->total_num_rows = (global_df->NoCodes ? 1 : 4);
	MaxNumOfCodes++;
	global_df->RootCodesArr = (CODES **)malloc(sizeof(CODES *) * MaxNumOfCodes);
	if (global_df->RootCodesArr == NULL)
		mem_err(TRUE, global_df);
/*
printf("MaxNumOfCodes=%d; tot=%d;\n", 
	MaxNumOfCodes,sizeof(CODES *) * MaxNumOfCodes);
getchar();
*/
	global_df->FirstCodeOfPrevLevel = global_df->RootCodes;
	MapArrToList(global_df->RootCodes);
	strcpy(global_df->OldCode,global_df->StartCodeArr[global_df->CursorCodeArr]->code);
	return(1);
}

int GetNewCodes(int i) {
	int len;
	FNType sfFile[FNSize];
	char fontName[512];
	WINDOW *w;
#ifdef _MAC_CODE
	short FName;

	DrawSoundCursor(0);
#endif

	RemoveLastUndo();
#ifdef _MAC_CODE
	len = 0;
	OSType typeList[1];
	typeList[0] = 'TEXT';
	sfFile[0] = EOS;
	if (myNavGetFile("Please locate codes file", -1, typeList, NULL, sfFile)) {
	} else {
		strcpy(global_df->err_message, DASHES);
		if (i > 0)
			return(67);
		else
			return(0);
	}
#elif defined(_WIN32) // _MAC_CODE
	OPENFILENAME	ofn;
	unCH			szFile[FILENAME_MAX];
	unCH			*szFilter;
	unCH			wDirPathName[FNSize];

	szFilter = _T("Utility Files (*.cut)\0*.cut\0All files (*.*)\0*.*\0\0");
	strcpy(szFile, "codes.cut");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0L;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = u_strcpy(wDirPathName, lib_dir, FNSize);
	ofn.lpstrTitle = _T("Please locate codes file");
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;

	if (GetOpenFileName(&ofn) == 0) {
		strcpy(global_df->err_message, DASHES);
		if (i > 0)
			return(67);
		else
			return(0);
	}
	len = 0;
	u_strcpy(sfFile, szFile, FNSize);
#else /* _WIN32 */
	strcpy(st,"Codes file: ");
	len = strlen(st);
	if (!new_getstr(st,len,FF)) {
		strcpy(global_df->err_message, DASHES);
		if (i > 0)
			return(67);
		else
			return(0);
	}
	if (isfiledir(st+len)) {
		strcpy(global_df->err_message, "+Can't read a directory.");
		if (i > 0)
			return(67);
		else
			return(0);
	}
	strncpy(sfFile, st+len, FNSize);
	sfFile[FNSize] = EOS;
#endif /* _MAC_CODE && _WIN32 */
	if (global_df->cod_fname)
		free(global_df->cod_fname);
	global_df->cod_fname = (FNType *)malloc((strlen(sfFile)+1)*sizeof(FNType));
	if (global_df->cod_fname == NULL) {
		strcpy(global_df->err_message, "+Out of core memory.");
		if (i > 0)
			return(67);
		else
			return(0);
	}
	strcpy(global_df->cod_fname, sfFile);

	global_df->EdWinSize = 0;
	global_df->CodeWinStart = 0;
	ResetUndos();
	FreeCodesMem();
	fontName[0] = EOS;
	uS.str2FNType(sfFile, 0L, "");
	if (init_codes(global_df->cod_fname, sfFile, fontName)) {
		if (!init_windows(true, 1, false))
			mem_err(TRUE, global_df);
		if (fontName[0] != EOS) {
#ifdef _MAC_CODE
			FName = 0;
			if (GetFontNumber(fontName, &FName)) {
				if (global_df != NULL)
					w = global_df->w2;
				else
					w = NULL;
				if (w != NULL) {
					for (i=0; i < w->num_rows; i++) {
						w->RowFInfo[i]->FName = FName;
					}
				}
			}
#elif defined(_WIN32) // _MAC_CODE
			if (global_df != NULL)
				w = global_df->w2;
			else
				w = NULL;
			if (w != NULL) {
				for (i = 0; i < w->num_rows; i++) {
					strcpy(w->RowFInfo[i]->FName, fontName);
				}
			}
#endif
		}
	} else
		strcpy(global_df->err_message, "+Can't open codes file");
	return(67);
}
