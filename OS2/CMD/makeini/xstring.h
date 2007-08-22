
/*
 *@@sourcefile xstring.h:
 *      header file for xstring.c. See notes there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_DOSDATETIME
 *@@include #include <os2.h>
 *@@include #include "helpers\xstring.h"
 */

/*
 *      Copyright (C) 1999-2000 Ulrich M”ller.
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

#if __cplusplus
extern "C" {
#endif

#ifndef XSTRING_HEADER_INCLUDED
    #define XSTRING_HEADER_INCLUDED

    #ifndef XWPENTRY
        #error You must define XWPENTRY to contain the standard linkage for the XWPHelpers.
    #endif

    /*
     *@@ XSTRING:
     *
     *@@added V0.9.6 (2000-11-01) [umoeller]
     */

    typedef struct _XSTRING
    {
        PSZ             psz;            // ptr to string or NULL
        ULONG           ulLength;       // length of *psz
        ULONG           cbAllocated;    // memory allocated in *psz
                                        // (>= ulLength + 1)
        ULONG           ulDelta;        // allocation delta (0 = none)
                                        // V0.9.9 (2001-03-07) [umoeller]

        // if memory debugging is enabled, enable the following
        // extra fields globally... even if the caller doesn't
        // want the replacement calls, the xstr* implementations
        // are compiled with these fields, so they must always be
        // present V0.9.14 (2001-08-01) [umoeller]
        #if defined(__XWPMEMDEBUG__) // setup.h only
            const char      *file;
            unsigned long   line;
            const char      *function;
        #endif

    } XSTRING, *PXSTRING;

    #if defined(__DEBUG_MALLOC_ENABLED__) && !defined(DONT_REPLACE_XSTR_MALLOC) // setup.h, helpers\memdebug.c
        #define xstrInit(a, b) xstrInitDebug((a), (b), __FILE__, __LINE__, __FUNCTION__)
        void XWPENTRY xstrInitDebug(PXSTRING pxstr,
                                    ULONG ulPreAllocate,
                                    const char *file,
                                    unsigned long line,
                                    const char *function);
        typedef void XWPENTRY XSTRINITDEBUG(PXSTRING pxstr,
                                            ULONG ulPreAllocate,
                                            const char *file,
                                            unsigned long line,
                                            const char *function);
        typedef XSTRINITDEBUG *PXSTRINITDEBUG;
    #else
        void XWPENTRY xstrInit(PXSTRING pxstr, ULONG ulPreAllocate);
        typedef void XWPENTRY XSTRINIT(PXSTRING pxstr, ULONG ulPreAllocate);
        typedef XSTRINIT *PXSTRINIT;
    #endif

    /* void XWPENTRY xstrInit(PXSTRING pxstr, ULONG ulPreAllocate);
    typedef void XWPENTRY XSTRINIT(PXSTRING pxstr, ULONG ulPreAllocate);
    typedef XSTRINIT *PXSTRINIT; */

    void xstrInitSet2(PXSTRING pxstr, PSZ pszNew, ULONG ulNewLength);

    void XWPENTRY xstrInitSet(PXSTRING pxstr, PSZ pszNew);
    typedef void XWPENTRY XSTRINITSET(PXSTRING pxstr, PSZ pszNew);
    typedef XSTRINITSET *PXSTRINITSET;

    #if defined(__DEBUG_MALLOC_ENABLED__) && !defined(DONT_REPLACE_XSTR_MALLOC) // setup.h, helpers\memdebug.c
        #define xstrInitCopy(a, b, c) xstrInitCopyDebug((a), (b), (c), __FILE__, __LINE__, __FUNCTION__)
        void XWPENTRY xstrInitCopyDebug(PXSTRING pxstr,
                                        const char *pcszSource,
                                        ULONG ulExtraAllocate,
                                        const char *file,
                                        unsigned long line,
                                        const char *function);
        typedef void XWPENTRY XSTRINITCOPYDEBUG(PXSTRING pxstr,
                                                const char *pcszSource,
                                                ULONG ulExtraAllocate,
                                                const char *file,
                                                unsigned long line,
                                                const char *function);
        typedef XSTRINITCOPYDEBUG *PXSTRINITCOPYDEBUG;
    #else
        void XWPENTRY xstrInitCopy(PXSTRING pxstr, const char *pcszSource, ULONG ulExtraAllocate);
        typedef void XWPENTRY XSTRINITCOPY(PXSTRING pxstr, const char *pcszSource, ULONG ulExtraAllocate);
        typedef XSTRINITCOPY *PXSTRINITCOPY;
    #endif

    void XWPENTRY xstrClear(PXSTRING pxstr);
    typedef void XWPENTRY XSTRCLEAR(PXSTRING pxstr);
    typedef XSTRCLEAR *PXSTRCLEAR;

    ULONG XWPENTRY xstrReserve(PXSTRING pxstr, ULONG ulBytes);
    typedef ULONG XWPENTRY XSTRRESERVE(PXSTRING pxstr, ULONG ulBytes);
    typedef XSTRRESERVE *PXSTRRESERVE;

    void XWPENTRY xstrShrink(PXSTRING pxstr);
    typedef void XWPENTRY XSTRSHRINK(PXSTRING pxstr);
    typedef XSTRSHRINK *PXSTRSHRINK;

    PXSTRING XWPENTRY xstrCreate(ULONG ulPreAllocate);
    typedef PXSTRING XWPENTRY XSTRCREATE(ULONG ulPreAllocate);
    typedef XSTRCREATE *PXSTRCREATE;

    VOID XWPENTRY xstrFree(PXSTRING *ppxstr);
    typedef VOID XWPENTRY XSTRFREE(PXSTRING *ppxstr);
    typedef XSTRFREE *PXSTRFREE;

    ULONG XWPENTRY xstrset2(PXSTRING pxstr, PSZ pszNew, ULONG ulNewLength);

    ULONG XWPENTRY xstrset(PXSTRING pxstr, PSZ pszNew);
    typedef ULONG XWPENTRY XSTRSET(PXSTRING pxstr, PSZ pszNew);
    typedef XSTRSET *PXSTRSET;

    ULONG XWPENTRY xstrcpy(PXSTRING pxstr, const char *pcszSource, ULONG ulSourceLength);
    typedef ULONG XWPENTRY XSTRCPY(PXSTRING pxstr, const char *pcszSource, ULONG ulSourceLength);
    typedef XSTRCPY *PXSTRCPY;

    ULONG XWPENTRY xstrcpys(PXSTRING pxstr, const XSTRING *pcstrSource);
    typedef ULONG XWPENTRY XSTRCPYS(PXSTRING pxstr, const XSTRING *pcstrSource);
    typedef XSTRCPYS *PXSTRCPYS;

    ULONG XWPENTRY xstrcat(PXSTRING pxstr, const char *pcszSource, ULONG ulSourceLength);
    typedef ULONG XWPENTRY XSTRCAT(PXSTRING pxstr, const char *pcszSource, ULONG ulSourceLength);
    typedef XSTRCAT *PXSTRCAT;

    ULONG XWPENTRY xstrcatc(PXSTRING pxstr, CHAR c);
    typedef ULONG XWPENTRY XSTRCATC(PXSTRING pxstr, CHAR c);
    typedef XSTRCATC *PXSTRCATC;

    ULONG XWPENTRY xstrcats(PXSTRING pxstr, const XSTRING *pcstrSource);
    typedef ULONG XWPENTRY XSTRCATS(PXSTRING pxstr, const XSTRING *pcstrSource);
    typedef XSTRCATS *PXSTRCATS;

    /*
     *@@ xstrIsString:
     *      returns TRUE if psz contains something.
     *
     *@@added V0.9.6 (2000-10-16) [umoeller]
     */

    #define xstrIsString(psz) ( (psz != 0) && (*(psz) != 0) )

    ULONG XWPENTRY xstrrpl(PXSTRING pxstr,
                           ULONG ulFirstReplOfs,
                           ULONG cReplLen,
                           const char *pcszReplaceWith,
                           ULONG cReplaceWithLen);
    typedef ULONG XWPENTRY XSTRRPL(PXSTRING pxstr,
                                   ULONG ulFirstReplOfs,
                                   ULONG cReplLen,
                                   const char *pcszReplaceWith,
                                   ULONG cReplaceWithLen);
    typedef XSTRRPL *PXSTRRPL;

    PSZ XWPENTRY xstrFindWord(const XSTRING *pxstr,
                              ULONG ulOfs,
                              const XSTRING *pstrFind,
                              size_t *pShiftTable,
                              PBOOL pfRepeatFind,
                              const char *pcszBeginChars,
                              const char *pcszEndChars);
    typedef PSZ XWPENTRY XSTRFINDWORD(const XSTRING *pxstr,
                                      ULONG ulOfs,
                                      const XSTRING *pstrFind,
                                      size_t *pShiftTable,
                                      PBOOL pfRepeatFind,
                                      const char *pcszBeginChars,
                                      const char *pcszEndChars);
    typedef XSTRFINDWORD *PXSTRFINDWORD;

    ULONG XWPENTRY xstrFindReplace(PXSTRING pxstr,
                                   PULONG pulOfs,
                                   const XSTRING *pstrSearch,
                                   const XSTRING *pstrReplace,
                                   size_t *pShiftTable,
                                   PBOOL pfRepeatFind);
    typedef ULONG XWPENTRY XSTRFINDREPLACE(PXSTRING pxstr,
                                           PULONG pulOfs,
                                           const XSTRING *pstrSearch,
                                           const XSTRING *pstrReplace,
                                           size_t *pShiftTable,
                                           PBOOL pfRepeatFind);
    typedef XSTRFINDREPLACE *PXSTRFINDREPLACE;

    ULONG XWPENTRY xstrFindReplaceC(PXSTRING pxstr,
                                    PULONG pulOfs,
                                    const char *pcszSearch,
                                    const char *pcszReplace);
    typedef ULONG XWPENTRY XSTRFINDREPLACEC(PXSTRING pxstr,
                                            PULONG pulOfs,
                                            const char *pcszSearch,
                                            const char *pcszReplace);
    typedef XSTRFINDREPLACEC *PXSTRFINDREPLACEC;

    ULONG XWPENTRY xstrEncode(PXSTRING pxstr, const char *pcszEncode);
    typedef ULONG XWPENTRY XSTRENCODE(PXSTRING pxstr, const char *pcszEncode);
    typedef XSTRENCODE *PXSTRENCODE;

    ULONG XWPENTRY xstrEncodeASCII(PXSTRING pxstr);

    ULONG XWPENTRY xstrDecode2(PXSTRING pxstr,
                               CHAR cKey);

    ULONG XWPENTRY xstrDecode(PXSTRING pxstr);
    typedef ULONG XWPENTRY XSTRDECODE(PXSTRING pxstr);
    typedef XSTRDECODE *PXSTRDECODE;

    // V0.9.9 (2001-01-29) [lafaix]: constants added
    #define CRLF2LF TRUE
    #define LF2CRLF FALSE

    VOID XWPENTRY xstrConvertLineFormat(PXSTRING pxstr, BOOL fToCFormat);
    typedef VOID XWPENTRY XSTRCONVERTLINEFORMAT(PXSTRING pxstr, BOOL fToCFormat);
    typedef XSTRCONVERTLINEFORMAT *PXSTRCONVERTLINEFORMAT;

    VOID XWPENTRY xstrPrintf(XSTRING *pstr, PCSZ pcszFormat, ...);
    typedef VOID XWPENTRY XSTRPRINTF(XSTRING *pstr, PCSZ pcszFormat, ...);
    typedef XSTRPRINTF *PXSTRPRINTF;

    VOID xstrCatf(XSTRING *pstr,
                  PCSZ pcszFormat,
                  ...);
#endif

#if __cplusplus
}
#endif

