
/*
 *@@sourcefile trash.c:
 *      this file has implementation code for XWPTrashCan
 *      and XWPTrashObject.
 *
 *      Gets called from src\classes\xtrash.c and
 *      src\classes\xtrashobj.c mostly.
 *
 *      Function prefix for this file:
 *      --  trsh*
 *
 *      This file is ALL new with V0.9.1.
 *
 *@@header "filesys\trash.h"
 */

/*
 *      Copyright (C) 1999-2010 Ulrich M”ller.
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
 *  7)  headers in filesys\ (as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINPOINTERS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#define INCL_WINMLE
#define INCL_WINSHELLDATA
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>
#include <ctype.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xtrash.ih"
#include "xtrashobj.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "filesys\fileops.h"            // file operations implementation
#include "filesys\folder.h"             // XFolder implementation
// #include "filesys\object.h"             // XFldObject implementation
#include "filesys\trash.h"              // trash can implementation
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop
#include "xfobj.h"

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

// name of the mappings file in each \trash directory
#define MAPPINGS_FILE "@mapping"        // note: 8 chars maximum (FAT)!

/*
 *@@ TRASHMAPPINGTREENODE:
 *      entry for the binary tree in G_MappingsTreeRoot.
 *
 *      The tree is sorted according to pszSourceRealName.
 *      While we need to do searches BOTH according to
 *      the name and the SOM folder pointer, I chose the
 *      name as the sort key because string comparisons
 *      are significantly slower than a simple pointer
 *      equality check.
 *
 *      See InitMappings, trshGetMappingFromSource, and
 *      trshGetMappingFromTrashDir.
 *
 *      Assuming that C:\Documents\Important\myfile.txt
 *      gets moved into the trash can. This would result
 *      in the following:
 *
 *      1) Creation of the hidden C:\TRASH directory,
 *         if that doesn't exist yet.
 *
 *      2) Now, instead of creating "C:\TRASH\Documents\Important"
 *         as we used to do, we create a new dir with a number
 *         as its name (e.g. "C:\TRASH\1") and move myfile.txt
 *         there.
 *
 *      3) We then create a TRASHMAPPINGTREENODE for "C:\TRASH\1"
 *         and store "C:\TRASH\Documents\Important" with pcszRealTitle.
 *
 *      If "C:\Documents\Important\myfile2.txt" gets deleted, we
 *      can use that same mapping, since it's the same source
 *      directory.
 *
 *      Now, if the entire parent folder "C:\Documents\Important"
 *      gets deleted, we would create a second mapping "C:\TRASH\2"
 *      so we won't get the "file exists" errors any more.
 *      This allows us to keep deleted files and folders separate.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 *@@changed V0.9.9 (2001-04-07) [umoeller]: completely reworked mappings to not use object handles
 */

typedef struct _TRASHMAPPINGTREENODE
{
    TREE            Tree;
                // base tree node; we use ulKey for pszSourceRealName
    // PSZ             pszSourceRealName;
                // full path of where that folder originally was;
                // for example, if pFolderInTrash is "C:\TRASH\01234567",
                // this could be "C:\Documents\Important".
                // This PSZ is allocated using malloc() and serves
                // as the tree sort key.

    WPFolder        *pFolderInTrash;
                // awake subfolder of \trash. This is a DIRECT subdirectory
                // of \trash on each drive and contains the related
                // objects which were put here. This is ALWAYS HIDDEN.

    ULONG           ulIndex;
                // decimal number representing the name of pFolderInTrash;
                // they are all named with decimals, which we store here also

} TRASHMAPPINGTREENODE, *PTRASHMAPPINGTREENODE;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

/*
 * G_abSupportedDrives:
 *      drives support for trash can.
 *      Index 0 is for drive C:, 1 is for D:, and so on.
 *
 *      Each item can be one of the following:
 *
 *      --  XTRC_SUPPORTED: drive is supported.
 *
 *      --  XTRC_SUPPORTED: drive is not supported.
 *
 *      --  XTRC_INVALID: drive letter doesn't exist.
 */

static BYTE        G_abSupportedDrives[CB_SUPPORTED_DRIVES] = {0};

/*
 *  G_MappingsTreeRoot:
 *      red-black tree (helpers\tree.c) with
 *      TRASHMAPPINGTREENODE entries.
 */

static TREE        *G_MappingsTreeRoot = NULL;
static BOOL        G_fMappingsTreeInitialized = FALSE;

/*
 * G_abMappingDrivesDirty:
 *      array of bytes for each drive. If a byte is 1,
 *      the corresponding drive is "dirty" with respect
 *      to the mappings tree, and the mappings need to
 *      be flushed to disk on the next wpSaveDeferred.
 */

static BYTE        G_abMappingDrivesDirty[CB_SUPPORTED_DRIVES] = {0};

/* ******************************************************************
 *
 *   Trash dir mappings
 *
 ********************************************************************/

/*
 *@@ fnCompareStrings:
 *      tree comparison func (src\helpers\tree.c).
 *
 *@@added V0.9.13 (2001-06-27) [umoeller]
 */

STATIC int TREEENTRY fnCompareStrings(ULONG ul1, ULONG ul2)
{
    return strhicmp((PCSZ)ul1,
                    (PCSZ)ul2);
}

/*
 *@@ CreateMapping:
 *      creates a new TRASHMAPPINGTREENODE with the
 *      specified data.
 *
 *      This also marks the corresponding drive as
 *      "dirty" and calls wpSaveDeferred on the
 *      trash can to force the mappings to be saved
 *      again.
 *
 *      Caller must hold the trash can mutex.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

STATIC PTRASHMAPPINGTREENODE CreateMapping(ULONG ulMappingIndex,      // in: decimal in \trash subdir name
                                           WPFolder *pFolderInTrash,   // in: direct \trash subfolder
                                           PCSZ pcszSourceRealName,   // in: real name of source dir
                                           PBOOL pfNeedSave)   // out: set to TRUE if drives are dirty
                                                // and wpSaveDeferred must be called on the trash can
{
    PTRASHMAPPINGTREENODE pMapping;
    if (pMapping = malloc(sizeof(TRASHMAPPINGTREENODE)))
    {
        ULONG   ulDriveOfs = 0;
        pMapping->ulIndex = ulMappingIndex;
        pMapping->pFolderInTrash  = pFolderInTrash;
        pMapping->Tree.ulKey // pszSourceRealName
            = (ULONG)strdup(pcszSourceRealName);
        if (!treeInsert(&G_MappingsTreeRoot,
                        NULL,
                        (TREE*)pMapping,
                        fnCompareStrings))
        {
            if (pfNeedSave)
            {
                // set the corresponding drive to "dirty"
                ulDriveOfs = pcszSourceRealName[0] - 'C';
                        // 0 for C:, 1 for D:, ...
                G_abMappingDrivesDirty[ulDriveOfs] = 1;

                *pfNeedSave = TRUE;
            }
        }
    }

    return pMapping;
}

/*
 *@@ LoadMappingsForDrive:
 *      called from trshInitMappings to load the
 *      "@mapping" file from each drive.
 *
 *      Caller must hold the trash can's mutex.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

STATIC ULONG LoadMappingsForDrive(M_WPFolder *pFolderClass,
                                  PCSZ pcszTrashDir,    // in: "?:\trash"
                                  PBOOL pfNeedSave)     // out: set to TRUE if mappings are dirty
{
    ULONG   ulrc = 0;

    CHAR    szMapping[CCHMAXPATH];
    PSZ     pszDriveMappings = NULL;
    sprintf(szMapping,
            "%s\\" MAPPINGS_FILE,
            pcszTrashDir);

    if (    (!doshLoadTextFile(szMapping,
                               &pszDriveMappings,
                               NULL))
         && (pszDriveMappings)
       )
    {
        // now, each line in that file looks like this:

        // 123 C:\Documents\FullPath

        PCSZ pThis = pszDriveMappings;
        while (*pThis)
        {
            PSZ pSpace;
            if (pSpace = strchr(pThis, ' '))
            {
                PSZ pEOL = strhFindEOL(pSpace + 1, NULL);
                if (pEOL > pSpace)
                {
                    // awake the folder
                    WPFolder *pFolderInTrash;
                    CHAR szMappedDir[CCHMAXPATH];
                    // zero-terminate the folder name
                    *pSpace = '\0';
                    sprintf(szMappedDir,
                            "%s\\%s",
                            pcszTrashDir,
                            pThis);
                        // now we got:
                        // C:\TRASH\123

                    // awake the folder
                    if (!(pFolderInTrash  = _wpclsQueryFolder(pFolderClass,
                                                              szMappedDir,
                                                              TRUE)))   // lock
                    {
                        ULONG ulDriveOfs = 0;
                        // can't get the folder: this probably no longer
                        // exists...
                        // set the corresponding drive to "dirty"
                        ulDriveOfs = pcszTrashDir[0] - 'C';
                                // 0 for C:, 1 for D:, ...
                        G_abMappingDrivesDirty[ulDriveOfs] = 1;

                        *pfNeedSave = TRUE;
                    }
                    else
                    {
                        // folder still exists:
                        // add the mapping then
                        CHAR    c = *pEOL;
                        *pEOL = '\0';

                        CreateMapping(atoi(pThis),      // decimal
                                      pFolderInTrash,
                                      pSpace + 1,       // source folder name
                                      NULL);            // never save, we're restoring

                        // restore old EOL (can be \n or \0)
                        *pEOL = c;
                        ulrc++;
                    }

                    pThis = pEOL + 1;
                } // end if (pEOL > pSpace)
            } // end if (pSpace)
        } // while (*pThis)

        free(pszDriveMappings);
    }

    return ulrc;
}

/*
 *@@ InitMappings:
 *      this initializes the trash can mappings for
 *      the trash can, if they have not been initialized
 *      yet.
 *
 *      Basically, this initializes the global binary tree
 *      of TRASHMAPPINGTREENODE tree nodes by going over
 *      each supported drive and loading the "@mapping"
 *      file that could have been created by the trash
 *      can if mappings existed for that drive.
 *
 *      For each drive, this calls LoadMappingsForDrive
 *      in turn.
 *
 *      The trash dir mappings roughly work like this:
 *
 *      -- When this function gets called, the "@mapping"
 *         files are loaded from the "\trash" directories
 *         on each drive. Each line in such a file only
 *         consists of the short folder name (the direct
 *         hidden subdirectory of \trash), followed by
 *         a space, followed by the real name of the
 *         source folder.
 *
 *         Example: There is a folder "C:\trash\12345"
 *         which has the following entry in "C:\trash\@mapping":
 *
 +              12345 C:\Documents\Important\Taxes
 *
 *         If an object from "C:\trash\12345" gets
 *         restored, it will be restored to "C:\Documents\Important\Taxes".
 *
 *      -- XWPTrashObject::xwpQueryRelatedPath calls
 *         trshGetMapping in turn to find the real "deleted from"
 *         path for each related object.
 *
 *      -- trshDeleteIntoTrashCan (which is the implementation
 *         for XWPTrashCan::xwpDeleteIntoTrashCan) calls
 *         CreateMapping to create a new mapping for the source
 *         dir, if none exists yet.
 *
 *      The trash mappings are quite efficient. They use a red-black
 *      balanced binary tree internally (see helpers\tree.c) which
 *      uses the folder name as the sort key so a mapping can
 *      very quickly be found from a folder name (trshGetMappingFromSource).
 *      You can also find a mapping from the SOM folder pointer
 *      (trshGetMappingFromTrashDir).
 *
 *      The trash mappings are written back to disk by trshSaveMappings,
 *      which in turn gets called from XWPTrashCan::wpSaveState. We
 *      maintain a separate "dirty" flag for each drive on the system
 *      so that the "@mapping" files will only be written if something
 *      actually has changed. Using wpSaveState gives us sufficient
 *      efficiency in that we don't have to rewrite the mappings files
 *      on every change, but we'll have a couple of seconds of delay
 *      instead.
 *
 *      Note: This implies that if the system crashes before the
 *      mappings have been rewritten, the mappings will get lost.
 *      This does NOT mean however that the related objects will be
 *      lost... the trash can will only lose the information about
 *      the original "deleted from" directories and display the strange
 *      numbers instead. The user can still drag the trash object
 *      to some other location manually.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

STATIC VOID InitMappings(XWPTrashCan *somSelf,
                         PBOOL pfNeedSave)      // out: set to TRUE if drives are dirty
                                         // and wpSaveDeferred must be called on the trash can
{
    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        if (pobjLock = cmnLockObject(somSelf))
        {
            if (!G_fMappingsTreeInitialized)
            {
                // first call: initialize the mappings tree
                M_WPFolder  *pWPFolderClass = _WPFolder;
                BYTE        bIndex = 0;
                CHAR        cDrive = 0;

                treeInit(&G_MappingsTreeRoot, NULL);

                // zero the "dirty" array
                memset(&G_abMappingDrivesDirty, 0, sizeof(G_abMappingDrivesDirty));

                cDrive = 'C';  // (bIndex == 0) = drive C:

                for (bIndex = 0;
                     bIndex < CB_SUPPORTED_DRIVES;
                     bIndex++)
                {
                    if (G_abSupportedDrives[bIndex] == XTRC_SUPPORTED)
                    {
                        // get "\trash" dir on that drive
                        CHAR szTrashDir[50];
                        sprintf(szTrashDir, "%c:\\Trash",
                                cDrive);

                        // first check if that directory exists using CP functions;
                        // otherwise the WPS will initialize the drive which causes
                        // a CHKDSK if the system crashes even though the drive
                        // hasn't really been used
                        if (doshQueryDirExist(szTrashDir))   // V0.9.4 (2000-07-22) [umoeller]
                        {
                            // OK, we have a \trash dir:
                            // load the "@mapping" file from there
                            LoadMappingsForDrive(pWPFolderClass,
                                                 szTrashDir,
                                                 pfNeedSave);
                        }
                    }
                    cDrive++;
                }

                G_fMappingsTreeInitialized = TRUE;
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);
}

/*
 *@@ trshGetMapping:
 *      returns a TRASHMAPPINGTREENODE for the specified
 *      source folder name or NULL if none exists.
 *
 *      See trshInitMappings for an introduction to
 *      the trash can mappings.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

STATIC PTRASHMAPPINGTREENODE trshGetMappingFromSource(XWPTrashCan *pTrashCan,
                                                      PCSZ pcszSourceFolder)
{
    PTRASHMAPPINGTREENODE pMapping = NULL;

    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        if (pobjLock = cmnLockObject(pTrashCan))
        {
            pMapping = (PTRASHMAPPINGTREENODE)treeFind(G_MappingsTreeRoot,
                                                       (ULONG)pcszSourceFolder,
                                                       fnCompareStrings);
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);

    return pMapping;
}

/*
 *@@ trshGetMappingFromTrashDir:
 *      returns the TRASHMAPPINGTREENODE for the specified
 *      awake \trash subfolder.
 *
 *      This can't use the sorting of the tree because
 *      the tree is sorted according to the source folder
 *      names, but since we are not doing string comparisons
 *      here, this seemed OK to me.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

STATIC PTRASHMAPPINGTREENODE trshGetMappingFromTrashDir(XWPTrashCan *pTrashCan,
                                                        WPFolder *pFolderInTrash)
{
    PTRASHMAPPINGTREENODE pMapping = NULL;

    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        if (pobjLock = cmnLockObject(pTrashCan))
        {
            PTRASHMAPPINGTREENODE pNode = (PTRASHMAPPINGTREENODE)treeFirst(G_MappingsTreeRoot);
            while (pNode)
            {
                if (pNode->pFolderInTrash == pFolderInTrash)
                {
                    pMapping = pNode;
                    break;
                }

                pNode = (PTRASHMAPPINGTREENODE)treeNext((TREE*)pNode);
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);

    return pMapping;
}

/*
 *@@ trshFreeMapping:
 *      removes a mapping from the global tree and
 *      frees allocated memory.
 *
 *      See trshInitMappings for an introduction to
 *      the trash can mappings.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

VOID trshFreeMapping(XWPTrashCan *pTrashCan,
                     PTRASHMAPPINGTREENODE pMapping,
                     PBOOL pfNeedSave)       // out: set to TRUE if drives are dirty now
                                             // and wpSaveDeferred must be invoked on the trash can
{
    // BOOL fDirty = FALSE;

    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        if (pobjLock = cmnLockObject(pTrashCan))
        {
            treeDelete(&G_MappingsTreeRoot,
                       NULL,
                       (TREE*)pMapping);
            if (pMapping->Tree.ulKey) // pszSourceRealName)
            {
                // set the corresponding drive to "dirty"
                PSZ pszSourceRealName = (PSZ)pMapping->Tree.ulKey;
                ULONG ulDriveOfs = pszSourceRealName[0] - 'C';
                        // 0 for C:, 1 for D:, ...
                PMPF_TRASHCAN(("setting drive ofs %d dirty", ulDriveOfs));
                G_abMappingDrivesDirty[ulDriveOfs] = 1;

                // now clean up
                free((PSZ)pMapping->Tree.ulKey); // pMapping->pszSourceRealName);
                pMapping->Tree.ulKey = 0; // pMapping->pszSourceRealName = NULL;

                *pfNeedSave = TRUE;
            }

            free(pMapping);
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);
}

/*
 *@@ trshSaveMappings:
 *      saves all dirty mappings back to the "@mapping"
 *      file in the "\trash" dir on each drive.
 *
 *      This ONLY gets called from XWPTrashCan::wpSaveState.
 *
 *      See trshInitMappings for an introduction to
 *      the trash can mappings.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

VOID trshSaveMappings(XWPTrashCan *pTrashCan)
{
    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        if (pobjLock = cmnLockObject(pTrashCan))
        {
            PMPF_TRASHCAN(("Entering"));

            if (G_fMappingsTreeInitialized)
            {
                BYTE bIndex = 0;
                CHAR cDrive = 'C'; // (bIndex == 0) = drive C:

                for (bIndex = 0;
                     bIndex < CB_SUPPORTED_DRIVES;
                     bIndex++, cDrive++)
                {
                    if (G_abMappingDrivesDirty[bIndex])
                    {
                        // this drive is dirty:
                        // compose a new "@mappings" file for this
                        CHAR szMappingsFile[CCHMAXPATH];
                        XSTRING     strMappings;
                        ULONG       cEntries = 0;
                        PTRASHMAPPINGTREENODE pNode;

                        xstrInit(&strMappings, 1000);

                        // now traverse the tree and add all mappings which
                        // belong to this drive
                        pNode = (PTRASHMAPPINGTREENODE)treeFirst(G_MappingsTreeRoot);
                        while (pNode)
                        {
                            PSZ pszSourceRealName = (PSZ)pNode->Tree.ulKey;
                            if (toupper(pszSourceRealName[0]) == cDrive)
                            {
                                // this mapping is for our drive:
                                xstrcat(&strMappings,
                                        _wpQueryTitle(pNode->pFolderInTrash),
                                                // same as filename... we only use decimal digits here
                                        0);
                                xstrcatc(&strMappings, ' ');
                                xstrcat(&strMappings,
                                        pszSourceRealName,
                                        0);
                                xstrcatc(&strMappings, '\n');

                                cEntries++;
                            }

                            pNode = (PTRASHMAPPINGTREENODE)treeNext((TREE*)pNode);
                        }

                        sprintf(szMappingsFile,
                                "%c:\\trash\\" MAPPINGS_FILE,
                                cDrive);

                        if (cEntries)
                            doshWriteTextFile(szMappingsFile,
                                              strMappings.psz,
                                              NULL,
                                              NULL);
                        else
                            DosForceDelete(szMappingsFile);

                        // clean up
                        xstrClear(&strMappings);

                        // unset "dirty" flag
                        G_abMappingDrivesDirty[bIndex] = 0;
                    }
                }
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);
}

/*
 *@@ GetLowestTrashDirDecimal:
 *      returns a decimal number which is not
 *      presently used in the trash mappings.
 *
 *      Caller must hold trash can mutex.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

STATIC ULONG GetLowestTrashDirDecimal(VOID)
{
    ULONG ul = 0;
    PTRASHMAPPINGTREENODE pNode = (PTRASHMAPPINGTREENODE)treeFirst(G_MappingsTreeRoot);
    while (pNode)
    {
        if (pNode->ulIndex == ul)
        {
            // this one's already used:
            // try next
            ul++;
            // start over
            pNode = (PTRASHMAPPINGTREENODE)treeFirst(G_MappingsTreeRoot);
        }
        else
            pNode = (PTRASHMAPPINGTREENODE)treeNext((TREE*)pNode);
    }

    return ul;
}

/*
 *@@ trshGetOrCreateTrashDir:
 *      returns a subdirectory of \trash on the
 *      drive where pcszSourceFolder resides,
 *      which corresponds to pcszSourceFolder.
 *
 *      If a matching trash dir already exists,
 *      it is returned.
 *
 *      Otherwise a new directory and a corresponding
 *      internal mapping is created, and the new
 *      folder is returned.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

WPFolder* trshGetOrCreateTrashDir(XWPTrashCan *pTrashCan,
                                  PCSZ pcszSourceFolder, // in: real name of source folder
                                  PBOOL pfNeedSave)
{
    WPFolder    *pFolderInTrash = NULL;
    PTRASHMAPPINGTREENODE pMapping;

    // initialize the mappings if this hasn't been done yet
    InitMappings(pTrashCan, pfNeedSave);

    // check if we have a mapping for the source folder yet
    if (pMapping = trshGetMappingFromSource(pTrashCan,
                                            pcszSourceFolder))
        // got one: return the folder
        pFolderInTrash = pMapping->pFolderInTrash;
    else
    {
        // no: create a new trash dir...

        WPObject *pobjLock = NULL;
        TRY_LOUD(excpt1)
        {
            if (pobjLock = cmnLockObject(pTrashCan))
            {
                // 1) we need a unique decimal as the name
                //    of the trash dir; we used to be using
                //    the source folder's HOBJECT, but this
                //    led to excessive handle creation for
                //    the \trash subdirectories... instead,
                //    we now always try to use low decimal
                //    numbers so that handles are not necessarily
                //    recreated

                ULONG   ulDecimal = GetLowestTrashDirDecimal();
                CHAR    szSubdirOfTrash[CCHMAXPATH];

                sprintf(szSubdirOfTrash,
                        "%c:\\Trash\\%d",
                        *pcszSourceFolder,      // drive letter from source
                        ulDecimal);             // rest of path

                if (!doshQueryDirExist(szSubdirOfTrash))
                    // does not exist yet:
                    // create \trash subdirectory
                    doshCreatePath(szSubdirOfTrash,
                                   TRUE);   // hide that directory

                // 2) awake the \trash subdir now
                if (pFolderInTrash = _wpclsQueryFolder(_WPFolder,
                                                       szSubdirOfTrash,
                                                       TRUE))       // lock object
                {
                    // 3) create a mapping
                    pMapping = CreateMapping(ulDecimal,
                                             pFolderInTrash,
                                             pcszSourceFolder,
                                             pfNeedSave);
                }
            }
        }
        CATCH(excpt1) {} END_CATCH();

        if (pobjLock)
            _wpReleaseObjectMutexSem(pobjLock);
    }

    return pFolderInTrash;
}

/* ******************************************************************
 *
 *   Trash object creation
 *
 ********************************************************************/

/*
 *@@ trshCreateTrashObjectsList:
 *      this creates a new linked list (PLINKLIST, linklist.c,
 *      which is returned) containing all the trash objects in
 *      the specified trash can.
 *
 *      The list's item data pointers point to the XWPTrashObject*
 *      instances directly. Note that each item's related object
 *      on the list can possibly be a folder containing many more
 *      objects.
 *
 *      The list is created in any case, even if the trash can is
 *      empty. lstFree must therefore always be used to free this list.
 *
 *      If (fRelatedObjects == TRUE), not the trash objects, but
 *      the related objects will be added to the list instead.
 *
 *@@changed V0.9.3 (2000-04-28) [umoeller]: now pre-resolving wpQueryContent for speed
 *@@changed V0.9.3 (2000-04-28) [umoeller]: added fRelatedObjects
 */

PLINKLIST trshCreateTrashObjectsList(XWPTrashCan* somSelf,
                                     BOOL fRelatedObjects,
                                     PULONG pulCount)   // out: no. of objects on list
{
    ULONG       cObjects = 0;
    PLINKLIST   pllTrashObjects = lstCreate(FALSE);
                                // FALSE: since we store the XWPTrashObjects directly,
                                // the list node items must not be free()'d
    if (pllTrashObjects)
    {
        XWPTrashObject* pTrashObject = 0;
        // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
        for (   pTrashObject = _wpQueryContent(somSelf, NULL, (ULONG)QC_FIRST);
                (pTrashObject);
                pTrashObject = *__get_pobjNext(pTrashObject)
            )
        {
            // pTrashObject now has the XWPTrashObject to delete
            lstAppendItem(pllTrashObjects,
                          (fRelatedObjects)
                            ? _xwpQueryRelatedObject(pTrashObject)
                            : pTrashObject);
            cObjects++;
        }
    }

    if (pulCount)
        *pulCount = cObjects;

    return pllTrashObjects;
}

/*
 *@@ trshCreateTrashObject:
 *      this creates a new transient "trash object" in the
 *      given trashcan by invoking wpclsNew upon the XWPTrashObject
 *      class object (somSelf of this function).
 *
 *      This returns the new trash object. If NULL is returned,
 *      either an error occurred creating the trash object or
 *      a trash object with the same pRelatedObject already
 *      existed in the trash can.
 *
 *      WARNING: This does not move the related object to a TRASH
 *      directory.
 *      This function is only used internally by XWPTrashCan to
 *      map the contents of the "\Trash" directories into the
 *      open trashcan, either when the trash can is first populated
 *      (trshPopulateFirstTime) or later, when objects are moved
 *      into the trash can. So do not call this function manually.
 *
 *      To move an object into a trashcan, call
 *      XWPTrashCan::xwpDeleteIntoTrashCan, which automatically
 *      determines whether this function needs to be called.
 *
 *      There is no opposite concept of deleting a trash object.
 *      Instead, delete the related object, which will automatically
 *      destroy the trash object thru XFldObject::wpUnInitData.
 *
 *      Postconditions:
 *
 *      The related object is locked by this function to make sure
 *      it is never made dormant because the trash object will
 *      access it frequently, e.g. for the icon.
 *
 *@@changed V0.9.1 (2000-01-31) [umoeller]: this used to be M_XWPTrashObject::xwpclsCreateTrashObject
 *@@changed V0.9.3 (2000-04-09) [umoeller]: only folders are calculated on File thread now
 *@@changed V0.9.6 (2000-10-25) [umoeller]: now locking the related object
 */

XWPTrashObject* trshCreateTrashObject(M_XWPTrashObject *somSelf,
                                      XWPTrashCan* pTrashCan, // in: the trashcan to create the object in
                                      WPObject* pRelatedObject) // in: the object which the new trash object
                                                                // should represent
{
    XWPTrashObject *pTrashObject = NULL;

    if (    (pTrashCan)
         && (pRelatedObject)
       )
    {
        CHAR    szSetupString[300];

        PMPF_TRASHCAN(("Creating trash object \"%s\" in \"%s\"",
                    _wpQueryTitle(pRelatedObject),
                    _wpQueryTitle(pTrashCan)));

        _wpLockObject(pRelatedObject);

        // create setup string; we pass the related object
        // as an address string directly. Looks ugly, but
        // it's the only way the trash object can know about
        // the related object immediately during creation.
        // Otherwise the object details are not fully set
        // on creation, and the trash can's Details view
        // flickers like crazy. Duh.
        sprintf(szSetupString,
                "RELATEDOBJECT=%lX",
                pRelatedObject);

        // create XWPTrashObject instance;
        // XWPTrashObject::wpSetupOnce handles the rest
        pTrashObject = _wpclsNew(somSelf,   // class object
                                 _wpQueryTitle(pRelatedObject),
                                        // same title as related object
                                 szSetupString, // setup string
                                 pTrashCan, // where to create the object
                                 TRUE);  // lock trash object too
    }

    return pTrashObject;
}

/*
 *@@ trshSetupOnce:
 *      implementation for XWPTrashObject::wpSetupOnce.
 *      This parses the RELATEDOBJECT setup string
 *      to set the related object accordingly, which has
 *      been given to wpclsNew by trshCreateTrashObject.
 *
 *      Returns FALSE upon errors, which is then passed on
 *      as the return value of wpSetupOnce to abort object
 *      creation.
 *
 *      Preconditions: Whoever uses RELATEDOBJECT must lock
 *      the object before using this.
 *
 *@@added V0.9.3 (2000-04-09) [umoeller]
 */

BOOL trshSetupOnce(XWPTrashObject *somSelf,
                   PSZ pszSetupString)
{
    CHAR    szRelatedAddress[100];
    ULONG   cbRelatedAddress = sizeof(szRelatedAddress);

    // parse "RELATEDOBJECT="; this has the SOM object pointer
    // in hexadecimal
    if (_wpScanSetupString(somSelf,
                           pszSetupString,
                           "RELATEDOBJECT",
                           szRelatedAddress,
                           &cbRelatedAddress))
    {
        // found:
        WPObject *pRelatedObject;
        XWPTrashCan *pTrashCan;

        // sscanf(szRelatedAddress, "%lX", &pRelatedObject);
                // optimized V0.9.20 (2002-08-04) [umoeller]

        if (    (pRelatedObject = (WPObject *)strtoul(szRelatedAddress, NULL, 16))
             && (pTrashCan = _wpQueryFolder(somSelf))
           )
        {
            // store related object in trash object
            _xwpSetRelatedObject(somSelf, pRelatedObject);

            // store trash object in related object;
            // this will cause the trash object to be freed
            // when the related object gets destroyed
            _xwpSetTrashObject(pRelatedObject, somSelf);

            // raise trash can "busy" flag by one;
            // this is decreased by trshCalcTrashObjectSize
            // again
            _xwpTrashCanBusy(pTrashCan,
                             +1);        // inc busy

            if (_somIsA(pRelatedObject, _WPFolder))
                // related object is a folder:
                // this can have many objects, so have size
                // of trash object calculated on File thread;
                // this will update the details later
                xthrPostFileMsg(FIM_CALCTRASHOBJECTSIZE,
                                (MPARAM)somSelf,
                                (MPARAM)pTrashCan);
            else
                // non-folder:
                // set size synchronously
                trshCalcTrashObjectSize(somSelf,
                                        pTrashCan);

            return TRUE;
        }
    }

    return FALSE;
}

/*
 *@@ trshCalcTrashObjectSize:
 *      implementation for the FIM_CALCTRASHOBJECTSIZE
 *      message on the File thread. This gets called
 *      after a trash object has been created (trshSetupOnce,
 *      while in trshCreateTrashObject) in two situations:
 *
 *      -- on the File thread if the trash object's related
 *         object is a folder, because we need to recurse into
 *         the subfolders then;
 *
 *      -- synchronously by trshSetupOnce directly if the object
 *         is not a folder, because then querying the size is
 *         fast.
 *
 *      This invokes XWPTrashObject::xwpSetExpandedObjectSize
 *      on the trash object.
 *
 *@@added V0.9.2 (2000-02-28) [umoeller]
 *@@changed V0.9.6 (2000-10-25) [umoeller]: now doing a much faster object count
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

VOID trshCalcTrashObjectSize(XWPTrashObject *pTrashObject,
                             XWPTrashCan *pTrashCan)
{
    if (pTrashObject)
    {
        WPObject *pRelatedObject = _xwpQueryRelatedObject(pTrashObject);
        // create structured file list for this object;
        // if this is a folder, this can possibly take a
        // long time
        PEXPANDEDOBJECT pSOI = fopsExpandObjectDeep(pRelatedObject,
                                                    TRUE);  // folders only
        if (pSOI)
        {
            _xwpSetExpandedObjectSizeL(pTrashObject,
                                       &pSOI->llSizeThis,  // V1.0.9
                                       pTrashCan);
            fopsFreeExpandedObject(pSOI);
        }
    }

    _xwpTrashCanBusy(pTrashCan,
                     -1);        // dec busy
}

/*
 *@@ trshComposeRelatedPath:
 *      implementation for the first call of
 *      XWPTrashObject::xwpQueryRelatedPath.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 *@@changed V1.0.1 (2003-01-30) [umoeller]: optimized
 */

PSZ trshComposeRelatedPath(XWPTrashObject *somSelf)
{
    XWPTrashObjectData *somThis = XWPTrashObjectGetData(somSelf);

    PMPF_TRASHCAN(("    pRelatedObject: 0x%lX", _pRelatedObject));

    if (_pRelatedObject)
    {
        WPFolder *pTrashDir = _wpQueryFolder(_pRelatedObject);

        PMPF_TRASHCAN(("    pTrashDir: 0x%lX", pTrashDir));

        if (pTrashDir)
        {
            // get trash can (must be folder of trash object)
            BOOL        fNeedRefresh = FALSE;
            XWPTrashCan *pTrashCan = _wpQueryFolder(somSelf);

            // find the mapping for this
            PTRASHMAPPINGTREENODE pMapping;
            if (pMapping = trshGetMappingFromTrashDir(pTrashCan,
                                                      pTrashDir))
            {
                // we have a mapping: use that
                _pszSourcePath = strdup((PSZ)pMapping->Tree.ulKey); // pszSourceRealName);
            }
            else
            {
                // we have no mapping: use the folder's name
                CHAR szPathInTrash[CCHMAXPATH];
                if (_wpQueryFilename(pTrashDir, szPathInTrash, TRUE))
                {
                    // CHAR    szSourcePath[CCHMAXPATH];

                    // optimized the following V1.0.1 (2003-01-30) [umoeller]

                    // copy stuff after "?:\Trash", including the leading '\'
                    ULONG   len = strlen(szPathInTrash + 8);

                    if (_pszSourcePath = malloc(len + 2))
                    {
                        // copy drive letter
                        _pszSourcePath[0] = szPathInTrash[0];
                        // copy ':'
                        _pszSourcePath[1] = ':';
                        // copy stuff after "?:\Trash"
                        memcpy(_pszSourcePath + 2,
                               szPathInTrash + 8,
                               len + 1);
                    }

                    PMPF_TRASHCAN(("    szPathInTrash: %s", STRINGORNULL(_pszSourcePath)));
                }
            }

            if (fNeedRefresh)
                _wpSaveDeferred(pTrashCan);

        } // end if (pTrashDir)
    }

    return _pszSourcePath;
}

/* ******************************************************************
 *
 *   Trash can populating
 *
 ********************************************************************/

/*
 *@@ AddTrashObjectsForTrashDir:
 *      this routine gets called from trshPopulateFirstTime
 *      for each "?:\Trash" directory which exists on all
 *      drives. pTrashDir must be the corresponding WPFolder
 *      object for that directory.
 *
 *      We then query the folder's contents and create trash
 *      objects in pTrashCan accordingly.
 *
 *      Note: we used to recurse if another folder is found which
 *      was hidden. This should now only occur any longer on the
 *      first recursion level, i.e. when this function is called
 *      for the main \trash directory, because we no longer create
 *      "deep" paths in the \trash directory.
 *
 *      This returns the total number of trash objects that were
 *      created.
 *
 *@@changed V0.9.1 (2000-01-29) [umoeller]: this returned 0 always; fixed, so that subdirs won't be deleted always
 *@@changed V0.9.3 (2000-04-11) [umoeller]: now returning BOOL
 *@@changed V0.9.3 (2000-04-25) [umoeller]: deleting empty TRASH directories finally works
 *@@changed V0.9.3 (2000-04-28) [umoeller]: now pre-resolving wpQueryContent for speed
 *@@changed V0.9.5 (2000-08-25) [umoeller]: object count was wrong
 *@@changed V0.9.9 (2001-02-06) [umoeller]: added trash dir mappings
 */

STATIC BOOL AddTrashObjectsForTrashDir(M_XWPTrashObject *pXWPTrashObjectClass, // in: _XWPTrashObject (for speed)
                                       XWPTrashCan *pTrashCan,  // in: trashcan to add objects to
                                       WPFolder *pTrashDir,     // in: trash directory to examine
                                       PBOOL pfNeedSave,     // out: set to TRUE if mappings are dirty and
                                                                // wpSaveDeferred is needed
                                       PULONG pulObjectCount)   // out: object count (req. ptr)
{
    BOOL        brc = FALSE,
                fTrashDirFolderLocked = FALSE;
    WPObject    *pObject;

    LINKLIST    llEmptyDirs;        // list of WPFolder's which are to be freed
                                    // because they're empty
    ULONG       ulTrashObjectCountSub = 0;

    if (!pXWPTrashObjectClass)
        // error
        return 0;

    PMPF_TRASHCAN(("  Entering AddTrashObjectsForTrashDir for %s", _wpQueryTitle(pTrashDir)));

    lstInit(&llEmptyDirs, FALSE);       // no auto-free items, these are WPFOlder* pointers

    TRY_LOUD(excpt1)
    {
        // populate
        if (fdrCheckIfPopulated(pTrashDir,
                                FALSE))        // full populate
        {
            // populated:

            // request semaphore for that trash dir
            // to protect the contents list
            if (fTrashDirFolderLocked = !_wpRequestFolderMutexSem(pTrashDir, 4000))
            {
                // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
                for (   pObject = _wpQueryContent(pTrashDir, NULL, (ULONG)QC_FIRST);
                        (pObject);
                        pObject = *__get_pobjNext(pObject)
                    )
                {
                    BOOL    fAddTrashObject = TRUE;
                    if (_somIsA(pObject, _WPFileSystem))
                    {
                        // FS object found:
                        // get the file name first
                        CHAR    szFSPath[2*CCHMAXPATH] = "";
                        ULONG   cbFSPath = sizeof(szFSPath);
                        ULONG   ulAttrs = 0;
                        if (_wpQueryRealName(pObject,
                                             szFSPath,
                                             &cbFSPath,
                                             TRUE))     // fully q'fied
                        {
                            BOOL fIsFolder = (_somIsA(pObject, _WPFolder));
                            if (    (fIsFolder)
                                 && (doshQueryPathAttr(szFSPath, &ulAttrs)
                                        == NO_ERROR)
                               )
                            {
                                // it's a folder, and it still exists:
                                // check if it's hidden... if so, it is
                                // most probably one of the pseudo-folders
                                // in the trash can, for which we have added
                                // a mapping on "delete"
                                if (ulAttrs & FILE_HIDDEN)
                                {
                                    // hidden directory: this is a trash directory!

                                    PMPF_TRASHCAN(("    Recursing with %s", _wpQueryTitle(pObject)));

                                    brc = AddTrashObjectsForTrashDir(pXWPTrashObjectClass,
                                                                     pTrashCan,
                                                                     pObject, // new trash dir
                                                                     pfNeedSave,
                                                                     &ulTrashObjectCountSub);

                                    PMPF_TRASHCAN(("    Recursion returned %d objects", ulTrashObjectCountSub));

                                    if (ulTrashObjectCountSub == 0)
                                    {
                                        // no objects found in this trash folder
                                        // or subfolders (if any):
                                        // delete this folder, it's useless,
                                        // but we can only do this outside the wpQueryContent
                                        // loop, so delay this
                                        lstAppendItem(&llEmptyDirs,
                                                      pObject); // the empty WPFolder

                                        PMPF_TRASHCAN(("    Adding %s to dirs 2be deleted",
                                                    _wpQueryTitle(pObject)));
                                    }

                                    // don't create a trash object for this directory...
                                    fAddTrashObject = FALSE;
                                }
                            } // end if (    (_somIsA(pObject, _WPFolder)...
                            else if (!fIsFolder)
                            {
                                // ignore the "@mapping" file that is used
                                // in the trash root
                                if (!stricmp(&szFSPath[1],
                                             ":\\TRASH\\" MAPPINGS_FILE))
                                    fAddTrashObject = FALSE;
                            }

                        }
                    } // end if (_somIsA(pObject, _WPFileSyste))

                    if (fAddTrashObject)
                    {
                        // non-folder or folder which is not hidden:
                        // add to trashcan!
                        PMPF_TRASHCAN(("    Adding %s...",
                                        _wpQueryTitle(pObject)));

                        trshCreateTrashObject(pXWPTrashObjectClass,
                                              pTrashCan,
                                              pObject);  // related object
                        (*pulObjectCount)++;

                        PMPF_TRASHCAN(("    *pulObjectCount is now %d",
                               (*pulObjectCount)));
                    }
                } // end for (   pObject = _wpQueryContent(...
            } // end if (fTrashDirFolderLocked)
            else
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "Couldn't request mutex semaphore for \\trash subdir.");
        } // end if (fdrCheckIfPopulated(pTrashDir))
        else
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "wpPopulate failed for \\trash subdir.");
    }
    CATCH(excpt1)
    {
        // exception occurred:
        brc = FALSE;        // report error
    } END_CATCH();

    if (fTrashDirFolderLocked)
        _wpReleaseFolderMutexSem(pTrashDir);

    *pulObjectCount += ulTrashObjectCountSub;

    // OK, now that we're done running thru the subdirectories,
    // delete the empty ones before returning to the caller
    {
        PLISTNODE   pDirNode = lstQueryFirstNode(&llEmptyDirs);
        while (pDirNode)
        {
            WPFolder *pFolder = (WPFolder*)pDirNode->pItemData;

            // free the mapping, if one exists
            PTRASHMAPPINGTREENODE pMapping = trshGetMappingFromTrashDir(pTrashCan,
                                                                        pFolder);
            if (pMapping)
                trshFreeMapping(pTrashCan,
                                pMapping,
                                pfNeedSave);

            // free the folder
            PMPF_TRASHCAN(("    Freeing empty folder %s", _wpQueryTitle(pFolder)));

            _wpFree(pFolder);

            pDirNode = pDirNode->pNext;
        }

        lstClear(&llEmptyDirs);
    }

    PMPF_TRASHCAN(("  End of AddTrashObjectsForTrashDir for %s;", _wpQueryTitle(pTrashDir)));
    PMPF_TRASHCAN(("              returning %d objects ", (*pulObjectCount)));

    return brc;
}

/*
 *@@ trshPopulateFirstTime:
 *      this gets called when XWPTrashCan::wpPopulate gets
 *      called for the very first time only. In that case,
 *      we need to go over all supported drives and add
 *      trash objects according to the \TRASH directories.
 *
 *      For subsequent wpPopulate calls, this is not
 *      necessary because the trash objects persist while
 *      the WPS is running.
 *
 *@@added V0.9.1 (2000-01-31) [umoeller]
 *@@changed V0.9.1 (2000-02-04) [umoeller]: added status bar updates while populating
 *@@changed V0.9.5 (2000-08-25) [umoeller]: now deleting empty trash dirs
 *@@changed V0.9.5 (2000-08-27) [umoeller]: fixed object counts
 *@@changed V0.9.7 (2001-01-17) [umoeller]: this returned FALSE always, which stopped shutdown... fixed
 *@@changed V0.9.9 (2001-02-06) [umoeller]: added trash dir mappings
 *@@changed V0.9.9 (2001-04-02) [umoeller]: now using fast populate
 */

BOOL trshPopulateFirstTime(XWPTrashCan *somSelf,
                           ULONG ulFldrFlags)
{
    BOOL            brc = TRUE;     // fixed V0.9.7 (2001-01-17) [umoeller]
    BOOL            fNeedSave = FALSE;
    XWPTrashCanData *somThis = XWPTrashCanGetData(somSelf);

    TRY_LOUD(excpt1)
    {
        _wpSetFldrFlags(somSelf, ulFldrFlags | FOI_POPULATEINPROGRESS);

        // populate with all:
        if ((ulFldrFlags & FOI_POPULATEDWITHALL) == 0)
        {
            CHAR        szTrashDir[30],
                        cDrive;
            BYTE        bIndex;     // into G_abSupportedDrives[]

            M_WPFolder  *pWPFolderClass = _WPFolder;
            M_XWPTrashObject *pXWPTrashObjectClass = _XWPTrashObject;

            // now go over all drives and create
            // trash objects for all the objects
            // in the "\Trash" directories.
            // Note that M_XWPTrashObject::xwpclsCreateTrashObject
            // will automatically check for whether a trash
            // object exists for a given related object.

            cDrive = 'C';  // (bIndex == 0) = drive C:

            // load all the mappings, if this hasn't been done yet
            InitMappings(somSelf,
                         &fNeedSave);

            for (bIndex = 0;
                 bIndex < CB_SUPPORTED_DRIVES;
                 bIndex++)
            {
                if (G_abSupportedDrives[bIndex] == XTRC_SUPPORTED)
                {
                    // drive supported:
                    WPFolder    *pTrashDir;

                    // update status bars for open views
                    _cDrivePopulating = cDrive;
                    stbUpdate(somSelf);

                    // get "\trash" dir on that drive
                    sprintf(szTrashDir, "%c:\\Trash",
                            cDrive);

                    PMPF_TRASHCAN(("  trshPopulateFirstTime: Getting trash dir %s", szTrashDir));

                    // first check if that directory exists using CP functions;
                    // otherwise the WPS will initialize the drive which causes
                    // a CHKDSK if the system crashes even though the drive
                    // hasn't really been used
                    if (doshQueryDirExist(szTrashDir))   // V0.9.4 (2000-07-22) [umoeller]
                    {
                        // OK, we have a \trash dir:

                        // now go populate all that
                        pTrashDir = _wpclsQueryFolder(pWPFolderClass,   // _WPFolder
                                                      szTrashDir,
                                                      TRUE);       // lock object
                        if (pTrashDir)
                        {
                            ULONG ulObjectCount = 0;
                            // "\Trash" exists for this drive;
                            AddTrashObjectsForTrashDir(pXWPTrashObjectClass, // _XWPTrashObject
                                                       somSelf,     // trashcan to add objects to
                                                       pTrashDir,   // initial trash dir;
                                                       &fNeedSave,
                                                       &ulObjectCount);
                                   // this routine will recurse

                            PMPF_TRASHCAN(("got %d objects for %s", ulObjectCount, szTrashDir));

                            if (ulObjectCount == 0)
                            {
                                PMPF_TRASHCAN(("    is empty, deleting!"));

                                // no trash objects found:
                                _wpFree(pTrashDir);
                            }
                        }
                    }
                }
                cDrive++;
            }

            PMPF_TRASHCAN(("Added new trash objects, now %d", _ulTrashObjectCount));

            ulFldrFlags |= (FOI_POPULATEDWITHFOLDERS
                              | FOI_POPULATEDWITHALL);
        }
    } CATCH(excpt1) { } END_CATCH();

    _cDrivePopulating = 0;
    stbUpdate(somSelf);

    // now insert all the trash objects in one flush V0.9.9 (2001-04-02) [umoeller]
    fdrInsertAllContents(somSelf);

    ulFldrFlags &= ~FOI_POPULATEINPROGRESS;
    _wpSetFldrFlags(somSelf, ulFldrFlags);

    if (fNeedSave)
        _wpSaveDeferred(somSelf);

    return brc;
}

/*
 *@@ trshRefresh:
 *      implementation for XWPTrashCan::wpRefresh.
 *      This calls XWPTrashObject::xwpValidateTrashObject
 *      on each trash object in the trash can.
 *
 *@@added V0.9.1 (2000-01-31) [umoeller]
 */

BOOL trshRefresh(XWPTrashCan *somSelf)
{
    // create list of all trash objects
    PLINKLIST   pllTrashObjects = trshCreateTrashObjectsList(somSelf,
                                                             FALSE, // trash objects
                                                             NULL);
    PLISTNODE   pNode = lstQueryFirstNode(pllTrashObjects);

    // now go thru the list
    while (pNode)
    {
        XWPTrashObject* pTrashObject = (XWPTrashObject*)pNode->pItemData;

        _xwpValidateTrashObject(pTrashObject);
        // else error: means that item has been destroyed

        // go for next
        pNode = pNode->pNext;
    }
    lstFree(&pllTrashObjects);

    stbUpdate(somSelf);

    return TRUE;
}

/* ******************************************************************
 *
 *   Trash can / trash object operations
 *
 ********************************************************************/

/*
 *@@ trshDeleteIntoTrashCan:
 *      implementation for XWPTrashCan::xwpDeleteIntoTrashCan.
 *
 *      When an object is thus "deleted" into the trashcan,
 *      this function does the following:
 *
 *      1)  create a hidden directory "\Trash" on the drive
 *          where the object resides, if that directory doesn't
 *          exist already;
 *
 *      2)  create a subdirectory in "\Trash" which represents
 *          the source path of the object.
 *
 *          Note: we used to create a path according to the path of
 *          the object; i.e., if "F:\Tools\XFolder\xfldr.dll"
 *          is moved into the trash can, "F:\Trash\Tools\XFolder"
 *          will be created.
 *
 *          This implementation has changed with V0.9.9 to avoid
 *          two problems:
 *
 *          -- With the old implementation, if an object was first
 *             deleted and then afterwards the folder in which the
 *             object used to reside, this caused the "file exists"
 *             dialog to pop up because the target subdirectory of
 *             "\trash" already existed. This is solved now.
 *
 *          -- Creating such a "deep path" in "\trash" caused
 *             excessive creation of WPS file-system handles
 *             if the object to be deleted had a file-system
 *             handle assigned because the WPS needed to create
 *             parent handles for each path component of that
 *             "deep path".
 *
 *          So instead, we now create a hidden folder with a
 *          number as its name (e.g. C:\trash\1) and rather
 *          add a mapping for that.
 *
 *          As a result, the concept of "trash directory mappings"
 *          had to be introduced. See InitMappings for an
 *          introduction. CreateMapping gets called by this function.
 *
 *      3)  Move the object which is being deleted into that
 *          new directory (using wpMoveObject, so that all WPS
 *          shadows etc. remain valid);
 *
 *      4)  create a new instance of XWPTrashObject in the
 *          trash can (somSelf) which should represent the
 *          object by calling trshCreateTrashObject.
 *          However, this is only done if the trash can has
 *          already been populated (otherwise we'd get duplicate
 *          trash objects in the trash can when populating).
 *
 *      This returns FALSE upon errors.
 *
 *@@added V0.9.1 (2000-02-03) [umoeller]
 *@@changed V0.9.4 (2000-06-17) [umoeller]: now closing folder subviews properly
 *@@changed V0.9.9 (2001-02-06) [umoeller]: added trash dir mappings
 *@@changed V0.9.9 (2001-04-07) [umoeller]: completely reworked mappings to not use object handles
 */

BOOL trshDeleteIntoTrashCan(XWPTrashCan *pTrashCan, // in: trash can where to create trash object
                            WPObject *pObject)      // in: object to delete
{
    BOOL        fNeedSave = FALSE;
    BOOL        brc = FALSE;
    WPFolder    *pSourceFolder,
                *pFolderInTrash;
    CHAR        szSourceFolder[CCHMAXPATH];

    if (    (pObject)
         && (pSourceFolder = _wpQueryFolder(pObject))
         && (_wpQueryFilename(pSourceFolder, szSourceFolder, TRUE))
       )
    {
        PMPF_TRASHCAN(("Source folder: %s", szSourceFolder));

        // get the \trash subdir corresponding to pSourceFolder;
        // this function either creates one or gets an existing
        // one if things have been deleted from pSourceFolder
        // already
        if (    (pFolderInTrash = trshGetOrCreateTrashDir(pTrashCan,
                                                          szSourceFolder,
                                                          &fNeedSave))
                // close all open views
             && (wpshCloseAllViews(pObject))
           )
        {
            // MOVE the object now
            if (fopsMoveObjectConfirmed(pObject,
                                        pFolderInTrash))
            {
                // successfully moved:
                XWPTrashCanData *somThis = XWPTrashCanGetData(pTrashCan);

                // set original object's deletion data
                // to current date/time
                _xwpSetDeletion(pObject, TRUE);

                // return TRUE
                brc = TRUE;

                // if the trash can has been populated
                // already, add a matching trash object;
                // otherwise wpPopulate will do this
                // later
                if (_fAlreadyPopulated)
                {
                    SOMClass *pTrashObjectClass = _XWPTrashObject;
                    PMPF_TRASHCAN(("Trash can is populated: creating trash object"));
                    PMPF_TRASHCAN(("pTrashObjectClass: 0x%lX", pTrashObjectClass));
                    if (pTrashObjectClass)
                    {
                        if (trshCreateTrashObject(pTrashObjectClass,
                                                  pTrashCan,    // trash can
                                                  pObject))
                            PMPF_TRASHCAN(("Created trash object successfully"))
                            ;
                    }
                }
                else
                {
                    // not populated yet:
                    PMPF_TRASHCAN(("Trash can not populated, skipping trash object"));

                    // just raise the number of trash items
                    // and change the icon, wpPopulate will
                    // later correct this number
                    _ulTrashObjectCount++;
                    _xwpSetCorrectTrashIcon(pTrashCan, FALSE);

                }
            } // end if (fopsMoveObject(pObject, ...
        } // end if (wpshCloseAllViews(pObject))
    } // end if (_wpQueryFilename(pFolder, szFolder, TRUE))

    if (fNeedSave)
        _wpSaveDeferred(pTrashCan);

    return brc;
}

/*
 *@@ trshRestoreFromTrashCan:
 *      implementation for XWPTrashObject::xwpRestoreFromTrashCan.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V0.9.5 (2000-08-24) [umoeller]: dragging objects made them undeletable; fixed.
 *@@changed V0.9.5 (2000-09-20) [pr]: fixed deletion data in related object
 */

BOOL trshRestoreFromTrashCan(XWPTrashObject *pTrashObject,
                             WPFolder *pTargetFolder)
{
    BOOL        brc = FALSE;
    XWPTrashObjectData *somThis = XWPTrashObjectGetData(pTrashObject);

    TRY_LOUD(excpt1)
    {
        do  // for breaks only
        {
            if (_pRelatedObject)
            {
                WPFolder    *pTargetFolder2 = 0;
                PTASKREC    pTaskRecSelf = _wpFindTaskRec(pTrashObject);
                                // this is != NULL if we're being called
                                // from wpMoveObject(somSelf), that is,
                                // the object is dragged out of the trash
                                // can
                PTASKREC    pTaskRecRelated = _wpFindTaskRec(_pRelatedObject);
                                // from what I see, this is NULL always

                PMPF_TRASHCAN(("_pRelatedObject: %s",
                            _wpQueryTitle(_pRelatedObject) ));

                cmnDumpTaskRec(pTrashObject, "xwpRestoreFromTrashCan (XWPTrashObject)", pTaskRecSelf);
                cmnDumpTaskRec(pTrashObject, "xwpRestoreFromTrashCan (related obj)", pTaskRecRelated);

                if (pTargetFolder)
                {
                    // target folder specified: use that one
                    pTargetFolder2 = pTargetFolder;
                    _wpSetTaskRec(_pRelatedObject,
                                  pTaskRecSelf,
                                  pTaskRecRelated);     // NULL
                }
                else
                {
                    PSZ         pszOriginalPath = NULL;
                    // target folder not specified: use the original dir
                    // where the object was deleted from
                    if (pszOriginalPath = _xwpQueryRelatedPath(pTrashObject))
                    {
                        PMPF_TRASHCAN(("    using original path %s",
                                pszOriginalPath));

                        // make sure that the original directory exists;
                        // it might not if a whole folder (tree) was
                        // moved into the trash can
                        if (!doshQueryDirExist(pszOriginalPath))
                            // doesn't exist: recreate that directory
                            if (doshCreatePath(pszOriginalPath,
                                               FALSE)   // do not hide those directories
                                        != NO_ERROR)
                                // stop and return FALSE
                                break;

                        pTargetFolder2 = _wpclsQueryFolder(_WPFolder,
                                                           pszOriginalPath,
                                                           TRUE);       // lock object
                    }
                }

                if (pTargetFolder2)
                {
                    // folder exists:
                    if (fopsMoveObjectConfirmed(_pRelatedObject,
                                                pTargetFolder2))
                    {
                        // successfully moved:
                        // V0.9.5 (2000-09-20) [pr]
                        // clear original object's deletion data
                        _xwpSetDeletion(_pRelatedObject, FALSE);

                        if (pTaskRecSelf)
                        {
                            // unset task records V0.9.5 (2000-08-24) [umoeller]
                            _wpSetTaskRec(pTrashObject,
                                          NULL,     // new task rec
                                          pTaskRecSelf); // old task rec
                            _wpSetTaskRec(_pRelatedObject,
                                          NULL,     // new task rec
                                          pTaskRecSelf); // old task rec
                        }
                        // destroy the trash object
                        brc = _wpFree(pTrashObject);
                    }
                } // end if (pTargetFolder2)
            } // end if (_pRelatedObject)
        } while (FALSE);
    }
    CATCH(excpt1) { } END_CATCH();

    return brc;
}

/*
 *@@ trshDragOver:
 *      implementation for XWPTrashCan::wpDragOver.
 *
 *      We check whether the object(s) are deleteable and
 *      return flags accordingly.
 *
 *      This must return the return values of DM_DRAGOVER,
 *      which is a MRFROM2SHORT.
 *
 *      -- USHORT 1 usDrop must be:
 *
 *          --  DOR_DROP (0x0001): Object can be dropped.
 *
 *          --  DOR_NODROP (0x0000): Object cannot be dropped at this time,
 *              but type and format are supported. Send DM_DRAGOVER again.
 *
 *          --  DOR_NODROPOP (0x0002):  Object cannot be dropped at this time,
 *              but only the current operation is not acceptable.
 *
 *          --  DOR_NEVERDROP (0x0003): Object cannot be dropped at all. Do
 *              not send DM_DRAGOVER again.
 *
 *      -- USHORT 2 usDefaultOp must specify the default operation, which can be:
 *
 *          --  DO_COPY 0x0010:
 *
 *          --  DO_LINK 0x0018:
 *
 *          --  DO_MOVE 0x0020:
 *
 *          --  DO_CREATE 0x0040: create object from template
 *
 *          --  DO_NEW: from PMREF:
 *              Default operation is create another.  Use create another to create
 *              an object that has default settings and data.  The result of using
 *              create another is identical to creating an object from a template.
 *              This value should be defined as DO_UNKNOWN+3 if it is not
 *              recognized in the current level of the toolkit.
 *
 *          --  Other: This value should be greater than or equal to (>=) DO_UNKNOWN
 *              but not DO_NEW.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V1.0.6 (2006-08-19) [erdmann]: allow discard operations like the Shredder @@fixes 817
 */

MRESULT trshDragOver(XWPTrashCan *somSelf,
                     PDRAGINFO pdrgInfo)
{
    USHORT      usDrop = DOR_NODROP,
                usDefaultOp = DO_MOVE;
    ULONG       ulItemNow = 0;

    if (    (pdrgInfo->usOperation != DO_MOVE)
         && (pdrgInfo->usOperation != DO_DEFAULT)
       )
    {
        usDrop = DOR_NODROPOP;      // invalid drop operation, but send msg again
    }
    else
    {
        // valid operation: set flag to drag OK first
        usDrop = DOR_DROP;

        // now go thru objects being dragged
        for (ulItemNow = 0;
             ulItemNow < pdrgInfo->cditem;
             ulItemNow++)
        {
            DRAGITEM    drgItem;
            if (DrgQueryDragitem(pdrgInfo,
                                 sizeof(drgItem),
                                 &drgItem,
                                 ulItemNow))
            {
                WPObject *pObjDragged = NULL;

                // V1.0.6 (2006-08-19) [erdmann]: added DRM_DISCARD processing
                if (!DrgVerifyRMF(&drgItem, "DRM_DISCARD", NULL))
                {
                    if (wpshQueryDraggedObject(&drgItem,
                                               &pObjDragged))
                    {
                        // we got the object:
                        // check if it's deletable
                        if (fopsValidateObjOperation(XFT_MOVE2TRASHCAN,
                                                     NULL, // no callback
                                                     pObjDragged,
                                                     NULL)
                                != NO_ERROR)
                        {
                            // no:
                            usDrop = DOR_NEVERDROP;
                            break;
                        }
                    }
                    else
                    {
                        // not acceptable:
                        usDrop = DOR_NEVERDROP; // do not send msg again
                        // and stop processing
                        break;
                    }
                }
            }
        }
    }

    // compose 2SHORT return value
    return MRFROM2SHORT(usDrop, usDefaultOp);
}

/*
 *@@ trshMoveDropped2TrashCan:
 *      implementation for XWPTrashCan::wpDrop.
 *
 *      This collects all dropped items from the
 *      DRAGINFO (which was passed to wpDrop) and
 *      starts a file task for moving the items to
 *      the trash can, using fopsStartTaskFromList.
 *
 *      Returns either RC_DROP_ERROR or RC_DROP_DROPCOMPLETE.
 *
 *      We delete the objects into the trash can by invoking
 *      XWPTrashCan::xwpDeleteIntoTrashCan.
 *
 *      This must return one of:
 *
 *      -- RC_DROP_DROPCOMPLETE 2: all objects processed; prohibit
 *         further wpDrop calls
 *
 *      -- RC_DROP_ERROR -1: error occurred, terminate drop
 *
 *      -- RC_DROP_ITEMCOMPLETE 1: one item processed, call wpDrop
 *         again for subsequent items
 *
 *      -- RC_DROP_RENDERING 0: request source rendering for this
 *         object, call wpDrop for next object.
 *
 *      We process all items at once here and return RC_DROP_DROPCOMPLETE
 *      unless an error occurs.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V0.9.7 (2001-01-13) [umoeller]: fixed some weirdos, incl. a possible memory leak
 *@@changed V1.0.6 (2006-08-11) [erdmann]: use correct function call @@fixes 815
 *@@changed V1.0.6 (2006-08-19) [erdmann]: allow discard operations like the Shredder @@fixes 817
 *@@changed V1.0.6 (2006-09-30) [erdmann]: cleanup drag structures correctly @@fixes 817
 */

MRESULT trshMoveDropped2TrashCan(XWPTrashCan *somSelf,
                                 PDRAGINFO pdrgInfo)
{
    MRESULT     mrc = (MRESULT)RC_DROP_ERROR;
    ULONG       ulItemNow = 0;
    PLINKLIST   pllDroppedObjects = lstCreate(FALSE);   // no auto-free
    BOOL        fItemsAdded = FALSE,
                fStartTask = TRUE;
    WPFolder    *pSourceFolder = NULL;

    for (ulItemNow = 0;
         ulItemNow < pdrgInfo->cditem;
         ulItemNow++)
    {
        DRAGITEM    drgItem;
        if (DrgQueryDragitem(pdrgInfo,
                             sizeof(drgItem),
                             &drgItem,
                             ulItemNow))
        {
            BOOL    fThisValid = FALSE;
            WPObject *pobjDropped = NULL;

            // V1.0.6 (2006-08-19) [erdmann]: added DRM_DISCARD processing
            if (DrgVerifyRMF(&drgItem, "DRM_DISCARD", NULL))
            {
                ULONG ulRet = (ULONG) DrgSendTransferMsg(drgItem.hwndItem,
                                                         DM_DISCARDOBJECT,
                                                         MPFROMP(pdrgInfo),
                                                         MPVOID);

                // only accept the operation if the source can handle the discard
                fThisValid = (ulRet == DRR_SOURCE);
            }
            else
                if (wpshQueryDraggedObject(&drgItem,
                                           &pobjDropped))
                {
                    if (fStartTask)
                    {
                        // no errors so far:
                        // add item to list
                        APIRET frc = NO_ERROR;
                        frc = fopsValidateObjOperation(XFT_MOVE2TRASHCAN,
                                                       NULL, // no callback
                                                       pobjDropped,
                                                       NULL);

                        if (frc == NO_ERROR)
                        {
                            lstAppendItem(pllDroppedObjects,
                                          pobjDropped);
                            pSourceFolder = _wpQueryFolder(pobjDropped);
                            fItemsAdded = TRUE;
                            fThisValid = TRUE;
                        }
                    }
                }

            // notify source of the success of
            // this operation (target rendering)
            // V1.0.6 (2006-08-11) [erdmann]: use correct function call
            DrgSendTransferMsg(drgItem.hwndItem,        // source
                               DM_ENDCONVERSATION,
                               (MPARAM)(drgItem.ulItemID),
                               (MPARAM)((fThisValid)
                                 ? DMFL_TARGETSUCCESSFUL
                                 : DMFL_TARGETFAIL));

            if (!fThisValid)
                fStartTask = FALSE;
        }
    }

    if (    (fStartTask)
         && (fItemsAdded)
       )
    {
        // OK:
        // start "move to trashcan" with the new list
        fopsStartTaskFromList(XFT_MOVE2TRASHCAN,
                              NULLHANDLE,       // no anchor block, asynchronously
                              pSourceFolder,
                              NULL,             // target folder: not needed
                              pllDroppedObjects);

        DrgDeleteDraginfoStrHandles(pdrgInfo); // V1.0.6 (2006-09-30) [erdmann]: @@fixes 817
        DrgFreeDraginfo(pdrgInfo);
        mrc = (MRESULT)RC_DROP_DROPCOMPLETE;
                // means: _all_ items have been processed,
                // and wpDrop should _not_ be called again
                // by the WPS for the next items, if any
    }

    // in any case, free the list V0.9.7 (2001-01-13) [umoeller]
    lstFree(&pllDroppedObjects);

    return mrc;
}

/*
 *@@ trshEmptyTrashCan:
 *      implementation for XWPTrashCan::xwpEmptyTrashCan.
 *
 *@@added V0.9.1 (2000-01-31) [umoeller]
 *@@changed V0.9.3 (2000-04-28) [umoeller]: switched implementation to XFT_TRUEDELETE
 *@@changed V0.9.7 (2001-01-17) [umoeller]: now returning ULONG
 *@@changed V0.9.20 (2002-07-12) [umoeller]: changed dialog confirmation title
 */

ULONG trshEmptyTrashCan(XWPTrashCan *somSelf,
                        HAB hab,             // in: synchronous operation, as with fopsStartTask
                        HWND hwndConfirmOwner,
                        PULONG pulDeleted)   // out: if TRUE is returned, no. of deleted objects; can be 0
{
    APIRET  frc = NO_ERROR;

    TRY_LOUD(excpt1)
    {
        if (hwndConfirmOwner)
            // confirmations desired:
            if (cmnMessageBoxExt(hwndConfirmOwner,
                                 // 168,      // "trash can"
                                 242,       // Empty Trash Can V0.9.20 (2002-07-12) [umoeller]
                                 NULL, 0,
                                 169,      // "really empty?"
                                 MB_YESNO | MB_DEFBUTTON2)
                       != MBID_YES)
                frc = FOPSERR_CANCELLEDBYUSER;

        if (frc == NO_ERROR)
        {
            // make sure the trash objects are up-to-date
            PLINKLIST   pllTrashObjects;
            ULONG       cObjects = 0;

            if ((_wpQueryFldrFlags(somSelf) & FOI_POPULATEDWITHALL) == 0)
            {
                // trash can not populated yet:
                if (hab)
                    frc = fopsStartPopulate(hab,  // synchronously
                                            somSelf);
                else
                    frc = FOPSERR_NOT_HANDLED_ABORT;
            }

            if (frc == NO_ERROR)
            {
                // create list of all related objects from the
                // trash objects in the trash can
                pllTrashObjects = trshCreateTrashObjectsList(somSelf,
                                                             TRUE,      // related objects
                                                             &cObjects);

                if (cObjects)
                    // delete related objects
                    frc = fopsStartTaskFromList(XFT_TRUEDELETE,
                                                hab,
                                                somSelf,        // source folder
                                                NULL,           // target folder: not needed
                                                pllTrashObjects); // list with objects

                lstFree(&pllTrashObjects);
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    return frc;
}

/*
 *@@ trshValidateTrashObject:
 *      implementation for XWPTrashObject::xwpValidateTrashObject.
 *      See remarks there.
 *
 *@@added V0.9.2 (2000-02-28) [umoeller]
 */

APIRET trshValidateTrashObject(XWPTrashObject *somSelf)
{
    APIRET arc = NO_ERROR;
    XWPTrashObjectData *somThis = XWPTrashObjectGetData(somSelf);

    if (_pRelatedObject == 0)
        // not set yet:
        arc = ERROR_INVALID_HANDLE;
    else
        if (!wpshCheckObject(_pRelatedObject))
            // pointer invalid:
            arc = ERROR_FILE_NOT_FOUND;
        else
        {
            // object seems to be valid:
            // check if it's a file-system object and
            // if the actual file still exists
            if (_somIsA(_pRelatedObject, _WPFileSystem))
            {
                // yes:
                CHAR szFilename[2*CCHMAXPATH];
                if (_wpQueryFilename(_pRelatedObject, szFilename, TRUE))
                    if (access(szFilename, 0) != 0)
                    {
                        // file doesn't exist any more:
                        arc = ERROR_FILE_NOT_FOUND;
                    }
            }
        }

    if (arc != NO_ERROR)
        // any error found:
        // destroy the object
        _wpFree(somSelf);

    return arc;
}

/* ******************************************************************
 *
 *   Trash can drives support
 *
 ********************************************************************/

/*
 *@@ trshSetDrivesSupport:
 *      implementation for M_XWPTrashCan::xwpclsSetDrivesSupport.
 *
 *@@added V0.9.1 (2000-02-03) [umoeller]
 *@@changed V0.9.12 (2001-05-18) [umoeller]: changed defaults to support only FAT, FAT32, HPFS, JFS
 *@@changed V1.0.9 (2009-01-03) [pr]: defaults moved to xwpclsQueryDrivesSupport @@fixes 1114
 */

BOOL trshSetDrivesSupport(PBYTE pabSupportedDrives)
{
    BOOL brc = FALSE;
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (pabSupportedDrives)
            {
                // drives specified:
                memcpy(G_abSupportedDrives, pabSupportedDrives, CB_SUPPORTED_DRIVES);
                // write to INI
                PrfWriteProfileData(HINI_USER,
                                    (PSZ)INIAPP_XWORKPLACE,
                                    (PSZ)INIKEY_TRASHCANDRIVES,
                                    G_abSupportedDrives,
                                    sizeof(G_abSupportedDrives));
            }
            else
            {
                // pointer is NULL:
                // use defaults then
                _xwpclsQueryDrivesSupport(_XWPTrashCan, G_abSupportedDrives);  // V1.0.9
                // delete INI key
                PrfWriteProfileString(HINI_USER,
                                      (PSZ)INIAPP_XWORKPLACE,
                                      (PSZ)INIKEY_TRASHCANDRIVES,
                                      NULL);        // delete
            }

            brc = TRUE;
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    if (fLocked)
        krnUnlock();

    return brc;
}

/*
 *@@ trshQueryDrivesSupport:
 *      implementation for M_XWPTrashCan::xwpclsQueryDrivesSupport.
 *
 *@@added V0.9.1 (2000-02-03) [umoeller]
 *@@changed V1.0.9 (2009-01-03) [pr]: defaults moved from xwpclsSetDrivesSupport @@fixes 1114
 */

BOOL trshQueryDrivesSupport(PBYTE pabSupportedDrives)
{
    BOOL brc = FALSE;

    // V1.0.9
    TRY_LOUD(excpt1)
    {
        ULONG   ulLogicalDrive = 3;     // start with drive C:
        BYTE    bIndex = 0;             // index into G_abSupportedDrives

        memset(G_abSupportedDrives, 0, sizeof(G_abSupportedDrives));

        for (ulLogicalDrive = 3;
             ulLogicalDrive < CB_SUPPORTED_DRIVES + 3;
             ulLogicalDrive++)
        {
            APIRET  arc = doshAssertDrive(ulLogicalDrive, 0);

            switch (arc)
            {
                case NO_ERROR:
                {
                    // drive is ready:
                    CHAR szFS[30];

                    // do some more checks V0.9.12 (2001-05-18) [umoeller]
                    G_abSupportedDrives[bIndex] = XTRC_UNSUPPORTED;
                    if (!doshQueryDiskFSType(ulLogicalDrive,
                                             szFS,
                                             sizeof(szFS)))
                    {
                        if (    (!stricmp(szFS, "FAT"))
                             || (!stricmp(szFS, "HPFS"))
                             || (!stricmp(szFS, "JFS"))
                             || (!stricmp(szFS, "FAT32"))
                           )
                            // alright, let's support this
                            G_abSupportedDrives[bIndex] = XTRC_SUPPORTED;
                    }
                }
                break;

                case ERROR_INVALID_DRIVE:
                    G_abSupportedDrives[bIndex] = XTRC_INVALID;
                break;

                default:
                    // this includes ERROR_NOT_READY, ERROR_NOT_SUPPORTED
                    G_abSupportedDrives[bIndex] = XTRC_UNSUPPORTED;
            }

            bIndex++;
        }
    } CATCH(excpt1) { } END_CATCH();

    if (pabSupportedDrives)
    {
        memcpy(pabSupportedDrives, &G_abSupportedDrives, sizeof(G_abSupportedDrives));
        brc = TRUE;
    }

    return brc;
}

/*
 *@@ trshLoadDrivesSupport:
 *      loads the drives support data from OS2.INI.
 *      Used in M_XWPTrashCan::wpclsInitData.
 *
 *@@added V0.9.1 (2000-02-03) [umoeller]
 */

VOID trshLoadDrivesSupport(M_XWPTrashCan *somSelf)
{
    ULONG   cbSupportedDrives = sizeof(G_abSupportedDrives);
    memset(G_abSupportedDrives, XTRC_INVALID, cbSupportedDrives);
    if (!PrfQueryProfileData(HINI_USER,
                             (PSZ)INIAPP_XWORKPLACE,
                             (PSZ)INIKEY_TRASHCANDRIVES,
                             G_abSupportedDrives,
                             &cbSupportedDrives))
        // data not found:
        _xwpclsSetDrivesSupport(somSelf,
                                NULL);     // defaults
}

/*
 *@@ trshIsOnSupportedDrive:
 *      returns NO_ERROR only if pObject is on a drive for which
 *      trash can support has been enabled.
 *
 *      Otherwise this returns:
 *
 *      --  FOPSERR_TRASHDRIVENOTSUPPORTED
 *
 *      --  FOPSERR_WPQUERYFILENAME_FAILED
 *
 *@@added V0.9.2 (2000-03-04) [umoeller]
 *@@changed V0.9.16 (2001-11-10) [umoeller]: now returning APIRET
 *@@changed V0.9.16 (2001-11-10) [umoeller]: fixed UNC objects
 */

APIRET trshIsOnSupportedDrive(WPObject *pObject)
{
    APIRET frc = FOPSERR_NOT_HANDLED_ABORT;

    WPFolder *pFolder;
    CHAR szFolderPath[CCHMAXPATH];

    if (    (pFolder = _wpQueryFolder(pObject))
         && (_wpQueryFilename(pFolder, szFolderPath, TRUE))
       )
    {
        nlsUpper(szFolderPath);

        frc = FOPSERR_TRASHDRIVENOTSUPPORTED;   // for drive A: and B: also

        if (    (szFolderPath[0] >= 'C')
             && (szFolderPath[0] <= 'Z')        // exclude UNC V0.9.16 (2001-11-10) [umoeller]
           )
        {
            // is on hard disk:
            if (G_abSupportedDrives[szFolderPath[0] - 'C'] == XTRC_SUPPORTED)
                frc = NO_ERROR;
        }
    }
    else
        frc = FOPSERR_WPQUERYFILENAME_FAILED;       // V0.9.16 (2001-11-10) [umoeller]

    return frc;
}

/* ******************************************************************
 *
 *   Trash can frame subclassing
 *
 ********************************************************************/

// LINKLIST    llSubclassedTrashCans;
// HMTX        hmtxSubclassedTrashCans = NULLHANDLE;

/*
 *  THIS ENTIRE CODE HAS BEEN REMOVED WITH V0.9.7.
 *
 *  All the processing that used to be in here is now processed
 *  by the subclassed folder frame procedure. We don't really
 *  need the overhead of subclassing trash can frames again.
 *
 *  Instead, we have now introduced XFolder::xwpProcessViewCommand,
 *  which is overridden for XWPTrashCan.
 */

/* ******************************************************************
 *
 *   XWPTrashCan notebook callbacks (notebook.c)
 *
 ********************************************************************/

static const XWPSETTING G_TrashCanSettingsBackup[] =
    {
        sflTrashConfirmEmpty,
        sfAlwaysTrueDelete          // V0.9.19 (2002-04-14) [umoeller]
    };

static const CONTROLDEF
    OptionsGroup = LOADDEF_GROUP(ID_XTDI_OPTIONSGROUP, SZL_AUTOSIZE),
    AlwaysTrueDeleteCB = LOADDEF_AUTOCHECKBOX(ID_XTDI_ALWAYSTRUEDELETE),
    ConfirmEmptyCB = LOADDEF_AUTOCHECKBOX(ID_XTDI_CONFIRMEMPTY),
    ConfirmDestroyCB = LOADDEF_AUTOCHECKBOX(ID_XTDI_CONFIRMDESTROY);

static const DLGHITEM dlgTrashSettings[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),
                START_GROUP_TABLE(&OptionsGroup),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&ConfirmEmptyCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&ConfirmDestroyCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&AlwaysTrueDeleteCB),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

/*
 *@@ trshTrashCanSettingsInitPage:
 *      notebook callback function (notebook.c) for the
 *      "TrashCan" settings page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.19 (2002-04-14) [umoeller]: now using dialog formatter, added "always true delete" here
 */

VOID trshTrashCanSettingsInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                                  ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_TrashCanSettingsBackup,
                                         ARRAYITEMCOUNT(G_TrashCanSettingsBackup));

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-04-14) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      dlgTrashSettings,
                      ARRAYITEMCOUNT(dlgTrashSettings));
    }

    if (flFlags & CBI_ENABLE)
    {
    }

    if (flFlags & CBI_SET)
    {
        ULONG fl = cmnQuerySetting(sflTrashConfirmEmpty);

        // V0.9.19 (2002-04-14) [umoeller]
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XTDI_ALWAYSTRUEDELETE,
                              cmnQuerySetting(sfAlwaysTrueDelete));

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XTDI_CONFIRMEMPTY,
                              (fl & TRSHCONF_EMPTYTRASH) != 0);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XTDI_CONFIRMDESTROY,
                              (fl & TRSHCONF_DESTROYOBJ) != 0);
    }
}

/*
 *@@ trshTrashCanSettingsItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "TrashCan" settings page.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.19 (2002-04-14) [umoeller]: now using dialog formatter, added "always true delete" here
 */

MRESULT trshTrashCanSettingsItemChanged(PNOTEBOOKPAGE pnbp,
                                        ULONG ulItemID, USHORT usNotifyCode,
                                        ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;

    ULONG flChanged = 0;

    switch (ulItemID)
    {
        case ID_XTDI_ALWAYSTRUEDELETE:      // V0.9.19 (2002-04-14) [umoeller]
            cmnSetSetting(sfAlwaysTrueDelete, ulExtra);
        break;

        case ID_XTDI_CONFIRMEMPTY:
            flChanged = TRSHCONF_EMPTYTRASH;
        break;

        case ID_XTDI_CONFIRMDESTROY:
            flChanged = TRSHCONF_DESTROYOBJ;
        break;

        case DID_UNDO:
            // "Undo" button:
            // restore the settings for this page
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_TrashCanSettingsBackup));

            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        case DID_DEFAULT:
            // set the default settings for this settings page
            // (this is in common.c because it's also used at
            // Desktop startup)
            cmnSetDefaultSettings(pnbp->inbp.ulPageID);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;
    }

    if (flChanged)
    {
        ULONG fl = cmnQuerySetting(sflTrashConfirmEmpty);
        if (ulExtra)
            fl |= flChanged;
        else
            fl &= ~flChanged;
        cmnSetSetting(sflTrashConfirmEmpty, fl);
    }

    return mrc;
}

/*
 *@@ trshTrashCanDrivesInitPage:
 *      notebook callback function (notebook.c) for the
 *      trash can "Drives" settings page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 */

VOID trshTrashCanDrivesInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                                ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: backup drives array for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = malloc(CB_SUPPORTED_DRIVES);
        _xwpclsQueryDrivesSupport(_XWPTrashCan, pnbp->pUser);
    }

    if (flFlags & CBI_SET)
    {
        ULONG   bIndex = 0;
        CHAR    szDriveName[3] = "C:";
        HWND    hwndSupportedLB = WinWindowFromID(pnbp->hwndDlgPage, ID_XTDI_SUPPORTED_LB),
                hwndUnsupportedLB = WinWindowFromID(pnbp->hwndDlgPage, ID_XTDI_UNSUPPORTED_LB);

        winhDeleteAllItems(hwndSupportedLB);
        winhDeleteAllItems(hwndUnsupportedLB);

        for (bIndex = 0;
             bIndex < CB_SUPPORTED_DRIVES;
             bIndex++)
        {
            if (G_abSupportedDrives[bIndex] != XTRC_INVALID)
            {
                // if drive is supported, insert into "supported",
                // otherwise into "unsupported"
                HWND    hwndLbox = (G_abSupportedDrives[bIndex] == XTRC_SUPPORTED)
                                        ? hwndSupportedLB
                                        : hwndUnsupportedLB;
                // insert
                LONG lInserted = WinInsertLboxItem(hwndLbox,
                                                   LIT_END,
                                                   szDriveName);
                // set item's handle to array index
                winhSetLboxItemHandle(hwndLbox,
                                      lInserted,
                                      bIndex);
            }

            // raise drive letter
            (szDriveName[0])++;
        }
    }

    if (flFlags & CBI_ENABLE)
    {
        // enable "Add" button if items are selected
        // in the "Unsupported" listbox
        WinEnableControl(pnbp->hwndDlgPage, ID_XTDI_ADD_SUPPORTED,
                         (winhQueryLboxSelectedItem(WinWindowFromID(pnbp->hwndDlgPage,
                                                                    ID_XTDI_UNSUPPORTED_LB),
                                                     LIT_FIRST)
                                != LIT_NONE));
        // enable "Remove" button if items are selected
        // in the "Supported" listbox
        WinEnableControl(pnbp->hwndDlgPage, ID_XTDI_REMOVE_SUPPORTED,
                         (winhQueryLboxSelectedItem(WinWindowFromID(pnbp->hwndDlgPage,
                                                                    ID_XTDI_SUPPORTED_LB),
                                                     LIT_FIRST)
                                != LIT_NONE));
    }
}

/*
 *@@ StoreSupportedDrives:
 *      this gets called from trshTrashCanDrivesItemChanged
 *      to read the (un)supported drives from the listbox.
 *
 *@@added V0.9.1 (99-12-14) [umoeller]
 */

STATIC BOOL StoreSupportedDrives(HWND hwndSupportedLB, // in: list box with supported drives
                                 HWND hwndUnsupportedLB) // in: list box with unsupported drives
{
    BOOL    brc = FALSE;

    if ((hwndSupportedLB) && (hwndUnsupportedLB))
    {
        BYTE    abSupportedDrivesNew[CB_SUPPORTED_DRIVES];
        SHORT   sIndexThis = 0;
        SHORT   sItemCount;

        // set all drives to "invalid"; only those
        // items will be overwritten which are in the
        // listboxes
        memset(abSupportedDrivesNew, XTRC_INVALID, sizeof(abSupportedDrivesNew));

        // go thru "supported" listbox
        sItemCount = WinQueryLboxCount(hwndSupportedLB);
        for (sIndexThis = 0;
             sIndexThis < sItemCount;
             sIndexThis++)
        {
            ULONG   ulIndexHandle = winhQueryLboxItemHandle(hwndSupportedLB,
                                                            sIndexThis);
            abSupportedDrivesNew[ulIndexHandle] = XTRC_SUPPORTED;
        }

        // go thru "unsupported" listbox
        sItemCount = WinQueryLboxCount(hwndUnsupportedLB);
        for (sIndexThis = 0;
             sIndexThis < sItemCount;
             sIndexThis++)
        {
            ULONG   ulIndexHandle = winhQueryLboxItemHandle(hwndUnsupportedLB,
                                                            sIndexThis);
            abSupportedDrivesNew[ulIndexHandle] = XTRC_UNSUPPORTED;
        }

        // update trash can class with that data
        _xwpclsSetDrivesSupport(_XWPTrashCan,
                                abSupportedDrivesNew);
    }

    return brc;
}

/*
 *@@ trshTrashCanDrivesItemChanged:
 *      notebook callback function (notebook.c) for the
 *      trash can "Drives" settings page.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.4 (2000-07-15) [umoeller]: multiple selections weren't moved
 */

MRESULT trshTrashCanDrivesItemChanged(PNOTEBOOKPAGE pnbp,
                                      ULONG ulItemID, USHORT usNotifyCode,
                                      ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;
    BOOL fSave = TRUE;

    static BOOL fNoDeselection = FALSE;

    switch (ulItemID)
    {
        /*
         * ID_XTDI_UNSUPPORTED_LB:
         * ID_XTDI_SUPPORTED_LB:
         *      if list box selections change,
         *      re-enable "Add"/"Remove" buttons
         */

        case ID_XTDI_UNSUPPORTED_LB:
        case ID_XTDI_SUPPORTED_LB:
            if (usNotifyCode == LN_SELECT)
            {
                // deselect all items in the other listbox
                if (!fNoDeselection)
                {
                    fNoDeselection = TRUE;
                            // this recurses
                    winhLboxSelectAll(WinWindowFromID(pnbp->hwndDlgPage,
                                                      ((ulItemID == ID_XTDI_UNSUPPORTED_LB)
                                                        ? ID_XTDI_SUPPORTED_LB
                                                        : ID_XTDI_UNSUPPORTED_LB)),
                                      FALSE); // deselect
                    fNoDeselection = FALSE;
                }

                // re-enable items
                pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
            }
        break;

        /*
         * ID_XTDI_ADD_SUPPORTED:
         *      "Add" button: move item(s)
         *      from "unsupported" to "supported"
         *      list box
         */

        case ID_XTDI_ADD_SUPPORTED:
        {
            HWND    hwndSupportedLB = WinWindowFromID(pnbp->hwndDlgPage, ID_XTDI_SUPPORTED_LB),
                    hwndUnsupportedLB = WinWindowFromID(pnbp->hwndDlgPage, ID_XTDI_UNSUPPORTED_LB);

            fNoDeselection = TRUE;
            while (TRUE)
            {
                SHORT sItemStart = winhQueryLboxSelectedItem(hwndUnsupportedLB,
                                                             LIT_FIRST);
                if (sItemStart == LIT_NONE)
                    break;

                // move item
                winhMoveLboxItem(hwndUnsupportedLB,
                                 sItemStart,
                                 hwndSupportedLB,
                                 LIT_SORTASCENDING,
                                 TRUE);         // select
            }
            fNoDeselection = FALSE;

            // re-enable buttons
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);

            // update internal drives data
            StoreSupportedDrives(hwndSupportedLB,
                                 hwndUnsupportedLB);
        }
        break;

        /*
         * ID_XTDI_REMOVE_SUPPORTED:
         *      "Add" button: move item(s)
         *      from "unsupported" to "supported"
         *      list box
         */

        case ID_XTDI_REMOVE_SUPPORTED:
        {
            HWND    hwndSupportedLB = WinWindowFromID(pnbp->hwndDlgPage, ID_XTDI_SUPPORTED_LB),
                    hwndUnsupportedLB = WinWindowFromID(pnbp->hwndDlgPage, ID_XTDI_UNSUPPORTED_LB);
            // CHAR    szItemText[10];

            fNoDeselection = TRUE;
            while (TRUE)
            {
                SHORT sItemStart = winhQueryLboxSelectedItem(hwndSupportedLB,
                                                             LIT_FIRST);
                if (sItemStart == LIT_NONE)
                    break;

                // move item
                winhMoveLboxItem(hwndSupportedLB,
                                 sItemStart,
                                 hwndUnsupportedLB,
                                 LIT_SORTASCENDING,
                                 TRUE);         // select
            }
            fNoDeselection = FALSE;

            // re-enable buttons
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);

            // update internal drives data
            StoreSupportedDrives(hwndSupportedLB,
                                 hwndUnsupportedLB);
        }
        break;

        case DID_UNDO:
            // copy array back which was stored in init callback
            _xwpclsSetDrivesSupport(_XWPTrashCan,
                                    pnbp->pUser);  // backup data
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        case DID_DEFAULT:
            // set defaults
            _xwpclsSetDrivesSupport(_XWPTrashCan,
                                    NULL);     // defaults
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        default:
            fSave = FALSE;
    }

    return mrc;
}

/*
 *@@ trshTrashCanIconInitPage:
 *      notebook callback function (notebook.c) for the
 *      trash can "Icon" settings page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *      Note that this code is only used any more if the
 *      user has disabled the icon pages replacements.
 *
 *@@added V0.9.4 (2000-08-03) [umoeller]
 */

VOID trshTrashCanIconInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                              ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call:
        // backup object title for "Undo" button
        pnbp->pUser = strhdup(_wpQueryTitle(pnbp->inbp.somSelf), NULL);

        WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_XTDI_ICON_TITLEMLE,
                          MLM_SETTEXTLIMIT,
                          (MPARAM)255,
                          0);
    }

    if (flFlags & CBI_SET)
    {
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XTDI_ICON_TITLEMLE,
                          _wpQueryTitle(pnbp->inbp.somSelf));
    }
}

/*
 *@@ trshTrashCanIconItemChanged:
 *      notebook callback function (notebook.c) for the
 *      trash can "Icon" settings page.
 *      Reacts to changes of any of the dialog controls.
 *
 *      Note that this code is only used any more if the
 *      user has disabled the icon pages replacements.
 *
 *@@added V0.9.4 (2000-08-03) [umoeller]
 */

MRESULT trshTrashCanIconItemChanged(PNOTEBOOKPAGE pnbp,
                                    ULONG ulItemID, USHORT usNotifyCode,
                                    ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;

    switch (ulItemID)
    {
        case ID_XTDI_ICON_TITLEMLE:
            switch (usNotifyCode)
            {
                case MLN_KILLFOCUS:
                {
                    PSZ pszNewTitle;
                    if (!(pszNewTitle = winhQueryWindowText(pnbp->hwndControl)))
                    {
                        // no title: restore old
                        WinSetWindowText(pnbp->hwndControl,
                                         _wpQueryTitle(pnbp->inbp.somSelf));
                        cmnMessageBoxExt(pnbp->hwndDlgPage,
                                         104,   // error
                                         NULL, 0,
                                         187,   // old name restored
                                         MB_OK);
                    }
                    else
                        _wpSetTitle(pnbp->inbp.somSelf, pszNewTitle);
                    free(pszNewTitle);
                }
                break;
            }
        break;

        case DID_UNDO:
            // set backed-up title
            _wpSetTitle(pnbp->inbp.somSelf, (PSZ)pnbp->pUser);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

        case DID_DEFAULT:
            // set class default title
            _wpSetTitle(pnbp->inbp.somSelf,
                        _wpclsQueryTitle(_somGetClass(pnbp->inbp.somSelf)));
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;
    }

    return mrc;
}
