/**********************************************************************
	"Copyright 2009 Leonid Spektor. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX)
#define _main contacts_main
#define call contacts_call
#define getflag contacts_getflag
#define init contacts_init
#define usage contacts_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3

#define GLOSSARY "T=TAPE; D=DVD; R=DVD-R; C=COPY\\n"

#ifndef SPECIALTEXTFILESTR
	#define SPECIALTEXTFILESTR "lXs Special Text file saves all fonts LxS\n"
#endif

extern struct tier *defheadtier;
extern char LeaveSlashes;
extern char OverWriteFile;
extern char AddCEXExtension;

typedef struct {
	char name[1024];
	char field[1024];
} repeats;

typedef struct {
	FNType lName[1024];
	FNType fName[1024];
	char title[2500];
	char org[1024];
	char web[1024];
	char notes[4096];
	repeats phone[10];
	int		phoneCnt;
	repeats email[10];
	int		emailCnt;
	repeats addrs[10];
	int		addrsCnt;
} tiCard;

static tiCard *iCard;
static char doGPS;
static char isTapes;
static char isOldFormat;
static char isUTFFile;
static int  fieldLen;

static void init_iCard(void) {
	iCard->lName[0] = EOS;
	iCard->fName[0] = EOS;
	iCard->title[0] = EOS;
	iCard->org[0]   = EOS;
	iCard->web[0]   = EOS;
	iCard->notes[0] = EOS;
	iCard->phoneCnt = 0;
	iCard->emailCnt = 0;
	iCard->addrsCnt = 0;
}

void init(char f) {
	extern char GExt[];

	if (f) {
		doGPS = FALSE;
		fieldLen = 2100;
		OverWriteFile = TRUE;
		AddCEXExtension = FALSE;
		onlydata = 1;
		stout = FALSE;
		isUTFFile = TRUE;
		LeaveSlashes = '\0';
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier) {
			if (defheadtier->nexttier)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		nomap = TRUE;
		isTapes = FALSE;
		isOldFormat = FALSE;
	} else {
		if (doGPS)
			strcpy(GExt, ".cdc");
		else if (!isOldFormat && chatmode == 0)
			strcpy(GExt, ".txt");
	}
	init_iCard();
}

void usage() {
	printf("Usage: contacts [-c -g -o %s] filename(s)\n",mainflgs());
	printf("contacts phones.cdc\n");
	printf("contacts -y Attractions.e32\n");
	printf("contacts -g current.txt\n");
	printf("contacts -c -y tapes.cdc\n");
	printf("contacts -c -y -o tapes.cdc - OLD iPod format\n");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = CONTACTS;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	iCard = NEW(tiCard);
	if (iCard == NULL)
		fprintf(stderr,"ERROR: Out of memory.\n");
	else {
		bmain(argc,argv,NULL);
		free(iCard);
	}
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'c':
			isTapes = TRUE;
			no_arg_option(f);
			break;
		case 'g':
			doGPS = TRUE;
			no_arg_option(f);
			maingetflag("-y","",i);
			break;
		case 'o':
			isOldFormat = TRUE;
			no_arg_option(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

#define FieldDelim(chrs) (chrs == ',' || chrs == '\n' || chrs == EOS)
#define isjunk(chrs) (isSpace(chrs) || chrs == '\014' || chrs== '\n' || chrs== ',' || chrs== ';' || chrs== '-')

static char FieldMarker(char *line, int pos) {
	if (pos == 0 || isjunk(line[pos-1])) {
		if (line[pos] == 't' || line[pos] == 'c' || line[pos] == 'w' || line[pos] == 'h' ||
			line[pos] == 'p' || line[pos] == 'o' || line[pos] == 'n' || 
			line[pos] == 'a' || line[pos] == 'A' || line[pos] == 'e' || line[pos] == 'E') {
			if (line[pos+1] == ':')
				return(true);
		}
	}
	return(false);
}

static void fill_iCard(char *line) {
	int pos, i;
	char *tField;

	i = 0;
	for (pos=0; !FieldDelim(line[pos]) && !FieldMarker(line,pos); pos++)
		iCard->lName[i++] = line[pos];
	iCard->lName[i] = EOS;
	if (line[pos] == ',') {
		i = 0;
		for (pos++; !FieldDelim(line[pos]) && !FieldMarker(line,pos); pos++)
			iCard->fName[i++] = line[pos];
		iCard->fName[i] = EOS;
	}
	if (line[pos] == EOS)
		return;
	do {
		while (!FieldMarker(line,pos) && line[pos] != EOS)
			pos++;
		if (line[pos] == EOS)
			break;
		if (line[pos] == 't') {
			if (iCard->title[0] == EOS)
				tField = iCard->title;
			else {
				strcat(iCard->title, "\\n");
				tField = iCard->title + strlen(iCard->title);
			}
		} else if (line[pos] == 'c') {
			if (iCard->phoneCnt < 10) {
				tField = iCard->phone[iCard->phoneCnt].field;
				strcpy(iCard->phone[iCard->phoneCnt].name, "CELL");
				iCard->phoneCnt++;
			} else
				tField = NULL;
		} else if (line[pos] == 'w') {
			if (iCard->phoneCnt < 10) {
				tField = iCard->phone[iCard->phoneCnt].field;
				strcpy(iCard->phone[iCard->phoneCnt].name, "WORK");
				iCard->phoneCnt++;
			} else
				tField = NULL;
		} else if (line[pos] == 'h') {
			if (iCard->phoneCnt < 10) {
				tField = iCard->phone[iCard->phoneCnt].field;
				strcpy(iCard->phone[iCard->phoneCnt].name, "HOME");
				iCard->phoneCnt++;
			} else
				tField = NULL;
		} else if (line[pos] == 'p') {
			if (iCard->phoneCnt < 10) {
				tField = iCard->phone[iCard->phoneCnt].field;
				strcpy(iCard->phone[iCard->phoneCnt].name, "PAGER");
				iCard->phoneCnt++;
			} else
				tField = NULL;
		} else if (line[pos] == 'e') {
			if (iCard->emailCnt < 10) {
				tField = iCard->email[iCard->emailCnt].field;
				strcpy(iCard->email[iCard->emailCnt].name, "HOME");
				iCard->emailCnt++;
			} else
				tField = NULL;
		} else if (line[pos] == 'E') {
			if (iCard->emailCnt < 10) {
				tField = iCard->email[iCard->emailCnt].field;
				strcpy(iCard->email[iCard->emailCnt].name, "WORK");
				iCard->emailCnt++;
			} else
				tField = NULL;
		} else if (line[pos] == 'o') {
			if (iCard->org[0] == EOS)
				tField = iCard->org;
			else {
				strcat(iCard->org, "\\n");
				tField = iCard->org + strlen(iCard->org);
			}
		} else if (line[pos] == 'n') {
			if (iCard->notes[0] == EOS)
				tField = iCard->notes;
			else {
				strcat(iCard->notes, "\\n");
				tField = iCard->notes + strlen(iCard->notes);
			}
		} else if (line[pos] == 'a') {
			if (iCard->addrsCnt < 10) {
				tField = iCard->addrs[iCard->addrsCnt].field;
				strcpy(iCard->addrs[iCard->addrsCnt].name, "HOME");
				iCard->addrsCnt++;
			} else
				tField = NULL;
		} else if (line[pos] == 'A') {
			if (iCard->addrsCnt < 10) {
				tField = iCard->addrs[iCard->addrsCnt].field;
				strcpy(iCard->addrs[iCard->addrsCnt].name, "WORK");
				iCard->addrsCnt++;
			} else
				tField = NULL;
		} else {
			fprintf(stderr, "ERROR: Can't handle field \"%c:\"\n", line[pos]);
			tField = NULL;
		}
		pos++;
		if (line[pos] == EOS)
			break;
		pos++;
		
		i = 0;
		for (; !FieldMarker(line,pos) && line[pos] != EOS; pos++) {
			if (tField != NULL)
				tField[i++] = line[pos];
		}
		if (tField != NULL)
			tField[i] = EOS;
	} while (line[pos] != EOS) ;
}

static void cleanup(char *st, char justReplace) {
	register int i;
	register int j;

	i = strlen(st) - 1;
	while (i >= 0 && (FieldDelim(st[i]) || isjunk(st[i]))) i--;
	st[i+1] = EOS;
	if (justReplace)
		strcat(st, " ");
	for (i=0; (FieldDelim(st[i]) || isjunk(st[i])) && st[i] != EOS; i++) ;
	if (i > 0)
		strcpy(st, st+i);

	for (i=0; st[i] != EOS; i++) {
		if (!isUTFFile && !isOldFormat && !chatmode) {
			if (st[i] <= 0 || st[i] > 127) {
				fprintf(stderr,"ERROR: Bad char found '%c' (%d) on line %s.\n", st[i], st[i], st);
				free(iCard);
				cutt_exit(0);
			}
		}
		if (st[i] == '\n' || (st[i] == '\\' && st[i+1] == 'n')) {
			if (st[i] == '\\' && st[i+1] == 'n')
				i++;
			else {
				if (isjunk(st[i+1]) && !justReplace) {
					st[i++] = '\\';
					st[i] = 'n';
				} else
					st[i] = ' ';
			}
			if (isjunk(st[i+1]) && st[i+1] != EOS) {
				i++;
				j = i;
				while (isjunk(st[i]) && st[i] != EOS)
					i++;
				strcpy(st+j, st+i);
			}
		}
	}
}

static void cleanupFileName(FNType *fn, char justReplace) {
	register int i;
	register int j;

	i = strlen(fn) - 1;
	while (i >= 0 && (FieldDelim(fn[i]) || isjunk(fn[i]))) i--;
	fn[i+1] = EOS;
	if (justReplace)
		uS.str2FNType(fn, strlen(fn), " ");
	for (i=0; (FieldDelim(fn[i]) || isjunk(fn[i])) && fn[i] != EOS; i++) ;
	if (i > 0)
		strcpy(fn, fn+i);

	for (i=0; fn[i] != EOS; i++) {
		if (!isUTFFile && !isOldFormat && !chatmode) {
			if (fn[i] <= 0 || fn[i] > 127) {
				fprintf(stderr,"ERROR: Bad char found '%c' (%d) on line %s.\n", fn[i], fn[i], fn);
				free(iCard);
				cutt_exit(0);
			}
		}
		if (fn[i] == '\n' || (fn[i] == '\\' && fn[i+1] == 'n')) {
			if (fn[i] == '\\' && fn[i+1] == 'n')
				i++;
			else {
				if (isjunk(fn[i+1]) && !justReplace) {
					fn[i++] = '\\';
					fn[i] = 'n';
				} else
					fn[i] = ' ';
			}
			if (isjunk(fn[i+1]) && fn[i+1] != EOS) {
				i++;
				j = i;
				while (isjunk(fn[i]) && fn[i] != EOS)
					i++;
				strcpy(fn+j, fn+i);
			}
		}
	}
}

static char makeTapesTitle(char *notes, char *title) {
	int i, cnt;
	char targetChar;

	title[0] = EOS;
	notes = notes + strlen(GLOSSARY);
	for (; *notes != '-' && *notes != '=' && *notes != EOS; notes++) ;
	if (*notes == EOS)
		return(FALSE);
	targetChar = *notes;
	i = 0;
	cnt = 0;
	for (notes++; *notes != EOS; notes++) {
		if ((isSpace(*notes) && cnt> 10) || *notes== '\n' || (*notes== '\\' &&  *(notes+1)== 'n'))
			break;
		title[i++] = *notes;
		cnt++;
	}
	if (*notes == EOS) {
		if (title[i-2] == '\\' && title[i-1] == 'n')
			i = i - 2;
		else if (title[i-1] == '\n')
			i = i - 1;
	} else
		title[i++] = '-';
	title[i] = EOS;

	notes = strrchr(notes, targetChar);
	if (notes == NULL)
		return(TRUE);
	cnt = 0;
	for (notes++; *notes != EOS; notes++) {
		if ((isSpace(*notes) && cnt> 10) || *notes== '\n' || (*notes== '\\' &&  *(notes+1)== 'n'))
			break;
		title[i++] = *notes;
		cnt++;
	}
	title[i] = EOS;
	return(TRUE);
}

static char makeOtherTitle(char *notes, char *title) {
	int i, j;

	title[0] = EOS;
	for (; *notes != '*' && *notes != EOS; notes++) ;
	if (*notes == EOS)
		return(FALSE);
	i = 0;
	for (; *notes != EOS; notes++) {
		if (isSpace(*notes) || *notes== '\n' || (*notes== '\\' &&  *(notes+1)== 'n'))
			break;
		title[i++] = *notes;
	}
	title[i] = EOS;

	for (i=0; templineC1[i] != EOS; i++) {
		if (templineC1[i] == '*') {
			for (j=i; templineC1[j] != '=' && templineC1[j] != EOS; j++) ;
			if (templineC1[j] == EOS)
				break;
			templineC1[j] = EOS;
			if (!strcmp(title, templineC1+i)) {
				templineC1[j] = '=';
				for (i=0,j++; templineC1[j] != ';' && templineC1[j] != EOS; j++)
					title[i++] = templineC1[j];
				title[i] = EOS;
				return(TRUE);
			} else
				templineC1[j] = '=';
			if (templineC1[j] != EOS)	
				i = j;
			else
				i = j - 1;
		}
	}
	title[0] = EOS;
	return(FALSE);
}

static void output_iCard(void) {
	int pos;
	char let;

	if (isOldFormat || chatmode) {
		fprintf(fpout, "BEGIN:VCARD\n");
		fprintf(fpout, "VERSION:3.0\n");
		cleanupFileName(iCard->lName, false);
		cleanupFileName(iCard->fName, false);
		if (chatmode == 0) {
			fprintf(fpout, "FN;CHARSET=macintosh:%s\n", iCard->lName);
		} else {
			if (iCard->lName[0] == EOS)
				uS.str2FNType(iCard->lName, strlen(iCard->lName), "Generic");
			if (isOldFormat)
				fprintf(fpout, "N:�%s;%s;;;\n", iCard->lName, iCard->fName);
			else
				fprintf(fpout, "N:%s;%s;;;\n", iCard->lName, iCard->fName);
			fprintf(fpout, "FN:%s %s\n", iCard->fName, iCard->lName);
		}
		cleanup(iCard->org, false);
		if (iCard->org[0] != EOS)
			fprintf(fpout, "ORG:%s\n", iCard->org);
		cleanup(iCard->title, false);
		if (iCard->title[0] != EOS)
			fprintf(fpout, "TITLE%s:%s\n",((chatmode==0) ? ";CHARSET=macintosh" : ""),iCard->title);

		for (pos=0; pos < iCard->phoneCnt; pos++) {
			cleanup(iCard->phone[pos].name, false);
			cleanup(iCard->phone[pos].field, false);
			if (iCard->phone[pos].name[0] != EOS && iCard->phone[pos].field[0] != EOS)
				fprintf(fpout, "TEL;type=%s%s:%s\n", iCard->phone[pos].name, 
								((pos==0) ? ";type=pref" : ""), iCard->phone[pos].field);
		}

		for (pos=0; pos < iCard->emailCnt; pos++) {
			cleanup(iCard->email[pos].name, false);
			cleanup(iCard->email[pos].field, false);
			if (iCard->email[pos].name[0] != EOS && iCard->email[pos].field[0] != EOS)
				fprintf(fpout, "EMAIL;type=%s%s:%s\n", iCard->email[pos].name, 
								((pos==0) ? ";type=pref" : ""), iCard->email[pos].field);
		}

		cleanup(iCard->web, false);
		if (iCard->web[0] != EOS)
			fprintf(fpout, "URL:%s\n", iCard->web);

		for (pos=0; pos < iCard->addrsCnt; pos++) {
			cleanup(iCard->addrs[pos].name, false);
			cleanup(iCard->addrs[pos].field, false);
			if (iCard->addrs[pos].name[0] != EOS && iCard->addrs[pos].field[0] != EOS)
				fprintf(fpout, "ADR;type=%s%s:;;%s\n", iCard->addrs[pos].name, 
								((pos==0) ? ";type=pref" : ""), iCard->addrs[pos].field);
		}

		cleanup(iCard->notes, false);
		if (iCard->notes[0] != EOS)
			fprintf(fpout, "NOTE:%s\n", iCard->notes);
		fprintf(fpout, "END:VCARD\n");
	} else {
		fclose(fpout);
		uS.str2FNType(iCard->lName+1, strlen(iCard->lName+1), ".txt");
		if ((fpout=fopen(iCard->lName+1, "w")) == NULL) {
			fprintf(stderr,"ERROR: Can't write file %s.\n", iCard->lName+1);
			free(iCard);
			cutt_exit(0);
		}
#ifdef _MAC_CODE
		settyp(iCard->lName+1, 0, 0, TRUE);
#endif
		fprintf(stderr,"\tCreating file: %s.\n", iCard->lName+1);
		fprintf(fpout, "<?xml encoding=\"UTF-8\"?>\n");
		if (isTapes) {
			let = (iCard->lName[1] == '0' ? iCard->lName[2] : iCard->lName[1]);
			if (makeTapesTitle(iCard->notes,templineC3))
				fprintf(fpout, "<TITLE>%c:%s</TITLE>\n", let, templineC3);
		} else {
			if (makeOtherTitle(iCard->notes,templineC3))
				fprintf(fpout, "<TITLE>%s</TITLE>\n", templineC3);
		}
		for (pos=0; iCard->notes[pos] != EOS; pos++) {
			if (iCard->notes[pos] == '\\' && iCard->notes[pos+1] == 'n') {
				fputc('\n', fpout);
				pos++;
			} else
				fputc(iCard->notes[pos], fpout);
		}
	}
	init_iCard();
}

static char isEmptyLine(char *line, char extraC) {
	register int i;

	for (i=0; line[i] != EOS; i++) {
		if (!isjunk(line[i]) && line[i] != extraC)
			return(false);
	}
	return(true);
}

static char textFieldMarker(char *line, char *name) {
	int i;

	if (line[0] == '*' && isalnum((unsigned char)line[1])) {
		for (i=0; !isSpace(line[i]) && line[i] != EOS; i++)
			name[i] = line[i];
		name[i] = EOS;
		return(true);
	}
	return(false);
}

static void convertTapesLine(char *line) {
	int i;

	if (line[0] == '*' && line[1] == '0')
		return;
	if (line[0] == '\\' && line[1] == 'n')
		return;

	if (line[0] == '*' && line[1] == '1') {
		strcpy(uttline, "=");
		strcat(uttline, line+2);
		uS.remblanks(uttline);
		strcat(uttline, ":");
		goto Fin;
	}

	strcpy(uttline+2, line+19);
	if (line[8] == 'R' && (isOldFormat || !isUTFFile)) {
		for (i=2; uttline[i] != '(' && uttline[i] != EOS; i++) ;
		if (uttline[i] != EOS)
			strcpy(uttline+2, uttline+i);
	}
	for (i=2; uttline[i] != '_' && uttline[i] != EOS; i++) ;
	if (uttline[i] != EOS) {
		uttline[i] = EOS;
		for (i++; uttline[i] == '_'; i++) ;
		if (uttline[i] == ' ')
			i++;
		if (!strncmp(uttline+i, "TAPE", 4))
			uttline[0] = 'T';
		else if (!strncmp(uttline+i, "DVD-", 4))
			uttline[0] = 'R';
		else if (!strncmp(uttline+i, "DVD", 3))
			uttline[0] = 'D';
		else if (!strncmp(uttline+i, "JVC", 3))
			uttline[0] = 'C';
		else if (!strncmp(uttline+i, "TDK", 3))
			uttline[0] = 'C';
		else if (!strncmp(uttline+i, "SONY", 4))
			uttline[0] = 'C';
		else if (!strncmp(uttline+i, "SCOTCH", 6))
			uttline[0] = 'C';
		else {
			fprintf(stderr, "ERROR: unknown media type on line %ld\n    %s\n", lineno, line);
			free(iCard);
			cutt_exit(0);
		}
	} else {
			fprintf(stderr, "ERROR: missing media type on line %ld\n    %s\n", lineno, line);
			free(iCard);
			cutt_exit(0);
	}
	uttline[1] = '-';
Fin:
	strcpy(line, uttline);
	strcat(line, "\\n");
}

static void theRest(void) {
	int len, cnt;
	char name[10], oldName[10], *fieldS;
	FNType *ext;

	cnt = 0;
	oldName[0] = EOS;
	templineC[0] = EOS;
	templineC1[0] = EOS;
	if (!isOldFormat && !chatmode && isTapes)
		strcpy(templineC, GLOSSARY);
	if (isTapes) {
		strcpy(name, "*0Movies");
		cnt = 1;
		uS.sprintf(iCard->lName, "%s-%d", name, cnt++);
	}
	if (isOldFormat || chatmode) {
		fieldLen = 2100;
		fieldS = iCard->title;
	} else {
		fieldLen = 4025;
		fieldS = iCard->notes;
	}

	ext = strrchr(oldfname, '.');
	if (ext == NULL)
		ext = oldfname;
	isUTFFile = uS.FNTypecmp(ext, ".e32", 0L);

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (chatmode == 0) {
			if (isEmptyLine(utterance->line, '*')) {
				if (isTapes && utterance->line[0] == '\n') {
					strcpy(utterance->line, "\\n");
				} else
					continue;
			}
			if (isTapes) {
				if (utterance->line[0] == '@')
					continue;
				if (!strncmp(utterance->line, SPECIALTEXTFILESTR, strlen(SPECIALTEXTFILESTR)))
					continue;
				if (!strncmp(utterance->line, "What's on the tapes", 19))
					break;
				convertTapesLine(utterance->line);
			} else {
				if (uS.partcmp(utterance->line, "#TITLE:", FALSE, FALSE)) {
					strcat(templineC1, utterance->line+7);
					continue;
				}
				cleanup(utterance->line, true);
			}

			if (textFieldMarker(utterance->line, name)) {
				len = strlen(fieldS)+strlen(templineC);
				 if (len > fieldLen) {
					if (fieldS[0] != EOS) {
						output_iCard();
						uS.sprintf(iCard->lName, "%s-%d", name, cnt++);
					} else {
						fprintf(stderr, "ERROR: line %ld is too long\n", lineno);
						free(iCard);
						cutt_exit(0);
					}
				}
				len = strlen(fieldS);
				if (fieldS[0] != EOS) {
					if (fieldS[len-2] == '\\' && fieldS[len-1] == 'n') ;
					else
						strcat(fieldS, "\\n");
					strcat(fieldS, "\\n");
				}
				strcat(fieldS, templineC);
				templineC[0] = EOS;
				if (!isOldFormat && !chatmode && isTapes)
					strcpy(templineC, GLOSSARY);
				if (uS.mStricmp(name, oldName)) {
					if (fieldS[0] != EOS) {
						output_iCard();
					}
					cnt = 1;
					uS.sprintf(iCard->lName, "%s-%d", name, cnt++);
				}
				strcpy(oldName, name);
				if (isTapes)
					continue;
			} else {
				len = strlen(templineC)+strlen(utterance->line);
				if (len > fieldLen) {
					if (isTapes) {
						strcat(fieldS, templineC);
						templineC[0] = EOS;
						if (!isOldFormat && !chatmode && isTapes)
							strcpy(templineC, GLOSSARY);
						if (fieldS[0] != EOS) {
							output_iCard();
							uS.sprintf(iCard->lName, "%s-%d", name, cnt++);
						} else {
							fprintf(stderr, "ERROR: line %ld is too long\n", lineno);
							free(iCard);
							cutt_exit(0);
						}
					} else {
						fprintf(stderr, "ERROR: line %ld is too long\n", lineno);
						free(iCard);
						cutt_exit(0);
					}
				}
			}
			strcat(templineC, utterance->line);
		} else {
			if (!uS.partcmp(utterance->speaker, "*", FALSE, FALSE))
				continue;
			init_iCard();
			if (!isEmptyLine(utterance->line, EOS)) {
				fill_iCard(utterance->line);
				output_iCard();
			}
		}
	}
	if (chatmode == 0 && (fieldS[0] != EOS || templineC[0] != EOS)) {
		len = strlen(fieldS)+strlen(templineC);
		 if (len > fieldLen) {
			if (fieldS[0] != EOS) {
				output_iCard();
			} else {
				fprintf(stderr, "ERROR: line %ld is too long\n", lineno);
				free(iCard);
				cutt_exit(0);
			}
		}
		len = strlen(fieldS);
		if (fieldS[0] != EOS) {
			if (fieldS[len-2] == '\\' && fieldS[len-1] == 'n') ;
			else
				strcat(fieldS, "\\n");
			strcat(fieldS, "\\n");
		}
		strcat(fieldS, templineC);
		if (fieldS[0] != EOS) {
			output_iCard();
		}
	}
#ifdef _MAC_CODE
	if (isOldFormat || chatmode)
		settyp(newfname, 0, 0, TRUE);
	else
		unlink(newfname);
#endif
}

static long converToMinSecs(char *from, char *to, long i) {
	char  degS[256], mS[5], restS[256];
	int   mI, degI, k;
	long  j;
	float restF, mF;

	j = strlen(to);
	for (k=0; isdigit(from[i]); i++)
		degS[k++] = from[i];
	degS[k] = EOS;
	if (from[i] == '.')
		i++;
	degI = atoi(degS);
	if (isdigit(from[i])) {
		k = 0;
		restS[k++] = '0';
		restS[k++] = '.';
		for (; isdigit(from[i]); i++)
			restS[k++] = from[i];
		restS[k] = EOS;
		mS[0] = restS[0];
		mS[1] = restS[1];
		mS[2] = restS[2];
		mS[3] = restS[3];
		mS[4] = EOS;
		mF = atof(mS);
		mF = mF * 60.0000;
		mI = (int)mF;
		restF = atof(restS);
		mF = (float)mI;
		restF = (restF - (mF / 60.0000)) * 3600.0000;
		while (restF >= 60.0000) {
			restF -= 60.0000;
			mI++;
		}
		while (mI >= 60) {
			mI -= 60;
			degI++;
		}
		sprintf(to+j, "%d%c%c %d' %.0f\"", degI, 0xc2, 0xb0, mI, restF);
	} else {
		sprintf(to+j, "%d%c%c", degI, 0xc2, 0xb0);
	}
	return(i);
}

static void processXMLTags(char *st) {
	while (*st != EOS) {
		if (!strncmp(st, "&amp;", 5)) {
			*st = '&';
			st++;
			strcpy(st, st+4);
		} else if (!strncmp(st, "&quot;", 6)) {
			*st = '"';
			st++;
			strcpy(st, st+5);
		} else if (!strncmp(st, "&apos;", 6)) {
			*st = '\'';
			st++;
			strcpy(st, st+5);
		} else if (!strncmp(st, "&lt;", 4)) {
			*st = '<';
			st++;
			strcpy(st, st+3);
		} else if (!strncmp(st, "&gt;", 4)) {
			*st = '>';
			st++;
			strcpy(st, st+3);
		} else
			st++;
	}
}

static void convertGPSCoordinates(void) {
	char isFirstFound, isCmtMode;
	long i, j;

	fprintf(fpout, "%s\n", UTF8HEADER);
	isFirstFound = FALSE;
	isCmtMode = FALSE;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		for (i=0L; isSpace(utterance->line[i]); i++) ;
		if (isCmtMode) {
			for (; utterance->line[i] != EOS; i++) {
				if (!strncmp(utterance->line+i, "</cmt>", 6)) {
					isCmtMode = FALSE;
					break;
				}
				templineC1[j++] = utterance->line[i];
			}
			if (!isCmtMode) {
				templineC1[j] = EOS;
				for (j=0L; templineC1[j] != EOS; j++) {
					if (templineC1[j] == '\n')
						templineC1[j] = ',';
				}
				processXMLTags(templineC1);
				if (templineC1[0] != EOS)
					fprintf(fpout, "comm: %s\n", templineC1);
			}
		} else if (!strncmp(utterance->line+i, "<wpt", 4)) {
			isFirstFound = TRUE;
			for (; !isdigit(utterance->line[i]) && utterance->line[i] != EOS; i++) ;
			templineC1[0] = EOS;
			if (isdigit(utterance->line[i])) {
				strcat(templineC1, "lat: ");
				if (i > 0 && utterance->line[i-1] == '-')
					strcat(templineC1, "S");
				else
					strcat(templineC1, "N");

				i = converToMinSecs(utterance->line, templineC1, i);
			}
			for (; !isdigit(utterance->line[i]) && utterance->line[i] != EOS; i++) ;
			if (isdigit(utterance->line[i])) {
				strcat(templineC1, ", lon: ");
				if (i > 0 && utterance->line[i-1] == '-')
					strcat(templineC1, "W");
				else if (i > 0 && utterance->line[i-1] == '+')
					strcat(templineC1, "E");
				i = converToMinSecs(utterance->line, templineC1, i);
			}
			processXMLTags(templineC1);
			if (templineC1[0] != EOS)
				fprintf(fpout, "%s\n", templineC1);
		} else if (!isFirstFound) {
		} else if (!strncmp(utterance->line+i, "<sym>", 5)) {
		} else if (!strncmp(utterance->line+i, "</gpx>", 6)) {
		} else if (!strncmp(utterance->line+i, "</wpt>", 6)) {
		} else if (!strncmp(utterance->line+i, "</cmt>", 6)) {
			if (isCmtMode) {
				templineC1[j] = EOS;
				for (j=0L; templineC1[j] != EOS; j++) {
					if (templineC1[j] == '\n')
						templineC1[j] = ',';
				}
				processXMLTags(templineC1);
				if (templineC1[0] != EOS)
					fprintf(fpout, "comm: %s\n", templineC1);
				isCmtMode = FALSE;
			}
		} else if (!strncmp(utterance->line+i, "<cmt>", 5)) {
			i += 5;
			isCmtMode = TRUE;
			for (; isSpace(utterance->line[i]); i++) ;
			for (j=0L; utterance->line[i] != EOS; i++) {
				if (!strncmp(utterance->line+i, "</cmt>", 6)) {
					isCmtMode = FALSE;
					break;
				}
				templineC1[j++] = utterance->line[i];
			}
			if (!isCmtMode) {
				templineC1[j] = EOS;
				processXMLTags(templineC1);
				if (templineC1[0] != EOS)
					fprintf(fpout, "comm: %s\n", templineC1);
			}
		} else if (!strncmp(utterance->line+i, "<name>", 6)) {
			i += 6;
			for (; isSpace(utterance->line[i]); i++) ;
			for (j=0L; utterance->line[i] != EOS; i++) {
				if (!strncmp(utterance->line+i, "</name>", 7))
					break;
				templineC1[j++] = utterance->line[i];
			}
			templineC1[j] = EOS;
			processXMLTags(templineC1);
			if (templineC1[0] != EOS)
				fprintf(fpout, "name: %s\n", templineC1);
		} else {
			processXMLTags(utterance->line);
			fprintf(fpout, utterance->line);
		}
	}
}

void call() {
	if (doGPS)
		convertGPSCoordinates();
	else
		theRest();
}
