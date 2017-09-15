#include <Windows.h>
#include <tchar.h>
#include <cstdio>
#include <atlbase.h>
#include <atlstr.h>
#include <delayimp.h>
#include "objmgr.hpp"

EXTERN_C int __cdecl DelayLoadError(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);
    return _vtprintf(lpszFormat, args);
};

using NtObjMgr::GenericObject;
using NtObjMgr::Directory;
using NtObjMgr::SymbolicLink;

void PrintDirectory(Directory& current, ATL::CString Prefix = _T(""))
{
    for(size_t i = 0; i < current.size(); i++)
    {
        GenericObject* entry = current[i];
        SymbolicLink const* symlink = dynamic_cast<SymbolicLink const*>(entry);
        Directory* directory = dynamic_cast<Directory*>(entry);

        if(directory)
        {
            CString newPrefix(Prefix + _T("  "));
            _tprintf(_T("%s%ws [%ws]\n"), Prefix.GetString(), directory->name().GetString(), directory->type().GetString());
            PrintDirectory(*directory, newPrefix);
        }
        else if(symlink)
        {
            _tprintf(_T("%s%ws [%ws] -> %ws\n"), Prefix.GetString(), symlink->name().GetString(), symlink->type().GetString(), symlink->target().GetString());
        }
        else
        {
            _tprintf(_T("%s%ws [%ws]\n"), Prefix.GetString(), entry->name().GetString(), entry->type().GetString());
        }
    }
}

#if TEST_RES_MAPPING
#include "objtypes.h"
#endif

int _tmain(int /*argc*/, _TCHAR** /*argv*/)
{
    Directory objmgr_root;
    PrintDirectory(objmgr_root);

#if TEST_RES_MAPPING
    for(size_t i = 0; i < sizeof(resIconTypeMapping)/sizeof(resIconTypeMapping[0]); i++)
    {
        _tprintf(_T("%ws [%u]\n"), resIconTypeMapping[i].typeName, resIconTypeMapping[i].resId);
    }
#endif

    return 0;
}
