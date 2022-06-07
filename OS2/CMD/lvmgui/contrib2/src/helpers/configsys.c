
/*
 *@@sourcefile configsys.c:
 *      contains helper functions for parsing and changing lines
 *      in the CONFIG.SYS file.
 *
 *      Some of these used to be in stringh.c. However, that
 *      code was really old and was largely rewritten. Besides,
 *      this fixes some bugs that have popped up in WarpIN.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  csys*       CONFIG.SYS helper functions.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@added V0.9.7 (2001-01-15) [umoeller]
 *@@header "helpers\configsys.h"
 */

/*
 *      Copyright (C) 1997-2002 Ulrich Mîller.
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

#define INCL_DOSPROFILE
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\stringh.h"
#include "helpers\xstring.h"            // extended string helpers

#include "helpers\configsys.h"          // CONFIG.SYS helpers

#pragma hdrstop

/*
 *@@category: Helpers\Control program helpers\CONFIG.SYS
 *      CONFIG.SYS parsing and manipulation. See configsys.c.
 */

/*
 *@@ csysLoadConfigSys:
 *      loads a CONFIG.SYS file.
 *
 *      If pcszFile is specified, that file is loaded.
 *      If (pcszFile == NULL), this loads the system CONFIG.SYS
 *      file (?:\CONFIG.SYS on the boot drive).
 *
 *      If this returns NO_ERROR, the contents of the file
 *      are put into ppszContents, which will then point
 *      to a new buffer (which must be free()'d by the caller).
 *
 *      Note that in that buffer, all \r\n pairs have been
 *      converted to \n (as with fopen()).
 */

APIRET csysLoadConfigSys(const char *pcszFile,     // in: CONFIG.SYS filename or NULL
                         PSZ *ppszContents)
{
    APIRET arc = NO_ERROR;

    CHAR szFile[CCHMAXPATH];
    if (pcszFile == NULL)
    {
        sprintf(szFile, "%c:\\CONFIG.SYS", doshQueryBootDrive());
        pcszFile = szFile;
    }

    if (!(arc = doshLoadTextFile(pcszFile,
                                 ppszContents,
                                 NULL)))
    {
        // convert all \r\n to \n
        XSTRING     strBuf;
        xstrInitSet(&strBuf, *ppszContents);
        xstrConvertLineFormat(&strBuf,
                              TRUE);        // to C format
        *ppszContents = strBuf.psz;
    }

    return arc;
}

/*
 *@@ csysWriteConfigSys:
 *
 *@@added V0.9.7 (2001-01-15) [umoeller]
 */

APIRET csysWriteConfigSys(const char *pcszFile,     // in: CONFIG.SYS filename or NULL
                          const char *pcszContents,
                          PSZ pszBackup)
{
    APIRET arc = NO_ERROR;

    XSTRING     strBuf;

    CHAR szFile[CCHMAXPATH];
    if (pcszFile == NULL)
    {
        sprintf(szFile, "%c:\\CONFIG.SYS", doshQueryBootDrive());
        pcszFile = szFile;
    }

    // convert all \n to \r\n
    xstrInitCopy(&strBuf, pcszContents, 0);
    xstrConvertLineFormat(&strBuf,
                          FALSE);        // C to OS/2 format

    arc = doshWriteTextFile(pcszFile,
                            strBuf.psz,
                            NULL,
                            pszBackup);

    xstrClear(&strBuf);

    return arc;
}

/*
 *@@ csysFindKey:
 *      finds pcszKey in pcszSearchIn.
 *
 *      This is similar to strhistr, but this one makes sure the
 *      key is at the beginning of a line. Spaces and tabs before
 *      the key are tolerated.
 *
 *      Returns NULL if the key was not found. Otherwise this
 *      returns the exact pointer to pcszKey in pcszSearchIn
 *      and puts the address of the first character of the line
 *      (after \r or \n) into ppStartOfLine.
 *
 *      Used by csysGetParameter/csysSetParameter; useful
 *      for analyzing CONFIG.SYS settings.
 *
 *      Remarks:
 *
 *      --  With pcszKey, you can either specify a CONFIG.SYS
 *          key WITH an equals sign (e.g. search for "LIBPATH=")
 *          or the key name only (e.g. "SET SHELL").
 *
 *          Only in the latter case, this function makes sure
 *          that the search key is complete; e.g. it will not
 *          find "SET SHELLHANDLESINC=" then.
 *
 *      --  If the line found starts with leading spaces,
 *          *ppStartOfLine will still receive the start of
 *          the line.
 *          For example, if you're searching for "SET PATH"
 *          and the PATH line is
 *
 +              "   SET PATH=C:\os2"
 *
 *          the first space in the line is written into
 *          *ppStartOfLine.
 *
 *@@changed V0.9.0 [umoeller]: fixed bug in that this would also return something if only the first chars matched
 *@@changed V0.9.0 [umoeller]: fixed bug which could cause character before pszSearchIn to be examined
 *@@changed V0.9.7 (2001-01-15) [umoeller]: moved this here from stringh.c; renamed from strhFindKey
 *@@changed V0.9.7 (2001-01-15) [umoeller]: now checking for tabs too
 *@@changed V0.9.11 (2001-04-25) [umoeller]: this never found lines which had leading spaces, fixed
 *@@changed V0.9.12 (2001-05-22) [umoeller]: added checks for key termination (SET SHELLHANDLESINC)
 */

PSZ csysFindKey(const char *pcszSearchIn,   // in: text buffer to search
                const char *pcszKey,        // in: key to search for (e.g. "PATH=")
                PSZ *ppStartOfLine,         // out: start of line which contains pcszKey;
                                            // ptr can be NULL
                PBOOL pfIsAllUpperCase)     // out: TRUE if key is completely in upper
                                            // case; ptr can be NULL
{
    const char  *p = pcszSearchIn;
    PSZ         pReturn = NULL;
    ULONG       ulKeyLength = strlen(pcszKey);

    BOOL        fSearchKeyContainsEquals = (strchr(pcszKey, '=') != 0);

    do
    {
        // find the key
        // (on first loop, p is start of buffer;
        // on subsequent loops, p is somewhere in bufer,
        // if we've found the key somewhere else)
        if (p = strhistr(p, pcszKey))
        {
            // make sure the key is at the beginning of a line
            // by going backwards until we find a char != " "
            const char *pStartOfLine = p;

            // fixed this V0.9.11 (2001-04-25) [umoeller]
            while (    (pStartOfLine > pcszSearchIn)
                    && (    (*(pStartOfLine - 1) == ' ')
                         || (*(pStartOfLine - 1) == '\t')       // allow tabs too
                       )
                  )
                --pStartOfLine;

            // if we're at the beginning of a line or
            // at the beginning of the buffer at all,
            // let's go on
            if (    (pStartOfLine == pcszSearchIn)     // order fixed V0.9.0, RÅdiger Ihle
                 || (*(pStartOfLine - 1) == '\r')
                 || (*(pStartOfLine - 1) == '\n')
               )
            {
                // OK, we're at the start of a line:

                // check the character AFTER the
                // key if it's a space or '=';
                // without this check, we'd find
                // SET SHELL= if caller is really
                // looking for SET SHELLHANDLESINC=,
                // but only do this check if the caller
                // doesn't have '=' in the search string
                // already
                // V0.9.12 (2001-05-22) [umoeller]
                CHAR c = *(p + ulKeyLength);
                if (    (fSearchKeyContainsEquals)
                     || (c == ' ')
                     || (c == '=')
                     || (c == '\n')
                     || (c == '\r')
                     || (c == '\t')
                   )
                {
                    // return address of key
                    pReturn = (PSZ)p;
                    // return start of line
                    if (ppStartOfLine)
                        *ppStartOfLine = (PSZ)pStartOfLine;

                    // test for all upper case?
                    if (pfIsAllUpperCase)
                    {
                        ULONG   ul = 0;
                        *pfIsAllUpperCase = TRUE;

                        for (ul = 0;
                             ul < ulKeyLength;
                             ul++)
                        {
                            if (islower(*(p + ul)))
                            {
                                *pfIsAllUpperCase = FALSE;
                                break; // for
                            }
                        }
                    }

                    break; // do
                } // else search next key
            } // else search next key

            p += ulKeyLength; // search on after this key
                    // now skipping ulKeyLength V0.9.12 (2001-05-22) [umoeller]
        }
        else
            // nothing more found:
            break;
    } while ((!pReturn) && (p != NULL) && (p != pcszSearchIn));

    return pReturn;
}

/*
 *@@ csysGetParameter:
 *      searches pcszSearchIn for the key pcszKey and gets
 *      its parameter.
 *
 *      If found, it returns a pointer to the following
 *      characters in pcszSearchIn and, if pszCopyTo != NULL,
 *      copies the rest of the line to that buffer, of which
 *      cbCopyTo specified the size.
 *
 *      If the key is not found, NULL is returned.
 *      String search is done by calling csysFindKey.
 *      This is useful for querying CONFIG.SYS settings.
 *
 *      <B>Example:</B>
 *
 *      For the CONFIG.SYS line
 *
 +          PAUSEONERROR=YES
 *
 *      this would return "YES" if you searched for "PAUSEONERROR=".
 */

PSZ csysGetParameter(const char *pcszSearchIn,  // in: text buffer to search
                     const char *pcszKey,       // in: key to search for (e.g. "PATH=")
                     PSZ pszCopyTo,             // out: key value
                     ULONG cbCopyTo)            // out: sizeof(*pszCopyTo)
{
    PSZ     p,
            prc = NULL;

    if (p = csysFindKey(pcszSearchIn, pcszKey, NULL, NULL))
    {
        prc = p + strlen(pcszKey);
        if (pszCopyTo)
        {
            // copy to pszCopyTo
            ULONG cb;
            PSZ pEOL;
            if (pEOL = strhFindEOL(prc, &cb))
            {
                if (cb > cbCopyTo)
                    cb = cbCopyTo-1;
                strhncpy0(pszCopyTo, prc, cb);
            }
        }
    }

    return prc;
}

/*
 *@@ csysSetParameter:
 *      searches *ppszBuf for the key pszKey; if found, it
 *      replaces the characters following this key up to the
 *      end of the line with pszParam. If pszKey is not found in
 *      *ppszBuf, it is appended to the file in a new line.
 *
 *      If any changes are made, *ppszBuf is re-allocated.
 *
 *      This function searches w/out case sensitivity.
 *
 *      Returns a pointer to the new parameter inside the
 *      reallocated buffer, or NULL if the key was not found
 *      (and a new item was added).
 *
 *      NOTE: This assumes that the file uses \n for line
 *      breaks only (not \r\n).
 *
 *@@changed V0.9.0 [umoeller]: changed function prototype to PSZ* ppszSearchIn
 *@@changed V0.9.7 (2001-01-15) [umoeller]: fixed various bugs with the mem buffers
 */

PSZ csysSetParameter(PSZ* ppszBuf,          // in: text buffer to search
                     const char *pcszKey,   // in: key to search for (e.g. "PATH=")
                     const char *pcszNewParam, // in: new parameter to set for key
                     BOOL fRespectCase)     // in: if TRUE, pszNewParam will
                             // be converted to upper case if the found key is
                             // in upper case also. pszNewParam should be in
                             // lower case if you use this.
{
    BOOL    fIsAllUpperCase = FALSE;
    PSZ     pKey,
            pReturn = NULL;
    if (pKey = csysFindKey(*ppszBuf,
                           pcszKey,
                           NULL,
                           &fIsAllUpperCase))
    {
        // key found in file:
        // ULONG   ulOfs;

        // replace existing parameter
        PSZ pOldParam;
        if (pOldParam = pKey + strlen(pcszKey))
        {
            // pOldParam now has the old parameter, which we
            // will overwrite now; pOldParam points into *ppszBuf

            ULONG   ulOldParamOfs = pOldParam - *ppszBuf;

            PSZ pEOL;
            if (pEOL = strhFindEOL(pOldParam, NULL))
            {
                // pEOL now has first end-of-line after the parameter

                // char count to replace
                ULONG   ulToReplace = pEOL - pOldParam;

                XSTRING strBuf,
                        strReplaceWith;

                // ULONG   ulOfs = 0;
                // PSZ pszOldParamCopy = strhSubstr(pOldParam, pEOL);
                /* (PSZ)malloc(cbOldParam+1);
                strncpy(pszOldCopy, pOldParam, cbOldParam);
                pszOldCopy[cbOldParam] = '\0'; */

                xstrInitSet(&strBuf, *ppszBuf);
                        // this must not be freed!

                xstrInitCopy(&strReplaceWith, pcszNewParam, 0);
                // check for upper case desired?
                if ((fRespectCase) && (fIsAllUpperCase))
                    strupr(strReplaceWith.psz);

                xstrrpl(&strBuf,
                        ulOldParamOfs,
                        ulToReplace,
                        strReplaceWith.psz,
                        strReplaceWith.ulLength);      // adjusted V0.9.11 (2001-04-22) [umoeller]

                xstrClear(&strReplaceWith);

                /* xstrFindReplaceC(&strBuf,
                                 &ulOfs,
                                 pszOldParamCopy,
                                 pszNewParam); */

                // free(pszOldParamCopy);

                // replace output buffer
                *ppszBuf = strBuf.psz;
                // return ptr into that
                pReturn = *ppszBuf + ulOldParamOfs;
            }
        }
    }
    else
    {
        // key not found: append to end of file
        XSTRING strContents;
        xstrInitCopy(&strContents, *ppszBuf, 0);
        /* PSZ pszNew = (PSZ)malloc(strlen(*ppszBuf)
                              + strlen(pcszKey)
                              + strlen(pszNewParam)
                              + 5);     // 2 * \r\n + null byte
                */

        xstrcatc(&strContents, '\n');
        xstrcat(&strContents, pcszKey, 0);
        xstrcat(&strContents, pcszNewParam, 0);
        xstrcatc(&strContents, '\n');
        /* sprintf(pszNew, "%s\r\n%s%s\r\n",
                *ppszBuf, pcszKey, pszNewParam); */

        free(*ppszBuf);
        *ppszBuf = strContents.psz;
    }

    return pReturn;
}

/*
 *@@ csysDeleteLine:
 *      this deletes the line in pszSearchIn which starts with
 *      the key pszKey. Returns TRUE if the line was found and
 *      deleted.
 *
 *      This copies within pszSearchIn.
 *
 *@@changed V0.9.7 (2001-01-15) [umoeller]: fixed wrong beginning of line
 */

BOOL csysDeleteLine(PSZ pszSearchIn,        // in: buffer to search
                    PSZ pszKey)             // in: key to find
{
    BOOL    fIsAllUpperCase = FALSE;
    PSZ     pStartOfLine = NULL;
    PSZ     pKey;
    BOOL    brc = FALSE;

    if (pKey = csysFindKey(pszSearchIn,
                           pszKey,
                           &pStartOfLine,
                           &fIsAllUpperCase))
    {
        PSZ pEOL;
        if (pEOL = strhFindEOL(pKey, NULL))
        {
            // pEOL now has first end-of-line after the key;
            // go to first character after EOL
            while (    (*pEOL)
                    && (   (*pEOL == '\n')
                        || (*pEOL == '\r')
                       )
                  )
                pEOL++;

            // delete line by overwriting it with
            // the next line
            strcpy(pStartOfLine,
                   pEOL);
        }
        else
        {
            // EOL not found: we must be at the end of the file
            *pKey = '\0';
        }
        brc = TRUE;
    }

    return brc;
}

/*
 *@@ csysManipulate:
 *      makes a single CONFIG.SYS change according to the
 *      CONFIGMANIP which was passed in.
 *
 *      If the manipulation succeeded, NO_ERROR is returned
 *      and pstrChanged receives a new string describing
 *      what was changed. This can be:
 *
 *      --  "DLL xxx":  deleted a line containing "xxx".
 *
 *      --  "DLP xxx":  deleted part of line.
 *
 *      --  "NWL xxx":  added an all new line.
 *
 *      --  "NWP xxx":  added a new part to an existing line.
 *
 *      This has been moved here from WarpIN so that C programs
 *      can use these features as well.
 *
 *      This returns:
 *
 *      -- NO_ERROR: changed OK, or nothing happened.
 *
 *      -- CFGERR_NOSEPARATOR: with CFGRPL_ADDLEFT or CFGRPL_ADDRIGHT
 *              modes, no "=" character was found.
 *
 *      -- CFGERR_MANIPULATING: error in string handling. This should
 *              never happen, and if it does, it's a bug.
 *
 *      Preconditions:
 *
 *      -- This assumes that the line breaks are represented
 *         by \n ONLY. If *ppszContents has \r\n line breaks,
 *         they must be converted to pure \n first.
 *
 *@@changed V0.9.18 (2002-03-24) [umoeller]: fixed leftover duplicate ';' chars with CFGRPL_REMOVEPART
 */

APIRET csysManipulate(PSZ *ppszContents,        // in/out: CONFIG.SYS text (reallocated)
                      PCONFIGMANIP pManip,      // in: CONFIG.SYS manipulation instructions
                      PBOOL pfDirty,            // out: set to TRUE if something was changed
                      PXSTRING pstrChanged)     // out: new string describing changes (must be free'd)
{
    APIRET  arc = NO_ERROR;

    PSZ     pszCommand = 0,
            pszArgNew = 0;
    BOOL    fIsAllUpperCase = FALSE;

    // The CONFIG.SYS manipulation is a six-step process,
    // depending on the data in the BSCfgSysManip instance.
    // This is an outline what needs to be done when;
    // the implementation follows below.

    // 1)  First find out if a line needs to be updated or deleted
    //     or if we only need to write a new line.
    //     This depends on BSCfgSysManip.iReplaceMode.
           BOOL    fFindLine = FALSE;

    // 2)  If we need to find a line, find the
    //     line and copy it into pszLineToUpdate.
    //     Set pInsertAt to the original line, if found.
    //     If not found, set fInsertWRTVertical to TRUE.
           PSZ     pInsertAt = 0;
                // address of that line in original buffer if (fFindLine == TRUE) and
                // found
           BOOL    fInsertWRTVertical = FALSE;
                // TRUE means set a new pInsertAt depending on Manip.iVertical;
                // this is set to TRUE if (fFindLine == FALSE) or line not found

    // 3)  If a line needs to be deleted, well, delete it then.
           PSZ     pLineToDelete = 0;
                // pointer to start of original line (pLineToUpdate);
                // points to character right after \n
           PSZ     pszDeletedLine = 0;
                // copy of that line if deleted (this has the stuff between
                // the EOL chars, not including those)

    // 4)  If (fInsertWRTVertical == TRUE), set pInsertAt to a position in
    //     the original buffer, depending on Manip.iVertical.

    // 5)  Compose a new line to be inserted at pInsertAt.
           PSZ     pszLineToInsert = 0;

    // 6)  Insert that line at pInsertAt.

    /*
     * Prepare data
     *
     */

    // First, check if the new-line string has a "=" char
    PSZ pSep;
    if (pSep = strchr(pManip->pszNewLine, '='))
    {
        // yes: separate in two strings
        pszCommand = strhSubstr(pManip->pszNewLine, pSep);
        pszArgNew = strdup(pSep+1);
    }
    else
    {
        // no:
        pszCommand = strdup(pManip->pszNewLine);
        pszArgNew = 0;
    }

    /*
     * GO!!
     *
     */

    // **** 1): do we need to search for a line?
    switch (pManip->iReplaceMode)
    {
        case CFGRPL_ADD:
            fFindLine = FALSE;
            fInsertWRTVertical = TRUE;      // always find new position
        break;

        case CFGRPL_UNIQUE:
        case CFGRPL_ADDLEFT:
        case CFGRPL_ADDRIGHT:
        case CFGRPL_REMOVELINE:
        case CFGRPL_REMOVEPART:
            fFindLine = TRUE;               // search line first; this sets
                                            // fInsertWRTVertical if not found
        break;
    }

    // **** 2): search line, if needed
    if (fFindLine)
    {
        PSZ     pKeyFound = 0,
             // address of key found
                pStartOfLineWithKeyFound = 0,
             // pointer to that line in original buffer; NULL if not found
                pszSearch = *ppszContents;

        do // while ((pManip->pszUniqueSearchString2) && (pszSearch));
        {
            pKeyFound = csysFindKey(pszSearch,          // CONFIG.SYS text
                                    pszCommand,         // stuff to search for
                                    &pStartOfLineWithKeyFound,
                                    &fIsAllUpperCase);
                // returns beginning of line

            if (!pKeyFound)
                break;
            else
                if (pManip->pszUniqueSearchString2)
                {
                    // UNIQUE(xxx) mode:
                    // find end of line
                    PSZ pEOL = strhFindEOL(pKeyFound, NULL);

                    // find "="
                    PSZ p = strchr(pKeyFound, '=');
                    if (p && p < pEOL)
                    {
                        // find UNIQUE(xxx) substring in line
                        p = strhistr(p, pManip->pszUniqueSearchString2);
                        if (p && p < pEOL)
                            // found: OK, keep pLineToUpdate
                            break;
                        else
                            pKeyFound = NULL;   // V0.9.7 (2001-01-15) [umoeller]
                    }

                    // else: search on
                    pszSearch = pEOL;
                }
        } while ((pManip->pszUniqueSearchString2) && (pszSearch));

        if (pKeyFound)
        {
            // line found:
            pLineToDelete = pStartOfLineWithKeyFound;
            pInsertAt = pStartOfLineWithKeyFound;
        }
        else
            // line not found:
            // insert new line then,
            // respecting the iVertical flag,
            // unless we are in "remove" mode
            if (    (pManip->iReplaceMode != CFGRPL_REMOVELINE)
                 && (pManip->iReplaceMode != CFGRPL_REMOVEPART)
               )
                fInsertWRTVertical = TRUE;
    }

    // **** 3): delete original line (OK)
    if (pLineToDelete)
    {
        // pLineToDelete is only != NULL if we searched and found a line above
        // and points to the character right after \n;
        // find end of this line
        PSZ pEOL = strhFindEOL(pLineToDelete, NULL);

        // make a copy of the line to be deleted;
        // this might be needed later
        pszDeletedLine = strhSubstr(pLineToDelete, pEOL);

        // pEOL now points to the \n char or the terminating 0 byte;
        // if not null byte, advance pointer
        while (*pEOL == '\n')
                pEOL++;

        // overwrite beginning of line with next line;
        // the buffers overlap, but this works
        strcpy(pLineToDelete, pEOL);

        // we've made changes...
        *pfDirty = TRUE;
    }

    // **** 4):
    if (fInsertWRTVertical)
    {
        // this is only TRUE if a) we didn't search for a line
        // or b) we did search, but we didn't find a line
        // (but never with CFGRPL_REMOVE* mode);
        // we need to find a new vertical position in CONFIG.SYS then
        // and set pInsertAt to the beginning of a line (after \n)
        switch (pManip->iVertical)
        {
            case CFGVRT_BOTTOM:
                // insert at the very bottom (default)
                pInsertAt = *ppszContents + strlen(*ppszContents);
            break;

            case CFGVRT_TOP:
                // insert at the very top
                pInsertAt = *ppszContents;
            break;

            case CFGVRT_BEFORE:
            {
                // Manip.pszSearchString has the search string
                // before whose line we must insert
                PSZ pFound;
                if (pFound = strhistr(*ppszContents,
                                      pManip->pszVerticalSearchString))
                {
                    // search string found:
                    // find beginning of line
                    while (     (pFound > *ppszContents)
                             && (*pFound != '\n')
                          )
                        pFound--;

                    if (pFound > *ppszContents)
                        // we're on the \n char now:
                        pFound++;

                    pInsertAt = pFound;
                }
                else
                    // search string not found: insert at top then
                    pInsertAt = *ppszContents;
            break; }

            case CFGVRT_AFTER:
            {
                // Manip.pszSearchString has the search string
                // after whose line we must insert
                PSZ pFound;
                if (pFound = strhistr(*ppszContents,
                                      pManip->pszVerticalSearchString))
                {
                    // search string found:
                    // find end of line
                    pInsertAt = strhFindNextLine(pFound, NULL);
                }
                else
                    // search string not found: insert at bottom then
                    pInsertAt = *ppszContents + strlen(*ppszContents);
            break; }
        }
    }

    // **** 5): compose new line
    switch (pManip->iReplaceMode)
    {
        case CFGRPL_ADD:
        case CFGRPL_UNIQUE:
            // that's easy, the new line is simply given
            pszLineToInsert = strdup(pManip->pszNewLine);
        break;

        case CFGRPL_REMOVELINE:
            // that's easy, just leave pszLineToInsert = 0;
        break;

        case CFGRPL_REMOVEPART:
            if (pszDeletedLine)
            {
                // parse the line which we removed above,
                // find the part which was to be removed,
                // and set pszLineToInsert to the rest of that
                PSZ     pArgToDel = 0;
                // find the subpart to look for
                if ((pArgToDel = strhistr(pszDeletedLine, pszArgNew)))
                {
                    // ULONG ulOfs = 0;
                    // pArgToDel now has the position of the
                    // part to be deleted in the old part
                    /* pszLineToInsert = strdup(pszDeletedLine);
                    strhFindReplace(&pszLineToInsert,
                                    &ulOfs,
                                    pszArgNew,   // search string
                                    "");         // replacement string
                    */

                    // V0.9.18 (2002-03-24) [umoeller]
                    // the above left duplicate ";" characters in the line
                    ULONG ulArgNewLen = strlen(pszArgNew);
                    ULONG cbFirst, cb;
                    PSZ pAfter = pArgToDel + ulArgNewLen;
                    while (*pAfter == ';')
                        ++pAfter;

                    // now recompose
                    cbFirst = pArgToDel - pszDeletedLine;
                    cb =   cbFirst                          // length up to pArgToDel
                         + strlen(pAfter)                   // length of remainder
                         + 1;                               // null terminator;
                    if (pszLineToInsert = (PSZ)malloc(cb))
                    {
                        if (cbFirst)
                            memcpy(pszLineToInsert, pszDeletedLine, cbFirst);
                        strcpy(pszLineToInsert + cbFirst,
                               pAfter);
                    }
                    else
                        arc = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                    // subpart not found:
                    // reinsert the old line
                    pszLineToInsert = strdup(pszDeletedLine);
            }
            // else line to be deleted was not found: leave pszLineToInsert = 0;
        break;

        case CFGRPL_ADDLEFT:
        case CFGRPL_ADDRIGHT:
            // add something to the left or right of the existing
            // argument:
            // check if we found a line above
            if (pszDeletedLine)
            {
                PSZ     pszCommandOld = 0,
                        pszArgOld = 0;
                // yes: split the existing line
                pSep = strchr(pszDeletedLine, '=');
                if (pSep)
                {
                    // "=" found: separate in two strings
                    pszCommandOld = strhSubstr(pszDeletedLine, pSep);
                    pszArgOld = strdup(pSep+1);
                }
                else
                {
                    // no "=" found: that's strange
                    /* throw(BSConfigExcpt(CFGEXCPT_NOSEPARATOR,
                                        strdup(pszDeletedLine))); */
                    arc = CFGERR_NOSEPARATOR;
                    break;
                }

                if ((pszCommandOld) && (pszArgOld))
                {
                    // does the new arg exist in old arg already?
                    if (strhistr(pszArgOld, pszArgNew) == 0)
                    {
                        // no: go!
                        ULONG   cbCommandOld = strlen(pszCommandOld);
                        ULONG   cbArgOld = strlen(pszArgOld);
                        ULONG   cbArgNew = strlen(pszArgNew);   // created at the very top

                        // compose new line
                        pszLineToInsert = (PSZ)malloc(cbCommandOld
                                                        + cbArgOld
                                                        + cbArgNew
                                                        + 5);

                        if (pManip->iReplaceMode == CFGRPL_ADDLEFT)
                            // add new arg to the left:
                            sprintf(pszLineToInsert,
                                    "%s=%s;%s",
                                    pszCommandOld,
                                    pszArgNew,
                                    pszArgOld);
                        else
                            // add new arg to the right:
                            if (*(pszArgOld + cbArgOld - 1) == ';')
                                // old arg has terminating ";" already:
                                sprintf(pszLineToInsert,
                                        "%s=%s%s",
                                        pszCommandOld,
                                        pszArgOld,
                                        pszArgNew);
                            else
                                // we need to append a ";":
                                sprintf(pszLineToInsert,
                                        "%s=%s;%s",
                                        pszCommandOld,
                                        pszArgOld,
                                        pszArgNew);
                    } // end if (stristr(pszArgOld, pszArgNew) == 0)
                    else
                        // exists already:
                        // reinsert the line we deleted above
                        pszLineToInsert = strdup(pszDeletedLine);

                    free(pszCommandOld);
                    free(pszArgOld);
                }
            } // end if (pszDeletedLine)
            else
                // line was not found: add a new one then
                // (the position has been determined above)
                pszLineToInsert = strdup(pManip->pszNewLine);
        break;
    }

    // **** 6): insert new line at pInsertAt
    if ((pszLineToInsert) && (arc == NO_ERROR))
    {
        PSZ     pszInsert2 = NULL;
        PSZ     pszNew = NULL;
        ULONG   cbInsert = strlen(pszLineToInsert);
        BOOL    fLineBreakNeeded = FALSE;

        // check if the char before pInsertAt is EOL
        if (pInsertAt > *ppszContents)
            if (*(pInsertAt-1) != '\n')
                fLineBreakNeeded = TRUE;

        if (fLineBreakNeeded)
        {
            // not EOL (this might be the case if we're
            // appending to the very bottom of CONFIG.SYS)
            pszInsert2 = (PSZ)malloc(cbInsert + 5);
            // insert an extra line break then
            *pszInsert2 = '\n';
            memcpy(pszInsert2 + 1, pszLineToInsert, cbInsert);
            strcpy(pszInsert2 + cbInsert + 1, "\n");
        }
        else
        {
            // OK:
            pszInsert2 = (PSZ)malloc(cbInsert + 3);
            memcpy(pszInsert2, pszLineToInsert, cbInsert);
            strcpy(pszInsert2 + cbInsert, "\n");
        }

        pszNew = strhins(*ppszContents,         // source
                         (pInsertAt - *ppszContents), // offset
                         pszInsert2);         // insertion string
        if (pszNew)
        {
            free(pszInsert2);
            free(*ppszContents);
            *ppszContents = pszNew;

            // we've made changes...
            *pfDirty = TRUE;
        }
        else
            arc = CFGERR_MANIPULATING;
    }
    else
        if (    (pManip->iReplaceMode != CFGRPL_REMOVELINE)
             && (pManip->iReplaceMode != CFGRPL_REMOVEPART)
           )
            arc = CFGERR_MANIPULATING;

    if (arc == NO_ERROR)
    {
        // finally, create log entries for Undo().
        // This uses the first three characters of the
        // each log entry to signify what happend:
        //  --  "DLL":      deleted line
        //  --  "DLP":      deleted part of line
        //  --  "NWL":      added an all new line
        //  --  "NWP":      added a new part to an existing line
        // There may be several log entries per manipulator
        // if a line was deleted and then re-inserted (i.e. updated).

        if (pstrChanged)
        {
            switch (pManip->iReplaceMode)
            {
                case CFGRPL_UNIQUE:
                    if (pszDeletedLine)
                    {
                        // did we delete a line?
                        xstrcat(pstrChanged, "DLL ", 0);
                        xstrcat(pstrChanged, pszDeletedLine, 0);
                        xstrcatc(pstrChanged, '\n');
                    }

                // no break, because we also added something
                goto LABEL_CFGRPL_ADD ;
                            // VAC gives us warnings otherwise...

                case CFGRPL_ADD:
LABEL_CFGRPL_ADD:
                    if (pszLineToInsert)
                    {
                        // added a line:
                        xstrcat(pstrChanged, "NWL ", 0);
                        xstrcat(pstrChanged, pszLineToInsert, 0);
                        xstrcatc(pstrChanged, '\n');
                    }
                break;

                case CFGRPL_ADDLEFT:
                case CFGRPL_ADDRIGHT:
                    if (pszDeletedLine)
                    {
                        // a line was found and updated:
                        // store the new part only
                        xstrcat(pstrChanged, "NWP ", 0);
                        xstrcat(pstrChanged, pManip->pszNewLine, 0);
                    }
                    else
                    {
                        xstrcat(pstrChanged, "NWL ", 0);
                        xstrcat(pstrChanged, pszLineToInsert, 0);
                    }
                    xstrcatc(pstrChanged, '\n');
                break;

                case CFGRPL_REMOVELINE:
                    if (pszDeletedLine)
                    {
                        // did we delete a line?
                        xstrcat(pstrChanged, "DLL ", 0);
                        xstrcat(pstrChanged, pszDeletedLine, 0);
                        xstrcatc(pstrChanged, '\n');
                    }
                break;

                case CFGRPL_REMOVEPART:
                    if (pszDeletedLine)
                    {
                        // did we delete a part?
                        xstrcat(pstrChanged, "DLP ", 0);
                        xstrcat(pstrChanged, pManip->pszNewLine, 0);
                        xstrcatc(pstrChanged, '\n');
                    }
                break;
            }
        } // end if (pstrChanged)
    }

    // clean up
    if (pszCommand)
        free(pszCommand);
    if (pszArgNew)
        free(pszArgNew);
    if (pszDeletedLine)
        free(pszDeletedLine);
    if (pszLineToInsert)
        free(pszLineToInsert);

    return arc;
}

/* ******************************************************************
 *
 *   Swappath
 *
 ********************************************************************/

extern CHAR     G_szSwapperFilename[CCHMAXPATH] = "";

/*
 *@@ csysParseSwapPath:
 *
 *@@added V0.9.9 (2001-02-08) [umoeller]
 *@@changed V0.9.16 (2001-11-10) [umoeller]: moved to helpers
 */

BOOL csysParseSwapPath(const char *pcszConfigSys,    // in: if NULL, this gets loaded
                       PSZ pszSwapPath,              // out: swapper directory
                       PULONG pulMinFree,            // out: min free
                       PULONG pulMinSize)            // out: min size
{
    BOOL brc = FALSE;

    PSZ pszConfigSysTemp = 0;

    if (!pcszConfigSys)
    {
        // not specified: load it
        if (csysLoadConfigSys(NULL, &pszConfigSysTemp) == NO_ERROR)
            pcszConfigSys = pszConfigSysTemp;
    }

    if (pcszConfigSys)
    {
        // parse SWAPPATH command
        PSZ p;
        if (p = csysGetParameter(pcszConfigSys, "SWAPPATH=", NULL, 0))
        {
            CHAR    szSwap[CCHMAXPATH];
            ULONG   ulMinFree = 2048, ulMinSize = 2048;
            // int     iScanned;
            sscanf(p,
                   "%s %d %d",
                   &szSwap, &ulMinFree, &ulMinSize);

            if (pszSwapPath)
                strcpy(pszSwapPath, szSwap);
            if (pulMinFree)
                *pulMinFree = ulMinFree;
            if (pulMinSize)
                *pulMinSize = ulMinSize;

            if (G_szSwapperFilename[0] == '\0')
            {
                // first call: copy to global so that the swapper
                // monitors will always use the old one, in case
                // the user changes this
                strcpy(G_szSwapperFilename, szSwap);
                if (G_szSwapperFilename[strlen(G_szSwapperFilename)-1] != '\\')
                    strcat(G_szSwapperFilename, "\\");
                strcat(G_szSwapperFilename, "swapper.dat");
            }

            brc = TRUE;
        }
    }

    if (pszConfigSysTemp)
        free(pszConfigSysTemp);

    return brc;
}

/*
 *@@ csysQuerySwapperSize:
 *      returns the current size of the swap file
 *      in bytes.
 *
 *@@added V0.9.9 (2001-02-08) [umoeller]
 *@@changed V0.9.16 (2001-11-10) [umoeller]: moved to helpers
 */

ULONG csysQuerySwapperSize(VOID)
{
    ULONG ulrc = 0;

    if (G_szSwapperFilename[0] == '\0')
    {
        // first call: compose the path
        csysParseSwapPath(NULL,
                          NULL,
                          NULL,
                          NULL);
    }

    if (G_szSwapperFilename[0])
        doshQueryPathSize(G_szSwapperFilename, &ulrc);

    return ulrc;
}

// testcase

/* int main()
{
    PSZ pszContents = NULL;
    if (0 == csysLoadConfigSys(NULL, &pszContents))
    {
        CHAR    szValue[1000];
        PSZ pszValue = csysGetParameter(pszContents,
                                        "IFS=",
                                        szValue,
                                        sizeof(szValue));
        if (pszValue)
            printf("Value: %s", szValue);
    }

    return 0;
} */
