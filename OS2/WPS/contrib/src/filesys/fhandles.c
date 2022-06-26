
/*
 *@@sourcefile fhandles.c:
 *      complete file-system object handles management.
 *      This cooperates with XWPFileSystem to replace the WPS's
 *      handles management, while being fully compatible to it.
 *
 *      This file is all new with V0.9.5.
 *
 *      ################ CONSTRUCTION SITE ####################
 *      ## This code is not used in the release version yet. ##
 *      ################ CONSTRUCTION SITE ####################
 *
 *      Function prefix for this file:
 *      --  fhdl*
 *
 *      We only deal with file-system object handles (or file-system
 *      handles, for short) because abstract objects always must
 *      have a handle. Otherwise the WPS wouldn't be able to find
 *      them in OS2.INI.
 *
 *      By contrast, file-system handles are only created on request.
 *      The original idea was to create a file-system object handle
 *      on the first call of wpQueryHandle on a file-system object.
 *      If that was never called, the file-system object never got
 *      a handle.
 *
 *      File-system handles are used in the WPS for the following:
 *
 *      -- Storing folder positions. As a result, a folder receives
 *         an object handle the first time it's opened.
 *
 *      -- Storing references in shadows and program objects. The
 *         shadow stores the object it points to as a handle; the
 *         program object stores the executable name and the startup
 *         directory as a handle, if these objects exist; otherwise
 *         the full path is stored (I think, this is undocumented).
 *
 *      -- Besides, XWorkplace uses object handles for storing
 *         object hotkeys.
 *
 *      -- Any other application (or parts of the WPS) can use
 *         object handles for anything.
 *
 *      To replace file-system object handling, XWorkplace must do
 *      the following:
 *
 *      -- On Desktop startup, load the handles table from OS2SYS.INI.
 *
 *      -- Override wpQueryHandle for file-system objects to do the
 *         following:
 *
 *         -- Return the object handle if it already exists in the table.
 *
 *         -- Create a new object handle if it doesn't exist yet.
 *
 *      -- Override wpclsQueryObject and wpclsObjectFromHandle, which
 *         return a SOM pointer for a given HOBJECT, to search the
 *         internal table instead if the specified handle is a
 *         file-system object handle.
 *
 *      -- Override wpMoveObject for file-system objects to update
 *         the internal table with the new location, if a handle
 *         existed for the object.
 *
 *      -- Override wpFree for file-system objects to delete the
 *         handle from the table, if it exists. Actually, this is
 *         the only new functionality here.
 *
 *@@added V0.9.5 [umoeller]
 *@@header "filesys\fhandles.h"
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

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_WINMESSAGEMGR
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\except.h"             // exception handling
#include "helpers\wphandle.h"           // file-system object handles

// SOM headers which don't crash with prec. header files

// XWorkplace implementation headers

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *  Global variables
 *
 ********************************************************************/

static HMTX    G_hmtxHandles = NULLHANDLE;

/* ******************************************************************
 *
 *  Helpers
 *
 ********************************************************************/

/*
 *@@ fhdlLockHandles:
 *      locks the handles section.
 */

BOOL fhdlLockHandles(ULONG ulTimeout)
{
    if (G_hmtxHandles)
        return !DosRequestMutexSem(G_hmtxHandles,
                                   ulTimeout);

    // first call:
    return !DosCreateMutexSem(NULL,     // unnamed
                              &G_hmtxHandles,
                              0,        // unshared
                              TRUE);
}

/*
 *@@ fhdlUnlockHandles:
 *      unlocks the handles section.
 */

BOOL fhdlUnlockHandles(VOID)
{
    return !DosReleaseMutexSem(G_hmtxHandles);
}

/* ******************************************************************
 *
 *  File-system handles interface
 *
 ********************************************************************/

/*
 *@@ fhdlLoadHandles:
 *      loads or reloads the handles from OS2SYS.INI.
 *      This gets called ONCE at system startup from
 *      M_XWPFileSystem::wpclsInitData.
 */

/* BOOL fhdlLoadHandles(VOID)
{
    BOOL    brc = FALSE;
    BOOL    fLocked = FALSE;

    ULONG   ulNesting = 0;
    DosEnterMustComplete(&ulNesting);

    TRY_LOUD(excpt1)
    {
        fLocked = fhdlLockHandles(SEM_INDEFINITE_WAIT);
        if (fLocked)
        {
            if (G_pHandlesBuffer)
            {
                // already loaded:
                free(G_pHandlesBuffer);
                G_pHandlesBuffer = NULL;
                G_cbHandlesBuffer = 0;
            }

            wphQueryActiveHandles(HINI_SYSTEM,
                                  G_szActiveHandles,
                                  sizeof(G_szActiveHandles));

            if (wphReadAllBlocks(HINI_SYSTEM,
                                 G_szActiveHandles,
                                 &G_pHandlesBuffer,
                                 &G_cbHandlesBuffer))
            {
                // successfully loaded:
                // build hash table
                PBYTE pEnd = G_pHandlesBuffer + G_cbHandlesBuffer;
                // start at beginning of buffer
                PBYTE pCur = G_pHandlesBuffer + 4;

                memset(G_NodeHashTable, 0, sizeof(G_NodeHashTable));

                // now set up hash table
                while (pCur < pEnd)
                {
                    if (!memicmp(pCur, "DRIV", 4))
                    {
                        // pCur points to a DRIVE node:
                        // these never have handles, so skip this
                        PDRIV pDriv = (PDRIV)pCur;
                        pCur += sizeof(DRIV) + strlen(pDriv->szName);
                    }
                    else if (!memicmp(pCur, "NODE", 4))
                    {
                        // pCur points to a regular NODE: offset pointer first
                        PNODE pNode = (PNODE)pCur;
                        // store PNODE in hash table
                        G_NodeHashTable[pNode->usHandle] = pNode;
                        pCur += sizeof (NODE) + pNode->usNameSize;
                    }
                }

                brc = TRUE;
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fLocked)
        fhdlUnlockHandles();

    DosExitMustComplete(&ulNesting);

    return brc;
} */


