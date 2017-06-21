#ifndef CL_SEARCH
#define CL_SEARCH

extern "C"
{

#define SearchPresets struct SearchPresetsS
struct SearchPresetsS {
	const char *label;
	char include;
	char exclude;
	const char *POS;
	const char *stem;
	const char *prefix;
	const char *suffix;
	const char *fusion;
	const char *trans;
	char oOption;
} ;
extern SearchPresets presets[];

}

#endif /* CL_SEARCH */
