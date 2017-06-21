/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3
#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main insert_main
#define call insert_call
#define getflag insert_getflag
#define init insert_init
#define usage insert_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define PERIOD 50

struct IDs {
	char *language; // @Languages:, @Language of #:
	char *corpus;   // such as: MacWhinney, Bates, Sachs, etc...
	char *code;		// @Participants: *CHI
	char *age;		// @Age of #:
	char *sex;		// @Sex of #:
	char *group;	// @Group of #:
	char *SES;		// @Ses of #:
	char *role;		// @Participants: Target_Child
	char *education;// @Education of #:
	char *custom_field;//
	struct IDs *next_id;
} ;

struct htiers {
    char sp[SPEAKERLEN];			/* code descriptor field of the turn	*/
    AttTYPE attSp[SPEAKERLEN];		/* Attributes assiciated with speaker name	*/
    char line[UTTLINELEN+1];		/* text field of the turn		*/ // found uttlinelen
    AttTYPE attLine[UTTLINELEN+1];	/* Attributes assiciated with text*/
	struct htiers *next_tier;
} ;

static char corpus[512];
static char auto_corpus[512];
static struct IDs *root_ids;
static struct htiers *root_htiers, *languages, *participants;
extern char OverWriteFile;

void usage() {
	printf("Usage: insert [+cS %s] filename(s)\n", mainflgs());
	puts("+cS: Specify the name of corpus (default: file name use as corpus name)");
	mainusage(FALSE);
	puts("\nExample: insert +c\"corpus\" text.cha");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		onlydata = 1;
		root_ids = NULL;
		root_htiers = NULL;
		languages = NULL;
		participants = NULL;
		corpus[0] = EOS;
		auto_corpus[0] = EOS;
	}
}

static void ID_cleanup(void) {
	struct IDs *t;

	while (root_ids != NULL) {
		t = root_ids;
		root_ids = root_ids->next_id;
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
		if (t->custom_field != NULL)
			free(t->custom_field);
		free(t);
	}
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
	CLAN_PROG_NUM = INSERT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	ID_cleanup();
	root_htiers  = tiers_cleanup(root_htiers);
	languages    = tiers_cleanup(languages);
	participants = tiers_cleanup(participants);
}

static void insert_overflow() {
	ID_cleanup();
	root_htiers  = tiers_cleanup(root_htiers);
	languages    = tiers_cleanup(languages);
	participants = tiers_cleanup(participants);
	fprintf(stderr,"Insert: no more memory available.\n");
	cutt_exit(0);
}

static char *insert_strsave(char *s) {
	int  i;
	char *p;

	for (i=0; isSpace(s[i]); i++) ;
	if (i > 0)
		strcpy(s, s+i);
	uS.remblanks(s);
	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p, s);
	else
		insert_overflow();
	return(p);
}

static void insert_cleanup_lang(char *lang) {
	while (*lang) {
		if (isSpace(*lang))
			strcpy(lang, lang+1);
		else if (!uS.mStrnicmp(lang, "legacy,", 7))
			strcpy(lang, lang+7);
		else if (!uS.mStrnicmp(lang, "legacy", 6))
			strcpy(lang, lang+6);
		else if (!uS.mStrnicmp(lang, "ca,", 3))
			strcpy(lang, lang+3);
		else if (!uS.mStrnicmp(lang, "ca", 2))
			strcpy(lang, lang+2);
		else
			lang++;
	}
	if (*(lang-1) == ',')
		strcpy(lang-1, lang);
}

static void err_diff_data(const char *mess, const char *sp, const char *oldd, const char *newd) {
	fputs("----------------------------------------\n",stderr);
	fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
	fprintf(stderr, "Data discrepancy in field: %s %s:\n    \"%s\" - \"%s\"\n", mess, sp, oldd, newd);
}

static void cleanUpLangusges(char *line, char *tline) {
	long i, j;

	for (i=0L,j=0L; line[i] != EOS; ) {
		if (line[i] == '=') {
			for (; !isSpace(line[i]) && line[i] != ',' && line[i] != '\n' && line[i] != EOS; i++) ;
		} else {
			tline[j] = line[i];
			j++;
			i++;
		}
	}
	tline[j] = EOS;
}

static void add_to_ID(char *lang,char *corp,char *code,char *age,char *sex,char *group,char *SES,char *role,char *educ, char *FU) {
	struct IDs *t;

	if (code == NULL)
		return;

	if (root_ids == NULL) {
		if ((root_ids=NEW(struct IDs)) == NULL)
			insert_overflow();
		t = root_ids;
		t->next_id = NULL;
		t->language = NULL;
		t->corpus = NULL;
		t->code = NULL;
		t->age = NULL;
		t->sex = NULL;
		t->group = NULL;
		t->SES = NULL;
		t->role = NULL;
		t->education = NULL;
		t->custom_field = NULL;
		t->code = insert_strsave(code);
		if (languages != NULL) {
			cleanUpLangusges(languages->line, templineC4);
			t->language = insert_strsave(templineC4);
			insert_cleanup_lang(t->language);
		}
		if (corpus[0] != EOS) 
			t->corpus = insert_strsave(corpus);
		else if (auto_corpus[0] != EOS)
			t->corpus = insert_strsave(auto_corpus);
	} else {
		for (t=root_ids; t->next_id != NULL; t=t->next_id) {
			if (uS.mStricmp(t->code, code) == 0)
				break;
		}
		if (uS.mStricmp(t->code, code) != 0) {
			t->next_id = NEW(struct IDs);
			t = t->next_id;
			if (t == NULL)
				insert_overflow();
			t->next_id = NULL;
			t->language = NULL;
			t->corpus = NULL;
			t->code = NULL;
			t->age = NULL;
			t->sex = NULL;
			t->group = NULL;
			t->SES = NULL;
			t->role = NULL;
			t->education = NULL;
			t->custom_field = NULL;
			t->code = insert_strsave(code);
			if (languages != NULL) {
				cleanUpLangusges(languages->line, templineC4);
				t->language = insert_strsave(templineC4);
				insert_cleanup_lang(t->language);
			}
			if (corpus[0] != EOS) 
				t->corpus = insert_strsave(corpus);
			else if (auto_corpus[0] != EOS)
				t->corpus = insert_strsave(auto_corpus);
		}
	}
	if (lang != NULL) {
		if (t->language != NULL)
			free(t->language);
		uS.remblanks(lang);
		insert_cleanup_lang(lang);
		t->language = insert_strsave(lang);
	}
	if (corp != NULL) {
		uS.remblanks(corp);
		if (t->corpus == NULL)
			t->corpus = insert_strsave(corp);
		else if (uS.mStricmp(auto_corpus, corpus) != 0 && uS.mStricmp(auto_corpus, t->corpus) == 0) {
			free(t->corpus);
			t->corpus = insert_strsave(corp);
		} else if (corpus[0] == EOS && uS.mStricmp(corp, t->corpus))
			err_diff_data("corpus", t->code, corp, t->corpus);
	}
	if (age != NULL) {
		uS.remblanks(age);
		if (t->age == NULL)
			t->age = insert_strsave(age);
		else if (uS.mStricmp(age, t->age))
			err_diff_data("Age of", t->code, age, t->age);
	}
	if (sex != NULL) {
		uS.remblanks(sex);
		if (t->sex == NULL)
			t->sex = insert_strsave(sex);
		else if (uS.mStricmp(sex, t->sex))
			err_diff_data("Sex of", t->code, sex, t->sex);
	}
	if (group != NULL) {
		uS.remblanks(group);
		if (t->group == NULL)
			t->group = insert_strsave(group);
		else if (uS.mStricmp(group, t->group))
			err_diff_data("Group of", t->code, group, t->group);
	}
	if (SES != NULL) {
		uS.remblanks(SES);
		if (t->SES == NULL)
			t->SES = insert_strsave(SES);
		else if (uS.mStricmp(SES, t->SES))
			err_diff_data("Ses of", t->code, SES, t->SES);
	}
	if (role != NULL) {
		uS.remblanks(role);
		if (t->role == NULL)
			t->role = insert_strsave(role);
		else if (uS.mStricmp(role, t->role))
			err_diff_data("Role of", t->code, role, t->role);
	}
	if (educ != NULL) {
		uS.remblanks(educ);
		if (t->education == NULL)
			t->education = insert_strsave(educ);
		else if (uS.mStricmp(educ, t->education))
			err_diff_data("Education of", t->code, educ, t->education);
	}
	if (FU != NULL) {
		uS.remblanks(FU);
		t->custom_field = insert_strsave(FU);
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
					add_to_ID(NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL);
				} else if (cnt == 0) {
					strcpy(sp, s);
					add_to_ID(NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
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
					add_to_ID(NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL);
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
			add_to_ID(word, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 1) {	// corpus
			add_to_ID(NULL, word, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 2) {	// code

		} else if (cnt == 3) {	// age
			add_to_ID(NULL, NULL, sp, word, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 4) {	// sex
			add_to_ID(NULL, NULL, sp, NULL, word, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 5) {	// group
			add_to_ID(NULL, NULL, sp, NULL, NULL, word, NULL, NULL, NULL, NULL);
		} else if (cnt == 6) {	// SES
			add_to_ID(NULL, NULL, sp, NULL, NULL, NULL, word, NULL, NULL, NULL);
		} else if (cnt == 7) {	// role
			add_to_ID(NULL, NULL, sp, NULL, NULL, NULL, NULL, word, NULL, NULL);
		} else if (cnt == 8) {	// education
			add_to_ID(NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, word, NULL);
		} else if (cnt == 9) {	// custom_field
			add_to_ID(NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, word);
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
			insert_overflow();
		t = root;
	} else {
		for (t=root; t->next_tier != NULL; t=t->next_tier) ;
		if ((t->next_tier=NEW(struct htiers)) == NULL)
			insert_overflow();
		t = t->next_tier;
	}
	t->next_tier = NULL;
	att_cp(0, t->sp, sp, t->attSp, attSp);
	att_cp(0, t->line, line, t->attLine, attLine);
	return(root);
}

void call() {
	char isFirstSpeaker = FALSE;
	long tlineno = 0L;
	struct htiers *t;
	struct IDs *tID;

/*
	char *c;
	strcpy(auto_corpus, oldfname);
	c = strrchr(auto_corpus, '.');
	if (c != NULL)
		*c = EOS;
*/
	strcpy(auto_corpus, "change_me_later");
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (!stout) {
			tlineno = tlineno + 1L;
			if (tlineno % PERIOD == 0)
				fprintf(stderr,"\r%ld ",tlineno);
		}
		if (isFirstSpeaker || utterance->speaker[0] == '*') {
			if (!isFirstSpeaker) {
				for (t=root_htiers; t != NULL; t=t->next_tier) {
					if (uS.partcmp(t->sp, "@Begin", FALSE, FALSE)   ||
						uS.isUTF8(t->sp) ||
						uS.partcmp(t->sp, FONTHEADER, FALSE, FALSE) ||
						uS.partcmp(t->sp, CKEYWORDHEADER, FALSE, FALSE)) {
						printout(t->sp,t->line,t->attSp,t->attLine,FALSE);
						t->sp[0] = EOS;
					}
				}
				if (languages != NULL)
					printout(languages->sp,languages->line,languages->attSp,languages->attLine,FALSE);
				else {
				}
				if (participants != NULL)
					printout(participants->sp,participants->line,participants->attSp,participants->attLine,FALSE);
				for (tID=root_ids; tID != NULL; tID=tID->next_id) {
					fprintf(fpout, "@ID:\t");
					if (tID->language != NULL)
						fprintf(fpout, "%s|", tID->language);
					else
						fprintf(fpout, "en|");

					if (tID->corpus != NULL)
						fprintf(fpout, "%s|", tID->corpus);
					else
						fprintf(fpout, "|");

					if (tID->code != NULL)
						fprintf(fpout, "%s|", tID->code);
					else
						fprintf(fpout, "|");

					if (tID->age != NULL)
						fprintf(fpout, "%s|", tID->age);
					else
						fprintf(fpout, "|");

					if (tID->sex != NULL)
						fprintf(fpout, "%s|", tID->sex);
					else
						fprintf(fpout, "|");

					if (tID->group != NULL)
						fprintf(fpout, "%s|", tID->group);
					else
						fprintf(fpout, "|");

					if (tID->SES != NULL)
						fprintf(fpout, "%s|", tID->SES);
					else
						fprintf(fpout, "|");

					if (tID->role != NULL)
						fprintf(fpout, "%s|", tID->role);
					else
						fprintf(fpout, "|");

					if (tID->education != NULL)
						fprintf(fpout, "%s|", tID->education);
					else
						fprintf(fpout, "|");
					if (tID->custom_field != NULL)
						fprintf(fpout, "%s|", tID->custom_field);
					else
						fprintf(fpout, "|");
					fprintf(fpout, "\n");
				}
				for (t=root_htiers; t != NULL; t=t->next_tier) {
					if (t->sp[0] != EOS)
						printout(t->sp,t->line,t->attSp,t->attLine,FALSE);
				}
			}
			isFirstSpeaker = TRUE;
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			if (uS.partcmp(utterance->speaker, "@Languages", FALSE, FALSE)) {
				languages = add_to_htier(languages, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else if (uS.patmat(utterance->speaker,"@Language of *")) {
/*
				uS.extractString(templineC4, utterance->speaker, "@Language of ", ':');
				add_to_ID(utterance->line, NULL, templineC4, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
*/
				root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else if (uS.partcmp(utterance->speaker, PARTICIPANTS, FALSE, FALSE)) {
				handleParticipants(utterance->line);
				participants = add_to_htier(participants, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else if (uS.patmat(utterance->speaker,AGEOF)) {
				uS.extractString(templineC4, utterance->speaker, "@Age of ", ':');
				add_to_ID(NULL, NULL, templineC4, utterance->line, NULL, NULL, NULL, NULL, NULL, NULL);
//				root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else if (uS.patmat(utterance->speaker,SEXOF)) {
				uS.extractString(templineC4, utterance->speaker, "@Sex of ", ':');
				add_to_ID(NULL, NULL, templineC4, NULL, utterance->line, NULL, NULL, NULL, NULL, NULL);
//				root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else if (uS.patmat(utterance->speaker,GROUPOF)) {
				uS.extractString(templineC4, utterance->speaker, "@Group of ", ':');
				add_to_ID(NULL, NULL, templineC4, NULL, NULL, utterance->line, NULL, NULL, NULL, NULL);
//				root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else if (uS.patmat(utterance->speaker,SESOF)) {
				uS.extractString(templineC4, utterance->speaker, "@Ses of ", ':');
				add_to_ID(NULL, NULL, templineC4, NULL, NULL, NULL, utterance->line, NULL, NULL, NULL);
//				root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else if (uS.patmat(utterance->speaker,EDUCOF)) {
				uS.extractString(templineC4, utterance->speaker, "@Education of ", ':');
				add_to_ID(NULL, NULL, templineC4, NULL, NULL, NULL, NULL, NULL, utterance->line, NULL);
//				root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else if (uS.partcmp(utterance->speaker, "@ID", FALSE, FALSE)) {
				if (!handleIDs(utterance->line))
					root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
			} else
				root_htiers = add_to_htier(root_htiers, utterance->speaker, utterance->attSp, utterance->line, utterance->attLine);
		}
	}

	if (!stout)
		fprintf(stderr,"\n");
	ID_cleanup();
	root_htiers  = tiers_cleanup(root_htiers);
	languages    = tiers_cleanup(languages);
	participants = tiers_cleanup(participants);
}

void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'c':
			strcpy(corpus, getfarg(f,f1,i));
			break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}
