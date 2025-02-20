/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main quotes_main
#define call quotes_call
#define getflag quotes_getflag
#define init quotes_init
#define usage quotes_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

extern struct tier *defheadtier;
extern char OverWriteFile;

static UTTER *utt;

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
	printf("move quoted text at the end of tier to new a tier\n");
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = QUOTES;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	utt = NEW(UTTER);
	if (utt == NULL)
		fprintf(stderr, "\tERROR: Out of memory\n");
	else {
		bmain(argc,argv,NULL);
		free(utt);
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

static long quotes_FindScope(char *line, long i) {
	if (!uS.isRightChar(line, i, '[', &dFnt, MBF) && i >= 0) {
		for (; !uS.isRightChar(line, i, '[', &dFnt, MBF) && i >= 0; i--) ;
	}
	if (i < 0) {
		fprintf(stderr,"Missing '[' character in file %s\n",oldfname);
		fprintf(stderr,"In tier on line: %ld.\n", lineno);
		fprintf(stderr,"text=%s\n",line);
		return(-2L);
	} else
		i--;
	while (!uS.isRightChar(line, i, '>', &dFnt, MBF) && uS.isskip(line,i,&dFnt,MBF) && i >= 0) {
		if (uS.isRightChar(line, i, ']', &dFnt, MBF) || uS.isRightChar(line, i, '<', &dFnt, MBF))
			break;
		else
			i--;
	}
	if (uS.isRightChar(line, i, '>', &dFnt, MBF)) {
		while (!uS.isRightChar(line, i, '<', &dFnt, MBF) && i >= 0) {
			if (uS.isRightChar(line, i, ']', &dFnt, MBF))
				i = quotes_FindScope(line,i);
			else
				i--;
		}
		if (i < 0) {
			fprintf(stderr,"Missing '<' character in file %s\n", oldfname);
			fprintf(stderr,"In tier on line: %ld.\n", lineno+tlineno);
			fprintf(stderr,"text=%s\n",line);
			return(-2L);
		} else
			i--;
	} else if (i < 0) ;
	else if (uS.isRightChar(line, i, ']', &dFnt, MBF)) {
		i = quotes_FindScope(line,i);
	} else {
		while (!uS.isskip(line,i,&dFnt,MBF) && i >= 0) {
			if (uS.isRightChar(line, i, ']', &dFnt, MBF))
				i = quotes_FindScope(line,i);
			else
				i--;
		}
	}
	return(i);
}

static long findBeg(char *line, AttTYPE *attLine, long i, long *cnt) {
	AttTYPE attFiler[5];
	char AngB, isDone;
	int ABCnt;
	long bSQ, eSQ;

	for (eSQ=i; line[eSQ] != ']' && line[eSQ] != EOS; eSQ++) ;
	if (line[eSQ] == ']')
		eSQ++;
	for (; line[i] != '[' && i > 0L; i--) ;
	if (line[i] != '[')
		return(0L);
	for (bSQ=i-1; isSpace(line[bSQ]) && bSQ > 0L; bSQ--) ;
	if (bSQ == 0L)
		return(0L);

	if (line[bSQ] == '>')
		AngB = TRUE;
	else {
		bSQ++;
		AngB = FALSE;
	}

	i = quotes_FindScope(line, i);
	if (i == -2L)
		return(0L);
	att_cp(0, line+bSQ, line+eSQ, attLine+bSQ, attLine+eSQ);
	for (; isSpace(line[i]) && i >= 0L; i--) ;
	i++;
	for (ABCnt=0; ABCnt < 5; ABCnt++)
		attFiler[ABCnt] = 0;
	if (i == 0) {
		for (; isSpace(line[i]); i++) ;
		if (line[i] == '<' && AngB)
			i++;
		for (; isSpace(line[i]); i++) ;
		att_cp(0, templineC1, "+\" ", tempAtt, attFiler);
		att_cp(0, templineC1+3, line+i, tempAtt+3, attLine+i);
		isDone = TRUE;
	} else {
		if (line[i] == '<' && AngB)
			AngB = FALSE;
		line[i] = EOS;
		att_cp(0, templineC1, line, tempAtt, attLine);
		att_cp(0, templineC1+i, " +\"/.", tempAtt+i, attFiler);
		isDone = FALSE;
	}
	printout(utterance->speaker, templineC1, utterance->attSp, tempAtt, FALSE);


	if (!isDone) {
		for (i++; isSpace(line[i]); i++) ;
		if (line[i] == '<' && AngB)
			i++;
		for (; isSpace(line[i]); i++) ;
		att_cp(0, utt->speaker, utterance->speaker, utt->attSp, utterance->attSp);
		att_cp(0, utt->line, "+\" ", utt->attLine, attFiler);
		att_cp(0, utt->line+3, line+i, utt->attLine+3, attLine+i);
	}


	*cnt = *cnt + 1;
	return(1);
}

static long IDQuoteAndBreak(char *line, AttTYPE *attLine, long *cnt) {
	char sq, dq;
	long i;

	sq = FALSE;
	dq = FALSE;
	for (i=strlen(line)-1; i > 0L; i--) {
		if (line[i] == ']')
			sq = TRUE;
		else if (line[i] == '[')
			sq = FALSE;
		else if (sq && line[i] == '"') {
			i = findBeg(line, attLine, i, cnt);
			break;
		} else if (!sq) {
			if (!uS.isskip(line,i,&dFnt,MBF)) {
				for (; !isSpace(line[i]) && i > 0L; i--) ;
				if (i == 0L)
					break;
				if (isSpace(line[i]) && line[i+1] != '+') {
					i = 0L;
					break;
				}
			}
		}
	}
	return(i);
}

void call() {
	long cnt = 0;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	utt->speaker[0] = EOS;
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			if (utt->speaker[0] != EOS) {
				printout(utt->speaker, utt->line, utt->attSp, utt->attLine, FALSE);
				utt->speaker[0] = EOS;
			}
			if (IDQuoteAndBreak(utterance->line, utterance->attLine, &cnt) <= 0L)
				printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
		} else
			printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
	}
	if (cnt > 0)
		fprintf(stderr, "Changed %ld tiers\n", cnt);
}
