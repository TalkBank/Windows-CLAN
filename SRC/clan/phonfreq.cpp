/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main phonfreq_main
#define call phonfreq_call
#define getflag phonfreq_getflag
#define init phonfreq_init
#define usage phonfreq_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define PHONLINE "%pho:"
#define ALPHAFILE "alphabet.cut"

#define WORDS struct words_s
WORDS {
	char *word;
	WORDS *next_word;
} *RootWords;

#define ALPHA struct alpha_s
ALPHA {
	char *sound;
	ALPHA *next_sound;
} *RootAlphabet;

struct phonfreq_tnode {
	char *sound;
	unsigned beg;
	WORDS *BegWords;
	unsigned end;
	WORDS *EndWords;
	unsigned other;
	WORDS *OtherWords;
	struct phonfreq_tnode *left;
	struct phonfreq_tnode *right;
} *phonfreq_root;

char phonfreq_word[BUFSIZ], sound[BUFSIZ];
char PhoLine[SPEAKERLEN];


void usage() {
   puts("PHONFREQ ");
   printf("Usage: phonfreq [bS %s] filename(s)\n", mainflgs());
   printf("+bS: set phonological tier name to S, (default %s)\n", PHONLINE);
#ifdef UNX
   puts("+LF: specify full path F of the lib folder");
#endif
   mainusage(TRUE);
}

static void phonfreq_overflow() {
	fprintf(stderr,"phonfreq: no more core available.\n");
	cutt_exit(1);
}

static char *phonfreq_strsave(char *s) {
	char *p;

	if ((p = (char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		phonfreq_overflow();
	return(p);
}

static void GetAlphabet() {
	ALPHA *p;
	FILE *fp;
	FNType mFileName[FNSize];

	if ((fp=OpenGenLib(ALPHAFILE,"r",TRUE,TRUE,mFileName)) == NULL) {
		fprintf(stderr, "WARNING: Can't open either one of the alphabet files:\n\t\"%s\", \"%s\"\n", ALPHAFILE, mFileName);
		return;
	}
	while (fgets_cr(sound, BUFSIZ, fp) != NULL) {
		if (uS.isUTF8(sound) || uS.partcmp(sound, FONTHEADER, FALSE, FALSE))
			continue;
		uS.remblanks(sound);
		if (*sound == EOS)
			continue;
		if (RootAlphabet == NULL) {
			RootAlphabet = NEW(ALPHA);
			if (RootAlphabet == NULL)
				phonfreq_overflow();
			p = RootAlphabet;
		} else {
			p->next_sound = NEW(ALPHA);
			if (p->next_sound == NULL)
				phonfreq_overflow();
			p = p->next_sound;
		}
		p->sound = phonfreq_strsave(sound);
		p->next_sound = NULL;
	}
	fclose(fp);
}

void init(char first) {
	if (first) {
		PhoLine[0] = EOS;
		phonfreq_root = NULL;
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		mor_initwords();
		FilterTier = 1;
		RootWords = NULL;
		RootAlphabet = NULL;
	} else {
		if (RootAlphabet == NULL)
			GetAlphabet();
		if (PhoLine[0] == EOS) {
			strcpy(PhoLine, PHONLINE);
			uS.uppercasestr(PhoLine, &dFnt, C_MBF);
			maketierchoice(PhoLine,'+',FALSE);
		}
	}

	if (!combinput || first)
		phonfreq_root = NULL;
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'b':
			if (*(f-2) == '+') {
				uS.uppercasestr(f, &dFnt, C_MBF);
				strcpy(PhoLine, f);
			}
			if (!*f) {
				fprintf(stderr,"String expected after %s option.\n", f-2);
				cutt_exit(0);
			}
			maketierchoice(f,*(f-2),FALSE);
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

static WORDS *pf_GetWord(WORDS *rw) {
	WORDS *w;

	w = NEW(WORDS);
	if (w == NULL)	
		phonfreq_overflow();
	w->next_word = rw;
	rw = w;
	if (RootWords == NULL) {
		RootWords = NEW(WORDS);
		if (RootWords == NULL)
			phonfreq_overflow();
		w = RootWords;
	} else {
		for (w=RootWords; 1; w = w->next_word) {
			if (!strcmp(w->word, phonfreq_word)) {
				rw->word = w->word;
				return(rw);
			}
			if (w->next_word == NULL)
				break;
		}
		w->next_word = NEW(WORDS);
		if (w->next_word == NULL)
			phonfreq_overflow();
		w = w->next_word;
	}
	w->word = phonfreq_strsave(phonfreq_word);
	w->next_word = NULL;
	rw->word = w->word;
	return(rw);
}

static void phonfreq_AddWord(struct phonfreq_tnode *p, int pos) {
	if (pos == 1) {
		p->other++;
		if (onlydata == 1)
			p->OtherWords = pf_GetWord(p->OtherWords);
	} else if (pos == 2) {
		p->end++;
		if (onlydata == 1)
			p->EndWords = pf_GetWord(p->EndWords);
	} else {
		p->beg++;
		if (onlydata == 1)
			p->BegWords = pf_GetWord(p->BegWords);
	}
}

static struct phonfreq_tnode *phonfreq_talloc(char *sound) {
	struct phonfreq_tnode *p;

	if ((p = NEW(struct phonfreq_tnode)) == NULL)
		phonfreq_overflow();
	p->sound = sound;
	p->beg = 0;
	p->end = 0;
	p->other = 0;
	p->BegWords = NULL;
	p->EndWords = NULL;
	p->OtherWords = NULL;
	return(p);
}

static struct phonfreq_tnode *phonfreq_tree(struct phonfreq_tnode *p,char *sound,int pos) {
	int cond;
	struct phonfreq_tnode *t = p;

	if (p == NULL) {
		p = phonfreq_talloc(phonfreq_strsave(sound));
		phonfreq_AddWord(p, pos);
		p->left = p->right = NULL;
	} else if ((cond=strcmp(sound,p->sound)) == 0)
		phonfreq_AddWord(p, pos);
	else if (cond < 0)
		p->left = phonfreq_tree(p->left, sound, pos);
	else {
		while ((cond=strcmp(sound,p->sound)) >0 && p->right!=NULL)
			p = p->right;
		if (cond == 0)
			phonfreq_AddWord(p, pos);
		else if (cond < 0)
			p->left = phonfreq_tree(p->left, sound, pos);
		else
			p->right = phonfreq_tree(p->right, sound, pos);
		return(t);
	}
	return(p);
}

static void phonfreq_treeprint(struct phonfreq_tnode *p) {
	WORDS *w, *tw;
	struct phonfreq_tnode *t;

	if (p != NULL) {
		phonfreq_treeprint(p->left);
		do {
			fprintf(fpout,"%3u  %-4s ", p->beg+p->end+p->other, p->sound);
			if (onlydata == 0) {
				fprintf(fpout,"initial = %3u, ", p->beg);
				fprintf(fpout,"final = %3u, ", p->end);
				fprintf(fpout,"other = %3u\n", p->other);
			} else {
				fprintf(fpout,"\n    initial = %3u", p->beg);
				for (w=p->BegWords; w != NULL; ) {
					fprintf(fpout,"\n\t%s", w->word);
					tw = w;
					w = w->next_word;
					free(tw);
				}
				fprintf(fpout,"\n    final = %3u", p->end);
				for (w=p->EndWords; w != NULL; ) {
					fprintf(fpout,"\n\t%s", w->word);
					tw = w;
					w = w->next_word;
					free(tw);
				}
				fprintf(fpout,"\n    other = %3u", p->other);
				for (w=p->OtherWords; w != NULL; ) {
					fprintf(fpout,"\n\t%s", w->word);
					tw = w;
					w = w->next_word;
					free(tw);
				}
				putc('\n', fpout);
			}
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				phonfreq_treeprint(p->right);
				break;
			}
			t = p;
			p = p->right;
			if (t->sound)
				free(t->sound);
			free(t);
		} while (1);
		if (p->sound)
			free(p->sound);
		free(p);
	}
}

static void phonfreq_pr_result(void) {
	phonfreq_treeprint(phonfreq_root);
}

static char *GetSound(char *st) {
	register int i;
	register char *su;
	ALPHA *t;

	for (t=RootAlphabet; t != NULL; t=t->next_sound) {
		for (i=0, su=t->sound; su[i]; i++) {
			if (st[i] != su[i]) {
				if (st[i] == EOS || su[i] != '*')
					break;
			}
			sound[i] = st[i];
		}
		if (su[i] == EOS) {
			sound[i] = EOS;
			return(st+i);
		}
	}
	i = 0;
	do {
		sound[i++] = *st;
//		if (dFnt.isUTF) {
			if (UTF8_IS_SINGLE(*st) || UTF8_IS_TRAIL(*st))
				break;
//		} else
//			break;
		st++;
	} while (1) ;
	sound[i] = EOS;
	return(st+1);
}

static void ProcessWord(char *s) {
	int pos;

	while (*s) {
		pos = 1;
		if (s == phonfreq_word)
			pos = 0;
		s = GetSound(s);
		if (*s == EOS) {
			if (pos == 0)
				pos = 3;
			else
				pos = 2;
		}
		if (exclude(sound))
			phonfreq_root = phonfreq_tree(phonfreq_root, sound, pos);
	}
}

void call() {
	register int i;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		strcpy(templineC,utterance->speaker);
		uS.uppercasestr(templineC, &dFnt, MBF);
		if (uS.partcmp(templineC, PhoLine, FALSE, FALSE)) {
			i = 0;
			while ((i=getword(utterance->speaker, uttline, phonfreq_word, NULL, i))) {
				uS.remblanks(phonfreq_word);
				ProcessWord(phonfreq_word);
			}
		}
	}
	if (!combinput)
		phonfreq_pr_result();
}

static void phonfreq_cleanup() {
	ALPHA *t;
	WORDS *w;

	while (RootAlphabet != NULL) {
		t = RootAlphabet;
		RootAlphabet = RootAlphabet->next_sound;
		if (t->sound)
			free(t->sound);
		free(t);
	}
	while (RootWords != NULL) {
		w = RootWords;
		RootWords = RootWords->next_word;
		if (w->word)
			free(w->word);
		free(w);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
	CLAN_PROG_NUM = PHONFREQ;
	chatmode = CHAT_MODE;
	OnlydataLimit = 1;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,phonfreq_pr_result);
	phonfreq_cleanup();
}
