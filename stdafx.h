// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#ifdef NDEBUG
#   ifdef _WIN64
#       pragma optimize("gs", on)
#   else
#       pragma optimize("gsy", on)
#   endif
#endif

#define _ATL_NO_MSIMG
#define _ATL_NOFORCE_MANIFEST
#define _ATL_NO_OPENGL
#define _ATL_NO_COM_SUPPORT
#define _ATL_NO_PERF_SUPPORT
#ifndef _ATL_STATIC_REGISTRY
#   define _ATL_STATIC_REGISTRY
#endif

#define _WTL_USE_CSTRING

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

typedef CAppModule CNtObjectsAppModule;
extern CNtObjectsAppModule _Module;

#include <atlwin.h>
#include <Shellapi.h>

#if defined _M_IX86
#   pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#   pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#   pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#   pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifndef DDKBUILD
#include <delayimp.h>
#endif // DDKBUILD
#include "ntnative.h"

#ifndef DDKBUILD
#   if defined(_CONSOLE) || defined(_CONSOLE_APP)
#       define DelayLoadError _tprintf
#   else
        EXTERN_C int __cdecl DelayLoadError(LPCTSTR lpszFormat, ...);
#   endif
    EXTERN_C void force_resolve_ntdll(void);
#endif // DDKBUILD

typedef struct _objtype_comment_t
{
    union {
        LPCTSTR pattern;
        LPCTSTR exact;
    } match;
    LPCTSTR comment;
    unsigned int case_insensitive:1;
    unsigned int pattern_match:1;
} objtype_comment_t;

extern const objtype_comment_t comments[];
LPCTSTR findComment(LPCTSTR matchString);
