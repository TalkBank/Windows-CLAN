/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "ced.h"
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

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 1

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char isRecurive;

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
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
	printf("Converts SLX text files to CHAT text files\n");
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	cutt_exit(0);
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
		case 'r':
			maingetflag(f-2,f1,i);
			break;
		default:
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
	}
}

void call() {
	char *s, comm[BUFSIZ+2];
	int  len;
	FILE *fp;
	FNType oldname[FILENAME_MAX+2];
	FNType cann[FILENAME_MAX+2];

	comm[0] = EOS;
	strcpy(FileName1, oldfname);
	s = strrchr(FileName1, '/');
	if (s != NULL) {
		*s = EOS;
		strcpy(oldname, s+1);
		s = strrchr(FileName1, '/');
		if (s != NULL) {
			*s = EOS;
			strcpy(cann, s+1);
			s = strrchr(FileName1, '/');
			if (s != NULL) {
				*s = EOS;
				strcpy(FileName2, FileName1);
				strcat(FileName2, "/CANDOR-16/");
				strcat(FileName2, cann);
				strcat(FileName2, "/");
				len = strlen(FileName2);
				strcat(FileName2, cann);
				FileName2[len+1] = EOS;
				strcat(FileName2, "-cha/");
				s = strrchr(oldname, '.');
				if (s != NULL) {
					*s = EOS;
					strcat(FileName2, oldname);
					strcat(FileName2, "-trans.cha");
					fp = fopen(FileName2, "r");
					if (fp != NULL) {
						while (fgets_cr(templineC1, UTTLINELEN, fp)) {
							if (uS.partcmp(templineC1, "@Comment:", FALSE, FALSE)) {
								strcpy(comm, templineC1);
								fgets_cr(templineC1, UTTLINELEN, fp);
								strcat(comm, templineC1);
								break;
							}
						}
						fclose(fp);
					} else {
						fprintf(stderr, "\nCan't open file: %s\n\n", FileName2);
					}
				}
			}
		}
	}
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*' && comm[0] != EOS) {
			fprintf(fpout, "%s", comm);
			comm[0] = EOS;
		}
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
}
