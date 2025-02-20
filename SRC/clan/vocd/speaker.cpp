/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************
 * speaker.c                                               *
 * - functions for managing speakers, dependent tiers, etc.*
 *                                                         *
 * VOCD Version 1.0, August 1999                           *
 * G T McKee, The University of Reading, UK.               *
 ***********************************************************/

#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
	#include "cu.h"
	#undef strcmp
	#define strcmp uS.mStricmp
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

#include "args.h"
#include "vocdefs.h"
#include "speaker.h"
#include "tokens.h"

extern char isLanguageExplicit;


/* 
 * speaker_linecount
 *
 * Count the number of lines associated with a speaker.
 *
 */
int speaker_linecount (VOCDSP *speaker)
{
   Line *lpt;
   int n = 0;

   lpt = speaker->firstline;
   while (lpt!=NULL) {
      n++;
      lpt=lpt->nextline;
   }
   return n;
}

/* 
 * speaker_tokencount
 *
 * Count the number of tokens associated with a speaker.
 *
 */
int speaker_tokencount (VOCDSP *speaker)
{
   Line *lpt;
   int n = 0;

   lpt = speaker->firstline;
   while (lpt!=NULL) {
      n+=lpt->no_words;
      lpt=lpt->nextline;
   }
   return n;
}

/* 
 * speaker_ttr
 *
 * Calculate the TYPE-TOKEN ratio for a specific speaker.
 * Return the ttr.
 */
double speaker_ttr( VOCDSP *speaker, int *typs, int *tkns )
{
   int tokens, types=0;
   Line *lpt;
   int n_words;
   char **words;
   NODE *root;
   double  ttr;

   /* get the token count */
   tokens = speaker_tokencount (speaker);

   /* assemble the words into types */
   root = tokens_create_node(NO_SUBNODES);
   lpt = speaker->firstline;

   while (lpt!=NULL) {
      n_words = lpt->no_words;
      words   = lpt->words;

      while (n_words--) {
         if (!tokens_member_simple(*words,root)) {
            if (tokens_insert(root, *words)==-1) {
               fprintf(stderr, "speaker_ttr: line with empty word:");
               speaker_fprint_line (stderr, lpt->no_words, lpt->words);
              
            };
            types++;
         }
         words++;
      }
      lpt=lpt->nextline;
   }

   *typs = types;
   *tkns = tokens;
   ttr = (double ) types / (double ) tokens;

   return ttr;
}


/* 
 * speaker_print_line
 *
 * Print a line associated with a speaker.
 *
 */
void speaker_print_line (int n, char **w)
{
   int c;

   c = n;
   while (c--)
      printf("%s ",*w++);
   printf("\n");
}


/* 
 * speaker_print_speaker
 *
 * Print all of the lines associated with a speaker.
 *
 
void speaker_print_speaker( VOCDSP *speaker )
{
   Line *lpt;

   lpt = speaker->firstline;
   while (lpt!=NULL) {
      if (lpt->no_words>0)
         speaker_print_line (lpt->no_words, lpt->words);
      lpt = lpt->nextline;
   }
}
*/

/* 
 * speaker_fprint_line
 *
 * Print a line of tokens associated with a speaker.
 *
 */
void speaker_fprint_line (FILE *ofp, int n, char **w)
{
   int c;

   c = n;
   while (c--)
      fprintf(ofp, "%s ",*w++);

   fprintf(ofp,"\n");
}


/* 
 * speaker_fprint_speaker
 *
 * Print all of lines of tokens associated with a speaker.
 *
 */
void speaker_fprint_speaker( FILE *ofp, VOCDSP *speaker )
{
   Line *lpt;

   lpt = speaker->firstline;
   while (lpt!=NULL) {
      if (lpt->no_words>0)
         speaker_fprint_line (ofp, lpt->no_words, lpt->words);
      lpt = lpt->nextline;
   }
}


/* 
 * speaker_add_line
 *
 * Add a new line to the list of lines for "speaker".
 *
 */
 
 // lxs addition to filter input data file tiers lxslxs
 static void filter_input_line(char *pt) {
 	long len = strlen(pt) - 1, len2;
 
 	for (; len >= 0; len--) {
 		if (pt[len] == ']') {
 			if (len > 0 && (pt[len-1] == ' ' || pt[len-1] == '\t'))
 				strcpy(pt+len-1, pt+len);
		} else if (pt[len] == HIDEN_C) {
			len2 = len;
			for (len--; len >= 0 && pt[len] != HIDEN_C; len--) ;
			if (len < 0)
				len = len2;
			else
				strcpy(pt+len, pt+len2+1);
		} else if (pt[len] == '@' && pt[len+1] == 's' && pt[len+2] == ':' && isLanguageExplicit == 1) {
			len2 = len;
			for (; !uS.isskip(pt, len2, &dFnt, MBF) && pt[len2] != EOS; len2++) ;
			if (len < len2)
				strcpy(pt+len, pt+len2);
		}
 	}
 }
 
int speaker_add_line ( VOCDSP * speaker, char *pt, long lineno )
{
   int argc;
   char **argv;
   Line * new_line;

   filter_input_line(pt);
   argc = parse_args(pt, &argv);
   if (argc>0) {
      new_line = (Line *) malloc (sizeof(Line));
      if (new_line==NULL) {
         fprintf(stderr, "speaker_add_line: no memory\n");
         return 0;
      }
	  new_line->lineno = lineno;
      new_line->no_words = argc;
      new_line->words    = argv;
      new_line->nextline = NULL;
      if (speaker->firstline == NULL) {
         speaker->firstline = new_line;
         speaker->lastline = new_line;
      }
      else {
         speaker->lastline->nextline = new_line;
         speaker->lastline = new_line;
      }
   }
   return 1;
}

/* 
 * speaker_append_to_line
 *
 * Append a line to the last line for "speaker".
 *
 */
int speaker_append_to_line ( VOCDSP * speaker, char *pt )
{
   int argc;
   char **argv;
   Line *last;
   int new_len;
   char **new_words;
   int i,k;


   argc = parse_args(pt, &argv);

   if (speaker->lastline == NULL) {
         fprintf(stderr, "speaker_append_to_line: no previous line\n");
         return -1;
   }

   if (argc>0) {
      last = speaker->lastline;
      new_len = argc + last->no_words;

      new_words = (char **) malloc (new_len * sizeof(char *));
      if (new_words==NULL) {
         fprintf(stderr, "speaker_append_to_line: no memory\n");
         return -1;
      }

      i = 0;
      while (last->no_words--) {
         new_words[i] = last->words[i];
		 i++;
      }

      k = 0;
      while (k<argc) {
         new_words[i++] = argv[k++];
      }

      free (last->words);
      free (argv);
      last->no_words = i;
      last->words = new_words;

   }
   return 1;
}


/* 
 * speaker_add_speaker
 *
 * Add a speaker to the speaker list.
 *
 */
VOCDSP *speaker_add_speaker(VOCDSPs ** s, char *name, char *fname, char *IDs, char isTemp, char isUsed)
{
	VOCDSPs *newS;
	VOCDSPs *speakers;

	uS.remblanks(name);
	speakers = *s;
	if (speakers==NULL) {
		newS = (VOCDSPs *) malloc (sizeof(VOCDSPs));
		if (newS==NULL) {
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return NULL;
		}
		memset(newS, 0, sizeof(VOCDSPs));
		newS->isTemp = isTemp;
		newS->speaker = (VOCDSP *) malloc (sizeof(VOCDSP));
		if (newS->speaker==NULL) {
			free(newS);
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return NULL;
		}
		memset(newS->speaker, 0, sizeof(VOCDSP));
		speakers = newS;
		*s = newS;
	} else  {   /* speakers already present on the list */
		/* move to last speaker on list */
		while (speakers->next != NULL) {
			if (uS.partcmp(name,speakers->speaker->code,FALSE,FALSE)) {
				speakers->speaker->isUsed = isUsed;
				return speakers->speaker;
			}
			speakers = speakers->next;
		}
		if (uS.partcmp(name,speakers->speaker->code,FALSE,FALSE)) {
			speakers->speaker->isUsed = isUsed;
			return speakers->speaker;
		}
		/* add new speaker to the end of the list */
		newS = (VOCDSPs *) malloc (sizeof(VOCDSPs));
		if (newS==NULL) {
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return NULL;
		}
		memset(newS, 0, sizeof(VOCDSPs));
		newS->isTemp = isTemp;
		newS->speaker = (VOCDSP *) malloc (sizeof(VOCDSP));
		if (newS->speaker==NULL) {
			free(newS);
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return NULL;
		}
		memset(newS->speaker, 0, sizeof(VOCDSP));
		speakers->next = newS;
		speakers = newS;
	}
	speakers->speaker->isUsed = isUsed;
	if (fname == NULL)
		speakers->speaker->fname = NULL;
	else {
		speakers->speaker->fname = (char *)malloc(strlen(fname)+1);
		if (speakers->speaker->fname == NULL) {
			free(newS->speaker);
			free(newS);
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return NULL;
		}
		strcpy(speakers->speaker->fname, fname);
	}
	if (IDs == NULL)
		speakers->speaker->IDs = NULL;
	else {
		speakers->speaker->IDs = (char *)malloc(strlen(IDs)+1);
		if (speakers->speaker->IDs == NULL) {
			if (newS->speaker->fname != NULL)
				free(newS->speaker->fname);
			free(newS->speaker);
			free(newS);
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return NULL;
		}
		strcpy(speakers->speaker->IDs, IDs);
	}
	speakers->speaker->code = (char *)malloc(strlen(name)+1);
	if (speakers->speaker->code == NULL) {
		if (newS->speaker->fname != NULL)
			free(newS->speaker->fname);
		if (newS->speaker->IDs != NULL)
			free(newS->speaker->IDs);
		free(newS->speaker);
		free(newS);
		fprintf(stderr, "speaker_add_speaker: no memory\n");
		return NULL;
	}
	strcpy(speakers->speaker->code, name);
	speakers->speaker->firstline = NULL;
	speakers->speaker->lastline = NULL;
	return speakers->speaker;
}

/*
 * speaker_add_dependent_tier
 *
 * Add a dependent tier to all the speakers.
 *

int speaker_add_dependent_tier( Tier **d, char *description) 
{
	Tier *dependents, *dpt;
	char *pt;
	
	pt = description;
	dependents = *d;
	
	if (dependents==NULL) {
		dpt = (Tier *) malloc(sizeof(Tier));
		if (dpt == NULL) {
			fprintf(stderr, "add-speakers: no memory to add dependent tier\n");
			return 0;
		}
		dpt->next = NULL;
		if (strlen(pt) > MAX_LEN_CODE-1) {
			fprintf(stderr, "add-speakers: MAX_LEN_CODE limit exceeded\n");
			return 0;
		}
		strcpy(dpt->code,pt);
		*d = dpt;
	}
	else {
		// move to end of list
		while (dependents->next != NULL)
			dependents=dependents->next;
		
		// add dependent tier to tail
		dpt = (Tier *) malloc(sizeof(Tier));
		if (dpt == NULL) {
			fprintf(stderr, "add-speakers: no memory to add dependent tier\n");
			return 0;
		}
		dpt->next = NULL;
		if (strlen(pt) > MAX_LEN_CODE-1) {
			fprintf(stderr, "add-speakers: MAX_LEN_CODE limit exceeded\n");
			return 0;
		}
		strcpy(dpt->code, pt);
		dependents->next = dpt;
	}
	return 1;
}
*/
/* 
 * speaker_free_up_tier
 *
 * Insert a dependent tier.
 *
 */

VOCDSPs *speaker_free_up_speakers (VOCDSPs *rp, char isAll)
{
	VOCDSPs *t, *p;
	Line *lpt, *tmp_line;

	if (isAll) {
		while (rp != NULL) {
			t = rp;
			rp = rp->next;
			lpt = t->speaker->firstline;
			t->speaker->firstline = NULL;
			t->speaker->lastline = NULL;
			while (lpt!=NULL) {
				tmp_line = lpt;
				lpt = lpt->nextline;
				speaker_free_line(tmp_line);
			}
			if (t->speaker->fname != NULL)
				free(t->speaker->fname);
			if (t->speaker->IDs != NULL)
				free(t->speaker->IDs);
			if (t->speaker->code != NULL)
				free(t->speaker->code);
			free(t);
		}
	} else {
		t = rp;
		p = rp;
		while (p != NULL) {
			if (p->isTemp)
				break;
			t = p;
			p = p->next;
		}
		if (rp != p)
			t->next = NULL;
		else
			rp = NULL;
		while (p != NULL) {
			t = p;
			p = p->next;
			lpt = t->speaker->firstline;
			t->speaker->firstline = NULL;
			t->speaker->lastline = NULL;
			while (lpt!=NULL) {
				tmp_line = lpt;
				lpt = lpt->nextline;
				speaker_free_line(tmp_line);
			}
			if (t->speaker->fname != NULL)
				free(t->speaker->fname);
			if (t->speaker->IDs != NULL)
				free(t->speaker->IDs);
			if (t->speaker->code != NULL)
				free(t->speaker->code);
			free(t);
		}
	}
	return(rp);
}


/* 
 * speaker_free_up_tier
 *
 * Insert a dependent tier.
 *

void speaker_free_up_tier (Code *p)
{
	Code *t;

	while (p != NULL) {
		t = p;
		p = p->next;
		free(t);
	}
}
*/
/* 
 * speaker_insert_tier
 *
 * Insert a dependent tier.
 *

Code * speaker_insert_tier (Code *c, char *name)
{
	Code *cptr;
	char * pt;
	int len;

	cptr = (Code *) malloc (sizeof(Code));

	if (cptr==NULL) {
		fprintf(stderr, "Cannot allocate memory\n");
		return NULL;
	}

	memset(cptr, 0, sizeof(Code));
	pt = name;
	len = strlen(pt);
	if (len <= MAX_LEN_CODE-2) {   // changed this to allow other than 3 char tiers
		if (*pt != '*' && *pt != '%' && *pt != '@')
			strcpy(cptr->code,"*");
		else
			cptr->code[0] = EOS;
		strcat(cptr->code,pt);

		for (len=1; cptr->code[len] != EOS;) {
			if (cptr->code[len] == '*')
				strcpy(cptr->code+len, cptr->code+len+1);
			else
				len++;
		}
	}

	cptr->next = c;

	return cptr;
}
*/

/*
 * speaker_fprint_tiers
 *
 * Print the list of dependent tiers for a speaker.
 *
 
void speaker_fprint_tiers (FILE *ofp, VOCDSPs *speakers, Tier *dependents)
{
   VOCDSPs *spks;
   Tier *dptr;

   fprintf(ofp,"Main Tiers:\n");
   spks = speakers;
   while (spks != NULL) {
      fprintf(ofp,"%s\n",spks->speaker->code);
      spks = spks->next;
   }
// lxs lxslxs lxs
   fprintf(ofp,"Dependent Tiers:\n");
   dptr = dependents;
   while (dptr!=NULL) {
      fprintf(ofp,"  %s\n", dptr->code);
      dptr = dptr->next;
   }
}
*/
/*
 * speaker_exclude_speaker
 * 
 * Exclude a speakers' main tier.
 *

int speaker_exclude_speaker(VOCDSPs ** s, char *name, char isTemp)
{
	char * pt;
	VOCDSPs *newS;
	VOCDSP  *spk;
	VOCDSPs *speakers;
	int len;

	pt = name;    // "*XXX"

	len = strlen(pt);
	if (len <= 1) {
		fprintf(stderr, "ALL speakers can not be excluded.\n");
		return 0;
	}

	speakers = *s;

	if (speakers==NULL) {
		newS = (VOCDSPs *) malloc (sizeof(VOCDSPs));

		if (newS==NULL) {
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return 0;
		}
		memset(newS, 0, sizeof(VOCDSPs));

		newS->isTemp = isTemp;
		newS->speaker = (VOCDSP *) malloc (sizeof(VOCDSP));
		if (newS->speaker==NULL) {
			free(newS);
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return 0;
		}
		memset(newS->speaker, 0, sizeof(VOCDSP));
		speakers = newS;
		*s = newS;
	} else  {   // speakers already present on the list

		// move to last speaker on list
		while (speakers->next != NULL) {
			speakers = speakers->next;
		}

		// add new speaker to the end of the list
		newS = (VOCDSPs *) malloc (sizeof(VOCDSPs));
		if (newS==NULL) {
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return 0;
		}
		memset(newS, 0, sizeof(VOCDSPs));

		newS->isTemp = isTemp;
		newS->speaker = (VOCDSP *) malloc (sizeof(VOCDSP));
		if (newS->speaker==NULL) {
			free(newS);
			fprintf(stderr, "speaker_add_speaker: no memory\n");
			return 0;
		}
		memset(newS->speaker, 0, sizeof(VOCDSP));
		speakers->next = newS;
		speakers = newS;
	}

	spk = speakers->speaker;

	if (len<=MAX_LEN_CODE-1) {
		strcpy(spk->code,pt);
	}
	return 1;
}
*/

/* 
 * speaker_create_sequence
 *
 * Create a single sequence of tokens from speaker tokens.
 *
 */

int speaker_create_sequence(VOCDSP *speaker, char ***token_seq)
{
   int tokens;
   Line *lpt;
   char **t_seq, **wp;
//   VOCDSPs *Spk_pt;
   int i;

   /* get the total token count */
//   Spk_pt = speakers;
   tokens = 0;
//   while (Spk_pt !=NULL) {
      tokens += speaker_tokencount (speaker);
//      tokens += speaker_tokencount (Spk_pt->speaker);
//      Spk_pt = Spk_pt->next;
//   }

   /* create the array to hold the pointers to the words */
   t_seq = (char **) malloc (tokens * sizeof(char*));
   if (t_seq==NULL) {
      fprintf(stderr,"speaker_create_sequence: no memory\n");
      return -1;
   }

   wp = t_seq;
//   Spk_pt = speakers;
//   while (Spk_pt != NULL) {
      lpt = speaker->firstline;
//      lpt = Spk_pt->speaker->firstline;
      while (lpt!=NULL) {
         for (i=0;i<lpt->no_words;i++)
             *wp++ = lpt->words[i];
         lpt = lpt->nextline;
      }  
//      Spk_pt = Spk_pt->next;
//   }
 
   *token_seq = t_seq;
   return tokens;
}
 
/*
 * speaker_utterances
 *
 * Print a list of the speaker utterances.
 
void speaker_utterances(VOCDSPs *speakers, FILE *ofp)
{
   VOCDSPs *Spk_pt;

   Spk_pt = speakers;                // print the lines
   while (Spk_pt != NULL) {
      speaker_fprint_speaker(ofp, Spk_pt->speaker);
      Spk_pt = Spk_pt->next;
   }
}
*/
/* 
 * speaker_free_line
 *
 * Free up a line.
 *
 */
void speaker_free_line (Line *line)
{
   char **words;
   int i;

   words = line->words;
   for (i=0; i<line->no_words;i++)
      free(words[i]);

   free (line);
}

/* 
 * speaker_parse_file
 *
 * Parse a CHAT file.
 *
 
int speaker_parse_file( FILE * fp, VOCDSPs **speakers, Tier * dependent_tiers) {

	char		isInclude;
	char        * pt;
	char   		line[MAX_LINE];
	char 		code[MAX_LEN_CODE];
	int         into_dependent_tiers = 0;
	int			i;
	long		lineno;
	Tier        * current_dptr=NULL;
	VOCDSP      * current_speaker = NULL;
	VOCDSPs     * Spk_pt;

	lineno = 0L;
	while (1) { 

		pt = line;
		if (fgets(pt,MAX_LINE,fp)==NULL) {
			break;
		}
		if (!uS.isUTF8(pt))
			lineno++;
		switch (line[0]) {
			case '@'  : // Header line
				i = 0;
				while (*pt != ':' && *pt != EOS) {
					if (i < MAX_LEN_CODE-1)
						code[i++] = *pt;
					pt++;
				}
				if (*pt == ':')
					pt++;
				code[i] = EOS;
				while (isSpace(*pt))
					pt++;
				break;
			case '*'  : // main line
				current_speaker=NULL;
				current_dptr = NULL;
				into_dependent_tiers = 0;

				Spk_pt = *speakers;
//				isInclude = Spk_pt->speaker->include;
				isInclude = 0;
				i = 0;
				while (*pt != ':' && *pt != EOS) {
					if (i < MAX_LEN_CODE-1)
						code[i++] = *pt;
					pt++;
				}
				if (*pt == ':')
					pt++;
				code[i] = EOS; 

				if (isInclude) {
					while (Spk_pt!=NULL) {
						if (uS.partcmp(code,Spk_pt->speaker->code,FALSE,FALSE)) {
							if (!nomain) {
								if (!speaker_add_line(Spk_pt->speaker, pt, lineno)) {
									printf("cannot add line\n");
									break;
								} 
							}
							current_speaker = Spk_pt->speaker;
							break;
						}

						Spk_pt = Spk_pt->next;
					}
				} else {
					while (Spk_pt!=NULL) {
						if (uS.partcmp(code,Spk_pt->speaker->code,FALSE,FALSE)) {
							break;
						}
						Spk_pt = Spk_pt->next;
					}
// Does not work!!! lxs lxslxs
					if (Spk_pt == NULL) {
						if (!speaker_add_line(Spk_pt->speaker, pt, lineno)) {
							printf("cannot add line\n");
							break; 
						}
						current_speaker = Spk_pt->speaker;
					}
// Does not work!!! lxs lxslxs
				}
				break;

			case '%'  : // dependency line
				current_dptr = NULL;

				if (into_dependent_tiers==0)
					into_dependent_tiers = 1;

				if (current_speaker!=NULL) {
					// search the dependents for this speaker
					Tier *dptr;

					i = 0;
					while (*pt != ':' && *pt != EOS) {
						if (i < MAX_LEN_CODE-1)
							code[i++] = *pt;
						pt++;
					}
					if (*pt == ':')
						pt++;
					code[i] = EOS; 

					dptr = dependent_tiers;
					while (dptr !=NULL) {
						// Is this dependent tier to be included?
						if (uS.partcmp(code,dptr->code,FALSE,FALSE)) {
							if (!speaker_add_line (current_speaker, pt, lineno)) {
								fprintf(stderr,"cannot add line\n");
								break;
							}
							current_dptr = dptr;
							break;
						}
						dptr = dptr->next;
					}
				}
				break;
			default   : ; // fprintf(stderr, "unrecognised symbol \"%o\"\n", line[0]) ;
		}

	} // while

	return 0;

}
*/


/*
 * make a copy of a speakers data structure & contents
 * NB: Need to clean up free space properly!
 

VOCDSPs * speaker_copy_speaker(VOCDSPs *original)
{
   VOCDSPs *oSpk_pt, *cSpk_pt, *headSpk_pt, *tempSpk_pt;
   Line *olpt, *clpt, *tmpline;
   int i, len;

   cSpk_pt = NULL;
   headSpk_pt = NULL;
   tempSpk_pt = NULL;
   oSpk_pt = original;
   while (oSpk_pt!=NULL) {
      // Add a new speaker
      tempSpk_pt = (VOCDSPs *) malloc (sizeof(VOCDSPs));
      if (tempSpk_pt==NULL) {
         fprintf(stderr, "speaker_copy_speaker: no memory\n"); 
         return NULL;
      }
      memset(tempSpk_pt,0, sizeof(VOCDSPs));
      if (headSpk_pt==NULL) {
         headSpk_pt = tempSpk_pt;
      }
      else {
         cSpk_pt->next = tempSpk_pt;
      }
      cSpk_pt    = tempSpk_pt;

      // For this speaker, add the speaker
      if (oSpk_pt->speaker == NULL) {  // shouldn't be null
         free(cSpk_pt);
         fprintf(stderr, "speaker_copy_speaker: no speaker to copy\n"); 
         return NULL;  // XXX Should this be return NULLL XXX
      }

      // add the speaker data structure
      cSpk_pt->speaker = (VOCDSP *) malloc (sizeof(VOCDSP));
      if (cSpk_pt->speaker==NULL) {
         free(cSpk_pt);
         fprintf(stderr, "speaker_copy_speaker: no memory\n");
         return NULL;  // XXX Should this be return NULLL XXX
      }
      memset(cSpk_pt->speaker, 0, sizeof(VOCDSP));
      // copy the character code
	   cSpk_pt->isTemp = oSpk_pt->isTemp;
	   strcpy(cSpk_pt->speaker->code,oSpk_pt->speaker->code);
      // copy the include
      // copy all the lines, and point to both the first line and the last line
      olpt = oSpk_pt->speaker->firstline;
      while (olpt!=NULL) {
         // create a new Line structure
         tmpline = (Line *) malloc (sizeof(Line));
         if (tmpline==NULL) {
            fprintf(stderr, "speaker_copy_speaker (line): no memory\n");
            return NULL;
         }
         tmpline->nextline = NULL;
         tmpline->no_words = olpt->no_words;
         // now the words - need to malloc an array of the appropriate size
         tmpline->words = (char **) malloc (olpt->no_words * sizeof(char *));
         // check that this worked, then copy the words across
         for (i=0;i<olpt->no_words; i++)  {
            len = strlen(olpt->words[i]);
            tmpline->words[i] = (char *) malloc(len+1);
            strncpy(tmpline->words[i],olpt->words[i], len+1);
         }
         // need a pointer to keep track of the lines as they are added
         if (cSpk_pt->speaker->firstline == NULL) {
            cSpk_pt->speaker->firstline = tmpline;
         }
         else {
            clpt->nextline = tmpline;
         }
         cSpk_pt->speaker->lastline  = tmpline;
         clpt = tmpline;

         olpt = olpt->nextline;
      }
      oSpk_pt = oSpk_pt->next;
   }
   return headSpk_pt;
}
*/
