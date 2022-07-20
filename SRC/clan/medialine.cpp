/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#ifndef UNX
	#include "ced.h"
#else
	#include "c_curses.h"
#endif

#if !defined(UNX)
#define _main medialine_main
#define call medialine_call
#define getflag medialine_getflag
#define init medialine_init
#define usage medialine_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

extern struct tier *defheadtier;
extern char OverWriteFile;

#define HEADERTIERS struct HeadTiers
HEADERTIERS {
	char *sp;
	AttTYPE *attSp;
	char *line;
	AttTYPE *attLine;
	HEADERTIERS *nextTier;
} ;

static char isAddAudio, isAddVideo, isMissing, isUnlinked;
static HEADERTIERS *tiersRoot;

void init(char f) {
	if (f) {
		tiersRoot = NULL;
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		isAddAudio = FALSE;
		isAddVideo = FALSE;
		isMissing = FALSE;
		isUnlinked = FALSE;
	} else {
		if (!isAddAudio && !isAddVideo) {
			fprintf(stderr,"\n    Specify either +a or +v option.\n");
			cutt_exit(0);
		}
	}
}

void usage() {
	printf("Adds @Media header with media filename derived from .cha file name.\n");
	printf("Usage: medialine [%s] filename(s)\n", mainflgs());
	puts("+a: add \"audio\" to @media header");
	puts("+v: add \"video\" to @media header");
	puts("+m: add \"missing\" to @media header");
	puts("+u: add \"unlinked\" to @media header");
	mainusage(TRUE);
}

static HEADERTIERS *freeTiers(HEADERTIERS *p) {
	HEADERTIERS *t;

	while (p != NULL) {
		t = p;
		p = p->nextTier;
		if (t->sp != NULL)
			free(t->sp);
		if (t->attSp != NULL)
			free(t->attSp);
		if (t->line != NULL)
			free(t->line);
		if (t->attLine != NULL)
			free(t->attLine);
		free(t);
	}
	return(NULL);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = MEDIALINE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'a':
			isAddAudio = TRUE;
			break;
		case 'v':
			isAddVideo = TRUE;
			break;
		case 'm':
			isMissing = TRUE;
			break;
		case 'u':
			isUnlinked = TRUE;
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static HEADERTIERS *addHeaderTier(HEADERTIERS *root) {
	int i;
	HEADERTIERS *p = NULL;

	if (root == NULL) {
		p = NEW(HEADERTIERS);
		root = p;
	} else {
		for (p=root; p->nextTier != NULL; p=p->nextTier) ;
		p->nextTier = NEW(HEADERTIERS);
		p = p->nextTier;
	}
	if (p == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		root = freeTiers(root);
		cutt_exit(0);
	}
	p->nextTier = NULL;
	p->sp = NULL;
	p->attSp = NULL;
	p->line = NULL;
	p->attLine = NULL;
	i = strlen(utterance->speaker);
	if ((p->sp=(char *)malloc(i+1)) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		root = freeTiers(root);
		cutt_exit(0);
	}
	if ((p->attSp=(AttTYPE *)malloc((i+1)*sizeof(AttTYPE))) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		root = freeTiers(root);
		cutt_exit(0);
	}
	i = strlen(utterance->line);
	if ((p->line=(char *)malloc(i+1)) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		root = freeTiers(root);
		cutt_exit(0);
	}
	if ((p->attLine=(AttTYPE *)malloc((i+1)*sizeof(AttTYPE))) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		root = freeTiers(root);
		cutt_exit(0);
	}
	att_cp(0L, p->sp, utterance->speaker, p->attSp, utterance->attSp);
	att_cp(0L, p->line, utterance->line, p->attLine, utterance->attLine);
	uS.remblanks(p->sp);
	return(root);
}

static void printOutHeaders(HEADERTIERS *tier, char *mediafname) {
	char isidf;

	isidf = FALSE;
	for (; tier != NULL; tier=tier->nextTier) {
		if (uS.partcmp(tier->sp,"@ID:",FALSE,FALSE) || uS.partcmp(tier->sp,"@Birth of ",FALSE,FALSE)) {
			isidf = TRUE;
		}
		if (mediafname[0] != EOS && isidf && !uS.partcmp(tier->sp,"@ID:",FALSE,FALSE) && !uS.partcmp(tier->sp,"@Birth of ",FALSE,FALSE)) {
			strcpy(spareTier1, "@Media:\t");
			strcat(spareTier1, mediafname);
			if (isAddVideo)
				strcat(spareTier1, ", video");
			if (isAddAudio)
				strcat(spareTier1, ", audio");
			if (isMissing)
				strcat(spareTier1, ", missing");
			if (isUnlinked)
				strcat(spareTier1, ", unlinked");
			fprintf(fpout, "%s\n", spareTier1);
			mediafname[0] = EOS;
		}
		printout(tier->sp, tier->line, tier->attSp, tier->attLine, FALSE);
	}
	if (mediafname[0] != EOS) {
		strcpy(spareTier1, "@Media:\t");
		strcat(spareTier1, mediafname);
		if (isAddVideo)
			strcat(spareTier1, ", video");
		if (isAddAudio)
			strcat(spareTier1, ", audio");
		if (isMissing)
			strcat(spareTier1, ", missing");
		if (isUnlinked)
			strcat(spareTier1, ", unlinked");
		fprintf(fpout, "%s\n", spareTier1);
		mediafname[0] = EOS;
	}
}

void call() {
	char *s, mediafname[BUFSIZ], isContinue;

	extractFileName(mediafname, oldfname);
	s = strchr(mediafname, '.');
	if (s != NULL)
		*s = '\0';
	isContinue = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*' || uS.partcmp(utterance->speaker, "@End", FALSE, FALSE)) {
			printOutHeaders(tiersRoot, mediafname);
			tiersRoot = freeTiers(tiersRoot);
			printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
			isContinue = TRUE;
			break;
		} else if (!uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE)) {
			tiersRoot = addHeaderTier(tiersRoot);
		}
	}
	if (mediafname[0] != EOS) {
		fprintf(fpout, "@Media:\t%s, audio\n", mediafname);
		mediafname[0] = EOS;
	}
	if (isContinue) {
		while (getwholeutter()) {
			printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
		}
	}
	tiersRoot = freeTiers(tiersRoot);
}
