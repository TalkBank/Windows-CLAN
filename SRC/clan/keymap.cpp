/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#define CHAT_MODE 2

#include "cu.h"

#if !defined(UNX)
#define _main keymap_main
#define call keymap_call
#define getflag keymap_getflag
#define init keymap_init
#define usage keymap_usage
#endif

#define inthere keymap_inthere
#define pr_result keymap_pr_result
#define IS_WIN_MODE FALSE
#include "mul.h"

extern char tct;
extern struct tier *defheadtier;

#define CODECHAR '$'
#define KEYWORDS struct symbols
#define MAPKEYS struct keymaps
#define SPEAKERS struct speakers

KEYWORDS {
	char *keywordname;
	char *mask;
	KEYWORDS *nextkeyword;
} ;

MAPKEYS {
	char *mapkeyname;
	char *mask;
	int  count;
	char flag;
	long ln;
	SPEAKERS *maps;
	MAPKEYS *nextmapkey;
} ;

SPEAKERS {
	char *sp;
	int count;
	MAPKEYS *mapkey;
	SPEAKERS *nextspeaker;
} ;

KEYWORDS *rootkey;
SPEAKERS *rootspeaker;
char keymap_CodeTierGiven;

void usage() {
	puts("KEYMAP creates a contingency table for coded speech functions.");
	printf("Usage: keymap bS [%s] filename(s)\n", mainflgs());
	puts("+bS: sets a key code to S");
	mainusage(FALSE);
	puts("\nExample:");
	puts("       keymap +b'$CW' +t%spa *.cha");
	puts("       keymap +b'$CW' +b'$CR' +t%spa *.cha");
	cutt_exit(0);
}

void init(char f) {
	if (f) {
		rootkey = NULL;
		rootspeaker = NULL;
		FilterTier = 1;
		LocalTierSelect = TRUE;
		tct = TRUE;
		keymap_CodeTierGiven = FALSE;
	} else {
		if (!keymap_CodeTierGiven) {
			fprintf(stderr,"Please specify a code tier with \"+t\"option.\n");
			cutt_exit(0);
		}
		if (!combinput)
			rootspeaker = NULL;
		if (!rootkey) {
			fprintf(stderr, "Please specify initiation marker with \"+b\" option.\n");
			cutt_exit(0);
		}
	}
}

static int mkmask(char *st, char *mask) {
	int ft = TRUE, pf = FALSE;

	for (; *st; st++) {
		if (isalnum((unsigned char)*st) || *st == '\\') {
			if (*st == '\\') {
				if (*(st+1))
					st++;
			}
			if (ft) {
				ft = FALSE;
				*mask++ = '*';
			}
		} else {
			if (*st == '%')
				pf = TRUE;
			ft = TRUE;
			*mask++ = *st;
		}
	}
	*mask = EOS;
	return(pf);
}

static void SetUpKeyWords(char *st) {
	KEYWORDS *p;
	char mask[BUFSIZ];

	if ((p=NEW(KEYWORDS)) == NULL) out_of_mem();
	if (!mkmask(st,mask)) *mask = EOS;
	p->mask = (char *)malloc((size_t)strlen(mask)+1);
	if (p->mask == NULL)
		out_of_mem();
	strcpy(p->mask, mask);
	if (!nomap)
		uS.lowercasestr(st, &dFnt, C_MBF);
	p->keywordname = st;
	p->nextkeyword = rootkey;
	rootkey = p;
}

static char *inthere(char *st, KEYWORDS *p) {
	while (p != NULL) {
		if (uS.patmat(st,p->keywordname)) {
			uS.remblanks(st);
			return(p->mask);
		}
		p = p->nextkeyword;
	}
	return(NULL);
}

static SPEAKERS *FindSpeaker(char *spname) {
	SPEAKERS *tsp;

	if (rootspeaker == NULL) {
		if ((rootspeaker=NEW(SPEAKERS)) == NULL)
			out_of_mem();
		tsp = rootspeaker;
	} else {
		tsp = rootspeaker;
		while (1) {
			if (!strcmp(spname,tsp->sp))
				return(tsp);
			if (tsp->nextspeaker == NULL)
				break;
			tsp = tsp->nextspeaker;
		}
		if ((tsp->nextspeaker=NEW(SPEAKERS)) == NULL)
			out_of_mem();
		tsp = tsp->nextspeaker;
	}
	tsp->sp = (char *)malloc((size_t)strlen(spname)+1);
	if (tsp->sp == NULL) {
		printf("No more memory.\n"); cutt_exit(1);
	}
	strcpy(tsp->sp, spname);
	tsp->mapkey = NULL;
	tsp->nextspeaker = NULL;
	return(tsp);
}

static SPEAKERS *RegisterSpeakerKeyword(char *word, char *mask, SPEAKERS *sp, long ln) {
	MAPKEYS *tkey, *tkey2, *new_key;

	if (sp->mapkey == NULL) {
		if ((sp->mapkey=NEW(MAPKEYS)) == NULL)
			out_of_mem();
		tkey = sp->mapkey;
		tkey->nextmapkey = NULL;
	} else {
		tkey = sp->mapkey;
		tkey2 = NULL;
		while (1) {
			if (!strcmp(word,tkey->mapkeyname)) {
				tkey->count++;
				tkey->flag = 1;
				tkey->ln = ln;
				return(sp);
			}
			if (strcmp(word,tkey->mapkeyname) < 0) {
				if ((new_key=NEW(MAPKEYS)) == NULL)
					out_of_mem();
				new_key->nextmapkey = tkey;
				if (tkey2 == NULL)
					sp->mapkey = new_key;
				else
					tkey2->nextmapkey = new_key;
				tkey = new_key;
				break;
			}
			if (tkey->nextmapkey == NULL) {
				if ((tkey->nextmapkey=NEW(MAPKEYS)) == NULL)
					out_of_mem();
				tkey = tkey->nextmapkey;
				tkey->nextmapkey = NULL;
				break;
			}
			tkey2 = tkey;
			tkey = tkey->nextmapkey;
		}
	}
	tkey->mapkeyname = (char *)malloc((size_t) strlen(word)+1);
	if (tkey->mapkeyname == NULL)
		out_of_mem();
	strcpy(tkey->mapkeyname, word);
	tkey->mask = mask;
	tkey->count = 1;
	tkey->flag = 1;
	tkey->ln = ln;
	tkey->maps = NULL;
	return(sp);
}

static SPEAKERS *RegisterKeyword(char *word,char *mask,char *spname,long ln,SPEAKERS *sp) {
	if (sp == NULL)
		sp = FindSpeaker(spname);
	return(RegisterSpeakerKeyword(word,mask,sp,ln));
}

static SPEAKERS *GetRightSpeaker(char *thisspname, MAPKEYS *mkey) {
	SPEAKERS *tsp;

	if (mkey->maps == NULL) {
		if ((mkey->maps=NEW(SPEAKERS)) == NULL)
			out_of_mem();
		tsp = mkey->maps;
	} else {
		tsp = mkey->maps;
		while (1) {
			if (!strcmp(thisspname,tsp->sp)) {
				tsp->count++;
				return(tsp);
			}
			if (tsp->nextspeaker == NULL)
				break;
			tsp = tsp->nextspeaker;
		}
		if ((tsp->nextspeaker=NEW(SPEAKERS)) == NULL)
			out_of_mem();
		tsp = tsp->nextspeaker;
	}
	tsp->sp = (char *)malloc((size_t)strlen(thisspname)+1);
	if (tsp->sp == NULL)
		out_of_mem();
	strcpy(tsp->sp, thisspname);
	tsp->count = 1;
	tsp->mapkey = NULL;
	tsp->nextspeaker = NULL;
	return(tsp);
}

static void StoreSpeakerMap(SPEAKERS *tmaps, char *word) {
	MAPKEYS *tkey, *tkey2, *new_key;

	if (tmaps->mapkey == NULL) {
		if ((tmaps->mapkey=NEW(MAPKEYS)) == NULL)
			out_of_mem();
		tkey = tmaps->mapkey;
		tkey->nextmapkey = NULL;
	} else {
		tkey = tmaps->mapkey;
		tkey2 = NULL;
		while (1) {
			if (!strcmp(word,tkey->mapkeyname)) {
				tkey->count++;
				return;
			}
			if (strcmp(word,tkey->mapkeyname) < 0) {
				if ((new_key=NEW(MAPKEYS)) == NULL)
					out_of_mem();
				new_key->nextmapkey = tkey;
				if (tkey2 == NULL)
					tmaps->mapkey = new_key;
				else
					tkey2->nextmapkey = new_key;
				tkey = new_key;
				break;
			}
			if (tkey->nextmapkey == NULL) {
				if ((tkey->nextmapkey=NEW(MAPKEYS)) == NULL)
					out_of_mem();
				tkey = tkey->nextmapkey;
				tkey->nextmapkey = NULL;
				break;
			}
			tkey2 = tkey;
			tkey = tkey->nextmapkey;
		}
	}
	tkey->mapkeyname = (char *)malloc((size_t)strlen(word)+1);
	if (tkey->mapkeyname == NULL)
		out_of_mem();
	strcpy(tkey->mapkeyname, word);
	tkey->count = 1;
	tkey->mask = NULL;
	tkey->maps = NULL;
}

static void reset_sp_flag(SPEAKERS *lastspfound, long lastspln) {
	MAPKEYS *mkey;

	if (lastspfound != NULL) {
		for (mkey=lastspfound->mapkey; mkey != NULL; mkey = mkey->nextmapkey) {
			if (mkey->ln == lastspln)
				mkey->flag = 0;
		}
	}
}

static char addmap(SPEAKERS *lastspfound, char *thisspname, char *word) {
	MAPKEYS *mkey;
	SPEAKERS *tmaps;

	for (mkey=lastspfound->mapkey; mkey != NULL; mkey = mkey->nextmapkey) {
		if (mkey->flag) {
			tmaps = GetRightSpeaker(thisspname,mkey);
			if (*mkey->mask) {
				uS.patmat(word,mkey->mask);
				uS.remblanks(word);
			}
			StoreSpeakerMap(tmaps,word);
		}
	}
	return(0);
}

static void pr_result(void) {
	MAPKEYS  *key1, *Okey1, *key2, *Okey2;
	SPEAKERS *tsp1, *Otsp1, *tsp2, *Otsp2;

	for (tsp1=rootspeaker; tsp1 != NULL; ) {
		fprintf(fpout,"Speaker %s:\n", tsp1->sp);

		for (key1=tsp1->mapkey; key1 != NULL; ) {
			fprintf(fpout,"  Key word \"%s\" found %d time%s\n",
				key1->mapkeyname, key1->count, ((key1->count== 1) ? "": "s"));

			for (tsp2=key1->maps; tsp2 != NULL; ) {
				fprintf(fpout,"      %d instances followed by speaker \"%s\", of these\n",
										tsp2->count, tsp2->sp);

				for (key2=tsp2->mapkey; key2 != NULL; ) {
					fprintf(fpout,"\tcode \"%s\" maps %d time%s\n", 
						key2->mapkeyname, key2->count, ((key2->count == 1) ? "": "s"));
					Okey2 = key2;
					key2 = key2->nextmapkey;
					free(Okey2); 
				}
				Otsp2 = tsp2;
				tsp2 = tsp2->nextspeaker;
				free(Otsp2);
			}
			Okey1 = key1;
			key1 = key1->nextmapkey;
			free(Okey1); 
		}
		Otsp1 = tsp1;
		tsp1 = tsp1->nextspeaker;
		free(Otsp1);
	}
}

void call() {
	register int i;
	char *mask;
	char word[BUFSIZ], spname[SPEAKERLEN];
	SPEAKERS *spfound = NULL, *lastspfound = NULL;
	char CodeTierFound = TRUE;
	long ln = 0L, spln, lastspln = 0L;

	currentatt = 0;
	currentchar = (char)getc_cr(fpin, &currentatt);
	while (getwholeutter()) {
/*
if (checktier(utterance->speaker)) {
	printf("sp=%s; uttline=%s", utterance->speaker, uttline);
	if (uttline[strlen(uttline)-1] != '\n') putchar('\n');
}
*/
		if (*utterance->speaker == '@') ;
		else if (*utterance->speaker == '*') {
			ln++;
			i = strlen(utterance->speaker);
			for (; utterance->speaker[i] != ':'; i--) ;
			utterance->speaker[i] = EOS;
			uS.uppercasestr(utterance->speaker, &dFnt, MBF);
			strcpy(spname,utterance->speaker);
			if (CodeTierFound) {
				reset_sp_flag(lastspfound, lastspln);
				lastspfound = spfound;
				lastspln = spln;
				spfound = NULL;
				spln = 0L;
				CodeTierFound = FALSE;
			}
		} else if (checktier(utterance->speaker)) {/* right tier line found */
			CodeTierFound = TRUE;
			if (lastspfound != NULL) {
				i = 0;
				while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
					if (*word == CODECHAR) {
						if (exclude(word))
							addmap(lastspfound,spname,word);
					}
				}
			}
			if (checktier(spname)) {
				i = 0;
				while ((i=getword(utterance->speaker, uttline, word, NULL, i))) {
					if (*word == CODECHAR) {
						if ((mask=inthere(word,rootkey)) != NULL) {
							spfound=RegisterKeyword(word,mask,spname,ln,spfound);
							spln = ln;
						}
					}
				}
			}
		}
	}
	if (!combinput)
		pr_result();
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'b':
				SetUpKeyWords(getfarg(f,f1,i));
				  break;
		case 't':
				if (*f == '%')
				  	keymap_CodeTierGiven = TRUE;
				maingetflag(f-2,f1,i);
				break;
		default:
				maingetflag(f-2,f1,i);
				break;
	}
}

static void keymap_cleanup_key(KEYWORDS *p) {
	KEYWORDS *t;

	while (p != NULL) {
		t = p;
		p = p->nextkeyword;
		if (t->mask)
			free(t->mask);
		free(t);
	}
}

CLAN_MAIN_RETURN main(int argc, char *argv[]) {
	isWinMode = IS_WIN_MODE;
	CLAN_PROG_NUM = KEYMAP;
	chatmode = CHAT_MODE;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,pr_result);
	keymap_cleanup_key(rootkey);
}
