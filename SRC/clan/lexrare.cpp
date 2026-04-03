/**********************************************************************
	"Copyright 1990-2026 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main lexrare_main
#define call lexrare_call
#define getflag lexrare_getflag
#define init lexrare_init
#define usage lexrare_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define NUMGEMITEMS 10

extern char GExt[];
extern char OverWriteFile;
extern char outputOnlyData;
extern char PostCodeMode;
extern char R7Colon;
extern char R7Caret;
extern void (*fatal_error_function)(char);
extern struct tier *defheadtier;


struct lexrare_freqTnode {
	char *word;
	unsigned int count;
	struct lexrare_freqTnode *left;
	struct lexrare_freqTnode *right;
};

struct lexrare_speakers {
	char isSpeakerFound;
	char isMORFound;

	char *fname;
	char *sp;
	char *ID;

	char *prompt_name;
	float common_words;
	float rare_words;
	float total_words;
	float ratio_words; // rare_words*100/total_words

	struct lexrare_freqTnode *freq_comm;
	long total_comm;		/* total number of words */
	long diff_comm;			/* number of different words */
	struct lexrare_freqTnode *freq_rare;
	long total_rare;		/* total number of words */
	long diff_rare;			/* number of different words */

	struct lexrare_speakers *next_sp;
} ;

static int  lexrare_SpecWords, DBGemNum;
static char *DBGems[NUMGEMITEMS], attrib_file[FILENAME_MAX+2];
static char lexrare_ftime, spTierOnly, isCommFreq, isRareFreq, isAllCommFreq, isAllRareFreq;
static char lexrare_BBS[5], lexrare_CBS[5];
static char lexrare_group, lexrare_n_option, isNOptionSet, GemMode, specialOptionUsed;
struct lexrare_freqTnode *freq_all_comm;
static long total_all_comm, diff_all_comm;
struct lexrare_freqTnode *freq_all_rare;
static long total_all_rare, diff_all_rare;
static struct lexrare_speakers *sp_head;
IEWORDS *common_words_list;


void usage() {
	puts("LEXRARE creates a spreadsheet with a percentage of rare words out of total number of words.");
#ifdef UNX
	printf("LEXRARE NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO RUN LEXRARE ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cLEXRARE NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO RUN LEXRARE ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	printf("Usage: lexrare [%s] filename(s)\n",mainflgs());
	puts("+g : Gem tier should contain all words specified by +gS");
	puts("+gS: select gems which are labeled by label S");
	puts("+lc: output freqency of common words");
	puts("+lr: output reqency of rare words");
	puts("+lac: output freqency of common words across all data files");
	puts("+lar: output reqency of rare words across all data files");
	puts("+n : Gem is terminated by the next @G (default: automatic detection)");
	puts("-n : Gem is defined by @BG and @EG (default: automatic detection)");
	puts("+pF: specify common words file F (default: lexrare_mor.cut)");
	mainusage(FALSE);
#ifdef UNX
	printf("LEXRARE NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO RUN LEXRARE ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cLEXRARE NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO RUN LEXRARE ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	cutt_exit(0);
}

static void lexrare_freqFreetree(struct lexrare_freqTnode *p) {
	if (p != NULL) {
		lexrare_freqFreetree(p->left);
		lexrare_freqFreetree(p->right);
		free(p->word);
		free(p);
	}
}

static struct lexrare_speakers *freespeakers(struct lexrare_speakers *p) {
	struct lexrare_speakers *ts;

	while (p) {
		ts = p;
		p = p->next_sp;
		if (ts->fname != NULL)
			free(ts->fname);
		if (ts->sp != NULL)
			free(ts->sp);
		if (ts->ID != NULL)
			free(ts->ID);
		if (ts->prompt_name != NULL)
			free(ts->prompt_name);
		if (ts->freq_comm != NULL) {
			lexrare_freqFreetree(ts->freq_comm);
			ts->freq_comm = NULL;
		}
		if (ts->freq_rare != NULL) {
			lexrare_freqFreetree(ts->freq_rare);
			ts->freq_rare = NULL;
		}
		free(ts);
	}
	return(NULL);
}

static void lexrare_error(char IsOutOfMem) {
	if (IsOutOfMem)
		fputs("ERROR: Out of memory.\n",stderr);
	sp_head = freespeakers(sp_head);

	if (freq_all_comm != NULL) {
		lexrare_freqFreetree(freq_all_comm);
		freq_all_comm = NULL;
	}
	if (freq_all_rare != NULL) {
		lexrare_freqFreetree(freq_all_rare);
		freq_all_rare = NULL;
	}

	common_words_list = freeIEWORDS(common_words_list);
	cutt_exit(0);
}

static void lexrare_initTSVars(struct lexrare_speakers *ts) {
	ts->isSpeakerFound = FALSE;
	ts->isMORFound = FALSE;
	
	ts->common_words= 0.0;
	ts->rare_words  = 0.0;
	ts->total_words	= 0.0;
	ts->ratio_words = 0.0;
	
	ts->total_comm = 0L;	/* total number of words */
	ts->diff_comm  = 0L;	/* number of different words */
	ts->total_rare = 0L;	/* total number of words */
	ts->diff_rare  = 0L;	/* number of different words */
}

static struct lexrare_speakers *lexrare_FindSpeaker(char *fname, char *sp, char *ID, char *prompt_name, char isSpeakerFound) {
	struct lexrare_speakers *ts, *tsp;

	uS.remblanks(sp);
	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->fname, fname) == 0) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
				ts->isSpeakerFound = isSpeakerFound;
				if (ts->prompt_name != NULL)
					free(ts->prompt_name);
				if ((ts->prompt_name=(char *)malloc(strlen(prompt_name)+1)) == NULL) {
					lexrare_error(TRUE);
				}
				strcpy(ts->prompt_name, prompt_name);
				return(ts);
			}
		}
	}
	if ((ts=NEW(struct lexrare_speakers)) == NULL)
		lexrare_error(TRUE);
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL) {
		free(ts);
		lexrare_error(TRUE);
	}
	if (sp_head == NULL) {
		sp_head = ts;
	} else {
		for (tsp=sp_head; tsp->next_sp != NULL; tsp=tsp->next_sp) ;
		tsp->next_sp = ts;
	}
	ts->next_sp = NULL;
	ts->freq_comm = NULL;
	ts->freq_rare = NULL;
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		lexrare_error(TRUE);
	}
	strcpy(ts->sp, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			lexrare_error(TRUE);
		strcpy(ts->ID, ID);
	}
	ts->isSpeakerFound = isSpeakerFound;
	if ((ts->prompt_name=(char *)malloc(strlen(prompt_name)+1)) == NULL) {
		lexrare_error(TRUE);
	}
	strcpy(ts->prompt_name, prompt_name);
	lexrare_initTSVars(ts);
	return(ts);
}

static void addDBGems(char *gem) {
	if (DBGemNum >= NUMGEMITEMS) {
		fprintf(stderr, "\nERROR: Too many keywords specified. The limit is %d\n", NUMGEMITEMS);
		lexrare_error(FALSE);
	}
	DBGems[DBGemNum] = gem;
	DBGemNum++;
}

static IEWORDS *lexrare_addwdptr(char *wd, IEWORDS *wdptr) {
	int i;
	IEWORDS *tempwd;

	for (i=strlen(wd)-1; wd[i]== ' ' || wd[i]== '\t' || wd[i]== '\n'; i--) ;
	wd[++i] = EOS;
	for (i=0; wd[i] == ' ' || wd[i] == '\t'; i++) ;
	tempwd = NEW(IEWORDS);
	if (tempwd == NULL) {
		lexrare_error(TRUE);
	} else {
		tempwd->nextword = wdptr;
		wdptr = tempwd;
		wdptr->word = (char *)malloc(strlen(wd+i)+1);
		if (wdptr->word == NULL) {
			lexrare_error(TRUE);
		}
		if (!nomap)
			uS.lowercasestr(wd+i, &dFnt, C_MBF);
		strcpy(wdptr->word, wd+i);
	}
	return(wdptr);
}

static IEWORDS *lexrare_rdexclf(FNType *fname, IEWORDS *wdptr) {
	FILE *efp;
	char wd[512];
	FNType mFileName[FNSize];

	if (*fname == '+') {
		*wd = '+';
		fname++;
	} else
		*wd = ' ';

	if (*fname == EOS) {
		fprintf(stderr, "No file name for lexrare_mor specified!\n");
		lexrare_error(FALSE);
	}
	if ((efp=OpenGenLib(fname,"r",TRUE,TRUE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of common words files:\n\t\"%s\", \"%s\"\n", fname, mFileName);
		cutt_exit(0);
	}
	fprintf(stderr, "    Using common words file: %s\n", mFileName);
	while (fgets_cr(wd+1, 511, efp)) {
		if (uS.isUTF8(wd+1) || uS.isInvisibleHeader(wd+1))
			continue;
		if (wd[1] == ';' && wd[2] == '%' && wd[3] == '*' && wd[4] == ' ')
			continue;
		if (wd[1] == '#' && wd[2] == ' ')
			continue;
		wdptr = lexrare_addwdptr(wd, wdptr);
	}
	return(wdptr);
}

void init(char f) {
	if (f) {
		common_words_list = NULL;
		sp_head = NULL;
		freq_all_comm = NULL;
		total_all_comm = 0L;
		diff_all_comm  = 0L;
		freq_all_rare = NULL;
		total_all_rare = 0L;
		diff_all_rare  = 0L;
		isCommFreq = FALSE;
		isRareFreq = FALSE;
		isAllCommFreq = FALSE;
		isAllRareFreq = FALSE;
		lexrare_ftime = TRUE;
		spTierOnly = FALSE;
		fatal_error_function = lexrare_error;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		maininitwords();
		mor_initwords();
		specialOptionUsed = FALSE;
		GemMode = '\0';
		lexrare_SpecWords = 0;
		lexrare_group = FALSE;
		lexrare_n_option = FALSE;
		isNOptionSet = FALSE;
		strcpy(lexrare_BBS, "@*&#");
		strcpy(lexrare_CBS, "@*&#");
		DBGemNum = 0;
		strcpy(attrib_file, "lexrare_mor.cut");
	} else {
		if (lexrare_ftime) {
			lexrare_ftime = FALSE;
			if (chatmode) {
				if (GemMode != '\0' || isNOptionSet == TRUE) {
					LocalTierSelect = TRUE;
				} else {
					maketierchoice("@ID:",'+',FALSE);
					maketierchoice("@Comment:",'+',FALSE);
					if (spTierOnly == FALSE)
						maketierchoice("%mor:",'+',FALSE);
				}
			} else {
				fprintf(stderr, "lexrare can only run on CHAT data files\n\n");
				cutt_exit(0);
			}
			if (lexrare_group && GemMode == '\0') {
				fprintf(stderr, "\nThe \"+g\" option has to used with \"+gS\" option.\n");
				cutt_exit(0);
			}
			if (!f_override)
				stout = FALSE;
			outputOnlyData = TRUE;
			OverWriteFile = TRUE;
			R7Colon = FALSE;
			R7Caret = FALSE;
			
			AddCEXExtension = ".xls";
			combinput = TRUE;
			if (attrib_file[0] != EOS) {
				common_words_list = lexrare_rdexclf(attrib_file, common_words_list);
			}
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	char wd[1024+2];

	f++;
	switch(*f++) {
		case 'g':
			if (*f == EOS) {
				lexrare_group = TRUE;
				specialOptionUsed = TRUE;
			} else {
				GemMode = 'i';
				lexrare_SpecWords++;
				addDBGems(getfarg(f,f1,i));
			}
			break;
		case 'l':
			if (*f == 'c') {
				isCommFreq = TRUE;
			} else if (*f == 'r') {
				isRareFreq = TRUE;
			} else if (*f == 'a') {
				f++;
				if (*f == 'c') {
					isAllCommFreq = TRUE;
				} else if (*f == 'r') {
					isAllRareFreq = TRUE;
				} else {
					fprintf(stderr,"Please specify +lc or +lr options\n");
					cutt_exit(0);
				}
			} else {
				fprintf(stderr,"Please specify +lc or +lr options\n");
				cutt_exit(0);
			}
			break;
		case 'n':
			if (*(f-2) == '+') {
				lexrare_n_option = TRUE;
				strcpy(lexrare_BBS, "@G:");
				strcpy(lexrare_CBS, "@*&#");
				specialOptionUsed = TRUE;
			} else {
				strcpy(lexrare_BBS, "@BG:");
				strcpy(lexrare_CBS, "@EG:");
			}
			isNOptionSet = TRUE;
			no_arg_option(f);
			break;
		case 'p':
			if (*f) {
				removeExtraSpace(wd);
				uS.remFrontAndBackBlanks(wd);
				strncpy(attrib_file, f, FILENAME_MAX);
				attrib_file[FILENAME_MAX] = EOS;
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 's':
			specialOptionUsed = TRUE;
			if (*f == '[' && *(f+1) == '-') {
				maingetflag(f-2,f1,i);
			} else if ((*f == '[' && *(f+1) == '+') || ((*f == '+' || *f == '~') && *(f+1) == '[' && *(f+2) == '+')) {
				maingetflag(f-2,f1,i);
			} else {
				fprintf(stderr, "Please specify only postcodes, \"[+ ...]\", or precodes \"[- ...]\" with +/-s option.\n");
				cutt_exit(0);
			}
			break;
		case 't':
			if (*(f-2) == '+' && (*f == '*' || *f == '#')) {
				maingetflag(f-2,f1,i);
			} else if (*(f-2) == '-' && (!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:"))) {
				spTierOnly = TRUE;
				maingetflag(f-2,f1,i);
			} else {
				fprintf(stderr, "\nPlease specify only one speaker tier code with \"+t\" option.\n");
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void lexrare_freqTreeprint(struct lexrare_freqTnode *p, FILE *fp) {
	if (p != NULL) {
		lexrare_freqTreeprint(p->left, fp);
		do {
			fprintf(fp,"%3u ",p->count);
			fprintf(fp,"%-10s\n", p->word);
			if (p->right == NULL)
				break;
			if (p->right->left != NULL) {
				lexrare_freqTreeprint(p->right, fp);
				break;
			}
			p = p->right;
		} while (1);
	}
}

static void lexrare_pr_result(void) {
	char  *sFName;
	struct lexrare_speakers *ts;

	if (sp_head == NULL) {
		if (lexrare_SpecWords) {
			fprintf(stderr,"\nERROR: No speaker matching +t option found\n");
			fprintf(stderr,"OR No specified gems found for this speaker\n\n");
		} else
			fprintf(stderr, "\nERROR: No speaker matching +t option found\n\n");
	}
	excelHeader(fpout, newfname, 105);
	excelRow(fpout, ExcelRowStart);
	excelStrCell(fpout, "File");
	excelCommasStrCell(fpout, "Language,Corpus,Code,Age(Month),Sex,Group,Race,SES,Role,Education,Custom_field");

	excelStrCell(fpout, "prompt_name");
	excelStrCell(fpout, "#_common_words");
	excelStrCell(fpout, "#_rare_words");
	excelStrCell(fpout, "#_total_words");
	excelStrCell(fpout, "%_of_rare_words");
	excelRow(fpout, ExcelRowEnd);

	for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
		if (!ts->isSpeakerFound) {
			if (lexrare_SpecWords) {
				fprintf(stderr,"\nWARNING: No specified gems found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			} else
				fprintf(stderr, "\nWARNING: No data found for speaker \"%s\" in file \"%s\"\n\n", ts->sp, ts->fname);
			continue;
		}
		
		sFName = strrchr(ts->fname, PATHDELIMCHR);
		if (sFName != NULL)
			sFName = sFName + 1;
		else
			sFName = ts->fname;
		excelRow(fpout, ExcelRowStart);
		excelStrCell(fpout, sFName);
		if (ts->ID) {
			excelOutputID(fpout, ts->ID);
		} else {
			excelCommasStrCell(fpout, ".,.");
			excelStrCell(fpout, ts->sp);
			excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
		}
		
		if (spTierOnly == FALSE && !ts->isMORFound) {
			fprintf(stderr, "\n*** File \"%s\": Speaker \"%s\"\n", sFName, ts->sp);
			fprintf(stderr, "WARNING: Speaker \"%s\" has no \"%s\" tiers.\n\n", ts->sp, "%mor:");
			excelCommasStrCell(fpout, "NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA");
		} else if (ts->total_words == 0.0) {
			fprintf(stderr, "\n*** File \"%s\": Speaker \"%s\"\n", sFName, ts->sp);
			fprintf(stderr, "WARNING: Speaker \"%s\" has no words to count.\n\n", ts->sp);
			excelCommasStrCell(fpout, "NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA");
		} else {
			if (ts->prompt_name == NULL)
				excelStrCell(fpout, "NA");
			else
				excelStrCell(fpout, ts->prompt_name);
			excelNumCell(fpout, "%.0f", ts->common_words);
			excelNumCell(fpout, "%.0f", ts->rare_words);
			excelNumCell(fpout, "%.0f", ts->total_words);
			excelNumCell(fpout, "%.4f", (ts->rare_words * 100.000) / ts->total_words);
		}
		excelRow(fpout, ExcelRowEnd);
	}
	excelFooter(fpout);
	sp_head = freespeakers(sp_head);
}


static struct lexrare_freqTnode *lexrare_talloc(char *word, unsigned int count) {
	struct lexrare_freqTnode *p;

	if ((p=NEW(struct lexrare_freqTnode)) == NULL)
		lexrare_error(TRUE);
	p->word = word;
	p->count = count;
	return(p);
}

static char *lexrare_strsave(char *s) {
	char *p;

	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		lexrare_error(TRUE);
	return(p);
}

static struct lexrare_freqTnode *lexrare_freqTree(struct lexrare_freqTnode *p, long *diff, lexrare_speakers *ts, char *w) {
	int cond;
	struct lexrare_freqTnode *t = p;

	if (p == NULL) {
		(*diff)++;
		p = lexrare_talloc(lexrare_strsave(w), 0);
		p->count++;
		p->left = p->right = NULL;
	} else if ((cond=strcmp(w,p->word)) == 0) {
		p->count++;
	} else if (cond < 0)
		p->left = lexrare_freqTree(p->left, diff, ts, w);
	else {
		for (; (cond=strcmp(w,p->word)) > 0 && p->right != NULL; p=p->right) ;
		if (cond == 0) {
			p->count++;
		} else if (cond < 0) {
			p->left = lexrare_freqTree(p->left, diff, ts, w);
		} else {
			p->right = lexrare_freqTree(p->right, diff, ts, w); // if cond > 0
		}
		return(t);
	}
	return(p);
}


static int excludeGemKeywords(char *word) {
	int i;

	if (word[0] == '+' || strcmp(word, "!") == 0 || strcmp(word, "?") == 0 || strcmp(word, ".") == 0)
		return(FALSE);
	for (i=0; i < DBGemNum; i++) {
		if (uS.mStricmp(DBGems[i], word) == 0)
			return(TRUE);
	}
	return(FALSE);
}

static int isRightText(char *gem_word) {
	int i = 0;
	int found = 0;

	if (GemMode == '\0')
		return(TRUE);
	filterwords("@", uttline, excludeGemKeywords);
	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i)))
		found++;
	if (GemMode == 'i')
		return((lexrare_group == FALSE && found) || (lexrare_SpecWords == found));
	else
		return((lexrare_group == TRUE && lexrare_SpecWords > found) || (found == 0));
}

static char isCommonWord(char *word) {
	IEWORDS *twd;
	
	for (twd=common_words_list; twd != NULL; twd = twd->nextword) {
		if (uS.mStricmp(twd->word, word) == 0)
			return(TRUE);
	}
	return(FALSE);
}

static void OutputWordsLists(struct lexrare_speakers *ts, const FNType *fname) {
	char *s;
	float t1, t2;
	FILE *fp;

	strcpy(FileName1, fname);
	if (isCommFreq) {
		s = strrchr(FileName1, '.');
		if (s != NULL) {
			*s = EOS;
			strcat(FileName1, ".common.cex");
			if ((fp=fopen(FileName1, "w")) != NULL) {
				for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
					if (ts->freq_comm != NULL) {
						lexrare_freqTreeprint(ts->freq_comm, fp);
						fprintf(fp,"------------------------------\n");
						fprintf(fp,"%5ld  Total number of different item types used\n", ts->diff_comm);
						fprintf(fp,"%5ld  Total number of items (tokens)\n", ts->total_comm);
						t1 = (float)ts->total_comm; t2 = (float)ts->diff_comm;
						fprintf(fp,"%5.3f  Type/Token ratio\n", t2/t1);
						lexrare_freqFreetree(ts->freq_comm);
						ts->freq_comm = NULL;
					}
				}
				fclose(fp);
			}
		}
	}
	strcpy(FileName1, fname);
	if (isRareFreq) {
		s = strrchr(FileName1, '.');
		if (s != NULL) {
			*s = EOS;
			strcat(FileName1, ".rare.cex");
			if ((fp=fopen(FileName1, "w")) != NULL) {
				for (ts=sp_head; ts != NULL; ts=ts->next_sp) {
					if (ts->freq_rare != NULL) {
						lexrare_freqTreeprint(ts->freq_rare, fp);
						fprintf(fp,"------------------------------\n");
						fprintf(fp,"%5ld  Total number of different item types used\n", ts->diff_rare);
						fprintf(fp,"%5ld  Total number of items (tokens)\n", ts->total_rare);
						t1 = (float)ts->total_rare; t2 = (float)ts->diff_rare;
						fprintf(fp,"%5.3f  Type/Token ratio\n", t2/t1);
						lexrare_freqFreetree(ts->freq_rare);
						ts->freq_rare = NULL;
					}
				}
				fclose(fp);
			}
		}
	}
	
}

void call()	{		/* tabulate array of word lengths */
	int  i, j, found;
	char lRightspeaker;
	char word[BUFSIZ+1], comp_word[BUFSIZ+1], prompt_name[BUFSIZ+1], *s;
	char isOutputGem;
	struct lexrare_speakers *ts;
	MORFEATS word_feats, *compd, *feat;

	strcpy(prompt_name, "NA");
	if (isNOptionSet == FALSE) {
		strcpy(lexrare_BBS, "@*&#");
		strcpy(lexrare_CBS, "@*&#");
	}
	if (lexrare_SpecWords) {
		isOutputGem = FALSE;
	} else {
		isOutputGem = TRUE;
	}
	ts = NULL;
	lRightspeaker = FALSE;
	found = 0;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
 if (lineno > tlineno) {
	tlineno = lineno + 200;
 }
*/
		if (LocalTierSelect == TRUE) {
			if (!checktier(utterance->speaker)) {
				if (*utterance->speaker == '*')
					lRightspeaker = FALSE;
				continue;
			} else {
				if (*utterance->speaker == '*') {
					i = isPostCodeFound(utterance->speaker, utterance->line);
					if ((PostCodeMode == 'i' && i == 1) || (PostCodeMode == 'e' && i == 5))
						lRightspeaker = FALSE;
					else
						lRightspeaker = TRUE;
				}
				if (!lRightspeaker && *utterance->speaker != '@')
					continue;
			}
		} else {
			if (*utterance->speaker == '*') {
				i = isPostCodeFound(utterance->speaker, utterance->line);
				if ((PostCodeMode == 'i' && i == 1) || (PostCodeMode == 'e' && i == 5)) {
					lRightspeaker = FALSE;
					continue;
				} else
					lRightspeaker = TRUE;
			} else if (!lRightspeaker && *utterance->speaker != '@')
				continue;
		}
		if (lexrare_SpecWords && !strcmp(lexrare_BBS, "@*&#")) {
			if (uS.partcmp(utterance->speaker,"@BG:",FALSE,FALSE)) {
				lexrare_n_option = FALSE;
				strcpy(lexrare_BBS, "@BG:");
				strcpy(lexrare_CBS, "@EG:");
			} else if (uS.partcmp(utterance->speaker,"@G:",FALSE,FALSE)) {
				lexrare_n_option = TRUE;
				strcpy(lexrare_BBS, "@G:");
				strcpy(lexrare_CBS, "@*&#");
			}
		}

		if (*utterance->speaker == '@') {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
					uS.remblanks(utterance->line);
					lexrare_FindSpeaker(oldfname, templineC, utterance->line, prompt_name, FALSE);
				}
			} else if (uS.partcmp(utterance->speaker,"@Comment:",FALSE,FALSE)) {
				if (uS.partcmp(utterance->line, "prompt_name", FALSE, FALSE)) {
					i = strlen("prompt_name");
					while (isSpace(utterance->line[i]) || utterance->line[i] == '-')
						i++;
					j = 0;
					while (utterance->line[i] != '\n' && utterance->line[i] != EOS) {
						prompt_name[j++] = utterance->line[i++];
					}
					prompt_name[j] = EOS;
				}
			} else if (uS.partcmp(utterance->speaker,lexrare_BBS,FALSE,FALSE)) {
				if (lexrare_n_option) {
					if (isRightText(word)) {
						isOutputGem = TRUE;
					} else
						isOutputGem = FALSE;
				} else {
					if (isRightText(word)) {
						found++;
						if (found == 1 || GemMode != '\0') {
							isOutputGem = TRUE;
						}
					}
				}
			} else if (found > 0 && uS.partcmp(utterance->speaker,lexrare_CBS,FALSE,FALSE)) {
				if (lexrare_n_option) {
				} else {
					if (isRightText(word)) {
						found--;
						if (found == 0) {
							if (lexrare_SpecWords)
								isOutputGem = FALSE;
							else {
								isOutputGem = TRUE;
							}
						}
					}
				}
			}
		} else if (*utterance->speaker == '*' && isOutputGem) {
			ts = lexrare_FindSpeaker(oldfname, templineC, NULL, prompt_name, TRUE);
			if (spTierOnly == TRUE && ts != NULL) {
				i = 0;
				while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
					if (word[0] != EOS) {
						ts->total_words++;
						if (isCommonWord(word) == TRUE) {
							if (isCommFreq) {
								ts->total_comm++;
								ts->freq_comm = lexrare_freqTree(ts->freq_comm, &ts->diff_comm, ts, word);
							}
							if (isAllCommFreq) {
								total_all_comm++;
								freq_all_comm = lexrare_freqTree(freq_all_comm, &diff_all_comm, ts, word);
							}
							ts->common_words++;
						} else {
							if (isRareFreq) {
								ts->total_rare++;
								ts->freq_rare = lexrare_freqTree(ts->freq_rare, &ts->diff_rare, ts, word);
							}
							if (isAllRareFreq) {
								total_all_rare++;
								freq_all_rare = lexrare_freqTree(freq_all_rare, &diff_all_rare, ts, word);
							}
							ts->rare_words++;
						}
					}
				}
			}
		} else if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE) && ts != NULL && isOutputGem && spTierOnly == FALSE) {
			ts->isMORFound = TRUE;
			i = 0;
			while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
				if (word[0] != EOS) {
					s = strchr(word, '|');
					if (s != NULL) {
						strcpy(templineC2, word);
						if (ParseWordMorElems(templineC2, &word_feats) == FALSE)
							lexrare_error(FALSE);
						for (feat=&word_feats; feat != NULL; feat=feat->clitc) {
							ts->total_words++;
							if (feat->compd != NULL) {
								comp_word[0] = EOS;
								for (compd=feat; compd != NULL; compd=compd->compd) {
									if (compd->stem != NULL && compd->stem[0] != EOS)
										strcat(comp_word, compd->stem);
								}
								if (isCommonWord(comp_word) == TRUE) {
									if (isCommFreq) {
										ts->total_comm++;
										ts->freq_comm = lexrare_freqTree(ts->freq_comm, &ts->diff_comm, ts, comp_word);
									}
									if (isAllCommFreq) {
										total_all_comm++;
										freq_all_comm = lexrare_freqTree(freq_all_comm, &diff_all_comm, ts, comp_word);
									}
									ts->common_words++;
								} else {
									if (isRareFreq) {
										ts->total_rare++;
										ts->freq_rare = lexrare_freqTree(ts->freq_rare, &ts->diff_rare, ts, comp_word);
									}
									if (isAllRareFreq) {
										total_all_rare++;
										freq_all_rare = lexrare_freqTree(freq_all_rare, &diff_all_rare, ts, comp_word);
									}
									ts->rare_words++;
								}
							} else {
								if (isCommonWord(feat->stem) == TRUE) {
									if (isCommFreq) {
										ts->total_comm++;
										ts->freq_comm = lexrare_freqTree(ts->freq_comm, &ts->diff_comm, ts, feat->stem);
									}
									if (isAllCommFreq) {
										total_all_comm++;
										freq_all_comm = lexrare_freqTree(freq_all_comm, &diff_all_comm, ts, feat->stem);
									}
									ts->common_words++;
								} else {
									if (isRareFreq) {
										ts->total_rare++;
										ts->freq_rare = lexrare_freqTree(ts->freq_rare, &ts->diff_rare, ts, feat->stem);
									}
									if (isAllRareFreq) {
										total_all_rare++;
										freq_all_rare = lexrare_freqTree(freq_all_rare, &diff_all_rare, ts, feat->stem);
									}
									ts->rare_words++;
								}
							}
						}
						freeUpFeats(&word_feats);
					}
				}
			}
		}
	}
	if (!combinput) {
		lexrare_pr_result();
	}
	OutputWordsLists(ts, oldfname);
}

static void OutputAllWordsLists(const FNType *fname) {
	float t1, t2;
	FILE *fp;

	strcpy(FileName1, fname);
	if (isAllCommFreq) {
		strcat(FileName1, ".common.cex");
		if ((fp=fopen(FileName1, "w")) != NULL) {
			if (freq_all_comm != NULL) {
				lexrare_freqTreeprint(freq_all_comm, fp);
				fprintf(fp,"------------------------------\n");
				fprintf(fp,"%5ld  Total number of different item types used\n", diff_all_comm);
				fprintf(fp,"%5ld  Total number of items (tokens)\n", total_all_comm);
				t1 = (float)total_all_comm; t2 = (float)diff_all_comm;
				fprintf(fp,"%5.3f  Type/Token ratio\n", t2/t1);
				lexrare_freqFreetree(freq_all_comm);
				freq_all_comm = NULL;
			}
			fclose(fp);
		}
	}
	strcpy(FileName1, fname);
	if (isAllRareFreq) {
		strcat(FileName1, ".rare.cex");
		if ((fp=fopen(FileName1, "w")) != NULL) {
			if (freq_all_rare != NULL) {
				lexrare_freqTreeprint(freq_all_rare, fp);
				fprintf(fp,"------------------------------\n");
				fprintf(fp,"%5ld  Total number of different item types used\n", diff_all_rare);
				fprintf(fp,"%5ld  Total number of items (tokens)\n", total_all_rare);
				t1 = (float)total_all_rare; t2 = (float)diff_all_rare;
				fprintf(fp,"%5.3f  Type/Token ratio\n", t2/t1);
				lexrare_freqFreetree(freq_all_rare);
				freq_all_rare = NULL;
			}
			fclose(fp);
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = LEXRARE;
	chatmode = CHAT_MODE;
	OnlydataLimit = 1;
	UttlineEqUtterance = FALSE;
	sp_head = NULL;

	freq_all_comm = NULL;
	total_all_comm = 0L;
	diff_all_comm  = 0L;
	freq_all_rare = NULL;
	total_all_rare = 0L;
	diff_all_rare  = 0L;

	bmain(argc,argv,lexrare_pr_result);

	sp_head = freespeakers(sp_head);
	OutputAllWordsLists("all_files");
	common_words_list = freeIEWORDS(common_words_list);
}
