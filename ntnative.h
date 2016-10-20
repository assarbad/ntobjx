///////////////////////////////////////////////////////////////////////////////
///
/// Type and function declarations for the NT native API.
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

#ifndef __NTNATIVE_H_VER__
#define __NTNATIVE_H_VER__ 2016102020
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif // Check for "#pragma once" support

// Fake the SAL annotations where they don't exist.
// Now I am running VS2005 with Windows 7 SP1 SDK integrated. Possible that's why it works there.
#if (_MSC_VER < 1400)
#   define __success(x)
#   define __field_range(x, y)
#   define __kernel_entry
#   define __in
#   define __in_bcount(x)
#   define __in_opt
#   define __inout
#   define __inout_opt
#   define __out
#   define __out_bcount(x)
#   define __out_opt
#   define __out_bcount_opt(x)
#   define __reserved
#endif

#if defined(DDKBUILD)
#   include <WinDef.h>
#else
#   pragma push_macro("NTSYSCALLAPI")
#   ifdef NTSYSCALLAPI
#       undef NTSYSCALLAPI
#       define NTSYSCALLAPI
#   endif
#   include <winternl.h>
#   pragma pop_macro("NTSYSCALLAPI")
#endif // DDKBUILD
#pragma warning(disable:4005)
#include <ntstatus.h>
#pragma warning(default:4005)

#if defined(DDKBUILD)
#   if defined(__cplusplus)
    extern "C" {
#   endif
    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING;
    typedef UNICODE_STRING *PUNICODE_STRING;
    typedef const UNICODE_STRING *PCUNICODE_STRING;

    typedef struct _OBJECT_ATTRIBUTES {
        ULONG Length;
        HANDLE RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG Attributes;
        PVOID SecurityDescriptor;
        PVOID SecurityQualityOfService;
    } OBJECT_ATTRIBUTES;
    typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

#   ifndef _NTDEF_
    typedef __success(return >= 0) LONG NTSTATUS;
    typedef NTSTATUS *PNTSTATUS;
#   endif

    typedef struct _IO_STATUS_BLOCK {
        union {
            NTSTATUS Status;
            PVOID Pointer;
        } DUMMYUNIONNAME;

        ULONG_PTR Information;
    } IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#   define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#   define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#   define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

    __kernel_entry NTSTATUS
        NTAPI 
        NtClose (
        IN HANDLE Handle
        );

    VOID
        NTAPI 
        RtlInitUnicodeString (
        PUNICODE_STRING DestinationString,
        PCWSTR SourceString
        );

#   if defined(__cplusplus)
    }
#   endif
#endif // DDKBUILD

#ifndef InitializeObjectAttributes
#   define InitializeObjectAttributes( p, n, a, r, s ) { \
        (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
        (p)->RootDirectory = r;                             \
        (p)->Attributes = a;                                \
        (p)->ObjectName = n;                                \
        (p)->SecurityDescriptor = s;                        \
        (p)->SecurityQualityOfService = NULL;               \
        }
#endif

#if (_MSC_VER < 1400)
typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation = 0,
    ObjectTypeInformation = 2
} OBJECT_INFORMATION_CLASS;
#endif

#ifndef RTL_CONSTANT_STRING
#if defined(__cplusplus)
extern "C++"
{
    char _RTL_CONSTANT_STRING_type_check(const char *s);
    char _RTL_CONSTANT_STRING_type_check(const WCHAR *s);
    // __typeof would be desirable here instead of sizeof.
    template <size_t N> class _RTL_CONSTANT_STRING_remove_const_template_class;
    template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(char)>  {public: typedef  char T; };
    template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(WCHAR)> {public: typedef WCHAR T; };
#   define _RTL_CONSTANT_STRING_remove_const_macro(s) \
        (const_cast<_RTL_CONSTANT_STRING_remove_const_template_class<sizeof((s)[0])>::T*>(s))
}
#else
    char _RTL_CONSTANT_STRING_type_check(const void *s);
#   define _RTL_CONSTANT_STRING_remove_const_macro(s) (s)
#endif // __cplusplus
#define RTL_CONSTANT_STRING(s) \
{ \
    sizeof( s ) - sizeof( (s)[0] ), \
    sizeof( s ) / sizeof(_RTL_CONSTANT_STRING_type_check(s)), \
    _RTL_CONSTANT_STRING_remove_const_macro(s) \
}
#endif // RTL_CONSTANT_STRING

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef OBJECT_TYPE_CREATE
#   define OBJECT_TYPE_CREATE (0x0001)
#endif // OBJECT_TYPE_CREATE

#ifndef OBJECT_TYPE_ALL_ACCESS
#   define OBJECT_TYPE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)
#endif // OBJECT_TYPE_ALL_ACCESS

#ifndef DIRECTORY_QUERY
#   define DIRECTORY_QUERY                 (0x0001)
#endif // DIRECTORY_QUERY
#ifndef DIRECTORY_TRAVERSE
#   define DIRECTORY_TRAVERSE              (0x0002)
#endif // DIRECTORY_TRAVERSE
#ifndef DIRECTORY_CREATE_OBJECT
#   define DIRECTORY_CREATE_OBJECT         (0x0004)
#endif // DIRECTORY_CREATE_OBJECT
#ifndef DIRECTORY_CREATE_SUBDIRECTORY
#   define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)
#endif // DIRECTORY_CREATE_SUBDIRECTORY

#ifndef DIRECTORY_ALL_ACCESS
#   define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)
#endif // DIRECTORY_ALL_ACCESS

#ifndef SYMBOLIC_LINK_QUERY
#   define SYMBOLIC_LINK_QUERY (0x0001)
#endif // SYMBOLIC_LINK_QUERY

#ifndef SYMBOLIC_LINK_ALL_ACCESS
#   define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)
#endif // SYMBOLIC_LINK_ALL_ACCESS

#ifndef _NTDEF_
    typedef __success(return >= 0) LONG NTSTATUS;
    typedef NTSTATUS *PNTSTATUS;
#   define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#   define NT_INFORMATION(Status) ((((ULONG)(Status)) >> 30) == 1)
#   define NT_WARNING(Status) ((((ULONG)(Status)) >> 30) == 2)
#   define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)
#endif

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

//
// Section Access Rights.
//


#define SECTION_QUERY                0x0001
#define SECTION_MAP_WRITE            0x0002
#define SECTION_MAP_READ             0x0004
#define SECTION_MAP_EXECUTE          0x0008
#define SECTION_EXTEND_SIZE          0x0010
#define SECTION_MAP_EXECUTE_EXPLICIT 0x0020 // not included in SECTION_ALL_ACCESS

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)

typedef struct _OBJECT_BASIC_INFORMATION {
    ULONG Attributes;
    ACCESS_MASK DesiredAccess;
    ULONG HandleCount;
    ULONG ReferenceCount;
    ULONG PagedPoolUsage;
    ULONG NonPagedPoolUsage;
    ULONG Reserved[3];
    ULONG NameInformationLength;
    ULONG TypeInformationLength;
    ULONG SecurityDescriptorLength;
    LARGE_INTEGER CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

typedef enum _ATOM_INFORMATION_CLASS
{
    AtomBasicInformation,
    AtomTableInformation,
} ATOM_INFORMATION_CLASS;

typedef enum _EVENT_INFORMATION_CLASS
{
    EventBasicInformation
} EVENT_INFORMATION_CLASS;

typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    KeyVirtualizationInformation,
    KeyHandleTagsInformation,
    MaxKeyInfoClass  // MaxKeyInfoClass should always be the last enum
} KEY_INFORMATION_CLASS;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;

typedef enum _MUTANT_INFORMATION_CLASS
{
    MutantBasicInformation,
    MutantOwnerInformation
} MUTANT_INFORMATION_CLASS;

typedef enum _SECTION_INFORMATION_CLASS {
    SectionBasicInformation,
    SectionImageInformation
} SECTION_INFORMATION_CLASS;

typedef enum _SEMAPHORE_INFORMATION_CLASS
{
    SemaphoreBasicInformation
} SEMAPHORE_INFORMATION_CLASS;

typedef enum _TIMER_INFORMATION_CLASS
{
    TimerBasicInformation
} TIMER_INFORMATION_CLASS;

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
    //          Data[1];            // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 {
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

#if (_MSC_VER  < 1500)
typedef struct _KEY_VALUE_ENTRY {
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;
#endif

typedef struct _MUTANT_BASIC_INFORMATION {
    LONG CurrentCount;
    BOOLEAN OwnedByCaller;
    BOOLEAN AbandonedState;
} MUTANT_BASIC_INFORMATION, *PMUTANT_BASIC_INFORMATION;

typedef struct _SECTIONBASICINFO {
    PVOID BaseAddress;
    ULONG AllocationAttributes;
    LARGE_INTEGER MaximumSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef struct _SECTION_IMAGE_INFORMATION {
    PVOID TransferAddress;
    ULONG ZeroBits;
    ULONG MaximumStackSize;
    ULONG CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        } minmaj;
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN ImageFlags;
    ULONG ComPlusNativeReady: 1;
    ULONG ComPlusILOnly: 1;
    ULONG ImageDynamicallyRelocated: 1;
    ULONG Reserved: 5;
    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG CheckSum;
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

//
//  The context structure is used when generating 8.3 names.  The caller must
//  always zero out the structure before starting a new generation sequence
//

typedef struct _GENERATE_NAME_CONTEXT {

    //
    //  The structure is divided into two strings.  The Name, and extension.
    //  Each part contains the value that was last inserted in the name.
    //  The length values are in terms of wchars and not bytes.  We also
    //  store the last index value used in the generation collision algorithm.
    //

    USHORT Checksum;
    BOOLEAN ChecksumInserted;

    __field_range(<=, 8) UCHAR NameLength;        // not including extension
    WCHAR NameBuffer[8];                          // e.g., "ntoskrnl"

    __field_range(<=, 4) ULONG ExtensionLength;   // including dot
    WCHAR ExtensionBuffer[4];                     // e.g., ".exe"

    ULONG LastIndexValue;

} GENERATE_NAME_CONTEXT;
typedef GENERATE_NAME_CONTEXT *PGENERATE_NAME_CONTEXT;

NTSTATUS
NTAPI
RtlGetVersion(
    PRTL_OSVERSIONINFOW lpVersionInformation
    );

NTSTATUS
NTAPI
NtQueryEvent(
    __in HANDLE EventHandle,
    __in EVENT_INFORMATION_CLASS EventInformationClass,
    __out_bcount(EventInformationLength) PVOID EventInformation,
    __in ULONG EventInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSTATUS
NTAPI
NtQueryMutant(
    __in HANDLE MutantHandle,
    __in MUTANT_INFORMATION_CLASS MutantInformationClass,
    __out_bcount(MutantInformationLength) PVOID MutantInformation,
    __in ULONG MutantInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSTATUS
NTAPI
NtQuerySemaphore(
    __in HANDLE SemaphoreHandle,
    __in SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    __out_bcount(SemaphoreInformationLength) PVOID SemaphoreInformation,
    __in ULONG SemaphoreInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSTATUS
NTAPI
NtQuerySection(
    IN HANDLE SectionHandle,
    IN SECTION_INFORMATION_CLASS SectionInformationClass,
    OUT PVOID SectionInformation,
    IN ULONG SectionInformationLength,
    OUT PULONG ReturnLength OPTIONAL
     );

NTSTATUS
NTAPI
NtOpenDirectoryObject(
    __out PHANDLE DirectoryHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtQueryDirectoryObject(
    __in HANDLE DirectoryHandle,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in BOOLEAN ReturnSingleEntry,
    __in BOOLEAN RestartScan,
    __inout PULONG Context,
    __out_opt PULONG ReturnLength
    );

NTSTATUS
NTAPI
NtOpenSymbolicLinkObject(
    __out PHANDLE LinkHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtQuerySymbolicLinkObject(
    __in HANDLE LinkHandle,
    __inout PUNICODE_STRING LinkTarget,
    __out_opt PULONG ReturnedLength
    );

NTSTATUS
NTAPI
NtOpenEvent(
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtOpenMutant(
    __out PHANDLE MutantHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtOpenSection(
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtOpenTimer(
    __out PHANDLE TimerHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS
NTAPI
NtOpenSemaphore(
    __out PHANDLE SemaphoreHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtOpenEventPair(
    __out PHANDLE EventPairHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtOpenIoCompletion(
    __out PHANDLE IoCompletionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtOpenKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
NTAPI
NtCreateKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __reserved ULONG TitleIndex,
    __in_opt PUNICODE_STRING Class,
    __in ULONG CreateOptions,
    __out_opt PULONG Disposition
    );

NTSTATUS
NTAPI
NtEnumerateKey(
    __in HANDLE KeyHandle,
    __in ULONG Index,
    __in KEY_INFORMATION_CLASS KeyInformationClass,
    __out_bcount_opt(Length) PVOID KeyInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );

NTSTATUS
NTAPI
NtEnumerateValueKey(
    __in HANDLE KeyHandle,
    __in ULONG Index,
    __in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_bcount_opt(Length) PVOID KeyValueInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );

NTSTATUS
NTAPI
NtQueryKey(
    __in HANDLE KeyHandle,
    __in KEY_INFORMATION_CLASS KeyInformationClass,
    __out_bcount_opt(Length) PVOID KeyInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );

NTSTATUS
NTAPI
NtQueryValueKey(
    __in HANDLE KeyHandle,
    __in PUNICODE_STRING ValueName,
    __in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_bcount_opt(Length) PVOID KeyValueInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );

NTSTATUS
NTAPI
RtlValidateUnicodeString(
    __in __reserved ULONG Flags,
    __in PCUNICODE_STRING String
    );

NTSTATUS
NTAPI
RtlDowncaseUnicodeString(
         PUNICODE_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    );

NTSTATUS /* VOID in pre-Vista*/
NTAPI
RtlGenerate8dot3Name(
    __in    PCUNICODE_STRING Name,
    __in    BOOLEAN AllowExtendedCharacters,
    __inout PGENERATE_NAME_CONTEXT Context,
    __inout PUNICODE_STRING Name8dot3
    );

NTSTATUS
NTAPI
RtlVolumeDeviceToDosName(
    __in PVOID VolumeDeviceObject,
    __out PUNICODE_STRING DosName
    );

NTSTATUS
NTAPI
NtCreateSection(
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt PLARGE_INTEGER MaximumSize,
    __in ULONG SectionPageProtection,
    __in ULONG AllocationAttributes,
    __in_opt HANDLE FileHandle
    );

NTSTATUS
NTAPI
NtMapViewOfSection(
    __in HANDLE SectionHandle,
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __in ULONG_PTR ZeroBits,
    __in SIZE_T CommitSize,
    __inout_opt PLARGE_INTEGER SectionOffset,
    __inout PSIZE_T ViewSize,
    __in SECTION_INHERIT InheritDisposition,
    __in ULONG AllocationType,
    __in ULONG Win32Protect
    );

NTSTATUS
NTAPI
NtUnmapViewOfSection(
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress
    );

#define ZwClose NtClose
#define ZwCreateFile NtCreateFile
#define ZwOpenFile NtOpenFile
#define ZwDeviceIoControlFile NtDeviceIoControlFile
#define ZwWaitForSingleObject NtWaitForSingleObject
#define ZwQueryInformationProcess NtQueryInformationProcess
#define ZwQueryInformationThread NtQueryInformationThread
#define ZwQueryObject NtQueryObject
#define ZwQuerySystemInformation NtQuerySystemInformation
#define ZwQuerySystemTime NtQuerySystemTime
#define ZwQueryEvent NtQueryEvent
#define ZwQueryMutant NtQueryMutant
#define ZwQuerySemaphore NtQuerySemaphore
#define ZwQuerySection NtQuerySection
#define ZwOpenDirectoryObject NtOpenDirectoryObject
#define ZwQueryDirectoryObject NtQueryDirectoryObject
#define ZwOpenSymbolicLinkObject NtOpenSymbolicLinkObject
#define ZwQuerySymbolicLinkObject NtQuerySymbolicLinkObject
#define ZwOpenEvent NtOpenEvent
#define ZwOpenMutant NtOpenMutant
#define ZwOpenSection NtOpenSection
#define ZwOpenSemaphore NtOpenSemaphore
#define ZwOpenTimer NtOpenTimer
#define ZwOpenEventPair NtOpenEventPair
#define ZwOpenIoCompletion NtOpenIoCompletion
#define ZwOpenKey NtOpenKey
#define ZwCreateKey NtCreateKey
#define ZwEnumerateKey NtEnumerateKey
#define ZwEnumerateValueKey NtEnumerateValueKey
#define ZwQueryValueKey NtQueryValueKey
#define ZwCreateSection NtCreateSection
#define ZwMapViewOfSection NtMapViewOfSection
#define ZwUnmapViewOfSection NtUnmapViewOfSection

#if defined(__cplusplus)
}
#endif

#endif // __NTNATIVE_H_VER__
