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

#if defined(_MAC_CODE) && defined(PLUGINPROJ)
ClanProgInfo gToolInfo = {
	_main,
	usage,
	getflag,
	init,
	call,
	NULL
};
#endif /* _MAC_CODE */

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char *options_ext[];
extern char ced_line[];

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
		options_ext[TEMP] = ".tmp";
	}
}

void usage() {
	printf("Usage: fixID [%s] filename(s)\n",mainflgs());
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

static long makeXMLAge(char *st, long i, char mc, char ec) {
	long s, j;
	int y, m, d;
	char isY, isM, isD, dash, t;

	isY = FALSE;
	isM = FALSE;
	isD = FALSE;
	dash = FALSE;
	y = 0;
	m = 0;
	d = 0;
	j = strlen(ced_line);
	while (st[i] != ec) {
		while (isSpace(st[i]) || st[i] == '\n') i++;
		if (st[i] == ec)
			break;
		s = i;
		while (isdigit(st[i])) i++;
		if (st[i] == ';') {
			isY = TRUE;
			st[i] = EOS;
			y = atoi(st+s);
			st[i] = ';';
			i++;
		} else if (st[i] == mc && isM == FALSE) {
			isM = TRUE;
			st[i] = EOS;
			m = atoi(st+s);
			st[i] = mc;
			i++;
		} else if (isY && isM) {
			isD = TRUE;
			t = st[i];
			st[i] = EOS;
			d = atoi(st+s);
			st[i] = t;
		}
		if (st[i] == '-') {
			if (m == 0 && d == 0 && !isM && !isD)
				sprintf(ced_line+strlen(ced_line), "P%dY - ", y);
			else if (d == 0 && !isD)
				sprintf(ced_line+strlen(ced_line), "P%dY%dM - ", y, m);
			else
				sprintf(ced_line+strlen(ced_line), "P%dY%dM%dD - ", y, m, d);
			dash = TRUE;
			isY = FALSE; isM = FALSE; isD = FALSE;
			y = 0; m = 0; d = 0;
			i++;
		}
		if (st[i] == ec)
			break;
		if ((st[i] == 't' || st[i] == 'T') && (st[i+1] == 'o' || st[i+1] == 'O')) {
			if (m == 0 && d == 0 && !isM && !isD)
				sprintf(ced_line+strlen(ced_line), "P%dY - ", y);
			else if (d == 0 && !isD)
				sprintf(ced_line+strlen(ced_line), "P%dY%dM - ", y, m);
			else
				sprintf(ced_line+strlen(ced_line), "P%dY%dM%dD - ", y, m, d);
			dash = TRUE;
			isY = FALSE; isM = FALSE; isD = FALSE;
			y = 0; m = 0; d = 0;
			st[i] = '-';
			i++;
			strcpy(st+i, st+i+1);
		} else if (!isSpace(st[i]) && !isdigit(st[i]) && st[i] != '\n') {
			fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Unexpected \"age\" format on @ID tier\n");
			fprintf(stderr, "%s%s\n", utterance->speaker, utterance->line);
			cutt_exit(0);
		}
	}
	if (!isY && !isM && !isD && dash) {
		fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr, "Unexpected \"age\" format on @ID tier\n");
		fprintf(stderr, "%s%s\n", utterance->speaker, utterance->line);
		cutt_exit(0);
	}
	if (m == 0 && d == 0 && !isM && !isD)
		sprintf(ced_line+strlen(ced_line), "P%dY", y);
	else if (d == 0 && !isD)
		sprintf(ced_line+strlen(ced_line), "P%dY%dM", y, m);
	else
		sprintf(ced_line+strlen(ced_line), "P%dY%dM%dD", y, m, d);
	return(i);
}

static long parseAge(char *st, long i, char mc, char ec) {
	char isY, isM, isD;

	isY = FALSE;
	isM = FALSE;
	isD = FALSE;
	while (st[i] != ec) {
		while (isSpace(st[i]) || st[i] == '\n') i++;
		if (st[i] == ec)
			break;
		while (isdigit(st[i])) i++;
		if (st[i] == ';') {
			isY = TRUE;
			i++;
		} else if (st[i] == mc && isM == FALSE) {
			isM = TRUE;
			if (st[i] == '-')
				st[i] = '.';
			i++;
		} else if (st[i] == '.' && ec == EOS && isM == FALSE) {
			isM = TRUE;
			i++;
		} else if (isY && isM) {
			isD = TRUE;
		}
		if (st[i] == '-') {
			isY = FALSE;
			isM = FALSE;
			isD = FALSE;
			i++;
		}
		if ((st[i] == 't' || st[i] == 'T') && (st[i+1] == 'o' || st[i+1] == 'O')) {
			isY = FALSE;
			isM = FALSE;
			isD = FALSE;
			st[i] = '-';
			i++;
			strcpy(st+i, st+i+1);
		} else if (!isSpace(st[i]) && !isdigit(st[i]) && st[i] != ec && st[i] != '\n') {
			fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Unexpected \"@Age of\" format\n");
			fprintf(stderr, "%s%s\n", utterance->speaker, utterance->line);
			cutt_exit(0);
		}
	}
	return(i);
}

static void parseID(char mc, char ec, char cv) {
	int  cnt;
	long i, j;

	cnt = 0;
	i = 0L;
	j = 0;
	ced_line[0] = EOS;
	while (utterance->line[i] != EOS) {
		if (utterance->line[i] == ec) {
			if (cv != EOS)
				utterance->line[i] = cv;
			cnt++;
		} else if (utterance->line[i] == cv) {
			fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Found \".\" on @ID tier\n");
			fprintf(stderr, "%s%s\n", utterance->speaker, utterance->line);
			cutt_exit(0);
		}
		ced_line[j++] = utterance->line[i];
		if (cnt == 3) {
			ced_line[j] = EOS;
//			i = makeXMLAge(utterance->line, i+1, mc, ec);
			i = parseAge(utterance->line, i+1, mc, ec);
			j = strlen(ced_line);
		} else
			i++;
	}
	ced_line[j] = EOS;
//	strcpy(utterance->line, ced_line);
}

void call() {
	char mc, ec, cv;

	ec = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		strcpy(templineC, utterance->speaker);
		uS.uppercasestr(templineC, dFnt.Encod, FALSE);
		if (partcmp(templineC, "@ID:", FALSE)) {
			if (strncmp(utterance->line, "english", 7) == 0) {
				strcpy(utterance->line+2, utterance->line+7);
			}
			if (utterance->line[2] == '|') {
				ec = '|';
				mc = '.';
				cv = '.';
			} else if (utterance->line[2] == '.') {
				ec = '.';
				mc = '-';
				cv = '|';
			} else {
				fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Can't find field delimiter on @ID tier\n");
				fprintf(stderr, "%s%s\n", utterance->speaker, utterance->line);
				cutt_exit(0);
			}
			if (ec != '|')
				parseID(mc, ec, cv);
		} else if (partcmp(templineC, "@AGE OF", FALSE)) {
			if (ec == EOS) {
				fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Found @Age of before @ID tier\n");
				fprintf(stderr, "%s%s\n", utterance->speaker, utterance->line);
				cutt_exit(0);
			}
// old age format
			parseAge(utterance->line, 0L, mc, EOS);
// old age format
/* xml age format
			ced_line[0] = EOS;
			makeXMLAge(utterance->line, 0, mc, EOS);
			strcpy(utterance->line, ced_line);
*/// xml age format
		}
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
}
