/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- database.hpp	- v1.0.0 - november 1998
This files contains the main functions and definitions. For the set of specific definitions that may be modified
like separator used for tags, defautl filenames, ... see MOR.HPP

All the information that is needed to tags a text with POST process is regrouped in one single file. This file contains four kind of information mainly:
	1- the list of tags + 1bis - reverse list of tags
	2- the list of disambiguation rules
	3- matrix(s) for disambiguation is this options is used
	4- lexicon of the language is this option is used

This is useful to avoid having multiples files that have the same name with different extension or whatever system is used
to group files. The idea is that all reading and writing functions correspond to the same file descriptor whatever the
type of information used. So records from all above sources are mixed in the same file.

To sort all this information, it is first organized using the 'workspace' functions (see workspace.cpp & workspace.hpp)
This organisation consists in having a file header that points to a chained list of record that will contain any kind
of information. This info1, info2, ... correspond to the 4 above elements.

|--------|       |---------|       |---------|
| main   |------>| chunk 1 |------>| chunk 2 |--->end
|        |       |---------|       |---------|
| header |       | info 1  |       | info 2  |
|--------|       |---------|       |---------|

The full workspace can be fully loaded in real memory or in virtual memory as a file. This option is set at compile time.
A option to POST can be written to choose it in the command line.

The workspace functions are:
int create_all( FNType* filename );	// CREATE an existing workspace
int open_all( FNType* filename );		// OPEN an existing workspace
int save_all();				// SAVE the opened workspace
int close_all();			// CLOSE the opened workspace
-- these are kinds of macros that interface with the basic workspace functions (see workspace.?pp)

The different info1, info2 points to structures implemented using hashfile functions described in the atom.?pp, hasc.?pp,
and hashbsc.?pp files. The memory scheme is implemented in storage.?pp

|=========================================================================================================================*/

#ifndef __DATABASE_HPP__
#define __DATABASE_HPP__

#include "atom.hpp"
#include "workspace.hpp"
#include "hashc.hpp"
#include "tags.hpp"
#include "mortags.hpp"
#include "input.hpp"

#define _rules_hash_entries_ 5000
#define _matrix_hash_entries_ 4000
#define _localwords_hash_entries_ 2000
#define _words_hash_entries_ 60000
#define _brillwords_hash_entries_ 10	// keep this for future use. Not necessary anymore.
#define _brillrules_hash_entries_ 200

#ifdef MacOsx
#define MSA_VERSION "3.0(compiled on MacOsx)"
#elif (defined WINDOWS_VC9)
#define MSA_VERSION "3.0(compiled on Windows VisualC++)"
#else
#define MSA_VERSION "3.0"
#endif
#define MSA_DATE "February 2012"
// #define MaxNbClasses 1024
// #define default_databasename "eng.db"
#define default_constrname "eng.constr"
#define catfix_name "catfix.cut"

#define AnaNote_OK	0
#define AnaNote_unknownword	1
#define AnaNote_norule	2
#define AnaNote_noresolution	4
#define AnaNote_nomatrix	8
#define AnaNote_sizelimit	16

#define Separator1default '^'
#define Separator2default '!'

extern char Separator1;
extern char Separator2;

extern char rightSpTier;

void define_databasename(FNType *dbn);
FNType* fileresolve(FNType* fn, FNType* str);

// there are two possible output formats: vertical (without chat info) and chat oriented.
#define VertOutput 1
#define ChatOutput 2
#define ChatFullOutput 4
#define AllWords 8
#define BrillsRules 16
#define PrintForTest 32
#define BrillsOnly 64
#define DefaultAffixes 128
#define InternalXMorFormat 256

// highest level functions first.
int post_init(FNType* dbname, int inmemory=1);
int post_word_init(FNType* dbname, int inmemory=1);
int post_file(FNType* inputname, int style, int style_unkwords, int brill, FNType* outfilename, int filter_out, int LINELIMIT=70, char* argv[]=NULL, int argc=0, int internal_mor=0, int priority_freq_local=1 );
int train_open(FNType* dbname, int cmode, int inmemory=1);
int train_file(FNType* inputname, int style, int withmatrix, FNType* outfilename);

#define KeepAll 1
#define ThrowAwayAll 2
#define Keep 3
#define ThrowAway 4

// utility functions
int mkword( char* words_filename, char* app_filename );
int mkrule( char* words_filename, char* rules_filename, char* app_filename );
int mkmatrix( int dim, char* matrix_filename, char* app_filename );

// int analyze_file( char* namein, char* nameout, int style=0 );
void post_analyzemorline(char* MLine, int style, int style_unkwords, int brill, FILE* outfile, int filter_out, int LINELIMIT = 70, int priority_freq_local=1);

// workspace
int create_all( FNType* fn, int inmemory=1 );
int open_all( FNType* fn, int RW=1, int inmemory=1 );
int save_all();
int close_all();

int* multclass_addone( int* x, int f );
int* multclass_extend( int* x, int nc );

void put_info_into_database(char* tag, char* info);
int get_info_from_database(char* tag, char* info);

#define address_savedate "*address_savedate*"

// words data
extern int _word_hashdic_;
int open_word_dic( const char* words_filename, int readonly=0 );
char* get_word_dic( char* w, char* cs );
void list_word(FILE* out);
// create entry of word dictionary
char* create_parts_of_word(char* d, char* cat, char* pho, char* f1, char* f2, char* f3);
void add_word_dic(char* w, char* ncat, char* npho, char* nf1, char* nf2, char* nf3);
void add_singleword_dic(char* w, char* ncat, char* npho, char* nf1, char* nf2, char* nf3);
// for add_word_dic, some arguments can be optional: then they are given as NULL or empty strings
// get parts of words in dictionary
char* cat_from_word(char* word, char* d);	// gets category
char* catpho_from_word(char* word, char* d);	// gets category and phonology
char* cat_from_word_form(char* word, char* form, char* d);	// gets category + original form
char* catpho_from_word_form(char* word, char* form, char* d); // gets category + original form + phono
char* phono_from_word(char* word, char* d);	// gets phonology
char* freq1_from_word(char* word, char* d);	// gets frequency 1 (lemma form frequency)
char* freq2_from_word(char* word, char* d);	// gets frequency 2 (basic form frequency)
char* freq3_from_word(char* word, char* d);	// gets frequency 3 (local corpus frequency)

char* get_nth_cat(char* cat, int nth, char* d);	// copy the nth category in the words categories
char* get_nth_elt(char* cat, int nth, char* d);	// copy the nth element in the phonology and frequencies

// int get_ambtags_of_words( CLASSNAME** array, int n, AMBTAG* &ambs, int* annotation );

// rules data

extern int _rules_hashdic_ ;
extern int _words_classnames_default_ [];
extern const char *_words_classnames_def_noms_ [];

long32 tags_number();

int open_rules_dic( const char* words_filename, int readonly=0 );
void add_rules_dic(AMBTAG at, int atsz, TAG v);
void list_rules(FILE* out);
char* insert_rule( mortag* MT1, ambmortag* AMT1, mortag* MT2, ambmortag* AMT2 );
void insert_matrix_2(	mortag* MT1, ambmortag* AMT1, mortag* MT2, ambmortag* AMT2 );
void insert_matrix_3(	mortag* MT1, ambmortag* AMT1, mortag* MT2, ambmortag* AMT2,
			mortag* MT3, ambmortag* AMT3 );
void insert_matrix_4(	mortag* MT1, ambmortag* AMT1, mortag* MT2, ambmortag* AMT2,
			mortag* MT3, ambmortag* AMT3, mortag* MT4, ambmortag* AMT4 );

void gbin_filter_create(FNType* filename);
int  gbin_filter_open();
void gbin_filter_free();
void gbin_filter_initialize();

void gbout_filter_create(FNType* filename);
void gbout_filter_free();

void list_filter(FILE* out);

// matrix data
extern int _matrix_hashdic_;
int open_matrix_dic( const char* matrix_filename, int readonly=0 );
void add_matrix_dic( TAG* m, int s );
void list_matrix(FILE* out);
int width_of_matrix();
void set_width_of_matrix(int wd);

// local word data
extern int _localword_hashdic_;
int open_localword_dic( const char* filename, int readonly=0 );
void add_local_word( char* ams, int wt, int nbt );
int get_local_word( char* ams );
void list_localword(FILE* out);

// R is allocated by analyze_algorithm until freed by the dynalloc procedure.
int analyze_algorithm( AMBTAG* A, int n, AMBTAG* &R, int* annotation, int style=0, FILE* out=(FILE*)0 ); //, int* taglocalwords );

void local_abort ( long32 size, const char* funname );	// general ABORT function

// Brill's Rules
// style for Brill's rules training
#define FullRules 1
#define LexicalAccess 2
#define PartialRules 4
#define AnyWord "-----"

// style for Brill's rules analysis
#define FullAndSlow 1
#define NotFullButQuick 0

#define MAXSIZEBRILLRULE 1024

extern int _brillrule_hashdic_ ;
extern int _brillword_hashdic_ ;

void init_rules(void);
void init_mortags(void);
void init_tags(void);
void init_trainproc(void);
void free_trainproc(void);

int train_brill(FNType* name, FNType* outfilename, int brillstyle=FullRules, int brill_threshold = 2);
int open_brills_rules(); // open and read the set of rules.
int initialize_lexicon(); // to be used only in RESTRICT_MOVE is set to 1
void delete_lexicon(); // free the lexicon
int process_with_brills_rules(char** words, TAG* original, TAG* newversion, AMBTAG* A, int size, int style_analyze=NotFullButQuick); // uses Brill's rules on a previously computed sentence.
#define staart_word "."
#define staart_tag "pct"
void contextrl( char **correct_tag_corpus, char **word_corpus, char **guess_tag_corpus, int corpus_size, FILE* outfile, int brillstyle=FullRules, int brill_threshold = 2 );
int is_in_WORDS(char* w);
int is_in_SEENTAGGING(char* st);
int open_brillword_dic( const char* filename, int readonly=0 );
void add_brillword( char* w, TAG t );
void list_brillword(FILE* out);
int open_brillrule_dic( const char* filename, int readonly=0 );
void list_brillrule(FILE* out, int style);
int add_brillrule( char* word, char* tag, char* rule, int at_number=0 );
int add_brillrule_at_number( int num, char* word, char* tag, char* rule );
long32* get_brillrule(const char* word, char* tag, char* ctn, int& _nrec);
char* get_next_brillrule(char* ctn, int& _nrec, long32* _rec);
char* get_brillrule_by_number( int i, char* ctn );
int suppress_brillrule( char* word, char* tag, char* bad_rule, int full=1 );
int suppress_brillrule_by_number( int n, int full=1 );
int howmany_brillrule();
void init_brill_rule_for_tagging();
void free_brill_rule_for_tagging();
int nb_brillrules();
int onemore_brillrule();
int split_original_line( int nMorW, char* OrigLine, char** OrigW, char* buffer, int maxlen );

#endif /* __DATABASE_LSP_HPP__ */
