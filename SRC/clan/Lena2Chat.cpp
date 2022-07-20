/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "cu.h"
#include "cutt-xml.h"
/* // NO QT
#ifdef _WIN32
	#include <TextUtils.h>
#endif
*/
#if !defined(UNX)
#define _main lena2chat_main
#define call lena2chat_call
#define getflag lena2chat_getflag
#define init lena2chat_init
#define usage lena2chat_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define MYSTRLEN 128

struct vocs {
	int  num;
	char name[SPEAKERLEN];
	long beg, end;
	struct vocs *nexvoc;
} ;

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char GExt[];
extern char isRecursive;

extern const char *MonthNames[];


static char mediaFName[FILENAME_MAX+2];
static Element *Lena_stack[30];

void usage() {
	printf("convert LENA XML files to CHAT files\n");
	printf("Usage: lena2chat [%s] filename(s)\n",mainflgs());
	mainusage(FALSE);
	puts("\nExample:  lena2chat +re *.its");
	cutt_exit(0);
}

void init(char s) {
	if (s) {
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
		stout = TRUE;
		onlydata = 1;
		CurrentElem = NULL;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = LENA2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
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
/*
static char Lena_isUttDel(char *line) {
	char bullet;
	long i;

	bullet = FALSE;
	i = strlen(line);
	for (; i >= 0L; i--) {
		if ((uS.IsUtteranceDel(line, i) || uS.IsCAUtteranceDel(line, i)) && !bullet)
			return(TRUE);
		if (line[i] == HIDEN_C)
			bullet = !bullet;
	}
	return(FALSE);
}
*/
static void Lena_makeText(char *line) {
	char *e;
	unsigned int c;

	while (*line != EOS) {
		if (!strncmp(line, "&amp;", 5)) {
			*line = '&';
			line++;
			strcpy(line, line+4);
		} else if (!strncmp(line, "&quot;", 6)) {
			*line = '"';
			line++;
			strcpy(line, line+5);
		} else if (!strncmp(line, "&apos;", 6)) {
			*line = '\'';
			line++;
			strcpy(line, line+5);
		} else if (!strncmp(line, "&lt;", 4)) {
			*line = '<';
			line++;
			strcpy(line, line+3);
		} else if (!strncmp(line, "&gt;", 4)) {
			*line = '>';
			line++;
			strcpy(line, line+3);
		} else if (*line == '{' && *(line+1) == '0' && *(line+2) == 'x') {
			for (e=line; *e != EOS && *e != '}'; e++) ;
			if (*e == '}') {
				sscanf(line+1, "%x", &c);
				*line = c;
				line++;
				strcpy(line, e+1);
			} else
				line++;
		} else
			line++;
	}
}

/* Lena-ITS Begin **************************************************************** */
/*
static void getCurrentLenaData(Element *CurrentElem, char *line) {
	Element *data;
	
	if (CurrentElem->cData != NULL) {
		strncpy(line, CurrentElem->cData, UTTLINELEN);
		line[UTTLINELEN-1] = EOS;
	} else if (CurrentElem->data != NULL) {
		line[0] = EOS;
		for (data=CurrentElem->data; data != NULL; data=data->next) {
			if (!strcmp(data->name, "CONST")) {
				strncpy(line, data->cData, UTTLINELEN);
				line[UTTLINELEN-1] = EOS;
			}
		}
	} else
		line[0] = EOS;
}
*/

static struct vocs *freeVocs(struct vocs *p) {
	struct vocs *tt;

	while (p != NULL) {
		tt = p;
		p = p->nexvoc;
		free(tt);
	}
	return(NULL);
}

static struct vocs *addNewVoc(struct vocs *root, int num, const char *name, long beg, long end) {
	struct vocs *nt;

	if (root == NULL) {
		root = NEW(struct vocs);
		nt = root;
	} else {
		nt = root;
		while (1) {
			if (nt->num == num && !strcmp(nt->name, name)) {
				if (beg >= 0)
					nt->beg = beg;
				if (end >= 0)
					nt->end = end;
				return(root);
			}
			if (nt->nexvoc == NULL)
				break;
			nt = nt->nexvoc;
		}
		nt->nexvoc = NEW(struct vocs);
		nt = nt->nexvoc;
	}
	nt->nexvoc = NULL;
	nt->num = num;
	strcpy(nt->name, name);
	nt->beg = beg;
	nt->end = end;
	return(root);
}

static struct vocs *sortVocs(struct vocs *root_org) {
	struct vocs *nt_org, *nt, *tnt, *root;

	root = NULL;
	for (nt_org=root_org; nt_org != NULL; nt_org=nt_org->nexvoc) {
		if (root == NULL) {
			root = NEW(struct vocs);
			nt = root;
			nt->nexvoc = NULL;
		} else {
			tnt= root;
			nt = root;
			while (1) {
				if (nt == NULL)
					break;
				if (nt_org->beg < nt->beg) {
					break;
				}
				tnt = nt;
				nt = nt->nexvoc;
			}
			if (nt == NULL) {
				tnt->nexvoc = NEW(struct vocs);
				nt = tnt->nexvoc;
				nt->nexvoc = NULL;
			} else if (nt == root) {
				root = NEW(struct vocs);
				root->nexvoc = nt;
				nt = root;
			} else {
				nt = NEW(struct vocs);
				nt->nexvoc = tnt->nexvoc;
				tnt->nexvoc = nt;
			}
		}
		nt->num = nt_org->num;
		strcpy(nt->name, nt_org->name);
		nt->beg = nt_org->beg;
		nt->end = nt_org->end;
	}
	root_org = freeVocs(root_org);
	return(root);
}


static char isMoreStartEnds(char *tag, char *name, int *num) {
	int i, j;

	name[0] = EOS;
	*num = 0;
	j = 0;
	if (!strncmp(tag, "start", 5)) {
		name[j++] = 's';
		i = 5;
	} else if (!strncmp(tag, "end", 3)) {
		name[j++] = 'e';
		i = 3;
	} else
		i = 0;
	if (i > 0) {
		for (; tag[i] != EOS && !isdigit(tag[i]); i++) {
			name[j++] = tag[i];
		}
		if (isdigit(tag[i])) {
			name[j] = EOS;
			*num = atoi(tag+i);
			return(TRUE);
		}
	}
	return(FALSE);
}

static void changeSec2MsecTime(char *s) {
	int i;

	for (i=0; s[i] != EOS; ) {
		if (s[i] == '.')
			strcpy(s+i, s+i+1);
		else
			i++;
		if (s[i] == 'S')
			s[i] = '0';
	}
}

static char getNextLenaTier(void) {
	int i, cnt, pauseNum, convNum;
	long ln = 0L, tln = 0L;
	long beg, end, tt;
	char dateS[BUFSIZ], timeS[BUFSIZ], xDB[BUFSIZ], sp[MYSTRLEN], wcnt[MYSTRLEN], com[BUFSIZ];
	struct vocs *root_voc, *p;
	Attributes *att;

	if (CurrentElem == NULL)
		return(FALSE);

	pauseNum = 0;
	convNum = 0;
	do {
		if (ln > tln) {
			tln = ln + 200;
			fprintf(stderr,"\r%ld ",ln);
		}
		ln++;
		if (CurrentElem == NULL) {
			if (stackIndex < 0)
				return(FALSE);
			do {
				CurrentElem = Lena_stack[stackIndex];
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				if (!strcmp(CurrentElem->name, "Pause") && stackIndex == 0) {
					fprintf(fpout, "@Eg:\tPause %d\n", pauseNum);
				}
				if (!strcmp(CurrentElem->name, "Conversation") && stackIndex == 0)
					fprintf(fpout, "@Eg:\tConversation %d\n", convNum);
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL) {
					if (stackIndex < 0)
						return(FALSE);
				} else
					break;
			} while (1) ;
		}
		if (!strcmp(CurrentElem->name, "Recording") && CurrentElem->data != NULL && stackIndex == -1) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "num")) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						fprintf(fpout, "@Comment:	start of recording %d\n", atoi(att->value+i));
					}
				} else if (!strcmp(att->name, "startClockTime")) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						strcpy(templineC, att->value+i);
						dateS[0] = templineC[8];
						dateS[1] = templineC[9];
						dateS[2] = '-';
						i = atoi(templineC+5) - 1;
						if (i < 0 || i > 11)
							strcpy(dateS+3, "UNK");
						else
							strcpy(dateS+3, MonthNames[i]);
						dateS[6]  = '-';
						dateS[7]  = templineC[0];
						dateS[8]  = templineC[1];
						dateS[9]  = templineC[2];
						dateS[10] = templineC[3];
						dateS[11] = EOS;
						timeS[0] = templineC[11];
						timeS[1] = templineC[12];
						timeS[2] = templineC[13];
						timeS[3] = templineC[14];
						timeS[4] = templineC[15];
						timeS[5] = templineC[16];
						timeS[6] = templineC[17];
						timeS[7] = templineC[18];
						timeS[8]  = EOS;
					}
				} else if (!strcmp(att->name, "endClockTime")) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						timeS[8]  = '-';
						timeS[9]  = templineC[11];
						timeS[10] = templineC[12];
						timeS[11] = templineC[13];
						timeS[12] = templineC[14];
						timeS[13] = templineC[15];
						timeS[14] = templineC[16];
						timeS[15] = templineC[17];
						timeS[16] = templineC[18];
						timeS[17] = EOS;
					}
				}
			}
			fprintf(fpout, "@Date:	%s\n", dateS);
			fprintf(fpout, "@Time Duration:	%s\n", timeS);
			if (stackIndex < 29) {
				stackIndex++;
				Lena_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "Pause") && CurrentElem->data != NULL && stackIndex == 0) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "num")) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						pauseNum = atoi(att->value+i);
					}
				}
			}
			fprintf(fpout, "@Bg:\tPause %d\n", pauseNum);
			if (stackIndex < 29) {
				stackIndex++;
				Lena_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "Conversation") && CurrentElem->data != NULL && stackIndex == 0) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "num")) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						convNum = atoi(att->value+i);
					}
				}
			}
			fprintf(fpout, "@Bg:\tConversation %d\n", convNum);
			if (stackIndex < 29) {
				stackIndex++;
				Lena_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "Segment") && stackIndex == 1) {
			beg = 0L;
			end = 0L;
			sp[0]   = EOS;
			wcnt[0] = EOS;
			com[0]  = EOS;
			xDB[0]  = EOS;
			root_voc = NULL;
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "spkr")) {
					strcpy(sp, "*");
					strcat(sp, att->value);
					strcat(sp, ":");
				} else if (!strcmp(att->name, "startTime")) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						changeSec2MsecTime(att->value+i);
						beg = atol(att->value+i);
					}
				} else if (!strcmp(att->name, "endTime")) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						changeSec2MsecTime(att->value+i);
						end = atol(att->value+i);
					}
				} else if (!strncmp(att->name, "startUtt", 8)) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						changeSec2MsecTime(att->value+i);
						tt = atol(att->value+i);
						i = atoi(att->name+8);
						root_voc = addNewVoc(root_voc, i, "vocalization", tt, -1);
					}
				} else if (!strncmp(att->name, "endUtt", 6)) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						changeSec2MsecTime(att->value+i);
						tt = atol(att->value+i);
						i = atoi(att->name+6);
						root_voc = addNewVoc(root_voc, i, "vocalization", -1, tt);
					}
				} else if (!strncmp(att->name, "average_dB", 6)) {
					strcat(xDB, "average_dB=\"");
					strcat(xDB, att->value);
					strcat(xDB, "\"");
				} else if (!strncmp(att->name, "peak_dB", 6)) {
					strcat(xDB, " peak_dB=\"");
					strcat(xDB, att->value);
					strcat(xDB, "\"");
				} else if (!strcmp(att->name, "conversationInfo")) {
					strcpy(com, att->value);
				} else {
					for (i=0; att->name[i] != EOS; i++) {
						if (!strncmp(att->name+i, "WordCnt", 7))
							break;
					}
					if (att->name[i] != EOS) {
						strcpy(wcnt, "&=w");
						strcat(wcnt, att->value);
						for (i=0; wcnt[i] != EOS; i++) {
							if (wcnt[i] == '.')
								wcnt[i] = '_';
						}
					} else if (isMoreStartEnds(att->name, templineC, &cnt)) {
						for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
						if (isdigit(att->value[i])) {
							changeSec2MsecTime(att->value+i);
							tt = atol(att->value+i);
							if (uS.mStricmp(templineC+1, "Cry") == 0)
								strcpy(templineC+1, "crying");
							else if (isupper((unsigned char)templineC[1]))
								templineC[1] = (char)tolower((unsigned char)templineC[1]);
							if (templineC[0] == 's')
								root_voc = addNewVoc(root_voc, cnt, templineC+1, tt, -1);
							else
								root_voc = addNewVoc(root_voc, cnt, templineC+1, -1, tt);
						}
					}
				}
			}
			root_voc = sortVocs(root_voc);
			p = root_voc;
			strcpy(templineC, wcnt);
			if (root_voc == NULL) {
				if (templineC[0] == EOS)
					strcat(templineC, "0");
				sprintf(templineC1, " . %c%ld_%ld%c", HIDEN_C, beg, end, HIDEN_C);
				strcat(templineC, templineC1);
			} else {
				if (p->beg > beg) {
					if (templineC[0] == EOS)
						strcat(templineC, "0");
					sprintf(templineC1, " %c%ld_%ld%c\n", HIDEN_C, beg, p->beg, HIDEN_C);
					strcat(templineC, templineC1);
				}
				while (p != NULL) {
					if (p->nexvoc == NULL) {
						if (p->end < end) {
							sprintf(templineC1, "&=%s %c%ld_%ld%c\n", p->name, HIDEN_C, p->beg, p->end, HIDEN_C);
							strcat(templineC, templineC1);
							sprintf(templineC1, "0 . %c%ld_%ld%c\n", HIDEN_C, p->end, end, HIDEN_C);
						} else
							sprintf(templineC1, "&=%s . %c%ld_%ld%c\n", p->name, HIDEN_C, p->beg, p->end, HIDEN_C);
						strcat(templineC, templineC1);
						break;
					} else {
						sprintf(templineC1, "&=%s %c%ld_%ld%c\n", p->name, HIDEN_C, p->beg, p->end, HIDEN_C);
						if (p->end < p->nexvoc->beg) {
							strcat(templineC, templineC1);
							sprintf(templineC1, "0 %c%ld_%ld%c\n", HIDEN_C, p->end, p->nexvoc->beg, HIDEN_C);
						}
						strcat(templineC, templineC1);
						p = p->nexvoc;
					}
				}
			}
			printout(sp, templineC, NULL, NULL, TRUE);
			if (com[0] != EOS)
				printout("%com:", com, NULL, NULL, TRUE);
			if (xDB[0] != EOS)
				printout("%xdb:", xDB, NULL, NULL, TRUE);
			root_voc = freeVocs(root_voc);
		}
		if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;

	} while (1);
	return(FALSE);
}

static char getNextLenaHeader(char *gender, char *DOB, int *d, char *mediaFName, char *version) {
	int i, isUPL_Header, isTransferredUPL, isApplicationData;
	Attributes *att;

	if (CurrentElem == NULL)
		return(FALSE);

	isUPL_Header = -1;
	isTransferredUPL = -1;
	isApplicationData = -1;
	do {
		if (CurrentElem != NULL && !strcmp(CurrentElem->name, "ITS") && CurrentElem->next == NULL) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "fileName")) {
					strcpy(mediaFName, "e");
					strncat(mediaFName, att->value, FILENAME_MAX-1);
					mediaFName[FILENAME_MAX] = EOS;
					Lena_makeText(mediaFName);
				}
			}
			CurrentElem = CurrentElem->data;
		} else if (CurrentElem != NULL && !strcmp(CurrentElem->name, "ProcessingUnit"))
			CurrentElem = CurrentElem->data;
		else if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;
		
		if (CurrentElem == NULL) {
			if (stackIndex < 0)
				return(FALSE);
			do {
				CurrentElem = Lena_stack[stackIndex];
				if (isUPL_Header == stackIndex)
					isUPL_Header = -1;
				if (isTransferredUPL == stackIndex)
					isTransferredUPL = -1;
				if (isApplicationData == stackIndex)
					isApplicationData = -1;
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL) {
					if (stackIndex < 0)
						return(FALSE);
				} else
					break;
			} while (1) ;
		}
		
		if (!strcmp(CurrentElem->name, "UPL_Header") && CurrentElem->data != NULL && stackIndex == -1) {
			if (stackIndex < 29) {
				stackIndex++;
				isUPL_Header = stackIndex;
				Lena_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "Recording") && CurrentElem->data != NULL && stackIndex == -1) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "startClockTime")) {
					for (i=0; !isdigit(att->value[i]) && att->value[i] != EOS; i++) ;
					if (isdigit(att->value[i])) {
						strcpy(templineC, att->value+i);
						d[0] = atoi(templineC+8);
						d[1] = atoi(templineC+5);
						d[2] = atoi(templineC);
					}
				}
			}
			break;
		}
		if (!strcmp(CurrentElem->name, "TransferredUPL") && isUPL_Header == 0 && CurrentElem->data != NULL) {
			if (stackIndex < 29) {
				stackIndex++;
				isTransferredUPL = stackIndex;
				Lena_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "ApplicationData") && isTransferredUPL == 1 && CurrentElem->data != NULL) {
			if (stackIndex < 29) {
				stackIndex++;
				isApplicationData = stackIndex;
				Lena_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "PrimaryChild") && isApplicationData == 2) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "DOB")) {
					strncpy(DOB, att->value, MYSTRLEN);
					DOB[MYSTRLEN] = EOS;
					Lena_makeText(DOB);
				} else if (!strcmp(att->name, "Gender")) {
					*gender = att->value[0];
				}
			}
		}
		if (!strcmp(CurrentElem->name, "Application") && isApplicationData == 2) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "Version")) {
					strncpy(version, att->value, MYSTRLEN);
					version[MYSTRLEN] = EOS;
					Lena_makeText(version);
				}
			}
		}
	} while (1);
	return(FALSE);
}
/* Lena-ITS End ****************************************************************** */

static int nd(int n) {
	if (n == 2)
		return(28);
	else if (n == 1 || n == 3 || n == 5 || n == 7 ||
			 n == 8 || n == 10 || n == 12)
		return(31);
	else
		return(30);
}

static void comuteAge(int *b, int *d, char *ageS, char *fNameAgeS) {
	char st[256];
	int a[3], temp_d_month;

	a[2] = d[2] - b[2];
	a[1] = d[1] - b[1];
	a[0] = d[0] - b[0];
	if (a[1] <= 0) {
		a[2]--;
		a[1] += 12;
	}
	if (a[0] < 0) {
		if (d[1] == b[1])
			temp_d_month = d[1];
		else
			temp_d_month = d[1] - 1;
		if (temp_d_month < 1)
			temp_d_month = 12;
		a[0] += nd(temp_d_month);
		if (a[0] < 0)
			a[0] = a[0] + (nd(d[1]) - nd(temp_d_month));
		a[1]--;
		if (a[1] <= 0) {
			a[2]--;
			a[1] += 12;
		}
	}
	if (a[1] >= 12) {
		a[2]++;
		a[1] -= 12;
	}
	if (a[1] < 10 && a[0] < 10)
		sprintf(ageS,"%d;0%d.0%d", a[2], a[1], a[0]);
	else if (a[1] < 10)
		sprintf(ageS,"%d;0%d.%d", a[2], a[1], a[0]);
	else if (a[0] < 10)
		sprintf(ageS,"%d;%d.0%d", a[2], a[1], a[0]);
	else
		sprintf(ageS,"%d;%d.%d", a[2], a[1], a[0]);
	fNameAgeS[0] = EOS;
	sprintf(st,"%d", a[2]);
	if (a[2] < 10)
		strcat(fNameAgeS, "0");
	strcat(fNameAgeS, st);
	sprintf(st,"%d", a[1]);
	if (a[1] < 10)
		strcat(fNameAgeS, "0");
	strcat(fNameAgeS, st);
	sprintf(st,"%d", a[0]);
	if (a[0] < 10)
		strcat(fNameAgeS, "0");
	strcat(fNameAgeS, st);
}

static FILE *Lena2chat_openfile(const FNType *oldfname, FNType *fname) {
	register int i = 0;
	register int len;
	FILE *fp;

	len = strlen(fname);
	if (AddCEXExtension[0] != EOS)
		uS.str2FNType(fname, strlen(fname), AddCEXExtension);
	for (i=97; i < 123 && !access(fname,0); i++) {
		sprintf(fname+len, "%c", i);
		if (AddCEXExtension[0] != EOS)
			uS.str2FNType(fname, strlen(fname), AddCEXExtension);
	}
	if (i >= 123) {
		strcpy(fname+len, "?: Too many versions.");
		return(NULL);
	}
	if (!uS.mStricmp(oldfname, fname))
		uS.str2FNType(fname, strlen(fname), ".1");
	fp = fopen(fname,"w");
#ifdef _MAC_CODE
	settyp(fname, 'TEXT', the_file_creator.out, FALSE);
#endif
	return(fp);
}

void call() {		/* this function is self-explanatory */
	int month, d[3], b[3], len;
	char gender, genderS[32], DOB[MYSTRLEN+2], DOB_clan[32], ageS[MYSTRLEN+2], fNameAgeS[MYSTRLEN+2], version[MYSTRLEN+2], *s;
	char isMP3;
	FNType newMediaFname[FNSize];
	FILE *tFPout;
	extern const char *MonthNames[];

	BuildXMLTree(fpin);
	b[0] = 0; b[1] = 0; b[2] = 0;
	d[0] = 0; d[1] = 0; d[2] = 0;
	DOB[0] = EOS;
	ageS[0] = EOS;
	fNameAgeS[0] = EOS;
	mediaFName[0] = EOS;
	gender = 0;
	mediaFName[0] = EOS;
	version[0] = EOS;
	getNextLenaHeader(&gender, DOB, d, mediaFName, version);
	strcpy(mediaFName, oldfname);
	s = strchr(mediaFName, '.');
	if (s != NULL)
		*s = EOS;
	if (gender == 'm' || gender == 'M')
		strcpy(genderS, "male");
	else if (gender == 'f' || gender == 'F')
		strcpy(genderS, "female");
	else
		genderS[0] = EOS;

	if (DOB[0] != EOS) {
		DOB_clan[0] = DOB[8];
		DOB_clan[1] = DOB[9];
		b[0] = atoi(DOB+8);
		DOB_clan[2] = '-';
		month = atoi(DOB+5);
		b[1] = month;
		if (month < 1 || month > 12)
			strcpy(DOB_clan+3, "???");
		else
			strcpy(DOB_clan+3, MonthNames[month-1]);
		DOB_clan[6] = '-';
		b[2] = atoi(DOB);
		DOB_clan[7] = DOB[0];
		DOB_clan[8] = DOB[1];
		DOB_clan[9] = DOB[2];
		DOB_clan[10] = DOB[3];
		DOB_clan[11] = EOS;
		comuteAge(b, d, ageS, fNameAgeS);
	}
	if (DOB[0] == EOS) {
		freeXML_Elements();
		fprintf(stderr, "Can't find Date of Birth information.");
		cutt_exit(0);
	}
	if (d[0] == 0 && d[1] == 0 && d[2] == 0) {
		freeXML_Elements();
		fprintf(stderr, "Can't find Date of transcript information.");
		cutt_exit(0);
	}
	if (ageS[0] == EOS) {
		freeXML_Elements();
		fprintf(stderr, "Can't compute Age of the child.");
		cutt_exit(0);
	}
	FileName1[0] = EOS;
	if (isRecursive) {
		strcpy(DirPathName, oldfname);
		s = strrchr(DirPathName, PATHDELIMCHR);
		if (s != NULL) {
			*s = EOS;
			strcat(FileName1, DirPathName);
			strcat(FileName1, PATHDELIMSTR);
			s = strrchr(DirPathName, PATHDELIMCHR);
			if (s != NULL) {
				strcat(FileName1, s+1);
				strcat(FileName1, "_");
			}
		}
	} else {
		strcpy(DirPathName, wd_dir);
		len = strlen(DirPathName) - 1;
		if  (len >= 0) {
			if (DirPathName[len] == PATHDELIMCHR)
				DirPathName[len] = EOS;
		}
		s = strrchr(DirPathName, PATHDELIMCHR);
		if (s != NULL) {
			strcat(FileName1, s+1);
			strcat(FileName1, "_");
		}
	}
	if (s == NULL) {
		strcat(FileName1, "CAN'T_FIND_DIR_");
	}
	tFPout = fpout;
	strcat(FileName1, fNameAgeS);
	strcat(FileName1, ".its");
	oldfname = FileName1;
	parsfname(FileName1,newfname,GExt);
	if ((fpout=Lena2chat_openfile(FileName1,newfname)) == NULL) {
		fprintf(stderr,"Can't create file \"%s\", perhaps it is opened by another application\n",newfname);
	} else {
		fprintf(fpout, "%s\n", UTF8HEADER);
		fprintf(fpout, "@Begin\n");
		fprintf(fpout, "@Languages:\teng\n");
		fprintf(fpout, "@Participants:\tSIL Silence LENA, MAN Male_Adult_Near Male,  MAF Male_Adult_Far Male, FAN\n");
		fprintf(fpout, "\tFemale_Adult_Near Female, FAF Female_Adult_Far Female, CHN Key_Child_Clear Target_Child, CHF\n");
		fprintf(fpout, "\tKey_Child_Unclear Target_Child, CXN Other_Child_Near Child, CXF Other_Child_Far Child, NON\n");
		fprintf(fpout, "\tNoise_Near LENA, NOF Noise_Far LENA, OLN Overlap_Near LENA, OLF Overlap_Far LENA, TVN\n");
		fprintf(fpout, "\tElectronic_Sound_Near Media, TVF Electronic_Sound_Far Media\n");
		fprintf(fpout, "@Options:\tmulti\n");
		fprintf(fpout, "@ID:\teng|LENA|SIL|||||LENA||Silence|\n");
		fprintf(fpout, "@ID:\teng|LENA|MAN|||||Male||Male_Adult_Near|\n");
		fprintf(fpout, "@ID:\teng|LENA|MAF|||||Male||Male_Adult_Far|\n");
		fprintf(fpout, "@ID:\teng|LENA|FAN|||||Female||Female_Adult_Near|\n");
		fprintf(fpout, "@ID:\teng|LENA|FAF|||||Female||Female_Adult_Far|\n");
		fprintf(fpout, "@ID:\teng|LENA|CHN|%s|%s|||Target_Child||Key_Child_Clear|\n", ageS, genderS);
		fprintf(fpout, "@ID:\teng|LENA|CHF|%s|%s|||Target_Child||Key_Child_Unclear|\n", ageS, genderS);
		fprintf(fpout, "@ID:\teng|LENA|CXN|||||Child||Other_Child_Near|\n");
		fprintf(fpout, "@ID:\teng|LENA|CXF|||||Child||Other_Child_Far|\n");
		fprintf(fpout, "@ID:\teng|LENA|NON|||||LENA||Noise_Near|\n");
		fprintf(fpout, "@ID:\teng|LENA|NOF|||||LENA||Noise_Far|\n");
		fprintf(fpout, "@ID:\teng|LENA|OLN|||||LENA||Overlap_Near|\n");
		fprintf(fpout, "@ID:\teng|LENA|OLF|||||LENA||Overlap_Far|\n");
		fprintf(fpout, "@ID:\teng|LENA|TVN|||||Media||Electronic_Sound_Near|\n");
		fprintf(fpout, "@ID:\teng|LENA|TVF|||||Media||Electronic_Sound_Far|\n");
		fprintf(fpout, "@Birth of CHN:\t%s\n", DOB_clan);
		fprintf(fpout, "@Birth of CHF:\t%s\n", DOB_clan);
		strcpy(FileName1, newfname);
		s = strrchr(FileName1, '.');
		if (s != NULL)
			*s = EOS;
		s = strrchr(FileName1, '.');
		if (s != NULL) {
			if (strcmp(s, ".lena") == 0)
				*s = EOS;
		}
		s = strrchr(FileName1, PATHDELIMCHR);
		if (s != NULL)
			fprintf(fpout, "%s\t%s, %s\n", MEDIAHEADER, s+1, "audio");
		else
			fprintf(fpout, "%s\t%s, %s\n", MEDIAHEADER, FileName1, "audio");
		fprintf(fpout, "@Comment:\told mediafile name \"%s\"\n", mediaFName);
		fprintf(fpout, "@Comment:\tLENA %s\n", version);

		getNextLenaTier();
		fprintf(stderr, "\r        \r");
//		fprintf(stderr, "\n");
		fprintf(fpout, "@End\n");
		fclose(fpout);
		fpout = tFPout;
		fprintf(stderr,"Output file <%s>\n",newfname);

		if (isRecursive) {
			strcpy(DirPathName, oldfname);
			s = strrchr(DirPathName, PATHDELIMCHR);
			if (s != NULL) {
				*s = EOS;
			}
		} else {
			strcpy(DirPathName, wd_dir);
			len = strlen(DirPathName) - 1;
			if  (len >= 0) {
				if (DirPathName[len] == PATHDELIMCHR)
					DirPathName[len] = EOS;
			}
		}
		strcpy(FileName2, mediaFName);
		len = strlen(FileName2);
		strcat(FileName2, ".wav");
		if (access(FileName2, 0)) {
			FileName2[len] = EOS;
			strcat(FileName2, ".mp3");
			isMP3 = TRUE;
		} else
			isMP3 = FALSE;
		strcpy(newMediaFname, FileName1);
		if (isMP3)
			strcat(newMediaFname, ".mp3");
		else
			strcat(newMediaFname, ".wav");
		rename(FileName2, newMediaFname);
	}
	freeXML_Elements();
}
