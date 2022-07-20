#include "ced.h"
#include "my_ctype.h"
#include "c_clan.h"
#ifdef _WIN32
#include "Clan2Doc.h"
#endif

#if defined(_MAC_CODE)
	#define isFontDefaultUni(x, y) ((x == ArialUnicodeFOND/* || x == SecondDefUniFOND*/) && y == defFontSize())
#elif defined(_WIN32)
	#define isFontDefaultUni(x, y) ((strcmp(x, "Arial Unicode MS") == 0 || strcmp(x, "Cambria") == 0/* || strcmp(x, "CAfont") == 0*/) && y == defFontSize())
#else
#endif /* _WIN32 */


struct WriteHelp {
	FNType *fn;
#ifdef _WIN32
	unCH Font[LF_FACESIZE];
	long Size;
#else
	short Font, Size;
#endif
	int  charSet;
	char isEndFound;
	char isFontChanged;
	FILE *fpout;
} ;

int  LineNumberDigitSize;
char LineNumberingType;
char  last_cr_char = 0;

char *fgets_ced(char *beg,int size,FILE *fp,unsigned long *ocnt) {
	char isCNT;
	unsigned long cnt;
	extern short DOSdata;

	cnt = 0L;
	size--;
	beg[cnt] = (char)getc(fp);
	if (feof(fp)) {
		if (ocnt != NULL)
			*ocnt = cnt;
		return(NULL);
	}

	if (last_cr_char == '\r' && beg[cnt] == '\n') {
#if defined(_MAC_CODE)
		if (DOSdata == 0)
			DOSdata = 5;
#endif
	}
#if defined(_WIN32)
	else if (last_cr_char == '\n' && beg[cnt] != '\r') {
		if (DOSdata == 0)
			DOSdata = 5;
	}
#endif
	do {
		isCNT = TRUE;
		if (beg[cnt] == '\r') {
			if (last_cr_char != '\n') {
				beg[cnt] = '\n';
				last_cr_char = '\r';
				break;
			} else {
				isCNT = FALSE;
			}
		} else if (beg[cnt] == '\n') {
			if (last_cr_char != '\r') {
				beg[cnt] = '\n';
				last_cr_char = '\n';
				break;
			} else {
				isCNT = FALSE;
			}
		}
		last_cr_char = 0;
		if (cnt >= size)
			break;
		if (isCNT)
			cnt++;
		beg[cnt] = (char)getc(fp);
		if (feof(fp)) {
			cnt--;
			break;
		}
	} while (1) ;
	cnt++;
	if (beg[0] == '\032' && cnt == 1L)		 
		cnt = 0L;
	beg[cnt] = EOS;
	if (ocnt != NULL)
		*ocnt = cnt;
	return(beg);
}

#if !defined(_WIN32)
int new_getstr(unCH *st, int len, int (* fn)(WINDOW *w,unCH *st,unCH c)) {
	int c, i = len;
	WINDOW *w, *wi;

	w = NULL;
	wi = newwin(1, global_df->num_of_cols, global_df->CodeWinStart, 0, 0);
	wstandend(wi);
	mvwaddstr(wi,0,0,cl_T(": "));
	waddstr(wi,st);
	wclrtoeol(wi, true);
	wrefresh(wi);
	while (1) {
		c = wgetch(wi);
#ifdef VT200
		c = VT200key(c,wi);
#endif
rep:
		if (c == CTRL('M')) break;
		else if (c == CTRL('G') || c == 27) {
			if (fn != NULL && w != NULL) {
				werase(w);	wrefresh(w);  delwin(w);
				touchwin(global_df->w1); touchwin(global_df->wm); wrefresh(global_df->w1); wrefresh(global_df->wm);
			}
			werase(wi); wrefresh(wi); delwin(wi);
			if (!issoundwin()) {
				touchwin(global_df->w2); wrefresh(global_df->w2);
			}
			return(0);
		} else if ((c == '?' || c == ' ') && fn != NULL) {
			st[i] = EOS;
			if (c == ' ') {
				fn(w, st+len, '\002');
				i = strlen(st);
			} else {
				if (w == NULL) {
					w = newwin(NumOfRowsOfDefaultWindow() -
							   COM_WIN_SIZE - MID_WIN_SIZE,global_df->num_of_cols,0,0,0);
					wstandend(w);
				}
				c = fn(w, st+len, '\001');
				goto rep;
			}
		} else if (c == CTRL('H') || c == 127) {
			if (i > len) i--;
		} else if (i < 79 && c == '\n' && fn == NULL) {
			st[i++] = '\n';
		} else if (i < 79 && c == CTRL('I') && fn == NULL) {
			int new_col;
			new_col = ((((i-len) / TabSize) + 1) * TabSize) + len;
			while (i < new_col && i < 79) st[i++] = ' ';
		} else if (i < 79 && c >= 32 && c < 256) st[i++] = (char)c;
		st[i] = EOS;

		mvwaddstr(wi,0,0,cl_T(": "));
		waddstr(wi,st);
		wclrtoeol(wi, true);
		wrefresh(wi);
	}
	if (fn != NULL && w != NULL) {
		werase(w);	wrefresh(w);  delwin(w);
		touchwin(global_df->w1); touchwin(global_df->wm); wrefresh(global_df->w1); wrefresh(global_df->wm);
	}
	werase(wi); wrefresh(wi); delwin(wi);
	if (!issoundwin()) {
		touchwin(global_df->w2); wrefresh(global_df->w2);
	}
	if (w == NULL) return(1);
	else return(2);
}
#endif /* _WIN32 */

static void locateCharInText(myFInfo *fi, ROWS *cRow, long pos, long *aPos, long *skip) {
	long i;
	ROWS *tr;

	*aPos = 0L;
	*skip = 0;
	for (tr=fi->head_text->next_row; tr != fi->tail_text; tr = tr->next_row) {
		if (tr == cRow) {
			for (i=0; tr->line[i] != EOS && i < pos; i++) {
				if (tr->line[i]!='\n' && !isSpace(tr->line[i]) && tr->line[i]!=NL_C && tr->line[i]!=SNL_C) {
					*skip = 0;
					(*aPos)++;
				} else
					(*skip)++;
			}
			break;
		}
		for (i=0; tr->line[i] != EOS; i++) {
			if (tr->line[i]!='\n' && !isSpace(tr->line[i]) && tr->line[i]!=NL_C && tr->line[i]!=SNL_C) {
				*skip = 0;
				(*aPos)++;
			} else
				(*skip)++;
		}
	}
}

void getTextCursor(myFInfo *g_df, WindowInfo *wi) {
	ROWS  *tr;

	if (g_df == NULL)
		return;

	tr = g_df->row_txt;
	locateCharInText(g_df, g_df->top_win, 0, &wi->topC, &wi->skipTop);
	locateCharInText(g_df, tr, g_df->col_chr, &wi->pos1C, &wi->skipP1);
	if (g_df->row_win2 || g_df->col_win2 != -2L) {
		wi->pos2C = g_df->row_win2;
		if (wi->pos2C < 0L) {
			for (; wi->pos2C && tr->prev_row != g_df->head_text; wi->pos2C++, tr=tr->prev_row) ;
		} else if (wi->pos2C > 0L) {
			for (; wi->pos2C && tr->next_row != g_df->tail_text; wi->pos2C--, tr=tr->next_row) ;
		}
		locateCharInText(g_df, tr, g_df->col_chr2, &wi->pos2C, &wi->skipP2);
	} else {
		wi->pos2C = wi->pos1C;
		wi->skipP2 = wi->skipP1;
	}
}

static int PutLine(unCH *sOrg, FILE *fpout, FNType *fn, char isConvert, char isNoBlankLines) {
	int i;

	if (*sOrg != EOS) {
		if (isConvert) {
			for (i=0; sOrg[i] != EOS; i++) {
				if (sOrg[i] >= 0xFF01 && sOrg[i] <= 0xFF5E) {
					sOrg[i] = sOrg[i] - 0xFEE0;
				} else if (sOrg[i] == 0x3000) {
					uS.shiftright(sOrg+i, 1);
					sOrg[i++] = 0x0020;
					sOrg[i] = 0x0020;
				} else if (sOrg[i] == 0x3001)
					sOrg[i] = 0x002C;
				else if (sOrg[i] == 0x3002)
					sOrg[i] = 0x002E;
				else if (sOrg[i] == 0x30FB)
					sOrg[i] = 0x002F;
				else if (sOrg[i] == 0x300C)
					sOrg[i] = 0x201C;
				else if (sOrg[i] == 0x300D)
					sOrg[i] = 0x201D;
			}
			UnicodeToUTF8(sOrg, strlen(sOrg), (unsigned char *)templineC3, NULL, UTTLINELEN);
		} else {
			for (i=0; sOrg[i]; i++)
				templineC3[i] = (char)sOrg[i];
			templineC3[i] = EOS;
		}
		if (fputs(templineC3,fpout) == EOF) {
			sprintf(global_df->err_message, "+Disk full or file \"%s\" or disk is read only.", fn); 
			return(FALSE);
		}
	} else if (isNoBlankLines) 
		return(TRUE);

	if (isUnixCRs) {
		if (fputc(0x0A,fpout) == EOF) {
			sprintf(global_df->err_message, "+Disk full or file \"%s\" or disk is read only.", fn); 
			return(FALSE);
		}
	} else if (rawTextInput) {
		if (global_df->crType == MacCrType) {
			if (fputs("\n",fpout) == EOF) {
				sprintf(global_df->err_message, "+Disk full or file \"%s\" or disk is read only.", fn); 
				return(FALSE);
			}
		} else /* if (global_df->crType == PCCrType) */{
			if (fputc(0x0D,fpout) == EOF) {
				sprintf(global_df->err_message, "+Disk full or file \"%s\" or disk is read only.", fn); 
				return(FALSE);
			}
#ifdef _MAC_CODE
			if (fputc(0x0A,fpout) == EOF) {
				sprintf(global_df->err_message, "+Disk full or file \"%s\" or disk is read only.", fn); 
				return(FALSE);
			}
#endif
		}
	} else {
		if (fputc('\n',fpout) == EOF) {
			sprintf(global_df->err_message, "+Disk full or file \"%s\" or disk is read only.", fn); 
			return(FALSE);
		}
	}
	return(TRUE);
}

static char AddFontInfo(FONTINFO *fontInfo, const char *hd, const char *es) {
	int len;
	strcat(templine3, hd);
#ifdef _MAC_CODE
	Str255 pFontName;
	char FontName[256];
	int CharSet;

	GetFontName(fontInfo->FName, pFontName);
	p2cstrcpy(FontName, pFontName);
	if (FontName[0] == EOS)
		return(FALSE);
	CharSet = my_FontToScript(fontInfo->FName, fontInfo->CharSet);
	if (CharSet == 1 && strcmp(FontName, "CAfont") && strcmp(FontName, "Arial Unicode MS") && 
						strcmp(FontName, "Ascender Uni Duo")) {
		if (fontInfo->FSize < 14)
			strcat(templine3, "Win95:MS Mincho:-15:128");
		else if (fontInfo->FSize > 14)
			strcat(templine3, "Win95:MS Mincho:-19:128");
		else
			strcat(templine3, "Win95:MS Mincho:-16:128");
		return(TRUE);
	} else {
		len = strlen(templine3);
		u_strcpy(templine3+len, FontName, UTTLINELEN-len);
	}
#elif defined(_WIN32)
	float tf;
	long FSize;
	extern float scalingSize;

	strcat(templine3, "Win95:");
	len = strlen(templine3);
	u_strcpy(templine3+len, fontInfo->FName, UTTLINELEN-len);
#endif
	strcat(templine3, ":");
#ifdef _MAC_CODE
	uS.sprintf(templine3+strlen(templine3), cl_T("%d"), fontInfo->FSize);
#endif
#ifdef _WIN32
	if (scalingSize != 1) {
		tf = (float)fontInfo->FSize;
		tf = tf / scalingSize;
		FSize = (long)tf;
	} else
		FSize = fontInfo->FSize;
	uS.sprintf(templine3+strlen(templine3), cl_T("%ld"), FSize);
#endif
	strcat(templine3, ":");
	uS.sprintf(templine3+strlen(templine3), cl_T("%d"), (int)fontInfo->CharSet);
	strcat(templine3, es);
	return(TRUE);
}

static int SaveHeaders(FILE *fpout, FNType *fn) {
	int   i;
	char  isWinDimChanged;
	short top, left, height, width;
	WindowInfo *wi;
	COLORTEXTLIST *tCT;

	if (global_df->isUTF && !global_df->dontAskDontTell) {
		if (!PutLine(cl_T(UTF8HEADER), fpout, fn, FALSE, FALSE))
			return(0);
	}
	if (global_df->PIDs != NULL) {
		strcpy(templine3, PIDHEADER);
		strcat(templine3, "\t");
		strcat(templine3, global_df->PIDs);
		if (!PutLine(templine3, fpout, fn, TRUE, FALSE))
			return(0);
	}
	if (global_df->RootColorText != NULL) {
		strcpy(templine3, CKEYWORDHEADER);
		strcat(templine3, "\t");
		i = strlen(templine3);
		for (tCT=global_df->RootColorText; tCT != NULL; tCT=tCT->nextCT) {
			strcat(templine3+i, tCT->keyWord);
			i = strlen(templine3);
			uS.sprintf(templine3+i, cl_T(" %d %d %d %d "), tCT->cWordFlag, tCT->red, tCT->green, tCT->blue);
			i = strlen(templine3);
		}
		if (!PutLine(templine3, fpout, fn, TRUE, FALSE))
			return(0);
	}
	getTextCursor(global_df, &global_df->winInfo);
	wi = &global_df->winInfo;
	isWinDimChanged = FALSE;
#ifdef _MAC_CODE
	GetWindTopLeft(global_df->wind, &left, &top);
	height = windHeight(global_df->wind);
	width = windWidth(global_df->wind);
	if (top != defWinSize.top || left != defWinSize.left || height != defWinSize.height || width != defWinSize.width)
		isWinDimChanged = TRUE;
#else // _WIN32
	top = 0; left  = 0; height = 0; width = 0;
#endif // _WIN32
	if (isWinDimChanged || wi->topC != 0L || wi->skipTop != 0L || wi->pos1C != 0L || wi->skipP1 != 0L || wi->pos2C != 0L || wi->skipP2 != 0L) {
		strcpy(templine3, WINDOWSINFO);
		strcat(templine3, "\t");
		uS.sprintf(templine3+strlen(templine3), cl_T("%d_%d_%d_%d_%ld_%ld_%ld_%ld_%ld_%ld"), top, left, height, width,
				   wi->topC, wi->skipTop, wi->pos1C, wi->skipP1, wi->pos2C, wi->skipP2);
		if (!PutLine(templine3, fpout, fn, TRUE, FALSE))
			return(0);
	}
	return(1);
}

long DealWithAtts(unCH *line, long i, AttTYPE att, AttTYPE oldAtt, char isJustStats) {
	if (att != oldAtt) {
		if (is_underline(att) != is_underline(oldAtt)) {
			if (!isJustStats) {
				if (is_underline(att))
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, underline_start);
				else
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, underline_end);
			}
			i += 2;
		}
		if (is_italic(att) != is_italic(oldAtt)) {
			if (!isJustStats) {
				if (is_italic(att))
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, italic_start);
				else
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, italic_end);
			}
			i += 2;
		}
		if (is_bold(att) != is_bold(oldAtt)) {
			if (!isJustStats) {
				if (is_bold(att))
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, bold_start);
				else
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, bold_end);
			}
			i += 2;
		}
		if (is_error(att) != is_error(oldAtt)) {
			if (!isJustStats) {
				if (is_error(att))
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, error_start);
				else
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, error_end);
			}
			i += 2;
		}
		if (is_word_color(att) != is_word_color(oldAtt)) {
			if (!isJustStats) {
				char color;

				color = get_color_num(att);
				if (color) {
					if (color == blue_color)
						uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, blue_start);
					else if (color == red_color)
						uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, red_start);
					else if (color == green_color)
						uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, green_start);
					else // if (color == magenta_color)
						uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, magenta_start);
				} else
					uS.sprintf(line+i, cl_T("%c%c"), ATTMARKER, color_end);
			}
			i += 2;
		}
	}
	return(i);
}

static AttTYPE *DEALWITHATTRIBUTES(unCH *pt, AttTYPE *att, long *cnt, AttTYPE *oldAtt) {
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

static char WriteLinesToFile(ROWS *rt, unCH tierType, void *dataOrg) {
	long i;
	char isBullet;
	unCH *s;
	AttTYPE *att;
	AttTYPE oldAtt;
	struct WriteHelp *data;

	if (rt->line[0] == EOS)
		return(TRUE);
	data = (struct WriteHelp *)dataOrg;
	*templine3 = EOS;
	oldAtt = 0;

	s = rt->line;
	att = rt->att;
	i = 0;
	if (global_df->ChatMode) {
		if (data->isFontChanged || (!rawTextInput && (!isFontDefaultUni(rt->Font.FName, rt->Font.FSize) || !global_df->isUTF))) {
			if (!isFontEqual(data->Font, rt->Font.FName) || 
				  data->Size != rt->Font.FSize ||
				  data->charSet != rt->Font.CharSet) {
				if (*s == '@') {
					strcpy(templine3, FONTHEADER);
					if (AddFontInfo(&rt->Font, "\t", "")) {
						if (!PutLine(templine3, data->fpout, data->fn, FALSE, FALSE))
							return(FALSE);
					}
				} else {
					if (isSpeaker(*s)) {
						while (*s && *s != ':') {
							att = DEALWITHATTRIBUTES(templine3, att, &i, &oldAtt);
							if (*s != NL_C)
								templine3[i++] = *s;
							s++;
						}
						if (*s == ':') {
							att = DEALWITHATTRIBUTES(templine3, att, &i, &oldAtt);
							templine3[i++] = *s++;
						}
					}
					while (isSpace(*s)) {
						att = DEALWITHATTRIBUTES(templine3, att, &i, &oldAtt);
						templine3[i++] = *s++;
					}
					templine3[i] = EOS;
/* 2009-09-14 [%fnt: *:*]
					AddFontInfo(&rt->Font, FONTMARKER, "] ");
					i = strlen(templine3);
*/
				}
				SetFontName(data->Font, rt->Font.FName);
				data->Size = rt->Font.FSize;
				data->charSet = rt->Font.CharSet;
				if (isFontDefaultUni(rt->Font.FName, rt->Font.FSize) && global_df->isUTF)
					data->isFontChanged = FALSE;
				else
					data->isFontChanged = TRUE;
			}
		}
	} else if (global_df->SpecialTextFile) {
		if (!isFontEqual(data->Font, rt->Font.FName) || 
						data->Size != rt->Font.FSize || data->charSet != rt->Font.CharSet) {
			strcpy(templine3, FONTHEADER);
			if (AddFontInfo(&rt->Font, "\t", "")) {
				if (!PutLine(templine3, data->fpout, data->fn, FALSE, FALSE))
					return(FALSE);
			}
			SetFontName(data->Font, rt->Font.FName);
			data->Size = rt->Font.FSize;
			data->charSet = rt->Font.CharSet;
		}
	}
	isBullet = FALSE;
	while (*s != EOS) {
		if (*s == HIDEN_C) {
			isBullet = !isBullet;
			if (!isSpace(templine3[i-1]) && i > 1L && isBullet)
				templine3[i++] = ' ';
		}
		att = DEALWITHATTRIBUTES(templine3, att, &i, &oldAtt);
		if (*s != NL_C)
			templine3[i++] = *s;
		s++;
	}
	i = DealWithAtts(templine3, i, 0, oldAtt, FALSE);
	templine3[i] = EOS;
	if (global_df->ChatMode) {
		i = strlen(templine3) - 1;
		for (; i >= 0 && isSpace(templine3[i]); i--) ;
		templine3[++i] = EOS;
	}
	if (global_df->ChatMode && !uS.mStricmp(templine3, "@End"))
		data->isEndFound = TRUE;
	if (!PutLine(templine3, data->fpout, data->fn, TRUE, data->isEndFound))
		return(FALSE);
	return(TRUE);
}

#ifdef _MAC_CODE
static void SaveFontOfTEXT(unCH *buf, FNType *name) {
	Handle InfoHand;
	int   oldResRefNum, i;
	short ResRefNum;
	struct MPSR1020 *fontInfo;
	FSRef ref;
	FSSpec fss;

	InfoHand = NULL;
	oldResRefNum = CurResFile();
	my_FSPathMakeRef(name, &ref); 
	FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, &fss, NULL);
	if ((ResRefNum=HOpenResFile(fss.vRefNum,fss.parID,fss.name,fsRdWrShPerm)) == -1) {
		if (!access(name,0)) {
			HCreateResFile(fss.vRefNum,fss.parID,fss.name);
			ResRefNum=HOpenResFile(fss.vRefNum,fss.parID,fss.name,fsRdWrShPerm);
		}
	}
/*
	if ((ResRefNum=FSOpenResFile(&ref, fsRdWrShPerm)) == -1) {
		if (!access(name, 0)) {
			FSCreateResourceFork(&ref, 0, NULL, 0);
			ResRefNum = FSOpenResFile(&ref, fsRdWrShPerm);
		} else
			ResRefNum = -1;
	}
*/
	if (ResRefNum != -1) {
		UseResFile(ResRefNum);
		InfoHand = Get1Resource('MPSR',1020);
		if (InfoHand) {
			HLock(InfoHand);
			fontInfo = (struct MPSR1020 *)*InfoHand;
			for (i=0; buf[i] != EOS; i++)
				fontInfo->font[i] = (char)buf[i];
			fontInfo->font[i] = EOS;
			HUnlock(InfoHand);
			ChangedResource(InfoHand);
		} else {
			InfoHand = NewHandle(sizeof(struct MPSR1020));
			if (InfoHand) {
				HLock(InfoHand);
				fontInfo = (struct MPSR1020 *)*InfoHand;
				for (i=0; buf[i] != EOS; i++)
					fontInfo->font[i] = (char)buf[i];
				fontInfo->font[i] = EOS;
				HUnlock(InfoHand);
				AddResource(InfoHand, 'MPSR', 1020, "\p");
			}
		}
		CloseResFile(ResRefNum);
	}
	UseResFile(oldResRefNum);
}
#endif // _MAC_CODE

static char UnwrapLines(char (*finLine)(ROWS *rt, unCH tierType, void *), void *data) {
	long pos, oPos;
	char isEOSFound;
	unCH tierType;
	ROWS *rt, *end, newRow;

	rt = global_df->head_text->next_row;
	end = global_df->tail_text;
	ced_line[0] = EOS;
	newRow.line = ced_line;
	newRow.att  = tempAtt;
	copyFontInfo(&newRow.Font, &rt->Font, FALSE);
	tierType = 0;
	pos = 0L;
	isEOSFound = FALSE;
	while (rt != end) {
		if (rt->line[0] == '*' || rt->line[0] == '%' || rt->line[0] == '@')
			tierType = rt->line[0];
		oPos = 0L;
		if (rt->line[0] == '\t' && isEOSFound)
			rt->line[0] = ' ';
		while (rt->line[oPos] != NL_C && rt->line[oPos] != EOS) {
			ced_line[pos] = rt->line[oPos];
			if (rt->att == NULL)
				tempAtt[pos] = 0;
			else
				tempAtt[pos] = rt->att[oPos];
			pos++;
			oPos++;
		}
		if (rt->line[oPos] == NL_C) {
			isEOSFound = FALSE;
			if (pos > 0) {
				ced_line[pos] = EOS;
				if (!(*finLine)(&newRow, tierType, data)) {
					return(FALSE);
				}
			}
			ced_line[0] = EOS;
			pos = 0L;
		} else
			isEOSFound = TRUE;
		copyFontInfo(&newRow.Font, &rt->Font, FALSE);
		rt = rt->next_row;
	}
	if (pos > 0) {
		ced_line[pos] = EOS;
		if (!(*finLine)(&newRow, tierType, data)) {
			return(FALSE);
		}
	}
	return(TRUE);
}

int SaveToFile(FNType *fn, char isGiveErrorMessage) {
	struct WriteHelp data;
	FONTINFO *fontInfo;

	if ((data.fpout=fopen(fn,"w")) == NULL) {
		if (isGiveErrorMessage)
			sprintf(global_df->err_message, "+Can't write file %s. Maybe it is opened by another application.", fn); 
		return(0);
	}
	if (!mem_error)
		ChangeCurLineAlways(0);
#ifdef _MAC_CODE
	if (!global_df->dontAskDontTell)
		settyp(fn, 'TEXT', the_file_creator.out, FALSE);
#endif
	fontInfo = &global_df->head_text->next_row->Font;

/*
	if (global_df->isUTF) {
		fprintf(fpout, "%c%c%c", 0xef, 0xbb, 0xbf); // BOM UTF8
	}
*/
	if (!SaveHeaders(data.fpout, fn)) {
		fclose(data.fpout);
		return(0);
	}

	if (global_df->ChatMode) {
/* 2016-03-29 no more @Font header
		if (!rawTextInput && (!isFontDefaultUni(fontInfo->FName, fontInfo->FSize) || !global_df->isUTF)) {
			strcpy(templine3, FONTHEADER);
			if (AddFontInfo(fontInfo, "\t", "")) {
				if (!PutLine(templine3, data.fpout, fn, FALSE, FALSE)) {
					fclose(data.fpout);
					return(0);
				}
			}
		}
*/
	} else if (global_df->SpecialTextFile) {
		strcpy(templine3, SPECIALTEXTFILESTR);
		strcat(templine3, "\n");
		templine3[strlen(templine3)-1] = EOS;
		if (!PutLine(templine3, data.fpout, fn, FALSE, FALSE)) {
			fclose(data.fpout);
			return(0);
		}
		strcpy(templine3, FONTHEADER);
		if (AddFontInfo(fontInfo, "\t", "")) {
			if (!PutLine(templine3, data.fpout, fn, FALSE, FALSE)) {
				fclose(data.fpout);
				return(0);
			}
		}
	}

	data.fn = fn;
	SetFontName(data.Font, global_df->head_text->next_row->Font.FName);
	data.Size = global_df->head_text->next_row->Font.FSize;
	data.charSet = global_df->head_text->next_row->Font.CharSet;
	data.isEndFound = FALSE;
	data.isFontChanged = FALSE;

	if (doReWrap && global_df->ChatMode) {
		if (!UnwrapLines(WriteLinesToFile, (void *)&data)) {
			fclose(data.fpout);
			return(0);
		}
	} else {
		if (!Re_WrapLines(WriteLinesToFile, SCREENWIDTH, FALSE, (void *)&data)) {
			fclose(data.fpout);
			return(0);
		}
	}
	fclose(data.fpout);

#ifdef _MAC_CODE
	if (!global_df->ChatMode) {
		strcpy(templine3, FONTHEADER);
		if (AddFontInfo(fontInfo, "\t", ""))
			SaveFontOfTEXT(templine3, fn);
	}
#endif
	return(1);
}

static char isfiledir(FNType *filepath) {
	return(FALSE);
}

int SaveCurrentFile(int i) {
	char   dateFound;
	UInt32 dateValue;
	FNType new_name[1024], *s;
	extern char isShowCHECKMessage;

	if (i > -1)
		RemoveLastUndo();
	if (global_df == NULL) {
		return(38);
	}
#ifdef _MAC_CODE
	if (isRefEQZero(global_df->fileName)) {
		clWriteFile(-1);
		return(38);
	}
#elif defined(_WIN32)
	if (!isalpha(global_df->fileName[0]) || global_df->fileName[1] != ':') {
		clWriteFile(-1);
		return(38);
	}
#endif
	if (isfiledir(global_df->fileName)) {
		strcpy(global_df->err_message, "+Error writting; Use \"Save as\" command and change the file name.");
		return(38);
	}
	if (global_df->DataChanged == '\0') {
		if (getFileDate(global_df->fileName, &dateValue))
			dateFound = TRUE;
		else
			dateFound = FALSE;
		SaveToFile(global_df->fileName, FALSE);
		if (dateFound)
			setFileDate(global_df->fileName, dateValue);
		strcpy(global_df->err_message, DASHES);
		return(38);
	}
	if ((MakeBackupFile || global_df->MakeBackupFile) && !access(global_df->fileName,0)) {
		strcpy(new_name,global_df->fileName);
		uS.str2FNType(new_name, strlen(new_name), ".bak");
		unlink(new_name);

		if (rename(global_df->fileName,new_name)) {
			strcpy(global_df->err_message, "+Error making backup file. File NOT saved.");
			return(38);
		}
	}
	if (!SaveToFile(global_df->fileName, TRUE)) {
		if ((MakeBackupFile || global_df->MakeBackupFile) && !access(new_name,0)) {
			unlink(global_df->fileName);
			if (rename(new_name,global_df->fileName)) {
				strcpy(global_df->err_message, "+File NOT saved.");
				return(38);
			}
		}
		return(38);
	}
/* 2008-04-28
	if (!MakeBackupFile && !global_df->MakeBackupFile) {
		if (unlink(new_name)) {
			if (unlink(new_name)) {
				if (unlink(new_name))
					unlink(new_name);
			}
		}
	}
*/
	sprintf(global_df->err_message, "-File \"%s\" written.", global_df->fileName);
	s = strrchr(global_df->fileName, PATHDELIMCHR);
	if (s == NULL)
		s = global_df->fileName;
	else
		s++;
	if (!strcmp(s, ALIAS_FILE_D) || !strcmp(s, ALIAS_FILE_U)) {
		re_readAliases();
	}
	global_df->DataChanged = '\0';
	strcpy(new_name,global_df->fileName);
	uS.str2FNType(new_name, strlen(new_name), ".ckp");
#if defined(_WIN32)
	GlobalDoc->SetModifiedFlag(FALSE);
#endif /* _WIN32 */
	if (unlink(new_name)) {
		if (unlink(new_name)) {
			if (unlink(new_name))
				unlink(new_name);
		}
	}
	ResetUndos();
	if (isShowCHECKMessage) {
		if (global_df->checkMessCnt == 1) {
			do_warning("Please remember to run CHECK whenever you save a file", 0);
		} else if (global_df->checkMessCnt >= 4) {
			global_df->checkMessCnt = 0;
		}
		global_df->checkMessCnt++;
	}
	return(38);
}

void SaveCKPFile(char isask) {
	FNType new_name[FNSize];

	if (*global_df->fileName == EOS) {
		return;
	}
	if (isRefEQZero(global_df->fileName) && isask) {
		clWriteFile(-2);
	}
	if (!mem_error) {
		strcpy(global_df->err_message, "-Checkpoint ... ");
		draw_mid_wm();
	}
	strcpy(new_name,global_df->fileName);
	uS.str2FNType(new_name, strlen(new_name), ".ckp");
	if (isfiledir(new_name)) {
		strcpy(global_df->err_message, "+Error checkpoint; Change the file name.");
		return;
	}
	if (SaveToFile(new_name, TRUE)) {
		strcpy(global_df->err_message, "-Checkpoint ... Done!");
	} else
		unlink(new_name);
	ResetUndos();
}

int clWriteFile(int i) {
	int len;
	char *s;
#ifdef _MAC_CODE
	FNType fileName[FNSize];
	WindowProcRec	*windProc;

	DrawSoundCursor(0);
#elif defined(_WIN32) // _MAC_CODE
	char fileName[FNSize];
#endif
	if (global_df == NULL)
		return(22);

	if (i > -1)
		RemoveLastUndo();
#ifdef _MAC_CODE
	strcpy(fileName, global_df->fileName);
  	if (!myNavPutFile(fileName, "Save File", 'TEXT', NULL)) {
		strcpy(global_df->err_message, DASHES);
		return(22);
	}
	len = 0;
#elif defined(_WIN32) // _MAC_CODE
	OPENFILENAME	ofn;
	char			fName[FILENAME_MAX];
	unCH			szFile[FNSize], szFileTitle[FILENAME_MAX];
	unCH			*szFilter;

	szFilter = _T("CHAT files (*.cha)\0*.cha\0CUT files (*.cut)\0*.cut\0All files (*.*)\0*.*\0\0");
//	szFilter = NULL;
	u_strcpy(szFile, global_df->fileName, FNSize);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = AfxGetApp()->m_pMainWnd->m_hWnd;//GlobalDC->GetWindow()->m_hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0L;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = cl_T("Write File");
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;

	DrawCursor(1);
	if (GetSaveFileName(&ofn) == 0) {
		strcpy(global_df->err_message, DASHES);
		DrawCursor(0);
		return(22);
	}
	DrawCursor(0);
	u_strcpy(fileName, szFile, FNSize);
	len = strlen(fileName);
	if (ofn.nFilterIndex == 1) {
		if (uS.mStrnicmp(fileName+len-4, ".cut", 4) == 0) {
			fileName[len-4] = EOS;
		}
		if (uS.mStrnicmp(fileName+len-4, ".cha", 4)) {
			strcat(fileName, ".cha");
		}
	} else if (ofn.nFilterIndex == 2) {
		if (uS.mStrnicmp(fileName+len-4, ".cha", 4) == 0) {
			fileName[len-4] = EOS;
		}
		if (uS.mStrnicmp(fileName+len-4, ".cut", 4)) {
			strcat(fileName, ".cut");
		}
	}
	u_strcpy(szFile, fileName, FNSize);
	extractFileName(fName, fileName);
	u_strcpy(szFileTitle, fName, FILENAME_MAX);
	len = 0;
#else /* _WIN32 */
	strcpy(fileName,"Write file: ");
	len = strlen(fileName);
	if (!new_getstr(fileName,len,FF)) { strcpy(global_df->err_message, DASHES); return(22); }
#endif /* !_MAC_CODE && !_WIN32*/
	if (isfiledir(fileName+len)) {
		strcpy(global_df->err_message, "+Error writting; Change the file name.");
		return(22);
	}
	if (i > -2) {
		if (!SaveToFile(fileName+len, TRUE)) {
			unlink(fileName+len);
			return(22);
		}
	}

	if (global_df->isTempFile != 1) {
#ifdef _MAC_CODE
		CFStringRef	theStr;

		if ((windProc=WindowProcs(global_df->wind)))
			ChangeWindowsMenuItem(windProc->wname, FALSE);
#endif
		strcpy(global_df->fileName, fileName+len);
#ifdef _MAC_CODE
		theStr = my_CFStringCreateWithBytes(global_df->fileName);
		if (theStr != NULL) {
			SetWindowTitleWithCFString(global_df->wind, theStr);
			CFRelease(theStr);
		}
		if (windProc) {
			if (strcmp(windProc->wname, global_df->fileName))
				global_df->RowLimit = 0;
			strcpy(windProc->wname, global_df->fileName);
			ChangeWindowsMenuItem(windProc->wname, TRUE);
		}
#elif defined(_WIN32)
		GlobalDoc->SetPathName(szFile, TRUE);
		GlobalDoc->SetTitle(szFileTitle);
		GlobalDoc->SetModifiedFlag(FALSE);
#endif
	}
	sprintf(global_df->err_message, "-File \"%s\" written.", fileName+len);
	s = strrchr(fileName+len, PATHDELIMCHR);
	if (s == NULL)
		s = fileName+len;
	else
		s++;
	if (!strcmp(s, ALIAS_FILE_D) || !strcmp(s, ALIAS_FILE_U)) {
		re_readAliases();
	}
	global_df->DataChanged = '\0';
	ResetUndos();
	return(22);
}

char GetSpeakerNames(unCH *st, int num, unsigned long size) {
	if (global_df->SpeakerNames[num] == NULL)
		return(FALSE);
	strncpy(st, global_df->SpeakerNames[num], size);
	st[size] = EOS;
	return(TRUE);
}

char AllocConstString(char *st, int sp) {
	int i;

	if (*st != EOS) {
		if (ConstString[sp] != NULL)
			free(ConstString[sp]);
		ConstString[sp] = (char *)malloc((size_t)(strlen(st)+1));
		if (ConstString[sp] == NULL) {
			if (global_df)
				strcpy(global_df->err_message, "+Can not allocate more memory.");
			else
				do_warning("Can not allocate more memory.", 0);
			return(FALSE);
		}
		strcpy(ConstString[sp], st);
		for (i=0; ConstString[sp][i]; i++) {
			if (ConstString[sp][i] == '\n' || ConstString[sp][i] == '\r') {
				if (global_df->ChatMode)
					ConstString[sp][i] = SNL_C;
				else
					ConstString[sp][i] = NL_C;
			}
		}
	} else if (ConstString[sp] != NULL) {
		free(ConstString[sp]);
		ConstString[sp] = NULL;
	}
	return(TRUE);
}

void AddConstString(int sp) {
	int i;
	char DisplayAll = FALSE;

	if (DeleteChank(1)) {
		SaveUndoState(FALSE);
		if (ConstString[sp] != NULL) {
			for (i=0; ConstString[sp][i]; i++) {
				if (ConstString[sp][i] == SNL_C || ConstString[sp][i] == NL_C)
					DisplayAll = TRUE;
			}
			global_df->FreqCount++;
			u_strcpy(templineW, ConstString[sp], UTTLINELEN);
			AddText(NULL, templineW[0], 0, 1L);
			if (templineW[1]) {
				if (global_df->UndoList->key != INSTKEY)
					SaveUndoState(FALSE);
				AddText(templineW+1,EOS,DisplayAll,(long)strlen(templineW+1));
			}
		} else
			RemoveLastUndo();
	}
}

char AllocSpeakerNames(unCH *st, int sp) {
	if (*st != EOS) {
		if (global_df->SpeakerNames[sp] != NULL)
			free(global_df->SpeakerNames[sp]);
		global_df->SpeakerNames[sp] = (unCH *)malloc((strlen(st)+1)*sizeof(unCH));
		if (global_df->SpeakerNames[sp] == NULL) {
			strcpy(global_df->err_message, "+Can not allocate more memory.");
			return(FALSE);
		}
		strcpy(global_df->SpeakerNames[sp], st);
	} else if (global_df->SpeakerNames[sp] != NULL) {
		free(global_df->SpeakerNames[sp]);
		global_df->SpeakerNames[sp] = NULL;
	}
	return(TRUE);
}

/* 12-04-04
void AddSpeakerNames(int sp) {
	long  i;
	short doSearch;
	LINE *line;

	if (DeleteChank(1)) {
		SaveUndoState(FALSE);
		if (global_df->SpeakerNames[sp] != NULL) {
			char t = global_df->ChatMode;
			i = 0L;
			doSearch = 1;
			if (global_df->row_txt == global_df->cur_line) {
				line = global_df->head_row->next_char;
				if (global_df->head_row_len==0L || (global_df->head_row_len==1L && line->c==NL_C)) {
					doSearch = 0;
					global_df->ChatMode = FALSE;
				} else if ((line->c== '*' && global_df->head_row_len== 1L) ||
						   (line->c=='*' && line->next_char->c==NL_C && global_df->head_row_len==2L) ||
						   (line->c=='*' && line->next_char->c==SNL_C && global_df->head_row_len==2L)) {
					doSearch = 0;
					i = 1L;
					global_df->ChatMode = FALSE;
				}
			} else {
				if (global_df->row_txt->line[0] == EOS || global_df->row_txt->line[0] == NL_C ||
					global_df->row_txt->line[0] == SNL_C) {
					doSearch = 0;
					global_df->ChatMode = FALSE;
				} else if (global_df->row_txt->line[0] == '*' &&
						(global_df->row_txt->line[1] == NL_C || global_df->row_txt->line[1] == SNL_C ||
						 global_df->row_txt->line[1] == EOS)) {
					doSearch = 0;
					i = 1L;
					global_df->ChatMode = FALSE;
				}
			}

			if (doSearch) {
				if (global_df->row_txt == global_df->cur_line) {
					line = global_df->head_row->next_char;
					if (global_df->head_row_len >= 2L) {
						if ((line->c== '*' && line->next_char->c == ':')) {
							doSearch = -1;
						}
					}
				} else {
					if (global_df->row_txt->line[0] == '*' && global_df->row_txt->line[1] == ':') {
						doSearch = -1;
					}
				}
			}
			if (doSearch == 1) {
				global_df->redisplay = 0;
				while (!AtBotEnd(global_df->row_txt,global_df->tail_text, FALSE)) {
					if (AtBotEnd(global_df->row_txt, global_df->cur_line, FALSE)) {
						if (global_df->head_row->next_char->c == '*' ||
							global_df->head_row->next_char->c == '@' || 
							global_df->head_row->next_char == global_df->tail_row)
							break;
					} else {
						if (*global_df->row_txt->next_row->line == '*' ||
							*global_df->row_txt->next_row->line == '@')
							break;
					}
					MoveDown(-1);
				}
				global_df->redisplay = 1;

				if (!AtBotEnd(global_df->row_txt,global_df->tail_text, FALSE)) {
					if (AtBotEnd(global_df->row_txt, global_df->cur_line, FALSE)) {
						line = global_df->head_row->next_char;
						if (global_df->head_row_len >= 2L) {
							if ((line->c== '*' && line->next_char->c == ':' && global_df->head_row_len== 2L)) {
								MoveDown(-1);
								doSearch = -1;
							}
						}
					} else {
						if (global_df->row_txt->next_row->line[0] == '*' && 
									global_df->row_txt->next_row->line[1] == ':') {
							MoveDown(-1);
							doSearch = -1;
						}
					}
				}
			}

			if (doSearch == -1) {
				BeginningOfLine(-1);
				MoveRight(1);
				strcpy(templine1, global_df->SpeakerNames[sp]+1);
				i = strlen(templine1) - 1;
				while (isSpace(templine1[i]) || templine1[i] == ':') i--;
				templine1[++i] = EOS;
				SaveUndoState(FALSE);
				AddText(templine1, EOS, 0, (long)strlen(templine1));
				SaveUndoState(FALSE);
				MoveRight(1);
				MoveRight(1);
			} else
				AddCodeTier(global_df->SpeakerNames[sp]+i, FALSE);
			global_df->ChatMode = t;
			AddText(global_df->SpeakerNames[sp], EOS, 0, (long)strlen(global_df->SpeakerNames[sp]));
		} else
			RemoveLastUndo();
	}
}
*/
/* 02-08-05
void AddSpeakerNames(int sp) {
//	if (DeleteChank(1)) {
		SaveUndoState(FALSE);
		if (global_df->SpeakerNames[sp] != NULL) {
			long isRemTab = 0L;

			if (global_df->row_txt == global_df->cur_line) {
				LINE *line;

				line = global_df->col_txt;
				global_df->redisplay = 0;
				if (global_df->head_row_len >= 3L && line != global_df->tail_row && line->c == '*' && 
													 line->next_char != global_df->tail_row && line->next_char->c== ':' && 
													 line->next_char->next_char != global_df->tail_row && line->next_char->next_char->c== '\t') {
					SaveUndoState(FALSE); DeleteNextChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
				} else if (global_df->head_row_len >= 3L && line->prev_char != global_df->head_row && line->prev_char->c == '*' && 
													 line != global_df->tail_row && line->c== ':' && 
													 line->next_char != global_df->tail_row && line->next_char->c== '\t') {
					SaveUndoState(FALSE); DeletePrevChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
				} else if (global_df->head_row_len >= 3L && line->prev_char != global_df->head_row && line->prev_char->c== ':' &&
													 line->prev_char->prev_char != global_df->head_row && line->prev_char->prev_char->c == '*' && 
													 line != global_df->tail_row && line->c== '\t') {
					SaveUndoState(FALSE); DeletePrevChar(-1);
					SaveUndoState(FALSE); DeletePrevChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
				} else if (line->c == '\t')
					isRemTab = strlen(global_df->SpeakerNames[sp]) - 1;
				global_df->redisplay = 1;
			} else {
				unCH *line;

				line = global_df->row_txt->line;
				global_df->redisplay = 0;
				if (global_df->col_chr == 0 && line[global_df->col_chr] == '*' && line[global_df->col_chr+1] == ':' && line[global_df->col_chr+2] == '\t') {
					SaveUndoState(FALSE); DeleteNextChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
				} else if (global_df->col_chr == 1 && line[global_df->col_chr-1] == '*' && line[global_df->col_chr] == ':' && line[global_df->col_chr+1] == '\t') {
					SaveUndoState(FALSE); DeletePrevChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
				} else if (global_df->col_chr == 2 && line[global_df->col_chr-2] == '*' && line[global_df->col_chr-1] == ':' && line[global_df->col_chr] == '\t') {
					SaveUndoState(FALSE); DeletePrevChar(-1);
					SaveUndoState(FALSE); DeletePrevChar(-1);
					SaveUndoState(FALSE); DeleteNextChar(-1);
				} else if (line[global_df->col_chr] == '\t')
					isRemTab = strlen(global_df->SpeakerNames[sp]) - 1;
				global_df->redisplay = 1;
			}

			if (isRemTab != 0L) {
				global_df->SpeakerNames[sp][isRemTab] = EOS;
			}
			SaveUndoState(FALSE);
			AddText(global_df->SpeakerNames[sp], EOS, 0, (long)strlen(global_df->SpeakerNames[sp]));
			if (isRemTab != 0L) {
				global_df->SpeakerNames[sp][isRemTab] = '\t';
			}
		} else
			RemoveLastUndo();
//	}
}
*/
static void removeSpeakerTag(void) {
	char isColFound = FALSE;
	int  cnt;
	LINE *line;

	cnt = 0;
	line = global_df->col_txt;
	if (global_df->head_row_len >= 2L && line != global_df->tail_row && line->c == '*') {
		for (; line != global_df->tail_row && line->c != ':'; line=line->next_char) {
			templine[cnt++] = line->c;
		}
		if (line != global_df->tail_row && line->c == ':') {
			templine[cnt++] = line->c;
			line = line->next_char;
			isColFound = TRUE;
		}
		if (line != global_df->tail_row && isSpace(line->c)) {
			templine[cnt++] = line->c;
		}
		templine[cnt] = EOS;
	}

	if (isColFound) {
		SaveUndoState(FALSE);
		global_df->UndoList->key = DELTRPT;
		if ((global_df->UndoList->str=(unCH *)malloc((cnt+1L)*sizeof(unCH))) == NULL)
			mem_err(TRUE, global_df);
		strcpy(global_df->UndoList->str, templine);

		global_df->redisplay = 0;

		while (global_df->col_txt != global_df->tail_row && global_df->col_txt->c != ':')
			DeleteNextChar(-1);
		if (global_df->col_txt != global_df->tail_row && global_df->col_txt->c == ':')
			DeleteNextChar(-1);
		if (global_df->col_txt != global_df->tail_row && isSpace(global_df->col_txt->c))
			DeleteNextChar(-1);

		global_df->redisplay = 1;
	} else if (isSpace(global_df->col_txt->c)) {
		cnt = 0;
		if (line != global_df->tail_row && isSpace(line->c)) {
			templine[cnt++] = line->c;
		}
		templine[cnt] = EOS;

		SaveUndoState(FALSE);
		global_df->UndoList->key = DELTRPT;
		if ((global_df->UndoList->str=(unCH *)malloc((cnt+1L)*sizeof(unCH))) == NULL)
			mem_err(TRUE, global_df);
		strcpy(global_df->UndoList->str, templine);

		global_df->redisplay = 0;
		if (global_df->col_txt != global_df->tail_row && isSpace(global_df->col_txt->c))
			DeleteNextChar(-1);
		global_df->redisplay = 1;
	}
}

void AddSpeakerNames(int sp) {
	if (global_df->SpeakerNames[sp] != NULL) {
		if (global_df->row_win2 == 0L && global_df->col_win2 == -2) {
			ChangeCurLine();
			BeginningOfLine(-1);
			removeSpeakerTag();
			SaveUndoState(FALSE);
			global_df->UndoList->key = INSTRPT;
			AddText(global_df->SpeakerNames[sp], EOS, 0, (long)strlen(global_df->SpeakerNames[sp]));
		} else {
			char isRefresh;
			char tRedisplay;
			long cnt;

			isRefresh = FALSE;
			cnt = global_df->row_win2;
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
			if (cnt < 0L) {
				tRedisplay = global_df->redisplay;
				global_df->redisplay = FALSE;
				MoveUp(-1);
				for (; cnt && !AtTopEnd(global_df->row_txt,global_df->head_text,FALSE); cnt++) {
					if (global_df->row_txt->line[0] == '*') {
						SaveUndoState(FALSE);
						global_df->UndoList->key = MOVERPT;
						ChangeCurLine();
						BeginningOfLine(-1);
						removeSpeakerTag();
						SaveUndoState(FALSE);
						global_df->UndoList->key = INSTRPT;
						AddText(global_df->SpeakerNames[sp], EOS, -1, (long)strlen(global_df->SpeakerNames[sp]));
						isRefresh = TRUE;
					}
					SaveUndoState(FALSE);
					global_df->UndoList->key = MOVERPT;
					MoveUp(-1);
				}
				global_df->redisplay = tRedisplay;
				if (isRefresh)
					DisplayTextWindow(NULL, 1);
			} else if (cnt > 0L) {
				tRedisplay = global_df->redisplay;
				global_df->redisplay = FALSE;
				MoveDown(-1);
				for (; cnt && !AtBotEnd(global_df->row_txt,global_df->tail_text,FALSE); cnt--) {
					if (global_df->row_txt->line[0] == '*') {
						SaveUndoState(FALSE);
						global_df->UndoList->key = MOVERPT;
						ChangeCurLine();
						BeginningOfLine(-1);
						removeSpeakerTag();
						SaveUndoState(FALSE);
						global_df->UndoList->key = INSTRPT;
						AddText(global_df->SpeakerNames[sp], EOS, -1, (long)strlen(global_df->SpeakerNames[sp]));
						isRefresh = TRUE;
					}
					SaveUndoState(FALSE);
					global_df->UndoList->key = MOVERPT;
					MoveDown(-1);
				}
				global_df->redisplay = tRedisplay;
				if (isRefresh)
					DisplayTextWindow(NULL, 1);
			} else {
				ChangeCurLine();
				BeginningOfLine(-1);
				removeSpeakerTag();
				SaveUndoState(FALSE);
				global_df->UndoList->key = INSTRPT;
				AddText(global_df->SpeakerNames[sp], EOS, 0, (long)strlen(global_df->SpeakerNames[sp]));
			}
		}
	} else
		RemoveLastUndo();
}

long roundUp(double num) {
	long t;

	t = (long)num;
	num = num - t;
	if (num > 0.5)
		t++;
	return(t);
}

FONTINFO *GetTextFont(myFInfo *df) {
	ROWS *t;
	FONTINFO *font[10];
	int size[10], occ[10];
	int index, j, max, pos;

	index = 0;
	for (t=df->head_text->next_row; t != df->tail_text; t=t->next_row) {
		for (j=0; j < index && size[j] != t->Font.FHeight; j++) ;
		if (j == index) {
			if (index < 10) {
				size[index] = t->Font.FHeight;
				occ[index] = 1;
				font[index] = &t->Font;
				index++;
			}
		} else
			occ[j]++;
	}
	if (index == 0)
		return(NULL);
	max = 0;
	pos = 0;
	for (j=0; j < index; j++) {
		if (occ[j] > max) {
			max = occ[j];
			pos = j;
		}
	}
	return(font[pos]);
}

char init_windows(int com, char refresh, char isJustTextWin) {
	int numRows;

#if defined(_MAC_CODE) || defined(_WIN32)
	global_df->WinChange = FALSE;
#endif
	if (global_df->w1 != NULL) {
#if !defined(_MAC_CODE) && !defined(_WIN32)
		if (!isJustTextWin) {
			werase(global_df->w1);
			wrefresh(global_df->w1);
		}
#endif
		delwin(global_df->w1);
		global_df->w1 = NULL;
	}
	numRows = global_df->total_num_rows - global_df->CodeWinStart + MID_WIN_SIZE;
	global_df->total_num_rows = WindowPageLength(&numRows);
	global_df->num_of_cols = WindowPageWidth();
	if (com || global_df->EdWinSize < 1 || global_df->EdWinSize > global_df->total_num_rows - 2) {
		com = numRows;
		if (global_df->total_num_rows < 6 || global_df->NoCodes) 
			numRows = global_df->total_num_rows - 1 - MID_WIN_SIZE;
		else
			numRows = global_df->total_num_rows - com;
		if (!isJustTextWin)
			global_df->EdWinSize = numRows;
		global_df->CodeWinStart = global_df->EdWinSize + MID_WIN_SIZE;
	} else
		numRows = global_df->EdWinSize;
		/* #rows, #cols, upper left row, upper left col */
	global_df->w1 = newwin(numRows, global_df->num_of_cols, 0, 0, getNumberOffset());
	if (global_df->w1 == NULL)
		return(FALSE);
	
	global_df->w1->isUTF = global_df->isUTF;

	werase(global_df->w1);

#if defined(_MAC_CODE) || defined(_WIN32)
	global_df->RdW = global_df->w1;
#endif
	if (isJustTextWin)
		return(TRUE);

	if (global_df->w2 != NULL) {
		werase(global_df->w2);
		wrefresh(global_df->w2);
		delwin(global_df->w2);
		global_df->w2 = NULL;
	}
	global_df->w2 = newwin(COM_WIN_SIZE, global_df->num_of_cols, global_df->CodeWinStart, 0, 0);
	if (global_df->w2 == NULL) {
		delOneWin(global_df->w1);
		return(FALSE);
	}
//	global_df->w2->isUTF = global_df->isUTF;
	werase(global_df->w2);
	wrefresh(global_df->w2);
	if (global_df->wm != NULL) {
		werase(global_df->wm);
		wrefresh(global_df->wm);
		delwin(global_df->wm); 
		global_df->wm = NULL;
	}
	global_df->wm = newwin(2, global_df->num_of_cols, global_df->CodeWinStart-MID_WIN_SIZE, 0, 0);
	if (global_df->wm == NULL) {
		delOneWin(global_df->w1);
		delOneWin(global_df->w2);
		return(FALSE);
	}
//	global_df->wm->isUTF = global_df->isUTF;
	wstandout(global_df->wm);
	draw_mid_wm();
	FindMidWindow();
	DisplayTextWindow(NULL, refresh);
	global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
/*	if (!global_df->ChatMode) */
	if (CheckLeftCol(global_df->col_win))
		DisplayTextWindow(NULL, refresh);

	if (global_df->SoundWin) {
		SoundMode(1);
		SaveUndoState(FALSE);
		SoundMode(2);
	}
#if defined(_MAC_CODE) || defined(_WIN32)
	wmove(global_df->w1, global_df->row_win, global_df->col_win - global_df->LeftCol);
	if (refresh > 0)
		wrefresh(global_df->w1);
#else
	wmove(w1, (int)row_win, (int)(global_df->col_win-global_df->LeftCol));
	wrefresh(w1);
#endif

#if defined(_MAC_CODE) || defined(_WIN32)
	global_df->WinChange = TRUE;
#endif
	return(TRUE);
}

// check file extensions
char isCHATFile(FNType *fname) {
	register int i, j, k;
	unCH ext[16];

	j = strlen(fname) - 1;
	for (; j >= 0 && fname[j] != '.'; j--) ;
	if (j < 0)
		return(FALSE);

	if (fname[j] == '.') {
		ext[0] = towupper(fname[j+1]);
		ext[1] = towupper(fname[j+2]);
		ext[2] = towupper(fname[j+3]);
		if (ext[0] == 'C' && ext[1] == 'H' && (ext[2] == 'A' || iswdigit(ext[2])) )
			return('\001');
		else if ((ext[0] == 'D' && ext[1] == 'A' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
				 (ext[0] == 'C' && ext[1] == 'U' && (ext[2] == 'T' || iswdigit(ext[2])))
				)
			return('\003');
		else if ((ext[0] == 'C' && ext[1] == 'H' && (ext[2] == 'P' || iswdigit(ext[2]))) ||
				 (ext[0] == 'D' && ext[1] == 'A' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
				 (ext[0] == 'D' && ext[1] == 'E' && (ext[2] == 'L' || iswdigit(ext[2]))) ||
				 (ext[0] == 'D' && ext[1] == 'S' && (ext[2] == 'S' || iswdigit(ext[2]))) ||
				 (ext[0] == 'F' && ext[1] == 'I' && (ext[2] == 'X' || iswdigit(ext[2]))) ||
				 (ext[0] == 'F' && ext[1] == 'L' && (ext[2] == 'O' || iswdigit(ext[2]))) ||
				 (ext[0] == 'F' && ext[1] == 'X' && (ext[2] == 'B' || iswdigit(ext[2]))) ||
				 (ext[0] == 'G' && ext[1] == 'E' && (ext[2] == 'M' || iswdigit(ext[2]))) ||
				 (ext[0] == 'I' && ext[1] == 'D' && (ext[2] == 'N' || iswdigit(ext[2]))) ||
				 (ext[0] == 'I' && ext[1] == 'N' && (ext[2] == 'S' || iswdigit(ext[2]))) ||
				 (ext[0] == 'L' && ext[1] == 'O' && (ext[2] == 'W' || iswdigit(ext[2]))) ||
				 (ext[0] == 'M' && ext[1] == 'O' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
				 (ext[0] == 'P' && ext[1] == 'S' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
				 (ext[0] == 'S' && ext[1] == 'T' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
				 (ext[0] == 'T' && ext[1] == 'M' && (ext[2] == 'P' || iswdigit(ext[2]))) ||
				 (ext[0] == 'T' && ext[1] == 'O' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
				 (ext[0] == 'U' && ext[1] == 'T' && (ext[2] == 'F' || iswdigit(ext[2])))
				)
			return('\002');
		else if (ext[0] == 'C' && ext[1] == 'E' && ext[2] == 'X') {
			for (i=j-1; i > 0 && fname[i] != '.'; i--) ;
			if (i > 0 && j - i < 14) {
				k = 0;
				for (i++; i < j; i++)
					ext[k++] = towupper(fname[i]);
				ext[k] = EOS;
				if ((ext[0] == 'C' && ext[1] == 'H' && (ext[2] == 'P' || iswdigit(ext[2]))) ||
					(ext[0] == 'D' && ext[1] == 'A' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
					(ext[0] == 'D' && ext[1] == 'E' && (ext[2] == 'L' || iswdigit(ext[2]))) ||
					(ext[0] == 'D' && ext[1] == 'S' && (ext[2] == 'S' || iswdigit(ext[2]))) ||
					(ext[0] == 'F' && ext[1] == 'I' && (ext[2] == 'X' || iswdigit(ext[2]))) ||
					(ext[0] == 'F' && ext[1] == 'L' && (ext[2] == 'O' || iswdigit(ext[2]))) ||
					(ext[0] == 'F' && ext[1] == 'X' && (ext[2] == 'B' || iswdigit(ext[2]))) ||
					(ext[0] == 'G' && ext[1] == 'E' && (ext[2] == 'M' || iswdigit(ext[2]))) ||
					(ext[0] == 'I' && ext[1] == 'D' && (ext[2] == 'N' || iswdigit(ext[2]))) ||
					(ext[0] == 'I' && ext[1] == 'N' && (ext[2] == 'S' || iswdigit(ext[2]))) ||
					(ext[0] == 'L' && ext[1] == 'O' && (ext[2] == 'W' || iswdigit(ext[2]))) ||
					(ext[0] == 'M' && ext[1] == 'O' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
					(ext[0] == 'P' && ext[1] == 'S' && (ext[2] == 'T' || iswdigit(ext[2]))) ||
					(ext[0] == 'S' && ext[1] == 'T' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
					(ext[0] == 'T' && ext[1] == 'M' && (ext[2] == 'P' || iswdigit(ext[2]))) ||
					(ext[0] == 'T' && ext[1] == 'O' && (ext[2] == 'R' || iswdigit(ext[2]))) ||
					(ext[0] == 'U' && ext[1] == 'T' && (ext[2] == 'F' || iswdigit(ext[2]))) ||
					!strncmp(ext,"CHIP",4)  || !strncmp(ext,"DATE",4)  || !strncmp(ext,"KWAL",4)  ||
					!strncmp(ext,"SYNC",4)  ||
					!strncmp(ext,"DELIM",5) || !strncmp(ext,"CHSTR",5) || !strncmp(ext,"INDNT",5) ||
					!strncmp(ext,"COMBO",5) || !strncmp(ext,"FIXIT",5) || !strncmp(ext,"TORDR",5) ||
					!strncmp(ext,"MEDIA",5) ||!strncmp(ext,"IPCORE",5) ||
					!strncmp(ext,"LOWCAS",6)|| !strncmp(ext,"FXBLTS",6)|| !strncmp(ext,"CP2UTF",6)||
					!strncmp(ext,"MGRASP",6)|| !strncmp(ext,"PMORTM",6)||
					!strncmp(ext,"COMB",4)
				   )
					return('\002');
			} else
				return('\004');
		} else if (ext[0] == 'X' && ext[1] == 'L' && ext[2] == 'S') {
			for (i=j-1; i > 0 && fname[i] != '.'; i--) ;
			if (i > 0 && j - i < 14) {
				k = 0;
				for (i++; i < j; i++)
					ext[k++] = towupper(fname[i]);
				ext[k] = EOS;
				if (!strncmp(ext,"KWAL",4)  || !strncmp(ext,"KIDEVAL",3)|| !strncmp(ext,"MLT",3)  || !strncmp(ext,"MLU",3)   ||
					!strncmp(ext,"OUT",3)   || !strncmp(ext,"MRTBL",5) || !strncmp(ext,"TIMDUR",6) || !strncmp(ext,"VOCD",4) ||
					!strncmp(ext,"WDLEN",5) || !strncmp(ext,"FLUCALC",5) || !strncmp(ext,"SCRIPT",3))
					return('\005');
			}
		}
	}
	return(FALSE);
}

char isCEXFile(FNType *fname) {
	register int j;
	unCH ext[4];

	j = strlen(fname) - 1;
	for (; j >= 0 && fname[j] != '.'; j--) ;
	if (j < 0)
		return(FALSE);
	if (fname[j] == '.') {
		ext[0] = towupper(fname[j+1]);
		ext[1] = towupper(fname[j+2]);
		ext[2] = towupper(fname[j+3]);
		if (ext[0] == 'C' && ext[1] == 'E' && ext[2] == 'X')
			return('\001');
		else
			return(FALSE);
	} else
		return(FALSE);
}

char SetChatModeOfDatafileExt(FNType *fname, char isJustCheck) {
	char res;
	extern short DOSdata;

	res = isCHATFile(fname);
	if (res == '\002') {
		res = TRUE;
		if (!isJustCheck)
			DOSdata = 0;
	} else if (res == '\003' || res == '\005')
		res = FALSE;
	else if (res == '\004')
		res = TRUE;
	return(res);
}
// end check file extensions

unCH BigWBuf[UTTLINELEN+UTTLINELEN];

unCH *cl_T(const char *st) {
	strcpy(BigWBuf, st);
	return(BigWBuf);
}
/*
char my_isalnum(char c) {
	return((char)isalnum((int)c))
}

char my_isalpha(char c) {
	return((char)isalpha((int)c))
}

char my_isdigit(char c) {
	return((char)isdigit((int)c))
}

char my_isspace(char c) {
	return((char)isspace((int)c))
}

char my_islower(char c) {
	return((char)islower((int)c))
}

char my_isupper(char c) {
	return((char)isupper((int)c))
}

char my_tolower(char c) {
	return((char)tolower((int)c))
}

char my_toupper(char c) {
	return((char)toupper((int)c))
}
*/
