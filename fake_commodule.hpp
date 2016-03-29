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
