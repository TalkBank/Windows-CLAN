/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#include "cu.h"
#include "check.h"
#ifndef UNX
#include "ced.h"
#else
#define RGBColor int
#include "c_curses.h"
#endif
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main joinitems_main
#define call joinitems_call
#define getflag joinitems_getflag
#define init joinitems_init
#define usage joinitems_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define UTTSEGMENTSIZE 20
#define UTTS struct Uttrs
UTTS {
    char *sp;
    AttTYPE *attSp;
    char *line;
    char *uttline;
    AttTYPE *attLine;
	long lineno;
} ;

extern struct tier *defheadtier;
extern struct tier *headtier;
extern char R7Slash;
extern char R7Tilda;
extern char R7Caret;
extern char R7Colon;
extern char OverWriteFile;
extern char isRemoveCAChar[];

static UTTS uttSegment[UTTSEGMENTSIZE];
static int uttSegmentsLen;
static FILE *err_mess;
static char isErrorErrFile;
static char separacter;

void init(char f) {
	if (f) {
		separacter = '-';
		err_mess = NULL;
		stout = FALSE;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+0");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
		addword('\0','\0',"+<\">");
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		uttSegmentsLen = 0;
		onlydata = 1;
	} else {
		isRemoveCAChar[NOTCA_DOUBLE_COMMA] = FALSE;
		isRemoveCAChar[NOTCA_VOCATIVE] = FALSE;
		R7Slash = TRUE;
		R7Tilda = TRUE;
		R7Caret = TRUE;
		R7Colon = TRUE;
		isRemoveCAChar[NOTCA_CROSSED_EQUAL] = TRUE;
		isRemoveCAChar[NOTCA_LEFT_ARROW_CIRCLE] = TRUE;
		if (wdptr == NULL) {
			fprintf(stderr,"Please specify items to join with +s option\n");
		}
		if (headtier == NULL) {
			fprintf(stderr,"Please specify tier to look on for items with +t option\n");
		}
		if (wdptr == NULL || headtier == NULL)
			cutt_exit(0);
	}
}

void usage() {
	printf("find items specified by +s and join them with preceding words\n");
	printf("Usage: joinitems [cC sS tS] filename(s)\n");
	puts("+cC: use the separator character C (default: -).");
	puts("+sS: join item S with previous word.");
	puts("+tS: look for item specified with +s on tier S");
	puts("\nExample:");
	puts("       joinitems +s\"ptl:case|*\" +t%mor *.cha");
	cutt_exit(0);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = JOINITEMS;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	isErrorErrFile = FALSE;
	bmain(argc,argv,NULL);
	if (err_mess != NULL) {
#ifdef UNX
		fprintf(stderr, "\nErrors were found. Please read file \"0joint_errors.cut\".\n\n");
#else
		fprintf(stderr, "\n%c%cErrors were found. Please read file \"0joint_errors.cut\".%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
		fclose(err_mess);
	}
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'c':
			separacter = *f;
			break;
		case 's':
			addword('s','i',getfarg(f,f1,i));
			break;
		case 't':
			maketierchoice(getfarg(f,f1,i), '+', FALSE);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void freeUttSegs(void) {
	int i;

	for (i=0; i < uttSegmentsLen; i++) {
		if (uttSegment[i].sp != NULL)
			free(uttSegment[i].sp);
		if (uttSegment[i].attSp != NULL)
			free(uttSegment[i].attSp);
		if (uttSegment[i].line != NULL)
			free(uttSegment[i].line);
		if (uttSegment[i].uttline != NULL)
			free(uttSegment[i].uttline);
		if (uttSegment[i].attLine != NULL)
			free(uttSegment[i].attLine);
	}
	uttSegmentsLen = 0;
}

static void writeError(UTTS *mainTier, UTTS *depTier, const char *mess) {
	if (err_mess == NULL && !isErrorErrFile) {
		err_mess = fopen("0joint_errors.cut", "w");
		if (err_mess == NULL) {
			fprintf(stderr,"\n\n\tCan't open error file \"0joint_errors.cut\"\n\n");
		}
		isErrorErrFile = TRUE;
	}
	if (err_mess != NULL) {
		fputs("----------------------------------------\n",err_mess);
		fprintf(err_mess,"*** File \"%s\": line %ld.\n", oldfname, mainTier->lineno);
		fprintf(err_mess,"%s\n", mess);
		fprintf(err_mess,"%s%s", mainTier->sp, mainTier->line);
		fprintf(err_mess,"%s%s", depTier->sp, depTier->line);
	}
}

static void extractStems(char *to, char *from) {
	int i;

	for (; *from != EOS && *from != '|'; from++) ;
	if (*from == EOS)
		*to = EOS;
	else
		from++;
	for (i=0; isalnum(from[i]); i++)
		to[i] = from[i];
	to[i] = EOS;
}

static char shiftLeft(UTTS *mainTier, UTTS *depTier, int mbeg) {
	int i, t, mend, mRemBeg, mRemEnd;

	i = 0;
	templineC[i++] = separacter;
	for (mend=mbeg; !uS.isskip(mainTier->line,mend,&dFnt,MBF); mend++) {
		templineC[i++] = mainTier->line[mend];
	}
	templineC[i] = EOS;
	mRemBeg = mbeg;
	mRemEnd = mend;
	mend = mbeg - 2;
	for (mbeg=mend; mbeg >= 0 && mainTier->line[mbeg] != '['; mbeg--) ;
	if (mbeg < 0)
		return(FALSE);
	if (mainTier->line[mbeg+1] == ':') {
		i = 0;
		for (t=mbeg; t <= mend; t++) {
			templineC1[i++] = mainTier->line[t];
		}
		templineC1[i] = EOS;
		sprintf(templineC2, "Warning: ONLY moved speaker tier word \"%s\" to inside %s", templineC, templineC1);
		writeError(mainTier, depTier, templineC2);
		for (mbeg=mend; mbeg >= 0 && mainTier->line[mbeg] != '[' && (uS.isskip(mainTier->line,mbeg,&dFnt,MBF) || mainTier->line[mbeg] == ']'); mbeg--) ;
		if (mbeg < 0 || mainTier->line[mbeg] == '[')
			return(FALSE);
		att_cp(0L, mainTier->line+mRemBeg-1, mainTier->line+mRemEnd, mainTier->attLine+mRemBeg-1, mainTier->attLine+mRemEnd);
		mbeg++;
		att_shiftright(mainTier->line+mbeg, mainTier->attLine+mbeg, strlen(templineC));
		for (i=0; templineC[i] != EOS; i++)
			mainTier->line[mbeg++] = templineC[i];
	} else if (mainTier->line[mbeg+1] == '*' || mainTier->line[mbeg+1] == '?') {
		for (; mbeg >= 0 && mainTier->line[mbeg] != ']' && (uS.isskip(mainTier->line,mbeg,&dFnt,MBF) || mainTier->line[mbeg] == '['); mbeg--) {
			if (mainTier->line[mbeg] == '<' || mainTier->line[mbeg] == '>')
				return(FALSE);
		}
		if (mbeg < 0)
			return(FALSE);
		if (mainTier->line[mbeg] == ']') {
			mend = mbeg;
			for (; mbeg >= 0 && mainTier->line[mbeg] != '['; mbeg--) ;
			if (mbeg < 0)
				return(FALSE);
			if (mainTier->line[mbeg+1] == ':') {
				i = 0;
				for (t=mbeg; t <= mend; t++) {
					templineC1[i++] = mainTier->line[t];
				}
				templineC1[i] = EOS;
				sprintf(templineC2, "Warning: ONLY moved speaker tier word \"%s\" to inside %s", templineC, templineC1);
				writeError(mainTier, depTier, templineC2);
				for (mbeg=mend; mbeg >= 0 && mainTier->line[mbeg] != '[' && (uS.isskip(mainTier->line,mbeg,&dFnt,MBF) || mainTier->line[mbeg] == ']'); mbeg--) ;
				if (mbeg < 0 || mainTier->line[mbeg] == '[')
					return(FALSE);
			} else
				return(FALSE);
		}
		att_cp(0L, mainTier->line+mRemBeg-1, mainTier->line+mRemEnd, mainTier->attLine+mRemBeg-1, mainTier->attLine+mRemEnd);
		mbeg++;
		att_shiftright(mainTier->line+mbeg, mainTier->attLine+mbeg, strlen(templineC));
		for (i=0; templineC[i] != EOS; i++)
			mainTier->line[mbeg++] = templineC[i];
		if (mainTier->line[mbeg+1] == ':') {
		}
	} else
		return(FALSE);
	return(TRUE);
}

static int process_tiers(UTTS *mainTier, UTTS *depTier) {
	int  isChanged;
	char mw[SPEAKERLEN], dw[SPEAKERLEN];
	int mi, mio, di, dio;

	if (depTier == NULL)
		return(FALSE);
	isChanged = 0;
	mi = 0;
	di = 0;
	do {
		mio = mi;
		do {
			mi = getword(mainTier->sp, mainTier->uttline, mw, NULL, mi);
			if (uS.isRightChar(mw,0,'[',&dFnt,MBF))
				;
			else if (mw[0] == '+') {
				if ((mw[1] == '"' || mw[1] == '^' || mw[1] == ',' || mw[1] == '+' || mw[1] == '<') && mw[2] == EOS)
					;
				else if (mw[1] == EOS)
					;
				else
					break;
			} else
				break;
		} while (1) ;
		while (uS.isskip(mainTier->uttline,mio,&dFnt,MBF))
			mio++;
		do {
			dio = di;
			di = getword(depTier->sp, depTier->uttline, dw, NULL, di);
		} while (di != 0 && strcmp(dw, "n|quote") == 0) ;
		while (uS.isskip(depTier->uttline,dio,&dFnt,MBF))
			dio++;
		if (mi == 0 && di != 0) {
			writeError(mainTier, depTier, "Error: Main speaker tier has fewer words than chosen dependent tier does");
		}
		if (di == 0)
			break;
		if (exclude(dw)) {
			extractStems(templineC, dw);
			if (strcmp(mw, templineC)) {
				sprintf(templineC, "Warning: Speaker tier word \"%s\" and dependent tier word \"%s\" don't match", mw, dw);
				writeError(mainTier, depTier, templineC);
			}
			if (mio > 1 && isSpace(mainTier->line[mio-1]) && mainTier->line[mio-2] == '\n') {
				mainTier->line[mio-1] = separacter;
				isChanged++;
				att_cp(0L, mainTier->line+(mio-2), mainTier->line+(mio-1), mainTier->attLine+(mio-2), mainTier->attLine+(mio-1));
				mi--;
			} else if (mio > 1 && isSpace(mainTier->line[mio-1]) && mainTier->line[mio-2] == ']' && shiftLeft(mainTier, depTier, mio)) {
				isChanged++;
			} else if (mio > 1 && uS.isskip(mainTier->line,mio-2,&dFnt,MBF)) {
				sprintf(templineC, "Error: Too many spaces before speaker tier item \"%s\"", mw);
				writeError(mainTier, depTier, templineC);
			} else if (mio > 0 && !uS.isskip(mainTier->line,mio-1,&dFnt,MBF)) {
				sprintf(templineC, "Error: There is no space between speaker tier item \"%s\" and previous word", mw);
				writeError(mainTier, depTier, templineC);
			} else if (mio == 1 && uS.isskip(mainTier->line,mio-1,&dFnt,MBF)) {
				sprintf(templineC, "Error: There is no previous word to speaker tier item \"%s\"", mw);
				writeError(mainTier, depTier, templineC);
			} else if (uS.isskip(mainTier->line,mio-1,&dFnt,MBF)) {
				mainTier->line[mio-1] = separacter;
				isChanged++;
			}
		}
	} while (1) ;
	return(isChanged);
}

void call() {
	int  i;
	int totalChanges, isChanged;
	UTTS *mainTier, *depTier;

	mainTier = NULL;
	depTier = NULL;
	totalChanges = 0;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			if (uttSegmentsLen > 0) {
				isChanged = process_tiers(mainTier, depTier);
				totalChanges += isChanged;
				for (i=0; i < uttSegmentsLen; i++) {
					printout(uttSegment[i].sp, uttSegment[i].line, uttSegment[i].attSp, uttSegment[i].attLine, (i == 0 && isChanged));
				}
				freeUttSegs();
			} else
				uttSegmentsLen = 0;
			uttSegment[uttSegmentsLen].sp = (char *)malloc(strlen(utterance->speaker)+1);
			uttSegment[uttSegmentsLen].attSp = (AttTYPE *)malloc((strlen(utterance->speaker)+1)*sizeof(AttTYPE));
			uttSegment[uttSegmentsLen].line = (char *)malloc(strlen(utterance->line)+1);
			uttSegment[uttSegmentsLen].uttline = (char *)malloc(strlen(uttline)+1);
			uttSegment[uttSegmentsLen].attLine = (AttTYPE *)malloc((strlen(utterance->line)+1)*sizeof(AttTYPE));
			if (uttSegment[uttSegmentsLen].sp == NULL || uttSegment[uttSegmentsLen].attSp == NULL || uttSegment[uttSegmentsLen].line == NULL ||
				uttSegment[uttSegmentsLen].uttline == NULL || uttSegment[uttSegmentsLen].attLine == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"ERROR: out of memory!!!\n");
				fprintf(stderr,"%s%s\n", utterance->speaker, utterance->line);
				freeUttSegs();
				cutt_exit(0);
			}
			uttSegment[uttSegmentsLen].lineno = lineno;
			att_cp(0L, uttSegment[uttSegmentsLen].sp, utterance->speaker, uttSegment[uttSegmentsLen].attSp, utterance->attSp);
			att_cp(0L, uttSegment[uttSegmentsLen].line, utterance->line, uttSegment[uttSegmentsLen].attLine, utterance->attLine);
			strcpy(uttSegment[uttSegmentsLen].uttline, uttline);
			mainTier = &uttSegment[uttSegmentsLen];
			depTier = NULL;
			uttSegmentsLen++;
		} else if (utterance->speaker[0] == '%') {
			if (uttSegmentsLen == 0) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"ERROR: dependent tier preceded by header tier.\n");
				fprintf(stderr,"%s%s\n", utterance->speaker, utterance->line);
				freeUttSegs();
				cutt_exit(0);
			}
			if (uttSegmentsLen == UTTSEGMENTSIZE) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"Internal error: too many dependent tiers. Variable \"UTTSEGMENTSIZE\" needs to be increated!\n");
				fprintf(stderr,"%s%s\n", utterance->speaker, utterance->line);
				freeUttSegs();
				cutt_exit(0);
			}
			uttSegment[uttSegmentsLen].sp = (char *)malloc(strlen(utterance->speaker)+1);
			uttSegment[uttSegmentsLen].attSp = (AttTYPE *)malloc((strlen(utterance->speaker)+1)*sizeof(AttTYPE));
			uttSegment[uttSegmentsLen].line = (char *)malloc(strlen(utterance->line)+1);
			uttSegment[uttSegmentsLen].uttline = (char *)malloc(strlen(uttline)+1);
			uttSegment[uttSegmentsLen].attLine = (AttTYPE *)malloc((strlen(utterance->line)+1)*sizeof(AttTYPE));
			if (uttSegment[uttSegmentsLen].sp == NULL || uttSegment[uttSegmentsLen].attSp == NULL || uttSegment[uttSegmentsLen].line == NULL ||
				uttSegment[uttSegmentsLen].uttline == NULL || uttSegment[uttSegmentsLen].attLine == NULL) {
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"ERROR: out of memory!!!\n");
				fprintf(stderr,"%s%s\n", utterance->speaker, utterance->line);
				freeUttSegs();
				cutt_exit(0);
			}
			uttSegment[uttSegmentsLen].lineno = lineno;
			att_cp(0L, uttSegment[uttSegmentsLen].sp, utterance->speaker, uttSegment[uttSegmentsLen].attSp, utterance->attSp);
			att_cp(0L, uttSegment[uttSegmentsLen].line, utterance->line, uttSegment[uttSegmentsLen].attLine, utterance->attLine);
			strcpy(uttSegment[uttSegmentsLen].uttline, uttline);
			if (checktier(utterance->speaker)) {
				depTier = &uttSegment[uttSegmentsLen];
			}
			uttSegmentsLen++;
		} else {
			for (i=0; i < uttSegmentsLen; i++) {
				printout(uttSegment[i].sp, uttSegment[i].line, uttSegment[i].attSp, uttSegment[i].attLine, FALSE);
			}
			freeUttSegs();
			printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
		}
	}
	if (uttSegmentsLen > 0) {
		isChanged = process_tiers(mainTier, depTier);
		totalChanges += isChanged;
		for (i=0; i < uttSegmentsLen; i++) {
			printout(uttSegment[i].sp, uttSegment[i].line, uttSegment[i].attSp, uttSegment[i].attLine, (i == 0 && isChanged));
		}
	}
	freeUttSegs();
	if (totalChanges == 0)
		fprintf(stderr, "\tNo changes made\n");
	else
		fprintf(stderr, "\tTotal changes made: %d\n", totalChanges);
}
