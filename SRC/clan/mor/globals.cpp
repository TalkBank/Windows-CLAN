/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include "cu.h"
#include "morph.h"

/* global variables for lexicon (as trie structure) */

LEX_PTR lex_root;
LEXF_PTR lex_frow;
TRIE_PTR trie_root;   /* root of letter trie */
long trie_node_ctr;   /* number of trie nodes */
long letter_ctr;   /* number of letter nodes */
long trie_entry_ctr;  /* number of entry nodes */

FEAT_PTR features;    /* root of feature tables */

FEAT_PTR *feat_codes;   /* array of pointers into feature table tree */

int feat_node_ctr;   /* number of feature nodes */
int val_node_ctr;  /* number of value nodes */

/* global variables for pattern matching routines */

STRING **match_stack;   /* workspace used by pattern matcher */
STRING **next_match;        /* stack pointer- next free space in stack */
SEG_REC *word_segments;  /* substrings that a word is parsed into */

VAR_REC *var_tab; /* table of variable names, expansions, & matches */


/* global variables for allo rules */

ARULE_FILE_PTR ar_files_list;

CRULE_PTR crule_list;
int crules_linenum;

RULEPACK_PTR startrules_list;
RULEPACK_PTR endrules_list;

DRULE_PTR drule_list;
int drules_linenum;

RESULT_REC *result_list; /* results of arules or crules */
int result_index; /* keeps track of number of results */

int DEBUG_CLAN;
FILE *debug_fp;
long rt_entry_ctr;

long mem_ctr;

