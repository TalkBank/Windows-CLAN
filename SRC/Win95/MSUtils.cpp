#include "ced.h"
#ifndef __MWERKS__
#include "msutil.h"
#endif

BOOL AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2)
{
#ifdef _MAC
	FSSpec fssTemp1;
	FSSpec fssTemp2;
	if (!UnwrapFile(lpszPath1, &fssTemp1) || !UnwrapFile(lpszPath2, &fssTemp2))
		return FALSE;
	return fssTemp1.vRefNum == fssTemp2.vRefNum &&
		fssTemp1.parID == fssTemp2.parID &&
		EqualString(fssTemp1.name, fssTemp2.name, false, true);
#else
	// use case insensitive compare as a starter
	if (lstrcmpi(lpszPath1, lpszPath2) != 0)
		return FALSE;

	// on non-DBCS systems, we are done
	if (!GetSystemMetrics(SM_DBCSENABLED))
		return TRUE;

	// on DBCS systems, the file name may not actually be the same
	// in particular, the file system is case sensitive with respect to
	// "full width" roman characters.
	// (ie. fullwidth-R is different from fullwidth-r).
	int nLen = lstrlen(lpszPath1);
	if (nLen != lstrlen(lpszPath2))
		return FALSE;
	ASSERT(nLen < _MAX_PATH);

	// need to get both CT_CTYPE1 and CT_CTYPE3 for each filename
	LCID lcid = GetThreadLocale();
	WORD aCharType11[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath1, -1, aCharType11));
	WORD aCharType13[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath1, -1, aCharType13));
	WORD aCharType21[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath2, -1, aCharType21));
#ifdef _DEBUG
	WORD aCharType23[_MAX_PATH];
	VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath2, -1, aCharType23));
#endif

	// for every C3_FULLWIDTH character, make sure it has same C1 value
	int i = 0;
	for (LPCTSTR lpsz = lpszPath1; *lpsz != 0; lpsz = _tcsinc(lpsz))
	{
		// check for C3_FULLWIDTH characters only
		if (aCharType13[i] & C3_FULLWIDTH)
		{
			ASSERT(aCharType23[i] & C3_FULLWIDTH); // should always match!

			// if CT_CTYPE1 is different then file system considers these
			// file names different.
			if (aCharType11[i] != aCharType21[i])
				return FALSE;
		}
		++i; // look at next character type
	}
	return TRUE; // otherwise file name is truly the same
#endif
}
/*
BOOL AFXAPI _AfxCompareClassName(HWND hWnd, LPCTSTR lpszClassName)
{
	ASSERT(::IsWindow(hWnd));
	WindowProc szTemp[32];
	::GetClassName(hWnd, szTemp, _countof(szTemp));
	return lstrcmpi(szTemp, lpszClassName) == 0;
}
*/