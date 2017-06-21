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

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
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
	printf("Usage: tempStripBCH [%s] filename(s)\n",mainflgs());
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

static char foundXXXAnywhere(char *line) {
	int pos;

	for (pos=0; line[pos] != EOS; pos++) {
		if (pos == 0 || uS.isskip(line,pos-1,FALSE)) {
			if (uS.isskip(line,pos+3,FALSE) || line[pos+3] == EOS) {
				strncpy(templineC2, line+pos, 3);
				templineC2[3] = EOS;
				uS.lowercasestr(templineC2, FALSE);
				if (strcmp(templineC2, "xxx") == 0 ||
					strcmp(templineC2, "yyy") == 0 ||
					strcmp(templineC2, "www") == 0)
					return(TRUE);
			}
		}
		if (pos == 0 || uS.isskip(line,pos-1,FALSE)) {
			if (uS.isskip(line,pos+7,FALSE) || line[pos+7] == EOS) {
				strncpy(templineC2, line+pos, 7);
				templineC2[7] = EOS;
				uS.lowercasestr(templineC2, FALSE);
				if (strcmp(templineC2, "unk|xxx") == 0 ||
					strcmp(templineC2, "unk|yyy") == 0 ||
					strcmp(templineC2, "unk|www") == 0)
					return(TRUE);
			}
		}
	}
	return(FALSE);	
}

static int foundBCHAndRemove(char *line, int pos) {
	int orgPos;

	orgPos = pos;
	pos++;
	if (line[pos] != '+')
		return(orgPos);
	for (pos++; isSpace(line[pos]); pos++) ;
	strncpy(templineC, line+pos, 3);
	templineC[3] = EOS;
	uS.lowercasestr(templineC, FALSE);
	if (strcmp(templineC, "bch") != 0)
		return(orgPos);
	for (pos += 3; isSpace(line[pos]); pos++) ;
	if (line[pos] != ']')
		return(orgPos);
	strcpy(line+orgPos, line+pos+1);
	return(orgPos-1);	
}

static void findAndRemoveBCH(char *line) {
	int pos;
	
	for (pos=0; line[pos] != EOS; pos++) {
		if (line[pos] == '[') {
			pos = foundBCHAndRemove(line, pos);
		}
	}
}

void call() {
	long ln = 0L;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (foundXXXAnywhere(utterance->line)) {
			findAndRemoveBCH(utterance->line);
			printout(utterance->speaker, utterance->line, NULL, NULL, 0, FALSE);
		} else
			printout(utterance->speaker, utterance->line, NULL, NULL, 0, FALSE);
	}
}
