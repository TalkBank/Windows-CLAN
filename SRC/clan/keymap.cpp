/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 2

#include "cu.h"

#if !defined(UNX)
#define _main keymap_main
#define call keymap_call
#define getflag keymap_getflag
#define init keymap_init
#define usage keymap_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

extern char GExt[];
extern char tct;
extern char Preserve_dir;
extern char stin_override;
extern struct tier *defheadtier;

#define CODECHAR '$'

#define KEYWORDS struct symbols
KEYWORDS {
	char *keywordname;
	char *mask;
	KEYWORDS *nextkeyword;
} ;

#define SPEAKERS struct speakers
#define MAPKEYS struct keymaps
MAPKEYS {
	char *mapkeyname;
	char *mask;
	int  count;
	FNType fname[FNSize];
	long ln;
	SPEAKERS *maps;
	MAPKEYS *nextmapkey;
} ;

SPEAKERS {
	char *sp;
	int count;
	MAPKEYS *mapkey;
	SPEAKERS *nextspeaker;
} ;

#define KEYWDROW struct keywdrows
KEYWDROW {
	char *fname;
	char *sp;
	char *keyword;
	int  count;
	int  rowCnt;
	KEYWDROW *nextrow;
} ;

#define EXCELROW struct excelrows
EXCELROW {
	int count;
	int rowCnt;
	EXCELROW *nextrow;
} ;

#define EXCELCOL struct excelcols
EXCELCOL {
	char *keyword;
	KEYWDROW *keyrows;
	EXCELROW *rows;
	EXCELCOL *nextcol;
} ;

static int  ExcelCnt;
static char isExcel, isPresedeCodes;
static char keymap_CodeTierGiven;
static KEYWORDS *rootkey, *rootcompkey;
static SPEAKERS *rootspeaker;
static EXCELCOL *rootexcel;

void usage() {
	puts("KEYMAP creates a contingency table for coded speech functions.");
	printf("Usage: keymap [bS cS d %s] filename(s)\n", mainflgs());
	puts("+bS: sets a key code to S");
	puts("+b@F: sets a key code to codes specified in file @F.");
	puts("+cS: sets complimentary key code to S");
	puts("+c@F: sets complimentary key code to codes specified in file @F.");
	puts("+d : outputs in SPREADSHEET format");
	puts("+o : include codes that precede target code(s)");
	mainusage(FALSE);
	puts("\nExample:");
	puts("       keymap +b'$CW' +t%spa *.cha");
	puts("       keymap +b'$CW' +b'$CR' +t%spa *.cha");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		ExcelCnt = 0;
		isExcel = FALSE;
		isPresedeCodes = FALSE;
		rootkey = NULL;
		rootexcel = NULL;
		rootcompkey = NULL;
		rootspeaker = NULL;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		tct = TRUE;
		keymap_CodeTierGiven = '\0';
	} else {
		if (keymap_CodeTierGiven == '\0') {
			fprintf(stderr,"Please specify a code tier with \"+t%%\" option.\n");
			cutt_exit(0);
		}
		if (!rootkey) {
			fprintf(stderr, "Please specify initiation marker with \"+b\" option.\n");
			cutt_exit(0);
		}
		if (isExcel) {
			stout = TRUE;
		}
		if (rootcompkey != NULL)
			combinput = FALSE;
		if (!combinput)
			rootspeaker = NULL;
	}
}

static int mkmask(char *st, char *mask) {
	int ft = TRUE, pf = FALSE;

	for (; *st; st++) {
		if (isalnum((unsigned char)*st) || *st == '\\') {
			if (*st == '\\') {
				if (*(st+1))
					st++;
			}
			if (ft) {
				ft = FALSE;
				*mask++ = '*';
			}
		} else {
			if (*st == '%')
				pf = TRUE;
			ft = TRUE;
			*mask++ = *st;
		}
	}
	*mask = EOS;
	return(pf);
}

static void SetUpKeyWords(char *st) {
	KEYWORDS *p;
	char mask[BUFSIZ];

	if ((p=NEW(KEYWORDS)) == NULL)
		out_of_mem();
	if (!mkmask(st, mask))
		*mask = EOS;
	p->mask = (char *)malloc((size_t)strlen(mask)+1);
	if (p->mask == NULL)
		out_of_mem();
	strcpy(p->mask, mask);
	if (!nomap)
		uS.lowercasestr(st, &dFnt, C_MBF);
	p->keywordname = (char *)malloc(strlen(st)+1);
	if (p->keywordname == NULL)
		out_of_mem();
	strcpy(p->keywordname, st);
	p->nextkeyword = rootkey;
	rootkey = p;
}

static void SetUpCompKeyWords(char *st) {
	KEYWORDS *p;
	char mask[BUFSIZ];

	if ((p=NEW(KEYWORDS)) == NULL)
		out_of_mem();
	if (!mkmask(st,mask))
		*mask = EOS;
	p->mask = (char *)malloc((size_t)strlen(mask)+1);
	if (p->mask == NULL)
		out_of_mem();
	strcpy(p->mask, mask);
	if (!nomap)
		uS.lowercasestr(st, &dFnt, C_MBF);
	p->keywordname = (char *)malloc(strlen(st)+1);
	if (p->keywordname == NULL)
		out_of_mem();
	strcpy(p->keywordname, st);
	p->nextkeyword = rootcompkey;
	rootcompkey = p;
}

static void getFileCodes(char opt, const FNType *fname) {
	FILE *efp;
	char wd[1024];
	int  len;
	FNType mFileName[FNSize];

	if (*fname == EOS) {
		if (opt == 'b')
			fprintf(stderr,	"No codes file specified.\n");
		else if (opt == 'c')
			fprintf(stderr,	"No complimentary codes file specified.\n");
		cutt_exit(0);
	}

	if ((efp=OpenGenLib(fname,"r",TRUE,TRUE,mFileName)) == NULL) {
		if (opt == 'b')
			fprintf(stderr, "Can't open either one of codes files:\n\t\"%s\", \"%s\"\n", fname, mFileName);
		else if (opt == 'c')
			fprintf(stderr, "Can't open either one of complimentary codes files:\n\t\"%s\", \"%s\"\n", fname, mFileName);
		cutt_exit(0);
	}
	if (opt == 'b')
		fprintf(stderr, "    Using codes file: %s\n", mFileName);
	else if (opt == 'c')
		fprintf(stderr, "    Using complimentary codes file: %s\n", mFileName);
	while (fgets_cr(wd, 1024, efp)) {
		if (uS.isUTF8(wd) || uS.isInvisibleHeader(wd))
			continue;
		if (wd[0] == '#' && wd[1] == ' ')
			continue;
		for (len=strlen(wd)-1; (wd[len]== ' ' || wd[len]== '\t' || wd[len]== '\n') && len >= 0; len--) ;
		if (len < 0)
			continue;
		wd[len+1] = EOS;
		if (wd[0] == '\'' && wd[len] == '\'') {
			wd[len] = EOS;
			strcpy(wd, wd+1);
		}
		removeExtraSpace(wd);
		uS.remFrontAndBackBlanks(wd);
		if (opt == 'b')
			SetUpKeyWords(wd);
		else if (opt == 'c')
			SetUpCompKeyWords(wd);

	}
	fclose(efp);
}

static char *keymap_inthere(char *st, KEYWORDS *p) {
	while (p != NULL) {
		if (uS.patmat(st,p->keywordname)) {
			uS.remblanks(st);
			return(p->mask);
		}
		p = p->nextkeyword;
	}
	return(NULL);
}

static SPEAKERS *FindSpeaker(char *spName) {
	SPEAKERS *tsp;

	if (rootspeaker == NULL) {
		if ((rootspeaker=NEW(SPEAKERS)) == NULL)
			out_of_mem();
		tsp = rootspeaker;
	} else {
		tsp = rootspeaker;
		while (1) {
			if (!strcmp(spName,tsp->sp))
				return(tsp);
			if (tsp->nextspeaker == NULL)
				break;
			tsp = tsp->nextspeaker;
		}
		if ((tsp->nextspeaker=NEW(SPEAKERS)) == NULL)
			out_of_mem();
		tsp = tsp->nextspeaker;
	}
	tsp->sp = (char *)malloc((size_t)strlen(spName)+1);
	if (tsp->sp == NULL) {
		printf("No more memory.\n"); cutt_exit(1);
	}
	strcpy(tsp->sp, spName);
	tsp->mapkey = NULL;
	tsp->nextspeaker = NULL;
	return(tsp);
}

static SPEAKERS *RegisterSpeakerKeyword(char *word, char *mask, SPEAKERS *curSp, long ln) {
	MAPKEYS *tkey, *tkey2, *new_key;

	if (curSp->mapkey == NULL) {
		if ((curSp->mapkey=NEW(MAPKEYS)) == NULL)
			out_of_mem();
		tkey = curSp->mapkey;
		tkey->nextmapkey = NULL;
	} else {
		tkey = curSp->mapkey;
		tkey2 = NULL;
		while (1) {
			if (!strcmp(word,tkey->mapkeyname)) {
				tkey->count++;
				strcpy(tkey->fname, oldfname);
				tkey->ln = ln;
				return(curSp);
			}
			if (strcmp(word,tkey->mapkeyname) < 0) {
				if ((new_key=NEW(MAPKEYS)) == NULL)
					out_of_mem();
				new_key->nextmapkey = tkey;
				if (tkey2 == NULL)
					curSp->mapkey = new_key;
				else
					tkey2->nextmapkey = new_key;
				tkey = new_key;
				break;
			}
			if (tkey->nextmapkey == NULL) {
				if ((tkey->nextmapkey=NEW(MAPKEYS)) == NULL)
					out_of_mem();
				tkey = tkey->nextmapkey;
				tkey->nextmapkey = NULL;
				break;
			}
			tkey2 = tkey;
			tkey = tkey->nextmapkey;
		}
	}
	tkey->mapkeyname = (char *)malloc((size_t)strlen(word)+1);
	if (tkey->mapkeyname == NULL)
		out_of_mem();
	strcpy(tkey->mapkeyname, word);
	tkey->mask = mask;
	tkey->count = 1;
	strcpy(tkey->fname, oldfname);
	tkey->ln = ln;
	tkey->maps = NULL;
	return(curSp);
}

static SPEAKERS *RegisterKeyword(char *word, char *mask, char *spName, long ln, SPEAKERS *curSp) {
	if (curSp == NULL)
		curSp = FindSpeaker(spName);
	return(RegisterSpeakerKeyword(word,mask,curSp,ln));
}

static SPEAKERS *GetRightSpeaker(char *spName, MAPKEYS *mkey) {
	SPEAKERS *tsp;

	if (mkey->maps == NULL) {
		if ((mkey->maps=NEW(SPEAKERS)) == NULL)
			out_of_mem();
		tsp = mkey->maps;
	} else {
		tsp = mkey->maps;
		while (1) {
			if (!strcmp(spName,tsp->sp)) {
				tsp->count++;
				return(tsp);
			}
			if (tsp->nextspeaker == NULL)
				break;
			tsp = tsp->nextspeaker;
		}
		if ((tsp->nextspeaker=NEW(SPEAKERS)) == NULL)
			out_of_mem();
		tsp = tsp->nextspeaker;
	}
	tsp->sp = (char *)malloc((size_t)strlen(spName)+1);
	if (tsp->sp == NULL)
		out_of_mem();
	strcpy(tsp->sp, spName);
	tsp->count = 1;
	tsp->mapkey = NULL;
	tsp->nextspeaker = NULL;
	return(tsp);
}

static void StoreSpeakerMap(SPEAKERS *tmaps, char *word) {
	MAPKEYS *tkey, *tkey2, *new_key;

	if (tmaps->mapkey == NULL) {
		if ((tmaps->mapkey=NEW(MAPKEYS)) == NULL)
			out_of_mem();
		tkey = tmaps->mapkey;
		tkey->nextmapkey = NULL;
	} else {
		tkey = tmaps->mapkey;
		tkey2 = NULL;
		while (1) {
			if (!strcmp(word,tkey->mapkeyname)) {
				tkey->count++;
				return;
			}
			if (strcmp(word,tkey->mapkeyname) < 0) {
				if ((new_key=NEW(MAPKEYS)) == NULL)
					out_of_mem();
				new_key->nextmapkey = tkey;
				if (tkey2 == NULL)
					tmaps->mapkey = new_key;
				else
					tkey2->nextmapkey = new_key;
				tkey = new_key;
				break;
			}
			if (tkey->nextmapkey == NULL) {
				if ((tkey->nextmapkey=NEW(MAPKEYS)) == NULL)
					out_of_mem();
				tkey = tkey->nextmapkey;
				tkey->nextmapkey = NULL;
				break;
			}
			tkey2 = tkey;
			tkey = tkey->nextmapkey;
		}
	}
	tkey->mapkeyname = (char *)malloc((size_t)strlen(word)+1);
	if (tkey->mapkeyname == NULL)
		out_of_mem();
	strcpy(tkey->mapkeyname, word);
	tkey->count = 1;
	tkey->mask = NULL;
	tkey->maps = NULL;
}

static char addmap(SPEAKERS *lastspfound, long lastSpLn, char *spName, char *word, char *complSt) {
	int len;
	MAPKEYS *mkey;
	SPEAKERS *tmaps;

	for (mkey=lastspfound->mapkey; mkey != NULL; mkey = mkey->nextmapkey) {
		if (mkey->ln == lastSpLn && uS.mStricmp(mkey->fname, oldfname) == 0) {
			tmaps = GetRightSpeaker(spName, mkey);
			if (*mkey->mask) {
				uS.patmat(word, mkey->mask);
				uS.remblanks(word);
			}
			if (complSt[0] != EOS) {
				len = strlen(complSt);
				strcat(complSt, word);
				strcpy(word, complSt);
				complSt[len] = EOS;
			}
			StoreSpeakerMap(tmaps, word);
		}
	}
	return(0);
}


static void addExcelSp_KeywordRows(char *fnameOrg, char *sp, char *keyword, int count) {
	char *fname;
	KEYWDROW *r;

	if (rootexcel == NULL) {
		rootexcel = NEW(EXCELCOL);
		if (rootexcel == NULL)
			out_of_mem();
		rootexcel->keyword = NULL;
		rootexcel->keyrows = NEW(KEYWDROW);
		rootexcel->rows = NULL;
		rootexcel->nextcol = NULL;
		r = rootexcel->keyrows;
	} else {
		for (r=rootexcel->keyrows; r->nextrow != NULL; r=r->nextrow) ;
		r->nextrow = NEW(KEYWDROW);
		r = r->nextrow;
	}
	if (r == NULL)
		out_of_mem();
// ".xls"
	fname = strrchr(fnameOrg, PATHDELIMCHR);
	if (fname != NULL)
		fname = fname + 1;
	else
		fname = fnameOrg;
	r->fname = (char *)malloc((size_t) strlen(fname)+1);
	if (r->fname == NULL)
		out_of_mem();
	strcpy(r->fname, fname);
	r->sp = (char *)malloc((size_t) strlen(sp)+1);
	if (r->sp == NULL)
		out_of_mem();
	strcpy(r->sp, sp);
	r->keyword = (char *)malloc((size_t) strlen(keyword)+1);
	if (r->keyword == NULL)
		out_of_mem();
	strcpy(r->keyword, keyword);
	r->count = count;
	r->rowCnt = ExcelCnt;
	r->nextrow = NULL;
}

static EXCELROW *addExcelRows(EXCELROW *root, int count) {
	EXCELROW *p;

	if (root == NULL) {
		root = NEW(EXCELROW);
		p = root;
	} else {
		for (p=root; p->nextrow != NULL; p=p->nextrow) ;
		p->nextrow = NEW(EXCELROW);
		p = p->nextrow;
	}
	if (p == NULL)
		out_of_mem();
	p->count = count;
	p->rowCnt = ExcelCnt;
	p->nextrow = NULL;
	return(root);
}

static void addExcelCols_Rows(char *sp, char *keyword, int count) {
	EXCELCOL *tkey, *tkey2, *new_key;

//	if (sp[0] == '*')
//		sp++;
	strcpy(spareTier1, sp);
	strcat(spareTier1, ":");
	strcat(spareTier1, keyword);
	if (rootexcel->nextcol == NULL) {
		rootexcel->nextcol = NEW(EXCELCOL);
		if (rootexcel->nextcol == NULL)
			out_of_mem();
		tkey = rootexcel->nextcol;
		tkey->keyrows = NULL;
		tkey->rows = NULL;
		tkey->nextcol = NULL;
	} else {
		tkey = rootexcel->nextcol;
		tkey2 = NULL;
		while (1) {
			if (!strcmp(spareTier1, tkey->keyword)) {
				tkey->rows = addExcelRows(tkey->rows, count);
				return;
			}
			if (strcmp(spareTier1,tkey->keyword) < 0) {
				if ((new_key=NEW(EXCELCOL)) == NULL)
					out_of_mem();
				new_key->keyrows = NULL;
				new_key->rows = NULL;
				new_key->nextcol = tkey;
				if (tkey2 == NULL)
					rootexcel->nextcol = new_key;
				else
					tkey2->nextcol = new_key;
				tkey = new_key;
				break;
			}
			if (tkey->nextcol == NULL) {
				if ((tkey->nextcol=NEW(EXCELCOL)) == NULL)
					out_of_mem();
				tkey = tkey->nextcol;
				tkey->keyrows = NULL;
				tkey->rows = NULL;
				tkey->nextcol = NULL;
				break;
			}
			tkey2 = tkey;
			tkey = tkey->nextcol;
		}
	}
	tkey->keyword = (char *)malloc((size_t) strlen(spareTier1)+1);
	if (tkey->keyword == NULL)
		out_of_mem();
	strcpy(tkey->keyword, spareTier1);
	tkey->rows = addExcelRows(tkey->rows, count);
}

static void keymap_pr_result(void) {
	MAPKEYS  *key1, *Okey1, *key2, *Okey2;
	SPEAKERS *tsp1, *Otsp1, *tsp2, *Otsp2;

	for (tsp1=rootspeaker; tsp1 != NULL; ) {
		if (!isExcel)
			fprintf(fpout,"Speaker %s:\n", tsp1->sp);

		for (key1=tsp1->mapkey; key1 != NULL; ) {
			if (!isExcel)
				fprintf(fpout,"  Key word \"%s\" found %d time%s\n", key1->mapkeyname, key1->count, ((key1->count== 1) ? "": "s"));
			else
				addExcelSp_KeywordRows(oldfname, tsp1->sp, key1->mapkeyname, key1->count);
			for (tsp2=key1->maps; tsp2 != NULL; ) {
				if (!isExcel) {
					if (isPresedeCodes)
						fprintf(fpout,"      %d instances associated with speaker \"%s\", of these\n", tsp2->count, tsp2->sp);
					else
						fprintf(fpout,"      %d instances followed by speaker \"%s\", of these\n", tsp2->count, tsp2->sp);
				}
				for (key2=tsp2->mapkey; key2 != NULL; ) {
					if (!isExcel) {
						if (isPresedeCodes) {
							if (strrchr(key2->mapkeyname, '-') != NULL)
								fprintf(fpout,"\tcode \"%s\" precedes %d time%s\n", key2->mapkeyname, key2->count, ((key2->count == 1) ? "": "s"));
							else
								fprintf(fpout,"\tcode \"%s\" follows %d time%s\n", key2->mapkeyname, key2->count, ((key2->count == 1) ? "": "s"));
						} else
							fprintf(fpout,"\tcode \"%s\" maps %d time%s\n", key2->mapkeyname, key2->count, ((key2->count == 1) ? "": "s"));
					} else
						addExcelCols_Rows(tsp2->sp, key2->mapkeyname, key2->count);
					Okey2 = key2;
					key2 = key2->nextmapkey;
					if (Okey2->mapkeyname != NULL)
						free(Okey2->mapkeyname);
					free(Okey2); 
				}
				Otsp2 = tsp2;
				tsp2 = tsp2->nextspeaker;
				free(Otsp2);
			}
			Okey1 = key1;
			key1 = key1->nextmapkey;
			if (Okey1->mapkeyname != NULL)
				free(Okey1->mapkeyname);
			free(Okey1); 
			ExcelCnt++;
		}
		Otsp1 = tsp1;
		tsp1 = tsp1->nextspeaker;
		free(Otsp1);
	}
}

static void cleanUpComplSt(char *st) {
	while (*st != EOS) {
		if (*st == '[' || *st == ']' || *st == '+' || isSpace(*st)) {
			strcpy(st, st+1);
		} else
			st++;
	}
}

static int getNextWord(char *cleanLine, char *orgWord, int i) {
	int  j;

	for (; isSpace(cleanLine[i]); i++) ;
	j = 0;
	for (; cleanLine[i] != EOS && !isSpace(cleanLine[i]); i++) {
		orgWord[j++] = cleanLine[i];
	}
	orgWord[j] = EOS;
	if (orgWord[0] == EOS)
		return(0);
	if (cleanLine[i] != EOS) {
		i++;
	}
	return(i);
}

void call() {
	register int i, j;
	char *mask, *maskCompl;
	char word[BUFSIZ+1], fullWord[BUFSIZ+SPEAKERLEN+1];
	char spName[SPEAKERLEN], lastSpName[SPEAKERLEN];
	char depName[SPEAKERLEN], complSt[BUFSIZ+2], num[64];
	SPEAKERS *tCS, *curSp, *lastSp, *lastDiffSp;
//	char CodeTierFound;
	long ln = 0L, curSpLn = 0L, lastSpLn = 0L, lastDiffSpLn = 0L, removedCnt = 0L;

	curSp = NULL;
	lastSp = NULL;
	lastDiffSp = NULL;
	spName[0] = EOS;
	lastSpName[0] = EOS;
	complSt[0] = EOS;
	spareTier2[0] = EOS;
	spareTier3[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
if (checktier(utterance->speaker)) {
	printf("sp=%s; uttline=%s", utterance->speaker, uttline);
	if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
}
*/
		if (*utterance->speaker == '@') ;
		else if (*utterance->speaker == '*') {
			ln++;
			i = strlen(utterance->speaker);
			for (; utterance->speaker[i] != ':'; i--) ;
			utterance->speaker[i] = EOS;
			uS.uppercasestr(utterance->speaker, &dFnt, MBF);
			strcpy(lastSpName,spName);
			strcpy(spName,utterance->speaker);
			lastSp = curSp;
			lastSpLn = curSpLn;
			if (lastSpName[0] != EOS && spName[0] != EOS && strcmp(lastSpName, spName) != 0) {
				lastDiffSp = lastSp;
				lastDiffSpLn = lastSpLn;
				removedCnt = 0L;
			} else
				removedCnt++;
			curSp = NULL;
			curSpLn = 0L;
			if (lastSp == NULL)
				complSt[0] = EOS;
			if (isPresedeCodes) {
				strcpy(spareTier3, spareTier2);
				spareTier2[0] = EOS;
			}
			if (rootcompkey != NULL) {
				i = 0;
				while ((i=getword(utterance->speaker, utterance->line, word, NULL, i))) {
					if ((maskCompl=keymap_inthere(word,rootcompkey)) != NULL) {
						cleanUpComplSt(word);
						strcat(complSt, word);
						strcat(complSt, "-");
					}
				}
			}
		} else if (checktier(utterance->speaker)) {/* right tier line found */
			i = strlen(utterance->speaker);
			for (; utterance->speaker[i] != ':'; i--) ;
			utterance->speaker[i] = EOS;
			uS.lowercasestr(utterance->speaker, &dFnt, MBF);
			strcpy(depName,utterance->speaker);
			if (isPresedeCodes) {
				i = 0;
				while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
					if (*word == CODECHAR) {
						if (exclude(word)) {
							if (keymap_CodeTierGiven == '\002') {
								strcpy(fullWord, depName);
								strcat(fullWord, word);
							} else
								strcpy(fullWord, word);
							strcat(fullWord, "-1");
							if (spareTier2[0] != EOS)
								strcat(spareTier2, " ");
							strcat(spareTier2, fullWord);
						}
					}
				}
			}
			if (lastSp != NULL) {
				i = 0;
				while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
					if (*word == CODECHAR) {
						if (exclude(word)) {
							if (keymap_CodeTierGiven == '\002') {
								strcpy(fullWord, depName);
								strcat(fullWord, word);
							} else
								strcpy(fullWord, word);
							addmap(lastSp, lastSpLn, spName, fullWord, complSt);
						}
					}
				}
				complSt[0] = EOS;
			}
			if (lastDiffSp != NULL && lastSp != lastDiffSp) {
				i = 0;
				while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
					if (*word == CODECHAR) {
						if (exclude(word)) {
							if (keymap_CodeTierGiven == '\002') {
								strcpy(fullWord, depName);
								strcat(fullWord, word);
							} else
								strcpy(fullWord, word);
							if (removedCnt > 0) {
								sprintf(num, "+%ld", removedCnt);
								strcat(fullWord, num);
							}
							addmap(lastDiffSp, lastDiffSpLn, spName, fullWord, complSt);
						}
					}
				}
				complSt[0] = EOS;
			}
			if (checktier(spName)) {
				i = 0;
				while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
					if (*word == CODECHAR) {
						if ((mask=keymap_inthere(word,rootkey)) != NULL) {
							if (keymap_CodeTierGiven == '\002') {
								strcpy(fullWord, depName);
								strcat(fullWord, word);
							} else
								strcpy(fullWord, word);
							tCS = RegisterKeyword(fullWord, mask, spName, ln, curSp);
							if (isPresedeCodes && tCS != NULL && spareTier3[0] != EOS) {
								word[0] = EOS;
								j = 0;
								while ((j=getNextWord(spareTier3, fullWord, j))) {
									addmap(tCS, ln, lastSpName, fullWord, word);
								}
							}
							if (curSp == NULL) {
								curSp = tCS;
								curSpLn = ln;
							}
						}
					}
				}
			}
		}
	}
	if (!combinput)
		keymap_pr_result();
}

void getflag(char *f, char *f1, int *i) {
	char wd[1024+2];

	f++;
	switch(*f++) {
		case 'b':
			strncpy(wd, f, 1024);
			wd[1024] = EOS;
			uS.remFrontAndBackBlanks(wd);
			if (wd[0] == '@') {
				getFileCodes('b', wd+1);
			} else {
				SetUpKeyWords(getfarg(f,f1,i));
			}
			break;
		case 'c':
			strncpy(wd, f, 1024);
			wd[1024] = EOS;
			uS.remFrontAndBackBlanks(wd);
			if (wd[0] == '@') {
				getFileCodes('c', wd+1);
			} else {
				SetUpCompKeyWords(getfarg(f,f1,i));
			}
			break;
		case 'd':
			no_arg_option(f);
			isExcel = TRUE;
			break;
		case 'o':
			no_arg_option(f);
			isPresedeCodes = TRUE;
			break;
		case 't':
				if (*f == '%') {
					if (keymap_CodeTierGiven == '\0')
				  		keymap_CodeTierGiven = '\001';
					else
						keymap_CodeTierGiven = '\002';
				}
				maingetflag(f-2,f1,i);
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}

static void keymap_cleanup_key(KEYWORDS *p) {
	KEYWORDS *t;

	while (p != NULL) {
		t = p;
		p = p->nextkeyword;
		if (t->keywordname)
			free(t->keywordname);
		if (t->mask)
			free(t->mask);
		free(t);
	}
}

static void keymap_cleanup_excel(EXCELCOL *p) {
	EXCELCOL *t;

	while (p != NULL) {
		t = p;
		p = p->nextcol;
		if (t->keyword)
			free(t->keyword);
		free(t);
	}
}

static void pr_Excel(FILE *StatFpOut, EXCELCOL *cols, char isCombinput) {
	int rowCount;
	float fcount;
	EXCELCOL *col;
	KEYWDROW *twdr;
	EXCELROW *tr;

	if (cols == NULL)
		return;

	excelRow(StatFpOut, ExcelRowStart);
	fprintf(StatFpOut, "%c%c%c", 0xef, 0xbb, 0xbf); // BOM UTF8
	excelCommasStrCell(StatFpOut, "file name,speaker,keyword,count");
	for (col=cols->nextcol; col != NULL; col=col->nextcol) {
		excelCommasStrCell(StatFpOut, col->keyword);
	}
	excelRow(StatFpOut, ExcelRowEnd);
	for (rowCount=0; rowCount < ExcelCnt; rowCount++) {
		excelRow(StatFpOut, ExcelRowStart);
		col = cols;
		if (col->keyrows != NULL && col->keyrows->rowCnt == rowCount) {
			if (isCombinput)
				excelStrCell(StatFpOut, "Combined Files");
			else
				excelStrCell(StatFpOut, col->keyrows->fname);
			excelStrCell(StatFpOut, col->keyrows->sp);
			excelStrCell(StatFpOut, col->keyrows->keyword);
			fcount = (float)col->keyrows->count;
			excelNumCell(StatFpOut, "%.0f", fcount);
			twdr = col->keyrows;
			col->keyrows = col->keyrows->nextrow;
			if (twdr->fname != NULL)
				free(twdr->fname);
			if (twdr->sp != NULL)
				free(twdr->sp);
			if (twdr->keyword != NULL)
				free(twdr->keyword);
			free(twdr);
		}
		for (col=col->nextcol; col != NULL; col=col->nextcol) {
			if (col->rows != NULL && col->rows->rowCnt == rowCount) {
				fcount = (float)col->rows->count;
				excelNumCell(StatFpOut, "%.0f", fcount);
				tr = col->rows;
				col->rows = col->rows->nextrow;
				free(tr);
			} else
				excelStrCell(StatFpOut, "0");
		}
		excelRow(StatFpOut, ExcelRowEnd);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	char tCombinput;
	FNType *fn;
	FNType StatName[FNSize];
	FILE *StatFpOut = NULL;

	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = KEYMAP;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv, keymap_pr_result);
	keymap_cleanup_key(rootkey);
	keymap_cleanup_key(rootcompkey);
	if (rootexcel != NULL) {
		AddCEXExtension = ".xls";
		if (Preserve_dir)
			strcpy(StatName, wd_dir);
		else
			strcpy(StatName, od_dir);
		addFilename2Path(StatName, "stat.keymap");
		strcat(StatName, GExt);
		parsfname(StatName, FileName1, "");
		tCombinput = combinput;
		combinput = FALSE;
		if ((StatFpOut=openwfile(StatName, FileName1, NULL)) == NULL) {
			fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",FileName1);
			keymap_cleanup_excel(rootexcel);
		} else {
			if (Preserve_dir) {
				fn = strrchr(FileName1, PATHDELIMCHR);
				if (fn == NULL)
					fn = FileName1;
				else
					fn++;
			} else
				fn = FileName1;
			excelHeader(StatFpOut, fn, 95);
			pr_Excel(StatFpOut, rootexcel, tCombinput);
			excelFooter(StatFpOut);
			fclose(StatFpOut);
			if (!stin_override)
				fprintf(stderr,"Output file <%s>\n", fn);
			else
				fprintf(stderr,"Output file \"%s\"\n", fn);
			keymap_cleanup_excel(rootexcel);
		}
		combinput = tCombinput;
	}
}
