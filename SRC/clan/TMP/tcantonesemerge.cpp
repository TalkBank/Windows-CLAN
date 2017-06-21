#define VERSION "(16-07-97)"

#include "cu.h"

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h" 

#define CHAT_MODE 3

extern struct tier *defheadtier;

char lineTag[UTTLINELEN];
char lineMac[UTTLINELEN];

void init(char f) {
	if (f) {
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier->nexttier != NULL)
			free(defheadtier->nexttier);
		free(defheadtier);
		defheadtier = NULL;
	}
}

void usage()			/* print proper usage and exit */
{
	printf("Usage: temp [%s] filename(s)\n", mainflgs());
	mainusage();
}

/* temp_parsfname(old,fname,fext) take the file name in "old" removes, if it 
   exists, an extention from the end of it. Then it removes the full path, if 
   it exists. Then it copies the remainder to the "fname" and concatanate the 
   extention "ext" to the end of string in "fname".
*/
static void temp_parsfname(char *old, char *fname, char *fext) {
    register int i;
    register int ti;

    i = strlen(old) - 1;
    while (i >= 0 && isSpace(old[i]))
    	i--;
    old[i+1] = '\0';
    i = strlen(old);
    while (i > 0 && old[i] != '.' && old[i] != PATHDELIM)
    	i--;
    ti = 0;

    if (old[i] == PATHDELIM)
    	i = strlen(old);
    else if (old[i] == '.') {
		if (old[i-1] == PATHDELIM)
			i = strlen(old);
    } else {
        ti = 0;
        if (old[i] != '.')
        	i = strlen(old);
    }

    while (ti < i)
    	*fname++ = old[ti++];
    strcpy(fname,fext);
}

void call() {
	int iMac, iTag, t;
	long macLineno, tagLineno;
	char tagCurrentchar, macCurrentchar;
	char DoneTag = FALSE, DoneMac = FALSE;
	char new_name[FILENAME_MAX];
	FILE *fptag, *tfpin;

	temp_parsfname(oldfname,new_name,".tag");
	if ((fptag=fopen(new_name, "r")) == NULL) {
		fprintf(stderr,"Can't open file %s.\n", new_name);
		cutt_exit(0);
	}
	macLineno = tagLineno = lineno;
    tfpin = fpin;
    lineno = macLineno;
    currentchar = (char)getc_cr(fpin);
    do {
    	DoneMac = !getwholeutter();
    } while (!DoneMac && !partcmp(utterance->speaker,"%can:",FALSE));
    strcpy(lineMac, utterance->line);
    macCurrentchar = currentchar;
    macLineno = lineno;
    fpin = fptag;
    lineno = tagLineno;
    currentchar = (char)getc_cr(fpin);
    do {
	    getwholeutter();
    } while (!DoneTag && !partcmp(utterance->speaker,"*",FALSE));
    strcpy(lineTag, utterance->line);
    tagCurrentchar = currentchar;
    tagLineno = lineno;

	iMac = iTag = 0;
	while (!DoneTag || !DoneMac) {
		if (lineTag[iTag] == EOS) {
			if (!DoneTag) {
			    fpin = fptag;
    			lineno = tagLineno;
			    currentchar = tagCurrentchar;
			    do {
				    DoneTag = !getwholeutter();
				    if (partcmp(utterance->speaker,"*inv:",FALSE))
				    	break;
			    } while (!DoneTag && (!partcmp(utterance->speaker,"*",FALSE) || islower(utterance->speaker[1])));
			    strcpy(lineTag, utterance->line);
			    iTag = 0;
			    tagCurrentchar = currentchar;
   				tagLineno = lineno;
			}
		}
		if (lineMac[iMac] == EOS) {
			if (!DoneMac) {
			    fpin = tfpin;
				lineno = macLineno;
			    currentchar = macCurrentchar;
			    do {
			    	DoneMac = !getwholeutter();
			    } while (!DoneMac && !partcmp(utterance->speaker,"%can:",FALSE));
			    strcpy(lineMac, utterance->line);
			    iMac = 0;
			    macCurrentchar = currentchar;
			    macLineno = lineno;
			}
		}
		if (lineTag[iTag] == '[' && (lineTag[iTag+1] == '=' || lineTag[iTag+1] == '+' ||
									 lineTag[iTag+1] == '!')) {
			t = iTag;
			while (lineTag[t] != ']' && lineTag[t] != EOS)
				t++;
			if (lineTag[t] == ']') {
				iTag = t;
				continue;
			}
		}
		if (lineTag[iTag] == '[' && (lineTag[iTag+1] == '<' || lineTag[iTag+1] == '>')
								 && (lineTag[iTag+2] == ']' || lineTag[iTag+2] == '}')) {
			iTag += 3;
			continue;
		}
		if (lineTag[iTag] == ' ' || lineTag[iTag] == '\t' || lineTag[iTag] == '\n' ||
			lineTag[iTag] == '+' || lineTag[iTag] == '-' || lineTag[iTag] == '/' || 
			lineTag[iTag] == '"' || lineTag[iTag] == '0' || lineTag[iTag] == ':' ||
			lineTag[iTag] == '\'' || lineTag[iTag] == 202 || uS.isskip(lineTag, iTag, FALSE)) {
			iTag++;
			continue;
		}
		if (lineMac[iMac] == '[' && (lineMac[iMac+1] == '=' || lineMac[iMac+1] == '+' || 
									 lineMac[iMac+1] == '!')) {
			t = iMac;
			while (lineMac[t] != ']' && lineMac[t] != EOS)
				t++;
			if (lineMac[t] == ']') {
				iMac = t;
				continue;
			}
		}
		if (lineMac[iMac] == '[' && (lineMac[iMac+1] == '<' || lineMac[iMac+1] == '>')
								 && (lineMac[iMac+2] == ']' || lineMac[iMac+2] == '}')) {
			iMac += 3;
			continue;
		}
		if (lineMac[iMac] == ' ' || lineMac[iMac] == '\t' || lineMac[iMac] == '\n' ||
			lineMac[iMac] == '+' || lineMac[iMac] == '-' || lineMac[iMac] == '/' ||
			lineMac[iMac] == '"' || lineMac[iMac] == '0' || lineMac[iMac] == ':' || 
			lineMac[iMac] == '\'' || uS.isskip(lineMac, iMac, FALSE)) {
			iMac++;
			continue;
		}
		if (lineTag[iTag] != lineMac[iMac]) {
			fprintf(fpout,"*** File \"%s\": line %ld\n", oldfname, macLineno);
			fputs(lineMac, fpout);
			fprintf(fpout,"*** File \"%s\": line %ld\n", new_name, tagLineno);
			fputs(lineTag, fpout);
			fputs("*******************************************************\n", fpout);
			iTag = strlen(lineTag);
			iMac = strlen(lineMac);
			fclose(fptag);
			cutt_exit(0);
		} else {
			iTag++;
			iMac++;
		}
	}
	fclose(fptag);
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	lversion = VERSION;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
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
