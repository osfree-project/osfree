
/*
 *@@sourcefile tmsgfile.c:
 *      replacement code for DosGetMessage for decent NLS support.
 *
 *      This code has the following advantages over DosGetMessage:
 *
 *      1)  No external utility is necessary to change message
 *          files. Simply edit the text, and on the next call,
 *          the file is recompiled at run-time.
 *
 *      2)  To identify messages, any string can be used, not
 *          only numerical IDs.
 *
 *      The .TMF file must have the following format:
 *
 *      1)  Any message must start on the beginning of a line
 *          and look like this:
 *
 +              <--MSGID-->: message text
 *
 *          "MSGID" can be any string and will serve as the
 *          message identifier for tmfGetMessage.
 *          Leading spaces after "-->:" will be ignored.
 *
 *      2)  The message text may span across several lines.
 *          If so, the line breaks are returned as plain
 *          newline (\n) characters by tmfGetMessage.
 *
 *      3)  Comments in the file are supported if they start
 *          with a semicolon (";") at the beginning of a line.
 *          Any following text is ignored until the next
 *          message ID.
 *
 *      This file was originally added V0.9.0. The original
 *      code was contributed by Christian Langanke, but this
 *      has been completely rewritten with V0.9.16 to use my
 *      fast string functions now. Also, tmfGetMessage now
 *      requires tmfOpenMessageFile to be called beforehand
 *      and keeps all messages in memory for speed.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes:
 *      --  tmf*   text message file functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\tmsgfile.h"
 *@@added V0.9.0 [umoeller]
 */

/*
 *      Copyright (C) 2001-2002 Ulrich M”ller.
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
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\datetime.h"
#include "helpers\dosh.h"
#include "helpers\eah.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\tree.h"
#include "helpers\xstring.h"

#include "helpers\tmsgfile.h"

/*
 *@@category: Helpers\Control program helpers\Text message files (TMF)
 *      see tmsgfile.c.
 */

/* ******************************************************************
 *
 *   Declarations
 *
 ********************************************************************/

/*
 *@@ MSGENTRY:
 *      private representation of a message in a text
 *      message file. Each tree entry in
 *      TMFMSGFILE.IDsTreeRoot points to one of these
 *      structures.
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 */

typedef struct _MSGENTRY
{
    TREE        Tree;               // ulKey points to strID.psz
    XSTRING     strID;              // message ID
    ULONG       ulOfsText;          // offset of start of message text in file
    ULONG       cbText;             // length of text in msg file
} MSGENTRY, *PMSGENTRY;

// globals
STATIC PCSZ     G_pcszStartMarker = "\n<--",
                G_pcszEndMarker = "-->:";

/* ******************************************************************
 *
 *   Functions
 *
 ********************************************************************/

/*
 *@@ LoadAndCompile:
 *      loads and compiles the message file into the
 *      given TMFMSGFILE struct.
 *
 *      This has been extracted from tmfOpenMessageFile
 *      to allow for recompiling on the fly if the
 *      message file's last-write date/time changed.
 *
 *      Preconditions:
 *
 *      --  pFile->pszFilename must have been set.
 *
 *      All other fields get initialized here.
 *
 *@@added V0.9.18 (2002-03-24) [umoeller]
 *@@changed V0.9.20 (2002-07-19) [umoeller]: optimized, no longer holding all msgs im mem
 *@@changed V1.0.1 (2002-12-16) [pr]: Ctrl-Z fix
 */

APIRET LoadAndCompile(PTMFMSGFILE pTmf,     // in: TMF struct to set up
                      PXFILE pFile)         // in: opened XFILE for msg file
{
    APIRET  arc;

    PSZ     pszContent = NULL;
    ULONG   cbRead;

    if (!(arc = doshReadText(pFile,
                             &pszContent,
                             &cbRead)))    // bytes read including null char
    {
        // file loaded:

        PCSZ    pStartOfFile,
                pStartOfMarker;

        ULONG   ulStartMarkerLength = strlen(G_pcszStartMarker),
                ulEndMarkerLength = strlen(G_pcszEndMarker);

        XSTRING strContents;

        // initialize TMFMSGFILE struct
        treeInit(&pTmf->IDsTreeRoot, NULL);

        xstrInit(&strContents,
                 cbRead);       // including null byte
        xstrcpy(&strContents,
                pszContent,
                cbRead - 1);    // not including null byte

        // go build a tree of all message IDs...
        pStartOfFile = strContents.psz;

        // find first start message marker
        pStartOfMarker = strstr(pStartOfFile,
                                G_pcszStartMarker);     // start-of-line marker
        while (    (pStartOfMarker)
                && (!arc)
              )
        {
            // start marker found:
            PCSZ pStartOfMsgID = pStartOfMarker + ulStartMarkerLength;
            // search next start marker
            PCSZ pStartOfNextMarker = strstr(pStartOfMsgID + 1,
                                             G_pcszStartMarker);    // "\n<--"
            // and the end-marker
            PCSZ pEndOfMarker = strstr(pStartOfMsgID + 1,
                                       G_pcszEndMarker);              // "-->:"

            PMSGENTRY pNew;

            // sanity checks...

            if (    (pStartOfNextMarker)
                 && (pStartOfNextMarker < pEndOfMarker)
               )
            {
                // next start marker before end marker:
                // that doesn't look correct, skip this entry
                pStartOfMarker = pStartOfNextMarker;
                continue;
            }

            if (!pEndOfMarker)
                // no end marker found:
                // that's invalid too, and there can't be any
                // message left in the file then...
                break;

            // alright, this ID looks correct now
            if (!(pNew = NEW(MSGENTRY)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                // length of the ID
                ULONG   ulIDLength = pEndOfMarker - pStartOfMsgID;
                PCSZ    pStartOfText = pEndOfMarker + ulEndMarkerLength,
                        pNextComment;

                ZERO(pNew);

                // copy the string ID (between start and end markers)
                xstrInit(&pNew->strID, 0);
                xstrcpy(&pNew->strID,
                        pStartOfMsgID,
                        ulIDLength);
                // make ulKey point to the string ID for tree sorting
                pNew->Tree.ulKey = (ULONG)pNew->strID.psz;

                // skip leading spaces
                while (*pStartOfText == ' ')
                    pStartOfText++;

                // store offset of start of text
                pNew->ulOfsText = pStartOfText - pStartOfFile;

                // check if there's a comment before the
                // next item
                if (pNextComment = strstr(pStartOfText, "\n;"))
                {
                    if (    (!pStartOfNextMarker)
                         || (pNextComment < pStartOfNextMarker)
                       )
                        pNew->cbText =    // offset of comment marker
                                          (pNextComment - pStartOfFile)
                                        - pNew->ulOfsText;
                    else
                        // we have a next marker AND
                        // the comment comes after it
                        pNew->cbText =    // offset of next marker
                                         (pStartOfNextMarker - pStartOfFile)
                                       - pNew->ulOfsText;
                }
                else
                    if (pStartOfNextMarker)
                        // other markers left:
                        pNew->cbText =    // offset of next marker
                                         (pStartOfNextMarker - pStartOfFile)
                                       - pNew->ulOfsText;
                    else
                        // this was the last message:
                        pNew->cbText = strlen(pStartOfText);

                // remove trailing newlines and Ctrl-Z
                while (    (pNew->cbText)
                        && (    (pStartOfText[pNew->cbText - 1] == '\n')
                             || (pStartOfText[pNew->cbText - 1] == '\r')
                             || (pStartOfText[pNew->cbText - 1] == '\x1a') // V1.0.1 (2002-12-16) [pr]
                           )
                      )
                    (pNew->cbText)--;

                // store this thing
                if (!treeInsert(&pTmf->IDsTreeRoot,
                                NULL,
                                (TREE*)pNew,
                                treeCompareStrings))
                    // successfully inserted:
                    (pTmf->cIDs)++;
            }

            // go on with next start marker (can be NULL)
            pStartOfMarker = pStartOfNextMarker;
        } // end while (    (pStartOfMarker) ...

        free(pszContent);

    } // end else if (!(pTmf = NEW(TMFMSGFILE)))

    return arc;
}

/*
 *@@ tmfOpenMessageFile:
 *      opens a .TMF message file for future use
 *      with tmfGetMessage.
 *
 *      Use tmfCloseMessageFile to close the file
 *      again and free all resources.
 *
 *      Returns:
 *
 *      --  NO_ERROR: *ppMsgFile has received the
 *          new TMFMSGFILE structure.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      plus any of the errors of doshLoadTextFile,
 *      such as ERROR_FILE_NOT_FOUND.
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 *@@changed V0.9.20 (2002-07-19) [umoeller]: optimized, no longer holding all msgs im mem
 */

APIRET tmfOpenMessageFile(const char *pcszMessageFile, // in: fully q'fied .TMF file name
                          PTMFMSGFILE *ppMsgFile)     // out: TMFMSGFILE struct
{
    APIRET arc;

    ULONG   cbFile = 0; // V1.0.7 (2005-05-21) [pr]: Workaround for kernel bug
    PXFILE  pFile;
    if (!(arc = doshOpen(pcszMessageFile,
                         XOPEN_READ_EXISTING,
                         &cbFile,
                         &pFile)))
    {
        // create a TMFMSGFILE entry
        PTMFMSGFILE pTmf;
        if (!(pTmf = NEW(TMFMSGFILE)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            ZERO(pTmf);
            pTmf->pszFilename = strdup(pcszMessageFile);

            // TMFMSGFILE created:
            if (!(arc = LoadAndCompile(pTmf, pFile)))
            {
                // set timestamp to that of the file
                FILESTATUS3 fs3;
                if (!(arc = DosQueryFileInfo(pFile->hf,
                                             FIL_STANDARD,
                                             &fs3,
                                             sizeof(fs3))))
                {
                    dtCreateFileTimeStamp(pTmf->szTimestamp,
                                          &fs3.fdateLastWrite,
                                          &fs3.ftimeLastWrite);

                    // output
                    *ppMsgFile = pTmf;
                }
            }

            if (arc)
                // error:
                tmfCloseMessageFile(&pTmf);
        }

        doshClose(&pFile);
    }

    return arc;
}

/*
 *@@ FreeInternalMem:
 *      cleans out the internal message file compilation
 *      and the content string. Used by both tmfCloseMessageFile
 *      and tmfGetMessage to allow for recompiles when the
 *      last-write date/time changed.
 *
 *@@added V0.9.18 (2002-03-24) [umoeller]
 */

STATIC VOID FreeInternalMem(PTMFMSGFILE pTmf)
{
    LONG    cItems;
    TREE**  papNodes;

    if (    (cItems = pTmf->cIDs)
         && (papNodes = treeBuildArray(pTmf->IDsTreeRoot,
                                       &cItems))
       )
    {
        ULONG ul;
        for (ul = 0; ul < cItems; ul++)
        {
            PMSGENTRY pNodeThis = (PMSGENTRY)(papNodes[ul]);

            xstrClear(&pNodeThis->strID);

            free(pNodeThis);
        }

        free(papNodes);
    }
}

/*
 *@@ tmfCloseMessageFile:
 *      closes a message file opened by
 *      tmfOpenMessageFile, frees all resources,
 *      and sets *ppMsgFile to NULL for safety.
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 */

APIRET tmfCloseMessageFile(PTMFMSGFILE *ppMsgFile)
{
    if (ppMsgFile && *ppMsgFile)
    {
        PTMFMSGFILE pTmf = *ppMsgFile;

        if (pTmf->pszFilename)
            free(pTmf->pszFilename);

        FreeInternalMem(pTmf);

        free(pTmf);
        *ppMsgFile = NULL;

        return NO_ERROR;
    }

    return ERROR_INVALID_PARAMETER;
}

/*
 *@@ tmfGetMessage:
 *      replacement for DosGetMessage.
 *
 *      After you have opened a .TMF file with tmfOpenMessageFile,
 *      you can pass it to this function to retrieve a message
 *      with the given string (!) ID. See tmsgfile.c for details.
 *
 *      Note that this will invoke xstrcpy on the given XSTRING
 *      buffer. In other words, the string must be initialized
 *      (see xstrInit), but will be replaced.
 *
 *      This does perform the same brain-dead string replacements
 *      as DosGetMessage, that is, the string "%1" will be
 *      replaced with pTable[0], "%2" will be replaced with
 *      pTable[1], and so on.
 *
 *      Returns:
 *
 *      --  NO_ERROR;
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_MR_MID_NOT_FOUND
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 *@@changed V0.9.18 (2002-03-24) [umoeller]: now recompiling if last write date changed
 *@@changed V0.9.20 (2002-07-19) [umoeller]: optimized, no longer holding all msgs im mem
 */

APIRET tmfGetMessage(PTMFMSGFILE pMsgFile,      // in: msg file opened by tmfOpenMessageFile
                     PCSZ pcszMessageName,      // in: msg name to look for (case-sensitive!)
                     PXSTRING pstr,             // out: message string, if found (XSTRING must be initialized)
                     PCSZ *pTable,              // in: replacement table or NULL
                     ULONG cTableEntries)       // in: count of items in pTable or null
{
    APIRET  arc;
    ULONG   cbFile = 0; // V1.0.7 (2005-05-21) [pr]: Workaround for kernel bug
    PXFILE  pFile;

    if (    (!pMsgFile)
         || (!pMsgFile->pszFilename)
       )
        return ERROR_INVALID_PARAMETER;

    // open the file again V0.9.20 (2002-07-19) [umoeller]
    if (!(arc = doshOpen(pMsgFile->pszFilename,
                         XOPEN_READ_EXISTING,
                         &cbFile,
                         &pFile)))
    {
        // check if last-write date/time changed compared
        // to the last time we opened the thing...
        // V0.9.18 (2002-03-24) [umoeller]
        FILESTATUS3 fs3;
        if (!(arc = DosQueryFileInfo(pFile->hf,
                                     FIL_STANDARD,
                                     &fs3,
                                     sizeof(fs3))))
        {
            CHAR szTemp[30];
            dtCreateFileTimeStamp(szTemp,
                                  &fs3.fdateLastWrite,
                                  &fs3.ftimeLastWrite);
            if (strcmp(szTemp, pMsgFile->szTimestamp))
            {
                // last write date changed:
                _Pmpf((__FUNCTION__ ": timestamp changed, recompiling"));
                FreeInternalMem(pMsgFile);

                if (!(arc = LoadAndCompile(pMsgFile, pFile)))
                    strcpy(pMsgFile->szTimestamp, szTemp);
            }
        }

        if (!arc)
        {
            // go find the message in the tree
            PMSGENTRY pEntry;
            if (!(pEntry = (PMSGENTRY)treeFind(pMsgFile->IDsTreeRoot,
                                               (ULONG)pcszMessageName,
                                               treeCompareStrings)))
                arc = ERROR_MR_MID_NOT_FOUND;
            else
            {
                PSZ     pszMsg;
                ULONG   cbRead = pEntry->cbText;

                if (!(pszMsg = (PSZ)malloc(cbRead + 1)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else if (!(arc = doshReadAt(pFile,
                                            pEntry->ulOfsText,
                                            &cbRead,
                                            pszMsg,
                                            DRFL_NOCACHE | DRFL_FAILIFLESS)))
                {
                    // null-terminate
                    pszMsg[cbRead] = '\0';
                    xstrset2(pstr,
                             pszMsg,
                             cbRead);

                    // kick out \r\n
                    xstrConvertLineFormat(pstr,
                                          CRLF2LF);

                    // now replace strings from the table
                    if (cTableEntries && pTable)
                    {
                        CHAR szFind[10] = "%0";
                        ULONG ul;
                        for (ul = 0;
                             ul < cTableEntries;
                             ul++)
                        {
                            ULONG ulOfs = 0;

                            _ultoa(ul + 1, szFind + 1, 10);
                            while (xstrFindReplaceC(pstr,
                                                    &ulOfs,
                                                    szFind,
                                                    pTable[ul]))
                                ;
                        }
                    }
                }
            }
        }

        doshClose(&pFile);
    }

    return arc;
}

/* test case */

#ifdef __TMFDEBUG__

int main(int argc, char *argv[])
{
    APIRET arc;
    PTMFMSGFILE pMsgFile;

    if (argc < 3)
        printf("tmsgfile <file> <msgid>\n");
    else
    {
        if (!(arc = tmfOpenMessageFile(argv[1], &pMsgFile)))
        {
            XSTRING str;
            xstrInit(&str, 0);
            if (!(arc = tmfGetMessage(pMsgFile,
                                      argv[2],
                                      &str,
                                      NULL,
                                      0)))
            {
                printf("String:\n%s", str.psz);
            }
            else
                printf("tmfGetMessage returned %d\n", arc);

            xstrClear(&str);

            tmfCloseMessageFile(&pMsgFile);
        }
        else
            printf("tmfOpenMessageFile returned %d\n", arc);
    }
}

#endif
