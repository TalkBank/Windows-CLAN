/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "check.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main chat2xmar_main
#define call chat2xmar_call
#define getflag chat2xmar_getflag
#define init chat2xmar_init
#define usage chat2xmar_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#define TORT struct TimeOrders
struct TimeOrders {
	long num;
	long time;
	char *bookmark;
	struct TimeOrders *nextTR;
} ;

#define ANNOTATIONS struct AllAnnotations
struct AllAnnotations {
	long beg;
	long end;
	char *st;
	ANNOTATIONS *nextAnnotation;
} ;

#define ALLTIERS struct AllTiers
struct AllTiers {
	char name[20];
	char typeRef;
	long num;
	long lastTime;
	char isPrinted;
	ANNOTATIONS *annotation;
	ALLTIERS *nextTier;
} ;

#define HEADERS struct HeaderTiers
struct HeaderTiers {
	char name[20];
	char *text;
	char gender;
	char language[20];
	char *IDtext;
	HEADERS *nextItem;
} ;


extern struct tier *defheadtier;
extern char OverWriteFile;

static TORT *timeOrder;
static ALLTIERS *RootTiers;
static ALLTIERS *RootPhoto;
static HEADERS *RootHeaders;
static HEADERS *RootSpeakers;
static FNType mediaExt[10];


/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		timeOrder = NULL;
		OverWriteFile = TRUE;
		AddCEXExtension = ".xml";
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
		RootTiers = NULL;
		RootPhoto = NULL;
		RootHeaders = NULL;
		RootSpeakers = NULL;
		mediaExt[0] = EOS;
	} else {
		if (mediaExt[0] == EOS) {
			fprintf(stderr,"Please use +e option to specify media file extension.\n    (For example: +e.mov or +e.mpg or +e.wav)\n");
			cutt_exit(0);
		}
	}
}

void usage() {
	printf("convert CHAT files to EXMARaLDA XML files\n");
	printf("Usage: chat2xmar [eS %s] filename(s)\n", mainflgs());
	puts("+eS: Specify media file name extension.");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CHAT2XMAR;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
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

static TORT *freeTR(TORT *p) {
	TORT *t;

	while (p != NULL) {
		t = p;
		p = p->nextTR;
		if (t->bookmark != NULL)
			free(t->bookmark);
		free(t);
	}
	return(NULL);
}


static void freeAnnotations(ANNOTATIONS *p) {
	ANNOTATIONS *t;

	while (p != NULL) {
		t = p;
		p = p->nextAnnotation;
		if (t->st != NULL)
			free(t->st);
		free(t);
	}
}

static ALLTIERS *freeTiers(ALLTIERS *p) {
	ALLTIERS *t;

	while (p != NULL) {
		t = p;
		p = p->nextTier;
		freeAnnotations(t->annotation);
		free(t);
	}
	return(NULL);
}

static HEADERS *freeHeaders(HEADERS *p) {
	HEADERS *t;

	while (p != NULL) {
		t = p;
		p = p->nextItem;
		if (t->text != NULL)
			free(t->text);
		if (t->IDtext != NULL)
			free(t->IDtext);
		free(t);
	}
	return(NULL);
}

static void addTimes(long time_value, char *bookmark) {
    TORT *pt, *t, *tt;

	if ((pt=NEW(TORT)) == NULL)
		out_of_mem();

	if (timeOrder == NULL) {
		timeOrder = pt;
		pt->nextTR = NULL;
	} else if (timeOrder->time > time_value && time_value >= 0L) {
		pt->nextTR = timeOrder;
		timeOrder = pt;
	} else {
		t = timeOrder;
		tt = timeOrder->nextTR;
		while (tt != NULL) {
			if (tt->time == time_value) {
				if (tt->bookmark == NULL && bookmark == NULL)
					return;
				else
					break;
			}
			if (tt->time > time_value && time_value >= 0L)
				break;
			t = tt;
			tt = tt->nextTR;
		}
		if (tt == NULL) {
			t->nextTR = pt;
			pt->nextTR = NULL;
		} else {
			pt->nextTR = tt;
			t->nextTR = pt;
		}
	}
	pt->num = 0;
	pt->time = time_value;
	if (bookmark == NULL)
		pt->bookmark = NULL;
	else {
		pt->bookmark = (char *)malloc(strlen(bookmark)+1);
		if (pt->bookmark == NULL) out_of_mem();
		strcpy(pt->bookmark, bookmark);
	}
}

static void NumberTimes(void) {
    TORT *t;
    long num;

	num = 0L;
	for (t=timeOrder; t != NULL; t=t->nextTR) {
		t->num = num;
		num++;
	}
}


static void getTimeNum(long time_value, long *num) {
    TORT *t;

	*num = 0L;
	for (t=timeOrder; t != NULL; t = t->nextTR) {
		if (t->time == time_value) {
			*num = t->num;
			break;
		}
	}
}

static void addAnnotation(ALLTIERS *tier, const FNType *mediaFName, long Beg, long End, char *st, char isFirstBullet) {
	ANNOTATIONS *t;
	ANNOTATIONS *root;

	root = tier->annotation;
	if (root == NULL) {
		if ((root=NEW(ANNOTATIONS)) == NULL) out_of_mem();
		t = root;
	} else {
		for (t=root; t->nextAnnotation != NULL; t=t->nextAnnotation) ;
		if ((t->nextAnnotation=NEW(ANNOTATIONS)) == NULL) out_of_mem();
		t = t->nextAnnotation;
	}
	t->nextAnnotation = NULL;
	t->beg   = Beg;
	t->end   = End;
	t->st    = (char *)malloc(strlen(st)+1);
	if (t->st == NULL) out_of_mem();
	strcpy(t->st, st);
	tier->annotation = root;
}

static ALLTIERS *addTierAnnotation(ALLTIERS *root, const FNType *mediaFName, const char *name, long *BegO, long *EndO, char *st, char isFirstBullet, char isChangeTime) {
	char typeRef;
	int  len;
	long num, Beg, End;
	ALLTIERS *t;

	if (name[0] == '*')
		typeRef = 't';
	else
		typeRef = 'a';

    len = strlen(st);
	num = 2L;
	Beg = *BegO;
	End = *EndO;
	if (root == NULL) {
		if ((root=NEW(ALLTIERS)) == NULL) out_of_mem();
		t = root;
	} else {
		for (t=root; 1; t=t->nextTier) {
		    if (strcmp(t->name, name) == 0 && t->typeRef == typeRef) {
			    if (Beg < t->lastTime)
			    	Beg = t->lastTime;
			    if (End <= Beg) {
			    	if (len < 5)
			    		End = Beg + 5;
			    	else if (len >= 5 && len < 10)
			    		End = Beg + 80;
			    	else if (len >= 10)
			    		End = Beg + 200;
			    }
				addTimes(Beg, NULL);
				addTimes(End, NULL);
				t->lastTime = End;
		    	addAnnotation(t, mediaFName, Beg, End, st, isFirstBullet);
		    	if (name[0] == '*' && isChangeTime) {
		    		*BegO = Beg;
		    		*EndO = End;
		    	}
		    	return(root);
		    }
		    num++;
		    if (t->nextTier == NULL)
		    	break;
		}
		if ((t->nextTier=NEW(ALLTIERS)) == NULL)
			out_of_mem();
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
	t->isPrinted = FALSE;
	strcpy(t->name, name);
	t->typeRef = typeRef;
	t->num = num;
	addTimes(Beg, NULL);
	addTimes(End, NULL);
	t->lastTime = End;
	t->annotation = NULL;
	addAnnotation(t, mediaFName, Beg, End, st, isFirstBullet);
	if (name[0] == '*' && isChangeTime) {
		*BegO = Beg;
		*EndO = End;
	}
	return(root);
}

static void addHeaders(char *name, char *line) {
	char	*bs, *es;
	HEADERS *t;

	bs = line;
	es = line + strlen(line);
	textToXML(templineC, bs, es);

	if (RootHeaders == NULL) {
		if ((RootHeaders=NEW(HEADERS)) == NULL) out_of_mem();
		t = RootHeaders;
	} else {
		for (t=RootHeaders; t->nextItem != NULL; t=t->nextItem) ;
		if ((t->nextItem=NEW(HEADERS)) == NULL) out_of_mem();
		t = t->nextItem;
	}

	t->nextItem = NULL;
	strcpy(t->name, name);
	t->text = (char *)malloc(strlen(templineC)+1);
	if (t->text == NULL) out_of_mem();
	strcpy(t->text, templineC);
	t->gender = '\0';
	t->language[0] = EOS;
	t->IDtext = NULL;
}

static void addSpeakersInfo(char *name, char *line) {
	HEADERS *t;

	if (RootSpeakers == NULL) {
		if ((RootSpeakers=NEW(HEADERS)) == NULL) out_of_mem();
		t = RootSpeakers;
	} else {
		for (t=RootSpeakers; t->nextItem != NULL; t=t->nextItem) ;
		if ((t->nextItem=NEW(HEADERS)) == NULL) out_of_mem();
		t = t->nextItem;
	}

	t->nextItem = NULL;
	strcpy(t->name, name);
	t->text = (char *)malloc(strlen(line)+1);
	if (t->text == NULL) out_of_mem();
	strcpy(t->text, line);
	t->gender = '\0';
	t->language[0] = EOS;
	t->IDtext = NULL;
}

static void GetSpeakersInfo(char *line) {
	char sp[SPEAKERLEN], rest[SPEAKERLEN];
	char *s, *e, t, wc;
	short cnt = 0;

	for (; *line && (*line == ' ' || *line == '\t'); line++) ;
	s = line;
	sp[0] = EOS;
	rest[0] = EOS;
	while (*s) {
		if (*line == ','  || *line == ' '  ||
			*line == '\t' || *line == '\n' || *line == EOS) {

			if (*line == ',' && !isSpace(*(line+1)) && *(line+1) != '\n')
				return;

			wc = ' ';
			e = line;
			for (; *line == ' ' || *line == '\t' || *line == '\n'; line++) ;
			if (*line != ',' && *line != EOS)
				line--;
			else
				wc = ',';

			if (*line) {
				t = *e;
				*e = EOS;
				if (cnt == 2 || wc == ',') {
					if (rest[0] == EOS)
						strcpy(rest, s);
					else {
						strcat(rest, " ");
						strcat(rest, s);
					}
					addSpeakersInfo(sp, rest);
					sp[0] = EOS;
					rest[0] = EOS;
				} else if (cnt == 0) {
					strcpy(sp, s);
				} else if (cnt == 1) {
					strcpy(rest, s);
				}
				*e = t;
				if (wc == ',') {
					if (cnt == 0)
						return;
					cnt = -1;
					sp[0] = EOS;
					rest[0] = EOS;
				}
				for (line++; *line==' ' || *line=='\t' || *line=='\n' || *line==','; line++) {
					if (*line == ',') {
						if (cnt == 0)
							return;
						cnt = -1;
						sp[0] = EOS;
						rest[0] = EOS;
					}
				}
			} else {
				for (line=e; *line; line++) ;
				if (cnt == 0) {
					return;
				} else {
					t = *e;
					*e = EOS;
					if (rest[0] == EOS)
						strcpy(rest, s);
					else {
						strcat(rest, " ");
						strcat(rest, s);
					}
					addSpeakersInfo(sp, rest);
					sp[0] = EOS;
					rest[0] = EOS;
					*e = t;
				}
				for (line=e; *line; line++) ;
			}
			if (cnt == 2) {
				cnt = 0;
				sp[0] = EOS;
				rest[0] = EOS;
			} else
				cnt++;
			s = line;
		} else
			line++;
	}
}

static void ParseIDsInfo(char *line, char *oLanguage) {
	char sp[SPEAKERLEN], gender, language[20];
	int s = 0, e = 0, cnt;
	char word[BUFSIZ], *st;
	HEADERS *t;

	uS.remblanks(line);
	sp[0] = EOS;
	gender = '\0';
	language[0] = EOS;
	cnt = 0;
	word[0] = EOS;
	while (1) {
		st = word;
		if ((*st=line[s]) == '|') {
			e = s + 1;
		} else {
			while ((*st=line[s]) == '|' || uS.isskip(line, s, &dFnt, MBF)) {
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
		}
		*st = EOS;
		if (cnt == 0) {
			if (word[0] != EOS)
				strncpy(language, word, 19);
			language[19] = EOS;
		} else if (cnt == 1) {
		} else if (cnt == 2) {
			if (word[0] != EOS)
				strncpy(sp, word, 19);
		} else if (cnt == 3) {
		} else if (cnt == 4) {
			if (word[0] != EOS) {
				if (!strcmp(word, "male"))
					gender = 'm';
				if (!strcmp(word, "female"))
					gender = 'f';
			}
		} else if (cnt == 5) {
		} else if (cnt == 6) {
		} else if (cnt == 7) {
		} else if (cnt == 8) {
		}
		s = e;
		cnt++;
	}

	for (t=RootSpeakers; t != NULL; t=t->nextItem) {
		if (uS.mStricmp(sp, t->name) == 0) {
			t->IDtext = (char *)malloc(strlen(line)+1);
			if (t->IDtext == NULL) out_of_mem();
			strcpy(t->IDtext, line);
			if (language[0] == EOS)
				strcpy(t->language, oLanguage);
			else
				strcpy(t->language, language);
			t->gender = gender;
		}
	}
}

static void PrintAnnotations(ALLTIERS *trs, char *s) {
	long BegNum, EndNum;
	ANNOTATIONS *anns;

	if (trs->name[0] == '%') {
		fprintf(fpout, "\t\t<tier id=\"TIE%ld\" speaker=\"%s\" category=\"%s\" type=\"%c\" display-name=\"%s [%s]\">\n", trs->num, s+1, trs->name, trs->typeRef, s+1, trs->name);
	} else
		fprintf(fpout, "\t\t<tier id=\"TIE%ld\" speaker=\"%s\" category=\"main\" type=\"%c\" display-name=\"%s [main]\">\n", trs->num, s+1, trs->typeRef, s+1);

	for (anns=trs->annotation; anns != NULL; anns=anns->nextAnnotation) {
		getTimeNum(anns->beg, &BegNum);
		getTimeNum(anns->end, &EndNum);
		fprintf(fpout, "\t\t\t<event start=\"T%ld\" end=\"T%ld\">%s</event>\n", BegNum, EndNum, anns->st);
	}
	fprintf(fpout, "\t\t</tier>\n");
	trs->isPrinted = TRUE;
}
/* 2009-10-13
static void PrintLinkAnnotations(ALLTIERS *trs, char *s) {
	long BegNum, EndNum;
	ANNOTATIONS *anns;

	fprintf(fpout, "\t\t<tier id=\"%s\" category=\"%s\" type=\"%c\" display-name=\"[%s]\">\n", s+1, trs->name, trs->typeRef, trs->name);
	for (anns=trs->annotation; anns != NULL; anns=anns->nextAnnotation) {
		getTimeNum(anns->beg, &BegNum);
		getTimeNum(anns->end, &EndNum);
		fprintf(fpout, "\t\t\t<event start=\"T%d\" end=\"T%d\" medium=\"%s\" url=\"media/%s\">%s</event>\n", BegNum, EndNum, trs->name, anns->st, anns->st);
	}
	fprintf(fpout, "\t\t</tier>\n");
	trs->isPrinted = TRUE;
}
*/
static void PrintTiers(void) {
	char *s;
	char spName[20];
	ALLTIERS *maintrs, *trs;

	for (maintrs=RootTiers; maintrs != NULL; maintrs=maintrs->nextTier) {
		if (maintrs->name[0] != '*')
			continue;

		strcpy(spName, maintrs->name+1);
		PrintAnnotations(maintrs, maintrs->name);

		for (trs=RootTiers; trs != NULL; trs=trs->nextTier) {
			if (trs->isPrinted == TRUE || trs->name[0] == '*')
				continue;

			if (trs->name[0] == '%') {
				s = strrchr(trs->name, '@');
				if (s == NULL)
					s = trs->name;
				else {
					if (strcmp(spName, s+1) != 0)
						continue;
					*s = EOS;
				}
			} else
				s = trs->name;

			PrintAnnotations(trs, s);
		}
	}
}

void call() {
	long		Beg, End, BegSp = 0L, EndSp = 0L, t;
	long		bulletsCnt, numTiers;
	double		timeSecs;
	FNType		mediaFName[FNSize], oldMediaFName[FNSize];
	FNType		errfile[FNSize];
	char		name[20], spName[20], language[20];
	char		*bs, *es, *s;
	char		isBulletFound, isFirstBullet, ftime, isSpTierFound, isMediaHeaderFound, mediaRes;
	FILE		*errFp;
	TORT        *times;
	HEADERS		*tp;
	ALLTIERS	*trs;

	isMediaHeaderFound = FALSE;
	language[0] = EOS;
	mediaFName[0] = EOS;
	oldMediaFName[0] = EOS;
	ftime = TRUE;
	errFp = NULL;
	isSpTierFound = FALSE;
	bulletsCnt = 0L;
	numTiers = 0L;
	Beg = End = 0L;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE)) {
			getMediaName(utterance->line, mediaFName, FNSize);
			isMediaHeaderFound = TRUE;
		}
		uS.remblanks(utterance->speaker);
		if (utterance->speaker[0] == '@' && !isSpTierFound) {
			if (uS.isUTF8(utterance->speaker) ||
					uS.partcmp(utterance->speaker, "@Begin", FALSE, FALSE) ||
					uS.partcmp(utterance->speaker, "@End", FALSE, FALSE))
				;
			else if (uS.partcmp(utterance->speaker, PARTICIPANTS, FALSE, FALSE))
				GetSpeakersInfo(utterance->line);
			else if (uS.partcmp(utterance->speaker, IDOF, FALSE, FALSE))
				ParseIDsInfo(utterance->line, language);
			else {
				if (uS.partcmp(utterance->speaker, "@Languages", FALSE, FALSE)) {
					strncpy(language, utterance->line, 20);
					language[19] = EOS;
					uS.remblanks(language);
				}
				t = strlen(utterance->speaker) - 1;
				if (utterance->speaker[t] == ':')
					utterance->speaker[t] = EOS;
				addHeaders(utterance->speaker, utterance->line);
			}
			continue;
		} else if (utterance->speaker[0] == '@' && isSpTierFound) {
			if (strcmp(utterance->speaker, "@End")) {
				strcpy(templineC, "CHAT:");
				strcat(templineC, utterance->speaker);
				t = strlen(templineC);
				textToXML(templineC+t, utterance->line, utterance->line+strlen(utterance->line));
				addTimes(EndSp, templineC);
/*
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
					fprintf(errFp, "It is illegal to have Header Tiers '@' inside the transcript\n");
					fprintf(errFp, "%s%s", utterance->speaker, utterance->line);
				} else {
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
					fprintf(stderr, "It is illegal to have Header Tiers '@' inside the transcript\n");
				}
*/
			}
			continue;
		} else
			isSpTierFound = TRUE;
		if (utterance->speaker[0] == '*')
			numTiers++;
		isBulletFound = FALSE;
		t = strlen(utterance->speaker) - 1;
		for (; t >= 0L && (isSpace(utterance->speaker[t]) || utterance->speaker[t] == ':'); t--) ;
		utterance->speaker[++t] = EOS;
		if (utterance->speaker[0] == '*') {
			BegSp = 0L;
			EndSp = 0L;
		}
		isFirstBullet = TRUE;
		bs = utterance->line;
		for (es=bs; *es != EOS;) {
			if (*es == HIDEN_C) {
				if (isMediaHeaderFound)
					mediaRes = getMediaTagInfo(es, &Beg, &End);
				else
					mediaRes = getOLDMediaTagInfo(es, NULL, mediaFName, &Beg, &End);
				if (mediaRes) {
					if (strrchr(mediaFName, '.') == NULL && mediaFName[0] != EOS)
						strcat(mediaFName, mediaExt);

					if (strcmp(oldMediaFName, mediaFName)) {
						for (trs=RootTiers; trs != NULL; trs=trs->nextTier)
							trs->lastTime = 0L;
						strcpy(oldMediaFName, mediaFName);
					}
					if (Beg == 1L)
						Beg = 0L;
					textToXML(templineC, bs, es);
					if (templineC[0] != EOS) {
						if (utterance->speaker[0] == '*') {
							strcpy(name, utterance->speaker);
							strcpy(spName, utterance->speaker+1);
							isBulletFound = TRUE;
						} else {
							strcpy(name, utterance->speaker);
							strcat(name, "@");
							strcat(name, spName);
							if (Beg < BegSp)
								Beg = BegSp;
						}
						RootTiers = addTierAnnotation(RootTiers, mediaFName, name, &Beg, &End, templineC, isFirstBullet, TRUE);
						if (utterance->speaker[0] == '*') {
							if (BegSp == 0L)
								BegSp = Beg;
							EndSp = End;
						}
					}
				}

				for (bs=es+1; *bs != HIDEN_C && *bs != EOS; bs++) ;
				if (*bs == HIDEN_C)
					bs++;
				es = bs;
			} else
				 es++;
			isFirstBullet = FALSE;
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
				fprintf(errFp,"Missing bullet on tier\n");
				fprintf(errFp,"%s:\t%s", utterance->speaker, utterance->line);
			} else {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, lineno);
				fprintf(stderr,"Missing bullet on tier\n");
				fprintf(stderr,"%s:\t%s", utterance->speaker, utterance->line);
			}
		}
		textToXML(templineC, bs, es);
		if (templineC[0] != EOS) {
			if (utterance->speaker[0] == '*') {
				strcpy(name, utterance->speaker);
				strcpy(spName, utterance->speaker+1);
				if (BegSp == 0L)
					BegSp = -2;
				if (EndSp == 0L)
					EndSp = -2;
				if (!isMediaHeaderFound)
					mediaFName[0] = EOS;
				RootTiers = addTierAnnotation(RootTiers, "", name, &BegSp, &EndSp, templineC, isFirstBullet, FALSE);
			} else {
				strcpy(name, utterance->speaker);
				strcat(name, "@");
				strcat(name, spName);
				RootTiers = addTierAnnotation(RootTiers, mediaFName, name, &BegSp, &EndSp, templineC, isFirstBullet, FALSE);
			}
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
		if (bulletsCnt != numTiers) {
			fprintf(stderr, "\nTHIS FILE DID NOT FULLY CONVERT. SOME BULLETS WERE MISSING.\n");
			fprintf(stderr, "PLEASE LOOKS AT FILE \"%s\" FOR ALL ERROR MESSAGES\n\n", errfile);
		}
		if (errFp != NULL) {
			fprintf(errFp, "\n\n\n    Number of tiers found: %ld\n", numTiers);
			fprintf(errFp, "    Number of missing bullets: %ld\n", numTiers-bulletsCnt);
		}
	}

	fprintf(fpout, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fpout, "<!-- (c) http://www.rrz.uni-hamburg.de/exmaralda -->\n");
	fprintf(fpout, "<basic-transcription>\n");


	fprintf(fpout, "\t<head>\n");
	fprintf(fpout, "\t\t<meta-information>\n");
	fprintf(fpout, "\t\t\t<project-name/>\n");
	fprintf(fpout, "\t\t\t<transcription-name/>\n");
	fprintf(fpout, "\t\t\t<referenced-file url=\"%s\"/>\n", oldMediaFName);
	fprintf(fpout, "\t\t\t<ud-meta-information>\n");
	for (tp=RootHeaders; tp != NULL; tp=tp->nextItem) {
		fprintf(fpout, "\t\t\t\t<ud-information attribute-name=\"CHAT:%s\">%s</ud-information>\n", tp->name, tp->text);
	}
	fprintf(fpout, "\t\t\t</ud-meta-information>\n");
	fprintf(fpout, "\t\t\t<comment/>\n");
	fprintf(fpout, "\t\t\t<transcription-convention/>\n");
	fprintf(fpout, "\t\t</meta-information>\n");
	fprintf(fpout, "\t\t<speakertable>\n");
	for (tp=RootSpeakers; tp != NULL; tp=tp->nextItem) {
		fprintf(fpout, "\t\t\t<speaker id=\"%s\">\n", tp->name);
		fprintf(fpout, "\t\t\t\t<abbreviation>%s</abbreviation>\n", tp->name);
		if (tp->gender == '\0')
			fprintf(fpout, "\t\t\t\t<sex value=\"u\"/>\n");
		else
			fprintf(fpout, "\t\t\t\t<sex value=\"%c\"/>\n", tp->gender);
		fprintf(fpout, "\t\t\t\t<languages-used/>\n");
//		fprintf(fpout, "\t\t\t\t<languages-used>%s</languages-used>\n", tp->language);
		fprintf(fpout, "\t\t\t\t<l1/>\n");
		fprintf(fpout, "\t\t\t\t<l2/>\n");
		fprintf(fpout, "\t\t\t\t<ud-speaker-information>\n");

		fprintf(fpout, "\t\t\t\t\t<ud-information attribute-name=\"CHAT:Name\">%s</ud-information>\n", tp->name);
		s = strrchr(tp->text,' ');
		if (s != NULL) {
			*s = EOS;
			fprintf(fpout, "\t\t\t\t\t<ud-information attribute-name=\"CHAT:Full Name\">%s</ud-information>\n", tp->text);
			fprintf(fpout, "\t\t\t\t\t<ud-information attribute-name=\"CHAT:Role\">%s</ud-information>\n", s+1);
		} else
			fprintf(fpout, "\t\t\t\t\t<ud-information attribute-name=\"CHAT:Role\">%s</ud-information>\n", tp->text);
		if (tp->IDtext != NULL && tp->IDtext[0] != EOS)
			fprintf(fpout, "\t\t\t\t\t<ud-information attribute-name=\"CHAT:ID\">%s</ud-information>\n", tp->IDtext);

		fprintf(fpout, "\t\t\t\t</ud-speaker-information>\n");
		fprintf(fpout, "\t\t\t\t<comment/>\n");
		fprintf(fpout, "\t\t\t</speaker>\n");
	}
	fprintf(fpout, "\t\t</speakertable>\n");
	fprintf(fpout, "\t</head>\n");

	fprintf(fpout, "\t<basic-body>\n");
	fprintf(fpout, "\t\t<common-timeline>\n");
	NumberTimes();
	for (times=timeOrder; times != NULL; times=times->nextTR) {
		timeSecs = (double)times->time;
		timeSecs = timeSecs / 1000.0000000;
		sprintf(templineC, "%lf", timeSecs);
		for (t=strlen(templineC)-1; templineC[t] == '0' && t > 0; t--) ;
		if (templineC[t] == '.')
			templineC[t+2] = EOS;
		else
			templineC[t+1] = EOS;
		if (times->bookmark != NULL && times->bookmark[0] != EOS)
			fprintf(fpout, "\t\t\t<tli id=\"T%ld\" time=\"%s\" bookmark=\"%s\"/>\n", times->num, templineC, times->bookmark);
		else
			fprintf(fpout, "\t\t\t<tli id=\"T%ld\" time=\"%s\"/>\n", times->num, templineC);
	}
	fprintf(fpout, "\t\t</common-timeline>\n");

	PrintTiers();

	fprintf(fpout, "\t</basic-body>\n");
	fprintf(fpout, "</basic-transcription>\n");


	if (errFp != NULL)
		fclose(errFp);
	timeOrder = freeTR(timeOrder);
	RootTiers = freeTiers(RootTiers);
	RootPhoto = freeTiers(RootPhoto);
	RootHeaders = freeHeaders(RootHeaders);
	RootSpeakers = freeHeaders(RootSpeakers);
}
