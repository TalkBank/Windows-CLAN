/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#include "cu.h"

#if !defined(UNX)
#define _main gemfreq_main
#define call gemfreq_call
#define getflag gemfreq_getflag
#define init gemfreq_init
#define usage gemfreq_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define HASHMAX 50

struct gemfreq_tnode {
	char *word;
	unsigned int count;
	struct gemfreq_tnode *left;
	struct gemfreq_tnode *right;
};

struct gemfreq_tgem {
	char *key;
	unsigned int count;
	struct gemfreq_tnode *root;
	struct gemfreq_tgem *NextGem;
} *gemfreq_Gem;

static int  gemfreq_SpecWords;
static char *gemfreq_Hash[HASHMAX];
static char isSort = FALSE;
static char gemfreq_BBS[5], gemfreq_CBS[5];
static char gemfreq_n_option;
static char gemfreq_wholeTier;
static char gemfreq_includeNested;
static char gemfreq_group;
static char gemfreq_FTime;
static char gemfreq_WdMode = '\0'; /* 'i' - means to include ml_WdHead words, else exclude	*/
static IEWORDS *gemfreq_WdHead;	/* contains words to included/excluded gem keywords			*/

void usage() {
	puts("GEMFREQ creates a frequency word count of a each gem data");
	printf("Usage: gemfreq [e g n o wS yN %s] filename(s)\n",mainflgs());
	puts("+e : do not output any nested gems along with matched one");
	puts("+g : marker tier should contain all words specified by +s");
	puts("+n : each Gem is terminated by the next @G");
	puts("+o : sort output by descending frequency");
	puts("+wS: search for word S or words in file @S in an input file ");
	puts("-wS: exclude word S or words in file @S from a given input file");
	puts("+yN: display the whole N=1 unchanged or N=0 cleaned up tier");
	mainusage(TRUE);
}

void init(char first) {
	if (first) {
		isSort = FALSE;
		gemfreq_WdMode = '\0';
		gemfreq_WdHead = NULL;
		gemfreq_FTime = TRUE;
		gemfreq_n_option = FALSE;
		gemfreq_wholeTier = 0;
		gemfreq_includeNested = TRUE;
		gemfreq_group = FALSE;
		gemfreq_SpecWords = 0;
		strcpy(gemfreq_BBS, "@BG:");
		strcpy(gemfreq_CBS, "@EG:");
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+.");
		addword('\0','\0',"+?");
		addword('\0','\0',"+!");
		maininitwords();
		mor_initwords();
		FilterTier = 1;
		LocalTierSelect = TRUE;
	} else {
		if (gemfreq_FTime) {
			unsigned int i = 0;

			gemfreq_FTime = FALSE;
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '!' ||
					GlobalPunctuation[i] == '?' ||
					GlobalPunctuation[i] == '.') 
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}
			if (gemfreq_WdHead != NULL) {
				maketierchoice(gemfreq_BBS,'+',FALSE);
				if (!gemfreq_n_option)
					maketierchoice(gemfreq_CBS,'+',FALSE);
			}
			if (isMORSearch() && chatmode) {
				maketierchoice("%mor",'+',FALSE);
				nomain = TRUE;
			}
			if (isGRASearch() && chatmode) {
				maketierchoice("%gra",'+',FALSE);
				nomain = TRUE;
			}
		}
		if (gemfreq_SpecWords == 0) {
			IEWORDS *tmp;
			for (tmp=gemfreq_WdHead; tmp != NULL; tmp=tmp->nextword)
				gemfreq_SpecWords++;
		}
	}

	if (!combinput || first) {
		gemfreq_Gem = NULL;
	}
}

static void gemfreq_freetree(struct gemfreq_tnode *p) {
	if (p != NULL) {
		gemfreq_freetree(p->left);
		gemfreq_freetree(p->right);
		free(p->word);
		free(p);
	}
}


static void gemfreq_freegem() {
	struct gemfreq_tgem *t;

	while (gemfreq_Gem != NULL) {
		t = gemfreq_Gem;
		gemfreq_Gem = gemfreq_Gem->NextGem;
		free(t->key);
		gemfreq_freetree(t->root);
		free(t);
	}
	gemfreq_Gem = NULL;
}

static void gemfreq_overflow() {
	fprintf(stderr,"Gemfreq: no more core available.\n");
	gemfreq_WdHead = freeIEWORDS(gemfreq_WdHead);
	gemfreq_freegem();
	cutt_exit(1);
}

static void gemfreq_addwd(char opt, char ch, char *wd) {
	register int i;
	IEWORDS *tempwd;

	for (i=strlen(wd)-1; wd[i]== ' ' || wd[i]== '\t' || wd[i]== '\n'; i--) ;
	wd[++i] = EOS;
	if (wd[0] == '+')
		i = 1;
	else
		i = 0;
	for (; wd[i] == ' ' || wd[i] == '\t'; i++) ;
	if (!gemfreq_WdMode) {
		gemfreq_WdMode = ch;
	} else if (gemfreq_WdMode != ch) {
		if (opt == '\0')
			opt = ' ';
		fprintf(stderr,"-%c option can not be used together with option +%c.\n", opt, opt);
		fprintf(stderr, "Offending word is: %s\n", wd+i);
		gemfreq_freegem();
		gemfreq_WdHead = freeIEWORDS(gemfreq_WdHead);
		cutt_exit(0);
	}
	tempwd = NEW(IEWORDS);
	if (tempwd == NULL) {
		fprintf(stderr,"Memory ERROR. Use less include/exclude words.\n");
		gemfreq_freegem();
		gemfreq_WdHead = freeIEWORDS(gemfreq_WdHead);
		cutt_exit(1);
	} else {
		tempwd->word = (char *)malloc(strlen(wd+i)+1);
		if(tempwd->word == NULL) {
			fprintf(stderr,"No more space left in core.\n");
			free(tempwd);
			gemfreq_freegem();
			gemfreq_WdHead = freeIEWORDS(gemfreq_WdHead);
			cutt_exit(1);
		}
		tempwd->nextword = gemfreq_WdHead;
		gemfreq_WdHead = tempwd;
		if (!nomap)
			uS.lowercasestr(wd+i, &dFnt, C_MBF);
		strcpy(gemfreq_WdHead->word, wd+i);
	}
}

static void gemfreq_mkwdlist(char opt, char ch, FNType *fname) {
	FILE *efp;
	char wd[BUFSIZ];

	if (*fname == '+') {
		*wd = '+';
		fname++;
	} else
		*wd = ' ';

	if (*fname == EOS) {
		fprintf(stderr, "No file name for the +g option was specified.\n");
		gemfreq_freegem();
		gemfreq_WdHead = freeIEWORDS(gemfreq_WdHead);
		cutt_exit(0);
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(wd_dir);
#endif
	if ((efp = fopen(fname, "r")) == NULL) {
		fprintf(stderr,"Can not access %s file: %s\n",((ch=='i')?"include":"exclude"),fname);
		gemfreq_freegem();
		gemfreq_WdHead = freeIEWORDS(gemfreq_WdHead);
		cutt_exit(0);
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
#endif
	while (fgets_cr(wd+1, 511, efp)) {
		if (uS.isUTF8(wd+1) || uS.isInvisibleHeader(wd+1))
			continue;
		gemfreq_addwd(opt, ch, wd);
	}
	fclose(efp);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'e':
				gemfreq_includeNested = FALSE;
				no_arg_option(f);
				break;
		case 'g':
				gemfreq_group = TRUE;
				no_arg_option(f);
				break;
		case 'n':
				gemfreq_n_option = TRUE;
				strcpy(gemfreq_BBS, "@G:");
				strcpy(gemfreq_CBS, "@*&#");
				no_arg_option(f);
				break;
		case 'o':
				isSort = TRUE;
				no_arg_option(f);
				break;
		case 'y':
				if (*f == '1')
					gemfreq_wholeTier = 2;
				else
					gemfreq_wholeTier = 1;
				break;
		case 's':
				if (*f) {
					if (*(f-2) == '+') {
						if (*f == '@') {
							f++;
							gemfreq_mkwdlist('o','i', getfarg(f,f1,i));
						} else {
							if (*f == '\\' && *(f+1) == '@')
								f++;
							gemfreq_addwd('o','i',getfarg(f,f1,i));
						}
					} else {
						if (*f == '@') {
							f++;
							gemfreq_mkwdlist('o','e', getfarg(f,f1,i));
						} else {
							if (*f == '\\' && *(f+1) == '@')
								f++;
							gemfreq_addwd('o','e',getfarg(f,f1,i));
						}
					}
				} else {
					fprintf(stderr,"Missing argument for option: %s\n", f-2);
					cutt_exit(0);
				}
				break;
		case 'w':
				*(f-1) = 's';
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static int gemfreq_excludewd(char *word) {
	IEWORDS *twd;

	for (twd=gemfreq_WdHead; twd != NULL; twd = twd->nextword) {
		if (uS.patmat(word, twd->word)) {
			if (gemfreq_WdMode == 'i')
				return(TRUE);
			else
				return(FALSE);
		}
	}
	if (gemfreq_WdMode == 'i')
		return(FALSE);
	else
		return(TRUE);
}

static char *gemfreq_strsave(char *s) {
	char *p;

	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		gemfreq_overflow();
	return(p);
}

static struct gemfreq_tnode *gemfreq_talloc(char *word, unsigned int count) {
	struct gemfreq_tnode *p;

	if ((p = NEW(struct gemfreq_tnode)) == NULL)
		gemfreq_overflow();
	p->word = word;
	p->count = count;
	return(p);
}

static struct gemfreq_tnode *gemfreq_tree(struct gemfreq_tnode *p, char *w) {
	int cond;
	struct gemfreq_tnode *t = p;

	if (p == NULL) {
		p = gemfreq_talloc(gemfreq_strsave(w),0);
		p->count++;
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->word)) == 0)
		p->count++;
	else if (cond < 0)
		p->left = gemfreq_tree(p->left, w);
	else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0)
			p->count++;
		else if (cond < 0)
			p->left = gemfreq_tree(p->left, w);
		else
			p->right = gemfreq_tree(p->right, w); /* if cond > 0 */
		return(t);
	}
	return(p);
}

static void gemfreq_treeprint(struct gemfreq_tnode *p) {
	struct gemfreq_tnode *t;

	if (p != NULL) {
		gemfreq_treeprint(p->left);
		do {
			fprintf(fpout,"    %3u ",p->count);
			fprintf(fpout,"%s\n", p->word);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				gemfreq_treeprint(p->right);
				break;
			}
			t = p;
			p = p->right;
			free(t->word);
			free(t);
		} while (1);
		free(p->word);
		free(p);
	}
}

static struct gemfreq_tnode *gemfreq_stree(struct gemfreq_tnode *p, struct gemfreq_tnode *m) {
	struct gemfreq_tnode *t = p;

	if (p == NULL) {
		p = gemfreq_talloc(gemfreq_strsave(m->word),m->count);
		p->left = p->right = NULL;
	} else if (p->count < m->count) p->left = gemfreq_stree(p->left, m);
	else if (p->count == m->count) {
		for (; p->count>= m->count && p->right!= NULL; p=p->right) ;
		if (p->count < m->count) p->left = gemfreq_stree(p->left, m);
		else p->right = gemfreq_stree(p->right, m);
		return(t);
	} else p->right = gemfreq_stree(p->right, m);
	return(p);
}

static void gemfreq_sorttree(struct gemfreq_tgem *tgem, struct gemfreq_tnode *p) {
	if (p != NULL) {
		gemfreq_sorttree(tgem, p->left);
		tgem->root = gemfreq_stree(tgem->root,p);
		gemfreq_sorttree(tgem, p->right);
		free(p->word);
		free(p);
	}
}

static void gemfreq_treesort(struct gemfreq_tgem *tgem) {
	struct gemfreq_tnode *p;

	p = gemfreq_Gem->root;
	tgem->root = NULL;
	gemfreq_sorttree(tgem, p);
	gemfreq_treeprint(tgem->root);
}

static void gemfreq_pr_result(void) {
	struct gemfreq_tgem *t;

	while (gemfreq_Gem != NULL) {
		fprintf(fpout,"%3u tier%sin gem \"", gemfreq_Gem->count, 
								(gemfreq_Gem->count>1 ? "s " : " "));
		fprintf(fpout,"%s\":\n", gemfreq_Gem->key);
		if (isSort)
			gemfreq_treesort(gemfreq_Gem);
		else
			gemfreq_treeprint(gemfreq_Gem->root);
		t = gemfreq_Gem;
		gemfreq_Gem = gemfreq_Gem->NextGem;
		free(t->key);
		free(t);
	}
}

static char gemfreq_IsEmptyLine(void) {
	register int i;

	for (i=0; uttline[i]; i++) {
		if (!uS.isskip(uttline,i,&dFnt,MBF))
			break;
	}
	if (uttline[i] == EOS)
		return(TRUE);
	else
		return(FALSE);
}

static int gemfreq_RightText(char *gem_word) {
	int i = 0;
	int found = 0;

	if (gemfreq_WdMode == '\0')
		return(TRUE);

	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i))) {
		if (gemfreq_excludewd(gem_word))
			found++;
	}

	if (gemfreq_WdMode == 'i' || gemfreq_WdMode == 'I')
		return((gemfreq_group == FALSE && found) || (gemfreq_SpecWords == found));
	else 
		return((gemfreq_group == TRUE && gemfreq_SpecWords > found) || (found == 0));
}

static struct gemfreq_tgem *FindGemItem(char *st) {
	register int res;
	struct gemfreq_tgem *tgem, *ttgem;

	if (gemfreq_Gem == NULL) {
		if ((tgem=NEW(struct gemfreq_tgem)) == NULL)
			gemfreq_overflow();
		gemfreq_Gem = tgem;
		tgem->NextGem = NULL;
	} else {
		ttgem = tgem = gemfreq_Gem;
		while (1) {
			if (tgem == NULL)
				break;
			else {
				res = strcmp(tgem->key,st);
				if (res == 0) {
					tgem->count++;
					return(tgem);
				} else if (res > 0)
					break;
			}
			ttgem = tgem;
			tgem = tgem->NextGem;
		}
		if (tgem == NULL) {
			ttgem->NextGem = NEW(struct gemfreq_tgem);
			if (ttgem->NextGem == NULL)
				gemfreq_overflow();
			tgem = ttgem->NextGem;
			tgem->NextGem = NULL;
		} else if (tgem == gemfreq_Gem) {
			gemfreq_Gem = NEW(struct gemfreq_tgem);
			if (gemfreq_Gem == NULL)
				gemfreq_overflow();
			gemfreq_Gem->NextGem = tgem;
			tgem = gemfreq_Gem;
		} else {
			if ((tgem=NEW(struct gemfreq_tgem)) == NULL)
				gemfreq_overflow();
			tgem->NextGem = ttgem->NextGem;
			ttgem->NextGem = tgem;
		}
	}
	tgem->key = gemfreq_strsave(st);
	tgem->count = 1;
	tgem->root = NULL;
	return(tgem);
}

static void CpUttline(char *to, char *fr) {
	while (*fr != EOS) {
		*to++ = *fr;
		if (*fr == '\n' || *fr == '\t') *fr = ' ';
		if (*fr == ' ') {
			for (fr++; *fr == '\n' || *fr == '\t' || *fr == ' '; fr++) ;
		} else fr++;
	}
	*to = *fr;
}
static void cleanUpTier(char *line) {
	for (; *line != EOS; line++) {
		if (*line == '\n' || *line == '\t')
			*line = ' ';
	}
}

static int isFullLine(char *s) {
	long i;

	for (i=0L; s[i]; i++) {
		if (!isSpace(s[i]) && s[i] != '\n')
			return(TRUE);
	}
	return(FALSE);
}


void call() {
	int found = 0;
	int  i;
	char isOutputGem;
	char word[BUFSIZ];
	char rightspeaker;
	struct gemfreq_tgem *gemfreq_GemLocal;
	unsigned int tlineno = 0;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	if (gemfreq_SpecWords) {
		*templineC2 = EOS;
		isOutputGem = FALSE;
	} else {
		strcpy(templineC2, "~other");
		isOutputGem = TRUE;
	}
	for (i=0; i < HASHMAX; i++)
		gemfreq_Hash[i] = NULL;
	rightspeaker = FALSE;
	while (getwholeutter()) {
		if (lineno > (long)tlineno) {
			tlineno = (unsigned int)lineno + 200;
			fprintf(stderr,"\r%ld ",lineno);
		}
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*')
				rightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*')
				rightspeaker = TRUE;
			if (!rightspeaker && *utterance->speaker != '@')
				continue;
		}

		uS.uppercasestr(utterance->speaker, &dFnt, MBF);
		if (uS.partcmp(utterance->speaker,gemfreq_BBS,FALSE,FALSE)) {
			if (gemfreq_n_option) {
				if (gemfreq_RightText(word)) {
					uS.remblanks(uttline);
					if (gemfreq_IsEmptyLine())
						strcpy(templineC2, "~Unmarked gems");
					else
						CpUttline(templineC2, uttline);
					isOutputGem = TRUE;
				} else
					isOutputGem = FALSE;
			} else if (gemfreq_includeNested) {
				if (gemfreq_RightText(word)) {
					found++;
					if (found == 1 || WordMode != '\0') {
						uS.remblanks(uttline);
						if (gemfreq_IsEmptyLine())
							strcpy(templineC2, "~Unmarked gems");
						else
							CpUttline(templineC2, uttline);
						isOutputGem = TRUE;
					}
				}
			} else {
				found++;
				if (gemfreq_RightText(word)) {
					uS.remblanks(uttline);
					if (gemfreq_IsEmptyLine())
						strcpy(templineC2, "~Unmarked gems");
					else
						CpUttline(templineC2, uttline);
					isOutputGem = TRUE;
				} else
					isOutputGem = FALSE;
				if (found < HASHMAX && isOutputGem) {
					gemfreq_Hash[found] = (char *)malloc(strlen(templineC2)+1);
					if (gemfreq_Hash[found] != NULL)
						strcpy(gemfreq_Hash[found], templineC2);
				}
			}
		} else if (found > 0 && uS.partcmp(utterance->speaker,gemfreq_CBS,FALSE,FALSE)) {
			if (gemfreq_n_option) {

			} else if (gemfreq_includeNested) {
				if (gemfreq_RightText(word)) {
					found--;
					if (found == 0) {
						if (gemfreq_SpecWords)
							isOutputGem = FALSE;
						else {
							isOutputGem = TRUE;
							strcpy(templineC2, "~other");
						}
					}
				}
			} else {
				if (gemfreq_Hash[found] != NULL) {
					free(gemfreq_Hash[found]);
					gemfreq_Hash[found] = NULL;
				}
				found--;
				if (found == 0) {
					if (gemfreq_SpecWords) {
						*templineC2 = EOS;
						isOutputGem = FALSE;
					} else {
						strcpy(templineC2, "~other");
						isOutputGem = TRUE;
					}
				} else if (found < HASHMAX) {
					if (gemfreq_Hash[found] == NULL) {
						*templineC2 = EOS;
						isOutputGem = FALSE;
					} else {
						strcpy(templineC2, gemfreq_Hash[found]);
						isOutputGem = TRUE;
					}
				}
			}
		}
		if (*utterance->speaker != '@' && isOutputGem) {
			filterwords(utterance->speaker,uttline,exclude);
			gemfreq_GemLocal = NULL;
			if (isFullLine(uttline)) {
				if (*utterance->speaker != '*' || !nomain)
					gemfreq_GemLocal = FindGemItem(templineC2);
				else
					continue;
			} else if ((i=isPostCodeFound(utterance->speaker, utterance->line)) == 0) {
				if (*utterance->speaker != '*' || !nomain)
					gemfreq_GemLocal = FindGemItem(templineC2);
				else
					continue;
			} else if (*utterance->speaker == '*' && i == 5) {
				rightspeaker = FALSE;
				if (*utterance->speaker == '*' && nomain)
					continue;
			} else if (*utterance->speaker == '*' && i == 1) {
				rightspeaker = FALSE;
				if (*utterance->speaker == '*' && nomain)
					continue;
			} else if (*utterance->speaker == '%' && i == 1 && rightspeaker) {
				gemfreq_GemLocal = FindGemItem(templineC2);
				if (uttline != utterance->line)
					strcpy(uttline,utterance->line);
				filterwords(utterance->speaker,uttline,excludedef);
			}
			if (gemfreq_GemLocal != NULL) {
				if (gemfreq_wholeTier) {
					if (isFullLine(uttline)) {
						uS.remblanks(utterance->line);
						cleanUpTier(utterance->line);
						if (gemfreq_wholeTier == 2)
							gemfreq_GemLocal->root = gemfreq_tree(gemfreq_GemLocal->root, utterance->line);
						else
							gemfreq_GemLocal->root = gemfreq_tree(gemfreq_GemLocal->root, uttline);
					}
				} else {
					i = 0;
					while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
						gemfreq_GemLocal->root = gemfreq_tree(gemfreq_GemLocal->root, word);
					}
				}
			}
		}
	}
	for (i=0; i < HASHMAX; i++) {
		if (gemfreq_Hash[i] != NULL) {
			free(gemfreq_Hash[i]);
			gemfreq_Hash[i] = NULL;
		}
	}
	fprintf(stderr, "\r      \r");
	if (!combinput)
		gemfreq_pr_result();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = GEMFREQ;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE; /* UttlineEqUtterance must be = FALSE */
	bmain(argc,argv,gemfreq_pr_result);
}
