///////////////////////////////////////////////////////////////////////////////
///
/// The GUI controls and main frame window classes for ntobjx.
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

#pragma once

#ifndef __ATLAPP_H__
#   error ntobjx.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
#   error ntobjx.h requires atlwin.h to be included first
#endif

#ifndef __ATLCTRLX_H__
#   error ntobjx.h requires atlctrlx.h to be included first
#endif

#ifndef __ATLSPLIT_H__
#   error ntobjx.h requires atlsplit.h to be included first
#endif

#ifndef __ATLFRAME_H__
#   error ntobjx.h requires atlframe.h to be included first
#endif

#ifndef __ATLDLGS_H__
#   error ntobjx.h requires atldlgs.h to be included first
#endif

#ifndef __ATLCTRLS_H__
#   error ntobjx.h requires atlctrls.h to be included first
#endif

#ifndef __ATLSECURITY_H__
#   error ntobjx.h requires atlsecurity.h to be included first
#endif

#include "util/SimpleBuffer.h"
#ifndef VTRACE
#   define VTRACE(...) while (false) {}
#endif
#define CLL_NO_ENSURE_VERSION_CLASS
#include "util/LoadLibrary.h"
#include "util/VersionInfo.h"
#ifndef NTOBJX_NO_XML_EXPORT
#define _ALLOW_RTCc_IN_STL
#pragma warning(push)
#pragma warning(disable:4995)
#include "pugixml.hpp"
#pragma warning(pop)
#endif

using ATL::CAccessToken;
using ATL::CAtlMap;
using ATL::CString;
using NtObjMgr::Directory;
using NtObjMgr::GenericObject;
using NtObjMgr::SymbolicLink;
using NtObjMgr::objtype_t;
using NtObjMgr::otGeneric;
using NtObjMgr::otSymlink;
using NtObjMgr::otDirectory;
using NtObjMgr::ObjectHandle;
using WTL::CFileDialog;

#define WM_VISIT_DIRECTORY           (WM_USER+100)
#define WM_SET_ACTIVE_OBJECT         (WM_USER+101)
#define WM_SELECT_TREEVIEW_DIRECTORY (WM_USER+102)

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

namespace
{
    int const maxVisitedDepth = 10;
    int const imgListElemWidth = 16;
    int const imgListElemHeight = 16;

    // Requires open clipboard and doesn't close it
    BOOL SetClipboardString(LPCTSTR lpsz)
    {
        CString s(lpsz);
        SIZE_T allocLength = (s.GetLength() + 2) * sizeof(TCHAR);
        ATLTRACE2(_T("Buffer size: %u\n"), allocLength);
        HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, allocLength);
        if(hMem)
        {
            if(LPVOID lpMem = ::GlobalLock(hMem))
            {
                LPTSTR lpStr = static_cast<LPTSTR>(lpMem);
                _tcscpy_s(lpStr, s.GetLength() + 1, s.GetString());
                ::GlobalUnlock(hMem);
            }
#   if defined(_UNICODE) || defined(UNICODE)
            return (NULL != ::SetClipboardData(CF_UNICODETEXT, hMem));
#   else
#       error "Not implemented for ANSI/CF_TEXT"
#   endif
        }
        else
        {
            ATLTRACE2(_T("Could not allocate memory using GlobalAlloc().\n"));
        }
        return FALSE;
    }

    void CopyItemToClipboard(HWND hWndSource, int idCmd, GenericObject* obj)
    {
        if(::OpenClipboard(hWndSource))
        {
            CString s;
            switch(idCmd)
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

#   if defined(_UNICODE) || defined(UNICODE)
                    CString fullName(obj->fullname());
                    CString escapedName;
                    LPTSTR str = fullName.LockBuffer();
                    escapedName.Preallocate(fullName.GetAllocLength());
                    for(int i = 0; i < fullName.GetLength(); i++)
                    {
                        USHORT w = static_cast<USHORT>(str[i]);
                        if(w > 0xFF)
                        {
                            escapedName.AppendFormat(_T("\\u%04X"), w);
                        }
                        else
                        {
                            switch(str[i])
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
                                if(_istprint(str[i]))
                                    escapedName.AppendChar(str[i]);
                                else
                                    escapedName.AppendFormat(_T("\\u%04X"), w);
                                break;
                            }
                        }
                    }
                    escapedName.AppendChar(0);
                    s.Format(_T("L\"%s\""), escapedName.GetString());
#   else
                    s.Format(_T("\"%s\""), obj->fullname());
#   endif
                    ATLTRACE2(_T("Copy full path as C string: %s\n"), s.GetString());
                }
                break;
            case ID_POPUPMENU_COPYSYMLINKTARGET:
                if(SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(obj))
                {
                    s = symlink->target();
                    ATLTRACE2(_T("Copy symlink target: %s\n"), s.GetString());
                }
                break;
            }
            ATLVERIFY(SetClipboardString(s.GetString()));
            ATLVERIFY(::CloseClipboard());
        }
    }

    // Example from https://msdn.microsoft.com/en-us/library/windows/desktop/aa376389.aspx
    BOOL IsUserAdmin()
    {
        BOOL bResult;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID AdministratorsGroup;
        bResult = ::AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
        if (bResult)
        {
            if (!::CheckTokenMembership(NULL, AdministratorsGroup, &bResult))
            {
                bResult = FALSE;
            }
            ::FreeSid(AdministratorsGroup);
        }

        return bResult;
    }

    BOOL IsElevated()
    {
        BOOL bResult = FALSE;
        HANDLE hToken = NULL;
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
        if(::IsWindow(m_hWnd))
        {
            ::SendMessage(m_hWnd, WM_SETREDRAW, FALSE, 0);
        }
    }

    ~CSuppressRedraw()
    {
        if(::IsWindow(m_hWnd))
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
        PrepareImageList_();
    }

    virtual ~CObjectImageList()
    {
    }

    inline operator CImageList&()
    {
        return m_imagelist;
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
        ATLASSERT(obj != NULL);
        LPCTSTR typeName = obj->type();
        int retval = 0;
        for (size_t i = 0; i < resIconTypeMappingSize; i++)
        {
            if (0 == _tcsicmp(typeName, resIconTypeMapping[i].typeName))
            {
                if (IDI_DIRECTORY == resIconTypeMapping[i].resId)
                {
                    // This program is not multi-threaded, so this works, otherwise we'd need locking here
                    static int emptydir = -1;
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
    void PrepareImageList_()
    {
        ATLTRACE2(_T("Preparing image list for %s\n"), _T(__FUNCTION__));
        const int iResIconTypeMappingSize = static_cast<int>(resIconTypeMappingSize);
        ATLVERIFY(m_imagelist.Create(imgListElemWidth, imgListElemHeight, ILC_COLOR32 | ILC_MASK, iResIconTypeMappingSize, iResIconTypeMappingSize));
        for (int i = 0; i < iResIconTypeMappingSize; i++)
        {
            HICON hIcon = static_cast<HICON>(AtlLoadIconImage(MAKEINTRESOURCE(resIconTypeMapping[i].resId), LR_CREATEDIBSECTION, imgListElemWidth, imgListElemHeight));
            ATLTRACE2(_T("Icon handle: %p - %s\n"), hIcon, resIconTypeMapping[i].typeName);
            m_imagelist.AddIcon(hIcon);
        }
    }
};

#ifndef DDKBUILD
#   include <Aclui.h>
#   ifndef __ACCESS_CONTROL_API__
#       include <Aclapi.h>
#   endif
#   pragma comment(lib, "aclui.lib")
#endif // DDKBUILD

class CObjectPropertySheet :
    public CPropertySheetImpl<CObjectPropertySheet>
{
    class CObjectSecurityNAPage :
        public CPropertyPageImpl<CObjectSecurityNAPage>,
        public CWinDataExchange<CObjectSecurityNAPage>
    {
        typedef CPropertyPageImpl<CObjectSecurityNAPage> baseClass;
        GenericObject*  m_obj;
        ObjectHandle&   m_objHdl;
        bool            m_bFeatureUnavailable;
        CFont           m_font;
        CBrush          m_bkBrush;
        CEdit           m_edtExplanation;
        CObjectSecurityNAPage(); // hide it
    public:
        enum { IDD = IDD_SECURITY_NOT_AVAILABLE };

        BEGIN_MSG_MAP(CObjectSecurityNAPage)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
            CHAIN_MSG_MAP(baseClass)
        END_MSG_MAP()

        CObjectSecurityNAPage(GenericObject* obj, ObjectHandle& objHdl, bool bFeatureUnavailable = false)
            : baseClass((LPCTSTR)NULL)
            , m_obj(obj)
            , m_objHdl(objHdl)
            , m_bFeatureUnavailable(bFeatureUnavailable)
        {
        }

        LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
        {
            m_edtExplanation.Attach(GetDlgItem(IDC_EXPLANATION_WHY));

            if (m_bFeatureUnavailable)
            {
                m_edtExplanation.SetWindowText(_T("The feature has not been included in this build."));
            }
            else
            {
                if (!m_objHdl)
                {
                    CLoadLibrary ntdll(_T("ntdll.dll"));
                    CSimpleBuf<TCHAR> status(ntdll.formatMessage<TCHAR>(ntdll.getHandle(), static_cast<DWORD>(m_objHdl.getOpenStatus())));
                    m_edtExplanation.SetWindowText(status.Buffer());
                }
            }
            LONG oldStyle = m_edtExplanation.GetWindowLong(GWL_STYLE);
            m_edtExplanation.SetWindowLong(GWL_STYLE, oldStyle & ~ES_NOHIDESEL);
            m_edtExplanation.SetSelNone(TRUE);

            NONCLIENTMETRICS ncm = { 0 };
            ncm.cbSize = sizeof(ncm);

            if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
            {
                LOGFONT logFont;
                memcpy(&logFont, &ncm.lfMessageFont, sizeof(LOGFONT));
                logFont.lfWeight = FW_BOLD;
                HFONT hFont = m_font.CreateFontIndirect(&logFont);
                if (hFont)
                {
                    SendDlgItemMessage(IDC_EXPLANATION_WHY, WM_SETFONT, reinterpret_cast<WPARAM>(hFont));
                    SendDlgItemMessage(IDC_NO_SECURITY_CAPTION, WM_SETFONT, reinterpret_cast<WPARAM>(hFont));
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
                CDC dc(GetDC());
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

    class CObjectDetailsPage :
        public CPropertyPageImpl<CObjectDetailsPage>,
        public CWinDataExchange<CObjectDetailsPage>
    {
        typedef CPropertyPageImpl<CObjectDetailsPage> baseClass;
        GenericObject*  m_obj;
        ObjectHandle&   m_objHdl;
        CEdit           m_edtName;
        CEdit           m_edtFullname;
        CEdit           m_edtType;
        CFont           m_font;
        CBrush          m_bkBrush;
        CEdit           m_edtExplanation;
        CStatic         m_stcRefByPtr;
        CStatic         m_stcRefByHdl;
        CStatic         m_stcQuotaPaged;
        CStatic         m_stcQuotaNonPaged;
        CStatic         m_stcCreationTime;
        CStatic         m_stcGroupObjSpecific;
        CStatic         m_stcObjSpecName1;
        CStatic         m_stcObjSpecName2;
        CStatic         m_stcObjSpecName3;
        CStatic         m_stcObjSpecAttr1;
        CStatic         m_stcObjSpecAttr2;
        CStatic         m_stcObjSpecAttr3;
        CObjectDetailsPage(); // hide it
    public:
        enum { IDD = IDD_PROPERTIES };

        BEGIN_MSG_MAP(CObjectDetailsPage)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
            MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
            CHAIN_MSG_MAP(baseClass)
        END_MSG_MAP()

        CObjectDetailsPage(GenericObject* obj, ObjectHandle& objHdl)
            : baseClass((LPCTSTR)NULL)
            , m_obj(obj)
            , m_objHdl(objHdl)
        {
        }

        LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
        {
            m_edtName.Attach(GetDlgItem(IDC_EDIT_NAME));
            m_edtFullname.Attach(GetDlgItem(IDC_EDIT_FULLNAME));
            m_edtType.Attach(GetDlgItem(IDC_EDIT_TYPE));
            m_edtExplanation.Attach(GetDlgItem(IDC_EXPLANATION_WHY));
            m_stcRefByPtr.Attach(GetDlgItem(IDC_REF_BYPTR));
            m_stcRefByHdl.Attach(GetDlgItem(IDC_REF_BYHDL));
            m_stcQuotaPaged.Attach(GetDlgItem(IDC_QUOTA_PAGED));
            m_stcQuotaNonPaged.Attach(GetDlgItem(IDC_QUOTA_NONPAGED));
            m_stcCreationTime.Attach(GetDlgItem(IDC_OBJ_CREATION_TIME));
            m_stcGroupObjSpecific.Attach(GetDlgItem(IDC_GROUP_OBJSPECIFIC));
            m_stcObjSpecName1.Attach(GetDlgItem(IDC_STATIC_OBJSPEC_NAME1));
            m_stcObjSpecName2.Attach(GetDlgItem(IDC_STATIC_OBJSPEC_NAME2));
            m_stcObjSpecName3.Attach(GetDlgItem(IDC_STATIC_OBJSPEC_NAME3));
            m_stcObjSpecAttr1.Attach(GetDlgItem(IDC_STATIC_OBJSPEC_ATTR1));
            m_stcObjSpecAttr2.Attach(GetDlgItem(IDC_STATIC_OBJSPEC_ATTR2));
            m_stcObjSpecAttr3.Attach(GetDlgItem(IDC_STATIC_OBJSPEC_ATTR3));

            if(m_obj)
            {
                m_edtName.SetWindowText(m_obj->name());
                m_edtFullname.SetWindowText(m_obj->fullname());
                m_edtType.SetWindowText(m_obj->type());
            }

            LPCTSTR na = _T("n/a");
            if (m_objHdl)
            {
                CString str;
                if (m_objHdl.hasObjectInfo())
                {
                    str.Format(_T("%u"), m_objHdl.getObjectInfo().HandleCount);
                    m_stcRefByHdl.SetWindowText(str);
                    str.Format(_T("%u"), m_objHdl.getObjectInfo().ReferenceCount);
                    m_stcRefByPtr.SetWindowText(str);
                    str.Format(_T("%u"), m_objHdl.getObjectInfo().PagedPoolUsage);
                    m_stcQuotaPaged.SetWindowText(str);
                    str.Format(_T("%u"), m_objHdl.getObjectInfo().NonPagedPoolUsage);
                    m_stcQuotaNonPaged.SetWindowText(str);
                    m_stcCreationTime.SetWindowText(FormatCreationTime_(m_objHdl.getObjectInfo().CreationTime));

                    ShowObjectSpecificInfo_();
                }
                else
                {
                    m_stcRefByHdl.SetWindowText(na);
                    m_stcRefByPtr.SetWindowText(na);
                    m_stcQuotaPaged.SetWindowText(na);
                    m_stcQuotaNonPaged.SetWindowText(na);
                    NTSTATUS status = m_objHdl.getQueryStatus();
                    str.Format(_T("<Object query status: 0x%08X>"), status);
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
                str.Format(_T("<Object open status: 0x%08X>"), status);
                m_stcCreationTime.SetWindowText(str);
            }

            NONCLIENTMETRICS ncm = { 0 };
            ncm.cbSize = sizeof(ncm);

            if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
            {
                LOGFONT logFont;
                memcpy(&logFont, &ncm.lfMessageFont, sizeof(LOGFONT));
                logFont.lfWeight = FW_BOLD;
                HFONT hFont = m_font.CreateFontIndirect(&logFont);
                if (hFont)
                {
                    SendDlgItemMessage(IDC_EXPLANATION_WHY, WM_SETFONT, reinterpret_cast<WPARAM>(hFont));
                }
            }

            LONG oldStyle = m_edtExplanation.GetWindowLong(GWL_STYLE);
            m_edtExplanation.SetWindowLong(GWL_STYLE, oldStyle & ~ES_NOHIDESEL);
            m_edtExplanation.SetSelNone(TRUE);

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
                CDC dc(GetDC());
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

        LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
        {
            return DLGC_WANTALLKEYS | DefDlgProc(*this, uMsg, wParam, lParam);
        }

    private:

        static CString FormatCreationTime_(LARGE_INTEGER const& li)
        {
            CString str;
            if (li.QuadPart > 0)
            {
                ATLASSERT(li.HighPart >= 0); // anything else will fail our conversion below
                FILETIME ft = { li.LowPart, static_cast<DWORD>(li.HighPart) };
                SYSTEMTIME st;
                ATLVERIFY(FileTimeToSystemTime(&ft, &st));
                str.Format(_T("%04d-%02d-%02d %02d:%02d:%02d.%03d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            }
            else
            {
                str = _T("n/a");
            }
            return str;
        }

        void SetAttributesVisible_(int num)
        {
            m_stcGroupObjSpecific.ShowWindow(SW_SHOWNOACTIVATE);
            if (num < 0)
                m_edtExplanation.ShowWindow(SW_SHOWNOACTIVATE);
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

        void ShowObjectSpecificError_(NTSTATUS statusCode)
        {
            CLoadLibrary ntdll(_T("ntdll.dll"));
            CSimpleBuf<TCHAR> status(ntdll.formatMessage<TCHAR>(ntdll.getHandle(), static_cast<DWORD>(statusCode)));
            m_edtExplanation.SetWindowText(status.Buffer());
            SetAttributesVisible_(-1);
        }

        void ShowObjectSpecificInfo_()
        {
            CString str;
            if (ObjectHandle::CEventBasicInformation const* info = m_objHdl.getEventBasicInfo())
            {
                if (*info)
                {
                    SetAttributesVisible_(2);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_EVENT));
                    m_stcObjSpecName1.SetWindowText(str);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_EVENT));
                    m_stcObjSpecName2.SetWindowText(str);
                    str.Format(_T("%d"), info->EventState);
                    m_stcObjSpecAttr1.SetWindowText(str);
                    switch (info->EventType)
                    {
                    case SynchronizationEvent:
                        ATLVERIFY(str.LoadString(IDS_OBJSPEC_ATTR2_EVENT_SYNC));
                        break;
                    case NotificationEvent:
                        ATLVERIFY(str.LoadString(IDS_OBJSPEC_ATTR2_EVENT_NOTIFY));
                        break;
                    default:
                        str.Format(_T("<unknown=%d>"), info->EventType);
                        break;
                    }
                    m_stcObjSpecAttr2.SetWindowText(str);
                }
                else
                    ShowObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CIoCompletionBasicInformation const* info = m_objHdl.getIoCompletionBasicInfo())
            {
                if (*info)
                {
                    SetAttributesVisible_(1);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_IOCOMPLETION));
                    m_stcObjSpecName1.SetWindowText(str);
                    str.Format(_T("%u"), info->Depth);
                    m_stcObjSpecAttr1.SetWindowText(str);
                }
                else
                    ShowObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CKeyBasicInformation const* info = m_objHdl.getKeyBasicInfo())
            {
                if (*info)
                {
                    SetAttributesVisible_(2);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_KEY));
                    m_stcObjSpecName1.SetWindowText(str);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_KEY));
                    m_stcObjSpecName2.SetWindowText(str);
                    str = FormatCreationTime_(info->LastWriteTime);
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%u"), info->TitleIndex);
                    m_stcObjSpecAttr2.SetWindowText(str);
                }
                else
                    ShowObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CMutantBasicInformation const* info = m_objHdl.getMutantBasicInfo())
            {
                if (*info)
                {
                    SetAttributesVisible_(3);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_MUTANT));
                    m_stcObjSpecName1.SetWindowText(str);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_MUTANT));
                    m_stcObjSpecName2.SetWindowText(str);
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
                    ShowObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CSectionBasicInformation const* info = m_objHdl.getSectionBasicInfo())
            {
                if (*info)
                {
                    SetAttributesVisible_(3);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_SECTION));
                    m_stcObjSpecName1.SetWindowText(str);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_SECTION));
                    m_stcObjSpecName2.SetWindowText(str);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME3_SECTION));
                    m_stcObjSpecName3.SetWindowText(str);
                    str.Format(_T("0x%p"), info->BaseAddress);
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%I64d"), info->MaximumSize.QuadPart);
                    m_stcObjSpecAttr2.SetWindowText(str);
                    str.Format(_T("%08X"), info->AllocationAttributes);
                    m_stcObjSpecAttr3.SetWindowText(str);
                }
                else
                    ShowObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CSemaphoreBasicInformation const* info = m_objHdl.getSemaphoreBasicInfo())
            {
                if (*info)
                {
                    SetAttributesVisible_(2);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_SEMAPHORE));
                    m_stcObjSpecName1.SetWindowText(str);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_SEMAPHORE));
                    m_stcObjSpecName2.SetWindowText(str);
                    str.Format(_T("%u"), info->CurrentCount);
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%u"), info->MaximumCount);
                    m_stcObjSpecAttr2.SetWindowText(str);
                }
                else
                    ShowObjectSpecificError_(info->getQueryStatus());
            }

            if (ObjectHandle::CTimerBasicInformation const* info = m_objHdl.getTimerBasicInfo())
            {
                if (*info)
                {
                    SetAttributesVisible_(2);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME1_TIMER));
                    m_stcObjSpecName1.SetWindowText(str);
                    ATLVERIFY(str.LoadString(IDS_OBJSPEC_NAME2_TIMER));
                    m_stcObjSpecName2.SetWindowText(str);
                    str = info->TimerState ? _T("on") : _T("off");
                    m_stcObjSpecAttr1.SetWindowText(str);
                    str.Format(_T("%I64d"), info->RemainingTime.QuadPart);
                    m_stcObjSpecAttr2.SetWindowText(str);
                }
                else
                    ShowObjectSpecificError_(info->getQueryStatus());
            }
        }
    };

#ifndef DDKBUILD
    class CSecurityInformation :
        public ISecurityInformation
    {
        GenericObject*  m_obj;
        ObjectHandle&   m_objHdl;
    public:
        CSecurityInformation(GenericObject* obj, ObjectHandle& objHdl)
            : m_obj(obj)
            , m_objHdl(objHdl)
        {
        }

        STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj)
        {
            if(riid == IID_IUnknown || riid == IID_ISecurityInformation)
            {
                *ppvObj = this;
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)()
        {
            return 2;
        }

        STDMETHOD_(ULONG, Release)()
        {
            return 1;
        }

        STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo)
        {
            pObjectInfo->dwFlags = SI_ADVANCED | SI_NO_TREE_APPLY | SI_READONLY;
            pObjectInfo->hInstance = NULL;
            pObjectInfo->pszObjectName = const_cast<LPTSTR>(m_obj->name().GetString());

            return S_OK;
        }

        STDMETHOD(GetSecurity)(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR *ppSecurityDescriptor, BOOL /*fDefault*/)
        {
            HANDLE hObj = m_objHdl.getHandle();
            ATLASSERT(hObj != INVALID_HANDLE_VALUE);
            ATLASSERT(hObj != NULL);
            DWORD dwResult = ::GetSecurityInfo(hObj, SE_KERNEL_OBJECT, si, NULL, NULL, NULL, NULL, ppSecurityDescriptor);
            return (ERROR_SUCCESS == dwResult) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
        }

        STDMETHOD(SetSecurity)(SECURITY_INFORMATION /*si*/, PSECURITY_DESCRIPTOR /*pSecurityDescriptor*/)
        {
            ATLTRACENOTIMPL(_T(__FUNCTION__));
        }

        STDMETHOD(GetAccessRights) (const GUID* /*pguidObjectType*/, DWORD /*dwFlags*/, PSI_ACCESS *ppAccess, ULONG *pcAccesses, ULONG *piDefaultAccess)
        {
            ATLASSERT(TIMER_QUERY_STATE == 0x0001);
            ATLASSERT(TIMER_MODIFY_STATE == 0x0002);
            ATLASSERT(TIMER_QUERY_STATE == MUTANT_QUERY_STATE);
            ATLASSERT(TIMER_MODIFY_STATE == SEMAPHORE_MODIFY_STATE);
            ATLASSERT(TIMER_MODIFY_STATE == IO_COMPLETION_MODIFY_STATE);
            static SI_ACCESS accessNameMapping[] =
            {
                { &GUID_NULL, GENERIC_READ, _T("Read"), SI_ACCESS_GENERAL },
                { &GUID_NULL, GENERIC_WRITE, _T("Write"), SI_ACCESS_GENERAL },
                { &GUID_NULL, GENERIC_EXECUTE, _T("Execute"), SI_ACCESS_GENERAL },
                { &GUID_NULL, 0x0001, _T("Query State"), SI_ACCESS_GENERAL },
                { &GUID_NULL, 0x0002, _T("Modify State"), SI_ACCESS_GENERAL },
                { &GUID_NULL, SYNCHRONIZE, _T("Synchronize"), SI_ACCESS_GENERAL }
            };

            *ppAccess = accessNameMapping;
            *pcAccesses = _countof(accessNameMapping);
            *piDefaultAccess = 0;

            return S_OK;
        }

        STDMETHOD(MapGeneric)(const GUID* /*pguidObjectType*/, UCHAR* /*pAceFlags*/, ACCESS_MASK* /*pMask*/)
        {
            ATLTRACENOTIMPL(_T(__FUNCTION__));
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
#endif // DDKBUILD

    typedef CPropertySheetImpl<CObjectPropertySheet> baseClass;
    GenericObject* m_obj;
    ObjectHandle   m_objHdl;
    CObjectDetailsPage m_details;
#ifndef DDKBUILD
    ATL::CAutoPtr<CSecurityInformation> m_security;
#endif // DDKBUILD
    ATL::CAutoPtr<CObjectSecurityNAPage> m_securityNA;
public:
    BEGIN_MSG_MAP(CObjectPropertySheet)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    CObjectPropertySheet(GenericObject* obj, CObjectImageList& imagelist)
        : baseClass((LPCTSTR)NULL, 0, NULL)
        , m_obj(obj)
        , m_objHdl(obj)
        , m_details(m_obj, m_objHdl)
    {
        baseClass::m_psh.dwFlags |= PSH_NOAPPLYNOW;
        if (m_obj && imagelist.IndexByObjType(m_obj) >= 0)
        {
            baseClass::m_psh.dwFlags |= PSH_USEHICON;
            baseClass::m_psh.hIcon = imagelist.IconByObjType(m_obj);
        }

        ATLASSERT(NULL != m_obj);
        ATLTRACE2(_T("Adding 'Object Details' property page\n"));
        AddPage(m_details);

#ifndef DDKBUILD
        if (!m_objHdl)
        {
            m_securityNA.Attach(new CObjectSecurityNAPage(m_obj, m_objHdl));
            ATLTRACE2(_T("Adding 'Security (N/A)' property page\n"));
            AddPage(*m_securityNA);
        }
        else
        {
            m_security.Attach(new CSecurityInformation(m_obj, m_objHdl));
            ATLTRACE2(_T("Adding 'Security' property page\n"));
            AddPage(::CreateSecurityPage(m_security));
        }
#else
        m_securityNA.Attach(new CObjectSecurityNAPage(m_obj, m_objHdl, true));
        ATLTRACE2(_T("Adding 'Security (N/A)' property page - feature not built into program\n"));
        AddPage(*m_securityNA);
#endif // DDKBUILD

        SetTitle(obj->name(), PSH_PROPTITLE);
    }

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        return TRUE;
    }
};

template <typename T>
class CHyperLinkCtxMenuImpl : public CHyperLinkImpl<T>
{
public:
    BEGIN_MSG_MAP(CHyperLinkCtxMenuImpl<T>)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
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
        // Get the popup menu at index 0 from the menu resource
        CMenuHandle popup = menu.GetSubMenu(0);

        // Let the user pick
        int idCmd = popup.TrackPopupMenuEx(
            TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON
            , pt.x
            , pt.y
            , GetParent()
            , NULL
            );

        if(idCmd)
        {
            switch(idCmd)
            {
            case ID_POPUPMENU_COPYURL:
                if(m_lpstrHyperLink)
                {
                    ATLTRACE2(_T("Copy URL\n"), idCmd);
                    if(::OpenClipboard(GetParent()))
                    {
                        ATLVERIFY(SetClipboardString(m_lpstrHyperLink));
                        ATLVERIFY(::CloseClipboard());
                    }
                }
                break;
            default:
                ATLTRACE2(_T("Chosen command: %i\n"), idCmd);
                break;
            }
        }

        return 0;
    }
};

class CHyperLinkCtxMenu : public CHyperLinkCtxMenuImpl<CHyperLinkCtxMenu>
{
public:
    DECLARE_WND_CLASS(_T("WTL_CHyperLinkCtxMenu"))
};


class CAboutDlg :
    public CDialogImpl<CAboutDlg>
{
    CHyperLinkCtxMenu m_website, m_projectpage, m_revisionurl;
    CVersionInfo& m_verinfo;
    CIcon m_appicon;
    CFont m_progNameFont;
public:
    enum { IDD = IDD_ABOUT };

    BEGIN_MSG_MAP(CAboutDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    CAboutDlg(CVersionInfo& verinfo)
        : m_verinfo(verinfo)
    {
    }

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        CString website(m_verinfo[_T("Website")]);
        CString repo(m_verinfo[_T("Mercurial repository")]);
        CString revision(m_verinfo[_T("Mercurial revision")]);
        CString revisionurl(m_verinfo[_T("Mercurial revision URL")]);

        ATLVERIFY(m_website.SetHyperLink(website));
        if(int slash = website.ReverseFind(_T('/')))
        {
            ATLVERIFY(m_website.SetLabel(&website.GetString()[slash+1]));
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

        ATLVERIFY(SetDlgItemText(IDC_STATIC_COPYRIGHT, m_verinfo[_T("LegalCopyright")]));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_DESCRIPTION, m_verinfo[_T("FileDescription")]));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_PROGNAME, m_verinfo[_T("ProductName")]));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_PORTIONSCOPYRIGHT, m_verinfo[_T("Portions Copyright")]));

        CString caption(_T("About "));
        caption.Append(m_verinfo[_T("ProductName")]);
        caption.Append(_T(" "));
        caption.Append(m_verinfo[_T("FileVersion")]);
        caption.AppendFormat(_T(" (%d-bit)"), sizeof(void*) == 4 ? 32 : 64);
        SetWindowText(caption);

        m_appicon.LoadIcon(IDR_MAINFRAME);
        ATLVERIFY(!m_appicon.IsNull());
        SetIcon(m_appicon);

        NONCLIENTMETRICS ncm = { 0 };
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

        return TRUE;
    }

    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        return DLGC_WANTALLKEYS | DefDlgProc(*this, uMsg, wParam, lParam);
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

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(wID);
        return 0;
    }
};

typedef CWinTraitsOR<WS_TABSTOP | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_INFOTIP, WS_EX_CLIENTEDGE, CControlWinTraits> CNtObjectsTreeViewTraits;
typedef CWinTraitsOR<WS_TABSTOP | LVS_SHAREIMAGELISTS | LVS_SINGLESEL, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, CSortListViewCtrlTraits> CNtObjectsListViewTraits;

class CNtObjectsTreeView :
    public CWindowImpl<CNtObjectsTreeView, CTreeViewCtrlEx, CNtObjectsTreeViewTraits>
{
    bool m_bInitialized;
    Directory m_objroot;
    CImageList m_imagelist;
    HWND m_hFrameWnd;
    CAtlMap<Directory*, HTREEITEM> m_reverseLookup;
    typedef CWindowImpl<CNtObjectsTreeView, CTreeViewCtrlEx, CNtObjectsTreeViewTraits> baseClass;
public:
    /*lint -save -e446 */
    DECLARE_WND_SUPERCLASS(_T("NtObjectsTreeView"), CTreeViewCtrlEx::GetWndClassName())
    /*lint -restore */

    BEGIN_MSG_MAP(CNtObjectsTreeView)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnTVItemExpanding)
        NOTIFY_CODE_HANDLER(TVN_GETINFOTIP, OnGetInfoTip)
        NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTVSelChanged)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    CNtObjectsTreeView()
        : m_bInitialized(false)
        , m_hFrameWnd(NULL)
    {}

    inline void SetFrameWindow(HWND hFrameWnd)
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
        ATLVERIFY(ScreenToClient(&pt));
        UINT flags = 0;
        CTreeItem ti = HitTest(pt, &flags);
        if(!ti.IsNull())
        {
            ATLVERIFY(SelectItem(ti));

            if(Directory* dir = reinterpret_cast<Directory*>(ti.GetData()))
            {
                // Load menu resource
                CMenu menu;
                ATLVERIFY(menu.LoadMenu(IDR_POPUP_MENU1));
                // Get the popup menu at index 0 from the menu resource
                CMenuHandle popup = menu.GetSubMenu(0);

                // Let the user pick
                int idCmd = popup.TrackPopupMenuEx(
                    TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON
                    , spt.x
                    , spt.y
                    , GetParent()
                    , NULL
                    );

                if(idCmd)
                {
                    switch(idCmd)
                    {
                    case ID_POPUPMENU_COPYNAME:
                    case ID_POPUPMENU_COPYFULLPATH:
                    case ID_POPUPMENU_COPYASCSTRING:
                    case ID_POPUPMENU_COPYSYMLINKTARGET:
                        CopyItemToClipboard(m_hWnd, idCmd, dir);
                        break;
                    case ID_POPUPMENU_PROPERTIES:
                        ::SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(ID_VIEW_PROPERTIES, 0), reinterpret_cast<LPARAM>(m_hWnd));
                        break;
                    default:
                        ATLTRACE2(_T("Chosen command: %i\n"), idCmd);
                        break;
                    }
                }
            }
        }

        return FALSE;
    }

    LRESULT OnTVItemExpanding(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        ATLTRACE2(_T("%s\n"), _T(__FUNCTION__));
        LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);
        // Don't allow to collapse the root item (which is the one without a pointer to the NtObjMgr::Directory)
        if((TVE_COLLAPSE == pnmtv->action) && (pnmtv->itemNew.hItem == GetRootItem()))
        {
            ATLTRACE2(_T("Root item must not be collapsed\n"));
            return (bHandled = TRUE);
        }
        if(pnmtv->itemNew.state & TVIS_EXPANDEDONCE)
            return 0;
        if(pnmtv->itemNew.cChildren)
        {
#ifdef _DEBUG
            if(Directory* dir = reinterpret_cast<Directory*>(GetItemData(pnmtv->itemNew.hItem)))
                ATLTRACE2(_T("Sort children of %s (%p)\n"), dir->fullname().GetString(), pnmtv->itemNew.hItem);
            else
                ATLTRACE2(_T("Sort children of %p\n"), pnmtv->itemNew.hItem);
#endif
            SortTreeItemChildren_(pnmtv->itemNew.hItem);
        }
        return 0;
    }

    LRESULT OnGetInfoTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        LPNMTVGETINFOTIP pnmtv = reinterpret_cast<LPNMTVGETINFOTIP>(pnmh);
        ATLASSERT(NULL != pnmtv);
        Directory* dir = reinterpret_cast<Directory*>(pnmtv->lParam);
        bHandled = FALSE;
        if(dir)
        {
            LPCTSTR comment = findComment(dir->fullname());
            if(comment)
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
        ATLASSERT(NULL != pnmtv);
        bHandled = FALSE;
        Directory* dir = reinterpret_cast<Directory*>(pnmtv->itemNew.lParam);
        if(m_bInitialized && dir)
        {
            ATLTRACE2(_T("%s: %s\n"), _T(__FUNCTION__), dir->fullname().GetString());
            // Tell the frame to update the listview with the new Directory
            (void)::SendMessage(m_hFrameWnd, WM_VISIT_DIRECTORY, 0, reinterpret_cast<LPARAM>(dir));
            bHandled = TRUE;
        }
        return 0;
    }

    /*lint -save -e1536 */
    Directory& ObjectRoot()
    {
        return m_objroot;
    }
    /*lint -restore */

    bool EmptyAndRefill()
    {
        CSuppressRedraw suppressRedraw(m_hWnd);
        if(m_objroot.refresh())
        {
            if(::IsWindow(m_hWnd) && m_imagelist.IsNull())
            {
                PrepareImageList_();
            }
            if(DeleteAllItems())
            {
                // Recursively fill the tree
                Fill_(NULL, m_objroot);
                // Sort the tree
                SortTreeItemChildren_(GetRootItem());
                (void)Expand(GetRootItem());
                // Signal that the tree has been initialized
                m_bInitialized=true;
                return m_bInitialized;
            }
        }
        // Something failed
        m_bInitialized=false;
        return m_bInitialized;
    }

    inline void SelectRoot()
    {
        ATLTRACE2(_T("%s\n"), _T(__FUNCTION__));
        (void)SelectItem(GetRootItem());
    }

    inline CTreeItem LookupTreeItem(Directory* dir)
    {
        HTREEITEM ret = NULL;
        if(m_reverseLookup.Lookup(dir, ret))
        {
            ATLTRACE2(_T("Found %s in treeview\n"), dir->fullname().GetString());
            return CTreeItem(ret, this);
        }
        return NULL;
    }

    void SelectDirectory(Directory* dir, BOOL bIfNotAlreadySelected = FALSE)
    {
        CTreeItem ti(LookupTreeItem(dir));
        if(!ti.IsNull())
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

private:
    static int CALLBACK TreeViewCompareProc_(LPARAM lparam1, LPARAM lparam2, LPARAM /*lparamSort*/)
    {
        Directory* dir1 = reinterpret_cast<Directory*>(lparam1);
        Directory* dir2 = reinterpret_cast<Directory*>(lparam2);
        ATLASSERT(NULL != dir1);
        ATLASSERT(NULL != dir2);
        return wcscmp(dir1->name(), dir2->name());
    }

    // We create our image list dynamically from the icons
    void PrepareImageList_()
    {
        ATLTRACE2(_T("Preparing image list for %s\n"), _T(__FUNCTION__));
        WORD iconList[] = { IDI_OBJMGR_ROOT, IDI_DIRECTORY, IDI_EMPTY_DIRECTORY };
        ATLVERIFY(m_imagelist.Create(imgListElemWidth, imgListElemHeight, ILC_COLOR32 | ILC_MASK, (int)_countof(iconList), (int)_countof(iconList)));
        for(size_t i = 0; i < _countof(iconList); i++)
        {
            HICON hIcon = static_cast<HICON>(AtlLoadIconImage(MAKEINTRESOURCE(iconList[i]), LR_CREATEDIBSECTION, imgListElemWidth, imgListElemHeight));
            ATLTRACE2(_T("Icon handle: %p\n"), hIcon);
            m_imagelist.AddIcon(hIcon);
        }
        (void)SetImageList(m_imagelist);
    }

    void SortTreeItemChildren_(HTREEITEM parent)
    {
        TVSORTCB tvscb = { parent, TreeViewCompareProc_, 0 };
        (void)SortChildrenCB(&tvscb, TRUE);
    }

    void Fill_(HTREEITEM parent, Directory& current)
    {
        if(!parent)
        {
            m_reverseLookup.RemoveAll();
            parent = InsertItem(TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_PARAM, current.fullname(), 0, 0, 0, 0, reinterpret_cast<LPARAM>(&m_objroot), TVI_ROOT, TVI_LAST);
            m_reverseLookup[&m_objroot] = parent;
        }
        for(size_t i = 0; i < current.size(); i++)
        {
            Directory* entry = dynamic_cast<Directory*>(current[i]);
            if(entry)
            {
                const int imgidx = entry->size() ? 1 : 2;
                HTREEITEM curritem = InsertItem(TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_PARAM, entry->name(), imgidx, imgidx, 0, 0, reinterpret_cast<LPARAM>(entry), parent, TVI_LAST);
                m_reverseLookup[entry] = curritem;
                Fill_(curritem, *entry);
            }
        }
    }
};

class CNtObjectsListView :
    public CSortListViewCtrlImpl<CNtObjectsListView, CListViewCtrl, CNtObjectsListViewTraits>
{
    HWND m_hFrameWnd;
    CObjectImageList* m_pimagelist;
    typedef CSortListViewCtrlImpl<CNtObjectsListView, CListViewCtrl, CNtObjectsListViewTraits> baseClass;
public:
    /*lint -save -e446 */
    DECLARE_WND_SUPERCLASS(_T("NtObjectsListView"), CListViewCtrl::GetWndClassName())
    /*lint -restore */

    BEGIN_MSG_MAP(CNtObjectsListView)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
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

    CNtObjectsListView(HWND hFrameWnd = NULL)
        : baseClass()
        , m_hFrameWnd(hFrameWnd)
        , m_pimagelist(NULL)
    {}

    HWND Create(HWND hWndParent, CObjectImageList& imagelist, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
        DWORD dwStyle = 0, DWORD dwExStyle = 0,
        ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
    {
        m_pimagelist = &imagelist;
        return baseClass::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
    }

    inline void SetFrameWindow(HWND hFrameWnd)
    {
        m_hFrameWnd = hFrameWnd;
    }

    void FillFromDirectory(Directory& current)
    {
        CSuppressRedraw suppressRedraw(m_hWnd);
        DeleteAllItems();
        ATLASSERT(m_pimagelist != NULL);
#ifndef _DEBUG
        if (!m_pimagelist)
        {
            return; // Avoid outright crash
        }
#endif // !_DEBUG
        if(::IsWindow(m_hWnd) && !m_pimagelist->IsNull())
        {
            (void)SetImageList(*m_pimagelist, LVSIL_SMALL);
        }
        ResetAllColumns_();
        ATLASSERT(!m_pimagelist->IsNull());
        // To determine the required width for each column
        int widths[] = {-1, -1, -1};
        for(size_t i = 0; i < current.size(); i++)
        {
            int currItem = InsertItem(
                LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM
                , static_cast<int>(i)
                , current[i]->name()
                , 0
                , 0
                , m_pimagelist->IndexByObjType(current[i])
                , reinterpret_cast<LPARAM>(current[i])
                );
            int const nameWidth = GetStringWidth(current[i]->name());
            widths[0] = max(nameWidth, widths[0]);
            AddItem(
                currItem
                , 1
                , current[i]->type()
                );
            int const typeWidth = GetStringWidth(current[i]->type());
            widths[1] = max(typeWidth, widths[1]);
            ATLTRACE2(_T("%s:%i [%s:%i] / %i:%i\n"), current[i]->name().GetString(), GetStringWidth(current[i]->name()), current[i]->type().GetString(), GetStringWidth(current[i]->type()), widths[0], widths[1]);
            SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(current[i]);
            if(symlink)
            {
                AddItem(
                    currItem
                    , 2
                    , symlink->target()
                    );
                int const targetWidth = GetStringWidth(symlink->target());
                widths[2] = max(targetWidth, widths[2]);
                ATLTRACE2(_T("    %s:%i [%i]\n"), symlink->target().GetString(), GetStringWidth(symlink->target()), widths[2]);
            }
        }
        CRect rect;
        ATLVERIFY(GetWindowRect(&rect));
        int const maxWidths[] = {rect.right-rect.left - 50, 150, 200};
        // Try to set an optimum column width
        for(int i = 0; i <  static_cast<int>(_countof(widths)); i++)
        {
            if(-1 != widths[i])
            {
                int currcol = min(widths[i] + imgListElemWidth + (i ? 0 : 0x10), maxWidths[i]);
                ATLVERIFY(SetColumnWidth(i, currcol));
            }
        }
        // Sort according to name column
        SortItems(0);
    }

    LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        // Find where the user clicked, so we know the item
        LVHITTESTINFO lvhtti = {0};
        CPoint pt;

        if(ItemFromHitTest_(lvhtti, pt))
        {
            // If we found an item, select it
            ATLVERIFY(SelectItem(lvhtti.iItem));

            LVITEM item = {0};
            item.iItem = lvhtti.iItem;
            item.mask = LVIF_PARAM;
            ATLVERIFY(GetItem(&item));
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
            if(otDirectory == submenuidx)
            {
                if(Directory* dir = dynamic_cast<Directory*>(itemobj))
                {
                    if(!dir->size())
                        submenuidx = otGeneric;
                }
            }

            // Load menu resource
            CMenu menu;
            ATLVERIFY(menu.LoadMenu(IDR_POPUP_MENU1));
            // Get the popup menu at index 0 from the menu resource
            CMenuHandle popup = menu.GetSubMenu(submenuidx);

            // Let the user pick
            int idCmd = popup.TrackPopupMenuEx(
                TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON
                , pt.x
                , pt.y
                , GetParent()
                , NULL
                );

            if(idCmd)
            {
                switch(idCmd)
                {
                case ID_POPUPMENU_COPYNAME:
                case ID_POPUPMENU_COPYFULLPATH:
                case ID_POPUPMENU_COPYASCSTRING:
                case ID_POPUPMENU_COPYSYMLINKTARGET:
                    CopyItemToClipboard(m_hWnd, idCmd, itemobj);
                    break;
                case ID_POPUPMENU_PROPERTIES:
                    ::SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(ID_VIEW_PROPERTIES, 0), reinterpret_cast<LPARAM>(m_hWnd));
                    break;
                case ID_POPUPMENU_OPENDIRECTORY:
                    if(Directory* dir = dynamic_cast<Directory*>(itemobj))
                    {
                        OpenDirectory_(dir);
                    }
                    break;
                default:
                    ATLTRACE2(_T("Chosen command: %i\n"), idCmd);
                    break;
                }
            }
        }

        return 0;
    }

    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        return DLGC_WANTALLKEYS | DefWindowProc(uMsg, wParam, lParam);
    }

    LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        ATLTRACE2(_T("key code: %u\n"), wParam);
        switch(wParam)
        {
        case VK_RETURN:
            {
                int idx = GetNextItem(-1, LVNI_SELECTED);
                ATLTRACE2(_T("%i == GetNextItem(-1, LVNI_SELECTED)\n"), idx);
                if (idx >= 0)
                {
                    ATLASSERT(1 == GetSelectedCount());
                    if (-1 != idx)
                    {
                        OpenPropertiesOrDirectory_(idx);
                    }
                }
            }
            break;
        case VK_TAB:
            GetParent().GetNextDlgTabItem(m_hWnd).SetFocus(); // Allows to still use tab to change focus
            return 0;
        case VK_BACK:
            // FIXME|TODO
            break;
        }
        return DefWindowProc(uMsg, wParam, lParam);
    }

    LRESULT OnDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        ATLTRACE2(_T("%s\n"), _T(__FUNCTION__));
        // Find where the user clicked, so we know the item
        LVHITTESTINFO lvhtti = {0};
        CPoint pt;

        if(ItemFromHitTest_(lvhtti, pt))
        {
            // If we found an item, select it
            ATLVERIFY(SelectItem(lvhtti.iItem));
            OpenPropertiesOrDirectory_(lvhtti.iItem);
        }
        return 0;
    }

    LRESULT OnHeaderItemClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
    {
        ATLTRACE2(_T("%s\n"), _T(__FUNCTION__));
        LPNMHEADER p = reinterpret_cast<LPNMHEADER>(pnmh);
        if(p->iButton == 0)
        {
            int iOld = m_iSortColumn;
            bool bDescending = (m_iSortColumn == p->iItem) ? !m_bSortDescending : false;
            if(DoSortItems(p->iItem, bDescending))
            {
                NotifyParentSortChanged(p->iItem, iOld);
            }
        }
        return 0;
    }

    LRESULT OnLVItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        LPNMLISTVIEW pnmlv = reinterpret_cast<LPNMLISTVIEW>(pnmh);
        ATLASSERT(NULL != pnmlv);
        bHandled = FALSE;
        if(pnmlv && (pnmlv->uChanged & LVIF_STATE) && (pnmlv->uNewState & LVIS_SELECTED))
        {
            GenericObject* obj = reinterpret_cast<GenericObject*>(pnmlv->lParam);
            ATLASSERT(NULL != obj);
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
        ATLASSERT(NULL != pnmlv);
        bHandled = FALSE;
        if (pnmlv)
        {
            LVITEM item = { 0 };
            item.iItem = pnmlv->iItem;
            item.mask = LVIF_PARAM;
            ATLVERIFY(GetItem(&item));

            GenericObject* obj = reinterpret_cast<GenericObject*>(item.lParam);
            ATLASSERT(NULL != obj);
            if (obj)
            {
                LPCTSTR comment = findComment(obj->fullname());
                if (comment)
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

    // If item1 > item2 return 1, if item1 < item2 return -1, else return 0.
    int CompareItemsCustom(LVCompareParam* pItem1, LVCompareParam* pItem2, int iSortCol)
    {
        if(!iSortCol)
        {
            GenericObject* obj1 = reinterpret_cast<GenericObject*>(pItem1->dwItemData);
            GenericObject* obj2 = reinterpret_cast<GenericObject*>(pItem2->dwItemData);
            bool isDir1 = (otDirectory == obj1->objtype());
            bool isDir2 = (otDirectory == obj2->objtype());
            if(isDir1 && isDir2)
                return _tcsicmp(pItem1->pszValue, pItem2->pszValue);
            if(!isDir1 && !isDir2)
                return _tcsicmp(pItem1->pszValue, pItem2->pszValue);
            if(!isDir1 && isDir2)
                return 1;
            if(isDir1 && !isDir2)
                return -1;
        }
        return _tcsicmp(pItem1->pszValue, pItem2->pszValue);
    }

private:

    void OpenDirectory_(Directory* dir) const
    {
        ATLASSERT(dir != NULL);
        ATLTRACE2(_T("%s: %s\n"), _T(__FUNCTION__), dir->fullname().GetString());
        // Tell the frame to update the listview with the new Directory
        (void)::SendMessage(m_hFrameWnd, WM_SELECT_TREEVIEW_DIRECTORY, 0, reinterpret_cast<LPARAM>(dir));
    }

    void OpenPropertiesOrDirectory_(int idx) const
    {
        ATLASSERT(idx != -1);
        LVITEM item = {0};
        item.iItem = idx;
        item.mask = LVIF_PARAM;
        ATLVERIFY(GetItem(&item));
        GenericObject* itemobj = reinterpret_cast<GenericObject*>(item.lParam);
        if(otDirectory == itemobj->objtype())
        {
            if(Directory* dir = dynamic_cast<Directory*>(itemobj))
            {
                if(dir->size())
                {
                    OpenDirectory_(dir);
                    return;
                }
                // If it's empty, we'll show the properties instead, the user may still navigate
                // into the empty Directory using the treeview
            }
        }
        ::SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(ID_VIEW_PROPERTIES, 0), reinterpret_cast<LPARAM>(m_hWnd));
    }

    bool ItemFromHitTest_(LVHITTESTINFO& lvhtti, CPoint& pt) const
    {
        ATLVERIFY(::GetCursorPos(&pt));
        ATLTRACE2(_T("Point of click: %d/%d\n"), pt.x, pt.y);

        lvhtti.pt = pt;
        ATLVERIFY(ScreenToClient(&lvhtti.pt));
        return (-1 != HitTest(&lvhtti));
    }

    void DeleteAllColumns_()
    {
        ATLTRACE2(_T("Column count: %i\n"), GetColumnCount());
        // On older Windows/common control versions it's impossible to remove column 0
        for(int i = 1; i < GetColumnCount(); i++)
        {
            ATLVERIFY(DeleteColumn(i));
        }
    }

    void ResetAllColumns_()
    {
        static const struct { UINT resId; } columnDefaults[] = {
            {ID_COLNAME_OBJNAME},
            {ID_COLNAME_OBJTYPE},
            {ID_COLNAME_OBJLNKTGT},
        };

        // FIXME|TODO: later we may want to reinitialize columns when switching UI languages
        if(GetColumnCount() < static_cast<int>(_countof(columnDefaults)))
        {
            DeleteAllColumns_();
            CString columnName;
            for(int idx = 0; idx < static_cast<int>(_countof(columnDefaults)); idx++)
            {
                ATLVERIFY(columnName.LoadString(columnDefaults[idx].resId));
                if(idx > (GetColumnCount() - 1))
                {
                    InsertColumn(idx, columnName);
                    SetColumnSortType(idx, LVCOLSORT_CUSTOM);
                    ATLTRACE2(_T("Inserted column %i with name \"%s\".\n"), idx, columnName.GetString());
                }
                else
                {
                    LVCOLUMN lvc = {0};
                    lvc.mask = LVCF_TEXT;
                    lvc.pszText = const_cast<LPTSTR>(columnName.GetString());
                    lvc.cchTextMax = columnName.GetLength() + 1;
                    SetColumn(idx, &lvc);
                    SetColumnSortType(idx, LVCOLSORT_TEXTNOCASE);
                }
            }
        }
    }
};

#ifndef LVS_EX_DOUBLEBUFFER
#   define LVS_EX_DOUBLEBUFFER     0x00010000
#endif
#ifndef TVS_EX_DOUBLEBUFFER
#   define TVS_EX_DOUBLEBUFFER     0x0004
#endif


class CNtObjectsStatusBar : public CMultiPaneStatusBarCtrlImpl<CNtObjectsStatusBar>
{
public:
    DECLARE_WND_SUPERCLASS(_T("NtObjectsStatusBar"), GetWndClassName())

    void SetPaneWidths(int pane_widths[], int nPanes)
    {
    }
};

/*lint -esym(1509, CNtObjectsMainFrame) */
class CNtObjectsMainFrame :
    public CFrameWindowImpl<CNtObjectsMainFrame>,
    public CUpdateUI<CNtObjectsMainFrame>,
    public CMessageFilter,
    public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(_T("NtObjectsMainFrame"), IDR_MAINFRAME)

    CNtObjectsStatusBar m_status;
    CSplitterWindow m_vsplit;
    CNtObjectsTreeView m_treeview;
    CNtObjectsListView m_listview;
    LONG m_objroot_initialized; // only write to this member using Interlocked*()
    bool m_bFirstOnIdle;
    bool m_bIsFindDialogOpen;
    GenericObject* m_activeObject;
    CVersionInfo m_verinfo;
    CSimpleArray<Directory*> m_visitedList;
    int m_visitedListIndex;
    HRESULT (CALLBACK* DllGetVersion)(DLLVERSIONINFO *);
    CObjectImageList m_imagelist;
    CAccessToken m_Token;

    CNtObjectsMainFrame()
        : m_objroot_initialized(0)
        , m_bFirstOnIdle(true) // to force initial refresh
        , m_bIsFindDialogOpen(false)
        , m_activeObject(0)
        , m_verinfo(ModuleHelper::GetResourceInstance())
        , m_visitedListIndex(-1)
        , m_imagelist()
        , DllGetVersion(0)
    {
        *(FARPROC*)&DllGetVersion = ::GetProcAddress(::GetModuleHandle(_T("shell32.dll")), "DllGetVersion");
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        if(CFrameWindowImpl<CNtObjectsMainFrame>::PreTranslateMessage(pMsg))
            return TRUE;
        // We need this such that WS_TABSTOP takes effect
        if(IsDialogMessage(pMsg))
            return TRUE;
        return FALSE;
    }

    BEGIN_UPDATE_UI_MAP(CNtObjectsMainFrame)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CNtObjectsMainFrame)
        CHAIN_MSG_MAP_MEMBER(m_treeview)
        CHAIN_MSG_MAP_MEMBER(m_listview)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
        MESSAGE_HANDLER(WM_APPCOMMAND, OnAppCommand)
        MESSAGE_HANDLER(WM_VISIT_DIRECTORY, OnVisitDirectory)
        MESSAGE_HANDLER(WM_SELECT_TREEVIEW_DIRECTORY, OnSelectTreeviewDirectory)
        MESSAGE_HANDLER(WM_SET_ACTIVE_OBJECT, OnSetActiveObject)
        MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)
        COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
        COMMAND_ID_HANDLER(ID_SHOW_ABOUT, OnShowAbout)
        COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnSaveAs)
        COMMAND_ID_HANDLER(ID_VIEW_FIND, OnFindObject)
        CHAIN_MSG_MAP(CUpdateUI<CNtObjectsMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CNtObjectsMainFrame>)
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        m_objroot_initialized = 0;

        ATLVERIFY(CreateSimpleStatusBar());
        m_status.SubclassWindow(m_hWndStatusBar);

        m_hWndClient = m_vsplit.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        // Add WS_EX_CONTROLPARENT such that tab stops work
        LONG_PTR exStyle = ::GetWindowLongPtr(m_hWndClient, GWL_EXSTYLE);
        ::SetWindowLongPtr(m_hWndClient, GWL_EXSTYLE, exStyle | WS_EX_CONTROLPARENT);
        // The splitter should be smaller than the default width
        m_vsplit.m_cxySplitBar = 3;

        ATLVERIFY(NULL != m_treeview.Create(m_hWndClient));
        ATLVERIFY(NULL != m_listview.Create(m_hWndClient, m_imagelist));
        if(DllGetVersion)
        {
            DLLVERSIONINFO dllvi = {sizeof(DLLVERSIONINFO), 0, 0, 0, 0};
            if (SUCCEEDED(DllGetVersion(&dllvi)) && dllvi.dwMajorVersion >= 6)
            {
                m_treeview.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
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

        ATLTRACE2(_T("Control ID for splitter: %i\n"), ::GetDlgCtrlID(m_vsplit));
        ATLTRACE2(_T("Control ID for treeview: %i\n"), ::GetDlgCtrlID(m_treeview));
        ATLTRACE2(_T("Control ID for listview: %i\n"), ::GetDlgCtrlID(m_listview));
        ATLASSERT(m_vsplit.m_hWnd == ::GetDlgItem(m_hWnd, ::GetDlgCtrlID(m_vsplit)));
        ATLASSERT(m_treeview.m_hWnd == ::GetDlgItem(m_hWndClient, ::GetDlgCtrlID(m_treeview)));
        ATLASSERT(m_listview.m_hWnd == ::GetDlgItem(m_hWndClient, ::GetDlgCtrlID(m_listview)));
        m_treeview.SetFrameWindow(m_hWnd);
        m_listview.SetFrameWindow(m_hWnd);

        CMenuHandle sysmenu(GetSystemMenu(FALSE));
        if(sysmenu.IsMenu())
        {
            CString menuString;
            if(menuString.LoadString(IDS_ABOUT_MENUITEM))
            {
                ATLVERIFY(sysmenu.AppendMenu(MF_SEPARATOR));
                ATLVERIFY(sysmenu.AppendMenu(MF_STRING, IDS_ABOUT_DESCRIPTION, menuString));
            }
        }

        m_vsplit.SetSplitterPanes(m_treeview, m_listview);
        UpdateLayout();
        m_vsplit.SetSplitterPosPct(20);
        m_vsplit.m_cxyMin = 180; //  minimum size of the treeview

        CString oldDlgTitle;
        BOOL b = GetWindowText(oldDlgTitle.GetBufferSetLength(MAX_PATH), MAX_PATH);
        ATLTRACE2(_T("%u -> GetWindowText\n"), b);
        if(b)
        {
            ATLTRACE2(_T("Old title: %s\n"), oldDlgTitle.GetString());
            CString newDlgTitle;
            newDlgTitle.Format(_T("%s: version %s, rev: %s"), oldDlgTitle.GetString(), m_verinfo[_T("ProductVersion")], m_verinfo[_T("Mercurial revision")]);
            ATLTRACE2(_T("New title: %s\n"), newDlgTitle.GetString());
            ATLVERIFY(SetWindowText(newDlgTitle.GetString()));
        }

        AdjustStatusBarPaneWidths_();

        // register object for message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(NULL != pLoop);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);

        return 0;
    }

    virtual BOOL OnIdle()
    {
        if(m_bFirstOnIdle)
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
        ATLASSERT(NULL != pLoop);
        pLoop->RemoveMessageFilter(this);
        pLoop->RemoveIdleHandler(this);

        bHandled = FALSE;
        return 1;
    }

    LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL&)
    {
        LPMINMAXINFO lpMMI = reinterpret_cast<LPMINMAXINFO>(lParam);

        lpMMI->ptMinTrackSize.x = 640;
        lpMMI->ptMinTrackSize.y = 480;
        lpMMI->ptMaxTrackSize.x = ::GetSystemMetrics(SM_CXMAXTRACK);
        lpMMI->ptMaxTrackSize.y = ::GetSystemMetrics(SM_CYMAXTRACK);

        return 0;
    }

    LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CWaitCursor wc;
        (void)InterlockedExchange(&m_objroot_initialized, 0);
        if(m_treeview.EmptyAndRefill())
        {
            // Invalidate the cached items
            m_visitedList.RemoveAll();
            m_visitedListIndex = -1;
            (void)InterlockedExchange(&m_objroot_initialized, 1);
            // The following triggers TVN_SELCHANGED which in turn fills the listview
            m_treeview.SelectRoot();
        }
        return 0;
    }

    LRESULT OnShowAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CAboutDlg dlg(m_verinfo);
        dlg.DoModal(m_hWnd);
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
        if(IDOK == dlg.DoModal(m_hWnd))
        {
            ATLTRACE2(_T("The picked file is: %s\n"), dlg.m_szFileName);
            SaveAs_(dlg.m_szFileName, m_treeview.ObjectRoot());
        }
        return 0;
    }

    LRESULT OnFindObject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        if(m_bIsFindDialogOpen)
        {
            ATLTRACE2(_T("Find dialog is already open\n"));
            return 0;
        }
        m_bIsFindDialogOpen = true;

        // Prepare the find dialog
        return 0;
    }

    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        ATLTRACE2(_T("WM_SYSCOMMAND lParam: %lu\n"), wParam);
        if(IDS_ABOUT_DESCRIPTION == wParam)
            (void)OnShowAbout(0, 0, 0, bHandled);
        else
            bHandled = FALSE;
        return 0;
    }

    LRESULT OnAppCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
        SHORT cmd  = GET_APPCOMMAND_LPARAM(lParam);
        if(int sz = m_visitedList.GetSize())
        {
            ATLASSERT(sz > 0);
            switch(cmd)
            {
            case APPCOMMAND_BROWSER_BACKWARD:
            case APPCOMMAND_MEDIA_PREVIOUSTRACK:
                ATLTRACE2(_T("Navigate backward\n"));
                Navigate_(false);
                break;
            case APPCOMMAND_BROWSER_FORWARD:
            case APPCOMMAND_MEDIA_NEXTTRACK:
                ATLTRACE2(_T("Navigate forward\n"));
                Navigate_(true);
                break;
            }
        }

        bHandled = FALSE;
        return 0;
    }

    // lParam contains a Directory*
    LRESULT OnVisitDirectory(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        ATLASSERT(lParam != 0);
        if(Directory* dir = reinterpret_cast<Directory*>(lParam))
        {
            ATLTRACE2(_T("%s: %s\n"), _T(__FUNCTION__), dir->fullname().GetString());
            VisitDirectory_(dir);
            SetActiveObject_(dir);
        }
        return 0;
    }

    // lParam contains a Directory*
    LRESULT OnSelectTreeviewDirectory(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        ATLASSERT(lParam != 0);
        if (Directory* dir = reinterpret_cast<Directory*>(lParam))
        {
            ATLTRACE2(_T("%s: %s\n"), _T(__FUNCTION__), dir->fullname().GetString());
            m_treeview.SelectDirectory(dir);
        }
        return 0;
    }

    inline LRESULT OnSetActiveObject(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        if(GenericObject* obj = reinterpret_cast<GenericObject*>(lParam))
        {
            SetActiveObject_(obj);
        }
        return 0;
    }

    LRESULT OnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        WORD wID = LOWORD(wParam);
        if((MF_SYSMENU & HIWORD(wParam)) && (IDS_ABOUT_DESCRIPTION == wID))
        {
            const int cchBuff = 256;
            TCHAR szBuff[cchBuff] = { 0 };

            int nRet = ::LoadString(ModuleHelper::GetResourceInstance(), wID, szBuff, cchBuff);
            for(int i = 0; i < nRet; i++)
            {
                if(szBuff[i] == _T('\n'))
                {
                    szBuff[i] = 0;
                    break;
                }
            }
            ::SendMessage(m_hWndStatusBar, SB_SIMPLE, TRUE, 0L);
            ::SendMessage(m_hWndStatusBar, SB_SETTEXT, (255 | SBT_NOBORDERS), (LPARAM)szBuff);
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
        if(m_activeObject)
        {
            ATLTRACE2(_T("Last selected item: %s\n"), m_activeObject->fullname().GetString());
        }
        if(hWndCtl == m_listview.m_hWnd)
        {
            ATLTRACE2(_T("From ListView\n"));
        }
        else if(hWndCtl == m_treeview.m_hWnd)
        {
            ATLTRACE2(_T("From TreeView\n"));
        }
        else
        {
            ATLTRACE2(_T("From main menu or hotkey\n"));
        }
#endif

        if(m_activeObject)
        {
#           ifndef DDKBUILD
            // First time the user wants to see the properties (and security settings), we enable two privileges
            if(!m_Token.GetHandle())
            {
                if(
                    m_Token.GetThreadToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY)
                    || m_Token.GetProcessToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY)
                    )
                {
                    bool bNotAllAssigned = false;
                    ATLTRACE2(_T("Opened thread or process token: %p\n"), m_Token.GetHandle());
                    LPCTSTR requiredPrivileges[] = { SE_SECURITY_NAME, SE_TAKE_OWNERSHIP_NAME };
                    for(size_t i = 0; i < _countof(requiredPrivileges); i++)
                    {
                        if(m_Token.EnablePrivilege(requiredPrivileges[i], NULL, &bNotAllAssigned))
                        {
                            ATLTRACE2(_T("%s %s enabled (%d)\n"), requiredPrivileges[i], (bNotAllAssigned) ? _T("could not be") : _T("was successfully"), GetLastError());
                        }
                    }
                }
            }
#           endif // DDKBUILD
            ATLTRACE2(_T("last selected = %p -> %s\n"), m_activeObject, m_activeObject->fullname().GetString());
            CObjectPropertySheet objprop(m_activeObject, m_imagelist);
            (void)objprop.DoModal(m_hWnd);
        }

        return 0;
    }

private:

    void Navigate_(bool /*forward*/)
    {
        ATLTRACE2(_T("%s\n"), _T(__FUNCTION__));
    }

    inline void SetActiveObject_(GenericObject* obj)
    {
        m_activeObject = obj;
        SetStatusBarItem_(obj);
    }

    void AdjustStatusBarPaneWidths_()
    {
        int panes[] = { ID_DEFAULT_PANE, IDS_STATUSBAR_PANE1, IDS_STATUSBAR_PANE2 };
        m_status.SetPanes(panes, _countof(panes));
    }

    void VisitDirectory_(Directory* dir)
    {
#ifdef _DEBUG
        LPCWSTR fullName = (dir) ? dir->fullname().GetString() : _T("");
        ATLTRACE2(_T("Visiting Directory: '%s'\n"), fullName);
#endif // _DEBUG
        m_listview.FillFromDirectory(*dir);
        // Keep a list of visited directories
        AddToVisitedList_(dir);
    }

    void AddToVisitedList_(Directory* dir)
    {
        if(dir)
        {
            ATLASSERT(maxVisitedDepth > 0);
            // Should we trim the list?
            while(m_visitedList.GetSize() >= maxVisitedDepth)
            {
                ATLVERIFY(m_visitedList.RemoveAt(0));
                ATLTRACE2(_T("Trimmed visited list. New size: %i\n"), m_visitedList.GetSize());
            }
            ATLVERIFY(m_visitedList.Add(dir));
        }
    }

    inline void SetStatusBarItem_(GenericObject* obj)
    {
        LPCWSTR fullName = (obj) ? obj->fullname().GetString() : NULL;
        ::SetWindowText(m_hWndStatusBar, fullName);
    }

    class IObjectDirectoryDumper
    {
    public:
        virtual void SymlinkObject(const SymbolicLink* obj) = 0;
        virtual void ContainedObject(const GenericObject* obj) = 0;
        virtual void EnterDirectory(const Directory* obj) = 0;
        virtual void LeaveDirectory() = 0;
        virtual LPCTSTR getFileName() const = 0;

        virtual void operator()(Directory& current)
        {
            for (size_t i = 0; i < current.size(); i++)
            {
                GenericObject* entry = current[i];

                if (Directory* directory = dynamic_cast<Directory*>(entry))
                {
                    this->EnterDirectory(directory);
                    this->operator()(*directory);
                    this->LeaveDirectory();
                    continue;
                }
                if (SymbolicLink const* symlink = dynamic_cast<SymbolicLink const*>(entry))
                {
                    this->SymlinkObject(symlink);
                    continue;
                }
                this->ContainedObject(entry);
            }
        }
    };

#ifndef NTOBJX_NO_XML_EXPORT
    class CXmlObjectDirectoryDumper :
        public IObjectDirectoryDumper
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
            m_document.save_file(m_fileName.GetString());
        }

        void SymlinkObject(const SymbolicLink* obj)
        {
            pugi::xml_node node = AddStandardObject_(obj);
            ATLVERIFY(node.append_attribute(_T("target")).set_value(obj->target().GetString()));
        }

        void ContainedObject(const GenericObject* obj)
        {
            (void)AddStandardObject_(obj);
        }

        void EnterDirectory(const Directory* obj)
        {
            ATLASSERT(obj != NULL);
            m_previousNode = m_currentNode;
            m_currentNode = m_currentNode.append_child(_T(OBJTYPESTR_DIRECTORY));
            ATLVERIFY(m_currentNode.append_attribute(_T("name")).set_value(obj->name().GetString()));
        }

        void LeaveDirectory()
        {
            m_currentNode = m_previousNode;
        }

        LPCTSTR getFileName() const
        {
            return m_fileName.GetString();
        }

    private:
        pugi::xml_node AddStandardObject_(const GenericObject* obj)
        {
            ATLASSERT(obj != NULL);
            pugi::xml_node node = m_currentNode.append_child(_T("Object"));
            ATLVERIFY(node.append_attribute(_T("type")).set_value(obj->type().GetString()));
            ATLVERIFY(node.append_attribute(_T("name")).set_value(obj->name().GetString()));
            return node;
        }
    };
#endif // !NTOBJX_NO_XML_EXPORT

    class CTxtObjectDirectoryDumper :
        public IObjectDirectoryDumper
    {
        typedef IObjectDirectoryDumper baseClass;
        FILE* m_file;
        bool m_bOpened;
        CString m_fileName;
        CString m_currentPrefix;
        CString m_previousPrefix;
    public:
        CTxtObjectDirectoryDumper(LPCTSTR lpszFileName)
            : m_file(NULL)
            , m_bOpened(false)
            , m_fileName(lpszFileName)
            , m_currentPrefix(_T("\t"))
        {
            m_bOpened = (0 == _tfopen_s(&m_file, lpszFileName, _T("w+, ccs=UTF-8")));
            if (m_bOpened)
            {
                _ftprintf(m_file, _T("\\\n")); // Root of object manager namespace
            }
        }

        ~CTxtObjectDirectoryDumper()
        {
            ATLTRACE2(_T("Saving text document to %s.\n"), m_fileName.GetString());
            m_bOpened = (0 != fclose(m_file));
        }

        void SymlinkObject(const SymbolicLink* obj)
        {
            ATLASSERT(obj != NULL);
            LPCTSTR linePrefix = m_currentPrefix.GetString();
            _ftprintf(m_file, _T("%s%s [%s] -> %s\n"), linePrefix, obj->name().GetString(), obj->type().GetString(), obj->target().GetString());
        }

        void ContainedObject(const GenericObject* obj)
        {
            ATLASSERT(obj != NULL);
            LPCTSTR linePrefix = m_currentPrefix.GetString();
            _ftprintf(m_file, _T("%s%s [%s]\n"), linePrefix, obj->name().GetString(), obj->type().GetString());
        }

        void EnterDirectory(const Directory* obj)
        {
            ATLASSERT(obj != NULL);
            LPCTSTR linePrefix = m_currentPrefix.GetString();
            _ftprintf(m_file, _T("%s\\%s [%s]\n"), linePrefix, obj->name().GetString(), obj->type().GetString());
            m_previousPrefix = m_currentPrefix;
            m_currentPrefix.AppendChar(_T('\t'));
        }

        void LeaveDirectory()
        {
            m_currentPrefix = m_previousPrefix;
        }

        LPCTSTR getFileName() const
        {
            return m_fileName.GetString();
        }
    };

    void SaveAs_(LPCTSTR lpszFileName, Directory& objroot)
    {
        if (size_t len = (lpszFileName) ? _tcslen(lpszFileName) : 0)
        {
#ifndef NTOBJX_NO_XML_EXPORT
            if ((len > 4) && (0 == _tcsicmp(_T(".xml"), &lpszFileName[len -4])))
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
};
