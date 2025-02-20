/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 0

#include "cu.h"

#if !defined(UNX)
#define _main combtier_main
#define call combtier_call
#define getflag combtier_getflag
#define init combtier_init
#define usage combtier_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 


char combTier[SPEAKERLEN];

void init(char f) {
	onlydata = 10;

	if (f) {
		*combTier = EOS;
		chatmode = 0;
	} else {
		if (*combTier == EOS) {
			fprintf(stderr,"Please specify tier to combine with \"+t\" option.\n");
			cutt_exit(0);
		}
	}
}

void usage()			/* print proper usage and exit */
{
	printf("Usage: combtier [t %s] filename(s)\n",mainflgs());
	fprintf(stderr,"Please specify tier to combine with \"+t\" option.\n");
	puts("+tS: Combine all tiers S into one tier.");
	mainusage(TRUE);
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = COMBTIER;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
}

void getflag(char *f, char *f1, int *i) {
	int j;
	f++;
	switch(*f++) {
		case 't':
				strcpy(combTier, f);
				uS.lowercasestr(combTier, &dFnt, C_MBF);
				uS.remblanks(combTier);
				j = strlen(combTier) - 1;
				if (combTier[j] != ':') {
					fprintf(stderr,"Please specify the full tier name.\n");
					fprintf(stderr,"For example: \"+t%%spa:\"\n");
					cutt_exit(0);
				}
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}

static void comtier_anal(char *com, char *sp) {
	int i;

	strcpy(templineC, sp);
	uS.lowercasestr(templineC, &dFnt, C_MBF);
	if (uS.partcmp(templineC, combTier, FALSE, TRUE)) {
		if (*com != EOS) {
			if (uS.partcmp("%com:", combTier, FALSE, TRUE)) {
				i = strlen(com) - 1;
				for (; i >= 0 && (com[i] == ' ' || com[i] == '\t' || com[i] == '\n'); i--) {
					if (com[i] == '.' || com[i] == '!' || com[i] == '?') break;
				}
				if (i < 0)
					com[0] = EOS;
				else if (uS.IsUtteranceDel(com,i))
					strcat(com, " ");
				else {
					com[++i] = EOS;
					strcat(com, ". ");
				}
			} else
				strcat(com, " ");
		}
		strcat(com, uttline);
	} else {
		if (*com) {
			if (sp[0] == '*' || sp[0] == '@') {
				if (uS.partcmp("%com:", combTier, FALSE, TRUE)) {
					i = strlen(com) - 1;
					for (; i >= 0 && (com[i] == ' ' || com[i] == '\t' || com[i] == '\n'); i--) {
						if (com[i] == '.' || com[i] == '!' || com[i] == '?') break;
					}
					if (i < 0)
						com[0] = EOS;
					else if (uS.IsUtteranceDel(com,i))
						strcat(com, " ");
					else {
						com[++i] = EOS;
						strcat(com, ".");
					}
				}
				printout(combTier, com, NULL, NULL, TRUE);
				*com = EOS;
			}
		}
		printout(sp, uttline, NULL, NULL, TRUE);
	}
}

void call() {
	register int i;
	char buf[BUFSIZ];
	char sp[SPEAKERLEN];

	spareTier1[0] = EOS;
	*uttline = EOS;
	while (fgets_cr(buf, BUFSIZ, fpin)) {
		if (isSpeaker(*buf)) {
			if (*uttline != EOS)
				comtier_anal(spareTier1, sp);

			for (i=0; buf[i] != ':' && buf[i] != '\n'; i++) sp[i] = buf[i];
			if (buf[i] == ':') {
				sp[i] = buf[i];
				for (i++; buf[i] == ' ' || buf[i] == '\t'; i++) sp[i] = buf[i];
			}
			sp[i] = EOS;
			strcpy(uttline, buf+i);
		} else
			strcat(uttline, buf);
	}
	if (*uttline != EOS)
		comtier_anal(spareTier1, sp);
	if (*spareTier1)
		printout(combTier, spareTier1, NULL, NULL, TRUE);
}
