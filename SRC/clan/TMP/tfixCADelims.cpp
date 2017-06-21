/**********************************************************************
	"Copyright 2010 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

extern struct tier *defheadtier;
extern char OverWriteFile;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void usage() {
	printf("remove multiple delims and move one to the end of tier\n");
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}


static long saveDelim(char *delSym, AttTYPE *delAtt, char *line, AttTYPE *attLine, long i) {
	long j, b, e;

	delSym[0] = EOS;
	for (i--; i >= 0L && uS.IsUtteranceDel(line, i); i--) ;
	if (!isalnum(line[i])) {
		for (; i >= 0L && line[i] != '[' && line[i] != ']' && !uS.isskip(line,i,&dFnt,MBF); i--) ;
	}
	b = i + 1;
	for (j=0L, i++; line[i] != EOS && !uS.isskip(line,i,&dFnt,MBF); j++, i++) {
		delSym[j] = line[i];
		delAtt[j] = attLine[i];
	}
	for (; line[i] != EOS && uS.IsUtteranceDel(line, i); j++, i++) {
		delSym[j] = line[i];
		delAtt[j] = attLine[i];
	}
	e = i;
	delSym[j] = EOS;
	att_cp(0,line+b,line+e,attLine+b,attLine+e);
	return(b-1);
}

static void fixDelims(char *line, AttTYPE *attLine) {
	char delSym[128];
	AttTYPE delAtt[128];
	long i, j, len;

	delSym[0] = EOS;
	for (i=0L; line[i] != EOS && line[i] != HIDEN_C; i++) {
		if (line[i] == '[' && line[i+1] == '+')
			break;
		if (line[i] == '\n' && line[i+1] == EOS)
			break;
		if (uS.IsUtteranceDel(line, i)) {
			i = saveDelim(delSym, delAtt, line, attLine, i);
		}
	}
	len = strlen(delSym);
	if (len > 0) {
		if (!uS.isskip(line,i,&dFnt,MBF) || line[i] == '[')
			att_shiftright(line+i, attLine+i, len+1);
		else
			att_shiftright(line+i, attLine+i, len);
		for (j=0L; j < len; j++, i++) {
			line[i] = delSym[j];
			attLine[i] = delAtt[j];
		}
		if (!uS.isskip(line,i,&dFnt,MBF) || line[i] == '[') {
			line[i] = ' ';
			attLine[i] = attLine[i-1];
		}
	}
}

void call() {
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			fixDelims(utterance->line, utterance->attLine);
		}
		printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
	}
}
