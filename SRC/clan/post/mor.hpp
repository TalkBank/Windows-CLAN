/**********************************************************************
	"Copyright 1990-2024 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

/*=========================================================================================================================|
-- mor.hpp	- v1.0.0 - november 1998
	- values related to the MOR program.
|=========================================================================================================================*/

#ifndef __MOR_HPP__
#define __MOR_HPP__

#define WhiteSpaces " \t\b\r\n"
//#define PunctuationsAtEnd ".!,;?+/"
#define PunctuationsAtEnd ".!;?+/"
#define PunctuationsInside ""
#define AmbiguitySeparator "^"
#define MorTagMainSeparator '|'
#define MorTagComment '%'
#define MorTagTranslation "="
#define MorTagCompoundWord '+'
#define MorTagComplements "&-"
#define MorTagComplementsMainTag "&-|"
#define MorTagComplementsRegular '-'
#define MorTagComplementsIrregular '&'
#define MorTagComplementsPrefix '#'
#define MorTagComplementsPrefixes "#"

#define MultiCategory "multicat/"
#define MaxMultiCategory 12
#define MaxParts 24
#define TagGroups "$~"
#define Preposed "$"
#define Postposed "~"

// the following definition is the list of punctuation ignored by MOR
#define non_internal_pcts ":()-"

#endif
