/**********************************************************************
	"Copyright 1990-2020 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************************
** morpho.cpp
** creates a mor line in a CHAT format file using the dictionary included in POST data
***********************************************************************/

#include "system.hpp"
#include "tags.hpp"
#include "database.hpp"
#include "morpho.hpp"
#include "compound.hpp"
#include "graphon.h"

/* catfix.cut : file describing direct categorisation of words ending in @something
*/
char** List_Catfix = NULL;

void load_Catfix(char* filename)
{
	if (List_Catfix) {
		msg( "File %s (fixed categories) can be read only once\n", filename );
		return;
	}
	FILE* cf = fopen( filename, "r" );
	if (!cf) return;
	int n = 0, l = 0;
	char line[MAXSIZECATEGORY];
	while ( fgets(line, MAXSIZECATEGORY, cf) ) {
		l++;
		if ( line[0] == '@' || line[0] == '%' ) continue;	// comments and other commands
		char* ms[2];
		int nm = split_with( line, ms, "+- \t\r\n", 2 );
		if (nm == 0) continue;
		if (nm != 2) {
			msg( "Error in file %s line %d\n", filename, l );
			continue;
		}
		n ++;
	}
	rewind(cf);
	List_Catfix = new char*[n+1];
	n=0;
	while ( fgets(line, MAXSIZECATEGORY, cf) ) {
		if ( line[0] == '@' || line[0] == '%' ) continue;	// comments and other commands
		char* ms[2];
		int nm = split_with( line, ms, "+- \t\r\n", 2 );
		if (nm == 0) continue;
		if (nm != 2) continue;
		char pat[MAXSIZECATEGORY];
		if (ms[0][0] == '*')
			strcpy(pat, ms[0]+1 );
		else
			strcpy(pat, ms[0] );
		List_Catfix[n] = new char [strlen(pat)+strlen(ms[1])+2];
		strcpy( List_Catfix[n], pat);
		strcpy( List_Catfix[n] + strlen(pat)+1, ms[1] );
		n ++;
	}
	fclose(cf);
	List_Catfix[n] = (char*)0;
}

char* is_Catfix(char* w)
{
	char* p = strchr(w, '@');
	if (!p) return NULL;
	char** l = List_Catfix;
	if (!l) return NULL;
	while (*l) {
		if ( !strcmp( *l, p ) ) {
			return *l+strlen(*l)+1;
		}
		l++;
	}
	return NULL;
}

/** prints the morphology **/
void print_morphology(int nm, char** ms, char* out, int style, int newwords, int linelimit)
{
	if (nm>1) {
		if ( newwords == 1 )
			; // sprintf( out, "%%xl:" ); or StyleNewWordsPho
		else if ( style & StyleMorForm )
			sprintf( out, "%%xmform:" );
		else if ( style & StyleMorPho )
			sprintf( out, "%%xmpho:" );
		else if ( style & StyleMorFormPho )
			sprintf( out, "%%xmfpho:" );
		else if ( style & StyleXMor )
			sprintf( out, "%%xmor:" );
		else
			sprintf( out, "%%mor" );
		int lg = 1;
		for (int k=1; k<nm; k++) {
			if ( is_bullet(ms[k]) || ms[k][0] == '[' || (ms[k][0] == '(' && ms[k][0] == '.' ) ) continue;
			char w[MAXSIZECATEGORY];
			char sn[MAXSIZEWORD], *s;
			if ( sizeof(sn) >= strlen(ms[k]) ) {
				stripof(ms[k],sn);
				s = sn;
			} else
				s = ms[k];
			int nl = lg == 0 ? 1 : 0;
			if ( !is_empty(s) && !is_chat_punctuation(s) ) {
				if ( s[0]>='A' && s[0]<='Z' && newwords==0) {
					lg += strlen(s) + 7;
					if (style&(StyleNewWordsPho|StyleMorFormPho|StyleMorPho)) {
						char p[MAXSIZEWORD];
						phonetize(s,p);
						if (style&StyleMorFormPho)
							sprintf( out+strlen(out), "%sn:prop|%s{%s}(%s)", (k==1||nl==1)?"\t":" ", s, s, p );
						else
							sprintf( out+strlen(out), "%sn:prop|%s(%s)", (k==1||nl==1)?"\t":" ", s, p );
					} else
						if (style&StyleMorForm)
							sprintf( out+strlen(out), "%sn:prop|%s{%s}", (k==1||nl==1)?"\t":" ", s, s );
						else
							sprintf( out+strlen(out), "%sn:prop|%s", (k==1||nl==1)?"\t":" ", s );
					if (lg>=linelimit) {
						sprintf( out+strlen(out), "\n" );
						lg = 0;
					}
					continue;
				}
				char* c = is_Catfix( s );
				//msg( "catfix: !%s!%s!\n", s, c );
				if (c) {
					if (newwords==0) {
						char* w = strchr( s, '@' );
						if (w) {
							*w='\0';
						}
						lg += strlen(s) + 3;
						if (style&(StyleMorPho|StyleMorFormPho)) {
							char p[MAXSIZEWORD];
							phonetize(s,p);
							if (style&StyleMorFormPho)
								sprintf( out+strlen(out), "%s%s|%s{%s}(%s)", (k==1||nl==1)?"\t":" ", c, s, s, p );
							else
								sprintf( out+strlen(out), "%s%s|%s(%s)", (k==1||nl==1)?"\t":" ", c, s, p );
						} else
							if (style&StyleMorForm)
								sprintf( out+strlen(out), "%s%s|%s{%s}", (k==1||nl==1)?"\t":" ", c, s, s );
							else
								sprintf( out+strlen(out), "%s%s|%s", (k==1||nl==1)?"\t":" ", c, s );
					}
				} else {
					c = get_word_dic( s, w );
					// msg("((%s!-!%s!-!%s))\n", s, w, c );
					// sprintf( out+strlen(out), " !%s!%s!", ms[k], s);
					if (newwords==1) {
						if (!c  && s[0] != '0'  && s[0] != '+') {
							lg += strlen(s) + 3;
							if (style&(StyleNewWordsPho|StyleMorFormPho|StyleMorPho)) {
								char p[MAXSIZEWORD];
								phonetize(s,p);
								if (style&StyleMorFormPho)
									sprintf( out+strlen(out), "%s?|%s{%s}(%s)\n", s, s, s, p);
								else
									sprintf( out+strlen(out), "%s?|%s(%s)\n", s, s, p);
							} else
								if (style&StyleMorForm)
									sprintf( out+strlen(out), "%s?|%s{%s}\n", s, s, s);
								else
									sprintf( out+strlen(out), "%s?|%s\n", s, s);
						}
					} else if (!c) {
						// msg("NULL ((%s!-!%s!-!%s))\n", s, w, c );
						lg += strlen(s) + 3;
						if (style&(StyleNewWordsPho|StyleMorFormPho|StyleMorPho)) {
							char p[MAXSIZEWORD];
							phonetize(s,p);
							if (style&StyleMorFormPho)
								sprintf( out+strlen(out), "%s?|%s{%s}(%s)", (k==1||nl==1)?"\t":" ", s, s, p);
							else
								sprintf( out+strlen(out), "%s?|%s(%s)", (k==1||nl==1)?"\t":" ", s, p);
						} else
							if (style&StyleMorForm)
								sprintf( out+strlen(out), "%s?|%s{%s}", (k==1||nl==1)?"\t":" ", s, s);
							else
								sprintf( out+strlen(out), "%s?|%s", (k==1||nl==1)?"\t":" ", s);
					} else {
						char categ[MAXSIZECATEGORY];
						//msg("OK ((%d %s!-!%s!-!%s))\n", style, s, w, c );
						if (style&StyleMorForm) {
							cat_from_word_form(c, s, categ);
						} else if (style&StyleMorFormPho) {
							catpho_from_word_form(c, s, categ);
						} else if (style&StyleMorPho) {
							catpho_from_word(c, categ);
						} else
							cat_from_word(c, categ);
						//msg("CATEG (%s)--(%s!-!%s!-!%s))\n", categ, s, w, c );
						lg += strlen(categ) + 1;
						sprintf( out+strlen(out), "%s%s", (k==1||nl==1)?"\t":" ", categ);
					}
				}
				if (lg>=linelimit && newwords==0) {
					sprintf( out+strlen(out), "\n" );
					lg = 0;
				}
			} else if (is_chat_punctuation(s) && newwords==0) {
				lg += strlen(s) + 1;
				sprintf( out+strlen(out), "%s%s", (k==1||nl==1)?"\t":" ", s );
			}
		}
		if (lg != 0 && newwords==0) sprintf( out+strlen(out), "\n" );
	}
}

char* split_and_pack(char* from, char* sep, char* to)
{
	char s[MAXSIZECATEGORY];
	strcpy( s, from );
	char *m[32];
	int nm = split_with( s, m, sep, 32);
	sort_and_pack_in_string( m, nm, to, MAXSIZECATEGORY, sep);
	return to;
}

/** prints the phonology **/
void print_phonology(int nm, char** ms, char* out, int style, int linelimit)
{
	if (nm>1) {
		sprintf( out, style & StyleSPho ? "%%xspho:" : "%%xpho:" );
		int lg = 1;
		for (int k=1; k<nm; k++) {
			if ( is_bullet(ms[k]) || ms[k][0] == '[' ) continue;
			char w[MAXSIZECATEGORY];
			char sn[MAXSIZEWORD], *s;
			if ( sizeof(sn) >= strlen(ms[k]) ) {
				stripof(ms[k],sn);
				s = sn;
			} else
				s = ms[k];
			if ( !is_empty(s) && !is_chat_punctuation(s) ) {
				int nl = lg == 0 ? 1 : 0;
				char* c = is_Catfix( s );
				// msg ( "catfix: !%s!%s!\n", s, c );
				if (c) {
					char o[MAXSIZECATEGORY];
					strcat(o, "XXX" );
					sprintf( out+strlen(out), "%s%s", (k==1||nl==1)?"\t":" ", o );
					lg += strlen(o)+1;
				} else {
					c = get_word_dic( s, w );
					// sprintf( out+strlen(out), " !%s!%s!", ms[k], s);
					if (!c) {
						lg += strlen(s) + 3;
						sprintf( out+strlen(out), "%sXXX", (k==1||nl==1)?"\t":" " );
					} else {
						char phono[MAXSIZECATEGORY];
						phono_from_word(c, phono);
						if (style&StyleSPho) {
							char pck[MAXSIZECATEGORY];
							split_and_pack(phono, "|", pck);
							strcpy(phono, pck);
						}
						lg += strlen(phono) + 1;
						sprintf( out+strlen(out), "%s%s", (k==1||nl==1)?"\t":" ", phono);
					}
				}
				if (lg>=linelimit) {
					sprintf( out+strlen(out), "\n" );
					lg = 0;
				}
			}
		}
		if (lg != 0) sprintf( out+strlen(out), "\n" );
	}
}

char* compute_morline(char* T, char* MLine, int language_s, int output_s, int linelimit)
{
	char* ms[124];
	char  res[4096];
	char _res2[4096];
	char* res2 = _res2;
	int nres = 4096;
	int nm =0;
	char* sbegin = "";
	char* send = CompoundSepsEnd;
	if ( language_s==CompoundFR ) send = CompoundSepsEndApostrophe;
	
	nm = split_all_formats_CHAT_special( T, ms, res, CompoundWhiteSpaces, sbegin, send, CompoundPunctuations, sizeof(ms)/sizeof(char*), sizeof(res) );
	
	if (output_s & (StyleNewWords|StyleNewWordsPho) )
		print_morphology(nm, ms, MLine, output_s, 1, linelimit);
	else if (output_s & (StyleMor|StyleXMor|StyleMorPho|StyleMorFormPho|StyleMorForm) )
		print_morphology(nm, ms, MLine, output_s, 0, linelimit);
	else if (output_s & (StyleXPho|StyleSPho) )
		print_phonology(nm, ms, MLine, output_s, linelimit);
	return MLine;
}
