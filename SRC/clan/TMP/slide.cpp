#define CHAT_MODE 1
/*
#define DEBUG
*/
#include "ced.h"
#include "cu.h"
#if defined(_MAC_CODE)
    #include "c_curses.h"
    #include <unix.h>
#else /* defined(_MAC_CODE) */
    #include <curses.h>
	#include <sys/types.h>
#endif /* defined(_MAC_CODE) */


#ifdef VT200
#include "vt200key.h"
#endif

#ifndef CTRL
#define CTRL(c) (c & 037)
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main slide_main
#define call slide_call
#define getflag slide_getflag
#define init slide_init
#define usage slide_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define WIN_MOVE_BY 69
#define SP_FIELD  10
#define WIN_WIDTH 79

#define TIERS_SLIDE struct tiers_s
TIERS_SLIDE {
    char sp[SPEAKERLEN];
    char sub_name[SPEAKERLEN];
    char *line;
    unsigned int index, last_index;
    unsigned int size;
    TIERS_SLIDE *NextTier;
} ;

/* *********************** slide prototypes ************************* */

void CheckLineLen(TIERS_SLIDE *tt, unsigned int len);
void slide_pr_result(void);
void AddTier(char *sp, char *lastsp, char *lastutt);
void AjustOtherSp(TIERS_SLIDE *tier);
void ReajustOneToOneTier(TIERS_SLIDE *ctier, char *lastutt);
void ReajustError(TIERS_SLIDE *ctier, char *lutt);
void AjustSameSp(char *s_sp, char *c_sp, unsigned int pos, int  diff);
void ShiftAndSpaceOut(char *so, int num);
void DisplayData(void);
void slide_MoveLeftChar(unsigned int *pos, int *cur_col, TIERS_SLIDE *cur_tt);
void slide_MoveLeftWord(unsigned int *pos, int *cur_col, TIERS_SLIDE *cur_tt);
void slide_MoveRightChar(unsigned int *pos, int *cur_col, TIERS_SLIDE *cur_tt);
void slide_MoveRightWord(unsigned int *pos, int *cur_col, TIERS_SLIDE *cur_tt);
TIERS_SLIDE *FindSp(char *sp, char *lastsp);

/* **************************************************************** */

TIERS_SLIDE *root_tier;

void usage() {
    puts("SLIDE");
    printf("Usage: slide [%s] filename(s)\n",mainflgs());
    mainusage();
}

void init(char first) {
    if (first) {
	onlydata = 1;
	addword('\0','\0',"+xxx");
	addword('\0','\0',"+yyy");
    addword('\0','\0',"+www");
	maininitwords();
    }
    if (!combinput || first) {
	root_tier = NULL;
    }
}

void getflag(char *f, char *f1, int *i) {
    f++;
    switch(*f++) {
        default:
            maingetflag(f-2,f1,i);
            break;
    }
}

void slide_MoveLeftChar(unsigned int *pos, int *cur_col, TIERS_SLIDE *cur_tt) {
    if (*pos == 0 && *cur_col == 0) return;
    if (*cur_col == 0) (*pos)--;
    else (*cur_col)--;
}

void slide_MoveLeftWord(unsigned int *pos, int *cur_col, TIERS_SLIDE *cur_tt) {
    if (*pos == 0 && *cur_col == 0) return;
    while ((cur_tt->line[(unsigned)(*cur_col) + (*pos)] == ' '  ||
	    cur_tt->line[(unsigned)(*cur_col) + (*pos)] == '\t') &&
	   (*pos != 0 || *cur_col != 0))
	slide_MoveLeftChar(pos, cur_col, cur_tt);
    if (*pos == 0 && *cur_col == 0) return;
    while (cur_tt->line[(unsigned)(*cur_col) + (*pos)] != ' '  && 
	   cur_tt->line[(unsigned)(*cur_col) + (*pos)] != '\t' &&
	   (*pos != 0 || *cur_col != 0))
	slide_MoveLeftChar(pos, cur_col, cur_tt);
}

void slide_MoveRightChar(unsigned int *pos, int *cur_col, TIERS_SLIDE *cur_tt) {
    if (cur_tt->line[(unsigned)(*cur_col) + (*pos)] == EOS) return;
    if (*cur_col == WIN_WIDTH-SP_FIELD) (*pos)++;
    else (*cur_col)++;
}

void slide_MoveRightWord(unsigned int *pos, int *cur_col, TIERS_SLIDE *cur_tt) {
    if (cur_tt->line[(unsigned)(*cur_col) + (*pos)] == EOS) return;
    while (cur_tt->line[(unsigned)(*cur_col) + (*pos)] == ' '  ||
	   cur_tt->line[(unsigned)(*cur_col) + (*pos)] == '\t')
	slide_MoveRightChar(pos, cur_col, cur_tt);
    while (cur_tt->line[(unsigned)(*cur_col) + (*pos)] != ' '  && 
	   cur_tt->line[(unsigned)(*cur_col) + (*pos)] != '\t' &&
	   cur_tt->line[(unsigned)(*cur_col) + (*pos)] != EOS)
	slide_MoveRightChar(pos, cur_col, cur_tt);
}

void DisplayData() {
    register unsigned int i;
    register int col;
    register int chr;
    unsigned int CmdCnt;
    int num_rows = 0;
    int f_row;
    unsigned int pos;
    unsigned int MaxChar;
    int cur_row, cur_col;
    int total_num_rows;
    TIERS_SLIDE *tt, *cur_tt;
    WINDOW *w;

    initscr(); 
    raw(); 
    nonl();
    noecho();
#if defined(_MAC_CODE) || defined(_WIN32) 
    total_num_rows = WindowPageLength(0) - 2;
#else
    total_num_rows = WindowPageLength() - 2;
#endif
    MaxChar = 0;
    for (tt=root_tier; tt != NULL; tt=tt->NextTier) {
	if (tt->index > MaxChar) MaxChar = tt->index;
	num_rows++;
    }
    f_row = (total_num_rows / 2) - (num_rows / 2);
    w = newwin(total_num_rows, 80, 0, 0, 0);
    wstandend(w);
    wclear(w);
    pos = 0;
    CmdCnt = 1;
    cur_row = 0;
    cur_col = 0;
    while (1) {
	chr = f_row;
	for (tt=root_tier; tt != NULL; tt=tt->NextTier) {
	    if (chr == cur_row + f_row) cur_tt = tt;
	    mvwaddstr(w,chr,0,tt->sp);
	    waddstr(w,"     ");
	    col = SP_FIELD;
	    for (i=pos; tt->line[i] && col < WIN_WIDTH; i++) {
		if (tt->line[i]== '\t' || tt->line[i]== '\n')
			waddch(w, ' ',NULL,0L,0);
		else
			waddch(w, tt->line[i],NULL,0L,0);
		col++;
	    }
	    wclrtoeol(w);
	    chr++;
	}
	sprintf(uttline,
	    "SLIDE: window #%d, character %u out of %u; cmd repeat %u",
	    ((pos+cur_col)/WIN_MOVE_BY) + 1, pos+cur_col, MaxChar, CmdCnt);
	mvwaddstr(w,chr+1,0,uttline);
	wclrtoeol(w);
	wclrtobot(w);
#if defined(_MAC_CODE) || defined(_WIN32) 
	wmove(w, (long)(cur_row+f_row), cur_col+SP_FIELD);
#else
	wmove(w, cur_row+f_row, cur_col+SP_FIELD);
#endif
	wrefresh(w);

	chr = wgetch(w);
#ifdef VT200
        chr = VT200key(chr,w);
#endif
	if (chr == CTRL('N')
#if defined(_MAC_CODE)
	    || chr == 31
#endif
#ifdef VT200
            || chr == KEY_ARROW_DOWN
#endif
	    ) { /* move one line down */
	    if (++cur_row >= num_rows) cur_row = 0;
	    for (chr=0, tt=root_tier; tt != NULL; chr++, tt=tt->NextTier) {
		if (chr == cur_row) { cur_tt = tt; break; }
	    }
	    if ((unsigned)cur_col+pos > cur_tt->index) {
		if (pos > cur_tt->index) {
		    register int i;
		    pos = cur_tt->index;
		    for (i=0; i < WIN_WIDTH-SP_FIELD && pos > 0; i++, pos--) ;
		}
		cur_col = (int)(cur_tt->index - pos);
	    }
	    CmdCnt = 1;
	} else if (chr == CTRL('P')
#if defined(_MAC_CODE) 
	    || chr == 30
#endif
#ifdef VT200 
	    || chr == KEY_ARROW_UP
#endif
	    ) { /* move one line up */
	    if (--cur_row < 0) cur_row = num_rows - 1;
	    for (chr=0, tt=root_tier; tt != NULL; chr++, tt=tt->NextTier) {
		if (chr == cur_row) { cur_tt = tt; break; }
	    }
	    if ((unsigned)cur_col+pos > cur_tt->index) {
		if (pos > cur_tt->index) {
		    register int i;
		    pos = cur_tt->index;
		    for (i=0; i < WIN_WIDTH-SP_FIELD && pos > 0; i++, pos--) ;
		}
		cur_col = (int)(cur_tt->index - pos);
	    }
	    CmdCnt = 1;
	} else if (chr == CTRL('F')
#ifdef VT200
	    || chr == KEY_ARROW_RIGHT
#endif
	    ) { /* move right */
	    for (i=CmdCnt; i > 0; i--)
		slide_MoveRightChar(&pos, &cur_col, cur_tt);
	    CmdCnt = 1;
	} else if (chr == CTRL('B')
#ifdef VT200
	    || chr == KEY_ARROW_LEFT
#endif
	    ) { /* move left */
	    for (i=CmdCnt; i > 0; i--)
		slide_MoveLeftChar(&pos, &cur_col, cur_tt);
	    CmdCnt = 1;
	} else if (chr == '\033') {
	    chr = wgetch(w);
#ifdef VT200
            chr = VT200key(chr,w);
#endif
	    if (chr == 'f' || chr == 'F') {
		for (i=CmdCnt; i > 0; i--)
		    slide_MoveRightWord(&pos, &cur_col, cur_tt);
		CmdCnt = 1;
	    } else if (chr == 'b' || chr == 'B') {
		for (i=CmdCnt; i > 0; i--)
		    slide_MoveLeftWord(&pos, &cur_col, cur_tt);
		CmdCnt = 1;
	    } else if (chr == 'a' || chr == 'A') {
		pos = 0; cur_col = 0;
		CmdCnt = 1;
	    } else if (chr == 'e' || chr == 'E') {
		register int i;
		pos = cur_tt->index;
		for (i=0; i < WIN_WIDTH-SP_FIELD && pos > 0; i++, pos--) ;
		cur_col = cur_tt->index - pos;
		CmdCnt = 1;
	    }
	} else if (FALSE
#if defined(_MAC_CODE)
	    || chr == 29
#endif
	    ) { /* move right word */
	    for (i=CmdCnt; i > 0; i--)
		slide_MoveRightWord(&pos, &cur_col, cur_tt);
	    CmdCnt = 1;
	} else if (FALSE
#if defined(_MAC_CODE)
	    || chr == 28
#endif
	    ) { /* move left word */
	    for (i=CmdCnt; i > 0; i--)
		slide_MoveLeftWord(&pos, &cur_col, cur_tt);
	    CmdCnt = 1;
	} else if (chr == CTRL('A')) { /* beginning of line */
	    if (cur_col == 0)
		for (i=0; i < WIN_MOVE_BY && pos > 0; i++) pos--;
	    cur_col = 0;
	    CmdCnt = 1;
	} else if (chr == CTRL('E')) { /* end of line */
	    if (cur_col == WIN_WIDTH-SP_FIELD) {
		pos += WIN_MOVE_BY;
		if (pos+cur_col-SP_FIELD > cur_tt->index)
		    pos = cur_tt->index - (WIN_WIDTH - SP_FIELD);
	    } else {
		cur_col = WIN_WIDTH - SP_FIELD;
		if (pos+cur_col > cur_tt->index) cur_col= cur_tt->index - pos;
	    }
	    CmdCnt = 1;
	} else if (chr == CTRL('X')) {
	    chr = wgetch(w);
#ifdef VT200
            chr = VT200key(chr,w);
#endif
	    if (chr == CTRL('C')) break;
	} else if (chr == CTRL('U')) {
	    CmdCnt += 4;
	} else if (chr == CTRL('G')) {
	    CmdCnt = 1;
	} else if (chr == CTRL('Q')
#if defined(_MAC_CODE)
	    || chr == 'q' /* change to clear key */
#endif
#ifdef VT200   /* Remember that CTRL-Q serves as XON on many systems! */
	    || chr == KEY_F6    
	    || chr == 'q' /* change to clear key */
#endif
	    ) { /* end second window */
	    break;
	}
    }
    werase(w);    wrefresh(w);    delwin(w);
    clear();
    refresh();
    endwin();
}

void slide_pr_result(void) {
    register int i;
    TIERS_SLIDE *tt;

    if (fpout != stdout) {
	for (tt=root_tier; tt != NULL; tt=tt->NextTier) {
	    fputs(tt->sp, fpout);
	    if (*tt->sub_name) fputs(tt->sub_name, fpout);
	    else fprintf(fpout, "     ");
	    for (i=0; tt->line[i]; i++) {
		if (tt->line[i]== '\t' || tt->line[i]== '\n') putc(' ',fpout);
		else putc(tt->line[i], fpout);
	    }
	    putc('\n', fpout);
	}
    } else DisplayData();

    while (root_tier != NULL) {
	tt = root_tier;
	root_tier = root_tier->NextTier;
	free(tt->line);
	free(tt);
    }
}

TIERS_SLIDE *FindSp(char *sp, char *lastsp) {
    register unsigned int i;
    TIERS_SLIDE *tt, *tt1;

    if (root_tier == NULL) {
	if ((tt=NEW(TIERS_SLIDE)) == NULL) out_of_mem();
	root_tier = tt;
    } else {
	for (tt=root_tier; 1; tt=tt->NextTier) {
	    if (strcmp(tt->sp, sp) == 0) {
		if (*tt->sub_name == EOS) return(tt);
		else if (strcmp(tt->sub_name, lastsp) == 0) return(tt);
	    }
	    if (tt->NextTier == NULL) break;
	}
	if ((tt->NextTier=NEW(TIERS_SLIDE)) == NULL) out_of_mem();
	tt = tt->NextTier;
    }
    if ((tt->line=(char *)malloc(UTTLINELEN+1)) == NULL) out_of_mem();
    strcpy(tt->sp, sp);
    if (*sp == '*') tt->sub_name[0] = EOS;
    else strcpy(tt->sub_name, lastsp);
    tt->line[0] = EOS;
    tt->index = 0;
    tt->last_index = 0;
    tt->size = UTTLINELEN;
    tt->NextTier = NULL;
    for (tt1=root_tier; tt1 != NULL; tt1=tt1->NextTier) {
	if (*tt->sp == '*' && *tt1->sp == '*') {
	    CheckLineLen(tt, tt1->index);
	    for (i=0; i < tt1->index; i++) tt->line[i] = ' ';
	    tt->line[i] = EOS;
	    tt->index = tt1->index;
	    tt->last_index = tt->index;
	    break;
	} else if (*tt->sp == '%' && !strcmp(tt1->sp, tt->sub_name)) {
	    CheckLineLen(tt, tt1->last_index);
	    for (i=0; i < tt1->last_index; i++) tt->line[i] = ' ';
	    tt->line[i] = EOS;
	    tt->index = tt1->last_index;
	    break;
	}
    }
    return(tt);
}

void CheckLineLen(TIERS_SLIDE *tt, unsigned int len) {
    char *s;

    if (tt->size - tt->index <= len) {
	s = tt->line;
	tt->size = tt->size + len + UTTLINELEN;
	if ((tt->line=(char *)malloc(tt->size+1)) == NULL) out_of_mem();
	strcpy(tt->line, s);
	free(s);
    }
}

void AjustOtherSp(TIERS_SLIDE *tier) {
    register unsigned int i;
    char *sp;
    TIERS_SLIDE *tt;

    if (*tier->sp == '*') sp = tier->sp;
    else sp = tier->sub_name;
    for (tt=root_tier; tt != NULL; tt=tt->NextTier) {
	if ((*tt->sp == '*' && strcmp(tt->sp, sp)) ||
	    (*tt->sp != '*' && strcmp(tt->sub_name, sp))) {
	    CheckLineLen(tt, tier->index - tt->index);
	    for (i=tt->index; i < tier->index; i++) tt->line[i] = ' ';
	    tt->line[i] = EOS;
	    tt->index = i;
	    tt->last_index = i;
	}
    }
}

void AjustSameSp(char *s_sp, char *c_sp, unsigned int pos, int  diff) {
    TIERS_SLIDE *tt;

    for (tt=root_tier; tt != NULL; tt=tt->NextTier) {
	if (!strcmp(tt->sub_name, s_sp) && strcmp(tt->sp, c_sp)) {
	    CheckLineLen(tt, diff);
	    ShiftAndSpaceOut(tt->line+pos, diff);
	    tt->index = tt->index + diff;
	}
    }
}

void ShiftAndSpaceOut(char *so, int num) {
    register char *sl;

    if (num == 0) return;
    for (sl=so; *sl; sl++) ;
    while (sl >= so) {
	sl[num] = sl[0];
	sl--;
    }
    for (; num > 0; num--) *so++ = ' ';
}

void ReajustError(TIERS_SLIDE *ctier, char *lutt) {
    register int ui;
    register int sdiff;
    register int cdiff;
    char FirstAjustPart;
    char *beg, *tsline, *sline, *cline;
    TIERS_SLIDE *stier;

    FirstAjustPart = TRUE;
    stier = FindSp(ctier->sub_name, "");
    sline = stier->line+stier->last_index;
    cline = ctier->line+ctier->last_index;
    ui = 0;
    while (1) {
	if (FirstAjustPart) {
	    FirstAjustPart = FALSE;
	    if (*sline== EOS || *(sline+1) == EOS) break;
	    while (*cline == ' ' || *cline == '\t' || *cline == '\n') cline++;
	    while (*(sline+2) != EOS) {
		if (*sline == '[' && *(sline+1) == '*') {
		    tsline = sline + 3;
		    sline--;
		    while (uS.isskip(stier->line+stier->last_index,(int)(sline-(stier->line+stier->last_index)),dFnt.Encod,MBF) || *sline == '\n') sline--;
		    beg = stier->line+stier->last_index;
		    while (!uS.isskip(stier->line+stier->last_index,(int)(sline-(stier->line+stier->last_index)),dFnt.Encod,MBF) && sline > beg) sline--;
		    if (uS.isskip(stier->line+stier->last_index, (int)(sline-(stier->line+stier->last_index)),dFnt.Encod,MBF)) sline++;
		    break;
		}
		sline++;
	    }
	    if (*(sline+2) == EOS || *cline == EOS) break;
	} else {
	    FirstAjustPart = TRUE;
	    while (*cline != ';' && *cline) cline++;
	    if (*cline == ';') {
		cline++;
		while (*cline==' ' || *cline== '\t' || *cline== '\n') cline++;
	    }
	    sline = tsline;
	    while (*sline == ' ' || *sline == '\t' || *sline == '\n') sline++;
	}
	sdiff = (int)(sline - stier->line);
	cdiff = (int)(cline - ctier->line);
/*
printf("1; FP=%d;\n", FirstAjustPart);
printf("1; sdiff=%d;sline=%s; cdiff=%d;cline=%s;\n",sdiff,sline,cdiff,cline);
*/
	if (cdiff > sdiff) {
	    cdiff = cdiff - sdiff;
	    CheckLineLen(stier, cdiff);
	    ShiftAndSpaceOut(sline, cdiff);
	    ShiftAndSpaceOut(lutt+ui, cdiff);
	    AjustSameSp(stier->sp,ctier->sp,(unsigned int)(sline-stier->line),cdiff);
	    sline += cdiff;
	    ui += cdiff;
	    stier->index = stier->index + cdiff;
	} else if (cdiff < sdiff) {
	    cdiff = sdiff - cdiff;
	    CheckLineLen(ctier, cdiff);
	    ShiftAndSpaceOut(cline, cdiff);
	    cline += cdiff;
	    ctier->index = ctier->index + cdiff;
	}
    }
}

void ReajustOneToOneTier(TIERS_SLIDE *ctier, char *lutt) {
    register int ui;
    register int sdiff;
    register int cdiff;
    char *sline, *cline;
    TIERS_SLIDE *stier;

    stier = FindSp(ctier->sub_name, "");
    sline = stier->line+stier->last_index;
    cline = ctier->line+ctier->last_index;
    ui = 0;
/*
printf("1; s_sp=%s; lutt=%s; st_line=%s;\n", stier->sp, lutt, stier->line);
printf("1; c_sp=%s; ct_line=%s;\n", ctier->sp, ctier->line);
printf("0; sline=%s; cline=%s;\n", sline, cline);
*/
    while (*sline && *cline) {
	while (*sline == ' ' || *sline == '\t' || *sline == '\n') {
	    ui++; sline++;
	}
	while (*cline == ' ' || *cline == '\t' || *cline == '\n') cline++;
	sdiff = (int)(sline - stier->line);
	cdiff = (int)(cline - ctier->line);
/*
printf("1; sdiff=%d;sline=%s; cdiff=%d; cline=%s;\n",sdiff,sline,cdiff,cline);
*/
	if (cdiff > sdiff) {
	    cdiff = cdiff - sdiff;
	    CheckLineLen(stier, cdiff);
	    ShiftAndSpaceOut(sline, cdiff);
	    ShiftAndSpaceOut(lutt+ui, cdiff);
	    AjustSameSp(stier->sp,ctier->sp,(unsigned int)(sline-stier->line),cdiff);
	    sline += cdiff;
	    ui += cdiff;
	    stier->index = stier->index + cdiff;
	    if (lutt[ui] == ' ' || lutt[ui] == '\n') cline--;
/*
printf("1.1; sdiff=%d; sline=%s;\n", sdiff, sline);
*/
	} else if (cdiff < sdiff) {
	    cdiff = sdiff - cdiff;
	    CheckLineLen(ctier, cdiff);
	    ShiftAndSpaceOut(cline, cdiff);
	    cline += cdiff;
	    ctier->index = ctier->index + cdiff;
	    if (lutt[ui] == ' ' || lutt[ui] == '\n') cline--;
/*
printf("1.2; cdiff=%d; cline=%s;\n", cdiff, cline);
*/
	}
/*
printf("2; sdiff=%d;sline=%s; cdiff=%d; cline=%s;\n",sdiff,sline,cdiff,cline);
*/
	while (*sline != ' ' && *sline != '\t' && *sline != '\n' && *sline) {
	    ui++; sline++;
	}
	while (*cline!=' ' && *cline!='\t' && *cline!='\n' && *cline) cline++;
    }
    
/*    sdiff = (int)(sline - stier->line); */
/*    cdiff = (int)(cline - ctier->line); */
      sdiff = strlen(stier->line);
      cdiff = strlen(ctier->line);
/*
printf("2; s_sp=%s; lutt=%s; st_line=%s;\n", stier->sp, lutt, stier->line);
printf("2; c_sp=%s; ct_line=%s;\n", ctier->sp, ctier->line);
*/
    if (cdiff > sdiff) {
	cdiff = cdiff-sdiff;
	CheckLineLen(stier, cdiff);
	ShiftAndSpaceOut(sline, cdiff);
	ShiftAndSpaceOut(lutt+ui, cdiff);
	AjustSameSp(stier->sp,ctier->sp,(unsigned int)(sline-stier->line),cdiff);
	sline += cdiff;
	ui += cdiff;
	stier->index = stier->index + cdiff;
    } else if (cdiff < sdiff) {
	cdiff = cdiff - sdiff;
	CheckLineLen(ctier, cdiff);
	ShiftAndSpaceOut(cline, cdiff);
	cline += cdiff;
	ctier->index = ctier->index + cdiff;
    }
/*
printf("3; s_sp=%s; lutt=%s; st_line=%s;\n", stier->sp, lutt, stier->line);
printf("3; c_sp=%s; ct_line=%s;\n", ctier->sp, ctier->line);
*/
}

void AddTier(char *sp, char *lastsp, char *lutt) {
    unsigned int len;
    TIERS_SLIDE *tier;

    tier = FindSp(sp, lastsp);
    len = (unsigned int)strlen(utterance->line);
    CheckLineLen(tier, len);
    strcat(tier->line, utterance->line);
    tier->index = tier->index + len;
    if (*sp == '%') {
	if (partcmp(utterance->speaker, "%err:", FALSE)) ReajustError(tier, lutt);
	else ReajustOneToOneTier(tier, lutt);
    }
    AjustOtherSp(tier);
}

void call() {
    char LastSp[SPEAKERLEN];
    char LastUttline[UTTLINELEN+UTTLINELEN];

	currentatt = 0;
    currentchar = (char)getc_cr(fpin, &currentatt);
    while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
	if (*utterance->speaker == '*') uS.uppercasestr(utterance->speaker, dFnt.Encod, MBF);
	else if (*utterance->speaker == '%') uS.lowercasestr(utterance->speaker, dFnt.Encod, MBF);
	uS.remblanks(utterance->speaker);
	if (*utterance->speaker == '*') {
	    strcpy(LastSp,utterance->speaker);
	    strcpy(LastUttline, uttline);
	}
	AddTier(utterance->speaker, LastSp, LastUttline);
    }
#ifdef DEBUG
puts("press RETURN"); getc(stdin);
#endif
    if (!combinput) slide_pr_result();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    CLAN_PROG_NUM = SLIDE;
    chatmode = CHAT_MODE;
    OnlydataLimit = 0;
    UttlineEqUtterance = FALSE;
    bmain(argc,argv,slide_pr_result);
}
