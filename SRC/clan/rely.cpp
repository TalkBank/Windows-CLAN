/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main rely_main
#define call rely_call
#define getflag rely_getflag
#define init rely_init
#define usage rely_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

struct code {
	char *name;
	char matched;
	struct code *nextcode;
} ;

struct rtiers {
	char *speaker;	/* code descriptor field of the turn	 */
	AttTYPE *attSp;
	char *line;		/* text field of the turn		 */
	AttTYPE *attLine;
	char matched;
	char isKappaChecked;
	long lineno;
	struct rtiers *nexttier;	/* pointer to the next utterance, if	 */
} ;				/* there is only 1 utterance, i.e. no	 */

struct files {
	int  res;
	long lineno;
	long tlineno;
	FNType fname[FNSize];
	char currentchar;
	AttTYPE currentatt;
	FILE *fpin;
	struct rtiers *tiers;
	struct code *rootcode;
} ;

struct rely_tnode {
	char *word;
	char *POS;
	long fFileCnt;
	long sFileCnt;
	struct rely_tnode *left;
	struct rely_tnode *right;
};

static int  fTotalUtts, sTotalUtts;
static int  targc;
static char **targv;
static char FirstFile, FirstArgsPrint;
static char isCombineForAllFiles;
static char isComputeAphasia;
static char isComputeStudentCorrectness; // 2019-11-20
static char isSOption;
static char isAddTiersToMasterFile;
static char isCompTiers;
static char isMerge;
static char rely_FTime;
static char isHeaderTierSpecified, isDepSpTierSpecified, isDepTierSpecified, isDepTierIncluded;
static char includeBullets;
static long controlItemCnt, studenItemCnt, studentItemsMatched, studentItemsNotInMaster;
static float KappaCats, agreedNum, totalNum;
static FNType OutFile[FNSize];
static struct rely_tnode *rely_words_root;
static struct files ffile, sfile;

extern char OverWriteFile;
extern struct tier *headtier;
extern struct tier *defheadtier;

void usage() {
	puts("RELY ");
	printf("Usage: rely [a b c d dN u m %s] control-filename second-filename\n", mainflgs());
	puts("+a : add tiers from second file to first (control) file");
	puts("+b : include BULLETS in string comparison between first and control files");
	puts("+c : do not compare data on non-selected tier");
	puts("+d : Compute percentage agreement coefficient.");
	puts("+dmN: Compute student correctness. (+dm1 - first file is control, +dm2 second file is control)");
	puts("+dN: Compute Cohen's kappa coefficient and specify number of possible categories.");
	puts("+u : Compute Kappa across all pairs of files specified (default: for each individual pair).");
	puts("+m : merge files and place error flags inside output file");
	mainusage(TRUE);
}

void init(char first) {
	if (first) {
		includeBullets = FALSE;
		isCompTiers = TRUE;
		isMerge = FALSE;
		isAddTiersToMasterFile = FALSE;
		isHeaderTierSpecified = FALSE;
		isDepSpTierSpecified = FALSE;
		isDepTierSpecified = FALSE;
		isDepTierIncluded = FALSE;
		rely_FTime = TRUE;
		isCombineForAllFiles = FALSE;
		isComputeAphasia = FALSE;
		isComputeStudentCorrectness = FALSE;
		isSOption = FALSE;
		KappaCats = 0.0;
		agreedNum = 0.0;
		totalNum = 0.0;
		fTotalUtts = 0;
		sTotalUtts = 0;
		rely_words_root = NULL;
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		if (rely_FTime) {
			unsigned int i = 0;

			rely_FTime = FALSE;
			if (isComputeAphasia) {
				addword('\0','\0',"+</?>");
				addword('\0','\0',"+</->");
				addword('\0','\0',"+<///>");
				addword('\0','\0',"+<//>");
				addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
				addword('\0','\0',"+0");
				addword('\0','\0',"+&*");
				addword('\0','\0',"+-*");
				addword('\0','\0',"+#*");
				addword('\0','\0',"+(*.*)");
				FilterTier = 1;
			} else {
				for (i=0; GlobalPunctuation[i]; ) {
					if (GlobalPunctuation[i] == '!' ||
						GlobalPunctuation[i] == '?' ||
						GlobalPunctuation[i] == '.') 
						strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
					else
						i++;
				}
			}
			if (isAddTiersToMasterFile && !isDepTierIncluded) {
				fprintf(stderr, "Please specify dependent tier you are adding with +t option\n");
				cutt_exit(0);
			}
			if (isAddTiersToMasterFile && isComputeAphasia) {
				fprintf(stderr, "+d option can not be used with +a option\n");
				cutt_exit(0);
			}
			if (isAddTiersToMasterFile && isComputeStudentCorrectness) {
				fprintf(stderr, "+dm option can not be used with +a option\n");
				cutt_exit(0);
			}
			if (isComputeStudentCorrectness && isSOption) {
				fprintf(stderr, "+dm option can not be used with +s option\n");
				cutt_exit(0);
			}
			if (KappaCats > 0.0 && isComputeStudentCorrectness) {
				fprintf(stderr, "+dm option can not be used with +dN option\n");
				cutt_exit(0);
			}
			if (fpin == stdin) {
				fprintf(stderr, "Input must come from files\n");
				cutt_exit(0);
			}
			if (isAddTiersToMasterFile)
				isMerge = TRUE;
			if (isMerge)
				chatmode = 3;

			if (isDepSpTierSpecified && !isHeaderTierSpecified) {
				defheadtier = NEW(struct tier);
				defheadtier->include = FALSE;
				defheadtier->pat_match = FALSE;
				strcpy(defheadtier->tcode,"@");
				defheadtier->nexttier = NULL;
			} else if (isHeaderTierSpecified && !isDepSpTierSpecified) {
			}
		}
		if (KappaCats > 0.0) {
			if (!isDepTierSpecified) {
				fprintf(stderr, "Please specify at least one dependent tier with +t option to compare\n");
				cutt_exit(0);
			}
			if (!isCombineForAllFiles) {
				agreedNum = 0.0;
				totalNum = 0.0;
			}
			isCompTiers = FALSE;
		} else if (isCombineForAllFiles && !isComputeAphasia && !isDepTierIncluded) {
			fprintf(stderr, "Please specify at least one dependent tier with +t option to compare\n");
			cutt_exit(0);
		} else if (isComputeStudentCorrectness && !isDepTierIncluded) {
			fprintf(stderr, "Please specify the dependent tier with +t option to compare\n");
			cutt_exit(0);
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'a':
			isAddTiersToMasterFile = TRUE;
			onlydata = 1;
			no_arg_option(f);
			break;
		case 'b':
			includeBullets = TRUE;
			no_arg_option(f);
			break;
		case 'c':
			isCompTiers = FALSE;
			no_arg_option(f);
			break;
		case 'd':
			if (*f == EOS) {
				isComputeAphasia = TRUE;
			} else if (*f == 'm' || *f == 'M') {
				if (*(f+1) == '2')
					isComputeStudentCorrectness = 2;
				else if (*(f+1) == EOS || *(f+1) == '1')
					isComputeStudentCorrectness = 1;
				else {
					fprintf(stderr, "+dm optionmust be either +dm, +dm1 or +dm2\n");
					cutt_exit(0);
				}
			} else {
				KappaCats = atoi(f);
				if (KappaCats <= 0.0) {
					fprintf(stderr,"Total categories value for Kappa calculation has to be greater than 0.\n");
					cutt_exit(0);
				}
			}
			break;
		case 'm':
			isMerge = TRUE;
			onlydata = 1;
			no_arg_option(f);
			break;
		case 'u':
			isCombineForAllFiles = TRUE;
			no_arg_option(f);
			break;
		case 's':
			if (*f != EOS)
				isSOption = TRUE;
			maingetflag(f-2,f1,i);
			break;
		case 't':
			if (*f == '%') {
				if (*(f-2) == '+')
					isDepTierIncluded = TRUE;
				isDepTierSpecified = TRUE;
				isDepSpTierSpecified = TRUE;
			} else if (*f == '*') {
				isDepSpTierSpecified = TRUE;
			} else if (*f == '@') {
				isHeaderTierSpecified = TRUE;
			}
			maingetflag(f-2,f1,i);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static struct rtiers *FreeUpTiers(struct rtiers *p) {
	struct rtiers *t;

	while (p != NULL) {
		t = p;
		p = p->nexttier;
		free(t->speaker);
		free(t->attSp);
		free(t->line);
		free(t->attLine);
		free(t);
	}
	return(NULL);
}

static struct code *freeupcodes(struct code *p) {
	struct code *t;

	while (p != NULL) {
		t = p;
		p = p->nextcode;
		free(t->name);
		free(t);
	}
	return(NULL);
}

static void rely_freetree(struct rely_tnode *p) {
	if (p != NULL) {
		rely_freetree(p->left);
		rely_freetree(p->right);
		if (p->word != NULL)
			free(p->word);
		if (p->POS != NULL)
			free(p->POS);
		free(p);
	}
}

static struct rely_tnode *rely_talloc(char *word, char *POS) {
	struct rely_tnode *p;
	
	if ((p=NEW(struct rely_tnode)) == NULL) {
		fputs("ERROR: Out of memory.\n",stderr);
		rely_freetree(rely_words_root);
		ffile.tiers = FreeUpTiers(ffile.tiers);
		sfile.tiers = FreeUpTiers(sfile.tiers);
		cutt_exit(0);
	}
	p->left = p->right = NULL;
	p->POS = NULL;
	p->fFileCnt = 0L;
	p->sFileCnt = 0L;
	if ((p->word=(char *)malloc(strlen(word)+1)) != NULL)
		strcpy(p->word, word);
	else {
		free(p);
		fputs("ERROR: Out of memory.\n",stderr);
		rely_freetree(rely_words_root);
		ffile.tiers = FreeUpTiers(ffile.tiers);
		sfile.tiers = FreeUpTiers(sfile.tiers);
		cutt_exit(0);
	}
	if (POS != NULL) {
		if (p->POS != NULL) {
			strcpy(templineC3, p->POS);
			free(p->POS);
		} else
			templineC3[0] = EOS;
		if ((p->POS=(char *)malloc(strlen(POS)+strlen(templineC3)+3)) != NULL) {
			if (templineC3[0] != EOS) {
				strcpy(p->POS, templineC3);
				strcat(p->POS, ", ");
				strcat(p->POS, POS);
			} else
				strcpy(p->POS, POS);
		} else {
			free(p->word);
			free(p);
			fputs("ERROR: Out of memory.\n",stderr);
			rely_freetree(rely_words_root);
			ffile.tiers = FreeUpTiers(ffile.tiers);
			sfile.tiers = FreeUpTiers(sfile.tiers);
			cutt_exit(0);
		}
	}
	return(p);
}

static struct rely_tnode *rely_tree(rely_tnode *p,char *w,char *m,char isfFile) {
	int cond;
	
	if (p == NULL) {
		p = rely_talloc(w, m);
		if (isfFile)
			p->fFileCnt = 1L;
		else
			p->sFileCnt = 1L;
	} else if ((cond=strcmp(w,p->word)) == 0) {
		if (isfFile)
			p->fFileCnt++;
		else
			p->sFileCnt++;
	} else if (cond < 0) {
		p->left = rely_tree(p->left, w, m, isfFile);
	} else {
		p->right = rely_tree(p->right, w, m, isfFile); /* if cond > 0 */
	}
	return(p);
}


static struct rtiers *AddTiers(struct rtiers *p, char *sp, AttTYPE *attSp, char *line, AttTYPE *attLine, long lineno) {
	if (p == NULL) {
		if ((p=NEW(struct rtiers)) == NULL) out_of_mem();
		if ((p->speaker=(char *)malloc(strlen(sp)+1)) == NULL) out_of_mem();
		if ((p->attSp=(AttTYPE *)malloc((strlen(sp)+1)*sizeof(AttTYPE))) == NULL) out_of_mem();
		if ((p->line=(char *)malloc(strlen(line)+1)) == NULL) out_of_mem();
		if ((p->attLine=(AttTYPE *)malloc((strlen(line)+1)*sizeof(AttTYPE))) == NULL) out_of_mem();
		att_cp(0, p->speaker, sp, p->attSp, attSp);
		att_cp(0, p->line, line, p->attLine, attLine);
		p->matched = FALSE;
		p->lineno = lineno;
		p->nexttier = NULL;
	} else
		p->nexttier = AddTiers(p->nexttier, sp, attSp, line, attLine, lineno);
	return(p);
}

static struct code *AddWord(struct code *p, char *w) {
	if (p == NULL) {
		if ((p=NEW(struct code)) == NULL) out_of_mem();
		if ((p->name=(char *)malloc(strlen(w)+1)) == NULL) out_of_mem();
		strcpy(p->name, w);
		p->matched = FALSE;
		p->nextcode = NULL;
	} else
		p->nextcode = AddWord(p->nextcode, w);
	return(p);
}

static char issameword(struct code *p, char *w) {
	while (p != NULL) {
		if (!p->matched && !strcmp(p->name, w)) {
			p->matched = TRUE;
			return(TRUE);
		}
		p = p->nextcode;
	}
	return(FALSE);
}

static char smartCompareTiers(char *fline, char *sline) {
	if (includeBullets) {
		while (*fline != EOS && *sline != EOS) {
			while (isSpace(*fline) || *fline == '\n') fline++;
			while (isSpace(*sline) || *sline == '\n') sline++;
			if (*fline != *sline)
				return(TRUE);
			if (*fline == EOS || *sline == EOS)
				break;
			fline++;
			sline++;
		}
	} else {
		while (*fline != EOS && *sline != EOS) {
			while (isSpace(*fline) || *fline == '\n' || *fline == HIDEN_C) {
				if (*fline == HIDEN_C) {
					for (fline++; *fline != EOS && *fline != HIDEN_C; fline++) ;
					if (*fline == EOS)
						break;
					else
						fline++;
				} else
					fline++;
			}
			while (isSpace(*sline) || *sline == '\n' || *sline == HIDEN_C) {
				if (*sline == HIDEN_C) {
					for (sline++; *sline != EOS && *sline != HIDEN_C; sline++) ;
					if (*sline == EOS)
						break;
					else
						sline++;
				} else
					sline++;
			}
			if (*fline != *sline)
				return(TRUE);
			if (*fline == EOS || *sline == EOS)
				break;
			fline++;
			sline++;
		}
	}
	if (*fline != *sline)
		return(TRUE);
	else
		return(FALSE);
}

static void smartStrcpy(char *toS, char *fromS) {
	int i, j, k;

	for (i=0,j=0; fromS[i] != EOS; i++) {
		if (fromS[i] == HIDEN_C) {
			k = i;
			for (i++; fromS[i] != HIDEN_C && fromS[i] != EOS; i++) ;
			if (fromS[i] == EOS) {
				i = k;
				toS[j++] = fromS[i];
			}
		} else
			toS[j++] = fromS[i];
	}
	toS[j] = EOS;
}

static void CompareTiers(char *isErrorsFound) {
	register int i;
	char word[1024], rightTier, isErrorFound;
	float fTotalCodes, sTotalCodes;
	struct code *p;
	struct rtiers *ft, *st;

	if (isComputeAphasia || isComputeStudentCorrectness)
		return;
	if (ffile.tiers == NULL || sfile.tiers == NULL)
		return;
	uS.remblanks(ffile.tiers->speaker);
	uS.remblanks(sfile.tiers->speaker);
	if (strcmp(ffile.tiers->speaker, sfile.tiers->speaker)) {
		*isErrorsFound = TRUE;
		fprintf(stderr, "** Different main speaker tiers found:\n");
		fprintf(stderr, "    File \"%s\": line %ld\n", ffile.fname, ffile.lineno);
		fprintf(stderr, "        %s\n", ffile.tiers->speaker);
		fprintf(stderr, "    File \"%s\": line %ld\n", sfile.fname, sfile.lineno);
		fprintf(stderr, "        %s\n", sfile.tiers->speaker);
		rely_freetree(rely_words_root);
		ffile.tiers = FreeUpTiers(ffile.tiers);
		sfile.tiers = FreeUpTiers(sfile.tiers);
		if (fpout != NULL && fpout != stderr && fpout != stdout) {
			fprintf(stderr,"\nCURRENT OUTPUT FILE \"%s\" IS INCOMPLETE.\n",OutFile);
		}
		cutt_exit(0);
	}
	if (KappaCats > 0.0) {
		for (ft=ffile.tiers; ft != NULL; ft=ft->nexttier) {
			ft->isKappaChecked = FALSE;
		}
		for (st=sfile.tiers; st != NULL; st=st->nexttier) {
			st->isKappaChecked = FALSE;
		}
	}
	if (ffile.tiers->speaker[0] == '*' && !checktier(ffile.tiers->speaker)) {
		for (st=sfile.tiers; st != NULL; st=st->nexttier) {
			uS.remblanks(st->speaker);
			for (ft=ffile.tiers; ft != NULL; ft=ft->nexttier) {
				if (ft->speaker[0] == '@' && st->speaker[0] == '@' && st->matched)
					continue;
				if (ft->matched && !isAddTiersToMasterFile)
					continue;
				uS.remblanks(ft->speaker);
				if (strcmp(ft->speaker, st->speaker) == 0) {
					ft->isKappaChecked = TRUE;
					st->isKappaChecked = TRUE;
					if (isAddTiersToMasterFile) ;
					else if (*ft->speaker == '@' && isHeaderTierSpecified && !checktier(ft->speaker)) {
						if (isMerge)
							printout(ft->speaker, ft->line, ft->attSp, ft->attLine, FALSE);
					} else if (*ft->speaker != '@' && isDepSpTierSpecified && checktier(ft->speaker)) {
						if (isMerge) {
							printout(ft->speaker, ft->line, ft->attSp, NULL, FALSE);
						}
						ffile.rootcode = freeupcodes(ffile.rootcode);
						sfile.rootcode = freeupcodes(sfile.rootcode);
					} else {
						if (isMerge)
							printout(ft->speaker, ft->line, ft->attSp, ft->attLine, FALSE);
					}
					ft->matched = TRUE;
					st->matched = TRUE;
				} else if (isAddTiersToMasterFile)
					ft->matched = TRUE;
			}
		}
	} else {
		for (st=sfile.tiers; st != NULL; st=st->nexttier) {
			uS.remblanks(st->speaker);
			for (ft=ffile.tiers; ft != NULL; ft=ft->nexttier) {
				if (ft->speaker[0] == '@' && st->speaker[0] == '@' && st->matched)
					continue;
				if (ft->matched && !isAddTiersToMasterFile)
					continue;
				uS.remblanks(ft->speaker);
				if (strcmp(ft->speaker, st->speaker) == 0) {
					ft->isKappaChecked = TRUE;
					st->isKappaChecked = TRUE;
					if (isAddTiersToMasterFile) ;
					else if (ft->speaker[0] == '*' && nomain) ;
					else if (*ft->speaker == '@' && isHeaderTierSpecified && !checktier(ft->speaker)) {
						if (isMerge)
							printout(ft->speaker, ft->line, ft->attSp, ft->attLine, FALSE);
					} else if (*ft->speaker != '@' && isDepSpTierSpecified && checktier(ft->speaker)) {
						i = 0;
						if (includeBullets)
							strcpy(uttline, st->line);
						else
							smartStrcpy(uttline, st->line);
						sTotalCodes = 0.0;
						while ((i=getword(st->speaker, uttline, word, NULL, i))) {
							if (st->speaker[0] == '%')
								sTotalCodes++;
							sfile.rootcode = AddWord(sfile.rootcode, word);
						}
						if (includeBullets)
							strcpy(uttline, ft->line);
						else
							smartStrcpy(uttline, ft->line);
						i = 0;
						fTotalCodes = 0.0;
						while ((i=getword(ft->speaker, uttline, word, NULL, i))) {
							if (ft->speaker[0] == '%')
								fTotalCodes++;
							if (!issameword(sfile.rootcode, word)) {
								ffile.rootcode = AddWord(ffile.rootcode, word);
							} else if (ft->speaker[0] == '%')
								agreedNum++;
						}
						if (ft->speaker[0] == '%') {
							if (sTotalCodes > fTotalCodes)
								totalNum += sTotalCodes;
							else
								totalNum += fTotalCodes;
						}
						isErrorFound = FALSE;
						*uttline = EOS;
						if (isMerge) {
							for (p=sfile.rootcode; p != NULL; p=p->nextcode) {
								strcat(uttline, p->name);
								if (!p->matched) {
									strcat(uttline, ":?\"");
									uS.FNType2str(uttline, strlen(uttline), sfile.fname);
									strcat(uttline, "\"");
									isErrorFound = TRUE;
								}
								strcat(uttline, " ");
							}
							for (p=ffile.rootcode; p != NULL; p=p->nextcode) {
								strcat(uttline, p->name);
								strcat(uttline, ":?\"");
								uS.FNType2str(uttline, strlen(uttline), ffile.fname);
								strcat(uttline, "\" ");
								isErrorFound = TRUE;
							}
							if (!isErrorFound)
								printout(ft->speaker, ft->line, ft->attSp, NULL, FALSE);
							else
								printout(ft->speaker, uttline, ft->attSp, NULL, FALSE);
						} else {
							for (p=sfile.rootcode; p != NULL; p=p->nextcode) {
								if (!p->matched) {
									strcat(uttline, p->name);
									if (uttline[0] != EOS)
										strcat(uttline, ", ");
									isErrorFound = TRUE;
								}
							}
							templineC4[0] = EOS;
							for (p=ffile.rootcode; p != NULL; p=p->nextcode) {
								strcat(templineC4, p->name);
								if (templineC4[0] != EOS)
									strcat(templineC4, ", ");
								isErrorFound = TRUE;
							}
							if (isErrorFound) {
								*isErrorsFound = TRUE;
								fprintf(fpout, "** Some items mismatch:\n");
								fprintf(fpout, "    File \"%s\": line %ld\n", ffile.fname, ft->lineno);
								fprintf(fpout, "        %s\t%s", ft->speaker,ft->line);
								fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, st->lineno);
								fprintf(fpout, "        %s\t%s", st->speaker,st->line);
								uS.remblanks(templineC4);
								if (templineC4[0] != EOS) {
									fprintf(fpout, "    Unmatched item(s) from file \"%s\"\n", ffile.fname);
									fprintf(fpout, "        %s\n", templineC4);
								}
								uS.remblanks(uttline);
								if (uttline[0] != EOS) {
									fprintf(fpout, "    Unmatched item(s) from file \"%s\"\n", sfile.fname);
									fprintf(fpout, "        %s\n", uttline);
								}
	//							rely_freetree(rely_words_root);
	//							ffile.tiers = FreeUpTiers(ffile.tiers);
	//							sfile.tiers = FreeUpTiers(sfile.tiers);
	//							cutt_exit(0);
							}
						}
						ffile.rootcode = freeupcodes(ffile.rootcode);
						sfile.rootcode = freeupcodes(sfile.rootcode);
					} else {
						if (ft->speaker[0] == '*' && nomain) ;
						else if (isCompTiers && smartCompareTiers(ft->line, st->line)) {
							*isErrorsFound = TRUE;
							fprintf(fpout, "** Data on tiers doesn't match:\n");
							fprintf(fpout, "    File \"%s\": line %ld\n", ffile.fname, ft->lineno);
							fprintf(fpout, "        %s\t%s", ft->speaker,ft->line);
							fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, st->lineno);
							fprintf(fpout, "        %s\t%s", st->speaker,st->line);
	//						rely_freetree(rely_words_root);
	//						ffile.tiers = FreeUpTiers(ffile.tiers);
	//						sfile.tiers = FreeUpTiers(sfile.tiers);
	//						cutt_exit(0);
						} else if (isMerge)
							printout(ft->speaker, ft->line, ft->attSp, ft->attLine, FALSE);
					}
					ft->matched = TRUE;
					st->matched = TRUE;
				} else if (isAddTiersToMasterFile)
					ft->matched = TRUE;
			}
		}
	}
	if (KappaCats > 0.0) {
		for (ft=ffile.tiers; ft != NULL; ft=ft->nexttier) {
			if (ft->speaker[0] == '%' && ft->isKappaChecked == FALSE) {
				ft->isKappaChecked = TRUE;
				if (includeBullets)
					strcpy(uttline, ft->line);
				else
					smartStrcpy(uttline, ft->line);
				i = 0;
				while ((i=getword(ft->speaker, uttline, word, NULL, i))) {
					totalNum++;
				}
			}
		}
		for (st=sfile.tiers; st != NULL; st=st->nexttier) {
			if (st->speaker[0] == '%' && st->isKappaChecked == FALSE) {
				st->isKappaChecked = TRUE;
				if (includeBullets)
					strcpy(uttline, st->line);
				else
					smartStrcpy(uttline, st->line);
				i = 0;
				while ((i=getword(st->speaker, uttline, word, NULL, i))) {
					totalNum++;
				}
			}
		}
	}	
	if (checktier(ffile.tiers->speaker)) {
		for (ft=ffile.tiers; ft != NULL; ft=ft->nexttier) {
			if (isAddTiersToMasterFile) {
				if (ft->speaker[0] == '@')
					break;
				if (ft->matched)
					printout(ft->speaker, ft->line, ft->attSp, ft->attLine, FALSE);
			}
			if (!ft->matched && (isCompTiers || checktier(ft->speaker))) {
				*isErrorsFound = TRUE;
				fprintf(fpout, "** Unmatched tiers found around lines:\n");
				fprintf(fpout, "    File \"%s\": line %ld\n", ffile.fname, ffile.lineno);
				fprintf(fpout, "        %s\t%s", ft->speaker, ft->line);
				fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, sfile.lineno);
//				rely_freetree(rely_words_root);
//				ffile.tiers = FreeUpTiers(ffile.tiers);
//				sfile.tiers = FreeUpTiers(sfile.tiers);
//				cutt_exit(0);
			}
		}
	}
	if (checktier(sfile.tiers->speaker)) {
		for (st=sfile.tiers; st != NULL; st=st->nexttier) {
			rightTier = (checktier(st->speaker) && st->speaker[0] == '%');
			if (isAddTiersToMasterFile && !st->matched && rightTier) {
				printout(st->speaker, st->line, st->attSp, st->attLine, FALSE);
			} else if ((!st->matched && (isCompTiers || checktier(st->speaker))) ||
					   (isAddTiersToMasterFile && st->matched && rightTier)) {
				*isErrorsFound = TRUE;
				if (isAddTiersToMasterFile && st->matched)
					fprintf(fpout, "** Duplicate tier found around lines:\n");
				else
					fprintf(fpout, "** Unmatched tiers found around lines:\n");
				fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, sfile.lineno);
				fprintf(fpout, "        %s\t%s", st->speaker, st->line);
				fprintf(fpout, "    File \"%s\": line %ld\n", ffile.fname, ffile.lineno);
//				rely_freetree(rely_words_root);
//				ffile.tiers = FreeUpTiers(ffile.tiers);
//				sfile.tiers = FreeUpTiers(sfile.tiers);
//				cutt_exit(0);
			}
		}
	}
	if (isAddTiersToMasterFile) {
		for (; ft != NULL; ft=ft->nexttier) {
			printout(ft->speaker, ft->line, ft->attSp, ft->attLine, FALSE);
		}
	}
}

static char isRightWord(char *word) {
	if (uS.mStrnicmp(word,"yyy",3)==0   || uS.mStrnicmp(word,"www",3)==0   ||
		uS.mStrnicmp(word,"yyy@s",5)==0 || uS.mStrnicmp(word,"www@s",5)==0 ||
		word[0] == '+' || word[0] == '-' || word[0] == '!' || word[0] == '?' || word[0] == '.')
		return(FALSE);
	return(TRUE);
}

static void totalNumMatchedWords(struct rely_tnode *p, int *mCnt, int *fCnt, int *sCnt) {
	if (p != NULL) {
		totalNumMatchedWords(p->left, mCnt, fCnt, sCnt);
		totalNumMatchedWords(p->right, mCnt, fCnt, sCnt);
		*fCnt += p->fFileCnt;
		*sCnt += p->sFileCnt;
		if (p->fFileCnt <= p->sFileCnt)
			*mCnt += p->fFileCnt;
		else
			*mCnt += p->sFileCnt;
	}
}

static void createTokensList(char *s) {
	int i;

	s[0] = EOS;
	for (i=0; i < targc; i++) {
		if (targv[i][0] == '+' && targv[i][1] == 's') {
			if (s[0] != EOS)
				strcat(s, ", ");
			strcat(s, targv[i]+2);
		}
	}
}

static void rely_pr_result(FILE *fpo) {
	int fCnt, sCnt, mCnt;
	float kappa, tf, ts;

	if (KappaCats > 0.0) {
		kappa = ((agreedNum / totalNum) - (1 / KappaCats)) / (1 - (1 / KappaCats));
		fprintf(fpo, "\nCohen's kappa coefficient: %.4f\n", kappa);
	}
	if (isComputeStudentCorrectness) {
		if (isComputeStudentCorrectness == 1) {
			tf = fTotalUtts;
			ts = sTotalUtts;
			fprintf(fpo, "\ncontrol \"%s\": # utterances examined: %d\n", FileName1, fTotalUtts);
			fprintf(fpo, "student \"%s:\": # utterances examined: %d\n", FileName2, sTotalUtts);
		} else {
			tf = sTotalUtts;
			ts = fTotalUtts;
			fprintf(fpo, "\ncontrol \"%s\": # utterances examined: %d\n", FileName2, sTotalUtts);
			fprintf(fpo, "student \"%s:\": # utterances examined: %d\n", FileName1, fTotalUtts);
		}
		if (tf != 0.0) {
			kappa = (ts / tf) * 100;
		} else
			kappa = 0.0;
		fprintf(fpo, "%% utterances examined with matching items: %.4f\n", kappa);

		if (isComputeStudentCorrectness == 1) {
			fprintf(fpo, "\ncontrol \"%s\": # items examined: %ld\n", FileName1, controlItemCnt);
			fprintf(fpo, "student \"%s\": # items examined: %ld\n", FileName2, studenItemCnt);
		} else {
			fprintf(fpo, "\ncontrol \"%s\": # items examined: %ld\n", FileName2, controlItemCnt);
			fprintf(fpo, "student \"%s\": # items examined: %ld\n", FileName1, studenItemCnt);
		}
		fprintf(fpo, "# control items matched by student: %ld\n", studentItemsMatched);
		fprintf(fpo, "# control items missed by student: %ld\n", controlItemCnt - studentItemsMatched);
		tf = controlItemCnt;
		ts = studentItemsMatched;
		if (tf != 0.0) {
			kappa = (ts / tf) * 100;
		} else
			kappa = 0.0;
		fprintf(fpo, "%% precision: %.4f\n", kappa);

		fprintf(fpo, "\n# student items found in control: %ld\n", studentItemsMatched);
		fprintf(fpo, "# student items not found in control: %ld\n", studentItemsNotInMaster);
		tf = studenItemCnt;
		ts = studentItemsMatched;
		if (tf != 0.0) {
			kappa = (ts / tf) * 100;
		} else
			kappa = 0.0;
		fprintf(fpo, "%% accuracy: %.4f\n", kappa);

		fTotalUtts = 0;
		sTotalUtts = 0;
	} else if (isComputeAphasia) {
		fCnt = fTotalUtts;
		sCnt = sTotalUtts;
		if (fCnt > sCnt) {
			if (fCnt != 0) {
				tf = fCnt;
				ts = sCnt;
				kappa = (ts / tf) * 100;
			} else
				kappa = 0.0;
		} else {
			if (sCnt != 0) {
				tf = fCnt;
				ts = sCnt;
				kappa = (tf / ts) * 100;
			} else
				kappa = 0.0;
		}
		if (!isSOption) {
			fprintf(fpo, "\n%s: # utterances examined: %d\n", FileName1, fCnt);
			fprintf(fpo, "%s: # utterances examined: %d\n", FileName2, sCnt);
			fprintf(fpo, "%% of all utterances examined with matching codes %%: %.4f\n", kappa);
		}
		fCnt = 0;
		sCnt = 0;
		mCnt = 0;
		totalNumMatchedWords(rely_words_root, &mCnt, &fCnt, &sCnt);
		if (fCnt > sCnt) {
			if (fCnt != 0) {
				tf = fCnt;
				ts = mCnt;
				kappa = (ts / tf) * 100;
			} else
				kappa = 0.0;
		} else {
			if (sCnt != 0) {
				tf = sCnt;
				ts = mCnt;
				kappa = (ts / tf) * 100;
			} else
				kappa = 0.0;
		}
		createTokensList(templineC3);
		if (!isSOption) {
			fprintf(fpo, "\n%s: # words examined: %d\n", FileName1, fCnt);
			fprintf(fpo, "%s: # words examined: %d\n", FileName2, sCnt);
			fprintf(fpo, "# words (tokens, codes) matched: %d\n", mCnt);
			fprintf(fpo, "%% of all words examined with matching codes %%: %.4f\n", kappa);
		} else {
			fprintf(fpo, "\n%s: total %s: %d\n", FileName1, templineC3, fCnt);
			fprintf(fpo, "%s: total %s: %d\n", FileName2, templineC3, sCnt);
			fprintf(fpo, "total %s matched: %d\n", templineC3, mCnt);
			fprintf(fpo, "total %s %%: %.4f\n", templineC3, kappa);
		}
		rely_freetree(rely_words_root);
		rely_words_root = NULL;
		fTotalUtts = 0;
		sTotalUtts = 0;
	}
}

void call(void) {
	int  i;
	char cc;
	char isErrorsFound, isRightSpeaker;
	char word[1024], tword[1024];
	struct rtiers *ft, *st;

	if (FirstFile) {
		strcpy(ffile.fname, oldfname);
		ffile.lineno = lineno;
		ffile.tlineno = tlineno;
		ffile.tiers = NULL;
		ffile.rootcode = NULL;
		strcpy(OutFile, newfname);
		fprintf(stderr,"From file <%s>\n",oldfname);
		FirstFile = !FirstFile;
		return;
	} else {
		fprintf(stderr,"From file <%s> \n",oldfname);
#ifndef UNX
		if (WD_Not_Eq_OD)
			SetNewVol(wd_dir);
#endif
		if ((ffile.fpin=fopen(ffile.fname, "r")) == NULL) {
			fprintf(stderr, "Can't open file \"%s\".\n", ffile.fname);
			cutt_exit(0);
		}
		strcpy(FileName1, ffile.fname);
#ifndef UNX
		if (WD_Not_Eq_OD)
			SetNewVol(od_dir);
#endif
		strcpy(sfile.fname, oldfname);
		strcpy(FileName2, sfile.fname);
		sfile.fpin = fpin;
		if (!stout) {
			fclose(fpout);
			unlink(newfname);
			if ((fpout=fopen(OutFile, "w")) == NULL) {
				fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", OutFile);
				cutt_exit(0);
			}
#if defined(_MAC_CODE)
			settyp(OutFile, 'TEXT', the_file_creator.out, FALSE);
#endif
			if (FirstArgsPrint && !isMerge) {
				fprintf(fpout, "> ");
				printArg(targv, targc, fpout, FALSE, oldfname);
				fprintf(fpout, "\n");
			}
			FirstArgsPrint = FALSE;
		}
		sfile.lineno = lineno;
		sfile.tlineno = tlineno;
		sfile.tiers = NULL;
		sfile.rootcode = NULL;
		controlItemCnt = 0L;
		studenItemCnt = 0L;
		studentItemsMatched = 0L;
		studentItemsNotInMaster = 0L;
	}
	isErrorsFound = FALSE;
	ffile.currentatt = 0;
	ffile.currentchar = (char)getc_cr(ffile.fpin, &ffile.currentatt);
	sfile.currentatt = 0;
	sfile.currentchar = (char)getc_cr(sfile.fpin, &sfile.currentatt);
	if (isAddTiersToMasterFile) {
		if (ffile.currentchar == '*' && sfile.currentchar == '@') {
			fprintf(stderr, "Master file %s is missing header tier at the beginning,\n", ffile.fname);
			fprintf(stderr, "but there are header tier in the second file %s.\n", sfile.fname);
			if (fpout != NULL && fpout != stderr && fpout != stdout) {
				fprintf(stderr,"\nCURRENT OUTPUT FILE \"%s\" IS INCOMPLETE.\n",OutFile);
			}
			cutt_exit(0);
		}
		if (ffile.currentchar == '@' && sfile.currentchar == '*') {
			fpin = ffile.fpin;
			currentchar = ffile.currentchar;
			currentatt = ffile.currentatt;
			lineno  = ffile.lineno;
			tlineno = ffile.tlineno;
			do {
				if ((ffile.res = getwholeutter())) {
					ffile.fpin = fpin;
					ffile.currentchar = currentchar;
					ffile.currentatt = currentatt;
					ffile.lineno = lineno;
					ffile.tlineno = tlineno;
					ffile.tiers = NULL;
					fputs(utterance->speaker, fpout);
					fputs(uttline, fpout);
				}
			} while (ffile.res && currentchar != '*') ;
		}
	}
	do {
//		if (ffile.currentchar == '*' && sfile.currentchar == '@' && isAddTiersToMasterFile) ;
//		else {
			fpin = ffile.fpin;
			currentchar = ffile.currentchar;
			currentatt = ffile.currentatt;
			lineno  = ffile.lineno;
			tlineno = ffile.tlineno;
			cc = currentchar;
			do {
				if ((ffile.res = getwholeutter())) {
					ffile.fpin = fpin;
					ffile.currentchar = currentchar;
					ffile.currentatt = currentatt;
					ffile.lineno = lineno;
					ffile.tlineno = tlineno;
					if (utterance->speaker[0] == '@' && !checktier(utterance->speaker))
						continue;
					else if (uS.isInvisibleHeader(utterance->speaker) || uS.isUTF8(utterance->speaker))
						continue;
					ffile.tiers = AddTiers(ffile.tiers, utterance->speaker, utterance->attSp, uttline, utterance->attLine, lineno);
				}
			} while (/*isAddTiersToMasterFile && */ffile.res && currentchar != '*' && (currentchar != '@' || cc == '@')) ;
//		}

//		if (sfile.currentchar == '*' && ffile.currentchar == '@' && isAddTiersToMasterFile) ;
//		else {
			fpin = sfile.fpin;
			currentchar = sfile.currentchar;
			currentatt = sfile.currentatt;
			lineno  = sfile.lineno;
			tlineno = sfile.tlineno;
			cc = currentchar;
			do {
				if ((sfile.res = getwholeutter())) {
					sfile.fpin = fpin;
					sfile.currentchar = currentchar;
					sfile.currentatt = currentatt;
					sfile.lineno = lineno;
					sfile.tlineno = tlineno;
					if (utterance->speaker[0] == '@' && !checktier(utterance->speaker))
						continue;
					else if (uS.isInvisibleHeader(utterance->speaker) || uS.isUTF8(utterance->speaker))
						continue;
					sfile.tiers = AddTiers(sfile.tiers, utterance->speaker, utterance->attSp, uttline, utterance->attLine, lineno);
				}
			} while (/*isAddTiersToMasterFile && */sfile.res && currentchar != '*' && (currentchar != '@' || cc == '@')) ;
//		}
		if (isComputeStudentCorrectness) {
			struct code *fc, *sc;

			if (ffile.tiers != NULL && sfile.tiers != NULL && ffile.tiers->speaker[0] != sfile.tiers->speaker[0]) {
				uS.remblanks(ffile.tiers->speaker);
				uS.remblanks(sfile.tiers->speaker);
				if (strcmp(ffile.tiers->speaker, sfile.tiers->speaker) != 0) {
					fprintf(stderr, "Files \"%s\" and \"%s\" do not have the same speakers in the same place.\n", ffile.fname, sfile.fname);
					fprintf(stderr, "    File \"%s\": line %ld\n", ffile.fname, ffile.lineno);
					if (ffile.tiers != NULL)
						fprintf(stderr, "        %s\t%s", ffile.tiers->speaker, ffile.tiers->line);
					fprintf(stderr, "    File \"%s\": line %ld\n", sfile.fname, sfile.lineno);
					if (sfile.tiers != NULL)
						fprintf(stderr, "        %s\t%s", sfile.tiers->speaker, sfile.tiers->line);
					rely_freetree(rely_words_root);
					ffile.tiers = FreeUpTiers(ffile.tiers);
					sfile.tiers = FreeUpTiers(sfile.tiers);
					if (fpout != NULL && fpout != stderr && fpout != stdout) {
						fprintf(stderr,"\nCURRENT OUTPUT FILE \"%s\" IS INCOMPLETE.\n",OutFile);
					}
					cutt_exit(0);
				}
			}
			if (isComputeStudentCorrectness == 1 && ffile.tiers != NULL) {
				for (ft=ffile.tiers; ft != NULL; ft=ft->nexttier) {
					if (checktier(ft->speaker)) {
						if (ft->speaker[0] == '*' && !nomain)
							fTotalUtts++;
						if ((ft->speaker[0] == '*' && !isDepTierSpecified) || (ft->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								fTotalUtts++;
						}
						uS.remblanks(ft->speaker);
						if (ft->speaker[0] == '%') {
							smartStrcpy(uttline, ft->line);
							i = 0;
							while ((i=getword(ft->speaker, uttline, word, NULL, i))) {
								controlItemCnt++;
								ffile.rootcode = AddWord(ffile.rootcode, word);
							}
						}
					}
				}

				for (st=sfile.tiers; st != NULL; st=st->nexttier) {
					if (checktier(st->speaker)) {
						if (st->speaker[0] == '*' && !nomain)
							sTotalUtts++;
						if ((st->speaker[0] == '*' && !isDepTierSpecified) || (st->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								sTotalUtts++;
						}
						uS.remblanks(st->speaker);
						if (st->speaker[0] == '%') {
							smartStrcpy(uttline, st->line);
							i = 0;
							while ((i=getword(st->speaker, uttline, word, NULL, i))) {
								studenItemCnt++;
								sfile.rootcode = AddWord(sfile.rootcode, word);
							}
						}
					}
				}

				for (fc=ffile.rootcode; fc != NULL; fc=fc->nextcode) {
					for (sc=sfile.rootcode; sc != NULL; sc=sc->nextcode) {
						if (strcmp(fc->name, sc->name) == 0 && !fc->matched && !sc->matched) {
							studentItemsMatched++;
							fc->matched = TRUE;
							sc->matched = TRUE;
						}
					}
				}
				for (sc=sfile.rootcode; sc != NULL; sc=sc->nextcode) {
					if (!sc->matched) {
						studentItemsNotInMaster++;
					}
				}
				ffile.rootcode = freeupcodes(ffile.rootcode);
				sfile.rootcode = freeupcodes(sfile.rootcode);
			} else if (sfile.tiers != NULL) {
				for (ft=ffile.tiers; ft != NULL; ft=ft->nexttier) {
					if (checktier(ft->speaker)) {
						if (ft->speaker[0] == '*' && !nomain)
							fTotalUtts++;
						if ((ft->speaker[0] == '*' && !isDepTierSpecified) || (ft->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								fTotalUtts++;
							uS.remblanks(ft->speaker);
							if (ft->speaker[0] == '%') {
								smartStrcpy(uttline, ft->line);
								i = 0;
								while ((i=getword(ft->speaker, uttline, word, NULL, i))) {
									studenItemCnt++;
									ffile.rootcode = AddWord(ffile.rootcode, word);
								}
							}
						}
					}
				}
				for (st=sfile.tiers; st != NULL; st=st->nexttier) {
					if (checktier(st->speaker)) {
						if (st->speaker[0] == '*' && !nomain)
							sTotalUtts++;
						if ((st->speaker[0] == '*' && !isDepTierSpecified) || (st->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								sTotalUtts++;
						}
						uS.remblanks(st->speaker);
						if (st->speaker[0] == '%') {
							smartStrcpy(uttline, st->line);
							i = 0;
							while ((i=getword(st->speaker, uttline, word, NULL, i))) {
								controlItemCnt++;
								sfile.rootcode = AddWord(sfile.rootcode, word);
							}
						}
					}
				}
				for (sc=ffile.rootcode; sc != NULL; sc=sc->nextcode) {
					for (fc=sfile.rootcode; fc != NULL; fc=fc->nextcode) {
						if (strcmp(fc->name, sc->name) == 0 && !fc->matched && !sc->matched) {
							studentItemsMatched++;
							fc->matched = TRUE;
							sc->matched = TRUE;
						}
					}
				}
				for (fc=ffile.rootcode; fc != NULL; fc=fc->nextcode) {
					if (!fc->matched) {
						studentItemsNotInMaster++;
					}
				}
				ffile.rootcode = freeupcodes(ffile.rootcode);
				sfile.rootcode = freeupcodes(sfile.rootcode);
			}
		} else if (isComputeAphasia) {
			if (ffile.tiers != NULL && ffile.tiers->speaker[0] == '*' && checktier(ffile.tiers->speaker)) {
				for (ft=ffile.tiers; ft != NULL; ft=ft->nexttier) {
					if (checktier(ft->speaker)) {
						if (ft->speaker[0] == '*' && !nomain)
							fTotalUtts++;
						if ((ft->speaker[0] == '*' && !isDepTierSpecified) || (ft->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								fTotalUtts++;
							i = 0;
							while ((i=getword(ft->speaker, ft->line, word, NULL, i))) {
								uS.remblanks(word);
								strcpy(tword, word);
								if (exclude(word) && isRightWord(word)) {
									uS.remblanks(word);
									if (strcmp(word, tword))
										rely_words_root = rely_tree(rely_words_root, word, tword, TRUE);
									else
										rely_words_root = rely_tree(rely_words_root, word, NULL, TRUE);
								}
							}
						}
					}
				}
			}
			if (sfile.tiers != NULL && sfile.tiers->speaker[0] == '*' && checktier(sfile.tiers->speaker)) {
				for (st=sfile.tiers; st != NULL; st=st->nexttier) {
					if (checktier(st->speaker)) {
						if (st->speaker[0] == '*' && !nomain)
							sTotalUtts++;
						if ((st->speaker[0] == '*' && !isDepTierSpecified) || (st->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								sTotalUtts++;
							i = 0;
							while ((i=getword(st->speaker, st->line, word, NULL, i))) {
								uS.remblanks(word);
								strcpy(tword, word);
								if (exclude(word) && isRightWord(word)) {
									uS.remblanks(word);
									if (strcmp(word, tword))
										rely_words_root = rely_tree(rely_words_root, word, tword, FALSE);
									else
										rely_words_root = rely_tree(rely_words_root, word, NULL, FALSE);
								}
							}
						}
					}
				}
			}
		}
		if (ffile.res && !sfile.res && !isAddTiersToMasterFile) {
			if (isComputeStudentCorrectness) {
				fpin = ffile.fpin;
				currentchar = ffile.currentchar;
				currentatt = ffile.currentatt;
				lineno  = ffile.lineno;
				tlineno = ffile.tlineno;
				isRightSpeaker = FALSE;
				while (getwholeutter()) {
					if (utterance->speaker[0] == '*') {
						if (checktier(utterance->speaker))
							isRightSpeaker = TRUE;
						else
							isRightSpeaker = FALSE;
					}
					if (isRightSpeaker && checktier(utterance->speaker)) {
						if (utterance->speaker[0] == '*' && !nomain)
							fTotalUtts++;
						if ((utterance->speaker[0] == '*' && !isDepTierSpecified) || (utterance->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								fTotalUtts++;
							i = 0;
							while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
								uS.remblanks(word);
								if (exclude(word) && isRightWord(word)) {
									uS.remblanks(word);
									if (isComputeStudentCorrectness == 1) {
										controlItemCnt++;
									} else {
										studenItemCnt++;
										studentItemsNotInMaster++;
									}
								}
							}
						}
					}
				}
			} else if (isComputeAphasia) {
				fpin = ffile.fpin;
				currentchar = ffile.currentchar;
				currentatt = ffile.currentatt;
				lineno  = ffile.lineno;
				tlineno = ffile.tlineno;
				isRightSpeaker = FALSE;
				while (getwholeutter()) {
					if (utterance->speaker[0] == '*') {
						if (checktier(utterance->speaker))
							isRightSpeaker = TRUE;
						else
							isRightSpeaker = FALSE;
					}
					if (isRightSpeaker && checktier(utterance->speaker)) {
						if (utterance->speaker[0] == '*' && !nomain)
							fTotalUtts++;
						if ((utterance->speaker[0] == '*' && !isDepTierSpecified) || (utterance->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								fTotalUtts++;
							i = 0;
							while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
								uS.remblanks(word);
								strcpy(tword, word);
								if (exclude(word) && isRightWord(word)) {
									uS.remblanks(word);
									if (strcmp(word, tword))
										rely_words_root = rely_tree(rely_words_root, word, tword, TRUE);
									else
										rely_words_root = rely_tree(rely_words_root, word, NULL, TRUE);
								}
							}
						}
					}
				}
			} else {
				fprintf(stderr, "File \"%s\" ended before file \"%s\" did.\n",  sfile.fname, ffile.fname);
				rely_freetree(rely_words_root);
				ffile.tiers = FreeUpTiers(ffile.tiers);
				sfile.tiers = FreeUpTiers(sfile.tiers);
				if (fpout != NULL && fpout != stderr && fpout != stdout) {
					fprintf(stderr,"\nCURRENT OUTPUT FILE \"%s\" IS INCOMPLETE.\n",OutFile);
				}
				cutt_exit(0);
			}
		}
		if (!ffile.res && sfile.res) {
			if (isComputeStudentCorrectness) {
				fpin = sfile.fpin;
				currentchar = sfile.currentchar;
				currentatt = sfile.currentatt;
				lineno  = sfile.lineno;
				tlineno = sfile.tlineno;
				isRightSpeaker = FALSE;
				while (getwholeutter()) {
					if (utterance->speaker[0] == '*') {
						if (checktier(utterance->speaker))
							isRightSpeaker = TRUE;
						else
							isRightSpeaker = FALSE;
					}
					if (isRightSpeaker && checktier(utterance->speaker)) {
						if (utterance->speaker[0] == '*' && !nomain)
							sTotalUtts++;
						if ((utterance->speaker[0] == '*' && !isDepTierSpecified) || (utterance->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								sTotalUtts++;
							i = 0;
							while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
								uS.remblanks(word);
								strcpy(tword, word);
								if (exclude(word) && isRightWord(word)) {
									uS.remblanks(word);
									if (isComputeStudentCorrectness == 1) {
										studenItemCnt++;
										studentItemsNotInMaster++;
									} else {
										controlItemCnt++;
									}
								}
							}
						}
					}
				}
			} else if (isComputeAphasia) {
				fpin = sfile.fpin;
				currentchar = sfile.currentchar;
				currentatt = sfile.currentatt;
				lineno  = sfile.lineno;
				tlineno = sfile.tlineno;
				isRightSpeaker = FALSE;
				while (getwholeutter()) {
					if (utterance->speaker[0] == '*') {
						if (checktier(utterance->speaker))
							isRightSpeaker = TRUE;
						else
							isRightSpeaker = FALSE;
					}
					if (isRightSpeaker && checktier(utterance->speaker)) {
						if (utterance->speaker[0] == '*' && !nomain)
							sTotalUtts++;
						if ((utterance->speaker[0] == '*' && !isDepTierSpecified) || (utterance->speaker[0] == '%' && isDepTierSpecified)) {
							if (nomain)
								sTotalUtts++;
							i = 0;
							while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
								uS.remblanks(word);
								strcpy(tword, word);
								if (exclude(word) && isRightWord(word)) {
									uS.remblanks(word);
									if (strcmp(word, tword))
										rely_words_root = rely_tree(rely_words_root, word, tword, TRUE);
									else
										rely_words_root = rely_tree(rely_words_root, word, NULL, TRUE);
								}
							}
						}
					}
				}
			} else {
				fprintf(stderr, "File \"%s\" ended before file \"%s\" did.\n", ffile.fname, sfile.fname);
				rely_freetree(rely_words_root);
				ffile.tiers = FreeUpTiers(ffile.tiers);
				sfile.tiers = FreeUpTiers(sfile.tiers);
				if (fpout != NULL && fpout != stderr && fpout != stdout) {
					fprintf(stderr,"\nCURRENT OUTPUT FILE \"%s\" IS INCOMPLETE.\n",OutFile);
				}
				cutt_exit(0);
			}
		}
		if (!ffile.res && !sfile.res) {
			CompareTiers(&isErrorsFound);
			ffile.tiers = FreeUpTiers(ffile.tiers);
			sfile.tiers = FreeUpTiers(sfile.tiers);
			break;
		}
		if (!isAddTiersToMasterFile) {
			ft = ffile.tiers;
			st = sfile.tiers;
			while (ft != NULL && st != NULL) {
				uS.remblanks(ft->speaker);
				uS.remblanks(st->speaker);
				if (strcmp(ffile.tiers->speaker, sfile.tiers->speaker)) {
					isErrorsFound = TRUE;
					fprintf(fpout, "** Tier names do not match:\n");
					fprintf(fpout, "    File \"%s\": line %ld\n", ffile.fname, ft->lineno);
					if (ffile.tiers != NULL)
						fprintf(fpout, "        %s\n", ft->speaker);
					fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, st->lineno);
					if (sfile.tiers != NULL)
						fprintf(fpout, "        %s\n", st->speaker);
//					rely_freetree(rely_words_root);
//					ffile.tiers = FreeUpTiers(ffile.tiers);
//					sfile.tiers = FreeUpTiers(sfile.tiers);
//					cutt_exit(0);
				}
				ft = ft->nexttier;
				if (!isCompTiers) {
					while (ft != NULL && ft->speaker[0] == '%' && !checktier(ft->speaker))
						ft = ft->nexttier;
				}
				st = st->nexttier;
				if (!isCompTiers) {
					while (st != NULL && st->speaker[0] == '%' && !checktier(st->speaker)) 
						st = st->nexttier;
				}
			}
			if (ft != st && ffile.tiers == NULL) {
				if (isCompTiers) {
					isErrorsFound = TRUE;
					fprintf(fpout, "** Inconsistent number of dependent tiers found at speaker tier:\n");
					fprintf(fpout, "    File \"%s\": line %ld\n", ffile.fname, ffile.lineno);
					if (sfile.tiers != NULL) {
						fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, sfile.tiers->lineno);
						fprintf(fpout, "        %s\n", sfile.tiers->speaker);
					} else
						fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, sfile.lineno);
				}
			} else if (ft != st && (ffile.tiers->speaker[0] != '@' || (!isComputeAphasia && !isComputeStudentCorrectness))) {
				if (isCompTiers && checktier(ffile.tiers->speaker)) {
					isErrorsFound = TRUE;
					if (ffile.tiers->speaker[0] == '@') {
						fprintf(fpout, "** Inconsistent number of header tiers found (perhaps some hidden tiers matched)\n");
						fprintf(fpout, "   Starting at header tiers:\n");
					} else
						fprintf(fpout, "** Inconsistent number of dependent tiers found at speaker tier:\n");
					if (ffile.tiers != NULL) {
						fprintf(fpout, "    File \"%s\": line %ld\n", ffile.fname, ffile.tiers->lineno);
						fprintf(fpout, "        %s\n", ffile.tiers->speaker);
					} else
						fprintf(fpout, "    File \"%s\": line %ld\n", ffile.fname, ffile.lineno);
					if (sfile.tiers != NULL) {
						fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, sfile.tiers->lineno);
						fprintf(fpout, "        %s\n", sfile.tiers->speaker);
					} else
						fprintf(fpout, "    File \"%s\": line %ld\n", sfile.fname, sfile.lineno);
//					rely_freetree(rely_words_root);
//					ffile.tiers = FreeUpTiers(ffile.tiers);
//					sfile.tiers = FreeUpTiers(sfile.tiers);
//					cutt_exit(0);
				}
			}
		}
		if (ffile.currentchar == '*' || ffile.currentchar == '@' || ffile.currentchar == EOF) {
			CompareTiers(&isErrorsFound);
			ffile.tiers = FreeUpTiers(ffile.tiers);
			sfile.tiers = FreeUpTiers(sfile.tiers);
		}
		if ((!ffile.res || !sfile.res) && isAddTiersToMasterFile)
			break;
	} while (1) ;
	fclose(ffile.fpin);
	fpin = sfile.fpin;
	FirstFile = !FirstFile;
	if (!stout) {
		if (isMerge) {
			fprintf(stderr, "*** Merged data is in:\n");
			fprintf(stderr,"    File \"%s\"\n", OutFile);
		} else if (isErrorsFound) {
			fprintf(stderr, "*** Coder disagreements are reported in:\n");
			fprintf(stderr,"    File \"%s\"\n", OutFile);
		}
		fprintf(stderr,"Output file <%s>\n", OutFile);
	}
	if (!isCombineForAllFiles) {
		rely_pr_result(fpout);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	targv = argv;
	targc = argc;
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = RELY;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	FirstFile = TRUE;
	FirstArgsPrint = TRUE;
	FileName1[0] = EOS;
	FileName2[0] = EOS;
	bmain(argc,argv,NULL);
	if (!FirstFile) {
		fprintf(stderr, "The second file in pair was missing.\n");
	}
	if (isCombineForAllFiles) {
		rely_pr_result(stdout);
	}
}
