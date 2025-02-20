/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "ced.h"
#include "cu.h"
/* // NO QT
#ifdef _WIN32
	#include <TextUtils.h>
#endif
*/
#if !defined(UNX)
#define _main d2u_main
#define call d2u_call
#define getflag d2u_getflag
#define init d2u_init
#define usage d2u_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char PreserveFileTypes;

void usage() {
	puts("dos2unix converts DOS CRs to Unix ones");
	printf("Usage: dos2unix [%s] filename(s)\n",mainflgs());
	mainusage(FALSE);
	printf("\nExample: dos2unix *.h *.cpp\n");
	printf("\tdos2unix -re -1 *.h *.cpp\n");
	cutt_exit(0);
}

void init(char s) {
	if (s) {
		stout = FALSE;
		onlydata = 3;
		PreserveFileTypes = TRUE;
		replaceFile = TRUE;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = DOS2UNIX;
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

void call() {
	long i;
	while (fgets_cr(utterance->line, UTTLINELEN, fpin)) {
		if (!isUnixCRs) {
			for (i=0L; utterance->line[i] != EOS; i++) {
				if (utterance->line[i] == '\n')
					utterance->line[i] = 0x0A;
			}
		}
		fputs(utterance->line, fpout);
	}
}
