/**********************************************************************
	"Copyright 1990-2012 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "cu.h"

#if !defined(UNX)
#define _main statfreq_main
#define call statfreq_call
#define getflag statfreq_getflag
#define init statfreq_init
#define usage statfreq_usage
#endif

#define WordsHead statfreq_WordsHead
#define IS_WIN_MODE FALSE
#include "mul.h"

extern char OverWriteFile;
extern char *AddCEXExtension;

struct W_Struct {
    char *word;
    int count;
    struct W_Struct *left;
    struct W_Struct *right;
};

static struct W_Struct *WordsHead;

void usage() {
    puts("STATFREQ creates a special format output");
    printf("Usage: statfreq [%s] filename\n", mainflgs());
    mainusage(TRUE);
}

void init(char first) {
    if (first) {
//		OverWriteFile = TRUE;
		AddCEXExtension = ".xls";
		stout = FALSE;
		onlydata = 1;
    }
}

void getflag(char *f, char *f1, int *i) {
	maingetflag(f,f1,i);
}

static struct W_Struct *StoreWords(char isOutput, struct W_Struct *p, char *w, int count) {
    int cond;

    if (p == NULL) {
		if ((p = NEW(struct W_Struct)) == NULL)
			out_of_mem();
		if ((p->word = (char *)malloc(strlen(w)+1)) == NULL)
			out_of_mem();
		strcpy(p->word, w);
		p->count = 0;
		p->left = p->right = NULL;
    } else if ((cond = strcmp(w, p->word)) == 0) {
		if (isOutput)
			p->count = count;	
    } else if (cond < 0)
    	p->left = StoreWords(isOutput, p->left, w, count);
    else
    	p->right = StoreWords(isOutput, p->right, w, count); /* if cond > 0 */
    return(p);
}

static void print_table(struct W_Struct *p, char prname) {
	int i;

    if (p != NULL) {
		print_table(p->left,prname);
		do {
		    if (prname) {
				strcpy(uttline, p->word);
				for (i=0; uttline[i]; i++) {
					if (uttline[i] == '\t')
						uttline[i] = ' ';
				}
				fprintf(fpout,"%s\t",uttline);
		    } else {
				fprintf(fpout,"%d\t",p->count);
			}
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				print_table(p->right,prname);
				break;
		    }
		    p->count = 0;
		    p = p->right;
		} while (1);
		p->count = 0;
    }
}

static void statfreq_pr_result(char *fname, char *id, char *stats, char isPadIDs) {
	fprintf(fpout, "%s\t", fname);
	fprintf(fpout,"%s", id);
	if (isPadIDs)
		fprintf(fpout,".\t", id);
    print_table(WordsHead,FALSE);
    fprintf(fpout, "%s", stats);
    putc('\n', fpout);
}

static void statfreq_cleanup(struct W_Struct *p) {
    struct W_Struct *t;

    if (p != NULL) {
		statfreq_cleanup(p->left);
		do {
		    if (p->right == NULL)
		    	break;
		    if (p->right->left != NULL) {
				statfreq_cleanup(p->right);
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

static void readinfile(char isOutput) {
    int i, cnt;
    char *word;
	char isPadIDs, isMissingSpeaker;
	char fname[BUFSIZ];
	char id[BUFSIZ];
	char stats[BUFSIZ];
	
	*fname = EOS;
	*id = EOS;
	*stats = EOS;
	isPadIDs = FALSE;
	isMissingSpeaker = TRUE;
    while (!feof(fpin)) {
		if (fgets_cr(uttline, UTTLINELEN, fpin) != NULL) {
			if (uS.isUTF8(uttline) || uS.partcmp(uttline, FONTHEADER, FALSE, FALSE))
				continue;
		    uttline[strlen(uttline)-1] = EOS;
		    if (uS.partcmp(uttline,"@FILENAME:",FALSE,FALSE)) {
				if (isOutput && stats[0] != EOS) {
					if (id[0] == EOS) {
						if (isMissingSpeaker)
							strcpy(id, "SPEAKER NOT FOUND\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
						else
							strcpy(id, "@ID NOT FOUND\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
					}
					statfreq_pr_result(fname, id, stats, isPadIDs);
					*fname = EOS;
					*id = EOS;
					*stats = EOS;
					isPadIDs = FALSE;
					isMissingSpeaker = TRUE;
				}
				fgets_cr(fname, BUFSIZ, fpin);
				fname[strlen(fname)-1] = EOS;
			} else if (uS.partcmp(uttline,"@ID:",FALSE,FALSE)) {
				fgets_cr(id, BUFSIZ, fpin);
				id[strlen(id)-1] = EOS;
				cnt = 0;
				for (i=0; id[i]; i++) {
					if (id[i] =='\t')
						id[i] = '_';
					else if (id[i] == '|') {
						cnt++;
						id[i] = '\t';
					}
				}
				if (cnt < 10)
					isPadIDs = TRUE;
				else
					isPadIDs = FALSE;
			} else if (uS.partcmp(uttline,"@ST:",FALSE,FALSE)) {
				fgets_cr(stats, BUFSIZ, fpin);
				stats[strlen(stats)-1] = EOS;
				for (i=0; isSpace(stats[i]); i++) ;
				if (i > 0)
					strcpy(stats, stats+i);
				for (i=0; stats[i]; i++) {
					if (isSpace(stats[i])) {
						stats[i] = '\t';
					} else if (stats[i] == '-')
						stats[i] = '.';
				}
		    } else if (uS.partcmp(uttline,"@SPEAKER_FOUND@",FALSE,FALSE)) {
				isMissingSpeaker = FALSE;
		    } else {
				for (word=uttline; *word != ' ' && *word; word++) ;
				for (; *word == ' '; word++) ;
				if (*word) 
				    WordsHead = StoreWords(isOutput, WordsHead, word, atoi(uttline));
		    }
		}
    }
	if (isOutput && stats[0] != EOS) {
		if (id[0] == EOS) {
			if (isMissingSpeaker)
				strcpy(id, "SPEAKER NOT FOUND\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
			else
				strcpy(id, "@ID NOT FOUND\t.\t.\t.\t.\t.\t.\t.\t.\t.\t");
		}
		statfreq_pr_result(fname, id, stats, isPadIDs);
	}
}

void call() {
	WordsHead = NULL;
	readinfile(FALSE);
	rewind(fpin);
	fputs("file\tlanguage\tcorpus\tspeaker\tage\tsex\tgroup\tSES\trole\tEducation\tUnique File ID\t", fpout);
    print_table(WordsHead,TRUE);
	fprintf(fpout, "Types\tToken\tTTR");
    putc('\n', fpout);
	readinfile(TRUE);
    statfreq_cleanup(WordsHead);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    CLAN_PROG_NUM = STATFREQ;
    chatmode = CHAT_MODE;
    OnlydataLimit = 0;
    UttlineEqUtterance = TRUE;
	fprintf(stderr, "\nThis program is no longer needed.\n");
	fprintf(stderr, "Please look at output created by \"freq +d2\" or \"freq +d3\" command.\n\n");
//    bmain(argc,argv,NULL);
}
