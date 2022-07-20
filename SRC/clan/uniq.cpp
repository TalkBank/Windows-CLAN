/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "cu.h"

#if !defined(UNX)
#define _main uniq_main
#define call uniq_call
#define getflag uniq_getflag
#define init uniq_init
#define usage uniq_usage
#define pr_result uniq_pr_result
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

struct tnode		/* binary uniq_tree for word and count */
{
	char *word;
	int count;
	struct tnode *left;
	struct tnode *right;
};

static char uniq_sort = 0;
static long uniq_total;				/* total number of words */
static long uniq_diff;
static struct tnode *uniq_root;

void usage()	{		/* print proper usage and exit */
    puts("UNIQ reports repeated lines in a file");
    printf("Usage : uniq [o %s] filename(s)\n",mainflgs());
    puts("-o : sort output by descending frequency");
    mainusage(TRUE);
}

void init(char f) {
	if(f) {
		uniq_total = 0L;
		uniq_diff = 0L;
		chatmode = 0;
	}
	if (f || !combinput) {
		uniq_total = 0L;
		uniq_diff = 0L;
		uniq_root = NULL;
	}
}

static void uniq_overflow(void) {
    fprintf(stderr,"Uniq: no more core available.\n");
    cutt_exit(1);
}

static struct tnode *uniq_talloc(void) {
    struct tnode *p;

    if ((p=NEW(struct tnode)) == NULL) uniq_overflow();
    return(p);
}

static char *uniq_strsave(char *s) {
    char *p;

    if ((p=(char *)malloc(strlen(s)+1)) == NULL) uniq_overflow();
    strcpy(p, s);
    return(p);
}

static void uniq_remove_endspace(char *line) {
    register int i;

    i = strlen(line) - 1;
	if (line[i] == '\n' && i >= 0) {
		line[i] = EOS;
		i--;
	} else if (i >= UTTLINELEN-1)
		fputs("Line too long  : end ignored.\n", stderr);
    for (; (line[i] == ' ' || line[i] == '\t') && i >= 0; i--) ;
    line[++i] = EOS;
}

static void uniq_remove_startspace(char *line) { 
    char *beg;

    for (beg=line; *line == ' ' || *line == '\t'; line++);
    if (beg != line)
		strcpy(beg, line);
}

static struct tnode *uniq_stree(struct tnode *p, struct tnode *m) {
    struct tnode *t = p;

    if (p == NULL) {
		p = uniq_talloc();
		p->word = uniq_strsave(m->word);
		p->count = m->count;
		p->left = p->right = NULL;
    } else if (p->count < m->count)
    	p->left = uniq_stree(p->left, m);
    else if (p->count == m->count) {
		for (; p->count>= m->count && p->right!= NULL; p=p->right) ;
		if (p->count < m->count)
			p->left = uniq_stree(p->left, m);
		else
			p->right = uniq_stree(p->right, m);
		return(t);
    } else
    	p->right = uniq_stree(p->right, m);
    return(p);
}

static void uniq_sorttree(struct tnode *p) {
    if (p != NULL) {
		uniq_sorttree(p->left);
		uniq_root = uniq_stree(uniq_root,p);
		uniq_sorttree(p->right);
		free(p->word);
		free(p);
    }
}

static void uniq_treeprint(struct tnode *p, FILE *fp) {
    struct tnode *t;

    if (p != NULL) {
		uniq_treeprint(p->left, fp);
		do {
		    if (!onlydata)
				fprintf(fp, "%5d  ", p->count);
		    fprintf(fpout,"%s\n", p->word);
		    if (p->right == NULL) break;
		    if (p->right->left != NULL) {
				uniq_treeprint(p->right, fp);
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

static void uniq_treesort(struct tnode *p, FILE *fpout) {
    uniq_root = NULL;
    uniq_sorttree(p);
    uniq_treeprint(uniq_root,fpout);
}

static void pr_result() {
    if (uniq_sort)
		uniq_treesort(uniq_root, fpout);
    else
		uniq_treeprint(uniq_root, fpout);
	if (!onlydata)
		fprintf(fpout, "Unique number: %ld    Total number: %ld\n", uniq_diff, uniq_total);
}

static struct tnode *uniq_tree(struct tnode *p, char *w) {
    int cond;
    struct tnode *t = p;

    if (p == NULL) {
		uniq_diff++;
		p = uniq_talloc();
		p->word = uniq_strsave(w);
		p->count = 1;
		p->left = p->right = NULL;
    } else if ((cond=strcmp(w, p->word)) == 0) {
		p->count++;
    } else if (cond < 0) {
		p->left = uniq_tree(p->left, w);
    } else {
		for (; (cond=strcmp(w, p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0)
			p->count++;
		else if (cond < 0)
			p->left = uniq_tree(p->left, w);
		else
			p->right = uniq_tree(p->right, w);
		return(t);
    }
    return(p);
}

void call() {
    long ln = 0L;

    while (fgets_cr(uttline, UTTLINELEN, fpin) != NULL) {
		ln++;
		if (ln % 200 == 0) {
		    fprintf(stderr,"\r%ld    ", ln);
		}
		uniq_remove_endspace(uttline);
		uniq_remove_startspace(uttline);
		if (*uttline != EOS) {
		    if (!nomap)
		    	uS.lowercasestr(uttline, &dFnt, MBF);
		    if (exclude(uttline)) {
				uS.remFrontAndBackBlanks(uttline);
				if (uttline[0] != EOS) {
					uniq_total++;
					uniq_root = uniq_tree(uniq_root,uttline);
				}
		    }
		}
    }
    fprintf(stderr,"\n");
    if (!combinput)
    	pr_result();
}

void getflag(char *f, char *f1, int *i) {
    f++;
    switch(*f++) {
	case 'o':
		uniq_sort = 1;
		break;
	case 'y':
		break;
	default:
		maingetflag(f-2,f1,i);
		break;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    chatmode = CHAT_MODE;
    CLAN_PROG_NUM = UNIQ;
    OnlydataLimit = 1;
    UttlineEqUtterance = TRUE;
    bmain(argc,argv,pr_result);
}
