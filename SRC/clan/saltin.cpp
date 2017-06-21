/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
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

#define CHAT_MODE 0

#include "cu.h"

#define DispChanges 1

#if !defined(UNX)
#define _main saltin_main
#define call saltin_call
#define getflag saltin_getflag
#define init saltin_init
#define usage saltin_usage
#endif

#define pr_result saltin_pr_result

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
static int  lnum;
static char name[10][10];
static char *tim;
static char *saltin_templine;
static char *lcode;
static char *comm;
static char *comment;
static char *def_line;
static char *saltin_pcodes;
static char coding;
static char curspeaker;
static char brerror;
static char letread;
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

static void saltin_clean_up(void) {
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
	if (saltin_templine)
		free(saltin_templine);
	if (def_line)
		free(def_line);
	if (saltin_pcodes)
		free(saltin_pcodes);
	tim = NULL;
	comm = NULL;
	lcode = NULL;
	comment = NULL;
	saltin_templine = NULL;
	def_line = NULL;
	saltin_pcodes = NULL;
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
		saltin_clean_up();
		clearcodes();
		fprintf(stderr,"saltin: no more memory available.\n");
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
		saltin_templine = NULL;
		def_line = NULL;
		saltin_pcodes = NULL;
		headCode = NULL;
		headTier = NULL;
		if ((tim=(char *)malloc(LINE_LEN)) == NULL) out_of_mem();
		if ((saltin_templine=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); out_of_mem();
		}
		if ((lcode=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(saltin_templine); out_of_mem();
		}
		if ((comm=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(saltin_templine); free(lcode); out_of_mem();
		}
		if ((comment=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(saltin_templine); free(lcode); free(comm); out_of_mem();
		}
		if ((saltin_pcodes=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(saltin_templine); free(lcode); free(comm); free(comment); out_of_mem();
		}
		if ((def_line=(char *)malloc(LINE_LEN)) == NULL) {
			free(tim); free(saltin_templine); free(lcode); free(comm); free(comment); free(saltin_pcodes);
			out_of_mem();
		}
	} else {
		pos = 0;
		lnum = 0;
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
		if (coding == 3)
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
		if (saltin_templine)
			saltin_templine[0] = EOS;
		if (def_line)
			def_line[0] = EOS;
		if (saltin_pcodes)
			saltin_pcodes[0] = EOS;
	}
}

void usage() {
	puts("SALTIN changes a file from SALT to CHAT.");
	printf("Usage: saltin [cS h l %s] filename(s)\n", mainflgs());
	puts("+cS: use special coding system");
	puts("     a- Alison, c- Chris, s- Sophie");
	puts("+h : handle \"<...>\" as [% ...] (default: <...> [\"overlap\"])");
	puts("+lF: put codes on separate tier; use mapping file F to determine tier name.");
	mainusage(TRUE);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = SALTIN;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
	saltin_clean_up();
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
		saltin_clean_up();
		clearcodes();
		fprintf(stderr,"saltin: no more memory available.\n");
		cutt_exit(0);
	}
	nextone->code = (char *)malloc(strlen(st)+1);
	if (nextone->code == NULL) {
		saltin_clean_up();
		clearcodes();
		fprintf(stderr,"saltin: no more memory available.\n");
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
	int index = 0, l = 1;
	struct codes *nextone;
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
			} else {
				if (*word == '%')
					strcpy(word, word+1);
				nextone->map = (char *)malloc(strlen(word)+1);
				if (nextone->map == NULL) {
					saltin_clean_up();
					clearcodes();
					fprintf(stderr,"saltin: no more memory available.\n");
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
				if (chrs == '\n') l++;
			}
			term = chrs;
		} else if (chrs != '\n')
			word[index++] = chrs;
		else {
			fflush(stdout);
			fprintf(stderr, "ERROR: Found newline in string on line %d in file %s.\n",l,fname);
			fclose(fdic);
			saltin_clean_up();
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
			} if (*f == 'c' || *f == 'C') {
				coding = 2;
			} if (*f == 's' || *f == 'S') {
				coding = 3;
			} else {
				fprintf(stderr, "Please choose one of: 'a'\n");
				saltin_clean_up();
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

static char mgetc() {
	register int c;

	do {
		if ((c=getc_cr(fpin, NULL)) == '\n')
			lnum++;
	} while (c == '\032') ;
	if (feof(fpin))
		return('\0');
	return((char) c);
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
			if (*(st+1) == '%')
				strcpy(st,st+2);
			else
				strcpy(st,st+1);
		} else if (*st == ']' || *st == '.')
			strcpy(st,st+1);
		else
			st++;
	}
}

static void saltin_shiftright(char *st, int num) {
	register int i;

	for (i=strlen(st); i >= 0; i--)
		st[i+num] = st[i];
}

static void removeExtraSpaceFromTier(char *st) {
	int i;

	for (i=0; st[i] != EOS; ) {
		if (st[i]==' ' || st[i]=='\t' || (st[i]=='<' && (i==0 || st[i-1]==' ' || st[i-1]=='\t'))) {
			i++;
			while (st[i] == ' ' || st[i] == '\t')
				strcpy(st+i, st+i+1);
		} else
			i++;
	}
}

static void remJunkFrontAndBackBlanks(char *st) {
	register int i;

	for (i=0; isSpace(st[i]) || st[i] == '\n' || st[i] == ','; i++) ;
	if (i > 0)
		strcpy(st, st+i);
	i = strlen(st) - 1;
	while (i >= 0 && (isSpace(st[i]) || st[i] == '\n' || st[i] == ',' || st[i] == NL_C || st[i] == SNL_C)) i--;
	st[i+1] = EOS;
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
	saltin_shiftright(st+1, 3);
	st[1] = ' ';
	st[2] = delim;
	st[3] = ' ';
}

static int checkline(char *line) {
	char found = FALSE, *st, cb = FALSE, UTDfound;

	UTDfound = FALSE;
	for (st=line; *st; st++) {
		if (*st == '.' || *st == '!' || *st == '?') {
			UTDfound = TRUE;
			if (st[1] == ' ' && (st[2] == '[' || st[2] == '{')) {
				moveDelim(st);
				removeExtraSpaceFromTier(st);
			}
		}
		if (*st == '[') {
			if (*(st+1) == '%' && st == line)
				st[1] = '^';
		} else if (*st == '{') {
			if (*(st+1) == '%' && st == line)
				st[1] = '^';
			cb = TRUE;
			*st = '[';
		} else if (*st == '}') {
			cb = FALSE;
			*st = ']';
		} else {
			if (isalnum((unsigned char)*st) && !cb)
				found = TRUE;
		}
	}
	if (found)
		return(FALSE);
	if (coding == 1) {
		if (!UTDfound)
			strcat(line, ".");
		return(FALSE);
	}
	for (st=line; *st; st++) {
		if (*st == '[')
			strcpy(st,st+3);
		else if (*st == ']')
			strcpy(st,st+1);
	}
	return(TRUE);
}

static void fixuptime(char *st) {
	char *beg = st;

	for (; *st; st++) {
		if (*st == ':' && (*(st-1) == '(' || isdigit(*(st+1)))) {
			if ((!isdigit(*(st-1)) && !isdigit(*(st-2))) || st == beg) {
				if (*(st-1) == '(') {
					saltin_shiftright(st, 1);
					st[0] = '0';
					st += 1;
				} else {
					saltin_shiftright(st, 2);
					st[0] = '0';
					st[1] = '0';
					st += 2;
				}
			} else if (*(st-2) != '(' && (!isdigit(*(st-2)) || st-1 == beg)) {
				saltin_shiftright(st-1, 1);
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
					if ((sp=findsp(*st)) != (char *)NULL){
						sp++;
						i = strlen(sp)-1;
						saltin_shiftright(st,i-1);
						for (; i > 0; i--) *st++ = *sp++;
					}
				}
			}
		}
	}
}

static void print(const char *s) {
	if (flag)
		strcat(saltin_templine, "+");
	strcat(saltin_templine, s);
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
	if(flag) strcat(saltin_templine,"+");
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
	if ((c=saltin_templine[strlen(saltin_templine)-1])!= ' ' && c!= '\t') 
		strcat(saltin_templine," ");
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
		if(r == 0) print("zero");
		else conv(tline+i-3*r, r);
		if (!flag) saltin_templine[strlen(saltin_templine)-1] = EOS;
		if (!isdigit(c) && c != EOS) {
			strcat(saltin_templine,the_inline+j-1);
			strcat(saltin_templine," ");
		}
	}
}

static void fixupnumbers(char s, char *line) {
	int i;
	char *pos, tchr, isFZero;

	if (!isalpha(s))
		return;

	while (*line) {
		if (*line == '(' && uS.isPause(line,0,NULL,&i)) {
			if (*(line+1) == ':') {
				saltin_shiftright(line,1);
				line++;
				*line = '0';
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
			if (isalpha(*(line+1)) || isalpha(*(line-1)) || *(line-1) == '^' || *(line-1) == '-') 
				while (isdigit(*line)) line++;
			else {
				pos = line;
				while (isdigit(*line)) line++;
				tchr = *line;
				*line = EOS;
				anumber(pos);
				*line = tchr;
				strcpy(pos,line);
				saltin_shiftright(pos,strlen(saltin_templine));
				line = pos + strlen(saltin_templine);
				for (i=0; saltin_templine[i]; i++, pos++) *pos = saltin_templine[i];
				*saltin_templine = EOS;
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
				while (*t != EOS && !uS.isskip(t,0,&dFnt,MBF))
					t++;
				saltin_shiftright(t,1);
				*t = ']';
			}
		} else
			line++;
	}
}

static int saltin_tran(char *s) {
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

static void saltin_pm(int n, char *s) {
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
		d[1] = saltin_tran(str);
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
		saltin_pm(d[1],str);
		if (d[0] < 10)
			sprintf(s, "0%d-%s-%d", d[0], str, d[2]);
		else
			sprintf(s, "%d-%s-%d", d[0], str, d[2]);
		return(s);
	}
}

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
	else
	if (isupper((unsigned char)(int)chr) && (!nomap || isForce))
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
		saltin_shiftright(st, 2);
		st[0] = '0';
		st[1] = ' ';
	}
}

static char isSkipChar(char *st, int pos) {
	if (pos < 0 || uS.isskip(st, pos, &dFnt, MBF) || st[pos] == EOS || st[pos] == '\n')
		return(TRUE);
	return(FALSE);
}

static void Sophie_cleanupSpeaker(char *st) {
	int i, j, k;

	i = 0;
	while (st[i] != EOS) {
		if (uS.mStrnicmp(st+i, "oooh>", 5) == 0) {
			strcpy(st+i, st+i+1);
			for (j=i+4; isSpace(st[j]) || st[j] == '\n'; j++) ;
			if (strncmp(st+j, "[/?]", 4) == 0) {
				for (k=i; k > 0 && st[k] != '<'; k--) ;
				if (uS.mStrnicmp(st+k, "<ooh", 4) == 0) {
					strcpy(st+k, st+k+1);
					i--;
					j--;
				}
				i += 3;
				strcpy(st+i, st+j+4);
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
				}
				i += 4;
				strcpy(st+i, st+j+4);
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
				}
				i += 2;
				strcpy(st+i, st+j+4);
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
				}
				i += 3;
				strcpy(st+i, st+j+4);
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
				}
				i += 2;
				strcpy(st+i, st+j+4);
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
				}
				i += 3;
				strcpy(st+i, st+j+4);
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
				}
				i += 4;
				strcpy(st+i, st+j+4);
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
				}
				strcpy(st+i+4, st+j+4);
				saltin_shiftright(st+i, 2);
				st[i++] = '&';
				st[i++] = '=';
				i += 4;
				saltin_shiftright(st+i, 1);
				st[i++] = 's';
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
				}
				strcpy(st+i+6, st+j+4);
				saltin_shiftright(st+i, 2);
				st[i++] = '&';
				st[i++] = '=';
				i += 6;
			} else
				i++;
		} else if (uS.mStrnicmp(st+i, "[% ew:", 6) == 0) {
			for (j=i+5; st[j] != ']' && !isSpace(st[j]) && st[j] != EOS; j++) ;
			if (st[j] == ']') {
				strcpy(st+i+1, st+i+4);
				st[i+1] = ':';
				st[i+2] = ' ';
				i = j - 2;
				saltin_shiftright(st+i, 4);
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
			saltin_shiftright(st+i+1, 1);
			st[i+1] = '=';
			st[i+2] = '!';
		} else if (uS.mStrnicmp(st+i, "{% whispers}", 12) == 0) {
			st[i] = '[';
			st[i+11] = ']';
			saltin_shiftright(st+i+1, 1);
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
			saltin_shiftright(st+i, 1);
			st[i] = '&';
			i += 3;
			saltin_shiftright(st+i, 2);
			st[i] = ' ';
			st[i+1] = '&';
			i += 4;
		} else if (uS.mStrnicmp(st+i, "all_done", 8) == 0) {
			st[i+3] = '+';
		} else if (uS.mStrnicmp(st+i, "all_gone", 8) == 0) {
			st[i+3] = '+';
		} else if (uS.mStrnicmp(st+i, "ok", 2) == 0) {
			if ((i == 0 || uS.isskip(st, i-1, &dFnt, MBF) || st[i-1] == EOS || st[i-1] == '\n' || st[i-1] == '{') &&
				(isSkipChar(st,i+2) || st[i+2] == '{')) {
				saltin_shiftright(st+i+2, 2);
				i += 2;
				st[i++] = 'a';
				st[i++] = 'y';
			} else
				i++;
		} else if ((i == 0 || uS.isskip(st, i-1, &dFnt, MBF)) && uS.isskip(st, i+2, &dFnt, MBF) &&
				   (uS.mStrnicmp(st+i, "uh", 2) == 0 || uS.mStrnicmp(st+i, "um", 2) == 0)) {
			saltin_shiftright(st+i, 1);
			st[i] = '&';
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
					saltin_shiftright(st+j, 2);
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
					strcat(saltin_pcodes," [+ ");
					strcat(saltin_pcodes, templineC2+1);
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
			removeExtraSpaceFromTier(st);
			remJunkFrontAndBackBlanks(st);
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
			removeExtraSpaceFromTier(st);
			remJunkFrontAndBackBlanks(st);
			for (i=0; st[i] == '\t' || st[i] == ' ' || st[i] == ','; i++) ;
			if (i > 0)
				strcpy(st, st+i);
		}
		return(TRUE);
	}
	return(FALSE);
}

static void prline(char s, char *line) {
	char *sp;
	struct depTiers *t;

	for (; *line == '\t' || *line == ' '; line++) ;
	changenames(s,line);
	fixupnumbers(s,line);
	cleanup_line(s, line);
	if ((sp=findsp(s)) != NULL) {
		if (saltin_pcodes[0] != EOS) {
			if (coding == 3)
				uS.lowercasestr(saltin_pcodes, &dFnt, MBF);
			strcat(line, saltin_pcodes);
			saltin_pcodes[0] = EOS;
		}
		if (coding == 3) {
			if (overlap == -1)
				overlap = 0;
			spareTier2[0] = EOS;
			removeExtraSpaceFromTier(line);
			Sophie_AddZero(line);
			Sophie_cleanupSpeaker(line);
			Sophie_extractSin(line, templineC1, spareTier2);
			Sophie_CapitalizeCleanup(line);
			uS.lowercasestr(spareTier2, &dFnt, MBF);
			if (saltin_pcodes[0] != EOS) {
				uS.lowercasestr(saltin_pcodes, &dFnt, MBF);
				uS.remFrontAndBackBlanks(line);
				strcat(line, saltin_pcodes);
				saltin_pcodes[0] = EOS;
			}
			Sophie_AddZero(line);
		}
		uS.remFrontAndBackBlanks(line);
		removeExtraSpaceFromTier(line);
		strcpy(templineC, sp);
		if (*templineC == '*') {
			SpFound = TRUE;
			uS.remFrontAndBackBlanks(comm);
			removeExtraSpaceFromTier(comm);
			if (coding != 3 && *comm != EOS) {
				printout("%com:", comm, NULL, NULL, TRUE);
				comm[0] = EOS;
			}
			uS.remFrontAndBackBlanks(def_line);
			removeExtraSpaceFromTier(def_line);
			if (*def_line != EOS) {
				printout("%def:", def_line, NULL, NULL, TRUE);
				def_line[0] = EOS;
			}
			remJunkFrontAndBackBlanks(comment);
			removeExtraSpaceFromTier(comment);
			if (comment[0] != EOS) {
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
		if (checkline(line)) {
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
			removeExtraSpaceFromTier(spareTier2);
			if (spareTier2[0] != EOS)
				printout("%sin:", spareTier2, NULL, NULL, TRUE);
			spareTier2[0] = EOS;
			uS.remFrontAndBackBlanks(comm);
			removeExtraSpaceFromTier(comm);
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
			removeExtraSpaceFromTier(lcode);
			if (*lcode) {
				cleanCodes(lcode);
				printout("%cod:", lcode, NULL, NULL, TRUE);
				*lcode = EOS;
			}
		}
		if (brerror) {
			fprintf(fpout,"@Comment:\t%s\n", "ERROR: <> brackets misused");
		}
		brerror = FALSE;
		pos = 0;
		*utterance->line = EOS;
	} else if (s == '+') {
		cleancom(line);
		for (; *line == '\t' || *line == ' '; line++) ;
		if (*line) {
			int i;

			if (deff) {
				if (*def_line != EOS)
					strcat(def_line, ", ");
				strcat(def_line, line);
			} else if ((*line == 'i' || *line == 'I')		  &&
					   (*(line+1) == 'd' || *(line+1) == 'D') &&
					   *(line+2) == ' ') {
				line[2] = '=';
				i = strlen(line) - 1;
				for (; i >= 0 && line[i] == ' ' && line[i] == '.'; i--) ;
				line[++i] = EOS;
				if (*line) {
/*
					if (coding == 3 && SpFound) {
						if (*comm != EOS)
							strcat(comm, ", ");
						strcat(comm, line);
					} else {
*/
						if (*comment != EOS)
							strcat(comment, ", ");
						strcat(comment, line);
//					}
				}
			} else if ((*line == 'd' || *line == 'D')	   &&
					 (*(line+1) == 'o' || *(line+1) == 'O') &&
					 (*(line+2) == 'b' || *(line+2) == 'B') &&
					 *(line+3) == ' ') {
				i = strlen(line) - 1;
				for (; i >= 0 && line[i] == ' ' && line[i] == '.'; i--) ;
				line[++i] = EOS;
				i = 4;
				while (line[i] != 'l' && line[i] != 'o' && 
					!isdigit(line[i]) && line[i] != EOS) i++;
				strcpy(templineC1, line+i);
				strcpy(templineC3, "Birth of CHI");
				strcat(templineC3, DateRepr(templineC1));
				printout("@Comment:", templineC3, NULL, NULL, TRUE);
			} else if ((*line == 'd' || *line == 'D')	   &&
					 (*(line+1) == 'o' || *(line+1) == 'O') &&
					 (*(line+2) == 'e' || *(line+2) == 'E') &&
					 *(line+3) == ' ') {
				i = strlen(line) - 1;
				for (; i >= 0 && line[i] == ' ' && line[i] == '.'; i--) ;
				line[++i] = EOS;
				i = 4;
				while (line[i] != 'l' && line[i] != 'o' && 
					!isdigit(line[i]) && line[i] != EOS) i++;
				strcpy(templineC1, line+i);
				printout("@Date:", DateRepr(templineC1), NULL, NULL, TRUE);
			} else {
				for (; *line == '\t' || *line == ' '; line++) ;
				if (coding == 3 && overlap != 0 && isOvertalk(line, TRUE)) {
					if (*line) {
						if (comment[0] != EOS)
							strcat(comment, ", ");
						strcat(comment, line);
						remJunkFrontAndBackBlanks(comment);
						removeExtraSpaceFromTier(comment);
						if (comment[0] != EOS)
							printout("@Comment:", comment, NULL, NULL, TRUE);
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
/*
					if (coding == 3 && SpFound) {
						if (*comm != EOS)
							strcat(comm, ", ");
						strcat(comm, line);
					} else {
*/
						if (comment[0] != EOS)
							strcat(comment, ", ");
						strcat(comment, line);
//					}
				}
			}
		}
		deff = FALSE;
		pos = 0;
		*utterance->line = EOS;
	} else if (s == '-') {
		fixuptime(line);
		if (coding == 2)
			;
		else if (SpFound)
			printout("%tim:", line, NULL, NULL, TRUE);
		else
			printout("@Time Start:", line, NULL, NULL, TRUE);
		pos = 0;
		*utterance->line = EOS;
	} else if (s == '=') {
		cleancom(line);
		for (; *line == '\t' || *line == ' '; line++) ;
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
	} else if (s == ';' || s == ':') {
		strcat(tim,utterance->line);
		pos = 0;
		*utterance->line = EOS;
	} else
		fprintf(stderr, "** ERROR: unknown symbol <%c> found on %d line.\n",s,lnum);
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

static char parseline(char chr, char isComment) {
	char *st = utterance->line;

	if (chr == '^') {
		st[pos] = EOS;
		if (coding == 0) {
			strcat(st," xxx");
			pos += 4;
		}
		strcat(st," +/.");
		pos += 4;
	} else if (chr == '>' || chr == '~') {
		st[pos] = EOS;
		if (coding == 0) {
			strcat(st," xxx");
			pos += 4;
		}
		strcat(st," +...");
		pos += 5;
	} else if (chr == ',') {
		while (pos >= 0 && st[pos-1] == ' ')
			pos--;
		if (curspeaker != ':' && curspeaker != '-' && curspeaker != ';')
			st[pos++] = chr;
	} else if (chr == '.' || chr == '?' || chr == '!') {
		if (curspeaker != ':' && curspeaker != '-' && curspeaker != ';') {
			if (!isComment && isalnum(curspeaker) && pos > 0 && !isSpace(st[pos-1]))
				st[pos++] = ' ';
			st[pos++] = chr;
		}
	} else if (chr == ':') {
		if (pos > 0 && isdigit(st[pos-1])) {
			char isParanFound = isAlreadyParan(st, pos-1);

			if (curspeaker != '-' && !isParanFound) {
				st[pos] = EOS;
				pos = fix(st, pos-1);
				st[pos++] = ':';
			} else
				st[pos++] = ':';
			chr = mgetc();
			if (isdigit(chr)) {
				do {
					st[pos++] = chr;
					chr = mgetc();
				} while (isdigit(chr) || chr == ':') ;
			}
			if (curspeaker != '-') {
				st[pos++] = '.';
				if (!isParanFound)
					st[pos++] = ')';
			}
		} else {
			chr = mgetc();
			if (isdigit(chr)) {
				if (curspeaker != '-') {
					st[pos++] = '(';
				}
				st[pos++] = ':';
				do {
					st[pos++] = chr;
					chr = mgetc();
				} while (isdigit(chr)) ;
				if (curspeaker != ':') {
					st[pos++] = '.';
					st[pos++] = ')';
				}
			} else if (!isalpha(st[pos-1]) && !isalpha(chr)) {
				if (curspeaker != '-') {
					st[pos++] = '(';
					st[pos++] = '.';
					st[pos++] = ')';
				}
			} else
				st[pos++] = ':';
		}
		letread = TRUE;
	} else if (chr == '/' && curspeaker != '+' && curspeaker != '-' && curspeaker != '=') {
		char isStarFound = FALSE;

//		st[pos++] = '^';
		chr = mgetc();
		if (chr == '*') {
			int i, j, t;

			i = pos - 2;
			t = pos - 1;
			while (!uS.isskip(st,i,&dFnt,MBF) && i >= 0)
				i--;
			i++;
			if (i < t) {
				isStarFound = TRUE;
				st[t] = '-';
				j = (t - i) + 4;
				saltin_shiftright(st+t, j);
				pos = pos + j;
				j = t;
				st[j++] = ' ';
				st[j++] = '[';
				st[j++] = '*';
				st[j++] = ' ';
				while (i < t)
					st[j++] = st[i++];
			}
			st[pos++] = '0';
			chr = mgetc();
		}
tryagain:
		if (chr == '3') {
			chr = mgetc();
			if (isdigit(chr) || chr == '/') {
				st[pos++] = '3';
				if (chr == '/') {
					if (isStarFound)
						st[pos++] = '-';
					else
						st[pos++] = '^';
				} else
					st[pos++] = Lisupper(chr, FALSE);
			} else {
				if (pos > 0 && (st[pos-1] == 's' || st[pos-1] == 'S' ||
								st[pos-1] == 'o' || st[pos-1] == 'O'))
					st[pos++] = 'e';
				st[pos++] = Lisupper(chr, FALSE);
			}
		} else if (chr == 'D' || chr == 'd') {
			if (pos > 0 && !uS.isskip(st,pos-1,&dFnt,MBF))
				st[pos++] = 'e';
			st[pos++] = Lisupper(chr, FALSE);
		} else if (chr == 'E' || chr == 'e') {
			chr = mgetc();
			if (chr == 'D' || chr == 'd') {
				if (pos > 0 && (st[pos-1] != 'e' && st[pos-1] != 'E'))
					st[pos++] = 'e';
				st[pos++] = Lisupper(chr, FALSE);
			} else {
				st[pos++] = 'e';
				letread = TRUE;
			}
		} else if (chr == 'S' || chr == 's') {
			chr = mgetc();
			if (chr == '/') {
				chr = mgetc();
				if (chr == 'Z' || chr == 'z') {
					st[pos++] = 's';
					st[pos++] = '\'';
				} else {
					st[pos++] = 's';
					goto tryagain;
				}
			} else {
				if (pos > 0 && st[pos-1] == 's')
					st[pos++] = 'e';
				else if ((pos > 7 && uS.mStrnicmp(st+pos-8, "scratchy", 8) == 0)) {
					pos--;
					if (isupper((unsigned char)(int)st[pos])) {
						st[pos++] = 'E';
					} else {
						st[pos++] = 'e';
					}
				} else if (pos > 1 && uS.mStrnicmp(st+pos-1,"y",1) == 0) {
					if (uS.mStrnicmp(st+pos-2,"a",1)==0 || uS.mStrnicmp(st+pos-2,"e",1)==0 ||
						uS.mStrnicmp(st+pos-2,"i",1)==0 || uS.mStrnicmp(st+pos-2,"o",1)==0 ||
						uS.mStrnicmp(st+pos-2,"u",1)==0 || uS.mStrnicmp(st+pos-2,"y",1)==0) {
					} else {
						pos--;
						if (isupper((unsigned char)(int)st[pos])) {
							st[pos++] = 'I';
							st[pos++] = 'E';
						} else {
							st[pos++] = 'i';
							st[pos++] = 'e';
						}
					}
				}
				if (pos > 0 && isupper((unsigned char)(int)st[pos-1]))
					st[pos++] = 'S';
				else
					st[pos++] = 's';
				letread = TRUE;
			}
		} else if (chr == '\'') {
			chr = mgetc();
			if (chr == 'N' || chr == 'n') {
				chr = mgetc();
				if (chr == '\'') {
					st[pos++] = 'n';
				} else {
					st[pos++] = '\'';
					st[pos++] = 'n';
				}
			} else
				st[pos++] = '\'';
			letread = TRUE;
		} else if (chr == 'R' || chr == 'r') {
			chr = mgetc();
			if (!isalpha(chr)) {
				st[pos++] = 'e';
			} else {
				st[pos++] = '\'';
			}
			st[pos++] = 'r';
			letread = TRUE;
		} else if (chr == 'M' || chr == 'm') {
			if (pos > 0 && (st[pos-1] != '\''))
				st[pos++] = '\'';
			st[pos++] = 'm';
		} else if (chr == 'Z' || chr == 'z') {
			st[pos++] = '\'';
			st[pos++] = 's';
		} else if (chr == 'N' || chr == 'n') {
			chr = mgetc();
			if (chr == '\'') {
				st[pos++] = 'n';
				st[pos++] = '\'';
				chr = mgetc();
			} else if (!isalpha(chr)) {
				st[pos++] = 'e';
				st[pos++] = 'n';
			} else st[pos++] = 'n';
			letread = TRUE;
		} else if (chr == 'I' || chr == 'i') {
			st[pos++] = chr;
			chr = mgetc();
			if (chr == 'N' || chr == 'n') {
				st[pos++] = chr;
				chr = mgetc();
				if (chr == 'G' || chr == 'g') {
					if (st[pos-3] == 'E' || st[pos-3] == 'e') {
						pos -= 3;
						st[pos] = st[pos+1];
						pos++;
						st[pos] = st[pos+1];
						pos++;
					}
					st[pos++] = chr;
				} else {
					st[pos++] = '(';
					st[pos++] = chr;
					st[pos++] = ')';
					letread = TRUE;
				}
			} else if (chr == 'E' || chr == 'e') {
				st[pos++] = 'e';
				chr = mgetc();
				if (chr == 'S' || chr == 's') {
					if (isStarFound)
						st[pos++] = '-';
					else
						st[pos++] = '^';
					st[pos++] = 's';
				} else letread = TRUE;
			} else letread = TRUE;
		} else letread = TRUE;
		if (isStarFound)
			st[pos++] = ']';
	} else if (chr == 'I' || chr == 'i') {
		if (pos == 0 || !isalpha(st[pos-1])) {
			chr = mgetc();
			if (!isalpha(chr)) {
				if (chr != ']')
					st[pos++] = 'I';
				else
					st[pos++] = 'i';
				letread = TRUE;
			} else {
				st[pos++] = 'i';
				st[pos++] = Lisupper(chr, FALSE);
			}
		} else st[pos++] = 'i';
	} else if (chr == '%') {
		chr = mgetc();
		while (isalnum((unsigned char)chr)) {
			st[pos++] = Lisupper(chr, FALSE);
			chr = mgetc();
		}
		st[pos] = EOS;
		if (isComment)
			strcat(st," percent");
		else
			strcat(st,"@c");
		pos = strlen(st);
		letread = TRUE;
	} else if (chr == '*') {
		if (pos > 0 && (isalnum((unsigned char)st[pos-1]) || st[pos-1] == ')')) {
			int i;
			i = pos - 1;
			while (!uS.isskip(st,i,&dFnt,MBF) && i >= 0) {
				st[i+1] = st[i];
				i--;
			}
			st[++i] = '&';
			pos++;
		} else {
			chr = mgetc();
			if (isalnum((unsigned char)chr))
				st[pos++] = '0';
			else
				st[pos++] = '*';
			letread = TRUE;
		}
	} else if (!isComment && (chr== 'X' || chr== 'x') && (pos == 0 || !isalnum((unsigned char)st[pos-1]))) {
		int num = 1;
		while((chr=mgetc()) == 'X' || chr == 'x')
			num++;
		letread = TRUE;
		st[pos] = EOS;
		if (num == 1) {
			if (!isalnum((unsigned char)chr)) {
				strcat(st, "xxx");
				pos += 3;
			} else
				st[pos++] = 'x';
		} else {
			if (isalnum((unsigned char)chr))
				st[pos++] = '&';
			else {
				strcat(st, "xxx");
				pos += 3;
			}
		}
	} else if (!isComment && (chr == 'X' || chr == 'x') && pos != 0 && isalnum((unsigned char)st[pos-1])) {
		chr = mgetc();
		letread = TRUE;
		if (chr == 'x' || chr == 'X') {
			int i;
			while((chr=mgetc()) == 'X' || chr == 'x') ;
			i = pos - 1;
			while (!uS.isskip(st,i,&dFnt,MBF) && i >= 0) {
				st[i+1] = st[i];
				i--;
			}
			st[++i] = '&';
			pos++;
		} else
			st[pos++] = 'x';
	} else
		st[pos++] = Lisupper(chr, FALSE);
	return(chr);
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
			strcat(saltin_pcodes," [+ im]");
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

static char getcom(char end, char isComment) {
	int  lpos;
	char chr, PCode, lastC, *code, t;

	chr = mgetc();
	while (chr != end) {
		if (chr == '\0') {
			if (feof(fpin)) {
				fprintf(stderr, "ERROR: No matching %c found on line %d in file %s\n", end, lnum,oldfname);
				saltin_clean_up();
				clearcodes();
				cutt_exit(0);
			}
		} else if (chr == '\n') {
			while (chr == '\n') chr = mgetc();
			if (chr != ' ' && chr != '\t') {
				fprintf(stderr, "ERROR: No matching %c found on line %d in file %s\n", end, lnum,oldfname);
				saltin_clean_up();
				clearcodes();
				cutt_exit(0);
			} else while (chr == ' ' || chr == '\t') chr = mgetc();
			utterance->line[pos++] = ' ';
		} else if (!isComment && chr == '(' && curspeaker != '+' && curspeaker != '=') {
			lpos = pos;
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '<';
			chr = getcom(')', FALSE);
			if (checkmaze(lpos)) {
				utterance->line[pos++] = '>';
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				utterance->line[pos++] = '/';
				utterance->line[pos++] = '?';
				utterance->line[pos++] = ']';
				utterance->line[pos++] = ' ';
			}
			if (!letread)
				chr = mgetc();
		} else if (!isComment && chr == '<' && curspeaker != '+' && curspeaker != '=') {
			if (hel) {
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				utterance->line[pos++] = '%';
				utterance->line[pos++] = ' ';
				chr = getcom('>', FALSE);
				utterance->line[pos++] = ']';
			} else {
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '<';
				chr = getcom('>', FALSE);
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
						remJunkFrontAndBackBlanks(comment);
						removeExtraSpaceFromTier(comment);
						if (comment[0] != EOS)
							printout("@Comment:", comment, NULL, NULL, TRUE);
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
		} else if (chr == '[') {
			if (pos < 1)
				lastC = ' ';
			else
				lastC = utterance->line[pos-1];
			utterance->line[pos++] = ' ';
			utterance->line[pos++] = '[';
			lpos = pos;
			utterance->line[pos++] = '%';
			utterance->line[pos++] = ' ';
			chr = getcom(']', FALSE);
			if (!letread)
				chr = mgetc();
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
				saltin_shiftright(utterance->line, 1);
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
				saltin_shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '>';
				for (lpos=lpos-1; lpos >= 0 && (isalpha(utterance->line[lpos]) || utterance->line[lpos] == '\''); lpos--) ;
				lpos++;
				saltin_shiftright(utterance->line+lpos, 1);
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
				saltin_shiftright(utterance->line+lpos, 1);
				utterance->line[lpos] = '>';
				for (lpos=lpos-1; lpos >= 0 && (isalpha(utterance->line[lpos]) || utterance->line[lpos] == '\''); lpos--) ;
				lpos++;
				saltin_shiftright(utterance->line+lpos, 1);
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
				if (!uselcode && coding == 1 && !isalnum((unsigned char)lastC)) {
					utterance->line[pos-1] = EOS;
					strcat(lcode, " ");
					strcat(lcode, utterance->line+lpos-1);
					strcat(lcode, "]");
					pos = lpos - 2;
				} else if (PCode) {
					utterance->line[pos] = EOS;
					strcat(saltin_pcodes," [+ ");
					strcat(saltin_pcodes,utterance->line+lpos);
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
			chr = parseline(chr, isComment);
			if (!letread) chr = mgetc();
			letread = FALSE;
		}
	}
	return(chr);
}

static void saltin_remblanks(char *st) {
	int i;
	
	for (i=strlen(st)-1; i >= 0 && (st[i] == ' ' || st[i] == '\t'); i--) ;
	st[i+1] = EOS;
}

static char lgetline(char chr) {
	int lpos, dquote = -1;
	char PCode, lastC, *code, t;

	letread = FALSE;
//	if (nomap) {
//		if (isupper((unsigned char)(int)chr))
//			chr = (char)tolower((unsigned char)chr);
//	}
	if (toupper((unsigned char)overlap) == toupper((unsigned char)curspeaker) &&
		toupper((unsigned char)oldsp) != toupper((unsigned char)curspeaker)) { 
		fprintf(fpout,"@Comment:\t%s\n", "ERROR: Can't find matching closing overlap for line:");
		fprintf(fpout,"\t%s\n", spareTier1);
		overlap = 0;
	}
	while (1) {
		if (chr == '\0') {
			if (feof(fpin)) {
				if (dquote != -1) {
					utterance->line[dquote] = '"';
					dquote = -1;
				}
				break;
			}
		} else if (chr == '\n') {
			if (dquote != -1) {
				utterance->line[dquote] = '"';
				dquote = -1;
			}
			while (chr == '\n')
				chr = mgetc();
			if (chr != ' ' && chr != '\t') break;
			else while (chr == ' ' || chr == '\t') chr = mgetc();
			utterance->line[pos++] = ' ';
		} else {
			if (chr == '{') {
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '{';
				utterance->line[pos++] = '%';
				utterance->line[pos++] = ' ';
				chr = getcom('}', TRUE);
				utterance->line[pos++] = '}';
			} else if (chr == '[') {
				if (pos < 1)
					lastC = ' ';
				else
					lastC = utterance->line[pos-1];
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '[';
				lpos = pos;
				utterance->line[pos++] = '%';
				utterance->line[pos++] = ' ';
				chr = getcom(']', TRUE);
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
					saltin_shiftright(utterance->line, 1);
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
					saltin_shiftright(utterance->line+lpos, 1);
					utterance->line[lpos] = '>';
					for (lpos=lpos-1; lpos >= 0 && (isalpha(utterance->line[lpos]) || utterance->line[lpos] == '\''); lpos--) ;
					lpos++;
					saltin_shiftright(utterance->line+lpos, 1);
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
					saltin_shiftright(utterance->line+lpos, 1);
					utterance->line[lpos] = '>';
					for (lpos=lpos-1; lpos >= 0 && (isalpha(utterance->line[lpos]) || utterance->line[lpos] == '\''); lpos--) ;
					lpos++;
					saltin_shiftright(utterance->line+lpos, 1);
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
					if (curspeaker == '+') ;
					else if (!uselcode && coding == 1 && !isalnum((unsigned char)lastC)) {
						utterance->line[pos-1] = EOS;
						strcat(lcode, " ");
						strcat(lcode,utterance->line+lpos-1);
						strcat(lcode,"]");
						pos = lpos - 2;
					} else if (PCode) {
						utterance->line[pos] = EOS;
						strcat(saltin_pcodes," [+ ");
						strcat(saltin_pcodes,utterance->line+lpos);
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
			} else if (chr == '(' && curspeaker != '+' && curspeaker != '=') {
				lpos = pos;
				utterance->line[pos++] = ' ';
				utterance->line[pos++] = '<';
				chr = getcom(')', FALSE);
				if (checkmaze(lpos)) {
					utterance->line[pos++] = '>';
					utterance->line[pos++] = ' ';
					utterance->line[pos++] = '[';
					utterance->line[pos++] = '/';
					utterance->line[pos++] = '?';
					utterance->line[pos++] = ']';
					utterance->line[pos++] = ' ';
				}
			} else if (chr == '<' && curspeaker != '+' && curspeaker != '=') {
				if (hel) {
					utterance->line[pos++] = ' ';
					utterance->line[pos++] = '[';
					utterance->line[pos++] = '%';
					utterance->line[pos++] = ' ';
					chr = getcom('>', FALSE);
					utterance->line[pos++] = ']';
				} else {
					utterance->line[pos++] = ' ';
					utterance->line[pos++] = '<';
					chr = getcom('>', FALSE);
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
							remJunkFrontAndBackBlanks(comment);
							removeExtraSpaceFromTier(comment);
							if (comment[0] != EOS)
								printout("@Comment:", comment, NULL, NULL, TRUE);
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
			} else if (chr == '"') {
				if (dquote == -1) {
					dquote = pos;
					utterance->line[pos++] = (char)0xe2;
					utterance->line[pos++] = (char)0x80;
					utterance->line[pos++] = (char)0x9c;
				} else {
					dquote = -1;
					utterance->line[pos++] = (char)0xe2;
					utterance->line[pos++] = (char)0x80;
					utterance->line[pos++] = (char)0x9d;
				}
			} else
				chr = parseline(chr, FALSE);
			if (!letread)
				chr = mgetc();
			letread = FALSE;
		}
	}
	utterance->line[pos] = EOS;
	saltin_remblanks(utterance->line);
	return(chr);
}

static void printIDs(char *part) {
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
				strcpy(templineC1, "eng|change_me_later|");
				strcat(templineC1, code);
				strcat(templineC1, "|||||");
				strcat(templineC1, role);
				strcat(templineC1, "|||");
				printout("@ID:\t", templineC1, NULL, NULL, TRUE);
			}
		}
		b = e;
	}
}

static char gsp(char chr) {
	int i, p, sp;
	char part[256], done = FALSE;

	sp = -1;
	*part = EOS;
	chr = mgetc();
	while (!done) {
		while (!isalpha(chr) && chr != '\n' && !feof(fpin)) chr = mgetc();
		if (chr == '\n') {
			while (chr == '\n') chr = mgetc();
			if (chr != ' ' && chr != '\t') done = TRUE;
		} else if (!feof(fpin)) {
			if (*part != EOS)
				strcat(part,", ");
			sp++;
			name[sp][0] = Lisupper(chr, TRUE);
			strcpy(name[sp]+1,"*   :");
			for (i=2; isalpha(chr) && i < 5; i++) {
				name[sp][i] = Lisupper(chr, TRUE);
				chr = mgetc();
			}
			if (uS.mStrnicmp(name[sp]+2,"exa",3)==0 || uS.mStrnicmp(name[sp]+2,"res",3)==0) {
				while (isalpha(chr)) chr = mgetc();
				strcat(part,"INV Investigator");
				strcpy(name[sp]+2, "INV:");
			} else if (uS.mStrnicmp(name[sp]+2, "sub", 3) == 0) {
				while (isalpha(chr)) chr = mgetc();
				strcat(part,"PAR Participant");
				strcpy(name[sp]+2, "PAR:");
			} else if (uS.mStrnicmp(name[sp]+2, "chi", 3) == 0) {
				while (isalpha(chr)) chr = mgetc();
				strcat(part,"CHI Target_Child");
				strcpy(name[sp]+2, "CHI:");
			} else if (uS.mStrnicmp(name[sp]+2,"mot",3)==0 || uS.mStrnicmp(name[sp]+2,"mom",3)==0) {
				while (isalpha(chr)) chr = mgetc();
				strcat(part,"MOT Mother");
				strcpy(name[sp]+2, "MOT:");
			} else if (uS.mStrnicmp(name[sp]+2,"fat",3)==0) {
				while (isalpha(chr)) chr = mgetc();
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
				strcat(part, name[sp]+2);
				for (p=strlen(part); part[p] != ':' && p > i; p--) ;
				for (p--; part[p] == ' ' && p >= i; p--) ;
				p++;
				while (isalpha(chr)) {
					part[p++] = chr;
					chr = mgetc();
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
		}
	}
	if (*part != EOS) {
		printout("@Participants:\t", part, NULL, NULL, TRUE);
		printIDs(part);
	}
	return(chr);
}

void call() {
	char chr, ft = TRUE;

	curspeaker = '\0';
	SpFound = FALSE;
	chr = mgetc();
	while (chr == ' ' || chr == '\t' || chr == '\n')
		chr = mgetc();
	isSameLine = FALSE;
	fprintf(fpout, "%s\n", UTF8HEADER);
	fprintf(fpout, "@Begin\n");
	fprintf(fpout, "@Languages:	eng\n");
	while (!feof(fpin)) {
/*
printf("chr=%c;\n", chr);
*/
		if (chr == '$') {
			chr = gsp(chr);
		} else {
			if (isalpha(chr)) {
				curspeaker = Lisupper(chr, TRUE);
				ft = FALSE;
			} else
				curspeaker = chr;
			if (curspeaker != '+')
				clearcodes();
			if (isalpha(curspeaker) && *tim) {
				strcat(utterance->line,tim);
				strcat(utterance->line," ");
				pos = strlen(utterance->line);
				*tim = EOS;
			}
			while ((chr=mgetc()) == ' ' || chr == '\t') ;
			chr = lgetline(chr);
			if (curspeaker == '@' && utterance->line[0] == 'u' && utterance->line[1] == 't' &&
				utterance->line[2] == 'f' && utterance->line[3] == '8') {
				pos = 0;
				*utterance->line = EOS;
			} else if (curspeaker == '=' && ft)
				prline('+',utterance->line);
			else
				prline(curspeaker,utterance->line);
			if (isalpha(curspeaker)) {
				isSameLine = FALSE;
				if (toupper((unsigned char)oldsp) != toupper((unsigned char)curspeaker) && overlap == 0)
					oldsp = '\0';
				else
					oldsp = curspeaker;
			}
		}
	}
	if (overlap != 0) { 
		fprintf(fpout,"@Comment:\t%s\n", "ERROR: Can't find matching closing overlap for line:");
		fprintf(fpout,"\t%s\n", spareTier1);
	}
	printout("@End", NULL, NULL, NULL, FALSE);
	clearcodes();
}
