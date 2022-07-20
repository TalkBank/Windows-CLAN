/**********************************************************************
 "Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

#define CHAT_MODE 3
#include "cu.h"

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;

static char *lang;

void usage() {
	printf("Usage: temp [cS %s] filename(s)\n",mainflgs());
	puts("+cS: Specify language code for [- ] pre-code");
	mainusage(FALSE);
	puts("\nExample: temp +ceng *.cha");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		lang = NULL;
		stout = FALSE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (lang == NULL) {
			fprintf(stderr, "Please specify langauge code with option +cS.\n");
			cutt_exit(0);
		}
	}
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
		case 'c':
			if (*f == EOS) {
				fprintf(stderr, "Please specify langauge code with option +cS.\n");
				cutt_exit(0);
			}
			lang = getfarg(f,f1,i);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char isReplaceAts(char *line) {
	int  cnt, i, j;
	char fc, isAtsFound;

	fc = 0;
	cnt = 0;
	isAtsFound = 0;
	for (i=0; line[i] != EOS; i++) {
		if (uS.isRightChar(line, i, '[', &dFnt, MBF)) {
			for (j=i+1; line[j] != EOS && !uS.isRightChar(line, j, ']', &dFnt, MBF); j++) ;
			if (line[j] != EOS)
				i = j;
		} else if (line[i] == HIDEN_C) {
			for (j=i+1; line[j] != EOS && line[j] != HIDEN_C; j++) ;
			if (line[j] != EOS)
				i = j;
		} else if (line[i] == '@' && line[i+1] == 's' && (uS.isskip(line, i+2, &dFnt, C_MBF) || line[i+2] == '\n' || line[i+2] == EOS)) {
			isAtsFound = 1;
		} else if (fc != '0' && fc != '&' && fc != '+' && fc != '-' && fc != '#' && !uS.isskip(line, i, &dFnt, C_MBF) && line[i] != '\n' && isAtsFound != 1) {
			isAtsFound = -1;
		} else if (uS.isskip(line, i, &dFnt, C_MBF) || line[i] == '\n') {
			fc = line[i+1];
			if (isAtsFound == 1)
				cnt++;
			if (isAtsFound == -1)
				cnt--;
			isAtsFound = 0;
		}
	}
	if (cnt > 0)
		return(TRUE);
	else
		return(FALSE);
}

static void replaceAts(char *line, AttTYPE *atts) {
	int  i, j, len;

	len = strlen(lang) + 5;
	att_shiftright(line, atts, len);
	line[0] = '[';
	line[1] = '-';
	line[2] = ' ';
	for (i=3, j=0; lang[j] != EOS && i < len; i++, j++) {
		line[i] = lang[j];
	}
	line[i++] = ']';
	line[i++] = ' ';
	for (i=0; i < len; i++)
		atts[i] = 0;
	for (i=0; line[i] != EOS; i++) {
		if (uS.isRightChar(line, i, '[', &dFnt, MBF)) {
			for (j=i+1; line[j] != EOS && !uS.isRightChar(line, j, ']', &dFnt, MBF); j++) ;
			if (line[j] != EOS)
				i = j;
		} else if (line[i] == HIDEN_C) {
			for (j=i+1; line[j] != EOS && line[j] != HIDEN_C; j++) ;
			if (line[j] != EOS)
				i = j;
		} else if (line[i] == '@' && line[i+1] == 's' && (uS.isskip(line, i+2, &dFnt, C_MBF) || line[i+2] == '\n' || line[i+2] == EOS)) {
			att_cp(0L, line+i, line+i+2, atts+i, atts+i+2);
			i--;
		}
	}
}

void call() {
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			if (isReplaceAts(utterance->line)) {
				replaceAts(utterance->line, utterance->attLine);
			}
		}
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
}
