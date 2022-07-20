/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#ifdef _WIN32 
	#include "stdafx.h"
#endif
#include "c_curses.h"

#if !defined(UNX)
#define _main makemod_main
#define call makemod_call
#define getflag makemod_getflag
#define init makemod_init
#define usage makemod_usage
#endif

#include "mul.h" 

#define IS_WIN_MODE FALSE
#define CHAT_MODE 1

#define CMULEX "cmulex.cut"

extern struct tier *defheadtier;

/* *************** mor vars *********************** */
#define WLEXID struct wLexID
WLEXID {
    char *id;
    WLEXID *nextid;
} ;

#define WLEX struct wLex
WLEX {
    char lett;
    WLEXID *lexId;
    WLEX *nextlett;
    WLEX *samelett;
} ;

WLEX rootLex;
char isAllAlts;
/* *********************************************************** */

static void freeIds(WLEXID *t) {
	WLEXID *tt;

	while (t != NULL) {
		tt = t;
		t = t->nextid;
		free(tt->id);
		free(tt);
	}
}

static void FreeList(WLEX *curr) {
	WLEX *t;

	while (curr != NULL) {
		t = curr;
		curr = curr->samelett;
		FreeList(t->nextlett);
		freeIds(t->lexId);
		free(t);
	}
}

static void mod_overflow(void) {
	fprintf(stderr,"makemod: no more memory available.\n");
	FreeList(rootLex.nextlett);
	cutt_exit(0);
}

void usage() {
	printf("Usage: makemod [a %s] filename(s)\n",mainflgs());
	puts("+a : printout all alternative pronunciation");
#ifdef UNX
	puts("+LF: specify full path F of the lib folder");
#endif
	mainusage(TRUE);
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'a':
		    isAllAlts = TRUE;
		    no_arg_option(f);
		    break;
#ifdef UNX
		case 'L':
			int len;
			strcpy(lib_dir, f);
			len = strlen(lib_dir);
			if (len > 0 && lib_dir[len-1] != '/')
				strcat(lib_dir, "/");
			break;
#endif
		default:
			maingetflag(f-2,f1,i);
			break;
	}
}

static void AddLexID(WLEX *word, char *id) {
	WLEXID *t;

	if (word->lexId == NULL) {
		t = NEW(WLEXID);
		if (t == NULL)
			mod_overflow();
		word->lexId = t;
	} else {
		for (t=word->lexId; t->nextid != NULL; t=t->nextid) ;
		t->nextid = NEW(WLEXID);
		if (t->nextid == NULL)
			mod_overflow();
		t = t->nextid;
	}
	t->nextid = NULL;
	t->id = (char *)malloc(strlen(id)+1);
	if (t->id == NULL)
		mod_overflow();
	strcpy(t->id, id);
}

static WLEX *AddLetter(WLEX *t, char lett) {
	if (t->nextlett == NULL) {
		t->nextlett = NEW(WLEX);
		t = t->nextlett;
	} else {
		t = t->nextlett;
		if (t->lett == lett)
			return(t);
		while (t->samelett != NULL) {
			t = t->samelett;
			if (t->lett == lett)
				return(t);
		}
		t->samelett = NEW(WLEX);
		t = t->samelett;
	}
	if (t == NULL)
		mod_overflow();
	t->lexId = NULL;
	t->nextlett = NULL;
	t->samelett = NULL;
	t->lett = lett;
	return(t);
}

static void AddWordToLex(char *word, char *id) {
	int i;
	WLEX *t;
	
	i = strlen(word) - 1;
	if (word[i] == ')') {
		for (; word[i] != '(' && i > 0; i--) ;
		if (word[i] == '(')
			word[i] = EOS;
	}
	uS.uppercasestr(word, &dFnt, C_MBF);
	t = &rootLex;
	for (i=0; word[i]; i++) {
		t = AddLetter(t, word[i]);
	}
	AddLexID(t, id);
}

static void readLex(void) {
	int i, j;
	long ln;
	FILE *fp;
	FNType mFileName[FNSize];

	if ((fp=OpenGenLib(CMULEX,"r",TRUE,TRUE,mFileName)) == NULL) {
		fprintf(stderr, "Can't open file %s\n", CMULEX);
		cutt_exit(0);
	}
	ln = 0L;
	while (fgets_cr(templineC, UTTLINELEN, fp) != NULL) {
		if (uS.isUTF8(templineC) || uS.isInvisibleHeader(templineC))
			continue;
		if (templineC[0] == '#' || templineC[0] == '%')
			continue;
		i = 0;
		j = 0;
		while (!isSpace(templineC[i]) && templineC[i] != '\n' && templineC[i] != EOS)
			templineC2[j++] = templineC[i++];
		templineC2[j] = EOS;
		for (; isSpace(templineC[i]); i++) ;
		j = 0;
		while (templineC[i] != '\n' && templineC[i] != EOS)
			templineC3[j++] = templineC[i++];
		templineC3[j] = EOS;
		ln++;
		if (ln % 100L == 0) {
			fprintf(stderr,"\r%ld ",ln);
		}
		AddWordToLex(templineC2, templineC3);
	}
	fclose(fp);
}

static WLEX *GetLetter(WLEX *t, char lett) {
	if (t->nextlett != NULL) {
		t = t->nextlett;
		if (t->lett == lett)
			return(t);
		while (t->samelett != NULL) {
			t = t->samelett;
			if (t->lett == lett)
				return(t);
		}
	}
	return(NULL);
}

static char *GetWordID(char *word) {
	int i;
	WLEX *t;
	WLEXID *tt;
	
	uS.uppercasestr(word, &dFnt, MBF);
	t = &rootLex;
	for (i=0; word[i]; i++) {
		t = GetLetter(t, word[i]);
		if (t == NULL)
			return(NULL);
	}
	if (t->lexId == NULL)
		return(NULL);
	templineC2[0] = EOS;
	tt = t->lexId;
	while (tt != NULL) {
		strcat(templineC2, tt->id);
		strcat(templineC2, "^");
		tt = tt->nextid;
		if (!isAllAlts)
			break;
	}
	i = strlen(templineC2) - 1;
	if (i >= 0 && templineC2[i] == '^')
		templineC2[i] = EOS;
	return(templineC2);
}

void init(char f) {
	if (f) {
		isAllAlts = FALSE;
		stout = FALSE;
		onlydata = 1;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		if (defheadtier != NULL) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
		rootLex.lexId = NULL;
		rootLex.nextlett = NULL;
		rootLex.samelett = NULL;
	} else {
		if (rootLex.lexId == NULL && rootLex.nextlett == NULL) {
			readLex();
			fprintf(stderr,"\n");
		}	
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = MAKEMOD;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	bmain(argc,argv,NULL);
	FreeList(rootLex.nextlett);
}

void call() {
	int i;
	char w[256], *id;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
		if (checktier(utterance->speaker) && *utterance->speaker == '*') {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			i = 0;
			templineC3[0] = EOS;
			while ((i=getword(utterance->speaker, uttline, w, NULL, i))) {
				if ((id=GetWordID(w)) != NULL) {
					strcat(templineC3, id);
					strcat(templineC3, " ");
				} else
					strcat(templineC3, "??? ");
			}
			uS.remblanks(templineC3);
			printout("%mod",templineC3,NULL,NULL,TRUE);
		} else {
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		}
	}
}

