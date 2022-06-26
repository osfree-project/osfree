
/*
 *@@sourcefile classlst.h:
 *      header file for classlst.c, which contains the logic
 *      for displaying the WPS class tree in a container.
 *
 *      Most of this file was called "objdlgs.h" in V0.80.
 *
 *      In order to use this, you need to do the following:
 *
 *      1) #include classlst.h, but make sure you
 *         have #include'd os2.h before
 *
 *      2) set up the SELECTCLASSDATA structure below
 *         and pass this to WinDlgBox. Example:
 *
 *             SELECTCLASSDATA scd;
 *             strcpy(scd.szDlgTitle, "Select Class");
 *             strcpy(scd.szIntroText, "Select the class for which you want to change "
 *                                   "the status bar info.");
 *             strcpy(scd.szRootClass, "WPObject");
 *             strcpy(scd.szClassSelected, "WPObject");
 *             scd.ulHelpPanel = 0;
 *
 *             WinDlgBox(HWND_DESKTOP, hwndDlg,
 *                     fnwpSelectStatusBarClass,
 *                     NLS_MODULE, ID_XSD_SELECTCLASS,  // resources
 *                     &scd);
 *
 *      This also contains the logic for the "Internals"
 *      page in each object's settings notebook.
 *
 *@@include #define INCL_WINSTDCNR
 *@@include #define INCL_WINWORKPLACE
 *@@include #include <os2.h>
 *@@include #include <wpobject.h>
 *@@include #include "helpers\linklist.h"
 *@@include #include "shared\notebook.h"   // for notebook callback prototypes
 *@@include #include "setup\classlst.h"
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

#ifndef CLASSES_HEADER_INCLUDED
    #define CLASSES_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   "Classes to container" functions
     *
     ********************************************************************/

    /*
     *@@ WPSLISTITEM:
     *      WPS class data linked list item;
     *      one of these is created for each
     *      installed WPS class on the system
     *      and stored in an internal linked list.
     *      Also, the CLASSRECORDCORE structure
     *      has an additional pointer to access this.
     */

    typedef struct _WPSLISTITEM
    {
        struct _WPSLISTITEM *pParent;
                            // pointer to parent class definition
                            // or NULL if this is the root item
                            // V0.9.15 (2001-09-14) [umoeller]

        SOMClass            *pClassObject;
                            // pointer to class object (e.g. _WPFolder)
        SOMClass            *pParentClassObject;
                            // pointer to parent class object (e.g. _WPFileSystem)
        PSZ                 pszClassName;
                            // class name as returned by WinEnumObjectClasses
        PSZ                 pszModName;
                            // DLL name as returned by WinEnumObjectClasses
        PSZ                 pszReplacedWithClasses;
                            // points to a string list of classes which replace this
                            // class, if applicable, or NULL
        PSZ                 pszReplacesClass;
                            // points to the classname which this class replaces,
                            // if applicable, or NULL
        CHAR                szParentClass[256];
                            // contains parent class
        PRECORDCORE         pRecord;
                            // corresponding PCLASSRECORDCORE
        BOOL                fProcessed;
                            // anti-recursion flag; internal use only
        // Note: All PSZ's point into the POBJCLASS linked list in the
        // WPSCLASSESINFO structure returned by clsWpsClasses2Cnr.
    } WPSLISTITEM, *PWPSLISTITEM;

    /*
     *@@ CLASSRECORDCORE:
     *      extended RECORDCORE structure for WPS classes
     */

    typedef struct _CLASSRECORDCORE
    {
        RECORDCORE     recc;        // regular record core
        PWPSLISTITEM    pwps;       // pointer to additional data in internal linked list
    } CLASSRECORDCORE, *PCLASSRECORDCORE;

    #ifdef LINKLIST_HEADER_INCLUDED
        /*
         *@@ WPSCLASSESINFO:
         *      this gets returned by clsWpsClasses2Cnr
         *      and must be passed to clsCleanupWpsClasses
         */

        typedef struct _WPSCLASSESINFO
        {
            PLINKLIST       pllClassList;   // linked list of WPSLISTITEM's
            POBJCLASS       pObjClass;      // class list returned by WinEnumObjectClasses
        } WPSCLASSESINFO, *PWPSCLASSESINFO;

        /*
         *@@ SELECTCLASSDATA:
         *      input structure for configuring the cls*
         *      functions
         */

        typedef struct _SELECTCLASSDATA
        {
            PCSZ    pszDlgTitle;            // dialog title;
                                            // only used with clsSelectWpsClassDlg
            PCSZ    pszIntroText;           // intro text above container
                                            // only used with clsSelectWpsClassDlg
            PCSZ    pcszRootClass;          // class to begin with
            PCSZ    pcszOrphans;            // title of "Orphans" tree; if NULL,
                                            // orphaned classes will not be shown
            CHAR    szClassSelected[256];   // in: initially select this class
                                            // (always used)
                                            // out: class selected by user
                                            // (if used with clsSelectWpsClassDlg)
            PFNWP   pfnwpReturnAttrForClass;
                    // this callback func gets called just before a
                    // record core gets inserted into the cnr
                    // by clsAddClass2Cnr.
                    // This field is always used.
                    // Params passed to this callback func are:
                    //      HWND hwnd    container wnd handle
                    //      ULONG PSELECTCLASSDATA pscd
                    //                   pointer to this structure; you can
                    //                   read the fields at the bottom to
                    //                   get more info
                    //      PWPSLISTITEM mp1  structure with class info
                    //      PCLASSRECORDCORE mp2  _parent_ record core into which the
                    //                        new recc will be inserted; might be NULL
                    // This func must return the CRA_* flags for the new
                    // record core which will then be inserted.
                    // If this callback is not set (== NULL),
                    // a default of CRA_RECORDREADONLY will be used.

            PFNWP   pfnwpClassSelected;
                    // this callback func gets called when a a class is selected in the dlg.
                    // This field is only used with clsSelectWpsClassDlg.
                    // Params passed to this callback func are:
                    //      HWND hwnd    container wnd handle
                    //      ULONG msg    user-defined ulUserClassSelected below
                    //      PWPSLISTITEM mp1  structure with info about the selected class
                    //      MPARAM mp2   hwnd of info static text control below cnr
                    // If this func returns TRUE, the "OK" button will be enabled.
            ULONG   ulUserClassSelected;

            const char* pszHelpLibrary;         // help library
            ULONG   ulHelpPanel;            // help panel; if 0, the "Help"
                                            // button is disabled

            // all the following are for internal use,
            // you don't have to set these; you can
            // however read these in the callbacks
            HWND    hwndCnr;
            PWPSCLASSESINFO pwpsc;
            PCLASSRECORDCORE preccSelection,    // current "selection" emphasis
                             preccSource;       // current "source" emphasis
            BOOL    fSelectionValid;
            LONG    lBackground;
            LONG    lText;
            LONG    lStaticText;
            LONG    lSelBackground;
            LONG    lSelText;
            LONG    lRecurse;
            PRECORDCORE preccExpanded;          // for tree-view auto scroll
        } SELECTCLASSDATA, *PSELECTCLASSDATA;

        PWPSCLASSESINFO clsWpsClasses2Cnr(HWND hwndCnr,
                                          PCSZ pcszRootClass,
                                          PSELECTCLASSDATA pscd);

        VOID clsCleanupWpsClasses(PWPSCLASSESINFO pwpsd);

    #endif

    /* ******************************************************************
     *
     *   "Select class" dialog
     *
     ********************************************************************/

    #ifdef LINKLIST_HEADER_INCLUDED
        ULONG clsSelectWpsClassDlg(HWND hwndOwner,
                                   HMODULE hmod, ULONG idDlg,
                                   PSELECTCLASSDATA pscd);
    #endif

    /* ******************************************************************
     *
     *   Method information functions
     *
     ********************************************************************/

    #ifdef LINKLIST_HEADER_INCLUDED
        /*
         *@@ SOMMETHOD:
         *      contains information for a certain SOM
         *      method. These structures are contained
         *      in a linked list in METHODINFO (below).
         *
         *@@changed V0.9.6 (2000-10-16) [umoeller]: added token
         */

        typedef struct _SOMMETHOD
        {
            // information returned by somGetNthMethodData:
            somId       idMethodName;           // somId of method name
            PSZ         pszMethodName;          // method name (strdup of resolved somId; freed OK)
            ULONG       ulType;                 // 0=static, 1=dynamic, 2=nonstatic.
                                                // WPS methods are all static, I think.
            somMethodProc *pMethodProc;         // pointer to resolved method procedure
            somMToken   tok;                    // method token

            // interpreted data:
            SOMClass*   pIntroducedBy;          // class object of class which introduced the method.
                                                // This is always valid and might be _SOMObject.
                                                // Use _somGetName to get the class name.
            ULONG       ulIntroducedBy;         // inheritance distance to the class which
                                                // introduced this method. If the current class
                                                // introduced this, this is 0. If the parent introduced
                                                // this, this is 1, and so forth. Useful for sorting by
                                                // inheritance.
            LINKLIST    llOverriddenBy;         // linked list of class objects which override this method.
                                                // Each list item is a SOMClass*.
                                                // The list always exists, but might be empty.
            ULONG       ulOverriddenBy;         // inheritance distance to first class on pllOverriddenBy.
                                                // This is similar to ulIntroducedBy. If pllOverriddenBy
                                                // is empty, this is set to 0xFFFF.
        } SOMMETHOD, *PSOMMETHOD;

        /*
         *@@ METHODINFO:
         *      structure returned by clsQueryMethodInfo
         *      for a given SOM class object.
         *      This has a linked list of SOMMETHOD structures.
         */

        typedef struct _METHODINFO
        {
            ULONG       ulMethodCount;
            LINKLIST    llMethods;     // linked list of SOMMETHOD structures
        } METHODINFO, *PMETHODINFO;

        PMETHODINFO clsQueryMethodInfo(SOMClass *pClassObject,
                                       BOOL fClassMethods,
                                       PBOOL pfAbort);

        VOID clsFreeMethodInfo(PMETHODINFO *ppMethodInfo);
    #endif

#endif

