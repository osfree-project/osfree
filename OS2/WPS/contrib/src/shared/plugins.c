
/*
 *@@sourcefile plugins.c:
 *      Plugins manager, i.e. settings management that is
 *      supposed to be independent of the currently open
 *      XCenter views.
 *
 *      This file is new with V0.9.20 and contains code
 *      formerly in ctr_model.c.
 *
 *      Function prefix for this file:
 *      --  plg*
 *
 *@@header "shared\plugins.h"
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
#define INCL_DOSMODULEMGR
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINMESSAGEMGR
#define INCL_WINCOUNTRY
#define INCL_WINMENUS

#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xcenter.ih"

// XWorkplace implementation headers
#include "bldlevel.h"                   // XWorkplace build level definitions

#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "shared\plugins.h"             // public plugins interfaces

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

typedef struct _PRIVATEPLUGINCLASS
{
    PLUGINCLASS    Public;

    ULONG          ulVersionMajor,
                   ulVersionMinor,
                   ulVersionRevision;

    HMODULE        hmod;

    ULONG          ulUser3;

} PRIVATEPLUGINCLASS, *PPRIVATEPLUGINCLASS;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

/* ******************************************************************
 *
 *   Plugins category management
 *
 ********************************************************************/

/*
 *@@ plgRegisterCategory:
 *      register a plugin category.
 *
 *@@added V0.9.20 (2002-07-22) [lafaix]
 */

BOOL plgRegisterCategory(PPLUGINCATEGORY pplgCategory,
                         PCSZ pcszCategoryName,
                         PCSZ pcszLocalizedName,
                         ULONG cbCategoryData,
                         ULONG ulPredefined,
                         PVOID pPredefined)
{
    BOOL brc = FALSE;
// _Pmpf(("plgRegisterCategory called for category %s", pcszCategoryName));
    if (pplgCategory && pcszCategoryName)
    {
        pplgCategory->pcszName = pcszCategoryName;
        pplgCategory->pcszLocalizedName = pcszLocalizedName;
        pplgCategory->hmtx = NULLHANDLE;
        pplgCategory->fClassesLoaded = FALSE;
        pplgCategory->ulClassesRefCount = 0;
        pplgCategory->fModulesInitialized = 0;
        pplgCategory->ulExtra = cbCategoryData;
        pplgCategory->ulPredefinedClasses = ulPredefined;
        pplgCategory->pPredefinedClasses = pPredefined;
        brc = TRUE;
    }

    return brc;
}

/*
 *@@ plgQueryCategories:
 *
 *@@added V0.9.20 (2002-07-22) [lafaix]
 */

PLINKLIST plgQueryCategories(VOID)
{
    return NULL;
}

/* ******************************************************************
 *
 *   Plugins class management
 *
 ********************************************************************/

/*
 *@@ plgLockClasses:
 *      locks the category classes list.
 *
 *@@added V0.9.12 (2001-05-20) [umoeller]
 */

BOOL plgLockClasses(PPLUGINCATEGORY pplgCategory)
{
    if (!pplgCategory->hmtx)
        return !DosCreateMutexSem(NULL,
                                  &pplgCategory->hmtx,
                                  0,
                                  TRUE);       // request now

    return !DosRequestMutexSem(pplgCategory->hmtx, SEM_INDEFINITE_WAIT);
}

/*
 *@@ plgUnlockClasses:
 *
 *@@added V0.9.12 (2001-05-20) [umoeller]
 */

VOID plgUnlockClasses(PPLUGINCATEGORY pplgCategory)
{
    DosReleaseMutexSem(pplgCategory->hmtx);
}

/*
 *@@ FreeModule:
 *      wrapper around DosFreeModule which
 *      attempts to call the "UnInitModule"
 *      export from the plugin DLL beforehand.
 *
 *      May run on any thread. Caller must hold
 *      classes mutex.
 *
 *@@added V0.9.7 (2000-12-07) [umoeller]
 *@@changed V0.9.9 (2001-02-06) [umoeller]: added fCallUnInit
 */

STATIC APIRET FreeModule(HMODULE hmod,
                         BOOL fCallUnInit)     // in: if TRUE, "uninit" export gets called
{
    if (fCallUnInit)
    {
        // the following might crash
        TRY_QUIET(excpt2)
        {
            PFNPLGUNINITMODULE pfnPlgUnInitModule = NULL;
            APIRET arc2 = DosQueryProcAddr(hmod,
                                           2,      // ordinal
                                           NULL,
                                           (PFN*)(&pfnPlgUnInitModule));
            if ((arc2 == NO_ERROR) && (pfnPlgUnInitModule))
            {
                pfnPlgUnInitModule();
            }
        }
        CATCH(excpt2) {} END_CATCH();
    }

    return DosFreeModule(hmod);
}

/*
 *@@ plgLoadClasses:
 *      initializes the global array of widget classes.
 *
 *      This also goes thru the plugin category subdirectory
 *      of the XWorkplace installation directory and tests
 *      the DLLs in there for widget plugins.
 *
 *      Note: For each plgLoadClasses call, there must be
 *      a matching plgFreeClasses call, or the plugin DLLs
 *      will never be unloaded. This function maintains a
 *      reference count to the global data so calls to this
 *      function may be nested.
 *
 *      May run on any thread.
 *
 *@@changed V0.9.9 (2001-02-06) [umoeller]: added version management
 *@@changed V0.9.9 (2001-03-09) [umoeller]: added PRIVATEWIDGETCLASS wrapping
 *@@changed V0.9.9 (2001-03-09) [umoeller]: converted global array to linked list
 *@@changed V0.9.12 (2001-05-20) [umoeller]: added mutex protection to fix multiple loads
 */

VOID plgLoadClasses(PPLUGINCATEGORY pplgCategory)
{
    BOOL fLocked = FALSE;
    TRY_LOUD(excpt1)
    {
        if (fLocked = plgLockClasses(pplgCategory)) // V0.9.12 (2001-05-20) [umoeller]
        {
            if (!pplgCategory->fModulesInitialized)
            {
                // very first call:
                lstInit(&pplgCategory->llClasses, FALSE);
                lstInit(&pplgCategory->llModules, FALSE);
                pplgCategory->fModulesInitialized = TRUE;
            }

            if (!pplgCategory->fClassesLoaded)
            {
                // category classes not loaded yet (or have been released again):

                HAB             hab = WinQueryAnchorBlock(cmnQueryActiveDesktopHWND());

                HMODULE         hmodXFLDR = cmnQueryMainCodeModuleHandle();

                // built-in widget classes:
                APIRET          arc = NO_ERROR;
                CHAR            szPluginsDir[2*CCHMAXPATH],
                                szSearchMask[2*CCHMAXPATH];
                HDIR            hdirFindHandle = HDIR_CREATE;
                FILEFINDBUF3    ffb3 = {0};      // returned from FindFirst/Next
                ULONG           cbFFB3 = sizeof(FILEFINDBUF3);
                ULONG           ulFindCount = 1;  // look for 1 file at a time
                ULONG           ul;

                // step 1: append built-in classes to list
                for (ul = 0;
                     ul < pplgCategory->ulPredefinedClasses;
                     ul++)
                {
                    PPRIVATEPLUGINCLASS pClass = (PPRIVATEPLUGINCLASS)malloc(pplgCategory->ulExtra
                                                                           + sizeof(PRIVATEPLUGINCLASS));
                    memset(pClass,
                           0,
                           sizeof(PRIVATEPLUGINCLASS) + pplgCategory->ulExtra);
                    memcpy(&pClass->Public,
                           &pplgCategory->pPredefinedClasses[ul],
                           sizeof(PLUGINCLASS));

                    // adjust NLS title if it's not a string already
                    if (((ULONG)pClass->Public.pcszClassTitle) & PLUGIN_STRING_RESOURCE)
                    {
                        pClass->Public.pcszClassTitle = cmnGetString((ULONG)pClass->Public.pcszClassTitle & ~PLUGIN_STRING_RESOURCE);
                    }

                    lstAppendItem(&pplgCategory->llClasses,
                                  pClass);
                }

                // step 2: append plugin DLLs to list
                // compose path for widget plugin DLLs
                cmnQueryXWPBasePath(szPluginsDir);
                strcat(szPluginsDir, "\\plugins\\");
                strcat(szPluginsDir, pplgCategory->pcszName);
                sprintf(szSearchMask, "%s\\%s", szPluginsDir, "*.dll");

                // _PmpfF(("searching for '%s'", szSearchMask));

                arc = DosFindFirst(szSearchMask,
                                   &hdirFindHandle,
                                   // find everything except directories
                                   FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                                   &ffb3,
                                   cbFFB3,
                                   &ulFindCount,
                                   FIL_STANDARD);
                // and start looping...
                while (arc == NO_ERROR)
                {
                    // alright... we got the file's name in ffb3.achName
                    CHAR            szDLL[2*CCHMAXPATH],
                                    szError[CCHMAXPATH] = "";
                    HMODULE         hmod = NULLHANDLE;
                    APIRET          arc2 = NO_ERROR;

                    sprintf(szDLL, "%s\\%s", szPluginsDir, ffb3.achName);

                    // go load this!
                    if (arc2 = DosLoadModule(szError,
                                             sizeof(szError),
                                             szDLL,
                                             &hmod))
                    {
                        // error loading module:
                        // log this, but we'd rather not have a message box here
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "Unable to load plugin DLL \"%s\"."
                               "\n    DosLoadModule returned code %d and string: \"%s\"",
                               szDLL,
                               arc2,
                               szError);

                    }
                    else
                    {
                        CHAR    szErrorMsg[500] = "nothing.";
                                // room for error msg by DLL

                        // OK, since we've changed the prototype for the init module,
                        // it's time to do version management.
                        // V0.9.9 (2001-02-06) [umoeller]

                        // Check if the widget has the "query version" export.
                        PFNPLGQUERYVERSION pfnPlgQueryVersion = NULL;
                        // standard version if this fails: 0.9.8
                        ULONG       ulMajor = 0,
                                    ulMinor = 9,
                                    ulRevision = 8;
                        arc2 = DosQueryProcAddr(hmod,
                                                3,      // ordinal
                                                NULL,
                                                (PFN*)(&pfnPlgQueryVersion));

                        // (protect this with an exception handler, because
                        // this might crash)
                        TRY_QUIET(excpt2)
                        {
                            BOOL    fSufficientVersion = TRUE;

                            if ((arc2 == NO_ERROR) && (pfnPlgQueryVersion))
                            {
                                ULONG   ulPluginsMajor,
                                        ulPluginsMinor,
                                        ulPluginsRevision;
                                // we got the export:
                                pfnPlgQueryVersion(&ulMajor,
                                                   &ulMinor,
                                                   &ulRevision);

                                // check if this widget can live with this
                                // XCenter build level
                                sscanf(BLDLEVEL_VERSION,
                                       "%d.%d.%d",
                                       &ulPluginsMajor,
                                       &ulPluginsMinor,
                                       &ulPluginsRevision);

                                if (    (ulMajor > ulPluginsMajor)
                                     || (    (ulMajor == ulPluginsMajor)
                                          && (    (ulMinor > ulPluginsMinor)
                                               || (    (ulMinor == ulPluginsMinor)
                                                    && (ulRevision > ulPluginsRevision)
                                                  )
                                             )
                                        )
                                   )
                                    fSufficientVersion = FALSE;
                            }

                            if (fSufficientVersion)
                            {
                                PPLUGINCLASS paClasses = NULL;
                                ULONG   cClassesThis = 0;

                                // now check which INIT we can call
                                if (    (ulMajor > 0)
                                     || (ulMinor > 9)
                                     || (ulRevision > 8)
                                   )
                                {
                                    // new widget:
                                    // we can then afford the new prototype
                                    PFNPLGINITMODULE_099 pfnPlgInitModule = NULL;
                                    arc2 = DosQueryProcAddr(hmod,
                                                            1,      // ordinal
                                                            NULL,
                                                            (PFN*)(&pfnPlgInitModule));
                                    if ((arc2 == NO_ERROR) && (pfnPlgInitModule))
                                        cClassesThis = pfnPlgInitModule(hab,
                                                                        hmod,       // new!
                                                                        hmodXFLDR,
                                                                        &paClasses,
                                                                        szErrorMsg);
                                }
                                else
                                {
                                    // use the old prototype:
                                    PFNPLGINITMODULE_OLD pfnPlgInitModule = NULL;
                                    arc2 = DosQueryProcAddr(hmod,
                                                            1,      // ordinal
                                                            NULL,
                                                            (PFN*)(&pfnPlgInitModule));
                                    if ((arc2 == NO_ERROR) && (pfnPlgInitModule))
                                        cClassesThis = pfnPlgInitModule(hab,
                                                                        hmodXFLDR,
                                                                        &paClasses,
                                                                        szErrorMsg);
                                }

                                if (cClassesThis)
                                {
                                    // paClasses must now point to an array of
                                    // cClassesThis XCENTERWIDGETCLASS structures;
                                    // copy these
                                    for (ul = 0;
                                         ul < cClassesThis;
                                         ul++)
                                    {
                                        PPRIVATEPLUGINCLASS pClass = (PPRIVATEPLUGINCLASS)malloc(sizeof(*pClass));
                                        memset(pClass,
                                               0,
                                               sizeof(*pClass));
                                        memcpy(&pClass->Public,
                                               &paClasses[ul],
                                               sizeof(PLUGINCLASS));

                                        // load NLS string if this is a string ID
                                        // V0.9.19 (2002-05-07) [umoeller]
                                        if ((ULONG)pClass->Public.pcszClassTitle & PLUGIN_STRING_RESOURCE)
                                            pClass->Public.pcszClassTitle = cmnGetString(
                                                      ((ULONG)pClass->Public.pcszClassTitle)
                                                    & ~PLUGIN_STRING_RESOURCE);

                                        // store version
                                        pClass->ulVersionMajor = ulMajor;
                                        pClass->ulVersionMinor = ulMinor;
                                        pClass->ulVersionRevision = ulRevision;

                                        // store module
                                        pClass->hmod = hmod;
                                        lstAppendItem(&pplgCategory->llClasses,
                                                      pClass);
                                    }

                                    // append this module to the global list of
                                    // loaded modules
                                    lstAppendItem(&pplgCategory->llModules,
                                                  (PVOID)hmod);
                                } // end if (cClassesThis)
                                else
                                    // no classes in module or error:
                                    arc2 = ERROR_INVALID_DATA;
                            }
                        }
                        CATCH(excpt2)
                        {
                            arc2 = ERROR_INVALID_ORDINAL;
                        } END_CATCH();

                        if (arc2)
                        {
                            // error occurred (or crash):
                            // unload the module again
                            FreeModule(hmod,
                                       FALSE);      // do not call uninit

                            if (arc2 == ERROR_INVALID_DATA)
                                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                                       "InitModule call (export @1) failed for plugin DLL"
                                       "\n        \"%s\"."
                                       "\n    DLL returned error msg:"
                                       "\n        %s",
                                       szDLL,
                                       szErrorMsg);
                        }
                    } // end if DosLoadModule

                    // find next DLL
                    ulFindCount = 1;
                    arc = DosFindNext(hdirFindHandle,
                                      &ffb3,
                                      cbFFB3,
                                      &ulFindCount);
                } // while (arc == NO_ERROR)

                DosFindClose(hdirFindHandle);

                pplgCategory->fClassesLoaded = TRUE;
            }

            pplgCategory->ulClassesRefCount++;
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        plgUnlockClasses(pplgCategory);
}

/*
 *@@ plgQueryClasses:
 *      returns the global array of currently loaded
 *      plugin classes in the specified category. The
 *      linked list contains pointers to PLUGINCLASS
 *      structures.
 *
 *      For this to work, you must only use this
 *      function in a block between plgLoadClasses
 *      and plgFreeClasses. Do not modify the items
 *      on the list. Do not work on the list after
 *      you have called plgFreeClasses because
 *      the list might then have been freed.
 *
 *@@added V0.9.9 (2001-03-09) [umoeller]
 */

PLINKLIST plgQueryClasses(PPLUGINCATEGORY pplgCategory)
{
    return &pplgCategory->llClasses;
}

/*
 *@@ plgFreeClasses:
 *      decreases the reference count for the category
 *      plugin classes array by one. If 0 is reached,
 *      all allocated resources are freed, and plugin
 *      DLL's are unloaded.
 *
 *      See plgLoadClasses().
 *
 *      May run on any thread.
 *
 *@@added V0.9.7 (2000-12-02) [umoeller]
 *@@changed V0.9.9 (2001-03-09) [umoeller]: added PRIVATEWIDGETCLASS wrapping
 *@@changed V0.9.9 (2001-03-09) [umoeller]: converted global array to linked list
 *@@changed V0.9.12 (2001-05-20) [umoeller]: added mutex protection to fix multiple loads
 *@@changed V0.9.16 (2001-12-08) [umoeller]: added logging if unload fails
 */

VOID plgFreeClasses(PPLUGINCATEGORY pplgCategory)
{
    BOOL fLocked = FALSE;
    TRY_LOUD(excpt2)
    {
        if (fLocked = plgLockClasses(pplgCategory))
        {
            if (pplgCategory->ulClassesRefCount == 0)
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "ulClassesRefCount is already 0 for category %s!",
                       pplgCategory->pcszName);
            else
            {
                pplgCategory->ulClassesRefCount--;
                if (pplgCategory->ulClassesRefCount == 0)
                {
                    // no more references to the data:
                    PLISTNODE pNode = lstQueryFirstNode(&pplgCategory->llClasses);
                    while (pNode)
                    {
                        PPRIVATEPLUGINCLASS pClass
                            = (PPRIVATEPLUGINCLASS)pNode->pItemData;

                        free(pClass);

                        pNode = pNode->pNext;
                    }

                    // unload modules
                    pNode = lstQueryFirstNode(&pplgCategory->llModules);
                    while (pNode)
                    {
                        HMODULE hmod = (HMODULE)pNode->pItemData;
                        APIRET  arc;
                        CHAR    szModuleName[CCHMAXPATH];

                        // added logging if unload fails
                        // V0.9.16 (2001-12-08) [umoeller]
                        if (!DosQueryModuleName(hmod,
                                                sizeof(szModuleName),
                                                szModuleName))
                            arc = FreeModule(hmod,
                                             TRUE);       // call uninit
#ifdef __DEBUG__        // only in debug mode now V1.0.0 (2002-08-26) [umoeller]
                            if (arc)
                                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                                       "FreeModule returned %d for %s (hmod %lX)",
                                       arc,
                                       szModuleName,
                                       hmod);
#endif

                        pNode = pNode->pNext;
                    }

                    lstClear(&pplgCategory->llModules);
                    lstClear(&pplgCategory->llClasses);

                    pplgCategory->fClassesLoaded = FALSE;
                }
            }

            // _PmpfF(("leaving, ulCategoryClassesRefCount is %d", pplgCategory->ulClassesRefCount));
        }
    }
    CATCH(excpt2) {} END_CATCH();

    if (fLocked)
        plgUnlockClasses(pplgCategory);
}

/*
 *@@ plgFindClass:
 *      finds the PLGCLASS entry from the
 *      global array which has the given widget class
 *      name (_not_ PM window class name!).
 *
 *      Returns:
 *
 *      --  NO_ERROR: *ppClass has been set to the
 *          class struct.
 *
 *      --  PLGERR_INVALID_CLASS_NAME: class doesn't exist.
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      Preconditions:
 *
 *      --  Use this in a block between plgLoadClasses
 *          and plgFreeClasses.
 *
 *      --  Caller must hold the classes mutex.
 *
 *      Postconditions:
 *
 *      --  This returns a plain pointer to an item
 *          in the global classes array. Once the
 *          classes are unloaded, the pointer must
 *          no longer be used.
 *
 *      May run on any thread.
 *
 *@@added V0.9.7 (2000-12-02) [umoeller]
 *@@changed V0.9.9 (2001-03-09) [umoeller]: added PRIVATEWIDGETCLASS wrapping
 *@@changed V0.9.9 (2001-03-09) [umoeller]: converted global array to linked list
 *@@changed V0.9.12 (2001-05-12) [umoeller]: added extra non-null check
 *@@changed V0.9.19 (2002-04-25) [umoeller]: changed prototype, added fMustBeTrayable.
 */

APIRET plgFindClass(PPLUGINCATEGORY pplgCategory,
                    PCSZ pcszWidgetClass,
                    PCPLUGINCLASS *ppClass)
{
    APIRET arc = PLGERR_INVALID_CLASS_NAME;

    PLISTNODE pNode = lstQueryFirstNode(&pplgCategory->llClasses);

    if (!pcszWidgetClass || !ppClass)
        return ERROR_INVALID_PARAMETER;

    while (pNode)
    {
        PPLUGINCLASS pClass
            = (PPLUGINCLASS)pNode->pItemData;

        if (    (pClass)        // V0.9.12 (2001-05-12) [umoeller]
             && (!strcmp(pClass->pcszClass,
                         pcszWidgetClass))
           )
        {
            // found:
            *ppClass = pClass;
            arc = NO_ERROR;

            break;
        }

        pNode = pNode->pNext;
    }

    return arc;    // can be NULL
}

BOOL plgIsClassBuiltIn(PPLUGINCLASS pClass)
{
    if (pClass)
        return !((PPRIVATEPLUGINCLASS)pClass)->hmod;

    return FALSE;
}

APIRET plgQueryClassVersion(PPLUGINCLASS pClass,
                            PULONG pulMajor,
                            PULONG pulMinor,
                            PULONG pulRevision)
{
    APIRET arc = NO_ERROR;

    if (pClass)
    {
        if (pulMajor)
            *pulMajor = ((PPRIVATEPLUGINCLASS)pClass)->ulVersionMajor;
        if (pulMinor)
            *pulMinor = ((PPRIVATEPLUGINCLASS)pClass)->ulVersionMinor;
        if (pulRevision)
            *pulRevision = ((PPRIVATEPLUGINCLASS)pClass)->ulVersionRevision;
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

APIRET plgQueryClassModuleName(PPLUGINCLASS pClass,
                               ULONG cbName,
                               PCHAR pch)
{
    APIRET arc = NO_ERROR;

    if (pClass)
    {
        arc = DosQueryModuleName(((PPRIVATEPLUGINCLASS)pClass)->hmod,
                                 cbName,
                                 pch);
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

BOOL plgIsClassVisible(PPLUGINCLASS pClass)
{
    if (pClass)
        return TRUE;
    else
        return FALSE;
}

BOOL plgIsClassEnabled(PPLUGINCLASS pClass)
{
    if (pClass)
        return TRUE;
    else
        return FALSE;
}

/* ******************************************************************
 *
 *   Widget settings management
 *
 ********************************************************************/

/*
 *@@ plgCheckClass:
 *      checks if the given class is valid
 *
 *      Returns:
 *
 *      --  NO_ERROR: class exists and is trayable.
 *
 *      --  PLGERR_INVALID_CLASS_NAME: class doesn't exist.
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *@@added V0.9.19 (2002-04-25) [umoeller]
 */

APIRET plgCheckClass(PPLUGINCATEGORY pplgCategory,
                     PCSZ pcszClass)
{
    PCPLUGINCLASS    pClass;
    APIRET           arc = PLGERR_INVALID_CLASS_NAME;

    if (!pcszClass)
        return ERROR_INVALID_PARAMETER;

    if (plgLockClasses(pplgCategory))
    {
        arc = plgFindClass(pplgCategory,
                           pcszClass,
                           &pClass);

        plgUnlockClasses(pplgCategory);
    }

    return arc;
}


