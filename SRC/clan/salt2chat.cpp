/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/*
-cc:
1. Delete the codes- [$ g], [$ i]
2. animal-s new powerranger-s. [$ eu] ==> <animal-s new powerranger-s> [*]. (Whole utterance)
3. where [$ ew:what] ^'s this?        ==> <where> [: what] [*] ^'s this?
4. where [$ ew] ^'s this?             ==> <where> [*] ^'s this.
5. Delete "@Time Start:" and "%tim:"
*/

/*

 letread

*/

#define CHAT_MODE 0

#include "cu.h"

#define DispChanges 1

#if !defined(UNX)
#define _main salt2chat_main
#define call salt2chat_call
#define getflag salt2chat_getflag
#define init salt2chat_init
#define usage salt2chat_usage
#endif

#define pr_result salt2chat_pr_result

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define LINE_LEN 1500

extern char OverWriteFile;

struct scodes {
	char *scode;
	struct scodes *snext;
} ;
static struct scodes *shead;

struct codes {
	char *code;
	char *map;
	struct codes *nextMap;
} ;
static struct codes *headCode;

struct depTiers {
	char tier[4];
	char line[LINE_LEN];
	struct depTiers *nextTier;
} ;
static struct depTiers *headTier;

static int  pos;
static int  coding;
static long lnum;
static char name[10][10];
static char *tim;
static char *salt2chat_templine;
static char *lcode;
static char *comm;
static char *comment;
static char *def_line;
static char *salt2chat_pcodes;
static char curspeaker;
static char brerror;
//static char letread;
static char overlap;
static char oldsp;
static char isSameLine;
static char deff;
static char SpFound;
static char hel;
static char uselcode;

static int flag;
static int mmax = 21;
static const char *card[] =
{
	"hundred",
	"thousand",
	"million",
	"billion",
	"trillion",
	"quadrillion",
	"quintillion",
	"sextillion",
	"septillion",
	"octillion",
	"nonillion",
	"decillion",
	"undecillion",
	"duodecillion",
	"tredecillion",
	"quattuordecillion",
	"quindecillion",
	"sexdecillion",
	"septendecillion",
	"octodecillion",
	"novemdecillion",
	"vigintillion"
};
static const char *unit[] = {
	"zero",
	"one",
	"two",
	"three",
	"four",
	"five",
	"six",
	"seven",
	"eight",
	"nine"
};
static const char *teen[] = {
	"ten",
	"eleven",
	"twelve",
	"thirteen",
	"fourteen",
	"fifteen",
	"sixteen",
	"seventeen",
	"eighteen",
	"nineteen"
};
static const char *decade[] = {
	"zero",
	"ten",
	"twenty",
	"thirty",
	"forty",
	"fifty",
	"sixty",
	"seventy",
	"eighty",
	"ninety"
};
static char tline[100];

static void map_cleanup(void) {
	struct codes *t;
	
	while (headCode != NULL) {
		t = headCode;
		headCode = headCode->nextMap;
		if (t->code)
			free(t->code);
		if (t->map)
			free(t->map);
		free(t);
	}
}

static void tier_cleanup(void) {
	struct depTiers *t;
	
	while (headTier != NULL) {
		t = headTier;
		headTier = headTier->nextTier;
		free(t);
	}
}

static void salt2chat_clean_up(void) {
	map_cleanup();
	tier_cleanup();
	if (tim)
		free(tim);
	if (comm)
		free(comm);
	if (lcode)
		free(lcode);
	if (comment)
		free(comment);
	if (salt2chat_templine)
		free(salt2chat_templine);
	if (def_line)
		free(def_line);
	if (salt2chat_pcodes)
		free(salt2chat_pcodes);
	tim = NULL;
	comm = NULL;
	lcode = NULL;
	comment = NULL;
	salt2chat_templine = NULL;
	def_line = NULL;
	salt2chat_pcodes = NULL;
}

static void clearcodes() {
	struct scodes *temp;
	
	while (shead != NULL) {
		temp = shead;
		shead = shead->snext;
		free(temp->scode);
		free(temp);
	}
	shead = NULL;
}

static void addNewCod(char *code) {
	struct depTiers *dt;

	if (headTier == NULL) {
		headTier = NEW(struct depTiers);
		dt = headTier;
	} else {
		for (dt=headTier; dt->nextTier != NULL; dt=dt->nextTier) {
			if (strncmp(dt->tier, code, 3) == 0)
				return;
		}
		if (strncmp(dt->tier, code, 3) == 0)
			return;
		dt->nextTier = NEW(struct depTiers);
		dt = dt->nextTier;
	}
	if (dt == NULL) {
		salt2chat_clean_up();
		clearcodes();
		fprintf(stderr,"salt2chat: no more memory available.\n");
		cutt_exit(0);
	}
	strncpy(dt->tier, code, 3);
	dt->tier[3] = EOS;
	dt->line[0] = EOS;
	dt->nextTier = NULL;
}

void init(char f) {
	int i;
	struct codes *t;
	struct depTiers *dt;

	if (f) {
		AddCEXExtension = ".cha";
		OverWriteFile = TRUE;
		*utterance->speaker = '*';
		stout = FALSE;
		onlydata = 3;
		// defined in "mmaininit" and "globinit" - nomap = TRUE;
		hel = FALSE;
		uselcode = FALSE;
		coding = 0;
		tim = NULL;
		comm = NULL;
		lcode = NULL;
		comment = NULL;
		salt2chat_templine = NULL;
		def_line = NULL;
		salt2chat_pcodes = NULL;
		headCode = NULL;
		headTier = NULL;
		if ((tim=(char *)malloc(LINE_LEN)) == NULL) out_of_mem();
		if ((salt2chat_templine=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); out_of_mem();
		}
		if ((lcode=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(salt2chat_templine); out_of_mem();
		}
		if ((comm=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(salt2chat_templine); free(lcode); out_of_mem();
		}
		if ((comment=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(salt2chat_templine); free(lcode); free(comm); out_of_mem();
		}
		if ((salt2chat_pcodes=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(salt2chat_templine); free(lcode); free(comm); free(comment); out_of_mem();
		}
		if ((def_line=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(salt2chat_templine); free(lcode); free(comm); free(comment); free(salt2chat_pcodes);
			out_of_mem();
		}
	} else {
		pos = 0;
		oldsp = 0;
		overlap = 0;
		deff = FALSE;
		shead = NULL;
		brerror = FALSE;
		for (i=0; i < 10; i++)
			*name[i] = EOS;
		*utterance->line = EOS;
		if (uselcode && headTier == NULL) {
			for (t=headCode; t != NULL; t=t->nextMap) {
				addNewCod(t->map);
			}
		}
		if (coding == 3 || coding == 5)
			nomap = TRUE;
		for (dt=headTier; dt != NULL; dt=dt->nextTier) {
			dt->line[0] = EOS;
		}
		if (tim)
			tim[0] = EOS;
		if (comm)
			comm[0] = EOS;
		if (lcode)
			lcode[0] = EOS;
		if (comment)
			comment[0] = EOS;
		if (salt2chat_templine)
			salt2chat_templine[0] = EOS;
		if (def_line)
			def_line[0] = EOS;
		if (salt2chat_pcodes)
			salt2chat_pcodes[0] = EOS;
	}
}

void usage() {
	puts("SALT2CHAT changes a file from SALT to CHAT.");
	printf("Usage: salt2chat [cS h l %s] filename(s)\n", mainflgs());
	puts("+cS: use special coding system");
	puts("     a- Alison, b- Bulgarian, c- Chris, g- Ron Gillam, h- Hadley,");
	puts("     p- Peter Cahill, r- Richardson, s- Sophie");
	puts("+h : handle \"<...>\" as [% ...] (default: <...> [\"overlap\"])");
	puts("+lF: put codes on separate tier; use mapping file F to determine tier name.");
	mainusage(TRUE);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = SALT2CHAT;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	salt2chat_clean_up();
	clearcodes();
}

static struct codes *makenewMap(char *st) {
	struct codes *nextone;

	if (headCode == NULL) {
		headCode = NEW(struct codes);
		nextone = headCode;
	} else {
		nextone = headCode;
		while (nextone->nextMap != NULL)
			nextone = nextone->nextMap;
		nextone->nextMap = NEW(struct codes);
		nextone = nextone->nextMap;
	}
	if (nextone == NULL) {
		salt2chat_clean_up();
		clearcodes();
		fprintf(stderr,"salt2chat: no more memory available.\n");
		cutt_exit(0);
	}
	nextone->code = (char *)malloc(strlen(st)+1);
	if (nextone->code == NULL) {
		salt2chat_clean_up();
		clearcodes();
		fprintf(stderr,"salt2chat: no more memory available.\n");
		cutt_exit(0);
	}
	strcpy(nextone->code,st);
	uS.uppercasestr(nextone->code, &dFnt, MBF);
	nextone->nextMap = NULL;
	return(nextone);
}

static char *FindRightLine(char *code) {
	struct codes *t;
	struct depTiers *tt;

	strcpy(templineC1, code);
	uS.uppercasestr(templineC1, &dFnt, MBF);
	for (t=headCode; t != NULL; t=t->nextMap) {
		if (strcmp(t->code, templineC1) == 0) {
			for (tt=headTier; tt != NULL; tt=tt->nextTier) {
				if (strcmp(t->map, tt->tier) == 0)
					return(tt->line);
			}
		}
	}
	return(lcode);
}

static void readmap(FNType *fname) {
	FILE *fdic;
	char chrs, word[BUFSIZ], first = TRUE, term;
	int index = 0;
	long l = 1L;
	struct codes *nextone = NULL;
	FNType mFileName[FNSize];

	if ((fdic=OpenGenLib(fname,"r",TRUE,TRUE,mFileName)) == NULL) {
		fputs("Can't open either one of the mapping files:\n",stderr);
		fprintf(stderr,"\t\"%s\", \"%s\"\n", fname, mFileName);
		cutt_exit(0);
	}
	while ((chrs=(char)getc_cr(fdic, NULL)) != '"' && chrs!= '\'' && !feof(fdic)) {
		if (chrs == '\n')
			l++;
	}
	term = chrs;
	while (!feof(fdic)) {
		chrs = (char)getc_cr(fdic, NULL);
		if (chrs == term) {
			word[index] = EOS;
			uS.lowercasestr(word, &dFnt, C_MBF);
			if (first) {
				if (*word == '$')
					strcpy(word, word+1);
				nextone = makenewMap(word);
				if (DispChanges)
					fprintf(stderr,"Map code \"$%s\" to tier ",nextone->code);
				first = FALSE;
				nextone->map = NULL;
			} else if (nextone != NULL) {
				if (*word == '%')
					strcpy(word, word+1);
				nextone->map = (char *)malloc(strlen(word)+1);
				if (nextone->map == NULL) {
					salt2chat_clean_up();
					clearcodes();
					fprintf(stderr,"salt2chat: no more memory available.\n");
					cutt_exit(0);
				}
				strcpy(nextone->map, word);
				if (DispChanges) {
					if (strcmp(nextone->map, "*") == 0)
						fprintf(stderr,"\"%s\"\n",nextone->map);
					else
						fprintf(stderr,"\"%%%s\"\n",nextone->map);
				}
				first = TRUE;
			}
			word[0] = EOS;
			index = 0;
			while ((chrs=(char)getc_cr(fdic, NULL)) != '"' && chrs != '\'' && !feof(fdic)) {
				if (chrs == '\n')
					l++;
			}
			term = chrs;
		} else if (chrs != '\n')
			word[index++] = chrs;
		else {
			fflush(stdout);
			fprintf(stderr,"*** File \"%s\": line %ld\n", fname, l);
			fprintf(stderr, "ERROR: Found newline in string on line %ld in file %s.\n",l,fname);
			fclose(fdic);
			salt2chat_clean_up();
			clearcodes();
			cutt_exit(0);
		}
	}
	fclose(fdic);
	if (DispChanges)
		fprintf(stderr,"\n");
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++)
	{
		case 'c':
			if (*f == 'a' || *f == 'A') {
				coding = 1;
			} else if (*f == 'c' || *f == 'C') {
				coding = 2;
			} else if (*f == 's' || *f == 'S') {
				coding = 3;
			} else if (*f == 'g' || *f == 'G') {
				coding = 4;
			} else if (*f == 'p' || *f == 'P') {
				coding = 5;
			} else if (*f == 'h' || *f == 'H') {
				coding = 6;
			} else if (*f == 'r' || *f == 'R') {
				coding = 7;
			} else if (*f == 'b' || *f == 'B') {
				coding = 8;
			} else {
				fprintf(stderr, "Please choose one for +c option: 'a', 'b', 'c', 'g', 'h, 'p', 'r' or 's'\n");
				salt2chat_clean_up();
				clearcodes();
				cutt_exit(0);
			}
			break;
		case 'h':
			hel = TRUE;
			no_arg_option(f);
			break;
		case 'l':
			uselcode = TRUE;
			if (*f != EOS)
				readmap(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static char *findsp(char c) {
	int i;

	for (i=0; name[i][0] != EOS && i < 10; i++) {
		if (toupper((unsigned char)c) == toupper((unsigned char)name[i][0]))
			return(name[i]+1);
	}
	return(NULL);
}

static void cleancom(char *st) {
	char isRemCurly;

	isRemCurly = FALSE;
	while (*st) {
		if (*st == '{' && *(st+1) == '%' && *(st+2) == ' ' && *(st+3) == '(') {
			strcpy(st,st+3);
			isRemCurly = TRUE;
		} else if (*st == '}' && isRemCurly) {
			isRemCurly = FALSE;
			strcpy(st,st+1);
		} else if (*st == '[') {
			if (*(st+1) == '%' || *(st+1) == '^')
				strcpy(st,st+2);
			else
				strcpy(st,st+1);
		} else if (*st == ']'/* || (*st == '.' && coding != 6)*/)
			strcpy(st,st+1);
		else
			st++;
	}
}

static void remJunkBack(char *st) {
	register int i;

	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == ',' || st[i] == NL_C || st[i] == SNL_C)) i--;
	st[i+1] = EOS;
}

static void remJunkFrontAndBack(char *st) {
	register int i;

	for (i=0; isSpace(st[i]) || st[i] == '\n' || st[i] == ','; i++) ;
	if (i > 0)
		strcpy(st, st+i);
	remJunkBack(st);
}

static void moveDelim(char *st) {
	char delim, sbr;

	delim = *st;
	*st = ' ';
	for (; *st != EOS && *st != '[' && *st != '{'; st++) ;
	if (*st == EOS)
		return;
	if (*st == '[')
		sbr = ']';
	else if (*st == '{')
		sbr = '}';
	else
		return;
	for (; *st != EOS && *st != sbr; st++) ;
	if (*st == EOS)
		return;
	uS.shiftright(st+1, 3);
	st[1] = ' ';
	st[2] = delim;
	st[3] = ' ';
}

static int checkline(char *line) {
	int  i, s;
	char found, cb, UTDfound;

	cb = FALSE;
	found = FALSE;
	UTDfound = FALSE;
	for (i=0; line[i] != EOS; i++) {
		if (uS.IsUtteranceDel(line, i)) {
			UTDfound = TRUE;
			if (line[i+1] == ' ' && (line[i+2] == '[' || line[i+2] == '{')) {
				moveDelim(line+i);
				removeExtraSpace(line+i);
			}
		}
		if (line[i] == '[') {
			if (line[i+1] == '%') {
				if (i == 0)
					line[i+1] = '^';
				else if (isSpace(line[i+2]) && isalpha(line[i+3]) && line[i+4] == ']')
					line[i+1] = '^';
			}
		} else if (line[i] == '{') {
			if (coding == 6) {
				found = TRUE;
				line[i++] = ' ';
				if (line[i] == '%')
					line[i++] = ' ';
				while (1) {
					for (; uS.isskip(line,i,&dFnt,MBF) && line[i] != '}' && line[i] != '[' && line[i] != EOS; i++) ;
					if (line[i] == EOS || line[i] == '}')
						break;
					if (line[i] == '[') {
						for (; line[i] != ']' && line[i] != EOS; i++) ;
						if (line[i] == EOS)
							break;
						i++;
					} else {
						s = i;
						for (; !uS.isskip(line,i,&dFnt,MBF) && line[i] != '}' && line[i] != '[' && line[i] != EOS; i++) ;
						for (; isSpace(line[i]) || line[i] == '\n'; i++) ;
						if (line[i] != '[') {
							uS.shiftright(line+s, 1);
							line[s] = '&';
							i++;
						}
						if (line[i] == EOS)
							break;
					}
				}
				if (line[i] == '}')
					line[i] = ' ';
			} else {
				if (isalpha(line[i+1])) {
					uS.shiftright(line+i+1, 2);
					line[i+1] = '%';
					line[i+2] = ' ';
				} else if (line[i+1] == '%') {
					if (i == 0)
						line[i+1] = '^';
					else if (isSpace(line[i+2]) && isalpha(line[i+3]) && line[i+4] == ']')
						line[i+1] = '^';
				}
				cb = TRUE;
				line[i] = '[';
			}
		} else if (line[i] == '}') {
			cb = FALSE;
			line[i] = ']';
		} else {
			if (UTF8_IS_SINGLE((unsigned char)line[i]))
				found = TRUE;
			if (isalnum((unsigned char)line[i]) && !cb)
				found = TRUE;
		}
	}
	if (!UTDfound)
		strcat(line, ".");
	uS.remFrontAndBackBlanks(line);
	removeExtraSpace(line);
	if (found)
		return(FALSE);
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '[' && (line[i+1] == '<' || line[i+1] == '>') && (line[i+2] == ']' || isSpace(line[i+2]))) {
		} else if (line[i] == '[') {
			for (s=i+1; line[s] != EOS && line[s] != '[' && line[s] != ']' && line[s] != '<' && line[s] != '>'; s++) ;
			if (line[s] == ']') {
				strcpy(line+s,line+s+1);
				strcpy(line+i,line+i+3);
			}
		}
	}
	return(TRUE);
}

static void fixuptime(char *st) {
	int  sb;
	char *beg;

	sb = 0;
	beg = st;
	for (; *st; st++) {
		if (*st == '[')
			sb++;
		else if (*st == ']') {
			sb--;
			if (sb < 0)
				sb = 0;
		}
		if (sb == 0 && *st == ':' && *(st-1) != 'm' && (*(st-1) == '(' || isdigit(*(st+1)))) {
			if ((!isdigit(*(st-1)) && !isdigit(*(st-2))) || st == beg) {
				if (*(st-1) == '(') {
					uS.shiftright(st, 1);
					st[0] = '0';
					st += 1;
				} else {
					uS.shiftright(st, 2);
					st[0] = '0';
					st[1] = '0';
					st += 2;
				}
			} else if (*(st-2) != '(' && (!isdigit(*(st-2)) || st-1 == beg)) {
				uS.shiftright(st-1, 1);
				*(st-1) = '0';
				st += 1;
			}
		}
	}
}

static void changenames(char comline, char *st) {
	int i;
	char *sp, *beg, incom = FALSE;

	beg = st;
	for (; *st; st++) {
		if (*st == '{' || *st == '[') incom = TRUE;
		else if (*st == '}' || *st == ']') incom = FALSE;
		else if (incom || comline == '=') {
			if (isalpha(*st) && !isalpha(*(st+1))) {
				if (st == beg || *(st-1) == '[' || *(st-1) == ' ' ||
					*(st-1) == '<' || *(st-1) == '{' || *(st-1) == '\t') {
					if ((sp=findsp(*st)) != (char *)NULL) {
						sp++;
						i = strlen(sp)-1;
						uS.shiftright(st,i-1);
						for (; i > 0; i--) *st++ = *sp++;
					}
				}
			}
		}
	}
}

static void print(const char *s) {
	if (flag)
		strcat(salt2chat_templine, "+");
	strcat(salt2chat_templine, s);
	flag = 1;
}

static void ones(char d) {
	if(d=='0')
		return;
	print(unit[d-'0']);
}

static void tens(char *p) {
	switch(p[1]) {
			
		case '0':
			return;
			
		case '1':
			print(teen[p[2]-'0']);
			p[2] = '0';
			return;
	}
	
	print(decade[p[1]-'0']);
}

static void nline()
{
	if(flag) strcat(salt2chat_templine,"+");
	flag = 0;
}

static void conv(char *p, int c) {
	if(c > mmax) {
		conv(p, c-mmax);
		print(card[mmax]);
		nline();
		p += (c-mmax)*3;
		c = mmax;
	}
	while(c > 1) {
		c--;
		conv(p, 1);
		if(flag) print(card[c]);
		nline();
		p += 3;
	}
	ones(p[0]);
	if(flag) print(card[0]);
	tens(p);
	ones(p[2]);
}

/* next code is taken from unix game called "number" and modified to be
 used for this program */
static void anumber(char *the_inline) {
	char c;
	int r, i, j = 0;
	
	flag = 0;
	if ((c=salt2chat_templine[strlen(salt2chat_templine)-1])!= ' ' && c!= '\t') 
		strcat(salt2chat_templine," ");
	c = the_inline[j++];
	if (isdigit(c))  {
		i = 0;
		tline[i++] = '0';
		tline[i++] = '0';
		while(c == '0') c = the_inline[j++];
		while(isdigit(c) && i < 98) {
			tline[i++] = c;
			c = the_inline[j++];
		}
		tline[i] = 0;
		r = i/3;
		if(r == 0)
			print("zero");
		else
			conv(tline+i-3*r, r);
		if (!flag) salt2chat_templine[strlen(salt2chat_templine)-1] = EOS;
		if (!isdigit(c) && c != EOS) {
			strcat(salt2chat_templine,the_inline+j-1);
			strcat(salt2chat_templine," ");
		}
	}
}

static void fixupnumbers(char s, char *line) {
	int i;
	char *pos, tchr, isFZero, sb;

	if (!isalpha(s))
		return;

	sb = 0;
	while (*line) {
		if (*line == '[')
			sb++;
		else if (*line == ']') {
			sb--;
			if (sb < 0)
				sb = 0;
		}
		if (*line == '(' && uS.isPause(line,0,NULL,&i)) {
			if (*(line+1) == ':') {
				strcpy(line+1, line+2);
/*
				uS.shiftright(line,1);
				line++;
				*line = '0';
*/
			}
			isFZero = TRUE;
			for (line++; isdigit(*line) || *line==':' || *line=='.'; line++) {
				if (*line == '0' && isFZero && (isdigit(*(line+1)) || *(line+1) == ':')) {
					strcpy(line, line+1);
					line--;
				} else if (*line==':' && *(line-1) == '(' && *(line+1) == ')') {
					*line = '.';
				} else if (*line==':' && isFZero && !isdigit(*(line-1))) {
					strcpy(line, line+1);
					line--;
				}
				if (*line==':')
					isFZero = TRUE;
				else if (*line != '0' && *line != '(')
					isFZero = FALSE;
			}
		} else if (isdigit(*line)) {
			if (isalpha(*(line+1)) || isalpha(*(line-1)) || *(line-1) == '^' || *(line-1) == '-' || sb)
				while (isdigit(*line))
					line++;
			else {
				pos = line;
				while (isdigit(*line))
					line++;
				tchr = *line;
				*line = EOS;
				anumber(pos);
				*line = tchr;
				strcpy(pos,line);
				uS.shiftright(pos,strlen(salt2chat_templine));
				line = pos + strlen(salt2chat_templine);
				for (i=0; salt2chat_templine[i]; i++, pos++) *pos = salt2chat_templine[i];
				*salt2chat_templine = EOS;
			}
		} else
			line++;
	}
}


static void cleanup_line(char s, char *line) {
	char *t;

	if (!isalpha(s))
		return;

	while (*line) {
		if (*line == '&' && *(line+1) == '(') {
			line++;
			strcpy(line, line+1);
			for (t=line; *t != ')' && *t != EOS; t++) ;
			if (*t == ')')
				strcpy(t, t+1);
		} else if (*line == '[' && *(line+1) == '*' && *(line+2) == ' ') {
			line++;
			for (t=line; *t != ']' && *t != EOS; t++) ;
			if (*t == ']' && !uS.isskip(t,1,&dFnt,MBF)) {
				strcpy(t, t+1);
// correcting error codes	*t++ = ' ';
				while (*t != EOS && !uS.isskip(t,0,&dFnt,MBF))
					t++;
				uS.shiftright(t,1);
				*t = ']';
			}
		} else
			line++;
	}
}

static void salt2chat_pm(int n, char *s) {
	switch (n) {
		case 1: strcpy(s,"JAN"); break;
		case 2: strcpy(s,"FEB"); break;
		case 3: strcpy(s,"MAR"); break;
		case 4: strcpy(s,"APR"); break;
		case 5: strcpy(s,"MAY"); break;
		case 6: strcpy(s,"JUN"); break;
		case 7: strcpy(s,"JUL"); break;
		case 8: strcpy(s,"AUG"); break;
		case 9: strcpy(s,"SEP"); break;
		case 10: strcpy(s,"OCT"); break;
		case 11: strcpy(s,"NOV"); break;
		case 12: strcpy(s,"DEC"); break;
		default: strcpy(s," *** INTERNAL ERROR *** "); break;
	}
}

/*
static int salt2chat_tran(char *s) {
	if (!uS.mStricmp(s,"JAN"))
		return(1);
	else if (!uS.mStricmp(s,"FEB"))
		return(2);
	else if (!uS.mStricmp(s,"MAR"))
		return(3);
	else if (!uS.mStricmp(s,"APR"))
		return(4);
	else if (!uS.mStricmp(s,"MAY"))
		return(5);
	else if (!uS.mStricmp(s,"JUN"))
		return(6);
	else if (!uS.mStricmp(s,"JUL"))
		return(7);
	else if (!uS.mStricmp(s,"AUG"))
		return(8);
	else if (!uS.mStricmp(s,"SEP"))
		return(9);
	else if (!uS.mStricmp(s,"OCT"))
		return(10);
	else if (!uS.mStricmp(s,"NOV"))
		return(11);
	else if (!uS.mStricmp(s,"DEC"))
		return(12);
	else
		return(0);
}

static char *DateRepr(char *s) {
	int i = 0, j = 0;
	int d[3];
	char str[20];
	
	d[0] = 0; d[1] = 0; d[2] = 0;
	while (isdigit(s[j])) str[i++] = s[j++];
	str[i] = EOS;
	d[0] = atoi(str);
	j++;
	if (isdigit(s[j])) {
		i = 0;
		d[1] = d[0];
		while (isdigit(s[j])) str[i++] = s[j++];
		str[i] = EOS;
		d[0] = atoi(str);
	} else {
		str[0] = s[j++];
		str[1] = s[j++];
		str[2] = s[j++];
		str[3] = EOS;
		d[1] = salt2chat_tran(str);
	}
	j++;
	i = 0;
	while (isdigit(s[j])) str[i++] = s[j++];
	str[i] = EOS;
	d[2] = atoi(str);
	if (d[2] < 100) d[2] += 1900;
	if (d[0] == 0 || d[1] == 0 || d[2] == 0)
		return(s);
	else {
		salt2chat_pm(d[1],str);
		if (d[0] < 10)
			sprintf(s, "0%d-%s-%d", d[0], str, d[2]);
		else
			sprintf(s, "%d-%s-%d", d[0], str, d[2]);
		return(s);
	}
}
*/
static void addPostCodes(char *line, char *codes) {
	char isOpSq;

	line = line + strlen(line);
	*line++ = ' ';
	isOpSq = FALSE;
	while (*codes != EOS) {
		if (*codes == '$') {
			*line++ = '[';
			*line++ = '+';
			*line++ = ' ';
			codes++;
			isOpSq = TRUE;
		} else if (*codes == ',') {
			*line++ = ']';
			codes++;
			isOpSq = FALSE;
		} else
			*line++ = *codes++;
	}
	if (isOpSq)
		*line++ = ']';
	*line = EOS;
}

static void cleanCodes(char *lcode) {
	for (; *lcode != EOS; lcode++) {
		if (lcode[0] == '$' && lcode[1] == ' ')
			strcpy(lcode+1, lcode+2);
		else if (lcode[0] == '%' && lcode[1] == ' ') {
			lcode[0] = '$';
			strcpy(lcode+1, lcode+2);
		}
	}
}

static char Lisupper(char chr, char isForce) {
	if (curspeaker == '+' || curspeaker == '=')
		return(chr);
	else if (isupper((unsigned char)(int)chr) && (!nomap || isForce))
		return((char)(tolower((unsigned char)chr)));
	else
		return(chr);
}

static void Sophie_AddZero(char *st) {
	int i;
	
	for (i=0; isSpace(st[i]) || st[i] == '\n'; i++) ;
	if (st[i] == '+') {
		while (!uS.isskip(st,i,&dFnt,MBF))
			i++;
	}
	if (uS.IsUtteranceDel(st, i)) {
		uS.shiftright(st, 2);
		st[0] = '0';
		st[1] = ' ';
	}
}

static char isSkipChar(char *st, int pos) {
	if (pos < 0 || uS.isskip(st, pos, &dFnt, MBF) || st[pos] == EOS || st[pos] == '\n')
		return(TRUE);
	return(FALSE);
}

static int removeVBarFromPrevWord(char *st, int i) {
	int j, k;

	k = i - 1;
	while (k > 0 && isSpace(st[k]))
		k--;
	j = k + 1;
	while (k > 0 && isalnum((unsigned char)st[k]))
		k--;
	if (k >= 0 && st[k] == '|') {
		strcpy(st+k, st+j);
		return(j - k);
	}
	return(0);
}

static char isSingleDelim(char *st) {
	if (*st == '.' || *st == '?' || *st == '!')
		return(TRUE);
	return(FALSE);
}

static char isSpaceSingleDelim(char *st) {
	if (isSpace(*st)) {
		while (isSpace(*st))
			st++;
		if (*st == '.' || *st == '?' || *st == '!')
			return(TRUE);
	}
	return(FALSE);
}

/* begin fluency conversion: 
 b-bo-b-boy   rab-bbit-it   r-r-rabbit  he-e-e   b-b-b-boy  bo-bo-bo-boy  g-go   th-th-this  w-w-when  w- when  p-put  th-th-this  co-co-co-could  c-car
 ↫b↫bo↫b↫boy ra↫b↫bbit↫it↫ ↫r-r↫rabbit he↫e-e↫ ↫b-b-b↫boy ↫bo-bo-bo↫boy ↫g↫go ↫th-th↫this ↫w-w↫when ↫w↫when ↫p↫put ↫th-th↫this ↫co-co-co↫could ↫c↫car
*/
static void getLeftItem(char *s, char *st, int i) {
	i--;
	if (i < 0 || st[i] == '-' || uS.isskip(st, i, &dFnt, MBF))
		s[0] = EOS;
	else {
		s[0] = st[i];
		s[1] = EOS;
		i--;
		if (i >= 0 && st[i] != '-' && !uS.isskip(st, i, &dFnt, MBF)) {
			s[1] = s[0];
			s[0] = st[i];
			s[2] = EOS;
			i--;
		}
	}
}

static void getRightItem(char *s, char *st, int i) {
	i++;
	if (st[i] == EOS || st[i] == '-' || uS.isskip(st, i, &dFnt, MBF))
		s[0] = EOS;
	else {
		s[0] = st[i];
		s[1] = EOS;
		i++;
		if (st[i] != EOS && st[i] != '-' && !uS.isskip(st, i, &dFnt, MBF)) {
			s[1] = st[i];
			s[2] = EOS;
			i++;
		}
	}
}

static char isPatToEnd(char *ls, char *st, int i, int *patEnd) {
	int len;

	len = strlen(ls) + 1;
	if (st[i+len] != EOS && st[i+len] != '-' && !uS.isskip(st, i+len, &dFnt, MBF)) {
		*patEnd = i;
		return(FALSE);
	}
	while (st[i] != EOS) {
		if (st[i] == '-') {
			if (ls[0] == st[i+1]) {
				if (ls[1] == st[i+2]) {
					if (ls[1] == EOS) {
						*patEnd = i + 2;
						return(TRUE);
					} else if (st[i+3] == EOS || uS.isskip(st, i+3, &dFnt, MBF)) {
						*patEnd = i + 3;
						return(TRUE);
					} else if (st[i+3] == '-') {
					} else {
						*patEnd = i;
						return(FALSE);
					}
				} else {
					if (ls[1] == EOS && (st[i+2] == EOS || uS.isskip(st, i+2, &dFnt, MBF))) {
						*patEnd = i + 2;
						return(TRUE);
					} else if (ls[1] == EOS && st[i+2] == '-') {
					} else {
						*patEnd = i;
						return(FALSE);
					}
				}
			} else {
				*patEnd = i;
				return(FALSE);
			}
		} else if (uS.isskip(st, i, &dFnt, MBF)) {
			*patEnd = i;
			return(TRUE);
		}
		i++;
	}
	*patEnd = i;
	return(TRUE);
}

static int fluWord(char *st, int i) {
	int j, patEnd, offset;
	char ls[3], rs[3];

	while (st[i] != EOS && !uS.isskip(st, i, &dFnt, MBF)) {
		getLeftItem(ls, st, i);
		if (ls[0] == EOS)
			return(i+1);
		if (uS.isskip(st, i+1, &dFnt, MBF)) {
			for (j=i+1; st[j] != EOS && uS.isskip(st, j, &dFnt, MBF); j++) ;
			if (st[j] != EOS) {
				if (uS.mStrnicmp(st+j, ls, strlen(ls)) == 0) {
					strcpy(st+i+1, st+j);
				}
			}
		}
		getRightItem(rs, st, i);
		if (rs[0] == EOS)
			return(i+1);
		if (ls[0] != rs[0] && ls[0] != EOS) {
			ls[0] = ls[1];
			ls[1] = EOS;
			rs[1] = EOS;
		} else if (ls[1] != rs[1]) {
			if (ls[1] == EOS)
				rs[1] = EOS;
		}
		if (strlen(ls) != strlen(rs) || uS.mStricmp(ls, rs) != 0)
			i++;
		else if (ls[0] == rs[0] && ls[0] != EOS) {

			if (ls[1] == EOS || rs[1] == EOS) {
				j = i - 1;
			} else if (ls[1] == rs[1]) {
				j = i - 2;
			} else {
				j = i - 1;
			}
			if (st[j-1] == '-')
				j--;
			if (isPatToEnd(ls, st, i, &patEnd)) {
				j = i;
			}
			if (st[j] == '-')
				offset = 2;
			else
				offset = 3;
			uS.shiftright(st+j, offset);
			st[j++] = (char)0xE2;
			st[j++] = (char)0x86;
			st[j++] = (char)0xAB;
			j = patEnd + offset;
			if (st[j] == '-')
				uS.shiftright(st+j, 2);
			else
				uS.shiftright(st+j, 3);
			st[j++] = (char)0xE2;
			st[j++] = (char)0x86;
			st[j++] = (char)0xAB;
			i = j;
		} else
			i++;
		while (st[i] != EOS && st[i] != '-' && !uS.isskip(st, i, &dFnt, MBF))
			   i++;
	}
	return(i);
}
/* end fluency conversion:
 b-bo-b-boy   rab-bbit-it   r-r-rabbit  he-e-e   b-b-b-boy  bo-bo-bo-boy  g-go   th-th-this  w-w-when  w- when  p-put  th-th-this  co-co-co-could  c-car
 ↫b↫bo↫b↫boy ra↫b↫bbit↫it↫ ↫r-r↫rabbit he↫e-e↫ ↫b-b-b↫boy ↫bo-bo-bo↫boy ↫g↫go ↫th-th↫this ↫w-w↫when ↫w↫when ↫p↫put ↫th-th↫this ↫co-co-co↫could ↫c↫car
*/

// shiftright	uS.shiftright(st+i, 2);
//cleanup speakers
static void Sophie_cleanupSpeaker(char *st) {
	int i, j, k, offset;

	i = 0;
	while (st[i] != EOS) {
		if (isSingleDelim(st+i) && isSpaceSingleDelim(st+i+1)) {
			st[i] = ' ';
			i++;
		} else if (st[i] == '-') {
			if (st[i+1] == '-')
				i += 2;
			else if (coding == 7)
				i++;
			else
				i = fluWord(st, i);
		} else if (st[i] == '_' && (i == 0 || !isalnum(st[i-1]) || !isalnum(st[i+1]))) {
			st[i] = ' ';
			i++;
		} else if (uS.mStrnicmp(st+i, "oooh>", 5) == 0) {
			strcpy(st+i, st+i+1);
			for (j=i+4; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<ooh", 4) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					i += 3;
					strcpy(st+i, st+j+4);
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "uhuh>", 5) == 0) {
			for (j=i+5; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<uhuh", 5) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					i += 4;
					strcpy(st+i, st+j+4);
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "ooo>", 4) == 0 || uS.mStrnicmp(st+i, "mmm>", 4) == 0) {
			strcpy(st+i, st+i+1);
			for (j=i+3; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if ( uS.mStrnicmp(st+k, "<oo", 3) == 0 || uS.mStrnicmp(st+k, "<mm", 3) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					i += 2;
					strcpy(st+i, st+j+4);
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "hmm>", 4) == 0 || uS.mStrnicmp(st+i, "huh>", 4) == 0 ||
				   uS.mStrnicmp(st+i, "ooh>", 4) == 0 || uS.mStrnicmp(st+i, "oop>", 4) == 0) {
			for (j=i+4; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<hmm", 4) == 0 || uS.mStrnicmp(st+k, "<huh", 4) == 0 ||
					uS.mStrnicmp(st+k, "<ooh", 4) == 0 || uS.mStrnicmp(st+k, "<oop", 4) == 0 ||
					uS.mStrnicmp(st+k, "<oh", 3) == 0 || uS.mStrnicmp(st+k, "<ah", 3) == 0 ||
					uS.mStrnicmp(st+k, "<mm", 3) == 0 || uS.mStrnicmp(st+k, "<oo", 3) == 0 ||
					uS.mStrnicmp(st+k, "<uh", 3) == 0 || uS.mStrnicmp(st+k, "<um", 3) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					i += 3;
					strcpy(st+i, st+j+4);
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "oh>", 3) == 0 || uS.mStrnicmp(st+i, "ah>", 3) == 0 ||
				   uS.mStrnicmp(st+i, "mm>", 3) == 0 || uS.mStrnicmp(st+i, "oo>", 3) == 0) {
			for (j=i+3; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<oh", 3) == 0 || uS.mStrnicmp(st+k, "<ah", 3) == 0 ||
					uS.mStrnicmp(st+k, "<mm", 3) == 0 || uS.mStrnicmp(st+k, "<oo", 3) == 0 ||
					uS.mStrnicmp(st+k, "<uh", 3) == 0 || uS.mStrnicmp(st+k, "<um", 3) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					i += 2;
					strcpy(st+i, st+j+4);
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "&uh>", 4) == 0 || uS.mStrnicmp(st+i, "&um>", 4) == 0) {
			for (j=i+4; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<&uh", 4) == 0 || uS.mStrnicmp(st+k, "<&um", 4) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					i += 3;
					strcpy(st+i, st+j+4);
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "whoa>", 5) == 0) {
			for (j=i+5; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<whoa", 5) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					i += 4;
					strcpy(st+i, st+j+4);
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "gasp>", 5) == 0) {
			for (j=i+5; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<gasp", 5) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					strcpy(st+i+4, st+j+4);
					uS.shiftright(st+i, 2);
					st[i++] = '&';
					st[i++] = '=';
					i += 4;
					uS.shiftright(st+i, 1);
					st[i++] = 's';
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "laughs>", 7) == 0) {
			for (j=i+7; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<laughs", 7) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
					strcpy(st+i+6, st+j+4);
					uS.shiftright(st+i, 2);
					st[i++] = '&';
					st[i++] = '=';
					i += 6;
				} else
					i++;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "[% ew:", 6) == 0) {
			for (j=i+5; st[j] != ']' && !isSpace(st[j]) && st[j] != EOS; j++) ;
			if (st[j] == ']') {
				offset = removeVBarFromPrevWord(st, i);
				i = i - offset;
				j = j - offset;
				strcpy(st+i+1, st+i+4);
				st[i+1] = ':';
				st[i+2] = ' ';
				i = j - 2;
				uS.shiftright(st+i, 4);
				st[i++] = ' ';
				st[i++] = '[';
				st[i++] = '*';
				st[i++] = ']';
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "[% eo:", 6) == 0) {
			for (j=i+5; st[j] != ']' && !isSpace(st[j]) && st[j] != EOS; j++) ;
			if (st[j] == ']') {
				offset = removeVBarFromPrevWord(st, i);
				i = i - offset;
				j = j - offset;
				strcpy(st+i+1, st+i+4);
				st[i+1] = ':';
				st[i+2] = ' ';
				i = j - 2;
				uS.shiftright(st+i, 4);
				st[i++] = ' ';
				st[i++] = '[';
				st[i++] = '*';
				st[i++] = ']';
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "[% * ap]", 8) == 0) {
			strcpy(st+i, st+i+8);
		} else if (uS.mStrnicmp(st+i, "[% *ap]", 7) == 0) {
			strcpy(st+i, st+i+7);
		} else if (uS.mStrnicmp(st+i, "[% ap]", 6) == 0) {
			strcpy(st+i, st+i+6);
		} else if (uS.mStrnicmp(st+i, "[% jar]", 7) == 0) {
			strcpy(st+i, st+i+7);
		} else if (uS.mStrnicmp(st+i, "[% dm]", 6) == 0) {
			strcpy(st+i, st+i+6);
		} else if (uS.mStrnicmp(st+i, "[% ts]", 6) == 0) {
			strcpy(st+i, st+i+6);
		} else if (uS.mStrnicmp(st+i, "{% laughing}", 12) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'l';
			st[i++] = 'a';
			st[i++] = 'u';
			st[i++] = 'g';
			st[i++] = 'h';
			st[i++] = 's';
			st[i++] = ' ';
			strcpy(st+i, st+i+3);
		} else if (uS.mStrnicmp(st+i, "{% laughs}", 10) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'l';
			st[i++] = 'a';
			st[i++] = 'u';
			st[i++] = 'g';
			st[i++] = 'h';
			st[i++] = 's';
			st[i++] = ' ';
			strcpy(st+i, st+i+1);
		} else if (uS.mStrnicmp(st+i, "{% gasps}", 9) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'g';
			st[i++] = 'a';
			st[i++] = 's';
			st[i++] = 'p';
			st[i++] = 's';
			st[i++] = ' ';
			strcpy(st+i, st+i+1);
		} else if (uS.mStrnicmp(st+i, "{% squeal}", 10) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 's';
			st[i++] = 'q';
			st[i++] = 'u';
			st[i++] = 'e';
			st[i++] = 'a';
			st[i++] = 'l';
			st[i++] = 's';
			st[i++] = ' ';
		} else if (uS.mStrnicmp(st+i, "{% squeals}", 11) == 0) {
			strcpy(st+i, st+i+1);
			st[i++] = '&';
			st[i++] = '=';
			i += 7;
			st[i++] = ' ';
		} else if (uS.mStrnicmp(st+i, "{% toy noise}", 13) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'i';
			st[i++] = 'm';
			st[i++] = 'i';
			st[i++] = 't';
			st[i++] = ':';
			st[i++] = 't';
			st[i++] = 'o';
			st[i++] = 'y';
			st[i++] = ' ';
			strcpy(st+i, st+i+2);
		} else if (uS.mStrnicmp(st+i, "{% toy noises}", 14) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'i';
			st[i++] = 'm';
			st[i++] = 'i';
			st[i++] = 't';
			st[i++] = ':';
			st[i++] = 't';
			st[i++] = 'o';
			st[i++] = 'y';
			st[i++] = ' ';
			strcpy(st+i, st+i+3);
		} else if (uS.mStrnicmp(st+i, "{% car noise}", 13) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'i';
			st[i++] = 'm';
			st[i++] = 'i';
			st[i++] = 't';
			st[i++] = ':';
			st[i++] = 'c';
			st[i++] = 'a';
			st[i++] = 'r';
			st[i++] = ' ';
			strcpy(st+i, st+i+2);
		} else if (uS.mStrnicmp(st+i, "{% car noises}", 14) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'i';
			st[i++] = 'm';
			st[i++] = 'i';
			st[i++] = 't';
			st[i++] = ':';
			st[i++] = 'c';
			st[i++] = 'a';
			st[i++] = 'r';
			st[i++] = ' ';
			strcpy(st+i, st+i+3);
		} else if (uS.mStrnicmp(st+i, "{% makes driving sound}", 23) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'i';
			st[i++] = 'm';
			st[i++] = 'i';
			st[i++] = 't';
			st[i++] = ':';
			st[i++] = 'd';
			st[i++] = 'r';
			st[i++] = 'i';
			st[i++] = 'v';
			st[i++] = 'i';
			st[i++] = 'n';
			st[i++] = 'g';
			st[i++] = ' ';
			strcpy(st+i, st+i+8);
		} else if (uS.mStrnicmp(st+i, "{% makes driving sounds}", 24) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'i';
			st[i++] = 'm';
			st[i++] = 'i';
			st[i++] = 't';
			st[i++] = ':';
			st[i++] = 'd';
			st[i++] = 'r';
			st[i++] = 'i';
			st[i++] = 'v';
			st[i++] = 'i';
			st[i++] = 'n';
			st[i++] = 'g';
			st[i++] = ' ';
			strcpy(st+i, st+i+9);
		} else if (uS.mStrnicmp(st+i, "{% makes stopping sound}", 24) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'i';
			st[i++] = 'm';
			st[i++] = 'i';
			st[i++] = 't';
			st[i++] = ':';
			st[i++] = 's';
			st[i++] = 't';
			st[i++] = 'o';
			st[i++] = 'p';
			st[i++] = 'p';
			st[i++] = 'i';
			st[i++] = 'n';
			st[i++] = 'g';
			st[i++] = ' ';
			strcpy(st+i, st+i+9);
		} else if (uS.mStrnicmp(st+i, "{% makes stopping sounds}", 25) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'i';
			st[i++] = 'm';
			st[i++] = 'i';
			st[i++] = 't';
			st[i++] = ':';
			st[i++] = 's';
			st[i++] = 't';
			st[i++] = 'o';
			st[i++] = 'p';
			st[i++] = 'p';
			st[i++] = 'i';
			st[i++] = 'n';
			st[i++] = 'g';
			st[i++] = ' ';
			strcpy(st+i, st+i+10);
		} else if (uS.mStrnicmp(st+i, "{% kissing noise}", 17) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'k';
			st[i++] = 'i';
			st[i++] = 's';
			st[i++] = 's';
			st[i++] = 'e';
			st[i++] = 's';
			st[i++] = ' ';
			strcpy(st+i, st+i+8);
		} else if (uS.mStrnicmp(st+i, "{% kissing noises}", 18) == 0) {
			st[i++] = '&';
			st[i++] = '=';
			st[i++] = 'k';
			st[i++] = 'i';
			st[i++] = 's';
			st[i++] = 's';
			st[i++] = 'e';
			st[i++] = 's';
			st[i++] = ' ';
			strcpy(st+i, st+i+9);
		} else if (uS.mStrnicmp(st+i, "{% sing-song voice}", 19) == 0) {
			st[i] = '[';
			st[i+18] = ']';
			i++;
			st[i++] = '=';
			st[i++] = '!';
			st[i++] = ' ';
			st[i++] = 's';
			st[i++] = 'i';
			st[i++] = 'n';
			st[i++] = 'g';
			st[i++] = 's';
			st[i++] = 'o';
			st[i++] = 'n';
			st[i++] = 'g';
			strcpy(st+i, st+i+6);
		} else if (uS.mStrnicmp(st+i, "{% sing-song voices}", 20) == 0) {
			st[i] = '[';
			st[i+19] = ']';
			i++;
			st[i++] = '=';
			st[i++] = '!';
			st[i++] = ' ';
			st[i++] = 's';
			st[i++] = 'i';
			st[i++] = 'n';
			st[i++] = 'g';
			st[i++] = 's';
			st[i++] = 'o';
			st[i++] = 'n';
			st[i++] = 'g';
			strcpy(st+i, st+i+7);
		} else if (uS.mStrnicmp(st+i, "{% sing song voice}", 19) == 0) {
			st[i] = '[';
			st[i+18] = ']';
			i++;
			st[i++] = '=';
			st[i++] = '!';
			st[i++] = ' ';
			st[i++] = 's';
			st[i++] = 'i';
			st[i++] = 'n';
			st[i++] = 'g';
			st[i++] = 's';
			st[i++] = 'o';
			st[i++] = 'n';
			st[i++] = 'g';
			strcpy(st+i, st+i+6);
		} else if (uS.mStrnicmp(st+i, "{% sing song voices}", 20) == 0) {
			st[i] = '[';
			st[i+19] = ']';
			i++;
			st[i++] = '=';
			st[i++] = '!';
			st[i++] = ' ';
			st[i++] = 's';
			st[i++] = 'i';
			st[i++] = 'n';
			st[i++] = 'g';
			st[i++] = 's';
			st[i++] = 'o';
			st[i++] = 'n';
			st[i++] = 'g';
			strcpy(st+i, st+i+7);
		} else if (uS.mStrnicmp(st+i, "{% whisper}", 11) == 0) {
			st[i] = '[';
			st[i+10] = ']';
			uS.shiftright(st+i+1, 1);
			st[i+1] = '=';
			st[i+2] = '!';
		} else if (uS.mStrnicmp(st+i, "{% whispers}", 12) == 0) {
			st[i] = '[';
			st[i+11] = ']';
			uS.shiftright(st+i+1, 1);
			st[i+1] = '=';
			st[i+2] = '!';
		} else if (uS.mStrnicmp(st+i, "thank you_THANKYOU", 18) == 0) {
			st[i+5] = '_';
		} else if (uS.mStrnicmp(st+i, "THANKYOU_thank you", 18) == 0) {
			st[i+14] = '_';
		} else if (uS.mStrnicmp(st+i, "uh_oh", 5) == 0) {
			strcpy(st+i+2, st+i+3);
		} else if (uS.mStrnicmp(st+i, "uh_uh", 5) == 0) {
			strcpy(st+i+2, st+i+3);
		} else if (uS.mStrnicmp(st+i, "car_seat", 8) == 0) {
			strcpy(st+i+3, st+i+4);
		} else if (uS.mStrnicmp(st+i, "lala", 4) == 0) {
			st[i] = Lisupper(st[i], TRUE);
			st[i+1] = Lisupper(st[i+1], TRUE);
			st[i+2] = Lisupper(st[i+2], TRUE);
			st[i+3] = Lisupper(st[i+3], TRUE);
			uS.shiftright(st+i, 1);
			st[i] = '&';
			i += 3;
			uS.shiftright(st+i, 2);
			st[i] = ' ';
			st[i+1] = '&';
			i += 4;
		} else if (uS.mStrnicmp(st+i, "alldone", 7) == 0) {
			uS.shiftright(st+i+3, 1);
			st[i+3] = '_';
			i += 8;
		} else if (uS.mStrnicmp(st+i, "allgone", 7) == 0) {
			uS.shiftright(st+i+3, 1);
			st[i+3] = '_';
			i += 8;
		} else if (uS.mStrnicmp(st+i, "rockabye", 8) == 0) {
			uS.shiftright(st+i+4, 1);
			st[i+4] = '_';
			uS.shiftright(st+i+6, 1);
			st[i+6] = '_';
			i += 10;
		} else if (uS.mStrnicmp(st+i, "goodnight", 9) == 0) {
			uS.shiftright(st+i+4, 1);
			st[i+4] = '_';
			i += 10;
		} else if (uS.mStrnicmp(st+i, "ok", 2) == 0) {
			if ((i == 0 || isSkipChar(st,i-1) || st[i-1] == '{') && (isSkipChar(st,i+2) || st[i+2] == '{')) {
				uS.shiftright(st+i+2, 2);
				i += 2;
				st[i++] = 'a';
				st[i++] = 'y';
			} else
				i++;
		} else if ((i == 0 || uS.isskip(st, i-1, &dFnt, MBF)) && uS.isskip(st, i+2, &dFnt, MBF) &&
				   (uS.mStrnicmp(st+i, "uh", 2) == 0 || uS.mStrnicmp(st+i, "um", 2) == 0)) {
			uS.shiftright(st+i, 1);
			st[i] = '&';
		} else if ((i == 0 || uS.isskip(st, i-1, &dFnt, MBF)) && uS.mStrnicmp(st+i, "mrs", 3) == 0) {
			for (j=i+3; isSpace(st[j]); j++) ;
			if (st[j] == '.') {
				strcpy(st+j, st+j+1);
				uS.shiftright(st+i, 4);
				st[i++] = 'M';
				st[i++] = 'i';
				st[i++] = 's';
				st[i++] = 's';
				st[i++] = 'u';
				st[i++] = 's';
				st[i++] = ' ';
			} else
				i++;
		} else if ((i == 0 || uS.isskip(st, i-1, &dFnt, MBF)) && uS.mStrnicmp(st+i, "mr", 2) == 0) {
			for (j=i+2; isSpace(st[j]); j++) ;
			if (st[j] == '.') {
				strcpy(st+j, st+j+1);
				uS.shiftright(st+i, 5);
				st[i++] = 'M';
				st[i++] = 'i';
				st[i++] = 's';
				st[i++] = 't';
				st[i++] = 'e';
				st[i++] = 'r';
				st[i++] = ' ';
			} else
				i++;
		} else if ((i == 0 || uS.isskip(st, i-1, &dFnt, MBF)) && uS.mStrnicmp(st+i, "ms", 2) == 0) {
			for (j=i+2; isSpace(st[j]); j++) ;
			if (st[j] == '.') {
				strcpy(st+j, st+j+1);
				uS.shiftright(st+i, 1);
				st[i++] = 'M';
				st[i++] = 's';
				st[i++] = ' ';
			} else
				i++;
		} else if ((i == 0 || uS.isskip(st, i-1, &dFnt, MBF)) && uS.mStrnicmp(st+i, "dr", 2) == 0) {
			for (j=i+2; isSpace(st[j]); j++) ;
			if (st[j] == '.') {
				strcpy(st+j, st+j+1);
				uS.shiftright(st+i, 5);
				st[i++] = 'D';
				st[i++] = 'o';
				st[i++] = 'c';
				st[i++] = 't';
				st[i++] = 'o';
				st[i++] = 'r';
				st[i++] = ' ';
			} else
				i++;
		} else if ((i == 0 || uS.isskip(st, i-1, &dFnt, MBF)) && uS.mStrnicmp(st+i, "st", 2) == 0) {
			for (j=i+2; isSpace(st[j]); j++) ;
			if (st[j] == '.') {
				strcpy(st+j, st+j+1);
				uS.shiftright(st+i, 5);
				st[i++] = 's';
				st[i++] = 't';
				st[i++] = 'r';
				st[i++] = 'e';
				st[i++] = 'e';
				st[i++] = 't';
				st[i++] = ' ';
			} else
				i++;
		} else
			i++;
	}
}

static void Sophie_CapitalizeCleanup(char *st) {
	int i, j, f;

	for (i=0; uS.isskip(st, i, &dFnt, MBF) && st[i] != '[' && st[i] != '{' && st[i] != '&'; i++) ;
	f = i;
	while (st[i] != EOS) {
		if (i == 0 || uS.isskip(st, i-1, &dFnt, MBF)) {
			if (st[i]=='i' && (st[i+1]=='\'' || isSkipChar(st,i+1))) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "big_bird", 8) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
				st[i+4] = (char)toupper((unsigned char)st[i+4]);
			} else if (uS.mStrnicmp(st+i, "pooh_bear", 9) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
				st[i+5] = (char)toupper((unsigned char)st[i+5]);
			} else if (uS.mStrnicmp(st+i, "ernie", 5) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "cookie_monster", 14) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
				st[i+7] = (char)toupper((unsigned char)st[i+7]);
			} else if (uS.mStrnicmp(st+i, "oscar_the_grouch", 5) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
				st[i+10] = (char)toupper((unsigned char)st[i+10]);
			} else if (uS.mStrnicmp(st+i, "oscar the grouch", 16) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
				st[i+10] = (char)toupper((unsigned char)st[i+10]);
			} else if (uS.mStrnicmp(st+i, "oscar", 5) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "bert", 4) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "mickey_mouse", 12) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
				st[i+7] = (char)toupper((unsigned char)st[i+7]);
			} else if (uS.mStrnicmp(st+i, "donald_duck", 11) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
				st[i+7] = (char)toupper((unsigned char)st[i+7]);
			} else if (uS.mStrnicmp(st+i, "donald", 6) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "barney", 6) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "lydi", 4) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "andrew", 6) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "william", 7) == 0) {
				st[i] = (char)toupper((unsigned char)st[i]);
			} else if (uS.mStrnicmp(st+i, "[% om]", 6) == 0) {
				if (st[f] != '&' && st[f] != '[' && st[f] != '{') {
					for (; st[i] != EOS && st[i] != ']'; i++)
						st[i] = ' ';
					if (st[i] == ']')
						st[i++] = ' ';
					else
						i--;
					for (j=f; !uS.isskip(st, j, &dFnt, MBF); j++) ;
					uS.shiftright(st+j, 2);
					st[j++] = '@';
					st[j++] = 'o';
					i = i + 2;
				}
			}
			if (!uS.isskip(st, i, &dFnt, MBF)/* || st[i] == '[' || st[i] == '{'*/ || st[i] == '&') {
				f = i;
			}			
		}
		i++;
	}
}

static void printIDs(char *part, const char *targetSP, char *age, char *gen) {
	char *b, *e, *code, *role;

	b = part;
	while (*b != EOS) {
		e = strchr(b, ',');
		if (e == NULL)
			e = b + strlen(b);
		else
			*e++ = EOS;
		for (code=b; isSpace(*code); code++) ;
		if (*code != EOS) {
			for (role=code; *role != EOS && !isSpace(*role); role++) ;
			if (*role != EOS) {
				*role++ = EOS;
				uS.remblanks(role);
				strcpy(templineC1, "eng|");
				if (coding == 4)
					strcat(templineC1, "Ron|");
				else if (coding == 7)
					strcat(templineC1, "change_me_later|");
				else
					strcat(templineC1, "change_me_later|");
				strcat(templineC1, code);
				strcat(templineC1, "|");
				if (age != NULL && targetSP != NULL) {
					if (uS.mStricmp(targetSP, code) == 0)
						strcat(templineC1, age);
				}
				strcat(templineC1, "|");
				if (gen != NULL && targetSP != NULL) {
					if (uS.mStricmp(targetSP, code) == 0)
						strcat(templineC1, gen);
				}
				strcat(templineC1, "|||");
				strcat(templineC1, role);
				strcat(templineC1, "|||");
				printout("@ID:\t", templineC1, NULL, NULL, TRUE);
			}
		}
		b = e;
	}
}

static void insertInPart(const char *code, char *part, char *buf) {
	int i, j, k, len;

	len = strlen(code);
	i = 0;
	j = 0;
	while (part[i] != EOS) {
		if (uS.mStrnicmp(part+i, code, len) == 0) {
			for (k=0; code[k] != EOS; k++) {
				templineC[j++] = code[k];
				i++;
			}
			for (k=0; buf[k] != EOS; k++)
				templineC[j++] = buf[k];
			if (!isSpace(templineC[j-1]))
				templineC[j++] = ' ';
		} else
			templineC[j++] = part[i++];
	}
	templineC[j] = EOS;
}

static void convertDate(const char *label, char *date) {
	int  i;
	char month[32], day[32], year[32];

	i = atoi(date);
	salt2chat_pm(i, month);
	while (isdigit(*date))
		date++;
	if (*date == '/' || *date == '-')
		date++;
	i = 0;
	while (isdigit(*date)) {
		day[i++] = *date;
		date++;
	}
	day[i] = EOS;
	if (*date == '/' || *date == '-')
		date++;
	i = 0;
	while (isdigit(*date)) {
		year[i++] = *date;
		date++;
	}
	year[i] = EOS;
	templineC[0] = EOS;
	if (day[1] == EOS)
		strcat(templineC, "0");
	strcat(templineC, day);
	strcat(templineC, "-");
	if (month[1] == EOS)
		strcat(templineC, "0");
	strcat(templineC, month);
	strcat(templineC, "-");
	if (strlen(year) < 4)
		strcat(templineC, "19");
	strcat(templineC, year);
	printout(label, templineC, NULL, NULL, TRUE);
}

static void convert_PrintComment(char *comm) {
	int  i;
	char buf[256];

	while (*comm != EOS) {
		for (i=0; *comm != EOS && *comm != '\001'; comm++) {
			if (*comm == ':') {
				if (i > 0 && !isSpace(buf[i-1]))
					buf[i++] = ' ';
				buf[i++] = '=';
			} else
				buf[i++] = *comm;
		}
		buf[i] = EOS;
		uS.remblanks(buf);
		if (buf[0] != EOS) {
//			if (islower(buf[0]))
//				buf[0] = toupper(buf[0]);
			printout("@Comment:", buf, NULL, NULL, TRUE);
		}
		while (*comm == '\001' || isspace(*comm))
			comm++;
	}
}

static void fixAge(char *age) {
	int i;
	char isDotFound;

	isDotFound = FALSE;
	for (i=0; age[i] != EOS; i++) {
		if (age[i] == ';' && isdigit(age[i+1]) && !isdigit(age[i+2])) {
			uS.shiftright(age+i+1, 1);
			age[i+1] = '0';
		} else if (age[i] == '.' && isdigit(age[i+1]) && !isdigit(age[i+2])) {
			uS.shiftright(age+i+1, 1);
			age[i+1] = '0';
		}
		if (age[i] == '.')
			isDotFound = TRUE;
	}
	if (!isDotFound)
		strcat(age, ".");
}

static void processPartComm(char *part, char *comm) {
	int  i;
	char *b, *e, name[64], gen[32], age[32], date[64], DOB[64], trans[64];

	name[0] = EOS;
	gen[0]  = EOS;
	age[0]  = EOS;
	date[0] = EOS;
	DOB[0]  = EOS;
	trans[0]= EOS;
	for (e=comm; *e != EOS; e++) {
		if (*e == '\t' || *e == '\n')
			*e = ' ';
	}
	remJunkFrontAndBack(comm);
	removeExtraSpace(comm);
	e = comm;
	while (*e != EOS) {
		for (; isSpace(*e); e++) ;
		if (uS.mStrnicmp(e, "Name:", 5) == 0 || uS.mStrnicmp(e, "Name ", 5) == 0) {
			b = e;
			i = 0;
			for (e=e+5; *e == ':' || isSpace(*e); e++) ;
			for (; *e != EOS && *e != '\001'; e++) {
				if (isSpace(*e))
					*e = '_';
				name[i++] = *e;
			}
			name[i] = EOS;
			if (*e == '\001')
				e++;
			strcpy(b, e);
			e = b;
		} else if (uS.mStrnicmp(e, "Gender:", 7) == 0 || uS.mStrnicmp(e, "Gender ", 7) == 0) {
			b = e;
			i = 0;
			for (e=e+7; *e == ':' || isSpace(*e); e++) ;
			for (; *e != EOS && *e != '\001'; e++) {
				gen[i++] = *e;
			}
			gen[i] = EOS;
			if (*e == '\001')
				e++;
			strcpy(b, e);
			e = b;
		} else if (uS.mStrnicmp(e, "CA:", 3) == 0 || strncmp(e, "CA ", 3) == 0) {
			b = e;
			i = 0;
			for (e=e+3; *e == ':' || isSpace(*e); e++) ;
			for (; *e != EOS && *e != '\001'; e++) {
				age[i++] = *e;
			}
			age[i] = EOS;
			if (*e == '\001')
				e++;
			strcpy(b, e);
			e = b;
		} else if (uS.mStrnicmp(e, "DOE:", 4) == 0 || uS.mStrnicmp(e, "DOE ", 4) == 0) {
			b = e;
			i = 0;
			for (e=e+4; *e != EOS && *e != '\001' && !isdigit(*e); e++) ;
			for (; *e != EOS && *e != '\001'; e++) {
				date[i++] = *e;
			}
			date[i] = EOS;
			if (*e == '\001')
				e++;
			strcpy(b, e);
			e = b;
		} else if (uS.mStrnicmp(e, "DOB:", 4) == 0 || uS.mStrnicmp(e, "DOB ", 4) == 0) {
			b = e;
			i = 0;
			for (e=e+4; *e != EOS && *e != '\001' && !isdigit(*e); e++) ;
			for (; *e != EOS && *e != '\001'; e++) {
				DOB[i++] = *e;
			}
			DOB[i] = EOS;
			if (*e == '\001')
				e++;
			strcpy(b, e);
			e = b;
		} else if (uS.mStrnicmp(e, "Transcriber:", 12) == 0 || uS.mStrnicmp(e, "Transcriber ", 12) == 0) {
			b = e;
			i = 0;
			for (e=e+12; *e == ':' || isSpace(*e); e++) ;
			for (; *e != EOS && *e != '\001'; e++) {
				trans[i++] = *e;
			}
			trans[i] = EOS;
			if (*e == '\001')
				e++;
			strcpy(b, e);
			e = b;
		} else {
			for (; *e != EOS && *e != '\001'; e++) ;
			if (*e == '\001') {
				e++;
			}
		}
	}
	remJunkFrontAndBack(name);
	remJunkFrontAndBack(gen);
	if (name[0] != EOS)
		insertInPart("CHI ", part, name);
	else
		strcpy(templineC, part);
	printout("@Participants:\t", templineC, NULL, NULL, TRUE);
	if (gen[0] == 'F' || gen[0] == 'f')
		strcpy(gen, "female");
	else if (gen[0] == 'M' || gen[0] == 'm')
		strcpy(gen, "male");
	remJunkFrontAndBack(age);
	fixAge(age);
	printIDs(part, "CHI", age, gen);
	remJunkFrontAndBack(DOB);
	if (DOB[0] != EOS)
		convertDate("@Birth of CHI:\t", DOB);
	remJunkFrontAndBack(date);
	if (date[0] != EOS)
		convertDate("@Date:\t", date);
	remJunkFrontAndBack(trans);
	if (trans[0] != EOS)
		printout("@Transcriber:\t", trans, NULL, NULL, TRUE);
	part[0] = EOS;
	remJunkFrontAndBack(comm);
	if (comm[0] != EOS)
		convert_PrintComment(comm);
	comm[0] = EOS;
}

static char isMatchBR(char *s, int i) {
	char c;

	c = s[i];
	if (c == '[')
		c = ']';
	else if (c == '{')
		c = '}';
	for (; s[i] != c && s[i] != EOS; i++) ;
	if (s[i] == EOS)
		return(FALSE);
	else
		return(TRUE);
}

static char isLangCode(char *s, int i) {
	if (uS.mStrnicmp(s+i, "[v]", 3) == 0)
		return(1);
	if (s[i] == '[' && s[i+1] == 'v' && isdigit(s[i+2]) && s[i+3] == ']')
		return(2);
	if (uS.mStrnicmp(s+i, "[s]", 3) == 0)
		return(3);
	if (uS.mStrnicmp(s+i, "[sv]", 4) == 0)
		return(4);
	return(0);
}

static char isItem(char *s, int i) {
	if (s[i] == '[' || s[i] == '{') {
		if (isMatchBR(s, i))
			return(TRUE);
		else
			return(FALSE);
	} else if (s[i] == ']')
		return(TRUE);
	else if (!uS.isskip(s,i,&dFnt,MBF) && s[i] != '\n')
		return(TRUE);
	return(FALSE);
}

static void breakupLangCodeWords(char *line, int fi, char *w, char **vw, char **sw) {
	int  f, e, vi, si;
	char *v, *s, nextCode, curCode, isEOS;

	if (w[0] == EOS)
		return;
	nextCode = 0;
	for (; line[fi] != EOS; fi++) {
		if ((nextCode=isLangCode(line, fi)) != 0)
			break;
	}
	v = w + strlen(w) + 1;
	*v = EOS;
	s = v + strlen(w) + 1;
	*s = EOS;
	isEOS = FALSE;
	curCode = 0;
	f = 0;
	e = 0;
	vi = 0;
	si = 0;
	while (w[f] != EOS) {
		if (w[e] == EOS || (curCode=isLangCode(w, e)) != 0) {
			if (w[e] == EOS) {
				if (nextCode == 0)
					curCode = 1;
				else
					curCode = nextCode;
				isEOS = TRUE;
			}
			if (curCode == 1 || curCode == 2) {
				w[e] = EOS;
				strcat(v, w+f);
				vw[0] = v;
				if (!isEOS) {
					if (curCode == 1)
						strcpy(w+e, w+e+3);
					else
						strcpy(w+e, w+e+4);
				}
				f = e;
			} else if (curCode == 3) {
				w[e] = EOS;
				strcat(s, w+f);
				sw[0] = s;
				if (!isEOS)
					strcpy(w+e, w+e+3);
				f = e;
			} else if (curCode == 4) {
				w[e] = EOS;
				strcat(v, w+f);
				vw[0] = v;
				strcat(s, w+f);
				sw[0] = s;
				if (!isEOS)
					strcpy(w+e, w+e+4);
				f = e;
			}
		} else
			e++;
	}
}

static char isMostlyUpper(char *s, char isUC) {
	int   p;
	float c, l, perc;
	
	p = 0;
	if (s[p] == 'I' && s[p+1] == '\'')
		return(FALSE);
	if (!strcmp(s, "I"))
		return(FALSE);
	c = 0.0;
	l = 0.0;
	for (; s[p] != EOS; p++) {
		if (s[p] == '/' || s[p] == '\'')
			break;
		if (isupper((unsigned char)s[p]))
			c++;
		else if (islower((unsigned char)s[p]))
			l++;
	}
	if (strncmp(s+p-3, "ing", 3) == 0 && l >= 3.0 && c > 1.0)
		l = l - 3.0;
	if (c == 1 && l == 1 && isUC) {
		c = 2;
		l = 0;
	}
	l = l + c;
	if (l <= 0.0)
		return(FALSE);
	perc = (c * 100.0000) / l;
	if (perc > 50.00)
		return(TRUE);
	return(FALSE);
}

static void breakupWords(char *w, char **vw, char **sw) {
	int  vi, si;
	char *nw, isUC;

	nw = strchr(w, '_');
	if (nw == NULL) {
		if (w[0] != EOS) {
			if (isMostlyUpper(w, 0))
				sw[0] = w;
			else
				vw[0] = w;
		}
	} else {
		isUC = FALSE;
		vi = 0;
		si = 0;
		while (w != NULL) {
			if (nw != NULL)
				*nw = EOS;
			if (w[0] != EOS) {
				if (isMostlyUpper(w, isUC))
					sw[si++] = w;
				else
					vw[vi++] = w;
			}
			if (nw == NULL)
				break;
			w = nw + 1;
			nw = strchr(w, '_');
			isUC = TRUE;
		}
	}
}

static void Sophie_extractSin(char *line, char *toline, char *sin) {
	int  fi, ti, wi;
	char *vw[50], *sw[50], sqb, langCode, isLangCodeFound;

	sin[0] = EOS;
	isLangCodeFound = FALSE;
	for (fi=0; line[fi] != EOS; fi++) {
		if (isLangCode(line, fi)) {
			isLangCodeFound = TRUE;
			break;
		}
	}
	fi = 0;
	ti = 0;
	sqb = FALSE;
	while (line[fi] != EOS) {
		while (!isItem(line, fi) && line[fi] != EOS) {
			toline[ti++] = line[fi++];
		}
		if (line[fi] == '[')
			sqb = ']';
		else if (line[fi] == '{')
			sqb = '}';
		else
			sqb = 0;
		wi = 0;
		if (line[fi] != EOS)
			templineC2[wi++] = line[fi++];
		while ((isItem(line, fi) || sqb) && line[fi] != EOS) {
			templineC2[wi++] = line[fi];
			if (sqb == line[fi]) {
				fi++;
				break;
			} else
				fi++;
		}
		templineC2[wi] = EOS;
		if (templineC2[0]=='{' || templineC2[0]=='[' || templineC2[0]=='&' || templineC2[0]=='+') {
			if (templineC2[0] != '&')
				uS.lowercasestr(templineC2, &dFnt, MBF);
			if (templineC2[0] == '{') {
				for (wi=strlen(templineC2)-1; wi > 0 && (templineC2[wi] == '}' || isSpace(templineC2[wi])); wi--) ;
				wi++;
				templineC2[wi] = EOS;
				if (*comm != EOS)
					strcat(comm, ", ");
				strcat(comm, templineC2+2);
			} else if ((langCode=isLangCode(templineC2, 0)) != 0) {
				if (langCode == 2) {
					strcat(salt2chat_pcodes," [+ ");
					strcat(salt2chat_pcodes, templineC2+1);
				}
			} else {
				toline[ti++] = ' ';
				strcpy(toline+ti, templineC2);
				ti = strlen(toline);
				if (templineC2[0] != '+')
					toline[ti++] = ' ';
			}
		} else if (templineC2[0] != EOS) {
			for (wi=0; wi < 50; wi++)
				vw[wi] = NULL;
			for (wi=0; wi < 50; wi++)
				sw[wi] = NULL;
			if (isLangCodeFound)
				breakupLangCodeWords(line, fi, templineC2, vw, sw);
			else
				breakupWords(templineC2, vw, sw);
			toline[ti] = EOS;
			for (wi=0; wi < 50; wi++) {
				if (vw[wi] == NULL && sw[wi] == NULL)
					break;
				if (vw[wi] == NULL/* || vw[wi][0] == EOS */) {
					if (wi == 0)
						strcat(toline, " 0");
				} else {
					if (wi == 0)
						strcat(toline, " ");
					else
						strcat(toline, "_");
					uS.lowercasestr(vw[wi], &dFnt, MBF);
					strcat(toline, vw[wi]);
				}
				if (sw[wi] == NULL/* || sw[wi][0] == EOS */) {
					if (wi == 0)
						strcat(sin, " 0");
				} else {
					if (wi == 0)
						strcat(sin, " s:");
					else
						strcat(sin, "_");
					strcat(sin, sw[wi]);
				}
			}
			ti = strlen(toline);
		}
	}
	toline[ti] = EOS;
	strcpy(line, toline);
}

static char isOvertalk(char *st, char isRemove) {
	int i, iC, iChild, iO;

	iC = -1;
	iChild = -1;
	iO = -1;
	for (i=0; st[i] != EOS; i++) {
		if (iC == -1 && ((i == 0 && uS.mStrnicmp(st+i, "C ", 2) == 0) ||
			(i > 0 && isSpace(st[i-1]) && uS.mStrnicmp(st+i, "C ", 2) == 0)))
			iC = i;
		if (iChild == -1 && uS.mStrnicmp(st+i, "Child ", 6) == 0)
			iChild = i;
		if ((iC != -1 || iChild != -1) && uS.mStrnicmp(st+i, "overtalk", 8) == 0)
			iO = i;
	}
	if (iC != -1 && iO != -1) {
		if (isRemove) {
			strcpy(st+iC, st+iC+1);
			iO--;
			strcpy(st+iO, st+iO+8);
			removeExtraSpace(st);
			remJunkFrontAndBack(st);
			for (i=0; st[i] == '\t' || st[i] == ' ' || st[i] == ','; i++) ;
			if (i > 0)
				strcpy(st, st+i);
		}
		return(TRUE);
	}
	if (iChild != -1 && iO != -1) {
		if (isRemove) {
			strcpy(st+iChild, st+iChild+5);
			iO -= 5;
			strcpy(st+iO, st+iO+8);
			removeExtraSpace(st);
			remJunkFrontAndBack(st);
			for (i=0; st[i] == '\t' || st[i] == ' ' || st[i] == ','; i++) ;
			if (i > 0)
				strcpy(st, st+i);
		}
		return(TRUE);
	}
	return(FALSE);
}

static void cleanComment(char *comment) {
	int i;

	for (i=0; comment[i] != EOS; i++) {
		if (comment[i] == '\001')
			comment[i] = ',';
	}
}

static void parseLineIntoSingleUtterance(char *sp, char *line) {
	int  b, e;
	char t;

	b = 0;
	while (line[b] != EOS) {
		for (e=b; line[e] != '!' && line[e] != '?' && line[e] != '.' && line[e] != EOS; e++) ;
		if (line[e] == EOS) {
			strcat(line+e, ".");
		}
		for (; line[e] == '!' || line[e] == '?' || line[e] == '.'; e++) ;
		if (line[e] == (char)0xE2 && line[e+1] == (char)0x80 && line[e+2] == (char)0x9D) {
			if (e > 1 && line[e-2] == ' ' && (line[e-1] == '!' || line[e-1] == '?' || line[e-1] == '.')) {
				t = line[e-1];
				line[e-2] = (char)0xE2;
				line[e-1] = (char)0x80;
				line[e]   = (char)0x9D;
				line[e+1] = ' ';
				line[e+2] = t;

			}
			e = e + 3;
		}
		t = line[e];
		line[e] = EOS;
		while (isSpace(line[b]))
			   b++;
		printout(templineC, line+b, NULL, NULL, TRUE);
		line[e] = t;
		b = e;
	}
}

static void fixTabs(char *line) {
	if (line[0] == '@' && (line[1] == 'g' || line[1] == 'G') && line[2] == ':' && line[3] == ' ') {
		line[1] = 'G';
		line[3] = '\t';
		if (isSpace(line[4]))
			strcpy(line+4, line+5);
		if (isSpace(line[4]))
			strcpy(line+4, line+5);
		if (isSpace(line[4]))
			strcpy(line+4, line+5);
	}
}

static void prline(char tcode, char *line, char *part) {
	char *sp;
	struct depTiers *t;

	for (; isSpace(*line); line++) ;
	changenames(tcode,line);
	fixupnumbers(tcode,line);
	cleanup_line(tcode, line);
	if ((sp=findsp(tcode)) != NULL) {
		if (part[0] != EOS) {
			processPartComm(part, comment);
		}
		if (salt2chat_pcodes[0] != EOS) {
			if (coding == 3)
				uS.lowercasestr(salt2chat_pcodes, &dFnt, MBF);
			strcat(line, salt2chat_pcodes);
			salt2chat_pcodes[0] = EOS;
		}
		if (coding == 3) {
			if (overlap == -1)
				overlap = 0;
			spareTier2[0] = EOS;
			removeExtraSpace(line);
			Sophie_AddZero(line);
			Sophie_cleanupSpeaker(line);
			Sophie_extractSin(line, templineC1, spareTier2);
			Sophie_CapitalizeCleanup(line);
			uS.lowercasestr(spareTier2, &dFnt, MBF);
			if (salt2chat_pcodes[0] != EOS) {
				uS.lowercasestr(salt2chat_pcodes, &dFnt, MBF);
				uS.remFrontAndBackBlanks(line);
				strcat(line, salt2chat_pcodes);
				salt2chat_pcodes[0] = EOS;
			}
			Sophie_AddZero(line);
		} else {
			Sophie_AddZero(line);
			Sophie_cleanupSpeaker(line);
		}
		uS.remFrontAndBackBlanks(line);
		removeExtraSpace(line);
		strcpy(templineC, sp);
		if (*templineC == '*') {
			SpFound = TRUE;
			uS.remFrontAndBackBlanks(comm);
			removeExtraSpace(comm);
			if (coding != 3 && *comm != EOS) {
				printout("%com:", comm, NULL, NULL, TRUE);
				comm[0] = EOS;
			}
			uS.remFrontAndBackBlanks(def_line);
			removeExtraSpace(def_line);
			if (*def_line != EOS) {
				printout("%def:", def_line, NULL, NULL, TRUE);
				def_line[0] = EOS;
			}
			remJunkFrontAndBack(comment);
			removeExtraSpace(comment);
			if (comment[0] != EOS) {
				cleanComment(comment);
				printout("@Comment:", comment, NULL, NULL, TRUE);
				comment[0] = EOS;
			}
			uS.uppercasestr(templineC+1, &dFnt, MBF);
		} else
			if (*templineC == '%')
				uS.lowercasestr(templineC+1, &dFnt, MBF);
		if (coding == 1 && !uselcode && *lcode) {
			strcat(line, lcode);
			*lcode = EOS;
		}
		if (uselcode && headTier != NULL) {
			for (t=headTier; t != NULL; t=t->nextTier) {
				if (*t->line && !strcmp(t->tier, "*")) {
					addPostCodes(line, t->line);
					t->line[0] = EOS;
				}
			}
		}
		if (coding == 7) {
			if (tcode == '@') {
				fixTabs(uttline);
				fprintf(fpout, "%s\n", uttline);
			} else if (*line != EOS) {
				fixuptime(line);
				parseLineIntoSingleUtterance(templineC, line);
			} else {
				strcpy(line,"0 .");
				printout(templineC, line, NULL, NULL, TRUE);
			}
		} else if (coding != 8 && checkline(line)) {
			strcpy(templineC1, "0 .");
			printout(templineC,templineC1, NULL, NULL, TRUE);
			if (coding == 3)
				strcpy(spareTier2, "0");
			fixuptime(line);
			printout("%act:", line, NULL, NULL, TRUE);
		} else if (*line != EOS) {
			fixuptime(line);
			printout(templineC, line, NULL, NULL, TRUE);
		} else {
			strcpy(line,"0 .");
			printout(templineC, line, NULL, NULL, TRUE);
			if (coding == 3)
				strcpy(spareTier2, "0");
		}

		if (coding == 3) {
			uS.remFrontAndBackBlanks(spareTier2);
			removeExtraSpace(spareTier2);
			if (spareTier2[0] != EOS)
				printout("%sin:", spareTier2, NULL, NULL, TRUE);
			spareTier2[0] = EOS;
			uS.remFrontAndBackBlanks(comm);
			removeExtraSpace(comm);
			if (*comm != EOS) {
				printout("%com:", comm, NULL, NULL, TRUE);
				comm[0] = EOS;
			}
		}
		if (uselcode) {
			if (headTier != NULL) {
				for (t=headTier; t != NULL; t=t->nextTier) {
					if (*t->line && strcmp(t->tier, "*")) {
						sprintf(templineC, "%%%s:", t->tier);
						printout(templineC, t->line, NULL, NULL, TRUE);
						t->line[0] = EOS;
					}
				}
			}
			uS.remFrontAndBackBlanks(lcode);
			removeExtraSpace(lcode);
			if (*lcode) {
				cleanCodes(lcode);
				printout("%cod:", lcode, NULL, NULL, TRUE);
				*lcode = EOS;
			}
		}
		if (brerror) {
			fprintf(fpout,"%%xov:\t%s\n", "ERROR: <> brackets misused");
		}
		brerror = FALSE;
		pos = 0;
		*utterance->line = EOS;
		if (overlap > 0 && toupper((unsigned char)overlap) != toupper((unsigned char)curspeaker) &&
			toupper((unsigned char)oldsp) != toupper((unsigned char)curspeaker)) {
			fprintf(fpout,"%%xov:\t%s\n", "ERROR: Can't find matching closing overlap for line:");
			fprintf(fpout,"\t%s\n", spareTier1);
			overlap = 0;
		}
	} else if (tcode == '+') {
		cleancom(line);
		for (; isSpace(*line); line++) ;
		if (*line) {
			int i;

			if (deff) {
				if (*def_line != EOS)
					strcat(def_line, ", ");
				strcat(def_line, line);
			} else if ((*line == 'i' || *line == 'I')		  &&
					   (*(line+1) == 'd' || *(line+1) == 'D') &&
					   *(line+2) == ' ' && (coding != 4 || part[0] == EOS)) {
				line[2] = '=';
				i = strlen(line) - 1;
				for (; i >= 0 && line[i] == ' ' && line[i] == '.'; i--) ;
				line[++i] = EOS;
				if (*line) {
					if (*comment != EOS)
						strcat(comment, ", ");
					strcat(comment, line);
				}
			} else {
				for (; isSpace(*line); line++) ;
				if (coding == 3 && overlap != 0 && isOvertalk(line, TRUE)) {
					if (*line) {
						if (comment[0] != EOS)
							strcat(comment, ", ");
						strcat(comment, line);
						remJunkFrontAndBack(comment);
						removeExtraSpace(comment);
						if (comment[0] != EOS) {
							cleanComment(comment);
							printout("@Comment:", comment, NULL, NULL, TRUE);
						}
						comment[0] = EOS;
					}
					strcpy(templineC1, "<0> [<] .");
					printout("*CHI:", templineC1, NULL, NULL, TRUE);
					strcpy(templineC1, "0");
					printout("%sin:", templineC1, NULL, NULL, TRUE);
					overlap = 0;
					oldsp = 'C';
				} else if (*line) {
					if (coding == 3 && overlap == 0 && isOvertalk(line, FALSE)) {
						overlap = -1;
						oldsp = 'C';
					}
					if (comment[0] != EOS)
						strcat(comment, "\001 ");
					strcat(comment, line);
				}
			}
		}
		deff = FALSE;
		pos = 0;
		*utterance->line = EOS;
	} else if (tcode == '-') {
		fixuptime(line);
		if (coding == 2)
			;
		else if (SpFound) {
			if (coding == 8) {
				printout("%tim:", line, NULL, NULL, TRUE);
			}
		} else {
			if (coding == 0 || coding == 4 || coding == 5 || coding == 6 || coding == 7 || coding == 8) {
				if (part[0] != EOS) {
					processPartComm(part, comment);
				}
			}
			printout("@Time Start:", line, NULL, NULL, TRUE);
		}
		pos = 0;
		*utterance->line = EOS;
	} else if (tcode == '=') {
		if (coding == 0 || coding == 4 || coding == 5 || coding == 6 || coding == 7 || coding == 8) {
			if (part[0] != EOS) {
				processPartComm(part, comment);
			}
		}
		cleancom(line);
		for (; isSpace(*line); line++) ;
		if (*line) {
			if (coding == 3 || !SpFound) {
				if (*comment != EOS)
					strcat(comment, ", ");
				strcat(comment, line);
			} else {
				if (*comm != EOS)
					strcat(comm, ", ");
				strcat(comm, line);
			}
		}
		pos = 0;
		*utterance->line = EOS;
	} else if (tcode == ';' || tcode == ':') {
		strcat(tim,utterance->line);
		pos = 0;
		*utterance->line = EOS;
	} else {
		fprintf(stderr,"*** File \"%s\": line %ld\n", oldfname, lnum);
		fprintf(stderr, "ERROR: unknown symbol <%c> found.\n", tcode);
	}
}

static char isAlreadyParan(char *st, int i) {
	int t;

	for (t=i; isdigit(st[i]) && i >= 0; i--) ;
	if (i >= 0 && st[i] == '(')
		return(TRUE);
	return(FALSE);
}

static int fix(char *st, int i) {
	int j, t;

	j = i + 1;
	st[j+1] = st[j];
	for (t=j; isdigit(st[i]) && i >= 0; i--)
		st[t--] = st[i];
	st[++i] = '(';
	return(j+1);
}

static char rightSyllable(char *st, int pos) {
	int i, vCnt;

	vCnt = 0;
	for (i=pos; i >= 0 && !uS.isskip(st,i,&dFnt,MBF); i--) {
		if (uS.isVowel(st+i) && !uS.isVowel(st+i+1) && i > 0 && !uS.isskip(st,i-1,&dFnt,MBF))
			vCnt++;
		else if (uS.isVowel(st+i) && !uS.isVowel(st+i+1)  && (i == 0 || uS.isskip(st,i-1,&dFnt,MBF))) {
			if (uS.mStrnicmp(st+i, "open", pos-i+1) == 0 || uS.mStrnicmp(st+i, "answer", pos-i+1) == 0)
				vCnt++;
		}
	}
	if (vCnt > 1 && uS.mStrnicmp(st+pos-2,"gin",3) != 0 && uS.mStrnicmp(st+pos-2,"ret",3) != 0)
		return(FALSE);
	return(TRUE);
}

static char isExceptionLet(char c) {
/* original list
	if (c == 'w' || c == 'x' || c == 'y' ||
		c == 'W' || c == 'X' || c == 'Y')
*/
/* new list
	if (c == 'w' || c == 'x' || c == 'y' || c == 'r' || c == 's' || c == 'n' ||
		c == 'W' || c == 'X' || c == 'Y' || c == 'R' || c == 'S' || c == 'N')
*/
	if (c == 'w' || c == 'x' || c == 'y' || c == 's' ||
		c == 'W' || c == 'X' || c == 'Y' || c == 'S')
		return(TRUE);
	return(FALSE);
}

static char isCodeEOFound(char *s) {
	for (; isalpha(*s); s++) ;
	for (; isSpace(*s); s++) ;
	if (*s == '[')
		return(TRUE);
	else
		return(FALSE);
}

static char *parseline(char *s, char isComment) {
	int  i;
	char *st = utterance->line;

	if (coding == 7) {
		if (isSingleDelim(s)) {
			if (curspeaker != ':' && curspeaker != '-' && curspeaker != ';') {
				if (!isComment && isalnum(curspeaker) && pos > 0 && !isSpace(st[pos-1]))
					st[pos++] = ' ';
				st[pos++] = *s;
			}
			s++;
		} else if (*s == 'I' || *s == 'i') {
			if (pos == 0 || !isalpha(st[pos-1])) {
				s++;
				if (!isalpha(*s)) {
					if (*s != ']')
						st[pos++] = 'I';
					else
						st[pos++] = 'i';
				} else {
					if (isalpha(curspeaker)) {
						st[pos++] = 'i';
						st[pos++] = Lisupper(*s, FALSE);
					} else {
						st[pos++] = *(s-1);
						st[pos++] = *s;
					}
					s++;
				}
			} else {
				st[pos++] = 'i';
				s++;
			}
		} else {
			st[pos++] = Lisupper(*s, FALSE);
			s++;
		}
	} else if (*s == '|' && curspeaker != '=' && curspeaker != '+') {
		s++;
		if (!isCodeEOFound(s)) {
			st[pos++] = ' ';
			st[pos++] = '[';
			st[pos++] = ':';
			st[pos++] = ' ';
			for (; isalpha(*s) || *s == '\'' || *s == '/'; s++) {
				if (*s == '/') {
					s = parseline(s, isComment);
					s--;
				} else
					st[pos++] = *s;
			}
			st[pos++] = ']';
			st[pos++] = ' ';
		} else {
			for (; isalpha(*s); s++) ;
		}
	} else if (*s == '^' && curspeaker != '=' && curspeaker != '+') {
		st[pos] = EOS;
		if (coding == 0 || coding == 6 || coding == 7 || coding == 8) {
			strcat(st," xxx");
			pos += 4;
		}
		strcat(st," +/.");
		pos += 4;
		s++;
	} else if ((*s == '>' || *s == '~') && curspeaker != '=' && curspeaker != '+') {
		st[pos] = EOS;
		if (coding == 0 || coding == 6 || coding == 7 || coding == 8) {
			strcat(st," xxx");
			pos += 4;
		}
		strcat(st," +...");
		pos += 5;
		s++;
	} else if (*s == ',' && curspeaker != '=' && curspeaker != '+') {
		while (pos >= 0 && st[pos-1] == ' ')
			pos--;
		if (curspeaker != ':' && curspeaker != '-' && curspeaker != ';')
			st[pos++] = *s;
		s++;
		if (!isSpace(*s) && *s != ')' && *s != ']' && *s != '}' && *s != '>')
			st[pos++] = ' ';
	} else if (isSingleDelim(s)) {
		if (curspeaker != ':' && curspeaker != '-' && curspeaker != ';') {
			if (!isComment && isalnum(curspeaker) && pos > 0 && !isSpace(st[pos-1]))
				st[pos++] = ' ';
			st[pos++] = *s;
		}
		s++;
	} else if (*s == ':' && !isComment && curspeaker != '=' && curspeaker != '+' && curspeaker != '-' && curspeaker != '=') {
		s++;
		if (pos > 0 && isdigit(st[pos-1])) {
			char isParanFound = isAlreadyParan(st, pos-1);

			if (curspeaker != '-' && !isParanFound) {
				st[pos] = EOS;
				pos = fix(st, pos-1);
				st[pos++] = ':';
			} else
				st[pos++] = ':';
			if (isdigit(*s)) {
				do {
					st[pos++] = *s;
					s++;
				} while (isdigit(*s) || *s == ':') ;
			}
			if (curspeaker != '-') {
				st[pos++] = '.';
				if (!isParanFound)
					st[pos++] = ')';
			}
		} else {
			if (isdigit(*s)) {
				if (curspeaker != '-') {
					st[pos++] = '(';
				}
				st[pos++] = ':';
				do {
					st[pos++] = *s;
					s++;
				} while (isdigit(*s)) ;
				st[pos++] = '.';
				st[pos++] = ')';
			} else if (!isalpha(st[pos-1]) && !isalpha(*s)) {
				if (curspeaker != '-') {
					st[pos++] = '(';
					st[pos++] = '.';
					st[pos++] = ')';
				}
			} else
				st[pos++] = ':';
		}
	} else if (*s == '/' && curspeaker != '=' && curspeaker != '+' && curspeaker != '-') {
		char isStarFound = FALSE;

		s++;
		if (*s == '*') {
/*
			int j, t;

			i = pos - 1;
			t = pos;
			while (!uS.isskip(st,i,&dFnt,MBF) && i >= 0)
				i--;
			i++;
			if (i < t) {
				isStarFound = TRUE;
				st[t] = '-';
				j = (t - i) + 4;
				uS.shiftright(st+t, j);
				pos = pos + j;
				j = t;
				st[j++] = ' ';
				st[j++] = '[';
				st[j++] = '*';
				st[j++] = ' ';
				while (i < t)
					st[j++] = st[i++];
			}
*/
			isStarFound = TRUE;
			st[pos++] = ' ';
			st[pos++] = '[';
			st[pos++] = '*';
			st[pos++] = ' ';
			st[pos++] = 'm';
			st[pos++] = ':';
			st[pos++] = '0';
			s++;
		}
tryagain:
		if (*s == '3') {
			s++;
			if (isdigit(*s) || *s == '/') {
				st[pos++] = '3';
				if (*s == '/') {
					if (isStarFound)
						st[pos++] = '-';
					else
						st[pos++] = '^';
				} else
					st[pos++] = Lisupper(*s, FALSE);
			} else {
				if (isStarFound)
					st[pos++] = '3';
				if (pos > 0 && (st[pos-1] == 's' || st[pos-1] == 'z')) {
					st[pos++] = 'e';
				} else if (pos > 1 && ((st[pos-2] == 's' && st[pos-1] == 'h') ||
									   (st[pos-2] == 'c' && st[pos-1] == 'h') ||
									   (st[pos-2] == 'd' && st[pos-1] == 'z'))) {
					st[pos++] = 'e';
				} else if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && !uS.isVowel(st+pos-2)) {
					pos--;
					st[pos++] = 'i';
					st[pos++] = 'e';
				} else if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && uS.mStrnicmp(st+pos-2,"e",1)==0) {
					pos--;
					pos--;
					st[pos++] = 'i';
					st[pos++] = 'e';
				} else if (pos > 0 && (st[pos-1] == 's' || st[pos-1] == 'S' ||
								st[pos-1] == 'o' || st[pos-1] == 'O'))
					st[pos++] = 'e';
				st[pos++] = Lisupper(*s, FALSE);
			}
			s++;
		} else if (*s == 'D' || *s == 'd') {
			s++;
			if (*s == '\'') {
				s++;
				if (*s == 'S' || *s == 's') {
					st[pos++] = ' ';
					st[pos++] = '(';
					st[pos++] = 'd';
					st[pos++] = 'o';
					st[pos++] = 'e';
					st[pos++] = ')';
					st[pos++] = 's';
					s++;
				} else if (*s == 'D' || *s == 'd') {
					st[pos++] = ' ';
					st[pos++] = '(';
					st[pos++] = 'd';
					st[pos++] = 'i';
					st[pos++] = ')';
					st[pos++] = 'd';
					s++;
				} else {
					st[pos++] = 'd';
					st[pos++] = '\'';
				}
			} else {
				if (pos > 0 && !uS.isskip(st,pos-1,&dFnt,MBF))
					st[pos++] = 'e';
				st[pos++] = 'd';
			}
		} else if (*s == 'H' || *s == 'h') {
			s++;
			if (*s == '\'') {
				s++;
				if (*s == 'S' || *s == 's') {
					st[pos++] = ' ';
					st[pos++] = '(';
					st[pos++] = 'h';
					st[pos++] = 'a';
					st[pos++] = ')';
					st[pos++] = 's';
					s++;
				} else {
					st[pos++] = 'h';
					st[pos++] = '\'';
					if (isStarFound) {
						st[pos++] = Lisupper(*s, FALSE);
						s++;
					}
				}
			} else {
				st[pos++] = 'h';
			}
		} else if (*s == 'E' || *s == 'e') {
			s++;
			if (*s == 'D' || *s == 'd') {
				if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && !uS.isVowel(st+pos-2)) {
					pos--;
					st[pos++] = 'i';
					st[pos++] = 'e';
				} else if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && uS.mStrnicmp(st+pos-2,"e",1)==0) {
					pos--;
					pos--;
					st[pos++] = 'i';
					st[pos++] = 'e';
				} else if (pos > 2 && uS.isConsonant(st+pos-1) && uS.isVowel(st+pos-2) && uS.isConsonant(st+pos-3)) {
					if (!isExceptionLet(st[pos-1]) && rightSyllable(st, pos-1)) {
						st[pos] = Lisupper(st[pos-1], FALSE);
						pos++;
					}
					st[pos++] = 'e';
				} else if (pos > 0 && (st[pos-1] != 'e' && st[pos-1] != 'E')) {
					st[pos++] = 'e';
				}
				st[pos++] = Lisupper(*s, FALSE);
				s++;
			} else if (*s == 'R' || *s == 'r') {
				if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && !uS.isVowel(st+pos-2)) {
					pos--;
					st[pos++] = 'i';
					st[pos++] = 'e';
				} else if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && uS.mStrnicmp(st+pos-2,"e",1)==0) {
					pos--;
					pos--;
					st[pos++] = 'i';
					st[pos++] = 'e';
				} else if (pos > 2 && uS.isConsonant(st+pos-1) && uS.isVowel(st+pos-2) && uS.isConsonant(st+pos-3)) {
					if (!isExceptionLet(st[pos-1])) {
						st[pos] = Lisupper(st[pos-1], FALSE);
						pos++;
						st[pos++] = 'e';
					}
				} else if (pos > 0 && (st[pos-1] != 'e' && st[pos-1] != 'E'))
					st[pos++] = 'e';
				st[pos++] = Lisupper(*s, FALSE);
				s++;
			} else if ((*s == 'S' || *s == 's') && ((*(s+1) == 'T' || *(s+1) == 't') || !isalpha(*(s+1)))) {
				if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && !uS.isVowel(st+pos-2)) {
					pos--;
					st[pos++] = 'i';
					st[pos++] = 'e';
				} else if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && uS.mStrnicmp(st+pos-2,"e",1)==0) {
					pos--;
					pos--;
					st[pos++] = 'i';
					st[pos++] = 'e';
				} else if (pos > 2 && uS.isConsonant(st+pos-1) && uS.isVowel(st+pos-2) && uS.isConsonant(st+pos-3)) {
					if (!isExceptionLet(st[pos-1])) {
						st[pos] = Lisupper(st[pos-1], FALSE);
						pos++;
						st[pos++] = 'e';
					}
				} else if (pos > 0 && (st[pos-1] != 'e' && st[pos-1] != 'E'))
					st[pos++] = 'e';
				st[pos++] = Lisupper(*s, FALSE);
				s++;
			} else {
				st[pos++] = 'e';
			}
		} else if (*s == 'S' || *s == 's') {
			if (pos > 0 && (st[pos-1] == 's' || st[pos-1] == 'x' || st[pos-1] == 'z')) {
				st[pos++] = 'e';
			} else if (pos > 1 && ((st[pos-2] == 's' && st[pos-1] == 'h') ||
								   (st[pos-2] == 'c' && st[pos-1] == 'h') ||
								   (st[pos-2] == 'd' && st[pos-1] == 'z'))) {
				st[pos++] = 'e';
			} else if ((pos > 7 && uS.mStrnicmp(st+pos-8, "scratchy", 8) == 0)) {
				pos--;
				st[pos++] = 'e';
			} else if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && !uS.isVowel(st+pos-2)) {
				pos--;
				st[pos++] = 'i';
				st[pos++] = 'e';
			} else if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0 && uS.mStrnicmp(st+pos-2,"e",1)==0) {
				pos--;
				pos--;
				st[pos++] = 'i';
				st[pos++] = 'e';
			}
			st[pos++] = 's';
			s++;
			if (*s == '/') {
				s++;
				if (*s == '*' && (*(s+1) == 'Z' || *(s+1) == 'z')) {
					s += 2;
					st[pos++] = '\'';
				} else if (*s == 'Z' || *s == 'z') {
					st[pos++] = '\'';
					s++;
				} else {
					goto tryagain;
				}
			}
		} else if (*s == '\'') {
			s++;
			if (*s == 'N' || *s == 'n') {
				s++;
				if (*s == '\'') {
					st[pos++] = 'n';
				} else {
					st[pos++] = '\'';
					st[pos++] = 'n';
					if (isStarFound) {
						st[pos++] = Lisupper(*s, FALSE);
						s++;
					}
				}
			} else if (*s == 'U' || *s == 'u') {
				s++;
				if (*s == 'S' || *s == 's') {
					st[pos++] = '\'';
				} else {
					st[pos++] = '\'';
					st[pos++] = 'u';
					if (isStarFound) {
						st[pos++] = Lisupper(*s, FALSE);
						s++;
					}
				}
			} else {
				st[pos++] = '\'';
				if (isStarFound) {
					st[pos++] = Lisupper(*s, FALSE);
					s++;
				}
			}
		} else if (*s == 'R' || *s == 'r') {
			s++;
			if (!isalpha(*s)) {
				st[pos++] = 'e';
			} else {
				st[pos++] = '\'';
			}
			st[pos++] = 'r';
		} else if (*s == 'M' || *s == 'm') {
			if (pos > 0 && (st[pos-1] != '\''))
				st[pos++] = '\'';
			st[pos++] = 'm';
			s++;
		} else if (*s == 'Z' || *s == 'z') {
			st[pos++] = '\'';
			st[pos++] = 's';
			s++;
		} else if (*s == 'N' || *s == 'n') {
			s++;
			if (*s == '\'') {
				st[pos++] = 'n';
				st[pos++] = '\'';
				s++;
			} else if (!isalpha(*s)) {
				st[pos++] = 'e';
				st[pos++] = 'n';
				if (isStarFound) {
					st[pos++] = Lisupper(*s, FALSE);
					s++;
				}
			} else {
				st[pos++] = 'n';
				if (isStarFound) {
					st[pos++] = Lisupper(*s, FALSE);
					s++;
				}
			}
		} else if (*s == 'I' || *s == 'i') {
			st[pos++] = 'i';
			s++;
			if (*s == '|') {
			} else if (*s == 'N' || *s == 'n') {
				st[pos++] = 'n';
				s++;
				if (*s == 'G' || *s == 'g') {
					if (pos > 4 && uS.isConsonant(st+pos-3) && uS.isVowel(st+pos-4) && uS.isConsonant(st+pos-5)) {
						if (!isExceptionLet(st[pos-3]) && rightSyllable(st, pos-3)) {
							st[pos-2] = Lisupper(st[pos-3], FALSE);
							st[pos-1] = 'i';
							st[pos++] = 'n';
						}
					} else if (pos > 2 && (st[pos-3] == 'E' || st[pos-3] == 'e')) {
						if (pos > 4 && (st[pos-4] == 'B' || st[pos-4] == 'b') && !isalpha(st[pos-5])) {
						} else if (pos == 4 && (st[pos-4] == 'B' || st[pos-4] == 'b')) {
						} else if (pos > 3 && (st[pos-4] == 'E' || st[pos-4] == 'e')) {
						} else {
							if (pos > 3 && (st[pos-4] == 'I' || st[pos-4] == 'i')) {
								st[pos-4] = 'y';
							}
							pos -= 3;
							st[pos] = st[pos+1];
							pos++;
							st[pos] = st[pos+1];
							pos++;
						}
					}
					st[pos++] = 'g';
					s++;
				} else if (*s == '|') {
					if (pos > 4 && uS.isConsonant(st+pos-3) && uS.isVowel(st+pos-4) && uS.isConsonant(st+pos-5)) {
						if (!isExceptionLet(st[pos-3])) {
							st[pos-2] = Lisupper(st[pos-3], FALSE);
							st[pos-1] = 'i';
							st[pos++] = 'n';
							if (isStarFound) {
								st[pos++] = Lisupper(*s, FALSE);
								s++;
							}
						}
					}
				} else {
					st[pos++] = '(';
					st[pos++] = *s;
					st[pos++] = ')';
				}
			} else if (*s == 'E' || *s == 'e') {
				st[pos++] = 'e';
				s++;
				if (*s == 'S' || *s == 's') {
					if (isStarFound)
						st[pos++] = '-';
					else
						st[pos++] = '^';
					st[pos++] = 's';
					s++;
				}
			}
		}
		if (isStarFound) {
			st[pos++] = ']';
		}
	} else if (*s == 'I' || *s == 'i') {
		if (pos == 0 || !isalpha(st[pos-1])) {
			s++;
			if (!isalpha(*s)) {
				if (*s != ']')
					st[pos++] = 'I';
				else
					st[pos++] = 'i';
			} else {
				if (isalpha(curspeaker)) {
					st[pos++] = 'i';
					st[pos++] = Lisupper(*s, FALSE);
				} else {
					st[pos++] = *(s-1);
					st[pos++] = *s;
				}
				s++;
			}
		} else {
			st[pos++] = 'i';
			s++;
		}
	} else if (*s == '%') {
		s++;
		while (isalnum((unsigned char)*s) || *s == '_') {
			st[pos++] = Lisupper(*s, FALSE);
			s++;
		}
		st[pos] = EOS;
		if (isComment)
			strcat(st," percent");
		else
			strcat(st,"@c");
		pos = strlen(st);
	} else if (*s == '*') {
		if (pos > 1 && isSpace(st[pos-1]) && isSpace(s[1]) && (isalnum((unsigned char)st[pos-2]) || st[pos-2] == ')')) {
			i = pos - 2;
			while (!uS.isskip(st,i,&dFnt,MBF) && i >= 0) {
				st[i+1] = st[i];
				i--;
			}
			st[++i] = '&';
			s++;
		} else if (pos > 0 && (isalnum((unsigned char)st[pos-1]) || st[pos-1] == ')')) {
			i = pos - 1;
			while (!uS.isskip(st,i,&dFnt,MBF) && i >= 0) {
				st[i+1] = st[i];
				i--;
			}
			st[++i] = '&';
			pos++;
			s++;
		} else {
			s++;
			if (isalnum((unsigned char)*s))
				st[pos++] = '0';
			else
				st[pos++] = '*';
		}
	} else if (!isComment && (*s == 'X' || *s == 'x') && (pos == 0 || !isalnum((unsigned char)st[pos-1]))) {
		int num = 1;

		s++;
		while(*s == 'X' || *s == 'x') {
			s++;
			num++;
		}
		st[pos] = EOS;
		if (num == 1) {
			if (!isalnum((unsigned char)*s)) {
				strcat(st, "xxx");
				pos += 3;
			} else
				st[pos++] = 'x';
		} else {
			if (isalnum((unsigned char)*s))
				st[pos++] = '&';
			else {
				strcat(st, "xxx");
				pos += 3;
			}
		}
	} else if (!isComment && (*s == 'X' || *s == 'x') && pos != 0 && isalnum((unsigned char)st[pos-1])) {
		s++;
		if (*s == 'x' || *s == 'X') {
			do {
				s++;
			} while(*s == 'X' || *s == 'x') ;
			i = pos - 1;
			while (!uS.isskip(st,i,&dFnt,MBF) && i >= 0) {
				st[i+1] = st[i];
				i--;
			}
			st[++i] = '&';
			pos++;
		} else
			st[pos++] = 'x';
	} else {
		st[pos++] = Lisupper(*s, FALSE);
		s++;
	}
	return(s);
}


static void checkcodes(int lpos) {
	struct scodes *temp;
	for (temp=shead; temp != NULL; temp=temp->snext) {
		if (!strcmp(temp->scode,utterance->line+lpos)) {
			deff = TRUE;
			break;
		}
	}
}

static void addtocodes(int lpos) {
	struct scodes *temp;
	
	temp = NEW(struct scodes);
	temp->scode = (char *)malloc(strlen(utterance->line+lpos)+1);
	strcpy(temp->scode,utterance->line+lpos);
	temp->snext = shead;
	shead = temp;
}

static char isSophiePostcode(char *line, int len) {
	if (coding != 3)
		return(FALSE);
	if (uS.mStrnicmp(line, "noresponse", len) == 0 || uS.mStrnicmp(line, "clarQ", len) == 0 ||
		uS.mStrnicmp(line, "nc", len) == 0 || uS.mStrnicmp(line, "np", len) == 0 ||
		uS.mStrnicmp(line, "na", len) == 0 || uS.mStrnicmp(line, "nr", len) == 0 ||
		uS.mStrnicmp(line, "jl", len) == 0 || uS.mStrnicmp(line, "EU", len) == 0 ||
		uS.mStrnicmp(line, "voc", len) == 0 ||
		uS.mStrnicmp(line, "NR:", 3) == 0 || uS.mStrnicmp(line, "R?:", 3) == 0 ||
		uS.mStrnicmp(line, "RE:", 3) == 0 ||uS.mStrnicmp(line, "R:", 2) == 0 ||
		uS.mStrnicmp(line, "EU:", 3) == 0)
		return(TRUE);
	return(FALSE);
}

static char isSophieAmpersandCode(char *line, char *code) {
	int i;

	if (coding != 3)
		return(FALSE);
	while (isSpace(*line))
		line++;
	if (uS.mStricmp(line, "o") == 0 || uS.mStricmp(line, "wl") == 0 || uS.mStricmp(line, "g") == 0) {
		strcpy(code, line);
		for (i=strlen(code)-1; i >= 0 && (isSpace(code[i]) || code[i] == ']'); i--) ;
		i++;
		code[i] = EOS;
		if (code[0] == 'g') {
			uS.lowercasestr(code, &dFnt, C_MBF);
			if (code[1] == EOS)
				strcat(code, ":");
		} else
			uS.uppercasestr(code, &dFnt, C_MBF);
		return(TRUE);
	} else if (uS.mStricmp(line, "GP") == 0 || uS.mStricmp(line, "G:POINT") == 0 || uS.mStricmp(line, "GES:PT") == 0 ||
			   uS.mStricmp(line, "POINT") == 0 || uS.mStricmp(line, "POINTS") == 0) {
		strcpy(code, "g:point");
		return(TRUE);
	} else if (uS.mStricmp(line, "G:REACH") == 0) {
		strcpy(code, "g:reach");
		return(TRUE);
	} else if (uS.mStricmp(line, "GES:NOD") == 0 || uS.mStricmp(line, "NODS") == 0 || uS.mStricmp(line, "GN") == 0) {
		strcpy(code, "g:nod");
		return(TRUE);
	} else if (uS.mStricmp(line, "GES:TOUCH") == 0) {
		strcpy(code, "g:touch");
		return(TRUE);
	} else if (uS.mStricmp(line, "GES") == 0) {
		strcpy(code, "g:");
		return(TRUE);
	} else if (uS.mStricmp(line, "ghs") == 0) {
		strcpy(code, "g:headshake");
		return(TRUE);
	} else if (uS.mStricmp(line, "go") == 0) {
		strcpy(code, "g:other");
		return(TRUE);
	} 
	return(FALSE);
}

static char isSophieErrorOrPostcode(char *line, int lpos, char *code) {
	char isLastCharSkip;

	if (coding != 3)
		return(FALSE);
	if (lpos - 3 >= 0)
		isLastCharSkip = uS.isskip(line, lpos-3, &dFnt, MBF);
	else
		isLastCharSkip = TRUE;
	line = line + lpos + 2;
	while (isSpace(*line))
		line++;
	code[0] = EOS;
	if (uS.mStricmp(line, "im") == 0 || uS.mStricmp(line, "IMIT") == 0 || uS.mStricmp(line, "IMM") == 0 ||
		uS.mStricmp(line, "IM_SIGN") == 0) {
		if (!isLastCharSkip)
			strcpy(code, "[* im]");
		else
			strcat(salt2chat_pcodes," [+ im]");
		return(TRUE);
	}
	return(FALSE);
}

static char isSophieLangCode(char *line, char *code) {
	if (coding != 3)
		return(FALSE);
	while (isSpace(*line))
		line++;
	if (uS.mStricmp(line, "v") == 0) {
		strcpy(code, "[v]");
		return(TRUE);
	} else if (line[0] == 'v' && isdigit(line[1]) && line[2] == EOS) {
		strcpy(code, "[");
		strcat(code, line);
		strcat(code, "]");
		return(TRUE);
	} else if (uS.mStricmp(line, "s") == 0 || uS.mStricmp(line, "sign") == 0) {
		strcpy(code, "[s]");
		return(TRUE);
	} else if (uS.mStricmp(line, "sv") == 0) {
		strcpy(code, "[sv]");
		return(TRUE);
	}
	return(FALSE);
}

static int checkmaze(int lpos) {
	int i;
	
	for (i=lpos; i < pos; i++) {
		if (isalpha(utterance->line[i])) return(TRUE);
	}
	strcpy(utterance->line+lpos, utterance->line+lpos+2);
	pos -= 2;
	return(FALSE);
}

static char *getcom(char *s, char end, char isComment) {
	int  lpos;
	char PCode, lastC, *code, t, beg;

	beg = *s;
	s++;
	while (*s != end) {
		if (*s == EOS) {
			fprintf(stderr,"*** File \"%s\": line %ld\n", oldfname, lnum);
			fprintf(stderr, "ERROR: No matching %c found\n", end);
			salt2chat_clean_up();
			clearcodes();
			cutt_exit(0);
		} else if (*s == '\n') {
			while (isSpace(*s) && *s == '\n')
				s++;
			utterance->line[pos++] = ' ';
		} else if (!isComment && *s == '(' && curspeaker != '+' && curspeaker != '=') {
			lpos = pos;
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '<';
			s = getcom(s, ')', FALSE);
			if (*s == ')')
				s++;
			if (checkmaze(lpos)) {
				utterance->line[pos++] = '>';
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				utterance->line[pos++] = '/';
				utterance->line[pos++] = '?';
				utterance->line[pos++] = ']';
				utterance->line[pos++] = ' ';
			}
		} else if (*s == '<' && curspeaker != '+' && curspeaker != '=') {
			if (hel) {
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				utterance->line[pos++] = '%';
				utterance->line[pos++] = ' ';
				s = getcom(s, '>', FALSE);
				if (*s == '>')
					s++;
				utterance->line[pos++] = ']';
			} else {
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '<';
				s = getcom(s, '>', FALSE);
				if (*s == '>')
					s++;
				utterance->line[pos++] = '>';
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				if (toupper((unsigned char)oldsp) == toupper((unsigned char)curspeaker) &&
					(toupper((unsigned char)overlap) == toupper((unsigned char)curspeaker) || overlap == 0)) {
					utterance->line[pos++]= (char)((overlap == 0)? '<' : '>');
				} else {
					if (toupper((unsigned char)overlap) == toupper((unsigned char)curspeaker))
						brerror = TRUE;
					if (overlap == -1) {
						isOvertalk(comment, TRUE);
						remJunkFrontAndBack(comment);
						removeExtraSpace(comment);
						if (comment[0] != EOS) {
							cleanComment(comment);
							printout("@Comment:", comment, NULL, NULL, TRUE);
						}
						comment[0] = EOS;
						strcpy(templineC1, "<0> [>] .");
						printout("*CHI:", templineC1, NULL, NULL, TRUE);
						strcpy(templineC1, "0");
						printout("%sin:", templineC1, NULL, NULL, TRUE);
					}
					utterance->line[pos++] = (char)((overlap != 0)? '<' : '>');
					if (!isSameLine) {
						if (overlap != 0)
							overlap = 0;
						else
							overlap = curspeaker;
					}
				}
				utterance->line[pos++] = ']';
				utterance->line[pos] = EOS;
				if (overlap != 0)
					strcpy(spareTier1, utterance->line);
				utterance->line[pos++] = ' ';
				isSameLine = TRUE;
			}
		} else if (*s == '[' && *(s+1) == '^') {
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '[';
			utterance->line[pos++] = '^';
			s++;
			s = getcom(s, ']', TRUE);
			if (*s == ']')
				s++;
			utterance->line[pos++] = ']';
		} else if (*s == '[') {
			if (pos < 1)
				lastC = ' ';
			else
				lastC = utterance->line[pos-1];
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '[';
			lpos = pos;
//			utterance->line[pos++] = '%';
			utterance->line[pos++] = '^';
			utterance->line[pos++] = ' ';
			s = getcom(s, ']', FALSE);
			if (*s == ']')
				s++;
			utterance->line[pos] = EOS;
			if (isSophieAmpersandCode(utterance->line+lpos+2, templineC1)) {
				pos = lpos - 1;
				strcpy(utterance->line+pos, "&=");
				pos += 2;
				strcpy(utterance->line+pos, templineC1);
				pos += strlen(templineC1);
				utterance->line[pos++] = ' ';
			} else if (isSophieErrorOrPostcode(utterance->line, lpos, templineC1)) {
				pos = lpos - 1;
				if (templineC1[0] != EOS) {
					strcpy(utterance->line+pos, templineC1);
					pos += strlen(templineC1);
					utterance->line[pos++] = ' ';
				}
			} else if (isSophieLangCode(utterance->line+lpos+2, templineC1)) {
				pos = lpos - 2;
				strcpy(utterance->line+pos, templineC1);
				pos += strlen(templineC1);
			} else if (!uS.mStricmp(utterance->line+lpos, "% g") && coding == 2) {
				utterance->line[lpos-2] = EOS;
				pos = strlen(utterance->line);
			} else if (!uS.mStricmp(utterance->line+lpos, "% i") && coding == 2) {
				utterance->line[lpos-2] = EOS;
				pos = strlen(utterance->line);
			} else if (!uS.mStricmp(utterance->line+lpos, "% eu") && coding == 2) {
				if (lpos >= 3 && isSpace(utterance->line[lpos-3]))
					utterance->line[lpos-3] = EOS;
				else
					utterance->line[lpos-2] = EOS;
				uS.shiftright(utterance->line, 1);
				utterance->line[0] = '<';
				strcat(utterance->line, "> [*] ");
				pos = strlen(utterance->line);
			} else if (!uS.mStricmp(utterance->line+lpos, "% ew") && coding == 2) {
				if (lpos >= 3 && isSpace(utterance->line[lpos-3]))
					lpos = lpos - 3;
				else
					lpos = lpos - 2;
				utterance->line[lpos] = EOS;
				for (lpos--; lpos >= 0 && isSpace(utterance->line[lpos]); lpos--) ;
				lpos++;
				uS.shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '>';
				for (lpos=lpos-1; lpos >= 0 && (isalpha(utterance->line[lpos]) || utterance->line[lpos] == '\''); lpos--) ;
				lpos++;
				uS.shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '<';
				pos = strlen(utterance->line);
				strcat(utterance->line, " [*]");
				pos = strlen(utterance->line);
			} else if (!uS.mStrnicmp(utterance->line+lpos, "% ew:", 5) && coding == 2) {
				utterance->line[pos] = EOS;
				utterance->line[lpos] = ':';
				pos = lpos + 2;
				strcpy(utterance->line+pos, utterance->line+pos+3);			
				for (lpos=lpos-2; lpos >= 0 && isSpace(utterance->line[lpos]); lpos--) ;
				lpos++;
				uS.shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '>';
				for (lpos=lpos-1; lpos >= 0 && (isalpha(utterance->line[lpos]) || utterance->line[lpos] == '\''); lpos--) ;
				lpos++;
				uS.shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '<';
				pos = strlen(utterance->line);
				strcat(utterance->line, "] [*]");
				pos = strlen(utterance->line);
			} else {
				if (utterance->line[lpos+1] == '+' || isSophiePostcode(utterance->line+lpos+2, pos-lpos-2)) {
					strcpy(utterance->line+lpos, utterance->line+lpos+2);
					pos -= 2;
					PCode = TRUE;
				} else
					PCode = FALSE;
				if (curspeaker == '+')
					checkcodes(lpos);
				else
					addtocodes(lpos);
				utterance->line[pos++] = ']';
				if (!isSpace(*s) && *s != ')' && *s != ']' && *s != '}' && *s != '>')
					utterance->line[pos++] = ' ';
				if (!uselcode && coding == 1 && !isalnum((unsigned char)lastC)) {
					utterance->line[pos-1] = EOS;
					strcat(lcode, " ");
					strcat(lcode, utterance->line+lpos-1);
					strcat(lcode, "]");
					pos = lpos - 2;
				} else if (PCode) {
					utterance->line[pos] = EOS;
					strcat(salt2chat_pcodes," [+ ");
					strcat(salt2chat_pcodes,utterance->line+lpos);
					pos = lpos - 2;
					utterance->line[pos] = EOS;
				} else if (uselcode && isalnum((unsigned char)curspeaker) && (coding != 1 || !isalnum((unsigned char)lastC))) {
					t = utterance->line[pos-1];
					utterance->line[pos-1] = EOS;
					code = FindRightLine(utterance->line+lpos+1);
					if (code != NULL) {
						if (*code)
							strcat(code, ", ");
						strcat(code,utterance->line+lpos);
						pos = lpos - 2;
						utterance->line[pos] = EOS;
					} else
						utterance->line[pos-1] = t;
				}
			}
		} else {
			s = parseline(s, isComment);
		}
	}
	utterance->line[pos] = EOS;
	remJunkBack(utterance->line);
	pos = strlen(utterance->line);
	if (pos > 0 && utterance->line[pos-1] == beg) {
		strcat(utterance->line, "xxx");
		pos = strlen(utterance->line);
	}
	return(s);
}

static void salt2chat_remblanks(char *st) {
	int i;
	
	for (i=strlen(st)-1; i >= 0 && (st[i] == ' ' || st[i] == '\t'); i--) ;
	st[i+1] = EOS;
}

static void parseSaltTier(char *s) {
	int lpos;
	char PCode, lastC, *code, t;

	for (s++; isSpace(*s); s++)
		;
	if (coding == 7) {
		if (s[0] == ':' && isSpace(s[1])) {
			for (s++; isSpace(*s); s++)
				;
		}
	}
//	if (nomap) {
//		if (isupper((unsigned char)(int)*s))
//			*s = (char)tolower((unsigned char)*s);
//	}
 	while (1) {
		if (*s == EOS) {
			break;
		} else if (*s == '\n') {
			while (isSpace(*s) || *s == '\n')
				s++;
			utterance->line[pos++] = ' ';
		} else if (coding == 7) {
			s = parseline(s, FALSE);
		} else if (*s == '{') {
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '{';
			utterance->line[pos++] = '%';
			utterance->line[pos++] = ' ';
			s = getcom(s, '}', TRUE);
			if (*s == '}')
				s++;
			utterance->line[pos++] = '}';
		} else if (*s == '[' && *(s+1) == '^') {
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '[';
			utterance->line[pos++] = '^';
			s++;
			s = getcom(s, ']', TRUE);
			if (*s == ']')
				s++;
			utterance->line[pos++] = ']';
		} else if (*s == '[') {
			if (pos < 1)
				lastC = ' ';
			else
				lastC = utterance->line[pos-1];
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '[';
			lpos = pos;
//			utterance->line[pos++] = '%';
			utterance->line[pos++] = '^';
			utterance->line[pos++] = ' ';
			s = getcom(s, ']', TRUE);
			if (*s == ']')
				s++;
			utterance->line[pos] = EOS;
			if (isSophieAmpersandCode(utterance->line+lpos+2, templineC1)) {
				pos = lpos - 1;
				strcpy(utterance->line+pos, "&=");
				pos += 2;
				strcpy(utterance->line+pos, templineC1);
				pos += strlen(templineC1);
				utterance->line[pos++] = ' ';
			} else if (isSophieErrorOrPostcode(utterance->line, lpos, templineC1)) {
				pos = lpos - 1;
				if (templineC1[0] != EOS) {
					strcpy(utterance->line+pos, templineC1);
					pos += strlen(templineC1);
					utterance->line[pos++] = ' ';
				}
			} else if (isSophieLangCode(utterance->line+lpos+2, templineC1)) {
				pos = lpos - 2;
				strcpy(utterance->line+pos, templineC1);
				pos += strlen(templineC1);
			} else if (!uS.mStricmp(utterance->line+lpos, "% g") && coding == 2) {
				utterance->line[lpos-2] = EOS;
				pos = strlen(utterance->line);
			} else if (!uS.mStricmp(utterance->line+lpos, "% i") && coding == 2) {
				utterance->line[lpos-2] = EOS;
				pos = strlen(utterance->line);
			} else if (!uS.mStricmp(utterance->line+lpos, "% eu") && coding == 2) {
				if (lpos >= 3 && isSpace(utterance->line[lpos-3]))
					utterance->line[lpos-3] = EOS;
				else
					utterance->line[lpos-2] = EOS;
				uS.shiftright(utterance->line, 1);
				utterance->line[0] = '<';
				strcat(utterance->line, "> [*] ");
				pos = strlen(utterance->line);
			} else if (!uS.mStricmp(utterance->line+lpos, "% ew") && coding == 2) {
				if (lpos >= 3 && isSpace(utterance->line[lpos-3]))
					lpos = lpos - 3;
				else
					lpos = lpos - 2;
				utterance->line[lpos] = EOS;
				for (lpos--; lpos >= 0 && isSpace(utterance->line[lpos]); lpos--) ;
				lpos++;
				uS.shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '>';
				for (lpos=lpos-1; lpos >= 0 && (isalpha(utterance->line[lpos]) || utterance->line[lpos] == '\''); lpos--) ;
				lpos++;
				uS.shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '<';
				pos = strlen(utterance->line);
				strcat(utterance->line, " [*]");
				pos = strlen(utterance->line);
			} else if (!uS.mStrnicmp(utterance->line+lpos, "% ew:", 5) && coding == 2) {
				utterance->line[pos] = EOS;
				utterance->line[lpos] = ':';
				pos = lpos + 2;
				strcpy(utterance->line+pos, utterance->line+pos+3);			
				for (lpos=lpos-2; lpos >= 0 && isSpace(utterance->line[lpos]); lpos--) ;
				lpos++;
				uS.shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '>';
				for (lpos=lpos-1; lpos >= 0 && (isalpha(utterance->line[lpos]) || utterance->line[lpos] == '\''); lpos--) ;
				lpos++;
				uS.shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '<';
				pos = strlen(utterance->line);
				strcat(utterance->line, "] [*]");
				pos = strlen(utterance->line);
			} else {
				if (utterance->line[lpos+1] == '+' || isSophiePostcode(utterance->line+lpos+2, pos-lpos-2)) {
					strcpy(utterance->line+lpos, utterance->line+lpos+2);
					pos -= 2;
					PCode = TRUE;
				} else
					PCode = FALSE;
				if (curspeaker == '+')
					checkcodes(lpos);
				else
					addtocodes(lpos);
				utterance->line[pos++] = ']';
				if (!isSpace(*s) && *s != ')' && *s != ']' && *s != '}' && *s != '>')
					utterance->line[pos++] = ' ';
				if (curspeaker == '+') ;
				else if (!uselcode && coding == 1 && !isalnum((unsigned char)lastC)) {
					utterance->line[pos-1] = EOS;
					strcat(lcode, " ");
					strcat(lcode,utterance->line+lpos-1);
					strcat(lcode,"]");
					pos = lpos - 2;
				} else if (PCode) {
					utterance->line[pos] = EOS;
					strcat(salt2chat_pcodes," [+ ");
					strcat(salt2chat_pcodes,utterance->line+lpos);
					pos = lpos - 2;
					utterance->line[pos] = EOS;
				} else if (uselcode && isalnum((unsigned char)curspeaker) && 
											(coding != 1 || !isalnum((unsigned char)lastC))) {
					t = utterance->line[pos-1];
					utterance->line[pos-1] = EOS;
					code = FindRightLine(utterance->line+lpos+1);
					if (code != NULL) {
						if (*code)
							strcat(code, ", ");
						strcat(code,utterance->line+lpos);
						pos = lpos - 2;
						utterance->line[pos] = EOS;
					} else
						utterance->line[pos-1] = t;
				}
			}
		} else if (*s == '(' && curspeaker != '+' && curspeaker != '=') {
			lpos = pos;
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '<';
			s = getcom(s, ')', FALSE);
			if (*s == ')')
				s++;
			if (checkmaze(lpos)) {
				utterance->line[pos++] = '>';
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				utterance->line[pos++] = '/';
				utterance->line[pos++] = '?';
				utterance->line[pos++] = ']';
				utterance->line[pos++] = ' ';
			}
		} else if (*s == '<' && curspeaker != '+' && curspeaker != '=') {
			if (hel) {
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				utterance->line[pos++] = '%';
				utterance->line[pos++] = ' ';
				s = getcom(s, '>', FALSE);
				if (*s == '>')
					s++;
				utterance->line[pos++] = ']';
			} else {
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '<';
				s = getcom(s, '>', FALSE);
				if (*s == '>')
					s++;
				utterance->line[pos++] = '>';
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				if (toupper((unsigned char)oldsp) == toupper((unsigned char)curspeaker) &&
					(toupper((unsigned char)overlap) == toupper((unsigned char)curspeaker) || overlap == 0)) {
					utterance->line[pos++]= (char)((overlap == 0)? '<' : '>');
				} else {
					if (toupper((unsigned char)overlap) == toupper((unsigned char)curspeaker))
						brerror = TRUE;
					if (overlap == -1) {
						isOvertalk(comment, TRUE);
						remJunkFrontAndBack(comment);
						removeExtraSpace(comment);
						if (comment[0] != EOS) {
							cleanComment(comment);
							printout("@Comment:", comment, NULL, NULL, TRUE);
						}
						comment[0] = EOS;
						strcpy(templineC1, "<0> [>] .");
						printout("*CHI:", templineC1, NULL, NULL, TRUE);
						strcpy(templineC1, "0");
						printout("%sin:", templineC1, NULL, NULL, TRUE);
					}
					if (!isSameLine || overlap == 0) {
						utterance->line[pos++] = (char)((overlap != 0)? '<' : '>');
						if (overlap != 0)
							overlap = 0;
						else
							overlap = curspeaker;
					} else
						utterance->line[pos++] = '>';
				}
				utterance->line[pos++] = ']';
				utterance->line[pos] = EOS;
				if (overlap != 0)
					strcpy(spareTier1, utterance->line);
				utterance->line[pos++] = ' ';
				isSameLine = TRUE;
			}
		} else
			s = parseline(s, FALSE);
	}
	utterance->line[pos] = EOS;
	salt2chat_remblanks(utterance->line);
}

static void gsp(char *s, char *part) {
	int i, p, sp;

	sp = -1;
	*part = EOS;
	if (coding == 7) {
		sp++;
		name[sp][0] = '@';
		strcpy(name[sp]+1,"@");
	}
	while (!isalpha(*s) && *s != '\n' && *s != EOS)
		s++;
	while (*s != EOS) {
		if (*part != EOS)
			strcat(part,", ");
		if (sp == 9) {
			salt2chat_clean_up();
			clearcodes();
			fprintf(stderr,"salt2chat: exceeded speakers limit of 10.\n");
			cutt_exit(0);
		}
		sp++;
		name[sp][0] = Lisupper(*s, TRUE);
		strcpy(name[sp]+1,"*   :");
		for (i=2; isalpha(*s) && i < 5; i++) {
			name[sp][i] = Lisupper(*s, TRUE);
			s++;
		}
		if (uS.mStrnicmp(name[sp]+2,"exa",3)==0 || uS.mStrnicmp(name[sp]+2,"res",3)==0 || uS.mStrnicmp(name[sp]+2,"cli",3)==0) {
			while (isalpha(*s))
				s++;
			strcat(part,"INV Investigator");
			strcpy(name[sp]+2, "INV:");
		} else if (uS.mStrnicmp(name[sp]+2, "sub", 3) == 0 || uS.mStrnicmp(name[sp]+2, "par", 3) == 0) {
			while (isalpha(*s))
				s++;
			strcat(part,"PAR Participant");
			strcpy(name[sp]+2, "PAR:");
		} else if (uS.mStrnicmp(name[sp]+2, "chi", 3) == 0) {
			while (isalpha(*s))
				s++;
			strcat(part,"CHI Target_Child");
			strcpy(name[sp]+2, "CHI:");
		} else if (uS.mStrnicmp(name[sp]+2,"mot",3)==0 || uS.mStrnicmp(name[sp]+2,"mom",3)==0) {
			while (isalpha(*s))
				s++;
			strcat(part,"MOT Mother");
			strcpy(name[sp]+2, "MOT:");
		} else if (uS.mStrnicmp(name[sp]+2,"fat",3)==0) {
			while (isalpha(*s))
				s++;
			strcat(part,"FAT Father");
			strcpy(name[sp]+2, "FAT:");
		} else {
			p = strlen(part);
			strcat(part,name[sp]+2);
			for (i=strlen(part); part[i] != ':' && i > 0; i--) ;
			for (i--; part[i] == ' ' && i >= 0; i--) ;
			i++;
			part[i++] = ' ';
			part[i++] = EOS;
			uS.uppercasestr(part+p, &dFnt, MBF);
			p = strlen(part);
			strcat(part, name[sp]+2);
			if (isalpha(part[p]))
				part[p] = toupper(part[p]);
			for (p=strlen(part); part[p] != ':' && p > i; p--) ;
			for (p--; part[p] == ' ' && p >= i; p--) ;
			p++;
			while (isalpha(*s)) {
				part[p++] = *s;
				s++;
			}
			part[p] = EOS;
			for (; part[i] != EOS; i++) {
				part[i]= Lisupper(part[i], TRUE);
			}
			if (strcmp(name[sp]+1, "*EXA:") == 0) {
				strcat(part,"INV Investigator");
				strcpy(name[sp]+2, "INV:");
			} else if (strcmp(name[sp]+1, "*RES:") == 0) {
				strcat(part,"INV Investigator");
				strcpy(name[sp]+2, "INV:");
			} else if (strcmp(name[sp]+1, "*SUB:") == 0) {
				strcat(part,"PAR Participant");
				strcpy(name[sp]+2, "PAR:");
			}
		}
		while (!isalpha(*s) && *s != '\n' && *s != EOS)
			s++;
	}
	if (*part != EOS) {
		if (coding != 0 && coding != 4 && coding != 5 && coding != 6 && coding != 7 && coding != 8) {
			printout("@Participants:\t", part, NULL, NULL, TRUE);
			printIDs(part, NULL, NULL, NULL);
		}
	}
}
static int strCnt(char *cur) {
	int cnt = 0;

	while (isalpha(*cur)) {
		cnt++;
		cur++;
	}
	return(cnt);
}

static void cleanVBars(char *line) {
	int b, e;

	b = 0;
	while (line[b] != EOS) {
		if (line[b] == '|' && b > 0 && isalnum(line[b-1])) {
			for (e=b+1; isalnum(line[e]); e++) ;
			if (e > b)
				strcpy(line+b, line+e);
			else
				b++;
		} else
			b++;
	}
}

static void cleanCodeSuffixes(char *line) {
	int i, cb, sf, sfb, len;
	char st[BUFSIZ+1];

	cb = -1;
	sf = 0;
	for (sf=0; line[sf] != EOS; sf++) {
		if (line[sf] == '[')
			cb = sf;
		else if (cb >= 0 && line[sf] == ']' && line[sf+1] == '/') {
			sf++;
			sfb = sf;
			for (i=0; i < BUFSIZ && (isalnum(line[sf]) || line[sf] == '/' || line[sf] == '\''); sf++)
				st[i++] = line[sf];
			st[i] = EOS;
			if (cb < sfb) {
				if (sfb < sf)
					strcpy(line+sfb, line+sf);
				len = strlen(st);
				uS.shiftright(line+cb, len);
				for (i=0; i < len; i++)
					line[cb++] = st[i];
				cb = -1;
				sf--;
			}
		} else if (uS.isskip(line,sf,&dFnt,MBF))
			cb = -1;
	}
}

static void cleanUpQuotes(char *line) {
	int i, j;
	char isConvDblQT;

	for (i=0; line[i] != EOS; i++) {
		if (line[i] == (char)0xD2 || line[i] == (char)0x93) {
			uS.shiftright(line+i,2);
			line[i] = (char)0xE2;
			line[i+1] = (char)0x80;
			line[i+2] = (char)0x9C;
		} else if (line[i] == (char)0xD3 || line[i] == (char)0x94) {
			uS.shiftright(line+i,2);
			line[i] = (char)0xE2;
			line[i+1] = (char)0x80;
			line[i+2] = (char)0x9D;
		}

		if (i > 0 && line[i-1] == '/' && line[i] == (char)0x92) {
			line[i] = '\'';
		} else if (line[i] == (char)0xEF && line[i+1] == (char)0x81 && line[i+2] == (char)0xBC) {
			for (j=i+3; isSpace(line[j]); j++) ;
			if (j > i + 1)
				strcpy(line+i+1, line+j);
			line[i] = '|';
		} else if (line[i] == (char)0xE2 && line[i+1] == (char)0x80 && line[i+2] == (char)0x98) {
			isConvDblQT = FALSE;
			for (j=i+1; line[j] != EOS; j++) {
				if (line[j] == (char)0xE2 && line[j+1] == (char)0x80 && line[j+2] == (char)0x98) {
					break;
				} else if (line[j] == (char)0xE2 && line[j+1] == (char)0x80 && line[j+2] == (char)0x99) {
					if (!isalnum(line[j+3]))
						isConvDblQT = TRUE;
					break;
				}
			}
			if (isConvDblQT == FALSE) {
				strcpy(line+i, line+i+2);
				line[i] = '\'';
			} else {
				line[i+2] = (char)0x9C;
				line[j+2] = (char)0x9D;
			}
		} else if (line[i] == (char)0xE2 && line[i+1] == (char)0x80 && line[i+2] == (char)0x9C) {
			for (j=i+1; line[j] != EOS; j++) {
				if (line[j] == (char)0xE2 && line[j+1] == (char)0x80 && line[j+2] == (char)0x99) {
					strcpy(line+j, line+j+2);
					line[j] = '\'';
				} else if (line[j] == (char)0xE2 && line[j+1] == (char)0x80 && line[j+2] == (char)0x9B) {
					strcpy(line+j, line+j+2);
					line[j] = '\'';
				} else if (line[j] == '"') {
					uS.shiftright(line+j,2);
					line[j] = (char)0xE2;
					line[j+1] = (char)0x80;
					line[j+2] = (char)0x9D;
				} else if (line[j] == (char)0xE2 && line[j+1] == (char)0x80 && line[j+2] == (char)0x9D) {
					i = j + 2;
					break;
				} else if (line[j] == (char)0xE2 && line[j+1] == (char)0x80 && line[j+2] == (char)0x9C) {
					line[j+2] = (char)0x9D;
				}
			}
		} else if (line[i] == (char)0xE2 && line[i+1] == (char)0x80 && line[i+2] == (char)0x99) {
			strcpy(line+i, line+i+2);
			line[i] = '\'';
		} else if (line[i] == (char)0xE2 && line[i+1] == (char)0x80 && line[i+2] == (char)0x9B) {
			strcpy(line+i, line+i+2);
			line[i] = '\'';
		} else if (line[i] == (char)0xE2 && line[i+1] == (char)0x80 && line[i+2] == (char)0x93) {
			strcpy(line+i, line+i+2);
			line[i] = '-';
		} else if (line[i] == (char)0xE2 && line[i+1] == (char)0x80 && line[i+2] == (char)0x92) {
			strcpy(line+i, line+i+2);
			line[i] = '-';
		} else if (UTF8_IS_SINGLE((unsigned char)line[i])) {
			if (line[i] == (char)0xD0 || line[i] == (char)0x96 || line[i] == (char)0xF1) {
				line[i] = '-';
			} else if (line[i] == (char)0xC9 || line[i] == (char)0x85) {
				uS.shiftright(line+i,2);
				line[i] = (char)0xE2;
				line[i+1] = (char)0x80;
				line[i+2] = (char)0xA6;
				i = i + 2;
			} else if (line[i] == '`' || line[i] == (char)0xB4 || line[i] == (char)0xD5 || line[i] == (char)0x92) {
				line[i] = '\'';
			}
		}
	}
	j = -1;
	isConvDblQT = FALSE;
	for (i=0; line[i] != EOS; i++) {
		if (line[i] == '"') {
			isConvDblQT = !isConvDblQT;
			if (isConvDblQT) {
				uS.shiftright(line+i,2);
				line[i] = (char)0xE2;
				line[i+1] = (char)0x80;
				line[i+2] = (char)0x9C;
				j = i;
			} else {
				uS.shiftright(line+i,2);
				line[i] = (char)0xE2;
				line[i+1] = (char)0x80;
				line[i+2] = (char)0x9D;
				j = i;
			}
		} else if (line[i] == (char)0xE2 && line[i+1] == (char)0x80 && line[i+2] == (char)0x9C) {
			isConvDblQT = TRUE;
		} else if (line[i] == (char)0xE2 && line[i+1] == (char)0x80 && line[i+2] == (char)0x9D) {
			isConvDblQT = FALSE;
		}
	}
	if (isConvDblQT) {
		for (i=j+1; line[i] != EOS; i++) {
			if (uS.isskip(line,i,&dFnt,MBF))
				break;
		}
		if (i > j+1) {
			uS.shiftright(line+i,3);
			line[i] = (char)0xE2;
			line[i+1] = (char)0x80;
			line[i+2] = (char)0x9D;
		}
	}
}

static char isEnd(char *cur) {
	for (; *cur != EOS; cur++) {
		if (isalnum(*cur))
			return(FALSE);
	}
	return(TRUE);
}

static char addCurToOut(char *out, char *cur, char isAddSpaceChr) {
	int i, ab, sb;

	lnum++;
	if (isAddSpaceChr && out[0] != EOS)
		strcat(out, " ");
	if (cur[0] == '+' || cur[0] == '-' || cur[0] == '=' || cur[0] == ';' || cur[0] == ':') {
		strcat(out, cur);
		uS.remblanks(out);
		cur[0] = EOS;
		return(FALSE);
	}
	ab = 0;
	sb = 0;
	for (i=0; cur[i] != EOS; i++) {
		if (cur[i] == '[')
			sb++;
		else if (cur[i] == ']') {
			sb--;
			if (sb < 0)
				sb = 0;
		}
		if (cur[i] == '<')
			ab++;
		else if (cur[i] == '<') {
			ab--;
			if (ab < 0)
				ab = 0;
		}
		if (sb > 0) {
		} else if (isSingleDelim(cur+i)) {
			for (i++; isSpace(cur[i]); i++) ;
			if (isupper(cur[i]) && findsp(cur[i]) && isSpace(cur[i+1])) {
				strncat(out, cur, i);
				uS.remblanks(out);
				strcpy(cur, cur+i);
				return(TRUE);
			}
			if (cur[i] == EOS)
				break;
		} else if (cur[i] == '>' && ab == 0) {
			uS.remblanks(cur);
			for (i++; isSpace(cur[i]); i++) ;
			if (!isEnd(cur+i)) {
				strncat(out, cur, i);
				uS.remblanks(out);
				strcpy(cur, cur+i);
				uS.shiftright(cur, 2);
				cur[0] = toupper((unsigned char)curspeaker);
				cur[1] = ' ';
				return(TRUE);
			}
		} else if (cur[i] == '^') {
			uS.remblanks(cur);
			for (i++; isSpace(cur[i]); i++) ;
			if (!isEnd(cur+i)) {
				strncat(out, cur, i);
				uS.remblanks(out);
				strcpy(cur, cur+i);
				uS.shiftright(cur, 2);
				cur[0] = toupper((unsigned char)curspeaker);
				cur[1] = ' ';
				return(TRUE);
			}
		}

	}
	strcat(out, cur);
	uS.remblanks(out);
	cur[0] = EOS;
	return(FALSE);
}

static void getSaltTier(char *out, char *cur, char isSaltHeaderFound) {
	int i, outLen;

	out[0] = EOS;
	if (cur[0] != EOS) {
		if (addCurToOut(out, cur, FALSE)) {
			uS.remFrontAndBackBlanks(out);
			return;
		}
	}
	while (fgets_cr(cur, UTTLINELEN, fpin)) {
		if (isSaltHeaderFound) {
			if (findsp(cur[0]) != NULL || cur[0] == '+' || cur[0] == '-' || cur[0] == '=' || cur[0] == ';' || cur[0] == ':')
				break;
			if (isupper(cur[0]) && isSpace(cur[1]) && cur[0] != 'I' && cur[0] != 'X')
				break;
			if (!isSpace(cur[0]) && cur[0] != '\n' && cur[0] != EOS) {
				outLen = strlen(out);
				if (uS.IsUtteranceDel(cur, 0) || cur[0] == ',' || cur[0] == ';' || cur[0] == '/' || cur[0] == '\'') {
					if (addCurToOut(out, cur, FALSE))
						break;
				} else if (cur[0] == '%') {
					if (strCnt(cur+1) <= 2 && outLen >= 3 &&
						isSpace(out[outLen-3]) && isalpha(out[outLen-2]) && isalpha(out[outLen-1])) {
						if (addCurToOut(out, cur, FALSE))
							break;
					}
				} else if (cur[0] == '[' || cur[0] == ']' || cur[0] == '(' || cur[0] == ')') {
				} else if (UTF8_IS_LEAD((unsigned char)cur[0]) && cur[0] == (char)0xE2) {
					if (addCurToOut(out, cur, FALSE))
						break;
				} else if ((isdigit(cur[0]) && cur[1] == ':') ||
						   (isdigit(cur[0]) && isdigit(cur[1]) && cur[2] == ':')) {
					uS.shiftright(cur, 2);
					cur[0] = '-';
					cur[1] = ' ';
					break;
				} else {
					if (!isalpha(cur[0]))
						break;

					if (strCnt(cur) < 2) {
						if (cur[0] != 'I' && cur[0] != 'X') {
							if (addCurToOut(out, cur, FALSE))
								break;
						}
					} else if (strCnt(cur) < 3) {
						if (uS.mStrnicmp(cur, "if", 2) != 0 && uS.mStrnicmp(cur, "it", 2) != 0 && uS.mStrnicmp(cur, "in", 2) != 0) {
							if (addCurToOut(out, cur, FALSE))
								break;
						}
					} else if (strCnt(cur) == 3) {
						if (outLen >= 3 && isSpace(out[outLen-3]) && isalpha(out[outLen-2]) && isalpha(out[outLen-1]) && uS.mStrnicmp(cur, "tty", 3) == 0) {
							if (addCurToOut(out, cur, FALSE))
								break;
						}
					}
				}
			} else if (cur[0] != EOS) {
				for (i=0; isSpace(cur[i]); i++) ;
				if (cur[i] == '+' || cur[i] == '-' || cur[i] == ';' || cur[i] == ':') {
					if ((isSpace(cur[i+1]) || cur[i+1] == '\n' || cur[i+1] == EOS)) {
						strcpy(cur, cur+i);
						break;
					}
				} else if (cur[i] == '=') {
					if ((isSpace(cur[i+1]) || cur[i+1] == '\n' || cur[i+1] == EOS) || isalnum(cur[i+1])) {
						strcpy(cur, cur+i);
						break;
					}
				} else if (isupper(cur[i]) && findsp(cur[i])) {
					if ((isSpace(cur[i+1]) || cur[i+1] == '\n' || cur[i+1] == EOS)) {
						strcpy(cur, cur+i);
						break;
					}
				} else if ((isdigit(cur[i]) && cur[i+1] == ':') ||
						   (isdigit(cur[i]) && isdigit(cur[i+1]) && cur[i+2] == ':')) {
					strcpy(cur, cur+i);
					uS.shiftright(cur, 2);
					cur[0] = '-';
					cur[1] = ' ';
					break;
				}
			}
		} else if (!isSpace(cur[0]) && cur[0] != '\n' && cur[0] != EOS)
			break;
		if (cur[0] != EOS) {
			if (cur[0] != '\n')
				lnum++;
			addCurToOut(out, cur, TRUE);
		}
	}
	uS.remFrontAndBackBlanks(out);
	if (feof(fpin) && cur[0] == EOF)
		cur[0] = EOS;
}

void call() {
	int  i;
	char fSpTier, isSaltHeaderFound;
	char part[512];

	lnum = 0;
	part[0] = EOS;
	curspeaker = '\0';
	SpFound = FALSE;
	spareTier3[0] = EOS;
	isSaltHeaderFound = FALSE;
	if (coding == 7) {
		fgets_cr(spareTier3, UTTLINELEN, fpin);
		if (spareTier3[0] != '$') {
			strcpy(uttline, "$ Clinician, Participant");
			isSaltHeaderFound = TRUE;
			gsp(uttline, part);
			if ((spareTier3[0] != 'C' && spareTier3[0] != 'P') || (spareTier3[1] != ':' && spareTier3[1] != ' ')) {
				if (spareTier3[0] == '@' && (spareTier3[1] == 'g' || spareTier3[1] == 'G') && (spareTier3[2] == ':' || spareTier3[2] == ' ')) {
				} else {
					uS.shiftright(spareTier3, 2);
					spareTier3[0] = '+';
					spareTier3[1] = ' ';
				}
			}
		} else {
			getSaltTier(uttline, spareTier3, isSaltHeaderFound);
			isSaltHeaderFound = TRUE;
			gsp(uttline, part);
		}
	} else {
		while (fgets_cr(spareTier3, UTTLINELEN, fpin)) {
			for (i=0; isSpace(spareTier3[i]) || spareTier3[i] == '\n'; i++) ;
			if (i > 0)
				strcpy(spareTier3, spareTier3+i);
			if (spareTier3[0] == '$')
				break;
			if (!uS.isUTF8(spareTier3) && !uS.isInvisibleHeader(spareTier3)) {
				lnum++;
			}
		}
		if (spareTier3[0] != '$') {
			clearcodes();
			fprintf(stderr,"*** File \"%s\"\n", oldfname);
			fprintf(stderr, "ERROR: Can't locate speaker identifying line $.\n");
		}
		getSaltTier(uttline, spareTier3, isSaltHeaderFound);
		isSaltHeaderFound = TRUE;
		gsp(uttline, part);
	}
	isSameLine = FALSE;
	fSpTier = TRUE;
	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	eng\n");
	while (!feof(fpin) || spareTier3[0] != EOS) {
		getSaltTier(uttline, spareTier3, isSaltHeaderFound);
		if (uS.isUTF8(uttline) || uS.isInvisibleHeader(uttline)) {
		} else if (uttline[0] != EOS) {
			if (isalpha(uttline[0])) {
				curspeaker = Lisupper(uttline[0], TRUE);
				fSpTier = FALSE;
			} else
				curspeaker = uttline[0];
			if (curspeaker != '+')
				clearcodes();
			if (isalpha(curspeaker) && *tim) {
				strcat(utterance->line,tim);
				strcat(utterance->line," ");
				pos = strlen(utterance->line);
				*tim = EOS;
			}
			if (isalpha(uttline[0]) && findsp(uttline[0]) != NULL && uttline[1] == '.' && uttline[2] != EOS) {
				uttline[1] = ' ';
			}
			if (coding == 6)
				cleanVBars(uttline);
			if (isalpha(curspeaker))
				cleanCodeSuffixes(uttline);
			cleanUpQuotes(uttline);
			parseSaltTier(uttline);
			if (curspeaker == '=' && fSpTier)
				prline('+',utterance->line, part);
			else
				prline(curspeaker,utterance->line, part);
			if (isalpha(curspeaker)) {
				isSameLine = FALSE;
				if (toupper((unsigned char)oldsp) != toupper((unsigned char)curspeaker) && overlap == 0)
					oldsp = '\0';
				else
					oldsp = curspeaker;
			}
		}
	}
	uS.remFrontAndBackBlanks(comm);
	removeExtraSpace(comm);
	if (coding != 3 && *comm != EOS) {
		printout("%com:", comm, NULL, NULL, TRUE);
		comm[0] = EOS;
	}
	uS.remFrontAndBackBlanks(def_line);
	removeExtraSpace(def_line);
	if (*def_line != EOS) {
		printout("%def:", def_line, NULL, NULL, TRUE);
		def_line[0] = EOS;
	}
	remJunkFrontAndBack(comment);
	removeExtraSpace(comment);
	if (comment[0] != EOS) {
		cleanComment(comment);
		printout("@Comment:", comment, NULL, NULL, TRUE);
		comment[0] = EOS;
	}

	if (overlap != 0) {
		fprintf(fpout,"@Comment:\t%s\n", "ERROR: Can't find matching closing overlap for line:");
		fprintf(fpout,"\t%s\n", spareTier1);
	}
	fprintf(fpout, "@End\n");
	clearcodes();
}
