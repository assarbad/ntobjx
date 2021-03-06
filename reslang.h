///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///
/// Fixups for the messed up implementation of switching languages in Windows
/// versions prior to Windows Vista.
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
#ifndef __RESLANG_H_VER__
#define __RESLANG_H_VER__ 2016120400
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#    pragma once
#endif // Check for "#pragma once" support

#pragma warning(push)           /* disable code analyzer warnings for ATL & WTL libraries */
#pragma warning(disable : 6387) /* warning C6387 : '...' could be '0' : this does not adhere to the specification for \
                                   the function '...'. */
#pragma warning(disable : 6001) /* warning C6001: Using uninitialized memory '...'. */
#pragma warning(disable : 6011) /* warning C6011: Dereferencing NULL pointer '...'.  */
#include <atlctrls.h>
#include <atlctrlw.h>
#pragma warning(pop) /* restore code analyzer warnings*/
#include "resource.h"

EXTERN_C BOOL HookLdrFindResource_U(HINSTANCE resmodule);

// This class ensures on Windows XP and earlier that the SetThreadLocale()
// function will get the exact LANGID of available resources matching a
// given primary LANGID. This way we work around the quirky decision making
// process regarding resource selection in older versions of Windows
// (pre-Vista).
class CLanguageSetter
{
    typedef LANGID(WINAPI* TFNSetThreadUILanguage)(LANGID LangId);

    DWORD m_dwMajorVersion;
    TFNSetThreadUILanguage m_pfnSetThreadUILanguage;
    WTL::CSimpleStack<LANGID> m_langIDs;
    LANGID m_fallback;
    LANGID m_pickedLangID;
    bool const m_bHasLists;

  public:
    CLanguageSetter(LANGID fallback = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                    HMODULE hInstance = ModuleHelper::GetResourceInstance(),
                    LPCTSTR lpType = RT_DIALOG,
                    LPCTSTR lpName = MAKEINTRESOURCE(IDD_ABOUT));
    bool operator!() const;
    operator bool() const;
    LANGID set(LANGID langID = ::GetUserDefaultLangID());
    LANGID picked() const;

  private:
#define RES_LANGID_LIST_LEN 0x10
    typedef struct _RESLANGIDLIST
    {
        size_t size;
        size_t index;
        LANGID* list;
    } RESLANGIDLIST;

    LANGID matchResourceLang_(LANGID localeID) const;
    static BOOL CALLBACK NtObjectsEnumResLangProc_(HMODULE /*hModule*/, LPCWSTR /*lpType*/, LPCWSTR /*lpName*/, WORD wLanguage, LONG_PTR lParam);
    bool EnumAboutBoxLanguages_(HMODULE hInstance, LPCTSTR lpType, LPCTSTR lpName);
};

extern CLanguageSetter gLangSetter;

#endif // __RESLANG_H_VER__
