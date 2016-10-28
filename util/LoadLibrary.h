////////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2007-2011 FRISK Software International (FRISK)
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
/// - Redistributions of source code must retain the above copyright notice,
///   this list of conditions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
/// - Neither the name of FRISK Software International nor the names of its
///   contributors may be used to endorse or promote products derived from this
///   software without specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
/// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
/// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
/// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
/// POSSIBILITY OF SUCH DAMAGE.
///
///
/// Note: This is the newer (simplified) version of the BSD license.
///
////////////////////////////////////////////////////////////////////////////////
#ifndef __LOADLIBRARY_H_VER__
#define __LOADLIBRARY_H_VER__ 2016102019
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif // Check for "#pragma once" support
#include <Windows.h>
#include <tchar.h>
#if (__SIMPLEBUFFER_H_VER__ < 2016102019)
#   include "SimpleBuffer.h"
#endif
// Save the macro, in case it was declared elsewhere
#pragma push_macro("CLL_VERBOSE_VTRACE")
#if !defined(VTRACE)
#   ifdef __BORLANDC__ // special treatment for BCB, as so often :-/
#       include <stdio.h>
#   else
#       include <cstdio>
#   endif // __BORLANDC__
#   include "tracer.h"
#   include "fpav_macros.h"
#   ifdef VERBOSE_CLOADLIBRARY_DEBUGGING
#       define CLL_VERBOSE_VTRACE VTRACE
#   else // VERBOSE_CLOADLIBRARY_DEBUGGING
#       define CLL_VERBOSE_VTRACE(...) while(false) {}
#   endif // VERBOSE_CLOADLIBRARY_DEBUGGING
#else
#   define CLL_VERBOSE_VTRACE(...) while(false) {}
#endif



class CLoadLibrary
{
    CLoadLibrary(const CLoadLibrary&);
    CLoadLibrary operator=(const CLoadLibrary&);

    // Little helper class that wraps API calls depending on the used character pointer type
    class Helper
    {
    public:
        template<typename CharType> static DWORD WrapFormatMessage_(HMODULE lpSource, DWORD dwMessageId, WCHAR** lpBuffer, va_list* Arguments)
        {
            return ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, lpSource, dwMessageId, 0, (LPWSTR)(lpBuffer), 0x20, Arguments);
        }
        template<typename CharType> static DWORD WrapFormatMessage_(HMODULE lpSource, DWORD dwMessageId, CHAR** lpBuffer, va_list* Arguments)
        {
            return ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, lpSource, dwMessageId, 0, (LPSTR)(lpBuffer), 0x20, Arguments);
        }
        template<typename CharType> static DWORD WrapFormatSysMessage_(DWORD dwMessageId, WCHAR** lpBuffer, va_list* Arguments)
        {
            return ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwMessageId, 0, (LPWSTR)(lpBuffer), 0x20, Arguments);
        }
        template<typename CharType> static DWORD WrapFormatSysMessage_(DWORD dwMessageId, CHAR** lpBuffer, va_list* Arguments)
        {
            return ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwMessageId, 0, (LPSTR)(lpBuffer), 0x20, Arguments);
        }
        template<typename CharType> static DWORD WrapGetModuleFileName_(HMODULE hModule, WCHAR* lpFilename, DWORD nSize)
        {
            return ::GetModuleFileNameW(hModule, lpFilename, nSize);
        }
        template<typename CharType> static DWORD WrapGetModuleFileName_(HMODULE hModule, CHAR* lpFilename, DWORD nSize)
        {
            return ::GetModuleFileNameA(hModule, lpFilename, nSize);
        }
        template<typename CharType> static DWORD WrapGetFullPathName_(const WCHAR* lpFileName, DWORD nBufferLength, WCHAR* lpBuffer, WCHAR** lpFilePart)
        {
            return ::GetFullPathNameW(lpFileName, nBufferLength, lpBuffer, lpFilePart);
        }
        template<typename CharType> static DWORD WrapGetFullPathName_(const CHAR* lpFileName, DWORD nBufferLength, CHAR* lpBuffer, CHAR** lpFilePart)
        {
            return ::GetFullPathNameA(lpFileName, nBufferLength, lpBuffer, lpFilePart);
        }
        template<typename CharType> static int WrapLoadString_(HINSTANCE hInstance, UINT uID, WCHAR* lpBuffer, int nBufferMax)
        {
            return ::LoadStringW(hInstance, uID, lpBuffer, nBufferMax);
        }
        template<typename CharType> static int WrapLoadString_(HINSTANCE hInstance, UINT uID, CHAR* lpBuffer, int nBufferMax)
        {
            return ::LoadStringA(hInstance, uID, lpBuffer, nBufferMax);
        }
        template<typename CharType> static WCHAR* lastSlash(WCHAR* string)
        {
            WCHAR* lastslash = 0;
            if((0 != (lastslash = ::wcsrchr(string, L'\\'))) || (0 != (lastslash = ::wcsrchr(string, L'/'))))
            {
                WCHAR alternate = L'/';
                switch(*lastslash)
                {
                case L'/':
                    alternate = L'\\';
                    break;
                default:
                    break;
                }
                if(WCHAR* evenlater = ::wcsrchr(string, alternate))
                {
                    return evenlater;
                }
            }
            return lastslash;
        }
        template<typename CharType> static CHAR* lastSlash(CHAR* string)
        {
            CHAR* lastslash = 0;
            if((0 != (lastslash = ::strrchr(string, '\\'))) || (0 != (lastslash = ::strrchr(string, '/'))))
            {
                CHAR alternate = '/';
                switch(*lastslash)
                {
                case '/':
                    alternate = '\\';
                    break;
                default:
                    break;
                }
                if(CHAR* evenlater = ::strrchr(string, alternate))
                {
                    return evenlater;
                }
            }
            return lastslash;
        }
    };
public:
    CLoadLibrary(HMODULE module = 0)
        : m_module(module)
        , m_AsDataFile(false)
        , m_NeedToFree(0 == module)
    {
        VTRACE("Loading with%s module handle (default ctor) -> %p", (module) ? "" : "out", module);
    }
    CLoadLibrary(LPCTSTR LibName, bool AsDataFile = false)
        : m_module(0)
        , m_AsDataFile(false)
        , m_NeedToFree(false)
    {
        Load(LibName, NULL, AsDataFile);
    }
    CLoadLibrary(LPCTSTR LibName, LPCTSTR DllDirectory, bool AsDataFile = false)
        : m_module(0)
        , m_AsDataFile(false)
        , m_NeedToFree(false)
    {
        Load(LibName, DllDirectory, AsDataFile);
    }
    ~CLoadLibrary()

    {
        (void)Free(); //lint !e1551
    }
    inline bool IsOK() const
    {
        return (0 != m_module);
    }
    void Load(LPCTSTR LibName, LPCTSTR DllDirectory, bool AsDataFile = false)
    {
        if(!LibName)
        {
            VTRACE(TEXT("Null pointer was passed as DLL name"));
            return;
        }
        m_requestedFile = LibName;
        m_AsDataFile = AsDataFile;
        // Initial flags for LoadLibraryEx
        DWORD dwLoadFlags = (m_AsDataFile) ? LOAD_LIBRARY_AS_DATAFILE : 0;
        CSimpleBuf<TCHAR> dllFilePath(LibName);
        // If the directory was given, we do a little special handling here
        if(DllDirectory && (0 < lstrlen(DllDirectory)))
        {
            CSimpleBuf<TCHAR> fullDirPath(MAX_PATH);
            DWORD dwNeeded = ::GetFullPathName(DllDirectory, fullDirPath.Length<DWORD>(), fullDirPath, NULL);
            // Check that we got all of it
            if(dwNeeded > fullDirPath.Length<DWORD>())
            {
                // ... nope, adjust the buffer size and do it again
                if(fullDirPath.ReAlloc(dwNeeded))
                {
                    dwNeeded = ::GetFullPathName(DllDirectory, fullDirPath.Length<DWORD>(), fullDirPath, NULL);
                }
#ifdef _DEBUG
                else
                {
                    VTRACE("Apparently we are out of memory ...");
                    (void)fullDirPath.ReAlloc(0);
                }
#endif // _DEBUG
            }
            // If we got something, load the DLL in the proper way ...
            if(fullDirPath.Length<size_t>() && (dwNeeded <= fullDirPath.Length<DWORD>()))
            {
                m_requestedDllDirectory = DllDirectory;
                dwLoadFlags |= LOAD_WITH_ALTERED_SEARCH_PATH;
                // Now assemble the absolute path to the DLL
                dllFilePath = fullDirPath;
                dllFilePath += TEXT("\\");
                dllFilePath += LibName;
            }
#ifdef _DEBUG
            else
            {
                VTRACE(TEXT("I think the MSDN documentation has a flaw ..."));
            }
#endif // _DEBUG
        }
        VTRACE(TEXT("Loading: \"%s\" (as data file == %d)"), dllFilePath.Buffer(), AsDataFile);
        m_module = ::LoadLibraryEx(dllFilePath, NULL, dwLoadFlags);
        VTRACE(TEXT("Result of LoadLibraryEx -> %p (error code: %d)"), m_module, ::GetLastError());
        m_NeedToFree = (0 != m_module);
    }
    inline void Load(LPCTSTR LibName, bool AsDataFile = false)
    {
        Load(LibName, NULL, AsDataFile);
    }
    inline bool Free() /*lint -sem(CLoadLibrary::Free,cleanup) */
    {
        if(m_NeedToFree && m_module)
        {
#ifdef _DEBUG
            CSimpleBuf<TCHAR> name(getModuleBaseName<TCHAR>(m_module).Buffer());
            VTRACE(TEXT("Freeing module: %p (%s)"), m_module, (name.Buffer()) ? name.Buffer() : getRequestedDllName());
#endif // _DEBUG
            return (FALSE != ::FreeLibrary(Detach()));
        }
        return false;
    }
    template<typename CharType> CSimpleBuf<CharType> getModuleFileName() const
    {
        if(getHandle())
        {
            return getModuleFileName<CharType>(getHandle());
        }
        return size_t(0);
    }
    CSimpleBuf<CHAR> getModuleFileNameA() { return getModuleFileName<CHAR>(); }
    CSimpleBuf<WCHAR> getModuleFileNameW() { return getModuleFileName<WCHAR>(); }
    template<typename CharType> CSimpleBuf<CharType> getModuleBaseName() const
    {
        if(getHandle())
        {
            return getModuleBaseName<CharType>(getHandle());
        }
        return size_t(0);
    }
    CSimpleBuf<CHAR> getModuleBaseNameA() { return getModuleBaseName<CHAR>(); }
    CSimpleBuf<WCHAR> getModuleBaseNameW() { return getModuleBaseName<WCHAR>(); }
    template<typename CharType> CSimpleBuf<CharType> getModuleFilePath() const
    {
        if(getHandle())
        {
            return getModuleFilePath<CharType>(getHandle());
        }
        return size_t(0);
    }
    CSimpleBuf<CHAR> getModuleFilePathA() { return getModuleFilePath<CHAR>(); }
    CSimpleBuf<WCHAR> getModuleFilePathW() { return getModuleFilePath<WCHAR>(); }
    // Retrieves the full path (i.e. rooted) to the module file
    template<typename CharType> static CSimpleBuf<CharType> getModuleFileName(HMODULE module)
    {
        CSimpleBuf<CharType> temp(MAX_PATH);
        DWORD dwLength = 0;
        while(0 != (dwLength = Helper::WrapGetModuleFileName_<CharType>(module, temp, temp.Length<DWORD>()))
            && (ERROR_INSUFFICIENT_BUFFER == ::GetLastError()))
        {
            temp.ReAlloc(dwLength * 2);
        }
        if(dwLength)
        {
            // Replace any forward slashes by back slashes
            for(size_t i = 0; i < temp.LengthSZ<size_t>(); i++)
            {
                if(TEXT('/') == temp.Buffer()[i])
                {
                    temp.Buffer()[i] = TEXT('\\');
                }
            }
            CSimpleBuf<CharType> temp2(temp.Length<size_t>());
            while((0 != (dwLength = Helper::WrapGetFullPathName_<CharType>(temp, temp2.Length<DWORD>(), temp2, 0)))
                && (dwLength > temp2.Length<DWORD>()))
            {
                temp2.ReAlloc(1+dwLength);
            }
            if(dwLength)
            {
                return temp2;
            }
            return temp;
        }
        return size_t(0);
    }
    static CSimpleBuf<CHAR> getModuleFileNameA(HMODULE module) { return getModuleFileName<CHAR>(module); }
    static CSimpleBuf<WCHAR> getModuleFileNameW(HMODULE module) { return getModuleFileName<WCHAR>(module); }
    // Retrieves the file name portion of the path
    template<typename CharType> static CSimpleBuf<CharType> getModuleBaseName(HMODULE module)
    {
        CSimpleBuf<CharType> temp(getModuleFileName<CharType>(module));
        CharType* lastslash = 0;
        if((temp) && (temp.LengthSZ<size_t>()) && (0 != (lastslash = const_cast<CharType*>(Helper::lastSlash<CharType>(temp)))))
        {
            return ++lastslash; // Get everything behind the slash
        }
        return temp;
    }
    static CSimpleBuf<CHAR> getModuleBaseNameA(HMODULE module) { return getModuleBaseName<CHAR>(module); }
    static CSimpleBuf<WCHAR> getModuleBaseNameW(HMODULE module) { return getModuleBaseName<WCHAR>(module); }
    // Retrieves the path to the module (without trailing backslash)
    template<typename CharType> static CSimpleBuf<CharType> getModuleFilePath(HMODULE module)
    {
        CSimpleBuf<CharType> temp(getModuleFileName<CharType>(module));
        CharType* lastslash = 0;
        if((temp) && (temp.LengthSZ<size_t>()) && (0 != (lastslash = const_cast<CharType*>(Helper::lastSlash<CharType>(temp)))))
        {
            *lastslash = 0; // zero-terminate the string
            return temp.Buffer(); // Force ctor for constant C-strings to be called
        }
        return size_t(0);
    }
    static CSimpleBuf<CHAR> getModuleFilePathA(HMODULE module) { return getModuleFilePath<CHAR>(module); }
    static CSimpleBuf<WCHAR> getModuleFilePathW(HMODULE module) { return getModuleFilePath<WCHAR>(module); }
    // Counterpart to the static getResString() member function
    template<typename CharType> CSimpleBuf<CharType> getResString(UINT uID) const
    {
        if(getHandle())
        {
            return getResString<CharType>(getHandle(), uID);
        }
        return size_t(0);
    }
    CSimpleBuf<CHAR> getResStringA(UINT uID) const { return getResString<CHAR>(uID); }
    CSimpleBuf<WCHAR> getResStringW(UINT uID) const { return getResString<WCHAR>(uID); }
    // If (hMod == NULL), the current module handle (current process) will be used.
    template<typename CharType> static CSimpleBuf<CharType> getResString(HMODULE hMod, UINT uID)
    {
        if(!hMod)
        {
            hMod = ::GetModuleHandle(NULL);
        }
        CSimpleBuf<CharType> buf(MAX_PATH);
        int len = 0;
        while(0 != (len = Helper::WrapLoadString_<CharType>(hMod, uID, buf, buf.Length<int>())))
        {
            if(len < (buf.Length<int>() - 1)) // This is border-line, but better be safe than sorry
            {
                break;
            }
            buf.ReAlloc(buf.Length<size_t>() * 2);
        }
        if(len)
        {
            return buf;
        }
        VTRACE(TEXT("String with ID %d not found inside \"%s\""), uID, getModuleBaseName<TCHAR>(hMod).Buffer());
        return size_t(0);
    }
    static CSimpleBuf<CHAR> getResStringA(HMODULE hMod, UINT uID) { return getResString<CHAR>(hMod, uID); }
    static CSimpleBuf<WCHAR> getResStringW(HMODULE hMod, UINT uID) { return getResString<WCHAR>(hMod, uID); }
    // Counterpart to the static getResStringLang() member function
    CSimpleBuf<WCHAR> getResStringLang(UINT uID, LANGID lLang)
    {
        return getResStringLang(getHandle(), uID, lLang);
    }
    // The function gets a string from resource based on the LANGID passed.
    // Limitation: Returns only Unicode strings, since resources store them as Unicode anyway.
    // This code originates from :
    // <http://support.microsoft.com/default.aspx?scid=kb%3Ben-us%3B200893> (KB200893)
    static CSimpleBuf<WCHAR> getResStringLang(HMODULE hModule, UINT uStringID, LANGID lLang)
    {
        if(!hModule)
        {
            hModule = ::GetModuleHandle(NULL);
        }
        const UINT idRsrcBlk = uStringID / 16 + 1;
        const int strIndex  = uStringID % 16;
        if(HRSRC hResource = ::FindResourceEx(hModule, RT_STRING, MAKEINTRESOURCE(idRsrcBlk), lLang))
        {
            if(WCHAR *pwchMem = (WCHAR*)(::LoadResource(hModule, hResource)))
            {
                WCHAR *pwchCur = pwchMem;
                // Blocks of 16 strings each
                for(int i = 0; i < 16; i++)
                {
                    // Check the string length
                    if(WORD cchString = *pwchCur) // First WORD is the length of the string (in wide characters)
                    {
                        // Step to the actual string contents
                        pwchCur++;
                        // Compare the index inside the block with the index we seek
                        if(i == strIndex)
                        {
                            // The string has been found in the string table.
                            CSimpleBuf<WCHAR> res(cchString+1);
                            // lstrcpyn is allergic to NULL pointers
                            if(res)
                            {
                                ::lstrcpynW(res, pwchCur, cchString+1);
                                res.Buffer()[cchString] = 0;
                            }
                            VTRACE(L"Resource string \"%s\"", res.Buffer());
                            return res;
                        }
                        // Step over this string
                        pwchCur += cchString;
                    }
                    else
                    {
                        pwchCur++;
                    }
                }
            }
        }
        return size_t(0);
    }
    template<typename CharType> static CSimpleBuf<CharType> formatMessage(HMODULE hMod, DWORD MessageID, ...)
    {
        CharType* retBuf = 0;
        va_list ptr;
        va_start(ptr, MessageID);
        if(Helper::WrapFormatMessage_<CharType>(
            hMod,
            MessageID,
            (CharType**)(&retBuf),
            &ptr
            ) && (0 != retBuf))
        {
            CSimpleBuf<CharType> buf(retBuf);
            LocalFree((HLOCAL)(retBuf));
            va_end(ptr);
            // Strip any trailing line breaks
            for(CharType* c = &buf.Buffer()[buf.LengthSZ<size_t>()-1]; ((*c == '\r') || (*c == '\n')) && (c > buf.Buffer()); c--)
            {
                *c = 0;
            }
            return buf;
        }
        va_end(ptr);
        return size_t(0);
    }
    template<typename CharType> static CSimpleBuf<CharType> formatSystemMessage(DWORD MessageID, ...)
    {
        CharType* retBuf = 0;
        va_list ptr;
        va_start(ptr, MessageID);
        if(Helper::WrapFormatSysMessage_<CharType>(
            MessageID,
            (CharType**)(&retBuf),
            &ptr
            ) && (0 != retBuf))
        {
            CSimpleBuf<CharType> buf(retBuf);
            LocalFree((HLOCAL)(retBuf));
            va_end(ptr);
            // Strip any trailing line breaks
            for(CharType* c = &buf.Buffer()[buf.LengthSZ<size_t>()-1]; ((*c == '\r') || (*c == '\n')) && (c > buf.Buffer()); c--)
            {
                *c = 0;
            }
            return buf;
        }
        va_end(ptr);
        return size_t(0);
    }
    template<typename FunctionType> FunctionType getProcAddress(LPCSTR FuncName) const
    {
        FunctionType pfnTemp = (FunctionType)((m_module) ? ::GetProcAddress(m_module, FuncName) : 0);
        CLL_VERBOSE_VTRACE("DLL is loaded: %s -> retrieving function \"%s\" (%s) -> %p", m_module ? "ok" : "fail", FuncName, typeid(FunctionType).name().c_str(), pfnTemp);
        return pfnTemp;
    }
    inline HMODULE getHandle() const
    {
        return m_module;
    }
    inline HMODULE Detach() /*lint -sem(CLoadLibrary::Detach,cleanup) */
    {
        HMODULE hTemp = m_module;
        m_module = 0;
        m_NeedToFree = false;
        return hTemp;
    }
    TCHAR const * const getRequestedDllName() const
    {
        return m_requestedFile.Buffer();
    }
    TCHAR const * const getRequestedDllPath() const
    {
        return m_requestedDllDirectory.Buffer();
    }
protected:
    CSimpleBuf<TCHAR> m_requestedFile;
    CSimpleBuf<TCHAR> m_requestedDllDirectory;
    HMODULE m_module;
    bool m_AsDataFile;
    bool m_NeedToFree;
};

#ifndef CLL_NO_ENSURE_VERSION_CLASS
class CEnsureVersion: public CLoadLibrary
{
    typedef struct _VS_VERSIONINFO
    {
        WORD    wLength;
        WORD    wValueLength;
        WORD    wType;
        WCHAR   szKey[16]; // L"VS_VERSION_INFO\0"
        WORD    Padding1;
        VS_FIXEDFILEINFO Value;
    } VS_VERSIONINFO, *PVS_VERSIONINFO;
    CEnsureVersion(const CEnsureVersion&);
    CEnsureVersion operator=(const CEnsureVersion&);
    // Those are inherited methods which we do not want to have visible.
    void Load(LPCTSTR, bool);
    void Load(LPCTSTR, LPCTSTR, bool);
    bool Free();
    HMODULE getHandle() const;
    HMODULE Detach();
public:
    // Generic loading of a DLL as a data file to retrieve version info
    CEnsureVersion(LPCTSTR LibName, WORD wMajor, WORD wMinor, WORD wPatch, WORD wBuild)
        : CLoadLibrary(LibName, true)
        , m_LastError(ERROR_RESOURCE_TYPE_NOT_FOUND)
        , m_Initialized(false)
    {
        m_VersionFile.QuadPart = 0;
        m_VersionNeed.QuadPart = 0;
        if(CLoadLibrary::getHandle())
        {
            m_Initialized = (setNeededInfo_(wMajor, wMinor, wPatch, wBuild) & retrieveInfo_());
        }
    }
    // Ensure that the DLL is loaded from within a particular directory
    CEnsureVersion(LPCTSTR LibName, LPCTSTR DllDirectory, WORD wMajor, WORD wMinor, WORD wPatch, WORD wBuild)
        : CLoadLibrary(LibName, DllDirectory, true)
        , m_LastError(ERROR_RESOURCE_TYPE_NOT_FOUND)
    {
        m_VersionFile.QuadPart = 0;
        m_VersionNeed.QuadPart = 0;
        if(CLoadLibrary::getHandle())
        {
            m_Initialized = (setNeededInfo_(wMajor, wMinor, wPatch, wBuild) & retrieveInfo_());
        }
    }
    // This is for the current process' module
    CEnsureVersion()
        : CLoadLibrary()
        , m_LastError(ERROR_RESOURCE_TYPE_NOT_FOUND)
    {
        m_VersionFile.QuadPart = 0;
        m_VersionNeed.QuadPart = 0;
        retrieveInfo_(::GetModuleHandle(NULL));
    }
    inline WORD getMajor() const
    {
        return HIWORD(m_VersionFile.HighPart);
    }
    inline WORD getMinor() const
    {
        return LOWORD(m_VersionFile.HighPart);
    }
    inline WORD getPatch() const
    {
        return HIWORD(m_VersionFile.LowPart);
    }
    inline WORD getBuild() const
    {
        return LOWORD(m_VersionFile.LowPart);
    }
    operator bool() const
    {
        return (ERROR_SUCCESS == m_LastError);
    }
    inline bool IsNewer() const
    {
        VTRACE("file ver: %08X:%08X; req. ver: %08X:%08X", m_VersionFile.HighPart, m_VersionFile.LowPart, m_VersionNeed.HighPart, m_VersionNeed.LowPart);
        return IsOK() && (m_VersionFile.QuadPart > m_VersionNeed.QuadPart);
    }
    inline bool IsNewerOrEqual() const
    {
        VTRACE("file ver: %08X:%08X; req. ver: %08X:%08X", m_VersionFile.HighPart, m_VersionFile.LowPart, m_VersionNeed.HighPart, m_VersionNeed.LowPart);
        return IsOK() && (m_VersionFile.QuadPart >= m_VersionNeed.QuadPart);
    }
    inline bool IsOlder() const
    {
        VTRACE("file ver: %08X:%08X; req. ver: %08X:%08X", m_VersionFile.HighPart, m_VersionFile.LowPart, m_VersionNeed.HighPart, m_VersionNeed.LowPart);
        return IsOK() && (m_VersionFile.QuadPart < m_VersionNeed.QuadPart);
    }
    inline bool IsEqual() const
    {
        VTRACE("file ver: %08X:%08X; req. ver: %08X:%08X", m_VersionFile.HighPart, m_VersionFile.LowPart, m_VersionNeed.HighPart, m_VersionNeed.LowPart);
        return IsOK() && (m_VersionFile.QuadPart == m_VersionNeed.QuadPart);
    }
    inline bool IsOK() const
    {
        return (IsInitialized() && IsLoaded());
    }
    inline bool IsInitialized() const
    {
        return m_Initialized;
    }
    inline bool IsLoaded() const
    {
        return CLoadLibrary::IsOK();
    }
    CSimpleBuf<TCHAR> getVersionStr() const
    {
#ifdef _MSC_VER // These are only usable with MSVC for now
        return CSimpleBuf<TCHAR>::format(TEXT("%d.%d.%d.%d"), getMajor(), getMinor(), getPatch(), getBuild());
#else // _MSC_VER
        static const TCHAR fmtstr[] = TEXT("%d.%d.%d.%d");
        CSimpleBuf<TCHAR> buf(sizeof(fmtstr) * 5);
        if(buf)
        {
            if(0 < _stprintf(buf.Buffer(), fmtstr, getMajor(), getMinor(), getPatch(), getBuild()))
            {
                return buf;
            }
        }
        return TEXT("");
#endif // _MSC_VER
    }
    CSimpleBuf<TCHAR> getExpectedVersionStr() const
    {
#ifdef _MSC_VER // These are only usable with MSVC for now
        return CSimpleBuf<TCHAR>::format(TEXT("%d.%d.%d.%d"), HIWORD(m_VersionNeed.HighPart), LOWORD(m_VersionNeed.HighPart), HIWORD(m_VersionNeed.LowPart), LOWORD(m_VersionNeed.LowPart));
#else // _MSC_VER
        static const TCHAR fmtstr[] = TEXT("%d.%d.%d.%d");
        CSimpleBuf<TCHAR> buf(sizeof(fmtstr) * 5);
        if(buf)
        {
            if(0 < _stprintf(buf.Buffer(), fmtstr, HIWORD(m_VersionNeed.HighPart), LOWORD(m_VersionNeed.HighPart), HIWORD(m_VersionNeed.LowPart), LOWORD(m_VersionNeed.LowPart)))
            {
                return buf;
            }
        }
        return TEXT("");
#endif // _MSC_VER
    }
    ~CEnsureVersion()
    {
    }
protected:
    bool retrieveInfo_(HMODULE hModule = 0)
    {
        HRSRC hRsrc = NULL;
        HGLOBAL hResData = NULL;
        PVS_VERSIONINFO lpVerInfo = NULL;
        DWORD dwSize = 0;
        if(0 != (hRsrc = ::FindResource((hModule) ? hModule : CLoadLibrary::getHandle(), MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION)))
            if((0 != (dwSize = ::SizeofResource((hModule) ? hModule : CLoadLibrary::getHandle(), hRsrc)))
                && (0 != (hResData = ::LoadResource((hModule) ? hModule : CLoadLibrary::getHandle(), hRsrc)))
                && (0 != (lpVerInfo = (PVS_VERSIONINFO)(::LockResource(hResData))))
                && (lpVerInfo->wLength <= WORD(dwSize))
                && (VS_FFI_SIGNATURE == lpVerInfo->Value.dwSignature))
            {
                m_VersionFile.HighPart = lpVerInfo->Value.dwFileVersionMS;
                m_VersionFile.LowPart = lpVerInfo->Value.dwFileVersionLS;
                m_LastError = ERROR_SUCCESS;
                return true;
            }
            else
            {
                m_LastError = ERROR_INVALID_PARAMETER;
            }
        return false;
    }
    inline bool setNeededInfo_(WORD wMajor, WORD wMinor, WORD wPatch, WORD wBuild)
    {
        m_VersionNeed.HighPart = (wMajor << 16) | wMinor;
        m_VersionNeed.LowPart = (wPatch << 16) | wBuild;
        VTRACE("req. ver: %08X:%08X == %d.%d.%d.%d", m_VersionNeed.HighPart, m_VersionNeed.LowPart, wMajor, wMinor, wPatch, wBuild);
        return true;
    }
    DWORD m_LastError;
    bool m_Initialized;
    ULARGE_INTEGER m_VersionFile, m_VersionNeed;
};
#endif // CLL_NO_ENSURE_VERSION_CLASS
// Restore macro from above
#pragma pop_macro("CLL_VERBOSE_VTRACE")

#endif // __LOADLIBRARY_H_VER__
