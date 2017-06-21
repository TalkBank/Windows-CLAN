/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "cu.h"
#include "cutt-xml.h"
#ifdef _WIN32
	#include <TextUtils.h>
#endif

#if !defined(UNX)
#define _main elan2chat_main
#define call elan2chat_call
#define getflag elan2chat_getflag
#define init elan2chat_init
#define usage elan2chat_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

extern struct tier *defheadtier;
extern char OverWriteFile;
extern char cutt_isMultiFound;

#define IDSTRLEN 128
#define ALLCHATTIERS struct AllChatTiers
struct AllChatTiers {
	char isWrap;
	char ID[IDSTRLEN+1];
	char *sp;
	char *line;
	long beg, end;
	ALLCHATTIERS *depTiers;
	ALLCHATTIERS *nextTier;
} ;

static char mediaFName[FILENAME_MAX+2];
static char isMultiBullets;
static ALLCHATTIERS *RootTiers;
static long *Times_table, Times_index;
static Element *Elan_stack[30];

void usage() {
	printf("convert Elan XML files to CHAT files\n");
	printf("Usage: elan2chat [b %s] filename(s)\n",mainflgs());
	puts("+b: Specify that multiple bullets per line (default only one bullet per line).");
	mainusage(TRUE);
}

void init(char s) {
	if (s) {
		OverWriteFile = TRUE;
		AddCEXExtension = ".cha";
		stout = FALSE;
		onlydata = 1;
		RootTiers = NULL;
		CurrentElem = NULL;
		Times_table = NULL;
		Times_index = 0L;
		isMultiBullets = FALSE;
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
	CLAN_PROG_NUM = ELAN2CHAT;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
}

void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'b':
			isMultiBullets = TRUE;
			no_arg_option(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static ALLCHATTIERS *freeTiers(ALLCHATTIERS *p) {
	ALLCHATTIERS *t;

	while (p != NULL) {
		t = p;
		p = p->nextTier;
		if (t->depTiers != NULL)
			freeTiers(t->depTiers);
		if (t->sp != NULL)
			free(t->sp);
		if (t->line != NULL)
			free(t->line);
		free(t);
	}
	return(NULL);
}

static void Elan_fillNT(ALLCHATTIERS *nt, const char *ID, long beg, long end, const char *sp, const char *line) {
	if (beg != 0L || end != 0L)
		sprintf(templineC3, " %c%ld_%ld%c", HIDEN_C, beg, end, HIDEN_C);
	else
		templineC3[0] = EOS;

	nt->depTiers = NULL;
	strcpy(nt->ID, ID);
	nt->isWrap = TRUE;
	nt->sp = (char *)malloc(strlen(sp)+1);
	if (nt->sp == NULL) out_of_mem();
	strcpy(nt->sp, sp);
	nt->line = (char *)malloc(strlen(line)+strlen(templineC3)+1);
	if (nt->line == NULL) out_of_mem();
	strcpy(nt->line, line);
	if (templineC3[0] != EOS)
		strcat(nt->line, templineC3);
	nt->beg = beg;
	nt->end = end;
}

static void Elan_refillNTline(ALLCHATTIERS *nt, char *ID, long beg, long end, char *line) {
	char *oline;

	oline = nt->line;

	if (beg != 0L || end != 0L)
		sprintf(templineC3, " %c%ld_%ld%c", HIDEN_C, beg, end, HIDEN_C);
	else
		templineC3[0] = EOS;

	if (isMultiBullets) {
		nt->isWrap = TRUE;
	} else {
		nt->isWrap = FALSE;
	}
	if (strlen(nt->ID)+strlen(ID) < IDSTRLEN)
		strcat(nt->ID, ID);
	nt->end = end;
	nt->line = (char *)malloc(strlen(oline)+strlen(line)+strlen(templineC3)+3);
	if (nt->line == NULL) out_of_mem();
	strcpy(nt->line, oline);
	strcat(nt->line, "\n\t");
	strcat(nt->line, line);
	if (templineC3[0] != EOS)
		strcat(nt->line, templineC3);
	free(oline);
}

static ALLCHATTIERS *Elan_addDepTiers(ALLCHATTIERS *depTiers, char *ID, long beg, long end, char *sp, char *line) {
	ALLCHATTIERS *nt, *tnt;

	if (depTiers == NULL) {
		if ((depTiers=NEW(ALLCHATTIERS)) == NULL)
			out_of_mem();
		nt = depTiers;
		nt->nextTier = NULL;
	} else {
		tnt= depTiers;
		nt = depTiers;
		while (nt != NULL) {
			if (!strcmp(nt->sp, sp)) {
				Elan_refillNTline(nt, ID, beg, end, line);
				return(depTiers);
			}
			if (beg < nt->beg)
				break;
			tnt = nt;
			nt = nt->nextTier;
		}
		if (nt == NULL) {
			tnt->nextTier = NEW(ALLCHATTIERS);
			if (tnt->nextTier == NULL)
				out_of_mem();
			nt = tnt->nextTier;
			nt->nextTier = NULL;
		} else if (nt == depTiers) {
			depTiers = NEW(ALLCHATTIERS);
			if (depTiers == NULL)
				out_of_mem();
			depTiers->nextTier = nt;
			nt = depTiers;
		} else {
			nt = NEW(ALLCHATTIERS);
			if (nt == NULL)
				out_of_mem();
			nt->nextTier = tnt->nextTier;
			tnt->nextTier = nt;
		}
	}

	Elan_fillNT(nt, ID, beg, end, sp, line);
	return(depTiers);
}

static ALLCHATTIERS *Elan_insertNT(ALLCHATTIERS *nt, ALLCHATTIERS *tnt) {
	if (nt == RootTiers) {
		RootTiers = NEW(ALLCHATTIERS);
		if (RootTiers == NULL)
			out_of_mem();
		RootTiers->nextTier = nt;
		nt = RootTiers;
	} else {
		nt = NEW(ALLCHATTIERS);
		if (nt == NULL)
			out_of_mem();
		nt->nextTier = tnt->nextTier;
		tnt->nextTier = nt;
	}
	return(nt);
}

static char Elan_isUttDel(char *line) {
	char bullet;
	long i;

/* 2010-01-20
	if (isMultiBullets)
		return(FALSE);
*/
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

static char Elan_isPostCodes(char *line) {
	char bullet;
	int  sq;
	long i;

	sq = 0;
	bullet = FALSE;
	i = strlen(line) - 1;
	for (; i >= 0L && (isSpace(line[i]) || isdigit(line[i]) || line[i] == '#' || line[i] == '_' || bullet || sq); i--) {
		if (line[i] == HIDEN_C)
			bullet = !bullet;
		else if (line[i] == ']')
			sq++;
		else if (line[i] == '[') {
			sq--;
			if (line[i+1] != '+')
				return(FALSE);
		}
	}
	if (i < 0L)
		return(TRUE);
	else
		return(FALSE);
}

static char isFoundBegOverlap(char *line) {
	for (; *line != EOS; line++) {
		if (UTF8_IS_LEAD((unsigned char)*line) && *line == (char)0xe2 && *(line+1) == (char)0x8c) {
			if (*(line+2) == (char)0x88) // raised [
				return(TRUE);
			if (*(line+2) == (char)0x8a) // lowered [
				break;
		}
	}

	return(FALSE);
}

static char Elan_isLineJustPause(char *line) {
	int i;

	for (i=0; isSpace(line[i]); i++) ;
	if (uS.isRightChar(line,i,'(',&dFnt,MBF) && uS.isPause(line,i,NULL,&i)) {
		if (uS.isRightChar(line,i,')',&dFnt,MBF)) {
			for (i++; isSpace(line[i]); i++) ;
			if (line[i] == EOS || line[i] == '\n')
				return(TRUE);
		}
	}
	return(FALSE);
}

static char isIDMatch(char *ID, char *refID) {
	for (; *ID != EOS; ID++) {
		if (uS.partwcmp(ID, refID))
			return(TRUE);
	}
	return(FALSE);
}

static void Elan_addToTiers(char *ID, char *refID, long beg, long end, char *sp, char *refSp, char *line) {
	char isRefIDMatch, isPostCodeFound, isJustPause;
	ALLCHATTIERS *nt, *tnt;

	if (RootTiers == NULL) {
		if ((RootTiers=NEW(ALLCHATTIERS)) == NULL)
			out_of_mem();
		nt = RootTiers;
		nt->nextTier = NULL;
	} else {
		isRefIDMatch = FALSE;
		isJustPause = Elan_isLineJustPause(line);
		isPostCodeFound = Elan_isPostCodes(line);
		tnt= RootTiers;
		nt = RootTiers;
		while (nt != NULL) {
			if (sp[0] == '%' && refID[0] != EOS && isIDMatch(nt->ID, refID)) {
				isRefIDMatch = TRUE;
				nt->depTiers = Elan_addDepTiers(nt->depTiers, ID, beg, end, sp, line);
				return;
			} else if (sp[0] == '%' && refID[0] == EOS) {
				if (beg >= nt->beg && beg < nt->end && uS.partcmp(refSp, nt->sp, FALSE, TRUE)) {
					nt->depTiers = Elan_addDepTiers(nt->depTiers, ID, beg, end, sp, line);
					return;
				} else if (end-beg < 3 && beg >= nt->beg && beg <= nt->end && uS.partcmp(refSp, nt->sp, FALSE, TRUE)) {
					nt->depTiers = Elan_addDepTiers(nt->depTiers, ID, beg, end, sp, line);
					return;
				} else if (beg < nt->beg) {
					nt = Elan_insertNT(nt, tnt);
					Elan_fillNT(nt, "0", beg, end, refSp, "0.");
					nt->depTiers = Elan_addDepTiers(nt->depTiers, ID, beg, end, sp, line);
					return;
				}
			} else if (sp[0] == '*') {
				if (beg>=nt->end && beg<=nt->end+50 && !strcmp(sp,nt->sp) && (!Elan_isUttDel(nt->line) || isJustPause)) {
					Elan_refillNTline(nt, ID, beg, end, line);
					return;
				} else if (beg == nt->end && !strcmp(sp, nt->sp) && isPostCodeFound) {
					Elan_refillNTline(nt, ID, beg, end, line);
					return;
				} else if (beg == nt->beg && isFoundBegOverlap(line)) {
					break;
				} else if (beg < nt->beg)
					break;
			}
			tnt = nt;
			nt = nt->nextTier;
		}
		if (nt == NULL) {
			if (!isRefIDMatch && sp[0] == '%' && refID[0] != EOS) {

			}
			tnt->nextTier = NEW(ALLCHATTIERS);
			if (tnt->nextTier == NULL)
				out_of_mem();
			nt = tnt->nextTier;
			nt->nextTier = NULL;
			if (sp[0] == '%' && refID[0] == EOS) {
				Elan_fillNT(nt, "0", beg, end, refSp, "0.");
				nt->depTiers = Elan_addDepTiers(nt->depTiers, ID, beg, end, sp, line);
				return;
			}
		} else 
			nt = Elan_insertNT(nt, tnt);
	}

	Elan_fillNT(nt, ID, beg, end, sp, line);
}

static char Elan_isOverlapFound(char *line, char ovChar) {
	long i;

	for (i=0L; line[i] != EOS; i++) {
		if (line[i] == (char)0xe2 && line[i+1] == (char)0x8c && line[i+2] == (char)ovChar)
			return(TRUE);
	}
	return(FALSE);
}

static void Elan_finalTimeSort(void) {
	char isWrongOverlap;
	ALLCHATTIERS *nt, *prev_nt, *prev_prev_nt;

	nt = RootTiers;
	prev_nt = nt;
	prev_prev_nt = nt;
	while (nt != NULL) {
		if (prev_nt != nt) {
			if (nt->beg == prev_nt->beg && nt->end < prev_nt->end) {
				prev_nt->nextTier = nt->nextTier;
				nt->nextTier = prev_nt;
				if (prev_prev_nt == RootTiers) {
					RootTiers = nt;
					prev_prev_nt = RootTiers;
				} else
					prev_prev_nt->nextTier = nt;
				prev_nt = prev_prev_nt;
				nt = prev_nt->nextTier;
			} else if (nt->beg == prev_nt->beg && nt->end == prev_nt->end) {
				isWrongOverlap = 0;
				if (Elan_isOverlapFound(nt->line, (char)0x88) && !Elan_isOverlapFound(nt->line, (char)0x8a))
					isWrongOverlap++;
				if (isWrongOverlap == 1) {
					if (Elan_isOverlapFound(prev_nt->line, (char)0x8a) && !Elan_isOverlapFound(prev_nt->line, (char)0x88))
						isWrongOverlap++;
				}
				if (isWrongOverlap == 2) {
					prev_nt->nextTier = nt->nextTier;
					nt->nextTier = prev_nt;
					if (prev_prev_nt == RootTiers) {
						RootTiers = nt;
						prev_prev_nt = RootTiers;
					} else
						prev_prev_nt->nextTier = nt;
					prev_nt = prev_prev_nt;
					nt = prev_nt->nextTier;
					if (nt == NULL)
						break;
				}
			}
		}
		prev_prev_nt = prev_nt;
		prev_nt = nt;
		nt = nt->nextTier;
	}
}

static void Elan_printOutTiers(ALLCHATTIERS *p) {
	while (p != NULL) {
		printout(p->sp, p->line, NULL, NULL, p->isWrap);
		if (p->depTiers != NULL)
			Elan_printOutTiers(p->depTiers);
		p = p->nextTier;
	}
}

static void Elan_makeText(char *line) {
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

/* Elan-EAF Begin **************************************************************** */
static void sortElanSpAndDepTiers(void) {
	char firstDepTierFound;
	int  lStackIndex;
	Attributes *att;
	Element *nt, *tnt, *firstTier;
	
	if (CurrentElem == NULL)
		return;
	
	firstTier = NULL;
	firstDepTierFound = FALSE;
	lStackIndex = -1;
	nt = CurrentElem;
	tnt = nt;
	do {
		if (nt != NULL && !strcmp(nt->name, "ANNOTATION_DOCUMENT") && nt->next == NULL && lStackIndex == -1) {
			nt = nt->data;
			lStackIndex++;
			if (nt == NULL)
				return;
		}
		if (!strcmp(nt->name, "TIER") && lStackIndex == 0) {
			if (firstTier == NULL)
				firstTier = tnt;
			for (att=nt->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "TIER_ID")) {
					if (att->value[0] == '%')
						firstDepTierFound = TRUE;
					else if (firstDepTierFound && att->value[0] == '*' && tnt != NULL) {
						tnt->next = nt->next;
						nt->next = firstTier->next;
						firstTier->next = nt;
						nt = tnt;
					}
					break;
				}
			}
		}
		tnt = nt;
		nt = nt->next;
	} while (nt != NULL);
}

static void setElanTimeOrderElem(void) {
	int  lStackIndex;
	long index;
	long timeValue = 0L;
	Attributes *att;
	Element *TimeSlots, *Time_Order = NULL;
	
	if (CurrentElem == NULL)
		return;
	lStackIndex = -1;
	Time_Order = CurrentElem;
	do {
		if (Time_Order != NULL && !strcmp(Time_Order->name, "ANNOTATION_DOCUMENT") && Time_Order->next == NULL && lStackIndex == -1) {
			Time_Order = Time_Order->data;
			lStackIndex++;
		} else if (Time_Order != NULL)
			Time_Order = Time_Order->next;
		
		if (Time_Order == NULL) {
			return;
		}
		if (!strcmp(Time_Order->name, "TIME_ORDER") && lStackIndex == 0) {
			Times_index = 0L;
			TimeSlots = Time_Order->data;
			while (TimeSlots != NULL) {
				for (att=TimeSlots->atts; att != NULL; att=att->next) {
					if (!strcmp(att->name, "TIME_SLOT_ID")) {
						index = atol(att->value+2);
						if (index > Times_index)
							Times_index = index;
						break;
					}
				}
				TimeSlots = TimeSlots->next;
			}
			
			if (Times_index > 0) {
				Times_index++;
				Times_table = (long *)malloc((size_t)Times_index * (size_t)sizeof(long));
			}
			
			if (Times_table == NULL)
				return;
			
			TimeSlots = Time_Order->data;
			while (TimeSlots != NULL) {
				index = 0L;
				timeValue = 0L;
				for (att=TimeSlots->atts; att != NULL; att=att->next) {
					if (!strcmp(att->name, "TIME_SLOT_ID")) {
						index = atol(att->value+2);
					} else if (!strcmp(att->name, "TIME_VALUE")) {
						timeValue = atol(att->value);
					} 
				}
				Times_table[index] = timeValue;
				TimeSlots = TimeSlots->next;
			}
			
			freeElements(Time_Order->data);
			Time_Order->data = NULL;
			return;
		}
	} while (1);
}

static long getElanTimeValue(char *TimeSlotIDSt) {
	long index;
	
	if (Times_table == NULL)
		return(0L);
	
	index = atol(TimeSlotIDSt+2);
	if (index >= 0L && index < Times_index)
		return(Times_table[index]);
	else
		return(0L);
}

static void getCurrentElanData(Element *CurrentElem, UTTER *utterance) {
	Element *data;
	
	if (CurrentElem->cData != NULL) {
		strncpy(utterance->line, CurrentElem->cData, UTTLINELEN);
		utterance->line[UTTLINELEN-1] = EOS;
	} else if (CurrentElem->data != NULL) {
		utterance->line[0] = EOS;
		for (data=CurrentElem->data; data != NULL; data=data->next) {
			if (!strcmp(data->name, "CONST")) {
				strncpy(utterance->line, data->cData, UTTLINELEN);
				utterance->line[UTTLINELEN-1] = EOS;
			}
		}
	} else
		utterance->line[0] = EOS;
}

static char getElanAnnotation(Element *cElem, UTTER *utterance, long *beg, long *end, char *ID, char *refID) {
	Attributes *att;
	
	if (cElem == NULL)
		return(FALSE);
	
	ID[0]    = EOS;
	refID[0] = EOS;
	*beg = 0L;
	*end = 0L;
	
	for (att=cElem->atts; att != NULL; att=att->next) {
		if (!strcmp(att->name, "ANNOTATION_ID")) {
			strcpy(ID, att->value);
		} else if (!strcmp(att->name, "ANNOTATION_REF")) {
			strcpy(refID, att->value);
		} else if (!strcmp(att->name, "TIME_SLOT_REF1")) {
			*beg = getElanTimeValue(att->value);
		} else if (!strcmp(att->name, "TIME_SLOT_REF2")) {
			*end = getElanTimeValue(att->value);
		} 
	}
	
	cElem = cElem->data;
	utterance->line[0] = EOS;
	if (cElem == NULL || strcmp(cElem->name, "ANNOTATION_VALUE"))
		return(FALSE);
	getCurrentElanData(cElem, utterance);
	return(TRUE);
}

static char getNextElanTier(UTTER *utterance, long *beg, long *end, char *ID, char *refID) {
	Attributes *att;
	char *p;
	char tier_id[SPEAKERLEN+2];
	char parent_ref[SPEAKERLEN+2];
	
	if (CurrentElem == NULL)
		return(FALSE);
	
	do {
		if (CurrentElem != NULL && !strcmp(CurrentElem->name, "ANNOTATION_DOCUMENT") && CurrentElem->next == NULL)
			CurrentElem = CurrentElem->data;
		else if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;
		
		if (CurrentElem == NULL) {
			if (stackIndex >= 0) {
				CurrentElem = Elan_stack[stackIndex];
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL)
					return(FALSE);
			} else
				return(FALSE);
		}
		
		if (!strcmp(CurrentElem->name, "TIER") && CurrentElem->data != NULL) {
			if (stackIndex < 29) {
				tier_id[0] = EOS;
				parent_ref[0] = EOS;
				for (att=CurrentElem->atts; att != NULL; att=att->next) {
					if (!strcmp(att->name, "TIER_ID")) {
						strncpy(tier_id, att->value, SPEAKERLEN);
						tier_id[SPEAKERLEN-1] = EOS;
					} else if (!strcmp(att->name, "PARENT_REF")) {
						strncpy(parent_ref, att->value, SPEAKERLEN);
						parent_ref[SPEAKERLEN-1] = EOS;
					}
				}
				if (parent_ref[0] == EOS) {
					p = strchr(tier_id, '@');
					if (p != NULL)
						strcpy(parent_ref, p+1);
				}
				utterance->speaker[0] = EOS;
				if (parent_ref[0] == EOS) {
					if (tier_id[0] != '*')
						strcat(utterance->speaker, "*");
					strcat(utterance->speaker, tier_id);
				} else {
					if (tier_id[0] != '%')
						strcat(utterance->speaker, "%");
					p = strchr(tier_id, '@');
					if (p != NULL) {
						if (parent_ref[0] == '*' && !uS.mStricmp(p+1, parent_ref+1))
							parent_ref[0] = EOS;
						else if (!uS.mStricmp(p+1, parent_ref))
							parent_ref[0] = EOS;
						else
							*p = '_';
					}
					strcat(utterance->speaker, tier_id);
					if (parent_ref[0] != EOS) {
						strcat(utterance->speaker, "@");
						if (parent_ref[0] == '*')
							strcat(utterance->speaker, parent_ref+1);
						else
							strcat(utterance->speaker, parent_ref);
					}
				}
				stackIndex++;
				Elan_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				if (Times_table != NULL)
					free(Times_table);
				Times_table = NULL;
				Times_index = 0L;
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "ANNOTATION") && stackIndex == 0) {
			if (getElanAnnotation(CurrentElem->data, utterance, beg, end, ID, refID))
				return(TRUE);
		}
	} while (1);
	return(FALSE);
}

static char getNextElanHeader(UTTER *utterance, char *mediaFName) {
	Attributes *att;
	
	if (CurrentElem == NULL)
		return(FALSE);
	
	do {
		if (CurrentElem != NULL && !strcmp(CurrentElem->name, "ANNOTATION_DOCUMENT") && CurrentElem->next == NULL)
			CurrentElem = CurrentElem->data;
		else if (CurrentElem != NULL)
			CurrentElem = CurrentElem->next;
		
		if (CurrentElem == NULL) {
			if (stackIndex >= 0) {
				CurrentElem = Elan_stack[stackIndex];
				stackIndex--;
				freeElements(CurrentElem->data);
				CurrentElem->data = NULL;
				CurrentElem = CurrentElem->next;
				if (CurrentElem == NULL)
					return(FALSE);
			} else
				return(FALSE);
		}
		
		if (!strcmp(CurrentElem->name, "HEADER") && CurrentElem->data != NULL) {
			if (stackIndex < 29) {
				stackIndex++;
				Elan_stack[stackIndex] = CurrentElem;
				CurrentElem = CurrentElem->data;
			} else {
				fprintf(stderr, "Internal error exceeded stack size of %d\n", stackIndex);
				freeXML_Elements();
				if (Times_table != NULL)
					free(Times_table);
				Times_table = NULL;
				Times_index = 0L;
				cutt_exit(0);
			}
		}
		if (!strcmp(CurrentElem->name, "PROPERTY") && stackIndex == 0) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "NAME")) {
					strncpy(utterance->speaker, att->value, SPEAKERLEN);
					utterance->speaker[SPEAKERLEN-1] = EOS;
					if (*utterance->speaker == '@') {
						getCurrentElanData(CurrentElem, utterance);
						return(TRUE);
					}
				}
			}
		}
		if (!strcmp(CurrentElem->name, "MEDIA_DESCRIPTOR") && stackIndex == 0) {
			for (att=CurrentElem->atts; att != NULL; att=att->next) {
				if (!strcmp(att->name, "MEDIA_URL")) {
					strncpy(mediaFName, att->value, FILENAME_MAX);
					mediaFName[FILENAME_MAX] = EOS;
				}
			}
		}
	} while (1);
	return(FALSE);
}
/* Elan-EAF End ****************************************************************** */

void call() {		/* this function is self-explanatory */
	const char *mediaType;
	char ID[IDSTRLEN+1], refID[IDSTRLEN+1], refSp[SPEAKERLEN], *p;
	char isBeginPrinted, isOptionFound, isFirstHeaderFound;
	long len;
	long beg, end;
	long lineno = 0L, tlineno = 0L;

	mediaFName[0] = EOS;
	BuildXMLTree(fpin);

	isBeginPrinted = FALSE;
	isOptionFound = FALSE;
	isFirstHeaderFound = FALSE;
	while (getNextElanHeader(utterance, mediaFName)) {
		uS.remblanks(utterance->speaker);
		len = strlen(utterance->speaker) - 1;
		if (utterance->line[0] != EOS) {
			if (utterance->speaker[len] != ':')
				strcat(utterance->speaker, ":");
		} else {
			if (utterance->speaker[len] == ':')
				utterance->speaker[len] = EOS;
		}
		if (uS.partcmp(utterance->speaker, MEDIAHEADER, FALSE, FALSE))
			continue;
		else if (uS.partcmp(utterance->speaker, "@Languages:", FALSE, FALSE)) {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			continue;
		} else if (uS.partcmp(utterance->speaker, "@Participants:", FALSE, FALSE)) {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			continue;
		} else if (uS.partcmp(utterance->speaker, "@ID:", FALSE, FALSE)) {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			continue;
		} else if (uS.partcmp(utterance->speaker, "@Options:", FALSE, FALSE)) {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			uS.remblanks(utterance->line);
			if (isMultiBullets) {
				for (beg=0; utterance->line[beg] != EOS; beg++) {
					if (!strncmp(utterance->line+beg, "multi", 5))
						break;
				}
				if (utterance->line[beg] == EOS)
					strcat(utterance->line, ", multi");
			}
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			isOptionFound = TRUE;
			continue;
		} else if (!isFirstHeaderFound && uS.partcmp(utterance->speaker, FONTHEADER, FALSE, FALSE)) {
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			isBeginPrinted = TRUE;
			continue;
		} else {
			if (!isBeginPrinted) {
				fprintf(fpout, "%s\n", UTF8HEADER);
				fprintf(fpout, "@Begin\n");
			}
			Elan_makeText(utterance->line);
			printout(utterance->speaker,utterance->line,NULL,NULL,TRUE);
		}
		isFirstHeaderFound = TRUE;
	}
	if (!isBeginPrinted) {
		fprintf(fpout, "%s\n", UTF8HEADER);
		fprintf(fpout, "@Begin\n");
	}
	if (isMultiBullets && !isOptionFound)
		fprintf(fpout, "@Options:\tmulti\n");
	p = strrchr(mediaFName, '/');
	if (p != NULL)
		strcpy(mediaFName, p+1);
	p = strrchr(mediaFName, '.');
	if (p != NULL) {
		if (uS.mStricmp(p, ".wav") == 0 || uS.mStricmp(p, ".aif") == 0 || uS.mStricmp(p, ".aiff") == 0)
			mediaType = "audio";
		else
			mediaType = "video";
		*p = EOS;
	} else
		mediaType = "video";
	fprintf(fpout, "%s\t%s, %s\n", MEDIAHEADER, mediaFName, mediaType);

	ResetXMLTree();
	sortElanSpAndDepTiers();
	setElanTimeOrderElem();

	refSp[0] = EOS;
	while (getNextElanTier(utterance, &beg, &end, ID, refID)) {
		if (lineno > tlineno) {
			tlineno = lineno + 200;
			fprintf(stderr,"\r%ld ",lineno);
		}
		lineno++;
		uS.remblanks(utterance->speaker);
		if (utterance->speaker[0] == '%') {
			p = strchr(utterance->speaker, '@');
			if (p != NULL) {
				strcpy(refSp, "*");
				strcat(refSp, p+1);
				strcat(refSp, ":");
				*p = EOS;
			}
		} else
			refSp[0] = EOS;
		len = strlen(utterance->speaker);
		if (utterance->speaker[len-1] != ':')
			strcat(utterance->speaker, ":");
		Elan_makeText(utterance->line);
		Elan_addToTiers(ID, refID, beg, end, utterance->speaker, refSp, utterance->line);
//		fprintf(fpout, "ID=%s, IDRef=%s; (%ld-%ld)\n", ID, refID, beg, end);
//		fprintf(fpout, "%s:\t%s\n", utterance->speaker, utterance->line);
	}
	fprintf(stderr, "\r	  \r");
	cutt_isMultiFound = isMultiBullets;
	Elan_finalTimeSort();
	Elan_printOutTiers(RootTiers);
	fprintf(fpout, "@End\n");
	freeXML_Elements();
	if (Times_table != NULL)
		free(Times_table);
	Times_table = NULL;
	Times_index = 0L;
	RootTiers = freeTiers(RootTiers);
}
