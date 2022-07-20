#include "ced.h"
#include "my_ctype.h"
#ifdef _WIN32 
#include "Clan2Doc.h"
#endif

FNType CODEFNAME[FNSize];
FNType STATEFNAME[FNSize];
char DefChatMode;
char DefCodingScheme;
char DefPriorityCodes;
char MakeBackupFile;
char StartInEditorMode;
char ShowPercentOfFile;
char *ConstString[10];
unCH NextTierName[80];
FNType CodesFName[FNSize], KeysFName[FNSize];
char ced_version[32];
short SearchFFlag;

void draw_mid_wm() {
	register int len;
	ROWS *tr1;
	unCH buf[ERRMESSAGELEN];

	if (global_df == NULL)
		return;
	if (global_df->wm == NULL) {
		if (global_df->err_message[0] == '+') {
			strcpy(ced_lineC, global_df->err_message+1);
			strcpy(global_df->err_message, DASHES);
			do_warning(ced_lineC, 0);
		}
		return;
	}
	if (ced_version[0] == EOS)
		mvwaddstr(global_df->wm,0L,0L,cl_T("CLAN "));
	else
		mvwaddstr(global_df->wm,0L,0L,cl_T(ced_version));
	if (global_df->EditorMode)
		waddstr(global_df->wm,cl_T("[E|"));
	else
		waddstr(global_df->wm,cl_T("[C|"));
	len = 7;
	if (!global_df->ChatMode) {
		waddstr(global_df->wm,cl_T("TEXT")); len += 4;
	} else {
		waddstr(global_df->wm,cl_T("CHAT")); len += 4;
	}
	if (rawTextInput) {
		waddstr(global_df->wm,cl_T("-RAW")); len += 4;
	}
	waddstr(global_df->wm,cl_T("] ")); len += 2;
	if (global_df->DataChanged == '\001')
		waddstr(global_df->wm,cl_T("* "));
	else
		waddstr(global_df->wm,cl_T("  "));
	len += 2;
	global_df->numberOfRows = global_df->wLineno;
	for (tr1=global_df->row_txt; !AtBotEnd(tr1,global_df->tail_text,FALSE); tr1=ToNextRow(tr1,FALSE))
		global_df->numberOfRows++;
	
	if (ShowPercentOfFile) {
		uS.sprintf(buf, cl_T("%ld%%"), (global_df->lineno * 100L) / global_df->numberOfRows);
	} else {
		uS.sprintf(buf, cl_T("%ld"), global_df->lineno);
	}
	waddstr(global_df->wm, buf);

	len += 5;
	if (global_df->err_message[0]) {
		if (global_df->err_message[0] == '+') {
			strcpy(ced_lineC, global_df->err_message+1);
			strcpy(global_df->err_message, DASHES);
			do_warning(ced_lineC, 0);
			wclrtoeol(global_df->wm, true);
			wrefresh(global_df->wm);
			wrefresh(global_df->w2);
		} else {
			u_strcpy(buf, global_df->err_message+1, ERRMESSAGELEN);
			len += strlen(buf);
			if (len > global_df->num_of_cols && !PlayingContSound && global_df->EditorMode) {
				wclrtoeol(global_df->wm, true);
				wmove(global_df->wm, 1, 0);
			}
			waddstr(global_df->wm,cl_T(" : "));
			waddstr(global_df->wm, buf);
			wclrtoeol(global_df->wm, true);
			wrefresh(global_df->wm);
			if (len > global_df->num_of_cols && !PlayingContSound && global_df->EditorMode) {
				wmove(global_df->wm, 1, 0);
				wclrtoeol(global_df->wm, true);
				if (global_df->SoundWin == NULL)
					touchwin(global_df->w2);
			}
		}
	} else {
		wclrtoeol(global_df->wm, true);
		wrefresh(global_df->wm);
		if (global_df->SoundWin == NULL)
			wrefresh(global_df->w2);
	}
}

void ced_getflag(char *f) {
	f++;
	switch(*f++) {
		case 'h':
		case 'H':
		case '?':
#if !defined(_MAC_CODE) && !defined(_WIN32)
			if (w1) return;
			puts("+a : always auto wrap long line");
			puts("+bN: set number of commands and words before auto-save");
			printf("+cF: specify codes file name (default: %s)\n", CODEFNAME);
			puts("-d : do NOT create backup file");
			puts("-e : do NOT start in an editor mode");
			puts("+h : show this help message");
			printf("+kF: specify key bindings file name (default: %s)\n", STATEFNAME);
			puts("+lN: re-order codes (0 = do not change order, 1 = advance to top, 2 = advance one step)");
			puts("+p : display current position as percentage of the whole file");
			puts("+sN: progam will make identical copies of codes across branches");
			puts("+tS: set next speaker name to S");
			puts("+w : input file is in CHAT format");
			puts("-w : input file is not in CHAT format");
			printf("+xS: disambiguate tier S (default %s)", DisTier);
			puts("+v : show version number");
	
			exit(0);
#endif
			break;
		case 'a':
			DefAutoWrap = TRUE;
			break;
		case 'f':
			break;
		case 'g':
			if (*(f-2) == '+') {
				if (*f == '1')
					SetGOption(1,1);
				else if (*f == '3')
					SetGOption(3,1);
			} else {
				if (*f == '1')
					SetGOption(1,0);
				else if (*f == '3')
					SetGOption(3,0);
			}
			break;
		case 'e':
			StartInEditorMode = FALSE;
			break;
		case 'p':
//			ShowPercentOfFile = TRUE;
			break;
		case 'l':
			DefPriorityCodes = (char)atoi(f);
			if (DefPriorityCodes > '\002') {
#if defined(_MAC_CODE) || defined(_WIN32)
				DefPriorityCodes = '\002';
#else
				if (w1) DefPriorityCodes = '\002';
				else {
					fprintf(stderr,"Legal numbers for \"+l\" option are 1 or 2.\n");
					exit(0);
				}
#endif
			}
			break;
		case 's':
			if (*f == '0') DefCodingScheme = EOS;
			else DefCodingScheme = '\001';
			break;
		case 'd':
			if (*(f-2) == '+')
				MakeBackupFile = TRUE;
			else
				MakeBackupFile = FALSE;
			break;
		case 'b':
			FreqCountLimit = atoi(f);
			break;
		case 'c':
			if (*f)
				uS.str2FNType(CODEFNAME, 0L, f);
			break;
		case 't':
			if (*f) {
				int i;
				if (*f != '*') strcpy(NextTierName, cl_T("*"));
				else NextTierName[0] = EOS;
				strcat(NextTierName, f);
				uS.uppercasestr(NextTierName, &dFnt, C_MBF);
				for (i=0; !isSpace(NextTierName[i]) && NextTierName[i] != '\n' && 
														NextTierName[i]!= EOS; i++) ;
				NextTierName[i] = EOS;
			}
			break;
		case 'k':
			if (*f)
				uS.str2FNType(STATEFNAME, 0L, f);
			break;
		case 'x':
			if (*f) {
				strncpy(DisTier, f, 50);
				DisTier[50] = EOS;
				uS.uppercasestr(DisTier, &dFnt, C_MBF);
			}
			break;
		case 'w':
/* 12-4-99
			if (*(f-2) == '+') {
				DefChatMode = 1;
			} else DefChatMode = 0;
*/
			DefChatMode = 2;
			break;
		default:
#if !defined(_MAC_CODE) && !defined(_WIN32)
			if (!w1) {
				fprintf(stderr,"Invalid option: %s.\n", f-2);
				exit(0);
			}
#endif
			break;
	}
}
/* 2009-10-13
static void setTextFont(ROWS *tr1, 	short *fID, short *FHeight) {
#ifdef _MAC_CODE
	char  fontName[256];

	if (global_df != NULL && global_df->isUTF) {
		if (*FHeight == 0) {
			strcpy(fontName, defUniFontName);
			if (!GetFontNumber(fontName, fID))
				return;
		}
		tr1->Font.FName = *fID;
		tr1->Font.FSize = defUniFontSize;
		tr1->Font.CharSet = my_FontToScript(tr1->Font.FName, 0);
		tr1->Font.Encod = tr1->Font.CharSet;
		if (*FHeight == 0)
			*FHeight = GetFontHeight(&tr1->Font, NULL, global_df->wind);
		tr1->Font.FHeight = *FHeight;
	} else {
		if (*FHeight == 0) {
			strcpy(fontName, "CAfont");
			if (!GetFontNumber(fontName, fID))
				return;
		}
		tr1->Font.FName = *fID;
		tr1->Font.FSize = 13L;
		tr1->Font.CharSet = 1;
		tr1->Font.Encod = NSCRIPT;
		if (*FHeight == 0)
			*FHeight = GetFontHeight(&tr1->Font, NULL, global_df->wind);
		tr1->Font.FHeight = *FHeight;
	}
#elif defined(_WIN32)
	if (global_df != NULL && global_df->isUTF) {
		strcpy(tr1->Font.FName, defUniFontName);
		tr1->Font.FSize = defUniFontSize;
		tr1->Font.CharSet = 0;
		tr1->Font.Encod = NSCRIPT;
		if (*FHeight == 0)
			*FHeight = GetFontHeight(&tr1->Font, NULL);
		tr1->Font.FHeight = *FHeight;
	} else {
		strcpy(tr1->Font.FName, "CAfont");
		tr1->Font.FSize = -15L;
		tr1->Font.CharSet = 0;
		tr1->Font.Encod = NSCRIPT;
		if (*FHeight == 0)
			*FHeight = GetFontHeight(&tr1->Font, NULL);
		tr1->Font.FHeight = *FHeight;
	}
#endif
}
*/
