/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// change date to current: <oai:datestamp>2008-04-04</oai:datestamp>

#define CHAT_MODE 4
#include "cu.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main cmdi_main
#define call cmdi_call
#define getflag cmdi_getflag
#define init cmdi_init
#define usage cmdi_usage
#define mkdir my_mkdir
#else
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

#if defined(UNX) || defined(_MAC_CODE)
#define MODE S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH
#endif
#ifdef _WIN32
#define MODE 0
#endif


extern char OverWriteFile;
extern char isRecursive;
extern struct tier *defheadtier;

/*
	Title:	AphasiaBank Legacy Olness Corpus
	Creator:	Olness, Gloria
	Subject:	aphasia
	Subject.olac:linguistic-field:	neurolinguistics
	Subject.olac:language:	en
	Type:	Sound
	Type:	moving image
	Description:	 conversations with English aphasics
	Publisher:	TalkBank
	Contributor:	
	Date:		2005
	Type:	Text
	Type.olac:linguistic-type:	primary_text
	Type.olac:discourse-type:	dialogue
	Format:		 
	Identifier:	1-59642-253-X
	Language:	
	Relation:
	Coverage:
	Rights:
	IMDI_Genre:	discourse
	IMDI_Interactivity:	interactive
	IMDI_PlanningType:	spontaneous
	IMDI_Involvement:	non-elicited
	IMDI_SocialContext:	family
	IMDI_EventStructure:	conversation
	IMDI_Channel:	face to face
	IMDI_Task:	unspecified
	IMDI_Modalities:	Spoken
	IMDI_Subject:	unspecified
	IMDI_EthnicGroup:	unspecified
	IMDI_RecordingConditions:	unspecified
	IMDI_AccessAvailability:	open access, "https://creativecommons.org/licenses/by-nc-sa/3.0/"
	IMDI_Continent:	Europe
	IMDI_Country:	United Kingdom
	IMDI_WrittenResourceSubType:	SubType info
	IMDI_ProjectDescription:	some One Guy text
	IMDI_MediaFileDescription:	22050 Hz, 16 bit, 1 channel
 	IMDI_PID: 11312/t-00001784-1
	DOI:	doi:10.21415/T5TP59
*/

#define PID_SIZE	64
#define LANGNUM		10
#define LANGLEN		10

#define CODESIZE	20
#define IDFIELSSIZE 100

#define TOTANUMLANGS 64

#define CMDIIDSTYPE struct IDSs
struct IDSs {
	char lang[LANGNUM][LANGLEN+1];	// @Languages:, @Language of #:
	char corpus[IDFIELSSIZE+1];		// such as: MacWhinney, Bates, Sachs, etc...
	char code[CODESIZE+1];			// @Participants: *CHI
	char age[IDFIELSSIZE+1];		// @Age of #:
	char sex;						// @Sex of #:
	char group[IDFIELSSIZE+1];		// @Group of #:
	char SES[IDFIELSSIZE+1];		// @Ses of #:
	char role[IDFIELSSIZE+1];		// @Participants: Target_Child
	char education[IDFIELSSIZE+1];	// @Education of #:
	char custom_field[IDFIELSSIZE+1];// file name or other unique ID
	char spname[IDFIELSSIZE+1];		// @Participants: Jane
	char birth[IDFIELSSIZE+1];		// @Birth of #
	char Llang[LANGNUM];			// @L1 of #
	char BirthPlace[IDFIELSSIZE+1];	// @Birthplace of #
	struct IDSs *next_id;
} ;

#define METADATA struct metadatas
struct metadatas {
	char *tag;
	char *data;
	struct metadatas *next;
} ;

#define PATHTREE struct Path_Tree
struct Path_Tree {
	char *name;
	int  sl;
	char PIDp;
	long PID;
	struct Path_Tree *child;
	struct Path_Tree *sibling;
} ;

#define DUPPIDLIST struct dup_PID_list
struct dup_PID_list {
	char *fname;
	char PIDp;
	long PID;
	struct dup_PID_list *next;
} ;

struct ServersArr {
	const char *dataURL;
	const char *gitName;
	const char *serverName;
	const char *mediaName;
	const char *serverInfo;
};

static struct ServersArr serversList[] = {
//           URL                               GIT             Server Name    Media Name     Description
	/* 0*/	{"http://dali.talkbank.org/",      "",             "",            "",            ""}, // this line has to be in 0 position
	/* 1*/	{"https://aphasia.talkbank.org/",  "aphasia-data", "AphasiaBank", "aphasia",     "AphasiaBank is the component of TalkBank dedicated to data on language in aphasia"},
	/* 2*/	{"https://asd.talkbank.org/",      "asd-data",     "ASDBank",     "asd",         "ASDBank is the component of TalkBank dedicate to data on language in autism"},
	/* 3*/	{"https://biling.talkbank.org/",   "biling-data",  "BilingBank",  "biling",      "BilingBank is the component of TalkBank dedicate to data on language in bilingualism"},
	/* 4*/	{"https://ca.talkbank.org/",       "ca-data",      "CABank",      "ca",          "CABank is the component of TalkBank dedicate to Conversation Analysis"},
	/* 5*/	{"https://childes.talkbank.org/",  "childes-data", "CHILDES",     "childes",     "CHILDES is the component of TalkBank dedicated to child language data"},
	/* 6*/	{"https://class.talkbank.org/",    "class-data",   "ClassBank",   "class",       "ClassBank is the component of TalkBank dedicate to classroom discourse"},
	/* 7*/	{"https://dementia.talkbank.org/", "dementia-data","DementiaBank","dementia",    "DementiaBank is the component of TalkBank dedicated to data on language in dementia"},
	/* 8*/	{"https://fluency.talkbank.org/",  "fluency-data", "FluencyBank", "fluency",     "FluencyBank is the component of TalkBank dedicated to data on language in stuttering"},
	/* 9*/	{"https://homebank.talkbank.org/", "homebank-data","HomeBank",    "homebank",    "HomeBank is the component of TalkBank dedicated to data on daylong recording in the home"},
	/*10*/	{"https://phonbank.talkbank.org/", "phon-data",    "PhonBank",    "phonbank",    "PhonBank is the component of TalkBank dedicated to data on child phonology"},
	/*11*/	{"https://rhd.talkbank.org/",      "rhd-data",     "RHDBank",     "rhd",         "RHDBank is the component of TalkBank dedicated to data on language in right hemisphere damage"},
	/*12*/	{"https://samtalebank.talkbank.org/","samtale-data","SamtaleBank","samtalebank","SamtaleBank is the component of TalkBank dedicated to data on conversations in Danish"},
	/*13*/	{"https://slabank.talkbank.org/",  "slabank-data", "SLABank",     "slabank",     "SLABank is the component of TalkBank dedicate to second language learning"},
	/*14*/	{"https://tbi.talkbank.org/",      "tbi-data",     "TBIBank",     "tbi",         "TBIBank is the component of TalkBank dedicated to data on language in traumatic brain injury"},
	/*15*/	{"https://psychosis.talkbank.org",      "psychosis-data",     "PsychosisBank",     "psychosis",      "PsychosisBank is the component of TalkBank dedicated to data on language in psychosis"},
	/*16*/	{NULL, NULL, NULL, NULL, NULL}
} ;
/*
 URL								GIT				Media		Web Page
 0 http://dali.talkbank.org/
 1 https://aphasia.talkbank.org		aphasia-data	aphasia 	AphasiaBank
 2 https://asd.talkbank.org			asd-data		asd			ASDBank
 3 https://biling.talkbank.org		biling-data		biling		BilingBank
 4 https://ca.talkbank.org			ca-data			ca			CABank
 5 https://childes.talkbank.org		childes-data	childes		CHILDES
 6 https://class.talkbank.org		class-data		class		ClassBank
 7 https://dementia.talkbank.org	dementia-data	dementia	DementiaBank
 8 https://fluency.talkbank.org		fluency-data	fluency		FluencyBank
 9 https://homebank.talkbank.org	homebank-data	homebank	HomeBank
 10 https://phonbank.talkbank.org	phon-data		phonbank	PhonBank
 11 https://rhd.talkbank.org		rhd-data		rhd			RHDBank
 12 https://samtalebank.talkbank.org samtale-data	samtalebank	SamtaleBank
 13 https://slabank.talkbank.org	slabank-data	slabank		SLABank
 14 https://tbi.talkbank.org		tbi-data		tbi			TBIBank
 15 https://psychosis.talkbank.org	psychosis-data	psychosis	PsychosisBank

 */

static char currentURL[BUFSIZ+2], LandingPage[BUFSIZ+2];
static char ftime, ftimeAdjust;
static char isJustTest, showAll, isPIDDupTest;
static char cur_date[50];
static char missingLangs[TOTANUMLANGS][4];
static char isMissingLang[TOTANUMLANGS];
static int missingLangsIndex;
static int wdStartI, wdCurStartI;
static int serverCnt;
static long tln = 0L, cln = 0L, PIDcnt, PIDtodo, filesChanged;
static PATHTREE *tree_root;
static DUPPIDLIST *root_PID;
static FILE *errorFp, *PIDFp;

static void getCurTime(char *st) {
	time_t timer;
	struct tm *timeS;
	time(&timer);
	timeS = localtime(&timer);
	sprintf(cur_date, "%d-%s%d-%s%d", timeS->tm_year+1900,
			((timeS->tm_mon+1) < 10 ? "0":""), timeS->tm_mon+1, ((timeS->tm_mday) < 10 ? "0":""), timeS->tm_mday);
}

static PATHTREE *free_tree(PATHTREE *p) {
	PATHTREE *sib, *t;

	if (p != NULL) {
		if (p->child != NULL)
			free_tree(p->child);
		sib = p->sibling;
		while (sib != NULL) {
			if (sib->child != NULL)
				free_tree(sib->child);
			t = sib;
			sib = sib->sibling;
			if (t->name != NULL)
				free(t->name);
			free(t);
		}
		if (p->name != NULL)
			free(p->name);
		free(p);
	}
	return(NULL);
}

static DUPPIDLIST *free_PID_list(DUPPIDLIST *p) {
	DUPPIDLIST *t;

	while (p != NULL) {
		if (p->fname != NULL)
			free(p->fname);
		t = p;
		p = p->next;
		free(t);
	}
	return(NULL);
}

static void free_metadata(METADATA *p) {
	METADATA *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		if (t->tag != NULL)
			free(t->tag);
		if (t->data != NULL)
			free(t->data);
		free(t);
	}
}

void init(char f) {
	if (f) {
		showAll = FALSE;
		isJustTest = TRUE;
		isPIDDupTest = FALSE;
		cur_date[0] = EOS;
		isRecursive = TRUE;
		stout = TRUE;
		combinput = TRUE;
		onlydata = 1;
		tree_root = NULL;
		ftime = TRUE;
		ftimeAdjust = TRUE;
		PIDtodo = 0L;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		AddCEXExtension = ".xml";
		if (defheadtier != NULL) {
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (cur_date[0] == EOS)
			getCurTime(cur_date);
	}
}

void usage() {
	printf("Usage: cmdi [c dS l] filename(s)\n");
	puts("+c : Add @PID: headers to CHAT files, if needed.");
	puts("     (default: test data/media consistency and needed PIDs, NO *.cmdi files created).");
	puts("+dS: specify date in YYYY-MM-DD.");
	puts("+l : show where in CHAT files all languages missing from ISO-639.cut file occur.");
	puts("+p : check for duplicate PIDs in files.");
	puts("\nInstructions:");
	puts("  In Unix Terminal \"cd\" to every server's repo and type:");
	puts("git status");
	puts("git pull");
	puts("  It would be best to run CHATTER on \"~/data/\" folder at this time too");
	puts("\n  In CLAN's \"Commands\" window:");
	puts("  Set working directory to \"~/data/\"");
	puts("  Run command to check for duplicate PIDs run command:");
	puts("cmdi +p *.cha");
	puts("  If errors found, then fix them and run above command again");
	puts("  If no duplicate PIDs found, then run command:");
	puts("cmdi *.cha");
	puts("  If languages are missing, then run the following command to get files list:");
	puts("cmdi -l *.cha");
	puts("\nAfter there are no more errors reported by \"cmdi\" do the following:");
	puts("1  Run Franklin's XML validator on \"data-cmdi\" folder by using this command");
	puts("validate-xml ~/data-cmdi 2> log.txt");
	puts("   XML validator is at: https://github.com/FranklinChen/validate-xml-rust");
	puts("2  Check for errors in the log.txt file by using this command:");
	puts("grep -v validates log.txt");
	puts("3  If there are errors found by XML validator, then fix them");
	puts("4  In CLAN's \"Commands\" window run \"cmdi *.cha\" again");
	puts("5  Repeat from step 1.");
	puts("Continue to the next step only if XML validator validates all .cmdi files without any errors.");
	puts("\nNow create CMDI scripts and add PIDs to database data files:");
	puts("  In CLAN's \"Commands\" window run command:");
	puts("cmdi +c *.cha");
	puts("  In Unix Terminal type:");
	puts("  run CHATTER on \"~/data/\" folder");
	puts("If CHATTER finds error, then fix them anyway you can including deleting repos and starting from scratch");
	puts("\nProceed only if CHATTER does not find any errors");
	puts("cd <to every server's repo>");
	puts("git status");
	puts("git commit -a");
	puts("deploy");
	puts("\tAT THIS POINT TELL LEONID TO FINISH TRANSFER DATA TO DALI");
	puts("*********************************************************************");
	puts("\tTransfer CMDI files inside \"data-cmdi\" folder to \"dali.talkbank.org\" server to folder \"/var/www/web/data-cmdi/\"");
	puts("\nAdding PIDs to Handle Server");
	puts("\tTransfer file \"~/data/0PID_batch.txt\" to \"Mac Gabby\" server to folder \"/Users/WORK/CLAN-data/Handle/hs\"");
	puts("After above files are transferred to \"dali.talkbank.org\" server and Mac Gabby they can be deleted.");
	puts("\nConnect to \"dali.talkbank.org\" with command \"ssh macw@dali.talkbank.org\"");
	puts("\n  In Unix Terminal on \"dali.talkbank.org\" type:");
	puts("/var/www/hs/stop.sh");
	puts("cd /var/www/hs/svr_1");
	puts("First backup and then delete folders \"bdbje\", \"txns\" and file \"txn_id\" to \"/var/www/hs/bck\"");
	puts("/var/www/hs/start.sh&");
	puts("Transfer file \"/var/www/hs/svr_1/admpriv.bin\" from \"dali.talkbank.org\" to \"Mac Gabby\" folder \"~/Downloads/admpriv.bin\"");
	puts("\n  In Unix Terminal on \"Mac Gabby\" type:");
	puts("cd /Users/WORK/CLAN-data/Handle/hs/bin/");
	puts("./hdl-admintool");
	puts("  Choose the menu option \"Tools->Home/Unhome Prefix\"");
	puts("  In the \"Prefix\" box enter \"0.NA/11312\"");
	puts("  Under \"By Site Info File (siteinfo.bin)\" click \"Choose File...\"");
	puts("  Select \"siteinfo.json\" file from directory: \"/Users/WORK/CLAN-data/Handle/hs/siteinfo.json\"");
	puts("  Click \"Do It\" button, in next window click \"OK\" button, Then enter password \"kgb1984\"");
	puts("\nStill inside \"hdl-admintool\" create new PIDs:");
	puts("  Choose the menu option \"Tools->Batch Processor\"");
	puts("  In \"Batch Processor\" window click \"Add\" button and select file \"0PID_batch.txt\"");
	puts("  Click on \"Run Batch(es)\" button and hopefully it all works....");
	cleanupLanguages();
	cutt_exit(0);
}

static void writeError(const char *err) {
	if (errorFp == NULL) {
		strcpy(FileName1, wd_dir);
		addFilename2Path(FileName1, "0error.cut");
		errorFp = fopen(FileName1, "w");
	}
	if (errorFp != NULL)
		fprintf(errorFp, "%s", err);
	else
		fprintf(stderr, "%s", err);
}

static void generateHDL(char *st, long PID) {
	if (PID < 10L)
		sprintf(st, "0000000%ld", PID);
	else if (PID < 100L)
		sprintf(st, "000000%ld", PID);
	else if (PID < 1000L)
		sprintf(st, "00000%ld", PID);
	else if (PID < 10000L)
		sprintf(st, "0000%ld", PID);
	else if (PID < 100000L)
		sprintf(st, "000%ld", PID);
	else if (PID < 1000000L)
		sprintf(st, "00%ld", PID);
	else if (PID < 10000000L)
		sprintf(st, "0%ld", PID);
	else
		sprintf(st, "%ld", PID);
}

static void getServerURL(int c, char *out) {
	if (c >= 0 && c < serverCnt) {
		strcpy(out, serversList[c].dataURL);
	} else {
		fprintf(stderr, "\n   Unrecognized repo name code == 0.\n");
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		cutt_exit(0);
	}
}

static void getMediaServerURL(int c, char *out) {
	if (c > 0 && c < serverCnt) {
		strcpy(out, "https://media.talkbank.org/");
		strcat(out, serversList[c].mediaName);
	} else {
		fprintf(stderr, "\n   Unrecognized repo name code == 0.\n");
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		cutt_exit(0);
	}
}

static int getCodeForRepoName(char *name) {
	int i;
	char *s;

	for (i=1; serversList[i].dataURL != NULL; i++) {
		if (uS.mStrnicmp(name, serversList[i].gitName, strlen(serversList[i].gitName)) == 0)
			return(i);
	}
	if (serversList[i].dataURL == NULL) {
		s = strchr(name, '/');
		if (s != NULL)
			*s = EOS;
		fprintf(stderr, "\n   Unrecognized repo name: \"%s\"\n", name);
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		cutt_exit(0);
	}
	return(0);
}

static void removeServernameAddFilename2Path(char *out, char *in) {
	int i, len;

	if (*in == PATHDELIMCHR)
		in++;

	for (i=1; serversList[i].dataURL != NULL; i++) {
		len = strlen(serversList[i].gitName);
		if (uS.mStrnicmp(in, serversList[i].gitName, len) == 0) {
			addFilename2Path(out, in+len);
			return;
		}
	}

	addFilename2Path(out, in);
}

static void outputServerTitle(FILE *fp, int c, METADATA *mContributor) {
	if (c == 0) {
		fprintf(stderr, "\n   Internal error p->sl == 0.\n");
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		cutt_exit(0);
	} else if (c > 0 && c < serverCnt) {
		if (mContributor != NULL)
			fprintf(fp, "          <Name>%s</Name>\n", mContributor->data);
		else
			fprintf(fp, "          <Name>%s</Name>\n", serversList[c].serverName);
		fprintf(fp, "          <Title>%s</Title>\n", serversList[c].serverName);
	} else {
		if (mContributor != NULL)
			fprintf(fp, "          <Name>%s</Name>\n", mContributor->data);
		else
			fprintf(fp, "          <Name>%s</Name>\n", "Unspecified");
		fprintf(fp, "          <Title>%s</Title>\n", "Unspecified");
	}
}

static void outputServerDescription(FILE *fp, int c) {
	if (c == 0) {
		fprintf(stderr, "\n   Internal error p->sl == 0.\n");
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		cutt_exit(0);
	} else if (c > 0 && c < serverCnt) {
		fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", serversList[c].serverInfo);
	} else {
		fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", "Unspecified");
	}
}

static unsigned char writePIDbatch(const char *URL, char *PIDs, char PIDp, long PID, unsigned char PIDc) {
	if (PID == 0) {
		fprintf(stderr, "\n   Internal error PID == 0.\n");
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		cutt_exit(0);
	}
	PIDs[0] = EOS;
	if (PIDFp == NULL) {
		strcpy(FileName1, wd_dir);
		addFilename2Path(FileName1, "0PID_batch.txt");
		PIDFp = fopen(FileName1, "w");
		if (PIDFp == NULL) {
			fprintf(stderr, "\n   ERROR: Can't create file \"%s\".\n", FileName1);
			return(0);
		}
	}
	generateHDL(templineC3, PID);
	sprintf(PIDs, "11312/%c-%s-%d", PIDp, templineC3, PIDc);
	fprintf(PIDFp, "CREATE %s\n", PIDs);
	fprintf(PIDFp, "100 HS_ADMIN 86400 1110 ADMIN 200:111111111111:0.NA/11312\n");
	fprintf(PIDFp, "3 URL 86400 1110 UTF8 %s\n\n", URL);
	PIDtodo++;
	PIDc++;
	return(PIDc);
}

static METADATA *add2MetadataList(METADATA *mdata, char *h, char *l) {
	char *s;
	METADATA *p;

	if (mdata == NULL) {
		p = NEW(METADATA);
		mdata = p;
	} else {
		for (p=mdata; p->next != NULL; p=p->next) {
			if (uS.mStricmp(p->tag, h) == 0) {
				s = p->data;
				if (s != NULL) {
					p->data = (char *)malloc(strlen(l)+strlen(s)+3);
					if (p->data == NULL) {
						tree_root = free_tree(tree_root);
						free_metadata(mdata);
						cleanupLanguages();
						fprintf(stderr, "Out of Memory!!");
						cutt_exit(0);
					}
					strcpy(p->data, s);
					strcat(p->data, "; ");
					strcat(p->data, l);
					free(s);
					return(mdata);
				}
			}
		}
		if (uS.mStricmp(p->tag, h) == 0) {
			s = p->data;
			if (s != NULL) {
				p->data = (char *)malloc(strlen(l)+strlen(s)+3);
				if (p->data == NULL) {
					tree_root = free_tree(tree_root);
					free_metadata(mdata);
					cleanupLanguages();
					fprintf(stderr, "Out of Memory!!");
					cutt_exit(0);
				}
				strcpy(p->data, s);
				strcat(p->data, ", ");
				strcat(p->data, l);
				free(s);
				return(mdata);
			}
		}
		p->next = NEW(METADATA);
		p = p->next;
	}
	if (p == NULL) {
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	p->next = NULL;
	p->tag = NULL;
	p->data = NULL;
	p->tag = (char *)malloc(strlen(h)+1);
	if (p->tag == NULL) {
		tree_root = free_tree(tree_root);
		free_metadata(mdata);
		cleanupLanguages();
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	strcpy(p->tag, h);
	p->data = (char *)malloc(strlen(l)+1);
	if (p->data == NULL) {
		tree_root = free_tree(tree_root);
		free_metadata(mdata);
		cleanupLanguages();
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	strcpy(p->data, l);
	return(mdata);
}

static void filterTextForXML(char *an, char *bs) {
	long i;
	
	i = 0L;
	for (; *bs != EOS; bs++) {
		if (*bs == '&') {
			strcpy(an+i, "&amp;");
			i = strlen(an);
		} else if (*bs == '"') {
			strcpy(an+i, "&quot;");
			i = strlen(an);
		} else if (*bs == '\'') {
			strcpy(an+i, "&apos;");
			i = strlen(an);
		} else if (*bs == '<') {
			strcpy(an+i, "&lt;");
			i = strlen(an);
		} else if (*bs == '>') {
			strcpy(an+i, "&gt;");
			i = strlen(an);
		} else if (*bs == '\n')
			an[i++] = ' ';
		else if (*bs == '\t')
			an[i++] = ' ';
		else if (*bs >= 0 && *bs < 32) {
			sprintf(an+i,"{0x%x}", *bs);
			i = strlen(an);
		} else
			an[i++] = *bs;
	}
	an[i] = EOS;
}

static char isHeadTerm(char *id, int i) {
	int j;

	if (id[i] != ':')
		return(FALSE);
	id[i] = EOS;
	if (!strcmp(id, "Title")) {
	} else if (!strcmp(id, "Creator")) {
	} else if (!strcmp(id, "Subject")) {
	} else if (!strcmp(id, "Subject.olac:language")) {
	} else if (!strcmp(id, "Subject.olac:linguistic-field")) {
	} else if (!strcmp(id, "Subject.childes:participant")) {
	} else if (!strcmp(id, "Description")) {
	} else if (!strcmp(id, "Publisher")) {
	} else if (!strcmp(id, "Contributor")) {
	} else if (!strcmp(id, "Date")) {
	} else if (!strcmp(id, "Type")) {
	} else if (!strcmp(id, "Type.olac:linguistic-type")) {
	} else if (!strcmp(id, "Type.olac:discourse-type")) {
	} else if (!strcmp(id, "Format")) {
	} else if (!strcmp(id, "Identifier")) {
	} else if (!strcmp(id, "Language")) {
	} else if (!strcmp(id, "Relation")) {
	} else if (!strcmp(id, "Coverage")) {
	} else if (!strcmp(id, "Rights")) {
	} else if (!strcmp(id, "IMDI_Genre")) {
	} else if (!strcmp(id, "IMDI_Interactivity")) {
	} else if (!strcmp(id, "IMDI_PlanningType")) {
	} else if (!strcmp(id, "IMDI_Involvement")) {
	} else if (!strcmp(id, "IMDI_SocialContext")) {
	} else if (!strcmp(id, "IMDI_EventStructure")) {
	} else if (!strcmp(id, "IMDI_Channel")) {
	} else if (!strcmp(id, "IMDI_Task")) {
	} else if (!strcmp(id, "IMDI_Modalities")) {
	} else if (!strcmp(id, "IMDI_Subject")) {
	} else if (!strcmp(id, "IMDI_EthnicGroup")) {
	} else if (!strcmp(id, "IMDI_RecordingConditions")) {
	} else if (!strcmp(id, "IMDI_AccessAvailability")) {
	} else if (!strcmp(id, "IMDI_Continent")) {
	} else if (!strcmp(id, "IMDI_Country")) {
	} else if (!strcmp(id, "IMDI_WrittenResourceSubType")) {
	} else if (!strcmp(id, "IMDI_ProjectDescription")) {
	} else if (!strcmp(id, "IMDI_MediaFileDescription")) {
	} else if (!strcmp(id, "DOI")) {
	} else if (!strcmp(id, CMDIPIDHEADER)) {
	} else {
		id[i] = ':';
		if (isSpace(id[i+1])) {
			for (j=0; j < i; j++) {
				if (isSpace(id[j]) || id[j] == '\n')
					break;
			}
			if (j == i)
				return(TRUE);
			else
				return(FALSE);
		} else
			return(FALSE);
	}
	id[i] = ':';
	return(TRUE);
}

static METADATA *parseFields(METADATA *mdata, char *line, char *fname, long ln) {
	long i, k;
	char *h, *l;
	
	for (i=0; line[i] != EOS && !isHeadTerm(line, i); i++) ;
	h = line;
	if (line[i] == EOS)
		l = line+i;
	else {
		line[i] = EOS;
		for (i=i+1; isSpace(line[i]); i++) ;
		l = line+i;
	}
	filterTextForXML(templineC2, l);
	strcpy(l, templineC2);
	if (!strcmp(h, "Title")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Creator")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Subject")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Subject.olac:language")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Subject.olac:linguistic-field")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Subject.childes:participant")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Description")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Publisher")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Contributor")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Date")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Type")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Type.olac:linguistic-type")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Type.olac:discourse-type")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Format")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Identifier")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Language")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Relation")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Coverage")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "Rights")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Genre")) {
		if (l[0] != EOS) {
			if (islower(*l))
				*l = toupper(*l);
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Interactivity")) {
		if (l[0] != EOS) {
			if (*l == 'u')
				*l = 'U';
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_PlanningType")) {
		if (l[0] != EOS) {
			if (*l == 'u')
				*l = 'U';
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Involvement")) {
		if (l[0] != EOS) {
			if (*l == 'u')
				*l = 'U';
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_SocialContext")) {
		if (l[0] != EOS) {
			if (islower(*l))
				*l = toupper(*l);
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_EventStructure")) {
		if (l[0] != EOS) {
			if (islower(*l))
				*l = toupper(*l);
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Channel")) {
		if (l[0] != EOS) {
			if (islower(*l))
				*l = toupper(*l);
			if (uS.mStricmp(l, "face to face") == 0)
				strcpy(l, "Face to Face");
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Task")) {
		if (l[0] != EOS) {
			if (*l == 'u')
				*l = 'U';
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Modalities")) {
		if (l[0] != EOS) {
			if (*l == 'u')
				*l = 'U';
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Subject")) {
		if (l[0] != EOS) {
			if (*l == 'u')
				*l = 'U';
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_EthnicGroup")) {
		if (l[0] != EOS) {
			if (islower(*l))
				*l = toupper(*l);
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_RecordingConditions")) {
		if (l[0] != EOS) {
			if (islower(*l))
				*l = toupper(*l);
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_AccessAvailability")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Continent")) {
		if (l[0] != EOS) {
			for (k=0; l[k] != EOS; k++) {
				if (l[k] == ' ')
					l[k] = '-';
			}
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_Country")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_WrittenResourceSubType")) {
		if (l[0] != EOS) {
			if (*l == 'u')
				*l = 'U';
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_ProjectDescription")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "IMDI_MediaFileDescription")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, "DOI")) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (!strcmp(h, CMDIPIDHEADER)) {
		if (l[0] != EOS) {
			mdata = add2MetadataList(mdata, h, l);
		}
	} else if (h[0] != EOS) {
		sprintf(templineC2, "*** File \"%s\": line %ld.\nIllegal field \"%s\" found.\n", fname, ln, h);
		writeError(templineC2);
	}
	return(mdata);
}

static METADATA *readMetadata(PATHTREE *p, char *wd_cur, const char *name) {
	int  i;
	long ln;
	FILE *fp;
	METADATA *mdata;

	getServerURL(p->sl, LandingPage);
	addFilename2Path(LandingPage, "access");
	removeServernameAddFilename2Path(LandingPage, wd_cur+wdCurStartI);
	strcat(LandingPage, ".html");
	strcpy(FileName1, wd_dir);
	addFilename2Path(FileName1, wd_cur+wdCurStartI);
	addFilename2Path(FileName1, name);
	mdata = NULL;
	fp = fopen(FileName1, "r");
	if (fp == NULL) {
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open file \"%s\".\n", FileName1);
		cutt_exit(0);
	} else {
		ln = 0L;
		templineC[0] = EOS;
		while (fgets_cr(templineC1, UTTLINELEN, fp)) {
			if (uS.isUTF8(templineC1) || uS.isInvisibleHeader(templineC1))
				continue;
			ln++;
			if (!strcmp(templineC1,"\n"))
				continue;
			if (templineC1[0] == '#')
				break;
			uS.remblanks(templineC1);
			for (i=0; templineC1[i] != EOS && !isHeadTerm(templineC1, i); i++) ;
			if (templineC1[i] == ':') {
				if (templineC[0] != EOS)
					mdata = parseFields(mdata, templineC, FileName1, ln-1);
				strcpy(templineC, templineC1);
			} else if (templineC[0] != EOS) {
				strcat(templineC, templineC1);
			}
		}
		if (templineC[0] != EOS)
			mdata = parseFields(mdata, templineC, FileName1, ln);
		fclose(fp);
	}
	return(mdata);
}

static void getFileDateSize(FNType *fn, char *date, char *size) {
#ifdef _MAC_CODE
	FSCatalogInfo catalogInfo;
	FSRef  ref;
	CFAbsoluteTime oCFTime;
	
	*date = 0L;
	my_FSPathMakeRef(fn, &ref); 
	if (FSGetCatalogInfo(&ref, kFSCatInfoContentMod, &catalogInfo, NULL, NULL, NULL) == noErr) {
		if (UCConvertUTCDateTimeToCFAbsoluteTime(&catalogInfo.contentModDate, &oCFTime) == noErr) {
			// write a header of sorts
			// Date format: YYYY '-' MM '-' DD ' ' hh ':' mm ':' ss.fff
			CFTimeZoneRef tz = CFTimeZoneCopySystem();    // specifically choose system time zone for logs
			CFGregorianDate gdate = CFAbsoluteTimeGetGregorianDate(oCFTime, tz);
			sprintf(date, "%ld-%s%d-%s%d", gdate.year, ((gdate.month) < 10 ? "0":""), gdate.month, ((gdate.day) < 10 ? "0":""), gdate.day);
			CFRelease(tz);
		}
		if (FSGetCatalogInfo(&ref, kFSCatInfoDataSizes, &catalogInfo, NULL, NULL, NULL) == noErr) {
			sprintf(size, "%lluKB", catalogInfo.dataPhysicalSize/1000);
		}
	}
#endif
#ifdef UNX
	struct tm* timeS;				// create a time structure
	struct stat attrib;			// create a file attribute structure

	stat(fn, &attrib);		// get the attributes of afile.txt
	timeS = gmtime(&(attrib.st_mtime));	// Get the last modified time and put it into the time structure
	sprintf(date, "%d-%s%d-%s%d", timeS->tm_year+1900,
			((timeS->tm_mon+1) < 10 ? "0":""), timeS->tm_mon+1, ((timeS->tm_mday) < 10 ? "0":""), timeS->tm_mday);
	sprintf(size, "%ldKB", (long)attrib.st_size / 1000);
#endif
}

static void clean_ids(CMDIIDSTYPE *p) {
	CMDIIDSTYPE *t;
	
	while (p != NULL) {
		t = p;
		p = p->next_id;
		free(t);
	}
}

static void convertDate(char *dest, char *birth) {
	char *s;

	dest[0] = EOS;
	s = strrchr(birth, '-');
	if (s == NULL)
		return;
	*s = EOS;
	strcpy(dest, s+1);
	s = strrchr(birth, '-');
	if (s == NULL) {
		dest[0] = EOS;
		return;
	}
	*s = EOS;
	strcat(dest, "-");
	if (!uS.mStricmp(s+1,"JAN"))
		strcat(dest, "01-");
	else if (!uS.mStricmp(s+1,"FEB"))
		strcat(dest, "02-");
	else if (!uS.mStricmp(s+1,"MAR"))
		strcat(dest, "03-");
	else if (!uS.mStricmp(s+1,"APR"))
		strcat(dest, "04-");
	else if (!uS.mStricmp(s+1,"MAY"))
		strcat(dest, "05-");
	else if (!uS.mStricmp(s+1,"JUN"))
		strcat(dest, "06-");
	else if (!uS.mStricmp(s+1,"JUL"))
		strcat(dest, "07-");
	else if (!uS.mStricmp(s+1,"AUG"))
		strcat(dest, "08-");
	else if (!uS.mStricmp(s+1,"SEP"))
		strcat(dest, "09-");
	else if (!uS.mStricmp(s+1,"OCT"))
		strcat(dest, "10-");
	else if (!uS.mStricmp(s+1,"NOV"))
		strcat(dest, "11-");
	else if (!uS.mStricmp(s+1,"DEC"))
		strcat(dest, "12-");
	else {
		dest[0] = EOS;
		return;
	}
	if (strlen(birth) == 1)
		strcat(dest, "0");
	strcat(dest, birth);
}

static CMDIIDSTYPE *add_to_IDs(CMDIIDSTYPE *rootIDs, METADATA *cmdata, char *fname, long ln, char *lang,char *corp,char *code,char *age,char *sex,
						   char *group,char *SES,char *role,char *educ,char*fu,char *spn,char *birth,char *Llang, char *BirthPlace) {
	int i;
	char *e, tlang[LANGLEN+1];
	CMDIIDSTYPE *p;
	
	if (code == NULL)
		return(rootIDs);
	
	uS.remblanks(code);
	uS.uppercasestr(code, NULL, 0);
	if (rootIDs == NULL) {
		if ((rootIDs=NEW(CMDIIDSTYPE)) == NULL)
			return(NULL);
		p = rootIDs;
		for (i=0; i < LANGNUM; i++)
			p->lang[i][0] = EOS;
		p->corpus[0] = EOS;
		p->code[0] = EOS;
		p->age[0] = EOS;
		p->sex = 0;
		p->group[0] = EOS;
		p->SES[0] = EOS;
		p->role[0] = EOS;
		p->education[0] = EOS;
		p->custom_field[0] = EOS;
		p->spname[0] = EOS;
		p->birth[0] = EOS;
		for (i=0; i < LANGNUM; i++)
			p->Llang[i] = FALSE;
		p->BirthPlace[0] = EOS;
		p->next_id = NULL;
	} else {
		for (p=rootIDs; p->next_id != NULL; p=p->next_id) {
			if (uS.mStricmp(p->code, code) == 0)
				break;
		}
		if (uS.mStricmp(p->code, code) != 0) {
			p->next_id = NEW(CMDIIDSTYPE);
			p = p->next_id;
			if (p == NULL) {
				clean_ids(rootIDs);
				tree_root = free_tree(tree_root);
				free_metadata(cmdata);
				cleanupLanguages();
				fprintf(stderr, "\n   Out of Memory.\n");
				cutt_exit(0);
			}
			for (i=0; i < LANGNUM; i++)
				p->lang[i][0] = EOS;
			p->corpus[0] = EOS;
			p->code[0] = EOS;
			p->age[0] = EOS;
			p->sex = 0;
			p->group[0] = EOS;
			p->SES[0] = EOS;
			p->role[0] = EOS;
			p->education[0] = EOS;
			p->custom_field[0] = EOS;
			p->spname[0] = EOS;
			p->birth[0] = EOS;
			for (i=0; i < LANGNUM; i++)
				p->Llang[i] = FALSE;
			p->BirthPlace[0] = EOS;
			p->next_id = NULL;
		}
	}
	if (p->code[0] == EOS) {
		uS.remblanks(code);
		filterTextForXML(templineC2, code);
		strncpy(p->code, templineC2, CODESIZE);
		p->code[CODESIZE] = EOS;
	}
	if (lang != NULL) {
		uS.remblanks(lang);
		while (*lang != EOS) {
			e = strchr(lang, ',');
			if (e != NULL) {
				*e = EOS;
				for (; isSpace(*lang); lang++) ;
				strncpy(tlang, lang, LANGLEN);
				tlang[LANGLEN] = EOS;
				uS.remblanks(tlang);
				*e = ',';
				lang = e + 1;
			} else {
				for (; isSpace(*lang); lang++) ;
				strncpy(tlang, lang, LANGLEN);
				tlang[LANGLEN] = EOS;
				uS.remblanks(tlang);
				lang = lang + strlen(lang);
			}
			uS.remblanks(tlang);
			for (i=0; i < LANGNUM; i++) {
				if (uS.mStricmp(tlang, p->lang[i]) == 0)
					break;
			}
			if (i >= LANGNUM) {
				for (i=0; i < LANGNUM && p->lang[i][0] != EOS; i++) ;
				if (i < LANGNUM) {
					strcpy(p->lang[i], tlang);
				}
			}
		}
	}
	if (corp != NULL) {
		uS.remblanks(corp);
		filterTextForXML(templineC2, corp);
		strncpy(p->corpus, templineC2, IDFIELSSIZE);
		p->corpus[IDFIELSSIZE] = EOS;
	}
	if (age != NULL) {
		uS.remblanks(age);
		filterTextForXML(templineC2, age);
		i = strlen(templineC2) - 1;
		if (templineC2[i] == '.' || templineC2[i] == ';')
			templineC2[i] = EOS;
		for (i=0; templineC2[i] != EOS; ) {
			if (templineC2[i] == '-') {
				templineC2[i] = '/';
				i++;
			} else if (isSpace(templineC2[i]))
				strcpy(templineC2+i, templineC2+i+1);
			else if ((templineC2[i] == '.' || templineC2[i] == ';') && (templineC2[i+1] == '-' || isSpace(templineC2[i+1])))
				strcpy(templineC2+i, templineC2+i+1);
			else
				i++;
		}
		strncpy(p->age, templineC2, IDFIELSSIZE);
		p->age[IDFIELSSIZE] = EOS;
	}
	if (sex != NULL) {
		if (sex[0] == 'm' || sex[0] == 'M')
			p->sex = 'm';
		else if (sex[0] == 'f' || sex[0] == 'F')
			p->sex = 'f';
		else
			p->sex = 0;
	}
	if (group != NULL) {
		uS.remblanks(group);
		filterTextForXML(templineC2, group);
		strncpy(p->group, templineC2, IDFIELSSIZE);
		p->group[IDFIELSSIZE] = EOS;
	}
	if (SES != NULL) {
		uS.remblanks(SES);
		filterTextForXML(templineC2, SES);
		strncpy(p->SES, templineC2, IDFIELSSIZE);
		p->SES[IDFIELSSIZE] = EOS;
	}
	if (role != NULL) {
		uS.remblanks(role);
		filterTextForXML(templineC2, role);
		strncpy(p->role, templineC2, IDFIELSSIZE);
		p->role[IDFIELSSIZE] = EOS;
	}
	if (educ != NULL) {
		uS.remblanks(educ);
		filterTextForXML(templineC2, educ);
		strncpy(p->education, templineC2, IDFIELSSIZE);
		p->education[IDFIELSSIZE] = EOS;
	}
	if (fu != NULL) {
		uS.remblanks(fu);
		filterTextForXML(templineC2, fu);
		strncpy(p->custom_field, templineC2, IDFIELSSIZE);
		p->custom_field[IDFIELSSIZE] = EOS;
	}
	if (spn != NULL) {
		uS.remblanks(spn);
		filterTextForXML(templineC2, spn);
		strncpy(p->spname, templineC2, IDFIELSSIZE);
		p->spname[IDFIELSSIZE] = EOS;
	}
	if (birth != NULL) {
		uS.remblanks(birth);
		convertDate(templineC2, birth);
		strncpy(p->birth, templineC2, IDFIELSSIZE);
		p->birth[IDFIELSSIZE] = EOS;
	}
	if (Llang != NULL) {
		uS.remblanks(Llang);
		while (*Llang != EOS) {
			e = strchr(Llang, ',');
			if (e != NULL) {
				*e = EOS;
				for (; isSpace(*Llang); Llang++) ;
				strncpy(tlang, Llang, LANGLEN);
				tlang[LANGLEN] = EOS;
				*e = ',';
				Llang = e + 1;
			} else {
				for (; isSpace(*Llang); Llang++) ;
				strncpy(tlang, Llang, LANGLEN);
				tlang[LANGLEN] = EOS;
				Llang = Llang + strlen(Llang);
			}
			uS.remblanks(tlang);
			for (i=0; i < LANGNUM; i++) {
				if (uS.mStricmp(tlang, p->lang[i]) == 0) {
					p->Llang[i] = TRUE;
					break;
				}
			}
			if (i >= LANGNUM) {
				for (i=0; i < LANGNUM && p->lang[i][0] != EOS; i++) ;
				if (i < LANGNUM) {
					strcpy(p->lang[i], tlang);
					p->Llang[i] = TRUE;
				}
			}
		}
	}
	if (BirthPlace != NULL) {
		uS.remblanks(BirthPlace);
		filterTextForXML(templineC2, BirthPlace);
		strncpy(p->BirthPlace, templineC2, IDFIELSSIZE);
		p->BirthPlace[IDFIELSSIZE] = EOS;
	}
	return(rootIDs);
}

static CMDIIDSTYPE *handleParticipants(CMDIIDSTYPE *rootIDs, METADATA *cmdata, char *fname, long ln, char *line) {
	char sp[SPEAKERLEN];
	char *s, *e, t, wc, tchFound;
	short cnt = 0;
	
	for (; *line && (*line == ' ' || *line == '\t'); line++) ;
	s = line;
	tchFound = FALSE;
	sp[0] = EOS;
	while (*s) {
		if (*line == ',' || isSpace(*line) || *line == '\n' || *line == NL_C || *line == SNL_C || *line == EOS) {
			wc = ' ';
			e = line;
			for (; *line != EOS && (isSpace(*line) || *line == '\n' || *line == NL_C || *line == SNL_C); line++) ;
			if (*line != ',' && *line != EOS)
				line--;
			else
				wc = ',';
			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == ',') {
					rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL, NULL, NULL, NULL, NULL);
					if (rootIDs == NULL)
						return(NULL);
				} else if (cnt == 1) {
					rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, s, NULL, NULL, NULL);
					if (rootIDs == NULL)
						return(NULL);
				} else if (cnt == 0) {
					strcpy(sp, s);
					rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
					if (rootIDs == NULL)
						return(NULL);
				}
				*e = t;
				if (wc == ',') {
					cnt = -1;
					sp[0] = EOS;
				}
				for (line++; isSpace(*line) || *line=='\n' || *line == NL_C || *line == SNL_C || *line==','; line++) {
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
					rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, NULL, NULL, s, NULL, NULL, NULL, NULL, NULL, NULL);
					if (rootIDs == NULL)
						return(NULL);
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
	return(rootIDs);
}


static CMDIIDSTYPE *handleIDs(CMDIIDSTYPE *rootIDs, METADATA *cmdata, char *fname, long ln, char *line) {
	int t, s = 0, e = 0, cnt;
	char sp[SPEAKERLEN];
	char word[SPEAKERLEN], *st;
	
	word[0] = EOS;
	sp[0] = EOS;
	while (line[s] != EOS && line[s] != NL_C && line[s] != SNL_C) {
		if (!isSpace(line[s]))
			break;
		s++;
	}
	if (line[s] == EOS || line[s] == NL_C || line[s] == SNL_C)
		return(rootIDs);
	t = s;
	cnt = 0;
	while (1) {
		st = word;
		while ((*st=line[s]) == '|' || isSpace(line[s])) {
			if (line[s] == EOS || line[s] == NL_C || line[s] == SNL_C)
				break;
			if (line[s] == '|')
				cnt++;
			s++;
		}
		if (*st == EOS || *st == NL_C || *st == SNL_C)
			break;
		e = s + 1;
		while ((*++st=line[e]) != EOS && *st != NL_C && *st != SNL_C) {
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
	if (sp[0] == EOS || sp[0] == NL_C || sp[0] == SNL_C)
		return(rootIDs);
	word[0] = EOS;
	s = t;
	cnt = 0;
	while (1) {
		st = word;
		while ((*st=line[s]) == '|' || isSpace(line[s])) {
			if (line[s] == EOS || line[s] == NL_C || line[s] == SNL_C)
				break;
			if (line[s] == '|')
				cnt++;
			s++;
		}
		if (*st == EOS || *st == NL_C || *st == SNL_C)
			break;
		e = s + 1;
		while ((*++st=line[e]) != EOS && *st != NL_C && *st != SNL_C) {
			e++;
			if (line[e-1] == '|')
				break;
		}
		*st = EOS;
		if (cnt == 0) {			// language
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, word, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 1) {	// corpus
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, word, sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 2) {	// code
			
		} else if (cnt == 3) {	// age
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, word, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 4) {	// sex
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, word, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 5) {	// group
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, word, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 6) {	// SES
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, NULL, word, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 7) {	// role
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, NULL, NULL, word, NULL, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 8) {	// education
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, word, NULL, NULL, NULL, NULL, NULL);
		} else if (cnt == 9) {	// file unique ID
			rootIDs = add_to_IDs(rootIDs, cmdata, fname, ln, NULL, NULL, sp, NULL, NULL, NULL, NULL, NULL, NULL, word, NULL, NULL, NULL, NULL);
		}
		s = e;
		if (rootIDs == NULL)
			return(NULL);
		cnt++;
	}
	
	return(rootIDs);
}

static void addToUnknownLangs(char *langCode, char isMissing) {
	int i;

	if (showAll) {
		if (isMissing)
			sprintf(templineC2, "*** File \"%s\"\nMissing translation lang code: %s\n", FileName2, langCode);
		else
			sprintf(templineC2, "*** File \"%s\"\nlang code: %s\n", FileName2, langCode);
		writeError(templineC2);
	} else if (missingLangsIndex < TOTANUMLANGS) {
		for (i=0; i < missingLangsIndex; i++) {
			if (uS.mStricmp(langCode, missingLangs[i]) == 0) {
				if (isMissingLang[i] ==  FALSE && isMissing)
					isMissingLang[i] = TRUE;
				return;
			}
		}
		strcpy(missingLangs[i], langCode);
		isMissingLang[i] = isMissing;
		missingLangsIndex++;
	}
}

static void writeISO639codes(FILE *fp, METADATA *cmdata) {
	METADATA *m;
	char *comma, *tl;

	for (m=cmdata; m != NULL; m=m->next) {
		if (!strcmp(m->tag, "Subject.olac:language")) {
			tl = m->data;
			if (tl != NULL) {
				comma = tl;
				do {
					comma = strchr(tl, ',');
					if (comma != NULL) {
						*comma = EOS;
						fprintf(fp, "        <ISO639>\n");
						fprintf(fp, "            <iso-639-3-code>%s</iso-639-3-code>\n", tl);
						fprintf(fp, "        </ISO639>\n");
						tl = comma + 1;
						while (isSpace(*tl))
							tl++;
					} else {
						fprintf(fp, "        <ISO639>\n");
						fprintf(fp, "            <iso-639-3-code>%s</iso-639-3-code>\n", tl);
						fprintf(fp, "        </ISO639>\n");
						break;
					}
				} while (*tl != EOS) ;
			}
		}
	}
}

static void createCollectionFile(PATHTREE *p, char *wd_cur, METADATA *cmdata) {
	int  i, idNum;
	char *s, PID_Self[PID_SIZE];
	FILE *fp;
	PATHTREE *sib;
	METADATA *m, *mTitle, *mDescr, *mSubj, *mPubl, *mCreator, *iModalities;

//	if (cmdata == NULL)
//		return;
	getServerURL(0, currentURL);
	addFilename2Path(currentURL, "data-cmdi");
	addFilename2Path(currentURL, wd_cur+wdCurStartI);
	s = strrchr(currentURL, PATHDELIMCHR);
	if (s != NULL) {
		*s = EOS;
		if (*(s+1) == EOS) {
			s = strrchr(currentURL, PATHDELIMCHR);
			if (s != NULL)
				*s = EOS;
		}
	}
	addFilename2Path(currentURL, p->name);
	i = 8;
	while (currentURL[i] != EOS) {
		if (currentURL[i] == '\\')
			currentURL[i] = '/';
		if (currentURL[i] == '/' && (currentURL[i+1] == '/' || currentURL[i+1] == '\\'))
			strcpy(currentURL+i, currentURL+i+1);
		else
			i++;
	}
	for (i=strlen(currentURL); i > 0 && currentURL[i] != '/'; i--) {
		if (currentURL[i] == '.') {
			currentURL[i] = EOS;
			break;
		}
	}
	strcpy(templineC, currentURL);
	strcat(templineC, ".cmdi");
	filterTextForXML(templineC2, templineC);
//	if (isJustTest)
//		return;
	strcpy(FileName1, wd_cur);
	strcat(FileName1, ".cmdi");
	fp = fopen(FileName1, "w");
	if (fp == NULL) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open file \"%s\".\n", FileName1);
		cutt_exit(0);
	}
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fp, "<CMD CMDVersion=\"1.1\" xmlns=\"http://www.clarin.eu/cmd/\"\n");
	fprintf(fp, "                xmlns:MPI=\"http://www.mpi.nl/\"\n");
	fprintf(fp, "                xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"\n");
	fprintf(fp, "                xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
//	fprintf(fp, "                xsi:schemaLocation=\"http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles/clarin.eu:cr1:p_1393514855467/xsd\">\n");
//	fprintf(fp, "                xsi:schemaLocation=\"http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles/clarin.eu:cr1:p_1284723009187/xsd\">\n");
	fprintf(fp, "                xsi:schemaLocation=\"http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/1.1/profiles/clarin.eu:cr1:p_1345561703620/xsd\">\n");
	fprintf(fp, "  <Header>\n");
	fprintf(fp, "    <MdCreator>CLAN cmdi</MdCreator>\n");
	fprintf(fp, "    <MdCreationDate>%s</MdCreationDate>\n", cur_date);
	fprintf(fp, "    <MdSelfLink>%s</MdSelfLink>\n", templineC2);
	fprintf(fp, "    <MdProfile>clarin.eu:cr1:p_1345561703620</MdProfile>\n");
	fprintf(fp, "    <MdCollectionDisplayName>TalkBank</MdCollectionDisplayName>\n");
	fprintf(fp, "  </Header>\n");

	fprintf(fp, "  <Resources>\n");
	fprintf(fp, "    <ResourceProxyList>\n");
	idNum = 0;
	for (sib=p->child; sib != NULL; sib=sib->sibling) {
//		strcpy(FileName1, p->name);
//		i = strlen(sib->name) - 4;
//		if (uS.mStricmp(sib->name+i, ".cha") == 0 || uS.mStricmp(sib->name+i, ".cdc") == 0 || uS.mStricmp(sib->name+i, ".zip") == 0) {
			if (sib->PID != -1 && sib->PID != 0) {
				generateHDL(templineC3, sib->PID);
				sprintf(PID_Self, "11312/%c-%s-%d", sib->PIDp, templineC3, 1);
				strcpy(templineC, "hdl:");
				strcat(templineC, PID_Self);
			} else {
				strcpy(templineC, currentURL);
				addFilename2Path(templineC, sib->name);
				for (i=strlen(templineC); i > 0 && templineC[i] != '/'; i--) {
					if (templineC[i] == '.') {
						templineC[i] = EOS;
						break;
					}
				}
				strcat(templineC, ".cmdi");
			}
			filterTextForXML(templineC2, templineC);
			idNum++;
			fprintf(fp, "      <ResourceProxy id=\"p%d\">\n", idNum);
			fprintf(fp, "        <ResourceType>Metadata</ResourceType>\n");
			fprintf(fp, "        <ResourceRef>%s</ResourceRef>\n", templineC2);
			fprintf(fp, "      </ResourceProxy>\n");
//		}
	}
	if (LandingPage[0] != EOS && cmdata != NULL) {
		fprintf(fp, "      <ResourceProxy id=\"landingPage\">\n", idNum);
		fprintf(fp, "        <ResourceType>LandingPage</ResourceType>\n");
		fprintf(fp, "        <ResourceRef>%s</ResourceRef>\n", LandingPage);
		fprintf(fp, "      </ResourceProxy>\n");
	}
	fprintf(fp, "    </ResourceProxyList>\n");
	fprintf(fp, "    <JournalFileProxyList/>\n");
	fprintf(fp, "    <ResourceRelationList/>\n");
	fprintf(fp, "  </Resources>\n");

	fprintf(fp, "  <Components>\n");
	mTitle = NULL;
	mDescr = NULL;
	mSubj = NULL;
	mPubl = NULL;
	mCreator = NULL;
	iModalities = NULL;
	for (m=cmdata; m != NULL; m=m->next) {
		if (!strcmp(m->tag, "Title"))
			mTitle = m;
		else if (!strcmp(m->tag, "Description"))
			mDescr = m;
		else if (!strcmp(m->tag, "Subject"))
			mSubj = m;
		else if (!strcmp(m->tag, "Publisher"))
			mPubl = m;
		else if (!strcmp(m->tag, "Creator"))
			mCreator = m;
		else if (!strcmp(m->tag, "IMDI_Modalities"))
			iModalities = m;
	}
/*
	fprintf(fp, "    <talkbank-corpus>\n");
	fprintf(fp, "      <Corpus>\n");
	filterTextForXML(templineC2, p->name);
	fprintf(fp, "        <Name>%s</Name>\n", templineC2);
	if (mTitle != NULL) {
		fprintf(fp, "        <Title>\"%s - %s\"</Title>\n", mTitle->data, templineC2);
	} else {
		filterTextForXML(templineC2, p->name);
		fprintf(fp, "        <Title>Corpus \"%s\"</Title>\n", templineC2);
	}
	fprintf(fp, "        <descriptions>\n");
	if (mDescr != NULL) {
		fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", mDescr->data);
	} else if (mSubj != NULL) {
		fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", mSubj->data);
	} else {
		fprintf(fp, "            <Description LanguageId=\"\">Participants' Annotations</Description>\n");
	}
	fprintf(fp, "        </descriptions>\n");
	idNum = 0;
	for (sib=p->child; sib != NULL; sib=sib->sibling) {
//		if (uS.mStricmp(sib->name, "0metadata.cdc")) {
			strcpy(FileName1, p->name);
			addFilename2Path(FileName1, sib->name);
			s = strrchr(FileName1, '.');
			if (s != NULL)
				*s = EOS;
			strcat(FileName1, ".cmdi");
			filterTextForXML(templineC2, FileName1);
			strcpy(FileName1, sib->name);
			s = strrchr(FileName1, '.');
			if (s != NULL)
				*s = EOS;
			filterTextForXML(templineC3, FileName1);
			idNum++;
			fprintf(fp, "        <CorpusLink ref=\"p%d\">\n", idNum);
			fprintf(fp, "          <CorpusLinkContent Name=\"%s\">%s</CorpusLinkContent>\n", templineC3, templineC2);
			fprintf(fp, "        </CorpusLink>\n");
//		}
	}
	fprintf(fp, "      </Corpus>\n");
	fprintf(fp, "    </talkbank-corpus>\n");
*/
	fprintf(fp, "    <collection>\n");
	fprintf(fp, "      <CollectionInfo>\n");

	filterTextForXML(templineC2, p->name);
	fprintf(fp, "        <Name>%s</Name>\n", templineC2);
	if (mTitle != NULL) {
		fprintf(fp, "        <Title>\"%s - %s\"</Title>\n", mTitle->data, templineC2);
	} else {
		filterTextForXML(templineC2, p->name);
		fprintf(fp, "        <Title>Corpus \"%s\"</Title>\n", templineC2);
	}
	if (mCreator != NULL)
		fprintf(fp, "        <Owner>%s</Owner>\n", mCreator->data);
	writeISO639codes(fp, cmdata); // <ISO639>
	if (iModalities != NULL) {
		fprintf(fp, "        <Modality>\n");
		fprintf(fp, "            <Modality>%s</Modality>\n", iModalities->data);
		fprintf(fp, "        </Modality>\n");
	}

	fprintf(fp, "        <Description>\n");
	if (mDescr != NULL) {
		fprintf(fp, "            <Description>%s - Corpus: %s</Description>\n", mDescr->data, templineC2);
	} else if (mSubj != NULL) {
		fprintf(fp, "            <Description>%s - Corpus: %s</Description>\n", mSubj->data, templineC2);
	} else {
		fprintf(fp, "            <Description>Participants' Annotations - Corpus: %s</Description>\n", templineC2);
	}
	fprintf(fp, "        </Description>\n");
	fprintf(fp, "      </CollectionInfo>\n");
	fprintf(fp, "      <License>\n");
	fprintf(fp, "          <DistributionType>public</DistributionType>\n");
	fprintf(fp, "          <LicenseName>CC BY-NC-SA 3</LicenseName>\n");
	fprintf(fp, "          <LicenseURL>%s</LicenseURL>\n", "https://creativecommons.org/licenses/by-nc-sa/3.0/");
	fprintf(fp, "      </License>\n");
	fprintf(fp, "      <Contact>\n");
	fprintf(fp, "          <Person>%s</Person>\n", "Prof. Brian MacWhinney");
	fprintf(fp, "          <Email>%s</Email>\n", "macw@cmu.edu");
	fprintf(fp, "          <Organisation>%s</Organisation>\n", "Carnegie Mellon University");
	fprintf(fp, "          <Website>%s</Website>\n", "https://psyling.talkbank.org/");
	fprintf(fp, "      </Contact>\n");
	fprintf(fp, "    </collection>\n");
	fprintf(fp, "  </Components>\n");
	fprintf(fp, "</CMD>\n");
	fclose(fp);
}

static char invalidPID(char *PID) {
	int i;

	if (uS.mStrnicmp(PID, "11312/", 6) != 0)
		return(TRUE);
	PID += 6;
	if (*PID != 'c' && *PID != 't' && *PID != 'a')
		return(TRUE);
	PID++;
	if (*PID != '-')
		return(TRUE);
	for (i=0, PID++; i < 8; i++, PID++) {
		if (!isdigit(*PID))
			return(TRUE);
	}
	if (*PID != '-')
		return(TRUE);
	PID++;
	if (!isdigit(*PID))
		return(TRUE);
	return(FALSE);
}

static void createMetadataCDCFile(PATHTREE *p, char *wd_cur, METADATA *cmdata) {
	int  i, j;
	long ln;
	unsigned char PIDc;
	char isUTFound, isPIDFound;
	char *line, *e, tlang[LANGLEN+1];
	char *s, fsize[50], fdate[50], fquality[5], ftimepos[50], tdate[50], flang[LANGNUM][LANGLEN+1],
		ttranscriber[256], ttranscription[256], tnumber[50], tinteraction[50],
		PID_Self[PID_SIZE], PID_ZipArchive[PID_SIZE];
	METADATA *m, *mTitle, *mCreator, *mDescr, *mSubj, *mPubl, *mContributor, *mDate, *mDiscourse, *mId,
		*iGenre, *iInteractivity, *iPlanningType, *iInvolvement, *iSocialContext, *iEventStructure,
		*iChannel, *iTask, *iModalities, *iSubject, *iEthnicGroup, *iRecordingConditions, *iAccess,
		*iContinent, *iCountry, *iWrittenSubtype, *iProjectDescr, *iMediaFileDescr, *DOI, *iCorpusPID,
		*iLanguage;
	CMDIIDSTYPE *rootIDs, *tID;
	FILE *fp, *fpn;

	cln++;
	if (cln > tln) {
		tln = cln + 200;
#if !defined(CLAN_SRV)
		fprintf(stderr,"\r%ld ", cln);
#endif
#if !defined(UNX)
		my_flush_chr();
#endif
	}
#if defined(_MAC_CODE) || defined(_WIN32)
#ifndef _COCOA_APP
	SysEventCheck(100L);
#endif
#endif
	s = strrchr(p->name, '.');
	if (s == NULL || uS.mStricmp(s, ".cdc"))
		return;
	isPIDFound = FALSE;
	mTitle = NULL;
	mDescr = NULL;
	mSubj = NULL;
	mPubl = NULL;
	mCreator = NULL;
	mContributor = NULL;
	mDate = NULL;
	mDiscourse = NULL;
	mId = NULL;
	iLanguage = NULL;
	iGenre = NULL;
	iInteractivity = NULL;
	iPlanningType = NULL;
	iInvolvement = NULL;
	iSocialContext = NULL;
	iEventStructure = NULL;
	iChannel = NULL;
	iTask = NULL;
	iModalities = NULL;
	iSubject = NULL;
	iEthnicGroup = NULL;
	iRecordingConditions = NULL;
	iAccess = NULL;
	iContinent = NULL;
	iCountry = NULL;
	iWrittenSubtype = NULL;
	iProjectDescr = NULL;
	iMediaFileDescr = NULL;
	DOI = NULL;
	iCorpusPID = NULL;
	isUTFound = 0;
	strcpy(fquality, "5");
	strcpy(ftimepos, "Unspecified");
	tdate[0] = EOS;
	j = 0;
	for (i=0; i < LANGNUM; i++)
		flang[i][0] = EOS;
	ttranscriber[0] = EOS;
	ttranscription[0] = EOS;
	tnumber[0] = EOS;
	tinteraction[0] = EOS;
	rootIDs = NULL;
	cMediaFileName[0] = EOS;
	strcpy(FileName2, wd_dir);
	addFilename2Path(FileName2, wd_cur+wdCurStartI);
	addFilename2Path(FileName2, p->name);
	fp = fopen(FileName2, "r");
	strcpy(fdate, cur_date);
	if (fp == NULL) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open 0Metadata file \"%s\".\n", FileName2);
		cutt_exit(0);
	} else {
		getFileDateSize(FileName2, fdate, fsize);
		ln = 0L;
		templineC[0] = EOS;
		while (fgets_cr(templineC, UTTLINELEN, fp)) {
			if (uS.partcmp(templineC, CMDIPIDHEADER, FALSE, FALSE)) {
				for (i=strlen(CMDIPIDHEADER)+1; isSpace(templineC[i]); i++) ;
				uS.remblanks(templineC+i);
				if (isPIDFound) {
					sprintf(templineC2, "In File \"%s:\"\n    More than one %s header found\n", FileName2, CMDIPIDHEADER);
					writeError(templineC2);
				}
				if (invalidPID(templineC+i)) {
					fprintf(stderr, "\nIn File \"%s:\"\n    %s header is corrupt or has bad prefix\n", FileName2, CMDIPIDHEADER);
					tree_root = free_tree(tree_root);
					free_metadata(cmdata);
					cleanupLanguages();
					cutt_exit(0);
				}
				isPIDFound = TRUE;
			}
		}
		fclose(fp);
	}
	if (isKillProgram) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		cutt_exit(0);
	}
	PIDc = 1;
	getServerURL(0, currentURL);
	addFilename2Path(currentURL, "data-cmdi");
	addFilename2Path(currentURL, wd_cur+wdCurStartI);
	addFilename2Path(currentURL, p->name);
	i = 8;
	while (currentURL[i] != EOS) {
		if (currentURL[i] == '\\')
			currentURL[i] = '/';
		if (currentURL[i] == '/' && (currentURL[i+1] == '/' || currentURL[i+1] == '\\'))
			strcpy(currentURL+i, currentURL+i+1);
		else
			i++;
	}
	for (i=strlen(currentURL); i > 0 && currentURL[i] != '/'; i--) {
		if (currentURL[i] == '.') {
			currentURL[i] = EOS;
			break;
		}
	}
	strcat(currentURL, ".cmdi");
	filterTextForXML(templineC2, currentURL);
	PIDc = writePIDbatch(templineC2, PID_Self, p->PIDp, p->PID, PIDc);
	if (PIDc == 0) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		cutt_exit(0);
	}
	if (!isPIDFound) {
		if (isJustTest) {
			filesChanged++;
		} else {
			fp = fopen(FileName2, "r");
			if (fp != NULL) {
				strcpy(FileName1, FileName2);
				strcat(FileName1, ".del");
				fpn = fopen(FileName1,"w");
				if (fpn != NULL) {
					filesChanged++;
// fprintf(stderr, "File Changed: %s;\n", FileName2);
					templineC[0] = EOS;
					fprintf(fpn, "%s\n", UTF8HEADER);
					fprintf(fpn, "%s:\t%s\n", CMDIPIDHEADER, PID_Self);
					while (fgets_cr(templineC, UTTLINELEN, fp)) {
						if (uS.partcmp(templineC, UTF8HEADER, FALSE, FALSE) == FALSE)
							fprintf(fpn, "%s", templineC);
					}
					fclose(fp);
					fclose(fpn);
					if (isKillProgram) {
						unlink(FileName1);
						tree_root = free_tree(tree_root);
						free_metadata(cmdata);
						cleanupLanguages();
						cutt_exit(0);
					}
					unlink(FileName2);
#ifndef UNX
					if (rename_each_file(FileName1, FileName2, FALSE) == -1) {
						tree_root = free_tree(tree_root);
						free_metadata(cmdata);
						cleanupLanguages();
						fprintf(stderr, "\n   Can't rename original file \"%s\". Perhaps it is opened by some application.\n", FileName2);
						cutt_exit(0);
					}
#else
					rename(FileName1, FileName2);
#endif
				} else {
					tree_root = free_tree(tree_root);
					free_metadata(cmdata);
					cleanupLanguages();
					fprintf(stderr, "\n   Can't create temp file \"%s\".\n", FileName1);
					fclose(fp);
					cutt_exit(0);
				}
			}
		}
	}
	getServerURL(p->sl, templineC);
	addFilename2Path(templineC, "data/");
	removeServernameAddFilename2Path(templineC, wd_cur+wdCurStartI);
	strcat(templineC, ".zip");
	i = 8;
	while (templineC[i] != EOS) {
		if (templineC[i] == '\\')
			templineC[i] = '/';
		if (templineC[i] == '/' && (templineC[i+1] == '/' || templineC[i+1] == '\\'))
			strcpy(templineC+i, templineC+i+1);
		else
			i++;
	}
	filterTextForXML(templineC2, templineC);
	PIDc = writePIDbatch(templineC2, PID_ZipArchive, p->PIDp, p->PID, PIDc);
	if (PIDc == 0) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		cutt_exit(0);
	}
//	if (!isJustTest) {
		for (m=cmdata; m != NULL; m=m->next) {
			if (!strcmp(m->tag, "Title")) {
				mTitle = m;
			} else if (!strcmp(m->tag, "Description")) {
				mDescr = m;
			} else if (!strcmp(m->tag, "Subject")) {
				mSubj = m;
			} else if (!strcmp(m->tag, "Publisher")) {
				mPubl = m;
			} else if (!strcmp(m->tag, "Creator")) {
				mCreator = m;
			} else if (!strcmp(m->tag, "Contributor")) {
				mContributor = m;
			} else if (!strcmp(m->tag, "Date")) {
				mDate = m;
			} else if (!strcmp(m->tag, "Type.olac:discourse-type")) {
				mDiscourse = m;
			} else if (!strcmp(m->tag, "Identifier")) {
				mId = m;
			} else if (!strcmp(m->tag, "Subject.olac:language")) {
				iLanguage = m;
			} else if (!strcmp(m->tag, "IMDI_Genre")) {
				iGenre = m;
			} else if (!strcmp(m->tag, "IMDI_Interactivity")) {
				iInteractivity = m;
			} else if (!strcmp(m->tag, "IMDI_PlanningType")) {
				iPlanningType = m;
			} else if (!strcmp(m->tag, "IMDI_Involvement")) {
				iInvolvement = m;
			} else if (!strcmp(m->tag, "IMDI_SocialContext")) {
				iSocialContext = m;
			} else if (!strcmp(m->tag, "IMDI_EventStructure")) {
				iEventStructure = m;
			} else if (!strcmp(m->tag, "IMDI_Channel")) {
				iChannel = m;
			} else if (!strcmp(m->tag, "IMDI_Task")) {
				iTask = m;
			} else if (!strcmp(m->tag, "IMDI_Modalities")) {
				iModalities = m;
			} else if (!strcmp(m->tag, "IMDI_Subject")) {
				iSubject = m;
			} else if (!strcmp(m->tag, "IMDI_EthnicGroup")) {
				iEthnicGroup = m;
			} else if (!strcmp(m->tag, "IMDI_RecordingConditions")) {
				iRecordingConditions = m;
			} else if (!strcmp(m->tag, "IMDI_AccessAvailability")) {
				iAccess = m;
			} else if (!strcmp(m->tag, "IMDI_Continent")) {
				iContinent = m;
			} else if (!strcmp(m->tag, "IMDI_Country")) {
				iCountry = m;
			} else if (!strcmp(m->tag, "IMDI_WrittenResourceSubType")) {
				iWrittenSubtype = m;
			} else if (!strcmp(m->tag, "IMDI_ProjectDescription")) {
				iProjectDescr = m;
			} else if (!strcmp(m->tag, "IMDI_MediaFileDescription")) {
				iMediaFileDescr = m;
			} else if (!strcmp(m->tag, "DOI")) {
				DOI = m;
			} else if (!strcmp(m->tag, CMDIPIDHEADER)) {
				iCorpusPID = m;
			}
		}
		strcpy(FileName1, wd_cur);
		addFilename2Path(FileName1, p->name);
		s = strrchr(FileName1, '.');
		if (s != NULL)
			*s = EOS;
		strcat(FileName1, ".cmdi");
		fp = fopen(FileName1, "w");
		if (fp == NULL) {
			tree_root = free_tree(tree_root);
			free_metadata(cmdata);
			cleanupLanguages();
			fprintf(stderr, "\n   Can't open file \"%s\".\n", FileName1);
			cutt_exit(0);
		}
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(fp, "<CMD CMDVersion=\"1.1\" xmlns=\"http://www.clarin.eu/cmd/\"\n");
		fprintf(fp, "                xmlns:MPI=\"http://www.mpi.nl/\"\n");
		fprintf(fp, "                xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"\n");
		fprintf(fp, "                xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
		fprintf(fp, "                xsi:schemaLocation=\"http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/1.1/profiles/clarin.eu:cr1:p_1475136016232/xsd\">\n");
		fprintf(fp, "  <Header>\n");
		fprintf(fp, "    <MdCreator>CLAN cmdi</MdCreator>\n");
		fprintf(fp, "    <MdCreationDate>%s</MdCreationDate>\n", cur_date);
//2017-05-02		fprintf(fp, "    <MdSelfLink>%s</MdSelfLink>\n", currentURL); // 2016-12-08
		fprintf(fp, "    <MdSelfLink>hdl:%s</MdSelfLink>\n", PID_Self);
		fprintf(fp, "    <MdProfile>clarin.eu:cr1:p_1475136016232</MdProfile>\n");
		fprintf(fp, "    <MdCollectionDisplayName>TalkBank</MdCollectionDisplayName>\n");
		fprintf(fp, "  </Header>\n");
		fprintf(fp, "  <Resources>\n");
		fprintf(fp, "    <ResourceProxyList>\n");
		fprintf(fp, "      <ResourceProxy id=\"%s\">\n", "Corpus_Archive");
		fprintf(fp, "        <ResourceType mimetype=\"text/x-chat\">Resource</ResourceType>\n");
		fprintf(fp, "        <ResourceRef>hdl:%s</ResourceRef>\n", PID_ZipArchive);
		fprintf(fp, "      </ResourceProxy>\n");
		fprintf(fp, "    </ResourceProxyList>\n");
		fprintf(fp, "    <JournalFileProxyList/>\n");
		fprintf(fp, "    <ResourceRelationList/>\n");
		fprintf(fp, "  </Resources>\n");
		fprintf(fp, "  <Components>\n");
		fprintf(fp, "    <talkbank-license-session>\n");
		fprintf(fp, "    <License>\n");
		fprintf(fp, "      <DistributionType>public</DistributionType>\n");
		fprintf(fp, "      <LicenseName>CC BY-NC-SA 3</LicenseName>\n");
		fprintf(fp, "      <LicenseURL>%s</LicenseURL>\n", "https://creativecommons.org/licenses/by-nc-sa/3.0/");
		fprintf(fp, "    </License>\n");
		fprintf(fp, "    <Session>\n");
		filterTextForXML(templineC2, p->name);
		s = strrchr(templineC2, '.');
		if (s != NULL)
			*s = EOS;
		fprintf(fp, "      <Name>%s</Name>\n", templineC2);
		if (mTitle != NULL) {
			fprintf(fp, "      <Title>\"%s - %s\"</Title>\n", mTitle->data, templineC2);
		} else {
			fprintf(fp, "      <Title>Child \"%s\"</Title>\n", templineC2);
		}
		if (tdate[0] != EOS)
			fprintf(fp, "      <Date>%s</Date>\n", tdate);
		else
			fprintf(fp, "      <Date>%s</Date>\n", "Unspecified");
		fprintf(fp, "      <MDGroup>\n");
		fprintf(fp, "        <Location>\n");
		if (iContinent != NULL && iCountry != NULL) {
			fprintf(fp, "          <Continent>%s</Continent>\n", iContinent->data);
			fprintf(fp, "          <Country>%s</Country>\n", iCountry->data);
		} else if (iContinent == NULL && iCountry == NULL) {
			fprintf(fp, "          <Continent>%s</Continent>\n", "North-America");
			fprintf(fp, "          <Country>%s</Country>\n", "United States");
		} else {
			sprintf(templineC2, "*** File \"%s\"\nNo \"IMDI_Continent\" or \"IMDI_Country\" field found in 0metadata.cdc for 0Metadata file\n", FileName2);
			writeError(templineC2);
		}
		fprintf(fp, "          <Region/>\n");
		fprintf(fp, "          <Address/>\n");
		fprintf(fp, "        </Location>\n");
		fprintf(fp, "        <Project>\n");
		outputServerTitle(fp, p->sl, mContributor);
		if (DOI != NULL)
			fprintf(fp, "          <Id>DOI %s</Id>\n", DOI->data);
		else
			fprintf(fp, "          <Id/>\n");
		fprintf(fp, "          <Contact>\n");
		fprintf(fp, "            <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "            <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "            <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "            <Organisation>%s</Organisation>\n", "Carnegie Mellon University");
		fprintf(fp, "          </Contact>\n");
		fprintf(fp, "          <descriptions>\n");
		if (iProjectDescr != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", iProjectDescr->data);
		} else
			outputServerDescription(fp, p->sl);
		fprintf(fp, "          </descriptions>\n");
		fprintf(fp, "        </Project>\n");
		fprintf(fp, "        <Keys/>\n");
		fprintf(fp, "        <Content>\n");
		if (iGenre != NULL) {
			strcpy(templineC2, iGenre->data);
		} else if (mDiscourse != NULL) {
			if (uS.mStricmp(mDiscourse->data, "narrative") == 0)
				strcpy(templineC2, "Narrative");
			else
				strcpy(templineC2, "Discourse");
		} else
			strcpy(templineC2, "Discourse");
		fprintf(fp, "          <Genre>%s</Genre>\n", templineC2);
		fprintf(fp, "          <SubGenre/>\n");
		if (iTask != NULL)
			fprintf(fp, "          <Task>%s</Task>\n", iTask->data);
		else
			fprintf(fp, "          <Task>%s</Task>\n", "Unspecified");
		if (iModalities != NULL)
			fprintf(fp, "          <Modalities>%s</Modalities>\n", iModalities->data);
		else
			fprintf(fp, "          <Modalities>%s</Modalities>\n", "Spoken");
		if (iSubject != NULL)
			fprintf(fp, "          <Subject>%s</Subject>\n", iSubject->data);
		else
			fprintf(fp, "          <Subject>%s</Subject>\n", "Unspecified");
		fprintf(fp, "          <CommunicationContext>\n");
		if (iInteractivity != NULL)
			fprintf(fp, "            <Interactivity>%s</Interactivity>\n", iInteractivity->data);
		else
			fprintf(fp, "            <Interactivity>%s</Interactivity>\n", "interactive");
		if (iPlanningType != NULL)
			fprintf(fp, "            <PlanningType>%s</PlanningType>\n", iPlanningType->data);
		else
			fprintf(fp, "            <PlanningType>%s</PlanningType>\n", "spontaneous");
		if (iInvolvement != NULL)
			fprintf(fp, "            <Involvement>%s</Involvement>\n", iInvolvement->data);
		else
			fprintf(fp, "            <Involvement>%s</Involvement>\n", "non-elicited");
		if (iSocialContext != NULL)
			fprintf(fp, "            <SocialContext>%s</SocialContext>\n", iSocialContext->data);
		else
			fprintf(fp, "            <SocialContext>%s</SocialContext>\n", "Family");
		if (iEventStructure != NULL)
			fprintf(fp, "            <EventStructure>%s</EventStructure>\n", iEventStructure->data);
		else
			fprintf(fp, "            <EventStructure>%s</EventStructure>\n", "Conversation");
		fprintf(fp, "          </CommunicationContext>\n");
		fprintf(fp, "          <Content_Languages>\n");
		if (iLanguage != NULL) {
			line = iLanguage->data;
			while (*line != EOS) {
				e = strchr(line, ',');
				if (e != NULL) {
					*e = EOS;
					for (; isSpace(*line); line++) ;
					strncpy(tlang, line, LANGLEN);
					tlang[LANGLEN] = EOS;
					*e = ',';
					line = e + 1;
				} else {
					for (; isSpace(*line); line++) ;
					strncpy(tlang, line, LANGLEN);
					tlang[LANGLEN] = EOS;
					line = line + strlen(line);
				}
				uS.remblanks(tlang);
				for (i=0; i < LANGNUM; i++) {
					if (uS.mStricmp(tlang, flang[i]) == 0)
						break;
				}
				if (i >= LANGNUM) {
					for (i=0; i < LANGNUM && flang[i][0] != EOS; i++) ;
					if (i < LANGNUM) {
						strcpy(flang[i], tlang);
					}
				}
			}
		}
		templineC2[0] = EOS;
		for (i=0; i < LANGNUM; i++) {
			if (flang[i][0] != EOS) {
				fprintf(fp, "            <Content_Language>\n");
				if (strlen(flang[i]) < 3)
					addToUnknownLangs(flang[i], FALSE);
				getLanguageCodeAndName(flang[i], TRUE, NULL);
				if (strlen(flang[i]) == 2)
					strcpy(templineC2, "ISO639-1:");
				else
					strcpy(templineC2, "ISO639-3:");
				strcat(templineC2, flang[i]);
				fprintf(fp, "              <Id>%s</Id>\n", templineC2);
				if (!getLanguageCodeAndName(flang[i], FALSE, templineC2)) {
					addToUnknownLangs(flang[i], TRUE);
					strcpy(templineC2, "Unspecified");
				}
				fprintf(fp, "              <Name>%s</Name>\n", templineC2);
				fprintf(fp, "              <Dominant>%s</Dominant>\n", "Unspecified");
				fprintf(fp, "              <SourceLanguage>%s</SourceLanguage>\n", "Unspecified");
				fprintf(fp, "              <TargetLanguage>%s</TargetLanguage>\n", "Unspecified");
				fprintf(fp, "            </Content_Language>\n");
			}
		}
		uS.remblanks(templineC2);
		fprintf(fp, "            </Content_Languages>\n");
		fprintf(fp, "            <Keys>\n");
		if (ttranscriber[0] != EOS)
			fprintf(fp, "              <Key Name=\"Transcriber\">%s</Key>\n", ttranscriber);
		if (ttranscription[0] != EOS)
			fprintf(fp, "              <Key Name=\"Transcription\">%s</Key>\n", ttranscription);
		if (tnumber[0] != EOS)
			fprintf(fp, "              <Key Name=\"NumberOfParticipants\">%s</Key>\n", tnumber);
		if (tinteraction[0] != EOS)
			fprintf(fp, "              <Key Name=\"InteractionType\">%s</Key>\n", tinteraction);
		fprintf(fp, "            </Keys>\n");
		fprintf(fp, "          </Content>\n");
		fprintf(fp, "          <Actors>\n");
		for (tID=rootIDs; tID != NULL; tID=tID->next_id) {
			fprintf(fp, "            <Actor>\n");
			fprintf(fp, "              <Role/>\n");
			if (tID->spname[0] == EOS)
				fprintf(fp, "              <Name>Standard Actor</Name>\n");
			else
				fprintf(fp, "              <Name>%s</Name>\n", tID->spname);
			fprintf(fp, "              <FullName/>\n");
			if (tID->code[0] == EOS)
				fprintf(fp, "              <Code>%s</Code>\n", "Unspecified");
			else
				fprintf(fp, "              <Code>%s</Code>\n", tID->code);
			if (tID->role[0] == EOS)
				fprintf(fp, "              <FamilySocialRole>%s</FamilySocialRole>\n", "Unspecified");
			else
				fprintf(fp, "              <FamilySocialRole>%s</FamilySocialRole>\n", tID->role);
			if (iEthnicGroup == NULL)
				fprintf(fp, "              <EthnicGroup>%s</EthnicGroup>\n", "Unspecified");
			else
				fprintf(fp, "              <EthnicGroup>%s</EthnicGroup>\n", iEthnicGroup->data);
			if (tID->age[0] == EOS)
				fprintf(fp, "              <Age>%s</Age>\n", "Unspecified");
			else
				fprintf(fp, "              <Age>%s</Age>\n", tID->age);
			if (tID->birth[0] == EOS)
				fprintf(fp, "              <BirthDate>%s</BirthDate>\n", "Unspecified");
			else
				fprintf(fp, "              <BirthDate>%s</BirthDate>\n", tID->birth);
			if (tID->sex == 0)
				fprintf(fp, "              <Sex>Unknown</Sex>\n");
			else if (tID->sex == 'm')
				fprintf(fp, "              <Sex>Male</Sex>\n");
			else
				fprintf(fp, "              <Sex>Female</Sex>\n");
			if (tID->education[0] == EOS)
				fprintf(fp, "              <Education>%s</Education>\n", "Unspecified");
			else
				fprintf(fp, "              <Education>%s</Education>\n", tID->education);
			fprintf(fp, "              <Anonymized>true</Anonymized>\n");
			fprintf(fp, "              <Contact>\n");
			fprintf(fp, "                <Name/>\n");
			fprintf(fp, "                <Address/>\n");
			fprintf(fp, "                <Email/>\n");
			fprintf(fp, "                <Organisation/>\n");
			fprintf(fp, "              </Contact>\n");
			fprintf(fp, "              <Keys/>\n");
			fprintf(fp, "              <Actor_Languages>\n");
			for (i=0; i < LANGNUM; i++) {
				if (tID->lang[i][0] != EOS) {
					fprintf(fp, "                <Actor_Language>\n");
					if (strlen(tID->lang[i]) < 3)
						addToUnknownLangs(tID->lang[i], FALSE);
					getLanguageCodeAndName(tID->lang[i], TRUE, NULL);
					if (strlen(tID->lang[i]) == 2)
						strcpy(templineC2, "ISO639-1:");
					else
						strcpy(templineC2, "ISO639-3:");
					strcat(templineC2, tID->lang[i]);
					fprintf(fp, "                  <Id>%s</Id>\n", templineC2);
					if (!getLanguageCodeAndName(tID->lang[i], FALSE, templineC2)) {
						addToUnknownLangs(tID->lang[i], TRUE);
						strcpy(templineC2, "Unspecified");
					}
					fprintf(fp, "                  <Name>%s</Name>\n", templineC2);
					if (tID->Llang[i])
						fprintf(fp, "                  <MotherTongue>%s</MotherTongue>\n", "true");
					else
						fprintf(fp, "                  <MotherTongue>%s</MotherTongue>\n", "Unspecified");
					fprintf(fp, "                  <PrimaryLanguage>%s</PrimaryLanguage>\n", "Unspecified");
					fprintf(fp, "                </Actor_Language>\n");
				}
			}
			fprintf(fp, "              </Actor_Languages>\n");
			fprintf(fp, "            </Actor>\n");
		}
		fprintf(fp, "          </Actors>\n");
		fprintf(fp, "      </MDGroup>\n");
		fprintf(fp, "      <Resources>\n");
//	} // !isJustTest
	clean_ids(rootIDs);
	fsize[0] = EOS;
	strcpy(fdate, cur_date);
	if (!access(FileName2, 0)) {
		getFileDateSize(FileName2, fdate, fsize);
	} else {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open 0Metadata file \"%s\".\n", FileName2);
		cutt_exit(0);
	}
//	if (!isJustTest) {
		fprintf(fp, "        <WrittenResource ref=\"Corpus_Archive\">\n");
		getServerURL(p->sl, templineC);
		addFilename2Path(templineC, "data/");
		removeServernameAddFilename2Path(templineC, wd_cur+wdCurStartI);
		strcat(templineC, ".zip");
		i = 8;
		while (templineC[i] != EOS) {
			if (templineC[i] == '\\')
				templineC[i] = '/';
			if (templineC[i] == '/' && (templineC[i+1] == '/' || templineC[i+1] == '\\'))
				strcpy(templineC+i, templineC+i+1);
			else
				i++;
		}
		filterTextForXML(templineC2, templineC);
		fprintf(fp, "          <ResourceLink>%s</ResourceLink>\n", templineC2);
		fprintf(fp, "          <MediaResourceLink/>\n");
		if (tdate[0] != EOS)
			fprintf(fp, "          <Date>%s</Date>\n", tdate);
		else
			fprintf(fp, "          <Date>%s</Date>\n", "Unspecified");
		fprintf(fp, "          <Type>Annotation</Type>\n");
		if (iWrittenSubtype != NULL)
			fprintf(fp, "          <SubType>%s</SubType>\n", iWrittenSubtype->data);
		else
			fprintf(fp, "          <SubType/>\n");
		fprintf(fp, "          <Format>%s</Format>\n", "text/x-chat");
		if (fsize[0] == EOS)
			fprintf(fp, "          <Size/>\n");
		else
			fprintf(fp, "          <Size>%s</Size>\n", fsize);
		fprintf(fp, "          <Derivation>%s</Derivation>\n", "Annotation");
		fprintf(fp, "          <CharacterEncoding>%s</CharacterEncoding>\n", "UTF-8");
		fprintf(fp, "          <ContentEncoding/>\n");
		fprintf(fp, "          <LanguageId/>\n");
		fprintf(fp, "          <Anonymized>true</Anonymized>\n");
		fprintf(fp, "          <Validation>\n");
		fprintf(fp, "            <Type>%s</Type>\n", "Formal/Content");
		fprintf(fp, "            <Methodology>%s</Methodology>\n", "Automatic");
		fprintf(fp, "            <Level>100</Level>\n");
		fprintf(fp, "          </Validation>\n");
		fprintf(fp, "          <Access>\n");
		if (iAccess != NULL)
			fprintf(fp, "            <Availability>%s</Availability>\n", iAccess->data);
		else
			fprintf(fp, "            <Availability>%s</Availability>\n", "open access");
//		fprintf(fp, "            <Date>%s</Date>\n", fdate);
		fprintf(fp, "            <Date>%s</Date>\n", "Unspecified");
		if (mCreator != NULL)
			fprintf(fp, "            <Owner>%s</Owner>\n", mCreator->data);
		else
			fprintf(fp, "            <Owner/>\n");
		if (mPubl != NULL)
			fprintf(fp, "            <Publisher>%s</Publisher>\n", mPubl->data);
		else
			fprintf(fp, "            <Publisher/>\n");
		fprintf(fp, "            <Contact>\n");
		fprintf(fp, "              <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "              <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "              <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "              <Organisation>%s</Organisation>\n", "CMU");
		fprintf(fp, "            </Contact>\n");
		fprintf(fp, "          </Access>\n");
		fprintf(fp, "          <descriptions>\n");
		if (mDescr != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "            <Description LanguageId=\"\"/>\n");
		}
		fprintf(fp, "          </descriptions>\n");
		fprintf(fp, "          <Keys>\n");
		if (ttranscriber[0] != EOS)
			fprintf(fp, "          <Key Name=\"Transcriber\">%s</Key>\n", ttranscriber);
		if (ttranscription[0] != EOS)
			fprintf(fp, "          <Key Name=\"Transcription\">%s</Key>\n", ttranscription);
		if (tnumber[0] != EOS)
			fprintf(fp, "          <Key Name=\"NumberOfParticipants\">%s</Key>\n", tnumber);
		if (tinteraction[0] != EOS)
			fprintf(fp, "          <Key Name=\"InteractionType\">%s</Key>\n", tinteraction);
		fprintf(fp, "          </Keys>\n");
		fprintf(fp, "        </WrittenResource>\n");
		fprintf(fp, "      </Resources>\n");
		fprintf(fp, "      <References>\n");
		fprintf(fp, "        <descriptions>\n");
		if (mDescr != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "        <Description LanguageId=\"\"/>\n");
		}
		fprintf(fp, "        </descriptions>\n");
		fprintf(fp, "      </References>\n");
		fprintf(fp, "    </Session>\n");
		fprintf(fp, "    </talkbank-license-session>\n");
		fprintf(fp, "  </Components>\n");
		fprintf(fp, "</CMD>\n");
		fclose(fp);
//	} // !isJustTest
}

static void createSessionCHATFile(PATHTREE *p, char *wd_cur, METADATA *cmdata) {
	int  i, j, cnt;
	long ln;
	unsigned char PIDc;
	char isAudioFound, isVideoFound, isMissingFound, isUnlinkedFound, isMediaFound, isMediaHeaderFound,
		isUTFound, isPIDFound;
	const char *fformat;
	char *s, t, fsize[50], fdate[50], fquality[5], ftimepos[50], tdate[50], flang[LANGNUM][LANGLEN+1],
		 ttranscriber[256], ttranscription[256], tnumber[50], tinteraction[50],
		 PID_Self[PID_SIZE], PID_transcript[PID_SIZE], PID_media[PID_SIZE];
	char *line, *e, tlang[LANGLEN+1];
	METADATA *m, *mTitle, *mCreator, *mDescr, *mSubj, *mPubl, *mContributor, *mDate, *mDiscourse, *mId,
			*iGenre, *iInteractivity, *iPlanningType, *iInvolvement, *iSocialContext, *iEventStructure,
			*iChannel, *iTask, *iModalities, *iSubject, *iEthnicGroup, *iRecordingConditions, *iAccess,
			*iContinent, *iCountry, *iWrittenSubtype, *iProjectDescr, *iMediaFileDescr, *DOI, *iCorpusPID;
	CMDIIDSTYPE *rootIDs, *tID;
	FILE *fp, *fpn;

	cln++;
	if (cln > tln) {
		tln = cln + 200;
#if !defined(CLAN_SRV)
		fprintf(stderr,"\r%ld ", cln);
#endif
#if !defined(UNX)
		my_flush_chr();
#endif
	}
#if defined(_MAC_CODE) || defined(_WIN32)
#ifndef _COCOA_APP
	SysEventCheck(100L);
#endif
#endif
	s = strrchr(p->name, '.');
	if (s == NULL || uS.mStricmp(s, ".cha"))
		return;
	isPIDFound = FALSE;
	mTitle = NULL;
	mDescr = NULL;
	mSubj = NULL;
	mPubl = NULL;
	mCreator = NULL;
	mContributor = NULL;
	mDate = NULL;
	mDiscourse = NULL;
	mId = NULL;
	iGenre = NULL;
	iInteractivity = NULL;
	iPlanningType = NULL;
	iInvolvement = NULL;
	iSocialContext = NULL;
	iEventStructure = NULL;
	iChannel = NULL;
	iTask = NULL;
	iModalities = NULL;
	iSubject = NULL;
	iEthnicGroup = NULL;
	iRecordingConditions = NULL;
	iAccess = NULL;
	iContinent = NULL;
	iCountry = NULL;
	iWrittenSubtype = NULL;
	iProjectDescr = NULL;
	iMediaFileDescr = NULL;
	DOI = NULL;
	iCorpusPID = NULL;
	isUTFound = 0;
	isAudioFound = FALSE;
	isVideoFound = FALSE;
	isMediaHeaderFound = FALSE;
	isMissingFound = FALSE;
	isUnlinkedFound = FALSE;
	strcpy(fquality, "5");
	strcpy(ftimepos, "Unspecified");
	tdate[0] = EOS;
	j = 0;
	for (i=0; i < LANGNUM; i++)
		flang[i][0] = EOS;
	ttranscriber[0] = EOS;
	ttranscription[0] = EOS;
	tnumber[0] = EOS;
	tinteraction[0] = EOS;
	rootIDs = NULL;
	cMediaFileName[0] = EOS;
	strcpy(FileName2, wd_dir);
	addFilename2Path(FileName2, wd_cur+wdCurStartI);
	addFilename2Path(FileName2, p->name);
	fp = fopen(FileName2, "r");
	strcpy(fdate, cur_date);
	if (fp == NULL) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open CHAT file \"%s\".\n", FileName2);
		cutt_exit(0);
	} else {
		getFileDateSize(FileName2, fdate, fsize);
		ln = 0L;
		templineC[0] = EOS;
		while (fgets_cr(templineC1, UTTLINELEN, fp)) {
			if (templineC1[0] == '@' || templineC1[0] == '*' || templineC1[0] == '%') {
				for (i=0; templineC[i] != EOS; i++) {
					if (templineC[i] == '\t' || templineC[i] == '\n')
						templineC[i] = ' ';
				}
				if (uS.partcmp(templineC,MEDIAHEADER,FALSE,FALSE)) {
					isMediaHeaderFound = TRUE;
					for (i=strlen(MEDIAHEADER); isSpace(templineC[i]); i++) ;
					getMediaName(templineC+i, cMediaFileName, FILENAME_MAX);
					s = strrchr(cMediaFileName, '.');
					if (s != NULL)
						*s = EOS;
					cnt = 0;
					while (templineC[i]) {
						for (; uS.isskip(templineC, i, &dFnt, MBF) || templineC[i] == ',' || templineC[i] == '\n'; i++) ;
						if (templineC[i] == EOS)
							break;
						for (j=i; !uS.isskip(templineC, j, &dFnt, MBF) && templineC[j] != ',' && templineC[j] != '\n' && templineC[j] != EOS; j++) ;
						t = templineC[j];
						templineC[j] = EOS;
						cnt++;
						if (cnt > 1) {
							if (!uS.mStricmp(templineC+i, "audio"))
								isAudioFound = TRUE;
							else if (!uS.mStricmp(templineC+i, "video"))
								isVideoFound = TRUE;
							else if (!uS.mStricmp(templineC+i, "missing"))
								isMissingFound = TRUE;
							else if (!uS.mStricmp(templineC+i, "unlinked"))
								isUnlinkedFound = TRUE;
						}
						templineC[j] = t;
						i = j;
					}
				} else if (uS.partcmp(templineC,"@Recording Quality:",FALSE,FALSE)) {
					for (i=strlen("@Recording Quality:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					if (uS.mStricmp(templineC+i, "1") == 0)
						strcpy(fquality, "1");
					else if (uS.mStricmp(templineC+i, "poor") == 0 || uS.mStricmp(templineC+i, "2") == 0)
						strcpy(fquality, "2");
					else if (uS.mStricmp(templineC+i, "medium") == 0 || uS.mStricmp(templineC+i, "3") == 0)
						strcpy(fquality, "3");
					else if (uS.mStricmp(templineC+i, "good") == 0 || uS.mStricmp(templineC+i, "4") == 0)
						strcpy(fquality, "4");
					else if (uS.mStricmp(templineC+i, "excellent") == 0 || uS.mStricmp(templineC+i, "5") == 0)
						strcpy(fquality, "5");
				} else if (uS.partcmp(templineC,"@Recording Quality:",FALSE,FALSE)) {
					for (i=strlen("@Recording Quality:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					strcpy(ftimepos, templineC+i);
				} else if (uS.partcmp(templineC,"@Languages:",FALSE,FALSE)) {
					for (i=strlen("@Languages:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					line = templineC + i;
					while (*line != EOS) {
						e = strchr(line, ',');
						if (e != NULL) {
							*e = EOS;
							for (; isSpace(*line); line++) ;
							strncpy(tlang, line, LANGLEN);
							tlang[LANGLEN] = EOS;
							*e = ',';
							line = e + 1;
						} else {
							for (; isSpace(*line); line++) ;
							strncpy(tlang, line, LANGLEN);
							tlang[LANGLEN] = EOS;
							line = line + strlen(line);
						}
						uS.remblanks(tlang);
						for (i=0; i < LANGNUM; i++) {
							if (uS.mStricmp(tlang, flang[i]) == 0)
								break;
						}
						if (i >= LANGNUM) {
							for (i=0; i < LANGNUM && flang[i][0] != EOS; i++) ;
							if (i < LANGNUM) {
								strcpy(flang[i], tlang);
							}
						}
					}
				} else if (uS.partcmp(templineC, "@L1 of ", FALSE, FALSE)) {
					uS.extractString(templineC2, templineC, "@L1 of ", ':');
					for (i=strlen("@L1 of "); templineC[i] != ':' && templineC[i] != EOS; i++) ;
					if (templineC[i] == ':') {
						for (i++; isSpace(templineC[i]); i++) ;
						rootIDs = add_to_IDs(rootIDs, cmdata, FileName2, ln, NULL, NULL, templineC2, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, templineC+i, NULL);
					}
				} else if (uS.partcmp(templineC, "@Birthplace of ", FALSE, FALSE)) {
					uS.extractString(templineC2, templineC, "@Birthplace of ", ':');
					for (i=strlen("@Birthplace of "); templineC[i] != ':' && templineC[i] != EOS; i++) ;
					if (templineC[i] == ':') {
						for (i++; isSpace(templineC[i]); i++) ;
						rootIDs = add_to_IDs(rootIDs, cmdata, FileName2, ln, NULL, NULL, templineC2, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, templineC+i);
					}
				} else if (uS.partcmp(templineC, "@ID:", FALSE, FALSE)) {
					for (i=strlen("@ID:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					rootIDs = handleIDs(rootIDs, cmdata, FileName2, ln, templineC+i);
				} else if (uS.partcmp(templineC, "@Participants:", FALSE, FALSE)) {
					for (i=strlen("@Participants:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					rootIDs = handleParticipants(rootIDs, cmdata, FileName2, ln, templineC+i);
				} else if (uS.partcmp(templineC, "@Birth of ", FALSE, FALSE)) {
					uS.extractString(templineC2, templineC, "@Birth of ", ':');
					for (i=strlen("@Birth of "); templineC[i] != ':' && templineC[i] != EOS; i++) ;
					if (templineC[i] == ':') {
						for (i++; isSpace(templineC[i]); i++) ;
						rootIDs = add_to_IDs(rootIDs, cmdata, FileName2, ln, NULL, NULL, templineC2, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, templineC+i, NULL, NULL);
					}
				} else if (uS.partcmp(templineC, "@Date:", FALSE, FALSE)) {
					for (i=strlen("@Date:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					if (tdate[0] == EOS)
						convertDate(tdate, templineC+i);
				} else if (uS.partcmp(templineC, "@Transcriber:", FALSE, FALSE)) {
					for (i=strlen("@Transcriber:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					filterTextForXML(templineC2, templineC+i);
					strncpy(ttranscriber, templineC2, 255);
					ttranscriber[255] = EOS;
				} else if (uS.partcmp(templineC, "@Transcription:", FALSE, FALSE)) {
					for (i=strlen("@Transcription:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					filterTextForXML(templineC2, templineC+i);
					strncpy(ttranscription, templineC2, 255);
					ttranscription[255] = EOS;
				} else if (uS.partcmp(templineC, "@Number:", FALSE, FALSE)) {
					for (i=strlen("@Number:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					filterTextForXML(templineC2, templineC+i);
					strncpy(tnumber, templineC2, 49);
					tnumber[49] = EOS;
				} else if (uS.partcmp(templineC, "@Interaction Type:", FALSE, FALSE)) {
					for (i=strlen("@Interaction Type:"); isSpace(templineC[i]); i++) ;
					uS.remblanks(templineC+i);
					filterTextForXML(templineC2, templineC+i);
					strncpy(tinteraction, templineC2, 49);
					tinteraction[49] = EOS;
				} else if (uS.partcmp(templineC, UTF8HEADER, FALSE, FALSE)) {
					isUTFound = 1;
				}
				if (isUTFound) {
					if (uS.partcmp(templineC, UTF8HEADER, FALSE, FALSE)) {
					} else {
						if (uS.partcmp(templineC, PIDHEADER, FALSE, FALSE)) {
							for (i=strlen(PIDHEADER); isSpace(templineC[i]); i++) ;
							uS.remblanks(templineC+i);
							if (isUTFound == 2) {
								sprintf(templineC2, "In File \"%s\"\n    %s header found in a wrong place\n", FileName2, PIDHEADER);
								writeError(templineC2);
							}
							if (isPIDFound) {
								sprintf(templineC2, "In File \"%s\"\n    More than one %s header found\n", FileName2, PIDHEADER);
								writeError(templineC2);
							}
							if (invalidPID(templineC+i)) {
								fprintf(stderr, "\nIn File \"%s\"\n    %s header is corrupt or has bad prefix\n", FileName2, PIDHEADER);
								tree_root = free_tree(tree_root);
								free_metadata(cmdata);
								cleanupLanguages();
								cutt_exit(0);
							}
							isPIDFound = TRUE;
						}
						isUTFound = 2;
					}
				}
				if (templineC1[0] == '*' || templineC1[0] == '%')
					break;
				strcpy(templineC, templineC1);
			} else if (isSpace(templineC1[0]))
				strcat(templineC, templineC1);
		}
		fclose(fp);
	}
	if (isKillProgram) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		cutt_exit(0);
	}
	PIDc = 1;
	getServerURL(0, currentURL);
	addFilename2Path(currentURL, "data-cmdi");
	addFilename2Path(currentURL, wd_cur+wdCurStartI);
	addFilename2Path(currentURL, p->name);
	i = 8;
	while (currentURL[i] != EOS) {
		if (currentURL[i] == '\\')
			currentURL[i] = '/';
		if (currentURL[i] == '/' && (currentURL[i+1] == '/' || currentURL[i+1] == '\\'))
			strcpy(currentURL+i, currentURL+i+1);
		else
			i++;
	}
	for (i=strlen(currentURL); i > 0 && currentURL[i] != '/'; i--) {
		if (currentURL[i] == '.') {
			currentURL[i] = EOS;
			break;
		}
	}
	strcat(currentURL, ".cmdi");
	filterTextForXML(templineC2, currentURL);
	PIDc = writePIDbatch(templineC2, PID_Self, p->PIDp, p->PID, PIDc);
	if (PIDc == 0) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		cutt_exit(0);
	}
	if (!isPIDFound) {
		if (isJustTest) {
			filesChanged++;
		} else {
			fp = fopen(FileName2, "r");
			if (fp != NULL) {
				strcpy(FileName1, FileName2);
				strcat(FileName1, ".del");
				fpn = fopen(FileName1,"w");
				if (fpn != NULL) {
					char isEmpty;
					isEmpty = TRUE;
					isUTFound = 0;
					filesChanged++;
// fprintf(stderr, "File Changed: %s;\n", FileName2);
					templineC[0] = EOS;
					while (fgets_cr(templineC, UTTLINELEN, fp)) {
						isEmpty = FALSE;
						if (uS.partcmp(templineC, "@Begin", FALSE, FALSE) && isUTFound == 0) {
							fprintf(fpn, "%s\n", UTF8HEADER);
							fprintf(fpn, "%s\t%s\n", PIDHEADER, PID_Self);
							isUTFound = 1;
						}
						fprintf(fpn, "%s", templineC);
						if (uS.partcmp(templineC, UTF8HEADER, FALSE, FALSE) && isUTFound == 0) {
							fprintf(fpn, "%s\t%s\n", PIDHEADER, PID_Self);
							isUTFound = 1;
						}

					}
					if (isEmpty) {
						fprintf(fpn, "%s\n", UTF8HEADER);
						fprintf(fpn, "%s\t%s\n", PIDHEADER, PID_Self);
					}
					fclose(fp);
					fclose(fpn);
					if (isKillProgram) {
						unlink(FileName1);
						tree_root = free_tree(tree_root);
						free_metadata(cmdata);
						cleanupLanguages();
						cutt_exit(0);
					}
					unlink(FileName2);
#ifndef UNX
					if (rename_each_file(FileName1, FileName2, FALSE) == -1) {
						tree_root = free_tree(tree_root);
						free_metadata(cmdata);
						cleanupLanguages();
						fprintf(stderr, "\n   Can't rename original file \"%s\". Perhaps it is opened by some application.\n", FileName2);
						cutt_exit(0);
					}
#else
					rename(FileName1, FileName2);
#endif
				} else {
					tree_root = free_tree(tree_root);
					free_metadata(cmdata);
					cleanupLanguages();
					fprintf(stderr, "\n   Can't create temp file \"%s\".\n", FileName1);
					fclose(fp);
					cutt_exit(0);
				}
			}
		}
	}
	getServerURL(p->sl, templineC);
	addFilename2Path(templineC, "data-orig/");
	removeServernameAddFilename2Path(templineC, wd_cur+wdCurStartI);
	addFilename2Path(templineC, p->name);
	i = 8;
	while (templineC[i] != EOS) {
		if (templineC[i] == '\\')
			templineC[i] = '/';
		if (templineC[i] == '/' && (templineC[i+1] == '/' || templineC[i+1] == '\\'))
			strcpy(templineC+i, templineC+i+1);
		else
			i++;
	}
	filterTextForXML(templineC2, templineC);
	PIDc = writePIDbatch(templineC2, PID_transcript, p->PIDp, p->PID, PIDc);
	if (PIDc == 0) {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		cutt_exit(0);
	}
	isMediaFound = FALSE;
	if (isMediaHeaderFound) {
		strcpy(FileName1, wd_dir);
		FileName1[wdStartI] = EOS;
		addFilename2Path(FileName1, "media");
		j = strlen(FileName1);
		removeServernameAddFilename2Path(FileName1, wd_cur+wdCurStartI);
		addFilename2Path(FileName1, cMediaFileName);
		i = strlen(FileName1);
/*
		if (isAudioFound) {
			strcat(FileName1, ".wav");
			if (!access(FileName1, 0)) {
				isMediaFound = TRUE;
			} else {
				FileName1[i] = EOS;
				strcat(FileName1, ".mp3");
				if (!access(FileName1, 0)) {
					isMediaFound = TRUE;
				} else {
					FileName1[i] = EOS;
					strcat(FileName1, ".aif");
					if (!access(FileName1, 0)) {
						isMediaFound = TRUE;
					} else {
						FileName1[i] = EOS;
						strcat(FileName1, ".aiff");
						if (!access(FileName1, 0)) {
							isMediaFound = TRUE;
						}
					}
				}
			}
		} else if (isVideoFound) {
			strcat(FileName1, ".mov");
			if (!access(FileName1, 0)) {
				isMediaFound = TRUE;
			} else {
				FileName1[i] = EOS;
				strcat(FileName1, ".mp4");
				if (!access(FileName1, 0)) {
					isMediaFound = TRUE;
				} else {
					FileName1[i] = EOS;
					strcat(FileName1, ".m4v");
					if (!access(FileName1, 0)) {
						isMediaFound = TRUE;
					} else {
						FileName1[i] = EOS;
						strcat(FileName1, ".mpg");
						if (!access(FileName1, 0)) {
							isMediaFound = TRUE;
						} else {
							FileName1[i] = EOS;
							strcat(FileName1, ".mpeg");
							if (!access(FileName1, 0)) {
								isMediaFound = TRUE;
							}
						}
					}
				}
			}
		}
*/
		if (isMissingFound) {
		} else if (isAudioFound) {
			strcat(FileName1, ".mp3");
			isMediaFound = TRUE;
		} else if (isVideoFound) {
			strcat(FileName1, ".mp4");
			isMediaFound = TRUE;
		}
	}
	if (isMediaFound) {
		getMediaServerURL(p->sl, templineC);
		addFilename2Path(templineC, FileName1+j);
		i = 8;
		while (templineC[i] != EOS) {
			if (templineC[i] == '\\')
				templineC[i] = '/';
			if (templineC[i] == '/' && (templineC[i+1] == '/' || templineC[i+1] == '\\'))
				strcpy(templineC+i, templineC+i+1);
			else
				i++;
		}
		filterTextForXML(templineC2, templineC);
		PIDc = writePIDbatch(templineC2, PID_media, p->PIDp, p->PID, PIDc);
		if (PIDc == 0) {
			tree_root = free_tree(tree_root);
			free_metadata(cmdata);
			cleanupLanguages();
			cutt_exit(0);
		}
	}
//	if (!isJustTest) {
		for (m=cmdata; m != NULL; m=m->next) {
			if (!strcmp(m->tag, "Title")) {
				mTitle = m;
			} else if (!strcmp(m->tag, "Description")) {
				mDescr = m;
			} else if (!strcmp(m->tag, "Subject")) {
				mSubj = m;
			} else if (!strcmp(m->tag, "Publisher")) {
				mPubl = m;
			} else if (!strcmp(m->tag, "Creator")) {
				mCreator = m;
			} else if (!strcmp(m->tag, "Contributor")) {
				mContributor = m;
			} else if (!strcmp(m->tag, "Date")) {
				mDate = m;
			} else if (!strcmp(m->tag, "Type.olac:discourse-type")) {
				mDiscourse = m;
			} else if (!strcmp(m->tag, "Identifier")) {
				mId = m;
			} else if (!strcmp(m->tag, "IMDI_Genre")) {
				iGenre = m;
			} else if (!strcmp(m->tag, "IMDI_Interactivity")) {
				iInteractivity = m;
			} else if (!strcmp(m->tag, "IMDI_PlanningType")) {
				iPlanningType = m;
			} else if (!strcmp(m->tag, "IMDI_Involvement")) {
				iInvolvement = m;
			} else if (!strcmp(m->tag, "IMDI_SocialContext")) {
				iSocialContext = m;
			} else if (!strcmp(m->tag, "IMDI_EventStructure")) {
				iEventStructure = m;
			} else if (!strcmp(m->tag, "IMDI_Channel")) {
				iChannel = m;
			} else if (!strcmp(m->tag, "IMDI_Task")) {
				iTask = m;
			} else if (!strcmp(m->tag, "IMDI_Modalities")) {
				iModalities = m;
			} else if (!strcmp(m->tag, "IMDI_Subject")) {
				iSubject = m;
			} else if (!strcmp(m->tag, "IMDI_EthnicGroup")) {
				iEthnicGroup = m;
			} else if (!strcmp(m->tag, "IMDI_RecordingConditions")) {
				iRecordingConditions = m;
			} else if (!strcmp(m->tag, "IMDI_AccessAvailability")) {
				iAccess = m;
			} else if (!strcmp(m->tag, "IMDI_Continent")) {
				iContinent = m;
			} else if (!strcmp(m->tag, "IMDI_Country")) {
				iCountry = m;
			} else if (!strcmp(m->tag, "IMDI_WrittenResourceSubType")) {
				iWrittenSubtype = m;
			} else if (!strcmp(m->tag, "IMDI_ProjectDescription")) {
				iProjectDescr = m;
			} else if (!strcmp(m->tag, "IMDI_MediaFileDescription")) {
				iMediaFileDescr = m;
			} else if (!strcmp(m->tag, "DOI")) {
				DOI = m;
			} else if (!strcmp(m->tag, CMDIPIDHEADER)) {
				iCorpusPID = m;
			}
		}
		strcpy(FileName1, wd_cur);
		addFilename2Path(FileName1, p->name);
		s = strrchr(FileName1, '.');
		if (s != NULL)
			*s = EOS;
		strcat(FileName1, ".cmdi");
		fp = fopen(FileName1, "w");
		if (fp == NULL) {
			tree_root = free_tree(tree_root);
			free_metadata(cmdata);
			cleanupLanguages();
			fprintf(stderr, "\n   Can't open file \"%s\".\n", FileName1);
			cutt_exit(0);
		}
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(fp, "<CMD CMDVersion=\"1.1\" xmlns=\"http://www.clarin.eu/cmd/\"\n");
		fprintf(fp, "                xmlns:MPI=\"http://www.mpi.nl/\"\n");
		fprintf(fp, "                xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"\n");
		fprintf(fp, "                xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
		fprintf(fp, "                xsi:schemaLocation=\"http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/1.1/profiles/clarin.eu:cr1:p_1475136016232/xsd\">\n");
		fprintf(fp, "  <Header>\n");
		fprintf(fp, "    <MdCreator>CLAN cmdi</MdCreator>\n");
		fprintf(fp, "    <MdCreationDate>%s</MdCreationDate>\n", cur_date);
//2017-05-02		fprintf(fp, "    <MdSelfLink>%s</MdSelfLink>\n", currentURL); // 2016-12-08
		fprintf(fp, "    <MdSelfLink>hdl:%s</MdSelfLink>\n", PID_Self);
		fprintf(fp, "    <MdProfile>clarin.eu:cr1:p_1475136016232</MdProfile>\n");
		fprintf(fp, "    <MdCollectionDisplayName>TalkBank</MdCollectionDisplayName>\n");
		fprintf(fp, "  </Header>\n");
		fprintf(fp, "  <Resources>\n");
		fprintf(fp, "    <ResourceProxyList>\n");
		fprintf(fp, "      <ResourceProxy id=\"%s\">\n", "Transcript_file");
		fprintf(fp, "        <ResourceType mimetype=\"text/x-chat\">Resource</ResourceType>\n");
		fprintf(fp, "        <ResourceRef>hdl:%s</ResourceRef>\n", PID_transcript);
		fprintf(fp, "      </ResourceProxy>\n");
		if (isMediaFound) {
			fprintf(fp, "      <ResourceProxy id=\"%s\">\n", "Media_file");
			fprintf(fp, "        <ResourceType mimetype=\"audio/x-mpeg\">Resource</ResourceType>\n");
			fprintf(fp, "        <ResourceRef>hdl:%s</ResourceRef>\n", PID_media);
			fprintf(fp, "      </ResourceProxy>\n");
		}
		fprintf(fp, "    </ResourceProxyList>\n");
		fprintf(fp, "    <JournalFileProxyList/>\n");
		fprintf(fp, "    <ResourceRelationList/>\n");
		fprintf(fp, "  </Resources>\n");
		fprintf(fp, "  <Components>\n");
		fprintf(fp, "    <talkbank-license-session>\n");
		fprintf(fp, "    <License>\n");
		fprintf(fp, "      <DistributionType>public</DistributionType>\n");
		fprintf(fp, "      <LicenseName>CC BY-NC-SA 3</LicenseName>\n");
		fprintf(fp, "      <LicenseURL>%s</LicenseURL>\n", "https://creativecommons.org/licenses/by-nc-sa/3.0/");
		fprintf(fp, "    </License>\n");
		fprintf(fp, "    <Session>\n");
		filterTextForXML(templineC2, p->name);
		s = strrchr(templineC2, '.');
		if (s != NULL)
			*s = EOS;
		fprintf(fp, "      <Name>%s</Name>\n", templineC2);
		if (mTitle != NULL) {
			fprintf(fp, "      <Title>\"%s - %s\"</Title>\n", mTitle->data, templineC2);
		} else {
			fprintf(fp, "      <Title>Child \"%s\"</Title>\n", templineC2);
		}
		if (tdate[0] != EOS)
			fprintf(fp, "      <Date>%s</Date>\n", tdate);
		else
			fprintf(fp, "      <Date>%s</Date>\n", "Unspecified");
/*
		else if (mDate != NULL)
			fprintf(fp, "      <Date>%s</Date>\n", mDate->data);
		else
			fprintf(fp, "      <Date>%s</Date>\n", fdate);
		fprintf(fp, "      <descriptions>\n");
		if (mDescr != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "        <Description LanguageId=\"\"/>\n");
		}
		fprintf(fp, "      </descriptions>\n");
 */
		fprintf(fp, "      <MDGroup>\n");
		fprintf(fp, "        <Location>\n");
		if (iContinent != NULL && iCountry != NULL) {
			fprintf(fp, "          <Continent>%s</Continent>\n", iContinent->data);
			fprintf(fp, "          <Country>%s</Country>\n", iCountry->data);
		} else if (iContinent == NULL && iCountry == NULL) {
			fprintf(fp, "          <Continent>%s</Continent>\n", "North-America");
			fprintf(fp, "          <Country>%s</Country>\n", "United States");
		} else {
			sprintf(templineC2, "*** File \"%s\"\nNo \"IMDI_Continent\" or \"IMDI_Country\" field found in 0metadata.cdc for CHAT file\n", FileName2);
			writeError(templineC2);
		}
		fprintf(fp, "          <Region/>\n");
		fprintf(fp, "          <Address/>\n");
		fprintf(fp, "        </Location>\n");
		fprintf(fp, "        <Project>\n");
		outputServerTitle(fp, p->sl, mContributor);
		if (DOI != NULL)
			fprintf(fp, "          <Id>DOI %s</Id>\n", DOI->data);
		else
			fprintf(fp, "          <Id/>\n");
		fprintf(fp, "          <Contact>\n");
		fprintf(fp, "            <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "            <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "            <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "            <Organisation>%s</Organisation>\n", "CMU");
		fprintf(fp, "          </Contact>\n");
		fprintf(fp, "          <descriptions>\n");
		if (iProjectDescr != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", iProjectDescr->data);
		} else
			outputServerDescription(fp, p->sl);
		fprintf(fp, "          </descriptions>\n");
		fprintf(fp, "        </Project>\n");
		fprintf(fp, "        <Keys/>\n");
		fprintf(fp, "        <Content>\n");
		if (iGenre != NULL) {
			strcpy(templineC2, iGenre->data);
		} else if (mDiscourse != NULL) {
			if (uS.mStricmp(mDiscourse->data, "narrative") == 0)
				strcpy(templineC2, "Narrative");
			else
				strcpy(templineC2, "Discourse");
		} else
			strcpy(templineC2, "Discourse");
		fprintf(fp, "          <Genre>%s</Genre>\n", templineC2);
		fprintf(fp, "          <SubGenre/>\n");
		if (iTask != NULL)
			fprintf(fp, "          <Task>%s</Task>\n", iTask->data);
		else
			fprintf(fp, "          <Task>%s</Task>\n", "Unspecified");
		if (iModalities != NULL)
			fprintf(fp, "          <Modalities>%s</Modalities>\n", iModalities->data);
		else
			fprintf(fp, "          <Modalities>%s</Modalities>\n", "Spoken");
		if (iSubject != NULL)
			fprintf(fp, "          <Subject>%s</Subject>\n", iSubject->data);
		else
			fprintf(fp, "          <Subject>%s</Subject>\n", "Unspecified");
		fprintf(fp, "          <CommunicationContext>\n");
		if (iInteractivity != NULL)
			fprintf(fp, "            <Interactivity>%s</Interactivity>\n", iInteractivity->data);
		else
			fprintf(fp, "            <Interactivity>%s</Interactivity>\n", "interactive");
		if (iPlanningType != NULL)
			fprintf(fp, "            <PlanningType>%s</PlanningType>\n", iPlanningType->data);
		else
			fprintf(fp, "            <PlanningType>%s</PlanningType>\n", "spontaneous");
		if (iInvolvement != NULL)
			fprintf(fp, "            <Involvement>%s</Involvement>\n", iInvolvement->data);
		else
			fprintf(fp, "            <Involvement>%s</Involvement>\n", "non-elicited");
		if (iSocialContext != NULL)
			fprintf(fp, "            <SocialContext>%s</SocialContext>\n", iSocialContext->data);
		else
			fprintf(fp, "            <SocialContext>%s</SocialContext>\n", "Family");
		if (iEventStructure != NULL)
			fprintf(fp, "            <EventStructure>%s</EventStructure>\n", iEventStructure->data);
		else
			fprintf(fp, "            <EventStructure>%s</EventStructure>\n", "Conversation");
/*
		if (iChannel != NULL)
			fprintf(fp, "            <Channel>%s</Channel>\n", iChannel->data);
		else
			fprintf(fp, "            <Channel>%s</Channel>\n", "Face to Face");
*/
		fprintf(fp, "          </CommunicationContext>\n");
		fprintf(fp, "          <Content_Languages>\n");
//		fprintf(fp, "            <descriptions>\n");
//		fprintf(fp, "              <Description LanguageId=\"\"/>\n");
//		fprintf(fp, "            </descriptions>\n");
		templineC2[0] = EOS;
		for (i=0; i < LANGNUM; i++) {
			if (flang[i][0] != EOS) {
				fprintf(fp, "            <Content_Language>\n");
				if (strlen(flang[i]) < 3)
					addToUnknownLangs(flang[i], FALSE);
				getLanguageCodeAndName(flang[i], TRUE, NULL);
				if (strlen(flang[i]) == 2)
					strcpy(templineC2, "ISO639-1:");
				else
					strcpy(templineC2, "ISO639-3:");
				strcat(templineC2, flang[i]);
				fprintf(fp, "              <Id>%s</Id>\n", templineC2);
				if (!getLanguageCodeAndName(flang[i], FALSE, templineC2)) {
					addToUnknownLangs(flang[i], TRUE);
					strcpy(templineC2, "Unspecified");
				}
				fprintf(fp, "              <Name>%s</Name>\n", templineC2);
				fprintf(fp, "              <Dominant>%s</Dominant>\n", "Unspecified");
				fprintf(fp, "              <SourceLanguage>%s</SourceLanguage>\n", "Unspecified");
				fprintf(fp, "              <TargetLanguage>%s</TargetLanguage>\n", "Unspecified");
//				fprintf(fp, "              <descriptions>\n");
//				fprintf(fp, "                <Description LanguageId=\"\"/>\n");
//				fprintf(fp, "              </descriptions>\n");
				fprintf(fp, "            </Content_Language>\n");
			}
		}
		uS.remblanks(templineC2);
		fprintf(fp, "            </Content_Languages>\n");
		fprintf(fp, "            <Keys>\n");
		if (ttranscriber[0] != EOS)
			fprintf(fp, "              <Key Name=\"Transcriber\">%s</Key>\n", ttranscriber);
		if (ttranscription[0] != EOS)
			fprintf(fp, "              <Key Name=\"Transcription\">%s</Key>\n", ttranscription);
		if (tnumber[0] != EOS)
			fprintf(fp, "              <Key Name=\"NumberOfParticipants\">%s</Key>\n", tnumber);
		if (tinteraction[0] != EOS)
			fprintf(fp, "              <Key Name=\"InteractionType\">%s</Key>\n", tinteraction);
		fprintf(fp, "            </Keys>\n");
//		fprintf(fp, "            <descriptions>\n");
//		fprintf(fp, "              <Description LanguageId=\"\"/>\n");
//		fprintf(fp, "            </descriptions>\n");
		fprintf(fp, "          </Content>\n");
		fprintf(fp, "          <Actors>\n");
//		fprintf(fp, "            <descriptions>\n");
//		fprintf(fp, "              <Description LanguageId=\"\"/>\n");
//		fprintf(fp, "            </descriptions>\n");
		for (tID=rootIDs; tID != NULL; tID=tID->next_id) {
			fprintf(fp, "            <Actor>\n");
			fprintf(fp, "              <Role/>\n");
			if (tID->spname[0] == EOS)
				fprintf(fp, "              <Name>Standard Actor</Name>\n");
			else
				fprintf(fp, "              <Name>%s</Name>\n", tID->spname);
			fprintf(fp, "              <FullName/>\n");
			if (tID->code[0] == EOS)
				fprintf(fp, "              <Code>%s</Code>\n", "Unspecified");
			else
				fprintf(fp, "              <Code>%s</Code>\n", tID->code);
			if (tID->role[0] == EOS)
				fprintf(fp, "              <FamilySocialRole>%s</FamilySocialRole>\n", "Unspecified");
			else
				fprintf(fp, "              <FamilySocialRole>%s</FamilySocialRole>\n", tID->role);
			if (iEthnicGroup == NULL)
				fprintf(fp, "              <EthnicGroup>%s</EthnicGroup>\n", "Unspecified");
			else
				fprintf(fp, "              <EthnicGroup>%s</EthnicGroup>\n", iEthnicGroup->data);
			if (tID->age[0] == EOS)
				fprintf(fp, "              <Age>%s</Age>\n", "Unspecified");
			else
				fprintf(fp, "              <Age>%s</Age>\n", tID->age);
			if (tID->birth[0] == EOS)
				fprintf(fp, "              <BirthDate>%s</BirthDate>\n", "Unspecified");
			else
				fprintf(fp, "              <BirthDate>%s</BirthDate>\n", tID->birth);
			if (tID->sex == 0)
				fprintf(fp, "              <Sex>Unknown</Sex>\n");
			else if (tID->sex == 'm')
				fprintf(fp, "              <Sex>Male</Sex>\n");
			else
				fprintf(fp, "              <Sex>Female</Sex>\n");
			if (tID->education[0] == EOS)
				fprintf(fp, "              <Education>%s</Education>\n", "Unspecified");
			else
				fprintf(fp, "              <Education>%s</Education>\n", tID->education);
			fprintf(fp, "              <Anonymized>true</Anonymized>\n");
			fprintf(fp, "              <Contact>\n");
			fprintf(fp, "                <Name/>\n");
			fprintf(fp, "                <Address/>\n");
			fprintf(fp, "                <Email/>\n");
			fprintf(fp, "                <Organisation/>\n");
			fprintf(fp, "              </Contact>\n");
			fprintf(fp, "              <Keys/>\n");
//			fprintf(fp, "              <descriptions>\n");
//			fprintf(fp, "                <Description LanguageId=\"\"/>\n");
//			fprintf(fp, "              </descriptions>\n");
			fprintf(fp, "              <Actor_Languages>\n");
//			fprintf(fp, "                <descriptions>\n");
//			fprintf(fp, "                  <Description LanguageId=\"\"/>\n");
//			fprintf(fp, "                </descriptions>\n");
			for (i=0; i < LANGNUM; i++) {
				if (tID->lang[i][0] != EOS) {
					fprintf(fp, "                <Actor_Language>\n");
					if (strlen(tID->lang[i]) < 3)
						addToUnknownLangs(tID->lang[i], FALSE);
					getLanguageCodeAndName(tID->lang[i], TRUE, NULL);
					if (strlen(tID->lang[i]) == 2)
						strcpy(templineC2, "ISO639-1:");
					else
						strcpy(templineC2, "ISO639-3:");
					strcat(templineC2, tID->lang[i]);
					fprintf(fp, "                  <Id>%s</Id>\n", templineC2);
					if (!getLanguageCodeAndName(tID->lang[i], FALSE, templineC2)) {
						addToUnknownLangs(tID->lang[i], TRUE);
						strcpy(templineC2, "Unspecified");
					}
					fprintf(fp, "                  <Name>%s</Name>\n", templineC2);
					if (tID->Llang[i])
						fprintf(fp, "                  <MotherTongue>%s</MotherTongue>\n", "true");
					else
						fprintf(fp, "                  <MotherTongue>%s</MotherTongue>\n", "Unspecified");
					fprintf(fp, "                  <PrimaryLanguage>%s</PrimaryLanguage>\n", "Unspecified");
//					fprintf(fp, "                  <descriptions>\n");
//					fprintf(fp, "                    <Description LanguageId=\"\"/>\n");
//					fprintf(fp, "                  </descriptions>\n");
					fprintf(fp, "                </Actor_Language>\n");
				}
			}
			fprintf(fp, "              </Actor_Languages>\n");
			fprintf(fp, "            </Actor>\n");
		}
		fprintf(fp, "          </Actors>\n");
		fprintf(fp, "      </MDGroup>\n");
		fprintf(fp, "      <Resources>\n");
//	} // !isJustTest
	clean_ids(rootIDs);
	isMediaFound = FALSE;
	fformat = "Unspecified";
	fsize[0] = EOS;
	strcpy(fdate, cur_date);
	if (isMediaHeaderFound) {
		// checking for actual media file
		strcpy(FileName1, wd_dir);
		FileName1[wdStartI] = EOS;
		addFilename2Path(FileName1, "media");
		j = strlen(FileName1);
		removeServernameAddFilename2Path(FileName1, wd_cur+wdCurStartI);
		addFilename2Path(FileName1, cMediaFileName);
		i = strlen(FileName1);
/*
		if (isAudioFound) {
			strcat(FileName1, ".wav");
			if (!access(FileName1, 0)) {
				isMediaFound = TRUE;
				fformat = "audio/x-wav";
				getFileDateSize(FileName1, fdate, fsize);
			} else {
				FileName1[i] = EOS;
				strcat(FileName1, ".mp3");
				if (!access(FileName1, 0)) {
					isMediaFound = TRUE;
					fformat = "audio/x-mpeg";
					getFileDateSize(FileName1, fdate, fsize);
				} else {
					FileName1[i] = EOS;
					strcat(FileName1, ".aif");
					if (!access(FileName1, 0)) {
						isMediaFound = TRUE;
						fformat = "audio/x-aiff";
						getFileDateSize(FileName1, fdate, fsize);
					} else {
						FileName1[i] = EOS;
						strcat(FileName1, ".aiff");
						if (!access(FileName1, 0)) {
							isMediaFound = TRUE;
							fformat = "audio/x-aiff";
							getFileDateSize(FileName1, fdate, fsize);
						}
					}
				}
			}
		} else if (isVideoFound) {
			strcat(FileName1, ".mov");
			if (!access(FileName1, 0)) {
				isMediaFound = TRUE;
				fformat = "video/quicktime";
				getFileDateSize(FileName1, fdate, fsize);
			} else {
				FileName1[i] = EOS;
				strcat(FileName1, ".mp4");
				if (!access(FileName1, 0)) {
					isMediaFound = TRUE;
					fformat = "video/mp4";
					getFileDateSize(FileName1, fdate, fsize);
				} else {
					FileName1[i] = EOS;
					strcat(FileName1, ".m4v");
					if (!access(FileName1, 0)) {
						isMediaFound = TRUE;
						fformat = "video/mp4";
						getFileDateSize(FileName1, fdate, fsize);
					} else {
						FileName1[i] = EOS;
						strcat(FileName1, ".mpg");
						if (!access(FileName1, 0)) {
							isMediaFound = TRUE;
							fformat = "video/x-mpeg1";
							getFileDateSize(FileName1, fdate, fsize);
						} else {
							FileName1[i] = EOS;
							strcat(FileName1, ".mpeg");
							if (!access(FileName1, 0)) {
								isMediaFound = TRUE;
								fformat = "video/x-mpeg1";
								getFileDateSize(FileName1, fdate, fsize);
							}
						}
					}
				}
			}
		}
*/
		if (isMissingFound) {
		} else if (isAudioFound) {
			strcat(FileName1, ".mp3");
			isMediaFound = TRUE;
			fformat = "audio/x-mpeg";
//			getFileDateSize(FileName1, fdate, fsize);
		} else if (isVideoFound) {
			strcat(FileName1, ".mp4");
			isMediaFound = TRUE;
			fformat = "video/mp4";
//			getFileDateSize(FileName1, fdate, fsize);
		}
		if (!isMediaFound && !isMissingFound && !isUnlinkedFound) {
			FileName1[i] = EOS;
			sprintf(templineC2, "No media file: %s; CHAT: %s\n", FileName1, FileName2);
			writeError(templineC2);
		}
	}
	if (isMediaFound) {
		fprintf(fp, "        <MediaFile ref=\"Media_file\">\n");
		getMediaServerURL(p->sl, templineC);
		addFilename2Path(templineC, FileName1+j);
		i = 8;
		while (templineC[i] != EOS) {
			if (templineC[i] == '\\')
				templineC[i] = '/';
			if (templineC[i] == '/' && (templineC[i+1] == '/' || templineC[i+1] == '\\'))
				strcpy(templineC+i, templineC+i+1);
			else
				i++;
		}
		filterTextForXML(templineC2, templineC);
		fprintf(fp, "          <ResourceLink>%s</ResourceLink>\n", templineC2);
		fprintf(fp, "          <Type>%s</Type>\n", ((isAudioFound) ? "audio" : "video"));
		fprintf(fp, "          <Format>%s</Format>\n", fformat);
		if (fsize[0] == EOS)
			fprintf(fp, "          <Size/>\n", fsize);
		else
			fprintf(fp, "          <Size>%s</Size>\n", fsize);
		fprintf(fp, "          <Quality>%s</Quality>\n", fquality);
		if (iRecordingConditions == NULL)
			fprintf(fp, "          <RecordingConditions>%s</RecordingConditions>\n", "Unspecified");
		else
			fprintf(fp, "          <RecordingConditions>%s</RecordingConditions>\n", iRecordingConditions->data);
		fprintf(fp, "          <TimePosition>\n");
		fprintf(fp, "            <Start>%s</Start>\n", ftimepos);
		fprintf(fp, "            <End>Unspecified</End>\n");
		fprintf(fp, "          </TimePosition>\n");
		fprintf(fp, "          <Access>\n");
		if (iAccess != NULL)
			fprintf(fp, "            <Availability>%s</Availability>\n", iAccess->data);
		else
			fprintf(fp, "            <Availability>%s</Availability>\n", "open access");
//		fprintf(fp, "            <Date>%s</Date>\n", fdate);
		fprintf(fp, "            <Date>%s</Date>\n", "Unspecified");
		if (mCreator != NULL)
			fprintf(fp, "            <Owner>%s</Owner>\n", mCreator->data);
		else
			fprintf(fp, "            <Owner/>\n");
		if (mPubl != NULL)
			fprintf(fp, "            <Publisher>%s</Publisher>\n", mPubl->data);
		else
			fprintf(fp, "            <Publisher/>\n");
		fprintf(fp, "            <Contact>\n");
		fprintf(fp, "              <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "              <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "              <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "              <Organisation>%s</Organisation>\n", "CMU");
		fprintf(fp, "            </Contact>\n");
//		fprintf(fp, "            <descriptions>\n");
//		fprintf(fp, "              <Description LanguageId=\"\"/>\n");
//		fprintf(fp, "            </descriptions>\n");
		fprintf(fp, "          </Access>\n");
		fprintf(fp, "          <descriptions>\n");
		if (iMediaFileDescr != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", iMediaFileDescr->data);
		} else if (mDescr != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "            <Description LanguageId=\"\"/>\n");
		}
		fprintf(fp, "          </descriptions>\n");
		fprintf(fp, "          <Keys/>\n");
		fprintf(fp, "        </MediaFile>\n");
	}
	isMediaFound = FALSE;
	fformat = "Unspecified";
	fsize[0] = EOS;
	strcpy(fdate, cur_date);
	if (!access(FileName2, 0)) {
		getFileDateSize(FileName2, fdate, fsize);
	} else {
		tree_root = free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open CHAT file \"%s\".\n", FileName2);
		cutt_exit(0);
	}
//	if (!isJustTest) {
		fprintf(fp, "        <WrittenResource ref=\"Transcript_file\">\n");
		getServerURL(p->sl, templineC);
		addFilename2Path(templineC, "data-orig/");
		removeServernameAddFilename2Path(templineC, wd_cur+wdCurStartI);
		addFilename2Path(templineC, p->name);
		i = 8;
		while (templineC[i] != EOS) {
			if (templineC[i] == '\\')
				templineC[i] = '/';
			if (templineC[i] == '/' && (templineC[i+1] == '/' || templineC[i+1] == '\\'))
				strcpy(templineC+i, templineC+i+1);
			else
				i++;
		}
		filterTextForXML(templineC2, templineC);
		fprintf(fp, "          <ResourceLink>%s</ResourceLink>\n", templineC2);
		fprintf(fp, "          <MediaResourceLink/>\n");
		if (tdate[0] != EOS)
			fprintf(fp, "          <Date>%s</Date>\n", tdate);
		else
			fprintf(fp, "          <Date>%s</Date>\n", "Unspecified");
/*
		else if (mDate != NULL)
			fprintf(fp, "          <Date>%s</Date>\n", mDate->data);
		else
			fprintf(fp, "          <Date>%s</Date>\n", fdate);
*/
		fprintf(fp, "          <Type>Annotation</Type>\n");
		if (iWrittenSubtype != NULL)
			fprintf(fp, "          <SubType>%s</SubType>\n", iWrittenSubtype->data);
		else
			fprintf(fp, "          <SubType/>\n");
		fprintf(fp, "          <Format>%s</Format>\n", "text/x-chat");
		if (fsize[0] == EOS)
			fprintf(fp, "          <Size/>\n");
		else
			fprintf(fp, "          <Size>%s</Size>\n", fsize);
		fprintf(fp, "          <Derivation>%s</Derivation>\n", "Annotation");
		fprintf(fp, "          <CharacterEncoding>%s</CharacterEncoding>\n", "UTF-8");
		fprintf(fp, "          <ContentEncoding/>\n");
		fprintf(fp, "          <LanguageId/>\n");
		fprintf(fp, "          <Anonymized>true</Anonymized>\n");
		fprintf(fp, "          <Validation>\n");
		fprintf(fp, "            <Type>%s</Type>\n", "Formal/Content");
		fprintf(fp, "            <Methodology>%s</Methodology>\n", "Automatic");
		fprintf(fp, "            <Level>100</Level>\n");
//		fprintf(fp, "            <descriptions>\n");
//		fprintf(fp, "              <Description LanguageId=\"\"/>\n");
//		fprintf(fp, "            </descriptions>\n");
		fprintf(fp, "          </Validation>\n");
		fprintf(fp, "          <Access>\n");
		if (iAccess != NULL)
			fprintf(fp, "            <Availability>%s</Availability>\n", iAccess->data);
		else
			fprintf(fp, "            <Availability>%s</Availability>\n", "open access");
//		fprintf(fp, "            <Date>%s</Date>\n", fdate);
		fprintf(fp, "            <Date>%s</Date>\n", "Unspecified");
		if (mCreator != NULL)
			fprintf(fp, "            <Owner>%s</Owner>\n", mCreator->data);
		else
			fprintf(fp, "            <Owner/>\n");
		if (mPubl != NULL)
			fprintf(fp, "            <Publisher>%s</Publisher>\n", mPubl->data);
		else
			fprintf(fp, "            <Publisher/>\n");
		fprintf(fp, "            <Contact>\n");
		fprintf(fp, "              <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "              <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "              <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "              <Organisation>%s</Organisation>\n", "CMU");
		fprintf(fp, "            </Contact>\n");
//		fprintf(fp, "            <descriptions>\n");
//		fprintf(fp, "              <Description LanguageId=\"\">%s</Description>\n", "WrittenResource->Access");
//		fprintf(fp, "            </descriptions>\n");
		fprintf(fp, "          </Access>\n");
		fprintf(fp, "          <descriptions>\n");
		if (mDescr != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "            <Description LanguageId=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "            <Description LanguageId=\"\"/>\n");
		}
		fprintf(fp, "          </descriptions>\n");
		fprintf(fp, "          <Keys>\n");
		if (ttranscriber[0] != EOS)
			fprintf(fp, "          <Key Name=\"Transcriber\">%s</Key>\n", ttranscriber);
		if (ttranscription[0] != EOS)
			fprintf(fp, "          <Key Name=\"Transcription\">%s</Key>\n", ttranscription);
		if (tnumber[0] != EOS)
			fprintf(fp, "          <Key Name=\"NumberOfParticipants\">%s</Key>\n", tnumber);
		if (tinteraction[0] != EOS)
			fprintf(fp, "          <Key Name=\"InteractionType\">%s</Key>\n", tinteraction);
		fprintf(fp, "          </Keys>\n");
		fprintf(fp, "        </WrittenResource>\n");
/*
		fprintf(fp, "        <Anonyms>\n");
//		fprintf(fp, "          <ResourceLink/>\n");
		fprintf(fp, "          <Access>\n");
		fprintf(fp, "            <Availability/>\n");
//		fprintf(fp, "            <Date>%s</Date>\n", fdate);
		fprintf(fp, "            <Date>%s</Date>\n", "Unspecified");
		fprintf(fp, "            <Owner/>\n");
		fprintf(fp, "            <Publisher/>\n");
		fprintf(fp, "            <Contact>\n");
		fprintf(fp, "              <Name/>\n");
		fprintf(fp, "              <Address/>\n");
		fprintf(fp, "              <Email/>\n");
		fprintf(fp, "              <Organisation/>\n");
		fprintf(fp, "            </Contact>\n");
//		fprintf(fp, "            <descriptions>\n");
//		fprintf(fp, "              <Description LanguageId=\"\">%s</Description>\n", "Anonyms->Access");
//		fprintf(fp, "            </descriptions>\n");
		fprintf(fp, "          </Access>\n");
		fprintf(fp, "        </Anonyms>\n");
*/
		fprintf(fp, "      </Resources>\n");
		fprintf(fp, "      <References>\n");
		fprintf(fp, "        <descriptions>\n");
		if (mDescr != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "        <Description LanguageId=\"\"/>\n");
		}
		fprintf(fp, "        </descriptions>\n");
		fprintf(fp, "      </References>\n");
		fprintf(fp, "    </Session>\n");
		fprintf(fp, "    </talkbank-license-session>\n");
		fprintf(fp, "  </Components>\n");
		fprintf(fp, "</CMD>\n");
		fclose(fp);
//	} // !isJustTest
}

static void writingTree(PATHTREE *p, char *wd_cur, METADATA *cmdata) {
	int len;
	PATHTREE *sib;
	METADATA *mdata, *ldata;

	if (p != NULL) {
		mdata = NULL;
		if (p->name != NULL && !uS.mStricmp(p->name, "0metadata.cdc")) {
			mdata = readMetadata(p, wd_cur, p->name);
			cmdata = mdata;
		} else {
			for (sib=p->sibling; sib != NULL; sib=sib->sibling) {
				if (sib->name != NULL && !uS.mStricmp(sib->name, "0metadata.cdc")) {
					mdata = readMetadata(p, wd_cur, sib->name);
					cmdata = mdata;
					break;
				}
			}
		}
		len = strlen(wd_cur);
		if (p->child != NULL) {
			if (p->name != NULL) {
				addFilename2Path(wd_cur, p->name);
				ldata = NULL;
				if (mkdir(wd_cur, MODE)) {
					strcpy(templineC2, "rm -rf \"");
					strcat(templineC2, wd_cur);
					strcat(templineC2, "\"");
					system(templineC2);
					if (mkdir(wd_cur, MODE)) {
						tree_root = free_tree(tree_root);
						free_metadata(cmdata);
						cleanupLanguages();
						fprintf(stderr, "\n   Error creating folder \"%s\".\n   Folder already exists, Please delete it first.\n", wd_cur);
						cutt_exit(0);
					}
				}
				if (cmdata == NULL) {
					strcpy(FileName1, wd_dir);
					addFilename2Path(FileName1, wd_cur+wdCurStartI);
					addFilename2Path(FileName1, "0metadata.cdc");
					if (!access(FileName1, 0)) {
						ldata = readMetadata(p, wd_cur, "0metadata.cdc");
					}
				}
				if (ldata != NULL)
					createCollectionFile(p, wd_cur, ldata);
				else
					createCollectionFile(p, wd_cur, cmdata);
				if (ldata != NULL)
					free_metadata(ldata);
			}
			writingTree(p->child, wd_cur, cmdata);
		} else if (p->name != NULL) {
			if (!uS.mStricmp(p->name, "0metadata.cdc"))
				createMetadataCDCFile(p, wd_cur, cmdata);
			else
				createSessionCHATFile(p, wd_cur, cmdata);
		}
		sib = p->sibling;
		while (sib != NULL) {
			wd_cur[len] = EOS;
			if (sib->child != NULL) {
				if (sib->name != NULL) {
					addFilename2Path(wd_cur, sib->name);
					ldata = NULL;
					if (mkdir(wd_cur, MODE)) {
						strcpy(templineC2, "rm -rf \"");
						strcat(templineC2, wd_cur);
						strcat(templineC2, "\"");
						system(templineC2);
						if (mkdir(wd_cur, MODE)) {
							tree_root = free_tree(tree_root);
							free_metadata(cmdata);
							cleanupLanguages();
							fprintf(stderr, "\n   Error creating folder \"%s\".\n   Folder already exists, Please delete it first.\n", wd_cur);
							cutt_exit(0);
						}
					}
					if (cmdata == NULL) {
						strcpy(FileName1, wd_dir);
						addFilename2Path(FileName1, wd_cur+wdCurStartI);
						addFilename2Path(FileName1, "0metadata.cdc");
						if (!access(FileName1, 0)) {
							ldata = readMetadata(p, wd_cur, "0metadata.cdc");
						}
					}
					if (ldata != NULL)
						createCollectionFile(sib, wd_cur, ldata);
					else
						createCollectionFile(sib, wd_cur, cmdata);
					if (ldata != NULL)
						free_metadata(ldata);
				}
				writingTree(sib->child, wd_cur, cmdata);
			} else if (sib->name != NULL) {
				if (!uS.mStricmp(sib->name, "0metadata.cdc"))
					createMetadataCDCFile(sib, wd_cur, cmdata);
				else
					createSessionCHATFile(sib, wd_cur, cmdata);
			}
			sib = sib->sibling;
		}
		wd_cur[len] = EOS;
		if (mdata != NULL)
			free_metadata(mdata);
	}
}

static void addNewPIDNums(PATHTREE *p) {
	PATHTREE *sib;

	if (p != NULL) {
		if (p->PID == 0L) {
			PIDcnt++;
			p->PID = PIDcnt;
		}
		for (sib=p->sibling; sib != NULL; sib=sib->sibling) {
			if (sib->PID == 0L) {
				PIDcnt++;
				sib->PID = PIDcnt;
			}
			if (sib->child != NULL)
				addNewPIDNums(sib->child);
		}
		if (p->child != NULL)
			addNewPIDNums(p->child);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int  i;
	char mtdST[15], *s;
	char wd_cur[FNSize];

#ifdef UNX
	getcwd(wd_dir, FNSize);
	strcpy(od_dir, wd_dir);
	strcpy(lib_dir, DEPDIR);
	strcpy(mor_lib_dir, DEPDIR);
#endif
	strcpy(mtdST, "0metadata.cdc");
	if (argc > 1)
		argv[argc++] = mtdST;
	isJustTest = TRUE;
	isPIDDupTest = FALSE;
	if (argc > 1) {
		for (i=1; i < argc; i++) {
			if (*argv[i] == '+' || *argv[i] == '-') {
				if (argv[i][1] == 'c') {
					isJustTest = FALSE;
				} else if (argv[i][1] == 'p') {
					isPIDDupTest = TRUE;
				}
			}
		}
	}
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CMDI_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	strcpy(wd_cur, wd_dir);
	i = strlen(wd_cur) - 1;
	if (wd_cur[i] == PATHDELIMCHR)
		wd_cur[i] = EOS;
	s = strrchr(wd_cur, PATHDELIMCHR);
	if (s != NULL) {
		*s = EOS;
	}
	wdStartI = strlen(wd_dir);
	addFilename2Path(wd_cur, "data-cmdi");
	wdCurStartI = strlen(wd_cur);
	filesChanged = 0L;
	errorFp = NULL;
	PIDcnt = 0L;
	PIDFp = NULL;
	if (argc > 1 && !isPIDDupTest) {
		if (!access(wd_cur, 0)) {
			strcpy(templineC2, "rm -rf \"");
			strcat(templineC2, wd_cur);
			strcat(templineC2, "\"");
			system(templineC2);
			if (mkdir(wd_cur, MODE)) {
				tree_root = free_tree(tree_root);
				cleanupLanguages();
				fprintf(stderr, "\n   Error creating folder \"%s\".\n   Folder already exists, Please delete it first.\n", wd_cur);
				cutt_exit(0);
			}
		} else if (mkdir(wd_cur, MODE)) {
			tree_root = free_tree(tree_root);
			cleanupLanguages();
			fprintf(stderr, "\n   Error creating folder \"%s\".\n   Folder already exists, Please delete it first.\n", wd_cur);
			cutt_exit(0);
		}
	}
	tln = 0L;
	cln = 0L;
	ftime = TRUE;
	ftimeAdjust = TRUE;
	missingLangsIndex = 0;
	initLanguages();
	serverCnt = 0;
	for (i=0; serversList[i].dataURL != NULL; i++) {
		serverCnt++;
	}
	root_PID = NULL;
	bmain(argc,argv,NULL);
	if (isKillProgram) {
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		cutt_exit(0);
	}
#if !defined(CLAN_SRV)
	fprintf(stderr,"\r%ld ", cln);
#endif
	fprintf(stderr, "\nDone reading folders data\n");
	if (!isPIDDupTest) {
		if (!isJustTest)
			fprintf(stderr, "Writing data\n");
		else
			fprintf(stderr, "Testing data\n");
		LandingPage[0] = EOS;
		tln = 0L;
		cln = 0L;
		addNewPIDNums(tree_root);
		writingTree(tree_root, wd_cur, NULL);
#if !defined(CLAN_SRV)
		fprintf(stderr,"\r%ld ", cln);
#endif
		if (!isJustTest)
			fprintf(stderr, "\nDone writing data\n");
		else
			fprintf(stderr, "\nDone testing data\n");
		for (i=0; i < missingLangsIndex; i++) {
			if (isMissingLang[i] == TRUE)
				sprintf(templineC2, "Missing translation for language code: %s\n", missingLangs[i]);
			else
				sprintf(templineC2, "Two letter language code found: %s\n", missingLangs[i]);
			writeError(templineC2);
		}
	}
	cleanupLanguages();
	tree_root = free_tree(tree_root);
	root_PID = free_PID_list(root_PID);
	strcpy(FileName1, wd_dir);
	addFilename2Path(FileName1, "0error.cut");
	if (errorFp != NULL) {
		fclose(errorFp);
		fprintf(stderr, "\n    Errors were detected. Please read file:\n");
		fprintf(stderr,"*** File \"%s\"\n", FileName1);
		fprintf(stderr, "    Fix errors and run \"cmdi *.cha\" command again\n");
	} else {
		if (isPIDDupTest) {
			fprintf(stderr,"Success! No duplicate PIDs found.\n");
		} else if (isJustTest) {
#ifdef UNX
			printf("\nTHIS WAS JUST A TEST.\n");
#else
			printf("\n%c%cTHIS WAS JUST A TEST.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
			fprintf(stderr,"Success! No errors found.\n");
#ifdef UNX
			printf("NOW PLEASE RUN FOLLOWING COMMANDS:\n");
#else
			printf("%c%cNOW PLEASE RUN FOLLOWING COMMANDS:%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
			puts("1  Run Franklin's XML validator on \"data-cmdi\" folder by using this command");
			puts("validate-xml ~/data-cmdi 2> log.txt");
			puts("   XML validator is at: https://github.com/FranklinChen/validate-xml-rust");
			puts("2  Check for errors in the log.txt file by using this command:");
			puts("grep -v validates log.txt");
			puts("3  If there are errors found by XML validator, then fix them");
			puts("4  In CLAN's \"Commands\" window run \"cmdi *.cha\" again");
			puts("5  Repeat from step 1.");
			puts("Continue to the next step only if XML validator validates all .cmdi files without any errors.");
			puts("\nNow create CMDI scripts and add PIDs to database data files:");
			puts("  In CLAN's \"Commands\" window run command:");
			puts("cmdi +c *.cha");
		}
		unlink(FileName1);
	}
	if (!isPIDDupTest) {
		if (isJustTest)
			fprintf(stderr,"\n    NUMBER OF CHAT FILES WILL BE CHANGED: %ld\n\n", filesChanged);
		else
			fprintf(stderr,"\n    NUMBER OF CHAT FILES CHANGED: %ld\n\n", filesChanged);
		if (PIDFp != NULL) {
			fclose(PIDFp);
			if (!isJustTest) {
#ifdef UNX
				printf("PLEASE DO THE FOLLOWING NOW:\n");
#else
				printf("%c%cPLEASE DO THE FOLLOWING NOW:%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
				puts("  In Unix Terminal type:");
				puts("  run CHATTER on \"~/data/\" folder");
				puts("If CHATTER finds error, then fix them anyway you can including deleting repos and starting from scratch");
				puts("\nProceed only if CHATTER does not find any errors");
				puts("cd <to every server's repo>");
				puts("git status");
				puts("git commit -a");
				puts("deploy");
				puts("\tAT THIS POINT TELL LEONID TO FINISH TRANSFER DATA TO ALPHA");
				puts("*********************************************************************");
				puts("\tTransfer CMDI files inside \"data-cmdi\" folder to \"dali.talkbank.org\" server to folder \"/var/www/web/data-cmdi/\"");
				puts("\nAdding PIDs to Handle Server");
				puts("\tTransfer file \"~/data/0PID_batch.txt\" to \"Mac Gabby\" server to folder \"/Users/WORK/CLAN-data/Handle/hs\"");
				puts("After above files are transferred to \"dali.talkbank.org\" server and Mac Gabby they can be deleted.");
				puts("\nConnect to \"dali.talkbank.org\" with command \"ssh macw@dali.talkbank.org\"");
				puts("\n  In Unix Terminal on \"dali.talkbank.org\" type:");
				puts("/var/www/hs/stop.sh");
				puts("cd /var/www/hs/svr_1");
				puts("First backup and then delete folders \"bdbje\", \"txns\" and file \"txn_id\" to \"/var/www/hs/bck\"");
				puts("/var/www/hs/start.sh&");
				puts("Transfer file \"/var/www/hs/svr_1/admpriv.bin\" from \"dali.talkbank.org\" to \"Mac Gabby\" folder \"~/Downloads/admpriv.bin\"");
				puts("\n  In Unix Terminal on \"Mac Gabby\" type:");
				puts("cd /Users/WORK/CLAN-data/Handle/hs/bin/");
				puts("./hdl-admintool");
				puts("  Choose the menu option \"Tools->Home/Unhome Prefix\"");
				puts("  In the \"Prefix\" box enter \"0.NA/11312\"");
				puts("  Under \"By Site Info File (siteinfo.bin)\" click \"Choose File...\"");
				puts("  Select \"siteinfo.json\" file from directory: \"/Users/WORK/CLAN-data/Handle/hs/siteinfo.json\"");
				puts("  Click \"Do It\" button, in next window click \"OK\" button, Then enter password \"kgb1984\"");
				puts("\nStill inside \"hdl-admintool\" create new PIDs:");
				puts("  Choose the menu option \"Tools->Batch Processor\"");
				puts("  In \"Batch Processor\" window click \"Add\" button and select file \"0PID_batch.txt\"");
				puts("  Click on \"Run Batch(es)\" button and hopefully it all works....");
			}
		}
	}
	if (isKillProgram) {
		tree_root = free_tree(tree_root);
		cleanupLanguages();
		cutt_exit(0);
	}
}

static DUPPIDLIST *add_to_PID_list(DUPPIDLIST *root, char PIDp, long PID) {
	char *fname;
	DUPPIDLIST *nt;

	if (oldfname[wdStartI] == PATHDELIMCHR)
		fname = oldfname+wdStartI+1;
	else
		fname = oldfname+wdStartI;
	if (root == NULL) {
		root = NEW(DUPPIDLIST);
		nt = root;
	} else {
		nt = root;
		while (1) {
			if (nt->PIDp == PIDp && nt->PID == PID) {
				sprintf(templineC2, "Duplicate PID: %c-%ld-1 in files: %s; %s\n", PIDp, PID, nt->fname, fname);
				writeError(templineC2);
				return(root);
			}
			if (nt->next == NULL)
				break;
			nt = nt->next;
		}
		nt->next = NEW(DUPPIDLIST);
		nt = nt->next;
		if (nt == NULL)
			return(root);
	}
	if (nt == NULL) {
		tree_root = free_tree(tree_root);
		root_PID = free_PID_list(root_PID);
		cleanupLanguages();
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	nt->fname = (char *)malloc(strlen(fname)+1);
	if (nt->fname == NULL) {
		tree_root = free_tree(tree_root);
		root_PID = free_PID_list(root_PID);
		cleanupLanguages();
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	strcpy(nt->fname, fname);
	nt->PIDp = PIDp;
	nt->PID = PID;
	nt->next = NULL;
	return(root);
}

void call() {
	int  i, sl;
	long PID;
	char *s, PIDp;
	char wd_cur[FNSize], name[FILENAME_MAX];
	PATHTREE *p;

	PIDp = 'a';
	s = strrchr(oldfname, '.');
	if (s != NULL) {
		if (uS.mStricmp(s, ".cha") == 0) {
			PID = 0L;
			while (fgets_cr(templineC2, UTTLINELEN, fpin)) {
				if (templineC2[0] == '*' || uS.partcmp(templineC2, "@Begin", FALSE, FALSE))
					break;
				if (uS.partcmp(templineC2, PIDHEADER, FALSE, FALSE)) {
					for (i=strlen(PIDHEADER); isSpace(templineC2[i]); i++) ;
					uS.remblanks(templineC2+i);
					if (invalidPID(templineC2+i)) {
						fprintf(stderr, "\nIn File \"%s\"\n    %s header is corrupt or has bad prefix\n", oldfname, PIDHEADER);
						tree_root = free_tree(tree_root);
						cleanupLanguages();
						cutt_exit(0);
					}
					s = strrchr(templineC2, '-');
					if (s != NULL)
						*s = EOS;
					s = strrchr(templineC2, '-');
					if (s != NULL) {
						PID = atol(s+1);
						PIDp = *(s-1);
						if (PIDp == 'a' && PIDcnt < PID)
							PIDcnt = PID;
						if (isPIDDupTest)
							root_PID = add_to_PID_list(root_PID, PIDp, PID);
					}
				}
			}
		} else if (uS.mStricmp(s, ".cdc") == 0) {
			PID = 0L;
			while (fgets_cr(templineC2, UTTLINELEN, fpin)) {
				if (uS.partcmp(templineC2, CMDIPIDHEADER, FALSE, FALSE)) {
					for (i=strlen(CMDIPIDHEADER)+1; isSpace(templineC2[i]); i++) ;
					uS.remblanks(templineC2+i);
					if (invalidPID(templineC2+i)) {
						fprintf(stderr, "\nIn File \"%s\"\n    %s: header is corrupt or has bad prefix\n", oldfname, CMDIPIDHEADER);
						tree_root = free_tree(tree_root);
						cleanupLanguages();
						cutt_exit(0);
					}
					s = strrchr(templineC2, '-');
					if (s != NULL)
						*s = EOS;
					s = strrchr(templineC2, '-');
					if (s != NULL) {
						PID = atol(s+1);
						PIDp = *(s-1);
						if (PIDp == 'a' && PIDcnt < PID)
							PIDcnt = PID;
						if (isPIDDupTest)
							root_PID = add_to_PID_list(root_PID, PIDp, PID);
					}
				}
			}
		} else
			PID = -1L;
	} else
		PID = -1L;
	if (oldfname[wdStartI] == PATHDELIMCHR)
		strcpy(wd_cur, oldfname+wdStartI+1);
	else
		strcpy(wd_cur, oldfname+wdStartI);
	p = tree_root;
	if (strchr(wd_cur, ' ')) {
		sprintf(templineC2, "No space characters allowed in file or path name: %s\n", wd_cur);
		writeError(templineC2);
	}
	sl = getCodeForRepoName(wd_cur);

	do {
		s = strchr(wd_cur, PATHDELIMCHR);
		if (s != NULL)
			*s = EOS;
		strcpy(name, wd_cur);
//		if (!uS.mStricmp(name, "AphasiaBank") || !uS.mStricmp(name, "Password"))
//			return;
		if (tree_root == NULL) {
			tree_root = NEW(PATHTREE);
			if (tree_root == NULL) {
				tree_root = free_tree(tree_root);
				cleanupLanguages();
				fprintf(stderr, "Out of Memory!!");
				cutt_exit(0);
			}
			p = tree_root;
			p->child = NULL;
			p->sibling = NULL;
			p->name = NULL;
			p->sl = sl;
			if (s == NULL)
				p->PID = PID;
			else
				p->PID = -1;
			p->PIDp = PIDp;
			p->name = (char *)malloc(strlen(name)+1);
			if (p->name == NULL) {
				tree_root = free_tree(tree_root);
				cleanupLanguages();
				fprintf(stderr, "Out of Memory!!");
				cutt_exit(0);
			}
			strcpy(p->name, name);
		} else {
			if (p->name == NULL) {
				p->name = (char *)malloc(strlen(name)+1);
				if (p->name == NULL) {
					tree_root = free_tree(tree_root);
					cleanupLanguages();
					fprintf(stderr, "Out of Memory!!");
					cutt_exit(0);
				}
				strcpy(p->name, name);
				p->sl = sl;
				if (s == NULL)
					p->PID = PID;
				else
					p->PID = -1;
				p->PIDp = PIDp;
			} else {
				while (p->sibling != NULL) {
					if (uS.mStricmp(p->name, name) == 0)
						break;
					p = p->sibling;
				}
				if (uS.mStricmp(p->name, name) != 0) {
					p->sibling = NEW(PATHTREE);
					if (p->sibling == NULL) {
						tree_root = free_tree(tree_root);
						cleanupLanguages();
						fprintf(stderr, "Out of Memory!!");
						cutt_exit(0);
					}
					p = p->sibling;
					p->child = NULL;
					p->sibling = NULL;
					p->name = NULL;
					p->sl = sl;
					if (s == NULL)
						p->PID = PID;
					else
						p->PID = -1;
					p->PIDp = PIDp;
					p->name = (char *)malloc(strlen(name)+1);
					if (p->name == NULL) {
						tree_root = free_tree(tree_root);
						cleanupLanguages();
						fprintf(stderr, "Out of Memory!!");
						cutt_exit(0);
					}
					strcpy(p->name, name);
				}
			}
		}
		if (s != NULL) {
			strcpy(wd_cur, s+1);
			if (p->child == NULL) {
				p->child = NEW(PATHTREE);
				if (p->child == NULL) {
					tree_root = free_tree(tree_root);
					cleanupLanguages();
					fprintf(stderr, "Out of Memory!!");
					cutt_exit(0);
				}
				p = p->child;
				p->child = NULL;
				p->sibling = NULL;
				p->name = NULL;
				p->sl = sl;
				p->PID = -1;
				p->PIDp = PIDp;
			} else
				p = p->child;
		}
	} while (s != NULL) ;
	if (ftime) {
		ftime = FALSE;
		ReadLangsFile(FALSE);
		fprintf(stderr, "Reading folders data\n");
	}
	cln++;
	if (cln > tln) {
		tln = cln + 200;
#if !defined(CLAN_SRV)
		fprintf(stderr,"\r%ld ", cln);
#endif
#if !defined(UNX)
		my_flush_chr();
#endif
	}
#if defined(_MAC_CODE) || defined(_WIN32)
#ifndef _COCOA_APP
	SysEventCheck(100L);
#endif
#endif
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
				isJustTest = FALSE;
				no_arg_option(f);
				break;
		case 'd':
				strncpy(cur_date, f, 49);
				cur_date[49] = EOS;
				break;
		case 'l':
				showAll = TRUE;
				no_arg_option(f);
				break;
		case 'p':
				isPIDDupTest = TRUE;
				no_arg_option(f);
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}
