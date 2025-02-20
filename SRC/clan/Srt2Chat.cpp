/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "check.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main srt2chat_main
#define call srt2chat_call
#define getflag srt2chat_getflag
#define init srt2chat_init
#define usage srt2chat_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

extern struct tier *defheadtier;
extern char OverWriteFile;

static char isUseDepTier;


void init(char f) {
	if (f) {
		isUseDepTier = FALSE;
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		AddCEXExtension = "";
	}
}

void usage() {
	printf("Convert SRT notation to CHAT specific notation\n");
	printf("Usage: srt2chat [%s] filename(s)\n", mainflgs());
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SRT2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 't':
			if (*f == '%') {
				isUseDepTier = TRUE;
			}
			maingetflag(f-2,f1,i);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static int calculateOffset(char *s) {
	char *col, *num, count;
	int time;

	col = strrchr(s,',');
	if (col != NULL)
		*col = ':';
	count = 0;
	time = 0;
	do {
		col = strrchr(s,':');
		if (col != NULL) {
			*col = EOS;
			num = col + 1;
		} else
			num = s;

		if (count == 0) {
			time = atoi(num);
		} else if (count == 1) {
			time = time + (atoi(num) * 1000);
		} else if (count == 2) {
			time = time + (atoi(num) * 60 * 1000);
		} else if (count == 3) {
			time = time + (atoi(num) * 3600 * 1000);
		}
		count++;
	} while (col != NULL && count < 4) ;
	return(time);
}

void call() {
	int  i;
	char t, *BegStr, *EndStr, *isDelimFound;
	int Beg, End;

	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	eng\n");
	fprintf(fpout, "@Participants:\tTXT Text\n");
	fprintf(fpout, "@ID:	eng|text|TXT|||||Text|||\n");
	BegStr = strrchr(oldfname, '.');
	if (BegStr != NULL) {
		t = *BegStr;
		*BegStr = EOS;
	}
	fprintf(fpout, "@Media:	%s, audio\n", oldfname);
	if (BegStr != NULL) {
		*BegStr = t;
	}
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (isdigit(utterance->line[0]) && strchr(utterance->line, ':') == NULL) {
			Beg = 0;
			End = 0;
			continue;
		}
		uS.remblanks(utterance->line);
		if (utterance->line[0] == EOS) {
			Beg = 0;
			End = 0;
			continue;
		}
		if (isdigit(utterance->line[0]) && strchr(utterance->line, ':') != NULL) {
			BegStr = utterance->line;
			EndStr = NULL;
			for (i=0; utterance->line[i] != EOS; i++) {
				if (utterance->line[i] == '-' && utterance->line[i+1] == '-' && utterance->line[i+2] == '>') {
					utterance->line[i] = EOS;
					i += 3;
					for (; isSpace(utterance->line[i]); i++) ;
					EndStr = utterance->line+i;
					break;
				}
			}
			if (EndStr != NULL) {
				uS.remblanks(BegStr);
				uS.remblanks(EndStr);
				Beg = calculateOffset(BegStr);
				End = calculateOffset(EndStr);
			}
			continue;
		}
		isDelimFound = strchr(utterance->line, '.');
		if (isDelimFound == NULL) {
			isDelimFound = strchr(utterance->line, '?');
			if (isDelimFound == NULL) {
				isDelimFound = strchr(utterance->line, '!');
			}
		}
		if (Beg != 0 && End != 0) {
			if (isDelimFound == NULL)
				fprintf(fpout, "*TXT:\t%s . %c%d_%d%c\n", utterance->line, HIDEN_C, Beg, End, HIDEN_C);
			else
				fprintf(fpout, "*TXT:\t%s %c%d_%d%c\n", utterance->line, HIDEN_C, Beg, End, HIDEN_C);
		} else {
			if (isDelimFound == NULL)
				fprintf(fpout, "*TXT:\t%s .\n", utterance->line);
			else
				fprintf(fpout, "*TXT:\t%s\n", utterance->line);
		}
	}
	fprintf(fpout, "@End\n");
}
