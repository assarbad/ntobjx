///////////////////////////////////////////////////////////////////////////////
///
/// Little class which reads the version info from a loaded PE file.
///
/// Licensed under the MIT license (see below).
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

#ifndef __VERSIONINFO_H_VER__
#define __VERSIONINFO_H_VER__ 2016102018
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif // Check for "#pragma once" support

#include <Windows.h>
#ifndef _ATL_USE_CSTRING
#   define _ATL_USE_CSTRING
#endif // _ATL_USE_CSTRING
#ifndef _WTL_NO_CSTRING
#   define _WTL_NO_CSTRING
#endif // _WTL_NO_CSTRING
#include <atlstr.h>

class CVersionInfo
{
    LPVOID m_lpVerInfo;
    VS_FIXEDFILEINFO* m_pFixedFileInfo;
    DWORD m_useTranslation;
public:
    CVersionInfo(HINSTANCE hInstance)
        : m_lpVerInfo(NULL)
        , m_pFixedFileInfo(NULL)
        , m_useTranslation(0)
    {
        HRSRC hVersionResource = ::FindResource(hInstance, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
        if (NULL != hVersionResource)
        {
            if (DWORD dwSize = ::SizeofResource(hInstance, hVersionResource))
            {
                if (HGLOBAL hVersionResourceData = ::LoadResource(hInstance, hVersionResource))
                {
                    if (LPVOID pVerInfoRO = ::LockResource(hVersionResourceData))
                    {
                        if (NULL != (m_lpVerInfo = ::LocalAlloc(LMEM_FIXED, dwSize)))
                        {
                            ::CopyMemory(m_lpVerInfo, pVerInfoRO, dwSize);
                            UINT uLen;
                            if (::VerQueryValue(m_lpVerInfo, _T("\\"), (LPVOID*)&m_pFixedFileInfo, &uLen))
                            {
                                ATLTRACE2(_T("%u.%u\n"), HIWORD(m_pFixedFileInfo->dwFileVersionMS), LOWORD(m_pFixedFileInfo->dwFileVersionMS));
                                DWORD* translations;
                                if (::VerQueryValue(m_lpVerInfo, _T("\\VarFileInfo\\Translation"), (LPVOID*)&translations, &uLen))
                                {
                                    size_t const numTranslations = uLen / sizeof(DWORD);
                                    ATLTRACE2(_T("Number of translations: %u\n"), (UINT)numTranslations);
                                    for (size_t i = 0; i < numTranslations; i++)
                                    {
                                        ATLTRACE2(_T("Translation %u: %08X\n"), (UINT)i, translations[i]);
                                        switch (LOWORD(translations[i]))
                                        {
                                        case MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL): // fall through
                                        case MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US):
                                            if (1200 == HIWORD(translations[i])) // only Unicode entries
                                            {
                                                m_useTranslation = translations[i];
                                                return;
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                m_pFixedFileInfo = NULL;
                            }
                        }
                    }
                }
            }
        }
    }

    virtual ~CVersionInfo()
    {
        ::LocalFree(m_lpVerInfo);
    }

    LPCTSTR operator[](LPCTSTR lpszKey) const
    {
        ATL::CString fullName;
        fullName.Format(_T("\\StringFileInfo\\%04X%04X\\%s"), LOWORD(m_useTranslation), HIWORD(m_useTranslation), lpszKey);
        ATLTRACE2(_T("Full name: %s\n"), fullName.GetString());
        UINT uLen = 0;
        LPTSTR lpszBuf = NULL;
        if (::VerQueryValue(m_lpVerInfo, fullName, (LPVOID*)&lpszBuf, &uLen))
        {
            ATLTRACE2(_T("Value: %s\n"), lpszBuf);
            return lpszBuf;
        }
        ATLTRACE2(_T("Value: NULL\n"));
        return NULL;
    }
};

#endif // __VERSIONINFO_H_VER__