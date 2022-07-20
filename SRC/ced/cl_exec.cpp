#include "ced.h"
#include "cu.h"
#include "mul.h"
#ifdef _COCOA_APP
#import "ListBoxController.h"
#endif
#ifdef _WIN32
	#include "w95_commands.h"
#endif

#define AL_COM		"al"
#define ALIAS_COM	"alias"
#define ALIAS_USAGE "#$%^aliases_commandusage#$%^"
#define BAT_COM		"bat"
#define BATCH_COM	"batch"
#define DIR_COM		"dir"
#define LIST_COM	"list"
#define RM_COM		"rm"
#define DEL_COM		"del"
#define CD_COM		"cd"
#define REN_COM		"ren"
#define RENAME_COM	"rename"
#define COPY_COM	"copy"
#define INFO_COM	"info"
#define QMARK_COM	"?"
#define HELP_COM	"help"
#define TY_COM		"ty"
#define TYPE_COM	"type"
#define OPEN_COM	"open"

#ifdef _MAC_CODE
	#include <sys/stat.h>
	#include <dirent.h>
#endif // _MAC_CODE

struct exec_FileList {
	FNType fname[FNSize];
	struct exec_FileList *next_file;
} ;
typedef struct exec_FileList exec_FileList;

struct fopts {
	char query;
	char force;
	char set_creator; 
	char set_type; 
	char set_lowcase;
	char set_uppercase;
	char show_not_matched;
	char isRecursive;
	FNType path[FNSize];
	FNType arg1[FNSize];
	FNType arg2[FNSize];
	int  dir_numdirs;
	int  dir_numfiles;
	long size;
	exec_FileList *root_file;
	creator_type temp_creator, 
				 temp_type;
} ;
typedef struct fopts fopts;

extern int F_numfiles;

#ifndef _COCOA_APP
  #ifdef _MAC_CODE
	char *fbuffer = NULL;
  #endif // _MAC_CODE
#endif
#ifdef _WIN32
	static CFileFind fileFind;
#endif /* _WIN32 */

char *alias_com;
FNType openFileName[FNSize];

int  cl_argc;
char *cl_argv[MAX_ARGS];
static UInt32 aliasDFileDateValue = 0L, aliasUFileDateValue = 0L;
static struct redirects pipe1, pipe2;
ALIASES_LIST *aliases = NULL;

struct redirects *pipe_out, *pipe_in;
struct redirects redirect_out, redirect_in;

#ifndef _COCOA_APP
char init_clan(void) {
  #ifdef _MAC_CODE
	if (fbuffer != NULL)
		return(TRUE);
	fbuffer = (char *)malloc(EXPANSION_SIZE + 2L);
	if (fbuffer == NULL) {
		do_warning("Can't run CLAN; Out of memory", 0);
		return(FALSE);
	}
  #endif // _MAC_CODE
	return(TRUE);
}
#endif
/*
 //	if (!findInLib(ALIAS_FILE_D, openFileName))

static char findInLib(FNType *fname, FNType *mFileName) {
	int  t;
#if defined(_MAC_CODE)
	struct dirent *dp;
	struct stat sb;
	DIR *cDIR;
#endif // _MAC_CODE
#ifdef _WIN32
	BOOL notDone;
	CString dirname;
	CFileFind fileFind;
	FNType tFileName[FILENAME_MAX];
#endif // _WIN32

	mFileName[0] = EOS;
	if (!isRefEQZero(lib_dir)) {	// we have a lib
		strcpy(mFileName,lib_dir);
		t = strlen(mFileName);
		addFilename2Path(mFileName, fname);
		if (!access(mFileName,0))
			return(TRUE);
#if defined(_MAC_CODE)
		SetNewVol(lib_dir);
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
				mFileName[t] = EOS;
				addFilename2Path(mFileName, dp->d_name);
				addFilename2Path(mFileName, fname);
				if (!access(mFileName,0)) {
					closedir(cDIR);
					if (WD_Not_Eq_OD)
						SetNewVol(od_dir);
					else
						SetNewVol(wd_dir);
					return(TRUE);
				}
			}
			closedir(cDIR);
		}
		if (WD_Not_Eq_OD)
			SetNewVol(od_dir);
		else
			SetNewVol(wd_dir);
#endif // _MAC_CODE
#ifdef _WIN32
		SetNewVol(lib_dir);
		if (!fileFind.FindFile(_T("*.*"), 0)) {
			fileFind.Close();
		} else {
			do {
				notDone = fileFind.FindNextFile();
				dirname = fileFind.GetFileName();
				if (!fileFind.IsDirectory())
					continue;
				dirname = fileFind.GetFileName();
				if (!strcmp(dirname, ".") || !strcmp(dirname, ".."))
					continue;
				mFileName[t] = EOS;
				u_strcpy(tFileName, dirname, FILENAME_MAX);
				addFilename2Path(mFileName, tFileName);
				addFilename2Path(mFileName, fname);
				if (!access(mFileName,0)) {
					fileFind.Close();
					if (WD_Not_Eq_OD)
						SetNewVol(od_dir);
					else
						SetNewVol(wd_dir);
					return(TRUE);
				}
			} while (notDone) ;
			fileFind.Close();
		}
		if (WD_Not_Eq_OD)
			SetNewVol(od_dir);
		else
			SetNewVol(wd_dir);
#endif // _WIN32
		strcpy(mFileName,lib_dir);
		addFilename2Path(mFileName, fname);
	}
	return(FALSE);
}
*/
void free_aliases(void) {
	ALIASES_LIST *t;

	while (aliases != NULL) {
		t = aliases;
		aliases = aliases->next_alias;
		if (t->alias != NULL)
			free(t->alias);
		if (t->argv0 != NULL)
			free(t->argv0);
		if (t->rest != NULL)
			free(t->rest);
		free(t);
	}
}

static void createAliasesList(FILE *fp, char loc, char *Dfname, char *Ufname, char isInit) {
	char isPullDownCommand, isNeedsArgs;
	int argv0, rest, ln;
	ALIASES_LIST *newP, *p;

	ln = 0;
	isNeedsArgs = 0;
	isPullDownCommand = 0;
	while (fgets_ced(ced_lineC, UTTLINELEN, fp, NULL)) {
		if (uS.isUTF8(ced_lineC) || uS.isInvisibleHeader(ced_lineC))
			continue;
		ln++;
		uS.remFrontAndBackBlanks(ced_lineC);
		if (strcmp(ced_lineC, "%pull-down-command") == 0)
			isPullDownCommand = 1;
		if (strcmp(ced_lineC, "%argument-needed") == 0)
			isNeedsArgs = 1;
		if (ced_lineC[0] == EOS || ced_lineC[0] == '%' || ced_lineC[0] == '#')
			continue;
		for (argv0=0; ced_lineC[argv0] != EOS && ced_lineC[argv0] != ' ' && ced_lineC[argv0] != '\t'; argv0++)
			ced_lineC[argv0] = (char)tolower((unsigned char)ced_lineC[argv0]);
		if (ced_lineC[argv0] == EOS) {
			return;
		}
		ced_lineC[argv0] = EOS;
		for (argv0++; ced_lineC[argv0] == ' ' || ced_lineC[argv0] == '\t'; argv0++) ;
		if (ced_lineC[argv0] == EOS) {
			return;
		}
		for (rest=argv0; ced_lineC[rest] != EOS && ced_lineC[rest] != ' ' && ced_lineC[rest] != '\t'; rest++)
			ced_lineC[rest] = (char)tolower((unsigned char)ced_lineC[rest]);
		if (ced_lineC[rest] != EOS) {
			ced_lineC[rest] = EOS;
			for (rest++; ced_lineC[rest] == ' ' || ced_lineC[rest] == '\t'; rest++) ;
		}
		newP = NEW(ALIASES_LIST);
		if (newP == NULL) {
			return;
		}
		newP->next_alias = NULL;
		newP->isPullDownC = isPullDownCommand;
		newP->isNeedsArgs = isNeedsArgs;
		newP->loc = loc;
		newP->ln  = ln;
		newP->alias = (char *)malloc(strlen(ced_lineC)+1);
		if (newP->alias == NULL) {
			free(newP);
			return;
		}
		strcpy(newP->alias, ced_lineC);
		uS.remblanks(newP->alias);
		newP->argv0 = (char *)malloc(strlen(ced_lineC+argv0)+1);
		if (newP->argv0 == NULL) {
			free(newP->alias);
			free(newP);
			return;
		}
		strcpy(newP->argv0, ced_lineC+argv0);
		uS.remblanks(newP->argv0);
		newP->rest = (char *)malloc(strlen(ced_lineC+rest)+1);
		if (newP->rest == NULL) {
			free(newP->alias);
			free(newP->argv0);
			free(newP);
			return;
		}
		strcpy(newP->rest, ced_lineC+rest);
		uS.remblanks(newP->rest);
		isNeedsArgs = 0;
		isPullDownCommand = 0;
		if  (isInit == 2)
			fprintf(stdout,"%s  =  %s %s\n", newP->alias, newP->argv0, newP->rest);
		if (aliases == NULL) {
			aliases = newP;
		} else {
			for (p=aliases; p->next_alias != NULL; p=p->next_alias) {
				if (isInit != 1 && strcmp(p->alias, newP->alias) == 0) {
					fputs("----------------------------------------\n",stderr);
					if (p->loc == 'd')
						fprintf(stderr,"*** File \"%s\": line %ld.\n", Dfname, p->ln);
					else
						fprintf(stderr,"*** File \"%s\": line %ld.\n", Ufname, p->ln);
					if (newP->loc == 'd')
						fprintf(stderr,"*** File \"%s\": line %ld.\n", Dfname, newP->ln);
					else
						fprintf(stderr,"*** File \"%s\": line %ld.\n", Ufname, newP->ln);
					fprintf(stderr, "    ERROR: Found the same aliase name \"%s\" define twice\n", p->alias);
					fputs("----------------------------------------\n",stderr);
				}
			}
			if (isInit != 1 && strcmp(p->alias, newP->alias) == 0) {
				fputs("----------------------------------------\n",stderr);
				if (p->loc == 'd')
					fprintf(stderr,"*** File \"%s\": line %ld.\n", Dfname, p->ln);
				else
					fprintf(stderr,"*** File \"%s\": line %ld.\n", Ufname, p->ln);
				if (newP->loc == 'd')
					fprintf(stderr,"*** File \"%s\": line %ld.\n", Dfname, newP->ln);
				else
					fprintf(stderr,"*** File \"%s\": line %ld.\n", Ufname, newP->ln);
				fprintf(stderr, "    ERROR: Found the same aliase name \"%s\" define twice\n", p->alias);
				fputs("----------------------------------------\n",stderr);
			}
			p->next_alias = newP;
		}
	}
}

void readAliases(char isInit) {
	char isD, isU;
	UInt32 dateDValue = 0L, dateUValue = 0L;
	FILE *fp;

	isD = FALSE;
	isU = FALSE;
	strcpy(FileName1, lib_dir);
	addFilename2Path(FileName1, ALIAS_FILE_U);
	if (!access(FileName1,0))
		isU = TRUE;
	strcpy(openFileName, lib_dir);
	addFilename2Path(openFileName, "fixes");
	addFilename2Path(openFileName, ALIAS_FILE_D);
	if (!access(openFileName,0))
		isD = TRUE;
	if (!isD && !isU) {
		aliasDFileDateValue = 0L;
		aliasUFileDateValue = 0L;
		if (isInit == 0)
			free_aliases();
		return;
	}
	if (isInit != 0) {
		aliases = NULL;
		if (isD)
			getFileDate(openFileName, &aliasDFileDateValue);
		else
			aliasDFileDateValue = 0L;
		if (isU)
			getFileDate(FileName1, &aliasUFileDateValue);
		else
			aliasUFileDateValue = 0L;
	} else {
		if (isD && isU) {
			getFileDate(openFileName, &dateDValue);
			getFileDate(FileName1, &dateUValue);
			if (aliasDFileDateValue != 0L && dateDValue != 0L && aliasDFileDateValue == dateDValue &&
				aliasUFileDateValue != 0L && dateUValue != 0L && aliasUFileDateValue == dateUValue) {
				return;
			}
			aliasDFileDateValue = dateDValue;
			aliasUFileDateValue = dateUValue;
		} else if (isD) {
			getFileDate(openFileName, &dateDValue);
			if (aliasDFileDateValue != 0L && dateDValue != 0L && aliasDFileDateValue == dateDValue) {
				if (aliasUFileDateValue == 0L)
					return;
				else
					aliasUFileDateValue = 0L;
			}
			aliasDFileDateValue = dateDValue;
		} else if (isU) {
			getFileDate(FileName1, &dateUValue);
			if (aliasUFileDateValue != 0L && dateUValue != 0L && aliasUFileDateValue == dateUValue) {
				if (aliasDFileDateValue == 0L)
					return;
				else
					aliasDFileDateValue = 0L;
			}
			aliasUFileDateValue = dateUValue;
		}
		free_aliases();
	}
	if (isU) {
		if ((fp=fopen(FileName1, "r")) == NULL)
			return;
		createAliasesList(fp, 'u', openFileName, FileName1, isInit);
		fclose(fp);
	}
	if (isD) {
		if ((fp=fopen(openFileName, "r")) == NULL)
			return;
		createAliasesList(fp, 'd', openFileName, FileName1, isInit);
		fclose(fp);
	}
}

static void alias(void) {
	char isD, isU;
#ifdef _WIN32
	extern char *nameOverride, *pathOverride;
	extern char tFileBuf[];
#endif /* _WIN32 */

	isD = FALSE;
	isU = FALSE;
	strcpy(FileName1, lib_dir);
	addFilename2Path(FileName1, ALIAS_FILE_U);
	if (!access(FileName1,0))
		isU = TRUE;
	strcpy(openFileName, lib_dir);
	addFilename2Path(openFileName, "fixes");
	addFilename2Path(openFileName, ALIAS_FILE_D);
	if (!access(openFileName,0))
		isD = TRUE;
	if (!isD && !isU) {
		fprintf(stderr, "Can't open either one of aliases file:\n\t\"%s\" or \"%s\"\n", openFileName, FileName1);
		return;
	}
	if (cl_argc < 2) {
		free_aliases();
		fprintf(stderr,"Aliases list:\n\n");
		readAliases(2);
		fprintf(stderr,"\nadditional usage: al(ias) [d or u].\n");
		fprintf(stderr,"d: opens default \"CLAN/lib/fixes/0aliases.cut\" file.\n");
		fprintf(stderr,"u: opens user's \"CLAN/lib/aliases.cut\" file.\n");
	} else if (cl_argc >= 2) {
		if (isD && isU) {
			if (cl_argv[1][0] == 'd' || cl_argv[1][0] == 'D') {
			} else if (cl_argv[1][0] == 'u' || cl_argv[1][0] == 'U') {
				strcpy(openFileName, FileName1);
			} else {
				fprintf(stderr,"\nUsage: al(ias) [d or u].\n");
				fprintf(stderr,"d: opens default \"CLAN/lib/fixes/0aliases.cut\" file.\n");
				fprintf(stderr,"u: opens user's \"CLAN/lib/aliases.cut\" file.\n");
				fprintf(stderr,"\nTo get usage of an alias type that alias name in Commands window\n");
				return;
			}
		} else if (isU) {
			strcpy(openFileName, FileName1);
		}
#ifdef _MAC_CODE
#ifndef _COCOA_APP
		isAjustCursor = FALSE;
		OpenAnyFile(openFileName, 1962, TRUE);
#endif
#endif // _MAC_CODE
#ifdef _WIN32
		strcpy(tFileBuf, ALIAS_FILE_D);
		nameOverride = tFileBuf;
		extractPath(DirPathName, openFileName);
		pathOverride = DirPathName;
		::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW, NULL);
#endif // _WIN32
	}
}

char getAliasProgName(char *inBuf, char *progName, int size) {
	ALIASES_LIST *p;

	progName[0] = EOS;
	for (p=aliases; p != NULL; p=p->next_alias) {
		if (strcmp(inBuf, p->alias) == 0) {
			strncpy(progName, p->argv0, size);
			progName[size] = EOS;
			return(TRUE);
		}
	}
	return(FALSE);
}

static void displayAliasUsage(ALIASES_LIST *p, char *str) {
	int  len;
	FILE *fp;

	if (p->loc == 'd') {
		strcpy(FileName1, lib_dir);
		addFilename2Path(FileName1, "fixes");
		addFilename2Path(FileName1, ALIAS_FILE_D);
	} else {
		strcpy(FileName1, lib_dir);
		addFilename2Path(FileName1, ALIAS_FILE_U);
	}
	fp = fopen(FileName1, "r");
	if (fp == NULL) {
		fprintf(stdout,"%s=%s %s\n", p->alias, p->argv0, p->rest);
		return;
	}
	len = strlen(p->alias);
	str[0] = EOS;
	while (fgets_ced(ced_lineC, UTTLINELEN, fp, NULL)) {
		uS.remFrontAndBackBlanks(ced_lineC);
		if (uS.isUTF8(ced_lineC) || uS.isInvisibleHeader(ced_lineC) ||
			ced_lineC[0] == EOS || ced_lineC[0] == '%')
			continue;
		if (uS.mStrnicmp(ced_lineC, p->alias, len) == 0) {
			fprintf(stdout,"%s", str);
			fprintf(stdout,"%s=%s %s\n", p->alias, p->argv0, p->rest);
			return;
		}
		if (ced_lineC[0] == '#') {
			strcat(str, ced_lineC);
			strcat(str, "\n");
		} else
			str[0] = EOS;
	}
	fclose(fp);
}

static char *matchAlias(char *inputBuf) {
	int  k, c;
	char *com, alias_st[256+1];
	ALIASES_LIST *p;

	if (inputBuf == NULL)
		return(NULL);
	com = inputBuf;
	k = 0;
	for (c=0; com[c] != EOS && com[c] != ' ' && com[c] != '\t'; c++)  {
		if (k < 256)
			alias_st[k++] = (char)tolower((unsigned char)com[c]);
	}
	alias_st[k] = EOS;
	for (p=aliases; p != NULL; p=p->next_alias) {
		if (strcmp(alias_st, p->alias) == 0) {
			alias_com = (char *)malloc(EXPANSION_SIZE + 2L);
			if (alias_com == NULL)
				return(NULL);
			for (; com[c] == ' ' || com[c] == '\t'; c++)  ;
			if ((com[c] == EOS && p->isNeedsArgs) || (com[c] == '?' && !p->isNeedsArgs)) {
				strcpy(alias_com, ALIAS_USAGE);
				strcat(alias_com, " ");
				displayAliasUsage(p, alias_com+strlen(alias_com));
				return(alias_com);
			} else {
				strcpy(alias_com, p->argv0);
				strcat(alias_com, " ");
				strcat(alias_com, p->rest);
				k = strlen(alias_com);
				alias_com[k++] = ' ';
				while (com[c] != EOS)
					alias_com[k++] = com[c++];
				alias_com[k] = EOS;
				OutputToScreen(cl_T("ALIAS> "));
				fprintf(stderr, "%s\n", alias_com);
				return(alias_com);
			}
		}
	}
	return(NULL);
}

static char make_new_str(FNType *old_pat, FNType *old_s, FNType *new_pat, FNType *new_s) {
	int j, k;
	int n, m;
	int t, end;
	int starCnt;
	char isCopy, isNStarMatched, isNStarFound;

	isNStarMatched = FALSE;
	isNStarFound = FALSE;
	starCnt = 0;
	*new_s = EOS;
	if (old_s[0] == EOS) {
		fprintf(stderr, "empty original string specified.\n");
		return(FALSE);
	}
	for (j = 0, k = 0; old_pat[k]; j++, k++) {
/* 27-03-02
		if (old_pat[k] == '\\') {
			k++;
		} else if (old_pat[k] == '_') {
			while (*new_pat != '_' && *new_pat != '*' && *new_pat != EOS)
				*new_s++ = *new_pat++;
			if (*new_pat != EOS) {
				new_pat++;
				*new_s++ = old_s[j];
			}
		} else
*/
		if (old_pat[k] == '*') {	  /* wildcard */
			starCnt++;
			k++; t = j;
f1:
			while (old_s[j] && (islower((unsigned char)old_s[j]) ? (char)toupper((unsigned char)old_s[j]) : old_s[j]) != 
						(islower((unsigned char)old_pat[k]) ? (char)toupper((unsigned char)old_pat[k]) : old_pat[k])) j++;
			end = j;
			if (old_s[j]) {
	    		for (m=j+1, n=k+1; old_s[m] && old_pat[n]; m++, n++) {
					if (old_pat[n] == '*') break;
// 27-03-02					else if (old_pat[n] == '_') break;
					else if ((islower((unsigned char)old_s[m]) ? (char)toupper((unsigned char)old_s[m]) : old_s[m]) != 
					 (islower((unsigned char)old_pat[n]) ? (char)toupper((unsigned char)old_pat[n]) : old_pat[n])) {
		    			j++;
						goto f1;
					}
				}
				if (old_s[m] && !old_pat[n]) {
					j++;
					goto f1;
				}
			}
			isCopy = FALSE;
// 27-03-02			while (*new_pat != '_' && *new_pat != '*' && *new_pat != EOS)
			while (*new_pat != EOS) {
				if (*new_pat == '*') {
					isCopy = TRUE;
					break;
				}
				if (*new_pat == '\\' && isdigit(*(new_pat+1)) && *(new_pat+2) == '*') {
					isNStarFound = TRUE;
					if (atoi(new_pat+1) == starCnt) {
						isNStarMatched = TRUE;
						isCopy = TRUE;
						while (*new_pat == '\\' || isdigit(*new_pat))
							new_pat++;
					}
					break;
				}
				*new_s++ = *new_pat++;
			}
			if (isCopy) {
				new_pat++;
				while (t < end)
					*new_s++ = old_s[t++];
			}
			if (old_s[j] == EOS || old_pat[k] == EOS)
				break;
		}
	}
	while (*new_pat != EOS)
		*new_s++ = *new_pat++;
	*new_s = EOS;
	if (isNStarFound && !isNStarMatched) {
		fprintf(stderr, "Can't match replacement string wildcard number to original string wildcard.\n");
		return(FALSE);
	}
	return(TRUE);
}

static void set_creator_type(creator_type *cr_ptr, const char *s) {
	int i;

	cr_ptr->out = '    ';
	for (i=0; i < 4; i++, s++) {
	 	if (*s == EOS)
			break;
		cr_ptr->in[i] = *s;
	}
}

/* make cl_argv begin */
static void free_FileList(exec_FileList *p) {
	exec_FileList *t;

	while (p != NULL) {
		t = p;
		p = p->next_file;
		free(t);
	}
}

char *NextArg(char *com) {
	char *s, qf = FALSE;

	s = com;
	while (*s != EOS) {
		if ((*s == '\'' || *s == '"') && (s == com || *(s-1) != '\\')) {
			if (!qf) {
				qf = *s;
				strcpy(s, s+1);
			} else if (qf == *s) {
				qf = FALSE;
				strcpy(s, s+1);
			} else
				s++;
		} else if ((*s == ' ' ||  *s == '\t') && !qf) {
			break;
		} else
			s++;
 	}
 	if (qf) {
		if (qf == '\''/*'*/)
			do_warning("Matching ' character is missing. If this character is part of search string, please surround it with matching \"", 0);
		else
			do_warning("Matching \" character is missing. If this character is part of search string, please surround it with matching '", 0);
 		return(NULL);
 	}
 	if (*s != EOS) {
 		*s = EOS;
 		s++;
 	}
 	return(s);
}

char MakeArgs(char *com) {
	register char *endCom;

	while (*com != EOS) {
		for (; *com == ' ' || *com == '\t'; com++) ;
		if (*com == EOS)
			break;
		endCom = NextArg(com);
		if (endCom == NULL)
			return(FALSE);

		if (cl_argc >= MAX_ARGS) {
			do_warning("out of memory; Too many arguments.", 0);
			return(FALSE);
		}
		cl_argv[cl_argc++] = com;
		com = endCom;
	}
	return(TRUE);
}
/* make cl_argv end */

static char exec_addToFileList(fopts *args, const FNType *fname) {
	exec_FileList *tF, *t, *tt;
	extern char *MEMPROT;

	tF = NEW(exec_FileList);
	if (tF == NULL) {
#ifndef _COCOA_APP
		if (MEMPROT)
			free(MEMPROT);
#endif
		do_warning("Out of memory", 0);
		return(FALSE);
	}
	if (args->root_file == NULL) {
		args->root_file = tF;
		tF->next_file = NULL;
	} else if (strcmp(args->root_file->fname, fname) > 0) {
		tF->next_file = args->root_file;
		args->root_file = tF;
	} else {
		t = args->root_file;
		tt = args->root_file->next_file;
		while (tt != NULL) {
			if (strcmp(tt->fname, fname) > 0)
				break; 
			t = tt;
			tt = tt->next_file;
		}
		if (tt == NULL) {
			t->next_file = tF;
			tF->next_file = NULL;
		} else {
			tF->next_file = tt;
			t->next_file = tF;
		}
    }
	strcpy(tF->fname, fname);
	return(TRUE);
}

static char rec_dir(fopts *args, char isIncludeDirs, void(*func)(fopts *args)) {
	int i, index;
	FNType fname[FILENAME_MAX];
	int  dPos;
#if defined(UNX) || defined(_MAC_CODE)
	struct dirent *dp;
	struct stat sb;
	DIR *cDIR;
#endif
#ifdef _WIN32
	BOOL notDone;
	CFileFind dirFind;
	CString fnameFound;
#endif /* _WIN32 */

	dPos = strlen(args->path);
	if (SetNewVol(args->path))
		return(TRUE);
	free_FileList(args->root_file);
	args->root_file = NULL;
	index = 1;
	for (i=1; i < cl_argc; i++) {
		if (*cl_argv[i] == '-' || *cl_argv[i] == '+')
			continue;
		if (cl_argv[i][0] == PATHDELIMCHR) {
			if (!args->show_not_matched) {
				if (!exec_addToFileList(args, cl_argv[i]))
					return(FALSE);
			}
		}
	}
	while ((index=Get_File(fname, index)) != 0) {
		if (args->arg1[0] != EOS) {
			if (uS.fIpatmat(fname, args->arg1)) {
				if (!args->show_not_matched) {
					if (!exec_addToFileList(args, fname))
						return(FALSE);
				}
			} else if (args->show_not_matched) {
				if (!exec_addToFileList(args, fname))
					return(FALSE);
			}
		} else {
			for (i=1; i < cl_argc; i++) {
				if (*cl_argv[i] == '-' || *cl_argv[i] == '+' || cl_argv[i][0] == PATHDELIMCHR)
					continue;
				if (uS.fIpatmat(fname, cl_argv[i])) {
					if (!args->show_not_matched) {
						if (!exec_addToFileList(args, fname))
							return(FALSE);
					}
				} else if (args->show_not_matched) {
					if (!exec_addToFileList(args, fname))
							return(FALSE);
				}
			}
		}
	}

	if (isIncludeDirs) {
		index = 1;
		while ((index=Get_Dir(fname, index)) != 0) {
			if (args->arg1[0] != EOS) {
				if (uS.fIpatmat(fname, args->arg1)) {
					if (!args->show_not_matched) {
						if (!exec_addToFileList(args, fname))
							return(FALSE);
					}
				} else if (args->show_not_matched) {
					if (!exec_addToFileList(args, fname))
						return(FALSE);
				}
			} else {
				for (i=1; i < cl_argc; i++) {
					if (*cl_argv[i] == '-' || *cl_argv[i] == '+')
						continue;
					if (uS.fIpatmat(fname, cl_argv[i])) {
						if (!args->show_not_matched) {
							if (!exec_addToFileList(args, fname))
								return(FALSE);
						}
					} else if (args->show_not_matched) {
						if (!exec_addToFileList(args, fname))
							return(FALSE);
					}
				}
			}
		}
	}

	func(args);

	if (!args->isRecursive)
		return(TRUE);
#if defined(UNX) || defined(_MAC_CODE)
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
			addFilename2Path(args->path, dp->d_name);
			uS.str2FNType(args->path, strlen(args->path), PATHDELIMSTR);
 		 	if (!rec_dir(args,isIncludeDirs,func)) {
				args->path[dPos] = EOS;
				closedir(cDIR);
				return(FALSE);
			}
			args->path[dPos] = EOS;
			SetNewVol(args->path);
		}
		closedir(cDIR);
	}
#endif // _MAC_CODE
#ifdef _WIN32
	notDone = dirFind.FindFile(cl_T("*.*"), 0);
	while (notDone) {
		notDone = dirFind.FindNextFile();
		if (!dirFind.IsDirectory())
			continue;
		fnameFound = dirFind.GetFileName();
		if (!strcmp(fnameFound, ".") || !strcmp(fnameFound, ".."))
			continue;
		u_strcpy(fname, (unCH *)(LPCTSTR)fnameFound, FILENAME_MAX);
		strcat(args->path, fname);
		uS.str2FNType(args->path, strlen(args->path), PATHDELIMSTR);
		if (!rec_dir(args,isIncludeDirs,func)) {
			args->path[dPos] = EOS;
			dirFind.Close();
			return(FALSE);
		}
		args->path[dPos] = EOS;
	}
	dirFind.Close();
#endif // _WIN32
	return(TRUE);
}

/* copy file begin */
#define	COPY_SIZE 8192L	

static void copy_current_dir(fopts *args) {
	int i, temp, count;
	char *buf, isArgWild;
	FNType *hold, *after_dir;
	FNType new_name[FNSize];
	char message[FNSize*2+50]; 
	exec_FileList *tFile;
	FILE *fsrc, *fdest;
#ifdef _MAC_CODE
	creator_type original_type,original_creator;
#endif /* _MAC_CODE */

	if (args->root_file == NULL) {
		strcpy(new_name, args->path);
		addFilename2Path(new_name, args->arg1);
		sprintf(message, "Can't open file(s): %s.", new_name);
		do_warning(message, 0);
		return;
	}
	fdest = NULL;
	if ((buf=(char *)malloc(COPY_SIZE)) == NULL) {
		do_warning("Out of memory.", 0);
		return;
	}
	isArgWild = FALSE;
	for (i=0; args->arg2[i] != EOS; i++) {
		if (args->arg2[i] == '*') {
			isArgWild = TRUE;
			break;
		}
	}
	if (!isArgWild) {
		if (args->root_file == NULL) {
			free(buf);
			return;
		}

		strcpy(new_name, args->arg2);
		if (args->set_lowcase) {
			hold = strrchr(new_name,PATHDELIMCHR);
			if (hold) hold++;
			else hold = new_name;
			uS.lowercasestr(hold, &dFnt, FALSE);
		} else if (args->set_uppercase) {
			hold = strrchr(new_name,PATHDELIMCHR);
			if (hold) hold++;
			else hold = new_name;
			uS.uppercasestr(hold, &dFnt, C_MBF);
		}

		if (args->query) {
			if (args->isRecursive) {
				strcpy(message, "Copy file: \"");
				strcat(message, args->path);
				addFilename2Path(message, args->root_file->fname);
				strcat(message, "\" to  \"");
				strcat(message, args->path);
				addFilename2Path(message, new_name);
				strcat(message, "\"? ");
			} else
				sprintf(message,"Copy file: \"%s\" to  \"%s\"? ", args->root_file->fname, new_name);
			
			temp = QueryDialog(message, 140);
			if (temp == -1) {
				free(buf);
				return;
			} else if (temp == 0) {
				free(buf);
				fprintf(stderr, "Copy aborted.\n");
				return;
			}
		}

		if (!uS.mStricmp(args->root_file->fname, new_name)) {
			free(buf);
			fprintf(stderr, "Can't copy file to itself.\n");
			strcpy(message, args->path);
			addFilename2Path(message, args->root_file->fname);
			fprintf(stderr, "    File \"%s\" not copied.\n", message);
			return;
		}

		fdest = fopen(new_name,"wb");
		if (fdest == NULL) {
			free(buf);
			strcpy(message, "Can't open file: ");
			strcat(message, args->path);
			addFilename2Path(message, new_name);
			do_warning(message, 0);
			return;
		}
	}

	count = 0;
	for (tFile=args->root_file; tFile != NULL; tFile=tFile->next_file) {
		if (isArgWild) {
			if ((after_dir=strrchr(tFile->fname,PATHDELIMCHR)) != NULL)
				after_dir++;
			else
				after_dir = tFile->fname;
			if (!make_new_str(args->arg1, after_dir, args->arg2, new_name)) {
				fprintf(stderr, "Copy aborted.\n");
				return;
			}
			if (args->set_lowcase) {
				hold = strrchr(new_name,PATHDELIMCHR);
				if (hold) hold++;
				else hold = new_name;
				uS.lowercasestr(hold, &dFnt, FALSE);
			} else if (args->set_uppercase) {
				hold = strrchr(new_name,PATHDELIMCHR);
				if (hold) hold++;
				else hold = new_name;
				uS.uppercasestr(hold, &dFnt, C_MBF);
			}

			if (args->query) {
				if (args->isRecursive) {
					strcpy(message, "Copy file: \"");
					strcat(message, args->path);
					addFilename2Path(message, tFile->fname);
					strcat(message, "\" to  \"");
					strcat(message, args->path);
					addFilename2Path(message, new_name);
					strcat(message, "\"? ");
				} else
					sprintf(message,"Copy file: \"%s\" to  \"%s\"? ", tFile->fname, new_name);

				temp = QueryDialog(message, 140);
				if (temp == -1)
					continue;
				else if (temp == 0) {
					free(buf);
					fprintf(stderr, "Copy aborted.\n");
					return;
				}
			}

			if (!uS.mStricmp(tFile->fname, new_name)) {
				fprintf(stderr, "Can't copy file to itself.\n");
				strcpy(message, args->path);
				addFilename2Path(message, tFile->fname);
				fprintf(stderr, "    File \"%s\" not copied.\n", message);
				continue;
			}
		}
		if ((fsrc=fopen(tFile->fname,"rb")) == NULL) {
			free(buf);
			if (!isArgWild)
				fclose(fdest);
			strcpy(message, "Can't open file: ");
			strcat(message, args->path);
			addFilename2Path(message, tFile->fname);
			do_warning(message, 0);
			return;
		}
		if (isArgWild) {
			fdest = fopen(new_name,"wb");
			if (fdest == NULL) {
				free(buf);
				fclose(fsrc);
				strcpy(message, "Can't open file: ");
				strcat(message, args->path);
				addFilename2Path(message, new_name);
				do_warning(message, 0);
				return;
			}
		}
		temp = 0L;
		while (TRUE) {
			temp = fwrite(buf,1L,fread(buf,1L,COPY_SIZE,fsrc),fdest);
			if (temp < COPY_SIZE)
				break;
		}

#ifdef _MAC_CODE
		original_type.out = 'TEXT';
#ifndef _COCOA_APP
		original_creator.out = '????';
#else
		original_creator.out = 0L;
#endif
		gettyp(tFile->fname, &original_type.out, &original_creator.out);
		if (!args->set_creator)
			args->temp_creator.out = original_creator.out;
		if (!args->set_type)
			args->temp_type.out = original_type.out;			
		settyp(new_name, args->temp_type.out, args->temp_creator.out, FALSE);
#endif /* _MAC_CODE */
		if (isArgWild) {
			if (fdest)
				fclose(fdest);
			fdest = NULL;
		}
		if (fsrc)
			fclose(fsrc);
		if (isArgWild || count == 0) {
			if (args->isRecursive) {
				strcpy(message, "Copied \"");
				strcat(message, args->path);
				addFilename2Path(message, tFile->fname);
				strcat(message, "\" to  \"");
				strcat(message, args->path);
				addFilename2Path(message, new_name);
				strcat(message, "\"");
				fprintf(stdout,"%s\n", message);
			} else
				fprintf(stdout,"Copied \"%s\" to \"%s\"\n", tFile->fname, new_name);
		} else {
			if (args->isRecursive) {
				strcpy(message, "Appended \"");
				strcat(message, args->path);
				addFilename2Path(message, tFile->fname);
				strcat(message, "\" to  \"");
				strcat(message, args->path);
				addFilename2Path(message, new_name);
				strcat(message, "\"");
				fprintf(stdout,"%s\n", message);
			} else
				fprintf(stdout,"Appended \"%s\" to \"%s\"\n", tFile->fname, new_name);
		}
		count++;
	}
	if (fdest)
		fclose(fdest);
	free(buf);
}


static void cpy(void) {
	int  i;
	fopts args;

	args.query = FALSE;
	args.show_not_matched = FALSE;
	args.set_creator = FALSE; 
	args.set_type = FALSE; 
	args.set_lowcase = FALSE;
	args.set_uppercase = FALSE;
	args.isRecursive = FALSE;
	args.temp_type.out = 0L;
	args.temp_creator.out = 0L; 
	args.arg1[0] = EOS;
	args.arg2[0] = EOS;
	for (i=1; i < cl_argc; i++) {
		if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='q') {
			args.query = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='r') {
			args.isRecursive = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='c') {			
			set_creator_type(&args.temp_creator, cl_argv[i]+2);
			args.set_creator = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='t') {
			set_creator_type(&args.temp_type, cl_argv[i]+2); 
			args.set_type = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='l') {
			args.set_lowcase = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='u') {
			args.set_uppercase = TRUE;	
		} else if (args.arg1[0] == EOS) {
			uS.str2FNType(args.arg1, 0L, cl_argv[i]);
		} else 
			uS.str2FNType(args.arg2, 0L, cl_argv[i]);
	}

	if (args.arg1[0] == EOS || args.arg2[0] == EOS) {
		fprintf(stderr,"Usage copy [-luctqr] oldfile newfile\n");
		fprintf(stderr,"-l changes filenames to lowercase\n");
		fprintf(stderr,"-u changes filenames to uppercase\n");
#ifdef _MAC_CODE
		fprintf(stderr,"-cMSWD changes creator to MSWD, etc.\n");
		fprintf(stderr,"-tTEXT changes file type to TEXT\n");
#endif /* _MAC_CODE */
		fprintf(stderr,"-q verification\n");
		fprintf(stderr,"-r recursive\n");
		fprintf(stderr,"For example file name aki12_boofoo.cha:\n");
		fprintf(stderr,"    copy aki*_*.cha \\1*.cha\n");
		fprintf(stderr,"Will create file: 12.cha\n");
		fprintf(stderr,"    copy aki*_*.cha \\2*.cha\n");
		fprintf(stderr,"Will create to file: boofoo.cha\n");
		return;
	}
	if (args.isRecursive && (strrchr(args.arg1,PATHDELIMCHR) || strrchr(args.arg2,PATHDELIMCHR))) {
		fprintf(stderr,"Recursive copy to a non-original directory is not allowed.\n");
		return;
	}
	
	args.root_file = NULL;
	uS.lowercasestr(args.arg1, &dFnt, C_MBF);
	strcpy(args.path, wd_dir);
	rec_dir(&args,FALSE,copy_current_dir);
	free_FileList(args.root_file);
}
/* copy file end */

/* rename file begin */
int rename_each_file(FNType *s, FNType *t, char isChangeCase) {
	FNType old[FNSize], newf[FNSize];
	FNType *after_dir; 

	if ((after_dir = strrchr(s,PATHDELIMCHR)) != NULL) {
		strncpy(newf,s,after_dir - s + 1);
		newf[after_dir - s + 1] = EOS;
	} else	
		newf[0] = EOS;
		
	if ((after_dir = strrchr(t,PATHDELIMCHR)) != NULL)
		strcat(newf,after_dir + 1);
	else
		strcat(newf,t);

	strcpy(old,s);

	if (!isChangeCase && !access(newf,0))
		return(-1);
	if (rename(old, newf))
		return(-1);
	return(0);
}

static void ren_current_dir(fopts *args) {
	int temp, count;
	FNType *hold, *after_dir;
	FNType new_name[FNSize];
	char message[FNSize*2+50], isChangeCase;
	exec_FileList *tFile;

	isChangeCase = FALSE;
	if (args->isRecursive && !args->query)
		fprintf(stdout,"\nDirectory: %s\n", args->path);
	count = 0;
	for (tFile=args->root_file; tFile != NULL; tFile=tFile->next_file) {
		if ((after_dir=strrchr(tFile->fname,PATHDELIMCHR)) != NULL)
			after_dir++;
		else
			after_dir = tFile->fname;
		if (!make_new_str(args->arg1, after_dir, args->arg2, new_name)) {
			fprintf(stderr, "Rename aborted.\n");
			return;
		}
		if (args->set_lowcase) {
			hold = strrchr(new_name,PATHDELIMCHR);
			if (hold) hold++;
			else hold = new_name;
			uS.lowercasestr(hold, &dFnt, FALSE);
			isChangeCase = TRUE;
		} else if (args->set_uppercase) {
			hold = strrchr(new_name,PATHDELIMCHR);
			if (hold) hold++;
			else hold = new_name;
			uS.uppercasestr(hold, &dFnt, C_MBF);
			isChangeCase = TRUE;
		}

		if (args->query) {
			sprintf(message,"Rename file: \"%s\" to  \"%s\"? ", tFile->fname, new_name);
			temp = QueryDialog(message, 140);
			if (temp == -1) continue;
			else if (temp == 0) {
				fprintf(stderr, "Rename aborted.\n");
				return;
			}
		}
		if (args->force && uS.mStricmp(new_name, tFile->fname))
			unlink(new_name);

		if (rename_each_file(tFile->fname,new_name,isChangeCase)) {
			if (!args->force) {
				fprintf(stderr,"%s%c%s NOT RENAMED",args->path,PATHDELIMCHR,tFile->fname);
				fprintf(stderr, "\n    try using \"+f\" option to force rename.\n");
			} else
				fprintf(stderr,"Rename error: %s%s NOT RENAMED, perhaps it is locked\n",args->path,tFile->fname);
		} else {
			count++;
			args->dir_numfiles++;
#ifdef _MAC_CODE
			if (args->set_creator || args->set_type) 
				settyp(new_name, args->temp_type.out, args->temp_creator.out, FALSE);
#endif /* _MAC_CODE */
		}
	}
	fprintf(stderr, "%d file%s renamed.\n", count, ((count == 1) ? "" : "s"));
}

static void ren(void) {
	int  i;
	fopts args;

	args.query = FALSE;
	args.show_not_matched = FALSE;
	args.set_creator = FALSE; 
	args.set_type = FALSE; 
	args.set_lowcase = FALSE;
	args.set_uppercase = FALSE;
	args.isRecursive = FALSE;
	args.force = FALSE;
	args.temp_type.out = 0L;
	args.temp_creator.out = 0L;
	args.dir_numfiles = 0;
	args.arg1[0] = EOS;
	args.arg2[0] = EOS;
	for (i=1; i < cl_argc; i++) {
		if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='q') {
			args.query = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='r') {
			args.isRecursive = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='c') {			
			set_creator_type(&args.temp_creator, cl_argv[i]+2);
			args.set_creator = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='t') {
			set_creator_type(&args.temp_type, cl_argv[i]+2);
			args.set_type = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='l') {
			args.set_lowcase = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='u') {
			args.set_uppercase = TRUE;
		} else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='f') {
			args.force = TRUE;
		} else if (args.arg1[0] == EOS) {
			uS.str2FNType(args.arg1, 0L, cl_argv[i]);
		} else 
			uS.str2FNType(args.arg2, 0L, cl_argv[i]);
	}
	if (args.arg1[0] == EOS || args.arg2[0] == EOS) {
		fprintf(stderr,"Usage ren [-aluctqr] oldfile newfile\n");
		fprintf(stderr,"-l changes filenames to lowercase\n");
		fprintf(stderr,"-u changes filenames to uppercase\n");
		fprintf(stderr,"-f rename even if the new file does already exist\n");
#ifdef _MAC_CODE
		fprintf(stderr,"-cMSWD changes creator to MSWD, etc.\n");
		fprintf(stderr,"-tTEXT changes file type to TEXT\n");
#endif /* _MAC_CODE */
		fprintf(stderr,"-q verification\n");
		fprintf(stderr,"-r recursive\n");
		fprintf(stderr,"For example file name aki12_boofoo.cha:\n");
		fprintf(stderr,"    ren aki*_*.cha \\1*.cha\n");
		fprintf(stderr,"Will rename to file: 12.cha\n");
		fprintf(stderr,"    ren aki*_*.cha \\2*.cha\n");
		fprintf(stderr,"Will rename to file: boofoo.cha\n");
		return;
	}

	if (args.isRecursive && (strrchr(args.arg1,PATHDELIMCHR) || strrchr(args.arg2,PATHDELIMCHR))) {
		fprintf(stderr,"Recursive rename to a non-original directory is not allowed.\n");
		return;
	}
	if ((strrchr(args.arg1,'*') != NULL && strrchr(args.arg2,'*') == NULL) || (strrchr(args.arg1,'*') == NULL && strrchr(args.arg2,'*') != NULL)) {
		fprintf(stderr,"Both arguments must have '*' wildcard character(s).\n");
		return;
	}
	args.root_file = NULL;
	uS.lowercasestr(args.arg1, &dFnt, C_MBF);
	strcpy(args.path, wd_dir);
	rec_dir(&args,TRUE,ren_current_dir);
	free_FileList(args.root_file);
	if (args.isRecursive)
		fprintf(stderr, "\nTotal number of files renamed %d.\n", args.dir_numfiles);
}
/* rename file end */

/* show dir begin */
static void show_list(void) {
	int index, len, ncols, longest_name;
	FNType fname[FNSize];

	if (F_numfiles <= 0) {
		fprintf(stderr, "No files were selected to go with '@', \"File In\" button\n");
		return;
	}

	longest_name = 0;
	for (index=1; index <= F_numfiles; index++) {
		get_selected_file(index, fname, FNSize);
		if ((len=strlen(fname)) > longest_name)
			longest_name = len;
	}
	
	ncols = 80 / (longest_name + 3);
	if (!ncols)
		ncols = 1;

	len = 0;
	index = 1;
	for (index=1; index <= F_numfiles; index++) {
		get_selected_file(index, fname, FNSize);
		if (!(++len % ncols))
			fprintf(stdout,"%-*s\n", longest_name, fname);
		else
			fprintf(stdout,"%-*s   ", longest_name, fname);
	}

	if (len % ncols)
		fprintf(stdout,"\n");
}

static void cleanUpFName(FNType *name) {
	while (*name != EOS) {
		if (*name == ATTMARKER) {
			if (*(name+1) > 0 && *(name+1) < 32)
				strcpy(name, name+2);
			else
				strcpy(name, name+1);
		} else if (*name == '\n' || *name == '\r') {
			*name = '_';
		} else
			name++;
	}
}

static void show_current_dir(fopts *args) {
	FNType fname[FILENAME_MAX+1];
	int  i, index,longest_name,len, ncols, fcount, dcount;
	long size;
	exec_FileList *tFile;

	if (args->isRecursive)
		fprintf(stdout,"\nDirectory: %s\n", args->path);

	longest_name = 0;
	index = 1;
	while ((index=Get_Dir(fname,index)) != 0) {
		for (i=1; i < cl_argc; i++) {
			if ((cl_argv[i][0] == '-' || cl_argv[i][0] == '+') && 
				(cl_argv[i][1] == 'r' || cl_argv[i][1] == 'l')) continue;
			if (uS.fIpatmat(fname, cl_argv[i])) {
				if ((len=strlen(fname)+1) > longest_name)
					longest_name = len;
			}
		}
	}

	for (tFile=args->root_file; tFile != NULL; tFile=tFile->next_file) {
		if ((len=strlen(tFile->fname)) > longest_name)
			longest_name = len;
	}

	ncols = 80 / (longest_name + 3);
	if (!ncols)
		ncols = 1;
	size = 0L;
	fcount = 0;
	dcount = 0;
	len = 0;
	index = 1;
	while ((index=Get_Dir(fname,index)) != 0) {
		for (i=1; i < cl_argc; i++) {
			if ((cl_argv[i][0] == '-' || cl_argv[i][0] == '+') && 
				(cl_argv[i][1] == 'r' || cl_argv[i][1] == 'l')) continue;
			if (uS.fIpatmat(fname, cl_argv[i])) {
				uS.str2FNType(fname, strlen(fname), PATHDELIMSTR);
				args->dir_numdirs++;
				dcount++;
				cleanUpFName(fname);
				if (!(++len % ncols))
					fprintf(stdout,"%-*s\n", longest_name, fname);
				else
					fprintf(stdout,"%-*s  ", longest_name, fname);
			}
		}
	}

	for (tFile=args->root_file; tFile != NULL; tFile=tFile->next_file) {
		args->dir_numfiles++;
		fcount++;
		cleanUpFName(tFile->fname);
		if (!(++len % ncols))
			fprintf(stdout,"%-*s\n", longest_name, tFile->fname);
		else
			fprintf(stdout,"%-*s  ", longest_name, tFile->fname);
	}

	if (len % ncols)
		fprintf(stdout,"\n");
	if (args->isRecursive) {
		fprintf(stdout,"%d files, %d directories", fcount, dcount);
		fprintf(stdout,"\n");
	}
}

static void show_directory(void) {
	int i;
	char isArgGiven, wild_arg[3];
	fopts args;

	isArgGiven = FALSE;
	args.size = 0L;
	args.dir_numdirs = 0;
	args.dir_numfiles = 0;
	args.show_not_matched = FALSE;
	args.isRecursive = FALSE;
	args.arg1[0] = EOS;
	for (i=1; i < cl_argc; i++) {
		if ((cl_argv[i][0] == '-' || cl_argv[i][0] == '+') && cl_argv[i][1] == 'r')
			args.isRecursive = TRUE;
		else if ((cl_argv[i][0] == '-' || cl_argv[i][0] == '+') && cl_argv[i][1] == 'n')
			args.show_not_matched = TRUE;
		else {
//2013-11-22			uS.lowercasestr(cl_argv[i], &dFnt, C_MBF);
//2013-11-22			uS.str2FNType(args.arg1, 0L, cl_argv[i]);
			isArgGiven = TRUE;
			if (cl_argv[i][0] == '@' && cl_argv[i][1] == EOS) {
				uS.str2FNType(args.arg1, 0L, cl_argv[i]);
				show_list();
				cl_argc--;
//2013-11-22				cl_argv[i][0] = PATHDELIMCHR;
				args.dir_numfiles += F_numfiles;
			}
		}
	}

	if (!isArgGiven) {
		strcpy(wild_arg, "*");
		cl_argv[cl_argc] = wild_arg;
		cl_argc++;
	}

	args.root_file = NULL;
	strcpy(args.path, wd_dir);
	rec_dir(&args,FALSE,show_current_dir);
	free_FileList(args.root_file);

	if (args.dir_numfiles == 0 && args.dir_numdirs == 0) {
		fprintf(stderr,"\nNo matches were found.\n");
	} else {
		fprintf(stdout,"\n%d files, %d directories", args.dir_numfiles, args.dir_numdirs);
		fprintf(stdout,"\n");
	}
}
/* show dir end */

/* rm file begin */
static void rm_current_dir(fopts *args) {
	int temp, fcount;
	char message[FILENAME_MAX+50];
	exec_FileList *tFile;

	if (args->isRecursive && !args->query)
		fprintf(stdout,"\nDirectory: %s\n", args->path);

	fcount = 0;
	for (tFile=args->root_file; tFile != NULL; tFile=tFile->next_file) {
		if (args->query) {
			sprintf(message,"Delete file: \"%s\"? ", tFile->fname);
			temp = QueryDialog(message, 140);
			if (temp == -1) continue;
			else if (temp == 0) {
				fprintf(stderr, "Delete aborted.\n");
				return;
			}
		}
		if (unlink(tFile->fname))
			fprintf(stderr,"File delete error: %s NOT DELETED\n", tFile->fname);
		else {
			fcount++;
			args->dir_numfiles++;
		}
	}
	fprintf(stdout,"%d file%s deleted.\n", fcount, ((fcount == 1) ? "" : "s"));
}

static void rm(void) {
	int i;
	fopts args;

	if (cl_argc < 2) {
		fprintf(stderr,"Usage del [-qr] filename(s)\n");
		fprintf(stderr,"-q verification\n");
		fprintf(stderr,"-r recursive\n");
		return;
	}

	args.query = FALSE;
	args.show_not_matched = FALSE;
	args.isRecursive = FALSE;
	args.dir_numfiles = 0;
	args.arg1[0] = EOS;
	for (i=1; i < cl_argc; i++) {
		if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='q')
			args.query = TRUE;
		else if ((cl_argv[i][0]=='-' || cl_argv[i][0]=='+') && cl_argv[i][1]=='r')
			args.isRecursive = TRUE;
		else
			uS.lowercasestr(cl_argv[i], &dFnt, C_MBF);
	}

	args.root_file = NULL;
	strcpy(args.path, wd_dir);
	rec_dir(&args,FALSE,rm_current_dir);
	free_FileList(args.root_file);

	if (!args.dir_numfiles) {
		fprintf(stderr,"\nNo file(s) were found.\n");
	} else if (args.isRecursive) {
		fprintf(stdout,"\nTotal number of files deleted %d.\n", args.dir_numfiles);
	}
}
/* rm file end */

/* show info begin */
void show_info(char isJustComm) {
#ifndef _COCOA_APP
	long mysize;
#endif
	NewFontInfo finfo;

	finfo.fontName[0] = EOS;
	SetDefaultCAFinfo(&finfo);
	selectChoosenFont(&finfo, TRUE, TRUE);
	if (!isJustComm) {
#ifdef _MAC_CODE
#ifndef _COCOA_APP
		Str255 pFontName;
		char FontName[256];
#endif
		creator_type the_file_creator;

#ifndef _COCOA_APP
		MaxMem( (Size * ) &mysize);
  		mysize = FreeMem();
		mysize = (long) CompactMem((Size) mysize);
#endif

		the_file_creator.out = PROGCREATOR;
		fprintf(stdout,"Output File Creator is set to '%c%c%c%c'\n",
			the_file_creator.in[0], the_file_creator.in[1],
			the_file_creator.in[2], the_file_creator.in[3]);
#ifndef _COCOA_APP
		GetFontName(dFnt.fontId, pFontName);
		p2cstrcpy(FontName, pFontName);
		fprintf(stdout,"Screen Font is set to: %s %ld\n", FontName, dFnt.fontSize);
		fprintf(stdout,"Free memory %ld bytes\n", mysize);
#endif
#endif // _MAC_CODE
#ifdef _WIN32
		MEMORYSTATUS lpBuffer;

		GlobalMemoryStatus(&lpBuffer);
		mysize = (long)lpBuffer.dwAvailPhys;
		fprintf(stdout,"Screen Font is set to: %s %ld\n", dFnt.fontName, dFnt.fontSize);
		fprintf(stdout,"Free memory %ld bytes\n", mysize);
#endif /* _WIN32 */
	}
	ListAvailable(isJustComm);
	free_aliases();
	fprintf(stdout,"\n");
	fprintf(stderr,"Or any of the following aliases:\n");
	readAliases(2);
	fprintf(stdout,"\n");
	fprintf(stdout,"Supplementary commands available:\n");
	if (isJustComm == 2) {
		fprintf(stdout, "al(ias)   bat(ch)   cd        copy      del       dir\n");
		fprintf(stdout, "help [a]  info      list      open      ren(ame)  rm\n");
		fprintf(stdout, "ty(pe)    ? [a]\n");
	} else {
		fprintf(stdout, "al(ias)   bat(ch)   cd   dir   info   ren(ame)   rm\n");
	}
	if (!isJustComm) {
		fprintf(stdout,"\n");
		fprintf(stdout,"Your library directory is: %s\n", lib_dir);
		fprintf(stdout,"Your mor library directory is: %s\n", mor_lib_dir);
		fprintf(stdout,"Your working directory is: %s\n", wd_dir);
		fprintf(stdout,"Your output directory is:  %s\n", od_dir);
	}
}
/* show info end */

static void type(void) {
	int i, index;
	FNType fname[FNSize];
#ifdef _MAC_CODE
	FNType dummy[FNSize];
#endif // _MAC_CODE
#ifdef _WIN32
	extern char *nameOverride, *pathOverride;
	extern char tFileBuf[];
#endif /* _WIN32 */

	if (cl_argc < 2) {
		fprintf(stderr,"Specify file(s) to display.\n");
		return;
	}
	uS.lowercasestr(cl_argv[0], &dFnt, C_MBF);
	index = 1;
	while ((index=Get_File(fname, index)) != 0) {
		for (i=1;i<cl_argc;i++) {
			if (uS.fIpatmat(fname, cl_argv[i])) {
#ifdef _MAC_CODE
				strcpy(dummy, wd_dir);
				addFilename2Path(dummy, fname);
#ifndef _COCOA_APP
				isAjustCursor = TRUE;
				OpenAnyFile(dummy, 1962, TRUE);
#endif
#endif // _MAC_CODE
#ifdef _WIN32
				strcpy(tFileBuf, fname);
				nameOverride = tFileBuf;
				pathOverride = wd_dir;
				::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW, NULL);
				break;
#endif // _WIN32
			}
		}
	}
}

static void cd(FNType *new_dir) {
	FNType mDirPathName[FNSize];

#ifdef _MAC_CODE
	SetNewVol(wd_dir);
	if (chdir(new_dir)) {
		fprintf(stderr,"change directory error %s\n", new_dir);
	} else {
		getcwd(mDirPathName, FNSize);
		fprintf(stderr,"directory set to: %s\n", mDirPathName);
#ifdef _COCOA_APP
		SetWdLibFolder(mDirPathName, wdFolders);
#else
		if (!WD_Not_Eq_OD)
			strcpy(od_dir, mDirPathName);
		strcpy(wd_dir, mDirPathName);
		UpdateWindowNamed(Commands_str);
		WriteCedPreference();
#endif
	}
#endif // _MAC_CODE
#ifdef _WIN32	
	SetNewVol(wd_dir);
	if (my_chdir(new_dir)) {
		fprintf(stderr,"change directory error %s\n", new_dir);
	} else {
		strcpy(mDirPathName, wd_dir);
		my_getcwd(wd_dir, FNSize);
		strcat(wd_dir, PATHDELIMSTR);
		my_fprintf(stderr,"directory set to: %s\n",wd_dir);
		if (strcmp(od_dir, mDirPathName) == 0) {
			strcpy(od_dir, wd_dir);
		}
		
		if (clanDlg != NULL) {
			if (strcmp(od_dir, wd_dir) == 0)
				clanDlg->m_OdSt = "";
			else {
				u_strcpy(clanDlg->t_st, od_dir, FNSize);
				AdjustName(clanDlg->od_st, clanDlg->t_st, 39);
				clanDlg->m_OdSt = clanDlg->od_st;
			}
			u_strcpy(clanDlg->t_st, wd_dir, FNSize);
			AdjustName(clanDlg->wd_st, clanDlg->t_st, 39);
			clanDlg->m_WdSt = clanDlg->wd_st;
			clanDlg->UpdateData(FALSE);
		}
		WriteCedPreference();
	}
#endif // _WIN32
}

static void run_CLAN(char *inputBuf, char tDataChanged) {
	char *com;
#ifndef _COCOA_APP
#ifdef _MAC_CODE
	Size mysize;

	MaxMem( (Size * ) &mysize);
  	mysize = FreeMem();
	mysize = (long) CompactMem((Size) mysize);
#endif // _MAC_CODE
#endif
#ifdef _WIN32
	extern char *nameOverride, *pathOverride;
	extern char tFileBuf[];
#endif /* _WIN32 */
	alias_com = NULL;
	readAliases(0);
	com = matchAlias(inputBuf);
	if (com == NULL)
		com = inputBuf;
	openFileName[0] = EOS;
	cl_argc = 0;
	cl_argv[cl_argc++] = com;
	for (; *com != EOS && *com != ' ' && *com != '\t'; com++) {
		*com = (char)tolower((unsigned char)*com);
	}
	if (*com != EOS) {
		*com = EOS;
		com++;
	}
	if (!strcmp(cl_argv[0], ALIAS_USAGE)) {
	} else if (!strcmp(cl_argv[0], DIR_COM)) {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		show_directory();
	} else if (!strcmp(cl_argv[0], LIST_COM)) {
		show_list();
	} else if (!strcmp(cl_argv[0], DEL_COM) || !strcmp(cl_argv[0], RM_COM)) {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		rm();
	} else if (!strcmp(cl_argv[0], CD_COM)) {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		if (cl_argc == 1) {
			fprintf(stderr,"Please specify a new directory.\n");
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		cd(cl_argv[1]);
	} else if (!strcmp(cl_argv[0], REN_COM) || !strcmp(cl_argv[0], RENAME_COM)) {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		ren();
	} else if (!strcmp(cl_argv[0], COPY_COM)) {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		cpy();
	} else if (!strcmp(cl_argv[0], INFO_COM)) {
		show_info(0);
	} else if (!strcmp(cl_argv[0], QMARK_COM) || !strcmp(cl_argv[0], HELP_COM)) {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		if (cl_argc == 1) {
			show_info(1);
		} else {
			uS.remFrontAndBackBlanks(cl_argv[1]);
	 		if (*cl_argv[1] == 'A' || *cl_argv[1] == 'a') {
				show_info(2);
			} else {
				show_info(1);
			}
		}
	} else if (!strcmp(cl_argv[0], TY_COM) || !strcmp(cl_argv[0], TYPE_COM) || !strcmp(cl_argv[0], OPEN_COM)) {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		global_df->DataChanged = tDataChanged;
#ifndef _COCOA_APP
		PosAndDispl();
#endif
		type();
	} else if (!strcmp(cl_argv[0], BAT_COM) || !strcmp(cl_argv[0], BATCH_COM)) {
		fprintf(stderr, "Usage: bat(ch) filename [argument1, argument2, ..., argument9]\n");
		fprintf(stderr, "In batch files use arguments %%1, %%2, ..., %%9 on command lines to access those arguments\n");
		fprintf(stderr, "In batch files use arguments -%%1, -%%2, ..., -%%9 if those arguments are optional\n");
	} else if (!strcmp(cl_argv[0], ALIAS_COM) || !strcmp(cl_argv[0], AL_COM)) {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		alias();
	} else {
		if (!MakeArgs(com)) {
			if (alias_com != NULL)
				free(alias_com);
			return;
		}
		globinit();
		main_clan(cl_argc, cl_argv);
/*
		if (openFileName[0] != EOS) {
#ifdef _MAC_CODE
			isAjustCursor = TRUE;
			OpenAnyFile(openFileName, 1962, TRUE);
#endif // _MAC_CODE
#ifdef _WIN32
			strcpy(tFileBuf, openFileName);
			nameOverride = tFileBuf;
			pathOverride = wd_dir;
			::PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_COMMAND, ID_FILE_NEW, NULL);
#endif // _WIN32
		}
*/
	}
	if (alias_com != NULL)
		free(alias_com);
}

static char *FindPipe(char *com) {
	char *s, qf;
	struct redirects *t;

	t = pipe_in;
	pipe_in = pipe_out;
	pipe_out = t;
	if (pipe_in->fp != NULL) {
		fclose(pipe_in->fp);
		pipe_in->fp = fopen(pipe_in->fn, "r");
	}
	s = com;
	qf = FALSE;
	while (*s != EOS) {
		if (*s == '|') {
			if (s > com && *(s-1) != ',' && *(s-1) != '^' && *(s-1) != '~' && *(s-1) != '$' && *(s-1) != '@' && *(s-1) != '0' &&
				*(s-1) != 'm' &&  *(s-1) != 'g' && *(s-1) != 'l' &&  *(s-1) != '"' && *(s-1) != '\'' && !qf) {
				*s++ = EOS;
				while (*s == ' ' || *s == '\t' || *s == '|') 
					s++;
				uS.remFrontAndBackBlanks(com);
				if (pipe_out->fp != NULL)
					fclose(pipe_out->fp);
				pipe_out->fp = fopen(pipe_out->fn, "w");
				return(s);
			}
		} else if ((*s == '\'' || *s == '"') && (s == com || *(s-1) != '\\')) {
			if (!qf) {
				qf = *s;
			} else if (qf == *s) {
				qf = FALSE;
			}
		}
		s++;
 	}
 	if (qf) {
		if (qf == '\'')
			do_warning("Matching ' character is missing. If this character is part of search string, please surround it with matching \"", 0);
		else
			do_warning("Matching \" character is missing. If this character is part of search string, please surround it with matching '", 0);
 		return(NULL);
 	}
	if (pipe_out->fp != NULL) {
		fclose(pipe_out->fp);
		pipe_out->fp = NULL;
		unlink(pipe_out->fn);
	}
	return(s);
}

static char isSOption(char *s, char *com) {
	while (s != com && !isSpace(*s))
		s--;
	if (isSpace(*s))
		s++;
	if (*s != '+' && *s != '-' && *(s+1) != 's')
		return(FALSE);
	return(TRUE);
}

static char FindRedirect(char *com) {
	char *s, *t;
	char qf, append;

	if (redirect_in.fp != NULL)
		fclose(redirect_in.fp);
	if (redirect_out.fp != NULL)
		fclose(redirect_out.fp);
	redirect_in.fp  = NULL;
	redirect_out.fp = NULL;
	redirect_out.all = FALSE;
	s = com;
	qf = FALSE;
	while (*s != EOS) {
		if (*s == '<' && !qf) {
			if (isSOption(s, com)) {
				s++;
			} else {
				if (pipe_in->fp != NULL) {
					do_warning("Redirect input can't be used with pipe in.", 0);
					return(0);
				}
				if (redirect_in.fp != NULL) {
					do_warning("Only one redirect input can be specified.", 0);
					return(0);
				}
				*s++ = EOS;
				uS.remFrontAndBackBlanks(com);
				for (; *s == ' ' || *s == '\t'; s++) ;
				t = NextArg(s);
				if (WD_Not_Eq_OD)
					SetNewVol(wd_dir);
				uS.str2FNType(redirect_in.fn, 0L, s);
				redirect_in.fp = fopen(redirect_in.fn, "r");
				if (redirect_in.fp == NULL) {
					sprintf(templineC, "Can't open input file: %s.", redirect_in.fn);
					do_warning(templineC, 0);
					return(0);
				}
				s = t;
			}
		} else if (*s == '>' && !qf) {
			if (isSOption(s, com)) {
				s++;
			} else {
				if (pipe_out->fp != NULL) {
					do_warning("Redirect out can't be used with pipe out.", 0);
					return(0);
				}
				if (redirect_out.fp != NULL) {
					do_warning("Only one redirect output can be specified.", 0);
					return(0);
				}
				append = FALSE;
				*s++ = EOS;
				uS.remFrontAndBackBlanks(com);
				for (; *s == ' ' || *s == '\t'; s++) ;
				if (*s == '>') {
					append = TRUE;
					for (s++; *s == ' ' || *s == '\t'; s++) ;
				}
				if (*s == '&') {
					redirect_out.all = TRUE;
					for (s++; *s == ' ' || *s == '\t'; s++) ;
				}
				t = NextArg(s);
				if (WD_Not_Eq_OD)
					SetNewVol(od_dir);
				uS.str2FNType(redirect_out.fn, 0L, s);
				if (append)
					redirect_out.fp = fopen(redirect_out.fn, "a");
				else
					redirect_out.fp = fopen(redirect_out.fn, "w");
				if (redirect_out.fp == NULL) {
					sprintf(templineC, "Can't open output file: %s.", redirect_out.fn);
					do_warning(templineC, 0);
					return(0);
				}
#ifdef _MAC_CODE
#ifndef _COCOA_APP
				settyp(redirect_out.fn, 'TEXT', the_file_creator.out, FALSE);
#endif
#endif /* _MAC_CODE */
				s = t;
			}
		} else if ((*s == '\'' || *s == '"') && (s == com || *(s-1) != '\\')) {
			if (!qf) {
				qf = *s;
			} else if (qf == *s) {
				qf = FALSE;
			}
			s++;
		} else
			s++;
 	}
 	if (qf) {
		if (qf == '\'')
			do_warning("Matching ' character is missing. If this character is part of search string, please surround it with matching \"", 0);
		else
			do_warning("Matching \" character is missing. If this character is part of search string, please surround it with matching '", 0);
 		return(0);
 	} 	
	return(1);
}

void InitRedirects(void) {
	pipe1.fp = NULL;
	uS.str2FNType(pipe1.fn, 0L, "pipe1");
	pipe2.fp = NULL;
	uS.str2FNType(pipe2.fn, 0L, "pipe2");
	pipe_in  = &pipe1;
	pipe_out = &pipe2;
	redirect_in.fp  = NULL;
	redirect_out.fp = NULL;
}

static void filterInput(char *inputBuf) {
	long i;

	for (i=0L; inputBuf[i] != EOS; i++) {
		if (inputBuf[i] == (char)0xE2 && inputBuf[i+1] == (char)0x80 && inputBuf[i+2] == (char)0x93) {
			strcpy(inputBuf+i, inputBuf+i+2);
			inputBuf[i] = '-';
		}
/* 2019-07-17
		else if (inputBuf[i] == (char)0xE2 && inputBuf[i+1] == (char)0x80 && (inputBuf[i+2] == (char)0x9C || inputBuf[i+2] == (char)0x9D)) {
			strcpy(inputBuf+i, inputBuf+i+2);
			inputBuf[i] = '"';
		}
*/
	}
}

void execute(char *inputBuf, char tDataChanged) {
	char *endPipe;

	InitRedirects();
	filterInput(inputBuf);
	fprintf(stderr, "%s\n", inputBuf);
	do {
		SetNewVol(od_dir);
		if ((endPipe=FindPipe(inputBuf)) == NULL)
			break;
		if (!FindRedirect(inputBuf))
			break;
		run_CLAN(inputBuf, tDataChanged);
		if (isKillProgram)
			break;
		strcpy(inputBuf, endPipe);
	} while (*inputBuf != EOS) ;

	SetNewVol(od_dir);
	if (pipe_in->fp != NULL) {
		fclose(pipe_in->fp);
		unlink(pipe_in->fn);
	}
	if (pipe_out->fp != NULL) {
		fclose(pipe_out->fp);
		unlink(pipe_out->fn);
	}
	if (redirect_in.fp != NULL)
		fclose(redirect_in.fp);
	if (redirect_out.fp != NULL)
		fclose(redirect_out.fp);
}
