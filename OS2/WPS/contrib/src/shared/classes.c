
/*
 *@@sourcefile classlst.c:
 *      WPS class list functions.
 *
 *      Function prefix for this file:
 *      --  cls*
 *
 *      This contains a "WPS class list library",
 *      which can be used by any part of XWorkplace to have
 *      the WPS class list inserted into any container. See
 *      clsWpsClasses2Cnr for details.
 *
 *      New with V0.9.0 are the "Method information" functions,
 *      which return method information for a given class. See
 *      clsQueryMethodInfo for details.
 *
 *      These functions have been moved to this new file with
 *      V0.9.1.
 *
 *@@header "shared\classes.h"
 */

/*
 *      Copyright (C) 1997-2006 Ulrich M”ller.
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

/*
 *@@todo:
 *  somIsObj (somapi.h):
 *      Test whether <obj> is a valid SOM object. This test is based solely on
 *      the fact that (on this architecture) the first word of a SOM object is a
 *      pointer to its method table. The test is therefore most correctly understood
 *      as returning true if and only if <obj> is a pointer to a pointer to a
 *      valid SOM method table. If so, then methods can be invoked on <obj>.
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
#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WINTIMER
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINSTDCNR
#define INCL_WINMLE
#define INCL_WINSTDFILE
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xclslist.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classes.h"             // WPS class list helper functions
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file

#pragma hdrstop                     // VAC++ keeps crashing otherwise
#include <wpclsmgr.h>               // this includes SOMClassMgr

/* ******************************************************************
 *
 *   "Classes to container" functions
 *
 ********************************************************************/

/*
 *@@  clsAddClass2Cnr:
 *      this inserts one single WPS class record core into the
 *      given container.
 *
 *      We create a new record core (of CLASSRECORDCORE type) from the
 *      data which is given in the WPSLISTITEM structure.
 *      SELECTCLASSDATA is a "Select class" dialog description
 *      structure used throughout this file.
 *      All these structs are defined in classlst.h.
 *
 *      <B>Usage:</B> this gets called from the other class list
 *      functions to have reccs inserted. No need to call this
 *      manually.
 *
 *@@changed V0.9.1 (99-12-10) [umoeller]: moved this func here from config\clslist.c
 */

PCLASSRECORDCORE clsAddClass2Cnr(HWND hwndCnr,                  // in: cnr to insert into
                                 PCLASSRECORDCORE preccParent,  // in: parent recc for tree view
                                 PWPSLISTITEM pwpsMyself,       // in: class description
                                 PSELECTCLASSDATA pscd)         // in: dlg info
{
    // recc attributes
    ULONG               usAttrs = CRA_EXPANDED | CRA_RECORDREADONLY;

    PCLASSRECORDCORE    preccNew
        = (PCLASSRECORDCORE)cnrhAllocRecords(hwndCnr,
                                             sizeof(CLASSRECORDCORE),
                                             1);
    PMPF_CLASSLIST(("  clsAddClass2Cnr %s\n", pwpsMyself->pszClassName));

    if (preccNew)
    {
        // store the WPS class info structure into recc
        // set extra recc attribute (WPSLISTITEM)
        if (preccNew->pwps = pwpsMyself)
        {
            pwpsMyself->pRecord = (PRECORDCORE)preccNew;

            if (pscd)
                // now call callback function defined in
                // SELECTCLASSDATA to determine the recc
                // attributes; this way, a class can be
                // disabled, expanded etc.
                if (pscd->pfnwpReturnAttrForClass)
                    usAttrs = (USHORT)pscd->pfnwpReturnAttrForClass(
                                                      hwndCnr,
                                                      (ULONG)pscd,
                                                      (MPARAM)pwpsMyself,
                                                      (MPARAM)preccParent);
                else
                {
                    // no callback defined: set read-only
                    usAttrs = CRA_RECORDREADONLY;
                    // and expand three tree view levels
                    if (pscd->lRecurse < 3)
                        usAttrs |= CRA_EXPANDED;
                }
        }
        else
            // pwpsMyself might be NULL for the "Orphans" tree parent item
            usAttrs = CRA_RECORDREADONLY;

        PMPF_CLASSLIST(("    Inserting %s\n", pwpsMyself->pszClassName));

        // insert the record (helpers/winh.c)
        cnrhInsertRecords(hwndCnr,
                          (PRECORDCORE)preccParent,
                          (PRECORDCORE)preccNew,
                          TRUE, // invalidate
                          (pwpsMyself)
                              ? pwpsMyself->pszClassName
                              : pscd->pcszOrphans,     // "Orphans" string
                          usAttrs,
                          1);

        // select the new record?
        if (usAttrs & (CRA_INUSE | CRA_CURSORED | CRA_PICKED | CRA_SELECTED))
        {
            PMPF_CLASSLIST(("    Selecting %s\n", pwpsMyself->pszClassName));

            WinSendMsg(hwndCnr,
                       CM_SETRECORDEMPHASIS,
                       (MPARAM)preccNew,
                       MPFROM2SHORT(TRUE,
                                    (USHORT)(usAttrs & (CRA_INUSE
                                                        | CRA_CURSORED
                                                        | CRA_PICKED
                                                        | CRA_SELECTED)
                                                       )
                                            )
                      );
        }

        // expand the new record? V0.9.15 (2001-09-14) [umoeller]
        if (usAttrs & CRA_EXPANDED)
            WinSendMsg(hwndCnr,
                       CM_EXPANDTREE,
                       (MPARAM)preccNew,
                       0);

        // mark the class as processed (needed by clsWPSClasses2Cnr)
        if (pwpsMyself)
            pwpsMyself->fProcessed = TRUE;
    }

    return preccNew;
}

/*
 *@@ clsResolveReplacements:
 *      called from clsWpsClasses2Cnr to resolve class
 *      replacements.
 *
 *      Initially this gets called with pwpsCurrent pointing
 *      to the root item to be inserted. This will then
 *      recurse into the children.
 *
 *      This was added with 0.9.15 to finally resolve all
 *      class replacements _before_ the records get
 *      inserted into the container, because otherwise
 *      the "get record attrs" callback can never determine
 *      whether a class replacement is in effect.
 *
 *@@added V0.9.15 (2001-09-14) [umoeller]
 */

VOID clsResolveReplacements(PWPSLISTITEM pwpsCurrent,
                                // in: class to process; on the first
                                // call, this will be the "root" class,
                                // but other classes when recursing
                            PWPSCLASSESINFO pwpsci)
                                // in: complete list
{
    PLISTNODE pNode = 0;
    PWPSLISTITEM pwpsParentToCheck;

    PMPF_CLASSLIST(("Checking replacements for %s",
                    pwpsCurrent->pszClassName));

    // query if any parent class has been replaced by this class;
    // in order to find out about that, we climb up the parent record
    // cores in the container and ask the WPS class manager if
    // replacements are in effect
    pwpsParentToCheck = pwpsCurrent->pParent;
            // start out with parent; this might be NULL if
            // the class has no parent (i.e. is the root class)
    while (pwpsParentToCheck)
    {
        // get class name for parent class object
        // PSZ pszParentToCheckClass = _somGetName(pwpsParentToCheck->pClassObject);
        // call WPS class manager to query replacement;
        // SOMClassMgrObject is a global variable defined
        // by the SOM kernel, and the WPS replaces the
        // SOM class manager with its own (see WPSREF)
        if (_wpReplacementIsInEffect(SOMClassMgrObject,
                                     pwpsParentToCheck->pszClassName, // parent class
                                     pwpsCurrent->pszClassName)) // current class
        {
            // yes, pszParentToCheckClass has been replaced by current class:
            // store replacement in our structure
            PMPF_CLASSLIST(("  %s replaces %s",
                            pwpsCurrent->pszClassName,
                            pwpsParentToCheck->pszClassName));
            pwpsCurrent->pszReplacesClass = strdup(pwpsParentToCheck->pszClassName);
            // did we find replacements of the parent already?
            if (pwpsParentToCheck->pszReplacedWithClasses)
            {
                // if there are several class replacements for the parent,
                // append this class name to the other replacements
                PSZ psz2 = malloc(   strlen(pwpsParentToCheck->pszReplacedWithClasses)
                                   + strlen(pwpsCurrent->pszClassName)
                                   + 6);
                sprintf(psz2, "%s, %s",
                        pwpsParentToCheck->pszReplacedWithClasses,
                        pwpsCurrent->pszClassName);
                free(pwpsParentToCheck->pszReplacedWithClasses);
                pwpsParentToCheck->pszReplacedWithClasses = psz2;
            }
            else
            {
                // otherwise, use this class name only
                pwpsParentToCheck->pszReplacedWithClasses
                = strdup(pwpsCurrent->pszClassName);
            }
        }
        else
            PMPF_CLASSLIST(("  %s does not replace %s",
                            pwpsCurrent->pszClassName,
                            pwpsParentToCheck->pszClassName));

        // climb up to next parent
        pwpsParentToCheck = pwpsParentToCheck->pParent;
    } // end while (preccParentToCheck)

    // now recurse for all children of this item
    pNode = lstQueryFirstNode(pwpsci->pllClassList);
    while (pNode)
    {
        PWPSLISTITEM pwps0 = pNode->pItemData;
        if (pwps0->pParentClassObject == pwpsCurrent->pClassObject)
        {
            // this class is an immediate child of ours:
            // recurse! This will add this class's
            // record core to the cnr and check for
            // children again
            clsResolveReplacements(pwps0,
                                   pwpsci);
        }
        pNode = pNode->pNext;
    }
}

/*
 *@@ clsAddClassTree2Cnr:
 *      this gets initially called by clsWpsClasses2Cnr
 *      with pwpsCurrent being the list item for the
 *      root class (normally WPObject). This will insert
 *      that class into the container and then recurse
 *      to process descendant classes.
 *
 *      <B>Usage:</B> this gets called from the other class list
 *      functions to have reccs inserted. No need to call this
 *      manually.
 *
 *@@changed V0.9.1 (99-12-10) [umoeller]: moved this func here from config\clslist.c
 */

VOID clsAddClassTree2Cnr(HWND hwndCnr,
                                // in: container window
                                // (this is constant even for recursive
                                // calls)
                         PWPSLISTITEM pwpsCurrent,
                                // in: class to process; on the first
                                // call, this will be the "root" class,
                                // but other classes when recursing
                         PWPSCLASSESINFO pwpsci,
                                // in: complete list
                         PCLASSRECORDCORE preccParent,
                                // in: parent record for tree view;
                                // on the first call, this will be
                                // NULL for the root level
                         PSELECTCLASSDATA pscd)
                                // in: insertion info structure
                                // (this is constant even for recursive
                                // calls)
{
    PLISTNODE pNode = 0;

#if 0
    PWPSLISTITEM pwps0, pwpsParentToCheck;
    PCLASSRECORDCORE preccParentToCheck;
    PSZ pszParentToCheckClass;

    // V0.9.15 (2001-09-14) [umoeller]:
    // moved clsAddClass2Cnr call down

    // query if any parent class has been replaced by this class;
    // in order to find out about that, we climb up the parent record
    // cores in the container and ask the WPS class manager if
    // replacements are in effect
    preccParentToCheck = preccParent;
            // start out with parent; this might be NULL if
            // the class has no parent (i.e. is the root class)
    while (preccParentToCheck)
    {
        // get WPSLISTITEM struct from record core
        pwpsParentToCheck = preccParentToCheck->pwps;
        // get class name for parent class object
        pszParentToCheckClass = _somGetName(pwpsParentToCheck->pClassObject);
        // call WPS class manager to query replacement;
        // SOMClassMgrObject is a global variable defined
        // by the SOM kernel, and the WPS replaces the
        // SOM class manager with its own (see WPSREF)
        if (_wpReplacementIsInEffect(SOMClassMgrObject,
                                     pszParentToCheckClass,      // parent class
                                     pwpsCurrent->pszClassName)) // current class
        {
            // yes, pszParentToCheckClass has been replaced by current class:
            // store replacement in our structure
            pwpsCurrent->pszReplacesClass = strdup(pszParentToCheckClass);
            // did we find replacements of the parent already?
            if (pwpsParentToCheck->pszReplacedWithClasses)
            {
                // if there are several class replacements for the parent,
                // append this class name to the other replacements
                PSZ psz2 = malloc(   strlen(pwpsParentToCheck->pszReplacedWithClasses)
                                   + strlen(pwpsCurrent->pszClassName)
                                   + 6);
                sprintf(psz2, "%s, %s",
                        pwpsParentToCheck->pszReplacedWithClasses,
                        pwpsCurrent->pszClassName);
                free(pwpsParentToCheck->pszReplacedWithClasses);
                pwpsParentToCheck->pszReplacedWithClasses = psz2;
            }
            else
            {
                // otherwise, use this class name only
                pwpsParentToCheck->pszReplacedWithClasses
                = strdup(pwpsCurrent->pszClassName);
            }
        }

        // climb up to next parent; we do this
        // by finding the parent record core in the container
        preccParentToCheck = (PCLASSRECORDCORE)WinSendMsg(hwndCnr,
                                                          CM_QUERYRECORD,
                                                          (MPARAM)preccParentToCheck,
                                                          MPFROM2SHORT(CMA_PARENT,
                                                                        // get parent recc
                                                                       CMA_ITEMORDER));
        if (preccParentToCheck == (PCLASSRECORDCORE)-1)
            // container reported error: stop
            preccParentToCheck = NULL;
        // if it's NULL, the while loop will exit
    } // end while (preccParentToCheck)
#endif

    // add the current class to the cnr;
    // initially, this is WPObject, for recursive calls,
    // this will be a child of WPObject
    clsAddClass2Cnr(hwndCnr,
                    preccParent, // parent recc
                    pwpsCurrent,
                    pscd);
            // V0.9.15 (2001-09-14) [umoeller]:
            // moved this code down, because the "get recc attrs"
            // callback checks for replacement classes, which
            // are only determined in the above code... duh,
            // no wonder this never worked

    // now go thru WPS class list and add all immediate
    // children of ours
    pNode = lstQueryFirstNode(pwpsci->pllClassList);
    while (pNode)
    {
        PWPSLISTITEM pwps0 = pNode->pItemData;
        if (pwps0->pParentClassObject == pwpsCurrent->pClassObject)
        {
            // this class is an immediate child of ours:
            // recurse! This will add this class's
            // record core to the cnr and check for
            // children again
            clsAddClassTree2Cnr(hwndCnr,
                                pwps0,
                                pwpsci,
                                (PCLASSRECORDCORE)pwpsCurrent->pRecord,
                                pscd);
        }
        pNode = pNode->pNext;
    }
}

/*
 *@@ clsWpsClasses2Cnr:
 *      this func inserts a WPS class hierarchy starting with
 *      the class "pszCurrentClass" as the root class into
 *      a given container window. This cnr should be in tree
 *      view to have a meaningful display, and better be
 *      empty before calling this.
 *
 *      This function is used directly by the XWPClassList class,
 *      but could be used with any other existing container also.
 *
 *      Also, this func gets called by fnwpSelectWPSClass (for
 *      the modal "Select class" dialog).
 *
 *      This function takes a SELECTCLASSDATA as input, which
 *      has lots of information about how classes are to be inserted.
 *
 *      In that structure, you can set up some callbacks
 *      that will be called to allow certain extra configuration.
 *      This is defined and explained in classlst.h.
 *      If any callback is set to NULL, safe defaults are used.
 *
 *      This function only inserts all the containers as
 *      CLASSRECORDCORE structures. It does _not_ handle
 *      anything later. It is your responsibility to make
 *      the container useful, e.g. by intercepting container
 *      WM_CONTROL messages to provide for context menus and
 *      all that.
 *
 *      Take a look at config/classlst.c to find out how the
 *      XWPClassList class view utilizes this function.
 *
 *      <B>Warning:</B> This function eats up a lot of memory,
 *      depending on how many WPS classes are installed. This
 *      memory is NOT freed when this function returns, because
 *      the container still needs it. AFTER destroying the
 *      container window, you must issue clsCleanupWpsClasses
 *      (below) with the PWPSCLASSESINFO return value of this
 *      function. This will free the internal linked list of
 *      WPS class items and all allocated SOM resources.
 *
 *@@changed V0.9.1 (99-12-07) [umoeller]: fixed memory leak
 *@@changed V0.9.1 (99-12-10) [umoeller]: moved this func here from config\clslist.c
 *@@changed V0.9.15 (2001-09-14) [umoeller]: reorganized to fix replacements resolution which never worked in time
 *@@changed V1.0.2 (2003-11-13) [umoeller]: fixed broken DLL name lookup
 */

PWPSCLASSESINFO clsWpsClasses2Cnr(HWND hwndCnr, // in: guess what this is
                                  PCSZ pcszRootClass,  // in: the "root" class of the display,
                                                     // e.g. "WPObject" -- make sure this
                                                     // class exists, or nothing happens
                                  PSELECTCLASSDATA pscd)          // in: insertion info struct
{
    PWPSCLASSESINFO  pwpsciReturn = NULL;
    POBJCLASS        pObjClass;
    PWPSLISTITEM     pwpsRoot = NULL;
    somId            somidRoot;
    SOMClass         *pRootClassObject = NULL;

    SOMClassSequence RegisteredClasses;

    ULONG   ulSize;
    ULONG   ul;

    /*
     * 1) prepare data
     *
     */

    // create common buffer
    pwpsciReturn = malloc(sizeof(WPSCLASSESINFO));
    memset(pwpsciReturn, 0, sizeof(WPSCLASSESINFO));

    pwpsciReturn->pllClassList = lstCreate(TRUE);   // items are freeable

    // get WPS class list
    pwpsciReturn->pObjClass = (POBJCLASS)winhQueryWPSClassList();

    /*
     * 2) handle orphans
     *
     */

    if (pscd->pcszOrphans)
    {
        // "Orphans" tree requested: load all
        // registered WPS classes.
        PCLASSRECORDCORE preccOrphans = NULL;
                // for "root" record cor ("Orphaned classes")
        pObjClass = pwpsciReturn->pObjClass;

        // now go thru the WPS class list
        while (pObjClass)
        {
            // the following will LOAD the class, if it
            // hasn't been loaded by the WPS already
            SOMClass    *pClassThis;
            somId       somidThis = somIdFromString(pObjClass->pszClassName);
            if (!(pClassThis = _somFindClass(SOMClassMgrObject, somidThis, 0, 0)))
            {
                // class object == NULL: means that class loading failed,
                // i.e. it is registered with the WPS, but could not
                // be found
                PWPSLISTITEM pwpsNew;

                PMPF_SOMFREAK(("[%s] is orphaned (somid 0x%lX)",
                               pObjClass->pszClassName,
                               somidThis));

                if (pwpsNew = NEW(WPSLISTITEM))
                {
                    ZERO(pwpsNew);

                    // create "orphaned" tree the first time
                    if (!preccOrphans)
                        preccOrphans = clsAddClass2Cnr(hwndCnr,
                                                       NULL,       // parent recc
                                                       NULL,       // "orphans" string
                                                       pscd);

                    // set up WPSLISTITEM data
                    pwpsNew->pszClassName = pObjClass->pszClassName;
                    pwpsNew->pszModName = pObjClass->pszModName;
                    // add orphaned class to "Orphaned" tree
                    clsAddClass2Cnr(hwndCnr, preccOrphans, pwpsNew,
                                    pscd);
                    // append list item to list V0.9.1 (99-12-07)
                    lstAppendItem(pwpsciReturn->pllClassList,
                                  pwpsNew);
                }
            }

            PMPF_SOMFREAK(("[%s] is 0x%lX (somid 0x%lX)",
                           pObjClass->pszClassName,
                           pClassThis,
                           somidThis));

            SOMFree(somidThis);

            // next class
            pObjClass = pObjClass->pNext;
        } // end while (pObjClass)
    } // end if (pscd->pszOrphans)

    // OK, now we have the orphaned classes in the container.

    /*
     * 3) create linked list of WPSLISTITEMs
     *
     */

    // Now get SOMClassMgr's list of registered classes.
    // This is only mentioned in SOMCM.IDL and not
    // otherwise documented, but it works. This list
    // only contains the class objects which have
    // actually been successfully loaded.
    // (Note that we attempted to load all registered
    // classes above.)
    // (This is new with V0.82. Previous versions used the
    // WPS class list, which returned garbage sometimes.)
    RegisteredClasses = __get_somRegisteredClasses(SOMClassMgrObject);
            // this func returns a SOMClassSequence:
            //      _length     has the no. of items
            //      _buffer[_length] has the items

    // get class object of root class to process
    if (somidRoot = somIdFromString((PSZ)pcszRootClass))
    {
        PLISTNODE pNode;

        // find root class object (e.g. M_WPObject).
        pRootClassObject = _somFindClass(SOMClassMgrObject, somidRoot, 0, 0);

        // Note: somFindClass will return the replacement class
        // if the given root class has been replaced.
        // (That's how class replacements work. See somSubstituteClass
        // in SOMREF for details.) For example, if pszRootClass
        // is "WPObject", we will now have the XFldObject class
        // object. So if the title is not right, we need to climb
        // up the parents until we find the "real" root.
        while (     (pRootClassObject)
                 && (strcmp(_somGetName(pRootClassObject), pcszRootClass))
              )
            pRootClassObject = _somGetParent(pRootClassObject);

        // now go thru SOMClassMgr's class list
        // and build a linked list (linklist.h)
        // with all the classes
        // (RegisteredClasses is a SOMClassSequence)
        for (ul = 0;
             ul < RegisteredClasses._length;
             ul++)
        {
            PWPSLISTITEM pwpsNew;

            // current class to work on
            SOMClass *pClass = RegisteredClasses._buffer[ul];

            // filter out unwanted classes; the SOM class
            // tree contains SOMClass, SOMObject, the interface
            // repository and all that, which we don't want,
            // so we include only descendants of the "root"
            // class
            if (    (_somDescendedFrom(pClass, pRootClassObject))
                 && (pwpsNew = NEW(WPSLISTITEM))
               )
            {
                // set up WPSLISTITEM data
                ZERO(pwpsNew);

                pwpsNew->pClassObject = pClass;
                pwpsNew->pszClassName = _somGetName(pClass);

                PMPF_SOMFREAK(("%d: found class [%s] at 0x%lX",
                               ul,
                               pwpsNew->pszClassName,
                               pClass));

                // are we currently working on the root class object?
                // If so, store it for later
                if (pwpsNew->pClassObject == pRootClassObject)
                    pwpsRoot = pwpsNew;

                // browse WPS class list to find out the DLL;
                // this will find the DLL only if the class
                // has been registered with the WPS (there are
                // some classes that are not)
                // V1.0.2 (2003-11-13) [umoeller]: reset pObjClass, this broke
                // DLL name resolution at one point (this must have worked once!)
                pObjClass = pwpsciReturn->pObjClass;
                while (pObjClass)
                {
                    if (!strcmp(pwpsNew->pszClassName,
                                pObjClass->pszClassName))
                    {
                        PMPF_SOMFREAK(("found dll %s for %s",
                               pObjClass->pszModName,
                               pwpsNew->pszClassName));

                        pwpsNew->pszModName = pObjClass->pszModName;
                        break;
                    }

                    pObjClass = pObjClass->pNext;
                }

                // else DLL = NULL; this happens if class loading
                // failed or if an undocumented WPS class does not
                // appear in the "official" WPS class list

                // get class's parent
                if (pwpsNew->pParentClassObject = _somGetParent(pClass))
                {
                    strlcpy(pwpsNew->szParentClass,
                            _somGetName(pwpsNew->pParentClassObject),
                            sizeof(pwpsNew->szParentClass));
                    PMPF_SOMFREAK(("    parent is [%s] at 0x%lX",
                                   pwpsNew->szParentClass,
                                   pwpsNew->pParentClassObject));
                }
                else
                    PMPF_SOMFREAK(("    no parent found"));

                // mark this list item as "not processed" for later
                // pwpsNew->fProcessed = FALSE; we zeroed above V1.0.1 (2003-02-01) [umoeller]

                // append list item to list
                lstAppendItem(pwpsciReturn->pllClassList,
                              pwpsNew);
            }
        }

        /*
         * 4) resolve parents
         *
         */

        // now run thru the list we have created and for
        // each WPSLISTITEM, set the parent pointer
        for (pNode = lstQueryFirstNode(pwpsciReturn->pllClassList);
             pNode;
             pNode = pNode->pNext)
        {
            PWPSLISTITEM pThis = (PWPSLISTITEM)pNode->pItemData;

            // root item has no parent
            if (pThis != pwpsRoot)
            {
                PLISTNODE pNode2;

                for (pNode2 = lstQueryFirstNode(pwpsciReturn->pllClassList);
                     pNode2;
                     pNode2 = pNode2->pNext)
                {
                    PWPSLISTITEM pThis2 = (PWPSLISTITEM)pNode2->pItemData;
                    if (!strcmp(pThis->szParentClass,
                                pThis2->pszClassName))
                    {
                        PMPF_CLASSLIST(("Parent of %s is %s",
                                        pThis->pszClassName,
                                        pThis2->pszClassName));
                        pThis->pParent = pThis2;
                        break;
                    }
                }
            }
        }

        /*
         * 5) resolve replacements
         *
         */

        clsResolveReplacements(pwpsRoot,
                               pwpsciReturn);

        /*
         * 6) create and insert records
         *
         */

        // now insert the root record (normally WPObject);
        // this will then recurse and also handle class replacement info
        clsAddClassTree2Cnr(hwndCnr,
                            pwpsRoot,
                            pwpsciReturn,
                            NULL,
                            pscd);

        SOMFree(somidRoot);

        // free the sequence buffer V1.0.1 (2003-02-01) [umoeller]
        SOMFree(RegisteredClasses._buffer);
    }

    return pwpsciReturn;
}

/*
 *@@ clsCleanupWpsClasses:
 *      this cleans up the resources allocated by
 *      clsWpsClasses2Cnr (memory and SOM stuff);
 *      pwpsci must be the pointer returned
 *      by clsWpsClasses2Cnr.
 *
 *@@changed V0.9.1 (99-12-10) [umoeller]: moved this func here from config\clslist.c
 *@@changed V1.0.1 (2002-12-15) [pr]: prevent null pointer trap @@fixes 243
 */

VOID clsCleanupWpsClasses(PWPSCLASSESINFO pwpsci) // in: struct returned by clsWpsClasses2Cnr
{
    if (pwpsci)
    {
        PLISTNODE pNode = lstQueryFirstNode(pwpsci->pllClassList);
        PWPSLISTITEM pItem;

        while (pNode)
        {
            pItem = pNode->pItemData;
            if (pItem->pszReplacedWithClasses)
                free(pItem->pszReplacedWithClasses);
            if (pItem->pszReplacesClass)
                free(pItem->pszReplacesClass);

            pNode = pNode->pNext;
        }

        lstFree(&pwpsci->pllClassList);

        free(pwpsci->pObjClass);
        free(pwpsci);
    }
}

/* ******************************************************************
 *
 *   "Select class" dialog
 *
 ********************************************************************/

/*
 *@@ fnwpSelectWPSClass:
 *      dlg proc for "Select class" dialog for selecting a
 *      class in single-object status bar mode
 *
 *@@changed V0.9.0 [umoeller]: changed colors
 *@@changed V0.9.1 (99-12-10) [umoeller]: moved this func here from config\clslist.c
 */

MRESULT EXPENTRY fnwpSelectWPSClass(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc;
    PSELECTCLASSDATA pscd = (PSELECTCLASSDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);

    switch(msg)
    {
        /*
         * WM_INITDLG:
         *      set up the container data and colors
         */

        case WM_INITDLG:
        {
            CNRINFO CnrInfo;
            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)mp2);
            pscd = (PSELECTCLASSDATA)mp2;

            WinSetWindowText(hwndDlg, pscd->pszDlgTitle);
            WinSetDlgItemText(hwndDlg, ID_XLDI_INTROTEXT, pscd->pszIntroText);

            // setup container
            pscd->hwndCnr = WinWindowFromID(hwndDlg, ID_XLDI_CNR);
            WinSendMsg(pscd->hwndCnr, CM_QUERYCNRINFO, &CnrInfo, (MPARAM)sizeof(CnrInfo));
            CnrInfo.pSortRecord = (PVOID)fnCompareName;
            CnrInfo.flWindowAttr =
                    CV_TREE | CA_TREELINE | CV_TEXT
                    | CA_OWNERDRAW;
            CnrInfo.cxTreeIndent = 30;
            WinSendMsg(pscd->hwndCnr, CM_SETCNRINFO,
                    &CnrInfo,
                    (MPARAM)(CMA_PSORTRECORD | CMA_FLWINDOWATTR | CMA_CXTREEINDENT));

            // hide help button if panel was not specified
            if (pscd->ulHelpPanel == 0)
                winhShowDlgItem(hwndDlg, DID_HELP, FALSE);

            // get container colors for owner draw later (changed V0.9.0)
            /* pscd->lBackground =
                    winhQueryPresColor(hwndDlg, PP_BACKGROUNDCOLOR, FALSE, SYSCLR_BACKGROUND);
            pscd->lText =
                    winhQueryPresColor(hwndDlg, PP_FOREGROUNDCOLOR, FALSE, SYSCLR_WINDOWTEXT);
            pscd->lStaticText =
                    WinQuerySysColor(HWND_DESKTOP, SYSCLR_SHADOWTEXT, 0);
            pscd->lSelBackground =
                    winhQueryPresColor(hwndDlg, PP_HILITEBACKGROUNDCOLOR, SYSCLR_HILITEBACKGROUND);
            pscd->lSelText =
                    winhQueryPresColor(hwndDlg, PP_HILITEFOREGROUNDCOLOR, SYSCLR_HILITEFOREGROUND); */

            // fill container later
            WinPostMsg(hwndDlg, WM_FILLCNR, MPNULL, MPNULL);

            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        /*
         * WM_FILLCNR:
         *      this fills the cnr with all the WPS classes
         */

        case WM_FILLCNR:
        {
            PRECORDCORE preccSelected;
            HPOINTER hptrOld = winhSetWaitPointer();
            pscd->pwpsc = clsWpsClasses2Cnr(pscd->hwndCnr,
                                            pscd->pcszRootClass,
                                            pscd);  // also contains callback
            // scroll cnr to the item that is selected
            preccSelected = WinSendMsg(pscd->hwndCnr, CM_QUERYRECORDEMPHASIS,
                                      (MPARAM)CMA_FIRST,
                                      (MPARAM)CRA_SELECTED);
            cnrhScrollToRecord(pscd->hwndCnr, preccSelected,
                               CMA_TEXT, TRUE);
            WinSetPointer(HWND_DESKTOP, hptrOld);
        }
        break;

        /*
         * WM_DRAWITEM:
         *      container control owner draw: the
         *      cnr only allows the same color for
         *      all text items, so we need to draw
         *      the text ourselves
         */

        case WM_DRAWITEM:
            mrc = cnrhOwnerDrawRecord(mp2,
                                      CODFL_DISABLEDTEXT);
        break;

        /*
         * WM_CONTROL:
         *      capture cnr notifications
         */

        case WM_CONTROL:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            if (SHORT1FROMMP(mp1) == ID_XLDI_CNR)
            {
                switch (SHORT2FROMMP(mp1)) // notify code
                {
                    /*
                     * CN_ENTER:
                     *      double-click or "Enter" on
                     *      record core
                     */

                    case CN_ENTER:
                        if (pscd->fSelectionValid)
                        {
                            WinPostMsg(hwndDlg, WM_COMMAND,
                                    (MPARAM)DID_OK,
                                    MPNULL);
                        }
                    break;

                    /*
                     * CN_EMPHASIS:
                     *      selection changed: call
                     *      callback func specified in
                     *      SELECTCLASSDATA
                     */

                    case CN_EMPHASIS:
                    {
                        // get cnr notification struct
                        PNOTIFYRECORDEMPHASIS pnre = (PNOTIFYRECORDEMPHASIS)mp2;
                        if (pnre)
                            if (    (pnre->fEmphasisMask & CRA_SELECTED)
                                 && (pnre->pRecord)
                               )
                            {
                                pscd->preccSelection = (PCLASSRECORDCORE)(pnre->pRecord);
                                // per definition, this callback func must
                                // return TRUE if the OK button is to be
                                // enabled
                                if (pscd->pfnwpClassSelected) {
                                    pscd->fSelectionValid =
                                        (BOOL)(*(pscd->pfnwpClassSelected))(
                                            pscd->hwndCnr,
                                            pscd->ulUserClassSelected,
                                            // mp1: WPSLISTITEM struct
                                            (MPARAM)( ((PCLASSRECORDCORE)(pnre->pRecord))->pwps ),
                                            // mp2: hwnd of info static text control below cnr
                                            (MPARAM)( WinWindowFromID(hwndDlg, ID_XLDI_TEXT2) )
                                        );
                                    // enable OK button according to
                                    // callback return value
                                    WinEnableControl(hwndDlg, DID_OK, pscd->fSelectionValid);
                                }
                            }
                    }
                    break;
                }
            }
        break;

        /*
         * WM_COMMAND:
         *      if OK button was pressed (or CN_ENTER posted, see above),
         *      update the SELECTCLASSDATA struct and close dialog
         */

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case DID_OK:
                    if (pscd->preccSelection)
                        strlcpy(pscd->szClassSelected,
                                pscd->preccSelection->pwps->pszClassName,
                                sizeof(pscd->szClassSelected));
                break;
            }
            // close dialog
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;

        case WM_HELP:
            if (pscd->ulHelpPanel)
                _wpDisplayHelp(cmnQueryActiveDesktop(),
                               pscd->ulHelpPanel,
                               (PSZ)pscd->pszHelpLibrary);
        break;

        case WM_DESTROY:
            // cleanup
            clsCleanupWpsClasses(pscd->pwpsc);
            // we do not clean up the SELECTCLASSDATA (QWL_USER),
            // because this is expected to be static
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ clsSelectWpsClassDlg:
 *      this is a one-shot func for displaying a
 *      "Select WPS class" dialog.
 *
 *      This takes a SELECTCLASSDATA as input, which must
 *      be filled beforehand, e.g. as follows:
 +         SELECTCLASSDATA scd;
 +         memset(&scd, 0, sizeof(SELECTCLASSDATA));
 +         strcpy(scd.szDlgTitle, "Select Class");
 +         strcpy(scd.szIntroText, "Select the class for which you want to change "
 +                               "the status bar info.");
 +         strcpy(scd.szRootClass, "WPObject");
 +         strcpy(scd.szClassSelected, "WPObject");
 *
 *      The selected WPS class will then be written into
 *      the SELECTCLASSDATA structure.
 *
 *      This function requires the dialog templates from the
 *      NLS DLLs.
 *
 *      This func returns either DID_OK or DID_CANCEL,
 *      in which case you shouldn't evaluate the return
 *      values in SELECTCLASSDATA.
 *
 *      hmod and idDlg specify the dlg template for
 *      this func.
 *
 *@@changed V0.9.1 (99-12-10) [umoeller]: moved this func here from config\clslist.c
 */

ULONG clsSelectWpsClassDlg(HWND hwndOwner,
                           HMODULE hmod,
                           ULONG idDlg,
                           PSELECTCLASSDATA pscd)
{
    return WinDlgBox(HWND_DESKTOP, hwndOwner,
                     fnwpSelectWPSClass,
                     hmod, idDlg,
                     pscd);
}

/* ******************************************************************
 *
 *   Method information functions
 *
 ********************************************************************/

/*
 *@@ clsQueryMethods:
 *      this returns method information for the given class object
 *      in a newly allocated METHODINFO structure (classlst.h).
 *
 *      Pass this structure to clsFreeMethodInfo when you're done.
 *
 *      If (fClassMethods == FALSE), _instance_ method information
 *      is returned.
 *
 *      If (fClassMethods == TRUE), _class_ method information is
 *      returned.
 *
 *      This function is reentrant, so it can be called on a thread
 *      other than thread 1. This is recommended anyway because this
 *      method can take several seconds on slower systems.
 *
 *      If *pfAbort is set to TRUE while this function is running
 *      (from another thread, of course), this function aborts
 *      immediately and returns NULL.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (99-12-10) [umoeller]: moved this func here from config\clslist.c
 */

PMETHODINFO clsQueryMethodInfo(SOMClass *pClassObject,  // in: class to query method for
                               BOOL fClassMethods,      // in: class or instance methods?
                               PBOOL pfAbort)           // in: when this is set to TRUE, this function aborts
{
    PMETHODINFO pmi;
    if (pmi = (PMETHODINFO)malloc(sizeof(METHODINFO)))
    {
        ULONG ul = 0;

        // query class methods instead of instance methods?
        if (fClassMethods)
            // yes:
            // get "class object of the class object";
            // apparently, there is such a thing in SOM
            // internally, since all the SOMClass methods
            // which (according to the SOM docs) can be
            // invoked on class objects do work for
            // metaclasses also
            pClassObject = _somGetClass(pClassObject);
                // so now we have the "metaclass object"

        pmi->ulMethodCount = _somGetNumMethods(pClassObject);
            // this method returns the number of methods currently supported by the
            // specified class, including inherited methods (both static and dynamic);
            // so we now have the count of either instance or class methods

        // create methods list (holding SOMMETHOD structs)
        lstInit(&pmi->llMethods, FALSE);     // don't free items;
                                                // we'll do this manually later

        // now go for all the method indices
        for (ul = 0;
             ul < pmi->ulMethodCount;
             ul++)
        {
            somMethodData md;   // for somGetNthMethodData below
                /* From somapi.h:
                      typedef struct somMethodDataStruct {
                          somId id;
                          long type;            // 0=static, 1=dynamic 2=nonstatic
                          somId descriptor;     // for use with IR interfaces
                          somMToken mToken;     // NULL for dynamic methods
                          somMethodPtr method;  // depends on resolution context
                          somSharedMethodData *shared;
                      } somMethodData, *somMethodDataPtr;
                */

            if (_somGetNthMethodData(pClassObject, // class or metaclass object
                                     ul,           // index
                                     &md)          // out: buffer
                    == FALSE)
            {
                // not found:
                // store methods which we have successfully retrieved
                pmi->ulMethodCount = ul;
                // and get outta here NOW
                break;
            }
            else
            {
                // method found:
                SOMClass        *pClassObjectThis,  // current (meta)class object
                                *pClassObjectLast;  // (meta)class object of prev. loop

                somMethodProc   *pMethodThis,       // current method pointer
                                *pMethodLast;       // method pointer of prev. loop

                LONG            lInheritance = 0;   // inheritance counter
                                                    // while we climb up the parents tree

                // allocate structure for returning method information
                PSOMMETHOD psm;
                if (!(psm = (PSOMMETHOD)malloc(sizeof(SOMMETHOD))))
                    break;
                memset(psm, 0, sizeof(SOMMETHOD));

                // copy info from somMethodData
                psm->idMethodName = md.id;
                psm->pszMethodName = strdup(somStringFromId(md.id));    // freed later
                psm->ulType = md.type;
                psm->pMethodProc = pMethodLast = md.method;     // method pointer

                psm->tok = _somGetMethodToken(pClassObject,
                                              md.id);

                // create linked list which will store class objects
                // which override this method; this might be empty
                // later if no class overrides
                lstInit(&psm->llOverriddenBy, FALSE);        // no auto-free
                psm->ulOverriddenBy = 0xFFFF;       // default value for no overrides

                // climb up the parents tree
                pClassObjectLast = pClassObjectThis = pClassObject;
                while (pClassObjectThis)
                {
                    // resolve the method with the current (parent)
                    // class object to check
                    // a)   whether it supports the method -> method introduction
                    // b)   whether method pointers differ from the original
                    //                                     -> method overrides
                    if (_somFindMethod(pClassObjectThis,
                                       md.id,
                                       &pMethodThis))
                    {
                        // this class object does support the method:
                        if (pMethodLast)
                            // compare to the last one we had
                            if (pMethodThis != pMethodLast)
                            {
                                // not equal: this means that the
                                // previous class has overridden the method
                                // --> store current class object in overrider list
                                lstAppendItem(&psm->llOverriddenBy, pClassObjectLast);
                                if (psm->ulOverriddenBy == 0xFFFF)
                                    // store inheritance distance
                                    psm->ulOverriddenBy = lInheritance;
                            }

                        // store items for next loop
                        pMethodLast = pMethodThis;
                        pClassObjectLast = pClassObjectThis;

                        // next higher parent
                        pClassObjectThis = _somGetParent(pClassObjectThis);
                        lInheritance++;
                    }
                    else
                    {
                        // this parent does not support the method
                        // any more: store previous class object as
                        // class which introduced the method
                        psm->pIntroducedBy = pClassObjectLast;
                        break;
                    }
                }

                if (lInheritance > 0)
                {
                    if (psm->pIntroducedBy == NULL)
                        // no class found: use SOMObject then
                        psm->pIntroducedBy = _SOMObject;
                    psm->ulIntroducedBy = lInheritance - 1;
                }
                else
                {
                    // we have a problem here;
                    // this appears to happen for a few
                    // strange class methods
                    psm->ulIntroducedBy = 0xFFFF;
                    psm->pIntroducedBy = 0;
                }

                PMPF_CLASSLIST(("ulInheritance: %d", lInheritance));

                // store all this method info in list
                lstAppendItem(&pmi->llMethods,
                              psm);
            }
        } // end for
    }

    return pmi;
}

/*
 *@@ clsFreeMethodInfo:
 *      this frees the resources allocated by
 *      clsQueryMethods.
 *
 *      ppMethodInfo must be a pointer to the
 *      pointer containing the return value
 *      of clsQueryMethods. This pointer will
 *      be set to NULL by this function.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (99-12-10) [umoeller]: moved this func here from config\clslist.c
 */

VOID clsFreeMethodInfo(PMETHODINFO *ppMethodInfo)
{
    if (    (ppMethodInfo)
         && (*ppMethodInfo)
       )
    {
        PLISTNODE pNode = lstQueryFirstNode(&((*ppMethodInfo)->llMethods));
        while (pNode)
        {
            PSOMMETHOD psm = (PSOMMETHOD)pNode->pItemData;
            if (psm->pszMethodName)
                free(psm->pszMethodName);
            // free list of class objects (no auto-free)
            lstClear(&psm->llOverriddenBy);
            free(psm);
            pNode = pNode->pNext;
        }

        // free SOMMETHOD list
        lstClear(&((*ppMethodInfo)->llMethods));

        free(*ppMethodInfo);
        *ppMethodInfo = NULL;
    }
}


