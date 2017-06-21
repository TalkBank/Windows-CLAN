/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/* ContextRL.cpp - contextual-rule-learn */
/* Copyright @ 1993 MIT and University of Pennsylvania*/
/* Written by Eric Brill */
/* interfaced with POSTTRAIN - Christophe Parisse */

#include "system.hpp"

#include "database.hpp"
#include "darray.hpp"
#include "registry.hpp"
#include "memory.hpp"

static char AnyWordS[256+1];
static int funcmp_reverse(char *s, char *t)
{
    return - strcmp(s,t);
}

/* old 20-03-03
static void ssort( char** L, int nL, int reverse )
{
    int (*funcmp)(char *s,char *t) = (int (*)(char*,char*))strcmp;
    if ( reverse ) funcmp = funcmp_reverse;
    register int  gap, i, j;
    char *sw;

    for (gap=nL/2; gap>0; gap/=2)
       for (i=gap; i<nL; i++)
          for (j=i-gap; j>=0; j-=gap) {
             if ( (*funcmp)( L[j], L[j+gap]) <= 0)
                  break;
                sw = L[j];
                L[j] = L[j+gap];
                L[j+gap] = sw;
          }
}
*/

static void ssort( char** L, int nL, int reverse )
{
	register int  gap, i, j, res;
	char *sw;

	for (gap=nL/2; gap>0; gap/=2)
		for (i=gap; i<nL; i++)
			for (j=i-gap; j>=0; j-=gap) {
				if ( reverse )
					res = funcmp_reverse( L[j], L[j+gap]);
				else
					res = strcmp( L[j], L[j+gap]);
				if ( res <= 0)
					break;
				sw = L[j];
				L[j] = L[j+gap];
				L[j+gap] = sw;
			}
}

static char *mystrdup(char* thestr) 
{

  return strcpy((char *)malloc(strlen(thestr)+1),thestr);
}

/* NUMTAGS and NUMWDS are (roughly) guesses of the max number of words
or tags that could appear to the right of a particular tag */

#define NUMTAGS 100  
#define NUMWDS  300

void implement_change( char *thearg, char **thetagcorpus, char **thewordcorpus, int corpuslen );
int  localbest;
char localbestthing[100];
char *wrong,*right;
char *word_to_check;
char *tempforcreate;

/********************************************************************/
static void increment_array( Registry *thehash, char  *thestr )
{
	int *numforhash;
	
	
	if ((numforhash = (int *)Registry_get(*thehash,thestr)) != NULL) {
		(*numforhash)++; 
	}
	else {
		numforhash = (int *)malloc(sizeof(int));
		*numforhash = 1;
		Registry_add(*thehash, thestr, (char *)numforhash); 
	}
}
/***********************************************************************/
static void increment_array_create( Registry *thehash, char  *thestr )
{
	int *numforhash;
	
	
	if ((numforhash = (int *)Registry_get(*thehash,thestr)) != NULL) {
		(*numforhash)++; 
	}
	else {
		numforhash = (int *)malloc(sizeof(int));
		*numforhash = 1;
		tempforcreate = (char*)malloc((1+strlen(thestr))*sizeof(char));
		strcpy(tempforcreate,thestr);
		Registry_add(*thehash,tempforcreate,(char *)numforhash); 
	}
}
/***********************************************************************/
static void decrement_array( Registry *thehash, char  *thestr )
{
	int *numforhash;
	
	
	if ((numforhash = (int *)Registry_get(*thehash,thestr)) != NULL) {
		(*numforhash)--; 
	}
	else {
		numforhash = (int *)malloc(sizeof(int));
		*numforhash = -1;
		Registry_add(*thehash,thestr, (char *)numforhash); 
	}
}
/***********************************************************************/
static void decrement_array_create( Registry *thehash, char  *thestr )
{
	int *numforhash;
	
	
	if ((numforhash = (int *)Registry_get(*thehash,thestr)) != NULL) {
		(*numforhash)--; 
	}
	else {
		numforhash = (int *)malloc(sizeof(int));
		*numforhash = -1;
		tempforcreate = (char*)malloc((1+strlen(thestr))*sizeof(char));
		strcpy(tempforcreate,thestr);
		Registry_add(*thehash,tempforcreate,(char *)numforhash); 
	}
}
/***********************************************************************/
static void check_counts( Registry *goodhash, const char *label )
{
	Darray tempkey,tempval;
	int tempcount,tempbest;
	int FREEFLAG;
	
	FREEFLAG = 0;
	
	if (strcmp(label,"PREVBIGRAM") == 0 ||
		strcmp(label,"NEXTBIGRAM") == 0 ||
		strcmp(label,"WDPREVTAG") == 0 ||
		strcmp(label,"WDNEXTTAG") == 0 ||
		strcmp(label,"WDANDTAG") == 0 ||
		strcmp(label,"WDAND2AFT") == 0 ||
		strcmp(label,"WDAND2BFR") == 0 ||
		strcmp(label,"WDAND2TAGAFT") == 0 ||
		strcmp(label,"WDAND2TAGBFR") == 0 ||
		strcmp(label,"RBIGRAM") == 0 ||
		strcmp(label,"LBIGRAM") == 0 ||
		strcmp(label,"SURROUNDTAG") == 0) 
		FREEFLAG = 1;
	
	tempkey = Darray_create();
	Darray_hint(tempkey,10,Registry_entry_count(*goodhash));
	tempval = Darray_create();
	Darray_hint(tempval,10,Registry_entry_count(*goodhash));
	
	Registry_fetch_contents(*goodhash,tempkey,tempval);
	for (tempcount=0;tempcount<Darray_len(tempkey);++tempcount) {
		if ( (tempbest = *(int *)Darray_get(tempval,tempcount)) > localbest) {
			localbest = tempbest;
			sprintf(localbestthing,"%s %s %s %s %s", word_to_check, wrong, right, label, (char *)Darray_get(tempkey,tempcount)); 
		}
		free(Darray_get(tempval,tempcount));
		if (FREEFLAG)
			free(Darray_get(tempkey,tempcount));
	}
	Darray_destroy(tempval);
	Darray_destroy(tempkey);
	Registry_destroy(*goodhash);
}

static void free_counts( Registry *goodhash, const char *label )
{
	Darray tempkey,tempval;
	int tempcount;
	int FREEFLAG;
	
	FREEFLAG = 0;
	
	if (strcmp(label,"PREVBIGRAM") == 0 ||
		strcmp(label,"NEXTBIGRAM") == 0 ||
		strcmp(label,"WDPREVTAG") == 0 ||
		strcmp(label,"WDNEXTTAG") == 0 ||
		strcmp(label,"WDANDTAG") == 0 ||
		strcmp(label,"WDAND2AFT") == 0 ||
		strcmp(label,"WDAND2BFR") == 0 ||
		strcmp(label,"WDAND2TAGAFT") == 0 ||
		strcmp(label,"WDAND2TAGBFR") == 0 ||
		strcmp(label,"RBIGRAM") == 0 ||
		strcmp(label,"LBIGRAM") == 0 ||
		strcmp(label,"SURROUNDTAG") == 0) 
		FREEFLAG = 1;
	
	tempkey = Darray_create();
	Darray_hint(tempkey,10,Registry_entry_count(*goodhash));
	tempval = Darray_create();
	Darray_hint(tempval,10,Registry_entry_count(*goodhash));
	
	Registry_fetch_contents(*goodhash,tempkey,tempval);
	for (tempcount=0;tempcount<Darray_len(tempkey);++tempcount) {
		free(Darray_get(tempval,tempcount));
		if (FREEFLAG)
			free(Darray_get(tempkey,tempcount));
	}
	Darray_destroy(tempval);
	Darray_destroy(tempkey);
	Registry_destroy(*goodhash);
}

/**********************************************************************/

static void init_hash( Registry *thehash, int numhint )
{
	*thehash =  Registry_create(Registry_strcmp,Registry_strhash);
	Registry_size_hint(*thehash,numhint);
}

/***********************************************************************/


void contextrl( char **correct_tag_corpus, char **word_corpus, char **guess_tag_corpus, int corpus_size, FILE* outfile, int brillstyle, int brill_threshold )
{
	Darray errorlist,temperrorkey,temperrorval;
	Registry errorlistcount;
	
	char **error_list;
	int  sz_error_list;
	char *split_ptr[5];
	
	char wdpair[2048];
	int CONTINUE = 10000;
	int count,count2,numwrong,lengthcount;
	char globalprint[500];
	char forpasting[500];
	int globalbest = 0;
	char flag[20];
	Registry curwd,wdandtag,wdand2aft,wdand2bfr;
	Registry wdand2tagaft,wdand2tagbfr;
	Registry wdnexttag,wdprevtag;
	Registry rbigram,lbigram;
	Registry next1tag,prev1tag;
	Registry next1or2tag,prev1or2tag;
	Registry next1or2or3tag,prev1or2or3tag;
	Registry next1wd,prev1wd;
	Registry next1or2wd,prev1or2wd;
	Registry next1or2or3wd,prev1or2or3wd;
	Registry nextbigram,prevbigram;
	Registry surroundtag;
	Registry next2tag,prev2tag;
	Registry next2wd,prev2wd;
	int printscore;
	char atempstr2[2048];
	char onewdbfr[512],onewdaft[512],twowdbfr[512],twowdaft[512];
	char onetagbfr[512],twotagbfr[512],onetagaft[512],twotagaft[512];
	
	
	/*SEENTAGGING is a hash table of word/tag pairs, from the lexicon*/
	/* WORDS is a hash table of all words in the lexicon */

	strncpy(AnyWordS, AnyWord, 256);
	AnyWordS[256] = EOS;
	word_to_check = AnyWordS;
	
	while( CONTINUE > brill_threshold ) {
		/* this is the rule learning loop. We continue learning rules until the score of the best rule 
		found drops below the THRESHOLD, that is, WHEN NO RULES CAN BE FOUND WHOSE IMPROVEMENT IS GREATER
		THAN THE THRESHOLD, LEARNING STOPS. */
		
		/* generate and sort confusion matrix ( tag x confused for tag y with count z ) */
		msgfile(outfile,"\n");
		msgfile(outfile,"==========BEGINNING A LEARNING ITERATION===========\n");
		errorlist = Darray_create();
		Darray_hint(errorlist,10,500);
		temperrorkey = Darray_create();
		temperrorval = Darray_create();
		Darray_hint(temperrorkey,10,500);
		Darray_hint(temperrorval,10,500);
		
		init_hash(&errorlistcount,500);
		
		printscore=0;
		for(count=0;count<corpus_size;++count) {
			// msg ("%s %s %s\n",  word_corpus[count], guess_tag_corpus[count], correct_tag_corpus[count] );
			if (strcmp(guess_tag_corpus[count],correct_tag_corpus[count]) != 0) { 
				++printscore;
				if (brillstyle&LexicalAccess)
					sprintf(forpasting,"%s %s %s", word_corpus[count],
						guess_tag_corpus[count],
						correct_tag_corpus[count]);
				else
					sprintf(forpasting,"%s %s %s", word_to_check, 
						guess_tag_corpus[count],
						correct_tag_corpus[count]);
				increment_array_create(&errorlistcount,forpasting);
			} 
		}
		
		Registry_fetch_contents(errorlistcount,temperrorkey,temperrorval);
		sz_error_list = 0;
		for (count=0;count<Darray_len(temperrorkey);++count) {
			if (*(int *)Darray_get(temperrorval,count) > brill_threshold )
				sz_error_list ++;
		}
		error_list = (char**)malloc( sizeof(char*) * sz_error_list );
		int nb=0;
		for (count=0;count<Darray_len(temperrorkey);++count) {
			if (*(int *)Darray_get(temperrorval,count) > brill_threshold ) {
				char sz[128];
				sprintf(sz,"%8d %s",*(int *)Darray_get(temperrorval,count),
					(char *)Darray_get(temperrorkey,count));
				error_list[nb++] = mystrdup(sz);
			}
			free(Darray_get(temperrorval,count));
			free(Darray_get(temperrorkey,count));
		}
		Darray_destroy(temperrorval);
		Darray_destroy(temperrorkey);
		Registry_destroy(errorlistcount);
		
		msgfile(outfile,"NUMBER OF ERRORS: %d\n",printscore);
		
		/* reverse sort error_list */
		ssort( error_list, sz_error_list, 1 );
		for (nb=0; nb<sz_error_list;nb++) {
			Darray_addh(errorlist,error_list[nb]);
			// msg( "%d %s\n", errorlist, error_list[nb] );
		}
		free(error_list);
		
		globalbest= 0;
		strcpy(globalprint,"");
		
		/* for each pair of tags in the confusion matrix */
		for (count=0;count<Darray_len(errorlist);++count) {
			localbest =0;
			strcpy(localbestthing,"");
			
			split_with((char *)Darray_get(errorlist,count), split_ptr, " ", 5);
			
			if (brillstyle&LexicalAccess) {
				word_to_check = split_ptr[1]; /* this is the only word the rules will apply to */
				wrong = split_ptr[2]; /* this is the bad tag in the confusion matrix */
				right = split_ptr[3]; /* this is the good tag */
			} else {
				wrong = split_ptr[2]; /* this is the bad tag in the confusion matrix */
				right = split_ptr[3]; /* this is the good tag */
			}
			numwrong = atoi(split_ptr[0]); /* this is the number of confusions */
			if (numwrong > globalbest) {
				
				msgfile(outfile,"From confusion matrix, tagged as: {%s %s} should be: {%s %s}\n", word_to_check, wrong, word_to_check, right);
				msgfile(outfile,"Best rule found for this iteration so far: %d %s\n",globalbest,globalprint);
				init_hash(&curwd,NUMWDS/2);
				init_hash(&wdandtag,(NUMWDS*NUMWDS)/4);
				init_hash(&wdand2aft,(NUMWDS*NUMWDS)/4);
				init_hash(&wdand2bfr,(NUMWDS*NUMWDS)/4);
				init_hash(&wdand2tagaft,(NUMWDS*NUMTAGS)/4);
				init_hash(&wdand2tagbfr,(NUMWDS*NUMTAGS)/4);
				init_hash(&rbigram,(NUMWDS*NUMWDS)/4);
				init_hash(&lbigram,(NUMWDS*NUMWDS)/4);
				init_hash(&wdnexttag,(NUMWDS*NUMTAGS)/4);
				init_hash(&wdprevtag,(NUMWDS*NUMTAGS)/4);
				init_hash(&next1tag,NUMTAGS/2);
				init_hash(&prev1tag,NUMTAGS/2);
				init_hash(&next1or2tag,NUMTAGS/2);
				init_hash(&prev1or2tag,NUMTAGS/2);
				init_hash(&next1wd,NUMWDS/2);
				init_hash(&prev1wd,NUMWDS/2);
				init_hash(&next1or2wd,NUMWDS/2);
				init_hash(&prev1or2wd,NUMWDS/2);
				init_hash(&next1or2or3tag,NUMTAGS/2);
				init_hash(&prev1or2or3tag,NUMTAGS/2);
				init_hash(&next1or2or3wd,NUMWDS/2);
				init_hash(&prev1or2or3wd,NUMWDS/2);
				init_hash(&nextbigram,NUMTAGS);
				init_hash(&prevbigram,NUMTAGS);
				init_hash(&surroundtag,NUMTAGS);
				init_hash(&next2tag,NUMTAGS/2);
				init_hash(&prev2tag,NUMTAGS/2);
				init_hash(&next2wd,NUMWDS/2);
				init_hash(&prev2wd,NUMWDS/2);

				/* a proposed transformation tag X --> tag Y can have 3 outcomes.
				if neither X nor Y is the correct tag, then the transformation
				has no effect.  If X is correct, then the change is detremental.
				if Y is correct, then it is a good change. */
				
				lengthcount = corpus_size;
				for(count2=0;count2<lengthcount;++count2) {

					if (brillstyle&LexicalAccess)
						if ( strcmp(word_corpus[count2], word_to_check) ) continue;

					sprintf(atempstr2,"%s %s",word_corpus[count2],right);
					if (is_in_WORDS(word_corpus[count2]) &&
						! is_in_SEENTAGGING(atempstr2)) 
						strcpy(flag,"NOMATCH");
					else if (strcmp(guess_tag_corpus[count2],wrong) == 0 &&
						strcmp(correct_tag_corpus[count2],right) == 0 ) 
						strcpy(flag,"BADMATCH");
					else if (strcmp(guess_tag_corpus[count2],wrong) == 0 &&
						strcmp(correct_tag_corpus[count2],wrong) == 0 ) 
						strcpy(flag,"GOODMATCH");
					else 
						strcpy(flag,"NOMATCH");

					/* ok, the terminology is a bit screwed up.  If we have a "BADMATCH", that */
					/* means the current tagging is bad, and the transformation will fix it */
					/* ( a good thing ) */
					if (strcmp(flag,"BADMATCH") == 0) {
						increment_array(&curwd,word_corpus[count2]);
						sprintf(forpasting,"%s %s",word_corpus[count2],
							guess_tag_corpus[count2]);
						increment_array_create(&wdandtag,forpasting);
						if (count2 != lengthcount-1) {
							strcpy(onewdaft,word_corpus[count2+1]);
							strcpy(onetagaft,guess_tag_corpus[count2+1]);
							sprintf(wdpair,"%s %s",word_corpus[count2],
								word_corpus[count2+1]);
							increment_array_create(&rbigram,wdpair);
							sprintf(wdpair,"%s %s",word_corpus[count2],
								guess_tag_corpus[count2+1]);
							increment_array_create(&wdnexttag,wdpair);
							increment_array(&next1or2tag,
								guess_tag_corpus[count2+1]);
							increment_array(&next1or2or3tag,
								guess_tag_corpus[count2+1]);
							increment_array(&next1tag,guess_tag_corpus[count2+1]);
							increment_array(&next1wd,word_corpus[count2+1]);
							increment_array(&next1or2wd,word_corpus[count2+1]);
							increment_array(&next1or2or3wd,word_corpus[count2+1]);
						}
						if (count2 < lengthcount-2) {
							strcpy(twotagaft,guess_tag_corpus[count2+2]);
							strcpy(twowdaft,word_corpus[count2+2]);
							sprintf(forpasting,"%s %s",guess_tag_corpus[count2+1],
								guess_tag_corpus[count2+2]);
							increment_array_create(&nextbigram,forpasting);
							sprintf(forpasting,"%s %s",word_corpus[count2],
								word_corpus[count2+2]);
							increment_array_create(&wdand2aft,forpasting);
							sprintf(forpasting,"%s %s",word_corpus[count2],
								guess_tag_corpus[count2+2]);
							increment_array_create(&wdand2tagaft,forpasting);
							increment_array(&next2tag,guess_tag_corpus[count2+2]);
							increment_array(&next2wd,word_corpus[count2+2]);
							if (strcmp(guess_tag_corpus[count2+2],onetagaft) != 0)
							{ 
								increment_array(&next1or2tag,
									guess_tag_corpus[count2+2]);
								increment_array(&next1or2or3tag,
									guess_tag_corpus[count2+2]);
							}
							if (strcmp(word_corpus[count2+2],onewdaft) != 0) {
								increment_array(&next1or2wd,word_corpus[count2+2]);
								increment_array(&next1or2or3wd,word_corpus[count2+2]);
							}
						}
						if (count2 < lengthcount-3) {
							if (strcmp(guess_tag_corpus[count2+3],onetagaft) != 0
								&&
								strcmp(guess_tag_corpus[count2+3],twotagaft) != 0)
								increment_array(&next1or2or3tag,
								guess_tag_corpus[count2+3]);
							if (strcmp(word_corpus[count2+3],onewdaft) != 0
								&&
								strcmp(word_corpus[count2+3],twowdaft) != 0)
								increment_array(&next1or2or3wd,
								word_corpus[count2+3]);
						}
						if (count2 != 0) {
							strcpy(onewdbfr,word_corpus[count2-1]);
							strcpy(onetagbfr,guess_tag_corpus[count2-1]);
							sprintf(wdpair,"%s %s",word_corpus[count2-1],
								word_corpus[count2]);
							increment_array_create(&lbigram,wdpair);
							sprintf(wdpair,"%s %s",guess_tag_corpus[count2-1],
								word_corpus[count2]);
							increment_array_create(&wdprevtag,wdpair);
							increment_array(&prev1tag,guess_tag_corpus[count2-1]);
							increment_array(&prev1wd,word_corpus[count2-1]);
							increment_array(&prev1or2wd,word_corpus[count2-1]);
							increment_array(&prev1or2or3wd,word_corpus[count2-1]);
							increment_array(&prev1or2tag,
								guess_tag_corpus[count2-1]);
							increment_array(&prev1or2or3tag,
								guess_tag_corpus[count2-1]);
							if (count2 < lengthcount-1) {
								sprintf(forpasting,"%s %s",guess_tag_corpus[count2-1],
									guess_tag_corpus[count2+1]);
								increment_array_create(&surroundtag,forpasting);
							}
						}
						if (count2 > 1) {
							strcpy(twotagbfr,guess_tag_corpus[count2-2]);
							strcpy(twowdbfr,word_corpus[count2-2]);
							sprintf(forpasting,"%s %s",guess_tag_corpus[count2-2],
								guess_tag_corpus[count2-1]);
							increment_array_create(&prevbigram,forpasting);
							sprintf(forpasting,"%s %s",word_corpus[count2-2],
								word_corpus[count2]);
							increment_array_create(&wdand2bfr,forpasting);
							sprintf(forpasting,"%s %s",guess_tag_corpus[count2-2],
								word_corpus[count2]);
							increment_array_create(&wdand2tagbfr,forpasting);
							increment_array(&prev2tag,guess_tag_corpus[count2-2]);
							increment_array(&prev2wd,word_corpus[count2-2]);
							if (strcmp(guess_tag_corpus[count2-2],onetagbfr) != 0){ 
								increment_array(&prev1or2tag,
									guess_tag_corpus[count2-2]);
								increment_array(&prev1or2or3tag,
									guess_tag_corpus[count2-2]);
							}
							if (strcmp(word_corpus[count2-2],onewdbfr) != 0) {
								increment_array(&prev1or2wd,word_corpus[count2-2]);
								increment_array(&prev1or2or3wd,word_corpus[count2-2]);
							}
						}
						if (count2 > 2) {
							if (strcmp(guess_tag_corpus[count2-3],onetagbfr) != 0
								&&
								strcmp(guess_tag_corpus[count2-3],twotagbfr) != 0)
								increment_array(&prev1or2or3tag,
								guess_tag_corpus[count2-3]);
							if (strcmp(word_corpus[count2-3],onewdbfr) != 0
								&&
								strcmp(word_corpus[count2-3],twowdbfr) != 0)
								increment_array(&prev1or2or3wd,
								word_corpus[count2-3]);
						}
					}
	
					/* this is the case where the current tagging is correct, and therefore the */
					/* transformation will hurt */
					else if (strcmp(flag,"GOODMATCH") == 0) {
						decrement_array(&curwd,word_corpus[count2]);
						sprintf(forpasting,"%s %s",word_corpus[count2],
							guess_tag_corpus[count2]);
						decrement_array_create(&wdandtag,forpasting);
						if (count2 != lengthcount-1) {
							strcpy(onewdaft,word_corpus[count2+1]);
							strcpy(onetagaft,guess_tag_corpus[count2+1]);
				            		sprintf(wdpair,"%s %s",word_corpus[count2],
								word_corpus[count2+1]);
							decrement_array_create(&rbigram,wdpair);
							sprintf(wdpair,"%s %s",word_corpus[count2],
								guess_tag_corpus[count2+1]);
							decrement_array_create(&wdnexttag,wdpair);
							decrement_array(&next1tag,guess_tag_corpus[count2+1]);
							decrement_array(&next1wd,word_corpus[count2+1]);
							decrement_array(&next1or2wd,word_corpus[count2+1]);
							decrement_array(&next1or2tag,guess_tag_corpus[count2+1]);
							decrement_array(&next1or2or3tag,guess_tag_corpus[count2+1]);
							decrement_array(&next1or2or3wd,word_corpus[count2+1]);
						}
						if (count2 < lengthcount-2) {
							strcpy(twotagaft,guess_tag_corpus[count2+2]);
							strcpy(twowdaft,word_corpus[count2+2]);
							sprintf(forpasting,"%s %s",guess_tag_corpus[count2+1],
								guess_tag_corpus[count2+2]);
							decrement_array_create(&nextbigram,forpasting);
							sprintf(forpasting,"%s %s",word_corpus[count2],
								word_corpus[count2+2]);
							decrement_array_create(&wdand2aft,forpasting);
							sprintf(forpasting,"%s %s",word_corpus[count2],
								guess_tag_corpus[count2+2]);
							decrement_array_create(&wdand2tagaft,forpasting);
							decrement_array(&next2tag,guess_tag_corpus[count2+2]);
							decrement_array(&next2wd,word_corpus[count2+2]);
							if (strcmp(guess_tag_corpus[count2+2],onetagaft) !=0) {
								decrement_array(&next1or2tag,
									guess_tag_corpus[count2+2]);
								decrement_array(&next1or2or3tag,
									guess_tag_corpus[count2+2]); }
							if (strcmp(word_corpus[count2+2],onewdaft) != 0) {
								decrement_array(&next1or2wd,word_corpus[count2+2]);
								decrement_array(&next1or2or3wd,word_corpus[count2+2]);
							}
						}
						if (count2 < lengthcount-3) {
							if (strcmp(guess_tag_corpus[count2+3],onetagaft) !=0 
								&&
								strcmp(guess_tag_corpus[count2+3],twotagaft) !=0 )
								decrement_array(&next1or2or3tag,
								guess_tag_corpus[count2+3]);
							if (strcmp(word_corpus[count2+3],onewdaft) != 0
								&&
								strcmp(word_corpus[count2+3],twowdaft) != 0)
								decrement_array(&next1or2or3wd,
								word_corpus[count2+3]);
						}
						if (count2 != 0) {
							strcpy(onewdbfr,word_corpus[count2-1]);
							strcpy(onetagbfr,guess_tag_corpus[count2-1]);
							sprintf(wdpair,"%s %s",word_corpus[count2-1],
								word_corpus[count2]);
							decrement_array_create(&lbigram,wdpair);
							sprintf(wdpair,"%s %s",guess_tag_corpus[count2-1],
								word_corpus[count2]);
							decrement_array_create(&wdprevtag,wdpair);
							decrement_array(&prev1tag,guess_tag_corpus[count2-1]);
							decrement_array(&prev1wd,word_corpus[count2-1]);
							decrement_array(&prev1or2wd,word_corpus[count2-1]);
							decrement_array(&prev1or2or3wd,word_corpus[count2-1]);
							decrement_array(&prev1or2tag,
								guess_tag_corpus[count2-1]);
							decrement_array(&prev1or2or3tag,
								guess_tag_corpus[count2-1]);
							if (count2 < lengthcount-1) {
								sprintf(forpasting,"%s %s",guess_tag_corpus[count2-1],
									guess_tag_corpus[count2+1]);
								decrement_array_create(&surroundtag,forpasting);
							}
						}
						if (count2 >1 ) { 
							strcpy(twotagbfr,guess_tag_corpus[count2-2]);
							strcpy(twowdbfr,word_corpus[count2-2]);
							sprintf(forpasting,"%s %s",guess_tag_corpus[count2-2],
								guess_tag_corpus[count2-1]);
							decrement_array_create(&prevbigram,forpasting);
							sprintf(forpasting,"%s %s",word_corpus[count2-2],
								word_corpus[count2]);
							decrement_array_create(&wdand2bfr,forpasting);
							sprintf(forpasting,"%s %s",guess_tag_corpus[count2-2],
								word_corpus[count2]);
							decrement_array_create(&wdand2tagbfr,forpasting);
							decrement_array(&prev2tag,guess_tag_corpus[count2-2]);
							decrement_array(&prev2wd,word_corpus[count2-2]);
							if (strcmp(guess_tag_corpus[count2-2],onetagbfr) != 0)
							{ 
								decrement_array(&prev1or2tag,
									guess_tag_corpus[count2-2]);
								decrement_array(&prev1or2or3tag,
									guess_tag_corpus[count2-2]); }
							if (strcmp(word_corpus[count2-2],onewdbfr) != 0) {
								decrement_array(&prev1or2wd,word_corpus[count2-2]);
								decrement_array(&prev1or2or3wd,word_corpus[count2-2]);
							}
						}
						if (count2 > 2) {
							if (strcmp(guess_tag_corpus[count2-3],onetagbfr) != 0 
								&&
								strcmp(guess_tag_corpus[count2-3],twotagbfr) != 0)
								decrement_array(&prev1or2or3tag,
								guess_tag_corpus[count2-3]);
							if (strcmp(word_corpus[count2-3],onewdbfr) != 0
								&&
								strcmp(word_corpus[count2-3],twowdbfr) != 0)
								decrement_array(&prev1or2or3wd,
								word_corpus[count2-3]);
						}
					}
				}  

				/* now we go through all of the contexts we've recorded, and see which */
				/* contextual trigger provides the biggest win */

				if ( !( brillstyle & PartialRules ) ) {
					check_counts(&prev1tag,"PREVTAG");
					check_counts(&next1tag,"NEXTTAG");
					check_counts(&next1or2tag,"NEXT1OR2TAG");
					check_counts(&prev1or2tag,"PREV1OR2TAG");
					check_counts(&wdnexttag,"WDNEXTTAG");
					check_counts(&wdprevtag,"WDPREVTAG");
					check_counts(&next1wd,"NEXTWD");
					check_counts(&prev1wd,"PREVWD");
					check_counts(&curwd,"CURWD");
					check_counts(&rbigram,"RBIGRAM");
					check_counts(&lbigram,"LBIGRAM");
					check_counts(&next1or2or3tag,"NEXT1OR2OR3TAG");
					check_counts(&prev1or2or3tag,"PREV1OR2OR3TAG");
				} else {
					free_counts(&prev1tag,"PREVTAG");
					free_counts(&next1tag,"NEXTTAG");
					free_counts(&next1or2tag,"NEXT1OR2TAG");
					free_counts(&prev1or2tag,"PREV1OR2TAG");
					free_counts(&wdnexttag,"WDNEXTTAG");
					free_counts(&wdprevtag,"WDPREVTAG");
					free_counts(&next1wd,"NEXTWD");
					free_counts(&prev1wd,"PREVWD");
					free_counts(&curwd,"CURWD");
					free_counts(&rbigram,"RBIGRAM");
					free_counts(&lbigram,"LBIGRAM");
					free_counts(&next1or2or3tag,"NEXT1OR2OR3TAG");
					free_counts(&prev1or2or3tag,"PREV1OR2OR3TAG");
				}
				check_counts(&prevbigram,"PREVBIGRAM");
				check_counts(&nextbigram,"NEXTBIGRAM");
				check_counts(&surroundtag,"SURROUNDTAG");

				check_counts(&next1or2wd,"NEXT1OR2WD");
				check_counts(&prev1or2wd,"PREV1OR2WD");
				check_counts(&next2tag,"NEXT2TAG");
				check_counts(&prev2tag,"PREV2TAG");
				check_counts(&next2wd,"NEXT2WD");
				check_counts(&prev2wd,"PREV2WD");
				check_counts(&wdandtag,"WDANDTAG");
				check_counts(&wdand2aft,"WDAND2AFT");
				check_counts(&wdand2bfr,"WDAND2BFR");
				check_counts(&wdand2tagaft,"WDAND2TAGAFT");
				check_counts(&wdand2tagbfr,"WDAND2TAGBFR");
				check_counts(&next1or2or3wd,"NEXT1OR2OR3WD");
				check_counts(&prev1or2or3wd,"PREV1OR2OR3WD");
		  
				if (localbest > globalbest) {
					globalbest = localbest;
					strcpy(globalprint,localbestthing);
				}
			}
		}
		if ( strcmp(globalprint,"") ) {
			implement_change(globalprint, guess_tag_corpus, word_corpus, corpus_size);
			char wrd[64];
			char tag[24];
			sscanf( globalprint, "%s %s", wrd, tag );
			add_brillrule(wrd, tag , globalprint);
			msgfile(outfile,"new rule : %s\n",globalprint);
		}
		CONTINUE = globalbest; 
		for (count=0;count<Darray_len(errorlist);++count)
			free(Darray_get(errorlist,count));
		Darray_destroy(errorlist); 
	}
}


/*************  The code below is for updating the corpus by applying
the learned rule ***************************************/

static void change_the_tag( char **theentry, char *thetag, int theposition )

{
	free(theentry[theposition]);
	theentry[theposition] = mystrdup(thetag);
}


void implement_change( char *thearg, char **thetagcorpus, char **thewordcorpus, int corpuslen )
{
	
	char line[5000];	/* input line buffer */
	char *split_ptr[6];
	int  count,tempcount1,tempcount2;
	char old[100], nouv[100], when[300], tag[100], lft[100],
		rght[100], prev1[100], prev2[100], next1[100], next2[100], 
		curtag[100], word[300], word_target[300];
	char atempstr2[256], curwd[100];
	int islexical ;
	
	strcpy(line,thearg);
	if (strlen(line) > 1) {
		split_with(line, split_ptr, " ", 6);
		strcpy(word_target, split_ptr[0]);

		if ( !strcmp(word_target, AnyWord) )
			islexical = 0;
		else
			islexical = 1;

		strcpy(old, split_ptr[1]);
		// nouv = mystrdup(split_ptr[2]);
		strcpy(nouv, split_ptr[2]);
		strcpy(when, split_ptr[3]);
		
		if (strcmp(when, "NEXTTAG") == 0 ||
			strcmp(when, "NEXT2TAG") == 0 ||
			strcmp(when, "NEXT1OR2TAG") == 0 ||
			strcmp(when, "NEXT1OR2OR3TAG") == 0 ||
			strcmp(when, "PREVTAG") == 0 ||
			strcmp(when, "PREV2TAG") == 0 ||
			strcmp(when, "PREV1OR2TAG") == 0 ||
			strcmp(when, "PREV1OR2OR3TAG") == 0) {
			strcpy(tag, split_ptr[4]);
		} 
		else if (strcmp(when, "NEXTWD") == 0 ||
			strcmp(when,"CURWD") == 0 || 
			strcmp(when, "NEXT2WD") == 0 ||
			strcmp(when, "NEXT1OR2WD") == 0 ||
			strcmp(when, "NEXT1OR2OR3WD") == 0 ||
			strcmp(when, "PREVWD") == 0 ||
			strcmp(when, "PREV2WD") == 0 ||
			strcmp(when, "PREV1OR2WD") == 0 ||
			strcmp(when, "PREV1OR2OR3WD") == 0) {
			strcpy(word, split_ptr[4]);
		}
		else if (strcmp(when, "WDANDTAG") == 0 ) {
			strcpy(tag, split_ptr[5]);
			strcpy(word, split_ptr[4]);
		}
		else if (strcmp(when, "SURROUNDTAG") == 0) {
			strcpy(lft, split_ptr[4]);
			strcpy(rght, split_ptr[5]);
		} else if (strcmp(when, "PREVBIGRAM") == 0) {
			strcpy(prev1, split_ptr[4]);
			strcpy(prev2, split_ptr[5]);
		} else if (strcmp(when, "NEXTBIGRAM") == 0) {
			strcpy(next1, split_ptr[4]);
			strcpy(next2, split_ptr[5]);
		} else if (strcmp(when,"LBIGRAM") == 0||
			strcmp(when,"WDPREVTAG") == 0) {
			strcpy(prev1,split_ptr[4]);
			strcpy(word,split_ptr[5]); 
		} else if (strcmp(when,"RBIGRAM") == 0 ||
			strcmp(when,"WDNEXTTAG") == 0) {
			strcpy(word,split_ptr[4]);
			strcpy(next1,split_ptr[5]); 
		} else if (strcmp(when,"WDAND2BFR")== 0 ||
			strcmp(when,"WDAND2TAGBFR")== 0) {
			strcpy(prev2,split_ptr[4]);
			strcpy(word,split_ptr[5]);}
		else if (strcmp(when,"WDAND2AFT")== 0 ||
			strcmp(when,"WDAND2TAGAFT")== 0) {
			strcpy(next2,split_ptr[5]);
			strcpy(word,split_ptr[4]);}
		
		for (count = 0; count <corpuslen; ++count) {
			if ( islexical && strcmp(word_target,thewordcorpus[count]) )
				continue;
			strcpy(curtag, thetagcorpus[count]);
			if (strcmp(curtag, old) == 0) {
				strcpy(curwd,thewordcorpus[count]);
				sprintf(atempstr2,"%s %s",curwd,nouv);
				
				if (! is_in_WORDS(curwd) ||
					is_in_SEENTAGGING(atempstr2)) {
					
					if (strcmp(when, "SURROUNDTAG") == 0) {
						if (count < corpuslen-1 && count > 0) {
							if (strcmp(lft, thetagcorpus[count - 1]) == 0 &&
								strcmp(rght, thetagcorpus[count + 1]) == 0) 
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "NEXTTAG") == 0) {
						if (count < corpuslen-1) {
							if (strcmp(tag, thetagcorpus[count + 1]) == 0) 
								change_the_tag(thetagcorpus,nouv,count);
						}
					}  
					else if (strcmp(when, "CURWD") == 0) {
						if (strcmp(word, thewordcorpus[count]) == 0)
							change_the_tag(thetagcorpus,nouv,count);
					}  
					else if (strcmp(when, "WDANDTAG") == 0) {
						if (  strcmp(word, thewordcorpus[count]) ==
							0 &&
							strcmp(tag, thetagcorpus[count]) ==
							0)
							change_the_tag(thetagcorpus,nouv,count);
					}  
					else if (strcmp(when, "NEXTWD") == 0) {
						if (count < corpuslen-1) {
							if (strcmp(word, thewordcorpus[count + 1]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}  else if (strcmp(when, "RBIGRAM") == 0) {
						if (count < corpuslen-1) {
							if (strcmp(word, thewordcorpus[count]) ==
								0 &&
								strcmp(next1, thewordcorpus[count+1]) ==
								0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} 
					else if (strcmp(when, "WDNEXTTAG") == 0) {
						if (count < corpuslen-1) {
							if (strcmp(word, thewordcorpus[count]) ==
								0 &&
								strcmp(next1, thetagcorpus[count+1]) ==
								0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}
					
					else if (strcmp(when, "WDAND2AFT") == 0) {
						if (count < corpuslen-2) {
							if (strcmp(word, thewordcorpus[count]) ==
								0 &&
								strcmp(next2,thewordcorpus[count+2]) ==
								0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}
					else if (strcmp(when, "WDAND2TAGAFT") == 0) {
						if (count < corpuslen-2) {
							if (strcmp(word, thewordcorpus[count]) ==
								0 &&
								strcmp(next2, thetagcorpus[count+2]) ==
								0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}
					else if (strcmp(when, "NEXT2TAG") == 0) {
						if (count < corpuslen-2) {
							if (strcmp(tag,thetagcorpus[count + 2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "NEXT2WD") == 0) {
						if (count < corpuslen-2) {
							if (strcmp(word, thewordcorpus[count + 2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "NEXTBIGRAM") == 0) {
						if (count < corpuslen-2) {
							if
								(strcmp(next1, thetagcorpus[count + 1]) == 0 &&
								strcmp(next2, thetagcorpus[count + 2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "NEXT1OR2TAG") == 0) {
						if (count < corpuslen-1) {
							if (count < corpuslen-2) 
								tempcount1 = count+2;
							else 
								tempcount1 = count+1;
							if
								(strcmp(tag, thetagcorpus[count + 1]) == 0 ||
								strcmp(tag, thetagcorpus[tempcount1]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}  else if (strcmp(when, "NEXT1OR2WD") == 0) {
						if (count < corpuslen-1) {
							if (count < corpuslen-2) 
								tempcount1 = count+2;
							else 
								tempcount1 = count+1;
							if
								(strcmp(word, thewordcorpus[count + 1]) == 0 ||
								strcmp(word, thewordcorpus[tempcount1]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}   else if (strcmp(when, "NEXT1OR2OR3TAG") == 0) {
						if (count < corpuslen-1) {
							if (count < corpuslen-2)
								tempcount1 = count+2;
							else 
								tempcount1 = count+1;
							if (count < corpuslen-3)
								tempcount2 = count+3;
							else 
								tempcount2 =count+1;
							if
								(strcmp(tag, thetagcorpus[count + 1]) == 0 ||
								strcmp(tag, thetagcorpus[tempcount1]) == 0 ||
								strcmp(tag, thetagcorpus[tempcount2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "NEXT1OR2OR3WD") == 0) {
						if (count < corpuslen-1) {
							if (count < corpuslen-2)
								tempcount1 = count+2;
							else 
								tempcount1 = count+1;
							if (count < corpuslen-3)
								tempcount2 = count+3;
							else 
								tempcount2 =count+1;
							if
								(strcmp(word, thewordcorpus[count + 1]) == 0 ||
								strcmp(word, thewordcorpus[tempcount1]) == 0 ||
								strcmp(word, thewordcorpus[tempcount2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}  else if (strcmp(when, "PREVTAG") == 0) {
						if (count > 0) {
							if (strcmp(tag, thetagcorpus[count - 1]) == 0) {
								change_the_tag(thetagcorpus,nouv,count);
							}
						}
					} else if (strcmp(when, "PREVWD") == 0) {
						if (count > 0) {
							if (strcmp(word, thewordcorpus[count - 1]) == 0) {
								change_the_tag(thetagcorpus,nouv,count);
							}
						}
					}  else if (strcmp(when, "LBIGRAM") == 0) {
						if (count > 0) {
							if (strcmp(word, thewordcorpus[count]) ==
								0 &&
								strcmp(prev1, thewordcorpus[count-1]) ==
								0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}
					else if (strcmp(when, "WDPREVTAG") == 0) {
						if (count > 0) {
							if (strcmp(word, thewordcorpus[count]) ==
								0 &&
								strcmp(prev1, thetagcorpus[count-1]) ==
								0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}
					else if (strcmp(when, "WDAND2BFR") == 0) {
						if (count > 1) {
							if (strcmp(word, thewordcorpus[count]) ==
								0 &&
								strcmp(prev2, thewordcorpus[count-2]) ==
								0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}
					else if (strcmp(when, "WDAND2TAGBFR") == 0) {
						if (count > 1) {
							if (strcmp(word, thewordcorpus[count]) ==
								0 &&
								strcmp(prev2, thetagcorpus[count-2]) ==
								0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}
					else if (strcmp(when, "PREV2TAG") == 0) {
						if (count > 1) {
							if (strcmp(tag, thetagcorpus[count - 2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "PREV2WD") == 0) {
						if (count > 1) {
							if (strcmp(word, thewordcorpus[count - 2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "PREV1OR2TAG") == 0) {
						if (count > 0) {
							if (count > 1) 
								tempcount1 = count-2;
							else
								tempcount1 = count-1;
							if (strcmp(tag, thetagcorpus[count - 1]) == 0 ||
								strcmp(tag, thetagcorpus[tempcount1]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "PREV1OR2WD") == 0) {
						if (count > 0) {
							if (count > 1) 
								tempcount1 = count-2;
							else
								tempcount1 = count-1;
							if (strcmp(word, thewordcorpus[count - 1]) == 0 ||
								strcmp(word, thewordcorpus[tempcount1]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "PREV1OR2OR3TAG") == 0) {
						if (count > 0) {
							if (count>1) 
								tempcount1 = count-2;
							else 
								tempcount1 = count-1;
							if (count >2) 
								tempcount2 = count-3;
							else 
								tempcount2 = count-1;
							if (strcmp(tag, thetagcorpus[count - 1]) == 0 ||
								strcmp(tag, thetagcorpus[tempcount1]) == 0 ||
								strcmp(tag, thetagcorpus[tempcount2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "PREV1OR2OR3WD") == 0) {
						if (count > 0) {
							if (count>1) 
								tempcount1 = count-2;
							else 
								tempcount1 = count-1;
							if (count >2) 
								tempcount2 = count-3;
							else 
								tempcount2 = count-1;
							if (strcmp(word, thewordcorpus[count - 1]) == 0 ||
								strcmp(word, thewordcorpus[tempcount1]) == 0 ||
								strcmp(word, thewordcorpus[tempcount2]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					} else if (strcmp(when, "PREVBIGRAM") == 0) {
						if (count > 1) {
							if (strcmp(prev1, thetagcorpus[count - 2]) == 0 &&
								strcmp(prev2, thetagcorpus[count - 1]) == 0)
								change_the_tag(thetagcorpus,nouv,count);
						}
					}
					else 
						msg("ERROR: %s is not an allowable transform type\n", when);
				}
			}
		}
	}
}
	  
