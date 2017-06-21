#include "cu.h"
#include "ced.h"
#include "MMedia.h"
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


#define DATECOL 1
#define AGECOL  2
#define SPKRCOL 3

struct t_tiers {
	char *text;
	struct t_tiers *nextTier;
} ;

struct t_lines {
	float  lineTime;
	struct t_tiers *tier;
	struct t_lines *nextLine; 
} ;

static struct t_tiers *t_RootTiers;
static struct t_lines *t_Rootlines;
static char corpus[256];
static char dateFormat;

void init(char f) {
	if (f) {
		t_RootTiers = NULL;
		t_Rootlines = NULL;
		stout = TRUE;
		onlydata = 1;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		corpus[0] = EOS;
		dateFormat = 0;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (*corpus == EOS) {
			fprintf(stderr,"Please specify corpus name with \"+c\" option.\n");
			cutt_exit(0);
		}
		if (dateFormat == 0) {
			fprintf(stderr,"Please specify date format with \"+de\" or \"+da\" option.\n");
			cutt_exit(0);
		}
		
	}
}

void usage() {
	printf("Usage: temp [+cS dC %s] filename(s)\n",mainflgs());
	puts("Example: insert +c\"corpus\" +de");
	puts("+cS: Specify the name of corpus");
	puts("+dC: Specify 'e' for european date format and 'a' for american");
	mainusage();
}

static struct t_tiers *freeTiers(struct t_tiers *p) {
	struct t_tiers *t;

	while (p != NULL) {
		t = p;
		p = p->nextTier;
		if (t->text != NULL)
			free(t->text);
		free(t);
	}
	return(NULL);
}

static struct t_lines *freeLines(struct t_lines *p) {
	struct t_lines *t;

	while (p != NULL) {
		t = p;
		p = p->nextLine;
		t->tier = freeTiers(t->tier);
		free(t);
	}
	return(NULL);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
	t_RootTiers = freeTiers(t_RootTiers);
	t_Rootlines = freeLines(t_Rootlines);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'c':
			strcpy(corpus, getfarg(f,f1,i));
			break;
		case 'd':
			if (*f == 'e' || *f == 'E')
				dateFormat = 1;
			else if (*f == 'a' || *f == 'A')
				dateFormat = 2;
			else {
				fprintf(stderr,"Please use either \"+de\" or \"+da\".\n");
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char *t_mkstring(char *s) {
	char *p;

	if ((p=(char *)malloc(strlen(s)+1)) == NULL) {
		fprintf(stderr,"ERROR: no more memory available.\n");
		t_RootTiers = freeTiers(t_RootTiers);
		t_Rootlines = freeLines(t_Rootlines);
		cutt_exit(0);
	}
	strcpy(p, s);
	return(p);
}

static struct t_tiers *addNewTier(struct t_tiers *tier, char *text) {
	struct t_tiers *t;

	if (tier == NULL) {
		tier = NEW(struct t_tiers);
		t = tier;
	} else {
		t = tier;
		while (t->nextTier != NULL)
			t = t->nextTier;
		t->nextTier = NEW(struct t_tiers);
		t = t->nextTier;
	}
	if (t == NULL) {
		fprintf(stderr,"temp: no more memory available.\n");
		t_RootTiers = freeTiers(t_RootTiers);
		t_Rootlines = freeLines(t_Rootlines);
		cutt_exit(0);
	}

	t->text = NULL;
	t->nextTier = NULL;
	t->text = t_mkstring(text);
	return(tier);
}

static struct t_lines *addNewLine(struct t_lines *lines, struct t_lines *curLine) {
	struct t_lines *nt,*tnt;

	if (lines == NULL) {
		lines = curLine;
	} else {
		nt = lines;
		if (curLine->lineTime == 0.0) {
			while (nt->nextLine != NULL)
				nt = nt->nextLine;
			nt->nextLine = curLine;
		} else {
			tnt = lines;
			while (1) {
				if (nt == NULL) break;
				else if (curLine->lineTime < nt->lineTime)
					break;
				tnt = nt;
				nt = nt->nextLine;
			}
			if (nt == NULL) {
				tnt->nextLine = curLine;
			} else if (nt == lines) {
				curLine->nextLine = nt;
				lines = curLine;
			} else {
				curLine->nextLine = tnt->nextLine;
				tnt->nextLine = curLine;
			}
		}
	}

	return(lines);
}

static char *changeDate(char *date) {
	int i, j, n;
	extern char *MonthNames[];

	strcpy(templineC2, date);
	j = 0;
	for (i=0; isSpace(templineC2[i]); i++) ;
	if (dateFormat == 1) { // euro
		for (; isdigit(templineC2[i]); i++)
			date[j++] = templineC2[i];
		date[j] = EOS;
		for (; !isalnum(templineC2[i]) && templineC2[i] != EOS; i++) ;
		if (templineC2[i] == EOS)
			return(date);
		n = atoi(templineC2+i);
		if (n > 0 && n <= 12) {
			strcat(date, "-");
			strcat(date, MonthNames[n-1]);
		} else {
			fputs("----------------------------------------\n",stderr);
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Illegal month specification: %d. (Must be 1-12)\n", n);
			t_RootTiers = freeTiers(t_RootTiers);
			t_Rootlines = freeLines(t_Rootlines);
			cutt_exit(0);
		}
		for (; isalnum(templineC2[i]); i++) ;
		for (; !isalnum(templineC2[i]) && templineC2[i] != EOS; i++) ;
		if (templineC2[i] == EOS)
			return(date);
		strcat(date, "-");
		j = strlen(date);
		for (; isdigit(templineC2[i]); i++)
			date[j++] = templineC2[i];
		date[j] = EOS;
	} else if (dateFormat == 2) { // US
		n = atoi(templineC2+i);
		for (; isalnum(templineC2[i]); i++) ;
		for (; !isalnum(templineC2[i]) && templineC2[i] != EOS; i++) ;
		if (templineC2[i] == EOS)
			return(date);
		for (; isdigit(templineC2[i]); i++)
			date[j++] = templineC2[i];
		date[j] = EOS;
		if (n > 0 && n <= 12) {
			strcat(date, "-");
			strcat(date, MonthNames[n-1]);
		} else {
			fputs("----------------------------------------\n",stderr);
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Illegal month specification: %d. (Must be 1-12)\n", n);
			t_RootTiers = freeTiers(t_RootTiers);
			t_Rootlines = freeLines(t_Rootlines);
			cutt_exit(0);
		}
		for (; !isalnum(templineC2[i]) && templineC2[i] != EOS; i++) ;
		if (templineC2[i] == EOS)
			return(date);
		strcat(date, "-");
		j = strlen(date);
		for (; isdigit(templineC2[i]); i++)
			date[j++] = templineC2[i];
		date[j] = EOS;
	}
	return(date);
}

static void OutputFile(char *date, char *age, struct t_lines *org) {
	int col, i;
	char *c;
	struct t_tiers *tSp;
	struct t_tiers *t;
	struct t_lines *p;

	if (org != NULL) {
		if (fpout != NULL && fpout != stdout) {
			fclose(fpout);
			unlink(newfname);
		}
		i = 0;
		templineC2[0] = EOS;
repeatName:
		strcpy(templineC1, oldfname);
		c = strchr(templineC1, '.');
		if (c != NULL)
			*c = EOS;
		strcat(templineC1, "=");
		strcat(templineC1, date);
		strcat(templineC1, templineC2);
		strcat(templineC1, ".cex");
		if (!access(templineC1,0)) {
			templineC2[0] = '-';
			templineC2[1] = (char)(i + '0');
			templineC2[2] = (char)EOS;
			i++;
			goto repeatName;
		}
		fpout = fopen(templineC1, "w");
		if (fpout == NULL) {
			fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",templineC1);
			t_RootTiers = freeTiers(t_RootTiers);
			t_Rootlines = freeLines(t_Rootlines);
			cutt_exit(0);
		}
		
		fprintf(fpout, "@Begin\n");
		fprintf(fpout, "@Languages:	en\n");
		fprintf(fpout, "@Participants:	CHI Target_Child\n");
		fprintf(fpout, "@ID:	%s|%s|CHI|%s||||Target_Child||\n", "en", corpus, age);
		fprintf(fpout, "@Age of CHI:	%s\n", age);
		fprintf(fpout, "@Date:	%s\n", changeDate(date));
	}
	for (p=org; p != NULL; p=p->nextLine) {
		col = 0;
		tSp = t_RootTiers;
		for (t=p->tier; t != NULL; t=t->nextTier) {
			col++;
			if (tSp == NULL)
				printout("%ERROR:", t->text, NULL, NULL, TRUE);
			else {
				if (col == 1)
					strcpy(spC, "*");
				else
					strcpy(spC, "%");
				strcat(spC, tSp->text);
				strcat(spC, ":");
				if (!strcmp(spC, "*Orth:"))
					strcpy(spC, "*CHI:");
				else if (!strcmp(spC, "%PhonAdult:"))
					strcpy(spC, "%mod:");
				else if (!strcmp(spC, "%CvAdult:"))
					strcpy(spC, "%xcva:");
				else if (!strcmp(spC, "%Morph:"))
					strcpy(spC, "%xpos:");
				else if (!strcmp(spC, "%Trans:"))
					strcpy(spC, "%eng:");
				else if (!strcmp(spC, "%ChildSeqNr:"))
					strcpy(spC, "%xseq:");
				else if (!strcmp(spC, "%PhonChild:"))
					strcpy(spC, "%pho:");
				else if (!strcmp(spC, "%CvChild:"))
					strcpy(spC, "%xcvc:");
				else if (!strcmp(spC, "%ImiSpon:"))
					strcpy(spC, "%ximi:");
				else if (!strcmp(spC, "%phenomena:"))
					strcpy(spC, "%xphe:");
				else if (!strcmp(spC, "%ProsStrucCh:"))
					strcpy(spC, "%xkey:");
				printout(spC, t->text, NULL, NULL, TRUE);
				tSp = tSp->nextTier;
			}
		}
	}
	if (org != NULL) {
		fprintf(fpout, "@End\n");
		fprintf(stderr,"Done with file <%s>\n",templineC1);
	}
}

static char *getLastText(struct t_lines *lastLine, int colNum, long i, char t) {
	int col;
	struct t_tiers *tier;

	if (lastLine == NULL || lastLine->tier == NULL) {
		templineC[i] = t;
		fputs("----------------------------------------\n",stderr);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr,"No value to duplicate for line (1):\n");
		fprintf(stderr,"%s\n", templineC);
		t_RootTiers = freeTiers(t_RootTiers);
		t_Rootlines = freeLines(t_Rootlines);
		cutt_exit(0);
	}
	col = SPKRCOL - 1;
	for (tier=lastLine->tier; tier != NULL; tier=tier->nextTier) {
		col++;
		if (col == colNum)
			break;
	}
	if (tier == NULL || tier->text == NULL) {
		templineC[i] = t;
		fputs("----------------------------------------\n",stderr);
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr,"No value to duplicate for line (2):\n");
		fprintf(stderr,"%s\n", templineC);
		t_RootTiers = freeTiers(t_RootTiers);
		t_Rootlines = freeLines(t_Rootlines);
		cutt_exit(0);
	}
	return(tier->text);
}

void call() {
	int col;
	char t, isSpBlank;
	char date[64], age[64];
	long i, j, beg;
	struct t_lines *curLine, *lastLine;

	lineno = 0L;
	if (fgets_cr(templineC, UTTLINELEN, fpin) == NULL) {
		return;
	}
	lineno++;
	col = 0;
	for (i=0L; templineC[i] != EOS && templineC[i] != '\n'; i++) {
		col++;
		beg = i;
		for (; templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS; i++) ;
		if (i > beg) {
			t = templineC[i];
			templineC[i] = EOS;
			if (col == DATECOL || col == AGECOL)
				;
			else
				t_RootTiers = addNewTier(t_RootTiers, templineC+beg);
			templineC[i] = t;
		}
	}

	date[0] = EOS;
	age[0] = EOS;
	curLine = NULL;
	while (fgets_cr(templineC, UTTLINELEN, fpin) != NULL) {
		lastLine = curLine;		

		curLine = NEW(struct t_lines);
		if (curLine == NULL) {
			fprintf(stderr,"temp: no more memory available.\n");
			t_RootTiers = freeTiers(t_RootTiers);
			t_Rootlines = freeLines(t_Rootlines);
			cutt_exit(0);
		}
		curLine->lineTime = 0.0;
		curLine->tier = NULL;
		curLine->nextLine = NULL;

		lineno++;
		col = 0;
		for (i=0L; templineC[i] != EOS && templineC[i] != '\n'; i++) {
			col++;
			beg = i;
			for (; templineC[i] != '\t' && templineC[i] != '\n' && templineC[i] != EOS; i++) ;
			if (i >= beg) {
				t = templineC[i];
				templineC[i] = EOS;
				if (col == DATECOL) {
					if (templineC[beg] != EOS) {
						OutputFile(date, age, t_Rootlines);
						t_Rootlines = freeLines(t_Rootlines);
						age[0] = EOS;
						lastLine = NULL;
						strcpy(date, templineC+beg);
					}
				} else if (col == AGECOL) {
					if (templineC[beg] != EOS)
						strcpy(age, templineC+beg);
				} else if (col == SPKRCOL) {
					if (templineC[beg] == EOS) {
						isSpBlank = TRUE;
						curLine->tier = addNewTier(curLine->tier, getLastText(lastLine, col, i, t));
					} else {
						isSpBlank = FALSE;
						curLine->tier = addNewTier(curLine->tier, templineC+beg);
					}
				} else {
					if (templineC[beg] == EOS) {
						if (col < 8 && isSpBlank)
							curLine->tier = addNewTier(curLine->tier, getLastText(lastLine, col, i, t));
						else
							curLine->tier = addNewTier(curLine->tier, "0");
					} else {
						curLine->tier = addNewTier(curLine->tier, templineC+beg);
					}
					if (col == 13) {
						for (j=beg; templineC[j] != EOS; j++) {
							if (templineC[j] == ',')
								templineC[j] = '.';
						}
						curLine->lineTime = atof(templineC+beg);
					}
				}
				templineC[i] = t;
			}
		}
		t_Rootlines = addNewLine(t_Rootlines, curLine);
	}
	if (t_Rootlines != NULL) {
		OutputFile(date, age, t_Rootlines);
		t_Rootlines = freeLines(t_Rootlines);
	}
}
