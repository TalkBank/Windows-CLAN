/**********************************************************************
 "Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
*/

// CONLL: word_num	word	stem	POS 	GRA_LINK_NUM	GRA 	?	more_MOR

#define CHAT_MODE 1
#include "cu.h"
#include "check.h"
#include "c_curses.h"

#if !defined(UNX)
#define _main Chat2Conll_main
#define call Chat2Conll_call
#define getflag Chat2Conll_getflag
#define init Chat2Conll_init
#define usage Chat2Conll_usage
#endif

#include "mul.h"
#define IS_WIN_MODE FALSE

#define CH2CNL_UTT struct chat2conll_utts
CH2CNL_UTT {
    char speaker[SPEAKERLEN];
    char line[UTTLINELEN+1];
    CH2CNL_UTT *nextutt;
} ;

extern char OverWriteFile;
extern char isRemoveCAChar[];

static char ftime, isFloSpecified, isMorSpecified, isGraSpecified;
static char coding, puncS[6];

void usage() {
	printf("Usage: chat2conll [cS %s] filename(s)\n",mainflgs());
	puts("+cS: use special coding system (default: Depparse - MaltParser, TurboParser)");
	puts("    +cDe(pparse), +cMa(ltParser), +cTu(rboParser)");
	puts("    +cAn(CoraCorpus), +cCo(nnexor), +cCl(earparser)");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(FALSE);
	puts("\nExample for Connexor format:");
	puts("    chat2conll +cco filename");
	puts("For training Depparse, MaltParser or TurboParser:");
	puts("    chat2conll +t%mor +t%grt filename");
	puts("For analysis by Depparse, MaltParser or TurboParser:");
	puts("    chat2conll +t%mor filename");
	cutt_exit(0);
}

void init(char s) {
	int i;

	if (s) {
		ftime = TRUE;
		coding = 0;
		strcpy(puncS, "Punc");
		stout = FALSE;
		onlydata = 3;
		OverWriteFile = TRUE;
		AddCEXExtension = ".txt";
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+0");
		addword('\0','\0',"+&*");
		addword('\0','\0',"+-*");
		addword('\0','\0',"+#*");
		addword('\0','\0',"+(*.*)");
		isFloSpecified = FALSE;
		isMorSpecified = FALSE;
		isGraSpecified = FALSE;
	} else {
		if (coding == 0) {
			fprintf(stderr, "Please specify parser name with +c option.\n");
			fprintf(stderr, "Please choose one of:\n");
			fprintf(stderr, "    +cDe(pparse)\n");
			fprintf(stderr, "    +cMa(ltParser)\n");
			fprintf(stderr, "    +cTu(rboParser)\n");
			fprintf(stderr, "    +cAn(CoraCorpus)\n");
			fprintf(stderr, "    +cCo(nnexor)\n");
			fprintf(stderr, "    +cCl(earparser)\n");
			cutt_exit(0);
		}
		if (ftime) {
			ftime = FALSE;
			if (coding == 3) {
				if (!isFloSpecified)
					maketierchoice("%flo:",'+','\001');
				isFloSpecified = TRUE;
			} else {
				addword('\0','\0',"+</?>");
				addword('\0','\0',"+</->");
				addword('\0','\0',"+<///>");
				addword('\0','\0',"+<//>");
				addword('\0','\0',"+</>");
			}
//2019-04-29			isRemoveCAChar[NOTCA_OPEN_QUOTE] = FALSE;
//2019-04-29			isRemoveCAChar[NOTCA_CLOSE_QUOTE] = FALSE;
			isRemoveCAChar[NOTCA_VOCATIVE] = FALSE;
			isRemoveCAChar[NOTCA_DOUBLE_COMMA] = FALSE;
//			if (isMorSpecified == FALSE && !isFloSpecified)
//				maketierchoice("%mor:",'+','\001');
			for (i=0; GlobalPunctuation[i]; ) {
				if (GlobalPunctuation[i] == '!' ||
					GlobalPunctuation[i] == '?' ||
					GlobalPunctuation[i] == '.')
					strcpy(GlobalPunctuation+i,GlobalPunctuation+i+1);
				else
					i++;
			}
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CHAT2CONLL;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'c':
			if ((*f == 'd' || *f == 'D') && (*(f+1) == 'e' || *(f+1) == 'E')) {
				coding = 1; // Depparse
			} else if ((*f == 'm' || *f == 'M') && (*(f+1) == 'a' || *(f+1) == 'A')) {
				coding = 1; // MaltParser
			} else if ((*f == 't' || *f == 'T') && (*(f+1) == 'u' || *(f+1) == 'U')) {
				coding = 1; // TurboParser
			} else if ((*f == 'a' || *f == 'A') && (*(f+1) == 'n' || *(f+1) == 'N')) {
				coding = 2; // AnCoraCorpus
			} else if ((*f == 'c' || *f == 'C') && (*(f+1) == 'o' || *(f+1) == 'O')) {
				coding = 3; // Connexor
			} else if ((*f == 'c' || *f == 'C') && (*(f+1) == 'l' || *(f+1) == 'L')) {
				coding = 4; // Clearparser
			} else {
				fprintf(stderr, "Please choose one of:\n");
				fprintf(stderr, "    +cDe(pparse)\n");
				fprintf(stderr, "    +cMa(ltParser)\n");
				fprintf(stderr, "    +cTu(rboParser)\n");
				fprintf(stderr, "    +cAn(CoraCorpus)\n");
				fprintf(stderr, "    +cCo(nnexor)\n");
				fprintf(stderr, "    +cCl(earparser)\n");
				cutt_exit(0);
			}
			break;
#ifdef UNX
		case 'L':
			strcpy(lib_dir, f);
			len = strlen(lib_dir);
			if (len > 0 && lib_dir[len-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		case 's':
			if (*(f-2) != '-' || (*f != '<' && *f != '[')) {
				fprintf(stderr,"Invalid option: %s\n", f-2);
				cutt_exit(0);
			} else
				maingetflag(f-2,f1,i);
			break;
		case 't':
			if (!uS.mStricmp(f, "%mor") || !uS.mStricmp(f, "%mor:") ||
				!uS.mStricmp(f, "%trn") || !uS.mStricmp(f, "%trn:")) {
				if (isMorSpecified) {
					fputs("Only one %mor or %trn +t option allowed\n",stderr);
					cutt_exit(0);
				}
				isMorSpecified = TRUE;
			} else if (!uS.mStricmp(f, "%gra") || !uS.mStricmp(f, "%gra:") ||
					   !uS.mStricmp(f, "%grt") || !uS.mStricmp(f, "%grt:")) {
				if (isGraSpecified) {
					fputs("Only one %gra or %grt +t option allowed\n",stderr);
					cutt_exit(0);
				}
				isGraSpecified = TRUE;
			} else if (!uS.mStricmp(f, "%flo")) {
				isFloSpecified = TRUE;
			}
			maingetflag(f-2,f1,i);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static CH2CNL_UTT *freeUtts(CH2CNL_UTT *p) {
	CH2CNL_UTT *t;

	while (p != NULL) {
		t = p;
		p = p->nextutt;
		free(t);
	}
	return(NULL);
}

static int excludeSpMorWords(char *word) {
	if (uS.mStricmp(word, "xx") == 0 || uS.mStricmp(word, "yy") == 0 || word[0] == '[') {
		return(FALSE);
	} else if (word[0] == '+') {
		if (isMorExcludePlus(word))
			return(FALSE);
	}
	return(TRUE);
}

static void removeAngleBrackets(char *line) {
	long i;

	i = 0L;
	while (line[i] != EOS) {
		if ((i == 0 || line[i-1] != '+') && (line[i] == '<' || line[i] == '>')) {
			strcpy(line+i, line+i+1);
		} else
			i++;
	}
}

static CH2CNL_UTT *add2Utts(CH2CNL_UTT *root_utts) {
	CH2CNL_UTT *utt;

	if (root_utts == NULL) {
		utt = NEW(CH2CNL_UTT);
		if (utt == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = freeUtts(root_utts);
			cutt_exit(0);
		}
		root_utts = utt;
	} else {
		for (utt=root_utts; utt->nextutt != NULL; utt=utt->nextutt) ;
		if ((utt->nextutt=NEW(CH2CNL_UTT)) == NULL) {
			fprintf(stderr,"Error: out of memory\n");
			root_utts = freeUtts(root_utts);
			cutt_exit(0);
		}
		utt = utt->nextutt;
	}
	utt->nextutt = NULL;
	strcpy(utt->speaker, utterance->speaker);
//	if (coding == 3)
//		strcpy(utt->line, utterance->line);
//	else
		strcpy(utt->line, utterance->tuttline);
	if (coding == 3)
		removeAngleBrackets(utt->line);
	return(root_utts);
}

static char isEmptyTier(CH2CNL_UTT *utt) {
	int i;
	char word[BUFSIZ];

	i = 0;
	while ((i=getword(utt->speaker, utt->line, word, NULL, i))) {
		if (!uS.IsUtteranceDel(word, 0) && word[0] != '+' && word[0] != '(' && word[0] != '-' &&
			word[0] != '[' && !uS.isToneUnitMarker(word))
			return(FALSE);
	}
	return(TRUE);
}

static char isMorChar(char c) {
	if (c == '#' || c == '|' || c == '-' || c == '&' || c == '=' || c == '~' || c == '$' || c == '+')
		return(TRUE);
	return(FALSE);
}

static char *parseMor(char *item, char *stem, char *mor) {
	int  i, j, oi;
	char *p, *s, isStem;

	mor[0] = EOS;
	stem[0] = EOS;
	p = strchr(item, '|');
	if (p == NULL) {
		if (uS.IsUtteranceDel(item, 0) || item[0] == '+')
			strcpy(stem, item);
		return(puncS);
	}
	i = 0;
	j = 0;
	oi = 0;
	isStem = FALSE;
	while (item[i] != EOS) {
		if (!isStem)
			mor[j] = item[i];
		if (item[i] == '#') {
			isStem = FALSE;
			if (j == 0 || mor[j-1] != '|')
				mor[j++] = '|';
			oi = j;
		} else if (item[i] == '|') {
			isStem = TRUE;
			mor[j] = EOS;
			if (j > oi) {
				strcpy(mor+oi, mor+j);
				j = oi;
			}
		} else if (isMorChar(item[i])) {
			isStem = FALSE;
			if (j == 0 || mor[j-1] != '|') {
				mor[j++] = '|';
				mor[j++] = item[i];
			}
			oi = j;
		} else if (!isStem)
			j++;
		i++;
	}
	for (j--; j >= 0 && (isSpace(mor[j]) || mor[j] == '|'); j--) ;
	j++;
	mor[j] = EOS;
	for (j=0; mor[j] != EOS && mor[j] == '|'; j++) ;
	if (j > 0)
		strcpy(mor, mor+j);
	stem[0] = EOS;
	*p = EOS;
	s = p;
	for (p--; p > item && *p != '#'; p--) ;
	if (*p == '#')
		p++;
	i = 0;
	do {
		if (i > 0)
			stem[i++] = '+';
		for (s++; *s != EOS && !isMorChar(*s); s++) {
			stem[i++] = *s;
		}
		s = strchr(s, '|');
	} while (s != NULL) ;
	stem[i] = EOS;
	return(p);
}

static char *parseGra(char *item, char *pnum) {
	char *g, *p;

	pnum[0] = EOS;
	g = strrchr(item, '|');
	if (g == NULL)
		return(NULL);
	*g = EOS;
	for (p=g-1; p > item && *p != '|'; p--) ;
	if (*p == '|')
		p++;
	strcpy(pnum, p);
	g++;
	return(g);
}

static void createConll(CH2CNL_UTT *root_utts, long ln) {
	int i, j, b, e, cnt;
	int pnum[250];
	char *s, *word[250], *stem[250], *POS[250], *gra[250], *mor[250], isSQ;
	CH2CNL_UTT *utt, *spUtt, *floUtt, *morUtt, *graUtt;

	spUtt = NULL;
	floUtt = NULL;
	morUtt = NULL;
	graUtt = NULL;
	for (utt=root_utts; utt != NULL; utt=utt->nextutt) {
		if (utt->speaker[0] == '*') {
			spUtt = utt;
			filterwords(spUtt->speaker, spUtt->line, excludeSpMorWords);
		} else if (uS.partcmp(utt->speaker, "%mor:", FALSE, FALSE) ||
				   uS.partcmp(utt->speaker, "%trn:", FALSE, FALSE)) {
			morUtt = utt;
		} else if (uS.partcmp(utt->speaker, "%gra:", FALSE, FALSE) ||
				   uS.partcmp(utt->speaker, "%grt:", FALSE, FALSE)) {
			graUtt = utt;
		} else if (uS.partcmp(utt->speaker, "%flo:", FALSE, FALSE)) {
			floUtt = utt;
		}
	}
	if (isEmptyTier(spUtt) && floUtt == NULL)
		return;
	if (floUtt == NULL && coding == 3)
		return;
	for (i=0; i < 250; i++) {
		word[i] = NULL;
		stem[i] = NULL;
		POS[i]  = NULL;
		pnum[i] = -1;
		gra[i]  = NULL;
		mor[i]  = NULL;
	}
	cnt = 0;
	if (floUtt != NULL) {
		isSQ = FALSE;
		for (i=0; floUtt->line[i] != EOS; i++) {
			if (floUtt->line[i] == '[')
				isSQ = TRUE;
			else if (floUtt->line[i] == ']') {
				floUtt->line[i] = ' ';
				isSQ = FALSE;
			}
			if (floUtt->line[i] == '\n' || isSQ)
				floUtt->line[i] = ' ';
		}
		i = 0;
		b = 0;
		e = 0;
		while (floUtt->line[b] != EOS) {
			if (!isSpace(floUtt->line[b])) {
				word[i] = floUtt->line + b;
				for (e=b; !isSpace(floUtt->line[e]) && floUtt->line[e] != EOS; e++) ;
				if (floUtt->line[e] != EOS) {
					floUtt->line[e] = EOS;
					b = e + 1;
				} else
					b = e;
				if (coding == 3) {
					for (j=0; word[i][j] != '@' && word[i][j] != EOS; j++) ;
					if (word[i][j] == '@' && j > 0) {
						while (!uS.isskip(word[i],j,&dFnt,MBF) && word[i][j] != EOS)
							strcpy(word[i]+j, word[i]+j+1);
					}
				}
				i++;
			} else
				b++;
		}
		cnt = i;
//		printout(floUtt->speaker, floUtt->line, NULL, NULL, FALSE);
	} else if (spUtt != NULL) {
		for (i=0; spUtt->line[i] != EOS; i++) {
			if (spUtt->line[i] == '\n')
				spUtt->line[i] = ' ';
		}
		if (morUtt != NULL) {
			mor_link.error_found = FALSE;
			createMorUttline(spareTier1,spUtt->line,"%mor:",morUtt->line,FALSE,TRUE);
			if (mor_link.error_found) {
#ifdef UNX
				fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, ln);
				fprintf(stderr, "ERROR: %%MOR: TIER DOES NOT LINK IN SIZE TO ITS SPEAKER TIER.\n");
				fprintf(stderr, "QUITTING.\n\n");
#else
				fprintf(stderr,"\n%c%c*** File \"%s\": line %ld.%c%c\n", ATTMARKER, error_start, oldfname, ln, ATTMARKER, error_end);
				fprintf(stderr, "%c%cERROR: %%MOR: TIER DOES NOT LINK IN SIZE TO ITS SPEAKER TIER.%c%c\n", ATTMARKER, error_start, ATTMARKER, error_end);
				fprintf(stderr, "%c%cQUITTING.%c%c\n\n", ATTMARKER, error_start, ATTMARKER, error_end);
#endif
				cutt_exit(0);
			} else {
				processSPTier(spUtt->line, spareTier1);
			}
		}
		i = 0;
		b = 0;
		e = 0;
		while (spUtt->line[b] != EOS) {
			if (!isSpace(spUtt->line[b])) {
				word[i++] = spUtt->line + b;
				for (e=b; !isSpace(spUtt->line[e]) && spUtt->line[e] != EOS; e++) ;
				if (spUtt->line[e] != EOS) {
					spUtt->line[e] = EOS;
					b = e + 1;
				} else
					b = e;
			} else
				b++;
		}
		cnt = i;
//		printout(spUtt->speaker, spUtt->line, NULL, NULL, FALSE);
	}
	if (morUtt != NULL && isMorSpecified) {
		for (i=0; morUtt->line[i] != EOS; i++) {
			if (morUtt->line[i] == '~' || morUtt->line[i] == '$' || morUtt->line[i] == '\n')
				morUtt->line[i] = ' ';
		}
		i = 0;
		b = 0;
		e = 0;
		while (morUtt->line[b] != EOS) {
			if (!isSpace(morUtt->line[b])) {
				s = morUtt->line + b;
				for (e=b; !isSpace(morUtt->line[e]) && morUtt->line[e] != EOS; e++) ;
				if (morUtt->line[e] != EOS) {
					morUtt->line[e] = EOS;
					b = e + 1;
				} else
					b = e;
				POS[i] = parseMor(s, templineC1, templineC2);
				if (strlen(templineC1) > 0) {
					stem[i] = (char *)malloc(strlen(templineC1)+1);
					if (stem[i] == NULL) {
						fprintf(stderr,"ERROR: Out of memory.\n");
						for (i=0; i < 250; i++) {
							if (stem[i] != NULL)
								free(stem[i]);
							if (mor[i] != NULL)
								free(mor[i]);
						}
						cutt_exit(0);
					}
					strcpy(stem[i], templineC1);
				}
				if (strlen(templineC2) > 0) {
					mor[i] = (char *)malloc(strlen(templineC2)+1);
					if (mor[i] == NULL) {
						fprintf(stderr,"ERROR: Out of memory.\n");
						for (i=0; i < 250; i++) {
							if (stem[i] != NULL)
								free(stem[i]);
							if (mor[i] != NULL)
								free(mor[i]);
						}
						cutt_exit(0);
					}
					strcpy(mor[i], templineC2);
				}
				i++;
			} else
				b++;
		}
		if (cnt != i) {
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr,"number of speaker words does not match number of words on %s tier.\n", morUtt->speaker);
		}
//		printout(morUtt->speaker, morUtt->line, NULL, NULL, FALSE);
	}
	if (graUtt != NULL && isGraSpecified) {
		for (i=0; graUtt->line[i] != EOS; i++) {
			if (graUtt->line[i] == '~' || graUtt->line[i] == '$' || graUtt->line[i] == '\n')
				graUtt->line[i] = ' ';
		}
		i = 0;
		b = 0;
		e = 0;
		while (graUtt->line[b] != EOS) {
			if (!isSpace(graUtt->line[b])) {
				s = graUtt->line + b;
				for (e=b; !isSpace(graUtt->line[e]) && graUtt->line[e] != EOS; e++) ;
				if (graUtt->line[e] != EOS) {
					graUtt->line[e] = EOS;
					b = e + 1;
				} else
					b = e;
				gra[i] = parseGra(s, templineC1);
				if (templineC1[0] == EOS || !isdigit(templineC1[0]))
					pnum[i] = -1;
				else
					pnum[i] = atoi(templineC1);
				i++;
			} else
				b++;
		}
		if (cnt != i) {
			fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
			fprintf(stderr,"number of speaker words does not match number of words on %s tier.\n", graUtt->speaker);
		}
//		printout(graUtt->speaker, graUtt->line, NULL, NULL, FALSE);
	}
	for (i=0; i < cnt; i++) {
		if (coding == 3) {
			if (i > 0 && word[i] != NULL && !uS.IsUtteranceDel(word[i], 0))
				fprintf(fpout, " ");
			fprintf(fpout, "%s", word[i]); // FORM
		} else {
			fprintf(fpout, "%d\t", i+1);	// ID
			if (word[i] == NULL)			// FORM
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", word[i]);
			if (stem[i] == NULL)			// LEMMA
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", stem[i]);
		}
		if (coding == 1) { // Depparse - MaltParser, TurboParser
			if (POS[i] == NULL)				// CPOSTAG
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", POS[i]);
			if (POS[i] == NULL)				// POSTAG
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", POS[i]);
			if (mor[i] == NULL)				// FEATS
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", mor[i]);
			if (pnum[i] == -1)				// HEAD
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%d\t", pnum[i]);
			if (gra[i] == NULL)				// DEPREL
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", gra[i]);
			fprintf(fpout, "_\t");			// PHEAD
			fprintf(fpout, "_\t");			// PDEPREL
			fprintf(fpout, "\n");
		} else if (coding == 2) { // AnCoraCorpus
			if (POS[i] == NULL)				// POS
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", POS[i]);
			if (pnum[i] == -1)				// HEAD
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%d\t", pnum[i]);
			if (gra[i] == NULL)				// DEPREL
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", gra[i]);
			fprintf(fpout, "_\t");			// ?
			if (mor[i] == NULL)				// FEATS
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", mor[i]);
			fprintf(fpout, "\n");
		} else if (coding == 3) { // Connexor
/*
			if (gra[i] == NULL)	{			// DEPREL
				fprintf(fpout, "_\t");
			} else if (pnum[i] == -1)				// HEAD
				fprintf(fpout, "%s>0\t", gra[i]);
			else
				fprintf(fpout, "%s>%d\t", gra[i], pnum[i]);
			if (POS[i] == NULL && mor[i] == NULL)
				fprintf(fpout, "_\t");
			else {
				if (POS[i] != NULL)				// POS
					fprintf(fpout, "$%s", POS[i]);
				if (mor[i] != NULL)				// FEATS
					fprintf(fpout, " %s", mor[i]);
			}
			fprintf(fpout, "\n");
*/
		} else if (coding == 4) { // Clearparser
			if (POS[i] == NULL)				// POSTAG
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", POS[i]);
			if (mor[i] == NULL)				// FEATS
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", mor[i]);
			if (pnum[i] == -1)				// HEAD
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%d\t", pnum[i]);
			if (gra[i] == NULL)				// DEPREL
				fprintf(fpout, "_\t");
			else
				fprintf(fpout, "%s\t", gra[i]);
			fprintf(fpout, "\n");
		} else
			fprintf(fpout, "\n");
	}
	fprintf(fpout, "\n");
	for (i=0; i < 250; i++) {
		if (stem[i] != NULL)
			free(stem[i]);
		if (mor[i] != NULL)
			free(mor[i]);
	}
}

void call() {		/* this function is self-explanatory */
	long ln;
	CH2CNL_UTT *root_utts;

	ln = lineno;
	root_utts = NULL;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (utterance->speaker[0] == '*') {
			if (root_utts != NULL) {
				createConll(root_utts, ln);
				root_utts = freeUtts(root_utts);
			}
			ln = lineno;
			root_utts = add2Utts(root_utts);
		} else if (utterance->speaker[0] == '%') {
			root_utts = add2Utts(root_utts);
		} else {
			if (root_utts != NULL) {
				createConll(root_utts, ln);
				root_utts = freeUtts(root_utts);
			}
		}
	}
	if (root_utts != NULL) {
		createConll(root_utts, ln);
		root_utts = freeUtts(root_utts);
	}
}
