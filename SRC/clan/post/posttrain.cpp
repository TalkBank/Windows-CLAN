/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- posttrain.cpp        - v1.0.0 - november 1998

this file contains the main procedure of the POST training software.

int main( int argc, char** argv );
        This function parses the DOS input line.
        It sorts out the option but do not perform any computation.
        and calls the analyze process using the function 'train'
        RETURNS: 0 if OK, -1 if there is an error in the parameters, -2 if there is an error in the analyze process.

        It stands as an example of the calling process.
        The main function train(), may be called from any program.

        Arguments of train:

        int train( char* databasename, char* name_of_input_MORandTRNed_file,
                   int creation_mode, int style_of_analyze, FILE* output_text_file );

|=========================================================================================================================*/

#include "system.hpp"
#include "database.hpp"

#ifdef POSTCODE
	#define CHAT_MODE 1
  #if !defined(UNX)
	#define _main posttrain_main
	#define call posttrain_call
	#define getflag posttrain_getflag
	#define init posttrain_init
	#define usage posttrain_usage
  #endif
	#define main_return return

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
			case 's':
			case 't':
				maingetflag(f-2,f1,i);
				break;
		}
	}
#elif UNX || (!defined(POSTCODE) && !defined(MAC_CODE))
	#define CLAN_MAIN_RETURN int
	#define main_return return(0);
#else
	#include <io.h>

	#define main_return return
#endif

extern char isReportConnlError;

static int style;
static int brillstyle = FullRules;
static int brill_threshold = 2;
static int cmode = 1;
static int withmatrix = 4;
static FNType outfilename[FNSize];
static FNType output_for_brill[FNSize];
static int inmemory = 1;

#ifdef CLANEXTENDED
#define call posttrain_call
#endif

void call(
#ifndef POSTCODE
			char *oldfname
#endif
                                ) {
	int res;
#ifdef POSTCODE
	fclose(fpin);
	fpin = NULL;
#endif
	if (style&BrillsRules) {
		if (style&PrintForTest) {
#if UNX || (!defined(POSTCODE) && !defined(MAC_CODE))
			res = strcmp(outfilename, "con");
#else
			res = uS.FNTypecmp(outfilename, "con", 0L);
#endif
			if ( res && outfilename[0] != '\0' )
				train_file( oldfname, (PrintForTest|BrillsRules), withmatrix, outfilename ); // normal training
			else {
				msg( "with option +bx, ouput filename cannot be the console. Please use option +o'filename'\n");
			}
			return;
		}
		if (!(style&BrillsOnly))
			train_file( oldfname, (style&~BrillsRules), withmatrix, outfilename ); // normal training
		train_brill( oldfname, output_for_brill, brillstyle, brill_threshold ); // train brill's rules
	} else
		train_file( oldfname, style, withmatrix, outfilename );  // normal training
	cmode = 1;
}

#ifdef CLANEXTENDED
void posttrain_main( int argc, char** argv )
#else
CLAN_MAIN_RETURN main( int argc, char** argv)
#endif
{
	int i;
	FNType gb_filter_name[FNSize];
	FNType dbname[FNSize];

	outfilename[0] = 0;
	output_for_brill[0] = 0;
	gb_filter_name[0] = 0;
//2009-07-22	define_databasename(dbname);
	strcpy(dbname, "post.db");
	strcpy(gb_filter_name, "tags.cut");
	style = ChatOutput;
/* 2012-05-03 lxs added BEGIN*/
	if ( access(gb_filter_name, 04)) {
#if defined(POSTCODE) || defined(CLANEXTENDED)
		FILE *tfp;
		FNType mFileName[FNSize];
		tfp=OpenMorLib(gb_filter_name, "r", FALSE, FALSE, mFileName);
		if (tfp == NULL)
			tfp= OpenGenLib(gb_filter_name,"r",FALSE,FALSE,mFileName);
		if (tfp == NULL) {
#endif
			style |= DefaultAffixes;
			gb_filter_name[0] = 0;
#if defined(POSTCODE) || defined(CLANEXTENDED)
		} else {
			fclose(tfp);
		}
#endif
	}
/* 2012-05-03 lxs added END*/
//	style |= DefaultAffixes;

//	int withsuffix = 0;

	brillstyle = FullRules;
	brill_threshold = 2;
	cmode = 1;
	withmatrix = 4;
	inmemory = 1;

	init_connl();
	init_mortags();
	init_tags();
	init_rules();
	init_trainproc();

	if ( argc<2 ) {
		msg("POST-TRAINING: version - %s - INSERM - PARIS - FRANCE\n",MSA_VERSION);
#ifdef POSTCODE
		msg("Usage: posttrain [a cF eF m oF gN sS tF] filename(s)\n");
#else
		msg("Usage: posttrain [a cF eF m oF gN] filename(s)\n");
#endif
		msg("+a : used for training the frequencies of specific word categories\n");
		msg("+b : extended learning using Brill's rules\n");
		msg("-b : Brill's rules training only\n");
		msg("+boF: append output of Brill's training to file F (default: send it to screen)\n");
		msg("+bN: parameter for Brill's rules\n");
		msg("    1- means normal rules are produced (default)\n");
		msg("    2- means only lexical rules are produced\n");
		msg("    3- means partial rules are produced\n");
		msg("    4- means partial rules with limited lexical access are produced\n");
		msg("+btN: threshold for Brill's rules (default=2)\n");
		msg("+c : create new POST database file with the name %s (default)\n", dbname);
		msg("+cF: create new POST database file with the name F\n");
		msg("-c : add to an existing version of %s\n", dbname);
		msg("-cF: add to an existing POST database file with the name F\n");
		if (isReportConnlError)
			msg("+dv : do not report missing elements from \"connl.cut\" file\n");
		else
			msg("+dv : report missing elements from \"connl.cut\" file\n");
		msg("+eF: the affixes and stems in file F are used for training (default: \"tags.cut\" file is used)\n");
		msg("-e : no specific file of affixes and stems is used for training (default: if no \"tags.cut\" file found)\n");
		msg("+mN: load the disambiguation matrices N into memory (about 700K)\n");
		msg("    0- means no matrix training at all\n");
		msg("    2- means matrix training with matrix of size 2\n");
		msg("    3- means matrix training with matrix of size 3\n");
		msg("    4- means matrix training with matrix of size 4 (default)\n");
		msg("+oF: append errors output to file F (default: send it to screen)\n");
		msg("+gN: choose any combination of N output formats\n");
		msg("    0- gives an error for every incompatibility of %%mor: and %%trn: lines (default)\n");
		msg("    1- gives one word per line in three columns: word, %%trn: and %%mor: categories\n");
		msg("    2- is like +s0 but it outputs all tiers, error or no error.\n");
//		msg("+x : use syntactic categories suffixes to deal with tag compounds.\n");
		msg("+lm: reduce memory use (but longer processing time).\n");
#ifdef INTERNAL
		msg("+ifS: S input format (mor or xmor) of mor field\n");
#endif
#ifdef POSTCODE
		puts("+re: run program recursively on all sub-directories.");
		msg("+sS: search for postcode S and include tiers that have it\n");
		msg("-sS: search for postcode S and exclude tiers that have it\n");
		puts("+tS: include tier code S");
		puts("-tS: exclude tier code S");
		puts("    +/-t#Target_Child - select target child's tiers");
		puts("    +/-t@id=\"*|Mother|*\" - select mother's tiers");
#endif
		main_return;
	}

// computation of parameters.
	for (i=1;i<argc;i++ ) {
		if (argv[i][0] == '+' || argv[i][0] == '-') {
			switch( argv[i][1] ) {
				case 'd' :
					if (argv[i][2] == 'v' || argv[i][2] == 'V') {
						isReportConnlError = !isReportConnlError;
					} else {
						msg ( "bad parameter after +d: should be a database file name\n" );
						main_return;
					}
					break;
				case 'a' :
					style |= AllWords;
					break;
				case 'b' :
					style |= BrillsRules;
					if ( argv[i][0] == '-' ) style |= BrillsOnly;
					if ( argv[i][2] == 'o' ) {
#if UNX || (!defined(POSTCODE) && !defined(MAC_CODE))
						strcpy(output_for_brill, argv[i]+3);
#else
						uS.str2FNType(output_for_brill, 0L, argv[i]+3);
#endif
					} else if ( argv[i][2] == 'x' ) style |= PrintForTest;
					else if ( argv[i][2] == 't' ) {
						brill_threshold = atoi( &argv[i][3] );
						if ( brill_threshold < 0 || brill_threshold > 100000 ) {
							msg ( "bad parameter after +bt: range is 0 - 100000\n" );
							main_return;
						}
					} else if ( argv[i][2] == '1' ) brillstyle = FullRules;
					else if ( argv[i][2] == '2' ) brillstyle = LexicalAccess|FullRules;
					else if ( argv[i][2] == '3' ) brillstyle = PartialRules;
					else if ( argv[i][2] == '4' ) brillstyle = LexicalAccess|PartialRules;
					break;
				case 'l' :
					if (argv[i][2] == 'm') {
						inmemory = 0;
					} else
						msg ( "bad use of parameter: %s\n", argv[i] );
					break;
				case 'c' :
					if (argv[i][0] == '+') {
						cmode = 1;
					} else {
						cmode = 0;
					}
					if (argv[i][2] != EOS) {
						strcpy(dbname, argv[i]+2);
//						uS.str2FNType(dbname, 0L, argv[i]+2);
					}
					break;
/*
				case 'x' :
					withsuffix = 1;
					break;
*/
				case 'm' :
					if ( argv[i][2] == '2' ) withmatrix = 2;
					else if  ( argv[i][2] == '3' ) withmatrix = 3;
					else if  ( argv[i][2] == '4' ) withmatrix = 4;
					else if  ( argv[i][2] == '0' ) withmatrix = 0;
					else {
						msg ( "bad parameter after +m: should be 0, 2, 3 or 4\n" );
						main_return;
					}
					break;
				case 'g' :
					if ( argv[i][2] == '0' ) style |= ChatOutput;
					else if  ( argv[i][2] == '1' ) style |= VertOutput;
					else if  ( argv[i][2] == '2' ) style |= ChatFullOutput;
					else {
						msg ( "bad parameter after +g: should be 0,1 or 2\n" );
						main_return;
					}
					break;
				case 'o' :
#if UNX || (!defined(POSTCODE) && !defined(MAC_CODE))
					strcpy(outfilename, argv[i]+2);
#else
					uS.str2FNType(outfilename, 0L, argv[i]+2);
#endif
					break;
				case 'e' :
					if (argv[i][0] == '-') {
						gb_filter_name[0] = 0;
						style |= DefaultAffixes;
					} else {
						if ( argv[i][2] == '\0' ) {
							msg ( "there must be a file name after +e\n" );
							main_return;
						} else {
#if UNX || (!defined(POSTCODE) && !defined(MAC_CODE))
							strcpy(gb_filter_name, argv[i]+2);
#else
							uS.str2FNType(gb_filter_name, 0L, argv[i]+2);
#endif
							if ( access(gb_filter_name, 04)) {
#if defined(POSTCODE) || defined(CLANEXTENDED)
								FILE *tfp;
								FNType mFileName[FNSize];
								tfp=OpenMorLib(gb_filter_name, "r", FALSE, FALSE, mFileName);
								if (tfp == NULL)
									tfp= OpenGenLib(gb_filter_name,"r",FALSE,FALSE,mFileName);
								if (tfp == NULL) {
#endif
									msg ( "unable to open %s for reading (parameter %s)\n", argv[i]+2, argv[i] );
									main_return;
#if defined(POSTCODE) || defined(CLANEXTENDED)
								} else {
									fclose(tfp);
								}
#endif
							}
							// if option +e is set and ok, suppress default option DefaultAffixes
							style &= ~DefaultAffixes;
						}
					}
					break;
#ifdef INTERNAL
				case 'i' :
					if (argv[i][2] == 'f') {
						if ( strstr(argv[i]+3,"xmor") )
							style |= InternalXMorFormat;
						break;
					}
					break;
#endif
#ifdef POSTCODE // following is NEEDED FOR CLAN
				case 'r' :
					break;
				case 's' :
					break;
				case 't' :
					break;
#endif // above is NEEDED FOR CLAN
				default:
					msg ( "unknown parameter %s\n", argv[i] );
					main_return;
					break;
			}
#ifndef POSTCODE // following is NEEDED FOR CLAN
			argv[i][0] = '\0';      // strip the parameter (should not be used any more)
#endif // above is NEEDED FOR CLAN
		}
	}

	unlink( outfilename );
	// FILE* f = fopen(outfilename, "w" );
	// if (f) fclose(f);

	// initialize database
	if (!train_open( dbname, cmode, inmemory ))
		main_return;

	if ( cmode == 0 && gb_filter_name[0]!='\0' ) {
		msg( "ERROR: Option +e%s can only be used when option +c is also used.\n", gb_filter_name);
//		msg( "warning: option +e%s ignored\n", gb_filter_name );
		main_return;
	}

	if ( cmode == 1 ) {
		if (gb_filter_name[0]!='\0') {
			gbin_filter_create(gb_filter_name);
		} else {
			gbin_filter_initialize();
		}
	} else {
		gbin_filter_open();
	}

	readConnlFile(dbname);
#ifdef POSTCODE
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = POSTTRAIN;
	OnlydataLimit = 0;
	UttlineEqUtterance = TRUE;
	bmain(argc,argv,NULL);
#else
	// skip through all parameters to analyze the required files
	for ( i=1;i<argc;i++ ) {
		if (argv[i][0]!='\0') {
			call( argv[i] );
			cmode = 1;
		}
	}
#endif
	freeConnl();
	free_trainproc();
	gbin_filter_free();
	gbout_filter_free();
	dynfreeall();
	close_all();
}
