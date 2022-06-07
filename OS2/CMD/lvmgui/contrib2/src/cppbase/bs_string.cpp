
/*
 *@@sourcefile bs_string.cpp:
 *      BSString implementation.
 *
 *@@header "cppbase\bs_string.h"
 */

/*
 *      This file Copyright (C) 1999-2015 Ulrich M”ller.
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

#define INCL_DOSSEMAPHORES
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "setup.h"

#include "helpers\stringh.h"
#include "helpers\xstring.h"

// base includes
#include "cppbase\bs_base.h"
#include "cppbase\bs_string.h"
#include "cppbase\bs_errors.h"

#pragma hdrstop

// #define dprintf printf

DEFINE_CLASS(BSString, BSStringBase);

/* ******************************************************************
 *
 *  BSString implementation
 *
 ********************************************************************/

/*
 *@@ BSString:
 *      copy constructor to copy from
 *      another BSString. As opposed to the
 *      first version, you can specify a substring
 *      here.
 *
 *      Characters are copied from s starting
 *      at ulPos. Copying is stopped at the end
 *      of s or if n characters have been copied.
 */

BSString::BSString(const BSString &s,       // in: source string
                   size_type ulPos,         // in: first pos to copy
                   size_type n)             // in: maximum no. of chars to copy, defaults to npos
    : BSStringBase(tBSString)
{
    STRINGLOCK;
    // Init();      is called by parent already V0.9.20 (2002-07-03) [umoeller]
    CopyFrom(s, ulPos, n);
}

/*
 *@@ BSString:
 *      copy constructor to copy from a C string.
 */

BSString::BSString(const char *psz)         // in: string to copy
    : BSStringBase(tBSString)
{
    STRINGLOCK;
    // Init();      is called by parent already V0.9.20 (2002-07-03) [umoeller]
    CopyFrom(psz);
}

/*
 *@@ BSString:
 *      copy constructor to extract a new
 *      BSString from a codepaged C string.
 *
 *      Characters are copied starting at p1.
 *      Copying is stopped at p2, which is not
 *      included.
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 */

BSString::BSString(const char *p1, const char *p2)
    : BSStringBase(tBSString)
{
    STRINGLOCK;
    // Init();      is called by parent already V0.9.20 (2002-07-03) [umoeller]
    CopyFrom(p1, p2);
}

/*
 *@@ BSString:
 *      copy constructor to convert from a
 *      UTF-8 BSUString, using the
 *      specified BSUniCodec for conversion.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

BSString::BSString(BSUniCodec *pCodec,
                   const BSUString &ustr)
    : BSStringBase(tBSString)
{
    STRINGLOCK;

    if (!pCodec)
        throw BSExcptBase("pCodec is NULL.");

    size_type st;
    if (st = ustr.length())
    {
        pCodec->Uni2Codepage(*this,
                             ustr.GetBuffer(),
                             st);
    }
}

/*
 *@@ operator[]:
 *      returns the ul'th character of the
 *      string or 0 if ul is too large.
 */

char BSString::operator[](const size_type ul)       // in: character offset
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
 *      assigns a codepaged BSString to this.
 *
 *      This is also used internally for
 *      BSString::operator=.
 *
 *      Returns *this.
 *
 *@@added V0.9.7 (2001-01-07) [umoeller]
 */

BSString& BSString::assign(const BSString &s)
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
 *@@ assign:
 *      assigns a part of the given codepaged
 *      BSString to this.
 *
 *      Characters are copied from s starting
 *      at ulPos. Copying is stopped at the end
 *      of s or if n characters have been copied.
 *
 *      Returns *this.
 *
 *@@added V0.9.7 (2001-01-07) [umoeller]
 */

BSString& BSString::assign(const BSString &s,
                           size_type ulPos,     // in: start copy, defaults to 0
                           size_type n)         // in: copy count, defaults to npos
{
    STRINGLOCK;
    FreeBuf();
    CopyFrom(s, ulPos, n);

    return (*this);
}

/*
 *@@ assign:
 *      assigns a codepaged C string to this.
 *
 *      Returns *this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

BSString& BSString::assign(const char *psz)
{
    STRINGLOCK;
    FreeBuf();
    CopyFrom(psz);

    return (*this);
}

/*
 *@@ assign:
 *      copies from a C string.
 *
 *      Characters are copied starting at p1.
 *      Copying is stopped at p2, which is not
 *      included.
 *
 *      Returns *this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

BSString& BSString::assign(const char *p1, const char *p2)
{
    STRINGLOCK;
    FreeBuf();
    CopyFrom(p1, p2);

    return (*this);
}

/*
 *@@ assign:
 *      assigns the given single character to this.
 *
 *      Returns *this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

BSString& BSString::assign(char c) // in: character to copy
{
    STRINGLOCK;
    FreeBuf();
    CopyFrom(c);

    return (*this);
}

/*
 *@@ assignUtf8:
 *      assigns the given UTF-8 BSUString to this,
 *      using the specified BSUniCodec for conversion.
 *
 *      Returns *this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

BSString& BSString::assignUtf8(BSUniCodec *pCodec,
                               const BSUString &ustr)
{
    STRINGLOCK;
    FreeBuf();

    if (!pCodec)
        throw BSExcptBase("pCodec is NULL.");

    size_type st;
    if (st = ustr.length())
    {
        pCodec->Uni2Codepage(*this,
                             ustr.GetBuffer(),
                             st);
    }

    return (*this);
}

/*
 *@@ append:
 *      appends the given codepaged BSString to this.
 *
 *      Returns *this.
 *
 *@@changed V0.9.7 (2001-01-15) [umoeller]: ClearShiftTable was missing
 */

BSString& BSString::append(const BSString &s)       // in: string to append
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
 *@@ append:
 *      appends the given codepaged C string to this.
 *
 *      Returns *this.
 */

BSString& BSString::append(const char *psz)         // in: string to append
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
 *@@ append:
 *      appends the given single character to this.
 *
 *      Returns *this.
 *
 *@@changed V0.9.7 (2001-01-15) [umoeller]: now using xstrcatc
 */

BSString& BSString::append(char c)            // in: character to append
{
    STRINGLOCK;
    if (c)
    {
        // assert that we have a modifiable
        // copy with enough space
        reserve(1 + size() + 1);
        xstrcatc(&_pBuf->_str, c);
    }

    return (*this);
}

/*
 *@@ appendCP:
 *      assigns the given UTF-8 BSUString to this,
 *      using the specified BSUniCodec for conversion
 *      before appending.
 *
 *      Returns *this.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

BSString& BSString::appendUtf8(BSUniCodec *pCodec,
                               const BSUString &s)
{
    STRINGLOCK;
    if (!pCodec)
        throw BSExcptBase("pCodec is NULL.");

    size_type st;
    if (st = s.length())
    {
        BSString strTemp;
        pCodec->Uni2Codepage(strTemp,
                             s.GetBuffer(),
                             st);
        append(strTemp);
    }

    return (*this);
}

/*
 *@@ compare:
 *      returns 0 if the two strings are the
 *      same, -1 if "this" is smaller, 1 if
 *      "s" is smaller.
 *
 *      As opposed to the standard compare,
 *      this only compares the first "n"
 *      characters of the member string,
 *      counting from ulPos.
 *
 *      If ulPos is specified, the returned offset
 *      is from the beginning of the string, not
 *      from the offset.
 */

int BSString::compare(size_type ulPos,
                      size_type n,
                      const BSString &s)
              const
{
    STRINGLOCK;
    const BSString strTemp(*this, ulPos, n);
    return (strTemp.compare(s));
}

/*
 *@@ find:
 *      first version of find.
 *
 *      Finds the first position of a substring in the string.
 *
 *      The search is started at iPos, which defaults to 0.
 *      Returns BSString::npos if not found.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: now using strhmemfind
 */

size_type BSString::find(const BSString &strFind,   // in: string to find
                         size_type ulPos)           // in: offset to start at (defaults to 0)
                    const
{
    STRINGLOCK;
    if (    (ulPos < size())
         && (strFind.size())
       )
    {
        // if this is the first time strFind is used as
        // a "find" string, allocate a shift table...
        if (strFind._pBuf->_pShiftTable == NULL)
        {
            strFind._pBuf->_pShiftTable = (size_t*)malloc(sizeof(size_t) * 256);
            strFind._pBuf->_fRepeat = FALSE;
        }

        char *p;
        if (p = (char*)strhmemfind(_pBuf->_str.psz + ulPos, // in: haystack
                                   _pBuf->_str.ulLength - ulPos,
                                   strFind._pBuf->_str.psz,
                                   strFind._pBuf->_str.ulLength,
                                   strFind._pBuf->_pShiftTable,
                                   &strFind._pBuf->_fRepeat))
            return (p - _pBuf->_str.psz);

        /* char *p;
        if (p = strstr(_pBuf->_str.psz + ulPos,
                       strFind._pBuf->_str.psz))
            // found: calculate offset
            return (p - _pBuf->_str.psz); */
    }

    return (npos);
}

/*
 *@@ find:
 *      second version of find.
 *
 *      Finds the first position of a substring in the string.
 *
 *      The search is started at iPos, which defaults to 0.
 *      Returns BSString::npos if not found.
 */

size_type BSString::find(const char *pszFind,   // in: string to find
                         size_type ulPos)       // in: offset to start at (defaults to 0)
                    const
{
    STRINGLOCK;
    if (    (ulPos < size())
         && (pszFind)
         && (*pszFind)
       )
    {
        char *p;
        if (p = strstr(_pBuf->_str.psz + ulPos,
                       pszFind))
            // found: calculate offset
            return (p - _pBuf->_str.psz);
    }

    return (npos);
}

/*
 *@@ find:
 *      third version of find.
 *
 *      Finds the first position of a character in the string.
 *      The search is started at iPos, which defaults to 0.
 *
 *      Returns BSString::npos if not found.
 */

size_type BSString::find(char c,            // in: character to find
                         size_type ulPos)   // in: offset to start at (defaults to 0)
                    const
{
    STRINGLOCK;
    if (    (ulPos < size())
         && (c)
       )
    {
        char *p;
        if (p = strchr(_pBuf->_str.psz + ulPos, c))
            // found: calculate offset
            return (p - _pBuf->_str.psz);
    }

    return (npos);
}

/*
 *@@ rfind:
 *      finds the last position of a character in the string.
 *
 *      Returns BSString::npos if not found.
 *
 *@@changed V0.9.18 (2002-03-08) [umoeller]: rewritten using strrchr
 */

size_type BSString::rfind(char c,           // in: character to find
                          size_type ulPos)  // in: position to search from (from the right);
                                            // defaults to npos (means end of string)
                    const
{
    STRINGLOCK;
    size_type Size;

    if (    (Size = size())
         && (c)
       )
    {
        const char *p;
        if (    (ulPos == npos)
             || (ulPos > Size)
           )
            p = _pBuf->_str.psz;
        else
            p = _pBuf->_str.psz + ulPos;

        if (p = strrchr(p, c))
            // found: calculate offset
            return (p - _pBuf->_str.psz);

        /* char *p;

        if (    (ulPos == npos)
             || (ulPos > Size)
           )
            p = _str.psz + Size - 1;
        else
            p = _str.psz + ulPos;

        // search backwards
        for (;
             p >= _str.psz;
             p--)
            if (*p == c)
                return (p - _str.psz);
        */
    }

    return (npos);
}

/*
 *@@ find_first_of:
 *      finds the first character in the member
 *      string which is the same as "c".
 *
 *      The search is started at ulPos, which
 *      defaults to 0.
 *
 *      Returns BSString::npos if not found.
 *
 *@@added V1.0.21 (2015-01-29) [pr]
 */

size_type BSString::find_first_of(char c,             // in: character to find
                                  size_type ulPos)    // in: start of search (defaults to 0)
                    const
{
    STRINGLOCK;
    size_type Size = size();

    if (    (ulPos < Size)
         && (c)
       )
    {
        char *p;
        size_type ul;
        for (p = _pBuf->_str.psz + ulPos, ul = ulPos;
             ul < Size;
             ++p, ++ul)
        {
            if (*p != c)
                return (ul);
        }
    }

    // not found:
    return (npos);
}

/*
 *@@ find_first_of:
 *      finds the first character in the member
 *      string which is one of the chars in achChars.
 *
 *      The search is started at ulPos, which
 *      defaults to 0.
 *
 *      Returns BSString::npos if not found.
 *
 *@@added V0.9.7 (2001-01-07) [umoeller]
 */

size_type BSString::find_first_of(const char *achChars, // in: chars array to look for
                                  size_type ulPos)      // in: start of search (defaults to 0)
                    const
{
    STRINGLOCK;
    size_type Size = size();

    if (    (ulPos < Size)
         && (achChars)
         && (*achChars)
       )
    {
        char *p;
        size_type ul;
        for (p = _pBuf->_str.psz + ulPos, ul = ulPos;
             ul < Size;
             ++p, ++ul)
        {
            if (strchr(achChars, *p))
                // *p is in achChars:
                return (ul);
        }
    }

    // not found:
    return (npos);
}

/*
 *@@ find_first_not_of:
 *      finds the first character in the member
 *      string which is different from "c".
 *
 *      The search is started at ulPos, which
 *      defaults to 0.
 *
 *      Returns BSString::npos if not found.
 */

size_type BSString::find_first_not_of(char c,             // in: character to ignore
                                      size_type ulPos)    // in: start of search (defaults to 0)
                    const
{
    STRINGLOCK;
    size_type Size = size();

    if (    (ulPos < Size)
         && (c)
       )
    {
        char *p;
        size_type ul;
        for (p = _pBuf->_str.psz + ulPos, ul = ulPos;
             ul < Size;
             ++p, ++ul)
        {
            if (*p != c)
                return (ul);
        }
    }

    // not found:
    return (npos);
}

/*
 *@@ find_first_not_of:
 *      finds the first character in the member
 *      string which is NOT one of the chars in achChars.
 *
 *      The search is started at ulPos, which
 *      defaults to 0.
 *
 *      Returns BSString::npos if not found.
 *
 *@@added V0.9.7 (2001-01-07) [umoeller]
 */

size_type BSString::find_first_not_of(const char *achChars, // in: characters to ignore
                                      size_type ulPos)    // in: start of search (defaults to 0)
          const
{
    STRINGLOCK;
    size_type Size = size();

    if (    (ulPos < Size)
         && (achChars)
         && (*achChars)
       )
    {
        char *p;
        size_type ul;
        for (p = _pBuf->_str.psz + ulPos, ul = ulPos;
             ul < Size;
             ++p, ++ul)
        {
            if (!strchr(achChars, *p))
                // *p is NOT in achChars:
                return (ul);
        }
    }

    // not found:
    return (npos);
}

/*
 *@@ replace:
 *      replaces "nThis" characters, starting at
 *      position ulPosThis, in the member string
 *      with "strReplace".
 *
 *      Returns *this.
 *
 *@@added V0.9.7 (2001-01-15) [umoeller]
 */

BSString& BSString::replace(size_type ulPosThis,        // in: ofs of first char to replace
                            size_type nThis,            // in: char count to replace
                            const BSString &strReplace) // in: string to replace with
{
    STRINGLOCK;
    if (size())
    {
        reserve(0);

        if (nThis == npos)
            // I'm not sure if the C++ string allows setting nThis to npos...
            // but we'll just support it, it won't hurt
            nThis = _pBuf->_str.ulLength - ulPosThis;

        xstrrpl(&_pBuf->_str,
                ulPosThis,
                nThis,
                strReplace._pBuf->_str.psz,
                strReplace._pBuf->_str.ulLength);
    }

    return (*this);
}

/*
 *@@ substr:
 *      creates a new string containing parts of
 *      the member string.
 *
 *      Copying is started at offset ulPos, from
 *      which n characters are copied. If
 *      (n == BSString::npos), all remaining characters
 *      are copied.
 *
 *      This always returns a string, but it may be
 *      empty if invalid parameters are specified.
 */

BSString BSString::substr(size_type ulPos,      // in: start pos, defaults to 0
                          size_type n)          // in: no. of characters to copy,
                                                // defaults to npos (== up to rest of string)
                   const
{
    STRINGLOCK;
    return BSString(*this, ulPos, n);
}

/*
 *@@ _find_word:
 *      searches for strWord in the string, whose offset
 *      is returned if found (or BSString::npos if not).
 *
 *      As opposed to BSString::find, this finds strWord
 *      only if it is a "word". A search string is
 *      considered a word if the character _before_
 *      it is in pcszBeginChars and the char _after_
 *      it is in pcszEndChars.
 *
 *      Example:
 +          BSString str("This is an example.");
 +          str._find_word(, "is");
 *
 *      returns the offset of "is", but not the "is" in "This".
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 */

size_type BSString::_find_word(BSString &strFind,
                               size_type ulPos,     // defaults to 0
                               const char *pcszBeginChars, //  = "\x0d\x0a ()/\\-,.",
                               const char *pcszEndChars) // = "\x0d\x0a ()/\\-,.:;")
                    const
{
    STRINGLOCK;
    if (    (ulPos < size())
         && (strFind.size())
       )
    {
        // if this is the first time strFind is used as
        // a "find" string, allocate a shift table...
        if (strFind._pBuf->_pShiftTable == NULL)
        {
            strFind._pBuf->_pShiftTable = (size_t*)malloc(sizeof(size_t) * 256);
            strFind._pBuf->_fRepeat = FALSE;
        }

        char *p;
        if (p = xstrFindWord(&_pBuf->_str,
                             ulPos,
                             &strFind._pBuf->_str,
                             strFind._pBuf->_pShiftTable,
                             (PBOOL)&strFind._pBuf->_fRepeat,
                             pcszBeginChars,
                             pcszEndChars))
            // found: calculate offset
            return (p - _pBuf->_str.psz);
    }

    return (npos);
}

/*
 *@@ _find_word:
 *      second version of _find_word, which takes
 *      a const PSZ as input.
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 */

size_type BSString::_find_word(const char *pszFind,
                               size_type ulPos,     // defaults to 0
                               const char *pcszBeginChars, //  = "\x0d\x0a ()/\\-,.",
                               const char *pcszEndChars) // = "\x0d\x0a ()/\\-,.:;")
                    const
{
    STRINGLOCK;
    if (    (ulPos < size())
         && (pszFind)
         && (*pszFind)
       )
    {
        char *p;
        if (p = strhFindWord(_pBuf->_str.psz + ulPos,
                             pszFind,
                             pcszBeginChars,
                             pcszEndChars))
            // found: calculate offset
            return (p - _pBuf->_str.psz);
    }

    return (npos);
}

/*
 *@@ _extract_word:
 *      extracts a word from the member string,
 *      which is written into strTarget.
 *
 *      Returns 1 (TRUE) if the specified word still
 *      existed or 0 otherwise.
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 *
 *@@added V0.9.7 (2001-01-07) [umoeller]
 */

int BSString::_extract_word(unsigned long ulIndex,  // in: word index
                            BSString &strTarget,    // out: word
                            size_type ulPos,        // defaults to 0
                            const char *pcszBeginChars, //  = "\x0d\x0a ()/\\-,.",
                            const char *pcszEndChars) // = "\x0d\x0a ()/\\-,.:;")
    const
{
    STRINGLOCK;
    int rc = 0;

    unsigned long ul = 0;
    for (ul = 0;
         ul <= ulIndex;
         ++ul)
    {
        ULONG ulStart = find_first_not_of(pcszBeginChars, ulPos);
        if (ulStart == npos)
            break;
        else
        {
            ULONG ulEnd = find_first_of(pcszEndChars, ulStart + 1);
            if (ulEnd == npos)
            {
                if (ul == ulIndex)
                {
                    // copy till the end
                    strTarget.assign(*this,
                                     ulStart);
                    rc = 1;
                    break;
                }
                else
                    // not there yet: get outta here
                    break;
            }
            else
                if (ul == ulIndex)
                {
                    // that's the word we want:
                    strTarget.assign(*this,
                                     ulStart,
                                     (ulEnd - ulStart));
                    rc = 1;
                    break;
                }
                else
                    // we need more words:
                    ulPos = ulEnd;
                    // and search on
        }
    }

    return (rc);
}

/*
 *@@ _format:
 *      this removes all EOL's and double spaces.
 *      Also removes leading and trailing spaces.
 *
 *      Note: as far as I know, something like this
 *      is NOT defined with the C++ string class.
 */

VOID BSString::_format()
{
    STRINGLOCK;
    reserve(0);

    if (_pBuf && _pBuf->_str.psz)
    {
        PSZ p;
        // replace \n, \r with spaces (length is constant)
        for (p = _pBuf->_str.psz;
             p < _pBuf->_str.psz + _pBuf->_str.ulLength;
             ++p)
        {
            if (*p == 0x0a)
                *p = ' ';
            else if (*p == 0x0d)
                *p = ' ';
        }

        // remove double spaces
        for (p = _pBuf->_str.psz;
             p < _pBuf->_str.psz + _pBuf->_str.ulLength;
             ++p)
        {
            if ((*p == ' ') && (*(p+1) == ' '))
            {
                PSZ p2 = p;
                while (*p2)
                {
                    *p2 = *(p2+1);
                    ++p2;
                }
                --(_pBuf->_str.ulLength);
                --p;
            }
        }

        // remove leading spaces
        for (p = _pBuf->_str.psz;
             p < _pBuf->_str.psz + _pBuf->_str.ulLength;
             ++p)
        {
            if (*p == ' ')
            {
                PSZ p2 = p;
                while (*p2)
                {
                    *p2 = *(p2+1);
                    ++p2;
                }
                _pBuf->_str.ulLength--;
            }
            else
                break;
        }

        // remove trailing spaces
        for (p = _pBuf->_str.psz + _pBuf->_str.ulLength - 1;
             p > _pBuf->_str.psz;
             p--)
        {
            if (*p == ' ')
            {
                *p = 0;
                _pBuf->_str.ulLength--;
            }
            else
                break;
        }
    }
}

int operator==(const BSString &s1, const BSString &s2)
{
    return (s1.compare(s2) == 0);
}

int operator==(const char *psz1, const BSString &s2)
{
    return (s2.compare(psz1) == 0);
}

int operator==(const BSString &s1, const char *psz2)
{
    return (s1.compare(psz2) == 0);
}

int operator!=(const BSString &s1, const BSString &s2)
{
    return (s1.compare(s2) != 0);
}

int operator!=(const char *psz1, const BSString &s2)
{
    return (s2.compare(psz1) != 0);
}

int operator!=(const BSString &s1, const char *psz2)
{
    return (s1.compare(psz2) != 0);
}

int operator<(const BSString &s1, const BSString &s2)
{
    return (s1.compare(s2) < 0);
}

BSString operator+(const BSString &s1, const BSString &s2)
{
    BSString str(s1);
    str.append(s2);
    return (str);
}

BSString operator+(const char *psz1, const BSString &s2)
{
    BSString str(psz1);
    str.append(s2);
    return (str);
}

BSString operator+(const BSString &s1, const char *psz2)
{
    BSString str(s1);
    str.append(psz2);
    return (str);
}


// test case

/* int main()
{
    BSString str1("This is a test string."),
             str2(str1, 0);

    printf("Original was: %s\n", str1.c_str());
    printf("Second is:    %s\n", str2.c_str());

    str2 = str1 + " Now we appended something.";
    printf("Third is:     %s\n", str2.c_str());

    BSString strFind = "e";
    BSString strRepl = "DJASDKL™ASDPO(u";
    ULONG ulPos = 0;
    while (str2._find_replace(strFind, strRepl, &ulPos)
                != string::npos)
            ;

    printf("After repl:   %s\n", str2.c_str());

    return (0);
}
*/
