///////////////////////////////////////////////////////////////////////////////
///
/// Main entry point to ntobjx.
///
/// Dual-licensed under MS-PL and MIT license (see below).
///
///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2016 Oliver Schneider (assarbad.net)
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

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlsplit.h>

#include "resource.h"

#include "objmgr.hpp"
#include "objtypes.h"
using NtObjMgr::Directory;
using NtObjMgr::GenericObject;
using NtObjMgr::SymbolicLink;

#include "ntobjx.h"
#include <stdarg.h>

CNtObjectsAppModule _Module;

#ifdef DDKBUILD
#   if (_ATL_VER < 0x0700)
#       include <atlimpl.cpp>
#   endif //(_ATL_VER < 0x0700)
#endif // DDKBUILD

#define PATTERN_ENTRY(match, comment) { _T(match), _T(comment), 0x2 }
#define EXACT_ENTRY(match, comment)   { _T(match), _T(comment), 0x0 }
#define EXACTI_ENTRY(match, comment)   { _T(match), _T(comment), 0x1 }

const objtype_comment_t comments[] = {
    EXACTI_ENTRY("\\ArcName", "Symlinks mapping ARC-style (Advanced RISC Computing) names to NT-style names; used during early boot stages"),
    EXACTI_ENTRY("\\BaseNamedObjects", "This is where global ALPC port, event, job, mutex (mutant), semaphore, symbolic link, section (memory mapped file) and waitable timer objects live"),
    EXACTI_ENTRY("\\Callback", "Callback objects"),
    EXACTI_ENTRY("\\Device", "Where drivers conventionally create and look for device objects"),
    EXACTI_ENTRY("\\DosDevices", "Legacy name and alias for the local \\??\nAlso see: \\GLOBAL??"),
    EXACTI_ENTRY("\\Driver", "Contains instances of driver objects"),
    EXACTI_ENTRY("\\FileSystem", "Available FS, FS filter and FS recognizer device and driver objects"),
    EXACTI_ENTRY("\\GLOBAL??", "Global (across all sessions) 'DOS' device names on a machine with terminal services enabled"),
    EXACTI_ENTRY("\\KernelObjects", "Contains event objects that signal low resource conditions, memory errors, the completion of certain operating system tasks, as well as objects representing sessions\nSource: Windows Internals, 6th ed."),
    EXACTI_ENTRY("\\KnownDlls", "Section objects of preloaded known DLLs and Win32-path to known DLLs\nAlso see: HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs\\DllDirectory"),
    EXACTI_ENTRY("\\KnownDlls32", "Section objects of preloaded known 32-bit DLLs on 64-bit systems\nAlso see: HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs\\DllDirectory32"),
    EXACTI_ENTRY("\\NLS", "Section names for NLS tables"),
    EXACTI_ENTRY("\\ObjectTypes", "Catalog of available object types\nHint: names correspond to Type column"),
    EXACTI_ENTRY("\\PSXSS", "Objects relating to the POSIX subsystem (Subsystem for UNIX Applications)"),
    EXACTI_ENTRY("\\RPC Control", "ALPC ports used by remote procedure calls (RPCs), and events used by Conhost .exe as part of the console isolation mechanism\nSource: Windows Internals, 6th ed."),
    EXACTI_ENTRY("\\Security", "ALPC ports and events used by names of objects specific to the security subsystem\nSource: Windows Internals, 6th ed."),
    EXACTI_ENTRY("\\Sessions", "Contains session-local objects, e.g. network-drive mappings\nCombines several object directories to provide a per-session namespace\nSource: 'Session Namespace', chapter 3, Windows Internals, 6th ed."),
    EXACTI_ENTRY("\\UMDFCommunicationPorts", "ALPC ports used by the User-Mode Driver Framework (UMDF)\nSource: Windows Internals, 6th ed."),
    EXACTI_ENTRY("\\Windows", "Contains window station objects"),
    EXACTI_ENTRY("\\Windows\\WindowStations", "Catalog of window stations\nA WindowStation is the container for a Desktop, which in turn hosts the visible windows"),
    // Non-Directory objects
    EXACTI_ENTRY("\\??", "Contains session-local 'DOS' device names, such as drive letters ('C:', ...) and 'NUL'; also see \\GLOBAL??"),
    EXACTI_ENTRY("\\REGISTRY", "The root of the registry as seen outside the Win32 subsystem"),
    EXACTI_ENTRY("\\SystemRoot", "The device path to your %SystemRoot%; e.g. the true path of C:\\Windows"),
    EXACTI_ENTRY("\\Device\\Mailslot", "The device that offers mailslots in their own 'DOS' namespace"),
    EXACTI_ENTRY("\\Device\\NamedPipe", "The device that offers pipes in their own 'DOS' namespace"),
    EXACTI_ENTRY("\\Device\\PhysicalMemory", "Section allows reresenting 'physical' memory"),
    //PATTERN_ENTRY("\\Device\\HarddiskVolume*", "Device object that represents a volume, i.e. partition, on a disk"),
};

#undef PATTERN_ENTRY
#undef EXACT_ENTRY

LPCTSTR findComment(LPCTSTR matchString)
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
                return comments[i].comment;
        }
        else
        {
            ATLASSERT(!"NOT IMPLEMENTED");
        }
    }
    return NULL;
}


int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CNtObjectsMainFrame wndMain;

    if(wndMain.CreateEx() == NULL)
    {
        ATLTRACE(_T("Main window creation failed!\n"));
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

void * __cdecl operator new(size_t bytes)
{
    return malloc(bytes);
}

void __cdecl operator delete(void *ptr)
{
    free(ptr);
}

extern "C" int __cdecl __purecall(void)
{
    return 0;
}
#endif // DDKBUILD

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
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
