/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "c_curses.h"
#ifdef _WIN32
	#include "stdafx.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main play2chat_main
#define call play2chat_call
#define getflag play2chat_getflag
#define init play2chat_init
#define usage play2chat_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

#define SPKRTIER 1
#define DEPTTIER 2
#define GESTTIER 3

#define TAGSLEN  128

extern struct tier *defheadtier;
extern char OverWriteFile;
extern long option_flags[];
extern char GExt[];

struct linesRec {
	char sp[TAGSLEN];
	char whichTier;
	char whichSpkr;
	char *line;
	long bt, et;
	struct linesRec *added;
	struct linesRec *dep;
	struct linesRec *nextLine;
} ;
typedef struct linesRec LINESLIST;

static char isDebug;
static LINESLIST *linesRoot;

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		linesRoot = NULL;
		isDebug = FALSE;
		OverWriteFile = TRUE;
		AddCEXExtension = "";
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	} else {
		strcpy(GExt, ".cha");
		option_flags[CLAN_PROG_NUM] = 0L;
	}
}

void usage() {
	printf("Converts Datavyu text files to CHAT text files\n");
	printf("Usage: temp [d %s] filename(s)\n",mainflgs());
	puts("+d : check utterances for illegal overlaps");
	mainusage(TRUE);
}

static LINESLIST *freeLines(LINESLIST *p) {
	LINESLIST *t;

	while (p != NULL) {
		t = p;
		p = p->nextLine;
		if (t->line != NULL)
			free(t->line);
		if (t->added != NULL)
			t->added = freeLines(t->added);
		if (t->dep != NULL)
			t->dep = freeLines(t->dep);
		free(t);
	}
	return(NULL);
}

static void freeMem(void) {
	linesRoot = freeLines(linesRoot);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = PLAY2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;

	bmain(argc,argv,NULL);

	linesRoot = freeLines(linesRoot);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'd':
			isDebug = TRUE;
			break;
		default:
			fprintf(stderr,"Invalid option: %s\n", f-2);
			cutt_exit(0);
			break;
	}
}

static char isRightSpeaker(LINESLIST *nt, char whichSpkr) {
	if (nt->whichTier != SPKRTIER)
		return(FALSE);

	if ((nt->whichSpkr == 'm' && whichSpkr != 'c') ||
		(nt->whichSpkr == 'c' && whichSpkr != 'm'))
		return(TRUE);

	return(FALSE);
}

static void fillInTier(LINESLIST *nt, char *sp, char *line, char whichTier, char whichSpkr, long bt, long et) {
	int  i;
	char isUTDFound;

	isUTDFound = FALSE;
	for (i=strlen(line)-1; i >= 0; i--) {
		if (uS.IsUtteranceDel(line, i))
			isUTDFound = TRUE;
		if (!uS.isskip(line,i,&dFnt,MBF))
			break;
	}
	nt->added = NULL;
	nt->dep = NULL;
	nt->line = NULL;

	if (bt == et || et == 0L)
		et = bt + 3L;
	if (isUTDFound)
		sprintf(templineC2, " %c%ld_%ld%c\n", HIDEN_C, bt, et, HIDEN_C);
	else
		sprintf(templineC2, ". %c%ld_%ld%c\n", HIDEN_C, bt, et, HIDEN_C);
	nt->whichTier = whichTier;
	nt->whichSpkr = whichSpkr;
	strcpy(nt->sp, sp);
	nt->line = (char *)malloc(strlen(line)+strlen(templineC2)+1);
	if (nt->line == NULL) {
		freeMem();
		out_of_mem();
	}
	strcpy(nt->line, line);
	strcat(nt->line, templineC2);
	nt->bt = bt;
	nt->et = et;
}


static LINESLIST *add_dep_tier(LINESLIST *root, char whichTier, char whichSpkr, char *sp, char *line, long bt, long et) {
	LINESLIST *nt, *tnt;

	if (root == NULL) {
		if ((root=NEW(LINESLIST)) == NULL)
			out_of_mem();
		nt = root;
		nt->nextLine = NULL;
	} else {
		tnt= root;
		nt = root;
		while (nt != NULL) {
			if (bt <= nt->bt)
				break;
			tnt = nt;
			nt = nt->nextLine;
		}
		if (nt == NULL) {
			tnt->nextLine = NEW(LINESLIST);
			if (tnt->nextLine == NULL)
				out_of_mem();
			nt = tnt->nextLine;
			nt->nextLine = NULL;
		} else if (nt == root) {
			root = NEW(LINESLIST);
			if (root == NULL)
				out_of_mem();
			root->nextLine = nt;
			nt = root;
		} else {
			nt = NEW(LINESLIST);
			if (nt == NULL)
				out_of_mem();
			nt->nextLine = tnt->nextLine;
			tnt->nextLine = nt;
		}
	}
	fillInTier(nt, sp, line, whichTier, whichSpkr, bt, et);
	return(root);
}

static LINESLIST *pc_add_each_line(LINESLIST *root, char whichLevel, char whichTier, char whichSpkr, char *sp, char *line, long bt, long et) {
	LINESLIST *nt, *tnt, *lnt, *ltnt;

	if (bt == 157318L)
		bt = 157318L;
	if (root == NULL) {
		if ((root=NEW(LINESLIST)) == NULL)
			out_of_mem();
		nt = root;
		nt->nextLine = NULL;
	} else {
		tnt= root;
		nt = root;
		if (whichTier == SPKRTIER) {
			while (nt != NULL) {
				if (bt < nt->bt) {
					if (nt->whichTier == SPKRTIER || whichLevel == 1)
						break;
				}
				tnt = nt;
				nt = nt->nextLine;
			}
		} else {
			lnt = NULL;
			ltnt = NULL;
			while (nt != NULL) {
				if (bt >= nt->bt && et <= nt->et && bt < et) {
					if (isRightSpeaker(nt, whichSpkr) || whichLevel == 1) {
						ltnt = nt;
						lnt = nt->nextLine;
						break;
					}
				}
				tnt = nt;
				nt = nt->nextLine;
			}
			if (ltnt != NULL) {
				tnt = ltnt;
				nt  = lnt;
			} else {
				tnt= root;
				nt = root;
				while (nt != NULL) {
					if (bt >= nt->bt && bt <= nt->et) {
						if (isRightSpeaker(nt, whichSpkr) || whichLevel == 1) {
							ltnt = nt;
							lnt = nt->nextLine;
							if (bt != nt->bt && nt->nextLine != NULL && nt->whichTier == SPKRTIER && bt == nt->nextLine->bt) {
							} else
								break;
						}
					} else if (bt == nt->bt && nt->et == 0L) {
						if (isRightSpeaker(nt, whichSpkr) || whichLevel == 1) {
							ltnt = nt;
							lnt = nt->nextLine;
							break;
						}
					}

					tnt = nt;
					nt = nt->nextLine;
				}
				if (ltnt != NULL) {
					tnt = ltnt;
					nt  = lnt;
				} else {
					tnt= root;
					nt = root;
					while (nt != NULL) {
						if (whichLevel == 0) {
							if (nt->whichTier == SPKRTIER && bt < nt->bt) {
								nt->added = pc_add_each_line(nt->added, 1, whichTier, whichSpkr, sp, line, bt, et);
								return(root);
							}
						} else {
							if (bt <= nt->bt) {
								break;
							}
						}
						tnt = nt;
						nt = nt->nextLine;
					}
				}
			}
		}

		if (whichLevel == 0 && (whichTier == DEPTTIER || whichTier == GESTTIER)) {
			tnt->dep = add_dep_tier(tnt->dep, whichTier, whichSpkr, sp, line, bt, et);
			return(root);
		}
		if (nt == NULL) {
			tnt->nextLine = NEW(LINESLIST);
			if (tnt->nextLine == NULL)
				out_of_mem();
			nt = tnt->nextLine;
			nt->nextLine = NULL;
		} else if (nt == root) {
			root = NEW(LINESLIST);
			if (root == NULL)
				out_of_mem();
			root->nextLine = nt;
			nt = root;
		} else {
			nt = NEW(LINESLIST);
			if (nt == NULL)
				out_of_mem();
			nt->nextLine = tnt->nextLine;
			tnt->nextLine = nt;
		}
	}
	fillInTier(nt, sp, line, whichTier, whichSpkr, bt, et);
	return(root);
}

static void cleanupLine(char *st) {
	int i;

	for (i=0; st[i] != EOS;) {
		if ((st[i] >= 0 && st[i] < 32) || st[i] == 0x7f)
			strcpy(st+i, st+i+1);
		else
			i++;
	}
}

static void changeCodeName(char *sp) {
	char buf[BUFSIZ];

	strcpy(buf, sp);
	if (uS.mStrnicmp(buf, "%mom", 4) == 0) {
		strcpy(sp, "%xm");
		strcat(sp, buf+4);
	} else if (uS.mStrnicmp(buf, "%child", 6) == 0) {
		strcpy(sp, "%xc");
		strcat(sp, buf+6);
	} else if (uS.mStrnicmp(buf, "%baby", 5) == 0) {
		strcpy(sp, "%xb");
		strcat(sp, buf+5);
	} else if (uS.mStricmp(sp, "%gesture:") == 0) {
		strcpy(sp, "%xges:");
	}
}

static void fillInPostcodes(LINESLIST *dep, char isJustOne, char *postcodes) {
	int i;

	for (; dep != NULL; dep=dep->nextLine) {
		if (uS.mStricmp(dep->sp, "%momutterancetype:") == 0 || uS.mStricmp(dep->sp, "%babyutterancetype:") == 0) {
			for (i=0L; dep->line[i] != EOS; i++) {
				if (dep->line[i] == '.') {
					dep->line[i] = ' ';
				} else if (dep->line[i] == HIDEN_C) {
					dep->line[i] = EOS;
					break;
				}
			}
			uS.remFrontAndBackBlanks(dep->line);
			if (postcodes[0] != EOS)
				strcat(postcodes, " [+ u:");
			else
				strcat(postcodes, "[+ u:");
			strcat(postcodes, dep->line);
			strcat(postcodes, "]");
		}
		if (isJustOne)
			break;
	}
}

static char isExcludeTier(char *sp) {
	if (uS.mStricmp(sp, "%momspeech:") == 0 || uS.mStricmp(sp, "%babyvoc:") == 0 ||
		uS.mStricmp(sp, "%momutterancetype:") == 0 || uS.mStricmp(sp, "%babyutterancetype:") == 0) {
		return(TRUE);
	}
	return(FALSE);
}

static void printAddedSpeaker(LINESLIST *e) {
	char whichSpkr, tsp[TAGSLEN];
	long i, bt, et;
	LINESLIST *nt, *tnt, *p;
	char isDone;

	nt = e;
	do {
		whichSpkr = EOS;
		bt = -1L;
		et = 0L;
		tnt = nt;
		while (nt != NULL) {
			if (whichSpkr == EOS) {
				whichSpkr = nt->whichSpkr;
			} else if (whichSpkr != nt->whichSpkr)
				break;
			if (bt == -1L || bt > nt->bt)
				bt = nt->bt;
			if (et < nt->et)
				et = nt->et;
			nt = nt->nextLine;
		}
		templineC3[0] = EOS;
		for (p=tnt; p != nt; p=p->nextLine) {
			uS.remblanks(p->line);
			fillInPostcodes(p, TRUE, templineC3);
		}
/* 2019-07-22 no bullets on 0.
		sprintf(templineC2, "0. %s %c%ld_%ld%c\n", templineC3, HIDEN_C, bt, et, HIDEN_C);
*/		sprintf(templineC2, "0. %s\n", templineC3);
		if (whichSpkr == 'm')
			printout("*MOT:", templineC2, NULL, NULL, TRUE);
		else if (whichSpkr == 'c')
			printout("*CHI:", templineC2, NULL, NULL, TRUE);
		else
			printout("*UNK:", templineC2, NULL, NULL, TRUE);

		do {
			isDone = TRUE;
			tsp[0] = EOS;
			templineC3[0] = EOS;
			for (p=tnt; p != nt; p=p->nextLine) {
/* 2019-07-22 no bullets on 0.
				if (bt == p->bt && et == p->et) {
					for (i=0L; p->line[i] != EOS; i++) {
						if (p->line[i] == HIDEN_C) {
							p->line[i] = EOS;
							break;
						}
					}
				}
*/
				uS.remblanks(p->line);
				if (tsp[0] == EOS && p->line[0] != EOS) {
					strcpy(tsp, p->sp);
					strcat(templineC3, p->line);
					strcat(templineC3, " ");
					p->line[0] = EOS;
					isDone = FALSE;
				} else if (uS.mStricmp(tsp, p->sp) == 0) {
					strcat(templineC3, p->line);
					strcat(templineC3, " ");
					p->line[0] = EOS;
					isDone = FALSE;
				}
			}
			for (i=0; templineC3[i] != EOS; i++) {
				if (templineC3[i] == '\n')
					templineC3[i] = ' ';
			}
			uS.remFrontAndBackBlanks(templineC3);
			removeExtraSpace(templineC3);
			if (tsp[0] != EOS && templineC3[0] != EOS && !isExcludeTier(tsp)) {
				changeCodeName(tsp);
				printout(tsp, templineC3, NULL, NULL, TRUE);
			}
		} while (isDone == FALSE) ;
	} while (nt != NULL) ;
}

static void insertPostcodes(char *inLine, char *postcode, char *outLine) {
	int i, o, p;

	if (postcode[0] == EOS){
		strcpy(outLine, inLine);
	} else {
		o = 0;
		for (i=0L; inLine[i] != EOS; i++) {
			if (inLine[i] == HIDEN_C && postcode[0] != EOS) {
				for (p=0; postcode[p] != EOS; p++) {
					outLine[o++] = postcode[p];
				}
				outLine[o++] = ' ';
				postcode[0] = EOS;
			}
			outLine[o++] = inLine[i];
		}
		outLine[o] = EOS;
	}
}

static void pc_printTiers(LINESLIST *p, long spBt, long spEt) {
	int i;

	for (; p != NULL; p=p->nextLine) {
		if (p->added != NULL) {
			printAddedSpeaker(p->added);
		}
		if (p->whichTier == DEPTTIER || p->whichTier == GESTTIER) {
			if (spBt == p->bt && spEt == p->et) {
				for (i=0L; p->line[i] != EOS; i++) {
					if (p->line[i] == HIDEN_C) {
						p->line[i] = EOS;
						break;
					}
				}
			}
		}
		uS.remblanks(p->line);
		templineC3[0] = EOS;
		if (p->whichTier == SPKRTIER)
			fillInPostcodes(p->dep, FALSE, templineC3);
		if (p->line[0] != EOS && !isExcludeTier(p->sp)) {
			insertPostcodes(p->line, templineC3, templineC2);
			changeCodeName(p->sp);
			printout(p->sp, templineC2, NULL, NULL, TRUE);
		}
		if (p->dep != NULL) {
			pc_printTiers(p->dep, p->bt, p->et);
		}
	}
}

static int pm(int n, char *s) {
	register int i;
	extern const char *MonthNames[];

	if (n == 0) {
		strcpy(templineC, s);
		uS.uppercasestr(templineC, &dFnt, MBF);
		for (i=0; i < 12; i++) {
			if (!strcmp(templineC,MonthNames[i]))
				return(i+1);
		}
		if (i >= 12) {
			fprintf(stderr, "Illegal month specification: %s. Must be English names of month:\n", s);
			fprintf(stderr, "  JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC\n");
			cutt_exit(0);
		}
	} else {
		if (n > 0 && n <= 12)
			strcpy(s, MonthNames[n-1]);
		else {
			fprintf(stderr, "Illegal month specification: %d. (Must be 1-12)\n", n);
			cutt_exit(0);
		}
	}
	return(0);
}

static int play2chat_nd(int n) {
	if (n == 2)
		return(28);
	else if (n == 1 || n == 3 || n == 5 || n == 7 ||
			 n == 8 || n == 10 || n == 12)
		return(31);
	else
		return(30);
}

static void play2chat_comage(int b[], int d[], char *str) {
	int a[3], temp_d_month;

	a[2] = d[2] - b[2];
	a[1] = d[1] - b[1];
	a[0] = d[0] - b[0];
	if (a[1] <= 0) {
		a[2]--;
		a[1] += 12;
	}
	if (a[0] < 0) {
		if (d[1] == b[1])
			temp_d_month = d[1];
		else
			temp_d_month = d[1] - 1;
		if (temp_d_month < 1)
			temp_d_month = 12;
		a[0] += play2chat_nd(temp_d_month);
		if (a[0] < 0)
			a[0] = a[0] + (play2chat_nd(d[1]) - play2chat_nd(temp_d_month));
		a[1]--;
		if (a[1] <= 0) {
			a[2]--;
			a[1] += 12;
		}
	}
	if (a[1] >= 12) {
		a[2]++;
		a[1] -= 12;
	}
	if (str != NULL) {
		if (a[0] == 0 && a[1] == 0 && a[2] == 0)
			str[0] = EOS;
		else if (a[1] < 10 && a[0] < 10)
			sprintf(str,"%d;0%d.0%d",a[2],a[1],a[0]);
		else if (a[1] < 10)
			sprintf(str,"%d;0%d.%d",a[2],a[1],a[0]);
		else if (a[0] < 10)
			sprintf(str,"%d;%d.0%d",a[2],a[1],a[0]);
		else
			sprintf(str,"%d;%d.%d",a[2],a[1],a[0]);
	}
}

static void output_ID(char *ID, long IDln, char *dataFname) {
	int  i, j;
	int a[3], b[3], d[3], age;
	char *name, *fname, *dateS, *BDayS, *ageS, *genderS;
	char str[128], bdate[256], date[256];
	const char *gender;

	strcpy(templineC, "CHI Child, MOT Mother");
	printout("@Participants:", templineC, NULL, NULL, TRUE);
	name = ID;

	fname = ID;
	dateS = NULL;
	BDayS = NULL;
	ageS  = NULL;
	genderS= NULL;
	for (BDayS=fname; *BDayS != ',' && *BDayS != EOS; BDayS++) ;
	if (*BDayS == ',') {
		*BDayS = EOS;
		BDayS++;
		for (dateS=BDayS; *dateS != ',' && *dateS != EOS; dateS++) ;
		if (*dateS == ',') {
			*dateS = EOS;
			dateS++;
			for (ageS=dateS; *ageS != ',' && *ageS != EOS; ageS++) ;
			if (*ageS == ',') {
				*ageS = EOS;
				ageS++;
				for (genderS=ageS; *genderS != ',' && *genderS != EOS; genderS++) ;
				if (*genderS == ',') {
					*genderS = EOS;
					genderS++;
				}
			}
		}
	}
/*
	fprintf(fpout, "ID:\t%s ", name);
	if (fname != NULL)
		fprintf(fpout, "%s ", fname);
	if (dateS != NULL)
		fprintf(fpout, "%s ", dateS);
	if (BDayS != NULL)
		fprintf(fpout, "%s ", BDayS);
	if (ageS != NULL) {
		fprintf(fpout, "%s ", ageS);
	}
	if (genderS != NULL) {
		fprintf(fpout, "%c", *genderS);
	}
	fprintf(fpout, "\n");
*/
	if (genderS != NULL) {
		if (*genderS == 'm')
			gender = "male";
		else if (*genderS == 'f')
			gender = "female";
		else
			gender = "";
	} else
		gender = "";
	if (BDayS != NULL) {
		for (i=0, j=0; isdigit(BDayS[j]); )
			str[i++] = BDayS[j++];
		str[i] = EOS;
		b[1] = atoi(str);
		if (BDayS[j] != '/' && BDayS[j] != '-') {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, IDln);
			fprintf(stderr, "\tBad birthday format\n\n");
			return;
		}

		j++;
		for (i=0; isdigit(BDayS[j]); )
			str[i++] = BDayS[j++];
		str[i] = EOS;
		b[0] = atoi(str);
		if (BDayS[j] != '/' && BDayS[j] != '-') {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, IDln);
			fprintf(stderr, "\tBad birthday format\n\n");
			return;
		}

		j++;
		for (i=0; isdigit(BDayS[j]); )
			str[i++] = BDayS[j++];
		str[i] = EOS;
		b[2] = atoi(str);
		pm(b[1], str);
		if (b[0] < 10) // 14-OCT-2010 from 2014/10/25
			sprintf(bdate, "0%d-%s-%d", b[0], str, b[2]);
		else
			sprintf(bdate, "%d-%s-%d", b[0], str, b[2]);
	}
	if (dateS != NULL) {
		for (i=0, j=0; isdigit(dateS[j]); )
			str[i++] = dateS[j++];
		str[i] = EOS;
		d[1] = atoi(str);
		if (dateS[j] != '/' && dateS[j] != '-') {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, IDln);
			fprintf(stderr, "\tBad date format\n\n");
			return;
		}

		j++;
		for (i=0; isdigit(dateS[j]); )
			str[i++] = dateS[j++];
		str[i] = EOS;
		d[0] = atoi(str);
		if (dateS[j] != '/' && dateS[j] != '-') {
			fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, IDln);
			fprintf(stderr, "\tBad date format\n\n");
			return;
		}

		j++;
		for (i=0; isdigit(dateS[j]); )
			str[i++] = dateS[j++];
		str[i] = EOS;
		d[2] = atoi(str);
		pm(d[1],str);
		if (d[0] < 10) // 18-AUG-2014 from 2016/10/24
			sprintf(date, "0%d-%s-%d", d[0], str, d[2]);
		else
			sprintf(date, "%d-%s-%d", d[0], str, d[2]);
	}
	if (ageS != NULL) {
		age = atoi(ageS);
		if (age != 0) {
			a[0] = age / 12;
			a[1] = age - (a[0] * 12);
			if (a[0] != 0 && a[1] == 0)
				sprintf(str, "%d;", a[0]);
			else
				sprintf(str, "%d;%d.", a[0], a[1]);
		} else
			str[0] = EOS;
	} else
		str[0] = EOS;
	if (str[0] == EOS)
		play2chat_comage(b, d, str);
	fprintf(fpout, "@ID:	eng|%s|CHI|%s|%s|||Child|||\n", name, str, gender);
	fprintf(fpout, "@ID:	eng|%s|MOT|||||Mother|||\n", name);
	fprintf(fpout, "@Birth of %s:	%s\n", "CHI", bdate);
	fprintf(fpout, "@Date:	%s\n", date);
}

static long calculateTime(char *s) {
	char *col, *num, count;
	long time;

	count = 0;
	time = 0L;
	do {
		col = strrchr(s,':');
		if (col != NULL) {
			*col = EOS;
			num = col + 1;
		} else
			num = s;

		if (count == 0) {
			time = atol(num);
		} else if (count == 1) {
			time = time + (atol(num) * 1000);
		} else if (count == 2) {
			time = time + (atol(num) * 60 * 1000);
		} else if (count == 3) {
			time = time + (atol(num) * 3600 * 1000);
		}
		count++;
	} while (col != NULL && count < 4) ;
	return(time);
}

void call() {
	char chatName[TAGSLEN], sp[TAGSLEN], oSp[TAGSLEN], whichTier, whichSpkr, isDotComma;
	char *btS, *etS, *line, *ts;
	char *ID;
	long bt, et, oEt, len, chatNameLen, cnt, ln, IDln;
	char isAddTier;

	ID = NULL;
	chatName[0] = EOS;
	ln = 0L;
	IDln = 0L;
	templineC1[0] = EOS;
    while (fgets_cr(templineC, UTTLINELEN, fpin)) {
    	ln++;
		if (uS.isUTF8(templineC) || templineC[0] == '#')
			continue;
		uS.remblanks(templineC);
		cleanupLine(templineC);
		if (templineC[0] == EOS)
			continue;
		strcat(templineC1, templineC);
		len = strlen(templineC1) - 1;
		if (len < 0 || templineC1[len] != '\\') {
			removeExtraSpace(templineC1);
			strcpy(templineC2, templineC1);
			if (!isdigit(templineC1[0])) {
				for (line=templineC1; !isSpace(*line) && *line != EOS; line++) ;
				if (isSpace(*line))
					*line = EOS;
				strcpy(chatName, templineC1);
				if (uS.mStrnicmp(chatName, "transcribe", 10) == 0) {
					whichTier = SPKRTIER;
				} else if (uS.mStrnicmp(chatName, "gesture", 10) == 0) {
					whichTier = GESTTIER;
				} else {
					whichTier = DEPTTIER;
				}
				templineC1[0] = EOS;
				continue;
			}

			for (btS=templineC1; isSpace(*btS); btS++) ;
			if (*btS == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "File is corrupted\n");
				cutt_exit(0);
			}

			for (etS=btS; *etS != ',' && *etS != EOS; etS++) ;
			if (*etS == ',') {
				*etS = EOS;
				etS++;
			}
			for (; *etS == ','; etS++) ;
			if (*etS == EOS) {
				freeMem();
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "File is corrupted\n");
				cutt_exit(0);
			}

			for (line=etS; *line != ','  && *line != EOS; line++) ;
			if (*line == ',') {
				*line = EOS;
				line++;
			}
			for (; *line == ','; line++) ;

			bt = calculateTime(btS);
			et = calculateTime(etS);

#ifdef UNX
			if (et == 0L) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "WARNING: line's END time = 0.\n");
				fprintf(stderr, "%s\t%s\n", sp+1, templineC2);
			} else if (bt > et) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "WARNING: line's BEG time is greater than END time.\n");
				fprintf(stderr, "%s\t%s\n", sp+1, templineC2);
			}
#else
			if (et == 0L) {
				fprintf(stderr,"%c%c*** File \"%s\": line %ld.%c%c\n", ATTMARKER, error_start, oldfname, ln, ATTMARKER, error_end);
				fprintf(stderr, "%c%cWARNING: line's END time = 0.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
				fprintf(stderr, "%c%c%s\t%s%c%c\n", ATTMARKER, error_start, sp+1, templineC2, ATTMARKER, error_end);
			} else if (bt > et) {
				fprintf(stderr,"%c%c*** File \"%s\": line %ld.%c%c\n", ATTMARKER, error_start, oldfname, ln, ATTMARKER, error_end);
				fprintf(stderr, "%c%cWARNING: line's BEG time is greater than END time.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
				fprintf(stderr, "%c%c%s\t%s%c%c\n", ATTMARKER, error_start, sp+1, templineC2, ATTMARKER, error_end);
			}
#endif
			isAddTier = TRUE;
			if (whichTier == SPKRTIER) {
				if (*line == '(')
					line++;
				if (*line == 'b' || *line == 'c') {
					whichSpkr = 'c';
					strcpy(sp, "*CHI:");
					line++;
				} else if (*line == 'm') {
					whichSpkr = 'm';
					strcpy(sp, "*MOT:");
					line++;
				} else {
					whichSpkr = 'u';
					if (*(line+1) == ',')
						line++;
					strcat(sp, "*ERR:");
				}
				if (*line == ',')
					line++;
				ts = line;
				while (*ts != EOS) {
					if (*ts == ')')
						strcpy(ts, ts+1);
					else
						ts++;
				}
				if (isDebug && strcmp(oSp, sp) == 0 && bt < oEt) {
#ifdef UNX
					fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
					fprintf(stderr, "ERROR: the same speaker's current and previous utterances overlaps.\n");
					fprintf(stderr, "%s\t%s\n", oSp+1, templineC3);
					fprintf(stderr, "%s\t%s\n", sp+1, templineC2);
#else
					fprintf(stderr,"%c%c*** File \"%s\": line %ld.%c%c\n", ATTMARKER, error_start, oldfname, ln, ATTMARKER, error_end);
					fprintf(stderr, "%c%cERROR: the same speaker's current and previous utterances overlaps.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
					fprintf(stderr, "%c%c%s\t%s%c%c\n", ATTMARKER, error_start, oSp+1, templineC3, ATTMARKER, error_end);
					fprintf(stderr, "%c%c%s\t%s%c%c\n", ATTMARKER, error_start, sp+1, templineC2, ATTMARKER, error_end);
#endif
				}
				strcpy(oSp, sp);
				strcpy(templineC3, templineC2);
				oEt = et;
				len = strlen(line) - 1;
				if (len > 2 && line[len] == '/' && line[len-1] == '/' && !uS.IsUtteranceDel(line, len-2)) {
					uS.shiftright(line+len-1, 2);
					line[len-1] = ' ';
					line[len] = '+';
					strcat(line, ".");
				}
			} else if (whichTier == GESTTIER) {
				if (*line == '(')
					line++;
				if (*line == 'b' || *line == 'c') {
					whichSpkr = 'c';
					line++;
				} else if (*line == 'm') {
					whichSpkr = 'm';
					line++;
				} else {
					whichSpkr = 'u';
					if (*(line+1) == ',')
						line++;
				}
				if (*line == ',')
					line++;
				ts = line;
				while (*ts != EOS) {
					if (*ts == ')')
						strcpy(ts, ts+1);
					else
						ts++;
				}
				strcpy(oSp, sp);
				strcpy(templineC3, templineC2);
				oEt = et;
				strcpy(sp, "%");
				strcat(sp, chatName);
				strcat(sp, ":");
			} else {
				chatNameLen = strlen(chatName) - 4;
				if (strcmp(chatName, "reliability_blocks") == 0 || strcmp(chatName+chatNameLen, "_rel") == 0) {
					templineC1[0] = EOS;
					continue;
				}
				strcpy(sp, "%");
				if (strcmp(chatName, "comments") == 0) {
					strcat(sp, "xcom");
					whichSpkr = 'u';
				} else {
					strcat(sp, chatName);
					if (*chatName == 'b' || *chatName == 'c') {
						whichSpkr = 'c';
					} else if (*chatName == 'm') {
						whichSpkr = 'm';
					} else {
						whichSpkr = 'u';
					}
				}
				if (strcmp(chatName, "momemo")==0 && *line=='(' && *(line+1)=='.' && *(line+2)==')')
					isAddTier = FALSE;
				strcat(sp, ":");
				if (*line == '(' && *(line+1) == '.' && *(line+2) == ')') {
					*line = 'o';
					*(line+1) = 'f';
					*(line+2) = 'f';
				}
				if (*line == '(') {
					line++;
					if (*line == '.')
						line++;
				}
				isDotComma = FALSE;
				ts = line;
				while (*ts != EOS) {
					if (*ts == '.' && *(ts+1) == ',') {
						isDotComma = TRUE;
						strcpy(ts, ts+2);
					} else if (isDotComma && *ts == '.' && *(ts+1) == ')') {
						strcpy(ts, ts+2);
					} else if (*ts == ')') {
						strcpy(ts, ts+1);
					} else
						ts++;
				}
				if (isDotComma) {
					cnt = 0L;
					ts = line;
					while (*ts != EOS) {
						if (isalnum(*ts))
							cnt++;
						if (*ts == ',') {
							strcpy(ts, ts+1);
						} else
							ts++;
					}
					if (cnt > 1) {
						freeMem();
						fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
						fprintf(stderr, "ERROR: line of \"%s\" has more than one code:\n", sp+1);
						fprintf(stderr, "%s\n", templineC2);
						cutt_exit(0);
					}
				}
			}
			if (uS.mStricmp(sp, "%id:") == 0 || uS.mStricmp(sp, "%PLAY_id:") == 0) {
				IDln = ln;
				ID = (char *)malloc(strlen(line)+1);
				if (ID  != NULL)
					strcpy(ID, line);
			} else if (line[0] == EOS) {
				fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "WARNING: line does not have any code.\n");
				fprintf(stderr, "%s\t%s\n", sp+1, templineC2);
			} else if (isAddTier)
				linesRoot = pc_add_each_line(linesRoot, 0, whichTier, whichSpkr, sp, line, bt, et);
			templineC1[0] = EOS;
		} else
			templineC1[len] = ' ';
	}
	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	eng\n");
	if (ID == NULL) {
		fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
		fprintf(stderr, "ERROR: Can't find \"id ...\" line.\n");
	} else {
		output_ID(ID, IDln, oldfname);
		free(ID);
	}
	strcpy(templineC3, newfname);
	ts = strrchr(templineC3, '.');
	if (ts != NULL)
		*ts = EOS;
	fprintf(fpout,"@Media:	%s, audio\n", templineC3);
	pc_printTiers(linesRoot, 0L, 0L);
	fprintf(fpout, "@End\n");
	linesRoot = freeLines(linesRoot);
}
