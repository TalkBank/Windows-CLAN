/**********************************************************************
 "Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

#define CHAT_MODE 0
#include "cu.h"

#if !defined(UNX)
#define _main spreadsheet_main
#define call spreadsheet_call
#define getflag spreadsheet_getflag
#define init spreadsheet_init
#define usage spreadsheet_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern int  getc_cr_lc;
extern char OverWriteFile;
extern char outputOnlyData;
extern struct tier *defheadtier;

struct table_cols {
	char *str;
	struct table_cols *nextCol;
};

struct comb_cols {
	char *title;
	struct table_cols *cols;
	struct comb_cols *nextCombCol;
};

struct table_rows {
	struct comb_cols *cCols;
	struct table_rows *nextRow;
};

struct from_items {
	char *fromS;
	struct from_items *next;
} ;

struct search_items {
	struct from_items *from;
	char *toS;
	struct search_items *next;
} ;


static char isTranspose;
static struct table_rows *root_rows;
static struct search_items *root_items;

void usage() {
	printf("Rotate table by turning columns into row and vice versa or if\n");
	printf("+sF option used to merge rows with matching cells in F file into one row\n");
	printf("Usage: spreadsheet [sF %s] filename(s)\n",mainflgs());
	puts("+sF: merge rows with matching cells in F file into one row");
	mainusage(TRUE);
}

void init(char f) {
	if (f) {
		isTranspose = TRUE;
		stout = FALSE;
		outputOnlyData = TRUE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		AddCEXExtension = ".xls";
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		root_rows = NULL;
		root_items = NULL;
	}
}

static void freeTableCols(struct table_cols *p) {
	struct table_cols *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextCol;
		if (t->str)
			free(t->str);
		free(t);
	}
}

static void freeCombCols(struct comb_cols *p) {
	struct comb_cols *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextCombCol;
		if (t->title != NULL)
			free(t->title);
		freeTableCols(t->cols);
		free(t);
	}
}

static struct table_rows *freeTableRows(struct table_rows *p) {
	struct table_rows *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextRow;
		freeCombCols(t->cCols);
		free(t);
	}
	return(NULL);
}

static void freeSearchFrom(struct from_items *p) {
	struct from_items *t;
	
	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->fromS)
			free(t->fromS);
		free(t);
	}
}

static struct search_items *freeSearchItems(struct search_items *p) {
	struct search_items *t;
	
	while (p != NULL) {
		t = p;
		p = p->next;
		freeSearchFrom(t->from);
		if (t->toS)
			free(t->toS);
		free(t);
	}
	return(NULL);
}

static void memOverflow(const char *err) {
	root_rows = freeTableRows(root_rows);
	root_items = freeSearchItems(root_items);
	fprintf(stderr,"%s\n", err);
	cutt_exit(0);
}

static void addColToCombCol(struct comb_cols *cCols, char *str) {
	struct table_cols *p;
	
	if (cCols->cols == NULL) {
		cCols->cols = NEW(struct table_cols);
		p = cCols->cols;
	} else {
		for (p=cCols->cols; p->nextCol != NULL; p=p->nextCol) ;
		p->nextCol = NEW(struct table_cols);
		p = p->nextCol;
	}
	if (p == NULL) {
		memOverflow("Error: out of memory");
	}
	p->nextCol = NULL;
	if (str == NULL)
		p->str = NULL;
	else {
		if ((p->str=(char *)malloc(strlen(str)+1)) == NULL) {
			memOverflow("Error: out of memory");
		}
		strcpy(p->str, str);
	}
}

static void addDataToCol(struct table_rows *row, int pos, const char *title, char *str) {
	int cnt;
	struct comb_cols *cCol;
	struct table_cols *col;

	if (title == NULL) {
		cCol = row->cCols;
		if (cCol != NULL)
			cCol = cCol->nextCombCol;
		if (cCol == NULL) {
			memOverflow("Internal error 2!");
		}
		for (; cCol != NULL; cCol=cCol->nextCombCol) {
			cnt = 0;
			for (col=cCol->cols; cnt < pos && col != NULL; col=col->nextCol) {
				cnt++;
			}
			if (col == NULL) {
				memOverflow("Internal error 3!");
			}
			if (col->str == NULL) {
				if ((col->str=(char *)malloc(strlen(str)+1)) == NULL) {
					memOverflow("Error: out of memory");
				}
				strcpy(col->str, str);
			}
		}
	} else {
		for (cCol=row->cCols; cCol != NULL; cCol=cCol->nextCombCol) {
			if (strcmp(cCol->title, title) == 0) {
				cnt = 0;
				for (col=cCol->cols; cnt < pos && col != NULL; col=col->nextCol) {
					cnt++;
				}
				if (col == NULL) {
					memOverflow("Internal error 4!");
				}
				if (col->str == NULL) {
					if ((col->str=(char *)malloc(strlen(str)+1)) == NULL) {
						memOverflow("Error: out of memory");
					}
					strcpy(col->str, str);
				}
				return;
			}
		}
		if (cCol == NULL) {
			memOverflow("Internal error 5!");
		}
	}
}

static struct comb_cols *addCombColToRow(struct table_rows *cRow, const char *title) {
	struct comb_cols *p;

	if (cRow->cCols == NULL) {
		cRow->cCols = NEW(struct comb_cols);
		p = cRow->cCols;
	} else {
		for (p=cRow->cCols; p->nextCombCol != NULL; p=p->nextCombCol) {
			if (strcmp(p->title, title) == 0) {
				return(p);
			}
		}
		if (strcmp(p->title, title) == 0) {
			return(p);
		}
		p->nextCombCol = NEW(struct comb_cols);
		p = p->nextCombCol;
	}
	if (p == NULL) {
		memOverflow("Error: out of memory");
	}
	p->nextCombCol = NULL;
	p->cols = NULL;
	if ((p->title=(char *)malloc(strlen(title)+1)) == NULL) {
		memOverflow("Error: out of memory");
	}
	strcpy(p->title, title);
	return(p);
}

static struct table_rows *addRowToTable(struct table_rows *root) {
	struct table_rows *p;

	if (root == NULL) {
		root = NEW(struct table_rows);
		p = root;
	} else {
		for (p=root; p->nextRow != NULL; p=p->nextRow) ;
		p->nextRow = NEW(struct table_rows);
		p = p->nextRow;
	}
	if (p == NULL) {
		memOverflow("Error: out of memory");
	}
	p->cCols = NULL;
	p->nextRow = NULL;
	return(p);
}

static struct search_items *addSearchItem(struct search_items *cur, char *fromS, char *toS) {
	int l1, l2;
	struct from_items *nt, *tnt;

	if (fromS == NULL && toS == NULL) {
		cur = NEW(struct search_items);
		if (cur == NULL) {
			memOverflow("Error: out of memory");
		}			
		cur->next = root_items;
		root_items = cur;
		cur->from = NULL;
		cur->toS = NULL;
	} else if (fromS != NULL && cur != NULL) {
		l1 = strlen(fromS);
		if (cur->from == NULL) {
			cur->from = NEW(struct from_items);
			nt = cur->from;
			if (nt == NULL) {
				memOverflow("Error: out of memory");
			}
			nt->next = NULL;
		} else {
			tnt= cur->from;
			nt = cur->from;
			while (1) {
				if (nt == NULL)
					break;
				else {
					l2 = strlen(nt->fromS);
					if (l1 > l2) {
						break;
					}
				}
				tnt = nt;
				nt = nt->next;
			}
			if (nt == NULL) {
				tnt->next = NEW(struct from_items);
				nt = tnt->next;
				if (nt == NULL) {
					memOverflow("Error: out of memory");
				}
				nt->next = NULL;
			} else if (nt == cur->from) {
				cur->from = NEW(struct from_items);
				if (cur->from == NULL) {
					memOverflow("Error: out of memory");
				}
				cur->from->next = nt;
				nt = cur->from;
			} else {
				nt = NEW(struct from_items);
				if (nt == NULL) {
					memOverflow("Error: out of memory");
				}
				nt->next = tnt->next;
				tnt->next = nt;
			}
		}
		if ((nt->fromS=(char *)malloc(strlen(fromS)+1)) == NULL) {
			memOverflow("Error: out of memory");
		}
		strcpy(nt->fromS, fromS);
	} else if (toS != NULL && cur != NULL) {
		if ((cur->toS=(char *)malloc(strlen(toS)+1)) == NULL) {
			memOverflow("Error: out of memory");
		}
		strcpy(cur->toS, toS);
	}
	return(cur);
}

static void readRows(char *fname) {
	int  i, j;
	char isQF;
	FILE *fp;
	struct search_items *cur;
	
	if ((fp=fopen(fname, "r")) == NULL) {
		sprintf(templineC, "Can't open search list file: %s", fname);
		memOverflow(templineC);
	}
	cur = NULL;
	while (fgets_cr(templineC, UTTLINELEN, fp) != NULL) {
		if (templineC[0] == '%' || templineC[0] == '#')
			continue;
		if (uS.isUTF8(templineC) || uS.partcmp(templineC, FONTHEADER, FALSE, FALSE))
			continue;
		if (!strcmp(templineC,"\n"))
			continue;
		isQF = 0;
		j = 0;
		for (i=0; templineC[i] != EOS; i++) {
			if (isQF == templineC[i])
				isQF = 0;
			else if (templineC[i] == '"')
				isQF = '"';
			else if (templineC[i] == '\'')
				isQF = '\'';
			if (!isQF && (isSpace(templineC[i]) || templineC[i] == '\n')) {
				templineC1[j] = EOS;
				if (templineC1[0] != EOS) {
					if (root_items == NULL || cur == NULL)
						cur = addSearchItem(cur, NULL, NULL);
					if (templineC1[0] != '=')
						cur = addSearchItem(cur, templineC1, NULL);
					else {
						cur = addSearchItem(cur, NULL, templineC1+1);
						cur = addSearchItem(cur, NULL, NULL);
					}
				}
				j = 0;
			} else
				templineC1[j++] = templineC[i];
		}	
		templineC1[j] = EOS;
		if (templineC1[0] != EOS) {
			if (root_items == NULL || cur == NULL)
				cur = addSearchItem(cur, NULL, NULL);
			if (templineC1[0] != '=')
				cur = addSearchItem(cur, templineC1, NULL);
			else {
				cur = addSearchItem(cur, NULL, templineC1+1);
				cur = addSearchItem(cur, NULL, NULL);
			}
		}
	}
	fclose(fp);
	if (root_items != NULL && root_items->from == NULL && root_items->toS == NULL) {
		cur = root_items;
		root_items = root_items->next;
		free(cur);
	}
}

static void extractStarMatch(char *s, char *pat, char *new_s) {
    register int j, k;
    register int n, m;
    int t, end;

	*new_s = EOS;
    if (s[0] == EOS) return;
    for (j = 0, k = 0; pat[k]; j++, k++) {
		if (pat[k] == '*') {	  /* wildcard */
			k++; t = j;
f1:
			while (s[j] && (islower((unsigned char)s[j]) ? (char)toupper((unsigned char)s[j]) : s[j]) != 
				   (islower((unsigned char)pat[k]) ? (char)toupper((unsigned char)pat[k]) : pat[k])) {
				*new_s++ = s[j++];
			}
			end = j;
			if (s[j]) {
	    		for (m=j+1, n=k+1; s[m] && pat[n]; m++, n++) {
					if (pat[n] == '*') break;
					else if ((islower((unsigned char)s[m]) ? (char)toupper((unsigned char)s[m]) : s[m]) != 
							 (islower((unsigned char)pat[n]) ? (char)toupper((unsigned char)pat[n]) : pat[n])) {
						*new_s++ = s[j++];
						goto f1;
					}
				}
				if (s[m] && !pat[n]) {
					*new_s++ = s[j++];
					goto f1;
				}
			}
			if (s[j] == EOS || pat[k] == EOS) break;
		}
	}
	*new_s = EOS;
}

static struct table_rows *findRowInTable(char *fromS) {
	struct table_rows *cRow;

	for (cRow=root_rows; cRow != NULL; cRow=cRow->nextRow) {
		if (cRow->cCols != NULL && cRow->cCols->cols != NULL && cRow->cCols->cols->str != NULL) {
			if (uS.mStricmp(cRow->cCols->cols->str, fromS) == 0) {
				return(cRow);
			}
		}
	}
	return(NULL);
}


static struct table_rows *findConvertedRow(char *fromS, char *mod, struct search_items *curS) {
	struct from_items *from;

	while (curS != NULL) {
		for (from=curS->from; from != NULL; from=from->next) {
			if (uS.patmat(fromS, from->fromS)) {
				extractStarMatch(fromS, from->fromS, mod);
				strcpy(fromS, curS->toS);
				return(findRowInTable(fromS));
			}
		}
		curS = curS->next;
	}
	return(NULL);
}

static void outputTable(struct table_rows *p) {
	char isFirstCol, isFirstRow;
	struct comb_cols *cCol;
	struct table_cols *col;

	isFirstRow = TRUE;
	while (p != NULL) {
		isFirstCol = TRUE;
		for (cCol=p->cCols; cCol != NULL; cCol=cCol->nextCombCol) {
			for (col=cCol->cols; col != NULL; col=col->nextCol) {
				if (!isFirstCol)
					fprintf(fpout, "\t");
				else
					isFirstCol = FALSE;
				if (col->str != NULL) {
					fprintf(fpout, "%s", col->str);
					if (isFirstRow && cCol->title[0] != EOS && strcmp(cCol->title, "*DO_NOT_ADD_TO_TITLE*") != 0)
						fprintf(fpout, "-%s", cCol->title);
				}
			}
		}
		fprintf(fpout, "\n");
		isFirstRow = FALSE;
		p = p->nextRow;
	}
}

static void doTranspose(void) {
	int  i;
	char chrs;
	struct comb_cols *cCol;
	struct table_rows *cRow;

	i = 0;
	cRow = NULL;
	getc_cr_lc = '\0';
	while (!feof(fpin)) {
		if ((chrs=(char)getc_cr(fpin, NULL)) == EOF)
			break;
		if (chrs == '\t' || chrs == '\n') {
			templineC[i] = EOS;
			if (root_rows == NULL) {
				root_rows = addRowToTable(root_rows);
				cRow = root_rows;
			} else if (cRow == NULL) {
				cRow = addRowToTable(root_rows);
			}
			cCol = addCombColToRow(cRow, "");
			addColToCombCol(cCol, templineC);
			if (chrs == '\n')
				cRow = root_rows;
			else
				cRow = cRow->nextRow;
			i = 0;
		} else
			templineC[i++] = chrs;
	}
}

static void populateTable(struct table_rows *cRow, int ids, int datas) {
	int cnt;
	struct table_rows *tRow;
	struct comb_cols *cCol, *tCol;
	struct table_cols *col;

	tRow = cRow;
	while (cRow != NULL) {
		tCol = tRow->cCols;
		cCol = cRow->cCols;
		while (tCol != NULL) {
			if (cCol == NULL) {
				cCol = addCombColToRow(cRow, tCol->title);
			}
			if (strcmp(cCol->title, "*DO_NOT_ADD_TO_TITLE*") == 0) {
				cnt = 0;
				for (col=cCol->cols; col != NULL; col=col->nextCol) {
					cnt++;
				}
				for (; cnt < ids; cnt++) {
					addColToCombCol(cCol, NULL);
				}
			} else {
				cnt = 0;
				for (col=cCol->cols; col != NULL; col=col->nextCol) {
					cnt++;
				}
				for (; cnt < datas; cnt++) {
					addColToCombCol(cCol, NULL);
				}
			}
			tCol = tCol->nextCombCol;
			cCol = cCol->nextCombCol;
		}
		cRow = cRow->nextRow;
	}
}

static void doCombine(void) {
	int  i, col, lCol, colOffset;
	char chrs, isTitleRow;
	struct comb_cols *cCol;
	struct table_rows *cRow, *titleRow;

	i = 0;
	col = 0;
	lCol = 0;
	isTitleRow = TRUE;
	colOffset = 1;
	titleRow = NULL;
	cRow = NULL;
	getc_cr_lc = '\0';
	while (!feof(fpin)) {
		if ((chrs=(char)getc_cr(fpin, NULL)) == EOF)
			break;
		if (chrs == '\t' || chrs == '\n') {
			templineC[i] = EOS;
			if (chrs == '\t' || chrs == '\n') {
				if (col == 0) {
					cRow = findConvertedRow(templineC, templineC1, root_items);
					if (root_rows == NULL) {
						root_rows = addRowToTable(root_rows);
						cRow = root_rows;
						cCol = addCombColToRow(cRow, "*DO_NOT_ADD_TO_TITLE*");
						addColToCombCol(cCol, templineC);
						if (isTitleRow)
							titleRow = cRow;
						else
							cCol = addCombColToRow(titleRow, templineC1);
					} else if (cRow == NULL) {
						cRow = addRowToTable(root_rows);
						cCol = addCombColToRow(cRow, "*DO_NOT_ADD_TO_TITLE*");
						addColToCombCol(cCol, templineC);
						if (isTitleRow)
							titleRow = cRow;
						else
							cCol = addCombColToRow(titleRow, templineC1);
					}
				} else if (isTitleRow) {
					if ((!uS.mStricmp(templineC, "Language")     && col == 1) || (!uS.mStrnicmp(templineC, "Speaker", 7) && col == 1) || 
						(!uS.mStricmp(templineC, "Corpus")       && col == 2) ||
						(!uS.mStricmp(templineC, "Code")         && col == 3) || (!uS.mStrnicmp(templineC, "Speaker", 7) && col == 3) ||
						(!uS.mStricmp(templineC, "Age")          && col == 4) ||
						(!uS.mStricmp(templineC, "Sex")          && col == 5) ||
						(!uS.mStricmp(templineC, "Group")        && col == 6) ||
						(!uS.mStricmp(templineC, "SES")          && col == 7) ||
						(!uS.mStricmp(templineC, "Role")         && col == 8) ||
						(!uS.mStricmp(templineC, "Education")    && col == 9) ||
						(!uS.mStricmp(templineC, "Custom_field") && col == 10)|| (!uS.mStrnicmp(templineC, "Unique File", 11) && col == 10))
						colOffset = col + 1;
					if (col < colOffset) {
						cCol = addCombColToRow(cRow, "*DO_NOT_ADD_TO_TITLE*");
						addColToCombCol(cCol, templineC);
					}
				} else
					cCol = addCombColToRow(titleRow, templineC1);
				col++;
				if (chrs == '\n') {
					cRow = cRow->nextRow;
					if (!isTitleRow && lCol != col) {
						memOverflow("Number of columns is not the same across all rows");
					}
					lCol = col;
					col = 0;
					isTitleRow = FALSE;
				}
			}
			i = 0;
		} else
			templineC[i++] = chrs;
	}
	if (root_rows == NULL) {
		memOverflow("\nERROR: No readable data found. Is input file in TEXT encoding format?");
	}
	populateTable(root_rows, colOffset, lCol-colOffset);
	rewind(fpin);
	i = 0;
	col = 0;
	cRow = NULL;
	isTitleRow = TRUE;
	getc_cr_lc = '\0';
	while (!feof(fpin)) {
		if ((chrs=(char)getc_cr(fpin, NULL)) == EOF)
			break;
		if (chrs == '\t' || chrs == '\n') {
			templineC[i] = EOS;
			if (chrs == '\t' || chrs == '\n') {
				if (isTitleRow) {
					if (col == 0) {
						cRow = root_rows;
					} else if (col >= colOffset) {
						addDataToCol(cRow, col-colOffset, NULL, templineC);
					}
				} else {
					if (col == 0) {
						cRow = findConvertedRow(templineC, templineC1, root_items);
						if (cRow == NULL) {
							cRow = findRowInTable(templineC);
							if (cRow == NULL) {
								memOverflow("Internal error 1!");
							}
						}
					} else if (col < colOffset) {
						addDataToCol(cRow, col, "*DO_NOT_ADD_TO_TITLE*", templineC);
					} else {
						addDataToCol(cRow, col-colOffset, templineC1, templineC);
					}
				}
				col++;
				if (chrs == '\n') {
					col = 0;
					isTitleRow = FALSE;
				}
			}
			i = 0;
		} else
			templineC[i++] = chrs;
	}
}

void call(void) {
	if (isTranspose) {
		doTranspose();
	} else {
		doCombine();
	}
	outputTable(root_rows);
	root_rows = freeTableRows(root_rows);
	getc_cr_lc = '\0';

}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SPREADSH;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
	root_items = freeSearchItems(root_items);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 's':
			if (*f == EOS || (*f == '@' && *(f+1) == EOS)) {
				memOverflow("Please specify search list file name.");
			} else if (*f == '@')
				f++;
			readRows(f);
			isTranspose = FALSE;
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

