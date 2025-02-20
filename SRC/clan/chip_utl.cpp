/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* "chip_utl.c": This module handles sequences of words.		*/

#include "cu.h"
#include "chip.h"

//#define utter	((real_utterance) utter_in)
//#define a_utter	((real_utterance) a_utter_in)
//#define b_utter	((real_utterance) b_utter_in)


/* The following group of global variables indicate what options the user */
/* specified when this program was run.					  */

#define MAX_SPEAKERS	10  /* Maximum number of user-specified speakers  */
#define MAX_SPEAKER_LEN 8   /* Maximum length for user-specified speaker  */
int number_of_children = 0;
int number_of_adults = 0;
static char children[MAX_SPEAKERS+1][MAX_SPEAKER_LEN+1]; /* User specified ids  */
static char adults[MAX_SPEAKERS+1][MAX_SPEAKER_LEN+1];   /* User specified ids  */
enum classification_enum default_speaker = other;	/* Default = other but */
							 /* may be set by user  */

extern int chip_done;
/* DEFAULT VALUES FOR PROGRAM OPTIONS */
extern int DOING_ADU;
extern int DOING_CHI;
extern int DOING_SLF;
extern int DOING_CLASS;
extern int DOING_SUBST;

/* GLOBAL INTERACTIONAL INFORMATION */
extern struct inter_data adu_dat;
extern struct inter_data chi_dat;
extern struct inter_data asr_dat;
extern struct inter_data csr_dat;

real_utterance *new_utter() {
	int i;
	real_utterance *the_utter;
	the_utter = (real_utterance *) malloc(sizeof(real_utterance));

	if (the_utter == NULL)
		chip_exit("new_utter", TRUE);
	else {
		the_utter->speaker = NULL;
		the_utter->word_count = 0;
		for (i = 0; i < max_words_per_utterance; i++) {
			the_utter->words[i] = NULL;
		}
	}
	return (the_utter);
}

static void free_word(real_word *a) {
	if (a != NULL) {
		if (a->itself != NULL) free(a->itself);
		if (a->stem != NULL) free(a->stem);
		if (a->prefix != NULL) free(a->prefix);
		if (a->suffix != NULL) free(a->suffix);
		if (a->pos != NULL) free(a->pos);
		free(a);
	}
}


void free_utter(real_utterance *utter) {
	int i;

	if (utter != NULL) {
 		if (utter->speaker != NULL)
 			free(utter->speaker);
 		for (i = utter_words(utter)-1; i >= 0; i--)
 			free_word(utter->words[i]);
		free(utter);
	}
}

int same_speaker(char *sp1, char *sp2) {
	return (strcmp(sp1, sp2) == 0);
}

enum classification_enum classify(char speaker[]) {
	int i;

	if (speaker[0] == '%') {	/* Speaker ids only start with "*". */
		return not_main;		/* Not a main utterance line.		*/
	}

/* If we can find this speaker in our list of children, return "child".		*/
	for (i = 0; i < number_of_children; i++) {
		if (same_speaker(speaker, children[i])) {
			return child;
		}
	}

/* If we can find this speaker in our list of adults, return "adult".		*/
	for (i = 0; i < number_of_adults; i++) {
		if (same_speaker(speaker, adults[i]))
			return adult;
	}

/* We didn't find this speaker.  Return the default.		*/
/* (Remember that the default can be set by the user.)		*/
	return default_speaker;
}


enum classification_enum utter_class(real_utterance *utter) {
	return utter->thisClass;
}


void set_child_speaker(char speaker[]) {
	strncpy(children[number_of_children], speaker, MAX_SPEAKER_LEN);
	children[number_of_children][MAX_SPEAKER_LEN] = EOS;
	if (number_of_children < MAX_SPEAKERS) {
		number_of_children ++;
	}
}

void set_adult_speaker(char speaker[]) {
	strncpy(adults[number_of_adults], speaker, MAX_SPEAKER_LEN);
	adults[number_of_adults][MAX_SPEAKER_LEN] = EOS;
	if (number_of_adults < MAX_SPEAKERS) {
		number_of_adults ++;
	}
}

void set_default_speaker(enum classification_enum speaker) {
	default_speaker = speaker;
}

void output_child_codes(FILE *fp) {
	int i;

	for (i=0; i < number_of_adults; i++) {
		if (i == 0)
			fprintf(fp, "%s", children[i]);
		else
			fprintf(fp, ", %s", children[i]);
	}
}

void output_adult_codes(FILE *fp) {
	int i;

	for (i=0; i < number_of_adults; i++) {
		if (i == 0)
			fprintf(fp, "%s", adults[i]);
		else
			fprintf(fp, ", %s", adults[i]);
	}
}

void output_utter(
	FILE *f,					/* The output file				*/
	real_utterance *utter)	/* The parsed utterance to output   */
{
	int i;

	fprintf(f, "*%s: ", utter->speaker);
	for (i = 0; i < utter->word_count; i ++) {
		output_word_text(f, utter->words[i]);

		fprintf(f, " ");
	}

	fprintf(f, "\n");
}


int utter_words(real_utterance *utter) {
	if (utter == NULL) return 0;
	return (utter->word_count);
}

real_word *utter_word(real_utterance *utter, int index) {
	if (utter == NULL) return NULL;

	return (utter->words[index]);
}


int is_speaker(real_utterance *utter) {
	if (utter->speaker[0] == '*') {
		return (TRUE);
	} else
		return (FALSE);
}

char *utter_speaker(real_utterance *utter) {
	if (utter == NULL) return (NULL);
	return (utter->speaker);
}


/* "words.c": This handles words and their attributes from CHAT files.		*/

/* Written by Bruce Adams, April 13, 1988.					*/
/* Updated by Jeff Sokolov for morphological analyses, Winter, 1990		*/

static int DO_SUFFIX = FALSE;
static int DO_PREFIX = FALSE;

static void trimPostClitics(char *st) {
	int i;

	i = strlen(st) - 1;
	while (i >= 0 && st[i] == '~')
		i--;
	i++;
	st[i] = EOS;
}

/* Returns a new atom if space is available */
static char *atomize(char *str) {
	char *new_atom;
	
	new_atom = (char *) malloc((size_t) (strlen(str) + 1));
	if (!new_atom)
		chip_exit("Atomization error.", TRUE);	/* DC was here */
	strcpy(new_atom, str);
	return (new_atom);
}

static char *get_stem(char *surf, MORFEATS *word_feats) {
	char stem[CHIP_MAX_WORD_LENGTH];
	int i, j;
	MORFEATS *word_clitc, *word_feat;

	if (surf == NULL && word_feats == NULL) {
		fprintf(stderr, "No string specified for get_stem.\n");
		chip_exit(NULL, TRUE);
	}
	make_null(stem);
	if (surf != NULL) {
		i = 0; j = 0;
		for (i = 0; surf[i] != EOS; i++) {
			if (!uS.isskip(surf,i,&dFnt,MBF)) {
				if (surf[i] == '#') {
					DO_PREFIX = TRUE;
					make_null(stem); j = 0;
				} else if (surf[i] == '-' || surf[i] == '&') {
					DO_SUFFIX = TRUE;
					stem[j] = EOS;
					return (atomize(stem));
				} else {
					stem[j++] = surf[i];
				}
			}
		}
		stem[j] = EOS;
	} else if (word_feats != NULL) {
		for (word_clitc=word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID == '$') {
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+' && stem[0] != EOS)
						strcat(stem, "+");
					if (word_feat->stem != NULL)
						strcat(stem, word_feat->stem);
				}
				if (stem[0] != EOS)
					strcat(stem, "$");
			}
		}
		for (word_clitc=word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID != '$'){
				if (word_clitc->typeID == '~')
					strcat(stem, "~");
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+' && stem[0] != EOS)
						strcat(stem, "+");
					if (word_feat->stem != NULL)
						strcat(stem, word_feat->stem);
				}
			}
		}
	}
	trimPostClitics(stem);
	if (stem[0] != EOS) {
		return (atomize(stem));
	}
	return (NULL);
}

static char *get_prefix(char *surf, MORFEATS *word_feats) {
	char prefix[CHIP_MAX_WORD_LENGTH];
	int i, j;
	MORFEATS *word_clitc, *word_feat;

	if (surf == NULL && word_feats == NULL) {
		fprintf(stderr, "No string specified for get_prefix.\n");
		chip_exit(NULL, TRUE);
	}
	make_null(prefix);
	if (surf != NULL) {
		i = 0; j = 0;
		for (i = 0; ((surf[i] != EOS) && DO_PREFIX); i++) {
			if (!uS.isskip(surf, i, &dFnt, MBF)) {
				if (surf[i] != '#') {
					prefix[j++] = surf[i];
				} else if (surf[i] == '#') {
					prefix[j++] = '#';
					DO_PREFIX = FALSE;
				}
			}
		}
		prefix[j] = EOS;
	} else if (word_feats != NULL) {
		for (word_clitc=word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID == '$') {
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+' && prefix[0] != EOS)
						strcat(prefix, "+");
					addIxesToSt(prefix, word_feat->prefix, NUM_PREF, "#", FALSE);
				}
				if (prefix[0] != EOS)
					strcat(prefix, "$");
			}
		}
		for (word_clitc=word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID != '$') {
				if (word_clitc->typeID == '~')
					strcat(prefix, "~");
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+' && prefix[0] != EOS)
						strcat(prefix, "+");
					addIxesToSt(prefix, word_feat->prefix, NUM_PREF, "#", FALSE);
				}
			}
		}
	}
	trimPostClitics(prefix);
	if (prefix[0] != EOS) {
		DO_PREFIX = FALSE;
		return (atomize(prefix));
	}
	return (NULL);
}

static char *get_suffix(char *surf, MORFEATS *word_feats) {
	char suffix[CHIP_MAX_WORD_LENGTH];
	int i, j;
	MORFEATS *word_clitc, *word_feat;

	if (surf == NULL && word_feats == NULL) {
		fprintf(stderr, "No string specified for get_suffix.\n");
		chip_exit(NULL, TRUE);
	}
	DO_SUFFIX = FALSE;
	make_null(suffix);
	if (surf != NULL) {
		i = 0; j = 0;
		for (i = 0; surf[i] != EOS; i++) {
			if (!uS.isskip(surf, i, &dFnt, MBF)) {
				if (surf[i] == '-' || surf[i] == '&' || DO_SUFFIX) {
					if (surf[i] == '0') {
						return NULL;
					 } else {
						suffix[j++] = surf[i];
						DO_SUFFIX = TRUE;
					}
				}
			}
		}
		suffix[j] = EOS;
	} else if (word_feats != NULL) {
		DO_SUFFIX = TRUE;
		for (word_clitc=word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID == '$') {
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+' && suffix[0] != EOS)
						strcat(suffix, "+");
					addIxesToSt(suffix, word_feat->suffix, NUM_SUFF, "-", TRUE);
					addIxesToSt(suffix, word_feat->fusion, NUM_FUSI, "&", TRUE);
				}
				if (suffix[0] != EOS)
					strcat(suffix, "$");
			}
		}
		for (word_clitc=word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID != '$') {
				if (word_clitc->typeID == '~')
					strcat(suffix, "~");
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+' && suffix[0] != EOS) {
						if (word_clitc->typeID != '~')
							break;
						strcat(suffix, "+");
					}
					addIxesToSt(suffix, word_feat->suffix, NUM_SUFF, "-", TRUE);
					addIxesToSt(suffix, word_feat->fusion, NUM_FUSI, "&", TRUE);
				}
			}
		}
	}
	trimPostClitics(suffix);
	if (suffix[0] != EOS && DO_SUFFIX) {
		DO_SUFFIX = FALSE;
		return (atomize(suffix));
	}
	return (NULL);
}

/* Sets the POS of a_in */
static char *get_pos(char *surf, MORFEATS *word_feats) {
	char pos[CHIP_MAX_WORD_LENGTH];
	MORFEATS *word_clitc, *word_feat;

	if (surf == NULL && word_feats == NULL) {
		fprintf(stderr, "No string specified for get_pos.\n");
		chip_exit(NULL, TRUE);
	}
	make_null(pos);
	if (word_feats != NULL) {
		for (word_clitc=word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID == '$') {
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+' && pos[0] != EOS)
						strcat(pos, "+");
					if (word_feat->pos != NULL)
						strcat(pos, word_feat->pos);
				}
				if (pos[0] != EOS)
					strcat(pos, "$");
			}
		}
		for (word_clitc=word_feats; word_clitc != NULL; word_clitc=word_clitc->clitc) {
			if (word_clitc->typeID != '$') {
				if (word_clitc->typeID == '~')
					strcat(pos, "~");
				for (word_feat=word_clitc; word_feat != NULL; word_feat=word_feat->compd) {
					if (word_feat->typeID == '+' && pos[0] != EOS)
						strcat(pos, "+");
					if (word_feat->pos != NULL)
						strcat(pos, word_feat->pos);
				}
			}
		}
	}
	trimPostClitics(pos);
	if (pos[0] != EOS) {
		return (atomize(pos));
	}
	return (NULL);
}

/* Initializes a new word */
static real_word *new_word(char *surf, char *mor) {
	real_word *the_word;
	MORFEATS word_feats;

	the_word = (real_word *) malloc((size_t) sizeof(real_word));
	if(!the_word)
		chip_exit("new_word", TRUE);

	if (the_word != NULL) {
		DO_SUFFIX = FALSE;
		DO_PREFIX = FALSE;
		if (mor != NULL) {
			if (ParseWordMorElems(mor, &word_feats) == FALSE)
				mor = NULL;
		}
		if (surf == NULL)
			the_word->itself = get_stem(NULL, &word_feats);
		else
			the_word->itself = atomize(surf);
		if (mor == NULL) {
			the_word->stem = get_stem(surf, NULL);
			if (DO_PREFIX) {
				the_word->prefix = get_prefix(surf, NULL);
			} else
				the_word->prefix = NULL;
			if (DO_SUFFIX) {
				the_word->suffix = get_suffix(surf, NULL);
			} else
				the_word->suffix = NULL;
			the_word->pos = NULL;
		} else {
			the_word->stem = get_stem(NULL, &word_feats);
			the_word->prefix = get_prefix(NULL, &word_feats);
			the_word->suffix = get_suffix(NULL, &word_feats);
			the_word->pos = get_pos(NULL, &word_feats);
		}
		the_word->match = match_unmarked;
		if (mor != NULL) {
			freeUpFeats(&word_feats);
		}
	}
	return(the_word);
}

static char chip_isUttDel(char *s) {
	int i;

	for (i=0; s[i] != EOS; i++) {
		if (uS.IsUtteranceDel(s, i))
			return(TRUE);
	}
	return(FALSE);
}

int parse_utter(real_utterance *utter) {
	int i, index = 0;
	char *p = NULL, isMergedTiers;
	char mor_word[BUFSIZ], surf_word[BUFSIZ];
	
	if (chip_done == 0)
		return(FALSE);
	if (utterance == NULL) {
		fprintf(stderr, "parse_utter: CLAN's global 'utterance' is NULL.\n");
		chip_exit(NULL, TRUE);
	}
	if (utter == NULL) {
		fprintf(stderr, "parse_utter: The argument 'utter' is NULL.\n");
		chip_exit(NULL, TRUE);
	}
	while (utterance->speaker[0] != '*') {
		if (*utterance->speaker && onlydata != 2 && onlydata != 3)
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		if ((chip_done=getwholeutter()) == 0)
			return(FALSE);
	}
	/* Classify the type of utterance (child, adult, or other) but  */
	/* first find the colon at end of speaker ID and blast it.		*/
	/* fixed DC  NULL to EOS */
	strcpy(templineC3, utterance->speaker);
	for (p = &utterance->speaker[1]; *p != EOS && *p != ':'; p++) ;
	*p = EOS;
	utter->speaker = atomize(&utterance->speaker[1]); 
	utter->thisClass = classify(utter->speaker);
	isMergedTiers = FALSE;
	if (isMorTier) {
		if (onlydata != 2 && onlydata != 3)
			printout(templineC3,utterance->line,utterance->attSp,utterance->attLine,FALSE);
		while ((chip_done=getwholeutter()) && utterance->speaker[0] != '*') {
			isMergedTiers = FALSE;
			if (onlydata != 2 && onlydata != 3)
				printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
			if (uS.partcmp(utterance->speaker,"%mor:",FALSE,FALSE)) {
				if (!DOING_SUBST || wdptr != NULL) { // 2024-05-06
					if (uttline != utterance->line)
						strcpy(uttline,utterance->line);
					createMorUttline(utterance->line, org_spTier, "%mor:", utterance->tuttline, TRUE, TRUE);
					strcpy(utterance->tuttline, utterance->line);
					if (strchr(uttline, dMarkChr) == NULL) {
						fprintf(stderr,"\n*** File \"%s\": line %ld.\n", oldfname, lineno);
						fprintf(stderr,"%%mor: tier does not have one-to-one correspondence to its speaker tier.\n");
						chip_exit(NULL, TRUE);
					}
					isMergedTiers = TRUE;
				}
				break;
			}
		}
		if (utterance->speaker[0] == '*') {
			utter->word_count = 0;
			return TRUE;
		}
	} 
	utter->word_count = 0;
	index = 0;
	surf_word[0] = EOS;
	mor_word[0] = EOS;
	if (isMorTier) {
		if (DOING_SUBST) {
			for (i=0; uttline[i] != EOS; i++) {
				if (uttline[i] == '~')
					uttline[i] = ' ';
				else if (uttline[i] == '$')
					uttline[i] = ' ';
			}
			if (wdptr != NULL)// 2024-05-06
				index = getNextDepTierPair(uttline, mor_word, surf_word, NULL, index);
			else
				index = getword(utterance->speaker, uttline, mor_word, NULL, index);
		} else
			index = getNextDepTierPair(uttline, mor_word, surf_word, NULL, index);
	} else {
		index = getword(utterance->speaker, uttline, surf_word, NULL, index);
	}
	while (index != 0) {
		if (isMergedTiers == TRUE && 
			((mor_word[0]  != EOS && (exclude(mor_word)  == FALSE || excludedef(mor_word)  == FALSE)) ||
			 (surf_word[0] != EOS && (exclude(surf_word) == FALSE || excludedef(surf_word) == FALSE)))) {
		} else if (!chip_isUttDel(surf_word)) {
			if (isMorTier) {
				if (DOING_SUBST)
					utter->words[utter->word_count] = new_word(NULL, mor_word);
				else
					utter->words[utter->word_count] = new_word(surf_word, mor_word);
			} else {
				utter->words[utter->word_count] = new_word(surf_word, NULL);
			}
			if (utter->words[utter->word_count] != NULL) {
				utter->word_count ++;
			} else {
				fprintf(stderr, "Couldn't add a new word.\n");
				return FALSE;
			}
		}
		if (isMorTier) {
			if (DOING_SUBST) {
				if (wdptr != NULL)// 2024-05-06
					index = getNextDepTierPair(uttline, mor_word, surf_word, NULL, index);
				else
					index = getword(utterance->speaker, uttline, mor_word, NULL, index);
			} else
				index = getNextDepTierPair(uttline, mor_word, surf_word, NULL, index);
		} else {
			index = getword(utterance->speaker, uttline, surf_word, NULL, index);
		}
	}
	if (!isMorTier && onlydata != 2 && onlydata != 3)
		printout(templineC3,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	while ((chip_done=getwholeutter()) && utterance->speaker[0] == '%') {
		if (onlydata != 2 && onlydata != 3)
			printout(utterance->speaker,utterance->line,utterance->attSp,utterance->attLine,FALSE);
	}
	return TRUE;	/* Indicate that we found an utterance.  */
}


/* Returns the text of a_in */
char *word_text (real_word *a) {
	if (a == NULL) return (NULL);

	return a->itself;
}

/* Returns the stem of a_in */
char *word_stem (real_word *a) {
	if (a == NULL) return (NULL);

	return a->stem;
}

/* Returns the suffix of a_in */
char *word_suffix (real_word *a) {
	if (a == NULL) return NULL;

	return a->suffix;
}

/* Returns the prefix of a_in */
char *word_prefix (real_word *a) {
	if (a == NULL) return NULL;

	return a->prefix;
}


/* Returns a number greater than zero if the stems of the		*/
/* two words match:												*/
/*	0 if there is no match									*/

int word_match(real_word *a, real_word *b, char isCheckMatch) /* Returns TRUE if a_in and b_in match */
{
	char *a_word;
	char *b_word;
 
	if (a == NULL || b == NULL)
		return FALSE;
	if (isCheckMatch && (a->match == match_exact || b->match == match_exact))
		return FALSE;
	a_word = word_stem(a);
	b_word = word_stem(b);
	if (a_word == NULL || b_word == NULL)
		return FALSE;
	if (strcmp(a_word, b_word) == 0) {
		if (!isCheckMatch && (a->match != match_exact || b->match != match_exact))
			return FALSE;
		else
			return TRUE;
	}
	return FALSE;
}

int word_pos_match(real_word *a, real_word *b, char isCheckMatch) {
	char *a_pos, *a_col;
	char *b_pos, *b_col;

	if (a == NULL || b == NULL)
		return FALSE;
	if (isCheckMatch && (a->match != match_unmarked || b->match != match_unmarked))
		return FALSE;
	a_pos = word_pos(a);
	b_pos = word_pos(b);
	if (a_pos == NULL || b_pos == NULL)
		return FALSE;
	a_col = strchr(a_pos, ':');
	if (a_col != NULL)
		*a_col = EOS;
	b_col = strchr(b_pos, ':');
	if (b_col != NULL)
		*b_col = EOS;
	if (uS.mStricmp(a_pos, b_pos) == 0) {
		if (a_col != NULL)
			*a_col = ':';
		if (b_col != NULL)
			*b_col = ':';
		return TRUE;
	}
	if (a_col != NULL)
		*a_col = ':';
	if (b_col != NULL)
		*b_col = ':';
	return FALSE;
}

void insert_code(real_word *a, enum match_enum code) {
	a->match = code;
}


enum match_enum word_code(real_word *a) {  /* Returns the match-code for a_in */
	if (a == NULL)
		return (match_nomatch);
	return a->match;
}


/* Prints a_in to file f */
void output_word_text(FILE *f, real_word *a) {
	if (a == NULL) {
		fprintf(f, "<NULL>");
	} else {
		fprintf(f, "%s", a->itself);
	}
}

/* Returns the POS of a_in */
char *word_pos(real_word *a) {
	if (a == NULL) return NULL;

	return a->pos;
}


/* TRUE if the POS of a_in and b_in match */
int pos_match(real_word *a, real_word *b) {
	char *a_pos;
	char *b_pos;

	if (a == NULL || b == NULL) return FALSE;
	a_pos = word_pos(a);
	b_pos = word_pos(b);
	if (a_pos == NULL || b_pos == NULL) {
		return FALSE;
	} else if (strcmp(a_pos, b_pos) == 0) {
		return TRUE;
	} else
		return FALSE;
}

/* code from pr */
void pr_ops()
{
	double total_adu_ops = 0.0; double total_chi_ops = 0.0;
	double total_asr_ops = 0.0; double total_csr_ops = 0.0;

	/* Output ADD_OPS */
	fprintf(fpout, "ADD_OPS  \t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.add_ops);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.add_ops);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.add_ops);
		fprintf(fpout, "%.0f", csr_dat.add_ops);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output DEL_OPS */
	fprintf(fpout, "DEL_OPS  \t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.del_ops);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.del_ops);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.del_ops);
		fprintf(fpout, "%.0f", csr_dat.del_ops);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output EXA_OPS */
	fprintf(fpout, "EXA_OPS  \t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.exa_ops);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.exa_ops);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.exa_ops);
		fprintf(fpout, "%.0f", csr_dat.exa_ops);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output SUB_OPS */
	if (DOING_SUBST) {
		fprintf(fpout, "SUB_OPS  \t");
		if (DOING_ADU) {
			fprintf(fpout, "%.0f\t", adu_dat.sub_ops);
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			fprintf(fpout, "%.0f\t", chi_dat.sub_ops);
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			fprintf(fpout, "%.0f\t", asr_dat.sub_ops);
			fprintf(fpout, "%.0f", csr_dat.sub_ops);
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");
	}

	total_adu_ops = adu_dat.add_ops + adu_dat.del_ops + 
					adu_dat.exa_ops + adu_dat.sub_ops;
	total_chi_ops = chi_dat.add_ops + chi_dat.del_ops + 
					chi_dat.exa_ops + chi_dat.sub_ops;
	total_asr_ops = asr_dat.add_ops + asr_dat.del_ops + 
					asr_dat.exa_ops + asr_dat.sub_ops;
	total_csr_ops = csr_dat.add_ops + csr_dat.del_ops + 
					csr_dat.exa_ops + csr_dat.sub_ops;

	/* Output %_ADD_OPS */
	fprintf(fpout, "%%_ADD_OPS\t");
	if (DOING_ADU) {
		if (total_adu_ops > 0) 
		   fprintf(fpout, "%2.2f\t", (adu_dat.add_ops / total_adu_ops));
		else fprintf(fpout, "0.00\t");
	 } else fprintf(fpout, "adu\t");
	 if (DOING_CHI) {
		if (total_chi_ops > 0) 
		   fprintf(fpout, "%2.2f\t", (chi_dat.add_ops / total_chi_ops));
		else fprintf(fpout, "0.00\t");
	 } else fprintf(fpout, "chi\t");
	 if (DOING_SLF) {
		if (total_asr_ops > 0) 
			   fprintf(fpout, "%2.2f\t", (asr_dat.add_ops / total_asr_ops));
		else fprintf(fpout, "0.00\t");
		if (total_csr_ops > 0) 
			   fprintf(fpout, "%2.2f", (csr_dat.add_ops / total_csr_ops));
		else fprintf(fpout, "0.00");
	 } else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");	
	
	/* Output %_DEL_OPS */
	fprintf(fpout, "%%_DEL_OPS\t");
	if (DOING_ADU) {
		if (total_adu_ops > 0)
		   fprintf(fpout, "%2.2f\t", (adu_dat.del_ops / total_adu_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (total_chi_ops > 0)
		   fprintf(fpout, "%2.2f\t", (chi_dat.del_ops / total_chi_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (total_asr_ops > 0) 
		   fprintf(fpout, "%2.2f\t", (asr_dat.del_ops / total_asr_ops));
		else fprintf(fpout, "0.00\t");
		if (total_csr_ops > 0) 
		   fprintf(fpout, "%2.2f", (csr_dat.del_ops / total_csr_ops));
		else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_EXA_OPS */
	fprintf(fpout, "%%_EXA_OPS\t");
	if (DOING_ADU) {
		if (total_adu_ops > 0)
		   fprintf(fpout, "%2.2f\t", (adu_dat.exa_ops / total_adu_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (total_chi_ops > 0)
		   fprintf(fpout, "%2.2f\t", (chi_dat.exa_ops / total_chi_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (total_asr_ops > 0) 
		   fprintf(fpout, "%2.2f\t", (asr_dat.exa_ops / total_asr_ops));
		else fprintf(fpout, "0.00\t");
		if (total_csr_ops > 0) 
		   fprintf(fpout, "%2.2f", (csr_dat.exa_ops / total_csr_ops));
		else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");	
	
	/* Output %_SUB_OPS */
	if (DOING_SUBST) {
		fprintf(fpout, "%%_SUB_OPS\t");
		if (DOING_ADU) {
		   if (total_adu_ops > 0)
			  fprintf(fpout, "%2.2f\t", (adu_dat.sub_ops / total_adu_ops));
		   else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
		   if (total_chi_ops > 0)
			  fprintf(fpout, "%2.2f\t", (chi_dat.sub_ops / total_chi_ops));
		   else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			if (total_asr_ops > 0) 
				fprintf(fpout, "%2.2f\t", (asr_dat.sub_ops / total_asr_ops));
			else fprintf(fpout, "0.00\t");
			if (total_csr_ops > 0) 
				fprintf(fpout, "%2.2f", (csr_dat.sub_ops / total_csr_ops));
			else fprintf(fpout, "0.00");
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");
	}
}

void pr_words()
{
	double total_adu_words = 0.0;	double total_chi_words = 0.0;
	double total_asr_words = 0.0;	double total_csr_words = 0.0;
	
	/* Output ADD_WORD */
	fprintf(fpout, "ADD_WORD\t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.add_words);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.add_words);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.add_words);
		fprintf(fpout, "%.0f", csr_dat.add_words);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output DEL_WORD */
	fprintf(fpout, "DEL_WORD\t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.del_words);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.del_words);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.del_words);
		fprintf(fpout, "%.0f", csr_dat.del_words);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output EXA_WORD */
	fprintf(fpout, "EXA_WORD\t"); 
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.exa_words);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.exa_words);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.exa_words);
		fprintf(fpout, "%.0f", csr_dat.exa_words);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output SUB_WORD */
	if (DOING_SUBST) {
		fprintf(fpout, "SUB_WORD\t");
		if (DOING_ADU) {
			fprintf(fpout, "%.0f\t", adu_dat.sub_words);
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			fprintf(fpout, "%.0f\t", chi_dat.sub_words);
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			fprintf(fpout, "%.0f\t", asr_dat.sub_words);
			fprintf(fpout, "%.0f", csr_dat.sub_words);
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");
	}
	
	total_adu_words = adu_dat.add_words + adu_dat.del_words + 
					  adu_dat.exa_words + adu_dat.sub_words;
	total_chi_words = chi_dat.add_words + chi_dat.del_words + 
					  chi_dat.exa_words + chi_dat.sub_words;
	total_asr_words = asr_dat.add_words + asr_dat.del_words + 
		  			  asr_dat.exa_words + asr_dat.sub_words;
	total_csr_words = csr_dat.add_words + csr_dat.del_words + 
		  			  csr_dat.exa_words + csr_dat.sub_words;

	/* Output %_ADD_WORDS */
	fprintf(fpout, "%%_ADD_WORDS\t");
	if (DOING_ADU) {
		if (total_adu_words > 0) 
			   fprintf(fpout, "%2.2f\t", (adu_dat.add_words / total_adu_words));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (total_chi_words > 0) 
			   fprintf(fpout, "%2.2f\t", (chi_dat.add_words / total_chi_words));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (total_asr_words > 0) 
			fprintf(fpout, "%2.2f\t", (asr_dat.add_words / total_asr_words));
		else fprintf(fpout, "0.00\t");
		if (total_csr_words > 0) 
			fprintf(fpout, "%2.2f", (csr_dat.add_words / total_csr_words));
		else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_DEL_WORDS */
	fprintf(fpout, "%%_DEL_WORDS\t");
	if (DOING_ADU) {
		if (total_adu_words > 0)
		   fprintf(fpout, "%2.2f\t", (adu_dat.del_words / total_adu_words));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (total_chi_words > 0)
		   fprintf(fpout, "%2.2f\t", (chi_dat.del_words / total_chi_words));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (total_asr_words > 0) 
			fprintf(fpout, "%2.2f\t", (asr_dat.del_words / total_asr_words));
		else fprintf(fpout, "0.00\t");
		if (total_csr_words > 0) 
			fprintf(fpout, "%2.2f", (csr_dat.del_words / total_csr_words));
		else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_EXA_WORDS */
	fprintf(fpout, "%%_EXA_WORDS\t");
	if (DOING_ADU) {
		if (total_adu_words > 0)
		   fprintf(fpout, "%2.2f\t", (adu_dat.exa_words / total_adu_words));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (total_chi_words > 0)
		   fprintf(fpout, "%2.2f\t", (chi_dat.exa_words / total_chi_words));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (total_asr_words > 0) 
			fprintf(fpout, "%2.2f\t", (asr_dat.exa_words / total_asr_words));
		else fprintf(fpout, "0.00\t");
		if (total_csr_words > 0) 
			fprintf(fpout, "%2.2f", (csr_dat.exa_words / total_csr_words));
		else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_SUB_WORDS */
	if (DOING_SUBST) {
		fprintf(fpout, "%%_SUB_WORDS\t");
		if (DOING_ADU) {
		   if (total_adu_words > 0)
  			 fprintf(fpout, "%2.2f\t", (adu_dat.sub_words / total_adu_words));
		   else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
		   if (total_chi_words > 0)
			 fprintf(fpout, "%2.2f\t", (chi_dat.sub_words / total_chi_words));
		   else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			if (total_asr_words > 0) 
				fprintf(fpout, "%2.2f\t", (asr_dat.sub_words / total_asr_words));
			else fprintf(fpout, "0.00\t");
			if (total_csr_words > 0) 
				fprintf(fpout, "%2.2f", (csr_dat.sub_words / total_csr_words));
			else fprintf(fpout, "0.00");
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");
	}
}

void pr_morph()
{
	/* Output MADD */
	fprintf(fpout, "MORPH_ADD\t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.madd);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.madd);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.madd);
		fprintf(fpout, "%.0f", csr_dat.madd);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output MDEL */
	fprintf(fpout, "MORPH_DEL\t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.mdel);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.mdel);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.mdel);
		fprintf(fpout, "%.0f", csr_dat.mdel);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output MEXA */
	fprintf(fpout, "MORPH_EXA\t"); 
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.mexa);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.mexa);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.mexa);
		fprintf(fpout, "%.0f", csr_dat.mexa);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output MSUB */
	fprintf(fpout, "MORPH_SUB\t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.msub);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.msub);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.msub);
		fprintf(fpout, "%.0f", csr_dat.msub);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");
	
	/* Output %_MORPH_ADD */
	fprintf(fpout, "%%_MORPH_ADD\t");
	if (DOING_ADU) {
		if (adu_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (adu_dat.madd / adu_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (chi_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (chi_dat.madd / chi_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (asr_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (asr_dat.madd / asr_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
		if (csr_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f", (csr_dat.madd / csr_dat.exa_words));
		} else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_MORPH_DEL */
	fprintf(fpout, "%%_MORPH_DEL\t");
	if (DOING_ADU) {
		if (adu_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (adu_dat.mdel / adu_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (chi_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (chi_dat.mdel / chi_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (asr_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (asr_dat.mdel / asr_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
		if (csr_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f", (csr_dat.mdel / csr_dat.exa_words));
		} else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_MORPH_EXA */
	fprintf(fpout, "%%_MORPH_EXA\t");
	if (DOING_ADU) {
		if (adu_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (adu_dat.mexa / adu_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (chi_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (chi_dat.mexa / chi_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (asr_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (asr_dat.mexa / asr_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
		if (csr_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f", (csr_dat.mexa / csr_dat.exa_words));
		} else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_MORPH_SUB */
	fprintf(fpout, "%%_MORPH_SUB\t");
	if (DOING_ADU) {
		if (adu_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (adu_dat.msub / adu_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (chi_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (chi_dat.msub / chi_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (asr_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f\t", (asr_dat.msub / asr_dat.exa_words));
		} else fprintf(fpout, "0.00\t");
		if (csr_dat.exa_words > 0) {
			fprintf(fpout, "%2.2f", (csr_dat.msub / csr_dat.exa_words));
		} else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");
}

void pr_avg()
{
	/* Output AV_WORD_ADD */
	fprintf(fpout, "AV_WORD_ADD\t");
	if (DOING_ADU) {
		if (adu_dat.add_ops > 0)
			fprintf(fpout, "%2.2f\t", (adu_dat.add_words / adu_dat.add_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (chi_dat.add_ops > 0)
			fprintf(fpout, "%2.2f\t", (chi_dat.add_words / chi_dat.add_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (asr_dat.add_ops > 0)
			fprintf(fpout, "%2.2f\t", (asr_dat.add_words / asr_dat.add_ops));
		else fprintf(fpout, "0.00\t");
		if (csr_dat.add_ops > 0)
			fprintf(fpout, "%2.2f", (csr_dat.add_words / csr_dat.add_ops));
		else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output AV_WORD_DEL */
	fprintf(fpout, "AV_WORD_DEL\t");
	if (DOING_ADU) {
		if (adu_dat.del_ops > 0)
			fprintf(fpout, "%2.2f\t", (adu_dat.del_words / adu_dat.del_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (chi_dat.del_ops > 0)
			fprintf(fpout, "%2.2f\t", (chi_dat.del_words / chi_dat.del_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (asr_dat.del_ops > 0)
			fprintf(fpout, "%2.2f\t", (asr_dat.del_words / asr_dat.del_ops));
		else fprintf(fpout, "0.00\t");
		if (csr_dat.del_ops > 0)
			fprintf(fpout, "%2.2f", (csr_dat.del_words / csr_dat.del_ops));
		else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output AV_WORD_EXA */
	fprintf(fpout, "AV_WORD_EXA\t");
	if (DOING_ADU) {
		if (adu_dat.exa_ops > 0)
			fprintf(fpout, "%2.2f\t", (adu_dat.exa_words / adu_dat.exa_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (chi_dat.exa_ops > 0)
			fprintf(fpout, "%2.2f\t", (chi_dat.exa_words / chi_dat.exa_ops));
		else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (asr_dat.exa_ops > 0)
			fprintf(fpout, "%2.2f\t", (asr_dat.exa_words / asr_dat.exa_ops));
		else fprintf(fpout, "0.00\t");
		if (csr_dat.exa_ops > 0)
			fprintf(fpout, "%2.2f", (csr_dat.exa_words / csr_dat.exa_ops));
		else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output AV_WORD_SUB */
	if (DOING_SUBST) {
		fprintf(fpout, "AV_WORD_SUB\t");
		if (DOING_ADU) {
			if (adu_dat.sub_ops > 0)
			 fprintf(fpout, "%2.2f\t", (adu_dat.sub_words / adu_dat.sub_ops));
			else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			if (chi_dat.sub_ops > 0)
			 fprintf(fpout, "%2.2f\t", (chi_dat.sub_words / chi_dat.sub_ops));
			else fprintf(fpout, "0.00\t");
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			if (asr_dat.sub_ops > 0)
				fprintf(fpout, "%2.2f\t", (asr_dat.sub_words / asr_dat.sub_ops));
			else fprintf(fpout, "0.00\t");
			if (csr_dat.sub_ops > 0)
				fprintf(fpout, "%2.2f", (csr_dat.sub_words / csr_dat.sub_ops));
			else fprintf(fpout, "0.00");
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");
	}
}

void pr_front()
{
	/* Output Fronted_Words */
	fprintf(fpout, "FRONTED  \t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.fro_words);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.fro_words);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.fro_words);
		fprintf(fpout, "%.0f", csr_dat.fro_words);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_FRONTED_WORDS */
	fprintf(fpout, "%%_FRONTED\t");
	if (DOING_ADU) {
		if (adu_dat.clresponses > 0) {
			fprintf(fpout, "%2.2f\t", adu_dat.fro_words/adu_dat.clresponses);
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (chi_dat.clresponses > 0) {
			fprintf(fpout, "%2.2f\t", chi_dat.fro_words/chi_dat.clresponses);
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (asr_dat.clresponses > 0) {
			fprintf(fpout, "%2.2f\t", asr_dat.fro_words/asr_dat.clresponses);
		} else fprintf(fpout, "0.00\t");
		if (csr_dat.clresponses > 0) {
			fprintf(fpout, "%2.2f", csr_dat.fro_words/csr_dat.clresponses);
		} else fprintf(fpout, "0.00");
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");
}

void pr_special()
{
	/* Output IMITATIONS */
	fprintf(fpout, "IMITAT   \t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", (adu_dat.exact_match+adu_dat.expanded_match+adu_dat.reduced_match+adu_dat.subst_match));
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", (chi_dat.exact_match+chi_dat.expanded_match+chi_dat.reduced_match+chi_dat.subst_match));
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", (asr_dat.exact_match+asr_dat.expanded_match+asr_dat.reduced_match+asr_dat.subst_match));
		fprintf(fpout, "%.0f", (csr_dat.exact_match+csr_dat.expanded_match+csr_dat.reduced_match+csr_dat.subst_match));
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_IMITAT */
	fprintf(fpout, "%%_IMITAT\t");
	if (DOING_ADU) {
		if (DOING_CLASS) {
			if (adu_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t",((adu_dat.exact_match+adu_dat.expanded_match+adu_dat.reduced_match+adu_dat.subst_match)/adu_dat.clresponses));
			} else fprintf(fpout, "0.00\t");
		} else if (adu_dat.utters > 0) {
			fprintf(fpout, "%2.3f\t",((adu_dat.exact_match+adu_dat.expanded_match+adu_dat.reduced_match+adu_dat.subst_match)/adu_dat.utters));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (DOING_CLASS) {
			if (chi_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t",((chi_dat.exact_match+chi_dat.expanded_match+chi_dat.reduced_match+chi_dat.subst_match)/chi_dat.clresponses));
			} else fprintf(fpout, "0.00\t");
		} else if (chi_dat.utters > 0) {
			fprintf(fpout, "%2.3f\t",((chi_dat.exact_match+chi_dat.expanded_match+chi_dat.reduced_match+chi_dat.subst_match)/chi_dat.utters));
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (DOING_CLASS) {
			if (asr_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t",((asr_dat.exact_match+asr_dat.expanded_match+asr_dat.reduced_match+asr_dat.subst_match)/asr_dat.clresponses));
			} else fprintf(fpout, "0.00\t");
			if (csr_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t",((csr_dat.exact_match+csr_dat.expanded_match+csr_dat.reduced_match+csr_dat.subst_match)/csr_dat.clresponses));
			} else fprintf(fpout, "0.00\t");

		} else {
			if (adu_dat.utters > 0) {
				fprintf(fpout, "%2.3f\t",((asr_dat.exact_match+asr_dat.expanded_match+asr_dat.reduced_match+asr_dat.subst_match)/adu_dat.utters));
			} else fprintf(fpout, "0.00\t");
			if (chi_dat.utters > 0) {
				fprintf(fpout, "%2.3f",((csr_dat.exact_match+csr_dat.expanded_match+csr_dat.reduced_match+csr_dat.subst_match)/chi_dat.utters));
			} else fprintf(fpout, "0.00");
		}
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	
	
	/* Output EXACT_MATCHES */
	fprintf(fpout, "EXACT    \t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.exact_match);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.exact_match);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.exact_match);
		fprintf(fpout, "%.0f", csr_dat.exact_match);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output EXPANDED_MATCHES */
	fprintf(fpout, "EXPAN    \t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.expanded_match);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.expanded_match);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.expanded_match);
		fprintf(fpout, "%.0f", csr_dat.expanded_match);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output REDUCED_MATCHES */
	fprintf(fpout, "REDUC    \t");
	if (DOING_ADU) {
		fprintf(fpout, "%.0f\t", adu_dat.reduced_match);
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		fprintf(fpout, "%.0f\t", chi_dat.reduced_match);
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		fprintf(fpout, "%.0f\t", asr_dat.reduced_match);
		fprintf(fpout, "%.0f", csr_dat.reduced_match);
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output SUBST_MATCHES */
	if (DOING_SUBST) {
		fprintf(fpout, "SUBST    \t");
		if (DOING_ADU) {
			fprintf(fpout, "%.0f\t", adu_dat.subst_match);
		} else fprintf(fpout, "adu\t");
		if (DOING_CHI) {
			fprintf(fpout, "%.0f\t", chi_dat.subst_match);
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			fprintf(fpout, "%.0f\t", asr_dat.subst_match);
			fprintf(fpout, "%.0f", csr_dat.subst_match);
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");
	}

	/* Output %_EXACT_MATCHES */
	fprintf(fpout, "%%_EXACT  \t");
	if (DOING_ADU) {
		if (DOING_CLASS) {
			if (adu_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", adu_dat.exact_match/adu_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
		} else if (adu_dat.utters > 0) {
			fprintf(fpout, "%2.3f\t", adu_dat.exact_match/adu_dat.utters);
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (DOING_CLASS) {
			if (chi_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", chi_dat.exact_match/chi_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
		} else if (chi_dat.utters > 0) {
			fprintf(fpout, "%2.3f\t", chi_dat.exact_match/chi_dat.utters);
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (DOING_CLASS) {
			if (asr_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", asr_dat.exact_match/asr_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
			if (csr_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f", csr_dat.exact_match/csr_dat.clresponses);
			} else fprintf(fpout, "0.00");

		} else {
			if (adu_dat.utters > 0) {
				fprintf(fpout, "%2.3f\t", asr_dat.exact_match/adu_dat.utters);
			} else fprintf(fpout, "0.00\t");
			if (chi_dat.utters > 0) {
				fprintf(fpout, "%2.3f", csr_dat.exact_match/chi_dat.utters);
			} else fprintf(fpout, "0.00");
		}
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_EXPANDED_MATCHES */
	fprintf(fpout, "%%_EXPAN  \t");
	if (DOING_ADU) {
		if (DOING_CLASS) {
			if (adu_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", adu_dat.expanded_match/adu_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
		} else if (adu_dat.utters > 0) {
			fprintf(fpout, "%2.3f\t", adu_dat.expanded_match/adu_dat.utters);
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (DOING_CLASS) {
			if (chi_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", chi_dat.expanded_match/chi_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
		} else if (chi_dat.utters > 0) {
			fprintf(fpout, "%2.3f\t", chi_dat.expanded_match/chi_dat.utters);
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (DOING_CLASS) {
			if (asr_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", asr_dat.expanded_match/asr_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
			if (csr_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f", csr_dat.expanded_match/csr_dat.clresponses);
			} else fprintf(fpout, "0.00");

		} else {
			if (adu_dat.utters > 0) {
				fprintf(fpout, "%2.3f\t", asr_dat.expanded_match/adu_dat.utters);
			} else fprintf(fpout, "0.00\t");
			if (chi_dat.utters > 0) {
				fprintf(fpout, "%2.3f", csr_dat.expanded_match/chi_dat.utters);
			} else fprintf(fpout, "0.00");
		}
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_REDUCED_MATCHES */
	fprintf(fpout, "%%_REDUC  \t");
	if (DOING_ADU) {
		if (DOING_CLASS) {
			if (adu_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", adu_dat.reduced_match/adu_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
		} else if (adu_dat.utters > 0) {
			fprintf(fpout, "%2.3f\t", adu_dat.reduced_match/adu_dat.utters);
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "adu\t");
	if (DOING_CHI) {
		if (DOING_CLASS) {
			if (chi_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", chi_dat.reduced_match/chi_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
		} else if (chi_dat.utters > 0) {
			fprintf(fpout, "%2.3f\t", chi_dat.reduced_match/chi_dat.utters);
		} else fprintf(fpout, "0.00\t");
	} else fprintf(fpout, "chi\t");
	if (DOING_SLF) {
		if (DOING_CLASS) {
			if (asr_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f\t", asr_dat.reduced_match/asr_dat.clresponses);
			} else fprintf(fpout, "0.00\t");
			if (csr_dat.clresponses > 0) {
				fprintf(fpout, "%2.3f", csr_dat.reduced_match/csr_dat.clresponses);
			} else fprintf(fpout, "0.00");

		} else {
			if (adu_dat.utters > 0) {
				fprintf(fpout, "%2.3f\t", asr_dat.reduced_match/adu_dat.utters);
			} else fprintf(fpout, "0.00\t");
			if (chi_dat.utters > 0) {
				fprintf(fpout, "%2.3f", csr_dat.reduced_match/chi_dat.utters);
			} else fprintf(fpout, "0.00");
		}
	} else {
		fprintf(fpout, "asr\t");
		fprintf(fpout, "csr");
	}
	fprintf(fpout, "\n");

	/* Output %_SUBST_MATCHES */
	if (DOING_SUBST) {
		fprintf(fpout, "%%_SUBST  \t");
		if (DOING_ADU) {
			if (DOING_CLASS) {
				if (adu_dat.clresponses > 0) {
					fprintf(fpout, "%2.3f\t", adu_dat.subst_match/adu_dat.clresponses);
				} else fprintf(fpout, "0.00\t");
			} else fprintf(fpout, "NA\t");
	   } else fprintf(fpout, "adu\t");
	   if (DOING_CHI) {
		   if (DOING_CLASS) {
			   if (chi_dat.clresponses > 0) {
				   fprintf(fpout, "%2.3f\t", chi_dat.subst_match/chi_dat.clresponses);
				} else fprintf(fpout, "0.00\t");
		   } else fprintf(fpout, "NA\t");
		} else fprintf(fpout, "chi\t");
		if (DOING_SLF) {
			if (DOING_CLASS) {
				if (asr_dat.clresponses > 0) {
					fprintf(fpout, "%2.3f\t", asr_dat.subst_match/asr_dat.clresponses);
				} else fprintf(fpout, "0.00\t");
				if (csr_dat.clresponses > 0) {
					fprintf(fpout, "%2.3f", csr_dat.subst_match/csr_dat.clresponses);
				} else fprintf(fpout, "0.00");
			} else {
				fprintf(fpout, "NA\t");
				fprintf(fpout, "NA");
			}
		} else {
			fprintf(fpout, "asr\t");
			fprintf(fpout, "csr");
		}
		fprintf(fpout, "\n");
	}
}
