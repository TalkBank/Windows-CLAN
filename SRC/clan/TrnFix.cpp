/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3

#include "cu.h"
#if !defined(UNX)
#include "c_curses.h"

#define _main trnfix_main
#define call trnfix_call
#define getflag trnfix_getflag
#define init trnfix_init
#define usage trnfix_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

extern struct tier *headtier;
extern struct tier *defheadtier;
extern char OverWriteFile;

static char *trn, *mor, isDis;
static char tier[2][12];

void usage() {
	puts("trnfix compairs %trn tier to %mor tier.");
	printf("Usage: trnfix [%s] filename(s)\n", mainflgs());
	puts("+a : disambiguate words, then compare, (default: compare whole words)");
	puts("+bS: Specify tiers you want to compare, (default: %mor and %trn)");
	mainusage(FALSE);
	puts("\nExample: trnfix +b%gra +b%grt filename");
	cutt_exit(0);
}

void init(char first) {
	if (first) {
		tier[0][0] = EOS;
		tier[1][0] = EOS;
		isDis = FALSE;
		OverWriteFile = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		if (tier[0][0] != EOS && tier[1][0] == EOS) {
			fprintf(stderr,"Please specify the second tier to compare with +b option.\n");
			cutt_exit(0);
		}
		if (tier[0][0] == EOS && tier[1][0] == EOS) {
			strcpy(tier[0], "%mor:");
			strcpy(tier[1], "%trn:");
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	int len;

	f++;
	switch(*f++) {
		case 'a':
			isDis = TRUE;
			no_arg_option(f);
			break;
		case 'b':
			if (!*f) {
				fprintf(stderr,"Dependent tier name expected after %s option.\n", f-2);
				cutt_exit(0);
			}
			len = strlen(f) - 1;
			if (tier[0][0] == EOS) {
				strncpy(tier[0], f, 10);
				if (tier[0][len] != ':')
					strcat(tier[0], ":");
			} else if (tier[1][0] == EOS) {
				strncpy(tier[1], f, 10);
				if (tier[1][len] != ':')
					strcat(tier[1], ":");
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char my_equal(char trnC, char morC) {
	if ((isSpace(trnC) || trnC == '\n') && (isSpace(morC) || morC == '\n'))
		return(TRUE);
	if (trnC == morC)
		return(TRUE);
	return(FALSE);
}

static void compairTiers(long ln, char isMorFirst) {
	char isErrorFound, isBegTErrorMarker, isBegMErrorMarker;
	long ti, mi, ts, ms;

	ti = 0L;
	mi = 0L;
	ts = 0L;
	ms = 0L;
	isBegTErrorMarker = FALSE;
	isBegMErrorMarker = FALSE;
	isErrorFound = FALSE;
	while (trn[ti] != EOS && mor[mi] != EOS) {
		if (!my_equal(trn[ti], mor[mi])) {
			if (isDis) {
				if (mor[mi] == '^' && (isSpace(trn[ti]) || trn[ti] == '\n' || trn[ti] == EOS)) {
					while (!isSpace(mor[mi]) && mor[mi] != '\n' && mor[mi] != EOS)
						mi++;
				} else {
					while (!isSpace(mor[mi]) && mor[mi] != '\n'  && mor[mi] != '^' && mor[mi] != EOS)
						mi++;
					if (mor[mi] == '^') {
						mi++; 
						while (!isSpace(trn[ti]) && trn[ti] != '\n' && ti != 0)
							ti--;
						if (isSpace(trn[ti]) || trn[ti] == '\n')
							ti++;
					} else {
						while (!isSpace(trn[ti]) && trn[ti] != '\n' && trn[ti] != EOS)
							ti++;
						isErrorFound = TRUE;
					}
				}
			} else {
				while (!isSpace(trn[ti]) && trn[ti] != '\n' && trn[ti] != EOS)
					ti++;
				while (!isSpace(mor[mi]) && mor[mi] != '\n' && mor[mi] != EOS)
					mi++;
				isErrorFound = TRUE;
			}
#ifndef UNX
			if (isErrorFound) {
				if (!isBegTErrorMarker) {
					uS.shiftright(trn+ts, 2);
					trn[ts++] = ATTMARKER;
					trn[ts] = error_start;
					ti += 2;
					isBegTErrorMarker = TRUE;
				}
				if (!isBegMErrorMarker) {
					uS.shiftright(mor+ms, 2);
					mor[ms++] = ATTMARKER;
					mor[ms] = error_start;
					mi += 2;
					isBegMErrorMarker = TRUE;
				}
			}
#endif
		} else {
			if (isSpace(trn[ti]) || trn[ti] == '\n') {
#ifndef UNX
				if (isBegTErrorMarker) {
					uS.shiftright(trn+ti, 2);
					trn[ti++] = ATTMARKER;
					trn[ti++] = error_end;
					isBegTErrorMarker = FALSE;
				}
#endif
				while (isSpace(trn[ti]) || trn[ti] == '\n')
					ti++;
				ts = ti;
			} else {
				if (trn[ti] != EOS)
					ti++;
			}
			if (isSpace(mor[mi]) || mor[mi] == '\n') {
#ifndef UNX
				if (isBegMErrorMarker) {
					uS.shiftright(mor+mi, 2);
					mor[mi++] = ATTMARKER;
					mor[mi++] = error_end;
					isBegMErrorMarker = FALSE;
				}
#endif
				while (isSpace(mor[mi]) || mor[mi] == '\n')
					mi++;
				ms = mi;
			} else {
				if (mor[mi] != EOS)
					mi++;
			}
		}
	}
	while (isSpace(trn[ti]) || trn[ti] == '\n')
		ti++;
	while (isSpace(mor[mi]) || mor[mi] == '\n')
		mi++;
	if (trn[ti] != EOS || mor[mi] != EOS)
		isErrorFound = TRUE;
	if (isErrorFound) {
#ifndef UNX
		if (isBegTErrorMarker) {
			uS.shiftright(trn+ti, 2);
			trn[ti++] = ATTMARKER;
			trn[ti] = error_end;
		}
		if (isBegMErrorMarker) {
			uS.shiftright(mor+mi, 2);
			mor[mi++] = ATTMARKER;
			mor[mi] = error_end;
		}
#endif
		fprintf(fpout,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
		if (isMorFirst) {
			fprintf(fpout,"  %s %s", tier[0], mor);
			fprintf(fpout,"  %s %s", tier[1], trn);
		} else {
			fprintf(fpout,"  %s %s", tier[1], trn);
			fprintf(fpout,"  %s %s", tier[0], mor);
		}
	}
}

void call(void) {
	char RightTier, isMorFirst;
	long ln;

	ln = lineno;
	RightTier = FALSE;
	trn[0] = mor[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			isMorFirst = -1;
			RightTier = checktier(utterance->speaker);
		}
		if (RightTier) {
			if (utterance->speaker[0] == '*' || uS.partcmp(utterance->speaker, "@End", FALSE, FALSE)) {
				if (trn[0] != EOS && mor[0] != EOS) {
					compairTiers(ln, isMorFirst);
				} else if (trn[0] == EOS && mor[0] != EOS) {
					fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, ln);
					fprintf(fpout,"  Missing \"%s\" tier, while tier \"%s\" is present\n", tier[1], tier[0]);
				}
				trn[0] = mor[0] = EOS;
				ln = lineno;
			} else if (uS.partcmp(utterance->speaker,tier[0], FALSE, TRUE)) {
				if (isMorFirst == -1)
					isMorFirst = 1;
				strcpy(mor, utterance->line);
			} else if (uS.partcmp(utterance->speaker,tier[1], FALSE, TRUE)) {
				if (isMorFirst == -1)
					isMorFirst = 0;
				strcpy(trn, utterance->line);
			}
		}
	}
	if (RightTier) {
		if (trn[0] != EOS && mor[0] != EOS) {
			compairTiers(ln, isMorFirst);
		} else if (trn[0] == EOS && mor[0] != EOS) {
			fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(fpout,"  Missing \"%s\" tier, while tier \"%s\" is present\n", tier[1], tier[0]);
		}
		trn[0] = mor[0] = EOS;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = TRNFIX;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	trn = (char *)malloc(UTTLINELEN+2);
	if (trn == NULL) {
		fprintf(stderr, "ERROR: Out of memory\n");
		cutt_exit(0);
	}
	mor = (char *)malloc(UTTLINELEN+2);
	if (mor == NULL) {
		free(trn);
		fprintf(stderr, "ERROR: Out of memory\n");
		cutt_exit(0);
	}
	bmain(argc,argv,NULL);
	free(trn);
	free(mor);
}
