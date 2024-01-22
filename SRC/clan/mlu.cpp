/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "mllib.h"
#include <math.h>
#include "c_curses.h"

#ifdef UNX
	#define RGBColor int
#endif

#if !defined(UNX)
#define _main mlu_main
#define call mlu_call
#define getflag mlu_getflag
#define init mlu_init
#define usage mlu_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char R8;
extern char *Toldsp;
extern char OverWriteFile;
extern struct tier *headtier;

#define MLUSP struct mlu_cnt
struct mlu_cnt {
	char isSpeakerFound;
	char isPSDFound; /* +/. */
	char *fname;
	char *sp;
	char *ID;
	char *utt;
	float morf, mlu_utter, morfsqr;
	MLUSP *next_sp;
} ;

static MLUSP *mlu_head;
static char mlu_ftime1, mlu_ftime2, mlu_isMorExcluded, mlu_isCombineSpeakers, morTierSelected;
static float morf, mlu_utter, morfsqr;

void usage() {
	puts("MLU- Mean Length Utterance computes the number of utterances, morphemes");
	puts("     and their ratio.");
#ifdef UNX
	printf("MLU NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO RUN MLU ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cMLU NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO RUN MLU ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	printf("Usage: mlu [bS cS gS oN %s] filename(s)\n", mainflgs());
//	printf("Usage: mlu [aC bS cS gF %s] filename(s)\n", mainflgs());
//	puts("+a : count all utterances even the ones without words");
//	puts("+at: count utterances that have [+ trn] code even the ones without words");
	printf("+bS: add all S characters to morpheme delimiters list (default: %s)\n", rootmorf);
	puts("-bS: remove all S characters from be morphemes list");
	puts("-b:  counts words, not morphemes");
	puts("+cS: look for clause marker S or markers listed in file @S");
	puts("+gS: exclude utterance consisting solely of specified word S or words in file @S");
	puts("+o3: combine selected speakers from each file into one results list for that file");
	mainusage(FALSE);
#ifdef UNX
	printf("MLU NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.\n");
	printf("TO RUN MLU ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.\n");
#else
	printf("%c%cMLU NOW WORKS ON \"%%mor:\" TIER BY DEFAULT.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
	printf("%c%cTO RUN MLU ON MAIN SPEAKER TIER PLEASE USE \"-t%%mor:\" OPTION.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+<\\% bch>");
		addword('\0','\0',"+uh");
		addword('\0','\0',"+um");
		addword('\0','\0',"+fil|uh");
		addword('\0','\0',"+fil|um");
//		addword('\0','\0',"+xxx");
//		addword('\0','\0',"+yyy");
//		addword('\0','\0',"+www");
//		addword('\0','\0',"+xxx@s*");
//		addword('\0','\0',"+yyy@s*");
//		addword('\0','\0',"+www@s*");
//		addword('\0','\0',"+unk|xxx");
//		addword('\0','\0',"+unk|yyy");
//		addword('\0','\0',"+*|www");
		addword('\0','\0',"+:*");
		addword('\0','\0',"+$*");
		maininitwords();
		mor_initwords();
		isMLUEpostcode = TRUE;
		ml_isSkip = FALSE;
		ml_isXXXFound = FALSE;
		ml_isYYYFound = FALSE;
		ml_isPlusSUsed = FALSE;
		ml_WdMode = '\0';
		ml_WdHead = NULL;
		mlu_ftime1 = TRUE;
		mlu_ftime2 = TRUE;
		morTierSelected = FALSE;
		mlu_isMorExcluded = FALSE;
		mlu_isCombineSpeakers = FALSE;
	} else {
		if (mlu_ftime2) {
			maketierchoice("%mor",'+',FALSE);
			nomain = TRUE;
			mlu_ftime2 = FALSE;
			if (isMORSearch() || ml_isclause())
				linkMain2Mor = TRUE;
		} else if (morTierSelected && ml_isclause()) {
			linkMain2Mor = TRUE;
		}
		if (R8)
			linkMain2Mor = FALSE;
		if (onlydata == 1 || onlydata == 3) {
			combinput = TRUE;
			OverWriteFile = TRUE;
			AddCEXExtension = ".xls";
		}
	}
	if (!combinput || f) {
		mlu_head = NULL;
	}
	if ((onlydata == 1 || onlydata == 2 || onlydata == 3) && mlu_ftime1 && chatmode) {
		maketierchoice("@ID:",'+',FALSE);
/*
		if (IDField == NULL && *headtier->tcode == '@') {
			fputs("\nPlease use \"+t\" option to specify speaker code or ID\n",stderr);
			ml_exit(0);
		}
*/
		mlu_ftime1 = FALSE;
		if ((onlydata == 1 || onlydata == 3) && !f_override)
			stout = FALSE;
	}
	ml_spchanged = TRUE;
	if (Toldsp == NULL) {
		Toldsp = (char *)malloc(SPEAKERLEN+1);
		*Toldsp = EOS;
	}
}

static void mlu_outputIDInfo(char *fname, char *s) {
	int  cnt;
	char isBlank;

	cnt = 0;
	isBlank = TRUE;
	fprintf(fpout,"%s\t", fname);
	for (; *s != EOS; s++) {
		if (*s == '|') {
			if (isBlank)
				fprintf(fpout,".");
			fprintf(fpout,"\t");
			isBlank = TRUE;
			cnt++;
		} else if (isSpace(*s)) {
			if (isBlank) {
				fprintf(fpout,".");
				isBlank = FALSE;
			} else
				fputc(*s, fpout);
		} else {
			isBlank = FALSE;
			fputc(*s, fpout);
		}
	}
	if (cnt < 10)
		fprintf(fpout,".\t");
}

static void mlu_pr_result(void) {
	char *sFName;
	float tt;
	MLUSP *ts;

	if (onlydata == 1) {
		excelHeader(fpout, newfname, 95);
		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"File,Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field,");
		sprintf(spareTier1,"#%s,#%s,%s/%s,Standard deviation",
				(ml_isclause() ? "clauses" : "utterances"),((rootmorf[0] == EOS || mlu_isMorExcluded) ? "words" : "morphemes"),
				((rootmorf[0] == EOS || mlu_isMorExcluded) ? "words" : "morphemes"),(ml_isclause() ? "clauses" : "utterances"));
		excelCommasStrCell(fpout, spareTier1);
		excelRow(fpout, ExcelRowEnd);
	} else if (onlydata == 3) {
		excelHeader(fpout, newfname, 95);
		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"File,Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field,");
		sprintf(spareTier1,"#%s, utterance", ((rootmorf[0] == EOS || mlu_isMorExcluded) ? "words" : "morphemes"));
		excelCommasStrCell(fpout, spareTier1);
		excelRow(fpout, ExcelRowEnd);
	}
	while (mlu_head) {
		morf	  = mlu_head->morf;
		mlu_utter = mlu_head->mlu_utter;
		morfsqr   = mlu_head->morfsqr;
		if (!onlydata || !chatmode) {
			fprintf(fpout, "MLU for Speaker: %s\n", mlu_head->sp);
			if (ml_isXXXFound && ml_isYYYFound) {
				fprintf(fpout,"  MLU (xxx and yyy are EXCLUDED from the morpheme counts, but are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				fprintf(fpout,"  MLU (www is EXCLUDED from the %s and morpheme counts):\n", (ml_isclause() ? "clause" : "utterance"));
			} else if (ml_isXXXFound) {
				fprintf(fpout,"  MLU (xxx is EXCLUDED from the morpheme counts, but is INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				fprintf(fpout,"  MLU (yyy and www are EXCLUDED from the %s and morpheme counts):\n", (ml_isclause() ? "clause" : "utterance"));
			} else {
				fprintf(fpout,"  MLU (xxx, yyy and www are EXCLUDED from the %s and morpheme counts):\n", (ml_isclause() ? "clause" : "utterance"));
			}
			fprintf(fpout,"	Number of: %s = %.0f, %s = %.0f\n",
					(ml_isclause() ? "clauses" : "utterances"), mlu_utter,
					((rootmorf[0] == EOS || mlu_isMorExcluded) ? "words" : "morphemes"), morf);
			if (mlu_utter > (float)0.0) {
				fprintf(fpout,"\tRatio of %s over %s = %.3f\n",
						((rootmorf[0] == EOS || mlu_isMorExcluded) ? "words" : "morphemes"), 
							(ml_isclause() ? "clauses" : "utterances"), morf/mlu_utter);
				if (mlu_utter > (float)0.0) {
					// tt = (morfsqr - ((morf * morf) / mlu_utter)) / mlu_utter;
					// fprintf(fpout,"\tStandard deviation = %.3f\n", sqrt(tt));
					tt = (morfsqr / mlu_utter) - ((morf / mlu_utter) * (morf / mlu_utter));
					if (mlu_utter > (float)1.0 && tt >= (float)0.0) {
						fprintf(fpout,"\tStandard deviation = %.3f\n", sqrt(tt));
					} else
						fprintf(fpout,"\tStandard deviation = %s\n", "NA");
				}
			}
			putc('\n',fpout);
		} else if (onlydata == 1) {
			if (mlu_head->isSpeakerFound) {
				excelRow(fpout, ExcelRowStart);
				sFName = strrchr(mlu_head->fname, PATHDELIMCHR);
				if (sFName != NULL)
					sFName = sFName + 1;
				else
					sFName = mlu_head->fname;
				excelStrCell(fpout, sFName);
				if (mlu_head->ID) {
					excelOutputID(fpout, mlu_head->ID);
				} else {
					excelCommasStrCell(fpout, ".,.");
					excelStrCell(fpout, mlu_head->sp);
					excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
				}
				excelNumCell(fpout, "%.0f", mlu_utter);
				excelNumCell(fpout, "%.0f", morf);
				if (mlu_utter > (float)0.0) {
					excelNumCell(fpout, "%.3f", morf/mlu_utter);
					tt = (morfsqr / mlu_utter) - ((morf/mlu_utter) * (morf/mlu_utter));
					if (mlu_utter > (float)1.0 && tt >= (float)0.0) {
						excelNumCell(fpout, "%.3f", sqrt(tt));
					} else
						excelStrCell(fpout, "NA");
				} else {
					excelStrCell(fpout, "NA");
					excelStrCell(fpout, "NA");
				}
				excelRow(fpout, ExcelRowEnd);
			}
		} else if (onlydata == 3) {
			if (mlu_head->isSpeakerFound) {
				excelRow(fpout, ExcelRowStart);
				sFName = strrchr(mlu_head->fname, PATHDELIMCHR);
				if (sFName != NULL)
					sFName = sFName + 1;
				else
					sFName = mlu_head->fname;
				excelStrCell(fpout, sFName);
				if (mlu_head->ID) {
					excelOutputID(fpout, mlu_head->ID);
				} else {
					excelCommasStrCell(fpout, ".,.");
					excelStrCell(fpout, mlu_head->sp);
					excelCommasStrCell(fpout, ".,.,.,.,.,.,.,.");
				}
				excelNumCell(fpout, "%.0f", morf);
				if (mlu_head->utt == NULL)
					excelStrCell(fpout, "NA");
				else
					excelStrCell(fpout, mlu_head->utt);
				excelRow(fpout, ExcelRowEnd);
			}
		} else {
			if (mlu_head->ID) {
				mlu_outputIDInfo(mlu_head->fname, mlu_head->ID);
			} else
				fprintf(fpout,"%s\t.\t.\t%s\t.\t.\t.\t.\t.\t.\t.\t", mlu_head->fname, mlu_head->sp);
			fprintf(fpout,"%5.0f\t%5.0f", mlu_utter, morf);
			if (mlu_utter > (float)0.0) {
				fprintf(fpout,"\t%7.3f", morf/mlu_utter);
				tt = (morfsqr / mlu_utter) - ((morf/mlu_utter) * (morf/mlu_utter));
				if (mlu_utter > (float)1.0 && tt >= (float)0.0) {
					fprintf(fpout,"\t%7.3f", sqrt(tt));
				} else
					fprintf(fpout,"\tNA");
			} else
				fprintf(fpout,"\tNA\tNA");
			putc('\n',fpout);
		}
		ts = mlu_head;
		mlu_head = mlu_head->next_sp;
		if (ts->fname != NULL)
			free(ts->fname);
		if (ts->sp != NULL)
			free(ts->sp);
		if (ts->ID)
			free(ts->ID);
		if (ts->utt)
			free(ts->utt);
		free(ts);
	}
	if (onlydata == 1 || onlydata == 3)
		excelFooter(fpout);
	mlu_head = NULL;
}

static MLUSP *mlu_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound) {
	MLUSP *ts, *tsp;

	uS.remblanks(sp);
	if (onlydata == 3) {
		for (ts=mlu_head; ts != NULL; ts=ts->next_sp) {
			if (uS.mStricmp(ts->fname, fname) == 0 || (combinput && onlydata != 1)) {
				if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
					if (ts->ID != NULL && ID == NULL)
						ID = ts->ID;
					if (ts->isSpeakerFound == FALSE)
						return(ts);
				}
			}
		}
		isSpeakerFound = FALSE;
	} else {
		for (ts=mlu_head; ts != NULL; ts=ts->next_sp) {
			if (uS.mStricmp(ts->fname, fname) == 0 || (combinput && onlydata != 1)) {
				if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
					ts->isSpeakerFound = isSpeakerFound;
					return(ts);
				}
			}
		}
	}
	if ((ts=NEW(MLUSP)) == NULL)
		out_of_mem();
// ".xls"
	if ((ts->fname=(char *)malloc(strlen(fname)+1)) == NULL)
		out_of_mem();
	strcpy(ts->fname, fname);
	if ((ts->sp=(char *)malloc(strlen(sp)+1)) == NULL)
		out_of_mem();
	strcpy(ts->sp, sp);
	if (ID == NULL)
		ts->ID = NULL;
	else {
		if ((ts->ID=(char *)malloc(strlen(ID)+1)) == NULL)
			out_of_mem();
		strcpy(ts->ID, ID);
	}
	ts->utt = NULL;
	ts->isSpeakerFound = isSpeakerFound;
	ts->isPSDFound = FALSE;
	ts->morf	= (float)0.0;
	ts->mlu_utter   = (float)0.0;
	ts->morfsqr = (float)0.0;
	ts->next_sp = NULL;
	if (mlu_head == NULL) {
		mlu_head = ts;
	} else {
		for (tsp=mlu_head; tsp->next_sp != NULL; tsp=tsp->next_sp) ;
		tsp->next_sp = ts;
	}
	return(ts);
}

char mlu_excludeUtter(char *line, int pos, char *isWordsFound) { // xxx, yyy, www
	int  i, j;
	char isSkipFirst = FALSE;

	if (MBF) {
		if (my_CharacterByteType(line, (short)pos, &dFnt) != 0 ||
			my_CharacterByteType(line, (short)pos+1, &dFnt) != 0 ||
			my_CharacterByteType(line, (short)pos+2, &dFnt) != 0)
			isSkipFirst = TRUE;
	}
	if (!isSkipFirst) {
		i = 0;
		j = 0;
		if (pos == 0 || uS.isskip(line,pos-1,&dFnt,MBF)) {
			for (j=pos; line[j] == 'x' || line[j] == 'X' ||
						line[j] == 'y' || line[j] == 'Y' ||
						line[j] == 'w' || line[j] == 'W' || 
						line[j] == '(' || line[j] == ')'; j++) {
				if (line[j] != '(' && line[j] != ')')
					templineC2[i++] = line[j];
			}
		}
		if (i == 3) {
			if (uS.isskip(line, j, &dFnt,MBF) || line[j] == '@')
				templineC2[i] = EOS;
			else
				templineC2[0] = EOS;
		} else
			templineC2[i] = EOS;
		uS.lowercasestr(templineC2, &dFnt, FALSE);
		if ((ml_isXXXFound && strcmp(templineC2, "xxx") == 0) ||
			(ml_isYYYFound && strcmp(templineC2, "yyy") == 0)) {
			if (isWordsFound == NULL && (CntWUT == 2 || CntWUT == 3)) {
			} else {
				line[pos] = ' ';
				line[pos+1] = ' ';
				line[pos+2] = ' ';
			}
			if (isWordsFound != NULL)
				*isWordsFound = TRUE;
		} else if (strcmp(templineC2, "xxx") == 0 ||
				   strcmp(templineC2, "yyy") == 0 ||
				   strcmp(templineC2, "www") == 0) {
//			if (uS.isskip(line,pos+3,&dFnt,MBF) || line[pos+3] == EOS)
				return(TRUE);
		}
	}
	if (chatmode) {
		if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
			if (MBF) {
				if (my_CharacterByteType(line, (short)pos, &dFnt)   != 0 ||
					my_CharacterByteType(line, (short)pos+1, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+2, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+3, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+4, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+5, &dFnt) != 0 ||
					my_CharacterByteType(line, (short)pos+6, &dFnt) != 0)
					return(FALSE);
			}
			if (!uS.isskip(line,pos+7,&dFnt,MBF) && line[pos+7] != EOS)
				return(FALSE);
			strncpy(templineC2, line+pos, 7);
			templineC2[7] = EOS;
			uS.lowercasestr(templineC2, &dFnt, FALSE);
			if ((ml_isXXXFound && strcmp(templineC2, "unk|xxx") == 0) ||
				(ml_isYYYFound && strcmp(templineC2, "unk|yyy") == 0)) {
				if (isWordsFound == NULL && (CntWUT == 2 || CntWUT == 3)) {
				} else {
					line[pos] = ' ';
					line[pos+1] = ' ';
					line[pos+2] = ' ';
					line[pos+3] = ' ';
					line[pos+4] = ' ';
					line[pos+5] = ' ';
					line[pos+6] = ' ';
				}
				if (isWordsFound != NULL)
					*isWordsFound = TRUE;
			} else if (strcmp(templineC2, "unk|xxx") == 0 ||
					   strcmp(templineC2, "unk|yyy") == 0 ||
					   strcmp(templineC2, "unk|www") == 0)
				return(TRUE);
		}
	}
	return(FALSE);	
}

// C_NNLA, EVAL, EVALD, KIDEVAL, MAXWD, WDLEN and SUGAR
void call() {
	register int pos;
	MLUSP *ts = NULL;
	float tLocalmorf = 0.0, localmorf = 0.0;
	char tmp, isWordsFound, sq, aq, isSkip;
	char isPSDFound = FALSE, curPSDFound, isAmbigFound, isDepTierFound;

	isDepTierFound = FALSE;
	isSkip = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (ml_getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (linkMain2Mor && uS.partcmp(utterance->speaker,"%mor",FALSE,FALSE)) {
			removeMainTierWords(uttline);
		}
		if ((onlydata == 1 || onlydata == 2 || onlydata == 3) && chatmode) {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
					uS.remblanks(utterance->line);
					mlu_FindSpeaker(oldfname, templineC, utterance->line, FALSE);
				}
				continue;
			}
		}
		if (!chatmode) {
			strcpy(templineC, "GENERIC");
			ts = mlu_FindSpeaker(oldfname, templineC, NULL, TRUE);
			morf	  = ts->morf;
			mlu_utter = ts->mlu_utter;
			morfsqr   = ts->morfsqr;
			tLocalmorf = localmorf;
			isPSDFound = ts->isPSDFound;
		} else if (*utterance->speaker == '*') {
			if (mlu_isCombineSpeakers) {
				strcpy(templineC, "*COMBINED*");
			} else {
				strcpy(templineC, utterance->speaker);
//				ml_checktier(templineC);
			}
			ts = mlu_FindSpeaker(oldfname, templineC, NULL, TRUE);
			morf	  = ts->morf;
			mlu_utter = ts->mlu_utter;
			morfsqr   = ts->morfsqr;
			tLocalmorf = localmorf;
			isPSDFound = ts->isPSDFound;
		} else if (*utterance->speaker == '%') {
			isDepTierFound = TRUE;
			if (ts == NULL && nomain && mlu_isCombineSpeakers) {
				strcpy(templineC, "*COMBINED*");
				ts = mlu_FindSpeaker(oldfname, templineC, NULL, TRUE);
				morf	  = ts->morf;
				mlu_utter = ts->mlu_utter;
				morfsqr   = ts->morfsqr;
				tLocalmorf = localmorf;
				isPSDFound = ts->isPSDFound;
				isSkip = FALSE;
			}
		}
		if (*utterance->speaker == '*') {
			isSkip = FALSE;
			if (!ml_isclause()) {
				for (pos=0; utterance->line[pos] != EOS; pos++) {
					if ((pos == 0 || uS.isskip(utterance->line,pos-1,&dFnt,MBF)) && utterance->line[pos] == '+' && 
						uS.isRightChar(utterance->line,pos+1,',',&dFnt, MBF) && isPSDFound) {
						if (mlu_utter > 0.0)
							mlu_utter--;
						isPSDFound = FALSE;
					}
				}
				ts->mlu_utter = mlu_utter;
			}
			if (isMLUEpostcode && isPostCodeOnUtt(utterance->line, "[+ mlue]")) {
				isSkip = TRUE;
				localmorf = tLocalmorf;
			}
			if (nomain) {
				for (pos=0; utterance->line[pos] != EOS; pos++) {
					if (mlu_excludeUtter(uttline, pos, &isWordsFound)) { // xxx, yyy, www
						isSkip = TRUE;
						localmorf = tLocalmorf;
						break;
					}
				}
				continue;
			}
		} else if (!nomain)
			isSkip = FALSE;
		curPSDFound = FALSE;
		isWordsFound = FALSE;
		pos = 0;
		sq = FALSE;
		aq = FALSE;
		do {
			if (mlu_excludeUtter(uttline, pos, &isWordsFound)) { // xxx, yyy, www
				isSkip = TRUE;
				localmorf = tLocalmorf;
				break;
			}
			if (uS.isRightChar(uttline, pos, '[', &dFnt, MBF)) {
				sq = TRUE;
			} else if (uS.isRightChar(uttline, pos, ']', &dFnt, MBF))
				sq = FALSE;
			else if (chatmode && *utterance->speaker == '%') {
				if (uS.isRightChar(uttline, pos, '<', &dFnt, MBF)) {
					int tl;
					for (tl=pos+1; !uS.isRightChar(uttline, tl, '>', &dFnt, MBF) && uttline[tl]; tl++) {
						if (isdigit(uttline[tl])) ;
						else if (uttline[tl]== ' ' || uttline[tl]== '\t' || uttline[tl]== '\n') ;
						else if ((tl-1 == pos+1 || !isalpha(uttline[tl-1])) && 
								 uS.isRightChar(uttline, tl, '-', &dFnt, MBF) && !isalpha(uttline[tl+1])) ;
						else if ((tl-1 == pos+1 || !isalpha(uttline[tl-1])) && 
								 (uS.isRightChar(uttline,tl,'u', &dFnt, MBF) || uS.isRightChar(uttline,tl,'U', &dFnt, MBF)) && 
								 !isalpha(uttline[tl+1])) ;
						else if ((tl-1 == pos+1 || !isalpha(uttline[tl-1])) && 
								 (uS.isRightChar(uttline,tl,'w', &dFnt, MBF) || uS.isRightChar(uttline,tl,'W', &dFnt, MBF)) && 
								 !isalpha(uttline[tl+1])) ;
						else if ((tl-1 == pos+1 || !isalpha(uttline[tl-1])) && 
								 (uS.isRightChar(uttline,tl,'s', &dFnt, MBF) || uS.isRightChar(uttline,tl,'S', &dFnt, MBF)) && 
								 !isalpha(uttline[tl+1])) ;
						else
							break;
					}
					if (uS.isRightChar(uttline, tl, '>', &dFnt, MBF)) aq = TRUE;
				} else
					if (uS.isRightChar(uttline, pos, '>', &dFnt, MBF)) aq = FALSE;
			}
			if (!uS.isskip(uttline,pos,&dFnt,MBF) && !sq && !aq) {
				isAmbigFound = FALSE;
				isWordsFound = TRUE;
				tmp = TRUE;
				while (uttline[pos]) {
					if (uttline[pos] == '^')
						isAmbigFound = TRUE;
					if (uS.isskip(uttline,pos,&dFnt,MBF)) {
						if (uS.IsUtteranceDel(utterance->line, pos)) {
							if (!uS.atUFound(utterance->line, pos, &dFnt, MBF))
								break;
						} else
							break;
					}
					if (!uS.ismorfchar(uttline, pos, &dFnt, rootmorf, MBF) && !isAmbigFound) {
						if (tmp) {
							if (uttline[pos] != EOS) {
								if (pos >= 2 && uttline[pos-1] == '+' && uttline[pos-2] == '|')
									;
								else {
									morf = morf + 1;
									localmorf = localmorf + 1;
								}
							}
							tmp = FALSE;
						}
					} else
						tmp = TRUE;
					pos++;
				}
			}
			if (!ml_isclause()) {
				if ((pos == 0 || uS.isskip(utterance->line,pos-1,&dFnt,MBF)) && utterance->line[pos] == '+' && 
					uS.isRightChar(utterance->line,pos+1,',',&dFnt, MBF) && isPSDFound) {
					if (mlu_utter > 0.0)
						mlu_utter--;
					else if (mlu_utter == 0.0) 
						isWordsFound = FALSE;
				}
				if (isTierContSymbol(utterance->line, pos, TRUE)) {
					if (mlu_utter > 0.0)
						mlu_utter--;
					else if (mlu_utter == 0.0) 
						isWordsFound = FALSE;
				}
			}
			if (isTierContSymbol(utterance->line, pos, FALSE))
				curPSDFound = TRUE;
			if (ml_UtterClause(utterance->line, pos)) {
				if (uS.isRightChar(utterance->line, pos, '[', &dFnt, MBF)) {
					for (; utterance->line[pos] && !uS.isRightChar(utterance->line, pos, ']', &dFnt, MBF); pos++) ;
				}
				if (uS.isRightChar(utterance->line, pos, ']', &dFnt, MBF))
					sq = FALSE;
				if (isWordsFound) {
					if (localmorf != (float)0.0) {
						morfsqr = morfsqr + localmorf * localmorf;
						localmorf = (float)0.0;
					}
					mlu_utter = mlu_utter + (float)1.0;
					isWordsFound = FALSE;
				}
			}
			if (uttline[pos])
				pos++;
			else
				break;
		} while (uttline[pos]) ;
		if (!isSkip) {
			if (localmorf != (float)0.0) {
				morfsqr = morfsqr + localmorf * localmorf;
				localmorf = (float)0.0;
			}
			if (ts != NULL) {
				ts->morf	  = morf;
				ts->mlu_utter = mlu_utter;
				ts->morfsqr   = morfsqr;
				ts->isPSDFound = curPSDFound;
				if (onlydata == 3) {
					strcpy(templineC4, org_spTier);
					for (pos=0; templineC4[pos] != EOS; pos++) {
						if (templineC4[pos] == '\n')
							templineC4[pos] = ' ';
						if (templineC4[pos] == HIDEN_C) {
							templineC4[pos] = EOS;
							break;
						}
					}
					removeExtraSpace(templineC4);
					if ((ts->utt=(char *)malloc(strlen(templineC4)+1)) != NULL)
						strcpy(ts->utt, templineC4);
					ts->isSpeakerFound = TRUE;
				}
			}
		}
	}
	if (isDepTierFound && ts == NULL) {
		fprintf(stderr,"\n    No speaker tiers were specified.\n");
		fprintf(stderr,"    Try add +o3 option and running command again.\n\n");
	}
	if (!combinput)
		mlu_pr_result();
}

void getflag(char *f, char *f1, int *i) {
	int j;
	char *morf, *t, *s;

	f++;
	switch(*f++) {
		case 'b':
				morf = getfarg(f,f1,i);
				if (*(f-2) == '-') {
					if (*morf != EOS) {
						for (j=0; rootmorf[j] != EOS; ) {
							if (uS.isCharInMorf(rootmorf[j],morf)) 
								strcpy(rootmorf+j,rootmorf+j+1);
							else
								j++;
						}
					} else
						rootmorf[0] = EOS;
				} else {
					if (*morf != EOS) {
						t = rootmorf;
						rootmorf = (char *)malloc(strlen(t)+strlen(morf)+1);
						if (rootmorf == NULL) {
							fprintf(stderr,"No more space left in core.\n");
							ml_exit(1);
						}
						strcpy(rootmorf,t);
						strcat(rootmorf,morf);
						free(t);
					}
				}
				break;
		case 'c':
			if (!*f) {
				fprintf(stderr,"Specify clause delemeters after +c option.\n");
				ml_exit(0);
			} 
			if (*f == '@') {
				f++;
				ml_AddClauseFromFile('c', f);
			} else {
				if (*f == '\\')
					f++;
				ml_AddClause('c', f);
			}
			break;
		case 'g':
			if (*f == '@') {
				f++;
				ml_mkwdlist('g','e',getfarg(f,f1,i));
			} else {
				if (*f == '\\' && *(f+1) == '@')
					f++;
				ml_addwd('g','e',f);
			}
/*
			if (*(f-2) == '+') ml_mkwdlist('g','i',getfarg(f,f1,i));
			else ml_mkwdlist('g','e',getfarg(f,f1,i));
*/
			break;
		case 'o':
			if (*f == '3') {
				mlu_isCombineSpeakers = TRUE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 't':
			if (*(f-2) == '+' && *f == '%') {
				nomain = TRUE;
				mlu_ftime2 = FALSE;
				if (!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:"))
					morTierSelected = TRUE;
				linkMain2Mor = FALSE;
			} else if (*(f-2) == '+' && *f == '*' && *(f+1) == EOS) {
				mlu_isCombineSpeakers = TRUE;
			}
			if (*(f-2) == '-' && *f == '%') {
				if (!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:")) {
					mlu_ftime2 = FALSE;
					linkMain2Mor = FALSE;
					mlu_isMorExcluded = TRUE;
				}
			} else
				maingetflag(f-2,f1,i);
			break;
		case 's':
			t = getfarg(f,f1,i);
			s = t;
			if (*s == '+' || *s == '~')
				s++;
			if (*s == '[' || *s == '<') {
				s++;
				if (*s == '+') {
					for (s++; isSpace(*s); s++) ;
					if ((*s     == 'm' || *s     == 'M') &&
						(*(s+1) == 'l' || *(s+1) == 'L') &&
						(*(s+2) == 'u' || *(s+2) == 'U') &&
						(*(s+3) == 'e' || *(s+3) == 'E') &&
						(*(s+4) == ']' || *(s+4) == '>' )) {
						isMLUEpostcode = FALSE;
						break;
					}
				}
			}
			j = strlen(t) - 3;
			if (uS.mStricmp(t+j, "xxx") == 0) {
				if (*(f-2) == '+')
					ml_isXXXFound = TRUE;
				else {
					fprintf(stderr,"Utterances containing \"%s\" are excluded by default.\n", f);
					fprintf(stderr,"Excluding \"%s\" is not allowed.\n", f);
					ml_exit(0);
				}
				break;
			} else if (uS.mStricmp(t+j, "yyy") == 0) {
				if (*(f-2) == '+')
					ml_isYYYFound = TRUE;
				else {
					fprintf(stderr,"Utterances containing \"%s\" are excluded by default.\n", f);
					fprintf(stderr,"Excluding \"%s\" is not allowed.\n", f);
					ml_exit(0);
				}
				break;
			} else if (uS.mStricmp(t+j, "www") == 0) {
				fprintf(stderr,"%s \"%s\" is not allowed.\n", ((*(f-2) == '+') ? "Including" : "Excluding"), f);
				ml_exit(0);
				break;
			} else if (mlu_ftime2)
				linkMain2Mor = TRUE;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = MLU;
	chatmode = CHAT_MODE;
	OnlydataLimit = 3;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,mlu_pr_result);
	ml_WdHead = freeIEWORDS(ml_WdHead);
}
