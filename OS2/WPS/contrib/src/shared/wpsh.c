
/*
 *@@sourcefile wpsh.c:
 *      this file contains SOM and WPS helper functions.
 *
 *      Usage: All WPS classes.
 *
 *      The functions in this file fall into several
 *      categories:
 *
 *      --  Helper functions for messing with SOM method
 *          resolution. See wpshResolveFor and others.
 *
 *      --  Miscellaneous Desktop object helpers. See
 *          wpshCheckObject and others.
 *
 *      --  Miscellaneous object view helpers. See
 *          wpshCloseAllViews, wpshQuerySourceObject, and
 *          others.
 *
 *      --  Wrappers to call some important WPS methods
 *          that were not made public by IBM. See
 *          fdrRequestFolderMutexSem and others.
 *
 *      This file started out with V0.84 as "xwps.c". Most of
 *      these functions used to have the cmn* prefix and were
 *      spread across all over the .C source files, which
 *      was not very lucid. As a result, I have put these
 *      together in a separate file.
 *
 *      The code has been made independent of XWorkplace with V0.9.0,
 *      moved to the shared\ directory, and renamed to "wpsh.c".
 *
 *      All the functions in this file have the wpsh* prefix (changed
 *      with V0.9.0).
 *
 *@@header "shared\wpsh.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich Mîller.
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
 *  3)  SOM headers which work with precompiled header files
 *  4)  headers in /helpers
 *  5)  headers in /main with dlgids.h and common.h first
 *  6)  #pragma hdrstop to prevent VAC++ crashes
 *  7)  other needed SOM headers
 *  8)  for non-SOM-class files: corresponding header (e.g. classlst.h)
 */

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WINSHELLDATA
#define INCL_WINSTDCNR
#define INCL_WININPUT
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"
#include "xfldr.ih"

#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "filesys\folder.h"             // XFolder implementation
#include "filesys\fdrviews.h"           // common code for folder views
#include "filesys\object.h"             // XFldObject implementation

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   SOM helpers
 *
 ********************************************************************/

/*
 *@@ wpshResolveFor:
 *      this resolves a method pointer as implemented by
 *      pClass. If pClass is NULL, the class of somSelf
 *      is used for resolving that method (which leads to
 *      somGetClass(somSelf) internally).
 *
 *      Returns NULL if the method could not be resolved.
 *
 *      "Resolving" means finding the address of a function
 *      as implemented for an object by a certain class.
 *      To call a method with manual resolve, you must first
 *      resolve the method name into a function address and
 *      then call that function indirectly.
 *
 *      This helper is especially useful in the following
 *      situations:
 *
 *      --  You want to call a method, but you don't have
 *          a C binding for it -- either because you don't
 *          want to include the header which defines it,
 *          or you don't have that header in the first
 *          place (e.g. because it's an undocumented WPS
 *          method).
 *
 *          In that case, specify pClass to NULL. This
 *          will have this function find the class object
 *          for somSelf's class itself.
 *
 *      --  You must explicitly resolve a method pointer
 *          for a parent class because you have introduced
 *          a method which is really an override, but there's
 *          no C binding for the parent call. See the example
 *          below.
 *
 *      --  You have manually patched a SOM method table
 *          using _somOverrideSMethod to override a static
 *          WPS method which is undocumented.
 *
 *      Remarks:
 *
 *      1.  This returns a PVOID. You must manually cast
 *          the return pointer to a function prototype
 *          pointer.
 *
 *          For documented WPS methods, you will find that
 *          the SOM header files prototype all functions as
 *
 +              somTD_Classname_Methodname,
 *
 *          e.g. somTD_XFldObject_xwpQuerySetup for xwpQuerySetup
 *          as implemented by XFldObject.
 *
 *          For some other methods, you will find prototypes
 *          in wpsh.h.
 *
 *      2.  Do not pass the underscore with the method name.
 *
 *      3.  This can return NULL for a number of reasons,
 *          e.g. the parent class was not found, or no parent
 *          class implements such a method. Make sure you
 *          spell the method name right.
 *
 *      4.  This works for instance and class methods. To
 *          resolve a class method, specify the class object
 *          with somSelf and set pClass to NULL. This will
 *          then find the class object for the class object...
 *          yes, such a thing exists. The metaclass of a
 *          metaclass is SOMClass (grin). What infinite wisdom
 *          in SOM there.
 *
 *      Example: Assume this class hierarchy:
 *
 +      WPObject
 +        +-- XFldObject
 +              +-- WPFileSystem
 +                    +-- WPFolder
 +                          +-- XFolder
 +                                +-- DemoFolder
 *
 *      Let's say that both XFldObject and DemoFolder introduce
 *      the "wpDemo" method. From DemoFolder, you want to call
 *      the parent implementation (which will lead to XFldObject's
 *      implementation). So do this:
 *
 +          DEMO_TYPEDEF pDemo = wpshResolveFor(somSelf,
 +                                              _XFldObject,
 +                                              "wpDemo");
 +
 +          if (pDemo)      // points to XFldObject::pDemo now
 +              pDemo(somSelf, ...);
 *
 *@@added V0.9.4 (2000-08-02) [umoeller]
 *@@changed V0.9.12 (2001-05-22) [umoeller]: this didn't work for parent classes, fixed
 */

PVOID wpshResolveFor(SOMObject *somSelf,  // in: instance
                     SOMClass *pClass,    // in: class we should resolve for or NULL
                     const char *pcszMethodName) // in: method name (e.g. "wpQueryTitle")
{
    PVOID pvrc = 0;

    somId somidMethod;

    if (!pClass)
        if (!(pClass = _somGetClass(somSelf)))
            return NULL;

    if (somidMethod = somIdFromString((PSZ)pcszMethodName))
    {
        somMToken tok;;
        if (tok = _somGetMethodToken(pClass,
                                     somidMethod))
        {
            // finally, resolve method
            // now using somClassResolve V0.9.12 (2001-05-22) [umoeller]
            if (!(pvrc = (PVOID)somClassResolve(pClass,
                                                tok)))
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "somClassResolved failed for %s", pcszMethodName);
        }
        else
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "Cannot get method token for %s.", pcszMethodName);

        SOMFree(somidMethod);
    }
    else
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Cannot get somId for %s.", pcszMethodName);

    return pvrc;
}

/*
 *@@ wpshParentNumResolve:
 *      similar to wpshParentResolve, but this uses
 *      somParentNumResolve, which should be faster.
 *
 *      With pClass, specify any class object which supports
 *      the specified method.
 *
 *      For example, to resolve WPObject::wpMenuItemSelected,
 *      specify:
 *
 +          wpshParentNumResolve(_WPObject
 *
 *
 *@@added V0.9.6 (2000-10-16) [umoeller]
 */

PVOID wpshParentNumResolve(SOMClass *pClass,    // in: any class object which supports the method
                           somMethodTabs parentMTab, // in: parent method table
                                                     // (e.g. XFldDataFileCClassData.parentMtab)
                           const char *pcszMethodName) // in: method name (e.g. "wpQueryTitle")
{
    PVOID pvrc = 0;

    somId somidMethod;
    if (somidMethod = somIdFromString((PSZ)pcszMethodName))
    {
        // get method token for parent class
        /* SOMClass *pParentClass = _somGetParent(pClass);
                    // we must manually get the parent, because
                    // e.g. _WPDataFile would return _XFldDataFile

        if (pParentClass)
        { */

            // this is not needed, the method token is
            // always the same for parent classes and
            // subclasses V0.9.12 (2001-05-22) [umoeller]

        somMToken tok;
        if (tok = _somGetMethodToken(pClass, somidMethod))
        {
            // somMethodTabs pmt = _somGetPClsMtabs(pParentClass);
            // finally, resolve method for parent
            pvrc = (PVOID)somParentNumResolve(parentMTab,
                                              1,      // first parent
                                              tok);

        }

        SOMFree(somidMethod);
    }

    return pvrc;
}

/*
 *@@ wpshOverrideStaticMethod:
 *      this function manually patches a class's method
 *      table to override a method at runtime.
 *
 *      This is a wrapper around SOMClass::somOverrideSMethod,
 *      which is half-documented in somcls.idl in the toolkit
 *      headers. There we find:
 *
 *          "This method can be used instead of somAddStaticMethod or
 *          somAddDynamicMethod when it is known that the class'
 *          parent class already supports this method.  This call
 *          does not require the method descriptor and stub methods
 *          that the others do."
 *
 *      If you don't understand what this means... I don't
 *      either. I just tested whether this method can be used
 *      to hack a class's method table at runtime, and it
 *      worked.
 *
 *      Even though SOMREF says that use of that method is
 *      deprecated, it still works from my testing. Since
 *      SOM is never going to change any more (because IBM
 *      has stopped development), I think we can safely use
 *      this.
 *
 *      Now, this function is especially useful if you want
 *      to
 *
 *      --  override a method which only exists on Warp 4.
 *          It is an XWorkplace development policy that
 *          XWorkplace should compile with the Warp 3
 *          toolkit headers, so we can't override those
 *          methods directly.
 *
 *      --  override an undocumented WPS method.
 *
 *      To override such a method, pass in the class object
 *      (e.g. _XFldDataFile if you want to override a
 *      method for instances of XFldDataFile), the method
 *      name (without the underscore), and the address
 *      of the function which implements the override.
 *
 *      However, great care must be taken if you patch the
 *      method tables manually, most notably:
 *
 *      --  The method must exist in a parent class of
 *          somSelf. You can't add a new method this way.
 *
 *      --  The method must be a static SOM method. As
 *          far as I know, this is the case for all WPS
 *          methods (the WPS doesn't use dynamic methods).
 *
 *      --  Only patch the method tables when the class
 *          is initialized (best in the class's wpclsInitData).
 *          If you do it later, you're asking for problems.
 *
 *      --  Check the class object for whether it's really
 *          the class object you want... and not that of
 *          a descendant class. Descendant classes will
 *          inherit your method override anyway (tested).
 *
 *      --  The function passed in here must have the
 *          _System calling convention.
 *
 *      --  The function must have EXACTLY the same
 *          arguments as the method you override. SOM
 *          passes the arguments on the stack, and if
 *          they don't match -- boom.
 *
 *      --  You have no C binding for calling the parent
 *          method that you overrode. Use wpshParentNumResolve
 *          to resolve the parent function, which you then
 *          can call.
 *
 *      --  Presently, this has only been tested for
 *          overriding instance methods.
 *
 *      If you don't follow these rules, you get crashes.
 *      Period. So this isn't exactly trivial, but it works.
 *      XWorkplace uses this in M_XFldDataFile::wpclsInitData
 *      to patch in XFldDataFile::wpModifyMenu, for example.
 *
 *      <B>Example:</B>
 *
 +          // prototype of method override; must have
 +          // _System and same arguments as method that
 +          // is overridden
 +
 +          BOOL _System xfdf_wpModifyMenu(XFldDataFile *somSelf,
 +                                         HWND hwndMenu,
 +                                         HWND hwndCnr,
 +                                         ULONG iPosition,
 +                                         ULONG ulMenuType,
 +                                         ULONG ulView,
 +                                         ULONG ulReserved);
 +
 +          // in wpclsInitData, call this:
 +          SOM_Scope void  SOMLINK xfdfM_wpclsInitData(M_XFldDataFile *somSelf)
 +          {
 +              ....
 +
 +              if (somSelf == _XFldDataFile)
 +                  wpshOverrideStaticMethod(somSelf,
 +                                           "wpModifyMenu",
 +                                           (somMethodPtr)xfdf_wpModifyMenu);
 +          }
 +
 *
 *@@added V0.9.7 (2001-01-15) [umoeller]
 */

BOOL wpshOverrideStaticMethod(SOMClass *somSelf,            // in: class object (e.g. _XFldDataFile)
                              const char *pcszMethodName,   // in: method name (without underscore)
                              somMethodPtr pMethodPtr)      // in: new method override implementation
{
    BOOL brc = FALSE;
    somId somidMethod;
    if (!(somidMethod = somIdFromString((PSZ)pcszMethodName))) // V1.0.3 (2005-01-13) [pr]
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Cannot get id for \"%s\".", pcszMethodName);
    else
    {
        // check if the method exists
        somMToken tok;
                // somMToken is typedef'd from somToken,
                // which in turn is typedef'd from void*
                // see sombtype.h
        if (!(tok = _somGetMethodToken(somSelf, somidMethod)))
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "Cannot get method token for \"%s\".", pcszMethodName);
        else
        {
            _somOverrideSMethod(somSelf,
                                somidMethod,
                                pMethodPtr);
                    // duh, no return code here
            brc = TRUE;
        }

        SOMFree(somidMethod);
    }

    return brc;
}

/* ******************************************************************
 *
 *   WPObject helpers
 *
 ********************************************************************/

/*
 *@@ wpshStore:
 *      like strhStore, but uses wpAllocMem for allocating
 *      memory.
 *
 *@@added V0.9.16 (2002-01-26) [umoeller]
 */

APIRET wpshStore(WPObject *somSelf,
                 PSZ *ppszTarget,         // in/out: string pointer
                 PCSZ pcszSource,         // in: string to store
                 PULONG pulLength)        // out: length of new string (ptr can be NULL)
{
    ULONG   ulLength = 0;

    if (!ppszTarget)
        return ERROR_INVALID_PARAMETER;

    if (*ppszTarget)
        _wpFreeMem(somSelf, *ppszTarget);

    if (    (pcszSource)
         && (ulLength = strlen(pcszSource))
       )
    {
        APIRET  arc = NO_ERROR;
        if (*ppszTarget = (PSZ)_wpAllocMem(somSelf,
                                           ulLength + 1,
                                           &arc))
            memcpy(*ppszTarget, pcszSource, ulLength + 1);
        else
            return arc;
    }
    else
        *ppszTarget = NULL;

    if (pulLength)
        *pulLength = ulLength;

    return NO_ERROR;
}

/*
 *@@ wpshCheckObject:
 *      checks pObject for validity.
 *
 *      Since somIsObj doesn't seem to be working right,
 *      here is a new function which checks if pObject
 *      points to a valid Desktop object. This is done by
 *      temporarily installing yet another xcpt handler,
 *      so if the object ain't valid, calling this function
 *      doesn't crash, but returns FALSE only.
 *
 *      Even though the SOM reference says that somIsObj
 *      should be "failsafe", it is not. I've read somewhere
 *      that that function only checks if the object pointer
 *      points to something that looks like a method table,
 *      which doesn't look very failsafe to me, so use this
 *      one instead.
 *
 *      Note: this should not be called very often,
 *      because a SOM method is called upon the given
 *      object, which takes a little bit of time. Also, if
 *      the object is not valid, an exception is generated
 *      internally.
 *
 *      So call this only if you're unsure whether an object
 *      is valid.
 *
 *@@changed V0.9.0 [umoeller]: now using TRY_xxx macros
 *@@changed V0.9.10 (2001-04-10) [pr]: null pObject now returns FALSE
 */

BOOL wpshCheckObject(WPObject *pObject)
{
    BOOL                  brc = FALSE;

    if (pObject)
    {
        TRY_QUIET(excpt1)
        {
            // call an object method; if this doesn't fail,
            // TRUE is returned, otherwise an xcpt occurs
            _wpQueryTitle(pObject);
            brc = TRUE;
        }
        CATCH(excpt1)
        {
            // the thread exception handler puts us here if an exception
            // occurred, i.e. the object was not valid:
            brc = FALSE;

            PMPF_ORDEREDLIST(("Invalid object found."));

        } END_CATCH();
    }

    return brc;
}

/*
 *@@ wpshQueryView:
 *      kinda reverse to wpshQueryFrameFromView, this
 *      returns the OPEN_* flag which represents the
 *      specified view.
 *
 *      For example, pass the hwndFrame of a folder's
 *      Details view to this func, and you'll get
 *      OPEN_DETAILS back.
 *
 *      Returns OPEN_UNKNOWN (-1) upon errors.
 *
 *@@changed V0.9.2 (2000-03-06) [umoeller]: added object mutex protection
 *@@changed V1.0.0 (2002-08-26) [umoeller]: now returning -1 on errors because 0 is OPEN_DEFAULT
 */

ULONG wpshQueryView(WPObject* somSelf,      // in: object to examine
                    HWND hwndFrame)         // in: frame window of open view of somSelf
{
    ULONG   ulView = OPEN_UNKNOWN;      // -1 V1.0.0 (2002-08-26) [umoeller]

    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        if (pobjLock = cmnLockObject(somSelf))
        {
            PUSEITEM    pUseItem = NULL;
            for (pUseItem = _wpFindUseItem(somSelf, USAGE_OPENVIEW, NULL);
                 pUseItem;
                 pUseItem = _wpFindUseItem(somSelf, USAGE_OPENVIEW, pUseItem))
            {
                PVIEWITEM pViewItem = (PVIEWITEM)(pUseItem+1);
                if (pViewItem->handle == hwndFrame)
                {
                    ulView = pViewItem->view;
                    break;
                }
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);

    return ulView;
}

/*
 *@@ wpshIsViewCnr:
 *      returns TRUE if hwndCnr belongs to an
 *      open view of somSelf.
 *
 *      Naturally, this can only return TRUE
 *      if somSelf is a folder. If it is, we
 *      enumerate the open folder views and
 *      check if the container is one of them.
 *
 *      This can be used in wpModifyPopupMenu
 *      to check whether a popup menu has been
 *      requested on the cnr whitespace; in that
 *      case, TRUE should be returned for the
 *      hwndCnr passed with wpModifyPopupMenu.
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 *@@changed V1.0.0 (2002-08-26) [umoeller]: rewritten to be much faster and work with split views too
 */

BOOL wpshIsViewCnr(WPObject *somSelf,
                   HWND hwndCnr)
{
#if 1
    HWND    hwndFrame;

    if (hwndFrame = WinQueryWindow(hwndCnr, QW_PARENT))
    {
        if (    (WPObject*)WinSendMsg(hwndFrame,
                                      WM_QUERYOBJECTPTR,
                                      NULL,
                                      NULL)
             == somSelf
           )
            return TRUE;
    }

    return FALSE;

#else
    BOOL    brc = FALSE;

    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        if (pobjLock = cmnLockObject(somSelf))
        {
            PUSEITEM    pUseItem = NULL;
            for (pUseItem = _wpFindUseItem(somSelf, USAGE_OPENVIEW, NULL);
                 pUseItem;
                 pUseItem = _wpFindUseItem(somSelf, USAGE_OPENVIEW, pUseItem))
            {
                PVIEWITEM pViewItem = (PVIEWITEM)(pUseItem+1);
                if (WinWindowFromID(pViewItem->handle, FID_CLIENT) == hwndCnr)
                {
                    brc = TRUE;
                    break;
                }
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);

    return brc;
#endif
}

/*
 *@@ wpshQuerySourceObject:
 *      this helper function evaluates a given container
 *      to find out which objects have been selected while
 *      a context menu is open. The WPS gives the items on
 *      which the menu action should be performed upon
 *      container "source" emphasis, so this is what we
 *      evaluate.
 *
 *      This is used from XWorkplace to collect objects from
 *      a folder container when an operation is to be
 *      executed and the typical WPS behavior must be imitated.
 *      There are no WPS methods for this. By contrast, the
 *      WPS normally instead calls wpMenuItemSelected for every
 *      single selected object, which is not optimal in many
 *      cases.
 *
 *      This function only works when a context menu is open,
 *      because otherwise WPS cnr items don't have source emphasis.
 *
 *      However, if (fKeyboardMode == TRUE), this function
 *      does not check for source emphasis, but selection
 *      emphasis instead. Only in that case this function
 *      can be used even when no context menu is open on
 *      the container. This is useful for processing hotkeys
 *      on objects, because the WPS makes those work on
 *      selected objects only.
 *
 *      The result of this evaluation is stored in
 *      *pulSelection, which can be:
 *
 *      --   SEL_WHITESPACE the context menu was opened on the
 *                          whitespace of the container;
 *                          this func then returns the folder itself.
 *                          This is only possible if (fKeyboard ==
 *                          FALSE) because keyboard operations
 *                          never affect the currently open folder.
 *
 *      --   SEL_SINGLESEL  the context menu was opened for a
 *                          single selected object (that is,
 *                          exactly one object is selected and
 *                          the menu was opened above that object):
 *                          this func then returns that object.
 *
 *      --   SEL_MULTISEL   the context menu was opened on one
 *                          of a multitude of selected objects;
 *                          this func then returns the first of the
 *                          selected objects. Only in that case,
 *                          keep calling wpshQueryNextSourceObject
 *                          to get the other selected objects.
 *
 *      --   SEL_SINGLEOTHER the context menu was opened for a
 *                          single object _other_ than the selected
 *                          objects:
 *                          this func then returns that object.
 *                          This is only possible if (fKeyboard ==
 *                          FALSE).
 *
 *      --   SEL_NONEATALL: no object is selected. This is only
 *                          possible if (fKeyboard == TRUE); only
 *                          in that case, NULL is returned also.
 *
 *      Note that these flags are defined in include\helpers\cnrh.h,
 *      which should be included.
 *
 *      Keep in mind that if this function returns something other
 *      than the folder of the container (SEL_WHITESPACE), the
 *      returned object might be a shadow, which you might need to
 *      dereference before working on it.
 *
 *@@changed V0.9.1 (2000-01-29) [umoeller]: moved this here from fdrmenus.c; changed prefix
 *@@changed V0.9.1 (2000-01-31) [umoeller]: added fKeyboardMode support
 */

WPObject* wpshQuerySourceObject(WPFolder *somSelf,     // in: folder with open menu
                                HWND hwndCnr,          // in: cnr
                                BOOL fKeyboardMode,    // in: if TRUE, check selected instead of source only
                                PULONG pulSelection)   // out: selection flags
{
    WPObject        *pObject = NULL;

    do
    {
        PMINIRECORDCORE pmrcSource = 0,
                        pmrcSelected = 0;
        if (!fKeyboardMode)
        {
            // not keyboard, but mouse mode:
            // get first object with source emphasis
            if (!(pmrcSource = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                           CM_QUERYRECORDEMPHASIS,
                                                           (MPARAM)CMA_FIRST,
                                                           (MPARAM)CRA_SOURCE)))
            {
                // if CM_QUERYRECORDEMPHASIS returns NULL
                // for source emphasis (CRA_SOUCE),
                // this means the whole container has source
                // emphasis --> context menu on folder whitespace
                pObject = somSelf;   // folder
                *pulSelection = SEL_WHITESPACE;
                // we're done
                break;
            }
            else if ((LONG)pmrcSource == -1)
                // error:
                break;
            // else: we have at least one object with source emphasis
        }

        // get first _selected_ now
        pmrcSelected = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                   CM_QUERYRECORDEMPHASIS,
                                                   (MPARAM)CMA_FIRST,
                                                   (MPARAM)CRA_SELECTED);
        if ((LONG)pmrcSelected == -1)
            // error:
            break;

        if (!fKeyboardMode)
        {
            // not keyboard, but mouse mode:
            // get the object with source emphasis
            // (this is != NULL at this point)
            pObject = OBJECT_FROM_PREC(pmrcSource);

            // check if first source object is equal to
            // first selected object, i.e. the menu was
            // opened on one or several selected objects
            if (pmrcSelected != pmrcSource)
            {
                // no:
                // only one object, but not one of
                // the selected ones
                *pulSelection = SEL_SINGLEOTHER;
                // we're done
                break;
            }
        }
        else
        {
            // keyboard mode:
            if (pmrcSelected)
                pObject = OBJECT_FROM_PREC(pmrcSelected);
            else
            {
                // no selected object: that's no object
                // at all
                *pulSelection = SEL_NONEATALL;
                // we're done
                break;
            }
        }

        // we're still going if
        // a)   we're in menu mode and the first
        //      source object equals the first selected
        //      object or
        // b)   we're in keyboard mode and any
        //      selected object was found.
        // Now, are several objects selected?
        if (pmrcSelected = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                       CM_QUERYRECORDEMPHASIS,
                                                       (MPARAM)pmrcSelected,
                                                            // get second obj
                                                       (MPARAM)CRA_SELECTED))
            // several objects:
            *pulSelection = SEL_MULTISEL;
        else
            // only one object:
            *pulSelection = SEL_SINGLESEL;
    } while (FALSE);

    // note that we have _not_ dereferenced shadows
    // here, because this will lead to confusion for
    // finding other selected objects in the same
    // folder; dereferencing shadows is therefore
    // the responsibility of the caller
    return pObject;       // can be NULL
}

/*
 *@@ wpshQueryNextSourceObject:
 *      if wpshQuerySourceObject above returns SEL_MULTISEL,
 *      you can keep calling this helper func to get the
 *      other objects with source emphasis until this function
 *      returns NULL.
 *
 *      This will return the next object after pObject which
 *      is selected or NULL if it's the last.
 *
 *@@changed V0.9.1 (2000-01-29) [umoeller]: moved this here from fdrmenus.c; changed prefix
 */

WPObject* wpshQueryNextSourceObject(HWND hwndCnr,
                                    WPObject *pObject)
{
    PMINIRECORDCORE pmrcCurrent;
    if (pmrcCurrent = _wpQueryCoreRecord(pObject))
    {
        PMINIRECORDCORE pmrcNext
            = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                          CM_QUERYRECORDEMPHASIS,
                                          (MPARAM)pmrcCurrent,
                                          (MPARAM)CRA_SELECTED);
        if (    (pmrcNext)
             && ((LONG)pmrcNext != -1)
           )
            return OBJECT_FROM_PREC(pmrcNext);
    }

    return NULL;
}

/*
 *@@ wpshCloseAllViews:
 *      closes all views of an object.
 *
 *      For non-folders, this behaves just like wpClose.
 *      For folders however, this closes all views of
 *      the folder itself plus any views of open subfolders
 *      as well.
 *
 *      Returns FALSE on errors.
 *
 *@@added V0.9.4 (2000-06-17) [umoeller]
 */

BOOL wpshCloseAllViews(WPObject *pObject)
{
    BOOL brc;

    if (brc = _wpClose(pObject))
    {
        if (_somIsA(pObject, _WPFolder))
        {
            // it's a folder:
            PLINKLIST pllOpenFolders;
            if (pllOpenFolders = lstCreate(FALSE))
            {
                WPFolder *pOpenFolder;
                PLISTNODE pFolderNode = NULL;
                for (pOpenFolder = _wpclsQueryOpenFolders(_WPFolder, NULL, QC_FIRST, FALSE); // no lock
                     pOpenFolder;
                     pOpenFolder = _wpclsQueryOpenFolders(_WPFolder, pOpenFolder, QC_NEXT, FALSE)) // no lock
                {
                    if (wpshResidesBelow(pOpenFolder, pObject))
                        lstAppendItem(pllOpenFolders, pOpenFolder);
                }

                // OK, now we have a list of open folders:
                pFolderNode = lstQueryFirstNode(pllOpenFolders);
                while (pFolderNode)
                {
                    pOpenFolder = (WPFolder*)pFolderNode->pItemData;
                    if (!(brc = _wpClose(pOpenFolder)))
                        // error:
                        break;

                    pFolderNode = pFolderNode->pNext;
                }

                lstFree(&pllOpenFolders);
            }
            else
                brc = FALSE;
        }
    }

    return brc;
}

/*
 *@@ wpshQueryDraggedObject:
 *      this helper function can be used with wpDragOver
 *      and/or wpDrop to resolve a DRAGITEM to a Desktop object.
 *      This supports both DRM_OBJECT and DRM_OS2FILE
 *      rendering mechanisms.
 *
 *      If (ppObject != NULL), the object is resolved and
 *      written into that pointer as a SOM pointer to the
 *      object.
 *
 *      This does NOT resolve shadows.
 *
 *      Returns:
 *      -- 0: invalid object or mechanism not supported.
 *      -- 1: DRM_OBJECT mechanism, *ppObject is valid.
 *      -- 2: DRM_OS2FILE mechanism, *ppObject is valid.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 */

ULONG wpshQueryDraggedObject(PDRAGITEM pdrgItem,
                             WPObject **ppObjectFound)
{
    ULONG   ulrc = 0;

    // check DRM_OBJECT
    if (DrgVerifyRMF(pdrgItem,
                     "DRM_OBJECT",      // mechanism
                     NULL))             // any format
    {
        // get the object pointer:
        // the WPS stores the MINIRECORDCORE in drgItem.ulItemID
        WPObject *pObject;
        if (pObject = OBJECT_FROM_PREC(pdrgItem->ulItemID))
        {
            ulrc = 1;
            if (ppObjectFound)
                *ppObjectFound = pObject;
        }
    }
    // check DRM_FILE (used by other PM applications)
    else if (DrgVerifyRMF(pdrgItem,
                          "DRM_OS2FILE",       // mechanism
                          NULL))            // any format
    {
        CHAR    szFullFile[2*CCHMAXPATH];
        ULONG   cbFullFile;
        // get source directory; this always ends in "\"
        if (cbFullFile = DrgQueryStrName(pdrgItem->hstrContainerName, // source container
                                         sizeof(szFullFile),
                                         szFullFile))
        {
            // append file name to source directory
            if (DrgQueryStrName(pdrgItem->hstrSourceName,
                                sizeof(szFullFile) - cbFullFile,
                                szFullFile + cbFullFile))
            {
                ulrc = 2;
                if (ppObjectFound)
                    *ppObjectFound = _wpclsQueryObjectFromPath(_WPFileSystem,
                                                               szFullFile);
            }
        }
    }

    return ulrc;
}

/*
 *@@ wpshQueryDraggedObjectCnr:
 *      kinda similar to wpshQueryDraggedObject,
 *      but this handles the CN_DRAGOVER container
 *      notification code instead.
 *
 *      This can be used in any dialog procedure
 *      for PM containers which should accept
 *      objects via drag an drop. When you receive
 *      WM_CONTROL with CN_DRAGOVER, call this
 *      helper as follows:
 *
 +      case CN_DRAGOVER:
 +          HOBJECT hobjBeingDragged = NULLHANDLE;
 +          MRESULT mrc = wpshQueryDraggedObjectCnr((PCNRDRAGINFO)mp2,
 +                                                  &hobjBeingDragged);
 +          return mrc;
 *
 *      If a valid object has been dragged over the cnr,
 *      *phObject will be set to the object handle. Otherwise
 *      it will always be set to NULLHANDLE.
 *
 *      NOTE: This produces an object handle for the object.
 *      Use this func only if you really need a handle.
 *
 *@@added V0.9.3 (2000-04-27) [umoeller]
 */

MRESULT wpshQueryDraggedObjectCnr(PCNRDRAGINFO pcdi,
                                  HOBJECT *phObject)
{
    PDRAGITEM   pdrgItem;
    USHORT      usIndicator = DOR_NODROP,
                    // cannot be dropped, but send
                    // DM_DRAGOVER again
                usOp = DO_UNKNOWN;
                    // target-defined drop operation:
                    // user operation (we don't want
                    // the WPS to copy anything)

    // reset output variable
    *phObject = NULLHANDLE;

    // OK so far:
    // get access to the drag'n'drop structures
    if (DrgAccessDraginfo(pcdi->pDragInfo))
    {
        if (
                // accept no more than one single item at a time;
                // we cannot move more than one file type
                (pcdi->pDragInfo->cditem != 1)
            )
        {
            usIndicator = DOR_NEVERDROP;
        }
        else
        {

            if (    // accept only default drop operation
                    (pcdi->pDragInfo->usOperation == DO_DEFAULT)
                    // get the item being dragged (PDRAGITEM)
                 && (pdrgItem = DrgQueryDragitemPtr(pcdi->pDragInfo, 0))
                    // WPS object?
                 && (DrgVerifyRMF(pdrgItem, "DRM_OBJECT", NULL))
               )
            {
                // the WPS stores the MINIRECORDCORE of the
                // object in ulItemID of the DRAGITEM structure;
                // we use OBJECT_FROM_PREC to get the SOM pointer
                WPObject *pSourceObject
                            = OBJECT_FROM_PREC(pdrgItem->ulItemID);
                if (pSourceObject = _xwpResolveIfLink(pSourceObject))
                {
                    // store object handle to output
                    if (*phObject = _wpQueryHandle(pSourceObject))
                        usIndicator = DOR_DROP;
                }
            }
        }

        DrgFreeDraginfo(pcdi->pDragInfo);
    }

    // and return the drop flags
    return MRFROM2SHORT(usIndicator, usOp);
}

/* ******************************************************************
 *
 *   WPFolder helpers
 *
 ********************************************************************/

#if 0       // disabled V0.9.19 (2002-06-15) [umoeller]

/*
 *@@ wpshPopulateWithShadows:
 *      awakes all shadows in the specified folder.
 *
 *      This runs _wpclsFindObjectFirst and its companions
 *      on the folder... essentially, a sample of this is
 *      in WPREF with that method documentation. In addition
 *      however, we protect the folder contents with mutexes
 *      properly.
 *
 *@@added V0.9.9 (2001-03-12) [umoeller]
 */

BOOL wpshPopulateWithShadows(WPFolder *somSelf)
{
    // OK, the following is from the WPREF docs for
    // wpclsFindObjectFirst... appears to work
    BOOL        brc = FALSE;

    if (    (somSelf)
         && (!(_wpQueryFldrFlags(somSelf) & FOI_POPULATEDWITHALL))
       )
    {
        M_WPObject  *pWPObject = _WPObject;
        BOOL        fFindSem = FALSE,
                    fFolderSem = FALSE;
        HFIND       hFind = 0;          // find handle

        TRY_LOUD(excpt1)
        {
            // request the find mutex... we are awaking objects here
            if (fFindSem = !fdrRequestFindMutexSem(somSelf, SEM_INDEFINITE_WAIT))
            {
                if (fFolderSem = !fdrRequestFolderMutexSem(somSelf, SEM_INDEFINITE_WAIT))
                {
                    CLASS       aClasses[2];        // array of classes to look for
                    OBJECT      aObjects[100];      // buffer for returned objects
                    ULONG       ulCount,
                                ulErrorID;
                                                    // objects count

                    // set "populate" flag
                    _wpModifyFldrFlags(somSelf,
                                       FOI_POPULATEINPROGRESS,
                                       FOI_POPULATEINPROGRESS);

                    // here's the trick how to awake all shadows:
                    // run wpclsFindObjectFirst with the WPShadow class object
                    // on the folder...
                    aClasses[0] = _WPShadow;
                    aClasses[1] = NULL;         // list terminator

                    // reset the error indicators
                    _wpclsSetError(pWPObject, 0);

                    brc = FALSE;
                    // attempt to find the first stack of objects into
                    // the buffer
                    ulCount = ARRAYITEMCOUNT(aObjects);
                    brc = _wpclsFindObjectFirst(pWPObject,
                                                aClasses,    // classes to find
                                                &hFind,      // out: handle
                                                (PSZ)NULL,   // we don't care about the titled
                                                somSelf,     // folder to search
                                                FALSE,       // no recurse into subfolders
                                                NULL,        // extended criteria (?!?)
                                                aObjects,    // out: objects found
                                                &ulCount);   // in: size of array;
                                                             // out: object count found
                            // all objs are locked!!

                    ulErrorID = _wpclsQueryError(pWPObject);

                    while (    (!brc)
                            && (ulErrorID == WPERR_BUFFER_OVERFLOW)
                          )
                    {
                        // buffer wasn't large enough:
                        // get next set
                        _wpclsSetError(pWPObject, 0);
                        ulCount = ARRAYITEMCOUNT(aObjects);
                        brc = _wpclsFindObjectNext(pWPObject,
                                                   hFind,
                                                   aObjects,
                                                   &ulCount );
                        ulErrorID = _wpclsQueryError(pWPObject);
                    }

                    // OK, now we got all shadows in the folder too.
                } // end if (fFolderSem)
            } // end if (fFindSem)
        }
        CATCH(excpt1)
        {
            brc = FALSE;
        } END_CATCH();

        // release mutexes in reverse order
        if (fFolderSem)
            fdrReleaseFolderMutexSem(somSelf);
        if (fFindSem)
            fdrReleaseFindMutexSem(somSelf);

        // clean up
        _wpclsSetError(pWPObject, 0);

        if (hFind)
            _wpclsFindObjectEnd(pWPObject, hFind);

        // unset "populate" flag again
        _wpModifyFldrFlags(somSelf,
                           FOI_POPULATEINPROGRESS,
                           0);
    }

    return brc;
}

#endif

/*
 *@@ wpshResidesBelow:
 *      returns TRUE if pChild resides either in
 *      or somewhere below the folder hierarchy
 *      of pFolder.
 */

BOOL wpshResidesBelow(WPObject *pChild,
                      WPFolder *pFolder)
{
    BOOL        rc = FALSE;
    WPObject    *pObj;
    if ( (pFolder) && (pChild))
    {
        pObj = pChild;
        while (pObj)
        {
            if (pFolder == pObj)
            {
                rc = TRUE;
                break;
            }
            else
                pObj = _wpQueryFolder(pObj);
        }
    }

    return rc;
}

/*
 *@@ wpshContainsFile:
 *      this returns a file-system object if the folder contains the
 *      file pszRealName.
 *
 *      This does _not_ use wpPopulate, but DosFindFirst to find the
 *      file, including a subfolder of the same name. Abstract objects
 *      are not found. If such an object does not exist, NULL is
 *      returned.
 */

WPFileSystem* wpshContainsFile(WPFolder *pFolder,   // in: folder to examine
                               const char *pcszRealName)     // in: file-name (w/out path)
{
    CHAR        szRealName[2*CCHMAXPATH];
    ULONG       cbRealName = sizeof(szRealName);
    WPObject    *prc = NULL;

    if (_wpQueryRealName(pFolder, szRealName, &cbRealName, TRUE))
    {
        HDIR          hdirFindHandle = HDIR_SYSTEM;
        FILEFINDBUF3  FindBuffer     = {0};      // Returned from FindFirst/Next
        ULONG         ulResultBufLen = sizeof(FILEFINDBUF3);
        ULONG         ulFindCount    = 1;        // Look for 1 file at a time
        APIRET        rc             = NO_ERROR; // Return code

        szRealName[cbRealName++] =  '\\';
        strlcpy(szRealName + cbRealName,
                pcszRealName,
                sizeof(szRealName) - cbRealName);

        rc = DosFindFirst(szRealName,
                          &hdirFindHandle,
                          FILE_DIRECTORY
                              | FILE_SYSTEM
                              | FILE_ARCHIVED
                              | FILE_HIDDEN
                              | FILE_READONLY,
                          // FILE_NORMAL,
                          &FindBuffer,
                          ulResultBufLen,
                          &ulFindCount,
                          FIL_STANDARD);
        DosFindClose(hdirFindHandle);
        if (rc == NO_ERROR)
            prc = _wpclsQueryObjectFromPath(_WPFileSystem, szRealName);
    }

    return prc;
}

/*
 *@@ wpshCreateFromTemplate:
 *      enhanced wpCreateFromTemplate, which can automatically
 *      make the title of the new object editable and reposition
 *      the newly created object to the mouse position.
 *      This is _not_ a replacement of wpCreateFromTemplate and
 *      _not_ a SOM method, but only used when XFolder creates objects.
 *      This returns the new object.
 *
 *@@changed V0.9.0 [umoeller]: changed function prototype to be XWorkplace-independent
 *@@changed V0.9.2 (2000-02-26) [umoeller]: removed mutex semaphores
 *@@changed V0.9.2 (2000-02-26) [umoeller]: removed CM_QUERYRECORDINFO
 *@@changed V0.9.9 (2001-03-27) [umoeller]: now using wpCopyObject instead of wpCreateFromTemplate; this should fix the system hangs
 *@@changed V0.9.14 (2001-07-28) [umoeller]: fixed invisible new obj in tree views (workaround for WPS bug)
 *@@changed V1.0.0 (2002-08-26) [umoeller]: removed hab param which was never used; optimized
 *@@changed V1.0.0 (2002-08-26) [umoeller]: fixed tree view, finally
 *@@changed V1.0.1 (2002-12-15) [umoeller]: fixed tree view for disk objects @@fixes 284
 */

WPObject* wpshCreateFromTemplate(WPObject *pTemplate,
                                    // in: the template to create from
                                 WPFolder* pFolder,
                                    // in: the target folder to create the new object in
                                 HWND hwndFrame,
                                    // in: the frame wnd in which the object
                                    // should be manipulated/repositioned
                                 USHORT usOpenSettings,
                                    // 0: do nothing after creation
                                    // 1: open settings notebook
                                    // 2: make title editable
                                 BOOL fReposition,
                                    // in: if TRUE, the new object will be repositioned
                                 POINTL* pptlMenuMousePos)
                                    // in: for Icon views: position to create
                                    // object at or NULL for default pos
{
    WPObject            *pNewObject = NULL;
    HPOINTER            hptrOld;
    BOOL                fChangeIconPos = FALSE,
                        fShiftPressed = doshQueryShiftState();

    TRY_LOUD(excpt1)
    {
        HWND    hwndCnr;

        // change the mouse pointer to "wait" state
        hptrOld = winhSetWaitPointer();

        if (    (pFolder)
             && (pTemplate)
             && (hwndFrame)
             && (hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT))
           )
        {
            CNRINFO         CnrInfo;
            POINTL          ptlMoveTo;

            PMPF_MENUS(("[%s] entering", _wpQueryTitle(pTemplate)));

            cnrhQueryCnrInfo(hwndCnr, &CnrInfo);

            // position newly created object in window?
            if (    (fReposition)
                 && (pptlMenuMousePos)
               )
            {
                // only do this in "true" Icon views (not Tree, not Name, not Text)
                if (    ((CnrInfo.flWindowAttr & (CV_ICON | CV_TREE)) == CV_ICON)
                        // and only if "Always sort" is off
                     && (CnrInfo.pSortRecord == NULL)
                   )
                {
                    ptlMoveTo.x = pptlMenuMousePos->x;
                    ptlMoveTo.y = pptlMenuMousePos->y;

                    WinMapWindowPoints(HWND_DESKTOP,
                                       hwndCnr,
                                       &ptlMoveTo,
                                       1);

                    /*  replaced this V1.0.0 (2002-08-26) [umoeller]
                    // the passed mouse coords are relative to screen,
                    // so subtract frame wnd coords
                    WinQueryWindowPos(hwndFrame, &swp);
                    pptlMenuMousePos->x -= swp.x;
                    pptlMenuMousePos->y -= swp.y;

                    // subtract cnr coords also
                    WinQueryWindowPos(hwndCnr, &swp);
                    pptlMenuMousePos->x -= swp.x;
                    pptlMenuMousePos->y -= swp.y;
                    */

                    // add cnr work area offset
                    ptlMoveTo.x += CnrInfo.ptlOrigin.x;
                    ptlMoveTo.y += CnrInfo.ptlOrigin.y;

                    fChangeIconPos = TRUE;
                }
            } // end if ((hwndCnr) && (fReposition) && (pptlMenuMousePos))

            // use wpCopyObject instead of wpCreateFromTemplate,
            // which can hang the system badly (swapper growing)...
            // from my testing, it looks as though wpCreateFromTemplate
            // enters a critical section somewhere, no wonder this messes
            // up everything

            // So just copy the object and remove OBJSTYLE_TEMPLATE later...
            if (pNewObject = _wpCopyObject(pTemplate,
                                           pFolder,
                                           TRUE))       // lock
            {
                PMINIRECORDCORE pmrc,
                                precParent = NULL;
                BOOL            fInsert = TRUE;

                _wpModifyStyle(pNewObject,
                               OBJSTYLE_TEMPLATE,
                               0);

                // now check if the object is already inserted in
                // the container; this is the case usually for
                // icon and details views, but we have a problem
                // with tree views (because templates are not inserted
                // and the tree view does not pick up the style change)
                // and split views as well. So check if the cnr has
                // the object inserted:

                if (CnrInfo.flWindowAttr & CV_TREE)
                {
                    // insert only if the new object is a folder,
                    // or if the folder has "show all in tree view"
                    // set

                    if (fInsert =    (_somIsA(pNewObject, _WPFolder))
                                  || (fdrHasShowAllInTreeView(pFolder))
                       )
                    {
                        WPObject    *pobjUnder;
                        // if we're in tree view, insert the new
                        // object under the folder where we just
                        // created the thing

                        // but check if this is a root folder...
                        // in that case, not the root folder's
                        // record is inserted, but the one of the
                        // disk V1.0.1 (2002-12-15) [umoeller]
                        if (ctsIsRootFolder(pFolder))
                            pobjUnder = _wpQueryDisk(pFolder);
                        else
                        {
                            // populate the folder (synchronously)
                            fdrCheckIfPopulated(pFolder,
                                                TRUE);      // folders only

                            pobjUnder = pFolder;
                        }

                        precParent = _wpQueryCoreRecord(pobjUnder);

                        WinSendMsg(hwndCnr,
                                   CM_EXPANDTREE,
                                   (MPARAM)precParent,
                                   0);
                    }
                }

                if (fInsert)
                {
                    if (!fdrvIsObjectInCnr(pNewObject,
                                           hwndCnr))
                        // object still not in there: insert it then
                        _wpCnrInsertObject(pNewObject,
                                           hwndCnr,
                                           &ptlMoveTo,
                                           precParent,  // NULL if not tree view
                                           NULL);

                    if (pmrc = _wpQueryCoreRecord(pNewObject))
                    {
                        // move new object to mouse pos, if allowed;
                        // we must do this "manually" by manipulating the
                        // cnr itself, because the WPS methods for setting
                        // icon positions simply don't work (I think this
                        // broke with Warp 3)

                        if (fChangeIconPos)       // valid-data flag set above
                        {
                            // the WPS shares records among views, so we need
                            // to update the record core info first
                            WinSendMsg(hwndCnr,
                                       CM_QUERYRECORDINFO,
                                       (MPARAM)&pmrc,
                                       (MPARAM)1);         // one record only

                            // un-display the new object at the old (default) location
                            WinSendMsg(hwndCnr,
                                       CM_ERASERECORD,
                                           // this only changes the visibility of the
                                           // record without changing the recordcore;
                                           // this msg is intended for drag'n'drop and such
                                       (MPARAM)pmrc,
                                       NULL);

                            // move object
                            pmrc->ptlIcon.x = ptlMoveTo.x;
                            pmrc->ptlIcon.y = ptlMoveTo.y;

                            // repaint at new position
                            WinSendMsg(hwndCnr,
                                       CM_INVALIDATERECORD,
                                       (MPARAM)&pmrc,
                                       MPFROM2SHORT(1,     // one record only
                                                    CMA_REPOSITION | CMA_ERASE));
                        }

                        // scroll cnr work area to make the new object visible
                        cnrhScrollToRecord(hwndCnr,
                                           (PRECORDCORE)pmrc,
                                           CMA_TEXT,
                                           FALSE);

                        // the object is now created; depending on the
                        // Global settings, we will now either open
                        // the settings notebook of it or make its title
                        // editable

                        if (    (usOpenSettings == 1)
                             || (fShiftPressed)
                           )
                        {
                            // open settings of the newly created object
                            _wpViewObject(pNewObject,
                                          NULLHANDLE,
                                          OPEN_SETTINGS,
                                          0L);
                        }
                        else if (usOpenSettings == 2)
                        {
                            // make the title of the newly created object
                            // editable (container "direct editing"), if
                            // the settings allow it and the folder is open
                            BOOL            fStartEditing = TRUE;
                            CNREDITDATA     CnrEditData = {0};

                            // the WPS shares records among views, so we need
                            // to update the record core info first
                            WinSendMsg(hwndCnr,
                                       CM_QUERYRECORDINFO,
                                       (MPARAM)&pmrc,
                                       (MPARAM)1);         // one record only

                            // first check if the folder window whose
                            // context menu was used is in Details view
                            // or other
                            if (CnrInfo.flWindowAttr & CV_DETAIL)
                            {
                                PFIELDINFO      pFieldInfo = 0;

                                // Details view: now, this is wicked; we
                                // need to find out the "Title" column of
                                // the container which we need to pass to
                                // the container for enabling direct editing
                                pFieldInfo = (PFIELDINFO)WinSendMsg(hwndCnr,
                                                                    CM_QUERYDETAILFIELDINFO,
                                                                    MPNULL,
                                                                    (MPARAM)CMA_FIRST);

                                // pFieldInfo now points to the first Details
                                // column; now we go through all the Details
                                // columns until we find one which is not
                                // read-only (which should be the title); we
                                // cannot assume "column two" or anything like
                                // this, because this folder might have
                                // Details settings which are different from
                                // the defaults
                                while ((pFieldInfo) && ((LONG)pFieldInfo != -1))
                                {
                                    if (pFieldInfo->flData & CFA_FIREADONLY)
                                        break; // while

                                    // else get next column
                                    pFieldInfo = (PFIELDINFO)WinSendMsg(hwndCnr,
                                                                        CM_QUERYDETAILFIELDINFO,
                                                                        pFieldInfo,
                                                                        (MPARAM)CMA_NEXT);
                                }

                                if (pFieldInfo)
                                {
                                    // found:
                                    // in Details view, direct editing needs the
                                    // column info plus a fixed constant
                                    CnrEditData.pFieldInfo = pFieldInfo;
                                    CnrEditData.id = CID_LEFTDVWND;
                                }
                                else
                                    fStartEditing = FALSE;
                            }
                            else
                            {
                                // other than Details view: that's easy,
                                // we only need the container ID
                                CnrEditData.pFieldInfo = NULL;
                                CnrEditData.id = WinQueryWindowUShort(hwndCnr, QWS_ID);
                            }

                            if (fStartEditing)
                            {
                                CnrEditData.cb = sizeof(CnrEditData);
                                CnrEditData.hwndCnr = hwndCnr;
                                // pass the MINIRECORDCORE of the new object
                                CnrEditData.pRecord = (PRECORDCORE)pmrc;
                                // use existing (template default) title
                                CnrEditData.ppszText = NULL;
                                CnrEditData.cbText = 0;

                                // finally, this message switches to
                                // direct editing of the title
                                WinSendMsg(hwndCnr,
                                           CM_OPENEDIT,
                                           (MPARAM)&CnrEditData,
                                           MPNULL);
                                // V1.0.4 (2005-02-23) [pr]: Select all text
                                WinSendDlgItemMsg(hwndCnr,
                                                  CID_MLE,
                                                  WM_CHAR,
                                                  MPFROM2SHORT(KC_VIRTUALKEY | KC_SHIFT,0),
                                                  MPFROM2SHORT(0, VK_END));
                            }
                        } // end else if (usOpenSettings == 2)
                    }
                } // end if (fInsert)
            } // end if (pNewObject);
        } // end if ((pFolder) && (pTemplate) && (hwndFrame))

        // after all this, reset the mouse pointer
        WinSetPointer(HWND_DESKTOP, hptrOld);
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    return pNewObject;
}

/*
 *@@ wpshQueryFrameFromView:
 *       this routine gets the frame window handle of the
 *       specified object view (OPEN_* flag, e.g. OPEN_CONTENTS
 *       or OPEN_SETTINGS), as found in the view items list
 *       of somSelf (wpFindViewItem).
 *
 *       Returns NULLHANDLE if the specified view is not
 *       currently open.
 *
 *@@changed V1.0.0 (2002-08-28) [umoeller]: optimized
 */

HWND wpshQueryFrameFromView(WPFolder *somSelf,  // in: folder to examine
                            ULONG ulView)       // in: OPEN_CONTENTS etc.
{
    BOOL    fLocked = FALSE;
    HWND    hwndFrame = 0;

    TRY_LOUD(excpt1)
    {
        // go search the open views of the folder and get the
        // frame handle of the desired view (ulView)

        /*  replaced V1.0.0 (2002-08-28) [umoeller]
        for (pViewItem = _wpFindViewItem(somSelf, VIEW_ANY, NULL);
             pViewItem;
             pViewItem = _wpFindViewItem(somSelf, VIEW_ANY, pViewItem)) */

        if (fLocked = !_wpRequestObjectMutexSem(somSelf, SEM_INDEFINITE_WAIT))
        {
            PUSEITEM pui;
            for (pui = _wpFindUseItem(somSelf, USAGE_OPENVIEW, NULL);
                 pui;
                 pui = _wpFindUseItem(somSelf, USAGE_OPENVIEW, pui))
            {
                PVIEWITEM pvi = (PVIEWITEM)(pui + 1);

                if (pvi->view == ulView)
                {
                     hwndFrame = pvi->handle;
                     break;     // V1.0.0 (2002-08-28) [umoeller]
                }
            } // end for
        }
    }
    CATCH(excpt1)
    {
        hwndFrame = NULLHANDLE;
    } END_CATCH();

    if (fLocked)
        _wpReleaseObjectMutexSem(somSelf);

    return hwndFrame;
}

/*
 *@@ wpshQueryLogicalDisk:
 *      as opposed to wpQueryDisk, of which I really don't
 *      know what it returns, this returns the logical drive
 *      number (1 = A, 2 = B, etc.) on which the WPObject
 *      resides. This works also for objects which are not
 *      file-system based; for these, their folder is examined
 *      instead.
 *
 *      Note that if the object is a file-system object on
 *      a remote drive (and thus returns a UNC name), 0
 *      is returned because then there's no logical disk.
 *
 *@@changed V0.9.16 (2001-10-04) [umoeller]: fixed remote objects
 */

ULONG wpshQueryLogicalDisk(WPObject *somSelf)
{
    if (somSelf)
    {
        WPFileSystem    *pFSObj = NULL;
        CHAR            szRealName[CCHMAXPATH];

        if (!_somIsA(somSelf, _WPFileSystem))
            pFSObj = _wpQueryFolder(somSelf);
        else
            pFSObj = somSelf;

        if (    (pFSObj)
             && (_wpQueryFilename(pFSObj, szRealName, TRUE))
             // rule out UNC names
             && (szRealName[1] == ':')      // V0.9.16 (2001-10-04) [umoeller]
           )
        {
            return (szRealName[0] - 'A' + 1); // = 1 for "A", 2 for "B" etc.
        }
    }

    return 0;
}

/*
 *@@ wpshQueryDiskFreeFromFolder:
 *      returns the free space on the drive where a
 *      given folder resides (in bytes).
 *
 *@@changed V0.9.0 [umoeller]: fixed another > 4 GB bug (thanks to RÅdiger Ihle)
 *@@changed V0.9.16 (2001-10-02) [umoeller]: rewritten
 */

double wpshQueryDiskFreeFromFolder(WPFolder *somSelf)
{
    ULONG       ulDisk;
    double      dFree;

    if (    (ulDisk = wpshQueryLogicalDisk(somSelf))
         && (!doshQueryDiskFree(ulDisk, &dFree))
       )
    {
        return dFree;
    }

    return 0;
}

/*
 *@@ wpshQueryDiskSizeFromFolder:
 *      returns the total size of the drive where a
 *      given folder resides (in bytes).
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 *@@changed V0.9.16 (2001-10-02) [umoeller]: rewritten
 */

double wpshQueryDiskSizeFromFolder(WPFolder *somSelf)
{
    ULONG       ulDisk;
    double      dSize;

    if (    (ulDisk = wpshQueryLogicalDisk(somSelf))
         && (!doshQueryDiskSize(ulDisk, &dSize))
       )
    {
        return dSize;
    }

    return 0;
}

/* ******************************************************************
 *
 *   Method call helpers
 *
 ********************************************************************/

/*
 * wpshParentQuerySetup2:
 *      little helper for calling the parent
 *      xwpQuerySetup2 method. Standard code
 *      required for every xwpQuerySetup2
 *      override.
 *
 *added V0.9.16 (2001-10-19) [umoeller]
 *removed V1.0.1 (2002-12-08) [umoeller]
 */

/*

BOOL wpshParentQuerySetup2(WPObject *somSelf,       // in: object
                           SOMClass *pClass,        // in: parent class
                           PVOID pstrSetup)         // in: setup string buffer
{
    somTD_XFldObject_xwpQuerySetup2 pxwpQuerySetup2;

    if (pxwpQuerySetup2 = (somTD_XFldObject_xwpQuerySetup2)wpshResolveFor(
                                                     somSelf,
                                                     pClass,
                                                     "xwpQuerySetup2"))
    {
        return (pxwpQuerySetup2(somSelf, pstrSetup));
    }

    return FALSE;
}

*/
