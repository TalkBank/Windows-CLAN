#include "cu.h"
#include "ced.h"
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

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#ifdef UNX
	#define FILETYPE 0L
#else
	#define FILETYPE 'TEXT'
#endif

extern struct tier *defheadtier;
extern char OverWriteFile;

enum {
	READY,
	OUTPUT,
	DONE
} ;

struct DEPTIERS {
	char *sp;
	char *line;
	long time;
	char status;
	struct DEPTIERS *next;
} *rootDepTier;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

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
		rootDepTier = NULL;
	}
}

void usage() {
	printf("merge linked CHAT file with time-marked comment files\n");
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage();
}


static struct DEPTIERS *freeDepTiers(struct DEPTIERS *p, char *fname) {
	struct DEPTIERS *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->status == READY) {
			fprintf(stderr, "****Can't match tier");
			if (fname != NULL)
				fprintf(stderr, " in file \"%s\"", fname);
			fprintf(stderr, " to time %ld:\n", t->time);
			fprintf(stderr, "  %s\t%s\n", t->sp, t->line);
		}
		if (t->sp)
			free(t->sp);
		if (t->line)
			free(t->line);
		free(t);
	}
	return(NULL);
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

static struct DEPTIERS *makeDepTier(char *sp, long time, char *line) {
	struct DEPTIERS *nextone;

	if (rootDepTier == NULL) {
		rootDepTier = NEW(struct DEPTIERS);
		nextone = rootDepTier;
	} else {
		nextone = rootDepTier;
		while (nextone->next != NULL)
			nextone = nextone->next;
		nextone->next = NEW(struct DEPTIERS);
		nextone = nextone->next;
	}
	if (nextone == NULL) {
		fprintf(stderr, "OUT OF MEMORY\n");
		freeDepTiers(rootDepTier, NULL);
		cutt_exit(0);
	}
	nextone->status = READY;
	nextone->time = time;
	nextone->line = NULL;
	nextone->next = NULL;
	nextone->sp = (char *)malloc(strlen(sp)+3);
	if (nextone->sp == NULL) {
		fprintf(stderr, "OUT OF MEMORY\n");
		freeDepTiers(rootDepTier, NULL);
		cutt_exit(0);
	}
	strcpy(nextone->sp,"%");
	strcat(nextone->sp,sp);
	strcat(nextone->sp,":");
	nextone->line = (char *)malloc(strlen(line)+1);
	if (nextone->line == NULL) {
		fprintf(stderr, "OUT OF MEMORY\n");
		freeDepTiers(rootDepTier, NULL);
		cutt_exit(0);
	}
	strcpy(nextone->line, line);
	return(rootDepTier);
}

static long convTime2Msecs(char *s) {
	char *col, *num, count;
	double time;

	s[8] = EOS;
	count = 1;
	time = 0.0;
	do {
		col = strrchr(s,':');
		if (col != NULL) {
			*col = EOS;
			num = col + 1;
		} else 
			num = s;

		if (count == 1) {
			time = time + ((double)atof(num) * 1000.0);
		} else if (count == 2) {
			time = time + ((double)atof(num) * 60.0 * 1000.0);
		} else if (count == 3) {
			time = time + ((double)atof(num) * 3600.0 * 1000.0);
		}
		count++;
	} while (col != NULL && count < 4) ;
	return((long)time);
}

static char isTimeMarker(char *st) {
	if (isdigit(st[0]) && isdigit(st[1]) && st[2] == ':' &&
		isdigit(st[3]) && isdigit(st[4]) && st[5] == ':' &&
		isdigit(st[6]) && isdigit(st[7]) && isSpace(st[8]))
		return(TRUE);
	else {
		if (isdigit(st[0]) && isdigit(st[1]) && st[2] == ':') {
			fprintf(stderr, "Possible time marker:\n\t%s", st);
		}
		return(FALSE);
	}
}

static void getDepTiers(char *fname, FILE *fp) {
	int i, j;
	char *ext;

	ext = strrchr(fname, '.');
	if (ext != NULL)
		*ext = EOS;
	templineC[0] = EOS;
	while (fgets_cr(templineC1, UTTLINELEN, fp) != NULL) {
		if (uS.isUTF8(templineC1))
			continue;
		if (templineC1[0] == (char)0xEF && templineC1[1] == (char)0xBB && templineC1[2] == (char)0xBF)
			i = 3;
		else
			i = 0;
		while (isSpace(templineC1[i]) || templineC1[i] == '\n')
			i++;
		if (isTimeMarker(templineC1+i)) {
			if (templineC[0] != EOS) {
				j = 8;
				while (isSpace(templineC[j]))
					j++;
				rootDepTier = makeDepTier(fname, convTime2Msecs(templineC), templineC+j);
			}
			strcpy(templineC, templineC1+i);
		} else if (templineC1[i] != EOS)
			strcat(templineC, templineC1);
	}
	if (templineC[0] != EOS) {
		j = 8;
		while (isSpace(templineC[j]))
			j++;
		rootDepTier = makeDepTier(fname, convTime2Msecs(templineC), templineC+j);
	}
}

static void readDepTiers(void) {
	int  index;
	long cnt, tcnt;
	char pat[FILENAME_MAX+1];
	char tFName[FILENAME_MAX];
	myMacDirRef dref, *tdref;
	FILE *fp;

#if defined(_MAC_CODE)
	HGetVol(nil, &dref.vRefNum, &dref.parID);
	tdref = &dref;
#else
	tdref = NULL;
#endif
	strcpy(pat, "*.txt");
	uS.lowercasestr(pat, &dFnt, C_MBF);
	cnt = 0L;
	tcnt = 0L;
	index = 1;
	while (index=Get_File(tFName, tdref, index, FILETYPE, 0L, FALSE)) {
		uS.lowercasestr(tFName, &dFnt, C_MBF);
		if (uS.fpatmat(tFName, pat)) {
			if ((fp = fopen(tFName, "r"))) {
				fprintf(stderr,"\rUsing dep tier: %s.\n", tFName);
				getDepTiers(tFName, fp);
				fclose(fp);
				fp = NULL;
			}
		}
	}
}

static void OutputDepTiers(struct DEPTIERS *p) {
	for (; p != NULL; p=p->next) {
		if (p->status == OUTPUT) {
			printout(p->sp, p->line, NULL, NULL, TRUE);
			p->status = DONE;
		}
	}
}

static int FindBulletInfoInLine(char *line, int index, long *beg, long *end) {
	register int i;
	char buf[FILENAME_MAX];

	if (line[index] != HIDEN_C)
		return(FALSE);
	for (i=0, index++; line[index]; i++, index++) {
		if (line[index] != MOVIETIER[i])
			break;
	}
	if (MOVIETIER[i] != EOS)
		return(FALSE);
	for (; line[index] && (isSpace(line[index]) || line[index] == '_'); index++) ;
	if (line[index] != '"')
		return(FALSE);
	index++;
	if (line[index] == EOS)
		return(FALSE);
	for (i=0; line[index] && line[index] != '"'; index++)
		buf[i++] = line[index];
	buf[i] = EOS;
	if (line[index] == EOS)
		return(FALSE);
	
	for (; line[index] && !isdigit((int)line[index]); index++) ;
	if (line[index] == EOS)
		return(FALSE);
	for (i=0; line[index] && isdigit((int)line[index]); index++)
		buf[i++] = line[index];
	buf[i] = EOS;
	*beg = atol(buf);

	for (; line[index] && !isdigit((int)line[index]); index++) ;
	if (line[index] == EOS)
		return(FALSE);
	for (i=0; line[index] && isdigit((int)line[index]); index++)
		buf[i++] = line[index];
	buf[i] = EOS;
	*end = atol(buf);

	for (; line[index] && (isSpace(line[index]) || line[index] == '_'); index++) ;
	if (line[index] == '-') {
		index++;
	}

	if (line[index] != HIDEN_C)
		return(FALSE);
	return(TRUE);
}

static void getLinkTimeAndMarkDepTier(char *line) {
	int i;
	long beg, end;
	struct DEPTIERS *p;

	for (i=0; line[i] != EOS; i++) {
		if (line[i] == HIDEN_C) {
			if (FindBulletInfoInLine(line, i, &beg, &end)) {
				for (p=rootDepTier; p != NULL; p=p->next) {
					if (p->status == READY && p->time >= beg && p->time <= end)
						p->status = OUTPUT;
				}
			}
			for (i++; line[i] != HIDEN_C && line[i] != EOS; i++) ;
			if (line[i] == EOS)
				break;
		}
	}
}

void call() {
	readDepTiers();
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		getLinkTimeAndMarkDepTier(utterance->line);
		printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
		OutputDepTiers(rootDepTier);
	}
	rootDepTier = freeDepTiers(rootDepTier, oldfname);
}
