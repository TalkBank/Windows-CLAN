/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

#if !defined(UNX)
#define _main indentoverlap_main
#define call indentoverlap_call
#define getflag indentoverlap_getflag
#define init indentoverlap_init
#define usage indentoverlap_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#define OVARRAYSIZE 30
#define NUMOFOVS 15
#define TYPE 0
#define ISOVMATCHED 1

#define I_UTT struct indent_utterance
I_UTT {
    char    *sp;
    AttTYPE *attSp;
    char    *line;
    AttTYPE *attLine;
	char openOV[NUMOFOVS][2]; // 0 index = type, 1 index = is matched
	char closeOV[NUMOFOVS][2];
	long ln;
    I_UTT *nextUtt;
} *rootUtt;

struct indent_overlaps {
	int  OvCol;
	int  OvCrs;
	int  openOvIndex;
	char isMatched;
	char isSkip;
	char OvNum;
	I_UTT *OvUtt;
} ;

extern struct tier *defheadtier;
extern char OverWriteFile;

static char indent_ftime, tReplaceFile;
static int indexOV;
static struct indent_overlaps openOv[OVARRAYSIZE+1];


void init(char f) {
	if (f) {
		indent_ftime = TRUE;
		rootUtt = NULL;
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		if (indent_ftime) {
			tReplaceFile = replaceFile;
			indent_ftime = FALSE;
		}
		replaceFile = tReplaceFile;
	}
}

void usage() {
	printf("indents CA overlaps\n");
	printf("Usage: indent [%s] filename(s)\n", mainflgs());
	mainusage(TRUE);
}

static I_UTT *freeUtts(I_UTT *p) {
	I_UTT *t;

	while (p != NULL) {
		t = p;
		p = p->nextUtt;
    	if (t->sp)
    		free(t->sp);
    	if (t->attSp)
    		free(t->attSp);
    	if (t->line)
    		free(t->line);
    	if (t->attLine)
    		free(t->attLine);
		free(t);
	}
	return(NULL);
}

static void indent_exit(int i) {
	rootUtt = freeUtts(rootUtt);
	cutt_exit(i);
}

static char *indent_mkstring(const char *s) {
	char *p;

	if (s == NULL)
		return(NULL);

	if ((p=(char *)malloc(strlen(s)+1)) == NULL) {
		fprintf(stderr,"ERROR: no more memory available.\n");
		indent_exit(0);
	}
	strcpy(p, s);
	return(p);
}

static AttTYPE *indent_mkATTs(AttTYPE *att, const char *s) {
	long i;
	AttTYPE *p;

	if (att == NULL || s == NULL)
		return(NULL);

	if ((p=(AttTYPE *)malloc((strlen(s)+1)*sizeof(AttTYPE))) == NULL) {
		fprintf(stderr,"ERROR: no more memory available.\n");
		indent_exit(0);
	}

	for (i=0; s[i] != EOS; i++)
		p[i] = att[i];
	return(p);
}

static AttTYPE *check_st_atts(const char *s, AttTYPE *att) {
	long i;

	if (att == NULL || s == NULL)
		return(NULL);

	for (i=0; s[i] != EOS; i++) {
		if (att[i] != 0)
			return(att);
	}
	return(NULL);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = INDENT;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);

	rootUtt = freeUtts(rootUtt);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void addUtt(const char *sp, const char *line, AttTYPE *attSp, AttTYPE *attLine, long ln) {
	I_UTT *p;

	attSp = check_st_atts(sp, attSp);
	attLine = check_st_atts(line, attLine);
	if (rootUtt == NULL) {
		if ((rootUtt=NEW(I_UTT)) == NULL) {
			fprintf(stderr,"ERROR: no more core memory available.\n");
			indent_exit(0);
		}
		p = rootUtt;
	} else {
		for (p=rootUtt; p->nextUtt != NULL; p=p->nextUtt) ;
		if ((p->nextUtt=NEW(I_UTT)) == NULL) {
			fprintf(stderr,"ERROR: no more core memory available.\n");
			indent_exit(0);
		}
		p = p->nextUtt;
	}
    p->sp      = NULL;
    p->attSp   = NULL;
    p->line    = NULL;
    p->attLine = NULL;
    p->nextUtt = NULL;
    p->sp      = indent_mkstring(sp);
    p->attSp   = indent_mkATTs(attSp, sp);
    p->line    = indent_mkstring(line);
    p->attLine = indent_mkATTs(attLine, line);
	p->ln      = ln;
}

static void creatNewOV(I_UTT *utt) {
	int i, crCnt, openOvIndex;
	int col;
	char *line;
	short res;

	col = 0;
	crCnt = 0;
	openOvIndex = -1;
	line = utt->line;
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '\t' && (i == 0 || line[i-1] != '\n'))
			line[i] = ' ';
		res = my_CharacterByteType(line, i, &dFnt);
		if (line[i] == (char)0xe2 && line[i+1] == (char)0x8c && line[i+2] == (char)0x88) {
			openOvIndex++;
			if (indexOV >= OVARRAYSIZE) {
				fprintf(stderr, "ERROR: Overlap array too small.\n");
				indent_exit(0);
			}
			if (i >= 3 && line[i-3] == (char)0xe2 && line[i-2] == (char)0x8c && line[i-1] == (char)0x8a)
				openOv[indexOV].OvCol = col - 1;
			else
				openOv[indexOV].OvCol = col;
			openOv[indexOV].OvCrs = crCnt;
			if (line[i+3] >= '0' && line[i+3] <= '9')
				openOv[indexOV].OvNum = line[i+3];
			else
				openOv[indexOV].OvNum = 0;
			if (openOvIndex >= 0 && openOvIndex < NUMOFOVS)
				utt->openOV[openOvIndex][TYPE] = openOv[indexOV].OvNum;
			openOv[indexOV].openOvIndex = openOvIndex;
			openOv[indexOV].isMatched = FALSE;
			openOv[indexOV].isSkip = FALSE;
			openOv[indexOV].OvUtt = utt;
			indexOV++;
			i += 2;
			res = 0;
		}
		if (res == 0 || res == 1) {
			if (line[i] != '\t' || i == 0 || line[i-1] != '\n')
				col++;
		}
		if (line[i] == '\n') {
			col = 0;
			crCnt++;
		}
	}
}

static int adjustForWord(char *line, int pos) {
	for (pos--; pos >= 0 && !uS.isskip(line,pos,&dFnt,MBF) && line[pos] != '[' && line[pos] != ']' && line[pos] != HIDEN_C; pos--) ;
	pos++;
	return(pos);
}

static void prevLineAjusted(int col, int orgCol, int crCnt, I_UTT *tOpenOvUtt) {
	int index, pos;
	AttTYPE att, *tAttLine;

	att_cp(0, templineC, tOpenOvUtt->line, tempAtt, tOpenOvUtt->attLine);
	pos = 0;
	for (index=0; templineC[index] != EOS && crCnt > 0; index++) {
		if (templineC[index] == '\n') {
			index++;
			if (templineC[index] == '\t')
				index++;
			pos = index;
			crCnt--;
			if (crCnt <= 0)
				break;
		}
	}
	if (templineC[index] == EOS)
		pos = orgCol;
	else
		pos = pos + orgCol;
	pos = adjustForWord(templineC, pos);
	att = tempAtt[pos];
	att_shiftright(templineC+pos, tempAtt+pos, col-orgCol);
	for (; orgCol < col; orgCol++) {
		templineC[pos] = ' ';
		tempAtt[pos] = att;
		pos++;
	}
	if (tOpenOvUtt->line != NULL)
		free(tOpenOvUtt->line);
	if (tOpenOvUtt->attLine != NULL)
		free(tOpenOvUtt->attLine);
	tAttLine = check_st_atts(templineC, tempAtt);
    tOpenOvUtt->line    = indent_mkstring(templineC);
    tOpenOvUtt->attLine = indent_mkATTs(tAttLine, templineC);
}

static char handlePrevOV(I_UTT *tCloseOvUtt) {
	int i, posShift, closeOvIndex;
	int col, index;
	char closeOvNum;
	AttTYPE att, *tAttLine;
	short res;

	att_cp(0, templineC, tCloseOvUtt->line, tempAtt, tCloseOvUtt->attLine);
	col = 0;
	i = 0;
	for (index=0; index < indexOV; index++) {
		if (openOv[index].OvNum == 0)
			i++;
	}
	if (i < 2) {
		for (index=0; index < indexOV; index++)
			openOv[index].isMatched = FALSE;
	}
	closeOvIndex = -1;
	for (i=0; templineC[i] != EOS && indexOV > 0; i++) {
		if (templineC[i] == '\t' && (i == 0 || templineC[i-1] != '\n'))
			templineC[i] = ' ';
		if (templineC[i] == '\n')
			col = 0;
		res = my_CharacterByteType(templineC, i, &dFnt);
		if (templineC[i] == (char)0xe2 && templineC[i+1] == (char)0x8c && templineC[i+2] == (char)0x8a) {
			closeOvIndex++;
			if (templineC[i+3] >= '0' && templineC[i+3] <= '9')
				closeOvNum = templineC[i+3];
			else
				closeOvNum = 0;
			if (closeOvIndex >= 0 && closeOvIndex < NUMOFOVS)
				tCloseOvUtt->closeOV[closeOvIndex][TYPE] = closeOvNum;
			for (index=0; index < indexOV; index++) {
				if (openOv[index].OvUtt != NULL && !uS.mStricmp(tCloseOvUtt->sp, openOv[index].OvUtt->sp))
					;
				else
				if (openOv[index].isSkip == FALSE && openOv[index].isMatched == FALSE && openOv[index].OvNum == closeOvNum)
					break;
			}
			if (index < indexOV) {
				if (closeOvNum == 0 && openOv[index].OvNum == closeOvNum)
					openOv[index].isMatched = TRUE;
				if (closeOvIndex >= 0 && closeOvIndex < NUMOFOVS)
					tCloseOvUtt->closeOV[closeOvIndex][ISOVMATCHED] = TRUE;
				if (openOv[index].OvUtt != NULL) {
					if (openOv[index].OvUtt != NULL && openOv[index].openOvIndex >= 0 && openOv[index].openOvIndex < NUMOFOVS)
						openOv[index].OvUtt->openOV[openOv[index].openOvIndex][ISOVMATCHED] = TRUE;
				}
				if (col < openOv[index].OvCol) {
					if (i >= 3 && templineC[i-3] == (char)0xe2 && templineC[i-2] == (char)0x8c && templineC[i-1] == (char)0x88) {
						col--;
						i -= 3;
					}
					posShift = adjustForWord(templineC, i);
					att = tempAtt[posShift];
					att_shiftright(templineC+posShift, tempAtt+posShift, openOv[index].OvCol-col);
					for (; col < openOv[index].OvCol; col++,i++,posShift++) {
						templineC[posShift] = ' ';
						tempAtt[posShift] = att;
					}
				} else if (col > openOv[index].OvCol) {
					posShift = adjustForWord(templineC, i);
					for (i--,posShift--; col>openOv[index].OvCol && posShift>0 && templineC[posShift]==' ' && templineC[posShift-1]==' '; col--,i--,posShift--)
						att_cp(0, templineC+posShift, templineC+posShift+1, tempAtt+posShift, tempAtt+posShift+1);
//						strcpy(templineC+posShift, templineC+posShift+1);
					i++;
					if (col > openOv[index].OvCol && openOv[index].OvCol == 0) {
						if (tCloseOvUtt->line != NULL)
							free(tCloseOvUtt->line);
						if (tCloseOvUtt->attLine != NULL)
							free(tCloseOvUtt->attLine);
						tAttLine = check_st_atts(templineC, tempAtt);
						tCloseOvUtt->line    = indent_mkstring(templineC);
						tCloseOvUtt->attLine = indent_mkATTs(tAttLine, templineC);
						prevLineAjusted(col, openOv[index].OvCol, openOv[index].OvCrs, openOv[index].OvUtt);
						return(TRUE);
					}
				}
				i += 2;
				res = 0;
			}
		} 
		if (res == 0 || res == 1) {
			col++;
		}
	}
	for (i=0; templineC[i] != EOS && indexOV > 0; i++) {
		if (templineC[i] == (char)0xe2 && templineC[i+1] == (char)0x8c && templineC[i+2] == (char)0x88) {
			if (templineC[i+3] >= '0' && templineC[i+3] <= '9')
				closeOvNum = templineC[i+3];
			else
				closeOvNum = 0;
			for (index=0; index < indexOV; index++) {
				if (openOv[index].OvNum == closeOvNum)
					openOv[index].isSkip = TRUE;
			}
		}
	}
	if (tCloseOvUtt->line != NULL)
		free(tCloseOvUtt->line);
	if (tCloseOvUtt->attLine != NULL)
		free(tCloseOvUtt->attLine);
	tAttLine = check_st_atts(templineC, tempAtt);
	tCloseOvUtt->line    = indent_mkstring(templineC);
	tCloseOvUtt->attLine = indent_mkATTs(tAttLine, templineC);
	return(FALSE);
}

void call() {
	int  i;
	short tierCnt;
	I_UTT *cUT, *nextUtt;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		addUtt(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, lineno);
	}
beginAgain:
	for (cUT=rootUtt; cUT != NULL; cUT=cUT->nextUtt) {
		for (i=0; i < NUMOFOVS; i++) {
			cUT->openOV[i][TYPE] = -1;
			cUT->openOV[i][ISOVMATCHED] = FALSE;
			cUT->closeOV[i][TYPE] = -1;
			cUT->closeOV[i][ISOVMATCHED] = FALSE;
		}
	}
	for (cUT=rootUtt; cUT != NULL; cUT=cUT->nextUtt) {
		if (cUT->sp[0] == '*') {
			for (i=0; cUT->sp[i] != ':' && cUT->sp[i] != EOS; i++) ;
			if (cUT->sp[i] == ':') {
				i++;
				if (isSpace(cUT->sp[i])) {
					cUT->sp[i] = '\t';
					cUT->sp[i+1] = EOS;
				}
			}
			tierCnt = 0;
			indexOV = 0;
			creatNewOV(cUT);
			for (nextUtt=cUT->nextUtt; tierCnt < 30 && indexOV > 0 && nextUtt != NULL; nextUtt=nextUtt->nextUtt) {
				tierCnt++;
				if (handlePrevOV(nextUtt)) {
					goto beginAgain;
				}
			}
		}
	}
	for (cUT=rootUtt; cUT != NULL; cUT=cUT->nextUtt) {
		for (i=0; i < NUMOFOVS; i++) {
			if (cUT->openOV[i][TYPE] >= 0 && cUT->openOV[i][ISOVMATCHED] == FALSE) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, cUT->ln);
				if (cUT->openOV[i][TYPE] < '0') {
					fprintf(stderr, "Can't find closing overlap marker for a ");
					if (i == 0) fprintf(stderr, "first ");
					else if (i == 1) fprintf(stderr, "second ");
					else if (i == 2) fprintf(stderr, "third ");
					else fprintf(stderr, "%dth ", i+1);
					fprintf(stderr, "overlap %c%c%c on any following speaker tiers.\n", 0xe2, 0x8c, 0x88);
				} else
					fprintf(stderr, "Can't find closing overlap marker for %c%c%c%c on any following speaker tiers.\n",0xe2,0x8c,0x88,cUT->openOV[i][TYPE]);
				fprintf(stderr, "It might be missing or mismatched or use of opening overlap instead of closing\n");
				fprintf(stderr, "    or match occurs between speakers with the same speaker code.\n");
				replaceFile = FALSE;
			}
			if (cUT->closeOV[i][TYPE] >= 0 && cUT->closeOV[i][ISOVMATCHED] == FALSE) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, cUT->ln);
				if (cUT->closeOV[i][TYPE] < '0') {
					fprintf(stderr, "Can't find opening overlap marker for a ");
					if (i == 0) fprintf(stderr, "first ");
					else if (i == 1) fprintf(stderr, "second ");
					else if (i == 2) fprintf(stderr, "third ");
					else fprintf(stderr, "%dth ", i+1);
					fprintf(stderr, "overlap %c%c%c on any previous speaker tiers.\n", 0xe2, 0x8c, 0x8a);
				} else
					fprintf(stderr, "Can't find opening overlap marker for %c%c%c%c on any previous speaker tiers.\n",0xe2,0x8c,0x8a,cUT->closeOV[i][TYPE]);
				fprintf(stderr, "It might be missing or mismatched or use of closing overlap instead of opening\n");
				fprintf(stderr, "    or match occurs between speakers with the same speaker code.\n");
				replaceFile = FALSE;
			}
		}
		printout(cUT->sp, cUT->line, cUT->attSp, cUT->attLine, FALSE);
	}
	rootUtt = freeUtts(rootUtt);
}
