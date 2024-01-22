/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


/* morph header file */

#include <stdio.h>
#include <stdlib.h>

/* magic numbers */

#define MAX_WORD		2048
#define MAXLINE			2048
#define MAXENTRY		1024  /* maximum length (in chars) of entry in core lexicon */
#define MAXCAT			1024  /* maximum length (in chars) of cat info in entry  */
#define MAX_PARSES		125  /* max number of parses for a word */
#define MAX_FEATS		5000 /* maximum possible number of features */
#define MAX_VALS		5000 /* maximum possible number of values */
#define MAX_MATCHES		150  /* max number of matches per word (size of match_stack) */
#define MAX_SEGS		10   /* number of expected subpatterns in a word */
#define MAX_PATLEN		1024 /* max length of a reg ex pattern */
#define MAX_STRLEN		1024 /* max length of a text string */
#define MAX_VARNAME		10   /* max length of variable name */
#define MAX_NUMVAR		30   /* max number of variables in 1 (composite) pattern */
#define EMPTY			-99  /* used to initailize array */
#define COMMENT_CHAR	'%'
#define CONTINUE_CHAR	'\\'

#ifndef SUCCEED
#define SUCCEED   1
#endif
#ifndef FAIL
#define FAIL  0
#endif

/* error condition macros */
#define ERROR_MOR(ERR,CODE,ST)		{\
					if (ST[0] != EOS) 	 \
						fprintf(stderr,"\n****error: %s\n\t%s\n****\n", ERR, ST);	\
					else															\
						fprintf(stderr,"\n**error: %s\n", ERR);						\
					return(CODE);   }

#define ERROR_COND(COND,ERR,CODE) if (COND) { ERROR_MOR(ERR,CODE,""); }

/* codes for conditions on entries */

#define LEXSURF_CHK 0
#define LEXCAT_CHK 1
#define LEXSURF_NOT 2
#define LEXCAT_NOT 3
#define STARTSURF_CHK 4
#define NEXTSURF_CHK 5
#define STARTSURF_NOT 6
#define NEXTSURF_NOT 7
#define STARTCAT_CHK 8
#define NEXTCAT_CHK 9
#define STARTCAT_NOT 10
#define NEXTCAT_NOT 11
#define MATCHCAT 12
#define NOT 2  /* amount to increment check to make it check_not */

/* conditions on drules */
#define PREVSURF_CHK 20
#define PREVCAT_CHK 21
#define PREVSURF_NOT 22
#define PREVCAT_NOT 23
#define CURSURF_CHK 24
#define CURCAT_CHK 25
#define CURSURF_NOT 26
#define CURCAT_NOT 27

/* codes for operations on entries */

#define COPY_LEXCAT 0
#define COPY_STARTCAT 1
#define COPY_NEXTCAT 2
#define ADD_CAT 3
#define DEL_CAT 4
#define SET_CAT 5
#define ADD_START_FEAT 6
#define ADD_NEXT_FEAT 7

/* codes for rule-types */

#define START_RULE 0
#define END_RULE 1
//#define DONT_ADD_NEXTSURF 2

/* usefule typedefs */
#define BOOL int
#define STRING char
//typedef char STRING;
typedef unsigned short FEATTYPE;

/* data structures */

#ifndef MLEX_H
/* PARSE_REC used to hold intermediate stages of parse of word */
typedef struct parse_rec{
	FEATTYPE cat[MAXCAT];
	char parse[MAX_WORD];
	char trans[MAX_WORD];
	char comp[MAX_WORD];
	struct rulepack *rp;
} PARSE_REC, *PARSE_REC_PTR;
#endif

/* lxs */
/* structure used to convert long into chars */
struct LongToChar {
    unsigned char c1, c2, c3, c4;
} ;

/* structure used to convert integers into chars */
struct ShortToChar {
    unsigned char c1, c2;
} ;

typedef struct lex_node {
	int letter;
	struct entry_list *entries;
	short num_letters;
	struct lex_node *next_lett;
} LEX_REC, *LEX_PTR;

typedef struct lex_fnode {
    char used;
    unsigned long cnt;
    struct lex_node *next_lett;
} LEXF_REC, *LEXF_PTR;
/* lxs */

/* letter tree node record */
typedef struct trie_node {
	int num_letters;
	struct letter_rec *letters;
	struct entry_list *entries;
} TRIE_NODE, *TRIE_PTR;

/* array of letter values */
typedef struct letter_rec {
	int letter;
	struct trie_node *t_node;
} LETTER_REC, *LETTER_PTR;

typedef struct entry_list{
	struct entry_list* next_elist;
	FEATTYPE *entry;
	STRING *stem;
	STRING *trans;
	STRING *comp;
} ENTRY_LIST, *ELIST_PTR;

/* records for storing feature value pair informaion */
typedef struct feat_node {
	int bf;         /* balance field */
	int code;
	struct feat_node *left;
	struct feat_node *right;
	struct val_node *values;
	STRING *name;
} FEAT_NODE, *FEAT_PTR;

typedef struct val_node {
	int code;
	struct val_node *next;
	STRING *name;
} VAL_NODE, *VAL_PTR;

/* data structures used by pattern matching routines */

typedef struct variable_rec{
	char var_name[MAX_VARNAME];
	char var_pat[MAX_PATLEN];
	char var_inst[MAX_STRLEN]; 
	short var_index;
} VAR_REC, *VAR_PTR;

typedef struct segment_rec{
	int index; 
	STRING *end;
} SEG_REC, *SEG_PTR;

/* data structures for arules */

typedef struct arule_file {
//	char arfname[FNSize];
	char isDefault;
	struct arule *arules_list;
	struct arule_file *next_file;
} ARULE_FILE, *ARULE_FILE_PTR;

typedef struct arule {
	STRING *name;
	char isPostRule;
	STRING *post_rule_name;
	struct arule *post_rule_ptr;
	struct arule_clause *clauses;
	struct arule *next_rule;
} ARULES, *ARULES_PTR;

typedef struct arule_clause {
	struct lex_entry *lex;
	struct allo_entry *allos;
	struct arule_clause *next_clause;
} ARULE_CLAUSE, *ARULE_CLAUSE_PTR;

typedef struct lex_entry {
	int cond_type;
	struct cat_cond_rec *cat_cond;
	STRING *surf_cond;
} LEX_ENTRY, *LEX_ENTRY_PTR;

typedef struct cat_cond_rec {
	int cond_type;
	struct cat_cond_rec *next_cond;
	FEATTYPE *fvp;
} CAT_COND, *CAT_COND_PTR;

typedef struct allo_entry {
	struct allo_entry *next_allo;
	struct cat_op_rec *cat_op;
	STRING *surf_template;
	STRING *stem_template;
} ALLO_ENTRY, *ALLO_ENTRY_PTR;

typedef struct cat_op_rec {
	struct cat_op_rec *next_op;
	int op_type;
	FEATTYPE *fvp;
} CAT_OP, *CAT_OP_PTR;

/* RESULT_REC used to hold output of rules */
typedef struct result_rec{
	char *name;
	LEX_ENTRY_PTR clause_type;
	char surface[MAX_WORD];
	FEATTYPE cat[MAXCAT];
	char stem[MAX_WORD];
	char trans[MAX_WORD];
	char comp[MAX_WORD];
} RESULT_REC, *RESULT_REC_PTR;

/* data structures for crules */

typedef struct crule {
	int rule_type;
	struct crule_clause *clauses;
	struct crule *next_rule;
	STRING *name;
} CRULE, *CRULE_PTR;

typedef struct crule_clause {
	struct crule_cond *conditions;
	struct crule_op *operations;
	struct rulepack *rulepackages;
	struct crule_clause *next_clause;
} CRULE_CLAUSE, *CRULE_CLAUSE_PTR;

typedef struct crule_cond {
	struct crule_cond *next_cond;
	int cond_type;
	FEATTYPE *op_cat;
	STRING *op_surf;
} CRULE_COND, *CRULE_COND_PTR;

typedef struct crule_op {
	struct crule_op *next_op;
	int op_type;
	FEATTYPE *data;
} CRULE_OP, *CRULE_OP_PTR;

typedef struct rulepack {
	struct crule *rule_ptr;
	struct rulepack *next_rule;
	STRING *rulename;
} RULEPACK, *RULEPACK_PTR;

/* data structures for drules */

typedef struct drule {
	struct crule_cond *choose_list;
	struct crule_cond *prev_cond_list;
	struct crule_cond *next_cond_list;
	struct drule *next_rule;
	STRING *name;
} DRULE, *DRULE_PTR;

extern int DEBUG_CLAN;
extern FILE *debug_fp;
extern STRING estr[];

#include "proto.h"
