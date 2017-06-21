#include "cu.h"

#pragma export on

extern "C"
{
	extern char *executeClan(char *com, char *location);
}

#pragma export off

#define BUFFERINCS 		25000L
#define NUMBEROFFILES	5000
#define	MAX_SRV_ARGS	200L

static char *fileNames[NUMBEROFFILES+2];
static long fileNamesIndex;
static char *resultStr;
static long resultStrLen;
static long resultStrIndex;
static struct redirects pipe1, pipe2;
struct redirects *pipe_out, *pipe_in;
struct redirects redirect_in;

extern char appendStrToResultStr(const char *st);
extern void main_clan(int argc, char *argv[]);

/*
CFMutableStringRef resultStr;
extern CFMutableStringRef executeClan(char *com, char *location);
resultStr = CFStringCreateMutable(NULL, 0);


CFStringAppendCString(resultStr, "One ", kCFStringEncodingUTF8);
CFStringAppendCString(resultStr, "two", kCFStringEncodingUTF8);
// CFStringAppend(resultStr, CFSTR("One "));
// CFStringAppend(resultStr, CFSTR("two."));
*/

int rename_each_file(char *s, char *t, char force, myMacDirRef *dref) {
	rename( s, t );
	return(0);
}

char appendStrToResultStr(const char *st) {
	resultStrIndex += strlen(st);
	if (resultStrIndex >= resultStrLen) {
		char *t;

		while (1) {
			t = (char *)malloc(resultStrLen+BUFFERINCS+3);
			if (t == NULL)
				return(FALSE);
			resultStrLen += BUFFERINCS;
			if (resultStrIndex >= resultStrLen)
				free(t);
			else
				break;
		}
		strcpy(t, resultStr);
		free(resultStr);
		resultStr = t;
	}
	strcat(resultStr, st);
	return(TRUE);
}

static char *NextSrvArg(char *s) {
	char qf = FALSE;

	while (*s != EOS) {
		if (*s == '\'' /*'*/ || *s == '"') {
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
			fprintf(stderr, "Matching ' character is missing.\n");
		else
			fprintf(stderr, "Matching \" character is missing.\n");
 		return(NULL);
 	}
 	if (*s != EOS) {
 		*s = EOS;
 		s++;
 	}
 	return(s);
}

static int WildCard(char *w) {
	for (; *w; w++) {
		if (*w == '*')
			return(TRUE);
	}
	return(FALSE);
}

static char run_CLAN_Prog(char *com, int argc, char *argv[]) {
	char *endCom;

	argv[argc++] = com;
	for (; *com != EOS && *com != ' ' && *com != '\t'; com++) {
		*com = tolower(*com);
	}
	if (*com != EOS) {
		*com = EOS;
		com++;
	}

	while (*com != EOS) {
		for (; *com == ' ' || *com == '\t'; com++) ;
		if (*com == EOS)
			break;
		endCom = NextSrvArg(com);
		if (endCom == NULL)
			return(FALSE);

		if (argc >= MAX_SRV_ARGS) {
			fprintf(stderr, "out of memory; Too many arguments.");
			return(FALSE);
		}
		if (com[0] == '-' || com[0] == '+')
			argv[argc++] = com;
		else if (!WildCard(com))
			argv[argc++] = com;
		else {
			int  index;
			char tFName[FILENAME_MAX];

			index = 1;
			while (index=Get_File(tFName, NULL, index, 0L, 0L, FALSE)) {
				if (uS.fpatmat(tFName, com)) {
					if (fileNamesIndex >= NUMBEROFFILES) {
						fprintf(stderr, "Too many files.");
						return(FALSE);
					}
					fileNames[fileNamesIndex] = (char *)malloc(strlen(tFName)+1);
					if (fileNames[fileNamesIndex] == NULL) {
						fprintf(stderr, "out of memory; Too many files.");
						return(FALSE);
					}
					strcpy(fileNames[fileNamesIndex], tFName);
					argv[argc++] = fileNames[fileNamesIndex];
					fileNamesIndex++;
				}
			}
		}
		com = endCom;
	}

	globinit();

	main_clan(argc, argv);
	return(TRUE);
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
		if (*s == '|' && !qf) {
			*s++ = EOS;
			while (*s == ' ' || *s == '\t' || *s == '|') 
				s++;
			uS.remFrontAndBackBlanks(com);
			if (pipe_out->fp != NULL)
				fclose(pipe_out->fp);
			pipe_out->fp = fopen(pipe_out->fn, "w");
			return(s);
		} else if (*s == '\'' || *s == '"') {
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
			fprintf(stderr, "Matching ' character is missing.\n");
		else
			fprintf(stderr, "Matching \" character is missing.\n");
 		return(NULL);
 	}
	if (pipe_out->fp != NULL) {
		fclose(pipe_out->fp);
		pipe_out->fp = NULL;
		unlink(pipe_out->fn);
	}
	return(s);
}

static char FindRedirect(char *com) {
	char *s, qf, *t, append;

	if (redirect_in.fp != NULL)
		fclose(redirect_in.fp);
	redirect_in.fp  = NULL;
	s = com;
	qf = FALSE;
	while (*s != EOS) {
		if (*s == '<' && !qf) {
			if (pipe_in->fp != NULL) {
				fprintf(stderr, "Redirect input can't be used with pipe in.\n");
				return(0);
			}
			if (redirect_in.fp != NULL) {
				fprintf(stderr, "Only one redirect input can be specified.\n");
				return(0);
			}
			*s++ = EOS;
			uS.remFrontAndBackBlanks(com);
			for (; *s == ' ' || *s == '\t'; s++) ;
			t = NextSrvArg(s);
			redirect_in.fn = s;
			redirect_in.fp = fopen(redirect_in.fn, "r");
			if (redirect_in.fp == NULL) {
				fprintf(stderr, "Can't open input file: %s.\n", redirect_in.fn);
				return(0);
			}
			s = t;
		} else if (*s == '>' && !qf) {
			if (pipe_out->fp != NULL) {
				fprintf(stderr, "Redirect out can't be used with pipe out.\n");
				return(0);
			}
			append = FALSE;
			*s++ = EOS;
			uS.remFrontAndBackBlanks(com);
			for (; *s == ' ' || *s == '\t'; s++) ;
			if (*s == '&') {
				for (s++; *s == ' ' || *s == '\t'; s++) ;
			} else if (*s == '>') {
				append = TRUE;
				for (s++; *s == ' ' || *s == '\t'; s++) ;
			}
			t = NextSrvArg(s);
			s = t;
		} else if (*s == '\'' || *s == '"') {
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
			fprintf(stderr, "Matching ' character is missing.\n");
		else
			fprintf(stderr, "Matching \" character is missing.\n");
 		return(0);
 	} 	
	return(1);
}

char *executeClan(char *com, char *location) {
	int  argc;
	char **argv = NULL;
	char *endPipe;

	resultStrLen = BUFFERINCS;
	resultStrIndex = 0L;
	resultStr = (char *)malloc(resultStrLen+3);
	if (resultStr == NULL) {
		resultStr = (char *)malloc(25);
		if (resultStr == NULL)
			return(NULL);
		else {
			strcpy(resultStr, "Out of memory!\n");
			return(resultStr);
		}
	}

	if (*com == EOS) {
	    fprintf(stderr, "Usage: srv command_line\n");
		return(resultStr);
	}

	for (argc=0; argc < NUMBEROFFILES; argc++)
		fileNames[argc] = NULL;
	fileNamesIndex = 0L;

	argv = (char **) malloc(MAX_SRV_ARGS*sizeof(char *));
	if (argv == NULL) {
		fprintf(stderr, "Can't run CLAN; Out of memory\n");
		return(resultStr);
	}
	argc = 0;
	pipe1.fp = NULL;
	pipe1.fn = "pipe1";
	pipe2.fp = NULL;
	pipe2.fn = "pipe2";
	pipe_in  = &pipe1;
	pipe_out = &pipe2;
	redirect_in.fp  = NULL;
	if (chdir(location) != 0) {
		fprintf(stderr, "Error locating directory: %s;\n", location);
		free(argv);
		return(resultStr);
	}
	strncpy(wd_st_full, location, FILENAME_MAX);
	wd_st_full[FILENAME_MAX-1] = EOS;
	do {
		if ((endPipe=FindPipe(com)) == NULL)
			break;
		if (!FindRedirect(com))
			break;
		if (!run_CLAN_Prog(com, argc, argv)) {
			free(argv);
			for (argc=0; argc < fileNamesIndex; argc++)
				free(fileNames[argc]);
			return(resultStr);
		}
		if (isKillProgram)
			break;
		strcpy(com, endPipe);
	} while (*com != EOS) ;

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
	free(argv);
	for (argc=0; argc < fileNamesIndex; argc++)
		free(fileNames[argc]);
	return(resultStr);
}
