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
#define CHAT_MODE 0

struct fwmWords {
	char *orig;
	char *clean;
	struct fwmWords *nextword;
} ;

extern struct tier *defheadtier;
extern char OverWriteFile;

static fwmWords *fwm_wdptr;

static struct fwmWords *freefwmWords(struct fwmWords *ptr) {
	struct fwmWords *t;

	while (ptr != NULL) {
		t = ptr;
		ptr = ptr->nextword;
		if (t->orig)
			free(t->orig);
		if (t->clean)
			free(t->clean);
		free(t);
	}
	return(ptr);
}

static void modrepError(void) {
	fwm_wdptr = freefwmWords(fwm_wdptr);
	cutt_exit(0);
}

static struct fwmWords *fwm_rdexclf(FNType *fname) {
	int i;
	char orig[512];
	struct fwmWords *twd, *wdptr;
	FILE *efp;

#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(wd_dir);
#endif
	if ((efp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "Can not access file: %s\n", fname);
		modrepError();
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
#endif
	wdptr = NULL;
	while (fgets_cr(orig, 511, efp)) {
		if (uS.isUTF8(orig) || uS.isInvisibleHeader(orig))
			continue;
		if (orig[0] == '#' && orig[1] == ' ')
			continue;
		if (orig[0] == '#' && orig[1] == ' ')
			continue;

		uS.remblanks(orig);
		for (i=0; isdigit(orig[i]) || isSpace(orig[i]); i++) ;
		strcpy(orig, orig+i);

		if (wdptr == NULL) {
			twd = NEW(struct fwmWords);
			if (twd == NULL) {
				fprintf(stderr,"Memory ERROR. Use less words.\n");
				cutt_exit(1);
			}
			wdptr = twd;
		} else {
			for (twd=wdptr; twd->nextword != NULL; twd=twd->nextword) ;
			twd->nextword = NEW(struct fwmWords);
			if (twd->nextword == NULL) {
				fprintf(stderr,"Memory ERROR. Use less words.\n");
				cutt_exit(1);
			}
			twd = twd->nextword;
		}
		twd->orig = (char *)malloc((strlen(orig)+1)*sizeof(char));
		if (twd->orig == NULL) {
			fprintf(stderr,"Memory ERROR. Use less words.\n");
			cutt_exit(1);
		}
		strcpy(twd->orig, orig);
		i = 0;
		while (orig[i] != EOS) {
			if (orig[i] == '(' || orig[i] == ')' || orig[i] == '^')
				strcpy(orig+i, orig+i+1);
			else
				i++;
		}
		twd->clean = (char *)malloc((strlen(orig)+2)*sizeof(char));
		if (twd->clean == NULL) {
			fprintf(stderr,"Memory ERROR. Use less words.\n");
			cutt_exit(1);
		}
		strcpy(twd->clean, orig);
		strcat(twd->clean, "\"");
		twd->nextword = NULL;
	}
	return(wdptr);
}

void usage() {
	puts("TEMP output all first word matched as found");
	printf("Usage: temp [%s] filename(s)\n",mainflgs());
	printf("+oF: specify file name @F with words to search for\n");
	mainusage(TRUE);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	fwm_wdptr = freefwmWords(fwm_wdptr);
}
		
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'o':
			if (*f) {
				if (*f == '@') {
					f++;
					fwm_wdptr = fwm_rdexclf(f);
				} else {
					fprintf(stderr,"@ with file name is expected after %s option.\n", f-2);
				}
			} else {
				fprintf(stderr,"String expected after %s option.\n", f-2);
				modrepError();
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

void init(char f) {
	if (f) {
		fwm_wdptr = NULL;
		onlydata = 1;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	}
}

void call() {
	int len, offset;
	struct fwmWords *twd;

	while (fgets_cr(uttline, UTTLINELEN, fpin) != NULL) {
		uS.remblanks(uttline);
		for (twd=fwm_wdptr; twd != NULL; twd=twd->nextword) {
			len = strlen(twd->clean);
			if (uttline[0] == '"')
				offset = 1;
			else
				offset = 0;
			if (strncmp(uttline+offset, twd->clean, len) == 0)
				fprintf(fpout, "\"%s\"\t%s\n", twd->orig, uttline);			
		}
	}
}
