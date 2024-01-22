/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// postModRules.cpp
// november 7, 2000
// modify Brill's manually according to data given in a file.

#ifdef POSTCODE
	#define CHAT_MODE 0
#endif
#include "system.hpp"
#include "database.hpp"

#ifdef POSTCODE
  #if !defined(UNX)
	#define _main postmodrules_main
	#define call postmodrules_call
	#define getflag postmodrules_getflag
	#define init postmodrules_init
	#define usage postmodrules_usage
  #endif
	#define main_return return

	#define IS_WIN_MODE FALSE
	#include "mul.h"

	#define stricmp uS.mStricmp

	void usage() { }
	void init(char f) { }
	void getflag(char *f, char *f1, int *i) { }
	void call( ) { }

#elif defined(UNX) || (!defined(POSTCODE) && !defined(MAC_CODE))
	#define CLAN_MAIN_RETURN int
	#define main_return return(0);
#else
	#include <io.h>

	#define main_return return
#endif

int split_in_two(char* line, char* tag, char* elt);
// returns -1 if msg( "first keyword too long\n" ); - Max length 24.
// returns -2 if msg( "only one keyword on the line\n" );
// returns 0 if empty line
// returns 1 if OK
int split_in_two(char* line, char* tag, char* elt)
{
	int d;
	int k;
	for (d=0; line[d] && strchrConst(" \t\r\b\n",line[d]); d++) ;
	if ( !line[d] ) return 0; // ligne blanche.
	char* p = strchr( line, '#' );
	int e = (p) ? (p-line)-1 : strlen(line)-1 ;
	for ( ; e>=0 && strchrConst(" \t\r\b\n",line[e]); e--) ;
	e++;
	if ( e<=0 ) return 0; // ligne vide.
	// trouver le premier mot.
	for (k=d; line[k] && !strchrConst(" \t\r\b\n",line[k]); k++) ;
	if ( !line[k] ) // un seul mot sur la ligne.
		return -2;
	// arrÍt sur le premier blanc
	if ( (k-d)>=24 )
		return -1;
	strncpy( tag, &line[d], (k-d) );
	tag[k-d]='\0';
	for ( ; line[k] && strchrConst(" \t\r\b\n",line[k]); k++) ;
	if ( !line[k] ) // un seul mot sur la ligne.
		return -2;
	strncpy( elt, &line[k], e-k );
	elt[e-k]='\0';
	
// msg( "[%s] [%s]", tag, elt );

	return 1;
}

int is_a_keyword( char* key );
// possible commands:
// suppress from word tag	- return 1
// suppress from number	- return 2
// add from word tag	- return 3
// add at number	- return 4
// unknow keyword	- return 0
int is_a_keyword( char* key )
{
	if ( !stricmp( key, "add" ) )	return 3;
	if ( !stricmp( key, "addn" ) )	return 4;
	if ( !stricmp( key, "del" ) )	return 1;
	if ( !stricmp( key, "deln" ) )	return 2;
	return 0;
}

int is_a_rulename( char* rulename );
	// return 0 if nt a rulename
	// else return number of arguments
int is_a_rulename( char* rulename )
{
	if ( !stricmp( rulename, "NEXTTAG" ) ) return 5;	// tag
	if ( !stricmp( rulename, "NEXT2TAG" ) ) return 5;
	if ( !stricmp( rulename, "NEXT1OR2TAG" ) ) return 5;
	if ( !stricmp( rulename, "NEXT1OR2OR3TAG" ) ) return 5;
	if ( !stricmp( rulename, "PREVTAG" ) ) return 5;
	if ( !stricmp( rulename, "PREV2TAG" ) ) return 5;
	if ( !stricmp( rulename, "PREV1OR2TAG" ) ) return 5;
	if ( !stricmp( rulename, "PREV1OR2OR3TAG" ) ) return 5;
	if ( !stricmp( rulename, "NEXTWD" ) ) return 5;	// word
	if ( !stricmp( rulename, "CURWD" ) ) return 5;
	if ( !stricmp( rulename, "NEXT2WD" ) ) return 5;
	if ( !stricmp( rulename, "NEXT1OR2WD" ) ) return 5;
	if ( !stricmp( rulename, "NEXT1OR2OR3WD" ) ) return 5;
	if ( !stricmp( rulename, "PREVWD" ) ) return 5;
	if ( !stricmp( rulename, "PREV2WD" ) ) return 5;
	if ( !stricmp( rulename, "PREV1OR2WD" ) ) return 5;
	if ( !stricmp( rulename, "PREV1OR2OR3WD" ) ) return 5;

	if ( !stricmp( rulename, "WDANDTAG" ) ) return 6;	// word tag
	if ( !stricmp( rulename, "SURROUNDTAG" ) ) return 6;	// left right
	if ( !stricmp( rulename, "PREVBIGRAM" ) ) return 6;	// prev1 prev2
	if ( !stricmp( rulename, "NEXTBIGRAM" ) ) return 6;	// next1 next2
	if ( !stricmp( rulename, "WDPREVTAG" ) ) return 6;	// prev1 word
	if ( !stricmp( rulename, "WDNEXTTAG" ) ) return 6;	// word next1
	if ( !stricmp( rulename, "WDAND2TAGBFR" ) ) return 6;	// prev2 word
	if ( !stricmp( rulename, "WDAND2TAGAFT" ) ) return 6;	// word next2
	if ( !stricmp( rulename, "LBIGRAM" ) ) return 6;	// prevw1 word
	if ( !stricmp( rulename, "RBIGRAM" ) ) return 6;	// word nextw1
	if ( !stricmp( rulename, "WDAND2BFR" ) ) return 6;	// prevw2 word
	if ( !stricmp( rulename, "WDAND2AFT" ) ) return 6;	// word nextw2

	return 0;
}

static int inmemory = 1;

#ifdef CLANEXTENDED
void postmodrules_main( int argc, char** argv )
#else
CLAN_MAIN_RETURN main( int argc, char** argv)
#endif
{
	FILE* actions = stdout;
	int i;
	int forcecreate = 0;
	FNType database_name[FNSize];

	init_tags();
	inmemory = 1;
//2009-07-22	define_databasename(database_name);
	strcpy(database_name, "post.db");
	if ( argc<2 ) {
		msg("POSTMODRULES: version - %s - INSERM - %s\n",MSA_VERSION,MSA_DATE);
		msg("Usage: postmodrules [dF rF]\n");
		msg("+dF: use POST database file F (default is %s).\n", database_name);
		msg("+rF: specify name of file (F) containing actions that modify rules.\n");
		msg("+c : force creation of Brill's rules.\n");
		msg("+lm: reduce memory use (but longer processing time).\n");
		main_return;
	}

	for (i=1;i<argc;i++ ) {
		if ( argv[i][0] == '+' || argv[i][0] == '-' ) {
			switch( argv[i][1] ) {
				case 'l' :
					if (argv[i][2] == 'm') {
						inmemory = 0;
					} else
						msg ( "bad use of parameter: %s\n", argv[i] );
					break;
				case 'c' :
					forcecreate = 1;
					break;
				case 'r' :
					actions = fopen(argv[i]+2, "r" );
					if (!actions) {
						msg ("Warning: %s cannot be opened\n", argv[i]+2);
						main_return;
					}
#ifdef POSTCODE
#ifdef _MAC_CODE
					settyp(argv[i]+2, 'TEXT', the_file_creator.out, FALSE);
#endif
#endif
					break;
				case 'd' :
					if (argv[i][2] != EOS) {
#if defined(UNX) || (!defined(POSTCODE) && !defined(MAC_CODE))
						strcpy(database_name, argv[i]+2);
#else
						uS.str2FNType(database_name, 0L, argv[i]+2);
#endif
					} else {
						msg ( "Bad parameter after +d: should be a database file name\n" );
						main_return;
					}
					break;
			}
			argv[i][0] = '\0';
		}
	}

	if ( !open_all( database_name, 0, inmemory ) ) {
		msg ("error: %s cannot be opened\n", database_name );
		main_return;
	}

	if ( forcecreate ) {
		if ( !open_brillrule_dic( "brillrules", 0 ) ) {
			msg( "Cannot create Brill's rules in this database" );
			main_return;
		}
		if ( !open_brillword_dic( "brillwords", 0 ) ) {
			msg( "Cannot create Brill's words in this database" );
			main_return;
		}
	} else if ( !open_brillrule_dic( "brillrules", 1 ) ) {
		msg( "No Brill's rules in this database" );
		msg( "You must use flag +c to allow creation of Brill's rules.\n");
		main_return;
	}

	char line[MAXSIZEBRILLRULE];
	int n=0;
	while ( fgets( line, MAXSIZEBRILLRULE, actions ) ) {
		n++;
		char tag[25];
		char rule[MAXSIZEBRILLRULE];

		if ( line[0] == '%' ) continue; // comment line!

		int r=split_in_two(line, tag, rule);
		if (r==-1) {
			msg( "line %d: bad format - the first keyword is not a correct line-command.\n", n );
		} else if (r==-2) {
			msg( "line %d: bad format - should have at least 5 elements.\n", n );
		} else if (r==1) {
			char* wrds[12];
			int nw = split_with( rule, wrds, " \t\r\n", 12 );
			int rn, kn;
			if (nw < 5)
				msg( "line %d: bad format - should have at least 5 elements.\n", n );
			else if ( !(kn=is_a_keyword( tag )) )
				msg( "line %d: bad format - %s is not a correct line-command.\n", n, tag );
			else if ( !(rn=is_a_rulename( wrds[3] )) )
				msg( "line %d: bad format - %s is not a correct rule name.\n", n, wrds[3] );
			else {
				// possible commands:
				// suppress from word tag	- return 1
				// suppress from number	- return 2
				// add from word tag	- return 3
				// add at number	- return 4
				switch( kn ) {
					case 1:
						if ( rn != nw ) {
							msg( "line %d: bad format - should have exactly %d elements.\n", n, rn );
						} else {
							char ruletext[MAXSIZEBRILLRULE];
							if ( rn == 5 )
								sprintf( ruletext, "%s %s %s %s %s", wrds[0], wrds[1], wrds[2], wrds[3], wrds[4] );
							else
								sprintf( ruletext, "%s %s %s %s %s %s", wrds[0], wrds[1], wrds[2], wrds[3], wrds[4], wrds[5] );
							suppress_brillrule( wrds[0], wrds[1], ruletext );
						}
					break;
					case 2:
						if ( rn+1 != nw ) {
							msg( "line %d: bad format - should have exactly %d elements.\n", n, rn+1 );
						} else {
							suppress_brillrule_by_number( atoi(wrds[rn]) );
						}
					break;
					case 3:
						if ( rn != nw ) {
							msg( "line %d: bad format - should have exactly %d elements.\n", n, rn );
						} else {
							char ruletext[MAXSIZEBRILLRULE];
							if ( rn == 5 )
								sprintf( ruletext, "%s %s %s %s %s", wrds[0], wrds[1], wrds[2], wrds[3], wrds[4] );
							else
								sprintf( ruletext, "%s %s %s %s %s %s", wrds[0], wrds[1], wrds[2], wrds[3], wrds[4], wrds[5] );
							add_brillrule( wrds[0], wrds[1], ruletext );
						}
					break;
					case 4:
						if ( rn+1 != nw ) {
							msg( "line %d: bad format - should have exactly %d elements.\n", n, rn );
						} else {
							char ruletext[MAXSIZEBRILLRULE];
							if ( rn == 5 )
								sprintf( ruletext, "%s %s %s %s %s", wrds[0], wrds[1], wrds[2], wrds[3], wrds[4] );
							else
								sprintf( ruletext, "%s %s %s %s %s %s", wrds[0], wrds[1], wrds[2], wrds[3], wrds[4], wrds[5] );
							add_brillrule_at_number( atoi(wrds[rn]), wrds[0], wrds[1], ruletext );
						}
					break;
				}
			}
		}
	}

	save_all();
	close_all();
	fclose(actions);
}
