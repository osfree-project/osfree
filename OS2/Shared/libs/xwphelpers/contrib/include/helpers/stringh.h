
/*
 *@@sourcefile stringh.h:
 *      header file for stringh.c. See notes there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_DOSDATETIME
 *@@include #include <os2.h>
 *@@include #include "helpers\stringh.h"
 */

/*
 *      Copyright (C) 1997-2000 Ulrich M”ller.
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

#ifndef STRINGH_HEADER_INCLUDED
    #define STRINGH_HEADER_INCLUDED

    #if defined(__DEBUG_MALLOC_ENABLED__) && !defined(DONT_REPLACE_STRINGH_MALLOC) // setup.h, helpers\memdebug.c
        APIRET XWPENTRY strhStoreDebug(PSZ *ppszTarget,
                                       PCSZ pcszSource,
                                       PULONG pulLength,
                                       const char *pcszSourceFile,
                                       unsigned long ulLine,
                                       const char *pcszFunction);
        #define strhStore(a, b, c) strhStoreDebug((a), (b), (c), __FILE__, __LINE__, __FUNCTION__)
    #else
        APIRET XWPENTRY strhStore(PSZ *ppszTarget, PCSZ pcszSource, PULONG pulLength);
    #endif

    PSZ XWPENTRY strhcpy(PSZ string1, const char *string2);

    APIRET XWPENTRY strhCopyBuf(PSZ pszTarget,
                                PCSZ pcszSource,
                                ULONG cbTarget);

    #if defined(__DEBUG_MALLOC_ENABLED__) && !defined(DONT_REPLACE_STRINGH_MALLOC) // setup.h, helpers\memdebug.c
        PSZ XWPENTRY strhdupDebug(const char *pcszSource,
                                  unsigned long *pulLength,
                                  const char *pcszSourceFile,
                                  unsigned long ulLine,
                                  const char *pcszFunction);
        #define strhdup(a, pul) strhdupDebug((a), (pul), __FILE__, __LINE__, __FUNCTION__)
    #else
        PSZ XWPENTRY strhdup(const char *pcszSource,
                             unsigned long *pulLength);
    #endif

    int XWPENTRY strhcmp(const char *p1, const char *p2);

    int XWPENTRY strhicmp(const char *p1, const char *p2);

    PSZ XWPENTRY strhistr(const char *string1, const char *string2);

    ULONG XWPENTRY strhncpy0(PSZ pszTarget,
                             const char *pszSource,
                             ULONG cbSource);

    size_t XWPENTRY strlcpy(char *dst,
                            const char *src,
                            size_t siz);

    size_t strlcat(char *dst,
                   const char *src,
                   size_t siz);

    ULONG XWPENTRY strhlen(PCSZ pcsz);

    ULONG XWPENTRY strhSize(PCSZ pcsz);

    ULONG XWPENTRY strhCount(const char *pszSearch, CHAR c);

    BOOL XWPENTRY strhIsDecimal(PSZ psz);

    #if defined(__DEBUG_MALLOC_ENABLED__) && !defined(DONT_REPLACE_STRINGH_MALLOC) // setup.h, helpers\memdebug.c
        PSZ XWPENTRY strhSubstrDebug(const char *pBegin,      // in: first char
                                     const char *pEnd,        // in: last char (not included)
                                     const char *pcszSourceFile,
                                     unsigned long ulLine,
                                     const char *pcszFunction);
        #define strhSubstr(a, b) strhSubstrDebug((a), (b), __FILE__, __LINE__, __FUNCTION__)
    #else
        PSZ XWPENTRY strhSubstr(const char *pBegin, const char *pEnd);
    #endif

    PSZ XWPENTRY strhExtract(PCSZ pszBuf,
                             CHAR cOpen,
                             CHAR cClose,
                             PCSZ *ppEnd);

    PSZ XWPENTRY strhQuote(PSZ pszBuf,
                           CHAR cQuote,
                           PSZ *ppEnd);

    ULONG XWPENTRY strhStrip(PSZ psz);

    PSZ XWPENTRY strhins(const char *pcszBuffer,
                         ULONG ulInsertOfs,
                         const char *pcszInsert);

    ULONG XWPENTRY strhFindReplace(PSZ *ppszBuf,
                                   PULONG pulOfs,
                                   const char *pcszSearch,
                                   const char *pcszReplace);

    ULONG XWPENTRY strhWords(PSZ psz);

    #define STRH_BEGIN_CHARS    "\x0d\x0a "
    #define STRH_END_CHARS      "\x0d\x0a /-"

    BOOL XWPENTRY strhGetWord(PSZ *ppszStart,
                              const char *pLimit,
                              const char *pcszBeginChars,
                              const char *pcszEndChars, //  = "\x0d\x0a /-";
                              PSZ *ppszEnd);

    BOOL XWPENTRY strhIsWord(const char *pcszBuf,
                             const char *p,
                             ULONG cbSearch,
                             const char *pcszBeginChars,
                             const char *pcszEndChars);

    PSZ XWPENTRY strhFindWord(const char *pszBuf,
                              const char *pszSearch,
                              const char *pcszBeginChars,
                              const char *pcszEndChars);

    PSZ XWPENTRY strhFindEOL(const char *pcszSearchIn, ULONG *pulOffset);

    PSZ XWPENTRY strhFindNextLine(PSZ pszSearchIn, PULONG pulOffset);

    ULONG XWPENTRY strhBeautifyTitle(PSZ psz);
    typedef ULONG XWPENTRY STRHBEAUTIFYTITLE(PSZ psz);
    typedef STRHBEAUTIFYTITLE *PSTRHBEAUTIFYTITLE;

    ULONG strhBeautifyTitle2(PSZ pszTarget,
                             PCSZ pcszSource);

    BOOL XWPENTRY strhKillChar(PSZ psz,
                               CHAR c,
                               PULONG pulLength);

    PSZ XWPENTRY strhFindAttribValue(const char *pszSearchIn, const char *pszAttrib);

    PSZ XWPENTRY strhGetNumAttribValue(const char *pszSearchIn,
                                       const char *pszTag,
                                       PLONG pl);

    PSZ XWPENTRY strhGetTextAttr(const char *pszSearchIn, const char *pszTag, PULONG pulOffset);

    PSZ XWPENTRY strhFindEndOfTag(const char *pszBeginOfTag);

    ULONG XWPENTRY strhGetBlock(const char *pszSearchIn,
                                PULONG pulSearchOffset,
                                const char *pszTag,
                                PSZ *ppszBlock,
                                PSZ *ppszAttribs,
                                PULONG pulOfsBeginTag,
                                PULONG pulOfsBeginBlock);

    /* ******************************************************************
     *
     *   Miscellaneous
     *
     ********************************************************************/

    VOID XWPENTRY strhArrayAppend(PSZ *ppszRoot,
                                  const char *pcszNew,
                                  ULONG cbNew,
                                  PULONG pcbRoot);

    PSZ XWPENTRY strhCreateDump(PBYTE pb,
                                ULONG ulSize,
                                ULONG ulIndent);

    /* ******************************************************************
     *
     *   Fast string searches
     *
     ********************************************************************/

    void* XWPENTRY strhmemfind(const void *in_block,
                               size_t block_size,
                               const void *in_pattern,
                               size_t pattern_size,
                               size_t *shift,
                               BOOL *repeat_find);

    char* XWPENTRY strhtxtfind (const char *string,
                                const char *pattern);

#endif

#if __cplusplus
}
#endif

