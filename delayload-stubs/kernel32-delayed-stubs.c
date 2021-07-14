#define EncodePointer EncodePointer_mock
#define DecodePointer DecodePointer_mock
#define ActivateActCtx ActivateActCtx_mock
#define DeactivateActCtx DeactivateActCtx_mock
#pragma warning(disable: 4005)
#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
#   define WINVER 0x0500
#endif
#if (_MSCVER >= 1400)
#   define _NTDLLBUILD_
#   define _NTSYSTEM_
#endif
#include <Windows.h>
#pragma warning(default: 4005)
#undef EncodePointer
#undef DecodePointer
#undef ActivateActCtx
#undef DeactivateActCtx

PVOID
WINAPI
EncodePointer (
    __in_opt PVOID Ptr
    ) { return NULL; }

PVOID
WINAPI
DecodePointer (
    __in_opt PVOID Ptr
    ) { return NULL; }

BOOL
WINAPI
ActivateActCtx(
    __inout_opt HANDLE hActCtx,
    __out   ULONG_PTR * lpCookie
    ) { return FALSE; }

BOOL
WINAPI
DeactivateActCtx(
    __in DWORD dwFlags,
    __in ULONG_PTR ulCookie
    ) { return FALSE; }
