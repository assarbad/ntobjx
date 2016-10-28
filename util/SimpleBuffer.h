////////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2007-2008, 2010, 2011 FRISK Software International (FRISK)
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
#ifndef __SIMPLEBUFFER_H_VER__
#define __SIMPLEBUFFER_H_VER__ 2016102019
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif // Check for "#pragma once" support

#include <Windows.h>
#ifdef _MSC_VER // These are only usable with MSVC for now
# include <stdarg.h>
# ifndef _CRT_SECURE_NO_WARNINGS
#   include <strsafe.h>
# endif
#endif // _MSC_VER

#ifndef DBGPRINTF
#define DBGPRINTF(...) do {} while(false)
#endif
// Very simple wrapper for zero terminated strings of a given "character" type
template <typename T> class CSimpleBuf
{
public:
    typedef T value_type;
    // Default ctor
    CSimpleBuf(size_t count = 0)
        : m_Buf(0)
        , m_count(0)
    {
        DBGPRINTF("[%p::%s] (default ctor)\n", this, __FUNCTION__);
        if(count)
        {
            (void)ReAlloc(count);
        }
    }
    // Copy ctor operator for same class
    CSimpleBuf(const CSimpleBuf& RValue)
        : m_Buf(0)
        , m_count(0)
    {
        DBGPRINTF("[%p::%s] (copy ctor)\n", this, __FUNCTION__);
        operator=(RValue);
    }
    // Copy ctor operator for zero-terminated strings of value_type
    CSimpleBuf(const value_type* ExistingZeroTerminatedBuffer)
        : m_Buf(0)
        , m_count(0)
    {
        DBGPRINTF("[%p::%s] (copy ctor - elem*)\n", this, __FUNCTION__);
        if(ExistingZeroTerminatedBuffer)
        {
            operator=(ExistingZeroTerminatedBuffer);
        }
    }
    // Assignment operator for same class
    CSimpleBuf& operator=(const CSimpleBuf& RValue)
    {
        if(&RValue != this)
        {
            DBGPRINTF("[%p::%s] (assignment op)\n", this, __FUNCTION__);
            (void)ReAlloc(0); // free the buffer first
            if(RValue.Buffer() && ReAlloc(RValue.Length<size_t>()))
            {
                memcpy(Buffer(), RValue.Buffer(), min_(LengthBytes<size_t>(), RValue.LengthBytes<size_t>()));
            }
        }
        return *this;
    }
    // Assignment operator for zero-terminated strings of value_type
    CSimpleBuf& operator=(const value_type* RValue)
    {
        DBGPRINTF("[%p::%s] (assignment op - elem*)\n", this, __FUNCTION__);
        (void)ReAlloc(0); // free the buffer first
        if(RValue)
        {
            const size_t len = buflen_(RValue);
            if(!len)
            {
                (void)ReAlloc(1); // Make it an empty string, since the pointer was non-NULL, but length was zero!
            }
            else if(ReAlloc(len))
            {
                // Note that normally the m_buf can be larger than RValue, but not
                // the other way around, if we are at this point
                memcpy(Buffer(), RValue, min_(sizeof(value_type) * len, LengthBytes<size_t>()));
            }
        }
        return *this;
    }
    // Not-equals operator for same class (checks content!!!)
    bool operator!=(const CSimpleBuf& RValue)
    {
        DBGPRINTF("[%p::%s] (!= op)\n", this, __FUNCTION__);
        return (this == &RValue) ? false : operator!=(RValue.Buffer());
    }
    // Not-equals operator for zero-terminated strings of value_type (checks content!!!)
    bool operator!=(const value_type* RValue)
    {
        DBGPRINTF("[%p::%s] (!= op)\n", this, __FUNCTION__);
        return !operator==(RValue);
    }
    // Equals operator for same class (checks content!!!)
    bool operator==(const CSimpleBuf& RValue)
    {
        DBGPRINTF("[%p::%s] (== op)\n", this, __FUNCTION__);
        return (this == &RValue) ? true : operator==(RValue.Buffer());
    }
    // Equals operator for zero-terminated strings of value_type (checks content!!!)
    bool operator==(const value_type* RValue)
    {
        DBGPRINTF("[%p::%s] (!= op)\n", this, __FUNCTION__);
        // If the buffer address is the same or both are null pointers
        if((Buffer() == RValue) || ((Buffer() == 0) && (RValue == 0)))
        {
            return true;
        }
        // If either one is a null pointer, but the other one is not, return false
        else if((Buffer() == 0) || (RValue == 0))
        {
            return false;
        }
        return (0 == memcmp(Buffer(), RValue, min_(LengthBytes<size_t>(), sizeof(value_type) * buflen_(RValue))));
    }
    // Append operator for same class
    CSimpleBuf& operator+=(const CSimpleBuf& RValue)
    {
        DBGPRINTF("[%p::%s] %d\n", this, __FUNCTION__, (LengthSZ<size_t>() + RValue.LengthSZ<size_t>()));
        if(RValue.LengthSZ<size_t>() && ReAlloc(LengthSZ<size_t>() + RValue.LengthSZ<size_t>()))
        {
            // Append to the previous content
            memcpy(Buffer() + LengthSZ<size_t>(), RValue.Buffer(), sizeof(value_type) * RValue.LengthSZ<size_t>());
        }
        return *this;
    }
    // Append operator for zero-terminated strings of value_type
    CSimpleBuf& operator+=(const value_type* RValue)
    {
        DBGPRINTF("[%p::%s] %d\n", this, __FUNCTION__, (LengthSZ<size_t>() + buflen_(RValue)));
        const size_t len = buflen_(RValue);
        // No need to copy the buffer if we append a zero-length string ;)
        if(len && ReAlloc(LengthSZ<size_t>() + len))
        {
            // Append to the previous content
            memcpy(Buffer() + LengthSZ<size_t>(), RValue, sizeof(value_type) * len);
        }
        return *this;
    }
    // Append operator for value_type (ignores zero-termination)
    CSimpleBuf& operator+=(const value_type RValue)
    {
        DBGPRINTF("[%p::%s] %d\n", this, __FUNCTION__, (LengthSZ<size_t>() + 1));
        if(RValue && ReAlloc(LengthSZ<size_t>() + 1))
        {
            // Append to the previous content
            *(Buffer() + LengthSZ<size_t>()) = RValue;
        }
        return *this;
    }
    // Addition operator for same class
    CSimpleBuf operator+(const CSimpleBuf& RValue) const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return CSimpleBuf<T>(*this) += RValue;
    }
    // Addition operator for zero-terminated strings of value_type
    CSimpleBuf operator+(const value_type* RValue) const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return CSimpleBuf<T>(*this) += RValue;
    }
    // Addition operator for value_type
    CSimpleBuf operator+(const value_type RValue) const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return CSimpleBuf<T>(*this) += RValue;
    }
    ~CSimpleBuf()
    {
        DBGPRINTF("[%p::%s] (dtor)\n", this, __FUNCTION__);
        delete[] m_Buf;
    }
    inline void Clear()
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        // Clear the buffer
        memset(Buffer(), 0, LengthBytes<size_t>());
    }
    // Reallocates/allocates(!0) or frees(0) the memory in the internal buffer
    bool ReAlloc(size_t count)
    {
        value_type* tempBuf = 0;
        register size_t count_ = 0;
        if(count)
        {
            count_ = roundtopara_(count+1);
            DBGPRINTF("[%p::%s]: requested %d, got: %d\n", this, __FUNCTION__, count, count_);
            // We don't free it, if the user hasn't requested it explicitly by using ReAlloc(0)
            if(count_ <= m_count)
            {
                DBGPRINTF("[%p::%s]: (0 != count) && (count_ <= m_count)\n", this, __FUNCTION__);
                // Fill the "excess" part of the buffer with zeros
                memset(m_Buf + count, 0, sizeof(value_type) * (m_count - count));
                return true; // We don't allocate anything new, but the buffer will be fine
            }
            // If the requested size if above zero, attempt to allocate
            if(0 != (tempBuf = new value_type[count_]/* one as reserve */))
            {
                DBGPRINTF("[%p::%s]: new successful -> %d\n", this, __FUNCTION__, count_);
                // Clear the newly allocated buffer
                memset(tempBuf, 0, sizeof(value_type) * count_);
            }
            // Check whether allocation was successful and whether the old buffer is assigned
            if(tempBuf && m_Buf)
            {
                DBGPRINTF("[%p::%s]: copying old to new buffer\n", this, __FUNCTION__);
                // ... and copy old contents over, choosing the minimum of old and new count
                memcpy(tempBuf, m_Buf, sizeof(value_type) * min_(count, m_count));
            }
        }
        delete[] m_Buf;
        m_Buf = tempBuf;
        m_count = count_;
        DBGPRINTF("[%p::%s] new values: m_count %d, m_Buf %p\n", this, __FUNCTION__, m_count, m_Buf);
        return (0 != m_Buf);
    }
    // Returns the buffer that's being wrapped by the class
    inline value_type* Buffer() const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return m_Buf;
    };
    operator value_type*() const // Try to use the explicit "Buffer()" instead
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return m_Buf;
    }
    inline bool operator!() const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return (0 == m_Buf);
    }
    // Returns true if the buffer contains no characters.
    inline bool Empty() const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return (0 == LengthSZ<size_t>());
    }
    // Returns the length of the buffer in elements, including excess and
    // terminating zero elements
    inline size_t Length() const
    {
        return Length<size_t>();
    }
    // Returns the length of the buffer in elements, including excess and
    // terminating zero elements
    template <typename X> inline X Length() const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        if(!m_Buf)
        {
            return static_cast<X>(0);
        }
        // Attention we are casting (a spell)
        return static_cast<X>(m_count-1);
    }
    // Returns the length in elements until the terminating zero
    inline size_t LengthSZ() const
    {
        return LengthSZ<size_t>();
    }
    // Returns the length in elements until the terminating zero
    template <typename X> inline X LengthSZ() const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return static_cast<X>((m_Buf) ? buflen_(m_Buf) : 0);
    }
    // Returns the length (in Bytes) of the buffer
    inline size_t LengthBytes() const
    {
        return LengthBytes<size_t>();
    }
    template <typename X> inline X LengthBytes() const
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        if(!m_Buf)
        {
            return static_cast<X>(0);
        }
        return static_cast<X>((m_count-1) * sizeof(value_type));
    }
#ifdef _MSC_VER // These are only usable with MSVC for now
    CSimpleBuf<char> UnicodeToAnsi(UINT codePage = CP_ACP) const
    {
        return UnicodeToAnsi_(m_Buf, codePage);
    }
    CSimpleBuf<wchar_t> AnsiToUnicode(UINT codePage = CP_ACP) const
    {
        return AnsiToUnicode_(m_Buf, codePage);
    }
protected:
    // Fake support, but in fact it only works for (T == wchar_t) and (T == char)
    template <typename T> CSimpleBuf<char> UnicodeToAnsi_(const T* Value, UINT codePage) const
    {
        return size_t(0);
    }
    template <typename T> CSimpleBuf<char> UnicodeToAnsi_(const char* Value, UINT codePage) const
    {
        if(!Value)
        {
            return size_t(0);
        }
        return Value;
    }
    template <typename T> CSimpleBuf<char> UnicodeToAnsi_(const wchar_t* Value, UINT codePage) const
    {
        if(!Value)
        {
            return size_t(0);
        }
        CSimpleBuf<wchar_t> source(Value);
        if(int iNeeded = ::WideCharToMultiByte(codePage, 0, source.Buffer(), source.LengthSZ<int>(), NULL, 0, NULL, NULL))
        {
            CSimpleBuf<char> target(iNeeded);
            if(::WideCharToMultiByte(codePage, 0, source.Buffer(), source.LengthSZ<int>(), target.Buffer(), target.Length<int>(), NULL, NULL))
            {
                return target.Buffer();
            }
        }
        return size_t(0);
    }
    // Fake support, but in fact it only works for (T == wchar_t) and (T == char)
    template <typename T> CSimpleBuf<wchar_t> AnsiToUnicode_(const T* Value, UINT codePage) const
    {
        return size_t(0);
    }
    template <typename T> CSimpleBuf<wchar_t> AnsiToUnicode_(const char* Value, UINT codePage) const
    {
        if(!Value)
        {
            return size_t(0);
        }
        CSimpleBuf<char> source(Value);
        if(int iNeeded = ::MultiByteToWideChar(codePage, 0, source.Buffer(), source.LengthSZ<int>(), NULL, 0))
        {
            CSimpleBuf<wchar_t> target(iNeeded);
            if(::MultiByteToWideChar(codePage, 0, source.Buffer(), source.LengthSZ<int>(), target.Buffer(), target.Length<int>()))
            {
                return target.Buffer();
            }
        }
        return size_t(0);
    }
    template <typename T> CSimpleBuf<wchar_t> AnsiToUnicode_(const wchar_t* Value, UINT codePage) const
    {
        if(!Value)
        {
            return size_t(0);
        }
        return Value;
    }
private:
    // Little helper class that wraps calls depending on the used character type
    class PrintfHelper
    {
    public:
        template<typename CharType> static size_t vsnprintf(char* buffer, size_t len, const char* formatstr, va_list args)
        {
# ifdef _CRT_SECURE_NO_WARNINGS
            return _vsnprintf(buffer, len, formatstr, args);
# else
            HRESULT hr = StringCbVPrintfA(buffer, len * sizeof(CharType), formatstr, args);
            if(SUCCEEDED(hr) || (STRSAFE_E_INSUFFICIENT_BUFFER == hr))
            {
                return len;
            }
            return 0;
# endif
        }
        template<typename CharType> static size_t vsnprintf(wchar_t* buffer, size_t len, const wchar_t* formatstr, va_list args)
        {
# ifdef _CRT_SECURE_NO_WARNINGS
            return _vsnwprintf(buffer, len, formatstr, args);
# else
            HRESULT hr = StringCbVPrintfW(buffer, len * sizeof(CharType), formatstr, args);
            if(SUCCEEDED(hr) || (STRSAFE_E_INSUFFICIENT_BUFFER == hr))
            {
                return len;
            }
            return 0;
# endif
        }
    };
public:
    template <typename T> static CSimpleBuf<T> formatV(const T* FormatString, va_list args)
    {
        if(!FormatString)
        {
            return size_t(0);
        }
        // Allocate a minimum of twice the length of the format string
        CSimpleBuf<T> temp(2 * buflen_(FormatString));
        size_t retfromprintf = 0;
        // Loop and increase buffer size if the return value is -1
        // (meaning that the buffer was too small!)
        while(-1 == (retfromprintf = PrintfHelper::vsnprintf<T>(temp, temp.Length<size_t>(), FormatString, args)))
        {
            // Double the buffer size
            if(!temp.ReAlloc(2 * temp.Length<size_t>()))
            {
                // We just ran out of memory, as it seems ... return an empty string
                return size_t(0);
            }
        }
        // If we're here but the return value is negative, something went wrong!
        if(retfromprintf < 0)
        {
            return size_t(0); // Some error, return an empty string as well
        }
        // Return the contents of the buffer
        return temp.Buffer();
    }

    template <typename T> static CSimpleBuf<T> format(const T* FormatString, ...)
    {
        va_list args;
        va_start(args, FormatString);
        // va_end can be avoided because we do not call va_arg or something else that would modify "args"
        return formatV(FormatString, args);
    }
    // Non-static member function
    value_type* operator()(const value_type* FormatString, ...)
    {
        if(FormatString)
        {
            va_list args;
            va_start(args, FormatString);
            // Copy from the return value of the static format function
            operator=(formatV<value_type>(FormatString, args));
        }
        else
        {
            ReAlloc(0);
        }
        return Buffer();
    }
#endif // _MSC_VER
protected:
    // Quick version for char & Co.
    template <typename T> static size_t buflen_(const unsigned char* Value)
    {
        return lstrlenA(static_cast<const char*>(Value)); // lstrlenA
    }
    template <typename T> static size_t buflen_(const signed char* Value)
    {
        return lstrlenA(static_cast<const char*>(Value)); // lstrlenA
    }
    // Quick version for wchar_t & Co.
    template <typename T> static size_t buflen_(const unsigned short* Value)
    {
        return lstrlenW(static_cast<const wchar_t*>(Value)); // lstrlenW
    }
    template <typename T> static size_t buflen_(const signed short* Value)
    {
        return lstrlenW(static_cast<const wchar_t*>(Value)); // lstrlenW
    }
    // Quick version for int & Co.
    template <typename T> static size_t buflen_(const unsigned int* Value)
    {
        return elemlen_(Value);
    }
    template <typename T> static size_t buflen_(const signed int* Value)
    {
        return elemlen_(Value);
    }
    // If no better match is being found, use the slow version
    template <typename T> static size_t buflen_(const T* Value)
    {
        if(0 == Value)
        {
            return 0;
        }
        size_t retval = 0;
        if(const value_type* temp = Value)
        {
            value_type zero;
            memset(&zero, 0, sizeof(value_type));
            // Find the terminating zero "element"
            while(0 != memcmp(temp++, &zero, sizeof(value_type)))
            {
                ++retval;
            }
        }
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return retval;
    }
    inline size_t min_(size_t a, size_t b)
    {
        DBGPRINTF("[%p::%s]\n", this, __FUNCTION__);
        return ((a < b) ? a : b);
    }
    template<typename X> size_t elemlen_(const X* Value)
    {
        if(!Value) return 0;
        size_t retval = 0;
        while(*(Value++) != 0) ++retval;
        return retval;
    }
    inline size_t roundtopara_(size_t elemcount)
    {
        const size_t ParaSize = 16;
        return (((sizeof(value_type) * elemcount) + (ParaSize - 1)) & (~(ParaSize - 1))) / sizeof(value_type);
    }
    T* m_Buf;
    size_t m_count;
};
typedef CSimpleBuf<char> CSimpleCharBuf;
typedef CSimpleBuf<wchar_t> CSimpleWCharBuf;
#endif // __SIMPLEBUFFER_H_VER__
