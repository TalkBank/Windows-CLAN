/**********************************************************************
	"Copyright 2006 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 4

#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main columns_main
#define call columns_call
#define getflag columns_getflag
#define init columns_init
#define usage columns_usage
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

#define SCREENWIDTH 80

/* *********************** columns prototypes ************************* */
#ifdef STRICT_PROTO
#else
#endif
/* **************************************************************** */

extern long MAXOUTCOL;

int  col2, col3;
char AddLines;
char TwoColumn;
char TierHeaders;
char *ChildesName;

void usage() {
    puts("COLUMNS");
    printf("Usage: columns [bN cN h nS %s] filename(s)\n",mainflgs());
    puts("+bN: set second speaker's column to N");
    puts("+cN: set dependent tier's column to N");
    puts("+d : display dependent tiers in a separate column");
    puts("+h : do not include tier name in the output");
    puts("+lN: 0 - show all line numbers, 1 - only '*' tiers, 2 - only '*', '%' tiers");
    puts("+nS: set child's name to S");
    mainusage();
}

void init(char first) {
    if (first) {
	col2 = SCREENWIDTH / 2;
	col3 = col2 * 2;
	AddLines = FALSE;
	TwoColumn = TRUE;
	ChildesName = NULL;
	TierHeaders = TRUE;
	FilterTier = 0;
    }
}

void getflag(char *f, char *f1, int *i) {
    f++;
    switch(*f++) {
	case 'b':
		col2 = atoi(f); 
		if (col2 <= 0 || col2 >= 77) {
		    fputs("Column should be between 0 and 77", stderr);
		    cutt_exit(0);
		}
		break;
	case 'c':
		col3 = atoi(f); 
		if (col3 <= 0 || col3 >= 77) {
		    fputs("Column should be between 0 and 77", stderr);
		    cutt_exit(0);
		}
		break;
	case 'n':
		ChildesName = f;
		uS.uppercasestr(ChildesName, &dFnt, C_MBF);
		break;
	case 'd':
		TwoColumn = FALSE;
		col2 = SCREENWIDTH / 3;
		col3 = col2 * 2;
		no_arg_option(f);
		break;
	case 'h':
		TierHeaders = FALSE;
		no_arg_option(f);
		break;
	case 'l':
		AddLines = (char)(atoi(getfarg(f,f1,i))+1);
		if (AddLines > 3) {
		    fprintf(stderr, "The only +l levels allowed are 0-%d.\n", 2);
		    cutt_exit(0);
		}
		break;
        default:
		maingetflag(f-2,f1,i);
		break;
    }
}

void call() {
	register int i;
	char RightSp = TRUE;
	char temp2[SCREENWIDTH], temp3[SCREENWIDTH];
	char buf[SPEAKERLEN+UTTLINELEN];

	if (ChildesName == NULL) {
		fputs("Please specify child's name with \"+n\" option.", stderr);
		cutt_exit(0);
	}

	for (i=0; i < col2; i++) temp2[i] = ' ';
	temp2[i] = EOS;
	if (!TwoColumn) {
		for (i=0; i < col3; i++) temp3[i] = ' ';
		temp3[i] = EOS;
	}
	MAXOUTCOL = col2 - 2L;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		/*
		printf("found=%d; sp=%s; uttline=%s", found, utterance->speaker, uttline);
		if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
		*/
		if (*utterance->speaker == '*' && !TierHeaders) *buf = EOS;
		else strcpy(buf, utterance->speaker);
		strcat(buf, utterance->line);
		if (*utterance->speaker == '%') {
			if (!TwoColumn) {
				MAXOUTCOL = SCREENWIDTH - 1L;
				if (AddLines == 1 || AddLines == 3)
					sprintf(temp3+col3, "%ld ", lineno);
				else
					temp3[col3] = EOS;
				uS.cleanUpCodes(buf, &dFnt, MBF);
				printout(temp3,buf,NULL,NULL,FALSE);
			} else if (RightSp) {
				if (AddLines == 1 || AddLines == 3)
					sprintf(templineC, "%ld ", lineno);
				else
					*templineC = EOS;
				uS.cleanUpCodes(buf, &dFnt, MBF);
				printout(templineC,buf,NULL,NULL,FALSE);
			} else {
				if (AddLines == 1 || AddLines == 3)
					sprintf(temp2+col2, "%ld ", lineno);
				else
					temp2[col2] = EOS;
				uS.cleanUpCodes(buf, &dFnt, MBF);
				printout(temp2,buf,NULL,NULL,FALSE);
			}
		} else if (*utterance->speaker == '@') {
			MAXOUTCOL = col2 - 2L;
			if (AddLines == 1)
				sprintf(templineC, "%ld ", lineno);
			else
				*templineC = EOS;
			uS.cleanUpCodes(buf, &dFnt, MBF);
			printout(templineC,buf,NULL,NULL,FALSE);
		} else {
			if (*ChildesName != '*')
				strcpy(templineC, utterance->speaker+1);
			else
				strcpy(templineC, utterance->speaker);
			uS.uppercasestr(templineC, &dFnt, MBF);
			if (uS.partcmp(templineC,ChildesName,FALSE,TRUE)) {
				RightSp = TRUE;
				MAXOUTCOL = col2 - 2L;
				if (AddLines > 0)
					sprintf(templineC, "%ld ", lineno);
				else
					*templineC = EOS;
				uS.cleanUpCodes(buf, &dFnt, MBF);
				printout(templineC,buf,NULL,NULL,FALSE);
			} else {
				RightSp = FALSE;
				MAXOUTCOL = col3 - 2L;
				if (AddLines > 0)
					sprintf(temp2+col2, "%ld ", lineno);
				else
					temp2[col2] = EOS;
				uS.cleanUpCodes(buf, &dFnt, MBF);
				printout(temp2,buf,NULL,NULL,FALSE);
			}
		}
	}
}
/*
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    CLAN_PROG_NUM = COLUMNS;
    chatmode = CHAT_MODE;
    OnlydataLimit = 0;
    UttlineEqUtterance = FALSE;
    bmain(argc,argv,NULL);
}
