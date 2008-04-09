
/*
 *@@sourcefile xstring.c:
 *      string functions with memory management.
 *
 *      Usage: All OS/2 programs.
 *
 *      The functions in this file are intended as a replacement
 *      to the C library string functions (such as strcpy, strcat)
 *      in cases where the length of the string is unknown and
 *      dynamic memory management is desirable.
 *
 *      Instead of char* pointers, the functions in this file
 *      operate on XSTRING structures, which in turn contain
 *      a char* pointer pointing to heap memory, which is managed
 *      automatically.
 *
 *      Besides being convenient, these functions are highly
 *      optimized to use as few strlen's and memcpy's as
 *      possible.
 *
 *      Using these functions has the following advantages:
 *
 *      -- Automatic memory management. For example, xstrcat will
 *         automatically allocate new memory if the new string
 *         does not fit into the present buffer.
 *
 *      -- The length of the string is always known. Instead
 *         of running strlen (which consumes time), XSTRING.ulLength
 *         always contains the current length of the string.
 *
 *      -- The functions also differentiate between allocated
 *         memory and the length of the string. That is, for
 *         iterative appends, you can pre-allocate memory to
 *         avoid excessive reallocations.
 *
 *      These functions are also used internally by the
 *      WarpIN BSString class (and related classes).
 *
 *      Usage:
 *
 *      1) Allocate an XSTRING structure on the stack. Always
 *         call xstrInit on the structure, like this:
 *
 +              XSTRING str;
 +              xstrInit(&str, 0);      // no pre-allocation
 *
 *         Alternatively, use xstrCreate to have an XSTRING
 *         allocated from the heap.
 *
 *         Always call xstrClear(&str) to free allocated
 *         memory. Otherwise you'll get memory leaks.
 *         (For heap XSTRING's from xstrCreate, use xstrFree.)
 *
 *      2) To copy something into the string, use xstrcpy.
 *         To append something to the string, use xstrcat.
 *         See those functions for samples.
 *
 *      3) If you need the char* pointer (e.g. for a call
 *         to another function), use XSTRING.psz. However,
 *         you should ONLY modify the psz pointer yourself
 *         if the other XSTRING members are updated accordingly.
 *         You may, for example, change single characters
 *         in the psz buffer. By contrast, if you change the
 *         length of the string, you must update XSTRING.ulLength.
 *         Otherwise these functions will get into trouble.
 *
 *         Also, you should never assume that the "psz"
 *         pointer has not changed after you have called
 *         one of the xstr* functions because these can
 *         always reallocate the buffer if more memory
 *         was needed.
 *
 *      4) If (and only if) you have a char* buffer which
 *         is free()'able (e.g. from strdup()), you can
 *         use xstrset to avoid duplicate copying.
 *
 *      Function prefixes:
 *      --  xstr*       extended string functions.
 *
 *      The functions in this file used to be in stringh.c
 *      before V0.9.3 (2000-04-01). These have been largely
 *      rewritten with V0.9.6 (2000-11-01) and are now much
 *      more efficient.
 *
 *      Note: Version numbering in this file relates to XWorkplace
 *            version numbering.
 *
 *@@added V0.9.3 (2000-04-01) [umoeller]
 *@@header "helpers\xstring.h"
 */

/*
 *      Copyright (C) 1999-2002 Ulrich M”ller.
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

#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "stringh.h"
#define DONT_REPLACE_XSTR_MALLOC
#include "xstring.h"            // extended string helpers

/*
 *@@category: Helpers\C helpers\String management\XStrings (with memory management)
 *      See xstring.c.
 */

#ifdef __DEBUG_MALLOC_ENABLED__

/*
 *@@ xstrInitDebug:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

void XWPENTRY xstrInitDebug(PXSTRING pxstr,
                            ULONG ulPreAllocate,
                            PCSZ file,
                            unsigned long line,
                            PCSZ function)
{
    memset(pxstr, 0, sizeof(XSTRING));

    if (ulPreAllocate)
    {
        pxstr->psz = (PSZ)memdMalloc(ulPreAllocate,
                                     file,
                                     line,
                                     function);
        pxstr->cbAllocated = ulPreAllocate;
                // ulLength is still zero
        *(pxstr->psz) = 0;

        pxstr->ulDelta = ulPreAllocate * 10 / 100;
    }

    // else: pxstr->ulDelta is still 0
    pxstr->file = file;
    pxstr->line = line;
    pxstr->function = function;
}

#endif // __DEBUG_MALLOC_ENABLED__

/*
 *@@ xstrInit:
 *      initializes a new XSTRING. Always call this before
 *      using an XSTRING from the stack.
 *
 *      If (ulPreAllocate != 0), memory is pre-allocated
 *      for the string, but the string will be empty
 *      (its first byte is set to '\0'). In addition,
 *      pxstr->ulDelta will be set to 10% of ulPreAllocate.
 *
 *      This is useful if you plan to add more stuff to
 *      the string later so we don't have to reallocate
 *      all the time in xstrcat.
 *
 *      Do not use this on an XSTRING which is already
 *      initialized (this would cause memory leaks).
 *      Use xstrcpy or xstrset instead.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 *@@changed V0.9.9 (2001-03-09) [umoeller]: added ulDelta
 */

void xstrInit(PXSTRING pxstr,               // in/out: string
              ULONG ulPreAllocate)          // in: if > 0, memory to allocate
{
    memset(pxstr, 0, sizeof(XSTRING));

    if (ulPreAllocate)
    {
        pxstr->psz = (PSZ)malloc(ulPreAllocate);
        pxstr->cbAllocated = ulPreAllocate;
                // ulLength is still zero
        *(pxstr->psz) = 0;

        pxstr->ulDelta = ulPreAllocate * 10 / 100;
    }
    // else: pxstr->ulDelta is still 0
}

/*
 *@@ xstrInitSet2:
 *      this can be used instead of xstrInit if you
 *      have a free()'able string you want to initialize
 *      the XSTRING with.
 *
 *      This does not create a copy of pszNew. Instead,
 *      pszNew is used as the member string in pxstr
 *      directly.
 *
 *      Do not use this on an XSTRING which is already
 *      initialized (this would cause memory leaks).
 *      Use xstrcpy or xstrset instead.
 *
 *      Example:
 *
 +          XSTRING str;
 +          xstrInitSet(&str, strdup("blah"), 0);
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

void xstrInitSet2(PXSTRING pxstr,           // in/out: string
                  PSZ pszNew,               // in: malloc'd string to load pxstr with
                  ULONG ulNewLength)        // in: length of pszNew or 0 to run strlen()
{
    if (!pszNew)
        memset(pxstr, 0, sizeof(XSTRING));
    else
    {
        if (!ulNewLength)
            ulNewLength = strlen(pszNew);
        pxstr->psz = pszNew;
        pxstr->ulLength = ulNewLength;
        pxstr->cbAllocated = ulNewLength + 1;
        pxstr->ulDelta = ulNewLength * 10 / 100;
    }
}

/*
 *@@ xstrInitSet:
 *      shortcut to xstrInitSet2 to retain compatibility.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 *@@changed V0.9.9 (2001-03-09) [umoeller]: added ulDelta
 */

void xstrInitSet(PXSTRING pxstr,
                 PSZ pszNew)
{
    xstrInitSet2(pxstr, pszNew, 0);
}

#ifdef __DEBUG_MALLOC_ENABLED__

/*
 *@@ xstrInitCopyDebug:
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 */

void XWPENTRY xstrInitCopyDebug(PXSTRING pxstr,
                                PCSZ pcszSource,
                                ULONG ulExtraAllocate,
                                PCSZ file,
                                unsigned long line,
                                PCSZ function)
{
    if (pxstr)
    {
        memset(pxstr, 0, sizeof(XSTRING));

        if (pcszSource)
        {
            pxstr->ulLength = strlen(pcszSource);

            if (pxstr->ulLength)
            {
                // we do have a source string:
                pxstr->cbAllocated = pxstr->ulLength + 1 + ulExtraAllocate;
                pxstr->psz = (PSZ)memdMalloc(pxstr->cbAllocated,
                                             file,
                                             line,
                                             function);
                // V0.9.16 (2002-01-05) [umoeller]
                memcpy(pxstr->psz,
                       pcszSource,
                       pxstr->ulLength);
                pxstr->psz[pxstr->ulLength] = '\0';

                pxstr->ulDelta = pxstr->cbAllocated * 10 / 100;
            }
        }
    }
}

#endif

/*
 *@@ xstrInitCopy:
 *      this can be used instead of xstrInit if you
 *      want to initialize an XSTRING with a copy
 *      of an existing string. This is a shortcut
 *      for xstrInit() and then xstrcpy().
 *
 *      As opposed to xstrInitSet, this does create
 *      a copy of pcszSource.
 *
 *      Do not use this on an XSTRING which is already
 *      initialized (this would cause memory leaks).
 *      Use xstrcpy or xstrset instead.
 *
 *      Example:
 *
 +          XSTRING str;
 +          xstrInitCopy(&str, "blah");
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 *@@changed V0.9.7 (2000-12-31) [umoeller]: added ulExtraAllocate
 *@@changed V0.9.9 (2001-03-09) [umoeller]: added ulDelta
 *@@changed V0.9.16 (2002-01-05) [umoeller]: use memcpy instead of strcpy
 */

void XWPENTRY xstrInitCopy(PXSTRING pxstr,
                  PCSZ pcszSource,
                  ULONG ulExtraAllocate)          // in: if > 0, extra memory to allocate
{
    if (pxstr)
    {
        memset(pxstr, 0, sizeof(XSTRING));

        if (pcszSource)
        {
            if (pxstr->ulLength = strlen(pcszSource))
            {
                // we do have a source string:
                pxstr->cbAllocated = pxstr->ulLength + 1 + ulExtraAllocate;
                pxstr->psz = (PSZ)malloc(pxstr->cbAllocated);
                // V0.9.16 (2002-01-05) [umoeller]
                memcpy(pxstr->psz,
                       pcszSource,
                       pxstr->ulLength);
                pxstr->psz[pxstr->ulLength] = '\0';

                pxstr->ulDelta = pxstr->cbAllocated * 10 / 100;
            }
        }
    }
}

/*
 *@@ xstrClear:
 *      clears the specified stack XSTRING and
 *      frees allocated memory.
 *
 *      This is the reverse to xstrInit.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 */

void xstrClear(PXSTRING pxstr)              // in/out: string
{
    if (pxstr->psz)
        free(pxstr->psz);

    memset(pxstr, 0, sizeof(XSTRING));
}

/*
 *@@ xstrReserve:
 *      this function makes sure that the specified
 *      XSTRING has at least ulBytes bytes allocated.
 *
 *      This function is useful if you plan to do
 *      a lot of string replacements or appends and
 *      want to avoid that the buffer is reallocated
 *      with each operation. Before those operations,
 *      call this function to make room for the operations.
 *
 *      If ulBytes is smaller than the current allocation,
 *      this function does nothing.
 *
 *      pxstr->ulDelta has no effect here.
 *
 *      The XSTRING must be initialized before the
 *      call.
 *
 *      Returns the new total no. of allocated bytes.
 *
 *@@added V0.9.7 (2001-01-07) [umoeller]
 *@@changed V0.9.9 (2001-03-09) [umoeller]: now using ulDelta
 *@@changed V0.9.12 (2001-05-21) [umoeller]: now reporting error on realloc fail
 */

ULONG xstrReserve(PXSTRING pxstr,
                  ULONG ulBytes)
{
    ULONG   cbNeeded = ulBytes;

    if (cbNeeded > pxstr->cbAllocated)
    {
        // we need more memory than we have previously
        // allocated:
        ULONG cbAllocate;
        if (pxstr->ulDelta)
        {
            // delta specified: allocate in chunks of that
            // V0.9.9 (2001-03-07) [umoeller]
            ULONG cbExtra = cbNeeded - pxstr->cbAllocated;
            cbExtra = (   (cbExtra + pxstr->ulDelta)
                        / pxstr->ulDelta
                      )
                      * pxstr->ulDelta;
                    // if we need 3 extra bytes and ulDelta is 10,
                    // this gives us 10 extra bytes
                    // if we need 3 extra bytes and ulDelta is 1000,
                    // this gives us 1000 extra bytes
            cbAllocate = pxstr->cbAllocated + cbExtra;
        }
        else
            // no delta specified:
            cbAllocate = cbNeeded;
        // V0.9.9 (2001-03-05) [umoeller]: use realloc;
        // this gives the C runtime a chance to expand the
        // existing block
#ifdef __DEBUG_MALLOC_ENABLED__
        if (pxstr->psz = (PSZ)memdRealloc(pxstr->psz,
                                          cbAllocate,
                                          pxstr->file,
                                          pxstr->line,
                                          pxstr->function))
#else
        if (pxstr->psz = (PSZ)realloc(pxstr->psz,
                                      cbAllocate))
#endif
                    // if pxstr->psz is NULL, realloc behaves like malloc
            pxstr->cbAllocated = cbAllocate;
                // ulLength is unchanged
        else
            // error: V0.9.12 (2001-05-21) [umoeller]
            pxstr->cbAllocated = 0;
    }
    // else: we have enough memory

    return pxstr->cbAllocated;
}

/*
 *@@ xstrShrink:
 *      reallocates the string buffer so that it
 *      is exactly the length of the string with
 *      its null byte, if the string has excessive
 *      memory allocated. Useful if you are sure
 *      that the string won't grow again.
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 */

void XWPENTRY xstrShrink(PXSTRING pxstr)
{
    if (    (pxstr)
         && (pxstr->psz)
         && (pxstr->cbAllocated > pxstr->ulLength + 1)
       )
    {
        pxstr->psz = (PSZ)realloc(pxstr->psz,
                                  pxstr->ulLength + 1);
        pxstr->cbAllocated = pxstr->ulLength + 1;
    }
}

/*
 *@@ xstrCreate:
 *      allocates a new XSTRING from the heap
 *      and calls xstrInit on it.
 *
 *      Always use xstrFree to free associated
 *      resources.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 */

PXSTRING xstrCreate(ULONG ulPreAllocate)
{
    PXSTRING pxstr;
    if (pxstr = (PXSTRING)malloc(sizeof(XSTRING)))
        xstrInit(pxstr, ulPreAllocate);

    return pxstr;
}

/*
 *@@ xstrFree:
 *      frees the specified heap XSTRING, which must
 *      have been created using xstrCreate.
 *
 *      This uses a pointer to a PXSTRING so that
 *      the pointer is automatically reset to NULL
 *      by this function AND to avoid confusion
 *      with xstrClear.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 *@@changed V0.9.12 (2001-05-24) [umoeller]: changed prototype to use pointer to pointer
 */

VOID xstrFree(PXSTRING *ppxstr)               // in/out: string
{
    PXSTRING p;
    if (    ppxstr
         && (p = *ppxstr)
       )
    {
        xstrClear(p);
        free(p);
        *ppxstr = NULL;
    }
}

/*
 *@@ xstrset2:
 *      sets the specified XSTRING to a new string
 *      without copying it.
 *
 *      pxstr is cleared before the new string is set.
 *
 *      This ONLY works if pszNew has been allocated from
 *      the heap using malloc() or strdup() and is thus
 *      free()'able.
 *
 *      This assumes that exactly strlen(pszNew) + 1
 *      bytes have been allocated for pszNew, which
 *      is true if pszNew comes from strdup().
 *
 *      With this function, you can pass in the
 *      length of the string in ulNewLength.
 *      Otherwise use xstrset.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

ULONG xstrset2(PXSTRING pxstr,              // in/out: string
               PSZ pszNew,                  // in: heap PSZ to use
               ULONG ulNewLength)           // in: length of string or 0 to run strlen here
{
    if (!pxstr)
        return 0;         // V0.9.9 (2001-02-14) [umoeller]

    xstrClear(pxstr);
    if (pxstr->psz = pszNew)
    {
        if (!ulNewLength)
            ulNewLength = strlen(pszNew);
        pxstr->ulLength = ulNewLength;
        pxstr->cbAllocated = ulNewLength + 1;

        pxstr->ulDelta = ulNewLength * 10 / 100;
    }
    // else null string: cbAllocated and ulLength are 0 already

    return pxstr->ulLength;
}

/*
 *@@ xstrset:
 *      shortcut for xstrset2 for retaining compatibility.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 *@@changed V0.9.9 (2001-02-14) [umoeller]: fixed NULL target crash
 */

ULONG xstrset(PXSTRING pxstr,               // in/out: string
              PSZ pszNew)                   // in: heap PSZ to use
{
    return xstrset2(pxstr, pszNew, 0);
}

/*
 *@@ xstrcpy:
 *      copies pcszSource to pxstr, for which memory is allocated
 *      as necessary.
 *
 *      If pxstr contains something, its contents are overwritten.
 *
 *      With ulSourceLength, specify the length of pcszSource
 *      or 0.
 *
 *      --  If you specify 0, this function will run
 *          strlen(pcszSource) and copy the entire source
 *          string.
 *
 *      --  If you already know the length of pcszSource, you
 *          can speed this function up by specifying the
 *          length.
 *
 *      --  You are required to specify ulSourceLength if you
 *          only want to copy a substring, or if pcszSource is
 *          not zero-terminated.
 *
 *      Returns the length of the new string (excluding the null
 *      terminator), or null upon errors.
 *
 *      Example:
 *
 +          XSTRING str;
 +          xstrInit(&str, 0);
 +          xstrcpy(&str, "blah", 0);
 *
 *      This sequence can be abbreviated using xstrInitCopy.
 *
 *      Memory cost: If there's enough room in pxstr for
 *      pcszSource, none. Otherwise pxstr is reallocated
 *      to hold enough room for pcszSource.
 *
 *@@changed V0.9.2 (2000-04-01) [umoeller]: renamed from strhxcpy
 *@@changed V0.9.6 (2000-11-01) [umoeller]: rewritten
 *@@changed V0.9.7 (2001-01-15) [umoeller]: added ulSourceLength
 *@@changed V0.9.9 (2001-01-28) [lafaix]: fixed memory leak and NULL source behavior
 *@@changed V0.9.9 (2001-02-14) [umoeller]: fixed NULL target crash
 *@@changed V0.9.9 (2001-02-16) [umoeller]: now supporting non-zero-terminated pcszSource
 *@@changed V0.9.9 (2001-03-09) [umoeller]: now using xstrReserve
 *@@changed V0.9.12 (2001-05-21) [umoeller]: added xstrReserve error checking
 */

ULONG XWPENTRY xstrcpy(PXSTRING pxstr,               // in/out: string
              PCSZ pcszSource,              // in: source, can be NULL
              ULONG ulSourceLength)         // in: length of pcszSource or 0
{
    if (!pxstr)
        return 0;         // V0.9.9 (2001-02-14) [umoeller]

    if (pcszSource)
    {
        // source specified:
        if (ulSourceLength == 0)
            // but not length:
            ulSourceLength = strlen(pcszSource);
    }
    else
        ulSourceLength = 0;

    if (ulSourceLength)
    {
        // we do have a source string:
        if (xstrReserve(pxstr,
                        // required memory:
                        ulSourceLength + 1))
        {
            memcpy(pxstr->psz,
                   pcszSource,
                   ulSourceLength);
            pxstr->psz[ulSourceLength] = '\0';
                    // V0.9.9 (2001-02-16) [umoeller]
                    // we must do this or otherwise we require pcszSource
                    // to be zero-terminated... not a good idea
        }
        else
            pxstr->ulLength = 0;        // error V0.9.12 (2001-05-21) [umoeller]
    }
    else
    {
        // no source specified or source is empty:
        if (pxstr->cbAllocated)
            // we did have a string: set to empty,
            // but leave allocated memory intact
            *(pxstr->psz) = 0;
        // else
            // we had no string previously: in that case
            // psz and ulLength and cbAllocated are all still NULL
    }

    // in all cases, set new length
    pxstr->ulLength = ulSourceLength;

    return pxstr->ulLength;
}

/*
 *@@ xstrcpys:
 *      shortcut to xstrcpy if the source is an XSTRING also.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

ULONG xstrcpys(PXSTRING pxstr,
               const XSTRING *pcstrSource)
{
    if (!pcstrSource)
        return 0;

    return xstrcpy(pxstr, pcstrSource->psz, pcstrSource->ulLength);
}

/*
 *@@ xstrcat:
 *      appends pcszSource to pxstr, for which memory is
 *      reallocated if necessary.
 *
 *      If pxstr is empty, this behaves just like xstrcpy.
 *
 *      With ulSourceLength, specify the length of pcszSource
 *      or 0 (see xstrcpy for details).
 *
 *      Returns the length of the new string (excluding the null
 *      terminator) if the string was changed, or 0 if nothing
 *      happened.
 *
 *      Note: To append a single character, xstrcatc is faster
 *      than xstrcat.
 *
 *      Example:
 *
 +          XSTRING str;
 +          xstrInit(&str, 0);
 +          xstrcpy(&str, "blah", 0);
 +          xstrcat(&str, "blup", 0);
 *
 *      After this, str.psz points to a new string containing
 *      "blahblup".
 *
 *      Memory cost: If there's enough room in pxstr for
 *      pcszSource, none. Otherwise pxstr is reallocated
 *      to hold enough room for pcszSource.
 *
 *@@changed V0.9.1 (99-12-20) [umoeller]: fixed memory leak
 *@@changed V0.9.1 (2000-01-03) [umoeller]: crashed if pszString was null; fixed
 *@@changed V0.9.2 (2000-04-01) [umoeller]: renamed from strhxcat
 *@@changed V0.9.3 (2000-05-11) [umoeller]: returned 0 if pszString was initially empty; fixed
 *@@changed V0.9.6 (2000-11-01) [umoeller]: rewritten
 *@@changed V0.9.7 (2000-12-10) [umoeller]: return value was wrong
 *@@changed V0.9.7 (2001-01-15) [umoeller]: added ulSourceLength
 *@@changed V0.9.9 (2001-02-16) [umoeller]: now supporting non-zero-terminated pcszSource
 *@@changed V0.9.9 (2001-03-09) [umoeller]: now using xstrReserve
 */

ULONG XWPENTRY xstrcat(PXSTRING pxstr,               // in/out: string
              PCSZ pcszSource,       // in: source, can be NULL
              ULONG ulSourceLength)         // in: length of pcszSource or 0
{
    ULONG   ulrc = 0;

    if (pxstr)
    {
        if (pcszSource)
        {
            if (ulSourceLength == 0)
                ulSourceLength = strlen(pcszSource);

            if (ulSourceLength)
            {
                // we do have a source string:

                // 1) memory management
                xstrReserve(pxstr,
                            // required memory:
                            pxstr->ulLength + ulSourceLength + 1);

                // 2) append source string:
                memcpy(pxstr->psz + pxstr->ulLength,
                       pcszSource,
                       ulSourceLength);

                pxstr->psz[pxstr->ulLength + ulSourceLength] = '\0';
                        // V0.9.9 (2001-02-16) [umoeller]
                        // we must do this or otherwise we require pcszSource
                        // to be zero-terminated... not a good idea

                // in all cases, set new length
                pxstr->ulLength += ulSourceLength;
                ulrc = pxstr->ulLength;     // V0.9.7 (2000-12-10) [umoeller]

            } // end if (ulSourceLength)
        }

        // else no source specified or source is empty:
        // do nothing
    }

    return ulrc;
}

/*
 *@@ xstrcatc:
 *      this is similar to xstrcat, except that this is
 *      for a single character. This is a bit faster than
 *      xstrcat.
 *
 *      If "c" is \0, nothing happens.
 *
 *      If pxstr is empty, this behaves just like xstrcpy.
 *
 *      Returns the length of the new string (excluding the null
 *      terminator) if the string was changed, or 0 if nothing
 *      happened.
 *
 *      Example:
 *
 +          XSTRING str;
 +          xstrInit(&str, 0);
 +          xstrcpy(&str, "blu", 0);
 +          xstrcatc(&str, 'p');
 *
 *      After this, str.psz points to a new string containing
 *      "blup".
 *
 *      Memory cost: If there's enough room in pxstr for
 *      c, none. Otherwise pxstr is reallocated
 *      to hold enough room for c.
 *
 *@@added V0.9.7 (2000-12-10) [umoeller]
 *@@changed V0.9.9 (2001-03-09) [umoeller]: now using xstrReserve
 */

ULONG xstrcatc(PXSTRING pxstr,     // in/out: string
               CHAR c)             // in: character to append, can be \0
{
    ULONG   ulrc = 0;

    if ((pxstr) && (c))
    {
        // 1) memory management
        xstrReserve(pxstr,
                    // required memory:
                    pxstr->ulLength  // existing length, without null terminator
                            + 1      // new character
                            + 1);    // null terminator
        // 2) append character:
        pxstr->psz[pxstr->ulLength] = c;
        pxstr->psz[pxstr->ulLength + 1] = '\0';

        // in all cases, set new length
        (pxstr->ulLength)++;
        ulrc = pxstr->ulLength;

    } // end if ((pxstr) && (c))

    return ulrc;
}

/*
 *@@ xstrcats:
 *      shortcut to xstrcat if the source is an XSTRING also.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

ULONG xstrcats(PXSTRING pxstr,
               const XSTRING *pcstrSource)
{
    if (!pcstrSource)
        return 0;

    return xstrcat(pxstr,
                   pcstrSource->psz,
                   pcstrSource->ulLength);
}

/*
 *@@ xstrrpl:
 *      replaces "cReplLen" characters in pxstr, starting
 *      at the position "ulFirstReplPos", with the first
 *      "cReplaceWithLen" characters from pcszReplaceWith.
 *
 *      If cReplaceWithLen is 0, characters are removed only.
 *
 *      Returns the new length of the string, excluding
 *      the null terminator, or 0 if the replacement failed
 *      (e.g. because the offsets were too large).
 *
 *      This has been extracted from xstrFindReplace because
 *      if you already know the position of a substring,
 *      you can now call this directly. This properly
 *      reallocates the string if more memory is needed.
 *
 *      Example:
 *
 +          XSTRING xstr, xstrReplacement;
 +          xstrInitCopy(&xstr, "This is a test string.");
 +          //  positions:       0123456789012345678901
 +          //                             1         2
 +
 +          xstrrpl(&xstr,
 +                  10,     // position of "test"
 +                  4,      // length of "test"
 +                  "stupid",
 +                  6);     // strlen("stupid")
 *
 *      This would yield "This is a stupid string."
 *
 *      Memory cost: If there's enough room in pxstr for
 *      the replacement, none. Otherwise pxstr is reallocated
 *      to hold enough room for the replacement.
 *
 *@@added V0.9.7 (2001-01-15) [umoeller]
 *@@changed V0.9.9 (2001-01-29) [lafaix]: fixed unnecessary allocation when pxstr was big enough
 *@@changed V0.9.9 (2001-02-14) [umoeller]: fixed NULL target crash
 *@@changed V0.9.9 (2001-03-09) [umoeller]: now using xstrReserve
 *@@changed V0.9.11 (2001-04-22) [umoeller]: replaced replacement XSTRING with PCSZ
 *@@changed V0.9.14 (2001-07-07) [umoeller]: this did nothing if cReplaceWithLen == 0, fixed
 */

ULONG xstrrpl(PXSTRING pxstr,                   // in/out: string
              ULONG ulFirstReplOfs,             // in: ofs of first char to replace
              ULONG cReplLen,                   // in: no. of chars to replace
              PCSZ pcszReplaceWith,             // in: string to replace chars with
              ULONG cReplaceWithLen)            // in: length of replacement string
                                                // (this MUST be specified; if 0, chars are removed only)
{
    ULONG   ulrc = 0;

    // security checks...
    if (    (pxstr)         // V0.9.9 (2001-02-14) [umoeller]
         && (ulFirstReplOfs + cReplLen <= pxstr->ulLength)
         && (    (pcszReplaceWith)
              || (cReplaceWithLen == 0)     // fixed V0.9.14 (2001-07-07) [umoeller]
            )
       )
    {
        // size of new buffer:
        ULONG   cbNeeded = pxstr->ulLength      // existing
                         + cReplaceWithLen      // plus replacement string length
                         - cReplLen             // minus replaced characters
                         + 1;                   // plus null terminator
        // offset where pszSearch was found
        PSZ     pFound = pxstr->psz + ulFirstReplOfs;

        // now check if we have enough memory...
        if (cbNeeded > pxstr->cbAllocated)
        {
            // we need more memory than we have previously
            // allocated:
            // reallocate using ulDelta V0.9.9 (2001-03-07) [umoeller]
            ULONG cbAllocate;
            PSZ pszNew;
            if (pxstr->ulDelta)
            {
                // delta specified: allocate in chunks of that
                // V0.9.9 (2001-03-07) [umoeller]
                ULONG cbExtra = cbNeeded - pxstr->cbAllocated;
                cbExtra = (   (cbExtra + pxstr->ulDelta)
                            / pxstr->ulDelta
                          )
                          * pxstr->ulDelta;
                        // if we need 3 extra bytes and ulDelta is 10,
                        // this gives us 10 extra bytes
                        // if we need 3 extra bytes and ulDelta is 1000,
                        // this gives us 1000 extra bytes
                cbAllocate = pxstr->cbAllocated + cbExtra;
            }
            else
                // no delta specified:
                cbAllocate = cbNeeded;
            // allocate new buffer
            pszNew = (PSZ)malloc(cbAllocate);
            // end V0.9.9 (2001-03-07) [umoeller]

            if (ulFirstReplOfs)
                // "found" was not at the beginning:
                // copy from beginning up to found-offset
                memcpy(pszNew,
                       pxstr->psz,
                       ulFirstReplOfs);     // up to "found"

            if (cReplaceWithLen)
            {
                // we have a replacement:
                // insert it next...
                // we no longer can be sure that pcszReplaceWith
                // is null terminated, so terminate explicitly
                // V0.9.11 (2001-04-22) [umoeller]
                memcpy(pszNew + ulFirstReplOfs,
                       pcszReplaceWith,
                       cReplaceWithLen);
                *(pszNew + ulFirstReplOfs + cReplaceWithLen) = '\0';
            }

            // copy rest:
            // pxstr      frontFOUNDtail
            //            0         1
            //            01234567890123
            //            ³    ³    ³  ³
            //            ³    ³    ÀÄ ulFirstReplOfs + cReplLen = 10
            //            ³    ³       ³
            //            ³    ÀÄ ulFirstReplOfs = 5
            //            ³            ³
            //            pxstr->ulLength = 14
            memcpy(pszNew + ulFirstReplOfs + cReplaceWithLen,
                   pFound + cReplLen,
                   // remaining bytes:
                   pxstr->ulLength - ulFirstReplOfs - cReplLen // 9
                        + 1); // null terminator

            // replace old buffer with new one
            free(pxstr->psz);
            pxstr->psz = pszNew;
            pxstr->ulLength = cbNeeded - 1;
            pxstr->cbAllocated = cbAllocate; // V0.9.9 (2001-03-07) [umoeller]
        } // end if (pxstr->cbAllocated < cbNeeded)
        else
        {
            // we have enough memory left,
            // we can just overwrite in the middle...
            // fixed V0.9.9 (2001-01-29) [lafaix]

            // calc length of string after "found"
            ULONG   cTailLength = pxstr->ulLength - ulFirstReplOfs - cReplLen;

            // first, we move the end to its new location
            // (memmove handles overlap if needed)
            memmove(pFound + cReplaceWithLen,
                    pFound + cReplLen,
                    cTailLength + 1); // including null terminator

            // now overwrite "found" in the middle
            if (cReplaceWithLen)
                memcpy(pFound,
                       pcszReplaceWith,
                       cReplaceWithLen);        // no null terminator

            // that's it; adjust the string length now
            pxstr->ulLength = cbNeeded - 1;
        }

        ulrc = cbNeeded - 1;
    } // end checks

    return ulrc;
}

/*
 *@@ xstrFindWord:
 *      searches for pstrFind in pxstr, starting at ulOfs.
 *      However, this only finds pstrFind if it's a "word",
 *      i.e. surrounded by one of the characters in the
 *      pcszBeginChars and pcszEndChars array.
 *
 *      This is similar to strhFindWord, but this uses
 *      strhmemfind for fast searching, and it doesn't
 *      have to calculate the string lengths because these
 *      already in XSTRING.
 *
 *      Returns 0 if no "word" was found, or the offset
 *      of the "word" in pxstr if found.
 *
 *@@added V0.9.6 (2000-11-12) [umoeller]
 *@@changed V0.9.9 (2001-02-14) [umoeller]: fixed NULL string crashs
 */

PSZ xstrFindWord(const XSTRING *pxstr,        // in: buffer to search ("haystack")
                 ULONG ulOfs,                 // in: where to begin search (0 = start)
                 const XSTRING *pstrFind,     // in: word to find ("needle")
                 size_t *pShiftTable,         // in: shift table (see strhmemfind)
                 PBOOL pfRepeatFind,          // in: repeat find? (see strhmemfind)
                 PCSZ pcszBeginChars,  // suggestion: "\x0d\x0a ()/\\-,."
                 PCSZ pcszEndChars)    // suggestion: "\x0d\x0a ()/\\-,.:;"
{
    PSZ     pReturn = 0;

    if (pxstr && pstrFind)      // V0.9.9 (2001-02-14) [umoeller]
    {
        ULONG   ulFoundLen = pstrFind->ulLength;

        if ((pxstr->ulLength) && (ulFoundLen))
        {
            PCSZ p = pxstr->psz + ulOfs;

            do  // while p
            {
                if (p = (PSZ)strhmemfind(p,         // in: haystack
                                         pxstr->ulLength - (p - pxstr->psz),
                                                    // remaining length of haystack
                                         pstrFind->psz,
                                         ulFoundLen,
                                         pShiftTable,
                                         pfRepeatFind))
                {
                    // string found:
                    // check if that's a word

                    if (strhIsWord(pxstr->psz,
                                   p,
                                   ulFoundLen,
                                   pcszBeginChars,
                                   pcszEndChars))
                    {
                        // valid end char:
                        pReturn = (PSZ)p;
                        break;
                    }

                    p += ulFoundLen;
                }
            } while (p);

        }
    }

    return pReturn;
}

/*
 *@@ xstrFindReplace:
 *      replaces the first occurence of pstrSearch with
 *      pstrReplace in pxstr.
 *
 *      Starting with V0.9.6, this operates entirely on
 *      XSTRING's for speed because we then know the string
 *      lengths already and can use memcpy instead of strcpy.
 *      This new version should be magnitudes faster,
 *      especially with large string bufffers.
 *
 *      pxstr and pstrReplace may not be NULL, but if
 *      pstrReplace is null or empty, this effectively
 *      erases pstrSearch in pxstr.
 *
 *      Returns the length of the new string (exclusing the
 *      null terminator) or 0 if pszSearch was not found
 *      (and pxstr was therefore not changed).
 *
 *      This starts the search at *pulOfs. If
 *      (*pulOfs == 0), this starts from the beginning
 *      of pxstr.
 *
 *      If the string was found, *pulOfs will be set to the
 *      first character after the new replacement string. This
 *      allows you to call this func again with the same strings
 *      to have several occurences replaced (see the example below).
 *
 *      There are two wrappers around this function which
 *      work on C strings instead (however, thus losing the
 *      speed advantage):
 *
 *      -- strhFindReplace operates on C strings only;
 *
 *      -- xstrFindReplaceC uses C strings for the search and replace
 *         parameters.
 *
 *      <B>Example usage:</B>
 *
 +          XSTRING strBuf,
 +                  strFind,
 +                  strRepl;
 +          size_t  ShiftTable[256];
 +          BOOL    fRepeat = FALSE;
 +          ULONG   ulOffset = 0;
 +
 +          xstrInitCopy(&strBuf, "Test phrase 1. Test phrase 2.", 0);
 +          xstrInitSet(&strFind, "Test");
 +          xstrInitSet(&strRepl, "Dummy");
 +          while (xstrFindReplace(&str,
 +                                 &ulPos,      // in/out: offset
 +                                 &strFind,    // search
 +                                 &strRepl,    // replace
 +                                 ShiftTable,
 +                                 &fRepeat))
 +              ;
 *
 *      would replace all occurences of "Test" in str with
 *      "Dummy".
 *
 *      Memory cost: Calls xstrrpl if pstrSearch was found.
 *
 *@@changed V0.9.0 [umoeller]: totally rewritten.
 *@@changed V0.9.0 (99-11-08) [umoeller]: crashed if *ppszBuf was NULL. Fixed.
 *@@changed V0.9.2 (2000-04-01) [umoeller]: renamed from strhxrpl
 *@@changed V0.9.6 (2000-11-01) [umoeller]: rewritten
 *@@changed V0.9.6 (2000-11-12) [umoeller]: now using strhmemfind
 *@@changed V0.9.7 (2001-01-15) [umoeller]: renamed from xstrrpl; extracted new xstrrpl
 *@@changed V1.0.1 (2003-02-02) [umoeller]: now allowing for NULL pstrReplace
 */

ULONG xstrFindReplace(PXSTRING pxstr,               // in/out: string
                      PULONG pulOfs,                // in: where to begin search (0 = start);
                                                    // out: ofs of first char after replacement string
                      const XSTRING *pstrSearch,    // in: search string; cannot be NULL
                      const XSTRING *pstrReplace,   // in: replacement string or NULL
                      size_t *pShiftTable,          // in: shift table (see strhmemfind)
                      PBOOL pfRepeatFind)           // in: repeat find? (see strhmemfind)
{
    ULONG    ulrc = 0;      // default: not found

    if ((pxstr) && (pstrSearch))
    {
        ULONG   cSearchLen = pstrSearch->ulLength;

        // can we search this?
        if (    (*pulOfs < pxstr->ulLength)
             && (cSearchLen)
           )
        {
            // yes:
            ULONG   ulOfs = *pulOfs;
            PCSZ pFound;
            if (pFound = (PCSZ)strhmemfind(pxstr->psz + ulOfs, // in: haystack
                                           pxstr->ulLength - ulOfs,
                                           pstrSearch->psz,
                                           cSearchLen,
                                           pShiftTable,
                                           pfRepeatFind))
            {
                ULONG ulFirstReplOfs = pFound - pxstr->psz;
                ULONG   lenRepl = pstrReplace ? (pstrReplace->ulLength) : 0;
                // found in buffer from ofs:
                // replace pFound with pstrReplace
                ulrc = xstrrpl(pxstr,
                               ulFirstReplOfs,              // where to start
                               cSearchLen,                  // chars to replace
                               pstrReplace ? (pstrReplace->psz) : NULL,
                               lenRepl);      // adjusted V0.9.11 (2001-04-22) [umoeller]

                // return new length
                *pulOfs = ulFirstReplOfs + lenRepl;
            } // end if (pFound)
        } // end if (    (*pulOfs < pxstr->ulLength) ...
    } // end if ((pxstr) && (pstrSearch) && (pstrReplace))

    return ulrc;
}

/*
 *@@ xstrFindReplaceC:
 *      wrapper around xstrFindReplace() which allows using
 *      C strings for the find and replace parameters.
 *
 *      This creates two temporary XSTRING's for pcszSearch
 *      and pcszReplace and thus cannot use the shift table
 *      for repetitive searches. As a result, this is slower
 *      than xstrFindReplace.
 *
 *      If you search with the same strings several times,
 *      you'll be better off using xstrFindReplace() directly.
 *
 *@@added V0.9.6 (2000-11-01) [umoeller]
 *@@changed V0.9.7 (2001-01-15) [umoeller]: renamed from xstrcrpl
 */

ULONG xstrFindReplaceC(PXSTRING pxstr,              // in/out: string
                       PULONG pulOfs,               // in: where to begin search (0 = start);
                                                    // out: ofs of first char after replacement string
                       PCSZ pcszSearch,      // in: search string; cannot be NULL
                       PCSZ pcszReplace)     // in: replacement string; cannot be NULL
{
    XSTRING xstrFind,
            xstrReplace;
    size_t  ShiftTable[256];
    BOOL    fRepeat = FALSE;
    // initialize find/replace strings... note that the
    // C strings are not free()'able, so we MUST NOT use xstrClear
    // before leaving
    xstrInitSet(&xstrFind, (PSZ)pcszSearch);
    xstrInitSet(&xstrReplace, (PSZ)pcszReplace);

    return xstrFindReplace(pxstr, pulOfs, &xstrFind, &xstrReplace, ShiftTable, &fRepeat);
}

// static encoding table for xstrEncode
static PSZ apszEncoding[] =
{
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2A", "%2B", "%2C", "%2D", "%2E", "%2F",
    "%30", "%31", "%32", "%33", "%34", "%35", "%36", "%37",
    "%38", "%39", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "%41", "%42", "%43", "%44", "%45", "%46", "%47",
    "%48", "%49", "%4A", "%4B", "%4C", "%4D", "%4E", "%4F",
    "%50", "%51", "%52", "%53", "%54", "%55", "%56", "%57",
    "%58", "%59", "%5A", "%5B", "%5C", "%5D", "%5E", "%5F",
    "%60", "%61", "%62", "%63", "%64", "%65", "%66", "%67",
    "%68", "%69", "%6A", "%6B", "%6C", "%6D", "%6E", "%6F",
    "%70", "%71", "%72", "%73", "%74", "%75", "%76", "%77",
    "%78", "%79", "%7A", "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

/*
 *@@ xstrEncode:
 *      encodes characters in a string.
 *
 *      This searches pxstr for all occurences of the
 *      characters in pcszEncode (which must be a
 *      null-terminated list of characters to be
 *      encoded). Each occurence that is found is
 *      replaced with "%hh", with "hh" being the
 *      two-digit hex number of the encoded character.
 *
 *      For example, to encode strings for the XCenter,
 *      set pcszEncode to "%,();=".
 *
 *      Returns the no. of characters replaced.
 *
 *      NOTE: You must make sure that pcszEncode ALWAYS
 *      contains the "%" character as well, which must
 *      always be encoded (i.e. escaped) because it is
 *      used for encoding the characters. Otherwise
 *      you won't be able to decode the string again.
 *
 *      Example: To encode all occurences of
 *      "a", "b", and "c" in a string, do this:
 *
 +          XSTRING str;
 +          xstrInitCopy(&str, "Sample characters.";
 +          xstrEncode(&str, "abc%";
 *
 *      would convert str to contain:
 *
 +          S%61mple %63hara%63ters.
 *
 *      Memory cost: None, except for that of xstrcpy.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 *@@changed V0.9.9 (2001-03-06) [lafaix]: rewritten.
 */

ULONG xstrEncode(PXSTRING pxstr,     // in/out: string to convert
                 PCSZ pcszEncode)    // in: characters to encode (e.g. "%,();=")
{
    ULONG ulrc = 0,
          ul,
          ulEncodeLength;

    if (    (pxstr)
         && (pxstr->ulLength)
         && (pcszEncode)
         && (ulEncodeLength = strlen(pcszEncode)))
    {
        PSZ pszDest = (PSZ)malloc(pxstr->ulLength * 3
                                  + 1),
            pszDestCurr = pszDest;

        if (pszDest)
        {
            for (ul = 0;
                 ul < pxstr->ulLength;
                 ul++)
            {
                ULONG ulEncode;

                for (ulEncode = 0;
                     ulEncode < ulEncodeLength;
                     ulEncode++)
                {
                    if (pxstr->psz[ul] == pcszEncode[ulEncode])
                    {
                        // use the static encoding table for speed
                        memcpy(pszDestCurr,
                               apszEncoding[(UCHAR)pcszEncode[ulEncode]],
                               3);
                        pszDestCurr += 3;
                        ulrc++;
                        goto iterate;
                    }
                }

                *pszDestCurr++ = pxstr->psz[ul];

                iterate:
                    ;
            }
        }

        // something was encoded; update pxstr
        if (ulrc)
        {
            *pszDestCurr = 0;

            xstrcpy(pxstr, pszDest, pszDestCurr - pszDest);
        }

        free(pszDest);
    }

    return ulrc;
}

/*
 *@@ xstrEncodeASCII:
 *      like xstrEncode, but instead of encoding characters
 *      from an array given by the caller, this encodes all
 *      non-ASCII characters (i.e. >= 128) plus the '%' char.
 *
 *@@added V1.0.2 (2003-02-07) [umoeller]
 */

ULONG xstrEncodeASCII(PXSTRING pxstr)     // in/out: string to convert
{
    ULONG ulrc = 0,
          ul,
          ulEncodeLength;

    if (    (pxstr)
         && (pxstr->ulLength)
       )
    {
        PSZ pszDest = (PSZ)malloc(pxstr->ulLength * 3
                                  + 1),
            pszDestCurr = pszDest;

        if (pszDest)
        {
            for (ul = 0;
                 ul < pxstr->ulLength;
                 ul++)
            {
                if (    ((UCHAR)pxstr->psz[ul] >= 128)
                     || (pxstr->psz[ul] == '%')
                   )
                {
                    memcpy(pszDestCurr,
                           apszEncoding[(UCHAR)pxstr->psz[ul]],
                           3);
                    pszDestCurr += 3;
                    ulrc++;
                    goto iterate;
                }

                *pszDestCurr++ = pxstr->psz[ul];

                iterate:
                    ;
            }
        }

        // something was encoded; update pxstr
        if (ulrc)
        {
            *pszDestCurr = 0;

            xstrcpy(pxstr, pszDest, pszDestCurr - pszDest);
        }

        free(pszDest);
    }

    return ulrc;
}

/*
 *@@ xstrDecode:
 *      decodes a string previously encoded by xstrEncode.
 *
 *      This simply assumes that all '%' characters in
 *      pxstr introduce encodings and the next two characters
 *      after '%' always are a hex character code. This
 *      only recognizes hex in upper case. All this will
 *      work properly with encodings from xstrEncode.
 *
 *      Returns the no. of encodings replaced.
 *
 *      Memory cost: None.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 *@@changed V0.9.9 (2001-03-06) [lafaix]: removed memory allocation
 *@@changed V0.9.16 (2002-02-02) [umoeller]: added cKey
 */

ULONG xstrDecode2(PXSTRING pxstr,       // in/out: string to be decoded
                  CHAR cKey)            // in: encoding key (normally '%')
{
    ULONG   ulrc = 0;

    if (    (pxstr)
         && (pxstr->ulLength)
       )
    {
        const char  *pSource = pxstr->psz;
        PSZ         pszDest  = (PSZ)pSource,
                    pDest    = (PSZ)pSource;
        CHAR        c;

        while ((c = *pSource++))
        {
            // pSource points to next char now

            if (c == cKey)
            {
                static char ach[] = "0123456789ABCDEF";

                // convert two chars after '%'
                CHAR        c2,         // first char after '%'     --> hi-nibble
                            c3;         // second char after '%'    --> lo-nibble
                const char  *p2,        // for first char: points into ach or is NULL
                            *p3;        // for second char: points into ach or is NULL
                if (    (c2 = *pSource)
                     && (p2 = strchr(ach, c2))
                     && (c3 = *(pSource + 1))
                     && (p3 = strchr(ach, c3))
                   )
                {
                    // both chars after '%' were valid:
                    *pDest++ =    // lo-nibble:
                                  (p3 - ach) // 0 for '0', 10 for 'A', ...
                                  // hi-nibble:
                                + ((p2 - ach) << 4);
                    // go on after that
                    pSource += 2;
                    // raise return count
                    ulrc++;
                    // next in loop
                    continue;
                }
            }

            // not encoding, or null after '%', or invalid encoding:
            // just leave thisalone
            *pDest++ = c;
        } // while ((ch = *pSource++))

        if (ulrc)
        {
            *pDest = 0;
            pxstr->ulLength = (pDest - pszDest);
        }
    }

    return ulrc;
}

/*
 *@@ xstrDecode:
 *      added for compatibility with exports.
 *
 *@@added V0.9.16 (2002-02-02) [umoeller]
 */

ULONG xstrDecode(PXSTRING pxstr)
{
    return xstrDecode2(pxstr, '%');
}

/*
 *@@ xstrConvertLineFormat:
 *      converts between line formats.
 *
 *      If (fToCFormat == CRLF2LF), all \r\n pairs are replaced
 *      with \n chars (UNIX or C format).
 *
 *      Reversely, if (fToCFormat == LF2CRLF), all \n chars
 *      are converted to \r\n pairs (DOS and OS/2 formats).
 *      No check is made whether this has already been done.
 *
 *@@added V0.9.7 (2001-01-15) [umoeller]
 */

VOID xstrConvertLineFormat(PXSTRING pxstr,
                           BOOL fToCFormat) // in: if CRLF2LF, to C format; if LF2CRLF, to OS/2 format.
{
    XSTRING     strFind,
                strRepl;
    size_t      ShiftTable[256];
    BOOL        fRepeat = FALSE;
    ULONG       ulOfs = 0;

    if (fToCFormat)
    {
        // OS/2 to C:
        xstrInitSet(&strFind, "\r\n");
        xstrInitSet(&strRepl, "\n");
    }
    else
    {
        // C to OS/2:
        xstrInitSet(&strFind, "\n");
        xstrInitSet(&strRepl, "\r\n");
    }

    while (xstrFindReplace(pxstr,
                           &ulOfs,
                           &strFind,
                           &strRepl,
                           ShiftTable,
                           &fRepeat))
            ;
}

/*
 *@@ xstrPrintf:
 *      like sprintf, but prints into an XSTRING
 *      bufer (which must be initialized).
 *
 *      Note that the internal stack buffer is
 *      limited to 2000 bytes, so watch out.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

VOID xstrPrintf(XSTRING *pstr,       // in/out: string buffer (must be init'ed)
                PCSZ pcszFormat,     // in: format string (like with printf)
                ...)                 // in: additional stuff (like with printf)
{
    va_list     args;
    CHAR        szBuf[2000];

    va_start(args, pcszFormat);
    vsprintf(szBuf, pcszFormat, args);
    va_end(args);

    xstrcpy(pstr, szBuf, 0);
}

/*
 *@@ xstrCatf:
 *      like xstrPrintf, but appends to the
 *      given XSTRING.
 *
 *@@added V0.9.19 (2002-04-14) [umoeller]
 */

VOID xstrCatf(XSTRING *pstr,       // in/out: string buffer (must be init'ed)
              PCSZ pcszFormat,     // in: format string (like with printf)
              ...)                 // in: additional stuff (like with printf)
{
    va_list     args;
    CHAR        szBuf[2000];

    va_start(args, pcszFormat);
    vsprintf(szBuf, pcszFormat, args);
    va_end(args);

    xstrcat(pstr, szBuf, 0);
}

// test case

/* int main(void)
{
    XSTRING str,
            strFind,
            strReplace;
    size_t  shift[256];
    BOOL    fRepeat = FALSE;
    ULONG   ulOfs = 0;

    xstrInit(&str, 0);
    xstrInit(&strFind, 0);
    xstrInit(&strReplace, 0);

    str.ulDelta = 50;

    xstrcpy(&str, "Test string 1. Test string 2. Test string 3. !", 0);
    xstrcpy(&strFind, "Test", 0);
    xstrcpy(&strReplace, "Dummy", 0);

    printf("Old string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    printf("Replacing \"%s\" with \"%s\".\n", strFind.psz, strReplace.psz);

    fRepeat = FALSE;
    ulOfs = 0;
    while (xstrFindReplace(&str,
                           &ulOfs,
                           &strFind,
                           &strReplace,
                           shift, &fRepeat));
        ;

    printf("New string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    printf("Appending \"blah\".\n");
    xstrcat(&str, "blah", 0);
    printf("New string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    xstrcpy(&strFind, strReplace.psz, 0);
    xstrClear(&strReplace);

    printf("Replacing \"%s\" with \"%s\".\n", strFind.psz, strReplace.psz);

    fRepeat = FALSE;
    ulOfs = 0;
    while (xstrFindReplace(&str,
                   &ulOfs,
                   &strFind,
                   &strReplace,
                   shift, &fRepeat));
        ;

    printf("New string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    xstrcpy(&strFind, " ", 0);
    xstrcpy(&strReplace, ".", 0);

    printf("Replacing \"%s\" with \"%s\".\n", strFind.psz, strReplace.psz);

    fRepeat = FALSE;
    ulOfs = 0;
    while (xstrFindReplace(&str,
                   &ulOfs,
                   &strFind,
                   &strReplace,
                   shift, &fRepeat));
        ;

    printf("New string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    xstrcpy(&strFind, ".", 0);
    xstrcpy(&strReplace, "*.........................*", 0);

    printf("Replacing \"%s\" with \"%s\".\n", strFind.psz, strReplace.psz);

    fRepeat = FALSE;
    ulOfs = 0;
    while (xstrFindReplace(&str,
                   &ulOfs,
                   &strFind,
                   &strReplace,
                   shift, &fRepeat));
        ;

    printf("New string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    xstrcpy(&strFind, "..........", 0);
    xstrcpy(&strReplace, "@", 0);

    printf("Replacing \"%s\" with \"%s\".\n", strFind.psz, strReplace.psz);

    fRepeat = FALSE;
    ulOfs = 0;
    while (xstrFindReplace(&str,
                   &ulOfs,
                   &strFind,
                   &strReplace,
                   shift, &fRepeat));
        ;

    printf("New string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    printf("Encoding @* chars.\n");
    xstrEncode(&str, "@*");
    printf("New string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    printf("Decoding @* chars.\n");
    xstrDecode(&str);
    printf("New string is: \"%s\" (%d/%d/%d)\n", str.psz, str.ulLength, str.cbAllocated, str.ulDelta);

    return 0;
} */


