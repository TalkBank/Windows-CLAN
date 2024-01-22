/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
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

#include "system.hpp"
#include "database.hpp"
#include "morpho.hpp"
#include "compound.hpp"

#ifdef POSTCODE
	#define CHAT_MODE 1
	#define _main postprog_main
	#define call postprog_call
	#define getflag postprog_getflag
	#define init postprog_init
	#define usage postprog_usage
	#define main_return return

	#define IS_WIN_MODE FALSE
	#include "mul.h"

	extern char isRecursive;
	static int g_argc;
	static char** g_argv;

	void usage() { }
	void init(char f) { }
	void getflag(char *f, char *f1, int *i) {
		f++;
		switch(*f++) {
			case 't':
				maingetflag(f-2,f1,i);
				break;
			case 's':
				maingetflag(f-2,f1,i);
				break;
			case 'r' :
				isRecursive = TRUE;
				break;
			case '1' :
				if (replaceFile) {
					if (*(f-2) == '-')
						replaceFile = FALSE;
				} else {
					if (*(f-2) == '+')
						replaceFile = TRUE;
				}
				break;
		}
	}
#elif defined(UNX) || (!defined(POSTCODE) && !defined(MAC_CODE))
	#define CLAN_MAIN_RETURN int
	#define main_return return(0);
#else
	#include <io.h>

	#define main_return return
#endif

static FNType outfilename[FNSize];
static int style = 0;
static int style_unkwords = 0;
static int brill = 1;
static int linelimit = 70;
static int filter_out = KeepAll;
static int inmemory = 1;
static int internal_mor = 0;
static int priority_freq_local = 1;

extern const char *posErrType;
extern char isReportConllError;

void call(
#ifndef POSTCODE
			char *oldfname,
			char* g_argv[],
			int g_argc
#endif
				) {
#ifdef POSTCODE
	fclose(fpin);
	fpin = NULL;
#endif
	post_file( oldfname, style, style_unkwords, brill, outfilename, filter_out, linelimit, g_argv, g_argc, internal_mor, priority_freq_local ) ;
}

#ifdef CLANEXTENDED
void post_main( int argc, char** argv )
#else
CLAN_MAIN_RETURN main( int argc, char** argv)
#endif
{
	FNType gb_filter_name [FNSize];
	gb_filter_name[0] = 0;
	// computation of parameters.
	FNType dbname[FNSize], mFileName[FNSize];
	int i;

//2009-07-22	define_databasename(dbname);
	strcpy(dbname, "post.db");
	outfilename[0] = 0;

#ifdef POSTCODE
	replaceFile = TRUE;
#endif
	style = 0;
	style_unkwords = 0;
	brill = 1;
	linelimit = 70;

	strcpy(gb_filter_name, "posttags.cut" );
#ifdef UNX
	strcpy(mFileName, gb_filter_name);
	filter_out = KeepAll;
	gb_filter_name[0] = 0;
#elif defined(POSTCODE)
	FILE *tfp;

	tfp=OpenMorLib(gb_filter_name, "r", FALSE, FALSE, mFileName);
	if (tfp == NULL) {
		gb_filter_name[0] = 0;
		filter_out = KeepAll;
	} else {
		fclose(tfp);
		filter_out = Keep;
	}
	g_argc = argc;
	g_argv = argv;
#else
	filter_out = ThrowAwayAll;
#endif
	init_conll();
	if ( argc<2 ) {
		msg("POST-ANALYZE: version - %s - INSERM - PARIS - FRANCE - %s\n",MSA_VERSION,MSA_DATE);
		msg("Usage: post [dF eNc f lN gN cF sS tF] filename(s)\n");
		msg("+dF : use POST database file F (if omitted, default is %s).\n", dbname);
		if (isReportConllError)
			msg("+dv : do not report missing elements from \"conll.cut\" file\n");
		else
			msg("+dv : report missing elements from \"conll.cut\" file\n");
		msg("+eNC: this option allows you to change the separator used.\n");
		msg("    It is a complement to the option +s2 and +s3 only.\n");
		msg("    +e1C- change separator used between different solutions to C (default '#').\n");
		msg("    +e2C- change separator used before info on parsing process to C (default '/').\n");
		msg("+f  : send output to file derived from input file name (default).\n");
		msg("+fF : send output to file F.\n");
		msg("-f  : send output to the screen.\n");
		msg("+lN : line limit.\n");
		msg("+gN : choose style N.\n");
		msg("    0- replace ambiguous %%mor lines with disambiguated ones (default).\n");
		msg("    1- keep ambiguous %%mor lines and add disambiguated %%pos lines.\n");
		msg("    2- output as in N=1, but with slashes marking undecidable cases.\n");
		msg("    3- keep ambiguous %%mor lines and add %%pos lines with debugging info.\n");
		msg("    4- inserts a %%nob line before the %%mor/%%pos line that presents the results of the analysis without using Brill rules.\n");
		msg("    5- outputs results for debuging POST grammars.\n");
		msg("    6- complete outputs results for debuging POST grammars.\n");
		msg("-b  : do not use Brill's rules to improve the results of the analysis (uses rules by default).\n");
		msg("+bs : use a slower but more thorough version of Brill's rules analysis.\n");
		msg("+c  : output all affixes. (if no \"posttags.cut\" found, then this is default option)\n");
		msg("+cF : output the affixes listed in file F and \"post.db\" (if \"posttags.cut\" found, then this is default option).\n");
		msg("-c  : output only the affixes defined during training with POSTTRAIN.\n");
		msg("-cF : omit the affixes in file F, but not the affixes defined during training with POSTTRAIN.\n");
		msg("+aN : applies local frequencies to <=Nth long utterances\n");
		msg("+lm : reduce memory use (but longer processing time).\n");
		msg("+unk: tries to process unknown words.\n");
		msg("+sS: search for postcode S and include tiers that have it\n");
		msg("-sS: search for postcode S and exclude tiers that have it\n");
		msg("+tS: include tier code S\n");
		msg("-tS: exclude tier code S\n");
		msg("    +/-t#Target_Child - select target child's tiers\n");
#ifdef POSTCODE
		msg("    +/-t@id=\"*|Mother|*\" - select mother's tiers\n");
#else
		msg("+t%%S: use %%S tier instead of %mor\n");
#endif
#ifdef INTERNAL
		msg("+i[c0|c1]: use internal mor analysis\n");
		msg("+ifS: S output format (mor or pmor, wmor, wpmor) of internal mor analysis\n");
		msg("+ieP: P is added to environment variable POST\n");
#endif
#ifdef POSTCODE
		msg("+re : run program recursively on all sub-directories.\n");
		if (replaceFile)
			msg("-1  : do not replace original data file with new one.\n");
		else
			msg("+1  : replace original data file with new one.\n");
#endif
		main_return;
	}

	init_tags();
	init_mortags();
	init_rules();

	inmemory = 1;
	internal_mor = InternalNone;
	// skip through all parameters to process options.
	for (i=1;i<argc;i++ ) {
		if ( argv[i][0] == '+' || argv[i][0] == '-' ) {
			switch( argv[i][1] ) {
				case 'g' :
					if ( argv[i][2] == '0' ) style = 0;
					else if  ( argv[i][2] == '1' ) style = 1;
					else if  ( argv[i][2] == '2' ) style = 2;
					else if  ( argv[i][2] == '3' ) style = 3;
					else if  ( argv[i][2] == '4' ) style = 4;
					else if  ( argv[i][2] == '5' ) style = 5;
					else if  ( argv[i][2] == '6' ) style = 6;
					else {
						msg ( "bad parameter after +g: should be 0 to 6\n" );
						main_return;
					}
					break;
				case 'd' :
					if (argv[i][2] == 'v' || argv[i][2] == 'V') {
						isReportConllError = !isReportConllError;
					} else if (argv[i][2] == '1' && argv[i][3] == EOS) {
					} else if (argv[i][2] != EOS) {
						strcpy(dbname, argv[i]+2);
//						uS.str2FNType(dbname, 0L, argv[i]+2);
					} else {
						msg ( "bad parameter after +d: should be a database file name\n" );
						main_return;
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
						if (linelimit < 10 || linelimit > 1024)
							linelimit = 70;
					}
					break;
				case 'a' :
					priority_freq_local = atoi( argv[i]+2 );
					if (priority_freq_local < 0 || priority_freq_local > 30) {
						msg ( "parameter priority freq local >=0 and <=30\n" );
						main_return;
					}
					break;
				case 'u' :
					if (argv[i][2] == 'n' && argv[i][3] == 'k') {
						style_unkwords = 1;
					}
					break;
				case 'c' :
					if ( argv[i][2] == '\0' ) {
						if (argv[i][0] == '+') filter_out = KeepAll;
						else filter_out = ThrowAwayAll;
						gb_filter_name[0] = 0;
					} else {
						strcpy(gb_filter_name, argv[i]+2);
//						uS.str2FNType(gb_filter_name, 0L, argv[i]+2);
#if defined(UNX)
						if ( access(gb_filter_name, 04))
#elif defined(__STDC__) && !defined(POSTCODE)
						if ( _access(gb_filter_name, 04))
#else
						if ( access(gb_filter_name, 04))
#endif
															{
#ifdef POSTCODE
							FILE *tfp;
							tfp=OpenMorLib(gb_filter_name, "r", FALSE, FALSE, mFileName);
							if (tfp == NULL)
								tfp= OpenGenLib(gb_filter_name,"r",FALSE,FALSE,mFileName);
							if (tfp == NULL) {
								msg ( "unable to open %s for reading (parameter %s)\n", argv[i]+2, argv[i] );
								main_return;
							} else {
								fclose(tfp);
							}
#else
							if ( fileresolve(gb_filter_name, mFileName) )
								strcpy( gb_filter_name, mFileName);
							else {
								msg ( "unable to open %s for reading (parameter %s)\n", argv[i]+2, argv[i] );
								main_return;
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
					if (argv[i][0] == '-') {
						strcpy(outfilename, "con");
//						uS.str2FNType(outfilename, 0L, "con");
					} else if (argv[i][2] != '\0') {
						strcpy(outfilename, argv[i]+2);
//						uS.str2FNType(outfilename, 0L, argv[i]+2);
					}
					break;
				case 'b' :
					if (argv[i][0] == '-')
						brill = 0;
					else if (argv[i][2] == 's')
						brill = 2;
					break;
				case 's' :
					break;
				case 't' :
					break;
#ifdef INTERNAL
				case 'i' :
					if (argv[i][2] == 'e') {
						setenv("POST", argv[i]+3, 1);
						break;
					}
					if (argv[i][2] == 'f') {
						internal_mor &= ~InternalNone;
						if ( strstr(argv[i]+3,"wpmor") )
							internal_mor |= StyleMorFormPho;
						else if ( strstr(argv[i]+3,"pmor") )
							internal_mor |= StyleMorPho;
						else if ( strstr(argv[i]+3,"wmor") )
							internal_mor |= StyleMorForm;
						else if ( strstr(argv[i]+3,"mor") )
							internal_mor |= StyleMor;
						else {
							msg("parameter +if only +ifmor or +ifpmor\n");
							main_return;
						}
						break;
					}
					load_Catfix( fileresolve( catfix_name , str ) );
					if (argv[i][2] == 'c') {
						internal_mor &= ~InternalNone;
						internal_mor |= InternalCompound|StyleMor;
						if (argv[i][3] == CompoundUS) {
							compound_set_language(CompoundUS);
							compound_set_keep_old_compounds(0);
							compound_set_change_minus_to_plus(1);
							compound_set_full_dot_at_end(1);
							/*!*/ init_exps( fileresolve("compounds.cut", str) );
						} else if (argv[i][3] == CompoundFR) {
							compound_set_language(CompoundFR);
							compound_set_keep_old_compounds(0);
							compound_set_change_minus_to_plus(1);
							compound_set_full_dot_at_end(1);
							/*!*/ init_exps( fileresolve("compounds.cut", str) );
						} else {
							msg("parameter +ic only +ic0 (US) or +ic1 (FR)\n");
							main_return;
						}
						break;
					}
					internal_mor &= ~InternalNone;
					internal_mor |= StyleMor;
					//msg("parameter +i only +if or +ic\n");
					//main_return;
					break;
#endif
#ifdef POSTCODE // following is NEEDED FOR CLAN
				case 'r' :
					break;
				case '1' :
					break;
#endif // above is NEEDED FOR CLAN
				case 'p':
					if (argv[i][2] == '1') {
						break;
					}
				default:
					msg ( "unknown parameter %s\n", argv[i] );
					main_return;
					break;
			}
		}
	}

	// initialization once for all.
	if ( (i=post_init(dbname, inmemory)) < 1) {
		if (i == -1) {
			if (brill == 2)
				msg( "warning: the are no Brill's rules in this database.\n");
			brill = 0;
		} else {
			msg ( "error: the file \"%s\" cannot be opened\n", dbname );
			main_return;
		}
	} else
		msg( "Brill's rules are used from this database.\n");

	gbin_filter_open();
	if (gb_filter_name[0]!='\0')
		gbout_filter_create(gb_filter_name);
	posErrType = "w";
	readConllFile(dbname);
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
			call(argv[i], argv, argc) ; // argv and argc are used of -+t option
		}
	}
#endif
	freeConll();
	gbin_filter_free();
	gbout_filter_free();
	dynfreeall();

	close_all();
}
