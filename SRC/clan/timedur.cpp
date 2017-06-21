/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 1

#include "cu.h"
#include "check.h"

#if !defined(UNX)
#define _main time_main
#define call time_call
#define getflag time_getflag
#define init time_init
#define usage time_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

extern char OverWriteFile;

struct sp_list {
	char *sp;
	char *IDs;
	struct sp_list *nextSpCode;
} ;

struct sp_stat {
	char prev_sp_name[SPEAKERLEN];
	char sp_name[SPEAKERLEN];
	char *IDs;
	unsigned long words;
	unsigned long utts;
	unsigned long lastEnd;
	unsigned long total;
	struct sp_stat *nextsp;
} ;

static char isSpeakerNameGiven;
static char timedur_ftime, timedur_ftime1;
static long ln;
static struct sp_list *spRoot;
static struct sp_stat *RootStats;

/*  ********************************************************* */

void usage() {
	puts("TIMEDUR ");
	printf("Usage: timedur [%s] filename(s)\n", mainflgs());
	puts("+d :  outputs default results in SPREADSHEET format");
	puts("+d1:  outputs ratio of words and utterances over time duration");
	puts("+d10: outputs above, +d1, results in SPREADSHEET format");
	mainusage(TRUE);
}

static void FreeSpList(void) {
	struct sp_list *p;
	
	while (spRoot != NULL) {
		p = spRoot;
		spRoot = spRoot->nextSpCode;
		if (p->sp != NULL)
			free(p->sp);
		if (p->IDs != NULL)
			free(p->IDs);
		free(p);
	}
	spRoot = NULL;
}

static void FreeSpStats(void) {
	struct sp_stat *p;

	while (RootStats != NULL) {
		p = RootStats;
		RootStats = RootStats->nextsp;
		if (p->IDs != NULL)
			free(p->IDs);
		free(p);
	}
	RootStats = NULL;
}

void init(char first) {
	if (first) {
		spRoot = NULL;
		RootStats = NULL;
		onlydata = 0;
		timedur_ftime = TRUE;
		timedur_ftime1 = TRUE;
		LocalTierSelect = TRUE;
		OverWriteFile = TRUE;
		isSpeakerNameGiven = FALSE;
		addword('\0','\0',"+xxx");
		addword('\0','\0',"+yyy");
		addword('\0','\0',"+www");
		addword('\0','\0',"+xxx@s*");
		addword('\0','\0',"+yyy@s*");
		addword('\0','\0',"+www@s*");
		addword('\0','\0',"+unk|xxx");
		addword('\0','\0',"+unk|yyy");
		addword('\0','\0',"+*|www");
		addword('\0','\0',"+.");
		addword('\0','\0',"+?");
		addword('\0','\0',"+!");
		maininitwords();
		mor_initwords();
	} else {
		if (timedur_ftime) {
			timedur_ftime = FALSE;
			if (onlydata == 1 || onlydata == 3) {
				if (onlydata == 3 && !isSpeakerNameGiven) {
					fprintf(stderr,"Please specify speaker's tier code with \"+t\" option on command line.\n");
					cutt_exit(0);
				}
				combinput = TRUE;
				if (!f_override)
					stout = FALSE;
				AddCEXExtension = ".xls";
			}
		}
	}
	FreeSpList();
	FreeSpStats();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'd':
			if (*f == EOS || *f == '0') {
				if (onlydata == 0)
					onlydata = 1;
				else if (onlydata == 2)
					onlydata = 3;
			} else if (*f == '1') {
				if (*(f+1) == '0')
					onlydata = 3;
				else if (onlydata == 0)
					onlydata = 2;
				else if (onlydata == 1)
					onlydata = 3;
			} else {
				fprintf(stderr, "The only +d levels allowed are 0-%d.\n", 1);
				cutt_exit(0);
			}
			break;
		case 't':
			if (*f == '@') {
				if (uS.mStrnicmp(f+1, "ID=", 3) == 0 && *(f+4) != EOS) {
					isSpeakerNameGiven = TRUE;
				}
			} else if (*f == '#') {
				isSpeakerNameGiven = TRUE;
			} else if (*f != '%' && *f != '@') {
				isSpeakerNameGiven = TRUE;
			}			
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static struct sp_stat *MkSpStruct(struct sp_stat *p, const char *prevsp, char *thissp, char *IDs) {	
	if (p == NULL) {
		if ((p=NEW(struct sp_stat)) == NULL) {
			fprintf(stderr, "\n\nTIMEDUR: out of memory\n\n");
			FreeSpList();
			FreeSpStats();
			cutt_exit(0);
		}
		RootStats = p;
	} else {
		if ((p->nextsp=NEW(struct sp_stat)) == NULL) {
			fprintf(stderr, "\n\nTIMEDUR: out of memory\n\n");
			FreeSpList();
			FreeSpStats();
			cutt_exit(0);
		}
		p = p->nextsp;
	}
	p->nextsp = NULL;
	if (IDs == NULL)
		p->IDs = NULL;
	else {
		if ((p->IDs=(char *)malloc(strlen(IDs)+1)) == NULL) {
			fprintf(stderr, "\n\nTIMEDUR: out of memory\n\n");
			FreeSpList();
			FreeSpStats();
			cutt_exit(0);
		}
		strcpy(p->IDs, IDs);
	}
	strcpy(p->prev_sp_name, prevsp);
	strcpy(p->sp_name, thissp);
	p->lastEnd = 0L;
	p->total = 0L;
	p->words = 0L;
	p->utts = 0L;
	return(p);
}

static void ProcessStats(char *prevsp, char *thissp, long *prev_stime, long *prev_etime, char *hc) {
	char mediaRes;
	long stime, etime, duration, tDiff;
	struct sp_stat *p;

	if (isdigit(hc[1]))
		mediaRes = getMediaTagInfo(hc, &stime, &etime);
	else
		mediaRes = getOLDMediaTagInfo(hc, NULL, NULL, &stime, &etime);
	if (mediaRes) {
		if (onlydata == 1) {
			fprintf(fpout, "%ld\t%s", ln, thissp);
			for (p=RootStats; p != NULL; p=p->nextsp) {
				if (*p->prev_sp_name == EOS && !strcmp(thissp,p->sp_name)) {
					duration = etime - stime;
					fprintf(fpout, "\t%ld", duration);
					p->total = p->total + duration;
					tDiff = p->lastEnd - stime;
					if (tDiff > 500L) {
						fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
						fprintf(stderr, "Speaker's start time \"%lu\" is less than same speaker's previous end time \"%lu\".\n", stime, p->lastEnd);
						fprintf(stderr, "Time difference is: %lu.\n", tDiff);
					}
					p->lastEnd = etime;
				} else  if ((!strcmp(prevsp,p->prev_sp_name) && !strcmp(thissp,p->sp_name)) || 
							(!strcmp(thissp,p->prev_sp_name) && !strcmp(prevsp,p->sp_name))) {
					fprintf(fpout, "\t%ld", stime-*prev_etime);
				} else {
					fprintf(fpout, "\t");
				}
			}
			fprintf(fpout, "\n");
		} else if (onlydata == 2 || onlydata == 3) {
			for (p=RootStats; p != NULL; p=p->nextsp) {
				if (*p->prev_sp_name == EOS && !strcmp(thissp,p->sp_name)) {
					duration = etime - stime;
					p->total = p->total + (duration / 1000L);
					tDiff = p->lastEnd - stime;
					if (tDiff > 500L) {
						fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
						fprintf(stderr, "Speaker's start time \"%lu\" is less than same speaker's previous end time \"%lu\".\n", stime, p->lastEnd);
						fprintf(stderr, "Time difference is: %lu.\n", tDiff);
					}
					p->lastEnd = etime;
				}
			}
		} else {
			fprintf(fpout, "%3ld %-3s|", ln, thissp);
			for (p=RootStats; p != NULL; p=p->nextsp) {
				if (*p->prev_sp_name == EOS && !strcmp(thissp,p->sp_name)) {
					duration = etime - stime;
					fprintf(fpout, "%6ld |", duration);
					p->total = p->total + duration;
					tDiff = p->lastEnd - stime;
					if (tDiff > 500L) {
						fprintf(stderr,"*** File \"%s\": line %ld.\n", oldfname, ln);
						fprintf(stderr, "Speaker's start time \"%lu\" is less than same speaker's previous end time \"%lu\".\n", stime, p->lastEnd);
						fprintf(stderr, "Time difference is: %lu.\n", tDiff);
					}
					p->lastEnd = etime;
				} else  if ((!strcmp(prevsp,p->prev_sp_name) && !strcmp(thissp,p->sp_name)) || 
							(!strcmp(thissp,p->prev_sp_name) && !strcmp(prevsp,p->sp_name))) {
					fprintf(fpout, "%6ld |", stime-*prev_etime);
				} else {
					fprintf(fpout, "       |");
				}
			}
			fprintf(fpout, "\n");
		}
		*prev_stime = stime;
		*prev_etime = etime;
	} else {
		fprintf(stderr, "Bad media tier format on line %ld\n", lineno);
		fprintf(stderr, "%s%s", utterance->speaker, utterance->line);
		FreeSpList();
		FreeSpStats();
		cutt_exit(0);
	}
}

static struct sp_list *AddSpeakersToList(struct sp_list *root, char *sp, char *IDs) {
	struct sp_list *t;

	uS.uppercasestr(sp, &dFnt, MBF);
	if (root == NULL) {
		root = NEW(struct sp_list);
		t = root;
	} else {
		for (t=root; t->nextSpCode != NULL; t=t->nextSpCode) {
			if (!strcmp(t->sp, sp)) {
				return(root);
			}
		}
		if (!strcmp(t->sp, sp)) {
			return(root);
		}
		t->nextSpCode = NEW(struct sp_list);
		t = t->nextSpCode;
	}
	if (t == NULL) {
		fprintf(stderr, "\n\nTIMEDUR: out of memory\n\n");
		FreeSpList();
		FreeSpStats();
		cutt_exit(0);
	}
	t->nextSpCode = NULL;
	t->IDs = NULL;
	if ((t->sp=(char *)malloc(strlen(sp)+1)) == NULL) {
		fprintf(stderr, "\n\nTIMEDUR: out of memory\n\n");
		FreeSpList();
		FreeSpStats();
		cutt_exit(0);
	}
	strcpy(t->sp, sp);
	if (IDs == NULL || onlydata != 3)
		t->IDs = NULL;
	else {
		if ((t->IDs=(char *)malloc(strlen(IDs)+1)) == NULL) {
			fprintf(stderr, "\n\nTIMEDUR: out of memory\n\n");
			FreeSpList();
			FreeSpStats();
			cutt_exit(0);
		}
		strcpy(t->IDs, IDs);
	}
	return(root);
}

static void spcpy(char *s, char *f) {
	if (*f == '*')
		f++;
	while (*f != ':' && *f != EOS)
		*s++ = *f++;
	*s = EOS;
}

static void MakeSpHeader(void) {
	struct sp_list *tsp, *tsp2;
	struct sp_stat *p = RootStats;

	for (tsp=spRoot; tsp != NULL; tsp=tsp->nextSpCode) {
		p = MkSpStruct(p, "", tsp->sp, tsp->IDs);
		p = MkSpStruct(p, tsp->sp, tsp->sp, NULL);
		for (tsp2=tsp->nextSpCode; tsp2 != NULL; tsp2=tsp2->nextSpCode) {
			p = MkSpStruct(p, tsp->sp, tsp2->sp, NULL);
		}
	}
	if (onlydata == 1) {
		fprintf(fpout, "Line #\tSpeaker");
		for (p=RootStats; p != NULL; p=p->nextsp) {
			if (*p->prev_sp_name == EOS)
				fprintf(fpout, "\t%s duration of", p->sp_name);
			else
				fprintf(fpout, "\t%s-%s duration between", p->prev_sp_name, p->sp_name);
		}
		fprintf(fpout, "\n");
	} else if (onlydata == 2) {
	} else if (onlydata == 3) {
		if (timedur_ftime1 || !combinput) {
			fprintf(fpout, "File\tLanguage\tCorpus\tCode\tAge\tSex\tGroup\tSES\tRole\tEducation\tCustom_field\t# Words\t# Utterances\tDuration Time\t# Words/Time\t# Utts/Time\n");
		}
	} else {
		fprintf(fpout, " #  Cur|");
		for (p=RootStats; p != NULL; p=p->nextsp) {
			if (*p->prev_sp_name == EOS)
				fprintf(fpout, "  %-3s  |", p->sp_name);
			else
				fprintf(fpout, "%3s-%-3s|", p->prev_sp_name, p->sp_name);
		}
		fprintf(fpout, "\n");
	}
	timedur_ftime1 = FALSE;
}

void call() {
	int i, wi, cnt;
	char *hc;
	char word[BUFSIZ];
	char thissp[SPEAKERLEN], prevsp[SPEAKERLEN], isFirstStartTime;
	long prev_stime, prev_etime, firstStartTime, tutts, twords, words;
	unsigned long transcript_total;
	float tt, tw, tu;
	struct sp_stat *p;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (onlydata == 3 && uS.partcmp(utterance->speaker,"@ID:",FALSE,FALSE)) {
			if (isIDSpeakerSpecified(utterance->line, templineC, TRUE)) {
				uS.remblanks(utterance->line);
				spcpy(thissp, templineC);
				spRoot = AddSpeakersToList(spRoot, thissp, utterance->line);
			}
		} else if (*utterance->speaker == '*' && (onlydata < 2 || checktier(utterance->speaker))) {
			spcpy(thissp, utterance->speaker);
			spRoot = AddSpeakersToList(spRoot, thissp, NULL);
		}
	}
	rewind(fpin);
	if (!stout && onlydata == 0) {
		fprintf(fpout, "@UTF8\n");
#ifdef _WIN32 
		fprintf(fpout, "%s	Win95:CAfont:-15:0\n", FONTHEADER);
#else
		fprintf(fpout, "%s	CAfont:13:7\n", FONTHEADER);
#endif
	}
	tutts = 0L;
	twords = 0L;
	isFirstStartTime = TRUE;
	lineno = 0L;
	tlineno = 1L;
	ln = 0L;
	*thissp = *prevsp = EOS;
	prev_stime = prev_etime = 0L;
	if (RootStats == NULL)
		MakeSpHeader();
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
printf("sp=%s; uttline=%s", utterance->speaker, uttline);
if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
*/
		if (*utterance->speaker == '*') {
			if (checktier(utterance->speaker)) {
				strcpy(prevsp, thissp);
				spcpy(thissp, utterance->speaker);
				ln = lineno;
				if ((hc=strchr(utterance->line, HIDEN_C)) != NULL) {
					if (isdigit(hc[1]) || uS.partcmp(hc+1,SOUNDTIER,FALSE,FALSE) || uS.partcmp(hc+1,REMOVEMOVIETAG,FALSE,FALSE)) {
						ProcessStats(prevsp, thissp, &prev_stime, &prev_etime, hc);
						if (isFirstStartTime) {
							isFirstStartTime = FALSE;
							firstStartTime = prev_stime;
						}
					}
				}
				if (onlydata == 2 || onlydata == 3) {
					words = 0L;
					i = 0;
					while ((i=getword(utterance->speaker, uttline, word, &wi, i))) {
						if (word[0] == '-' && !uS.isToneUnitMarker(word) && !exclude(word))
							continue;
						if (exclude(word)) {
							words++;
						}
					}
					for (p=RootStats; p != NULL; p=p->nextsp) {
						if (*p->prev_sp_name == EOS && !strcmp(thissp,p->sp_name)) {
							p->words = p->words + words;
							p->utts = p->utts + 1L;
							twords = twords + words;
							tutts = tutts + 1L;
						}
					}
				}
			} else
				*thissp = EOS;
		}
	}
	if (!isFirstStartTime) {
		transcript_total = 0L;
		if (onlydata == 1) {
			fprintf(fpout, "Totals\t");
			for (p=RootStats; p != NULL; p=p->nextsp) {
				if (*p->prev_sp_name == EOS) {
					fprintf(fpout, "\t%ld", p->total);
					transcript_total += p->total;
				} else
					fprintf(fpout, "\t");
			}
			fprintf(fpout, "\n\n");
			fprintf(fpout, "Transcript Total (excluding pauses between utterances) MSec:\t%ld\n", transcript_total);
			fprintf(fpout, "Transcript Total (including pauses between utterances) MSec:\t%ld\n", prev_etime - firstStartTime);
		} else if (onlydata == 2) {
			if (!stout)
				fprintf(fpout, "**** File name: %s\n", oldfname);
			for (p=RootStats; p != NULL; p=p->nextsp) {
				if (*p->prev_sp_name == EOS) {
					transcript_total += p->total;
					fprintf(fpout, "Ratio for Speaker: %s\n", p->sp_name);
					fprintf(fpout, "\tNumber of: utterances = %ld, words = %ld\n", p->utts, p->words);
					fprintf(fpout, "\tDuration time: %ld Secs\n", p->total);
					if (p->total > 0L) {
						tw = (float)p->words;
						tt = (float)p->total;
						fprintf(fpout,"\tRatio of words over time duration (#/Secs) = %.3f\n", tw/tt);
						tu = (float)p->utts;
						fprintf(fpout,"\tRatio of utterances over time duration (#/Secs) = %.3f\n", tu/tt);
					}
					putc('\n',fpout);
				}
			}
			fprintf(fpout,"Total number of: utterances = %ld, words = %ld\n", tutts, twords);
			fprintf(fpout, "Transcript Total Duration (excluding pauses between utterances): %ld Secs\n", transcript_total);
			if (transcript_total > 0L) {
				tw = (float)twords;
				tt = (float)transcript_total;
				fprintf(fpout,"\tRatio of total number of words over time duration (#/Secs) = %.3f\n", tw/tt);
				tu = (float)tutts;
				fprintf(fpout,"\tRatio of total number of utterances over time duration (#/Secs) = %.3f\n", tu/tt);
			}
			fprintf(fpout, "Transcript Total Duration (including pauses between utterances): %ld Secs\n", prev_etime - firstStartTime);
			if (prev_etime - firstStartTime > 0L) {
				tw = (float)twords;
				tt = (float)prev_etime - firstStartTime;
				fprintf(fpout,"\tRatio of total number of words over time duration (#/Secs) = %.3f\n", tw/tt);
				tu = (float)tutts;
				fprintf(fpout,"\tRatio of total number of utterances over time duration (#/Secs) = %.3f\n", tu/tt);
			}
			putc('\n',fpout);
		} else if (onlydata == 3) {
			for (p=RootStats; p != NULL; p=p->nextsp) {
				if (*p->prev_sp_name == EOS) {
					tw = (float)twords;
					tu = (float)tutts;
					tt = (float)p->total;
					hc = strrchr(oldfname, PATHDELIMCHR);
					if (hc == NULL)
						hc = oldfname;
					else
						hc++;
					strcpy(FileName1, hc);
					hc = strchr(FileName1, '.');
					if (hc != NULL)
						*hc = EOS;
					fprintf(fpout, "%s", FileName1);
					if (p->IDs != NULL) {
						cnt = 0;
						for (i=0; p->IDs[i] != EOS; i++) {
							if (p->IDs[i] == '|') {
								cnt++;
								if (cnt < 10)
									p->IDs[i] = '\t';
								else
									p->IDs[i] = EOS;
							}
						}
						fprintf(fpout,"\t%s", p->IDs);
					} else
						fprintf(fpout,"\t.\t.\t%s\t.\t.\t.\t.\t.\t.\t.", p->sp_name);
					if (p->total > 0L) {
						fprintf(fpout, "\t%ld\t%ld\t%ld\t%.3f\t%.3f\n", p->words, p->utts, p->total, tw/tt, tu/tt);
					} else {
						fprintf(fpout, "\t%ld\t%ld\t%ld\tN/A\tN/A\n",  p->words, p->utts, p->total);
					}
				}
			}				
		} else {
			fprintf(fpout, " Totals|");
			for (p=RootStats; p != NULL; p=p->nextsp) {
				if (*p->prev_sp_name == EOS) {
					fprintf(fpout, "%7ld|", p->total);
					transcript_total += p->total;
				} else
					fprintf(fpout, "       |");
			}
			fprintf(fpout, "\n\n");
			fprintf(fpout, "Transcript Total (excluding pauses between utterances): %ld MSec\n", transcript_total);
			fprintf(fpout, "Transcript Total (including pauses between utterances): %ld MSec\n", prev_etime - firstStartTime);
		}
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = TIMEDUR;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	FreeSpList();
	FreeSpStats();
}
