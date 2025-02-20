/**********************************************************************
	"Copyright 1990-2025 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef MY_CTYPE
#define MY_CTYPE

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
	_MSL_IMP_EXP_C int      _MSL_CDECL iswalnum(unCH);
	_MSL_IMP_EXP_C int      _MSL_CDECL iswalpha(unCH);
	_MSL_IMP_EXP_C int 		_MSL_CDECL iswdigit(unCH);
	_MSL_IMP_EXP_C int 		_MSL_CDECL iswspace(unCH);
	_MSL_IMP_EXP_C int 		_MSL_CDECL iswlower(unCH);
	_MSL_IMP_EXP_C int 		_MSL_CDECL iswupper(unCH);
	_MSL_IMP_EXP_C unCH 	_MSL_CDECL towlower(unCH);
	_MSL_IMP_EXP_C unCH 	_MSL_CDECL towupper(unCH);
  }	
*/
#endif /* else _WIN32 */

#endif // MY_CTYPE
