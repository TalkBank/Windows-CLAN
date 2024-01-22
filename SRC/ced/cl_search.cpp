#include "ced.h"
#include "cl_search.h"


SearchPresets presets[] = {
//	label,include,exclude,
//		Part-of-speech,stem,prefix,suffix,fusion,trans,oOption
	{"all nouns only", true, false, 
		"n,n:*", "", "", "", "", "", false} ,

	{"all verbs only", true, false,
		"v,cop,aux,aux:*,mod,mod:*,part", "", "", "", "", "", false} ,

	{"all modal words", true, false,
		"mod,mod:aux", "", "", "", "", "", false} ,

	{"all suffixes merged", true, false,
		"", "", "", "*", "|*", "", true} ,

	{"all suffixes+stems+POS merged", true, false,
		"*|*", "*|*", "", "*", "|*", "", true} ,

	{"all stems merged", true, false,
		"", "*", "", "", "", "", true} ,

	{"all noun stems merged", true, false,
		"n:*,n", "*", "", "", "", "", true} ,

	{"all plural noun stems merged", true, false,
		"n:*,n", "*", "", "PL", "", "", true} ,

	{"all noun stems and suffixes merged", true, false,
		"n:*,n", "*", "", "+*", "+*", "", true} ,

	{"all verb stems merged", true, false,
		"v,cop,aux,aux:*,mod,mod:*,part", "*", "", "", "", "", true} ,

	{"all past tense verbs merged", true, false,
		"v,cop,aux,aux:*,mod,mod:*,part", "*", "", "PAST", "", "", true} ,

	{"all irregular past tense verbs merged", true, false,
		"v,cop,aux,aux:*,mod,mod:*,part", "*", "", "", "PAST", "", true} ,

	{"all \"adv\" stems merged", true, false,
		"adv,adv:*", "*", "", "", "", "", true} ,

	{"all stems & parts of speech merged", true, false,
		"*", "*", "", "", "", "", true} ,

	{"exclude neologisms and unknowns", false, true,
		"neo,unk", "", "", "", "", "", true} ,

	{NULL, true, false, "", "", "", "", "", "", false}
} ;
