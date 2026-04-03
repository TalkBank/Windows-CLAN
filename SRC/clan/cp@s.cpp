/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#define TIERS struct cpsT
struct cpsT {
    char speaker[SPEAKERLEN];
    AttTYPE attSp[SPEAKERLEN];
    char line[UTTLINELEN+1];
    AttTYPE attLine[UTTLINELEN+1];
	TIERS *NextTier;
} ;

extern struct tier *defheadtier;
extern char OverWriteFile;

static long clineno;
static TIERS *RootTier;

void usage() {
	puts("TEMP copy @s from %ort to speakeer tier");
	printf("Usage: temp [%s] filename(s)\n",mainflgs());
	mainusage(TRUE);
}

static TIERS *freeTiersCluster(TIERS *p) {
	TIERS *t;

	while (p != NULL) {
		t = p;
		p = p->NextTier;
		free(t);
	}
	return(NULL);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
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

void init(char f) {
	if (f) {
		RootTier = NULL;
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		RootTier = freeTiersCluster(RootTier);
		RootTier = NULL;
	}
}

static TIERS *addDepTiers(TIERS *root, UTTER *utt) {
	TIERS *nt;

	if (root == NULL) {
		root = NEW(TIERS);
		if (root == NULL) {
			RootTier = freeTiersCluster(RootTier);
			fprintf(stderr,"Out of memory.\n");
			cutt_exit(0);
		}
		nt = root;
	} else {
		for (nt=root; nt->NextTier != NULL; nt = nt->NextTier) ;
		nt->NextTier = NEW(TIERS);
		nt = nt->NextTier;
	}
	if (nt == NULL) {
		RootTier = freeTiersCluster(RootTier);
		fprintf(stderr,"Out of memory.\n");
		cutt_exit(0);
	}
	nt->NextTier = NULL;
	att_cp(0, nt->speaker, utt->speaker, nt->attSp, utt->attSp);
	att_cp(0, nt->line, utt->line, nt->attLine, utt->attLine);
	return(root);
}

static int lenOfAts(char *ortat) {
	int len;
	
	for (len=0; !uS.isskip(ortat,len,&dFnt,MBF) && ortat[len] != EOS; len++) ;
	return(len);
}

static char checkForandAddAts(TIERS *sp, TIERS *ort) {
	int  spi, orti, len;
	char spsq, ortsq, spbul, ortbul;
	char *spat, *ortat;

	spi = 0;
	orti = 0;
	while (sp->line[spi] != EOS && ort->line[orti] != EOS) {
		if (sp->line[spi] == '[')
			spsq = TRUE;
		else
			spsq = FALSE;
		if (sp->line[spi] == HIDEN_C)
			spbul = TRUE;
		else
			spbul = FALSE;
		while ((uS.isskip(sp->line,spi,&dFnt,MBF) || spsq || spbul) && sp->line[spi] != EOS) {
			spi++;
			if (sp->line[spi] == '[')
				spsq = TRUE;
			else if (sp->line[spi] == ']')
				spsq = FALSE;
			else if (sp->line[spi] == HIDEN_C)
				spbul = !spbul;
		}
		if (ort->line[orti] == '[')
			ortsq = TRUE;
		else
			ortsq = FALSE;
		if (ort->line[orti] == HIDEN_C)
			ortbul = TRUE;
		else
			ortbul = FALSE;
		while ((uS.isskip(ort->line,orti,&dFnt,MBF) || ortsq || ortbul) && ort->line[orti] != EOS) {
			orti++;
			if (ort->line[orti] == '[')
				ortsq = TRUE;
			else if (ort->line[orti] == ']')
				ortsq = FALSE;
			else if (ort->line[orti] == HIDEN_C)
				ortbul = !ortbul;
		}

		spat = NULL;
		while (!uS.isskip(sp->line,spi,&dFnt,MBF) && sp->line[spi] != EOS) {
			if (sp->line[spi] == '@')
				spat = sp->line + spi;
			spi++;
		}
		ortat = NULL;
		while (!uS.isskip(ort->line,orti,&dFnt,MBF) && ort->line[orti] != EOS) {
			if (ort->line[orti] == '@')
				ortat = ort->line + orti;
			orti++;
		}
		
		if (spat == NULL && ortat != NULL) {
			len = lenOfAts(ortat);
			att_shiftright(sp->line+spi, sp->attLine+spi, len);
			while (len > 0) {
				sp->line[spi] = *ortat;
				sp->attLine[spi] = 0;
				spi++;
				ortat++;
				len--;
			}
		}
	}
	if (sp->line[spi] == EOS && ort->line[orti] != EOS) {
		fprintf(stderr,"\r*** File \"%s\": line %ld.\n", oldfname, clineno);
		fprintf(stderr,"Speaker tier has fewer words than %%ort tier.\n");
		return(TRUE);
	} else
		return(FALSE);
}

static void PrintOutTiers(void) {
	char errFound;
	TIERS *sp, *ort;
	TIERS *p;

	errFound = FALSE;
	sp  = NULL;
	ort = NULL;
	for (p=RootTier; p != NULL; p=p->NextTier) {
		if (p->speaker[0] == '*') {
			sp = p;
			break;
		}
	}
	for (p=RootTier; p != NULL; p=p->NextTier) {
		if (uS.partcmp(p->speaker, "%ort:", FALSE, FALSE)) {
			ort = p;
			break;
		}
	}
	if (sp == NULL && ort != NULL) {
		fprintf(stderr,"\r*** File \"%s\": line %ld.\n", oldfname, clineno);
		fprintf(stderr,"Can't find speker tier.\n");
		cutt_exit(0);
	} else if (ort == NULL && sp != NULL) {
		fprintf(stderr,"\r*** File \"%s\": line %ld.\n", oldfname, clineno);
		fprintf(stderr,"Can't find %%ort: tier.\n");
	} else if (sp != NULL && ort != NULL) {
		if (strchr(ort->line, '@') != NULL)
			errFound = checkForandAddAts(sp, ort);
	}
	for (p=RootTier; p != NULL; p=p->NextTier) {
		if (p->speaker[0] == '*')
			printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
	}
	if (ort == NULL && sp != NULL)
		fprintf(fpout,"%%err:\tCan't find %%ort: tier.\n");
	if (errFound)
		fprintf(fpout,"%%err:\tSpeaker tier has fewer words than %%ort tier.\n");
	for (p=RootTier; p != NULL; p=p->NextTier) {
		if (p->speaker[0] != '*')
			printout(p->speaker,p->line,p->attSp,p->attLine,FALSE);
	}
	RootTier = freeTiersCluster(RootTier);
}

void call() {

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	clineno = lineno;
	while (getwholeutter()) {
//		if (lineno % 200 == 0)
//			fprintf(stderr,"\r%ld ", lineno);

		if (*utterance->speaker == '@') {
			PrintOutTiers();
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			if (*utterance->speaker == '*') {
				PrintOutTiers();
				clineno = lineno;
			}
			RootTier = addDepTiers(RootTier, utterance);
		}
	}
	PrintOutTiers();
	fprintf(stderr, "\r	     \r");
}
