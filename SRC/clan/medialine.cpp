/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
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

struct NamesArr {
	const char *nName;
	const char *oName;
};

static struct NamesArr namesLst[] = {
//           nName             oName
	/* 0*/	{"*CTB:", "*Constantino Teodoro Bautista:"},
	/* 1*/	{"*FEF:", "*Filomeno Encarnación Fidencio:"},
	/* 2*/	{"*EGS:", "*Esteban Guadalupe Sierra:"},
	/* 3*/	{"*ETC:", "*Edmundo Teodoro Celso:"},
	/* 4*/	{"%CTB", "%Constantino Teodoro Bautista"},
	/* 5*/	{"%FEF", "%Filomeno Encarnación Fidencio"},
	/* 6*/	{"%EGS", "%Esteban Castillo Garcia"},
	/* 7*/	{"%ETC", "%Edmundo Teodoro Celso"},
	/* 8*/	{NULL, NULL}
} ;

#define HEADERTIERS struct HeadTiers
HEADERTIERS {
	char *sp;
	AttTYPE *attSp;
	char *line;
	AttTYPE *attLine;
	HEADERTIERS *nextTier;
} ;

static char isAddAudio, isAddVideo, isMissing, isUnlinked, isComment, isTable, isParticipants;
static FILE *tableFp;
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
		isComment = FALSE;
		isParticipants = FALSE;
		isTable = FALSE;
		isAddAudio = FALSE;
		isAddVideo = FALSE;
		isMissing = FALSE;
		isUnlinked = FALSE;
	} else {
		if (isTable)
			stout = TRUE;
		if (chatmode != 0 && (isComment == TRUE || isTable == TRUE || isParticipants == TRUE)) {
			fprintf(stderr,"\n    +c, +p or +t options can only be used with +y option.\n");
			cutt_exit(0);
		}
		if (!isAddAudio && !isAddVideo && !isComment && !isTable && !isParticipants) {
			fprintf(stderr,"\n    Specify either +a, +v, +c, +p or +t option.\n");
			cutt_exit(0);
		}
	}
}

void usage() {
	printf("Adds @Media header with media filename derived from .cha file name.\n");
	printf("Usage: medialine [a v m u %s] filename(s)\n", mainflgs());
	puts("+c: add @Comment: tier with original filename");
	puts("+a: add \"audio\" to @media header");
	puts("+v: add \"video\" to @media header");
	puts("+p: create @Participants: from part of filename");
	puts("+t: create table of new and old filenames");
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
	tableFp = NULL;
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = MEDIALINE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	if (tableFp != NULL)
		fclose(tableFp);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'c':
			isComment = TRUE;
			chatmode = 0;
			break;
		case 'p':
			isParticipants = TRUE;
			chatmode = 0;
			no_arg_option(f);
			break;
		case 't':
			isTable = TRUE;
			stout = TRUE;
			chatmode = 0;
			no_arg_option(f);
			break;
		case 'a':
			isAddAudio = TRUE;
			break;
		case 'v':
			isAddVideo = TRUE;
			no_arg_option(f);
			break;
		case 'm':
			isMissing = TRUE;
			no_arg_option(f);
			break;
		case 'u':
			isUnlinked = TRUE;
			no_arg_option(f);
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

#define SPMAX 10

static void outputParticipants(char *mediafname) {
	int i, j, iSp;
	char *sprs[SPMAX];

	iSp = 0;
	sprs[iSp] = mediafname;
	for (i=0; mediafname[i] != EOS && mediafname[i] != '_'; i++) {
		if (mediafname[i] == '-') {
			mediafname[i] = EOS;
			iSp++;
			sprs[iSp] = mediafname+i+1;
		}
	}
	mediafname[i] = EOS;
	strcpy(spareTier1, "@Participants:\t");
	for (i=0; i <= iSp; i++) {
		if (i > 0)
			strcat(spareTier1, ", ");
		for (j=0; sprs[i][j] != EOS && !isdigit(sprs[i][j]); j++) ;
		sprs[i][j] = EOS;
		strcat(spareTier1, sprs[i]);
		strcat(spareTier1, " Participant");
	}
	fprintf(fpout, "%s\n", spareTier1);
	for (i=0; i <= iSp; i++) {
		strcpy(spareTier1, "@ID:\teng|change_me_later|");
		sprs[i][j] = EOS;
		strcat(spareTier1, sprs[i]);
		strcat(spareTier1, "|||||Participant|||");
		fprintf(fpout, "%s\n", spareTier1);
	}
}

void call() {
	int  i, j;
	char *s, mediafname[BUFSIZ], oldDataFname[BUFSIZ], isContinue, isFoundComment, isPartID;
	FNType mDirPathName[FNSize];

	if (isTable == TRUE) {
		strcpy(mediafname, oldfname);
		strcpy(mediafname, mediafname+strlen(wd_dir)+1);
	} else {
		extractFileName(mediafname, oldfname);
		if (isComment == FALSE) {
			s = strchr(mediafname, '.');
			if (s != NULL)
				*s = '\0';
		}
	}
	isPartID = 0;
	isFoundComment = FALSE;
	isContinue = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (chatmode == 0) {
			if (isTable == TRUE) {
				if (uS.partcmp(utterance->line, "@Comment:\toriginal name was: ", FALSE, FALSE)) {
					isFoundComment = TRUE;
					strcpy(oldDataFname, utterance->line+29);
					uS.remblanks(oldDataFname);
				}
				if (isFoundComment) {
					if (tableFp == NULL) {
						strcpy(mDirPathName, wd_dir);
						addFilename2Path(mDirPathName, "0table.txt");
						tableFp = fopen(mDirPathName, "w");
						if (tableFp == NULL) {
							fprintf(stderr, "\nCan't created table file: %s\n", mDirPathName);
							cutt_exit(0);
						}
						fprintf(stderr, "\nCreated table file: %s\n", mDirPathName);
					}
					fprintf(tableFp, "%s\t%s\n", mediafname, oldDataFname);
					tiersRoot = freeTiers(tiersRoot);
					return;
				}
			} else if (isParticipants == TRUE) {
				if (uS.partcmp(utterance->line, "@Participants:", FALSE, FALSE) ||
					uS.partcmp(utterance->line, "@ID:", FALSE, FALSE) ||
					(isPartID == TRUE && utterance->line[0] == '\t')) {
					isPartID = 1;
				} else if (isPartID == 1) {
					outputParticipants(mediafname);
					isPartID = 0;
				}
				if (isPartID == FALSE) {
					for (j=0; namesLst[j].nName != NULL; j++) {
						if (uS.partcmp(utterance->line, namesLst[j].oName, FALSE, FALSE)) {
							strcpy(utterance->line, utterance->line+strlen(namesLst[j].oName)-strlen(namesLst[j].nName));
							for (i=0; namesLst[j].nName[i] != EOS; i++)
								utterance->line[i] = namesLst[j].nName[i];
							break;
						}
					}
					fprintf(fpout, "%s", utterance->line);
				}
			} else {
				if (utterance->line[0] == '*' && isContinue == FALSE) {
					if (isComment == TRUE) {
						fprintf(fpout, "@Comment:\toriginal name was: %s\n", mediafname);
					} else {
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
					}
					isContinue = TRUE;
				}
				if (isComment == TRUE && uS.partcmp(utterance->line, "@Comment:\toriginal name was: ", FALSE, FALSE)) {
				} else if (isComment == FALSE && uS.partcmp(utterance->line, MEDIAHEADER, FALSE, FALSE)) {
				} else
					fprintf(fpout, "%s", utterance->line);
			}
		} else {
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
	}
	if (chatmode != 0) {
		if (mediafname[0] != EOS) {
			fprintf(fpout, "@Media:\t%s, audio\n", mediafname);
			mediafname[0] = EOS;
		}
		if (isContinue) {
			while (getwholeutter()) {
				printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
			}
		}
	}
	tiersRoot = freeTiers(tiersRoot);
}
