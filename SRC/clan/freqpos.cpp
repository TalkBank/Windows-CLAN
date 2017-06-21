/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main freqpos_main
#define call freqpos_call
#define getflag freqpos_getflag
#define init freqpos_init
#define usage freqpos_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

struct freqpos_tnode {
	char *word;
	unsigned int beg;
	unsigned int end;
	unsigned int other;
	unsigned int OneWord;
	struct freqpos_tnode *left;
	struct freqpos_tnode *right;
} ;

static char DC;
static unsigned int g_beg, g_end, g_other, g_OneWord;
static IEWORDS *freqpos_wdptr;
static struct freqpos_tnode *freqpos_root;

void usage() {
   puts("FREQPOS ");
   printf("Usage: freqpos [d g %s] filename(s)\n", mainflgs());
   puts("+d : count word in first, second and last positions");
   puts("+gS: display only the word S or words in a file @S");
   mainusage(TRUE);
}

void init(char first) {
	if (first) {
		DC = FALSE;
		freqpos_root = NULL;
		freqpos_wdptr = NULL;
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		maininitwords();
		mor_initwords();
		FilterTier = 1;
	} else if (!combinput)
		freqpos_root = NULL;
}

static void addfreqpos_wdptr(char *wd) {
	register int i;
	IEWORDS *tempwd;

	for (i=strlen(wd)-1; wd[i]== ' ' || wd[i]== '\t' || wd[i]== '\n'; i--) ;
	wd[++i] = EOS;
	for (i=0; wd[i] == ' ' || wd[i] == '\t'; i++) ;
	if ((tempwd=NEW(IEWORDS)) == NULL)
		out_of_mem();
	else {
		tempwd->nextword = freqpos_wdptr;
		freqpos_wdptr = tempwd;
		freqpos_wdptr->word = (char *)malloc(strlen(wd+i)+1);
		if (freqpos_wdptr->word == NULL)
			out_of_mem();
		if (!nomap)
			uS.lowercasestr(wd+i, &dFnt, C_MBF);
		strcpy(freqpos_wdptr->word, wd+i);
	}
}

static void freqpos_exclude_file(FNType *fname) {
	FILE *efp;
	char wd[512];

	if (*fname == EOS) {
		fprintf(stderr, "No file name for the +g option was specified.\n");
		cutt_exit(0);
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(wd_dir);
#endif
	if ((efp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "Can not access include file: %s\n", fname);
		cutt_exit(0);
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
#endif
	while (fgets_cr(wd, 512, efp)) {
		if (uS.isUTF8(wd) || uS.partcmp(wd, FONTHEADER, FALSE, FALSE))
			continue;
		addfreqpos_wdptr(wd);
	}
	fclose(efp);
}

static int freqpos_rightword(char *word) {
	IEWORDS *twd;

	for (twd=freqpos_wdptr; twd != NULL; twd = twd->nextword) {
		if (uS.patmat(word, twd->word))
			return(TRUE);
	}
	if (freqpos_wdptr == NULL)
		return(TRUE);
	else
		return(FALSE);
}

static void freqpos_overflow(void) {
	fprintf(stderr,"phonfreq: no more core available.\n");
	cutt_exit(1);
}

static char *freqpos_strsave(char *s) {
	char *p;

	if ((p = (char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		freqpos_overflow();
	return(p);
}

static void freqpos_SetPos(struct freqpos_tnode *p, int pos) {
	if (pos == 1) p->other++;
	else if (pos == 3) p->end++;
	else if (pos == 4) p->OneWord++;
	else if (pos == 0) p->beg++;
}

static struct freqpos_tnode *freqpos_tree(struct freqpos_tnode *p,char *word,int pos) {
	int cond;
	struct freqpos_tnode *t = p;

	 if (p == NULL) {
		 if ((p=NEW(struct freqpos_tnode)) == NULL)
		 	freqpos_overflow();
		 p->word = freqpos_strsave(word);
		 p->beg = 0;
		 p->end = 0;
		 p->other = 0;
		 p->OneWord = 0;
		 freqpos_SetPos(p, pos);
		 p->left = p->right = NULL;
	} else if ((cond=strcmp(word,p->word)) == 0)
		freqpos_SetPos(p, pos);
	else if (cond < 0)
		p->left = freqpos_tree(p->left, word, pos);
	else {
		while ((cond=strcmp(word,p->word)) >0 && p->right!=NULL)
			p=p->right;
		if (cond == 0)
			freqpos_SetPos(p, pos);
		else if (cond < 0)
			p->left = freqpos_tree(p->left, word, pos);
		else
			p->right = freqpos_tree(p->right, word, pos);
		return(t);
	}
	return(p);
}

static void freqpos_treeprint(struct freqpos_tnode *p) {
	struct freqpos_tnode *t;

	if (p != NULL) {
		freqpos_treeprint(p->left);
		do {
			g_beg += p->beg;
			g_end += p->end;
			g_other += p->other;
			g_OneWord += p->OneWord;
			fprintf(fpout,"%3u  %-20s ", p->beg+p->end+p->other+p->OneWord, p->word);
			fprintf(fpout,"initial =%3u, ", p->beg);
			fprintf(fpout,"final =%3u, ", p->end);
			if (DC)
				fprintf(fpout,"second =%3u, ", p->other);
			else
				fprintf(fpout,"other =%3u, ", p->other);
			fprintf(fpout,"one word =%3u\n", p->OneWord);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				freqpos_treeprint(p->right);
				break;
			}
			t = p;
			p = p->right;
			if (t->word)
				free(t->word);
			free(t);
		} while (1);
		if (p->word)
			free(p->word);
		free(p);
	}
}

static void freqpos_pr_result(void) {
	g_beg = g_end = g_other = g_OneWord = 0;
	freqpos_treeprint(freqpos_root);
	fprintf(fpout,"\nNumber of words in an initial position =%3u\n", g_beg);
	if (DC)
		fprintf(fpout,"Number of words in a second position   =%3u\n",g_other);
	else
		fprintf(fpout,"Number of words in an other position   =%3u\n",g_other);
	fprintf(fpout,"Number of words in a final position    =%3u\n", g_end);
	fprintf(fpout,"Number of one word utterences          =%3u\n", g_OneWord);
}

void call() {
	register int i;
	register int pos;
	char word[BUFSIZ];

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (WordMode == 'e')
			filterwords(utterance->speaker, uttline, exclude);
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		i = 0;
		pos = 0;
		while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
			while (uttline[i]!= EOS && uS.isskip(uttline,i,&dFnt,MBF) && uttline[i]!= '[')
				i++;
			if (uttline[i] == EOS) {
				if (pos != 0)
					pos = 3;
				else
					pos = 4;
			}
			if (freqpos_rightword(word) && (!DC || pos != 2)) {
				freqpos_root = freqpos_tree(freqpos_root, word, pos);
			}
			if (DC) {
				if (pos < 2)
					pos++;
			} else
				pos = 1;
		}
	}
	if (!combinput)
		freqpos_pr_result();
}

void getflag(char *f, char *f1, int *i) {
	char s1[256], s2[256];
	f++;
	switch(*f++) {
		case 'd':
			DC = TRUE;
			no_arg_option(f);
			break;
		case 'g':
			if (*f) {
				if (*f == '@') {
					f++;
					freqpos_exclude_file(getfarg(f,f1,i));
				} else {
					if (*f == '\\' && *(f+1) == '@')
						f++;
					addfreqpos_wdptr(getfarg(f,f1,i));
				}
			} else {
				fprintf(stderr, "A word or file name is expected with %s option.", f-2);
				freqpos_wdptr = freeIEWORDS(freqpos_wdptr);
				cutt_exit(0);
			}
			break;
		case 't':
			if (*(f-2) == '+' && *f == '%') {
				int k=1;
				strcpy(s1, "-t*");
				strcpy(s2, "");
				maingetflag(s1,s2,&k);
			}
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = FREQPOS;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,freqpos_pr_result);
	freqpos_wdptr = freeIEWORDS(freqpos_wdptr);
}
