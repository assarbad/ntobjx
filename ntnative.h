///////////////////////////////////////////////////////////////////////////////
///
/// Type and function declarations for the NT native API.
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

#ifndef __NTNATIVE_H_VER__
#define __NTNATIVE_H_VER__ 2017091820
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

    NTSTATUS
    NTAPI
    NtClose(
        __in HANDLE Handle
    );

    VOID
    NTAPI
    RtlInitUnicodeString(
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

#if (_MSC_VER < 1400) || defined(DDKBUILD)
typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectAllInformation,
    ObjectDataInformation
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

typedef enum _IO_COMPLETION_INFORMATION_CLASS {
    IoCompletionBasicInformation
} IO_COMPLETION_INFORMATION_CLASS, *PIO_COMPLETION_INFORMATION_CLASS;

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

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE, *PEVENT_TYPE;

typedef struct _EVENT_BASIC_INFORMATION {
    EVENT_TYPE EventType;
    LONG EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

typedef struct _IO_COMPLETION_BASIC_INFORMATION {
    ULONG Depth;
} IO_COMPLETION_BASIC_INFORMATION, *PIO_COMPLETION_BASIC_INFORMATION;

typedef struct _KEY_BASIC_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG TitleIndex;
    ULONG NameLength;
    WCHAR Name[1]; // Variable length string
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

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

typedef struct _SEMAPHORE_BASIC_INFORMATION {
    ULONG CurrentCount;
    ULONG MaximumCount;
} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

typedef struct _TIMER_BASIC_INFORMATION {
    LARGE_INTEGER RemainingTime;
    BOOLEAN TimerState;
} TIMER_BASIC_INFORMATION, *PTIMER_BASIC_INFORMATION;

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

#ifndef PIO_APC_ROUTINE_DEFINED
#pragma warning(push) /* disable code analyzer warnings for ATL & WTL libraries */
#pragma warning(disable:28301) /* warning C28301 : No annotations for first declaration of ... */
typedef VOID (NTAPI *PIO_APC_ROUTINE) (__in PVOID ApcContext, __in PIO_STATUS_BLOCK IoStatusBlock, __in ULONG Reserved);
#define PIO_APC_ROUTINE_DEFINED
#pragma warning(pop) /* restore code analyzer warnings*/
#endif // PIO_APC_ROUTINE_DEFINED

typedef enum _NT_FILE_INFORMATION_CLASS
{
    FileInformationDirectory = 1,
    FileInformationFullDirectory, //  2
    FileInformationBothDirectory, //  3
    FileInformationBasic, //  4
    FileInformationStandard, //  5
    FileInformationInternal, //  6
    FileInformationEa, //  7
    FileInformationAccess, //  8
    FileInformationName, //  9
    FileInformationRename, //  10
    FileInformationLink, //  11
    FileInformationNames, //  12
    FileInformationDisposition, //  13
    FileInformationPosition, //  14
    FileInformationFullEa, //  15
    FileInformationMode, //  16
    FileInformationAlignment, //  17
    FileInformationAll, //  18
    FileInformationAllocation, //  19
    FileInformationEndOfFile, //  20
    FileInformationAlternateName, //  21
    FileInformationStream, //  22
    FileInformationPipe, //  23
    FileInformationPipeLocal, //  24
    FileInformationPipeRemote, //  25
    FileInformationMailslotQuery, //  26
    FileInformationMailslotSet, //  27
    FileInformationCompression, //  28
    FileInformationObjectId, //  29
    FileInformationCompletion, //  30
    FileInformationMoveCluster, //  31
    FileInformationQuota, //  32
    FileInformationReparsePoint, //  33
    FileInformationNetworkOpen, //  34
    FileInformationAttributeTag, //  35
    FileInformationTracking, //  36
    FileInformationIdBothDirectory, //  37
    FileInformationIdFullDirectory, //  38
    FileInformationValidDataLength, //  39
    FileInformationShortName, //  40
    FileInformationIoCompletionNotification, //  41
    FileInformationIoStatusBlockRange, //  42
    FileInformationIoPriorityHint, //  43
    FileInformationSfioReserve, //  44
    FileInformationSfioVolume, //  45
    FileInformationHardLink, //  46
    FileInformationProcessIdsUsingFile, //  47
    FileInformationNormalizedName, //  48
    FileInformationNetworkPhysicalName, //  49
    FileInformationIdGlobalTxDirectory, //  50
    FileInformationIsRemoteDevice, //  51
    FileInformationAttributeCache, //  52
    FileInformationMaximum,
} NT_FILE_INFORMATION_CLASS, *PNT_FILE_INFORMATION_CLASS;

#pragma pack(push, 4)
typedef struct _FILE_STREAM_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG StreamNameLength;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamAllocationSize;
    WCHAR StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;
#pragma pack(pop)

#ifndef DYNAMIC_NTNATIVE
NTSTATUS
NTAPI
RtlGetVersion(
    LPOSVERSIONINFOEXW lpVersionInformation
);

PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(
    __in PVOID Base
);

PVOID
NTAPI
RtlImageDirectoryEntryToData(
    __in PVOID Base,
    __in BOOLEAN MappedAsImage,
    __in USHORT DirectoryEntry,
    __out PULONG Size
);

PVOID
NTAPI
RtlImageRvaToVa(
    __in PIMAGE_NT_HEADERS NtHeaders,
    __in PVOID Base,
    __in ULONG Rva,
    __inout_opt PIMAGE_SECTION_HEADER *LastRvaSection
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
NtQueryIoCompletion(
    __in HANDLE IoCompletionHandle,
    __in IO_COMPLETION_INFORMATION_CLASS InformationClass,
    __out PVOID IoCompletionInformation,
    __in ULONG InformationBufferLength,
    __out_opt PULONG RequiredLength
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
    __in HANDLE SectionHandle,
    __in SECTION_INFORMATION_CLASS SectionInformationClass,
    __out PVOID SectionInformation,
    __in ULONG SectionInformationLength,
    __out_opt PULONG ReturnLength
 );

NTSTATUS
NTAPI
NtQueryTimer(
    __in HANDLE TimerHandle,
    __in TIMER_INFORMATION_CLASS TimerInformationClass,
    __out PVOID TimerInformation,
    __in ULONG TimerInformationLength,
    __out_opt PULONG ReturnLength
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
    __in PCUNICODE_STRING Name,
    __in BOOLEAN AllowExtendedCharacters,
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

NTSTATUS
NTAPI
NtQueryDirectoryFile(
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID FileInformation,
    __in ULONG Length,
    __in NT_FILE_INFORMATION_CLASS FileInformationClass,
    __in BOOLEAN ReturnSingleEntry,
    __in_opt PUNICODE_STRING FileName,
    __in BOOLEAN RestartScan
);

NTSTATUS
NTAPI
NtQueryInformationFile(
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID FileInformation,
    __in ULONG Length,
    __in NT_FILE_INFORMATION_CLASS FileInformationClass
);

#if defined(DDKBUILD)
NTSTATUS
NTAPI
NtQueryObject(
   __in_opt HANDLE Handle,
   __in OBJECT_INFORMATION_CLASS ObjectInformationClass,
   __out_bcount_opt(ObjectInformationLength) PVOID ObjectInformation,
   __in ULONG ObjectInformationLength,
   __out_opt PULONG ReturnLength
   );

NTSTATUS
NTAPI
NtOpenFile(
    __out PHANDLE FileHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in ULONG ShareAccess,
    __in ULONG OpenOptions
);
#endif // DDKBUILD
#else
typedef NTSTATUS (NTAPI *RtlGetVersion_t)(LPOSVERSIONINFOEXW);
typedef PIMAGE_NT_HEADERS (NTAPI *RtlImageNtHeader_t)(__in PVOID);
typedef PVOID (NTAPI *RtlImageDirectoryEntryToData_t)(__in PVOID, __in BOOLEAN, __in USHORT, __out PULONG);
typedef PVOID (NTAPI *RtlImageRvaToVa_t)(__in PIMAGE_NT_HEADERS, __in PVOID, __in ULONG, __inout_opt PIMAGE_SECTION_HEADER *);
typedef NTSTATUS (NTAPI *NtQueryEvent_t)(__in HANDLE, __in EVENT_INFORMATION_CLASS, __out PVOID, __in ULONG, __out_opt PULONG);
typedef NTSTATUS (NTAPI *NtQueryIoCompletion_t)(__in HANDLE, __in IO_COMPLETION_INFORMATION_CLASS, __out PVOID, __in ULONG, __out_opt PULONG);
typedef NTSTATUS (NTAPI *NtQueryMutant_t)(__in HANDLE, __in MUTANT_INFORMATION_CLASS, __out PVOID, __in ULONG, __out_opt PULONG);
typedef NTSTATUS (NTAPI *NtQuerySemaphore_t)(__in HANDLE, __in SEMAPHORE_INFORMATION_CLASS, __out PVOID, __in ULONG, __out_opt PULONG);
typedef NTSTATUS (NTAPI *NtQuerySection_t)(__in HANDLE, __in SECTION_INFORMATION_CLASS, __out PVOID, __in ULONG, __out PULONG);
typedef NTSTATUS (NTAPI *NtQueryTimer_t)(__in HANDLE, __in TIMER_INFORMATION_CLASS, __out PVOID, __in ULONG, __out PULONG);
typedef NTSTATUS (NTAPI *NtOpenDirectoryObject_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtQueryDirectoryObject_t)(__in HANDLE, __out PVOID, __in ULONG, __in BOOLEAN, __in BOOLEAN, __inout PULONG, __out_opt PULONG);
typedef NTSTATUS (NTAPI *NtOpenSymbolicLinkObject_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtQuerySymbolicLinkObject_t)(__in HANDLE, __inout PUNICODE_STRING, __out_opt PULONG);
typedef NTSTATUS (NTAPI *NtOpenEvent_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenMutant_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenSection_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenTimer_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenSemaphore_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenEventPair_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenIoCompletion_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtOpenKey_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES);
typedef NTSTATUS (NTAPI *NtCreateKey_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES, __reserved ULONG, __in_opt PUNICODE_STRING, __in ULONG, __out_opt PULONG);
typedef NTSTATUS (NTAPI *NtEnumerateKey_t)(__in HANDLE, __in ULONG, __in KEY_INFORMATION_CLASS, __out PVOID, __in ULONG, __out PULONG);
typedef NTSTATUS (NTAPI *NtEnumerateValueKey_t)(__in HANDLE, __in ULONG, __in KEY_VALUE_INFORMATION_CLASS, __out PVOID, __in ULONG, __out PULONG);
typedef NTSTATUS (NTAPI *NtQueryKey_t)(__in HANDLE, __in KEY_INFORMATION_CLASS, __out PVOID, __in ULONG, __out PULONG);
typedef NTSTATUS (NTAPI *NtQueryValueKey_t)(__in HANDLE, __in PUNICODE_STRING, __in KEY_VALUE_INFORMATION_CLASS, __out PVOID, __in ULONG, __out PULONG);
typedef NTSTATUS (NTAPI *RtlValidateUnicodeString_t)(__in __reserved ULONG, __in PCUNICODE_STRING);
typedef NTSTATUS (NTAPI *RtlDowncaseUnicodeString_t)(PUNICODE_STRING, __in PCUNICODE_STRING, __in BOOLEAN);
typedef NTSTATUS (NTAPI *RtlGenerate8dot3Name_t)(__in PCUNICODE_STRING, __in BOOLEAN, __inout PGENERATE_NAME_CONTEXT, __inout PUNICODE_STRING);
typedef NTSTATUS (NTAPI *RtlVolumeDeviceToDosName_t)(__in PVOID, __out PUNICODE_STRING);
typedef NTSTATUS (NTAPI *NtCreateSection_t)(__out PHANDLE, __in ACCESS_MASK, __in_opt POBJECT_ATTRIBUTES, __in_opt PLARGE_INTEGER, __in ULONG, __in ULONG, __in_opt HANDLE);
typedef NTSTATUS (NTAPI *NtMapViewOfSection_t)(__in HANDLE, __in HANDLE, __inout PVOID *, __in ULONG_PTR, __in SIZE_T, __inout_opt PLARGE_INTEGER, __inout PSIZE_T, __in SECTION_INHERIT, __in ULONG, __in ULONG);
typedef NTSTATUS (NTAPI *NtUnmapViewOfSection_t)(__in HANDLE, __in_opt PVOID);
typedef NTSTATUS (NTAPI *NtQueryDirectoryFile_t)(__in HANDLE, __in_opt HANDLE, __in_opt PIO_APC_ROUTINE, __in_opt PVOID, __out PIO_STATUS_BLOCK, __out PVOID, __in ULONG, __in NT_FILE_INFORMATION_CLASS, __in BOOLEAN, __in_opt PUNICODE_STRING, __in BOOLEAN);
typedef NTSTATUS (NTAPI *NtQueryInformationFile_t)(__in HANDLE, __out PIO_STATUS_BLOCK, __out PVOID, __in ULONG, __in NT_FILE_INFORMATION_CLASS);
typedef NTSTATUS (NTAPI *NtQueryObject_t)(__in_opt HANDLE, __in OBJECT_INFORMATION_CLASS, __out PVOID, __in ULONG, __out_opt PULONG);
typedef NTSTATUS (NTAPI *NtOpenFile_t)(__out PHANDLE, __in ACCESS_MASK, __in POBJECT_ATTRIBUTES, __out PIO_STATUS_BLOCK, __in ULONG, __in ULONG);

/*
  Use this to declare a variable of the name of a native function and of its
  proper type.
    Example     : NTNATIVE_FUNC(RtlGetVersion);
    Expands to  : RtlGetVersion_t RtlGetVersion;
*/
#define NTNATIVE_FUNC(x) x##_t x
/*
  Use to retrieve a function's pointer by passing a handle. Uses GetProcAddress()
  to do the job.
    Example     : NTNATIVE_GETPROCADDR(hNtDll, RtlGetVersion);
    Expands to  : GetProcAddress(hNtDll, "RtlGetVersion");
*/
#define NTNATIVE_GETPROCADDR(mod, x) GetProcAddress(mod, #x)
/*
  Use to declare a variable of the same name as the function and assign it the
  function's pointer by passing a module handle. Uses GetProcAddress() to do the
  job.
    Example     : NTNATIVE_DEFFUNC(hNtDll, RtlGetVersion);
    Expands to  : RtlGetVersion_t RtlGetVersion = (RtlGetVersion_t)GetProcAddress(hNtDll, "RtlGetVersion");
*/
#define NTNATIVE_DEFFUNC(mod, x) NTNATIVE_FUNC(x) = (x##_t)NTNATIVE_GETPROCADDR(mod, x)
/*
  Use to declare a variable of the same name as the function and assign it the
  function's pointer. Uses GetProcAddress() to do the job.
    Example     : NTDLL_DEFFUNC(RtlGetVersion);
    Expands to  : RtlGetVersion_t RtlGetVersion = (RtlGetVersion_t)GetProcAddress(hNtDll, "RtlGetVersion");
*/
#define NTDLL_DEFFUNC(x) NTNATIVE_DEFFUNC(hNtDll, x)

#endif

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
#define ZwQueryIoCompletion NtQueryIoCompletion
#define ZwQueryMutant NtQueryMutant
#define ZwQuerySemaphore NtQuerySemaphore
#define ZwQuerySection NtQuerySection
#define ZwQueryTimer NtQueryTimer
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
#define ZwQueryDirectoryFile NtQueryDirectoryFile
#define ZwQueryInformationFile NtQueryInformationFile

#if defined(__cplusplus)
}
#endif

#endif // __NTNATIVE_H_VER__
