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
	int OVCindex, OVindex;

	OVindex = 0;
	for (i++; utterance->line[i] == '[' || utterance->line[i] == ' '; i++) {
		if (utterance->line[i] == '[')
			OVindex++;
	}
	for (; utterance->line[i] != EOS; i++) {
		if (utterance->line[i] == ']') {
			OVCindex = 0;
			*OVe = i;
			for (i++; utterance->line[i] == ']' || utterance->line[i] == ' '; i++) {
				if (utterance->line[i] == ']')
					OVCindex++;
			}
			if (OVindex == OVCindex) {
				return(OVindex);
			} else {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Closing overlap number of ']' is not the same as opening number of '['.\n");
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
	int i, j, OVindex, OVe, OVt[25];
	char OVs[25];

	for (i=0; i < 25; i++) {
		OVs[i] = 0;
		OVt[i] = 0;
	}

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			for (i=0; utterance->line[i] != EOS; i++) {
				if (utterance->line[i] == '[') {
					OVindex = getOVindex(i, &OVe);
					if (OVindex == -1) {
						fprintf(stderr, "SKIPPING FILE: %s.\n\n", oldfname);
						return;
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
							att_shiftright(utterance->line+i, utterance->attLine+i, 2);
							utterance->attLine[i] = utterance->attLine[i+2];
							utterance->attLine[i+1] = utterance->attLine[i+2];
							utterance->line[i++] = 0xe2;
							utterance->line[i++] = 0x8c;
							utterance->line[i++] = 0x8a;
							i = OVe + 2;
							for (j=i; utterance->line[j] == ']' || utterance->line[j] == ' '; j++)
								utterance->line[j] = ' ';
							att_shiftright(utterance->line+i, utterance->attLine+i, 2);
							utterance->attLine[i] = utterance->attLine[i+2];
							utterance->attLine[i+1] = utterance->attLine[i+2];
							utterance->line[i++] = 0xe2;
							utterance->line[i++] = 0x8c;
							utterance->line[i++] = 0x8b;
						} else if (OVs[OVindex] == 0) {
							OVs[OVindex] = 2;
							OVt[OVindex] = 0;
							for (j=i; utterance->line[j] == '[' || utterance->line[j] == ' '; j++)
								utterance->line[j] = ' ';
							att_shiftright(utterance->line+i, utterance->attLine+i, 2);
							utterance->attLine[i] = utterance->attLine[i+2];
							utterance->attLine[i+1] = utterance->attLine[i+2];
							utterance->line[i++] = 0xe2;
							utterance->line[i++] = 0x8c;
							utterance->line[i++] = 0x88;
							i = OVe + 2;
							for (j=i; utterance->line[j] == ']' || utterance->line[j] == ' '; j++)
								utterance->line[j] = ' ';
							att_shiftright(utterance->line+i, utterance->attLine+i, 2);
							utterance->attLine[i] = utterance->attLine[i+2];
							utterance->attLine[i+1] = utterance->attLine[i+2];
							utterance->line[i++] = 0xe2;
							utterance->line[i++] = 0x8c;
							utterance->line[i++] = 0x89;
						}
					}
				}
			}
		}

		for (i=0; i < 25; i++) {
			if (OVs[i] == 1) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Previous overlap with %d '[' goes over this tier\n", i+1);
				fprintf(stderr, "%s\n", utterance->line);
			}
		}

		for (i=0; i < 25; i++) {
			if (OVs[i] == 1) {
				OVt[i] = OVt[i] + 1;
				if (OVt[i] == 2) {
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "Previous overlap with %d '[' goes over %d tiers\n", i+1, OVt[i]);
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
