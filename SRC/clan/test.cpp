/**********************************************************************
 "Copyright 1990-2016 Brian MacWhinney. Use is subject to Gnu Public License
 as stated in the attached "gpl.txt" file."
 */

#define CHAT_MODE 0
#include "cu.h"

#if !defined(UNX)
#define _main temp_main
#define call temp_call
#define getflag temp_getflag
#define init temp_init
#define usage temp_usage
#endif

#define IS_WIN_MODE FALSE
#include "mul.h"

extern struct tier *defheadtier;
extern char OverWriteFile;

char s[BUFSIZ];
char s2[BUFSIZ];

void usage() {
}

void init(char f) {
}

void call(void) {
}

void getflag(char *f, char *f1, int *i) {
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	int i, len, beg;
	FILE *fpb, *fpa;

	fpb = fopen("before.txt", "w");
	if (fpb == NULL) {
		fprintf(stderr, "ERROR opening file: before.txt\n");
		cutt_exit(0);
	}
	fpa = fopen("after.txt", "w");
	if (fpa == NULL) {
		fclose(fpb);
		fprintf(stderr, "ERROR opening file: after.txt\n");
		cutt_exit(0);
	}
	strcpy(s2, "1234567890");
	len = strlen(s2);
	beg = 3;
	for (i=0; i < 220; i++) {
		strcpy(s, s2);
		fprintf(fpb, "s=%s;\n", s+beg+1);
		strcpy(s+beg, s+beg+1);
		fprintf(fpa, "s=%s;\n", s+beg);
		s2[len] = 32 + i;
		len++;
		s2[len] = '\0';
	}
	fclose(fpb);
	fclose(fpa);
}
