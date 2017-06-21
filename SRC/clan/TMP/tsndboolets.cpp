#include "ced.h"
#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main sndbool_main
#define call sndbool_call
#define getflag sndbool_getflag
#define init sndbool_init
#define usage sndbool_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 3
/*
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

char isUnderline;
char isChangeDots;
UTTER tier;
char codeTier[UTTLINELEN+1];

/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		isUnderline = FALSE;
		isChangeDots = TRUE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	} else {
		if (!isChangeDots && !chatmode) {
			fprintf(stderr,"+c option can't be used on CA files or with +y option\n");
			cutt_exit(0);
		}
	}
}

void usage() {
	printf("Usage: SNDBoolets [c u %s] filename(s)\n",mainflgs());
	puts("+c: Do not change dots themselves");
	puts("+u: Replace old underline markers with new ones");
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = SNDBOOLETS;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}
		
void getflag(char *f, char *f1, int *i) {

	f++;
	switch(*f++) {
		case 'c':	/* Doing substitutions				    */
			isChangeDots = FALSE;
			no_arg_option(f);
			break;
		case 'u':
			isUnderline = TRUE;
			chatmode = 0;
			no_arg_option(f);
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

#define underline_start_old		"ULS"
#define underline_end_old		"ULE"

static void ChangeTextAtt(char *st) {
	register int i;

	for (i=1; st[i] && st[i] != ATTMARKER; i++) ;
	if (st[i] == ATTMARKER) {
		if (ut.partwcmp(st+1, underline_start_old)) {
			st[1] = underline_start;
			strcpy(st+2, st+i+1);
		} else if (ut.partwcmp(st+1, underline_end_old)) {
			st[1] = underline_end;
			strcpy(st+2, st+i+1);
		}
	}
}

void call() {
	char oldAtt;
	long i, len;

	if (isUnderline) {
		while (fgets_cr(utterance->line, UTTLINELEN, fpin)) {
			for (len=0L; utterance->line[len]; len++) {
				if (utterance->line[len] == ATTMARKER) {
					ChangeTextAtt(utterance->line+len);
				}
			}
			fputs(utterance->line, fpout);
		}
		return;
	}
	tier.speaker[0] = EOS;
	tier.line[0] = EOS;
	tier.tuttline[0] = EOS;
	codeTier[0] = EOS;
	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (isChangeDots) {
			for (len=0L; utterance->line[len]; len++) {
				if (utterance->line[len] == '\245' && utterance->line[len+1] == '%') {
					utterance->line[len] = HIDEN_C;
					for (len++; utterance->line[len] != '\245' && utterance->line[len]; len++) ;
					if (utterance->line[len] == '\245')
						utterance->line[len] = HIDEN_C;
					else if (utterance->line[len] == EOS)
						len--;
				}
			}
		}
		if (chatmode) {
			if (utterance->speaker[0] == '@' || utterance->speaker[0] == '*') {
				if (tier.speaker[0] != EOS)
					printout(tier.speaker, tier.line, tier.attSp, tier.attLine, TRUE);
				if (codeTier[0] != EOS) {
					oldAtt = 0;
					ActualPrint(codeTier, tier.tuttline, &oldAtt, FALSE, TRUE, fpout);
				}
				att_cp(0, tier.speaker, utterance->speaker, tier.attSp, utterance->attSp);
				att_cp(0, tier.line, utterance->line, tier.attLine, utterance->attLine);
				codeTier[0] = EOS;
				tier.tuttline[0] = EOS;
			} else if (ut.partcmp(utterance->speaker, "%snd:", FALSE, FALSE)) {
				uS.remblanks(tier.line);
				uS.remblanks(utterance->line);
				for (len=0L; utterance->line[len]; len++) {
					if (isSpace(utterance->line[len]))
						utterance->line[len] = '_';
				}
				len = strlen(tier.line);
				i = len;
				sprintf(tier.line+len, " %c%%snd:_", HIDEN_C);
				strcat(tier.line, utterance->line);
				len = strlen(tier.line);
				sprintf(tier.line+len, "%c\n", HIDEN_C);
				len = strlen(tier.line);
				while (i < len)
					tier.attLine[i++] = 0;
			} else {
				att_cp(strlen(codeTier),codeTier,utterance->speaker,tier.tuttline,utterance->attSp);
				att_cp(strlen(codeTier),codeTier,utterance->line,tier.tuttline,utterance->attLine);
			}
		} else {
			oldAtt = 0;
			ActualPrint(utterance->line, utterance->attLine, &oldAtt, FALSE, TRUE, fpout);
		}
	}
	if (tier.speaker[0] != EOS)
		printout(tier.speaker, tier.line, tier.attSp, tier.attLine, TRUE);
	if (codeTier[0] != EOS) {
		oldAtt = 0;
		ActualPrint(codeTier, tier.tuttline, &oldAtt, FALSE, TRUE, fpout);
	}
}
