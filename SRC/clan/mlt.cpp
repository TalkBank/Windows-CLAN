/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "mllib.h"
#include <math.h>

#if !defined(UNX)
#define _main mlt_main
#define call mlt_call
#define getflag mlt_getflag
#define init mlt_init
#define usage mlt_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern char *Toldsp;
extern char OverWriteFile;
extern struct tier *headtier;

#define MLTSP struct mlt_cnt
struct mlt_cnt {
	char isSpeakerFound;
	char isPSDFound; /* +/. */
	char *fname;
	char *sp;
	char *ID;
	float mlt_words, mlt_utter, mlt_turn, mlt_word_sqr;
	MLTSP *next_sp;
} ;

static MLTSP *mlt_head;
static char mlt_ftime, mlt_foutput, count_empty_utterences, isFromCall, mlt_isCombineSpeakers;
static float mlt_utter, mlt_words, mlt_word_sqr;

void usage() {
	puts("MLT- Mean Length Turn computes the number of utterances, turns");
	puts("     and their ratio.");
	printf("Usage: mlt [a cS gS oN %s] filename(s)\n",mainflgs());
//	puts("+a : count utterances that are not empty or have words specified with +/-s option");
	puts("+a : do not count empty utterances, 0.");
	puts("+cS: look for clause marker S or markers listed in file @S");
	puts("+gS: exclude utterance consisting solely of specified word S or words in file @S");
	puts("+o3: combine selected speakers from each file into one results list for that file");
	mainusage(TRUE);
}

void init(char f) {
	if (f) {
/*		addword('\0','\0',"~[+ trn]"); // to include trn tiers by default */
/*		addword('\0','\0',"~[\\% trn]"); */
/*		addword('\0','\0',"~<\\% trn>"); */
		addword('\0','\0',"+</?>");
		addword('\0','\0',"+</->");
		addword('\0','\0',"+<///>");
		addword('\0','\0',"+<//>");
		addword('\0','\0',"+</>");  // list R6 in mmaininit funtion in cutt.c
		addword('\0','\0',"+<\\% bch>");
//		addword('\0','\0',"+xxx");
//		addword('\0','\0',"+yyy");
//		addword('\0','\0',"+www");
//		addword('\0','\0',"+xxx@s*");
//		addword('\0','\0',"+yyy@s*");
//		addword('\0','\0',"+www@s*");
//		addword('\0','\0',"+unk|xxx");
//		addword('\0','\0',"+unk|yyy");
//		addword('\0','\0',"+*|www");
		addword('\0','\0',"+$*");
		maininitwords();
		mor_initwords();
		ml_isSkip = FALSE;
		ml_isXXXFound = TRUE;
		ml_isYYYFound = TRUE;
		ml_isWWWFound = TRUE;
		ml_isPlusSUsed = FALSE;
		ml_WdMode = '\0';
		ml_WdHead = NULL;
		mlt_ftime = TRUE;
		mlt_foutput = TRUE;
		mlt_isCombineSpeakers = FALSE;
		isFromCall = FALSE;
		count_empty_utterences = TRUE;
	} else {
		if (onlydata == 1) {
			combinput = TRUE;
			OverWriteFile = TRUE;
			AddCEXExtension = ".xls";
		}
	}
	if (!combinput || f) {
		mlt_head = NULL;
	}
	if ((onlydata == 1 || onlydata == 2) && mlt_ftime && chatmode) {
		maketierchoice("@ID:",'+',FALSE);
/*
		if (IDField == NULL && *headtier->tcode == '@') {
			fputs("\nPlease use \"+t\" option to specify speaker code or ID\n",stderr);
			ml_exit(0);
		}
*/
		mlt_ftime = FALSE;
		if (onlydata == 1 && !f_override)
			stout = FALSE;
	}
	ml_spchanged = TRUE;
	if (Toldsp == NULL) {
		Toldsp = (char *)malloc(SPEAKERLEN+1);
		*Toldsp = EOS;
	}
}

static void mlt_outputIDInfo(char *fname, char *s) {
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


static void mlt_pr_result(void) {
	char *sFName;
	float tt;
	float mlt_turn;
	MLTSP *ts;

	if (onlydata == 1) {
		excelHeader(fpout, newfname, 95);
		excelRow(fpout, ExcelRowStart);
		excelCommasStrCell(fpout,"File,Language,Corpus,Code,Age,Sex,Group,Race,SES,Role,Education,Custom_field,");
		if (!ml_isPlusSUsed) {
			sprintf(spareTier1,"#%s,#turns,#words,words/turns,%s/turns,words/%s,Standard deviation",
					(ml_isclause() ? "clauses" : "utterances"),(ml_isclause() ? "clauses" : "utterances"),(ml_isclause() ? "clauses" : "utterances"));
		} else {
			sprintf(spareTier1,"#%s,#turns,#specified words,words/turns,%s/turns,specified words/%s,Standard deviation",
					(ml_isclause() ? "clauses" : "utterances"),(ml_isclause() ? "clauses" : "utterances"),(ml_isclause() ? "clauses" : "utterances"));
		}
		excelCommasStrCell(fpout, spareTier1);
		excelRow(fpout, ExcelRowEnd);
	}
	if (onlydata == 1 && mlt_head == NULL && isFromCall) {
		excelRow(fpout, ExcelRowStart);
		sFName = strrchr(oldfname, PATHDELIMCHR);
		if (sFName == NULL)
			sFName = oldfname;
		else
			sFName++;
		excelStrCell(fpout, sFName);
		excelCommasStrCell(fpout,".,.,.,.,.,.,.,.,.,.,0,0,0,.,.,.");
		excelRow(fpout, ExcelRowEnd);
	}
	while (mlt_head) {
		mlt_words = mlt_head->mlt_words;
		mlt_utter = mlt_head->mlt_utter;
		mlt_turn  = mlt_head->mlt_turn;
		mlt_word_sqr = mlt_head->mlt_word_sqr;
		if (!onlydata || !chatmode) {
			fprintf(fpout, "MLT for Speaker: %s\n", mlt_head->sp);
			if (!ml_isPlusSUsed) {
				if (ml_isXXXFound && ml_isYYYFound && ml_isWWWFound) {
					fprintf(fpout,"  MLT (xxx, yyy and www are EXCLUDED from the word counts, but are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isXXXFound && ml_isYYYFound) {
					fprintf(fpout,"  MLT (xxx and yyy are EXCLUDED from the word counts, but are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (www is EXCLUDED from the %s and word counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isXXXFound && ml_isWWWFound) {
					fprintf(fpout,"  MLT (xxx and www are EXCLUDED from the word counts, but are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (yyy is EXCLUDED from the %s and word counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isWWWFound && ml_isYYYFound) {
					fprintf(fpout,"  MLT (www and yyy are EXCLUDED from the word counts, but are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (xxx is EXCLUDED from the %s and word counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isXXXFound) {
					fprintf(fpout,"  MLT (xxx is EXCLUDED from the word counts, but is INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (yyy and www are EXCLUDED from the %s and word counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isYYYFound) {
					fprintf(fpout,"  MLT (yyy is EXCLUDED from the word counts, but is INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (xxx and www are EXCLUDED from the %s and word counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isWWWFound) {
					fprintf(fpout,"  MLT (www is EXCLUDED from the word counts, but is INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (xxx and yyy are EXCLUDED from the %s and word counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else {
					fprintf(fpout,"  MLT (xxx, yyy and www are EXCLUDED from the %s and word counts):\n", (ml_isclause() ? "clause" : "utterance"));
				}
				fprintf(fpout, "    Number of: %s = %.0f, turns = %.0f, words = %.0f\n", (ml_isclause() ? "clauses" : "utterances"), mlt_utter, mlt_turn, mlt_words);
				if (mlt_turn > (float)0.0) {
					fprintf(fpout, "\tRatio of words over turns = %.3f\n",mlt_words/mlt_turn);
					fprintf(fpout, "\tRatio of %s over turns = %.3f\n", (ml_isclause() ? "clauses" : "utterances"), mlt_utter/mlt_turn);
				}

				if (mlt_utter > (float)0.0) {
					fprintf(fpout, "\tRatio of words over %s = %.3f\n", (ml_isclause() ? "clauses" : "utterances"), mlt_words/mlt_utter);
					tt = (mlt_word_sqr / mlt_utter) - ((mlt_words / mlt_utter) * (mlt_words / mlt_utter));
					if (mlt_utter > (float)1.0 && tt >= (float)0.0) {
						fprintf(fpout,"\tStandard deviation = %.3f\n", sqrt(tt));
					} else
						fprintf(fpout,"\tStandard deviation = %s\n", "NA");
				}

			} else {
				if (ml_isXXXFound && ml_isYYYFound && ml_isWWWFound) {
					fprintf(fpout,"  MLT (xxx, yyy and www are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isXXXFound && ml_isYYYFound) {
					fprintf(fpout,"  MLT (xxx and yyy are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (www is EXCLUDED from the %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isXXXFound && ml_isWWWFound) {
					fprintf(fpout,"  MLT (xxx and www are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (yyy is EXCLUDED from the %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isWWWFound && ml_isYYYFound) {
					fprintf(fpout,"  MLT (www and yyy are INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (xxx is EXCLUDED from the %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isXXXFound) {
					fprintf(fpout,"  MLT (xxx is INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (yyy and www are EXCLUDED from the %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isYYYFound) {
					fprintf(fpout,"  MLT (yyy is INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (xxx and www are EXCLUDED from the %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else if (ml_isWWWFound) {
					fprintf(fpout,"  MLT (www is INCLUDED in %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
					fprintf(fpout,"  MLT (xxx and yyy are EXCLUDED from the %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				} else {
					fprintf(fpout,"  MLT (xxx, yyy and www are EXCLUDED from the %s counts):\n", (ml_isclause() ? "clause" : "utterance"));
				}			

				fprintf(fpout, "    Number of: %s = %.0f, turns = %.0f, words = %.0f\n", (ml_isclause() ? "clauses" : "utterances"), mlt_utter, mlt_turn, mlt_words);
				if (mlt_turn > (float)0.0) {
					fprintf(fpout, "\tRatio of specified words over turns = %.3f\n",mlt_words/mlt_turn);
					fprintf(fpout, "\tRatio of %s over turns = %.3f\n", (ml_isclause() ? "clauses" : "utterances"), mlt_utter/mlt_turn);
				}
				if (mlt_utter > (float)0.0) {
					fprintf(fpout, "\tRatio of specified words over %s = %.3f\n", (ml_isclause() ? "clauses" : "utterances"), mlt_words/mlt_utter);
					tt = (mlt_word_sqr / mlt_utter) - ((mlt_words / mlt_utter) * (mlt_words / mlt_utter));
					if (mlt_utter > (float)1.0 && tt >= (float)0.0) {
						fprintf(fpout,"\tStandard deviation = %.3f\n", sqrt(tt));
					} else
						fprintf(fpout,"\tStandard deviation = %s\n", "NA");
				}
			}
			putc('\n',fpout);
		} else if (onlydata == 1) {
			if (mlt_head->isSpeakerFound) {
				excelRow(fpout, ExcelRowStart);
				sFName = strrchr(mlt_head->fname, PATHDELIMCHR);
				if (sFName != NULL)
					sFName = sFName + 1;
				else
					sFName = mlt_head->fname;
				excelStrCell(fpout, sFName);
				if (mlt_head->ID) {
					excelOutputID(fpout, mlt_head->ID);
				} else {
					excelCommasStrCell(fpout, ".,.");
					excelStrCell(fpout, mlt_head->sp);
					excelCommasStrCell(fpout, ",.,.,.,.,.,.,.");
				}
				excelNumCell(fpout, "%.0f", mlt_utter);
				excelNumCell(fpout, "%.0f", mlt_turn);
				excelNumCell(fpout, "%.0f", mlt_words);
				if (mlt_turn > (float)0.0) {
					excelNumCell(fpout, "%.3f", mlt_words/mlt_turn);
					excelNumCell(fpout, "%.3f", mlt_utter/mlt_turn);
				} else {
					excelStrCell(fpout, "NA");
					excelStrCell(fpout, "NA");
				}
				if (mlt_utter > (float)0.0) {
					excelNumCell(fpout, "%.3f", mlt_words/mlt_utter);
					tt = (mlt_word_sqr / mlt_utter) - ((mlt_words/mlt_utter) * (mlt_words/mlt_utter));
					if (mlt_utter > (float)1.0 && tt >= (float)0.0) {
						excelNumCell(fpout, "%.3f", sqrt(tt));
					} else
						excelStrCell(fpout, "NA");
				} else {
					excelStrCell(fpout, "NA");
					excelStrCell(fpout, "NA");
				}
				excelRow(fpout, ExcelRowEnd);
			}
		} else {
			if (mlt_head->ID) {
				mlt_outputIDInfo(mlt_head->fname, mlt_head->ID);
			} else
				fprintf(fpout,"%s\t.\t.\t%s\t.\t.\t.\t.\t.\t.\t.\t", mlt_head->fname, mlt_head->sp);
			fprintf(fpout,"%5.0f\t%5.0f\t%5.0f", mlt_utter, mlt_turn, mlt_words);
			if (mlt_turn > (float)0.0) {
				fprintf(fpout,"\t%7.3f\t%7.3f", mlt_words/mlt_turn, mlt_utter/mlt_turn);
			} else {
				fprintf(fpout,"\t.\t.");
			}
			if (mlt_utter > (float)0.0) {
				fprintf(fpout,"\t%7.3f",mlt_words/mlt_utter);
				tt = (mlt_word_sqr / mlt_utter) - ((mlt_words/mlt_utter) * (mlt_words/mlt_utter));
				if (mlt_utter > (float)1.0 && tt >= (float)0.0) {
					fprintf(fpout,"\t%7.3f", sqrt(tt));
				} else
					fprintf(fpout,"\tNA");
			} else {
				fprintf(fpout,"\tNA\tNA");
			}
			putc('\n',fpout);
		}
		ts = mlt_head;
		mlt_head = mlt_head->next_sp;
		if (ts->fname != NULL)
			free(ts->fname);
		if (ts->sp != NULL)
			free(ts->sp);
		if (ts->ID)
			free(ts->ID);
		free(ts);
	}
	if (onlydata == 1)
		excelFooter(fpout);
	mlt_head = NULL;
}

static MLTSP *mlt_FindSpeaker(char *fname, char *sp, char *ID, char isSpeakerFound) {
	MLTSP *ts, *tsp;

	uS.remblanks(sp);
	for (ts=mlt_head; ts != NULL; ts=ts->next_sp) {
		if (uS.mStricmp(ts->fname, fname) == 0 || (combinput && onlydata != 1)) {
			if (uS.partcmp(ts->sp, sp, FALSE, FALSE)) {
				ts->isSpeakerFound = isSpeakerFound;
				return(ts);
			}
		}
	}
	if ((ts=NEW(MLTSP)) == NULL)
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
	ts->isSpeakerFound = isSpeakerFound;
	ts->isPSDFound = FALSE;
	ts->mlt_words = (float)0.0;
	ts->mlt_utter = (float)0.0;
	ts->mlt_turn  = (float)0.0;
	ts->mlt_word_sqr = (float)0.0;
	ts->next_sp = NULL;
	if (mlt_head == NULL) {
		mlt_head = ts;
	} else {
		for (tsp=mlt_head; tsp->next_sp != NULL; tsp=tsp->next_sp) ;
		tsp->next_sp = ts;
	}
	return(ts);
}

void call() {
	register int pos;
	MLTSP *ts = NULL;
	char isWordsFound, sq, isSkip, addUttCnt;
	char isPSDFound, curPSDFound;
	float empty_utter, not_empty_utter;
	float tLocalword = 0.0, localword = 0.0;

	ts = NULL;
	isSkip = FALSE;
	not_empty_utter = 0.0;
	empty_utter = 0.0;
	isFromCall = TRUE;
	isPSDFound = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (ml_getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if ((onlydata == 1 || onlydata == 2) && chatmode) {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
				if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
					uS.remblanks(utterance->line);
					mlt_FindSpeaker(oldfname, templineC, utterance->line, FALSE);
				}
				continue;
			}
		}
		if (*utterance->speaker == '*') {
			if (nomain && ts != NULL) {
				if (not_empty_utter > 0.0)
					ts->mlt_utter = ts->mlt_utter + not_empty_utter;
				if (empty_utter > 0.0 && count_empty_utterences && !ml_isclause())
					ts->mlt_utter = ts->mlt_utter + empty_utter;
			}
			if (mlt_isCombineSpeakers) {
				strcpy(templineC, "*COMBINED*");
			} else {
				strcpy(templineC, utterance->speaker);
//				ml_checktier(templineC);
			}
			ts = mlt_FindSpeaker(oldfname, templineC, NULL, TRUE);
			mlt_words = ts->mlt_words;
			mlt_utter = ts->mlt_utter;
			mlt_word_sqr = ts->mlt_word_sqr;
			tLocalword = localword;
			isPSDFound = ts->isPSDFound;
		}
		if (nomain && *utterance->speaker == '*') {
			isSkip = FALSE;
			for (pos=0; utterance->line[pos] != EOS; pos++) {
				if ((pos == 0 || uS.isskip(utterance->line,pos-1,&dFnt,MBF)) && utterance->line[pos] == '+' && 
					uS.isRightChar(utterance->line,pos+1,',',&dFnt, MBF) && isPSDFound) {
					if (mlt_utter > 0.0) 
						mlt_utter--;
					isPSDFound = FALSE;
				}
			}
			ts->mlt_utter = mlt_utter;
			for (pos=0; utterance->line[pos] != EOS; pos++) {
				if (mlt_excludeUtter(uttline, pos, &isWordsFound)) { // xxx, yyy, www
					not_empty_utter = 0.0;
					empty_utter = 0.0;
					localword = tLocalword;
					isSkip = TRUE;
					break;
				}
				if (ml_UtterClause(utterance->line, pos)) {
					if (uS.isRightChar(utterance->line, pos, '[', &dFnt, MBF)) {
						while (utterance->line[pos] && !uS.isRightChar(utterance->line, pos, ']', &dFnt, MBF)) pos++;
					}
					if (uS.isRightChar(utterance->line, pos, ']', &dFnt, MBF))
						sq = FALSE;
					else if (utterance->line[pos] == EOS)
						pos--;
					if (isWordsFound)
						not_empty_utter = not_empty_utter + (float)1.0;
					else
						empty_utter = empty_utter + (float)1.0;
				}
			}
			continue;
		} else if (!nomain)
			isSkip = FALSE;
//printf("%s%s", utterance->speaker, utterance->line);
		not_empty_utter = 0.0;
		empty_utter = 0.0;
		curPSDFound = FALSE;
		isWordsFound = FALSE;
		pos = 0;
		sq = FALSE;
		if (count_empty_utterences && !ml_isclause())
			addUttCnt = TRUE;
		else
			addUttCnt = FALSE;
		do {
			if (mlt_excludeUtter(uttline, pos, &isWordsFound)) { // xxx, yyy, www
				isSkip = TRUE;
				localword = tLocalword;
				break;
			}
			if (uS.isRightChar(uttline, pos, '[', &dFnt, MBF)) {
				sq = TRUE;
			} else if (uS.isRightChar(uttline, pos, ']', &dFnt, MBF))
				sq = FALSE;
			if (!uS.isskip(uttline,pos,&dFnt,MBF) && !sq) {
				isWordsFound = TRUE;
				mlt_words = mlt_words + 1;
				localword = localword + 1;
				while (uttline[pos]) {
					if (uS.isskip(uttline,pos,&dFnt,MBF)) {
						if (uS.IsUtteranceDel(utterance->line, pos)) {
							if (!uS.atUFound(utterance->line, pos, &dFnt, MBF))
								break;
						} else
							break;
					}
					pos++;
				}
			}
			if ((pos == 0 || uS.isskip(utterance->line,pos-1,&dFnt,MBF)) && utterance->line[pos] == '+' && 
				uS.isRightChar(utterance->line,pos+1,',',&dFnt, MBF) && isPSDFound) {
				if (mlt_utter > 0.0) 
					mlt_utter--;
				else if (mlt_utter == 0.0) 
					isWordsFound = FALSE;
				addUttCnt = FALSE;
				isPSDFound = FALSE;
			}
			if (isTierContSymbol(utterance->line, pos, TRUE)) {
				if (mlt_utter > 0.0) 
					mlt_utter--;
				else if (mlt_utter == 0.0) 
					isWordsFound = FALSE;
				addUttCnt = FALSE;
			}
			if (isTierContSymbol(utterance->line, pos, FALSE))
				curPSDFound = TRUE;
			if (ml_UtterClause(utterance->line, pos)) {
				if (uS.isRightChar(utterance->line, pos, '[', &dFnt, MBF)) {
					while (utterance->line[pos] && !uS.isRightChar(utterance->line, pos, ']', &dFnt, MBF)) pos++;
				}
				if (uS.isRightChar(utterance->line, pos, ']', &dFnt, MBF))
					sq = FALSE;
				if (isWordsFound) {
					if (localword != (float)0.0) {
						mlt_word_sqr = mlt_word_sqr + localword * localword;
						localword = (float)0.0;
					}
					mlt_utter = mlt_utter + (float)1.0;
					isWordsFound = FALSE;
				} else
					empty_utter = empty_utter + (float)1.0;
				while (uS.IsCharUtteranceDel(utterance->line, pos+1))
					pos++;
			}
			if (uttline[pos])
				pos++;
			else
				break;
		} while (uttline[pos]) ;
		if (!isSkip) {
			if (localword != (float)0.0) {
				mlt_word_sqr = mlt_word_sqr + localword * localword;
				localword = (float)0.0;
			}
			if (ts != NULL) {
				if ((ts->mlt_utter != mlt_utter || (addUttCnt && empty_utter > 0.0)) && ml_spchanged) {
					ts->mlt_turn += 1.0;
					ml_spchanged = FALSE;
				}
				ts->mlt_words = mlt_words;
				ts->mlt_utter = mlt_utter;
				if (addUttCnt)
					ts->mlt_utter = ts->mlt_utter + empty_utter;
				ts->mlt_word_sqr = mlt_word_sqr;
				ts->isPSDFound = curPSDFound;
			}
		}
	}
	if (!combinput)
		mlt_pr_result();
	isFromCall = FALSE;
}

void getflag(char *f, char *f1, int *i) {
	int j;
	char *t;

	f++;
	switch(*f++) {
		case 'a':
			count_empty_utterences = FALSE;
			no_arg_option(f);
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
/*			if (*(f-2) == '+') ml_mkwdlist('g','i',getfarg(f,f1,i));
			else ml_mkwdlist('g','e',getfarg(f,f1,i));
*/
			break;
		case 'o':
			if (*f == '3') {
				mlt_isCombineSpeakers = TRUE;
			} else {
				fprintf(stderr,"Invalid argument for option: %s\n", f-2);
				cutt_exit(0);
			}
			break;
		case 't':
			if (*(f-2) == '+' && *f == '%') {
				nomain = TRUE;
			} else if (*(f-2) == '+' && *f == '*' && *(f+1) == EOS) {
				mlt_isCombineSpeakers = TRUE;
			}
			if (*(f-2) == '-' && *f == '%') {
			} else
				maingetflag(f-2,f1,i);
			break;
		case 's':
			t = getfarg(f,f1,i);
			j = strlen(t) - 3;
			if (*(f-2) == '-') {
				if (uS.mStricmp(t+j, "xxx") == 0) {
					ml_isXXXFound = FALSE;
					break;
				} else if (uS.mStricmp(t+j, "yyy") == 0) {
					ml_isYYYFound = FALSE;
					break;
				} else if (uS.mStricmp(t+j, "www") == 0) {
					ml_isWWWFound = FALSE;
					break;
				} else {
/*
					fprintf(stderr,"Invalid option: %s\n", f-2);
					fprintf(stderr,"Only \"-sxxx\" and \"-syyy\" and \"-swww\" are allowed\n");
					ml_exit(0);
					break;
*/
				}
			} else if (*(f-2) == '+') {
				if (uS.mStricmp(t+j, "xxx") == 0 || uS.mStricmp(t+j, "yyy") == 0 || uS.mStricmp(t+j, "www") == 0) {
					fprintf(stderr,"Utterances containing \"%s\" are included by default.\n", f);
					fprintf(stderr,"To exclude utterances with xxx, yyy or www please use \"-s\" option.\n");
					ml_exit(0);
					break;
				}
				if (strncmp(t, "[- ", 3) && strncmp(t, "[+ ", 3))
					count_empty_utterences = FALSE;
				ml_isPlusSUsed = TRUE;
			}
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = MLT;
	chatmode = CHAT_MODE;
	OnlydataLimit = 2;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,mlt_pr_result);
	ml_WdHead = freeIEWORDS(ml_WdHead);
}
