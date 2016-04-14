///////////////////////////////////////////////////////////////////////////////
///
/// Helper classes to inspect the NT object manager namespace.
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

#ifndef __OBJMGR_HPP_VER__
#define __OBJMGR_HPP_VER__ 2016041420
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif // Check for "#pragma once" support

#include "ntnative.h"
#include <atlstr.h>
#include <atlcoll.h>

namespace NtObjMgr{

    typedef enum
    {
        otGeneric,
        otSymlink,
        otDirectory,
    } objtype_t;

    // Base interface from which everything derives
    template<typename T> interface IObjectT
    {
        virtual ~IObjectT() {}
        virtual T const& name() const = 0;
        virtual T const& fullname() const = 0;
        virtual T const& type() const = 0;
        virtual objtype_t objtype() const = 0;
    };

    template<typename T> interface ISymlinkT
    {
        virtual ~ISymlinkT() {}
        virtual T const& target() const = 0;
    };

    template<typename T> interface IDirectoryT
    {
        virtual ~IDirectoryT() {}
        virtual size_t size() const = 0;
    };

    template<typename T> class SymbolicLinkT;
    template<typename T> class DirectoryT;

    template<typename T> class GenericObjectT:
        public IObjectT<T>
    {
        typedef IObjectT<T> Inherited;

        inline void setParent_()
        {
            if((m_parent.GetLength() == 1) && (m_parent[0] == L'\\'))
            {
                m_parent.Delete(0, 1);
            }
            if((m_parent.GetLength() > 1) && (m_parent[0] != L'\\'))
            {
                m_parent.Insert(0, L"\\");
            }
        }

    public:
        GenericObjectT(UNICODE_STRING const& name_, UNICODE_STRING const& tpname, LPCWSTR parent = NULL)
            : m_name(name_.Buffer, name_.Length / sizeof(WCHAR))
            , m_type(tpname.Buffer, tpname.Length / sizeof(WCHAR))
            , m_parent((parent) ? parent : L"")
            , m_fullname()
        {
            setParent_();
            m_fullname = m_parent + L"\\" + m_name;
        }
        GenericObjectT(LPCWSTR name_, LPCWSTR tpname, LPCWSTR parent = NULL)
            : m_name(name_)
            , m_type(tpname)
            , m_parent((parent) ? parent : L"")
            , m_fullname()
        {
            setParent_();
            m_fullname = m_parent + L"\\" + m_name;
        }

        virtual ~GenericObjectT()
        {
        }

        inline T const& name() const
        {
            return m_name;
        }

        inline T const& fullname() const
        {
            return m_fullname;
        }

        inline T const& type() const
        {
            return m_type;
        }

        inline objtype_t objtype() const
        {
            return otGeneric;
        }

    protected:
        T m_name;
        T m_type;
        T m_parent;
        T m_fullname;

    private:
        /* hide copy ctor as well as assignment operator */
        GenericObjectT(GenericObjectT const& rhs) {}
        GenericObjectT& operator=(GenericObjectT const&) { }
    };
    typedef GenericObjectT<ATL::CString> GenericObject;

    template<typename T> class SymbolicLinkT :
        public GenericObjectT<T>,
        public ISymlinkT<T>
    {
        /*lint -save -e1516 */
        typedef GenericObjectT<T> Inherited;
        /*lint -restore */

    public:
        SymbolicLinkT(UNICODE_STRING const& name_, UNICODE_STRING const& tpname, LPCWSTR parent = NULL)
            : Inherited(name_, tpname, parent)
            , m_linktgt()
            , m_lastStatus(STATUS_SUCCESS)
            , m_cached(refresh())
        {
        }

        SymbolicLinkT(LPCWSTR name_, LPCWSTR tpname, LPCWSTR parent = NULL)
            : Inherited(name_, tpname, parent)
            , m_linktgt()
            , m_lastStatus(STATUS_SUCCESS)
            , m_cached(refresh())
        {
        }

        virtual ~SymbolicLinkT()
        {
        }

        inline T const& target() const
        {
            return m_linktgt;
        }

        inline objtype_t objtype() const
        {
            return otSymlink;
        }

        inline NTSTATUS getLastNtStatus() const
        {
            return m_lastStatus;
        }

        bool refresh()
        {
            HANDLE hLink = NULL;
            OBJECT_ATTRIBUTES oa;
            UNICODE_STRING linkname;
            ::RtlInitUnicodeString(&linkname, LPCWSTR(fullname()));
            InitializeObjectAttributes(&oa, &linkname, 0, NULL, NULL);
            m_lastStatus = ::NtOpenSymbolicLinkObject(&hLink, SYMBOLIC_LINK_QUERY, &oa);
            if(NT_SUCCESS(m_lastStatus))
            {
                size_t const bufSize = 0x7FFF;
                CTempBuffer<WCHAR> buf(bufSize);
                LPWSTR lpszBuf = LPWSTR(buf);
                UNICODE_STRING usLinkTarget = { 0, bufSize * sizeof(WCHAR), lpszBuf };
                ULONG len = 0;
                m_lastStatus = ::NtQuerySymbolicLinkObject(hLink, &usLinkTarget, &len);
                if(NT_SUCCESS(m_lastStatus))
                {
                    m_linktgt = T(usLinkTarget.Buffer, usLinkTarget.Length / sizeof(WCHAR));
                }
                m_lastStatus = ::NtClose(hLink);
            }

            return false;
        }
    private:

        /* hide copy ctor as well as assignment operator */
        SymbolicLinkT(SymbolicLinkT const& rhs) {}
        SymbolicLinkT& operator=(SymbolicLinkT const&) { }

        T m_linktgt;
        NTSTATUS m_lastStatus;
        bool m_cached;
    };
    typedef SymbolicLinkT<ATL::CString> SymbolicLink;

    template<typename T> class DirectoryT :
        public GenericObjectT<T>,
        public IDirectoryT<T>
    {
        /*lint -save -e1516 */
        typedef GenericObjectT<T> Inherited;
        /*lint -restore */
        typedef ATL::CAtlArray<Inherited*, ATL::CPrimitiveElementTraits<Inherited*> > EntryList;
    public:
        DirectoryT(LPCWSTR objdir = L"")
            : Inherited(objdir, L"Directory")
            , m_entries()
            , m_lastStatus(STATUS_SUCCESS)
            , m_cached(refresh())
        {
        }

        DirectoryT(UNICODE_STRING const& name_, UNICODE_STRING const& tpname, LPCWSTR parent = NULL)
            : Inherited(name_, tpname, parent)
            , m_entries()
            , m_lastStatus(STATUS_SUCCESS)
            , m_cached(refresh())
        {
        }

        DirectoryT(LPCWSTR name_, LPCWSTR tpname, LPCWSTR parent = NULL)
            : Inherited(name_, tpname, parent)
            , m_entries()
            , m_lastStatus(STATUS_SUCCESS)
            , m_cached(refresh())
        {
        }

        virtual ~DirectoryT()
        {
            size_t const listsize = m_entries.GetCount();
            for(size_t i = 0; i < listsize; i++)
            {
                try
                {
                    delete m_entries[i];
                }
                catch (...)
                {
                    ATLTRACE2(_T("The delete operator barfed when cleaning up cached entries.\n"));
                }
            }
            m_entries.RemoveAll();
        }

        inline NTSTATUS getLastNtStatus() const
        {
            return m_lastStatus;
        }

        inline objtype_t objtype() const
        {
            return otDirectory;
        }

        bool refresh()
        {
            HANDLE hObjRoot = NULL;
            OBJECT_ATTRIBUTES oa;
            UNICODE_STRING objname;
            ::RtlInitUnicodeString(&objname, LPCWSTR(fullname())); // use m_root.c_str() with STL
            InitializeObjectAttributes(&oa, &objname, 0, NULL, NULL);
            m_lastStatus = ::NtOpenDirectoryObject(&hObjRoot, DIRECTORY_QUERY | DIRECTORY_TRAVERSE, &oa);
            if(NT_SUCCESS(m_lastStatus))
            {
                EntryList tmplist;
                size_t const bufSize = 0x10000;
                CTempBuffer<BYTE> buf(bufSize);
                ULONG start = 0, idx = 0, bytes;
                BOOLEAN restart = TRUE;
                for(;;)
                {
                    m_lastStatus = ::NtQueryDirectoryObject(hObjRoot, PBYTE(buf), bufSize, FALSE, restart, &idx, &bytes);
                    if(NT_SUCCESS(m_lastStatus))
                    {
                        POBJECT_DIRECTORY_INFORMATION const pdilist = reinterpret_cast<POBJECT_DIRECTORY_INFORMATION>(PBYTE(buf));
                        for(ULONG i = 0; i < idx - start; i++)
                        {
                            if(0 == wcsncmp(pdilist[i].TypeName.Buffer, L"Directory", pdilist[i].TypeName.Length / sizeof(WCHAR)))
                            {
                                tmplist.Add(new DirectoryT<T>(pdilist[i].Name, pdilist[i].TypeName, fullname()));
                            }
                            else if(0 == wcsncmp(pdilist[i].TypeName.Buffer, L"SymbolicLink", pdilist[i].TypeName.Length / sizeof(WCHAR)))
                            {
                                tmplist.Add(new SymbolicLinkT<T>(pdilist[i].Name, pdilist[i].TypeName, fullname()));
                            }
                            else
                            {
                                tmplist.Add(new Inherited(pdilist[i].Name, pdilist[i].TypeName, fullname()));
                            }
                        }
                    }
#ifdef _DEBUG
                    else
                    {
                        if(STATUS_NO_MORE_ENTRIES != m_lastStatus)
                            ATLTRACE2(_T("NtQueryDirectoryObject returned: %08X\n"), m_lastStatus);
                    }
#endif
                    if(STATUS_MORE_ENTRIES == m_lastStatus)
                    {
                        start = idx;
                        restart = FALSE;
                        continue;
                    }
                    if((STATUS_SUCCESS == m_lastStatus) || (STATUS_NO_MORE_ENTRIES == m_lastStatus)) // last items?
                    {
                        break;
                    }
                }
                switch(m_lastStatus)
                {
                case STATUS_SUCCESS:
                case STATUS_NO_MORE_ENTRIES:
                    {
                        EntryList deletions;
                        deletions.Copy(m_entries); // take copy of old list
                        size_t const todelete = deletions.GetCount();
                        m_entries.Copy(tmplist); // overwrite with new list
                        m_cached = true;
                        // Delete objects from old list
                        for(size_t i = 0; i < todelete; i++)
                        {
                            delete deletions[i];
                        }
                        m_lastStatus = ::NtClose(hObjRoot);
                    }
                    return true;
                default:
                    m_lastStatus = ::NtClose(hObjRoot);
                    break;
                }
            }
            return false;
        }

        inline size_t size() const
        {
            return m_entries.GetCount();
        }

        inline Inherited*& operator[](size_t idx)
        {
            return m_entries.GetAt(idx);
        }

    private:
        /* hide copy ctor as well as assignment operator */
        DirectoryT(DirectoryT const& rhs) {}
        DirectoryT& operator=(DirectoryT const&) { }

        EntryList m_entries; // remember, class members get initialized in order of declaration inside ctor!!!
        NTSTATUS m_lastStatus;
        bool m_cached;
    };

    typedef DirectoryT<ATL::CString> Directory;
} /* namespace */

#endif // __OBJMGR_HPP_VER__
