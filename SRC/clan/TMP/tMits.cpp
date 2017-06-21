/**********************************************************************
	"Copyright 2010 Leonid Spektor. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif

#if !defined(UNX) || defined(CLAN_SRV)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 0

extern struct tier *defheadtier;
extern char OverWriteFile;


/* ******************** ab prototypes ********************** */
/* *********************************************************** */

void init(char f) {
	if (f) {
		OverWriteFile = TRUE;
		stout = TRUE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
		}
		defheadtier = NULL;
	}
}

void usage() {
	printf("changes excel formated file to CHAT.\n");
	mainusage();
}


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = TEMP;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
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

static void error_mess(int num, FILE *fp) {
	if (fp != NULL)
		fclose(fp);
	fprintf(stderr, "Found error #%d.\n", num);
	cutt_exit(0);
}

void call() {
	char filename[128], age[128], oAge[128], *tab, cnt;
	char *word, *ipa, *pit, *com;
	FILE *fp = NULL;

	filename[0] = oldfname[0];
	fp = NULL;
	oAge[0] = EOS;
	while (fgets_cr(uttline, UTTLINELEN, fpin)) {
		uS.remblanks(uttline);
		if (uS.partwcmp(uttline, "Age"))
			continue;
		tab = strchr(uttline, '\t');
		if (tab == NULL)
			error_mess(1, fp);
		*tab = EOS;
		strcpy(age, uttline);
		if (strcmp(age, oAge)) {
			cnt++;
			if (cnt < 10)
				sprintf(filename+1, "0%d.ASCII.cha", cnt);
			else
				sprintf(filename+1, "%d.ASCII.cha", cnt);
			if (fp != NULL) {
				fprintf(fp, "@End\n");
				fclose(fp);
			}
			fp = fopen(filename, "w");
			if (fp == NULL)
				error_mess(2, fp);
			fprintf(stderr, "Created file: %s\n", filename);
			strcpy(oAge, age);
			fprintf(fp, "@Begin\n");
			fprintf(fp, "@Languages:\tja\n");
			strcpy(filename, oldfname);
			com = strchr(filename, '.');
			if (com != NULL)
				*com = EOS;
			uS.lowercasestr(filename+1, &dFnt, MBF);
			fprintf(fp, "@Participants:\tCHI %s Target_Child\n", filename);
			fprintf(fp, "@ID:\tja|ota|CHI|%s||||Target_Child||\n", age);
		}
		word = tab + 1;
		tab = strchr(word, '\t');
		if (tab == NULL) {
			ipa = "";
			pit = "";
			com = "";
			goto fin;
		}
		*tab = EOS;
		ipa = tab + 1;
		tab = strchr(ipa, '\t');
		if (tab == NULL) {
			pit = "";
			com = "";
			goto fin;
		}
		*tab = EOS;
		pit = tab + 1;
		tab = strchr(pit, '\t');
		if (tab == NULL) {
			com = "";
			goto fin;
		}
		*tab = EOS;
		com = tab + 1;
		tab = strchr(com, '\t');
		if (tab != NULL)
			*tab = EOS;
fin:
		fprintf(fp, "*CHI:\t%s.\n", word);
		if (*ipa != EOS)
			fprintf(fp, "%%pho:\t%s.\n", ipa);
		if (*pit != EOS)
			fprintf(fp, "%%pit:\t%s.\n", pit);
		if (*com != EOS)
			fprintf(fp, "%%com:\t%s.\n", com);
	}
	if (fp != NULL) {
		fprintf(fp, "@End\n");
		fclose(fp);
	}
}
