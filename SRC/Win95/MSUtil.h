#ifndef __MWERKS__
//#define _countof(array) (sizeof(array)/sizeof(array[0]))
// typedef char * va_list;
#define __va_start(parm)	(va_list) (&parm + 1)

extern BOOL AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);
extern BOOL AFXAPI _AfxCompareClassName(HWND hWnd, LPCTSTR lpszClassName);
#endif /* __MWERKS__*/