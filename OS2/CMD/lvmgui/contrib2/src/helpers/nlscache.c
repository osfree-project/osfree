
/*
 *@@sourcefile nlscache.c:
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  nls*
 *
 *      This file is new with V0.9.19, but contains functions
 *      formerly in nls.c.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\nlscache.h"
 *@@added V0.9.19 (2002-06-13) [umoeller]
 */

/*
 *      Copyright (C) 2001-2005 Ulrich M”ller.
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

#define INCL_DOSNLS
#define INCL_DOSDATETIME
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_WINSHELLDATA
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>

// XWP's setup.h replaces strchr and the like, and
// we want the originals in here
#define DONT_REPLACE_FOR_DBCS
#include "setup.h"                      // code generation and debugging options

#include "helpers/except.h"
#include "helpers/nls.h"
#include "helpers/nlscache.h"
#include "helpers/prfh.h"
#include "helpers/standards.h"
#include "helpers/stringh.h"
#include "helpers/tree.h"
#include "helpers/xstring.h"

#pragma hdrstop

/*
 *@@category: Helpers\National Language Support\String cache
 *      See nls.c.
 */

/* ******************************************************************
 *
 *   NLS string cache
 *
 ********************************************************************/

static HAB                 G_hab = NULLHANDLE;
static HMODULE             G_hmod = NULLHANDLE;
static PCSTRINGENTITY      G_paEntities = NULL;
static ULONG               G_cEntities = 0;

static HMTX        G_hmtxStringsCache = NULLHANDLE;
static TREE        *G_StringsCache;
static LONG        G_cStringsInCache = 0;


/*
 *@@ nlsReplaceEntities:
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 */

ULONG nlsReplaceEntities(PXSTRING pstr)
{
    ULONG ul,
          rc = 0;

    for (ul = 0;
         ul < G_cEntities;
         ul++)
    {
        ULONG ulOfs = 0;
        PCSTRINGENTITY pThis = &G_paEntities[ul];
        while (xstrFindReplaceC(pstr,
                                &ulOfs,
                                pThis->pcszEntity,
                                *(pThis->ppcszString)))
            rc++;
    }

    return rc;
}

/*
 *@@ nlsLoadString:
 *
 *@@changed V0.9.0 [umoeller]: "string not found" is now re-allocated using strdup (avoids crashes)
 *@@changed V0.9.0 (99-11-28) [umoeller]: added more meaningful error message
 *@@changed V0.9.2 (2000-02-26) [umoeller]: made temporary buffer larger
 *@@changed V0.9.16 (2001-09-29) [umoeller]: added entities support
 *@@changed V0.9.16 (2002-01-26) [umoeller]: added pulLength param
 *@@changed V1.0.0 (2002-09-17) [umoeller]: optimized
 *@@changed V1.0.1 (2002-12-11) [umoeller]: moved this here from XWorkplace common.c
 */

VOID nlsLoadString(ULONG ulID,
                   PSZ *ppsz,
                   PULONG pulLength)        // out: length of new string (ptr can be NULL)
{
    CHAR szBuf[500];
    XSTRING str;

    if (*ppsz)
        free(*ppsz);

    if (!WinLoadString(G_hab,
                       G_hmod,
                       ulID,
                       sizeof(szBuf),
                       szBuf))
        // loading failed:
        sprintf(szBuf,
                "LoadString error: string resource %d not found in module 0x%lX",
                ulID,
                G_hmod);

    xstrInitCopy(&str, szBuf, 0);
    nlsReplaceEntities(&str);
    *ppsz = str.psz;
    if (pulLength)
        *pulLength = str.ulLength;
    // do not free string
}

/*
 *@@ LockStrings:
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 */

STATIC BOOL LockStrings(VOID)
{
    BOOL brc = FALSE;

    if (G_hmtxStringsCache == NULLHANDLE)
    {
        brc = !DosCreateMutexSem(NULL,
                                 &G_hmtxStringsCache,
                                 0,
                                 TRUE);
        treeInit(&G_StringsCache,
                 &G_cStringsInCache);
    }
    else
        brc = !DosRequestMutexSem(G_hmtxStringsCache, SEM_INDEFINITE_WAIT);

    return brc;
}

/*
 *@@ UnlockStrings:
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 */

STATIC VOID UnlockStrings(VOID)
{
    DosReleaseMutexSem(G_hmtxStringsCache);
}

/*
 *@@ STRINGTREENODE:
 *      internal string node structure for cmnGetString.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 *@@changed V0.9.16 (2002-01-26) [umoeller]: no longer using malloc() for string
 */

typedef struct _STRINGTREENODE
{
    TREE        Tree;               // tree node (src\helpers\tree.c)
    CHAR        szLoaded[1];        // string that was loaded;
                                    // the struct is dynamic in size now
                                    // V0.9.16 (2002-01-26) [umoeller]
} STRINGTREENODE, *PSTRINGTREENODE;

/*
 *@@ Unload:
 *      removes all loaded strings from memory.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 *@@changed V1.0.1 (2002-12-11) [umoeller]: moved this here from XWorkplace common.c
 */

STATIC VOID Unload(VOID)
{
    // to delete all nodes, build a temporary
    // array of all string node pointers;
    // we don't want to rebalance the tree
    // for each node
    LONG            cNodes = G_cStringsInCache;
    PSTRINGTREENODE *papNodes
        = (PSTRINGTREENODE*)treeBuildArray(G_StringsCache,
                                           &cNodes);
    if (papNodes)
    {
        // delete all nodes in array
        ULONG ul;
        for (ul = 0;
             ul < cNodes;
             ul++)
        {
            free(papNodes[ul]);
        }

        free(papNodes);
    }

    // reset the tree to "empty"
    treeInit(&G_StringsCache,
             &G_cStringsInCache);
}

/*
 *@@ nlsInitStrings:
 *      initializes the NLS strings cache. Call this
 *      before calling nlsGetString for the first time.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

VOID nlsInitStrings(HAB hab,                    // in: anchor block
                    HMODULE hmod,               // in: module handle to load strings from
                    PCSTRINGENTITY paEntities,  // in: entities array or NULL
                    ULONG cEntities)            // in: array item count of paEntities or 0
{
    volatile BOOL    fLocked = FALSE; // XWP V1.0.4 (2005-10-09) [pr]

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockStrings())
        {
            if (G_cStringsInCache)
                // not first call:
                Unload();

            G_hab = hab;
            G_hmod = hmod;
            G_paEntities = paEntities;
            G_cEntities = cEntities;
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        UnlockStrings();
}

/*
 *@@ nlsGetString:
 *      returns a resource NLS string.
 *
 *      Before calling this for the first time, initialize
 *      the engine with nlsInitStrings.
 *
 *      After that, this function implements a fast string
 *      cache for various NLS strings. Compared to the
 *      standard method of preloading all NLS strings at
 *      program startup, this method of on-demand string
 *      loading has the following advantages:
 *
 *      --  Memory is only consumed for strings that are actually
 *          used.
 *
 *      --  Program startup should be a bit faster because we don't
 *          have to load a thousand strings at startup.
 *
 *      --  The memory buffer holding the string is probably close
 *          to the rest of the heap data that the caller allocated,
 *          so this might lead to less memory page fragmentation.
 *          (This is a wild guess though.)
 *
 *      --  To add a new NLS string, before this mechanism existed,
 *          three files had to be changed (and kept in sync): common.h
 *          to add a field to the NLSSTRINGS structure, dlgids.h to
 *          add the string ID, and xfldrXXX.rc to add the resource.
 *          With the new mechanism, there's no need to change common.h
 *          any more, so the danger of forgetting something is a bit
 *          reduced. Anyway, fewer recompiles are needed (maybe),
 *          and sending in patches to the code is a bit easier.
 *
 *      On input, specify a string resouce ID that exists
 *      in the hmod that was given to nlsInitStrings.
 *
 *      The way this works is that the function maintains a
 *      fast cache of string IDs and only loads the string
 *      resources on demand from the given NLS DLL. If a
 *      string ID is queried for the first time, the string
 *      is loaded. Otherwise the cached copy is returned.
 *
 *      There is a slight overhead to this function compared to
 *      simply getting a static string from an array, because
 *      the cache needs to be searched for the string ID. However,
 *      this uses a binary tree (balanced according to string IDs)
 *      internally, so this is quite fast still.
 *
 *      This never releases the strings again.
 *
 *      This never returns NULL. Even if loading the string failed,
 *      a string is returned; in that case, it's a meaningful error
 *      message specifying the ID that failed.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 *@@changed V0.9.16 (2001-10-19) [umoeller]: fixed bad string count which was never set
 *@@changed V0.9.16 (2002-01-26) [umoeller]: optimized heap locality
 *@@changed V1.0.1 (2002-12-11) [umoeller]: moved this here from XWorkplace common.c
 */

PCSZ nlsGetString(ULONG ulStringID)
{
    volatile BOOL    fLocked = FALSE; // XWP V1.0.4 (2005-10-09) [pr]
    PSZ     pszReturn = "Error";

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockStrings())
        {
            PSTRINGTREENODE pNode;

            if (pNode = (PSTRINGTREENODE)treeFind(G_StringsCache,
                                                  ulStringID,
                                                  treeCompareKeys))
                // already loaded:
                pszReturn = pNode->szLoaded;
            else
            {
                // not loaded: load now
                PSZ     psz = NULL;
                ULONG   ulLength = 0;

                nlsLoadString(ulStringID,
                              &psz,
                              &ulLength);

                if (    (!psz)
                     || (!(pNode = (PSTRINGTREENODE)malloc(   sizeof(STRINGTREENODE)
                                                               // has one byte for null
                                                               // terminator already
                                                            + ulLength)))
                   )
                    pszReturn = "malloc() failed.";
                else
                {
                    pNode->Tree.ulKey = ulStringID;
                    memcpy(pNode->szLoaded,
                           psz,
                           ulLength + 1);
                    treeInsert(&G_StringsCache,
                               &G_cStringsInCache,      // fixed V0.9.16 (2001-10-19) [umoeller]
                               (TREE*)pNode,
                               treeCompareKeys);
                    pszReturn = pNode->szLoaded;
                }

                if (psz)
                    free(psz);
            }
        }
        else
            // we must always return a string, never NULL
            pszReturn = "Cannot get strings lock.";
    }
    CATCH(excpt1)
    {
        pszReturn = "Error";
    }
    END_CATCH();

    if (fLocked)
        UnlockStrings();

    return pszReturn;
}

