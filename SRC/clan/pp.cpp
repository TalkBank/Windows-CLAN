
/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main pp_main
#define call pp_call
#define getflag pp_getflag
#define init pp_init
#define usage pp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

extern char OverWriteFile;

void init(char f) {
	char *s;

	if (f) {
		OverWriteFile = TRUE;
		AddCEXExtension = ".xml";
		onlydata = 1;
		stout = FALSE;
	} else {
		if ((s=strrchr(oldfname, '.')) != NULL) {
			AddCEXExtension = s;
		}
	}
}

void usage() {
	printf("Usage: pp filename(s)\n",mainflgs());
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = PP;
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

static void changeGroupIndent(char *groupIndent, int d) {
	int i;

	if (d > 0)
		strcat(groupIndent, "    ");
	else {
		i = strlen(groupIndent) - 4;
		if (i < 0)
			i = 0;
		groupIndent[i] = EOS;
	}
}

void call() {
	int c, lastC;
	char groupIndent[1024];
	char isCR, ftime;

	ftime = true;
	lastC = 0;
	isCR = true;
	groupIndent[0] = EOS;
	while (!feof(fpin)) {
		c = getc(fpin);
		if (feof(fpin)) {
			if (!isCR)
				putc('\n', fpout);
			break;
		}
		if (c == '<') {
			if (!ftime)
				putc('\n', fpout);
			lastC = c;
			if (!feof(fpin)) {
				c = getc(fpin);
				if (c != '/') {
					fputs(groupIndent, fpout);
					changeGroupIndent(groupIndent, 1);
				} else {
					changeGroupIndent(groupIndent, -1);
					fputs(groupIndent, fpout);
				}
			} else
				c = '\n';
			putc(lastC, fpout);
			if (c != '\n')
				putc(c, fpout);
			isCR = true;
		} else if (c == '>') {
			putc(c, fpout);
			if (lastC == '/' || lastC == '?')
				changeGroupIndent(groupIndent, -1);
			isCR = false;
		} else if (c != '\n' && c != 0x0d && c != 0x0a) {
			if (!isspace(c)) {
				if (!isCR) {
					putc('\n', fpout);
					fputs(groupIndent, fpout);
				}
				isCR = true;
			}
			putc(c, fpout);
		}
		lastC = c;
		ftime = false;
	}
}
