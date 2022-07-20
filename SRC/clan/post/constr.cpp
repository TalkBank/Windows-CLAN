/**********************************************************************
	"Copyright 1990-2020 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- constr.cpp	- v1.0.0 - july 2003

this file contains the main procedure of the CONSTR CLAN tool

Description of the Procedures and Function of the current file:

1) opens a rule file and compile it
2) opens POST-ed file(s) and processes it with the previous rules

constr_main	== general call function + help display
constr_call == process one POST-ed file
constr_compile == read and compile rule file
free_compile == free all compilation data
compute_constr( in, out ) == calculate constructions

|=========================================================================================================================*/

//#define DBG0
//#define DBG1
//#define DBG2

// local defines
#ifdef MacOsx
#define CONSTR_VERSION "v.0.9.1(compiled on MacOsx)"
#else
#define CONSTR_VERSION "v.0.9.1"
#endif

#ifdef POSTCODE
	#define CHAT_MODE 0
#endif
#include "system.hpp"
#include "database.hpp"
#include "constr.hpp"
#ifdef POSTCODE
	#define _main constr_main
	#define call constr_call
	#define IS_WIN_MODE FALSE
	#include "mul.h"

#else
	#define EOS '\0'
#endif

#define MaxSizeLine 2048
#define ConstrCompileSeps " \t\r\n"
#define MaxWordsOnLine 64

static int static_level = 0;
static int static_replace = 0;
static char static_tiername[MaxFnSize];
static char static_outtiername[MaxFnSize];
static char static_outsecondtiername[MaxFnSize];
static int static_noword = 0;
static int static_nonumber = 0;
static int static_macrodisplay = 0;
static int static_special = 0;
static int static_last_sec = 0;
static int static_ctrperline = 0;
static int static_nw = 0;
static int static_snw = 0;
static char static_extname[MaxFnSize];

#ifdef MacOsx
int strcmpi(char* s1, char* s2)
{
	return strcasecmp(s1,s2);
}
#endif

mortag* replace_a_in_b( mortag* a, mortag* b)
{
	/* 1) supprimer toutes les parties filtrees de b plus MT
	   2) inserer dans le resultat tout a (y compris MT)
	   3) renvoyer resultat alloue par dynalloc.

	   autre version plus simple
	   1) faire une copie de a par dynalloc
	   2) y rajouter les elements de b qui ne sont pas filtres sauf MT en les creant par dynalloc
	*/

#ifdef DBG0
msgfile( stderr, "replace " );
print_mortag_2( stderr, a );
msgfile( stderr, " *** " );
print_mortag_2( stderr, b );
#endif

	mortag* ret = (mortag*) dynalloc(sizeof(mortag));
	ret->MT=0; ret->nSP=0; ret->SP=0; ret->SPt=0; ret->GR=0; ret->translation=0;
	ret->MT = dynalloc(strlen(a->MT)+1);
	strcpy( ret->MT, a->MT );
	if (a->GR) {
		ret->GR = dynalloc( strlen(a->GR)+1 );
		strcpy( ret->GR, a->GR );
	}
	if (a->nSP) {
		ret->nSP = a->nSP;
		ret->SP = (char**)dynalloc( sizeof(char*) * (a->nSP+b->nSP) ); // cree de la place pour phase 2)
		ret->SPt = dynalloc(a->nSP+b->nSP); // cree de la place pour phase 2)
		memcpy( ret->SPt, a->SPt, a->nSP );
		int k;
		for (k=0;k<a->nSP;k++) {
			if (a->SP[k]) {
				ret->SP[k] = dynalloc( strlen(a->SP[k])+1 );
				strcpy( ret->SP[k], a->SP[k] );
			}
		}
	} else if (b->nSP) {
		ret->SP = (char**)dynalloc( sizeof(char*) * (b->nSP) ); // cree de la place pour phase 2)
		ret->SPt = dynalloc(b->nSP); // cree de la place pour phase 2)
	}

	if (!a->GR && b->GR && !filter_ok_out(b->GR)) {
		ret->GR = dynalloc( strlen(b->GR)+1 );
		strcpy( ret->GR, b->GR );
	}
	if (b->nSP) {
		int k, t;
		for (k=0, t=ret->nSP; k<b->nSP; k++) {
			if (b->SP[k] && !filter_ok_out(b->SP[k])) {
				ret->SP[t] = dynalloc( strlen(b->SP[k])+1 );
				strcpy( ret->SP[t], b->SP[k] );
				ret->SPt[t] = b->SPt[k];
				t++;
			}
		}
		ret->nSP = t;
	}

#ifdef DBG0
msgfile( stderr, " ==> " );
print_mortag_2( stderr, ret );
msgfile( stderr, "\n" );
#endif
	return ret;
}

static CRule* static_rules;
const int NbEntryRules = 64;
static CRule* static_macros;
const int NbEntryMacros = 128;

CRule::CRule()
{
	tail = 0;
	ntail = 0;
	head = 0;
	nhead = 0;
	next = 0;
	section = 0;
}

void CRule::init(mortag* h, int nh, mortag* t, int nt)
{
	tail = t;
	ntail = nt;
	head = h;
	nhead = nh;
}

void CRule::print(FILE* out)
{
	if (!tail) {
		msgfile(out, "Empty rule.\n");
		return;
	}
	msgfile(out, "(%d) ", nhead);
	int i;
	for (i=0;i<nhead;i++) {
		char* s=mortag_to_string( &head[i] );
		msgfile(out, "%s ", s );
	}
	msgfile(out, "\t=> (%d) ", ntail);
	for (i=0;i<ntail;i++) {
		char* s=mortag_to_string( &tail[i] );
		msgfile(out, "%s ", s );
	}
	if (next)
		msgfile(out, " * " );
}

void CRule::delete_rule_internal()
{
	int i;
	if (head) {
		for (i=0;i<nhead;i++) {
			delete_mortag_from_heap( &head[i] );
		}
		delete [] /*[nhead]*/ head;
	}
	if (tail) {
		for (i=0;i<ntail;i++) {
			delete_mortag_from_heap( &tail[i] );
		}
		delete [] /*[ntail]*/ tail;
	}
}

void CRule::delete_rule()
{
	delete_rule_internal();
	CRule* n = next;
	while (n) {
		n->delete_rule_internal();
		CRule* c = n;
		n = n->next;
		delete c;
	}
}

CRule* hashfind_rule(CRule* rules, int nbe, mortag* t0, int& n )
{
	char str[MaxSzTag];
	char* s_key = mortag_to_string_filtered(t0, str);
	int key = s_key[0], i = 2;
	key += (s_key[1]) ? s_key[1]*7 : 17;
	if ( s_key[1] ) while ( s_key[i] ) {
		key += s_key[i]*7;
		i++;
	}
	key = key % nbe;
	n = key;
	return &rules[key];
}

CRule* hashfind_rule(CRule* rules, int nbe, mortag* t0 )
{
	int k;
	return hashfind_rule(rules, nbe, t0, k);
}

void add_rule(mortag* h, mortag* t, int sz, int p_section)
{
	int key;
	CRule* hr = hashfind_rule(static_rules, NbEntryRules, &t[0], key );
	if ( !hr->head ) { // found empty slot.
		hr->init(h,1,t,sz);
		hr->section = p_section;
	} else if (hr->ntail < sz) {
		CRule* pr = new CRule;
		memcpy(pr, &static_rules[key], sizeof(CRule));
		hr->init(h,1,t,sz);
		hr->section = p_section;
		hr->next = pr;
	} else {
		CRule* cr = hr;
		while (cr->ntail >= sz && cr->next) {
			hr = cr;
			cr = cr->next;
		}
		if (!cr->next) {
			CRule* pr = new CRule;
			pr->init(h,1,t,sz);
			pr->section = p_section;
			if (cr->ntail >= sz) { //add after
				cr->next = pr;
				pr->next = 0;
			} else {
				hr->next = pr;
				pr->next = cr;
			}
		} else {
			CRule* pr = new CRule;
			pr->next = cr;
			pr->init(h,1,t,sz);
			pr->section = p_section;
			hr->next = pr;
		}
	}
}

void add_macro(mortag* h, mortag* t)
{
	CRule* cr = hashfind_rule(static_macros, NbEntryMacros, &t[0] );
	if ( !cr->head ) // found empty slot.
		cr->init(h,1,t,1);
	else {
		while (cr->next) cr = cr->next;
		cr->next = new CRule;
		cr->next->init(h,1,t,1);
	}
}

CRule* find_macro(mortag* t)
{
#ifdef DBG1
print_mortag_2( stderr, t );
msgfile( stderr, " hashfind " );
#endif
	CRule* cr = hashfind_rule(static_macros, NbEntryMacros, t );
	if ( !cr || !cr->head ) { // found empty slot.
		return 0;
	} else if ( is_eq_filtered( t, cr->tail ) ) {
		return cr;
	} else {
		while (cr->next) {
			cr = cr->next;
			if ( is_eq_filtered( t, cr->tail ) ) return cr;
		}
	}
	return 0;
}

mortag* apply_macros(mortag* t)
{
/*	CRule* m = find_macro(t);
	if (m) {
		if ( strlen(m->head->MT) > strlen(t->MT) ) {
			t->MT = dynalloc(strlen(m->head->MT)+1);
		}
		strcpy( t->MT, m->head->MT );
	}
	return t;
*/
	CRule* m = find_macro(t);
#ifdef DBG1
print_mortag_2( stderr, t );
msgfile( stderr, " * " );
if (m) print_mortag_2( stderr, m->head ); else msgfile( stderr, "none" );
msgfile( stderr, "\n" );
#endif
	if (m)
		return replace_a_in_b(m->head,t);
	else
		return t;
}

CRule* find_firstrule(mortag* t)
{
	CRule* cr = hashfind_rule(static_rules, NbEntryRules, t );
	if ( !cr->head ) // found empty slot.
		return 0;
	else if ( is_in_plus_gr( t, cr->tail ) ) return cr;
	else {
		while (cr->next) {
			cr = cr->next;
			if ( is_in_plus_gr( t, cr->tail ) ) return cr;
		}
	}
	return 0;
}

CRule* find_nextrule(CRule* cr, mortag* t)
{
	if ( !cr->head ) // found empty slot.
		return 0;
	else {
		while (cr->next) {
			cr = cr->next;
			if ( is_in_plus_gr( t, cr->tail ) ) return cr;
		}
	}
	return 0;
}

int constr_compile (char* rname)
{
	FILE *rfp;
	if ( (rfp=fopen(rname,"r")) == NULL ) {
#if (POSTCODE||CLANEXTENDED)
		rfp=OpenMorLib(rname, "r", FALSE, FALSE, NULL);
		if (rfp == NULL)
			rfp= OpenGenLib(rname, "r", FALSE, FALSE, NULL);
		if (rfp == NULL) {
#endif
			msg ( "Unable to open \"%s\" for reading\n", rname );
			return 0;
#if (POSTCODE||CLANEXTENDED)
		}
#endif
	}

	int section = 1;
	static_rules = new CRule [NbEntryRules];
	static_macros = new CRule [NbEntryMacros];
	// read FILE* rfp
	char line[MaxSizeLine];
	int	line_number = 0;
	int error = 0;
	while ( fgets( line, MaxSizeLine, rfp ) ) {
		line_number++;
		// check for comments (everything after a %)
		char* comment = strchr(line,'%');
		if (comment) *comment = '\0';
		// split into words.
		char* sw[MaxWordsOnLine];
		int curr_section = 0;
		int nw = split_with( line, sw, ConstrCompileSeps, sizeof(sw)/sizeof(char*) );
		if (nw == 0) continue;	// no word on the current line
		char** words = sw;
		if (words[0][0] == '#') { // section indicator.
			curr_section = atoi(words[0]+1);
			if (curr_section<1 || curr_section> 100) {
				msg( "section number restricted to 1 to 99 (see line %d)\n", line_number );
				error++;
				continue;
			}
			if (nw == 1) {
				section = curr_section;
				continue;
			} else {
				words++;
				nw--;
			}
		}
		if (static_last_sec<section) static_last_sec = section;
		if (nw < 3 ) {
			msg( "error on line %d: not enough words on line\n", line_number);
			error++;
			continue;
		}
		if ( !strcmpi( "is", words[1] ) ) {
			// if second word is 'IS', then definition of synonym tags names.
			int i;
			for (i=2;i<nw;i++) {
				mortag* t = new mortag;
				mortag* h = new mortag;
				if ( ! make_mortag_from_heap( words[0], h ) ) {
					msg( "out of memory line %d\n", line_number );
					return 0;
				}
				if ( ! make_mortag_from_heap( words[i], t ) ) {
					msg( "out of memory line %d\n", line_number );
					return 0;
				}

#ifdef DBG0
msgfile( stderr, "%s %s ", words[0], mortag_to_string(h) );
print_mortag_2( stderr, h );
msgfile( stderr, " is " );
msgfile( stderr, "%s %s ", words[i], mortag_to_string(t) );
print_mortag_2( stderr, t );
msgfile( stderr, "\n" );
#endif
				if ( find_macro(t) ) {
					char str[MaxSzTag];
					msg( "cannot define a macro character twice (%s-%s) : line %d\n", mortag_to_string_filtered(t,str), mortag_to_string(t), line_number );
					error++;
				} else
					add_macro( h, t );
			}
		} else if ( !strcmpi( "=", words[1] ) ) {
			// if second word is '=', then definition of rules.
			mortag* t = new mortag[nw-2];
			mortag* h = new mortag;
			if ( ! make_mortag_from_heap( words[0], h ) ) {
				msg( "out of memory line %d\n", line_number );
				return 0;
			}
			int i;
			for (i=2;i<nw;i++)
				if ( ! make_mortag_from_heap( words[i], &(t[i-2]) ) ) {
					msg( "out of memory line %d\n", line_number );
					return 0;
				}
				add_rule( h, t, nw-2, curr_section==0 ? section : curr_section );
		} else {
			int k;
			for (k=2;k<nw;k++) {
				if ( !strcmpi( "=", words[k] ) ) {
					// if word is '=', then definition of rule with multiple left part.
					if ( (nw-1)/2 < k ) { // right part to small
						msg("bad format on line %d\n", line_number );
						error ++;
					} else {
						msg( "// =.= not implemented yet\n" );
						error ++;
					}
					break;
				}
			}
			if ( k==nw ) {
				msg("bad format on line %d : missing '=' sign or 'is' not in second position\n", line_number );
				error ++;
			}
		}
	}
	fclose(rfp);
	return error == 0 ? 1 : 0;
}

void dump_compile()
{
	register int i;
	int nrules = 0, nentryrules = 0;
	for (i=0;i<NbEntryRules; i++) {
		if ( static_rules[i].head ) {
			nentryrules ++;
			nrules ++;
			CRule* n = static_rules[i].next;
			while (n) {
				n = n->next;
				nrules ++;
			}
		}
	}
	int nmacros = 0, nentrymacros = 0;
	for (i=0;i<NbEntryMacros; i++) {
		if ( static_macros[i].head ) {
			nentrymacros ++;
			nmacros ++;
			CRule* n = static_macros[i].next;
			while (n) {
				n = n->next;
				nmacros ++;
			}
		}
	}

	msg( "Total of %d rules (%d entries in hashcode)\n", nrules, nentryrules );
	msg( "Total of %d macros (%d entries in hashcode)\n", nmacros, nentrymacros );

	msg( "RULES...\n" );
	for (i=0;i<NbEntryRules; i++) {
		if ( static_rules[i].head ) {
			msg( "[%d]\n\t",i);
			static_rules[i].print(stdout);
			CRule* n = static_rules[i].next;
			while (n) {
				msg( "\n\t" );
				n->print(stdout);
				msg( " #%d", n->section ); 
				n = n->next;
			}
			msg( "\n" );
		}
	}
	msg( "MACROS...\n" );
	for (i=0;i<NbEntryMacros; i++) {
		if ( static_macros[i].head ) {
			msg( "[%d]\n\t",i);
			static_macros[i].print(stdout);
			CRule* n = static_macros[i].next;
			while (n) {
				msg( "\n\t" );
				n->print(stdout);
				n = n->next;
			}
			msg( "\n" );
		}
	}
}

void free_compile()
{
	int k;
	for (k=0;k<NbEntryRules; k++)
		static_rules[k].delete_rule();
	// delete_mortag_from_heap
	delete [] /*[NbEntryRules]*/ static_rules;
	for (k=0;k<NbEntryMacros; k++)
		static_macros[k].delete_rule();
	// delete_mortag_from_heap
	delete [] /*[NbEntryRules]*/ static_macros;
}

NTree::NTree()
{
	val = 0;
	mac = 0;
	down = 0;
	next = 0;
	prev = 0;
}

void NTree::init(mortag* m, int nm)
{
	if (nm==1) {
		// find if there are macros to apply
		val = apply_macros(m);
		if (val!=m) 
			mac = m;
		else
			mac = 0;
		down = 0;
		prev = 0;
		next = 0;
	} else {
		// find if there are macros to apply
		val = apply_macros(m);
		if (val!=m)
			mac = m;
		else
			mac = 0;
		down = 0;
		prev = 0;
		next = (NTree*)dynalloc(sizeof(NTree));
		next->init(m+1,nm-1);
	}
}

int NTree::compare_with_rule(CRule* r)
{
	// compare rule with MORTAGS
	if ( r->ntail == 1 ) return 1;
	int i = 1;
	NTree* t = this;
	do {
		if (!t->next) return 0;
		t = t->next;
		if ( !is_in_plus_gr( t->val, &r->tail[i] ) ) return 0;
		i++;
	} while (i<r->ntail);
	return 1;
}

CRule* NTree::find(int section)
{
	CRule* r = find_firstrule(val);
	if ( !r ) return 0;
#ifdef DBG2
print_mortag_2( stderr, val);
msgfile( stderr, "rule 1 : " );
r->print(stderr);
msgfile( stderr, "\n" );
#endif
	if ( r->section == section && this->compare_with_rule(r) )
		return r;
	while ( r=find_nextrule(r,val) ) {
#ifdef DBG2
print_mortag_2( stderr, val);
msgfile( stderr, "rule x : " );
r->print(stderr);
msgfile( stderr, "\n" );
#endif
		if ( r->section == section && this->compare_with_rule(r) )
			return r;
	}
	return 0;
}

NTree* find_all_constructions(NTree* head, int section)
{
	int foundaconstr;
	do {
		foundaconstr = 0;
		NTree* t1 = head;
		NTree* prev = 0;
		CRule* rule;
		do {
			if ( rule = t1->find(section) ) {
				foundaconstr = 1;
				// now substitute
				NTree* tl = t1;
				int i=rule->ntail-1;
				while (i>0) {
					tl = tl->next;
					i--;
				}
				NTree* new_node = new NTree;
				new_node->next = tl->next;
				new_node->down = t1;
				tl->next = 0;
				new_node->val = &rule->head[0];
				mortag* p = new_node->val;
				new_node->val = apply_macros(p); // apply a macro if possible
				if (new_node->val != p)
					new_node->mac = p;
				else
					new_node->mac = 0;
				if (t1==head) 
					head = new_node;
				else
					prev->next = new_node;
			} else {
				prev = t1;
				t1 = t1->next;
			}
		} while (t1 && foundaconstr==0);
	} while (foundaconstr==1);
	return head;
}

void NTree::l_print(FILE* out)
{
	char str[MaxSzTag];
	if (static_special == 2 || static_special == 3) {
		fprintf( out, "%s ", val->GR ? val->GR : val->MT );
	} else if ( static_noword == 1 ) {
		if ( static_macrodisplay == 0 ){
			if (mac)
				fprintf(out, "%s ", mortag_to_string_filtered(mac,str) );
			else
				fprintf(out, "%s ", mortag_to_string_filtered(val,str) );
		} else if (static_macrodisplay == 1) {
			fprintf(out, "%s ", mortag_to_string_filtered(val,str) );
		} else {
			if (mac) {
				fprintf(out, "%s/", mortag_to_string_filtered(mac,str) );
				fprintf(out, "%s ", mortag_to_string_filtered(val,str) );
			} else
				fprintf(out, "%s ", mortag_to_string_filtered(val,str) );
		}
	} else {
		if ( static_macrodisplay == 0 ){
			if (mac)
				fprintf(out, "%s ", mortag_to_string(mac) );
			else
				fprintf(out, "%s ", mortag_to_string(val) );
		} else if (static_macrodisplay == 1) {
			fprintf(out, "%s ", mortag_to_string(val) );
		} else {
			if (mac) {
				fprintf(out, "%s/", mortag_to_string(mac) );
				fprintf(out, "%s ", mortag_to_string(val) );
			} else
				fprintf(out, "%s ", mortag_to_string(val) );
		}
	}
}

void NTree::next_print(FILE* out, int level)
{
	l_print( out );
	if ((static_level>level || static_level==0) && down) down->down_print(out, level+1);
	if (next) next->next_print(out, level);
}

void NTree::down_print(FILE* out, int level)
{
//	if (static_ctrperline>0 && level==1)
//		fprintf(out,"\n%s\t", static_outsecondtiername);

	if ( static_nonumber == 0 )
		fprintf(out, "[%d ", level );
	else
		fprintf(out, "[ " );

	l_print( out );
	if ((static_level>level || static_level==0) && down) down->down_print(out, level+1);
	if (next) next->next_print(out, level);
	if ( static_nonumber == 0 )
		fprintf(out, "%d] ", level);
	else
		fprintf(out, "] " );
}

void NTree::nexttop_print(FILE* out)
{
	if (static_ctrperline>0) {
		fprintf(out,"\n%s\t", static_outsecondtiername);
	}

	if ( static_special == 2 ) {
		static_special = 0;
		l_print( out );
		static_special = 2;
	} else {
		l_print( out );
	}
	if ((static_level>=2 || static_level==0) && down) down->down_print(out, 2);
	if (next) next->nexttop_print(out);
}

void NTree::print(FILE* out)
{
	if (static_ctrperline==0) {
		fprintf(out,"%s\t", static_outtiername);
		if ( static_nonumber == 0 )
			fprintf(out, "[%d ", 1 );
		else
			fprintf(out, "[ " );
	} else {
		fprintf(out,"%s\t", static_outsecondtiername);
	}

	if ( static_special == 2 ) {
		static_special = 0;
		l_print( out );
		static_special = 2;
	} else {
		l_print( out );
	}
	if ((static_level>=2 || static_level==0) && down) down->down_print(out, 2);
	if (next) next->nexttop_print(out);
	if (static_ctrperline==0) {
		if ( static_nonumber == 0 )
			fprintf(out, "%d]", 1 );
		else
			fprintf(out, "]" );
	}

	if ( static_special == 1 ) {
		fprintf(out, " " );
		int sm = static_macrodisplay;
		int snw = static_noword;
		int sl = static_level;
		int snn = static_nonumber;
		static_macrodisplay = 1;
		static_noword = 0;
		static_level = 0;
		static_nonumber = 1;
		l_print( out );
		if (down) down->down_print(out, 2);
		if (next) next->nexttop_print(out);
		int static_macrodisplay = sm;
		int static_noword = snw;
		int static_level = sl;
		int static_nonumber = snn;
		fprintf(out, "\n" );
	} else {
		fprintf(out, "\n" );
	}
}

void constr_morline( char* T, FILE* out)
{
	// T contains all the tier to analyse.
	// first: split it !

#ifdef DBG2
msgfile( stderr, "{%s}\n", T);
#endif

	char* W[MaxNbWords];
	char* Line = (char*)dynalloc(strlen(T)*2);

	int nW = split_with_a( T, W, Line, WhiteSpaces, PunctuationsAtEnd, PunctuationsInside, MaxNbWords, strlen(T)*2 );
	
	// do the job only if utterance lenght constraints do not apply
	
	if (static_snw == -1 && nW-2 > static_nw) return;
	if (static_snw == 1 && nW-2 < static_nw) return;

	mortag* MT = (mortag*)dynalloc(sizeof(mortag)*nW);
	int i;
	for (i=0; i<nW; i++) {
#ifdef DBG0
msgfile( stderr, "%d [%s] ", i+1, W[i] );
#endif
		if ( ! make_mortag( W[i], &(MT[i]) ) ) {
			msg( "out of memory line?\n" );
			dynreset();
			return;
		}
#ifdef DBG0
print_mortag_2( stderr, &(MT[i]) );
msgfile( stderr, "\n" );
#endif
	}

	NTree* full_utterance;
	full_utterance = new NTree;

	// application of macros are done in init()
	full_utterance->init(&MT[1], nW-1);
	for (i = 1; i <= static_last_sec; i++ )
		full_utterance = find_all_constructions(full_utterance, i);
	full_utterance->print(out);

	dynreset();
}

void compute_constr( char* oldfname, FILE* out )
{ // calculate constructions
	fprintf( out, "@CTR: calculation of constructions : (#imbrication=%d)\n", static_level);

	int id = init_input( oldfname, "mor" );
	if (id<0) {
		msg ( "the file %s does not exist or is not available\n", oldfname );
		return;
	}

	long postLineno = 0L;
	int n, y; char* T;
	while ( n = get_tier(id, T, y, postLineno) ) {
		if ( !strnicmp( T, static_tiername, strlen(static_tiername) ) ) {
			fprintf( out, "%s", T );
			constr_morline( T, out);
		} else {
			fprintf( out, "%s", T );
		}
	}
	close_input(id);
}

#ifdef CLANEXTENDED
#define call constr_call
#endif

void call(
#ifndef POSTCODE
			char *oldfname
#endif
				) {
#ifdef POSTCODE
	fclose(fpin);
	fpin = NULL;
#endif
	char fno[MaxFnSize];
	strcpy( fno, oldfname );
	char* p = strrchr( fno, '.' );
	if (!p)
		p = &fno[strlen(fno)];
	strcpy( p, "." );
	strcat( p, static_extname );
	strcat( p, ".cex" );
	if ( static_replace == 2 ) {
	    int n = 0;
#if (defined POSTCODE || defined MacOsx)
		while ( access( fno, 00 ) == 0 )
#else
		while ( _access( fno, 00 ) != -1 )
#endif
        {
			sprintf( p, ".ct%d.cex", n );
			n++;
			if ( n==1000 ) {
				msg ( "Cannot generate a file using the inputfile %s\n", oldfname );
				return;
            }
        }
    }

	FILE* out = fopen( fno, "w" );
	if (!out) {
		msg("Unable to create file %s\n", fno );
		return;
	}

	compute_constr( oldfname, out );
	msg( "From file <%s>\n", oldfname );
	fclose(out);
	if ( static_replace == 1 ) {
		unlink( oldfname );
#ifdef POSTCODE
		rename_each_file(fno, inputname, FALSE);
#else
		rename( fno, oldfname );
#endif
	}
}

#ifdef CLANEXTENDED
void constr_main( int argc, char** argv )
#else
int main( int argc, char** argv )
#endif
{
	char gb_filter_name [256];
	strcpy( gb_filter_name, "" );
	// computation of parameters.
	char constrname[MaxFnSize];
	int i;
	int compile_only = 0;
	static_replace = 0;
	static_level = 0;
	static_macrodisplay = 0;
	static_ctrperline = 0;

	strcpy( constrname, default_constrname );
	strcpy( static_extname, "ctr" );
	strcpy( static_tiername, "%mor:" );
	strcpy( static_outtiername, "%ctr:" );
	strcpy( static_outsecondtiername, "%c01:" );

	if ( argc<2 ) {
		msg("CONSTR: version - %s - INSERM - PARIS - FRANCE\n",CONSTR_VERSION);
		msg("Usage: constr [+rF +sN +tS] filename(s)\n");
		msg("+rF : use rule file F (if omitted, default is %s).\n", default_constrname );
		msg("+lN : choose imbrication depth of display.\n");
		msg("    0- display all imbrications (default).\n");
		msg("    1-9: up to imbrication depth N.\n");
		msg("-word: do not display word information.\n");
		msg("-n  : do not indicate imbrication numbers.\n");
		msg("+mN : macros type of display.\n");
		msg("    0- don't display macros (display tags).\n");
		msg("    1- display macros only.\n");
		msg("    2- display macros and tags.\n");
		msg("+h  : display one construction per line.\n");
		msg("+tF : use the stem tags in file F along with the other syntactic categories.\n");
		msg("+c  : test if rule file is ok (no other processing).\n");
		msg("+iS : include tier code S (default %%mor:).\n");
		msg("+oS : write result as tier S (default %%ctr:).\n");
		msg("-wL : parse utterances of length inferior or equal to L .\n");
		msg("+wL : parse utterances of length superior or equal to L .\n");
		msg("+fS : send output to file (program will derive filename).\n");
		msg("+1  : replace original data file with new one.\n");
		msg("+2  : creates new file names.\n");
		return -1;
	}

	// skip through all parameters to process options.
	for (i=1;i<argc;i++ ) {
		if ( argv[i][0] == '+' || argv[i][0] == '-' ) {
			switch( argv[i][1] ) {
				case 'l' :
					if ( argv[i][2] >= '0' && argv[i][2] <= '9') static_level = argv[i][2] - '0';
					else {
						msg ( "bad parameter after +l: should be 0 up to 9\n" );
					return -1;
					}
					break;
				case 'r' :
					if (argv[i][2] != EOS)
						strcpy( constrname, argv[i]+2 );
					else {
						msg ( "bad parameter after +r: should be a rule file name\n" );
						return -1;
					}
					break;
				case 'm' :
					if ( argv[i][2] >= '0' && argv[i][2] <= '2') static_macrodisplay = argv[i][2] - '0';
					else {
						msg ( "bad parameter after +m: should be 0 up to 2\n" );
						return -1;
					}
					break;
				case 'w' :
				    if ( !stricmp( argv[i], "word" ) ) {
                        static_noword = 1;
                        break;
                    }
					static_nw = atoi(argv[i]+2);
					if (static_nw<1 || static_nw>=99) {
						msg ( "bad parameter after -+w: should be 1 up to 99\n" );
						return -1;
					}
					if ( argv[i][0] == '-' )
					    static_snw = -1;
					else
					    static_snw = 1;
					break;
				case 'i' :
					if (argv[i][2] != EOS) {
						if (argv[i][2]=='@' || argv[i][2]=='%' || argv[i][2]=='*')
							strcpy( static_tiername, argv[i]+2 );
						else {
							static_tiername[0] = '%';
							strcpy( static_tiername+1, argv[i]+2 );
						}
						if (static_tiername[strlen(static_tiername)-1] != ':') {
							strcat(static_tiername,":");
						}
					} else {
						msg ( "bad parameter after +t: should be a tier name\n" );
						return -1;
					}
					break;
				case 'f' :
					if (argv[i][2] != EOS) {
					    strcpy( static_extname, argv[i]+2 );
					} else {
						msg ( "bad parameter after +f: should be file extension name\n" );
						return -1;
					}
					break;
				case 'o' :
					if (argv[i][2] != EOS) {
						if (argv[i][2]=='@' || argv[i][2]=='%' || argv[i][2]=='*')
							strcpy( static_outtiername, argv[i]+2 );
						else {
							static_outtiername[0] = '%';
							strcpy( static_outtiername+1, argv[i]+2 );
						}
						if (static_outtiername[strlen(static_outtiername)-1] != ':') {
							strcat(static_outtiername,":");
						}
					} else {
						msg ( "bad parameter after +t: should be a tier name\n" );
						return -1;
					}
					break;
				case 't' :
					strcpy ( gb_filter_name, argv[i]+2 );
#if (defined POSTCODE || defined MacOsx)
					if ( access(gb_filter_name, 04)) {
#else
					if ( _access(gb_filter_name, 04)) {
#endif
#ifdef POSTCODE
						FILE *tfp;
						tfp=OpenMorLib(gb_filter_name, "r", FALSE, FALSE, NULL);
						if (tfp == NULL)
							tfp= OpenGenLib(gb_filter_name,"r",FALSE,FALSE,NULL);
						if (tfp == NULL) {
#endif
							msg ( "unable to open %s for reading (parameter %s)\n", argv[i]+2, argv[i] );
							return -1;
#ifdef POSTCODE
						} else {
							fclose(tfp);
						}
#endif
					}
					break;
				case '+' :
					if ( argv[i][2] >= '0' && argv[i][2] <= '3') static_special = argv[i][2] - '0';
					else {
						msg ( "bad parameter after ++: should be 0 up to 9\n" );
						return -1;
					}
					break;
				case 'n' :
					static_nonumber = 1;
					break;
				case 'h' :
					static_ctrperline = 1;
					break;
				case '1' :
					static_replace = 1;
					break;
				case '2' :
					static_replace = 2;
					break;
				case 'c' :
					compile_only = 1;
					break;
				default:
					msg ( "unknown parameter %s\n", argv[i] );
					return -1;
					break;
			}
		}
	}

	if (gb_filter_name[0]!='\0')
		gbout_filter_create(gb_filter_name);

	// initialization
	if ( constr_compile( constrname ) != 1) {
		msg( "Stop: error during rules preprocessing.\n");
		return -1;
	}

	if (compile_only) {
		dump_compile();
		free_compile();
		dynfreeall();
		return 0;
	}


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
	gbout_filter_free();
	free_compile();
	dynfreeall();
	return 0;
}
