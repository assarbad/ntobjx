///////////////////////////////////////////////////////////////////////////////
///
/// The GUI controls and main frame window classes for ntobjx.
///
/// Dual-licensed under MsPL and MIT license (see below).
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

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

namespace
{
    int const imgListElemWidth = 16;
    int const imgListElemHeight = 16;

    void CopyItemToClipboard(HWND hWndSource, int idCmd, GenericObject* obj)
    {
        if(::OpenClipboard(hWndSource))
        {
            ATL::CString s;
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
                    ATL::CString fullName(obj->fullname());
                    ATL::CString escapedName;
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
                ::SetClipboardData(CF_UNICODETEXT, hMem);
#   else
#       error "Not implemented for ANSI/CF_TEXT"
#   endif
            }
            else
            {
                ATLTRACE2(_T("Could not allocate memory using GlobalAlloc().\n"));
            }
            ATLVERIFY(::CloseClipboard());
        }
    }
}

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
        if(NULL != hVersionResource)
        {
            if(DWORD dwSize = ::SizeofResource(hInstance, hVersionResource))
            {
                if(HGLOBAL hVersionResourceData = ::LoadResource(hInstance, hVersionResource))
                {
                    if(LPVOID pVerInfoRO = ::LockResource(hVersionResourceData))
                    {
                        if(NULL != (m_lpVerInfo = ::LocalAlloc(LMEM_FIXED, dwSize)))
                        {
                            ::CopyMemory(m_lpVerInfo, pVerInfoRO, dwSize);
                            UINT uLen;
                            if(::VerQueryValue(m_lpVerInfo, _T("\\"), (LPVOID*)&m_pFixedFileInfo, &uLen))
                            {
                                ATLTRACE2(_T("%u.%u\n"), HIWORD(m_pFixedFileInfo->dwFileVersionMS), LOWORD(m_pFixedFileInfo->dwFileVersionMS));
                                DWORD* translations;
                                if(::VerQueryValue(m_lpVerInfo, _T("\\VarFileInfo\\Translation"), (LPVOID*)&translations, &uLen))
                                {
                                    size_t const numTranslations = uLen / sizeof(DWORD);
                                    ATLTRACE2(_T("Number of translations: %u\n"), (UINT)numTranslations);
                                    for(size_t i = 0; i < numTranslations; i++)
                                    {
                                        ATLTRACE2(_T("Translation %u: %08X\n"), (UINT)i, translations[i]);
                                        switch(LOWORD(translations[i]))
                                        {
                                        case MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL): // fallthrough
                                        case MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US):
                                            if(1200 == HIWORD(translations[i])) // only Unicode entries
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
        if(::VerQueryValue(m_lpVerInfo, fullName, (LPVOID*)&lpszBuf, &uLen))
        {
            ATLTRACE2(_T("Value: %s\n"), lpszBuf);
            return lpszBuf;
        }
        ATLTRACE2(_T("Value: NULL\n"));
        return NULL;
    }
};

class CAboutDlg :
    public CDialogImpl<CAboutDlg>
{
    CHyperLink m_website, m_projectpage;
    CVersionInfo m_verinfo;
public:
    enum { IDD = IDD_ABOUT };

    BEGIN_MSG_MAP(CAboutDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    END_MSG_MAP()

    CAboutDlg()
        : m_verinfo(_Module.GetModuleInstance())
    {
    }

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        ATL::CString website(m_verinfo[_T("Website")]);
        ATL::CString repo(m_verinfo[_T("Mercurial repository")]);
        ATL::CString revision(m_verinfo[_T("Mercurial revision")]);

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

        ATLVERIFY(SetDlgItemText(IDC_STATIC_COPYRIGHT, m_verinfo[_T("LegalCopyright")]));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_DESCRIPTION, m_verinfo[_T("FileDescription")]));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_PROGNAME, m_verinfo[_T("ProductName")]));
        ATLVERIFY(SetDlgItemText(IDC_STATIC_REVISION, m_verinfo[_T("Mercurial revision")]));

        ATL::CString caption(_T("About "));
        caption.Append(m_verinfo[_T("ProductName")]);
        caption.Append(_T(" "));
        caption.Append(m_verinfo[_T("FileVersion")]);
        SetWindowText(caption);

        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(wID);
        return 0;
    }
};

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
            CRect rect;
            ATLVERIFY(::GetClientRect(m_hWnd, &rect));
            ATLVERIFY(::InvalidateRect(m_hWnd, &rect, FALSE));
        }
    }
};

typedef CWinTraitsOR<WS_TABSTOP | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_INFOTIP, WS_EX_CLIENTEDGE, CControlWinTraits> CNtObjectsTreeViewTraits;
typedef CWinTraitsOR<WS_TABSTOP | LVS_SHAREIMAGELISTS | LVS_SINGLESEL, 0, CSortListViewCtrlTraits> CNtObjectsListViewTraits;

class CNtObjectsTreeView : public CWindowImpl<CNtObjectsTreeView, CTreeViewCtrlEx, CNtObjectsTreeViewTraits>
{
    bool m_bInitialized;
    Directory m_objroot;
    CImageList m_imagelist;
protected:
    typedef CWindowImpl<CNtObjectsTreeView, CTreeViewCtrlEx, CNtObjectsTreeViewTraits> baseClass;
public:
    DECLARE_WND_SUPERCLASS(_T("NtObjectsTreeView"), CTreeViewCtrlEx::GetWndClassName())

    BEGIN_MSG_MAP(CNtObjectsTreeView)
        MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRightButtonDown)
        NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnTVItemExpanding)
        NOTIFY_CODE_HANDLER(TVN_GETINFOTIP, OnGetInfoTip)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    CNtObjectsTreeView()
        : m_bInitialized(false)
    {}

    LRESULT OnRightButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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

            Directory* dir = reinterpret_cast<Directory*>(ti.GetData());
            if(dir)
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
                ATLTRACE2(_T("Sort children of %s (%p)\n"), dir->fullname(), pnmtv->itemNew.hItem);
            else
                ATLTRACE2(_T("Sort children of %p\n"), pnmtv->itemNew.hItem);
#endif // _DEBUG
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
                ATL::CString tip;
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

    Directory& ObjectRoot()
    {
        return m_objroot;
    }

    bool EmptyAndRefill()
    {
        if(m_objroot.refresh())
        {
            if(::IsWindow(m_hWnd) && m_imagelist.IsNull())
            {
                PrepareImageList_();
            }
            CSuppressRedraw suppressRedraw(m_hWnd);
            if(DeleteAllItems())
            {
                // Recursively fill the tree
                Fill_(NULL, m_objroot);
                // Sort the tree
                SortTreeItemChildren_(GetRootItem());
                (void)Expand(GetRootItem());
                (void)SelectItem(GetRootItem());
                // Signal that the tree has been initialized
                m_bInitialized=true;
                return m_bInitialized;
            }
        }
        // Something failed
        m_bInitialized=false;
        return m_bInitialized;
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
        WORD iconList[] = { IDI_OBJMGR_ROOT, IDI_DIRECTORY };
        ATLVERIFY(m_imagelist.Create(imgListElemWidth, imgListElemHeight, ILC_COLOR32 | ILC_MASK, (int)_countof(iconList), (int)_countof(iconList)));
        for(size_t i = 0; i < _countof(iconList); i++)
        {
            HICON hIcon = static_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(iconList[i]), IMAGE_ICON, imgListElemWidth, imgListElemHeight, LR_CREATEDIBSECTION));
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
            parent = InsertItem(TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_PARAM, current.fullname(), 0, 0, 0, 0, reinterpret_cast<LPARAM>(&m_objroot), TVI_ROOT, TVI_LAST);
        }
        for(size_t i = 0; i < current.size(); i++)
        {
            Directory* entry = dynamic_cast<Directory*>(current[i]);
            if(entry)
            {
                HTREEITEM curritem = InsertItem(TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_PARAM, entry->name(), 1, 1, 0, 0, reinterpret_cast<LPARAM>(entry), parent, TVI_LAST);
                Fill_(curritem, *entry);
            }
        }
    }
};

class CNtObjectsListView : public CSortListViewCtrlImpl<CNtObjectsListView, CListViewCtrl, CNtObjectsListViewTraits>
{
    CImageList m_imagelist;
protected:
    typedef CSortListViewCtrlImpl<CNtObjectsListView, CListViewCtrl, CNtObjectsListViewTraits> baseClass;
public:
    DECLARE_WND_SUPERCLASS(_T("NtObjectsListView"), CListViewCtrl::GetWndClassName())

    BEGIN_MSG_MAP(CNtObjectsListView)
        MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRightButtonDown)
        NOTIFY_CODE_HANDLER(HDN_ITEMCLICKA, OnHeaderItemClick)
        NOTIFY_CODE_HANDLER(HDN_ITEMCLICKW, OnHeaderItemClick)
        DEFAULT_REFLECTION_HANDLER()
        CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    CNtObjectsListView()
        : baseClass()
    {}

    void FillFromDirectory(Directory& current)
    {
        CSuppressRedraw suppressRedraw(m_hWnd);
        DeleteAllItems();
        if(::IsWindow(m_hWnd) && m_imagelist.IsNull())
        {
            PrepareImageList_();
        }
        ResetAllColumns_();
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
                , GetImageIndexByType_(current[i]->type())
                , reinterpret_cast<LPARAM>(current[i])
                );
            widths[0] = max(GetStringWidth(current[i]->name()), widths[0]);
            AddItem(
                currItem
                , 1
                , current[i]->type()
                );
            widths[1] = max(GetStringWidth(current[i]->type()), widths[1]);
            ATLTRACE2(_T("%s:%i [%s:%i] / %i:%i\n"), current[i]->name(), GetStringWidth(current[i]->name()), current[i]->type(), GetStringWidth(current[i]->type()), widths[0], widths[1]);
            SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(current[i]);
            if(symlink)
            {
                AddItem(
                    currItem
                    , 2
                    , symlink->target()
                    );
                widths[2] = max(GetStringWidth(symlink->target()), widths[2]);
                ATLTRACE2(_T("    %s:%i [%i]\n"), symlink->target(), GetStringWidth(symlink->target()), widths[2]);
            }
        }
        CRect rect;
        ATLVERIFY(GetWindowRect(&rect));
        int const maxWidths[] = {rect.right-rect.left - 50, 150, 200};
        // Try to set an optimum column width
        for(int i = 0; i <  _countof(widths); i++)
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

    LRESULT OnRightButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CPoint pt;
        ATLVERIFY(::GetCursorPos(&pt));
        ATLTRACE2(_T("Point of click: %d/%d\n"), pt.x, pt.y);

        // Find where the user clicked, so we know the item
        LVHITTESTINFO lvhtti = {0};
        lvhtti.pt = pt;
        ATLVERIFY(ScreenToClient(&lvhtti.pt));
        HitTest(&lvhtti);
        if(-1 != lvhtti.iItem)
        {
            // If we found an item, select it
            ATLVERIFY(SelectItem(lvhtti.iItem));

            LVITEM item = {0};
            item.iItem = lvhtti.iItem;
            item.mask = LVIF_PARAM;
            ATLVERIFY(GetItem(&item));
            GenericObject* itemobj = reinterpret_cast<GenericObject*>(item.lParam);
            SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(itemobj);

            // Load menu resource
            CMenu menu;
            ATLVERIFY(menu.LoadMenu(IDR_POPUP_MENU1));
            // Get the popup menu at index 0 from the menu resource
            CMenuHandle popup = menu.GetSubMenu((symlink) ? 1 : 0);

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
                default:
                    ATLTRACE2(_T("Chosen command: %i\n"), idCmd);
                    break;
                }

            }

        }

        return 0;
    }

    LRESULT OnHeaderItemClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
    {
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

    // If item1 > item2 return 1, if item1 < item2 return -1, else return 0.
    int CompareItemsCustom(LVCompareParam* pItem1, LVCompareParam* pItem2, int iSortCol)
    {
        if(!iSortCol)
        {
            GenericObject* obj1 = reinterpret_cast<GenericObject*>(pItem1->dwItemData);
            GenericObject* obj2 = reinterpret_cast<GenericObject*>(pItem2->dwItemData);
            bool isDir1 = (0 == _tcsicmp(_T("Directory"), obj1->type()));
            bool isDir2 = (0 == _tcsicmp(_T("Directory"), obj2->type()));
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

    // Retrieve the image index based on the type string
    int GetImageIndexByType_(LPCTSTR typeName)
    {
        int retval = 0;
        for(size_t i = 0; i < resIconTypeMappingSize; i++)
        {
            if(0 == _tcsicmp(typeName, resIconTypeMapping[i].typeName))
            {
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
        for(int i = 0; i < iResIconTypeMappingSize; i++)
        {
            HICON hIcon = static_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(resIconTypeMapping[i].resId), IMAGE_ICON, imgListElemWidth, imgListElemHeight, LR_CREATEDIBSECTION));
            ATLTRACE2(_T("Icon handle: %p - %s\n"), hIcon, resIconTypeMapping[i].typeName);
            m_imagelist.AddIcon(hIcon);
        }
        (void)SetImageList(m_imagelist, LVSIL_SMALL);
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
        // FIXME: later we may want to reinitialize columns when switching UI languages
        if(GetColumnCount() < _countof(columnDefaults))
        {
            DeleteAllColumns_();
            ATL::CString columnName;
            for(int idx = 0; idx < _countof(columnDefaults); idx++)
            {
                columnName.LoadString(columnDefaults[idx].resId);
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

class CNtObjectsMainFrame : 
    public CFrameWindowImpl<CNtObjectsMainFrame>,
    public CUpdateUI<CNtObjectsMainFrame>,
    public CMessageFilter,
    public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(_T("NtObjectsMainFrame"), IDR_MAINFRAME)

    CSplitterWindow m_vsplit;
    CNtObjectsTreeView m_treeview;
    CNtObjectsListView m_listview;
    LONG m_tree_initialized; // only write to this member using Interlocked*()
    bool m_bFirstOnIdle;
    UINT_PTR const m_AboutMenuItem;
    ATL::CString m_lastSelected;
    HRESULT (CALLBACK* DllGetVersion)(DLLVERSIONINFO *);

    CNtObjectsMainFrame()
        : m_bFirstOnIdle(true) // to force initial refresh
        , m_AboutMenuItem(::RegisterWindowMessage(_T("{2F95CC77-8F3F-4880-AA09-FDE7D65BA526}")))
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
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)
        COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
        COMMAND_ID_HANDLER(ID_SHOW_ABOUT, OnShowAbout)
        NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnLVItemChanged)
        NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTVSelChanged)
        CHAIN_MSG_MAP(CUpdateUI<CNtObjectsMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CNtObjectsMainFrame>)
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        m_tree_initialized = 0;

        CreateSimpleStatusBar();
        m_hWndClient = m_vsplit.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        // Add WS_EX_CONTROLPARENT such that tab stops work
        LONG exStyle = ::GetWindowLong(m_hWndClient, GWL_EXSTYLE);
        ::SetWindowLong(m_hWndClient, GWL_EXSTYLE, exStyle | WS_EX_CONTROLPARENT);
        // The splitter should be smaller than the default width
        m_vsplit.m_cxySplitBar = 3;

        ATLVERIFY(NULL != m_treeview.Create(m_hWndClient));
        ATLVERIFY(NULL != m_listview.Create(m_hWndClient));
        if(DllGetVersion)
        {
            DLLVERSIONINFO dllvi = {sizeof(DLLVERSIONINFO), 0, 0, 0, 0};
            if(SUCCEEDED(DllGetVersion(&dllvi)) && dllvi.dwMajorVersion >= 6)
                m_listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
            else
                m_listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
        }
        else
        {
            m_listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
        }
        ATLTRACE2(_T("Control ID for splitter: %i\n"), ::GetDlgCtrlID(m_vsplit));
        ATLTRACE2(_T("Control ID for treeview: %i\n"), ::GetDlgCtrlID(m_treeview));
        ATLTRACE2(_T("Control ID for listview: %i\n"), ::GetDlgCtrlID(m_listview));
        ATLASSERT(m_vsplit.m_hWnd == ::GetDlgItem(m_hWnd, ::GetDlgCtrlID(m_vsplit)));
        ATLASSERT(m_treeview.m_hWnd == ::GetDlgItem(m_hWndClient, ::GetDlgCtrlID(m_treeview)));
        ATLASSERT(m_listview.m_hWnd == ::GetDlgItem(m_hWndClient, ::GetDlgCtrlID(m_listview)));

        CMenuHandle sysmenu(GetSystemMenu(FALSE));
        if(sysmenu.IsMenu())
        {
            ATL::CString menuString;
            if(menuString.LoadString(IDS_ABOUT_MENUITEM))
            {
                ATLVERIFY(sysmenu.AppendMenu(MF_SEPARATOR));
                ATLVERIFY(sysmenu.AppendMenu(MF_STRING, m_AboutMenuItem, menuString));
            }
        }

        m_vsplit.SetSplitterPanes(m_treeview, m_listview);
        UpdateLayout();
        m_vsplit.SetSplitterPosPct(20);
        m_vsplit.m_cxyMin = 180; //  minimum size of the treeview

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

    LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        PostMessage(WM_CLOSE);
        return 0;
    }

    LRESULT OnViewProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
    {
#ifndef _DEBUG
        UNREFERENCED_PARAMETER(wID);
        UNREFERENCED_PARAMETER(wNotifyCode);
#endif
        ATLTRACE2(_T("hWndCtl = %p; wNotifyCode = %u (%04X); wID = %u (%04X)\n"), hWndCtl, wNotifyCode, wNotifyCode, wID, wID);
#ifdef _DEBUG
        if(!m_lastSelected.IsEmpty())
        {
            ATLTRACE2(_T("Last selected item: %s\n"), m_lastSelected.GetString());
        }
#endif

        if(hWndCtl == m_treeview.m_hWnd)
        {
            AtlMessageBox(m_hWnd, _T("Properties from TreeView"));
        }
        else if(hWndCtl == m_listview.m_hWnd)
        {
            AtlMessageBox(m_hWnd, _T("Properties from ListView"));
        }
        else
        {
            AtlMessageBox(m_hWnd, _T("Properties via hotkey or menu?!"));
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
            if(GenericObject* obj = reinterpret_cast<GenericObject*>(pnmlv->lParam))
            {
                SetSelected_(obj->fullname());
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
        if(m_tree_initialized && dir)
        {
            m_listview.FillFromDirectory(*dir);
            SetSelected_(dir->fullname());
            bHandled = TRUE;
        }
        return 0;
    }

    LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CWaitCursor wc;
        (void)InterlockedExchange(&m_tree_initialized, 0);
        if(m_treeview.EmptyAndRefill())
        {
            m_listview.FillFromDirectory(m_treeview.ObjectRoot());
            SetSelected_(m_treeview.ObjectRoot().fullname());
            (void)InterlockedExchange(&m_tree_initialized, 1);
        }
        return 0;
    }

    LRESULT OnShowAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CAboutDlg dlg;
        dlg.DoModal();
        return 0;
    }

    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        ATLTRACE2(_T("WM_SYSCOMMAND lParam: %lu\n"), wParam);
        if(m_AboutMenuItem == (UINT_PTR)wParam)
            (void)OnShowAbout(0, 0, 0, bHandled);
        else
            bHandled = FALSE;
        return 0;
    }

private:

    void SetSelected_(LPCTSTR fullName)
    {
        m_lastSelected = fullName;
        ::SetWindowText(m_hWndStatusBar, m_lastSelected);
    }
};
