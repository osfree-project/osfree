
/*
 *@@sourcefile acldb.c:
 *      database for access control lists (ACL's).
 *
 *      This file is a "black box" in that the rest of
 *      XWPShell does not care about how the ACL database
 *      is implemented. Instead, XWPShell simply requires
 *      that a function saclLoadDatabase exists, which
 *      loads the ACL database on startup. It is the
 *      responsibility of that function to call
 *      scxtCreateACLEntry for each resource definition,
 *      and the rest of XWPShell handles coordinating
 *      the database.
 *
 *      This ACLDB implementation uses a plain text file
 *      for speed, where each ACL entry consists of a
 *      single line like this:
 +
 +        "resname" id-hexflags [id-hexflags...]
 *
 *      being:
 *
 *      -- "resname": the name of the resource, being
 *              -- a drive letter (e.g. "G:")
 *              -- a full directory specification (e.g. "G:\DESKTOP")
 *              -- a full file path (e.g. "G:\DESKTOP\INDEX.HTML")
 *
 *      -- id: either a group id in the form "G2" or a user id in the
 *         form "U2"
 *
 *      -- hexflags: one byte in hexadecimal, specifying the ORed
 *         XWPACCESS_* flags for this resource.
 *
 *@@added V0.9.5 [umoeller]
 *@@header "security\xwpshell.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>

#include "setup.h"

#include "helpers\dosh.h"
#include "helpers\linklist.h"
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\tree.h"               // red-black binary trees

#include "helpers\xwpsecty.h"
#include "security\xwpshell.h"

/*
 *@@ saclLoadDatabase:
 *      loads the ACL database.
 *
 *      Called on startup by scxtInit.
 *
 *      The caller has locked the ACLDB before calling
 *      this, so this code need not worry about
 *      serialization.
 */

APIRET saclLoadDatabase(PULONG pulLineWithError)
{
    APIRET  arc = NO_ERROR;

    CHAR    szUserDB[CCHMAXPATH];
    PSZ     pszUserDB = NULL,
            pszDBPath = NULL;
    CHAR    szDBPath[CCHMAXPATH];
    FILE    *UserDBFile;

    ULONG   ulLineCount = 0;

    if (!(pszDBPath = getenv("ETC")))
    {
        // XWPUSERDB not specified:
        // default to "?:\os2" on boot drive
        sprintf(szDBPath, "%c:\\OS2", doshQueryBootDrive());
        pszDBPath = szDBPath;
    }
    sprintf(szUserDB, "%s\\xwpusers.acc", pszDBPath);

    if (!(UserDBFile = fopen(szUserDB, "r")))
        arc = _doserrno;
    else
    {
        CHAR        szLine[300];        // @@todo this is not safe!
        PSZ         pLine = NULL;
        while (pLine = fgets(szLine, sizeof(szLine), UserDBFile))
        {
            // "A:" G1-3F
            // pLine is on '"' char now
            PSZ         pFirstQuote,
                        pSecondQuote;
            PACLDBTREENODE pNewEntry;
            ULONG       ulResNameLen;

            while (     (*pLine)
                     && ( (*pLine == ' ') || (*pLine == '\t') )
                  )
                pLine++;

            if (    (!*pLine)
                 || (*pLine == '\n')
               )
                // empty line
                continue;

            pFirstQuote = pLine;
            if (    (*pFirstQuote != '"')
                 || (!(pSecondQuote = strchr(pLine + 2, '"')))
               )
            {
                arc = XWPSEC_DB_ACL_SYNTAX;
                break;
            }

            // "C:"
            //    ^ pSecondQuote
            // ^ pFirstQuote
            ulResNameLen = pSecondQuote - pFirstQuote - 1;

            // allocate an entry for this resource
            if (!(pNewEntry = (PACLDBTREENODE)malloc(   sizeof(ACLDBTREENODE)
                                                      + ulResNameLen)))
            {
                arc = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            lstInit(&pNewEntry->llPerms, TRUE);
            pNewEntry->usResNameLen = ulResNameLen;
            memcpy(pNewEntry->szResName,
                   pFirstQuote + 1,
                   ulResNameLen);
            pNewEntry->szResName[ulResNameLen] = '\0';
            nlsUpper(pNewEntry->szResName);

            // insert into global tree
            if (arc = scxtCreateACLEntry(pNewEntry))
                break;

            pLine = pSecondQuote + 1;

            // keep reading permissions for this resource on the same line
            while (!arc)
            {
                BYTE    bType = 0;
                while (    (*pLine)
                        && ( (*pLine == ' ') || (*pLine == '\t') )
                      )
                    pLine++;

                switch (*pLine++)
                {
                    case 'G':
                        bType = SUBJ_GROUP;
                    break;

                    case 'U':
                        bType = SUBJ_USER;
                    break;

                    case '\0':
                    case '\n':
                        // no more permissions in this line: stop
                    break;

                    default:
                        arc = XWPSEC_DB_ACL_SYNTAX;
                    break;
                }

                if (!bType)
                    break;
                else
                {
                    PSZ     pDash;
                    if (!(pDash = strchr(pLine, '-')))
                        arc = XWPSEC_DB_ACL_SYNTAX;
                    else
                    {
                        PACLDBPERM pPerm;
                        if (!(pPerm = NEW(ACLDBPERM)))
                            arc = ERROR_NOT_ENOUGH_MEMORY;
                        else
                        {
                            *pDash = '\0';
                            pPerm->id = atoi(pLine);
                            pPerm->bType = bType;
                            pPerm->fbPerm = strtol(pDash + 1, NULL, 16);

                            lstAppendItem(&pNewEntry->llPerms,
                                          pPerm);

                            pLine = pDash + 3;
                        }
                    }
                }
            } // while (!arc)

            #ifdef __DEBUG__
            {
                PLISTNODE pNode;
                _PmpfF(("permissions for res \"%s\":",
                        pNewEntry->szResName));
                FOR_ALL_NODES(&pNewEntry->llPerms, pNode)
                {
                    PACLDBPERM pPerm = pNode->pItemData;
                    _PmpfF(("   %s %d has perm 0x%lX",
                            (pPerm->bType == SUBJ_GROUP)
                                ? "group"
                                : "user",
                            pPerm->id,
                            pPerm->fbPerm));
                }
            }
            #endif

            if (arc)
                break;  // while

            ulLineCount++;
        } // end while ((pLine = fgets(szLine, sizeof(szLine), UserDBFile)) != NULL)

        *pulLineWithError = ulLineCount;

        fclose(UserDBFile);
    }

    return arc;
}


