/**********************************************************************
	"Copyright 1990-2017 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
 */

#include "ced.h"
#define CHAT_MODE 1
#include "cu.h"
#ifdef _WIN32
#include "stdafx.h"
#endif

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

static char isOvnFound;

void init(char f) {
	if (f) {
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		stout = FALSE;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		isOvnFound = TRUE; // FALSE;
	} else {
	}
}

void usage() {
	printf("Convert overlaps of form [...] or [[...]] or [[[...]]] to CA overlaps\n");
	printf("Usage: temp *.cha\n");
	mainusage(TRUE);
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

static int getOVindex(int i, int *OVe) {
	int j, OVCindex, OVindex, numO, numC;

	OVindex = 0;
	for (i++; utterance->line[i] == '[' || utterance->line[i] == ' '; i++) {
		if (utterance->line[i] == '[')
			OVindex++;
	}
	numO = 0;
	numC = 0;
	if (isdigit(utterance->line[i])) {
		numO = atoi(utterance->line+i) - 1;
		if (numO < 0)
			numO = 0;
		for (; isdigit(utterance->line[i]) || utterance->line[i] == ' '; i++) ;
		OVindex = OVindex + numO;
	}
	for (; utterance->line[i] != EOS; i++) {
		if (utterance->line[i] == ']') {
			numC = 0;
			if (i > 0 && isdigit(utterance->line[i-1])) {
				for (j=i-1; j >= 0 && isdigit(utterance->line[j]); j--) ;
				j++;
				numC = atoi(utterance->line+j) - 1;
				if (numC < 0)
					numC = 0;
			} else
				j = i;
			OVCindex = 0;
			*OVe = j;
			for (i++; utterance->line[i] == ']' || utterance->line[i] == ' '; i++) {
				if (utterance->line[i] == ']')
					OVCindex++;
			}
			if (numC > 0)
				OVCindex = OVCindex + numC;
			if (OVindex == OVCindex) {
				if (numO > 0 && numC > 0)
					isOvnFound = TRUE;
				return(OVindex);
			} else {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Closing overlap # of '%d]' is not the same as opening # of '[%d'.\n", OVCindex+1, OVindex+1);
				fprintf(stderr, "%s\n", utterance->line);
				return(-1);
			}
		}
	}
	fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
	fprintf(stderr, "Can't find closing overlap ']'.\n");
	fprintf(stderr, "%s\n", utterance->line);
	return(-1);
}

void call() {
	int i, j, ofN, OVindex, OVe, OVt[25], offset;
	char OVs[25], onSt[32], isNumFound;

	for (i=0; i < 25; i++) {
		OVs[i] = 0;
		OVt[i] = 0;
	}

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			i = 0;
			while (utterance->line[i] != EOS) {
				if (utterance->line[i] == '[') {
					OVindex = getOVindex(i, &OVe);
					if (OVindex == -1) {
//						fprintf(stderr, "SKIPPING FILE: %s.\n\n", oldfname);
						cutt_exit(0);
					} else {
						if (OVindex >= 25) {
							fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
							fprintf(stderr, "Internal error. Overlap buffer exceeded %d out of 25.\n", OVindex);
							fprintf(stderr, "%s\n", utterance->line);
							cutt_exit(0);
						}
						if (OVs[OVindex] == 1) {
							OVs[OVindex] = 0;
							OVt[OVindex] = 0;
							for (j=i; utterance->line[j] == '[' || utterance->line[j] == ' '; j++)
								utterance->line[j] = ' ';
							offset = 2;
							isNumFound = TRUE;
							if (OVindex > 0 && isOvnFound && !isdigit(utterance->line[j])) {
								offset++;
								if (OVindex+1 > 10)
									offset++;
								isNumFound = FALSE;
							}
							att_shiftright(utterance->line+i, utterance->attLine+i, offset);
							for (ofN=0; ofN < offset; ofN++) {
								utterance->attLine[i+ofN] = utterance->attLine[i+offset];
							}
							utterance->line[i++] = 0xe2;
							utterance->line[i++] = 0x8c;
							utterance->line[i++] = 0x8a;
							if (!isNumFound) {
								sprintf(onSt, "%d", OVindex+1);
								for (j=0; j < strlen(onSt); j++) {
									utterance->line[i++] = onSt[j];
								}
							}
							i = OVe + offset;
							offset = 3;
							isNumFound = TRUE;
							if (OVindex > 0 && isOvnFound && !isdigit(utterance->line[i])) {
								offset++;
								if (OVindex+1 > 10)
									offset++;
								isNumFound = FALSE;
							}
							for (j=i; isdigit(utterance->line[j]); j++) ;
							for (; utterance->line[j] == ']' || utterance->line[j] == ' '; j++)
								utterance->line[j] = ' ';
							att_shiftright(utterance->line+i, utterance->attLine+i, offset);
							for (ofN=0; ofN < offset; ofN++) {
								utterance->attLine[i+ofN] = utterance->attLine[i+offset];
							}
							utterance->line[i++] = 0xe2;
							utterance->line[i++] = 0x8c;
							utterance->line[i++] = 0x8b;
							if (!isNumFound) {
								sprintf(onSt, "%d", OVindex+1);
								for (j=0; j < strlen(onSt); j++) {
									utterance->line[i++] = onSt[j];
								}
							}
							if (utterance->line[i] != '[')
								i++;
						} else if (OVs[OVindex] == 0) {
							OVs[OVindex] = 2;
							OVt[OVindex] = 0;
							for (j=i; utterance->line[j] == '[' || utterance->line[j] == ' '; j++)
								utterance->line[j] = ' ';
							offset = 2;
							isNumFound = TRUE;
							if (OVindex > 0 && isOvnFound && !isdigit(utterance->line[j])) {
								offset++;
								if (OVindex+1 > 10)
									offset++;
								isNumFound = FALSE;
							}
							att_shiftright(utterance->line+i, utterance->attLine+i, offset);
							for (ofN=0; ofN < offset; ofN++) {
								utterance->attLine[i+ofN] = utterance->attLine[i+offset];
							}
							utterance->line[i++] = 0xe2;
							utterance->line[i++] = 0x8c;
							utterance->line[i++] = 0x88;
							if (!isNumFound) {
								sprintf(onSt, "%d", OVindex+1);
								for (j=0; j < strlen(onSt); j++) {
									utterance->line[i++] = onSt[j];
								}
							}
							i = OVe + offset;
							offset = 3;
							isNumFound = TRUE;
							if (OVindex > 0 && isOvnFound && !isdigit(utterance->line[i])) {
								offset++;
								if (OVindex+1 > 10)
									offset++;
								isNumFound = FALSE;
							}
							for (j=i; isdigit(utterance->line[j]); j++) ;
							for (; utterance->line[j] == ']' || utterance->line[j] == ' '; j++)
								utterance->line[j] = ' ';
							att_shiftright(utterance->line+i, utterance->attLine+i, offset);
							for (ofN=0; ofN < offset; ofN++) {
								utterance->attLine[i+ofN] = utterance->attLine[i+offset];
							}
							utterance->line[i++] = 0xe2;
							utterance->line[i++] = 0x8c;
							utterance->line[i++] = 0x89;
							if (!isNumFound) {
								sprintf(onSt, "%d", OVindex+1);
								for (j=0; j < strlen(onSt); j++) {
									utterance->line[i++] = onSt[j];
								}
							}
							if (utterance->line[i] != '[')
								i++;
						} else
							i++;
					}
				} else
					i++;
			}
		}

//		for (i=0; i < 25; i++) {
		i=0;
			if (OVs[i] == 1) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Previous overlap with '[%d' goes over this tier\n", i+1);
				fprintf(stderr, "%s\n", utterance->line);
			}
//		}

		for (i=0; i < 25; i++) {
			if (OVs[i] == 1) {
				OVt[i] = OVt[i] + 1;
				if (OVt[i] == 2) {
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "Previous overlap '[%d' goes over %d tiers\n", i+1, OVt[i]);
					fprintf(stderr, "%s\n", utterance->line);
					cutt_exit(0);
				}
			}
		}

		removeExtraSpace(utterance->line);
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		for (i=0; i < 25; i++)
			if (OVs[i] == 2)
				OVs[i] = 1;
	}
}
