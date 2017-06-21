/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// brilltagger.cpp
// open Brill's rule database.

#include "system.hpp"

// #define DEBUG_PRINT_brilltagger

#include "database.hpp"

/* This file is a modifed version of the orginal program of Eric Brill (see below) */
/* It has been modifed in order to be interfaced with CLAN - from CHILDES */

/* Copyright @ 1993 MIT and University of Pennsylvania */
/* Written by Eric Brill */

#include "darray.hpp"
#include "registry.hpp"
#include "memory.hpp"

int process_a_word( int size, int count, char** word_corpus_array, TAG* tag_corpus_array, int nbofrule, AMBTAG* A );
int process_a_rule( char* text_of_rule, int size, int count, char** word_corpus_array, TAG* tag_corpus_array, AMBTAG* A );
int process_a_rule_along_the_text( char* text_of_rule, char** word_corpus_array, TAG* tag_corpus_array, AMBTAG* A, int size_corpus );

#define MAXTAGLEN 256  /* max char length of pos tags */
#define MAXWORDLEN 256 /* max char length of words */

static int RESTRICT_MOVE = 1;   /* if this is set to 1, then a rule "change a tag */
                          /* from x to y" will only apply to a word if:
                             a) the word is unknown 
                             b) the word was tagged with y at least once in
                             the training set
                             When training on a very small corpus, better
                             performance might be obtained by setting this to
                             0, but for most uses it should be set to 1 */


static char staart[] = ".";
#define staart_tag "pct"

static int is_in_ambiguous(TAG nouv, AMBTAG* ambtag_array, int itext )
{
	// find if nouv is in ambtag_array[itext]
	int n = number_of_ambtag(ambtag_array[itext]);
	for (int i=0; i<n; i++ )
		if ( nouv == nth_of_ambtag( ambtag_array[itext], i ) ) return 1;
	return 0;
}

static int is_unknown_word( AMBTAG* ambtag_array, int itext )
{
	// find if nouv is in ambtag_array[itext]
	int i;
	int n = number_of_ambtag(ambtag_array[itext]);
// msg ("Nth of amb=%d\n", n);
	for (i=0; i<n; i++ ) {
		if ( !_words_classnames_def_noms_[i] ) return 0;
		TAG t = classname_to_tag( _words_classnames_def_noms_ [i] );
// msg ("%d] = /%s/ /%d/ - /%d/\n", i, _words_classnames_def_noms_ [i], t, nth_of_ambtag( ambtag_array[itext], i ) );
		if ( t != nth_of_ambtag( ambtag_array[itext], i ) ) return 0;
	}
// msg ("Fin %d] = /%lx/\n", i, _words_classnames_def_noms_ [i] );
	if ( _words_classnames_def_noms_[i] || i != n ) return 0;
	return 1;
}

// the lexicon

inline void change_the_tag(TAG* theentry, TAG thetag, int theposition)
{
  theentry[theposition] = thetag;
}

int process_with_brills_rules(char** words, TAG* original, TAG* newversion, AMBTAG* A, int size, int style_analyze) // uses Brill's rules on a previously computed sentence.
{
        char**  word_corpus_array;
        TAG*    tag_corpus_array;
        int     word_corpus_array_index;
        int     count;

#ifdef DBGsol
{
for (int o=0;o<size;o++)  msg( " %s %d", words[o], original[o] );
msg("\n");
}
#endif
        word_corpus_array = (char **)dynalloc(sizeof(char *) * (size+6));
        tag_corpus_array = (TAG *)dynalloc(sizeof(TAG) * (size+6));
        word_corpus_array_index = 0;


        word_corpus_array[word_corpus_array_index] = staart;
        tag_corpus_array[word_corpus_array_index++] = classname_to_tag(staart_tag);
        word_corpus_array[word_corpus_array_index] = staart;
        tag_corpus_array[word_corpus_array_index++] = classname_to_tag(staart_tag);
        word_corpus_array[word_corpus_array_index] = staart;
        tag_corpus_array[word_corpus_array_index++] = classname_to_tag(staart_tag);

        for (count=0; count<size; count++) {
                word_corpus_array[word_corpus_array_index] = words[count];
                tag_corpus_array[word_corpus_array_index++] = original[count];
        }

        word_corpus_array[word_corpus_array_index] = staart;
        tag_corpus_array[word_corpus_array_index++] = classname_to_tag(staart_tag);
        word_corpus_array[word_corpus_array_index] = staart;
        tag_corpus_array[word_corpus_array_index++] = classname_to_tag(staart_tag);
        word_corpus_array[word_corpus_array_index] = staart;
        tag_corpus_array[word_corpus_array_index++] = classname_to_tag(staart_tag);

	if ( style_analyze == FullAndSlow ) {
		int nbr = nb_brillrules();
		for (int i=1; i<=nbr; i++) {
			char ctn[256];
			if (!get_brillrule_by_number( i, ctn )) continue; // may happen if some rules have been suppressed.
			process_a_rule_along_the_text( ctn, word_corpus_array, tag_corpus_array, A, size );
		}
	} else {
		int processed;
		int* nb_of_rules = new int [size+6];
		memset( nb_of_rules, 0, (size+6)*sizeof(int) );
		do {
			processed = 0;
        		for (count = 3; count < size+3; ++count) {
				int nb = process_a_word( size, count, word_corpus_array, tag_corpus_array, nb_of_rules[count], A );
				if (nb != -1) {
					nb_of_rules[count] = nb;
					processed = 1;
				}
			}
		} while (processed);
		delete [] nb_of_rules;
	}

        for (count=0; count<size; count++) {
                newversion[count] = tag_corpus_array[count+3];
        }

        return 1;
}

static int get_last_number( char* ctn )
{
	char* p = strrchr( ctn, ' ' );
	p++;
	return atoi(p);
}

int process_a_word( int size, int count, char** word_corpus_array, TAG* tag_corpus_array, int prev_numrule, AMBTAG* A )
{
	char nt[24];
	char ctn[256];
	int n_record;
	long* record;

	tag_to_classname( tag_corpus_array[count], nt );
#ifdef DEBUG_PRINT_brilltagger
/** PRINT WORD, TAG, PREVTAG & WORD, NEXTTAG & WORD **/
msg( "[%d] W=[%s], C=[%d] /%s/, WP=[%s], CP=[%d], WN=[%s], CN=[%d]\n", count, word_corpus_array[count], tag_corpus_array[count], nt, word_corpus_array[count-1], tag_corpus_array[count-1], word_corpus_array[count+1], tag_corpus_array[count+1] );
#endif
	record = get_brillrule( word_corpus_array[count], nt, ctn, n_record );
#ifdef DEBUG_PRINT_brilltagger
msg ("found %s-%s ? = %lx\n", word_corpus_array[count], nt, record );
#endif
	if (record) {
		do {
			// get # of rule.
			int n = get_last_number( ctn );
			if ( n > prev_numrule ) {
				if ( process_a_rule( ctn, size, count, word_corpus_array, tag_corpus_array, A ) ) {
					delete [] record;
					return n;
				}
			}
		} while ( get_next_brillrule(ctn, n_record, record) );
		delete [] record;
	}

	record = get_brillrule( AnyWord, nt, ctn, n_record );
#ifdef DEBUG_PRINT_brilltagger
msg ("found AnyWord-%s ? = %lx\n", nt, record );
#endif
	if (record) {
		// Find an AnyWord rule to apply to tag "nt"
		do {
			// get # of rule.
			int n = get_last_number( ctn );
			if ( n > prev_numrule ) {
				if ( process_a_rule( ctn, size, count, word_corpus_array, tag_corpus_array, A ) ) {
					delete [] record;
					return n;
				}
			}
		} while ( get_next_brillrule(ctn, n_record, record) );
		delete [] record;
	}
	return -1;
}

int process_a_rule( char* text_of_rule, int size, int count, char** word_corpus_array, TAG* tag_corpus_array, AMBTAG* ambtag_array )
{
#ifdef DEBUG_PRINT_brilltagger
msg ("process rule : <%s>\n", text_of_rule );
#endif
	/* analyse the rule */
	char*	word_to_check;
        char    when[MAXWORDLEN],
		curwd[MAXWORDLEN],
		word[MAXWORDLEN],
		nextw1[MAXWORDLEN],
		nextw2[MAXWORDLEN],
		prevw1[MAXWORDLEN],
		prevw2[MAXWORDLEN];
        TAG     old, nouv,
		tag, lft, rght,
                prev1, prev2, next1,
                next2, curtag;

        char    atempstr2[256];

	const int TOCHECK = 0;
	const int OLD = 1;
	const int NOUV = 2;
	const int WHEN = 3;
	const int ARG1 = 4;
	const int ARG2 = 5;

	// decomposes a RULE.

	char* split_ptr[12];
	split_with( text_of_rule, split_ptr, " \t", 12 );
        word_to_check = split_ptr[TOCHECK];
        old = classname_to_tag(split_ptr[OLD]);
	nouv = classname_to_tag(split_ptr[NOUV]);
	strcpy(when, split_ptr[WHEN]);

	if ( strcmp(when, "NEXTTAG") == 0 || strcmp(when, "NEXT2TAG") == 0 ||
	        strcmp(when, "NEXT1OR2TAG") == 0 || strcmp(when, "NEXT1OR2OR3TAG") == 0 ||
	        strcmp(when, "PREVTAG") == 0 || strcmp(when, "PREV2TAG") == 0 ||
	        strcmp(when, "PREV1OR2TAG") == 0 || strcmp(when, "PREV1OR2OR3TAG") == 0) {
	    tag = classname_to_tag(split_ptr[ARG1]);
	} else if ( strcmp(when, "NEXTWD") == 0 || strcmp(when,"CURWD") == 0 ||
	        strcmp(when, "NEXT2WD") == 0 || strcmp(when, "NEXT1OR2WD") == 0 ||
	        strcmp(when, "NEXT1OR2OR3WD") == 0 || strcmp(when, "PREVWD") == 0 ||
	        strcmp(when, "PREV2WD") == 0 || strcmp(when, "PREV1OR2WD") == 0 ||
	        strcmp(when, "PREV1OR2OR3WD") == 0) {
	    strcpy(word, split_ptr[ARG1]);
	} else if (strcmp(when,"WDANDTAG")== 0 ) {
	    tag = classname_to_tag(split_ptr[ARG2]);
	    strcpy(word,split_ptr[ARG1]);
	} else if (strcmp(when, "SURROUNDTAG") == 0) {
	    lft = classname_to_tag(split_ptr[ARG1]);
	    rght = classname_to_tag(split_ptr[ARG2]);
	} else if (strcmp(when, "PREVBIGRAM") == 0) {
	    prev1 = classname_to_tag(split_ptr[ARG1]);
	    prev2 = classname_to_tag(split_ptr[ARG2]);
	} else if (strcmp(when, "NEXTBIGRAM") == 0) {
	    next1 = classname_to_tag(split_ptr[ARG1]);
	    next2 = classname_to_tag(split_ptr[ARG2]);
	} else if (strcmp(when,"WDPREVTAG") == 0) {
	    prev1 = classname_to_tag(split_ptr[ARG1]);
	    strcpy(word,split_ptr[ARG2]);
	} else if (strcmp(when,"WDNEXTTAG") == 0) {
	    strcpy(word,split_ptr[ARG1]);
	    next1 = classname_to_tag(split_ptr[ARG2]);
	} else if (strcmp(when,"WDAND2TAGBFR") == 0) {
	    prev2 = classname_to_tag(split_ptr[ARG1]);
	    strcpy(word,split_ptr[ARG2]);
	} else if (strcmp(when,"WDAND2TAGAFT") == 0) {
	    next2 = classname_to_tag(split_ptr[ARG2]);
	    strcpy(word,split_ptr[ARG1]);
	} else if (strcmp(when,"LBIGRAM") == 0) {
	    strcpy(prevw1, split_ptr[ARG1]);
	    strcpy(word,split_ptr[ARG2]);
	} else if (strcmp(when,"RBIGRAM") == 0) {
	    strcpy(word,split_ptr[ARG1]);
	    strcpy(nextw1, split_ptr[ARG2]);
	} else if (strcmp(when,"WDAND2BFR") == 0) {
	    strcpy(prevw2, split_ptr[ARG1]);
	    strcpy(word,split_ptr[ARG2]);
	} else if (strcmp(when,"WDAND2AFT") == 0) {
	    strcpy(nextw2, split_ptr[ARG2]);
	    strcpy(word,split_ptr[ARG1]);
	}
	curtag = tag_corpus_array[count];
	if ( curtag != old ) {
/**		char ttt[24];
		tag_to_classname( tag_corpus_array[count], ttt );
		msg( "ERROR curtag != old !!!!\n");
		msg( "%s %s ![%s]!![%s]!\n", split_ptr[WHEN], split_ptr[TOCHECK], split_ptr[OLD], word_corpus_array[count], ttt );
**/		return 0;
	}
	strcpy(curwd,word_corpus_array[count]);
		char tmp[128];
	tag_to_classname(nouv,tmp);
	sprintf(atempstr2,"%s %s",curwd,tmp);

#ifdef DEBUG_PRINT_brilltagger
msg ("1) /%s/ /%s/ %d %d\n",curwd, atempstr2, is_unknown_word( ambtag_array, count-3 ), is_in_ambiguous(nouv, ambtag_array, count-3 ) );
#endif

	// WARNING: ambtag_array has not been shifted by 3 elements to the right, so the info refered in ambtag_array[count-3] is to be compared with info refered by tag_corpus_array[count]
	/*** ALWAYS PROCEED IF THE WORD IS AN UNKNOWN WORD ***/
	if ( ! is_unknown_word( ambtag_array, count-3 ) ) {
		if ( RESTRICT_MOVE==1 ) { // check if tag is in authorized tags for that word.
			if ( ! is_in_ambiguous(nouv, ambtag_array, count-3 ) ) return 0;
		}
	}
#ifdef DEBUG_PRINT_brilltagger
msg ("3) MODIFY /%s/ unk=%s\n", atempstr2, is_unknown_word( ambtag_array, count-3 ) ? "yes" : "no" );
#endif

	if (strcmp(when, "SURROUNDTAG") == 0) {
	        if ( lft == tag_corpus_array[count - 1] &&
	             rght == tag_corpus_array[count + 1] ) {
	        	change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXTTAG") == 0) {
	        if ( tag == tag_corpus_array[count + 1] ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "WDANDTAG") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             tag == tag_corpus_array[count] ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "CURWD") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXTWD") == 0) {
	        if ( strcmp(word, word_corpus_array[count + 1]) == 0) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "RBIGRAM") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             strcmp(nextw1, word_corpus_array[count+1]) == 0 ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "WDNEXTTAG") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             next1 == tag_corpus_array[count+1] ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "WDAND2AFT") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             strcmp(nextw2, word_corpus_array[count+2]) == 0 ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "WDAND2TAGAFT") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             next2 == tag_corpus_array[count+2] ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXT2TAG") == 0) {
	        if ( tag == tag_corpus_array[count + 2] ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXT2WD") == 0) {
	        if ( strcmp(word, word_corpus_array[count + 2]) == 0 ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXTBIGRAM") == 0) {
	        if ( next1 == tag_corpus_array[count + 1] &&
	             next2 == tag_corpus_array[count + 2] ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXT1OR2TAG") == 0) {
	        if ( tag == tag_corpus_array[count + 1] ||
	             tag == tag_corpus_array[count + 2] ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXT1OR2WD") == 0) {
	        if ( strcmp(word, word_corpus_array[count + 1]) == 0 ||
	             strcmp(word, word_corpus_array[count + 2]) == 0 ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXT1OR2OR3TAG") == 0) {
	        if ( tag == tag_corpus_array[count + 1] ||
	             tag == tag_corpus_array[count + 2] ||
	             tag == tag_corpus_array[count + 3] ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "NEXT1OR2OR3WD") == 0) {
	        if ( strcmp(word, word_corpus_array[count + 1]) == 0 ||
	             strcmp(word, word_corpus_array[count + 2]) == 0 ||
	             strcmp(word, word_corpus_array[count + 3]) == 0 ) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREVTAG") == 0) {
	        if ( tag == tag_corpus_array[count - 1]) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREVWD") == 0) {
	        if ( strcmp(word, word_corpus_array[count - 1]) == 0) {
	                change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "LBIGRAM") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             strcmp(prevw1, word_corpus_array[count-1] ) == 0 ) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "WDPREVTAG") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             prev1 == tag_corpus_array[count-1]) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "WDAND2BFR") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             strcmp(prevw2, word_corpus_array[count-2]) == 0 ) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "WDAND2TAGBFR") == 0) {
	        if ( strcmp(word, word_corpus_array[count]) == 0 &&
	             prev2 == tag_corpus_array[count-2]) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREV2TAG") == 0) {
	        if ( tag == tag_corpus_array[count - 2] ) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREV2WD") == 0) {
	        if ( strcmp(word, word_corpus_array[count - 2]) == 0) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREV1OR2TAG") == 0) {
	        if ( tag == tag_corpus_array[count - 1] ||
	             tag == tag_corpus_array[count - 2]) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREV1OR2WD") == 0) {
	        if ( strcmp(word, word_corpus_array[count - 1]) == 0 ||
	             strcmp(word, word_corpus_array[count - 2]) == 0) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREV1OR2OR3TAG") == 0) {
	        if ( tag == tag_corpus_array[count - 1] ||
	             tag == tag_corpus_array[count - 2] ||
	             tag == tag_corpus_array[count - 3]) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREV1OR2OR3WD") == 0) {
	        if ( strcmp(word, word_corpus_array[count - 1]) == 0 ||
	             strcmp(word, word_corpus_array[count - 2]) == 0 ||
	             strcmp(word, word_corpus_array[count - 3]) == 0) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else if (strcmp(when, "PREVBIGRAM") == 0) {
	        if ( prev2 == tag_corpus_array[count - 1] &&
	             prev1 == tag_corpus_array[count - 2]) {
	                 change_the_tag(tag_corpus_array, nouv, count);
			return 1;
		}
	} else
		fprintf(stderr, "ERROR: %s is not an allowable transform type\n", when);
	return 0;
}


int process_a_rule_along_the_text( char* text_of_rule, char** word_corpus_array, TAG* tag_corpus_array, AMBTAG* ambtag_array, int size_corpus )
{
	// msg ("process rule : <%s>\n", text_of_rule );

	/* analyse the rule */
	char*	word_to_check;
        char    when[MAXWORDLEN],
		curwd[MAXWORDLEN],
		word[MAXWORDLEN],
		nextw1[MAXWORDLEN],
		nextw2[MAXWORDLEN],
		prevw1[MAXWORDLEN],
		prevw2[MAXWORDLEN];
        TAG     old, nouv,
		tag, lft, rght,
                prev1, prev2, next1,
                next2, curtag;

        char    atempstr2[256];

	const int TOCHECK = 0;
	const int OLD = 1;
	const int NOUV = 2;
	const int WHEN = 3;
	const int ARG1 = 4;
	const int ARG2 = 5;

	// decomposes a RULE.

	char* split_ptr[12];
	split_with( text_of_rule, split_ptr, " \t", 12 );
        word_to_check = split_ptr[TOCHECK];
        old = classname_to_tag(split_ptr[OLD]);
	nouv = classname_to_tag(split_ptr[NOUV]);
	strcpy(when, split_ptr[WHEN]);

	if ( strcmp(when, "NEXTTAG") == 0 || strcmp(when, "NEXT2TAG") == 0 ||
	        strcmp(when, "NEXT1OR2TAG") == 0 || strcmp(when, "NEXT1OR2OR3TAG") == 0 ||
	        strcmp(when, "PREVTAG") == 0 || strcmp(when, "PREV2TAG") == 0 ||
	        strcmp(when, "PREV1OR2TAG") == 0 || strcmp(when, "PREV1OR2OR3TAG") == 0) {
	    tag = classname_to_tag(split_ptr[ARG1]);
	} else if ( strcmp(when, "NEXTWD") == 0 || strcmp(when,"CURWD") == 0 ||
	        strcmp(when, "NEXT2WD") == 0 || strcmp(when, "NEXT1OR2WD") == 0 ||
	        strcmp(when, "NEXT1OR2OR3WD") == 0 || strcmp(when, "PREVWD") == 0 ||
	        strcmp(when, "PREV2WD") == 0 || strcmp(when, "PREV1OR2WD") == 0 ||
	        strcmp(when, "PREV1OR2OR3WD") == 0) {
	    strcpy(word, split_ptr[ARG1]);
	} else if (strcmp(when,"WDANDTAG")== 0 ) {
	    tag = classname_to_tag(split_ptr[ARG2]);
	    strcpy(word,split_ptr[ARG1]);
	} else if (strcmp(when, "SURROUNDTAG") == 0) {
	    lft = classname_to_tag(split_ptr[ARG1]);
	    rght = classname_to_tag(split_ptr[ARG2]);
	} else if (strcmp(when, "PREVBIGRAM") == 0) {
	    prev1 = classname_to_tag(split_ptr[ARG1]);
	    prev2 = classname_to_tag(split_ptr[ARG2]);
	} else if (strcmp(when, "NEXTBIGRAM") == 0) {
	    next1 = classname_to_tag(split_ptr[ARG1]);
	    next2 = classname_to_tag(split_ptr[ARG2]);
	} else if (strcmp(when,"WDPREVTAG") == 0) {
	    prev1 = classname_to_tag(split_ptr[ARG1]);
	    strcpy(word,split_ptr[ARG2]);
	} else if (strcmp(when,"WDNEXTTAG") == 0) {
	    strcpy(word,split_ptr[ARG1]);
	    next1 = classname_to_tag(split_ptr[ARG2]);
	} else if (strcmp(when,"WDAND2TAGBFR") == 0) {
	    prev2 = classname_to_tag(split_ptr[ARG1]);
	    strcpy(word,split_ptr[ARG2]);
	} else if (strcmp(when,"WDAND2TAGAFT") == 0) {
	    next2 = classname_to_tag(split_ptr[ARG2]);
	    strcpy(word,split_ptr[ARG1]);
	} else if (strcmp(when,"LBIGRAM") == 0) {
	    strcpy(prevw1, split_ptr[ARG1]);
	    strcpy(word,split_ptr[ARG2]);
	} else if (strcmp(when,"RBIGRAM") == 0) {
	    strcpy(word,split_ptr[ARG1]);
	    strcpy(nextw1, split_ptr[ARG2]);
	} else if (strcmp(when,"WDAND2BFR") == 0) {
	    strcpy(prevw2, split_ptr[ARG1]);
	    strcpy(word,split_ptr[ARG2]);
	} else if (strcmp(when,"WDAND2AFT") == 0) {
	    strcpy(nextw2, split_ptr[ARG2]);
	    strcpy(word,split_ptr[ARG1]);
	}

	for ( int itext=3; itext<size_corpus+3; itext++ ) {

		curtag = tag_corpus_array[itext];
		if ( curtag != old ) continue;

		strcpy(curwd,word_corpus_array[itext]);
		char tmp[128];
		tag_to_classname(nouv,tmp);
		sprintf(atempstr2,"%s %s",curwd,tmp);

#ifdef DEBUG_PRINT_brilltagger
msg ("1) /%s/ /%s/ %d %d\n",curwd, atempstr2, is_unknown_word( ambtag_array, itext-3 ), is_in_ambiguous(nouv, ambtag_array, itext-3 ) );
#endif

		// WARNING: ambtag_array has not been shifted by 3 elements to the right, so the info refered in ambtag_array[itext-3] is to be compared with info refered by tag_corpus_array[itext]
		/*** ALWAYS PROCEED IF THE WORD IS AN UNKNOWN WORD ***/
		if ( ! is_unknown_word( ambtag_array, itext-3 ) ) {
			if ( RESTRICT_MOVE==1 ) { // check if tag is in authorized tags for that word.
				if ( ! is_in_ambiguous(nouv, ambtag_array, itext-3 ) ) return 0;
			}
		}
#ifdef DEBUG_PRINT_brilltagger
msg ("3) MODIFY /%s/ unk=%s\n", atempstr2, is_unknown_word( ambtag_array, itext-3 ) ? "yes" : "no" );
#endif

		if (strcmp(when, "SURROUNDTAG") == 0) {
		        if ( lft == tag_corpus_array[itext - 1] &&
		             rght == tag_corpus_array[itext + 1] ) {
		        	change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXTTAG") == 0) {
		        if ( tag == tag_corpus_array[itext + 1] ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "WDANDTAG") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             tag == tag_corpus_array[itext] ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "CURWD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXTWD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext + 1]) == 0) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "RBIGRAM") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             strcmp(nextw1, word_corpus_array[itext+1]) == 0 ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "WDNEXTTAG") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             next1 == tag_corpus_array[itext+1] ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "WDAND2AFT") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             strcmp(nextw2, word_corpus_array[itext+2]) == 0 ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "WDAND2TAGAFT") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             next2 == tag_corpus_array[itext+2] ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXT2TAG") == 0) {
		        if ( tag == tag_corpus_array[itext + 2] ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXT2WD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext + 2]) == 0 ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXTBIGRAM") == 0) {
		        if ( next1 == tag_corpus_array[itext + 1] &&
		             next2 == tag_corpus_array[itext + 2] ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXT1OR2TAG") == 0) {
		        if ( tag == tag_corpus_array[itext + 1] ||
		             tag == tag_corpus_array[itext + 2] ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXT1OR2WD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext + 1]) == 0 ||
		             strcmp(word, word_corpus_array[itext + 2]) == 0 ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXT1OR2OR3TAG") == 0) {
		        if ( tag == tag_corpus_array[itext + 1] ||
		             tag == tag_corpus_array[itext + 2] ||
		             tag == tag_corpus_array[itext + 3] ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "NEXT1OR2OR3WD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext + 1]) == 0 ||
		             strcmp(word, word_corpus_array[itext + 2]) == 0 ||
		             strcmp(word, word_corpus_array[itext + 3]) == 0 ) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREVTAG") == 0) {
		        if ( tag == tag_corpus_array[itext - 1]) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREVWD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext - 1]) == 0) {
		                change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "LBIGRAM") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             strcmp(prevw1, word_corpus_array[itext-1] ) == 0 ) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "WDPREVTAG") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             prev1 == tag_corpus_array[itext-1]) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "WDAND2BFR") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             strcmp(prevw2, word_corpus_array[itext-2]) == 0 ) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "WDAND2TAGBFR") == 0) {
		        if ( strcmp(word, word_corpus_array[itext]) == 0 &&
		             prev2 == tag_corpus_array[itext-2]) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREV2TAG") == 0) {
		        if ( tag == tag_corpus_array[itext - 2] ) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREV2WD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext - 2]) == 0) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREV1OR2TAG") == 0) {
		        if ( tag == tag_corpus_array[itext - 1] ||
		             tag == tag_corpus_array[itext - 2]) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREV1OR2WD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext - 1]) == 0 ||
		             strcmp(word, word_corpus_array[itext - 2]) == 0) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREV1OR2OR3TAG") == 0) {
		        if ( tag == tag_corpus_array[itext - 1] ||
		             tag == tag_corpus_array[itext - 2] ||
		             tag == tag_corpus_array[itext - 3]) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREV1OR2OR3WD") == 0) {
		        if ( strcmp(word, word_corpus_array[itext - 1]) == 0 ||
		             strcmp(word, word_corpus_array[itext - 2]) == 0 ||
		             strcmp(word, word_corpus_array[itext - 3]) == 0) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else if (strcmp(when, "PREVBIGRAM") == 0) {
		        if ( prev2 == tag_corpus_array[itext - 1] &&
		             prev1 == tag_corpus_array[itext - 2]) {
		                 change_the_tag(tag_corpus_array, nouv, itext);
				return 1;
			}
		} else
			fprintf(stderr, "ERROR: %s is not an allowable transform type\n", when);
	}
	return 1;
}
