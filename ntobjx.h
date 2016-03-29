// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

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
}

typedef CWinTraitsOR<TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_INFOTIP, WS_EX_CLIENTEDGE, CControlWinTraits> CNtObjectsTreeViewTraits;
typedef CWinTraitsOR<LVS_SHAREIMAGELISTS | LVS_SINGLESEL, 0, CSortListViewCtrlTraits> CNtObjectsListViewTraits;

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

    LRESULT OnRightButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        CPoint pt;
        ATLVERIFY(::GetCursorPos(&pt));
        ATLTRACE2(_T("Point of click: %d/%d\n"), pt.x, pt.y);

        bHandled = TRUE;
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
            ATLTRACE2(_T("Sort children of %p\n"), pnmtv->itemNew.hItem);
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
            if(DeleteAllItems())
            {
                // Recursively fill the tree
                Fill_(NULL, m_objroot);
                // Sort the tree
                SortTreeItemChildren_(GetRootItem());
                (void)Expand(GetRootItem());
                (void)SelectItem(GetRootItem());
                // Signal that the tree has been initialized
                return (m_bInitialized=true); // assignment intentional
            }
        }
        // Something failed
        return (m_bInitialized=false); // assignment intentional
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
        ATLVERIFY(m_imagelist.Create(imgListElemWidth, imgListElemHeight, 0, 1, 1));
        WORD iconList[] = { IDI_OBJMGR_ROOT, IDI_DIRECTORY };
        CRect rect(0, 0, imgListElemWidth * _countof(iconList), imgListElemHeight);
        CMemoryDC memdc(GetDC(), rect);
        for(size_t i = 0; i < _countof(iconList); i++)
        {
            CIcon icon(static_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(iconList[i]), IMAGE_ICON, imgListElemWidth, imgListElemHeight, LR_DEFAULTCOLOR)));
            ATLVERIFY(!icon.IsNull());
            ATLVERIFY(memdc.DrawIconEx(static_cast<int>(i) * imgListElemWidth, 0, icon, imgListElemWidth, imgListElemHeight));
        }
        ATLVERIFY(-1 != m_imagelist.Add(memdc.GetCurrentBitmap()));
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
        CHAIN_MSG_MAP(baseClass)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    CNtObjectsListView()
        : baseClass()
    {}

    void FillFromDirectory(Directory& current)
    {
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
            SymbolicLink* symlink = dynamic_cast<SymbolicLink*>(current[i]);
            if(symlink)
            {
                AddItem(
                    currItem
                    , 2
                    , symlink->target()
                    );
                widths[2] = max(GetStringWidth(symlink->target()), widths[2]);
            }
        }
        CRect rect;
        ATLVERIFY(GetWindowRect(&rect));
        int const maxWidths[] = {rect.right-rect.left - 50, 100, 200};
        // Try to set an optimum column width
        for(int i = 0; i <  _countof(widths); i++)
        {
            if(-1 != widths[i])
            {
                ATLVERIFY(SetColumnWidth(i, min(widths[i] + 0x10, maxWidths[i])));
            }
        }
        // Sort according to name column
        SortItems(0);
    }

    LRESULT OnRightButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
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
                    CopyItemToClipboard_(idCmd, itemobj);
                    break;
                case ID_POPUPMENU_PROPERTIES:
                    ::SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(ID_VIEW_PROPERTIES, 0), NULL);
                    break;
                default:
                    ATLTRACE2(_T("Chosen command: %i\n"), idCmd);
                    break;
                }

            }

        }

        bHandled = TRUE;
        return 0;
    }

    LRESULT OnHeaderItemClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
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
        bHandled = TRUE;
        return 0;
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
        const int iResIconTypeMappingSize = static_cast<int>(resIconTypeMappingSize);
        ATLVERIFY(m_imagelist.Create(imgListElemWidth, imgListElemHeight, 0, iResIconTypeMappingSize, iResIconTypeMappingSize));
        CRect rect(0, 0, imgListElemWidth * iResIconTypeMappingSize, imgListElemHeight);
        CMemoryDC memdc(GetDC(), rect);
        for(int i = 0; i < iResIconTypeMappingSize; i++)
        {
            CIcon icon(static_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(resIconTypeMapping[i].resId), IMAGE_ICON, imgListElemWidth, imgListElemHeight, LR_DEFAULTCOLOR)));
            ATLVERIFY(!icon.IsNull());
            ATLVERIFY(memdc.DrawIconEx(i * imgListElemWidth, 0, icon, imgListElemWidth, imgListElemHeight));
        }
        ATLVERIFY(-1 != m_imagelist.Add(memdc.GetCurrentBitmap()));
        (void)SetImageList(m_imagelist, LVSIL_SMALL);
    }

    void DeleteAllColumns_()
    {
        ATLTRACE2(_T("Column count: %i\n"), GetColumnCount());
        // On older Windows/comctl versions it's impossible to remove column 0
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
                    ATLTRACE2(_T("Inserted column %i with name \"%s\".\n"), idx, columnName.GetString());
                }
                else
                {
                    LVCOLUMN lvc = {0};
                    lvc.mask = LVCF_TEXT;
                    lvc.pszText = const_cast<LPTSTR>(columnName.GetString());
                    lvc.cchTextMax = columnName.GetLength() + 1;
                    SetColumn(idx, &lvc);
                }
            }
        }
    }

    void CopyItemToClipboard_(int idCmd, GenericObject* obj)
    {
        if(OpenClipboard())
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
};

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

    CNtObjectsMainFrame()
        : m_bFirstOnIdle(true) // to force initial refresh
    {
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        return CFrameWindowImpl<CNtObjectsMainFrame>::PreTranslateMessage(pMsg);
    }

    BEGIN_UPDATE_UI_MAP(CNtObjectsMainFrame)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CNtObjectsMainFrame)
        CHAIN_MSG_MAP_MEMBER(m_treeview)
        CHAIN_MSG_MAP_MEMBER(m_listview)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)
        COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
        NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTVSelChanged)
        CHAIN_MSG_MAP(CUpdateUI<CNtObjectsMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CNtObjectsMainFrame>)
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        m_tree_initialized = 0;

        CreateSimpleStatusBar();
        m_hWndClient = m_vsplit.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

        ATLVERIFY(NULL != m_treeview.Create(m_vsplit));
        ATLVERIFY(NULL != m_listview.Create(m_vsplit));
        m_listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

        m_vsplit.SetSplitterPanes(m_treeview, m_listview);
        UpdateLayout();
        m_vsplit.SetSplitterPosPct(20);
        m_vsplit.m_cxyMin = 180; //  minimum size of the treeview

        // register object for message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
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
        ATLASSERT(pLoop != NULL);
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

    LRESULT OnViewProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        /* TODO */
        AtlMessageBox(m_hWnd, _T("Properties"));
        return 0;
    }

    LRESULT OnTVSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
    {
        if(m_tree_initialized)
        {
            LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);
            Directory* dir = reinterpret_cast<Directory*>(pnmtv->itemNew.lParam);
            if(dir != NULL)
            {
                m_listview.FillFromDirectory(*dir);
            }
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
            (void)InterlockedExchange(&m_tree_initialized, 1);
        }
        return 0;
    }

};

/*

EnumWindowStations
EnumDesktops
*/