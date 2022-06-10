
/*
 *@@sourcefile bs_string_uni.cpp:
 *      BSUString implementation.
 *
 *@@header "cppbase\bs_string.h"
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

/*
 *      This file Copyright (C) 2002 Ulrich M”ller.
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

#include "helpers\stringh.h"
#include "helpers\xstring.h"

#include "encodings\base.h"

// base includes
#include "cppbase\bs_base.h"
#include "cppbase\bs_string.h"
#include "cppbase\bs_errors.h"

#pragma hdrstop

// #define dprintf printf

DEFINE_CLASS(BSUString, BSStringBase);

/* ******************************************************************
 *
 *  Initialize the encoding engine
 *
 ********************************************************************/

// We initialize the case engine by creating a global variable
// of this class, which just calls encInitCase.

class BSDummyInitEncoding
{
    public:
        BSDummyInitEncoding()
        {
            encInitCase();
        }
};

static BSDummyInitEncoding G_Init;

/* ******************************************************************
 *
 *  BSUString
 *
 ********************************************************************/

/*
 *@@ BSUString:
 *      copy constructor to convert from a
 *      codepaged BSString, using the
 *      specified BSUniCodec for conversion.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

BSUString::BSUString(BSUniCodec *pCodec,
                     const BSString &str)
    : BSStringBase(tBSUString)
{
    STRINGLOCK;

    if (!pCodec)
        throw BSExcptBase("pCodec is NULL.");

    size_type st;
    if (st = str.length())
    {
        pCodec->Codepage2Uni(*this,
                             str.c_str(),
                             st);
    }
}

/*
 *@@ operator[]:
 *      returns the ul'th character of the
 *      string or 0 if ul is too large.
 *
 *      Warning: this counts bytes, not UTF-8
 *      characters.
 */

char BSUString::operator[](const size_type ul)       // in: character offset
                const
{
    STRINGLOCK;
    if (    (_pBuf)
         && (_pBuf->_str.psz)
         && (ul < _pBuf->_str.ulLength)
       )
        return (_pBuf->_str.psz[ul]);

    return ('\0');
}

/*
 *@@ assign:
 *      assigns another BSUString to this. This
 *      is the only direct assign that is supported
 *      for BSUString (and gets called thru
 *      BSUString::operator= as well); otherwise
 *      you will have to use one of the
 *      assignUtf8 methods.
 *
 *      Returns *this.
 */

BSUString& BSUString::assign(const BSUString &s)
{
    STRINGLOCK;
    // avoid self assignment
    if (_pBuf != s._pBuf)
    {
        FreeBuf();
        CopyFrom(s);
    }

    return (*this);
}

/*
 *@@ assignUtf8:
 *      assigns a UTF-8 C string to this.
 *
 *      This is intentionally named differently
 *      from assign to make sure noone accidentally
 *      assigns codepaged C strings. Note that
 *      the "=" operator won't work for C strings
 *      either.
 *
 *      Returns *this.
 */

BSUString& BSUString::assignUtf8(const char *psz)
{
    STRINGLOCK;
    FreeBuf();
    CopyFrom(psz);

    return (*this);
}

/*
 *@@ assignUtf8:
 *      copies from a UTF-8 C string, starting with
 *      p1 up to p2, which is not included.
 *
 *      This is intentionally named differently
 *      from assign to make sure noone accidentally
 *      assigns codepaged C strings.
 *
 *      Returns *this.
 */

BSUString& BSUString::assignUtf8(const char *p1, const char *p2)
{
    STRINGLOCK;
    FreeBuf();
    CopyFrom(p1, p2);

    return (*this);
}

/*
 *@@ assignCP:
 *      assigns a codepaged BSString to this, using
 *      the specified BSUniCodec for conversion.
 *
 *      Returns *this.
 */

BSUString& BSUString::assignCP(BSUniCodec *pCodec,
                               const BSString &str)
{
    STRINGLOCK;
    FreeBuf();

    if (!pCodec)
        throw BSExcptBase("pCodec is NULL.");

    size_type st;
    if (st = str.length())
    {
        pCodec->Codepage2Uni(*this,
                             str.c_str(),
                             st);
    }

    return (*this);
}

/*
 *@@ assignCP:
 *      assigns a codepaged C string to this, using
 *      the specified BSUniCodec for conversion.
 *
 *      Returns *this.
 */

BSUString& BSUString::assignCP(BSUniCodec *pCodec,
                               const char *psz)
{
    STRINGLOCK;
    FreeBuf();

    size_type st;
    if (st = strlen(psz))
    {
        pCodec->Codepage2Uni(*this,
                             psz,
                             st);
    }

    return (*this);
}

/*
 *@@ append:
 *      appends the given BSUString to this.
 *
 *      Also used internally by BSUString::operator+=.
 *
 *      Returns *this.
 */

BSUString& BSUString::append(const BSUString &s)
{
    STRINGLOCK;
    size_type ulSourceLen;
    if (ulSourceLen = s.size())
    {
        // assert that we have a modifiable
        // copy with enough space
        reserve(ulSourceLen + size() + 1);
        xstrcats(&_pBuf->_str, &s._pBuf->_str);
    }

    return (*this);
}

/*
 *@@ appendUtf8:
 *      appends the given UTF-8 C string to this.
 *
 *      Returns *this.
 */

BSUString& BSUString::appendUtf8(const char *psz)
{
    STRINGLOCK;
    size_type ulSourceLen;
    if (    (psz)
         && (ulSourceLen = strlen(psz))
       )
    {
        // assert that we have a modifiable
        // copy with enough space
        reserve(ulSourceLen + size() + 1);
        xstrcat(&_pBuf->_str, psz, ulSourceLen);
    }

    return (*this);
}

/*
 *@@ appendCP:
 *      appends the given codepaged BSString to this,
 *      using the specified BSUniCodec for conversion
 *      before appending.
 *
 *      Returns *this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

BSUString& BSUString::appendCP(BSUniCodec *pCodec,
                               const BSString &str)
{
    STRINGLOCK;
    if (str.length())
    {
        BSUString ustr;
        ustr.assignCP(pCodec, str);
        append(ustr);
    }

    return (*this);
}

/*
 *@@ compareI:
 *      case-insensitive compare.
 *
 *      Compares this against the given BSUString,
 *      without respect to case, with full
 *      Unicode support by calling encicmp().
 *
 *      Returns 0 if the two strings are the
 *      same, -1 if "this" is smaller, 1 if
 *      "s" is smaller.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

int BSUString::compareI(const BSUString &s) const
{
    return encicmp(_pBuf->_str.psz,
                   s._pBuf->_str.psz);
}

/*
 *@@ compareUtf8I:
 *      case-insensitive compare.
 *
 *      Compares this against the given UTF-8
 *      C string, without respect to case, with
 *      full Unicode support by calling encicmp().
 *
 *      Returns 0 if the two strings are the
 *      same, -1 if "this" is smaller, 1 if
 *      "pcsz" is smaller.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

int BSUString::compareUtf8I(const char *psz) const
{
    return encicmp(_pBuf->_str.psz,
                   psz);
}

/*
 *@@ operator==:
 *      compares two UStrings.
 */

int operator==(const BSUString &s1, const BSUString &s2)
{
    return (s1.compare(s2) == 0);
}

/*
 *@@ operator1=:
 *      compares two UStrings.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

int operator!=(const BSUString &s1, const BSUString &s2)
{
    return (s1.compare(s2) != 0);
}

/*
 *@@ operator+:
 *
 *@@added V0.9.19 (2002-04-14) [umoeller]
 */

BSUString operator+(const BSUString &s1, const BSUString &s2)
{
    BSUString str(s1);
    str.append(s2);
    return (str);
}


