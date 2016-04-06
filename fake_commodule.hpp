///////////////////////////////////////////////////////////////////////////////
///
/// Allows to reduce the size of executables linking the WTL by providing our
/// own minimal implementation of the CComModule, without the COM server
/// registration capabilities.
///
/// Include as follows:
///   #define _ATL_NO_COMMODULE
///   #include <atlbase.h>
///   #include "fake_commodule.hpp"
///   #undef _ATL_NO_COMMODULE
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

#ifndef __FAKE_COMMODULE_HPP_VER__
#define __FAKE_COMMODULE_HPP_VER__ 2016032300
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif // Check for "#pragma once" support

namespace ATL
{
    class CComModule;
    __declspec(selectany) CComModule* _pModule = NULL;
    class CComModule : public CAtlModuleT<CComModule>
    {
    public:
        CComModule()
        {
            // Should have only one instance of a class 
            // derived from CComModule in a project.
            ATLASSERT(_pModule == NULL);
            _pModule = this;
        }

        HINSTANCE SetResourceInstance(HINSTANCE h) throw() { return _AtlBaseModule.SetResourceInstance(h); }
        HINSTANCE GetModuleInstance() throw() { return _AtlBaseModule.m_hInst; }
        HINSTANCE GetResourceInstance() throw() { return _AtlBaseModule.m_hInstResource; }
        HRESULT Init(_ATL_OBJMAP_ENTRY*, HINSTANCE, const GUID*) throw() { return S_OK; }
    };
}

#endif // __FAKE_COMMODULE_HPP_VER__
