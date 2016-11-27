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
#include "../ntnative.h"

NTSTATUS
NTAPI
NtClose (
    IN HANDLE Handle
    ) { return 0; }

NTSTATUS
NTAPI
NtCreateFile (
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenFile (
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    ) { return 0; }

NTSTATUS
NTAPI
NtDeviceIoControlFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    ) { return 0; }

NTSTATUS
NTAPI
NtWaitForSingleObject (
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    ) { return 0; }

BOOLEAN
NTAPI
RtlIsNameLegalDOS8Dot3 (
    IN PUNICODE_STRING Name,
    IN OUT POEM_STRING OemName OPTIONAL,
    IN OUT PBOOLEAN NameContainsSpaces OPTIONAL
    ) { return 0; }

ULONG
NTAPI
RtlNtStatusToDosError (
   NTSTATUS Status
   ) { return 0; }

NTSTATUS
NTAPI
NtQueryInformationProcess (
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    ) { return 0; }

NTSTATUS
NTAPI
NtQueryInformationThread (
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    ) { return 0; }

NTSTATUS
NTAPI
NtQueryObject (
    __in_opt HANDLE Handle,
    __in OBJECT_INFORMATION_CLASS ObjectInformationClass,
    __out_bcount_opt(ObjectInformationLength) PVOID ObjectInformation,
    __in ULONG ObjectInformationLength,
    __out_opt PULONG ReturnLength
    ) { return 0; }

NTSTATUS
NTAPI
NtQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    ) { return 0; }

NTSTATUS
NTAPI
NtQuerySystemTime (
    OUT PLARGE_INTEGER SystemTime
    ) { return 0; }

NTSTATUS
NTAPI
RtlLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    ) { return 0; }

BOOLEAN
NTAPI
RtlTimeToSecondsSince1970 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    ) { return 0; }

VOID
NTAPI
RtlFreeAnsiString (
    PANSI_STRING AnsiString
    ) { }

VOID
NTAPI
RtlFreeUnicodeString (
    PUNICODE_STRING UnicodeString
    ) { }

VOID
NTAPI
RtlFreeOemString(
    POEM_STRING OemString
    ) { }

VOID
NTAPI
RtlInitString (
    PSTRING DestinationString,
    PCSZ SourceString
    ) { }

VOID
NTAPI
RtlInitAnsiString (
    PANSI_STRING DestinationString,
    PCSZ SourceString
    ) { }

VOID
NTAPI
RtlInitUnicodeString (
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    ) { }

NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString (
    PUNICODE_STRING DestinationString,
    PCANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    ) { return 0; }

NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString (
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    ) { return 0; }

NTSTATUS
NTAPI
RtlUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    ) { return 0; }

NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(
    __out PULONG BytesInMultiByteString,
    __in_bcount(BytesInUnicodeString) PWCH UnicodeString,
    __in ULONG BytesInUnicodeString
    ) { return 0; }

NTSTATUS
NTAPI
RtlCharToInteger (
    PCSZ String,
    ULONG Base,
    PULONG Value
    ) { return 0; }

NTSTATUS
NTAPI
RtlConvertSidToUnicodeString (
    PUNICODE_STRING UnicodeString,
    PSID Sid,
    BOOLEAN AllocateDestinationString
    ) { return 0; }

/*********************************  ntnative.h ***********************************/
NTSTATUS
NTAPI
RtlGetVersion (
    LPOSVERSIONINFOW lpVersionInformation
    ) { return 0; }

PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(
    IN PVOID Base
    ) { return NULL; }

PVOID
NTAPI
RtlImageDirectoryEntryToData(
    __in PVOID Base,
    __in BOOLEAN MappedAsImage,
    __in USHORT DirectoryEntry,
    __out PULONG Size
    ) { return NULL; }

PVOID
NTAPI
RtlImageRvaToVa(
    __in PIMAGE_NT_HEADERS NtHeaders,
    __in PVOID Base,
    __in ULONG Rva,
    __inout_opt PIMAGE_SECTION_HEADER *LastRvaSection
    ) { return NULL; }

NTSTATUS
NTAPI
NtQueryEvent (
    __in HANDLE EventHandle,
    __in EVENT_INFORMATION_CLASS EventInformationClass,
    __out_bcount(EventInformationLength) PVOID EventInformation,
    __in ULONG EventInformationLength,
    __out_opt PULONG ReturnLength
    ) { return 0; }

NTSTATUS
NTAPI
NtQueryKey(
    __in HANDLE KeyHandle,
    __in KEY_INFORMATION_CLASS KeyInformationClass,
    __out_bcount_opt(Length) PVOID KeyInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    ) { return 0; }

NTSTATUS
NTAPI
NtQueryIoCompletion(
    __in HANDLE IoCompletionHandle,
    __in IO_COMPLETION_INFORMATION_CLASS InformationClass,
    __out PVOID IoCompletionInformation,
    __in ULONG InformationBufferLength,
    __out PULONG RequiredLength OPTIONAL
    ) { return 0; }

NTSTATUS
NTAPI
NtQueryMutant(
    __in HANDLE MutantHandle,
    __in MUTANT_INFORMATION_CLASS MutantInformationClass,
    __out_bcount(MutantInformationLength) PVOID MutantInformation,
    __in ULONG MutantInformationLength,
    __out_opt PULONG ReturnLength
    ) { return 0; }

NTSTATUS
NTAPI
NtQuerySemaphore (
    __in HANDLE SemaphoreHandle,
    __in SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    __out_bcount(SemaphoreInformationLength) PVOID SemaphoreInformation,
    __in ULONG SemaphoreInformationLength,
    __out_opt PULONG ReturnLength
    ) { return 0; }

NTSTATUS
NTAPI
NtQuerySection(
    IN HANDLE SectionHandle,
    IN SECTION_INFORMATION_CLASS SectionInformationClass,
    OUT PVOID SectionInformation,
    IN ULONG SectionInformationLength,
    OUT PULONG ReturnLength OPTIONAL
     ) { return 0; }

NTSTATUS
NTAPI
NtQueryTimer(
    __in HANDLE TimerHandle,
    __in TIMER_INFORMATION_CLASS TimerInformationClass,
    __out PVOID TimerInformation,
    __in ULONG TimerInformationLength,
    __out PULONG ReturnLength OPTIONAL
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenTimer(
    __out PHANDLE TimerHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
) { return 0; }

NTSTATUS
NTAPI
NtOpenDirectoryObject(
    __out PHANDLE DirectoryHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

NTSTATUS
NTAPI
NtQueryDirectoryObject (
    __in HANDLE DirectoryHandle,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in BOOLEAN ReturnSingleEntry,
    __in BOOLEAN RestartScan,
    __inout PULONG Context,
    __out_opt PULONG ReturnLength
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenSymbolicLinkObject(
    __out PHANDLE LinkHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

NTSTATUS
NTAPI
NtQuerySymbolicLinkObject(
    __in HANDLE LinkHandle,
    __inout PUNICODE_STRING LinkTarget,
    __out_opt PULONG ReturnedLength
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenEvent (
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenMutant (
    __out PHANDLE MutantHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenSection(
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenSemaphore (
    __out PHANDLE SemaphoreHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenEventPair(
    __out PHANDLE EventPairHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenIoCompletion (
    __out PHANDLE IoCompletionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

NTSTATUS
NTAPI
NtOpenKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    ) { return 0; }

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
    ) { return 0; }

NTSTATUS
NTAPI
NtEnumerateKey(
    __in HANDLE KeyHandle,
    __in ULONG Index,
    __in KEY_INFORMATION_CLASS KeyInformationClass,
    __out_bcount_opt(Length) PVOID KeyInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    ) { return 0; }

NTSTATUS
NTAPI
NtEnumerateValueKey(
    __in HANDLE KeyHandle,
    __in ULONG Index,
    __in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_bcount_opt(Length) PVOID KeyValueInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    ) { return 0; }

NTSTATUS
NTAPI
NtQueryValueKey(
    __in HANDLE KeyHandle,
    __in PUNICODE_STRING ValueName,
    __in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_bcount_opt(Length) PVOID KeyValueInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    ) { return 0; }

NTSTATUS
NTAPI
RtlValidateUnicodeString(
    __in __reserved ULONG Flags,
    __in PCUNICODE_STRING String
    ) { return 0; }

NTSTATUS
NTAPI
RtlDowncaseUnicodeString(
         PUNICODE_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    ) { return 0; }

NTSTATUS /* VOID in pre-Vista*/
NTAPI
RtlGenerate8dot3Name (
    __in    PCUNICODE_STRING Name,
    __in    BOOLEAN AllowExtendedCharacters,
    __inout PGENERATE_NAME_CONTEXT Context,
    __inout PUNICODE_STRING Name8dot3
    ) { return 0; }

NTSTATUS
NTAPI
RtlVolumeDeviceToDosName(
    __in PVOID VolumeDeviceObject,
    __out PUNICODE_STRING DosName
    ) { return 0; }

NTSTATUS
NTAPI
NtCreateSection (
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt PLARGE_INTEGER MaximumSize,
    __in ULONG SectionPageProtection,
    __in ULONG AllocationAttributes,
    __in_opt HANDLE FileHandle
    ) { return 0; }

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
    ) { return 0; }

NTSTATUS
NTAPI
NtUnmapViewOfSection(
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress
    ) { return 0; }

