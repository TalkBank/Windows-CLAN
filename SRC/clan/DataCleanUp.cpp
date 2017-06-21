/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 3
#include "cu.h"

#if !defined(UNX)
#define _main dc_main
#define call dc_call
#define getflag dc_getflag
#define init dc_init
#define usage dc_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define PERIOD 50

extern char OverWriteFile;

static char BoldAtt, tierCont, other;


void usage() {
	printf("Usage: dataclean [b c o %s] filename(s)\n", mainflgs());
	puts("+b : work on 'BOLD' problem ONLY.");
	puts("+c : work on '++'and '+,' problem ONLY.");
	puts("+o : work on on all other problems ONLY.");
	mainusage(TRUE);
}

void init(char f) {
	if (f) {
		stout = FALSE;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		onlydata = 1;
		BoldAtt = FALSE;
		tierCont = FALSE;
		other = FALSE;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = DATACLEANUP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}

static long dc_FindAndChangeSpeaker(char *sp, AttTYPE *att, long isFound) {
	char changeAtts = FALSE;
	long pos;

	for (pos=0L; sp[pos] != EOS; pos++) {
		if (att[pos] != 0) {
			changeAtts = TRUE;
			att[pos] = 0;
		}
	}
	if (changeAtts)
		isFound++;
	for (pos=0L; sp[pos] != ':' && sp[pos] != '\n' && sp[pos] != EOS; pos++) ;
	if (sp[pos] == ':') {
		pos++;
		if (sp[pos] == ' ') {
			sp[pos] = '\t';
			isFound++;
		} else if (sp[pos] == '\n' || sp[pos] == EOS) {
			att_shiftright(sp+pos, att+pos, 1);
			sp[pos] = '\t';
			isFound++;
			return(isFound);
		}
	}
	return(isFound);
}

static long dc_FindAndChangeLine(char *line, AttTYPE *att, long isFound) {
	char isSQ;
	long pos;

	isSQ = FALSE;
	pos = 0L;
	while (line[pos] != EOS) {
		if (uS.isRightChar(line, pos, '[',&dFnt,MBF))
			isSQ = TRUE;
		else if (uS.isRightChar(line, pos, ']',&dFnt,MBF))
			isSQ = FALSE;

		if (isSQ) ;
		else
		if (line[pos] == '\t' && pos > 0 && line[pos-1] != '\n') {
			line[pos] = ' ';
			isFound++;
		} else 
		if (pos > 0 && uS.isRightChar(line,pos,'+',&dFnt,MBF) &&
					uS.IsUtteranceDel(line, pos+1) && !uS.isskip(line, pos-1, &dFnt, MBF)) {
			att_shiftright(line+pos, att+pos, 1);
			line[pos++] = ' ';
			isFound++;
		} else 
		if (uS.isRightChar(line,pos,'[',&dFnt,MBF) && !uS.isskip(line,pos-1,&dFnt,MBF) && pos > 0) {
			att_shiftright(line+pos, att+pos, 1);
			line[pos] = ' ';
			isFound++;
		} else 
		if (uS.isRightChar(line,pos,']',&dFnt,MBF) && !uS.isskip(line,pos+1,&dFnt,MBF) && line[pos+1] != EOS) {
			pos++;
			att_shiftright(line+pos, att+pos, 1);
			line[pos] = ' ';
			isFound++;
		} else 
		if ( uS.isRightChar(line,pos,  '#',&dFnt,MBF) && uS.isRightChar(line,pos+1,'l',&dFnt,MBF) && 
			 uS.isRightChar(line,pos+2,'o',&dFnt,MBF) && uS.isRightChar(line,pos+3,'n',&dFnt,MBF) &&
			 uS.isRightChar(line,pos+4,'g',&dFnt,MBF) && uS.isskip(line,pos+5,&dFnt,MBF)) {
			att_cp(0, line+pos, line+pos+3, att+pos, att+pos+3);
			line[pos++] = '#';
			line[pos++] = '#';
			isFound++;
		} else 
		if (pos > 0 && uS.isRightChar(line,pos,'.',&dFnt,MBF) && uS.isRightChar(line,pos+1,'.',&dFnt,MBF) && 
					uS.isRightChar(line,pos+2,'.',&dFnt,MBF) && !uS.isRightChar(line,pos-1,'+',&dFnt,MBF)) {
			if (!isSpace(line[pos-1])) {
				att_shiftright(line+pos, att+pos, 2);
				line[pos++] = ' ';
				line[pos++] = '+';
			} else {
				att_shiftright(line+pos, att+pos, 1);
				line[pos++] = '+';
			}
			isFound++;
		}
		pos++;
	}
	return(isFound);
}

static long dc_CheckTierContSymbol(char *line, char isSame, long isFound) {
	long pos;

	for (pos=0L; isSpace(line[pos]); pos++) ;
	if (line[pos] == '+' && line[pos+1] == '+' && isSame) {
		line[pos+1] = ',';
		isFound++;
	}
	if (line[pos] == '+' && line[pos+1] == ',' && !isSame) {
		line[pos+1] = '+';
		isFound++;
	}
	return(isFound);
}

static long dc_CheckBold(char *line, AttTYPE *att, long isFound) {
	long pos, t;
	char isOKtoChange;
	char changeAtts = FALSE;

	pos = 0;
	isOKtoChange = FALSE;
	do {
		if (isSpace(line[pos])) {
			changeAtts = FALSE;
			for (; line[pos] != EOS && isSpace(line[pos]); pos++) {
				if (att[pos] != 0) {
					changeAtts = TRUE;
					att[pos] = 0;
				}
			}
			if (changeAtts)
				isFound++;
		} else if (line[pos] == '[' && line[pos+1] == '-') {
			changeAtts = FALSE;
			for (; line[pos] != EOS && line[pos] != ']' && line[pos] != EOS; pos++) {
				if (att[pos] != 0) {
					changeAtts = TRUE;
					att[pos] = 0;
				}
			}
			if (line[pos] == ']') {
				if (att[pos] != 0) {
					changeAtts = TRUE;
					att[pos] = 0;
				}
				pos++;
			}
			if (changeAtts)
				isFound++;
		} else if (line[pos] == '+') {
			changeAtts = FALSE;
			for (; line[pos] != EOS && !isSpace(line[pos]) && line[pos] != '[' && line[pos] != EOS; pos++) {
				if (att[pos] != 0) {
					changeAtts = TRUE;
					att[pos] = 0;
				}
			}
			if (changeAtts)
				isFound++;
		} else
			isOKtoChange = TRUE;
	} while (!isOKtoChange && line[pos] != EOS) ;

	if (line[pos] == EOS)
		return(isFound);

	changeAtts = FALSE;
	for (; line[pos] != EOS; pos++) {
		if (uS.IsUtteranceDel(line, pos)) {
			t = pos;
			for (; t > 0; t--) {
				if (line[t] == '+' || uS.isskip(line,t,&dFnt,MBF) || isalnum((unsigned char)line[t]))
					break;
			}
			if (line[t] == '+') {
				for (; t <= pos; t++) {
					if (att[t] != 0) {
						changeAtts = TRUE;
						att[t] = 0;
					}
				}
			}
			isOKtoChange = FALSE;
		}

		if (!isOKtoChange) {
			if (att[pos] != 0) {
				changeAtts = TRUE;
				att[pos] = 0;
			}
		}
	}

	if (changeAtts)
		isFound++;

	changeAtts = FALSE;
	for (pos--; pos > 0 && isSpace(line[pos]); pos--) {
		if (!isOKtoChange) {
			if (att[pos] != 0) {
				changeAtts = TRUE;
				att[pos] = 0;
			}
		}
	}

	if (changeAtts)
		isFound++;

	return(isFound);
}

void call() {
	char RightSpeaker = FALSE, isPartFound = 1;
	char spname[SPEAKERLEN];
	long tlineno = 0L;
	long isFound;

	spname[0] = EOS;
	isFound = 0L;
	templineC4[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (!BoldAtt && !tierCont) {
			if (uS.partcmp(utterance->speaker, "@Participants", FALSE, FALSE)) {
				isPartFound = -1;
			}
			if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE) || uS.partcmp(utterance->speaker,"@Options:",FALSE,FALSE)) {
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				continue;
			} else if (isPartFound == 0) {
				strcat(templineC4, utterance->speaker);
				strcat(templineC4, utterance->line);
				isFound++;
				continue;
			}
			if (uS.partcmp(utterance->speaker, "@Begin", FALSE, FALSE)) {
				isPartFound = 0;
			}
		}
		if (!stout) {
			tlineno = tlineno + 1L;
			if (tlineno % PERIOD == 0)
				fprintf(stderr,"\r%ld ",tlineno);
		}
		isFound = dc_FindAndChangeSpeaker(utterance->speaker, utterance->attSp, isFound);

		if (!checktier(utterance->speaker) || 
							(!RightSpeaker && *utterance->speaker == '%') ||
							(*utterance->speaker == '*' && nomain)) {
			if (*utterance->speaker == '*') {
				if (nomain)
					RightSpeaker = TRUE;
				else
					RightSpeaker = FALSE;
				strcpy(spname, utterance->speaker);
			}
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		} else {
			if (*utterance->speaker == '*') {
				RightSpeaker = TRUE;
				if (!BoldAtt && !other)
					isFound = dc_CheckTierContSymbol(utterance->line, !uS.mStricmp(spname, utterance->speaker), isFound);
				if (!tierCont && !other)
					isFound = dc_CheckBold(utterance->line, utterance->attLine, isFound);
				strcpy(spname, utterance->speaker);
			}

			if (!BoldAtt && !tierCont)
				isFound = dc_FindAndChangeLine(utterance->line, utterance->attLine, isFound);

			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
		if (!BoldAtt && !tierCont) {
			if (isPartFound == -1) {
				if (templineC4[0] != EOS)
					fputs(templineC4, fpout);
				templineC4[0] = EOS;
				isPartFound = 1;
			}
		}
	}
	if (!stout)
		fprintf(stderr,"\n");
	if (isFound == 0L && fpout != stdout && !stout
#if !defined(UNX)
											 && !WD_Not_Eq_OD
#endif
											 					) {
		fprintf(stderr,"**- NO changes made in this file\n");
		if (!replaceFile) {
			fclose(fpout);
			fpout = NULL;
			if (unlink(newfname))
				fprintf(stderr, "Can't delete output file \"%s\".", newfname);
		}
	} else if (isFound > 0L)
		fprintf(stderr,"**+ %ld changes made in this file\n", isFound);
}

void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'b':
				BoldAtt = TRUE;
				if (tierCont || other) {
					fprintf(stderr,"+b option can't be used with either +c or +o\n");
					cutt_exit(0);
				}
				break;
		case 'c':
				tierCont = TRUE;
				if (BoldAtt || other) {
					fprintf(stderr,"+c option can't be used with either +b or +o\n");
					cutt_exit(0);
				}
				break;
		case 'o':
				other = TRUE;
				if (BoldAtt || tierCont) {
					fprintf(stderr,"+o option can't be used with either +b or +c\n");
					cutt_exit(0);
				}
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}
