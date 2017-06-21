/**********************************************************************
	"Copyright 1990-2014 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/


#include <ctype.h>
/*
#define isalnum my_isalnum
#define isalpha my_isalpha
#define isdigit my_isdigit
#define isspace my_isspace
#define islower my_islower
#define isupper my_isupper
#define tolower my_tolower
#define toupper my_toupper

extern char my_isalnum(char c);
extern char my_isalpha(char c);
extern char my_isdigit(char c);
extern char my_isspace(char c);
extern char my_islower(char c);
extern char my_isupper(char c);
extern char my_tolower(char c);
extern char my_toupper(char c);
*/
#if defined(_WIN32)
	#define isdigit(x) (x >= '0' && x <= '9')
#elif defined(_MAC_CODE)
	#include <wctype.h>
/*
  extern "C"
  {
	_MSL_IMP_EXP_C int      _MSL_CDECL iswalnum(wchar_t);
	_MSL_IMP_EXP_C int      _MSL_CDECL iswalpha(wchar_t);
	_MSL_IMP_EXP_C int 		_MSL_CDECL iswdigit(wchar_t);
	_MSL_IMP_EXP_C int 		_MSL_CDECL iswspace(wchar_t);
	_MSL_IMP_EXP_C int 		_MSL_CDECL iswlower(wchar_t);
	_MSL_IMP_EXP_C int 		_MSL_CDECL iswupper(wchar_t);
	_MSL_IMP_EXP_C wchar_t 	_MSL_CDECL towlower(wchar_t);
	_MSL_IMP_EXP_C wchar_t 	_MSL_CDECL towupper(wchar_t);
  }	
*/
#endif /* else _WIN32 */

