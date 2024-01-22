/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "check.h"
#include "mllib.h"
/* // NO QT
#ifdef _WIN32
	#include <TextUtils.h>
#endif
*/
extern char rightspeaker;
extern long FromWU, ToWU;
extern char *Toldsp;
extern char isLanguageExplicit;
extern int  IsModUttDel;
extern struct tier *headtier;

char ml_isSkip;
char ml_isXXXFound = FALSE;
char ml_isYYYFound = FALSE;
char ml_isWWWFound = FALSE;
char ml_isPlusSUsed = FALSE;
char isMLUEpostcode = TRUE;
char ml_WdMode = '\0';	/* 'i' - means to include ml_WdHead words, else exclude	*/
char ml_spchanged;
IEWORDS *ml_WdHead = NULL;		/* contains words to included/excluded					*/

static char isGWordFound;

void ml_exit(int i) {
	ml_WdHead = freeIEWORDS(ml_WdHead);
	cutt_exit(i);
}

void ml_addwd(char opt, char ch, char *wd) {
	register int i;
	char plus = FALSE;
	IEWORDS *tempwd;

	for (i=strlen(wd)-1; wd[i]== ' ' || wd[i]== '\t' || wd[i]== '\n'; i--) ;
	wd[++i] = EOS;
	if (wd[0] == '+') {
		plus = TRUE;
		i = 1;
	} else
		i = 0;
	for (; wd[i] == ' ' || wd[i] == '\t'; i++) ;
	if (!ml_WdMode) {
		if (!plus) {
			if (ml_WdHead != NULL)
				ml_WdHead = freeIEWORDS(ml_WdHead);
		}
		ml_WdMode = ch;
	} else if (ml_WdMode != ch) {
		if (opt == '\0')
			opt = ' ';
		fprintf(stderr, "-%c@ or -%c option can not be used together with either option +%c@ or +%c.\n",opt,opt,opt,opt);
		fprintf(stderr, "Offending word is: %s\n", wd+i);
		ml_exit(0);
	}
	tempwd = NEW(IEWORDS);
	if (tempwd == NULL) {
		fprintf(stderr,"Memory ERROR. Use less include/exclude words.\n");
		ml_exit(1);
	} else {
		tempwd->word = (char *)malloc(strlen(wd+i)+1);
		if(tempwd->word == NULL) {
			fprintf(stderr,"No more space left in core.\n");
			free(tempwd);
			ml_exit(1);
		}
		tempwd->nextword = ml_WdHead;
		ml_WdHead = tempwd;
		if (!nomap)
			uS.lowercasestr(wd+i, &dFnt, C_MBF);
		strcpy(ml_WdHead->word, wd+i);
	}
}

void ml_mkwdlist(char opt, char ch, FNType *fname) {
	FILE *efp;
	char wd[BUFSIZ];

	if (*fname == '+') {
		*wd = '+';
		fname++;
	} else
		*wd = ' ';

	if (*fname == EOS) {
		fprintf(stderr, "No file name for the +g option was specified.\n");
		ml_exit(0);
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(wd_dir);
#endif
	if ((efp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "Can not access %s file: %s\n", ((ch=='i') ? "include" : "exclude"), fname);
		ml_exit(0);
	}
#ifndef UNX
	if (WD_Not_Eq_OD)
		SetNewVol(od_dir);
#endif
	while (fgets_cr(wd+1, 511, efp)) {
		if (uS.isUTF8(wd+1) || uS.isInvisibleHeader(wd+1))
			continue;
		ml_addwd(opt,ch,wd);
	}
	fclose(efp);
}

static int excludewd(char *word) {
	IEWORDS *twd;

	for (twd=ml_WdHead; twd != NULL; twd = twd->nextword) {
		if (uS.patmat(word, twd->word)) {
			if (ml_WdMode == 'i')
				return(TRUE);
			else
				return(FALSE);
		}
	}
	if (ml_WdMode == 'i')
		return(FALSE);
	else
		return(TRUE);
}

/* ml_checktier(s) determines if the "s" tier should be trunketed.
void ml_checktier(char *s) {
	struct tier *p;

	if (!chatmode)
		return;
	for (p=headtier; p != NULL; p=p->nexttier) {
		if (uS.partcmp(s, p->tcode, p->pat_match, FALSE)) {
			if (p->include) {
				if (!strcmp(p->tcode, "*"))
					strcpy(s, "*ALL_SPEAKERS*");
				else
					strcpy(s, p->tcode);
				uS.uppercasestr(s, &dFnt, MBF);
			}
		}
	}
}
*/

static void lfilter(char *org) {
	register int i;
	char *temp, *beg, res, excl, delf;

	isGWordFound = FALSE;
	i = 0;
	beg = org+i;
	excl = TRUE;
	do {
		delf = FALSE;
		for (; uS.isskip(org,i,&dFnt,MBF) && org[i] && org[i] != '['; i++) {
			if (ml_UtterClause(org, i))
				delf = TRUE;
		}
		if (org[i]) {
			temp = org+i;
			if (!nomap) {
				if (MBF) {
					if (my_CharacterByteType(org, (short)i, &dFnt) == 0)
						org[i] = (char)tolower((unsigned char)org[i]);
				} else {
					org[i] = (char)tolower((unsigned char)org[i]);
				}
			}
			if (uS.isRightChar(org, i, '[', &dFnt, MBF)) {
				if (MBF) {
					for (i++; !uS.isRightChar(org, i, ']', &dFnt, MBF) && org[i]; i++) {
						if (!nomap) {
							if (my_CharacterByteType(org, (short)i, &dFnt) == 0)
								org[i] = (char)tolower((unsigned char)org[i]);
						}
					}
				} else {
					for (i++; !uS.isRightChar(org, i, ']', &dFnt, MBF) && org[i]; i++) {
						if (!nomap)
							org[i] = (char)tolower((unsigned char)org[i]);
					}
				}
				if (org[i]) i++;
			} else if (uS.isRightChar(org, i, '+', &dFnt, MBF) || uS.isRightChar(org, i, '-', &dFnt, MBF)) {
				for (i++; org[i] != EOS && (!uS.isskip(org,i,&dFnt,MBF) ||
						uS.isRightChar(org,i,'/',&dFnt,MBF) || uS.isRightChar(org,i,'<',&dFnt,MBF) ||
						uS.isRightChar(org,i,'.',&dFnt,MBF) || uS.isRightChar(org,i,'!',&dFnt,MBF) ||
						uS.isRightChar(org,i,'?',&dFnt,MBF) || uS.isRightChar(org,i,',',&dFnt,MBF)); i++) {
				}
			} else {
				if (MBF) {
					for (i++; !uS.isskip(org,i,&dFnt,MBF) && org[i]; i++) {
						if (!nomap) {
							if (my_CharacterByteType(org, (short)i, &dFnt) == 0)
								org[i] = (char)tolower((unsigned char)org[i]);
						}
					}
				} else {
					for (i++; !uS.isskip(org,i,&dFnt,MBF) && org[i]; i++) {
						if (!nomap)
							org[i] = (char)tolower((unsigned char)org[i]);
					}
				}
			}
			if (delf) {
				if (excl) {
					for (; beg != temp; beg++) *beg = ' ';
				}
				excl = TRUE;
				beg = temp;
			}
			res = org[i];
			org[i] = EOS;
			if (excludewd(temp))
				excl = FALSE;
			else if (CLAN_PROG_NUM == MLT)
				isGWordFound = TRUE;
			org[i] = res;
		}
	} while (org[i]) ;

	if (excl) {
		for (; beg != org+i; beg++)
			*beg = ' ';
	}
}

static void filterMorfs(char *org) {
	register int i;
	char *temp, t;

	i = 0;
	do {
		for (; uS.isskip(org,i,&dFnt,MBF) && org[i] && org[i] != '['; i++) ;
		if (org[i]) {
			temp = org+i;
			if (!nomap) {
				if (MBF) {
					if (my_CharacterByteType(org, (short)i, &dFnt) == 0)
						org[i] = (char)tolower((unsigned char)org[i]);
				} else {
					org[i] = (char)tolower((unsigned char)org[i]);
				}
			}
			if (uS.isRightChar(org, i, '[', &dFnt, MBF)) {
				if (MBF) {
					for (i++; !uS.isRightChar(org, i, ']', &dFnt, MBF) && org[i]; i++) {
						if (!nomap) {
							if (my_CharacterByteType(org, (short)i, &dFnt) == 0)
								org[i] = (char)tolower((unsigned char)org[i]);
						}
					}
				} else {
					for (i++; !uS.isRightChar(org, i, ']', &dFnt, MBF) && org[i]; i++) {
						if (!nomap)
							org[i] = (char)tolower((unsigned char)org[i]);
					}
				}
				if (org[i]) i++;
			} else if (uS.isRightChar(org, i, '+', &dFnt, MBF) || uS.isRightChar(org, i, '-', &dFnt, MBF)) {
				for (i++; org[i] != EOS && (!uS.isskip(org,i,&dFnt,MBF) ||
						uS.isRightChar(org,i,'.',&dFnt,MBF) || uS.isRightChar(org,i,'!',&dFnt,MBF) ||
						uS.isRightChar(org,i,'?',&dFnt,MBF) || uS.isRightChar(org,i,',',&dFnt,MBF)); i++) {
				}
			} else {
				if (MBF) {
					for (i++; !uS.isskip(org,i,&dFnt,MBF) && org[i]; i++) {
						if (!nomap) {
							if (my_CharacterByteType(org, (short)i, &dFnt) == 0)
								org[i] = (char)tolower((unsigned char)org[i]);
						}
					}
				} else {
					for (i++; !uS.isskip(org,i,&dFnt,MBF) && org[i]; i++) {
						if (!nomap)
							org[i] = (char)tolower((unsigned char)org[i]);
					}
				}
			}
			t = org[i];
			org[i] = EOS;
			if (temp[0] == '-') {
				if ((CLAN_PROG_NUM == MLT || !uS.ismorfchar(temp, 0, &dFnt, rootmorf, MBF)) && !uS.isToneUnitMarker(temp)) {
					for (; temp != org+i; temp++)
						*temp = ' ';
				}
			}
			org[i] = t;
		}
	} while (org[i]) ;
}

static char ml_excludeUtter(char *line) {
	int pos;

	if (CLAN_PROG_NUM == MLT) {
		for (pos=0; line[pos] != EOS; pos++) {
			if (mlt_excludeUtter(line, pos, NULL)) { // xxx, yyy, www
				return(TRUE);
			}
		}
	} else {
		if (isMLUEpostcode && isPostCodeOnUtt(line, "[+ mlue]"))
			return(TRUE);
		for (pos=0; line[pos] != EOS; pos++) {
			if (mlu_excludeUtter(line, pos, NULL)) { // xxx, yyy, www
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

static char isEmptyLine(char *line) {
	int pos;
	char sq;

	sq = FALSE;
	for (pos=0; line[pos] != EOS; pos++) {
		if (uS.isRightChar(line, pos, '[', &dFnt, MBF)) {
			sq = TRUE;
		} else if (uS.isRightChar(line, pos, ']', &dFnt, MBF))
			sq = FALSE;
		if (!uS.isskip(line,pos,&dFnt,MBF) && !sq) {
			return(FALSE);
		}
	}
	return(TRUE);
}

static void addUtteranceDel(char *sLine, char *line) {
	int  pos, i;
	char POS[BUFSIZ];

	for (pos=0; line[pos] != EOS; pos++) {
		if (line[pos] == '?' && line[pos+1] == '|')
			;
		else if (uS.IsUtteranceDel(line, pos)) {
			return;
		}
	}
	for (pos=0; sLine[pos] != EOS; pos++) {
		if (uS.IsUtteranceDel(sLine, pos)) {
			while (pos >= 0 && uS.IsCharUtteranceDel(sLine, pos))
				pos--;
			pos++;
			i = 0;
			while (uS.IsCharUtteranceDel(sLine, pos))
				POS[i++] = sLine[pos++];
			POS[i] = EOS;
			if (POS[0] != EOS) {
				strcat(line, " ");
				strcat(line, POS);
			}
			return;
		}
	}
}

int ml_getwholeutter() {
	int i, postRes;
	extern char ByTurn;
	extern char getrestline;
	extern char skipgetline;

	isGWordFound = FALSE;
	if (ByTurn >= 2) {
		ByTurn = 1;
		return(TRUE);
	}
	do {
		if (chatmode) {
			do {
				if (lineno > -1L) lineno = lineno + tlineno;
				tlineno = 0L;
				if (!getspeaker(utterance->speaker, utterance->attSp, 0))
					return(FALSE);
				if (*utterance->speaker == '*')
					postcodeRes = 0;
				if (*utterance->speaker == '@') {
					if (SPRole != NULL) {
						if (uS.partcmp(utterance->speaker,"@ID:",FALSE, FALSE)) {
							IsModUttDel = FALSE;
							cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
							skipgetline = TRUE;
							getrestline = 0;
							SetSPRoleIDs(utterance->line);
						} else if (uS.partcmp(utterance->speaker,PARTICIPANTS,FALSE,FALSE)) {
							IsModUttDel = FALSE;
							cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
							skipgetline = TRUE;
							getrestline = 0;
							SetSPRoleParticipants(utterance->line);
						}
					}
					if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
						IsModUttDel = FALSE;
						cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
						skipgetline = TRUE;
						getrestline = 0;
						cleanUpIDTier(utterance->line);
						if (IDField != NULL) {
							SetIDTier(utterance->line);
						}
					} else if (uS.partcmp(utterance->speaker,"@Languages:",FALSE,FALSE) || uS.partcmp(utterance->speaker,"@New Language:",FALSE,FALSE)) {
						if (isLanguageExplicit == 1) {
							IsModUttDel = FALSE;
							cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
							skipgetline = TRUE;
							getrestline = 0;
							addToLanguagesTable(utterance->line, utterance->speaker);
						}
					} else if (uS.partcmp(utterance->speaker,"@Options:",FALSE,FALSE)) {
						IsModUttDel = FALSE;
						cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
						skipgetline = TRUE;
						getrestline = 0;
						checkOptions(utterance->line);
/*
					} else if (uS.partcmp(utterance->speaker,MEDIAHEADER,FALSE,FALSE)) {
						IsModUttDel = FALSE;
						cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
						skipgetline = TRUE;
						getrestline = 0;
						getMediaName(utterance->line, cMediaFileName, FILENAME_MAX);
*/
					} else if (uS.isUTF8(utterance->speaker)) {
						dFnt.isUTF = 1;
						if (isOrgUnicodeFont(dFnt.orgFType)) {
							NewFontInfo finfo;
							SetDefaultUnicodeFinfo(&finfo);
							selectChoosenFont(&finfo, FALSE, FALSE);
						}
						tlineno--;
					} else if (uS.partcmp(utterance->speaker,FONTHEADER,FALSE,FALSE)) {
						IsModUttDel = FALSE;
						cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
						skipgetline = TRUE;
						getrestline = 0;
						if (cutt_SetNewFont(utterance->line, EOS))
							tlineno--;
					} else if (uS.isInvisibleHeader(utterance->speaker)) {
						tlineno--;
					}
					if (uS.partcmp(utterance->speaker, "@ENDTURN", FALSE, FALSE))
						ml_spchanged = TRUE;
					if (checktier(utterance->speaker))
						break;
					killline(utterance->line, utterance->attLine);
				} else if (rightspeaker || *utterance->speaker == '*') {
					if (checktier(utterance->speaker)) {
						rightspeaker = TRUE;
						break;
					} else if (*utterance->speaker == '*') {
						rightspeaker = FALSE;
						i = strlen(utterance->speaker) - 1;
						for (; i >= 0 && isSpace(utterance->speaker[i]); i--) ;
						utterance->speaker[i+1] = EOS;
						if (strcmp(Toldsp,utterance->speaker) != 0) {
							killline(utterance->line, utterance->attLine);
/*
							if (uttline != utterance->line)
								strcpy(uttline,utterance->line);
							filterData("*",uttline);
							lfilter(uttline);
							filterwords("*",uttline,exclude);
							postRes = isPostCodeFound(utterance);
							if (postRes == 5 || postRes == 1) {
							} else if (isGWordFound && isEmptyLine(uttline)) {
							} else if (!ml_excludeUtter(uttline))
								ml_spchanged = TRUE;
*/
							if (!ml_excludeUtter(utterance->line)) {
								ml_isSkip = FALSE;
								strcpy(Toldsp,utterance->speaker);
								ml_spchanged = TRUE;
							} else
								ml_isSkip = TRUE;
						} else
							killline(utterance->line, utterance->attLine);
					} else
						killline(utterance->line, utterance->attLine);
					if (*utterance->speaker == '*') {
						if (linkMain2Mor)
							cutt_cleanUpLine(utterance->speaker, utterance->line, utterance->attLine, 0);
						strcpy(org_spName, utterance->speaker);
						strcpy(org_spTier, utterance->line);
					}
				} else
					killline(utterance->line, utterance->attLine);
			} while (1) ;
			IsModUttDel = ((*utterance->speaker == '*') && (chatmode < 3));
			cutt_getline(utterance->speaker, utterance->line, utterance->attLine, 0);
			if (*utterance->speaker == '*') {
				strcpy(org_spName, utterance->speaker);
				strcpy(org_spTier, utterance->line);
			}
			if (uS.partwcmp(utterance->line, FONTMARKER))
				cutt_SetNewFont(utterance->line,']');
			if (*utterance->speaker == '%') {
				addUtteranceDel(org_spTier, utterance->line);
			}
			if (uttline != utterance->line)
				strcpy(uttline,utterance->line);
			if (uS.partcmp(utterance->speaker,"%mor:",FALSE, FALSE)) {
				if (isMORSearch() || linkMain2Mor) {
					createMorUttline(utterance->line, org_spTier, "%mor:", utterance->tuttline, TRUE, linkMain2Mor);
					strcpy(utterance->tuttline, utterance->line);
					filterMorTier(uttline, utterance->line, 2, linkMain2Mor);
				} else
					filterMorTier(uttline, NULL, 0, linkMain2Mor);
			}
			filterData(utterance->speaker,uttline);
			lfilter(uttline);
			filterMorfs(uttline);
			postRes = isPostCodeFound(utterance->speaker, utterance->line);
			if (postRes == 5 || postRes == 1) {
				if (!ml_excludeUtter(utterance->line)) {
					if (strcmp(Toldsp,utterance->speaker) != 0) {
						strcpy(Toldsp,utterance->speaker);
						ml_spchanged = TRUE;
					}
				}
			} else if (isGWordFound && isEmptyLine(uttline)) {
				if (!ml_excludeUtter(utterance->line)) {
					if (strcmp(Toldsp,utterance->speaker) != 0) {
						strcpy(Toldsp,utterance->speaker);
						ml_spchanged = TRUE;
					}
				}
			} else if (CntWUT || CntFUttLen) {
				if (CntWUT && ToWU && WUCounter > ToWU)
					return(FALSE);
				if (!ml_excludeUtter(utterance->line)) { // xxx, yyy, www
					if (*utterance->speaker == '*')
						ml_isSkip = FALSE;
					if (strcmp(Toldsp,utterance->speaker) != 0) {
						strcpy(Toldsp,utterance->speaker);
						ml_spchanged = TRUE;
					}
				} else if (*utterance->speaker == '*')
					ml_isSkip = TRUE;
				if (uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
					break;
				}
				strcpy(templineC, uttline);
//				if (FilterTier == 1)
//					filterwords(utterance->speaker,templineC,exclude);
				if (FilterTier > 1)
					filterwords(utterance->speaker,uttline,exclude);
				if (!ml_excludeUtter(templineC) && !ml_isSkip) { // xxx, yyy, www
					i = strlen(utterance->speaker) - 1;
					for (; i >= 0 && isSpace(utterance->speaker[i]); i--) ;
					utterance->speaker[i+1] = EOS;
					if (strcmp(Toldsp,utterance->speaker) != 0) {
						ml_spchanged = TRUE;
					}
					if (!CntFUttLen) {
						if (rightrange(*utterance->speaker,templineC,uttline))
							break;
					} else if (rightUttLen(utterance->speaker,utterance->line,templineC,&utterance->uttLen)) {
						if (!CntWUT)
							break;
						else if (rightrange(*utterance->speaker,templineC,uttline))
							break;
					}
				}
			} else {
				filterwords(utterance->speaker,uttline,exclude);
				break;
			}
		} else {
			if (!gettextspeaker())
				return(FALSE);
			postcodeRes = 0;
			IsModUttDel = chatmode < 3;
			cutt_getline("*", utterance->line, utterance->attLine, 0);
			if (uttline != utterance->line)
				strcpy(uttline,utterance->line);
			filterData("*",uttline);
			lfilter(uttline);
			postRes = isPostCodeFound(utterance->speaker, utterance->line);
			if (postRes == 5 || postRes == 1) {
			} else if (isGWordFound && isEmptyLine(uttline)) {
			} else if (CntWUT || CntFUttLen) {
				if (CntWUT && ToWU && WUCounter > ToWU)
					return(FALSE);
				strcpy(templineC, uttline);
//				if (FilterTier == 1)
//					filterwords("*",templineC,exclude);
				if (FilterTier > 1)
					filterwords("*",uttline,exclude);
				if (!CntFUttLen) {
					if (rightrange('*',templineC,uttline))
						break;
				} else if (rightUttLen("*",utterance->line,templineC,&utterance->uttLen)) {
					if (!CntWUT)
						break;
					else if (rightrange('*',templineC,uttline))
						break;
				}
			} else {
				filterwords("*",uttline,exclude);
				break;
			}
		}
	} while (1) ;
	if (chatmode && *utterance->speaker == '*') {
		i = strlen(utterance->speaker) - 1;
		for (; i >= 0 && (utterance->speaker[i] == ' ' ||
			utterance->speaker[i] == '\t'); i--) ;
		utterance->speaker[i+1] = EOS;
		if (!ml_excludeUtter(utterance->line)) {
			if (strcmp(Toldsp,utterance->speaker) != 0) {
				strcpy(Toldsp,utterance->speaker);
				ml_spchanged = TRUE;
			}
		}
	}
	if (ByTurn == 2)
		return(FALSE);
	else
		return(TRUE);
}
