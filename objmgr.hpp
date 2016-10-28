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
#define __OBJMGR_HPP_VER__ 2016102800
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif // Check for "#pragma once" support

#include "ntnative.h"
#include <atlstr.h>
#include <atlcoll.h>
#include "objtypes.h"
#ifdef _DEBUG // for textual NTSTATUS values in trace messages
#   include "util/SimpleBuffer.h"
#   ifndef VTRACE
#       define VTRACE(...) do {} while(false)
#   endif
#   define CLL_NO_ENSURE_VERSION_CLASS
#   include "util/LoadLibrary.h"
#   include "util/VersionInfo.h"
#   define TRACE_NTSTATUS(x) \
        do \
        { \
            CLoadLibrary ntdll(_T("ntdll.dll")); \
            CSimpleBuf<TCHAR> status(ntdll.formatMessage<TCHAR>(ntdll.getHandle(), static_cast<DWORD>(x))); \
            ATLTRACE2(_T("  NTSTATUS was '%s'\n"), status.Buffer()); \
        } while (0)
#else
#   define TRACE_NTSTATUS(x) do {} while (0)
#endif // _DEBUG

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

        typedef int (__cdecl* comparefunc_t)(void* /*context*/, const void* /*elem1*/, const void* /*elem2*/);
        static int __cdecl compareItem_(void* /*context*/, Inherited*& obj1, Inherited*& obj2)
        {
            ATLASSERT(NULL != obj1);
            ATLASSERT(NULL != obj2);
            if(obj1 && obj2)
            {
                return _tcsicmp(obj1->name().GetString(), obj2->name().GetString());
            }
            return 0;
        }

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
                        qsort_s(tmplist.GetData(), tmplist.GetCount(), sizeof(EntryList::INARGTYPE), (comparefunc_t)compareItem_, NULL);
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

    template<typename T> class ObjectHandleT
    {
    public:
        struct CEventBasicInformation : public EVENT_BASIC_INFORMATION
        {
            typedef EVENT_BASIC_INFORMATION baseClass;
        private:
            CEventBasicInformation();
            CEventBasicInformation& operator=(CEventBasicInformation&);
            NTSTATUS m_queryStatus;
        public:
            CEventBasicInformation(HANDLE hObject)
            {
                ULONG uRetLen = 0;
                m_queryStatus = ::NtQueryEvent(hObject, EventBasicInformation, static_cast<baseClass*>(this), sizeof(baseClass), &uRetLen);
                ATLTRACE2(_T("  status = %08X; length = %u; this = %p\n"), m_queryStatus, uRetLen, this);
                TRACE_NTSTATUS(m_queryStatus);
            }

            inline NTSTATUS getQueryStatus() const
            {
                return m_queryStatus;
            }

            operator baseClass const*() const
            {
                return NT_SUCCESS(m_queryStatus) ? this : 0;
            }
        };

        struct CIoCompletionBasicInformation : public IO_COMPLETION_BASIC_INFORMATION
        {
            typedef IO_COMPLETION_BASIC_INFORMATION baseClass;
        private:
            CIoCompletionBasicInformation();
            CIoCompletionBasicInformation& operator=(CIoCompletionBasicInformation&);
            NTSTATUS m_queryStatus;
        public:
            CIoCompletionBasicInformation(HANDLE hObject)
            {
                ULONG uRetLen = 0;
                m_queryStatus = ::NtQueryIoCompletion(hObject, IoCompletionBasicInformation, static_cast<baseClass*>(this), sizeof(baseClass), &uRetLen);
                ATLTRACE2(_T("  status = %08X; length = %u; this = %p\n"), m_queryStatus, uRetLen, this);
                TRACE_NTSTATUS(m_queryStatus);
            }

            inline NTSTATUS getQueryStatus() const
            {
                return m_queryStatus;
            }

            inline operator bool() const
            {
                return !this->operator!();
            }

            inline bool operator!() const
            {
                return NT_ERROR(m_queryStatus);
            }
        };

        struct KEY_BASIC_INFORMATION_WITH_NAMEBUF : public KEY_BASIC_INFORMATION
        {
            WCHAR NameBufferExtended[MAX_PATH * 2];
        };

        struct CKeyBasicInformation : public KEY_BASIC_INFORMATION_WITH_NAMEBUF
        {
            typedef KEY_BASIC_INFORMATION_WITH_NAMEBUF baseClass;
        private:
            CKeyBasicInformation();
            CKeyBasicInformation& operator=(CKeyBasicInformation&);
            NTSTATUS m_queryStatus;
        public:
            CKeyBasicInformation(HANDLE hObject)
            {
                ULONG uRetLen = 0;
                m_queryStatus = ::NtQueryKey(hObject, KeyBasicInformation, static_cast<baseClass*>(this), sizeof(baseClass), &uRetLen);
                ATLTRACE2(_T("  status = %08X; length = %u; this = %p\n"), m_queryStatus, uRetLen, this);
                TRACE_NTSTATUS(m_queryStatus);
            }

            inline NTSTATUS getQueryStatus() const
            {
                return m_queryStatus;
            }

            inline operator bool() const
            {
                return !this->operator!();
            }

            inline bool operator!() const
            {
                return NT_ERROR(m_queryStatus);
            }
        };

        struct CMutantBasicInformation : public MUTANT_BASIC_INFORMATION
        {
            typedef MUTANT_BASIC_INFORMATION baseClass;
        private:
            CMutantBasicInformation();
            CMutantBasicInformation& operator=(CMutantBasicInformation&);
            NTSTATUS m_queryStatus;
        public:
            CMutantBasicInformation(HANDLE hObject)
            {
                ULONG uRetLen = 0;
                m_queryStatus = ::NtQueryMutant(hObject, MutantBasicInformation, static_cast<baseClass*>(this), sizeof(baseClass), &uRetLen);
                ATLTRACE2(_T("  status = %08X; length = %u; this = %p\n"), m_queryStatus, uRetLen, this);
                TRACE_NTSTATUS(m_queryStatus);
            }

            inline NTSTATUS getQueryStatus() const
            {
                return m_queryStatus;
            }

            inline operator bool() const
            {
                return !this->operator!();
            }

            inline bool operator!() const
            {
                return NT_ERROR(m_queryStatus);
            }
        };

        struct CSectionBasicInformation : public SECTION_BASIC_INFORMATION
        {
            typedef SECTION_BASIC_INFORMATION baseClass;
        private:
            CSectionBasicInformation();
            CSectionBasicInformation& operator=(CSectionBasicInformation&);
            NTSTATUS m_queryStatus;
        public:
            CSectionBasicInformation(HANDLE hObject)
            {
                ULONG uRetLen = 0;
                m_queryStatus = ::NtQuerySection(hObject, SectionBasicInformation, static_cast<baseClass*>(this), sizeof(baseClass), &uRetLen);
                ATLTRACE2(_T("  status = %08X; length = %u; this = %p\n"), m_queryStatus, uRetLen, this);
                TRACE_NTSTATUS(m_queryStatus);
            }

            inline NTSTATUS getQueryStatus() const
            {
                return m_queryStatus;
            }

            inline operator bool() const
            {
                return !this->operator!();
            }

            inline bool operator!() const
            {
                return NT_ERROR(m_queryStatus);
            }
        };

        struct CSemaphoreBasicInformation : public SEMAPHORE_BASIC_INFORMATION
        {
            typedef SEMAPHORE_BASIC_INFORMATION baseClass;
        private:
            CSemaphoreBasicInformation();
            CSemaphoreBasicInformation& operator=(CSemaphoreBasicInformation&);
            NTSTATUS m_queryStatus;
        public:
            CSemaphoreBasicInformation(HANDLE hObject)
            {
                ULONG uRetLen = 0;
                m_queryStatus = ::NtQuerySemaphore(hObject, SemaphoreBasicInformation, static_cast<baseClass*>(this), sizeof(baseClass), &uRetLen);
                ATLTRACE2(_T("  status = %08X; length = %u; this = %p\n"), m_queryStatus, uRetLen, this);
                TRACE_NTSTATUS(m_queryStatus);
            }

            inline NTSTATUS getQueryStatus() const
            {
                return m_queryStatus;
            }

            inline operator bool() const
            {
                return !this->operator!();
            }

            inline bool operator!() const
            {
                return NT_ERROR(m_queryStatus);
            }
        };

        struct CTimerBasicInformation : public TIMER_BASIC_INFORMATION
        {
            typedef TIMER_BASIC_INFORMATION baseClass;
        private:
            CTimerBasicInformation();
            CTimerBasicInformation& operator=(CTimerBasicInformation&);
            NTSTATUS m_queryStatus;
        public:
            CTimerBasicInformation(HANDLE hObject)
            {
                ULONG uRetLen = 0;
                m_queryStatus = ::NtQueryTimer(hObject, TimerBasicInformation, static_cast<baseClass*>(this), sizeof(baseClass), &uRetLen);
                ATLTRACE2(_T("  status = %08X; length = %u; this = %p\n"), m_queryStatus, uRetLen, this);
                TRACE_NTSTATUS(m_queryStatus);
            }

            inline NTSTATUS getQueryStatus() const
            {
                return m_queryStatus;
            }

            inline operator bool() const
            {
                return !this->operator!();
            }

            inline bool operator!() const
            {
                return NT_ERROR(m_queryStatus);
            }
        };
    private:
        typedef NTSTATUS(NTAPI *openobj_fct_t)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);

        NTSTATUS m_openStatus;
        NTSTATUS m_queryStatus;
        bool m_bHasObjectInfo;
        GenericObjectT<T>* m_obj;
        openobj_fct_t m_openObjectFunction; // cached for reopen
        OBJECT_BASIC_INFORMATION m_obi;
        HANDLE m_hObject;
        CEventBasicInformation* m_eventBasicInfo;
        CIoCompletionBasicInformation* m_ioCompletionBasicInfo;
        CKeyBasicInformation* m_keyBasicInfo;
        CMutantBasicInformation* m_mutantBasicInfo;
        CSectionBasicInformation* m_sectionBasicInfo;
        CSemaphoreBasicInformation* m_semaphoreBasicInfo;
        CTimerBasicInformation* m_timerBasicInfo;
    public:
        ObjectHandleT(GenericObjectT<T>* obj, ACCESS_MASK DesiredAccess = READ_CONTROL | GENERIC_READ, ACCESS_MASK FallbackAccess1 = READ_CONTROL | FILE_READ_ATTRIBUTES | FILE_READ_ACCESS, ACCESS_MASK FallbackAccess2 = READ_CONTROL)
            : m_openStatus(STATUS_SUCCESS)
            , m_queryStatus(STATUS_SUCCESS)
            , m_bHasObjectInfo(false)
            , m_obj(obj)
            , m_openObjectFunction(m_obj ? PickOpenObjectFunction_(m_obj->type().GetString()) : NULL)
            , m_hObject(OpenByObjectType_(m_obj, m_openObjectFunction, m_openStatus, m_queryStatus, m_bHasObjectInfo, m_obi , DesiredAccess, FallbackAccess1, FallbackAccess2))
            , m_eventBasicInfo(0)
            , m_ioCompletionBasicInfo(0)
            , m_keyBasicInfo(0)
            , m_mutantBasicInfo(0)
            , m_sectionBasicInfo(0)
            , m_semaphoreBasicInfo(0)
            , m_timerBasicInfo(0)
        {
            if(!m_obj || (INVALID_HANDLE_VALUE == m_hObject))
            {
                ATLTRACE2(_T("Bailing out early m_ob == %p; m_hObject == %p\n"), m_obj, m_hObject);
                return;
            }
            
            if (0 == _tcsicmp(_T(OBJTYPESTR_EVENT), m_obj->type()))
            {
                ATLTRACE2(_T("Attempting to query object-specific info for %s\n"), m_obj->type().GetString());
                m_eventBasicInfo = new CEventBasicInformation(m_hObject);
            }

            if (0 == _tcsicmp(_T(OBJTYPESTR_IOCOMPLETION), m_obj->type()))
            {
                ATLTRACE2(_T("Attempting to query object-specific info for %s\n"), m_obj->type().GetString());
                m_ioCompletionBasicInfo = new CIoCompletionBasicInformation(m_hObject);
            }

            if (0 == _tcsicmp(_T(OBJTYPESTR_KEY), m_obj->type()))
            {
                ATLTRACE2(_T("Attempting to query object-specific info for %s\n"), m_obj->type().GetString());
                m_keyBasicInfo = new CKeyBasicInformation(m_hObject);
            }

            if (0 == _tcsicmp(_T(OBJTYPESTR_MUTANT), m_obj->type()))
            {
                ATLTRACE2(_T("Attempting to query object-specific info for %s\n"), m_obj->type().GetString());
                m_mutantBasicInfo = new CMutantBasicInformation(m_hObject);
            }

            if (0 == _tcsicmp(_T(OBJTYPESTR_SECTION), m_obj->type()))
            {
                ATLTRACE2(_T("Attempting to query object-specific info for %s\n"), m_obj->type().GetString());
                m_sectionBasicInfo = new CSectionBasicInformation(m_hObject);
            }

            if (0 == _tcsicmp(_T(OBJTYPESTR_SEMAPHORE), m_obj->type()))
            {
                ATLTRACE2(_T("Attempting to query object-specific info for %s\n"), m_obj->type().GetString());
                m_semaphoreBasicInfo = new CSemaphoreBasicInformation(m_hObject);
            }

            if (0 == _tcsicmp(_T(OBJTYPESTR_TIMER), m_obj->type()))
            {
                ATLTRACE2(_T("Attempting to query object-specific info for %s\n"), m_obj->type().GetString());
                m_timerBasicInfo = new CTimerBasicInformation(m_hObject);
            }
        }

        virtual ~ObjectHandleT()
        {
            if(INVALID_HANDLE_VALUE != m_hObject)
            {
                ::NtClose(m_hObject);
            }
        }

        inline HANDLE getHandle()
        {
            return m_hObject;
        }

        inline NTSTATUS getOpenStatus() const
        {
            return m_openStatus;
        }

        inline NTSTATUS getQueryStatus() const
        {
            return m_queryStatus;
        }

        inline bool hasObjectInfo() const
        {
            return m_bHasObjectInfo;
        }

        inline OBJECT_BASIC_INFORMATION const& getObjectInfo() const
        {
            return m_obi;
        }

        inline operator bool() const
        {
            return !this->operator!();
        }

        inline bool operator!() const
        {
            return (m_hObject == INVALID_HANDLE_VALUE) || (m_hObject == NULL);
        }

        // object-specific basic information
        inline CEventBasicInformation const* getEventBasicInfo() const { return m_eventBasicInfo; };
        inline CIoCompletionBasicInformation const* getIoCompletionBasicInfo() const { return m_ioCompletionBasicInfo; };
        inline CKeyBasicInformation const* getKeyBasicInfo() const { return m_keyBasicInfo;  };
        inline CMutantBasicInformation const* getMutantBasicInfo() const { return m_mutantBasicInfo;  };
        inline CSectionBasicInformation const* getSectionBasicInfo() const { return m_sectionBasicInfo; };
        inline CSemaphoreBasicInformation const* getSemaphoreBasicInfo() const { return m_semaphoreBasicInfo;  };
        inline CTimerBasicInformation const* getTimerBasicInfo() const { return m_timerBasicInfo;  };

    private:
        static NTSTATUS NTAPI OpenObjectAsFile_(__out PHANDLE Handle, __in ACCESS_MASK DesiredAccess, __in POBJECT_ATTRIBUTES ObjectAttributes)
        {
            IO_STATUS_BLOCK iostat;
            return NtOpenFile(Handle, DesiredAccess, ObjectAttributes, &iostat, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0);
        }

        static openobj_fct_t PickOpenObjectFunction_(LPCTSTR lpszTypeName)
        {
            if (!lpszTypeName)
            {
                return NULL;
            }

            static struct { LPCTSTR tpname; openobj_fct_t openFct; } openFunctions[] =
            {
                { _T(OBJTYPESTR_DIRECTORY), NtOpenDirectoryObject },
                { _T(OBJTYPESTR_EVENT), NtOpenEvent },
                { _T(OBJTYPESTR_EVENTPAIR), NtOpenEventPair },
                { _T(OBJTYPESTR_IOCOMPLETION), NtOpenIoCompletion },
                { _T(OBJTYPESTR_KEY), NtOpenKey },
                { _T(OBJTYPESTR_MUTANT), NtOpenMutant },
                { _T(OBJTYPESTR_SECTION), NtOpenSection },
                { _T(OBJTYPESTR_SEMAPHORE), NtOpenSemaphore },
                { _T(OBJTYPESTR_SYMBOLICLINK), NtOpenSymbolicLinkObject },
                { _T(OBJTYPESTR_TIMER), NtOpenTimer },
            };
            openobj_fct_t OpenObjectFct = OpenObjectAsFile_; // default

            // Find appropriate object-specific function
            for (size_t i = 0; i < _countof(openFunctions); i++)
            {
                if (0 == _tcsnicmp(lpszTypeName, openFunctions[i].tpname, _tcslen(openFunctions[i].tpname)))
                {
                    ATLTRACE2(_T("Found open function %p for %s.\n"), openFunctions[i].openFct, lpszTypeName);
                    OpenObjectFct = openFunctions[i].openFct;
                    break;
                }
            }

            return OpenObjectFct;
        }

        static HANDLE OpenByObjectType_(GenericObjectT<T>* obj, openobj_fct_t OpenObjectFunc, NTSTATUS& openStatus, NTSTATUS& queryStatus, bool& hasObjInfo, OBJECT_BASIC_INFORMATION& obi, ACCESS_MASK DesiredAccess = READ_CONTROL | GENERIC_READ, ACCESS_MASK FallbackAccess1 = READ_CONTROL | FILE_READ_ATTRIBUTES | FILE_READ_ACCESS, ACCESS_MASK FallbackAccess2 = READ_CONTROL)
        {
            memset(&obi, 0, sizeof(obi));

            if(!obj)
            {
                ATLTRACE2(_T("Passed GenericObjectT<T>* was NULL.\n"));
                return INVALID_HANDLE_VALUE;
            }
            if(!OpenObjectFunc)
            {
                ATLTRACE2(_T("OpenObjectFunc was NULL.\n"));
                return INVALID_HANDLE_VALUE;
            }

            HANDLE hObject = INVALID_HANDLE_VALUE;
            OBJECT_ATTRIBUTES oa;
            UNICODE_STRING objname;
            LPCTSTR lpszFullName = obj->fullname().GetString();
            ATLASSERT(lpszFullName != NULL);

            ::RtlInitUnicodeString(&objname, lpszFullName);
            InitializeObjectAttributes(&oa, &objname, 0, NULL, NULL);

            LPCTSTR lpszTypeName = obj->type().GetString();
#ifndef _DEBUG
            UNREFERENCED_PARAMETER(lpszTypeName);
#endif // !_DEBUG

            ATLASSERT(lpszTypeName != NULL);
            ATLASSERT(OpenObjectFunc != NULL);

            ATLTRACE2(_T("Trying to open %s as type %s.\n"), lpszFullName, lpszTypeName);
            openStatus = OpenObjectFunc(&hObject, DesiredAccess, &oa);
            // Try with a lesser access mask
            if (STATUS_ACCESS_DENIED == openStatus)
            {
                ATLTRACE2(_T("Failed open with %08X, trying again with access mask %08X\n"), openStatus, FallbackAccess1);
                NTSTATUS privOpenStatus = OpenObjectFunc(&hObject, FallbackAccess1, &oa);
                // Try with an even lesser access mask
                if(STATUS_ACCESS_DENIED == privOpenStatus)
                {
                    ATLTRACE2(_T("Failed open with %08X, trying again with access mask %08X\n"), openStatus, FallbackAccess2);
                    privOpenStatus = OpenObjectFunc(&hObject, FallbackAccess2, &oa);
                }
                openStatus = privOpenStatus;
            }
            if (NT_SUCCESS(openStatus))
            {
                ATLTRACE2(_T("Success. Returning handle %p.\n"), hObject);
                ULONG retLen = 0;
                queryStatus = NtQueryObject(hObject, ObjectBasicInformation, &obi, static_cast<ULONG>(sizeof(obi)), &retLen);
                if (NT_SUCCESS(queryStatus))
                {
                    hasObjInfo = true;
                    ATLTRACE2(_T("NtQueryObject succeeded with status 0x%08X.\n"), queryStatus);
                }
                else
                {
                    ATLTRACE2(_T("NtQueryObject failed with status 0x%08X.\n"), queryStatus);
                    memset(&obi, 0, sizeof(obi));
                }
                // Perhaps also read object-type-specific info for the known ones?
                return hObject;
            }
            TRACE_NTSTATUS(openStatus);
            switch (openStatus)
            {
            case STATUS_ACCESS_DENIED:
                ATLTRACE2(_T("Access denied.\n"));
                break;
            case STATUS_OBJECT_TYPE_MISMATCH:
                ATLTRACE2(_T("Object type mismatch for this open method.\n"));
                break;
            default:
                ATLTRACE2(_T("Something went wrong: %08X. Returning INVALID_HANDLE_VALUE.\n"), openStatus);
                break;
            }
            ATLTRACE2(_T("Returning INVALID_HANDLE_VALUE for %s.\n"), lpszFullName);
            return INVALID_HANDLE_VALUE;
        }
    };

    typedef ObjectHandleT<ATL::CString> ObjectHandle;
} /* namespace */

#endif // __OBJMGR_HPP_VER__
