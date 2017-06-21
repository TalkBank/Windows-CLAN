/**********************************************************************
	"Copyright 1990-2012 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************************
** morpho.hpp
** creates a mor line in a CHAT format file using the dictionary included in POST data
***********************************************************************/

/* catfix.cut : file describing direct categorisation of words ending in @something
*/
extern char** List_Catfix;
void load_Catfix(char* filename);
char* is_Catfix(char* w);
void print_morphology(int nm, char** ms, char* out, int withpho, int newwords, int linelimit);
char* split_and_pack(char* from, char* sep, char* to);
void print_phonology(int nm, char** ms, char* out, int style, int linelimit);
char* compute_morline(char* T, char* MLine, int language_s, int output_s, int linelimit);

const int StyleXMor = 1;
const int StyleMorPho = 2;
const int StyleXPho = 4;
const int StyleSPho = 8;
const int StyleMor = 16;
const int StyleNewWords = 32;
const int StyleNewWordsPho = 64;
const int StyleMorForm = 128;
const int StyleMorFormPho = 256;
const int InternalNone = 512;
const int InternalCompound = 1024;

#define UTTLINELEN 18000L
extern char morT[UTTLINELEN];
