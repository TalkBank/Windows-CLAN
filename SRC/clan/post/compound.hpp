/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************************
** compound.h
** normalizes and formats compound words in a CHAT format file
***********************************************************************/

int init_exps( FNType* fname, int list = 0);
void clear_exps();
char* filter_with_compounds(char* T, char* C);

#define change_doublequote_to_antislash 1
#define change_pointmiddle_to_antislash 1

#define CompoundPunctuations ".;!?,"
#define CompoundWhiteSpaces " \t\r\n"
#define CompoundPlusMoins "+-"
#define CompoundPlusMoinsWhiteSpaces "+- \t\r\n"
#define CompoundSepsEndApostrophe "'"
#define CompoundSepsEnd ""
#define CompoundSepsBegin ""
#define IgnoreOnFind "()<>#:/^"

/* parameters of compounds command */
void compound_set_language(int lg);	// by default do nothing to apostrophes, if == 1 do French style processing
void compound_set_full_dot_at_end(int p);	// by default do nothing. if == 1 add a full dot at the end of utterances if no utterance ending present , if == -1 suppress full dot.
void compound_set_keep_old_compounds(int p);	// means that compounds written in the text are kept as such ; if == 0 then only compounds from external file are valid (compounds written in the texte are ignored.
void compound_set_change_minus_to_plus(int p);	// to be used only if there is no mMOR marking in *CHI lines (for coumpounds, a-b notation equals a+b notation.) default for French.

int compound_get_language();	// by default do nothing to apostrophes, if == 1 do French style processing
int compound_get_full_dot_at_end();	// by default do nothing. if == 1 add a full dot at the end of utterances if no utterance ending present , if == -1 suppress full dot.
int compound_get_keep_old_compounds();	// means that compounds written in the text are kept as such ; if == 0 then only compounds from external file are valid (compounds written in the texte are ignored.
int compound_get_change_minus_to_plus();	// to be used only if there is no mMOR marking in *CHI lines (for coumpounds, a-b notation equals a+b notation.) default for French.

#define CompoundUS '0'
	// US ==> use CompoundSepsEnd
#define CompoundFR '1'
    // FR ==> use CompoundSepsEndApostrophe
