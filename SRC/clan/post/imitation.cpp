/**********************************************************************
	"Copyright 1990-2020 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// imitation.cpp
// 

#if defined(_MAC_CODE)
	#include "0global.h"
#endif
#include <assert.h>
#include <ctype.h>
#include "system.hpp"
#include "database.hpp"

const int maxofsentences = 1000000;

char **AT=0;	// All the sentences
int  nAT=0;	// number of sentences
char ***AW=0;	// All the words
int  *nAW=0;	// number of words
int  precision = 0;	// exact or fuzzy matching (for multi-words).
int  morpho = 0;	// split morphologic markers or no.

void nomemory()
{
	msg("no more memory available\n");
	exit(-2);
}

int isinlast_together( char** W, int nW, char** A, int nA)
{
	// find first W in A
	int i;
	for ( i=0; i<nA-nW+1 ; i++ ) {
		if ( !strcmp( W[0], A[i] ) ) break; // found the first W
	}
	if ( i==nA-nW+1 ) return 0;	// not found the first W
	for ( int k=1; k<nW ; k++ )
		if ( strcmp( W[k], A[i+k] ) ) return 0; // not found a word of W
	return 1;
}

int isinlast( char** W, int nW, char** A, int nA)
{
	// find first W in A
	for ( int w=0, a=0; w<nW; w++ ) {
		int i;
		for ( i=a; i<nA-(nW-w)+1 ; i++ ) {
			if ( !strcmp( W[w], A[i] ) ) break; // found W[w]
		}
		if ( i==nA-(nW-w)+1 ) return 0;	// not found W[w]
	}
	return 1;
}

int isin( char** W, int nW, char** A, int nA, int style=0)
{
	if (nW > nA) return 0;	// whatever the speaker, this is always true.
	if (nW < 2) return 0;	// whatever the speaker, this is always true.
	int c = strcmp( W[0], A[0] );
	if ( style == 2 || style == -2 ) {	// ask for repetition
		if ( c ) // different
			return 0;
	} else if ( !c ) {
		// same speaker : it cannot be an imitation
		return 0;
	}
	// same speaker if repetition, different speaker if imitation
	return precision==1 ? isinlast_together(W+1,nW-1,A+1,nA-1) : isinlast(W+1,nW-1,A+1,nA-1);
}

int issame( char** W, int nW, char** A, int nA, int style=0)
{
	if (nW != nA) return 0;	// whatever the speaker, this is always true.
	if (nW < 2) return 0;	// whatever the speaker, this is always true.
	int c = strcmp( W[0], A[0] );
	if ( style == 2 || style == -2 ) {	// ask for repetition
		if ( c ) // different
			return 0;
	} else if ( !c ) {
		// same speaker : it cannot be an imitation
		return 0;
	}
	// same speaker if repetition, different speaker if imitation
	for ( int k=1; k<nW ; k++ )
		if ( strcmp( W[k], A[k] ) ) return 0; // not found a word of W
	return 1;
}

void store_sentence( char* nom, int yesnom, char* T, int style, int up, int gap )
{
	if ( nAT==maxofsentences ) {
		msg("max number of sentences reached\n");
		exit(-2);
	}

	int nT = strlen(T);
	AT[nAT] = new char [nT+1];
	memcpy( AT[nAT], T, nT+1 );

	char* W[4096];
	int nW = split_with( AT[nAT], W, WhiteSpaces, 4096 );

	int startpoint = 0;
// printf ("gap=%d\n", gap);
	if (gap>0) {
		startpoint = nAT;
		while (gap>0 && startpoint>0) {
			startpoint--;
			if ( (style==1||style==0) && strcmp( W[0], AW[startpoint][0] ) || (style==2||style==-2) && !strcmp( W[0], AW[startpoint][0] ) )
				// not same speaker
				gap --;
		} 
	}
// printf ("gap=%d %d %d\n", gap, startpoint, nAT);
	int samenom = strnicmp( nom, W[0]+1, strlen(nom)) ;
	if ( (yesnom && !samenom) || (!yesnom && samenom) ) {
		if ( style == 3 ) { // filter only
			for (int k=0; k<nW-1 ; k++)
				msg( "%s ", W[k] );
			msg( "%s\n", W[nW-1] );
		} else if ( style == 1 || style == 2 ) { // style == 1 (imitation) or 2 (repetition) - POSITIVE VERIFICATION
			for ( int i=startpoint; i<nAT ; i++ ) {
				int t;
				if (up==2)
					t = issame(AW[i],nAW[i],W,nW,style);
				else if (up==3)
					t= (isin(W,nW,AW[i],nAW[i],style) || isin(AW[i],nAW[i],W,nW,style)) ? 1 : 0;
				else
					t=up==1?isin(W,nW,AW[i],nAW[i],style):isin(AW[i],nAW[i],W,nW,style);
				if (t==1) {
					for (int k=0; k<nW-1 ; k++)
						msg( "%s ", W[k] );
					msg( "%s\n", W[nW-1] );
					break;
				}
			}
		} else { // style == 0 (no imitation) or -2 (no repetition) - NEGATIVE VERIFICATION
			int i;
			for ( i=startpoint; i<nAT ; i++ ) {
				int t;
				if (up==2)
					t = issame(AW[i],nAW[i],W,nW,style);
				else if (up==3)
					t= (isin(W,nW,AW[i],nAW[i],style) || isin(AW[i],nAW[i],W,nW,style)) ? 1 : 0;
				else
					t=up==1?isin(W,nW,AW[i],nAW[i],style):isin(AW[i],nAW[i],W,nW,style);
				if (t==1) 
					break;
			}
			if (i==nAT) {
				for (int k=0; k<nW-1 ; k++)
					msg( "%s ", W[k] );
				msg( "%s\n", W[nW-1] );
			}
		}
	}

	AW[nAT] = new char* [nW];
	nAW[nAT] = nW;
	if (!AT[nAT] || !AW[nAT]) nomemory();
	memcpy( AW[nAT], W, nW*sizeof(char*) );

	nAT++;
}

#ifdef notdefined_stripchatcode
void stripchatcode(char* s, char* a)
{
	a[0] = '\0';
	int na = 0;
	int cp = 1;
	for ( int i = 0 ; i < strlen(s) ; i++ ) {
		char x = s[i];
		if ( x == '<' || x == '>' || x == ',' || x == ';' ) continue;
		if ( x == '#' || x == '@' ) {
			while ( s[i] && !strchr(" \t\r\n", s[i]) ) i++;
			continue;
		}
		if ( x == '[' )  { cp --; continue; }
		if ( x == ']' )  { cp ++; continue; }
		if ( cp<1 ) continue;
/*
		if ( strchr (".?!", x ) ) {
			a[na++] = ' ';
			a[na++] = x;
		} else
			a[na++] = x;
*/
		if ( strchr (".?!", x ) ) continue;	// strip silent code
		if ( morpho && x == '-' ) x = ' ';	// strip morphologic markers.
		a[na++] = x;
	}
	a[na++] = '\0';
}
#endif

void imitation( char* filename, char* nom, int yesnom, int style, int up, int gap )
{
	int id = init_input( filename, "mor" );
	if (id<0) {
		msg( "file %s not found\n", filename );
		return;
	}

	long lineno = 0L;
	int n, y; char* T;
	while ( n = get_tier(id, T, y, lineno) ) {
		if ( y == TTname ) {

			// cleans T from all special characters of CHAT.
			char A[4096];
			stripchatcode(T,A);

			store_sentence ( nom, yesnom, A, style, up, gap );
		}
	}
	close_input(id);
}


int main(int argc, char** argv)
{
	int style = 0, up = 0, gap = 0, yesnom = 1, i;
	char nom[256];
	nom[0] = '\0';

	if ( argc < 2 ) {
  usage:
		msg ("Usage: IMITATION filename [{+-}n'nom' {+-}i {+-}r +f +{-+ea} +p +m +g'number']\n");
		return -1;		
	}
	for ( i=1;i<argc;i++ ) {
		if ( strchr("+-/",argv[i][0]) ) {
			switch( tolower(argv[i][1]) ) {
				case 'i' :
					style = 1;
					if ( argv[i][0] == '-' ) style = 0;
				break;
				case 'r' :
					style = 2;
					if ( argv[i][0] == '-' ) style = -2;
				break;
				case 'f' :
					style = 3;
				break;
				case 'e' :
					up = 2;
				break;
				case 'a' :
					up = 3;
				break;
				case '-' :
					up = 1;
				break;
				case 'p' :
					precision = 1;
				break;
				case 'g' :
					gap = atoi( argv[i]+2 );
					if (gap<0) {
						msg ( "bad parameter for +g %s\n", argv[i] );
						goto usage;
					}
				break;
				case '+' :
					up = 0;
				break;
				case 'm' :
					morpho = 1 ;
				break;
				case 'n' :
					strcpy( nom, argv[i]+2 );
					if ( argv[i][0] == '-' ) yesnom = 0;
				break;
				default:
					msg ( "unknown parameter %s\n", argv[i] );
					goto usage;
				break;
			}
			argv[i][0] = '\0';	// strip the parameter (should not be used any more)
		}
	}

	AT = new char*  [maxofsentences];
	AW = new char** [maxofsentences];
	nAW = new int   [maxofsentences];
	if (!AT || !AW || !nAW) nomemory();

	// skip through all parameters to analyze the required files
	for ( i=1;i<argc;i++ ) {
		if (argv[i][0]!='\0') {
			imitation( argv[i], nom, yesnom, style, up, gap );
		}
	}
	return 0;
}
