
/*
 *@@sourcefile bs_string.cpp:
 *      BSString implementation.
 *
 *@@header "cppbase\bs_string.h"
 */

/*
 *      This file Copyright (C) 1999-2011 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "setup.h"

#include "helpers\nls.h"
#include "helpers\stringh.h"
#include "helpers\xstring.h"

// base includes
#include "cppbase\bs_base.h"
#include "cppbase\bs_string.h"

#pragma hdrstop

// #define dprintf printf

DEFINE_CLASS(BSStringBuf, BSRoot);

DEFINE_CLASS(BSStringBase, BSRoot);

// extern BSMutex G_mtxStrings = 1;

/* ******************************************************************
 *
 *  BSStringBase implementation
 *
 ********************************************************************/

// define npos; this must be in the implementation, or we'll
// get duplicate symbols from the linker
const size_type BSStringBase::npos = (size_type)(-1);

/*
 *@@ Init:
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

void BSStringBase::Init()
{
    _pBuf = NULL;
}

/*
 *@@ FreeBuf:
 *      protected helper method to free
 *      allocated resources.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: renamed from Cleanup()
 *@@changed V0.9.19 (2002-05-07) [umoeller]: fixed crash when buffers were shared, thanks knut
 */

void BSStringBase::FreeBuf()
{
    if (_pBuf)
    {
        if (_pBuf->_cShared)
        {
            // buffer is still shared: don't delete it,
            // just lower share count
            --(_pBuf->_cShared);
        }
        else
        {
            // buffer is not or no longer shared:
            // delete the buffer
            delete _pBuf;
            // _pBuf = NULL;    V0.9.19 (2002-05-07) [umoeller]
        }

        // always set _pBuf to NULL, otherwise any following
        // code will still use the buffer
        // V0.9.19 (2002-05-07) [umoeller]
        _pBuf = NULL;
    }
}

/*
 *@@ CopyFrom:
 *      copies from another string.
 *
 *      Preconditions: String must have
 *      been cleared before calling this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

void BSStringBase::CopyFrom(const BSStringBase &s)
{
    if (_pBuf != s._pBuf)
    {
        ++(s._pBuf->_cShared);
        _pBuf = s._pBuf;
    }
}

/*
 *@@ CopyFrom:
 *      copies from another string.
 *
 *      Preconditions: String must have
 *      been cleared before calling this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 *@@changed V0.9.18 (2002-03-16) [umoeller]: fixed overflow
 */

void BSStringBase::CopyFrom(const BSStringBase &s,
                            size_type ulPos,     // in: start copy, defaults to 0
                            size_type n)         // in: copy count, defaults to npos
{
    if (s._pBuf)
    {
        if (    (ulPos == 0)
             && (n == npos)
           )
        {
            // copy full string:
            // we can reuse the buf then
            CopyFrom(s);
        }
        else
        {
            size_type lenSource = s._pBuf->_str.ulLength;
            if (ulPos < lenSource)
            {
                // start is valid:
                if (n == npos)
                    n = 0;
                else if (    (n > lenSource)        // V0.9.18 (2002-03-16) [umoeller]
                          || (ulPos + n > s._pBuf->_str.ulLength)
                        )
                    // n is too large: delimit then
                    n = s._pBuf->_str.ulLength - ulPos;

                // create a new buffer
                _pBuf = new BSStringBuf(s._pBuf->_str.psz + ulPos,
                                        n,      // length
                                        0);     // allocation: default
            }
        }
    }
}

/*
 *@@ CopyFrom:
 *      copies from another string.
 *
 *      Preconditions: String must have
 *      been cleared before calling this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

void BSStringBase::CopyFrom(const char *psz)
{
    if (psz)
        _pBuf = new BSStringBuf(psz,
                                0,          // length: default
                                0);         // allocation: default
}

/*
 *@@ CopyFrom:
 *      copies from a C string, starting with
 *      p1 up to p2, which is not included.
 *
 *      Preconditions: String must have
 *      been cleared before calling this.
 */

void BSStringBase::CopyFrom(const char *p1, const char *p2)
{
    if (p2 > p1)
        _pBuf = new BSStringBuf(p1,
                                p2-p1,      // length: default
                                0);         // allocation: default
}

/*
 *@@ CopyFrom:
 *      copies from another single character.
 *
 *      Preconditions: String must have
 *      been cleared before calling this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

void BSStringBase::CopyFrom(char c) // in: character to copy
{
    if (c)
    {
        char sz[2];
        sz[0] = c;
        sz[1] = 0;
        _pBuf = new BSStringBuf(sz,
                                1,          // length
                                2);         // allocation
    }
}

/*
 *@@ BSStringBase:
 *      default constructor to create an empty string.
 *      This is protected so that only subclasses
 *      can create such a thing.
 */

BSStringBase::BSStringBase(BSClassID &Class)
    : BSRoot(Class)
{
    STRINGLOCK;
    Init();
}

/*
 *@@ BSStringBase:
 *      destructor.
 */

BSStringBase::~BSStringBase()
{
    STRINGLOCK;
    FreeBuf();
}

/*
 *@@ BSString:
 *      copy constructor.
 *
 *      This gets called explicitly by the
 *      subclasses since I made the default
 *      copy constructor private.
 *
 *      This gets called when BSStrings
 *      are thrown as exception information!
 *
 *@@added V0.9.1 (2000-01-07) [umoeller]
 */

BSStringBase::BSStringBase(const BSStringBase &s,
                           BSClassID &Class)
    : BSRoot(Class)
{
    STRINGLOCK;
    Init();
    CopyFrom(s);
}

/*
 *@@ reserve:
 *      reserves additional memory for the string.
 *
 *      This serves two purposes:
 *
 *      1)  As an internal method that is called every
 *          time before the string gets modified, to
 *          make sure we have a modifiable copy of the
 *          string with enough space if the string buffer
 *          is currently shared.
 *
 *      2)  As a user method to prepare a string for
 *          multiple subsequent operations to avoid having
 *          to reallocate the string every time.
 *
 *      If the string buf is currently shared, a new
 *      buffer is always allocated, even if stExtra is 0.
 *
 *      If the string buf is not shared, this checks if
 *      the string buffer has at least stExtra bytes. If
 *      so, nothing happens. Otherwise the buffer is
 *      reallocated to have at least stExtra bytes.
 *
 *@@added V0.9.7 (2001-01-07) [umoeller]
 */

void BSStringBase::reserve(size_type stExtra) // in: total bytes to reserve, defaults to 0
{
    STRINGLOCK;

    if (!_pBuf)
    {
        // string is empty:
        if (stExtra)
            _pBuf = new BSStringBuf(NULL,
                                    0,          // length
                                    stExtra);   // allocation
    }
    else
    {
        // we have a buffer:
        if (_pBuf->_cShared)
        {
            // we're sharing that buffer:
            // create a new one then
            PXSTRING pstr = &_pBuf->_str;
            if (stExtra < pstr->ulLength + 1)
                // caller wants less memory than the string size:
                // that's not a good idea, so raise that
                stExtra = pstr->ulLength + 1;

            FreeBuf();      // doesn't free because buf is shared
            _pBuf = new BSStringBuf(pstr->psz,
                                    pstr->ulLength,
                                    stExtra);
        }
        else
        {
            // buffer is not shared: expand that one
            // (this does nothing if stExtra is less than
            // the current size)
            xstrReserve(&_pBuf->_str, stExtra);
            _pBuf->ClearShiftTable();
        }
    }
}

/*
 *@@ _take_from:
 *      takes over the string buffers from an
 *      existing XSTRING without reallocating.
 *
 *      This also zeroes the XSTRING buffer
 *      then because the memory pointers
 *      must not be freed afterwards.
 */

void BSStringBase::_take_from(XSTRING &str)
{
    STRINGLOCK;

    FreeBuf();
    _pBuf = new BSStringBuf(str);
}

/*
 *@@ _itoa10:
 *      sets the string to the decimal representation
 *      of the given int value.
 *
 *      If cThousands is non-null, it is used as
 *      a thousands separator.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 *@@changed WarpIN V1.0.20 (2011-05-30) [pr]: fix signed/unsigned error
 */

void BSStringBase::_itoa10(int value,
                           char cThousands)     // in: thousands separator or 0 if none
{
    STRINGLOCK;

    CHAR sz[100];
    if (cThousands)
        nlsThousandsLong(sz, value, cThousands);
    else
        _itoa(value, sz, 10);

    FreeBuf();
    CopyFrom(sz);
}

/*
 *@@ _printf:
 *      this replaces the current string with
 *      a new string, which is formatted according
 *      to printf() rules.
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 */

size_type BSStringBase::_printf(const char *pszFormatString,
                                ...)
{
    STRINGLOCK;

    size_type ul = 0;

    char szBuf[5000];
    va_list arg_ptr;

    // allocate memory
    va_start(arg_ptr, pszFormatString);
    ul = vsprintf(szBuf, pszFormatString, arg_ptr);
    va_end(arg_ptr);

    FreeBuf();
    CopyFrom(szBuf);

    return (ul);
}

/*
 *@@ erase:
 *      removes at max n characters from the
 *      string, starting at ulPos.
 *
 *      Always returns *this.
 */

BSStringBase& BSStringBase::erase(size_type ulPos,      // in: start pos, defaults to 0
                                  size_type n)          // in: count, defaults to npos
{
    STRINGLOCK;

    size_type ulMyLen;

    if (    (ulMyLen = size())
         && (ulPos < ulMyLen)
         && (n)
       )
    {
        // we have a buffer:
        if (    (n == npos)
             && (ulPos == 0)
           )
            // delete all: that's easy
            FreeBuf();
        else
        {
            // assert that we have a modifiable
            // copy with enough space
            reserve(0);

            if (n == npos)
            {
                // delete rest of string:
                _pBuf->_str.psz[ulPos] = '\0';
                _pBuf->_str.ulLength = ulPos;
                // leave allocation
            }
            else
            {
                // delete something in the middle:
                // overwrite the part of the string at ulPos
                // with the following part, starting at ulPos + n
                char *pSource = _pBuf->_str.psz + ulPos + n,
                     *pTarget = _pBuf->_str.psz + ulPos;
                strcpy(pTarget, pSource);
                _pBuf->_str.ulLength -= n;
                    // allocation doesn't change
            }

            _pBuf->ClearShiftTable();
        }
    }

    return *this;
}

/*
 *@@ compare:
 *      returns 0 if the two strings are the
 *      same, -1 if "this" is smaller, 1 if
 *      "s" is smaller.
 */

int BSStringBase::compare(const BSStringBase &s)
                  const
{
    STRINGLOCK;

    // if both buffers are the same, that's easy
    if (_pBuf == s._pBuf)
        return 0;

    size_type ul1 = size(),
              ul2 = s.size();

    if ((ul1) && (ul2))
        return (strcmp(_pBuf->_str.psz, s._pBuf->_str.psz));
    else if (ul1)
        // ul1 defined, but ul2 not: string 2 is smaller
        return (1);
    else if (ul2)
        // ul1 not defined, but ul2 is: this is smaller
        return (-1);

    // none defined:
    return (0);
}

/*
 *@@ compare:
 *      compares the given C string to this.
 *
 *      Returns:
 *
 *      --  0: the two are the same.
 *
 *      --  < 0: this is smaller than psz.
 *
 *      --  > 0: psz is smaller than this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

int BSStringBase::compare(const char *psz)
                  const
{
    STRINGLOCK;

    // if both buffers are the same, that's easy
    if (    (_pBuf)
         && (_pBuf->_str.psz == psz)
       )
        return 0;

    size_type ul1 = size();

    if ((ul1) && (psz))
        return (strcmp(_pBuf->_str.psz, psz));
    else if (ul1)
        // ul1 defined, but ul2 not: string 2 is smaller
        return (1);
    else if (psz)
        // ul1 not defined, but ul2 is: this is smaller
        return (-1);

    // none defined:
    return (0);
}

/*
 *@@ _find_replace:
 *      replaces the first occurence of strFind
 *      with strReplaceWith which is found after
 *      the ulStartPos offset.
 *
 *      Returns the offset at which strFind was
 *      found or BSString::npos if it wasn't found.
 *
 *      To replace all occurences in the string,
 *      keep calling this function until BSString::npos
 *      is returned, like this:
 *
 +          string str = "part 1, part 2";
 +          ULONG ulPos = 0;
 +          while (     (ulPos == str._find_replace("part", "piece", ulPos))
 +                   != string::npos)
 +              ;
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 *
 *@@changed V1.0.1 (2003-02-02) [umoeller]: fixed crash if strReplaceWith had null buffer
 */

size_type BSStringBase::_find_replace(BSStringBase &strFind,
                                      const BSStringBase &strReplaceWith,
                                      size_type *pulPos) // in: where to start search (defaults to 0)
{
    STRINGLOCK;

    if (    (*pulPos < size())
         && (strFind.size())
       )
    {
        reserve(0);

        // if this is the first time strFind is used as
        // a "find" string, allocate a shift table...
        if (strFind._pBuf->_pShiftTable == NULL)
        {
            strFind._pBuf->_pShiftTable = (size_t*)malloc(sizeof(size_t) * 256);
            strFind._pBuf->_fRepeat = FALSE;
        }

        if (    (strFind._pBuf->_pShiftTable)
             && (xstrFindReplace(&_pBuf->_str,
                                 pulPos,
                                 &strFind._pBuf->_str,
                                 // fixed crash here V1.0.1 (2003-02-02) [umoeller]
                                 (strReplaceWith._pBuf)
                                    ? &strReplaceWith._pBuf->_str
                                    : NULL,
                                 strFind._pBuf->_pShiftTable,
                                 (PBOOL)&strFind._pBuf->_fRepeat))
           )
        {
            // found:
            return (*pulPos);
        }
    }

    // unchanged:
    return (npos);
}

/*
 *@@ _find_replace:
 *      second version of _find_replace, which takes
 *      a const PSZ as input.
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 */

size_type BSStringBase::_find_replace(const char *pszFind,
                                      const BSStringBase &strReplaceWith,
                                      size_type *pulPos)
{
    STRINGLOCK;

    if (    (*pulPos < size())
         && (pszFind)
         && (*pszFind)
       )
    {
        // create temporary "find" string
        BSString strFind(pszFind);
        // initialize shift table
        strFind._pBuf->_pShiftTable = (size_t*)malloc(sizeof(size_t) * 256);
        strFind._pBuf->_fRepeat = FALSE;
        return (_find_replace(strFind,
                              strReplaceWith,
                              pulPos));
    }

    return (npos);
}

/*
 *@@ _find_replace:
 *      third version of _find_replace, which simply
 *      replaces the first occurence of one character
 *      with another.
 *
 *      This is very inexpensive because no memory
 *      management is involved.
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 *
 *      Warning: Be careful when using this on
 *      BSUString instances. This can break UTF-8
 *      code sequences if the characters are non-ASCII.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 */

size_type BSStringBase::_find_replace(char cFind,
                                      char cReplace,
                                      size_type *pulPos)
{
    STRINGLOCK;

    PSZ p;
    if (p = strchr(_pBuf->_str.psz, cFind))
    {
        // offset:
        *pulPos = p - _pBuf->_str.psz;
        reserve(0);
        _pBuf->_str.psz[*pulPos] = cReplace;
        return (*pulPos);
    }

    return (npos);
}


