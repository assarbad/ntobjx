///////////////////////////////////////////////////////////////////////////////
///
/// Glue code to support legacy Windows versions (e.g. XP) from VS2019.
///
///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2021 Oliver Schneider (assarbad.net)
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

#define EncodePointer EncodePointer_hidden
#define DecodePointer DecodePointer_hidden
#define ActivateActCtx ActivateActCtx_hidden
#define DeactivateActCtx DeactivateActCtx_hidden
#include <Windows.h>
#undef EncodePointer
#undef DecodePointer
#undef ActivateActCtx
#undef DeactivateActCtx

extern "C" {
    DWORD wWinMainCRTStartup(LPVOID);

    static PVOID WINAPI EncodePointer_mock_(__in_opt PVOID Ptr)
    {
        return Ptr;
    }

    static PVOID WINAPI DecodePointer_mock_(__in_opt PVOID Ptr)
    {
        return Ptr;
    }

    static BOOL WINAPI ActivateActCtx_mock_(__inout_opt HANDLE hActCtx, __out   ULONG_PTR* lpCookie)
    {
        UNREFERENCED_PARAMETER(hActCtx);
        UNREFERENCED_PARAMETER(lpCookie);
        return TRUE;
    }

    static BOOL WINAPI DeactivateActCtx_mock_(__in DWORD dwFlags, __in ULONG_PTR ulCookie)
    {
        UNREFERENCED_PARAMETER(dwFlags);
        UNREFERENCED_PARAMETER(ulCookie);
        return TRUE;
    }


    // This works on 64-bit, 32-bit may need some more work
    PVOID(WINAPI* __imp_EncodePointer)(PVOID) = EncodePointer_mock_;
    PVOID(WINAPI* __imp_DecodePointer)(PVOID) = DecodePointer_mock_;
    BOOL(WINAPI* __imp_ActivateActCtx)(HANDLE, ULONG_PTR*) = ActivateActCtx_mock_;
    BOOL(WINAPI* __imp_DeactivateActCtx)(DWORD, ULONG_PTR) = DeactivateActCtx_mock_;

    // This should ensure that we get to run prior to any C runtime code
    // (possible exception would be TLS callbacks called by ntdll.dll)
    DWORD wWinMainCRTStartup_surrogate(LPVOID arg)
    {
        // https://www.bigmessowires.com/2015/10/02/what-happens-before-main/
        return wWinMainCRTStartup(arg);
    }

} // extern "C"

#pragma comment(linker, "/entry:wWinMainCRTStartup_surrogate")
