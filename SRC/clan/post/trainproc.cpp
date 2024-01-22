/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- trainproc.cpp	- v1.0.0 - november 1998
// syntactic training in chat format
// internal function for external program call
|=========================================================================================================================*/

#include "system.hpp"
// #define DBG0
// #define DBG1
// #define DBG2
// #define DBG8 /* for error on joker categories */

#include "database.hpp"
#if !defined(UNX)
  #include "c_curses.h"
#endif

static char** pwMorCompData=0;		// pointers to split words (not storage)
static char** pwMorDataOriginal=0;	// pointers to split words (not storage)
static char** pwTrainData=0;		// pointers to split words (not storage)
static char** pwTrainDataOriginal=0;// pointers to split words (not storage)
static char** pwUtterance=0;		// pointers to split words (not storage)

#ifdef POSTCODE
	#define strnicmp uS.mStrnicmp
	#define stricmp uS.mStricmp
#endif

void init_trainproc(void) {
	pwMorCompData=0;		// pointers to split words (not storage)
	pwMorDataOriginal=0;	// pointers to split words (not storage)
	pwTrainData=0;		// pointers to split words (not storage)
	pwTrainDataOriginal=0;	// pointers to split words (not storage)
	pwUtterance=0;		// pointers to split words (not storage)
}

void free_trainproc(void) {
	if (pwMorCompData != 0)
		delete [] pwMorCompData ;
	if (pwMorDataOriginal != 0)
		delete [] pwMorDataOriginal ;
	if (pwTrainData != 0)
		delete [] pwTrainData ;
	if (pwTrainDataOriginal != 0)
		delete [] pwTrainDataOriginal ;
	if (pwUtterance != 0)
		delete [] pwUtterance ;
}

int train_open(FNType* dbname, int cmode, int inmemory)
{
	int v;
	if (cmode) {
#ifdef POSTCODE
		strcpy(DirPathName, wd_dir);
		addFilename2Path(DirPathName, dbname);
		v = create_all( DirPathName, inmemory );
#else
		v = create_all( dbname, inmemory );
#endif
	} else
		v = open_all( dbname, inmemory );
	if (!v) {
		msg ( "error: cannot %s file %s\n", cmode ? "create" : "open", dbname);
		return 0;
	} else
		return 1;
}

#ifndef UNX
static void addColor(int num, char *line) {
	int  i, cnt;
	char isSpaceFound;

	isSpaceFound = FALSE;
	for (i=0; isSpace(line[i]) || line[i] == '\n' || line[i] == '\r'; i++) ;
	cnt = 1;
	for (; line[i] != EOS; i++) {
		if (isSpace(line[i]) || line[i] == '\n' || line[i] == '\r') {
			if (!isSpaceFound) {
				isSpaceFound = TRUE;
				cnt++;
			}
		} else
			isSpaceFound = FALSE;
		if (cnt == num) {
			for (; isSpace(line[i]) || line[i] == '\n' || line[i] == '\r'; i++) ;
			uS.shiftright(line+i, 2);
			line[i++] = ATTMARKER;
			line[i] = error_start;
			i += 2;
			for (; line[i] != EOS && !isSpace(line[i]) && line[i] != '\n' && line[i] != '\r'; i++) ;
			uS.shiftright(line+i, 2);
			line[i++] = ATTMARKER;
			line[i++] = error_end;
			break;
		}
	}
}
#endif
/*
static void getStemStr(char *to, int num, char *line) {
	int  i, j, cnt;
	char isSpaceFound;

	isSpaceFound = FALSE;
	for (i=0; isSpace(line[i]) || line[i] == '\n' || line[i] == '\r'; i++) ;
	cnt = 1;
	for (; line[i] != EOS; i++) {
		if (isSpace(line[i]) || line[i] == '\n' || line[i] == '\r') {
			if (!isSpaceFound) {
				isSpaceFound = TRUE;
				cnt++;
			}
		} else
			isSpaceFound = FALSE;
		if (cnt == num) {
			for (; isSpace(line[i]) || line[i] == '\n' || line[i] == '\r'; i++) ;
			if (line[i] == ATTMARKER)
				i += 2;
			for (; line[i] != EOS && line[i] != ATTMARKER && line[i] != '^' && line[i] != '|' && !isSpace(line[i]) && line[i] != '\n' && line[i] != '\r'; i++) ;
			j = 0;
			if (line[i] == '|') {
				for (; line[i] != EOS && line[i] != ATTMARKER && line[i] != '^' && !isSpace(line[i]) && line[i] != '\n' && line[i] != '\r'; i++) {
					to[j++] = line[i];
				}
			}
			to[j] = EOS;
			break;
		}
	}
}
*/
static void print_errmortag( FILE* out, ambmortag* amt, int num, char *line)
{
	int  j;

	msgfile( out, "(%d ", amt->nA );
	for ( j = 0; j < amt->nA ; j++ ) {
		msgfile( out, "{%d} ", j+1 );
		print_mortag( out, &amt->A[j] );
		if (j == 0) {
//			getStemStr(spareTier3, num, line);
//			msgfile( out, "%s",  spareTier3);
		}
		if (j!=amt->nA-1)
			msgfile( out, " " );
	}
	msgfile( out, " )" );
}

int train_file(FNType* name, int style, int withmatrix, FNType* outfilename)
{
	int res;
	// below allocated once for all.
	long32 postLineno = 0L, spLineno = 0L;
	long32 nb_of_new_rules = 0L;
	int nboklines = 0;
	char isConllError;
	struct SimpleTier *convMorT = NULL;

	int __total_words = 0;	// total words analyzed localy
	int __total_words_amb = 0;	// total words ambiguous of the above

	isConllError = FALSE;
	if ( pwMorCompData == 0 ) {
		pwMorCompData = new char* [MaxNbWords];
		pwMorDataOriginal = new char* [MaxNbWords];
		pwTrainData = new char* [MaxNbWords];
		pwTrainDataOriginal = new char* [MaxNbWords];
		pwUtterance = new char* [MaxNbWords];
	}

	FILE* outfile = stdout;
	if ( !(style&BrillsRules) ) {
#if defined(UNX) || (!defined(POSTCODE) && !defined(MAC_CODE))
		res = strcmp(outfilename, "con");
#else
		res = uS.FNTypecmp(outfilename, "con", 0L);
#endif
		if ( res && outfilename[0] != '\0' ) {	// if not output to console.
			outfile = fopen( outfilename, "a" );
			if (!outfile) {
				msg ( "the file \"%s\" cannot be opened or created\n", outfilename );
				return 0;
			}
#ifdef POSTCODE
#ifdef _MAC_CODE
			settyp(outfilename, 'TEXT', the_file_creator.out, FALSE);
#endif /* _MAC_CODE */
#endif		
		}	
	}
	if (!(style&BrillsRules)) msg( "training using file <%s>\n", name );

	int mor_groups =0;
	int total_errors =0;

	if ( ! open_rules_dic( "rules" ) ) {
		msg ( "trunk rules cannot be opened...\n" );
		return 0;
	}

	// initialize Matrix file.
	if ( ! open_matrix_dic( "matrix" ) ) {
		msg ( "trunk file %s cannot be opened...\n", "matrix" );
		return 0;
	}

	if ( width_of_matrix() < withmatrix )
		set_width_of_matrix(withmatrix);

	// initialize local word file.
	if ( ! open_localword_dic( "localwords" ) ) {
		msg ( "trunk file %s cannot be opened...\n", "localwords" );
		return 0;
	}
	int id = init_input( name, "mor" );
	if (id<0) {
		msg ( "training file %s cannot be found2\n", name);
		return 0;
	}

	FILE* FBRILLs = NULL;
	if ( style&BrillsRules) {
		// initialize local word file.
		if ( ! open_brillword_dic( "brillwords" ) ) {
			msg ( "brill word file %s cannot be opened...\n", "brillwords" );
			return 0;
		}
		if ( ! open_brillrule_dic( "brillrules" ) ) {
			msg ( "brill word file %s cannot be opened...\n", "brillrules" );
			return 0;
		}
		FBRILLs = fopen( outfilename, "w" );
		if ( !FBRILLs ) {
			msg ( "temporary file for Brill's rules cannot be created...\n" );
			return 0;
		}
	}


	int n, y;
	char  pct_at_end = '\0';	// store a possible punctuation at the end of the sentence.
	char* T;
	char* TMorForDisplay = NULL; // full %mor line
	char* TTrainForDisplay = NULL; // full %trn line
	char* TFUtterance = NULL; // full *CHI: line

	char* storeMorComp;	// storage for word split
	char* storeMorOriginal; // storage for word split and display
	char* storeTrain;	// storage for word split
	char* storeTrainOriginal; // storage for word split and display
	char* storeUtterance;	// storage for word split

	int nMorCompData, nTrainData, nUtterance, ii=7;	// must wait for a name

	rightSpTier = 0;
	while ( (n = get_tier(id, T, y, postLineno)) != 0 ) {

#ifdef DBG0
msgfile( outfile, "!--------------------------------------------!\n" );
msgfile( outfile, "[[000[[%s]]---]]\n", T );
#endif

		if ( y == TTname ) {
			spLineno = postLineno;
			dynreset();
			ii = 3;		// now wait for a %mor: and a %trn:

			pct_at_end = '\0';	// store a possible punctuation at the end of the sentence.
			int l = strlen(T) -1;
			while ( l>0 && strchrConst( " \n\t\r", T[l] ) ) l--;
			char* p=(char *)strchrConst( ".?!", T[l] );
			if (p) {
				if ( strnicmp(&T[l-2],"...",3) )	// checks if not a ...
					pct_at_end = *p;
			}

			TFUtterance = dynalloc( strlen(T)+2 );
			strcpy( TFUtterance, T );

			continue;
		} else if ( y == TTmor ) {
			if (isConllSpecified()) {
				convMorT = convertTier(T, name, &isConllError);
				if (isConllError)
					return 0;
				freeConvertTier(convMorT);
			}
			if ( (!(style&InternalXMorFormat) && !_strnicmp( T, "%mor", 4 )) || (style&InternalXMorFormat &&  !_strnicmp( T, "%xmor", 5 )) ) {
				ii &= ~1;
				int l = strlen(T) -1;
				while ( l>0 && strchrConst( " \n\t\r", T[l] ) ) l--;
				char* p=(char *)strchrConst( ".?!", T[l] );
				if (pct_at_end && !p ) { // incoherence between use of punctuation
					T[l+1] = ' ';
					T[l+2] = pct_at_end;
					T[l+3] = '\n';
					T[l+4] = '\0';
				}

				// makes a copy for later computation (mandatory) and display (optional)
				TMorForDisplay = dynalloc( strlen(T)+1002 );
				strcpy( TMorForDisplay, T );
			}
		} else if ( y == TTtrn ) {
			if (isConllSpecified()) {
				convMorT = convertTier(T, name, &isConllError);
				if (isConllError)
					return 0;
				freeConvertTier(convMorT);
			}
			if ( !_strnicmp( T, "%trn", 4 ) ) {

				ii &= ~2;
				int l = strlen(T) -1;
				while ( l>0 && strchrConst( " \n\t\r", T[l] ) ) l--;
				char* p=(char *)strchrConst( ".?!", T[l] );
				if (pct_at_end && !p ) { // incoherence between use of punctuation
					T[l+1] = ' ';
					T[l+2] = pct_at_end;
					T[l+3] = '\n';
					T[l+4] = '\0';
				}

				// makes copies for later computation (mandatory) and display (optional)
				TTrainForDisplay = dynalloc( strlen(T)+1002 );
				strcpy( TTrainForDisplay, T );

			}
		}

#ifdef DBG0
msgfile( outfile, "!+++++++++++++++++++++++++++++++++++++++++++!\n" );
msgfile( outfile, "{%d}\n", ii );
#endif

		if ( ii == 0 ) {
			// split %mor and %trn line
			{
				char *cleanedMorLine, *cleanedTrainLine;
				// FIRST SPLIT SENTENCE WITH WHITESPACES and normalize punctuation at the end
				// AMBIGUOUS SENTENCE (%mor)
#ifdef DBG1
msg("TMorForDisplay=/%s/\n",TMorForDisplay);
#endif
				storeMorComp = dynalloc( strlen(TMorForDisplay)*2+1 );
				// clean the line of new element such as [xx]
				cleanedMorLine = dynalloc( strlen(TMorForDisplay)+1 );
				strcpy( cleanedMorLine, TMorForDisplay );
				strip_of_brackets( cleanedMorLine );  // Clear post-codes
				nMorCompData = split_with_a( cleanedMorLine, pwMorCompData, storeMorComp, WhiteSpaces, PunctuationsAtEnd, PunctuationsInside, MaxNbWords, strlen(TMorForDisplay)*2 );
#ifdef DBG1
msg("TMorForDisplay=/%s/\n",TMorForDisplay);
#endif
				if ( style & VertOutput ) {
					storeMorOriginal = dynalloc( strlen(TMorForDisplay)*2+1 );
					split_with_a( TMorForDisplay, pwMorDataOriginal, storeMorOriginal, WhiteSpaces, PunctuationsAtEnd, PunctuationsInside, MaxNbWords, strlen(TMorForDisplay)*2 );
				}
				// replace :mor. with '.' because first word must of a sentence must always be a '.'
				strcpy( pwMorCompData[0], "." );
				int i, j;
				for (i=0; i<nMorCompData; i++) {
					if ( strchrConst( non_internal_pcts, pwMorCompData[i][0] ) ) {
						for (j=i; j<nMorCompData-1; j++ )
							pwMorCompData[j] = pwMorCompData[j+1];
						nMorCompData --;
					}
				}
#ifdef DBG0
msg("nMorCompData[%d]\n",nMorCompData);
for (i=0; i<nMorCompData; i++) msg("{%s} ", pwMorCompData[i] ); msg("\n");
#endif
				// CORRECT SENTENCE (%trn)
				storeTrain = dynalloc( strlen(TTrainForDisplay)*2+1 );
				storeTrainOriginal = dynalloc( strlen(TTrainForDisplay)*2+1 );

				// clean the line of new element such as [xx]
				cleanedTrainLine = dynalloc( strlen(TTrainForDisplay)+1 );
				strcpy( cleanedTrainLine, TTrainForDisplay );
				strip_of_brackets( cleanedTrainLine );  // Clear post-codes

				nTrainData = split_with_a( cleanedTrainLine, pwTrainData, storeTrain, WhiteSpaces, PunctuationsAtEnd, PunctuationsInside, MaxNbWords, strlen(TTrainForDisplay)*2 );
				// replace :mor. with '.' because first word must of a sentence must always be a '.'
				strcpy( pwTrainData[0], "." );
				for (i=0; i<nTrainData; i++) {
					if ( strchrConst( non_internal_pcts, pwTrainData[i][0] ) ) {
						for (j=i; j<nTrainData-1; j++ )
							pwTrainData[j] = pwTrainData[j+1];
						nTrainData --;
					}
				}
#ifdef DBG0
msg("nTrainData[%d]\n",nTrainData);
for (i=0; i<nTrainData; i++) msg("{%s} ", pwTrainData[i] ); msg("\n");
#endif

				if ( nMorCompData == nTrainData) { // otherwise it is not necessary to process tier information anymore.

					// HANDLES THE CASE OF TAG/GROUPS, WORD/GROUPS AND COMPOUND WORDS
					// Note: for training phase, there is no need to reconstruct the original splited groups
					// so it is not needed to store the actual separators of the groups but it is necessary to add
					// as many elements in the %trn line than in the %mor, so "joker" elements may be necessary.
#ifdef DBG8
	msgfile( outfile, "\n" );
#endif
					for (i=0; i<nMorCompData; i++) {
						char* st_mor;
						char* st_trn;
#ifdef DBG8
	msgfile( outfile, "%s! !%s!\n", pwTrainData[i], pwMorCompData[i] );
#endif
						if ( make_multi_categ( pwMorCompData[i], st_mor) )
							pwMorCompData[i] = st_mor;
						if ( make_multi_categ( pwTrainData[i], st_trn ) )
							pwTrainData[i] = st_trn;
					}
#ifdef DBG8
msg("nMorCompData[%d]\n",nMorCompData);
for (i=0; i<nMorCompData; i++) msg("{%s} ", pwMorCompData[i] ); msg("\n");
msg("nTrainData[%d]\n",nTrainData);
for (i=0; i<nTrainData; i++) msg("{%s} ", pwTrainData[i] ); msg("\n");
#endif
				}

			}
			// end of split

			ii = 7;	// will wait for next name

			storeUtterance = dynalloc( strlen(TFUtterance)*2 );
			nUtterance = split_original_line( nMorCompData, TFUtterance, pwUtterance, storeUtterance, strlen(TFUtterance)*2 );

			// In the case of the MOR line and the TRN line do not have the same number of words, just output
			if ( nTrainData != nMorCompData ) {
				if ( !(style&BrillsRules) ) {
					if ( style & VertOutput ) {
						msgfile(outfile, "@@@ @@@\n" );
						int i;
						for (i=0;i<nUtterance;i++) {
							msgfile(outfile, "@@@ %s ", pwUtterance[i] );
							msgfile(outfile, "%s ", (i<nTrainData) ? pwTrainData[i] : "---" );
							msgfile(outfile, "%s", (i<nMorCompData) ? pwMorCompData[i] : "---" );
							msgfile(outfile, "\n" );
						}
						msgfile(outfile, "@@@ @@@\n" );
					} else {
						msgfile( outfile, "\n%s", TFUtterance );
						msgfile(outfile,"%s", TMorForDisplay );
						msgfile(outfile,"%s", TTrainForDisplay );
/*						msgfile( outfile, "%%mor:" );
						int i;
						for (i=1; i<nMorCompData; i++) {
							msgfile( outfile, " %s", pwMorCompData[i] );
						}
						msgfile( outfile, "\n%%trn:" );
						for ( i=1 ; i<nTrainData; i++) {
							msgfile( outfile, " %s", pwTrainData[i] );
						}
*/						msgfile( outfile, "\n## not the same number of words between %%mor: and %%trn: lines\n" );
					}
					msgfile( outfile, "*** File \"%s\": line %ld.\n", name, spLineno );
					msgfile( outfile, "--------------------------------------\n");
					total_errors++;
					mor_groups++;
				}
			} else {
#ifdef DBG2
msgfile( outfile, "%s", TFUtterance );
msgfile( outfile, "%s", nUtterance == nMorCompData ? "Split was OK.\n" : "Bad Split or morphological construction (ignored)!!!\n" );
#endif
				// compute data for the training of the current utterance
				mortag* MT = (mortag*)dynalloc(sizeof(mortag) * nTrainData);
				ambmortag* AMT = (ambmortag*)dynalloc(sizeof(ambmortag) * nTrainData);
				int* pc = (int*)dynalloc(sizeof(int) * nTrainData);
				int* uk = (int*)dynalloc(sizeof(int) * nTrainData);
				int* ni = (int*)dynalloc(sizeof(int) * nTrainData);
				char** wrds = (char**)dynalloc(sizeof(char*) * nTrainData);
				int i;
				for (i=0; i<nMorCompData; i++) {
#ifdef DBG1
	if (style&BrillsRules) msgfile( outfile, "[%s]//[%s] ", pwTrainData[i], pwMorCompData[i] );
#endif
#ifdef DBG8
	msgfile( outfile, "%s! !%s!\n", pwTrainData[i], pwMorCompData[i] );
#endif
					if ( ! make_mortag( pwTrainData[i], &(MT[i]) ) ) goto no_memory;
					if ( ! make_ambmortag( pwMorCompData[i], &(AMT[i]) ) ) goto no_memory;
#ifdef DBG77
	msg( "resolutions " );
	print_mortag(outfile,&MT[i]);
	msg("\n");
	msg( " avant filtre " );
	print_ambmortag( outfile, &AMT[i] );
	msg("\n");
#endif
					if (style&BrillsRules) {
						// test if split was OK and not morphological construction.
						if ( nUtterance == nMorCompData ) {
							wrds[i] = dynalloc(1+strlen(pwUtterance[i]));
							strcpy(wrds[i], pwUtterance[i]);
#ifdef DBG2
msgfile( outfile, "(%s) ", wrds[i] );
#endif
						} else {
							if (MT[i].GR) {
								wrds[i] = dynalloc(1+strlen(MT[i].GR));
								strcpy(wrds[i], MT[i].GR);
							} else {
								wrds[i] = dynalloc(1+strlen(MT[i].MT));
								strcpy(wrds[i], MT[i].MT);
							}
#ifdef DBG2
msgfile( outfile, "[%s] ", wrds[i] );
#endif
						}
					}
					if ( !(style&BrillsRules) && i!=0 && i!=nMorCompData-1 && 
						( (style & AllWords) || nMorCompData <= 4 ) ) {
						__total_words ++;
						if ( AMT[i].nA != 1 )
							__total_words_amb ++;
						// INSERT Local word categories before suppressing word information.
						char WC[MaxSizeOfString];
						int wcw = prepare_ambtagword( &(AMT[i]), &(MT[i]), WC );
						if (wcw != -1) {
							add_local_word( WC, wcw, AMT[i].nA );
						}
#ifdef DBG1
	else {
	msg ( "-[%s]-%d-%d (", WC, wcw, AMT[i].nA ); print_mortag(outfile,&MT[i]); msg(")\n");
	}
#endif
					}

					pc[i] = is_pct(&(AMT[i]).A[0]);
					uk[i] = is_unknown_word(&(AMT[i]).A[0]);

#ifdef DBG7
	msg( "resolutions " );
	print_mortag(outfile,&MT[i]);
	msg("\n");
	msg( " avant filtre " );
	print_ambmortag( outfile, &AMT[i] );
	msg("\n");
#endif

					int found = 0;
					for (int k=0; k<AMT[i].nA; k++)
						if ( ! strncmp( MultiCategory, AMT[i].A[k].MT, strlen(MultiCategory) ) ) {
							found = 1;
							break;
						}

					if ( !found ) {
#ifdef DBG7
	msg( "usual %s\n", MT[i].MT );
#endif
						if (style&DefaultAffixes)
							filter_add_all_affixes( &AMT[i] );

						filter_mortag( &MT[i] );
						filter_and_pack_ambmortag( &AMT[i] );
					} else {
						MT[i].GR=0;
						MT[i].nSP=0;
						for (int k=0; k<AMT[i].nA; k++) {
							AMT[i].A[k].GR=0;
							AMT[i].A[k].nSP=0;
						}
					}
					ni[i] = is_in_amb( &AMT[i], &MT[i] );

#ifdef DBG8
	print_mortag(outfile,&MT[i]);
	msgfile( outfile, "\n");
	print_ambmortag(outfile,&AMT[i]);
	msgfile( outfile, "!\n" );
#endif
#ifdef DBG7
	msg( "after resolutions " );
	print_mortag(outfile,&MT[i]);
	msg("\n");
	msgfile( outfile, " apres filtre " );
	print_ambmortag(outfile,&AMT[i]);
	msgfile( outfile, " ni%d,p%d,u%d\n", ni[i],pc[i],uk[i] );
#endif

				}
#ifdef DBG7
msgfile( outfile, "\n" );
#endif

				// train the rules (and print output)
				// flag used to know if some error already occured in the utterance
#ifndef UNX
				if ( (style & VertOutput) == 0) {
					for (i=0; i<nMorCompData-1; i++) {
						if (  (pc[i] || (!uk[i] && ni[i]==1))
							&& (pc[i+1] || (!uk[i+1] && ni[i+1]==1)) ) {
						} else if ( !(style&BrillsRules) && !(pc[i+1] || (!uk[i+1] && ni[i+1]==1)) ) {
							addColor(i+2, TMorForDisplay);
							addColor(i+2, TTrainForDisplay);
						}
					}
				}
#endif
				int error_in_utterance = 0;
				for (i=0; i<nMorCompData-1; i++) {
					if (  (pc[i] || (!uk[i] && ni[i]==1))
					   && (pc[i+1] || (!uk[i+1] && ni[i+1]==1)) ) {
						if ( !(style&BrillsRules) ) {
							// evrything ok, insert rule.
							char * s2 = insert_rule( &MT[i], &AMT[i], &MT[i+1], &AMT[i+1] ); 
							if (s2)
								// twas OK. note it
								nb_of_new_rules ++;
							if ( !s2 ) {
								if ( style & VertOutput ) {
									msgfile( outfile, ">>> error line (p%d,p%d,u%d,u%d,n%d,n%d) (%s %s) * (%s %s)\n", pc[i], pc[i+1], uk[i], uk[i+1], ni[i], ni[i+1], pwTrainData[i], pwMorCompData[i], pwTrainData[i+1], pwMorCompData[i+1] );
									msgfile( outfile, "%s ", pwUtterance[i+1] );
									print_mortag(outfile, &MT[i+1]);
									msgfile( outfile, " %s", pwMorDataOriginal[i+1]);
									msgfile( outfile, " not_inserted\n" );
								} else {
									if (error_in_utterance==0) {
										msgfile(outfile,"%s", TFUtterance );
										msgfile(outfile,"%s", TTrainForDisplay );
									}
									msgfile( outfile, "#%d-%d insertion error --flags: p%d,p%d,u%d,u%d,n%d,n%d\n--for (%s %s) * (%s %s)\n", i+1, i+2, pc[i], pc[i+1], uk[i], uk[i+1], ni[i], ni[i+1], pwTrainData[i], pwMorCompData[i], pwTrainData[i+1], pwMorCompData[i+1] );
									error_in_utterance++;
								}
							} else if ( style & VertOutput ) {
								msgfile( outfile, "%s ", pwUtterance[i+1] );
								print_mortag(outfile, &MT[i+1]);
								msgfile( outfile, " %s\n", pwMorDataOriginal[i+1] );
							}
						}
						/*** else if BrillsRules is ON, this is a only fake processing, no real learning takes place ***/
					} else if ( !(style&BrillsRules) && (style & VertOutput || !(pc[i+1] || (!uk[i+1] && ni[i+1]==1))) ) {
						// no printing for ChatOutput of second word is OK.
						if ( style & VertOutput )
							msgfile( outfile, "%s ", pwUtterance[i+1] );
						else {
							if (error_in_utterance==0) {
								msgfile(outfile,"\n%s", TFUtterance );
								msgfile(outfile,"%s", TMorForDisplay );
								msgfile(outfile,"%s", TTrainForDisplay );
								/*** display of preprocessed data
								msgfile( outfile, "%%mor:" );
								int iii;
								for (iii=1; iii<nMorCompData; iii++) {
									msgfile( outfile, " %s", pwMorCompData[iii] );
								}
								msgfile( outfile, "\n%%trn:" );
								for ( iii=1 ; iii<nTrainData; iii++) {
									msgfile( outfile, " %s", pwTrainData[iii] );
								}
								msgfile(outfile, "\n" );
								***/
								putc('\n', outfile);
							}
							error_in_utterance++;
							msgfile(outfile, "#%d ", i+1 );
							if (i+1<nUtterance)
								msgfile( outfile, "Word: %s ", pwUtterance[i+1] );
						}
						putc('\n', outfile);
						msgfile( outfile, "%%mor: " );
						if ( style & VertOutput )
							msgfile( outfile, "%s", pwMorDataOriginal[i+1] );
						else {
							print_errmortag(outfile, &AMT[i+1], i+2, TMorForDisplay);
						}
						putc('\n', outfile);
						msgfile( outfile, "%%trn: " );
						print_mortag(outfile, &MT[i+1]);
//						getStemStr(spareTier3, i+2, TTrainForDisplay);
//						msgfile( outfile, "%s",  spareTier3);
						if ( ni[i+1]>1 ) {
							// ambiguous class conversion.
							msgfile( outfile, " => multiple_tags[%d]\n", ni[i+1] );
						} else {
							if (uk[i+1])
								msgfile( outfile, " => unknown_word\n" );
							else if (ni[i+1]==0)
								msgfile( outfile, " => different_tags\n" );
							else
								msgfile( outfile, "\n" );
						}
						putc('\n', outfile);
					}
				}
				if (!(style&BrillsRules)) {
					if ( (style & ChatOutput) && error_in_utterance > 0 ) {
						msgfile( outfile, "%d errors in the above utterance\n", error_in_utterance );
						msgfile( outfile, "*** File \"%s\": line %ld.\n", name, spLineno );
						msgfile( outfile, "--------------------------------------\n");
						mor_groups ++;
						total_errors ++;
					} else if ( style & ChatFullOutput ) {
						msgfile(outfile,"%s", TFUtterance );
						msgfile(outfile,"%s", TMorForDisplay );
						msgfile(outfile,"%s", TTrainForDisplay );
					}
					mor_groups ++;
				}

				if ( error_in_utterance == 0 ) {

					if ( (style&BrillsRules) && nMorCompData == nUtterance ) {
						int nottodo = 0;
						for (i=0; i<nMorCompData; i++)
							if ( uk[i] ) {
								nottodo=1; // there is an unknown word. Skip this sentence.
								break;
							}
						if (nottodo==0 && FBRILLs != NULL) {
							if ( style&PrintForTest ) {
								nboklines ++;
								
								fprintf( FBRILLs, "*utt: " );
								for (i=1; i<nUtterance; i++) fprintf( FBRILLs, " %s", pwUtterance[i] );
								// fprintf( FBRILLs, "\n" );
								fprintf( FBRILLs, "\n%%trn:" );
								for (i=1; i<nMorCompData; i++) {
									// TAG t = classname_to_tag(mortag_to_string(&MT[i]));
									// msgfile( FBRILLs, " %d", t );
									// msgfile( FBRILLs, " %s", mortag_to_string(&MT[i]) );
									msgfile( FBRILLs, " %s", pwTrainDataOriginal[i] );
								}
								fprintf( FBRILLs, "\n" );
								
								// fprintf( FBRILLs, "%s", TFUtterance );
							} else {
								nboklines ++;
#ifdef NotAddBrillWords
								fprintf( FBRILLs, "%%wrd: . " );
#else
								fprintf( FBRILLs, "%%wrd: . " );
							//	for (i=1; i<nUtterance; i++) fprintf( FBRILLs, " %s", pwUtterance[i] );
								for (i=1; i<nUtterance; i++) {
									fprintf( FBRILLs, " %s", wrds[i] );
									add_brillword( wrds[i], classname_to_tag(mortag_to_string(&MT[i])) );
								}
#endif
								AMBTAG* A=0;	// input of analyze
								AMBTAG* R=0;	// result of analyze
								A = (AMBTAG*)dynalloc(nMorCompData*sizeof(AMBTAG));

								// fprintf( FBRILLs, "\n%%mor:" );
								int maxl = 0, suml= 0;
								for (i=0; i<nMorCompData; i++) {
									if ( maxl < AMT[i].nA ) maxl = AMT[i].nA;
									suml += AMT[i].nA;
								}
								int* at = (int*)dynalloc((maxl*2+2)*sizeof(int));
								TAG* buf = (TAG*)dynalloc(((nMorCompData+suml*2)+2)*sizeof(TAG));
								for (i=0; i<nMorCompData; i++) {
									// msgfile( FBRILLs, " " );
									int* pC = ambmortag_to_multclass(&AMT[i],at);
									A[i] = AMBTAG(buf);
									buf += (number_of_multclass(pC)+1);
									multclass_to_ambtag( pC, A[i] );
	
									// msgfile( FBRILLs, "%d", number_of_ambtag(A[i]) );
									// for ( int j = 0; j < number_of_ambtag(A[i]) ; j++ )
									//	msgfile( FBRILLs, ",%d", nth_of_ambtag(A[i],j) );
								}
								fprintf( FBRILLs, "\n%%trn:" );
								for (i=0; i<nMorCompData; i++) {
									TAG t = classname_to_tag(mortag_to_string(&MT[i]));
									msgfile( FBRILLs, " %d", t );
								}
								int *annotation = (int *)dynalloc(nMorCompData*sizeof(int));	// store the kind of analysis performed for each word.
								memset( annotation, 0, nMorCompData*sizeof(int) );
								int jmpret = setjmp( mark );
								if( jmpret == 0 ) {
									analyze_algorithm( A, nMorCompData, R, annotation );
									fprintf( FBRILLs, "\n%%pos:" );
									for (i=0; i<nMorCompData; i++) {
										msgfile( FBRILLs, " %d", R[i][1] );
									}						
								} else {
									msgfile( outfile, "not enough memory for the following line :\n %s", T );
								}
								fprintf( FBRILLs, "\n" );
							}
						}
					} else {

						/* now create matrix if needed */
						if (withmatrix>=2) for (i=0; i<nMorCompData-1; i++)
							if (  (pc[i] || (!uk[i] && ni[i]==1))
							   && (pc[i+1] || (!uk[i+1] && ni[i+1]==1)) ) {
								// evrything ok, insert matrix.
								insert_matrix_2( &MT[i], &AMT[i], &MT[i+1], &AMT[i+1] ); 
							}
						if (withmatrix>=3) for (i=0; i<nMorCompData-2; i++)
							if (  (pc[i] || (!uk[i] && ni[i]==1))
							   && (pc[i+1] || (!uk[i+1] && ni[i+1]==1))
							   && (pc[i+2] || (!uk[i+2] && ni[i+2]==1)) ) {
								// evrything ok, insert matrix.
								insert_matrix_3( &MT[i], &AMT[i], &MT[i+1], &AMT[i+1], &MT[i+2], &AMT[i+2] ); 
							}
						if (withmatrix>=4) for (i=0; i<nMorCompData-3; i++)
							if (  (pc[i] || (!uk[i] && ni[i]==1))
							   && (pc[i+1] || (!uk[i+1] && ni[i+1]==1))
							   && (pc[i+2] || (!uk[i+2] && ni[i+2]==1))
							   && (pc[i+3] || (!uk[i+3] && ni[i+3]==1)) ) {
								// evrything ok, insert matrix.
								insert_matrix_4( &MT[i], &AMT[i], &MT[i+1], &AMT[i+1], &MT[i+2], &AMT[i+2], &MT[i+3], &AMT[i+3] ); 
							}

					}
				}
	 		}
		}

	}

	if ( !(style&BrillsRules) && (style&ChatOutput) ) {
		if (total_errors==0)
			msgfile( outfile, "\n----- there were no errors in file \"%s\"\n", name);
		else
			msgfile( outfile, "there were a total of %d errors in a total of %d MOR statements\n", total_errors, mor_groups );
	}
	dynreset();

	close_input(id);
	if (!(style&BrillsRules)) msgfile( outfile, "\n" );
	if ( nb_of_new_rules == 0L ) { // nothing has been done. may be an error in the command.
		if (!(style&BrillsRules)) msgfile( outfile, "No rules were processed. Have you checked the input file?\n" );
		if (!(style&BrillsRules)) msgfile( outfile, "No changes will be made to the database.\n" );
	} else {
		if (!(style&BrillsRules)) msgfile( outfile, "%ld rules were modified or created\n", nb_of_new_rules );
		save_all();
	}
	if (!(style&BrillsRules)) msgfile( outfile, "number of words processed: %d\n", __total_words ); // total words analyzed localy
	if (!(style&BrillsRules)) msgfile( outfile, "number of ambiguous words: %d\n", __total_words_amb ); // total words ambiguous of the above

	if ( style&BrillsRules) {
		fclose(FBRILLs);
	}

	if (outfile!=stdout) fclose(outfile);
	return (style&BrillsRules) ? nboklines : 1;
   no_memory:
	if (!(style&BrillsRules)) msgfile( outfile, "\n" );
	if (!(style&BrillsRules)) msg ( "memory leak during processing...\n" );
	dynreset();
	close_input(id);
	if ( nb_of_new_rules == 0L ) { // nothing has been done. may be an error in the command.
		if (!(style&BrillsRules)) msgfile( outfile, "No rules were processed. Have you checked the input file?\n" );
		if (!(style&BrillsRules)) msgfile( outfile, "No changes will be made to the database.\n" );
	} else {
		if (!(style&BrillsRules)) msgfile( outfile, "%ld rules were modified or created\n", nb_of_new_rules );
		save_all();
	}
	if (!(style&BrillsRules)) msgfile( outfile, "number of words processed: %d\n", __total_words ); // total words analyzed localy
	if (!(style&BrillsRules)) msgfile( outfile, "number of ambiguous words: %d\n", __total_words_amb ); // total words ambiguous of the above
	if (outfile!=stdout) fclose(outfile);
	return 0;
}
