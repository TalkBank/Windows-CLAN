/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/***********************************************************
 * args.c                                                  *
 * - command line and text line parsing                    *
 *                                                         *
 * VOCD Version 1.0, August 1999                           *
 * G T McKee, The University of Reading, UK.               *
 ***********************************************************/

#if defined(_MAC_CODE) || defined(_WIN32)
#include "cu.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
  #if !defined(UNX)
	#include <malloc.h>
  #endif
#endif

#include "args.h"


/**************************************************
 *  simple utility for parsing command strings    *
 **************************************************/

/* 
 * parse_args: segments the string in buf into symbols 
 * the sequence of symbols are returned in argv
 * returns the no. of symbols; 
 */

int parse_args(char * buf, char ***argv)
{
   char *ps, *ph;
   char **v;
   int c = 0, len=0;

   ps = buf;

   while (*ps!='\0') {
     if (*ps=='\n' || *ps=='\r') {
        *ps='\0';
        break;
     }
     ps++;
   }

   ps = buf;
   while (*ps ==' ' || *ps == '\t') ps++;   /* skip leading spaces */



   ph = ps;

   if (*ps=='\0')
      return c;


   while (*ps!='\0') {       /* count no. of arguments */
      ps++;
      if (*ps==' ' || *ps =='\0') {
         c++;
         while (*ps==' ') ps++; /* skip spaces */
      }
   }

   v = (char **) malloc (c * sizeof(char *));
   c = 0;
   ps=ph;
   while (*ps!='\0') {       /* copy args into a list */
      ps++;  len++;
      if (*ps==' ' || *ps =='\0') {
         v[c]= (char *) malloc(len+1);
         strncpy(v[c],ph,len);
         v[c][len]='\0';
         c++;
         while (*ps==' ') ps++; /* skip spaces */ 
         ph=ps;
         len=0;
      }  
   }  

   *argv = v;
   return c;
 
}

/**************************************************
 *  truncate filenames ending in ".cha"           *
 **************************************************/
/*
 * truncate_cha
 *
 * If filename contains ending '.cha', remote it.
 * Used when sending results to a file with same name,
 * but different extension.
 */
void truncate_cha (FNType *fname)
{
	int res;
   FNType * pt;

   pt = fname;
   while (*pt != '\0') {
      if (*pt=='.') {
#ifdef UNX
		  res = strcmp(pt, ".cha");
#else
		  res = uS.FNTypecmp(pt, ".cha", 4);
#endif
		  if (res==0) {
            *pt = '\0';
            return;
         }
      }   
      pt++;
   }
}

