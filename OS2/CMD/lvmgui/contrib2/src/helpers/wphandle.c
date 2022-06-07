
/*
 *@@sourcefile wphandle.c:
 *      this file contains the logic for dealing with
 *      those annoying WPS object handles in OS2SYS.INI.
 *      This does not use WPS interfaces, but parses
 *      the profiles directly.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  wph*   WPS object helper functions
 *
 *      Thanks go out to Henk Kelder for telling me the
 *      format of the WPS INI data. With V0.9.16, this
 *      file was completely rewritten and no longer uses
 *      his code though.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\wphandle.h"
 */

/*
 *      This file Copyright (C) 1997-2008 Ulrich M”ller,
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSERRORS

#define INCL_WINSHELLDATA
#define INCL_WINNLS
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <setjmp.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\except.h"
#include "helpers\linklist.h"
#include "helpers\nls.h"
#include "helpers\prfh.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\tree.h"
#include "helpers\xstring.h"

#define INCLUDE_WPHANDLE_PRIVATE
#include "helpers\wphandle.h"

/*
 *@@category: Helpers\PM helpers\Workplace Shell\Handles (OS2SYS.INI)
 *      See wphandle.c.
 */

/* ******************************************************************
 *
 *   Load handles functions
 *
 ********************************************************************/

/*
 *@@ wphQueryActiveHandles:
 *      returns the value of PM_Workplace:ActiveHandles
 *      in OS2SYS.INI as a new buffer.
 *
 *      There are always two buffers in OS2SYS.INI for object
 *      handles, called "PM_Workplace:HandlesX" with "X" either
 *      being "0" or "1".
 *
 *      It seems that every time the WPS does something in the
 *      handles section, it writes the data to the inactive
 *      buffer first and then makes it the active buffer by
 *      changing the "active handles" key. You can test this
 *      by creating a shadow on your Desktop.
 *
 *      This returns a new PSZ which the caller must free()
 *      after use.
 *
 *      This gets called by the one-shot function
 *      wphQueryHandleFromPath.
 *
 *@@changed V0.9.16 (2001-10-02) [umoeller]: rewritten
 */

APIRET wphQueryActiveHandles(HINI hiniSystem,
                             PSZ *ppszActiveHandles)        // out: active handles string (new buffer)
{
    PSZ pszActiveHandles;
    if (pszActiveHandles = prfhQueryProfileData(hiniSystem,
                                                WPINIAPP_ACTIVEHANDLES,
                                                WPINIAPP_HANDLESAPP,
                                                NULL))
    {
        *ppszActiveHandles = pszActiveHandles;
        return NO_ERROR;
    }

    return ERROR_WPH_NO_ACTIVEHANDLES_DATA;
}

/*
 *@@ wphQueryBaseClassesHiwords:
 *      returns the hiwords for the WPS base
 *      classes. Unless the user's system is
 *      really badly configured, this should
 *      set
 *
 *      --  pusHiwordAbstract to 2;
 *      --  pusHiwordFileSystem to 3.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_WPH_NO_BASECLASS_DATA
 *
 *      --  ERROR_WPH_INCOMPLETE_BASECLASS_DATA
 *
 *      This gets called automatically from wphLoadHandles.
 *
 *@@added V0.9.16 (2001-10-02) [umoeller]
 */

APIRET wphQueryBaseClassesHiwords(HINI hiniUser,
                                  PUSHORT pusHiwordAbstract,
                                  PUSHORT pusHiwordFileSystem)
{
    APIRET arc = NO_ERROR;

    // get the index of WPFileSystem from the base classes list...
    // we need this to determine the hiword for file-system handles
    // properly. Normally, this should be 3.
    ULONG cbBaseClasses = 0;
    PSZ pszBaseClasses;
    if (pszBaseClasses = prfhQueryProfileData(hiniUser,
                                              "PM_Workplace:BaseClass",
                                              "ClassList",
                                              &cbBaseClasses))
    {
        // parse that buffer... these has the base class names,
        // separated by 0. List is terminated by two zeroes.
        PSZ     pszClassThis = pszBaseClasses;
        ULONG   ulHiwordThis = 1;
        while (    (*pszClassThis)
                && (pszClassThis - pszBaseClasses < cbBaseClasses)
              )
        {
            if (!strcmp(pszClassThis, "WPFileSystem"))
                *pusHiwordFileSystem = ulHiwordThis;
            else if (!strcmp(pszClassThis, "WPAbstract"))
                *pusHiwordAbstract = ulHiwordThis;

            ulHiwordThis++;
            pszClassThis += strlen(pszClassThis) + 1;
        }

        // now check if we found both
        if (    (!(*pusHiwordFileSystem))
             || (!(*pusHiwordAbstract))
           )
            arc = ERROR_WPH_INCOMPLETE_BASECLASS_DATA;

        free(pszBaseClasses);
    }
    else
        arc = ERROR_WPH_NO_BASECLASS_DATA;

    return arc;
}

/*
 *@@ FreeChildrenTree:
 *      called from NukeNameTrees for recursion.
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 */

STATIC VOID FreeChildrenTree(TREE **ppChildrenTree,
                             PLONG plCount)
{
    LONG    cItems = *plCount;
    TREE**  papNodes = treeBuildArray(*ppChildrenTree,
                                      &cItems);
    if (papNodes)
    {
        ULONG ul;
        for (ul = 0; ul < cItems; ul++)
        {
            PNODETREENODE pNodeThis = (PNODETREENODE)papNodes[ul];

            FreeChildrenTree(&pNodeThis->ChildrenTree,
                             &pNodeThis->cChildren);

            free(pNodeThis);
        }

        free(papNodes);
        *plCount = 0;

        treeInit(ppChildrenTree, plCount);
    }
}

/*
 *@@ NukeNameTrees:
 *      frees all cache trees.
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 */

STATIC APIRET NukeNameTrees(PHANDLESBUF pHandlesBuf)
{
    APIRET arc = NO_ERROR;

    LONG    cItems = pHandlesBuf->cDrives;
    TREE**  papNodes = treeBuildArray(pHandlesBuf->DrivesTree,
                                      &cItems);
    if (papNodes)
    {
        ULONG ul;
        for (ul = 0; ul < cItems; ul++)
        {
            PDRIVETREENODE pNodeThis = (PDRIVETREENODE)papNodes[ul];

            FreeChildrenTree(&pNodeThis->ChildrenTree,
                             &pNodeThis->cChildren);

            free(pNodeThis);
        }

        free(papNodes);

        treeInit(&pHandlesBuf->DrivesTree,
                 &pHandlesBuf->cDrives);
    }

    return arc;
}

/*
 *@@ wphRebuildNodeHashTable:
 *      builds all the complex cache trees in the
 *      given handles buffer.
 *
 *      If (fQuitOnErrors == TRUE), we'll fail as
 *      soon as an invalid handle is found. Otherwise
 *      we will try to continue if the error is not fatal.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_WPH_CORRUPT_HANDLES_DATA
 *
 *      --  ERROR_WPH_DRIV_TREEINSERT_FAILED: duplicate DRIV node
 *          (non-fatal)
 *
 *@@added V0.9.16 (2001-10-02) [umoeller]
 *@@changed V0.9.17 (2002-02-05) [umoeller]: added fQuitOnErrors
 *@@changed XWP V1.0.8 (2008-05-25) [pr]: made DBCS safe @@fixes 1070
 */

APIRET wphRebuildNodeHashTable(HHANDLES hHandles,
                               BOOL fQuitOnErrors)
{
    APIRET arc = NO_ERROR;

    PHANDLESBUF pHandlesBuf;

    if (    (!(pHandlesBuf = (PHANDLESBUF)hHandles))
         || (!pHandlesBuf->pbData)
         || (!pHandlesBuf->cbData)
       )
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        // start at beginning of buffer
        PBYTE pCur = pHandlesBuf->pbData + 4;
        PBYTE pEnd = pHandlesBuf->pbData + pHandlesBuf->cbData;

        PDRIVETREENODE  pLastDriveTreeNode = NULL;

        memset(pHandlesBuf->NodeHashTable, 0, sizeof(pHandlesBuf->NodeHashTable));
        NukeNameTrees(pHandlesBuf);

        // now set up hash table
        while (    (pCur < pEnd)
                && (!arc)
              )
        {
            if (!memicmp(pCur, "DRIV", 4))
            {
                // pCur points to a DRIVE node:
                // these never have handles, so skip this
                PDRIVE pDriv = (PDRIVE)pCur;

                // upper the node name for string comparisons
                nlsUpper(pDriv->szName);  // V1.0.8 (2008-05-25)

                // create a drive tree node
                // (stored so we can append to this)
                if (!(pLastDriveTreeNode = NEW(DRIVETREENODE)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else
                {
                    pLastDriveTreeNode->Tree.ulKey = (ULONG)pDriv->szName;
                    pLastDriveTreeNode->pDriv = pDriv;
                    treeInit(&pLastDriveTreeNode->ChildrenTree,
                             &pLastDriveTreeNode->cChildren);
                    if (treeInsert(&pHandlesBuf->DrivesTree,
                                   &pHandlesBuf->cDrives,
                                   (TREE*)pLastDriveTreeNode,
                                   treeCompareStrings))
                        if (fQuitOnErrors)
                            arc = ERROR_WPH_DRIV_TREEINSERT_FAILED;
                }

                // next item
                pCur += sizeof(DRIVE) + strlen(pDriv->szName);
            }
            else if (!memicmp(pCur, "NODE", 4))
            {
                // pCur points to a regular NODE: offset pointer first
                PNODE pNode = (PNODE)pCur;
                PNODETREENODE pNew;

                // upper the node name for string comparisons
                nlsUpper(pNode->szName);  // V1.0.8 (2008-05-25)

                // create a node tree node
                if (!(pNew = NEW(NODETREENODE)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else
                {
                    TREE **ppTree = NULL;
                    PLONG pcChildren = NULL;

                    pNew->Tree.ulKey = (ULONG)pNode->szName;
                    pNew->pNode = pNode;
                    treeInit(&pNew->ChildrenTree,
                             &pNew->cChildren);
                    // now check where to insert this...
                    // does it have a parent?
                    if (pNode->usParentHandle)
                    {
                        PNODETREENODE pParent;
                        if (!(pParent = pHandlesBuf->NodeHashTable[pNode->usParentHandle]))
                        {
                            // this parent handle is invalid:
                            if (fQuitOnErrors)
                                arc = ERROR_WPH_INVALID_PARENT_HANDLE;
                        }
                        else
                        {
                            ppTree = &pParent->ChildrenTree;
                            pcChildren = &pParent->cChildren;
                        }
                    }
                    else
                        // null parent handle: then the parent
                        // must be a DRIVE node
                        if (pLastDriveTreeNode)
                        {
                            ppTree = &pLastDriveTreeNode->ChildrenTree;
                            pcChildren = &pLastDriveTreeNode->cChildren;
                        }
                        else
                            if (fQuitOnErrors)
                                arc = ERROR_WPH_NODE_BEFORE_DRIV;

                    if (ppTree && pcChildren)
                        if (!treeInsert(ppTree,
                                        pcChildren,
                                        (TREE*)pNew,
                                        treeCompareStrings))
                        {
                            // store PNODE in hash table
                            pHandlesBuf->NodeHashTable[pNode->usHandle] = pNew;
                            // do not free
                            pNew = NULL;
                        }
                        else
                            ;
                            // @@todo if this fails, there are
                            // several handles for short name!!!
                            // arc = ERROR_WPH_NODE_TREEINSERT_FAILED;

                    if (pNew)
                        free(pNew);

                }

                pCur += sizeof(NODE) + pNode->usNameSize;
            }
            else
            {
                arc = ERROR_WPH_CORRUPT_HANDLES_DATA;
                break;
            }
        }
    }

    if (!arc)
        pHandlesBuf->fCacheValid = TRUE;

    return arc;
}

/*
 *@@ wphLoadHandles:
 *      returns a HANDLESBUF structure which will hold
 *      all the handles from OS2SYS.INI. In addition,
 *      this calls wphQueryBaseClassesHiwords and puts
 *      the hiwords for WPAbstract and WPFileSystem into
 *      the HANDLESBUF as well.
 *
 *      Prerequisite before using any of the other wph*
 *      functions.
 *
 *      Call wphFreeHandles to free all data allocated
 *      by this function.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_WPH_NO_HANDLES_DATA: cannot read handle blocks.
 *
 *      --  ERROR_WPH_CORRUPT_HANDLES_DATA: cannot read handle blocks.
 *
 *@@added V0.9.16 (2001-10-02) [umoeller]
 */

APIRET wphLoadHandles(HINI hiniUser,      // in: HINI_USER or other INI handle
                      HINI hiniSystem,    // in: HINI_SYSTEM or other INI handle
                      const char *pcszActiveHandles,
                      HHANDLES *phHandles)
{
    APIRET arc = NO_ERROR;

    if (!phHandles)
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        PSZ pszKeysList;
        if (!(arc = prfhQueryKeysForApp(hiniSystem,
                                        pcszActiveHandles,
                                        &pszKeysList)))
        {
            PHANDLESBUF pReturn = NULL;

            ULONG   ulHighestBlock = 0,
                    ul,
                    cbTotal;
            PBYTE   pbData;

            const char *pKey2 = pszKeysList;
            while (*pKey2)
            {
                if (!memicmp((PVOID)pKey2, "BLOCK", 5))
                {
                    ULONG ulBlockThis = atoi(pKey2 + 5);
                    if (ulBlockThis > ulHighestBlock)
                        ulHighestBlock = ulBlockThis;
                }

                pKey2 += strlen(pKey2)+1; // next key
            }

            free(pszKeysList);

            if (!ulHighestBlock)
                arc = ERROR_WPH_NO_HANDLES_DATA;
            else
            {
                // now go read the data
                // (BLOCK1, BLOCK2, ..., BLOCKn)
                cbTotal = 0;
                pbData = NULL;
                for (ul = 1;
                     ul <= ulHighestBlock;
                     ul++)
                {
                    ULONG   cbBlockThis;
                    CHAR    szBlockThis[10];
                    sprintf(szBlockThis, "BLOCK%d", ul);
                    if (!PrfQueryProfileSize(hiniSystem,
                                             (PSZ)pcszActiveHandles,
                                             szBlockThis,
                                             &cbBlockThis))
                    {
                        arc = ERROR_WPH_PRFQUERYPROFILESIZE_BLOCK;
                        break;
                    }
                    else
                    {
                        ULONG   cbTotalOld = cbTotal;
                        cbTotal += cbBlockThis;
                        if (!(pbData = (BYTE*)realloc(pbData, cbTotal)))
                                // on first call, pbData is NULL and this
                                // behaves like malloc()
                        {
                            arc = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }

                        if (!PrfQueryProfileData(hiniSystem,
                                                 (PSZ)pcszActiveHandles,
                                                 szBlockThis,
                                                 pbData + cbTotalOld,
                                                 &cbBlockThis))
                        {
                            arc = ERROR_WPH_PRFQUERYPROFILEDATA_BLOCK;
                            break;
                        }
                    }
                }
            }

            if (!arc)
            {
                // all went OK:
                if (pReturn = NEW(HANDLESBUF))
                {
                    ZERO(pReturn);

                    treeInit(&pReturn->DrivesTree,
                             &pReturn->cDrives);

                    pReturn->pbData = pbData;
                    pReturn->cbData = cbTotal;

                    // and load the hiwords too
                    if (!(arc = wphQueryBaseClassesHiwords(hiniUser,
                                                           &pReturn->usHiwordAbstract,
                                                           &pReturn->usHiwordFileSystem)))
                        *phHandles = (HHANDLES)pReturn;
                }
                else
                    arc = ERROR_NOT_ENOUGH_MEMORY;
            }

            if (arc)
                // error:
                wphFreeHandles((HHANDLES*)&pReturn);
        }
    }

    return arc;
}

/*
 *@@ wphFreeHandles:
 *      frees all data allocated by wphLoadHandles
 *      and sets *ppHandlesBuf to NULL, for safety.
 *
 *@@added V0.9.16 (2001-10-02) [umoeller]
 */

APIRET wphFreeHandles(HHANDLES *phHandles)
{
    APIRET arc = NO_ERROR;

    PHANDLESBUF pHandlesBuf;
    if (    (phHandles)
         && (pHandlesBuf = (PHANDLESBUF)*phHandles)
       )
    {
        PBYTE pbData;

        NukeNameTrees(pHandlesBuf);

        if (pbData = pHandlesBuf->pbData)
            free(pbData);

        free(pHandlesBuf);
        *phHandles = NULLHANDLE;
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

/* ******************************************************************
 *
 *   Get HOBJECT from filename
 *
 ********************************************************************/

/*
 *@@ wphSearchBufferForHandle:
 *      returns the 16-bit file-system handle which corresponds
 *      to pszFilename, searching pHandlesBuffer. Note that you
 *      must OR the return value with the proper hiword to make
 *      this a valid WPS file-system handle.
 *
 *      You must pass a handles buffer to this function which
 *      has been filled using wphReadAllBlocks above.
 *
 *      This gets called by the one-shot function
 *      wphQueryHandleFromPath.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_FILE_NOT_FOUND
 *
 *      --  ERROR_INVALID_NAME
 *
 *      --  ERROR_WPH_CORRUPT_HANDLES_DATA
 *
 *      --  ERROR_WPH_CANNOT_FIND_HANDLE: no handle exists for the
 *          given filename.
 *
 *@@changed V0.9.16 (2001-10-19) [umoeller]: rewritten
 *@@changed V1.0.8 (2008-05-25) [pr]: made DBCS safe @@fixes 1070
 */

APIRET wphSearchBufferForHandle(HHANDLES hHandles,
                                PCSZ pcszFile,  // in: fully qlf'd filename to search for
                                PUSHORT pusHandle)  // out: 16-bit handle
{
    APIRET arc = NO_ERROR;

    PHANDLESBUF pHandlesBuf;

    _Pmpf((__FUNCTION__ ": entering"));

    if (    (hHandles)
         && (pHandlesBuf = (PHANDLESBUF)hHandles)
       )
    {
        // rebuild cache
        if (!pHandlesBuf->fCacheValid)
            arc = wphRebuildNodeHashTable(hHandles,
                                          TRUE);        // fail on errors

        if (!arc)
        {
            // We can thus work our way through the buffer by splitting the
            // fully qualified filename that we're searching for into the
            // different directory names and, each time, searching for the
            // corresponding NODE. If we have found that, we go for the next.
            // Example for C:\OS2\E.EXE:
            //   1) first search for the "C:" NODE
            //   2) then find the "OS2" node which has "C" as its parent NODE
            //      (we do this by comparing the parent object handles)
            //   3) then find the "E.EXE" NODE the same way
            // The "E.EXE" NODE then has the object handle we're looking for.

            // make a copy of the filename so we can play
            PSZ     pszFilename = strdup(pcszFile),
                    pEnd = NULL;

            nlsUpper(pszFilename);  // V1.0.8 (2008-05-25)

            // 1) OK, find the drive.

            // If this is an UNC name, the DRIVE node has the form
            // \\SERVER\RESOURCE.
            if (    (*pszFilename == '\\')
                 && (*(pszFilename + 1) == '\\')
               )
            {
                // UNC:
                // @@todo
            }
            else if (*(pszFilename + 1) == ':')
                // extract the drive then (without \)
                pEnd = pszFilename + 2;

            if (!pEnd)
                arc = ERROR_INVALID_NAME;
            else
            {
                PDRIVETREENODE pDrive = NULL;
                PNODETREENODE pNode;

                // find the DRIVE node
                CHAR cOld = *pEnd;
                *pEnd = 0;

                _Pmpf(("  searching for drive \"%s\"", pszFilename));

                if (!(pDrive = (PDRIVETREENODE)treeFind(pHandlesBuf->DrivesTree,
                                                        (ULONG)pszFilename,   // drive name
                                                        treeCompareStrings)))
                    arc = ERROR_WPH_NO_MATCHING_DRIVE_BLOCK;
                // find the root dir, which has the same name
                else if (!(pNode = (PNODETREENODE)treeFind(pDrive->ChildrenTree,
                                                           (ULONG)pszFilename,
                                                           treeCompareStrings)))
                    arc = ERROR_WPH_NO_MATCHING_ROOT_DIR;
                else
                {
                    // now we got the root dir... go for next path component
                    while (    (pEnd)
                            && (*pEnd = cOld)       // not null char
                            && (!arc)
                          )
                    {
                        // got another path component to search:
                        PSZ pCurrent = pEnd + 1,
                            pNext;

                        if (pNext = strchr(pCurrent, '\\'))
                        {
                            cOld = *pNext;
                            *pNext = 0;
                        }
                        else
                            // done:
                            cOld = 0;

                        _Pmpf(("  searching for node \"%s\"", pCurrent));

                        // find the next node
                        if (!(pNode = (PNODETREENODE)treeFind(pNode->ChildrenTree,
                                                              (ULONG)pCurrent,
                                                              treeCompareStrings)))
                            arc = ERROR_WPH_CANNOT_FIND_HANDLE;

                        pEnd = pNext;
                    }

                    if (!arc && pNode)
                        // found everything:
                        *pusHandle = pNode->pNode->usHandle;
                }
            } // end while

            free(pszFilename);
        }
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    _Pmpf((__FUNCTION__ ": returning %d", arc));

    // not found: end of buffer reached
    return arc;
}

/*
 *@@ wphQueryHandleFromPath:
 *      finds the object handle for the given fully qualified
 *      filename.
 *      This is a one-shot function, using wphQueryActiveHandles,
 *      wphReadAllBlocks, and wphSearchBufferForHandle.
 *
 *      Returns:
 *
 *      --  NO_ERROR: *phobj has received the object handle.
 *
 *      --  ERROR_FILE_NOT_FOUND: file does not exist.
 *
 *      plus the error codes of the other wph* functions called.
 *
 *@@changed V0.9.16 (2001-10-02) [umoeller]: rewritten
 */

APIRET wphQueryHandleFromPath(HINI hiniUser,      // in: HINI_USER or other INI handle
                              HINI hiniSystem,    // in: HINI_SYSTEM or other INI handle
                              const char *pcszName,    // in: fully qlf'd filename
                              HOBJECT *phobj)      // out: object handle found if NO_ERROR
{
    APIRET      arc = NO_ERROR;

    volatile PSZ         pszActiveHandles = NULL; // XWP V1.0.4 (2005-10-09) [pr]
    volatile HHANDLES    hHandles = NULLHANDLE;

    TRY_LOUD(excpt1)
    {
        // not found there: check the handles then

        if (arc = wphQueryActiveHandles(hiniSystem, (PSZ *) &pszActiveHandles))
            _Pmpf((__FUNCTION__ ": wphQueryActiveHandles returned %d", arc));
        else
        {
            if (arc = wphLoadHandles(hiniUser,
                                     hiniSystem,
                                     pszActiveHandles,
                                     (HHANDLES *) &hHandles))
                _Pmpf((__FUNCTION__ ": wphLoadHandles returned %d", arc));
            else
            {
                USHORT      usObjID;
                CHAR        szFullPath[2*CCHMAXPATH];
                _fullpath(szFullPath, (PSZ)pcszName, sizeof(szFullPath));

                // search that buffer
                if (!(arc = wphSearchBufferForHandle(hHandles,
                                                     szFullPath,
                                                     &usObjID)))
                    // found: OR 0x30000
                    *phobj = usObjID | (((PHANDLESBUF)hHandles)->usHiwordFileSystem << 16);
                else
                    arc = ERROR_FILE_NOT_FOUND;
            }
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION; // V0.9.19 (2002-07-01) [umoeller]
    } END_CATCH();

    if (pszActiveHandles)
        free(pszActiveHandles);
    if (hHandles)
        wphFreeHandles((HHANDLES *) &hHandles);

    return arc;
}

/* ******************************************************************
 *
 *   Get filename from HOBJECT
 *
 ********************************************************************/

/*
 *@@ ComposeThis:
 *      helper for wphComposePath recursion.
 *
 *@@added V0.9.16 (2001-10-02) [umoeller]
 *@@changed V0.9.19 (2002-04-14) [umoeller]: fixed wrong error for parent handles
 */

STATIC APIRET ComposeThis(PHANDLESBUF pHandlesBuf,
                          USHORT usHandle,         // in: handle to search for
                          PXSTRING pstrFilename,   // in/out: filename
                          ULONG ulLevel,           // in: recursion level (initially 0)
                          PNODE *ppNode)           // out: node found (ptr can be NULL)
{
    APIRET          arc = NO_ERROR;
    PNODETREENODE   pTreeNode;
    PNODE           pNode;
    if (    (pTreeNode = pHandlesBuf->NodeHashTable[usHandle])
         && (pNode = pTreeNode->pNode)
       )
    {
        // handle exists:
        if (pNode->usParentHandle)
        {
            // node has parent:
            // recurse first
            if (!(arc = ComposeThis(pHandlesBuf,
                                    pNode->usParentHandle,
                                    pstrFilename,
                                    ulLevel + 1,
                                    ppNode)))
            {
                // no error:
                xstrcatc(pstrFilename, '\\');
                xstrcat(pstrFilename, pNode->szName, pNode->usNameSize);
            }
        }
        else
            // no parent:
            xstrcpy(pstrFilename, pNode->szName, pNode->usNameSize);
    }
    else
        // handle not found:
        if (ulLevel == 0)       // V0.9.19 (2002-04-14) [umoeller]
            arc = ERROR_INVALID_HANDLE;
        else
            arc = ERROR_WPH_INVALID_PARENT_HANDLE;

    if (!arc)
        if (ppNode)
            *ppNode = pNode;

    return arc;
}

/*
 *@@ wphComposePath:
 *      returns the fully qualified path name for the specified
 *      file-system handle. This function is very fast because
 *      it uses a hash table for all the handles internally.
 *
 *      Warning: This calls a helper, which recurses.
 *
 *      This returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_WPH_CORRUPT_HANDLES_DATA: buffer data cannot be parsed.
 *
 *      --  ERROR_WPH_INVALID_HANDLE: usHandle cannot be found.
 *
 *      --  ERROR_WPH_INVALID_PARENT_HANDLE: a handle was found
 *          that has a broken parent handle.
 *
 *      --  ERROR_BUFFER_OVERFLOW: cbFilename is too small to
 *          hold the full path that was composed.
 *
 *@@added V0.9.16 (2001-10-02) [umoeller]
 */

APIRET wphComposePath(HHANDLES hHandles,
                      USHORT usHandle,      // in: loword of handle to search for
                      PSZ pszFilename,
                      ULONG cbFilename,
                      PNODE *ppNode)        // out: node found (ptr can be NULL)
{
    APIRET arc = NO_ERROR;

    PHANDLESBUF pHandlesBuf;
    if (    (hHandles)
         && (pHandlesBuf = (PHANDLESBUF)hHandles)
       )
    {
        TRY_LOUD(excpt1)
        {
            if (!pHandlesBuf->fCacheValid)
                arc = wphRebuildNodeHashTable(hHandles,
                                              TRUE);        // fail on errors

            if (!arc)
            {
                XSTRING str;
                xstrInit(&str, CCHMAXPATH);
                if (!(arc = ComposeThis(pHandlesBuf,
                                        usHandle,
                                        &str,
                                        0,      // initial recursion level
                                        ppNode)))
                    if (str.ulLength > cbFilename - 1)
                        arc = ERROR_BUFFER_OVERFLOW;
                    else
                        memcpy(pszFilename,
                               str.psz,
                               str.ulLength + 1);
                xstrClear(&str);
            }
        }
        CATCH(excpt1)
        {
            arc = ERROR_PROTECTION_VIOLATION; // V0.9.19 (2002-07-01) [umoeller]
        } END_CATCH();
    }

    return arc;
}

/*
 *@@ wphQueryPathFromHandle:
 *      reverse to wphQueryHandleFromPath, this gets the
 *      filename for hObject.
 *      This is a one-shot function, using wphQueryActiveHandles,
 *      wphLoadHandles, and wphComposePath.
 *      As a result, this function is _very_ expensive.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_INVALID_HANDLE: hObject is invalid.
 *
 *      --  ERROR_WPH_NOT_FILESYSTEM_HANDLE: hObject's hiword is wrong.
 *
 *@@changed V0.9.16 (2001-10-02) [umoeller]: rewritten
 */

APIRET wphQueryPathFromHandle(HINI hiniUser,      // in: HINI_USER or other INI handle
                              HINI hiniSystem,    // in: HINI_SYSTEM or other INI handle
                              HOBJECT hObject,    // in: 32-bit object handle
                              PSZ pszFilename,    // out: filename, if found
                              ULONG cbFilename)   // in: sizeof(*pszFilename)
{
    APIRET arc = NO_ERROR;

    TRY_LOUD(excpt1)
    {
        PSZ pszActiveHandles;
        if (arc = wphQueryActiveHandles(hiniSystem, &pszActiveHandles))
            _Pmpf((__FUNCTION__ ": wphQueryActiveHandles returned %d", arc));
        else
        {
            HHANDLES hHandles;
            if (arc = wphLoadHandles(hiniUser,
                                     hiniSystem,
                                     pszActiveHandles,
                                     &hHandles))
                _Pmpf((__FUNCTION__ ": wphLoadHandles returned %d", arc));
            else
            {
                USHORT usHiwordFileSystem = ((PHANDLESBUF)hHandles)->usHiwordFileSystem;

                // is this really a file-system object?
                if (HIUSHORT(hObject) == usHiwordFileSystem)
                {
                    // use loword only
                    USHORT      usObjID = LOUSHORT(hObject);

                    memset(pszFilename, 0, cbFilename);
                    arc = wphComposePath(hHandles,
                                         usObjID,
                                         pszFilename,
                                         cbFilename,
                                         NULL);

                    // _Pmpf((__FUNCTION__ ": wphFindPartName returned %d", arc));
                }
                else
                    arc = ERROR_WPH_NOT_FILESYSTEM_HANDLE;

                wphFreeHandles(&hHandles);
            }

            free(pszActiveHandles);
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION; // V0.9.19 (2002-07-01) [umoeller]
    } END_CATCH();

    return arc;
}

/*
 *@@ wphDescribeError:
 *      returns an error description for one of the handles
 *      engine errors, or NULL if the error code is not
 *      recognized.
 *
 *@@added V0.9.19 (2002-07-01) [umoeller]
 */

PCSZ wphDescribeError(APIRET arc)
{
    switch (arc)
    {
        case ERROR_WPH_NO_BASECLASS_DATA:
            return "Cannot find PM_Workplace:BaseClass in OS2.INI";

        case ERROR_WPH_NO_ACTIVEHANDLES_DATA:
            return "Cannot find PM_Workplace:ActiveHandles in OS2SYS.INI";

        case ERROR_WPH_INCOMPLETE_BASECLASS_DATA:
            return "PM_Workplace:ActiveHandles in OS2SYS.INI is incomplete";

        case ERROR_WPH_NO_HANDLES_DATA:
            return "Active handles block in OS2SYS.INI is empty";

        case ERROR_WPH_CORRUPT_HANDLES_DATA:
            return "Cannot parse data in active handles block in OS2SYS.INI";

        case ERROR_WPH_INVALID_PARENT_HANDLE:
            return "Handle has invalid parent handle";

        case ERROR_WPH_CANNOT_FIND_HANDLE:
            return "No handle exists for the given filename";

        case ERROR_WPH_DRIV_TREEINSERT_FAILED:
            return "Duplicate DRIV node (treeInsert failed)";

        case ERROR_WPH_NODE_TREEINSERT_FAILED:
            return "Duplicate NODE node (treeInsert failed)";

        case ERROR_WPH_NODE_BEFORE_DRIV:
            return "NODE node before DRIV node";

        case ERROR_WPH_NO_MATCHING_DRIVE_BLOCK:
            return "No matching DRIV node";

        case ERROR_WPH_NO_MATCHING_ROOT_DIR:
            return "No matching root directory";

        case ERROR_WPH_NOT_FILESYSTEM_HANDLE:
            return "Handle is not a file-system handle";

        case ERROR_WPH_PRFQUERYPROFILESIZE_BLOCK:
            return "PrfQueryProfileSize failed on reading one BLOCK in OS2SYS.INI";

        case ERROR_WPH_PRFQUERYPROFILEDATA_BLOCK:
            return "PrfQueryProfileData failed on reading one BLOCK in OS2SYS.INI";
    }

    return NULL;
}
