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
#define CHAT_MODE 1

#if defined(_MAC_CODE)
ClanProgInfo gToolInfo = {
	_main,
	usage,
	getflag,
	init,
	call,
	NULL
};
#endif /* _MAC_CODE */

extern struct tier *defheadtier;
extern char LeaveSlashes;

char mainCopy[UTTLINELEN];

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		onlydata = 1;
		stout = FALSE;
		FilterTier = 1;
		LeaveSlashes = '\0';
		LocalTierSelect = TRUE;
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  /* list R6 in InitOptions funtion in cutt.c */
//		addword('\0','\0',"+xxx");
//		addword('\0','\0',"+yyy");
//		addword('\0','\0',"+www");
		addword('\0','\0',"+xx");
		addword('\0','\0',"+yy");
		addword('\0','\0',"+0*");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		nomap = TRUE;
	}
}

void usage() {
	printf("Usage: tempAddxxx [%s] filename(s)\n",mainflgs());
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
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

static char foundXXXAnywhere(char *mainCopy) {
	int pos;

	for (pos=0; mainCopy[pos] != EOS; pos++) {
		if (pos == 0 || uS.isskip(mainCopy,pos-1,FALSE)) {
			if (uS.isskip(mainCopy,pos+3,FALSE) || mainCopy[pos+3] == EOS) {
				strncpy(templineC2, mainCopy+pos, 3);
				templineC2[3] = EOS;
				uS.lowercasestr(templineC2, FALSE);
				if (strcmp(templineC2, "xxx") == 0 ||
					strcmp(templineC2, "yyy") == 0 ||
					strcmp(templineC2, "www") == 0)
					return(TRUE);
			}
		}
	}
	return(FALSE);	
}

static char foundXXX(char *mainCopy, int pos) {
	if (!uS.isskip(mainCopy,pos+3,FALSE) && mainCopy[pos+3] != EOS)
		return(FALSE);
	strncpy(templineC, mainCopy+pos, 3);
	templineC[3] = EOS;
	uS.lowercasestr(templineC, FALSE);
	if (strcmp(templineC, "xxx") == 0 ||
		strcmp(templineC, "yyy") == 0 ||
		strcmp(templineC, "www") == 0)
		return(TRUE);
	return(FALSE);	
}

static char foundXXXAlready(char *utt, int pos) {
	if (utt[pos] == 'u' && utt[pos+1] == 'n' && utt[pos+2] == 'k' && utt[pos+3] == '|')
		pos += 4;
	strncpy(templineC2, utt+pos, 3);
	templineC2[3] = EOS;
	uS.lowercasestr(templineC2, FALSE);
	if (strcmp(templineC2, templineC) == 0) {
		if (uS.isskip(utt,pos+3,FALSE) || utt[pos+3] == EOS)
			return(TRUE);
	}
	return(FALSE);	
}

static int AddXXXToOut(char *mainCopy, int posMain, char *templineC3, int posOut) {
	char first;

	templineC3[posOut] = EOS;
	posOut--;
	while (uS.isskip(templineC3, posOut, FALSE) && posOut >= 0)
		posOut--;
	posOut++;
	first = (posOut == 0);
	strcpy(templineC2, templineC3+posOut);
	if (!first)
		templineC3[posOut++] = ' ';
	templineC3[posOut++] = 'u';
	templineC3[posOut++] = 'n';
	templineC3[posOut++] = 'k';
	templineC3[posOut++] = '|';
	while (!uS.isskip(mainCopy, posMain, FALSE) && mainCopy[posMain] != EOS) {
		templineC3[posOut++] = mainCopy[posMain++];
	}
	if (first)
		templineC3[posOut++] = ' ';
	templineC3[posOut] = EOS;
	strcat(templineC3, templineC2);
	return(strlen(templineC3));
}

static void copyXXX(char *mainCopy, long ln) {
	int posMain, posUtt, posOut;
	char isSkipUtt;

	posMain = 0;
	posUtt = 0;
	posOut = 0;
	while (uS.isskip(mainCopy, posMain, FALSE) && mainCopy[posMain] != EOS)
		posMain++;
	while (uS.isskip(uttline, posUtt, FALSE) && uttline[posUtt] != EOS)
		templineC3[posOut++] = uttline[posUtt++];
	while (1) {
		isSkipUtt = TRUE;
		if (foundXXX(mainCopy, posMain)) {
			if (!foundXXXAlready(uttline, posUtt)) {
				posOut = AddXXXToOut(mainCopy, posMain, templineC3, posOut);
				isSkipUtt = FALSE;
			}
		}
		while (!uS.isskip(mainCopy, posMain, FALSE) && mainCopy[posMain] != EOS)
			posMain++;
		while (uS.isskip(mainCopy, posMain, FALSE) && mainCopy[posMain] != EOS)
			posMain++;
		if (isSkipUtt) {
			while (!uS.isskip(uttline, posUtt, FALSE) && uttline[posUtt] != EOS)
				templineC3[posOut++] = uttline[posUtt++];
			while (uS.isskip(uttline, posUtt, FALSE) && uttline[posUtt] != EOS)
				templineC3[posOut++] = uttline[posUtt++];
		}
		if (uttline[posUtt] == EOS && mainCopy[posMain] == EOS)
			break;
	}
	templineC3[posOut] = EOS;
}

static void fixXXToXXX(void) {
	int i, j;
	
	for (i=0, j=0; utterance->line[i]; i++) {
		if (i == 0 || uS.isskip(utterance->line,i-1,FALSE) || utterance->line[i-1] == '|') {
			strncpy(templineC2, utterance->line+i, 2);
			templineC2[2] = EOS;
			uS.lowercasestr(templineC2, FALSE);
			if (strcmp(templineC2, "xx") == 0) {
				if (uS.isskip(utterance->line,i+2,FALSE) || utterance->line[i+2] == EOS) {
					templineC[j++] = 'x';
					templineC[j++] = 'x';
					templineC[j++] = 'x';
					i += 1;
				} else
					templineC[j++] = utterance->line[i];
			} else if (strcmp(templineC2, "yy") == 0) {
				if (uS.isskip(utterance->line,i+2,FALSE) || utterance->line[i+2] == EOS) {
					templineC[j++] = 'y';
					templineC[j++] = 'y';
					templineC[j++] = 'y';
					i += 1;
				} else
					templineC[j++] = utterance->line[i];
			} else if (strcmp(templineC2, "ww") == 0) {
				if (uS.isskip(utterance->line,i+2,FALSE) || utterance->line[i+2] == EOS) {
					templineC[j++] = 'w';
					templineC[j++] = 'w';
					templineC[j++] = 'w';
					i += 1;
				} else
					templineC[j++] = utterance->line[i];
			} else
				templineC[j++] = utterance->line[i];
		} else
			templineC[j++] = utterance->line[i];
	}
	templineC[j] = EOS;
	strcpy(utterance->line, templineC);
	if (uttline != utterance->line)
		strcpy(uttline,utterance->line);
}

void call() {
	long ln = 0L;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		fixXXToXXX();
		filterData(utterance->speaker,uttline);
		if (partcmp(utterance->speaker,"%trn:",FALSE)) {
			if (foundXXXAnywhere(mainCopy)) {
				strcpy(uttline, utterance->line);
				copyXXX(mainCopy, ln);
				printout(utterance->speaker, templineC3, NULL, NULL, 0, FALSE);
			} else
				printout(utterance->speaker, utterance->line, NULL, NULL, 0, FALSE);
		} else {
			if (utterance->speaker[0] == '*') {
				ln = lineno;
				strcpy(mainCopy, uttline);
			}
			printout(utterance->speaker, utterance->line, NULL, NULL, 0, FALSE);
		}
	}
}
