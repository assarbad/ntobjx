///////////////////////////////////////////////////////////////////////////////
///
/// Fixups for the messed up implementation of switching languages in Windows
/// versions prior to Windows Vista.
/// Hint: If you are curious, SetThreadLanguage() doesn't really do what the
/// documentation says it does. And numerous threads on the web tell one to
/// use an external resource-only DLL and use that to load any kind of
/// resources for a specific language. This makes it pretty much impossible to
/// create a monolithic multilingual executable for Windows versions prior to
/// Vista.
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
#include "reslang.h"

///////////////////////////////////////////////////////////////////////////////
/// The idea here is to hook the LdrFindResource_U function which is internally
/// used by all those resource loading functions and FindResource/Ex.
/// This should be safe to do since we're only aiming for no longer maintained
/// Windows versions which are bound not to change. And if they end up changing
/// someone will perhaps be nice enough to point it out in a defect report.
///////////////////////////////////////////////////////////////////////////////

namespace
{
    typedef struct _LDR_RESOURCE_INFO
    {
        ULONG_PTR Type;
        ULONG_PTR Name;
        ULONG_PTR Language;
    } LDR_RESOURCE_INFO, *PLDR_RESOURCE_INFO;

    typedef NTSTATUS(NTAPI * TFNLdrFindResource_U)(PVOID, PLDR_RESOURCE_INFO, ULONG, PIMAGE_RESOURCE_DATA_ENTRY *);
    static TFNLdrFindResource_U realLdrFindResource_U = NULL;
    static PVOID hResModule = NULL;

    NTSTATUS
        NTAPI
        LocalLdrFindResource_U(
        __in PVOID BaseAddress,
        __in PLDR_RESOURCE_INFO ResourceIdPath,
        __in ULONG ResourceIdPathLength,
        __out PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
        )
    {
        ATLASSERT(realLdrFindResource_U != NULL);
        if (hResModule == BaseAddress)
        {
            ATLTRACE2(_T("Hooked LdrFindResource_U(%p, %p, %u, %p)\n"), BaseAddress, ResourceIdPath, ResourceIdPathLength, ResourceDataEntry);
            if (ResourceIdPath && (3 == ResourceIdPathLength))
            {
                if (IS_INTRESOURCE(ResourceIdPath->Type))
                {
                    ATLTRACE2(_T("Resource type: %u (%08X); "), ResourceIdPath->Type, ResourceIdPath->Type);
                }
                else
                {
                    ATLTRACE2(_T("Resource type: %s; "), (LPCWSTR)ResourceIdPath->Type);
                }
                if (IS_INTRESOURCE(ResourceIdPath->Name))
                {
                    ATLTRACE2(_T("name: %u (%08X); "), ResourceIdPath->Name, ResourceIdPath->Name);
                }
                else
                {
                    ATLTRACE2(_T("name: %s; "), (LPCWSTR)ResourceIdPath->Name);
                }
                ATLTRACE2(_T("language: %08X (%u)\n"), ResourceIdPath->Language, ResourceIdPath->Language);
            }
            else
            {
                ATLTRACE2(_T("Not implemented for ResourceIdPathLength{%u} != 3\n"), ResourceIdPathLength);
            }
        }
        return realLdrFindResource_U(BaseAddress, ResourceIdPath, ResourceIdPathLength, ResourceDataEntry);
    }
}

void HookLdrFindResource_U(HINSTANCE resmodule)
{
    if (realLdrFindResource_U)
    {
        ATLTRACE2(_T("LdrFindResource_U already hooked\n"));
        return;
    }
    LPCTSTR lpszKernel32 = _T("kernel32.dll");
    LPCSTR lpszNtdll = "ntdll.dll";
    HMODULE hKrnlBase = ::GetModuleHandle(lpszKernel32);
    ATLASSERT(hKrnlBase != NULL);
    PVOID lpKrnlBase = (PVOID)hKrnlBase;
    ATLTRACE2(_T("%s = %p\n"), lpszKernel32, lpKrnlBase);
    ULONG_PTR origLdrFindResource_U = (ULONG_PTR)::GetProcAddress(::GetModuleHandleA(lpszNtdll), "LdrFindResource_U");
    ATLASSERT(origLdrFindResource_U != NULL);
    if (PIMAGE_NT_HEADERS nthdrs = RtlImageNtHeader(lpKrnlBase))
    {
        ATLTRACE2(_T("nthdrs[%p]\n"), nthdrs);
        ATLASSERT(nthdrs->OptionalHeader.NumberOfRvaAndSizes >= IMAGE_DIRECTORY_ENTRY_IMPORT);
        ULONG whatever;
        if (PIMAGE_IMPORT_DESCRIPTOR piid = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(lpKrnlBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &whatever))
        {
            while(piid->Name)
            {
                LPCSTR dllname = (LPCSTR)(piid->Name + (LPSTR)lpKrnlBase);
                if (0 == _strcmpi(dllname, lpszNtdll))
                {
                    // Since we don't have the OriginalFirstThunk info, we need
                    // to specify the "real" function address to search for.
                    ATLTRACE2(_T("Matched DLL name: %hs\n"), dllname);
                    // FirstThunk must be a valid RVA
                    if(piid->FirstThunk)
                    {
                        PULONG_PTR FirstThunk = (PULONG_PTR)(piid->FirstThunk + (LPSTR)(lpKrnlBase));
                        // Go through the list until we reach both RVAs with zero
                        while(*FirstThunk)
                        {
                            // Go through the IAT searching for the specific function
                            if(*FirstThunk == origLdrFindResource_U)
                            {
                                ATLTRACE2(_T("Found LdrFindResource_U function pointer\n"));
                                // Got it ...
                                DWORD dwOldProtect = 0;
                                // Un-protect the memory (i.e. make it writable)
                                if (::VirtualProtect(FirstThunk, sizeof(ULONG_PTR), PAGE_READWRITE, &dwOldProtect))
                                {
                                    __try
                                    {
                                        // Exchange the pointer with ours, retrieving the old one ...
                                        realLdrFindResource_U = (TFNLdrFindResource_U)(InterlockedExchangePointer((PVOID*)FirstThunk, (PVOID)LocalLdrFindResource_U));
                                        ATLTRACE2(_T("Hooked the LdrFindResource_U function\n"));
                                        hResModule = resmodule;
                                    }
                                    __finally
                                    {
                                        // Re-protect the memory (i.e. make it writable)
                                        ::VirtualProtect(FirstThunk, sizeof(ULONG_PTR), dwOldProtect, &dwOldProtect);
                                    }
                                }
                                break;
                            }
                            FirstThunk++;
                        }
                    }
                }
                piid++;
            }
        }
    }
}

bool CLanguageSetter::operator!() const
{
    return !operator bool();
}

CLanguageSetter::CLanguageSetter(LANGID fallback /*= MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)*/, HMODULE hInstance /*= ModuleHelper::GetResourceInstance()*/, LPCTSTR lpType /*= RT_DIALOG*/, LPCTSTR lpName /*= MAKEINTRESOURCE(IDD_ABOUT)*/)
    : m_dwMajorVersion(0)
    , m_pfnSetThreadUILanguage(0)
    , m_langIDs()
    , m_fallback(fallback)
    , m_bHasLists(EnumAboutBoxLanguages_(hInstance, lpType, lpName))
{
    OSVERSIONINFOEX osvix = { 0 };
    osvix.dwMajorVersion = 6;
    ULONGLONG dwlConditionMask = 0;
    dwlConditionMask = ::VerSetConditionMask(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    m_dwMajorVersion = ::VerifyVersionInfo(&osvix, VER_MAJORVERSION, dwlConditionMask) ? 6 : 5;
    ATLASSERT(m_dwMajorVersion != 0);

    if (m_dwMajorVersion >= 6)
    {
        m_pfnSetThreadUILanguage = (TFNSetThreadUILanguage)(::GetProcAddress(::GetModuleHandle(_T("kernel32.dll")), "SetThreadUILanguage"));
        ATLASSERT(m_pfnSetThreadUILanguage != 0);
    }
    else
    {
        ATLASSERT((m_dwMajorVersion < 6) && (m_pfnSetThreadUILanguage == 0));
    }
}

CLanguageSetter::operator bool() const
{
    return m_bHasLists;
}

LANGID CLanguageSetter::set(LANGID langID /*= ::GetUserDefaultLangID()*/)
{
    langID = matchResourceLang_(LANGIDFROMLCID(langID));
    if (m_pfnSetThreadUILanguage) // Vista and newer
    {
        ATLVERIFY(langID == m_pfnSetThreadUILanguage(langID));
        ATLTRACE2(_T("Using method for Vista and newer with SetThreadUILanguage(%04X)\n"), langID);
    }
    else // Windows XP, 2003 Server and older
    {
        ATLVERIFY(::SetThreadLocale(MAKELCID(langID, SORT_DEFAULT)));
        ATLTRACE2(_T("Using method for older Windows versions SetThreadLocale(%04X) -- GetThreadLocale() returns %04X\n"), MAKELCID(langID, SORT_DEFAULT), ::GetThreadLocale());
        ATLASSERT(::GetThreadLocale() == MAKELCID(langID, SORT_DEFAULT));
    }
    return langID;
}

LANGID CLanguageSetter::matchResourceLang_(LANGID localeID) const
{
    if (m_bHasLists)
    {
        LANGID const tomatch = PRIMARYLANGID(localeID);

        for (int i = 0; i < m_langIDs.GetSize(); i++)
        {
            LANGID curr = PRIMARYLANGID(m_langIDs[i]);

            if (tomatch == curr)
            {
                ATLTRACE2(_T("Returning first match based on primary language identifier %04X (primary lang: %04X).\n"), m_langIDs[i], tomatch);
                return m_langIDs[i];
            }
        }
    }
    return m_fallback; // our fallback LANGID
}

BOOL CALLBACK CLanguageSetter::NtObjectsEnumResLangProc_(HMODULE /*hModule*/, LPCWSTR /*lpType*/, LPCWSTR /*lpName*/, WORD wLanguage, LONG_PTR lParam)
{
    RESLANGIDLIST* lpResLangIdList = (RESLANGIDLIST*)lParam;
    ATLASSERT(lpResLangIdList != NULL);

    if (wLanguage == MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
    {
        ATLTRACE2(_T("Skipping neutral language resource\n"));
        return TRUE;
    }

    ATLASSERT(wLanguage != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    if (lpResLangIdList != NULL)
    {
        RESLANGIDLIST& resLangIdList = *lpResLangIdList;
        size_t& index = resLangIdList.index;
        if (index < resLangIdList.size)
        {
            resLangIdList.list[index] = wLanguage;
            index++;
        }
    }
    return TRUE;
}

bool CLanguageSetter::EnumAboutBoxLanguages_(HMODULE hInstance, LPCTSTR lpType, LPCTSTR lpName)
{
    bool retval = false;
    CTempBuffer<LANGID, RES_LANGID_LIST_LEN> resLangIdListBuf;
    LANGID* list = resLangIdListBuf.Allocate(RES_LANGID_LIST_LEN);
    ATLASSERT(list != NULL);

    if (list != NULL)
    {
        RESLANGIDLIST resLangIdList = { RES_LANGID_LIST_LEN, 0, list };
        memset(list, 0, sizeof(LANGID) * RES_LANGID_LIST_LEN);

        // We use the about box resource as our sentinel which languages are available
        retval = FALSE != ::EnumResourceLanguages(hInstance, lpType, lpName, NtObjectsEnumResLangProc_, (LONG_PTR)(&resLangIdList));

        if (retval)
        {
            for (size_t i = 0; i < resLangIdList.index; i++)
            {
                ATLASSERT(i < resLangIdList.size);
                m_langIDs.Push(resLangIdList.list[i]);
                ATLTRACE2(_T("Found resource language #%i %04X (%u)\n"), (int)i + 1, resLangIdList.list[i], resLangIdList.list[i]);
            }
        }
    }
    return retval;
}
