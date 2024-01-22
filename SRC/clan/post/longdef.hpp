/**********************************************************************
	"Copyright 1990-2020 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef _LONGDEF_H_
#define _LONGDEF_H_

#if defined(POSTCODE) || defined(_MAC_CODE)
  #if __LP64__
	#define long32 int
  #else
	#define long32 long
  #endif
#else
	#define long32 long
#endif /* defined(POSTCODE) || defined(_MAC_CODE) */

#endif /* ifndef _LONGDEF_H_ */
