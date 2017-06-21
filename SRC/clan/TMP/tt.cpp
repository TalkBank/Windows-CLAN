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
#define CHAT_MODE 0

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

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		OverWriteFile = TRUE;
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
	printf("Usage: tt [%s] filename(s)\n",mainflgs());
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

static long extractTime(char *st, long *from, long *to) {
	long b, e, fm, fs, fms, tm, ts, tms;

	fm = fs = fms = tm = ts = tms = 0L;
	b = 0L;
	for (b=0L; isSpace(st[b]); b++) ;
	if (!isdigit(st[b])) {
		fprintf(stderr, "Unexpected ending on line:\n%s", st);
		cutt_exit(0);
	}
	for (e=b; st[e] != EOS && st[e] != ':' && st[e] != '.'; e++) ;
	if (st[e] != '.') {
		fm = atol(st+b);
		b = e;
		if (st[b] != EOS)
			b++;
		if (!isdigit(st[b])) {
			fprintf(stderr, "Unexpected ending on line:\n%s", st);
			cutt_exit(0);
		}
		for (e=b; st[e] != EOS && st[e] != '.'; e++) ;
	}
	fs = atol(st+b);
	b = e;
	if (st[b] != EOS)
		b++;
	if (!isdigit(st[b])) {
		fprintf(stderr, "Unexpected ending on line:\n%s", st);
		cutt_exit(0);
	}
	for (e=b; st[e] != EOS && st[e] != '-'; e++) ;
	fms = atol(st+b);
	b = e;
	if (st[b] != EOS)
		b++;
	if (!isdigit(st[b])) {
		fprintf(stderr, "Unexpected ending on line:\n%s", st);
		cutt_exit(0);
	}

	for (e=b; st[e] != EOS && st[e] != ':' && st[e] != '.'; e++) ;
	if (st[e] != '.') {
		tm = atol(st+b);
		b = e;
		if (st[b] != EOS)
			b++;
		if (!isdigit(st[b])) {
			fprintf(stderr, "Unexpected ending on line:\n%s", st);
			cutt_exit(0);
		}
		for (e=b; st[e] != EOS && st[e] != '.'; e++) ;
	}
	ts = atol(st+b);
	b = e;
	if (st[b] != EOS)
		b++;
	if (!isdigit(st[b])) {
		fprintf(stderr, "Unexpected ending on line:\n%s", st);
		cutt_exit(0);
	}
	for (e=b; st[e] != EOS && st[e] != '\t'; e++) ;
	tms = atol(st+b);
	b = e;
	if (st[b] != '\t') {
		fprintf(stderr, "Unexpected ending on line:\n%s", st);
		cutt_exit(0);
	}
	b++;
	*from = fms + (fs * 1000) + (fm * 60000L);
	*to   = tms + (ts * 1000) + (tm * 60000L);
	return(b);
}

void call() {
	long i, from, to;

	while (!feof(fpin)) {
		if (fgets_cr(templineC, UTTLINELEN-1, fpin) == NULL)
			break;
		i = extractTime(templineC, &from, &to);
		if (i > 0)
			strcpy(templineC, templineC+i);
		i = strlen(templineC) - 1;
		if (templineC[i] == '\n')
			templineC[i] = EOS;
		else
			i++;
		sprintf(templineC+i, " %c%%mov:\"x.mov\"_%ld_%ld%c\n", HIDEN_C, from, to, HIDEN_C);
		fputs(templineC, fpout);
	}
}
