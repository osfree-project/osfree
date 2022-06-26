
/*
 *@@sourcefile folder.c:
 *      folder content implementation code. This contains
 *
 *      1)  folder mutex wrappers,
 *
 *      2)  folder content enumeration,
 *
 *      3)  fast binary balanced trees for folder contents,
 *
 *      4)  fdrQueryAwakeFSObject for fast finding of
 *          an awake file-system object,
 *
 *      4)  a replacement populate (fdrPopulate).
 *
 *      Some of this code is used all the time, some only
 *      if "turbo folders" are enabled.
 *
 *      A couple of remarks about the WPS's folder content
 *      management is in order.
 *
 *      1)  You need to differentiate between "folder content"
 *          and "objects in folder views", which are not necessarily
 *          the same, albeit hard to tell apart because of the
 *          ugly WPS implementation and bad documentation.
 *
 *          As soon as an object is made awake, it is added to
 *          the folder's _content_, which is essentially a
 *          linked list in folder and object instance data.
 *          When the object is made dormant, it is removed again.
 *          For this, WPFolder has the wpAddToContent and
 *          wpDeleteFromContent methods, which _always_ get
 *          called on the folder of the object that is being
 *          made awake or dormant. The standard WPFolder
 *          implementations of these two methods maintain the
 *          folder content lists.
 *
 *      2)  Awaking an object does not necessarily mean that the
 *          object is also inserted into open views of the folder.
 *          IBM recommends to override the aforementioned two
 *          methods for supporting add/remove in new folder views.
 *
 *      3)  WPFolder itself does not seem to contain a real
 *          linked list of its objects. Instead, each _object_
 *          has two "next" and "previous" instance variables,
 *          which point to the next and previous object in
 *          its folder.
 *
 *          Apparently, WPFolder::wpQueryContent iterates over
 *          these things.
 *
 *          Starting with V0.9.16, XFolder maintains two binary
 *          balanced trees of the file-system and abstract
 *          objects which have been added to the folder to allow
 *          for speedy lookup of whether an object is already
 *          awake. I believe this new code is way faster than
 *          the WPS way of looking this up.
 *
 *          For this, I have overridden wpAddToContent and
 *          wpDeleteFromContent. In addition, to refresh this
 *          data when objects are renamed, I have also added
 *          XWPFileSystem::wpSetTitleAndRenameFile and
 *          XWPFileSystem::xwpQueryUpperRealName.
 *
 *      4)  At least three additional folder mutexes are involved
 *          in folder content protection. See fdrRequestFolderMutexSem,
 *          fdrRequestFolderWriteMutexSem, and fdrRequestFindMutexSem.
 *
 *      This file is ALL new with V0.9.16 and contains
 *      code formerly in folder.c and wpsh.c.
 *
 *      Function prefix for this file:
 *      --  fdr*
 *
 *@@added V0.9.16 (2001-10-23) [umoeller]
 *@@header "filesys\folder.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINRECTANGLES
#define INCL_WINSYS             // needed for presparams
#define INCL_WINMENUS
#define INCL_WINTIMER
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_GPILOGCOLORTABLE
#define INCL_GPIPRIMITIVES
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\eah.h"                // extended attributes helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\threads.h"            // thread helpers
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"
#include "xfdisk.ih"
#include "xwpfsys.ih"
#include "xfldr.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

// #include "filesys\fdrmenus.h"           // shared folder menu logic
#include "filesys\fileops.h"            // file operations implementation
#include "filesys\filesys.h"            // various file-system object implementation code
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Folder semaphores
 *
 ********************************************************************/

/*
 *@@ GetMonitorObject:
 *      gets the RWMonitor object for the specified
 *      folder. See fdrRequestFolderWriteMutexSem.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: optimized
 */

STATIC SOMAny* GetMonitorObject(WPFolder *somSelf)
{
#ifdef __DEBUG__
    if (_somIsA(somSelf, _WPFolder))
#endif
    {
        XFolderData *somThis = XFolderGetData(somSelf);

        if (!_pMonitor)
        {
            // first call on this folder:
            // get the object and store it in the instance data
            _pMonitor = _wpQueryRWMonitorObject(somSelf);
                    // we can use the method now V0.9.20 (2002-07-25) [umoeller]
        }

        return _pMonitor;
    }

#ifdef __DEBUG__
    cmnLog(__FILE__, __LINE__, __FUNCTION__,
           "Function invoked on non-folder object.");
    return NULL;
#endif
}

/*
 *@@ fdrRequestFolderWriteMutexSem:
 *      requests the "folder write" mutex for the folder.
 *
 *      Apparently this extra semaphore gets requested
 *      each time before the folder's contents are actually
 *      modified. It gets requested for every single
 *      object that is added or removed from the contents.
 *
 *      Strangely, this semaphore is implemented through
 *      an extra SOM class called RWMonitor, which is not
 *      derived from WPObject, but from SOMObject directly.
 *      You can see that class with the "WPS class list"
 *      object if you enable "Show all SOM classes" in its
 *      settings notebook.
 *
 *      Because that class is completely undocumented and
 *      not prototyped, I have added this helper func
 *      to request the folder write mutex from this object.
 *
 *      Returns:
 *
 *      --  NO_ERROR: semaphore was successfully requested.
 *
 *      --  -1: error resolving methods. A log entry should
 *          have been added also.
 *
 *      --  other: error codes from DosRequestMutexSem probably.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

ULONG fdrRequestFolderWriteMutexSem(WPFolder *somSelf)
{
    ULONG ulrc = -1;

    SOMAny *pMonitor;

    if (pMonitor = GetMonitorObject(somSelf))
    {
        // this is never overridden so we can safely use
        // a static variable here
        static xfTD_RequestWrite pRequestWrite = NULL;

        if (!pRequestWrite)
            // first call:
            pRequestWrite = (xfTD_RequestWrite)wpshResolveFor(
                                           pMonitor,
                                           NULL,
                                           "RequestWrite");

        if (pRequestWrite)
            ulrc = pRequestWrite(pMonitor);
    }

    return ulrc;
}

/*
 *@@ fdrReleaseFolderWriteMutexSem:
 *      releases the "folder write" mutex requested by
 *      fdrRequestFolderWriteMutexSem.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

ULONG fdrReleaseFolderWriteMutexSem(WPFolder *somSelf)
{
    ULONG ulrc = -1;

    SOMAny *pMonitor;

    if (pMonitor = GetMonitorObject(somSelf))
    {
        static xfTD_ReleaseWrite pReleaseWrite = NULL;

        if (!pReleaseWrite)
            pReleaseWrite = (xfTD_ReleaseWrite)wpshResolveFor(
                                           pMonitor,
                                           NULL,
                                           "ReleaseWrite");

        if (pReleaseWrite)
            ulrc = pReleaseWrite(pMonitor);
    }

    return ulrc;
}

/*
 * fdrFlushNotifications:
 *      invokes WPFolder::wpFlushNotifications, which
 *      is only published with the Warp 4 toolkit.
 *
 *added V0.9.6 (2000-10-25) [umoeller]
 *changed V0.9.16 (2001-10-25) [umoeller]: moved this here from wpsh.c, changed prefix
 *removed V1.0.1 (2002-12-08) [umoeller], we have method code now
 */

/*
ULONG fdrFlushNotifications(WPFolder *somSelf)
{
    ULONG ulrc = 0;

    xfTD_wpFlushNotifications _wpFlushNotifications;

    if (_wpFlushNotifications
        = (xfTD_wpFlushNotifications)wpshResolveFor(somSelf,
                                                    NULL, // use somSelf's class
                                                    "wpFlushNotifications"))
        ulrc = _wpFlushNotifications(somSelf);

    return ulrc;
}
*/

/*
 *@@ fdrGetNotifySem:
 *      calls M_WPFolder::wpclsGetNotifySem to lock out
 *      the WPS auto-refresh-folder threads.
 *
 *      Even though we have method code to call this
 *      directly now, this mostly gets called in
 *      the refresh code, which starts going before
 *      the _WPFolder global works, so this wrapper
 *      is still useful (V1.0.1 (2002-12-08) [umoeller]).
 *
 *      Note that this requests a system-wide lock.
 *
 *      Be warned also that -- as opposed to all other
 *      "request" functions -- wpclsGetNotifySem
 *      returns a BOOL, not an APIRET.
 *
 *@@added V0.9.6 (2000-10-25) [umoeller]
 *@@changed V0.9.16 (2001-10-25) [umoeller]: moved this here from wpsh.c, changed prefix
 *@@changed V1.0.1 (2002-12-08) [umoeller]: now calling method directly, but we still need the wrapper
 */

BOOL fdrGetNotifySem(ULONG ulTimeout)
{
    // static xfTD_wpclsGetNotifySem _wpclsGetNotifySem = NULL;

    M_WPFolder *pWPFolder = _WPFolder;
            // THIS RETURNS NULL UNTIL THE FOLDER CLASS IS INITIALIZED

    if (pWPFolder)
    {
        /* if (    (_wpclsGetNotifySem)
                // first call: resolve...
             || (_wpclsGetNotifySem = (xfTD_wpclsGetNotifySem)wpshResolveFor(
                                                pWPFolder,
                                                NULL,
                                                "wpclsGetNotifySem"))
           ) */
            return _wpclsGetNotifySem(pWPFolder, ulTimeout);
    }

    return FALSE;
}

/*
 *@@fdrReleaseNotifySem:
 *
 *      Even though we have method code to call this
 *      directly now, this mostly gets called in
 *      the refresh code, which starts going before
 *      the _WPFolder global works, so this wrapper
 *      is still useful (V1.0.1 (2002-12-08) [umoeller]).
 *
 *@@added V0.9.6 (2000-10-25) [umoeller]
 *@@changed V0.9.16 (2001-10-25) [umoeller]: moved this here from wpsh.c, changed prefix
 *@@changed V1.0.1 (2002-12-08) [umoeller]: now calling method directly, but we still need the wrapper
 */

VOID fdrReleaseNotifySem(VOID)
{
    // static xfTD_wpclsReleaseNotifySem _wpclsReleaseNotifySem = NULL;
            // V1.0.1 (2002-12-08) [umoeller]

    M_WPFolder *pWPFolder = _WPFolder;
            // THIS RETURNS NULL UNTIL THE FOLDER CLASS IS INITIALIZED

    if (pWPFolder)
    {
        /* if (    (_wpclsReleaseNotifySem)
                // first call: resolve...
             || (_wpclsReleaseNotifySem = (xfTD_wpclsReleaseNotifySem)wpshResolveFor(
                                                pWPFolder,
                                                NULL,
                                                "wpclsReleaseNotifySem"))
           ) */
            _wpclsReleaseNotifySem(pWPFolder);
    }
}

/* ******************************************************************
 *
 *   Folder content management
 *
 ********************************************************************/

/*
 *@@ FastFindFSFromUpperName:
 *      retrieves the awake file-system object with the
 *      specified name (which _must_ be upper-cased)
 *      from the folder contents tree.
 *
 *      Preconditions:
 *
 *      --  Caller must hold the folder mutex sem.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 *@@changed V0.9.18 (2002-02-06) [umoeller]: removed mutex request, renamed
 */

STATIC WPObject* FastFindFSFromUpperName(WPFolder *pFolder,
                                         const char *pcszUpperShortName)
{
    XFolderData *somThis = XFolderGetData(pFolder);
    PFDRCONTENTITEM pNode;
    if (pNode = (PFDRCONTENTITEM)treeFind(
                         _FileSystemsTreeRoot,
                         (ULONG)pcszUpperShortName,
                         treeCompareStrings))
        return pNode->pobj;

    return NULL;
}

/*
 *@@ FastFindFSFromUpperName:
 *      calls FastFindFSFromUpperName in a folder mutex sem
 *      request block properly.
 *
 *@@added V0.9.18 (2002-02-06) [umoeller]
 */

STATIC WPObject* SafeFindFSFromUpperName(WPFolder *pFolder,
                                         const char *pcszUpperShortName)
{
    WPObject *pobjReturn = NULL;
    BOOL fFolderLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, SEM_INDEFINITE_WAIT))
        {
            pobjReturn = FastFindFSFromUpperName(pFolder,
                                                 pcszUpperShortName);
        }
    }
    CATCH(excpt1)
    {
        pobjReturn = NULL;
    } END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    return pobjReturn;
}

/*
 *@@ fdrFastFindFSFromName:
 *      goes thru the folder contents to find the first
 *      file-system object with the specified real name
 *      (not title!).
 *
 *      pcszShortName is looked for without respect to
 *      case.
 *
 *      Preconditions:
 *
 *      --  Caller must hold the folder mutex sem.
 *
 *@@added V0.9.9 (2001-02-01) [umoeller]
 *@@changed V0.9.16 (2001-10-25) [umoeller]: now using fast content trees
 *@@changed V0.9.18 (2002-02-06) [umoeller]: removed mutex request, renamed
 */

WPObject* fdrFastFindFSFromName(WPFolder *pFolder,
                                const char *pcszShortName)  // in: short real name to look for
{
    if (    (pcszShortName)
         && (*pcszShortName)
       )
    {
        // all the following rewritten V0.9.16 (2001-10-25) [umoeller]

        // avoid two strlen's (speed)
        ULONG   ulLength = strlen(pcszShortName);

        // upper-case the short name
        PSZ pszUpperRealName = _alloca(ulLength + 1);
        memcpy(pszUpperRealName, pcszShortName, ulLength + 1);
        nlsUpper(pszUpperRealName);

        return FastFindFSFromUpperName(pFolder,
                                       pszUpperRealName);
    }

    return NULL;
}

/*
 *@@ fdrSafeFindFSFromName:
 *      calls fdrFastFindFSFromName in a folder mutex sem
 *      request block properly.
 *
 *@@added V0.9.18 (2002-02-06) [umoeller]
 */

WPObject* fdrSafeFindFSFromName(WPFolder *pFolder,
                                const char *pcszShortName)  // in: short real name to look for
{
    WPObject *pobjReturn = NULL;
    BOOL fFolderLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, SEM_INDEFINITE_WAIT))
        {
            pobjReturn = fdrFastFindFSFromName(pFolder,
                                               pcszShortName);
        }
    }
    CATCH(excpt1)
    {
        pobjReturn = NULL;
    } END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    return pobjReturn;
}

/*
 *@@ HackContentPointers:
 *
 *      Preconditions:
 *
 *      --  The caller must hold the folder write mutex.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: rewritten to use SOM attributes now that we have them
 */

STATIC BOOL HackContentPointers(WPFolder *somSelf,
                                XFolderData *somThis,
                                WPObject *pObject)
{
    BOOL brc = FALSE;

    if (pObject)
    {
        BOOL fSubObjectLocked = FALSE;

        TRY_LOUD(excpt1)        // V0.9.9 (2001-04-01) [umoeller]
        {
            PIBMFOLDERDATA pFdrData;

            if (!(pFdrData = (PIBMFOLDERDATA)_pvWPFolderData))
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "Cannot get IBM folder data");
            else if (fSubObjectLocked = !_wpRequestObjectMutexSem(pObject, SEM_INDEFINITE_WAIT))
            {
                // this strange thing gets called by the original
                // wpAddToContent... it's a flag which shows whether
                // the object is in any container at all. We want
                // to set this to TRUE.
                PULONG pulContainerFlag = _wpQueryContainerFlagPtr(pObject);

                if (!*pulContainerFlag)
                   *pulContainerFlag = TRUE;

                // _PmpfF(("adding %s to %d",
                //         _wpQueryTitle(pObject),
                //         _wpQueryTitle(somSelf) ));

                // now check our contents...
                if (pFdrData->LastObj)
                {
                    // we had objects before:
                    // store new object as next object for
                    // previously last object
                    _wpSetNextObj(pFdrData->LastObj, pObject);
                    // store new object as new last object
                    pFdrData->LastObj = pObject;
                }
                else
                {
                    // no objects yet:
                    pFdrData->FirstObj = pObject;
                    pFdrData->LastObj = pObject;
                }

                // in any case, set the new object's "next object" to NULL
                // _xwpSetNextObj(pObject, NULL);   replaced V0.9.20 (2002-07-25) [umoeller]
                _wpSetNextObj(pObject, NULL);

                // for each object that was added, lock
                // the folder...
                _wpLockObject(somSelf);

                brc = TRUE;
            }
        }
        CATCH(excpt1)
        {
            brc = FALSE;
        } END_CATCH();

        // release the mutexes in reverse order V0.9.9 (2001-04-01) [umoeller]
        if (fSubObjectLocked)
            _wpReleaseObjectMutexSem(pObject);
    }

    return brc;
}

/*
 *@@ fdrAddToContent:
 *      implementation for the XFolder::wpAddToContent override.
 *
 *      Preconditions:
 *
 *      --  The caller must hold the folder write mutex.
 *
 *@@changed V0.9.9 (2001-04-02) [umoeller]: fixed mutex release order
 *@@changed V0.9.9 (2001-04-02) [umoeller]: removed object mutex request on folder
 *@@changed V0.9.16 (2001-10-25) [umoeller]: moved old code to HackContentPointers, added tree maintenance
 */

BOOL fdrAddToContent(WPFolder *somSelf,
                     WPObject *pObject,
                     BOOL *pfCallParent)        // out: call parent method
{
    BOOL    brc = TRUE;

    XFolderData *somThis = XFolderGetData(somSelf);
    PFDRCONTENTITEM pNew;
    ULONG flObject = objQueryFlags(pObject);

    if (_fDisableAutoCnrAdd)
    {
        // do not call the parent!!
        *pfCallParent = FALSE;
        // call our own implementation instead
        brc = HackContentPointers(somSelf, somThis, pObject);
    }

    // raise total objects count
    _cObjects++;

    // add to contents tree
    if (flObject & OBJFL_WPFILESYSTEM)
    {
        // WPFileSystem added:
        // add a new tree node and sort it according
        // to the object's upper-case real name
        PSZ pszUpperRealName;
        if (    (pszUpperRealName = _xwpQueryUpperRealName(pObject))
             && (pNew = NEW(FDRCONTENTITEM))
           )
        {
            pNew->Tree.ulKey = (ULONG)pszUpperRealName;
            pNew->pobj = pObject;

            if (treeInsert((TREE**)&_FileSystemsTreeRoot,
                           &_cFileSystems,
                           (TREE*)pNew,
                           treeCompareStrings))
            {
                // wow, this failed:
                PFDRCONTENTITEM pExisting;
                CHAR sz[CCHMAXPATH];
                _wpQueryFilename(pObject, sz, TRUE);
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "treeInsert failed for %s \"%s\" (adr 0x%lX, title \"%s\")",
                       _somGetClassName(pObject),
                       sz,
                       pObject,
                       _wpQueryTitle(pObject));

                if (pExisting = (PFDRCONTENTITEM)treeFind(
                                     _FileSystemsTreeRoot,
                                     (ULONG)pszUpperRealName,
                                     treeCompareStrings))
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                           "Existing object of %s (0x%lX, %s)",
                           _somGetClassName(pExisting->pobj),
                           pExisting->pobj,
                           (pExisting->pobj)
                               ? _wpQueryTitle(pExisting->pobj)
                               : "NULL");
                else
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                           "cannot find existing object!");

                brc = FALSE;        // V0.9.19 (2002-04-17) [umoeller]
            }
        }
    }
    else if (flObject & OBJFL_WPABSTRACT)
    {
        // WPAbstract added:
        // add a new tree node and sort it according
        // to the object's 32-bit handle (this is safe
        // because abstracts _always_ have a handle)
        HOBJECT hobj;
        if (    (hobj = _wpQueryHandle(pObject))
             && (pNew = NEW(FDRCONTENTITEM))
           )
        {
            // upper case!
            pNew->Tree.ulKey = hobj;
            pNew->pobj = pObject;

            if (treeInsert((TREE**)&_AbstractsTreeRoot,
                           &_cAbstracts,
                           (TREE*)pNew,
                           treeCompareKeys))
            {
                // wow, this failed:
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "treeInsert failed for %s 0x%lX (0x%lX, %s)",
                       _somGetClassName(pObject),
                       hobj,
                       pObject,
                       _wpQueryTitle(pObject));

                brc = FALSE;        // V0.9.19 (2002-04-17) [umoeller]
            }
        }
    }

    return brc;
}

/*
 *@@ fdrRealNameChanged:
 *      called by XWPFileSystem::wpSetRealName
 *      when an object's real name has changed. We then
 *      need to update our tree of FS objects which is
 *      sorted by real names.
 *
 *      This also sets a new upper-case real name in
 *      the file-system object's instance data.
 *
 *      Preconditions:
 *
 *      --  caller must hold the folder write mutex sem.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

BOOL fdrRealNameChanged(WPFolder *somSelf,          // in: folder of pFSObject
                        WPObject *pFSObject)        // in: FS object who is changing
{
    BOOL    brc = FALSE;
    PCSZ    pcszFailed = NULL;

    PSZ     pszOldRealName;

    if (pszOldRealName = _xwpQueryUpperRealName(pFSObject))
    {
        CHAR szNewUpperRealName[CCHMAXPATH];
        XFolderData *somThis = XFolderGetData(somSelf);
        PFDRCONTENTITEM pNode;

        _wpQueryFilename(pFSObject, szNewUpperRealName, FALSE);
        nlsUpper(szNewUpperRealName);

        PMPF_TURBOFOLDERS(("old %s, new %s", pszOldRealName, szNewUpperRealName));

        if (strcmp(pszOldRealName, szNewUpperRealName))
        {
            // real name changed:
            // find the old real name in the tree
            if (pNode = (PFDRCONTENTITEM)treeFind(
                             _FileSystemsTreeRoot,
                             (ULONG)pszOldRealName,
                             treeCompareStrings))
            {
                // 1) remove that node from the tree
                if (!treeDelete((TREE**)&_FileSystemsTreeRoot,
                                &_cFileSystems,
                                (TREE*)pNode))
                {
                    // 2) set the new real name on the fs object
                    XWPFileSystemData *somThat = XWPFileSystemGetData(pFSObject);

                    // update the fs object's instance data
                    wpshStore(pFSObject,
                              &somThat->pWszUpperRealName,
                              szNewUpperRealName,
                              NULL);
                    // refresh the tree node to point to the new buffer
                    pNode->Tree.ulKey = (ULONG)somThat->pWszUpperRealName;

                    // 3) re-insert
                    if (!treeInsert((TREE**)&_FileSystemsTreeRoot,
                                    &_cFileSystems,
                                    (TREE*)pNode,
                                    treeCompareStrings))
                    {
                        brc = TRUE;
                    }
                    else
                        pcszFailed = "treeInsert new";
                }
                else
                    pcszFailed = "treeDelete old";
            }
            else
                pcszFailed = "treeFind old";

            if (pcszFailed)
            {
                CHAR sz[CCHMAXPATH];
                _wpQueryFilename(pFSObject, sz, TRUE);
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "tree funcs failed (%s), for %s (old \"%s\", new \"%s\")",
                       pcszFailed,
                       sz,
                       pszOldRealName,
                       szNewUpperRealName);
            }
        } // end if (strcmp(pszOldRealName, szNewUpperRealName))
    }

    return brc;
}

/*
 *@@ fdrDeleteFromContent:
 *      implementation for the XFolder::wpDeleteFromContent override.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

BOOL fdrDeleteFromContent(WPFolder *somSelf,
                          WPObject* pObject)
{
    BOOL    brc = TRUE;

    XFolderData *somThis = XFolderGetData(somSelf);
    PFDRCONTENTITEM pNode;
    ULONG flObject = objQueryFlags(pObject);

    _cObjects--;

    // remove from contents tree
    if (flObject & OBJFL_WPFILESYSTEM)
    {
        // removing WPFileSystem:
        PCSZ pcszUpperRealName;
        if (pNode = (PFDRCONTENTITEM)treeFind(
                             _FileSystemsTreeRoot,
                             (ULONG)_xwpQueryUpperRealName(pObject),
                             treeCompareStrings))
        {
            if (!treeDelete((TREE**)&_FileSystemsTreeRoot,
                            &_cFileSystems,
                            (TREE*)pNode))
                brc = TRUE;
        }
    }
    else if (flObject & OBJFL_WPABSTRACT)
    {
        // removing WPAbstract:
        if (pNode = (PFDRCONTENTITEM)treeFind(
                             _AbstractsTreeRoot,
                             (ULONG)_wpQueryHandle(pObject),
                             treeCompareKeys))
        {
            if (!treeDelete((TREE**)&_AbstractsTreeRoot,
                            &_cAbstracts,
                            (TREE*)pNode))
                brc = TRUE;
        }
    }

    if (!brc)
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "tree funcs failed for %s",
               _wpQueryTitle(pObject));

    return brc;
}

/*
 *@@ fdrIsObjectFiltered:
 *      returns TRUE if pObject is currently filtered
 *      out of pFolder and should therefore not be visible.
 *
 *      An object is "filtered" if it does not match
 *      the include criteria from the "Include" page
 *      of the folder or if it has the OBJSTYLE_NOTVISIBLE
 *      style flag set.
 *
 *      This involves dealing with the undocumented
 *      "WPFilter" class, which is derived from WPTransient.
 *      As with the other interesting folder things, this
 *      class is undocumented, so once again it is
 *      impossible to deal with a standard folder feature
 *      properly without hacking into undocumented methods.
 *      Thanks a bunch, IBM.
 *
 *      Here we return TRUE only if
 *
 *      --  we can properly resolve all the methods;
 *
 *      --  _and_ the folder has a filter object;
 *
 *      --  _and_ the filter object excludes pObject
 *          from its include criteria.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: optimized to use wpQueryFldrFilter directly
 *@@changed V1.0.1 (2003-01-25) [umoeller]: fixed confusing default return value
 */

BOOL fdrIsObjectFiltered(WPFolder *pFolder,
                         WPObject *pObject)
{
    BOOL    brc = TRUE;     // if we can't find the filter, make
                            // the object visible!

    WPObject *pFilter;

    static xfTD_wpMatchesFilter s_pwpMatchesFilter = NULL;

    if (_wpQueryStyle(pObject) & OBJSTYLE_NOTVISIBLE)
        return TRUE;

    if (    (pFilter = _wpQueryFldrFilter(pFolder))        // V0.9.20 (2002-07-25) [umoeller]
            // now that we have the WPFilter object, we can try
            // to resolve the wpMatchesFilter method
         && (    (s_pwpMatchesFilter)
                 // first call: this method is never overridden,
                 // so we only resolve once for the system
              || (s_pwpMatchesFilter = (xfTD_wpMatchesFilter)wpshResolveFor(
                                                      pFilter,
                                                      NULL, // use somSelf's class
                                                      "wpMatchesFilter"))
            )
       )
    {
        if (s_pwpMatchesFilter(pFilter, pObject))
            brc = FALSE;
    }

    return brc;
}

/*
 *@@ fdrQueryContent:
 *      implementation for the XFolder::wpQueryContent override.
 *
 *      NOTE: The caller must lock the folder contents BEFORE
 *      the call. We can't this here because we can't guarantee
 *      that the folder's content list will remain valid after
 *      the call, unless the caller's processing is also protected.
 *
 *      The original implementation (WPFolder::wpQueryContent)
 *      appears to call folder mutex methods. I have not done
 *      this here. So far, it works.
 *
 *      This code ONLY gets called if XFolder::xwpSetDisableCnrAdd
 *      was called.
 *
 *@@changed V0.9.20 (2002-07-25) [umoeller]: rewritten to use SOM attributes now that we have them
 */

WPObject* fdrQueryContent(WPFolder *somSelf,
                          WPObject *pobjFind,
                          ULONG ulOption)
{
    WPObject *pobjReturn = NULL;

    TRY_LOUD(excpt1)
    {
        switch (ulOption)
        {
            case QC_FIRST:
                // that's easy
                pobjReturn = *__get_FirstObj(somSelf);
            break;

            case QC_NEXT:
                if (pobjFind)
                    pobjReturn = *__get_pobjNext(pobjFind);
            break;

            case QC_LAST:
                pobjReturn = *__get_LastObj(somSelf);
            break;
        }
    }
    CATCH(excpt1)
    {
        pobjReturn = NULL;
    } END_CATCH();

    return pobjReturn;
}

/*
 *@@ fdrQueryContentArray:
 *      returns an array of WPObject* pointers representing
 *      the folder contents. This does NOT populate the
 *      folder, but will only return the objects that are
 *      presently awake.
 *
 *      Returns NULL if the folder is empty. Otherwise
 *      *pulItems receives the array item count (NOT the
 *      array size).
 *
 *      If (flFilter & QCAFL_FILTERINSERTED), this checks each
 *      object and returns only those which have not been
 *      inserted into any container yet. Useful for
 *      wpclsInsertMultipleObjects, which will simply fail if
 *      any object has already been inserted.
 *
 *      Use free() to release the memory allocated here.
 *
 *      NOTE: The caller must lock the folder contents BEFORE
 *      the call. We can't this here because we can't guarantee
 *      that the array will remain valid after the call, unless
 *      the caller's processing is also protected.
 *
 *@@added V0.9.7 (2001-01-13) [umoeller]
 *@@changed V0.9.9 (2001-04-02) [umoeller]: added flFilter
 */

WPObject** fdrQueryContentArray(WPFolder *pFolder,
                                ULONG flFilter,           // in: filter flags
                                PULONG pulItems)
{
    WPObject** paObjects = NULL;

    TRY_LOUD(excpt1)
    {
        XFolderData *somThis = XFolderGetData(pFolder);
        if (_cObjects)
        {
            if (paObjects = (WPObject**)malloc(sizeof(WPObject*) * _cObjects))
            {
                WPObject **ppThis = paObjects;
                WPObject *pObject;

                ULONG ul = 0;
                // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
                for (   pObject = _wpQueryContent(pFolder, NULL, QC_FIRST);
                        pObject;
                        pObject = *__get_pobjNext(pObject))
                {
                    // add object if either filter is off,
                    // or if no RECORDITEM exists yet
                    if (    (!(flFilter & QCAFL_FILTERINSERTED))
                         || (!(_wpFindUseItem(pObject, USAGE_RECORD, NULL)))
                       )
                    {
                        // store obj
                        *ppThis = pObject;
                        // advance ptr
                        ppThis++;
                        // raise count
                        ul++;
                    }

                    if (ul >= _cObjects)
                        // shouldn't happen, but we don't want to
                        // crash the array
                        break;
                }

                *pulItems = ul;
            }
        }
    }
    CATCH(excpt1)
    {
        if (paObjects)
        {
            free(paObjects);
            paObjects = NULL;
        }
    } END_CATCH();

    return paObjects;
}

/*
 *@@ fdrNukeContents:
 *      deletes all the folder contents without any
 *      confirmations by invoking wpFree on each awake
 *      object. This does NOT populate the folder.
 *
 *      Note that this is not a polite way of cleaning
 *      a folder. This is ONLY used by
 *      XWPFontFolder::wpDeleteContents and
 *      XWPTrashCan::wpDeleteContents to nuke all the
 *      transient objects before those special folders
 *      get deleted themselves. This avoids the stupid
 *      "cannot delete object" messages the WPS would
 *      otherwise produce for each transient object.
 *
 *      DO NOT INVOKE THIS FUNCTION ON REGULAR FOLDERS.
 *      THERE'S NO WAY TO INTERRUPT THIS PROCESSING.
 *      THIS WOULD ALSO DELETE ALL FILES IN THE FOLDER
 *      AND ALL SUBFOLDERS.
 *
 *      Returns FALSE if killing one of the objects
 *      failed.
 *
 *@@added V0.9.9 (2001-02-08) [umoeller]
 *@@changed V0.9.12 (2001-04-29) [umoeller]: removed wpQueryContent calls
 */

BOOL fdrNukeContents(WPFolder *pFolder)
{
    BOOL        brc = FALSE,
                fFolderLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, SEM_INDEFINITE_WAIT))
        {
            ULONG   cObjects = 0,
                    ul;
            // now querying content array... we can't use wpQueryContent
            // while we're deleting the objects! V0.9.12 (2001-04-29) [umoeller]
            WPObject** papObjects = fdrQueryContentArray(pFolder,
                                                         0,     // no filter
                                                         &cObjects);
            brc = TRUE;

            for (ul = 0;
                 ul < cObjects;
                 ul++)
            {
                WPObject *pObject = papObjects[ul];
                if (!_wpFree(pObject))
                {
                    // error:
                    brc = FALSE;
                    // and stop
                    break;
                }
            }
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    return brc;
}

/* ******************************************************************
 *
 *   Awake-objects test
 *
 ********************************************************************/

static LINKLIST     G_llRootFolders;        // linked list of root folders,
                                            // holding plain WPFolder pointers
                                            // (no auto-free, of course)
static HMTX         G_hmtxRootFolders = NULLHANDLE;

// last folder cache
extern WPFolder     *G_pLastQueryAwakeFolder = NULL;
                        // this must be exported because if this goes
                        // dormant, XFolder::wpUnInitData must null this
static CHAR         G_szLastQueryAwakeFolderPath[CCHMAXPATH];

/*
 *@@ LockRootFolders:
 *      locks G_hmtxRootFolders. Creates the mutex on
 *      the first call.
 *
 *      Returns TRUE if the mutex was obtained.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

STATIC BOOL LockRootFolders(VOID)
{
    if (G_hmtxRootFolders)
        return !DosRequestMutexSem(G_hmtxRootFolders, SEM_INDEFINITE_WAIT);

    // first call:
    if (!DosCreateMutexSem(NULL,
                           &G_hmtxRootFolders,
                           0,
                           TRUE))      // request!
    {
        lstInit(&G_llRootFolders,
                FALSE);     // no auto-free
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ UnlockRootFolders:
 *      the reverse to LockRootFolders.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

STATIC VOID UnlockRootFolders(VOID)
{
    DosReleaseMutexSem(G_hmtxRootFolders);
}

/*
 *@@ fdrRegisterAwakeRootFolder:
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

BOOL fdrRegisterAwakeRootFolder(WPFolder *somSelf)
{
    BOOL brc = FALSE,
         fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockRootFolders())
        {
            CHAR sz[CCHMAXPATH] = "???";
            _wpQueryFilename(somSelf, sz, TRUE);
                // this gives us "M:", not "M:\" !

            lstAppendItem(&G_llRootFolders,
                          somSelf);

            _xwpModifyFlags(somSelf,
                            OBJLIST_QUERYAWAKEFSOBJECT,
                            OBJLIST_QUERYAWAKEFSOBJECT);

            brc = TRUE;
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    if (fLocked)
        UnlockRootFolders();

    return brc;
}

/*
 *@@ fdrRemoveAwakeRootFolder:
 *      called from XFldObject::wpUnInitData when the
 *      root folder goes dormant again. Note that
 *      this also gets called for any folder that
 *      was in the "awake folder" cache.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

BOOL fdrRemoveAwakeRootFolder(WPFolder *somSelf)
{
    BOOL brc = FALSE,
         fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockRootFolders())
        {
            lstRemoveItem(&G_llRootFolders,
                          somSelf);
                    // can fail if this wasn't really a root folder
                    // but only touched by the cache

            // invalidate cache
            G_pLastQueryAwakeFolder = NULL;

            brc = TRUE;
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    if (fLocked)
        UnlockRootFolders();

    return brc;
}

/*
 *@@ ProcessParticles:
 *
 *      Preconditions:
 *
 *      --  pStartOfParticle must point to the
 *          first particle name. For example,
 *          with C:\FOLDER\FILE.TXT, this must
 *          point to the FOLDER particle.
 *
 *      --  The path must be in upper case.
 *
 *      --  Caller must hold root folder mutex.
 *
 *      --  The path must not end in '\\'.
 *
 *      Postconditions:
 *
 *      --  This trashes the string buffer.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

STATIC WPFileSystem* ProcessParticles(WPFolder *pCurrentFolder,
                                      PSZ pStartOfParticle)
{
    // 0123456789012
    // C:\FOLDER\XXX
    //    ^ start of particle

    BOOL fQuit = FALSE;

    while (    (pStartOfParticle)
            && (*pStartOfParticle)
            && (!fQuit)
          )
    {
        PSZ pEndOfParticle;
        ULONG ulLengthThis;
        if (pEndOfParticle = strchr(pStartOfParticle, '\\'))
        {
            // 0123456789012
            // C:\FOLDER\XXX
            //    |     ^ end of particle
            //    ^ start of particle
            ulLengthThis =   pEndOfParticle
                           - pStartOfParticle;
            // overwrite backslash
            *pEndOfParticle = '\0';
        }
        else
        {
            // no more backslashes:
            ulLengthThis = strlen(pStartOfParticle);
            fQuit = TRUE;
        }

        if (ulLengthThis)
        {
            // ask the folder if this file name is awake
            WPFileSystem *pFound;
            // _Pmpf(("Scanning particle %s", pStartOfParticle));
            if (pFound = SafeFindFSFromUpperName(pCurrentFolder,
                                                 pStartOfParticle))
            {
                // found something:
                if (fQuit)
                {
                    // we're at the last node already:
                    // update the "last folder" cache
                    if (pCurrentFolder != G_pLastQueryAwakeFolder)
                    {
                        G_pLastQueryAwakeFolder = pCurrentFolder;
                        _wpQueryFilename(pCurrentFolder,
                                         G_szLastQueryAwakeFolderPath,
                                         TRUE);
                        nlsUpper(G_szLastQueryAwakeFolderPath);

                        // set the flag in the instance data
                        // so the cache ptr is invalidated
                        // once this thing goes dormant
                        _xwpModifyFlags(pCurrentFolder,
                                        OBJLIST_QUERYAWAKEFSOBJECT,
                                        OBJLIST_QUERYAWAKEFSOBJECT);
                    }
                    // return this
                    return pFound;
                }
                else
                {
                    pCurrentFolder = pFound;
                    // search on after that backslash
                    // C:\FOLDER\XXX
                    //          ^ end of particle
                    pStartOfParticle = pEndOfParticle + 1;
                    // C:\FOLDER\XXX
                    //           ^ new start of particle
                }
            }
            else
                // not awake:
                fQuit = TRUE;
        }
        else
            fQuit = TRUE;
    } // end while (    (pStartOfParticle)
           //         && (!fQuit)

    return NULL;
}

/*
 *@@ fdrQueryAwakeFSObject:
 *      returns the WPFileSystem for the specified
 *      full path if it is already awake, or NULL
 *      if it is not.
 *
 *      As opposed to M_WPFileSystem::wpclsQueryAwakeObject,
 *      this uses the fast XFolder content trees.
 *
 *      This compares without respect to case, but cannot
 *      handle UNC names. In other words, pcszFQPath must
 *      be something like C:\folder\file.txt.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

WPFileSystem* fdrQueryAwakeFSObject(PCSZ pcszFQPath)
{
    WPFileSystem *pReturn = NULL;

    BOOL    fLocked = FALSE;
    PSZ     pszUpperFQPath;

    TRY_LOUD(excpt1)
    {
        ULONG       ulFQPathLen;
        if (    (pcszFQPath)
             && (ulFQPathLen = strlen(pcszFQPath))
             && (ulFQPathLen > 1)
             && (pszUpperFQPath = malloc(ulFQPathLen + 1))
           )
        {
            memcpy(pszUpperFQPath, pcszFQPath, ulFQPathLen + 1);
            nlsUpper(pszUpperFQPath);

            // if the string terminates with '\',
            // remove that (the below code cannot handle that)
            if (pszUpperFQPath[ulFQPathLen - 1] == '\\')
                pszUpperFQPath[--ulFQPathLen] = '\0';

            // check if this is a local object; we can't support
            // UNC at this point
            if (pcszFQPath[1] == ':')
            {
                // this is a fully qualified path like C:\folder\file.txt:

                // now get the root folder from the drive letter
                if (fLocked = LockRootFolders())
                        // @@todo this isn't enough, the WPS might be
                        // awaking a root folder behind our back!!!
                {
                    PLISTNODE pNode;
                    WPFolder *pRoot = NULL;

                    // we are not in the root folder:
                    // now, since this routine probably gets
                    // called frequently for files in the same
                    // folder, we should cache the last folder
                    // found...
                    PSZ     pLastBackslash;
                    BOOL    fSkip = FALSE;

                    if (    // cache only non-root folders
                            (ulFQPathLen > 4)
                            // not first call?
                         && G_pLastQueryAwakeFolder
                         && (pLastBackslash = strrchr(pszUpperFQPath, '\\'))
                       )
                    {
                        *pLastBackslash = '\0';
                        if (!strcmp(G_szLastQueryAwakeFolderPath,
                                    pszUpperFQPath))
                        {
                            // same folder as last time:
                            // we don't need to run thru the
                            // particles then
                            // _Pmpf(("Scanning cache folder %s",
                               //     G_szLastQueryAwakeFolderPath));

                            pReturn = SafeFindFSFromUpperName(G_pLastQueryAwakeFolder,
                                                              pLastBackslash + 1);
                            // even if this returned NULL,
                            // skip the query
                            fSkip = TRUE;
                        }

                        // restore last backslash
                        *pLastBackslash = '\\';
                    }

                    if (!fSkip)
                    {
                        // cache wasn't used:
                        for (pNode = lstQueryFirstNode(&G_llRootFolders);
                             pNode;
                             pNode = pNode->pNext)
                        {
                            CHAR szRootThis[CCHMAXPATH];
                            WPFolder *pThis;
                            if (    (pThis = (WPFolder*)pNode->pItemData)
                                 && (_wpQueryFilename(pThis,
                                                      szRootThis,
                                                      TRUE))
                               )
                            {
                                // _Pmpf(("Scanning root folder %s", szRootThis));
                                if (szRootThis[0] == pszUpperFQPath[0])
                                {
                                    pRoot = pThis;
                                    break;
                                }
                            }
                        }

                        if (pRoot)
                        {
                            // we got the root folder:
                            if (ulFQPathLen == 2)
                            {
                                // caller wants root dir only:
                                // we can stop here
                                pReturn = pRoot;
                            }
                            else if (pcszFQPath[2] == '\\')
                            {
                                // go thru the path particles and,
                                // for each particle, query the parent
                                // folder for whether the component
                                // exists (this updates the cache)
                                pReturn = ProcessParticles(pRoot,
                                                           // first particle:
                                                           pszUpperFQPath + 3);
                            }
                        }
                        // else: we didn't even have the root folder,
                        // no need to search on... the others can't
                        // be awake either
                    }
                }
            }

            free(pszUpperFQPath);
        }
    }
    CATCH(excpt1)
    {
        pReturn = NULL;
    } END_CATCH();

    if (fLocked)
        UnlockRootFolders();

    return pReturn;
}

/* ******************************************************************
 *
 *   Folder populate with file-system objects
 *
 ********************************************************************/

#ifdef __DEBUG__

/*
 *@@ fdrDebugDumpFolderFlags:
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

VOID fdrDebugDumpFolderFlags(WPFolder *somSelf)
{
    /* ULONG fl = _wpQueryFldrFlags(somSelf);
    _PmpfF(("flags for folder %s", _wpQueryTitle(somSelf)));
    if (fl & FOI_POPULATEINPROGRESS)
        _Pmpf(("    FOI_POPULATEINPROGRESS"));
    if (fl & FOI_REFRESHINPROGRESS)
        _Pmpf(("    FOI_REFRESHINPROGRESS"));
    if (fl & FOI_ASYNCREFRESHONOPEN)
        _Pmpf(("    FOI_ASYNCREFRESHONOPEN"));
    if (fl & FOI_POPULATEDWITHFOLDERS)
        _Pmpf(("    FOI_POPULATEDWITHFOLDERS"));
    if (fl & FOI_POPULATEDWITHALL)
        _Pmpf(("    FOI_POPULATEDWITHALL"));
    */
}

#endif

#ifndef __NOTURBOFOLDERS__

/* ******************************************************************
 *
 *   Folder populate with abstract objects
 *
 ********************************************************************/

/*
 *@@ PopulateWithAbstracts:
 *      called from fdrPopulate to get the abstract
 *      objects.
 *
 *      This still needs to be rewritten. For now we
 *      use the slow wpclsFind* methods.
 *
 *      Preconditions:
 *
 *      --  Caller must hold the find mutex.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

STATIC BOOL PopulateWithAbstracts(WPFolder *somSelf,
                                  HWND hwndReserved,
                                  PMINIRECORDCORE pMyRecord,
                                  PBOOL pfExit)          // in: exit flag
{
    BOOL        fSuccess = FALSE;
    HFIND       hFind = 0;

    TRY_LOUD(excpt1)
    {
        CLASS       Classes2Find[2];
        WPObject    *aObjectsFound[500];
        ULONG       cObjects;
        BOOL        brcFind;
        ULONG       ulrc;

        Classes2Find[0] = _WPAbstract;
        Classes2Find[1] = NULL;

        _wpclsSetError(_WPObject,0);
        cObjects = ARRAYITEMCOUNT(aObjectsFound);
        brcFind = _wpclsFindObjectFirst(_WPObject,
                                        Classes2Find,    // _WPAbstract
                                        &hFind,
                                        NULL,            // all titles
                                        somSelf,         // folder
                                        FALSE,           // no subfolders
                                        NULL,            // no extended criteria
                                        aObjectsFound,   // out: objects
                                        &cObjects);      // in: size of buffer, out: objs found

        do
        {
            ulrc = _wpclsQueryError(_WPObject);

            if (ulrc == WPERR_OBJECT_NOT_FOUND)
            {
                // nothing found is not an error
                fSuccess = TRUE;
                break;
            }

            if (    (brcFind)      // no error: we're done
                 || (ulrc == WPERR_BUFFER_OVERFLOW) // more objects to go
               )
            {
                // process objects here
                if (hwndReserved)
                    WinPostMsg(hwndReserved,
                               0x0405,
                               (MPARAM)-1,
                               (MPARAM)pMyRecord);

                if (brcFind)
                {
                    // we're done:
                    fSuccess = TRUE;
                    break;
                }
                else if (ulrc == WPERR_BUFFER_OVERFLOW)
                {
                    // go for next chunk
                    cObjects = ARRAYITEMCOUNT(aObjectsFound);
                    brcFind = _wpclsFindObjectNext(_WPObject,
                                                   hFind,
                                                   aObjectsFound,
                                                   &cObjects);
                }
            }
            else
            {
                // bad error:
                // get out of here
                break;
            }

        } while (!*pfExit);
    }
    CATCH(excpt1)
    {
        fSuccess = FALSE;
    } END_CATCH();

    // clean up
    if (hFind)
        _wpclsFindObjectEnd(_WPObject, hFind);

    return fSuccess;
}

/* ******************************************************************
 *
 *   Folder populate (main)
 *
 ********************************************************************/

/*
 *@@ fdrPopulate:
 *      reimplementation for XFolder::wpPopulate if
 *      "turbo folders" are enabled. This is a
 *      100% replacement of WPFolder::wpPopulate.
 *
 *      This presently does _not_ get called
 *
 *      --  for desktops (don't want to mess with this
 *          yet);
 *
 *      --  for remote folders cos we can't cache UNC
 *          folders yet.
 *
 *      This is a lot faster than WPFolder::wpPopulate
 *      for file-system objects. For this,
 *      fsysPopulateWithFSObjects() gets called; see
 *      remarks there for details.
 *
 *      In addition, this supports an exit flag which, when
 *      set to TRUE, will cancel populate. This
 *      is useful to do populate on a transient thread
 *      while still being able to cancel populate,
 *      which is simply impossible with the standard wpPopulate.
 *
 *      If you call this with the address of a
 *      THREADINFO.fExit, you can even use the
 *      thread functions to cancel (see thrClose).
 *
 *      <B>Populate and Refresh</B>
 *
 *      wpPopulate gets called by wpRefresh also.
 *      In addition, populate can get called when the
 *      folder was already previously populated. Besides,
 *      certain individual objects may already be awake
 *      due to a previous WPS call such as
 *      WPFileSystem::wpclsQueryObjectFromPath.
 *
 *      As a result, we must check for each object
 *      whether it is already awake before awaking it.
 *
 *      The way this works is roughly the following
 *      (see the Warp 4 WPSREF for details):
 *
 *      1)  For each awake object, wpRefresh first turns
 *          on the DIRTYBIT and turns off the FOUNDBIT.
 *
 *      2)  wpPopulate runs DosFindFirst/Next on the
 *          directory contents (for file-system objects)
 *          and gets abstract data from OS2.INI. For
 *          each object, it checks if it is already
 *          awake.
 *
 *          If it is awake, check if its data changed
 *          and call wpRefresh on the object if
 *          necessary. In any case, turn on the FOUNDBIT
 *          and turn off the DIRTYBIT, since the object
 *          is fresh now.
 *
 *          If it is not awake, call wpclsMakeAwake
 *          and turn on the FOUNDBIT.
 *
 *      3)  After wpPopulate returns, wpRefresh checks
 *          the bits again.
 *
 *          If FOUNDBIT is off, the object is deleted.
 *
 *          If DIRTYBIT is still on, the object is
 *          refreshed.
 *
 *      <B>Mutex Semaphores</B>
 *
 *      Semaphore protection is especially important
 *      in order to avoid waking up objects several
 *      times.
 *
 *      The whole wpPopulate code is protected by
 *      the folder "find" mutex to avoid a race between
 *      multiple calls to wpPopulate.
 *
 *      In the "refresh or awake" code, each scan of the
 *      folder contents must be protected by the
 *      folder mutex in turn.
 *
 *      If the object is to be made awake, wpclsMakeAwake
 *      will call wpAddToContent on the folder, which
 *      requests the folder "write" mutex.
 *
 *      <B>Object Locking</B>
 *
 *      For each object that was found, we check if the object
 *      is already awake.
 *
 *      --  If so, for file-system objects, we check if the
 *          object needs a refresh. Abstracts are never refreshed.
 *          In any case though, if the object is awake already,
 *          it is locked once.
 *
 *      --  If the object is not awake, it is made awake through
 *          wpclsMakeAwake. This locks the object too.
 *
 *      In other words, all objects are locked. This applies
 *      only to the objects which were actually touched by
 *      this function though; so if (fFoldersOnly == TRUE)
 *      or (pcszFilemask != NULL), not all objects will have
 *      been locked.
 *
 *      For standard folder views, the WPS appears to unlock
 *      every object when the view is closed again. However,
 *      for our own views or if we populate for some other
 *      reason, the caller is responsible for unlocking.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

BOOL fdrPopulate(WPFolder *somSelf,
                 PCSZ pcszFolderFullPath, // in: wpQueryFilename(somSelf, TRUE)
                 HWND hwndReserved,
                 BOOL fFoldersOnly,
                 PBOOL pfExit)          // in: exit flag
{
    BOOL    fSuccess = FALSE;
    BOOL    fFindLocked = FALSE;

    ULONG   ulOldPrtyClass = -1,
            ulOldPrtyDelta = -1;

    ULONG   flFolderNew = 0;            // new folder flags

    PMINIRECORDCORE pMyRecord = _wpQueryCoreRecord(somSelf);

    TRY_LOUD(excpt1)
    {
        PMPF_TURBOFOLDERS(("POPULATING %s", pcszFolderFullPath));

        // there can only be one populate at a time
        if (fFindLocked = !_wpRequestFindMutexSem(somSelf, SEM_INDEFINITE_WAIT))
        {
            PPIB ppib;
            PTIB ptib;

            // tell everyone that we're populating
            _wpModifyFldrFlags(somSelf,
                               FOI_POPULATEINPROGRESS,
                               FOI_POPULATEINPROGRESS);

            // the standard WPS populate thread normally runs with
            // a higher priority, so make sure this is regular...
            // otherwise the folder window won't open, and the
            // system is hogged
            DosGetInfoBlocks(&ptib, &ppib);
            ulOldPrtyClass = (((ptib->tib_ptib2->tib2_ulpri) >> 8) & 0x00FF);
            ulOldPrtyDelta = ((ptib->tib_ptib2->tib2_ulpri) & 0x001F);

            DosSetPriority(PRTYS_THREAD,
                           PRTYC_REGULAR,
                           0,
                           0);      // current thread

            fdrDebugDumpFolderFlags(somSelf);

            if (hwndReserved)
                WinPostMsg(hwndReserved,
                           0x0405,
                           (MPARAM)-1,
                           (MPARAM)pMyRecord);

            // in any case, populate FS objects; this
            // will use folders only if the flag is set
            if (fSuccess = fsysPopulateWithFSObjects(somSelf,
                                                     hwndReserved,
                                                     pMyRecord,
                                                     pcszFolderFullPath,
                                                     fFoldersOnly,
                                                     NULL,        // all objects
                                                     pfExit))
            {
                // set folder flags to be set on exit:
                // we got at least "folders only" at this point
                flFolderNew = FOI_POPULATEDWITHFOLDERS;

                if (!fFoldersOnly)
                {
                    if (hwndReserved)
                        WinPostMsg(hwndReserved,
                                   0x0405,
                                   (MPARAM)-1,
                                   (MPARAM)pMyRecord);

                    // if we have something other than folders,
                    // we need to populate with abstracts too
                    if (fSuccess = PopulateWithAbstracts(somSelf,
                                                         hwndReserved,
                                                         pMyRecord,
                                                         pfExit))
                        // got this too:
                        flFolderNew |= FOI_POPULATEDWITHALL;
                }
            }
        }
    }
    CATCH(excpt1)
    {
        fSuccess = FALSE;
    } END_CATCH();

    // in any case, even if we crashed,
    // set new folder flags:
    //      clear FOI_POPULATEINPROGRESS
    //      clear FOI_ASYNCREFRESHONOPEN
    //      set FOI_POPULATEDWITHFOLDERS, if no error
    //      set FOI_POPULATEDWITHALL, if no error
    _wpModifyFldrFlags(somSelf,
                       FOI_POPULATEINPROGRESS | FOI_ASYNCREFRESHONOPEN
                            | FOI_POPULATEDWITHFOLDERS | FOI_POPULATEDWITHALL,
                       flFolderNew);

    if (    (ulOldPrtyClass != -1)
         && (ulOldPrtyDelta != -1)
       )
        DosSetPriority(PRTYS_THREAD,
                       ulOldPrtyClass,
                       ulOldPrtyDelta,
                       0);      // current thread

    if (hwndReserved)
        WinPostMsg(hwndReserved,
                   0x0405,
                   (MPARAM)1,
                   (MPARAM)pMyRecord);

    fdrDebugDumpFolderFlags(somSelf);

    if (fFindLocked)
        _wpReleaseFindMutexSem(somSelf);

    PMPF_TURBOFOLDERS(("returning %d", fSuccess));

    return fSuccess;
}

#endif

/*
 *@@ fdrCheckIfPopulated:
 *      this populates a folder if it's not populated yet.
 *      Saves you from querying the full path and all that.
 *
 *      If (fFoldersOnly == FALSE), this populates the folder
 *      with subfolders only if this hasn't been done yet.
 *
 *      If (fFoldersOnly == TRUE), this fully populates the
 *      folder if this hasn't been done yet.
 *
 *      Returns:
 *
 *      --  FALSE if wpPopulate failed
 *
 *      --  1 (TRUE) if the folder was indeed populated
 *
 *      --  2 if the folder was already populated and we
 *          determined we did not need a refresh.
 *
 *@@changed V0.9.4 (2000-08-03) [umoeller]: changed return code
 *@@changed V0.9.6 (2000-10-25) [umoeller]: added fFoldersOnly
 *@@changed V0.9.11 (2001-04-21) [umoeller]: disabled shadow populate for folders only
 *@@changed V0.9.16 (2001-10-25) [umoeller]: added quiet excpt handler around wpPopulate
 *@@changed V0.9.16 (2002-01-01) [umoeller]: added checks if a refresh is in order
 *@@changed V0.9.16 (2002-01-01) [umoeller]: renamed from wpshCheckIfPopulated, moved here from wpsh.c
 *@@changed V1.0.0 (2002-09-13) [umoeller]: returning 2 now if we didn't really populate
 *@@changed V1.0.4 (2005-10-09) [pr]: Cope with IBMDRIVEDATA being variable size
 */

ULONG fdrCheckIfPopulated(WPFolder *somSelf,
                          BOOL fFoldersOnly)
{
    ULONG       ulrc = FALSE;       // failed (0)
    ULONG       ulPopulateFlag = (fFoldersOnly)
                                    ? FOI_POPULATEDWITHFOLDERS
                                    : FOI_POPULATEDWITHALL;
    ULONG       ulFlags = _wpQueryFldrFlags(somSelf);
    PIBMDRIVEDATA pDriveData = _wpQueryDriveData(somSelf);
    BOOL        fNotLocal = FALSE;
    BOOL        fFixedDisk = TRUE;

    if (pDriveData)
    {
        if (G_ulDriveDataType == 1)
        {
            fNotLocal = pDriveData->ibmDD1.fNotLocal;
            fFixedDisk = pDriveData->ibmDD1.fFixedDisk;
        }
        else if (G_ulDriveDataType == 2)
        {
            fNotLocal = pDriveData->ibmDD2.fNotLocal;
            fFixedDisk = pDriveData->ibmDD2.fFixedDisk;
        }
    }

    if (    // (re)populate if the POPULATED_* flag is not set
            ((ulFlags & ulPopulateFlag) != ulPopulateFlag)
// V0.9.16 (2002-01-01) [umoeller]: added all the below checks
            // or if the folder has the refresh bit set
         || (ulFlags & FOI_ASYNCREFRESHONOPEN)
            // or if we can't get the drive data
         || (fNotLocal)
         || (!(fFixedDisk))
       )
    {
        // alright, needs populate:
        // turn off the "populated" bit to make sure we really
        // populate (this will refresh objects that are already awake)
        _wpModifyFldrFlags(somSelf,
                           FOI_POPULATEDWITHFOLDERS | FOI_POPULATEDWITHALL,
                           0);

        // put this in a quiet exception handler because
        // this tends to crash with the WPNetwork folder... grrrr...
        TRY_QUIET(excpt1)
        {
            CHAR    szRealName[2*CCHMAXPATH];
            if (_wpQueryFilename(somSelf, szRealName, TRUE))
                ulrc = _wpPopulate(somSelf,
                                   0,
                                   szRealName,
                                   fFoldersOnly);
        }
        CATCH(excpt1)
        {
            ulrc = FALSE;
        } END_CATCH();
    }
    else
        // already populated:
        ulrc = 2;       // V1.0.0 (2002-09-13) [umoeller]

    return ulrc;
}

/* ******************************************************************
 *
 *   Object insertion
 *
 ********************************************************************/

/*
 *@@ fdrCnrInsertObject:
 *      inserts an object into all currently open views,
 *      wherever this may be.
 *
 *      As a precondition, the object must already
 *      _reside_ in a folder. It is assumed that the
 *      object only hasn't been inserted yet.
 *
 *      This inserts the object into:
 *
 *      -- icon views;
 *
 *      -- details views;
 *
 *      -- tree views.
 *
 *@@added V0.9.7 (2001-01-13) [umoeller]
 */

BOOL fdrCnrInsertObject(WPObject *pObject)
{
    BOOL        brc = FALSE;
    WPObject    *pFolder;

    if (    (pObject)
         && (pFolder = _wpQueryFolder(pObject))
       )
    {
        WPObject *pobjLock = NULL;
        TRY_LOUD(excpt1)
        {
            // if pFolder is a root folder, we should really
            // insert the object below the corresponding disk object
            if (ctsIsRootFolder(pFolder))
                pFolder = _wpQueryDisk(pFolder);

            if (    (pFolder)
                 && (pobjLock = (!_wpRequestObjectMutexSem(pFolder, SEM_INDEFINITE_WAIT)) ? pFolder : NULL)
               )
            {
                PVIEWITEM   pViewItem;
                for (pViewItem = _wpFindViewItem(pFolder, VIEW_ANY, NULL);
                     pViewItem;
                     pViewItem = _wpFindViewItem(pFolder, VIEW_ANY, pViewItem))
                {
                    switch (pViewItem->view)
                    {
                        case OPEN_CONTENTS:
                        // case OPEN_TREE:
                        case OPEN_DETAILS:
                        {
                            HWND hwndCnr;
                            if (hwndCnr = WinWindowFromID(pViewItem->handle, FID_CLIENT))
                            {
                                PPOINTL pptlIcon = _wpQueryNextIconPos(pFolder);
                                if (_wpCnrInsertObject(pObject,
                                                       hwndCnr,
                                                       pptlIcon,
                                                       NULL,     // parent record
                                                       NULL))     // RECORDINSERT, next pos.
                                    brc = TRUE;
                            }
                        }
                    }
                }
            }
        }
        CATCH(excpt1) {} END_CATCH();

        if (pobjLock)
            _wpReleaseObjectMutexSem(pobjLock);
    }

    return brc;
}

/*
 *@@ fdrInsertAllContents:
 *      inserts the contents of pFolder into all currently
 *      open views of pFolder.
 *
 *      Preconditions:
 *
 *      --  The folder is assumed to be fully populated.
 *
 *      --  The caller must hold the folder mutex since we're
 *          going over the folder contents here.
 *
 *@@added V0.9.9 (2001-04-01) [umoeller]
 */

ULONG fdrInsertAllContents(WPFolder *pFolder)
{
     ULONG       cObjects = 0;
     WPObject    **papObjects;

     if (papObjects = fdrQueryContentArray(pFolder,
                                           QCAFL_FILTERINSERTED,
                                           &cObjects))
     {
         PVIEWITEM   pViewItem;
         for (pViewItem = _wpFindViewItem(pFolder, VIEW_ANY, NULL);
              pViewItem;
              pViewItem = _wpFindViewItem(pFolder, VIEW_ANY, pViewItem))
         {
             switch (pViewItem->view)
             {
                 case OPEN_CONTENTS:
                 case OPEN_TREE:
                 case OPEN_DETAILS:
                 {
                     HWND hwndCnr = WinWindowFromID(pViewItem->handle, FID_CLIENT);
                     POINTL ptlIcon = {0, 0};
                     if (hwndCnr)
                         _wpclsInsertMultipleObjects(_somGetClass(pFolder),
                                                     hwndCnr,
                                                     &ptlIcon,
                                                     (PVOID*)papObjects,
                                                     NULL,   // parentrecord
                                                     cObjects);
                 }
             }
         }

         free(papObjects);
     }

    return cObjects;
}


