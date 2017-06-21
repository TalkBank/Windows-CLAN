/**********************************************************************
	"Copyright 2006 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h"

#define IS_WIN_MODE FALSE
#define CHAT_MODE 1

struct slave {
	char *word;
	unsigned int count;
	struct slave *nextSlave;
};

struct anat_tnode {
	char *word;
	unsigned int count;
	struct slave *slaves;
	struct anat_tnode *left;
	struct anat_tnode *right;
};

extern char OverWriteFile;

static long wordCnt;
static char isStatOut;
static char anat_FTime;
static char *words_pairs;
static char master_sp[SPEAKERLEN], master_tier[UTTLINELEN+2];
static char slave_sp[SPEAKERLEN],  slave_tier[UTTLINELEN+2];
static struct anat_tnode *anat_root;
static struct anat_tnode **anat_RArr;
static struct anat_tnode *found_master_words[500];
static long found_master_words_cnt;

void usage() {
	printf("Usage: anat [cS d u %s] filename(s)\n",mainflgs());
    puts("+cS: specify word pairs S file.");
    puts("+d : output one word pair per line.");
	puts("+u : merge all specified files together.");
	mainusage();
}

static void freeSlaves(struct slave *p) {
	struct slave *t;

	while (p != NULL) {
		t = p;
		p = p->nextSlave;
		free(t->word);
		free(t);
	}
}

static void freeTree(struct anat_tnode *p) {
	struct anat_tnode *t;

	if (p != NULL) {
		freeTree(p->left);
		do {
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				freeTree(p->right);
				break;
			}
			t = p;
			p = p->right;
			freeSlaves(t->slaves);
			free(t->word);
			free(t);
		} while (1);
		freeSlaves(p->slaves);
		free(p->word);
		free(p);
	}
}

static void anat_overflow() {
	fprintf(stderr,"anat: no more memory available.\n");
	if (anat_RArr != NULL)
		free(anat_RArr);
	freeTree(anat_root);
	cutt_exit(0);
}

static char *anat_strsave(char *s) {
	char *p;

	if ((p = (char *)malloc((unsigned)(strlen(s)+1))) != NULL)
		strcpy(p, s);
	else
		anat_overflow();
	return(p);
}

static struct anat_tnode *addWord(char *word) {
	struct anat_tnode *p;

	if ((p = NEW(struct anat_tnode)) == NULL)
		anat_overflow();
	p->word = word;
	p->count = 0;
	return(p);
}

static struct slave *addSlave(struct slave *slaves, char *word) {
	struct slave *t;

	if ((t=NEW(struct slave)) == NULL)
		anat_overflow();
	t->nextSlave = slaves;
	t->word = word;
	t->count = 0;
	slaves = t;
	return(slaves);
}
/*
static int anat_compWords(const char *st1, const char *st2) {
	for (; *st2 != EOS; st1++, st2++) {
		if (*st1 != *st2) {
			if (*(st1-1) == 'v' && *st1 == ':') {
				while (*st1 != '|' && *st1 != EOS)
					st1++;
				if (*st1 != *st2 || *st1 == EOS)
					break;
			} else
				break;
		}
	}
	if (*st1 == EOS && *st2 == EOS)
		return(0);
	else if (*st1 > *st2)
		return(1);
	else
		return(-1);	
}
*/
static struct anat_tnode *mkTree(struct anat_tnode *p, char *w, char *slave) {
	int cond;
	struct anat_tnode *t = p;

	if (p == NULL) {
		wordCnt++;
		p = addWord(anat_strsave(w));
		p->slaves = addSlave(NULL, anat_strsave(slave));
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->word)) == 0)
		p->slaves = addSlave(p->slaves, anat_strsave(slave));
	else if (cond < 0)
		p->left = mkTree(p->left, w, slave);
	else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0)
			p->slaves = addSlave(p->slaves, anat_strsave(slave));
		else if (cond < 0)
			p->left = mkTree(p->left, w, slave);
		else
			p->right = mkTree(p->right, w, slave); /* if cond > 0 */
		return(t);
	}
	return(p);
}

static long map_tree2array(struct anat_tnode **arr, long i, struct anat_tnode *p) {
	if (p != NULL) {
		i = map_tree2array(arr, i, p->left);
		do {
			arr[i++] = p;
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				i = map_tree2array(arr, i, p->right);
				break;
			}
			p = p->right;
		} while (1);
	}
	return(i);
}

static void readWorList(void) {
	long tWordCnt;
	char word[512], slave[512], *s1, *s2, ftime;
	FILE *fp;

	if ((fp=OpenGenLib(words_pairs,"r",TRUE,FALSE,NULL)) == NULL)
		return;
	ftime = TRUE;
	while (fgets_cr(templineC, UTTLINELEN, fp)) {
		if (uS.isUTF8(templineC))
			continue;
		if (ftime) {
			ftime = FALSE;

			if (templineC[0] == (char)0xef && templineC[1] == (char)0xbb && templineC[2] == (char)0xbf)
				 strcpy(templineC, templineC+3);
			s1 = strchr(templineC, ',');
			if (s1 != NULL) {
				*s1 = EOS;
				strcpy(master_sp, templineC);
				strcat(master_sp, ":");
				s2 = strchr(s1+1, ',');
				if (s2 != NULL) {
					s2++;
					s1 = strchr(s2, ',');
					if (s1 != NULL) {
						*s1 = EOS;
						strcpy(slave_sp, s2);
						strcat(slave_sp, ":");
					}
				}
			}
			continue;
		}
		uS.remblanks(templineC);
		s1 = strrchr(templineC, ',');
		if (s1 != NULL) {
			strcpy(word, s1+1);
			strcpy(slave, s1+1);
			strcat(word, "|");
			strcat(slave, "|");
			s1 = strchr(templineC, ',');
			if (s1 != NULL) {
				*s1 = EOS;
				strcat(word, templineC);
				s2 = strchr(s1+1, ',');
				if (s2 != NULL) {
					s2++;
					s1 = strchr(s2, ',');
					if (s1 != NULL) {
						*s1 = EOS;
						strcat(slave, s2);
						anat_root = mkTree(anat_root, word, slave);
					}
				}
			}
		}
	}
	fclose(fp);

	anat_RArr = (struct anat_tnode **)malloc(sizeof(struct anat_tnode *) * wordCnt);
	if (anat_RArr == NULL)
		anat_overflow();
	tWordCnt = map_tree2array(anat_RArr, 0, anat_root);
}

void init(char first) {
	if (first) {
		anat_FTime = TRUE;
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+*|xxx");
		addword('\0','\0',"+*|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+.");
		addword('\0','\0',"+?");
		addword('\0','\0',"+!");
		maininitwords();
		isStatOut = FALSE;
		FilterTier = 1;
		OverWriteFile = TRUE;
		wordCnt = 0L;
		anat_root = NULL;
		anat_RArr = NULL;
		words_pairs = NULL;
		master_sp[0] = EOS;
		slave_sp[0] = EOS;
	} else {
		if (words_pairs == NULL) {
			fprintf(stderr,"Specify word pairs file name using +c option.\n");
			cutt_exit(0);
		}

		if (anat_FTime) {
			anat_FTime = FALSE;
			readWorList();
			if (master_sp[0] == EOS || slave_sp[0] == EOS) {
				fprintf(stderr,"Can't figure out the master and/or slave tier name.\n");
				cutt_exit(0);
			}
			maketierchoice(master_sp,'+',FALSE);
			maketierchoice(slave_sp,'+',FALSE);
			maketierchoice("%mor:",'+',FALSE);
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
		    if (!*f) {
		        fprintf(stderr,"Specify word pairs file name after +c option.\n");
				cutt_exit(0);
			}
		    words_pairs = f;
		    break;
		case 'd':
		    isStatOut = TRUE;
		    no_arg_option(f);
		    break;
		case 'u':
		    combinput = TRUE;
		    no_arg_option(f);
		    break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void anat_pr_result(void) {
	long i;
	struct slave *t;

	for (i=0; i < wordCnt; i++) {
		if (isStatOut) {
			if (anat_RArr[i]->count != 0) {
				for (t=anat_RArr[i]->slaves; t != NULL; t=t->nextSlave) {
					fprintf(fpout,"%s\t",anat_RArr[i]->word);
					fprintf(fpout,"%3u\t",anat_RArr[i]->count);
					fprintf(fpout,"%s\t",t->word);
					fprintf(fpout,"%3u\n",t->count);
					t->count = 0;
				}
				anat_RArr[i]->count = 0;
			}
		} else {
			if (anat_RArr[i]->count != 0) {
				fprintf(fpout,"%3u ",anat_RArr[i]->count);
				fprintf(fpout,"%s\n",anat_RArr[i]->word);
				anat_RArr[i]->count = 0;
				for (t=anat_RArr[i]->slaves; t != NULL; t=t->nextSlave) {
//					if (t->count != 0) {
						fprintf(fpout,"\t%3u ",t->count);
						fprintf(fpout,"%s\n",t->word);
						t->count = 0;
//					}
				}
			}
		}
	}
}

static struct anat_tnode *anat_BSearch(char *word) {
	int cond;
	long low, high, mid;

	low = 0;
	high = wordCnt - 1;
	while (low <= high) {
		mid = (low+high) / 2;
		if ((cond=strcmp(word, anat_RArr[mid]->word)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else {
			anat_RArr[mid]->count++;
			return(anat_RArr[mid]);
		}
	}
	return(NULL);
}

static void anat_cleanUpVerbs(char *word) {
	int i;

	if (word[0] == 'v') {
		for (i=1; word[i] != EOS && word[i] != '|'; i++) ;
		if (word[i] == '|' && i > 1)
			strcpy(word+1, word+i);
	}
}

static long anat_getword(register char *orgWord, char *tier, long i) {
	register long temp;
	register char sq;
	register char *word;

	word = orgWord;
	if (tier[i] == EOS)
		return(0);
	if (i > 0)
		i--;
	while ((*word=tier[i]) != EOS && uS.isskip(tier,i,&dFnt,MBF) && !uS.isRightChar(tier,i,'[',&dFnt,MBF)) {
		i++;
		if (*word == '<') {
			temp = i;
			for (i++; tier[i] != '>' && tier[i]; i++) {
				if (isdigit(tier[i])) ;
				else if (tier[i]== ' ' || tier[i]== '\t' || tier[i]== '\n') ;
				else if ((i-1 == temp+1 || !isalpha(tier[i-1])) &&
						 tier[i] == '-' && !isalpha(tier[i+1])) ;
				else if ((i-1 == temp+1 || !isalpha(tier[i-1])) &&
						 (tier[i] == 'u' || tier[i] == 'U') &&
						 !isalpha(tier[i+1])) ;
				else if ((i-1 == temp+1 || !isalpha(tier[i-1])) &&
						 (tier[i] == 'w' || tier[i] == 'W') &&
						 !isalpha(tier[i+1])) ;
				else if ((i-1 == temp+1 || !isalpha(tier[i-1])) &&
						 (tier[i] == 's' || tier[i] == 'S') &&
						 !isalpha(tier[i+1])) ;
				else break;
			}
			if (tier[i] == '>')
				i++;
			else
				i = temp;
		}
	}
	if (*word == EOS)
		return(0);
	if (uS.isRightChar(tier, i, '[',&dFnt,MBF)) {
		if (uS.isSqBracketItem(tier, i+1, &dFnt, MBF)) sq = TRUE;
		else sq = FALSE;
	} else sq = FALSE;
getword_rep:

	while ((*++word=tier[++i]) != EOS && (!uS.isskip(tier,i,&dFnt,MBF) || sq)) {
		if (uS.isRightChar(tier, i, ']', &dFnt, MBF)) {
			*++word = EOS;
			if (*orgWord == '[')
				uS.cleanUpCodes(orgWord, &dFnt, MBF);
			return(i+1);
		}
	}
	if (uS.isRightChar(tier, i, '[', &dFnt, MBF)) {
		if (!uS.isSqBracketItem(tier, i+1, &dFnt, MBF))
			goto getword_rep;
	}
	*word = EOS;
	anat_cleanUpVerbs(orgWord);
	if (tier[i] != EOS)
		i++;
	return(i);
}

static void findSlaveWords(struct anat_tnode *master_word, char *slave_tier) {
	long slave_i;
	char word[BUFSIZ];
	struct slave *t;

	slave_i = 0;
	while ((slave_i=anat_getword(word, slave_tier, slave_i))) {
		for (t=master_word->slaves; t != NULL; t=t->nextSlave) {
			if (!strcmp(word, t->word)) {
				t->count++;
			}
		}
	}
}

static void anatCleanUpTiers(char *tier) {
	char *s1, *s2;

	for (s1=tier; *s1 != EOS; s1++) {
		if (*s1 == '~' || *s1 == '^' || *s1 == '\n' || *s1 == '\t')
			*s1 = ' ';
	}

	s1 = strchr(tier, '|');
	while (s1 != NULL) {
		s2 = s1 + 1;

		for (s1--; (isalnum(*s1) || UTF8_IS_SINGLE((unsigned char)*s1)) && s1 > tier; s1--) ;
		for (; !isSpace(*s1) && s1 > tier; s1--)
			*s1 = ' ';

		for (tier=s2; isalnum(*tier) || !UTF8_IS_SINGLE((unsigned char)*tier); tier++) ;
		for (; !isSpace(*tier) && *tier != EOS; tier++)
			*tier = ' ';
		
		s1 = strchr(tier, '|');
	}
}

void call() {
	long master_i, i;
	char isMasterTier, isSlaveTier;
	char word[BUFSIZ];
	long tlineno = 0;
	struct anat_tnode *master_word;

	isMasterTier = FALSE;
	isSlaveTier = FALSE;
	master_tier[0] = EOS;
	slave_tier[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (lineno > tlineno) {
			tlineno = lineno + 200;
			fprintf(stderr,"\r%ld ",lineno);
		}
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (uS.partcmp(utterance->speaker,master_sp, FALSE, TRUE)) {
			if (isMasterTier) {
				fputs("\n----------------------------------------\n",stderr);
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Found two master tiers in a row\n");
				if (anat_RArr != NULL)
					free(anat_RArr);
				freeTree(anat_root);
				cutt_exit(0);
			}
			isMasterTier = TRUE;
		}
		if (uS.partcmp(utterance->speaker,slave_sp, FALSE, TRUE)) {
			if (isSlaveTier) {
				fputs("\n----------------------------------------\n",stderr);
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr, "Found two slave tiers in a row\n");
				if (anat_RArr != NULL)
					free(anat_RArr);
				freeTree(anat_root);
				cutt_exit(0);
			}
			isSlaveTier = TRUE;
		}
		if (uS.partcmp(utterance->speaker,"%mor:", FALSE, TRUE)) {
			if (isMasterTier && master_tier[0] == EOS)
				strcpy(master_tier, uttline);
			if (isSlaveTier && slave_tier[0] == EOS)
				strcpy(slave_tier, uttline);
		}
		if (master_tier[0] != EOS && slave_tier[0] != EOS) {
			anatCleanUpTiers(master_tier);
			anatCleanUpTiers(slave_tier);
			master_i = 0L;
			found_master_words_cnt = 0L;
			while ((master_i=anat_getword(word, master_tier, master_i))) {
				if ((master_word=anat_BSearch(word)) != NULL) {
					if (found_master_words_cnt >= 500) {
						fputs("\n----------------------------------------\n",stderr);
						fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
						fprintf(stderr, "Internal error an array of \"found_master_words\" was exceded\n");
						if (anat_RArr != NULL)
							free(anat_RArr);
						freeTree(anat_root);
						cutt_exit(0);
					}
					for (i=0L; i < found_master_words_cnt && found_master_words[i] != master_word; i++) ;
					if (i >= found_master_words_cnt) {
						findSlaveWords(master_word, slave_tier);
						found_master_words[found_master_words_cnt++] = master_word;
					}
				}
			}
			isMasterTier = FALSE;
			isSlaveTier = FALSE;
			master_tier[0] = EOS;
			slave_tier[0] = EOS;
		}
	}
	fprintf(stderr, "\r	  \r");
	if (!combinput)
		anat_pr_result();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = TEMP;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,anat_pr_result);
	if (anat_RArr != NULL)
		free(anat_RArr);
	freeTree(anat_root);
}
