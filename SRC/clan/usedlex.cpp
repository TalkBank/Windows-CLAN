/**********************************************************************
 "Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

#define CHAT_MODE 3
#include "cu.h"

#if !defined(UNX)
#define _main usedlex_main
#define call usedlex_call
#define getflag usedlex_getflag
#define init usedlex_init
#define usage usedlex_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;

struct tlex_tnode {
	char *word;
	struct tlex_tnode *left;
	struct tlex_tnode *right;
};

static struct tlex_tnode *tlex_root;

void usage() {
	printf("Usage: usedlex [%s] filename(s)\n",mainflgs());
	mainusage(TRUE);
}

void init(char f) {
	if (f) {
		stout = TRUE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
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
		mor_initwords();
		tlex_root = NULL;
	}
}

static void tlex_freetree(struct tlex_tnode *p) {
	if (p != NULL) {
		tlex_freetree(p->left);
		tlex_freetree(p->right);
		free(p->word);
		free(p);
	}
}

static char tlex_tree_find(struct tlex_tnode *p, char *w) {
	int cond;
	char res;
	
	if (p == NULL) {
		res = FALSE;
	} else if ((cond=strcmp(w,p->word)) == 0) {
		res = TRUE;
	} else if (cond < 0)
		res = tlex_tree_find(p->left, w);
	else
		res = tlex_tree_find(p->right, w); /* if cond > 0 */
	return(res);
}

static void OpenLex(FNType *mFileName, int *isOpenOne) {
	int  i, s, k, index, t, len;
	char word[BUFSIZ], isFoundNot, isFoundUsed;
	long ln;
	FNType tFName[FNSize], lex_out_used[FNSize], lex_out_not[FNSize];
	FILE *lex_fp_in, *lex_fp_used, *lex_fp_not;
	
	index = 1;
	t = strlen(mFileName);
	while ((index=Get_File(tFName, index)) != 0) {
		len = strlen(tFName) - 4;
		if (tFName[0] != '.' && len >= 0 && uS.FNTypeicmp(tFName+len, ".cut", 0) == 0) {
			addFilename2Path(mFileName, tFName);
			if ((lex_fp_in=fopen(mFileName, "r"))) {
				fprintf(stderr,"\rLooking at lexicon: %s.\n", mFileName);
				parsfname(mFileName, lex_out_used,".used.cex");
				lex_fp_used = fopen(lex_out_used, "w");
				if (lex_fp_used == NULL) {
					fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", lex_out_used);
					tlex_freetree(tlex_root);
					cutt_exit(0);
				}
				parsfname(mFileName, lex_out_not,".not.cex");
				lex_fp_not = fopen(lex_out_not, "w");
				if (lex_fp_not == NULL) {
					fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", lex_out_not);
					tlex_freetree(tlex_root);
					cutt_exit(0);
				}
				isFoundNot = FALSE;
				isFoundUsed = FALSE;
				ln = 0L;
				while (fgets_cr(templineC,UTTLINELEN,lex_fp_in)) {
					ln++;
					uS.remblanks(templineC);
					if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
						continue;
					for (s=0; isSpace(templineC[s]); s++) ;
					if (templineC[s] == '%' || templineC[s] == EOS)
						continue;
					for (i=s; templineC[i] != EOS; i++) {
						if (uS.mStrnicmp(templineC+i, "% not used", 10) == 0) {
							strcpy(templineC+i, templineC+i+10);
							break;
						}
					}
					k = 0;
					for (i=s; !isSpace(templineC[i]) && templineC[i] != EOS; i++) {
						word[k++] = templineC[i];
					}
					word[k] = EOS;
					if (!tlex_tree_find(tlex_root, word)) {
						strcat(templineC, " % not used");
						fprintf(lex_fp_not, "%s\n", templineC);
						isFoundNot = TRUE;
					} else {
						fprintf(lex_fp_used, "%s\n", templineC);
						isFoundUsed = TRUE;
					}
				}
				fclose(lex_fp_used);
				fclose(lex_fp_not);
				fclose(lex_fp_in);
				if (!isFoundNot)
					unlink(lex_out_not);
				if (!isFoundUsed)
					unlink(lex_out_used);
				lex_fp_in = NULL;
				*isOpenOne = TRUE;
			}
			mFileName[t] = EOS;
		}
	}
}

static void ParseLex(void) {
	int  isOpenOne = FALSE;
	FNType lexst[5], mFileName[FNSize];
	
	uS.str2FNType(lexst, 0L, "lex");
	uS.str2FNType(lexst, strlen(lexst), PATHDELIMSTR);
	strcpy(mFileName,mor_lib_dir);
	addFilename2Path(mFileName, lexst);
	if (!SetNewVol(mFileName)) {
		OpenLex(mFileName, &isOpenOne);
	}
	SetNewVol(wd_dir);
	if (!isOpenOne) {
		fprintf(stderr, "Can't find any lexicon files in folder: %s\n", mFileName);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = USEDLEX;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
	ParseLex();
	tlex_freetree(tlex_root);
}
		
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char *tlex_strsave(char *s) {
	char *p;
	
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else {
		fprintf(stderr,"temp: no more memory available.\n");
		cutt_exit(1);
	}
	return(p);
}

static struct tlex_tnode *tlex_talloc(char *word) {
	struct tlex_tnode *p;
	
	if ((p=NEW(struct tlex_tnode)) == NULL) {
		fprintf(stderr,"temp: no more memory available.\n");
		cutt_exit(1);
	}
	p->word = word;
	return(p);
}

static struct tlex_tnode *tlex_tree(struct tlex_tnode *p, char *w) {
	int cond;
	
	if (p == NULL) {
		p = tlex_talloc(tlex_strsave(w));
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->word)) == 0) {
	} else if (cond < 0)
		p->left = tlex_tree(p->left, w);
	else
		p->right = tlex_tree(p->right, w); /* if cond > 0 */
	return(p);
}

void call() {
	register int i, wi;
	char word[BUFSIZ];

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			i = 0;
			while ((i=getword(utterance->speaker, uttline, word, &wi, i))) {
				tlex_root = tlex_tree(tlex_root, word);
			}
		}
	}
}
