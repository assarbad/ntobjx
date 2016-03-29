#include "stdafx.h" /* Must alias DelayLoadError to a function that behaves like _tprintf */
#include <delayimp.h>

#pragma comment(lib, "delayimp")

namespace
{
    LPCSTR lpszNtDllName = "ntdll.dll";
    LPCSTR lpszNtDelayedDllName = "ntdll-delayed.dll";
}

static LONG WINAPI DelayLoadFilter(PEXCEPTION_POINTERS pExcPointers)
{
    LONG lDisposition = EXCEPTION_EXECUTE_HANDLER;
    PDelayLoadInfo pdli = (PDelayLoadInfo)(pExcPointers->ExceptionRecord->ExceptionInformation[0]);

    switch(pExcPointers->ExceptionRecord->ExceptionCode)
    {
    case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
        DelayLoadError(_T("The DLL %hs could not be loaded."), pdli->szDll);
        break;
    case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
        {
            LPCSTR lpszDllName = pdli->szDll;
            if(0 == lstrcmpiA(pdli->szDll, lpszNtDelayedDllName))
            {
                lpszDllName = lpszNtDllName;
            }
            if (pdli->dlp.fImportByName)
            {
                DelayLoadError(_T("Function %hs::%hs not found."), pdli->szDll, pdli->dlp.szProcName);
            }
            else
            {
                DelayLoadError(_T("Function %hs::#%u not found."), pdli->szDll, pdli->dlp.dwOrdinal);
            }
        }
        break;
    default:
        lDisposition = EXCEPTION_CONTINUE_SEARCH;
        break;
    }
    return lDisposition;
}

EXTERN_C void force_resolve_ntdll(void)
{
    __try
    {
        /* Force all delay-loaded symbols to be resolved at once */
        (void)__HrLoadAllImportsForDll(lpszNtDelayedDllName);
    }
    __except(DelayLoadFilter(GetExceptionInformation()))
    {
        ExitProcess(1);
    }

}

static FARPROC WINAPI NtdllDliHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
    switch(dliNotify)
    {
    case dliNotePreLoadLibrary:
        if(0 == lstrcmpiA(pdli->szDll, lpszNtDelayedDllName))
        {
            if(HMODULE hNtDll = ::GetModuleHandleA(lpszNtDllName))
            {
                return reinterpret_cast<FARPROC>(hNtDll);
            }
        }
        break; /* proceed with default processing  */
    default:
        break;
    }
    return NULL;
}

PfnDliHook __pfnDliNotifyHook2 = NtdllDliHook;
