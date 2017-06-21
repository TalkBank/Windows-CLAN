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
#define CHAT_MODE 0

extern char OverWriteFile;

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
	}
}

void usage() {
	printf("extract <a href ... ../sounds/*.mov ...> ... </a>\n");
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

static void extractSound(char *href) {
	char isSpFound;
	long i, j;

	j = 0L;
	templineC2[j] = EOS;
	for (i=0L; href[i] != EOS; i++) {
		if (href[i] == '.' && href[i+1] == '.' && href[i+2] == '/' && href[i+3] == 's' && href[i+4] == 'o' &&
			href[i+5] == 'u' && href[i+6] == 'n' && href[i+7] == 'd' && href[i+8] == 's' && href[i+9] == '/') {
			strcpy(templineC2, "../sounds/");
			i += 10;
			break;
		}
	}
	j = strlen(templineC2);
	for (; href[i] != EOS; i++) {
		templineC2[j++] = href[i];
		if (href[i-3] == '.' && href[i-2] == 'M' && href[i-1] == 'O' && href[i] == 'V')
			break;
	}
	if (j > 0L) {
		templineC2[j++] = ' ';
		templineC2[j++] = '=';
		templineC2[j++] = '=';
		templineC2[j++] = '>';
		templineC2[j++] = ' ';
	}
	for (; href[i] != EOS && href[i] != '>'; i++) ;
	if (href[i] == '>')
		i++;
	for (; href[i] != EOS; i++) {
		templineC2[j++] = href[i];
		if (href[i-3] == '<' && href[i-2] == '/' && href[i-1] == 'a' && href[i] == '>') {
			break;
		}
	}
	if (j > 0) {
		isSpFound = FALSE;
		templineC2[j] = EOS;
		for (i=0L; templineC2[i] != EOS;) {
			if (templineC2[i] == '\n')
				templineC2[i] = ' ';

			if (templineC2[i] == '<' && templineC2[i+1] == 'b' && templineC2[i+2] == 'r' && templineC2[i+3] == '>')
				strcpy(templineC2+i, templineC2+i+4);
			else if (isSpace(templineC2[i])) {
				if (isSpFound)
					strcpy(templineC2+i, templineC2+i+1);
				else {
					isSpFound = TRUE;
					i++;
				}
			} else {
				isSpFound = FALSE;
				i++;
			}
		}
		
		fprintf(fpout, "%s\n", templineC2);
	}
}

void call() {
	char isFound;
	long i, j;

	j = 0L;
	isFound = FALSE;
	while (fgets_cr(templineC, UTTLINELEN, fpin) != NULL) {
		for (i=0L; templineC[i] != EOS; i++) {
			if (isFound)
				templineC1[j++] = templineC[i];
			
			if (templineC[i-3] == '<' && templineC[i-2] == '/' && templineC[i-1] == 'a' && templineC[i] == '>' && isFound) {
				templineC1[j] = EOS;
				extractSound(templineC1);
				isFound = FALSE;
				j = 0L;
			}
			if (templineC[i] == '<' && templineC[i+1] == 'a' && templineC[i+2] == ' ' && 
				templineC[i+3] == 'h' && templineC[i+4] == 'r' && templineC[i+5] == 'e' && 
				templineC[i+6] == 'f' && templineC[i+7] == '=') {
				if (isFound && j > 0L) {
					templineC1[j] = EOS;
					extractSound(templineC1);
				}
				isFound = TRUE;
				j = 0L;
				templineC1[j++] = templineC[i];
			}
		}
	}
}
