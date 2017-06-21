/**********************************************************************
	"Copyright 2006 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

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

static int openOv[20], indexOV;

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
	indexOV = 0;
}

void usage() {
	printf("indents CA overlaps\n");
	printf("Usage: indent [%s] filename(s)\n", mainflgs());
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

static void handlePrevOV(char *line, AttTYPE *attLine) {
	int i;
	int col, index;
	char isClosingFound, isOpeningFound;
	AttTYPE att;
	short res;

	isClosingFound = FALSE;
	isOpeningFound = FALSE;
	col = 0;
	index = 0;
	for (i=0; line[i] != EOS && index < indexOV; i++) {
		if (line[i] == '\t' && (i == 0 || line[i-1] != '\n'))
			line[i] = ' ';
		if (line[i] == '\n')
			col = 0;
		res = my_CharacterByteType(line, i, &dFnt);
		if (line[i] == (char)0xe2 && line[i+1] == (char)0x8c && line[i+2] == (char)0x88) 
			isOpeningFound = TRUE;
		if (line[i] == (char)0xe2 && line[i+1] == (char)0x8c && line[i+2] == (char)0x8a) {
			isClosingFound = TRUE;
			if (col < openOv[index]) {
				att = attLine[i];
				att_shiftright(line+i, attLine+i, openOv[index]-col);
				for (; col < openOv[index]; col++, i++) {
					line[i] = ' ';
					attLine[i] = att;
				}
			} else if (col > openOv[index]) {
				for (i--; col > openOv[index] && i > 0 && line[i] == ' ' && line[i-1] == ' '; col--, i--) {
					strcpy(line+i, line+i+1);
				}
				i++;
			}
			index++;
			i += 2;
			res = 0;
		}
		if (res == 0 || res == 1) {
			col++;
		}
	}
	for (i=0; index < indexOV; index++, i++)
		openOv[i] = openOv[index];
	indexOV = i;
	if (!isClosingFound) {
		fprintf(stderr,"*** File \"%s\": ", oldfname);
		fprintf(stderr,"line %ld.\n", lineno);
		fprintf(stderr, "Possibly missing closing overlap marker.\n");
		if (isOpeningFound)
			fprintf(stderr, "Or use of opening overlap marker instead of closing one.\n");
		indexOV--;
	}
}

static void creatNewOV(char *line) {
	int i;
	int col;
	short res;

	col = 0;
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '\t' && (i == 0 || line[i-1] != '\n'))
			line[i] = ' ';
		res = my_CharacterByteType(line, i, &dFnt);
		if (line[i] == (char)0xe2 && line[i+1] == (char)0x8c && line[i+2] == (char)0x88) {
			if (indexOV >= 19) {
				fprintf(stderr, "ERROR: Overlap array too small.\n");
				cutt_exit(0);
			}
			openOv[indexOV++] = col;
			i += 2;
			res = 0;
		}
		if (res == 0 || res == 1) {
			if (line[i] != '\t' || i == 0 || line[i-1] != '\n')
				col++;
		}
		if (line[i] == '\n')
			col = 0;
	}
}

void call() {
	char ftime = TRUE;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (ftime) {
			if (!uS.partcmp(utterance->speaker,"@Font:",FALSE, TRUE)) {
#ifdef _WIN32 
				fputs("@Font:	Win95:CAFont:-15:0\n", fpout);
#else
				fputs("@Font:	CAFont:13:7\n", fpout);
#endif
			}
			ftime = FALSE;
		}
		if (utterance->speaker[0] == '*') {
			int i;

			for (i=0; utterance->speaker[i] != ':' && utterance->speaker[i] != EOS; i++) ;
			if (utterance->speaker[i] == ':') {
				i++;
				if (isSpace(utterance->speaker[i])) {
					utterance->speaker[i] = '\t';
					utterance->speaker[i+1] = EOS;
				}
			}
			if (indexOV > 0) {
				handlePrevOV(utterance->line, utterance->attLine);
			}
			creatNewOV(utterance->line);
		}
		printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
	}
}
