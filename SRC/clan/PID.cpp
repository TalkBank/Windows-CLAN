/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// change date to current: <oai:datestamp>2008-04-04</oai:datestamp>

#define CHAT_MODE 4
#include "cu.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main pid_main
#define call pid_call
#define getflag pid_getflag
#define init pid_init
#define usage pid_usage
#define mkdir my_mkdir
#else
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

#if defined(UNX) || defined(_MAC_CODE)
#define MODE S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH
#endif
#ifdef _WIN32
#define MODE 0
#endif


extern char OverWriteFile;
extern char isRecursive;
extern struct tier *defheadtier;

#define PID_SIZE	64
#define LANGNUM		10
#define LANGLEN		10

#define CODESIZE	20
#define IDFIELSSIZE 100

#define TOTANUMLANGS 64

#define METADATA struct metadatas
struct metadatas {
	char *tag;
	char *data;
	struct metadatas *next;
} ;

#define PATHTREE struct Path_Tree
struct Path_Tree {
	char *name;
	int  sl;
	char PIDp;
	long PID;
	struct Path_Tree *child;
	struct Path_Tree *sibling;
} ;

#define DUPPIDLIST struct dup_PID_list
struct dup_PID_list {
	char *fname;
	char PIDp;
	long PID;
	struct dup_PID_list *next;
} ;

struct ServersArr {
	const char *gitName;
};

static struct ServersArr serversList[] = {
//           GIT             Server Name    Media Name     Description
	/* 0*/	{""}, // this line has to be in 0 position
	/* 1*/	{"aphasia-data"},
	/* 2*/	{"asd-data"},
	/* 3*/	{"biling-data"},
	/* 4*/	{"ca-data"},
	/* 5*/	{"childes-data"},
	/* 6*/	{"class-data"},
	/* 7*/	{"dementia-data"},
	/* 8*/	{"fluency-data"},
	/* 9*/	{"homebank-data"},
	/*10*/	{"phon-data"},
	/*11*/	{"rhd-data"},
	/*12*/	{"samtale-data"},
	/*13*/	{"slabank-data"},
	/*14*/	{"tbi-data"},
	/*15*/	{"psychosis-data"},
	/*16*/	{NULL}
} ;

static char currentURL[BUFSIZ+2], LandingPage[BUFSIZ+2];
static char isJustTest;
static char missingLangs[TOTANUMLANGS][4];
static char isMissingLang[TOTANUMLANGS];
static int missingLangsIndex;
static int wdStartI, wdCurStartI;
static int serverCnt;
static long tln = 0L, cln = 0L, PIDcnt, PIDtodo, filesChanged;
static PATHTREE *tree_root;
static DUPPIDLIST *root_PID;
static FILE *errorFp, *PIDFp;

static PATHTREE *free_tree(PATHTREE *p) {
	PATHTREE *sib, *t;

	if (p != NULL) {
		if (p->child != NULL)
			free_tree(p->child);
		sib = p->sibling;
		while (sib != NULL) {
			if (sib->child != NULL)
				free_tree(sib->child);
			t = sib;
			sib = sib->sibling;
			if (t->name != NULL)
				free(t->name);
			free(t);
		}
		if (p->name != NULL)
			free(p->name);
		free(p);
	}
	return(NULL);
}

static DUPPIDLIST *free_PID_list(DUPPIDLIST *p) {
	DUPPIDLIST *t;

	while (p != NULL) {
		if (p->fname != NULL)
			free(p->fname);
		t = p;
		p = p->next;
		free(t);
	}
	return(NULL);
}

static void free_metadata(METADATA *p) {
	METADATA *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->tag != NULL)
			free(t->tag);
		if (t->data != NULL)
			free(t->data);
		free(t);
	}
}

void init(char f) {
	if (f) {
		isJustTest = TRUE;
		isRecursive = TRUE;
		stout = TRUE;
		combinput = TRUE;
		onlydata = 1;
		tree_root = NULL;
		PIDtodo = 0L;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		AddCEXExtension = ".xml";
		if (defheadtier != NULL) {
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
	}
}

void usage() {
	printf("Usage: pid [c dS l] filename(s)\n");
	puts("+c : Add @PID: headers to CHAT files, if needed.");
	puts("     (default: check for duplicate PIDs in files).");
	puts("\nInstructions:");
	puts("    In Unix Terminal \"cd\" to every server's repo and type:");
	puts("git status");
	puts("git pull");
	puts("    It would be best to run CHATTER on \"~/data/\" folder at this time too");
	puts("\n  In CLAN's \"Commands\" window:");
	puts("    Set working directory to \"~/data/\"");
	puts("    Run command to check for duplicate PIDs:");
	puts("pid *.cha");
	puts("    If errors found, then fix them and run above command again");
	puts("    If no duplicate PIDs found, then run command:");
	puts("pid +c *.cha");
	puts("    In Unix Terminal type:");
	puts("    run CHATTER on \"~/data/\" folder");
	puts("    If CHATTER finds error(s), then fix them anyway you can including");
	puts("      deleting repos and starting from scratch");
	puts("\n    Proceed only if CHATTER does not find any errors");
	puts("cd <to every server's repo>");
	puts("git status");
	puts("git commit -a");
	puts("deploy");
	cutt_exit(0);
}

static void writeError(const char *err) {
	if (errorFp == NULL) {
		strcpy(FileName1, wd_dir);
		addFilename2Path(FileName1, "0error.cut");
		errorFp = fopen(FileName1, "w");
	}
	if (errorFp != NULL)
		fprintf(errorFp, "%s", err);
	else
		fprintf(stderr, "%s", err);
}

static void generateHDL(char *st, long PID) {
	if (PID < 10L)
		sprintf(st, "0000000%ld", PID);
	else if (PID < 100L)
		sprintf(st, "000000%ld", PID);
	else if (PID < 1000L)
		sprintf(st, "00000%ld", PID);
	else if (PID < 10000L)
		sprintf(st, "0000%ld", PID);
	else if (PID < 100000L)
		sprintf(st, "000%ld", PID);
	else if (PID < 1000000L)
		sprintf(st, "00%ld", PID);
	else if (PID < 10000000L)
		sprintf(st, "0%ld", PID);
	else
		sprintf(st, "%ld", PID);
}

static int getCodeForRepoName(char *name) {
	int i;
	char *s;

	for (i=1; serversList[i].gitName != NULL; i++) {
		if (uS.mStrnicmp(name, serversList[i].gitName, strlen(serversList[i].gitName)) == 0)
			return(i);
	}
	if (serversList[i].gitName == NULL) {
		s = strchr(name, '/');
		if (s != NULL)
			*s = EOS;
		fprintf(stderr, "\n   Unrecognized repo name: \"%s\"\n", name);
		tree_root = free_tree(tree_root);
		cutt_exit(0);
	}
	return(0);
}

static void removeServernameAddFilename2Path(char *out, char *in) {
	int i, len;

	if (*in == PATHDELIMCHR)
		in++;

	for (i=1; serversList[i].gitName != NULL; i++) {
		len = strlen(serversList[i].gitName);
		if (uS.mStrnicmp(in, serversList[i].gitName, len) == 0) {
			addFilename2Path(out, in+len);
			return;
		}
	}

	addFilename2Path(out, in);
}

static unsigned char writePIDbatch(const char *URL, char *PIDs, char PIDp, long PID) {
	if (PID == 0) {
		fprintf(stderr, "\n   Internal error PID == 0.\n");
		tree_root = free_tree(tree_root);
		cutt_exit(0);
	}
	PIDs[0] = EOS;
	if (PIDFp == NULL) {
		strcpy(FileName1, wd_dir);
		addFilename2Path(FileName1, "0PID_batch.txt");
		PIDFp = fopen(FileName1, "w");
		if (PIDFp == NULL) {
			fprintf(stderr, "\n   ERROR: Can't create file \"%s\".\n", FileName1);
			return(FALSE);
		}
	}
	generateHDL(templineC3, PID);
	sprintf(PIDs, "11312/%c-%s-%d", PIDp, templineC3, 0);
	fprintf(PIDFp, "%s URL: %s\n", PIDs, URL);
	PIDtodo++;
	return(TRUE);
}

static METADATA *add2MetadataList(METADATA *mdata, char *h, char *l) {
	char *s;
	METADATA *p;

	if (mdata == NULL) {
		p = NEW(METADATA);
		mdata = p;
	} else {
		for (p=mdata; p->next != NULL; p=p->next) {
			if (uS.mStricmp(p->tag, h) == 0) {
				s = p->data;
				if (s != NULL) {
					p->data = (char *)malloc(strlen(l)+strlen(s)+3);
					if (p->data == NULL) {
						tree_root = free_tree(tree_root);
						free_metadata(mdata);
						fprintf(stderr, "Out of Memory!!");
						cutt_exit(0);
					}
					strcpy(p->data, s);
					strcat(p->data, "; ");
					strcat(p->data, l);
					free(s);
					return(mdata);
				}
			}
		}
		if (uS.mStricmp(p->tag, h) == 0) {
			s = p->data;
			if (s != NULL) {
				p->data = (char *)malloc(strlen(l)+strlen(s)+3);
				if (p->data == NULL) {
					tree_root = free_tree(tree_root);
					free_metadata(mdata);
					fprintf(stderr, "Out of Memory!!");
					cutt_exit(0);
				}
				strcpy(p->data, s);
				strcat(p->data, ", ");
				strcat(p->data, l);
				free(s);
				return(mdata);
			}
		}
		p->next = NEW(METADATA);
		p = p->next;
	}
	if (p == NULL) {
		tree_root = free_tree(tree_root);
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	p->next = NULL;
	p->tag = NULL;
	p->data = NULL;
	p->tag = (char *)malloc(strlen(h)+1);
	if (p->tag == NULL) {
		tree_root = free_tree(tree_root);
		free_metadata(mdata);
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	strcpy(p->tag, h);
	p->data = (char *)malloc(strlen(l)+1);
	if (p->data == NULL) {
		tree_root = free_tree(tree_root);
		free_metadata(mdata);
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	strcpy(p->data, l);
	return(mdata);
}

static char invalidPID(char *PID) {
	int i;

	if (uS.mStrnicmp(PID, "11312/", 6) != 0)
		return(TRUE);
	PID += 6;
	if (*PID != 'c' && *PID != 't' && *PID != 'a')
		return(TRUE);
	PID++;
	if (*PID != '-')
		return(TRUE);
	for (i=0, PID++; i < 8; i++, PID++) {
		if (!isdigit(*PID))
			return(TRUE);
	}
	if (*PID != '-')
		return(TRUE);
	PID++;
	if (!isdigit(*PID))
		return(TRUE);
	return(FALSE);
}

static void createSessionCHATFile(PATHTREE *p, char *wd_cur) {
	int  i;
	char isUTFound, isPIDFound;
	char *s,PID_Self[PID_SIZE];
	FILE *fp, *fpn;

	cln++;
	if (cln > tln) {
		tln = cln + 200;
#if !defined(CLAN_SRV)
		fprintf(stderr,"\r%ld ", cln);
#endif
#if !defined(UNX)
		my_flush_chr();
#endif
	}
#if defined(_MAC_CODE) || defined(_WIN32)
#ifndef _COCOA_APP
	SysEventCheck(100L);
#endif
#endif
	s = strrchr(p->name, '.');
	if (s == NULL || uS.mStricmp(s, ".cha"))
		return;
	isPIDFound = FALSE;
	isUTFound = 0;
	cMediaFileName[0] = EOS;
	strcpy(FileName2, wd_dir);
	addFilename2Path(FileName2, wd_cur+wdCurStartI);
	addFilename2Path(FileName2, p->name);
	fp = fopen(FileName2, "r");
	if (fp == NULL) {
		tree_root = free_tree(tree_root);
		fprintf(stderr, "\n   Can't open CHAT file \"%s\".\n", FileName2);
		cutt_exit(0);
	} else {
		templineC[0] = EOS;
		while (fgets_cr(templineC1, UTTLINELEN, fp)) {
			if (templineC1[0] == '@' || templineC1[0] == '*' || templineC1[0] == '%') {
				for (i=0; templineC[i] != EOS; i++) {
					if (templineC[i] == '\t' || templineC[i] == '\n')
						templineC[i] = ' ';
				}
				if (uS.partcmp(templineC, UTF8HEADER, FALSE, FALSE)) {
					isUTFound = 1;
				}
				if (isUTFound) {
					if (uS.partcmp(templineC, UTF8HEADER, FALSE, FALSE)) {
					} else {
						if (uS.partcmp(templineC, PIDHEADER, FALSE, FALSE)) {
							for (i=strlen(PIDHEADER); isSpace(templineC[i]); i++) ;
							uS.remblanks(templineC+i);
							if (isUTFound == 2) {
								sprintf(templineC2, "In File \"%s\"\n    %s header found in a wrong place\n", FileName2, PIDHEADER);
								writeError(templineC2);
							}
							if (isPIDFound) {
								sprintf(templineC2, "In File \"%s\"\n    More than one %s header found\n", FileName2, PIDHEADER);
								writeError(templineC2);
							}
							if (invalidPID(templineC+i)) {
								fprintf(stderr, "\nIn File \"%s\"\n    %s header is corrupt or has bad prefix\n", FileName2, PIDHEADER);
								tree_root = free_tree(tree_root);
								cutt_exit(0);
							}
							isPIDFound = TRUE;
						}
						isUTFound = 2;
					}
				}
				if (templineC1[0] == '*' || templineC1[0] == '%')
					break;
				strcpy(templineC, templineC1);
			} else if (isSpace(templineC1[0]))
				strcat(templineC, templineC1);
		}
		fclose(fp);
	}
	if (isKillProgram) {
		tree_root = free_tree(tree_root);
		cutt_exit(0);
	}
	strcpy(currentURL, wd_cur+wdCurStartI);
	addFilename2Path(currentURL, p->name);
	strcat(currentURL, ".cha");
	i = 8;
	while (currentURL[i] != EOS) {
		if (currentURL[i] == '\\')
			currentURL[i] = '/';
		if (currentURL[i] == '/' && (currentURL[i+1] == '/' || currentURL[i+1] == '\\'))
			strcpy(currentURL+i, currentURL+i+1);
		else
			i++;
	}
	for (i=strlen(currentURL); i > 0 && currentURL[i] != '/'; i--) {
		if (currentURL[i] == '.') {
			currentURL[i] = EOS;
			break;
		}
	}
	if (writePIDbatch(currentURL, PID_Self, p->PIDp, p->PID) == 0) {
		tree_root = free_tree(tree_root);
		cutt_exit(0);
	}
	if (!isPIDFound) {
		if (isJustTest) {
			filesChanged++;
		} else {
			fp = fopen(FileName2, "r");
			if (fp != NULL) {
				strcpy(FileName1, FileName2);
				strcat(FileName1, ".del");
				fpn = fopen(FileName1,"w");
				if (fpn != NULL) {
					char isEmpty;
					isEmpty = TRUE;
					isUTFound = 0;
					filesChanged++;
// fprintf(stderr, "File Changed: %s;\n", FileName2);
					templineC[0] = EOS;
					while (fgets_cr(templineC, UTTLINELEN, fp)) {
						isEmpty = FALSE;
						if (uS.partcmp(templineC, "@Begin", FALSE, FALSE) && isUTFound == 0) {
							fprintf(fpn, "%s\n", UTF8HEADER);
							fprintf(fpn, "%s\t%s\n", PIDHEADER, PID_Self);
							isUTFound = 1;
						}
						fprintf(fpn, "%s", templineC);
						if (uS.partcmp(templineC, UTF8HEADER, FALSE, FALSE) && isUTFound == 0) {
							fprintf(fpn, "%s\t%s\n", PIDHEADER, PID_Self);
							isUTFound = 1;
						}

					}
					if (isEmpty) {
						fprintf(fpn, "%s\n", UTF8HEADER);
						fprintf(fpn, "%s\t%s\n", PIDHEADER, PID_Self);
					}
					fclose(fp);
					fclose(fpn);
					if (isKillProgram) {
						unlink(FileName1);
						tree_root = free_tree(tree_root);
						cutt_exit(0);
					}
					unlink(FileName2);
#ifndef UNX
					if (rename_each_file(FileName1, FileName2, FALSE) == -1) {
						tree_root = free_tree(tree_root);
						fprintf(stderr, "\n   Can't rename original file \"%s\". Perhaps it is opened by some application.\n", FileName2);
						cutt_exit(0);
					}
#else
					rename(FileName1, FileName2);
#endif
				} else {
					tree_root = free_tree(tree_root);
					fprintf(stderr, "\n   Can't create temp file \"%s\".\n", FileName1);
					fclose(fp);
					cutt_exit(0);
				}
			}
		}
	}
}

static void writingTree(PATHTREE *p, char *wd_cur) {
	int len;
	PATHTREE *sib;

	if (p != NULL) {
		len = strlen(wd_cur);
		if (p->child != NULL) {
			if (p->name != NULL) {
				addFilename2Path(wd_cur, p->name);
			}
			writingTree(p->child, wd_cur);
		} else if (p->name != NULL) {
			createSessionCHATFile(p, wd_cur);
		}
		sib = p->sibling;
		while (sib != NULL) {
			wd_cur[len] = EOS;
			if (sib->child != NULL) {
				if (sib->name != NULL) {
					addFilename2Path(wd_cur, sib->name);
				}
				writingTree(sib->child, wd_cur);
			} else if (sib->name != NULL) {
				createSessionCHATFile(sib, wd_cur);
			}
			sib = sib->sibling;
		}
		wd_cur[len] = EOS;
	}
}

static void addNewPIDNums(PATHTREE *p) {
	PATHTREE *sib;

	if (p != NULL) {
		if (p->PID == 0L) {
			PIDcnt++;
			p->PID = PIDcnt;
		}
		for (sib=p->sibling; sib != NULL; sib=sib->sibling) {
			if (sib->PID == 0L) {
				PIDcnt++;
				sib->PID = PIDcnt;
			}
			if (sib->child != NULL)
				addNewPIDNums(sib->child);
		}
		if (p->child != NULL)
			addNewPIDNums(p->child);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int  i;
	char *s;
	char wd_cur[FNSize];

#ifdef UNX
	getcwd(wd_dir, FNSize);
	strcpy(od_dir, wd_dir);
	strcpy(lib_dir, DEPDIR);
	strcpy(mor_lib_dir, DEPDIR);
#endif
	isJustTest = TRUE;
	if (argc > 1) {
		for (i=1; i < argc; i++) {
			if (*argv[i] == '+' || *argv[i] == '-') {
				if (argv[i][1] == 'c') {
					isJustTest = FALSE;
				}
			}
		}
	}
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = PID_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	strcpy(wd_cur, wd_dir);
	i = strlen(wd_cur) - 1;
	if (wd_cur[i] == PATHDELIMCHR)
		wd_cur[i] = EOS;
	s = strrchr(wd_cur, PATHDELIMCHR);
	if (s != NULL) {
		*s = EOS;
	}
	wdStartI = strlen(wd_dir);
	addFilename2Path(wd_cur, "data-pid");
	wdCurStartI = strlen(wd_cur);
	filesChanged = 0L;
	errorFp = NULL;
	PIDcnt = 0L;
	PIDFp = NULL;
	tln = 0L;
	cln = 0L;
	missingLangsIndex = 0;
	initLanguages();
	serverCnt = 0;
	for (i=0; serversList[i].gitName != NULL; i++) {
		serverCnt++;
	}
	root_PID = NULL;
	bmain(argc,argv,NULL);
	if (isKillProgram) {
		tree_root = free_tree(tree_root);
		cutt_exit(0);
	}
#if !defined(CLAN_SRV)
	fprintf(stderr,"\r%ld ", cln);
#endif
	fprintf(stderr, "\nDone reading folders data\n");
	if (!isJustTest)
		fprintf(stderr, "Writing data\n");
	else
		fprintf(stderr, "Testing data\n");
	fprintf(stderr, "Current largest PID is %ld\n", PIDcnt);
	LandingPage[0] = EOS;
	tln = 0L;
	cln = 0L;
	addNewPIDNums(tree_root);
	writingTree(tree_root, wd_cur);
#if !defined(CLAN_SRV)
	fprintf(stderr,"\r%ld ", cln);
#endif
	if (!isJustTest) {
		fprintf(stderr, "\nDone writing data\n");
		fprintf(stderr, "New largest PID is %ld\n", PIDcnt);
	} else {
		fprintf(stderr, "\nDone testing data\n");
		fprintf(stderr, "New largest PID will be %ld\n", PIDcnt);
	}
	for (i=0; i < missingLangsIndex; i++) {
		if (isMissingLang[i] == TRUE)
			sprintf(templineC2, "Missing translation for language code: %s\n", missingLangs[i]);
		else
			sprintf(templineC2, "Two letter language code found: %s\n", missingLangs[i]);
		writeError(templineC2);
	}
	tree_root = free_tree(tree_root);
	root_PID = free_PID_list(root_PID);
	strcpy(FileName1, wd_dir);
	addFilename2Path(FileName1, "0error.cut");
	if (errorFp != NULL) {
		fclose(errorFp);
		fprintf(stderr, "\n    Errors were detected. Please read file:\n");
		fprintf(stderr,"*** File \"%s\"\n", FileName1);
		fprintf(stderr, "    Fix errors and run \"pid *.cha\" command again\n");
	} else {
		if (isJustTest) {
#ifdef UNX
			printf("\nTHIS WAS JUST A TEST.\n");
#else
			printf("\n%c%cTHIS WAS JUST A TEST.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
			fprintf(stderr,"Success! No duplicate PIDs found.\n");
#ifdef UNX
			printf("NOW PLEASE RUN THE FOLLOWING COMMAND:\n");
#else
			printf("%c%cNOW PLEASE RUN THE FOLLOWING COMMAND:%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
			puts("pid +c *.cha");
		}
		unlink(FileName1);
	}
	if (isJustTest)
		fprintf(stderr,"\n    NUMBER OF CHAT FILES WILL BE CHANGED: %ld\n\n", filesChanged);
	else
		fprintf(stderr,"\n    NUMBER OF CHAT FILES HAVE BEEN CHANGED: %ld\n\n", filesChanged);
	if (PIDFp != NULL) {
		fclose(PIDFp);
		if (!isJustTest) {
#ifdef UNX
			printf("PLEASE DO THE FOLLOWING NOW:\n");
#else
			printf("%c%cPLEASE DO THE FOLLOWING NOW:%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
			puts("  run CHATTER on \"~/data/\" folder");
			puts("If CHATTER finds error, then fix them anyway you can including deleting repos and starting from scratch");
			puts("\nProceed only if CHATTER does not find any errors");
			puts("cd <to every server's repo>");
			puts("git status");
			puts("git commit -a");
			puts("deploy");
		}
	}
	if (isKillProgram) {
		tree_root = free_tree(tree_root);
		cutt_exit(0);
	}
}

static DUPPIDLIST *add_to_PID_list(DUPPIDLIST *root, char PIDp, long PID) {
	char *fname;
	DUPPIDLIST *nt;

	if (oldfname[wdStartI] == PATHDELIMCHR)
		fname = oldfname+wdStartI+1;
	else
		fname = oldfname+wdStartI;
	if (root == NULL) {
		root = NEW(DUPPIDLIST);
		nt = root;
	} else {
		nt = root;
		while (1) {
			if (nt->PIDp == PIDp && nt->PID == PID) {
				sprintf(templineC2, "Duplicate PID: %c-%ld-1 in files:\n	%s;\n\t%s\n", PIDp, PID, nt->fname, fname);
				writeError(templineC2);
				return(root);
			}
			if (nt->next == NULL)
				break;
			nt = nt->next;
		}
		nt->next = NEW(DUPPIDLIST);
		nt = nt->next;
		if (nt == NULL)
			return(root);
	}
	if (nt == NULL) {
		tree_root = free_tree(tree_root);
		root_PID = free_PID_list(root_PID);
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	nt->fname = (char *)malloc(strlen(fname)+1);
	if (nt->fname == NULL) {
		tree_root = free_tree(tree_root);
		root_PID = free_PID_list(root_PID);
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	strcpy(nt->fname, fname);
	nt->PIDp = PIDp;
	nt->PID = PID;
	nt->next = NULL;
	return(root);
}

void call() {
	int  i, sl;
	long PID;
	char *s, PIDp;
	char wd_cur[FNSize], name[FILENAME_MAX];
	PATHTREE *p;

	PIDp = 'a';
	s = strrchr(oldfname, '.');
	if (s != NULL) {
		if (uS.mStricmp(s, ".cha") == 0) {
			PID = 0L;
			while (fgets_cr(templineC2, UTTLINELEN, fpin)) {
				if (templineC2[0] == '*' || uS.partcmp(templineC2, "@Begin", FALSE, FALSE))
					break;
				if (uS.partcmp(templineC2, PIDHEADER, FALSE, FALSE)) {
					for (i=strlen(PIDHEADER); isSpace(templineC2[i]); i++) ;
					uS.remblanks(templineC2+i);
					if (invalidPID(templineC2+i)) {
						fprintf(stderr, "\nIn File \"%s\"\n    %s header is corrupt or has bad prefix\n", oldfname, PIDHEADER);
						tree_root = free_tree(tree_root);
						cutt_exit(0);
					}
					s = strrchr(templineC2, '-');
					if (s != NULL)
						*s = EOS;
					s = strrchr(templineC2, '-');
					if (s != NULL) {
						PID = atol(s+1);
						PIDp = *(s-1);
						if (PIDp == 'a' && PIDcnt < PID)
							PIDcnt = PID;
						if (isJustTest)
							root_PID = add_to_PID_list(root_PID, PIDp, PID);
					}
				}
			}
		} else
			PID = -1L;
	} else
		PID = -1L;
	if (oldfname[wdStartI] == PATHDELIMCHR)
		strcpy(wd_cur, oldfname+wdStartI+1);
	else
		strcpy(wd_cur, oldfname+wdStartI);
	p = tree_root;
	if (strchr(wd_cur, ' ')) {
		sprintf(templineC2, "No space characters allowed in file or path name: %s\n", wd_cur);
		writeError(templineC2);
	}
	sl = getCodeForRepoName(wd_cur);

	do {
		s = strchr(wd_cur, PATHDELIMCHR);
		if (s != NULL)
			*s = EOS;
		strcpy(name, wd_cur);
		if (tree_root == NULL) {
			tree_root = NEW(PATHTREE);
			if (tree_root == NULL) {
				tree_root = free_tree(tree_root);
				fprintf(stderr, "Out of Memory!!");
				cutt_exit(0);
			}
			p = tree_root;
			p->child = NULL;
			p->sibling = NULL;
			p->name = NULL;
			p->sl = sl;
			if (s == NULL)
				p->PID = PID;
			else
				p->PID = -1;
			p->PIDp = PIDp;
			p->name = (char *)malloc(strlen(name)+1);
			if (p->name == NULL) {
				tree_root = free_tree(tree_root);
				fprintf(stderr, "Out of Memory!!");
				cutt_exit(0);
			}
			strcpy(p->name, name);
		} else {
			if (p->name == NULL) {
				p->name = (char *)malloc(strlen(name)+1);
				if (p->name == NULL) {
					tree_root = free_tree(tree_root);
					fprintf(stderr, "Out of Memory!!");
					cutt_exit(0);
				}
				strcpy(p->name, name);
				p->sl = sl;
				if (s == NULL)
					p->PID = PID;
				else
					p->PID = -1;
				p->PIDp = PIDp;
			} else {
				while (p->sibling != NULL) {
					if (uS.mStricmp(p->name, name) == 0)
						break;
					p = p->sibling;
				}
				if (uS.mStricmp(p->name, name) != 0) {
					p->sibling = NEW(PATHTREE);
					if (p->sibling == NULL) {
						tree_root = free_tree(tree_root);
						fprintf(stderr, "Out of Memory!!");
						cutt_exit(0);
					}
					p = p->sibling;
					p->child = NULL;
					p->sibling = NULL;
					p->name = NULL;
					p->sl = sl;
					if (s == NULL)
						p->PID = PID;
					else
						p->PID = -1;
					p->PIDp = PIDp;
					p->name = (char *)malloc(strlen(name)+1);
					if (p->name == NULL) {
						tree_root = free_tree(tree_root);
						fprintf(stderr, "Out of Memory!!");
						cutt_exit(0);
					}
					strcpy(p->name, name);
				}
			}
		}
		if (s != NULL) {
			strcpy(wd_cur, s+1);
			if (p->child == NULL) {
				p->child = NEW(PATHTREE);
				if (p->child == NULL) {
					tree_root = free_tree(tree_root);
					fprintf(stderr, "Out of Memory!!");
					cutt_exit(0);
				}
				p = p->child;
				p->child = NULL;
				p->sibling = NULL;
				p->name = NULL;
				p->sl = sl;
				p->PID = -1;
				p->PIDp = PIDp;
			} else
				p = p->child;
		}
	} while (s != NULL) ;
	cln++;
	if (cln > tln) {
		tln = cln + 200;
#if !defined(CLAN_SRV)
		fprintf(stderr,"\r%ld ", cln);
#endif
#if !defined(UNX)
		my_flush_chr();
#endif
	}
#if defined(_MAC_CODE) || defined(_WIN32)
#ifndef _COCOA_APP
	SysEventCheck(100L);
#endif
#endif
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
				isJustTest = FALSE;
				no_arg_option(f);
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}
