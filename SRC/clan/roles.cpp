/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main roles_main
#define call roles_call
#define getflag roles_getflag
#define init roles_init
#define usage roles_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define DICNAME "roles.cut"
#define PERIOD 50

extern struct tier *headtier;
extern char OverWriteFile;

static FNType dicname[256];
static char roles_FirstTime;

static struct spkrs {
	char *orig;
	char isPrintedout;
	char *code;
	char isSameCode;
	char *role;
	struct spkrs *next;
} *rootIDs;

static void roles_cleanup(void) {
	struct spkrs *t;

	while (rootIDs != NULL) {
		t = rootIDs;
		rootIDs = rootIDs->next;
		free(t->orig);
		free(t->code);
		free(t->role);
		free(t);
	}
	rootIDs = NULL;
}

static void roles_overflow() {
	fprintf(stderr,"roles: no more memory available.\n");
	roles_cleanup();
	cutt_exit(0);
}

static void roles_makenewsym(char *orig, char *code, char *role) {
	struct spkrs *nextone;

	if (rootIDs == NULL) {
		rootIDs = NEW(struct spkrs);
		nextone = rootIDs;
	} else {
		nextone = rootIDs;
		while (nextone->next != NULL) nextone = nextone->next;
		nextone->next = NEW(struct spkrs);
		nextone = nextone->next;
	}
	if (nextone == NULL)
		roles_overflow();
	nextone->next = NULL;
	nextone->code = NULL;
	nextone->isSameCode = FALSE;
	nextone->role = NULL;
	nextone->isPrintedout = FALSE;
	
	nextone->orig = (char *)malloc(strlen(orig)+1);
	if (nextone->orig == NULL)
		roles_overflow();
	strcpy(nextone->orig, orig);
	
	nextone->role = (char *)malloc(strlen(role)+1);
	if (nextone->role == NULL)
		roles_overflow();
	strcpy(nextone->role, role);
	
	nextone->code = (char *)malloc(strlen(code)+1);
	if (nextone->role == NULL)
		roles_overflow();
	strcpy(nextone->code, code);
	
	fprintf(stderr, "From string: \"%s\" to role: \"%s\" code: \"%s\"\n", nextone->orig, nextone->role, nextone->code);
}

static void roles_readdict(void) {
	FILE *fdic;
	char line[1024], *o, *c, *r;
	FNType mFileName[FNSize];
	struct spkrs *rootID, *nextID;

	if (dicname[0] == EOS) {
		uS.str2FNType(dicname, 0L, DICNAME);
	}
	if ((fdic=OpenGenLib(dicname,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the changes files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", dicname, mFileName);
		cutt_exit(0);
	}
	while (fgets_cr(line, 1024, fdic)) {
		if (uS.isUTF8(line) || uS.isInvisibleHeader(line))
			continue;
		if (line[0] == ';' && line[1] == ' ')
			continue;
		if (line[0] == '#' && line[1] == ' ')
			continue;
		uS.remFrontAndBackBlanks(line);
		if (line[0] == EOS)
			continue;
		for (c=line; isSpace(*c) || *c == '\n'; c++) ;
		o = c;
		for (; *c != ' ' && *c != '\t' && *c != '\n' && *c != EOS; c++) ;
		if (*c != EOS) {
			*c = EOS;
			for (c++; *c == ' ' || *c == '\t' || *c == '\n'; c++) ;
			if (*c != EOS) {
				for (r=c; *r != ' ' && *r != '\t' && *r != '\n' && *r != EOS; r++) ;
				if (*r != EOS) {
					*r = EOS;
					for (r++; *r == ' ' || *r == '\t' || *r == '\n'; r++) ;
					roles_makenewsym(o, c, r);
				}
			}
		}
	}
	fclose(fdic);
	fprintf(stderr,"\n");
	for (rootID=rootIDs; rootID != NULL; rootID=rootID->next) {
		for (nextID=rootID->next; nextID != NULL; nextID=nextID->next) {
			if (uS.mStricmp(rootID->code, nextID->code) == 0)
				nextID->isSameCode = TRUE;
		}
	}
}

void init(char f) {

    if (f) {
		stout = FALSE;
		*dicname = EOS;
		roles_FirstTime = TRUE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		rootIDs = NULL;
		onlydata = 1;
	} else if (roles_FirstTime) {
		roles_readdict();
		roles_FirstTime = FALSE;
    }
}

void usage()			/* print proper usage and exit */
{
	puts("change speaker codes and roles.");
    printf("Usage: roles [cF %s] filename(s)\n",mainflgs());
	printf("+cF: dictionary file (Default %s) \n", DICNAME);
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(FALSE);
	puts("Dictionary file format: \"original_code speaker_code speaker_role\"");
	puts("Example:");
	puts("\troles +croles.cut *.cha");
	cutt_exit(0);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    chatmode = CHAT_MODE;
	CLAN_PROG_NUM = ROLES;
    OnlydataLimit = 0;
    UttlineEqUtterance = FALSE;
    bmain(argc,argv,NULL);
	roles_cleanup();
}
	
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
				if (*f)
					uS.str2FNType(dicname, 0L, getfarg(f,f1,i));
				break;
#ifdef UNX
		case 'L':
			int len;
			strcpy(lib_dir, f);
			len = strlen(lib_dir);
			if (len > 0 && lib_dir[len-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

void call() {   
	long tlineno = 0L;
	long isFound, origLen;
	int i, j;
	char idLangCurpus[SPEAKERLEN], isIDPrinted;
	struct spkrs *IDs;

	idLangCurpus[0] = EOS;
	isIDPrinted = FALSE;
	isFound = 0L;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		tlineno = tlineno + 1L;
		if (tlineno % PERIOD == 0)
			fprintf(stderr,"\r%ld ",tlineno);
		for (i=0; isSpace(utterance->line[i]); i++) ;
		if (utterance->speaker[0] == '@') {
			if (uS.partcmp(templineC,PARTICIPANTS,FALSE,FALSE) && templineC[strlen(PARTICIPANTS)] == ':') {
				templineC[0] = EOS;
				for (IDs=rootIDs; IDs != NULL; IDs=IDs->next) {
					if (IDs->isSameCode == FALSE) {
						strcat(templineC, IDs->code);
						strcat(templineC," ");
						strcat(templineC, IDs->role);
						if (IDs->next != NULL)
							strcat(templineC,", ");
					}
				}
				isFound++;
				printout("@Participants:", templineC, NULL, NULL, TRUE);
			} else if (uS.partcmp(templineC,IDOF,FALSE,FALSE) && templineC[strlen(IDOF)] == ':') {
				if (isIDPrinted == FALSE) {
					j = 0;
					for (i=0; utterance->line[i] != '|' && utterance->line[i] != EOS; i++)
						idLangCurpus[j++] = utterance->line[i];
					if (utterance->line[i] != EOS) {
						idLangCurpus[j++] = '|';
						for (i++; utterance->line[i] != '|' && utterance->line[i] != EOS; i++)
							idLangCurpus[j++] = utterance->line[i];
					}
					templineC[0] = EOS;
					strcpy(templineC, idLangCurpus);
					for (IDs=rootIDs; IDs != NULL; IDs=IDs->next) {
						if (IDs->isSameCode == FALSE) {
							strcpy(templineC, idLangCurpus);
							strcat(templineC, "|");
							strcat(templineC, IDs->code);
							strcat(templineC, "|");
							strcat(templineC, "|");
							strcat(templineC, "|");
							strcat(templineC, "|");
							strcat(templineC, "|");
							strcat(templineC, IDs->role);
							strcat(templineC, "|");
							strcat(templineC, "|");
							strcat(templineC, "|");
							printout("@ID:", templineC, NULL, NULL, FALSE);
							isFound++;
							isIDPrinted = TRUE;
						}
					}
				}
			} else 
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine, 0);
		} else if (utterance->speaker[0] == '*') {
			for (IDs=rootIDs; IDs != NULL; IDs=IDs->next) {
				origLen = strlen(IDs->orig);
				if (uS.mStrnicmp(IDs->orig, utterance->speaker+1, origLen) == 0 && utterance->speaker[origLen+1] == ':') {
					strcpy(utterance->speaker+1, utterance->speaker+origLen+1);
					uS.shiftright(utterance->speaker+1, strlen(IDs->code));
					j = 1;
					for (i=0; IDs->code[i] != EOS; i++) {
						utterance->speaker[j++] = IDs->code[i];
					}
					isFound++;
					break;
				}
			}
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine, 0);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine, 0);
		}
	}
	fprintf(stderr,"\n");
#ifndef UNX
	if (isFound == 0L && fpout != stdout && !WD_Not_Eq_OD) {
		fprintf(stderr,"**- NO changes made in this file\n");
		if (!replaceFile) {
			fclose(fpout);
			fpout = NULL;
			if (unlink(newfname))
				fprintf(stderr, "Can't delete output file \"%s\".", newfname);
		}
	} else
#endif
	if (isFound > 0L)
		fprintf(stderr,"**+ %ld changes made in this file\n", isFound);
	else
		fprintf(stderr,"**- NO changes made in this file\n");
}
