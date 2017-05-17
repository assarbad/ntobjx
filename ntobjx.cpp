///////////////////////////////////////////////////////////////////////////////
///
/// Main entry point to ntobjx.
///
/// Dual-licensed under MS-PL and MIT license (see below).
///
///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2016, 2017 Oliver Schneider (assarbad.net)
///
/// Permission is hereby granted, free of charge, to any person obtaining a
/// copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation
/// the rights to use, copy, modify, merge, publish, distribute, sublicense,
/// and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
///
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning(push) /* disable code analyzer warnings */
#pragma warning(disable:6387) /* warning C6387 : '...' could be '0' : this does not adhere to the specification for the function '...'. */
#pragma warning(disable:6001) /* warning C6001: Using uninitialized memory '...'. */
#pragma warning(disable:6011) /* warning C6011: Dereferencing NULL pointer '...'.  */
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlctrlw.h>
#include <atlsplit.h>
#include <atlsecurity.h>
#pragma warning(pop) /* restore code analyzer warnings*/
#include <stdarg.h>

#include "resource.h"
#include "reslang.h"
#include "ntnative.h"

#include "objmgr.hpp"
#include "objtypes.h"
using NtObjMgr::Directory;
using NtObjMgr::GenericObject;
using NtObjMgr::SymbolicLink;

#include "ntobjx.h"

CNtObjectsAppModule _Module;

#ifdef DDKBUILD
#   if (_ATL_VER < 0x0700)
#       include <atlimpl.cpp>
#   endif //(_ATL_VER < 0x0700)
#endif // DDKBUILD

#define EXACTI_ENTRY(match, comment)   { _T(match), comment, 0x1 }

/*lint -save -e651 */
const objtype_comment_t comments[] = {
    EXACTI_ENTRY("\\ArcName", IDS_OBJTT_ARCNAME),
    EXACTI_ENTRY("\\BaseNamedObjects", IDS_OBJTT_BASENAMEDOBJECTS),
    EXACTI_ENTRY("\\Callback", IDS_OBJTT_CALLBACK),
    EXACTI_ENTRY("\\Device", IDS_OBJTT_DEVICE),
    EXACTI_ENTRY("\\DosDevices", IDS_OBJTT_DOSDEVICES),
    EXACTI_ENTRY("\\Driver", IDS_OBJTT_DRIVER),
    EXACTI_ENTRY("\\FileSystem", IDS_OBJTT_FILESYSTEM),
    EXACTI_ENTRY("\\GLOBAL\?\?", IDS_OBJTT_GLOBALDOSDEV),
    EXACTI_ENTRY("\\KernelObjects", IDS_OBJTT_KERNELOBJECTS),
    EXACTI_ENTRY("\\KnownDlls", IDS_OBJTT_KNOWNDLLS),
    EXACTI_ENTRY("\\KnownDlls32", IDS_OBJTT_KNOWNDLLS32),
    EXACTI_ENTRY("\\NLS", IDS_OBJTT_NLS),
    EXACTI_ENTRY("\\ObjectTypes", IDS_OBJTT_OBJECTTYPES),
    EXACTI_ENTRY("\\PSXSS", IDS_OBJTT_PSXSS),
    EXACTI_ENTRY("\\RPC Control", IDS_OBJTT_RPCCTL),
    EXACTI_ENTRY("\\Security", IDS_OBJTT_SECURITY),
    EXACTI_ENTRY("\\Sessions", IDS_OBJTT_SESSIONS),
    EXACTI_ENTRY("\\UMDFCommunicationPorts", IDS_OBJTT_UDFCOMMPORTS),
    EXACTI_ENTRY("\\Windows", IDS_OBJTT_WINDOWS),
    EXACTI_ENTRY("\\Windows\\WindowStations", IDS_OBJTT_WINDOWSTATIONS),
    // Non-Directory objects
    EXACTI_ENTRY("\\\?\?", IDS_OBJTT_DOSDEV),
    EXACTI_ENTRY("\\REGISTRY", IDS_OBJTT_REGISTRY),
    EXACTI_ENTRY("\\SystemRoot", IDS_OBJTT_SYSTEMROOT),
    EXACTI_ENTRY("\\Device\\Mailslot", IDS_OBJTT_MAILSLOT),
    EXACTI_ENTRY("\\Device\\NamedPipe", IDS_OBJTT_NAMEDPIPE),
    EXACTI_ENTRY("\\Device\\PhysicalMemory", IDS_OBJTT_PHYSMEM),
};
/*lint -restore */

#undef EXACTI_ENTRY

WORD findComment(LPCTSTR matchString)
{
    for(size_t i = 0; i < sizeof(comments)/sizeof(comments[0]); i++)
    {
        if(!comments[i].pattern_match)
        {
            bool found;

            if(comments[i].case_insensitive)
                found = (0 == _tcsicmp(comments[i].match.exact, matchString));
            else
                found = (0 == _tcscmp(comments[i].match.exact, matchString));

            if(found)
                return comments[i].resId;
        }
        else
        {
            ATLASSERT(!"NOT IMPLEMENTED");
        }
    }
    return NULL;
}

OSVERSIONINFOEXW const& GetOSVersionInfo()
{
    static OSVERSIONINFOEXW osvix = { sizeof(OSVERSIONINFOEXW), 0, 0, 0, 0,{ 0 } }; // not an error, this has to be the W variety!
    if (osvix.dwMajorVersion == 0)
    {
        ATLVERIFY(NT_SUCCESS(RtlGetVersion(&osvix)));
    }
    return osvix;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
    OSVERSIONINFOEXW const& osvix = GetOSVersionInfo();
    // Set the default language
    (void)gLangSetter.set();

    if (osvix.dwMajorVersion < 6) /* language handling for resources works starting with Vista (SetThreadUILanguage), but not before that */
    {
        if(!HookLdrFindResource_U(_Module.GetModuleInstance()))
        {
            ::Beep(500, 500);
            ATLTRACE2(_T("Failed to hook LdrFindResource_U, so switching languages is unlikely to work as expected.\n"));
        }
    }

    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CNtObjectsMainFrame wndMain(osvix);

    if(wndMain.CreateEx() == NULL)
    {
        ATLTRACE2(_T("Main window creation failed!\n"));
        return 0;
    }

    wndMain.ShowWindow(nCmdShow);

    int nRet = theLoop.Run();

    _Module.RemoveMessageLoop();
    return nRet;
}

#ifndef DDKBUILD
EXTERN_C int __cdecl DelayLoadError(LPCTSTR lpszFormat, ...)
{
    ATL::CString msg;
    va_list args;
    va_start(args, lpszFormat);
    msg.FormatV(lpszFormat, args);
    AtlMessageBox(NULL, msg.GetBuffer(), _T("Delay Load Error"), MB_OK | MB_ICONERROR);
    return msg.GetLength();
};

#if (_MSC_VER < 1910)
void * __cdecl operator new(size_t bytes)
{
    return malloc(bytes);
}

void __cdecl operator delete(void *ptr)
{
    free(ptr);
}
#endif // (_MSC_VER < 1910)

extern "C" int __cdecl __purecall(void)
{
    return 0;
}
#endif // DDKBUILD

#pragma warning(suppress: 28251)
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
#if 0 && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif // _DEBUG

    HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ATLASSERT(SUCCEEDED(hRes));

    AtlInitCommonControls(ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES);

#ifndef DDKBUILD
    force_resolve_ntdll();
#endif // DDKBUILD
    hRes = _Module.Init(NULL, hInstance);
    ATLASSERT(SUCCEEDED(hRes));

    int nRet = Run(lpstrCmdLine, nCmdShow);

    _Module.Term();
    ::CoUninitialize();

    return nRet;
}
