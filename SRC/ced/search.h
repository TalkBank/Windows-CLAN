#ifndef _C_SEARCH_
#define _C_SEARCH_
extern "C"
{

	extern unCH SearchString[];
	extern unCH ReplaceString[];
	extern char CaseSensSearch;
	extern char SearchWrap;
	extern char SearchFromList;
	extern char ReplaceFromList;
	extern char searchListFileName[];
	extern char replaceListFileName[];
	extern char replaceExecPos;
	
	extern int  FindString(unCH *st, char updateScreen, char isWrap, char isSearchFromList);
	extern int  SearchForward(int i);
	extern int  SearchReverse(int i);
	extern int  Replace(int i);
	extern int  ReplaceAndFind(int i);
	extern int  replaceAndFindNext(char isOnlyFind);
	extern char readSearchList(FNType *fname);
	extern char replaceOne(char DoAll, int SrchStrLen, int ReplStrLen);
	extern short isThereSearchList(void);
	extern void init_Search(void);

#ifdef _MAC_CODE
	extern char FindDialog(unCH *SearchString, char ActiveSDirection);
	extern char ReplaceDialog(unCH *SearchString, unCH *ReplaceString, FNType *fname);
	extern char ContReplaceDialog(unCH *SearchString, unCH *ReplaceString);
#endif // _MAC_CODE
}

#endif // _C_SEARCH_
