/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main cooccur_main
#define call cooccur_call
#define getflag cooccur_getflag
#define init cooccur_init
#define usage cooccur_usage
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

#define MAXNUMWORDS 20

struct Location {
	FNType *filename;
	long lineno;
	struct Location *next;
} ;

struct Cluster_s {
	unsigned int num_occ;
	char *cluster;
	struct Location *loc;
	struct Cluster_s *left, *right;
} *root_cluster;

extern char OverWriteFile;
extern char isRecursive;

static char cooccur_ftime;
static char *wheel[MAXNUMWORDS];
static char isSort = FALSE;
static short matchLoc;
static int notch = 2;		/* number of words rolling around wheel */

void usage() {
	puts("COOCCUR rolls through text a word cluster at a time and prints them.");
	printf("Usage: cooccur [l nN o %s] filename(s)\n",mainflgs());
	puts("+b: match words specified by +/-s only at the beginning of cluster");
	puts("-b: match words specified by +/-s only at the end of cluster");
	puts("+nN: print clusters of N words (default 2)");
	puts("+o : sort output by descending frequency");
	mainusage(TRUE);
}

void init(char f) {
	int i;

	if (f) {
		maininitwords();
		mor_initwords();
		isSort = FALSE;
		notch = 2;
		matchLoc = 0;
		cooccur_ftime = TRUE;
	} else {
		FilterTier = 1;
		if (cooccur_ftime) {
			cooccur_ftime = FALSE;
			for (i=0; i < notch; i++) {
				if ((wheel[i]=(char *)malloc(BUFSIZ+1)) == NULL) {
					for (i=0; i < MAXNUMWORDS; i++) {
						if (wheel[i] != NULL)
							free(wheel[i]);
					}
					fprintf(stderr,"Error: out of memory\n");
					cutt_exit(0);
				}
			}
		}
	}

	if (!combinput || f)
		root_cluster = NULL;
	if (combinput || isRecursive)
		OverWriteFile = TRUE;
}

static void AddLoc(struct Cluster_s *p) {
	struct Location *loc;

	if (p->loc == NULL) {
		if ((loc=NEW(struct Location)) == NULL)
			out_of_mem();
		p->loc = loc;
	} else {
		for (loc=p->loc; loc->next != NULL; loc=loc->next) ;
		if ((loc->next=NEW(struct Location)) == NULL)
			out_of_mem();
		loc = loc->next;
	}
	loc->filename = (FNType *)malloc((strlen(oldfname)+1)*sizeof(FNType));
	if (loc->filename == NULL)
		out_of_mem();
	strcpy(loc->filename, oldfname);
	loc->lineno = lineno;
	loc->next = NULL;
}

static struct Cluster_s *AddToCluster(char *w) {
	struct Cluster_s *p;

	if ((p=NEW(struct Cluster_s)) == NULL)
		out_of_mem();
	p->cluster = (char *)malloc(strlen(w)+1);
	if (p->cluster == NULL)
		out_of_mem();
	strcpy(p->cluster, w);
	p->num_occ = 1;
	p->loc = NULL;
	if (onlydata == 2)
		AddLoc(p);
	p->left = p->right = NULL;
	return(p);
}

static struct Cluster_s *cooccur_tree(struct Cluster_s *p, char *w) {
	int cond;

	if (p == NULL)
		p = AddToCluster(w);
	else if ((cond = strcmp(w, p->cluster)) == 0) {
		p->num_occ++;
		if (onlydata == 2)
			AddLoc(p);
	} else if (cond < 0)
		p->left = cooccur_tree(p->left, w);
	else
		p->right = cooccur_tree(p->right, w);
	return(p);
}

static struct Cluster_s *cooccur_stree(struct Cluster_s *p, struct Cluster_s *m) {
	struct Cluster_s *t = p;

	if (p == NULL) {
		if ((p=NEW(struct Cluster_s)) == NULL)
			out_of_mem();
		p->cluster = m->cluster;
		p->num_occ = m->num_occ;
		if (onlydata == 2)
			p->loc = m->loc;
		else
			p->loc = NULL;
		p->left = p->right = NULL;
	} else if (p->num_occ < m->num_occ)
		p->left = cooccur_stree(p->left, m);
	else if (p->num_occ == m->num_occ) {
		for (; p->num_occ>= m->num_occ && p->right!= NULL; p=p->right) ;
		if (p->num_occ < m->num_occ)
			p->left = cooccur_stree(p->left, m);
		else
			p->right = cooccur_stree(p->right, m);
		return(t);
	} else
		p->right = cooccur_stree(p->right, m);
	return(p);
}

static void cooccur_sorttree(struct Cluster_s *p) {
	if (p != NULL) {
		cooccur_sorttree(p->left);
		root_cluster = cooccur_stree(root_cluster, p);
		cooccur_sorttree(p->right);
		free(p);
	}
}

static void PrintLocation(struct Location *p) {
	struct Location *t;

	while (p != NULL) {
		fprintf(fpout,"      File \"%s\": line %ld.\n", p->filename, p->lineno);
		t = p;
		p = p->next;
		free(t->filename);
		free(t);
	}
}

static void PrintClusters(struct Cluster_s *cluster) {
	if (cluster != NULL) {
		PrintClusters(cluster->left);
		if (onlydata == 0 || onlydata == 2)
			fprintf(fpout, "%3u  ", cluster->num_occ);
		fprintf(fpout,"%s\n", cluster->cluster);
		if (onlydata == 2)
			PrintLocation(cluster->loc);
		PrintClusters(cluster->right);
		free(cluster->cluster);
		free(cluster);
	}
}

static void cooccur_pr_result(void) {
	if (isSort) {
		struct Cluster_s *p;
		p = root_cluster;
		root_cluster = NULL;
		cooccur_sorttree(p);
		PrintClusters(root_cluster);
	} else
		PrintClusters(root_cluster);
}

void call() {	/* roll word wheel through text */
	int i, j, k;
	char found;
	char word[BUFSIZ+1];
	long tlineno = 0;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	*spareTier1 = EOS;
	for (i=0; i < notch; i++)
		wheel[i][0] = EOS;
	while (getwholeutter()) {
		if (lineno > tlineno) {
			tlineno = lineno + 200;
#if !defined(CLAN_SRV)
			fprintf(stderr,"\r%ld ",lineno);
#endif
		}
/*
printf("#%ld; sp=%s; uttline=%s", lineno, utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		k = 0;
		for (i=0; i < notch; i++) {
			if ((k=getword(utterance->speaker, uttline, word, NULL, k)) == 0)
				break;
			strncpy(wheel[i], word, BUFSIZ);
			wheel[i][BUFSIZ] = EOS;
		}
		found = FALSE;
		for (j=0; j < i; j++) {
			if (exclude(wheel[j]))
				found = TRUE;
			if (j != 0)
				strcat(spareTier1, " ");
			strcat(spareTier1, wheel[j]);
		}
		for (j=0; spareTier1[j] == ' ' || spareTier1[j] == '\t'; j++) ;
		if (spareTier1[j] && found && i == notch) {
			if (matchLoc > 0) {
				if (!exclude(wheel[0]))
					found = FALSE;
			} else if (matchLoc < 0) {
				if (notch > 0 && !exclude(wheel[notch-1]))
					found = FALSE;
			}
			if (found)
				root_cluster = cooccur_tree(root_cluster, spareTier1+j);
		}
		*spareTier1 = EOS;
		if (k > 0) {
			while ((k=getword(utterance->speaker, uttline, word, NULL, k))) {
				for (i=0; i < notch-1; i++)
					strcpy(wheel[i], wheel[i+1]);
				strncpy(wheel[notch-1], word, BUFSIZ);
				wheel[notch-1][BUFSIZ] = EOS;
				found = FALSE;
				for (i=0; i < notch; i++) {
					if (exclude(wheel[i]))
						found = TRUE;
					if (i != 0)
						strcat(spareTier1, " ");
					strcat(spareTier1, wheel[i]);
				}
				for (j=0; spareTier1[j] == ' ' || spareTier1[j] == '\t'; j++) ;
				if (spareTier1[j] && found) {
					if (matchLoc > 0) {
						if (!exclude(wheel[0]))
							found = FALSE;
					} else if (matchLoc < 0) {
						if (notch > 0 && !exclude(wheel[notch-1]))
							found = FALSE;
					}
					if (found)
						root_cluster=cooccur_tree(root_cluster,spareTier1+j);
				}
				*spareTier1 = EOS;
			}
		}
	}
#if !defined(CLAN_SRV)
	fprintf(stderr, "\r	  \r");
#endif
	if (!combinput)
		cooccur_pr_result();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int i;

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = COOCCUR;
	OnlydataLimit = 2;
	UttlineEqUtterance = FALSE;
	for (i=0; i < MAXNUMWORDS; i++)
		wheel[i] = NULL;
	bmain(argc,argv,cooccur_pr_result);
	for (i=0; i < MAXNUMWORDS; i++) {
		if (wheel[i] != NULL)
			free(wheel[i]);
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch (*f++) {
		case 'b':
			if (*(f-2) == '+') {
				matchLoc = 1;
			} else {
				matchLoc = -1;
			}
			no_arg_option(f);
			break;
		case 'n':
			if ((notch=atoi(f)) > MAXNUMWORDS) {
				fprintf(stderr,"Clusters must be smaller than %d.", MAXNUMWORDS);
				cutt_exit(0);
			}
			break;
		case 'o':
			isSort = TRUE;
			no_arg_option(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
