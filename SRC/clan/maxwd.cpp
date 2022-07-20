/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1
#define LNGBUF (BUFSIZ * 2)

#include "cu.h"

#if !defined(UNX)
#define _main maxwd_main
#define call maxwd_call
#define getflag maxwd_getflag
#define init maxwd_init
#define usage maxwd_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define OBJS struct mobj
OBJS {
	char *speaker;
	char *utte;
	char isDepsSpName;
	char *deps;
	FNType *fn;
	long lineno;
	int  size;
	OBJS *cont;
	OBJS *nextobj;
	OBJS *prevobj;
} *maxwd_head;

extern char maxwd_which;

static char maxwd_word[512];
static char uniq_length, ftime, maxwd_isMorExcluded;
static int  mxd_pos;
static int  bo;
static long TotalCnt = -1L;

void usage() {
	puts("MAXWD searches for the longest word in a file or set of files.");
	printf("Usage: maxwd [a bS cN gN xN %s] filename(s)\n",mainflgs());
	puts("+a : consider ONLY unique length utterances/words");
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list (-b: empty morphemes list)");
	puts("+cN: display N longest utterances/words");
	puts("+gN: look for longest utterance instead of longest word");
	puts("     1 - number of morph; 2 - number of word; 3 - number of chars");
	mainusage(TRUE);
}

static void makering(int w) {
	OBJS *temp;

	if ((maxwd_head=NEW(OBJS)) == NULL)
		out_of_mem();
	maxwd_head->speaker = NULL;
	maxwd_head->utte = NULL;
	maxwd_head->isDepsSpName = FALSE;
	maxwd_head->deps = NULL;
	maxwd_head->fn = NULL;
	maxwd_head->lineno = 0;
	maxwd_head->size = 0;
	maxwd_head->cont = NULL;
	maxwd_head->nextobj = maxwd_head;
	maxwd_head->prevobj = maxwd_head;
	for (; w > 1; w--) {
		if ((temp=NEW(OBJS)) == NULL)
			out_of_mem();
		temp->speaker = NULL;
		temp->utte = NULL;
		temp->isDepsSpName = FALSE;
		temp->deps = NULL;
		temp->fn = NULL;
		temp->lineno = 0;
		temp->size = 0;
		temp->cont = NULL;
		temp->nextobj = maxwd_head->nextobj;
		temp->prevobj = maxwd_head;
		maxwd_head->nextobj->prevobj = temp;
		maxwd_head->nextobj = temp;
	}
}

static void clearcont(OBJS *tt) {
	OBJS *tt1;

	while (tt != NULL) {
		tt1 = tt->cont;
		if (tt->speaker != NULL) free(tt->speaker);
		if (tt->utte != NULL) free(tt->utte);
		if (tt->deps != NULL) free(tt->deps);
		if (tt->fn != NULL) free(tt->fn);
		free(tt);
		tt = tt1;
	}
}

static void cleanupring() {
	OBJS *temp;

	temp = maxwd_head;
	do {
		clearcont(temp->cont);
		temp->cont = NULL;
		if (temp->speaker!= NULL) {free(temp->speaker); temp->speaker= NULL;}
		if (temp->utte != NULL) { free(temp->utte); temp->utte = NULL; }
		if (temp->deps != NULL) { free(temp->deps); temp->deps = NULL; }
		if (temp->fn != NULL) { free(temp->fn);		temp->fn = NULL; }
		temp->lineno = 0;
		temp->size = 0;
		temp = temp->nextobj;
	} while (temp != maxwd_head) ;
}

static void maxwd_cleanup(OBJS *p) {
	OBJS *t;

	if (p != NULL) {
		t = p->nextobj;
		p->nextobj = NULL;
		p = t;
		while (p != NULL) {
			t = p;
			p = p->nextobj;
			if (t->speaker != NULL) free(t->speaker);
			if (t->utte != NULL) free(t->utte);
			if (t->deps != NULL) free(t->deps);
			if (t->fn != NULL) free(t->fn);
			clearcont(t->cont);
			free(t);
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	long j;
	char *morf, *t;

	f++;
	switch(*f++) {
		case 'a':
			uniq_length = TRUE;
			no_arg_option(f);
			break;
		case 'b':
				morf = getfarg(f,f1,i);
				if (*(f-2) == '-') {
					if (*morf != EOS) {
						for (j=0L; rootmorf[j] != EOS; ) {
							if (uS.isCharInMorf(rootmorf[j],morf)) 
								strcpy(rootmorf+j,rootmorf+j+1);
							else
								j++;
						}
					} else
						rootmorf[0] = EOS;
				} else {
					if (*morf != EOS) {
						t = rootmorf;
						rootmorf = (char *)malloc(strlen(t)+strlen(morf)+1);
						if (rootmorf == NULL) {
							fprintf(stderr,"No more space left in core.\n");
							cutt_exit(1);
						}
						strcpy(rootmorf,t);
						strcat(rootmorf,morf);
						free(t);
					}
				}
				break;
		case 'g':
				if ((maxwd_which=(char)atoi(getfarg(f,f1,i))) < 1 || maxwd_which > 3) {
					fprintf(stderr, "Argument to option %s should be between 1 and 3\n", (f-2));
					cutt_exit(0);
				}
				if (filterUttLen != 0L && CntFUttLen == 0) {
					if (maxwd_which == 1)
						CntFUttLen = 2;
					else if (maxwd_which == 2)
						CntFUttLen = 1;
					else if (maxwd_which == 3)
						CntFUttLen = 3;
				}
				break;
		case 'c':
				j = atol(getfarg(f,f1,i));
				if (j < 1L) {
					fprintf(stderr, "Argument to option %s should be greater than 0\n", (f-2));
					cutt_exit(0);
				}
				TotalCnt = j;
				makering(j);
				break;
		case 't':
			if (*(f-2) == '-' && *f == '%') {
				if (!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:")) {
					maxwd_isMorExcluded = TRUE;
				}
			} else
				maingetflag(f-2,f1,i);
			break;
		default:
				if (*(f-1) == 'o' && *f == '*') {
					fprintf(stderr, "The +o option for speaker tier is not allowed. Illegal option: %s\n", (f-2));
					cutt_exit(0);
				} else
					maingetflag(f-2,f1,i);
				break;
		}
}

void init(char f) {
	if (f) {
		ftime = TRUE;
		bo = 0;
		maxwd_which = 0;
		mxd_pos = 0;
		TotalCnt = -1L;
		maxwd_head = NULL;
		uniq_length = FALSE;
		maxwd_isMorExcluded = FALSE;
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+$*");
		maininitwords();
		mor_initwords();
	} else {
		if (ftime) {
			ftime = FALSE;
			if (maxwd_which == 1) {
				if (maxwd_isMorExcluded) {
					nomain = FALSE;
				} else {
					nomain = TRUE;
					maketierchoice("%mor",'+',FALSE);
				}
			}
		}
		if (filterUttLen != 0L && CntFUttLen == 0) {
			fprintf(stderr,"Please specify count type with +x option\n");
			fprintf(stderr,"\tw - words, c - characters or m - morphemes.\n");
			if (maxwd_head != NULL)
				cleanupring();
			cutt_exit(0);
		}
		if (maxwd_head == NULL)
			makering(1);
		else if (!combinput)
			cleanupring();
	}
}

static void maxwd_pr_result(void) {
	OBJS *temp, *tt;
	long cnt = 0L;

	temp = maxwd_head;
	do {
		for (tt=temp; tt != NULL; tt=tt->cont) {
			if (tt->utte == NULL)
				continue;
			cnt++;
			if (TotalCnt > 0L && cnt > TotalCnt)
				return;
			if (onlydata < 1) {
				fprintf(fpout,"*** File \"%s\": ", tt->fn);
				if (lineno > 0) fprintf(fpout,"line %ld: ", tt->lineno);
			}
			if (maxwd_which) {
				int i;

				if (onlydata < 2) {
					if (maxwd_which == 1) {
						fprintf(fpout, "    Number of morphemes of chosen words %d. ",tt->size);
					} else if (maxwd_which == 2) {
						fprintf(fpout, "    Number of chosen words %d. ",tt->size);
					} else if (maxwd_which == 3) {
						fprintf(fpout, "    Total character length of chosen words %d. ",tt->size);
					}
				}
				if (onlydata < 2)
					putc('\n',fpout);
				for (i=strlen(tt->utte)-1; i>=0 && tt->utte[i] == '\n'; i--) ;
				tt->utte[++i] = EOS;
				for (i=0; tt->utte[i] == ' ' || tt->utte[i] == '\t'; i++) ;
				if (tt->deps != NULL && tt->isDepsSpName)
					fputs(tt->deps, fpout);
				printout(tt->speaker,tt->utte+i,NULL,NULL,TRUE);
				if (tt->deps != NULL && !tt->isDepsSpName)
					fputs(tt->deps, fpout);
			} else {
				if (onlydata < 2)
					fprintf(fpout,"%d characters long:\n", tt->size);
				fprintf(fpout,"%s\n", tt->utte);
			}
		}
		temp = temp->prevobj;
	} while (temp != maxwd_head) ;
}

static void addToDeps(OBJS *tt, char *sp, char *line) {
	char *t;

	if (tt == NULL)
		return;
	if (tt->deps == NULL) {
		tt->deps = (char *)malloc(strlen(sp)+strlen(line)+1);
		if (tt->deps == NULL) out_of_mem();
		*tt->deps = EOS;
	} else {
		t = tt->deps;
		tt->deps = (char *)malloc(strlen(t)+strlen(sp)+strlen(line)+1);
		if (tt->deps == NULL) out_of_mem();
		strcpy(tt->deps, t);
		free(t);
	}
	if (sp[0] == '*')
		tt->isDepsSpName = TRUE;
	strcat(tt->deps, sp);
	strcat(tt->deps, line);
}

static OBJS *rearange(OBJS *temp) {
	OBJS *tt;

	tt = maxwd_head->nextobj;
	maxwd_head->nextobj = tt->nextobj;
	tt->nextobj->prevobj = maxwd_head;
	if (tt->cont != NULL) clearcont(tt->cont);
	if (tt->speaker != NULL) free(tt->speaker);
	if (tt->utte != NULL) free(tt->utte);
	if (tt->deps != NULL) free(tt->deps);
	if (tt->fn != NULL) free(tt->fn);
	free(tt);
	tt = maxwd_head;
	while (tt->nextobj != temp)
		tt = tt->nextobj;
	if ((tt->nextobj=NEW(OBJS)) == NULL)
		out_of_mem();
	tt->nextobj->prevobj = tt;
	tt = tt->nextobj;
	tt->speaker = NULL;
	tt->utte = NULL;
	tt->isDepsSpName = FALSE;
	tt->deps = NULL;
	tt->fn = NULL;
	tt->lineno = 0;
	tt->size = 0;
	tt->cont = NULL;
	tt->nextobj = temp;
	temp->prevobj = tt;
	return(tt);
}

static OBJS *findmax(int csize) {
	int i, j, fi;
	OBJS *temp, *tt;
/*
temp=maxwd_head;
printf("csize=%d; %4s->", csize, word);
do {
	if (temp->utte == NULL) printf("0(NULL) ");
	else printf("%d(%s) ", temp->size, temp->utte);
	temp = temp->nextobj;
} while (temp != maxwd_head) ;
printf("\n");
*/
	if (csize < 1)
		return(NULL);

	tt = maxwd_head;
	temp = maxwd_head->nextobj;
	while (temp->utte == NULL && temp != maxwd_head) {
		tt = temp;
		temp = temp->nextobj;
	}
	if (temp->size > csize) {
		if (tt->utte != NULL)
			return(NULL);
		else
			temp = tt;
	}
	while (temp->size < csize && temp != maxwd_head)
		temp = temp->nextobj;
	if (temp->size < csize && temp == maxwd_head) {
		temp = temp->nextobj;
		maxwd_head = temp;
		tt = temp;
		clearcont(tt->cont);
		if (tt->speaker != NULL) free(tt->speaker);
		if (tt->utte != NULL) free(tt->utte);
		if (tt->deps != NULL) free(tt->deps);
		if (tt->fn != NULL) free(tt->fn);
		tt->cont = NULL;
		tt->speaker = NULL;
		tt->utte = NULL;
		tt->isDepsSpName = FALSE;
		tt->deps = NULL;
		tt->fn = NULL;
	} else if (temp->size == csize) {
		if (uniq_length) return(NULL);

		if (temp->cont == NULL) {
			temp->cont = NEW(OBJS);
			tt = temp->cont;
		} else {
			for (tt=temp->cont; 1; tt = tt->cont)
				if (tt->cont == NULL) break;
			tt->cont = NEW(OBJS);
			tt = tt->cont;
		}
		if (tt == NULL) out_of_mem();
		tt->speaker = NULL;
		tt->utte = NULL;
		tt->isDepsSpName = FALSE;
		tt->deps = NULL;
		tt->fn = NULL;
		tt->lineno = 0;
		tt->size = 0;
		tt->cont = NULL;
		tt->nextobj = NULL;
		tt->prevobj = NULL;
	} else {
		tt = rearange(temp);
	}

	if (maxwd_which) {
		char sq = FALSE;

		tt->speaker = (char *)malloc(strlen(utterance->speaker)+1);
		if (tt->speaker == NULL) out_of_mem();
		strcpy(tt->speaker, utterance->speaker);
		fi = mxd_pos;
		while (uS.isskip(utterance->line,fi,&dFnt,MBF) || sq) {
			if (utterance->line[fi] == EOS) break;
			if (utterance->line[fi] == '[') sq = TRUE;
			else if (utterance->line[fi] == ']') sq = FALSE;
			fi++;
		}
		for (i=fi; utterance->line[i] != HIDEN_C && utterance->line[i] != EOS; i++) ;
		templineC[0] = EOS;
		j = 0;
		if (utterance->line[i] == HIDEN_C) {
			templineC[j++] = utterance->line[i];
			for (i++; utterance->line[i] != HIDEN_C && utterance->line[i] != EOS; i++)
				templineC[j++] = utterance->line[i];
			if (utterance->line[i] == HIDEN_C) {
				templineC[j++] = utterance->line[i];
				templineC[j] = EOS;
			} else
				templineC[0] = EOS;
		}
		if (utterance->line[fi] != EOS) fi--;
		tt->utte = (char *)malloc(fi-bo+8+j);
		if (tt->utte == NULL) out_of_mem();
		for (i=0; bo <= fi; bo++) {
			tt->utte[i++] = utterance->line[bo];
		}
		while (utterance->line[bo]=='.')
			tt->utte[i++]= utterance->line[bo++];
		tt->utte[i] = EOS;
		if (templineC[0] != EOS)
			strcat(tt->utte, templineC);
	} else {
		if ((tt->utte=(char *)malloc(strlen(maxwd_word)+1)) == NULL) out_of_mem();
		strcpy(tt->utte, maxwd_word);
	}
	if ((tt->fn=(FNType *)malloc((strlen(oldfname)+1)*sizeof(FNType))) == NULL) out_of_mem();
	strcpy(tt->fn, oldfname);
	tt->lineno = lineno;
	tt->size = csize;
/*
temp=maxwd_head;
printf("after          ");
do {
	if (temp->utte == NULL) printf("0(NULL) ");
	else printf("%d(%s) ", temp->size, temp->utte);
	temp = temp->nextobj;
} while (temp != maxwd_head) ;
printf("\n");
*/
	if (spareTier1[0] != EOS) {
		addToDeps(tt, spareTier1, spareTier2);
		spareTier1[0] = EOS;
		spareTier2[0] = EOS;
	}
	return(tt);
}

static int maxwd_count(char *c) {
	register int wlen = 0;

	while (*c != EOS) {
		if (*c != '+' && *c != '-' && *c != '#'  &&
			*c != '`' && *c != '~' && *c != '$' && *c != '\'') {
			if (*c == '@') break;
			else if (*c == '^') {
				c++;
				if (*c == EOS)
					break;
			} else /* if (dFnt.isUTF) */ {
				if (UTF8_IS_SINGLE((unsigned char)*c) || UTF8_IS_LEAD((unsigned char)*c))
					wlen++;
			}
//			else
//				wlen++;
		}
		c++;
	}
	return(wlen);
}

static void findword() {
	int i;
	int csize;

	spareTier1[0] = EOS;
	spareTier2[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (*utterance->speaker == '@')
			continue;
		i = 0;
		csize = 0;
		while ((i=getword(utterance->speaker, uttline, maxwd_word, NULL, i))) {
			csize = maxwd_count(maxwd_word);
			findmax(csize);
		}
	}
}

static void findutt() {
	int csize;
	char delf = TRUE, sq, tmp, isAmbigFound;
	OBJS *cmain;

	spareTier1[0] = EOS;
	spareTier2[0] = EOS;
	cmain = NULL;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (*utterance->speaker == '@') {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			}
			continue;
		}
		if (CheckOutTier(utterance->speaker)) {
			if (maxwd_which && !maxwd_isMorExcluded && uS.partcmp(utterance->speaker,"%mor",FALSE,FALSE)) {
			} else {
				if (*utterance->speaker == '%') {
					addToDeps(cmain, utterance->speaker, utterance->line);
				}
				continue;
			}
		}
		csize = 0;
		if (chatmode) {
			delf = TRUE;
			if (maxwd_which == 1 && onlydata == 2 && !maxwd_isMorExcluded) {
				strcpy(spareTier1, org_spName);
				strcpy(spareTier2, org_spTier);
			}
		}
		mxd_pos = 0;
		bo = 0;
		if (cMediaFileName[0] != EOS)
			changeBullet(utterance->line, utterance->attLine);
		do {
			if (!uS.isskip(uttline,mxd_pos,&dFnt,MBF)) {
				if (filterUttLen != 0L && CntFUttLen == 1 && maxwd_which == 2)
					csize = utterance->uttLen;
				else if (filterUttLen != 0L && CntFUttLen == 2 && maxwd_which == 1)
					csize = utterance->uttLen;
				else if (filterUttLen != 0L && CntFUttLen == 3 && maxwd_which == 3)
					csize = utterance->uttLen;
				else if (maxwd_which == 3) { /* add word length's together */
					for (; !uS.isskip(uttline,mxd_pos,&dFnt,MBF) && uttline[mxd_pos]; csize++)
						mxd_pos++;
				} else if (maxwd_which == 1) { /* count number of morphs */
					isAmbigFound = FALSE;
					tmp = TRUE;
					while (!uS.isskip(uttline,mxd_pos,&dFnt,MBF) && uttline[mxd_pos]) {
						if (uttline[mxd_pos] == '^')
							isAmbigFound = TRUE;
						if (!uS.ismorfchar(uttline, mxd_pos, &dFnt, rootmorf, MBF) && !isAmbigFound) {
							if (tmp) {
								if (uttline[mxd_pos] != '0') csize++;
								tmp = FALSE;
							}
						} else
							tmp = TRUE;
						mxd_pos++;
					}
				} else { /* count number of words  */
					while (!uS.isskip(uttline,mxd_pos,&dFnt,MBF) && uttline[mxd_pos])
						mxd_pos++;
					csize++;
				}
				if (delf)
					delf = FALSE;
			}
			if (uS.IsUtteranceDel(uttline, mxd_pos)) {
				cmain = findmax(csize);
				delf = TRUE;
				csize = 0;
				sq = FALSE;
				bo = mxd_pos+1;
				while (uS.isskip(utterance->line,bo,&dFnt,MBF) || sq) {
					if (utterance->line[bo] == EOS) break;
					if (utterance->line[bo] == '[') sq = TRUE;
					else if (utterance->line[bo] == ']') sq = FALSE;
					bo++;
				}
			}
			if (uttline[mxd_pos])
				mxd_pos++;
			else
				break;
		} while (uttline[mxd_pos]) ;
		if (!delf && csize)
			cmain = findmax(csize);
	}
}

void call() {		/* inspect each file for longest word */
	if (maxwd_which)
		findutt();
	else
		findword();
	if (!combinput)
		maxwd_pr_result();
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = MAXWD;
	chatmode = CHAT_MODE;
	OnlydataLimit = 2;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,maxwd_pr_result);
	maxwd_cleanup(maxwd_head);
}
