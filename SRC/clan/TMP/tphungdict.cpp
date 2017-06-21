#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

#if defined(_MAC_CODE) && defined(PLUGINPROJ)
ClanProgInfo gToolInfo = {
	_main,
	usage,
	getflag,
	init,
	call,
	NULL
};
#endif /* _MAC_CODE */

extern struct tier *defheadtier;

FILE *fpVerb,
	 *fpNoun,
	 *fpAdjective,
	 *fpNumeral,
	 *fpPronoun,
	 *fpAdverb,
	 *fpPrefix,
	 *fpPPosition,
	 *fpConjunct,
	 *fpInterject,
	 *fpOWS,
	 *fpArticle,
	 *fpParticiple,
	 *fpTMorph,
	 *fpOFPS,
	 *fpMFPS,
	 *fpNUII,
	 *fpRUII,
	 *fpUnknown,
	 *fpUnknownType;
/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		stout = TRUE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	}
	fpVerb = NULL;
	fpNoun = NULL;
	fpAdjective = NULL;
	fpNumeral = NULL;
	fpPronoun = NULL;
	fpAdverb = NULL;
	fpPrefix = NULL;
	fpPPosition = NULL;
	fpConjunct = NULL;
	fpInterject = NULL;
	fpOWS = NULL;
	fpArticle = NULL;
	fpParticiple = NULL;
	fpTMorph = NULL;
	fpOFPS = NULL;
	fpMFPS = NULL;
	fpNUII = NULL;
	fpRUII = NULL;
	fpUnknown = NULL;
	fpUnknownType = NULL;
}

void usage() {
	printf("Usage: HungDict [%s] filename(s)\n",mainflgs());
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static FILE *openFileAndWrite(FILE *fp, char *fname, char *line) {
	if (fp == NULL) {
		fp = fopen(fname, "w");
		if (fp == NULL) {
			fprintf(stderr, "Can't open file %s\n", fname);
			cutt_exit(0);
		}
#ifdef _MAC_CODE
		settyp(fname, 'TEXT', the_file_creator.out);
#endif
	}
	fprintf(fp, "%s\n", uttline);
	return(fp);
}

void call() {
	int i;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (lineno % 500 == 0)
			fprintf(stderr,"\r%ld ", lineno);
		i = strlen(uttline) - 1;
		if (i < 0)
			continue;
		if (uttline[i] == '\n')
			uttline[i] = EOS;
		if (uttline[2] == '0' && uttline[3] == '0' && uttline[4] == '0') {
			fpUnknown = openFileAndWrite(fpUnknown, "0unknown.dict", uttline);
		} else {
			for (i=2; i < 5; i++) {
				if (uttline[i] == '1')
					fpVerb = openFileAndWrite(fpVerb, "verb.dict", uttline);
				else if (uttline[i] == '2')
					fpNoun = openFileAndWrite(fpNoun, "noun.dict", uttline);
				else if (uttline[i] == '3')
					fpAdjective = openFileAndWrite(fpAdjective, "adjective.dict", uttline);
				else if (uttline[i] == '4')
					fpNumeral = openFileAndWrite(fpNumeral, "numeral.dict", uttline);
				else if (uttline[i] == '5')
					fpPronoun = openFileAndWrite(fpPronoun, "pronoun.dict", uttline);
				else if (uttline[i] == '6')
					fpAdverb = openFileAndWrite(fpAdverb, "adverb.dict", uttline);
				else if (uttline[i] == '7')
					fpPrefix = openFileAndWrite(fpPrefix, "prefix.dict", uttline);
				else if (uttline[i] == '8')
					fpPPosition = openFileAndWrite(fpPPosition, "postposition.dict", uttline);
				else if (uttline[i] == '9')
					fpConjunct = openFileAndWrite(fpConjunct, "conjunction.dict", uttline);
				else if (uttline[i] == 'A' || uttline[i] == 'a')
					fpInterject = openFileAndWrite(fpInterject, "interjection.dict", uttline);
				else if (uttline[i] == 'B' || uttline[i] == 'b')
					fpOWS = openFileAndWrite(fpOWS, "one_word_sentence.dict", uttline);
				else if (uttline[i] == 'C' || uttline[i] == 'c')
					fpArticle = openFileAndWrite(fpArticle, "article.dict", uttline);
				else if (uttline[i] == 'D' || uttline[i] == 'd')
					fpParticiple = openFileAndWrite(fpParticiple, "participle.dict", uttline);
				else if (uttline[i] == 'K' || uttline[i] == 'k')
					fpTMorph = openFileAndWrite(fpTMorph, "tied_morpheme.dict", uttline);
				else if (uttline[i] == 'M' || uttline[i] == 'm')
					fpOFPS = openFileAndWrite(fpOFPS, "only_first_pers_sing.dict", uttline);
				else if (uttline[i] == 'N' || uttline[i] == 'n')
					fpMFPS = openFileAndWrite(fpMFPS, "mainly_first_pers_sing.dict", uttline);
				else if (uttline[i] == 'P' || uttline[i] == 'p')
					fpNUII = openFileAndWrite(fpNUII, "not_used_in_imperative.dict", uttline);
				else if (uttline[i] == 'Q' || uttline[i] == 'q')
					fpRUII = openFileAndWrite(fpRUII, "rarely_used_in_imperative.dict", uttline);
				else if (uttline[i] != '0' && uttline[i] != ' ' && uttline[i] != '\t')
					fpUnknownType = openFileAndWrite(fpUnknownType, "00Unknown_Type_Charecter.dict", uttline);
			}
		}
	}
	if (fpVerb != NULL)
		fclose(fpVerb);
	if (fpNoun != NULL)
		fclose(fpNoun);
	if (fpAdjective != NULL)
		fclose(fpAdjective);
	if (fpNumeral != NULL)
		fclose(fpNumeral);
	if (fpPronoun != NULL)
		fclose(fpPronoun);
	if (fpAdverb != NULL)
		fclose(fpAdverb);
	if (fpPrefix != NULL)
		fclose(fpPrefix);
	if (fpPPosition != NULL)
		fclose(fpPPosition);
	if (fpConjunct != NULL)
		fclose(fpConjunct);
	if (fpInterject != NULL)
		fclose(fpInterject);
	if (fpOWS != NULL)
		fclose(fpOWS);
	if (fpArticle != NULL)
		fclose(fpArticle);
	if (fpParticiple != NULL)
		fclose(fpParticiple);
	if (fpTMorph != NULL)
		fclose(fpTMorph);
	if (fpOFPS != NULL)
		fclose(fpOFPS);
	if (fpMFPS != NULL)
		fclose(fpMFPS);
	if (fpNUII != NULL)
		fclose(fpNUII);
	if (fpRUII != NULL)
		fclose(fpRUII);
	if (fpUnknown != NULL)
		fclose(fpUnknown);
	if (fpUnknownType != NULL)
		fclose(fpUnknownType);
}
