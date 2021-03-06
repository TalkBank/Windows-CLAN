/****************************************************************************
 *																			*
 *	BuildTree.cpp															*
 *																			*
 *	int BuildTree( CTreeCtrl& tree, CString& origstr )						*
 *		+ tree: the tree control where the f-struct will be displayed.		*
 *		+ origstr: a string containing the f-struct in LISP format.			*
 *				   (contents of the %syn line)								*
 *																			*
 *		- returns: barely useful error codes.								*
 *																			*
 ****************************************************************************/
  
#include "ced.h"
#include "FStructViewer.h"

#define MAX_DEPTH 1024
//#define __PRINTERRORMSGS

// prints error messages
void PrintError( char *str1, char *str2 )
{
#ifdef __PRINTERRORMSGS
	MessageBox( NULL, str1, str2, NULL );
#endif
}

// Expands (or collapses) every branch in the tree, recursively (of course :-)
void ExpandTree( CTreeCtrl* tree, HTREEITEM item,  UINT code ) {

	// base case
	if( !item )
		return;

	// go deep
	if( tree->ItemHasChildren( item ) ) {
		ExpandTree( tree, tree->GetNextItem( item, TVGN_CHILD ), code );
	}
	
	// action
	tree->Expand( item, code );
	
	// go wide
	ExpandTree( tree, tree->GetNextItem( item, TVGN_NEXT ), code );
}

// Preprocessor for the string input to BuildTree().
// Changes the structure of *MULTIPLE* and *OR* blocks from the
// original LCFlex format. 
int PreprocStr( CString& theStr ) 
{
	int i, j, cnt;

	// go through the entire string
	for( i = 0; i < theStr.GetLength(); i++ ) {

		if( ( theStr.Mid( i, 10 ) ) == "*MULTIPLE*" ) {
			// found a *multiple* block

			theStr = theStr.Left( i ) + "(" + 
					theStr.Right( theStr.GetLength() - i );
			i++;

			// go through the string and insert 
			cnt = 1;
			j = i;
			while( ( cnt > 0 ) && ( j < theStr.GetLength() ) ) {
				
				// starting a block?
				if( theStr[j] == '(' ) {
					cnt++;
				}

				// closing a block?
				if( theStr[j] == ')' ) {
					cnt--;

					// closing the entire *MULTIPLE* sub-block?
					if( cnt == 1 ) {

						// if it is the last *MULTIPLE* sub-block, add ")"
						if( theStr[ j + 1 ] == ')' ) {
							theStr = theStr.Left( j + 1 ) + ")" +
								theStr.Right( theStr.GetLength() - j - 1 );
							i += 1;
							j += 1;
						}
						// if it is not the last, add ")(*multiple* "
						else {
							theStr = theStr.Left( j + 1 ) + ")(*multiple* " +
								theStr.Right( theStr.GetLength() - j - 1 );
							i += 13;
							j += 12;
						}
					}
				}
				j++;
			}
		}

	/* THE *OR* BLOCK GOES HERE */
	}

	return 0;
}

// Build an f-structure tree from a string containing the lisp format from LCFlex
// returns	0 : fine
//		   -1 : error (fstructure was malformed).
int BuildTree( CTreeCtrl& tree, CString& origstr )
{
	int i, level, wstate;
	CString tmpstr, str;
	HTREEITEM tmpitem[MAX_DEPTH];

	memset( tmpitem, 0, sizeof( HTREEITEM ) * MAX_DEPTH );

	level = 0;
	tmpitem[1] = TVI_ROOT;

	if( origstr.IsEmpty() )
		return 0;

	// the first thing to do is fix all the white space.
	// we take all \n, \t, \13, and \32 and convert them to a single \32,
	// and remove all \n, \13, \t and \32 from in between parens.
	wstate = 0;
	for( i = 0; i < origstr.GetLength(); i++ ) {
		if( wstate ) {
			// skip white space
			if( ( origstr[i] == '\n' ) || ( origstr[i] == '\t' ) ||
				( origstr[i] == 13 ) || ( origstr[i] == ' ' ) ) {
				continue;
			}
			else {
				wstate = 0;
				if( origstr[i] == ')' ) {
					str.TrimRight();
				}
			}
		}
		
		// are we starting white space?
		if( ( origstr[i] == '\n' ) || ( origstr[i] == '\t' ) ||
			( origstr[i] == 13 ) || ( origstr[i] == ' ' ) ) {
			wstate = 1;
			if( i && ( ( origstr[i - 1] == '(' ) || ( origstr[i - 1] == ')' ) ) ) {
				continue;
			}
			str += ' ';
			continue;
		}

		str += origstr[i];
	}

	str.TrimLeft();
	str.TrimRight();

	PreprocStr( str ); 

	// the first character must be an open-paren
	if( str[0] != '(' ) {
		PrintError( "F-Structure must be in LISP format, as generated by LCFlex.", "Syntax error" );
		return -1;
	}

	// go through the entire string
	for( i = 1; i < str.GetLength(); i++ ) {

		// if we see an open-paren, go one level deeper
		if( str[i] == '(' ) {
			level++;
			continue;
		}

		// if we see a close-paren, finish current item, go one level up
		if( str[i] == ')' ) {
			
			// at this point we should have a string for the value
			if( tmpstr.IsEmpty() && ( str[i - 1] != ')' ) ) {
				PrintError( "Value expected.", "Syntax error" );
				return -1;
			}

			if( str[i - 1] == ')' ) {
				level--;
				continue;
			}

			// insert the new item
			tmpstr.MakeLower();
			tmpstr.SetAt( 0, toupper( tmpstr[0] ) );
			tree.InsertItem( tmpstr, tmpitem[level] );
			tmpstr = "";
			level--;
			continue;
		}

		// if we see a space, we just read a label
		if( str[i] == ' ' ) {

			// at this point we should have a string for the label
			if( tmpstr.IsEmpty() ) {
				if( str[i - 1] == ')' ) 
					continue;
				PrintError( "Label expected.", "Syntax error" );
				return -1;
			}

			// check if the value corresponding to this label is simple or complex
			if( str[i + 1] == '(' ) {
				// complex.  insert label now, value will be a child
				tmpstr.MakeLower();
				tmpstr.SetAt( 0, toupper( tmpstr[0] ) );
				if( !tmpitem[level] && tmpitem[level + 1] ) {
					level++;
				}
				tmpitem[level + 2] = tree.InsertItem( tmpstr, tmpitem[level] );
				tmpstr = "";
			}
			else {
				// simple. value corresponding to this label will follow immediatelly.
				tmpstr += ": ";
			}

			continue;
		}

		// remove * from the root values (not necessary, makes it prettier)
		if( str[i] == '*' ) 
			continue;

		tmpstr = tmpstr + str[i];
	}
	
	return 0;
}


/* THIS IS THE *OR* BLOCK */
/* BUGGY, DO NOT USE      */

		/*
		if( ( theStr.Mid( i, 4 ) ) == "*OR*" ) {
			// found an *or* block

			theStr = theStr.Left( i ) + "(" + 
					theStr.Right( theStr.GetLength() - i );
			i++;

			// go through the string and insert 
			cnt = 1;
			j = i;
			while( ( cnt > 0 ) && ( j < theStr.GetLength() ) ) {
				
				// starting a block?
				if( theStr[j] == '(' ) {
					cnt++;
				}


				// closing a block?
				if( theStr[j] == ')' ) {
					cnt--;

					// closing the entire *OR* sub-block?
					if( cnt == 1 ) {

						// if it is the last *OR* sub-block, add ")"
						if( theStr[ j + 1 ] == ')' ) {
							theStr = theStr.Left( j + 1 ) + ")" +
								theStr.Right( theStr.GetLength() - j - 1 );
							i += 1;
							j += 1;
						}
						// if it is not the last, add ")(*or* "
						else {
							theStr = theStr.Left( j + 1 ) + ")(*or* " +
								theStr.Right( theStr.GetLength() - j - 1 );
							i += 7;
							j += 6;
						}
					}

					if( cnt == 0 ) {
						theStr = theStr.Left( j + 1 ) + ") " +
							theStr.Right( theStr.GetLength() - j - 1 );	
					}
				}
				j++;
			}
		}*/