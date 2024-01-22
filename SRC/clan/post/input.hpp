/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/* input.hpp
	- input from different types of data:
	- vertical texts.
	- raw texts.
	- CLAN & MOR files.
*/

#ifndef __INPUT_HPP__
#define __INPUT_HPP__

//   Detailed format (-x- means it is already implemented)
//	- training input:

#define VTT "vtt" // - - vertical texts: one word + one tag per line.
#define MFT "mft" // - - MOR files (%mor: tagged text -one tag each-)

//	- analyze input:
#define VTA "vta" // -x- vertical texts
#define RTA "rta" // - - raw texts (any format, text only).
#define CFA "cfa" // - - CLAN format - create %mor: with one tag (without using CLAN).
#define MOR "mor" // - - MOR files (re-analyze %mor: fields).

#define fVTT 1
#define fMFT 2
#define fVTA 3
#define fRTA 4
#define fCFA 5
#define fMOR 6

#if defined(POSTCODE) || defined(UNX)
struct SimpleTier {
	char *org;
	char *ow;
	char *sw;
	char isCliticChr;
	struct SimpleTier *nextChoice;
	struct SimpleTier *nextW;
} ;

struct ConllTable {
	char *from;
	char *to;
	struct ConllTable *next;
} ;

extern int  getNextMorItem(char *line, char *oItem, int *morI, int i);
extern char isConllSpecified(void);
extern void printclean(FILE *out, const char *sp, char *line);
extern void init_conll(void);
extern void freeConll(void);
extern void readConllFile(FNType *dbname);
extern void freeConvertTier(struct SimpleTier *p);
extern SimpleTier *convertTier(char *T, FNType* fn, char *isConllError);

#endif

#if defined(UNX)

#ifndef NEW
	#define NEW(type) ((type *)malloc((size_t) sizeof(type)))
#endif /* NEW */

#ifndef TRUE
	#define TRUE  1
#endif
#ifndef FALSE
	#define FALSE 0
#endif
#define isSpace(c)	 ((c) == (char)' ' || (c) == (char)'\t')

extern char templineC[];
extern char templineC1[];
extern char templineC2[];

extern void remFrontAndBackBlanks(char *st);

#endif

/*
	data is allocated inside 'get_input()' and allocated at the end of the program.
	mo memory desallocation scheme is necessary (it is done by 'close_input()').

	ID stands for Input Descriptor
*/
int	init_input( FNType* filename, const char* type );	// return 1 if OK, 0 if not
int	get_input( int ID, char** &data );		// return number of words (or lines).
int	get_tier(int ID, char* &tier, int &typeoftier, long32 &ln); // return nb. of chars.
void	close_input( int ID );

/* get_input returns a full sentence or utterance, with a max size of 500.
   END of SENTENCE is '.', '?', '!'.
   for CLAN format (and MOR) end of tier is considered as end of sentence.

   If the sentence is too long32, ';', ':', and ',' are also considered as end of sentence.
*/

/*
	For vertical texts, return from input is full lines (whatever is of the lines).
	For raw texts, see language parameters below.
*/

int get_type_of_tier( char* tier, char* name=0 ) ;
void strip_of_brackets( char* line );
char* strip_of_one_bracket( char* line, char* tag, int mltag );

#define TTgeneral 1
#define TTmor 2
#define TTdependant 3
#define TTthisname 4
#define TTname 5
#define TTunknown 6
#define TTtrn 7
#define TTwrd 8
#define TTpos 9
#define TTcnl 10

#endif

