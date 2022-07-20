#include "ced.h"
#include "c_clan.h"
#include "search.h"
#include "MMedia.h"
#include "my_ctype.h"
#include "ids.h"
/* // NO QT
#ifdef _WIN32
	#include <TextUtils.h>
#endif
*/
#if !defined(KEY_BACKSPACE)
#define KEY_BACKSPACE	8
#endif
#if !defined(KEY_ENTER)
#define KEY_ENTER	13	/* [ <-| ] */
#endif

#ifdef _WIN32 
#include "CedDlgs.h"
#include "Clan2Doc.h"
#endif

static int  ToUpperCase(int i);
static int  ToLowerCase(int i);
static int  CopyText(int i);
static int  PasteText(int i);
static int  FindText(int i);
static int  SpecialTextFile(int i);
static int  CloseCurrentWindow(int i);
static char IsRightCommand(int (*fn)(int));

static int  ColorKeywords(int i);

#define FUNC_INFO struct func_info_s
struct func_info_s {
    int (*func_ptr)(int);
    const char *func_name;
} all_func[] = {
	/* 0*/ { IllegalFunction,		"illegal-command"} ,
	/* 1*/ { SelfInsert,			"self-insert"} ,
	/* 2*/ { HandleCTRLX,			"CTRLX_Prefix"} ,
	/* 3*/ { HandleESC,				"ESC_Prefix"} ,
	/* 4*/ { EditMode,				"editor-coder-mode-toggle"} ,
	/* 5*/ { Space,					""} ,
	/* 6*/ { ExitCED,				"exit"} ,
	/* 7*/ { Replace,				"string-replace"} ,
	/* 8*/ { EndCurTierGotoNext,	"coding-finish-current-tier-goto-next"} ,
	/* 9*/ { ToTopLevel,			"coding-finish-current-code-or-tier"} ,
	/*10*/ { ToUpperCase,			"convert-regin-to-upper-case"} ,
	/*11*/ { ToLowerCase,			"convert-regin-to-lower-case"} ,
	/*12*/ { GotoLine,				"goto-line"} ,
	/*13*/ { DescribeKey,			"commands-describe-key"} ,
	/*14*/ { CursorsCommand,		""} ,
	/*15*/ { Apropos,				"help"} ,
	/*16*/ { MoveDown,				"cursor-down"} ,
	/*17*/ { MoveUp,				"cursor-up"} ,
	/*18*/ { MoveRight,				"cursor-right"} ,
	/*19*/ { MoveLeft,				"cursor-left"} ,
	/*20*/ { IllegalFunction,		""} ,
	/*21*/ { NewLine,				""} ,
	/*22*/ { clWriteFile,			"file-write-new-name"} ,
	/*23*/ { DeletePrevChar,		"delete-previous-character"} ,
	/*24*/ { MoveRightWord,			"cursor-forward-word"} ,
	/*25*/ { MoveLeftWord,			"cursor-backward-word"} ,
	/*26*/ { BeginningOfWindow,		"cursor-beginning-of-window"} ,
	/*27*/ { EndOfWindow,			"cursor-end-of-window"} ,
	/*28*/ { BeginningOfLine,		"cursor-beginning-of-line"} ,
	/*29*/ { EndOfLine,				"cursor-end-of-line"} ,
	/*30*/ { NextPage,				"cursor-next-page"} ,
	/*31*/ { PrevPage,				"cursor-previous-page"} ,
	/*32*/ { DeleteNextChar,		"delete-next-character"} ,
	/*33*/ { KillLine,				"delete-line"} ,
	/*34*/ { YankKilled,			"delete-restore"} ,
	/*35*/ { DeleteNextWord,		"delete-next-word"} ,
	/*36*/ { DeletePrevWord,		"delete-previous-word"} ,
	/*37*/ { GoToNextMainSpeaker,	"goto-next-speaker"} ,
	/*38*/ { SaveCurrentFile,		"file-save-current"} ,
	/*39*/ { GetCursorCode,			"coding-insert-highlighted-code-at-cursor"} ,
	/*40*/ { GetFakeCursorCode,		"coding-show-subcodes-under-cursor"} ,
	/*41*/ { SearchForward,			"search-forward"} ,
	/*42*/ { MoveCodeCursorUp,		"move-code-cursor-up"} ,
	/*43*/ { MoveCodeCursorDown,	"move-code-cursor-down"} ,
	/*44*/ { BeginningOfFile,		"cursor-beginning-of-file"} ,
	/*45*/ { EndOfFile,				"cursor-end-of-file"} ,
	/*46*/ { Undo,					"undo-previous-command"} ,
	/*47*/ { AproposFile,			"commands-store-list-to-file"} ,
	/*48*/ { RefreshScreen,			"redisplay-screen"} ,
	/*49*/ { PlayMedia,				"play-current-tier"} ,
	/*50*/ { SoundToTextSync,		"find-text-associated-with-sound"} ,
	/*51*/ { SearchReverse,			"search-reverse"} ,
	/*52*/ { MoveLineUp,			"window-move-one-line-up"} ,
	/*53*/ { MoveLineDown,			"window-move-one-line-down"} ,
	/*54*/ { WriteState,			"commands-save-keys-bindings"} ,
	/*55*/ { ExecCommand,			"commands-execute"} ,
	/*56*/ { PercentOfFile,			"display-percent-of-file"} ,
	/*57*/ { SetNextTierName,		"set-next-tier-name"} ,
	/*58*/ { ChatModeSet,			"text-mode"} ,
	/*59*/ { VisitFile,				"file-visit"} ,
	/*60*/ { AbortCommand,			"abort"} ,
	/*61*/ { DefConstString,		"macro-set-string"} ,
	/*62*/ { InsertConstString,		"macro-insert-string"} ,
	/*63*/ { SoundMode,				"sound-mode-transcribe"} ,
	/*64*/ { PasteText,				"edit_paste-buffer-text"} ,
	/*65*/ { SelectTiers,			"select-tiers"} ,
	/*66*/ { setIDs,				"id"} ,
	/*67*/ { GetNewCodes,			"file-get-new-codes"} ,
	/*68*/ { clGetVersion,			"version"} ,
	/*69*/ { MorDisambiguate,		"disambiguate-mor-tier"} ,
	/*70*/ { MoveCodeCursorLeft,	"move-code-cursor-left"} ,
	/*71*/ { PlayContMedia,			"play-from-now-on"} ,
	/*72*/ { Check,					"check-with-default-depfile"} ,
	/*73*/ { ShowParagraphMarks,	"show-paragraph-marks"} ,
	/*74*/ { EnterAscii,			"enter-ascii-code"} ,
	/*75*/ { SelectAll,				"select-all-text"} ,
	/*76*/ { FindSame,				"repeat-last-search"} ,
	/*77*/ { MoveCodeCursorRight,	"move-code-cursor-right"} ,
	/*78*/ { FindText,				"edit_find-text"} ,
	/*79*/ { CopyText,				"edit_copy-selected-text"} ,
	/*80*/ { MovieThumbNails,		"movie-thumb-nails"} ,
	/*81*/ { PlayStepForward,		"play-current-walker-step"} ,
	/*82*/ { CloseCurrentWindow,	"close-window"} ,
	/*83*/ { PlayStepBackward,		"play-current-walker-step"} ,
	/*84*/ { Redo,					"redo-undo-command"} ,
	/*85*/ { ColorKeywords,			"color-text-keywords"} ,// "#lxs-mode"} ,
	/*86*/ { IllegalFunction,		""} ,
	/*87*/ { IllegalFunction,		""} ,
	/*88*/ { MovieToTextSync,		"find-text-associated-with-movie"} ,
	/*89*/ { QuickTrinscribeMedia,	"transcribe-from-now-on"} ,
	/*90*/ { SpecialTextFile,		"#special-text-font-file"} ,
	/*91*/ { FindNextMediaTier,		"find-next-tier-with-media"} ,
	/*92*/ { MoveMediaEndRight,		"move-media-end-right"} ,
	/*93*/ { MoveMediaEndLeft,		"move-media-end-left"} ,
	/*94*/ { MoveMediaBegRight,		"move-media-beg-right"} ,
	/*95*/ { MoveMediaBegLeft,		"move-media-beg-left"} ,
	/*96*/ { FindPrevMediaTier,		"find-previous-tier-with-media"} ,
	/*97*/ { PlayContSkipMedia,		"play-from-now-on-minus-silence"} ,

	/* very last */ { NULL,			"\0"}
} ;

unsigned int globalWhichInput;

int InKey;
int isPlayS = 0;
int	F_key = 0;
int FreqCountLimit;
static FUNC_INFO *norm_key[524], *ESC_key[524], *X_key[128], *F_keys[50];

char isKeyEqualCommand(int key, int commNum) {
	register int i;

	if (key >= 524)
		return(FALSE);

	for (i=0; all_func[i].func_ptr != NULL; i++) {
		if (F_key) {
			if (norm_key[F_key]->func_ptr == all_func[i].func_ptr && commNum == i)
				return(TRUE);
		} else {
			if (norm_key[key]->func_ptr == all_func[i].func_ptr && commNum == i)
				return(TRUE);
		}
	}
	return(FALSE);
}

int GetNumberOfCommands(void) {
	int i;

	for (i=0; all_func[i].func_ptr != NULL; i++) ;
	return(i);
}

int getCommandNumber(char *cmd) {
	int  i;
	char fn[SPEAKERLEN];

	uS.uppercasestr(cmd, &dFnt, FALSE);
	for (i=0; all_func[i].func_ptr != NULL; i++) {
		strcpy(fn, all_func[i].func_name);
		uS.uppercasestr(fn, &dFnt, FALSE);
		if (!strcmp(cmd, fn)) {
			if (all_func[i].func_ptr != SelfInsert &&
				all_func[i].func_ptr != IllegalFunction &&
				all_func[i].func_ptr != HandleESC &&
				all_func[i].func_ptr != HandleCTRLX &&
				all_func[i].func_ptr != ExecCommand)
				return(i);
			else
				return(0);
		}
	}
	return(0);
}

static int MatchStr(const char *s1, const char *s2, char c) {
	int len;

	if ((len=strlen(s2)) == 0) return(0);
	if (c)
		return(uS.partwcmp(s1, s2));
	while (*s1 != EOS) {
		if (!strncmp(s2,s1,len)) return(1);
		s1++;
	}
	return(0);
}

void SortCommands(int *sl, const char *pat, char ch) {
	register int j;
	register int k;
	register int i;

	sl[0] = -1;
	for (i=0; all_func[i].func_ptr != NULL; i++) {
		if ((all_func[i].func_ptr != IllegalFunction &&
			 all_func[i].func_ptr != SelfInsert && pat[0] == EOS) ||
			 MatchStr(all_func[i].func_name,pat,ch)) {
			if (all_func[i].func_name[0] == EOS || 
				(all_func[i].func_name[0] == '#' && pat[0] == EOS))
				continue;
			for (j=0; sl[j] != -1; j++) {
				if(strcmp(all_func[sl[j]].func_name,all_func[i].func_name)>0){
					for (k=j; sl[k] != -1; k++) ;
					sl[k+1] = -1;
					for (; k > j; k--) sl[k] = sl[k-1];
					break;
				}
			}
			if (sl[j] == -1) {
				sl[j] = i;
				sl[j+1] = -1;
			} else sl[j] = i;
		}
	}
}

static void GetKeyName(char *s, const char *pref, int j) {
	if (j == 27)
		sprintf(s+strlen(s),"%sESC ", pref);
	else if (j < 32) {
		if (!strcmp(pref, "F"))
			sprintf(s+strlen(s),"%s%d ", pref, j);
		else
			sprintf(s+strlen(s),"%s^%c ", pref, j+64);
	} else if (j == (int)' ')
		sprintf(s+strlen(s),"%s[Space] ", pref);
	else if (j < 127)
		sprintf(s+strlen(s),"%s%c ", pref, j);
	else if (j == 127)
		sprintf(s+strlen(s),"%s[Del] ", pref);
	else if (j < 268)
		sprintf(s+strlen(s),"%sF%d ", pref, j-256);
	else if (j < 280)
		sprintf(s+strlen(s),"%sshift-F%d ", pref, j-268);
	else if (j < 291)
		sprintf(s+strlen(s),"%s^F%d ", pref, j-280);
	else if (j < 301)
		sprintf(s+strlen(s),"%sAlt-F%d ", pref, j-290);
	else if (j < 422) {
		switch(j) {
			case 403: sprintf(s+strlen(s),"%sCTRL-[Left Arrow] ", pref); break;
			case 404: sprintf(s+strlen(s),"%sCTRL-[Right Arrow] ", pref); break;
			case 405: sprintf(s+strlen(s),"%sCTRL-[End] ", pref); break;
			case 406: sprintf(s+strlen(s),"%sCTRL-[PgDn] ", pref); break;
			case 407: sprintf(s+strlen(s),"%sCTRL-[Home] ", pref); break;
			case 408: sprintf(s+strlen(s),"%sCTRL-[PgUp] ", pref); break;
			case 409: sprintf(s+strlen(s),"%s[Down Arrow] ", pref); break;
			case 410: sprintf(s+strlen(s),"%s[Up Arrow] ", pref); break;
			case 411: sprintf(s+strlen(s),"%s[Left Arrow] ", pref); break;
			case 412: sprintf(s+strlen(s),"%s[Right Arrow] ", pref); break;
			case 413: sprintf(s+strlen(s),"%s[Home] ", pref); break;
			case 414: sprintf(s+strlen(s),"%s[End] ", pref); break;
			case 415: sprintf(s+strlen(s),"%s[PgDn] ", pref); break;
			case 416: sprintf(s+strlen(s),"%s[PgUp] ", pref); break;
			case 417: sprintf(s+strlen(s),"%s[Delete] ", pref); break;
			case 418: sprintf(s+strlen(s),"%s[Insert] ", pref); break;
			case 419: sprintf(s+strlen(s),"%s[Prt Sc *] ", pref); break;
#if defined(_MAC_CODE)
			case 420: sprintf(s+strlen(s),"%sCMD-[Left Arrow] ", pref); break;
			case 421: sprintf(s+strlen(s),"%sCMD-[Right Arrow] ", pref); break;
#elif defined(_WIN32)
			case 420: sprintf(s+strlen(s),"%sCTRL-Shift-[Left Arrow] ", pref); break;
			case 421: sprintf(s+strlen(s),"%sCTRL-Shift-[Right Arrow] ", pref); break;
#endif
		}
	} else if (j < 457) {
#ifdef MSC
		strcat(s+strlen(s), pref);
		strcat(s+strlen(s), "Alt-");
		switch (j) {
			case KEY_ALT_1: strcat(s+strlen(s),"1"); break;
			case KEY_ALT_2: strcat(s+strlen(s),"2"); break;
			case KEY_ALT_3: strcat(s+strlen(s),"3"); break;
			case KEY_ALT_4: strcat(s+strlen(s),"4"); break;
			case KEY_ALT_5: strcat(s+strlen(s),"5"); break;
			case KEY_ALT_6: strcat(s+strlen(s),"6"); break;
			case KEY_ALT_7: strcat(s+strlen(s),"7"); break;
			case KEY_ALT_8: strcat(s+strlen(s),"8"); break;
			case KEY_ALT_9: strcat(s+strlen(s),"9"); break;
			case KEY_ALT_0: strcat(s+strlen(s),"0"); break;
			case KEY_ALT_DASH:strcat(s+strlen(s),"DASH");break;
			case KEY_ALT_EQU: strcat(s+strlen(s),"="); break;
			case KEY_ALT_Q: strcat(s+strlen(s),"Q"); break;
			case KEY_ALT_W: strcat(s+strlen(s),"W"); break;
			case KEY_ALT_E: strcat(s+strlen(s),"E"); break;
			case KEY_ALT_R: strcat(s+strlen(s),"R"); break;
			case KEY_ALT_T: strcat(s+strlen(s),"T"); break;
			case KEY_ALT_Y: strcat(s+strlen(s),"Y"); break;
			case KEY_ALT_U: strcat(s+strlen(s),"U"); break;
			case KEY_ALT_I: strcat(s+strlen(s),"I"); break;
			case KEY_ALT_O: strcat(s+strlen(s),"O"); break;
			case KEY_ALT_P: strcat(s+strlen(s),"P"); break;
			case KEY_ALT_A: strcat(s+strlen(s),"A"); break;
			case KEY_ALT_S: strcat(s+strlen(s),"S"); break;
			case KEY_ALT_D: strcat(s+strlen(s),"D"); break;
			case KEY_ALT_F: strcat(s+strlen(s),"F"); break;
			case KEY_ALT_G: strcat(s+strlen(s),"G"); break;
			case KEY_ALT_H: strcat(s+strlen(s),"H"); break;
			case KEY_ALT_J: strcat(s+strlen(s),"J"); break;
			case KEY_ALT_K: strcat(s+strlen(s),"K"); break;
			case KEY_ALT_L: strcat(s+strlen(s),"L"); break;
			case KEY_ALT_Z: strcat(s+strlen(s),"Z"); break;
			case KEY_ALT_X: strcat(s+strlen(s),"X"); break;
			case KEY_ALT_C: strcat(s+strlen(s),"C"); break;
			case KEY_ALT_V: strcat(s+strlen(s),"V"); break;
			case KEY_ALT_B: strcat(s+strlen(s),"B"); break;

			case KEY_ALT_N: strcat(s+strlen(s),"N"); break;
			case KEY_ALT_M: strcat(s+strlen(s),"M"); break;
		}
		strcat(s+strlen(s), " ");
#endif
	}
}

void AddKeysToCommand(unsigned int cm, char *st) {
	register int j;

	strcpy(st,all_func[cm].func_name);
	strcat(st, ":  ");

/*
	if (all_func[cm].func_ptr == EndCurTierGotoNext) {
		sprintf(st+strlen(st),"<Coder mode only:> f F ");
	} else if (all_func[cm].func_ptr == GetCursorCode) {
		sprintf(st+strlen(st),"<Coder mode only:> j J ");
	}
*/
	for (j=0; j < 523; j++) {
		if (&all_func[cm] == norm_key[j]) {
			if (strlen(st)+3 > 77) { strcat(st,"..."); goto cont; }
			else GetKeyName(st,"",j);
		}
	}
	for (j=0; j < 523; j++) {
		if (&all_func[cm] == ESC_key[j]) {
			if (strlen(st)+7 > 77) { strcat(st,"..."); goto cont; }
			else GetKeyName(st,"ESC-",j);
		}
	}
	for (j=0; j < 128; j++) {
		if (&all_func[cm] == X_key[j]) {
			if (strlen(st)+6 > 77) { strcat(st,"..."); break; }
			else GetKeyName(st,"^X-",j);
		}
	}
	for (j=0; j < 50; j++) {
		if (&all_func[cm] == F_keys[j]) {
			if (strlen(st)+6 > 77) { strcat(st,"..."); break; }
			else GetKeyName(st,"F",j);
		}
	}
cont:
	for (j=strlen(st)-1; st[j] == ' ' && j >= 0; j--) ;
	st[++j] = EOS;
}

static int RowCheck(WINDOW *w, int row, char en, int *ch) {
#if !defined(_WIN32)
	int c, nr;

#if defined(_MAC_CODE)
	nr = w->num_rows;
#else
	if (*ch) nr =global_df-> EdWinSize;
	else nr = global_df->total_num_rows;
#endif
	if (row >= nr-1 || en) {
		wstandout(w);
		if (en)
			mvwaddstr(w,row,0,cl_T("--Finish--"));
		else
			mvwaddstr(w,row,0,cl_T("--MORE--"));
		wclrtoeol(w, true);
		wrefresh(w);
		c = wgetch(w);
#ifdef VT200
		c = VT200key(c,w);
#endif
		wstandend(w);
		werase(w);
		if (c != ' ' && *ch) { row = -1; *ch = c; }
		else if (c == 'q' || c == 'Q' || c == CTRL('G')) row = -1;
		else row = 0;
	}
#endif /* !defined(_WIN32) */
	return(row);
}

int DisplayCommands(WINDOW *w, char *st, char ch) {
	int sl[256];
	char s[SPEAKERLEN], stt[SPEAKERLEN];
	char found = FALSE;
	int c;
	int i, j, row = 0;

#if defined(_MAC_CODE)
	if (ch == '\002' && *st == EOS)
		return(0);
#endif
	c = (int)ch;
	stt[0] = EOS;
	SortCommands(sl,st,ch);
	if (sl[0] != -1)
		found = TRUE;
	for (i=0; sl[i] != -1; i++) {
		AddKeysToCommand(sl[i], s);
		if (ch == '\002') {
			if (stt[0] == EOS) 
				for (j=0; (stt[j]=s[j]) != ':' && stt[j]; j++) ;
			else
				for (j=0; stt[j] == s[j] && stt[j]; j++) ;
			stt[j] = EOS;
		} else {
			strcpy(templine4, s);
			mvwaddstr(w,row,0,templine4);
			wclrtoeol(w, true);
			if ((row=RowCheck(w,++row,FALSE,&c)) == -1)
				break;
		}
	}
	if (ch != '\002') {
		if (!found) {
			mvwaddstr(w,row,0,cl_T("NO MATCH FOUND!"));
			wclrtoeol(w, true);
			row++;
		}
		if (row > 0 || !found)
			row=RowCheck(w,row,TRUE,&c);
		if (ch == '\001')
			return(c);
	} else
		if (stt[0] != EOS) strcpy(st, stt);
	return(0);
}

static int ReadState(FNType *fname, FNType *cname) {
	register int i;
	register int p1;
	register int p2;
	unsigned long cnt;
	FILE *fp;
	char buf[SPEAKERLEN];

	fp = NULL;
	if (*cname && (fp=fopen(cname,"rb")) != NULL)
		fname = cname;
	else if (fname != NULL && *fname)
		fp = fopen(fname,"rb");
	if (fp != NULL) {
		last_cr_char = 0;
		if (fgets_ced(buf, SPEAKERLEN, fp, &cnt) == NULL)
			return(FALSE);
		else {
			if (strcmp(buf, "@#$lxs$#@sxl@#$\n"))
				return(FALSE);
		}
		while (fgets_ced(buf, SPEAKERLEN, fp, &cnt)) {
			p1 = atoi(buf+1);
			for (i=1; isdigit(buf[i]); i++) ;
			p2 = atoi(buf+i+1);
			if (*buf == 'N') norm_key[p1] = (FUNC_INFO *)&all_func[p2];
			else if (*buf == 'E') ESC_key[p1] = (FUNC_INFO *)&all_func[p2];
			else if (*buf == 'X') X_key[p1] = (FUNC_INFO *)&all_func[p2];
			else if (*buf == 'F') F_keys[p1] = (FUNC_INFO *)&all_func[p2];
#if !defined(_MAC_CODE) && !defined(_WIN32)
//			else if (*buf == 'P') ShowPercentOfFile = TRUE;
			else if (*buf == 'W') StartInEditorMode = FALSE;
#endif
		}
		fclose(fp);
		return(TRUE);
	} else
		return(FALSE);
}

void init_keys(FNType *fname, FNType *cname) {
    int i;

#if defined(_MAC_CODE) || defined(_WIN32)
    for (i=0; i < 32; i++)   norm_key[i] = (FUNC_INFO *)&all_func[1];
#else
    for (i=0; i < 32; i++)   norm_key[i] = (FUNC_INFO *)&all_func[0];
#endif
    for (i=32; i < 524; i++) norm_key[i] = (FUNC_INFO *)&all_func[1];
    for (i=0; i < 524; i++) ESC_key[i] = (FUNC_INFO *)&all_func[0];
    for (i=0; i < 128; i++) X_key[i] = (FUNC_INFO *)&all_func[0];
    for (i=0; i < 50; i++) F_keys[i] = (FUNC_INFO *)&all_func[0];
    if (ReadState(fname, cname))
    	return;

    norm_key[CTRL('I')]		= (FUNC_INFO *)&all_func[1];
    norm_key[CTRL('X')]		= (FUNC_INFO *)&all_func[2];
    norm_key[27/*CTRL('[')*/]= (FUNC_INFO *)&all_func[3];
    norm_key[' ']			= (FUNC_INFO *)&all_func[5];
    norm_key['\t']			= (FUNC_INFO *)&all_func[5];
    norm_key[CTRL('T')]		= (FUNC_INFO *)&all_func[8];
    norm_key[CTRL('U')]		= (FUNC_INFO *)&all_func[10];
    norm_key[CTRL('L')]		= (FUNC_INFO *)&all_func[12];
    norm_key[CTRL('N')]		= (FUNC_INFO *)&all_func[16];
    norm_key[CTRL('P')]		= (FUNC_INFO *)&all_func[17];
    norm_key[CTRL('F')]		= (FUNC_INFO *)&all_func[18];
    norm_key[CTRL('B')]		= (FUNC_INFO *)&all_func[19];
    norm_key[KEY_ENTER]		= (FUNC_INFO *)&all_func[21];
    norm_key[KEY_BACKSPACE]	= (FUNC_INFO *)&all_func[23];
    norm_key[CTRL('A')]		= (FUNC_INFO *)&all_func[28];
    norm_key[CTRL('E')]		= (FUNC_INFO *)&all_func[29];
    norm_key[CTRL('V')]		= (FUNC_INFO *)&all_func[30];
    norm_key[CTRL('D')]		= (FUNC_INFO *)&all_func[32];
    norm_key[CTRL('K')]		= (FUNC_INFO *)&all_func[33];
    norm_key[CTRL('Y')]		= (FUNC_INFO *)&all_func[34];
    norm_key[CTRL('C')]		= (FUNC_INFO *)&all_func[39];
    norm_key[CTRL('S')]		= (FUNC_INFO *)&all_func[41];
    norm_key['-']			= (FUNC_INFO *)&all_func[42];
    norm_key['+']			= (FUNC_INFO *)&all_func[43];
    norm_key['=']			= (FUNC_INFO *)&all_func[43];
    norm_key[CTRL('R')]		= (FUNC_INFO *)&all_func[51];
    norm_key[CTRL('Q')]		= (FUNC_INFO *)&all_func[52];
    norm_key[CTRL('Z')]		= (FUNC_INFO *)&all_func[53];
    norm_key[CTRL('G')]		= (FUNC_INFO *)&all_func[60];
    norm_key[CTRL('W')]		= (FUNC_INFO *)&all_func[62];

    F_keys[4]				= (FUNC_INFO *)&all_func[49];
    F_keys[5]				= (FUNC_INFO *)&all_func[89];
    F_keys[6]				= (FUNC_INFO *)&all_func[96];
    F_keys[7]				= (FUNC_INFO *)&all_func[83];
    F_keys[8]				= (FUNC_INFO *)&all_func[91];
    F_keys[9]				= (FUNC_INFO *)&all_func[81];
	F_keys[19]				= (FUNC_INFO *)&all_func[96];


#ifdef VT200
    norm_key[KEY_ARROW_DOWN]	= (FUNC_INFO *)&all_func[16];
    norm_key[KEY_ARROW_UP]		= (FUNC_INFO *)&all_func[17];
    norm_key[KEY_ARROW_RIGHT]	= (FUNC_INFO *)&all_func[18];
    norm_key[KEY_ARROW_LEFT]	= (FUNC_INFO *)&all_func[19];
    norm_key[KEY_REMOVE]		= (FUNC_INFO *)&all_func[32];
    norm_key[KEY_PREV_SCREEN]	= (FUNC_INFO *)&all_func[31];
    norm_key[KEY_NEXT_SCREEN]	= (FUNC_INFO *)&all_func[30];
    norm_key[KEY_INSERT_HERE]	= (FUNC_INFO *)&all_func[39];
    norm_key[KEY_FIND]			= (FUNC_INFO *)&all_func[41];
#endif

#if defined(_MAC_CODE) || defined(_WIN32)
    norm_key[404]		= (FUNC_INFO *)&all_func[94];
    norm_key[403]		= (FUNC_INFO *)&all_func[95];
    norm_key[405]		= (FUNC_INFO *)&all_func[29];
    norm_key[407]		= (FUNC_INFO *)&all_func[28];
    norm_key[409]		= (FUNC_INFO *)&all_func[16];
    norm_key[410]		= (FUNC_INFO *)&all_func[17];
    norm_key[412]		= (FUNC_INFO *)&all_func[18];
    norm_key[411]		= (FUNC_INFO *)&all_func[19];
    norm_key[413]		= (FUNC_INFO *)&all_func[44];
    norm_key[414]		= (FUNC_INFO *)&all_func[45];
    norm_key[415]		= (FUNC_INFO *)&all_func[30];
    norm_key[416]		= (FUNC_INFO *)&all_func[31];
    norm_key[421]		= (FUNC_INFO *)&all_func[92];
    norm_key[420]		= (FUNC_INFO *)&all_func[93];
#endif

    norm_key[127]		= (FUNC_INFO *)&all_func[32];

    ESC_key['e']		= (FUNC_INFO *)&all_func[4];
    ESC_key['E']		= (FUNC_INFO *)&all_func[4];
    ESC_key['r']		= (FUNC_INFO *)&all_func[7];
    ESC_key['R']		= (FUNC_INFO *)&all_func[7];
    ESC_key['q']		= (FUNC_INFO *)&all_func[7];
    ESC_key['Q']		= (FUNC_INFO *)&all_func[7];
    ESC_key['c']		= (FUNC_INFO *)&all_func[9];
    ESC_key['C']		= (FUNC_INFO *)&all_func[9];
    ESC_key['u']		= (FUNC_INFO *)&all_func[11];
    ESC_key['U']		= (FUNC_INFO *)&all_func[11];
    ESC_key['[']		= (FUNC_INFO *)&all_func[14];
    ESC_key['?']		= (FUNC_INFO *)&all_func[15];
    ESC_key['/']		= (FUNC_INFO *)&all_func[15];
    ESC_key['f']		= (FUNC_INFO *)&all_func[24];
    ESC_key['F']		= (FUNC_INFO *)&all_func[24];
    ESC_key['b']		= (FUNC_INFO *)&all_func[25];
    ESC_key['B']		= (FUNC_INFO *)&all_func[25];
    ESC_key[',']		= (FUNC_INFO *)&all_func[26];
    ESC_key['.']		= (FUNC_INFO *)&all_func[27];
    ESC_key['v']		= (FUNC_INFO *)&all_func[31];
    ESC_key['V']		= (FUNC_INFO *)&all_func[31];
    ESC_key['d']		= (FUNC_INFO *)&all_func[35];
    ESC_key['D']		= (FUNC_INFO *)&all_func[35];
    ESC_key[CTRL('H')]	= (FUNC_INFO *)&all_func[36];
//    ESC_key[27]			= (FUNC_INFO *)&all_func[37];
    ESC_key['s']		= (FUNC_INFO *)&all_func[40];
    ESC_key['S']		= (FUNC_INFO *)&all_func[40];
    ESC_key['<']		= (FUNC_INFO *)&all_func[44];
    ESC_key['>']		= (FUNC_INFO *)&all_func[45];
    ESC_key['h']		= (FUNC_INFO *)&all_func[47];
    ESC_key['H']		= (FUNC_INFO *)&all_func[47];
    ESC_key['z']		= (FUNC_INFO *)&all_func[52];
    ESC_key['Z']		= (FUNC_INFO *)&all_func[52];
    ESC_key['w']		= (FUNC_INFO *)&all_func[54];
    ESC_key['W']		= (FUNC_INFO *)&all_func[54];
    ESC_key['x']		= (FUNC_INFO *)&all_func[55];
    ESC_key['X']		= (FUNC_INFO *)&all_func[55];
    ESC_key['p']		= (FUNC_INFO *)&all_func[56];
    ESC_key['P']		= (FUNC_INFO *)&all_func[56];
    ESC_key['t']		= (FUNC_INFO *)&all_func[57];
    ESC_key['T']		= (FUNC_INFO *)&all_func[57];
    ESC_key['m']		= (FUNC_INFO *)&all_func[58];
    ESC_key['M']		= (FUNC_INFO *)&all_func[58];
    ESC_key['n']		= (FUNC_INFO *)&all_func[61];
    ESC_key['N']		= (FUNC_INFO *)&all_func[61];
    ESC_key['0']		= (FUNC_INFO *)&all_func[63];
    ESC_key[224]		= (FUNC_INFO *)&all_func[63];
    ESC_key['1']		= (FUNC_INFO *)&all_func[49];
    ESC_key[38]			= (FUNC_INFO *)&all_func[49];
    ESC_key['4']		= (FUNC_INFO *)&all_func[65];
    ESC_key[39]			= (FUNC_INFO *)&all_func[65];
    ESC_key['3']		= (FUNC_INFO *)&all_func[67];
    ESC_key[34]			= (FUNC_INFO *)&all_func[67];
    ESC_key['2']		= (FUNC_INFO *)&all_func[69];
    ESC_key[233]		= (FUNC_INFO *)&all_func[69];
    ESC_key['8']		= (FUNC_INFO *)&all_func[71];
    ESC_key[33]			= (FUNC_INFO *)&all_func[71];
    ESC_key['l']		= (FUNC_INFO *)&all_func[72];
    ESC_key['L']		= (FUNC_INFO *)&all_func[72];
    ESC_key['a']		= (FUNC_INFO *)&all_func[73];
    ESC_key['A']		= (FUNC_INFO *)&all_func[73];
    ESC_key['g']		= (FUNC_INFO *)&all_func[74];
    ESC_key['G']		= (FUNC_INFO *)&all_func[74];
    ESC_key['7']		= (FUNC_INFO *)&all_func[78];
    ESC_key[232]		= (FUNC_INFO *)&all_func[78];
    ESC_key['6']		= (FUNC_INFO *)&all_func[79];
    ESC_key[167]		= (FUNC_INFO *)&all_func[79];
    ESC_key['5']		= (FUNC_INFO *)&all_func[80];
    ESC_key[40]			= (FUNC_INFO *)&all_func[80];
    ESC_key['9']		= (FUNC_INFO *)&all_func[97];
    ESC_key[231]		= (FUNC_INFO *)&all_func[97];

#if !defined(_WIN32)
    X_key[CTRL('C')]	= (FUNC_INFO *)&all_func[82];
    X_key[CTRL('K')]	= (FUNC_INFO *)&all_func[13];
    X_key[CTRL('W')]	= (FUNC_INFO *)&all_func[22];
    X_key[CTRL('S')]	= (FUNC_INFO *)&all_func[38];
    X_key[CTRL('U')]	= (FUNC_INFO *)&all_func[46];
    X_key[CTRL('R')]	= (FUNC_INFO *)&all_func[48];
    X_key[CTRL('V')]	= (FUNC_INFO *)&all_func[59];
    X_key[CTRL('L')]	= (FUNC_INFO *)&all_func[77];
#endif
}

int WriteState(int d) {
	FILE *fp;
	register int i;
	register int j;
	FNType fileName[FNSize];

#ifdef _MAC_CODE
	DrawSoundCursor(0);
#endif
	RemoveLastUndo();
#ifdef _MAC_CODE
	i = 0;
	strcpy(fileName, defaultPath);
	addFilename2Path(fileName, KEYS_BIND_FILE);
  	if (!myNavPutFile(fileName, "Save Program State as:", 'TEXT', NULL)) {
		strcpy(global_df->err_message, DASHES);
		return(54);
  	}
#elif defined(_WIN32) /* !_MAC_CODE */
    OPENFILENAME	ofn;
	unCH			szFile[FILENAME_MAX];
	unCH			*szFilter;
	unCH			wDirPathName[FNSize];

	i = 0;
	szFilter = _T("Utility Files (*.cut)\0*.cut\0All files (*.*)\0*.*\0\0");
    strcpy(szFile, KEYS_BIND_FILE);
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
    ofn.lpstrTitle = _T("Save Program State as:");
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;

	if (GetSaveFileName(&ofn) == 0) {
		strcpy(global_df->err_message, DASHES);
		return(54);
	}
	u_strcpy(fileName, szFile, FNSize);
#else
	strcpy(fileName,"State file: ");
	i = strlen(fileName);
	if (!new_getstr(fileName,i,FF)) { strcpy(global_df->err_message, DASHES); return(54); }
#endif /* !_MAC_CODE && !_WIN32*/
	if ((fp=fopen(fileName+i, "w")) == NULL) {
		strcpy(global_df->err_message, "+Can't write keys file!");
		return(54);
	}
#ifdef _MAC_CODE
	settyp(fileName+i, 'TEXT', the_file_creator.out, FALSE);
#endif /* _MAC_CODE */
	fprintf(fp, "@#$lxs$#@sxl@#$\n");
#if !defined(_MAC_CODE) && !defined(_WIN32)
	if (ced_total_lineno)
		fprintf(fp, "P1 0\n");
	if (!ChatMode)
		fprintf(fp, "W1 0\n");
#endif
	for (i=0; i < 524; i++) {
		if (norm_key[i]->func_ptr != IllegalFunction &&
			norm_key[i]->func_ptr != SelfInsert) {
			for (j=0; all_func[j].func_ptr != NULL; j++) {
				if (&all_func[j] == norm_key[i]) {
					fprintf(fp, "N%d %d\n", i, j);
					break;
				}
			}
		}
	}
	for (i=0; i < 524; i++) {
		if (ESC_key[i]->func_ptr != IllegalFunction &&
			ESC_key[i]->func_ptr != SelfInsert) {
			for (j=0; all_func[j].func_ptr != NULL; j++) {
				if (&all_func[j] == ESC_key[i]) {
					fprintf(fp, "E%d %d\n", i, j);
					break;
				}
			}
		}
	}
	for (i=0; i < 128; i++) {
		if (X_key[i]->func_ptr != IllegalFunction &&
			X_key[i]->func_ptr != SelfInsert) {
			for (j=0; all_func[j].func_ptr != NULL; j++) {
				if (&all_func[j] == X_key[i]) {
					fprintf(fp, "X%d %d\n", i, j);
					break;
				}
			}
		}
	}
	for (i=0; i < 50; i++) {
		if (F_keys[i]->func_ptr != IllegalFunction &&
			F_keys[i]->func_ptr != SelfInsert) {
			for (j=0; all_func[j].func_ptr != NULL; j++) {
				if (&all_func[j] == F_keys[i]) {
					fprintf(fp, "F%d %d\n", i, j);
					break;
				}
			}
		}
	}
	fclose(fp);
	strcpy(global_df->err_message, "-Done!");
	return(54);
}
/* 2009-10-13
static void MakeWin(int (* fn)(WINDOW *w, char *st, char c), char *st) {
	WINDOW *w;

	w = newwin(NumOfRowsOfDefaultWindow(), global_df->num_of_cols, 0, 0, 0);
	wstandend(w);
	fn(w,st,FALSE);
	werase(w); wrefresh(w); delwin(w);
	strcpy(global_df->err_message, DASHES);
	touchwin(global_df->w1); touchwin(global_df->wm); 
	wrefresh(global_df->w1); wrefresh(global_df->wm);
	if (!issoundwin()) {
		touchwin(global_df->w2); wrefresh(global_df->w2);
	}
}
*/
int Apropos(int d) {
#if defined(_MAC_CODE)
	OpenHelpWindow("");
#endif // _MAC_CODE
#if defined(_WIN32) 
	CHelpWindow dlg;
	int sl[256];
	long i;
	char st[SPEAKERLEN];

	dlg.m_doHelp = FALSE;
	SortCommands(sl,"",'\001');
	dlg.m_HelpOutput = _T("");
	for (i=0; sl[i] != -1; i++) {
		AddKeysToCommand(sl[i], st);
		strcat(st, "\r\n");
		dlg.m_HelpOutput = dlg.m_HelpOutput + st;
	}
	dlg.DoModal();
#endif/* _WIN32 */
	return(15);
}

int AproposFile(int d) {
	FILE *fp;
	int i;
	int sl[256];
	char s[256];
	FNType fileName[FNSize];

#ifdef _MAC_CODE
	if (d > -1) {
		DrawSoundCursor(0);
		RemoveLastUndo();
	}
#endif // _MAC_CODE
#ifdef _MAC_CODE
	strcpy(fileName, defaultPath);
	addFilename2Path(fileName, "keys_list.cut");
	if (!myNavPutFile(fileName, "Save Keys Bindings as:", 'TEXT', NULL)) {
		if (d > -1)
			strcpy(global_df->err_message, DASHES);
		return(47);
	}
#elif defined(_WIN32) /* !_MAC_CODE */
    OPENFILENAME	ofn;
	unCH			szFile[FILENAME_MAX];
	unCH			*szFilter;
	unCH			wDirPathName[FNSize];

	i = 0;
	szFilter = _T("All files (*.*)\0*.*\0\0");
    strcpy(szFile, "keys_list.cut");
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
    ofn.lpstrTitle = _T("Save Keys Bindings as:");
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;

	if (GetSaveFileName(&ofn) == 0) {
		return(47);
	}
	u_strcpy(fileName, szFile, FNSize);
#endif /* else !_MAC_CODE && !_WIN32*/
	if ((fp=fopen(fileName,"w")) == NULL) {
		do_warning("Can't write to file!", 0);
		return(47);
	}
#ifdef _MAC_CODE
	settyp(fileName, 'TEXT', the_file_creator.out, FALSE);
#endif /* _MAC_CODE */
	SortCommands(sl,"\0",'\0');
	fprintf(fp, "%s\n", UTF8HEADER);
	for (i=0; sl[i] != -1; i++) {
		AddKeysToCommand(sl[i], s);
		fprintf(fp,"%s\n",s);
	}
	fclose(fp);
	return(47);
}

int CloseCurrentWindow(int i) {
#ifdef _MAC_CODE
	PrepareStruct	saveRec;
	WindowPtr		front;

	if (StdInWindow != NULL && global_df != NULL && !strcmp(StdInWindow,global_df->fileName))
		do_warning(StdInErrMessage, 0);
	else {
		front = FrontWindow();
		PrepareWindA4(front, &saveRec);
		mCloseWindow(front);
		RestoreWindA4(&saveRec);
	}
#endif // _MAC_CODE
	return(82);
}

int ExecCommand(int d) {
#ifdef _MAC_CODE
	DrawSoundCursor(0);
	RemoveLastUndo();
	OpenHelpWindow(global_df->fileName);
	return(55);
#endif
#if defined(_WIN32)
	register int i;
	register int j;
	char st[SPEAKERLEN], fn[SPEAKERLEN];
	CHelpWindow dlg;
	int sl[256];
	
	if (global_df == NULL)
		return(55);
	DrawSoundCursor(0);
	RemoveLastUndo();
	DrawCursor(1);
	dlg.m_doHelp = TRUE;
	SortCommands(sl,"",'\001');
	dlg.m_HelpOutput = _T("");
	for (i=0; sl[i] != -1; i++) {
		AddKeysToCommand(sl[i], st);
		strcat(st, "\r\n");
		dlg.m_HelpOutput = dlg.m_HelpOutput + st;
	}
	if (dlg.DoModal() == IDOK) {
		DrawCursor(0);
		u_strcpy(st, dlg.m_HelpInput, SPEAKERLEN);
		j = 0;
	} else {
		DrawCursor(0);
		strcpy(global_df->err_message,DASHES);
		return(55);
	}
	if (st[j] == EOS) {
		strcpy(global_df->err_message,DASHES);
		return(55);
	}
	uS.uppercasestr(st+j, &dFnt, FALSE);
	for (i=0; all_func[i].func_ptr != NULL; i++) {
		strcpy(fn, all_func[i].func_name);
		uS.uppercasestr(fn, &dFnt, FALSE);
		if (!strcmp(st+j, fn)) {
			strcpy(global_df->err_message, DASHES);
			if (all_func[i].func_ptr != SelfInsert &&
				all_func[i].func_ptr != IllegalFunction &&
				all_func[i].func_ptr != HandleESC &&
				all_func[i].func_ptr != HandleCTRLX &&
				all_func[i].func_ptr != ExecCommand)
				return((*all_func[i].func_ptr)(1));
			else
				return(55);
		}
	}
	strcpy(global_df->err_message, "+Command not found!");
	return(55);
#endif
}

/* begin commands */
int IllegalFunction(int i) {
	RemoveLastUndo();
	strcpy(global_df->err_message, BEEP);
#ifdef _MAC_CODE
	FlushEvents(keyDownMask, 0);
	FlushEvents(keyUpMask, 0);
#endif
	return(-1);
}

int AbortCommand(int i) {
	RemoveLastUndo();
	return(60);
}

int clGetVersion(int d) {
	extern char VERSION[];

    RemoveLastUndo();
    strcpy(global_df->err_message, "-");
    strcat(global_df->err_message, VERSION);
    return(68);
}

int  NONEXISTANT(int d) {
	return(1);
}

#ifndef _WIN32
static int ShowAscii(WINDOW *w, unCH *st, unCH ch) {
	register int tTextWinSize;
	register int largestRowNum = w->num_rows;
	int  width, t;
	char s[SPEAKERLEN];
	int c, let = 0;
	int i, row = 0;
	GrafPtr oldPort;

	if (ch == '\002')
		return(0);
	c = (int)ch;
	t = w->num_rows;
#if (TARGET_API_MAC_CARBON == 1)
	Rect rect;
	GetWindowPortBounds(global_df->wind, &rect);
	width = rect.right-rect.left-SCROLL_BAR_SIZE-LEFTMARGIN;
#else
	width = global_df->wind->portRect.right-global_df->wind->portRect.left-SCROLL_BAR_SIZE-LEFTMARGIN;
#endif
	GetPort(&oldPort);
	SetPortWindowPort(global_df->wind);
	TextFont(global_df->row_txt->Font.FName);
	TextSize(global_df->row_txt->Font.FSize);
	tTextWinSize = global_df->TextWinSize-global_df->row_txt->Font.FHeight;
	while (let < 256) {
		w->num_rows = t;
		if (row == 0)
			tTextWinSize = global_df->TextWinSize-global_df->row_txt->Font.FHeight;
		copyFontInfo(w->RowFInfo[row], &global_df->row_txt->Font, FALSE);
		tTextWinSize -= global_df->row_txt->Font.FHeight;
		if (tTextWinSize < 0) {
			largestRowNum = row+1;
			w->num_rows = largestRowNum;
			row--;
			goto CheckRowNum;
		}
		*s = EOS;
		for (i=32;let < 256; ) {
			sprintf(s+i, "%3d-%c|  ", let, (char)let);
			if (TextWidth(s, 0, strlen(s)) >= width) {
				s[i] = EOS;
				break;
			}
			let++;
			i = strlen(s);
		}
		strcpy(templine4, s);
		mvwaddstr(w,row,0,templine4);
		wclrtoeol(w, true);
CheckRowNum:
		if ((row=RowCheck(w,++row,FALSE,&c)) == -1)
			break;
	}
	SetPort(oldPort);
	w->num_rows = largestRowNum;
	if (row > 0)
		row=RowCheck(w,row,TRUE,&c);
	w->num_rows = t;
	return(c);
}
#endif /* !_WIN32 */

int EnterAscii(int i) {
#if defined(_MAC_CODE) || defined(_WIN32)
	DrawSoundCursor(0);
#endif
	RemoveLastUndo();
#ifdef _MAC_CODE
	strcpy(sp,"Enter ASCII number: ");
	i = strlen(sp);
	if (!new_getstr(sp,i,ShowAscii)) {
		strcpy(global_df->err_message,DASHES);
		return(74);
	}
#endif
#if defined(_WIN32) 
	CGetAscii dlg;

	DrawCursor(1);
	dlg.m_AsciiInput = _T("");
	dlg.m_AsciiOutput = _T("");
	for (i=33; i < 256; i++) {
		if (i == 9)
			uS.sprintf(sp, cl_T("%3d: TAB\r\n"), i);
		else if (i == 10)
			uS.sprintf(sp, cl_T("%3d: LF\r\n"), i);
		else if (i == 13)
			uS.sprintf(sp, cl_T("%3d: CR\r\n"), i);
		else if (i == 149)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+Shift+8\r\n"), i, (char)i);
		else if (i == 159)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,Shift+Y\r\n"), i, (char)i);
		else if (i == 167)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+up_arrow\r\n"), i, (char)i);
		else if (i == 175)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+down_arrow\r\n"), i, (char)i);
		else if (i == 192)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,Shift+A\r\n"), i, (char)i);
		else if (i == 193)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',Shift+A\r\n"), i, (char)i);
		else if (i == 194)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,Shift+A\r\n"), i, (char)i);
		else if (i == 195)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+~,Shift+A\r\n"), i, (char)i);
		else if (i == 196)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,Shift+A\r\n"), i, (char)i);
		else if (i == 197)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+@,Shift+A\r\n"), i, (char)i);
		else if (i == 198)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+&,Shift+A\r\n"), i, (char)i);
		else if (i == 199)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+,,Shift+C\r\n"), i, (char)i);
		else if (i == 200)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,Shift+E\r\n"), i, (char)i);
		else if (i == 201)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',Shift+E\r\n"), i, (char)i);
		else if (i == 202)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,Shift+E\r\n"), i, (char)i);
		else if (i == 203)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,Shift+E\r\n"), i, (char)i);
		else if (i == 204)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,Shift+I\r\n"), i, (char)i);
		else if (i == 205)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',Shift+I\r\n"), i, (char)i);
		else if (i == 206)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,Shift+I\r\n"), i, (char)i);
		else if (i == 207)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,Shift+I\r\n"), i, (char)i);
		else if (i == 209)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+~,Shift+N\r\n"), i, (char)i);
		else if (i == 210)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,Shift+O\r\n"), i, (char)i);
		else if (i == 211)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',Shift+O\r\n"), i, (char)i);
		else if (i == 212)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,Shift+O\r\n"), i, (char)i);
		else if (i == 213)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+~,Shift+O\r\n"), i, (char)i);
		else if (i == 214)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,Shift+O\r\n"), i, (char)i);
		else if (i == 216)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+/,Shift+O\r\n"), i, (char)i);
		else if (i == 217)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,Shift+U\r\n"), i, (char)i);
		else if (i == 218)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',Shift+U\r\n"), i, (char)i);
		else if (i == 219)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,Shift+U\r\n"), i, (char)i);
		else if (i == 220)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,Shift+U\r\n"), i, (char)i);
		else if (i == 221)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',Shift+Y\r\n"), i, (char)i);
		else if (i == 223)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+&,S\r\n"), i, (char)i);
		else if (i == 224)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,A\r\n"), i, (char)i);
		else if (i == 225)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',A\r\n"), i, (char)i);
		else if (i == 226)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,A\r\n"), i, (char)i);
		else if (i == 227)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+~,A\r\n"), i, (char)i);
		else if (i == 228)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,A\r\n"), i, (char)i);
		else if (i == 229)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+@,A\r\n"), i, (char)i);
		else if (i == 230)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+&,A\r\n"), i, (char)i);
		else if (i == 231)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+,,C\r\n"), i, (char)i);
		else if (i == 232)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,E\r\n"), i, (char)i);
		else if (i == 233)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',E\r\n"), i, (char)i);
		else if (i == 234)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,E\r\n"), i, (char)i);
		else if (i == 235)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,E\r\n"), i, (char)i);
		else if (i == 236)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,I\r\n"), i, (char)i);
		else if (i == 237)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',I\r\n"), i, (char)i);
		else if (i == 238)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,I\r\n"), i, (char)i);
		else if (i == 239)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,I\r\n"), i, (char)i);
		else if (i == 241)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+~,N\r\n"), i, (char)i);
		else if (i == 242)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,O\r\n"), i, (char)i);
		else if (i == 243)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',O\r\n"), i, (char)i);
		else if (i == 244)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,O\r\n"), i, (char)i);
		else if (i == 245)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+~,O\r\n"), i, (char)i);
		else if (i == 246)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,O\r\n"), i, (char)i);
		else if (i == 248)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+/,O\r\n"), i, (char)i);
		else if (i == 249)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+`,U\r\n"), i, (char)i);
		else if (i == 250)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',U\r\n"), i, (char)i);
		else if (i == 251)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+^,U\r\n"), i, (char)i);
		else if (i == 252)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,U\r\n"), i, (char)i);
		else if (i == 253)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+',Y\r\n"), i, (char)i);
		else if (i == 255)
			uS.sprintf(sp, cl_T("%3d: %c: Ctrl+:,Y\r\n"), i, (char)i);
		else
			uS.sprintf(sp, cl_T("%3d: %c\r\n"), i, (char)i);
		dlg.m_AsciiOutput = dlg.m_AsciiOutput + sp;
	}
	if (dlg.DoModal() == IDOK) {
		DrawCursor(0);
		i = 0;
		strcpy(sp, dlg.m_AsciiInput);
	} else {
		DrawCursor(0);
		strcpy(global_df->err_message,DASHES);
		*sp = EOS;
		return(74);
	}
#endif/* _WIN32 */
	SaveUndoState(FALSE);
	for (; sp[i] != EOS; i++)
		SelfInsert(sp[i]);
	*sp = EOS;
	strcpy(global_df->err_message, DASHES);
	return(74);
}

int SetNextTierName(int d) {
	register int i;
	unCH st[SPEAKERLEN];

#if defined(_MAC_CODE) || defined(_WIN32)
	DrawSoundCursor(0);
#endif
	RemoveLastUndo();
#ifdef _MAC_CODE
	strcpy(st, "Speaker name [");
	strcat(st, NextTierName);
	strcat(st, "]: ");
	i = strlen(st);
	if (!new_getstr(st,i,NULL)) {
		strcpy(global_df->err_message, DASHES);
		return(57);
	}
#endif // _MAC_CODE
#if defined(_WIN32) 
	CNextTier dlg;

	DrawCursor(1);
	dlg.m_NextTier = NextTierName;
	if (dlg.DoModal() == IDOK) {
		DrawCursor(0);
		i = 0;
		strcpy(st, dlg.m_NextTier);
	} else {
		DrawCursor(0);
		strcpy(global_df->err_message, DASHES);
		return(57);
	}
#endif/* _WIN32 */
	if (st[i] != EOS) {
		if (st[i] != '*')
			strcpy(NextTierName, "*");
		else
			NextTierName[0] = EOS;
		strcat(NextTierName, st+i);
		uS.uppercasestr(NextTierName, &dFnt, FALSE);
	}
	strcpy(global_df->err_message, DASHES);
	return(57);
}
    
int DescribeKey(int i) {
#ifdef _MAC_CODE
	RemoveLastUndo();
	strcpy(global_df->err_message, "-Press key to test");
	draw_mid_wm(); 
	i = wgetch(global_df->w1);
	sprintf(global_df->err_message, "-key=%c (dec=%d, oct=%o, 0x%x)", i, i, i, i);
	draw_mid_wm(); 
#endif
#if defined(_WIN32)
 	IllegalFunction(i);
#endif
	return(13);
}

int ChatModeSet(int d) {
	if (global_df == NULL)
		return(58);
	RemoveLastUndo();
	if (global_df->RowLimit)
		return(58);
	if (global_df->ChatMode) {
		global_df->ChatMode = FALSE;
		strcpy(global_df->err_message, "-TEXT mode is on");
	} else {
		global_df->ChatMode = TRUE;
		strcpy(global_df->err_message, "-CHAT mode is on");
	}
#ifdef _MAC_CODE
	SetTextWinMenus(TRUE);
#endif
	RefreshAllTextWindows(TRUE);
	return(58);
}

int PercentOfFile(int d) {
	RemoveLastUndo();
/*
	if (ShowPercentOfFile)
		ShowPercentOfFile = FALSE;
	else
		ShowPercentOfFile = TRUE;
*/
	return(56);
}

int ShowParagraphMarks(int d) {
	char tDataChanged;

	tDataChanged = global_df->DataChanged;
	if (global_df->ShowParags) {
		global_df->ShowParags = '\0';
		if (!global_df->ChatMode) {
			if (global_df->row_txt == global_df->cur_line) {
				if (global_df->col_txt->prev_char->c==NL_C && 
							global_df->col_txt->prev_char!= global_df->head_row &&
							global_df->col_txt == global_df->tail_row) {
					global_df->col_chr--;
					global_df->col_txt = global_df->col_txt->prev_char;
					global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
				}
			} else {
				if (global_df->row_txt->line[global_df->col_chr-1] == NL_C && 
							global_df->col_chr > 0L && 
							global_df->col_chr == (int)strlen(global_df->row_txt->line)) {
					global_df->col_chr--;
					global_df->col_win = ComColWin(FALSE, global_df->row_txt->line, global_df->col_chr);
				}
			}
		} else {
			BeginningOfLine(-1);
			CheckLeftCol(global_df->col_win);
		}
	} else
		global_df->ShowParags = '\002';

	ChangeCurLine();
	global_df->col_win = ComColWin(FALSE, NULL, global_df->col_chr);
	DisplayTextWindow(NULL, 1);
#ifdef _MAC_CODE
	SetTextWinMenus(TRUE);
#endif
	global_df->DataChanged = tDataChanged;
	return(73);
}

int DefConstString(int i) {
#if defined(_MAC_CODE) || defined(_WIN32)
	DoDefContStrings();
#else
	int sp, len;
	char st[SPEAKERLEN];
	WINDOW *wi;

	RemoveLastUndo();
	wi = newwin(1, num_of_cols, CodeWinStart, 0, 0);
	wstandend(wi);
	werase(wi); touchwin(wi); wrefresh(wi); 
	mvwaddstr(wi,0,0,": String number: ");
	wrefresh(wi);
	sp = wgetch(wi) - '0';
	werase(wi); wrefresh(wi); delwin(wi);
	if (sp < 0 || sp > 9) {
		if (sp == CTRL('G')-'0' || sp == 13-'0') strcpy(err_message, DASHES);
		else strcpy(err_message, "+Number must be between 0 and 9");
		if (!issoundwin()) {
			touchwin(w2); wrefresh(w2);
		}
		return(61);
	}

	sprintf(st, "String (%d): ", sp);
	len = strlen(st);
	if (!new_getstr(st,len,NULL)) { strcpy(err_message, DASHES); return(61); }
	if (AllocConstString(st+len, sp)) strcpy(err_message, DASHES);
#endif /* else _MAC_CODE && _WIN32 */
	WriteCedPreference();
	return(61);
}

int InsertConstString(int i) {
	register int sp;

#if defined(_MAC_CODE) || defined(_WIN32)
	DrawSoundCursor(0);
#endif
#ifdef _WIN32
	CConstStringNumber dlg;

	DrawCursor(1);
	dlg.m_StringNumber = 1;
	if (dlg.DoModal() != IDOK) {
		DrawCursor(0);
		strcpy(global_df->err_message, DASHES);
		return(62);
	}
	DrawCursor(0);
	sp = dlg.m_StringNumber - 1;
#elif defined(_MAC_CODE)
	sp = DoConstantString();
	if (sp == -1) {
		strcpy(global_df->err_message, DASHES);
		return(62);
	}
#else
	WINDOW *wi;

	wi = newwin(1, global_df->num_of_cols, global_df->CodeWinStart, 0, 0);
	wstandend(wi);
	werase(wi); touchwin(wi); wrefresh(wi); 
	mvwaddstr(wi,0,0,": String number: ");
	wclrtoeol(wi, true);
	wrefresh(wi);
	sp = wgetch(wi) - '0';
	werase(wi); wrefresh(wi); delwin(wi);
	if (!issoundwin()) {
		touchwin(global_df->w2); wrefresh(global_df->w2);
	}
#endif /* else _WIN32 */
	if (sp < 0 || sp > 9) {
		RemoveLastUndo();
		if (sp == (int)CTRL('G')-'0' || sp == 13-'0')
			strcpy(global_df->err_message, DASHES);
		else
			strcpy(global_df->err_message, "+Number must be between 0 and 9");
		return(62);
	}

	AddConstString(sp);
	strcpy(global_df->err_message, DASHES);
	return(62);
}

int ToTopLevel(int i) {
	if (global_df->EditorMode && i != -1) {
		IllegalFunction(i);
		return(9);
    }
	if (global_df->RootCodesArr[0] != global_df->CurCode && i != -1) {
		global_df->FirstCodeOfPrevLevel = global_df->CurFirstCodeOfPrevLevel;
		MapArrToList(global_df->CurCode);
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
    } else if (global_df->CurCode != global_df->RootCodes) {
		global_df->EnteredCode = '\0';
		global_df->CurFirstCodeOfPrevLevel = global_df->RootCodes;
		global_df->CurCode = global_df->RootCodes;
		global_df->FirstCodeOfPrevLevel = global_df->RootCodes;
		MapArrToList(global_df->CurCode);
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
		GetCurCode();
		FindRightCode(1);
	}
	return(9);
}

int EditMode(int i) {
	if (global_df == NULL)
		return(4);
	if (global_df->RowLimit){
		RemoveLastUndo();
		return(4);
	}
#if defined(_MAC_CODE) || defined(_WIN32)
	if (global_df->SoundWin) {
		RemoveLastUndo();
		strcpy(global_df->err_message, "+Finish working with sound!");
		return(4);
	}
#endif
	if (global_df->NoCodes) {
		if (!GetNewCodes(-1)) {
			strcpy(global_df->err_message, "-NO CODES DEFINED");
			RemoveLastUndo();
			return(4);
		}
		if (global_df->NoCodes) {
			RemoveLastUndo();
			return(4);
		}
	}
	if (!global_df->EditorMode)
		ToTopLevel(-1);
	global_df->EditorMode = !global_df->EditorMode;
#ifdef _MAC_CODE
	SetTextWinMenus(TRUE);
#endif
	if (global_df->EditorMode) {
		werase(global_df->w2); wrefresh(global_df->w2);
	} else {
		global_df->EndCodeArr = DisplayCodes(COM_WIN_SIZE);
		DisplayCursor(COM_WIN_SIZE);
	}
	strcpy(global_df->err_message, DASHES);
	return(4);
}

static void ConvertText(char toUpper) {
	long i;
	LINE *tct;
    ROWS *rt;

#ifdef _MAC_CODE
	DrawCursor(0);
#endif
	if (global_df->row_win2 == 0 && global_df->col_win2 != -2L) {
		ChangeCurLine();
		if (global_df->col_win > global_df->col_win2) {
			for (tct=global_df->head_row->next_char, i=0; i < global_df->col_chr2; i++)
				tct = tct->next_char;
			for (; i < global_df->col_chr; i++, tct=tct->next_char) {
				if (toUpper)
					tct->c = (unCH)to_unCH_upper(tct->c);
				else
					tct->c = (unCH)to_unCH_lower(tct->c);
			}
		} else {
			for (tct=global_df->head_row->next_char, i=0; i < global_df->col_chr; i++)
				tct = tct->next_char;
			for (; i < global_df->col_chr2; i++, tct=tct->next_char) {
				if (toUpper)
					tct->c = (unCH)to_unCH_upper(tct->c);
				else
					tct->c = (unCH)to_unCH_lower(tct->c);
			}
		}
		DisplayTextWindow(global_df->cur_line, 1);
	} else if (global_df->row_win2 == 1 || global_df->row_win2 == -1) {
		ChangeCurLine();
		if (global_df->row_win2 == 1) {
			for (tct=global_df->head_row->next_char, i=0; i < global_df->col_chr; i++)
				tct = tct->next_char;
			for (; i < strlen(global_df->row_txt->line); i++, tct=tct->next_char) {
				if (toUpper)
					tct->c = (unCH)to_unCH_upper(tct->c);
				else
					tct->c = (unCH)to_unCH_lower(tct->c);
			}
		} else {
			rt = ToPrevRow(global_df->row_txt, FALSE);
			if (rt != global_df->head_text) {
				for (i=global_df->col_chr2; i < strlen(rt->line); i++) {
					if (toUpper)
						rt->line[i] = to_unCH_upper(rt->line[i]);
					else
						rt->line[i] = to_unCH_lower(rt->line[i]);
				}
			}
		}
		DisplayTextWindow(NULL, 1);
	} else if (global_df->row_win2 || global_df->col_win2 != -2) {
		do_warning("Selected region is too big.", 0);
	}
	global_df->LeaveHighliteOn = TRUE;
}

static int ToUpperCase(int i) {
    ConvertText(TRUE);
    return(10);
}

static int ToLowerCase(int i) {
    ConvertText(FALSE);
    return(11);
}

static int CopyText(int i) {
#ifdef _MAC_CODE
	char t;
	t = global_df->ScrollBar;
	global_df->ScrollBar = 1;
    doCopy();
    global_df->ScrollBar = t;
#else
	do_warning("Command is not imlemented on this platform", 0);
#endif
    return(79);
}

static int PasteText(int i) {
#ifdef _MAC_CODE
    doPaste();
#else
	do_warning("Command is not imlemented on this platform", 0);
#endif
    return(64);
}

void EnterSelection(void) {
    ROWS *rt;

	if (global_df == NULL) ;
	else if (global_df->row_win2 == 0 && global_df->col_win2 != -2L) {
		ChangeCurLineAlways(0);
		if (global_df->col_win > global_df->col_win2) {
			if (global_df->col_chr-global_df->col_chr2 < SPEAKERLEN) {
				strncpy(SearchString, global_df->row_txt->line+global_df->col_chr2, global_df->col_chr-global_df->col_chr2);
				SearchString[global_df->col_chr-global_df->col_chr2] = EOS;
			}
		} else {
			if (global_df->col_chr2-global_df->col_chr < SPEAKERLEN) {
				strncpy(SearchString, global_df->row_txt->line+global_df->col_chr, global_df->col_chr2-global_df->col_chr);
				SearchString[global_df->col_chr2-global_df->col_chr] = EOS;
			}
		}
		if (!CaseSensSearch)
			uS.uppercasestr(SearchString, &dFnt, C_MBF);
	} else if (global_df->row_win2 == 1 || global_df->row_win2 == -1) {
		long i;
		ChangeCurLineAlways(0);
		if (global_df->row_win2 == 1) {
			i = strlen(global_df->row_txt->line) - global_df->col_chr;
			if (i < SPEAKERLEN) {
				strncpy(SearchString, global_df->row_txt->line+global_df->col_chr, i);
				SearchString[i] = EOS;
			}
			rt = ToNextRow(global_df->row_txt, FALSE);
			if (rt != global_df->tail_text) {
				if (i+global_df->col_chr2 < SPEAKERLEN) {
					strncpy(SearchString+i, rt->line, global_df->col_chr2);
					SearchString[i+global_df->col_chr2] = EOS;
				}
			}
		} else {
			rt = ToPrevRow(global_df->row_txt, FALSE);
			if (rt != global_df->head_text) {
				i = strlen(rt->line) - global_df->col_chr2;
				if (i < SPEAKERLEN) {
					strncpy(SearchString, rt->line+global_df->col_chr2, i);
					SearchString[i] = EOS;
				}
			} else
				i = 0L;
			rt = ToNextRow(rt, FALSE);
			if (i+global_df->col_chr < SPEAKERLEN) {
				strncpy(SearchString+i, rt->line,global_df->col_chr);
				SearchString[i+global_df->col_chr] = EOS;
			}
		}
		for (i=0; SearchString[i]; i++) {
			if (SearchString[i] == NL_C)
				SearchString[i] = '\r';
		}
		if (!CaseSensSearch)
			uS.uppercasestr(SearchString, &dFnt, C_MBF);
	}
}

static int FindText(int i) {
	EnterSelection();
	FindSame(-1);
    return(78);
}

static int SpecialTextFile(int i) {
	global_df->SpecialTextFile = TRUE;
    return(90);
}

int ColorKeywords(int i) {
	RemoveLastUndo();
#ifdef _MAC_CODE
	DoColorKeywords();
#else
	strcpy(global_df->err_message, "+ONLY IMPLEMENTED FOR MAC SYSTEMS!");
#endif
    return(85);
}

int Space(int i) {
	global_df->FreqCount++;
	if (DeleteChank(1)) {
		if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
			PutCursorInWindow(global_df->w1);
		SaveUndoState(FALSE);
		AddText(NULL, (unCH)i, isShowLineNums, 1L);
		if (global_df->LastCommand == 1 && global_df->UndoList->NextUndo->key == INSTKEY) 
			RemoveLastUndo();
    }
    return(5);
}

int SelfInsert(int i) {
    if (!global_df->EditorMode) {
		IllegalFunction(i);
	    return(1);
	} else {
		if (global_df->isTempFile == 1 || (global_df->isTempFile && global_df->EditorMode))
			strcpy(global_df->err_message, "+This is read only file. It can not be modified.");
		else
		if (DeleteChank(1)) {
			if (global_df->row_win < 0 || global_df->row_win >= (long)global_df->EdWinSize)
				PutCursorInWindow(global_df->w1);
			SaveUndoState(FALSE);
			if ((unCH)i == '[' || (unCH)i == ']')
				global_df->gAtt = 0;
			AddText(NULL, (unCH)i, 0, 1L);
			if (global_df->LastCommand == 1 && global_df->UndoList->NextUndo->key == INSTKEY) 
				RemoveLastUndo();
		}
		return(1);
    }
}

int HandleESC(int i) {
	if (globalWhichInput == 0) {
		strcpy(global_df->err_message, ESC_KEY);
		if (*global_df->err_message)
			draw_mid_wm();
		if (global_df->row_win2 || global_df->col_win2 != -2)
			global_df->LeaveHighliteOn = TRUE;
		globalWhichInput = 3;
		return(3);
	} else {
		globalWhichInput = 0;
	}
	if (i < 524) {
		strcpy(global_df->err_message, DASHES);
		if (global_df->EditorMode)
			return((*ESC_key[i]->func_ptr)(i));
		else if (global_df->CurCode != global_df->RootCodes) {
			if (IsRightCommand(ESC_key[i]->func_ptr)) {
				return((*ESC_key[i]->func_ptr)(i));
			} else if (ESC_key[i]->func_ptr == NewLine || ESC_key[i]->func_ptr == Space) {
				return(GetCursorCode(i));
			} else if (ESC_key[i]->func_ptr == MoveUp) {
				return(MoveCodeCursorUp(i));
			} else if (ESC_key[i]->func_ptr == MoveDown) {
				return(MoveCodeCursorDown(i));
			}  else if (ESC_key[i]->func_ptr == MoveLeft) {
				return(MoveCodeCursorLeft(i));
			}  else if (ESC_key[i]->func_ptr == MoveRight) {
				return(MoveCodeCursorRight(i));
			} else {
				IllegalFunction(-1);
				strcpy(global_df->err_message, "+Please finish coding current line first.");
				return(-1);
			}
		} else
			return((*ESC_key[i]->func_ptr)(i));
	} else
		return(IllegalFunction(-1));
}


int HandleCTRLX(int i) {
	if (globalWhichInput == 0) {
		strcpy(global_df->err_message, CTRL_X);
		if (*global_df->err_message)
			draw_mid_wm();
		if (global_df->row_win2 || global_df->col_win2 != -2)
			global_df->LeaveHighliteOn = TRUE;
		globalWhichInput = 2;
		return(3);
	} else {
		globalWhichInput = 0;
	}
	if (i < 128) {
		strcpy(global_df->err_message, DASHES);
		if (global_df->EditorMode)
			return((*X_key[i]->func_ptr)(i));
		else if (global_df->CurCode != global_df->RootCodes) {
			if (IsRightCommand(X_key[i]->func_ptr)) {
				return((*X_key[i]->func_ptr)(i));
			} else if (X_key[i]->func_ptr == NewLine || X_key[i]->func_ptr == Space) {
				return(GetCursorCode(i));
			} else if (X_key[i]->func_ptr == MoveUp) {
				return(MoveCodeCursorUp(i));
			} else if (X_key[i]->func_ptr == MoveDown) {
				return(MoveCodeCursorDown(i));
			} else if (X_key[i]->func_ptr == MoveLeft) {
				return(MoveCodeCursorLeft(i));
			} else if (X_key[i]->func_ptr == MoveRight) {
				return(MoveCodeCursorRight(i));
			} else {
				IllegalFunction(-1);
				strcpy(global_df->err_message, "+Please finish coding current line first.");
				return(-1);
			}
		} else
			return((*X_key[i]->func_ptr)(i));
	} else
		return(IllegalFunction(-1));
}

int RefreshScreen(int i) {
#if defined(_MAC_CODE) || defined(_WIN32)
	global_df->WinChange = FALSE;
#endif
	if (i != -1) RemoveLastUndo();
	if (global_df == NULL) return(48);
	touchwin(global_df->w1);
	wrefresh(global_df->w1);
	if (global_df->wm != NULL) {
#if defined(_MAC_CODE) || defined(_WIN32)
		sp_touchwin(global_df->wm);
#else
		touchwin(w);
#endif
		wrefresh(global_df->wm);
		if (!issoundwin()) {
			touchwin(global_df->w2);
			wrefresh(global_df->w2);
		}
	}
#if defined(_MAC_CODE) || defined(_WIN32)
	global_df->WinChange = TRUE;
#endif
	return(48);
}

int ExitCED(int i) {
#ifndef _WIN32
	char res;
	MACWINDOWS *t;
	extern MACWINDOWS *RootWind;
	
	while (RootWind) {
		global_df = RootWind->windRec->FileInfo;
		DrawSoundCursor(0);
		res = 'y';
		if (global_df) {
			if (global_df->DataChanged == '\001') {
				res = QuitDialog(global_df->fileName);
				if (res == 'n') {
					strcpy(global_df->err_message, DASHES);
					return(6);
				}
			}
		}
		t = RootWind;
		global_df = t->windRec->FileInfo;
		SetPBCglobal_df(true, 0L);
		FreeFileInfo(t->windRec->FileInfo);
		if (t->windRec->VScrollHnd != NULL)
			DisposeControl(t->windRec->VScrollHnd);
		if (t->windRec->HScrollHnd != NULL)
			DisposeControl(t->windRec->HScrollHnd);
#if (TARGET_API_MAC_CARBON == 1)
		if (t->windRec->idocID) {
//			FixTSMDocument(t->windRec->idocID);
			DeactivateTSMDocument(t->windRec->idocID);
			DeleteTSMDocument(t->windRec->idocID);
		}
		if (t->windRec->mouseEventHandler)
			RemoveEventHandler(t->windRec->mouseEventHandler);
		if (t->windRec->textHandler)
			DisposeEventHandlerUPP(t->windRec->textHandler);
		if (t->windRec->textEventHandler)
			RemoveEventHandler(t->windRec->textEventHandler);
#endif
		ChangeWindowsMenuItem(t->windRec->wname, FALSE);
		if (t->windRec->CloseProc)
			t->windRec->CloseProc(t->wind);
		DisposeWindow(t->wind);
		RootWind = RootWind->nextWind;
		free(t->windRec);
		free(t);
	}
	global_df = NULL;
#else /* !_WIN32 */
	::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_CLOSE, 0, 0);
#endif /* _WIN32 */
	return(0);
}

int SelectAll(int i) {
#if defined(_MAC_CODE) || defined(_WIN32)
	long tn;
    ROWS *rt;

	global_df->row_win = global_df->wLineno - 1L;
	global_df->window_rows_offset = global_df->row_win;
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
	EndOfLine(-1);
    global_df->col_win2 = 0L;
    global_df->col_chr2 = 0L;
    global_df->row_win2 = 0L;
    rt = global_df->row_txt;
	while (!AtTopEnd(rt, global_df->head_text, FALSE)) {
		global_df->row_win2--;
		rt = ToPrevRow(rt, FALSE);
	}

	if (global_df->row_win2 < 0) {
		global_df->row_win2 += global_df->row_win;
		tn = global_df->row_win; global_df->row_win = global_df->row_win2; global_df->row_win2 = tn;
		global_df->row_win2 -= global_df->row_win;
		tn = global_df->col_win; global_df->col_win = global_df->col_win2; global_df->col_win2 = tn;
		tn = global_df->col_chr; global_df->col_chr = global_df->col_chr2; global_df->col_chr2 = tn;

		global_df->row_txt = ToNextRow(global_df->head_text, FALSE);
		global_df->lineno  = 1L;
		global_df->wLineno = 1L;
		global_df->window_rows_offset = 0L;
		if (global_df->row_txt == global_df->cur_line) 
			global_df->col_txt = global_df->head_row->next_char;
		if (global_df->top_win != global_df->row_txt) {
			global_df->top_win = global_df->row_txt;
			DisplayTextWindow(NULL, 1);
		}
	}
#else
    EndOfFile(1);
    col_win2 = 0;
    col_chr2 = 0;
    row_win2 = ced_lineno;
    row_win2 *= -1;
#endif
	if (global_df->col_win == 0L  && global_df->col_chr == 0L  && global_df->row_win == 0L &&
		global_df->col_win2 == 0L && global_df->col_chr2 == 0L && global_df->row_win2 == 0L) {
		global_df->row_win2 = 0L;
		global_df->col_win2 = -2L;
	} else
		global_df->LeaveHighliteOn = TRUE;
    return(75);
}

int FindSame(int i) {
	global_df->row_win2 = 0L;
	global_df->col_win2 = -2L;
	global_df->col_chr2 = -2L;
	if (SearchFFlag)
		global_df->LastCommand = SearchForward(-1);
	else
		global_df->LastCommand = SearchReverse(-1);
    return(76);
}
/* end commands */

static char IsRightCommand(int (*fn)(int)) {
    if (fn == Undo			 ||
	fn == Apropos			 ||
	fn == ExitCED			 ||
	fn == EditMode			 ||
	fn == HandleESC			 ||
	fn == HandleCTRLX		 ||
	fn == SelfInsert		 ||
	fn == ToTopLevel		 ||
	fn == MoveLineUp		 ||
	fn == MoveLineDown		 ||
	fn == CursorsCommand	 ||
	fn == GetCursorCode		 ||
	fn == GetFakeCursorCode	 ||
	fn == MoveCodeCursorUp	 ||
	fn == MoveCodeCursorDown ||
	fn == MoveCodeCursorLeft ||
	fn == MoveCodeCursorRight ||
	fn == EndCurTierGotoNext)
		return(TRUE);
    else
    	return(FALSE);
}

#ifndef _WIN32
#include "progs.h"

void DoCharStdInput(unCH *st) {
	int c;
	int LastCommand;
	int (*fn)(int);

	do {
		c = ced_getc();
		if (global_df == NULL) {
			if (isPlayS == 6) {
				strcpy(st, "\n");
				isPlayS = 0;
				cutt_exit(0);
				return;
			}
			isPlayS = 0;
			continue;
		}
		SaveUndoState(FALSE);
		if (isPlayS) {
			if (isPlayS == 6) {
				isPlayS = 0;
				if (CLAN_PROG_NUM == DSS) {
					strcpy(st, "q\n");
					do_warning(StdDoneMessage, -1);
				} else {
					strcpy(st, "\n");
					cutt_exit(0);
				}
				return;
			} else if (isPlayS == -5 || isPlayS == -8) {
				isPlayS = 0;
				continue;
			}
			fn = all_func[isPlayS].func_ptr;
			c = 1;
			isPlayS = 0;
		} else if (F_key) {
			fn = F_keys[F_key]->func_ptr;
			c = 1;
			F_key = 0;
		} else if (c == 3) {
			c = 13;
			fn = NewLine;
		} else if (c < 524)
			fn = norm_key[c]->func_ptr;
		else
			fn = SelfInsert;

		if (fn == MoveCodeCursorDown || fn == MoveCodeCursorUp || fn == MoveCodeCursorLeft || fn == MoveCodeCursorRight)
			fn = SelfInsert;
		if (fn == SelfInsert			 ||
				fn == Space				 ||
				fn == GotoLine			 ||
				fn == SearchForward		 ||
				fn == SearchReverse		 ||
				fn == FindSame			 ||
				fn == Replace			 ||
				fn == ExecCommand		 ||
				fn == AproposFile		 ||
				fn == Apropos			 ||
				fn == ToUpperCase		 ||
				fn == ToLowerCase		 ||
				fn == MoveDown			 ||
				fn == MoveUp			 ||
				fn == MoveRight			 ||
				fn == MoveLeft			 ||
				fn == MoveRightWord		 ||
				fn == MoveLeftWord		 ||
				fn == BeginningOfFile	 ||
				fn == EndOfFile			 ||
				fn == BeginningOfWindow	 ||
				fn == EndOfWindow		 ||
				fn == BeginningOfLine	 ||
				fn == EndOfLine			 ||
				fn == NextPage			 ||
				fn == MoveLineUp		 ||
				fn == MoveLineDown		 ||
				fn == PrevPage			 ||
				fn == Undo				 ||
				fn == ChatModeSet		 ||
				fn == DeleteNextChar	 ||
				fn == DeletePrevChar	 ||
				fn == DeletePrevWord	 ||
				fn == DeleteNextWord	 ||
				fn == YankKilled		 ||
				fn == KillLine			 ||
				fn == SelectAll) {
			LastCommand = (*fn)(c);
			if (LastCommand == 0) {
				strcpy(st, "\n");
				cutt_exit(0);
				return;
			} if (global_df == NULL) {
				strcpy(st, "\n");
				cutt_exit(0);
				return;
			}
			global_df->LastCommand = LastCommand;
		} else if (fn != NewLine) {
			do_warning(StdInErrMessage, 0);
			RemoveLastUndo();
		} else { /*  if (fn == NewLine) */
			if (StdInWindow != NULL && global_df != NULL && !strcmp(StdInWindow,global_df->fileName)) {
				RemoveLastUndo();
				break;
			} else {
				LastCommand = (*fn)(c);
				if (LastCommand == 0) {
					strcpy(st, "\n");
					cutt_exit(0);
					return;
				} if (global_df == NULL) {
					strcpy(st, "\n");
					cutt_exit(0);
					return;
				}
				global_df->LastCommand = LastCommand;
			}
		}
		draw_mid_wm();
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
	} while (TRUE) ;
	ChangeCurLineAlways(0);
	strncpy(st, global_df->row_txt->line, UTTLINELEN);
	for (c=0; st[c] != EOS && st[c] != '>'; c++) ;
	if (st[c] == '>') {
		for (c++; isSpace(st[c]); c++) ;
		strcpy(st, st+c);
	}
    c = strlen(st) - 1;
    while (c >= 0 && (st[c] == NL_C || st[c] == SNL_C || isSpace(st[c]))) c--;
    st[++c] = '\n';
    st[++c] = EOS;
	EndOfFile(-1);
	OutputToScreen(cl_T("\n"));
	SetScrollControl();
	ResetUndos();
	draw_mid_wm();
	wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
	wrefresh(global_df->w1);
}

void DoStdInput(char *st) {
	int c;
	int LastCommand;
	int (*fn)(int);

	do {
		c = ced_getc();
		if (global_df == NULL) {
			if (isPlayS == 6) {
				strcpy(st, ":q");
				isPlayS = 0;
				do_warning(StdDoneMessage, -1);
				return;
			}
			isPlayS = 0;
			continue;
		}
		SaveUndoState(FALSE);
		if (isPlayS) {
			if (isPlayS == 6) {
				strcpy(st, ":q");
				isPlayS = 0;
				do_warning(StdDoneMessage, -1);
				return;
			} else if (isPlayS == -5) {
				DrawCursor(0);
				DrawSoundCursor(0);
				FindFileLine(FALSE, NULL);
				isPlayS = 0;
				continue;
			} else if (isPlayS == -8) {
				DrawCursor(0);
				DrawSoundCursor(0);
				ShowGRA();
				isPlayS = 0;
				continue;
			}
			fn = all_func[isPlayS].func_ptr;
			c = 1;
			isPlayS = 0;
		} else if (globalWhichInput) {
			SaveUndoState(all_func[globalWhichInput].func_ptr == Undo);
			if (all_func[globalWhichInput].func_ptr == MoveDown ||
				all_func[globalWhichInput].func_ptr == MoveUp) {
				if (global_df->cursorCol == 0L)
					global_df->cursorCol = global_df->col_win;
			} else
				global_df->cursorCol = 0L;
			fn = all_func[globalWhichInput].func_ptr;
		} else if (F_key) {
			fn = F_keys[F_key]->func_ptr;
			c = 1;
			F_key = 0;
		} else if (c == 3) {
			c = 13;
			fn = NewLine;
		} else if (c < 524)
			fn = norm_key[c]->func_ptr;
		else
			fn = SelfInsert;

		if (fn == MoveCodeCursorDown || fn == MoveCodeCursorUp || fn == MoveCodeCursorLeft || fn == MoveCodeCursorRight)
			fn = SelfInsert;
		if (fn == SelfInsert			 ||
				fn == HandleESC			 ||
				fn == HandleCTRLX		 ||
				fn == Space				 ||
				fn == GotoLine			 ||
				fn == SearchForward		 ||
				fn == SearchReverse		 ||
				fn == FindSame			 ||
				fn == Replace			 ||
				fn == ExecCommand		 ||
				fn == AproposFile		 ||
				fn == Apropos			 ||
				fn == ToUpperCase		 ||
				fn == ToLowerCase		 ||
				fn == MoveDown			 ||
				fn == MoveUp			 ||
				fn == MoveRight			 ||
				fn == MoveLeft			 ||
				fn == MoveRightWord		 ||
				fn == MoveLeftWord		 ||
				fn == BeginningOfFile	 ||
				fn == EndOfFile			 ||
				fn == BeginningOfWindow	 ||
				fn == EndOfWindow		 ||
				fn == BeginningOfLine	 ||
				fn == EndOfLine			 ||
				fn == NextPage			 ||
				fn == MoveLineUp		 ||
				fn == MoveLineDown		 ||
				fn == PrevPage			 ||
				fn == Undo				 ||
				fn == ChatModeSet		 ||
				fn == DeleteNextChar	 ||
				fn == DeletePrevChar	 ||
				fn == DeletePrevWord	 ||
				fn == DeleteNextWord	 ||
				fn == YankKilled		 ||
				fn == KillLine			 ||
				fn == SelectAll) {
			LastCommand = (*fn)(c);
			if (LastCommand == 0) {
				strcpy(st, ":q");
				return;
			} if (global_df == NULL) {
				strcpy(st, ":q");
				return;
			}
			global_df->LastCommand = LastCommand;
		} else if (fn != NewLine) {
			do_warning(StdInErrMessage, 0);
			RemoveLastUndo();
		} else { /*  if (fn == NewLine) */
			if (StdInWindow != NULL && global_df != NULL && !strcmp(StdInWindow,global_df->fileName)) {
				RemoveLastUndo();
				break;
			} else {
				LastCommand = (*fn)(c);
				if (LastCommand == 0) {
					strcpy(st, ":q");
					return;
				} if (global_df == NULL) {
					strcpy(st, ":q");
					return;
				}
				global_df->LastCommand = LastCommand;
			}
		}
		draw_mid_wm();
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
		if (isKillProgram) {
			strcpy(st, ":q");
			isPlayS = 0;
			return;
		}
	} while (TRUE) ;
	ChangeCurLineAlways(0);
	u_strcpy(st, global_df->row_txt->line, 1024);
	for (c=0; st[c] != EOS && st[c] != '>'; c++) ;
	if (st[c] == '>') {
		for (c++; isSpace(st[c]); c++) ;
		strcpy(st, st+c);
	}
    c = strlen(st) - 1;
    while (c >= 0 && (st[c] == NL_C || st[c] == SNL_C || isSpace(st[c]))) c--;
    st[c+1] = EOS;
	EndOfFile(-1);
	OutputToScreen(cl_T("\n"));
	SetScrollControl();
	ResetUndos();
	draw_mid_wm();
	wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
	wrefresh(global_df->w1);
}
#endif

static int callExecKey(unsigned int c, unsigned int arg, char isAllFunc) {
	int LastCommand = 0;

	if (globalWhichInput) {
		SaveUndoState(all_func[globalWhichInput].func_ptr == Undo);
		if (all_func[globalWhichInput].func_ptr == MoveDown ||
			all_func[globalWhichInput].func_ptr == MoveUp) {
			if (global_df->cursorCol == 0L)
				global_df->cursorCol = global_df->col_win;
		} else
			global_df->cursorCol = 0L;
		LastCommand = (*all_func[globalWhichInput].func_ptr)((int)arg);		
	} else {
		if (isAllFunc == 1) {
			if (c >= 524) {
			} else {
				SaveUndoState(all_func[c].func_ptr == Undo);
				if (all_func[c].func_ptr == MoveDown ||
					  all_func[c].func_ptr == MoveUp) {
					if (global_df->cursorCol == 0L)
						global_df->cursorCol = global_df->col_win;
				} else
					global_df->cursorCol = 0L;
				LastCommand = (*all_func[c].func_ptr)((int)arg);
			}
		} else if (isAllFunc == 2) {
			if (c >= 50) {
			} else {
				SaveUndoState(F_keys[c]->func_ptr == Undo);
				if (F_keys[c]->func_ptr == MoveDown || F_keys[c]->func_ptr == MoveUp) {
					if (global_df->cursorCol == 0L)
						global_df->cursorCol = global_df->col_win;
				} else
					global_df->cursorCol = 0L;
				LastCommand = (*F_keys[c]->func_ptr)((int)arg);
			}
		} else {
			if (c >= 524) {
				SaveUndoState(FALSE);
				global_df->cursorCol = 0L;
				LastCommand = SelfInsert((int)arg);
			} else {
				SaveUndoState(norm_key[c]->func_ptr == Undo);
				if (norm_key[c]->func_ptr == MoveDown ||
					  norm_key[c]->func_ptr == MoveUp) {
					if (global_df->cursorCol == 0L)
						global_df->cursorCol = global_df->col_win;
				} else
					global_df->cursorCol = 0L;
				LastCommand = (*norm_key[c]->func_ptr)((int)arg);
			}
		}
	}
	if (LastCommand == 0)
		return(1); /* quit */
	if (global_df == NULL)
		return(2);
	global_df->LastCommand = LastCommand;
	return(0);
}
/*
return values:
	0 - ALL OK;
  Mac:
	1 - return;
	2,3 - continue;
  PC:
	1,2 - return_Win95_call
	3 - finish_win95_call
*/

int proccessKeys(unsigned int c) {
	register int tKey;

	if (isPlayS) {
		ControlAllControls(FALSE, 0);
		c = 0;
		tKey = isPlayS;
		InKey = 0;
		isPlayS = 0;
		if (!global_df->EditorMode) {
			if (tKey == 17)
				tKey = 42;
			else if (tKey == 19)
				tKey = 70;
			else if (tKey == 16)
				tKey = 43;
			else if (tKey == 18)
				tKey = 77;
		}
		if (tKey == -1) {
			SaveUndoState(FALSE);
			tKey = (int)DefChatMode;
			DefChatMode = global_df->ChatMode;
			global_df->LastCommand = VisitFile(-1);
			DefChatMode = (char)tKey;
		} else if (tKey > -1) {
			if ((tKey=callExecKey((unsigned int)tKey, 1, 1)) != 0)
				return(tKey);
		} else {
			if (tKey != -6)
				SaveUndoState(FALSE);
			if (tKey == -5) {
#ifdef _MAC_CODE
				DrawCursor(0);
				DrawSoundCursor(0);
#endif
				FindFileLine(FALSE, NULL);
			} else if (tKey == -8) {
#ifdef _MAC_CODE
				DrawCursor(0);
				DrawSoundCursor(0);
#endif
				ShowGRA();
			} else
				ExecSoundCom(tKey);
		}
#ifdef _MAC_CODE
		if (global_df != NULL) {
			if (!*global_df->err_message && global_df->SoundWin!= NULL) {
				PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
				if (global_df->SnTr.dtype == 0) {
					PrintSoundWin(0,0,0L);
					DrawSoundCursor(1);
				}
			} else 
				draw_mid_wm();
		}
#endif
		goto EndKey;
	}
	if (F_key) {
		ControlAllControls(FALSE, 0);
		c = 0;
		tKey = F_key;
		InKey = 0;
		F_key = 0;
		if ((tKey=callExecKey((unsigned int)tKey, (unsigned int)tKey, 2)) != 0)
			return(tKey);
#ifdef _MAC_CODE
		if (global_df != NULL) {
			if (!*global_df->err_message && global_df->SoundWin!= NULL) {
				PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
				if (global_df->SnTr.dtype == 0) {
					PrintSoundWin(0,0,0L);
					DrawSoundCursor(1);
				}
			} else 
				draw_mid_wm();
		}
#endif
		goto EndKey;
	}
	strcpy(global_df->err_message, DASHES);

	if (global_df->EditorMode) {
		if (c >= 524)
			;
		else if (norm_key[c]->func_ptr != SelfInsert && 
			norm_key[c]->func_ptr != NewLine  && norm_key[c]->func_ptr != Space  &&
			norm_key[c]->func_ptr != MoveDown && norm_key[c]->func_ptr != MoveUp &&
			norm_key[c]->func_ptr != MoveRight && norm_key[c]->func_ptr != MoveLeft)
			ControlAllControls(FALSE, 0);
		if (c == 3) {
			SaveUndoState(FALSE);
			global_df->LastCommand = NewLine(c);
		} else {
			if ((tKey=callExecKey(c, c, 0)) != 0)
				return(tKey);
		}
	} else {
		ControlAllControls(FALSE, -1);
		if (global_df->CurCode != global_df->RootCodes) {
			if (c >= 524) {
				if ((tKey=callExecKey(c, c, 0)) != 0)
					return(tKey);
			} else if (IsRightCommand(norm_key[c]->func_ptr)) {
				if ((tKey=callExecKey(c, c, 0)) != 0)
					return(tKey);
			} else if (norm_key[c]->func_ptr == NewLine || norm_key[c]->func_ptr == Space) {
				SaveUndoState(FALSE);
				global_df->LastCommand = GetCursorCode(c);
			} else if (norm_key[c]->func_ptr == MoveUp) {
				SaveUndoState(FALSE);
				global_df->LastCommand = MoveCodeCursorUp(c);
			} else if (norm_key[c]->func_ptr == MoveDown) {
				SaveUndoState(FALSE);
				global_df->LastCommand = MoveCodeCursorDown(c);
			} else if (norm_key[c]->func_ptr == MoveLeft) {
				SaveUndoState(FALSE);
				global_df->LastCommand = MoveCodeCursorLeft(c);
			} else if (norm_key[c]->func_ptr == MoveRight) {
				SaveUndoState(FALSE);
				global_df->LastCommand = MoveCodeCursorRight(c);
			} else {
				SaveUndoState(FALSE);
				global_df->LastCommand = IllegalFunction(-1);
				strcpy(global_df->err_message, "+Please finish coding current line first.");
			}
		} else if (global_df->cod_fname == NULL) {
			global_df->LeaveHighliteOn = TRUE;
			if (c >= 524) {
				if ((tKey=callExecKey(c, c, 0)) != 0)
					return(tKey);
			} else if (norm_key[c]->func_ptr == MoveUp) {
				SaveUndoState(FALSE);
				global_df->LastCommand = MoveCodeCursorUp(c);
			} else if (norm_key[c]->func_ptr == MoveDown) {
				SaveUndoState(FALSE);
				global_df->LastCommand = MoveCodeCursorDown(c);
			} else if (norm_key[c]->func_ptr == MoveLeft) {
				SaveUndoState(FALSE);
				global_df->LastCommand = MoveCodeCursorLeft(c);
			} else if (norm_key[c]->func_ptr == MoveRight) {
				SaveUndoState(FALSE);
				global_df->LastCommand = MoveCodeCursorRight(c);
			} else if (norm_key[c]->func_ptr == NewLine || norm_key[c]->func_ptr == Space) {
				SaveUndoState(FALSE);
				global_df->LastCommand = GetCursorCode(c);
			} else {
				if ((tKey=callExecKey(c, c, 0)) != 0)
					return(tKey);
			}
			global_df->LeaveHighliteOn = TRUE;
		} else {
			if (c >= 524) {
				if ((tKey=callExecKey(c, c, 0)) != 0)
					return(tKey);
			} else if (norm_key[c]->func_ptr == NewLine || norm_key[c]->func_ptr == Space) {
				SaveUndoState(FALSE);
				global_df->LastCommand = GetCursorCode(c);
			} else {
				if ((tKey=callExecKey(c, c, 0)) != 0)
					return(tKey);
			}
		}
	}
EndKey:
	if (global_df != NULL) {
		if (FreqCountLimit > 0 && global_df->FreqCount >= FreqCountLimit) {
			SaveCKPFile(TRUE);
			global_df->FreqCount = 0;
		} else if (global_df->FreqCountLimit > 0 && global_df->FreqCount >= global_df->FreqCountLimit) {
			SaveCKPFile(TRUE);
			global_df->FreqCount = 0;
		}
		if (FreqCountLimit <= 0 && global_df->FreqCountLimit <= 0) {
			global_df->FreqCount = 0;
		}
	}
	return(3);
}

#ifdef _WIN32
UINT win95_call(UINT nChar, UINT nRepCnt, UINT nFlags, UINT whichInput) {
	register UINT c;

	DrawCursor(0);
	globalWhichInput = whichInput;
	if (global_df == NULL)
		goto return_Win95_call;
	if (global_df->isExtend)
		global_df->LeaveHighliteOn = TRUE;
	else if (isPlayS == 0)
		global_df->LeaveHighliteOn = FALSE;
	c = nChar;
	if (proccessKeys(c) != 3)
		goto return_Win95_call;
	if (global_df != NULL) {
		if (!*global_df->err_message && global_df->SoundWin != NULL) {
			PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
		} else if (PBC.walk != 1)
			draw_mid_wm();
		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
		if (!global_df->EditorMode && global_df->CurCode != global_df->RootCodes) {
			if (global_df->ScrollBar != '\002')
				ControlAllControls(2, 0);
		} else {
			if (global_df->ScrollBar != '\001')
				ControlAllControls(1, 0);
		}
		cedDFnt.isUTF = global_df->isUTF;
		cedDFnt.Encod = my_FontToScript(global_df->row_txt->Font.FName, global_df->row_txt->Font.CharSet);
	}
return_Win95_call:
	if (global_df != NULL) {
		if (!global_df->LeaveHighliteOn) {
			global_df->row_win2 = 0L;
			global_df->col_win2 = -2L;
			global_df->col_chr2 = -2L;
		}
		global_df->isExtend = 0;
		DrawCursor(4);
		if (global_df->DataChanged)
			GlobalDoc->SetModifiedFlag(TRUE);
		SetScrollControl();
		if (global_df->row_txt == global_df->cur_line) {
			if (global_df->col_txt->prev_char != global_df->head_row)
				global_df->gAtt = global_df->col_txt->prev_char->att;
			else
				global_df->gAtt = 0;
		} else if (global_df->row_txt->att != NULL) {
			if (global_df->col_chr > 0)
				global_df->gAtt = global_df->row_txt->att[global_df->col_chr-1];
			else
				global_df->gAtt = 0;
		} else
			global_df->gAtt = 0;
		if (CheckLeftCol(global_df->col_win)) {
			DrawCursor(0);
			DisplayTextWindow(NULL, 1);
			wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
			DrawCursor(1);
		}
	}
	return(globalWhichInput);
}
#endif /* _WIN32 */

#ifndef _WIN32 
void PrepWindow(short id) {
	if (global_df) {
		SetPortWindowPort(global_df->wind);
/*	if (!global_df->ChatMode) */
		if (CheckLeftCol(global_df->col_win))
			DisplayTextWindow(NULL, 1);

		wmove(global_df->w1, global_df->row_win, global_df->col_win-global_df->LeftCol);
		wrefresh(global_df->w1);
		global_df->isExtend = 0;
		if (!global_df->EditorMode && global_df->CurCode != global_df->RootCodes) {
			if (global_df->ScrollBar != '\002')
				ControlAllControls(2, id);
		} else {
			if (global_df->ScrollBar != '\001')
				ControlAllControls(1, id);
		}
		if (global_df->LastCommand != 1) {
			short tDefScript;
			
			TextFont(global_df->row_txt->Font.FName);
			TextSize(global_df->row_txt->Font.FSize);
			tDefScript = my_FontToScript(global_df->row_txt->Font.FName, global_df->row_txt->Font.CharSet);
			if (!global_df->isUTF && AutoScriptSelect && tDefScript != cedDFnt.Encod) {
				cedDFnt.Encod = tDefScript;
				KeyScript(cedDFnt.Encod);
				SetCurrentFontParams(global_df->row_txt->Font.FName, global_df->row_txt->Font.FSize);
				SetTextWinMenus(TRUE);
			} else if (global_df->LastCommand ==  7 || global_df->LastCommand == 12 || 
					   global_df->LastCommand == 16 || global_df->LastCommand == 17 ||
					   global_df->LastCommand == 18 || global_df->LastCommand == 19 ||
					   global_df->LastCommand == 23 || global_df->LastCommand == 35 ||
					   global_df->LastCommand == 36 || global_df->LastCommand == 41 ||
					   global_df->LastCommand == 44 || global_df->LastCommand == 45 ||
					   global_df->LastCommand == 76 || global_df->LastCommand == 78) {
				SetCurrentFontParams(global_df->row_txt->Font.FName, global_df->row_txt->Font.FSize);
				SetTextWinMenus(TRUE);
			}
		}
	}
}

void mac_call(void) {
	register int c;
	GrafPtr oldPort;
	extern char  isMORXiMode;
	extern SInt16 currentKeyScript;

	InKey = 0;
	ControlAllControls(FALSE, 0);
	GetPort(&oldPort);
	globalWhichInput = 0;
	do {
		PrepWindow(0);
		SetPort(oldPort);

		NonModal = 0;
		if (InKey) {
			c = InKey;
			InKey = 0;
		} else if (global_df == NULL)
			c = wgetch(NULL);
		else
			c = wgetch(global_df->w1);
		NonModal = 1;
		if (isMORXiMode) {
			if (isPlayS == 6) {
				isPlayS = 0;
				do_warning(StdDoneMessage, -1);
				continue;
			}
		}
		if (global_df == NULL) {
			if (ExitCED(1) == 0)
				return;
			continue;
		}

		GetPort(&oldPort);
		SetPortWindowPort(global_df->wind);

		if (!global_df->isUTF && AutoScriptSelect && currentKeyScript != my_FontToScript(global_df->row_txt->Font.FName, global_df->row_txt->Font.CharSet)) {
			FontInfo fi;

			global_df->row_txt->Font.FName = GetScriptVariable(currentKeyScript, smScriptAppFond);
//			global_df->row_txt->Font.FSize = ;
			global_df->row_txt->Font.CharSet = currentKeyScript;
			TextFont(global_df->row_txt->Font.FName);
			TextSize(global_df->row_txt->Font.FSize);
			GetFontInfo(&fi);
			global_df->row_txt->Font.FHeight = fi.ascent + fi.descent + fi.leading + FLINESPACE;
		} else {
			TextFont(global_df->row_txt->Font.FName);
			TextSize(global_df->row_txt->Font.FSize);
		}

//lll		DrawCursor(0);
		if (proccessKeys((unsigned int)c) == 1)
			return;
//lll		DrawCursor(1);
		if (global_df) {
			if (!*global_df->err_message && global_df->SoundWin != NULL) {
				PutSoundStats(global_df->SnTr.WEndF - global_df->SnTr.WBegF);
			} else 
				draw_mid_wm();
		}
	} while (1) ;
}
#endif /* !_WIN32 */
