/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- mortags.hpp	- v1.0.0 - november 1998
// library of functions to compute the tag format of mor and clan files.
|=========================================================================================================================*/

#ifndef __MORTAGS_HPP__
#define __MORTAGS_HPP__

#include "mor.hpp"

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=mac68k 
#endif
*/

struct mortag {		// contains an tag coming from a %mor: line (with the word)
	char*  MT;	// main tag
	int    nSP;	// number of supplementary parts
	char** SP;	// supplementary parts
	char*  SPt;	// type of supplementary parts
	char*  GR;	// generic root
	char*  translation;	// translation of main word (follows sign '=')
	mortag() { MT=0; nSP=0; SP=0; SPt=0; GR=0; translation=0; }
};

struct ambmortag {	// contains a set of mortags.
	mortag*  A;	// array
	int	nA;	// size
};

/* 2019-10-10
#ifdef _MAC_CODE
#pragma options align=reset 
#endif
*/

// this function decompose an mortag
int split_mortag(char* strmortag, char* &maintag, char** subparts, char* subpartstype, char* &genericroot, int maxofsp, char* &translation);
// this function creates a mortag in memory using the dynalloc allocation scheme
int make_mortag(char* str, mortag* mt);
int make_mortag_from_heap(char* str, mortag* mt);
void delete_mortag_from_heap(mortag* mt);
// this function creates an ambiguous mortag in memory using the dynalloc allocation scheme
int make_ambmortag(char* str, ambmortag* amt);
int* ambmortag_to_multclass(ambmortag* amt, int* ma);

// these functions are used to select in %mor:tags what can be processed by POST.
int filter_ok( char* sp );
int filter_ok_out( char* sp );
// char* filter_mortag( mortag* mt );
void sort_and_pack_in_string( char** S1, int n, char* st1, int sz_st1, const char* sep );
int filter_and_pack_ambmortag( ambmortag* a );
int is_eq_filtered( mortag* a, mortag* b);
char* mortag_to_string_filtered( mortag* mt, char* storage );
void filter_add_all_affixes_of_mortag( mortag* mt );
void filter_add_all_affixes( ambmortag* amt );

mortag* filter_mortag( mortag* mt );
int filter_and_pack_ambmortag_keep_mainword( ambmortag* a );
mortag* filter_mortag_keep_mainword( mortag* mt );
int mortags_cmp( mortag* mt1, mortag* mt2 );
char* mortag_to_string( mortag* mt );
int is_unknown_word( mortag* mt );
int is_pct( mortag* mt );
int is_in( mortag* a, mortag* b ); // is b in a
int is_in_plus_gr( mortag* a, mortag* b ); // is b in a
int is_in_amb( ambmortag* a, mortag* b);
int is_eq( mortag* a, mortag* b );
int is_eq_no_GR( mortag* a, mortag* b );
int where_in_amb( ambmortag* a, mortag* b);

int prepare_ambtagword( ambmortag* a, mortag* m, char* result );
void prepare_ambtagword( ambmortag* a, char* result );

// print functions
void print_mortag( FILE* out, mortag* mt );
void print_ambmortag( FILE* out, ambmortag* amt );
void print_mortag_2( FILE* out, mortag* mt );
void print_ambmortag_2( FILE* out, ambmortag* amt );

// this function split a line into parts.
int split_with(char* line, char** words, const char* seps, int maxofwords=4096);
int split_with_a(char* line, char** words, char* res, const char* seps, const char* sepskeep, const char* sepskeepatend, int maxofwords=4096, int maxofres=4096 );
int split_Prefixes(char* line, char** words, int maxofwords=4096);

// these functions create multiple word categories
int make_multi_categ(char* initial_string, char* &st);
int make_multi_categ_single(char* initial_string, char* &st);

// these functions split multiple word tags into single word tags
// int split_multiple_mortag(char* initial_string, char** &st, int withtagsuffix);
// int split_multiple_mortag_single(char* initial_string, char** &st, int withtagsuffix);

#endif
