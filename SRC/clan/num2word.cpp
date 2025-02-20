/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main num2word_main
#define call num2word_call
#define getflag num2word_getflag
#define init num2word_init
#define usage num2word_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define MINFILESNUM 300

extern struct tier *defheadtier;
extern char OverWriteFile;
extern int  totalFilesNum; // 2019-04-18 TotalNumFiles

static int  n2w_FileCount;
static char n2w_lang;
static double n2w_oPrc;

struct ZeroToTwenty {
	const char *dName;
};
static struct ZeroToTwenty zeroToTwenty[] = {
//           dName
	/* 0*/	{"zero"},
	/* 1*/	{"one"},
	/* 2*/	{"two"},
	/* 3*/	{"three"},
	/* 4*/	{"four"},
	/* 5*/	{"five"},
	/* 6*/	{"six"},
	/* 7*/	{"seven"},
	/* 8*/	{"eight"},
	/* 9*/	{"nine"},
	/*10*/	{"ten"},
	/*11*/	{"eleven"},
	/*12*/	{"twelve"},
	/*13*/	{"thirteen"},
	/*14*/	{"fourteen"},
	/*15*/	{"fifteen"},
	/*16*/	{"sixteen"},
	/*17*/	{"seventeen"},
	/*18*/	{"eighteen"},
	/*19*/	{"nineteen"},
} ;

struct TwentyToNintyNine {
	const char *dName;
};
static struct TwentyToNintyNine twentyToNintyNine[] = {
//           dName
	/* 0*/	{""},
	/* 1*/	{"ERROR"},
	/* 2*/	{"twenty"},
	/* 3*/	{"thirty"},
	/* 4*/	{"forty"},
	/* 5*/	{"fifty"},
	/* 6*/	{"sixty"},
	/* 7*/	{"seventy"},
	/* 8*/	{"eighty"},
	/* 9*/	{"ninety"},
} ;

void init(char f) {
	if (f) {
		n2w_FileCount = 0;
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		onlydata = 1;
		OverWriteFile = TRUE;
		n2w_lang = '\0';
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		if (n2w_lang == '\0') {
			fprintf(stderr, "\nPlease specify data language with \"+l\" option.\n");
			fprintf(stderr, "  eng - English, jpn - Japanese\n");
			cutt_exit(0);
		}
	}
}

void usage()			/* print proper usage and exit */
{
	puts("NUM2WORD replace Arabic numbers with Kanji words");
	printf("Usage: num2word [%s] filename(s)\n",mainflgs());
	printf("+lS: specify data language (eng - English, jpn - Japanese)\n");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = NUM2WORD;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	n2w_oPrc = 0.0;
	n2w_FileCount = 0;
	bmain(argc,argv,NULL);
	if (totalFilesNum > MINFILESNUM && !stin) { // 2019-04-18 TotalNumFiles
		fprintf(stderr, "\r    	     \r");
	}
}
	
void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'l':
			if (!*f) {
				fprintf(stderr,"Letter expected after %s option.\n", f-2);
				cutt_exit(0);
			}
			if (*f == 'e' || *f == 'E') {
				n2w_lang = 'e';
			} else if (*f == 'j' || *f == 'J') {
				n2w_lang = 'j';
			} else {
				fprintf(stderr, "This language is not supported: %s.\n", f);
				fprintf(stderr, "Choose: eng - English, jpn - Japanese\n");
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void convertJpnNumber(char *buf, char *out) {
	int i, cnt;

	out[0] = EOS;
	if (buf[0] == EOS)
		return;

	cnt = strlen(buf);
	i = 0;
	if (cnt == 5) {
		switch(buf[i]) {
			case '0':
				break;
			case '1':
				strcat(out, "一");
				break;
			case '2':
				strcat(out, "二");
				break;
			case '3':
				strcat(out, "三");
				break;
			case '4':
				strcat(out, "四");
				break;	
			case '5':
				strcat(out, "五");
				break;
			case '6':
				strcat(out, "六");
				break;
			case '7':
				strcat(out, "七");
				break;
			case '8':
				strcat(out, "八");
				break;
			case '9':
				strcat(out, "九");
				break;
		}
		strcat(out, "万");
		i++;
		cnt--;
	} else if (cnt == 6) {
		strcat(out, "十");
		i++;
		switch(buf[i]) {
			case '0':
				break;
			case '1':
				strcat(out, "一");
				break;
			case '2':
				strcat(out, "二");
				break;
			case '3':
				strcat(out, "三");
				break;
			case '4':
				strcat(out, "四");
				break;
			case '5':
				strcat(out, "五");
				break;
			case '6':
				strcat(out, "六");
				break;
			case '7':
				strcat(out, "七");
				break;
			case '8':
				strcat(out, "八");
				break;
			case '9':
				strcat(out, "九");
				break;
		}
		strcat(out, "万");
		i++;
		cnt -= 2;
	} else if (cnt == 7) {
		strcat(out, "百");
		i++;
		switch(buf[i]) {
			case '0':
				break;
			case '1':
				strcat(out, "一十");
				break;
			case '2':
				strcat(out, "二十");
				break;
			case '3':
				strcat(out, "三十");
				break;
			case '4':
				strcat(out, "四十");
				break;
			case '5':
				strcat(out, "五十");
				break;
			case '6':
				strcat(out, "六十");
				break;
			case '7':
				strcat(out, "七十");
				break;
			case '8':
				strcat(out, "八十");
				break;
			case '9':
				strcat(out, "九十");
				break;
		}
		strcat(out, "万");
		i++;
		cnt -= 2;
	}
	for (; buf[i] != EOS; i++) {
		switch(buf[i]) {
			case '0':
				break;
			case '1':
				if (cnt == 1)
					strcat(out, "一");
				break;
			case '2':
				strcat(out, "二");
				break;
			case '3':
				strcat(out, "三");
				break;
			case '4':
				strcat(out, "四");
				break;
			case '5':
				strcat(out, "五");
				break;
			case '6':
				strcat(out, "六");
				break;
			case '7':
				strcat(out, "七");
				break;
			case '8':
				strcat(out, "八");
				break;
			case '9':
				strcat(out, "九");
				break;
		}
		if (buf[i] == '0') {
		} else if (cnt == 4) {
			strcat(out, "千");
		} else if (cnt == 3) {
			strcat(out, "百");
		} else if (cnt == 2) {
			strcat(out, "十");
		} else if (cnt == 1) {
		}
		cnt--;
	}
}

static void engDigitsBlock(char *blockOfThree, char *lout) {
	int  num;
	char numSt[3];

	num = atoi(blockOfThree);
	if (strlen(blockOfThree) > 0 && num == 0)
		return;
	if (num >= 0 && num < 20) {
		strcat(lout, zeroToTwenty[num].dName);
	} else if (num >= 20 && num < 100) {
		numSt[1] = EOS;
		numSt[0] = blockOfThree[0];
		num = atoi(numSt);
		strcat(lout, twentyToNintyNine[num].dName);
		numSt[0] = blockOfThree[1];
		num = atoi(numSt);
		if (num > 0) {
			strcat(lout, "-");
			strcat(lout, zeroToTwenty[num].dName);
		}
	} else if (num >= 100 && num < 1000) {
		numSt[1] = EOS;
		numSt[0] = blockOfThree[0];
		num = atoi(numSt);
		strcat(lout, zeroToTwenty[num].dName);
		strcat(lout, " hundred ");
		numSt[2] = EOS;
		numSt[0] = blockOfThree[1];
		numSt[1] = blockOfThree[2];
		num = atoi(numSt);
		engDigitsBlock(numSt, lout);
	}
}

static void convertEngNumber(char *buf, char *out) {
	int blockI, cnt, blockNum;
	char blockOfThree[4], lout[BUFSIZ], *isPointFound, isRepeated;

	isPointFound = strrchr(buf, '.');
	if (isPointFound != NULL) {
		*isPointFound = EOS;
	}
	out[0] = EOS;
	isRepeated = FALSE;
repeat:
	if (isRepeated) {
		for (; *buf == '0'; buf++)
			strcat(out, " zero ");
	}
	if (buf[0] == EOS)
		return;

	cnt = strlen(buf);
	blockNum = 0;
	if (cnt >= 10) {
		blockI = 0;
		if (cnt-blockNum >= 12) {
			if (buf[blockNum] != '0')
				blockOfThree[blockI++] = buf[blockNum];
//			if (buf[blockNum+1] != '0')
				blockOfThree[blockI++] = buf[blockNum+1];
			blockOfThree[blockI++] = buf[blockNum+2];
			blockNum += 3;
		} else if (cnt-blockNum >= 11) {
			if (buf[blockNum] != '0')
				blockOfThree[blockI++] = buf[blockNum];
			blockOfThree[blockI++] = buf[blockNum+1];
			blockNum += 2;
		} else if (cnt-blockNum >= 10) {
			blockOfThree[blockI++] = buf[blockNum];
			blockNum += 1;
		}
		blockOfThree[blockI] = EOS;
		lout[0] = EOS;
		engDigitsBlock(blockOfThree, lout);
		if (lout[0] != EOS) {
			strcat(out, lout);
			strcat(out, " billion ");
		}
	}
	if (cnt >= 7) {
		blockI = 0;
		if (cnt-blockNum >= 9) {
			if (buf[blockNum] != '0')
				blockOfThree[blockI++] = buf[blockNum];
//			if (buf[blockNum+1] != '0')
				blockOfThree[blockI++] = buf[blockNum+1];
			blockOfThree[blockI++] = buf[blockNum+2];
			blockNum += 3;
		} else if (cnt-blockNum >= 8) {
			if (buf[blockNum] != '0')
				blockOfThree[blockI++] = buf[blockNum];
			blockOfThree[blockI++] = buf[blockNum+1];
			blockNum += 2;
		} else if (cnt-blockNum >= 7) {
			blockOfThree[blockI++] = buf[blockNum];
			blockNum += 1;
		}
		blockOfThree[blockI] = EOS;
		lout[0] = EOS;
		engDigitsBlock(blockOfThree, lout);
		if (lout[0] != EOS) {
			strcat(out, lout);
			strcat(out, " million ");
		}
	}
	if (cnt >= 4) {
		blockI = 0;
		if (cnt-blockNum >= 6) {
			if (buf[blockNum] != '0')
				blockOfThree[blockI++] = buf[blockNum];
//			if (buf[blockNum+1] != '0')
				blockOfThree[blockI++] = buf[blockNum+1];
			blockOfThree[blockI++] = buf[blockNum+2];
			blockNum += 3;
		} else if (cnt-blockNum >= 5) {
			if (buf[blockNum] != '0')
				blockOfThree[blockI++] = buf[blockNum];
			blockOfThree[blockI++] = buf[blockNum+1];
			blockNum += 2;
		} else if (cnt-blockNum >= 4) {
			blockOfThree[blockI++] = buf[blockNum];
			blockNum += 1;
		}
		blockOfThree[blockI] = EOS;
		lout[0] = EOS;
		engDigitsBlock(blockOfThree, lout);
		if (lout[0] != EOS) {
			strcat(out, lout);
			strcat(out, " thousand ");
		}
	}
	blockI = 0;
	if (cnt-blockNum >= 3) {
		if (buf[blockNum] != '0')
			blockOfThree[blockI++] = buf[blockNum];
//		if (buf[blockNum+1] != '0')
			blockOfThree[blockI++] = buf[blockNum+1];
		blockOfThree[blockI++] = buf[blockNum+2];
		blockNum += 3;
	} else if (cnt-blockNum >= 2) {
		if (buf[blockNum] != '0')
			blockOfThree[blockI++] = buf[blockNum];
		blockOfThree[blockI++] = buf[blockNum+1];
		blockNum += 2;
	} else if (cnt-blockNum >= 1) {
		blockOfThree[blockI++] = buf[blockNum];
		blockNum += 1;
	}
	blockOfThree[blockI] = EOS;
	lout[0] = EOS;
	engDigitsBlock(blockOfThree, out);
	
	if (isPointFound != NULL) {
		buf = isPointFound + 1;
		strcat(out, " point ");
		isPointFound = NULL;
		isRepeated = TRUE;
		goto repeat;
	}
}

static char isYearNumbers(int k, char *line, int j) {
	if (k == 2 && isdigit(line[j+1]) && isdigit(line[j+2]) &&
				  (!isdigit(line[j+3]) ||
				   (isdigit(line[j+3]) && isdigit(line[j+4]) && !isdigit(line[j+5]))))
		return(TRUE);
	return(FALSE);
}

void call() {
	int i, j, k;
	char num[BUFSIZ], out[BUFSIZ], *line, sq, pr, bullet, isDollarFound;
	double prc, ffc, ftf;

	n2w_FileCount++;
	if (totalFilesNum > MINFILESNUM && !stin) { // 2019-04-18 TotalNumFiles
		ffc = (float)n2w_FileCount;
		ftf = (float)totalFilesNum;
		prc = (ffc * 100.0000) / ftf;
		if (prc >= n2w_oPrc) {
			fprintf(stderr, "\r%.1lf%% ", prc);
			n2w_oPrc = prc + 0.1;
		}
	} else
		fprintf(stderr,"From file <%s>\n", oldfname);
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			bullet = FALSE;
			sq = FALSE;
			pr = FALSE;
			line = utterance->line;
			for (i=0; line[i] != EOS; i++) {
				if (line[i] == HIDEN_C)
					bullet = !bullet;
				else if (uS.isRightChar(line, i, '[', &dFnt, MBF))
					sq = FALSE; // TRUE;
				else if (uS.isRightChar(line, i, ']', &dFnt, MBF))
					sq = FALSE;
				if (uS.isRightChar(line, i, '(', &dFnt, MBF))
					pr = FALSE; // TRUE;
				else if (uS.isRightChar(line, i, ')', &dFnt, MBF))
					pr = FALSE;
				if (sq == TRUE || pr == TRUE || bullet == TRUE || (line[i] == 0 && !isdigit(line[i+1]))) {
				} else if (((line[i] == '$' && isdigit(line[i+1])) || isdigit(line[i])) &&
							(i == 0 || isSpace(line[i-1]) || line[i-1] == '(' || line[i-1] == ':' ||
							 line[i-1] == ',' || line[i-1] == '_')) {
					k = 0;
					j = i;
					if (line[j] == '$') {
						isDollarFound = TRUE;
						j++;
					} else {
						isDollarFound = FALSE;
					}
					for (; isdigit(line[j]) || line[j] == ',' || line[j] == '.'; j++) {
						if (line[j] == ',' ) {
							if (!isdigit(line[j+1]) || isYearNumbers(k, line, j))
								break;
						} else if (line[j] == '.') {
							if (!isdigit(line[j+1]))
								break;
							else
								num[k++] = line[j];
						} else
							num[k++] = line[j];
					}
					num[k] = EOS;
					out[0] = EOS;
					if (strcmp(num, "0") != 0) {
						if (n2w_lang == 'e') {
							convertEngNumber(num, out);
							uS.remblanks(out);
							removeExtraSpace(out);
						} else if (n2w_lang == 'j') {
							convertJpnNumber(num, out);
							uS.remblanks(out);
							removeExtraSpace(out);
						}
					}
					if (out[0] != EOS || (line[i-1] == ':' && strlen(num) > 0)) {
						if (isDollarFound)
							strcat(out, " dollars");
						if (line[j] == '%') {
							strcat(out, " percent");
							if (isalnum(line[j+1]))
								strcat(out, " ");
							att_cp(0,line+j,line+j+1,utterance->attLine+j,utterance->attLine+j+1);
						} else if (line[j] == '-') {
							strcat(out, " to");
							if (isalnum(line[j+1]))
								strcat(out, " ");
							att_cp(0,line+j,line+j+1,utterance->attLine+j,utterance->attLine+j+1);
						} else if (line[j] == 't' && line[j+1] == 'h' && isSpace(line[2])) {
						} else if (isalnum(line[j])) {
							strcat(out, " ");
						}
						att_cp(0,line+i,line+j,utterance->attLine+i,utterance->attLine+j);
						att_shiftright(line+i, utterance->attLine+i, strlen(out));
						for (k=0; out[k] != EOS; k++) {
							line[i+k] = out[k];
							utterance->attLine[i+k] = 0;
						}
//						fprintf(stderr, "%s: %s\n", num, out);
						i = j - 1;
					}
				}
			}
		}
		printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
}
