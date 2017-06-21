/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************************
** splitchat.cpp
** normalizes and formats words in a CHAT format file
***********************************************************************/

#include "system.hpp"
#include "tags.hpp"
#include "compound.hpp"

/* three actions */
#define	add_to_word() { result[pt_result++] = line[pt_line++]; lg_cword++; if (pt_result>=maxofresult) return nb_words; }
#define	finish_word() { words[nb_words++] = &result[(pt_result-lg_cword)]; lg_cword=0; result[pt_result++] = '\0';  if (nb_words>=maxofwords) return nb_words; }
#define	skip() { pt_line++; }


/****
	splitting function
****/

/*	In this function separators from the parameter
	'sepsskip' are lost from the original string after segmentation
	'sepskeep' remain in the original string after segmentation (each one is a string itself).
	'sepsbegin' mark the begining of a word but belong to the word thus segmented.
	'sepsend' mark the end of a word but belong to the word thus segmented.

	The output string can be longer than the original string, as a NULL character is added for
	each separator to be kept. For this reason, a copy of the original is done in string parameter 'result'.

	words return an array of the segmented words
	the function returns the number of segmented words

	maxpofwords = maximum size of words
	maxpofresult = maximum size of result
*/
int split_all_formats(char* line, char** words, char* result, const char* sepsskip, const char* sepsbegin, const char* sepsend, const char* sepskeep, int maxofwords, int maxofresult )
{
	/* two states: */
	#define	in_word 1
	#define in_seps 2

	int pt_result = 0;
	int lg_cword = 0;
	int nb_words = 0;
	int pt_line = 0;
	int state = in_seps;

	// run through line
	while (line[pt_line] != '\0') {
		if ( strchrConst( sepsskip, line[pt_line] ) ) {
			if (state==in_seps) {
				skip();
			} else {
				finish_word();
				skip();
				state = in_seps;
			}
		} else if ( strchrConst( sepskeep, line[pt_line] ) ) {
			if (state==in_seps) {
				add_to_word();
				finish_word();
			} else {
				finish_word();
				add_to_word();
				finish_word();
				state = in_seps;
			}
		} else if ( strchrConst( sepsbegin, line[pt_line] ) ) {
			if (state==in_word) finish_word();
			add_to_word();
			state = in_word;
		} else if ( strchrConst( sepsend, line[pt_line] ) ) {
			add_to_word();
			finish_word();
			state = in_seps;
		} else {
			add_to_word();
			state = in_word;
		}
	}

	return nb_words;
}

static const char* special_plus [] = {
	"+...", "+..?", "+!?", "+/.",  "+/?",  "+//.",  "+//?",  "+\"/.",  "+\".",  "+\"",  "+^",  "+<",  "+.",  "+,", "++", (char*)0
};
static const char* special_moins [] = {
	"-?", "-!", "-.", "-'.", "-,.", "-,", "-_", "-'", "--", (char*)0
};
static const char* special_par [] = {
	"(.)", "(..)", "(...)", (char*)0
};
static const char* special_pct [] = {
	".", "!", "?", (char*)0
};
static const char* special_repet [] = {
	"[/]", "[//]", "[///]", (char*)0
};

// tests for all special forms.
static int test_special( char* p_line, const char** special )
{
	int i=0;
	while (special[i]) {
		int j, l = strlen(special[i]);
		for (j=0;j<l;j++) if ( p_line[j]==0 || special[i][j] != p_line[j] ) break;
		if (j==l) return l;
		i++;
	}
	return 0;
}

// tests for all special forms begining with +.
int test_special_plus( char* p_line )
{
	return test_special( p_line, special_plus);
}

// tests for all special forms for repetition
int test_special_repet( char* p_line )
{
	return test_special( p_line, special_repet);
}

// test for punctuation.
int is_chat_punctuation( char *s)
{
	if (  test_special( s, special_plus) 
	   || test_special( s, special_moins) 
	   || test_special( s, special_par) 
	   || test_special( s, special_pct) )
		return 1;
	else
		return 0;
}

int is_bullet(char* s) // test if it is a bullet.
{
	return strchr(s, '\x15') ? 1 : 0;
}

int is_empty(char* s)	// test if empry string.
{
	return s[0] == '\0' ? 1 : 0;
}

/*** Same as above but with all specifics for CHAT
	if (keep_old_compounds) ==> do not use + and - to split words.
	+ is a character that introduces utterance endings
	# is a character that introduces pauses

	\x15 ... \x15 is a word wherever it is
	[ ... ] is a word wherever it is

***/
int split_all_formats_CHAT_special(char* line, char** words, char* result, const char* sepsskip, const char* sepsbegin, const char* sepsend, const char* sepskeep, int maxofwords, int maxofresult )
{
	/* five states */
	#define	in_word 1
	#define in_seps 2
	#define in_pause 3
	#define in_ending 4
	#define in_shift15 5
	#define in_shiftsq 6

	int pt_result = 0;
	int lg_cword = 0;
	int nb_words = 0;
	int pt_line = 0;
	int state = in_seps;

	// run through line
	while (line[pt_line] != '\0') {
		// FIRST test shift \x15
		if ( line[pt_line] == '\x15' ) {
			if ( state == in_shift15 ) {
				add_to_word();
				finish_word();
				state = in_seps;
			} else {
				if (state==in_word) finish_word();
				add_to_word();
				state = in_shift15;
			}
			continue;
		}
		if (state == in_shift15) {
			add_to_word();
			continue;
		}

		// FIRST test shift []
		if ( line[pt_line] == '[' ) {
			if (state==in_word) finish_word();
			add_to_word();
			state = in_shiftsq;
			continue;
		}
		if ( line[pt_line] == ']' ) {
			add_to_word();
			finish_word();
			state = in_seps;
			continue;
		}
		if (state == in_shiftsq) {
			add_to_word();
			continue;
		}

		if ( line[pt_line] == '-' ) {
			// tests for all special forms.
			int n = test_special( &line[pt_line], special_moins );
			if (n!=0) {
				int i;
				if (state==in_word) finish_word();
				for (i=0;i<n;i++) add_to_word();
				finish_word();
				state = in_seps;
				continue;
			}
			if ( compound_get_change_minus_to_plus() == 0 || in_shift15) { // '-' ignored
				add_to_word();
				state = in_word;
			} else if ( compound_get_keep_old_compounds() == 1 && compound_get_change_minus_to_plus() == 1 ) { // change '-' to '+' and keep '+'
				line[pt_line] = '+';
				add_to_word();
				state = in_word;
			} else { // ( keep_old_compounds == 0 && change_minus_to_plus == 1 ) // split words : '-' eq. to sepsskip.
				if (state==in_seps) {
					skip();
				} else {
					finish_word();
					skip();
					state = in_seps;
				}
			}
			continue;
		}


		if ( line[pt_line] == '+' ) {
			// tests for all special forms.
			int n = test_special( &line[pt_line], special_plus );
			if (n!=0) {
				int i;
				if (state==in_word) finish_word();
				for (i=0;i<n;i++) add_to_word();
				finish_word();
				state = in_seps;
				continue;
			}
			if ( compound_get_keep_old_compounds() == 1 ) { // '+' ignored
				add_to_word();
				state = in_word;
			} else { // ( keep_old_compounds == 0 ) // split words : '+' eq. to sepsskip.
				if (state==in_seps) {
					skip();
				} else {
					finish_word();
					skip();
					state = in_seps;
				}
			}
			continue;
		}

		// pause ==> (.) (..) (...)
		if ( line[pt_line] == '(' ) {
			// tests for all special forms.
			int n = test_special( &line[pt_line], special_par );
			if (n!=0) {
				int i;
				if (state==in_word) finish_word();
				for (i=0;i<n;i++) add_to_word();
				finish_word();
				state = in_seps;
				continue;
			}
			int n1, n2;
			n = sscanf( &line[pt_line], "(%d.%d)", &n1, &n2);
			if (n==2) {
				if (state==in_word) finish_word();
				while ( line[pt_line]!=')' && line[pt_line] )
					add_to_word();
				add_to_word();
				finish_word();
				state = in_seps;
				continue;
			}
			add_to_word();
			state = in_word;
			continue;
		}

		if ( strchrConst( sepsskip, line[pt_line] ) ) {
			if (state==in_seps) {
				skip();
			} else {
				finish_word();
				skip();
				state = in_seps;
			}
		} else if ( strchrConst( sepskeep, line[pt_line] ) ) {
			if (state==in_seps) {
				add_to_word();
				finish_word();
			} else {
				finish_word();
				add_to_word();
				finish_word();
				state = in_seps;
			}
		} else if ( strchrConst( sepsbegin, line[pt_line] ) ) {
			if (state==in_word) finish_word();
			add_to_word();
			state = in_word;
		} else if ( strchrConst( sepsend, line[pt_line] ) ) {
			add_to_word();
			finish_word();
			state = in_seps;
		} else {
			add_to_word();
			state = in_word;
		}
	}

	return nb_words;
}

void stripof(char* s, char* a)
{
	while ( *s ) {
		if ( !strchrConst( IgnoreOnFind, *s ) ) *a++ = *s;
		s++;
	}
	*a = '\0';
}

