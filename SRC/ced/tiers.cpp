#include "ced.h"
#include "cu.h"
#ifdef _WIN32 
#include "CedDlgs.h"
#endif
#include "my_ctype.h"

#define HIDETIERFNAME "0hide.cut"

static void ResetWin(void) {
	long i;
	ROWS *p;

	if (global_df->row_txt == global_df->cur_line) {
		if (global_df->col_chr >= global_df->head_row_len) {
			global_df->col_chr = global_df->head_row_len;
			global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
		}
		for (global_df->col_txt=global_df->head_row->next_char, i=global_df->col_chr; i > 0; i--)
			global_df->col_txt = global_df->col_txt->next_char;
	} else if (global_df->col_chr > (long)strlen(global_df->row_txt->line)) {
		global_df->col_chr = strlen(global_df->row_txt->line);
		global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
	}

	i = global_df->row_win;
	p = global_df->row_txt;
	while (i > 0 && !AtTopEnd(p, global_df->head_text, FALSE)) {
		i--;
		p = ToPrevRow(p, FALSE);
	}
	global_df->top_win = p;
	if (i > 0L) global_df->row_win -= i;
	global_df->lineno  = 1L;
	global_df->wLineno = 1L;
	for (p=ToNextRow(global_df->head_text, FALSE); p != global_df->row_txt; p=ToNextRow(p,FALSE)) {
		if (isNL_CFound(p))
			global_df->lineno++;
		global_df->wLineno++;
	}
}

static void ResetTiers(void) {
	ROWS *p;
	long wLen, len;

	p = global_df->head_text->next_row;
	len  = 0L;
	wLen = 0L;
	while (p != global_df->tail_text) {
		TRUE_VIS_ID(p->flag);
		if (isNL_CFound(p))
			len++;
		wLen++;
		if (p == global_df->row_txt) {
			global_df->wLineno = wLen;
			global_df->lineno = len;
		}
		p = p->next_row;
	}
	global_df->numberOfRows = wLen;
	ResetWin();
}

static void LocateNewRow_txt(void) {
	if (CMP_VIS_ID(global_df->row_txt->flag)) ;
	else if (AtBotEnd(global_df->row_txt, global_df->tail_text, FALSE)) {
		if (AtTopEnd(global_df->row_txt, global_df->head_text, FALSE)) {
			ResetTiers();
			strcpy(global_df->err_message, "+No tiers were selected.");
			return;
		} else
			global_df->row_txt = ToPrevRow(global_df->row_txt, FALSE);
	} else
		global_df->row_txt = ToNextRow(global_df->row_txt, FALSE);
	ResetWin();
}

static int checktier(unCH *s) {
	TIERS *p;

	uS.uppercasestr(s, &dFnt, FALSE);
	for (p=global_df->headtier; p != NULL; p=p->nexttier) {
		if (uS.partcmp(s,p->tcode,FALSE,FALSE)) {
			break;
		}
	}
	if (p != NULL) {
		if (!p->include)
			return(FALSE);
	} else {
		if ((*s == '*' && global_df->tcs) || (*s == '@' && global_df->tch) || (*s == '%' && global_df->tct))
			return(FALSE);
	}
	return(TRUE);
}

static char SelectedTier(myFInfo *df, unCH *sp) {
	if (*sp == '@') {
		if (checktier(sp))
			return(TRUE);
	} else if (df->rightspeaker || *sp == '*') {
		if (checktier(sp)) {
			df->rightspeaker = TRUE;
/*			if (*sp != '*' || !nomain) */
				return(TRUE);
		} else if (*sp == '*') {
			df->rightspeaker = FALSE;
			return(FALSE);
		}
	}
	return(FALSE);
}

static int makeTiersChoice(unCH *SpId, char inc) {
	unCH *t;
	TIERS *nt, *tnt;
	register int l1;
	register int l2;

	if (inc == '-' && !strcmp(SpId,"*")) {
/*		nomain = 1; */
		return(1);
	}
	l1 = strlen(SpId);
	if ((t=(unCH *)malloc((l1+2)*sizeof(unCH))) == NULL) {
		strcpy(global_df->err_message, "+out of memory.");
		return(FALSE);
	}
	uS.uppercasestr(SpId, &dFnt, FALSE);
	if (*SpId != '*' && *SpId != '@' && *SpId != '%') strcpy(t, "*");
	else *t = EOS;
	strcat(t,SpId);
	if (global_df->headtier == NULL) {
		if ((global_df->headtier=NEW(TIERS)) == NULL) {
			strcpy(global_df->err_message, "+out of memory.");
			free(t);
			return(FALSE);
		}
		nt = global_df->headtier;
		nt->nexttier = NULL;
	} else {
		tnt= global_df->headtier;
		nt = global_df->headtier;
		while (1) {
			if (nt == NULL)
				break;
			else if (uS.partcmp(SpId,nt->tcode,FALSE,FALSE)) {
				l2 = strlen(nt->tcode);
				if (l1 > l2)
					break;
				else {
					if (nt->include == TRUE && inc == '-') {
						return(2);
					} else if (nt->include == FALSE && inc != '-') {
						return(2);
					} else {
						return(1);
					}
				}
			}
			tnt = nt;
			nt = nt->nexttier;
		}
		if (nt == NULL) {
			if ((tnt->nexttier=NEW(TIERS)) == NULL) {
				strcpy(global_df->err_message, "+out of memory.");
				free(t);
				return(FALSE);
			}
			nt = tnt->nexttier;
			nt->nexttier = NULL;
		} else if (nt == global_df->headtier) {
			if ((global_df->headtier=NEW(TIERS)) == NULL) {
				strcpy(global_df->err_message, "+out of memory.");
				free(t);
				return(FALSE);
			}
			global_df->headtier->nexttier = nt;
			nt = global_df->headtier;
		} else {
			if ((nt=NEW(TIERS)) == NULL) {
				strcpy(global_df->err_message, "+out of memory.");
				free(t);
				return(FALSE);
			}
			nt->nexttier = tnt->nexttier;
			tnt->nexttier = nt;
		}
	}

	nt->include = (char)((inc == '-') ? FALSE : TRUE);
	nt->tcode = t;
	if (nt->include) {
		if (*SpId == '@')
			global_df->tch = TRUE;
		else if (*SpId == '%')
			global_df->tct = TRUE;
		else
			global_df->tcs = TRUE;
	}
	return(1);
}

void ShowAllTiers(int i) {
	TIERS *tt;

	while (global_df->headtier != NULL) {
		tt = global_df->headtier;
		global_df->headtier = global_df->headtier->nexttier;
		if (tt->tcode)
			free(tt->tcode);
		free(tt);
	}
	global_df->tcs = FALSE;
	global_df->tch = FALSE;
	global_df->tct = FALSE;
	ResetTiers();
#ifdef _MAC_CODE
	SetTextWinMenus(TRUE);
#endif // _MAC_CODE
}

int SelectTiers(int i) {
	int len, res;
	long cntline;
	ROWS *p;
	char inc;
	unCH *s, *e;

#if defined(_MAC_CODE) || defined(_WIN32)
	DrawSoundCursor(0);
#endif
	RemoveLastUndo();
#ifdef _MAC_CODE
	do {
		strcpy(sp,"Show, hide or reset tiers? (s/h/r): ");
		len = strlen(sp);
		if(!new_getstr(sp,len,NULL)) {strcpy(global_df->err_message,DASHES); return(65);}
		if (sp[len] == 's' || sp[len] == 'S')
			break;
		else if (sp[len] == 'h' || sp[len] == 'H')
			break;
		else if (sp[len] == 'r' || sp[len] == 'R')
			break;
	} while (1) ;
	if (sp[len] == 'r' || sp[len] == 'R') {
		ShowAllTiers(0);
		goto cont;
	}
	inc = ((sp[len] == 's' || sp[len] == 'S') ? '+' : '-');
	if (inc == '+') {
		strcpy(sp, "List tiers to show: ");
	} else {
		strcpy(sp, "List tiers to hide: ");
	}
	len = strlen(sp);
	if (!new_getstr(sp,len,NULL)) { strcpy(global_df->err_message,DASHES); return(65);}
#endif // _MAC_CODE
#ifdef _WIN32
	CSelectTiers dlg;

	DrawCursor(1);
	dlg.m_ResetTiers = TRUE;
	dlg.m_IncludeTiers = FALSE;
	dlg.m_ExcludeTiers = FALSE;
	if (dlg.DoModal() != IDOK) {
		DrawCursor(0);
		strcpy(global_df->err_message,DASHES);
		return(65);
	}
	DrawCursor(0);
	if (dlg.m_ResetTiers) {
		ShowAllTiers(0);
		goto cont;
	}
	inc = ((dlg.m_IncludeTiers) ? '+' : '-');
	len = 0;
	strcpy(sp, dlg.m_Tiers);
#endif // _WIN32
	s = sp + len;
	while (*s) {
		while (isSpace(*s) || *s == ',' || *s == ';' || *s == '.') s++;
		for (e=s; !isSpace(*e) && *e!= ',' && *e!= ';' && *e!= '.' && *e; e++) ;
		len = (int)*e;
		*e = EOS;
		res = makeTiersChoice(s, inc);
		if (res == 0) {
			return(65);
		} else if (res == 2) {
			ShowAllTiers(0);
			goto cont;
		}
		*e = (char)len;
		s = e;
	}
	inc = TRUE;
	global_df->rightspeaker = TRUE;
	ChangeCurLineAlways(0);
	p = global_df->head_text->next_row;
	cntline = 0L;
	while (p != global_df->tail_text) {
		if (isSpeaker(p->line[0])) {
			s = p->line;
			for (e=sp; (*e=*s) && *s != ':'; s++, e++) ;
			*++e = EOS;
			if (SelectedTier(global_df, sp))
				inc = TRUE;
			else
				inc = FALSE;
		}
		if (inc) {
			cntline++;
			TRUE_VIS_ID(p->flag);
		} else {
			FALSE_VIS_ID(p->flag);
		}
		p = p->next_row;
	}
	global_df->numberOfRows = cntline;
	LocateNewRow_txt();
cont:
	DisplayTextWindow(NULL, 1);
	strcpy(global_df->err_message, DASHES);

	ResetUndos();
#ifdef _MAC_CODE
	SetTextWinMenus(TRUE);
#endif // _MAC_CODE
	return(65);
}

void checkForTiersToHide(myFInfo *df, char error) {
	char inc;
	unCH *s, *e, t;
	long cntline;
	ROWS *p;
	FNType mFileName[FNSize];
	FILE *fp;

	if (!df->ChatMode)
		return;

	extractPath(mFileName, df->fileName);
	if (!isRefEQZero(mFileName)) {
		addFilename2Path(mFileName, HIDETIERFNAME);
		fp = fopen(mFileName, "r");
	} else
		fp = NULL;
	if (fp == NULL)
		fp = OpenGenLib(HIDETIERFNAME,"r",FALSE,FALSE,mFileName);
	if (fp == NULL) {
		if (error)
			strcpy(df->err_message, "+Can't locate or open file: 0hide.cut. Make sure that lib directory is set correctly.");
		return;
	}

	ShowAllTiers(0);
	inc = '-';

	while (fgets_cr(ced_lineC, UTTLINELEN, fp) != NULL) {
		if (uS.isUTF8(ced_lineC) || uS.isInvisibleHeader(ced_lineC))
			continue;
		if (ced_lineC[0] == '#')
			continue;
		uS.remblanks(ced_lineC);
		if (ced_lineC[0] == EOS)
			continue;
		u_strcpy(ced_line, ced_lineC, UTTLINELEN);
		s = ced_line;
		while (*s) {
			while (isSpace(*s) || *s == ',' || *s == ';' || *s == '.') s++;
			for (e=s; !isSpace(*e) && *e!= ',' && *e!= ';' && *e!= '.' && *e; e++) ;
			t = *e;
			*e = EOS;
			if (!makeTiersChoice(s, inc))
				return; 
			*e = t;
			s = e;
		}
	}

	inc = TRUE;
	df->rightspeaker = TRUE;
	p = df->head_text->next_row;
	cntline = 0L;
	while (p != df->tail_text) {
		if (isSpeaker(p->line[0])) {
			s = p->line;
			for (e=sp; (*e=*s) && *s != ':'; s++, e++) ;
			*++e = EOS;
			if (SelectedTier(df, sp))
				inc = TRUE;
			else {
#ifdef _MAC_CODE
				isAjustCursor = FALSE;
#endif
				inc = FALSE;
			}
		}
		if (inc) {
			cntline++;
			TRUE_VIS_ID(p->flag);
		} else {
			FALSE_VIS_ID(p->flag);
		}
		p = p->next_row;
	}
	df->numberOfRows = cntline;
	LocateNewRow_txt();
#ifdef _MAC_CODE
	SetTextWinMenus(TRUE);
#endif // _MAC_CODE
}
