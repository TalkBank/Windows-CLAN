/**********************************************************************
	"Copyright 1990-2023 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
 */

#include "ced.h"
#define CHAT_MODE 0
#include "cu.h"
#ifdef _WIN32
#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#define mkdir my_mkdir
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#if defined(UNX) || defined(_MAC_CODE)
#define MODE S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH
#endif
#ifdef _WIN32
#define MODE 0
#endif

extern struct tier *defheadtier;
extern char OverWriteFile;

static int  errCodesCnt;
static char isFTime, isShowErr;
static char *Codes_FName;
static struct errcodes {
	char *A;
	struct errcodes *next;
} *root_errcodes;

static struct codes {
	char *A;
	char *E;
	char *F;
	char *G;
	char *H;
	char *I;
	char *J;
	char *K;
	char *L;
	char *M;
	char *N;
	struct codes *next;
} *root_codes;

static void persuade_cleanup(void) {
	struct codes *t;
	struct errcodes *te;

	while (root_errcodes != NULL) {
		te = root_errcodes;
		root_errcodes = root_errcodes->next;
		if (te->A != NULL)
			free(te->A);
		free(te);
	}
	while (root_codes != NULL) {
		t = root_codes;
		root_codes = root_codes->next;
		if (t->A != NULL)
			free(t->A);
		if (t->E != NULL)
			free(t->E);
		if (t->F != NULL)
			free(t->F);
		if (t->G != NULL)
			free(t->G);
		if (t->H != NULL)
			free(t->H);
		if (t->I != NULL)
			free(t->I);
		if (t->J != NULL)
			free(t->J);
		if (t->K != NULL)
			free(t->K);
		if (t->L != NULL)
			free(t->L);
		if (t->M != NULL)
			free(t->M);
		if (t->N != NULL)
			free(t->N);
		free(t);
	}
}

static void persuade_overflow() {
	fprintf(stderr,"chstring: no more memory available.\n");
	persuade_cleanup();
	cutt_exit(0);
}

static char *save_string(char *word) {
	int len;
	char *st;

	len = strlen(word) - 1;
	if (word[0] == '"' && word[len] == '"') {
		word[len] = EOS;
		len = 1;
	} else
		len = 0;
	st = (char *)malloc(strlen(word+len)+1);
	if (st == NULL)
		persuade_overflow();
	strcpy(st, word+len);
	return(st);
}

static void readcodes(void) {
	FILE *fdic;
	char chrs, qtc, word[BUFSIZ+1];
	int index, col;
	long persuade_ln = 0, persuade_tln = 0;
	struct codes *nextone = NULL;
	FNType mFileName[FNSize];

	if (Codes_FName == NULL) {
		fprintf(stderr,"Please specify codes file with +cF option");
		persuade_cleanup();
		cutt_exit(0);
	}
	if ((fdic=OpenGenLib(Codes_FName,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the changes files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", Codes_FName, mFileName);
		cutt_exit(0);
	}
	fprintf(stderr,"Reading codes file: %s\n", mFileName);
	col = 1;
	index = 0;
	qtc = 0;
	while (!feof(fdic)) {
		chrs = (char)getc_cr(fdic, NULL);
		if (qtc == 0 && index == 0 && chrs == '"') {
			qtc = chrs;
			if (!feof(fdic))
				chrs = (char)getc_cr(fdic, NULL);
			else
				qtc = 0;
		} else if (qtc == chrs) {
			if (!feof(fdic)) {
				chrs = (char)getc_cr(fdic, NULL);
				if (chrs == ',') {
					qtc = 0;
				} else {
					if (index < BUFSIZ)
						word[index++] = qtc;
				}
			} else
				qtc = 0;
		}
		if (qtc != 0) {
			if (index < BUFSIZ)
				word[index++] = chrs;
		} else if (chrs == ',' && col == 1) {
			word[index] = EOS;
			index = 0;
			if (root_codes == NULL) {
				root_codes = NEW(struct codes);
				nextone = root_codes;
			} else {
				nextone = root_codes;
				while (nextone->next != NULL) nextone = nextone->next;
				nextone->next = NEW(struct codes);
				nextone = nextone->next;
			}
			if (nextone == NULL)
				persuade_overflow();
			nextone->next = NULL;
			nextone->E = NULL;
			nextone->F = NULL;
			nextone->G = NULL;
			nextone->H = NULL;
			nextone->I = NULL;
			nextone->J = NULL;
			nextone->K = NULL;
			nextone->L = NULL;
			nextone->M = NULL;
			nextone->N = NULL;
			nextone->A = save_string(word);
			col++;
		} else if (chrs == ',' || chrs == '\n' || feof(fdic)) {
			word[index] = EOS;
			index = 0;
			switch(col) {
				case 5: 
					nextone->E = save_string(word);
					break;
				case 6: 
					nextone->F = save_string(word);
					break;
				case 7: 
					nextone->G = save_string(word);
					break;
				case 8: 
					nextone->H = save_string(word);
					break;
				case 9: 
					nextone->I = save_string(word);
					break;
				case 10: 
					nextone->J = save_string(word);
					break;
				case 11: 
					nextone->K = save_string(word);
					break;
				case 12: 
					nextone->L = save_string(word);
					break;
				case 13: 
					nextone->M = save_string(word);
					break;
				case 14: 
					nextone->N = save_string(word);
					break;
			}
			if (chrs == '\n' || feof(fdic)) {
				if (persuade_ln > persuade_tln) {
					persuade_tln = persuade_ln + 200;
					fprintf(stderr,"\r%ld ",persuade_ln);
				}
				persuade_ln++;
				col = 1;
			} else {
				col++;
			}
		} else {
			if (index < BUFSIZ)
				word[index++] = chrs;
		}
	}
	fprintf(stderr,"\r%ld\n", persuade_ln);
	fclose(fdic);
}

void init(char f) {
	if (f) {
		isFTime = TRUE;
		onlydata = 1;
		stout = TRUE;
		OverWriteFile = TRUE;
		isShowErr = FALSE;
		Codes_FName = NULL;
		root_codes = NULL;
		root_errcodes = NULL;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
//		root_list = NULL;
	} else {
		if (isFTime) {
			readcodes();
			isFTime = FALSE;
			stout = TRUE;
		}
	}
}

void usage() {
	printf("breakup corpus .csv file\n");
	printf("Usage: temp cF filename(s)\n");
	printf("+cF: codes file F\n");
	printf("+e : show errors\n");
	mainusage(TRUE);
}

static struct codes *getCodes(char *A) {
	struct codes *code;
	
	
	for (code=root_codes; code != NULL; code = code->next) {
		if (uS.mStricmp(A, code->A) == 0)
			return(code);
	}
	return(NULL);
}

static char addErrCodes(char *A) {
	struct errcodes *code;

	if (root_errcodes == NULL) {
		root_errcodes = NEW(struct errcodes);
		code = root_errcodes;
	} else {
		code = root_errcodes;
		while (code->next != NULL) {
			if (uS.mStricmp(A, code->A) == 0)
				return(FALSE);
			code = code->next;
		}
		if (uS.mStricmp(A, code->A) == 0)
			return(FALSE);
		code->next = NEW(struct errcodes);
		code = code->next;
	}
	code->next = NULL;
	code->A = save_string(A);
	errCodesCnt++;
	return(TRUE);
}

static void cleanUpText(char *st) {
	int i;
	
	for (i=0; st[i] != EOS; i++) {
		if (st[i] == '"' && st[i+1] == '"')
			strcpy(st+i, st+i+2);
	}
}

void call() {
	int index, col, i, e, tMemSize;
	char chrs, qtc;
	char *C, *tc, *col_text, oldA[BUFSIZ], race[64], SES[32];
	long persuade_ln = 0, persuade_tln = 0;
	struct codes *code;
	FNType mFileName[FNSize];
	FILE *fp;

	tMemSize = UTTLINELEN;
	col_text = (char *)malloc(tMemSize+1);
	if (col_text == NULL)
		persuade_overflow();
	C = NULL;
	col = 1;
	index = 0;
	qtc = 0;
	code = NULL;
	oldA[0] = EOS;
	fp = NULL;
	while (!feof(fpin)) {
		chrs = (char)getc_cr(fpin, NULL);
		if (qtc == 0 && index == 0 && chrs == '"') {
			qtc = chrs;
			if (!feof(fpin))
				chrs = (char)getc_cr(fpin, NULL);
			else
				qtc = 0;
		} else if (qtc == chrs) {
			if (!feof(fpin)) {
				chrs = (char)getc_cr(fpin, NULL);
				if (chrs == ',') {
					qtc = 0;
				} else {
					if (index < tMemSize) {
						col_text[index++] = qtc;
					} else {
						tMemSize += UTTLINELEN;
						col_text[index] = EOS;
						tc = col_text;
						col_text = (char *)malloc(tMemSize+1);
						if (col_text == NULL)
							persuade_overflow();
						strcpy(col_text, tc);
						free(tc);
						index = strlen(col_text);
						col_text[index++] = qtc;
					}
				}
			} else
				qtc = 0;
		}
		if (qtc != 0) {
			if (index < tMemSize) {
				col_text[index++] = chrs;
			} else {
				tMemSize += UTTLINELEN;
				col_text[index] = EOS;
				tc = col_text;
				col_text = (char *)malloc(tMemSize+1);
				if (col_text == NULL)
					persuade_overflow();
				strcpy(col_text, tc);
				free(tc);
				index = strlen(col_text);
				col_text[index++] = chrs;
			}
		} else if (chrs == ',' && col == 1) {
			col_text[index] = EOS;
			index = 0;
			code = getCodes(col_text);
			if (code == NULL && isShowErr == TRUE) {
				addErrCodes(col_text);
			}
			col++;
		} else if (chrs == ',' || chrs == '\n' || feof(fpin)) {
			col_text[index] = EOS;
			index = 0;
			switch(col) {
				case 3:
					cleanUpText(col_text);
					C = save_string(col_text);
					break;
			}
			if (chrs == '\n' || feof(fpin)) {
				if (persuade_ln > persuade_tln) {
					persuade_tln = persuade_ln + 1000;
					fprintf(stderr,"\r%ld ",persuade_ln);
				}
				if (code != NULL && uS.mStricmp(code->A, "essay_id_comp") != 0) {
					strcpy(mFileName, wd_dir);
					addFilename2Path(mFileName, "RESULTS");
					if (my_access(mFileName, 0) != 0) {
						if (mkdir(mFileName, MODE)) {
							persuade_cleanup();
							fprintf(stderr, "\n   Error creating folder \"%s\".\n", mFileName);
							fprintf(stderr, "   Folder already exists, Please delete it first.\n");
							cutt_exit(0);
						}
					}
					addFilename2Path(mFileName, code->E);
					if (my_access(mFileName, 0) != 0) {
						if (mkdir(mFileName, MODE)) {
							persuade_cleanup();
							fprintf(stderr, "\n   Error creating folder \"%s\".\n", mFileName);
							fprintf(stderr, "   Folder already exists, Please delete it first.\n");
							cutt_exit(0);
						}
					}
					addFilename2Path(mFileName, code->A);
					strcat(mFileName, ".cha");
					if (strcmp(oldA, code->A) != 0) {
						if (fp != NULL) {
							fprintf(fp, "@End\n");
							fclose(fp);
						}
						if ((fp=fopen(mFileName,"w")) == NULL) {
							fprintf(stderr, "\nCan't write file: %s\n", mFileName);
							cutt_exit(0);
						}
						strcpy(oldA, code->A);
						fprintf(fp, "@UTF8\n");
						fprintf(fp, "@Begin\n");
						fprintf(fp, "@Languages:\teng\n");
						fprintf(fp, "@Participants:	STU Student\n");
						fprintf(fp, "@ID:	eng|Persuade|STU||%s|TD|", 
								((uS.mStricmp(code->I, "M")  == 0) ? "male" : "female"));
						race[0] = EOS;
						if (code->L[0] != EOS) {
							if (uS.mStrnicmp(code->L, "Black", 5) == 0)
								strcpy(race, "Black");
							else if (uS.mStrnicmp(code->L, "Asian", 5) == 0)
								strcpy(race, "Asian");
							else if (uS.mStrnicmp(code->L, "Hispanic", 8) == 0)
								strcpy(race, "Latino");
							else if (uS.mStrnicmp(code->L, "White", 5) == 0)
								strcpy(race, "White");
							else if (uS.mStrnicmp(code->L, "American Indian", 15) == 0)
								strcpy(race, "Native");
							else if (uS.mStrnicmp(code->L, "Two or", 6) == 0)
								strcpy(race, "Multiple");
							else
								fprintf(stderr, "\nUnknown race: %s\n", code->L);
						}
						SES[0] = EOS;
						if (code->M[0] != EOS) {
							if (uS.mStrnicmp(code->M, "Not economically", 16) == 0)
								strcpy(SES, "MC");
							else if (uS.mStrnicmp(code->M, "Economically", 12) == 0)
								strcpy(SES, "LI");
							else
								fprintf(stderr, "\nUnknown SES: %s\n", code->M);
						}
						if (race[0] == EOS)
							fprintf(fp, "%s", SES);
						else if (SES[0] == EOS)
							fprintf(fp, "%s", race);
						else
							fprintf(fp, "%s,%s", race, SES);
						fprintf(fp, "|Student|%s|%s|\n", code->J, code->F);
						
						if (code->E[0] != EOS) {
							strcpy(templineC1, root_codes->E);
							strcat(templineC1, " - ");
							strcat(templineC1, code->E);
							fprintf(fp, "@Comment:\t"); printoutline(fp, templineC1);
						}
						
						if (code->G[0] != EOS) {
							strcpy(templineC1, root_codes->G);
							strcat(templineC1, " - ");
							strcat(templineC1, code->G);
							fprintf(fp, "@Comment:\t"); printoutline(fp, templineC1);
						}
						
						if (code->H[0] != EOS) {
							strcpy(templineC1, root_codes->H);
							strcat(templineC1, " - ");
							strcat(templineC1, code->H);
							fprintf(fp, "@Comment:\t"); printoutline(fp, templineC1);
						}
						
						if (code->K[0] != EOS) {
							strcpy(templineC1, root_codes->K);
							strcat(templineC1, " - ");
							strcat(templineC1, code->K);
							fprintf(fp, "@Comment:\t"); printoutline(fp, templineC1);
						}
						
						if (code->N[0] != EOS) {
							strcpy(templineC1, root_codes->N);
							strcat(templineC1, " - ");
							strcat(templineC1, code->N);
							fprintf(fp, "@Comment:\t"); printoutline(fp, templineC1);
						}

						if (fp != NULL && C != NULL) {
							i = 0;
							e = 0;
							templineC1[i] = EOS;
							while (C[e] != EOS) {
								templineC1[i++] = C[e];
								if (C[e] == '.' || C[e] == '?' || C[e] == '!') {
									templineC1[i] = EOS;
									uS.remFrontAndBackBlanks(templineC1);
									removeExtraSpace(templineC1);
									if (templineC1[0] != EOS) {
										fprintf(fp, "*STU:\t");
										printoutline(fp, templineC1);
									}
									i = 0;
									templineC1[i] = EOS;
								}
								e++;
							}
							templineC1[i] = EOS;
							uS.remFrontAndBackBlanks(templineC1);
							removeExtraSpace(templineC1);
							if (templineC1[0] != EOS) {
								fprintf(fp, "*STU:\t");
								printoutline(fp, templineC1);
								i = 0;
								templineC1[i] = EOS;
							}
							free(C);
							C = NULL;
						}
					}
				}
				code = NULL;
				col = 1;
				if (!feof(fpin))
					persuade_ln++;
			} else {
				col++;
			}
		} else {
			if (index < tMemSize) {
				col_text[index++] = chrs;
			} else {
				tMemSize += UTTLINELEN;
				col_text[index] = EOS;
				tc = col_text;
				col_text = (char *)malloc(tMemSize+1);
				if (col_text == NULL)
					persuade_overflow();
				strcpy(col_text, tc);
				free(tc);
				index = strlen(col_text);
				col_text[index++] = chrs;
			}
		}
	}
	free(col_text);
	if (fp != NULL)
		fclose(fp);
	fprintf(stderr,"\r%ld\n", persuade_ln);
//	printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine, FALSE);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	struct errcodes *code;

	errCodesCnt = 0;
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);

	if (root_errcodes != NULL) {
		fprintf(stderr,"\nCan't find %d codes for essay_id_comp:\n", errCodesCnt);
		for (code=root_errcodes; code != NULL; code=code->next) {
			fprintf(stderr,"    %s\n", code->A);
		}
	}
	persuade_cleanup();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
				Codes_FName = f;
				break;
		case 'e':
				isShowErr = TRUE;
				no_arg_option(f);
				break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
