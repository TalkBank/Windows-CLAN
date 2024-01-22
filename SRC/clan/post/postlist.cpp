/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// listrule.cpp
// 28 may 1998

#ifdef POSTCODE
	#define CHAT_MODE 0
#endif
#include "system.hpp"
#include "database.hpp"

#ifdef POSTCODE
  #if !defined(UNX)
	#define _main postlist_main
	#define call postlist_call
	#define getflag postlist_getflag
	#define init postlist_init
	#define usage postlist_usage
  #endif
	#define main_return return

	#define IS_WIN_MODE FALSE
	#include "mul.h"
	
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

#define Do_Rules 1
#define Do_Matrix 2
#define Do_LocalWords 64
#define Do_Words 4
#define Do_Tags 8
#define Do_Brill 16
#define Do_BrillNumber 32

static int inmemory = 1;

#ifdef CLANEXTENDED
void postlist_main( int argc, char** argv )
#else
CLAN_MAIN_RETURN main( int argc, char** argv)
#endif
{
	FILE* out = stdout;
	int i;
	int action = 0;
	FNType database_name[FNSize];

	init_mortags();
	init_tags();
	inmemory = 1;
//2009-07-22	define_databasename(database_name);
	strcpy(database_name, "post.db");
	if ( argc<2 ) {
		printf("POSTLIST: version - %s - INSERM - %s\n",MSA_VERSION,MSA_DATE);
		printf("Usage: postlist [dF e fF r m tF w]\n");
		printf("+dF: use POST database file F (default is %s).\n", database_name);
		printf("+e : outputs the list of all tags present in the database.\n");
		printf("+fF: specify name of result file to be F.\n");
		printf("+m : outputs all the matrix entries present in the database.\n");
		printf("+r : outputs all the rules present in the database.\n");
		printf("+w : outputs all the word frequencies gathered in the database.\n");
		printf("+x : outputs the category of all the words gathered in the database.\n");
		printf("+rb: outputs rule dictionary for Brill's tagger.\n");
		printf("+rn: outputs rule dictionary for Brill's tagger in numerical order.\n");
		printf("+wb: outputs word dictionary for Brill's tagger.\n");
		printf("+lm: reduce memory use (but longer processing time).\n");
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
				case 'f' :
					out = fopen(argv[i]+2, "w" );
					if (!out) {
						msg ("Warning: %s cannot be created\n", argv[i]+2);
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
						msg ( "bad parameter after +d: should be a database file name\n" );
						main_return;
					}
					break;
				case 'r' :
					action |= Do_Rules;
					if ( argv[i][2] == 'b' ) action |= Do_Brill;
					else if ( argv[i][2] == 'n' ) {
						action |= Do_Brill;
						action |= Do_BrillNumber;
					}
					break;
				case 'm' :
					action |= Do_Matrix;
					break;
				case 'w' :
					action |= Do_LocalWords;
					if ( argv[i][2] == 'b' ) action |= Do_Brill;
					break;
				case 'x' :
					action |= Do_Words;
					if ( argv[i][2] == 'b' ) action |= Do_Brill;
					break;
				case 'e' :
					action |= Do_Tags;
					break;
				default:
					msg("unknown parameter %s\n", argv[i] );
					main_return;
			}
			argv[i][0] = '\0';
		} else {
			msg("unknown parameter %s\n", argv[i] );
			main_return;
		}
	}

	if ( !open_all( database_name, 0, inmemory ) ) {
		msg ("error: %s cannot be opened\n", database_name );
		main_return;
	}

//	msgfile(out, "@UTF8\n" );

	if ( action == 0 ) {
		msgfile(out, "this database contains:\n" );
		msgfile(out, "%ld tags\n", tags_number() );

		if ( ! open_rules_dic( "rules", 1 ) )
			msgfile(out, "no rules\n" );
		else
			msgfile(out, "%ld rules\n", hashsize( _rules_hashdic_ ) );

		if ( ! open_matrix_dic( "matrix", 1 ) )
			msgfile(out, "no matrix\n" );
		else {
			msgfile(out, "a matrix of depth %d with ", width_of_matrix() );
			msgfile(out, "%ld entries\n", hashsize( _matrix_hashdic_ ) );
		}

		if ( ! open_localword_dic( "localwords", 1 ) )
			msgfile(out, "no local word frequency information\n" );
		else
			msgfile(out, "%ld information about word frequency\n", hashsize( _localword_hashdic_ ) );

		if ( ! open_brillword_dic( "brillwords", 1 ) ||
		     ! open_brillrule_dic( "brillrules", 1 ) )
			msg( "no Brill's rules in this database\n" );
		else {
			msgfile(out, "%d Brill's rules\n", nb_brillrules() );
//			msgfile(out, "%ld Brill's words\n", hashsize( _brillword_hashdic_ ) );
		}

		if ( ! open_word_dic( "words", 1 ) )
			msgfile(out, "no words information\n" );
		else
			msgfile(out, "%ld word entries\n", hashsize( _word_hashdic_ ) );

	}

	if ( action & Do_Tags ) {
		msgfile(out, "----- list of tags\n\n" );
		list_classnames( out );
		msgfile(out, "\n----- list of suffixes\n\n" );
		gbin_filter_open();
		list_filter( out );
	}

	if ( (action & Do_Rules) && !(action & Do_Brill) ) {
		if ( ! open_rules_dic( "rules", 1 ) ) {
			msg( "file %s cannot be opened...\n", "rules" );
			main_return;
		}
		msgfile(out, "----- list of rules\n\n" );
		list_rules( out );
	}

	if ( (action & Do_Rules) && (action & Do_Brill) && !(action & Do_BrillNumber) ) {
		if ( ! open_brillrule_dic( "brillrules", 1 ) ) {
			msg( "file %s cannot be opened...\n", "brillrules" );
			main_return;
		}
		msgfile(out, "----- list of Brill's rules\n\n" );
		list_brillrule( out, 1 );
	}

	if ( (action & Do_Rules) && (action & Do_Brill) && (action & Do_BrillNumber) ) {
		if ( ! open_brillrule_dic( "brillrules", 1 ) ) {
			msg( "file %s cannot be opened...\n", "brillrules" );
			main_return;
		}
		msgfile(out, "----- list of Brill's rules\n\n" );
		list_brillrule( out, 2 );
	}

	if ( action & Do_Matrix ) {
		if ( ! open_matrix_dic( "matrix", 1 ) ) {
			msg( "file %s cannot be opened...\n", "matrix" );
			main_return;
		}
		msgfile(out, "\n----- list of entries in the matrix\n\n" );
		list_matrix( out );
	}

	if ( action & Do_Words ) {
		if ( ! open_word_dic( "words", 1 ) ) {
			msg( "file %s cannot be opened...\n", "words" );
			main_return;
		}
		msgfile(out, "\n----- list of words\n\n" );
		list_word( out );
	}


	if ( (action & Do_LocalWords) && !(action & Do_Brill) ) {
		if ( ! open_localword_dic( "localwords", 1 ) ) {
			msg( "file %s cannot be opened...\n", "localwords" );
			main_return;
		}
		msgfile(out, "\n----- local list of words\n\n" );
		list_localword( out );
	}

	if ( (action & Do_LocalWords) && (action & Do_Brill) ) {
		if ( ! open_brillword_dic( "brillwords", 1 ) ) {
			msg( "file %s cannot be opened...\n", "brillwords" );
			main_return;
		}
		msgfile(out, "%ld Brill's words\n", hashsize( _brillword_hashdic_ ) );
		msgfile(out, "\n----- local list of words\n\n" );
		list_brillword( out );
	}

	if (out != stdout)
		fclose(out);
	free_trainproc();
	gbin_filter_free();
	gbout_filter_free();
	dynfreeall();
	close_all();
}
