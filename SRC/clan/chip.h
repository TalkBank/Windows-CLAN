/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* "utters.h": This contains the client visible headers for "utters.c".	    */

#define max_words_per_utterance	256
#define max_tiers_per_utterance 10
#define CHIP_MAX_WORD_LENGTH 256

extern int chip_done;
extern char isMorTier;

enum match_enum {
	match_unmarked,
	match_nomatch,
	match_added,
	match_deleted,
	match_subst,
	match_exact
} ;

enum classification_enum {	/* Kinds of utterances in a CHAT file.	    */
	adult,			/* An adult's utterance.		    */
	child,			/* A child's utterance.			    */
	aduslf,			/* An adult self-repetition		    */
	chislf,			/* A child's self-repetition		    */
	other,			/* An utterance for an unknown speaker.	    */
	not_main		/* Not and main utterance.		    */
} ;

/* GLOBAL DATA INDICES FOR THE DIFFERENT INTERACTIONAL-TYPES */
struct inter_data {
    double rep_index;	    /* Repetition index				    */
    double utters;	    /* Total number of utterances		    */
    double responses;	    /* Total number of responses		    */
    double clresponses;	    /* Total number of class-word responses	    */
    double no_overlap;	    /* Total number of non-overlapping responses    */
    double add_ops;	    /* Total number of add operations		    */
    double del_ops;	    /* Total number of delete operations	    */
    double exa_ops;	    /* Total number of exact operations		    */
    double sub_ops;	    /* Total number of substitution operations	    */
    double add_words;	    /* # of added words				    */
    double del_words;	    /* # of deleted words			    */
    double exa_words;	    /* # of repeated words			    */
    double sub_words;	    /* # of substituted words			    */
    double fro_words;	    /* # of fronted words from word-class	    */
    double madd;	    /* # of morphemes added			    */
    double mdel;	    /* # of morphemes deleted			    */
    double mexa;	    /* # of morphemes repeated			    */
    double msub;	    /* # of morphemes substituted for same stem	    */
    double exact_match;	    /* # of total exact matches			    */
    double expanded_match;  /* # of total expansions			    */
    double reduced_match;   /* # of total reductions			    */
    double subst_match;	    /* # of total substitutions			    */
    int dist;		    /* # distance between coded responses	    */
} ;

#define real_word struct word_struct
struct word_struct {
	char *itself;
	char *stem;
	char *prefix;
	char *suffix;
	char *pos;
	enum match_enum match;			/* Matching codes	*/
};

#define real_utterance struct real_utterance_struct
struct real_utterance_struct {
	char *speaker;				/* The speaker name	*/
	enum classification_enum thisClass;		/* Recast thisClass		*/
	int  word_count;
	real_word *words[max_words_per_utterance];
} ;

typedef enum match_enum match;
#define make_null(s)	(*(s) = EOS)	/* Make this string empty.	    */
//typedef char *atom;

real_utterance *new_utter(void);
void free_utter( real_utterance *utter);
int same_speaker(char *sp1, char *sp2);
enum classification_enum classify(char speaker[]);
enum classification_enum utter_class(real_utterance *utter);
void set_child_speaker(char speaker[]);
void set_adult_speaker(char speaker[]);
void output_child_codes(FILE *fp);
void output_adult_codes(FILE *fp);
void set_default_speaker(enum classification_enum speaker);
void chip_exit(const char *err, char isQuit);
void output_utter( FILE *f, real_utterance *utter);
int utter_words(real_utterance *utter);
real_word *utter_word(real_utterance *utter, int index);
int is_speaker(real_utterance *utter);
char *utter_speaker(real_utterance *utter);
char *word_text (real_word *a);
char *word_stem (real_word *a_in);
char *word_suffix (real_word *a);
char *word_prefix (real_word *a);
int word_match(real_word *a, real_word *b, char isCheckMatch);
int word_pos_match(real_word *a, real_word *b, char isCheckMatch);
void insert_code(real_word *a, enum match_enum code);
enum match_enum word_code(real_word *a);
void output_word_text(FILE *f, real_word *a);
char *word_pos(real_word *a);
int pos_match(real_word *a, real_word *b);
int parse_utter(real_utterance *utter);
void pr_ops(void);
void pr_words(void);
void pr_morph(void);
void pr_avg(void);
void pr_front(void);
void pr_special(void);
