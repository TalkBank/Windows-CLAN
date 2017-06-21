#include "ced.h"
#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define CHAT_MODE 0

extern struct tier *defheadtier;

char wcount;

#define TEXTSIZE 22000L
char *mainText;

#define INFOSIZE 5000
struct timing {
	long textIndex;
	long movieIndex;
} info[INFOSIZE];
long infoIndex;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		wcount = FALSE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
	}
}

void usage() {
	printf("Usage: temp(Informedia to CHAT) [-c %s] filename(s)\n",mainflgs());
    puts("+c: count words.");
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	mainText = (char *)malloc(TEXTSIZE);
	if (mainText == NULL) {
		fprintf(stderr, "OUT OF MEMORY; Please increase program's memory size\n");
		return;
	}
	bmain(argc,argv,NULL);
	free(mainText);
}
		
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
			wcount = TRUE;
			no_arg_option(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char Get_headers(void) {
	uttline[0] = '\t';
	uttline[1] = EOS;
	while (!feof(fpin)) {
		if (!fgets_cr(templineC, UTTLINELEN, fpin))
			break;
		if (templineC[0] == '*' && templineC[1] == '*')
			break;
		if (*templineC != '\n' && *templineC != EOS) {
			strcat(uttline, templineC);
			strcat(uttline, "\t\t");
		}
	}
	uS.remblanks(uttline);
	printout("@Comment:", uttline, NULL, NULL, 0, FALSE);
	return(feof(fpin));
}

static char Get_Transcript(void) {
	int i, len;

	len = 0L;
	mainText[0] = EOS;
	while (!feof(fpin)) {
		if (!fgets_cr(templineC, UTTLINELEN, fpin))
			break;
		if (isdigit(templineC[0])) {
			for (i=0; isdigit(templineC[i]); i++) ;
			for (; isSpace(templineC[i]); i++) ;
			if (strncmp(templineC+i, "timing pairs", 12) == 0)
				break;
		}
		len += strlen(templineC) + 1;
		if (len > TEXTSIZE) {
			fprintf(stderr, "Internal error; increase memory for mainText\n");
			cutt_exit(0);
		}
		strcat(mainText, templineC);
//		strcat(mainText, "\n");
	}
	return(feof(fpin));
}

static char Get_timing(void) {
	int i;

	infoIndex = 0L;
	while (!feof(fpin)) {
		if (!fgets_cr(templineC, UTTLINELEN, fpin))
			break;
		if (templineC[0] == '*' && templineC[1] == '*' && templineC[2] == '*' && templineC[3] == '*')
			break;
		if (isdigit(*templineC)) {
			if (infoIndex >= INFOSIZE) {
				fprintf(stderr, "Internal error; increase memory for info array\n");
				cutt_exit(0);
			}
			info[infoIndex].textIndex = atol(templineC);
			for (i=0; isdigit(templineC[i]); i++) ;
			for (; !isdigit(templineC[i]) && templineC[i] != EOS; i++) ;
			info[infoIndex].movieIndex = atol(templineC+i);
			infoIndex++;
		}
	}
	return(feof(fpin));
}

static void Match_timing(long beg, long end) {
	long i, len;
	long mBeg = 0L, mEnd = 0L;
	char begFound = FALSE;

	for (i=0L; i < infoIndex; i++) {
		if (!begFound && beg <= info[i].textIndex) {
			mBeg = info[i].movieIndex;
			begFound = TRUE;
		}
		if (end <= info[i].textIndex) {
			mEnd = info[i].movieIndex;
			if (mBeg == mEnd && i+1 < infoIndex)
				mEnd = info[i+1].movieIndex;
			break;
		}
	}
	if (mEnd == 0L) {
		if (infoIndex <= 1)
			mEnd = 0xFFFFFFF;
		else if (info[infoIndex-1].movieIndex <= mBeg)
			mEnd = mBeg;
		else
			mEnd = info[infoIndex-1].movieIndex;
	}
	len = end - beg;
	strncpy(templineC, mainText+beg, len);
	templineC[len] = EOS;
	for (i=0; isSpace(templineC[i]) || templineC[i] == '\n'; i++) ;
	if (i > 0)
		strcpy(templineC, templineC+i);
	uS.remblanks(templineC);
	if (*templineC != EOS) {
		len = strlen(templineC);
		if (begFound)
			sprintf(templineC+len, "%c%s\"%s\"_%ld_%ld%c",HIDEN_C,MOVIETIER,"cwvgh.mpg",mBeg,mEnd,HIDEN_C);
		printout("*TXT:", templineC, NULL, NULL, 0, TRUE);
	}
}

static void Match_segment(void) {
	long beg, end, cb;
	
	beg = 0L;
	cb = 0L;
	while (mainText[beg] != EOS) {
		end = beg;
/*
		for (; isSpace(mainText[end]) || mainText[end] == '{' ||
												mainText[end] == '\n' || cb; end++) {
			if (mainText[end] == '{')
				cb++;
			else if ((mainText[end] == '}' || mainText[end] == EOS) && cb > 0)
				cb--;
		}
*/
		Match_timing(beg, end);
		beg = end;
		if (mainText[beg] == EOS)
			break;
		end = beg;
		for (; !uS.IsUtteranceDel(mainText, end); end++) ;
		for (;  uS.IsUtteranceDel(mainText, end); end++) ;
		Match_timing(beg, end);
		beg = end;
	}
}

static char Proccess_segment(void) {
	if (Get_headers())
		return(TRUE);

	if (Get_Transcript()) {
		fprintf(stderr, "Unexpected ending - timing information for transcript is missing\n");
		return(TRUE);
	}
	if (Get_timing()) {
		Match_segment();
		return(TRUE);
	} else {
		Match_segment();
		return(FALSE);
	}
}

static void doWordCount(void) {
	long i = 1L, j;

	while (!feof(fpin)) {
		if (!fgets_cr(templineC, UTTLINELEN, fpin))
			break;
		j = 0;
		while (templineC[j]) {
			if (!isspace(templineC[j]) && templineC[j]) {
				fprintf(fpout, "%5ld: ", i);
				for (; !isspace(templineC[j]) && templineC[j]; j++, i++)
					putc(templineC[j],fpout);
				putc('\n',fpout);
			}
			for (; isspace(templineC[j]) && templineC[j]; j++, i++) ;
		}
	}
}

void call() {
	char done = FALSE;

	if (wcount) {
		doWordCount();
		return;
	}

#ifdef _WIN32 
	printout(FONTHEADER, "Win95:Courier New:-13", NULL, NULL, 0, TRUE);
#endif
#ifdef _MAC_CODE
	printout(FONTHEADER, "Courier:10", NULL, NULL, 0, TRUE);
#endif
	printout("@Begin", "", NULL, NULL, 0, FALSE);
	printout("@Participants:", "TXT Transcript", NULL, NULL, 0, TRUE);
	while (fgets_cr(templineC, UTTLINELEN, fpin)) {
		if (templineC[0] == '*' && templineC[1] == '*' && templineC[2] == '*' && templineC[3] == '*')
			break;
	}
	done = feof(fpin);
	while (!done) {
		done = Proccess_segment();
	}
	printout("@End", "", NULL, NULL, 0, FALSE);
}
