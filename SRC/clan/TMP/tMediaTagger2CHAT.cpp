/**********************************************************************
	"Copyright 2010 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "ced.h"
#include "cu.h"
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
#define CHAT_MODE 0

#define SPKRTIER 1
#define DEPTTIER 2
#define HEADTIER 3

#define TAGSLEN  128

struct TagsRec {
	char whichTier;
	char tag[TAGSLEN];
	char depOn[TAGSLEN];
	char chatName[TAGSLEN];
	struct TagsRec *nextTag;
} ;
typedef struct TagsRec TAGS;

struct linesRec {
	char whichTier;
	char depOn[TAGSLEN];
	char chatName[TAGSLEN];
	char *line;
	long bt, et;
	struct linesRec *nextLine;
} ;
typedef struct linesRec LINESLIST;

static TAGS *tiersRoot, *excludedTags;
static LINESLIST *linesRoot, *dLinesRoot;
static long ln;
static double framesMult;
static double timeOffset;
static char timeOffsetGiven;
static char *MFN;

extern struct tier *defheadtier;
extern char AddCEXExtension;
extern char OverWriteFile;
extern long option_flags[];

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	extern char GExt[];

	if (f) {
		framesMult = 0.0;
		timeOffsetGiven = FALSE;
		excludedTags = NULL;
		tiersRoot = NULL;
		linesRoot = NULL;
		dLinesRoot = NULL;
		MFN = NULL;
		OverWriteFile = TRUE;
		AddCEXExtension = FALSE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		strcpy(GExt, ".cha");
		option_flags[CLAN_PROG_NUM] = 0L;
		if (framesMult == 0.0) {
		    fprintf(stderr,"Please specify frames number with \"+f\" option.\n");
		    cutt_exit(0);
		}
		if (!timeOffsetGiven) {
		    fprintf(stderr,"Please specify movie start time offset number with \"+t\" option.\n");
		    cutt_exit(0);
		}
	}
}

void usage() {
	printf("Converts MediaTagger text files to CHAT text files\n");
	printf("Usage: temp [+dF +fN +mF +tN] filename(s)\n");
    puts("+dF: specify tags dependencies file F.");
    puts("+fN: look up frames/second (FPS:) number N in a movie's \"Movie Info\" window.");
    puts("+mF: specify movie file name F (default: dummy).");
    puts("+tN: specify movie segment start time offset relative to actual movie start.");
	puts("Example: temp +mmoviefilename +f25 +dfilename.cut +t42:06 *.cha");
	puts("         temp +f25 +dattribs.cut +t42:06 yucatec.txt");
}

static TAGS *freeTiers(TAGS *p) {
	TAGS *t;

	while (p != NULL) {
		t = p;
		p = p->nextTag;
		free(t);
	}
	return(NULL);
}

static LINESLIST *freeLines(LINESLIST *p) {
	LINESLIST *t;

	while (p != NULL) {
		t = p;
		p = p->nextLine;
		if (t->line != NULL)
			free(t->line);
		free(t);
	}
	return(NULL);
}

static void freeMem(void) {
	linesRoot = freeLines(linesRoot);
	dLinesRoot = freeLines(dLinesRoot);
	tiersRoot = freeTiers(tiersRoot);
	excludedTags = freeTiers(excludedTags);
}

static TAGS *add_each_tag(TAGS *root, char *fname, char whichTier, char *tag, char *depOn, char *chatName) {
	TAGS *p;

	if (tag == NULL)
		return(root);

	if (tag[0] == EOS)
		return(root);

	if (root == NULL) {
		if ((p=NEW(TAGS)) == NULL)
			out_of_mem();
		root = p;
	} else {
		for (p=root; p->nextTag != NULL && uS.mStricmp(p->tag, tag); p=p->nextTag) ;
		if (uS.mStricmp(p->tag, tag) == 0)
			return(root);

		if ((p->nextTag=NEW(TAGS)) == NULL)
			out_of_mem();
		p = p->nextTag;
	}

	p->nextTag = NULL;
	if ((strlen(tag) >= TAGSLEN) || (strlen(depOn) >= TAGSLEN) || (strlen(chatName) >= TAGSLEN)) {
		freeMem();
		fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
		fprintf(stderr, "Tag(s) too long. Longer than %d characters\n", TAGSLEN);
		cutt_exit(0);
	}
	p->whichTier = whichTier;
	strcpy(p->tag, tag);
	strcpy(p->depOn, depOn);
	if (chatName[0] != '*' && chatName[0] != '%' && chatName[0] != '@') {
		strcpy(p->chatName, "%");
		strcat(p->chatName, chatName);
	} else
		strcpy(p->chatName, chatName);
	strcat(p->chatName, ":");
	return(root);
}

static void rd_dep_f(char *fname) {
	FILE *fp;
	char *tag, *depOn, *chatName;
	long i, j;

	if (*fname == EOS) {
		fprintf(stderr,	"No dep. tags file specified.\n");
		cutt_exit(0);
	}

	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,NULL)) == NULL) {
		fprintf(stderr, "Can't open either one of dep. tags files:\n\t\"%s\", \"%s%s\"\n", fname, templineC, fname);
		cutt_exit(0);
	}
	ln = 0L;
	while (fgets_cr(templineC, 255, fp)) {
		ln++;
		if (uS.isUTF8(templineC))
			continue;
		if (templineC[0] == ';')
			continue;

		uS.remblanks(templineC);
		if (templineC[0] == EOS)
			continue;

		if (templineC[0] == '@') {
			for (i=0; isSpace(templineC[i]); i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			if (templineC[j] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing header name\n");
				cutt_exit(0);
			}
			templineC[j] = EOS;
			tag = templineC+i;
			for (i=j+1; isSpace(templineC[i]); i++) ;
			if (templineC[i] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			templineC[j] = EOS;
			depOn = templineC+i;
			tiersRoot = add_each_tag(tiersRoot, fname, HEADTIER, depOn, "", tag);
		} else if (templineC[0] == '*') {
			for (i=0; isSpace(templineC[i]); i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			if (templineC[j] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing speaker name\n");
				cutt_exit(0);
			}
			templineC[j] = EOS;
			tag = templineC+i;
			for (i=j+1; isSpace(templineC[i]); i++) ;
			if (templineC[i] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			templineC[j] = EOS;
			depOn = templineC+i;
			tiersRoot = add_each_tag(tiersRoot, fname, SPKRTIER, depOn, depOn, tag);
		} else if (templineC[0] == '-') {
			for (i=0; isSpace(templineC[i]) || templineC[i] == '-'; i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != '\n' && templineC[j] != EOS; j++) ;
			templineC[j] = EOS;
			tag = templineC+i;
			excludedTags = add_each_tag(excludedTags, fname, DEPTTIER, tag, tag, tag);
		} else {
			for (i=0; isSpace(templineC[i]); i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			if (templineC[j] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			templineC[j] = EOS;
			tag = templineC+i;
			for (i=j+1; isSpace(templineC[i]); i++) ;
			for (j=i; !isSpace(templineC[j]) && templineC[j] != EOS; j++) ;
			if (templineC[j] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			templineC[j] = EOS;
			depOn = templineC+i;
			for (i=j+1; isSpace(templineC[i]); i++) ;
			if (templineC[i] == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
				fprintf(stderr, "Missing info\n");
				cutt_exit(0);
			}
			for (j=i; !isSpace(templineC[j]) && templineC[j] != '\n' && templineC[j] != EOS; j++) ;
			templineC[j] = EOS;
			chatName = templineC+i;
			tiersRoot = add_each_tag(tiersRoot, fname, DEPTTIER, tag, depOn, chatName);
		}
	}
	fclose(fp);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;

	bmain(argc,argv,NULL);

	linesRoot = freeLines(linesRoot);
	dLinesRoot = freeLines(dLinesRoot);
	tiersRoot = freeTiers(tiersRoot);
	excludedTags = freeTiers(excludedTags);
}

static double calculateOffset(char *s) {
	char *col, *num, count;
	double FPS;
	double time;

	col = strrchr(s,';');
	if (col != NULL)
		*col = ':';
	FPS = 1 / (framesMult / 1000.0);
	count = 0;
	time = 0.0;
	do {
		col = strrchr(s,':');
		if (col != NULL) {
			*col = EOS;
			num = col + 1;
		} else 
			num = s;

		if (count == 0) {
			time = ((double)atof(num) / FPS) * 1000.0;
		} else if (count == 1) {
			time = time + ((double)atof(num) * 1000.0);
		} else if (count == 2) {
			time = time + ((double)atof(num) * 60.0 * 1000.0);
		} else if (count == 3) {
			time = time + ((double)atof(num) * 3600.0 * 1000.0);
		}
		count++;
	} while (col != NULL && count < 4) ;
	return(time);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'd':
			if (*f) {
				rd_dep_f(getfarg(f,f1,i));
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'f':
			framesMult = (double)atof(getfarg(f,f1,i));
			if (framesMult < 20.0 || framesMult > 30.0) {
				fprintf(stderr,"+s number is usually between 25 and 30.\n");
				cutt_exit(0);
			}
			framesMult = (1 / framesMult) * 1000.0;
			break;
		case 'm':
			MFN = f;
			break;
		case 's':
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
		case 't':
			if (framesMult == 0.0) {
			    fprintf(stderr,"Please specify frames number with \"+f\" option first.\n");
			    cutt_exit(0);
			}
			if (*f) {
				strcpy(templineC1, getfarg(f,f1,i));
				timeOffset = calculateOffset(templineC1);
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			timeOffsetGiven = TRUE;
			break;
		default:
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
	}
}

static char whatKindTag(char *tag, char *depOn, char *chatName) {
	TAGS *p;

	for (p=tiersRoot; p != NULL; p=p->nextTag) {
		if (tag == NULL) {
			if (p->whichTier == SPKRTIER)
				return(SPKRTIER);
		} else if (uS.mStricmp(p->tag, tag) == 0) {
			if (chatName != NULL)
				strcpy(chatName, p->chatName);
			if (depOn != NULL)
				strcpy(depOn, p->depOn);
			return(p->whichTier);
		}
	}
	return(0);
}

static LINESLIST *insertEmptySpeaker(LINESLIST *root, LINESLIST **ont, LINESLIST *tnt, char *depOn, long bt, long et) {
	char chatName[TAGSLEN];
	LINESLIST *nt = *ont;

	if (nt == NULL) {
		tnt->nextLine = NEW(LINESLIST);
		if (tnt->nextLine == NULL)
			out_of_mem();
		nt = tnt->nextLine;
		nt->nextLine = NULL;
	} else if (nt == root) {
		root = NEW(LINESLIST);
		if (root == NULL)
			out_of_mem();
		root->nextLine = nt;
		nt = root;
	} else {
		nt = NEW(LINESLIST);
		if (nt == NULL)
			out_of_mem();
		nt->nextLine = tnt->nextLine;
		tnt->nextLine = nt;
	}

	whatKindTag(depOn, NULL, chatName);
	sprintf(templineC2, " %c%s\"%s\"_%ld_%ld%c\n\t", HIDEN_C, SOUNDTIER, MFN, bt, et, HIDEN_C);

	nt->whichTier = SPKRTIER;
	strcpy(nt->depOn, depOn);
	strcpy(nt->chatName, chatName);
	nt->line = (char *)malloc(strlen("0.")+strlen(templineC2)+1);
	if (nt->line == NULL) {
		freeMem();
		out_of_mem();
	}
	strcpy(nt->line, "0.");
	strcat(nt->line, templineC2);

	nt->bt = bt;
	nt->et = et;

	*ont = nt;
	return(root);
}

static LINESLIST *add_each_line(LINESLIST *root, char isMerge, char whichTier, char *depOn, char *chatName, char *line, long bt, long et) {
	long len;
	LINESLIST *nt, *tnt;

	if (root == NULL) {
		if ((root=NEW(LINESLIST)) == NULL)
			out_of_mem();
		nt = root;
		nt->nextLine = NULL;
	} else {
		tnt= root;
		nt = root;
		if (whichTier == HEADTIER) {
			while (nt != NULL) {
				if (nt->whichTier != HEADTIER)
					break;
				tnt = nt;
				nt = nt->nextLine;
			}
		} else if (whichTier == SPKRTIER) {
			while (nt != NULL) {
				if (nt->whichTier == SPKRTIER && bt < nt->bt)
					break;
				tnt = nt;
				nt = nt->nextLine;
			}
		} else if (whichTier == DEPTTIER) {
			if (!isMerge) {
				while (nt != NULL) {
					if (bt < nt->bt)
						break;
					tnt = nt;
					nt = nt->nextLine;
				}
			} else {
				while (nt != NULL) {
					if (nt->whichTier == SPKRTIER && !uS.mStricmp(nt->depOn,depOn) && bt >= nt->bt && bt < nt->et) {
						tnt = nt;
						nt = nt->nextLine;
						break;
					} else if (nt->whichTier == SPKRTIER && bt < nt->bt) {
						root = insertEmptySpeaker(root, &nt, tnt, depOn, bt, et);
						tnt = nt;
						nt = nt->nextLine;
						break;
					}
					tnt = nt;
					nt = nt->nextLine;
				}
			}
		} else {
			return(root);
		}

		if (nt == NULL) {
			tnt->nextLine = NEW(LINESLIST);
			if (tnt->nextLine == NULL)
				out_of_mem();
			nt = tnt->nextLine;
			nt->nextLine = NULL;
		} else if (nt == root) {
			root = NEW(LINESLIST);
			if (root == NULL)
				out_of_mem();
			root->nextLine = nt;
			nt = root;
		} else {
			nt = NEW(LINESLIST);
			if (nt == NULL)
				out_of_mem();
			nt->nextLine = tnt->nextLine;
			tnt->nextLine = nt;
		}
	}
	nt->line = NULL;
	if (strlen(chatName) >= TAGSLEN || strlen(depOn) >= TAGSLEN) {
		freeMem();
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
		fprintf(stderr, "Tag is too long > %d\n", TAGSLEN);
		cutt_exit(0);
	}
	if (whichTier == SPKRTIER) {
		for (len=strlen(line); len >= 0; len--) {
			if (uS.IsUtteranceDel(line, len))
				break;
		}
		if (len < 0)
			sprintf(templineC2, " +. %c%s\"%s\"_%ld_%ld%c\n\t", HIDEN_C, SOUNDTIER, MFN, bt, et, HIDEN_C);
		else
			sprintf(templineC2, " %c%s\"%s\"_%ld_%ld%c\n\t", HIDEN_C, SOUNDTIER, MFN, bt, et, HIDEN_C);
	} else
		templineC2[0] = EOS;

	nt->whichTier = whichTier;
	strcpy(nt->depOn, depOn);
	strcpy(nt->chatName, chatName);
	if (!isMerge) {
		nt->line = (char *)malloc(strlen(line)+strlen(templineC2)+1);
		if (nt->line == NULL) {
			freeMem();
			out_of_mem();
		}
		strcpy(nt->line, line);
		strcat(nt->line, templineC2);
	} else
		nt->line = line;
	nt->bt = bt;
	nt->et = et;
	return(root);
}

static void addDepsToSpeaker(void) {
	LINESLIST *t;

	while (dLinesRoot != NULL) {
		t = dLinesRoot;
		linesRoot = add_each_line(linesRoot, TRUE, t->whichTier, t->depOn, t->chatName, t->line, t->bt, t->et);	
		dLinesRoot = dLinesRoot->nextLine;
		free(t);
	}
}

static char excludeTag(char *tag) {
	TAGS *t;

	for (t=excludedTags; t != NULL; t=t->nextTag) {
		if (!uS.mStricmp(t->tag, tag))
			return(TRUE);
	}
	return(FALSE);
}

static void cleanupLine(char *st) {
	int i;

	for (i=0; st[i] != EOS;) {
		if ((st[i] >= 0 && st[i] < 32) || st[i] == 0x7f)
			strcpy(st+i, st+i+1);
		else
			i++;
	}
}

static void outputTiers(LINESLIST *p) {
	for (; p != NULL; p=p->nextLine) {
		uS.remblanks(p->line);
		if (p->line[0] != EOS)
			printout(p->chatName, p->line, NULL, NULL, TRUE);
	}
}

static void output_Participants_ID(void) {
	int  len, i;
	TAGS *p;

	templineC[0] = EOS;
	for (p=tiersRoot; p != NULL; p=p->nextTag) {
		if (p->whichTier == SPKRTIER) {
			len = strlen(templineC);
			if (len > 0) {
				strcat(templineC, ", ");
				len = strlen(templineC);
			}
			strcpy(templineC2, p->chatName+1);
			for (i=strlen(templineC2)-1; i >= 0 && (isSpace(templineC2[i]) || templineC2[i] == ':'); i--) ;
			templineC2[++i] = EOS;
			sprintf(templineC+len, "%s Child", templineC2);
		}
	}
	if (templineC[0] != EOS)
		printout("@Participants:", templineC, NULL, NULL, TRUE);
	
	for (p=tiersRoot; p != NULL; p=p->nextTag) {
		if (p->whichTier == SPKRTIER) {
			strcpy(templineC2, p->chatName+1);
			for (i=strlen(templineC2)-1; i >= 0 && (isSpace(templineC2[i]) || templineC2[i] == ':'); i--) ;
			templineC2[++i] = EOS;
			fprintf(fpout, "@ID:	en|sample|%s|||||Child||\n", templineC2);
		}
	}
}

void call() {
	char isMFNNUll;
	char depOn[TAGSLEN];
	char chatName[TAGSLEN];
	char *btS, *etS, *tag, *line, whichTier, isSpeakerFound;
	long bt, et;
//	double t;

	if (whatKindTag(NULL, NULL, NULL) != SPKRTIER) {
	    fprintf(stderr,"Please specify the name of main speaker(s) in attribs.cut file.\n");
	    cutt_exit(0);
	}

	if (MFN == NULL || *MFN == EOS)
		isMFNNUll = TRUE;

	if (isMFNNUll) {
		strcpy(FileNameC2, oldfname);
		btS = strrchr(FileNameC2, '.');
		if (btS != NULL)
			*btS = EOS;
		MFN = FileNameC2;
	}

	ln = 0L;
	isSpeakerFound = FALSE;
    while (fgets_cr(templineC, UTTLINELEN, fpin)) {
    	ln++;
		if (uS.isUTF8(templineC))
			continue;
		uS.remblanks(templineC);
		cleanupLine(templineC);

/*
		for (btS=templineC; isSpace(*btS); btS++) ;
		for (; !isSpace(*btS) && *btS != EOS; btS++) ;
		for (; isSpace(*btS); btS++) ;
		if (*btS == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "File is corrupted\n");
			cutt_exit(0);
		}
		for (etS=btS; !isSpace(*etS) && *etS != EOS; etS++) ;
		if (isSpace(*etS)) {
			*etS = EOS;
			etS++;
		}
		for (; isSpace(*etS); etS++) ;
		for (; !isSpace(*etS) && *etS != EOS; etS++) ;
		for (; isSpace(*etS); etS++) ;

		if (*etS == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "File is corrupted\n");
			cutt_exit(0);
		}
		for (tag=etS; !isSpace(*tag) && *tag != EOS; tag++) ;
		if (isSpace(*tag)) {
			*tag = EOS;
			tag++;
		}
		for (; isSpace(*tag); tag++) ;

		if (*tag == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "File is corrupted\n");
			cutt_exit(0);
		}
		for (line=tag; !isSpace(*line) && *line != EOS; line++) ;
		if (isSpace(*line)) {
			*line = EOS;
			line++;
		}
		for (; isSpace(*line); line++) ;

		t = (double)atol(btS);
		bt = (long)(t * framesMult) - timeOffset;
		t = (double)atol(etS);
		et = (long)(t * framesMult) - timeOffset;
*/
		for (btS=templineC; isSpace(*btS); btS++) ;
		if (*btS == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "File is corrupted\n");
			cutt_exit(0);
		}
		for (etS=btS; !isSpace(*etS) && *etS != EOS; etS++) ;
		if (isSpace(*etS)) {
			*etS = EOS;
			etS++;
		}
		for (; isSpace(*etS); etS++) ;
		for (; !isSpace(*etS) && *etS != EOS; etS++) ;
		for (; isSpace(*etS); etS++) ;

		if (*etS == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "File is corrupted\n");
			cutt_exit(0);
		}

		for (tag=etS; !isSpace(*tag) && *tag != EOS; tag++) ;
		if (isSpace(*tag)) {
			*tag = EOS;
			tag++;
		}
		for (; isSpace(*tag); tag++) ;
		for (; !isSpace(*tag) && *tag != EOS; tag++) ;
		for (; isSpace(*tag); tag++) ;

		if (*tag == EOS) {
			freeMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr, "File is corrupted\n");
			cutt_exit(0);
		}
		for (line=tag; !isSpace(*line) && *line != EOS; line++) ;
		if (isSpace(*line)) {
			*line = EOS;
			line++;
		}
		for (; isSpace(*line); line++) ;

		bt = (long)(calculateOffset(btS) - timeOffset);
		et = (long)(calculateOffset(etS) - timeOffset);

		if (!excludeTag(tag)) {
			whichTier = whatKindTag(tag, depOn, chatName);

			if (whichTier == HEADTIER) {
				strcpy(templineC1, tag);
				strcat(templineC1, ": ");
				strcat(templineC1, line);
				linesRoot = add_each_line(linesRoot, FALSE, whichTier, depOn, chatName, templineC1, 0L, 0L);
			} else if (whichTier == SPKRTIER) {
				isSpeakerFound = TRUE;
				linesRoot = add_each_line(linesRoot, FALSE, whichTier, depOn, chatName, line, bt, et);
			} else if (whichTier == DEPTTIER)
				dLinesRoot = add_each_line(dLinesRoot, FALSE, whichTier, depOn, chatName, line, bt, et);
			else {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "Found a tag \"%s\", not defined in attribs.cut file\n", tag);
				cutt_exit(0);
			}
		}
    }
    if (!isSpeakerFound) {
		fprintf(stderr, "Can't find speaker(s) specified in attribs.txt file anywhere in this file: %s\n", oldfname);
		freeMem();
		cutt_exit(0);
    }
    fprintf(fpout, "@UTF8\n");
    fprintf(fpout, "@Begin\n");
    fprintf(fpout, "@Languages:	en\n");
    output_Participants_ID();
    addDepsToSpeaker();
    outputTiers(linesRoot);
    fprintf(fpout, "@End\n");
    linesRoot = freeLines(linesRoot);
    dLinesRoot = freeLines(dLinesRoot);
	if (isMFNNUll)
		MFN = NULL;
}
