/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3
#include "cu.h"

#if !defined(UNX)
#define _main dates_main
#define call dates_call
#define getflag dates_getflag
#define init dates_init
#define usage dates_usage
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

extern char OverWriteFile;

#define CHILD struct child
CHILD {
	char isUsed;
	char name[80];
	int a[3], b[3];
	char ga, /* age of child is given by the user */
		 gb; /* birth of child is given by the user */
	CHILD *next;
} *dates_head;

int d[3];
char gd, datePrinted;

/* ******************** dates prototypes ********************** */
/* *********************************************************** */

void usage() {
	printf("Usage: dates [+a S +b S +d S] filename(s)\n");
	puts("+a S: specify age of a child (year(s);month(s).day(s))");
	puts("+b S: specify birth of a child (12-JAN-1962) or (01/12/62)");
	puts("+d S: specify date of transcript (12-JAN-1962) or (01/12/62)");
	puts("\nExample: dates +a2;3.1 +b12-jan-1962");
	puts("           dates +b 31-aug-1963 +d 30-jul-1964");
	puts("           dates +b 08/31/63 +d 07/30/1964");
	puts("           dates +b 08/31/63 +d 07/30/1964 sample.cha");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		d[0] = 0; d[1] = 0; d[2] = 0;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		onlydata = 1;
		dates_head = NULL;
		gd = FALSE;
	}
	datePrinted = FALSE;
}

static CHILD *freeDates(CHILD *p) {
	CHILD *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		free(t);
	}
	return(NULL);
}

static int nd(int n) {
	if (n == 2)
		return(28);
	else if (n == 1 || n == 3 || n == 5 || n == 7 || 
			 n == 8 || n == 10 || n == 12)
		return(31);
	else
		return(30);
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

static void combirth(CHILD *p) {
	char code[128];

	p->b[2] = d[2] - p->a[2];
	p->b[1] = d[1] - p->a[1];
	p->b[0] = d[0] - p->a[0];
	if (p->b[1] <= 0) {
		p->b[2]--;
		p->b[1] += 12;
	}
	if (p->b[0] <= 0) {
		p->b[1]--;
		if (p->b[1] <= 0) {
			p->b[2]--;
			p->b[1] += 12;
		}
		if (p->b[1] < 1)
			p->b[0] += nd(12);
		else
			p->b[0] += nd(p->b[1]);
	}
	pm(p->b[1],code);
	if (p->b[0] < 10)
		fprintf(fpout,"@Birth of %s:	0%d-%s-%d\n",p->name,p->b[0],code,p->b[2]);
	else
		fprintf(fpout,"@Birth of %s:	%d-%s-%d\n",p->name,p->b[0],code,p->b[2]);
}

static void comdate(CHILD *p) {
	char code[128];

	if (datePrinted)
		return;

	d[2] = p->a[2] + p->b[2];
	d[1] = p->a[1] + p->b[1];
	d[0] = p->a[0] + p->b[0];
	while (d[1] > 12) {
		d[2]++;
		d[1] -= 12;
	}
	if (d[0] > nd(d[1])) {
		d[0] -= nd(d[1]);
		d[1]++;
		if (d[1] > 12) {
			d[1] -= 12;
			d[2]++;
		}
	}
	pm(d[1],code);
	if (d[0] < 10)
		fprintf(fpout,"@Date:	0%d-%s-%d\n",d[0],code,d[2]);
	else
		fprintf(fpout,"@Date:	%d-%s-%d\n",d[0],code,d[2]);
	datePrinted = TRUE;
}

static void comage(CHILD *p, char *st) {
	register int temp_d_month;

	p->a[2] = d[2] - p->b[2];
	p->a[1] = d[1] - p->b[1];
	p->a[0] = d[0] - p->b[0];
	if (p->a[1] <= 0) {
		p->a[2]--;
		p->a[1] += 12;
	}
	if (p->a[0] < 0) {
		if (d[1] == p->b[1])
			temp_d_month = d[1];
		else
			temp_d_month = d[1] - 1;
		if (temp_d_month < 1)
			temp_d_month = 12;
		p->a[0] += nd(temp_d_month);
		if (p->a[0] < 0)
			p->a[0] = p->a[0] + (nd(d[1]) - nd(temp_d_month));
		p->a[1]--;
		if (p->a[1] <= 0) {
			p->a[2]--;
			p->a[1] += 12;
		}
	}
	if (p->a[1] >= 12) {
		p->a[2]++;
		p->a[1] -= 12;
	}
	if (st != NULL) {
		if (p->a[0] < 10)
			sprintf(st,"%d;%d.0%d",p->a[2],p->a[1],p->a[0]);
		else
			sprintf(st,"%d;%d.%d",p->a[2],p->a[1],p->a[0]);
	} else {
		if (p->a[0] < 10)
			fprintf(fpout, "@Age of %s:	%d;%d.0%d\n",p->name,p->a[2],p->a[1],p->a[0]);
		else							
			fprintf(fpout, "@Age of %s:	%d;%d.%d\n",p->name,p->a[2],p->a[1],p->a[0]);
	}
}

static void changeToNewID(char *sp, char *line) {
	int i, j, k, bars;
	char  spC[SPEAKERLEN];
	CHILD *p;

	for (p=dates_head; p != NULL; p=p->next) {
		bars = 0;
		j = 0;
		k = 0;
		for (i=0; line[i] != EOS; i++) {
			if (bars == 2 && line[i] != '|') {
				spC[k++] = line[i];
			}
			if (bars != 3)
				templineC3[j++] = line[i];
			if (line[i] == '|')
				bars++;
			if (bars == 4) {
				break;
			}
		}
		spC[k] = EOS;
		if (uS.mStricmp(spC, p->name) == 0)
			break;
	}
	if (p == NULL || p->isUsed == TRUE)
		printout(sp,line,NULL,NULL,FALSE);
	else {
		templineC3[j] = EOS;
		if (p->ga) {
			if (p->a[0] < 10)
				sprintf(templineC3+j, "%d;%d.0%d", p->a[2], p->a[1], p->a[0]);
			else
				sprintf(templineC3+j, "%d;%d.%d", p->a[2], p->a[1], p->a[0]);
		}
		else if (p->gb && gd)
			comage(p, templineC3+j);
		j = strlen(templineC3);
		for (; line[i] != EOS; i++)
			templineC3[j++] = line[i];
		templineC3[j] = EOS;

		printout(sp,templineC3,NULL,NULL,FALSE);
		p->isUsed = TRUE;
	}
}

void call() {
	char isFirstSp;
	char code[128];
	CHILD *p;

	for (p=dates_head; p != NULL; p=p->next)
		p->isUsed = FALSE;

	isFirstSp = TRUE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE) && dates_head != NULL) {
			changeToNewID(utterance->speaker, utterance->line);
		} else if (uS.partcmp(utterance->speaker,"@Date:",FALSE,FALSE)) {
		} else if (utterance->speaker[0] == '*' && isFirstSp) {
			isFirstSp = FALSE;
			if (dates_head != NULL) {
				for (p=dates_head; p != NULL; p=p->next) {
					if (p->isUsed == TRUE) {
/*
						if (p->gb) {
							pm(p->b[1],code);
							if (p->b[0] < 10)
								fprintf(fpout,"@Birth of %s:	0%d-%s-%d\n",p->name,p->b[0],code,p->b[2]);
							else
								fprintf(fpout,"@Birth of %s:	%d-%s-%d\n",p->name,p->b[0],code,p->b[2]);
						}
*/
/*
						if (!p->gb && p->ga && gd)
							combirth(p);
*/
						if (!gd && p->gb && p->ga)
							comdate(p);
					} else {
						if (p->ga) {
							if (p->a[0] < 10)
								fprintf(fpout, "@Age of %s:	%d;%d.0%d\n",p->name,p->a[2],p->a[1],p->a[0]);
							else							
								fprintf(fpout, "@Age of %s:	%d;%d.%d\n",p->name,p->a[2],p->a[1],p->a[0]);
						}
						if (p->gb) {
							pm(p->b[1],code);
							if (p->b[0] < 10)
								fprintf(fpout,"@Birth of %s:	0%d-%s-%d\n",p->name,p->b[0],code,p->b[2]);
							else
								fprintf(fpout,"@Birth of %s:	%d-%s-%d\n",p->name,p->b[0],code,p->b[2]);
						}
						if (!p->ga && p->gb && gd)
							comage(p, NULL);
						else if (!p->gb && p->ga && gd)
							combirth(p);
						else if (!gd && p->gb && p->ga)
							comdate(p);
						p->isUsed = TRUE;
					}
				}
			}
			if (gd && !datePrinted) {
				pm(d[1],code);
				if (d[0] < 10)
					fprintf(fpout,"@Date:	0%d-%s-%d\n",d[0],code,d[2]);
				else
					fprintf(fpout,"@Date:	%d-%s-%d\n",d[0],code,d[2]);
				datePrinted = TRUE;
			}
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
}

static void singleCall(int argc, char *argv[]) {
	int   i;
	char code[128];
	CHILD *p;

	mmaininit();
	init(TRUE);
	fpout = stdout;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-') 
			getflag(argv[i],argv[i+1],&i);
	}		

	for (p=dates_head; p != NULL; p=p->next)
		p->isUsed = FALSE;

	p = dates_head;
	if (p == NULL) {
		fputs("No age or birth specified in the right format.\n",stderr);
	} else {
		while (p != NULL) {
			if (p->ga) {
				if (p->a[0] < 10)
					fprintf(fpout, "@Age of %s:	%d;%d.0%d\n",p->name,p->a[2],p->a[1],p->a[0]);
				else							
					fprintf(fpout, "@Age of %s:	%d;%d.%d\n",p->name,p->a[2],p->a[1],p->a[0]);
			}
			if (p->gb) {
				pm(p->b[1],code);
				if (p->b[0] < 10)
					fprintf(fpout,"@Birth of %s:	0%d-%s-%d\n",p->name,p->b[0],code,p->b[2]);
				else
					fprintf(fpout,"@Birth of %s:	%d-%s-%d\n",p->name,p->b[0],code,p->b[2]);
			}
			if (!p->ga) {
				if (p->gb && gd)
					comage(p, NULL);
				else
					fprintf(stderr,"Need birthday or date to compute age of %s.\n",p->name);
			}
			if (!p->gb) {
				if (p->ga && gd)
					combirth(p);
				else
					fprintf(stderr,"Need age or date to compute birth of %s.\n",p->name);
			}
			if (!gd) {
				if (p->gb && p->ga)
					comdate(p);
				else
					fprintf(stderr,"Need birthday or age to compute date.\n");
			}
			p = p->next;
		}
		if (gd && !datePrinted) {
			pm(d[1],code);
			if (d[0] < 10)
				fprintf(fpout,"@Date:	0%d-%s-%d\n",d[0],code,d[2]);
			else
				fprintf(fpout,"@Date:	%d-%s-%d\n",d[0],code,d[2]);
			datePrinted = TRUE;
		}
	}
	main_cleanup();
}

static CHILD *findchild(const char *st) {
	char cn[SPEAKERLEN];
	CHILD *p;

	if (*st == EOS)
		strcpy(cn,"CHI");
	else
		strcpy(cn, st);
	uS.remblanks(cn);
	uS.uppercasestr(cn, &dFnt, MBF);
	p = dates_head;
	if (p == NULL) {
		p = NEW(CHILD);
		strcpy(p->name,cn);
		p->next = NULL;
		p->ga = FALSE;
		p->gb = FALSE;
		p->a[0] = 0; p->a[1] = 0; p->a[2] = 0;
		p->b[0] = 0; p->b[1] = 0; p->b[2] = 0;
		dates_head = p;
	} else {
		while (p->next != NULL && strcmp(p->name,cn))
			p = p->next;
		if (strcmp(p->name,cn)) {
			p->next = NEW(CHILD);
			p = p->next;
			strcpy(p->name,cn);
			p->next = NULL;
			p->ga = FALSE;
			p->gb = FALSE;
			p->a[0] = 0; p->a[1] = 0; p->a[2] = 0;
			p->b[0] = 0; p->b[1] = 0; p->b[2] = 0;
		}
	}
	return(p);
}


static void getage(char *s, CHILD *p) {	
	int i, j;
	char str[20];

	for (i=0, j=0; isdigit(s[j]); )
		str[i++] = s[j++];
	str[i] = EOS;
	p->a[2] = atoi(str);
	for (i=0, j++; isdigit(s[j]); )
		str[i++] = s[j++];
	str[i] = EOS;
	p->a[1] = atoi(str);
	for (i=0, j++; isdigit(s[j]); )
		str[i++] = s[j++];
	str[i] = EOS;
	p->a[0] = atoi(str);
	if (p->a[0] == 0 && p->a[1] == 0 && p->a[2] == 0)
		p->ga = FALSE;
	else {
		p->ga = TRUE;
		if (p->a[0] > 31) {
			i = p->a[0] / 31;
			p->a[1] += i;
			i = i * 31;
			p->a[0] -= i;
		}
		if (p->a[1] > 12) {
			i = p->a[1] / 12;
			p->a[2] += i;
			i = i * 12;
			p->a[1] -= i;
		}
	}
}

static void getbirth(char *s, CHILD *p) {
	int i, j;
	char str[128];

	for (i=0, j=0; isdigit(s[j]); ) 
		str[i++] = s[j++];
	str[i] = EOS;
	p->b[0] = atoi(str);
	j++;
	if (isdigit(s[j])) {
		p->b[1] = p->b[0];
		for (i=0; isdigit(s[j]); )
			str[i++] = s[j++];
		str[i] = EOS;
		p->b[0] = atoi(str);
	} else {
		str[0] = s[j++];
		str[1] = s[j++];
		str[2] = s[j++];
		str[3] = EOS;
		p->b[1] = pm(0,str);
	}
	for (i=0, j++; isdigit(s[j]); )
		str[i++] = s[j++];
	str[i] = EOS;
	p->b[2] = atoi(str);
	if (p->b[2] < 100)
		p->b[2] += 1900;
	if (p->b[0] == 0 || p->b[1] == 0 || p->b[2] == 0) {
		p->gb = FALSE;
		p->b[0] = 0; p->b[1] = 0; p->b[2] = 0;
	} else {
		p->gb = TRUE;
	}
}

static void getdate(char *s) {
	int i,j;
	char str[128];

	for (i=0, j=0; isdigit(s[j]); )
		str[i++] = s[j++];
	str[i] = EOS;
	d[0] = atoi(str);
	j++;
	if (isdigit(s[j])) {
		i = 0;
		d[1] = d[0];
		for (i=0; isdigit(s[j]); )
			str[i++] = s[j++];
		str[i] = EOS;
		d[0] = atoi(str);
	} else {
		str[0] = s[j++];
		str[1] = s[j++];
		str[2] = s[j++];
		str[3] = EOS;
		d[1] = pm(0,str);
	}
	for (i=0, j++; isdigit(s[j]); )
		str[i++] = s[j++];
	str[i] = EOS;
	d[2] = atoi(str);
	if (d[2] < 100)
		d[2] += 1900;
	if (d[0] == 0 || d[1] == 0 || d[2] == 0) {
		gd = FALSE;
		d[0] = 0; d[1] = 0; d[2] = 0;
	} else {
		gd = TRUE;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int   i;
	char isFileFound;
	extern void VersionNumber(char isshortfrmt, FILE *fp);

	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = DATES;
	UttlineEqUtterance = TRUE;

	if (argc < 2)
		usage();
	if (argc > 1) {
		if (argv[1][0] == '-' || argv[1][0] == '+') {
			if (argv[1][1] == 'v') {
				if (argv[1][2] == EOS) {
					VersionNumber(FALSE, stdout);
					cutt_exit(0);
				}
			}
		}
	}

	isFileFound = FALSE;
	for (i=1; i < argc; i++) {
		if (*argv[i] == '+'  || *argv[i] == '-')  {
			if (argv[i][1] == 'a' || argv[i][1] == 'b' || argv[i][1] == 'd') {
				if (!isdigit(argv[i][2]))
					i++;
			}
		} else
			isFileFound = TRUE;
	}		
	if (isFileFound)
		bmain(argc,argv,NULL);
	else
		singleCall(argc, argv);
	dates_head = freeDates(dates_head);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++)
	{
		case 'a':
			if (isdigit(*f)) {
				getage(f,findchild("CHI"));
			} else {
				getage(f1,findchild(f));
				(*i)++;
			}
			break;
		case 'b':
			if (isdigit(*f)) {
				getbirth(f,findchild("CHI"));
			} else {
				getbirth(f1,findchild(f));
				(*i)++;
			}
			break;
		case 'd':
			if (isdigit(*f)) {
				getdate(f);
			} else {
				if (*f1 == '-' || *f1 == '+') { 
					puts("Bad date format"); 
					exit(0);
				}
				getdate(f1);
				(*i)++;
			}
			break;
		default:
			fprintf(stderr,"Invalid option: %s\n", f-1);
			break;
	}
}
