/**********************************************************************
	"Copyright 2006 Leonid Spektor. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

extern struct tier *defheadtier;
extern char OverWriteFile;

struct my_tnode {
	char *head;
	char *body;
	struct my_tnode *left;
	struct my_tnode *right;
};

static struct my_tnode *root;

void init(char f) {
	if (f) {
		root = NULL;
		OverWriteFile = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
		}
		defheadtier = NULL;
	}
	combinput = TRUE;
}

void usage() {
	printf("test.\n");
	mainusage();
}

static void my_treeprint(struct my_tnode *p) {
	struct my_tnode *t;
	
	if (p != NULL) {
		my_treeprint(p->left);
		do {
			fprintf(fpout,"%s\t\t\t%s\n", p->head, p->body);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				my_treeprint(p->right);
				break;
			}
			t = p;
			p = p->right;
			free(t->head);
			free(t->body);
			free(t);
		} while (1);
		free(p->head);
		free(p->body);
		free(p);
	}
}

static void my_pr_result(void) {
	my_treeprint(root);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,my_pr_result);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char *my_strsave(char *s) {
	char *p;
	
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else {
		fprintf(stderr,"my: no more memory available.\n");
		cutt_exit(1);
	}
	return(p);
}

static struct my_tnode *my_talloc(char *head, char *body) {
	struct my_tnode *p;
	
	if ((p=NEW(struct my_tnode)) == NULL) {
		fprintf(stderr,"my: no more memory available.\n");
		cutt_exit(1);
	}
	p->head = head;
	p->body = body;
	return(p);
}

static struct my_tnode *my_tree(struct my_tnode *p, char *head, char *body) {
	int cond;
	
	if (p == NULL) {
		p = my_talloc(my_strsave(head),my_strsave(body));
		p->left = p->right = NULL;
	} else if ((cond=uS.mStricmp(head,p->head)) == 0)
		return(p);
	else if (cond < 0)
		p->left = my_tree(p->left, head, body);
	else
		p->right = my_tree(p->right, head, body); /* if cond > 0 */
	return(p);
}

void call() {
	int i, j;

	while (fgets_cr(uttline, UTTLINELEN, fpin)) {
		uS.remblanks(uttline);
		for (i=0; isSpace(uttline[i]); i++) ;
		if (i > 0)
			strcpy(uttline, uttline+i);
		for (i=0; !isSpace(uttline[i]) && uttline[i] != EOS; i++) {
			if (islower(uttline[i]))
				uttline[i] = toupper(uttline[i]);
		}
		if (uttline[i] != EOS) {
			uttline[i] = EOS;
			i++;
			for (j=i; uttline[j] != EOS; j++) {
				if ((j == 0 || isSpace(uttline[j-1])) && islower(uttline[j]))
					uttline[j] = toupper(uttline[j]);
			}
			root = my_tree(root, uttline, uttline+i);
		}
	}
}
