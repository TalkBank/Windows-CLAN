// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EB2FA987_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
#define AFX_STDAFX_H__EB2FA987_D6AD_11D0_8D74_00A02481E866__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// QTML and QuickTime

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

//#define _AFX_NO_BSTR_SUPPORT
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "MacWindows.h"
#include <ConditionalMacros.h>
#include <QTML.h>
#include <Movies.h>
#include <scrap.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EB2FA987_D6AD_11D0_8D74_00A02481E866__INCLUDED_)
