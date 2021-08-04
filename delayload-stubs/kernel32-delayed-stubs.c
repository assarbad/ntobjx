#define EncodePointer EncodePointer_hidden
#define DecodePointer DecodePointer_hidden
#define ActivateActCtx ActivateActCtx_hidden
#define DeactivateActCtx DeactivateActCtx_hidden
#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
#   define WINVER 0x0500
#endif
#include <Windows.h>
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

// GetModuleHandleExW InitializeSListHead InterlockedPopEntrySList InterlockedPushEntrySList
