/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- trainbrill.cpp	- v1.0.0 - november 1998
// syntactic training for Brill's rules
|=========================================================================================================================*/

#include "system.hpp"
#include "database.hpp"

int train_brill( FNType* name, FNType* output_for_brill, int BrillStyle, int brill_threshold )
{
	msg( "now training Brill's rules using file <%s>\n", name );
	char tmpfilename[FNSize];
	tmpnam(tmpfilename);
	int nlines = train_file(name, BrillsRules, 0, tmpfilename);
		// goes through the training set and produces an intermediate training dataset in file tmpfilename

	int res;
	FILE* outfile = stdout;
#if defined(UNX) || (!defined(POSTCODE) && !defined(MAC_CODE))
	res = strcmp(output_for_brill, "con");
#else
	res = uS.FNTypecmp(output_for_brill, "con", 0L);
#endif
	if ( res && output_for_brill[0] != '\0' ) {	// if not output to console.
		outfile = fopen( output_for_brill, "a" );
		if (!outfile) {
			msg ( "the file \"%s\" cannot be opened or created\n", output_for_brill );
			return 0;
		}
#ifdef POSTCODE
#ifdef _MAC_CODE
		settyp(output_for_brill, 'TEXT', the_file_creator.out, FALSE);
#endif /* _MAC_CODE */
#endif			
	}

	// open intermediate training file.
	int id = init_input( tmpfilename, "mor" );
	if (id<0) {
		msg ( "training file %s cannot be found1\n", tmpfilename);
		if (outfile != stdout)
			fclose(outfile);
		return 0;
	}
	
	int n, y, L=-1;
// 2006-02-20	char  pct_at_end = '\0';	// store a possible punctuation at the end of the sentence.
	char* T;
	long32 postLineno = 0L;
	char** temp_words = new char* [MaxNbWords];
	int nbWrd = 0;
	rightSpTier = 0;
	while ( (n = get_tier(id, T, y, postLineno)) != 0 ) {
		if ( y == TTwrd ) {
			L++;
			if (L>=nlines) {
				msg( "error - two many lines - not supposed to happen - stop processing at line %d\n", postLineno );
				break;
			}
			nbWrd += split_with( T, temp_words, WhiteSpaces, MaxNbWords );
			nbWrd += 3;
		}
	}
	close_input(id);
	id = init_input( tmpfilename, "mor" );
	if (id<0) {
		msg ( "training file %s cannot be reopened\n", tmpfilename);
		if (outfile != stdout)
			fclose(outfile);
		return 0;
	}
	nbWrd += 3;
	char** Wrd = new char* [nbWrd];
	char** Trn = new char* [nbWrd];
	char** Pos = new char* [nbWrd];

	L=-1;
	postLineno = 0L;
	rightSpTier = 0;
	int i, nw=0, nt=0, np=0;
	while ( (n = get_tier(id, T, y, postLineno)) != 0 ) {
		if ( y == TTwrd ) {
			for (i=0;i<3;i++) {
				Wrd[nw] = new char [strlen(staart_word)+1];
				strcpy( Wrd[nw], staart_word);
				nw++;
				Trn[nt] = new char [strlen(staart_tag)+1];
				strcpy( Trn[nt], staart_tag);
				nt++;
				Pos[np] = new char [strlen(staart_tag)+1];
				strcpy( Pos[np], staart_tag);
				np++;
			}
			L++;
			if (L>=nlines) {
				msg( "error - two many lines - not supposed to happen - stop processing at line %d\n", postLineno );
				break;
			}
			int n = split_with( T, temp_words, WhiteSpaces, MaxNbWords );
			for (i=0;i<n;i++) {
				Wrd[nw] = new char [strlen(temp_words[i])+1];
				strcpy( Wrd[nw], temp_words[i]);
				nw++;
				if (nw>=nbWrd ) break;
			}
		} else if ( y == TTtrn ) {
			int n = split_with( T, temp_words, WhiteSpaces, MaxNbWords );
			for (i=0;i<n;i++) {
				int x = atoi(temp_words[i]);
				char xx[128];
				tag_to_classname( x, xx );
				Trn[nt] = new char [strlen(xx)+1];
				strcpy( Trn[nt], xx);
				nt++;
			}
		} else if ( y == TTpos ) {
			int n = split_with( T, temp_words, WhiteSpaces, MaxNbWords );
			for (i=0;i<n;i++) {
				int x = atoi(temp_words[i]);
				char xx[128];
				tag_to_classname( x, xx );
				Pos[np] = new char [strlen(xx)+1];
				strcpy( Pos[np], xx);
				np++;
			}
		} else {
			msg( "unknow tier - line is >>> %s", T );
		}
		if (nt>=nbWrd || np>=nbWrd || nw>=nbWrd ) break;
	}
	close_input(id);
	unlink(tmpfilename);
	if ( nw != nt || nw != np || nw != nbWrd-3 ) {
		msg( "error on file numbers %d %d %d %d\n", nw, nt, np, nbWrd );
		if (outfile != stdout)
			fclose(outfile);
		return 0;
	}

	for (i=0;i<3;i++) {
		Wrd[nw] = new char [strlen(staart_word)+1];
		strcpy( Wrd[nw], staart_word);
		nw++;
		Trn[nt] = new char [strlen(staart_tag)+1];
		strcpy( Trn[nt], staart_tag);
		nt++;
		Pos[np] = new char [strlen(staart_tag)+1];
		strcpy( Pos[np], staart_tag);
		np++;
	}
/***
	for (i=0;i<nbWrd;i++) {
		msg("%s %s %s\n", Wrd[i], Pos[i], Trn[i] );
	}
***/
	/** compute in memory **/

	contextrl( Trn, Wrd, Pos, nbWrd, outfile, BrillStyle, brill_threshold );

	if (outfile != stdout)
		fclose(outfile);

	save_all();

	// free memory
	for (L=0;L<nbWrd;L++) {
		delete [] Wrd[L];
		delete [] Trn[L];
		delete [] Pos[L];
	}
	delete [] Wrd;
	delete [] Trn;
	delete [] Pos;
	return 1;
}
