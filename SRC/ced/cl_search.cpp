#include "ced.h"
#include "cl_search.h"


SearchPresets presets[] = {
//	label,include,exclude,
//		Part-of-speech,stem,prefix,suffix,fusion,trans,oOption
	{"all nouns only", true, false, 
		"n,n:*", "", "", "", "", "", false} ,

	{"all verbs only", true, false,
		"v,cop,aux,aux:*,part", "", "", "", "", "", false} ,

	{"all modal words", true, false,
		"aux", "will,shall,can,may,must,should,could", "", "", "", "", false} ,

	{"all \"Wh\" (*:wh) words", true, false,
		"*:wh", "", "", "", "", "", false} ,

	{"all suffixes merged", true, false,
		"", "", "", "*", "|*", "", true} ,

	{"all suffixes+stems+POS merged", true, false,
		"*|*", "*|*", "", "*", "|*", "", true} ,

	{"all stems merged", true, false,
		"", "*", "", "", "", "", true} ,

	{"all noun stems merged", true, false,
		"n:*,n", "*", "", "", "", "", true} ,

	{"all noun stems and affix merged", true, false,
		"n:*,n", "*", "+*", "", "", "", true} ,

	{"all verb stems merged", true, false,
		"v,cop,aux,aux:*,part", "*", "", "", "", "", true} ,

	{"all past tense verbs merged", true, false,
		"v,cop,aux,aux:*,part", "*", "", "PAST", "", "", true} ,

	{"all \"adv\" stems merged", true, false,
		"adv,adv:*", "*", "", "", "", "", true} ,

	{"all stems & parts of speech merged", true, false,
		"*", "*", "", "", "", "", true} ,

	{"exclude neologisms and unknowns", false, true,
		"neo,unk", "", "", "", "", "", true} ,

	{NULL, true, false, "", "", "", "", "", "", false}
} ;
