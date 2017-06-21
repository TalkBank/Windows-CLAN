/**********************************************************************
 "Copyright 2010 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */


#define CHAT_MODE 3
#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define PERIOD 50

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char AddCEXExtension;
extern char isRecursive;

struct IDs {
	char isRightSp;
	char *l1; // @Languages:, @Language of #:
	char *language; // @Languages:, @Language of #:
	char *corpus;   // such as: MacWhinney, Bates, Sachs, etc...
	char *code;		// @Participants: *CHI
	char *age;		// @Age of #:
	char *sex;		// @Sex of #:
	char *group;	// @Group of #:
	char *SES;		// @Ses of #:
	char *role;		// @Participants: Target_Child
	char *education;// @Education of #:
	char *file_unique;//
	struct IDs *next_id;
} ;

struct htiers {
    char sp[SPEAKERLEN];			/* code descriptor field of the turn	*/
    AttTYPE attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
    char line[UTTLINELEN+1];		/* text field of the turn		*/ // found uttlinelen
    AttTYPE attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
	struct htiers *next_tier;
} ;

static struct IDs *root_ids;
static struct htiers *root_htiers, *local_tiers;

void usage() {
	printf("Usage: temp [re] filename(s)\n");
	puts("+re: run program recursively on all sub-directories.");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		stout = TRUE;
		onlydata = 1;
//		AddCEXExtension = FALSE;
		OverWriteFile = TRUE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		root_ids = NULL;
		root_htiers = NULL;
		local_tiers = NULL;
	}
}

static struct IDs *ID_cleanup(struct IDs *p) {
	struct IDs *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_id;
		if (t->l1 != NULL)
			free(t->l1);
		if (t->language != NULL)
			free(t->language);
		if (t->corpus != NULL)
			free(t->corpus);
		if (t->code != NULL)
			free(t->code);
		if (t->age != NULL)
			free(t->age);
		if (t->sex != NULL)
			free(t->sex);
		if (t->group != NULL)
			free(t->group);
		if (t->SES != NULL)
			free(t->SES);
		if (t->role != NULL)
			free(t->role);
		if (t->education != NULL)
			free(t->education);
		if (t->file_unique != NULL)
			free(t->file_unique);
		free(t);
	}
	return(NULL);
}

static struct htiers *tiers_cleanup(struct htiers *root) {
	struct htiers *t;
	
	while (root != NULL) {
		t = root;
		root = root->next_tier;
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
	root_ids = ID_cleanup(root_ids);
	root_htiers  = tiers_cleanup(root_htiers);
	local_tiers  = tiers_cleanup(local_tiers);
}
		
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'r':
			if (*f == 'e') {
				f++;
				no_arg_option(f);
				isRecursive = TRUE;
				break;
			}
		default:
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
	}
}

static void PSLC_overflow() {
	root_ids = ID_cleanup(root_ids);
	root_htiers  = tiers_cleanup(root_htiers);
	local_tiers  = tiers_cleanup(local_tiers);
	fprintf(stderr,"PSLC: no more memory available.\n");
	cutt_exit(0);
}

static char *PSLC_strsave(char *s) {
	int  i;
	char *p;
	
	for (i=0; isSpace(s[i]); i++) ;
	if (i > 0)
		strcpy(s, s+i);
	uS.remblanks(s);
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		PSLC_overflow();
	return(p);
}

static void err_diff_data(char *mess, char *sp, char *oldd, char *newd) {
	fputs("----------------------------------------\n",stderr);
	fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
	fprintf(stderr, "Data discrepancy in field: %s %s:\n    \"%s\" - \"%s\"\n", mess, sp, oldd, newd);
}

static void add_to_ID(char *l1, char *lang,char *corp,char *code,char *age,char *sex,char *group,char *SES,char *role,char *educ, char *FU) {
	struct IDs *t;
	
	if (code == NULL)
		return;
	
	if (root_ids == NULL) {
		if ((root_ids=NEW(struct IDs)) == NULL)
			PSLC_overflow();
		t = root_ids;
		t->next_id = NULL;
		t->isRightSp = FALSE;
		t->l1 = NULL;
		t->language = NULL;
		t->corpus = NULL;
		t->code = NULL;
		t->age = NULL;
		t->sex = NULL;
		t->group = NULL;
		t->SES = NULL;
		t->role = NULL;
		t->education = NULL;
		t->file_unique = NULL;
		t->code = PSLC_strsave(code);
	} else {
		for (t=root_ids; t->next_id != NULL; t=t->next_id) {
			if (uS.mStricmp(t->code, code) == 0)
				break;
		}
		if (uS.mStricmp(t->code, code) != 0) {
			t->next_id = NEW(struct IDs);
			t = t->next_id;
			if (t == NULL)
				PSLC_overflow();
			t->next_id = NULL;
			t->isRightSp = FALSE;
			t->l1 = NULL;
			t->language = NULL;
			t->corpus = NULL;
			t->code = NULL;
			t->age = NULL;
			t->sex = NULL;
			t->group = NULL;
			t->SES = NULL;
			t->role = NULL;
			t->education = NULL;
			t->file_unique = NULL;
			t->code = PSLC_strsave(code);
		}
	}
	if (l1 != NULL) {
		uS.remblanks(l1);
		if (t->l1 != NULL)
			free(t->l1);
		t->l1 = PSLC_strsave(l1);
	}
	if (lang != NULL) {
		uS.remblanks(lang);
		if (t->language == NULL)
			t->language = PSLC_strsave(lang);
		else if (!strcmp(t->language, "en")) {
			free(t->language);
			t->language = PSLC_strsave(lang);
		} else if (uS.mStricmp(lang, t->language))
			err_diff_data("Language of", t->code, lang, t->language);
	}
	if (corp != NULL) {
		uS.remblanks(corp);
		if (t->corpus == NULL)
			t->corpus = PSLC_strsave(corp);
		else if (uS.mStricmp(corp, t->corpus))
			err_diff_data("corpus", t->code, corp, t->corpus);
	}
	if (age != NULL) {
		uS.remblanks(age);
		if (t->age == NULL)
			t->age = PSLC_strsave(age);
		else if (uS.mStricmp(age, t->age))
			err_diff_data("Age of", t->code, age, t->age);
	}
	if (sex != NULL) {
		uS.remblanks(sex);
		if (t->sex == NULL)
			t->sex = PSLC_strsave(sex);
		else if (uS.mStricmp(sex, t->sex))
			err_diff_data("Sex of", t->code, sex, t->sex);
	}
	if (group != NULL) {
		uS.remblanks(group);
		if (t->group == NULL)
			t->group = PSLC_strsave(group);
		else if (uS.mStricmp(group, t->group))
			err_diff_data("Group of", t->code, group, t->group);
	}
	if (SES != NULL) {
		uS.remblanks(SES);
		if (t->SES == NULL)
			t->SES = PSLC_strsave(SES);
		else if (uS.mStricmp(SES, t->SES))
			err_diff_data("Ses of", t->code, SES, t->SES);
	}
	if (role != NULL) {
		uS.remblanks(role);
		if (t->role == NULL)
			t->role = PSLC_strsave(role);
		else if (uS.mStricmp(role, t->role))
			err_diff_data("Role of", t->code, role, t->role);
	}
	if (educ != NULL) {
		uS.remblanks(educ);
		if (t->education == NULL)
			t->education = PSLC_strsave(educ);
		else if (uS.mStricmp(educ, t->education))
			err_diff_data("Education of", t->code, educ, t->education);
	}
	if (FU != NULL) {
		uS.remblanks(FU);
		t->file_unique = PSLC_strsave(FU);
	}
}

static void handleParticipants(char *line) {
	char sp[SPEAKERLEN];
	char *s, *e, t, wc, tchFound;
	short cnt = 0;
	
	for (; *line && (*line == ' ' || *line == '\t'); line++) ;
	s = line;
	tchFound = FALSE;
	sp[0] = EOS;
	while (*s) {
		if (*line == ','  || *line == ' '  ||
			*line == '\t' || *line == '\n' || *line == EOS) {
			wc = ' ';
			e = line;
			for (; *line == ' ' || *line == '\t' || *line == '\n'; line++) {
			}
			if (*line != ',' && *line != EOS)
				line--;
			else
				wc = ',';
			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == ',') {
					add_to_ID(NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL);
				} else if (cnt == 0) {
					strcpy(sp, s);
					add_to_ID(NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				}
				*e = t;
				if (wc == ',') {
					cnt = -1;
					sp[0] = EOS;
				}
				for (line++; *line==' ' || *line=='\t' || *line=='\n' || *line==','; line++) {
					if (*line == ',') {
						cnt = -1;
						sp[0] = EOS;
					}
				}
			} else {
				for (line=e; *line; line++) {
				}
				if (cnt != 0) {
					t = *e;
					*e = EOS;
					add_to_ID(NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL);
					*e = t;
				}
				for (line=e; *line; line++) {
				}
			}
			if (cnt == 2) {
				cnt = 0;
				sp[0] = EOS;
			} else
				cnt++;
			s = line;
		} else
			line++;
	}
}

static char handleIDs(char *line) {
	int t, s = 0, e = 0, cnt;
	char sp[SPEAKERLEN];
	char word[SPEAKERLEN], *st;
	
	word[0] = EOS;
	sp[0] = EOS;
	while (line[s] != EOS) {
		if (!isSpace(line[s]))
			break;
		s++;
	}
	if (line[s] == EOS)
		return(FALSE);
	t = s;
	cnt = 0;
	while (1) {
		st = word;
		while ((*st=line[s]) == '|' || isSpace(line[s])) {
			if (line[s] == EOS)
				break;
			if (line[s] == '|')
				cnt++;
			s++;
		}
		if (*st == EOS)
			break;
		e = s + 1;
		while ((*++st=line[e]) != EOS) {
			e++;
			if (line[e-1] == '|')
				break;
		}
		*st = EOS;
		if (cnt == 0) {			// language
		} else if (cnt == 1) {	// corpus
		} else if (cnt == 2) {	// code
			strcpy(sp, word);
			break;
		}
		s = e;
		cnt++;
	}
	if (sp[0] == EOS)
		return(FALSE);
	word[0] = EOS;
	s = t;
	cnt = 0;
	while (1) {
		st = word;
		while ((*st=line[s]) == '|' || isSpace(line[s])) {
			if (line[s] == EOS)
				break;
			if (line[s] == '|')
				cnt++;
			s++;
		}
		if (*st == EOS)
			break;
		e = s + 1;
		while ((*++st=line[e]) != EOS) {
			e++;
			if (line[e-1] == '|')
				break;
		}
		*st = EOS;
		if (cnt == 0) {			// language
			add_to_ID(NULL, word, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 1) {	// corpus
			add_to_ID(NULL, NULL, word, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 2) {	// code
			
		} else if (cnt == 3) {	// age
			add_to_ID(NULL, NULL, NULL, sp, word, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 4) {	// sex
			add_to_ID(NULL, NULL, NULL, sp, NULL, word, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 5) {	// group
			add_to_ID(NULL, NULL, NULL, sp, NULL, NULL, word, NULL, NULL, NULL, NULL);
		} else if (cnt == 6) {	// SES
			add_to_ID(NULL, NULL, NULL, sp, NULL, NULL, NULL, word, NULL, NULL, NULL);
		} else if (cnt == 7) {	// role
			add_to_ID(NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, word, NULL, NULL);
		} else if (cnt == 8) {	// education
			add_to_ID(NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, word, NULL);
		} else if (cnt == 9) {	// file_unique
			add_to_ID(NULL, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, word);
		}
		s = e;
		cnt++;
	}
	
	return(TRUE);
}

static struct htiers *add_to_htier(struct htiers *root, char *sp, AttTYPE *attSp, char *line, AttTYPE *attLine) {
	struct htiers *t;

	if (root == NULL) {
		if ((root=NEW(struct htiers)) == NULL)
			PSLC_overflow();
		t = root;
	} else {
		for (t=root; t->next_tier != NULL; t=t->next_tier) ;
		if ((t->next_tier=NEW(struct htiers)) == NULL)
			PSLC_overflow();
		t = t->next_tier;
	}
	t->next_tier = NULL;
	att_cp(0, t->sp, sp, t->attSp, attSp);
	att_cp(0, t->line, line, t->attLine, attLine);
	return(root);
}

static void resetIDs(struct IDs *t) {
	for (; t != NULL; t=t->next_id) {
		t->isRightSp = FALSE;
	}	
}

static void add_to_languages(char *languages, char *line) {
	char *e, lang[100];
	char *bl;
	
	while (*line != EOS) {
		e = strchr(line, ',');
		if (e != NULL) {
			*e = EOS;
			strcpy(lang, line);
			*e = ',';
			line = e + 1;
		} else {
			strcpy(lang, line);
			line = line + strlen(line);
		}
		for (bl=languages; *bl != EOS;) {
			e = strchr(bl, ',');
			if (e != NULL) {
				*e = EOS;
				if (uS.mStricmp(lang, bl) == 0)
					break;
				*e = ',';
				bl = e + 1;
			} else {
				if (uS.mStricmp(lang, bl) == 0)
					break;
				bl = bl + strlen(bl);
			}
		}
		if (languages[0] == EOS) {
			strcat(languages, lang);
		} else if (*bl == EOS) {
			strcat(languages, ",");
			strcat(languages, lang);
		}
	}
}

static void printHeaders(struct IDs *root, char *mediaFileName) {
	struct IDs *t;

	templineC[0] = EOS;
	for (t=root; t != NULL; t=t->next_id) {
		if (t->isRightSp && t->language[0] != EOS)
			add_to_languages(templineC, t->language);
	}
	printout("@Languages:",templineC,NULL,NULL,TRUE);
	templineC[0] = EOS;
	for (t=root; t != NULL; t=t->next_id) {
		if (t->isRightSp) {
			if (templineC[0] != EOS)
				strcat(templineC, ", ");
			strcat(templineC, t->code);
			strcat(templineC, " ");
			strcat(templineC, t->role);
		}
	}
	printout("@Participants:",templineC,NULL,NULL,TRUE);
	for (t=root; t != NULL; t=t->next_id) {
		if (t->isRightSp) {
			templineC[0] = EOS;
			if (t->language == NULL)
				strcat(templineC, "en");
			else
				strcat(templineC, t->language);
			strcat(templineC, "|");
			if (t->corpus == NULL || t->corpus[0] == EOS)
				strcat(templineC, "change_corpus_later");
			else
				strcat(templineC, t->corpus);
			strcat(templineC, "|");
			strcat(templineC, t->code);
			strcat(templineC, "|");
			if (t->age != NULL)
				strcat(templineC, t->age);
			strcat(templineC, "|");
			if (t->sex != NULL)
				strcat(templineC, t->sex);
			strcat(templineC, "|");
			if (t->group != NULL)
				strcat(templineC, t->group);
			strcat(templineC, "|");
			if (t->SES != NULL)
				strcat(templineC, t->SES);
			strcat(templineC, "|");
			if (t->role != NULL)
				strcat(templineC, t->role);
			strcat(templineC, "|");
			if (t->education != NULL)
				strcat(templineC, t->education);
			strcat(templineC, "|");
			if (t->file_unique != NULL)
				strcat(templineC, t->file_unique);
			strcat(templineC, "|");
			printout("@ID:",templineC,NULL,NULL,TRUE);
		}
	}
	fprintf(fpout, "%s\t%s, audio\n", MEDIAHEADER, mediaFileName);
	for (t=root; t != NULL; t=t->next_id) {
		if (t->isRightSp && t->l1 != NULL && t->l1[0] != EOS) {
			sprintf(templineC, "@L1 of %s:\t%s",  t->code, t->l1);
			fprintf(fpout, "%s\n", templineC);
		}
	}
}

static void markID(struct IDs *t, char *sp) {
	int len;

	strcpy(templineC, sp+1);
	uS.remblanks(templineC);
	len = strlen(templineC) - 1;
	if (len >= 0 && templineC[len] == ':')
		templineC[len] = EOS;
	for (; t != NULL; t=t->next_id) {
		if (uS.mStricmp(t->code, templineC) == 0) {
			t->isRightSp = TRUE;
			break;
		}
	}
	if (t == NULL) {
		root_ids = ID_cleanup(root_ids);
		root_htiers  = tiers_cleanup(root_htiers);
		local_tiers  = tiers_cleanup(local_tiers);
		fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, lineno);
		fprintf(stderr,"Speaker \"%s\" not declaired in @ID or @Participants.\n", sp);
		cutt_exit(0);
	}
}

void call() {
	int  i, j;
	char qf, isBGFound;
	FILE *fp, *tfp;
	long tlineno = 0L;
	FNType tFileName[FNSize], mediaFileName[FILENAME_MAX];
	struct htiers *t;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		tlineno = tlineno + 1L;
		if (tlineno % PERIOD == 0)
			fprintf(stderr,"\r%ld ",tlineno);
		if (utterance->speaker[0] == '*' || uS.partcmp(utterance->speaker, "@Bg:", FALSE, FALSE))
			break;
		if (uS.partcmp(utterance->speaker, "@Begin", FALSE, FALSE)) {
		} else if (uS.partcmp(utterance->speaker, "@UTF8", FALSE, FALSE)) {
		} else if (uS.partcmp(utterance->speaker, "@Languages", FALSE, FALSE)) {
//			add_to_ID(NULL, utterance->line, NULL, templineC4, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (uS.patmat(utterance->speaker,"@Language of *")) {
			uS.extractString(templineC4, utterance->speaker, "@Language of ", ':');
			add_to_ID(utterance->line, NULL, NULL, templineC4, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (uS.partcmp(utterance->speaker, PARTICIPANTS, FALSE, FALSE)) {
			handleParticipants(utterance->line);
		} else if (uS.patmat(utterance->speaker,AGEOF)) {
			uS.extractString(templineC4, utterance->speaker, "@Age of ", ':');
			add_to_ID(NULL, NULL, NULL, templineC4, utterance->line, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (uS.patmat(utterance->speaker,SEXOF)) {
			uS.extractString(templineC4, utterance->speaker, "@Sex of ", ':');
			add_to_ID(NULL, NULL, NULL, templineC4, NULL, utterance->line, NULL, NULL, NULL, NULL, NULL);
		} else if (uS.patmat(utterance->speaker,GROUPOF)) {
			uS.extractString(templineC4, utterance->speaker, "@Group of ", ':');
			add_to_ID(NULL, NULL, NULL, templineC4, NULL, NULL, utterance->line, NULL, NULL, NULL, NULL);
		} else if (uS.patmat(utterance->speaker,SESOF)) {
			uS.extractString(templineC4, utterance->speaker, "@Ses of ", ':');
			add_to_ID(NULL, NULL, NULL, templineC4, NULL, NULL, NULL, utterance->line, NULL, NULL, NULL);
		} else if (uS.patmat(utterance->speaker,EDUCOF)) {
			uS.extractString(templineC4, utterance->speaker, "@Education of ", ':');
			add_to_ID(NULL, NULL, NULL, templineC4, NULL, NULL, NULL, NULL, NULL, utterance->line, NULL);
		} else if (uS.partcmp(utterance->speaker, "@ID", FALSE, FALSE)) {
			if (!handleIDs(utterance->line))
				root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
		} else
			root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
	}
	fp = NULL;
	isBGFound = FALSE;
	local_tiers = NULL;
	do {
		if (uS.partcmp(utterance->speaker, "@Bg:", FALSE, FALSE)) {
			isBGFound = TRUE;
			resetIDs(root_ids);
			if (local_tiers != NULL) {
				root_ids = ID_cleanup(root_ids);
				root_htiers  = tiers_cleanup(root_htiers);
				local_tiers  = tiers_cleanup(local_tiers);
				fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"Missing @Eg: tier around\n");
				cutt_exit(0);
			}
			uS.remblanks(utterance->line);
			strcpy(mediaFileName, utterance->line);
			if (extractPath(tFileName, oldfname))
				addFilename2Path(tFileName, utterance->line);
			else
				strcpy(tFileName, utterance->line);
			strcat(tFileName, ".cha.cex");
			fp = fopen(tFileName, "w");
			if (fp == NULL) {
				root_ids = ID_cleanup(root_ids);
				root_htiers  = tiers_cleanup(root_htiers);
				local_tiers  = tiers_cleanup(local_tiers);
				fprintf(fpout,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"Can't open file: %s\n", tFileName);
				cutt_exit(0);
			}
		} else if (uS.partcmp(utterance->speaker, "@Eg:", FALSE, FALSE)) {
			isBGFound = FALSE;
			tfp = fpout;
			fpout = fp;
			fprintf(fpout, "@UTF8\n");
			fprintf(fpout, "@Begin\n");
			printHeaders(root_ids, mediaFileName);
			for (t=root_htiers; t != NULL; t=t->next_tier) {
				printout(t->sp,t->line,t->attSp,t->attLine,FALSE);
			}		
			for (t=local_tiers; t != NULL; t=t->next_tier) {
				printout(t->sp,t->line,t->attSp,t->attLine,FALSE);
			}
			fprintf(fpout, "@End\n");
			local_tiers  = tiers_cleanup(local_tiers);
			fclose(fpout);
			fpout = tfp;
			fp = NULL;
		} else {
			if (utterance->speaker[0] == '*')
				markID(root_ids, utterance->speaker);
			for (i=0L; utterance->line[i]; i++) {
				if (utterance->line[i] == HIDEN_C) {
					i++;
					if (utterance->line[i] == '%' && (utterance->line[i+1] == 's' || utterance->line[i+1] == 'm')) {
						qf = FALSE;
						for (j=i; qf || !isdigit(utterance->line[j]); j++) {
							if (utterance->line[j] == HIDEN_C || utterance->line[j] == EOS)
								break;
							if (utterance->line[j] == '"')
								qf = !qf;
						}
						if (isdigit(utterance->line[j])) {
							att_cp(0, utterance->line+i, utterance->line+j, utterance->attLine+i, utterance->attLine+j);
						}
					}
				}
			}
			if (isBGFound)
				local_tiers = add_to_htier(local_tiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
		}
		tlineno = tlineno + 1L;
		if (tlineno % PERIOD == 0)
			fprintf(stderr,"\r%ld ",tlineno);
	} while (getwholeutter()) ;
	fprintf(stderr,"\n");
	root_ids = ID_cleanup(root_ids);
	root_htiers  = tiers_cleanup(root_htiers);
}
