/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************
 * filter.c                                                *
 * -  filters for processing input from the CHAT files.    *
 *                                                         *
 * VOCD Version 1.0, August 1999                           *
 * G T McKee, The University of Reading, UK.               *
 ***********************************************************/

#if defined(_MAC_CODE) || defined(_WIN32) || defined(UNX)
#include "cu.h"
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif
#include "vocdefs.h"
#include "filter.h"
#include "tokens.h"

/*
 * filter_angle
 *
 * filter the elements in angled ("<", ">") brackets.
 * the filter removes the angle brackets, leaving the words.
 * this is the default.
 *
 */
int filter_angle (Line *line, int c1, int c2, long flags)
{
   int i,k;
   char ** wp;
   int angle = 0;
   char *pt, *pd;

   wp = line->words;
   for (i=0,k=0;i<line->no_words;i++) {
      if (*wp[i]==c1) {
         angle++;
      }
      if (angle) {
         pd = wp[i];
         pt = wp[i];
         while (*pt != '\0') {
            if (*pt == c1)
               pt++;
            else if (*pt == c2) {
               *pd = '\0';
               angle--;      /* leaving the angle brackets */
               break;
            }
            else
               *pd++ = *pt++;
         }
         *pd = '\0';
      }
      wp[k++] = wp[i];
   }
   line->no_words = k;
   if (angle) { 
      fprintf(stderr, "filter_angle: not terminated\n");
      return -1;
   }
   return 1;
}

/*
 * filter_enclosed:
 *
 * filter a set of words enclosed in brackets.
 * c1 is opening bracket, c2 is closing bracket.
 *
 */
int filter_enclosed (Line *line, int c1, int c2, long flags)
{
   int i,k;
   int replacement = 0;
   int marked = 0;
   char ** wp;
   int enclosure = 0;
   char *pt;

   wp = line->words;
   for (i=0,k=0;i<line->no_words;i++) {
      if ( (*wp[i]==c1) && enclosure==0 ) {
         enclosure++;
         if (flags & COLON) {
            if (wp[i][1]==':' && wp[i][2]!='=') {  /* replacement text */
               if (i>0) {
                  i++; k--;
                  enclosure++;
                  replacement = 1;
               }
            }
         }
      }
      if (enclosure==1) {
         pt = wp[i];
         while (*pt != '\0')
            if (*pt++ == c2)
               enclosure--;
         continue;
      }
      if (enclosure==2 && replacement==1) {
         /* the previous token does not contain a terminating ">" */
         int j; int len; 

#ifdef TEST_BUG
         len = strlen(wp[k]);
         if ( wp[k][len-1] != '>' ) {
            wp[k][0] = '\0';
            marked++;
            replacement = 0;
            /* continue; */
         }
#endif
         len = strlen(wp[k]);
         if ( wp[k][len-1] != '>' ) {
            if (wp[k][0] == '<') { /* detected an opening angle bracket */
               wp[k][1] = '\0';
               replacement = 0;
               k++;
            }
            else {
               wp[k][0] = '\0';
               marked++;
               replacement = 0;
               /* continue; */
            }
         }
         /* check here for the replaced words - should be the sequence */
         else if (wp[k][len-1] == '>') { /* track back to "<" */
            for (j=k; j>=0; j--) {
               char *pt;

               pt = wp[j];
               while (*pt!='\0') {
                  if (*pt == '<')  {  /* detect start of retracing */
                     break;
                  }
                  pt++;
               }
               if (*pt=='<') {        /* terminate track back */
                  wp[j][0] = '\0';
                  marked++;
                  break;
               }
               wp[j][0] = '\0';       /* exclude intermediate word */
               marked++;
            }
            
         }
         replacement = 0;
      }  /* end if */ /* replace word or sequence in angle brackets */
      if (enclosure==2) {
         pt = wp[i];
         while (*pt != '\0')
            if (*pt == c2) {
               *pt = '\0';
               enclosure-=2;
            }
            else
               pt++;
      }
      wp[k++] = wp[i];
   }
   line->no_words = k;
   if (enclosure) { 
      fprintf(stderr, "append_to_line: non terminated enclosure\n");
/* GTM TEST CODE */
//fprintf(stderr, "enclosure = %i\n", enclosure);
//speaker_fprint_line(stderr, line->no_words, line->words);
      return -1;
   }
   return 1;
}


/*
 * filter_exclude:
 *
 * exclude a token from a line if it is in word list "exclude".
 *
 */
void filter_exclude (Line *line, NODE *exclude)
{
   int i,k;
   char **words;

   words = line->words;
   for (i=0,k=0;i<line->no_words; i++) {
      if ( tokens_member(words[i], exclude) ) {
         free(words[i]);
         continue;
      }
      words[k++] = words[i]; 
   }
   line->no_words = k;
}

/*
 * filter_split_token_before:
 *
 * split a token, starting from the character held in "string".
 * used, for example, to split "abc[de" into "abc" + "[de".
 *

 static int filter_split_token_before (Line *line, NODE *split_item, char * string, int split_category)
{
	int  i,k;
	char **words;
	char **new_words;
	int  new_len;
	int  len;
	int  split_pos;
	int  substr_len;
	char *c, *pt;

	// count no. of words that need to be split
	words = line->words;
	for (i=0,k=0;i<line->no_words; i++) {
		if ( tokens_member(words[i], split_item) ) {
			// GTM TEST CODE
			//printf("word_to_split %s\n",words[i]);
			k++;
		}
	}
	// GTM TEST CODE
	//printf("k = %i\n", k);
	if (k==0)  // no extension needed
		return 0;
	// make space for the new words
	new_len = k + line->no_words;
	new_words = (char**)malloc(new_len*sizeof(char*));
	if (new_words==NULL) {
		fprintf(stderr, "filter_split_token_before: no memory\n");
		return -1;
	}
	// copy across, splitting the appropriate words
	words = line->words;
	for (i=0,k=0;i<line->no_words; i++) {
		if ( tokens_member(words[i], split_item) ) {
			// find the position of the character
			//split_pos = 0;
			// point to the word to be split and skip the first
			// character. We assume there is at least one character
			// other than the split character at the head of the
			// word. Otherwise split has no meaning.
			// above caused an error. So I think i need
			// to set split_pos to 1 to begin with!
			pt = words[i]; pt++;
			split_pos = 1;
			while (*pt != string[0]) {
				split_pos++;
				pt++;
			}
			if (split_category) { // category is 1 or 2.
				substr_len = strlen(string);
				pt += substr_len;           // move to first char after string
				split_pos += substr_len;
			}
			// word to be inserted
			len = strlen(pt) + 1;
			c = (char*) malloc(len*sizeof(char));
			if (c==NULL) {
				fprintf(stderr, "filter_split_before: no memory\n");
				free(new_words);
				return -1;
			}
			strcpy(c,pt);
			// sever tail of existing word
			if (split_category==2) {
				words[i][split_pos-substr_len] = '\0';
			}
			else
				words[i][split_pos] = '\0';
			// copy over the two words
			new_words[k++] = words[i];
			new_words[k++] = c;
			continue;
		}
		new_words[k++] = words[i];
	}
	free(line->words);
	line->words = new_words;
	line->no_words = new_len;
	return 1;
}
*/
/*
static void filter_split_words (VOCDSPs *speakers, char * string, int split_category)
{
   NODE *split_item;
   char p[10];
   VOCDSPs *Spk_pt;
   Line *lpt;

   if (split_category==1) {
      strcpy(p,"*");
      strcat(p,string);
      strcat(p,"_*");
   }
   else if (split_category==2) {
      strcpy(p,"_*");
      strcat(p,string);
      strcat(p,"_*");
   } else { // split_category = 0
      strcpy(p,"_*");
      strcat(p,string);
      strcat(p,"*");
   }
   split_item = tokens_create_node(2);
   tokens_insert (split_item, p);
   Spk_pt = speakers;
   while (Spk_pt!=NULL) {
      lpt = Spk_pt->speaker->firstline;
      while (lpt!=NULL) {
         int ret = 0;

         do {
            ret = filter_split_token_before(lpt,split_item, string, split_category);
// GTM TEST CODE
//printf("fsw1: ret = %i\n", ret);
//printf("split_token_before:: %s\n", p);
//speaker_print_line(lpt->no_words, lpt->words);
         } while (ret == 1);
//printf("fsw: ret = %i\n", ret);
         lpt = lpt->nextline;
      }
      Spk_pt = Spk_pt->next;
   }
// GTM TEST CODE
//printf("HERE\n");
   tokens_free_tree(split_item);
}
*/
/*
 * filter_split_before
 *
 * split string from the end of the word, starting from
 * the character held in string.
 *
 
void filter_split_before (VOCDSPs *speakers, char * string)
{
	filter_split_words(speakers, string, 0);
}
*/
/*
 * filter_split_after
 *
 * split string from the end of the word, starting from
 * the end of string.
 *
 
void filter_split_after (VOCDSPs *speakers, char * string)
{
	filter_split_words(speakers, string, 1);
}
*/
/*
 * filter_split_at
 *
 * split string at a specified point, deleting the character/substring
 * used to locate the point.
 *
 
void filter_split_at (VOCDSPs *speakers, char * string)
{
	filter_split_words(speakers, string, 2);
}
*/
/*
 * filter_split_token_tail:
 * 
 * split a specified substring from the end of matching tokens.
 * split_item is used to do the matching, and string is the
 * substring in question.
 *

static int filter_split_token_tail (Line *line, NODE *split_item, char * string)
{
   int  i,k;
   char **words;
   char **new_words;
   int  new_len;
   int  len;
   int  split_pos;
   char *c;

   // count no. of words that need to be split
   words = line->words;
   for (i=0,k=0;i<line->no_words; i++) {
      if ( tokens_member(words[i], split_item) ) {
         k++;
      }
   }
   if (k==0)  // no extension needed
      return 0;
   // make space for the new words
   new_len = k + line->no_words;
   new_words = (char**)malloc(new_len*sizeof(char*));
   if (new_words==NULL) {
      fprintf(stderr, "filter_split_token_tail: no memory\n");
      return -1;
   }
   // copy across, splitting the appropriate words
   words = line->words;
   for (i=0,k=0;i<line->no_words; i++) {
      if ( tokens_member(words[i], split_item) ) {
         // word to be inserted
         len = strlen(string) + 1;
         c = (char*) malloc(len*sizeof(char));
         if (c==NULL) {
            fprintf(stderr, "filter_split_token_tail: no memory\n");
            free(new_words);
            return -1;
         }
         strcpy(c,string);
         // sever tail of existing word
         split_pos = strlen(words[i]) - strlen(string); 
         words[i][split_pos] = '\0';
         // copy over the two words
         new_words[k++] = words[i];
         new_words[k++] = c;
         continue;
      }
      new_words[k++] = words[i];
   }
   free(line->words);
   line->words = new_words;
   line->no_words = new_len;
   return 1;
}
*/
/*
 * filter_split_tail
 *
 * split string from the end of the word, and actually keep
 * as a separate word.
 *
 
void filter_split_tail (VOCDSPs *speakers, char * string)
{
	NODE *tail_item;
	char p[10];
	VOCDSPs *Spk_pt;
	Line *lpt;
	
	strcpy(p,"*");
	strcat(p,string);
	tail_item = tokens_create_node(2);
	tokens_insert (tail_item, p);
	Spk_pt = speakers;
	while (Spk_pt!=NULL) {
		lpt = Spk_pt->speaker->firstline;
		while (lpt!=NULL) {
			filter_split_token_tail(lpt,tail_item, string);
			lpt = lpt->nextline;
			
		}
		Spk_pt = Spk_pt->next;
	}
	tokens_free_tree(tail_item);
}
*/
/*
 * filter_terminators
 *
 * filter a set of terminators from a line;
 * assumes one character terminators.
 *
 */
void filter_terminators (Line *line, NODE *terminators)
{
   int i, len;
   char **words;

   words = line->words;
   for (i=0;i<line->no_words; i++) {
      if ( tokens_member(words[i], terminators) ) {
         len = strlen(words[i]);
         words[i][len-1] = '\0';
      }
   }
}

/*
 * filter_parentheses_from_word:
 * 
 * filter parentheses within a word
 *

static void filter_parentheses_from_word(char *word, int c1, int c2, long flags)
{
   char *pt, *pd;
   int substring = 0;

   pt = word;
   pd = word;
   while (*pt!='\0') {
      if (*pt == c1) {
         substring++;
         if (flags && REMOVE_INC)
            substring++;
         pt++;
         continue;
      }
      if (substring==1) {
         if (*pt == c2) {
            substring--;
            pt++;
         }
         else {
            *pd++ = *pt++;
         }
         continue;
      }
      if (substring==2) {
         if (*pt==c2) {
            substring-=2;
         }
         pt++;
         continue;
      }
      *pd++ = *pt++;

   } // while
   *pd = *pt;
}
*/
/*
 * filter_parentheses
 *
 * remove parentheses within words for a speaker
 *
 
void filter_parentheses(VOCDSPs *speakers, long flags)
{
	VOCDSPs *Spk_pt;
	Line *lpt;
	char *pt;
	int i;
	
	Spk_pt = speakers;
	while (Spk_pt != NULL) {
		lpt = Spk_pt->speaker->firstline;
		while (lpt!=NULL) {
			for (i=0;i<lpt->no_words;i++) {
				pt = lpt->words[i];
				filter_parentheses_from_word(pt,'(',')', flags);
			}
			lpt = lpt->nextline;
		}
		Spk_pt = Spk_pt->next;
	}
}
*/
/*
 * filter_capitalisation:
 *
 * retain tokens which start with a capital letter.
 *
 */
void filter_capitalisation(VOCDSP *speaker)
{
//	VOCDSPs *Spk_pt;
	Line *lpt;
	char *pt;
	int i, k;
	
//	Spk_pt = speakers;
//	while (Spk_pt != NULL) {
		lpt = speaker->firstline;
//		lpt = Spk_pt->speaker->firstline;
		while (lpt!=NULL) {
			for (i=0,k=0;i<lpt->no_words;i++) {
				pt = lpt->words[i];
				if (*pt >= 'a' && *pt <= 'z') {
					free(lpt->words[i]);
					continue;
				}
				lpt->words[k] = lpt->words[i];
				uS.lowercasestr(lpt->words[k], &dFnt, FALSE);
				k++;
			}
			lpt->no_words = k;
			lpt = lpt->nextline;
		}
//		Spk_pt = Spk_pt->next;
//	}
}

/*
 * filter_lowercase:
 * 
 * change all uppercase letters to lower case
 *
 
void filter_lowercase(VOCDSPs *speakers)
{
   VOCDSPs *Spk_pt;
   Line *lpt;
   char *pt;
   int i;

   Spk_pt = speakers;
   while (Spk_pt != NULL) { 
      lpt = Spk_pt->speaker->firstline;
      while (lpt!=NULL) {
         for (i=0;i<lpt->no_words;i++) {
             pt = lpt->words[i];
             while (*pt!='\0') {
                if (*pt >= 'A' && *pt <='Z')
                   *pt+=32;
                pt++;
             }
         }
         lpt = lpt->nextline;
      }
      Spk_pt = Spk_pt->next;
   }
}
*/

int filter_overlapping ( Line *line, long flags )
{
   int i,k;
   int len;
   char ** wp;
   int angle = 0;
 
   wp = line->words;
   /* search for [>] pattern, return its index;
      we know the pattern is present! */
   for (i=0;i<line->no_words;i++) {
      if (strncmp (wp[i], "[<]",3) == 0)
         break;
   }
   if (i==line->no_words)  /* for some reason we didn't find it! */
      return -1;
   /* Delete [*] here, leave as an empty word, it will be tidied up later */
   wp[i][0]='\0';
   for (k=i-1; k>=0; k--) {
      len = strlen(wp[k]);
      for (i=len-1; i>=0; i--) {
         int ch;
         ch = wp[k][i];
         if (ch=='>') {
            if (angle==0) {
               angle++;
               wp[k][i]='\0';  /* assuming '>' at end of word */
            }
            else
               angle++;     /* avoid intermediate angle brackets. */
         }
         if (ch=='<') {
            angle--;
            if (angle==0) {/* found opening bracket, delete it */
               char *s, *d;
               s = wp[k]; d = wp[k];
               s++;  /* skip over '<' - assuming at front of word! */
               while (*s != '\0')
                 *d++ = *s++;
               *d = '\0';
            }
         }
      }
   }
   return 1;
}
