/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************************
** compoundlib.cpp
** normalizes and formats compound words in a CHAT format file
***********************************************************************/

#include "system.hpp"
#include "tags.hpp"
#include "database.hpp"
#include "compound.hpp"

/* parameters of compounds command */
static int x_language_style = 0;	// by default do nothing to apostrophes, if == 1 do French style processing
static int x_full_dot_at_end = 0;	// by default do nothing. if == 1 add a full dot at the end of utterances if no utterance ending present , if == -1 suppress full dot.
static int x_keep_old_compounds = 1;	// means that compounds written in the text are kept as such ; if == 0 then only compounds from external file are valid (compounds written in the texte are ignored.
static int x_change_minus_to_plus = 0;	// to be used only if there is no mMOR marking in *CHI lines (for coumpounds, a-b notation equals a+b notation.) default for French.

/* parameters of compounds command */
void compound_set_language(int lg)	// by default do nothing to apostrophes, if == 1 do French style processing
{
	x_language_style = lg;
}
void compound_set_full_dot_at_end(int p)	// by default do nothing. if == 1 add a full dot at the end of utterances if no utterance ending present , if == -1 suppress full dot.
{
	x_full_dot_at_end = p;
}
void compound_set_keep_old_compounds(int p)	// means that compounds written in the text are kept as such ; if == 0 then only compounds from external file are valid (compounds written in the texte are ignored.
{
	x_keep_old_compounds = p;
}
void compound_set_change_minus_to_plus(int p)	// to be used only if there is no mMOR marking in *CHI lines (for coumpounds, a-b notation equals a+b notation.) default for French.
{
	x_change_minus_to_plus = p;
}

int compound_get_language()	// by default do nothing to apostrophes, if == 1 do French style processing
{
	return x_language_style;
}
int compound_get_full_dot_at_end()	// by default do nothing. if == 1 add a full dot at the end of utterances if no utterance ending present , if == -1 suppress full dot.
{
	return x_full_dot_at_end;
}
int compound_get_keep_old_compounds()	// means that compounds written in the text are kept as such ; if == 0 then only compounds from external file are valid (compounds written in the texte are ignored.
{
	return x_keep_old_compounds;
}
int compound_get_change_minus_to_plus()	// to be used only if there is no mMOR marking in *CHI lines (for coumpounds, a-b notation equals a+b notation.) default for French.
{
	return x_change_minus_to_plus;
}

#define maxSizeLine 4096
/****
	Organizing storage of compounds
****/

struct Exp {
	// elements pour le tri.
	int p,e,mo;
	// valeurs
	char** m;
	int   nm;
	Exp() { m = 0; nm = 0; }
	~Exp();
	int Init(char** s, int ns);
	int InitC(const char** s, int ns);
	int Cmp(char** s, int ns);
	int FindIn(char** s, int ns);
};
/* Global variables */

static Exp* s_exps=0;
static int  n_exps=0;

Exp::~Exp()
{ 
	if (nm!=0)
		for (int i=0;i<nm;i++)
			delete [] m[i];
	delete [] m;
	m=0;
	nm=0;
}

int Exp::Init(char** s, int ns)
{
	nm = ns;
	m = new char* [ns];
	for (int i=0; i<ns; i++) {
		m[i] = new char [strlen(s[i])+1];
		strcpy( m[i], s[i] );
	}
	return 1;
}

int Exp::InitC(const char** s, int ns)
{
	nm = ns;
	m = new char* [ns];
	for (int i=0; i<ns; i++) {
		m[i] = new char [strlen(s[i])+1];
		strcpy( m[i], s[i] );
	}
	return 1;
}

int Exp::Cmp(char** s, int ns)
{
	if (ns<nm) return 0;
	for (int i=0; i<ns; i++) {
		// strip s[i] of IgnoreOnFind
		char sn[MAXSIZEWORD];
		if ( sizeof(sn) >= strlen(s[i]) ) {
			stripof(s[i],sn);
			if ( strcmp( m[i], sn ) ) return 0;
		} else if ( strcmp( m[i], s[i] ) ) return 0;
	}
	return 1;
}

int Exp::FindIn(char** s, int ns)
{
	if (ns<nm) return -1;
	for (int i=0; i+nm<=ns; i++) {
		if ( Cmp( &s[i], nm ) ) return i;
	}
	return -1;
}

static int compare( const void *arg1, const void *arg2 )
{
	/* Compare all of both elements */
	Exp* a1 = ( Exp* ) arg1;
	Exp* a2 = ( Exp* ) arg2;

	int r = a2->p - a1->p;
	if ( r!=0 )
		return r;
	r = a2->mo - a1->mo;
	return - r;
}

void clear_exps()
{
  #if defined(UNX) || defined(POSTCODE)
	delete [] s_exps ;
  #else
	delete [n_exps] s_exps ;
  #endif
  s_exps = 0;
  n_exps = 0;
}

static int init_exps( FNType* fname, Exp* &s_exps, int &n_exps, int list_of_compounds)
{
	FILE* exps = fopen( fname, "r" );
	if (!exps) {
#if defined(POSTCODE) || defined(CLANEXTENDED)
		FNType mFileName[FNSize];
		exps=OpenMorLib(fname, "r", FALSE, FALSE, mFileName);
		if (exps == NULL)
			exps= OpenGenLib(fname,"r",FALSE,FALSE,mFileName);
		if (exps == NULL) {
#endif
			msg("Cannot find file %s\n", fname );
			return 0;
#if defined(POSTCODE) || defined(CLANEXTENDED)
		}
#endif
	}
	n_exps = 0;
	int erreurs = 0;
	int erreurs2 = 0;
	int noline=0;
	char line[maxSizeLine];
	char  tmp[512];
	const char* send = CompoundSepsEnd;
	strcpy( tmp, CompoundPlusMoinsWhiteSpaces );
	if ( compound_get_language() == CompoundFR ) send = CompoundSepsEndApostrophe;
	strcat( tmp, send );
	while( fgets(line, maxSizeLine, exps) ) {
		noline ++;
		if ( line[0] == '@' || line[0] == '%' ) continue;	// comments and other commands
		char* ms[24];
		int nm = split_with( line, ms, tmp, 24 );
		if (nm==0) continue;
		if (nm==1)  {
			// msg( "tmp=(%s-%s-%d) - ", tmp, send, compound_get_language() == CompoundFR );
			msg( "Error: one word only at line %d: %s\n", noline, line );
			erreurs ++;
			continue;
		}
		n_exps ++;
	}
	rewind(exps);

	int n = 0;
	if (n_exps==0) {
		msg( "Warning: no compounds given - cleaning file only\n" );
		s_exps = new Exp [1];
		const char* ms[2];
		ms[0] = "xxxxxxxxxxxxxxxxxxxxxxxx";
		ms[1] = "xxxxxxxxxxxxxxxxxxxxxxxx";
		s_exps[0].InitC(ms,2);
		return 1;
	}
	s_exps = new Exp [n_exps];
	while( fgets(line, maxSizeLine, exps) ) {
		char* ms[48];

		if ( line[0] == '@' || line[0] == '%' ) continue;	// comments and other commands
		int nm = split_all_formats( line, ms, tmp, CompoundPlusMoinsWhiteSpaces, "", send, "", sizeof(ms)/sizeof(char*), sizeof(tmp) );

		// { int i; printf ("nm=%d\n", nm); for (i=0;i<nm;i++) printf ("%d <%s>\n", i, ms[i] ); }

		if (nm==0) continue;
		if (nm==1) {
//			msg( "Error at line %d: %s", noline, line );
			erreurs2 ++;
			continue;
		}
		s_exps[n].Init(ms,nm);
		n++;
	}
	fclose(exps);
	if (erreurs != erreurs2 || n != n_exps) {
		clear_exps();
		return 0;
	}
	int l;
	for (n=0; n<n_exps; n++) {
		s_exps[n].p = 0;
		s_exps[n].mo = 0;
		s_exps[n].e = 0;
		for (l=0; l<n_exps; l++) {
			if ( s_exps[n].FindIn( s_exps[l].m, s_exps[l].nm ) != -1 ) { // n is in l
				s_exps[n].mo ++;
			} else if ( s_exps[l].FindIn( s_exps[n].m, s_exps[n].nm ) != -1 ) { // l is in n
				s_exps[n].p ++;
			} else {
				s_exps[n].e ++;
			}
		}
	}

	qsort( (void *)s_exps, (size_t)n_exps, sizeof(Exp), compare );

	if (list_of_compounds) {
		for (n=0; n<n_exps; n++) {
			msg( "%d %d %d ", s_exps[n].p, s_exps[n].mo, s_exps[n].e );
			for (l=0; l<s_exps[n].nm-1; l++)
				msg( "%s+", s_exps[n].m[l] );
			msg( "%s\n", s_exps[n].m[l] );
		}
	}
	return 1;
}

/****
	End storage of compounds
****/

#define change_doublequote_to_antislash 1
#define change_pointmiddle_to_antislash 1

static int pack_at( char** ms, int &nm, int n, Exp* exp, char* &res2, int &nres )
{
	int i;
	int len = 0;
	char* prev;
	for (i=0; i<exp->nm; i++)
//		len += strlen(exp->m[i])+1;
		len += strlen(ms[n+i])+1;
	if (nres<len) return 0;
	nres -= len;
	prev = ms[n];
	ms[n] = res2;
	res2 += len;
/***
	strcpy( ms[n], exp->m[0]);
	for (i=1; i<exp->nm; i++) {
		if ( language==0 || ms[n][strlen(ms[n])-1] != '\'' )
			strcat( ms[n], "+" );
		strcat( ms[n], exp->m[i] );
	}
***/
	strcpy( ms[n], prev );
	for (i=1; i<exp->nm; i++) {
		if ( compound_get_language() == CompoundUS || ms[n][strlen(ms[n])-1] != '\'' )
			strcat( ms[n], "+" );
		strcat( ms[n], ms[n+i] );
	}
	for (i=n+1; i<nm-(exp->nm-1) ; i++)
		ms[i] = ms[i+(exp->nm-1)];
	nm -= exp->nm-1;
	return 1;
}

static char* filter_with_compounds(char* T, char* C, Exp* &s_exps, int &n_exps)
{
	char* ms[124];
	char  res[4096];
	char _res2[4096];
	char* res2 = _res2;
	int nres = 4096;
	int nm =0;
	const char* sbegin = "";
	const char* send = CompoundSepsEnd;
	if ( compound_get_language() == CompoundFR ) send = CompoundSepsEndApostrophe;

	nm = split_all_formats_CHAT_special( T, ms, res, CompoundWhiteSpaces, sbegin, send, CompoundPunctuations, sizeof(ms)/sizeof(char*), sizeof(res) );

	// { int i; printf ("nm=%d\n", nm); for (i=0;i<nm;i++) printf ("%d <%s>\n", i, ms[i] ); }

	/** finds all compound words **/
	for (int x=0; x<n_exps; x++) {
		int n;
		while ( (n=s_exps[x].FindIn(ms,nm)) != -1 ) {
			if ( !pack_at( ms, nm, n, &s_exps[x], res2, nres ) )
				msg( "- not enough memory\n");
		}
	}

	/** puts the result in the string **/

	C[0] = '\0';
	int lw = 0;
	if ( compound_get_full_dot_at_end() ) {
		lw = nm-1;
		while (lw>0 && ( ms[lw][0]=='\x15' || ms[lw][0]=='[' ) ) lw--;
	}

	sprintf( C+strlen(C), "%s", ms[0]);
	if (nm>1) {
		/** special processing for old style French : a' +b ==> a'b **/
		for (int k=1; k<nm; k++) {
			//msg( "printing output %d %s %s\n", compound_get_language() == CompoundFR , ms[k-1], ms[k]);
			if (compound_get_language() == CompoundFR && ms[k][0] == '+' && ms[k-1][strlen(ms[k-1])-1] == '\'' )
					sprintf( C+strlen(C), "%s", &(ms[k][1]) );
			else
				sprintf( C+strlen(C), "%s%s", k==1?"\t":" ", ms[k]);

			if ( compound_get_full_dot_at_end() && k==lw && strcmp(ms[lw],".") && test_special_plus( ms[lw] ) == 0 && strchrConst( CompoundPunctuations, ms[lw][0] ) == 0 )
				sprintf( C+strlen(C), "%s.", k==0?"\t":" " );
		}
	}
	return C;
}

char* filter_with_compounds(char* T, char* C)
{
	if ( !s_exps ) {
		strcpy( C, T );
		return C;
	}
	return filter_with_compounds(T, C, s_exps, n_exps );
}

int init_exps( FNType* fname, int list)
{
	return init_exps( fname, s_exps, n_exps, list);
}
