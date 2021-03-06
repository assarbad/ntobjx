///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2016-2018 Oliver Schneider (assarbad.net)
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
#ifndef __EXEVERSION_H_VER__
#define __EXEVERSION_H_VER__ 2021072221

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#    pragma once
#endif

#include "hgid.h"

// ---------------------------------------------------------------------------
// Several defines have to be given before including this file. These are:
// ---------------------------------------------------------------------------
#define TEXT_AUTHOR        Oliver Schneider  // author (optional value)
#define PRD_MAJVER         1                 // major product version
#define PRD_MINVER         4                 // minor product version
#define PRD_PATCH          0                 // patch number
#define PRD_BUILD          HG_REV_NO         // build number for product
#define PRD_BUILD_NUMERIC  HG_REV_NO_NUMERIC // build number for product
#define FILE_MAJVER        PRD_MAJVER        // major file version
#define FILE_MINVER        PRD_MINVER        // minor file version
#define FILE_PATCH         PRD_PATCH         // patch number
#define FILE_BUILD         PRD_BUILD         // build number
#define FILE_BUILD_NUMERIC PRD_BUILD_NUMERIC // build number for product

// clang-format off
#define EXE_YEAR           2016-2018, 2021, 2022  // current year or timespan (e.g. 2003-2007)
#define TEXT_WEBSITE       https:/##/assarbad.net // website
// clang-format on

#define TEXT_PRODUCTNAME   NT Objects                               // product's name
#define TEXT_FILEDESC      Inspects the NT object manager namespace // component description
#define TEXT_COMPANY       Oliver Schneider(assarbad.net)           // company
#define TEXT_MODULE        ntobjx                                   // module name
#define TEXT_COPYRIGHT     Copyright \xA9 EXE_YEAR TEXT_AUTHOR      // copyright information
#define TEXT_INTERNALNAME  ntobjx.exe
#define HG_REPOSITORY      "https://sourceforge.net/p/ntobjx/code"
#define TEXT_PORTIONSCOPYRIGHT                                                                                         \
    "Portions (WTL, pugixml, FRISK utility classes), see source code for details:\r\nCopyright (c) Microsoft "         \
    "Corporation, WTL Team. All rights reserved.\r\nCopyright (c) Arseny Kapoulkine.\r\nCopyright (c) FRISK Software " \
    "International (FRISK). All rights reserved."

#define _ANSISTRING(text) #text
#define ANSISTRING(text)  _ANSISTRING(text)

#define _WIDESTRING(text) L##text
#define WIDESTRING(text)  _WIDESTRING(text)

#define PRESET_UNICODE_STRING(symbol, buffer) \
    UNICODE_STRING symbol = {sizeof(WIDESTRING(buffer)) - sizeof(WCHAR), sizeof(WIDESTRING(buffer)), WIDESTRING(buffer)};

#define CREATE_XVER(maj, min, patch, build) maj##, ##min##, ##patch##, ##build
#define CREATE_FVER(maj, min, patch, build) maj##.##min##.##patch##.##build
#define CREATE_PVER(maj, min, patch, build) maj##.##min##.##patch

#endif // __EXEVERSION_H_VER__
