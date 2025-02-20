/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"

#if !defined(UNX)
#define _main gem_main
#define call gem_call
#define getflag gem_getflag
#define init gem_init
#define usage gem_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

#define HASHMAX 50

extern char GExt[];
extern char PostCodeMode;
extern struct tier *defheadtier;
extern char isExpendX;

static int  SpecWords;
static char gem_Hash[HASHMAX];
static char BBS[10], CBS[10];
static char allGemsFound;
static char a_option;
static char gem_n_option;
static char group;
static char gem_ftime;
static char gem_includeNested;
static char gem_isTimestamp;
static char onlySelectedBG_EGHeaders;
static char gem_word[BUFSIZ];

void usage() {
    puts("GEM outputs chunks of bracketed data");
    printf("Usage: gem [a e g n %s] filename(s)\n",mainflgs());
    puts("+a : output all tiers, but the Gems (default: output Gem only)");
	puts("+b : output beg bullet value of the first speaker utterance in GEM");
	puts("+e : do not output any nested gems along with matched one");
    puts("+g : marker tier should contain all words specified by +s");
    puts("+n : each Gem is terminated by the next @G");
    mainusage(TRUE);
}

void init(char first) {
	IEWORDS *gemStr;

    if (first) {
		stout = FALSE;
		isExpendX = FALSE;
		allGemsFound = TRUE;
		gem_isTimestamp = FALSE;
    	a_option = FALSE;
		gem_n_option = FALSE;
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
	    addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		maininitwords();
		mor_initwords();
		group = FALSE;
		gem_ftime = TRUE;
		gem_includeNested = TRUE;
		onlySelectedBG_EGHeaders = FALSE;
		SpecWords = 0;
		strcpy(BBS, "@BG:");
		strcpy(CBS, "@EG:");
	} else {
/*
		if (a_option && gem_n_option) {
			fprintf(stderr,"Error: options \"+a\" and \"+n\" can not be used together.\n");
			cutt_exit(0);
		}
*/
		if (gem_ftime) {
			gem_ftime = FALSE;
			maketierchoice(BBS,'+',FALSE);
			if (!gem_n_option)
				maketierchoice(CBS,'+',FALSE);
			if (onlydata == 2) {
				maketierchoice("@ID:",'+',FALSE);
				maketierchoice(MEDIAHEADER,'+',FALSE);
				if (wdptr != NULL && strlen(wdptr->word) < 505) {
					strcpy(GExt, ".");
					strcat(GExt, wdptr->word);
					for (gemStr=wdptr->nextword; gemStr != NULL && strlen(GExt)+1+strlen(gemStr->word) < 505; gemStr=gemStr->nextword) {
						strcat(GExt, ",");
						strcat(GExt, gemStr->word);
					}
					strcat(GExt, ".gem");
				}
			}
			if (onlydata)
				delSkipedFile = "\n ****Warning: No gems found in this file\n";
		}
		if (SpecWords == 0) {
			IEWORDS *tmp;
			for (tmp=wdptr; tmp != NULL; tmp=tmp->nextword)
				SpecWords++;
		}
    }
}

void getflag(char *f, char *f1, int *i) {
    f++;
    switch(*f++) {
		case 'a':
			a_option = TRUE;
			no_arg_option(f);
			break;
		case 'b':
			gem_isTimestamp = TRUE;
			no_arg_option(f);
			break;
		case 'd':
			if (*f == '2') {
				onlySelectedBG_EGHeaders = TRUE;
			} else
				maingetflag(f-2,f1,i);
			break;
		case 'e':
			gem_includeNested = FALSE;
			no_arg_option(f);
			break;
		case 'g':
			group = TRUE;
			no_arg_option(f);
			break;
		case 'n':
			gem_n_option = TRUE;
			strcpy(BBS, "@G:");
			strcpy(CBS, "@*&#");
			no_arg_option(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
    }
}

static int gem_RightText() {
	int i = 0;
	int found = 0, allThereIs = 0;

	if (WordMode == '\0') {
		for (i=0; utterance->line[i]; i++) {
			if (!uS.isskip(utterance->line,i,&dFnt,MBF))
				break;
		}
		if (utterance->line[i] == EOS)
			return(1);
		else
			return(0);
	}
	while ((i=getword(utterance->speaker, uttline, gem_word, NULL, i)))
		found++;
	while ((i=getword(utterance->speaker, utterance->line, gem_word, NULL, i)))
		allThereIs++;
	if (WordMode == 'i' || WordMode == 'I') 
		return((group == FALSE && found) || (SpecWords == found && found == allThereIs));
	else 
		return((group == TRUE && (SpecWords > found || found != allThereIs)) || (group == FALSE && wdptr != NULL && found));
}

static long getBulletBeg(char *line) {
	int  i;
	char mediaRes;
	long Beg, End;

	for (i=0; line[i] != EOS; i++) {
		if (line[i] == HIDEN_C) {
			i++;
			if (isdigit(line[i]))
				mediaRes = getMediaTagInfo(line+i, &Beg, &End);
			else
				mediaRes = getOLDMediaTagInfo(line+i, NULL, NULL, &Beg, &End);
			if (mediaRes)
				return(Beg);
			else
				break;
		}
	}
	return(0L);
}

void call() {
	register int i;
	char *code;
	char isOutputGem, isGemsFound, isRightTextGem;
	int  found = 0, postRes;
	long beg;

	isGemsFound = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	isOutputGem = FALSE;
	for (i=0; i < HASHMAX; i++)
		gem_Hash[i] = FALSE;
	spareTier1[0] = EOS;
	while (getwholeutter()) {
/*
printf("found=%d; sp=%s; uttline=%s", found, utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (onlydata == 2) {
			if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE) ||
				uS.partcmp(utterance->speaker,MEDIAHEADER,FALSE,FALSE)) {
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				continue;
			}
		}
		strcpy(templineC,utterance->speaker);
		code = templineC;
		uS.uppercasestr(code, &dFnt, MBF);
		if (uS.partcmp(code,BBS,FALSE,FALSE)) {
			isRightTextGem = FALSE;
			if (gem_n_option) {
				if (gem_RightText()) {
					isRightTextGem = TRUE;
					if (!onlydata) {
						fprintf(fpout,"*** File \"%s\": ", oldfname);
						if (lineno > 0)
							fprintf(fpout,"line %ld.", lineno+1);
						if (a_option)
							fprintf(fpout," GEM EXCLUDED !");
						putc('\n', fpout);
					} else if (onlydata == 2) {
						fprintf(fpout,"@Comment:\t*** File \"%s\": ",oldfname);
						if (lineno > 0)
							fprintf(fpout,"line %ld.", lineno+1);
						if (a_option)
							fprintf(fpout," GEM EXCLUDED !");
						putc('\n', fpout);
					}
					if (a_option) {
						if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
							changeBullet(utterance->line, utterance->attLine);
						printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					}
					isOutputGem = TRUE;
				} else
					isOutputGem = FALSE;
			} else if (gem_includeNested) {
				if (gem_RightText()) {
					isRightTextGem = TRUE;
					found++;
					if (found == 1 || WordMode != '\0') {
						if (!onlydata) {
							fprintf(fpout,"*** File \"%s\": ", oldfname);
							if (lineno > 0)
								fprintf(fpout,"line %ld.", lineno+1);
							if (a_option)
								fprintf(fpout," GEM EXCLUDED !");
							putc('\n', fpout);
						} else if (onlydata == 2) {
							fprintf(fpout,"@Comment:\t*** File \"%s\": ",oldfname);
							if (lineno > 0)
								fprintf(fpout,"line %ld.", lineno+1);
							if (a_option)
								fprintf(fpout," GEM EXCLUDED !");
							putc('\n', fpout);
						}
						if (a_option) {
							if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
								changeBullet(utterance->line, utterance->attLine);
							printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
						}
						isOutputGem = TRUE;
					}
				}
			} else {
				found++;
				if (gem_RightText()) {
					isRightTextGem = TRUE;
					if (!onlydata) {
						fprintf(fpout,"*** File \"%s\": ", oldfname);
						if (lineno > 0)
							fprintf(fpout,"line %ld.", lineno+1);
						if (a_option)
							fprintf(fpout," GEM EXCLUDED !");
						putc('\n', fpout);
					} else if (onlydata == 2) {
						fprintf(fpout,"@Comment:\t*** File \"%s\": ",oldfname);
						if (lineno > 0)
							fprintf(fpout,"line %ld.", lineno+1);
						if (a_option)
							fprintf(fpout," GEM EXCLUDED !");
						putc('\n', fpout);
					}
					if (a_option) {
						if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
							changeBullet(utterance->line, utterance->attLine);
						printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					}
					isOutputGem = TRUE;
				} else
					isOutputGem = FALSE;
				if (found < HASHMAX) {
					gem_Hash[found] = isOutputGem;
				}
			}
			if ((isOutputGem && !a_option) || (!isOutputGem && a_option)) {
				if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
					changeBullet(utterance->line, utterance->attLine);
				if (!onlySelectedBG_EGHeaders || isRightTextGem) {
					if (gem_isTimestamp) {
						strcpy(spareTier1, utterance->line);
						uS.remblanks(spareTier1);
					} else
					    printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				}
			}
		} else if (found > 0 && uS.partcmp(code,CBS,FALSE,FALSE)) {
			isRightTextGem = FALSE;
			if (gem_n_option) {

			} else if (gem_includeNested) {
			    if (gem_RightText()) {
					isRightTextGem = TRUE;
			    	found--;
					if (found == 0 && !a_option) {
						if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
							changeBullet(utterance->line, utterance->attLine);
						if (!gem_isTimestamp)
							printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					}
					if (found == 0)
						isOutputGem = FALSE;
			    }
				if ((isOutputGem && !a_option) || (!isOutputGem && a_option)) {
					if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
						changeBullet(utterance->line, utterance->attLine);
					if (!onlySelectedBG_EGHeaders || isRightTextGem) {
						if (!gem_isTimestamp)
							printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					}
				}
			} else {
				if (gem_Hash[found] && !a_option) {
					if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
						changeBullet(utterance->line, utterance->attLine);
					if (!gem_isTimestamp)
						printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				}
				gem_Hash[found] = FALSE;
		    	found--;
				if (found == 0)
					isOutputGem = FALSE;
				else if (found < HASHMAX) {
					isOutputGem = gem_Hash[found];
				}
			}
		} else if ((isOutputGem && !a_option) || (!isOutputGem && a_option)) {
			if (onlySelectedBG_EGHeaders && uS.partcmp(code,CBS,FALSE,FALSE))
				isRightTextGem = FALSE;
			else
				isRightTextGem = TRUE;
			isGemsFound = TRUE;
			if (PostCodeMode == '\0') {
				if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
					changeBullet(utterance->line, utterance->attLine);
				if (isRightTextGem) {
					if (gem_isTimestamp) {
						beg = getBulletBeg(utterance->line);
						if (beg != 0 && spareTier1[0] != EOS) {
							if (gem_n_option)
								fprintf(fpout, "@G:\t%s: %ld\n", spareTier1, beg);
							else
								fprintf(fpout, "@Bg:\t%s: %ld\n", spareTier1, beg);
						}
						spareTier1[0] = EOS;
					} else
						printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
				}
			} else {
				postRes = isPostCodeFound(utterance->speaker, utterance->line);
				if (postRes != 5 && postRes != 1) {
					if (cMediaFileName[0] != EOS && (onlydata == 0 || combinput || stout))
						changeBullet(utterance->line, utterance->attLine);
					if (isRightTextGem) {
						if (gem_isTimestamp && spareTier1[0] != EOS) {
							beg = getBulletBeg(utterance->line);
							if (beg != 0 && spareTier1[0] != EOS) {
								if (gem_n_option)
									fprintf(fpout, "@G:\t%s: %ld\n", spareTier1, beg);
								else
									fprintf(fpout, "@Bg:\t%s: %ld\n", spareTier1, beg);
							}
							spareTier1[0] = EOS;
						} else
							printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
					}
				}
			}
		} else if (utterance->speaker[0] == '@' && checktier(utterance->speaker)) {
			if (!onlySelectedBG_EGHeaders || !uS.partcmp(code,CBS,FALSE,FALSE))
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
	if (!isGemsFound && !stout && onlydata) {
		fprintf(stderr, "\n ****Warning: No gems found in this file\n\n");
		if (allGemsFound)
			allGemsFound = FALSE;
		if (!replaceFile && fpout != stdout && !combinput) {
			fclose(fpout);
			fpout = NULL;
			if (unlink(newfname))
				fprintf(stderr, "Can't delete output file \"%s\".", newfname);
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
    isWinMode = IS_WIN_MODE;
    CLAN_PROG_NUM = GEM;
    chatmode = CHAT_MODE;
    OnlydataLimit = 3;
    UttlineEqUtterance = FALSE;
    bmain(argc,argv,NULL);
	if ((!allGemsFound || isFileSkipped) && onlydata) {
		fprintf(stderr, "\n ****Warning: Some files did not have gems.\n");
		fprintf(stderr, "         Please scroll up to see which files are missing gems.\n\n");
	}
}
