/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main subtitles_main
#define call subtitles_call
#define getflag subtitles_getflag
#define init subtitles_init
#define usage subtitles_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

static const char *MFN;
static long ln;
static double timeOffset;

extern struct tier *defheadtier;
extern char OverWriteFile;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		MFN = "change_me";
		timeOffset = 0.0000;
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	}
}

void usage() {
	printf("Converts subtitles text files to CHAT text files\n");
	printf("Usage: subtitles [mF tN %s] filename(s)\n", mainflgs());
    puts("+mF: specify movie file name F (default: dummy).");
    puts("+tN: specify movie segment start time offset relative to actual movie start.");
	mainusage(FALSE);
	puts("Example: subtitles +t3.2 *.str");
	puts("         subtitles +t-3 *.txt");
	puts("         subtitles +t12:06 *.txt");
	cutt_exit(0);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SUBTITLES;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}

static double calculateOffset(char *s) {
	int  count;
	char *col, *num, isNeg;
	double time;

	isNeg = FALSE;
	count = 0;
	time = 0.0;
	if (*s == '-') {
		isNeg = TRUE;
		s++;
	}
	col = strrchr(s,',');
	if (col != NULL)
		*col = ':';
	else {
		col = strrchr(s,'.');
		if (col != NULL)
			*col = ':';
		else
			count = 1;
	}
	do {
		col = strrchr(s,':');
		if (col != NULL) {
			*col = EOS;
			num = col + 1;
		} else 
			num = s;
		if (count == 0) {
			time = (double)atof(num);
		} else if (count == 1) {
			time = time + ((double)atof(num) * 1000.0);
		} else if (count == 2) {
			time = time + ((double)atof(num) * 60.0 * 1000.0);
		} else if (count == 3) {
			time = time + ((double)atof(num) * 3600.0 * 1000.0);
		}
		count++;
	} while (col != NULL && count < 4) ;
	if (isNeg)
		time *= -1;
	return(time);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'm':
			MFN = f;
			break;
		case 's':
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
		case 't':
			if (*f) {
				strcpy(templineC1, getfarg(f,f1,i));
				timeOffset = calculateOffset(templineC1);
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		default:
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
	}
}

static void cleanupLine(char *st) {
	int i;

	for (i=0; st[i] != EOS;) {
		if ((st[i] >= 0 && st[i] < 32) || st[i] == 0x7f)
			strcpy(st+i, st+i+1);
		else
			i++;
	}
}

static char isTimeValue(char *line) {
	int val;

	val = 0;
	for (; *line != EOS; line++) {
		if (*line == ':' || *line == ',')
			val++;
		if (isalpha(*line))
			return(FALSE);
	}
	if (val == 6)
		return(TRUE);
	else
		return(FALSE);
}

static char isNumberLine(char *line) {
	for (; *line != EOS; line++) {
		if (!isdigit(*line))
			return(FALSE);
	}
	return(TRUE);
}

void call() {
	char *btS, *etS;
	long bt, et;

    fprintf(fpout, "@UTF8\n");
    fprintf(fpout, "@Begin\n");
    fprintf(fpout, "@Languages:	eng\n");
	fprintf(fpout, "@Participants:	PAR Participant\n");
	fprintf(fpout, "@ID:	eng|change_me_later|PAR|||||Participant|||\n");
    fprintf(fpout, "@Media:	%s, video\n", MFN);
	bt = 0L;
	et = 0L;
	templineC1[0] = EOS;
    while (fgets_cr(templineC, UTTLINELEN, fpin)) {
    	ln++;
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE))
			continue;
		uS.remFrontAndBackBlanks(templineC);
		cleanupLine(templineC);
		if (isTimeValue(templineC)) {
			if (templineC1[0] != EOS)
				fprintf(fpout, "*PAR:\t%s %c%ld_%ld%c\n", templineC1, HIDEN_C, bt, et, HIDEN_C);
			templineC1[0] = EOS;
			for (btS=templineC; !isdigit(*btS); btS++) ;
			if (*btS == EOS) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "File is corrupted\n");
				cutt_exit(0);
			}
			for (etS=btS; (isdigit(*etS) || *etS == ':' || *etS == ',') && *etS != EOS; etS++) ;
			if (*etS == EOS) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "File is corrupted\n");
				cutt_exit(0);
			}
			*etS = EOS;
			for (etS++; !isdigit(*etS) && *etS != EOS; etS++) ;
			if (*etS == EOS) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "File is corrupted\n");
				cutt_exit(0);
			}
			bt = (long)(calculateOffset(btS) - timeOffset);
			et = (long)(calculateOffset(etS) - timeOffset);
		} else if (isNumberLine(templineC)) {
			continue;
		} else if (templineC[0] != EOS) {
			if (templineC1[0] != EOS)
				strcat(templineC1, " ");
			strcat(templineC1, templineC);
		}
    }
	if (templineC1[0] != EOS)
		fprintf(fpout, "*PAR:\t%s %c%ld_%ld%c\n", templineC1, HIDEN_C, bt, et, HIDEN_C);
	fprintf(fpout, "@End\n");
}
