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

extern char GExt[];
extern char OverWriteFile;
extern char outputOnlyData;

struct temp_list {
	char *st;
	struct temp_list *fnames;
	struct temp_list *next_sp;
} ;

extern struct tier *defheadtier;
extern char OverWriteFile;

static char temp_ftime;
static struct temp_list *sp_head;

void init(char f) {
	if (f) {
		temp_ftime = TRUE;
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		outputOnlyData = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (temp_ftime) {
			temp_ftime = FALSE;
			AddCEXExtension = ".csv";
			combinput = TRUE;
		}

	}
}

void usage() {
	printf("Create a list of all speakers and files they are in\n");
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	cutt_exit(0);
}

static struct temp_list *freefnames(struct temp_list *p) {
	struct temp_list *ts;

	while (p) {
		ts = p;
		p = p->fnames;
		if (ts->st != NULL)
			free(ts->st);
		free(ts);
	}
	return(NULL);
}
static struct temp_list *freelist(struct temp_list *p) {
	struct temp_list *ts;

	while (p) {
		ts = p;
		p = p->next_sp;
		if (ts->st != NULL)
			free(ts->st);
		if (ts->fnames != NULL)
			ts->fnames = freefnames(ts->fnames);
		free(ts);
	}
	return(NULL);
}

static void temp_error(char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	sp_head = freelist(sp_head);
	cutt_exit(0);
}

static struct temp_list *addFname(temp_list *root, char *fname) {
	char *s;
	struct temp_list *ts;
	
	uS.remblanks(fname);
	s = strrchr(fname, '/');
	if (s != NULL) {
		strcpy(FileName1, s+1);
	} else {
		strcpy(FileName1, fname);
	}
	if (root == NULL) {
		if ((root=NEW(struct temp_list)) == NULL)
			temp_error(TRUE);
		ts = root;
	} else {
		for (ts=root; ts->fnames != NULL; ts=ts->fnames) ;
		if ((ts->fnames=NEW(struct temp_list)) == NULL)
			temp_error(TRUE);
		ts = ts->fnames;
	}
	ts->fnames = NULL;
	ts->next_sp = NULL;
	if ((ts->st=(char *)malloc(strlen(FileName1)+1)) == NULL)
		temp_error(TRUE);
	strcpy(ts->st, FileName1);
	return(root);
}

static struct temp_list *temp_addSp(temp_list *root, char *sp) {
	struct temp_list *ts;

	uS.remblanks(sp);
	if (root == NULL) {
		if ((root=NEW(struct temp_list)) == NULL)
			temp_error(TRUE);
		ts = root;
	} else {
		for (ts=root; ts->next_sp != NULL; ts=ts->next_sp) {
			if (uS.partcmp(ts->st, sp, FALSE, FALSE)) {
				ts->fnames = addFname(ts->fnames, oldfname);
				return(root);
			}
		}
		if (uS.partcmp(ts->st, sp, FALSE, FALSE)) {
			ts->fnames = addFname(ts->fnames, oldfname);
			return(root);
		}
		if ((ts->next_sp=NEW(struct temp_list)) == NULL)
			temp_error(TRUE);
		ts = ts->next_sp;
	}
	ts->next_sp = NULL;
	if ((ts->st=(char *)malloc(strlen(sp)+1)) == NULL)
		temp_error(TRUE);
	strcpy(ts->st, sp);
	ts->fnames = addFname(ts->fnames, oldfname);
	return(root);
}

static void pr_result(void) {
	struct temp_list *ts, *tf;

	ts = sp_head;
	while (ts) {
		fprintf(fpout, "%s", ts->st);
		tf = ts->fnames;
		while (tf != NULL) {
			fprintf(fpout, ",%s", tf->st);
			tf = tf->fnames;
		}
		fprintf(fpout, "\n");
		ts = ts->next_sp;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	
	bmain(argc,argv,pr_result);

	sp_head = freelist(sp_head);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

void call() {
	int  i, j;
	char found;
	char spc[BUFSIZ+2];
	char sp[BUFSIZ+2];

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker, "@Comment:", FALSE, FALSE)) {
			strcpy(spc, utterance->line);
			found = FALSE;
			for (i=0; spc[i] != EOS; i++) {
				if (uS.partcmp(spc+i, "*PAR0:", FALSE, FALSE) || uS.partcmp(spc+i, "*PAR1:", FALSE, FALSE)) {
					i += 7;
					j = 0;
					for (; spc[i] != EOS && !isSpace(spc[i]) && spc[i] != '\n'; i++) {
						sp[j++] = spc[i];
					}
					sp[j] = EOS;
					sp_head = temp_addSp(sp_head, sp);
					found = TRUE;
				}
			}
			if (found == TRUE)
				break;
		}
	}
}
