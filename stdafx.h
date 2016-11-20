// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#if !defined(_UNICODE) && !defined(UNICODE)
#	error This applications must be built with _UNICODE defined.
#endif

#ifdef NDEBUG
#   ifdef _WIN64
#       pragma optimize("gs", on)
#   else
#       pragma optimize("gsy", on)
#   endif
#endif

#if 0 && defined(_DEBUG)
// https://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
#   define _CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
/*
#   ifndef DBG_NEW
#       define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#       define new DBG_NEW
#   endif
*/
#endif // _DEBUG

#define _ATL_NO_MSIMG
#define _ATL_NOFORCE_MANIFEST
#define _ATL_NO_OPENGL
#define _ATL_NO_COM_SUPPORT
#define _ATL_NO_PERF_SUPPORT
#ifndef _ATL_STATIC_REGISTRY
#   define _ATL_STATIC_REGISTRY
#endif

#define _ATL_USE_CSTRING
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _WTL_NO_CSTRING

// This activates a slight modification in atlctrlx.h which prevents the link
// color being read from the IE registry settings
#ifndef _WTL_HYPERLINK_NO_IESETTINGS
#   define _WTL_HYPERLINK_NO_IESETTINGS
#endif
// This activates a slight modification in atlctrlx.h which removes certain
// sort types, in particular those which lead to a dependency to oleaut32.dll
#ifndef _WTL_SORTLISTVIEW_NO_OLEAUT32
#   define _WTL_SORTLISTVIEW_NO_OLEAUT32
#endif
#ifndef _WTL_NEW_PAGE_NOTIFY_HANDLERS
#   define _WTL_NEW_PAGE_NOTIFY_HANDLERS
#endif

// Change these values to use different versions
#ifndef WINVER
#   define WINVER           0x0500
#endif
#ifndef _WIN32_WINNT
#   define _WIN32_WINNT     0x0501
#endif
#ifndef _WIN32_IE
#   define _WIN32_IE        0x0501
#endif
#ifdef _RICHEDIT_VER
#   define _RICHEDIT_VER    0x0500
#endif

#if 1
#   define _ATL_NO_COMMODULE
#   include <atlbase.h>
#   include "fake_commodule.hpp"
#   undef _ATL_NO_COMMODULE
#else
#   include <atlbase.h>
#endif

#include <atlapp.h> /* chokes if _ATL_NO_COMMODULE is defined */
#include <atlmisc.h>
#include <atlsimpcoll.h>

typedef CAppModule CNtObjectsAppModule;
extern CNtObjectsAppModule _Module;

#include <atlwin.h>
#include <atlddx.h>
#include <Shellapi.h>

#if (_MSC_VER >= 1400)
#   if defined _M_IX86
#       pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#   elif defined _M_IA64
#       pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#   elif defined _M_X64
#       pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#   else
#       pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#   endif
#endif

#ifndef DDKBUILD
#include <delayimp.h>
#endif // DDKBUILD
#include "ntnative.h"

#ifndef DDKBUILD
    EXTERN_C int __cdecl DelayLoadError(LPCTSTR lpszFormat, ...);
    EXTERN_C void force_resolve_ntdll(void);
#endif // DDKBUILD

typedef struct _objtype_comment_t
{
    union {
        LPCTSTR pattern;
        LPCTSTR exact;
    } match;
    WORD resId;
    unsigned int case_insensitive:1;
    unsigned int pattern_match:1;
} objtype_comment_t;

extern const objtype_comment_t comments[];
WORD findComment(LPCTSTR matchString);

// Compatibility for VS2003
#if (_MSC_VER < 1400)
#	define _tcscpy_s(x, y, z) _tcscpy(x, z)
#endif
