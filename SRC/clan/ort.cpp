/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3
#include "cu.h"
#if defined(_MAC_CODE)
	#include <sys/stat.h>
	#include <dirent.h>
#endif
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

#if !defined(UNX)
#define _main ort_main
#define call ort_call
#define getflag ort_getflag
#define init ort_init
#define usage ort_usage
#endif

#include "mul.h" 
#define IS_WIN_MODE FALSE

struct ccodes {
    char name[126];
    char oc[126];
    char nc[126];
    char line[1024];
    int isMatched;
	struct ccodes *next_code;
} ;

struct cdicts {
    char word[126];
    char sym[126];
	struct cdicts *next_word;
} ;

extern struct tier *defheadtier;
extern char OverWriteFile;

static struct ccodes *rc;
static struct cdicts *rd;
static char ftime;
static FNType codesTableFile[FNSize];
static char isCUsed;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */
static void freeRC(void) {
	struct ccodes *t;

	while (rc != NULL) {
		t = rc;
		rc = rc->next_code;
		free(t);
	}
}

static void addToTable(char *nc, int cnt, char *oc, char *line) {
	int l1, l2;
	struct ccodes *nt, *tnt;

	l1 = strlen(oc);
	if (rc == NULL) {
		rc = NEW(struct ccodes);
		nt = rc;
		nt->next_code = NULL;
	} else {
		tnt= rc;
		nt = rc;
		while (1) {
			if (nt == NULL) break;
			else if (uS.partcmp(oc,nt->oc,FALSE,TRUE)) {
				l2 = strlen(nt->oc);
				if (l1 > l2)
					break;
			}
			tnt = nt;
			nt = nt->next_code;
		}
		if (nt == NULL) {
			tnt->next_code = NEW(struct ccodes);
			nt = tnt->next_code;
			nt->next_code = NULL;
		} else if (nt == rc) {
			rc = NEW(struct ccodes);
			rc->next_code = nt;
			nt = rc;
		} else {
			nt = NEW(struct ccodes);
			nt->next_code = tnt->next_code;
			tnt->next_code = nt;
		}
	}

	strcpy(nt->name, nc);
	strcpy(nt->oc, oc);
	sprintf(nt->nc,"%s_%d", nc, cnt);
	strcpy(nt->line, line);
	nt->isMatched = 0;
}

static void readCodesTable(void) {
	FILE *fp;
	int i, cnt;
	char orgC[126], *tOrgC = NULL, *oc = NULL, *line = NULL;

#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(wd_dir);
#endif

	if ((fp=fopen(codesTableFile,"r")) == NULL) {
		fprintf(stderr, "Error opening file %s.\n", codesTableFile);
		cutt_exit(0);
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
#endif

	cnt = 0;
	orgC[0] = EOS;
    while (fgets_cr(templineC, UTTLINELEN, fp)) {
    	if (templineC[0] == '#')
    		continue;

    	for (i=0; templineC[i] != ',' && templineC[i]; i++) ;
    	if (templineC[i] == ',') i++;
 
    	for (; isSpace(templineC[i]); i++) ;
    	if (templineC[i] != ',') {
    		cnt = 0;
    		tOrgC = templineC+i;
    	}
    	cnt++;
     	for (; templineC[i] != ',' && templineC[i]; i++) ;
    	if (templineC[i] == ',') {
    		templineC[i] = EOS;
    		i++;
    		if (tOrgC != NULL && tOrgC[0])
    			strcpy(orgC, tOrgC);
    	}

    	for (; isSpace(templineC[i]); i++) ;
    	if (templineC[i] != ',' && templineC[i])
    		oc = templineC+i;
     	for (; templineC[i] != ',' && templineC[i]; i++) ;
    	if (templineC[i] == ',') {
    		templineC[i] = EOS;
    		i++;
    	}
   	
    	for (; isSpace(templineC[i]); i++) ;
    	if (templineC[i] != ',' && templineC[i])
    		line = templineC+i;

		if (orgC[0] == EOS) {
			fprintf(stderr, "Error reading file %s.\n", codesTableFile);
			freeRC();
			cutt_exit(0);
		}
		if (line != NULL)
			addToTable(orgC, cnt, oc, line);
    }
	fclose(fp);
/*
	struct ccodes *nt;

	if (chatmode == 0)
		return;

#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(wd_ref,wd_dir);
#endif

	strcpy(templineC, codesTableFile);
	strcat(templineC, ".cex");
	if ((fp=fopen(templineC,"w")) == NULL) {
		fprintf(stderr, "Error writing file %s.\n", templineC);
		freeRC();
		cutt_exit(0);
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_ref, od_dir);
#endif

	fprintf(fp, "%s\n", UTF8HEADER);
	for (nt=rc; nt != NULL; nt=nt->next_code) {
		fprintf(fp, "%s, %s, %s, %s", nt->name, nt->oc, nt->nc, nt->line);
	}
	fclose(fp);
*/
}

static void freeRD(void) {
	struct cdicts *t;

	while (rd != NULL) {
		t = rd;
		rd = rd->next_word;
		free(t);
	}
}

static void addToDictTable(char *word, const char *sym) {
	int l1, l2;
	struct cdicts *nt, *tnt;

	l1 = strlen(word);
	if (rd == NULL) {
		rd = NEW(struct cdicts);
		nt = rd;
		nt->next_word = NULL;
	} else {
		tnt= rd;
		nt = rd;
		while (1) {
			if (nt == NULL) break;
			else if (uS.partcmp(word,nt->word,FALSE,TRUE)) {
				l2 = strlen(nt->word);
				if (l1 > l2)
					break;
			}
			tnt = nt;
			nt = nt->next_word;
		}
		if (nt == NULL) {
			tnt->next_word = NEW(struct cdicts);
			nt = tnt->next_word;
			nt->next_word = NULL;
		} else if (nt == rd) {
			rd = NEW(struct cdicts);
			rd->next_word = nt;
			nt = rd;
		} else {
			nt = NEW(struct cdicts);
			nt->next_word = tnt->next_word;
			tnt->next_word = nt;
		}
	}

	strcpy(nt->word, word);
	strcpy(nt->sym, sym);
}

static void tOpenLex(FNType *mFileName, int *isOpenOne) {
	int  index, i, t, len;
	char *word;
	const char *sym;
	FNType tFName[FNSize];
	FILE *lex;

	index = 1;
	t = strlen(mFileName);
	while ((index=Get_File(tFName, index)) != 0) {
		len = strlen(tFName) - 4;
		if (len >= 0 && uS.FNTypeicmp(tFName+len, ".cut", 0) == 0) {
			addFilename2Path(mFileName, tFName);
			if ((lex=fopen(mFileName, "r"))) {
				fprintf(stderr,"\rUsing lexicon: %s.\n", mFileName);
				while (fgets_cr(templineC1, UTTLINELEN, lex)) {
					if (isalnum((unsigned char)templineC1[0])) {
						word = templineC1;
						for (i=0; !isSpace(templineC1[i]) && templineC1[i] != EOS; i++) ;
						if (templineC1[i] == EOS)
							continue;
						templineC1[i] = EOS;
						for (i++; templineC1[i] != '%' && templineC1[i] != EOS; i++) ;
						if (templineC1[i] != '%')
							sym = "";
						else {
							sym = templineC1+i+1;
							while (isSpace(*sym))
								sym++;
						}
						for (; templineC1[i] != '\n' && templineC1[i] != EOS; i++) ;
						templineC1[i] = EOS;
						addToDictTable(word, sym);
					}
				}
				fclose(lex);
				lex = NULL;
				*isOpenOne = TRUE;
			}
			mFileName[t] = EOS;
		}
	}
}

static char tFindAndOpenLex(void) {
	int  isOpenOne = FALSE;
	int  t;
	FNType lexst[5], mFileName[FNSize];
#if defined(_MAC_CODE)
	struct dirent *dp;
	struct stat sb;
	DIR *cDIR;
#elif defined(_WIN32)
	BOOL notDone;
	CString dirname;
	CFileFind fileFind;
	FNType tFileName[FNSize];
#endif
	
	uS.str2FNType(lexst, 0L, "lex");
	uS.str2FNType(lexst, strlen(lexst), PATHDELIMSTR);
	strcpy(mFileName, wd_dir);
	addFilename2Path(mFileName, lexst);
	SetNewVol(mFileName);
	
	tOpenLex(mFileName, &isOpenOne);
	if (!isOpenOne) {
		strcpy(mFileName,mor_lib_dir);
		addFilename2Path(mFileName, lexst);
		SetNewVol(mFileName);
		tOpenLex(mFileName, &isOpenOne);
		if (!isOpenOne) {
			strcpy(mFileName,mor_lib_dir);
			t = strlen(mFileName);
#if defined(_MAC_CODE)
			if ((cDIR=opendir(".")) != NULL) {
				while ((dp=readdir(cDIR)) != NULL) {
					if (stat(dp->d_name, &sb) == 0) {
						if (!S_ISDIR(sb.st_mode)) {
							continue;
						}
					} else
						continue;
					if (dp->d_name[0] == '.')
						continue;
					addFilename2Path(mFileName, dp->d_name);
					addFilename2Path(mFileName, lexst);
					if (!chdir(mFileName)) {
						tOpenLex(mFileName, &isOpenOne);
						if (isOpenOne)
							break;
					}
					mFileName[t] = EOS;
					SetNewVol(mFileName);
				}
				closedir(cDIR);
			}
#elif defined(_WIN32)
			if (!fileFind.FindFile(cl_T("*.*"), 0)) {
				fileFind.Close();
			} else {
				do {
					notDone = fileFind.FindNextFile();
					if (!fileFind.IsDirectory())
						continue;
					dirname = fileFind.GetFileName();
					if (!strcmp(dirname, ".") || !strcmp(dirname, ".."))
						continue;
					u_strcpy(tFileName, dirname, FNSize);
					addFilename2Path(mFileName, tFileName);
					addFilename2Path(mFileName, lexst);
					strcat(mFileName, PATHDELIMSTR);
					if (!chdir(mFileName)) {
						tOpenLex(mFileName, &isOpenOne);
						if (isOpenOne)
							break;
					}
					mFileName[t] = EOS;
					SetNewVol(mFileName);
				} while (notDone) ;
				fileFind.Close();
			}
#endif
		}
	}
	SetNewVol(wd_dir);
	if (!isOpenOne) {
		return(FALSE);
	} else
		return(TRUE);
}

static void readDictTable(void) {
	if (!tFindAndOpenLex()) {
		fprintf(stderr,"Can't open any lexicon file(s). Please make sure \"mor lib\" is set correctly.\n");
		freeRD();
		cutt_exit(0);
	}
}

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		stout = FALSE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		rc = NULL;
		rd = NULL;
		ftime = TRUE;
		isCUsed = FALSE;
		uS.str2FNType(codesTableFile, 0L, "0canhomo.cut");
	} else if (ftime) {
		if (isCUsed)
			readCodesTable();
		else {
			FilterTier = 1;
			readDictTable();
		}
		ftime = FALSE;
	}
}

void usage() {
	printf("Usage: ort *.cha\n");
//	puts("    ort +c0canhomo.cut +t%mor *.cha");
//	puts("    freq +c +c1 +u +t% +k *.tmp.cex");
//	puts("    ort +c0canhomo.cut 0allCan.cut -y");
//	puts("    ren 0allCan.tmp.cex 0allCan.new.cex");
//	puts("    ort *.cha");
	printf("+cF: homons table file name F (default: %s)\n", "0canhomo.cut");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	struct ccodes *t;

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = ORT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	if (isCUsed) {
		for (t=rc; t != NULL; t=t->next_code) {
			if (t->isMatched == 0) {
				fprintf(stdout, "Not Used Homons (%d): %s, %s, %s, %s", t->isMatched, t->name, t->oc, t->nc, t->line);
			}
		}
	}
	freeRC();
	freeRD();
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'c':
			isCUsed = TRUE;
			if (*f)
				uS.str2FNType(codesTableFile, 0L, f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static int foundCode(char *line, int pos, char *pat) {
	int len;

	if (strlen(line+pos) < strlen(pat))
		return(FALSE);
	else {
		len = pos;
		for (; *pat; pos++, pat++) {
			if (isSpace(line[pos]) || uS.isskip(line, pos, &dFnt, MBF))
				break;
			if (line[pos] != *pat)
				break;
		}
		if (*pat == EOS) {
			return(pos-len);
		} else
			return(FALSE);
	}
}

static int changeCode(char *line, AttTYPE *att, int pos, int wlen, char *from_word, char *to_word) {
	register int i;
	register int tlen;

	tlen = strlen(to_word);
	if (tlen > wlen) {
		int num;
		num = tlen - wlen;
		for (i=strlen(line); i >= pos; i--) {
			line[i+num] = line[i];
			att[i+num] = att[i];
		}
	} else if (tlen < wlen) {
		if (*to_word == EOS)
			att_cp(0, line+pos+tlen, line+pos+wlen, att+pos+tlen, att+pos+wlen);
		else
			att_cp(0, line+pos+tlen-1, line+pos+wlen-1, att+pos+tlen-1, att+pos+wlen-1);
	}

	for (; *to_word != EOS; pos++) {
		line[pos] = *to_word++;
		if (pos > 0)
			att[pos] = att[pos-1];
	}
	return(pos-1);
}	

static long FindAndChangeCode(char *line, AttTYPE *att, long isFound) {
	register int pos, z;
	struct ccodes *nextone;
	int wlen;
	unsigned char isExactMatch;

	pos = 0;
	while (!isalnum((unsigned char)line[pos]) && line[pos] != EOS)
		pos++;
	nextone = rc;
	isExactMatch = 0;
	if (chatmode == 0) {
		for (z=pos; isalpha(line[z]); z++) {
			if (isupper((unsigned char)line[z])) {
				isExactMatch = isExactMatch | 0x01;
				break;
			}
		}
	}
	while (nextone != NULL) {
		if ((wlen=foundCode(line,pos,nextone->oc))) {
			isFound++;
			isExactMatch = isExactMatch | 0x10;
			nextone->isMatched++;
			pos = changeCode(line,att,pos,wlen,nextone->oc,nextone->nc);
			break;
		}
		nextone = nextone->next_code;
	}
	while (isalpha(line[pos]))
		pos++;
	while (isdigit(line[pos]) || line[pos] == '_')
		pos++;

	if (chatmode == 0 && isExactMatch == 0x01)
		fprintf(stdout, "Missing Dict from Homons (%d): %s", isExactMatch, line);

	while (line[pos] != EOS) {
		if (chatmode == 0 && line[pos] == '\t')
			break;
		while (!isalnum((unsigned char)line[pos]) && line[pos] != EOS)
			pos++;
		if (line[pos] == EOS)
			break;
		nextone = rc;
		isExactMatch = 0;
		if (chatmode == 0) {
			for (z=pos; isalpha(line[z]); z++) {
				if (isupper((unsigned char)line[z])) {
					isExactMatch = isExactMatch | 0x01;
					break;
				}
			}
		}
		while (nextone != NULL) {
			if ((wlen=foundCode(line,pos,nextone->oc))) {
				isFound++;
				isExactMatch = isExactMatch | 0x10;
				nextone->isMatched++;
				pos = changeCode(line,att,pos,wlen,nextone->oc,nextone->nc);
				break;
			}
			nextone = nextone->next_code;
		}
		while (isalpha(line[pos]))
			pos++;
		while (isdigit(line[pos]) || line[pos] == '_')
			pos++;
		if (chatmode == 0 && isExactMatch == 0x01)
			fprintf(stdout, "Missing Dict from Homons (%d): %s", isExactMatch, line);
	}
	return(isFound);
}

static const char *FindSymForWord(char *word) {
	struct cdicts *t;

	for (t=rd; t != NULL; t=t->next_word) {
		if (!strcmp(word, t->word)) {
			if (t->sym[0] == EOS)
				return(t->word);
			else
				return(t->sym);
		}
	}
	if (isalpha(*word)) {
		if (strchr(word, '@')  || isupper((unsigned char)*word) || 
							!strcmp(word, "xxx") || !strcmp(word, "xx") ||
							!strcmp(word, "yyy") || !strcmp(word, "yy") ||
							!strcmp(word, "www"))
			return(word);
		else
			return("**");
	} else
		return(word);
}

static void CreatORTTier(void) {
	int i, j, k;
	const char *repl;

	j = 0;
	for (i=0; uttline[i] != EOS; ) {
		if (uS.isskip(uttline,i,&dFnt,MBF)) {
			templineC2[j++] = utterance->line[i++];
		} else {
			k = 0;
			for (; !uS.isskip(uttline,i,&dFnt,MBF) && uttline[i] != EOS; i++) {
				templineC1[k++] = uttline[i];
			}
			templineC1[k] = EOS;
			repl = FindSymForWord(templineC1);
			templineC2[j] = EOS;
			for (j=0; isSpace(repl[j]); j++) ;
			strcat(templineC2, repl+j);
			j = strlen(templineC2);
			for (; !uS.isskip(utterance->line,i,&dFnt,MBF); i++) ;
		}
	}
	templineC2[j] = EOS;
	uS.remblanks(templineC2);
	if (templineC2[0] != EOS)
		printout("%ort:	", templineC2, NULL, NULL, FALSE);
}

void call() {
	long isFound = 0L;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (isCUsed) {
			if (utterance->speaker[0] == '*' || uS.partcmp(utterance->speaker, "%mor", FALSE, TRUE) || chatmode == 0) {
				if (isCUsed) {
					isFound = FindAndChangeCode(utterance->line, utterance->attLine, isFound);
				}
				printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
			} else {
				printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
			}
		} else {
			if (utterance->speaker[0] == '*') {
				printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
				CreatORTTier();
			} else if (uS.partcmp(utterance->speaker, "%ort:", FALSE, TRUE)) ;
			else
				printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
		}
	}
	if (isCUsed) {
		if (isFound == 0L && fpout != stdout && !stout) {
			fprintf(stderr,"**- NO changes made in this file\n");
		} else if (isFound > 0L)
			fprintf(stderr,"**+ %ld changes made in this file\n", isFound);
	}
}
