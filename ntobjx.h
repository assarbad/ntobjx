///////////////////////////////////////////////////////////////////////////////
///
/// The GUI controls and main frame window classes for ntobjx.
///
/// Dual-licensed under MS-PL and MIT license (see below).
///
///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2016-2018, 2022 Oliver Schneider (assarbad.net)
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

#pragma once

#if (_MSVC_LANG < 201103L) && !defined(nullptr)
#    define nullptr 0 //-V1059
#endif

#ifndef __ATLAPP_H__
#    error ntobjx.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
#    error ntobjx.h requires atlwin.h to be included first
#endif

#ifndef __ATLCTRLX_H__
#    error ntobjx.h requires atlctrlx.h to be included first
#endif

#ifndef __ATLCTRLW_H__
#    error ntobjx.h requires atlctrlw.h to be included first
#endif

#ifndef __ATLSPLIT_H__
#    error ntobjx.h requires atlsplit.h to be included first
#endif

#ifndef __ATLFRAME_H__
#    error ntobjx.h requires atlframe.h to be included first
#endif

#ifndef __ATLDLGS_H__
#    error ntobjx.h requires atldlgs.h to be included first
#endif

#ifndef __ATLCTRLS_H__
#    error ntobjx.h requires atlctrls.h to be included first
#endif

#ifndef __ATLSECURITY_H__
#    error ntobjx.h requires atlsecurity.h to be included first
#endif

#ifndef __ATLUSER_H__
#    error ntobjx.h requires atluser.h to be included first
#endif

#ifndef __func__
#    define __func__ __FUNCTION__ //-V1059
#endif

#include "util/SimpleBuffer.h"
#ifndef VTRACE
#    define VTRACE(...) \
        while (false)   \
        {               \
        }
#endif
#define CLL_NO_ENSURE_VERSION_CLASS
#include "util/LoadLibrary.h"
#include "util/VersionInfo.h"
#ifndef NTOBJX_NO_XML_EXPORT
#    pragma warning(push) /* disable code analyzer warnings pugixml library */
#    pragma warning(disable : 4995)
#    pragma warning(disable : 6054)
#    pragma warning(disable : 6384)   /* warning C6384: Dividing sizeof a pointer by another value. */
#    pragma warning(suppress : 26495) /* static analyzer suppressions, this is 3rd party code to me */
#    include "pugixml.hpp"
#    pragma warning(pop)
#endif

#define FEATURE_FIND_OBJECT     1
#define FEATURE_OBJECT_SECURITY 1

using ATL::CAccessToken;
using ATL::CAtlMap;
using ATL::CString;
using NtObjMgr::Directory;
using NtObjMgr::GenericObject;
using NtObjMgr::ObjectHandle;
using NtObjMgr::objtype_t;
using NtObjMgr::otDirectory;
using NtObjMgr::otGeneric;
using NtObjMgr::otSymlink;
using NtObjMgr::SymbolicLink;
using WTL::CFileDialog;

#define WM_VISIT_DIRECTORY           (WM_USER + 100)
#define WM_SET_ACTIVE_OBJECT         (WM_USER + 101)
#define WM_SELECT_TREEVIEW_DIRECTORY (WM_USER + 102)
#define WM_DIRECTORY_UP              (WM_USER + 103)

#ifndef LVS_EX_DOUBLEBUFFER
#    define LVS_EX_DOUBLEBUFFER 0x00010000
#endif
#ifndef TVS_EX_DOUBLEBUFFER
#    define TVS_EX_DOUBLEBUFFER 0x0004
#endif

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

namespace //-V1068
{
    static const struct
    {
        UINT resId;
    } lvColumnDefaults[] = {
        {ID_COLNAME_OBJNAME},
        {ID_COLNAME_OBJTYPE},
        {ID_COLNAME_OBJLNKTGT},
    };

#ifdef _DEBUG
    int const maxVisitedDepth = 5; // this is more practical in debug builds
#else
    int const maxVisitedDepth = 0x400;
#endif // _DEBUG
    int const imgListElemWidth = 16;
    int const imgListElemHeight = 16;

    CString getformatCreationTimeString(LARGE_INTEGER const li)
    {
        CString str;
        if (li.QuadPart > 0)
        {
            ATLASSERT(li.HighPart >= 0); // anything else will fail our conversion below
            FILETIME ft = {li.LowPart, static_cast<DWORD>(li.HighPart)};
            SYSTEMTIME st;
            ATLVERIFY(FileTimeToSystemTime(&ft, &st));
            // DO NOT LOCALIZE THIS!
            str.Format(_T("%04d-%02d-%02d %02d:%02d:%02d.%03d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        }
        else
        {
#pragma warning(suppress : 6031)
            ATLVERIFY(str.LoadString(IDS_NOT_AVAILABLE_SHORT));
        }
        return str;
    }

    // Requires open clipboard and doesn't close it
    BOOL setClipboardString(LPCTSTR lpsz)
    {
        CString s(lpsz);
        SIZE_T allocLength = ((SIZE_T)s.GetLength() + 2U) * sizeof(TCHAR);
        ATLTRACE2(_T("Buffer size: %u\n"), allocLength);
        HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, allocLength);
        if (hMem)
        {
            if (LPVOID lpMem = ::GlobalLock(hMem))
            {
                LPTSTR lpStr = static_cast<LPTSTR>(lpMem);
                _tcscpy_s(lpStr, s.GetLength() + (size_t)1, s.GetString());
                ::GlobalUnlock(hMem);
            }
#if defined(_UNICODE) || defined(UNICODE)
            return (nullptr != ::SetClipboardData(CF_UNICODETEXT, hMem));
#else
#    error "Not implemented for ANSI/CF_TEXT"
#endif
        }
        else
        {
            ATLTRACE2(_T("Could not allocate memory using GlobalAlloc().\n"));
        }
        return FALSE;
    }

    CSimpleBuf<TCHAR> getStatusString(LONG status);

    CString getObjectDetailsString(GenericObject* obj, ObjectHandle* objHdl)
    {
        ATLASSERT(obj != nullptr);
        ATLASSERT(objHdl != nullptr);
        CString sObjDetails, str;
        sObjDetails.AppendFormat(IDS_OBJPROPERTY_NAME, LPCTSTR(obj->name()));
        sObjDetails.AppendFormat(IDS_OBJPROPERTY_FULLPATH, LPCTSTR(obj->fullname()));
        sObjDetails.AppendFormat(IDS_OBJPROPERTY_TYPE, LPCTSTR(obj->type()));
        if (SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(obj))
        {
            sObjDetails.AppendFormat(IDS_OBJPROPERTY_SYMLINKTGT, LPCTSTR(symlink->target()));
        }
        if (objHdl)
        {
            sObjDetails.Append(_T("\r\n"));
            if (!NT_SUCCESS(objHdl->getOpenStatus()))
            {
                CSimpleBuf<TCHAR> status(getStatusString(objHdl->getOpenStatus()));
                sObjDetails.AppendFormat(IDS_OBJPROPERTY_OPEN_STATUS, objHdl->getOpenStatus());
                sObjDetails.AppendFormat(_T("    %s\r\n"), status.Buffer());
            }
            else
            {
                if (!NT_SUCCESS(objHdl->getQueryStatus()))
                {
                    CSimpleBuf<TCHAR> status(getStatusString(objHdl->getQueryStatus()));
                    sObjDetails.AppendFormat(IDS_OBJPROPERTY_QUERY_STATUS, objHdl->getQueryStatus());
                    sObjDetails.AppendFormat(_T("    %s\r\n"), status.Buffer());
                }
            }
            if (objHdl->hasObjectInfo())
            {
                OBJECT_BASIC_INFORMATION const& obi = objHdl->getObjectInfo();
                sObjDetails.AppendFormat(IDS_OBJPROPERTY_HANDLE_COUNT, obi.HandleCount);
                sObjDetails.AppendFormat(IDS_OBJPROPERTY_REFERENCE_COUNT, obi.ReferenceCount);
                sObjDetails.AppendFormat(IDS_OBJPROPERTY_PAGEDPOOL, obi.PagedPoolUsage);
                sObjDetails.AppendFormat(IDS_OBJPROPERTY_NONPAGEDPOOL, obi.NonPagedPoolUsage);
                CString objCreation(getformatCreationTimeString(obi.CreationTime));
                sObjDetails.AppendFormat(IDS_OBJPROPERTY_CREATION_TIME, objCreation.GetString());
                // TODO:
                // Other object-specific properties
                // Security info as SDDL
            }
            else
            {
                NTSTATUS status = objHdl->getQueryStatus();
                str.Format(IDS_OBJ_QUERY_STATUS, status);
            }
        }
        return sObjDetails;
    }

    void copyItemToClipboard(HWND hWndSource, int idCmd, GenericObject* obj, ObjectHandle* objHdl = nullptr)
    {
        ATLASSERT(obj != nullptr);
        if (::OpenClipboard(hWndSource))
        {
            CString s;
            switch (idCmd)
            {
            case ID_POPUPMENU_COPYNAME:
                s = obj->name();
                ATLTRACE2(_T("Copy name: %s\n"), s.GetString());
                break;
            case ID_POPUPMENU_COPYFULLPATH:
                s = obj->fullname();
                ATLTRACE2(_T("Copy full path: %s\n"), s.GetString());
                break;
            case ID_POPUPMENU_COPYASCSTRING:
                {

#if defined(_UNICODE) || defined(UNICODE)
                    CString fullName(obj->fullname());
                    CString escapedName;
                    LPTSTR str = fullName.LockBuffer();
                    escapedName.Preallocate(fullName.GetAllocLength());
                    for (int i = 0; i < fullName.GetLength(); i++)
                    {
                        USHORT w = static_cast<USHORT>(str[i]);
                        if (w > 0xFF)
                        {
                            escapedName.AppendFormat(_T("\\u%04X"), w);
                        }
                        else
                        {
                            switch (str[i])
                            {
                            case _T('\a'):
                                escapedName.Append(_T("\\a"));
                                break;
                            case _T('\b'):
                                escapedName.Append(_T("\\b"));
                                break;
                            case _T('\f'):
                                escapedName.Append(_T("\\f"));
                                break;
                            case _T('\n'):
                                escapedName.Append(_T("\\n"));
                                break;
                            case _T('\r'):
                                escapedName.Append(_T("\\r"));
                                break;
                            case _T('\t'):
                                escapedName.Append(_T("\\t"));
                                break;
                            case _T('\v'):
                                escapedName.Append(_T("\\v"));
                                break;
                            case _T('"'):
                                escapedName.Append(_T("\\\""));
                                break;
                            case _T('\\'):
                                escapedName.Append(_T("\\\\"));
                                break;
                            default:
                                if (_istprint(str[i]))
                                    escapedName.AppendChar(str[i]);
                                else
                                    escapedName.AppendFormat(_T("\\u%04X"), w);
                                break;
                            }
                        }
                    }
                    escapedName.AppendChar(0);
                    s.Format(_T("L\"%s\""), escapedName.GetString());
#else
                    s.Format(_T("\"%s\""), obj->fullname());
#endif
                    ATLTRACE2(_T("Copy full path as C string: %s\n"), s.GetString());
                }
                break;
            case ID_POPUPMENU_COPYSYMLINKTARGET:
                if (SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(obj))
                {
                    s = symlink->target();
                    ATLTRACE2(_T("Copy symlink target: %s\n"), s.GetString());
                }
                break;
            case ID_COPY_DETAILS:
                s = getObjectDetailsString(obj, objHdl);
                break;
            }
            ATLVERIFY(setClipboardString(s.GetString()));
            ATLVERIFY(::CloseClipboard());
        }
    }

    CSimpleBuf<TCHAR> getStatusString(LONG status)
    {
        if (HRESULT_FACILITY(status) == FACILITY_WIN32)
        {
            CLoadLibrary kernel32;
            return CSimpleBuf<TCHAR>(kernel32.formatSystemMessage<TCHAR>(static_cast<DWORD>(status)));
        }
        else
        {
            CLoadLibrary ntdll(_T("ntdll.dll"));
            return CSimpleBuf<TCHAR>(ntdll.formatMessage<TCHAR>(ntdll.getHandle(), static_cast<DWORD>(status)));
        }
    }

    // Example from https://msdn.microsoft.com/en-us/library/windows/desktop/aa376389.aspx
    BOOL isUserAdmin()
    {
        BOOL bResult;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID AdministratorsGroup;
        bResult = ::AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
        if (bResult)
        {
            if (!::CheckTokenMembership(nullptr, AdministratorsGroup, &bResult))
            {
                bResult = FALSE;
            }
            ::FreeSid(AdministratorsGroup);
        }

        return bResult;
    }

    BOOL isElevated()
    {
        BOOL bResult = FALSE;
        HANDLE hToken = nullptr;
        if (::OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            TOKEN_ELEVATION Elevation;
            DWORD cbSize = sizeof(TOKEN_ELEVATION);
            if (::GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
            {
                bResult = Elevation.TokenIsElevated;
            }
        }
        if (hToken)
        {
            ::CloseHandle(hToken);
        }

        return bResult;
    }
} // anonymous namespace

class CSuppressRedraw
{
    HWND m_hWnd;
    CSuppressRedraw();
    CSuppressRedraw(CSuppressRedraw const&);
    CSuppressRedraw& operator=(CSuppressRedraw const&);

  public:
    CSuppressRedraw(HWND hWnd)
        : m_hWnd(hWnd)
    {
        if (::IsWindow(m_hWnd))
        {
            ::SendMessage(m_hWnd, WM_SETREDRAW, FALSE, 0);
        }
    }

    ~CSuppressRedraw()
    {
        if (::IsWindow(m_hWnd))
        {
            ::SendMessage(m_hWnd, WM_SETREDRAW, TRUE, 0);
            try
            {
                CRect rect;
                ATLVERIFY(::GetClientRect(m_hWnd, &rect));
                ATLVERIFY(::InvalidateRect(m_hWnd, &rect, FALSE));
            }
            catch (...)
            {
                ATLTRACE2(_T("Someone barfed at us while trying to invalidate the client rect.\n"));
            }
        }
    }
};

class CObjectImageList
{
    CImageList m_imagelist;

  public:
    CObjectImageList()
        : m_imagelist()
    {
        prepareImageList_();
    }

    virtual ~CObjectImageList()
    {
    }

    inline operator CImageList const&() const
    {
        return m_imagelist;
    }

    inline operator HIMAGELIST() const
    {
        return HIMAGELIST(m_imagelist);
    }

    inline bool IsNull() const
    {
        return m_imagelist.IsNull();
    }

    inline BOOL Create(int cx, int cy, UINT nFlags, int nInitial, int nGrow)
    {
        return m_imagelist.Create(cx, cy, nFlags, nInitial, nGrow);
    }

    inline BOOL Create(ATL::_U_STRINGorID bitmap, int cx, int nGrow, COLORREF crMask)
    {
        return m_imagelist.Create(bitmap, cx, nGrow, crMask);
    }

    inline int AddIcon(HICON hIcon)
    {
        return m_imagelist.AddIcon(hIcon);
    }

    int IndexByObjType(GenericObject* obj) const
    {
        return GetImageIndexByType_(obj);
    }

    HICON IconByObjType(GenericObject* obj)
    {
        return m_imagelist.ExtractIcon(GetImageIndexByType_(obj));
    }

  private:
    // Retrieve the image index based on the type string
    int GetImageIndexByType_(GenericObject* obj) const
    {
        ATLASSERT(nullptr != obj);
        LPCTSTR typeName = obj->type();
        int retval = 0;
        for (size_t i = 0; i < resIconTypeMappingSize; i++)
        {
            if (0 == _tcsicmp(typeName, resIconTypeMapping[i].typeName))
            {
                if (IDI_DIRECTORY == resIconTypeMapping[i].resId)
                {
                    // This program is not multi-threaded, so this works, otherwise we'd need locking here
                    static int emptydir = -1; //-V1096
                    if (Directory* dir = dynamic_cast<Directory*>(obj))
                    {
                        if (!dir->size())
                        {
                            if (-1 == emptydir)
                            {
                                for (size_t j = 0; j < resIconTypeMappingSize; j++)
                                {
                                    if (0 == _tcsicmp(_T(OBJTYPESTR_EMPTY_DIRECTORY), resIconTypeMapping[j].typeName))
                                    {
                                        emptydir = static_cast<int>(j);
                                        break;
                                    }
                                }
                            }
                            ATLASSERT(-1 != emptydir);
                            return emptydir;
                        }
                    }
                }
                return static_cast<int>(i);
            }
        }
        return retval;
    }

    // We create our image list dynamically from the icons
    void prepareImageList_()
    {
        ATLTRACE2(_T("Preparing image list for %hs\n"), __func__);
        const int iResIconTypeMappingSize = static_cast<int>(resIconTypeMappingSize);
        ATLVERIFY(m_imagelist.Create(imgListElemWidth, imgListElemHeight, ILC_COLOR32 | ILC_MASK, iResIconTypeMappingSize, iResIconTypeMappingSize));
        for (int i = 0; i < iResIconTypeMappingSize; i++)
        {
            HICON hIcon =
                static_cast<HICON>(AtlLoadIconImage(MAKEINTRESOURCE(resIconTypeMapping[i].resId), LR_CREATEDIBSECTION, imgListElemWidth, imgListElemHeight));
            ATLTRACE2(_T("Icon handle: %p - %s\n"), hIcon, resIconTypeMapping[i].typeName);
            m_imagelist.AddIcon(hIcon);
        }
    }
};

#if FEATURE_OBJECT_SECURITY
#    include <Aclui.h>
#    ifndef __ACCESS_CONTROL_API__
#        include <Aclapi.h>
#    endif
#    pragma comment(lib, "aclui.lib")
#endif // FEATURE_OBJECT_SECURITY

template <typename T> class CObjectPropertySheetT : public CPropertySheetImpl<CObjectPropertySheetT<T>>
{
    class CObjectSecurityNAPage : public CPropertyPageImpl<CObjectSecurityNAPage>
    {
        typedef CPropertyPageImpl<CObjectSecurityNAPage> baseClass;
        GenericObject* m_obj;
        ObjectHandle& m_objHdl;
        bool m_bFeatureUnavailable;
        CFont m_font;
        CBrush m_bkBrush;
        CEdit m_edtExplanation;
        CObjectSecurityNAPage(); // hide it
      public:
        enum
        {
            IDD = IDD_SECURITY_NOT_AVAILABLE
        };

        BEGIN_MSG_MAP(CObjectSecurityNAPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog) //-V1048
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        CHAIN_MSG_MAP(baseClass)
        END_MSG_MAP()

        CObjectSecurityNAPage(GenericObject* obj, ObjectHandle& objHdl, bool bFeatureUnavailable = false)
#pragma warning(suppress : 6387)
            : baseClass()
            , m_obj(obj)
            , m_objHdl(objHdl)
            , m_bFeatureUnavailable(bFeatureUnavailable)
        {
        }

        virtual ~CObjectSecurityNAPage()
        {
            m_obj = 0;
        }

        LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
        {
            m_edtExplanation.Attach(CWindow::GetDlgItem(IDC_EXPLANATION_WHY));

            if (m_bFeatureUnavailable)
            {
                CString str;
#pragma warning(suppress : 6031)
                m_edtExplanation.SetWindowText(str);
            }
            else
            {
                if (!m_objHdl)
                {
                    CSimpleBuf<TCHAR> status(getStatusString(m_objHdl.getOpenStatus()));
                    CString sStatus;
                    sStatus.Format(IDS_STATUS_DESCRIPTION, m_objHdl.getOpenStatus(), status.Buffer());
                    m_edtExplanation.SetWindowText(sStatus);
                }
            }
            m_edtExplanation.ModifyStyle(ES_NOHIDESEL, 0);
            m_edtExplanation.SetSelNone(TRUE);

            NONCLIENTMETRICS ncm = {0};
            ncm.cbSize = sizeof(ncm);

            if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
            {
                LOGFONT logFont;
                memcpy(&logFont, &ncm.lfMessageFont, sizeof(LOGFONT));
                logFont.lfWeight = FW_BOLD;
                HFONT hFont = m_font.CreateFontIndirect(&logFont);
                if (hFont)
                {
                    this->SendDlgItemMessage(IDC_EXPLANATION_WHY, WM_SETFONT, reinterpret_cast<WPARAM>(hFont));
                    this->SendDlgItemMessage(IDC_NO_SECURITY_CAPTION, WM_SETFONT, reinterpret_cast<WPARAM>(hFont));
                }
            }

            bHandled = FALSE;
            return TRUE;
        }

        LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            HWND hStatic = reinterpret_cast<HWND>(lParam);
            HDC hdcStatic = reinterpret_cast<HDC>(wParam);
            switch (::GetDlgCtrlID(hStatic))
            {
            case IDC_EXPLANATION_WHY:
                {
                    ::SetTextColor(hdcStatic, RGB(0x80, 0, 0));
                    CDC dc(CWindow::GetDC());
                    COLORREF dlgBkColor = dc.GetBkColor();
                    ::SetBkColor(hdcStatic, dlgBkColor);
                    m_bkBrush = ::CreateSolidBrush(dlgBkColor);
                    return reinterpret_cast<LRESULT>(HBRUSH(m_bkBrush));
                }
            default:
                break;
            }
            bHandled = FALSE;
            return FALSE;
        }
    };

    class CObjectDetailsPage : public CPropertyPageImpl<CObjectDetailsPage>
    {
        typedef CPropertyPageImpl<CObjectDetailsPage> baseClass;
        GenericObject* m_obj;
        ObjectHandle& m_objHdl;
        CAccelerator m_Accel;
        CEdit m_edtName;
        CEdit m_edtFullname;
        CEdit m_edtType;
        CFont m_font;
        CBrush m_bkBrush;
        CEdit m_edtExplanation;
        CStatic m_stcRefByPtr;
        CStatic m_stcRefByHdl;
        CStatic m_stcQuotaPaged;
        CStatic m_stcQuotaNonPaged;
        CStatic m_stcCreationTime;
        CStatic m_stcGroupObjSpecific;
        CStatic m_stcObjSpecName1;
        CStatic m_stcObjSpecName2;
        CStatic m_stcObjSpecName3;
        CStatic m_stcObjSpecAttr1;
        CStatic m_stcObjSpecAttr2;
        CStatic m_stcObjSpecAttr3;

        CObjectDetailsPage(); // hide it
      public:
        enum
        {
            IDD = IDD_PROPERTIES
        };

        BEGIN_MSG_MAP(CObjectDetailsPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog) //-V1048
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        COMMAND_ID_HANDLER(ID_COPY_DETAILS, OnCopyObjectDetails)
        CHAIN_MSG_MAP(baseClass)
        END_MSG_MAP()

        CObjectDetailsPage(GenericObject* obj, ObjectHandle& objHdl)
#pragma warning(suppress : 6387)
            : baseClass()
            , m_obj(obj)
            , m_objHdl(objHdl)
        {
        }

        LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
        {
            m_edtName.Attach(CWindow::GetDlgItem(IDC_EDIT_NAME));
            m_edtFullname.Attach(CWindow::GetDlgItem(IDC_EDIT_FULLNAME));
            m_edtType.Attach(CWindow::GetDlgItem(IDC_EDIT_TYPE));
            m_edtExplanation.Attach(CWindow::GetDlgItem(IDC_EXPLANATION_WHY));
            m_stcRefByPtr.Attach(CWindow::GetDlgItem(IDC_REF_BYPTR));
            m_stcRefByHdl.Attach(CWindow::GetDlgItem(IDC_REF_BYHDL));
            m_stcQuotaPaged.Attach(CWindow::GetDlgItem(IDC_QUOTA_PAGED));
            m_stcQuotaNonPaged.Attach(CWindow::GetDlgItem(IDC_QUOTA_NONPAGED));
            m_stcCreationTime.Attach(CWindow::GetDlgItem(IDC_OBJ_CREATION_TIME));
            m_stcGroupObjSpecific.Attach(CWindow::GetDlgItem(IDC_GROUP_OBJSPECIFIC));
            m_stcObjSpecName1.Attach(CWindow::GetDlgItem(IDC_STATIC_OBJSPEC_NAME1));
            m_stcObjSpecName2.Attach(CWindow::GetDlgItem(IDC_STATIC_OBJSPEC_NAME2));
            m_stcObjSpecName3.Attach(CWindow::GetDlgItem(IDC_STATIC_OBJSPEC_NAME3));
            m_stcObjSpecAttr1.Attach(CWindow::GetDlgItem(IDC_STATIC_OBJSPEC_ATTR1));
            m_stcObjSpecAttr2.Attach(CWindow::GetDlgItem(IDC_STATIC_OBJSPEC_ATTR2));
            m_stcObjSpecAttr3.Attach(CWindow::GetDlgItem(IDC_STATIC_OBJSPEC_ATTR3));

            ATLASSERT(m_obj);
            if (m_obj)
            {
                m_edtName.SetWindowText(m_obj->name());
                m_edtFullname.SetWindowText(m_obj->fullname());
                m_edtType.SetWindowText(m_obj->type());
            }

            CString na;
#pragma warning(suppress : 6031)
            ATLVERIFY(na.LoadString(IDS_NOT_AVAILABLE_SHORT));
            if (m_objHdl)
            {
                CString str;
                if (m_objHdl.hasObjectInfo())
                {
                    OBJECT_BASIC_INFORMATION const& obi = m_objHdl.getObjectInfo();
                    str.Format(_T("%u"), obi.HandleCount);
                    m_stcRefByHdl.SetWindowText(str);
                    str.Format(_T("%u"), obi.ReferenceCount);
                    m_stcRefByPtr.SetWindowText(str);
                    str.Format(_T("%u"), obi.PagedPoolUsage);
                    m_stcQuotaPaged.SetWindowText(str);
                    str.Format(_T("%u"), obi.NonPagedPoolUsage);
                    m_stcQuotaNonPaged.SetWindowText(str);
                    m_stcCreationTime.SetWindowText(getformatCreationTimeString(obi.CreationTime));

                    showObjectSpecificInfo_();
                }
                else
                {
                    m_stcRefByHdl.SetWindowText(na);
                    m_stcRefByPtr.SetWindowText(na);
                    m_stcQuotaPaged.SetWindowText(na);
                    m_stcQuotaNonPaged.SetWindowText(na);
                    NTSTATUS status = m_objHdl.getQueryStatus();
                    str.Format(IDS_OBJ_QUERY_STATUS, status);
                    m_stcCreationTime.SetWindowText(str);
                }
            }
            else
            {
                m_stcRefByHdl.SetWindowText(na);
                m_stcRefByPtr.SetWindowText(na);
                m_stcQuotaPaged.SetWindowText(na);
                m_stcQuotaNonPaged.SetWindowText(na);
                NTSTATUS status = m_objHdl.getOpenStatus();
                CString str;
                str.Format(IDS_OBJ_OPEN_STATUS, status);
                m_stcCreationTime.SetWindowText(str);
            }

            NONCLIENTMETRICS ncm = {0};
            ncm.cbSize = sizeof(ncm);

            if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
            {
                LOGFONT logFont;
                memcpy(&logFont, &ncm.lfMessageFont, sizeof(LOGFONT));
                logFont.lfWeight = FW_BOLD;
                HFONT hFont = m_font.CreateFontIndirect(&logFont);
                if (hFont)
                {
                    this->SendDlgItemMessage(IDC_EXPLANATION_WHY, WM_SETFONT, reinterpret_cast<WPARAM>(hFont));
                }
            }

            m_edtExplanation.ModifyStyle(ES_NOHIDESEL, 0);
            m_edtExplanation.SetSelNone(TRUE);

            // Load the accelerator table corresponding to the dialog's resource ID
            m_Accel.LoadAccelerators(IDD);
            if (!m_Accel.IsNull())
            {
                ATLTRACE2(_T("Loaded accelerator table: %p\n"), HACCEL(m_Accel));
            }
            else
            {
                ATLTRACE2(_T("Failed to load accelerators for object details page (last error: %d)\n"), GetLastError());
            }

            return TRUE;
        }

        LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            HWND hStatic = reinterpret_cast<HWND>(lParam);
            HDC hdcStatic = reinterpret_cast<HDC>(wParam);
            switch (::GetDlgCtrlID(hStatic))
            {
            case IDC_EXPLANATION_WHY:
                {
                    ::SetTextColor(hdcStatic, RGB(0x80, 0, 0));
                    CDC dc(CWindow::GetDC());
                    COLORREF dlgBkColor = dc.GetBkColor();
                    ::SetBkColor(hdcStatic, dlgBkColor);
                    m_bkBrush = ::CreateSolidBrush(dlgBkColor);
                    return reinterpret_cast<LRESULT>(HBRUSH(m_bkBrush));
                }
            default:
                break;
            }
            bHandled = FALSE;
            return FALSE;
        }

        LRESULT OnTranslateAccelerator(MSG* pMsg)
        {
            if (!m_Accel.IsNull())
            {
                if (m_Accel.TranslateAccelerator(CWindow::m_hWnd, pMsg))
                {
                    return PSNRET_MESSAGEHANDLED;
                }
            }
            return PSNRET_NOERROR;
        }

        LRESULT OnCopyObjectDetails(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
        {
            ATLASSERT(wID == ID_COPY_DETAILS);
            copyItemToClipboard(CWindow::m_hWnd, wID, m_obj, &m_objHdl);
            return FALSE;
        }

      private:
        void setAttributesVisible_(int num)
        {
            m_stcGroupObjSpecific.ShowWindow(SW_SHOWNOACTIVATE);
            if (num < 0)
            {
                m_edtExplanation.ShowWindow(SW_SHOWNOACTIVATE);
                return;
            }
            if (m_obj)
            {
                CString str;
                str.Format(IDS_OBJSPEC_INFO, m_obj->type().GetString());
                m_stcGroupObjSpecific.SetWindowText(str);
            }
            if (num >= 1)
            {
                m_stcObjSpecName1.ShowWindow(SW_SHOWNOACTIVATE);
                m_stcObjSpecAttr1.ShowWindow(SW_SHOWNOACTIVATE);
            }
            if (num >= 2)
            {
                m_stcObjSpecName2.ShowWindow(SW_SHOWNOACTIVATE);
                m_stcObjSpecAttr2.ShowWindow(SW_SHOWNOACTIVATE);
            }
            if (num >= 3)
            {
                m_stcObjSpecName3.ShowWindow(SW_SHOWNOACTIVATE);
                m_stcObjSpecAttr3.ShowWindow(SW_SHOWNOACTIVATE);
            }
        }

        void showObjectSpecificError_(NTSTATUS statusCode)
        {
            CSimpleBuf<TCHAR> status(getStatusString(m_objHdl.getOpenStatus()));
            CString sStatus;
            sStatus.Format(IDS_STATUS_DESCRIPTION, statusCode, status.Buffer());
            m_edtExplanation.SetWindowText(sStatus);
            if (m_obj)
            {
                CString str;
                str.Format(IDS_OBJSPEC_ERROR, m_obj->type().GetString());
                m_stcGroupObjSpecific.SetWindowText(str);
            }
            setAttributesVisible_(-1);
        }

#ifndef SEC_IMAGE_NO_EXECUTE
#    define SEC_IMAGE_NO_EXECUTE (SEC_IMAGE | SEC_NOCACHE)
#endif
        static CString getReadableAllocationAttributesString_(ULONG aa)
        {
            // TBD: should we localize these strings or rather not?
            CString str;
            if (PAGE_NOACCESS & aa)
            {
                str.Append(_T("!access+"));
            }
            if (PAGE_READONLY & aa)
            {
                str.Append(_T("ro+"));
            }
            if (PAGE_READWRITE & aa)
            {
                str.Append(_T("rw+"));
            }
            if (PAGE_WRITECOPY & aa)
            {
                str.Append(_T("cow+"));
            }
            if (PAGE_EXECUTE & aa)
            {
                str.Append(_T("x+"));
            }
            if (PAGE_EXECUTE_READ & aa)
            {
                str.Append(_T("xro+"));
            }
            if (PAGE_EXECUTE_READWRITE & aa)
            {
                str.Append(_T("xrw+"));
            }
            if (PAGE_EXECUTE_WRITECOPY & aa)
            {
                str.Append(_T("xcow+"));
            }
            if (PAGE_GUARD & aa)
            {
                str.Append(_T("guard+"));
            }
            if ((PAGE_NOCACHE & aa) || (SEC_NOCACHE & aa))
            {
                str.Append(_T("!cache+"));
            }
            if ((PAGE_WRITECOMBINE & aa) || (SEC_WRITECOMBINE & aa))
            {
                str.Append(_T("wcomb+"));
            }
            if (SEC_FILE & aa)
            {
                str.Append(_T("file+"));
            }
            if (SEC_IMAGE & aa)
            {
                if (SEC_IMAGE_NO_EXECUTE & aa)
                {
                    str.Append(_T("img!x+"));
                }
                else
                {
                    str.Append(_T("img+"));
                }
            }
            if (SEC_PROTECTED_IMAGE & aa)
            {
                str.Append(_T("protimg+"));
            }
            if (SEC_RESERVE & aa)
            {
                str.Append(_T("resrv+"));
            }
            if (SEC_COMMIT & aa)
            {
                str.Append(_T("commit+"));
            }
            if (SEC_LARGE_PAGES & aa)
            {
                str.Append(_T("lrgpgs+"));
            }

            if (str.GetLength())
            {
                str.Truncate(str.GetLength() - 1);
            }
            return str;
        }

        void showObjectSpecificInfo_()
        {
            CString str;

            if (m_obj && otSymlink == m_obj->objtype())
            {
                if (SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(m_obj))
                {
                    setAttributesVisible_(1);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_SYMLINK));
                    m_stcObjSpecName1.SetWindowText(str);
                    m_stcObjSpecAttr1.SetWindowText(symlink->target());
                    // m_tooltip.Attach(GetDlgItem(IDC_STATIC_OBJSPEC_ATTR3));
                }
            }

            if (ObjectHandle::CEventBasicInformation const* info = m_objHdl.getEventBasicInfo())
            {
                if (*info)
                {
                    setAttributesVisible_(2);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_EVENT));
                    m_stcObjSpecName1.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_EVENT));
                    m_stcObjSpecName2.SetWindowText(str);
                    str.Format(_T("%d"), info->EventState);
                    m_stcObjSpecAttr1.SetWindowText(str);
                    switch (info->EventType)
                    {
                    case SynchronizationEvent:
#pragma warning(suppress : 6031)
                        ATLVERIFY(str.LoadString(IDS_OBJSPEC_ATTR2_EVENT_SYNC));
                        break;
                    case NotificationEvent:
#pragma warning(suppress : 6031)
                        ATLVERIFY(str.LoadString(IDS_OBJSPEC_ATTR2_EVENT_NOTIFY));
                        break;
                    default:
                        str.Format(IDS_UNKNOWN_FMTSTR, info->EventType);
                        break;
                    }
                    m_stcObjSpecAttr2.SetWindowText(str);
                }
                else
                    showObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CIoCompletionBasicInformation const* info = m_objHdl.getIoCompletionBasicInfo())
            {
                if (*info)
                {
                    setAttributesVisible_(1);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_IOCOMPLETION));
                    m_stcObjSpecName1.SetWindowText(str);
                    str.Format(_T("%d"), info->Depth);
                    m_stcObjSpecAttr1.SetWindowText(str);
                }
                else
                    showObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CKeyBasicInformation const* info = m_objHdl.getKeyBasicInfo())
            {
                if (*info)
                {
                    setAttributesVisible_(2);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_KEY));
                    m_stcObjSpecName1.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_KEY));
                    m_stcObjSpecName2.SetWindowText(str);
                    str = getformatCreationTimeString(info->LastWriteTime);
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%u"), info->TitleIndex);
                    m_stcObjSpecAttr2.SetWindowText(str);
                }
                else
                    showObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CMutantBasicInformation const* info = m_objHdl.getMutantBasicInfo())
            {
                if (*info)
                {
                    setAttributesVisible_(3);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_MUTANT));
                    m_stcObjSpecName1.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_MUTANT));
                    m_stcObjSpecName2.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME3_MUTANT));
                    m_stcObjSpecName3.SetWindowText(str);
                    str = info->AbandonedState ? _T("yes") : _T("no");
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%d"), info->CurrentCount);
                    m_stcObjSpecAttr2.SetWindowText(str);
                    str = info->OwnedByCaller ? _T("yes") : _T("no");
                    m_stcObjSpecAttr3.SetWindowText(str);
                }
                else
                    showObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CSectionBasicInformation const* info = m_objHdl.getSectionBasicInfo())
            {
                if (*info)
                {
                    setAttributesVisible_(3);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_SECTION));
                    m_stcObjSpecName1.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_SECTION));
                    m_stcObjSpecName2.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME3_SECTION));
                    m_stcObjSpecName3.SetWindowText(str);
                    str.Format(_T("0x%p"), info->BaseAddress);
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%I64d"), info->MaximumSize.QuadPart);
                    m_stcObjSpecAttr2.SetWindowText(str);
                    str.Format(_T("%08X"), info->AllocationAttributes);
                    CString readable = getReadableAllocationAttributesString_(info->AllocationAttributes);
                    if (readable.GetLength())
                    {
                        str.AppendFormat(_T(" (%s)"), readable.GetString());
                    }
                    m_stcObjSpecAttr3.SetWindowText(str);
                }
                else
                    showObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CSemaphoreBasicInformation const* info = m_objHdl.getSemaphoreBasicInfo())
            {
                if (*info)
                {
                    setAttributesVisible_(2);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_SEMAPHORE));
                    m_stcObjSpecName1.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_SEMAPHORE));
                    m_stcObjSpecName2.SetWindowText(str);
                    str.Format(_T("%u"), info->CurrentCount);
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%u"), info->MaximumCount);
                    m_stcObjSpecAttr2.SetWindowText(str);
                }
                else
                    showObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CTimerBasicInformation const* info = m_objHdl.getTimerBasicInfo())
            {
                if (*info)
                {
                    setAttributesVisible_(2);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_TIMER));
                    m_stcObjSpecName1.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_TIMER));
                    m_stcObjSpecName2.SetWindowText(str);
                    str = info->TimerState ? _T("on") : _T("off");
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%I64d"), info->RemainingTime.QuadPart);
                    m_stcObjSpecAttr2.SetWindowText(str);
                }
                else
                    showObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CWinStaInformation const* info = m_objHdl.getWinStaInfo())
            {
                ATLTRACE2(_T("info = %p\n"), info);
                if (*info)
                {
                    setAttributesVisible_(3);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_WINSTA));
                    m_stcObjSpecName1.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_WINSTA));
                    m_stcObjSpecName2.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME3_WINSTA));
                    m_stcObjSpecName3.SetWindowText(str);
#pragma warning(suppress : 6031)
                    ATLVERIFY(str.LoadString(IDS_NOT_AVAILABLE_SHORT));
                    m_stcObjSpecAttr1.SetWindowText(info->username.GetLength() ? info->username : str);
                    m_stcObjSpecAttr2.SetWindowText(info->sid.GetLength() ? info->sid : str);
                    m_stcObjSpecAttr3.SetWindowText(info->flags.GetLength() ? info->flags : str);
                }
                else
                {
                    ATLTRACE2(_T("error = %p\n"), info);
                    showObjectSpecificError_(info->getQueryStatus());
                }
            }
        }
    };

#if FEATURE_OBJECT_SECURITY
    // https://
    // raw.githubusercontent.com/microsoft/Windows-classic-samples/main/Samples/CustomResourceManager/cpp/SecurityInformation1.cpp

    class CSecurityInformation : public ISecurityInformation
    {
        GenericObject* m_obj;
        GENERIC_MAPPING m_gmask;
        ObjectHandle& m_objHdl;
        ULONG m_refCount;

        bool setGenericMapping_(GenericObject* obj, GENERIC_MAPPING& mask)
        {
            if (obj)
            {
                extern NtObjMgr::ObjectTypeMap g_objTypeMap;

                OBJECT_TYPE_INFORMATION const* potInfo = g_objTypeMap.Lookup(obj->type());
                //.Lookup(obj->type);
                if (potInfo)
                {
                    mask = potInfo->GenericMapping;
                    return true;
                }
            }
            return false;
        }

      public:
        CSecurityInformation(GenericObject* obj, ObjectHandle& objHdl)
            : m_obj(obj)
#    if _MSVC_LANG >= 201103L
            , m_gmask({0, 0, 0, 0})
#    endif // _MSVC_LANG >= 201703L
            , m_objHdl(objHdl)
            , m_refCount(1)
        {
            ATLVERIFY(setGenericMapping_(m_obj, m_gmask));
        }

        STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) //-V835
        {
            if (riid == IID_IUnknown || riid == IID_ISecurityInformation)
            {
                *ppvObj = this;
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)()
        {
            return InterlockedIncrement(&m_refCount);
        }

        STDMETHOD_(ULONG, Release)()
        {
            return InterlockedDecrement(&m_refCount);
        }

        STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo)
        {
            pObjectInfo->dwFlags = SI_ADVANCED | SI_NO_TREE_APPLY | SI_READONLY;
            pObjectInfo->hInstance = nullptr;
            pObjectInfo->pszObjectName = const_cast<LPTSTR>(m_obj->name().GetString());

#    if 0
    SI_ADVANCED: Specifies the "Advanced" button will be shown so that the user can enter the advanced UI.
    SI_CONTAINER: Indicates that your object is a directory (or behaves like a directory). You should set this flag if the object is a directory so the ACL editor can apply inheritance on the object. You may also want to enable the inheritance stuff.
    SI_EDIT_AUDITS: Shows the "Auditing" tab (so the user can edit the SACL). To view / edit the SACL, you must have the "SeSecurityPrivilege".
    SI_EDIT_PERMS: Shows the "Advanced" permissions tab (so the user can fully edit the DACL).
    SI_EDIT_OWNER: Shows the Owner tab (so the user can change or take ownership).
    SI_EDIT_PROPERTIES: Shows the properties tab (so the user can edit the properties associated with the object).
    SI_EDIT_EFFECTIVE (undocumented, for Windows XP and later): Shows the "Effective Permissions" tab.
    SI_READONLY: Does not allow the user to change the security descriptor.
    SI_OWNER_READONLY: Does not allow the user to change the owner (to change ownership, the user must be the owner, or have the "SeTakeOwnershipPrivilege").
    SI_MAY_WRITE (undocumented): "Not sure if the user may write permission".
    SI_NO_ADDITIONAL_PERMISSION (Undocumented): ??????
    SI_NO_ACL_PROTECT: Select this if you haven't implemented inheritance. This hides the auto-inherit checkbox.
    SI_OWNER_RECURSE: Shows the "Sledgehammer" button, which enables the user to reset all children's security descriptors to default. Do not show this unless you have implemented recursive walking in your object.
    SI_RESET_DACL_TREE: Like SI_OWNER_RECURSE, but for the Advanced Permissions tab.
    SI_RESET_SACL_TREE: Like SI_RESET_DACL_TREE, but for the "Auditing" tab.
    SI_SERVER_IS_DC: Set this if you have determined the computer is in a domain environment.
    SI_EDIT_ALL: shorthand for SI_EDIT_AUDITS | SI_EDIT_PERMS | SI_EDIT_OWNER.
    SI_NO_TREE_APPLY: Hides the "Apply these permissions to objects and/or containers within this container only" checkbox. Set this flag if your object does not support inheritance.
    SI_PAGE_TITLE: Indicates you want a custom title to appear in the ACL editor. Specify the alternate title in the pszPageTitle member.
    SI_RESET: Shows the "Default" button, which will reset the security descriptor to defaults. If you don't know what the default security descriptor is, do not enable this flag.
    SI_RESET_SACL, SI_RESET_DACL, SI_RESET_OWNER: shows the "Default" button for certain tabs only.
    SI_OBJECT_GUID: (domain-specific) If your object supports multiple inheritance, you should enable this flag.
#    endif

            return S_OK;
        }

        STDMETHOD(GetSecurity)(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL /*fDefault*/)
        {
            HANDLE hObj = m_objHdl.getHandle();
            ATLASSERT(INVALID_HANDLE_VALUE != hObj);
            ATLASSERT(nullptr != hObj);
            DWORD dwResult = ::GetSecurityInfo(hObj, SE_KERNEL_OBJECT, si, nullptr, nullptr, nullptr, nullptr, ppSecurityDescriptor);
            return (ERROR_SUCCESS == dwResult) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
        }

        STDMETHOD(SetSecurity)(SECURITY_INFORMATION /*si*/, PSECURITY_DESCRIPTOR /*pSecurityDescriptor*/)
        {
            ATLTRACENOTIMPL(_T(__FUNCTION__));
        }

        STDMETHOD(GetAccessRights)
        (const GUID* /*pguidObjectType*/, DWORD /*dwFlags*/, PSI_ACCESS* ppAccess, ULONG* pcAccesses, ULONG* piDefaultAccess)
        {
            ATLASSERT(TIMER_QUERY_STATE == 0x0001);
            ATLASSERT(TIMER_MODIFY_STATE == 0x0002);
            ATLASSERT(TIMER_QUERY_STATE == MUTANT_QUERY_STATE);
            ATLASSERT(TIMER_MODIFY_STATE == SEMAPHORE_MODIFY_STATE);
            ATLASSERT(TIMER_MODIFY_STATE == IO_COMPLETION_MODIFY_STATE);
            static SI_ACCESS accessNameMapping[] = {{&GUID_NULL, GENERIC_READ, _T("Read"), SI_ACCESS_GENERAL}, //-V1096
                                                    {&GUID_NULL, GENERIC_WRITE, _T("Write"), SI_ACCESS_GENERAL},
                                                    {&GUID_NULL, GENERIC_EXECUTE, _T("Execute"), SI_ACCESS_GENERAL},
                                                    {&GUID_NULL, 0x0001, _T("Query State"), SI_ACCESS_GENERAL},
                                                    {&GUID_NULL, 0x0002, _T("Modify State"), SI_ACCESS_GENERAL},
                                                    { &GUID_NULL,
                                                      SYNCHRONIZE,
                                                      _T("Synchronize"),
                                                      SI_ACCESS_GENERAL }};

            *ppAccess = accessNameMapping;
            *pcAccesses = _countof(accessNameMapping);
            *piDefaultAccess = 0;

            return S_OK;
        }

        STDMETHOD(MapGeneric)(const GUID* pguidObjectType, UCHAR* pAceFlags, ACCESS_MASK* pMask)
        {
            if (!pAceFlags || !pMask)
            {
                return E_INVALIDARG;
            }

            // This sample doesn't include object inheritance, so that bit can be
            // safely removed.
            *pAceFlags &= ~OBJECT_INHERIT_ACE;
            GENERIC_MAPPING gmapping = m_gmask;
            ::MapGenericMask(pMask, &gmapping);

            UNREFERENCED_PARAMETER(pguidObjectType);
            return S_OK;
        }

        STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE* /*ppInheritTypes*/, ULONG* /*pcInheritTypes*/)
        {
            ATLTRACENOTIMPL(_T(__FUNCTION__));
        }

        STDMETHOD(PropertySheetPageCallback)(HWND /*hwnd*/, UINT /*uMsg*/, SI_PAGE_TYPE /*uPage*/)
        {
            ATLTRACENOTIMPL(_T(__FUNCTION__));
        }
    };
#endif // FEATURE_OBJECT_SECURITY

    typedef CPropertySheetImpl<CObjectPropertySheetT<T>> baseClass;

    GenericObject* m_obj;
    ObjectHandle m_objHdl;
    CObjectDetailsPage m_details;
#if FEATURE_OBJECT_SECURITY
    ATL::CAutoPtr<CSecurityInformation> m_security;
#endif // FEATURE_OBJECT_SECURITY
    ATL::CAutoPtr<CObjectSecurityNAPage> m_securityNA;

  public:
    BEGIN_MSG_MAP(CObjectPropertySheetT<T>)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog) //-V1048
    CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    CObjectPropertySheetT(GenericObject* obj, T& imagelist)
#pragma warning(suppress : 6387)
        : baseClass((LPCTSTR) nullptr, 0, nullptr)
        , m_obj(obj)
        , m_objHdl(obj)
        , m_details(m_obj, m_objHdl)
    {
        this->m_psh.dwFlags |= PSH_NOAPPLYNOW;
        if (m_obj && imagelist.IndexByObjType(m_obj) >= 0)
        {
            this->m_psh.dwFlags |= PSH_USEHICON;
            this->m_psh.hIcon = imagelist.IconByObjType(m_obj);
        }

        ATLASSERT(nullptr != m_obj);
        ATLTRACE2(_T("Adding 'Object Details' property page\n"));
        this->AddPage(m_details);

#if FEATURE_OBJECT_SECURITY
        if (!m_objHdl)
        {
            m_securityNA.Attach(new CObjectSecurityNAPage(m_obj, m_objHdl));
            ATLTRACE2(_T("Adding 'Security (N/A)' property page\n"));
            this->AddPage(*m_securityNA);
        }
        else
        {
            m_security.Attach(new CSecurityInformation(m_obj, m_objHdl));
            ATLTRACE2(_T("Adding 'Security' property page\n"));
            this->AddPage(::CreateSecurityPage(m_security));
        }
#else  // FEATURE_OBJECT_SECURITY
        m_securityNA.Attach(new CObjectSecurityNAPage(m_obj, m_objHdl, true));
        ATLTRACE2(_T("Adding 'Security (N/A)' property page - feature not built into program\n"));
        this->AddPage(*m_securityNA);
#endif // FEATURE_OBJECT_SECURITY

        this->SetTitle(obj->name(), PSH_PROPTITLE);
    }

    ~CObjectPropertySheetT()
    {
        m_obj = 0;
    }

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        return TRUE;
    }
};
typedef CObjectPropertySheetT<CObjectImageList> CObjectPropertySheet;

template <typename T> class CHyperLinkCtxMenuImpl : public CHyperLinkImpl<T>
{
    typedef CHyperLinkImpl<T> baseClass;

  public:
    BEGIN_MSG_MAP(CHyperLinkCtxMenuImpl<T>)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu) //-V1048
    CHAIN_MSG_MAP(CHyperLinkImpl<T>)
    DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CPoint pt;
        ATLVERIFY(::GetCursorPos(&pt));
        ATLTRACE2(_T("Point of click: %d/%d\n"), pt.x, pt.y);

        CMenu menu;
        ATLVERIFY(menu.LoadMenu(IDR_POPUP_HYPERLINK1));
        ATLASSERT(menu.IsMenu());

        if (menu.IsMenu())
        {
            // Get the popup menu at index 0 from the menu resource
            CMenuHandle popup = menu.GetSubMenu(0);
            ATLASSERT(popup.IsMenu());

            if (popup.IsMenu())
            {
                // Let the user pick
                int idCmd = popup.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, CWindow::GetParent(), nullptr);

                if (idCmd)
                {
                    switch (idCmd)
                    {
                    case ID_POPUPMENU_COPYURL:
                        if (this->m_lpstrHyperLink)
                        {
                            ATLTRACE2(_T("Copy URL\n"), idCmd);
                            if (::OpenClipboard(this->GetParent()))
                            {
                                ATLVERIFY(setClipboardString(this->m_lpstrHyperLink));
                                ATLVERIFY(::CloseClipboard());
                            }
                        }
                        break;
                    default:
                        ATLTRACE2(_T("Chosen command: %i\n"), idCmd);
                        break;
                    }
                }
            }
        }

        return 0;
    }
};

class CHyperLinkCtxMenu : public CHyperLinkCtxMenuImpl<CHyperLinkCtxMenu>
{
  public:
    DECLARE_WND_CLASS(_T("NtObjectsHyperLinkCtxMenu")) //-V1096
};

class CAboutDlg : public CDialogImpl<CAboutDlg>, public CMessageFilter
{
    typedef CDialogImpl<CAboutDlg> baseClass;

    CHyperLinkCtxMenu m_website, m_projectpage, m_revisionurl;
    CVersionInfo& m_verinfo;
    CIcon m_appicon;
    CFont m_progNameFont;
    CAccelerator m_Accel;
    CMessageLoop m_loop;

  public:
    enum
    {
        IDD = IDD_ABOUT
    };

    BEGIN_MSG_MAP(CAboutDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog) //-V1048
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
    COMMAND_ID_HANDLER(ID_COPY_ABOUT_INFO, OnCopyAboutInfo)
    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    CAboutDlg(CVersionInfo& verinfo)
        : m_verinfo(verinfo)
    {
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (!m_Accel.IsNull())
        {
            if (m_Accel.TranslateAccelerator(m_hWnd, pMsg))
            {
                return TRUE;
            }
        }
        if (IsWindow())
        {
            return IsDialogMessage(pMsg);
        }
        return FALSE;
    }

    void EmulateModal(_In_ HWND hWndParent = ::GetActiveWindow(), _In_ LPARAM dwInitParam = 0)
    {
        ATLASSERT(!m_bModal);
        ATLTRACE2(_T("Disabling parent window %p\n"), hWndParent);
        ::EnableWindow(hWndParent, FALSE);
        (void)Create(hWndParent, dwInitParam);
        (void)ShowWindow(SW_SHOW);
        m_loop.AddMessageFilter(this);
        ATLTRACE2(_T("Running \"inner\" message loop.\n"));
        m_loop.Run();
        ATLTRACE2(_T("Re-enabling parent window %p\n"), hWndParent);
        ::EnableWindow(hWndParent, TRUE);
        ::SetForegroundWindow(hWndParent);
        ATLTRACE2(_T("Destroying about dialog\n"));
        DestroyWindow();
    }

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        CString website(m_verinfo[_T("Website")]);
        CString repo(m_verinfo[_T("Mercurial repository")]);
        CString revision(m_verinfo[_T("Mercurial revision")]);
        CString revisionurl(m_verinfo[_T("Mercurial revision URL")]);

        ATLVERIFY(m_website.SetHyperLink(website));
        if (int slash = website.ReverseFind(_T('/')))
        {
            ATLVERIFY(m_website.SetLabel(&website.GetString()[slash + 1]));
        }
        else
        {
            ATLVERIFY(m_website.SetLabel(website));
        }
        ATLVERIFY(m_website.SubclassWindow(GetDlgItem(IDC_STATIC_WEBSITE)));

        ATLVERIFY(m_projectpage.SetLabel(repo));
        ATLVERIFY(m_projectpage.SetHyperLink(repo));
        ATLVERIFY(m_projectpage.SubclassWindow(GetDlgItem(IDC_STATIC_PROJECT_WEBSITE)));

        ATLVERIFY(m_revisionurl.SetLabel(revision));
        ATLVERIFY(m_revisionurl.SetHyperLink(revisionurl));
        ATLVERIFY(m_revisionurl.SubclassWindow(GetDlgItem(IDC_STATIC_REVISION)));

        CString str;
#pragma warning(suppress : 6031)
        ATLVERIFY(str.LoadString(IDS_PROGRAM_DESCRIPTION));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_COPYRIGHT, m_verinfo[_T("LegalCopyright")]));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_DESCRIPTION, str));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_PROGNAME, m_verinfo[_T("ProductName")]));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_PORTIONSCOPYRIGHT, m_verinfo[_T("Portions Copyright")]));

        CString caption;
        caption.Format(IDS_ABOUT, m_verinfo[_T("FileVersion")], sizeof(void*) * 8);
        SetWindowText(caption);

        m_appicon.LoadIcon(IDR_MAINFRAME);
        ATLVERIFY(!m_appicon.IsNull());
        SetIcon(m_appicon);

        NONCLIENTMETRICS ncm = {0};
        ncm.cbSize = sizeof(ncm);

        if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
        {
            LOGFONT logFont;
            memcpy(&logFont, &ncm.lfMessageFont, sizeof(LOGFONT));
            logFont.lfWeight = FW_BOLD;
            HFONT hFont = m_progNameFont.CreateFontIndirect(&logFont);
            if (hFont)
            {
                SendDlgItemMessage(IDC_STATIC_PROGNAME, WM_SETFONT, reinterpret_cast<WPARAM>(hFont));
            }
        }

        // Load the accelerator table corresponding to the dialog's resource ID
        m_Accel.LoadAccelerators(IDD);
        if (!m_Accel.IsNull())
        {
            ATLTRACE2(_T("Loaded accelerator table: %p\n"), HACCEL(m_Accel));
        }
        else
        {
            ATLTRACE2(_T("Failed to load accelerators for about box (last error: %d)\n"), GetLastError());
        }

        return TRUE;
    }

    LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        HWND hStatic = reinterpret_cast<HWND>(lParam);
        HDC hdcStatic = reinterpret_cast<HDC>(wParam);
        switch (::GetDlgCtrlID(hStatic))
        {
        case IDC_STATIC_PORTIONSCOPYRIGHT:
            ::SetTextColor(hdcStatic, RGB(0, 0x40, 0));
            ::SetBkColor(hdcStatic, ::GetSysColor(COLOR_3DFACE));
            return reinterpret_cast<LRESULT>(::GetSysColorBrush(COLOR_3DFACE));
        case IDC_STATIC_PROGNAME:
            ::SetTextColor(hdcStatic, RGB(0x80, 0, 0));
            ::SetBkColor(hdcStatic, ::GetSysColor(COLOR_3DFACE));
            return reinterpret_cast<LRESULT>(::GetSysColorBrush(COLOR_3DFACE));
        default:
            break;
        }
        bHandled = FALSE;
        return FALSE;
    }

    LRESULT OnCopyAboutInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        UNREFERENCED_PARAMETER(wID);
        ATLASSERT(wID == ID_COPY_ABOUT_INFO);
        if (::OpenClipboard(m_hWnd))
        {
            CString str;
            str.AppendFormat(_T("%s %s\r\n\r\n%s\r\n\r\n"), m_verinfo[_T("ProductName")], m_verinfo[_T("FileVersion")], m_verinfo[_T("LegalCopyright")]);
            str.AppendFormat(IDS_PROGRAM_DESCRIPTION);
            str.Append(_T("\r\n\r\n"));
            str.AppendFormat(IDS_MERCURIAL_REVISION, m_verinfo[_T("Mercurial revision")]);
            str.AppendFormat(IDS_MERCURIAL_REVURL, m_verinfo[_T("Mercurial revision URL")]);
            str.Append(_T("\r\n"));
            str.Append(m_verinfo[_T("Portions Copyright")]);
            ATLVERIFY(setClipboardString(str));
            ATLVERIFY(::CloseClipboard());
        }
        return FALSE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
#ifdef _DEBUG
        if (m_bModal)
        {
            EndDialog(wID);
        }
        else
        {
#else
        UNREFERENCED_PARAMETER(wID);
#endif // _DEBUG
            ATLTRACE2(_T("Removing message filter\n"));
            m_loop.RemoveMessageFilter(this);
            ATLTRACE2(_T("Posting WM_QUIT\n"));
            PostMessage(WM_QUIT);
#ifdef _DEBUG
        }
#endif // _DEBUG
        return 0;
    }
};

typedef CWinTraitsOR<WS_TABSTOP | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_INFOTIP, WS_EX_CLIENTEDGE, CControlWinTraits>
    CNtObjectsTreeViewTraits;
typedef CWinTraitsOR<WS_TABSTOP | LVS_SHAREIMAGELISTS | LVS_SINGLESEL, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, CSortListViewCtrlTraits> CNtObjectsListViewTraits;

template <typename T> class CNtObjectsTreeViewT : public CWindowImpl<CNtObjectsTreeViewT<T>, CTreeViewCtrlEx, CNtObjectsTreeViewTraits>
{
    typedef CAtlMap<Directory*, HTREEITEM> CReverseDirTreeMap;
    typedef CWindowImpl<CNtObjectsTreeViewT<T>, CTreeViewCtrlEx, CNtObjectsTreeViewTraits> baseClass;
    using baseClass::StartWindowProc;

  private:
    bool m_bInitialized;
    Directory m_objroot;
    CImageList m_imagelist;
    HWND m_hFrameWnd;
    CReverseDirTreeMap m_reverseLookup;

  public:
    /*lint -save -e446 */
    DECLARE_WND_SUPERCLASS(_T("NtObjectsTreeView"), CTreeViewCtrlEx::GetWndClassName())
    /*lint -restore */

    BEGIN_MSG_MAP(CNtObjectsTreeViewT<T>)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu) //-V1048
    NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnTVItemExpanding)
    NOTIFY_CODE_HANDLER(TVN_GETINFOTIP, OnGetInfoTip)
    NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTVSelChanged)
    DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    CNtObjectsTreeViewT()
        : m_bInitialized(false)
        , m_hFrameWnd(nullptr)
    {
    }

    inline void setFrameWindow(HWND hFrameWnd)
    {
        m_hFrameWnd = hFrameWnd;
    }

    LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CPoint pt, spt;
        ATLVERIFY(::GetCursorPos(&pt));
        ATLTRACE2(_T("Point of click: %d/%d\n"), pt.x, pt.y);
        spt = pt;

        // Find where the user clicked, so we know the item
        ATLVERIFY(this->ScreenToClient(&pt));
        UINT flags = 0;
        CTreeItem ti = this->HitTest(pt, &flags);
        if (!ti.IsNull())
        {
            ATLVERIFY(this->SelectItem(ti));

            if (Directory* dir = reinterpret_cast<Directory*>(ti.GetData()))
            {
                // Load menu resource
                CMenu menu;
                ATLVERIFY(menu.LoadMenu(IDR_POPUP_MENU1));
                ATLASSERT(menu.IsMenu());
                if (menu.IsMenu())
                {
                    // Get the popup menu at index 0 from the menu resource
                    CMenuHandle popup = menu.GetSubMenu(0);
                    ATLASSERT(popup.IsMenu());
                    if (popup.IsMenu())
                    {

                        // Let the user pick
                        int idCmd = popup.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, spt.x, spt.y, this->GetParent(), nullptr);

                        if (idCmd)
                        {
                            switch (idCmd)
                            {
                            case ID_POPUPMENU_COPYNAME:
                            case ID_POPUPMENU_COPYFULLPATH:
                            case ID_POPUPMENU_COPYASCSTRING:
                            case ID_POPUPMENU_COPYSYMLINKTARGET:
                                copyItemToClipboard(this->m_hWnd, idCmd, dir);
                                break;
                            case ID_POPUPMENU_PROPERTIES:
                                ::SendMessage(this->GetParent(), WM_COMMAND, MAKEWPARAM(ID_VIEW_PROPERTIES, 0), reinterpret_cast<LPARAM>(this->m_hWnd));
                                break;
                            default:
                                ATLTRACE2(_T("Chosen command: %i\n"), idCmd);
                                break;
                            }
                        }
                    }
                }
            }
        }

        return FALSE;
    }

    LRESULT OnTVItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        ATLTRACE2(_T("%hs\n"), __func__);
        LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);
        // Don't allow to collapse the root item (which is the one without a pointer to the NtObjMgr::Directory)
        if ((TVE_COLLAPSE == pnmtv->action) && (pnmtv->itemNew.hItem == this->GetRootItem()))
        {
            ATLTRACE2(_T("Root item must not be collapsed\n"));
            return (bHandled = TRUE);
        }
        if (pnmtv->itemNew.state & TVIS_EXPANDEDONCE)
            return 0;
        if (pnmtv->itemNew.cChildren)
        {
#ifdef _DEBUG
            if (Directory* dir = reinterpret_cast<Directory*>(this->GetItemData(pnmtv->itemNew.hItem)))
                ATLTRACE2(_T("Sort children of %s (%p)\n"), dir->fullname().GetString(), pnmtv->itemNew.hItem);
            else
                ATLTRACE2(_T("Sort children of %p\n"), pnmtv->itemNew.hItem);
#endif
            sortTreeItemChildren_(pnmtv->itemNew.hItem);
        }
        return 0;
    }

    LRESULT OnGetInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        LPNMTVGETINFOTIP pnmtv = reinterpret_cast<LPNMTVGETINFOTIP>(pnmh);
        ATLASSERT(nullptr != pnmtv);
        Directory* dir = reinterpret_cast<Directory*>(pnmtv->lParam);
        bHandled = FALSE;
        if (dir)
        {
            CString comment;
            if (comment.LoadString(findComment(dir->fullname())))
            {
                CString tip;
                tip.Preallocate(pnmtv->cchTextMax);
                // Name of the Directory object
                tip.Append(dir->fullname());
                // Line breaks
                tip.Append(_T("\n\n"));
                // Comment
                tip.Append(comment);
                _tcscpy_s(&pnmtv->pszText[0], pnmtv->cchTextMax - 1, tip.GetString());
                bHandled = TRUE;
            }
        }
        return 0;
    }

    LRESULT OnTVSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);
        ATLASSERT(nullptr != pnmtv);
        bHandled = FALSE;
        Directory* dir = reinterpret_cast<Directory*>(pnmtv->itemNew.lParam);
        if (m_bInitialized && dir)
        {
            ATLTRACE2(_T("%hs: %s\n"), __func__, dir->fullname().GetString());
            // Tell the frame to update the listview with the new Directory
            (void)::SendMessage(m_hFrameWnd, WM_VISIT_DIRECTORY, 0, reinterpret_cast<LPARAM>(dir));
            bHandled = TRUE;
        }
        return 0;
    }

    /*lint -save -e1536 */
    Directory& getObjectRoot()
    {
        return m_objroot;
    }
    /*lint -restore */

    bool emptyAndRefill()
    {
        CSuppressRedraw suppressRedraw(this->m_hWnd);
        if (m_objroot.refresh())
        {
            if (::IsWindow(this->m_hWnd) && m_imagelist.IsNull())
            {
                prepareImageList_();
            }
            if (this->DeleteAllItems())
            {
                // Recursively fill the tree
                fill_(nullptr, m_objroot);
                // Sort the tree
                sortTreeItemChildren_(this->GetRootItem());
                (void)this->Expand(this->GetRootItem());
                // Signal that the tree has been initialized
                m_bInitialized = true;
                return m_bInitialized;
            }
        }
        // Something failed
        m_bInitialized = false;
        return m_bInitialized;
    }

    inline void selectRootItem()
    {
        ATLTRACE2(_T("%hs\n"), __func__);
        (void)this->SelectItem(this->GetRootItem());
    }

    inline CTreeItem lookupTreeItem(Directory* dir)
    {
        HTREEITEM ret = nullptr;
        if (m_reverseLookup.Lookup(dir, ret))
        {
            ATLTRACE2(_T("Found %s in treeview\n"), dir->fullname().GetString());
            return CTreeItem(ret, this);
        }
        return nullptr;
    }

    void selectDirectory(Directory* dir, BOOL bIfNotAlreadySelected = FALSE)
    {
        CTreeItem ti(lookupTreeItem(dir));
        if (!ti.IsNull())
        {
            if (bIfNotAlreadySelected)
            {
                if (0 == (TVIS_SELECTED & ti.GetState(TVIS_SELECTED))) // better select always? ...
                {
                    ti.Select();
                }
            }
            else // unconditionally
            {
                ti.Select();
            }
        }
    }

    Directory* getParentFromSelection()
    {
        CTreeItem ti(this->GetSelectedItem());
        ATLASSERT(!ti.IsNull());
        if (!ti.IsNull())
        {
            CTreeItem parent(ti.GetParent());
            if (!parent.IsNull()) // we only want to navigate up if there is a parent ...
            {
                if (Directory* dir = reinterpret_cast<Directory*>(parent.GetData()))
                    return dir;
            }
        }
        return 0;
    }

  private:
    static int CALLBACK TreeViewCompareProc_(LPARAM lparam1, LPARAM lparam2, LPARAM /*lparamSort*/)
    {
        Directory* dir1 = reinterpret_cast<Directory*>(lparam1);
        Directory* dir2 = reinterpret_cast<Directory*>(lparam2);
        ATLASSERT(nullptr != dir1);
        ATLASSERT(nullptr != dir2);
        return wcscmp(dir1->name(), dir2->name());
    }

    // We create our image list dynamically from the icons
    void prepareImageList_()
    {
        ATLTRACE2(_T("Preparing image list for %hs\n"), __func__);
        WORD iconList[] = {IDI_OBJMGR_ROOT, IDI_DIRECTORY, IDI_EMPTY_DIRECTORY};
        ATLVERIFY(m_imagelist.Create(imgListElemWidth, imgListElemHeight, ILC_COLOR32 | ILC_MASK, (int)_countof(iconList), (int)_countof(iconList)));
        for (size_t i = 0; i < _countof(iconList); i++)
        {
            HICON hIcon = static_cast<HICON>(AtlLoadIconImage(MAKEINTRESOURCE(iconList[i]), LR_CREATEDIBSECTION, imgListElemWidth, imgListElemHeight));
            ATLTRACE2(_T("Icon handle: %p\n"), hIcon);
            m_imagelist.AddIcon(hIcon);
        }
        (void)this->SetImageList(m_imagelist);
    }

    void sortTreeItemChildren_(HTREEITEM parent)
    {
        TVSORTCB tvscb = {parent, TreeViewCompareProc_, 0};
        (void)this->SortChildrenCB(&tvscb, TRUE);
    }

    void fill_(HTREEITEM parent, Directory& current)
    {
        if (!parent)
        {
            m_reverseLookup.RemoveAll();
            parent = this->InsertItem(TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_PARAM,
                                      current.fullname(),
                                      0,
                                      0,
                                      0,
                                      0,
                                      reinterpret_cast<LPARAM>(&m_objroot),
                                      TVI_ROOT,
                                      TVI_LAST);
            m_reverseLookup[&m_objroot] = parent;
        }
        for (size_t i = 0; i < current.size(); i++)
        {
            Directory* entry = dynamic_cast<Directory*>(current[i]);
            if (entry)
            {
                const int imgidx = entry->size() ? 1 : 2;
                HTREEITEM curritem = this->InsertItem(TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_PARAM,
                                                      entry->name(),
                                                      imgidx,
                                                      imgidx,
                                                      0,
                                                      0,
                                                      reinterpret_cast<LPARAM>(entry),
                                                      parent,
                                                      TVI_LAST);
                m_reverseLookup[entry] = curritem;
                fill_(curritem, *entry);
            }
        }
    }
};
typedef CNtObjectsTreeViewT<CObjectImageList> CNtObjectsTreeView;

template <typename T> class CNtObjectsListViewT : public CSortListViewCtrlImpl<CNtObjectsListViewT<T>, CListViewCtrl, CNtObjectsListViewTraits>
{
    HWND m_hFrameWnd;
    T& m_imagelist;
    typedef CSortListViewCtrlImpl<CNtObjectsListViewT<T>, CListViewCtrl, CNtObjectsListViewTraits> baseClass;
    using baseClass::StartWindowProc;

  public:
    /*lint -save -e446 */
    DECLARE_WND_SUPERCLASS(_T("NtObjectsListView"), CListViewCtrl::GetWndClassName())
    /*lint -restore */

    BEGIN_MSG_MAP(CNtObjectsListView<T>)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu) //-V1048
    MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnDoubleClick)
    MESSAGE_HANDLER(WM_CHAR, OnChar)
    NOTIFY_CODE_HANDLER(HDN_ITEMCLICKA, OnHeaderItemClick)
    NOTIFY_CODE_HANDLER(HDN_ITEMCLICKW, OnHeaderItemClick)
    NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnLVItemChanged)
    NOTIFY_CODE_HANDLER(LVN_GETINFOTIP, OnGetInfoTip)
    DEFAULT_REFLECTION_HANDLER()
    CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    CNtObjectsListViewT(T& imagelist)
        : baseClass()
        , m_hFrameWnd(nullptr)
        , m_imagelist(imagelist)
    {
        this->m_bSortDescending = false;
    }

    inline void setFrameWindow(HWND hFrameWnd)
    {
        m_hFrameWnd = hFrameWnd;
    }

    void fillFromDirectory(Directory& current)
    {
        CSuppressRedraw suppressRedraw(this->m_hWnd);
        this->DeleteAllItems();
        if (::IsWindow(this->m_hWnd) && !m_imagelist.IsNull())
        {
            (void)this->SetImageList(m_imagelist, LVSIL_SMALL);
        }
        resetAllColumns_(this->m_iSortColumn);
        ATLASSERT(!m_imagelist.IsNull());
        ATLTRACE2(_T("%hs: %s\n"), __func__, current.fullname().GetString());
        // To determine the required width for each column
        int widths[] = {-1, -1, -1};
        for (size_t i = 0; i < current.size(); i++)
        {
            int currItem = this->InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,
                                            static_cast<int>(i),
                                            current[i]->name(),
                                            0,
                                            0,
                                            m_imagelist.IndexByObjType(current[i]),
                                            reinterpret_cast<LPARAM>(current[i]));
            int const nameWidth = this->GetStringWidth(current[i]->name());
            widths[0] = max(nameWidth, widths[0]);
            this->AddItem(currItem, 1, current[i]->type());
            int const typeWidth = this->GetStringWidth(current[i]->type());
            widths[1] = max(typeWidth, widths[1]);
            SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(current[i]);
            if (symlink)
            {
                this->AddItem(currItem, 2, symlink->target());
                int const targetWidth = this->GetStringWidth(symlink->target());
                widths[2] = max(targetWidth, widths[2]);
            }
        }
        CRect rect;
        ATLVERIFY(this->GetWindowRect(&rect));
        int const maxWidths[] = {rect.right - rect.left - 50, 150, 200};
        // Try to set an optimum column width
        for (int i = 0; i < static_cast<int>(_countof(widths)); i++)
        {
            if (-1 != widths[i])
            {
                int currcol = min(widths[i] + imgListElemWidth + (i ? 0 : 0x10), maxWidths[i]);
                ATLVERIFY(this->SetColumnWidth(i, currcol));
            }
        }

        // Sort according to sort column
        ATLVERIFY(SortItems(this->m_iSortColumn, this->m_bSortDescending));
    }

    bool SortItems(int iCol, bool bDescending)
    {
        // Is this still on the default?
        if (-1 == iCol)
        {
            iCol = 0;
        }
        ATLTRACE2(_T("SortItems(%i, %u) [%i, %u]\n"), iCol, bDescending, this->m_iSortColumn, this->m_bSortDescending);
        return baseClass::SortItems(iCol, bDescending);
    }

    LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        // Find where the user clicked, so we know the item
        LVHITTESTINFO lvhtti = {0};
        CPoint pt;

        if (getItemFromHitTest_(lvhtti, pt))
        {
            // If we found an item, select it
            ATLVERIFY(this->SelectItem(lvhtti.iItem));

            LVITEM item = {0};
            item.iItem = lvhtti.iItem;
            item.mask = LVIF_PARAM;
            ATLVERIFY(this->GetItem(&item));
            GenericObject* itemobj = reinterpret_cast<GenericObject*>(item.lParam);

            ATLASSERT(0 == otGeneric);
            ATLASSERT(1 == otSymlink);
            ATLASSERT(2 == otDirectory);

            /*
            Not a coincidence, the ordering of:

                otGeneric
                otSymlink
                otDirectory

            matches the sub menu indexes.
            */
            int submenuidx = itemobj->objtype();
            if (otDirectory == submenuidx)
            {
                if (Directory* dir = dynamic_cast<Directory*>(itemobj))
                {
                    if (!dir->size())
                        submenuidx = otGeneric;
                }
            }

            // Load menu resource
            CMenu menu;
            ATLVERIFY(menu.LoadMenu(IDR_POPUP_MENU1));
            ATLASSERT(menu.IsMenu());
            if (menu.IsMenu())
            {
                // Get the popup menu at index 0 from the menu resource
                CMenuHandle popup = menu.GetSubMenu(submenuidx);
                ATLASSERT(popup.IsMenu());

                if (popup.IsMenu())
                {
                    // Let the user pick
                    int idCmd = popup.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, this->GetParent(), nullptr);

                    if (idCmd)
                    {
                        switch (idCmd)
                        {
                        case ID_POPUPMENU_COPYNAME:
                        case ID_POPUPMENU_COPYFULLPATH:
                        case ID_POPUPMENU_COPYASCSTRING:
                        case ID_POPUPMENU_COPYSYMLINKTARGET:
                            copyItemToClipboard(this->m_hWnd, idCmd, itemobj);
                            break;
                        case ID_POPUPMENU_PROPERTIES:
                            ::SendMessage(this->GetParent(), WM_COMMAND, MAKEWPARAM(ID_VIEW_PROPERTIES, 0), reinterpret_cast<LPARAM>(this->m_hWnd));
                            break;
                        case ID_POPUPMENU_OPENDIRECTORY:
                            if (Directory* dir = dynamic_cast<Directory*>(itemobj))
                            {
                                openDirectory_(dir);
                            }
                            break;
                        default:
                            ATLTRACE2(_T("Chosen command: %i\n"), idCmd);
                            break;
                        }
                    }
                }
            }
        }

        return 0;
    }

    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        return DLGC_WANTALLKEYS | this->DefWindowProc(uMsg, wParam, lParam);
    }

    LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        ATLTRACE2(_T("key code: %u\n"), wParam);
        switch (wParam)
        {
        case VK_RETURN:
            {
                int idx = this->GetNextItem(-1, LVNI_SELECTED);
                ATLTRACE2(_T("%i == GetNextItem(-1, LVNI_SELECTED)\n"), idx);
                if (idx >= 0)
                {
                    ATLASSERT(1 == this->GetSelectedCount());
                    openPropertiesOrDirectory_(idx);
                }
            }
            break;
        case VK_TAB:
            this->GetParent().GetNextDlgTabItem(this->m_hWnd).SetFocus(); // Allows to still use tab to change focus
            return 0;
        case VK_BACK:
            // Tell the frame that the user asked to go one directory back up
            (void)::SendMessage(m_hFrameWnd, WM_DIRECTORY_UP, 0, 0);
            break;
        }
        return this->DefWindowProc(uMsg, wParam, lParam);
    }

    LRESULT OnDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        ATLTRACE2(_T("%hs\n"), __func__);
        // Find where the user clicked, so we know the item
        LVHITTESTINFO lvhtti = {0};
        CPoint pt;

        if (getItemFromHitTest_(lvhtti, pt))
        {
            // If we found an item, select it
            ATLVERIFY(this->SelectItem(lvhtti.iItem));
            openPropertiesOrDirectory_(lvhtti.iItem);
        }
        return 0;
    }

    LRESULT OnHeaderItemClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
    {
        ATLTRACE2(_T("%hs\n"), __func__);
        LPNMHEADER p = reinterpret_cast<LPNMHEADER>(pnmh);
        if (p->iButton == 0)
        {
            int iOld = this->m_iSortColumn;
            bool bDescending = (this->m_iSortColumn == p->iItem) ? !this->m_bSortDescending : false;
            if (SortItems(p->iItem, bDescending))
            {
                this->NotifyParentSortChanged(p->iItem, iOld);
            }
        }
        return 0;
    }

    LRESULT OnLVItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        LPNMLISTVIEW pnmlv = reinterpret_cast<LPNMLISTVIEW>(pnmh);
        ATLASSERT(nullptr != pnmlv);
        bHandled = FALSE;
        if (pnmlv && (pnmlv->uChanged & LVIF_STATE) && (pnmlv->uNewState & LVIS_SELECTED))
        {
            GenericObject* obj = reinterpret_cast<GenericObject*>(pnmlv->lParam);
            ATLASSERT(nullptr != obj);
            if (obj)
            {
                // Tell the frame that this object is now to be considered active
                (void)::SendMessage(m_hFrameWnd, WM_SET_ACTIVE_OBJECT, 0, reinterpret_cast<LPARAM>(obj));
                bHandled = TRUE;
            }
        }
        return 0;
    }

    LRESULT OnGetInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        LPNMLVGETINFOTIP pnmlv = reinterpret_cast<LPNMLVGETINFOTIP>(pnmh);
        ATLASSERT(nullptr != pnmlv);
        bHandled = FALSE;
        if (pnmlv)
        {
            LVITEM item = {0};
            item.iItem = pnmlv->iItem;
            item.mask = LVIF_PARAM;
            ATLVERIFY(this->GetItem(&item));

            GenericObject* obj = reinterpret_cast<GenericObject*>(item.lParam);
            ATLASSERT(nullptr != obj);
            if (obj)
            {
                CString comment;
                if (comment.LoadString(findComment(obj->fullname())))
                {
                    CString tip;
                    tip.Preallocate(pnmlv->cchTextMax);
                    // Name of the object
                    tip.Append(obj->fullname());
                    // Line breaks
                    tip.Append(_T("\n\n"));
                    // Comment
                    tip.Append(comment);
                    _tcscpy_s(&pnmlv->pszText[0], pnmlv->cchTextMax - 1, tip.GetString());
                    bHandled = TRUE;
                }
            }
        }
        return 0;
    }

#if 0
    // If item1 > item2 return 1, if item1 < item2 return -1, else return 0.
    int CompareItemsCustom(LVCompareParam* pItem1, LVCompareParam* pItem2, int iSortCol)
    {
        if (!iSortCol)
        {
            GenericObject* obj1 = reinterpret_cast<GenericObject*>(pItem1->dwItemData);
            GenericObject* obj2 = reinterpret_cast<GenericObject*>(pItem2->dwItemData);
            bool isDir1 = (otDirectory == obj1->objtype());
            bool isDir2 = (otDirectory == obj2->objtype());
            if (isDir1 && isDir2)
                return _tcsicmp(pItem1->pszValue, pItem2->pszValue);
            if (!isDir1 && !isDir2)
                return _tcsicmp(pItem1->pszValue, pItem2->pszValue);
            if (!isDir1 && isDir2)
                return 1;
            if (isDir1 && !isDir2)
                return -1;
        }
        return _tcsicmp(pItem1->pszValue, pItem2->pszValue);
    }
#endif

    void reloadColumnNames()
    {
        if (static_cast<int>(_countof(lvColumnDefaults)) == this->GetColumnCount())
        {
            CString columnName;

            for (int idx = 0; idx < static_cast<int>(_countof(lvColumnDefaults)); idx++)
            {
#pragma warning(suppress : 6031)
                ATLVERIFY(columnName.LoadString(lvColumnDefaults[idx].resId));

                LVCOLUMN lvc = {0};
                lvc.mask = LVCF_TEXT;
                lvc.pszText = const_cast<LPTSTR>(columnName.GetString());
                lvc.cchTextMax = columnName.GetLength() + 1;
                this->SetColumn(idx, &lvc);
            }
        }
    }

  private:
    void openDirectory_(Directory* dir) const
    {
        ATLASSERT(nullptr != dir);
        ATLTRACE2(_T("%hs: %s\n"), __func__, dir->fullname().GetString());
        // Tell the frame to update the listview with the new Directory
        (void)::SendMessage(m_hFrameWnd, WM_SELECT_TREEVIEW_DIRECTORY, 0, reinterpret_cast<LPARAM>(dir));
    }

    void openPropertiesOrDirectory_(int idx) const
    {
        ATLASSERT(idx != -1);
        LVITEM item = {0};
        item.iItem = idx;
        item.mask = LVIF_PARAM;
        ATLVERIFY(this->GetItem(&item));
        GenericObject* itemobj = reinterpret_cast<GenericObject*>(item.lParam);
        if (otDirectory == itemobj->objtype())
        {
            if (Directory* dir = dynamic_cast<Directory*>(itemobj))
            {
                if (dir->size())
                {
                    openDirectory_(dir);
                    return;
                }
                // If it's empty, we'll show the properties instead, the user may still navigate
                // into the empty Directory using the treeview
            }
        }
        ::SendMessage(this->GetParent(), WM_COMMAND, MAKEWPARAM(ID_VIEW_PROPERTIES, 0), reinterpret_cast<LPARAM>(this->m_hWnd));
    }

    bool getItemFromHitTest_(LVHITTESTINFO& lvhtti, CPoint& pt) const
    {
        ATLVERIFY(::GetCursorPos(&pt));
        ATLTRACE2(_T("Point of click: %d/%d\n"), pt.x, pt.y);

        lvhtti.pt = pt;
        ATLVERIFY(this->ScreenToClient(&lvhtti.pt));
        return (-1 != this->HitTest(&lvhtti));
    }

    void deleteAllColumns_()
    {
        ATLTRACE2(_T("Column count: %i\n"), this->GetColumnCount());
        // On older Windows/common control versions it's impossible to remove column 0
        for (int i = 1; i < this->GetColumnCount(); i++)
        {
            ATLVERIFY(this->DeleteColumn(i));
        }
    }

    void resetAllColumns_(int iSortColumn)
    {
        if (this->GetColumnCount() < static_cast<int>(_countof(lvColumnDefaults)))
        {
            deleteAllColumns_();
            CString columnName;
            for (int idx = 0; idx < static_cast<int>(_countof(lvColumnDefaults)); idx++)
            {
#pragma warning(suppress : 6031)
                ATLVERIFY(columnName.LoadString(lvColumnDefaults[idx].resId));
                if (idx > (this->GetColumnCount() - 1))
                {
                    this->InsertColumn(idx, columnName);
                    this->SetColumnSortType(idx, LVCOLSORT_TEXTNOCASE);
                    ATLTRACE2(_T("Inserted column %i with name \"%s\".\n"), idx, columnName.GetString());
                }
                else
                {
                    LVCOLUMN lvc = {0};
                    lvc.mask = LVCF_TEXT;
                    lvc.pszText = const_cast<LPTSTR>(columnName.GetString());
                    lvc.cchTextMax = columnName.GetLength() + 1;
                    this->SetColumn(idx, &lvc);
                    this->SetColumnSortType(idx, LVCOLSORT_TEXTNOCASE);
                }
            }
            this->m_iSortColumn = iSortColumn;
        }
    }
};
typedef CNtObjectsListViewT<CObjectImageList> CNtObjectsListView;

#if FEATURE_FIND_OBJECT
typedef CWinTraitsOR<WS_TABSTOP | LVS_SHAREIMAGELISTS | LVS_SINGLESEL, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, CSortListViewCtrlTraits>
    CNtObjectsFindResultsTraits;

template <typename T> class CNtObjectsFindResultsT : public CSortListViewCtrlImpl<CNtObjectsFindResultsT<T>, CListViewCtrl, CNtObjectsFindResultsTraits>
{
    HWND m_hFrameWnd;
    T& m_imagelist;
    typedef CSortListViewCtrlImpl<CNtObjectsFindResultsT<T>, CListViewCtrl, CNtObjectsFindResultsTraits> baseClass;
    using baseClass::StartWindowProc;

  public:
    /*lint -save -e446 */
    DECLARE_WND_SUPERCLASS(_T("NtObjectsFindResults"), CListViewCtrl::GetWndClassName())
    /*lint -restore */

    BEGIN_MSG_MAP(CNtObjectsFindResultsT)
    DEFAULT_REFLECTION_HANDLER()
    CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    CNtObjectsFindResultsT(T& imagelist)
        : baseClass()
        , m_hFrameWnd(nullptr)
        , m_imagelist(imagelist)
    {
    }

    inline void setFrameWindow(HWND hFrameWnd)
    {
        m_hFrameWnd = hFrameWnd;
    }
};
typedef CNtObjectsFindResultsT<CObjectImageList> CNtObjectsFindResults;

class CSearchPane : public CPaneContainerImpl<CSearchPane>
{
    typedef CPaneContainerImpl<CSearchPane> baseClass;

    UINT const m_strID;

  public:
    DECLARE_WND_CLASS(_T("SearchPane")) //-V1096

    BEGIN_MSG_MAP(CSearchPane)
    DEFAULT_REFLECTION_HANDLER()
    CHAIN_MSG_MAP(baseClass)
    FORWARD_NOTIFICATIONS() //-V1048
    END_MSG_MAP()

    CSearchPane()
        : baseClass()
        , m_strID(ID_OBJSEARCH_CAPTION)
    {
        SetPaneContainerExtendedStyle(PANECNT_VERTICAL | PANECNT_NOCLOSEBUTTON);
    }

    HWND Create(HWND hWndParent, LPCTSTR lpstrTitle = nullptr)
    {
        CString caption;
        if (!lpstrTitle)
        {
            if (caption.LoadString(m_strID))
            {
                lpstrTitle = caption.GetString();
            }
        }
        return baseClass::Create(hWndParent, lpstrTitle);
    }

    void renewVisibleTexts()
    {
        CString caption;
        if (caption.LoadString(m_strID))
        {
            SetTitle(caption);
        }
    }
};

template <typename T> class CNtObjectsFoundSplitterT : public CSplitterWindowImpl<CNtObjectsFoundSplitterT<T>>
{
    typedef CSplitterWindowImpl<CNtObjectsFoundSplitterT<T>> baseClass;
    using baseClass::StartWindowProc;

    T& m_imagelist;
    CNtObjectsFindResultsT<T> m_findresults;
    CSearchPane m_searchPane;
    int m_bSearchSplitPct;
    int m_cxySplitterMin;
    HRESULT(CALLBACK* DllGetVersion)(DLLVERSIONINFO*);

  public:
    DECLARE_WND_CLASS_EX(_T("NtObjectsFoundSplitter"), CS_DBLCLKS, COLOR_INFOBK)

    CNtObjectsFoundSplitterT(T& imagelist)
        : baseClass(false)
        , m_imagelist(imagelist)
        , m_findresults(imagelist)
        , m_searchPane()
        , m_bSearchSplitPct(75)
        , m_cxySplitterMin(-1)
        , DllGetVersion(0)
    {
        HMODULE hShell32 = ::GetModuleHandle(_T("shell32.dll"));
        if (hShell32)
        {
            *(FARPROC*)&DllGetVersion = ::GetProcAddress(hShell32, "DllGetVersion");
        }
    }

    void initialize(HWND hFrameWnd)
    {
        ATLVERIFY(nullptr != m_searchPane.Create(*this));
        ATLVERIFY(::IsWindow(m_searchPane)); //-V530
        ATLVERIFY(nullptr != m_findresults.Create(m_searchPane, ATL::CWindow::rcDefault));
        ATLVERIFY(::IsWindow(m_findresults)); //-V530
        m_searchPane.SetClient(m_findresults);

        if (DllGetVersion)
        {
            DLLVERSIONINFO dllvi = {sizeof(DLLVERSIONINFO), 0, 0, 0, 0};
            if (SUCCEEDED(DllGetVersion(&dllvi)) && dllvi.dwMajorVersion >= 6)
            {
                m_findresults.SetExtendedListViewStyle(CNtObjectsFindResultsTraits::GetWndExStyle(LVS_EX_DOUBLEBUFFER));
            }
            else
            {
                m_findresults.SetExtendedListViewStyle(CNtObjectsFindResultsTraits::GetWndExStyle(0));
            }
        }
        m_findresults.setFrameWindow(hFrameWnd); // make the frame window known to the listview control for found objects
    }

    void setTopPane(HWND hTopPane, bool bUpdate = true)
    {
        this->SetSplitterPanes(hTopPane, m_searchPane, bUpdate);
    }

    void setDefaultHidden()
    {
        this->SetSplitterPosPct(100);
        ATLVERIFY(this->SetSinglePaneMode(SPLIT_PANE_TOP));
    }

    void toggleHidden()
    {
        ATLTRACE2(_T("Single pane mode before toggling search pane: %d\n"), this->GetSinglePaneMode());
        switch (this->GetSinglePaneMode())
        {
        case SPLIT_PANE_NONE:
            m_bSearchSplitPct = this->GetSplitterPosPct(); // save old splitter position percentage
            this->SetSinglePaneMode(SPLIT_PANE_TOP);
            this->m_cxyMin = m_cxySplitterMin; // reset to default
            this->SetSplitterPosPct(100);
            break;
        case SPLIT_PANE_TOP:
            this->SetSinglePaneMode(SPLIT_PANE_NONE);
            if (m_cxySplitterMin < 0)
                m_cxySplitterMin = this->m_cxyMin;
            this->m_cxyMin = 100;
            this->SetSplitterPosPct(m_bSearchSplitPct);
            break;
        case SPLIT_PANE_BOTTOM:
            ATLASSERT(!_T("Not expecting SPLIT_PANE_BOTTOM here."));
            break;
        }
        ATLTRACE2(_T("Pane position now at %i %%\n"), baseClass::GetSplitterPosPct());
    }

    void renewVisibleTexts()
    {
        m_searchPane.renewVisibleTexts();
    }

  private:
    CNtObjectsFoundSplitterT(); // hide
};
typedef CNtObjectsFoundSplitterT<CObjectImageList> CNtObjectsFoundSplitter;
#endif // FEATURE_FIND_OBJECT

class CNtObjectsStatusBar : public CMultiPaneStatusBarCtrlImpl<CNtObjectsStatusBar>
{
  public:
    /*lint -save -e446 */
    DECLARE_WND_SUPERCLASS(_T("NtObjectsStatusBar"), GetWndClassName()) //-V1096
    /*lint -restore */

    void initializePanes(bool bIsAdmin, bool bIsElevated, bool bSetText = true)
    {
        ATLASSERT(!IsSimple());
        ATLASSERT(m_nPanes == 0);
        int panes[] = {
            ID_DEFAULT_PANE, bIsAdmin ? IDS_STATUSBAR_ADMIN : IDS_STATUSBAR_NOTADMIN, bIsElevated ? IDS_STATUSBAR_ELEVATED : IDS_STATUSBAR_NOTELEVATED};
        ATLVERIFY(SetPanes(panes, _countof(panes), bSetText));
    }

    void reloadPaneText(int idx)
    {
        ATLVERIFY(idx < m_nPanes);
        if (idx < m_nPanes)
        {
            CString resStr;
#pragma warning(suppress : 6031)
            ATLVERIFY(resStr.LoadString(m_pPane[idx]));
            ATLVERIFY(SetPaneText(m_pPane[idx], resStr));
        }
    }
};

class CNtObjectsSplitter : public CSplitterWindowImpl<CNtObjectsSplitter>
{
    typedef CSplitterWindowImpl<CNtObjectsSplitter> baseClass;

  public:
    DECLARE_WND_CLASS_EX(_T("NtObjectsSplitter"), CS_DBLCLKS, COLOR_INFOBK) //-V1096

    CNtObjectsSplitter()
        : baseClass(true)
    {
    }
};

template <typename T> class CVisitedListT : protected CSimpleArray<T>
{
    typedef CSimpleArray<T> baseClass;
    int const m_maxDepth;
    int m_cursorIdx;

    CVisitedListT(const CSimpleArray<T>& src);

  public:
    CVisitedListT(int const maxDepth = maxVisitedDepth)
        : baseClass()
        , m_maxDepth(maxDepth)
        , m_cursorIdx(-1)
    {
    }

    /*lint -save -e1509*/
    ~CVisitedListT() //-V832
    {
    }
    /*lint -restore*/

    inline T navigate(bool forward)
    {
        ATLASSERT(m_cursorIdx < 0);
        if (forward)
        {
            if (m_cursorIdx + 1 == 0)
            {
                ATLTRACE2(_T("Navigating forward doesn't work. Cursor already at the end of the list: %i (size == %i).\n"), m_cursorIdx, getSize());
                return nullptr; // must be able to convert to NULL
            }
            m_cursorIdx = -1;
        }
        else
        {
            if (getSize() + m_cursorIdx - 1 < 0)
            {
                ATLTRACE2(_T("Navigating backward doesn't work. Cursor already at the start of the list: %i (size == %i).\n"), m_cursorIdx, getSize());
                return nullptr; // must be able to convert to NULL
            }
            m_cursorIdx--;
        }
#ifdef _DEBUG
        for (int i = 0; i < getSize(); i++)
        {
            T item = baseClass::operator[](i);
            ATLASSERT(nullptr != item);
            if (i == getSize() + m_cursorIdx)
            {
                ATLTRACE2(_T("->  [%i] %s\n"), i, item->fullname().GetString());
            }
            else
            {
                ATLTRACE2(_T("    [%i] %s\n"), i, item->fullname().GetString());
            }
        }
#endif // _DEBUG
        ATLASSERT(getSize() + m_cursorIdx >= 0);
        ATLASSERT(getSize() + m_cursorIdx < getSize());
        ATLTRACE2(_T("New cursor position: %i (size == %i).\n"), m_cursorIdx, getSize());
        return baseClass::operator[](getSize() + m_cursorIdx);
    }

    inline BOOL push(T t)
    {
        int const nLast = getSize() - 1;
        if (nLast >= 0)
        {
            if (baseClass::operator[](nLast) == t)
            {
                return TRUE; // do not add dupes if the previous entry contained the same item
            }
        }
        // Trim the list to maximum depth
        while (getSize() >= m_maxDepth)
        {
            (void)shift();
        }
        BOOL retval = baseClass::Add(t);
        m_cursorIdx = -1;
#ifdef _DEBUG
        for (int i = 0; i < getSize(); i++)
        {
            T item = baseClass::operator[](i);
            ATLASSERT(nullptr != item);
            if (i == getSize() + m_cursorIdx)
            {
                ATLTRACE2(_T("->  [%i] %s\n"), i, item->fullname().GetString());
            }
            else
            {
                ATLTRACE2(_T("    [%i] %s\n"), i, item->fullname().GetString());
            }
        }
#endif // _DEBUG
        return retval;
    }

    inline T pop()
    {
        int const nLast = getSize() - 1;
        if (nLast < 0)
            return nullptr; // must be able to convert to NULL
        T t = baseClass::m_aT[nLast];
        if (!baseClass::RemoveAt(nLast))
            return nullptr;
        return t;
    }

    inline T shift()
    {
        if (getSize() <= 0)
            return nullptr; // must be able to convert to NULL
        T t = baseClass::m_aT[0];
        if (!baseClass::RemoveAt(0))
            return nullptr;
        return t;
    }

    inline const T& operator[](int nIndex) const
    {
        return baseClass::operator[](nIndex);
    }

    inline T& operator[](int nIndex)
    {
        return baseClass::operator[](nIndex);
    }

    inline void resetList()
    {
        baseClass::RemoveAll();
        m_cursorIdx = -1;
    }

    inline int getSize() const
    {
        return baseClass::GetSize();
    }

    inline operator bool() const
    {
        return (0 < baseClass::GetSize());
    }

    inline bool operator!() const
    {
        return !operator bool();
    }
};

/*lint -esym(1509, CNtObjectsMainFrame) */
class CNtObjectsMainFrame : public CFrameWindowImpl<CNtObjectsMainFrame>, public CUpdateUI<CNtObjectsMainFrame>, public CMessageFilter, public CIdleHandler
{
    typedef CFrameWindowImpl<CNtObjectsMainFrame> baseClass;

  public:
    DECLARE_FRAME_WND_CLASS(_T("NtObjectsMainFrame"), IDR_MAINFRAME) //-V1096

    CLanguageSetter& m_langSetter;
    CObjectImageList m_imagelist;
    CNtObjectsStatusBar m_status;
    CNtObjectsSplitter m_splitObjTree;
    CNtObjectsTreeView m_treeview;
    CNtObjectsListView m_listview;
#if FEATURE_FIND_OBJECT
    CNtObjectsFoundSplitter m_splitFind;
#endif // FEATURE_FIND_OBJECT
    bool m_bFirstOnIdle;
    GenericObject* m_activeObject;
    CVersionInfo m_verinfo;
    CVisitedListT<Directory*> m_visitedList;
    HRESULT(CALLBACK* DllGetVersion)(DLLVERSIONINFO*);
    CAccessToken m_Token;
    const bool m_bIsAdmin;
    const bool m_bIsElevated;
    LANGID m_currentLang;
    OSVERSIONINFOEXW const& m_osvix;

    CNtObjectsMainFrame(OSVERSIONINFOEXW const& osvix)
        : m_langSetter(gLangSetter)
        , m_imagelist()
        , m_treeview()
        , m_listview(m_imagelist)
#if FEATURE_FIND_OBJECT
        , m_splitFind(m_imagelist)
#endif                         // FEATURE_FIND_OBJECT
        , m_bFirstOnIdle(true) // to force initial refresh
        , m_activeObject(0)
        , m_verinfo(ModuleHelper::GetResourceInstance())
        , DllGetVersion(0)
        , m_bIsAdmin(isUserAdmin() != FALSE)
        , m_bIsElevated(isElevated() != FALSE)
        , m_currentLang(m_langSetter.set())
        , m_osvix(osvix)
    {
        HMODULE hShell32 = ::GetModuleHandle(_T("shell32.dll"));
        if (hShell32)
        {
            *(FARPROC*)&DllGetVersion = ::GetProcAddress(hShell32, "DllGetVersion");
        }
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        if (CFrameWindowImpl<CNtObjectsMainFrame>::PreTranslateMessage(pMsg))
            return TRUE;
        if (IsDialogMessage(pMsg))
            return TRUE;
        return FALSE;
    }

    BEGIN_UPDATE_UI_MAP(CNtObjectsMainFrame) //-V1096
    UPDATE_ELEMENT(ID_SWITCHLANGUAGE_ENGLISH, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_SWITCHLANGUAGE_GERMAN, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CNtObjectsMainFrame)
    CHAIN_MSG_MAP_MEMBER(m_treeview)
    CHAIN_MSG_MAP_MEMBER(m_listview)
    MESSAGE_HANDLER(WM_CREATE, OnCreate) //-V1048
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
    MESSAGE_HANDLER(WM_APPCOMMAND, OnAppCommand)
    MESSAGE_HANDLER(WM_VISIT_DIRECTORY, OnVisitDirectory)
    MESSAGE_HANDLER(WM_SELECT_TREEVIEW_DIRECTORY, OnSelectTreeviewDirectory)
    MESSAGE_HANDLER(WM_SET_ACTIVE_OBJECT, OnSetActiveObject)
    MESSAGE_HANDLER(WM_DIRECTORY_UP, OnDirectoryUp)
    MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
    COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
    COMMAND_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)
    COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
    COMMAND_ID_HANDLER(ID_SHOW_ABOUT, OnShowAbout)
    COMMAND_ID_HANDLER(ID_SHOW_ONLINEHELP, OnShowOnlineHelp)
    COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnSaveAs)
#if FEATURE_FIND_OBJECT
    COMMAND_ID_HANDLER(ID_VIEW_FIND, OnFindObject)
#endif // FEATURE_FIND_OBJECT
    COMMAND_ID_HANDLER(ID_SWITCHLANGUAGE_ENGLISH, OnSwitchLanguage)
    COMMAND_ID_HANDLER(ID_SWITCHLANGUAGE_GERMAN, OnSwitchLanguage)
    COMMAND_ID_HANDLER(ID_SWITCHLANGUAGE_POPUP, OnSwitchLanguage)
    CHAIN_MSG_MAP(CUpdateUI<CNtObjectsMainFrame>)
    CHAIN_MSG_MAP(CFrameWindowImpl<CNtObjectsMainFrame>)
    DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        ATLVERIFY(nullptr != (m_hWndStatusBar = createStatusBar_()));
#if FEATURE_FIND_OBJECT
        ATLVERIFY(nullptr != (m_hWndClient = createSplitter_(m_splitFind, m_hWnd)));
        ATLVERIFY(nullptr != createSplitter_(m_splitObjTree, m_splitFind));
#else
        ATLVERIFY(nullptr != (m_hWndClient = createSplitter_(m_splitObjTree, m_hWnd)));
#endif // FEATURE_FIND_OBJECT

        ATLVERIFY(nullptr != m_treeview.Create(m_splitObjTree));
        ATLVERIFY(nullptr != m_listview.Create(m_splitObjTree));

        ATLASSERT(::IsWindow(m_hWndStatusBar));
#if FEATURE_FIND_OBJECT
        m_splitFind.initialize(m_hWnd);
        ATLASSERT(::IsWindow(m_splitFind) && (m_hWndClient == m_splitFind));
        ATLASSERT(::IsWindow(m_splitObjTree));
#else
        ATLASSERT(::IsWindow(m_splitObjTree) && (m_hWndClient == m_splitObjTree));
#endif // FEATURE_FIND_OBJECT
        ATLASSERT(::IsWindow(m_treeview));
        ATLASSERT(::IsWindow(m_listview));

        ATLTRACE2(_T("Control ID for vertical splitter: %i\n"), ::GetDlgCtrlID(m_splitObjTree));
        ATLTRACE2(_T("Control ID for treeview: %i\n"), ::GetDlgCtrlID(m_treeview));
        ATLTRACE2(_T("Control ID for listview: %i\n"), ::GetDlgCtrlID(m_listview));
        ATLASSERT(m_treeview == ::GetDlgItem(m_splitObjTree, ::GetDlgCtrlID(m_treeview)));
        ATLASSERT(m_listview == ::GetDlgItem(m_splitObjTree, ::GetDlgCtrlID(m_listview)));
#if FEATURE_FIND_OBJECT
        ATLTRACE2(_T("Control ID for horizontal splitter: %i\n"), ::GetDlgCtrlID(m_splitFind));
        ATLASSERT(m_splitFind == ::GetDlgItem(m_hWnd, ::GetDlgCtrlID(m_splitFind)));
        ATLASSERT(m_splitObjTree == ::GetDlgItem(m_splitFind, ::GetDlgCtrlID(m_splitObjTree)));
#else
        ATLASSERT(m_splitObjTree == ::GetDlgItem(m_hWnd, ::GetDlgCtrlID(m_splitObjTree)));
#endif // FEATURE_FIND_OBJECT

        setFrameWindow_();
        enableShell60Features_();
        renewAboutInSystemMenu_(TRUE);

        ATLVERIFY(m_splitObjTree.SetSplitterPane(SPLIT_PANE_LEFT, m_treeview));
        ATLVERIFY(m_splitObjTree.SetSplitterPane(SPLIT_PANE_RIGHT, m_listview));

        UpdateLayout();

        m_splitObjTree.SetSplitterPosPct(20);
        m_splitObjTree.m_cxyMin = 180; //  minimum width of the treeview

        (void)updateMainFormLanguage(m_currentLang);

        // register object for message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(nullptr != pLoop);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);

        return 0;
    }

    void UpdateLayout(BOOL bResizeBars = TRUE)
    {
#if FEATURE_FIND_OBJECT
        m_splitFind.setTopPane(m_splitObjTree);
#endif // FEATURE_FIND_OBJECT

        baseClass::UpdateLayout(bResizeBars);

#if FEATURE_FIND_OBJECT
        m_splitFind.setDefaultHidden();
#endif // FEATURE_FIND_OBJECT
    }

    virtual BOOL OnIdle()
    {
        if (m_bFirstOnIdle)
        {
            m_bFirstOnIdle = false;
            // Force initial refresh
            PostMessage(WM_COMMAND, ID_VIEW_REFRESH, 0);
        }
        return FALSE;
    }

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        // unregister message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(nullptr != pLoop);
        pLoop->RemoveMessageFilter(this);
        pLoop->RemoveIdleHandler(this);

        bHandled = FALSE;
        return 1;
    }

    LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL&)
    {
        LPMINMAXINFO lpMMI = reinterpret_cast<LPMINMAXINFO>(lParam);

        lpMMI->ptMinTrackSize.x = 600;
        lpMMI->ptMinTrackSize.y = 400;
        lpMMI->ptMaxTrackSize.x = ::GetSystemMetrics(SM_CXMAXTRACK);
        lpMMI->ptMaxTrackSize.y = ::GetSystemMetrics(SM_CYMAXTRACK);

        return 0;
    }

    LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CWaitCursor wc;
        if (m_treeview.emptyAndRefill())
        {
            // Invalidate the cached items
            m_visitedList.resetList();
            // The following triggers TVN_SELCHANGED which in turn fills the listview
            m_treeview.selectRootItem();
        }
        return 0;
    }

    LRESULT OnShowAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CAboutDlg dlg(m_verinfo);
        dlg.EmulateModal(m_hWnd);
        return 0;
    }

    LRESULT OnShowOnlineHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        SHELLEXECUTEINFO sei = {0};
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_FLAG_NO_UI;
        sei.lpVerb = _T("open");
        CString url;
#pragma warning(suppress : 6031)
        ATLVERIFY(url.LoadString(IDS_URL_ONLINEHELP));
        sei.lpFile = url.GetString();
        ATLVERIFY(::ShellExecuteEx(&sei));
        return 0;
    }

    LRESULT OnSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
#ifdef NTOBJX_NO_XML_EXPORT
        LPCTSTR lpmszFilter = _T("Text files\0*.txt\0All files\0*.*\0\0");
#else
        LPCTSTR lpmszFilter = _T("Text files\0*.txt\0XML files\0*.xml\0All files\0*.*\0\0");
#endif // !NTOBJX_NO_XML_EXPORT
        LPCTSTR lpszDefExt = _T("txt");
        LPCTSTR lpszFileName = _T("ntobjx.txt");
        DWORD const dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
        CFileDialog dlg(FALSE, lpszDefExt, lpszFileName, dwFlags, lpmszFilter);
        if (IDOK == dlg.DoModal(m_hWnd))
        {
            ATLTRACE2(_T("The picked file is: %s\n"), dlg.m_szFileName);
            saveAs_(dlg.m_szFileName, m_treeview.getObjectRoot());
        }
        return 0;
    }

#if FEATURE_FIND_OBJECT
    LRESULT OnFindObject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_splitFind.toggleHidden();
        return 0;
    }
#endif // FEATURE_FIND_OBJECT

    LANGID updateMainFormLanguage(LANGID langID, BOOL bRefreshAll = FALSE)
    {
        if (m_currentLang != langID)
        {
            m_currentLang = m_langSetter.set(langID);
        }
        ATLASSERT(ID_SWITCHLANGUAGE_GERMAN == MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN));
        ATLASSERT(ID_SWITCHLANGUAGE_ENGLISH == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));

        setWindowTitle_();

        renewMainMenuAndAccelerators_();
        m_listview.reloadColumnNames();
        renewStatusBar_();
        renewAboutInSystemMenu_();
#if FEATURE_FIND_OBJECT
        m_splitFind.renewVisibleTexts();
#endif // FEATURE_FIND_OBJECT

        switch (m_currentLang)
        {
        case ID_SWITCHLANGUAGE_ENGLISH:
            UISetCheck(ID_SWITCHLANGUAGE_ENGLISH, 1, bRefreshAll);
            UISetCheck(ID_SWITCHLANGUAGE_GERMAN, 0, bRefreshAll);
            break;
        case ID_SWITCHLANGUAGE_GERMAN:
            UISetCheck(ID_SWITCHLANGUAGE_GERMAN, 1, bRefreshAll);
            UISetCheck(ID_SWITCHLANGUAGE_ENGLISH, 0, bRefreshAll);
            break;
        default:
            ATLASSERT(!"This should never happen");
            break;
        }
        return m_currentLang;
    }

    void showSwitchLanguagePopup()
    {
        CPoint pt, spt;
        ATLVERIFY(::GetCursorPos(&pt));
        ATLTRACE2(_T("Point of click: %d/%d\n"), pt.x, pt.y);
        spt = pt;

        // Load menu resource
        CMenu menu;
        ATLVERIFY(menu.LoadMenu(IDR_MAINFRAME));
        ATLASSERT(menu.IsMenu());

        if (menu.IsMenu())
        {
            ATLTRACE2(_T("%hs: Main menu items %i\n"), __func__, menu.GetMenuItemCount());
#ifdef _DEBUG
            dumpMenuItems_(menu);
#endif // _DEBUG
       // Get the popup menu at index 0 from the menu resource
            CMenuHandle viewmenu = menu.GetSubMenu(1);
            ATLASSERT(viewmenu.IsMenu());

            if (viewmenu.IsMenu())
            {
                ATLTRACE2(_T("%hs: View menu items %i\n"), __func__, viewmenu.GetMenuItemCount());
#ifdef _DEBUG
                dumpMenuItems_(viewmenu);
#endif // _DEBUG
                CMenuHandle popup = viewmenu.GetSubMenu(viewmenu.GetMenuItemCount() - 1);
                ATLTRACE2(_T("%hs: %i == Switch language menu item\n"), __func__, viewmenu.GetMenuItemCount() - 1);
                ATLASSERT(popup.IsMenu());

                if (popup.IsMenu())
                {
                    ATLTRACE2(_T("%hs: Switch Language menu items %i\n"), __func__, popup.GetMenuItemCount());
#ifdef _DEBUG
                    dumpMenuItems_(popup);
#endif // _DEBUG

                    // Let the user pick
                    int idCmd = popup.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, spt.x, spt.y, m_hWnd, nullptr);

                    if (idCmd)
                    {
                        switch (idCmd)
                        {
                        case ID_SWITCHLANGUAGE_ENGLISH:
                        case ID_SWITCHLANGUAGE_GERMAN:
                            ATLTRACE2(_T("Picked language %i\n"), idCmd);
                            if (static_cast<LANGID>(idCmd) != m_currentLang)
                            {
                                SendMessage(WM_COMMAND, static_cast<WPARAM>(idCmd));
                            }
                            break;
                        default:
                            ATLTRACE2(_T("Oi! That language isn't implemented in the GUI!\n"));
                            ATLASSERT(FALSE);
                            break;
                        }
                    }
#ifdef _DEBUG
                    else
                    {
                        ATLTRACE2(_T("TrackPopupMenuEx failed with error %li\n"), GetLastError());
                    }
#endif // _DEBUG
                }
            }
        }
    }

    LRESULT OnSwitchLanguage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        switch (wID)
        {
        case ID_SWITCHLANGUAGE_GERMAN:
        case ID_SWITCHLANGUAGE_ENGLISH:
            (void)updateMainFormLanguage(wID, TRUE);
            break;
        case ID_SWITCHLANGUAGE_POPUP:
            showSwitchLanguagePopup();
            break;
        default:
            ATLTRACE2(_T("Oi! That language isn't implemented in the GUI!\n"));
            ATLASSERT(FALSE);
            break;
        }
        return 0;
    }

    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        ATLTRACE2(_T("WM_SYSCOMMAND lParam: %lu\n"), wParam);
        if (IDS_ABOUT_DESCRIPTION == wParam)
            (void)OnShowAbout(0, 0, 0, bHandled);
        else
            bHandled = FALSE;
        return 0;
    }

    LRESULT OnAppCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        SHORT cmd = GET_APPCOMMAND_LPARAM(lParam);
        if (m_visitedList)
        {
            switch (cmd)
            {
            case APPCOMMAND_BROWSER_BACKWARD:
            case APPCOMMAND_MEDIA_PREVIOUSTRACK:
                bHandled = navigate_(false);
                break;
            case APPCOMMAND_BROWSER_FORWARD:
            case APPCOMMAND_MEDIA_NEXTTRACK:
                bHandled = navigate_(true);
                break;
            }
        }

        return 0;
    }

    // lParam contains a Directory*
    LRESULT OnVisitDirectory(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        ATLASSERT(lParam != 0);
        if (Directory* dir = reinterpret_cast<Directory*>(lParam))
        {
            ATLTRACE2(_T("%hs: %s\n"), __func__, dir->fullname().GetString());
            visitDirectory_(dir);
            setActiveObject_(dir);
        }
        return 0;
    }

    // lParam contains a Directory*
    LRESULT OnSelectTreeviewDirectory(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        ATLASSERT(lParam != 0);
        if (Directory* dir = reinterpret_cast<Directory*>(lParam))
        {
            ATLTRACE2(_T("%hs: %s\n"), __func__, dir->fullname().GetString());
            m_treeview.selectDirectory(dir);
        }
        return 0;
    }

    inline LRESULT OnSetActiveObject(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        if (GenericObject* obj = reinterpret_cast<GenericObject*>(lParam))
        {
            setActiveObject_(obj);
        }
        return 0;
    }

    inline LRESULT OnDirectoryUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        if (Directory* dir = m_treeview.getParentFromSelection())
        {
            ATLTRACE2(_T("%hs: %s\n"), __func__, dir->fullname().GetString());
            m_treeview.selectDirectory(dir);
        }
        return 0;
    }

    LRESULT OnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        WORD wID = LOWORD(wParam);
        if ((MF_SYSMENU & HIWORD(wParam)) && (IDS_ABOUT_DESCRIPTION == wID))
        {
            const int cchBuff = 256;
            TCHAR szBuff[cchBuff] = {0};

            int nRet = AtlLoadString(wID, szBuff, cchBuff);
            for (int i = 0; i < nRet; i++)
            {
                if (szBuff[i] == _T('\n'))
                {
                    szBuff[i] = 0;
                    break;
                }
            }
            if (::IsWindow(m_status))
            {
                ::SendMessage(m_status, SB_SIMPLE, TRUE, 0L);
                ::SendMessage(m_status, SB_SETTEXT, (255 | SBT_NOBORDERS), (LPARAM)szBuff);
            }
        }
        else
            bHandled = FALSE;
        return 0;
    }

    inline LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        PostMessage(WM_CLOSE);
        return 0;
    }

    LRESULT OnViewProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
    {
#ifndef _DEBUG
        UNREFERENCED_PARAMETER(wNotifyCode);
        UNREFERENCED_PARAMETER(wID);
        UNREFERENCED_PARAMETER(hWndCtl);
#endif
        ATLTRACE2(_T("hWndCtl = %p; wNotifyCode = %u (%04X); wID = %u (%04X)\n"), hWndCtl, wNotifyCode, wNotifyCode, wID, wID);
#ifdef _DEBUG
        if (m_activeObject)
        {
            ATLTRACE2(_T("Last selected item: %s\n"), m_activeObject->fullname().GetString());
        }
        if (hWndCtl == m_listview)
        {
            ATLTRACE2(_T("From ListView\n"));
        }
        else if (hWndCtl == m_treeview)
        {
            ATLTRACE2(_T("From TreeView\n"));
        }
        else
        {
            ATLTRACE2(_T("From main menu or hotkey\n"));
        }
#endif

        if (m_activeObject)
        {
#if FEATURE_OBJECT_SECURITY
            // First time the user wants to see the properties (and security settings), we enable two privileges
            if (!m_Token.GetHandle())
            {
                if (m_Token.GetThreadToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY) || m_Token.GetProcessToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY))
                {
                    bool bNotAllAssigned = false;
                    ATLTRACE2(_T("Opened thread or process token: %p\n"), m_Token.GetHandle());
                    LPCTSTR requiredPrivileges[] = {SE_SECURITY_NAME, SE_TAKE_OWNERSHIP_NAME};
                    for (size_t i = 0; i < _countof(requiredPrivileges); i++)
                    {
                        if (m_Token.EnablePrivilege(requiredPrivileges[i], nullptr, &bNotAllAssigned))
                        {
                            ATLTRACE2(_T("%s %s enabled (%d)\n"),
                                      requiredPrivileges[i],
                                      (bNotAllAssigned) ? _T("could not be") : _T("was successfully"),
                                      GetLastError());
                        }
                    }
                }
            }
#endif // FEATURE_OBJECT_SECURITY
            ATLTRACE2(_T("last selected = %p -> %s\n"), m_activeObject, m_activeObject->fullname().GetString());
            CObjectPropertySheet objprop(m_activeObject, m_imagelist);
            (void)objprop.DoModal(m_hWnd);
        }

        return 0;
    }

  private:
#ifdef _DEBUG
    template <typename T> void dumpMenuItems_(T& menu) const
    {
        ATLTRACE2(_T("%hs: ======================================\n"), __func__);
        for (int i = 0; i < menu.GetMenuItemCount(); i++)
        {
            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_FTYPE;
            ATLVERIFY(menu.GetMenuItemInfo(static_cast<UINT>(i), TRUE, &mii));
            if (mii.fType == MFT_SEPARATOR)
            {
                ATLTRACE2(_T("%hs: %i -> --------------[Separator]-------------\n"), __func__, i);
            }
            if (mii.fType == MFT_STRING)
            {
                CString s;
                mii.fMask = MIIM_STRING;
                mii.dwTypeData = s.GetBufferSetLength(MAX_PATH);
                mii.cch = MAX_PATH;
                if (menu.GetMenuItemInfo(static_cast<UINT>(i), TRUE, &mii))
                {
                    ATLTRACE2(_T("%hs: %i -> %s\n"), __func__, i, mii.dwTypeData);
                }
                else
                {
                    ATLTRACE2(_T("%hs: %i -> <error>\n"), __func__, i);
                }
            }
        }
    }
#endif // _DEBUG

    inline void disableSwitchLanguageMenuItem_()
    {
        CMenuHandle mainmenu = GetMenu();
        if (mainmenu.IsMenu())
        {
            CMenuHandle viewmenu = mainmenu.GetSubMenu(1);
            if (viewmenu.IsMenu())
            {
                int const idx = viewmenu.GetMenuItemCount() - 1;
                MENUITEMINFO mii = {0};
                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_STATE;
                if (viewmenu.GetMenuItemInfo(static_cast<UINT>(idx), TRUE, &mii))
                {
                    mii.fState |= MFS_DISABLED;
                    ATLVERIFY(viewmenu.SetMenuItemInfo(static_cast<UINT>(idx), TRUE, &mii));
                }
            }
        }
    }

    inline void renewAboutInSystemMenu_(BOOL bInitial = FALSE)
    {
        CMenuHandle sysmenu(GetSystemMenu(FALSE));
        if (sysmenu.IsMenu())
        {
            CString menuString;
            if (menuString.LoadString(IDS_ABOUT_MENUITEM))
            {
                if (bInitial) // append, not just set the text
                {
                    ATLVERIFY(sysmenu.AppendMenu(MF_SEPARATOR));
                    ATLVERIFY(sysmenu.AppendMenu(MF_STRING, IDS_ABOUT_DESCRIPTION, menuString));
                }
                else // simply set the text
                {
                    MENUITEMINFO mii = {0};
                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_STRING;
                    mii.dwTypeData = const_cast<LPTSTR>(menuString.GetString());
                    mii.cch = menuString.GetLength();

                    ATLVERIFY(sysmenu.SetMenuItemInfo(IDS_ABOUT_DESCRIPTION, FALSE, &mii));
                }
            }
        }
    }

    inline void renewStatusBar_()
    {
        if (::IsWindow(m_status))
        {
            ATLASSERT(m_status.m_nPanes == 3); // in case I ever change this, that's the tripping wire to notice
            m_status.reloadPaneText(1);
            m_status.reloadPaneText(2);

            if (m_activeObject)
            {
                setStatusBarItem_(m_activeObject);
            }
        }
    }

    inline void renewMainMenuAndAccelerators_()
    {
        HMENU hOldMenu = GetMenu();
        ATLASSERT(hOldMenu != nullptr);
        HMENU hNewMenu = AtlLoadMenu(GetWndClassInfo().m_uCommonResourceID);
        ATLASSERT(hNewMenu != nullptr);
        if (hNewMenu != nullptr)
        {
            ATLVERIFY(SetMenu(hNewMenu));
            ATLVERIFY(::DestroyMenu(hOldMenu));
        }
        if (m_hAccel)
        {
            HACCEL hOldAccel = m_hAccel;
            HACCEL hNewAccel = AtlLoadAccelerators(GetWndClassInfo().m_uCommonResourceID);
            ATLASSERT(hNewAccel != nullptr);
            if (hNewAccel)
            {
                ATLTRACE2(_T("Loaded accelerator table: %p\n"), hNewAccel);
                m_hAccel = hNewAccel;
                ATLVERIFY(::DestroyAcceleratorTable(hOldAccel));
            }
        }
    }

    inline void setWindowTitle_()
    {
        CString oldWndTitle, newDlgTitle;
#pragma warning(suppress : 6031)
        ATLVERIFY(oldWndTitle.LoadString(IDR_MAINFRAME));
        ATLTRACE2(_T("Old title: %s\n"), oldWndTitle.GetString());
        newDlgTitle.Format(IDS_TITLEBAR_FMTSTR, oldWndTitle.GetString(), m_verinfo[_T("FileVersion")], sizeof(void*) * 8);
        ATLTRACE2(_T("New title: %s\n"), newDlgTitle.GetString());
        ATLVERIFY(SetWindowText(newDlgTitle.GetString()));
    }

    inline void setActiveObject_(GenericObject* obj)
    {
        m_activeObject = obj;
        setStatusBarItem_(obj);
    }

    void visitDirectory_(Directory* dir)
    {
        ATLASSERT(dir != nullptr);
        if (!dir)
        {
            return;
        }
#ifdef _DEBUG
        LPCWSTR fullName = (dir) ? dir->fullname().GetString() : _T("");
        ATLTRACE2(_T("Visiting Directory: '%s'\n"), fullName);
#endif // _DEBUG
        m_listview.fillFromDirectory(*dir);
        // Keep a list of visited directories
        ATLVERIFY(m_visitedList.push(dir));
    }

    inline BOOL navigate_(bool forward)
    {
        ATLTRACE2(_T("%hs: Navigating %s\n"), __func__, (forward) ? _T("forward") : _T("backward"));
        Directory* dir = static_cast<Directory*>(m_visitedList.navigate(forward));
        if (!dir)
        {
            ATLTRACE2(_T("No valid Directory* returned.\n"));
            ::MessageBeep(MB_ICONEXCLAMATION);
            return FALSE;
        }
        m_treeview.selectDirectory(dir);
        if (forward)
        {
            // Keep a list of visited directories
            ATLVERIFY(m_visitedList.push(dir));
        }
        return TRUE;
    }

    inline void setStatusBarItem_(GenericObject* obj)
    {
        ATLASSERT(obj != nullptr);
        if (!::IsWindow(m_status))
        {
            return;
        }

        LPCWSTR fullName = (obj) ? obj->fullname().GetString() : nullptr;
        if (Directory* pdir = dynamic_cast<Directory*>(obj))
        {
            Directory& dir = *pdir;
            ULONG nSymLinks = 0, nDirectories = 0, nChildren = static_cast<ULONG>(dir.size());
            for (size_t i = 0; i < dir.size(); i++)
            {
                GenericObject* childobj = dir[i];

                ATLASSERT(childobj != nullptr);
                if (!childobj)
                    continue;

                switch (dir[i]->objtype())
                {
                case otSymlink:
                    nSymLinks++;
                    break;
                case otDirectory:
                    nDirectories++;
                    break;
                default:
                    break;
                }
            }
            CString str;
            str.Format(IDS_STATUSBAR_DIR_DETAILS, fullName, nDirectories, nSymLinks, nChildren);
            m_status.SetWindowText(str);
        }
        else
        {
            if (nullptr != fullName)
            {
                m_status.SetWindowText(fullName);
            }
        }
        m_status.SetPaneIcon(ID_DEFAULT_PANE, m_imagelist.IconByObjType(obj));
    }

    class IObjectDirectoryDumper
    {
      public:
        virtual ~IObjectDirectoryDumper()
        {
        }
        virtual void addSymlinkObject(const SymbolicLink* obj) = 0;
        virtual void addContainedObject(const GenericObject* obj) = 0;
        virtual void enterDirectory(const Directory* obj) = 0;
        virtual void leaveDirectory() = 0;
        virtual LPCTSTR getFileName() const = 0;

        virtual void operator()(Directory& current)
        {
            for (size_t i = 0; i < current.size(); i++)
            {
                GenericObject* entry = current[i];

                if (Directory* directory = dynamic_cast<Directory*>(entry))
                {
                    this->enterDirectory(directory);
                    this->operator()(*directory);
                    this->leaveDirectory();
                    continue;
                }
                if (SymbolicLink const* symlink = dynamic_cast<SymbolicLink const*>(entry))
                {
                    this->addSymlinkObject(symlink);
                    continue;
                }
                this->addContainedObject(entry);
            }
        }
    };

#ifndef NTOBJX_NO_XML_EXPORT
    class CXmlObjectDirectoryDumper : public IObjectDirectoryDumper
    {
        typedef IObjectDirectoryDumper baseClass;
        pugi::xml_document m_document;
        pugi::xml_node m_currentNode;
        pugi::xml_node m_previousNode;
        CString m_fileName;

      public:
        CXmlObjectDirectoryDumper(LPCTSTR lpszFileName)
            : m_document()
            , m_currentNode(m_document.append_child(_T("ObjectManager")).append_child(_T(OBJTYPESTR_DIRECTORY)))
            , m_fileName(lpszFileName)
        {
            m_currentNode.append_attribute(_T("name")).set_value(_T("\\"));
        }

        ~CXmlObjectDirectoryDumper()
        {
            ATLTRACE2(_T("Saving XML document to %s.\n"), m_fileName.GetString());
            try
            {
                m_document.save_file(m_fileName.GetString());
            }
            catch (...)
            {
                // ATLTRACE2(_T(__FUNCTION__) _T("\n")); FIXME.CLANG
            }
        }

        void addSymlinkObject(const SymbolicLink* obj)
        {
            pugi::xml_node node = addStandardObject_(obj);
            ATLVERIFY(node.append_attribute(_T("target")).set_value(obj->target().GetString()));
        }

        void addContainedObject(const GenericObject* obj)
        {
            (void)addStandardObject_(obj);
        }

        void enterDirectory(const Directory* obj)
        {
            ATLASSERT(obj != nullptr);
            m_previousNode = m_currentNode;
            m_currentNode = m_currentNode.append_child(_T(OBJTYPESTR_DIRECTORY));
            ATLVERIFY(m_currentNode.append_attribute(_T("name")).set_value(obj->name().GetString()));
        }

        void leaveDirectory()
        {
            m_currentNode = m_previousNode;
        }

        LPCTSTR getFileName() const
        {
            return m_fileName.GetString();
        }

      private:
        pugi::xml_node addStandardObject_(const GenericObject* obj)
        {
            ATLASSERT(obj != nullptr);
            pugi::xml_node node = m_currentNode.append_child(_T("Object"));
            ATLVERIFY(node.append_attribute(_T("type")).set_value(obj->type().GetString()));
            ATLVERIFY(node.append_attribute(_T("name")).set_value(obj->name().GetString()));
            return node;
        }
    };
#endif // !NTOBJX_NO_XML_EXPORT

    class CTxtObjectDirectoryDumper : public IObjectDirectoryDumper
    {
        typedef IObjectDirectoryDumper baseClass;
        FILE* m_file;
        bool m_bOpened;
        CString m_fileName;
        CString m_currentPrefix;
        CString m_previousPrefix;

      public:
        CTxtObjectDirectoryDumper(LPCTSTR lpszFileName)
            : m_file(nullptr)
            , m_bOpened(false)
            , m_fileName(lpszFileName)
            , m_currentPrefix(_T("\t"))
        {
            m_bOpened = (0 == _tfopen_s(&m_file, lpszFileName, _T("w+, ccs=UTF-8")));
            if (m_bOpened && m_file)
            {
                _ftprintf(m_file, _T("\\\n")); // Root of object manager namespace
            }
        }

        ~CTxtObjectDirectoryDumper()
        {
            ATLTRACE2(_T("Saving text document to %s.\n"), m_fileName.GetString());
            m_bOpened = (0 != fclose(m_file));
        }

        void addSymlinkObject(const SymbolicLink* obj)
        {
            ATLASSERT(obj != nullptr);
            LPCTSTR linePrefix = m_currentPrefix.GetString();
            _ftprintf(m_file, _T("%s%s [%s] -> %s\n"), linePrefix, obj->name().GetString(), obj->type().GetString(), obj->target().GetString());
        }

        void addContainedObject(const GenericObject* obj)
        {
            ATLASSERT(obj != nullptr);
            LPCTSTR linePrefix = m_currentPrefix.GetString();
            _ftprintf(m_file, _T("%s%s [%s]\n"), linePrefix, obj->name().GetString(), obj->type().GetString());
        }

        void enterDirectory(const Directory* obj)
        {
            ATLASSERT(obj != nullptr);
            LPCTSTR linePrefix = m_currentPrefix.GetString();
            _ftprintf(m_file, _T("%s\\%s [%s]\n"), linePrefix, obj->name().GetString(), obj->type().GetString());
            m_previousPrefix = m_currentPrefix;
            m_currentPrefix.AppendChar(_T('\t'));
        }

        void leaveDirectory()
        {
            m_currentPrefix = m_previousPrefix;
        }

        LPCTSTR getFileName() const
        {
            return m_fileName.GetString();
        }
    };

    void saveAs_(LPCTSTR lpszFileName, Directory& objroot)
    {
        if (size_t len = (lpszFileName) ? _tcslen(lpszFileName) : 0)
        {
#ifndef NTOBJX_NO_XML_EXPORT
            if ((len > 4) && (0 == _tcsicmp(_T(".xml"), &lpszFileName[len - 4])))
            {
                ATLTRACE2(_T("Assuming the user wants to save an XML, based on extension: %s\n"), lpszFileName);
                CXmlObjectDirectoryDumper dump(lpszFileName);
                dump(objroot);
            }
            else
            {
#endif // !NTOBJX_NO_XML_EXPORT
                ATLTRACE2(_T("Assuming the user wants to save an TXT, based on extension: %s\n"), lpszFileName);
                CTxtObjectDirectoryDumper dump(lpszFileName);
                dump(objroot);
#ifndef NTOBJX_NO_XML_EXPORT
            }
#endif // !NTOBJX_NO_XML_EXPORT
        }
    }

    inline HWND createStatusBar_()
    {
        HWND hWndStatusBar = m_status.Create(*this);
        m_status.initializePanes(m_bIsAdmin, m_bIsElevated);
        return hWndStatusBar;
    }

    template <typename T> HWND createSplitter_(T& splitter, HWND hWndParent, bool bFullDrag = true)
    {
        HWND hWndSplitter = splitter.Create(hWndParent, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        // Add WS_EX_CONTROLPARENT such that tab stops will work
        splitter.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
        // The splitter should be smaller than the default width
        splitter.m_cxySplitBar = 3;
        splitter.m_bFullDrag = bFullDrag; // enable/disable ghost bar instead of full "live" drag
        return hWndSplitter;
    }

    inline void enableShell60Features_()
    {
        if (DllGetVersion)
        {
            DLLVERSIONINFO dllvi = {sizeof(DLLVERSIONINFO), 0, 0, 0, 0};
            if (SUCCEEDED(DllGetVersion(&dllvi)) && dllvi.dwMajorVersion >= 6)
            {
                m_treeview.ModifyStyleEx(0, TVS_EX_DOUBLEBUFFER);
                m_listview.SetExtendedListViewStyle(CNtObjectsListViewTraits::GetWndExStyle(LVS_EX_DOUBLEBUFFER));
            }
            else
            {
                m_listview.SetExtendedListViewStyle(CNtObjectsListViewTraits::GetWndExStyle(0));
            }
        }
        else
        {
            m_listview.SetExtendedListViewStyle(CNtObjectsListViewTraits::GetWndExStyle(0));
        }
    }

    inline void setFrameWindow_()
    {
        m_treeview.setFrameWindow(m_hWnd); // make the frame window known to the treeview control
        m_listview.setFrameWindow(m_hWnd); // make the frame window known to the listview control
    }
};
