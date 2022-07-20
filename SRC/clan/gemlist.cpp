/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main gemlist_main
#define call gemlist_call
#define getflag gemlist_getflag
#define init gemlist_init
#define usage gemlist_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"


static char OnlyBGEGData;
static char gemlist_BBS[6], gemlist_CBS[6], gemlist_BG[6], gemlist_ftime;

void usage() {
	puts("GEMLIST outputs gems profile");
	printf("Usage: gemlist [wS %s] filename(s)\n", mainflgs());
	puts("-wS: exclude tiers with postcodes S from a given input file");
	mainusage(TRUE);
}

void init(char first) {
	if (first) {
		strcpy(gemlist_BBS, "@BG:");
		strcpy(gemlist_CBS, "@EG:");
		strcpy(gemlist_BG,  "@G:");
		gemlist_ftime = TRUE;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		nomain = 1;
		OnlyBGEGData = FALSE;
	} else if (gemlist_ftime) {
		gemlist_ftime = FALSE;
		maketierchoice(gemlist_BBS,'+','\001');
		maketierchoice(gemlist_CBS,'+','\001');
		maketierchoice(gemlist_BG, '+','\001');
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'w':
				if (*f) {
					if (*(f-2) == '+') {
/*
						if (*f == '[' && *(f+1) == '+') {
							*f = '<';
							f[strlen(f)-1] = '>';
							addword('o', 'i',getfarg(f,f1,i));
						} else {
*/
							fprintf(stderr,"Invalid option: %s\n", f-2);
							cutt_exit(0);
//						}
					} else {
						if (*f == '[' && *(f+1) == '+') {
							*f = '<';
							f[strlen(f)-1] = '>';
							addword('o','e',getfarg(f,f1,i));
						} else {
							fprintf(stderr,"Invalid argument for option: %s\n", f-2);
							cutt_exit(0);
						}
					}
				} else {
					fprintf(stderr,"Invalid argument for option: %s\n", f-2);
					cutt_exit(0);
				}
				break;
		case 'd':
			OnlyBGEGData = TRUE;
			no_arg_option(f);
			break;
		case 't':
			if (*(f-2) == '+' && *f != '%' && *f != '@') nomain = 0;
			maingetflag(f-2,f1,i);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

void call() {
    unsigned int cnt = 0, cnt2 = 0;
	int  i;
    char bgeg_found = FALSE, bg_found = FALSE, bgeg_first = FALSE, bg_first = FALSE;
	char rightspeaker;

	currentatt = 0;
    currentchar = (char)getc_cr(fpin, &currentatt);
    rightspeaker = FALSE;
    while (getwholeutter()) {
/*
printf("found=%d; sp=%s; uttline=%s", found, utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/

		if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*')
				rightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*')
				rightspeaker = TRUE;
			if (!rightspeaker && *utterance->speaker != '@')
				continue;
		}

		if (uS.partcmp(templineC,gemlist_BG,FALSE,FALSE)) {
			if (cnt2 && (bg_found || !OnlyBGEGData))
				fprintf(fpout, "\t%u main speaker tier(s).\n", cnt2);
			cnt2 = 0;
			bg_found = TRUE;
			bg_first = TRUE;
			fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(fpout, "%s%s", utterance->speaker, utterance->line);
		} else if (uS.partcmp(templineC,gemlist_BBS,FALSE,FALSE)) {
			if (cnt && (bgeg_found || !OnlyBGEGData))
				fprintf(fpout, "\t%u main speaker tier(s).\n", cnt);
			cnt = 0;
			bgeg_found = TRUE;
			bgeg_first = TRUE;
			fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(fpout, "%s%s", utterance->speaker, utterance->line);
		} else if (uS.partcmp(templineC,gemlist_CBS,FALSE,FALSE)) {
			if (cnt && (bgeg_found || !OnlyBGEGData))
				fprintf(fpout, "\t%u main speaker tier(s).\n", cnt);
			cnt = 0;
			bgeg_found = FALSE;
			fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(fpout, "%s%s", utterance->speaker, utterance->line);
		} else {
			if ((i=isPostCodeFound(utterance->speaker, utterance->line)) == 5 || i == 1) {
				if (*utterance->speaker == '*')
					rightspeaker = FALSE;
				else
					continue;
			}
		    if (rightspeaker && *utterance->speaker == '*') {
		    	cnt++;
		    	cnt2++;
		    }
		    if (nomain && *utterance->speaker == '*') ;
		    else if (rightspeaker && (bgeg_found || bg_found || !OnlyBGEGData)) {
   				fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(fpout, "%s%s", utterance->speaker, utterance->line);
			}
		}
    }
    if (bgeg_first && cnt && (bgeg_found || !OnlyBGEGData)) 
		fprintf(fpout, "\t%u main speaker tier(s).\n", cnt);
    if (bg_first && cnt2 && (bg_found || !OnlyBGEGData))
		fprintf(fpout, "\t%u main speaker tier(s).\n", cnt2);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    CLAN_PROG_NUM = GEMLIST;
    chatmode = CHAT_MODE;
    OnlydataLimit = 1;
    UttlineEqUtterance = FALSE;
    bmain(argc,argv,NULL);
}
