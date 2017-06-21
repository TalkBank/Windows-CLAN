/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/*
input:

*CHI: the cow has, came <has come> [//] to; the well to drink
*CHI: but there is no eh@fp <there was> [//] no: water there
*CHI: just a test


kwal t.cha +s"<//>" +d3 +s"+[//]"
kwal t.cha +s<//> +d3 +s+[//] +d
kwal t.cha +s<//> +d3 +d
kwal t.cha +d3 +s[//] +d


To filter out </>, <//>, <///>, </-> and </?>

kwal +o@ +o% +r2 +r4 +r6 +r7 +d3 +d t.cha
*/
#define CHAT_MODE 1

#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main kwal_main
#define call kwal_call
#define getflag kwal_getflag
#define init kwal_init
#define usage kwal_usage
#endif

#define IS_WIN_MODE TRUE
#include "mul.h"

#define MAXKEYS 35

struct utts_list {
	unsigned char keys[MAXKEYS];
	char *line;
	FNType *fname;
	long lineno;
	struct utts_list *NextUtt;
} ;

struct key {
	char *keyword;
	struct utts_list *utts;
	struct key *left;
	struct key *right;
} ;

#define KWALSP struct kwal_speakers_list
struct kwal_speakers_list {
	char *fname;
	char *sp;
	char *ID;
	KWALSP *next_sp;
} ;

static char uttword[SPEAKERLEN];
static char keyword[SPEAKERLEN];
static char kw, kwal_sort, outputOnlyMatched, isDuplicateTiers, isKeywordOneColumn;
static char *kwal_total_line;
static char kwalTSpeaker[SPEAKERLEN];
static char kwalTSInclude;
static char kwal_isOnlyOne;
static char isDepTierIncluded;
static char isBlankSpeakerName;
static char kwal_ftime, kwal_ftime2, kwal_ftime3;
static int  kwal_total_line_size;
static long kwal_tlineno;
static KWALSP *kwal_head;
static struct key *rootKey;
static struct utts_list *rootUtts;
static char *keywordsList[MAXKEYS];

extern int  aftwin;
extern int  IsModUttDel;
extern char tch;
extern char tct;
extern char R5;
extern char R7Slash;
extern char R7Tilda;
extern char R7Caret;
extern char R7Colon;
extern char isRemoveCAChar[];
extern char Parans;
extern char isExpendX;
extern char OverWriteFile;
extern char PostCodeMode;
extern char IncludeAllDefaultCodes;
extern struct tier *headtier, *HeadOutTier;
extern UTTER *lutter;

void usage() {			/* print usage and synopsis of options */
	puts("KWAL- Key Word And Line concordance program searches data for key words");
	puts("      and produces the keywords in the context.");
	printf("Usage: kwal [a b n %s] filename(s)\n",mainflgs());
	puts("+a : display output in alphabetical order of keywords, it will repeat the same tier for every keyword");
	puts("+b : apply match of +s ONLY if it is the only one item on the tier");
	puts("+nS: include all utterances from Speaker S when they follow a match for +s");
	puts("-nS: exclude all utterances from Speaker S when they follow a match for +s");
	mainusage(TRUE);
}

void init(char t) {
	register int i;
	struct tier *p;

	if (t) {
		isExpendX = FALSE;
		kwalTSpeaker[0] = EOS;
		kwalTSInclude = TRUE;
		isBlankSpeakerName = TRUE;
		kwal_isOnlyOne = FALSE;
		kwal_sort = FALSE;
		kwal_total_line = NULL;
		kwal_head = NULL;
		isDepTierIncluded = FALSE;
		kwal_ftime = TRUE;
		kwal_ftime2 = TRUE;
		kwal_ftime3 = TRUE;
		isDuplicateTiers = FALSE;
		isKeywordOneColumn = FALSE;
		if (outputOnlyMatched < 2) {
			maininitwords();
			mor_initwords();
		} else {
			OverWriteFile = TRUE;
		}
		for (i=0; GlobalPunctuation[i]; ) {
			if (GlobalPunctuation[i] == '!' ||
				GlobalPunctuation[i] == '?' ||
				GlobalPunctuation[i] == '.') 
				strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
			else
				i++;
		}
	} else {
		if (kwal_ftime2) {
			kwal_ftime2 = FALSE;
			if (isMorSearchListGiven() && chatmode)
				maketierchoice("%mor",'+',FALSE);
			if (onlydata == 5 && !f_override)
				stout = FALSE;
			if (kwal_sort)
				isKeywordOneColumn = FALSE;
		}
		for (p=headtier; p != NULL; p=p->nexttier) {
			if (p->tcode[0] == '%') {
				isDepTierIncluded = TRUE;
			}
		}
		if (utterance->nextutt == utterance) {
			utterance->speaker[0] = '*';
			utterance->speaker[1] = EOS;
		}
		if (kwal_isOnlyOne)
			FilterTier = 1;
		if (kwal_ftime3) {
			kwal_ftime3 = FALSE;
			if (onlydata == 5) {
				maketierchoice("@ID:",'+',FALSE);
				AddCEXExtension = ".xls";
			}
		}
	}
	if (!combinput || t) {
		rootKey = NULL;
		rootUtts = NULL;
		for (i=0; i < MAXKEYS; i++)
			keywordsList[i] = NULL;
	}
}

static void kwal_FindSpeaker(char *sp, char *ID) {
	KWALSP *ts;

	uS.remblanks(sp);
	if (kwal_head == NULL) {
		kwal_head = NEW(KWALSP);
		ts = kwal_head;
	} else {
		for (ts=kwal_head; 1; ts=ts->next_sp) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE) && uS.mStricmp(ts->fname, oldfname) == 0) {
				return;
			}
			if (ts->next_sp == NULL)
				break;
		}
		ts->next_sp = NEW(KWALSP);
		ts = ts->next_sp;
	}
	if (ts == NULL)
		out_of_mem();
	if ((ts->fname=(char *)malloc(strlen(oldfname)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(ts->fname, oldfname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(ts->sp, sp);
	if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL) {
		out_of_mem();
	}
	strcpy(ts->ID, ID);
	ts->next_sp = NULL;
}

static KWALSP *kwal_freeSpeakers(KWALSP *p) {
	KWALSP *t;

	while (p != NULL) {
		t = p;
		p = p->next_sp;
		if (t->fname != NULL)
			free(t->fname);
		if (t->sp != NULL)
			free(t->sp);
		if (t->ID)
			free(t->ID);
		free(t);
	}
	return(NULL);
}

static void kwal_outputIDInfo(FILE *fp, char *sp, char *fname) {
	int  cnt;
	char *s, isBlank;
	KWALSP *ts;

	s = strrchr(fname, PATHDELIMCHR);
	if (s == NULL)
		s = fname;
	else
		s++;
	fprintf(fp, "%s\t", s);
	for (ts=kwal_head; ts != NULL; ts=ts->next_sp) {
		if (uS.partcmp(ts->sp, sp, FALSE, FALSE) && uS.mStricmp(ts->fname, fname) == 0) {			
			cnt = 0;
			isBlank = TRUE;
			for (s=ts->ID; *s != EOS; s++) {
				if (*s == '|') {
					if (isBlank)
						fprintf(fp,".");
					fprintf(fp,"\t");
					isBlank = TRUE;
					cnt++;
				} else if (isSpace(*s)) {
					if (isBlank) {
						fprintf(fp,".");
						isBlank = FALSE;
					} else
						fputc(*s, fp);
				} else {
					isBlank = FALSE;
					fputc(*s, fp);
				}
			}
			if (cnt < 10)
				fprintf(fp,".\t");
			return;
		}
	}
	fprintf(fp,".\t.\t%s\t.\t.\t.\t.\t.\t.\t.\t", sp);
}

static unsigned char addToKeysList(char *keyword) {
	int i;

	for (i=0; i < MAXKEYS; i++) {
		if (keywordsList[i] == NULL) {
			if ((keywordsList[i]=(char *)malloc(strlen(keyword)+1)) == NULL) {
				out_of_mem();
			}
			strcpy(keywordsList[i], keyword);
			break;
		} else {
			if (uS.mStricmp(keywordsList[i], keyword) == 0)
				return((unsigned char)i);
		}
	}
	return((unsigned char)i);
}

static struct utts_list *AddLinearUtts(struct utts_list *root, long lineno, char *keyword, char *line) {
	int  i;
	char *b, *e, t;
	struct utts_list *u;

	if (root == NULL) {
		root = NEW(struct utts_list);
		if (root == NULL)
			out_of_mem();
		u = root;
	} else {
		for (u=root; u->NextUtt != NULL; u=u->NextUtt) ;
		u->NextUtt = NEW(struct utts_list);
		if (u->NextUtt == NULL) out_of_mem();
		u = u->NextUtt;
	}
	u->NextUtt = NULL;
	u->lineno = lineno;
	i = 0;
	b = keyword;
	while (isSpace(*b))
		b++;
	do {
		for (e=b; *e != EOS && *e != ','; e++) ;
		t = *e;
		*e = EOS;
		if (*b != EOS && i < MAXKEYS)
			u->keys[i++] = addToKeysList(b);
		*e = t;
		if (t == EOS)
			b = e;
		else
			b = e + 1;
		while (isSpace(*b))
			b++;
	} while (*b != EOS) ;
	for (; i < MAXKEYS; i++)
		u->keys[i] = MAXKEYS;
	u->line = (char *)malloc(strlen(line)+1);
	if (u->line == NULL)
		out_of_mem();
	strcpy(u->line, line);
	u->fname = (FNType *)malloc((strlen(oldfname)+1)*sizeof(FNType));
	if (u->fname == NULL)
		out_of_mem();
	strcpy(u->fname, oldfname);
	return(root);
}

static void AddUtt(struct key *p) {
	int i;
	struct utts_list *u;

	if ((u=p->utts) == NULL) {
		p->utts = NEW(struct utts_list);
		if (p->utts == NULL) out_of_mem();
		u = p->utts;
	} else {
		while (u->NextUtt != NULL)
			u = u->NextUtt;
		u->NextUtt = NEW(struct utts_list);
		if (u->NextUtt == NULL) out_of_mem();
		u = u->NextUtt;
	}
	u->NextUtt = NULL;
	for (i=0; i < MAXKEYS; i++)
		u->keys[i] = MAXKEYS;
	u->lineno = kwal_tlineno;
	u->line = (char *)malloc(strlen(kwal_total_line)+1);
	if (u->line == NULL)
		out_of_mem();
	strcpy(u->line, kwal_total_line);
	u->fname = (FNType *)malloc((strlen(oldfname)+1)*sizeof(FNType));
	if (u->fname == NULL)
		out_of_mem();
	strcpy(u->fname, oldfname);
}

static struct key *AddKeyword(struct key *p, char *w) {
	int cond;

	if (p == NULL) {
		if ((p=NEW(struct key)) == NULL) out_of_mem();
		p->keyword = (char *)malloc(strlen(w)+1);
		if (p->keyword == NULL) out_of_mem();
		strcpy(p->keyword, w);
		p->utts = NULL;
		AddUtt(p);
		p->left = p->right = NULL;
	} else if ((cond = strcmp(w, p->keyword)) == 0) AddUtt(p);
	else if (cond < 0) p->left = AddKeyword(p->left, w);
	else p->right = AddKeyword(p->right, w);
	return(p);
}

static void PrintUtt(struct utts_list *u, char *keyword) {
	int  i, j;
	struct utts_list *t;

	while (u != NULL) {
		if (onlydata == 0) {
			fputs("-------------------------------------\n",fpout);
			fprintf(fpout,"*** File \"%s\": line %ld. Keyword: %s\n", 
				u->fname, u->lineno, keyword);
		} else if (onlydata == 2) {
			fputs("@Comment:\t-------------------------------------\n",fpout);
			fprintf(fpout,"@Comment:\t*** File \"%s\": line %ld. Keyword: %s\n", 
				u->fname, u->lineno, keyword);
		}
		if (onlydata == 5) {
			if (u->line[0] == '*') {
				for (i=0L; u->line[i] != EOS && u->line[i] != ':'; i++)
					spareTier1[i] = u->line[i];
				if (u->line[i] == ':')
					spareTier1[i] = EOS;
				else
					strcpy(spareTier1, "NO_SPEAKER_NAME");
			} else
				strcpy(spareTier1, "NO_SPEAKER_NAME");
			kwal_outputIDInfo(fpout, spareTier1, u->fname);
			fprintf(fpout, "%ld\t", u->lineno);
			if (isKeywordOneColumn) {
				for (i=0; i < MAXKEYS; i++) {
					if (keywordsList[i] == NULL)
						break;
					for (j=0; j < MAXKEYS; j++) {
						if (u->keys[j] == MAXKEYS)
							break;
						if (i == u->keys[j]) {
							fprintf(fpout, "%s\t", keywordsList[i]);
							break;
						}
					}
					if (j >= MAXKEYS || u->keys[j] == MAXKEYS)
						fprintf(fpout, "\t");
				}
			} else if (keyword[0] != EOS)
				fprintf(fpout, "%s\t", keyword);
		}
		if (nomain) {
			remove_main_tier_print("", u->line, NULL);
		} else if (onlydata == 5) {
			remove_CRs_Tabs(u->line);
			fprintf(fpout, "%s\n", u->line);
		} else
			fputs(u->line,fpout);
		t = u;
		u = u->NextUtt;
		free(t->line);
		free(t->fname);
		free(t);
	}
}

static void PrintKeywords(struct key *p) {
	if (p != NULL) {
		PrintKeywords(p->left);
		PrintUtt(p->utts, p->keyword);
		PrintKeywords(p->right);
		free(p->keyword);
		free(p);
	}
}

static void AddToTotalLine(char *s) {
	register int len;
	
	if ((len=strlen(s)) >= kwal_total_line_size) {
		if (kwal_total_line != NULL) {
			char *t;

			kwal_total_line_size = strlen(kwal_total_line) + len;
			t = kwal_total_line;
			kwal_total_line = (char *)malloc((size_t)kwal_total_line_size+1);
			if (kwal_total_line == NULL) out_of_mem();
			strcpy(kwal_total_line, t);
			kwal_total_line_size -= strlen(kwal_total_line);
			free(t);
		} else {
			kwal_total_line_size = len;
			kwal_total_line = (char *)malloc((size_t)kwal_total_line_size+1);
			if (kwal_total_line == NULL) out_of_mem();
			*kwal_total_line = EOS;
		}
	}
	kwal_total_line_size -= len;
	strcat(kwal_total_line, s);
}

static void lbefprintout(void) {
	UTTER *temp;
	int w;

	w = 0;
	if (lutter != utterance) {
		temp = utterance;
		do {
			temp = temp->nextutt;
			w++;
		} while (temp != lutter) ;
	}
	for (temp=lutter->nextutt; w < aftwin; w++) temp = temp->nextutt;
	for (; temp != utterance; temp=temp->nextutt) {
		if (chatmode && *temp->speaker) {
			AddToTotalLine(temp->speaker);
			AddToTotalLine(temp->line);
		} else if (!chatmode && *temp->line)
			AddToTotalLine(temp->line);
	}
}

static void laftprintout() {
	UTTER *temp, *oldlutter;
	int w;

	w = 0;
	if (lutter != utterance) {
		temp = utterance;
		do {
			temp = temp->nextutt;
			w++;
			if (temp->speaker) {
				AddToTotalLine(temp->speaker);
				AddToTotalLine(temp->line);
			}
		} while (temp != lutter) ;
	}
	for (; w < aftwin; w++) {
		oldlutter = lutter;
		lutter = lutter->nextutt;
		if (chatmode) {
			if (!getmaincode()) {
				lutter = oldlutter;
				return;
			}
		} else {
			if (!gettextspeaker()) {
				lutter = oldlutter;
				return;
			}
			lutter = lutter->nextutt;
			IsModUttDel = chatmode < 3;
			cutt_getline("*", lutter->line, lutter->attLine, 0);
			strcpy(lutter->tuttline,lutter->line);
			if (FilterTier > 0)
				filterData("*",lutter->tuttline);
		}
		lutter->tlineno = lineno;
		lutter->uttLen = 0L;
		AddToTotalLine(lutter->speaker);
		AddToTotalLine(lutter->line);
	}
}

static void kwal_pr_result(void) {
	int i;

	if (isKeywordOneColumn) {
		PrintUtt(rootUtts, NULL);
		for (i=0; i < MAXKEYS; i++) {
			if (keywordsList[i] != NULL) {
				free(keywordsList[i]);
				keywordsList[i] = NULL;
			}
		}
	} else if (kwal_sort)
		PrintKeywords(rootKey);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int i;

	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = KWAL;
	chatmode = CHAT_MODE;
	OnlydataLimit = 5;
	UttlineEqUtterance = FALSE;
	outputOnlyMatched = 0;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+' || *argv[i] == '-') {
			if (argv[i][1] == 'd' && argv[i][2] == '3' && argv[i][3] == '0') {
				outputOnlyMatched = 2;
				break;
			}
		}
	}
	kwal_head = NULL;
	kwal_total_line = NULL;
	for (i=0; i < MAXKEYS; i++)
		keywordsList[i] = NULL;
	bmain(argc,argv,kwal_pr_result);
	kwal_head = kwal_freeSpeakers(kwal_head);
	if (kwal_total_line != NULL)
		free(kwal_total_line);
	for (i=0; i < MAXKEYS; i++) {
		if (keywordsList[i] != NULL) {
			free(keywordsList[i]);
		}
	}	
}

static void pr_idfld(char *wd, long lineno) {
	if (*oldfname == EOS) fprintf(fpout,"File \"%s\": ", "Stdin");
	else fprintf(fpout,"File \"%s\": ", oldfname);
	fprintf(fpout,"line %ld. ", lineno);
	if (*wd && (WordMode == 'i' || WordMode == 'I'))
		fprintf(fpout,"Keyword%s: %s ",((kw) ? "s" : ""),wd);
	putc('\n',fpout);
}

static char excludeutter(char isCheckNomain, char *isKeywordChecked) {
	register char found;
	register int i;
	char otherFound, oldFound;

	if (onlydata && *utterance->speaker == '@') {
		if ((utterance->speaker[1] == 'B' || utterance->speaker[1] == 'b') && 
			(utterance->speaker[2] == 'E' || utterance->speaker[2] == 'e') && 
			(utterance->speaker[3] == 'G' || utterance->speaker[3] == 'g') &&
			(utterance->speaker[4] == 'I' || utterance->speaker[4] == 'i') && 
			(utterance->speaker[5] == 'N' || utterance->speaker[5] == 'n'))
			return(FALSE);
		if ((utterance->speaker[1] == 'E' || utterance->speaker[1] == 'e') && 
			(utterance->speaker[2] == 'N' || utterance->speaker[2] == 'n') && 
			(utterance->speaker[3] == 'D' || utterance->speaker[3] == 'd'))
			return(FALSE);
	}
	if (!checktier(utterance->speaker))
		return(TRUE);
	if (isCheckNomain && nomain && utterance->speaker[0] == '*')
		return(TRUE);

	if (chatmode && uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE))
		filterwords("*",uttline,exclude);

	i = 0;
	while (uttline[i]) {
		if (uttline[i] == '\n') {
			if (uttline[i+1]=='*' || (uttline[i+1]=='%' && isBlankSpeakerName) || uttline[i+1]=='@') {
				for (; uttline[i] != ':' && uttline[i]; i++)
					uttline[i] = ' ';
				if (uttline[i] == ':')
					uttline[i++] = ' ';
			} else if (uS.isskip(uttline,i,&dFnt,MBF))
				uttline[i++] = ' ';
			else
				i++;
		} else if (uS.isskip(uttline,i,&dFnt,MBF) &&
				((outputOnlyMatched == 0 && WordMode != 'i' && WordMode != 'I') ||
					(uttline[i]!='[' && uttline[i]!=']' && uttline[i]!='<' && uttline[i]!='>')))
			uttline[i++] = ' ';
		else
			i++;
	}
	i = 0;
	if (WordMode == 'i' || WordMode == 'I')
		found = FALSE;
	else
		found = TRUE;
	otherFound = FALSE;
	oldFound = found;
	while ((i=getword(utterance->speaker, uttline, uttword, NULL, i))) {
		if (WordMode == 'i' || WordMode == 'I') {
			if (kwal_sort) {
				if (*isKeywordChecked == FALSE) {
					kwal_tlineno = utterance->tlineno;
					if (!kw) {
						if (kwal_total_line != NULL)
							free(kwal_total_line);
						kwal_total_line = NULL;
						kwal_total_line_size = 0;
						lbefprintout();
						AddToTotalLine(utterance->speaker);
						AddToTotalLine(utterance->line);
						laftprintout();
					}
					kw = 1;
					rootKey = AddKeyword(rootKey, uttword);
				}
			} else if (kwal_isOnlyOne) {
				if (exclude(uttword))
					found = TRUE;
				else if (!uS.IsUtteranceDel(uttword, 0) && uttword[0] != '+' && uttword[0] != '-')
					otherFound = TRUE;
			} else {
				found = TRUE;
				if (*keyword) {
					strcat(keyword,", ");
					kw = 1;
				}
				strcat(keyword,uttword);
			}
		} else if (outputOnlyMatched) {
			if (!exclude(uttword)) {
				int k=i;
				if (k >= 0) {
					k--;
					while (k >= 0 && uS.isskip(uttline,k,&dFnt,MBF))
						k--;
					while (k >= 0 && !uS.isskip(uttline,k,&dFnt,MBF)) {
						uttline[k] = ' ';
						k--;
					}
				}
			}
		} else if (!exclude(uttword)) {
			found = FALSE;
			if (!kwal_isOnlyOne)
				break;
		} else if (!uS.IsUtteranceDel(uttword, 0) && uttword[0] != '+' && uttword[0] != '-')
			otherFound = TRUE;
	}
	if (kwal_sort && *isKeywordChecked == FALSE && (WordMode == 'i' || WordMode == 'I')) 
		*isKeywordChecked = TRUE;
	if (kwal_isOnlyOne && otherFound)
		found = oldFound;

	if (CntWUT == 2) {
		if (!found && wdptr == NULL && mwdptr == NULL && !isMorSearchListGiven() && !isLanguageSearchListGiven())
			WUCounter--;
	}
	return(!found);
}

static int FullLine(char *s, char isCheckNomain) {
	long i;
	char dot[3], exl[3], qtm[3];

	if (isCheckNomain && nomain && utterance->speaker[0] == '*' && onlydata != 5)
		return(FALSE);

	if (WordMode == '\0' && tch == FALSE && PostCodeMode == '\0' && outputOnlyMatched == 0 && onlydata != 5)
		return(TRUE);

	strcpy(dot, ".");
	strcpy(exl, "!");
	strcpy(qtm, "?");
	for (i=0L; s[i]; i++) {
		if (s[i] == '%' && i > 0 && s[i-1] == '\n') {
			while (s[i] != ':' && s[i] != EOS)
				i++;
			if (s[i] == EOS)
				i--;
		} else if (s[i] == '.' || s[i] == '!' || s[i] == '?') {
			if ((s[i] == '.' && exclude(dot)) || (s[i] == '!' && exclude(exl)) || (s[i] == '?' && exclude(qtm)))
				return(TRUE);
		} else if (!isSpace(s[i]) && s[i] != '\n')
			return(TRUE);
	}
	if (*utterance->speaker == '@') {
			return(TRUE);
	}
	return(FALSE);
}

static void cleanUpOutput(void) {
	int  i, j, k;
	char isSpaceFound, isCopy, isDepTierFound;

	isCopy = TRUE;
	isSpaceFound = TRUE;
	isDepTierFound = FALSE;
	for (i=0L; utterance->line[i]; i++) {
		if (utterance->line[i] == '\n' && utterance->line[i+1] == '%') {
			isCopy = TRUE;
			isDepTierFound = TRUE;
		}
		if (isDepTierFound == FALSE && Parans == 1 && (utterance->line[i] == '(' || utterance->line[i] == ')'))
			isCopy = FALSE;
		if (!uS.isskip(uttline,i,&dFnt,MBF)) {
			if (isCopy)
				uttline[i] = utterance->line[i];
		} else
			isCopy = TRUE;
	}
	for (i=0L,j=0L; utterance->line[i]; i++) {
		if (utterance->line[i] == '\n' && utterance->line[i+1] == '%') {
			uttline[j] = utterance->line[i];
			utterance->attLine[j] = utterance->attLine[i];
			j++;
			i++;
			for (k=0; utterance->line[i] != ':' && utterance->line[i] != '\n' && utterance->line[i] != EOS; i++) {
				templineC3[k++] = utterance->line[i];
				uttline[j] = utterance->line[i];
				utterance->attLine[j] = utterance->attLine[i];
				j++;
			}
			if (utterance->line[i] == ':') {
				templineC3[k++] = ':';
				uttline[j] = utterance->line[i];
				utterance->attLine[j] = utterance->attLine[i];
				j++;
				i++;
				if (isSpace(utterance->line[i])) {
					uttline[j] = utterance->line[i];
					utterance->attLine[j] = utterance->attLine[i];
					j++;
					i++;
					isSpaceFound = TRUE;
				}
			}
			templineC3[k] = EOS;
			uS.uppercasestr(templineC3, &dFnt, MBF);
			if (CheckOutTier(templineC3)) {
				for (; utterance->line[i] != EOS; i++) {
					if (utterance->line[i] == '\n' && (utterance->line[i+1] == '%' ||
													   utterance->line[i+1] == EOS)) {
						isSpaceFound = FALSE;
						i--;
						break;
					}
					uttline[j] = utterance->line[i];
					utterance->attLine[j] = utterance->attLine[i];
					j++;
				}
			} else
				i--;
		} else if (utterance->line[i] == '\n' && utterance->line[i+1] == '\t') {
			uttline[j] = utterance->line[i];
			utterance->attLine[j] = utterance->attLine[i];
			j++;
			i++;
			uttline[j] = utterance->line[i];
			utterance->attLine[j] = utterance->attLine[i];
			j++;
			isSpaceFound = TRUE;
		} else if (utterance->line[i] == '\n') {
			uttline[j] = utterance->line[i];
			utterance->attLine[j] = utterance->attLine[i];
			j++;
		} else if (uS.isskip(uttline,i,&dFnt,MBF) && uttline[i]!='[' && uttline[i]!=']' && uttline[i]!='<' && uttline[i]!='>') {
			if (!isSpaceFound) {
				if (i != j) {
					uttline[j] = uttline[i];
					utterance->attLine[j] = utterance->attLine[i];
				}
				isSpaceFound = TRUE;
				j++;
			}
		} else {
			if (i != j) {
				uttline[j] = uttline[i];
				utterance->attLine[j] = utterance->attLine[i];
			}
			j++;
			isSpaceFound = FALSE;
		}
	}
	uttline[j] = EOS;
	if (onlydata == 5) {
		for (j=0; isSpace(uttline[j]) || uttline[j] == '\n'; j++) ;
	} else {
		for (j=0; isSpace(uttline[j]); j++) ;
	}
	if (j > 0)
		att_cp(0, uttline, uttline+j, utterance->attLine, utterance->attLine+j);
}

void call() {
	int  i;
	char isShow, isKeywordChecked;
	char nextTS;
	char *outLine, *comma, *keyw;

	nextTS = FALSE;
	kw = 0;
	*keyword = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (kwalTSpeaker[0] != EOS) {
			strcpy(templineC3, utterance->speaker);
			uS.uppercasestr(templineC3, &dFnt, MBF);
		}
		if (onlydata == 5) {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC2, TRUE)) {
					uS.remblanks(utterance->line);
					kwal_FindSpeaker(templineC2, utterance->line);
				}
				continue;
			}
		}
		if (CntWUT == 1) {
			outLine = uttline;
			for (i=0L; utterance->line[i]; i++) {
				if (!isSpace(uttline[i]))
					uttline[i] = utterance->line[i];
			}
		} else if (outputOnlyMatched) {
			blankoSelectedSpeakers(uttline);
			outLine = uttline;
		} else {
			blankoSelectedSpeakers(uttline);
			outLine = utterance->line;
		}
		if (kwalTSpeaker[0] != EOS && uS.partcmp(templineC3,kwalTSpeaker,TRUE,FALSE)) {
			if ((nextTS && kwalTSInclude) || (!nextTS && !kwalTSInclude)) {
				if (onlydata == 0) {
					fputs("----------------------------------------\n",fpout);
					fputs("*** ",fpout);
					pr_idfld(keyword,utterance->tlineno);
				} else if (onlydata == 2) {
					fputs("@Comment:\t----------------------------------------\n", fpout);
					fputs("@Comment:\t*** ",fpout);
					pr_idfld(keyword,utterance->tlineno);
				} else if (onlydata == 5) {
					if (!isKeywordOneColumn) {
						uS.remblanks(utterance->speaker);
						kwal_outputIDInfo(fpout, utterance->speaker, oldfname);
						fprintf(fpout, "%ld\t", utterance->tlineno);
						if (keyword[0] == EOS)
							comma = NULL;
						else {
							if (isDuplicateTiers) {
								comma = strchr(keyword, ',');
								if (comma != NULL)
									*comma = EOS;
							}
							fprintf(fpout, "%s\t", keyword);
						}
					}
				}
				if (!isKeywordOneColumn)
					*keyword = EOS;
				if (outputOnlyMatched)
					cleanUpOutput();
				if (utterance->nextutt == utterance || *utterance->speaker != '@' || !CheckOutTier(utterance->speaker))
					befprintout(onlydata != 1);
				if (!CheckOutTier(utterance->speaker) && *utterance->speaker == '*' && nomain) {
					remove_main_tier_print(utterance->speaker, outLine, utterance->attLine);
				} else if (chatmode == 0) {
					if (onlydata == 5)
						remove_CRs_Tabs(outLine);
					if (cMediaFileName[0] != EOS && onlydata != 1)
						changeBullet(outLine, utterance->attLine);
					if (onlydata == 5) {
						if (isKeywordOneColumn) {
							strcpy(spareTier1, utterance->speaker);
							strcat(spareTier1, outLine);
							rootUtts = AddLinearUtts(rootUtts, utterance->tlineno, keyword, spareTier1);
							*keyword = EOS;
						} else
							fprintf(fpout, "%s", outLine);
					} else
						printout(NULL,outLine,utterance->attSp,utterance->attLine,FALSE);
				} else {
					if (onlydata == 5)
						remove_CRs_Tabs(outLine);
					if (cMediaFileName[0] != EOS && onlydata != 1)
						changeBullet(outLine, utterance->attLine);
					if (onlydata == 5) {
						if (isKeywordOneColumn) {
							strcpy(spareTier1, utterance->speaker);
							strcat(spareTier1, outLine);
							rootUtts = AddLinearUtts(rootUtts, utterance->tlineno, keyword, spareTier1);
							*keyword = EOS;
						} else
							fprintf(fpout, "%s", outLine);
					} else
						printout(utterance->speaker,outLine,utterance->attSp,utterance->attLine,FALSE);
				}
				if (isDuplicateTiers && comma != NULL) {
					do {
						for (comma++; isSpace(*comma); comma++) ;
						if (*comma == EOS)
							break;
						keyw = comma;
						comma = strchr(keyw, ',');
						if (comma != NULL)
							*comma = EOS;
						putc('\n', fpout);
						kwal_outputIDInfo(fpout, utterance->speaker, oldfname);
						fprintf(fpout, "%ld\t", utterance->tlineno);
						fprintf(fpout, "%s\t", keyw);
						fprintf(fpout, "%s", outLine);
					} while (comma != NULL) ;
				}
				if (utterance->nextutt == utterance || *utterance->speaker != '@' || !CheckOutTier(utterance->speaker))
					aftprintout(onlydata != 1);
				if (onlydata == 5 && !isKeywordOneColumn)
					putc('\n', fpout);
			}
			nextTS = FALSE;
		} else {
			isKeywordChecked = FALSE;
			isShow = ((!excludeutter(TRUE, &isKeywordChecked) || (wdptr == NULL && isMorSearchListGiven() == FALSE)) && FullLine(uttline, TRUE));
			if (!isShow)
				isShow = (nextTS && kwalTSInclude);
			if (!isShow && nomain && wdptr == NULL && isMorSearchListGiven() == FALSE && onlydata != 5) {
				isShow = (*utterance->speaker== '*' && nomain && isDepTierIncluded && HeadOutTier == NULL);
				if (!isShow)
					isShow = (*utterance->speaker== '*' && nomain && FullLine(uttline,FALSE) && CheckOutTier(utterance->speaker));
				if (!isShow && tct) {
					for (i=0; outLine[i] != EOS; i++) {
						if (outLine[i] == '\n' && outLine[i+1] == '%')
							break;
					}
					if (outLine == utterance->line && outLine[i] != EOS) {
						isShow = TRUE;
						att_cp(0, utterance->line, utterance->line+i, utterance->attLine, utterance->attLine+i);
					}
				}
			}
			if (!isShow)
				isShow = (*utterance->speaker== '*' && nomain && isDepTierIncluded && !excludeutter(FALSE, &isKeywordChecked) && HeadOutTier == NULL && onlydata != 5);
			if (!isShow)
				isShow = (*utterance->speaker== '*' && nomain && FullLine(uttline, FALSE) && !excludeutter(FALSE, &isKeywordChecked) && CheckOutTier(utterance->speaker));
			if (!isShow)
				isShow = ((CntWUT || CntFUttLen) && !excludeutter(FALSE, &isKeywordChecked) && CheckOutTier(utterance->speaker) && utterance->nextutt == utterance);
			if (!isShow)
				isShow = ((CntWUT || CntFUttLen) && WordMode == '\0' && CheckOutTier(utterance->speaker) && utterance->nextutt == utterance);
			if (!isShow)
				isShow = (*utterance->speaker== '@' && CheckOutTier(utterance->speaker) /*&& utterance->nextutt == utterance*/);
			if (isShow) {
				if (kwalTSpeaker[0] != EOS)
					nextTS = TRUE;
				if (kwalTSInclude) {
					if (onlydata == 0) {
						fputs("----------------------------------------\n",fpout);
						fputs("*** ",fpout);
						pr_idfld(keyword,utterance->tlineno);
					} else if (onlydata == 2) {
						fputs("@Comment:\t----------------------------------------\n", fpout);
						fputs("@Comment:\t*** ",fpout);
						pr_idfld(keyword,utterance->tlineno);
					} else if (onlydata == 5) {
						if (!isKeywordOneColumn) {
							uS.remblanks(utterance->speaker);
							kwal_outputIDInfo(fpout, utterance->speaker, oldfname);
							fprintf(fpout, "%ld\t", utterance->tlineno);
							if (keyword[0] == EOS)
								comma = NULL;
							else {
								if (isDuplicateTiers) {
									comma = strchr(keyword, ',');
									if (comma != NULL)
										*comma = EOS;
								}
								fprintf(fpout, "%s\t", keyword);
							}
						}
					}
					if (!isKeywordOneColumn)
						*keyword = EOS;
					if (outputOnlyMatched)
						cleanUpOutput();
					if (utterance->nextutt == utterance || *utterance->speaker != '@' || !CheckOutTier(utterance->speaker))
						befprintout(onlydata != 1);
					if (!CheckOutTier(utterance->speaker) && *utterance->speaker == '*' && nomain) {
						remove_main_tier_print(utterance->speaker, outLine, utterance->attLine);
					} else if (chatmode == 0) {
						if (onlydata == 5)
							remove_CRs_Tabs(outLine);
						if (cMediaFileName[0] != EOS && onlydata != 1)
							changeBullet(outLine, utterance->attLine);
						if (onlydata == 5) {
							if (isKeywordOneColumn) {
								strcpy(spareTier1, utterance->speaker);
								strcat(spareTier1, outLine);
								rootUtts = AddLinearUtts(rootUtts, utterance->tlineno, keyword, spareTier1);
								*keyword = EOS;
							} else
								fprintf(fpout, "%s", outLine);
						} else
							printout(NULL,outLine,utterance->attSp,utterance->attLine,FALSE);
					} else {
						if (onlydata == 5)
							remove_CRs_Tabs(outLine);
						if (cMediaFileName[0] != EOS && onlydata != 1)
							changeBullet(outLine, utterance->attLine);
						if (onlydata == 5) {
							if (isKeywordOneColumn) {
								strcpy(spareTier1, utterance->speaker);
								strcat(spareTier1, outLine);
								rootUtts = AddLinearUtts(rootUtts, utterance->tlineno, keyword, spareTier1);
								*keyword = EOS;
							} else
								fprintf(fpout, "%s", outLine);
						} else
							printout(utterance->speaker,outLine,utterance->attSp,utterance->attLine,FALSE);
					}
					if (isDuplicateTiers && comma != NULL) {
						do {
							for (comma++; isSpace(*comma); comma++) ;
							if (*comma == EOS)
								break;
							keyw = comma;
							comma = strchr(keyw, ',');
							if (comma != NULL)
								*comma = EOS;
							putc('\n', fpout);
							kwal_outputIDInfo(fpout, utterance->speaker, oldfname);
							fprintf(fpout, "%ld\t", utterance->tlineno);
							fprintf(fpout, "%s\t", keyw);
							fprintf(fpout, "%s", outLine);
						} while (comma != NULL) ;
					}
					if (utterance->nextutt == utterance || *utterance->speaker != '@' || !CheckOutTier(utterance->speaker))
						aftprintout(onlydata != 1);
					if (onlydata == 5 && !isKeywordOneColumn)
						putc('\n', fpout);
				}
			} else
				nextTS = FALSE;
		}
		kw = 0;
	}
	if (!combinput) {
		if (isKeywordOneColumn) {
			PrintUtt(rootUtts, NULL);
			for (i=0; i < MAXKEYS; i++) {
				if (keywordsList[i] != NULL) {
					free(keywordsList[i]);
					keywordsList[i] = NULL;
				}
			}
			kwal_head = kwal_freeSpeakers(kwal_head);
		} else if (kwal_sort) {
			PrintKeywords(rootKey);
			kwal_head = kwal_freeSpeakers(kwal_head);
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'a':
				kwal_sort = TRUE;
				no_arg_option(f);
				break;
		case 'n':
				if (*f == '*')
					kwalTSpeaker[0] = EOS;
				else
					strcpy(kwalTSpeaker, "*");
				strcat(kwalTSpeaker, f);
				uS.uppercasestr(kwalTSpeaker, &dFnt, MBF);
				kwalTSInclude = (*(f-2) == '+');
				break;
		case 'b':
				kwal_isOnlyOne = TRUE;
				no_arg_option(f);
				break;
		case 'd':
				if (*f == '4' && *(f+1) == '0') {
					isDuplicateTiers = TRUE;
					isKeywordOneColumn = FALSE;
					onlydata = 5;
					combinput = TRUE;
					break;
/*
				} else if (*f == '4' && *(f+1) == '1') {
					isKeywordOneColumn = TRUE;
					isDuplicateTiers = FALSE;
					onlydata = 5;
					break;
*/
				} else if (*f == '4') {
					combinput = TRUE;
					isKeywordOneColumn = TRUE;
				} else if (*f == '9' && *(f+1) == '9') {
					isExpendX = TRUE;
					break;
				} else if (*f == '3' && *(f+1) == '0') {
					outputOnlyMatched = 2;
					R5 = FALSE;
					R7Slash = FALSE;
					R7Tilda = FALSE;
					R7Caret = FALSE;
					R7Colon = FALSE;
					isRemoveCAChar[NOTCA_CROSSED_EQUAL] = FALSE;
					isRemoveCAChar[NOTCA_LEFT_ARROW_CIRCLE] = FALSE;
					Parans = 2;
					IncludeAllDefaultCodes = TRUE;
					break;
				} else if (*f == '3') {
					outputOnlyMatched = 1;
					break;
				}
		case 's':
				if (*(f-2) == '+' && *(f-1) == 's') {
					if (*f == '\\' && *(f+1) == '%' && f[strlen(f)-1] == ':')
						isBlankSpeakerName = FALSE;
				}
		case 'z':
				if (kwal_ftime) {
					if (outputOnlyMatched != 2) {
						addword('\0','\0',"+xxx");
						addword('\0','\0',"+yyy");
						addword('\0','\0',"+www");
						addword('\0','\0',"+xxx@s*");
						addword('\0','\0',"+yyy@s*");
						addword('\0','\0',"+www@s*");
						addword('\0','\0',"+unk|xxx");
						addword('\0','\0',"+unk|yyy");
						addword('\0','\0',"+*|www");
					}
					kwal_ftime = FALSE;
				}
				maingetflag(f-2,f1,i);
				break;

		default:
				maingetflag(f-2,f1,i);
				break;
	}
}
