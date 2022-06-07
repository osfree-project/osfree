
/*
 *@@sourcefile stringh.c:
 *      contains string/text helper functions. These are good for
 *      parsing/splitting strings and other stuff used throughout
 *      XWorkplace.
 *
 *      Note that these functions are really a bunch of very mixed
 *      up string helpers, which you may or may not find helpful.
 *      If you're looking for string functions with memory
 *      management, look at xstring.c instead.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  strh*       string helper functions.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\stringh.h"
 */

/*
 *      Copyright (C) 1997-2016 Ulrich Mîller.
 *      Parts Copyright (C) 1991-1999 iMatix Corporation.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_WINSHELLDATA
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "setup.h"                      // code generation and debugging options

#define DONT_REPLACE_STRINGH_MALLOC
#include "helpers\stringh.h"
#include "helpers\xstring.h"            // extended string helpers

#pragma hdrstop

/*
 *@@category: Helpers\C helpers\String management
 *      See stringh.c and xstring.c.
 */

/*
 *@@category: Helpers\C helpers\String management\C string helpers
 *      See stringh.c.
 */

#ifdef __DEBUG_MALLOC_ENABLED__

/*
 *@@ strhStoreDebug:
 *      memory debug version of strhStore.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

APIRET (strhStoreDebug)(PSZ *ppszTarget,
                        PCSZ pcszSource,
                        PULONG pulLength,        // out: length of new string (ptr can be NULL)
                        PCSZ pcszSourceFile,
                        unsigned long ulLine,
                        PCSZ pcszFunction)
{
    ULONG ulLength = 0;



    if (ppszTarget)
    {
        if (*ppszTarget)
            free(*ppszTarget);

        if (    (pcszSource)
             && (ulLength = strlen(pcszSource))
           )
        {
            if (*ppszTarget = (PSZ)memdMalloc(ulLength + 1,
                                              pcszSourceFile,
                                              ulLine,
                                              pcszFunction))
                memcpy(*ppszTarget, pcszSource, ulLength + 1);
            else
                return ERROR_NOT_ENOUGH_MEMORY;
        }
        else
            *ppszTarget = NULL;
    }

    if (pulLength)
        *pulLength = ulLength;

    return NO_ERROR;
}

#endif

/*
 *@@ strhStore:
 *      stores a copy of the given string in the specified
 *      buffer. Uses strdup internally.
 *
 *      If *ppszTarget != NULL, the previous string is freed
 *      and set to NULL.
 *      If pcszSource != NULL, a copy of it is stored in the
 *      buffer.
 *
 *@@added V0.9.16 (2001-12-06) [umoeller]
 */

APIRET strhStore(PSZ *ppszTarget,
                 PCSZ pcszSource,
                 PULONG pulLength)        // out: length of new string (ptr can be NULL)
{
    ULONG ulLength = 0;

    if (ppszTarget)
    {
        if (*ppszTarget)
            free(*ppszTarget);

        if (    (pcszSource)
             && (ulLength = strlen(pcszSource))
           )
        {
            if (*ppszTarget = (PSZ)malloc(ulLength + 1))
                memcpy(*ppszTarget, pcszSource, ulLength + 1);
            else
                return ERROR_NOT_ENOUGH_MEMORY;
        }
        else
            *ppszTarget = NULL;
    }
    else
        return ERROR_INVALID_PARAMETER;

    if (pulLength)
        *pulLength = ulLength;

    return NO_ERROR;
}

/*
 *@@ strhcpy:
 *      like strdup, but this one doesn't crash if string2 is NULL,
 *      but sets the first byte in string1 to \0 instead.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

PSZ strhcpy(PSZ string1, PCSZ string2)
{
    if (string2)
        return strcpy(string1, string2);

    *string1 = '\0';
    return string1;
}

/*
 *@@ strhCopyBuf:
 *      copies pcszSource to pszTarget, taking
 *      its length into account.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_INVALID_PARAMETER: pcszSource is
 *          null or points to a null byte.
 *
 *      --  ERROR_FILENAME_EXCED_RANGE: pcszSource
 *          is too long to fit into pszTarget.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

APIRET strhCopyBuf(PSZ pszTarget,
                   PCSZ pcszSource,
                   ULONG cbTarget)
{
    ULONG cb;
    if (!pcszSource || !*pcszSource)
        return ERROR_INVALID_PARAMETER;
    cb = strlen(pcszSource) + 1;
    if (cb > cbTarget)
        return ERROR_FILENAME_EXCED_RANGE;

    memcpy(pszTarget,
           pcszSource,
           cb);
    return NO_ERROR;
}

#ifdef __DEBUG_MALLOC_ENABLED__

/*
 *@@ strhdupDebug:
 *      memory debug version of strhdup.
 *
 *@@added V0.9.0 [umoeller]
 */

PSZ strhdupDebug(PCSZ pcszSource,
                 unsigned long *pulLength,
                 PCSZ pcszSourceFile,
                 unsigned long ulLine,
                 PCSZ pcszFunction)
{
    PSZ     pszReturn = NULL;
    ULONG   ulLength = 0;

    if (    (pcszSource)
         && (ulLength = strlen(pcszSource))
       )
    {
        if (pszReturn = (PSZ)memdMalloc(ulLength + 1,
                                        pcszSourceFile,     // fixed V0.9.16 (2001-12-08) [umoeller]
                                        ulLine,
                                        pcszFunction))
            memcpy(pszReturn, pcszSource, ulLength + 1);
    }

    if (pulLength)
        *pulLength = ulLength;

    return pszReturn;
}

#endif // __DEBUG_MALLOC_ENABLED__

/*
 *@@ strhdup:
 *      like strdup, but this one doesn't crash if pszSource
 *      is NULL. Instead, this returns NULL if pcszSource is
 *      NULL or points to a null byte. In addition, this
 *      can report the length of the string (V0.9.16).
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.16 (2001-10-25) [umoeller]: added pulLength
 */

PSZ strhdup(PCSZ pcszSource,
            unsigned long *pulLength)       // out: length of string excl. null terminator (ptr can be NULL)
{
    PSZ     pszReturn = NULL;
    ULONG   ulLength = 0;

    if (    (pcszSource)
         && (ulLength = strlen(pcszSource))
       )
    {
        if (pszReturn = (PSZ)malloc(ulLength + 1))
            memcpy(pszReturn, pcszSource, ulLength + 1);
    }

    if (pulLength)
        *pulLength = ulLength;

    return pszReturn;
}

/*
 *@@ strhcmp:
 *      better strcmp. This doesn't crash if any of the
 *      string pointers are NULL, but returns a proper
 *      value then.
 *
 *      Besides, this is guaranteed to only return -1, 0,
 *      or +1, while strcmp can return any positive or
 *      negative value. This is useful for tree comparison
 *      funcs.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

int strhcmp(PCSZ p1, PCSZ p2)
{
    if (p1 && p2)
    {
        int i = strcmp(p1, p2);
        if (i < 0) return -1;
        if (i > 0) return +1;
    }
    else if (p1)
        // but p2 is NULL: p1 greater than p2 then
        return +1;
    else if (p2)
        // but p1 is NULL: p1 less than p2 then
        return -1;

    // return 0 if strcmp returned 0 above or both strings are NULL
    return 0;
}

/*
 *@@ strhicmp:
 *      like strhcmp, but compares without respect
 *      to case.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

int strhicmp(PCSZ p1, PCSZ p2)
{
    if (p1 && p2)
    {
        int i = stricmp(p1, p2);
        if (i < 0) return -1;
        if (i > 0) return +1;
    }
    else if (p1)
        // but p2 is NULL: p1 greater than p2 then
        return +1;
    else if (p2)
        // but p1 is NULL: p1 less than p2 then
        return -1;

    // return 0 if strcmp returned 0 above or both strings are NULL
    return 0;
}

/*
 *@@ strhistr:
 *      like strstr, but case-insensitive.
 *
 *@@changed V0.9.0 [umoeller]: crashed if null pointers were passed, thanks RÅdiger Ihle
 */

PSZ strhistr(PCSZ string1, PCSZ string2)
{
    PSZ prc = NULL;

    if ((string1) && (string2))
    {
        PSZ pszSrchIn = strdup(string1);
        PSZ pszSrchFor = strdup(string2);

        if ((pszSrchIn) && (pszSrchFor))
        {
            strupr(pszSrchIn);
            strupr(pszSrchFor);

            if (prc = strstr(pszSrchIn, pszSrchFor))
            {
                // prc now has the first occurence of the string,
                // but in pszSrchIn; we need to map this
                // return value to the original string
                prc = (prc-pszSrchIn) // offset in pszSrchIn
                      + (PSZ)string1;
            }
        }
        if (pszSrchFor)
            free(pszSrchFor);
        if (pszSrchIn)
            free(pszSrchIn);
    }

    return prc;
}

/*
 *@@ strhncpy0:
 *      like strncpy, but always appends a 0 character.
 *
 *@@changed V0.9.16 (2002-01-09) [umoeller]: fixed crash on null pszSource
 */

ULONG strhncpy0(PSZ pszTarget,
                PCSZ pszSource,
                ULONG cbSource)
{
    ULONG ul = 0;
    PSZ     pTarget = pszTarget,
            pSource;

    if (pSource = (PSZ)pszSource)       // V0.9.16 (2002-01-09) [umoeller]
    {
        for (ul = 0; ul < cbSource; ul++)
            if (*pSource)
                *pTarget++ = *pSource++;
            else
                break;
    }

    *pTarget = 0;

    return ul;
}

/*
 *@@ strlcpy:
 *      copies src to string dst of size siz.  At most siz-1 characters
 *      will be copied.  Always NUL terminates, unless siz == 0.
 *
 *      Returns strlen(src); if retval >= siz, truncation occurred.
 *
 *      Taken from the OpenBSD sources at
 *
 +          ftp://ftp.openbsd.org/pub/OpenBSD/src/lib/libc/string/strlcpy.c
 *
 *      Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *      All rights reserved.
 *
 *      OpenBSD licence applies (see top of that file).
 *
 *@@added V1.0.1 (2003-01-29) [umoeller]
 */

size_t strlcpy(char *dst,
               const char *src,
               size_t siz)
{
    register char       *d = dst;
    register const char *s = src;
    register size_t     n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0)
    {
        do
        {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (siz != 0)
            *d = '\0';      /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return (s - src - 1);    /* count does not include NUL */
}

/*
 *@@ strlcat:
 *      appends src to string dst of size siz. Unlike strncat,
 *      siz is the full size of dst, not space left. At most
 *      siz-1 characters will be copied.  Always NUL terminates,
 *      unless siz <= strlen(dst).
 *
 *      Returns strlen(src) + MIN(siz, strlen(initial dst)),
 *      in other words, strlen(dst) after the concatenation.
 *      If retval >= siz, truncation occurred.
 *
 *      Taken from the OpenBSD sources at
 *
 +          ftp://ftp.openbsd.org/pub/OpenBSD/src/lib/libc/string/strlcat.c
 *
 *      Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *      All rights reserved.
 *
 *      OpenBSD licence applies (see top of that file).
 *
 *@@added V1.0.1 (2003-01-29) [umoeller]
 */

size_t strlcat(char *dst,
               const char *src,
               size_t siz)
{
    register char       *d = dst;
    register const char *s = src;
    register size_t     n = siz;
    size_t              dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return(dlen + strlen(s));
    while (*s != '\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return (dlen + (s - src));   /* count does not include NUL */
}

/*
 *@@ strhlen:
 *      like strlen, but doesn't crash on
 *      null strings, but returns 0 also.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

ULONG strhlen(PCSZ pcsz)
{
    if (pcsz)
        return strlen(pcsz);

    return 0;
}

/*
 *@@ strhSize:
 *      returns the size of the given string, which
 *      is the memory required to allocate a copy,
 *      including the null terminator.
 *
 *      Returns 0 only if pcsz is NULL. If pcsz
 *      points to a null character, this returns 1.
 *
 *@@added V0.9.18 (2002-02-13) [umoeller]
 *@@changed V0.9.18 (2002-03-27) [umoeller]: now returning 1 for ptr to null byte
 */

ULONG strhSize(PCSZ pcsz)
{
    if (pcsz) //  && *pcsz) // V0.9.18 (2002-03-27) [umoeller]
        return strlen(pcsz) + 1;

    return 0;
}

/*
 * strhCount:
 *      this counts the occurences of c in pszSearch.
 */

ULONG strhCount(PCSZ pszSearch,
                CHAR c)
{
    PCSZ    p = pszSearch;
    ULONG   ulCount = 0;
    while (TRUE)
    {
        if (!(p = strchr(p, c)))
            return ulCount;

        ulCount++;
        p++;
    }
}

/*
 *@@ strhIsDecimal:
 *      returns TRUE if psz consists of decimal digits only.
 */

BOOL strhIsDecimal(PSZ psz)
{
    PSZ p = psz;
    while (*p != 0)
    {
        if (isdigit(*p) == 0)
            return FALSE;
        p++;
    }

    return TRUE;
}

#ifdef __DEBUG_MALLOC_ENABLED__

/*
 *@@ strhSubstrDebug:
 *      memory debug version of strhSubstr.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

PSZ strhSubstrDebug(PCSZ pBegin,      // in: first char
                    PCSZ pEnd,        // in: last char (not included)
                    PCSZ pcszSourceFile,
                    unsigned long ulLine,
                    PCSZ pcszFunction)
{
    PSZ pszSubstr = NULL;

    if (pEnd > pBegin)      // V0.9.9 (2001-04-04) [umoeller]
    {
        ULONG cbSubstr = (pEnd - pBegin);
        if (pszSubstr = (PSZ)memdMalloc(cbSubstr + 1,
                                        pcszSourceFile,
                                        ulLine,
                                        pcszFunction))
        {
            // strhncpy0(pszSubstr, pBegin, cbSubstr);
            memcpy(pszSubstr, pBegin, cbSubstr);        // V0.9.9 (2001-04-04) [umoeller]
            *(pszSubstr + cbSubstr) = '\0';
        }
    }

    return pszSubstr;
}

#endif // __DEBUG_MALLOC_ENABLED__

/*
 *@@ strhSubstr:
 *      this creates a new PSZ containing the string
 *      from pBegin to pEnd, excluding the pEnd character.
 *      The new string is null-terminated. The caller
 *      must free() the new string after use.
 *
 *      Example:
 +              "1234567890"
 +                ^      ^
 +                p1     p2
 +          strhSubstr(p1, p2)
 *      would return a new string containing "2345678".
 *
 *@@changed V0.9.9 (2001-04-04) [umoeller]: fixed crashes with invalid pointers
 *@@changed V0.9.9 (2001-04-04) [umoeller]: now using memcpy for speed
 */

PSZ strhSubstr(PCSZ pBegin,      // in: first char
               PCSZ pEnd)        // in: last char (not included)
{
    PSZ pszSubstr = NULL;

    if (pEnd > pBegin)      // V0.9.9 (2001-04-04) [umoeller]
    {
        ULONG cbSubstr = (pEnd - pBegin);
        if (pszSubstr = (PSZ)malloc(cbSubstr + 1))
        {
            memcpy(pszSubstr, pBegin, cbSubstr);        // V0.9.9 (2001-04-04) [umoeller]
            *(pszSubstr + cbSubstr) = '\0';
        }
    }

    return pszSubstr;
}

/*
 *@@ strhExtract:
 *      searches pszBuf for the cOpen character and returns
 *      the data in between cOpen and cClose, excluding
 *      those two characters, in a newly allocated buffer
 *      which you must free() afterwards.
 *
 *      Spaces and newlines/linefeeds are skipped.
 *
 *      If the search was successful, the new buffer
 *      is returned and, if (ppEnd != NULL), *ppEnd points
 *      to the first character after the cClose character
 *      found in the buffer.
 *
 *      If the search was not successful, NULL is
 *      returned, and *ppEnd is unchanged.
 *
 *      If another cOpen character is found before
 *      cClose, matching cClose characters will be skipped.
 *      You can therefore nest the cOpen and cClose
 *      characters.
 *
 *      This function ignores cOpen and cClose characters
 *      in C-style comments and strings surrounded by
 *      double quotes.
 *
 *      Example:
 *
 +          PSZ pszBuf = "KEYWORD { --blah-- } next",
 +              pEnd;
 +          strhExtract(pszBuf,
 +                      '{', '}',
 +                      &pEnd)
 *
 *      would return a new buffer containing " --blah-- ",
 *      and ppEnd would afterwards point to the space
 *      before "next" in the static buffer.
 *
 *@@added V0.9.0 [umoeller]
 */

PSZ strhExtract(PCSZ pszBuf,    // in: search buffer
                CHAR cOpen,     // in: opening char
                CHAR cClose,    // in: closing char
                PCSZ *ppEnd)    // out: if != NULL, receives first character after closing char
{
    PSZ pszReturn = NULL;
    PCSZ pOpen;
    if (    (pszBuf)
         && (pOpen = strchr(pszBuf, cOpen))
       )
    {
        // opening char found:
        // now go thru the whole rest of the buffer
        PCSZ     p = pOpen + 1;
        LONG    lLevel = 1;        // if this goes 0, we're done
        while (*p)
        {
            if (*p == cOpen)
                lLevel++;
            else if (*p == cClose)
            {
                lLevel--;
                if (lLevel <= 0)
                {
                    // matching closing bracket found:
                    // extract string
                    pszReturn = strhSubstr(pOpen + 1,   // after cOpen
                                           p);          // excluding cClose
                    if (ppEnd)
                        *ppEnd = p + 1;
                    break;      // while (*p)
                }
            }
            else if (*p == '\"')
            {
                // beginning of string:
                PCSZ p2 = p+1;
                // find end of string
                while ((*p2) && (*p2 != '\"'))
                    p2++;

                if (*p2 == '\"')
                    // closing quote found:
                    // search on after that
                    p = p2;     // raised below
                else
                    break;      // while (*p)
            }

            p++;
        }
    }

    return pszReturn;
}

/*
 *@@ strhQuote:
 *      similar to strhExtract, except that
 *      opening and closing chars are the same,
 *      and therefore no nesting is possible.
 *      Useful for extracting stuff between
 *      quotes.
 *
 *@@added V0.9.0 [umoeller]
 */

PSZ strhQuote(PSZ pszBuf,
              CHAR cQuote,
              PSZ *ppEnd)
{
    PSZ pszReturn = NULL,
        p1 = NULL;
    if ((p1 = strchr(pszBuf, cQuote)))
    {
        PSZ p2;
        if (p2 = strchr(p1+1, cQuote))
        {
            pszReturn = strhSubstr(p1+1, p2);
            if (ppEnd)
                // store closing char
                *ppEnd = p2 + 1;
        }
    }

    return pszReturn;
}

/*
 *@@ strhStrip:
 *      removes all double spaces.
 *      This copies within the "psz" buffer.
 *      If any double spaces are found, the
 *      string will be shorter than before,
 *      but the buffer is _not_ reallocated,
 *      so there will be unused bytes at the
 *      end.
 *
 *      Returns the number of spaces removed.
 *
 *@@added V0.9.0 [umoeller]
 */

ULONG strhStrip(PSZ psz)         // in/out: string
{
    PSZ     p;
    ULONG   cb = strlen(psz),
            ulrc = 0;

    for (p = psz; p < psz+cb; p++)
    {
        if ((*p == ' ') && (*(p+1) == ' '))
        {
            PSZ p2 = p;
            while (*p2)
            {
                *p2 = *(p2+1);
                p2++;
            }
            cb--;
            p--;
            ulrc++;
        }
    }
    return ulrc;
}

/*
 *@@ strhins:
 *      this inserts one string into another.
 *
 *      pszInsert is inserted into pszBuffer at offset
 *      ulInsertOfs (which counts from 0).
 *
 *      A newly allocated string is returned. pszBuffer is
 *      not changed. The new string should be free()'d after
 *      use.
 *
 *      Upon errors, NULL is returned.
 *
 *@@changed V0.9.0 [umoeller]: completely rewritten.
 */

PSZ strhins(PCSZ pcszBuffer,
            ULONG ulInsertOfs,
            PCSZ pcszInsert)
{
    PSZ     pszNew = NULL;

    if ((pcszBuffer) && (pcszInsert))
    {
        do {
            ULONG   cbBuffer = strlen(pcszBuffer);
            ULONG   cbInsert = strlen(pcszInsert);

            // check string length
            if (ulInsertOfs > cbBuffer + 1)
                break;  // do

            // OK, let's go.
            pszNew = (PSZ)malloc(cbBuffer + cbInsert + 1);  // additional null terminator

            // copy stuff before pInsertPos
            memcpy(pszNew,
                   pcszBuffer,
                   ulInsertOfs);
            // copy string to be inserted
            memcpy(pszNew + ulInsertOfs,
                   pcszInsert,
                   cbInsert);
            // copy stuff after pInsertPos
            strcpy(pszNew + ulInsertOfs + cbInsert,
                   pcszBuffer + ulInsertOfs);
        } while (FALSE);
    }

    return pszNew;
}

/*
 *@@ strhFindReplace:
 *      wrapper around xstrFindReplace to work with C strings.
 *      Note that *ppszBuf can get reallocated and must
 *      be free()'able.
 *
 *      Repetitive use of this wrapper is not recommended
 *      because it is considerably slower than xstrFindReplace.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 *@@changed V0.9.7 (2001-01-15) [umoeller]: renamed from strhrpl
 */

ULONG strhFindReplace(PSZ *ppszBuf,                // in/out: string
                      PULONG pulOfs,               // in: where to begin search (0 = start);
                                                   // out: ofs of first char after replacement string
                      PCSZ pcszSearch,      // in: search string; cannot be NULL
                      PCSZ pcszReplace)     // in: replacement string; cannot be NULL
{
    ULONG   ulrc = 0;
    XSTRING xstrBuf,
            xstrFind,
            xstrReplace;
    size_t  ShiftTable[256];
    BOOL    fRepeat = FALSE;
    xstrInitSet(&xstrBuf, *ppszBuf);
                // reallocated and returned, so we're safe
    xstrInitSet(&xstrFind, (PSZ)pcszSearch);
    xstrInitSet(&xstrReplace, (PSZ)pcszReplace);
                // these two are never freed, so we're safe too

    if ((ulrc = xstrFindReplace(&xstrBuf,
                                pulOfs,
                                &xstrFind,
                                &xstrReplace,
                                ShiftTable,
                                &fRepeat)))
        // replaced:
        *ppszBuf = xstrBuf.psz;

    return ulrc;
}

/*
 * strhWords:
 *      returns the no. of words in "psz".
 *      A string is considered a "word" if
 *      it is surrounded by spaces only.
 *
 *@@added V0.9.0 [umoeller]
 */

ULONG strhWords(PSZ psz)
{
    PSZ p;
    ULONG cb = strlen(psz),
          ulWords = 0;
    if (cb > 1)
    {
        ulWords = 1;
        for (p = psz; p < psz+cb; p++)
            if (*p == ' ')
                ulWords++;
    }
    return ulWords;
}

/*
 *@@ strhGetWord:
 *      finds word boundaries.
 *
 *      *ppszStart is used as the beginning of the
 *      search.
 *
 *      If a word is found, *ppszStart is set to
 *      the first character of the word which was
 *      found and *ppszEnd receives the address
 *      of the first character _after_ the word,
 *      which is probably a space or a \n or \r char.
 *      We then return TRUE.
 *
 *      The search is stopped if a null character
 *      is found or pLimit is reached. In that case,
 *      FALSE is returned.
 *
 *@@added V0.9.1 (2000-02-13) [umoeller]
 */

BOOL strhGetWord(PSZ *ppszStart,        // in: start of search,
                                        // out: start of word (if TRUE is returned)
                 PCSZ pLimit,    // in: ptr to last char after *ppszStart to be
                                        // searched; if the word does not end before
                                        // or with this char, FALSE is returned
                 PCSZ pcszBeginChars, // stringh.h defines STRH_BEGIN_CHARS
                 PCSZ pcszEndChars, // stringh.h defines STRH_END_CHARS
                 PSZ *ppszEnd)          // out: first char _after_ word
                                        // (if TRUE is returned)
{
    // characters after which a word can be started
    // PCSZ pcszBeginChars = "\x0d\x0a ";
    // PCSZ pcszEndChars = "\x0d\x0a /-";

    PSZ pStart = *ppszStart;

    // find start of word
    while (     (pStart < (PSZ)pLimit)
             && (strchr(pcszBeginChars, *pStart))
          )
        // if char is a "before word" char: go for next
        pStart++;

    if (pStart < (PSZ)pLimit)
    {
        // found a valid "word start" character
        // (which is not in pcszBeginChars):

        // find end of word
        PSZ  pEndOfWord = pStart;
        while (     (pEndOfWord <= (PSZ)pLimit)
                 && (strchr(pcszEndChars, *pEndOfWord) == 0)
              )
            // if char is not an "end word" char: go for next
            pEndOfWord++;

        if (pEndOfWord <= (PSZ)pLimit)
        {
            // whoa, got a word:
            *ppszStart = pStart;
            *ppszEnd = pEndOfWord;
            return TRUE;
        }
    }

    return FALSE;
}

/*
 *@@ strhIsWord:
 *      returns TRUE if p points to a "word"
 *      in pcszBuf.
 *
 *      p is considered a word if the character _before_
 *      it is in pcszBeginChars and the char _after_
 *      it (i.e. *(p+cbSearch)) is in pcszEndChars.
 *
 *@@added V0.9.6 (2000-11-12) [umoeller]
 *@@changed V0.9.18 (2002-02-23) [umoeller]: fixed end char check
 */

BOOL strhIsWord(PCSZ pcszBuf,
                PCSZ p,                 // in: start of word
                ULONG cbSearch,         // in: length of word
                PCSZ pcszBeginChars,    // suggestion: "\x0d\x0a ()/\\-,."
                PCSZ pcszEndChars)      // suggestion: "\x0d\x0a ()/\\-,.:;"
{
    // check previous char
    if (    (p == pcszBuf)
         || (strchr(pcszBeginChars, *(p-1)))
       )
    {
        // OK, valid begin char:
        // check end char
        CHAR    cNextChar;
        if (!(cNextChar = p[cbSearch]))
            // null terminator:
            return TRUE;
        else
        {
            // not null terminator: check if char is
            // in the list of valid end chars
            if (strchr(pcszEndChars, cNextChar))
            {
                // OK, is end char: avoid doubles of that char,
                // but allow spaces
                // fixed V0.9.18 (2002-02-23) [umoeller]
                CHAR cNextNext = p[cbSearch + 1];
                if (    (cNextNext != cNextChar)
                     || (cNextNext == ' ')
                     || (cNextNext == 0)
                   )
                    return TRUE;
            }
        }
    }

    return FALSE;
}

/*
 *@@ strhFindWord:
 *      searches for pszSearch in pszBuf, which is
 *      returned if found (or NULL if not).
 *
 *      As opposed to strstr, this finds pszSearch
 *      only if it is a "word". A search string is
 *      considered a word if the character _before_
 *      it is in pcszBeginChars and the char _after_
 *      it is in pcszEndChars.
 *
 *      Example:
 +          strhFindWord("This is an example.", "is");
 +          returns ...........^ this, but not the "is" in "This".
 *
 *      The algorithm here uses strstr to find pszSearch in pszBuf
 *      and performs additional "is-word" checks for each item found
 *      (by calling strhIsWord).
 *
 *      Note that this function is fairly slow compared to xstrFindWord.
 *
 *@@added V0.9.0 (99-11-08) [umoeller]
 *@@changed V0.9.0 (99-11-10) [umoeller]: tried second algorithm, reverted to original...
 */

PSZ strhFindWord(PCSZ pszBuf,
                 PCSZ pszSearch,
                 PCSZ pcszBeginChars,    // suggestion: "\x0d\x0a ()/\\-,."
                 PCSZ pcszEndChars)      // suggestion: "\x0d\x0a ()/\\-,.:;"
{
    PSZ     pszReturn = 0;
    ULONG   cbBuf = strlen(pszBuf),
            cbSearch = strlen(pszSearch);

    if ((cbBuf) && (cbSearch))
    {
        PCSZ p = pszBuf;

        do  // while p
        {
            p = strstr(p, pszSearch);
            if (p)
            {
                // string found:
                // check if that's a word

                if (strhIsWord(pszBuf,
                               p,
                               cbSearch,
                               pcszBeginChars,
                               pcszEndChars))
                {
                    // valid end char:
                    pszReturn = (PSZ)p;
                    break;
                }

                p += cbSearch;
            }
        } while (p);

    }
    return pszReturn;
}

/*
 *@@ strhFindEOL:
 *      returns a pointer to the next \r, \n or null character
 *      following pszSearchIn. Stores the offset in *pulOffset.
 *
 *      This should never return NULL because at some point,
 *      there will be a null byte in your string.
 *
 *@@added V0.9.4 (2000-07-01) [umoeller]
 */

PSZ strhFindEOL(PCSZ pcszSearchIn,        // in: where to search
                PULONG pulOffset)       // out: offset (ptr can be NULL)
{
    PCSZ    p = pcszSearchIn,
            prc = 0;
    while (TRUE)
    {
        if ( (*p == '\r') || (*p == '\n') || (*p == 0) )
        {
            prc = p;
            break;
        }
        p++;
    }

    if ((pulOffset) && (prc))
        *pulOffset = prc - pcszSearchIn;

    return (PSZ)prc;
}

/*
 *@@ strhFindNextLine:
 *      like strhFindEOL, but this returns the character
 *      _after_ \r or \n. Note that this might return
 *      a pointer to terminating NULL character also.
 */

PSZ strhFindNextLine(PSZ pszSearchIn, PULONG pulOffset)
{
    PSZ pEOL = strhFindEOL(pszSearchIn, NULL);
    // pEOL now points to the \r char or the terminating 0 byte;
    // if not null byte, advance pointer
    PSZ pNextLine = pEOL;
    if (*pNextLine == '\r')
        pNextLine++;
    if (*pNextLine == '\n')
        pNextLine++;
    if (pulOffset)
        *pulOffset = pNextLine - pszSearchIn;
    return pNextLine;
}

/*
 *@@ strhBeautifyTitle:
 *      replaces all line breaks (0xd, 0xa) with spaces.
 *      Returns the new length of the string or 0 on
 *      errors.
 *
 *@@changed V0.9.12 (2001-05-17) [pr]: multiple line break chars. end up as only 1 space
 *@@changed V0.9.19 (2002-06-18) [umoeller]: now returning length
 */

ULONG strhBeautifyTitle(PSZ psz)
{
    ULONG   ulrc;
    PSZ     p = psz;

    while (*p)
    {
        if (    (*p == '\r')
             || (*p == '\n')
           )
        {
            if (    (p != psz)
                 && (p[-1] == ' ')
               )
                memmove(p, p + 1, strlen(p));
            else
                *p++ = ' ';
        }
        else
            p++;
    }

    return (p - psz);
}

/*
 *@@ strhBeautifyTitle:
 *      like strhBeautifyTitle, but copies into
 *      a new buffer. More efficient.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 */

ULONG strhBeautifyTitle2(PSZ pszTarget,     // out: beautified string
                         PCSZ pcszSource)   // in: string to be beautified (can be NULL)
{
    ULONG   ulrc;
    PCSZ    pSource = pcszSource;
    PSZ     pTarget = pszTarget;
    CHAR    c;
    if (!pcszSource)
    {
        *pszTarget = '\0';
        return 0;
    }

    while (c = *pSource++)
    {
        if (    (c == '\r')
             || (c == '\n')
           )
        {
            if (    (pTarget == pszTarget)
                 || (pTarget[-1] != ' ')
               )
                *pTarget++ = ' ';
        }
        else
            *pTarget++ = c;
    }

    // null-terminate
    *pTarget = '\0';

    return (pTarget - pszTarget);
}

/*
 *@@ strhKillChar:
 *      removes the first occurence of c in psz
 *      by overwriting it with the following characters.
 *
 *      For this to work, you _must_ pass in strlen(psz)
 *      in the ULONG pointed to by ulLength. If
 *
 *      Returns TRUE only if c was actually found. In
 *      that case, *pulLength is decremented.
 *
 *@@added V1.0.1 (2003-01-30) [umoeller]
 */

BOOL strhKillChar(PSZ psz,
                  CHAR c,
                  PULONG pulLength)
{
    PSZ p;
    if (p = strchr(psz, c))
    {
        // "string~rest"
        //  ¿ƒƒƒƒƒŸ 6 chars (p - pszBuf)
        //  ¿ƒƒƒƒƒƒƒƒƒƒŸ 11 chars (ulLen)
        //        ^ p (pszBuf + 6)
        //  ^ pszBuf
        memmove(p,                      // pszBuf + 6
                p + 1,                  // pszBuf + 7
                // include null byte
                  *pulLength            // 11
                - (p - psz));           // - 6 = 5
        --(*pulLength);

        return TRUE;
    }

    return FALSE;
}

/*
 * strhFindAttribValue:
 *      searches for pszAttrib in pszSearchIn; if found,
 *      returns the first character after the "=" char.
 *      If "=" is not found, a space, \r, and \n are
 *      also accepted. This function searches without
 *      respecting case.
 *
 *      <B>Example:</B>
 +          strhFindAttribValue("<PAGE BLAH=\"data\">", "BLAH")
 +
 +          returns ....................... ^ this address.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.3 (2000-05-19) [umoeller]: some speed optimizations
 *@@changed V0.9.12 (2001-05-22) [umoeller]: fixed space bug, thanks Yuri Dario
 *@@changed WarpIN V1.0.11 (2006-08-29) [pr]: handle attrib names in quoted strings @@fixes 718
 *@@changed WarpIN V1.0.12 (2006-09-07) [pr]: fix attrib handling again @@fixes 718 @@fixes 836
 *@@changed WarpIN V1.0.23 (2016-09-23) [pr]: fix single quote handling and allow escaping @@fixes 1244
 */

PSZ strhFindAttribValue(const char *pszSearchIn, const char *pszAttrib)
{
    PSZ    prc = 0;
    PSZ    pszSearchIn2, p, pszStart, pszName, pszValue;
    ULONG  cbAttrib = strlen(pszAttrib),
           ulLength = strlen(pszSearchIn);
    BOOL   fInQuote = FALSE;
    CHAR   cQuote = '\0', cPrev = '\0';

    // use alloca(), so memory is freed on function exit
    pszSearchIn2 = (PSZ)alloca(ulLength + 1);
    memcpy(pszSearchIn2, pszSearchIn, ulLength + 1);

    // V1.0.12 (2006-09-07) [pr]: filter leading " and ' left over from the previous pass
    for (p = pszSearchIn2;   *p == '\'' || *p == '"'  || *p == ' '
                          || *p == '\n' || *p == '\r' || *p == '\t'; p++);
    for (pszStart = p; *p; cPrev = *(p++))
    {
        if (fInQuote)
        {
            // V1.0.12 (2006-09-07) [pr]: allow end of line to terminate a (broken) quote
            if ((*p == cQuote && cPrev != '\\') || *p == '\n' || *p == '\r')
                fInQuote = FALSE;
        }
        else
        {
            if (*p == '"' || *p == '\'')
            {
                fInQuote = TRUE;
                cQuote = *p;
            }
            else
            {
                if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
                {
                    *p = '\0';
                    pszName = strtok(pszStart, "=>");
                    pszStart = p + 1;
                    if (pszName && !stricmp(pszName, pszAttrib))
                    {
                        pszValue = strtok(NULL, "");
                        if (pszValue)
                            prc = (PSZ)pszSearchIn + (pszValue - pszSearchIn2);
                        else
                            prc = (PSZ)pszSearchIn + (pszName - pszSearchIn2) + cbAttrib;

                        return(prc);
                    }
                }
            }
        }
    }

    if (pszStart != p)
    {
        pszName = strtok(pszStart, "=>");
        if (pszName && !stricmp(pszName, pszAttrib))
        {
            pszValue = strtok(NULL, "");
            if (pszValue)
                prc = (PSZ)pszSearchIn + (pszValue - pszSearchIn2);
            else
                prc = (PSZ)pszSearchIn + (pszName - pszSearchIn2) + cbAttrib;
        }
    }

    return prc;
}

/*
 * strhGetNumAttribValue:
 *      stores the numerical parameter value of an HTML-style
 *      tag in *pl.
 *
 *      Returns the address of the tag parameter in the
 *      search buffer, if found, or NULL.
 *
 *      <B>Example:</B>
 +          strhGetNumAttribValue("<PAGE BLAH=123>, "BLAH", &l);
 *
 *      stores 123 in the "l" variable.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.9 (2001-04-04) [umoeller]: this failed on "123" strings in quotes, fixed
 */

PSZ strhGetNumAttribValue(const char *pszSearchIn,       // in: where to search
                          const char *pszTag,            // e.g. "INDEX"
                          PLONG pl)              // out: numerical value
{
    PSZ pParam;
    if ((pParam = strhFindAttribValue(pszSearchIn, pszTag)))
    {
        if (    (*pParam == '\"')
             || (*pParam == '\'')
           )
            pParam++;           // V0.9.9 (2001-04-04) [umoeller]

        sscanf(pParam, "%ld", pl);
    }

    return pParam;
}

/*
 * strhGetTextAttr:
 *      retrieves the attribute value of a textual HTML-style tag
 *      in a newly allocated buffer, which is returned,
 *      or NULL if attribute not found.
 *      If an attribute value is to contain spaces, it
 *      must be enclosed in quotes.
 *
 *      The offset of the attribute data in pszSearchIn is
 *      returned in *pulOffset so that you can do multiple
 *      searches.
 *
 *      This returns a new buffer, which should be free()'d after use.
 *
 *      <B>Example:</B>
 +          ULONG   ulOfs = 0;
 +          strhGetTextAttr("<PAGE BLAH="blublub">, "BLAH", &ulOfs)
 +                           ............^ ulOfs
 *
 *      returns a new string with the value "blublub" (without
 *      quotes) and sets ulOfs to 12.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V1.0.13 (2006-09-10) [pr]: improved parsing
 *@@changed WarpIN V1.0.23 (2016-09-23) [pr]: allow escaping @@fixes 1244
 */

PSZ strhGetTextAttr(const char *pszSearchIn,
                    const char *pszTag,
                    PULONG pulOffset)       // out: offset where found
{
    PSZ     pParam,
            pParam2,
            prc = NULL;
    ULONG   ulCount = 0;
    LONG    lNestingLevel = 0;

    if ((pParam = strhFindAttribValue(pszSearchIn, pszTag)))
    {
        // determine end character to search for: a space
        CHAR cEnd = ' ', cPrev = '\0';
        // V1.0.3 (2004-11-10) [pr]: @@fixes 461
        // V1.0.13 (2006-09-10) [pr]: optimized
        if ((*pParam == '"') || (*pParam == '\''))
        {
            // or, if the data is enclosed in quotes, a quote or single quote
            cEnd = *pParam;
            pParam++;
        }

        if (pulOffset)
            // store the offset
            (*pulOffset) = pParam - (PSZ)pszSearchIn;

        // now find end of attribute
        pParam2 = pParam;
        while (*pParam)
        {
            // V1.0.13 (2006-09-10) [pr]: line end terminates non-quoted attribute
            if (   (   (cEnd == ' ')
                    && ((*pParam == ' ') || (*pParam == '\r') || (*pParam == '\n')))
                || ((*pParam == cEnd) && (cPrev != '\\'))
               )
                // end character found
                break;
            else if (*pParam == '<')
                // yet another opening tag found:
                // this is probably some "<" in the attributes
                lNestingLevel++;
            else if (*pParam == '>')
            {
                lNestingLevel--;
                if (lNestingLevel < 0)
                    // end of tag found:
                    break;
            }

            ulCount++;
            cPrev = *(pParam++);
        }

        // copy attribute to new buffer, de-escaping if necessary
        if (ulCount && (prc = (PSZ) malloc(ulCount+1)))
        {
            ULONG i = 0, j = 0;

            for(cPrev = '\0'; i < ulCount; ++i, ++j)
            {
                if (cEnd != ' ' && pParam2[i] == cEnd && cPrev == '\\')
                    --j;

                cPrev = prc[j] = pParam2[i];
            }

            prc[j] = '\0';
        }
    }
    return prc;
}

/*
 * strhFindEndOfTag:
 *      returns a pointer to the ">" char
 *      which seems to terminate the tag beginning
 *      after pszBeginOfTag.
 *
 *      If additional "<" chars are found, we look
 *      for additional ">" characters too.
 *
 *      Note: You must pass the address of the opening
 *      '<' character to this function.
 *
 *      Example:
 +          PSZ pszTest = "<BODY ATTR=\"<BODY>\">";
 +          strhFindEndOfTag(pszTest)
 +      returns.................................^ this.
 *
 *@@added V0.9.0 [umoeller]
 */

PSZ strhFindEndOfTag(const char *pszBeginOfTag)
{
    PSZ     p = (PSZ)pszBeginOfTag,
            prc = NULL;
    LONG    lNestingLevel = 0;

    while (*p)
    {
        if (*p == '<')
            // another opening tag found:
            lNestingLevel++;
        else if (*p == '>')
        {
            // closing tag found:
            lNestingLevel--;
            if (lNestingLevel < 1)
            {
                // corresponding: return this
                prc = p;
                break;
            }
        }
        p++;
    }

    return prc;
}

/*
 * strhGetBlock:
 *      this complex function searches the given string
 *      for a pair of opening/closing HTML-style tags.
 *
 *      If found, this routine returns TRUE and does
 *      the following:
 *
 *          1)  allocate a new buffer, copy the text
 *              enclosed by the opening/closing tags
 *              into it and set *ppszBlock to that
 *              buffer;
 *
 *          2)  if the opening tag has any attributes,
 *              allocate another buffer, copy the
 *              attributes into it and set *ppszAttrs
 *              to that buffer; if no attributes are
 *              found, *ppszAttrs will be NULL;
 *
 *          3)  set *pulOffset to the offset from the
 *              beginning of *ppszSearchIn where the
 *              opening tag was found;
 *
 *          4)  advance *ppszSearchIn to after the
 *              closing tag, so that you can do
 *              multiple searches without finding the
 *              same tags twice.
 *
 *      All buffers should be freed using free().
 *
 *      This returns the following:
 *      --  0: no error
 *      --  1: tag not found at all (doesn't have to be an error)
 *      --  2: begin tag found, but no corresponding end tag found. This
 *             is a real error.
 *      --  3: begin tag is not terminated by "&gt;" (e.g. "&lt;BEGINTAG whatever")
 *
 *      <B>Example:</B>
 +          PSZ pSearch = "&lt;PAGE INDEX=1&gt;This is page 1.&lt;/PAGE&gt;More text."
 +          PSZ pszBlock, pszAttrs;
 +          ULONG ulOfs;
 +          strhGetBlock(&pSearch, "PAGE", &pszBlock, &pszAttrs, &ulOfs)
 *
 *      would do the following:
 *
 *      1)  set pszBlock to a new string containing "This is page 1."
 *          without quotes;
 *
 *      2)  set pszAttrs to a new string containing "&lt;PAGE INDEX=1&gt;";
 *
 *      3)  set ulOfs to 0, because "&lt;PAGE" was found at the beginning;
 *
 *      4)  pSearch would be advanced to point to the "More text"
 *          string in the original buffer.
 *
 *      Hey-hey. A one-shot function, fairly complicated, but indispensable
 *      for HTML parsing.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (2000-01-03) [umoeller]: fixed heap overwrites (thanks to string debugging)
 *@@changed V0.9.1 (2000-01-06) [umoeller]: changed prototype
 *@@changed V0.9.3 (2000-05-06) [umoeller]: NULL string check was missing
 */

ULONG strhGetBlock(const char *pszSearchIn, // in: buffer to search
                   PULONG pulSearchOffset, // in/out: offset where to start search (0 for beginning)
                   const char *pszTag,
                   PSZ *ppszBlock,      // out: block enclosed by the tags
                   PSZ *ppszAttribs,    // out: attributes of the opening tag
                   PULONG pulOfsBeginTag, // out: offset from pszSearchIn where opening tag was found
                   PULONG pulOfsBeginBlock) // out: offset from pszSearchIn where beginning of block was found
{
    ULONG   ulrc = 1;
    PSZ     pszBeginTag = (PSZ)pszSearchIn + *pulSearchOffset,
            pszSearch2 = pszBeginTag,
            pszClosingTag;
    ULONG   cbTag = strlen(pszTag);

    // go thru the block and check all tags if it's the
    // begin tag we're looking for
    while ((pszBeginTag = strchr(pszBeginTag, '<')))
    {
        if (memicmp(pszBeginTag+1, (void*)pszTag, strlen(pszTag)) == 0)
            // yes: stop
            break;
        else
            pszBeginTag++;
    }

    if (pszBeginTag)
    {
        // we found <TAG>:
        ULONG   ulNestingLevel = 0;

        PSZ     pszEndOfBeginTag = strhFindEndOfTag(pszBeginTag);
                                    // strchr(pszBeginTag, '>');
        if (pszEndOfBeginTag)
        {
            // does the caller want the attributes?
            if (ppszAttribs)
            {
                // yes: then copy them
                ULONG   ulAttrLen = pszEndOfBeginTag - pszBeginTag;
                PSZ     pszAttrs = (PSZ)malloc(ulAttrLen + 1);
                strncpy(pszAttrs, pszBeginTag, ulAttrLen);
                // add terminating 0
                *(pszAttrs + ulAttrLen) = 0;

                *ppszAttribs = pszAttrs;
            }

            // output offset of where we found the begin tag
            if (pulOfsBeginTag)
                *pulOfsBeginTag = pszBeginTag - (PSZ)pszSearchIn;

            // now find corresponding closing tag (e.g. "</BODY>"
            pszBeginTag = pszEndOfBeginTag+1;
            // now we're behind the '>' char of the opening tag
            // increase offset of that too
            if (pulOfsBeginBlock)
                *pulOfsBeginBlock = pszBeginTag - (PSZ)pszSearchIn;

            // find next closing tag;
            // for the first run, pszSearch2 points to right
            // after the '>' char of the opening tag
            pszSearch2 = pszBeginTag;
            while (     (pszSearch2)        // fixed V0.9.3 (2000-05-06) [umoeller]
                    &&  (pszClosingTag = strstr(pszSearch2, "<"))
                  )
            {
                // if we have another opening tag before our closing
                // tag, we need to have several closing tags before
                // we're done
                if (memicmp(pszClosingTag+1, (void*)pszTag, cbTag) == 0)
                    ulNestingLevel++;
                else
                {
                    // is this ours?
                    if (    (*(pszClosingTag+1) == '/')
                         && (memicmp(pszClosingTag+2, (void*)pszTag, cbTag) == 0)
                       )
                    {
                        // we've found a matching closing tag; is
                        // it ours?
                        if (ulNestingLevel == 0)
                        {
                            // our closing tag found:
                            // allocate mem for a new buffer
                            // and extract all the text between
                            // open and closing tags to it
                            ULONG ulLen = pszClosingTag - pszBeginTag;
                            if (ppszBlock)
                            {
                                PSZ pNew = (PSZ)malloc(ulLen + 1);
                                strhncpy0(pNew, pszBeginTag, ulLen);
                                *ppszBlock = pNew;
                            }

                            // raise search offset to after the closing tag
                            *pulSearchOffset = (pszClosingTag + cbTag + 1) - (PSZ)pszSearchIn;

                            ulrc = 0;

                            break;
                        } else
                            // not our closing tag:
                            ulNestingLevel--;
                    }
                }
                // no matching closing tag: search on after that
                pszSearch2 = strhFindEndOfTag(pszClosingTag);
            } // end while (pszClosingTag = strstr(pszSearch2, "<"))

            if (!pszClosingTag)
                // no matching closing tag found:
                // return 2 (closing tag not found)
                ulrc = 2;
        } // end if (pszBeginTag)
        else
            // no matching ">" for opening tag found:
            ulrc = 3;
    }

    return ulrc;
}

/* ******************************************************************
 *
 *   Miscellaneous
 *
 ********************************************************************/

/*
 *@@ strhArrayAppend:
 *      this appends a string to a "string array".
 *
 *      A string array is considered a sequence of
 *      zero-terminated strings in memory. That is,
 *      after each string's null-byte, the next
 *      string comes up.
 *
 *      This is useful for composing a single block
 *      of memory from, say, list box entries, which
 *      can then be written to OS2.INI in one flush.
 *
 *      To append strings to such an array, call this
 *      function for each string you wish to append.
 *      This will re-allocate *ppszRoot with each call,
 *      and update *pcbRoot, which then contains the
 *      total size of all strings (including all null
 *      terminators).
 *
 *      Pass *pcbRoot to PrfSaveProfileData to have the
 *      block saved.
 *
 *      Note: On the first call, *ppszRoot and *pcbRoot
 *      _must_ be both NULL, or this crashes.
 *
 *@@changed V0.9.13 (2001-06-21) [umoeller]: added cbNew
 */

VOID strhArrayAppend(PSZ *ppszRoot,         // in: root of array
                     const char *pcszNew,   // in: string to append
                     ULONG cbNew,           // in: size of that string or 0 to run strlen() here
                     PULONG pcbRoot)        // in/out: size of array
{
    PSZ pszTemp;

    if (!cbNew)     // V0.9.13 (2001-06-21) [umoeller]
        cbNew = strlen(pcszNew);

    pszTemp = (PSZ)malloc(*pcbRoot
                          + cbNew
                          + 1);    // two null bytes
    if (*ppszRoot)
    {
        // not first loop: copy old stuff
        memcpy(pszTemp,
               *ppszRoot,
               *pcbRoot);
        free(*ppszRoot);
    }
    // append new string
    strcpy(pszTemp + *pcbRoot,
           pcszNew);
    // update root
    *ppszRoot = pszTemp;
    // update length
    *pcbRoot += cbNew + 1;
}

/*
 *@@ strhCreateDump:
 *      this dumps a memory block into a string
 *      and returns that string in a new buffer.
 *
 *      You must free() the returned PSZ after use.
 *
 *      The output looks like the following:
 *
 +          0000:  FE FF 0E 02 90 00 00 00   ........
 +          0008:  FD 01 00 00 57 50 46 6F   ....WPFo
 +          0010:  6C 64 65 72 00 78 01 34   lder.x.4
 *
 *      Each line is terminated with a newline (\n)
 *      character only.
 *
 *@@added V0.9.1 (2000-01-22) [umoeller]
 */

PSZ strhCreateDump(PBYTE pb,            // in: start address of buffer
                   ULONG ulSize,        // in: size of buffer
                   ULONG ulIndent)      // in: indentation of every line
{
    PSZ     pszReturn = 0;
    XSTRING strReturn;
    CHAR    szTemp[1000];

    PBYTE   pbCurrent = pb;                 // current byte
    ULONG   ulCount = 0,
            ulCharsInLine = 0;              // if this grows > 7, a new line is started
    CHAR    szLine[400] = "",
            szAscii[30] = "         ";      // ASCII representation; filled for every line
    PSZ     pszLine = szLine,
            pszAscii = szAscii;

    xstrInit(&strReturn, (ulSize * 30) + ulIndent);

    for (pbCurrent = pb;
         ulCount < ulSize;
         pbCurrent++, ulCount++)
    {
        if (ulCharsInLine == 0)
        {
            memset(szLine, ' ', ulIndent);
            pszLine += ulIndent;
        }
        pszLine += sprintf(pszLine, "%02lX ", (ULONG)*pbCurrent);

        if ( (*pbCurrent > 31) && (*pbCurrent < 127) )
            // printable character:
            *pszAscii = *pbCurrent;
        else
            *pszAscii = '.';
        pszAscii++;

        ulCharsInLine++;
        if (    (ulCharsInLine > 7)         // 8 bytes added?
             || (ulCount == ulSize-1)       // end of buffer reached?
           )
        {
            // if we haven't had eight bytes yet,
            // fill buffer up to eight bytes with spaces
            ULONG   ul2;
            for (ul2 = ulCharsInLine;
                 ul2 < 8;
                 ul2++)
                pszLine += sprintf(pszLine, "   ");

            sprintf(szTemp, "%04lX:  %s  %s\n",
                            (ulCount & 0xFFFFFFF8),  // offset in hex
                            szLine,         // bytes string
                            szAscii);       // ASCII string
            xstrcat(&strReturn, szTemp, 0);

            // restart line buffer
            pszLine = szLine;

            // clear ASCII buffer
            strcpy(szAscii, "         ");
            pszAscii = szAscii;

            // reset line counter
            ulCharsInLine = 0;
        }
    }

    if (strReturn.cbAllocated)
        pszReturn = strReturn.psz;

    return pszReturn;
}

/* ******************************************************************
 *
 *   Fast string searches
 *
 ********************************************************************/

#define ASSERT(a)

/*
 *      The following code has been taken from the "Standard
 *      Function Library", file sflfind.c, and only slightly
 *      modified to conform to the rest of this file.
 *
 *      Written:    96/04/24  iMatix SFL project team <sfl@imatix.com>
 *      Revised:    98/05/04
 *
 *      Copyright:  Copyright (c) 1991-99 iMatix Corporation.
 *
 *      The SFL Licence allows incorporating SFL code into other
 *      programs, as long as the copyright is reprinted and the
 *      code is marked as modified, so this is what we do.
 */

/*
 *@@ strhmemfind:
 *      searches for a pattern in a block of memory using the
 *      Boyer-Moore-Horspool-Sunday algorithm.
 *
 *      The block and pattern may contain any values; you must
 *      explicitly provide their lengths. If you search for strings,
 *      use strlen() on the buffers.
 *
 *      Returns a pointer to the pattern if found within the block,
 *      or NULL if the pattern was not found.
 *
 *      This algorithm needs a "shift table" to cache data for the
 *      search pattern. This table can be reused when performing
 *      several searches with the same pattern.
 *
 *      "shift" must point to an array big enough to hold 256 (8**2)
 *      "size_t" values.
 *
 *      If (*repeat_find == FALSE), the shift table is initialized.
 *      So on the first search with a given pattern, *repeat_find
 *      should be FALSE. This function sets it to TRUE after the
 *      shift table is initialised, allowing the initialisation
 *      phase to be skipped on subsequent searches.
 *
 *      This function is most effective when repeated searches are
 *      made for the same pattern in one or more large buffers.
 *
 *      Example:
 *
 +          PSZ     pszHaystack = "This is a sample string.",
 +                  pszNeedle = "string";
 +          size_t  shift[256];
 +          BOOL    fRepeat = FALSE;
 +
 +          PSZ     pFound = strhmemfind(pszHaystack,
 +                                       strlen(pszHaystack),   // block size
 +                                       pszNeedle,
 +                                       strlen(pszNeedle),     // pattern size
 +                                       shift,
 +                                       &fRepeat);
 *
 *      Taken from the "Standard Function Library", file sflfind.c.
 *      Copyright:  Copyright (c) 1991-99 iMatix Corporation.
 *      Slightly modified by umoeller.
 *
 *@@added V0.9.3 (2000-05-08) [umoeller]
 */

void* strhmemfind(const void *in_block,     // in: block containing data
                  size_t block_size,        // in: size of block in bytes
                  const void *in_pattern,   // in: pattern to search for
                  size_t pattern_size,      // in: size of pattern block
                  size_t *shift,            // in/out: shift table (search buffer)
                  BOOL *repeat_find)        // in/out: if TRUE, *shift is already initialized
{
    size_t      byte_nbr,                       //  Distance through block
                match_size;                     //  Size of matched part
    const unsigned char
                *match_base = NULL,             //  Base of match of pattern
                *match_ptr  = NULL,             //  Point within current match
                *limit      = NULL;             //  Last potiental match point
    const unsigned char
                *block   = (unsigned char *) in_block,   //  Concrete pointer to block data
                *pattern = (unsigned char *) in_pattern; //  Concrete pointer to search value

    if (    (block == NULL)
         || (pattern == NULL)
         || (shift == NULL)
       )
        return NULL;

    //  Pattern must be smaller or equal in size to string
    if (block_size < pattern_size)
        return NULL;                  //  Otherwise it's not found

    if (pattern_size == 0)              //  Empty patterns match at start
        return (void*)block;

    //  Build the shift table unless we're continuing a previous search

    //  The shift table determines how far to shift before trying to match
    //  again, if a match at this point fails.  If the byte after where the
    //  end of our pattern falls is not in our pattern, then we start to
    //  match again after that byte; otherwise we line up the last occurence
    //  of that byte in our pattern under that byte, and try match again.

    if (!repeat_find || !*repeat_find)
    {
        for (byte_nbr = 0;
             byte_nbr < 256;
             byte_nbr++)
            shift[byte_nbr] = pattern_size + 1;
        for (byte_nbr = 0;
             byte_nbr < pattern_size;
             byte_nbr++)
            shift[(unsigned char)pattern[byte_nbr]] = pattern_size - byte_nbr;

        if (repeat_find)
            *repeat_find = TRUE;
    }

    //  Search for the block, each time jumping up by the amount
    //  computed in the shift table

    limit = block + (block_size - pattern_size + 1);
    ASSERT (limit > block);

    for (match_base = block;
         match_base < limit;
         match_base += shift[*(match_base + pattern_size)])
    {
        match_ptr  = match_base;
        match_size = 0;

        //  Compare pattern until it all matches, or we find a difference
        while (*match_ptr++ == pattern[match_size++])
        {
            ASSERT (match_size <= pattern_size &&
                    match_ptr == (match_base + match_size));

            // If we found a match, return the start address
            if (match_size >= pattern_size)
                return (void*)match_base;

        }
    }
    return NULL;                      //  Found nothing
}

/*
 *@@ strhtxtfind:
 *      searches for a case-insensitive text pattern in a string
 *      using the Boyer-Moore-Horspool-Sunday algorithm.  The string and
 *      pattern are null-terminated strings.  Returns a pointer to the pattern
 *      if found within the string, or NULL if the pattern was not found.
 *      Will match strings irrespective of case.  To match exact strings, use
 *      strhfind().  Will not work on multibyte characters.
 *
 *      Examples:
 +      char *result;
 +
 +      result = strhtxtfind ("AbracaDabra", "cad");
 +      if (result)
 +          puts (result);
 +
 *      Taken from the "Standard Function Library", file sflfind.c.
 *      Copyright:  Copyright (c) 1991-99 iMatix Corporation.
 *      Slightly modified.
 *
 *@@added V0.9.3 (2000-05-08) [umoeller]
 */

char* strhtxtfind (const char *string,            //  String containing data
                   const char *pattern)           //  Pattern to search for
{
    size_t
        shift [256];                    //  Shift distance for each value
    size_t
        string_size,
        pattern_size,
        byte_nbr,                       //  Index into byte array
        match_size;                     //  Size of matched part
    const char
        *match_base = NULL,             //  Base of match of pattern
        *match_ptr  = NULL,             //  Point within current match
        *limit      = NULL;             //  Last potiental match point

    ASSERT (string);                    //  Expect non-NULL pointers, but
    ASSERT (pattern);                   //  fail gracefully if not debugging
    if (string == NULL || pattern == NULL)
        return NULL;

    string_size  = strlen (string);
    pattern_size = strlen (pattern);

    //  Pattern must be smaller or equal in size to string
    if (string_size < pattern_size)
        return NULL;                  //  Otherwise it cannot be found

    if (pattern_size == 0)              //  Empty string matches at start
        return (char*)string;

    //  Build the shift table

    //  The shift table determines how far to shift before trying to match
    //  again, if a match at this point fails.  If the byte after where the
    //  end of our pattern falls is not in our pattern, then we start to
    //  match again after that byte; otherwise we line up the last occurence
    //  of that byte in our pattern under that byte, and try match again.

    for (byte_nbr = 0; byte_nbr < 256; byte_nbr++)
        shift [byte_nbr] = pattern_size + 1;

    for (byte_nbr = 0; byte_nbr < pattern_size; byte_nbr++)
        shift [(unsigned char) tolower (pattern [byte_nbr])] = pattern_size - byte_nbr;

    //  Search for the string.  If we don't find a match, move up by the
    //  amount we computed in the shift table above, to find location of
    //  the next potiental match.

    limit = string + (string_size - pattern_size + 1);
    ASSERT (limit > string);

    for (match_base = string;
         match_base < limit;
         match_base += shift [(unsigned char) tolower (*(match_base + pattern_size))])
      {
        match_ptr  = match_base;
        match_size = 0;

        //  Compare pattern until it all matches, or we find a difference
        while (tolower (*match_ptr++) == tolower (pattern [match_size++]))
          {
            ASSERT (match_size <= pattern_size &&
                    match_ptr == (match_base + match_size));

            //  If we found a match, return the start address
            if (match_size >= pattern_size)
                return (char*)match_base;
          }
      }

    return NULL;                      //  Found nothing
}


