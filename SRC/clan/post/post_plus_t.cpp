/**********************************************************************
	"Copyright 1990-2020 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- post.cpp	- v1.0.0 - november 1998

this file contains the main procedure of the POST analyze software.

Description of the Procedures and Function of the current file:

int main( int argc, char** argv );
	This function parses the DOS input line.
	It sorts out the option but do not perform any computation.
	and calls the analyze process using the function 'post_file'
	RETURNS: 0 if OK, -1 if there is an error in the parameters, -2 if there is an error in the analyze process.

	It stands as an example of the calling process.
	The main function post_file(), may be called from any program.

|=========================================================================================================================*/

#ifdef POSTCODE
	#define CHAT_MODE 0
#endif
#include "system.hpp"
#include "database.hpp"
#ifdef POSTCODE
	#define _main postprog_main
	#define call postprog_call
	#define getflag postprog_getflag
	#define init postprog_init
	#define usage postprog_usage
	#define IS_WIN_MODE FALSE
	#include "mul.h"

	extern char isRecursive;

	void usage() { }
	void init(char f) { }
	void getflag(char *f, char *f1, int *i) {
		f++;
		switch(*f++) {
			case 'r' :
				isRecursive = TRUE;
				break;
			case '1' :
				replaceFile = TRUE;
				break;
		}
	}
#else
	#include <io.h>
	#define EOS '\0'
#endif

static char outfilename[MaxFnSize];
static int style = 0;
static int style_unkwords = 0;
static int brill = 1;
static int linelimit = 70;
static int filter_out = KeepAll;
static int inmemory = 1;

extern char *posErrType;

void call(
#ifndef POSTCODE
			char *oldfname
#endif
				) {
#ifdef POSTCODE
	fclose(fpin);
	fpin = NULL;
#endif
	post_file( oldfname, style, style_unkwords, brill, outfilename, filter_out, linelimit ) ;
}

#ifdef CLANEXTENDED
void post_main( int argc, char** argv )
#else
#ifdef UNX
int
#else
void
#endif
main( int argc, char** argv)
#endif
{
	char gb_filter_name [256];
	strcpy( gb_filter_name, "" );
	// computation of parameters.
	char dbname[MaxFnSize];
	int i;

	strcpy( dbname, default_databasename );
	strcpy( outfilename, "" );

	if ( argc<2 ) {
		msg("POST-ANALYZE: version - %s - INSERM - PARIS - FRANCE\n",MSA_VERSION);
		msg("Usage: post [dF eNc f lN sN tF] filename(s)\n");
		msg("+dF : use POST database file F (if omitted, default is %s).\n", dbname);
		msg("+eNC: this option allows you to change the separator used.\n");
		msg("    It is a complement to the option +s2 and +s3 only.\n");
		msg("    +e1C- change separator used between different solutions to C (default '#').\n");
		msg("    +e2C- change separator used before info on parsing process to C (default '/').\n");
		msg("+f  : send output to file derived from input file name (default).\n");
		msg("+fF : send output to file F.\n");
		msg("-f  : send output to the screen.\n");
		msg("+lN : line limit.\n");
		msg("+sN : choose style N.\n");
		msg("    0- replace ambiguous %%mor lines with disambiguated ones (default).\n");
		msg("    1- keep ambiguous %%mor lines and add disambiguated %%pos lines.\n");
		msg("    2- output as in N=1, but with slashes marking undecidable cases.\n");
		msg("    3- keep ambiguous %%mor lines and add %%pos lines with debugging info.\n");
		msg("    4- outputs results in an internal format. It's useful for developer mostly.\n");
		msg("    5- inserts a %%nob line before the %%mor/%%pos line that presents the results of the analysis without using Brill rules.\n");
		msg("-b  : do not use Brill's rules to improve the results of the analysis (uses rules by default).\n");
		msg("+bs : use a slower but more thorough version of Brill's rules analysis.\n");
		msg("+t  : keep all syntactic categories(default).\n");
		msg("+tF : use the stem tags in file F along with the other syntactic categories.\n");
		msg("-t  : throw away all syntactic categories.\n");
		msg("-tF : throw away stem tags in file F along with the other syntactic categories.\n");
		msg("+lm : reduce memory use (but longer processing time).\n");
		msg("+unk: tries to process unknown words.\n");
#ifdef POSTCODE
		msg("+re : run program recursively on all sub-directories.\n");
		msg("+1  : replace original data file with new one.\n");
#endif
		return;
	}

	style = 0;
	brill = 1;
	linelimit = 70;
	filter_out = KeepAll;
	inmemory = 1;
	// skip through all parameters to process options.
	for (i=1;i<argc;i++ ) {
		if ( argv[i][0] == '+' || argv[i][0] == '-' ) {
			switch( argv[i][1] ) {
				case 's' :
					if ( argv[i][2] == '0' ) style = 0;
					else if  ( argv[i][2] == '1' ) style = 1;
					else if  ( argv[i][2] == '2' ) style = 2;
					else if  ( argv[i][2] == '3' ) style = 3;
					else if  ( argv[i][2] == '4' ) style = 4;
					else if  ( argv[i][2] == '5' ) style = 5;
					else {
						msg ( "bad parameter after +s: should be 0,1,2,3,4 or 5\n" );
					return;
					}
					break;
				case 'd' :
					if (argv[i][2] != EOS)
						strcpy( dbname, argv[i]+2 );
					else {
						msg ( "bad parameter after +d: should be a database file name\n" );
						return;
					}
					break;
				case 'e' :
					if ( argv[i][2] == '1' && argv[i][3] ) {
						Separator1 = argv[i][3];
					} else if ( argv[i][2] == '2' && argv[i][3] ) {
						Separator2 = argv[i][3];
					} else
						msg ( "bad use of parameter: %s\n", argv[i] );
					break;
				case 'l' :
					if (argv[i][2] == 'm') {
						inmemory = 0;
					} else {
						linelimit = atoi( argv[i]+2 );
						if (linelimit < 10 || linelimit > 1024) linelimit = 70;
					}
					break;
				case 'u' :
					if (argv[i][2] == 'n' && argv[i][3] == 'k') {
						style_unkwords = 1;
					}
					break;
				case 't' :
					if ( argv[i][2] == '\0' ) {
						if (argv[i][0] == '+') filter_out = KeepAll;
						else filter_out = ThrowAwayAll;
					} else {
						strcpy ( gb_filter_name, argv[i]+2 );
#ifdef __STDC__
						if ( _access(gb_filter_name, 04)) {
#else
						if ( access(gb_filter_name, 04)) {
#endif
#ifdef POSTCODE
							FILE *tfp;
							tfp=OpenMorLib(gb_filter_name, "r", FALSE, FALSE, NULL);
							if (tfp == NULL)
								tfp= OpenGenLib(gb_filter_name,"r",FALSE,FALSE,NULL);
							if (tfp == NULL) {
#endif
								msg ( "unable to open %s for reading (parameter %s)\n", argv[i]+2, argv[i] );
								return;
#ifdef POSTCODE
							} else {
								fclose(tfp);
							}
#endif
						}
						if ( argv[i][0] == '+' )
							filter_out = Keep;
						else
							filter_out = ThrowAway;
					}
					break;
				case 'f' :
					if (argv[i][0] == '-')
						strcpy(outfilename, "con" );
					else if (argv[i][2] != '\0') {
						strcpy(outfilename, argv[i]+2);
					}
					break;
				case 'b' :
					if (argv[i][0] == '-')
						brill = 0;
					else if (argv[i][2] == 's')
						brill = 2;
					break;
#ifdef POSTCODE // following is NEEDED FOR CLAN
				case 'r' :
					break;
				case '1' :
					break;
#endif // above is NEEDED FOR CLAN
				default:
					msg ( "unknown parameter %s\n", argv[i] );
					return;
					break;
			}
		}
	}

	// initialization once for all.
	if ( (i=post_init(dbname, inmemory)) < 1) {
		if (i == -1) {
			if (brill)
				msg( "warning: the are no Brill's rules in this database.\n");
			brill = 0;
		} else {
			msg ( "error: the file %s cannot be opened\n", dbname );
			return;
		}
	}

	gbin_filter_open();
	if (gb_filter_name[0]!='\0')
		gbout_filter_create(gb_filter_name);

	posErrType = "w";
#ifdef POSTCODE
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = POST;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
#else
	// skip through all parameters to analyze the required files
	for ( i=1;i<argc;i++ ) {
		if (argv[i][0] != '+' && argv[i][0] != '-') {
			call(argv[i]) ;
		}
	}
#endif
	gbin_filter_free();
	gbout_filter_free();
	dynfreeall();

	close_all();
}
