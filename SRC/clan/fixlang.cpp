/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#define CHAT_MODE 3
#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main fixlang_main
#define call fixlang_call
#define getflag fixlang_getflag
#define init fixlang_init
#define usage fixlang_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;

static char fixlang_ftime;

void init(char f) {
	if (f) {
		fixlang_ftime = 0;
		stout = FALSE;
		OverWriteFile = TRUE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		if (fixlang_ftime == 0) {
			fixlang_ftime = 1;
			ReadLangsFile(FALSE);
		}
	}
}

void usage() {
	printf("    fixlang changes two letter language code to three letter code.\n");
	printf("    It uses file CLAN/lib/fixes/ISO-639.cut file to translate from list of predefined codes.\n");
	printf("    If code is not found in file CLAN/lib/fixes/ISO-639.cut then it is left unchanged.\n");
	printf("    You can add missing language codes by edit \"CLAN/lib/fixes/ISO-639.cut\" text file.\n");
	printf("Usage: fixlang [%s] filename\n", mainflgs());
	mainusage(FALSE);
	puts("\nExample:");
	puts("\tfixlang -re *.cha");
	puts("\tren -re -f *.lan.cex *.cha");
	cutt_exit(0);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = FIXLANG;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	initLanguages();
	bmain(argc,argv,NULL);
	cleanupLanguages();
	if (fixlang_ftime == 2) {
		fprintf(stderr, "\nSome language codes couldn't be converted because of missing code entries.\n");
		fprintf(stderr, "You can add missing language codes by edit \"CLAN/lib/fixes/ISO-639.cut\" text file.\n");
	} else
		fprintf(stderr, "\nSuccess! No errors found.\n");
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}


void call() {
	int i, j, k, s, len;
	char t, check_fav_lang[BUFSIZ+1], word[BUFSIZ+1];

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE) || uS.partcmp(utterance->speaker,"@New Language:",FALSE,FALSE)) {
			for (i=0; uttline[i]; i++)
				utterance->attLine[i] = 0;
			templineC[0] = EOS;
			i = 0;
			while (uttline[i]) {
				for (; uS.isskip(uttline, i, &dFnt, MBF) || uttline[i] == '\n'; i++) ;
				if (uttline[i] == EOS)
					break;
				for (j=i; !uS.isskip(uttline, j, &dFnt, MBF) && uttline[j] != '=' && uttline[j] != '\n' && uttline[j] != EOS; j++) ;
				t = uttline[j];
				uttline[j] = EOS;
				strncpy(check_fav_lang, uttline+i, BUFSIZ);
				check_fav_lang[BUFSIZ] = EOS;
				if (!getLanguageCodeAndName(check_fav_lang, TRUE, NULL)) {
					if (strlen(uttline+i) < 3) {
						fprintf(stderr,"*** File \"%s\": line %ld; Can't convert code \"%s\".\n", oldfname, lineno, uttline+i);
						strcpy(check_fav_lang, uttline+i);
						fixlang_ftime = 2;
					}
				}
				if (templineC[0] != EOS)
					strcat(templineC, ", ");
				strcat(templineC, check_fav_lang);
				uttline[j] = t;
				if (uttline[j] == '=')
					for (; !uS.isskip(uttline, j, &dFnt, MBF) && uttline[j] != '\n' && uttline[j] != EOS; j++) ;
				i = j;
			}
			strcpy(uttline, templineC);
		} else if (uS.partcmp(utterance->speaker,IDOF,FALSE,FALSE)) {
			for (i=0; uttline[i]; i++)
				utterance->attLine[i] = 0;
			s = 0;
			while (uttline[s] != EOS) {
				if (!uS.isskip(uttline, s, &dFnt, MBF))
					break;
				s++;
			}
			if (uttline[s] != EOS && uttline[s] != '|') {
				for (i=0; uttline[s] != EOS && uttline[s] != '|'; i++, s++) {
					word[i] = uttline[s];
				}
				strcpy(templineC1, uttline+s);
				word[i] = EOS;
				templineC[0] = EOS;
				i = 0;
				while (word[i]) {
					for (; uS.isskip(word, i, &dFnt, MBF) || word[i] == '\n'; i++) ;
					if (word[i] == EOS)
						break;
					for (j=i; !uS.isskip(word, j, &dFnt, MBF) && word[j] != '=' && word[j] != '\n' && word[j] != EOS; j++) ;
					t = word[j];
					word[j] = EOS;
					strncpy(check_fav_lang, word+i, BUFSIZ);
					check_fav_lang[BUFSIZ] = EOS;
					if (!getLanguageCodeAndName(check_fav_lang, TRUE, NULL)) {
						if (strlen(word+i) < 3) {
							fprintf(stderr,"*** File \"%s\": line %ld; Can't convert code \"%s\".\n", oldfname, lineno, word+i);
							strcpy(check_fav_lang, word+i);
							fixlang_ftime = 2;
						}
					}
					if (templineC[0] != EOS)
						strcat(templineC, ",");
					strcat(templineC, check_fav_lang);
					word[j] = t;
					if (word[j] == '=')
						for (; !uS.isskip(word, j, &dFnt, MBF) && word[j] != '\n' && word[j] != EOS; j++) ;
					i = j;
				}
				strcpy(uttline, templineC);
				strcat(uttline, templineC1);
			}
		} else if (utterance->speaker[0] == '*') {
			for (i=0; uttline[i] != EOS; ) {
				if (uttline[i] == '[' && uttline[i+1] == '-' && uttline[i+2] == ' ') {
					i = i + 3;
					for (; (uS.isskip(uttline,i,&dFnt,MBF) || uttline[i] == '\n') && uttline[i] != ']' && uttline[s] != '\n' && uttline[i] != EOS; i++) ;
					if (uttline[i] != ']' && uttline[i] != EOS) {
						for (s=i; !uS.isskip(uttline,s,&dFnt,MBF) && uttline[s] != ']' && uttline[s] != '\n' && uttline[s] != EOS; s++) ;
						len = s - i;
						if (s > i) {
							strncpy(check_fav_lang, uttline+i, len);
							check_fav_lang[len] = EOS;
							if (!getLanguageCodeAndName(check_fav_lang, TRUE, NULL)) {
								if (len < 3) {
									strncpy(check_fav_lang, uttline+i, len);
									fprintf(stderr,"*** File \"%s\": line %ld; Can't convert code \"%s\".\n", oldfname, lineno, check_fav_lang);
									fixlang_ftime = 2;
								}
								i++;
							} else {
								len = strlen(check_fav_lang);
								if (len > 0) {
									att_cp(0L, utterance->line+i, utterance->line+s, utterance->attLine+i, utterance->attLine+s);
									att_shiftright(utterance->line+i, utterance->attLine+i, len);
									for (j=0; j < len; j++)
										utterance->line[i++] = check_fav_lang[j];
								} else
									i++;
							}
						} else
							i++;
					} else
						i++;
				} else if (uttline[i] == '@' && uttline[i+1] == 's' && uttline[i+2] == ':') {
					i = i + 3;
					if (!uS.isskip(uttline,i,&dFnt,MBF) && uttline[i] != '\n' && uttline[i] != EOS) {
						for (s=i; !uS.isskip(uttline,s,&dFnt,MBF) && uttline[s] != '\n' && uttline[s] != EOS; s++) ;
						if (s > i) {
							len = s - i;
							strncpy(word, uttline+i, len);
							word[len] = EOS;
							templineC[0] = EOS;
							k = 0;
							while (word[k]) {
								for (; uS.isskip(word, k, &dFnt, MBF) || word[k] == '+' || word[k] == '&' || word[k] == '\n'; k++) ;
								if (word[k] == EOS)
									break;
								for (j=k; !uS.isskip(word, j, &dFnt, MBF) && word[j] != '+' && word[j] != '&' && word[j] != '=' && word[j] != '\n' && word[j] != EOS; j++) ;
								t = word[j];
								word[j] = EOS;
								strncpy(check_fav_lang, word+k, BUFSIZ);
								check_fav_lang[BUFSIZ] = EOS;
								if (!getLanguageCodeAndName(check_fav_lang, TRUE, NULL)) {
									if (strlen(word+k) < 3) {
										fprintf(stderr,"*** File \"%s\": line %ld; Can't convert code \"%s\".\n", oldfname, lineno, word+k);
										strcpy(check_fav_lang, word+k);
										fixlang_ftime = 2;
									}
								}
								word[j] = t;
								strcat(templineC, check_fav_lang);
								if (word[j] == '+' || word[j] == '&') {
									len = strlen(templineC);
									templineC[len++] = word[j];
									templineC[len] = EOS;
								}
								if (word[j] == '=')
									for (; !uS.isskip(word, j, &dFnt, MBF) && word[j] != '+' && word[j] != '&' && word[j] != '\n' && word[j] != EOS; j++) ;
								k = j;
							}
							len = strlen(templineC);
							if (len > 0) {
								att_cp(0L, utterance->line+i, utterance->line+s, utterance->attLine+i, utterance->attLine+s);
								att_shiftright(utterance->line+i, utterance->attLine+i, len);
								for (j=0; j < len; j++)
									utterance->line[i++] = templineC[j];
							} else
								i++;
						} else
							i++;
					} else
						i++;
				} else
					i++;
			}
		}
		printout(utterance->speaker, utterance->line, utterance->attSp, utterance->attLine, FALSE);
	}
}
