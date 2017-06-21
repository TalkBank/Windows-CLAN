/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// change date to current: <oai:datestamp>2008-04-04</oai:datestamp>

#define CHAT_MODE 4
#include "cu.h"

#if !defined(UNX)
#define _main imdi_main
#define call imdi_call
#define getflag imdi_getflag
#define init imdi_init
#define usage imdi_usage
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
	IMDI_Modalities:	speech
	IMDI_Subject:	unspecified
	IMDI_EthnicGroup:	unspecified
	IMDI_RecordingConditions:	unspecified
	IMDI_AccessAvailability:	open access
	IMDI_Continent:	Europe
	IMDI_Country:	United Kingdom
	IMDI_WrittenResourceSubType:	SubType info
	IMDI_ProjectDescription:	some One Guy text
	IMDI_MediaFileDescription:	22050 Hz, 16 bit, 1 channel
*/
#define LANGNUM		10
#define LANGLEN		10

#define CODESIZE	20
#define IDFIELSSIZE 100

#define IDSTYPE struct IDSs
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
	struct Path_Tree *child;
	struct Path_Tree *sibling;
} ;

#define TOTANUMLANGS 64

static const char *URLaddress;
static char ftime, ftimeAdjust;
static char isJustTest, showAll;
static char cur_date[50];
static char missingLangs[TOTANUMLANGS][4];
static char isMissingLang[TOTANUMLANGS];
static int missingLangsIndex;
static int wdStartI, wdCurStartI;
static long tln = 0L, cln = 0L;
static PATHTREE *tree_root;
static FILE *errorFp;

static void getCurTime(char *st) {
	time_t timer;
	struct tm *timeS;
	time(&timer);
	timeS = localtime(&timer);
	sprintf(cur_date, "%d-%s%d-%s%d", timeS->tm_year+1900,
			((timeS->tm_mon+1) < 10 ? "0":""), timeS->tm_mon+1, ((timeS->tm_mday) < 10 ? "0":""), timeS->tm_mday);
}

void init(char f) {
	if (f) {
		showAll = FALSE;
		isJustTest = FALSE;
		cur_date[0] = EOS;
		isRecursive = TRUE;
		stout = TRUE;
		combinput = TRUE;
		onlydata = 1;
		tree_root = NULL;
		ftime = TRUE;
		ftimeAdjust = TRUE;
		URLaddress = NULL;
		errorFp = NULL;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		AddCEXExtension = "";
		if (defheadtier != NULL) {
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		if (cur_date[0] == EOS)
			getCurTime(cur_date);
		if (!isJustTest && URLaddress == NULL) {
			if (uS.partwcmp(wd_dir, "/web/childes/")) {
				URLaddress = "http://childes.talkbank.org/";
			} else if (uS.partwcmp(wd_dir, "/TalkBank/")) {
				URLaddress = "http://talkbank.org/";
			} else {
				fprintf(stderr, "\n   Please specify the database you working on with +c option.\n");
				cleanupLanguages();
				cutt_exit(0);
			}
		}
	}
}

void usage() {
	printf("Usage: imdi [cS dS l t] filename(s)\n");
	puts("+cS: specify the web URL.");
	puts("+dS: specify date in YYYY-MM-DD.");
	puts("+l : show where in CHAT files all languages missing from ISO-639.cut file occur.");
	puts("+t : just run test on data media consistency, this will not create *.imdi files.");
	puts("\nExample:");
	puts("\tWorking dir must be set to \"/web/childes/data-orig/\" or \"/TalkBank/data-orig/\"");
	puts("\timdi *.cha");
	puts("\timdi -t *.cha");
	cleanupLanguages();
	cutt_exit(0);
}

static void free_tree(PATHTREE *p) {
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

static void writeError(const char *err) {
	if (errorFp == NULL) {
		strcpy(FileName1, wd_dir);
		addFilename2Path(FileName1, "0error.cut");
		errorFp = fopen(FileName1, "w");
	}
	if (errorFp != NULL)
		fprintf(errorFp, "**** %s\n", err);
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
						free_tree(tree_root);
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
					free_tree(tree_root);
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
		p->next = NEW(METADATA);
		p = p->next;
	}
	if (p == NULL) {
		free_tree(tree_root);
		cleanupLanguages();
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	p->next = NULL;
	p->tag = NULL;
	p->data = NULL;
	p->tag = (char *)malloc(strlen(h)+1);
	if (p->tag == NULL) {
		free_tree(tree_root);
		free_metadata(mdata);
		cleanupLanguages();
		fprintf(stderr, "Out of Memory!!");
		cutt_exit(0);
	}
	strcpy(p->tag, h);
	p->data = (char *)malloc(strlen(l)+1);
	if (p->data == NULL) {
		free_tree(tree_root);
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
	} else {
		id[i] = ':';
		return(FALSE);
	}
	id[i] = ':';
	return(TRUE);
}

static METADATA *parseFields(METADATA *mdata, char *line, char *fname, long ln) {
	long i;
	char *h, *l;
	char *subH;
	
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
	subH = strchr(h, '.');
	if (subH != NULL) {
		*subH = EOS;
		subH++;
	}
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
			if (subH == NULL) {
				mdata = add2MetadataList(mdata, h, l);
			} else {
				if (!strcmp(subH, "olac:language")) {
					*(subH-1) = '.';
					mdata = add2MetadataList(mdata, h, l);
				} else if (!strcmp(subH, "olac:linguistic-field")) {
					*(subH-1) = '.';
					mdata = add2MetadataList(mdata, h, l);
				} else if (!strcmp(subH, "childes:participant")) {
					*(subH-1) = '.';
					mdata = add2MetadataList(mdata, h, l);
				} else {
					sprintf(templineC2, "*** File \"%s\": line %ld.\nIllegal field  \"%s.%s\" found.", fname, ln, h, subH);
					writeError(templineC2);
				}
			}
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
			if (subH == NULL) {
				mdata = add2MetadataList(mdata, h, l);
			} else {
				if (!strcmp(subH, "olac:linguistic-type")) {
					*(subH-1) = '.';
					mdata = add2MetadataList(mdata, h, l);
				} else if (!strcmp(subH, "olac:discourse-type")) {
					*(subH-1) = '.';
					mdata = add2MetadataList(mdata, h, l);
				} else {
					sprintf(templineC2, "*** File \"%s\": line %ld.\nIllegal field  \"%s.%s\" found.", fname, ln, h, subH);
					writeError(templineC2);
				}
			}
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
	} else if (h[0] != EOS) {
		sprintf(templineC2, "*** File \"%s\": line %ld.\nIllegal field \"%s\" found.", fname, ln, h);
		writeError(templineC2);
	}
	return(mdata);
}

static METADATA *readMetadata(char *wd_cur, const char *name) {
	int  i;
	long ln;
	FILE *fp;
	METADATA *mdata;

	strcpy(FileName1, wd_dir);
	addFilename2Path(FileName1, wd_cur+wdCurStartI);
	addFilename2Path(FileName1, name);
	fp = fopen(FileName1, "r");
	if (fp == NULL) {
		free_tree(tree_root);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open file \"%s\".\n", FileName1);
		cutt_exit(0);
	} else {
		mdata = NULL;
		ln = 0L;
		templineC[0] = EOS;
		while (fgets_cr(templineC1, UTTLINELEN, fp)) {
			ln++;
			if (uS.isUTF8(templineC1) || uS.partcmp(templineC1, FONTHEADER, FALSE, FALSE))
				continue;
			if (!strcmp(templineC1,"\n"))
				continue;
			if (templineC1[0] == '%')
				break;
			uS.remblanks(templineC1);
			for (i=0; templineC1[i] != EOS && !isHeadTerm(templineC1, i); i++) ;
			if (templineC1[i] == ':') {
				if (templineC[0] != EOS)
					mdata = parseFields(mdata, templineC, FileName1, ln);
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

static void clean_ids(IDSTYPE *p) {
	IDSTYPE *t;
	
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

static IDSTYPE *add_to_IDs(IDSTYPE *rootIDs, METADATA *cmdata, char *fname, long ln, char *lang,char *corp,char *code,char *age,char *sex,
						   char *group,char *SES,char *role,char *educ,char*fu,char *spn,char *birth,char *Llang, char *BirthPlace) {
	int i;
	char *e, tlang[LANGLEN+1];
	IDSTYPE *p;
	
	if (code == NULL)
		return(rootIDs);
	
	uS.remblanks(code);
	uS.uppercasestr(code, NULL, 0);
	if (rootIDs == NULL) {
		if ((rootIDs=NEW(IDSTYPE)) == NULL)
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
			p->next_id = NEW(IDSTYPE);
			p = p->next_id;
			if (p == NULL) {
				clean_ids(rootIDs);
				free_tree(tree_root);
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

static IDSTYPE *handleParticipants(IDSTYPE *rootIDs, METADATA *cmdata, char *fname, long ln, char *line) {
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


static IDSTYPE *handleIDs(IDSTYPE *rootIDs, METADATA *cmdata, char *fname, long ln, char *line) {
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
			sprintf(templineC2, "Missing translation lang code: %s; File %s", langCode, FileName2);
		else
			sprintf(templineC2, "lang code: %s; File %s", langCode, FileName2);
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

static void createCorpusIMDIFile(PATHTREE *p, char *wd_cur, METADATA *cmdata) {
	char *s;
	FILE *fp;
	PATHTREE *sib;
	METADATA *m, *mTitle, *mDescr, *mSubj, *mPubl;

	if (isJustTest)
		return;
	strcpy(FileName1, wd_cur);
	strcat(FileName1, ".imdi");
	fp = fopen(FileName1, "w");
	if (fp == NULL) {
		free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open file \"%s\".\n", FileName1);
		cutt_exit(0);
	}
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fp, "<METATRANSCRIPT xmlns=\"http://www.mpi.nl/IMDI/Schema/IMDI\"\n");
	fprintf(fp, "                xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
	fprintf(fp, "                Date=\"%s\"\n", cur_date);
	fprintf(fp, "                FormatId=\"IMDI 3.03\"\n");
	fprintf(fp, "                Originator=\"imdi.app\"\n");
	fprintf(fp, "                Type=\"CORPUS\"\n");
	fprintf(fp, "                Version=\"0\"\n");
	fprintf(fp, "                xsi:schemaLocation=\"http://www.mpi.nl/IMDI/Schema/IMDI ./IMDI_3.0.xsd\">\n");
	fprintf(fp, "  <Corpus>\n");
	filterTextForXML(templineC2, p->name);
	fprintf(fp, "    <Name>%s</Name>\n", templineC2);
	mTitle = NULL;
	mDescr = NULL;
	mSubj = NULL;
	mPubl = NULL;
	for (m=cmdata; m != NULL; m=m->next) {
		if (!strcmp(m->tag, "Title"))
			mTitle = m;
		else if (!strcmp(m->tag, "Description"))
			mDescr = m;
		else if (!strcmp(m->tag, "Subject"))
			mSubj = m;
		else if (!strcmp(m->tag, "Publisher"))
			mPubl = m;
	}
	if (mTitle != NULL) {
		fprintf(fp, "    <Title>\"%s\"</Title>\n", mTitle->data);
	} else {
		filterTextForXML(templineC2, p->name);
		fprintf(fp, "    <Title>Corpus \"%s\"</Title>\n", templineC2);
	}
	if (mDescr != NULL) {
		fprintf(fp, "    <Description LanguageId=\"\" Link=\"\">%s</Description>\n", mDescr->data);
	} else if (mSubj != NULL) {
		fprintf(fp, "    <Description LanguageId=\"\" Link=\"\">%s</Description>\n", mSubj->data);
	} else {
		fprintf(fp, "    <Description LanguageId=\"\" Link=\"\">Participants' Annotations</Description>\n");
	}
	for (sib=p->child; sib != NULL; sib=sib->sibling) {
		if (uS.mStricmp(sib->name, "0metadata.cdc")) {
			strcpy(FileName1, p->name);
			addFilename2Path(FileName1, sib->name);
			s = strrchr(FileName1, '.');
			if (s != NULL)
				*s = EOS;
			strcat(FileName1, ".imdi");
			filterTextForXML(templineC2, FileName1);
			fprintf(fp, "    <CorpusLink Name=\"\">%s</CorpusLink>\n", templineC2);
		}
	}
	fprintf(fp, "  </Corpus>\n");
	fprintf(fp, "</METATRANSCRIPT>\n");
	fclose(fp);
}

static void createSessionIMDIFile(PATHTREE *p, char *wd_cur, METADATA *cmdata) {
	int  i, j, cnt;
	long ln;
	char isAudioFound, isVideoFound, isMissingFound, isUnlinkedFound, isMediaFound, isMediaHeaderFound;
	const char *fformat;
	char *s, t, fsize[50], fdate[50], fquality[5], ftimepos[50], tdate[50], flang[LANGNUM][LANGLEN+1],
		 ttranscriber[256], ttranscription[256], tnumber[50], tinteraction[50];
	char *line, *e, tlang[LANGLEN+1];
	METADATA *m, *mTitle, *mCreator, *mDescr, *mSubj, *mPubl, *mContributor, *mDate, *mDiscourse, *mId, *iGenre, *iInteractivity,
			 *iPlanningType, *iInvolvement, *iSocialContext, *iEventStructure, *iChannel, *iTask, *iModalities,
			 *iSubject, *iEthnicGroup, *iRecordingConditions, *iAccess, *iContinent, *iCountry, *iWrittenSubtype, *iProjectDescr, *iMediaFileDescr;
	IDSTYPE *rootIDs, *tID;
	FILE *fp;

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
	SysEventCheck(100L);
#endif
	if (!uS.mStricmp(p->name, "0metadata.cdc"))
		return;
	s = strrchr(p->name, '.');
	if (s == NULL || uS.mStricmp(s, ".cha"))
		return;
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
	isAudioFound = FALSE;
	isVideoFound = FALSE;
	isMediaHeaderFound = FALSE;
	isMissingFound = FALSE;
	isUnlinkedFound = FALSE;
	strcpy(fquality, "5");
	strcpy(ftimepos, "Unspecified");
	tdate[0] = EOS;
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
		free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open CHAT file \"%s\".\n", FileName2);
		cutt_exit(0);
	} else {
		getFileDateSize(FileName2, fdate, fsize);
		ln = 0L;
		templineC[0] = EOS;
		while (fgets_cr(templineC1, 255, fp)) {
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
				}
				if (templineC1[0] == '*' || templineC1[0] == '%')
					break;
				strcpy(templineC, templineC1);
			} else if (isSpace(templineC1[0]))
				strcat(templineC, templineC1);
		}
		fclose(fp);
	}
	strcpy(FileName1, wd_cur);
	addFilename2Path(FileName1, p->name);
	s = strrchr(FileName1, '.');
	if (s != NULL)
		*s = EOS;
	strcat(FileName1, ".imdi");
	if (!isJustTest) {
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
			else if (!strcmp(m->tag, "Contributor"))
				mContributor = m;
			else if (!strcmp(m->tag, "Date"))
				mDate = m;
			else if (!strcmp(m->tag, "Type.olac:discourse-type"))
				mDiscourse = m;
			else if (!strcmp(m->tag, "Identifier"))
				mId = m;
			else if (!strcmp(m->tag, "IMDI_Genre"))
				iGenre = m;
			else if (!strcmp(m->tag, "IMDI_Interactivity"))
				iInteractivity = m;
			else if (!strcmp(m->tag, "IMDI_PlanningType"))
				iPlanningType = m;
			else if (!strcmp(m->tag, "IMDI_Involvement"))
				iInvolvement = m;
			else if (!strcmp(m->tag, "IMDI_SocialContext"))
				iSocialContext = m;
			else if (!strcmp(m->tag, "IMDI_EventStructure"))
				iEventStructure = m;
			else if (!strcmp(m->tag, "IMDI_Channel"))
				iChannel = m;
			else if (!strcmp(m->tag, "IMDI_Task"))
				iTask = m;
			else if (!strcmp(m->tag, "IMDI_Modalities"))
				iModalities = m;
			else if (!strcmp(m->tag, "IMDI_Subject"))
				iSubject = m;
			else if (!strcmp(m->tag, "IMDI_EthnicGroup"))
				iEthnicGroup = m;
			else if (!strcmp(m->tag, "IMDI_RecordingConditions"))
				iRecordingConditions = m;
			else if (!strcmp(m->tag, "IMDI_AccessAvailability"))
				iAccess = m;
			else if (!strcmp(m->tag, "IMDI_Continent"))
				iContinent = m;
			else if (!strcmp(m->tag, "IMDI_Country"))
				iCountry = m;
			else if (!strcmp(m->tag, "IMDI_WrittenResourceSubType"))
				iWrittenSubtype = m;
			else if (!strcmp(m->tag, "IMDI_ProjectDescription"))
				iProjectDescr = m;
			else if (!strcmp(m->tag, "IMDI_MediaFileDescription"))
				iMediaFileDescr = m;
		}
		fp = fopen(FileName1, "w");
		if (fp == NULL) {
			free_tree(tree_root);
			free_metadata(cmdata);
			cleanupLanguages();
			fprintf(stderr, "\n   Can't open file \"%s\".\n", FileName1);
			cutt_exit(0);
		}
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(fp, "<METATRANSCRIPT xmlns=\"http://www.mpi.nl/IMDI/Schema/IMDI\"\n");
		fprintf(fp, "                xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
		fprintf(fp, "                Date=\"%s\"\n", cur_date);
		fprintf(fp, "                FormatId=\"IMDI 3.03\"\n");
		fprintf(fp, "                Originator=\"imdi.app\"\n");
		fprintf(fp, "                Type=\"SESSION\"\n");
		fprintf(fp, "                Version=\"0\"\n");
		fprintf(fp, "                xsi:schemaLocation=\"http://www.mpi.nl/IMDI/Schema/IMDI ./IMDI_3.0.xsd\">\n");
		fprintf(fp, "  <Session>\n");
		filterTextForXML(templineC2, p->name);
		s = strrchr(templineC2, '.');
		if (s != NULL)
			*s = EOS;
		fprintf(fp, "    <Name>%s</Name>\n", templineC2);
		if (mTitle != NULL) {
			fprintf(fp, "    <Title>\"%s:%s\"</Title>\n", mTitle->data, templineC2);
		} else {
			fprintf(fp, "    <Title>Child \"%s\"</Title>\n", templineC2);
		}
		if (tdate[0] != EOS)
			fprintf(fp, "    <Date>%s</Date>\n", tdate);
		else
			fprintf(fp, "    <Date>%s</Date>\n", "Unspecified");
/*
		else if (mDate != NULL)
			fprintf(fp, "    <Date>%s</Date>\n", mDate->data);
		else
			fprintf(fp, "    <Date>%s</Date>\n", fdate);
*/
		if (mDescr != NULL) {
			fprintf(fp, "    <Description LanguageId=\"\" Link=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "    <Description LanguageId=\"\" Link=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "    <Description LanguageId=\"\" Link=\"\"/>\n");
		}
		fprintf(fp, "    <MDGroup>\n");
		fprintf(fp, "      <Location>\n");
		if (iContinent != NULL && iCountry != NULL) {
			fprintf(fp, "        <Continent Link=\"http://www.mpi.nl/IMDI/Schema/Continents.xml\" Type=\"ClosedVocabulary\">%s</Continent>\n", iContinent->data);
			fprintf(fp, "        <Country Link=\"http://www.mpi.nl/IMDI/Schema/Countries.xml\" Type=\"ClosedVocabulary\">%s</Country>\n", iCountry->data);
		} else if (iContinent == NULL && iCountry == NULL) {
			fprintf(fp, "        <Continent Link=\"http://www.mpi.nl/IMDI/Schema/Continents.xml\" Type=\"ClosedVocabulary\">%s</Continent>\n", "North-America");
			fprintf(fp, "        <Country Link=\"http://www.mpi.nl/IMDI/Schema/Countries.xml\" Type=\"ClosedVocabulary\">%s</Country>\n", "United States");
		} else {
			sprintf(templineC2, "No \"IMDI_Continent\" or \"IMDI_Country\" field found in 0metadata.cdc for CHAT file\n\t%s", FileName2);
			writeError(templineC2);
		}
		fprintf(fp, "        <Region/>\n");
		fprintf(fp, "        <Address/>\n");
		fprintf(fp, "      </Location>\n");
		fprintf(fp, "      <Project>\n");
		if (strncmp(URLaddress, "http://childes.talkbank.org/", 27) == 0) {
			if (mContributor != NULL)
				fprintf(fp, "        <Name>%s</Name>\n", mContributor->data);
			else
				fprintf(fp, "        <Name>%s</Name>\n", "CHILDES");
			fprintf(fp, "        <Title>%s</Title>\n", "Child Language Data Exchange System");
		} else if (strncmp(URLaddress, "http://talkbank.org/", 20) == 0) {
			if (mContributor != NULL)
				fprintf(fp, "        <Name>%s</Name>\n", mContributor->data);
			else
				fprintf(fp, "        <Name>%s</Name>\n", "TalkBank");
			fprintf(fp, "        <Title>%s</Title>\n", "TalkBank is an interdisciplinary research project");
		} else {
			if (mContributor != NULL)
				fprintf(fp, "        <Name>%s</Name>\n", mContributor->data);
			else
				fprintf(fp, "        <Name>%s</Name>\n", "Unspecified");
			fprintf(fp, "        <Title>%s</Title>\n", "Unspecified");
		}
		if (mId != NULL)
			fprintf(fp, "        <Id>ISBN %s</Id>\n", mId->data);
		else
			fprintf(fp, "        <Id/>\n");
		fprintf(fp, "        <Contact>\n");
		fprintf(fp, "          <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "          <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "          <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "          <Organisation>%s</Organisation>\n", "CMU");
		fprintf(fp, "        </Contact>\n");
		if (iProjectDescr != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", iProjectDescr->data);
		} else if (strncmp(URLaddress, "http://childes.talkbank.org/", 27) == 0) {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", "CHILDES is the child language component of the TalkBank system. TalkBank is a system for sharing and studying conversational interactions.");
		} else if (strncmp(URLaddress, "http://talkbank.org/", 20) == 0) {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", "TalkBank is a system for sharing and studying conversational interactions.");
		} else {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", "Unspecified");
		}
		fprintf(fp, "      </Project>\n");
		fprintf(fp, "      <Keys>\n");
//		fprintf(fp, "        <Key Name=\"Standard Template Key\"/>\n");
		fprintf(fp, "      </Keys>\n");
		fprintf(fp, "      <Content>\n");
		if (iGenre != NULL) {
			strcpy(templineC2, iGenre->data);
		} else if (mDiscourse != NULL) {
			if (uS.mStricmp(mDiscourse->data, "narrative") == 0)
				strcpy(templineC2, "Narrative");
			else
				strcpy(templineC2, "Discourse");
		} else
			strcpy(templineC2, "Discourse");
		fprintf(fp, "        <Genre Link=\"http://www.mpi.nl/IMDI/Schema/Content-Genre.xml\" Type=\"OpenVocabulary\">%s</Genre>\n", templineC2);
		fprintf(fp, "        <SubGenre Link=\"http://www.mpi.nl/IMDI/Schema/Content-SubGenre.xml\" Type=\"OpenVocabularyList\"/>\n");
		if (iTask != NULL)
			fprintf(fp, "        <Task Link=\"http://www.mpi.nl/IMDI/Schema/Content-Task.xml\" Type=\"OpenVocabulary\">%s</Task>\n", iTask->data);
		else
			fprintf(fp, "        <Task Link=\"http://www.mpi.nl/IMDI/Schema/Content-Task.xml\" Type=\"OpenVocabulary\">%s</Task>\n", "Unspecified");
		if (iModalities != NULL)
			fprintf(fp, "        <Modalities Link=\"http://www.mpi.nl/IMDI/Schema/Content-Modalities.xml\" Type=\"OpenVocabularyList\">%s</Modalities>\n", iModalities->data);
		else
			fprintf(fp, "        <Modalities Link=\"http://www.mpi.nl/IMDI/Schema/Content-Modalities.xml\" Type=\"OpenVocabularyList\">%s</Modalities>\n", "speech");
		if (iSubject != NULL)
			fprintf(fp, "        <Subject Link=\"http://www.mpi.nl/IMDI/Schema/Content-Subject.xml\" Type=\"OpenVocabularyList\">%s</Subject>\n", iSubject->data);
		else
			fprintf(fp, "        <Subject Link=\"http://www.mpi.nl/IMDI/Schema/Content-Subject.xml\" Type=\"OpenVocabularyList\">%s</Subject>\n", "Unspecified");
		fprintf(fp, "        <CommunicationContext>\n");
		if (iInteractivity != NULL)
			fprintf(fp, "          <Interactivity Link=\"http://www.mpi.nl/IMDI/Schema/Content-Interactivity.xml\" Type=\"ClosedVocabulary\">%s</Interactivity>\n", iInteractivity->data);
		else
			fprintf(fp, "          <Interactivity Link=\"http://www.mpi.nl/IMDI/Schema/Content-Interactivity.xml\" Type=\"ClosedVocabulary\">%s</Interactivity>\n", "interactive");
		if (iPlanningType != NULL)
			fprintf(fp, "          <PlanningType Link=\"http://www.mpi.nl/IMDI/Schema/Content-PlanningType.xml\" Type=\"ClosedVocabulary\">%s</PlanningType>\n", iPlanningType->data);
		else
			fprintf(fp, "          <PlanningType Link=\"http://www.mpi.nl/IMDI/Schema/Content-PlanningType.xml\" Type=\"ClosedVocabulary\">%s</PlanningType>\n", "spontaneous");
		if (iInvolvement != NULL)
			fprintf(fp, "          <Involvement Link=\"http://www.mpi.nl/IMDI/Schema/Content-Involvement.xml\" Type=\"ClosedVocabulary\">%s</Involvement>\n", iInvolvement->data);
		else
			fprintf(fp, "          <Involvement Link=\"http://www.mpi.nl/IMDI/Schema/Content-Involvement.xml\" Type=\"ClosedVocabulary\">%s</Involvement>\n", "non-elicited");
		if (iSocialContext != NULL)
			fprintf(fp, "          <SocialContext Link=\"http://www.mpi.nl/IMDI/Schema/Content-SocialContext.xml\" Type=\"ClosedVocabulary\">%s</SocialContext>\n", iSocialContext->data);
		else
			fprintf(fp, "          <SocialContext Link=\"http://www.mpi.nl/IMDI/Schema/Content-SocialContext.xml\" Type=\"ClosedVocabulary\">%s</SocialContext>\n", "Family");
		if (iEventStructure != NULL)
			fprintf(fp, "          <EventStructure Link=\"http://www.mpi.nl/IMDI/Schema/Content-EventStructure.xml\" Type=\"ClosedVocabulary\">%s</EventStructure>\n", iEventStructure->data);
		else
			fprintf(fp, "          <EventStructure Link=\"http://www.mpi.nl/IMDI/Schema/Content-EventStructure.xml\" Type=\"ClosedVocabulary\">%s</EventStructure>\n", "Conversation");
		if (iChannel != NULL)
			fprintf(fp, "          <Channel Link=\"http://www.mpi.nl/IMDI/Schema/Content-Channel.xml\" Type=\"ClosedVocabulary\">%s</Channel>\n", iChannel->data);
		else
			fprintf(fp, "          <Channel Link=\"http://www.mpi.nl/IMDI/Schema/Content-Channel.xml\" Type=\"ClosedVocabulary\">%s</Channel>\n", "Face to Face");
		fprintf(fp, "        </CommunicationContext>\n");
		fprintf(fp, "        <Languages>\n");
		fprintf(fp, "          <Description LanguageId=\"\" Link=\"\"/>\n");
		templineC2[0] = EOS;
		for (i=0; i < LANGNUM; i++) {
			if (flang[i][0] != EOS) {
				fprintf(fp, "            <Language>\n");
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
				fprintf(fp, "              <Name Link=\"http://www.mpi.nl/IMDI/Schema/MPI-Languages.xml\" Type=\"OpenVocabulary\">%s</Name>\n", templineC2);
				fprintf(fp, "              <Dominant Link=\"http://www.mpi.nl/IMDI/Schema/Boolean.xml\" Type=\"ClosedVocabulary\">%s</Dominant>\n", "Unspecified");
				fprintf(fp, "              <SourceLanguage Link=\"http://www.mpi.nl/IMDI/Schema/Boolean.xml\" Type=\"ClosedVocabulary\">%s</SourceLanguage>\n", "Unspecified");
				fprintf(fp, "              <TargetLanguage Link=\"http://www.mpi.nl/IMDI/Schema/Boolean.xml\" Type=\"ClosedVocabulary\">%s</TargetLanguage>\n", "Unspecified");
				fprintf(fp, "              <Description LanguageId=\"\" Link=\"\"/>\n");
				fprintf(fp, "            </Language>\n");
			}
		}
		uS.remblanks(templineC2);

		fprintf(fp, "        </Languages>\n");
		fprintf(fp, "        <Keys>\n");
		if (ttranscriber[0] != EOS)
			fprintf(fp, "          <Key Name=\"Transcriber\">%s</Key>\n", ttranscriber);
		if (ttranscription[0] != EOS)
			fprintf(fp, "          <Key Name=\"Transcription\">%s</Key>\n", ttranscription);
		if (tnumber[0] != EOS)
			fprintf(fp, "          <Key Name=\"NumberOfParticipants\">%s</Key>\n", tnumber);
		if (tinteraction[0] != EOS)
			fprintf(fp, "          <Key Name=\"InteractionType\">%s</Key>\n", tinteraction);
		fprintf(fp, "        </Keys>\n");
		fprintf(fp, "        <Description LanguageId=\"\" Link=\"\"/>\n");
		fprintf(fp, "      </Content>\n");
		fprintf(fp, "      <Actors>\n");
		fprintf(fp, "        <Description LanguageId=\"\" Link=\"\"/>\n");
		for (tID=rootIDs; tID != NULL; tID=tID->next_id) {
			fprintf(fp, "        <Actor>\n");
			fprintf(fp, "          <Role Link=\"http://www.mpi.nl/IMDI/Schema/Actor-Role.xml\" Type=\"OpenVocabularyList\"/>\n");
			if (tID->spname[0] == EOS)
				fprintf(fp, "          <Name>Standard Actor</Name>\n");
			else
				fprintf(fp, "          <Name>%s</Name>\n", tID->spname);
			fprintf(fp, "          <FullName/>\n");
			if (tID->code[0] == EOS)
				fprintf(fp, "          <Code/>\n");
			else
				fprintf(fp, "          <Code>%s</Code>\n", tID->code);
			if (tID->role[0] == EOS)
				fprintf(fp, "          <FamilySocialRole Link=\"http://www.mpi.nl/IMDI/Schema/Actor-FamilySocialRole.xml\" Type=\"OpenVocabularyList\"/>\n");
			else
				fprintf(fp, "          <FamilySocialRole Link=\"http://www.mpi.nl/IMDI/Schema/Actor-FamilySocialRole.xml\" Type=\"OpenVocabularyList\">%s</FamilySocialRole>\n", tID->role);
			fprintf(fp, "          <Languages>\n");
			fprintf(fp, "            <Description LanguageId=\"\" Link=\"\"/>\n");
			for (i=0; i < LANGNUM; i++) {
				if (tID->lang[i][0] != EOS) {
					fprintf(fp, "            <Language>\n");
					if (strlen(tID->lang[i]) < 3)
						addToUnknownLangs(tID->lang[i], FALSE);
					getLanguageCodeAndName(tID->lang[i], TRUE, NULL);
					if (strlen(tID->lang[i]) == 2)
						strcpy(templineC2, "ISO639-1:");
					else
						strcpy(templineC2, "ISO639-3:");
					strcat(templineC2, tID->lang[i]);
					fprintf(fp, "              <Id>%s</Id>\n", templineC2);
					if (!getLanguageCodeAndName(tID->lang[i], FALSE, templineC2)) {
						addToUnknownLangs(tID->lang[i], TRUE);
						strcpy(templineC2, "Unspecified");
					}
					fprintf(fp, "              <Name Link=\"http://www.mpi.nl/IMDI/Schema/MPI-Languages.xml\" Type=\"OpenVocabulary\">%s</Name>\n", templineC2);
					if (tID->Llang[i])
						fprintf(fp, "              <MotherTongue Link=\"http://www.mpi.nl/IMDI/Schema/Boolean.xml\" Type=\"ClosedVocabulary\">%s</MotherTongue>\n", "true");
					else
						fprintf(fp, "              <MotherTongue Link=\"http://www.mpi.nl/IMDI/Schema/Boolean.xml\" Type=\"ClosedVocabulary\">%s</MotherTongue>\n", "Unspecified");
					fprintf(fp, "              <PrimaryLanguage Link=\"http://www.mpi.nl/IMDI/Schema/Boolean.xml\" Type=\"ClosedVocabulary\">%s</PrimaryLanguage>\n", "Unspecified");
					fprintf(fp, "              <Description LanguageId=\"\" Link=\"\"/>\n");
					fprintf(fp, "            </Language>\n");
				}
			}
			fprintf(fp, "          </Languages>\n");
			if (iEthnicGroup != NULL)
				fprintf(fp, "          <EthnicGroup>%s</EthnicGroup>\n", iEthnicGroup->data);
			else
				fprintf(fp, "          <EthnicGroup/>\n");
			if (tID->age[0] == EOS)
				fprintf(fp, "          <Age>Unspecified</Age>\n");
			else
				fprintf(fp, "          <Age>%s</Age>\n", tID->age);
			if (tID->birth[0] == EOS)
				fprintf(fp, "          <BirthDate>Unspecified</BirthDate>\n");
			else
				fprintf(fp, "          <BirthDate>%s</BirthDate>\n", tID->birth);
			if (tID->sex == 0)
				fprintf(fp, "          <Sex Link=\"http://www.mpi.nl/IMDI/Schema/Actor-Sex.xml\" Type=\"ClosedVocabulary\">Unknown</Sex>");
			else if (tID->sex == 'm')
				fprintf(fp, "          <Sex Link=\"http://www.mpi.nl/IMDI/Schema/Actor-Sex.xml\" Type=\"ClosedVocabulary\">Male</Sex>\n");
			else
				fprintf(fp, "          <Sex Link=\"http://www.mpi.nl/IMDI/Schema/Actor-Sex.xml\" Type=\"ClosedVocabulary\">Female</Sex>\n");
			if (tID->education[0] == EOS)
				fprintf(fp, "          <Education>Unspecified</Education>\n");
			else
				fprintf(fp, "          <Education>%s</Education>\n", tID->education);
			fprintf(fp, "          <Anonymized Link=\"http://www.mpi.nl/IMDI/Schema/Boolean.xml\" Type=\"ClosedVocabulary\">true</Anonymized>\n");
			fprintf(fp, "          <Contact>\n");
			fprintf(fp, "            <Name/>\n");
			fprintf(fp, "            <Address/>\n");
			fprintf(fp, "            <Email/>\n");
			fprintf(fp, "            <Organisation/>\n");
			fprintf(fp, "          </Contact>\n");
			fprintf(fp, "          <Keys>\n");
			if (tID->BirthPlace[0] != EOS)
				fprintf(fp, "            <Key Name=\"Standard Template Key\">Born in %s</Key>\n", tID->BirthPlace);
			fprintf(fp, "          </Keys>\n");
			fprintf(fp, "          <Description LanguageId=\"\" Link=\"\"/>\n");
			fprintf(fp, "        </Actor>\n");
		}
		fprintf(fp, "      </Actors>\n");
		fprintf(fp, "    </MDGroup>\n");
		fprintf(fp, "    <Resources>\n");
	}
	clean_ids(rootIDs);
	isMediaFound = FALSE;
	fformat = "Unspecified";
	fsize[0] = EOS;
	strcpy(fdate, cur_date);
	if (isMediaHeaderFound) {
		// checking for actual media file
		strcpy(FileName1, wd_dir);
		FileName1[wdStartI] = EOS;
		j = strlen(FileName1);
		addFilename2Path(FileName1, "media");
		addFilename2Path(FileName1, wd_cur+wdCurStartI);
		addFilename2Path(FileName1, cMediaFileName);
		i = strlen(FileName1);
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
		if (!isMediaFound && !isMissingFound && !isUnlinkedFound) {
			FileName1[i] = EOS;
			sprintf(templineC2, "No media file: %s; CHAT: %s", FileName1, FileName2);
			writeError(templineC2);
		}
	}
	if (!isJustTest && isMediaFound) {
		fprintf(fp, "      <MediaFile>\n");
		strcpy(templineC, URLaddress);
		strcat(templineC, FileName1+j);
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
		fprintf(fp, "        <ResourceLink>%s</ResourceLink>\n", templineC2);
		fprintf(fp, "        <Type Link=\"http://www.mpi.nl/IMDI/Schema/MediaFile-Type.xml\" Type=\"ClosedVocabulary\">%s</Type>\n", ((isAudioFound) ? "audio" : "video"));
		fprintf(fp, "        <Format Link=\"http://www.mpi.nl/IMDI/Schema/MediaFile-Format.xml\" Type=\"OpenVocabulary\">%s</Format>\n", fformat);
		if (fsize[0] == EOS)
			fprintf(fp, "        <Size/>\n", fsize);
		else
			fprintf(fp, "        <Size>%s</Size>\n", fsize);
		fprintf(fp, "        <Quality>%s</Quality>\n", fquality);
		if (iRecordingConditions != NULL)
			fprintf(fp, "        <RecordingConditions>%s</RecordingConditions>\n", iRecordingConditions->data);
		else
			fprintf(fp, "        <RecordingConditions/>\n");
		fprintf(fp, "        <TimePosition>\n");
		fprintf(fp, "          <Start>%s</Start>\n", ftimepos);
		fprintf(fp, "          <End>Unspecified</End>\n");
		fprintf(fp, "        </TimePosition>\n");
		fprintf(fp, "        <Access>\n");
		if (iAccess != NULL)
			fprintf(fp, "          <Availability>%s</Availability>\n", iAccess->data);
		else
			fprintf(fp, "          <Availability>%s</Availability>\n", "open access");
//		fprintf(fp, "          <Date>%s</Date>\n", fdate);
		fprintf(fp, "          <Date>%s</Date>\n", "Unspecified");
		if (mCreator != NULL)
			fprintf(fp, "          <Owner>%s</Owner>\n", mCreator->data);
		else
			fprintf(fp, "          <Owner/>\n");
		if (mPubl != NULL)
			fprintf(fp, "          <Publisher>%s</Publisher>\n", mPubl->data);
		else
			fprintf(fp, "          <Publisher/>\n");
		fprintf(fp, "          <Contact>\n");
		fprintf(fp, "            <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "            <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "            <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "            <Organisation>%s</Organisation>\n", "CMU");
		fprintf(fp, "          </Contact>\n");
		fprintf(fp, "          <Description LanguageId=\"\" Link=\"\"/>\n");
		fprintf(fp, "        </Access>\n");
		if (iMediaFileDescr != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", iMediaFileDescr->data);
		} else if (mDescr != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\"/>\n");
		}
		fprintf(fp, "        <Keys>\n");
		fprintf(fp, "        </Keys>\n");
		fprintf(fp, "      </MediaFile>\n");
	}
	isMediaFound = FALSE;
	fformat = "Unspecified";
	fsize[0] = EOS;
	strcpy(fdate, cur_date);
	if (isMediaHeaderFound) {
		// checking for streaming media file
		strcpy(FileName1, wd_dir);
		j = strlen(FileName1);
		addFilename2Path(FileName1, wd_cur+wdCurStartI);
		addFilename2Path(FileName1, "media");
		addFilename2Path(FileName1, cMediaFileName);
		strcat(FileName1, ".mov");
		if (!access(FileName1, 0)) {
			isMediaFound = TRUE;
			getFileDateSize(FileName1, fdate, fsize);
			fformat = "video/quicktime";
		}
		if (!isMediaFound && !isMissingFound && !isUnlinkedFound) {
			sprintf(templineC2, "No reference movie: %s; CHAT: %s", FileName1, FileName2);
			writeError(templineC2);
		}
	}
/*
	if (!isJustTest && isMediaFound) {
		fprintf(fp, "      <MediaFile>\n");
		strcpy(templineC, URLaddress);
		strcat(templineC, "data-orig/");
		strcat(templineC, FileName1+j);
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
		fprintf(fp, "        <ResourceLink>%s</ResourceLink>\n", templineC2);
		fprintf(fp, "        <Type Link=\"http://www.mpi.nl/IMDI/Schema/MediaFile-Type.xml\" Type=\"ClosedVocabulary\">%s</Type>\n", "video");
		fprintf(fp, "        <Format Link=\"http://www.mpi.nl/IMDI/Schema/MediaFile-Format.xml\" Type=\"OpenVocabulary\">%s</Format>\n", fformat);
		if (fsize[0] == EOS)
			fprintf(fp, "        <Size/>\n", fsize);
		else
			fprintf(fp, "        <Size>%s</Size>\n", fsize);
		fprintf(fp, "        <Quality>%s</Quality>\n", fquality);
		fprintf(fp, "        <RecordingConditions/>\n");
		fprintf(fp, "        <TimePosition>\n");
		fprintf(fp, "          <Start>%s</Start>\n", ftimepos);
		fprintf(fp, "          <End>Unspecified</End>\n");
		fprintf(fp, "        </TimePosition>\n");
		fprintf(fp, "        <Access>\n");
		fprintf(fp, "          <Availability>%s</Availability>\n", "open access");
		fprintf(fp, "          <Date>%s</Date>\n", fdate);
		if (mCreator != NULL)
			fprintf(fp, "          <Owner>%s</Owner>\n", mCreator->data);
		else
			fprintf(fp, "          <Owner/>\n");
		if (mPubl != NULL)
			fprintf(fp, "          <Publisher>%s</Publisher>\n", mPubl->data);
		else
			fprintf(fp, "          <Publisher/>\n");
		fprintf(fp, "          <Contact>\n");
		fprintf(fp, "            <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "            <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "            <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "            <Organisation>%s</Organisation>\n", "CMU");
		fprintf(fp, "          </Contact>\n");
		fprintf(fp, "          <Description LanguageId=\"\" Link=\"\"/>\n");
		fprintf(fp, "        </Access>\n");
		fprintf(fp, "        <Description LanguageId=\"\" Link=\"\"/>\n");
		fprintf(fp, "        <Keys>\n");
		fprintf(fp, "        </Keys>\n");
		fprintf(fp, "      </MediaFile>\n");
	}
*/
	fsize[0] = EOS;
	strcpy(fdate, cur_date);
	if (!access(FileName2, 0)) {
		getFileDateSize(FileName2, fdate, fsize);
	} else {
		free_tree(tree_root);
		free_metadata(cmdata);
		cleanupLanguages();
		fprintf(stderr, "\n   Can't open CHAT file \"%s\".\n", FileName2);
		cutt_exit(0);
	}
	if (!isJustTest) {
		fprintf(fp, "      <WrittenResource>\n");
		strcpy(templineC, URLaddress);
		strcat(templineC, "data-orig/");
		strcat(templineC, wd_cur+wdCurStartI);
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
		fprintf(fp, "        <ResourceLink>%s</ResourceLink>\n", templineC2);
		fprintf(fp, "        <MediaResourceLink/>\n");
		if (tdate[0] != EOS)
			fprintf(fp, "        <Date>%s</Date>\n", tdate);
		else
			fprintf(fp, "        <Date>%s</Date>\n", "Unspecified");
/*
		else if (mDate != NULL)
			fprintf(fp, "        <Date>%s</Date>\n", mDate->data);
		else
			fprintf(fp, "        <Date>%s</Date>\n", fdate);
*/
		fprintf(fp, "        <Type Link=\"http://www.mpi.nl/IMDI/Schema/WrittenResource-Type.xml\" Type=\"OpenVocabulary\">Annotation</Type>\n");
		if (iWrittenSubtype != NULL)
			fprintf(fp, "        <SubType Link=\"http://www.mpi.nl/IMDI/Schema/WrittenResource-SubType.xml\" Type=\"OpenVocabularyList\">%s</SubType>\n", iWrittenSubtype->data);
		else
			fprintf(fp, "        <SubType Link=\"http://www.mpi.nl/IMDI/Schema/WrittenResource-SubType.xml\" Type=\"OpenVocabularyList\"/>\n");
		fprintf(fp, "        <Format Link=\"http://www.mpi.nl/IMDI/Schema/WrittenResource-Format.xml\" Type=\"OpenVocabulary\">%s</Format>\n", "text/x-chat");
		if (fsize[0] == EOS)
			fprintf(fp, "        <Size/>\n", fsize);
		else
			fprintf(fp, "        <Size>%s</Size>\n", fsize);
		fprintf(fp, "        <Validation>\n");
		fprintf(fp, "          <Type Link=\"http://www.mpi.nl/IMDI/Schema/Validation-Type.xml\" Type=\"ClosedVocabulary\">%s</Type>\n", "Formal/Content");
		fprintf(fp, "          <Methodology Link=\"http://www.mpi.nl/IMDI/Schema/Validation-Methodology.xml\" Type=\"ClosedVocabulary\">%s</Methodology>\n", "Automatic");
		fprintf(fp, "          <Level>100</Level>\n");
		fprintf(fp, "          <Description LanguageId=\"\" Link=\"\"/>\n");
		fprintf(fp, "        </Validation>\n");
		fprintf(fp, "        <Derivation Link=\"http://www.mpi.nl/IMDI/Schema/WrittenResource-Derivation.xml\" Type=\"ClosedVocabulary\">%s</Derivation>\n", "Annotation");
		fprintf(fp, "        <CharacterEncoding>%s</CharacterEncoding>\n", "UTF-8");
		fprintf(fp, "        <ContentEncoding/>\n");
		fprintf(fp, "        <LanguageId/>\n");
		fprintf(fp, "        <Anonymized Link=\"http://www.mpi.nl/IMDI/Schema/Boolean.xml\" Type=\"ClosedVocabulary\">true</Anonymized>\n");
		fprintf(fp, "        <Access>\n");
		if (iAccess != NULL)
			fprintf(fp, "          <Availability>%s</Availability>\n", iAccess->data);
		else
			fprintf(fp, "          <Availability>%s</Availability>\n", "open access");
//		fprintf(fp, "          <Date>%s</Date>\n", fdate);
		fprintf(fp, "          <Date>%s</Date>\n", "Unspecified");
		if (mCreator != NULL)
			fprintf(fp, "          <Owner>%s</Owner>\n", mCreator->data);
		else
			fprintf(fp, "          <Owner/>\n");
		if (mPubl != NULL)
			fprintf(fp, "          <Publisher>%s</Publisher>\n", mPubl->data);
		else
			fprintf(fp, "          <Publisher/>\n");
		fprintf(fp, "          <Contact>\n");
		fprintf(fp, "            <Name>%s</Name>\n", "Prof. Brian MacWhinney");
		fprintf(fp, "            <Address>%s</Address>\n", "Psychology Dept., 5000 Forbes av., Pittsburgh, PA 15213");
		fprintf(fp, "            <Email>%s</Email>\n", "macw@cmu.edu");
		fprintf(fp, "            <Organisation>%s</Organisation>\n", "CMU");
		fprintf(fp, "          </Contact>\n");
		fprintf(fp, "          <Description LanguageId=\"\" Link=\"\"/>\n");
		fprintf(fp, "        </Access>\n");
		if (mDescr != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", mDescr->data);
		} else if (mSubj != NULL) {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\">%s</Description>\n", mSubj->data);
		} else {
			fprintf(fp, "        <Description LanguageId=\"\" Link=\"\"/>\n");
		}
		fprintf(fp, "        <Keys>\n");
		if (ttranscriber[0] != EOS)
			fprintf(fp, "          <Key Name=\"Transcriber\">%s</Key>\n", ttranscriber);
		if (ttranscription[0] != EOS)
			fprintf(fp, "          <Key Name=\"Transcription\">%s</Key>\n", ttranscription);
		if (tnumber[0] != EOS)
			fprintf(fp, "          <Key Name=\"NumberOfParticipants\">%s</Key>\n", tnumber);
		if (tinteraction[0] != EOS)
			fprintf(fp, "          <Key Name=\"InteractionType\">%s</Key>\n", tinteraction);
		fprintf(fp, "        </Keys>\n");
		fprintf(fp, "      </WrittenResource>\n");
		fprintf(fp, "      <Anonyms>\n");
		fprintf(fp, "        <ResourceLink/>\n");
		fprintf(fp, "        <Access>\n");
		fprintf(fp, "          <Availability/>\n");
//		fprintf(fp, "          <Date>%s</Date>\n", fdate);
		fprintf(fp, "          <Date>%s</Date>\n", "Unspecified");
		fprintf(fp, "          <Owner/>\n");
		fprintf(fp, "          <Publisher/>\n");
		fprintf(fp, "          <Contact>\n");
		fprintf(fp, "            <Name/>\n");
		fprintf(fp, "            <Address/>\n");
		fprintf(fp, "            <Email/>\n");
		fprintf(fp, "            <Organisation/>\n");
		fprintf(fp, "          </Contact>\n");
		fprintf(fp, "          <Description LanguageId=\"\" Link=\"\"/>\n");
		fprintf(fp, "        </Access>\n");
		fprintf(fp, "      </Anonyms>\n");
		fprintf(fp, "    </Resources>\n");
		fprintf(fp, "    <References>\n");
		fprintf(fp, "      <Description LanguageId=\"\" Link=\"\"/>\n");
		fprintf(fp, "    </References>\n");
		fprintf(fp, "  </Session>\n");
		fprintf(fp, "</METATRANSCRIPT>\n");
		fclose(fp);
	}
}

static void writingTree(PATHTREE *p, char *wd_cur, METADATA *cmdata) {
	int len;
	PATHTREE *sib;
	METADATA *mdata, *ldata;

	if (p != NULL) {
		mdata = NULL;
		if (p->name != NULL && !uS.mStricmp(p->name, "0metadata.cdc")) {
			mdata = readMetadata(wd_cur, p->name);
			cmdata = mdata;
		} else {
			for (sib=p->sibling; sib != NULL; sib=sib->sibling) {
				if (sib->name != NULL && !uS.mStricmp(sib->name, "0metadata.cdc")) {
					mdata = readMetadata(wd_cur, sib->name);
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
				if (!isJustTest) {
					if (mkdir(wd_cur, MODE)) {
						free_tree(tree_root);						free_metadata(cmdata);
						cleanupLanguages();
						fprintf(stderr, "\n   Error creating folder \"%s\".\n   Folder already exists, Please delete it first.\n", wd_cur);
						cutt_exit(0);
					}
					if (cmdata == NULL) {
						strcpy(FileName1, wd_dir);
						addFilename2Path(FileName1, wd_cur+wdCurStartI);
						addFilename2Path(FileName1, "0metadata.cdc");
						if (!access(FileName1, 0)) {
							ldata = readMetadata(wd_cur, "0metadata.cdc");
						}
					}
				}
				if (ldata != NULL)
					createCorpusIMDIFile(p, wd_cur, ldata);
				else
					createCorpusIMDIFile(p, wd_cur, cmdata);
				if (ldata != NULL)
					free_metadata(ldata);
			}
			writingTree(p->child, wd_cur, cmdata);
		} else if (p->name != NULL) {
			createSessionIMDIFile(p, wd_cur, cmdata);
		}
		sib = p->sibling;
		while (sib != NULL) {
			wd_cur[len] = EOS;
			if (sib->child != NULL) {
				if (sib->name != NULL) {
					addFilename2Path(wd_cur, sib->name);
					ldata = NULL;
					if (!isJustTest) {
						if (mkdir(wd_cur, MODE)) {
							free_tree(tree_root);
							free_metadata(cmdata);
							cleanupLanguages();
							fprintf(stderr, "\n   Error creating folder \"%s\".\n   Folder already exists, Please delete it first.\n", wd_cur);
							cutt_exit(0);
						}
						if (cmdata == NULL) {
							strcpy(FileName1, wd_dir);
							addFilename2Path(FileName1, wd_cur+wdCurStartI);
							addFilename2Path(FileName1, "0metadata.cdc");
							if (!access(FileName1, 0)) {
								ldata = readMetadata(wd_cur, "0metadata.cdc");
							}
						}
					}
					if (ldata != NULL)
						createCorpusIMDIFile(sib, wd_cur, ldata);
					else
						createCorpusIMDIFile(sib, wd_cur, cmdata);
					if (ldata != NULL)
						free_metadata(ldata);
				}
				writingTree(sib->child, wd_cur, cmdata);
			} else if (sib->name != NULL) {
				createSessionIMDIFile(sib, wd_cur, cmdata);
			}
			sib = sib->sibling;
		}
		wd_cur[len] = EOS;
		if (mdata != NULL)
			free_metadata(mdata);
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
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = IMDI_P;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	strcpy(wd_cur, wd_dir);
	s = strrchr(wd_cur, PATHDELIMCHR);
	if (s != NULL) {
		*s = EOS;
		wdCurStartI = strlen(s+1) + 1;		
	} else
		wdCurStartI = 0;
	wdStartI = strlen(wd_cur);
	addFilename2Path(wd_cur, "data-imdi");
	wdCurStartI = wdCurStartI + strlen(wd_cur);
	errorFp = NULL;
	isJustTest = FALSE;
	if (argc > 1) {
		for (i=1; i < argc; i++) {
			if (*argv[i] == '+'  || *argv[i] == '-') {
				if (argv[i][1] == 't') {
					isJustTest = TRUE;
					break;
				}
			}
		}
		if (!isJustTest) {
			if (mkdir(wd_cur, MODE)) {
				free_tree(tree_root);
				cleanupLanguages();
				fprintf(stderr, "\n   Error creating folder \"%s\".\n   Folder already exists, Please delete it first.\n", wd_cur);
				cutt_exit(0);
			}
		}
	}
	tln = 0L;
	cln = 0L;
	ftime = TRUE;
	ftimeAdjust = TRUE;
	missingLangsIndex = 0;
	initLanguages();
	bmain(argc,argv,NULL);
#if !defined(CLAN_SRV)
	fprintf(stderr,"\r%ld ", cln);
#endif
	fprintf(stderr, "\nDone reading folders data\n");
	if (!isJustTest)
		fprintf(stderr, "Writing data\n");
	else
		fprintf(stderr, "Testing data\n");
	tln = 0L;
	cln = 0L;
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
			sprintf(templineC2, "Missing tarnslation for language code: %s", missingLangs[i]);
		else
			sprintf(templineC2, "Two letter language code found: %s", missingLangs[i]);
		writeError(templineC2);
	}
	cleanupLanguages();
	free_tree(tree_root);
	tree_root = NULL;
	strcpy(FileName1, wd_dir);
	addFilename2Path(FileName1, "0error.cut");
	if (errorFp != NULL) {
		fclose(errorFp);
		fprintf(stderr, "\n    Errors were detected. Please read file:\n");
		fprintf(stderr,"*** File \"%s\": line 1.\n", FileName1);
	} else {
		fprintf(stderr,"\nSuccess! No errors found.\n\n");
		unlink(FileName1);
	}
}

void call() {
	char *s;
	char wd_cur[FNSize], name[FILENAME_MAX];
	PATHTREE *p;

	if (oldfname[wdStartI] == PATHDELIMCHR)
		strcpy(wd_cur, oldfname+wdStartI+1);
	else
		strcpy(wd_cur, oldfname+wdStartI);
	p = tree_root;
	if (strchr(wd_cur, ' ')) {
		sprintf(templineC2, "No space characters allowed in file or path name: %s", wd_cur);
		writeError(templineC2);
	}
	do {
		s = strchr(wd_cur, PATHDELIMCHR);
		if (s != NULL)
			*s = EOS;
		strcpy(name, wd_cur);
		if (uS.mStricmp(name, "data-orig") == 0) {
			if (strncmp(URLaddress, "http://childes.talkbank.org/", 27) == 0) {
				strcpy(name, "childes");
				if (ftimeAdjust) {
					ftimeAdjust = FALSE;
					wdCurStartI -= 2;
				}
			} else if (strncmp(URLaddress, "http://talkbank.org/", 20) == 0) {
				strcpy(name, "talkbank");
				if (ftimeAdjust) {
					ftimeAdjust = FALSE;
					wdCurStartI -= 1;
				}
			}
		}
		if (!uS.mStricmp(name, "AphasiaBank") || !uS.mStricmp(name, "Password"))
			return;
		if (tree_root == NULL) {
			tree_root = NEW(PATHTREE);
			if (tree_root == NULL) {
				free_tree(tree_root);
				cleanupLanguages();
				fprintf(stderr, "Out of Memory!!");
				cutt_exit(0);
			}
			p = tree_root;
			p->child = NULL;
			p->sibling = NULL;
			p->name = NULL;
			p->name = (char *)malloc(strlen(name)+1);
			if (p->name == NULL) {
				free_tree(tree_root);
				cleanupLanguages();
				fprintf(stderr, "Out of Memory!!");
				cutt_exit(0);
			}
			strcpy(p->name, name);
		} else {
			if (p->name == NULL) {
				p->name = (char *)malloc(strlen(name)+1);
				if (p->name == NULL) {
					free_tree(tree_root);
					cleanupLanguages();
					fprintf(stderr, "Out of Memory!!");
					cutt_exit(0);
				}
				strcpy(p->name, name);
			} else {
				while (p->sibling != NULL) {
					if (uS.mStricmp(p->name, name) == 0)
						break;
					p = p->sibling;
				}
				if (uS.mStricmp(p->name, name) != 0) {
					p->sibling = NEW(PATHTREE);
					if (p->sibling == NULL) {
						free_tree(tree_root);
						cleanupLanguages();
						fprintf(stderr, "Out of Memory!!");
						cutt_exit(0);
					}
					p = p->sibling;
					p->child = NULL;
					p->sibling = NULL;
					p->name = NULL;
					p->name = (char *)malloc(strlen(name)+1);
					if (p->name == NULL) {
						free_tree(tree_root);
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
					free_tree(tree_root);
					cleanupLanguages();
					fprintf(stderr, "Out of Memory!!");
					cutt_exit(0);
				}
				p = p->child;
				p->child = NULL;
				p->sibling = NULL;
				p->name = NULL;
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
	SysEventCheck(100L);
#endif
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
				URLaddress = f;
				break;
		case 't':
				isJustTest = TRUE;
				break;
		case 'l':
				showAll = TRUE;
				break;
		case 'd':
				strncpy(cur_date, f, 49);
				cur_date[49] = EOS;
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}
