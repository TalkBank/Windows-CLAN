/**********************************************************************
	"Copyright 1990-2022 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

// megrasp.cpp
//
// MEGRASP, the Maximum Entropy CHILDES parser
// v0.8b
// 
// Kenji Sagae
// Institute for Creative Technologies
// University of Southern California
//
// Original release:
// by Kenji Sagae
// University of Tokyo
// December 2006
//
// Based on childesparser, aka GRASP, which I 
// developed while at Carnegie Mellon University
//
// SSMaxEnt library by Yoshimasa Tsuruoka,
// University of Tokyo
//
// Update history:
//
// 14-12-06: beta release (KS)
// 22-02-07: best-first search (KS)
// 17-03-08: fixed prevact bug (KS)
// 17-03-08: new features (breaks old models, so disabled by default) (KS)
// 17-03-08: added evaluation option (KS)
// 10-07-09: use ksutils for tokenization (KS)
// 10-07-09: changed default tier names (KS)
// 10-07-09: changed default output file name (KS)
// 10-07-09: added multiple file processing (KS)

// To do:
//
// POS tagging


#include <map>
#include <sstream>
#include <queue>
#include <list>
#include "ksutil.h"
#include "blmvm.h"
#include "maxent.h"
#if defined(_MAC_CODE) || defined(_WIN32)
	#include "cu.h"
#elif defined(UNX)
	#include "../cu.h"
#endif

#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
  #if !defined(UNX)
	#define _main megrasp_main
	#define call megrasp_call
	#define getflag megrasp_getflag
	#define init megrasp_init
	#define usage megrasp_usage
  #endif

  #if defined(UNX)
	#include "../mul.h"
  #else
	#include "mul.h" 
  #endif

	#define CHAT_MODE 1
	#define IS_WIN_MODE FALSE

	extern struct tier *defheadtier;
	extern char OverWriteFile;

  #if !defined(UNX)
	extern void megraps_exit(int i);
  #endif
#else
	#define _MAX_PATH	2048
	#define FNSize _MAX_PATH+FILENAME_MAX
	#define UTTLINELEN 18000L
	#define CLAN_MAIN_RETURN int
	FILE *fpin, *fpout;
	char *oldfname;
	char newfname[FNSize];	/* the name of the currently opened file	   */
	char templineC[UTTLINELEN+2];		/* used to be: templine  */
	int outfnameSet;
#endif

#define MY_DEBUG 0
#define THRESHOLD 1e-4

using namespace std;

extern int NUMITER;

char fgets_megrasp_lc;

static char ftime;
static char isFoundDataFile;
static int BEAMSIZE;
static int TRAIN;   
static int MAXLINELEN;
static int eval;
static int heldout; // use how many training instances as held out data?
static int corr;
static int labcorr;
static int tot;
static int inpos;
static int ingra;
static int errnum;
static float ineq;    // inquality parameter for ME training
static char modelname[FNSize]; // the model name
static string graprefix( "" );        // the prefix for output GR lines
static string trainprefix( "" );      // the prefix for input training GR lines
static string posprefix( "" );        // the prefix for POS lines
static ME_Model memod; // The MaxEnt Model

class item {
public:
	
	string word;
	string lemma;
	string cpos;
	string morph;
	string pos;
	string prevpos;
	string nextpos;
	string gr;
	string goldgr;
	int link;
	int nch;
	int nlch;
	int nrch;
	string lchpos;
	string rchpos;
	string lchgr;
	string rchgr;
	int idx;
	int notdone;
	
	double prob;
	
	item() {
		word = "-";
		lemma = "-";
		cpos = "-";
		morph = "-";
		pos = "-";
		prevpos = "O";
		nextpos = "O";
		gr = "-";
		link = 0;
		nch = 0;
		nlch = 0;
		nrch = 0;
		lchpos = "O";
		rchpos = "O";
		lchgr = "O";
		rchgr = "O";
		idx = 0;
		notdone = 0;
		prob = 1.0;
		goldgr = "-";
	};
	
	item( const item &it ) {
		word = it.word;
		lemma = it.lemma;
		cpos = it.cpos;
		morph = it.morph;
		pos = it.pos;
		prevpos = it.prevpos;
		nextpos = it.nextpos;
		gr = it.gr;
		link = it.link;
		nch = it.nch;
		nlch = it.nlch;
		nrch = it.nrch;
		lchpos = it.lchpos;
		rchpos = it.rchpos;
		lchgr = it.lchgr;
		rchgr= it.rchgr;
		idx = it.idx;
		notdone = it.notdone;
		prob = it.prob;
		goldgr = it.goldgr;
	};
};


class parserstate {
public:
	vector<item*> q;
	vector<item*> s;
	
	vector<int> vlink;
	vector<string> vlabel;
	
	double prob;
	string prevact;
	
	parserstate() { prob = 1; prevact = "S"; }
	parserstate( parserstate &ps ) {
		s = ps.s;
		q = ps.q;
		vlink = ps.vlink;
		vlabel = ps.vlabel;
		prob = ps.prob;
		prevact = ps.prevact;
	}
	
	bool operator<( const parserstate & right ) const {
		return prob < right.prob;
	}
	
};

class pact {
public:
	string label;
	double prob;
	
	pact( double p, string lab ) { label = lab; prob = p; }
	bool operator<( const pact & right ) const {
		return prob < right.prob;
	}
};

class prioritize {
public:
	int operator()( const parserstate *p1, const parserstate *p2 ) {
		return p1->prob < p2->prob;
	}
};

static item dummyitem;

static vector<double> me_classify( ME_Model& memod, string& patt )
{
	vector<string> tokens;
	vector<double> res;
	Tokenize( patt, tokens );
	
	ME_Sample sptr;
	int i;
	for( i = 0; i < tokens.size() - 1; i++ ) {
		ostringstream ss;
		ss << i;
		string feat;
		feat = ss.str() + ":" + tokens[i];
		sptr.features.push_back( feat );
		
	}
	
	res = memod.classify( sptr );
	return res;
}

char *fgets_megrasp(char *beg, int size, FILE *fp) {
	register int i = 1;
	register char *buf;

	size--;
	buf = beg;
	*buf = (char)getc(fp);
	if (feof(fp))
		return(NULL);
	do {
		i++;
		if (*buf == '\r') {
			if (fgets_megrasp_lc != '\n') {
				*buf++ = '\n';
				fgets_megrasp_lc = '\r';
				break;
			} else
				i--;
		} else if (*buf == '\n') {
			if (fgets_megrasp_lc != '\r') {
				fgets_megrasp_lc = '\n';
				buf++;
				break;
			} else
				i--;
		} else
			buf++;
		fgets_megrasp_lc = '\0';
		if (i >= size)
			break;
		*buf = (char)getc(fp);
		if (feof(fp))
			break;
	} while (1) ;
	*buf = '\0';
	return(beg);
}

static int me_train( ME_Model& memod, string& patt )  
{
	vector<string> tokens;
	Tokenize( patt, tokens );
	ME_Sample *sptr = new ME_Sample;
	int i;
	for( i = 0; i < tokens.size() - 1; i++ ) {
		ostringstream ss;
		ss << i;
		string feat;
		feat = ss.str() + ":" + tokens[i];
		sptr->features.push_back( feat );
		//fprintf(stdout, "%s\n", feat.c_str());
	}
	//fprintf(stdout, "%s\n", tokens[i].c_str()); 
	sptr->label = tokens[i];
	memod.add_training_sample( *sptr );
	
	delete sptr;
	
	return 0;
}

static int discr( int n ) {
	if( n > 6 ) return 7;
	if( n > 3 ) return 4;
	return n;
}

static item* getstack( vector<item*> &v, int n ) {
	if( n > v.size() ) {
		return &dummyitem;
	}
	
	if( n < 1 ) {
		return &dummyitem;
	}
	
	return v[v.size() - n];
}


// more complete set of features than makefeats, but slower.
// the improvement on childes data seems to be very small
/*
static int makefeats1( vector<item*> &s, vector<item*> &q, 
               string &patt, string prevact ) {
	char tmpstr[1023];
	vector<string> vpatt;
	
	// patt = "- - - - - - - - - - - - - - - - - - - -\n\n";
	
	// vpatt.push_back( getstack( q, getstack( s, 1 )->idx + 0 )->pos );
	// vpatt.push_back( getstack( q, getstack( s, 2 )->idx + 2 )->pos );
	
	vpatt.push_back( getstack( s, 1 )->prevpos );
	vpatt.push_back( getstack( s, 2 )->nextpos );
	
	int dist = getstack( s, 1 )->idx - getstack( s, 2 )->idx;
	int dist2 = getstack( q, 1 )->idx - getstack( s, 1 )->idx;
	
	sprintf( tmpstr, "%d", discr( dist ) );
	vpatt.push_back( tmpstr );
	
	sprintf( tmpstr, "%d", discr( dist2 ) );
	vpatt.push_back( tmpstr );
	
	vpatt.push_back( getstack( s, 1 )->word );  
	vpatt.push_back( getstack( s, 1 )->pos );
	vpatt.push_back( getstack( s, 1 )->lchpos );  
	vpatt.push_back( getstack( s, 1 )->rchpos );
	vpatt.push_back( getstack( s, 1 )->lchgr );  
	vpatt.push_back( getstack( s, 1 )->rchgr );
	
	vpatt.push_back( getstack( s, 2 )->word );  
	vpatt.push_back( getstack( s, 2 )->pos );
	vpatt.push_back( getstack( s, 2 )->lchpos );  
	vpatt.push_back( getstack( s, 2 )->rchpos );
	vpatt.push_back( getstack( s, 2 )->lchgr );  
	vpatt.push_back( getstack( s, 2 )->rchgr );
	
	vpatt.push_back( getstack( s, 3 )->word );  
	vpatt.push_back( getstack( s, 3 )->pos );
	
	vpatt.push_back( getstack( q, 1 )->word );  
	vpatt.push_back( getstack( q, 1 )->pos );
	
	vpatt.push_back( getstack( q, 2 )->word );  
	vpatt.push_back( getstack( q, 2 )->pos );
	
	vpatt.push_back( getstack( q, 3 )->pos );  
	
	vpatt.push_back( getstack( s, 1 )->gr );  
	vpatt.push_back( getstack( s, 2 )->gr );  
	
	int nf = vpatt.size();
	// combine features
	for( int i = 0; i < nf; i++ ) {
		vpatt.push_back( getstack( s, 1 )->pos + vpatt[i] );
		//vpatt.push_back( getstack( s, 2 )->pos + vpatt[i] );
		vpatt.push_back( getstack( q, 1 )->pos + vpatt[i] );
		//vpatt.push_back( getstack( s, 1 )->gr + vpatt[i] );
		//vpatt.push_back( getstack( s, 2 )->gr + vpatt[i] );
		//vpatt.push_back( prevact + vpatt[i] );
	}
	
	vpatt.push_back( getstack( s, 1 )->pos + getstack( s, 2 )->pos +
					getstack( q, 1 )->pos );
	vpatt.push_back( getstack( s, 1 )->pos + getstack( s, 2 )->pos +
					getstack( q, 1 )->pos + getstack( q, 2 )->pos );
	vpatt.push_back( getstack( s, 1 )->pos + getstack( s, 2 )->pos +
					getstack( s, 3 )->pos );
	vpatt.push_back( getstack( s, 1 )->pos + getstack( s, 2 )->pos +
					getstack( s, 3 )->pos + getstack( q, 1 )->pos );
	
	vpatt.push_back( prevact + getstack( s, 1 )->pos );
	vpatt.push_back( prevact + getstack( s, 1 )->pos + getstack( s, 2 )->pos );
	vpatt.push_back( prevact + getstack( q, 1 )->pos );
	vpatt.push_back( prevact + getstack( s, 1 )->pos + getstack( q, 1 )->pos );
	
	patt = "";
	for( int i = 0; i < vpatt.size(); i++ ) {
		patt = patt + vpatt[i];
		patt = patt + " ";
	}
	
	return 0;
}
*/
// a simple set of features.
// probably enough for childes data
static int makefeats( vector<item*> &s, vector<item*> &q, 
			  string &patt, string prevact ) {
	char tmpstr[1023];
	vector<string> vpatt;
	
	// patt = "- - - - - - - - - - - - - - - - - - - -\n\n";
	
	// vpatt.push_back( getstack( q, getstack( s, 1 )->idx + 0 )->pos );
	// vpatt.push_back( getstack( q, getstack( s, 2 )->idx + 2 )->pos );
	
	vpatt.push_back( getstack( s, 1 )->prevpos );
	vpatt.push_back( getstack( s, 2 )->nextpos );
	
	int dist = getstack( s, 1 )->idx - getstack( s, 2 )->idx;
	int dist2 = getstack( q, 1 )->idx - getstack( s, 1 )->idx;
	
	sprintf( tmpstr, "%d", discr( dist ) );
	vpatt.push_back( tmpstr );
	
	sprintf( tmpstr, "%d", discr( dist2 ) );
	vpatt.push_back( tmpstr );
	
	vpatt.push_back( getstack( s, 1 )->word );  
	vpatt.push_back( getstack( s, 1 )->pos );
	vpatt.push_back( getstack( s, 1 )->lchpos );  
	vpatt.push_back( getstack( s, 1 )->rchpos );
	
	vpatt.push_back( getstack( s, 2 )->word );  
	vpatt.push_back( getstack( s, 2 )->pos );
	vpatt.push_back( getstack( s, 2 )->lchpos );  
	vpatt.push_back( getstack( s, 2 )->rchpos );
	
	vpatt.push_back( getstack( s, 3 )->word );  
	vpatt.push_back( getstack( s, 3 )->pos );
	
	vpatt.push_back( getstack( q, 1 )->word );  
	vpatt.push_back( getstack( q, 1 )->pos );
	
	vpatt.push_back( getstack( q, 2 )->word );  
	vpatt.push_back( getstack( q, 2 )->pos );
	
	vpatt.push_back( getstack( q, 3 )->pos );  
	
	vpatt.push_back( prevact );
	
	// vpatt.push_back( "???" );
	
	// vpatt.push_back( "\n\n" );
	
	patt = "";
	for( int i = 0; i < vpatt.size(); i++ ) {
		patt = patt + vpatt[i];
		patt = patt + " ";
	}
	
	// patt = patt + "???\n\n";
	
	return 0;
}

static void CleanUpLine(char *st) {
	int pos;

	for (pos=0; st[pos] != EOS; ) {
		if ((st[pos]==' ' || st[pos]=='\t') && (st[pos+1]==' ' || st[pos+1]=='\t' || st[pos+1]=='\n' || st[pos+1]==EOS))
			strcpy(st+pos, st+pos+1);
		else
			pos++;
	}
}

static void parse( FILE *fpout, vector<item*> &q, ME_Model &memod, string &graprefix, int &corr, int &labcorr, int &tot ) {
	
	vector<parserstate*> stvec;
	vector<item*> itvec;
	
	priority_queue<parserstate*, vector<parserstate*>, prioritize > pq;
	
	parserstate pst;
	vector<item*> goldq = q;
	
	for( int i = 0; i < q.size(); i++ ) {
		// fprintf(fpout, "%s", q[i]->word.c_str()) ;
		
		q[q.size() - 1 - q[i]->link]->notdone++;
		
		pst.vlink.push_back( 0 );
		pst.vlabel.push_back( "NONE" );
		
	}
	
	//  for( int i = q.size() - 1; i > 0; i-- ) {
	for( int i = 0; i < q.size(); i++ ) {
		pst.q.push_back( new item( *(q[i]) ) );
		itvec.push_back( pst.q.back() );
	}
	
	pst.prob = 1;
	
	parserstate *cpst = new parserstate( pst );
	stvec.push_back( cpst );
	
	pq.push( cpst );
	
	while( pq.size() > 0 ) {
		//while( ( q.size() > 1 ) ||
		//	 ( s.size() > 1 ) ) {
		
		///////////////////////////
		priority_queue<parserstate*, vector<parserstate*>, prioritize > pqtmp;
		pqtmp = pq;
		
		pq = std::priority_queue<parserstate*, vector<parserstate*>, prioritize >();
		
		int maxnum = 0;
		while( pqtmp.size() > 0 ) {
			pq.push( pqtmp.top() );
			pqtmp.pop();
			if( maxnum++ > BEAMSIZE ) {
				break;
			}
		}
		///////////////////////////
		
		if( MY_DEBUG ) fprintf(fpout, "Popping...\n\n");
		
		cpst = pq.top();
		pq.pop();
		
		if( ( cpst->s.size() == 1 ) &&
		   ( cpst->q.size() == 1 ) ) {
			// we found a complete parse
			break;
		}
		
		
		// prepare features for classifier
		string patt;
		makefeats( cpst->s, cpst->q, patt, cpst->prevact );
		
		if( MY_DEBUG ) fprintf(fpout, "%s", patt.c_str());
		
		string res;
		vector<string> tmpvec;
		vector<double> vp;
		string act;
		int actnum = 1;
		
		if( TRAIN ) {
			actnum = 1;
			
			if( cpst->s.size() > 1 ) {
				
				if( ( getstack( cpst->s, 1 )->link == getstack( cpst->s, 2 )->idx ) &&
				   !( getstack( cpst->s, 1 )->notdone ) ) {
					act = "L:" + getstack( cpst->s, 1 )->goldgr;
				}
				else if( ( getstack( cpst->s, 2 )->link == getstack( cpst->s, 1 )->idx ) &&
						!( getstack( cpst->s, 2 )->notdone ) ) {
					act = "R:" + getstack( cpst->s, 2 )->goldgr;
				}
				else {
					act = "S";
				}
				
				patt = patt + act;
				// fprintf(fpout, "%s", patt.c_str());
				me_train( memod, patt );
				
				vp.push_back( 1.0 );
			}
			else {
				act = "S";
				vp.push_back( 1.0 );
			}
			
		}
		else {
			patt = patt + "???\n";
			
			// classify action
			// res = p.parse( patt.c_str() );
			vp = me_classify( memod, patt );
			
			actnum = memod.num_classes();
		}
		
		if( MY_DEBUG ) fprintf(fpout, "\n\n---------------------------\n\n");
		
		priority_queue<pact> actpq;
		for( int i = 0; i < actnum; i++ ) {
			
			if( !TRAIN ) {
				act = memod.get_class_label( i );
			}
			
			if( ( cpst->s.size() < 2 ) && ( act != "S" ) ) {
				continue;
			}
			
			if( vp[i] < THRESHOLD ) {
				continue;
			}
			
			if( MY_DEBUG ) fprintf(fpout, "%lf  ---  %s\n", vp[i], act.c_str());
			
			parserstate *pstate = new parserstate( *cpst );
			stvec.push_back( pstate );
			
			// check left
			if( act[0] == 'L' ) {
				string label = act.substr( 2, act.size() - 1 );
				
				item *newitptr = new item;
				itvec.push_back( newitptr );
				
				newitptr->word = cpst->s[cpst->s.size() - 2]->word;
				newitptr->pos = cpst->s[cpst->s.size() - 2]->pos;
				newitptr->prevpos = cpst->s[cpst->s.size() - 2]->prevpos;
				newitptr->nextpos = cpst->s[cpst->s.size() - 2]->nextpos;
				newitptr->gr = label;
				newitptr->goldgr = cpst->s[cpst->s.size() - 2]->goldgr;
				newitptr->link = cpst->s[cpst->s.size() - 2]->link;
				newitptr->nch = cpst->s[cpst->s.size() - 2]->nch + 1;
				newitptr->nlch = cpst->s[cpst->s.size() - 2]->nlch + 1;
				newitptr->nrch = cpst->s[cpst->s.size() - 2]->nrch;
				newitptr->lchpos = cpst->s[cpst->s.size() - 1]->pos;
				newitptr->rchpos = cpst->s[cpst->s.size() - 2]->rchpos;
				newitptr->lchgr = label;
				newitptr->rchgr = cpst->s[cpst->s.size() - 2]->rchpos;	
				newitptr->idx = cpst->s[cpst->s.size() - 2]->idx;
				newitptr->notdone = cpst->s[cpst->s.size() - 2]->notdone - 1;
				
				pstate->prevact = act;
				pstate->vlink[cpst->s[cpst->s.size() - 1]->idx] = cpst->s[cpst->s.size() - 2]->idx;
				pstate->vlabel[cpst->s[cpst->s.size() - 1]->idx] = label;
				
				pstate->s.pop_back();
				pstate->s.pop_back();
				
				pstate->s.push_back( newitptr );
				pstate->prob *= vp[i];
				
				pq.push( pstate );
				
				continue;
			}
			
			// check right
			if( act[0] == 'R' ) {
				string label = act.substr( 2, act.size() - 1 );
				
				item *newitptr = new item;
				itvec.push_back( newitptr );
				
				newitptr->word = cpst->s[cpst->s.size() - 1]->word;
				newitptr->pos = cpst->s[cpst->s.size() - 1]->pos;
				newitptr->prevpos = cpst->s[cpst->s.size() - 1]->prevpos;
				newitptr->nextpos = cpst->s[cpst->s.size() - 1]->nextpos;
				newitptr->gr = label;
				newitptr->goldgr = cpst->s[cpst->s.size() - 1]->goldgr;
				newitptr->link = cpst->s[cpst->s.size() - 1]->link;
				newitptr->nch = cpst->s[cpst->s.size() - 1]->nch + 1;
				newitptr->nlch = cpst->s[cpst->s.size() - 1]->nlch;
				newitptr->nrch = cpst->s[cpst->s.size() - 1]->nrch;
				newitptr->lchpos = cpst->s[cpst->s.size() - 1]->lchpos;
				newitptr->rchpos = cpst->s[cpst->s.size() - 2]->pos;
				newitptr->lchgr = cpst->s[cpst->s.size() - 1]->lchpos;
				newitptr->rchgr = label;
				newitptr->idx = cpst->s[cpst->s.size() - 1]->idx;
				newitptr->notdone = cpst->s[cpst->s.size() - 1]->notdone - 1;
				
				pstate->prevact = act;
				pstate->vlink[cpst->s[cpst->s.size() - 2]->idx] = cpst->s[cpst->s.size() - 1]->idx;
				pstate->vlabel[cpst->s[cpst->s.size() - 2]->idx] = label;
				
				pstate->s.pop_back();
				pstate->s.pop_back();
				
				pstate->s.push_back( newitptr );
				pstate->prob *= vp[i];
				
				pq.push( pstate );
				
				continue;
			}
			
			if( cpst->q.size() > 1 ) {
				pstate->prevact = "S";
				item *newitptr = pstate->q.back();
				pstate->q.pop_back();
				
				pstate->s.push_back( newitptr );
				pstate->prob *= vp[i];
				
				pq.push( pstate );
				continue;
			}
		} // for i = 0 to memod.num_classes()
	} // while q or s
	
	// print out GRs
	if( !TRAIN ) {
		string outstr;
		int outlen = 8;
		goldq.resize( cpst->vlabel.size() );
		int goldn = goldq.size();
		
		outstr = graprefix + "\t";
		
		for( int i = 1; i < q.size() - 1; i++ ) {
			string idxstr = toString( i );
			string linkstr = toString( cpst->vlink[i] );
			
			outlen += idxstr.length() + linkstr.length() + 
			cpst->vlabel[i].length() + 3;
			
			if( MAXLINELEN && ( outlen > MAXLINELEN ) ) {
				outstr = outstr + "\n\t";
				outlen = outlen - ( MAXLINELEN - 8 );
			}
			
			outstr = outstr + idxstr + "|" + linkstr + "|" + cpst->vlabel[i] + " ";
			if( goldq[goldn - i - 1]->link == cpst->vlink[i] ) {
				corr++;
				if( goldq[goldn -i - 1]->goldgr == cpst->vlabel[i] ) {
					labcorr++;
				}
			}
			tot++;
		}
		strncpy(templineC1, outstr.c_str(), UTTLINELEN);
		templineC1[UTTLINELEN] = EOS;
		uS.remblanks(templineC1);
		CleanUpLine(templineC1);
		fprintf(fpout, "%s\n", templineC1);
	}
	
	// free all remaining items
	for( int i = 0; i < stvec.size(); i++ ) {
		delete stvec[i];
	}
	
	for( int i = 0; i < itvec.size(); i++ ) {
		delete itvec[i];  
	}
	
}

static int parseFile( FILE *fpin, FILE *fpout, ME_Model &memod, string &posprefix,
			  string &graprefix, string &trainprefix, int &corr, 
			  int &labcorr, int &tot, int eval ) {

	// the main part of the work starts here
	char res, isOldGRA;
	char isPostCodeExclude, isSpTier;
	char isMorTier; //2019-05-01
	int  postRes;
	string str;
	string origstr;
// 2009-07-30	vector<string> sent;
// 2009-07-30	vector<item*> q;
	string posline;
	string graline;
	long linenum = 0;
	int uttnum = 0;

	spareTier1[0] = EOS;
	posline = "";
	graline = "";
	// read a line
	isSpTier = FALSE;
	isMorTier = FALSE; //2019-05-01
	isPostCodeExclude = TRUE;
	fgets_megrasp_lc = '\0';
	if( !fgets_megrasp(templineC, UTTLINELEN, fpin) ) {
		return 0;
	}
	str = templineC;
	res = 1;
	isOldGRA = 0;
	// main loop
	do {
		// increase line counter
		linenum++;
#if defined(_MAC_CODE) || defined(_WIN32)
		if (linenum % 2000 == 0) { // PERIOD
// lxs 2019-03-15 if (!isRecursive)
				fprintf(stderr,"\r%d ",linenum);
			my_flush_chr();
			if (isKillProgram)
				megraps_exit(0);
		}
#endif
		if (strncmp(templineC, "%gra:", 5) == 0) {
			isMorTier = FALSE; //2019-05-01
			if (!isPostCodeExclude)
				isOldGRA = 1;
			else
				isOldGRA = 0;
			if (isSpTier) {
				if (!checktier(spareTier1)) {
					isPostCodeExclude = TRUE;
				} else {
					postRes = isPostCodeFound("*", spareTier1);
					isPostCodeExclude = (postRes == 5 || postRes == 1);
				}
			}
			isSpTier = FALSE;
		} else if (templineC[0] == '@' || templineC[0] == '%') {
			if (strncmp(templineC, "%mor:", 5) == 0) //2019-05-01
				isMorTier = TRUE;
			else
				isMorTier = FALSE;
			if (isSpTier) {
				if (!checktier(spareTier1)) {
					isPostCodeExclude = TRUE;
				} else {
					postRes = isPostCodeFound("*", spareTier1);
					isPostCodeExclude = (postRes == 5 || postRes == 1);
				}
			}
			isOldGRA = 0;
			isSpTier = FALSE;
		} else if (templineC[0] == '*') {
			isMorTier = FALSE; //2019-05-01
			isOldGRA = 0;
			isSpTier = TRUE;
			strcpy(spareTier1, templineC);
		} else if (isSpTier) {
			strcat(spareTier1, templineC);
		}

		// remove new line and carriage return characters
		if( str.find( "\r", 0 ) != string::npos ) {
			str.erase( str.find( "\r", 0 ), 1 );
		}
		
		if( str.find( "\n", 0 ) != string::npos ) {
			str.erase( str.find( "\n", 0 ), 1 );
		}

		// save the original line for printing later
		origstr = str;

		// remove material in square brackets
		int sp1 = 0;
		int sp2 = str.length() - 1;
		while( ( sp1 = str.find( "[", 0 ) ) != string::npos ) {
			sp2 = str.find( "]", sp1 );
			str.erase( sp1, sp2 - sp1 + 1 );
		}

		if (isMorTier) { //2019-05-01 convert 0word to word
			while( ( sp1 = str.find( " 0", 0 ) ) != string::npos ) {
				str.erase( sp1+1, 1 );
			}
			while( ( sp1 = str.find( "\t0", 0 ) ) != string::npos ) {
				str.erase( sp1+1, 1 );
			}
		}

		// make contractions into two separate tokens
		while( str.find( "$", 0 ) != string::npos ) {
			str[str.find( "$", 0 )] = ' ';
		}

		// make contractions into two separate tokens
		while( str.find( "~", 0 ) != string::npos ) {
			str[str.find( "~", 0 )] = ' ';
		}

		// is this a %MOR line?
		if( !isPostCodeExclude && str.substr( 0, posprefix.length() ) == posprefix ) {
			uttnum++;
			str = str.substr( posprefix.length() + 1, str.size() - ( posprefix.length() + 1 ) );
			inpos = 1;
			if( ingra ) {
				ingra = -1;
			}
			posline = str;
		}
		// or a %GRT line?
		else if( !isPostCodeExclude && str.substr( 0, trainprefix.length() ) == trainprefix ) {
			
			str = str.substr( trainprefix.length() + 1, str.size() - 
							 ( trainprefix.length() + 1 ) );
			ingra = 1;
			if( inpos ) {
				inpos = -1;
			}
			graline = str;
		} else {
			if (isPostCodeExclude) {
			} else if( str[0] == '\t' ) {
				str = str.substr( 1, str.size() - 1 );
				if( inpos > 0 ) {
					inpos++;
					posline = posline + " " + str;
				}
				if( ingra > 0 ) {
					ingra++;
					graline = graline + " " + str;
				}
			} else {
				if( inpos ) {
					inpos = -1;
				}
				if( ingra ) {
					ingra = -1;
				}
				if( ( inpos && !TRAIN ) || ( inpos && ingra ) ) {
					//( str[0] == '*' ) ||
					//( str[0] == '@' ) ) {
					inpos = 0;
					ingra = 0;
					if( posline != "" ) {
/* 2018-08-09
						// change possessive 's into a separate token
						while( posline.find( "-POSS", 0 ) != string::npos ) {
							posline.replace( posline.find( "-POSS", 0 ), 5, " poss|s" );
						}
*/
						// process the sentence
						string word;
						string pos;
						string gr;
						string linkstr;
						int link;
						
						vector<string> tokens;
						vector<string> gratokens;
						
						vector<item*> q;
						
						Tokenize( posline, tokens );
						if( TRAIN || eval ) {
							Tokenize( graline, gratokens );
							
							if( tokens.size() != gratokens.size() ) {
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
								fputc('\r', stderr);
#endif
								fputs("----------------------------------------\n",stderr);
								fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, linenum);
								fprintf(stderr, "Error in utterance number %d\n", uttnum);
								fprintf(stderr, "%s\n", posline.c_str());
								fprintf(stderr, "%s\n", graline.c_str());
								fprintf(stderr, "Skipping this utterance.\n\n");
								
								errnum++;
								
								continue;
							}
						}
						
						q.push_back( new item );
						q.back()->word = "RightWall";
						q.back()->pos = "RW";
						q.back()->idx = tokens.size() + 1;
						q.back()->link = 0;
						q.back()->notdone = 0;
						
						int skipsent = 0;
						for( int i = tokens.size() - 1; i >= 0; i-- ) {
							if( tokens[i].find( "|", 0 ) == string::npos ) {
								word = tokens[i];
								pos = "punct";
							}
							else {
								
								pos = tokens[i].substr( 0, tokens[i].find( "|", 0 ) );
								word = tokens[i].substr( tokens[i].find( "|", 0 ) + 1, tokens[i].size() - 1 );
							}
							
							if( TRAIN || eval ) {
								
								int sep1 = gratokens[i].find( "|", 0 );
								int sep2 = gratokens[i].find( "|", sep1 + 1 );
								
								linkstr = gratokens[i].substr( sep1 + 1, sep2 - ( sep1 + 1 ) );
								gr = gratokens[i].substr( sep2 + 1, gratokens[i].size() - 1 - sep2 );
								link = atoi( linkstr.c_str() );
								
								if( link >= gratokens.size() ) {
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
									fputc('\r', stderr);
#endif
									fputs("----------------------------------------\n",stderr);
									fprintf(stderr, "*** File \"%s\": line %ld.\n", oldfname, linenum);
									fprintf(stderr, "Error in utterance number %d\n", uttnum);
									fprintf(stderr, "%s\n", posline.c_str());
									fprintf(stderr, "%s\n", graline.c_str());
									fprintf(stderr, "Skipping this utterance.\n\n");
									
									errnum++;
									
									skipsent = 1;
									break;
								}
							}
							else {
								link = 0;
								gr = "NOGR";
							}
							
							q.push_back( new item );
							q.back()->word = word;
							q.back()->pos = pos;
							q.back()->idx = i + 1;
							
							q.back()->link = link;
							q.back()->goldgr = gr;
						}
						
						if( skipsent ) {
							while( q.size() ) {
								delete q.back();
								q.pop_back();
							}
							
							continue;
						}
						
						q.push_back( new item );
						q.back()->word = "LeftWall";
						q.back()->pos = "LW";
						q.back()->idx = 0;
						q.back()->link = 0;
						q.back()->notdone = 0;
						
						for( int i = 1; i < q.size(); i++ ) {
							q[i]->nextpos = q[i-1]->pos;
						}
						
						for( int i = q.size() - 2; i > 0; i-- ) {
							q[i]->prevpos = q[i+1]->pos;
						}
						
						// parse
						parse( fpout, q, memod, graprefix, corr, labcorr, tot );
						// free all remaining items
						for( int i = 0; i < q.size(); i++ ) {
							delete q[i];
						}
					}
				}
			}
		}
		if( !TRAIN ) {
			//if( !ingra ) {
			if (!isOldGRA) {
				fprintf(fpout, "%s\n", origstr.c_str());
			}
			//}
			origstr = "";
		}
		if (res == 0)
			str = "";
		else {
			if(!fgets_megrasp(templineC, UTTLINELEN, fpin) ) {
				res = 0;
				str = "";
			} else {
				str = templineC;
			}
		}
		
	} while( res || ( inpos && ingra ) );
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	fputc('\r', stderr);
  #if !defined(UNX)
	my_flush_chr();
	if (isKillProgram)
		megraps_exit(0);
  #endif
#endif
	if( TRAIN ) {
		fprintf(stderr, "Finished processing file with %d and %d errors.\n", uttnum, errnum);
	}
	
	return 0;
}

void usage() {
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	fprintf(stderr, "Usage: megrasp [e gS GS pS mS t iN cN %s] filename(s)\n", mainflgs());
#else	
	fprintf(stderr, "Usage: megrasp [-e] [-gStr] [-GStr] [-pStr] [-mStr] [-LInt] [-t [-iInt] [-cFloat]] inputfile\n\n");
	fprintf(stderr, "Command line options:\n\n");
#endif
	fprintf(stderr, "-e : evaluate accuracy (input file must contain gold standard GRs)\n");
	fprintf(stderr, "-gS: prefix for output lines, must be three characters (default: gra)\n");
	fprintf(stderr, "-GS: prefix for GR training lines, must be three characters (default: grt)\n");
	fprintf(stderr, "-pS: prefix for POS lines, must be three characters (default: mor)\n");
	fprintf(stderr, "-mS: model name (default: megrasp.mod)\n");
#ifdef UNX
	fprintf(stderr, "+LF: specify full path of the folder with \"megrasp.mod\" file\n");
#endif
#if !defined(_MAC_CODE) && !defined(_WIN32) && !defined(UNX)
	fprintf(stderr, "-oS: output file name (default: input file name + .txt)\n");
	fprintf(stderr, "-LN: number of characters before output line wraps (default: 80)\n");
#endif
	fprintf(stderr, "-t : training mode (parser runs in parse mode by default)\n");
	fprintf(stderr, "-iN: number of iterations for training ME model (default: 300)\n");
	fprintf(stderr, "-cN: inequality parameter for training ME model (default: 0.5).\n");
#if !defined(_MAC_CODE) && !defined(_WIN32) && !defined(UNX)
	fputc('\n', stderr);
	exit(0);
#else
	mainusage(TRUE);
#endif
}

void init(char f) {
	if (f) {
		ftime = 1;
		addword('\0','\0',"+[- *]");
		isFoundDataFile = 0;
		heldout = 0; // use how many training instances as held out data?
		BEAMSIZE = 5;
		TRAIN = 0; // if this global flag is on, we train the parser
		NUMITER = 300;   // extern int that sets the number of training iterations
		MAXLINELEN = 80;
		corr = 0;
		labcorr = 0;
		tot = 0;
		eval = 0;
		ineq = 1.0;    // inquality parameter for ME training
// initialize global variables: dummyitem and prevact
		inpos = 0;
		ingra = 0;
		errnum = 0;
		dummyitem.word = "-1";
		dummyitem.pos = "-1";
		dummyitem.prevpos = "-1";
		dummyitem.nextpos = "-1";
		dummyitem.gr = "-1";
		dummyitem.link = -1;
		dummyitem.nch = -1;
		dummyitem.nlch = -1;
		dummyitem.nrch = -1;
		dummyitem.lchpos = "-1";
		dummyitem.rchpos = "-1";
		dummyitem.idx = -1;
		dummyitem.notdone = 0;
// initialization done.
		strcpy(modelname, "megrasp.mod"); // the model name
		graprefix = "%gra:";        // the prefix for output GR lines
		trainprefix = "%grt:";      // the prefix for input training GR lines
		posprefix = "%mor:";        // the prefix for POS lines
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
		onlydata = 1;
		OverWriteFile = TRUE;
		stout = FALSE;
		if (defheadtier) {
			if (defheadtier->nexttier != NULL)
				free(defheadtier->nexttier);
			free(defheadtier);
			defheadtier = NULL;
		}
#else
		outfnameSet = 0;
#endif
	} else {
		if (!TRAIN) {
			inpos = 0;
			ingra = 0;
			errnum = 0;
// initialize global variables: dummyitem and prevact
			dummyitem.word = "-1";
			dummyitem.pos = "-1";
			dummyitem.prevpos = "-1";
			dummyitem.nextpos = "-1";
			dummyitem.gr = "-1";
			dummyitem.link = -1;
			dummyitem.nch = -1;
			dummyitem.nlch = -1;
			dummyitem.nrch = -1;
			dummyitem.lchpos = "-1";
			dummyitem.rchpos = "-1";
			dummyitem.idx = -1;
			dummyitem.notdone = 0;
		}
		if (ftime) {
			ftime = 0;
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
			if (TRAIN)
				stout = TRUE;
#endif
			// either we initialize the model for training,
			// or we load it from a file for parsing
			if( TRAIN ) {
				memod.set_heldout( heldout );
			} else {
				if( !memod.load_from_file( modelname ) ) {
#if defined(_MAC_CODE) || defined(_WIN32)
					megraps_exit( 0 );
#else
					exit(1);
#endif
				}
			}
		}
	}
}

void getflag(char *f, char *f1, int *i) {
	f++;
	switch(*f++) {
		case 'e':
			eval = 1;
			break;
		case 'i':
			NUMITER = atoi( f );
			break;
		case 'b':
			BEAMSIZE = atoi( f );
			break;
#if !defined(_MAC_CODE) && !defined(_WIN32) && !defined(UNX)
		case 'L':
			MAXLINELEN = atoi( f );
			break;
		case 'o':
			outfnameSet = 1;
			strcpy(newfname, f);
			break;
#endif
#ifdef UNX
		case 'L':
			int j;
			if (*f == '/')
				strcpy(mor_lib_dir, f);
			else {
				strcpy(mor_lib_dir, wd_dir);
				addFilename2Path(mor_lib_dir, f);
			}
			j = strlen(mor_lib_dir);
			if (j > 0 && mor_lib_dir[j-1] != '/')
				strcat(mor_lib_dir, "/");
			break;
#endif
		case 'c':
			ineq = atof( f );
			break; 
		case 'm':
			strcpy(modelname, f);
			break;
		case 'g':
			if (*f == '%')
				graprefix = (string)f + ":";
			else
				graprefix = (string)"%" + f + ":";
			if( graprefix.length() > 27 ) {
				fprintf(stderr, "Line prefix is too long, must be 25 characters or fewer.\n");
#if defined(_MAC_CODE) || defined(_WIN32)
				megraps_exit( 0 );
#else
				exit(1);
#endif
			}
			break;
		case 'G':
			if (*f == '%')
				trainprefix = (string)f + ":";
			else
				trainprefix = (string)"%" + f + ":";
			if( trainprefix.length() > 27 ) {
				fprintf(stderr, "Line prefix is too long, must be 25 characters or fewer.\n");
#if defined(_MAC_CODE) || defined(_WIN32)
				megraps_exit( 0 );
#else
				exit(1);
#endif
			}
			break;
		case 'p':
			if (*f != '1') {
				if (*f == '%')
					posprefix = (string)f + ":";
				else
					posprefix = (string)"%" + f + ":";
				if( posprefix.length() > 27 ) {
					fprintf(stderr, "Line prefix is too long, must be 25 characters or fewer.\n");
#if defined(_MAC_CODE) || defined(_WIN32)
					megraps_exit( 0 );
#else
					exit(1);
#endif
				}
			}
			break;
		case 't':
			if (*f == EOS)
				TRAIN = 1;
			else
				maingetflag(f-2,f1,i);
			break;
#if !defined(_MAC_CODE) && !defined(_WIN32) && !defined(UNX)
		case 'h':
			usage();
			break;
#else
		case 's':
			if (*f == '[' && *(f+1) == '-') {
				maingetflag(f-2,f1,i);
/* enabled 2017-12-04, disabled 2018-04-04
			} else if (*f == '[' && *(f+1) == '+') {
				maingetflag(f-2,f1,i);
*/
			} else {
				fprintf(stderr, "Please specify only language codes, \"[- ...]\", with +/-s option.\n");
				cutt_exit(0);
			}
			break;
		default:
			maingetflag(f-2,f1,i);
			break;
#endif
	}
}

#if defined(_MAC_CODE) || defined(_WIN32)
void megraps_exit(int i) {
	memod.allClear();
	cutt_exit(i);
}
#endif

void call() {
	isFoundDataFile = 1;
	parseFile( fpin, fpout, memod, posprefix, graprefix, trainprefix, corr, labcorr, tot, eval );
}

// Command line options
//
// -g str: prefix for output lines (default: %gra)
// -G str: prefix for GR training lines (default: %grt)
// -p str: prefix for POS lines (default: %mor)
// -m str: model name (default: megrasp.mod)
// -o str: output file name (default: input file name + .txt)
// -t : training mode (parser runs in parse mode by default)
// -i int: number of iterations for training ME model (default: 300)
// -c float: inequality parameter for training ME model (default: 1.0)
// -L int: length of output lines before line break (default: 80)
// -b int: size of the search beam (default: 5)
// -e: evaluation mode


CLAN_MAIN_RETURN main(int argc, char *argv[]) {
#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	isWinMode = IS_WIN_MODE;
	chatmode = CHAT_MODE;
	CLAN_PROG_NUM = MEGRASP;
	OnlydataLimit = 0;
	UttlineEqUtterance = FALSE;
	replaceFile = TRUE;
	bmain(argc,argv,NULL);
	if (isKillProgram) {
  #if defined(_MAC_CODE) || defined(_WIN32)
		megraps_exit( 0 );
  #else
		exit(1);
  #endif
	}
#else
	int i;

	init(1);
	if ( argc<2 ) {
		usage();
	}
	for (i=1; i < argc; i++ ) {
		if (argv[i][0] == '+' ||  argv[i][0] == '-') {
			getflag(argv[i], NULL, NULL);
		}
	}
	init(0);
	if( outfnameSet ) {
		fpout = fopen( newfname, "w" );
		if( fpout == NULL ) {
			fprintf(stderr, "Error opening output file %s\n", newfname);
			exit( 1 );
		}
	}
	for (i=1; i < argc; i++ ) {
		if (argv[i][0] != '+' &&  argv[i][0] != '-') {
			init(0);
			oldfname = argv[i];
			fpin = fopen(oldfname, "r" );
			if( fpin == NULL ) {
				fprintf(stderr, "File not found: %s\n", oldfname);
				exit( 1 );
			}
			if( !outfnameSet ) {
				strcpy(newfname, oldfname);
				strcat(newfname, ".txt");
				fpout = fopen( newfname, "w" );
				if( fpout == NULL ) {
					fprintf(stderr, "Error opening output file %s\n", newfname);
					exit( 1 );
				}
			}
			call();
			if( fpin != NULL ) {
				fclose(fpin);
			}
			if( !outfnameSet && fpout != NULL && fpout != stdout) {
				fclose(fpout);
			}
		}
	}
	if( outfnameSet  && fpout != NULL  && fpout != stdout) {
		fclose(fpout);
	}
	if( !isFoundDataFile ) {
		fprintf(stderr, "No input file given.\n");
		exit( 1 );
	}
#endif
	if (isFoundDataFile) {
		if( TRAIN ) {
			memod.train( 0, 0, ineq );
			memod.save_to_file( modelname );
		}
		
		if( eval ) {
			double acc = (double)corr / tot;
			double labacc = (double)labcorr / tot;
			fprintf(stderr, "Number of tokens: %d\n", tot);
			fprintf(stderr, "Unlabeled accuracy: %lf\n", acc); 
			fprintf(stderr, "Labeled accuracy: %lf\n", labacc); 
		}
	}
#if defined(_MAC_CODE) || defined(_WIN32)
	memod.allClear();
#endif
}
