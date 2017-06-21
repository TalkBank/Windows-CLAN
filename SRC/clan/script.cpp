/**********************************************************************
 "Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

#define CHAT_MODE 1
#include "cu.h"

#if !defined(UNX)
#define _main script_main
#define call script_call
#define getflag script_getflag
#define init script_init
#define usage script_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char OverWriteFile;
extern char outputOnlyData;
extern char cutt_isCAFound;
extern char cutt_isBlobFound;
extern char isRecursive;
extern struct tier *defheadtier;

struct script_fname {
	char *fname;
	int maxUttNum;
	float tmDur; 
	struct script_fname *nextFname;
};

struct script_utts {
	int cnt;
	int uttNum;
	struct script_utts *nextUtt;
};

struct contextListS {
	int count;
	char *context;
	struct contextListS *nextContext;
} ;

struct script_sp {
	struct contextListS *contextRoot;
	struct script_fname *fnameP;
	struct script_utts *spUtts;
	struct script_sp *nextSp;
};

struct script_tnode {
	char *word;
	char *POS;
	char isCode;
	struct script_utts *idealUtts;
	struct script_sp *sp_root;
	struct script_tnode *left;
	struct script_tnode *right;
};

static int  targc;
static char **targv;
static char ftime, isSpeakerNameGiven, isRemoveRetracedCodes;
static int  maxIdealUttNum;
static float ItmDur, tmDur, tmS;
static FILE *scriptFP;
static FILE *textFP;
static struct script_fname *files_root;
static script_tnode *words_root;

void usage() {
	printf("Usage: script [+e +s %s] filename(s)\n",mainflgs());
	puts("+e : count error codes in retraces or repeats. (default: don't count)");
	puts("+sF: specify template script file F");
	mainusage(FALSE);
	puts("\nExample:");
	puts("   Compare subjects data to ideal script \"eggs\" ....");
	puts("       script +t*PAR +seggs.cut *.cha");
	cutt_exit(0);
}

static void script_freeFiles(struct script_fname *p) {
	struct script_fname *t;

	while (p) {
		t = p;
		p = p->nextFname;
		free(t);
	}
}

static contextListS *freeWhat(contextListS *p) {
	contextListS *t;

	while (p != NULL) {
		t = p;
		p = p->nextContext;
		if (t->context != NULL)
			free(t->context);
		free(t);
	}
	return(NULL);
}

static void freeScriptUtts(struct script_utts *p) {
	struct script_utts *t;

	while (p) {
		t = p;
		p = p->nextUtt;
		free(t);
	}
}

static void freeSpeakers(struct script_sp *p) {
	struct script_sp *t;

	while (p) {
		t = p;
		p = p->nextSp;
		if (t->contextRoot != NULL)
			t->contextRoot = freeWhat(t->contextRoot);
		if (t->spUtts != NULL)
			freeScriptUtts(t->spUtts);
		free(t);
	}
}

static void script_freetree(struct script_tnode *p) {
	if (p != NULL) {
		script_freetree(p->left);
		script_freetree(p->right);
		if (p->idealUtts != NULL)
			freeScriptUtts(p->idealUtts);
		if (p->sp_root != NULL)
			freeSpeakers(p->sp_root);
		if (p->word != NULL)
			free(p->word);
		if (p->POS != NULL)
			free(p->POS);
		free(p);
	}
}

static void script_overflow(char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	script_freeFiles(files_root);
	files_root = NULL;
	script_freetree(words_root);
	words_root = NULL;
	if (scriptFP != NULL)
		fclose(scriptFP);
	cutt_exit(0);
}

static struct script_fname *findFileP(char *fname) {
	struct script_fname *p;

	if ((p=NEW(struct script_fname)) == NULL)
		script_overflow(TRUE);
	if ((p->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		free(p);
		script_overflow(TRUE);
	}
	strcpy(p->fname, fname);
	p->maxUttNum = 0;
	p->nextFname = files_root;
	files_root = p;
	return(p);
}

static char isCodeWord(char *word) {
	if (uS.mStricmp(word,"xxx")==0 || uS.mStrnicmp(word,"0",1)==0 || word[0]=='[')
		return(TRUE);
	return(FALSE);
}

static struct script_tnode *script_talloc(char *word, char *POS) {
	struct script_tnode *p;

	if ((p=NEW(struct script_tnode)) == NULL)
		script_overflow(TRUE);
	p->idealUtts = NULL;
	p->sp_root = NULL;
	p->left = p->right = NULL;
	p->isCode = isCodeWord(word);
	p->POS = NULL;
	if ((p->word=(char *)malloc(strlen(word)+1)) != NULL)
		strcpy(p->word, word);
	else {
		free(p);
		script_overflow(TRUE);
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
			script_overflow(TRUE);
		}
	}
	return(p);
}

static struct script_utts *addUtts(struct script_utts *root, int uttNum) {
	struct script_utts *t;

	if (root == NULL) {
		root = NEW(struct script_utts);
		t = root;
	} else {
		for (t=root; t->nextUtt != NULL; t=t->nextUtt) {
			if (t->uttNum == uttNum) {
				t->cnt++;
				return(root);
			}
		}
		if (t->uttNum == uttNum) {
			t->cnt++;
			return(root);
		}
		t->nextUtt = NEW(struct script_utts);
		t = t->nextUtt;
	}
	if (t == NULL)
		script_overflow(TRUE);
	t->cnt = 1;
	t->uttNum = uttNum;
	t->nextUtt = NULL;
	return(root);
}

static struct contextListS *script_AddContext(struct contextListS *contextRoot, char *context) {
	struct contextListS *p;

	if (contextRoot == NULL) {
		if ((p=NEW(struct contextListS)) == NULL)
			script_overflow(TRUE);
		contextRoot = p;
	} else {
		for (p=contextRoot; p->nextContext != NULL; p=p->nextContext) {
			if (!strcmp(p->context, context)) {
				p->count++;
				return(contextRoot);
			}
		}
		if (!strcmp(p->context, context)) {
			p->count++;
			return(contextRoot);
		}
		if ((p->nextContext=NEW(struct contextListS)) == NULL)
			script_overflow(TRUE);
		p = p->nextContext;
	}
	p->nextContext = NULL;
	if ((p->context=(char *)malloc(strlen(context)+1)) == NULL)
		script_overflow(TRUE);
	strcpy(p->context, context);
	p->count = 1;
	return(contextRoot);
}

static struct contextListS *script_AddToList(struct contextListS *contextRoot, int count, char *context) {
	struct contextListS *p;

	if (contextRoot == NULL) {
		if ((p=NEW(struct contextListS)) == NULL)
			script_overflow(TRUE);
		contextRoot = p;
	} else {
		for (p=contextRoot; p->nextContext != NULL; p=p->nextContext) {
			if (!strcmp(p->context, context)) {
				p->count += count;
				return(contextRoot);
			}
		}
		if (!strcmp(p->context, context)) {
			p->count += count;
			return(contextRoot);
		}
		if ((p->nextContext=NEW(struct contextListS)) == NULL)
			script_overflow(TRUE);
		p = p->nextContext;
	}
	p->nextContext = NULL;
	if ((p->context=(char *)malloc(strlen(context)+1)) == NULL)
		script_overflow(TRUE);
	strcpy(p->context, context);
	p->count = count;
	return(contextRoot);
}

static struct script_sp *addSp(struct script_sp *root, struct script_fname *fnameP, int uttNum, char *context) {
	struct script_sp *t;

	if (root == NULL) {
		root = NEW(struct script_sp);
		t = root;
	} else {
		for (t=root; t->nextSp != NULL; t=t->nextSp) {
			if (t->fnameP == fnameP) {
				t->spUtts = addUtts(t->spUtts, uttNum);
				if (context != NULL)
					t->contextRoot = script_AddContext(t->contextRoot, context);
				return(root);
			}
		}
		if (t->fnameP == fnameP) {
			t->spUtts = addUtts(t->spUtts, uttNum);
			if (context != NULL)
				t->contextRoot = script_AddContext(t->contextRoot, context);
			return(root);
		}
		t->nextSp = NEW(struct script_sp);
		t = t->nextSp;
	}
	t->contextRoot = NULL;
	t->spUtts = NULL;
	t->nextSp = NULL;
	if (t == NULL)
		script_overflow(TRUE);
	t->fnameP = fnameP;
	if (context != NULL)
		t->contextRoot = script_AddContext(t->contextRoot, context);
	t->spUtts = addUtts(t->spUtts, uttNum);
	return(root);
}

static struct script_tnode *script_tree(script_tnode *p,char *w,char *m,script_fname *fnameP,int uttNum,char *context) {
	int cond;

	if (p == NULL) {
		p = script_talloc(w, m);
		if (fnameP == NULL)
			p->idealUtts = addUtts(p->idealUtts, uttNum);
		else
			p->sp_root = addSp(p->sp_root, fnameP, uttNum, context);
	} else if ((cond=strcmp(w,p->word)) == 0) {
		if (fnameP == NULL)
			p->idealUtts = addUtts(p->idealUtts, uttNum);
		else
			p->sp_root = addSp(p->sp_root, fnameP, uttNum, context);
	} else if (cond < 0) {
		p->left = script_tree(p->left, w, m, fnameP, uttNum, context);
	} else {
		p->right = script_tree(p->right, w, m, fnameP, uttNum, context); /* if cond > 0 */
	}
	return(p);
}

static void dealWithDiscontinuousWord(char *word, int i) {
	if (word[strlen(word)-1] == '-') {
		while ((i=getword(utterance->speaker, uttline, templineC4, NULL, i))) {
			if (templineC4[0] == '-' && !uS.isToneUnitMarker(templineC4)) {
				strcat(word, templineC4+1);
				break;
			}
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

static void removeExtraSpaceFromTier(char *st) {
	int i;

	for (i=0; st[i] != EOS; ) {
		if (st[i]==' ' || st[i]=='\t' || (st[i]=='<' && (i==0 || st[i-1]==' ' || st[i-1]=='\t'))) {
			i++;
			while (st[i] == ' ' || st[i] == '\t')
				strcpy(st+i, st+i+1);
		} else
			i++;
	}
}

static void script_filterscop(char *wline) {
	int pos, temp, res, sqCnt;

	pos = strlen(wline) - 1;
	while (pos >= 0) {
		if (wline[pos] == HIDEN_C) {
			temp = pos;
			for (pos--; wline[pos] != HIDEN_C && pos >= 0; pos--) ;
			if (wline[pos] == HIDEN_C && pos >= 0) {
				for (; temp >= pos; temp--)
					wline[temp] = ' ';
			}
		} else if (uS.isRightChar(wline, pos, ']', &dFnt, MBF)) {
			sqCnt = 0;
			temp = pos;
			for (pos--; (!uS.isRightChar(wline, pos, '[', &dFnt, MBF) || sqCnt > 0) && pos >= 0; pos--) {
				if (uS.isRightChar(wline, pos, ']', &dFnt, MBF))
					sqCnt++;
				else if (uS.isRightChar(wline, pos, '[', &dFnt, MBF))
					sqCnt--;
			}
			if (pos < 0) {
				pos = temp - 1;
				if (isRecursive || cutt_isCAFound || cutt_isBlobFound)
					continue;
				else
					cutt_exit(1);
			}
			wline[temp] = EOS;
			uS.isSqCodes(wline+pos+1, templineC3, &dFnt, TRUE);
			wline[temp] = ']';
			uS.lowercasestr(templineC3, &dFnt, MBF);
			res = isExcludeScope(templineC3);
			if (res == 0 || res == 2)
				ExcludeScope(wline,pos, TRUE);
		} else
			pos--;
	}
}

static void CleanUpLine(char *st) {
	int pos;

	for (pos=0; st[pos] != EOS; ) {
		if (st[pos] == '\n' || st[pos] == '\t')
			st[pos] = ' ';
		if ((isSpace(st[pos]) || st[pos] == '\n') && (isSpace(st[pos+1]) || st[pos+1] == EOS))
			strcpy(st+pos, st+pos+1);
		else
			pos++;
	}
}

static void script_process_tier(struct script_fname *fnameP, int uttNum) {
	int  i, wi;
	char word[1024], tword[1024];
	long stime, etime;

	if (utterance->speaker[0] == '*') {
		removeExtraSpaceFromTier(utterance->line);
		if (uttNum <= 0) {
			fprintf(stderr,"Internal error: uttNum <= 0.\n");
			fprintf(stderr,"Quiting program.\n");
			script_overflow(FALSE);
		}
		for (i=0; utterance->line[i] != EOS; i++) {
			if (utterance->line[i] == HIDEN_C && isdigit(utterance->line[i+1])) {
				if (getMediaTagInfo(utterance->line+i, &stime, &etime)) {
					if (tmS < 0.0)
						tmS = (float)(stime / 1000);
					tmDur = (float)(etime / 1000);
					tmDur = tmDur - tmS;
				}
			}
		}
		strcpy(spareTier1, utterance->line);
		if (linkMain2Mor) {
			cutt_cleanUpLine(utterance->speaker, utterance->line, utterance->attLine, 0);
		} else {
			i = 0;
			while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
				if (word[0] == '-' && !uS.isToneUnitMarker(word) && !exclude(word))
					continue;
				dealWithDiscontinuousWord(word, i);
				if (exclude(word) && isRightWord(word)) {
					uS.remblanks(word);
					words_root = script_tree(words_root, word, NULL, fnameP, uttNum, NULL);
				}
			}
		}
		strcpy(org_spTier, utterance->line);
		if (isRemoveRetracedCodes)
			script_filterscop(spareTier1);
		i = 0;
		while ((i=getword(utterance->speaker, spareTier1, word, &wi, i))) {
			if (uS.isSqCodes(word, tword, &dFnt, FALSE)) {
				if (!strncmp(tword, "[*", 2)) {
					findWholeScope(wi, templineC4);
					uS.remFrontAndBackBlanks(templineC4);
					CleanUpLine(templineC4);
					uS.remblanks(tword);
					words_root = script_tree(words_root, tword, NULL, fnameP, uttNum, templineC4);
				}
			}
		}
	} else if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
		if (uttNum <= 0) {
			fprintf(stderr,"Internal error: uttNum <= 0.\n");
			fprintf(stderr,"Quiting program.\n");
			script_overflow(FALSE);
		}
		if (linkMain2Mor && strchr(uttline, dMarkChr) != NULL) {
			i = 0;
			while ((i=getNextDepTierPair(uttline, word, tword, NULL, i)) != 0) {
				if (word[0] == '-' && !uS.isToneUnitMarker(word) && !exclude(word))
					continue;
				if (exclude(word) && isRightWord(word)) {
					uS.remblanks(word);
					uS.remblanks(tword);
					words_root = script_tree(words_root, tword, word, fnameP, uttNum, NULL);
				}
			}
		} else if (!R6) {
			i = 0;
			while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
				if (word[0] == '-' && !uS.isToneUnitMarker(word) && !exclude(word))
					continue;
				dealWithDiscontinuousWord(word, i);
				if (exclude(word) && isRightWord(word)) {
					uS.remblanks(word);
					words_root = script_tree(words_root, word, NULL, fnameP, uttNum, NULL);
				}
			}
		}
	}
}

static void totalNumWords(struct script_tnode *p, script_fname *fn, int *cnt) {
	struct script_sp *sp;
	struct script_utts *utt;

	if (p != NULL) {
		totalNumWords(p->left, fn, cnt);
		totalNumWords(p->right, fn, cnt);
		if (p->isCode)
			return;
		for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
			if (sp->fnameP == fn) {
				for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
					*cnt = *cnt + utt->cnt;
				}
			}
		}
	}
}

static void totalNumIdealWords(struct script_tnode *p, int *cnt) {
	struct script_utts *utt;

	if (p != NULL) {
		totalNumIdealWords(p->left, cnt);
		totalNumIdealWords(p->right, cnt);
		if (p->isCode)
			return;
		for (utt=p->idealUtts; utt != NULL; utt=utt->nextUtt) {
			*cnt = *cnt + utt->cnt;
		}
	}
}

static void totalNumCorrectWords(struct script_tnode *p, script_fname *fn, int *cnt) {
	int uttNum;
	struct script_sp *sp;
	struct script_utts *uttI, *utt;

	if (p != NULL) {
		totalNumCorrectWords(p->left, fn, cnt);
		totalNumCorrectWords(p->right, fn, cnt);
		if (p->isCode)
			return;
		for (uttNum=1; uttNum <= fn->maxUttNum; uttNum++) {
			uttI = NULL;
			for (utt=p->idealUtts; utt != NULL; utt=utt->nextUtt) {
				if (utt->uttNum == uttNum) {
					uttI = utt;
					break;
				}
			}
			if (uttI != NULL) {
				for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
					if (sp->fnameP == fn) {
						for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
							if (utt->uttNum == uttNum) {
								if (uttI->cnt >= utt->cnt)
									*cnt = *cnt + utt->cnt;
								else // if (uttI->cnt < utt->cnt)
									*cnt = *cnt + uttI->cnt;
								break;
							}
						}
					}
				}
			}
		}
	}
}

static void totalNumOmittedWords(struct script_tnode *p, script_fname *fn, int *cnt) {
	int  uttNum;
	char isFound;
	struct script_sp *sp;
	struct script_utts *uttI, *utt;

	if (p != NULL) {
		totalNumOmittedWords(p->left, fn, cnt);
		totalNumOmittedWords(p->right, fn, cnt);
		if (p->isCode)
			return;
		for (uttNum=1; uttNum <= maxIdealUttNum; uttNum++) {
			uttI = NULL;
			for (utt=p->idealUtts; utt != NULL; utt=utt->nextUtt) {
				if (utt->uttNum == uttNum) {
					uttI = utt;
					break;
				}
			}
			if (uttI != NULL) {
				isFound = FALSE;
				for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
					if (sp->fnameP == fn) {
						for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
							if (utt->uttNum == uttNum) {
								if (uttI->cnt > utt->cnt)
									*cnt = *cnt + (uttI->cnt - utt->cnt);
								isFound = TRUE;
								break;
							}
						}
					}
				}
				if (!isFound)
					*cnt = *cnt + uttI->cnt;
			}
		}
	}
}

static void totalNumAddedWords(struct script_tnode *p, script_fname *fn, int *cnt) {
	int  uttNum;
	struct script_sp *sp;
	struct script_utts *uttI, *utt;

	if (p != NULL) {
		totalNumAddedWords(p->left, fn, cnt);
		totalNumAddedWords(p->right, fn, cnt);
		if (p->isCode)
			return;
		for (uttNum=1; uttNum <= fn->maxUttNum; uttNum++) {
			uttI = NULL;
			for (utt=p->idealUtts; utt != NULL; utt=utt->nextUtt) {
				if (utt->uttNum == uttNum) {
					uttI = utt;
					break;
				}
			}
			for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
				if (sp->fnameP == fn) {
					for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
						if (utt->uttNum == uttNum) {
							if (uttI != NULL) {
								if (uttI->cnt < utt->cnt) {
									*cnt = *cnt + (utt->cnt - uttI->cnt);
								}
							} else
								*cnt = *cnt + utt->cnt;
							break;
						}
					}
				}
			}
		}
	}
}

static void totalNumErrCode(struct script_tnode *p, script_fname *fn, const char *code, int *cnt) {
	int len;
	struct script_sp *sp;
	struct script_utts *utt;

	if (p != NULL) {
		totalNumErrCode(p->left, fn, code, cnt);
		totalNumErrCode(p->right, fn, code, cnt);
		if (!p->isCode)
			return;
		len = strlen(code);
		if (strncmp(p->word, code, len))
			return;
		for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
			if (sp->fnameP == fn) {
				for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
					*cnt = *cnt + utt->cnt;
				}
			}
		}
	}
}

static void totalNumXX(struct script_tnode *p, script_fname *fn, int *cnt) {
	struct script_sp *sp;
	struct script_utts *utt;

	if (p != NULL) {
		totalNumXX(p->left, fn, cnt);
		totalNumXX(p->right, fn, cnt);
		if (p->isCode)
			return;
		if (strcmp(p->word, "xx"))
			return;
		for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
			if (sp->fnameP == fn) {
				for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
					*cnt = *cnt + utt->cnt;
				}
			}
		}
	}
}

static void totalNumXXX(struct script_tnode *p, script_fname *fn, int *cnt) {
	int  uttNum;
	struct script_sp *sp;
	struct script_utts *utt;

	if (p != NULL) {
		totalNumXXX(p->left, fn, cnt);
		totalNumXXX(p->right, fn, cnt);
		if (!p->isCode)
			return;
		if (strcmp(p->word, "xxx"))
			return;
		for (uttNum=1; uttNum <= fn->maxUttNum; uttNum++) {
			for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
				if (sp->fnameP == fn) {
					for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
						if (utt->uttNum == uttNum) {
							*cnt = *cnt + 1;
							break;
						}
					}
				}
			}
		}
	}
}

static void totalNum0s(struct script_tnode *p, script_fname *fn, int *cnt) {
	struct script_sp *sp;
	struct script_utts *utt;

	if (p != NULL) {
		totalNum0s(p->left, fn, cnt);
		totalNum0s(p->right, fn, cnt);
		if (!p->isCode)
			return;
		if (strcmp(p->word, "0"))
			return;
		for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
			if (sp->fnameP == fn) {
				for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
					*cnt = *cnt + utt->cnt;
				}
			}
		}
	}
}

static struct contextListS *listOmittedWords(struct contextListS *root_list, struct script_tnode *p, script_fname *fn) {
	int  uttNum;
	char isFound;
	struct script_sp *sp;
	struct script_utts *uttI, *utt;

	if (p != NULL) {
		root_list = listOmittedWords(root_list, p->left, fn);
		root_list = listOmittedWords(root_list, p->right, fn);
		if (p->isCode)
			return(root_list);
		for (uttNum=1; uttNum <= maxIdealUttNum; uttNum++) {
			uttI = NULL;
			for (utt=p->idealUtts; utt != NULL; utt=utt->nextUtt) {
				if (utt->uttNum == uttNum) {
					uttI = utt;
					break;
				}
			}
			if (uttI != NULL) {
				isFound = FALSE;
				for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
					if (sp->fnameP == fn) {
						for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
							if (utt->uttNum == uttNum) {
								if (uttI->cnt > utt->cnt) {
									strcpy(templineC3, p->word);
									if (p->POS != NULL) {
										strcat(templineC3, " - ");
										strcat(templineC3, p->POS);
									}
									root_list = script_AddToList(root_list, uttI->cnt-utt->cnt, templineC3);
								}
								isFound = TRUE;
								break;
							}
						}
					}
				}
				if (!isFound) {
					strcpy(templineC3, p->word);
					if (p->POS != NULL) {
						strcat(templineC3, " - ");
						strcat(templineC3, p->POS);
					}
					root_list = script_AddToList(root_list, uttI->cnt, templineC3);
				}
			}
		}
	}
	return(root_list);
}

static struct contextListS *listAddedWords(struct contextListS *root_list, struct script_tnode *p, script_fname *fn) {
	int  uttNum;
	struct script_sp *sp;
	struct script_utts *uttI, *utt;

	if (p != NULL) {
		root_list = listAddedWords(root_list, p->left, fn);
		root_list = listAddedWords(root_list, p->right, fn);
		if (p->isCode)
			return(root_list);
		for (uttNum=1; uttNum <= fn->maxUttNum; uttNum++) {
			uttI = NULL;
			for (utt=p->idealUtts; utt != NULL; utt=utt->nextUtt) {
				if (utt->uttNum == uttNum) {
					uttI = utt;
					break;
				}
			}
			for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
				if (sp->fnameP == fn) {
					for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt) {
						if (utt->uttNum == uttNum) {
							if (uttI != NULL) {
								if (uttI->cnt < utt->cnt) {
									strcpy(templineC3, p->word);
									if (p->POS != NULL) {
										strcat(templineC3, " - ");
										strcat(templineC3, p->POS);
									}
									root_list = script_AddToList(root_list, utt->cnt-uttI->cnt, templineC3);
								}
							} else {
								strcpy(templineC3, p->word);
								if (p->POS != NULL) {
									strcat(templineC3, " - ");
									strcat(templineC3, p->POS);
								}
								root_list = script_AddToList(root_list, utt->cnt, templineC3);
							}
							break;
						}
					}
				}
			}
		}
	}
	return(root_list);
}

static struct contextListS *listErrorCodes(struct contextListS *root_list, struct script_tnode *p, script_fname *fn) {
	int cnt;
	struct script_sp *sp;
	struct script_utts *utt;
	struct contextListS *err;

	if (p != NULL) {
		root_list = listErrorCodes(root_list, p->left, fn);
		root_list = listErrorCodes(root_list, p->right, fn);
		if (!p->isCode || p->word[0] != '[')
			return(root_list);
		for (sp=p->sp_root; sp != NULL; sp=sp->nextSp) {
			if (sp->fnameP == fn) {
				cnt = 0;
				for (utt=sp->spUtts; utt != NULL; utt=utt->nextUtt)
					cnt = cnt + utt->cnt;
				if (cnt > 1)
					fprintf(textFP,"  %3d %s\n", cnt, p->word);
				for (err=sp->contextRoot; err != NULL; err=err->nextContext) {
					if (cnt > 1)
						fprintf(textFP,"      %3d %s\n", err->count, err->context);
					else
						root_list = script_AddToList(root_list, err->count, err->context);
				}
			}
		}
	}
	return(root_list);
}

static void printList(struct contextListS *root_list) {
	struct contextListS *p;

	for (p=root_list; p != NULL; p=p->nextContext) {
		fprintf(textFP,"    %3d %s\n", p->count, p->context);
	}
}

static void script_pr_result(void) {
	int cnt, iWds, rerr, unerr;
	char  st[1024];
	float wpm, tfloat;
	struct script_fname *fn;
	struct contextListS *root_list;

	fprintf(fpout, "File name\tIdeal TIMDUR\tTIMDUR\t# wds produced\t# wds per minute\t");
	fprintf(fpout, "# wds ideal\t# wds correct\t%% wds correct\t# wds omitted\t");
	fprintf(fpout, "%% wds omitted\t# wds added\t# recog errors\t# unrecog errors\t");
	fprintf(fpout, "# utts with xxx\t# utts with 0\n");
	for (fn=files_root; fn != NULL; fn=fn->nextFname) {
		fprintf(fpout, "%s", fn->fname);
		Secs2Str(ItmDur, st);
		fprintf(fpout,"\t%s", st);
		Secs2Str(fn->tmDur, st);
		fprintf(fpout,"\t%s", st);
		cnt = 0;
		totalNumWords(words_root, fn, &cnt);
		fprintf(fpout, "\t%d", cnt);
		wpm = (float)cnt;
		wpm = wpm / (fn->tmDur / 60.0000);
		fprintf(fpout, "\t%.4f", wpm);
		iWds = 0;
		totalNumIdealWords(words_root, &iWds);
		fprintf(fpout, "\t%d", iWds);
		cnt = 0;
		totalNumCorrectWords(words_root, fn, &cnt);
		fprintf(fpout, "\t%d", cnt);
		wpm = (float)cnt;
		tfloat = iWds;
		wpm = wpm * 100.0000 / tfloat;
		fprintf(fpout, "\t%.0f", wpm);
		cnt = 0;
		totalNumOmittedWords(words_root, fn, &cnt);
		fprintf(fpout, "\t%d", cnt);
		wpm = (float)cnt;
		tfloat = iWds;
		wpm = wpm * 100.0000 / tfloat;
		fprintf(fpout, "\t%.0f", wpm);
		cnt = 0;
		totalNumAddedWords(words_root, fn, &cnt);
		fprintf(fpout, "\t%d", cnt);
		unerr = 0;
		cnt = 0;
		totalNumErrCode(words_root, fn, "[* s:uk", &cnt);
		unerr += cnt;
		cnt = 0;	
		totalNumErrCode(words_root, fn, "[* n:uk", &cnt);
		unerr += cnt;
		cnt = 0;
		totalNumErrCode(words_root, fn, "[*", &cnt);
		rerr = cnt - unerr;
		if (rerr < 0)
			rerr = 0;
		cnt = 0;	
		totalNumXX(words_root, fn, &cnt);
		unerr += cnt;
		fprintf(fpout, "\t%d", rerr);
		fprintf(fpout, "\t%d", unerr);
		cnt = 0;
		totalNumXXX(words_root, fn, &cnt);
		fprintf(fpout, "\t%d", cnt);
		cnt = 0;
		totalNum0s(words_root, fn, &cnt);
		fprintf(fpout, "\t%d", cnt);
		fputc('\n', fpout);
		if (textFP != NULL) {
			fprintf(textFP, "**** From file: %s\n", fn->fname);

			fprintf(textFP, "  List of omitted words:\n");
			root_list = NULL;
			root_list = listOmittedWords(root_list, words_root, fn);
			printList(root_list);
			root_list = freeWhat(root_list);

			fprintf(textFP, "\n  List of added words:\n");
			root_list = NULL;
			root_list = listAddedWords(root_list, words_root, fn);
			printList(root_list);
			root_list = freeWhat(root_list);

			fprintf(textFP, "\n  List of errors:\n");
			root_list = NULL;
			root_list = listErrorCodes(root_list, words_root, fn);
			printList(root_list);
			root_list = freeWhat(root_list);

			fprintf(textFP, "\n");
		}
	}
	fprintf(fpout, "\n");
//	printArg(targv, targc, fpout, FALSE, "");
	script_freeFiles(files_root);
	files_root = NULL;
	script_freetree(words_root);
	words_root = NULL;
}

void call() {
	int  uttNum;
	char lRightspeaker, tLinkMain2Mor;
	struct script_fname *fnameP;
	extern char GExt[];

	if (textFP == NULL) {
		AddCEXExtension = ".cex";
		parsfname(oldfname, FileName2, GExt);
		if ((textFP=openwfile(oldfname, FileName2, NULL)) == NULL) {
			fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n", FileName1);
			script_overflow(FALSE);
		}
	}
	fnameP = findFileP(oldfname);
	uttNum = 0;
	tmS = 0.0;
	tmDur = 0.0;
	lRightspeaker = FALSE;
	tLinkMain2Mor = linkMain2Mor;
	linkMain2Mor = TRUE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (!checktier(utterance->speaker)) {
			if (*utterance->speaker == '*')
				lRightspeaker = FALSE;
			continue;
		} else {
			if (*utterance->speaker == '*') {
				uttNum++;
				lRightspeaker = TRUE;
			}
			if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (uS.partcmp(utterance->speaker,"@Time Duration:",FALSE,FALSE)) {
			uS.remFrontAndBackBlanks(utterance->line);
			tmDur = tmDur + getTimeDuration(utterance->line);
		} else
			script_process_tier(fnameP, uttNum);
	}
	linkMain2Mor = tLinkMain2Mor;
	fnameP->tmDur = tmDur;
	fnameP->maxUttNum = uttNum;
	if (!combinput)
		script_pr_result();
}

static void readTemplateFile(void) {
	int  uttNum;
	char tLinkMain2Mor;
	FILE *tfpin;

	tLinkMain2Mor = linkMain2Mor;
	linkMain2Mor = TRUE;
	tfpin = fpin;
	fpin = scriptFP;
	uttNum = 0;
	tmS = 0.0;
	tmDur = 0.0;
	ItmDur = 0.0;
	maxIdealUttNum = 0;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (*utterance->speaker == '*')
			uttNum++;
		if (uS.partcmp(utterance->speaker,"@Time Duration:",FALSE,FALSE)) {
			uS.remFrontAndBackBlanks(utterance->line);
			tmDur = tmDur + getTimeDuration(utterance->line);
		} else
			script_process_tier(NULL, uttNum);
	}
	ItmDur = tmDur;
	maxIdealUttNum = uttNum;
	fpin = tfpin;
	linkMain2Mor = tLinkMain2Mor;
}

void init(char first) {
	if (first) {
		ftime = TRUE;
		stout = FALSE;
		outputOnlyData = TRUE;
		OverWriteFile = TRUE;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		AddCEXExtension = ".xls";
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
//		addword('\0','\0',"+0");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
		addword('\0','\0',"+<\">");
		mor_initwords();
// defined in "mmaininit" and "globinit" - nomap = TRUE;
		isSpeakerNameGiven = FALSE;
		isRemoveRetracedCodes = TRUE;
		words_root = NULL;
		files_root = NULL;
		textFP = NULL;
		scriptFP = NULL;
		textFP = NULL;
		tmS = 0.0;
		tmDur = 0.0;
		ItmDur = 0.0;
		maxIdealUttNum = 0;
	} else {
		AddCEXExtension = ".xls";
		if (ftime) {
			ftime = FALSE;
			if (!isSpeakerNameGiven) {
				fprintf(stderr,"Please specify speaker's tier code with \"+t\" option on command line.\n");
				script_overflow(FALSE);
			}
			if (scriptFP == NULL) {
				fprintf(stderr,"Please specify template script file name with \"+s\" option on command line.\n");
				script_overflow(FALSE);				
			}
			readTemplateFile();
			fclose(scriptFP);
			scriptFP = NULL;
		}
		if (!combinput || first) {
			if (textFP != NULL) {
				fclose(textFP);
				fprintf(stderr,"Output file <%s>\n", FileName2);
				textFP = NULL;
			}
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	targv = argv;
	targc = argc;
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SCRIPT_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,script_pr_result);
	script_freeFiles(files_root);
	files_root = NULL;
	script_freetree(words_root);
	words_root = NULL;
	if (scriptFP != NULL)
		fclose(scriptFP);
	if (textFP != NULL) {
		fprintf(stderr,"Output file <%s>\n", FileName2);
		fclose(textFP);
		textFP = NULL;
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'e':
			isRemoveRetracedCodes = FALSE;
			no_arg_option(f);
			break;
		case 's':
			if (*f == EOS) {
				fprintf(stderr,"Please specify template script file name with \"+s\" option on command line.\n");
				script_overflow(FALSE);
			} else {
				if ((scriptFP=OpenGenLib(f,"r",TRUE,TRUE,FileName1)) == NULL) {
					fprintf(stderr,"ERROR: Can't open script file: %s\n", FileName1);
					script_overflow(FALSE);
				}
				fprintf(stderr,"Database File: %s\n", FileName1);
			}
			break;
		case 't':
			if (*(f-2) == '+')
				isSpeakerNameGiven = TRUE;
			maingetflag(f-2,f1,i);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}
