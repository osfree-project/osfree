
/*
 *@@sourcefile configsys.h:
 *      header file for configsys.c. See notes there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@added V0.9.7 (2001-01-15) [umoeller]
 *@@include #include <os2.h>
 *@@include #include "helpers\xstring.h"            // for some funcs
 *@@include #include "helpers\configsys.h"
 */

/*
 *      Copyright (C) 1997-2001 Ulrich M”ller.
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

#ifndef CONFIGSYS_HEADER_INCLUDED
    #define CONFIGSYS_HEADER_INCLUDED

    typedef int CFGRPL;
    #define CFGRPL_ADD            0
    #define CFGRPL_UNIQUE         1
    #define CFGRPL_ADDLEFT        2
    #define CFGRPL_ADDRIGHT       3
    #define CFGRPL_REMOVELINE     4
    #define CFGRPL_REMOVEPART     5

    typedef int CFGVRT;
    #define CFGVRT_BOTTOM         0
    #define CFGVRT_TOP            1
    #define CFGVRT_BEFORE         2
    #define CFGVRT_AFTER          3

    /*
     *@@ CONFIGMANIP:
     *      describes a CONFIG.SYS manipulation.
     */

    typedef struct _CONFIGMANIP
    {
        CFGRPL      iReplaceMode;   // this is an int, really
                    // one of the following:
                    // -- CFGRPL_ADD (0): just add line (default; iVertical applies).
                    // -- CFGRPL_UNIQUE mode (1): existing line is replaced;
                    //    if not found, iVertical applies.
                    // -- CFGRPL_ADDLEFT mode (2): line is updated to the left;
                    //    if not found, iVertical applies.
                    // -- CFGRPL_ADDRIGHT mode (3): line is updated to the right;
                    //    if not found, iVertical applies.
                    // -- CFGRPL_REMOVELINE mode (4)
                    // -- CFGRPL_REMOVEPART mode (5)

        PCSZ        pszUniqueSearchString2;
                    // for UNIQUE(statement2):
                    // the "statement2" to search for
                    // V0.9.1 (2000-01-06) [umoeller]

        CFGVRT      iVertical;      // this is an int, really
                    // 0: add to bottom (default)
                    // 1: add to top
                    // 2: add before pszSearchString
                    // 3: add after pszSearchString
        PCSZ        pszVerticalSearchString;
                    // for iVertical == 2 or 3

        PCSZ        pszNewLine;
                    // this is a copy of stuff before the "|" char
                    // in pszConfigSys given to the constructor
    } CONFIGMANIP, *PCONFIGMANIP;

    // error codes
    #define CFGERR_FIRST                35000
    #define CFGERR_NOSEPARATOR          (CFGERR_FIRST + 1)
    #define CFGERR_MANIPULATING         (CFGERR_FIRST + 2)

    APIRET csysLoadConfigSys(const char *pcszFile,
                             PSZ *ppszContents);

    APIRET csysWriteConfigSys(const char *pcszFile,
                              const char *pcszContents,
                              PSZ pszBackup);

    PSZ csysFindKey(const char *pcszSearchIn,
                    const char *pcszKey,
                    PSZ *ppStartOfLine,
                    PBOOL pfIsAllUpperCase);

    PSZ csysGetParameter(const char *pcszSearchIn,
                         const char *pcszKey,
                         PSZ pszCopyTo,
                         ULONG cbCopyTo);

    PSZ csysSetParameter(PSZ* ppszBuf,
                         const char *pcszKey,
                         const char *pcszNewParam,
                         BOOL fRespectCase);

    BOOL csysDeleteLine(PSZ pszSearchIn, PSZ pszKey);

    #ifdef XSTRING_HEADER_INCLUDED
        APIRET csysManipulate(PSZ *ppszContents,
                              PCONFIGMANIP pManip,
                              PBOOL pfDirty,
                              PXSTRING pstrChanged);
    #endif

    /* ******************************************************************
     *
     *   Swappath
     *
     ********************************************************************/

    BOOL XWPENTRY csysParseSwapPath(const char *pcszConfigSys,
                                    PSZ pszSwapPath,
                                    PULONG pulMinFree,
                                    PULONG pulMinSize);
    typedef BOOL XWPENTRY CSYSPARSESWAPPATH(const char *pcszConfigSys,
                                            PSZ pszSwapPath,
                                            PULONG pulMinFree,
                                            PULONG pulMinSize);
    typedef CSYSPARSESWAPPATH *PCSYSPARSESWAPPATH;

    ULONG XWPENTRY csysQuerySwapperSize(VOID);
    typedef ULONG XWPENTRY CSYSQUERYSWAPPERSIZE(VOID);
    typedef CSYSQUERYSWAPPERSIZE *PCSYSQUERYSWAPPERSIZE;

#endif

#if __cplusplus
}
#endif

