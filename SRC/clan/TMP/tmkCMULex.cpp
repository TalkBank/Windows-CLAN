#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

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

/* ******************** ab prototypes ********************** */
#define TOTALNUM 60

char o[TOTALNUM][128], n[TOTALNUM][128];
int  index;

/* *********************************************************** */

static void readTable(void) {
	int i, j;
	char *c;
	FILE *fp;

	if ((fp=fopen("table.txt", "r")) == NULL) {
		fprintf(stderr, "Can't open file %s\n", "table.txt");
		return;
	}
	while ((c=fgets(templineC, UTTLINELEN, fp)) != NULL) {
		if (templineC[0] == '#')
			continue;
		i = 0;
		j = 0;
		while (!isSpace(templineC[i]) && templineC[i] != '\n' && templineC[i] != EOS)
			n[index][j++] = templineC[i++];
		n[index][j] = EOS;
		for (; isSpace(templineC[i]); i++) ;
		j = 0;
		while (templineC[i] != '\n' && templineC[i] != EOS)
			o[index][j++] = templineC[i++];
		o[index][j] = EOS;
		index++;
	}
	fclose(fp);
}

void init(char f) {
	if (f) {
		stout = FALSE;
		onlydata = 1;
		FilterTier = 0;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		index = 0;
		readTable();
	}
}

void usage() {
	printf("Usage: ModTrans [%s] filename(s)\n",mainflgs());
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

static int translate(char *ost, int org) {
	char num;
	char found;
	int i, j, k;

	found = FALSE;
	j = strlen(templineC2);
	for (k=0; k < index; k++) {
		i = org;
		j = 0;
		num = 0;
		while (o[k][j] != EOS) {
			if (ost[i] != o[k][j]) {
				if (isdigit(ost[i])) {
					num = ost[i] - '0';
					j--;
				} else
					break;
			}
			i++;
			j++;
		}
		if (o[k][j] == EOS) {
			if (isdigit(ost[i])) {
				num = ost[i] - '0';
				i++;
			}
		}
		if (isSpace(ost[i]) || ost[i] == '\n' || ost[i] == EOS) {
			if (num == 1)
				strcat(templineC2, "\"");
			else if (num == 2)
				strcat(templineC2, "%");
			strcat(templineC2, n[k]);
			strcat(templineC2, "_");
			found = TRUE;
			break;
		}
	}
	if (!found) {
		fprintf(stderr, "Can't translate \"%s\" from \"%s\"", ost+i, templineC);
		cutt_exit(0);
	}
	return(i);
}

void call() {
	char *c;
	int i, j;

	while ((c=fgets(templineC, UTTLINELEN, fpin)) != NULL) {
		if (*templineC == '#')
			fputs(templineC, fpout);
		else {
			i = 0;
			j = 0;
			templineC2[0] = EOS;
			while (!isSpace(templineC[i]) && templineC[i] != '\n' && templineC[i] != EOS)
				templineC2[j++] = templineC[i++];
			for (; isSpace(templineC[i]) || templineC[i] == '\n'; i++)
				templineC2[j++] = templineC[i];
			templineC2[j] = EOS;
			while (templineC[i] != EOS) {
				i = translate(templineC, i);				
				for (; isSpace(templineC[i]) || templineC[i] == '\n'; i++) ;
			}
			j = strlen(templineC2) - 1;
			if (templineC2[j] == '_')
				templineC2[j] = EOS;
			strcat(templineC2, "\n");
			fputs(templineC2, fpout);
		}
	}
}
