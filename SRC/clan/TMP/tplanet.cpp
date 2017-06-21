#define CHAT_MODE 0

#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

/* ******************** ab prototypes ********************** */
void isInputValid(void);
void ReadCells(void);
void PrintThem(void);
void PrintDoubleCheck(void);
void cleanupCells(void);
void cleanupBlocks(struct blocks *p);
void cleanupAtts(struct atts *p);
void cleanupSent(struct sents *p);
void cleanupComps(struct comps *p);
void Permute(void);
void PermuteBlock(struct blocks *block);
char *GetNameAndProb(int i, double *prob);
/* *********************************************************** */

struct atts {
	char  *name;
	double percent;
	struct atts *nextAtt;
} ;

struct blocks {
	struct atts *att;
	struct blocks *nextBlock;
} ;

struct cells {
	char  *name;
	double percent;
	struct blocks *block;
	struct cells *nextCell;
} *cell;

struct comps {
	struct atts *att;
	struct comps *nextComp;
} ;

struct sents {
	double prob;
	struct cells *cell;
	struct comps *comp;
	struct sents *nextSent;
} *sent;


unsigned int MinNumOfLine;
double cutoff;
double minProb;
char  temp[BUFSIZ];
int stackIndex, debug;
struct atts *stackAtt[25];
struct cells *stackCell;

void init(char f) {
	if (f) {
		cell = NULL;
		sent = NULL;
		chatmode = 0;
		MinNumOfLine = 0;
		cutoff = 0.0;
		minProb = 1.0;
		debug = 0;
	} else {
		cleanupCells();
		cell = NULL;
		cleanupSent(sent);
		sent = NULL;
	}
}

void usage() {
	printf("Usage: planet [-cN -dN -lN %s] filename(s)\n",mainflgs());
	printf("-cN: Set cutoff point to N (default: 0.0)\n");
	printf("-dN: Set debugging level to N: 0-3 (default: 0)\n");
	printf("-lN: Set minimum number of lines to N (default: 0)\n");
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
	cleanupCells();
	cleanupSent(sent);
}
		
void getflag(char *f, char *f1, int *i) {
	unsigned int t;
	double tt;

	f++;
	switch(*f++) {
		case 'l':
				t = (unsigned int)atoi(f);
				if (t > 0) MinNumOfLine = t;
				else {
					fprintf(stderr, "Minimum number of lines must be greater than 0\n");
					cutt_exit(0);
				}
				break;
		case 'd':
				t = atoi(f);
				debug = t;
				break;
		case 'c':
				tt = (double)atof(f);
				if (tt > 0) cutoff = tt;
				else {
					fprintf(stderr, "Cutoff point must be greater than 0\n");
					cutt_exit(0);
				}
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}

void cleanupComps(struct comps *p) {
	struct comps *t;

	while (p != NULL) {
		t = p;
		p = p->nextComp;
		free(t);
   }
}

void cleanupSent(struct sents *p) {
	struct sents *t;

	while (p != NULL) {
		t = p;
		p = p->nextSent;
 		cleanupComps(t->comp);
		free(t);
   }
}

void cleanupAtts(struct atts *p) {
	struct atts *t;

	while (p != NULL) {
		t = p;
		p = p->nextAtt;
 		free(t->name);
		free(t);
   }
}

void cleanupBlocks(struct blocks *p) {
	struct blocks *t;

	while (p != NULL) {
		t = p;
		p = p->nextBlock;
		cleanupAtts(t->att);
		free(t);
   }
}

void cleanupCells(void) {
	struct cells *t;

	while (cell != NULL) {
		t = cell;
		cell = cell->nextCell;
 		free(t->name);
		cleanupBlocks(t->block);
		free(t);
   }
}

char *GetNameAndProb(int i, double *prob) {
	int j, k;

	for (j=i; !isdigit(temp[j]) && temp[j]; j++) ;
	if (temp[j] == EOS) {
		fprintf(stderr, "Incomplete information in \"%s\" file\n", oldfname);
		cleanupCells();
		cleanupSent(sent);
		cutt_exit(0);
	}
	if ((temp[j-1] != ' ' && temp[j-1] != '\t') || j-1 <= i) {
		fprintf(stderr, "Data must be separated with a space in \"%s\" file\n", oldfname);
		cleanupCells();
		cleanupSent(sent);
		cutt_exit(0);
	}
	for (k=j-1; k >= i && (temp[k] == ' ' || temp[k] == '\t'); k--) ; 
	temp[k+1] = EOS;
	*prob = (double)atoi(temp+j);
	*prob = *prob / 100.0;
	return(temp+i);
}


void ReadCells(void) {
	int	i;
	char   *name;
	char   isNextBlock = TRUE;
	double prob;
	struct cells *tcell = NULL;
	struct blocks *block;
	struct atts *att;
	
	
	while (fgets(temp, BUFSIZ, fpin)) {
		if (*temp == ';')
			continue;
		for (i=0; temp[i]; i++) {
			if (temp[i] != ' ' && temp[i] != '\t' && temp[i] != '\n') break;
		}
		if (temp[i] == EOS) {
			isNextBlock = TRUE;
		} else if (temp[i] == '>') {
			isNextBlock = TRUE;
			for (i++; temp[i] == ' ' || temp[i] == '\t'; i++) ;
			name = GetNameAndProb(i, &prob);
			if (cell == NULL) {
				if ((tcell=NEW(struct cells)) == NULL) out_of_mem();
				cell = tcell;
			} else {
				if ((tcell->nextCell=NEW(struct cells)) == NULL) out_of_mem();
				tcell = tcell->nextCell;
			}
			tcell->name = (char *)malloc(strlen(name)+1);
			if (tcell->name == NULL) out_of_mem();
			strcpy(tcell->name, name);
			tcell->percent = prob;
			tcell->block = NULL;
			tcell->nextCell = NULL;
		} else {
			if (isNextBlock) {
				if (tcell == NULL) {
					fprintf(stderr, "First attribute in a file doesn't start with '>'\n");
					exit(1);
				}
				if (tcell->block == NULL) {
					tcell->block = NEW(struct blocks);
					block = tcell->block;
				} else {
					for (block=tcell->block; block->nextBlock != NULL; block=block->nextBlock) ;
					block->nextBlock = NEW(struct blocks);
					block = block->nextBlock;
				}
				if (block == NULL) out_of_mem();
				block->nextBlock = NULL;
				block->att = NEW(struct atts);
				att = block->att;
				isNextBlock = FALSE;
			} else {
				att->nextAtt = NEW(struct atts);
				att = att->nextAtt;
			}
			if (att == NULL) out_of_mem();
			att->nextAtt = NULL;
			att->name = NULL;
			name = GetNameAndProb(i, &prob);
			att->name = (char *)malloc(strlen(name)+1);
			if (att->name == NULL) out_of_mem();
			strcpy(att->name, name);
			att->percent = prob;
		}
	}
}

void isInputValid(void) {
	double total, CellTotal;
	long t;
	struct cells *tcell;
	struct blocks *block;
	struct atts *att, *tatt;

	CellTotal = 0.0;
	for (tcell=cell; tcell != NULL; tcell=tcell->nextCell) {
		CellTotal += tcell->percent;
		for (block=tcell->block; block != NULL; block=block->nextBlock) {
			total = 0.0;
			for (att=block->att; att != NULL; att=att->nextAtt) {
				total += att->percent;
				tatt = att;
			}
			t = (long)(total * 10000.0);
			if (t < 9999 || t > 10001) {
				fprintf(stderr, "Total for any one block must be equal to 100%%\n");
				fprintf(stderr, "    Last item in a block was \"%s\"\n", tatt->name);
				cleanupCells();
				cleanupSent(sent);
				cutt_exit(0);
			}
		}
	}
	t = (long)(CellTotal * 10000.0);
	if (t < 9999 || t > 10001) {
		fprintf(stderr, "Total for top cells must be equal to 100%%\n");
		cleanupCells();
		cleanupSent(sent);
		cutt_exit(0);
	}
}

void PrintDoubleCheck(void) {
	struct cells *tcell;
	struct blocks *block;
	struct atts *att;

	for (tcell=cell; tcell != NULL; tcell=tcell->nextCell) {
		fprintf(fpout, "Top Cell: %s %2.0f%% (%.2f)\n\n", tcell->name, tcell->percent*100.0, tcell->percent);
		for (block=tcell->block; block != NULL; block=block->nextBlock) {
			for (att=block->att; att != NULL; att=att->nextAtt) {
				fprintf(fpout, "    name: %s %2.0f%% (%.2f)\n", att->name, att->percent*100.0, att->percent);
			}
			fprintf(fpout, "\n");
		}
	}
	fprintf(fpout, "************************************************\n\n");
}


void PermuteBlock(struct blocks *block) {
	int i;
	double prob;
	struct atts *att;
	struct comps *tcomp;
	struct sents *tsent;

	if (block == NULL) {
		if (sent == NULL) {
			tsent = NEW(struct sents);
			sent = tsent;
		} else {
			for (tsent=sent; tsent->nextSent != NULL; tsent=tsent->nextSent) ;
			tsent->nextSent = NEW(struct sents);
			tsent = tsent->nextSent;
		}
		if (tsent == NULL) out_of_mem();
		prob = stackCell->percent;
		tsent->nextSent = NULL;
		tsent->cell = stackCell;
		tsent->comp = NULL;
		for (i=0; i < stackIndex; i++) {
			if (tsent->comp == NULL) {
				tcomp = NEW(struct comps);
				tsent->comp = tcomp;
			} else {
				tcomp->nextComp = NEW(struct comps);
				tcomp = tcomp->nextComp;
			}
			if (tcomp == NULL) out_of_mem();
			tcomp->nextComp = NULL;
			tcomp->att = stackAtt[i];
			prob *= tcomp->att->percent;
		}
		if (prob < cutoff) prob = cutoff;
		tsent->prob = prob;
		if (prob < minProb) minProb = prob;
	} else {
		for (att=block->att; att != NULL; att=att->nextAtt) {
			stackAtt[stackIndex] = att;
			stackIndex++;
			PermuteBlock(block->nextBlock);
			stackIndex--;
		}
	}
}


void Permute(void) {
	struct cells *tcell;

	for (tcell=cell; tcell != NULL; tcell=tcell->nextCell) {
		stackIndex = 0;
		stackCell = tcell;
		PermuteBlock(tcell->block);
	}
	cutoff = minProb;
}

static void PrintLineAndResetInput(unsigned int prob, char *u) {
	int i;

	fprintf(fpout, "%s %u\n", u+1, prob);
	for (i=1; i < 13; i++)
		u[i] = '0';
}

static void noun(int i, int j, char *c, char *u) {
	u[2] = '1';
	if (c[i] == 'A') u[4] = '1';
	else u[5] = '1';
	if (c[j] == 'S') u[6] = '1';
	else u[7] = '1';
}

static void verb(int i, int j, char *c, char *u) {
	u[3] = '1';
	if (c[i] == 'S') u[6] = '1';
	else if (c[i] == 'P') u[7] = '1';
	if (c[j] == 'X') u[11] = '1';
	else u[10] = '1';
}

static void nv(unsigned int prob, char *c, char *u) {
	int i, j;

	i = 2; j = 3;
	noun(i, j, c, u);
	if (c[4] == 'C') u[8] = '1';
	u[17] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[1] = '1';
	i = 7; j = 8;
	verb(i, j, c, u);
	if (c[7] == 'N' || c[7] == c[3]) u[35] = '1';
	u[33] = '1';
	PrintLineAndResetInput(prob, u);
}

static void vn(unsigned int prob, char *c, char *u) {
	int i, j;

	i = 3; j = 4;
	verb(i, j, c, u);
	u[33] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[1] = '1';
	i = 6; j = 7;
	noun(i, j, c, u);
	if (c[8] == 'C') u[9] = '1';
	u[35] = '1';
	u[16] = '1';
	PrintLineAndResetInput(prob, u);
}

static void nvn(unsigned int prob, char *c, char *u) {
	int i, j;

	i = 2; j = 3;
	noun(i, j, c, u);
	if (c[4] == 'C') u[8] = '1';
	if (c[8] == 'X') u[16] = '1';
	else u[15] = '1';
	u[17] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 7; j = 8;
	verb(i, j, c, u);
	u[33] = '1';
	if (c[7] == 'N' || c[7] == c[3]) u[35] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[1] = '1';
	i = 10; j = 11;
	noun(i, j, c, u);
	if (c[12] == 'C') u[9] = '1';
	if (c[8] == 'X') u[21] = '1';
	else u[22] = '1';
	if (c[7] == 'N' || c[7] == c[11]) u[36] = '1';
	PrintLineAndResetInput(prob, u);
}

static void tvnvn(unsigned int prob, char *c, char *u) {
	int i, j;

	i = 2; j = 3;
	noun(i, j, c, u);
	if (c[4] == 'C') u[8] = '1';
	u[15] = '1';
	u[17] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[12] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 7; j = 8;
	verb(i, j, c, u);
	u[34] = '1';
	if (c[7] == 'N' || c[7] == c[3]) u[35] = '1';
	u[18] = '1';
	u[20] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 10; j = 11;
	noun(i, j, c, u);
	if (c[12] == 'C') u[9] = '1';
	if (c[7] == 'N' || c[7] == c[11]) u[36] = '1';
	u[25] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 15; j = 16;
	verb(i, j, c, u);
	u[38] = '1';
	if (c[15] == 'N' || c[15] == c[3]) u[40] = '1';
	if (c[15] == 'N' || c[15] == c[11]) u[41] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[1] = '1';
	i = 18; j = 19;
	noun(i, j, c, u);
	if (c[20] == 'C') u[9] = '1';
	if (c[7] == 'N' || c[7] == c[19]) u[37] = '1';
	if (c[15] == 'N' || c[15] == c[19]) u[42] = '1';
	u[28] = '1';
	PrintLineAndResetInput(prob, u);
}

static void nvtvn(unsigned int prob, char *c, char *u) {
	int i, j;

	i = 2; j = 3;
	noun(i, j, c, u);
	if (c[4] == 'C') u[8] = '1';
	u[15] = '1';
	u[17] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 7; j = 8;
	verb(i, j, c, u);
	u[33] = '1';
	if (c[7] == 'N' || c[7] == c[3]) u[35] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 10; j = 11;
	noun(i, j, c, u);
	if (c[12] == 'C') u[9] = '1';
	if (c[7] == 'N' || c[7] == c[11]) u[36] = '1';
	u[22] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[12] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 15; j = 16;
	verb(i, j, c, u);
	u[39] = '1';
	u[24] = '1';
	u[26] = '1';
	if (c[15] == 'N' || c[15] == c[3]) u[40] = '1';
	if (c[15] == 'N' || c[15] == c[11]) u[41] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[1] = '1';
	i = 18; j = 19;
	noun(i, j, c, u);
	if (c[20] == 'C') u[9] = '1';
	u[31] = '1';
	if (c[7] == 'N' || c[7] == c[19]) u[37] = '1';
	if (c[15] == 'N' || c[15] == c[19]) u[42] = '1';
	PrintLineAndResetInput(prob, u);
}

static void nnvvn(unsigned int prob, char *c, char *u) {
	int i, j;

	i = 2; j = 3;
	noun(i, j, c, u);
	u[15] = '1';
	u[17] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 6; j = 7;
	noun(i, j, c, u);
	if (c[8] == 'C') u[8] = '1';
	u[19] = '1';
	u[24] = '1';
	u[26] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 11; j = 12;
	verb(i, j, c, u);
	u[34] = '1';
	if (c[11] == 'N' || c[11] == c[3]) u[35] = '1';
	if (c[11] == 'N' || c[11] == c[7]) u[36] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 15; j = 16;
	verb(i, j, c, u);
	u[38] = '1';
	if (c[15] == 'N' || c[15] == c[3]) u[40] = '1';
	if (c[15] == 'N' || c[15] == c[7]) u[41] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[1] = '1';
	i = 18; j = 19;
	noun(i, j, c, u);
	if (c[20] == 'C') u[9] = '1';
	u[28] = '1';
	if (c[11] == 'N' || c[11] == c[19]) u[37] = '1';
	if (c[15] == 'N' || c[15] == c[19]) u[42] = '1';
	PrintLineAndResetInput(prob, u);
}

static void nvnnv(unsigned int prob, char *c, char *u) {
	int i, j;

	i = 2; j = 3;
	noun(i, j, c, u);
	if (c[4] == 'C') u[8] = '1';
	u[15] = '1';
	u[17] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 7; j = 8;
	verb(i, j, c, u);
	u[33] = '1';
	if (c[7] == 'N' || c[7] == c[3]) u[35] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 10; j = 11;
	noun(i, j, c, u);
	if (c[12] == 'C') u[9] = '1';
	if (c[7] == 'N' || c[7] == c[11]) u[36] = '1';
	u[22] = '1';
	PrintLineAndResetInput(prob, u);
	
	i = 14; j = 15;
	noun(i, j, c, u);
	if (c[16] == 'C') u[8] = '1';
	u[25] = '1';
	u[30] = '1';
	u[32] = '1';
	if (c[7] == 'N' || c[7] == c[15]) u[37] = '1';
	PrintLineAndResetInput(prob, u);
	
	u[1] = '1';
	i = 19; j = 20;
	verb(i, j, c, u);
	u[39] = '1';
	if (c[19] == 'N' || c[19] == c[3]) u[40] = '1';
	if (c[19] == 'N' || c[19] == c[11]) u[41] = '1';
	if (c[19] == 'N' || c[19] == c[15]) u[42] = '1';
	PrintLineAndResetInput(prob, u);
}

static int MakeOutput(unsigned int prob, char *type, char *c) {
	int i;
	char u[46];

	for (i=0; i < 45; i++)
		u[i] = '0';
	u[i] = EOS;
	u[13] = u[43] = '1';

	if (!strcmp(type, "NV")) nv(prob, c, u);
	else if (!strcmp(type, "VN")) vn(prob, c, u);
	else if (!strcmp(type, "NVN")) nvn(prob, c, u);
	else if (!strcmp(type, "TVNVN")) tvnvn(prob, c, u);
	else if (!strcmp(type, "NVTVN")) nvtvn(prob, c, u);
	else if (!strcmp(type, "NNVVN")) nnvvn(prob, c, u);
	else if (!strcmp(type, "NVNNV")) nvnnv(prob, c, u);
	else {
		fprintf(stderr, "Unknowkn type=%s.\n", type);
		return(0);
	}
	return(1);
}

void PrintThem(void) {
	int pos, len;
	int ci;
	char c[256], type[128];
	unsigned int prob;
	double GrandTotal;
	struct sents *tsent;
	struct comps *tcomp;

	GrandTotal = 0.0;
	for (tsent=sent; tsent != NULL; tsent=tsent->nextSent) {
		GrandTotal += (tsent->prob / cutoff);
		prob = (unsigned int)(tsent->prob/cutoff);
		strcpy(type, tsent->cell->name);
		if (debug > 0) {
			fprintf(fpout, "#%5u; %2.8f: %s", prob, tsent->prob, tsent->cell->name);
			for (tcomp=tsent->comp; tcomp != NULL; tcomp=tcomp->nextComp) {
				fprintf(fpout, "-%s", tcomp->att->name);
			}
			fprintf(fpout, ": ");
		}


		ci = 1;
		len = strlen(tsent->cell->name);
		for (pos=0; pos < len; pos++) {
			if (pos != 0 && debug > 0)
				fprintf(fpout, "-");
			if (debug > 0) 
				fprintf(fpout, "(%c", tsent->cell->name[pos]);
			c[ci++] = tsent->cell->name[pos];
			for (tcomp=tsent->comp; tcomp != NULL; tcomp=tcomp->nextComp) {
				if (pos < strlen(tcomp->att->name)) {
					if (debug > 0)
						fprintf(fpout, "-%c", tcomp->att->name[pos]);
					c[ci++] = tcomp->att->name[pos];
				} else if (debug > 0)
					fprintf(fpout, "-@");
			}
			if (debug > 0)
				fprintf(fpout, ")");
			if (ci >= 256) {
				fprintf(stderr, "c size is larger than 256\n");
				return;
			}
		}
		if (debug > 0)
			fprintf(fpout, "\n");
		c[ci] = EOS;
		if (debug > 1)
			fprintf(fpout, "#%5u; %5s: %s\n", prob, type, c+1);
		if (!MakeOutput(prob, type, c))
			return;
	}
	if (debug > 0)
		fprintf(fpout, "\nTotal number of sentences is: %.0f\n", GrandTotal);
}

void call() {
	ReadCells();
	isInputValid();
	if (debug > 2)
		PrintDoubleCheck();
	Permute();
	PrintThem();
}
