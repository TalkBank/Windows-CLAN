/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3

#include "cu.h"

#if !defined(UNX)
#define _main lines_main
#define call lines_call
#define getflag lines_getflag
#define init lines_init
#define usage lines_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *headtier;

char lines_numbers;

void usage() {
	puts("LINES line number");
	printf("Usage: lines [n %s] filename(s)\n",mainflgs());
	puts("+n : remove all the line/tier numbers");
	mainusage(TRUE);
}

void init(char s) {
	if (s) {
		lines_numbers = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		onlydata = 5;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = LINES_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'n':
			lines_numbers = FALSE;
			chatmode = 4;
			no_arg_option(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
/* 2009-10-13
static void att_cp_one(long pos, char *desSt, char *srcSt, char *desAtt, char srcAtt) {
	long i;

	for (i=0; srcSt[i]; i++, pos++) {
		desSt[pos] = srcSt[i];
		desAtt[pos] = srcAtt;
	}
	desSt[pos] = EOS;
}
*/
void call() {		/* this function is self-explanatory */
	register int  i;
	register int  j;
	register long tl;
	char rightTier;
	char *pos, buf[SPEAKERLEN];
	AttTYPE *posAtt, bufAtt[SPEAKERLEN];

	if (!lines_numbers) {
		while (fgets_cr(uttline, UTTLINELEN, fpin)) {
			if (*uttline != '@')
				strcpy(uttline, uttline+6);
			fputs(uttline, fpout);
		}
	} else {
		tl = 0L;
		currentatt = 0;
		currentchar = (char)getc_cr(fpin, &currentatt);
		while (getwholeutter()) {
			if (uS.isInvisibleHeader(utterance->speaker) || uS.isUTF8(utterance->speaker)) {
				printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
				continue;
			}
			if (!chatmode)
				utterance->speaker[0] = EOS;
			if (!chatmode || headtier == NULL)
				rightTier = TRUE;
			else if (checktier(utterance->speaker) && (!nomain || *utterance->speaker != '*'))
				rightTier = '\002';
			else 
				rightTier = FALSE;

			if (rightTier)
				sprintf(buf,"%-5ld ", ++tl);
			else
				sprintf(buf,"      ");
			for (j=0; buf[j]; j++)
				bufAtt[j] = 0;
			i = 0;
			if (chatmode) {
				att_cp(strlen(buf), buf, utterance->speaker, bufAtt, utterance->attSp);
			}
			pos = utterance->line;
			posAtt = utterance->attLine;
			i = 0;
			while (utterance->line[i] != EOS) {
				if (utterance->line[i] == '\n') {
					utterance->line[i++] = EOS;
					printout(buf, pos, bufAtt, posAtt, FALSE);
					pos = utterance->line + i;
					posAtt = utterance->attLine + i;
					if (rightTier == TRUE)
						sprintf(buf,"%-5ld ", ++tl);
					else
						sprintf(buf,"      ");
					for (j=0; buf[j]; j++)
						bufAtt[j] = 0;
				} else
					i++;
			}
			if (i > 0 && utterance->line[i-1] != EOS) {
				while (isSpace(*pos)) {
					pos++;
					posAtt++;
				}
				printout(buf, pos, bufAtt, posAtt, FALSE);
			} else if (rightTier == TRUE)
				tl--;
		}
	}
}
