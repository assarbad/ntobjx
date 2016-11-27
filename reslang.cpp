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

#if 0
typedef struct _LDR_RESOURCE_INFO
{
    ULONG_PTR Type;
    ULONG_PTR Name;
    ULONG_PTR Language;
} LDR_RESOURCE_INFO, *PLDR_RESOURCE_INFO;

typedef NTSTATUS(NTAPI * TFNLdrFindResource_U)(PVOID, PLDR_RESOURCE_INFO, ULONG, PIMAGE_RESOURCE_DATA_ENTRY *);
static TFNLdrFindResource_U realLdrFindResource_U = NULL;

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
    return STATUS_INVALID_PARAMETER;
}
#endif // 0

void HookFindResource()
{
#if 0
    HMODULE hKrnlBase = ::GetModuleHandle(_T("kernel32.dll"));
    ATLASSERT(hKrnlBase != NULL);
    PVOID lpKrnlBase = static_cast<PVOID>(hKrnlBase);
    ATLTRACE2(_T("kernel32.dll = %p\n"), lpKrnlBase);
    if (PIMAGE_NT_HEADERS nthdrs = RtlImageNtHeader(lpKrnlBase))
    {
        ATLTRACE2(_T("nthdrs[%p]\n"), nthdrs);
        ULONG whatever;
        if (PIMAGE_IMPORT_DESCRIPTOR piid = static_cast<PIMAGE_IMPORT_DESCRIPTOR>(RtlImageDirectoryEntryToData(lpKrnlBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &whatever)))
        {
            ATLTRACE2(_T("piid[%p]; piid->Name[%p]\n"), piid, piid->Name);
            int idx = 0;
            while(piid->Name)
            {
                
                LPCSTR dllname = (LPCSTR)RtlImageRvaToVa(nthdrs, lpKrnlBase, piid->Name, NULL);
                ATLTRACE2(_T("{%p} piid[%i].Name = %08X == %u == %hs\n"), piid, idx, piid->Name, piid->Name, dllname);
                if (0 == _strcmpi(dllname, "ntdll.dll"))
                {
                    PULONG ft = (PULONG)RtlImageRvaToVa(nthdrs, lpKrnlBase, piid->FirstThunk, NULL);
                    PULONG oft = (PULONG)RtlImageRvaToVa(nthdrs, lpKrnlBase, piid->OriginalFirstThunk, NULL);
                    for (size_t i = 0; oft[i] && ft[i]; i++)
                    {
                        ATLASSERT(oft[i] != 0);
                        ATLASSERT(ft[i] != 0);
                        PIMAGE_THUNK_DATA oitd = (PIMAGE_THUNK_DATA)RtlImageRvaToVa(nthdrs, lpKrnlBase, oft[i], NULL);
                        ATLASSERT(oitd != NULL);
                        ATLASSERT(oitd->u1.AddressOfData != 0);
                        ATLASSERT(0 == (oitd->u1.Ordinal & IMAGE_ORDINAL_FLAG));
                        PIMAGE_IMPORT_BY_NAME oiibn = (PIMAGE_IMPORT_BY_NAME)oitd->u1.AddressOfData;
                        ATLTRACE2(_T("PIMAGE_IMPORT_BY_NAME[%p] = %p; .name = %p\n"), oiibn, &oiibn->Name);
                        ATLASSERT(oiibn != NULL);
                    }
                }
                piid++;
                idx++;
            }
        }
    }
#endif // 0
}

bool CLanguageSetter::operator!() const
{
    return !operator bool();
}

CLanguageSetter::CLanguageSetter(LANGID fallback /*= MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)*/, HMODULE hInstance /*= ModuleHelper::GetResourceInstance()*/, LPCTSTR lpType /*= RT_DIALOG*/, LPCTSTR lpName /*= MAKEINTRESOURCE(IDD_ABOUT)*/) : m_dwMajorVersion(0)
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