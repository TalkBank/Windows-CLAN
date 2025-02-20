/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************************
** compound.cpp
** normalizes and formats compound words in a CHAT format file
** Use: compound -f"fichier" files...
***********************************************************************/

#include "system.hpp"
#include "tags.hpp"
#include "database.hpp"
#include "compound.hpp"
#include "morpho.hpp"

#ifdef POSTCODE
	#define CHAT_MODE 0
	#define _main compound_main
	#define call compound_call
	#define getflag compound_getflag
	#define init compound_init
	#define usage compound_usage
	#define main_return return

	#define IS_WIN_MODE FALSE
	#include "mul.h"


	void usage() { }
	void init(char f) { }
	void getflag(char *f, char *f1, int *i) { }
#elif defined(UNX) || (!defined(POSTCODE) && !defined(MAC_CODE))
	#define CLAN_MAIN_RETURN int
	#define main_return return(0);
#else
	#include <io.h>

	#define main_return return
#endif /* !POSTCODE */

#define maxSizeLine 4096

static int replace_file = 0;	// old file is replaced by new file

static int compute_exps( int id, FILE* out)
{
	int n, y; char* T;
	long32 postLineno = 0L;
	rightSpTier = 1;
	char Compound_line[UTTLINELEN];
	while ( (n = get_tier(id, T, y, postLineno)) != 0 ) {
		if ( T[0] == '*') {
			filter_with_compounds( T, Compound_line );
			fprintf( out, "%s\n", Compound_line );
		} else {
			fprintf( out, "%s", T );
		}
	}

	return 1;
}

#ifdef CLANEXTENDED
#define call compound_call
#endif

void call(
#ifndef POSTCODE
			char *oldfname
#endif
		  ) {
#ifdef POSTCODE
	fclose(fpin);
	fpin = NULL;
#endif

	FNType fno[FNSize];
	strcpy( fno, oldfname );
	FNType* p = strrchr( fno, '.' );
	if (!p) {
		strcat(fno, ".cpn" );
		p = strrchr( fno, '.' );
	} else
		strcpy(p, ".cpn" );
	strcat(p, ".cex" );
	int n = 0;
#if defined(POSTCODE) || defined(UNX)
	while ( access( fno, 00 ) == 0 )
#else
	while ( _access( fno, 00 ) != -1 )
#endif
										{
		sprintf( p, ".cp%d.cex", n );
		n++;
		if ( n==1000 ) {
			msg ( "cannot generate a file using the inputfile %s\n", oldfname );
			return;
		}
	}

	int id = init_input( oldfname, "mor" );
	if (id<0) {
		msg ( "the file \"%s\" does not exist or is not available\n", oldfname );
		return;
	}

/*	FILE* in = fopen( oldfname, "r" );
	if (!in) {
		msg("Unable to open file %s\n", oldfname );
		return;
	}
*/
	FILE* out = fopen( fno, "w" );
	if (!out) {
		msg("Unable to create file %s\n", fno );
		close_input(id);
		//	fclose(in);
		return;
	}

	compute_exps( id, out );
	msg( "From file <%s>\n", oldfname );

	close_input(id);
//	fclose(in);

	fclose(out);
	if ( replace_file ) {
		unlink( oldfname );
#ifdef POSTCODE
		rename_each_file(fno, oldfname, FALSE);
#else
		rename( fno, oldfname );
#endif
	}
}

#ifdef CLANEXTENDED
void compound_main( int argc, char** argv )
#else
CLAN_MAIN_RETURN main( int argc, char** argv )
#endif
{
	FNType expsfilename[FNSize];
	
	expsfilename[0] = 0;
	if (argc<3) {
		msg("COMPOUND: version - %s - INSERM - PARIS - FRANCE - %s.\n",MSA_VERSION,MSA_DATE);
		msg("Usage: compound [+d +fF] filename(s)\n");
		msg("+fF: F is a file that contains all the compound words (one per line).\n");
		msg("+k:  Compounds that are not present in parameter +f file are not modified (default).\n" );
		msg("-k:  Only compound words which are present in parameter +f file are valid.\n" );
		msg("+m:  Change '-' to '+' (default for French)");
		msg("-m:  Do not change '-' to '+' (default for English)");
		msg("+lUS:  Normalizes for English (default).\n" );
		msg("+lFR:  Normalizes for French.\n" );
		msg("+d:  Full dots at end of line are normalized.\n" );
		msg("+1:  Replace original data file with new one.\n");
		main_return;
	}


	compound_set_language(0); // by default do nothing to apostrophes, if == 1 do French style processing
	compound_set_full_dot_at_end(0); // by default do nothing. if == 1 add a full dot at the end of utterances if no utterance ending present , if == -1 suppress full dot.
	compound_set_keep_old_compounds(1); // means that compounds written in the text are kept as such ; if == 0 then only compounds from external file are valid (compounds written in the texte are ignored.
	compound_set_change_minus_to_plus(0); // to be used only if there is no mMOR marking in *CHI lines (for coumpounds, a-b notation equals a+b notation.) default for French.

	int i, m_done=0, list_of_compounds=0;
	// skip through all parameters to process options.
	for (i=1;i<argc;i++ ) {
		if ( argv[i][0] == '+' || argv[i][0] == '-' ) {
			switch( argv[i][1] ) {
				case 'f' :
					strcpy(expsfilename, argv[i]+2);
					break;
				case 'd' :
					compound_set_full_dot_at_end(1);
					break;
				case 'x' :
					list_of_compounds = 1;
					break;
				case 'k' :
					if ( argv[i][0] == '+' ) compound_set_keep_old_compounds(1); else compound_set_keep_old_compounds(0);
					break;
				case 'm' :
					if ( argv[i][0] == '+' ) compound_set_change_minus_to_plus(1); else compound_set_change_minus_to_plus(0);
					m_done = 1;
					break;
				case 'l' :
					if ( (argv[i][2] == 's' && argv[i][3] == '0') ||
					     ((argv[i][2] == 'U'||argv[i][2] == 'u') && (argv[i][3] == 'S'||argv[i][3] == 's')) ) compound_set_language(CompoundUS);
					if ( (argv[i][2] == 's' && argv[i][3] == '1') ||
					     ((argv[i][2] == 'F'||argv[i][2] == 'f') && (argv[i][3] == 'R'||argv[i][3] == 'r')) ) {
						compound_set_language(CompoundFR);
						if (m_done==0) compound_set_change_minus_to_plus(1); // if not set by option before.
					}
					break;
				case '1' :
					replace_file = 1;
					break;
				default:
					msg ( "unknown parameter %s\n", argv[i] );
					main_return;
					break;
			}
		}
	}

	if ( expsfilename[0] == '\0' ) {
		msg( "You MUST provide the name of the file with all the compound words (use of option +f is mandatory).\n" );
		main_return;
	}

	if ( !init_exps( expsfilename, list_of_compounds ) ) { 
		main_return;
	}

#ifdef POSTCODE
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = COMPOUND;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
//	free(s_exps) ;
#else
	// skip through all parameters to analyze the required files
	for ( i=1;i<argc;i++ ) {
		if (argv[i][0] != '+' && argv[i][0] != '-') {
			call(argv[i]) ;
		}
	}
#endif
	clear_exps();
	main_return;
}
