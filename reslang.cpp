///////////////////////////////////////////////////////////////////////////////
///
/// Fixups for the messed up implementation of switching languages in Windows
/// versions prior to Windows Vista.
/// Hint: If you are curious, SetThreadLanguage() doesn't really do what the
/// documentation says it does. And numerous threads on the web tell one to
/// use an external resource-only DLL and use that to load any kind of
/// resources for a specific language. This makes it pretty much impossible to
/// create a monolithic multilingual executable for Windows versions prior to
/// Vista. This implementation attempts to fix that. So far successfully.
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

#include "common.h"

#include "reslang.h"

///////////////////////////////////////////////////////////////////////////////
/// The idea here is to hook the LdrFindResource_U function which is internally
/// used by all those resource loading functions and FindResource/Ex.
/// This should be safe to do since we're only aiming for no longer maintained
/// Windows versions which are bound not to change. And if they end up changing
/// someone will perhaps be nice enough to point it out in a defect report.
///////////////////////////////////////////////////////////////////////////////

CLanguageSetter gLangSetter = CLanguageSetter();

namespace
{
    typedef struct _LDR_RESOURCE_INFO
    {
        ULONG_PTR Type;
        ULONG_PTR Name;
        ULONG_PTR Language;
    } LDR_RESOURCE_INFO, *PLDR_RESOURCE_INFO;

    typedef NTSTATUS(NTAPI* TFNLdrFindResource_U)(PVOID, PLDR_RESOURCE_INFO, ULONG, PIMAGE_RESOURCE_DATA_ENTRY*);
    static TFNLdrFindResource_U realLdrFindResource_U = NULL;
    static PVOID hResModule = NULL;

    NTSTATUS
    NTAPI
    LocalLdrFindResource_U(__in PVOID BaseAddress,
                           __in PLDR_RESOURCE_INFO ResourceIdPath,
                           __in ULONG ResourceIdPathLength,
                           __out PIMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry)
    {
        ATLASSERT(realLdrFindResource_U != NULL);
        if (hResModule == BaseAddress) // we only filter our own resource module (this .exe)
        {
            ATLTRACE2(_T("Hooked LdrFindResource_U(%p, %p, %u, %p): "),
                      BaseAddress,
                      ResourceIdPath,
                      ResourceIdPathLength,
                      ResourceDataEntry);
            if (ResourceIdPath && (ResourceIdPathLength >= 2))
            {
#ifdef _DEBUG
                /*lint -save -e30*/
                if (IS_INTRESOURCE(ResourceIdPath->Type))
                {
                    LPCTSTR rt = NULL;
                    switch (ResourceIdPath->Type)
                    {
                    case RT_CURSOR:
                        rt = _T("RT_CURSOR");
                        break;
                    case RT_BITMAP:
                        rt = _T("RT_BITMAP");
                        break;
                    case RT_ICON:
                        rt = _T("RT_ICON");
                        break;
                    case RT_MENU:
                        rt = _T("RT_MENU");
                        break;
                    case RT_DIALOG:
                        rt = _T("RT_DIALOG");
                        break;
                    case RT_STRING:
                        rt = _T("RT_STRING");
                        break;
                    case RT_FONTDIR:
                        rt = _T("RT_FONTDIR");
                        break;
                    case RT_FONT:
                        rt = _T("RT_FONT");
                        break;
                    case RT_ACCELERATOR:
                        rt = _T("RT_ACCELERATOR");
                        break;
                    case RT_RCDATA:
                        rt = _T("RT_RCDATA");
                        break;
                    case RT_MESSAGETABLE:
                        rt = _T("RT_MESSAGETABLE");
                        break;
                    case RT_GROUP_CURSOR:
                        rt = _T("RT_GROUP_CURSOR");
                        break;
                    case RT_GROUP_ICON:
                        rt = _T("RT_GROUP_ICON");
                        break;
                    case RT_VERSION:
                        rt = _T("RT_VERSION");
                        break;
                    case RT_DLGINCLUDE:
                        rt = _T("RT_DLGINCLUDE");
                        break;
                    case RT_PLUGPLAY:
                        rt = _T("RT_PLUGPLAY");
                        break;
                    case RT_VXD:
                        rt = _T("RT_VXD");
                        break;
                    case RT_ANICURSOR:
                        rt = _T("RT_ANICURSOR");
                        break;
                    case RT_ANIICON:
                        rt = _T("RT_ANIICON");
                        break;
                    case RT_HTML:
                        rt = _T("RT_HTML");
                        break;
                    case RT_MANIFEST:
                        rt = _T("RT_MANIFEST");
                        break;
                    default:
                        rt = NULL;
                        break;
                    }
                    if (rt)
                        ATLTRACE2(_T("type: %15s; "), rt);
                    else
                        ATLTRACE2(_T("type: % 15u; "), ResourceIdPath->Type);
                }
                else
                {
                    ATLTRACE2(_T("Resource type: %s; "), (LPCWSTR)ResourceIdPath->Type);
                }
                if (IS_INTRESOURCE(ResourceIdPath->Name))
                {
                    ATLTRACE2(_T("name: % 5u; "), ResourceIdPath->Name);
                }
                else
                {
                    ATLTRACE2(_T("name: %s; "), (LPCWSTR)ResourceIdPath->Name);
                }
                WCHAR name[MAX_PATH] = {0};
                if (::GetLocaleInfo((LCID)ResourceIdPath->Language, LOCALE_SENGLANGUAGE, name, _countof(name)))
                    ATLTRACE2(_T("language: %s (%u)\n"),
                              (ResourceIdPath->Language) ? name : _T("Neutral"),
                              ResourceIdPath->Language);
                else
                    ATLTRACE2(_T("language: %04X (%u)\n"), ResourceIdPath->Language, ResourceIdPath->Language);
                    /*lint -restore*/
#endif                                         // _DEBUG
                if (!ResourceIdPath->Language) // Neutral?
                {
                    LDR_RESOURCE_INFO lri = {ResourceIdPath->Type, ResourceIdPath->Name, 0};
                    lri.Language = gLangSetter.picked();
                    NTSTATUS status = realLdrFindResource_U(BaseAddress, &lri, ResourceIdPathLength, ResourceDataEntry);
                    if (NT_ERROR(status))
                    {
                        ATLTRACE2(_T("LdrFindResource_U failed with %08X and manipulated ResourceIdPath. Retrying ")
                                  _T("with original values.\n"),
                                  status);
                        status =
                            realLdrFindResource_U(BaseAddress, ResourceIdPath, ResourceIdPathLength, ResourceDataEntry);
                        if (NT_ERROR(status))
                        {
                            ATLTRACE2(
                                _T("LdrFindResource_U failed (again) with %08X even with original ResourceIdPath.\n"),
                                status);
                        }
                    }
                    return status;
                }
            }
            else
            {
                ATLTRACE2(_T("not implemented for ResourceIdPathLength{%u} != 3\n"), ResourceIdPathLength);
            }
        }
        return realLdrFindResource_U(BaseAddress, ResourceIdPath, ResourceIdPathLength, ResourceDataEntry);
    }
} // namespace

BOOL HookLdrFindResource_U(HINSTANCE resmodule)
{
    if (realLdrFindResource_U)
    {
        ATLTRACE2(_T("LdrFindResource_U already hooked\n"));
        return TRUE;
    }
    LPCTSTR lpszKernel32 = _T("kernel32.dll");
    LPCSTR lpszNtdll = "ntdll.dll";
    HMODULE hKrnlBase = ::GetModuleHandle(lpszKernel32);
    ATLASSERT(hKrnlBase != NULL);
    PVOID lpKrnlBase = (PVOID)hKrnlBase;
    if (!lpKrnlBase)
    {
        ATLTRACE2(_T("We could not get a module handle for kernel32.dll? What kind of OS is this running on? ")
                  _T("Congratulations for tickling out this condition.\n"));
        return FALSE;
    }
    ATLTRACE2(_T("%s = %p\n"), lpszKernel32, lpKrnlBase);
    HMODULE hNtdll = ::GetModuleHandleA(lpszNtdll);
    if (!hNtdll)
    {
        ATLTRACE2(_T("We could not get a module handle for ntdll.dll? What kind of OS is this running on? ")
                  _T("Congratulations for tickling out this condition.\n"));
        return FALSE;
    }
    ULONG_PTR origLdrFindResource_U = (ULONG_PTR)::GetProcAddress(hNtdll, "LdrFindResource_U");
    ATLASSERT(origLdrFindResource_U != NULL);
    if (PIMAGE_NT_HEADERS nthdrs = RtlImageNtHeader(lpKrnlBase))
    {
        ATLTRACE2(_T("nthdrs[%p]\n"), nthdrs);
        ATLASSERT(nthdrs->OptionalHeader.NumberOfRvaAndSizes >= IMAGE_DIRECTORY_ENTRY_IMPORT);
        ULONG whatever;
        if (PIMAGE_IMPORT_DESCRIPTOR piid = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                lpKrnlBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &whatever))
        {
            while (piid->Name)
            {
                LPCSTR dllname = (LPCSTR)(piid->Name + (LPSTR)lpKrnlBase);
#pragma warning(suppress : 6387)
                if (0 == _strcmpi(dllname, lpszNtdll))
                {
                    // Since we don't have the OriginalFirstThunk info, we need
                    // to specify the "real" function address to search for.
                    ATLTRACE2(_T("Matched DLL name: %hs\n"), dllname);
                    // FirstThunk must be a valid RVA
                    if (piid->FirstThunk)
                    {
                        PULONG_PTR FirstThunk = (PULONG_PTR)(piid->FirstThunk + (LPSTR)(lpKrnlBase));
                        // Go through the list until we reach both RVAs with zero
                        while (*FirstThunk)
                        {
                            // Go through the IAT searching for the specific function
                            if (*FirstThunk == origLdrFindResource_U)
                            {
                                ATLTRACE2(_T("Found LdrFindResource_U function pointer\n"));
                                // Got it ...
                                __try
                                {
                                    MEMORY_BASIC_INFORMATION mbi;
                                    if (::VirtualQuery(FirstThunk, &mbi, sizeof(mbi)))
                                    {
                                        ATLTRACE2(_T("\
MEMORY_BASIC_INFORMATION mbi == {\n\
    .BaseAddress == %p;\n\
    .AllocationBase == %p;\n\
    .AllocationProtect == %08X;\n\
    .RegionSize == %u;\n\
    .State == %08X;\n\
    .Protect == %08X;\n\
    .Type == %08X;\n\
}; // %d-bit, address of interest %p\n"),
                                                  mbi.BaseAddress,
                                                  mbi.AllocationBase,
                                                  mbi.AllocationProtect,
                                                  mbi.RegionSize,
                                                  mbi.State,
                                                  mbi.Protect,
                                                  mbi.Type,
                                                  sizeof(void*) * 8,
                                                  FirstThunk);
                                        DWORD dwOldProtect = 0;
                                        // Un-protect the memory (i.e. make it writable)
                                        if (::VirtualProtect(
                                                FirstThunk, sizeof(ULONG_PTR), PAGE_EXECUTE_READWRITE, &dwOldProtect))
                                        {
                                            __try
                                            {
                                                // Exchange the pointer with ours, retrieving the old one ...
                                                /*lint -save -e611*/
                                                realLdrFindResource_U =
                                                    (TFNLdrFindResource_U)(InterlockedExchangePointer(
                                                        (PVOID*)FirstThunk, (PVOID)LocalLdrFindResource_U));
                                                /*lint -restore*/
                                                ATLTRACE2(_T("Hooked the LdrFindResource_U function\n"));
                                                hResModule = resmodule;
#pragma warning(suppress : 6242)
                                                return TRUE;
                                            }
                                            __finally
                                            {
                                                // Re-protect the memory (i.e. make it read-only again, typically)
                                                ::VirtualProtect(
                                                    FirstThunk, sizeof(ULONG_PTR), dwOldProtect, &dwOldProtect);
                                            }
                                        }
                                    }
                                }
                                __except (EXCEPTION_EXECUTE_HANDLER)
                                {
                                    ATLTRACE2(
                                        _T("Exception when unprotecting the IAT entry of LdrFindResource_U at %p\n"),
                                        FirstThunk);
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
    return FALSE;
}

bool CLanguageSetter::operator!() const
{
    return !operator bool();
}

CLanguageSetter::CLanguageSetter(LANGID fallback /*= MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)*/,
                                 HMODULE hInstance /*= ModuleHelper::GetResourceInstance()*/,
                                 LPCTSTR lpType /*= RT_DIALOG*/,
                                 LPCTSTR lpName /*= MAKEINTRESOURCE(IDD_ABOUT)*/)
    : m_dwMajorVersion(0)
    , m_pfnSetThreadUILanguage(0)
    , m_langIDs()
    , m_fallback(fallback)
    , m_pickedLangID(fallback)
    , m_bHasLists(EnumAboutBoxLanguages_(hInstance, lpType, lpName))
{
    OSVERSIONINFOEX osvix = {0};
    osvix.dwMajorVersion = 6;
    ULONGLONG dwlConditionMask = 0;
    dwlConditionMask = ::VerSetConditionMask(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    m_dwMajorVersion = ::VerifyVersionInfo(&osvix, VER_MAJORVERSION, dwlConditionMask) ? 6 : 5;
    ATLASSERT(m_dwMajorVersion != 0);

    if (m_dwMajorVersion >= 6)
    {
        HMODULE hKrnlBase = ::GetModuleHandle(_T("kernel32.dll"));
        if (hKrnlBase)
        {
            m_pfnSetThreadUILanguage = (TFNSetThreadUILanguage)(::GetProcAddress(hKrnlBase, "SetThreadUILanguage"));
        }
        ATLASSERT(m_pfnSetThreadUILanguage != 0);
    }
}

CLanguageSetter::operator bool() const
{
    return m_bHasLists;
}

LANGID CLanguageSetter::set(LANGID langID /*= ::GetUserDefaultLangID()*/)
{
    m_pickedLangID = matchResourceLang_(LANGIDFROMLCID(langID));
    if (m_pfnSetThreadUILanguage) // Vista and newer
    {
        ATLVERIFY(m_pickedLangID == m_pfnSetThreadUILanguage(m_pickedLangID));
        ATLTRACE2(_T("Using method for Vista and newer with SetThreadUILanguage(%04X)\n"), m_pickedLangID);
    }
    return m_pickedLangID;
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
                ATLTRACE2(_T("Returning first match based on primary language identifier %04X (primary lang: %04X).\n"),
                          m_langIDs[i],
                          tomatch);
                return m_langIDs[i];
            }
        }
    }
    return m_fallback; // our fallback LANGID
}

BOOL CALLBACK CLanguageSetter::NtObjectsEnumResLangProc_(
    HMODULE /*hModule*/, LPCWSTR /*lpType*/, LPCWSTR /*lpName*/, WORD wLanguage, LONG_PTR lParam)
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
        RESLANGIDLIST resLangIdList = {RES_LANGID_LIST_LEN, 0, list};
        memset(list, 0, sizeof(LANGID) * RES_LANGID_LIST_LEN);

        // We use the about box resource as our sentinel which languages are available
        retval = FALSE != ::EnumResourceLanguages(
                              hInstance, lpType, lpName, NtObjectsEnumResLangProc_, (LONG_PTR)(&resLangIdList));

        if (retval)
        {
            for (size_t i = 0; i < resLangIdList.index; i++)
            {
                ATLASSERT(i < resLangIdList.size);
                m_langIDs.Push(resLangIdList.list[i]);
                ATLTRACE2(_T("Found resource language #%i %04X (%u)\n"),
                          (int)i + 1,
                          resLangIdList.list[i],
                          resLangIdList.list[i]);
            }
        }
    }
    return retval;
}

LANGID CLanguageSetter::picked() const
{
    return m_pickedLangID;
}
