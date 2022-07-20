/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- postana.cpp	- v1.0.0 - november 1998
// syntactic disambiguation of a :mor:-ed chat file.

int post_file
	Gets lines of input from a MORed file
	Each MOR line (still with the %mor: tag) is processed using the function post_analyzemorline()
	(MOR lines should end with a carriage return for a nice output).

|=========================================================================================================================*/

#include "system.hpp"
#ifdef POSTCODE

extern long option_flags[];
extern char isRecursive;

#else

// #define UTTLINELEN 18000L
#ifndef UNX
	#include <io.h>
#endif

#endif
#if defined(UNX)

#ifndef EOS			/* End Of String marker			 */
	#define EOS '\0'
#endif

#endif
// #define debug_0
// #define debug_1
// #define debug_9 /* final categories decomposition and printing problem */
// #define debug_7	/* trace words and tags spliting and extraction */
// #define debug_8	/* trace words and tags output */

#include "database.hpp"
#include "morpho.hpp"
#include "compound.hpp"

#if !defined(UNX) && !defined(POSTCODE)
inline int isspace(char c) { return strchr(WhiteSpaces,c)?1:0; }
#endif

FILE *posErrFp;
const char *posErrType;
long32 spLineno;
char posT[UTTLINELEN], morT[UTTLINELEN], trnT[UTTLINELEN], spT[UTTLINELEN];

static void removeAllExtra(char *tT) {
	char spf = 0;

	if (*tT == '\0')
		return;
	while (*tT) {
		if (*tT == ' ' || *tT == '\t' || *tT == '\n') {
			if (!spf)
				tT++;
			else
				strcpy(tT, tT+1);
			spf = 1;
		} else {
			spf = 0;
			tT++;
		}
	}
	if (*(tT-1) != '\n')
		*(tT-1) = '\n';
}

static long32 lineCnt(char *s) {
	long32 ln = 0L;

	for (; *s; s++) {
		if (*s == '\n')
			ln++;
	}
	return(ln);
}

static int mystrcmp(char *s1, char *s2) {
	for (; *s1 != '\0' && *s2 != '\0'; s1++, s2++) {
		if (*s1 != *s2) {
			if ((isspace(*s1) && *s2 == '\n') ||
				(*s1 == '\n' && isspace(*s2)) ||
				(isspace(*s1) && isspace(*s2))) 
				;
			else
				break;
		}
	}
	return(*s1 != *s2);
}

static void doCompar(FNType *fname, long32 ln) {
	if (*posT == '\0' || *trnT == '\0')
		return;
	removeAllExtra(posT);
	removeAllExtra(trnT);
	if (mystrcmp(posT+5, trnT+5)) {
		if (posErrFp == NULL) {
			posErrFp = fopen("pos_error.cut", posErrType);
			if (posErrFp == NULL)
				return;
			else
				posErrType = "a";
#ifdef POSTCODE
#ifdef _MAC_CODE
			settyp("pos_error.cut", 'TEXT', the_file_creator.out, FALSE);
#endif
#endif
		}
		msgfile( posErrFp, "--------------------------------------\n");
		msgfile( posErrFp, "*** File \"%s\": line %ld.\n", fname, ln );
		msgfile( posErrFp, "%s", spT);
		msgfile( posErrFp, "%s", morT);
		msgfile( posErrFp, "%s", trnT);
		msgfile( posErrFp, "%s", posT);
	}
}

static void removeExtraSpaceFromMor(char *st) {
	int i;

	for (i=0; st[i] != EOS; ) {
		if (st[i]==' ' || st[i]=='\t' || (st[i]=='<' && (i==0 || st[i-1]==' ' || st[i-1]=='\t'))) {
			i++;
			while (st[i] == ' ' || st[i] == '\t')
				strcpy(st+i, st+i+1);
		} else
			i++;
	}
}

static void convertToMORAndPrint(FILE *out, struct SimpleTier *convMorT, char *mor) {
	int  i, len;
	char item[BUFSIZ];
	struct SimpleTier *elem;

	removeExtraSpaceFromMor(mor);
	for (i=0; mor[i] != ':' && mor[i] != EOS; i++) ;
	if (mor[i] == EOS) {
		mor[0] = EOS;
		return;
	}
	for (i++; isSpace(mor[i]); i++) ;
	if (mor[i] == EOS) {
		mor[0] = EOS;
		return;
	}
	strcpy(mor, mor+i);
	strcpy(templineC, mor);
	templineC1[0] = EOS;
	i = 0;
	while ((i=getNextMorItem(templineC, item, NULL, i))) {
		if (convMorT == NULL) {
			fprintf(stderr, "*** POST INTERNAL ERROR 1\n");
//			fprintf(out, "*** POST INTERNAL ERROR 1\n");
			break;
		}
#ifdef POSTCODE
		uS.remFrontAndBackBlanks(item);
#else
		remFrontAndBackBlanks(item);
#endif
		if (strchr(item, '^') != NULL) {
			if (templineC1[0] != EOS)
				strcat(templineC1, " ");
			strcat(templineC1, convMorT->org);
		} else {
			for (elem=convMorT; elem != NULL; elem=elem->nextChoice) {
				if (strcmp(item, elem->sw) == 0) {
					len = strlen(templineC1);
					if (elem->isCliticChr == '$') {
						if (templineC1[0] != EOS && templineC1[len-1] != '$')
							strcat(templineC1, " ");
						strcat(templineC1, elem->ow);
						strcat(templineC1, "$");
					} else if (elem->isCliticChr == '~') {
						strcat(templineC1, "~");
						strcat(templineC1, elem->ow);
					} else {
						if (templineC1[0] != EOS && templineC1[len-1] != '$')
							strcat(templineC1, " ");
						strcat(templineC1, elem->ow);
					}
					break;
				}
			}
			if (elem == NULL) {
				fprintf(stderr, "\n*** ERROR: POST inserted unknown element \"%s\"\n", item);
				fprintf(stderr, "  Expected elements are:\n");
				for (elem=convMorT; elem != NULL; elem=elem->nextChoice) {
					fprintf(stderr, "    \"%s\" for \'%s\"\n", elem->sw, elem->ow);
				}
				if (convMorT->isCliticChr == '$') {
					strcat(templineC1, item);
					strcat(templineC1, "$");
				} else if (convMorT->isCliticChr == '~') {
					strcat(templineC1, "~");
					strcat(templineC1, item);
				} else {
					len = strlen(templineC1);
					if (templineC1[0] != EOS && templineC1[len-1] != '$')
						strcat(templineC1, " ");
					strcat(templineC1, item);
				}
//				convMorT == NULL;
//				fprintf(out, "*** POST INTERNAL ERROR 2\n");
//				break;
			}
		}
		convMorT = convMorT->nextW;
	}
	if (convMorT != NULL) {
		fprintf(stderr, "*** POST INTERNAL ERROR 2\n");
//		fprintf(out, "*** POST INTERNAL ERROR 2\n");
	}	
	printclean(out, "%mor:", templineC1);
	printclean(out, "%cnl:", mor);
}

int post_file(FNType* inputname, int style, int style_unkwords, int brill, FNType* outfilename, int filter_out, int linelimit, char* argv[], int argc, int internal_mor, int priority_freq_local )
{
	int res;
	FNType fno[FNSize];
	long32 postLineno = 0L, tSpLineno;
	char isConllError;
	struct SimpleTier *convMorT = NULL;
	FILE* out = stdout;

	isConllError = FALSE;
	res = strcmp(outfilename, "con");
//	res = uS.FNTypecmp(outfilename, "con", 0L);  // CHK
	if ( res ) {	// if not output to console.
		if ( outfilename[0] == '\0' ) {
			strcpy( fno, inputname );
			FNType* p = strrchr( fno, '.' );
			if (!p) {
				strcat(fno, ".pst");
//				uS.str2FNType(fno, strlen(fno), ".pst");  // CHK
				p = strrchr( fno, '.' );
			} else {
				strcpy(p, ".pst");
//				uS.str2FNType(p, 0L, ".pst");  // CHK
			}
			int n = 0;
#ifdef POSTCODE
			while ( access( fno, 00 ) == 0 )
#elif defined(UNX)
			while ( access( fno, 00 ) == 0 )
#else
			while ( _access( fno, 00 ) != -1 )
#endif
			{
				sprintf( p, ".ps%d", n );
				n++;
				if ( n==1000 ) {
					msg ( "cannot generate a file using the inputfile %s\n", outfilename );
					return 0;
				}
			}
			strcat(fno, ".cex");
//			uS.str2FNType(fno, strlen(fno), ".cex");  // CHK
		} else {
			strcpy(fno, outfilename);
		}
		out = fopen( fno, "w" );
		if (!out) {
			msg ( "the file \"%s\" cannot be opened or created\n", fno );
			return 0;
		}
#ifdef POSTCODE
#ifdef _MAC_CODE
	settyp(fno, 'TEXT', the_file_creator.out, FALSE);
#endif
#endif
#ifdef POSTCODE
		if ((option_flags[CLAN_PROG_NUM] & FR_OPTION) && replaceFile) {
			msg( "From file <%s>\n", inputname );
		} else
			msg( "From file <%s>\n", inputname );
#else
		msg( "From file <%s>\n", inputname );
#endif
	} else
		msg( "From file <%s> to console\n", inputname );

	int id = init_input( inputname, "mor" );
	if (id<0) {
		msg ( "the file \"%s\" does not exist or is not available\n", inputname );
		return 0;
	}
	posErrFp = NULL;

	int n, y, ok_t_option = 0;
	char* T;
	spT[0] = '\0';
	posT[0] = '\0';
	morT[0] = '\0';
	trnT[0] = '\0';
	spLineno = 0L;
	tSpLineno = 0L;
	rightSpTier = 0;

	// test for selection of a different input (and output?) option through +t%... option
	int domoralt=-1, i;
	char moralt[9];
	moralt[0] = 0;
	for (i=0;i<argc;i++) {
		if ( !_strnicmp( argv[i], "+t%", 3) ) {
			domoralt = i;
			if ( strlen(argv[i]) < 9 ) {
				strcpy( moralt, argv[i]+2 );
			} else {
				strncpy( moralt, argv[i]+2, 8 );
				moralt[8] = '\0';
			}
		}
	}
	while ( (n = get_tier(id, T, y, postLineno)) != 0 ) {
		if ( domoralt != -1 && y == TTdependant ) { // process one line provided by alternate option
			if ( moralt[0] != 0 && !_strnicmp( T, moralt, strlen(moralt) ) && ok_t_option == 1 && internal_mor & InternalNone ) {
				int jmpret = setjmp( mark );
				if( jmpret == 0 ) {
					convMorT = NULL;
					if (isConllSpecified() && style == 0) {
						convMorT = convertTier(T, inputname, &isConllError);
						if (isConllError)
							return 0;
					}
					strcpy(morT, T);
#ifdef debug_0
					msg ( "T=%s", T );
#endif
					post_analyzemorline( T, style, style_unkwords, brill, out, filter_out, linelimit, priority_freq_local );
					if (style == 0) {
						if (convMorT != NULL) {
							convertToMORAndPrint(out, convMorT, morT);
						} else {
							printclean(out, NULL, morT);
						}
					}
					freeConvertTier(convMorT);
					convMorT = NULL;
				} else {
					strcpy(morT, T);
					msgfile( out, "not enough memory for the following line :\n %s", T );
					dynreset();
				}
			} else {
				spLineno += lineCnt(T);
				msgfile( out, "%s", T );
				dynreset();
			}
		} else if ( domoralt == -1 && y == TTmor ) {  // process only %mor lines
			if ( !_strnicmp( T, "%mor", 4 ) && ok_t_option == 1  && internal_mor & InternalNone ) {
				int jmpret = setjmp( mark );
				if( jmpret == 0 ) {
					convMorT = NULL;
					if (isConllSpecified() && style == 0) {
						convMorT = convertTier(T, inputname, &isConllError);
						if (isConllError)
							return 0;
					}
					strcpy(morT, T);
#ifdef debug_0
	msg ( "T=%s", T );
#endif
					post_analyzemorline( T, style, style_unkwords, brill, out, filter_out, linelimit, priority_freq_local);
					if (style == 0) {
						if (convMorT != NULL) {
							convertToMORAndPrint(out, convMorT, morT);
						} else {
							printclean(out, NULL, morT);
						}
					}
					freeConvertTier(convMorT);
					convMorT = NULL;
				} else {
					strcpy(morT, T);
					msgfile( out, "not enough memory for the following line :\n %s", T );
					dynreset();
				}
			} else {
				spLineno += lineCnt(T);
				msgfile( out, "%s", T );
				dynreset();
			}
		} else if ( y != TTgeneral && !_strnicmp( T, "%pos", 4 ) && (style==1||style==2||style==3) ) {
			dynreset();
		} else if (y == TTcnl) {
		} else {
			if (y == TTgeneral) {
			} else if (*T == '*') {
				// test for users selected through -t and +t options.
				ok_t_option = 1;
				for (i=0;i<argc;i++) {
					if ( !_strnicmp( argv[i], "+t*", 3) ) {
						ok_t_option = 0;
					} else if ( !_strnicmp( argv[i], "-t*", 3) ) {
						ok_t_option = 1;
					} 
				}
				for (i=0;i<argc;i++) {
					if ( !_strnicmp( argv[i], "+t*", 3) ) {
						if ( !_strnicmp( T, argv[i]+2, strlen(argv[i]+2) ) ) 
							ok_t_option = 1;
					} else if ( !_strnicmp( argv[i], "-t*", 3) ) {
						if ( !_strnicmp( T, argv[i]+2, strlen(argv[i]+2) ) ) 
							ok_t_option = 0;
					} 
				}
				if (ok_t_option==0) {
					// do not analyse;
					// msgfile( out, "---should be ignored\n" );
					goto ignore_t_option;
				}
				if (spT[0] != '\0') {
#ifdef POSTCODE
					if ((option_flags[CLAN_PROG_NUM] & FR_OPTION) && replaceFile) {
						doCompar(inputname, tSpLineno);
					} else
						doCompar(fno, tSpLineno);
#else
					doCompar(fno, tSpLineno);
#endif
				}
				posT[0] = '\0';
				morT[0] = '\0';
				trnT[0] = '\0';
				strcpy(spT, T);
				tSpLineno = spLineno;
			} else if ( !_strnicmp( T, "%pos", 4 )) {
				strcpy(posT, T);
			} else if ( !_strnicmp( T, "%trn", 4 )) {
				strcpy(trnT, T);
			}
ignore_t_option:
			spLineno += lineCnt(T);
			msgfile( out, "%s", T );
			dynreset();
#ifdef INTERNAL
			if ( y == TTname && !(internal_mor&InternalNone) && ok_t_option == 1 ) {
				// msgfile( out, "---ignore mor line and use internal morphology information\n" );
				int jmpret = setjmp( mark );
				if ( internal_mor & InternalCompound ) {
					filter_with_compounds( T, trnT );
					strcpy( T, trnT );
					char* p = strchr( trnT, ':' );
					if (!p) {
						msgfile( out, "%s\n", "%cpn: --missing : on line above" );
					} else {
						int l = p-trnT;
						msgfile ( out, "%%cpn%s\n", trnT+l );
					}
				}
				compute_morline( T, morT, 1 /*language_style*/, internal_mor /*output_style*/, linelimit );
				// msg( "%s\n", spT);
				// msg( "%s\n", morT);
				strcpy(trnT, morT);
				if( jmpret == 0 ) {
					post_analyzemorline( trnT, style, style_unkwords, brill, out, filter_out, linelimit, priority_freq_local );
				} else {
					msgfile( out, "not enough memory for the following line :\n %s", T );
					dynreset();
				}
			}
#endif
		}
	}
	close_input(id);
	if (posErrFp != NULL)
		fclose(posErrFp);
	res = strcmp(outfilename, "con");
//	res = uS.FNTypecmp(outfilename, "con", 0L);
	if ( res )	{// if not output to console.
		fclose(out);
#ifdef POSTCODE
		if ((option_flags[CLAN_PROG_NUM] & FR_OPTION) && replaceFile) {
			unlink(inputname);
			rename_each_file(fno, inputname, FALSE);
		}
#else
		unlink(inputname);
		rename(fno, inputname);
#endif
	}
#ifdef POSTCODE
	if ((option_flags[CLAN_PROG_NUM] & FR_OPTION) && replaceFile) {
// lxs 2019-03-15 if (!isRecursive)
			msg( "Output file <%s>\n", inputname );
	} else
		msg( "Output file <%s>\n", fno );
#else
	msg( "Output file <%s>\n", inputname );
#endif
	return 1;
}

//;-------------------------------------------------------------------------------------------;
//; make ambtags from morambtags
//;-------------------------------------------------------------------------------------------;
static int get_ambtags_of_MOR( char** W, int nW, char** &words, AMBTAG* &ambs, ambmortag* &AMT, int* annotation, int* &taglocalwords )
{
	int i;  /* lxs new fix/addition */

	if (nW==0) return 0;	// no words.

	AMT = (ambmortag*)dynalloc( sizeof(ambmortag) * nW );
	taglocalwords = (int*)dynalloc( sizeof(int) * nW );
	for (i=0; i<nW; i++ ) taglocalwords[i] = -1;

	char *buf = dynalloc( maxSizeBA );	// array of ambiguous tags.
	ambs = (AMBTAG*)buf;			// array of ambiguous tags.
	int pa = nW * sizeof(AMBTAG);
	int* pC;	// multclass
	int C[MaxNbTags];
	words = (char**)dynalloc(sizeof(char*)*nW);

	for ( i=0; i<nW; i++ ) {

		char *tmp_s = dynalloc(strlen(W[i])+1);

		if (W[i][0] == '0' && strchr(W[i], '^') != NULL)//2019-05-01
			strcpy( tmp_s, W[i]+1 );
		else
			strcpy( tmp_s, W[i] );
		make_ambmortag( tmp_s, &AMT[i] );
#ifdef debug_7
	msg ( "(%s) (", W[i] );
	print_ambmortag( stdout, &AMT[i] );
	msg ( ")\n" );
#endif


		if ( !AMT[i].A[0].GR ) {
			words[i] = dynalloc(3);
			strcpy( words[i], "." );
		} else {
			words[i] = dynalloc( strlen(AMT[i].A[0].GR)+1 );
			strcpy( words[i], AMT[i].A[0].GR );
		}

		if ( is_unknown_word(&AMT[i].A[0]) ) {
#ifdef debug_0
	msg ( "unknown MT[%s] GR[%s]\n", AMT[i].A[0].MT?AMT[i].A[0].MT:"NULL", AMT[i].A[0].GR?AMT[i].A[0].GR:"NULL" );
#endif
			annotation [i] |= AnaNote_unknownword;
			pC = _words_classnames_default_ ;
		} else {
			ambmortag tmp;
			char *tmp_s = dynalloc(strlen(W[i])+1);
			
			if (W[i][0] == '0' && strchr(W[i], '^') != NULL)//2019-05-01
				strcpy( tmp_s, W[i]+1 );
			else
				strcpy( tmp_s, W[i] );
			make_ambmortag( tmp_s, &tmp );

#ifdef debug_7
	msg ( "!%s! !", W[i] );
	print_ambmortag( stdout, &tmp );
	msg ( "!\n" );
#endif

			// Local word categories information before suppressing word information.
			char WC[MaxSizeOfString];
			prepare_ambtagword( &tmp, WC );
			int t = get_local_word( WC );
			if (t != -1)
				taglocalwords[i] = t;

			filter_and_pack_ambmortag( &tmp );
			pC = ambmortag_to_multclass(&tmp, C);

#ifdef debug_7
	msg ( "{%s} {", W[i] );
	print_ambmortag( stdout, &tmp );
	msg ( "}\n" );
#endif
		}

		int s = (number_of_multclass(pC)+1) * sizeof(TAG);
		if ( pa + s > maxSizeBA ) return 0;
		multclass_to_ambtag( pC, AMBTAG(buf+pa) );
		ambs[i] = (AMBTAG)(buf+pa);
		pa += s;
	}
	return nW;
}

// this function creates a mortag in memory using the dynalloc allocation scheme
// the input is a result of the analysis process (simplified tags)
static int make_mortag_analyzed(char* str, mortag* mt)
{
	int i;

	mt->GR = 0;
	mt->SPt = 0;
	mt->SP = 0;
	mt->nSP = 0;

	// first suffixes and maintag
	char* sp[32];
	int ns = split_with( str, sp, MorTagComplementsMainTag, 32 );

	// extract maintag
	mt->MT = dynalloc( strlen(str)+1 );
	if ( !mt->MT ) return 0; // no memory.
	strcpy( mt->MT, str );
	ns--;

	// then find prefixes.
	char* pref[32];
//	int np = split_with(mt->MT, pref, MorTagComplementsPrefixes, 32 ); // note if np==1, then value of maintag is still valid
	int np = split_Prefixes(mt->MT, pref, 32 ); // note if np==1, then value of maintag is still valid

	if (ns==0&&np==1) return 1; // no prefixes and suffixes

	mt->SP = (char**)dynalloc( (np-1+ns) * sizeof(char*) );
	if ( !mt->SP ) return 0; // no memory.

	int n = 0;
	if (np>1) {
		for (i=0;i<np-1;i++, n++) {
			mt->SP[n] = pref[i]; // allocation is already done for mt->MT
		}
		mt->MT = pref[i];
	}

	for ( int k=0; k<ns; k++, n++) {
		mt->SP[n] = dynalloc( strlen(sp[k+1])+1 );
		if ( !mt->SP[n] ) return 0; // no memory.
		strcpy( mt->SP[n], sp[k+1] );
	}
	mt->nSP = n;
	return 1;
}

static void sprint_mortag( char* s, mortag* mt )
{
	int k;

	s[0] = '\0';
	for ( k=0; k< mt->nSP; k++)
		if ( mt->SPt[k] == MorTagComplementsPrefix )
			sprintf( s+strlen(s), "%s#", mt->SP[k] );
	sprintf( s+strlen(s), "%s", mt->MT?mt->MT:"?" );
	if ( mt->GR ) sprintf( s+strlen(s), "|%s", mt->GR );
	for ( k=0; k< mt->nSP; k++)
		if ( mt->SPt[k] != MorTagComplementsPrefix )
			sprintf( s+strlen(s), "%c%s", mt->SPt[k], mt->SP[k] );
	if (mt->translation)
		sprintf( s+strlen(s), "=%s", mt->translation);
}

// finds the closest mortags (in an ambmortag) from a normal tag.
static void tag_to_classname_filtered_monocat(TAG T, ambmortag* amt, char* s, int filter_out)
{
	int i, in;

	if (!amt) { // lxs
		tag_to_classname(T,s);
		return;
	}

	char temp[512];
	tag_to_classname(T,temp);

	if ( !strcmp( temp, "pct" ) ) { // punctuation: direct computation of mortag.
		strcpy( s, amt->A[0].MT );
		return;
	}

	mortag mt;
	make_mortag_analyzed(temp,&mt);
#ifdef debug_9
	msg("ANA /%s/ ==> ", temp);
	print_mortag_2(stdout, &mt);
	print_ambmortag_2(stdout, amt);
	msg("\n");
#endif

	// gets all mortag from the original morphological decomposition of MOR.

	char* p[32];

	for (i=0,in=0;i<amt->nA;i++) {
#ifdef debug_90
		msg("ISIN  ", temp);
		print_mortag_2(stdout, &amt->A[i]);
		print_mortag_2(stdout, &mt);
		msg(" %d \n", is_in_plus_gr(&amt->A[i],&mt));
#endif

		if ( is_in_plus_gr(&amt->A[i],&mt) ) {

			// filter output if necessary.

			for ( int k=0; k<amt->A[i].nSP; k++ ) {
#ifdef debug_90
				msg("XX %d %d /%s/\n", i, k, amt->A[i].SP[k] );
#endif
				if ( amt->A[i].GR && !strcmp(amt->A[i].GR,amt->A[i].SP[k]) ) {
					// test if root is the same as the name of a suffix
					; // do nothing but strip the tag
				} else if ( filter_ok(amt->A[i].SP[k]) ) {
#ifdef debug_90
					msg("BELONG %d %d /%s/\n", i, k, amt->A[i].SP[k] );
#endif
					// SP[k] belongs to the training set. The whole tag should be kept only if it is in 'mt.SP'
					int keep = 0 ;
					for (int kk=0; kk<mt.nSP; kk++) {
#ifdef debug_90
						msg("TEST %d %s %d %d /%s/\n", kk, mt.SP[kk], i, k, amt->A[i].SP[k] );
#endif
						if ( !strcmp(mt.SP[kk], amt->A[i].SP[k] ) ) { // it is in : keep it
							keep = 1;
							break;
						}
					}
					if ( keep == 1 ) continue;
					// the mt->A[i] is not a correct tag for the analysis.
					goto skip;
				} else {
					if ( filter_out == KeepAll ) continue;
					if ( filter_out == Keep && filter_ok_out(amt->A[i].SP[k]) ) continue;
					if ( filter_out == ThrowAway && !filter_ok_out(amt->A[i].SP[k]) ) continue;
					// the case filter_out == ThrowAwayAll is the default here
				}
				// STRIP SP[k]
				for (int l=k; l<amt->A[i].nSP-1; l++) {
					amt->A[i].SP[l] = amt->A[i].SP[l+1];
					amt->A[i].SPt[l] = amt->A[i].SPt[l+1];
				}
				amt->A[i].nSP --;
				k--;
			}
			char ps[256];
			ps[0] = '\0';
			sprint_mortag( ps, &amt->A[i] );
			p[in] = dynalloc( strlen(ps)+1 );
			strcpy( p[in], ps );
#ifdef debug_90
			msg("BBB %d /%s/\n", in, p[in]);
#endif
			in++;
skip: ;
#ifdef debug_90
			msg("SKIP\n");
#endif
		}
		if (in==32) break;
	}

#ifdef debug_90
	msg("IN=%d\n",in);
#endif

	if (!in) {	// if not found any original morphological decomposition
		tag_to_classname(T,s);
		if ( amt->A[0].GR ) {
			char* p1 = strchr(s,'&');
			char* p2 = strchr(s,'-');
			if (p2&&p1) {
				if (p2<p1) p1 = p2;
			} else if (p2 && !p1)
				p1 = p2;
			if (p1) {
				char tmp[128];
				strcpy( tmp, p1);
				*p1 = '\0';
				strcat(s,"|");
				strcat(s,amt->A[0].GR);
				strcat(s,tmp);
			} else {
				strcat(s,"|");
				strcat(s,amt->A[0].GR);
			}
		}
	}
	else
		sort_and_pack_in_string( p, in, s, 256, "^" );
#ifdef debug_90
	msg("CCC /%s/\n", s);
#endif
}
/*
static int split_muticat( char* mcat, char** cats, int& ncats )
{
	// divide category into its parts
	ncats = split_with( mcat+strlen(MultiCategory)+1, cats, "/", MaxMultiCategory );
	if (ncats < 2) return 0;
	return 1;
}
*/
// finds the closest mortags (in an ambmortag) from a normal tag.
static void tag_to_classname_filtered_multicat(TAG T, ambmortag* amt, char* s, int filter_out)
{
	char temp[512];
	tag_to_classname(T,temp);
	// char* c = temp+strlen(MultiCategory)+1;
	int l = strlen(temp);
	for (int i=0;i<amt->nA;i++) {
		char* a = mortag_to_string(&amt->A[i]);
		if (a == 0)
			return;
		if ( !strncmp( temp, a, l ) && a[ l ] == '|' ) {
			// copy without the +
			// strcpy( s, &a[l+2] );
			char*t = s;
			char*f = a+l+2;
			while (*f) {
				if (*f != '\001') { // compound fix 2010-12-17 '+'
					*t++ = *f;
				}
				f++;
			}
			*t = '\0';
			if (amt->A[i].translation) {
				strcat(s, "=");
				strcat(s, amt->A[i].translation);
			}
			return;
		}
	}
	// ?? should not happen, do something to clean up
	tag_to_classname_filtered_monocat( T, amt, s, filter_out);
}

static void post_shiftright(char *st, int num) {
	register int i;

	for (i=strlen(st); i >= 0; i--)
		st[i+num] = st[i];
}

// finds the closest mortags (in an ambmortag) from a normal tag.
static void tag_to_classname_filtered(TAG T, ambmortag* amt, char* s, int filter_out)
{
	char temp[512];
	tag_to_classname(T,temp);
	/* here find and present compound categories */
	if ( !strncmp( MultiCategory, temp, strlen(MultiCategory) ) )
		tag_to_classname_filtered_multicat( T, amt, s, filter_out);
	else
		tag_to_classname_filtered_monocat( T, amt, s, filter_out);
}

static void display_tag_internal(int i, int lastMW, int* annotation, ambmortag* AMT, char** MW, AMBTAG* R, int filtered, int filter_out, char* tT,
	int& p, char &sq, int style, int style_unkwords, FILE *out)
{
	char s[MaxSzTag+2];

	s[0] = EOS;
	if (annotation[i]&AnaNote_unknownword ) /*( !strcmp( "?", AMT[i].A[0].MT ) && strchr( MW[i], '|' ) )*/ {
		if (style_unkwords==1) {
			tag_to_classname(R[i][1],s);
			strcat( s, MW[i]+1 );
		} else {
			strcpy( s, "?" );
			strcat( s, MW[i]+1 );
		}
#ifdef debug_1
		msg("case of unknown word %s (%d) {%s}\n", s, i, MW[i]+1 );
#endif
	} else {
		if (filtered)
			tag_to_classname_filtered(R[i][1],&AMT[i],s,filter_out);
		else
			tag_to_classname(R[i][1],s);
		if (MW[i][0] == '0' && strchr(MW[i], '^') != NULL) {//2019-05-01
			post_shiftright(s, 1);
			s[0] = '0';
		}
	}

#ifdef debug_9
		msg("------$%s$\n", s );
#endif
	// lxs 2019-01-08
	if (i == lastMW && (strcmp(MW[i], ".") == 0 || strcmp(MW[i], "!") == 0 || strcmp(MW[i], "?") == 0) && strcmp(MW[i], s))
		strcpy(s, MW[i]);
	// lxs 2019-01-08

	if (strchr(s, '['))
		sq = 1;
	if (strchr(s, ']'))
		sq = 0;

	if (style != 0)
		msgfile( out, "%s", s );
	strcat(tT, s);
	spLineno += lineCnt(s);
	p += strlen(s);
	if (style>=2 && style<5) {	/* display other solutions, simple version */
		for ( int j = 2; j < R[i][0] ; j++ ) {
			if ( R[i][j] != -1 ) {
				if (filtered)
					tag_to_classname_filtered(R[i][j],&AMT[i],s,filter_out);
				else
					tag_to_classname(R[i][j],s);
				if (MW[i][0] == '0' && strchr(MW[i], '^') != NULL) {//2019-05-01
					post_shiftright(s, 1);
					s[0] = '0';
				}
				msgfile( out, "%c%s", Separator1, s );
				sprintf(tT+strlen(tT), "%c%s", Separator1, s );
				spLineno += lineCnt(s);
				p += strlen(s) +1;
			}
		}
	}
	if (style>=3 && style<5) {
		if (annotation[i]&AnaNote_unknownword ) {
			msgfile( out, "%cUnk", Separator2 );
			sprintf(tT+strlen(tT), "%cUnk", Separator2 );
		}
		if (annotation[i]&AnaNote_norule ) {
			msgfile( out, "%cNoRule", Separator2 );
			sprintf(tT+strlen(tT), "%cNoRule", Separator2 );
		}
		if (annotation[i]&AnaNote_noresolution ) {
			msgfile( out, "%cNoPath", Separator2 );
			sprintf(tT+strlen(tT), "%cNoPath", Separator2 );
		}
	}
}

static void display_lastresults( FILE* out, int style, int style_unkwords, int linelimit, int* annotation, ambmortag* AMT, char** MW, int nMW, AMBTAG* R, int filter_out, int spLineno, int added_a_punctuation, int filtered, char* morCodeInfo )
{
	char *tT;
	char sq, first_on_line;
	// display best analyze only.
	char mh[6];
	int lastMW;

	if (style == 4)
		strcpy(mh,"%nob:");
	else if (style == 0 || style >= 5)
		strcpy(mh,"%mor:");
	else if (style == 1)
		strcpy(mh,"%pos:");
	else
		strcpy(mh,"%pst:");
	if (style != 0)
		msgfile( out, "%s\t", mh );
	if (style==1)
		tT = posT;
	else
		tT = morT;
	sprintf( tT, "%s\t", mh );

	if (style==4) style = 1;

	int i, p;
	int nowrap = 0;

	sq = 0;
	first_on_line = TRUE;
	lastMW = (added_a_punctuation ? nMW-1 : nMW);
	for (i=1,p=1; i < lastMW; i++) {
		if (p>linelimit && !sq && nowrap==0) {
			p = 0;
			if (style != 0)
				msgfile( out, "\n\t" );
			strcat(tT, "\n\t");
			spLineno++;
			first_on_line = TRUE;
		}
		nowrap = 0;

#ifdef debug_8
			char temp[256];
			tag_to_classname(R[i][1],temp);
			char temp2[256];
			tag_to_classname_filtered(R[i][1],&AMT[i],temp2,filter_out);
			msg( "%s --- %s\n", temp, temp2 );
#endif

		if (first_on_line == FALSE) {
			if (style != 0)
				msgfile( out, " " );
			strcat(tT, " ");
		} else
			first_on_line = FALSE;

		display_tag_internal( i, lastMW-1, annotation, AMT, MW, R, filtered, filter_out, tT, p, sq, style, style_unkwords, out);

		p++;
	}

	if ( morCodeInfo ) {
		if (style != 0)
			msgfile( out, "%s", morCodeInfo );
		strcat(tT, morCodeInfo);
	}
	if (style != 0)
		msgfile( out, "\n" );
	strcat(tT, "\n");
	spLineno++;
}

static int analyze_mortags ( char** MW, int nMW, int style, int style_unkwords, int brill, FILE* out, int filter_out, char* morCodeInfo, int linelimit, int priority_freq_local )
{
	/* initial version:
	   word classes computed by POST
	*/
	AMBTAG* A=0;	// input of analyze
	AMBTAG* R=0;	// result of analyze
	char** words=0;

	// add a punctuation to the end if there is not one.
	int added_a_punctuation = 0;
	char punct[2];
	if (style >= 6) {
		msgfile( out, "%%postdbg:\n" );
	}
	strcpy( punct, "." );
	if ( !strchrConst( PunctuationsAtEnd, MW[nMW-1][0] ) ) {
		added_a_punctuation = 1;
		MW[nMW] = punct;
		nMW++;
	}

	int *annotation = (int *)dynalloc ((nMW+2)*sizeof(int));	// store the kind of analysis performed for each word.
						// if word exists or not
						// if rule exists or not
						// if normal analysis can be performed or not
	memset( annotation, 0, (nMW+2)*sizeof(int) );

	// A is allocated by get_ambtags_of_words until the next call to analyze_algorithm.

	// make ambtags from ambmortags
	ambmortag* AMT=0;
	/*TAG*/ int* taglocalwords=0;
	get_ambtags_of_MOR( MW, nMW, words, A, AMT, annotation, taglocalwords );

#ifdef debug_0
	for (int ii=1; ii<nMW; ii++) {	// skip first word.
		msg( "%s == ", MW[ii] );
		print_ambmortag(stdout,&AMT[ii-1]);
		msg(" ");
		print_ambmortag(stdout,&AMT[ii]);
		if (annotation[ii]&AnaNote_unknownword ) msgfile( stdout, "@Unk" );
		if (annotation[ii]&AnaNote_norule ) msgfile( stdout, "@NoRule" );
		if (annotation[ii]&AnaNote_noresolution ) msgfile( stdout, "@NoPath" );
		msg("\n");
	}
	for (int ii=1; ii<nMW; ii++) {	// skip first word.
		print_ambtag( A[ii-1], stdout );
		msg(" ");
		print_ambtag( A[ii], stdout );
		msg("\n");
	}
#endif

	// R is allocated by analyze_algorithm until the next call to analyze_algorithm.
	analyze_algorithm( A, nMW, R, annotation, style, out );

	if (brill) {
		if (style == 4) {
			display_lastresults( out, 4, style_unkwords, linelimit, annotation, AMT, MW, nMW, R, filter_out, spLineno, added_a_punctuation, 0, morCodeInfo );
			style = 1;
		}

		int i;
		TAG* original = (TAG*)dynalloc(nMW*sizeof(TAG));
		TAG* newversion = (TAG*)dynalloc(nMW*sizeof(TAG));
		for (i=0; i<nMW; i++)
			original[i] = R[i][1];

		// spT contains the original line.
		// todo: split the original line and pass the original words to the analyzer. Uses the value of 'nW' to decide which splitting algorithm to use.
		char* OrigW[MaxNbWords];
		char* OrigLine = dynalloc(strlen(spT)*2);
		char* m_spT = dynalloc(strlen(spT) + 2 );
		strcpy( m_spT, spT );
		int l = strlen(m_spT)-1;
		while ( ( m_spT[l] == '\n' || m_spT[l] == '\r' || m_spT[l] == ' ' ) && l > 0 ) l--;
		if ( strchrConst( "?.!", m_spT[l] ) )
			m_spT[l+1] = '\0';
		else {
			m_spT[l+1] = ' ';
			m_spT[l+2] = '.';
			m_spT[l+3] = '\0';
		}
		if ( split_original_line( nMW, m_spT, OrigW, OrigLine, strlen(spT)*2 ) == nMW )	{ // if split original is OK, use real words.
			strcpy( OrigW[0], "." );
			process_with_brills_rules( OrigW, original, newversion, A, nMW, brill==2?FullAndSlow:0 ); // uses Brill's rules on a previously computed sentence.
		} else
			process_with_brills_rules( words, original, newversion, A, nMW, brill==2?FullAndSlow:0 ); // uses Brill's rules on a previously computed sentence.
		for (i=0; i<nMW; i++)
			R[i][1] = newversion[i];
	}

	if ( (nMW <= 3 || nMW <= priority_freq_local+2) && priority_freq_local>0 ) {
		// selects most problable tags for local words.
		for (int i=1; i<nMW; i++) {	// skip first word.
			if ( taglocalwords[i] != -1 ) {
				TAG t = A[i][taglocalwords[i]+1];
				for ( int k=1; k<R[i][0]; k++ ) {
					if ( R[i][k] == t ) {
						R[i][k] = R[i][1];
						R[i][1] = t;
						break;
					}
				}
			}
		}
	}

#ifdef debug_0
	for (int ii=0; ii<nMW; ii++) {	// skip first word.
		msg(" (%d)%d-",ii,R[ii][0]); for (int j=1;j<R[ii][0];j++) msg("%d-", R[ii][j] );
	} msg("\n");
#endif
	if (style == 5 || style == 6) {
		// final presentation of debug information (display internal data).
		msgfile( out, "%%postdbg:\tFinalCategory [AmbiguousTag(Complete)] [ResolvedTag] [BeforeResolution]\n" );
		for (int i=1; i<(added_a_punctuation?nMW-1:nMW); i++) {	// skip first word.
			char s[256];
			tag_to_classname_filtered(R[i][1],&AMT[i],s,filter_out);
			if (s[0] == '.' || s[0] == '!' || s[0] == '?')
				continue;
			msgfile( out, "\t%s \n", s );
			msgfile( out, "\tAMT[%d]=[", i);
			print_ambmortag( out, &AMT[i] );
			msgfile( out, "]\n\tR[%d][1]%d=[", i, R[i][1]);
			print_ambtag_regular( R[i], out, -1 );
			msgfile( out, "]\n\tA[%d]=[", i);
			print_ambtag_regular( A[i], out );
			char an[256];
			strcpy( an, "]\n" );
			if (annotation[i]&AnaNote_unknownword )
				strcat(an, "\t-unknown-word\n");
			if (annotation[i]&AnaNote_norule )
				strcat(an, "\t-no-rule\n");
			if (annotation[i]&AnaNote_noresolution )
				strcat(an, "\t-no-resolution\n");
			msgfile( out, "%s\n", an );
		}
//		msgfile( out, "\n" );
	}
	if (style==2 || style==3) {
		display_lastresults( out, style, style_unkwords, linelimit, annotation, AMT, MW, nMW, R, filter_out, spLineno, added_a_punctuation, 1, morCodeInfo );
		display_lastresults( out, 1, style_unkwords, linelimit, annotation, AMT, MW, nMW, R, filter_out, spLineno, added_a_punctuation, 1, morCodeInfo );
	} else if (style>=5) {
		display_lastresults( out, style, style_unkwords, linelimit, annotation, AMT, MW, nMW, R, filter_out, spLineno, added_a_punctuation, 1, morCodeInfo );
	} else
		display_lastresults( out, style, style_unkwords, linelimit, annotation, AMT, MW, nMW, R, filter_out, spLineno, added_a_punctuation, 1, morCodeInfo );
	return 1;
}

int post_init(FNType* dbname, int inmemory)
{
	FNType str[FNSize];
	if ( !open_all( fileresolve( dbname, str ), 0, inmemory ) ) {
		msg("cannot open \"%s\".\n***Please make sure that any file with \".db\" in \"mor lib\" folder is renamed to %s\n", dbname, dbname);
		return 0;
	}
	if ( ! open_rules_dic( "rules", 1 ) ) {
		msg ( "rules cannot be opened...\n" );
		return 0;
	}
	// initialize Matrix file.
	open_matrix_dic( "matrix", 1 ) ;
	open_localword_dic( "localwords", 1 ) ;
	int w = open_word_dic( "words", 1 );
	if (!open_brillword_dic( "brillwords", 1 )) return -1;
	if (!open_brillrule_dic( "brillrules", 1 )) return -1;

	if (!w) return 2; else return 1;
}

int post_word_init(FNType* dbname, int inmemory)
{
	if ( !open_all( dbname, 0, inmemory ) ) {
		msg("cannot open \"%s\".\n***Please make sure that any file with \".db\" in \"mor lib\" folder is renamed to %s\n", dbname, dbname);
		return 0;
	}
	int w = open_word_dic( "words", 1 );
	if (w) return 2; else return 1;
}

void post_analyzemorline(char* MLine, int style, int style_unkwords, int brill, FILE* out, int filter_out, int linelimit, int priority_freq_local)
{
	if (style != 0) {
		msgfile( out, "%s", MLine );
		spLineno += lineCnt(MLine);
	}

#ifdef debug_99
	msg( "Mline orinale <%s>\n", MLine );
#endif
	char* W[MaxNbWords];
	char* Line = dynalloc(strlen(MLine)*2);

	// clean the line of new element such as [xx]
	char* cleanedLine = dynalloc( strlen(MLine)+1 );
	char* morCodeInfo = dynalloc( morCodeInfoLength+1 );
	strcpy( cleanedLine, MLine );
	strip_of_one_bracket( cleanedLine, morCodeInfo, morCodeInfoLength ); // for post codes at the end of sentence.

	// FIRST SPLIT LINE WITH ONLY WHITE SPACES AS DELIMITERS and normalize punctuation at the end of the sentence
	int nW = split_with_a( cleanedLine, W, Line, WhiteSpaces, PunctuationsAtEnd, PunctuationsInside, MaxNbWords, strlen(MLine)*2 );
	int i;

	// replace :mor. with '.' because first word must of a sentence must always be a '.'
	strcpy( W[0], "." );

	// strip punctuations which should not be present in MOR line
	for (i=0; i<nW; i++) {
		if ( strchrConst( non_internal_pcts, W[i][0] ) ) {
			for ( int j=i; j<nW-1; j++ )
				W[j] = W[j+1];
			nW --;
		}
	}

#ifdef debug_99
	msg( "W==>");
	for (i=0; i<nW; i++) {
		msg(" [%s]", W[i] );
	}
	msg( "\n");
#endif

	// HANDLES THE CASE OF TAG/GROUPS, WORD/GROUPS AND COMPOUND WORDS
	// Here it will be necessary to reconstruct the original splited words, so it is necessary to store the actual groups separators
	for (i=0; i<nW; i++) {
		char* st;
		if ( make_multi_categ( W[i], st ) ) {
			W[i] = st;
		}
	}

#ifdef debug_99
	msg( "W==>");
	for (i=0; i<nW; i++) {
		msg(" [%s]", W[i] );
	}
	msg( "\n");
#endif

	analyze_mortags ( W, nW, style, style_unkwords, brill, out, filter_out, morCodeInfo, linelimit, priority_freq_local );
	dynreset();
}
