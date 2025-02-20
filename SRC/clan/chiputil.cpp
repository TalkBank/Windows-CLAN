/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "ced.h"
#include "cu.h"
#ifdef _MAC_CODE 
#import "AVController.h"
#elif _WIN32 
#endif

#if !defined(UNX)
#define _main chiputil_main
#define call chiputil_call
#define getflag chiputil_getflag
#define init chiputil_init
#define usage chiputil_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 1

extern struct tier *defheadtier;
extern char OverWriteFile;

enum {
	allTypes = 0,
	notSource,
	thisIsSource,
	linkToSource,
} ;

#define CHIPU_UTTS struct chip_data /* main utterance structure				*/
CHIPU_UTTS {
	char *speaker;		/* code descriptor field of the turn	*/
	char *line;		/* text field of the turn		*/ // found uttlinelen
	long ID;
	char isSource;
	char uttType;
	CHIPU_UTTS *deputts;
	CHIPU_UTTS *prevutt, *nextutt;		/* pointer to the next utterance, if	*/
} ;				/* there is only 1 utterance, i.e. no	*/


static char track_tier[32], outputType;
static long IDnum;
static CHIPU_UTTS *root_utt;

static CHIPU_UTTS *chiputil_free_utt(CHIPU_UTTS *p) {
	CHIPU_UTTS *t;

	while (p != NULL) {
		t = p;
		p = p->nextutt;
		chiputil_free_utt(t->deputts);
		if (t->speaker)
			free(t->speaker);
		if (t->line)
			free(t->line);
		free(t);
	}
	return(NULL);
}

static CHIPU_UTTS *add_speaker(CHIPU_UTTS *sp_utt, char *sp, char *line) {
	CHIPU_UTTS *prev_utt;

	if (root_utt == NULL) {
		if ((root_utt=NEW(CHIPU_UTTS)) == NULL)
			out_of_mem();
		prev_utt = NULL;
		sp_utt = root_utt;
	} else {
		if ((sp_utt->nextutt=NEW(CHIPU_UTTS)) == NULL) {
			root_utt = chiputil_free_utt(root_utt);
			out_of_mem();
		}
		prev_utt = sp_utt;
		sp_utt = sp_utt->nextutt;
	}
	sp_utt->prevutt = prev_utt;
	sp_utt->nextutt = NULL;
	sp_utt->deputts = NULL;
	sp_utt->ID = 0L;
	sp_utt->speaker = NULL;
	sp_utt->line = NULL;
	sp_utt->uttType = notSource;
	sp_utt->isSource = notSource;
	if ((sp_utt->speaker=(char *)malloc(strlen(sp)+1)) == NULL) {
		root_utt = chiputil_free_utt(root_utt);
		out_of_mem();
	}
	strcpy(sp_utt->speaker, sp);
	if ((sp_utt->line=(char *)malloc(strlen(line)+1)) == NULL) {
		root_utt = chiputil_free_utt(root_utt);
		out_of_mem();
	}
	strcpy(sp_utt->line, line);
	return(sp_utt);
}

static void add_deptier(CHIPU_UTTS *sp_utt, char *sp, char *line) {
	CHIPU_UTTS *deputt;

	if (sp_utt->deputts == NULL) {
		if ((sp_utt->deputts=NEW(CHIPU_UTTS)) == NULL) {
			root_utt = chiputil_free_utt(root_utt);
			out_of_mem();
		}
		deputt = sp_utt->deputts;
	} else {
		for (deputt=sp_utt->deputts; deputt->nextutt != NULL; deputt = deputt->nextutt) ;
		if ((deputt->nextutt=NEW(CHIPU_UTTS)) == NULL) {
			root_utt = chiputil_free_utt(root_utt);
			out_of_mem();
		}
		deputt = deputt->nextutt;
	}
	deputt->prevutt = NULL;
	deputt->nextutt = NULL;
	deputt->deputts = NULL;
	deputt->ID = 0L;
	deputt->speaker = NULL;
	deputt->line = NULL;
	if ((deputt->speaker=(char *)malloc(strlen(sp)+1)) == NULL) {
		root_utt = chiputil_free_utt(root_utt);
		out_of_mem();
	}
	strcpy(deputt->speaker, sp);
	if ((deputt->line=(char *)malloc(strlen(line)+1)) == NULL) {
		root_utt = chiputil_free_utt(root_utt);
		out_of_mem();
	}
	strcpy(deputt->line, line);
}


void usage() {
	puts("CHIPUTIL finds either source or non source utterances for either ADU or CHI");
	printf("Usage: chiputil [aS oS s %s] filename(s)\n", mainflgs());
	printf("+aS: track S tier for imitation $DIST code (default: %s)\n", track_tier);
	printf("+oS: include tier code S in output purposes ONLY\n");
	printf("-oS: exclude tier code S from output purposes ONLY\n");
	printf("+s : output only source utterances (default: output all utterances)\n");
	printf("-s : output only non-source utterances (default: output all utterances)\n");
	mainusage(FALSE);
	puts("Example: (first run CHIP command with \"+f +d\" options to create *.chip.cex files)");
	puts("       chiputil *.chip.cex");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		outputType = allTypes;
		root_utt = NULL;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
	}
}

static void findSource(CHIPU_UTTS *sp_utt, char *line) {
	int i, j, dist;
	char code[BUFSIZ];
	CHIPU_UTTS *utt;

	i = 0;
	while ((i=getword("%", line, code, &j, i))) {
		if (uS.mStricmp(code,"$DIST") == 0) {
			for (; !isSpace(line[j]) && line[j] != EOS; j++) ;
			for (; isSpace(line[j]) || line[j] == '='; j++) ;
			if (isdigit(line[j])) {
				IDnum++;
				dist = atoi(line+j);
				sp_utt->isSource = thisIsSource;
				sp_utt->uttType = linkToSource;
				sp_utt->ID = IDnum;
				for (utt=sp_utt->prevutt; utt != NULL; utt=utt->prevutt) {
					if (utt->speaker[0] == '*')
						dist--;
					if (dist == 0) {
						utt->isSource = thisIsSource;
						utt->uttType = thisIsSource;
						utt->ID = IDnum;
						break;
					}
				}
			}
			break;
		}
	}
}

static void printOutput(CHIPU_UTTS *t) {
	char st[32];

	while (t != NULL) {
		if (t->speaker[0] != '*' || outputType == allTypes || outputType == t->isSource) {
			if (checktier(t->speaker)) {
				printout(t->speaker, t->line, NULL, NULL, FALSE);
				if (t->speaker[0] == '*') {
					if (t->uttType == notSource) {
						sprintf(st, "$NON");
						printout("%chU:", st, NULL, NULL, FALSE);
					} else if (t->uttType == thisIsSource) {
						sprintf(st, "$SOURCE=%ld", t->ID);
						printout("%chU:", st, NULL, NULL, FALSE);
					} else if (t->uttType == linkToSource) {
						sprintf(st, "$IMITATION=%ld", t->ID);
						printout("%chU:", st, NULL, NULL, FALSE);
					}
				}
				printOutput(t->deputts);
			}
		}
		t = t->nextutt;
	}
}

void call() {
	CHIPU_UTTS *main_utt, *sp_utt;

	IDnum = 0L;
	root_utt = NULL;
	sp_utt = NULL;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '@') {
			main_utt = add_speaker(main_utt, utterance->speaker, utterance->line);
		} else if (utterance->speaker[0] == '*') {
			main_utt = add_speaker(main_utt, utterance->speaker, utterance->line);
			sp_utt = main_utt;
		} else if (utterance->speaker[0] == '%') {
			if (sp_utt == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno+tlineno);
				fprintf(stderr, "Found dependent tier that is not associate with any speaker:");
				fprintf(stderr, "    %s%s", utterance->speaker, utterance->line);
				root_utt = chiputil_free_utt(root_utt);
				return;	
			}
			add_deptier(sp_utt, utterance->speaker, utterance->line);
			if (uS.partcmp(utterance->speaker,track_tier,FALSE,FALSE)) {
				findSource(sp_utt, utterance->line);
			}
		}
	}
	printOutput(root_utt);
	root_utt = chiputil_free_utt(root_utt);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CHIPUTIL;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	strcpy(track_tier, "%chi:");
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'a':
			if (*f == EOS) {
				fprintf(stderr, "Please specify dependent tier for tracking imitations");
				cutt_exit(0);
			}
			strcpy(track_tier, f);
			break;
		case 's':
			if (*(f-2) == '+') {
				outputType = thisIsSource;
			} else {
				outputType = notSource;
			}
			no_arg_option(f);
			break;
		case 'o':
			maketierchoice(getfarg(f,f1,i), *(f-2), FALSE);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
