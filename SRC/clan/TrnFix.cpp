/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3

#include "cu.h"
#if !defined(UNX)
#include "c_curses.h"

#define _main trnfix_main
#define call trnfix_call
#define getflag trnfix_getflag
#define init trnfix_init
#define usage trnfix_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

extern struct tier *headtier;
extern struct tier *defheadtier;
extern char GExt[];
extern char OverWriteFile;
extern char Preserve_dir;

struct trnfix_lines {
	unsigned int count;
	char *line;
	struct trnfix_lines *nextLine;
} ;

struct trnfix_tnode {
	char *word;
	unsigned int count;
	struct trnfix_lines *lines;
	struct trnfix_tnode *left;
	struct trnfix_tnode *right;
};

static char *spk, *trn, *mor, isDis, whichDopt;
static char spkCode[12], morCode[12], trnCode[12];
static int  totalMismatched, totalWords;
static struct trnfix_tnode *errors;

void usage() {
	puts("trnfix compairs %trn tier to %mor tier.");
	printf("Usage: trnfix [%s] filename(s)\n", mainflgs());
	puts("+a : disambiguate words, then compare, (default: compare whole words)");
	puts("+bS: Specify tiers you want to compare, (default: %mor and %trn)");
	puts("+d : Include speaker tier in the output, (default: no speaker tier)");
	puts("+d1: Include speaker tier in the output and utterances in mismatches summary file");
	mainusage(FALSE);
	puts("\nExample: trnfix +b%gra +b%grt filename");
	cutt_exit(0);
}

void init(char first) {
	if (first) {
		spkCode[0] = EOS;
		morCode[0] = EOS;
		trnCode[0] = EOS;
		errors = NULL;
		isDis = FALSE;
		whichDopt = 0;
		OverWriteFile = TRUE;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		if (morCode[0] != EOS && trnCode[0] == EOS) {
			fprintf(stderr,"Please specify the second comparable tier to compare with +b%s option.\n", morCode);
			cutt_exit(0);
		} else if (morCode[0] == EOS && trnCode[0] != EOS) {
			fprintf(stderr,"Please specify the second comparable tier to compare with +b%s option.\n", trnCode);
			cutt_exit(0);
		}
		if (morCode[0] == EOS && trnCode[0] == EOS) {
			strcpy(morCode, "%mor:");
			strcpy(trnCode, "%trn:");
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	int len;

	f++;
	switch(*f++) {
		case 'a':
			isDis = TRUE;
			no_arg_option(f);
			break;
		case 'b':
			if (!*f) {
				fprintf(stderr,"Dependent tier name expected after %s option.\n", f-2);
				cutt_exit(0);
			}
			len = strlen(f) - 1;
			if (uS.mStrnicmp(f, "%mor", 4) == 0 || uS.mStrnicmp(f, "%gra", 4) == 0) {
				strncpy(morCode, f, 10);
				if (morCode[len] != ':')
					strcat(morCode, ":");
				uS.lowercasestr(morCode, &dFnt, FALSE);
			} else if (uS.mStrnicmp(f, "%trn", 4) == 0 || uS.mStrnicmp(f, "%grt", 4) == 0) {
				strncpy(trnCode, f, 10);
				if (trnCode[len] != ':')
					strcat(trnCode, ":");
				uS.lowercasestr(trnCode, &dFnt, FALSE);
			} else {
				if (morCode[0] == EOS) {
					strncpy(morCode, f, 10);
					if (morCode[len] != ':')
						strcat(morCode, ":");
					uS.lowercasestr(morCode, &dFnt, FALSE);
				} else if (trnCode[0] == EOS) {
					strncpy(trnCode, f, 10);
					if (trnCode[len] != ':')
						strcat(trnCode, ":");
					uS.lowercasestr(trnCode, &dFnt, FALSE);
				}
			}
			break;
		case 'd':
			if (*f == EOS)
				whichDopt = 1;
			else
				whichDopt = 2;
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void freeLines(struct trnfix_lines *lines) {
	struct trnfix_lines *t;

	while (lines != NULL) {
		t = lines;
		lines = lines->nextLine;
		if (t->line != NULL)
			free(t->line);
		free(t);
	}
}

static struct trnfix_tnode *trnfix_freetree(struct trnfix_tnode *p) {
	if (p != NULL) {
		trnfix_freetree(p->left);
		trnfix_freetree(p->right);
		free(p->word);
		freeLines(p->lines);
		free(p);
	}
	return(NULL);
}

static void trnfix_overflow() {
	fprintf(stderr,"trnfix: no more memory available.\n");
	errors = trnfix_freetree(errors);
	cutt_exit(0);
}

static struct trnfix_lines *trnfix_AddNewLines(struct trnfix_lines *lines, char *line) {
	struct trnfix_lines *p;

	if (lines == NULL) {
		if ((p=NEW(struct trnfix_lines)) == NULL)
			trnfix_overflow();
		lines = p;
	} else {
		for (p=lines; p->nextLine != NULL; p=p->nextLine) {
			if (!strcmp(p->line, line)) {
				p->count++;
				return(lines);
			}
		}
		if (!strcmp(p->line, line)) {
			p->count++;
			return(lines);
		}
		if ((p->nextLine=NEW(struct trnfix_lines)) == NULL)
			trnfix_overflow();
		p = p->nextLine;
	}
	p->nextLine = NULL;
	p->line = NULL;
	p->count = 1;
	if ((p->line=(char *)malloc(strlen(line)+1)) == NULL)
		trnfix_overflow();
	strcpy(p->line, line);
	return(lines);
}

static struct trnfix_tnode *trnfix_tree(struct trnfix_tnode *p, char *w, char *line) {
	int cond;
	struct trnfix_tnode *t = p;

	if (p == NULL) {
		if ((p=NEW(struct trnfix_tnode)) == NULL)
			trnfix_overflow();
		p->lines = NULL;
		if ((p->word=(char *)malloc(strlen(w)+1)) == NULL)
			trnfix_overflow();
		strcpy(p->word, w);
		p->count = 1;
		p->lines = trnfix_AddNewLines(p->lines, line);
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->word)) == 0) {
		p->count++;
		p->lines = trnfix_AddNewLines(p->lines, line);
	} else if (cond < 0) {
		p->left = trnfix_tree(p->left, w, line);
	} else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
			p->count++;
			p->lines = trnfix_AddNewLines(p->lines, line);
		} else if (cond < 0) {
			p->left = trnfix_tree(p->left, w, line);
		} else {
			p->right = trnfix_tree(p->right, w, line); /* if cond > 0 */
		}
		return(t);
	}
	return(p);
}

static void freq_printlines(FILE *fp, struct trnfix_tnode *p) {
	struct trnfix_lines *line;

	for (line=p->lines; line != NULL; line=line->nextLine) {
		fprintf(fp,"    %3u %s\n", line->count, line->line);
	}
}

static void trnfix_treeprint(FILE *fp, struct trnfix_tnode *p) {
	if (p != NULL) {
		trnfix_treeprint(fp, p->left);
		do {
			fprintf(fp,"%3u ", p->count);
			fprintf(fp,"%s\n", p->word);
			freq_printlines(fp, p);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				trnfix_treeprint(fp, p->right);
				break;
			}
			p = p->right;
		} while (1);
	}
}

static char my_equal(char trnC, char morC) {
	if ((isSpace(trnC) || trnC == '\n') && (isSpace(morC) || morC == '\n'))
		return(TRUE);
	if (trnC == morC)
		return(TRUE);
	return(FALSE);
}

static char puncts(long ti) {
	if (uS.mStrnicmp(trn+ti, "end|", 4) == 0 || uS.mStrnicmp(trn+ti, "beg|", 4) == 0 || uS.mStrnicmp(trn+ti, "cm|", 3) == 0
//2019-04-29		|| uS.mStrnicmp(trn+ti, "bq|", 3) == 0  || uS.mStrnicmp(trn+ti, "eq|", 3) == 0
		)
		return(TRUE);
	return(FALSE);
}

static void removeLemma(char *w) {
	int  s, e;
	char *sb;

	if (uS.mStricmp(morCode, "%gra:") == 0) {
		sb = strrchr(w, '|');
		if (sb != NULL) {
			strcpy(w, sb+1);
		}
	} else {
		for (s=0; w[s] != '|' && w[s] != EOS; s++) ;
		for (e=s; w[e] != '-' && w[e] != '&' && w[e] != '~' && w[e] != '=' && w[e] != '@' && w[e] != '*' && w[e] != EOS; e++) ;
		if (s+1 < e) {
			strcpy(w+s+1, w+e);
		}
		if (w[s] == '|') {
			for (s++; w[s] != '|' && w[s] != EOS; s++) ;
			for (e=s; w[e] != '-' && w[e] != '&' && w[e] != '~' && w[e] != '=' && w[e] != '@' && w[e] != '*' && w[e] != EOS; e++) ;
			if (s < e) {
				strcpy(w+s, w+e);
			}
		}
	}
}

static void concatAndDoubleTabs(char *to, char *from) {
	int t, f;
	char isHiddenC;

	t = strlen(to);
	isHiddenC = FALSE;
	for (f=0; from[f] != EOS; f++) {
		if (from[f] == HIDEN_C) {
			isHiddenC = !isHiddenC;
		} else if (!isHiddenC) {
			if (from[f] == '\t')
				to[t++] = '\t';
			to[t++] = from[f];
		}
	}
	to[t] = EOS;
}

static void compairTiers(long ln, long *total, long *mismatched) {
	char isErrorFound, isBegTErrorMarker, isBegMErrorMarker;
	char trnW[BUFSIZ+2], morW[BUFSIZ+2];
	int  si, sj;
	int  ti, mi, ts, ms;

	ti = 0L;
	mi = 0L;
	ts = 0L;
	ms = 0L;
	trnW[0] = EOS;
	morW[0] = EOS;
	isBegTErrorMarker = FALSE;
	isBegMErrorMarker = FALSE;
	isErrorFound = FALSE;
	while (trn[ti] != EOS && mor[mi] != EOS) {
		if (!my_equal(trn[ti], mor[mi])) {
			if (isDis) {
				if (mor[mi] == '^' && (isSpace(trn[ti]) || trn[ti] == '\n' || trn[ti] == EOS)) {
					while (!isSpace(mor[mi]) && mor[mi] != '\n' && mor[mi] != EOS)
						mi++;
				} else {
					while (!isSpace(mor[mi]) && mor[mi] != '\n'  && mor[mi] != '^' && mor[mi] != EOS)
						mi++;
					if (mor[mi] == '^') {
						mi++; 
						while (!isSpace(trn[ti]) && trn[ti] != '\n' && ti != 0)
							ti--;
						if (isSpace(trn[ti]) || trn[ti] == '\n')
							ti++;
					} else {
						while (!isSpace(trn[ti]) && trn[ti] != '\n' && trn[ti] != EOS)
							ti++;
						isErrorFound = TRUE;
					}
				}
			} else {
				while (!isSpace(trn[ti]) && trn[ti] != '\n' && trn[ti] != EOS)
					ti++;
				while (!isSpace(mor[mi]) && mor[mi] != '\n' && mor[mi] != EOS)
					mi++;
				isErrorFound = TRUE;
			}
#ifndef UNX
			if (isErrorFound) {
				if (!isBegTErrorMarker) {
					*mismatched = *mismatched + 1L;
					uS.shiftright(trn+ts, 2);
					trn[ts++] = ATTMARKER;
					trn[ts] = error_start;
					ti += 2;
					isBegTErrorMarker = TRUE;
				}
				if (!isBegMErrorMarker) {
					uS.shiftright(mor+ms, 2);
					mor[ms++] = ATTMARKER;
					mor[ms] = error_start;
					mi += 2;
					isBegMErrorMarker = TRUE;
				}
			}
#endif
		} else {
			if (isSpace(trn[ti]) || trn[ti] == '\n') {
#ifndef UNX
				if (isBegTErrorMarker) {
					si = 0;
					for (sj=ts+1; sj < ti && si < BUFSIZ; sj++) {
						trnW[si++] = trn[sj];
					}
					trnW[si] = EOS;
					uS.shiftright(trn+ti, 2);
					trn[ti++] = ATTMARKER;
					trn[ti++] = error_end;
					isBegTErrorMarker = FALSE;
				}
#endif
				while (isSpace(trn[ti]) || trn[ti] == '\n')
					ti++;
				if (trn[ti] != '+' && !uS.IsUtteranceDel(trn, ti) && !puncts(ti))
					*total = *total + 1L;
				ts = ti;
			} else {
				if (trn[ti] != EOS)
					ti++;
			}
			if (isSpace(mor[mi]) || mor[mi] == '\n') {
#ifndef UNX
				if (isBegMErrorMarker) {
					si = 0;
					for (sj=ms+1; sj < mi && si < BUFSIZ; sj++) {
						morW[si++] = mor[sj];
					}
					morW[si] = EOS;
					uS.shiftright(mor+mi, 2);
					mor[mi++] = ATTMARKER;
					mor[mi++] = error_end;
					isBegMErrorMarker = FALSE;
				}
				if (trnW[0] != EOS || morW[0] != EOS) {
					strcpy(spareTier2, morW);
					strcat(spareTier2, " <> ");
					strcat(spareTier2, trnW);
					removeLemma(morW);
					removeLemma(trnW);
					strcpy(spareTier1, morW);
					strcat(spareTier1, " <> ");
					strcat(spareTier1, trnW);
					if (whichDopt == 2) {
						strcat(spareTier2, "\n\t");
						sprintf(spareTier3, "*** File \"%s\": line %ld.\n", oldfname, ln);
						strcat(spareTier2, spareTier3);
						strcat(spareTier2, "\t");
						strcat(spareTier2, spkCode);
						concatAndDoubleTabs(spareTier2, spk);
						strcat(spareTier2, "\t");
						strcat(spareTier2, morCode);
						strcat(spareTier2, "\t");
						concatAndDoubleTabs(spareTier2, mor);
						strcat(spareTier2, "\t");
						strcat(spareTier2, trnCode);
						strcat(spareTier2, "\t");
						concatAndDoubleTabs(spareTier2, trn);
					}
					errors = trnfix_tree(errors, spareTier1, spareTier2);
					trnW[0] = EOS;
					morW[0] = EOS;
				}
#endif
				while (isSpace(mor[mi]) || mor[mi] == '\n')
					mi++;
				ms = mi;
			} else {
				if (mor[mi] != EOS)
					mi++;
			}
		}
	}
	while (isSpace(trn[ti]) || trn[ti] == '\n')
		ti++;
	while (isSpace(mor[mi]) || mor[mi] == '\n')
		mi++;
	if (trn[ti] != EOS || mor[mi] != EOS)
		isErrorFound = TRUE;
	if (isErrorFound) {
#ifndef UNX
		if (isBegTErrorMarker) {
			uS.shiftright(trn+ti, 2);
			trn[ti++] = ATTMARKER;
			trn[ti] = error_end;
		}
		if (isBegMErrorMarker) {
			uS.shiftright(mor+mi, 2);
			mor[mi++] = ATTMARKER;
			mor[mi] = error_end;
		}
#endif
		if (whichDopt) {
			fprintf(fpout,"@Comment:	*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(fpout,"%s %s", spkCode, spk);
			fprintf(fpout,"%s %s", morCode, mor);
			fprintf(fpout,"%s %s\n", trnCode, trn);
		} else {
			fprintf(fpout,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(fpout,"  %s %s", morCode, mor);
			fprintf(fpout,"  %s %s", trnCode, trn);
		}
	}
}

void call(void) {
	char RightTier;
	long ln, total, mismatched;

	ln = lineno;
	RightTier = FALSE;
	spk[0] = trn[0] = mor[0] = EOS;
	total = 0L;
	mismatched = 0L;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			RightTier = checktier(utterance->speaker);
		}
		if (RightTier) {
 			if (utterance->speaker[0] == '*' || uS.partcmp(utterance->speaker, "@End", FALSE, FALSE)) {
				if (trn[0] != EOS && mor[0] != EOS) {
					compairTiers(ln, &total, &mismatched);
				} else if (trn[0] == EOS && mor[0] != EOS) {
					fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, ln);
					fprintf(fpout,"  Missing \"%s\" tier, while tier \"%s\" is present\n", trnCode, morCode);
				} else if (trn[0] != EOS && mor[0] == EOS) {
					fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, ln);
					fprintf(fpout,"  Missing \"%s\" tier, while tier \"%s\" is present\n", morCode, trnCode);
				}
				trn[0] = mor[0] = EOS;
				strncpy(spkCode, utterance->speaker, 10);
				spkCode[11] = EOS;
				strcpy(spk, utterance->line);
				ln = lineno;
			} else if (uS.partcmp(utterance->speaker, morCode, FALSE, TRUE)) {
				strcpy(mor, utterance->line);
			} else if (uS.partcmp(utterance->speaker, trnCode, FALSE, TRUE)) {
				strcpy(trn, utterance->line);
			}
		}
	}
	if (RightTier) {
		if (trn[0] != EOS && mor[0] != EOS) {
			compairTiers(ln, &total, &mismatched);
		} else if (trn[0] == EOS && mor[0] != EOS) {
			fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(fpout,"  Missing \"%s\" tier, while tier \"%s\" is present\n", trnCode, morCode);
		} else if (trn[0] != EOS && mor[0] == EOS) {
			fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(fpout,"  Missing \"%s\" tier, while tier \"%s\" is present\n", morCode, trnCode);
		}
		spk[0] = trn[0] = mor[0] = EOS;
	}
	if (whichDopt) {
		fprintf(fpout,"\n@Comment:	Total number of items on %s\t%ld\n", trnCode, total);
		fprintf(fpout,"@Comment:	Total number of matches:\t%ld\n", total-mismatched);
		fprintf(fpout,"@Comment:	Total number of mis-matches:\t%ld\n\n", mismatched);
	} else {
		fprintf(fpout,"\nTotal number of items on %s\t%ld\n", trnCode, total);
		fprintf(fpout,"Total number of matches:\t%ld\n", total-mismatched);
		fprintf(fpout,"Total number of mis-matches:\t%ld\n\n", mismatched);
	}
	totalWords += total;
	totalMismatched += mismatched;
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	float perc, te, tw;
	FILE *fp;

	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = TRNFIX;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	errors = NULL;
	totalWords = 0;
	totalMismatched = 0;
	spk = (char *)malloc(UTTLINELEN+2);
	if (spk == NULL) {
		fprintf(stderr, "ERROR: Out of memory\n");
		cutt_exit(0);
	}
	trn = (char *)malloc(UTTLINELEN+2);
	if (trn == NULL) {
		fprintf(stderr, "ERROR: Out of memory\n");
		free(spk);
		cutt_exit(0);
	}
	mor = (char *)malloc(UTTLINELEN+2);
	if (mor == NULL) {
		free(spk);
		free(trn);
		fprintf(stderr, "ERROR: Out of memory\n");
		cutt_exit(0);
	}
	bmain(argc,argv,NULL);
#ifndef UNX
	if (Preserve_dir)
		strcpy(spareTier1, wd_dir);
	else
		strcpy(spareTier1, od_dir);
	AddCEXExtension = ".cut";
	addFilename2Path(spareTier1, "0_Summary");
	strcat(spareTier1, GExt);
//	OverWriteFile = FALSE;
	if ((fp=openwfile("", spareTier1, NULL)) != NULL) {
		fprintf(fp, "  %s <> %s\n", morCode, trnCode);
		trnfix_treeprint(fp, errors);
		fprintf(fp, "Total items on %s %d\n", trnCode, totalWords);
		fprintf(fp, "Total errors: %d\n", totalMismatched);
		te = (float)totalMismatched;
		tw = (float)totalWords;
		perc = 100.0000 - ((te * 100.0000) / tw);
		fprintf(fp, "%.2f%%\n", perc);

		fclose(fp);
	} else {
		fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n", spareTier1);
		cutt_exit(0);
	}
#endif
	errors = trnfix_freetree(errors);
	free(spk);
	free(trn);
	free(mor);
}
