#define CHAT_MODE 3

#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

struct rtiers {
	char *speaker;	/* code descriptor field of the turn	 */
	AttTYPE *attSp;
	char *line;		/* text field of the turn		 */
	AttTYPE *attLine;
	struct rtiers *nexttier;	/* pointer to the next utterance, if	 */
} ;				/* there is only 1 utterance, i.e. no	 */

struct files {
	int  res;
	long lineno;
	long tlineno;
	char fname[256];
	char currentchar;
	AttTYPE currentatt;
	FILE *fpin;
	struct rtiers *tiers;
} ;

static struct files mbFFile, mbSFile;

char mbOutFile[256];

extern struct tier *headtier;
extern struct tier *defheadtier;

char mbFirstFile;

void usage() {
	printf("Usage: temp [%s] master-filename second-filename\n", mainflgs());
	mainusage();
}

void init(char first) {
	if (first) {
		nomap = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static struct rtiers *FreeUpTiers(struct rtiers *p) {
	struct rtiers *t;

	while (p != NULL) {
		t = p;
		p = p->nexttier;
		free(t->speaker);
		free(t->attSp);
		free(t->line);
		free(t->attLine);
		free(t);
	}
	return(NULL);
}

static struct rtiers *AddTiers(struct rtiers *p, char *sp, AttTYPE *attSp, char *line, AttTYPE *attLine) {
	struct rtiers *t = p;

	if (p == NULL) {
		if ((p=NEW(struct rtiers)) == NULL) out_of_mem();
		if ((p->speaker=(char *)malloc(strlen(sp)+1)) == NULL) out_of_mem();
		if ((p->attSp=(AttTYPE *)malloc((strlen(sp)+1)*sizeof(AttTYPE))) == NULL) out_of_mem();
		if ((p->line=(char *)malloc(strlen(line)+1)) == NULL) out_of_mem();
		if ((p->attLine=(AttTYPE *)malloc((strlen(line)+1)*sizeof(AttTYPE))) == NULL) out_of_mem();
		att_cp(0, p->speaker, sp, p->attSp, attSp);
		att_cp(0, p->line, line, p->attLine, attLine);
		p->nexttier = NULL;
	} else p->nexttier = AddTiers(p->nexttier, sp, attSp, line, attLine);
	return(p);
}

static void CompareTiers(void) {
	register int i;
	char word[BUFSIZ], rightTier;

//		fprintf(stderr, "Different tiers found:\n");
//		fprintf(stderr, "    on line %ld in file \"%s\": %s\n", mbFFile.lineno, mbFFile.fname, templineC);
//		fprintf(stderr, "    on line %ld in file \"%s\": %s\n", mbSFile.lineno, mbSFile.fname, mbSFile.tiers->speaker);

// mbSFile.tiers
// mbFFile.tiers

}

void call(void) {
	char CT1, CT2;

	if (fpin == stdin) {
		fprintf(stderr, "Input must come from files\n");
		cutt_exit(0);
	}
	if (mbFirstFile) {
		strcpy(mbFFile.fname, oldfname);
		mbFFile.lineno = lineno;
		mbFFile.tlineno = tlineno;
		mbFFile.tiers = NULL;
		strcpy(mbOutFile, newfname);
		mbFirstFile = !mbFirstFile;
		return;
	} else {
#ifndef UNX
		if (WD_Not_Eq_OD)
			SetNewVol(wd_ref, wd_st_full);
#endif
		if ((mbFFile.fpin=fopen(mbFFile.fname, "r")) == NULL) {
			fprintf(stderr, "Can't open file \"%s\".\n", mbFFile.fname);
			cutt_exit(0);
		}
#ifndef UNX
		if (WD_Not_Eq_OD)
			SetNewVol(od_ref, od_st_full);
#endif
		strcpy(mbSFile.fname, oldfname);
		mbSFile.fpin = fpin;
		if (!stout) {
			fclose(fpout);
			unlink(newfname);
			if ((fpout=fopen(mbOutFile, "w")) == NULL) {
				fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", mbOutFile);
				cutt_exit(0);
			}
		}
#if defined(_MAC_CODE)
		settyp(mbOutFile, 'TEXT', the_file_creator.out, FALSE);
#endif
		mbSFile.lineno = lineno;
		mbSFile.tlineno = tlineno;
		mbSFile.tiers = NULL;
	}

	mbFFile.currentatt = 0;
	mbFFile.currentchar = (char)getc_cr(mbFFile.fpin, &mbFFile.currentatt);
	mbSFile.currentatt = 0;
	mbSFile.currentchar = (char)getc_cr(mbSFile.fpin, &mbSFile.currentatt);

	CT1 = CT2 = FALSE;
	do {
		fpin = mbFFile.fpin;
		currentchar = mbFFile.currentchar;
		currentatt = mbFFile.currentatt;
		lineno  = mbFFile.lineno;
		tlineno = mbFFile.tlineno;
		if ((mbFFile.res = getwholeutter())) {
			mbFFile.fpin = fpin;
			mbFFile.currentchar = currentchar;
			mbFFile.currentatt = currentatt;
			mbFFile.lineno = lineno;
			mbFFile.tlineno = tlineno;
			mbFFile.tiers = AddTiers(mbFFile.tiers, utterance->speaker, utterance->attSp, uttline, utterance->attLine);
			CT1 = (currentchar == '*' || currentchar == '@');
		} else
			CT2 = TRUE;

		fpin = mbSFile.fpin;
		currentchar = mbSFile.currentchar;
		currentatt = mbSFile.currentatt;
		lineno  = mbSFile.lineno;
		tlineno = mbSFile.tlineno;
		if ((mbSFile.res = getwholeutter())) {
			mbSFile.fpin = fpin;
			mbSFile.currentchar = currentchar;
			mbSFile.currentatt = currentatt;
			mbSFile.lineno = lineno;
			mbSFile.tlineno = tlineno;
			mbSFile.tiers = AddTiers(mbSFile.tiers, utterance->speaker, utterance->attSp, uttline, utterance->attLine);
			CT2 = (currentchar == '*' || currentchar == '@');
		} else
			CT2 = TRUE;

		if (mbFFile.res && !mbSFile.res) {
			fprintf(stderr, "File \"%s\" ended before file \"%s\" did.\n", 
							mbSFile.fname, mbFFile.fname);
			mbFFile.tiers = FreeUpTiers(mbFFile.tiers);
			mbSFile.tiers = FreeUpTiers(mbSFile.tiers);
			cutt_exit(0);
		}
		if (!mbFFile.res && mbSFile.res) {
			fprintf(stderr, "File \"%s\" ended before file \"%s\" did.\n", 
							mbFFile.fname, mbSFile.fname);
			mbFFile.tiers = FreeUpTiers(mbFFile.tiers);
			mbSFile.tiers = FreeUpTiers(mbSFile.tiers);
			cutt_exit(0);
		}
		if (!mbFFile.res && !mbSFile.res) {
			break;
		}
		if (CT1 != CT2) {
			fprintf(stderr, "Inconsistent number of dependent tiers found:\n");
			fprintf(stderr, "    on line %ld in file \"%s\".\n", 
							mbFFile.lineno, mbFFile.fname);
			fprintf(stderr, "    on line %ld in file \"%s\"\n", 
							mbSFile.lineno, mbSFile.fname);
			mbFFile.tiers = FreeUpTiers(mbFFile.tiers);
			mbSFile.tiers = FreeUpTiers(mbSFile.tiers);
			cutt_exit(0);
		}

		if (mbFFile.currentchar == '*' || mbFFile.currentchar == '@' ||
							mbFFile.currentchar == EOF) {
			CompareTiers();
			mbFFile.tiers = FreeUpTiers(mbFFile.tiers);
			mbSFile.tiers = FreeUpTiers(mbSFile.tiers);
		}
	} while (1) ;
	fclose(mbFFile.fpin);
	fpin = mbSFile.fpin;
	mbFirstFile = !mbFirstFile;
	if (!stout)
		fprintf(stderr, "\t**** Merged data is in a file <%s>.\n", mbOutFile); 
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = TEMP;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	mbFirstFile = TRUE;
	bmain(argc,argv,NULL);
	if (!mbFirstFile) {
		fprintf(stderr, "The second file in pair was missing.\n");
	}
}
