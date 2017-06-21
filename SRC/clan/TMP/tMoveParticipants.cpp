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

#if defined(_MAC_CODE)
ClanProgInfo gToolInfo = {
	_main,
	usage,
	getflag,
	init,
	call,
	NULL
};
#endif /* _MAC_CODE */

struct tiers {
	unsigned int tierNumber;	/* tier number in CA data files */
    char speaker[SPEAKERLEN];	/* code descriptor field of the turn	*/
    char attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
    char line[UTTLINELEN+1];	/* text field of the turn		*/
    char attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
	struct tiers *NextUtt;
} *headers;

extern struct tier *defheadtier;
extern char LeaveSlashes;

void init(char f) {
	if (f) {
		headers = NULL;
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void usage() {
	printf("Usage: temp [%s] filename(s)\n",mainflgs());
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

static struct tiers *freeHeaders(struct tiers *p) {
	struct tiers *t;

	while (p != NULL) {
		t = p;
		p = p->NextUtt;
		free(t);
	}
	return(NULL);
}

static void freeAll(char isExit) {
	headers = freeHeaders(headers);
	if (isExit) {
		cutt_exit(0);
	}
}

static struct tiers *addToHeaders(struct tiers *p, char *sp, char *line, char *attSp, char *attLine, unsigned int tierNumber) {
	struct tiers *u;
	
	if ((u=p) == NULL) {
		p = NEW(struct tiers);
		if (p == NULL) {
			fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Out of memory\n");
			freeAll(TRUE);
		}
		u = p;
	} else {
		while (u->NextUtt != NULL) {
			u = u->NextUtt;
		}
		u->NextUtt = NEW(struct tiers);
		if (u->NextUtt == NULL) {
			fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Out of memory\n");
			freeAll(TRUE);
		}
		u = u->NextUtt;
	}
	u->NextUtt = NULL;
	att_cp(0, u->speaker, sp, u->attSp, attSp);
	att_cp(0, u->line, line, u->attLine, attLine);
	u->tierNumber = tierNumber;
	return(p);
}

static void finishHeaders(char *sp, char *line, char *attSp, char *attLine, unsigned int tierNumber) {
	struct tiers *h;

	for (h=headers; h != NULL; h=h->NextUtt) {
		strcpy(templineC,h->speaker);
		uS.uppercasestr(templineC, defScript, MBF);
		if (partcmp(templineC,"@BEGIN",FALSE)) {
			printout(h->speaker,h->line,h->attSp,h->attLine,h->tierNumber,FALSE);
			printout(sp,line,attSp,attLine,tierNumber,FALSE);
		} else {
			printout(h->speaker,h->line,h->attSp,h->attLine,h->tierNumber,FALSE);
		}
	}
}

void call() {
	char isFirstSpeaker;

	isFirstSpeaker = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		strcpy(templineC,utterance->speaker);
		uS.uppercasestr(templineC, defScript, MBF);
		if (isFirstSpeaker) {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,utterance->tierNumber,FALSE);
		} else if (partcmp(templineC,PARTICIP,FALSE)) {
			finishHeaders(utterance->speaker, utterance->line,utterance->attSp,utterance->attLine,utterance->tierNumber);
			headers = freeHeaders(headers);
			isFirstSpeaker = TRUE;
		} else {
			headers = addToHeaders(headers, utterance->speaker, utterance->line,utterance->attSp,utterance->attLine,utterance->tierNumber);
		}
	}
	freeAll(FALSE);
}
