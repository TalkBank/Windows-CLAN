/**********************************************************************
	"Copyright 2006 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "ced.h"
#include "cu.h"
#include "cutt-xml.h"
#ifdef _WIN32
	#include "TextUtils.h"
#endif

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char AddCEXExtension;

#define UNK_TYPE 0
#define CHI_TYPE 1
#define MOT_TYPE 2

#define TAGSLEN  128

typedef struct PaidusAtribs ATTRIBS;
struct PaidusAtribs {
	char tag[TAGSLEN];
	char chatName[TAGSLEN];
	struct PaidusAtribs *nextTag;
} ;

typedef struct TimeTable TIMESTABLE;
struct TimeTable {
	char timeTag[7];
	long tagValue;
	long timeValue;
	char supplement[16];
	struct TimeTable *nextTime;
} ;

typedef struct AllChatTiers ALLCHATTIERS;
struct AllChatTiers {
	char *sp;
	char *line;
	TIMESTABLE *beg, *end;
	ALLCHATTIERS *depTiers;
	ALLCHATTIERS *nextTier;
} ;

#define IDSNUMBER 10
struct IDS {
	char lan[21];
	char sp[8];
	char sex[11];
	char age[16];
	char name[33];
	char whichSpeaker;
} ;
static struct IDS IDsTable[IDSNUMBER];
static int idsCnt;

static char mediaFName[FILENAME_MAX+2];
static ALLCHATTIERS *RootTiers;
static Element *Paidus_stack[30];
static ATTRIBS *attsRoot;
static TIMESTABLE *timeRoot;
#ifdef _MAC_CODE
static TextEncodingVariant lVariant;
#endif

void usage() {
	printf("convert Paidus XML files to CHAT files\n");
	printf("Usage: temp [d %s] filename(s)\n",mainflgs());
    puts("+dF: specify tags dependencies file F.");
	puts("Example: temp -dpadius_attribs.cut filename");
	mainusage();
}

static ATTRIBS *freeAttribs(ATTRIBS *p) {
	ATTRIBS *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextTag;
		free(t);
	}
	return(NULL);
}

static TIMESTABLE *freeTimeTable(TIMESTABLE *p) {
	TIMESTABLE *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextTime;
		free(t);
	}
	return(NULL);
}

static ALLCHATTIERS *freeTiers(ALLCHATTIERS *p) {
	ALLCHATTIERS *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextTier;
		if (t->depTiers != NULL)
			freeTiers(t->depTiers);
		if (t->sp != NULL)
			free(t->sp);
		if (t->line != NULL)
			free(t->line);
		free(t);
	}
	return(NULL);
}

static void freePaidusMem(void) {
	attsRoot = freeAttribs(attsRoot);
	timeRoot = freeTimeTable(timeRoot);
	RootTiers = freeTiers(RootTiers);
}

void init(char s) {
	extern char GExt[];

	if (s) {
		OverWriteFile = TRUE;
		AddCEXExtension = FALSE;
		stout = FALSE;
		onlydata = 1;
		attsRoot = NULL;
		timeRoot = NULL;
		RootTiers = NULL;
		RootElem = NULL;
#ifdef _MAC_CODE
		lVariant = kTextEncodingDefaultVariant;
#endif
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
		}
	} else
		strcpy(GExt, ".cha");
	idsCnt = 0;
}

static ATTRIBS *add_each_paidusTag(ATTRIBS *root, char *fname, long ln, char *tag, char *chatName) {
	long len;
	ATTRIBS *p;

	if (tag[0] == EOS)
		return(root);

	if (root == NULL) {
		if ((p=NEW(ATTRIBS)) == NULL)
			out_of_mem();
		root = p;
	} else {
		for (p=root; p->nextTag != NULL && uS.mStricmp(p->tag, tag); p=p->nextTag) ;
		if (uS.mStricmp(p->tag, tag) == 0)
			return(root);
		
		if ((p->nextTag=NEW(ATTRIBS)) == NULL)
			out_of_mem();
		p = p->nextTag;
	}

	p->nextTag = NULL;
	if ((strlen(tag) >= TAGSLEN) || (strlen(chatName) >= TAGSLEN)) {
		freePaidusMem();
		fprintf(stderr,"*** File \"%s\"", fname);
		if (ln != 0)
			fprintf(stderr,": line %ld.\n", ln);
		fprintf(stderr, "Tag(s) too long. Longer than %d characters\n", TAGSLEN);
		cutt_exit(0);
	}
	strcpy(p->tag, tag);
	strcpy(p->chatName, chatName);
	if (p->chatName[0] == '@' || p->chatName[0] == '*' || p->chatName[0] == '%') {
		len = strlen(p->chatName) - 1;
		if (p->chatName[len] != ':')
			strcat(p->chatName, ":");
	}
	return(root);
}

static void rd_PaidusAtts_f(char *fname) {
	int  cnt;
	char *tag, *chatName, isQTF;
	long i, j, ln;
	FNType mFileName[FNSize];
	FILE *fp;

	if (*fname == EOS) {
		fprintf(stderr,	"No dep. tags file specified.\n");
		cutt_exit(0);
	}
	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open either one of dep. tags files:\n\t\"%s\", \"%s\"\n", fname, mFileName);
		cutt_exit(0);
	}
	ln = 0L;
	while (fgets_cr(templineC, 255, fp)) {
		if (uS.isUTF8(templineC))
			continue;
		ln++;
		if (templineC[0] == ';')
			continue;
		uS.remblanks(templineC);
		if (templineC[0] == EOS)
			continue;
		i = 0;
		cnt = 0;
		tag = "";
		chatName = "";
		isQTF = FALSE;
		while (1) {
			for (; isSpace(templineC[i]); i++) ;
			if (templineC[i] == '"') {
				isQTF = !isQTF;
				i++;
			}
			for (j=i; (!isSpace(templineC[j]) || isQTF) && templineC[j] != EOS; j++) {
				if (templineC[j] == '"')
					isQTF = !isQTF;
			}
			if (cnt == 0)
				tag = templineC+i;
			else if (cnt == 1)
				chatName = templineC+i;
			i = j;
			if (i > 0 && templineC[i-1] == '"')
				i--;
			templineC[i] = EOS;
			if (templineC[j] == EOS)
				break;
			i = j + 1;
			cnt++;
		}
		if (tag[0] == EOS || chatName[0] == EOS) {
			freePaidusMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
			if (tag[0] == EOS)
				fprintf(stderr, "Missing Paidus tag\n");
			else if (chatName[0] == EOS)
				fprintf(stderr, "Missing Chat tier name for \"%s\"\n", tag);
			cutt_exit(0);
		}
		attsRoot = add_each_paidusTag(attsRoot, fname, ln, tag, chatName);
	}
	fclose(fp);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	attsRoot = freeAttribs(attsRoot);
}

void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'd':
			if (*f) {
				rd_PaidusAtts_f(getfarg(f,f1,i));
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static TIMESTABLE *addToTimeTable(TIMESTABLE *root, char *timeTag, char *supplement, long timeValue) {
	TIMESTABLE *p;
	
	if (timeTag[0] == EOS)
		return(root);
	
	if (root == NULL) {
		if ((p=NEW(TIMESTABLE)) == NULL)
			out_of_mem();
		root = p;
	} else {
		for (p=root; p->nextTime != NULL; p=p->nextTime) ;		
		if ((p->nextTime=NEW(TIMESTABLE)) == NULL)
			out_of_mem();
		p = p->nextTime;
	}
	p->nextTime = NULL;
	strcpy(p->timeTag, timeTag);
	p->tagValue = atol(timeTag+1);
	strcpy(p->supplement, supplement);
	p->timeValue = timeValue;
	return(root);
}

static TIMESTABLE *getTimeTableRef(TIMESTABLE *p, char *timeTag, char *supplement) {
	for (; p != NULL; p=p->nextTime) {
		if (!uS.mStricmp(p->timeTag, timeTag) && !uS.mStricmp(p->supplement, supplement))
			return(p);
	}
	fprintf(stderr, "Time used in records is not specified in \"common-timeline\" table:\n", stackIndex);
	fprintf(stderr, "    %s%s\n", timeTag, supplement);
	freeXML_Elements();
	freePaidusMem();
	cutt_exit(0);
	return(NULL);
}

static ALLCHATTIERS *Paidus_insertNT(ALLCHATTIERS *nt, ALLCHATTIERS *tnt) {
	if (nt == RootTiers) {
		RootTiers = NEW(ALLCHATTIERS);
		if (RootTiers == NULL)
			out_of_mem();
		RootTiers->nextTier = nt;
		nt = RootTiers;
	} else {
		nt = NEW(ALLCHATTIERS);
		if (nt == NULL)
			out_of_mem();
		nt->nextTier = tnt->nextTier;
		tnt->nextTier = nt;
	}
	return(nt);
}

static void Paidus_fillNT(ALLCHATTIERS *nt, TIMESTABLE *beg, TIMESTABLE *end, char *sp, char *line) {
	nt->depTiers = NULL;
	nt->sp = (char *)malloc(strlen(sp)+1);
	if (nt->sp == NULL)
		out_of_mem();
	strcpy(nt->sp, sp);
	nt->line = (char *)malloc(strlen(line)+1);
	if (nt->line == NULL)
		out_of_mem();
	strcpy(nt->line, line);
	nt->beg = beg;
	nt->end = end;
}

static ALLCHATTIERS *Paidus_addDepTiers(ALLCHATTIERS *depTiers, TIMESTABLE *beg, TIMESTABLE *end, char *sp, char *line) {
	ALLCHATTIERS *nt, *tnt;

	if (depTiers == NULL) {
		if ((depTiers=NEW(ALLCHATTIERS)) == NULL)
			out_of_mem();
		nt = depTiers;
		nt->nextTier = NULL;
	} else {
		tnt= depTiers;
		nt = depTiers;
		while (nt != NULL) {
			if (!strcmp(nt->sp, sp)) {
				fprintf(stderr, "Multiple identical dependent tiers per speaker time %s (%s%s-%s%s):\n",
						sp, beg->timeTag, beg->supplement, end->timeTag, end->supplement);
				fprintf(stderr, "    %s%s\n", sp, line);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
			tnt = nt;
			nt = nt->nextTier;
		}
		if (nt == NULL) {
			tnt->nextTier = NEW(ALLCHATTIERS);
			if (tnt->nextTier == NULL)
				out_of_mem();
			nt = tnt->nextTier;
			nt->nextTier = NULL;
		}
	}

	Paidus_fillNT(nt, beg, end, sp, line);
	return(depTiers);
}

static ALLCHATTIERS *Paidus_addToTiers(TIMESTABLE *beg, TIMESTABLE *end, char *sp, char *line) {
	ALLCHATTIERS *nt, *tnt;

	if (RootTiers == NULL) {
		if ((RootTiers=NEW(ALLCHATTIERS)) == NULL)
			out_of_mem();
		nt = RootTiers;
		nt->nextTier = NULL;
	} else {
		tnt= RootTiers;
		nt = RootTiers;
		while (nt != NULL) {
			if (sp[0] == '%') {
				if (beg == nt->beg) {
					nt->depTiers = Paidus_addDepTiers(nt->depTiers, beg, end, sp, line);
					return(nt);
				} else if (beg != NULL && nt->beg != NULL) {
					if (beg->supplement[0] != EOS || beg->timeValue == -1) {
						if (beg->tagValue < nt->beg->tagValue) {
							nt = Paidus_insertNT(nt, tnt);
							strcpy(templineC1, sp+1);
							strcat(templineC1, line);
							Paidus_fillNT(nt, beg, end, "@Comment:", templineC1);
							return(nt);
						}
					} else if (beg->timeValue < nt->beg->timeValue) {
						nt = Paidus_insertNT(nt, tnt);
						strcpy(templineC1, sp+1);
						strcat(templineC1, line);
						Paidus_fillNT(nt, beg, end, "@Comment:", templineC1);
						return(nt);
					}
				}
			}
			tnt = nt;
			nt = nt->nextTier;
		}
		if (nt == NULL) {
			tnt->nextTier = NEW(ALLCHATTIERS);
			if (tnt->nextTier == NULL)
				out_of_mem();
			nt = tnt->nextTier;
			nt->nextTier = NULL;
			if (sp[0] == '%') {
/*
				fprintf(stderr, "Can't find matching time for: %s (%s%s-%s%s)\n",
						sp, beg->timeTag, beg->supplement, end->timeTag, end->supplement);
				fprintf(stderr, "    %s%s\n", sp, line);
*/
				strcpy(templineC1, sp+1);
				strcat(templineC1, line);
				Paidus_fillNT(nt, beg, end, "@Comment:", templineC1);
				return(nt);
			}
		}
	}

	Paidus_fillNT(nt, beg, end, sp, line);
	return(nt);
}

static void Paidus_makeText(char *line) {
	char *e, c;

	while (*line != EOS) {
		if (!strncmp(line, "&amp;", 5)) {
			*line = '&';
			line++;
			strcpy(line, line+4);
		} else if (!strncmp(line, "&quot;", 6)) {
			*line = '"';
			line++;
			strcpy(line, line+5);
		} else if (!strncmp(line, "&apos;", 6)) {
			*line = '\'';
			line++;
			strcpy(line, line+5);
		} else if (!strncmp(line, "&lt;", 4)) {
			*line = '<';
			line++;
			strcpy(line, line+3);
		} else if (!strncmp(line, "&gt;", 4)) {
			*line = '>';
			line++;
			strcpy(line, line+3);
		} else if (*line == '{' && *(line+1) == '0' && *(line+2) == 'x') {
			for (e=line; *e != EOS && *e != '}'; e++) ;
			if (*e == '}') {
				sscanf(line+1, "%x", &c);
				*line = c;
				line++;
				strcpy(line, e+1);
			} else
				line++;
		} else
			line++;
	}
}

/* Paidus-XML Begin **************************************************************** */
static void convertPaidusTag2Chat(char *chatName, char *tag) {
	ATTRIBS *p;

	uS.remblanks(tag);
	chatName[0] = EOS;
	if (tag[0] == EOS)
		return;
	for (p=attsRoot; p != NULL; p=p->nextTag) {
		if (uS.mStricmp(p->tag, tag) == 0)
			break;
	}
	if (p == NULL) {
		fprintf(stderr, "\n#### Can't match paidus tag \"%s\" to any CHAT declaration in attributes file.\n\n", tag);
		attsRoot = add_each_paidusTag(attsRoot, oldfname, 0, tag, tag);
		strncpy(chatName, tag, SPEAKERLEN-2);
		chatName[SPEAKERLEN-3] = EOS;
		strcat(chatName, "_");
	} else {
		strcpy(chatName, p->chatName);
	}
}

static void cleanupText(char *text, char *name) {
	int  i;
	char isSpaceFount;

	if (!strcmp(name, "event")) {
		for (i=0; text[i] != EOS; i++) {
			if (isSpace(text[i]))
				text[i] = '_';
			else if (text[i] == '.') {
				strcpy(text+i, text+i+1);
				i--;
			}
		}
	} else if (!strcmp(name, "age")) {
		for (i=0; text[i] != EOS; i++) {
			if (text[i] == ',') {
				text[i] = '.';
			}
		}
	} else if (!strcmp(name, "phonetics") || !strcmp(name, "syllable-structure")) {
		isSpaceFount = TRUE;
		for (i=0; text[i] != EOS; i++) {
			if (text[i] == '[') {
				if (!isSpaceFount)
					text[i] = ' ';
				else {
					strcpy(text+i, text+i+1);
					i--;
				}
				isSpaceFount = TRUE;
			} else if (text[i] == ']') {
				if (!isSpaceFount)
					text[i] = ' ';
				else {
					strcpy(text+i, text+i+1);
					i--;
				}
				isSpaceFount = TRUE;
			} else if (isSpace(text[i]))
				isSpaceFount = TRUE;
			else
				isSpaceFount = FALSE;
		}
	} else if (!strcmp(name, "type")) {
		for (i=0; text[i] != EOS; i++) {
			if (!strncmp(text+i, "spontaneous", 11)) {
				uS.shiftright(text+i+11, 1);
				text[i+11] = ' ';
			}
		}
	} else if (!strcmp(name, "language")) {
		for (i=0; text[i] != EOS; i++) {
			if (!strncmp(text+i, "german", 6)) {
				uS.shiftright(text+i+6, 1);
				text[i+6] = ' ';
			}
		}
	}
}

static void getOtherComments(Element *Elem, UTTER *utt) {
	long len;
	Element *data;
	
	len = UTTLINELEN-3;
	utt->line[0] = EOS;
	while (Elem != NULL) {
		if (Elem->cData != NULL) {
			len = UTTLINELEN - strlen(utt->line);
			strncat(utt->line, Elem->cData, len);
			len = UTTLINELEN - strlen(utt->line);
			strncat(utt->line, " ", len);
			utt->line[UTTLINELEN-1] = EOS;
		} else if (Elem->data != NULL) {
			for (data=Elem->data; data != NULL; data=data->next) {
				if (!strcmp(data->name, "CONST")) {
					len = UTTLINELEN - strlen(utt->line);
					strncat(utt->line, data->cData, len);
					len = UTTLINELEN - strlen(utt->line);
					strncat(utt->line, " ", len);
					utt->line[UTTLINELEN-1] = EOS;
				}
			}
		}
		utt->line[UTTLINELEN-1] = EOS;
		Elem = Elem->next;
	}
	uS.remblanks(utt->line);
}

static void getSpeakerTier(Element *Elem, UTTER *utt) {
	long len;
	Element *data;
	Attributes *att;

	len = UTTLINELEN-3;
	utt->line[0] = EOS;
	while (Elem != NULL) {
		if ((!strcmp(Elem->name, "w") || !strcmp(Elem->name, "c")) && Elem->data != NULL) {
			if (Elem->cData != NULL) {
				if (Elem->cData[0] != EOS) {
					len = UTTLINELEN - strlen(utt->line);
					strncat(utt->line, Elem->cData, len);
					len = UTTLINELEN - strlen(utt->line);
					strncat(utt->line, " ", len);
					utt->line[UTTLINELEN-1] = EOS;
				}
			} else if (Elem->data != NULL) {
				for (data=Elem->data; data != NULL; data=data->next) {
					if (!strcmp(data->name, "CONST") && data->cData[0] != EOS) {
						len = UTTLINELEN - strlen(utt->line);
						strncat(utt->line, data->cData, len);
						len = UTTLINELEN - strlen(utt->line);
						strncat(utt->line, " ", len);
						utt->line[UTTLINELEN-1] = EOS;
					}
				}
			}
			utt->line[UTTLINELEN-1] = EOS;
		} else if (!strcmp(Elem->name, "event")) {
			for (att=Elem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "description") && att->value[0] != EOS) {
					len = UTTLINELEN - strlen(utt->line);
					strncat(utt->line, " &=", len);
					len -= 2;
					cleanupText(att->value, Elem->name);
					strncat(utt->line, att->value, len);
					len = UTTLINELEN - strlen(utt->line);
					strncat(utt->line, " ", len);
					utt->line[UTTLINELEN-1] = EOS;
				}
			}
		} else {
			fprintf(stderr, "Unrecognize element1: %s\n", Elem->name);
			freeXML_Elements();
			freePaidusMem();
			cutt_exit(0);
		}
		Elem = Elem->next;
	}
	uS.remblanks(utt->line);
	for (len=strlen(utt->line)-1; len >= 0L; len--) {
		if (uS.IsUtteranceDel(utt->line, len))
			break;
	}
	if (len < 0L)
		strcat(utt->line, " .");
}

static char getPaidusTiers(UTTER *utt) {
	long len;
	char sp[128];
	char timeTag[8];
	char supplement[11], *p;
	ALLCHATTIERS *currentSpeaker;
	TIMESTABLE *beg, *end;
	Element *data;
	Attributes *att;

	beg = NULL;
	end = NULL;
	if (RootElem == NULL)
		return(FALSE);

	currentSpeaker = NULL;
	do {
		if (CurrentElem == NULL && !strcmp(RootElem->name, "pepe") && RootElem->next == NULL)
			CurrentElem = RootElem->data;
		else if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;
		
		if (CurrentElem == NULL) {
			if (stackIndex >= 0) {
				currentSpeaker = NULL;
				CurrentElem = Paidus_stack[stackIndex];
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL)
					return(FALSE);
			} else
				return(FALSE);
		}

		if ((!strcmp(CurrentElem->name, "audio-comment") || !strcmp(CurrentElem->name, "nonverbal-comment")) && stackIndex == -1) {
			currentSpeaker = NULL;
			beg = NULL;
			end = NULL;
			utt->speaker[0] = EOS;
			utt->line[0] = EOS;
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "start") || !strcmp(att->name, "end")) {
					timeTag[0] = EOS;
					supplement[0] = EOS;
					p = strchr(att->value, '.');
					if (p != NULL) {
						strncpy(supplement, p, 15);
						supplement[15] = EOS;
						*p = EOS;
					}
					strncpy(timeTag, att->value, 7);
					timeTag[7] = EOS;
					if (!strcmp(att->name, "start"))
						beg = getTimeTableRef(timeRoot, timeTag, supplement);
					else
						end = getTimeTableRef(timeRoot, timeTag, supplement);
				}
			}
			getOtherComments(CurrentElem->data, utt);
			Paidus_makeText(utt->line);
			convertPaidusTag2Chat(sp, CurrentElem->name);
			len = strlen(sp);
			if (sp[len-1] != ':')
				strcat(sp, ":");					
			Paidus_addToTiers(beg, end, sp, utt->line);
		}
		if (!strcmp(CurrentElem->name, "record") && CurrentElem->data != NULL && stackIndex == -1) {
			currentSpeaker = NULL;
			beg = NULL;
			end = NULL;
			utt->speaker[0] = EOS;
			utt->line[0] = EOS;
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "speaker")) {
					strcpy(utt->speaker, "*");
					strncat(utt->speaker, att->value, 5);
					utt->speaker[7] = EOS;
					uS.uppercasestr(utt->speaker, &dFnt, C_MBF);
					uS.remblanks(utt->speaker);
					len = strlen(utt->speaker);
					if (utt->speaker[len-1] != ':')
						strcat(utt->speaker, ":");					
				} else if (!strcmp(att->name, "start") || !strcmp(att->name, "end")) {
					timeTag[0] = EOS;
					supplement[0] = EOS;
					p = strchr(att->value, '.');
					if (p != NULL) {
						strncpy(supplement, p, 15);
						supplement[15] = EOS;
						*p = EOS;
					}
					strncpy(timeTag, att->value, 7);
					timeTag[7] = EOS;
					if (!strcmp(att->name, "start"))
						beg = getTimeTableRef(timeRoot, timeTag, supplement);
					else
						end = getTimeTableRef(timeRoot, timeTag, supplement);
				}
			}
			if (stackIndex < 29) {
				stackIndex++;
				Paidus_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
				if (CurrentElem == NULL)
					continue;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
		}
		if (stackIndex == 0) {
			if (!strcmp(CurrentElem->name, "orthography")) {
				getSpeakerTier(CurrentElem->data, utt);
				Paidus_makeText(utt->line);
				if (utt->line[0] != EOS)
					currentSpeaker = Paidus_addToTiers(beg, end, utt->speaker, utt->line);
			} else if (!strcmp(CurrentElem->name, "phonetics") || !strcmp(CurrentElem->name, "syllable-structure") ||
					   !strcmp(CurrentElem->name, "type")      || !strcmp(CurrentElem->name, "language")           ||
					   !strcmp(CurrentElem->name, "utterance-number")) {
				len = UTTLINELEN-3;
				utt->line[0] = EOS;
				if (CurrentElem->cData != NULL) {
					len = UTTLINELEN - strlen(utt->line);
					strncat(utt->line, CurrentElem->cData, len);
					utt->line[UTTLINELEN-1] = EOS;
				} else if (CurrentElem->data != NULL) {
					for (data=CurrentElem->data; data != NULL; data=data->next) {
						if (!strcmp(data->name, "CONST")) {
							len = UTTLINELEN - strlen(utt->line);
							strncat(utt->line, data->cData, len);
							utt->line[UTTLINELEN-1] = EOS;
						}
					}
				}
				cleanupText(utt->line, CurrentElem->name);
				uS.remblanks(utt->line);
				Paidus_makeText(utt->line);
				convertPaidusTag2Chat(sp, CurrentElem->name);
				len = strlen(sp);
				if (sp[len-1] != ':')
					strcat(sp, ":");
				if (utt->line[0] != EOS) {
					if (currentSpeaker == NULL)
						Paidus_addToTiers(beg, end, sp, utt->line);
					else
						currentSpeaker->depTiers = Paidus_addDepTiers(currentSpeaker->depTiers, beg, end, sp, utt->line);
				}
			} else {
				fprintf(stderr, "Unrecognize element2: %s\n", CurrentElem->name);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
		}
	} while (1);
	return(FALSE);
}

static void getSpeakerInfo(Element *CurrentElem) {
	long len;
	Element *data1, *data2;
	Attributes *att;

	if (idsCnt >= IDSNUMBER) {
		fprintf(stderr, "Limit of %d speakers allowed has been exceeded\n", IDSNUMBER);
		freeXML_Elements();
		freePaidusMem();
		cutt_exit(0);
	}
	IDsTable[idsCnt].lan[0]  = EOS;
	IDsTable[idsCnt].sp[0]   = EOS;
	IDsTable[idsCnt].sex[0]  = EOS;
	IDsTable[idsCnt].age[0]  = EOS;
	IDsTable[idsCnt].name[0] = EOS;
	IDsTable[idsCnt].whichSpeaker = UNK_TYPE;
	for (att=CurrentElem->atts; att != NULL; att=att->next) {
		if (!strcmp(att->name, "id")) {
			strcpy(IDsTable[idsCnt].sp, "*");
			strncat(IDsTable[idsCnt].sp, att->value, 5);
			IDsTable[idsCnt].sp[7] = EOS;
			uS.uppercasestr(IDsTable[idsCnt].sp, &dFnt, C_MBF);
		}
	}
	for (CurrentElem=CurrentElem->data; CurrentElem != NULL; CurrentElem = CurrentElem->next) {
		if (!strcmp(CurrentElem->name, "abbreviation")) {
			if (CurrentElem->cData != NULL) {
				strncpy(IDsTable[idsCnt].name, CurrentElem->cData, 32);
				IDsTable[idsCnt].name[32] = EOS;
			} else if (CurrentElem->data != NULL) {
				for (data1=CurrentElem->data; data1 != NULL; data1=data1->next) {
					if (!strcmp(data1->name, "CONST")) {
						strncpy(IDsTable[idsCnt].name, data1->cData, 32);
						IDsTable[idsCnt].name[32] = EOS;
					}
				}
			}
			len = strlen(IDsTable[idsCnt].name);
			if (!uS.mStrnicmp(oldfname, IDsTable[idsCnt].name, len))
				IDsTable[idsCnt].whichSpeaker = CHI_TYPE;
			else if (!uS.mStrnicmp("Mutter", IDsTable[idsCnt].name, 6))
				IDsTable[idsCnt].whichSpeaker = MOT_TYPE;
		}
		if (!strcmp(CurrentElem->name, "sex")) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "value")) {
					if (att->value[0] == 'f' || att->value[0] == 'F')
						strcpy(IDsTable[idsCnt].sex, "female");
					else if (att->value[0] == 'm' || att->value[0] == 'M')
						strcpy(IDsTable[idsCnt].sex, "male");
					else {
						strncpy(IDsTable[idsCnt].sex, att->value, 10);
						IDsTable[idsCnt].sp[5] = EOS;
					}
				}
			}
		}
		if (!strcmp(CurrentElem->name, "languages-used")) {
			data1 = CurrentElem->data;
			len = 20;
			for (; data1 != NULL; data1=data1->next) {
				if (!strcmp(data1->name, "language")) {
					for (att=data1->atts; att != NULL; att=att->next) {
						if (!strcmp(att->name, "lang")) {
							if (len < 20) {
								strcat(IDsTable[idsCnt].lan, ",");
								len--;
							}
							strncat(IDsTable[idsCnt].lan, att->value, len);
							IDsTable[idsCnt].lan[20] = EOS;
						}
					}
				}
			}
		}
		if (!strcmp(CurrentElem->name, "ud-speaker-information")) {
			data1 = CurrentElem->data;
			for (; data1 != NULL; data1=data1->next) {
				if (!strcmp(data1->name, "ud-information")) {
					for (att=data1->atts; att != NULL; att=att->next) {
						if (!strcmp(att->name, "attribute-name") && !strcmp(att->value, "Age")) {
							if (data1->cData != NULL) {
								strncpy(IDsTable[idsCnt].age, data1->cData, 15);
								IDsTable[idsCnt].age[20] = EOS;
							} else if (data1->data != NULL) {
								for (data2=data1->data; data2 != NULL; data2=data2->next) {
									if (!strcmp(data2->name, "CONST")) {
										strncpy(IDsTable[idsCnt].age, data2->cData, 15);
										IDsTable[idsCnt].age[20] = EOS;
									}
								}
							}
							cleanupText(IDsTable[idsCnt].age, "age");
						}
					}
				}
			}
		}
	}
	idsCnt++;
}

static char getSpeakerPaidusHeader(char *mediaFName, char *projectName) {
	Element *data;
	Attributes *att;

	if (RootElem == NULL)
		return(FALSE);
	
	do {
		if (CurrentElem == NULL && !strcmp(RootElem->name, "pepe") && RootElem->next == NULL)
			CurrentElem = RootElem->data;
		else if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;
		
		if (CurrentElem == NULL) {
			if (stackIndex >= 0) {
				CurrentElem = Paidus_stack[stackIndex];
				stackIndex--;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL)
					return(FALSE);
			} else
				return(FALSE);
		}

		if (!strcmp(CurrentElem->name, "common-timeline") && stackIndex == -1) {
			return(FALSE);
		}
		if (!strcmp(CurrentElem->name, "head") && CurrentElem->data != NULL && stackIndex == -1) {
			if (stackIndex < 29) {
				stackIndex++;
				Paidus_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
				if (CurrentElem == NULL)
					continue;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "meta-information") && stackIndex == 0) {
			if (stackIndex < 29) {
				stackIndex++;
				Paidus_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
				if (CurrentElem == NULL)
					continue;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "speakertable") && stackIndex == 0) {
			if (stackIndex < 29) {
				stackIndex++;
				Paidus_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
				if (CurrentElem == NULL)
					continue;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "project-name") && stackIndex == 1) {
			if (CurrentElem->cData != NULL) {
				strncpy(projectName, CurrentElem->cData, 128);
				projectName[128-1] = EOS;
			} else if (CurrentElem->data != NULL) {
				for (data=CurrentElem->data; data != NULL; data=data->next) {
					if (!strcmp(data->name, "CONST")) {
						strncpy(projectName, data->cData, 128);
						projectName[128-1] = EOS;
					}
				}
			}
		}
		if (!strcmp(CurrentElem->name, "referenced-file") && stackIndex == 1) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "url")) {
					strncpy(mediaFName, att->value, FILENAME_MAX);
					mediaFName[FILENAME_MAX] = EOS;
				}
			}
		}
		if (!strcmp(CurrentElem->name, "speaker") && stackIndex == 1) {
			getSpeakerInfo(CurrentElem);
		}
	} while (1);
	return(FALSE);
}

static char getOtherPaidusHeader(UTTER *utt) {
	long len;
	Element *data;
	Attributes *att;
	
	if (RootElem == NULL)
		return(FALSE);
	
	do {
		if (CurrentElem == NULL && !strcmp(RootElem->name, "pepe") && RootElem->next == NULL)
			CurrentElem = RootElem->data;
		else if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;
		
		if (CurrentElem == NULL) {
			if (stackIndex >= 0) {
				CurrentElem = Paidus_stack[stackIndex];
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL)
					return(FALSE);
			} else
				return(FALSE);
		}
		
		if (!strcmp(CurrentElem->name, "common-timeline") && stackIndex == -1) {
			return(FALSE);
		}
		if (!strcmp(CurrentElem->name, "head") && CurrentElem->data != NULL && stackIndex == -1) {
			if (stackIndex < 29) {
				stackIndex++;
				Paidus_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
				if (CurrentElem == NULL)
					continue;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "meta-information") && stackIndex == 0) {
			if (stackIndex < 29) {
				stackIndex++;
				Paidus_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
				if (CurrentElem == NULL)
					continue;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "ud-meta-information") && stackIndex == 1) {
			if (stackIndex < 29) {
				stackIndex++;
				Paidus_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
				if (CurrentElem == NULL)
					continue;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				freePaidusMem();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "comment") && stackIndex == 1) {
			strcpy(utt->speaker, "@Comment");
			len = UTTLINELEN-3;
			utt->line[0] = EOS;
			if (CurrentElem->cData != NULL) {
				strncat(utt->line, CurrentElem->cData, len);
				utt->line[UTTLINELEN-1] = EOS;
			} else if (CurrentElem->data != NULL) {
				for (data=CurrentElem->data; data != NULL; data=data->next) {
					if (!strcmp(data->name, "CONST")) {
						len = UTTLINELEN - strlen(utt->line);
						strncat(utt->line, data->cData, len);
						utt->line[UTTLINELEN-1] = EOS;
					}
				}
			}
			uS.remblanks(utt->line);
			return(TRUE);
		}
		if (!strcmp(CurrentElem->name, "transcription-convention") && stackIndex == 1) {
			strcpy(utt->speaker, "@Comment");
			strcpy(utt->line, "transcription-convention=");
			len = UTTLINELEN - 3 - strlen(utt->line);
			if (CurrentElem->cData != NULL) {
				strncat(utt->line, CurrentElem->cData, len);
				utt->line[UTTLINELEN-1] = EOS;
			} else if (CurrentElem->data != NULL) {
				for (data=CurrentElem->data; data != NULL; data=data->next) {
					if (!strcmp(data->name, "CONST")) {
						len = UTTLINELEN - strlen(utt->line);
						strncat(utt->line, data->cData, len);
						utt->line[UTTLINELEN-1] = EOS;
					}
				}
			}
			uS.remblanks(utt->line);
			return(TRUE);
		}
		if (!strcmp(CurrentElem->name, "ud-information") && stackIndex == 2) {
			utt->speaker[0] = EOS;
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "attribute-name")) {
					convertPaidusTag2Chat(utt->speaker, att->value);
				}
			}
			len = UTTLINELEN-3;
			utt->line[0] = EOS;
			if (CurrentElem->cData != NULL) {
				strncat(utt->line, CurrentElem->cData, len);
				utt->line[UTTLINELEN-1] = EOS;
			} else if (CurrentElem->data != NULL) {
				for (data=CurrentElem->data; data != NULL; data=data->next) {
					if (!strcmp(data->name, "CONST")) {
						len = UTTLINELEN - strlen(utt->line);
						strncat(utt->line, data->cData, len);
						utt->line[UTTLINELEN-1] = EOS;
					}
				}
			}
			if (uS.partwcmp(utt->speaker, "@Age of"))
				cleanupText(utt->line, "age");
			uS.remblanks(utt->line);
			if (utt->speaker[0] != EOS && utt->line[0] != EOS)
				return(TRUE);
		}
	} while (1);
	return(FALSE);
}

static void getTimeTable(void) {
	char timeTag[8];
	long timeValue;
	double timeDB;
	char supplement[11], *p;
	char isFoundTime;
	Attributes *att;
	
	if (RootElem == NULL)
		return;

	isFoundTime = FALSE;
	do {
		if (CurrentElem == NULL && !strcmp(RootElem->name, "pepe") && RootElem->next == NULL)
			CurrentElem = RootElem->data;
		else if (CurrentElem != NULL) {
			if (stackIndex == -1 && !strcmp(CurrentElem->name, "common-timeline")) {
				if (stackIndex < 29) {
					stackIndex++;
					Paidus_stack[stackIndex] = CurrentElem;
					CurrentElem = CurrentElem->data;
					isFoundTime = TRUE;
				} else {
					fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
					freeXML_Elements();
					freePaidusMem();
					cutt_exit(0);
				}
			} else
				CurrentElem = CurrentElem->next;
		}
		if (CurrentElem == NULL) {
			if (stackIndex >= 0) {
				CurrentElem = Paidus_stack[stackIndex];
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL)
					return;
			} else
				return;
		}
		
		if (isFoundTime && stackIndex == -1) {
			return;
		}
		if (!strcmp(CurrentElem->name, "tli") && stackIndex == 0) {
			timeTag[0] = EOS;
			supplement[0] = EOS;
			timeValue = -1L;
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "id")) {
					p = strchr(att->value, '.');
					if (p != NULL) {
						strncpy(supplement, p, 15);
						supplement[15] = EOS;
						*p = EOS;
					}
					strncpy(timeTag, att->value, 7);
					timeTag[7] = EOS;
				}
				if (!strcmp(att->name, "time")) {
					timeDB = atof(att->value);
					timeDB = timeDB * 1000.000000;
					timeValue = (long)timeDB;
				}
			}
			timeRoot = addToTimeTable(timeRoot, timeTag, supplement, timeValue);
		}
	} while (1);
}
/* Paidus-XML End ****************************************************************** */

static void printHeaders(char *projectName) {
	int i, j;
	char sp[SPEAKERLEN], role[128];

	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	if (idsCnt > 0)
		fprintf(fpout, "@Languages:\t%s\n", IDsTable[0].lan);
	else
		fprintf(fpout, "@Languages:\tUNK\n");
	templineC1[0] = EOS;
	for (i=0; i < idsCnt; i++) {
		if (IDsTable[i].sp[0] == '*') {
			strcat(templineC1, ", ");
			if (IDsTable[i].whichSpeaker == CHI_TYPE)
				strcpy(sp, "CHI");
			else if (IDsTable[i].whichSpeaker == MOT_TYPE)
				strcpy(sp, "MOT");
			else
				strcpy(sp, IDsTable[i].sp+1);
			for (j=strlen(sp)-1; j >= 0 && (isSpace(sp[j]) || sp[j] == ':'); j--) ;
			sp[++j] = EOS;
			strcat(templineC1, sp);
			strcat(templineC1, " ");
			if (IDsTable[i].whichSpeaker != CHI_TYPE && IDsTable[i].whichSpeaker != MOT_TYPE) {
				strcat(templineC1, IDsTable[i].name);
				if (IDsTable[i].name[0] != EOS)
					strcat(templineC1, " ");
			}
			if (IDsTable[i].whichSpeaker == CHI_TYPE)
				strcat(templineC1, "Target_Child");
			else if (IDsTable[i].whichSpeaker == MOT_TYPE)
				strcat(templineC1, "Mother");
		}
	}
	for (j=0; templineC1[j] == ',' || isSpace(templineC1[j]); j++) ;
	printout("@Participants:", templineC1+j, NULL, NULL, TRUE);
	for (i=0; i < idsCnt; i++) {
		if (IDsTable[i].sp[0] == '*') {
			if (IDsTable[i].whichSpeaker == CHI_TYPE)
				strcpy(sp, "CHI");
			else if (IDsTable[i].whichSpeaker == MOT_TYPE)
				strcpy(sp, "MOT");
			else
				strcpy(sp, IDsTable[i].sp+1);
			for (j=strlen(sp)-1; j >= 0 && (isSpace(sp[j]) || sp[j] == ':'); j--) ;
			sp[++j] = EOS;
			if (IDsTable[i].whichSpeaker == CHI_TYPE)
				strcpy(role, "Target_Child");
			else if (IDsTable[i].whichSpeaker == MOT_TYPE)
				strcpy(role, "Mother");
			else
				role[0] = EOS;
			fprintf(fpout, "@ID:\t%s|%s|%s|%s|%s|||%s||\n", IDsTable[i].lan, projectName, sp, IDsTable[i].age, IDsTable[i].sex, role);
		}
	}
}

static void Paidus_printOutTiers(ALLCHATTIERS *p, TIMESTABLE *beg, TIMESTABLE *end) {
	int i;

	while (p != NULL) {
		strcpy(templineC1, p->line);
		if (p->sp[0] != '%') {
			if (p->beg != NULL && p->end != NULL) {
				if (p->beg->timeValue == -1 && p->end->timeValue == -1)
					sprintf(templineC3, " [+ %s%s_%s%s]", p->beg->timeTag, p->beg->supplement, p->end->timeTag, p->end->supplement);
				else if (p->beg->timeValue == -1)
					sprintf(templineC3, " [+ %s%s_%ld]", p->beg->timeTag, p->beg->supplement, p->end->timeValue);
				else if (p->end->timeValue == -1)
					sprintf(templineC3, " [+ %ld_%s%s]", p->beg->timeValue, p->end->timeTag, p->end->supplement);
				else
					sprintf(templineC3, " %c%ld_%ld%c", HIDEN_C, p->beg->timeValue, p->end->timeValue, HIDEN_C);
				strcat(templineC1, templineC3);
			}
		} else if (beg != NULL && end != NULL) {
			if (beg != p->beg || end != p->end) {
				if (p->beg->timeValue == -1 && p->end->timeValue == -1)
					sprintf(templineC3, " [+ %s%s_%s%s]", p->beg->timeTag, p->beg->supplement, p->end->timeTag, p->end->supplement);
				else if (p->beg->timeValue == -1)
					sprintf(templineC3, " [+ %s%s_%ld]", p->beg->timeTag, p->beg->supplement, p->end->timeValue);
				else if (p->end->timeValue == -1)
					sprintf(templineC3, " [+ %ld_%s%s]", p->beg->timeValue, p->end->timeTag, p->end->supplement);
				else
					sprintf(templineC3, " %c%ld_%ld%c", HIDEN_C, p->beg->timeValue, p->end->timeValue, HIDEN_C);
				strcat(templineC1, templineC3);
			}
		}
		for (i=0; i < idsCnt; i++) {
			if (uS.partcmp(p->sp, IDsTable[i].sp, FALSE, FALSE)) 
				break;
		}
		if (i < idsCnt) {
			if (IDsTable[i].whichSpeaker == CHI_TYPE)
				strcpy(templineC3, "*CHI:");
			else if (IDsTable[i].whichSpeaker == MOT_TYPE)
				strcpy(templineC3, "*MOT:");
			else
				strcpy(templineC3, p->sp);
		} else
			strcpy(templineC3, p->sp);
		printout(templineC3, templineC1, NULL, NULL, TRUE);
		if (p->depTiers != NULL)
			Paidus_printOutTiers(p->depTiers, p->beg, p->end);
		p = p->nextTier;
	}
}

void call() {		/* this function is self-explanatory */
	char *p, projectName[128];
	long len;
	long ln = 0L, tln = 0L;

	if (attsRoot == NULL) {
		rd_PaidusAtts_f("paidus_attribs.cut");
		if (attsRoot == NULL) {
			fprintf(stderr,"Please specify paidus_attribs.cut file with +d option.\n");
			cutt_exit(0);
		}
	}

	projectName[0] = EOS;
	mediaFName[0] = EOS;
	BuildXMLTree(fpin);

	while (getSpeakerPaidusHeader(mediaFName, projectName))
		;
	printHeaders(projectName);
	p = strrchr(mediaFName, '/');
	if (p != NULL)
		strcpy(mediaFName, p+1);
	p = strrchr(mediaFName, '.');
	if (p != NULL)
		*p = EOS;
	if (p != NULL && (uS.mStricmp(p+1, "mp3") == 0 || uS.mStricmp(p+1, "wav") == 0 || uS.mStricmp(p+1, "aif") == 0))
		fprintf(fpout, "%s\t%s, audio\n", MEDIAHEADER, mediaFName);
	else
		fprintf(fpout, "%s\t%s, video\n", MEDIAHEADER, mediaFName);
	
	ResetXMLTree();
	while (getOtherPaidusHeader(utterance)) {
		uS.remblanks(utterance->speaker);
		len = strlen(utterance->speaker) - 1;
		if (utterance->line[0] != EOS) {
			if (utterance->speaker[len] != ':')
				strcat(utterance->speaker, ":");
		} else {
			if (utterance->speaker[len] == ':')
				utterance->speaker[len] = EOS;
		}
		Paidus_makeText(utterance->line);
		printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
	}
	getTimeTable();

	ResetXMLTree();
	while (getPaidusTiers(utterance)) {
		if (ln > tln) {
			tln = ln + 200;
			fprintf(stderr,"\r%ld ",ln);
		}
		ln++;
	}
	fprintf(stderr, "\r	  \r");
	Paidus_printOutTiers(RootTiers, NULL, NULL);
 
	fprintf(fpout, "@End\n");
	freeXML_Elements();
	timeRoot = freeTimeTable(timeRoot);
	RootTiers = freeTiers(RootTiers);
}
