/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main chat2anvil_main
#define call chat2anvil_call
#define getflag chat2anvil_getflag
#define init chat2anvil_init
#define usage chat2anvil_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#define NAMESLEN 128
#define REF_CODE     "[r "
#define REF_CODE_LEN 3

struct AnvilAtribsRec {
	char tag[NAMESLEN];
	char chatName[NAMESLEN];
	char depOn[NAMESLEN];
	struct AnvilAtribsRec *nextTag;
} ;
typedef struct AnvilAtribsRec ATTRIBS;

#define ATTS struct attS
struct attS {
	char isComment;
	char *attName;
	char *st;
	ATTS *nextAtt;
} ;

#define ELS struct elS
struct elS {
	long xmlIndex;
	long chatRefID;
	double beg;
	double end;
	ATTS *elAtts;
	ELS *nextEl;
} ;

#define ALLTIERS struct AllTiers
struct AllTiers {
	char tierName[NAMESLEN];
	long lastTime;
	ELS *els;
	ALLTIERS *nextTier;
} ;

extern struct tier *defheadtier;
extern char OverWriteFile;

static char C2A_ftime;
static ATTRIBS *attsRoot;
static ALLTIERS *RootTiers;
static FNType mediaExt[10];

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	int i;

	if (f) {
		OverWriteFile = TRUE;
		AddCEXExtension = ".anvil";
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		attsRoot = NULL;
		RootTiers = NULL;
		mediaExt[0] = EOS;
		C2A_ftime = TRUE;
		FileName1[0] = EOS;
	} else {
		if (mediaExt[0] == EOS) {
			fprintf(stderr,"Please use +e option to specify media file extension.\n    (For example: +e.mov or +e.mpg or +e.wav)\n");
			cutt_exit(0);
		}
		if (C2A_ftime) {
			C2A_ftime = FALSE;
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '[' || GlobalPunctuation[i] == ']' || GlobalPunctuation[i] == ',') 
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}
		}
	}
}

void usage() {
	printf("convert CHAT files to Anvil XML files\n");
	printf("Usage: chat2anvil [dF eS %s] filename(s)\n", mainflgs());
    puts("+dF: specify attribs/tags dependencies file F.");
	puts("+eS: Specify media file name extension.");
	mainusage(TRUE);
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

static void freeElAtts(ATTS *p) {
	ATTS *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextAtt;
		if (t->attName != NULL)
			free(t->attName);
		if (t->st != NULL)
			free(t->st);
		free(t);
	}
}

static void freeEls(ELS *p) {
	ELS *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextEl;
		freeElAtts(t->elAtts);
		free(t);
	}
}

static ALLTIERS *freeTiers(ALLTIERS *p) {
	ALLTIERS *t;
	
	while (p != NULL) {
		t = p;
		p = p->nextTier;
		freeEls(t->els);
		free(t);
	}
	return(NULL);
}

static void freeChat2AnvilMem(void) {
	attsRoot = freeAttribs(attsRoot);
	RootTiers = freeTiers(RootTiers);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CHAT2ANVIL;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
	attsRoot = freeAttribs(attsRoot);
}
		
static ATTRIBS *add_each_anvilTag(ATTRIBS *root, const char *fname, long ln, const char *tag, const char *chatName, const char *depOn) {
	long len;
	ATTRIBS *p, *tp;

	if (tag[0] == EOS)
		return(root);

	if (root == NULL) {
		if ((p=NEW(ATTRIBS)) == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		root = p;
	} else {
		for (p=root; p->nextTag != NULL && uS.mStricmp(p->tag, tag); p=p->nextTag) ;
		if (uS.mStricmp(p->tag, tag) == 0)
			return(root);
		
		if ((p->nextTag=NEW(ATTRIBS)) == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		p = p->nextTag;
	}

	p->nextTag = NULL;
	if ((strlen(tag) >= NAMESLEN) || (strlen(depOn) >= NAMESLEN) || (strlen(chatName) >= NAMESLEN)) {
		freeChat2AnvilMem();
		fprintf(stderr,"*** File \"%s\"", fname);
		if (ln != 0)
			fprintf(stderr,": line %ld.\n", ln);
		fprintf(stderr, "Tag(s) too long. Longer than %d characters\n", NAMESLEN);
		cutt_exit(0);
	}
	strcpy(p->tag, tag);
	strcpy(p->chatName, chatName);
	if (p->chatName[0] == '@' || p->chatName[0] == '*' || p->chatName[0] == '%') {
		len = strlen(p->chatName) - 1;
		if (p->chatName[len] != ':')
			strcat(p->chatName, ":");
	}
	if (depOn[0] != EOS && chatName[0] != '*') {
		for (tp=root; tp != NULL; tp=tp->nextTag) {
			if (uS.mStricmp(tp->tag, depOn) == 0 || uS.partcmp(tp->chatName, depOn, FALSE, FALSE))
				break;
		}
		if (tp == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"*** File \"%s\"", fname);
			if (ln != 0)
				fprintf(stderr,": line %ld.\n", ln);
			fprintf(stderr, "\n  Can't match anvil tag \"%s\" to any speaker declaration in attributes file.\n", depOn);
			cutt_exit(0);
		}
		strcpy(p->depOn, tp->chatName);
	} else
		strcpy(p->depOn, depOn);

	return(root);
}

static void rd_AnvilAtts_f(const char *fname, char isFatalError) {
	int  cnt;
	const char *tag, *depOn, *chatName;
	char isQF;
	long i, j, ln;
	FILE *fp;
	
	if (*fname == EOS) {
		fprintf(stderr,	"No dep. tags file specified.\n");
		cutt_exit(0);
	}
	if ((fp=OpenGenLib(fname,"r",TRUE,FALSE,FileName1)) == NULL) {
		fprintf(stderr, "\n    Warning: Not using any attribs file: \"%s\", \"%s\"\n", fname, FileName1);
		if (isFatalError)
			cutt_exit(0);
		else {
//			fprintf(stderr, "    Please specify attribs file with +d option.\n\n");
			return;
		}
	}
	ln = 0L;
	while (fgets_cr(templineC, 255, fp)) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
			continue;
		ln++;
		if (templineC[0] == '#')
			continue;
		uS.remblanks(templineC);
		if (templineC[0] == EOS)
			continue;
		i = 0;
		cnt = 0;
		tag = "";
		chatName = "";
		depOn = "";
		while (1) {
			isQF = 0;
			for (; isSpace(templineC[i]); i++) ;
			if (templineC[i] == '"' || templineC[i] == '\'') {
				isQF = templineC[i];
				i++;
			}
			for (j=i; (!isSpace(templineC[j]) || isQF) && templineC[j] != EOS; j++) {
				if (templineC[j] == isQF) {
					isQF = 0;
					strcpy(templineC+j, templineC+j+1);
					j--;
				}
			}
			if (cnt == 0)
				tag = templineC+i;
			else if (cnt == 1)
				chatName = templineC+i;
			else if (cnt == 2)
				depOn = templineC+i;
			if (templineC[j] == EOS)
				break;
			templineC[j] = EOS;
			i = j + 1;
			cnt++;
		}
		if (tag[0] == EOS || chatName[0] == EOS) {
			freeChat2AnvilMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
			if (tag[0] == EOS)
				fprintf(stderr, "Missing Anvil tag\n");
			else if (chatName[0] == EOS)
				fprintf(stderr, "Missing Chat tier name for \"%s\"\n", tag);
			cutt_exit(0);
		}
		if (depOn[0] == EOS && chatName[0] == '%') {
			freeChat2AnvilMem();
			fprintf(stderr,"*** File \"%s\": line %ld.\n", fname, ln);
			fprintf(stderr, "Missing \"Associated with tag\" for \"%s\"\n", tag);
			cutt_exit(0);
		}
		attsRoot = add_each_anvilTag(attsRoot, fname, ln, tag, chatName, depOn);
	}
	fclose(fp);
}

void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'd':
			if (*f) {
				rd_AnvilAtts_f(getfarg(f,f1,i), TRUE);
			} else {
				fprintf(stderr,"Missing argument to option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 'e':
			if (f[0] != '.')
				uS.str2FNType(mediaExt, 0L, ".");
			else
				mediaExt[0] = EOS;
			uS.str2FNType(mediaExt, strlen(mediaExt), f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char *extractRefNum(char *chatRefSt, long *chatRefID, char *st) {
	char *es, t;
	
	for (; *st == '_'; st++) ;
	if (*st == EOS)
		return(NULL);
	
	for (es=st; *es != '_' && *es != EOS; es++) ;
	if (st[0] == 'r' && st[1] == '-')
		st += 2;
	if (*es != EOS) {
		t = *es;
		*es = EOS;
		*chatRefID = atol(st);
		strcpy(chatRefSt, st);
		*es = t;
	} else {
		*chatRefID = atol(st);
		strcpy(chatRefSt, st);
	}
	return(es);
}

static char findLinkInfo(ALLTIERS *trs, long chatRefID, char *refTrack, long *xmlIndex) {
	ELS *el;
	
	for (; trs != NULL; trs=trs->nextTier) {
		for (el=trs->els; el != NULL; el=el->nextEl) {
			if (el->chatRefID == chatRefID) {
				strcpy(refTrack, trs->tierName);
				*xmlIndex = el->xmlIndex;
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

static char *changeValueLinks(char *oldSt) {
	char *st = NULL, refTrack[NAMESLEN];
	long chatRefID, xmlIndex, len;

	templineC1[0] = EOS;
	while ((oldSt=extractRefNum(templineC2, &chatRefID, oldSt)) != NULL) {
		if (chatRefID > 0L) {
			len = strlen(templineC1);
			if (findLinkInfo(RootTiers, chatRefID, refTrack, &xmlIndex)) {
				if (len == 0L)
					sprintf(templineC1+len, "<value-link ref-track=\"%s\" ref-index=\"%ld\" />", refTrack, xmlIndex);
				else
					sprintf(templineC1+len, "\n         <value-link ref-track=\"%s\" ref-index=\"%ld\" />", refTrack, xmlIndex);
			} else {
				if (len == 0L)
					sprintf(templineC1+len, "%s", templineC2);
				else
					sprintf(templineC1+len, "\n         %s", templineC2);
			}
		}
	}
	if (templineC1[0] != EOS) {
		st = (char *)malloc(strlen(templineC1)+1);
		if (st == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		strcpy(st, templineC1);
		free(oldSt);
	}
	return(st);
}

static void addValueLinks(ALLTIERS *trs) {
	ELS *el;
	ATTS *elAtts;
	
	for (; trs != NULL; trs=trs->nextTier) {
		for (el=trs->els; el != NULL; el=el->nextEl) {
			for (elAtts=el->elAtts; elAtts != NULL; elAtts=elAtts->nextAtt) {
				if (elAtts->st[0] == 'r' && elAtts->st[1] == '-' && isdigit(elAtts->st[2]))
					elAtts->st = changeValueLinks(elAtts->st);
			}
		}
	}
}

static void PrintTiers(ALLTIERS *trs) {
	ELS *el;
	ATTS *elAtts;
	
	fprintf(fpout, "\n");
	fprintf(fpout, "  <body>\n");
	for (; trs != NULL; trs=trs->nextTier) {
		fprintf(fpout, "    <track name=\"%s\" type=\"primary\">\n", trs->tierName);
		for (el=trs->els; el != NULL; el=el->nextEl) {
			fprintf(fpout, "      <el index=\"%ld\" start=\"%lf\" end=\"%lf\">\n", el->xmlIndex, el->beg, el->end);
			for (elAtts=el->elAtts; elAtts != NULL; elAtts=elAtts->nextAtt) {
				if (elAtts->isComment) {
					fprintf(fpout, "       <comment>\n");
					fprintf(fpout, "         %s\n", elAtts->st);
					fprintf(fpout, "       </comment>\n");
				} else {
					if (elAtts->attName == NULL)
						fprintf(fpout, "       <attribute name=\"%s\">\n", "token");
					else
						fprintf(fpout, "       <attribute name=\"%s\">\n", elAtts->attName);
					fprintf(fpout, "         %s\n", elAtts->st);
					fprintf(fpout, "       </attribute>\n");
				}
			}
			fprintf(fpout, "     </el>\n");
		}
		fprintf(fpout, "    </track>\n");
	}
	fprintf(fpout, "  </body>\n");
	fprintf(fpout, "\n");
}

static void convertChatTiers2AnvilTag(char *tag, const char *chatName, const char *depOn) {
	char *s;
	ATTRIBS *p;

	for (p=attsRoot; p != NULL; p=p->nextTag) {
		if (!isSpeaker(p->chatName[0]))
			;
		else if (depOn[0] == EOS && uS.partcmp(chatName, p->chatName, FALSE, FALSE))
			break;
		else if (uS.partcmp(chatName, p->chatName, FALSE, FALSE) && uS.partcmp(depOn, p->depOn, FALSE, FALSE))
			break;
	}
	if (p == NULL) {
		if (!isSpeaker(chatName[0]) && attsRoot != NULL) {
			freeChat2AnvilMem();
			fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, lineno);
			fprintf(stderr, "Can't match CHAT tag \"%s\" to any CHAT declaration in attributes file.\n", chatName);
			fprintf(stderr, "To ignore this CHAT tag, add next line to file: %s\n", FileName1);
			fprintf(stderr, "IGNORE_THIS_TAG		%s\n", chatName);
			cutt_exit(0);
		}

		if (chatName[0] == '@')
			strcpy(tag, chatName);
		else if (chatName[0] == '*' || depOn[0] == EOS) {
			strcpy(tag, chatName);
			s = strrchr(tag, ':');
			if (s != NULL)
				*s = EOS;
			strcat(tag, ".words");
		} else {
			strcpy(tag, depOn);
			s = strrchr(tag, ':');
			if (s != NULL)
				*s = EOS;
			strcat(tag, ".");
			strcat(tag, chatName);
			s = strrchr(tag, ':');
			if (s != NULL)
				*s = EOS;
		}
	} else {
		strcpy(tag, p->tag);
	}
}

static char *isConvertChatCodes2AnvilAtts(char *name, char *st) {
	char *s;
	ATTRIBS *p;

	for (p=attsRoot; p != NULL; p=p->nextTag) {
		if (isSpeaker(p->chatName[0]))
			;
		else if (uS.partcmp(st, p->chatName, FALSE, FALSE))
			break;
	}
	if (p == NULL) {
		s = strchr(st, '_');
		if (st[0] != '$' || s == NULL || strncmp(st, "$COM_", 5) == 0) {
			name[0] = EOS;
			return(NULL);
		} else {
			*s = EOS;
			strncpy(name, st+1, NAMESLEN-1);
			name[NAMESLEN-1] = EOS;
			fprintf(stderr, "\n  Can't match CHAT tag \"%s\" to any CHAT declaration in attributes file.\n", st);
			*s = '_';
			return(s+1);
		}
	} else {
		strcpy(name, p->tag);
		return(st+strlen(p->chatName));
	}
}

static ATTS *add2Atts(ATTS *root, char isFirst, const char *name, char *st, char isComment) {
	ATTS *t;

	if (root == NULL || isFirst) {
		if ((t=NEW(ATTS)) == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		if (!isFirst)
			root = t;
	} else {
		for (t=root; t->nextAtt != NULL; t=t->nextAtt) ;
		if ((t->nextAtt=NEW(ATTS)) == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		t = t->nextAtt;
	}
	if (!isFirst)
		t->nextAtt = NULL;
	else
		t->nextAtt = root;
	t->attName    = NULL;
	t->st         = NULL;
	if (name != NULL) {
		t->attName = (char *)malloc(strlen(name)+1);
		if (t->attName == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		strcpy(t->attName, name);
	}
	t->isComment = isComment;
	t->st = (char *)malloc(strlen(st)+1);
	if (t->st == NULL) {
		freeChat2AnvilMem();
		fprintf(stderr,"Error: out of memory\n");
		cutt_exit(0);
	}
	strcpy(t->st, st);
	if (isFirst)
		root = t;
	return(root);
}

static ATTS *add2String(ATTS *root, char *st, long *chatRefID) {
	char *s, *es, t, name[NAMESLEN], sqb;

	templineC1[0] = EOS;
	while (*st != EOS) {
		for (; isSpace(*st) || uS.isskip(st, 0, &dFnt, MBF) || *st == '\n'; st++) ;
		if (*st == EOS)
			break;
		if (*st == '[')
			sqb = TRUE;
		else
			sqb = FALSE;
		for (es=st; ((!isSpace(*es) && !uS.isskip(st, 0, &dFnt, MBF) && *es != '\n') || sqb) && *es != EOS; es++) {
			if (*es == ']')
				sqb = FALSE;
		}
		t = *es;
		*es = EOS;
		if (!strncmp(st, REF_CODE, REF_CODE_LEN) && isdigit(st[REF_CODE_LEN])) {
			*chatRefID = atol(st+3);
		} else if ((s=isConvertChatCodes2AnvilAtts(name, st)) != NULL) {
			root = add2Atts(root, FALSE, name, s, FALSE);
		} else if (strncmp(st, "$COM_", 5) == 0) {
			st = st + 5;
			for (s=st; *s != EOS; s++) {
				if (*s == '_')
					*s = ' ';
			}
			root = add2Atts(root, FALSE, NULL, st, TRUE);
		} else {
			strcat(templineC1, st);
			strcat(templineC1, " ");
		}
		*es = t;
		st = es;
	}
	uS.remblanks(templineC1);
	if (templineC1[0] != EOS) {
		if (utterance->speaker[0] == '%')
			root = add2Atts(root, TRUE, "CHATtoken", templineC1, FALSE);
		else
			root = add2Atts(root, TRUE, NULL, templineC1, FALSE);
	}
	return(root);
}

static ELS *addAnnotation(ELS *root, long Beg, long End, char *st) {
	ELS *t;
	long xmlIndex;

	if (root == NULL) {
		if ((root=NEW(ELS)) == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		t = root;
		xmlIndex = 0L;
	} else {
		xmlIndex = 0L;
		for (t=root; 1; t=t->nextEl) {
			if (t->beg == Beg && t->end == End) {
				t->elAtts = add2String(t->elAtts, st, &t->xmlIndex);
				return(root);
			}
			if (t->nextEl == NULL)
				break;
			xmlIndex++;
		}
		if ((t->nextEl=NEW(ELS)) == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		xmlIndex++;
		t = t->nextEl;
	}
	t->nextEl    = NULL;
	t->xmlIndex  = xmlIndex;
	t->chatRefID = 0L;
	t->beg       = Beg;
	t->beg       = t->beg / 1000.0000;
	t->end       = End;
	t->end       = t->end / 1000.0000;
	t->elAtts    = NULL;
	t->elAtts = add2String(NULL, st, &t->chatRefID);
	return(root);
}

static void addTierAnnotation(const char *name, long *BegO, long *EndO, char *st, char isChangeTime, char isSpecFound) {
	int  len;
	char isFoundData;
	long Beg, End;
	ALLTIERS *t;

	if (!isSpecFound)
		isFoundData = TRUE;
	else
		isFoundData = FALSE;
	for (len=0; st[len] != EOS; len++) {
		if (st[len] == '\t' || st[len] == '\n')
			st[len] = ' ';
		if (st[len] != ' ' && st[len] != '0' && st[len] != '.')
			isFoundData = TRUE;
	}
	if (!isFoundData)
		return;
    len = strlen(st);
	Beg = *BegO;
	End = *EndO;
	if (RootTiers == NULL) {
		if ((RootTiers=NEW(ALLTIERS)) == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		t = RootTiers;
	} else {
		for (t=RootTiers; 1; t=t->nextTier) {
		    if (strcmp(t->tierName, name) == 0) {
			    if (Beg < t->lastTime)
			    	Beg = t->lastTime;
			    if (End <= Beg) {
			    	if (len < 5)
			    		End = Beg + 5;
			    	else if (len >= 5 && len < 10)
			    		End = Beg + 80;
			    	else if (len > 10)
			    		End = Beg + 200;
			    }
				t->lastTime = End;
				t->els = addAnnotation(t->els, Beg, End, st);
		    	if (name[0] == '*' && isChangeTime) {
		    		*BegO = Beg;
		    		*EndO = End;
		    	}
		    	return;
		    }
		    if (t->nextTier == NULL)
		    	break;
		}
		if ((t->nextTier=NEW(ALLTIERS)) == NULL) {
			freeChat2AnvilMem();
			fprintf(stderr,"Error: out of memory\n");
			cutt_exit(0);
		}
		t = t->nextTier;
	}
	
	if (End <= Beg) {
    	if (len < 5)
    		End = Beg + 5;
    	else if (len >= 5 && len < 10)
    		End = Beg + 80;
    	else if (len > 10)
    		End = Beg + 200;
	}
	t->nextTier = NULL;
	strcpy(t->tierName, name);
	t->lastTime = End;
	t->els = NULL;
	t->els = addAnnotation(NULL, Beg, End, st);
	if (name[0] == '*' && isChangeTime) {
		*BegO = Beg;
		*EndO = End;
	}
}

static void writeSpecsFile(ALLTIERS *trs) {
	char *tierNames[50], *s, isSpeaker, ts[256];
	int  i, j, tierNameIndex;
	FNType newfname[FNSize];
	FILE *fp;

	strcpy(newfname, oldfname);
	s = strrchr(newfname, '.');
	if (s != NULL)
		*s = EOS;
	strcat(newfname, ".anvil-spec.xml");
	fp = fopen(newfname, "w");
	if (fp == NULL)
		return;

	tierNameIndex = 0;
	for (; trs != NULL; trs=trs->nextTier) {
		for (i=0; i < tierNameIndex; i++) {
			if (strcmp(tierNames[i], trs->tierName) == 0)
				break;
		}
		if (i == tierNameIndex) {
			tierNames[tierNameIndex] = trs->tierName;
			tierNameIndex++;
			if (tierNameIndex >= 50)
				break;
		}
	}

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fp, "<annotation-spec>\n");
	fprintf(fp, "    <body>\n");
	fprintf(fp, "        <track-spec name=\"wave\" type=\"waveform\"/>\n");
	for (i=0; i < tierNameIndex; i++) {
		strcpy(templineC1, tierNames[i]);
		s = strrchr(templineC1, '.');
		if (s != NULL)
			*s = EOS;
		isSpeaker = FALSE;
		if (s == NULL) {
			if (templineC1[0] == '*')
				isSpeaker = TRUE;
			strcpy(ts, " words");
			s = ts;
		} else {
			*s = EOS;
			if (uS.mStricmp(s+1, "words") == 0) {
				isSpeaker = TRUE;
			} else if (*(s+1) == '%') {
			} else if (templineC1[0] == '*') {
				isSpeaker = TRUE;
			}
		}
		if (isSpeaker) {
			if (templineC1[0] == '*')
				fprintf(fp, "        <group name=\"%s\">\n", templineC1);
			else
				fprintf(fp, "        <group name=\"*%s\">\n", templineC1);
			fprintf(fp, "            <track-spec name=\"%s\" type=\"primary\">\n", s+1);
			fprintf(fp, "                <attribute name=\"token\" valuetype=\"String\"/>\n");
			fprintf(fp, "            </track-spec>\n");
			for (j=0; j < tierNameIndex; j++) {
				strcpy(templineC2, tierNames[j]);
				s = strrchr(templineC2, '.');
				isSpeaker = FALSE;
				if (s != NULL) {
					*s = EOS;
					s++;
					if (uS.mStricmp(s, "words") == 0) {
						isSpeaker = TRUE;
					} else if (strcmp(templineC1, templineC2) == 0 && *(s) == '%') {
					} else if (templineC2[0] == '*' || strcmp(templineC1, templineC2) != 0) {
						isSpeaker = TRUE;
					}
				} else
					s = templineC2;
				if (!isSpeaker) {
					fprintf(fp, "            <track-spec name=\"%s\" type=\"primary\">\n", s);
					fprintf(fp, "                <attribute name=\"CHATtoken\" valuetype=\"String\"/>\n");
					fprintf(fp, "            </track-spec>\n");
				}
			}
			fprintf(fp, "        </group>\n");
		}
	}
	fprintf(fp, "    </body>\n");
	fprintf(fp, "</annotation-spec>\n");
	fclose(fp);
	fprintf(fpout, "    <specification src=\"%s\" />\n", newfname);
}

void call() {
	long		Beg, End;
	long		bulletsCnt, numTiers;
	FNType		mediaFName[FNSize], errfile[FNSize], *s;
	char		tag[NAMESLEN], depOn[NAMESLEN];
	char		*bs, *es;
	char		isBulletFound, ftime, isSpTierFound, isMediaHeaderFound, mediaRes, isSpecFound;
	FILE		*errFp;

	if (attsRoot == NULL) {
		rd_AnvilAtts_f("attribs.cut", FALSE);
	}

	isMediaHeaderFound = FALSE;
	mediaFName[0] = EOS;
	depOn[0] = EOS;
	ftime = TRUE;
	errFp = NULL;
	isSpTierFound = FALSE;
	bulletsCnt = 0L;
	numTiers = 0L;
	Beg = End = 0L;
	fprintf(fpout, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fpout, "<annotation>\n");
	fprintf(fpout, "  <head>\n");
	isSpecFound = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		uS.remblanks(utterance->speaker);
		if (utterance->speaker[0] == '@') {
			if (!isSpTierFound) {
				if (uS.isUTF8(utterance->speaker) ||
					uS.isInvisibleHeader(utterance->speaker) ||
					uS.partcmp(utterance->speaker, "@Begin", FALSE, FALSE) || 
					uS.partcmp(utterance->speaker, "@End", FALSE, FALSE)) {
				} else if (uS.partcmp(utterance->speaker, "@Languages:", FALSE, FALSE) ||
						   uS.partcmp(utterance->speaker, "@Participants:", FALSE, FALSE) ||
						   uS.partcmp(utterance->speaker, "@Options:", FALSE, FALSE) ||
						   uS.partcmp(utterance->speaker, "@ID:", FALSE, FALSE)) {
					if (attsRoot == NULL) {
						bs = utterance->line;
						for (es=bs; *es != EOS; es++) ;
						textToXML(templineC, bs, es);
						uS.remFrontAndBackBlanks(templineC);
						fprintf(fpout, "    <info key=\"%s\" type=\"String\">\n", utterance->speaker);
						fprintf(fpout, "      %s\n", templineC);
						fprintf(fpout, "    </info>\n");
					}
				} else if (uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE)) {
					getMediaName(utterance->line, mediaFName, FNSize);
					isMediaHeaderFound = TRUE;
					s = strrchr(mediaFName,'.');
					if (s == NULL)
						strcat(mediaFName, mediaExt);
					fprintf(fpout, "    <video master=\"true\" src=\"%s\" />\n", mediaFName);
				} else if (uS.partcmp(utterance->speaker, "@Comment:", FALSE, FALSE)) {
					uS.remblanks(utterance->line);
					if (uS.partwcmp(utterance->line, "$SPECS_")) {
						isSpecFound = TRUE;
						fprintf(fpout, "    <specification src=\"%s\" />\n", utterance->line+strlen("$SPECS_"));
					} else {
						fprintf(fpout, "    <info key=\"%s\" type=\"String\">\n", "@Comment:");
						fprintf(fpout, "      %s\n", utterance->line);
						fprintf(fpout, "    </info>\n");
					}
				} else {
					convertChatTiers2AnvilTag(tag, utterance->speaker, "");
					if (strcmp(tag, "IGNORE_THIS_TAG") != 0) {
						uS.remblanks(utterance->line);
						fprintf(fpout, "    <info key=\"%s\" type=\"String\">\n", tag);
						fprintf(fpout, "      %s\n", utterance->line);
						fprintf(fpout, "    </info>\n");
					}
				}
			} else {
				if (strcmp(utterance->speaker, "@End")) {
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "It is illegal to have Header Tiers '@' inside the transcript\n");
					if (errFp != NULL) {
						fprintf(errFp,"*** File \"%s\": line %ld.\n", oldfname, lineno);
						fprintf(errFp, "It is illegal to have Header Tiers '@' inside the transcript\n");
					}
				}
			}
			continue;
		} else {
			isSpTierFound = TRUE;
		}
		if (utterance->speaker[0] == '*')
			numTiers++;
		isBulletFound = FALSE;
		bs = utterance->line;
		if (utterance->speaker[0] == '*') {
			strcpy(depOn, utterance->speaker);
		}
		for (es=bs; *es != EOS;) {
			if (*es == HIDEN_C) {
				if (isMediaHeaderFound)
					mediaRes = getMediaTagInfo(es, &Beg, &End);
				else
					mediaRes = getOLDMediaTagInfo(es, NULL, mediaFName, &Beg, &End);
				if (mediaRes) {
					textToXML(templineC, bs, es);
					if (templineC[0] != EOS) {
						if (utterance->speaker[0] == '*') {
							convertChatTiers2AnvilTag(tag, utterance->speaker, "");
							isBulletFound = TRUE;
						} else {
							convertChatTiers2AnvilTag(tag, utterance->speaker, depOn);
						}
						addTierAnnotation(tag, &Beg, &End, templineC, TRUE, isSpecFound);
					}
				}

				for (bs=es+1; *bs != HIDEN_C && *bs != EOS; bs++) ;
				if (*bs == HIDEN_C)
					bs++;
				es = bs;
			} else
				 es++;
		}
		if (isBulletFound)
			bulletsCnt++;
		else if (utterance->speaker[0] == '*') {
			if (errFp == NULL && ftime) {
				ftime = FALSE;
				parsfname(oldfname, errfile,".err.cex");
				errFp = fopen(errfile, "w");
				if (errFp == NULL) {
					fprintf(stderr, "Can't create file \"%s\", perhaps it is opened by another application\n", errfile);
				}
#ifdef _MAC_CODE
				else
					settyp(errfile, 'TEXT', the_file_creator.out, FALSE);
#endif
			}
			if (errFp != NULL) {
				fprintf(errFp,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(errFp,"%s:\t%s\n", utterance->speaker, utterance->line);
			}
		}
		textToXML(templineC, bs, es);
		if (templineC[0] != EOS) {
			if (utterance->speaker[0] == '*') {
				convertChatTiers2AnvilTag(tag, utterance->speaker, "");
			} else {
				convertChatTiers2AnvilTag(tag, utterance->speaker, depOn);
			}
			addTierAnnotation(tag, &Beg, &End, templineC, FALSE, isSpecFound);
		}
	}

	if (bulletsCnt <= 0L) {
		fprintf(stderr, "\n\n    NO BULLETS FOUND IN THE FILE\n\n\n");
		if (errFp != NULL)
			fprintf(errFp, "\n\n    NO BULLETS FOUND IN THE FILE\n");
	} else {
		if (bulletsCnt != numTiers)
			fprintf(stderr, "\n\n");
		fprintf(stderr, "    Number of tiers found: %ld\n", numTiers);
		fprintf(stderr, "    Number of missing bullets: %ld\n", numTiers-bulletsCnt);
		if (bulletsCnt != numTiers)
			fprintf(stderr, "\n**** PLEASE LOOKS AT FILE \"%s\"\n\n", errfile);
		if (errFp != NULL) {
			fprintf(errFp, "\n\n\n    Number of tiers found: %ld\n", numTiers);
			fprintf(errFp, "    Number of missing bullets: %ld\n", numTiers-bulletsCnt);
		}
	}
	if (!isSpecFound) {
		writeSpecsFile(RootTiers);
	}
	fprintf(fpout, "  </head>\n");
	addValueLinks(RootTiers);
	PrintTiers(RootTiers);
	fprintf(fpout, "</annotation>\n");

	if (errFp != NULL)
		fclose(errFp);
	RootTiers = freeTiers(RootTiers);
}
