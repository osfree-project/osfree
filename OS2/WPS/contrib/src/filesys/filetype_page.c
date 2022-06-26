
/*
 *@@sourcefile filetype.c:
 *      extended file types notebook code. This has
 *      the method implementations for XFldDataFile.
 *
 *      Function prefix for this file:
 *      --  ftyp*
 *
 *@@added V0.9.0 [umoeller]
 *@@header "filesys\filetype.h"
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

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT           // WM_CHAR
#define INCL_WINPOINTERS
#define INCL_WINPROGRAMLIST     // needed for wppgm.h
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINSTDDRAG
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xstring.h"            // extended string helpers

#include "expat\expat.h"                // XWPHelpers expat XML parser
#include "helpers\xml.h"                // XWPHelpers XML engine

// SOM headers which don't crash with prec. header files
#include "xfwps.ih"
// #include "xfobj.ih"
#include "xfdataf.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#define FILETYPE_PRIVATE
#include "filesys\filetype.h"           // extended file types implementation
#include "filesys\object.h"             // XFldObject implementation

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise
#include <wppgm.h>                      // WPProgram
#include <wppgmf.h>                     // WPProgramFile
#include "filesys\program.h"            // program implementation; WARNING: this redefines macros

/* ******************************************************************
 *
 *   Helper functions for file-type dialogs
 *
 ********************************************************************/

/*
 *@@ AddFileType2Cnr:
 *      this adds the given file type to the given
 *      container window, which should be in Tree
 *      view to have a meaningful display.
 *
 *      pliAssoc->pftrecc is set to the new record
 *      core, which is also returned.
 */

STATIC PFILETYPERECORD AddFileType2Cnr(HWND hwndCnr,           // in: cnr to insert into
                                       PFILETYPERECORD preccParent,  // in: parent recc for tree view
                                       PFILETYPELISTITEM pliAssoc,   // in: file type to add
                                       PLINKLIST pllCheck,      // in: list of types for checking records
                                       PLINKLIST pllDisable)    // in: list of types for disabling records
{
    PFILETYPERECORD preccNew
        = (PFILETYPERECORD)cnrhAllocRecords(hwndCnr, sizeof(FILETYPERECORD), 1);
    // recc attributes
    BOOL        fExpand = FALSE;
    ULONG       ulAttrs = CRA_COLLAPSED | CRA_DROPONABLE;      // records can be dropped

    if (preccNew)
    {
        PLISTNODE   pNode;

        // store reverse linkage V0.9.9 (2001-02-06) [umoeller]
        preccNew->recc.ulStyle = WS_VISIBLE | BS_AUTOCHECKBOX;
        preccNew->recc.usCheckState = 0;
        // for the CHECKBOXRECORDCORE item id, we use the list
        // item pointer... this is unique
        preccNew->recc.ulItemID = (ULONG)pliAssoc;

        preccNew->pliFileType = pliAssoc;

        if (pllCheck)
        {
            pNode = lstQueryFirstNode(pllCheck);
            while (pNode)
            {
                PSZ pszType = (PSZ)pNode->pItemData;
                if (!strcmp(pszType, pliAssoc->pszFileType))
                {
                    // matches:
                    preccNew->recc.usCheckState = 1;
                    fExpand = TRUE;
                    break;
                }
                pNode = pNode->pNext;
            }
        }

        if (pllDisable)
        {
            pNode = lstQueryFirstNode(pllDisable);
            while (pNode)
            {
                PSZ pszType = (PSZ)pNode->pItemData;
                if (!strcmp(pszType, pliAssoc->pszFileType))
                {
                    // matches:
                    preccNew->recc.usCheckState = 1;
                    ulAttrs |= CRA_DISABLED;
                    fExpand = TRUE;
                    break;
                }
                pNode = pNode->pNext;
            }
        }

        if (fExpand)
        {
            cnrhExpandFromRoot(hwndCnr,
                               (PRECORDCORE)preccParent);
        }

        // insert the record
        cnrhInsertRecords(hwndCnr,
                          (PRECORDCORE)preccParent,
                          (PRECORDCORE)preccNew,
                          TRUE, // invalidate
                          pliAssoc->pszFileType,
                          ulAttrs,
                          1);
    }

    pliAssoc->precc = preccNew;
    pliAssoc->fProcessed = TRUE;

    return preccNew;
}

/*
 *@@ AddFileTypeAndAllParents:
 *      adds the specified file type to the cnr;
 *      also adds all the parent file types
 *      if they haven't been added yet.
 */

STATIC PFILETYPERECORD AddFileTypeAndAllParents(HWND hwndCnr,          // in: cnr to insert into
                                                PLINKLIST pllFileTypes, // in: list of all file types
                                                PSZ pszKey,
                                                PLINKLIST pllCheck,      // in: list of types for checking records
                                                PLINKLIST pllDisable)    // in: list of types for disabling records
{
    PFILETYPERECORD     pftreccParent = NULL,
                        pftreccReturn = NULL;
    PLISTNODE           pAssocNode;

    // query the parent for pszKey
    PSZ pszParentForKey = prfhQueryProfileData(HINI_USER,
                                               INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                                               pszKey,
                                               NULL);

    if (pszParentForKey)
    {
        // key has a parent: recurse first! we need the
        // parent records before we insert the actual file
        // type as a child of this
        pftreccParent = AddFileTypeAndAllParents(hwndCnr,
                                                 pllFileTypes,
                                                 // recurse with parent
                                                 pszParentForKey,
                                                 pllCheck,
                                                 pllDisable);
        free(pszParentForKey);
    }

    // we arrive here after the all the parents
    // of pszKey have been added;
    // if we have no parent, pftreccParent is NULL

    // now find the file type list item
    // which corresponds to pKey
    pAssocNode = lstQueryFirstNode(pllFileTypes);
    while (pAssocNode)
    {
        PFILETYPELISTITEM pliAssoc = (PFILETYPELISTITEM)pAssocNode->pItemData;

        if (strcmp(pliAssoc->pszFileType,
                   pszKey) == 0)
        {
            if (!pliAssoc->fProcessed)
            {
                if (!pliAssoc->fCircular)
                    // add record core, which will be stored in
                    // pliAssoc->pftrecc
                    pftreccReturn = AddFileType2Cnr(hwndCnr,
                                                    pftreccParent,
                                                        // parent record; this might be NULL
                                                    pliAssoc,
                                                    pllCheck,
                                                    pllDisable);
                pliAssoc->fCircular = TRUE;
            }
            else
                // record core already created:
                // return that one
                pftreccReturn = pliAssoc->precc;

            // in any case, stop
            break;
        }

        pAssocNode = pAssocNode->pNext;
    }

    if (pAssocNode == NULL)
    {
        // no file type found which corresponds
        // to the hierarchy INI item: delete it,
        // since it has no further meaning
        PrfWriteProfileString(HINI_USER,
                              (PSZ)INIAPP_XWPFILETYPES,    // "XWorkplace:FileTypes"
                              pszKey,
                              NULL);  // delete key
    }

    // return the record core which we created;
    // if this is a recursive call, this will
    // be used as a parent by the parent call
    return pftreccReturn;
}

/*
 *@@ FillCnrWithAvailableTypes:
 *      fills the specified container with the available
 *      file types.
 *
 *      This happens in four steps:
 *
 *      1)  load the WPS file types list
 *          from OS2.INI ("PMWP_ASSOC_FILTER")
 *          and create a linked list from
 *          it in FILETYPESPAGEDATA.pllFileTypes;
 *
 *      2)  load the XWorkplace file types
 *          hierarchy from OS2.INI
 *          ("XWorkplace:FileTypes");
 *
 *      3)  insert all hierarchical file
 *          types in that list;
 *
 *      4)  finally, insert all remaining
 *          WPS file types which have not
 *          been found in the hierarchical
 *          list.
 *
 *      pllCheck and pllDisable are used to automatically
 *      check and/or disable the CHECKBOXRECORDCORE's.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 *@@changed V0.9.12 (2001-05-12) [umoeller]: fixed small memory leak
 */

STATIC VOID FillCnrWithAvailableTypes(HWND hwndCnr,
                                      PLINKLIST pllFileTypes,  // in: list to append types to
                                      PLINKLIST pllCheck,      // in: list of types for checking records
                                      PLINKLIST pllDisable)    // in: list of types for disabling records
{
    APIRET  arc;
    PSZ     pszAssocTypeList = NULL;

    HPOINTER hptrOld = winhSetWaitPointer();

    // step 1: load WPS file types list
    if (!(arc = prfhQueryKeysForApp(HINI_USER,
                                    WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                                    &pszAssocTypeList)))
    {
        PSZ         pKey = pszAssocTypeList;
        PSZ         pszFileTypeHierarchyList;
        PLISTNODE   pAssocNode;

        // if the list had been created before, free it now
        lstClear(pllFileTypes);

        while (*pKey != 0)
        {
            // for each WPS file type,
            // create a list item
            PFILETYPELISTITEM pliAssoc = malloc(sizeof(FILETYPELISTITEM));
            // mark as "not processed"
            pliAssoc->fProcessed = FALSE;
            // set anti-recursion flag
            pliAssoc->fCircular = FALSE;
            // store file type
            pliAssoc->pszFileType = strdup(pKey);
            // add item to list
            lstAppendItem(pllFileTypes, pliAssoc);

            // go for next key
            pKey += strlen(pKey)+1;
        }

        // step 2: load XWorkplace file types hierarchy
        WinEnableWindowUpdate(hwndCnr, FALSE);

        if (!(arc = prfhQueryKeysForApp(HINI_USER,
                                        INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                                        &pszFileTypeHierarchyList)))
        {
            // step 3: go thru the file type hierarchy
            // and add parents;
            // AddFileTypeAndAllParents will mark the
            // inserted items as processed (for step 4)

            pKey = pszFileTypeHierarchyList;
            while (*pKey != 0)
            {
                PFILETYPERECORD precc = AddFileTypeAndAllParents(hwndCnr,
                                                                 pllFileTypes,
                                                                 pKey,
                                                                 pllCheck,
                                                                 pllDisable);
                                                // this will recurse

                // go for next key
                pKey += strlen(pKey)+1;
            }

            free(pszFileTypeHierarchyList); // was missing V0.9.12 (2001-05-12) [umoeller]
        }

        // step 4: add all remaining file types
        // to root level
        pAssocNode = lstQueryFirstNode(pllFileTypes);
        while (pAssocNode)
        {
            PFILETYPELISTITEM pliAssoc = (PFILETYPELISTITEM)(pAssocNode->pItemData);
            if (!pliAssoc->fProcessed)
            {
                PFILETYPERECORD precc = AddFileType2Cnr(hwndCnr,
                                                        NULL,       // parent record == root
                                                        pliAssoc,
                                                        pllCheck,
                                                        pllDisable);
            }
            pAssocNode = pAssocNode->pNext;
        }

        WinShowWindow(hwndCnr, TRUE);

        free(pszAssocTypeList);
    }

    WinSetPointer(HWND_DESKTOP, hptrOld);
}

/*
 *@@ ftypClearTypesList:
 *      cleans up the mess FillCnrWithAvailableTypes
 *      created.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed, made public
 */

VOID ftypClearTypesList(HWND hwndCnr,              // in: cnr, can be NULLHANDLE
                        PLINKLIST pllFileTypes)
{
    PLISTNODE pAssocNode = lstQueryFirstNode(pllFileTypes);
    PFILETYPELISTITEM pliAssoc;

    // first clear the container because the records
    // point into the file-type list items
    if (hwndCnr)
        cnrhRemoveAll(hwndCnr);

    while (pAssocNode)
    {
        pliAssoc = pAssocNode->pItemData;
        if (pliAssoc->pszFileType)
            free(pliAssoc->pszFileType);
        // the pliAssoc will be freed by lstFree

        pAssocNode = pAssocNode->pNext;
    }

    lstClear(pllFileTypes);
}

/* ******************************************************************
 *
 *   XFldWPS notebook callbacks (notebook.c) for "File Types" page
 *
 ********************************************************************/

#ifndef __NOTURBOFOLDERS__

/*
 * ASSOCRECORD:
 *      extended record core structure for the
 *      "Associations" container.
 */

typedef struct _ASSOCRECORD
{
    RECORDCORE          recc;
    HOBJECT             hobj;
} ASSOCRECORD, *PASSOCRECORD;

/*
 * FILETYPEPAGEDATA:
 *      this is created in PCREATENOTEBOOKPAGE
 *      to store various data for the file types
 *      page.
 */

typedef struct _FILETYPESPAGEDATA
{
    // reverse linkage to notebook page data
    // (needed for subwindows)
    PNOTEBOOKPAGE pnbp;

    // linked list of file types (linklist.c);
    // this contains FILETYPELISTITEM structs
    LINKLIST llFileTypes;           // auto-free

    // linked list of various items which have
    // been allocated using malloc(); this
    // will be cleaned up when the page is
    // destroyed (CBI_DESTROY)
    LINKLIST llCleanup;             // auto-free

    // controls which are used all the time
    HWND    hwndTypesCnr,
            hwndAssocsCnr,
            hwndFiltersCnr,
            hwndIconStatic;

    // popup menus
    HWND    hmenuFileTypeSel,
            hmenuFileTypeNoSel,
            hmenuFileFilterSel,
            hmenuFileFilterNoSel,
            hmenuFileAssocSel,
            hmenuFileAssocNoSel;

    // non-modal "Import WPS Filters" dialog
    // or NULLHANDLE, if not open
    HWND    hwndWPSImportDlg;

    // original window proc of the "Associations" list box
    // (which has been subclassed to allow for d'n'd);
    PFNWP   pfnwpListBoxOriginal;

    // currently selected record core in container
    // (updated by CN_EMPHASIS)
    PFILETYPERECORD pftreccSelected;

    // drag'n'drop within the file types container
    // BOOL fFileTypesDnDValid;

    // drag'n'drop of Desktop objects to assocs container;
    // NULL if d'n'd is invalid
    WPObject *pobjDrop;
    // record core after which item is to be inserted
    PRECORDCORE preccAfter;

    // rename of file types:
    PSZ     pszFileTypeOld;         // NULL until a file type was actually renamed

} FILETYPESPAGEDATA, *PFILETYPESPAGEDATA;

// define a new rendering mechanism, which only
// our own container supports (this will make
// sure that we can only do d'n'd within this
// one container)
#define DRAG_RMF  "(DRM_XWPFILETYPES)x(DRF_UNKNOWN)"

#define RECORD_DISABLED CRA_DISABLED

/*
 *@@ AddAssocObject2Cnr:
 *      this adds the given WPProgram or WPProgramFile object
 *      to the given position in the "Associations" container.
 *      The object's object handle is stored in the ASSOCRECORD.
 *
 *      PM provides those "handles" for each list box item,
 *      which can be used for any purpose by an application.
 *      We use it for the object handles here.
 *
 *      Note: This does NOT invalidate the container.
 *
 *      Returns the new ASSOCRECORD or NULL upon errors.
 *
 *@@changed V0.9.4 (2000-06-14) [umoeller]: fixed repaint problems
 *@@changed V0.9.16 (2001-09-29) [umoeller]: added icons to assoc records
 */

STATIC PASSOCRECORD AddAssocObject2Cnr(HWND hwndAssocsCnr,
                                       WPObject *pObject,  // in: must be a WPProgram or WPProgramFile
                                       PRECORDCORE preccInsertAfter, // in: record to insert after (or CMA_FIRST or CMA_END)
                                       BOOL fEnableRecord) // in: if FALSE, the record will be disabled
{
    // ULONG ulrc = LIT_ERROR;

    PSZ pszObjectTitle = _wpQueryTitle(pObject);

    PASSOCRECORD preccNew
        = (PASSOCRECORD)cnrhAllocRecords(hwndAssocsCnr,
                                         sizeof(ASSOCRECORD),
                                         1);
    if (preccNew)
    {
        ULONG       flRecordAttr = CRA_RECORDREADONLY | CRA_DROPONABLE;
        WPObject    *pobj;

        if (!fEnableRecord)
            flRecordAttr |= RECORD_DISABLED;

        // store object handle for later
        preccNew->hobj = _wpQueryHandle(pObject);

        PMPF_ASSOCS(("flRecordAttr %lX", flRecordAttr));

        // add icon V0.9.16 (2001-09-29) [umoeller]
        if (pobj = objFindObjFromHandle(preccNew->hobj))
        {
            preccNew->recc.hptrIcon
            = preccNew->recc.hptrMiniIcon
            = _wpQueryIcon(pobj);
        }

        cnrhInsertRecordAfter(hwndAssocsCnr,
                              (PRECORDCORE)preccNew,
                              pszObjectTitle,
                              flRecordAttr,
                              (PRECORDCORE)preccInsertAfter,
                              FALSE);   // no invalidate
    }

    return preccNew;
}

/*
 *@@ WriteAssocs2INI:
 *      this updates the "PMWP_ASSOC_FILTER" key in OS2.INI
 *      when the associations for a certain file type have
 *      been changed.
 *
 *      This func automatically determines the file type
 *      to update (== the INI "key") from hwndCnr and
 *      reads the associations for it from the list box.
 *      The list box entries must have been added using
 *      AddAssocObject2Cnr, because otherwise this
 *      func cannot find the object handles.
 */

STATIC BOOL WriteAssocs2INI(PSZ  pszProfileKey, // in: either "PMWP_ASSOC_TYPE" or "PMWP_ASSOC_FILTER"
                            HWND hwndTypesCnr,  // in: cnr with selected FILETYPERECORD
                            HWND hwndAssocsCnr) // in: cnr with ASSOCRECORDs
{
    BOOL    brc = FALSE;

    if ((hwndTypesCnr) && (hwndAssocsCnr))
    {
        // get selected file type; since the cnr is in
        // Tree view, there can be only one
        PFILETYPERECORD preccSelected
            = (PFILETYPERECORD)WinSendMsg(hwndTypesCnr,
                                          CM_QUERYRECORDEMPHASIS,
                                          (MPARAM)CMA_FIRST,
                                          (MPARAM)CRA_SELECTED);
        if (    (preccSelected)
             && ((LONG)preccSelected != -1)
           )
        {
            // the file type is equal to the record core title
            PSZ     pszFileType = preccSelected->recc.recc.pszIcon;

            CHAR    szAssocs[1000] = "";
            ULONG   cbAssocs = 0;

            // now create the handles string for PMWP_ASSOC_FILTER
            // from the ASSOCRECORD handles (which have been set
            // by AddAssocObject2Cnr above)

            PASSOCRECORD preccThis = (PASSOCRECORD)WinSendMsg(
                                                hwndAssocsCnr,
                                                CM_QUERYRECORD,
                                                NULL,
                                                MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));

            while ((preccThis) && ((ULONG)preccThis != -1))
            {
                if (preccThis->hobj == 0)
                    // item not found: exit
                    break;

                // if the assocs record is not disabled (i.e. not from
                // the parent record), add it to the string
                if ((preccThis->recc.flRecordAttr & RECORD_DISABLED) == 0)
                {
                    cbAssocs += sprintf(&(szAssocs[cbAssocs]), "%d", preccThis->hobj) + 1;
                }

                preccThis = (PASSOCRECORD)WinSendMsg(hwndAssocsCnr,
                                                     CM_QUERYRECORD,
                                                     preccThis,
                                                     MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
            }

            if (cbAssocs == 0)
                // always write at least the initial 0 byte
                cbAssocs = 1;

            brc = PrfWriteProfileData(HINI_USER,
                                      pszProfileKey,
                                      pszFileType,        // from cnr
                                      szAssocs,
                                      cbAssocs);
        }
    }

    return brc;
}

/*
 *@@ UpdateAssocsCnr:
 *      this updates the given list box with associations,
 *      calling AddAssocObject2Cnr to add the
 *      program objects.
 *
 *      This gets called in several situations:
 *
 *      a)  when we're in "Associations"
 *          mode and a new record core gets selected in
 *          the container, or if we're switching to
 *          "Associations" mode; in this case, the
 *          "Associations" list box on the notebook
 *          page is updated (using "PMWP_ASSOC_TYPE");
 *
 *      b)  from the "Import WPS Filters" dialog,
 *          with the "Associations" list box in that
 *          dialog (and "PMWP_ASSOC_FILTER").
 */

STATIC VOID UpdateAssocsCnr(HWND hwndAssocsCnr,    // in: container to update
                            PSZ  pszTypeOrFilter,  // in: file type or file filter
                            PSZ  pszINIApp,        // in: "PMWP_ASSOC_TYPE" or "PMWP_ASSOC_FILTER"
                            BOOL fEmpty,           // in: if TRUE, list box will be emptied beforehand
                            BOOL fEnableRecords)   // in: if FALSE, the records will be disabled
{
    // get WPS associations from OS2.INI for this file type/filter
    ULONG cbAssocData;
    PSZ pszAssocData = prfhQueryProfileData(HINI_USER,
                                            pszINIApp, // "PMWP_ASSOC_TYPE" or "PMWP_ASSOC_FILTER"
                                            pszTypeOrFilter,
                                            &cbAssocData);

    if (fEmpty)
        // empty listbox
        WinSendMsg(hwndAssocsCnr,
                   CM_REMOVERECORD,
                   NULL,
                   MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));

    if (pszAssocData)
    {
        // pszAssocData now has the handles of the associated
        // objects (as decimal strings, which we'll decode now)
        PSZ     pAssoc = pszAssocData;
        if (pAssoc)
        {
            HOBJECT hobjAssoc;
            WPObject *pobjAssoc;

            // now parse the handles string
            while (*pAssoc)
            {
                sscanf(pAssoc, "%d", &hobjAssoc);
                pobjAssoc = objFindObjFromHandle(hobjAssoc);   // V0.9.9 (2001-04-02) [umoeller]
                if (pobjAssoc)
                {
                    PMPF_ASSOCS(("Adding record, fEnable: %d", fEnableRecords));

                    AddAssocObject2Cnr(hwndAssocsCnr,
                                       pobjAssoc,
                                       (PRECORDCORE)CMA_END,
                                       fEnableRecords);

                    // go for next object handle (after the 0 byte)
                    pAssoc += strlen(pAssoc) + 1;
                    if (pAssoc >= pszAssocData + cbAssocData)
                        break; // while (*pAssoc)
                }
                else
                    break; // while (*pAssoc)
            } // end while (*pAssoc)

            cnrhInvalidateAll(hwndAssocsCnr);
        }

        free(pszAssocData);
    }
}

/*
 *@@ AddFilter2Cnr:
 *      this adds the given file filter to the "Filters"
 *      container window, which should be in Name view
 *      to have a meaningful display.
 *
 *      The new record core is returned.
 */

STATIC PRECORDCORE AddFilter2Cnr(PFILETYPESPAGEDATA pftpd,
                                 PCSZ pcszFilter)    // in: filter name
{
    PRECORDCORE preccNew;
    if (preccNew = cnrhAllocRecords(pftpd->hwndFiltersCnr,
                                    sizeof(RECORDCORE),
                                    1))
    {
        PSZ pszNewFilter = strdup(pcszFilter);
        // always make filters upper-case
        nlsUpper(pszNewFilter);

        // insert the record (helpers/winh.c)
        cnrhInsertRecords(pftpd->hwndFiltersCnr,
                          (PRECORDCORE)NULL,      // parent
                          (PRECORDCORE)preccNew,
                          TRUE, // invalidate
                          pszNewFilter,
                          CRA_RECORDREADONLY,
                          1); // one record

        // store the new title in the linked
        // list of items to be cleaned up
        lstAppendItem(&pftpd->llCleanup, pszNewFilter);
    }

    return preccNew;
}

/*
 *@@ WriteXWPFilters2INI:
 *      this updates the "XWorkplace:FileFilters" key in OS2.INI
 *      when the filters for a certain file type have been changed.
 *
 *      This func automatically determines the file type
 *      to update (== the INI "key") from the "Types" container
 *      and reads the filters for that type for it from the "Filters"
 *      container.
 *
 *      Returns the number of filters written into the INI data.
 */

STATIC ULONG WriteXWPFilters2INI(PFILETYPESPAGEDATA pftpd)
{
    ULONG ulrc = 0;

    if (pftpd->pftreccSelected)
    {
        PSZ     pszFileType = pftpd->pftreccSelected->recc.recc.pszIcon;
        CHAR    szFilters[2000] = "";   // should suffice
        ULONG   cbFilters = 0;
        PRECORDCORE preccFilter = NULL;
        // now create the filters string for INIAPP_XWPFILEFILTERS
        // from the record core titles in the "Filters" container
        while (TRUE)
        {
            preccFilter = WinSendMsg(pftpd->hwndFiltersCnr,
                                     CM_QUERYRECORD,
                                     preccFilter,   // ignored when CMA_FIRST
                                     MPFROM2SHORT((preccFilter)
                                                    ? CMA_NEXT
                                                    // first call:
                                                    : CMA_FIRST,
                                           CMA_ITEMORDER));

            if ((preccFilter == NULL) || (preccFilter == (PRECORDCORE)-1))
                // record not found: exit
                break;

            cbFilters += sprintf(&(szFilters[cbFilters]),
                                "%s",
                                preccFilter->pszIcon    // filter string
                               ) + 1;
            ulrc++;
        }

        PrfWriteProfileData(HINI_USER,
                            (PSZ)INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                            pszFileType,        // from cnr
                            (cbFilters)
                                ? szFilters
                                : NULL,     // no items found: delete key
                            cbFilters);
    }

    return ulrc;
}

/*
 *@@ UpdateFiltersCnr:
 *      this updates the "Filters" container
 *      (which is only visible in "Filters" mode)
 *      according to FILETYPESPAGEDATA.pftreccSelected.
 *      (This structure is stored in CREATENOTEBOOKPAGE.pUser.)
 *
 *      This gets called when we're in "Filters"
 *      mode and a new record core gets selected in
 *      the container, or if we're switching to
 *      "Filters" mode.
 *
 *      In both cases, FILETYPESPAGEDATA.pftreccSelected
 *      has the currently selected file type in the container.
 */

STATIC VOID UpdateFiltersCnr(PFILETYPESPAGEDATA pftpd)
{
    // get text of selected record core
    PSZ pszFileType = pftpd->pftreccSelected->recc.recc.pszIcon;
    // pszFileType now has the selected file type

    // get the XWorkplace-defined filters for this file type
    ULONG cbFiltersData;
    PSZ pszFiltersData = prfhQueryProfileData(HINI_USER,
                                              INIAPP_XWPFILEFILTERS,  // "XWorkplace:FileFilters"
                                              pszFileType,
                                              &cbFiltersData);

    // empty container
    WinSendMsg(pftpd->hwndFiltersCnr,
               CM_REMOVERECORD,
               (MPARAM)NULL,
               MPFROM2SHORT(0,      // all records
                        CMA_FREE | CMA_INVALIDATE));

    if (pszFiltersData)
    {
        // pszFiltersData now has a string array of
        // defined filters, each null-terminated
        PSZ     pFilter = pszFiltersData;

        if (pFilter)
        {
            // now parse the filters string
            while (*pFilter)
            {
                // add the filter to the "Filters" container
                AddFilter2Cnr(pftpd, pFilter);

                // go for next object filter (after the 0 byte)
                pFilter += strlen(pFilter) + 1;
                if (pFilter >= pszFiltersData + cbFiltersData)
                    break; // while (*pFilter))
            } // end while (*pFilter)
        }

        free(pszFiltersData);
    }
}

/*
 *@@ CreateFileType:
 *      creates a new file type.
 *
 *      Returns FALSE if an error occurred, e.g. if
 *      the file type already existed.
 *
 *@@added V0.9.7 (2000-12-13) [umoeller]
 */

STATIC BOOL CreateFileType(PFILETYPESPAGEDATA pftpd,
                           PSZ pszNewType,             // in: new type (malloc!)
                           PFILETYPERECORD pParent)    // in: parent record or NULL if root type
{
    BOOL brc = FALSE;

    ULONG cbData = 0;
    // check if WPS type exists already
    if (    (!PrfQueryProfileSize(HINI_USER,
                                  (PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                                  pszNewType,
                                  &cbData))
         && (cbData == 0)
       )
    {
        // no:
        // write to WPS's file types list
        BYTE bData = 0;
        if (PrfWriteProfileData(HINI_USER,
                                (PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                                pszNewType,
                                &bData,
                                1))     // one byte
        {
            if (pParent)
                // parent specified:
                // add parent type to XWP's types to make
                // it a subtype
                brc = PrfWriteProfileString(HINI_USER,
                                            // application: "XWorkplace:FileTypes"
                                            (PSZ)INIAPP_XWPFILETYPES,
                                            // key --> the new file type:
                                            pszNewType,
                                            // string --> the parent:
                                            pParent->recc.recc.pszIcon);
            else
                brc = TRUE;

            if (brc)
            {
                // create new list item
                PFILETYPELISTITEM pliAssoc = (PFILETYPELISTITEM)malloc(sizeof(FILETYPELISTITEM));
                // mark as "processed"
                pliAssoc->fProcessed = TRUE;
                // store file type
                pliAssoc->pszFileType = pszNewType;     // malloc!
                // add record core, which will be stored in
                // pliAssoc->pftrecc
                AddFileType2Cnr(pftpd->hwndTypesCnr,
                                pParent,
                                pliAssoc,
                                NULL,
                                NULL);
                brc = (lstAppendItem(&pftpd->llFileTypes, pliAssoc) != NULL);
            }
        }
    }

    return brc;
}

/*
 *@@ CheckFileTypeDrag:
 *      checks a drag operation for whether the container
 *      can accept it.
 *
 *      This has been exported from ftypFileTypesInitPage
 *      because both CN_DRAGOVER and CN_DROP need to
 *      check this. CN_DRAGOVER is never received in a
 *      lazy drag operation.
 *
 *      This does not change pftpd->fFileTypesDnDValid,
 *      but returns TRUE or FALSE instead.
 *
 *@@added V0.9.7 (2000-12-13) [umoeller]
 */

STATIC BOOL CheckFileTypeDrag(PFILETYPESPAGEDATA pftpd,
                              PDRAGINFO pDragInfo,     // in: drag info
                              PFILETYPERECORD pTargetRec, // in: target record from CNRDRAGINFO
                              PUSHORT pusIndicator,    // out: DOR_* flag for indicator (ptr can be NULL)
                              PUSHORT pusOperation)    // out: DOR_* flag for operation (ptr can be NULL)
{
    BOOL brc = FALSE;

    // OK so far:
    if (
            // accept no more than one single item at a time;
            // we cannot move more than one file type
            (pDragInfo->cditem != 1)
            // make sure that we only accept drops from ourselves,
            // not from other windows
         || (pDragInfo->hwndSource
                    != pftpd->hwndTypesCnr)
        )
    {
        if (pusIndicator)
            *pusIndicator = DOR_NEVERDROP;
        PMPF_ASSOCS(("   invalid items or invalid target"));
    }
    else
    {

        // accept only default drop operation or move
        if (    (pDragInfo->usOperation == DO_DEFAULT)
             || (pDragInfo->usOperation == DO_MOVE)
           )
        {
            // get the item being dragged (PDRAGITEM)
            PDRAGITEM pdrgItem = DrgQueryDragitemPtr(pDragInfo, 0);
            if (pdrgItem)
            {
                if (    (pTargetRec // pcdi->pRecord  // target recc
                          != (PFILETYPERECORD)pdrgItem->ulItemID) // source recc
                     && (DrgVerifyRMF(pdrgItem, "DRM_XWPFILETYPES", "DRF_UNKNOWN"))
                   )
                {
                    // do not allow dragging the record on
                    // a child record of itself
                    // V0.9.7 (2000-12-13) [umoeller] thanks Martin Lafaix
                    if (!cnrhIsChildOf(pftpd->hwndTypesCnr,
                                       (PRECORDCORE)pTargetRec, // pcdi->pRecord,   // target
                                       (PRECORDCORE)pdrgItem->ulItemID)) // source
                    {
                        // allow drop
                        if (pusIndicator)
                            *pusIndicator = DOR_DROP;
                        if (pusOperation)
                            *pusOperation = DO_MOVE;
                        brc = TRUE;
                    }
                    else
                        PMPF_ASSOCS(("   target is child of source"));
                }
                else
                    PMPF_ASSOCS(("   invalid RMF"));
            }
            else
                PMPF_ASSOCS(("   cannot get drag item"));
        }
        else
            PMPF_ASSOCS(("   invalid operation 0x%lX", pDragInfo->usOperation));
    }

    return brc;
}

/* DLGTEMPLATE ID_XSD_FILETYPES LOADONCALL MOVEABLE DISCARDABLE
BEGIN
    DIALOG  "", ID_XSD_FILETYPES, 0, 0, 265, 162, NOT FS_DLGBORDER |
            WS_VISIBLE
    BEGIN
        CONTAINER       ID_XSDI_FT_CONTAINER, 10, 27, 110, 123,
                        CCS_SINGLESEL | CCS_VERIFYPOINTERS
                        PRESPARAMS PP_BACKGROUNDCOLOR, 0x00FFFFFFL
        LTEXT           "Filters", ID_XSDI_FT_FILTERS_TXT, 125, 144, 105, 8,
                        NOT WS_GROUP
        CONTAINER       ID_XSDI_FT_FILTERSCNR, 125, 102, 105, 39,
                        CCS_EXTENDSEL
                        PRESPARAMS PP_BACKGROUNDCOLOR, 0x00FFFFFFL
        LTEXT           "Associations", ID_XSDI_FT_ASSOCS_TXT, 125, 90, 105,
                        8, NOT WS_GROUP
        CONTAINER       ID_XSDI_FT_ASSOCSCNR, 125, 27, 105, 61,
                        CCS_EXTENDSEL
                        PRESPARAMS PP_BACKGROUNDCOLOR, 0x00FFFFFFL
        GROUPBOX        "File types", ID_XSDI_FT_GROUP, 5, 24, 230, 135,
                        DT_MNEMONIC
        PUSHBUTTON      "~Undo", DID_UNDO, 5, 6, 70, 12, WS_GROUP
        PUSHBUTTON      "~Default", DID_DEFAULT, 85, 6, 70, 12
        PUSHBUTTON      "~Help", DID_HELP, 165, 6, 70, 12, BS_HELP
    END
END */

#define FT_TEXT_HEIGHT          10
#define FT_CNR_HEIGHT           40

static const CONTROLDEF
    FTGroup = LOADDEF_GROUP(ID_XSDI_FT_GROUP, SZL_AUTOSIZE),
    FTCnr = CONTROLDEF_CONTAINER(ID_XSDI_FT_CONTAINER,
                                 100,
                                   2 * FT_TEXT_HEIGHT
                                 + 2 * FT_CNR_HEIGHT
                                 + 6 * COMMON_SPACING),
    FTFiltersTxt = CONTROLDEF_TEXT(LOAD_STRING, ID_XSDI_FT_FILTERS_TXT, 100, FT_TEXT_HEIGHT),
    FTFiltersCnr = CONTROLDEF_CONTAINER_EXTSEL(ID_XSDI_FT_FILTERSCNR, 100, FT_CNR_HEIGHT),
    FTAssocsTxt = CONTROLDEF_TEXT(LOAD_STRING, ID_XSDI_FT_ASSOCS_TXT, 100, FT_TEXT_HEIGHT),
    FTAssocsCnr = CONTROLDEF_CONTAINER_EXTSEL(ID_XSDI_FT_ASSOCSCNR, 100, FT_CNR_HEIGHT),
    FTCreateDatafileHandleCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FT_CREATEDATAFILEHANDLE);

static const DLGHITEM G_dlgFileTypes[] =
    {
        START_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&FTGroup),
                    START_ROW(0),
                        CONTROL_DEF(&FTCnr),
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&FTFiltersTxt),
                            START_ROW(0),
                                CONTROL_DEF(&FTFiltersCnr),
                            START_ROW(0),
                                CONTROL_DEF(&FTAssocsTxt),
                            START_ROW(0),
                                CONTROL_DEF(&FTAssocsCnr),
                        END_TABLE,
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&FTCreateDatafileHandleCB),
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE,
    };

/*
 *@@ G_ampFileTypesPage:
 *      resizing information for "File types" page.
 *      Stored in CREATENOTEBOOKPAGE of the
 *      respective "add notebook page" method.
 *
 *@@added V0.9.4 (2000-08-08) [umoeller]
 */

static MPARAM G_ampFileTypesPage[] =
    {
        MPFROM2SHORT(ID_XSDI_FT_GROUP, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XSDI_FT_CONTAINER, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XSDI_FT_FILTERS_TXT, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FT_FILTERSCNR, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FT_ASSOCS_TXT, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FT_ASSOCSCNR, XAC_MOVEX | XAC_SIZEY)
    };

extern MPARAM *G_pampFileTypesPage = G_ampFileTypesPage;
extern ULONG G_cFileTypesPage = sizeof(G_ampFileTypesPage) / sizeof(G_ampFileTypesPage[0]);

/*
 *@@ ftypFileTypesInitPage:
 *      notebook callback function (notebook.c) for the
 *      "File types" page in the "Workplace Shell" object.
 *
 *      This page is maybe the most complicated of the
 *      "Workplace Shell" settings pages, but maybe also
 *      the most useful. ;-)
 *
 *      In this function, we set up the following window
 *      hierarchy (all of which exists in the dialog
 *      template loaded from the NLS DLL):
 *
 +      CREATENOTEBOOKPAGE.hwndPage (dialog in notebook,
 +        |                          maintained by notebook.c)
 +        |
 +        +-- ID_XSDI_FT_CONTAINER (container with file types);
 +        |     this thing is rather smart in that it can handle
 +        |     d'n'd within the same window (see
 +        |     ftypFileTypesItemChanged for details).
 +        |
 +        +-- ID_XSDI_FT_FILTERSCNR (container with filters);
 +        |     this has the filters for the file type (e.g. "*.txt");
 +        |     a plain container in flowed text view.
 +        |     This gets updated via UpdateFiltersCnr.
 +        |
 +        +-- ID_XSDI_FT_ASSOCSCNR (container with associations);
 +              this container is in non-flowed name view to hold
 +              the associations for the current file type. This
 +              accepts WPProgram's and WPProgramFile's via drag'n'drop.
 +              This gets updated via UpdateAssocsCnr.
 *
 *      The means of interaction between all these controls is the
 *      FILETYPESPAGEDATA structure which contains lots of data
 *      so all the different functions know what's going on. This
 *      is created here and stored in CREATENOTEBOOKPAGE.pUser so
 *      that it is always accessible and will be free()'d automatically.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.6 (2000-10-16) [umoeller]: fixed excessive menu creation
 *@@changed V0.9.20 (2002-08-04) [umoeller]: now using dialog formatter
 *@@changed V0.9.20 (2002-08-04) [umoeller]: added setting for data file handles
 */

VOID ftypFileTypesInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                           ULONG flFlags)        // CBI_* flags (notebook.h)
{
    /*
     * CBI_INIT:
     *      initialize page (called only once)
     */

    if (flFlags & CBI_INIT)
    {
        PFILETYPESPAGEDATA pftpd;

        // first call: create FILETYPESPAGEDATA
        // structure;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pftpd = malloc(sizeof(FILETYPESPAGEDATA));
        pnbp->pUser = pftpd;
        memset(pftpd, 0, sizeof(FILETYPESPAGEDATA));

        pftpd->pnbp = pnbp;

        lstInit(&pftpd->llFileTypes, TRUE);

        // create "cleanup" list; this will hold all kinds
        // of items which need to be free()'d when the
        // notebook page is destroyed.
        // We just keep storing stuff in here so we need not
        // keep track of where we allocated what.
        lstInit(&pftpd->llCleanup, TRUE);

        // insert the controls using the dialog formatter
        // V0.9.20 (2002-08-04) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgFileTypes,
                      ARRAYITEMCOUNT(G_dlgFileTypes));

        // store container hwnd's
        pftpd->hwndTypesCnr = WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_FT_CONTAINER);
        pftpd->hwndFiltersCnr = WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_FT_FILTERSCNR);
        pftpd->hwndAssocsCnr = WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_FT_ASSOCSCNR);

        // set up file types container
        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_TREE | CA_TREELINE | CV_TEXT);
            cnrhSetTreeIndent(15);
            cnrhSetSortFunc(fnCompareName);
        } END_CNRINFO(pftpd->hwndTypesCnr);

        // set up filters container
        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_TEXT | CV_FLOW);
            cnrhSetSortFunc(fnCompareName);
        } END_CNRINFO(pftpd->hwndFiltersCnr);

        // set up assocs container
        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_NAME | CV_MINI
                         | CA_ORDEREDTARGETEMPH
                                        // allow dropping only _between_ records
                         | CA_OWNERDRAW);
                                        // for disabled records
            // no sort here
        } END_CNRINFO(pftpd->hwndAssocsCnr);

        // flags for cnr owner draw
        pnbp->inbp.ulCnrOwnerDraw = CODFL_DISABLEDTEXT | CODFL_MINIICON;

        pftpd->hmenuFileTypeSel = WinLoadMenu(HWND_OBJECT,
                                              cmnQueryNLSModuleHandle(FALSE),
                                              ID_XSM_FILETYPES_SEL);
        pftpd->hmenuFileTypeNoSel = WinLoadMenu(HWND_OBJECT,
                                                cmnQueryNLSModuleHandle(FALSE),
                                                ID_XSM_FILETYPES_NOSEL);
        pftpd->hmenuFileFilterSel = WinLoadMenu(HWND_OBJECT,
                                                cmnQueryNLSModuleHandle(FALSE),
                                                ID_XSM_FILEFILTER_SEL);
        pftpd->hmenuFileFilterNoSel = WinLoadMenu(HWND_OBJECT,
                                                  cmnQueryNLSModuleHandle(FALSE),
                                                  ID_XSM_FILEFILTER_NOSEL);
        pftpd->hmenuFileAssocSel = WinLoadMenu(HWND_OBJECT,
                                               cmnQueryNLSModuleHandle(FALSE),
                                               ID_XSM_FILEASSOC_SEL);
        pftpd->hmenuFileAssocNoSel = WinLoadMenu(HWND_OBJECT,
                                                 cmnQueryNLSModuleHandle(FALSE),
                                                 ID_XSM_FILEASSOC_NOSEL);
    }

    /*
     * CBI_SET:
     *      set controls' data
     */

    if (flFlags & CBI_SET)
    {
        PFILETYPESPAGEDATA pftpd = (PFILETYPESPAGEDATA)pnbp->pUser;

        ftypClearTypesList(pftpd->hwndTypesCnr,
                           &pftpd->llFileTypes);

        FillCnrWithAvailableTypes(pftpd->hwndTypesCnr,
                                  &pftpd->llFileTypes,
                                  NULL,         // check list
                                  NULL);        // disable list

        // V0.9.20 (2002-08-04) [umoeller]
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_FT_CREATEDATAFILEHANDLE,
                              cmnQuerySetting(sfDatafileOBJHANDLE));
    }

    /*
     * CBI_SHOW / CBI_HIDE:
     *      notebook page is turned to or away from
     */

    if (flFlags & (CBI_SHOW | CBI_HIDE))
    {
        PFILETYPESPAGEDATA pftpd = (PFILETYPESPAGEDATA)pnbp->pUser;
        if (pftpd->hwndWPSImportDlg)
            WinShowWindow(pftpd->hwndWPSImportDlg, (flFlags & CBI_SHOW));
    }

    /*
     * CBI_DESTROY:
     *      clean up page before destruction
     */

    if (flFlags & CBI_DESTROY)
    {
        PFILETYPESPAGEDATA pftpd = (PFILETYPESPAGEDATA)pnbp->pUser;

        winhDestroyWindow(&pftpd->hmenuFileTypeSel);
        winhDestroyWindow(&pftpd->hmenuFileTypeNoSel);
        winhDestroyWindow(&pftpd->hmenuFileFilterSel);
        winhDestroyWindow(&pftpd->hmenuFileFilterNoSel);
        winhDestroyWindow(&pftpd->hmenuFileAssocSel);
        winhDestroyWindow(&pftpd->hmenuFileAssocNoSel);

        winhDestroyWindow(&pftpd->hwndWPSImportDlg);

        ftypClearTypesList(pftpd->hwndTypesCnr,
                           &pftpd->llFileTypes);

        // destroy "cleanup" list; this will
        // also free all the data on the list
        // using free()
        lstClear(&pftpd->llCleanup);
    }
}

/*
 *@@ ImportNewTypes:
 *
 *@@added V0.9.16 (2001-12-02) [umoeller]
 */

STATIC VOID ImportNewTypes(PNOTEBOOKPAGE pnbp)
{
    CHAR szFilename[CCHMAXPATH];
    sprintf(szFilename, "%c:\\xwptypes.xtp", doshQueryBootDrive());
    if (cmnFileDlg(pnbp->hwndDlgPage,
                   szFilename,
                   WINH_FOD_INILOADDIR | WINH_FOD_INISAVEDIR,
                   HINI_USER,
                   INIAPP_XWORKPLACE,
                   "XWPFileTypesDlg"))
    {
        // create XML document then
        PCSZ apsz[2] = { szFilename, 0 };
        HPOINTER hptrOld = winhSetWaitPointer();
        XSTRING strError;
        APIRET arc;
        xstrInit(&strError, 0);

        arc = ftypImportTypes(szFilename, &strError);

        WinSetPointer(HWND_DESKTOP, hptrOld);

        if (!arc)
        {
            // call "init" callback to reinitialize the page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);

            cmnMessageBoxExt(pnbp->hwndDlgPage,
                                121,            // xwp
                                apsz,
                                1,
                                215,            // successfully imported from %1
                                MB_OK);
        }
        else
        {
            apsz[1] = strError.psz;
            cmnMessageBoxExt(pnbp->hwndDlgPage,
                                104,            // xwp: error
                                apsz,
                                2,
                                216,            // error %2 imported from %1
                                MB_OK);
        }

        xstrClear(&strError);
    }
}

/*
 *@@ ftypFileTypesItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "File types" page in the "Workplace Shell" object.
 *      Reacts to changes of any of the dialog controls.
 *
 *      This is a real monster function, since it handles
 *      all the controls on the page, including container
 *      drag and drop.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.7 (2000-12-10) [umoeller]: DrgFreeDraginfo was missing
 *@@changed V0.9.7 (2000-12-13) [umoeller]: fixed dragging file type onto its child; thanks Martin Lafaix
 *@@changed V0.9.7 (2000-12-13) [umoeller]: fixed cleanup problem with lazy drag; thanks Martin Lafaix
 *@@changed V0.9.7 (2000-12-13) [umoeller]: lazy drop menu items were enabled wrong; thanks Martin Lafaix
 *@@changed V0.9.7 (2000-12-13) [umoeller]: made lazy drop menu items work
 *@@changed V0.9.7 (2000-12-13) [umoeller]: added "Create subtype" menu item
 *@@changed V0.9.16 (2001-09-29) [umoeller]: added "properties" and "open folder" assoc items
 */

MRESULT ftypFileTypesItemChanged(PNOTEBOOKPAGE pnbp,
                                 ULONG ulItemID,
                                 USHORT usNotifyCode,
                                 ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;

    PFILETYPESPAGEDATA pftpd = (PFILETYPESPAGEDATA)pnbp->pUser;

    switch (ulItemID)
    {
        // V0.9.20 (2002-08-04) [umoeller]
        case ID_XSDI_FT_CREATEDATAFILEHANDLE:
            cmnSetSetting(sfDatafileOBJHANDLE, ulExtra);
        break;

        /*
         * ID_XSDI_FT_CONTAINER:
         *      "File types" container;
         *      supports d'n'd within itself
         */

        case ID_XSDI_FT_CONTAINER:
        {
            switch (usNotifyCode)
            {
                /*
                 * CN_EMPHASIS:
                 *      new file type selected in file types tree container:
                 *      update the data of the other controls.
                 *      (There can only be one selected record in the tree
                 *      at once.)
                 */

                case CN_EMPHASIS:
                {
                    // ulExtra has new selected record core;
                    if (pftpd->pftreccSelected != (PFILETYPERECORD)ulExtra)
                    {
                        PSZ     pszFileType = 0,
                                pszParentFileType = 0;
                        BOOL    fFirstLoop = TRUE;

                        // store it in FILETYPESPAGEDATA
                        pftpd->pftreccSelected = (PFILETYPERECORD)ulExtra;

                        UpdateFiltersCnr(pftpd);

                        // now go for this file type and keep climbing up
                        // the parent file types
                        pszFileType = strdup(pftpd->pftreccSelected->recc.recc.pszIcon);
                        while (pszFileType)
                        {
                            UpdateAssocsCnr(pftpd->hwndAssocsCnr,
                                                    // cnr to update
                                            pszFileType,
                                                    // file type to search for
                                                    // (selected record core)
                                            (PSZ)WPINIAPP_ASSOCTYPE,  // "PMWP_ASSOC_TYPE"
                                            fFirstLoop,   // empty container first
                                            fFirstLoop);  // enable records
                            fFirstLoop = FALSE;

                            // get parent file type;
                            // this will be NULL if there is none
                            pszParentFileType = prfhQueryProfileData(HINI_USER,
                                                    INIAPP_XWPFILETYPES,    // "XWorkplace:FileTypes"
                                                    pszFileType,
                                                    NULL);
                            free(pszFileType);
                            pszFileType = pszParentFileType;
                        } // end while (pszFileType)

                        if (pftpd->hwndWPSImportDlg)
                            // "Merge" dialog open:
                            // update "Merge with" text
                            WinSetDlgItemText(pftpd->hwndWPSImportDlg,
                                              ID_XSDI_FT_TYPE,
                                              pftpd->pftreccSelected->recc.recc.pszIcon);
                    }
                }
                break; // case CN_EMPHASIS

                /*
                 * CN_INITDRAG:
                 *      file type being dragged
                 * CN_PICKUP:
                 *      lazy drag initiated (Alt+MB2)
                 */

                case CN_INITDRAG:
                case CN_PICKUP:
                {
                    PCNRDRAGINIT pcdi = (PCNRDRAGINIT)ulExtra;

                    if (DrgQueryDragStatus())
                        // (lazy) drag currently in progress: stop
                        break;

                    if (pcdi)
                        // filter out whitespace
                        if (pcdi->pRecord)
                            cnrhInitDrag(pcdi->hwndCnr,
                                         pcdi->pRecord,
                                         usNotifyCode,
                                         DRAG_RMF,
                                         DO_MOVEABLE);

                }
                break; // case CN_INITDRAG

                /*
                 * CN_DRAGOVER:
                 *      file type being dragged over other file type
                 */

                case CN_DRAGOVER:
                {
                    PCNRDRAGINFO pcdi = (PCNRDRAGINFO)ulExtra;
                    USHORT      usIndicator = DOR_NODROP,
                                    // cannot be dropped, but send
                                    // DM_DRAGOVER again
                                usOp = DO_UNKNOWN;
                                    // target-defined drop operation:
                                    // user operation (we don't want
                                    // the WPS to copy anything)

                    PMPF_ASSOCS(("CN_DRAGOVER: entering"));

                    // get access to the drag'n'drop structures
                    if (DrgAccessDraginfo(pcdi->pDragInfo))
                    {
                        CheckFileTypeDrag(pftpd,
                                          pcdi->pDragInfo,
                                          (PFILETYPERECORD)pcdi->pRecord,   // target
                                          &usIndicator,
                                          &usOp);
                        DrgFreeDraginfo(pcdi->pDragInfo);
                    }

                    PMPF_ASSOCS(("CN_DRAGOVER: returning ind 0x%lX, op 0x%lX", usIndicator, usOp));

                    // and return the drop flags
                    mrc = (MRFROM2SHORT(usIndicator, usOp));
                }
                break; // case CN_DRAGOVER

                /*
                 * CN_DROP:
                 *      file type being dropped
                 *      (both for modal d'n'd and non-modal lazy drag).
                 *
                 *      Must always return 0.
                 */

                case CN_DROP:
                {
                    PCNRDRAGINFO pcdi = (PCNRDRAGINFO)ulExtra;

                    PMPF_ASSOCS(("CN_DROP: entering"));

                    // check global valid recc, which was set above
                    // get access to the drag'n'drop structures
                    if (DrgAccessDraginfo(pcdi->pDragInfo))
                    {
                        if (CheckFileTypeDrag(pftpd,
                                              pcdi->pDragInfo,
                                              (PFILETYPERECORD)pcdi->pRecord,   // target
                                              NULL, NULL))
                        {
                            // valid operation:
                            // OK, move the record core tree to the
                            // new location.

                            // This is a bit unusual, because normally
                            // it is the source application which does
                            // the operation as a result of d'n'd. But
                            // who cares, source and target are the
                            // same window here anyway, so let's go.
                            PDRAGITEM   pdrgItem;
                            if (pdrgItem = DrgQueryDragitemPtr(pcdi->pDragInfo, 0))
                            {
                                PFILETYPERECORD precDropped = (PFILETYPERECORD)pdrgItem->ulItemID;
                                PFILETYPERECORD precTarget = (PFILETYPERECORD)pcdi->pRecord;
                                // update container
                                if (cnrhMoveTree(pcdi->pDragInfo->hwndSource,
                                                 // record to move:
                                                 (PRECORDCORE)precDropped,
                                                 // new parent (might be NULL for root level):
                                                 (PRECORDCORE)precTarget,
                                                 // sort function (cnrsort.c)
                                                 (PFNCNRSORT)fnCompareName))
                                {
                                    // update XWP type parents in OS2.INI
                                    PrfWriteProfileString(HINI_USER,
                                                          // application: "XWorkplace:FileTypes"
                                                          (PSZ)INIAPP_XWPFILETYPES,
                                                          // key --> the dragged record:
                                                          precDropped->recc.recc.pszIcon,
                                                          // string --> the parent:
                                                          (precTarget)
                                                              // the parent
                                                              ? precTarget->recc.recc.pszIcon
                                                              // NULL == root: delete key
                                                              : NULL);
                                            // aaarrgh
                                }
                            }
                            else
                                PMPF_ASSOCS(("  Cannot get drag item"));
                        }

                        DrgFreeDraginfo(pcdi->pDragInfo);
                                    // V0.9.7 (2000-12-10) [umoeller]
                    }
                    else
                        PMPF_ASSOCS(("  Cannot get draginfo"));

                    // If CN_DROP was the result of a "real" (modal) d'n'd,
                    // the DrgDrag function in CN_INITDRAG (above)
                    // returns now.

                    // If CN_DROP was the result of a lazy drag (pickup and drop),
                    // the container will now send CN_DROPNOTIFY (below).

                    // In both cases, we clean up the resources: either in
                    // CN_INITDRAG or in CN_DROPNOTIFY.

                    PMPF_ASSOCS(("CN_DROP: returning"));

                }
                break; // case CN_DROP

                /*
                 * CN_DROPNOTIFY:
                 *      this is only sent to the container when
                 *      a lazy drag operation (pickup and drop)
                 *      is finished. Since lazy drag is non-modal
                 *      (DrgLazyDrag in CN_PICKUP returned immediately),
                 *      this is where we must clean up the resources
                 *      (the same as we have done in CN_INITDRAG modally).
                 *
                 *      According to PMREF, if a lazy drop was successful,
                 *      the target window is first sent DM_DROP and
                 *      the source window is is then posted DM_DROPNOTIFY.
                 *
                 *      The standard DM_DROPNOTIFY has the DRAGINFO in mp1
                 *      and the target window in mp2. With the container,
                 *      the target window goes into the CNRLAZYDRAGINFO
                 *      structure.
                 *
                 *      If a lazy drop is cancelled (e.g. for DrgCancelLazyDrag),
                 *      only the source window is posted DM_DROPNOTIFY, with
                 *      mp2 == NULLHANDLE.
                 */

                case CN_DROPNOTIFY:
                {
                    PCNRLAZYDRAGINFO pcldi = (PCNRLAZYDRAGINFO)ulExtra;

                    // get access to the drag'n'drop structures
                    if (DrgAccessDraginfo(pcldi->pDragInfo))
                    {
                        // get the moved record core
                        PDRAGITEM   pdrgItem = DrgQueryDragitemPtr(pcldi->pDragInfo, 0);

                        // remove record "pickup" emphasis
                        WinSendMsg(pcldi->pDragInfo->hwndSource, // hwndCnr
                                   CM_SETRECORDEMPHASIS,
                                   // record to move
                                   (MPARAM)(pdrgItem->ulItemID),
                                   MPFROM2SHORT(FALSE,
                                                CRA_PICKED));

                        // fixed V0.9.7 (2000-12-13) [umoeller]
                        DrgDeleteDraginfoStrHandles(pcldi->pDragInfo);
                        DrgFreeDraginfo(pcldi->pDragInfo);
                    }
                }
                break;

                /*
                 * CN_CONTEXTMENU:
                 *      ulExtra has the record core or NULL
                 *      if whitespace.
                 */

                case CN_CONTEXTMENU:
                {
                    HWND    hPopupMenu;

                    // get drag status
                    BOOL    fDragging = DrgQueryDragStatus();

                    // we store the container and recc.
                    // in the CREATENOTEBOOKPAGE structure
                    // so that the notebook.c function can
                    // remove source emphasis later automatically
                    pnbp->hwndSourceCnr = pnbp->hwndControl;
                    if (pnbp->preccSource = (PRECORDCORE)ulExtra)
                    {
                        // popup menu on container recc:
                        hPopupMenu = pftpd->hmenuFileTypeSel;

                        // if lazy drag is currently in progress,
                        // disable "Pickup" item (we can handle only one)
                        WinEnableMenuItem(hPopupMenu,
                                          ID_XSMI_FILETYPES_PICKUP,
                                          !fDragging);
                    }
                    else
                    {
                        // on whitespace: different menu
                        hPopupMenu = pftpd->hmenuFileTypeNoSel;

                        // already open: disable
                        WinEnableMenuItem(hPopupMenu,
                                          ID_XSMI_FILEFILTER_IMPORTWPS,
                                          (pftpd->hwndWPSImportDlg == NULLHANDLE));
                    }

                    // both menu types:
                    // disable drop and cancel-drag if
                    // no lazy drag in progress:
                    // V0.9.7 (2000-12-13) [umoeller]
                    WinEnableMenuItem(hPopupMenu,
                                      ID_XSMI_FILETYPES_DROP,
                                      fDragging);
                    // disable cancel-drag
                    WinEnableMenuItem(hPopupMenu,
                                      ID_XSMI_FILETYPES_CANCELDRAG,
                                      fDragging);
                    cnrhShowContextMenu(pnbp->hwndControl,     // cnr
                                        (PRECORDCORE)pnbp->preccSource,
                                        hPopupMenu,
                                        pnbp->hwndDlgPage);    // owner
                }
                break;

                case CN_REALLOCPSZ:
                {
                    // rename of file type has ended V0.9.9 (2001-02-06) [umoeller]:
                    PCNREDITDATA pced = (PCNREDITDATA)ulExtra;
                    if (pced->pRecord)
                    {
                        // this was for a record (should always be the case):
                        // we must now allocate sufficient memory for the
                        // new file type...
                        // PCNREDITDATA->cbText has the memory that the cnr
                        // needs to copy the string now.
                        // PFILETYPERECORD pRecord = (PFILETYPERECORD)pced->pRecord;
                        if (    (pced->cbText)
                             && (*(pced->ppszText))
                             && (strlen(*(pced->ppszText)))
                           )
                        {
                            pftpd->pszFileTypeOld = *(pced->ppszText);
                                        // this was allocated using malloc()
                            *(pced->ppszText) = malloc(pced->cbText);

                            mrc = (MPARAM)TRUE;
                        }
                    }
                }
                break;

                case CN_ENDEDIT:
                {
                    PCNREDITDATA pced = (PCNREDITDATA)ulExtra;
                    if (    (pced->pRecord)
                         && (pftpd->pszFileTypeOld)
                                // this is only != NULL if CN_REALLOCPSZ came in
                                // before, i.e. direct edit was not cancelled
                       )
                    {
                        PFILETYPERECORD pRecord = (PFILETYPERECORD)pced->pRecord;
                        PSZ     pszSet;
                        ULONG   ulMsg = 0;
                        PCSZ     pszMsg;
                        APIRET arc = ftypRenameFileType(pftpd->pszFileTypeOld,
                                                        *(pced->ppszText));

                        switch (arc)
                        {
                            case ERROR_FILE_NOT_FOUND:
                                pszMsg = pftpd->pszFileTypeOld;
                                ulMsg = 209;
                            break;

                            case ERROR_FILE_EXISTS:
                                pszMsg = *(pced->ppszText);
                                ulMsg = 210;
                            break;
                        }

                        if (ulMsg)
                        {
                            cmnMessageBoxExt(pnbp->hwndDlgPage,
                                                104,        // error
                                                &pszMsg,
                                                1,
                                                ulMsg,
                                                MB_CANCEL);
                            pszSet = pftpd->pszFileTypeOld;
                        }
                        else
                            pszSet = *(pced->ppszText);

                        // now take care of memory management here...
                        // we must reset the text pointers in the FILETYPERECORD
                        pRecord->recc.recc.pszIcon =
                        pRecord->recc.recc.pszText =
                        pRecord->recc.recc.pszName =
                        pRecord->recc.recc.pszTree = pszSet;

                        // also point the list item to the new text
                        pRecord->pliFileType->pszFileType = pszSet;

                        if (ulMsg)
                        {
                            // error:
                            // invalidate the record
                            WinSendMsg(pftpd->hwndTypesCnr,
                                       CM_INVALIDATERECORD,
                                       (MPARAM)&pRecord,
                                       MPFROM2SHORT(1,
                                                    CMA_ERASE | CMA_TEXTCHANGED));
                            // and free new
                            free(*(pced->ppszText));
                        }
                        else
                            // no error: free old
                            free(pftpd->pszFileTypeOld);

                        pftpd->pszFileTypeOld = NULL;
                    }
                }
                break;

            } // end switch (usNotifyCode)

        }
        break;  // case ID_XSDI_FT_CONTAINER

        /*
         * ID_XSMI_FILETYPES_DELETE:
         *      "Delete file type" context menu item
         *      (file types container tree)
         */

        case ID_XSMI_FILETYPES_DELETE:
        {
            PFILETYPERECORD pftrecc;
                        // this has been set in CN_CONTEXTMENU above
            if (pftrecc = (PFILETYPERECORD)pnbp->preccSource)
            {
                // delete file type from INI
                PrfWriteProfileString(HINI_USER,
                                      (PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE",
                                      pftrecc->recc.recc.pszIcon,
                                      NULL);     // delete
                // and remove record core from container
                WinSendMsg(pftpd->hwndTypesCnr,
                           CM_REMOVERECORD,
                           (MPARAM)&pftrecc,
                           MPFROM2SHORT(1,  // only one record
                                    CMA_FREE | CMA_INVALIDATE));
            }
        }
        break;

        /*
         * ID_XSMI_FILETYPES_NEW:
         *      "Create file type" context menu item
         *      (file types container tree); this can
         *      be both on whitespace and on a type record
         */

        case ID_XSMI_FILETYPES_NEW:
        {
            HWND hwndDlg;
            if (hwndDlg = cmnLoadDlg(pnbp->hwndFrame,  // owner
                                     WinDefDlgProc,
                                     ID_XSD_NEWFILETYPE,   // "New File Type" dlg
                                     NULL))
            {
                winhSetEntryFieldLimit(WinWindowFromID(hwndDlg, ID_XSDI_FT_ENTRYFIELD),
                                       50);

                if (WinProcessDlg(hwndDlg) == DID_OK)
                {
                    // initial data for file type: 1 null-byte,
                    // meaning that no associations have been defined...
                    // get new file type name from dlg
                    PSZ pszNewType;
                    if (pszNewType = winhQueryDlgItemText(hwndDlg,
                                                          ID_XSDI_FT_ENTRYFIELD))
                    {
                        if (!CreateFileType(pftpd,
                                            pszNewType,
                                            (PFILETYPERECORD)pnbp->preccSource))
                                                 // can be NULL
                        {
                            PCSZ pTable = pszNewType;
                            cmnMessageBoxExt(pnbp->hwndFrame,  // owner
                                                104,        // xwp error
                                                &pTable,
                                                1,
                                                196,
                                                MB_OK);
                        }
                    }
                }

                winhDestroyWindow(&hwndDlg);
            }

        }
        break;

        /*
         * ID_XSMI_FILETYPES_PICKUP:
         *      "Pickup file type" context menu item
         *      (file types container tree)
         */

        case ID_XSMI_FILETYPES_PICKUP:
        {
            if (    (pnbp->preccSource)
                 && (pnbp->preccSource != (PRECORDCORE)-1)
                 // lazy drag not currently in progress: V0.9.7 (2000-12-13) [umoeller]
                 && (!DrgQueryDragStatus())
               )
            {
                // initialize lazy drag just as if the
                // user had pressed Alt+MB2
                cnrhInitDrag(pftpd->hwndTypesCnr,
                             pnbp->preccSource,
                             CN_PICKUP,
                             DRAG_RMF,
                             DO_MOVEABLE);
            }
        }
        break;

        /*
         * ID_XSMI_FILETYPES_DROP:
         *      "Drop file type" context menu item
         *      (file types container tree)
         */

        case ID_XSMI_FILETYPES_DROP:
        {
            DrgLazyDrop(pftpd->hwndTypesCnr,
                        DO_MOVE,        // fixed V0.9.7 (2000-12-13) [umoeller]
                        &pnbp->ptlMenuMousePos);
                            // this is the pointer position at
                            // the time the context menu was
                            // requested (in Desktop coordinates),
                            // which should be over the target record
                            // core or container whitespace
        }
        break;

        /*
         * ID_XSMI_FILETYPES_CANCELDRAG:
         *      "Cancel drag" context menu item
         *      (file types container tree)
         */

        case ID_XSMI_FILETYPES_CANCELDRAG:
            // for one time, this is simple
            DrgCancelLazyDrag();
        break;

        /*
         * ID_XSDI_FT_FILTERSCNR:
         *      "File filters" container
         */

        case ID_XSDI_FT_FILTERSCNR:
        {
            switch (usNotifyCode)
            {
                /*
                 * CN_CONTEXTMENU:
                 *      ulExtra has the record core
                 */

                case CN_CONTEXTMENU:
                {
                    HWND    hPopupMenu;

                    // get drag status
                    // BOOL    fDragging = DrgQueryDragStatus();

                    // we store the container and recc.
                    // in the CREATENOTEBOOKPAGE structure
                    // so that the notebook.c function can
                    // remove source emphasis later automatically
                    pnbp->hwndSourceCnr = pnbp->hwndControl;
                    if (pnbp->preccSource = (PRECORDCORE)ulExtra)
                    {
                        // popup menu on container recc:
                        hPopupMenu = pftpd->hmenuFileFilterSel;
                    }
                    else
                    {
                        // no selection: different menu
                        hPopupMenu = pftpd->hmenuFileFilterNoSel;
                        // already open: disable
                        WinEnableMenuItem(hPopupMenu,
                                          ID_XSMI_FILEFILTER_IMPORTWPS,
                                          (pftpd->hwndWPSImportDlg == NULLHANDLE));
                    }

                    cnrhShowContextMenu(pnbp->hwndControl,
                                        (PRECORDCORE)pnbp->preccSource,
                                        hPopupMenu,
                                        pnbp->hwndDlgPage);    // owner
                }
                break;

            } // end switch (usNotifyCode)
        }
        break; // case ID_XSDI_FT_FILTERSCNR

        /*
         * ID_XSMI_FILEFILTER_DELETE:
         *      "Delete filter" context menu item
         *      (file filters container)
         */

        case ID_XSMI_FILEFILTER_DELETE:
        {
            ULONG       ulSelection = 0;

            PRECORDCORE precc = cnrhQuerySourceRecord(pftpd->hwndFiltersCnr,
                                                      pnbp->preccSource,
                                                      &ulSelection);

            while (precc)
            {
                PRECORDCORE preccNext = 0;

                if (ulSelection == SEL_MULTISEL)
                    // get next record first, because we can't query
                    // this after the record has been removed
                    preccNext = cnrhQueryNextSelectedRecord(pftpd->hwndFiltersCnr,
                                                        precc);

                WinSendMsg(pftpd->hwndFiltersCnr,
                           CM_REMOVERECORD,
                           (MPARAM)&precc,
                           MPFROM2SHORT(1,  // only one record
                                    CMA_FREE | CMA_INVALIDATE));

                precc = preccNext;
                // NULL if none
            }

            WriteXWPFilters2INI(pftpd);
        }
        break;

        /*
         * ID_XSMI_FILEFILTER_NEW:
         *      "New filter" context menu item
         *      (file filters container)
         */

        case ID_XSMI_FILEFILTER_NEW:
        {
            HWND hwndDlg;
            if (hwndDlg = cmnLoadDlg(pnbp->hwndFrame,  // owner
                                     WinDefDlgProc,
                                     ID_XSD_NEWFILTER, // "New Filter" dlg
                                     NULL))
            {
                WinSendDlgItemMsg(hwndDlg, ID_XSDI_FT_ENTRYFIELD,
                                    EM_SETTEXTLIMIT,
                                    (MPARAM)50,
                                    MPNULL);
                if (WinProcessDlg(hwndDlg) == DID_OK)
                {
                    CHAR szNewFilter[50];
                    // get new file type name from dlg
                    WinQueryDlgItemText(hwndDlg, ID_XSDI_FT_ENTRYFIELD,
                                    sizeof(szNewFilter)-1, szNewFilter);

                    AddFilter2Cnr(pftpd, szNewFilter);
                    WriteXWPFilters2INI(pftpd);
                }

                winhDestroyWindow(&hwndDlg);
            }
        }
        break;

        /*
         * ID_XSMI_FILEFILTER_IMPORTWPS:
         *      "Import filter" context menu item
         *      (file filters container)
         */

        case ID_XSMI_FILEFILTER_IMPORTWPS:
        {
            if (!pftpd->hwndWPSImportDlg)
            {
                // dialog not presently open:
                pftpd->hwndWPSImportDlg = cmnLoadDlg(pnbp->hwndFrame,  // owner
                                                     fnwpImportWPSFilters,
                                                     ID_XSD_IMPORTWPS, // "Import WPS Filters" dlg
                                                     pftpd);           // FILETYPESPAGEDATA for the dlg
            }
            WinShowWindow(pftpd->hwndWPSImportDlg, TRUE);
        }
        break;

        /*
         * ID_XSDI_FT_ASSOCSCNR:
         *      the "Associations" container, which can handle
         *      drag'n'drop.
         */

        case ID_XSDI_FT_ASSOCSCNR:
        {

            switch (usNotifyCode)
            {

                /*
                 * CN_INITDRAG:
                 *      file type being dragged (we don't support
                 *      lazy drag here, cos I'm too lazy)
                 */

                case CN_INITDRAG:
                {
                    PCNRDRAGINIT pcdi = (PCNRDRAGINIT)ulExtra;

                    if (pcdi)
                        // filter out whitespace
                        if (pcdi->pRecord)
                        {
                            // filter out disabled (parent) associations
                            if ((pcdi->pRecord->flRecordAttr & RECORD_DISABLED) == 0)
                                cnrhInitDrag(pcdi->hwndCnr,
                                             pcdi->pRecord,
                                             usNotifyCode,
                                             DRAG_RMF,
                                             DO_MOVEABLE);
                        }

                }
                break;  // case CN_INITDRAG

                /*
                 * CN_DRAGAFTER:
                 *      something's being dragged over the "Assocs" cnr;
                 *      we allow dropping only for single WPProgram and
                 *      WPProgramFile objects or if one of the associations
                 *      is dragged _within_ the container.
                 *
                 *      Note that since we have set CA_ORDEREDTARGETEMPH
                 *      for the "Assocs" cnr, we do not get CN_DRAGOVER,
                 *      but CN_DRAGAFTER only.
                 */

                case CN_DRAGAFTER:
                {
                    PCNRDRAGINFO pcdi = (PCNRDRAGINFO)ulExtra;
                    USHORT      usIndicator = DOR_NODROP,
                                    // cannot be dropped, but send
                                    // DM_DRAGOVER again
                                usOp = DO_UNKNOWN;
                                    // target-defined drop operation:
                                    // user operation (we don't want
                                    // the WPS to copy anything)

                    // reset global variable
                    pftpd->pobjDrop = NULL;

                    // OK so far:
                    // get access to the drag'n'drop structures
                    if (DrgAccessDraginfo(pcdi->pDragInfo))
                    {
                        if (
                                // accept no more than one single item at a time;
                                // we cannot move more than one file type
                                (pcdi->pDragInfo->cditem != 1)
                                // make sure that we only accept drops from ourselves,
                                // not from other windows
                            )
                        {
                            usIndicator = DOR_NEVERDROP;
                        }
                        else
                        {
                            // OK, we have exactly one item:

                            // 1)   is this a WPProgram or WPProgramFile from the WPS?
                            PDRAGITEM pdrgItem = DrgQueryDragitemPtr(pcdi->pDragInfo, 0);

                            if (DrgVerifyRMF(pdrgItem, "DRM_OBJECT", NULL))
                            {
                                // the WPS stores the MINIRECORDCORE of the
                                // object in ulItemID of the DRAGITEM structure;
                                // we use OBJECT_FROM_PREC to get the SOM pointer
                                WPObject *pObject = OBJECT_FROM_PREC(pdrgItem->ulItemID);

                                // dereference shadows
                                if (pObject = _xwpResolveIfLink(pObject))
                                {
                                    if (    (_somIsA(pObject, _WPProgram))
                                         || (_somIsA(pObject, _WPProgramFile))
                                       )
                                    {
                                        usIndicator = DOR_DROP;

                                        // store dragged object
                                        pftpd->pobjDrop = pObject;
                                        // store record after which to insert
                                        pftpd->preccAfter = pcdi->pRecord;
                                        if (pftpd->preccAfter == NULL)
                                            // if cnr whitespace: make last
                                            pftpd->preccAfter = (PRECORDCORE)CMA_END;
                                    }
                                }
                            } // end if (DrgVerifyRMF(pdrgItem, "DRM_OBJECT", NULL))

                            if (pftpd->pobjDrop == NULL)
                            {
                                // no object found yet:
                                // 2)   is this a record being dragged within our
                                //      container?
                                if (pcdi->pDragInfo->hwndSource != pftpd->hwndAssocsCnr)
                                    usIndicator = DOR_NEVERDROP;
                                else
                                {
                                    if (    (pcdi->pDragInfo->usOperation == DO_DEFAULT)
                                         || (pcdi->pDragInfo->usOperation == DO_MOVE)
                                       )
                                    {
                                        // do not allow drag upon whitespace,
                                        // but only between records
                                        if (pcdi->pRecord)
                                        {
                                            if (   (pcdi->pRecord == (PRECORDCORE)CMA_FIRST)
                                                || (pcdi->pRecord == (PRECORDCORE)CMA_LAST)
                                               )
                                                // allow drop
                                                usIndicator = DOR_DROP;

                                            // do not allow dropping after
                                            // disabled records
                                            else if ((pcdi->pRecord->flRecordAttr & RECORD_DISABLED) == 0)
                                                usIndicator = DOR_DROP;

                                            if (usIndicator == DOR_DROP)
                                            {
                                                pftpd->pobjDrop = (WPObject*)-1;
                                                        // ugly special flag: this is no
                                                        // object, but our own record core
                                                // store record after which to insert
                                                pftpd->preccAfter = pcdi->pRecord;

                                                // OH MY LORD...
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        DrgFreeDraginfo(pcdi->pDragInfo);
                    }

                    // and return the drop flags
                    mrc = (MRFROM2SHORT(usIndicator, usOp));
                }
                break;

                /*
                 * CN_DROP:
                 *      something _has_ now been dropped on the cnr.
                 */

                case CN_DROP:
                {
                    // check the global variable which has been set
                    // by CN_DRAGOVER above:
                    if (pftpd->pobjDrop)
                    {
                        // CN_DRAGOVER above has considered this valid:

                        if (pftpd->pobjDrop == (WPObject*)-1)
                        {
                            // special flag for record being dragged within
                            // the container:

                            // get access to the drag'n'drop structures
                            PCNRDRAGINFO pcdi = (PCNRDRAGINFO)ulExtra;
                            if (DrgAccessDraginfo(pcdi->pDragInfo))
                            {
                                // OK, move the record core tree to the
                                // new location.

                                // This is a bit unusual, because normally
                                // it is the source application which does
                                // the operation as a result of d'n'd. But
                                // who cares, source and target are the
                                // same window here anyway, so let's go.
                                PDRAGITEM   pdrgItem = DrgQueryDragitemPtr(pcdi->pDragInfo, 0);

                                // update container
                                PRECORDCORE preccMove = (PRECORDCORE)pdrgItem->ulItemID;
                                cnrhMoveRecord(pftpd->hwndAssocsCnr,
                                               preccMove,
                                               pftpd->preccAfter);

                                DrgFreeDraginfo(pcdi->pDragInfo);
                                            // V0.9.7 (2000-12-10) [umoeller]
                            }
                        }
                        else
                        {
                            // WPProgram or WPProgramFile:
                            AddAssocObject2Cnr(pftpd->hwndAssocsCnr,
                                               pftpd->pobjDrop,
                                               pftpd->preccAfter,
                                                    // record to insert after; has been set by CN_DRAGOVER
                                               TRUE);
                                                    // enable record
                            cnrhInvalidateAll(pftpd->hwndAssocsCnr);
                        }

                        // write the new associations to INI
                        WriteAssocs2INI((PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE",
                                        pftpd->hwndTypesCnr,
                                        pftpd->hwndAssocsCnr);
                        pftpd->pobjDrop = NULL;
                    }
                }
                break;

                /*
                 * CN_CONTEXTMENU ("Associations" container):
                 *      ulExtra has the record core
                 */

                case CN_CONTEXTMENU:
                {
                    HWND    hPopupMenu;
                    // we store the container and recc.
                    // in the CREATENOTEBOOKPAGE structure
                    // so that the notebook.c function can
                    // remove source emphasis later automatically
                    pnbp->hwndSourceCnr = pnbp->hwndControl;
                    if (pnbp->preccSource = (PRECORDCORE)ulExtra)
                    {
                        // popup menu on record core:
                        hPopupMenu = pftpd->hmenuFileAssocSel;
                        // association from parent file type:
                        // do not allow deletion
                        WinEnableMenuItem(hPopupMenu,
                                          ID_XSMI_FILEASSOC_DELETE,
                                          ((pnbp->preccSource->flRecordAttr & CRA_DISABLED) == 0));
                    }
                    else
                    {
                        // on whitespace: different menu
                        hPopupMenu = pftpd->hmenuFileAssocNoSel;

                        // already open: disable
                        WinEnableMenuItem(hPopupMenu,
                                          ID_XSMI_FILEFILTER_IMPORTWPS,
                                          (pftpd->hwndWPSImportDlg == NULLHANDLE));
                    }

                    cnrhShowContextMenu(pnbp->hwndControl,
                                        (PRECORDCORE)pnbp->preccSource,
                                        hPopupMenu,
                                        pnbp->hwndDlgPage);    // owner
                }
                break;
            } // end switch (usNotifyCode)
        }
        break;

        /*
         * ID_XSMI_FILEASSOC_DELETE:
         *      "Delete association" context menu item
         *      (file association container)
         */

        case ID_XSMI_FILEASSOC_DELETE:
        {
            ULONG       ulSelection = 0;

            PRECORDCORE precc = cnrhQuerySourceRecord(pftpd->hwndAssocsCnr,
                                                      pnbp->preccSource,
                                                      &ulSelection);

            while (precc)
            {
                PRECORDCORE preccNext = 0;

                if (ulSelection == SEL_MULTISEL)
                    // get next record first, because we can't query
                    // this after the record has been removed
                    preccNext = cnrhQueryNextSelectedRecord(pftpd->hwndAssocsCnr,
                                                        precc);

                WinSendMsg(pftpd->hwndAssocsCnr,
                           CM_REMOVERECORD,
                           (MPARAM)&precc,
                           MPFROM2SHORT(1,  // only one record
                                    CMA_FREE | CMA_INVALIDATE));

                precc = preccNext;
                // NULL if none
            }

            // write the new associations to INI
            WriteAssocs2INI((PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE",
                            pftpd->hwndTypesCnr,
                            pftpd->hwndAssocsCnr);
        }
        break;

        case ID_XSMI_FILEASSOC_SETTINGS:        // V0.9.16 (2001-09-29) [umoeller]
        case ID_XSMI_FILEASSOC_OPENFDR:         // V0.9.16 (2001-09-29) [umoeller]
        {
            ULONG ulSelection;
            PRECORDCORE precc = cnrhQuerySourceRecord(pftpd->hwndAssocsCnr,
                                                      pnbp->preccSource,
                                                      &ulSelection);

            while (precc)
            {
                PRECORDCORE preccNext = 0;

                HOBJECT hobj = ((PASSOCRECORD)precc)->hobj;
                WPObject *pobjAssoc;

                if (ulSelection == SEL_MULTISEL)
                    // get next record first, because we can't query
                    // this after the record has been removed
                    preccNext = cnrhQueryNextSelectedRecord(pftpd->hwndAssocsCnr,
                                                            precc);

                if (    (hobj)
                     && (pobjAssoc = objFindObjFromHandle(hobj))
                   )
                {
                    switch (ulItemID)
                    {
                        case ID_XSMI_FILEASSOC_SETTINGS:
                            _wpViewObject(pobjAssoc,
                                          NULLHANDLE,
                                          OPEN_SETTINGS,
                                          0);
                        break;

                        case ID_XSMI_FILEASSOC_OPENFDR:
                        {
                            WPFolder *pfdr;
                            if (pfdr = _wpQueryFolder(pobjAssoc))
                                _wpViewObject(pfdr,
                                              NULLHANDLE,
                                              OPEN_DEFAULT,
                                              0);
                        }
                        break;
                    }
                }

                precc = preccNext;
                // NULL if none
            }
        }
        break;

        /*
         * ID_XSMI_FILETYPES_IMPORT:
         *      "export file types" menu item.
         */

        case ID_XSMI_FILETYPES_IMPORT:
            ImportNewTypes(pnbp);
        break;

        /*
         * ID_XSMI_FILETYPES_EXPORT:
         *      "export file types" menu item.
         */

        case ID_XSMI_FILETYPES_EXPORT:
        {
            CHAR szFilename[CCHMAXPATH];
            sprintf(szFilename, "%c:\\xwptypes.xtp", doshQueryBootDrive());
            if (cmnFileDlg(pnbp->hwndDlgPage,
                           szFilename,
                           WINH_FOD_SAVEDLG | WINH_FOD_INILOADDIR | WINH_FOD_INISAVEDIR,
                           HINI_USER,
                           INIAPP_XWORKPLACE,
                           "XWPFileTypesDlg"))
            {
                // check if file exists
                CHAR szError[30];
                PCSZ apsz[2] = { szFilename, szError };

                if (    (access(szFilename, 0) != 0)
                        // confirm if file exists
                     || (cmnMessageBoxExt(pnbp->hwndDlgPage,
                                             121,            // xwp
                                             apsz,
                                             1,
                                             217,            // %1 exists
                                             MB_YESNO | MB_DEFBUTTON2)
                            == MBID_YES)
                   )
                {
                    // create XML document then
                    HPOINTER hptrOld = winhSetWaitPointer();
                    APIRET arc = ftypExportTypes(szFilename);
                    WinSetPointer(HWND_DESKTOP, hptrOld);

                    if (!arc)
                        cmnMessageBoxExt(pnbp->hwndDlgPage,
                                            121,            // xwp
                                            apsz,
                                            1,
                                            213,            // successfully exported to %1
                                            MB_OK);
                    else
                    {
                        sprintf(szError, "%u", arc);
                        cmnMessageBoxExt(pnbp->hwndDlgPage,
                                            104,            // xwp: error
                                            apsz,
                                            2,
                                            214,            // error %2 exporting to %1
                                            MB_OK);
                    }
                }
            }
        }
        break;
    }

    return mrc;
}

/*
 *@@ FillListboxWithWPSFilters:
 *      fills the "filters" listbox in the "import" dlg
 *      with the wps filters.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 *@@changed V1.0.5 (2006-05-15) [pr]: Fix WPS filters missing on Import @@fixes 399
 */

STATIC VOID FillListboxWithWPSFilters(HWND hwndDlg)
{
    HPOINTER hptrOld = winhSetWaitPointer();

    HWND        hwndListBox = WinWindowFromID(hwndDlg,
                                              ID_XSDI_FT_FILTERLIST);

    BOOL        fUnknownOnly = winhIsDlgItemChecked(hwndDlg,
                                                    ID_XSDI_FT_UNKNOWNONLY);

    // PLINKLIST   pllXWPTypesWithFilters = ftypGetCachedTypesWithFilters();

    APIRET      arc;

    PSZ         pszAssocsList = NULL,
                pszTypesWithFiltersList = NULL;

    WinSendMsg(hwndListBox,
               LM_DELETEALL,
               0, 0);

    arc = prfhQueryKeysForApp(HINI_USER,
                              INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                              &pszTypesWithFiltersList);
    if (arc == PRFERR_KEYSLIST)	 // V1.0.5 (2006-05-15) [pr]
        arc = NO_ERROR;

    if (!arc)
    {
        if (!(arc = prfhQueryKeysForApp(HINI_USER,
                                        WPINIAPP_ASSOCFILTER, // "PMWP_ASSOC_FILTER"
                                        &pszAssocsList)))
        {
            PSZ         pAssocThis = pszAssocsList;
            // add each WPS filter to the list box
            while (*pAssocThis != 0)
            {
                BOOL    fInsert = TRUE;
                ULONG   ulSize = 0;
                // insert the list box item only if
                // a) associations have been defined _and_
                // b) the filter has not been assigned to a
                //    file type yet

                // a) check WPS filters
                if (!PrfQueryProfileSize(HINI_USER,
                                         (PSZ)WPINIAPP_ASSOCFILTER,  // "PMWP_ASSOC_FILTER"
                                         pAssocThis,
                                         &ulSize))
                    fInsert = FALSE;
                else if (ulSize < 2)
                    // a length of 1 is the NULL byte == no assocs
                    fInsert = FALSE;

                if ((fInsert) && (fUnknownOnly))
                {
                    // b) now check XWorkplace filters
                    // V0.9.9 (2001-02-06) [umoeller]

                    PSZ pTypeWithFilterThis = pszTypesWithFiltersList;

                    // V1.0.5 (2006-05-15) [pr]
                    while (pTypeWithFilterThis && (*pTypeWithFilterThis) && (fInsert))
                    {
                        // pFilterThis has the current type now
                        // (e.g. "C Code");
                        // get filters for that (e.g. "*.c");
                        // this is another list of null-terminated strings
                        ULONG ulTypeLength;
                        if (ulTypeLength = strlen(pTypeWithFilterThis))
                        {
                            ULONG cbFiltersForTypeList = 0;     // including null byte
                            PSZ pszFiltersForTypeList;
                            if (pszFiltersForTypeList = prfhQueryProfileData(HINI_USER,
                                                                             INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                                                                             pTypeWithFilterThis,
                                                                             &cbFiltersForTypeList))
                            {
                                // second loop: thru all filters for this file type
                                PSZ pFilterThis = pszFiltersForTypeList;
                                while (*pFilterThis != 0)
                                {
                                    // check if this matches the data file name
                                    if (!strcmp(pFilterThis, pAssocThis))
                                    {
                                        fInsert = FALSE;
                                        break;
                                    }

                                    pFilterThis += strlen(pFilterThis) + 1;   // next type/filter
                                } // end while (*pFilterThis)

                                free(pszFiltersForTypeList);
                            }
                        }

                        pTypeWithFilterThis += ulTypeLength + 1;   // next type/filter
                    } // end while (*pTypeWithFilterThis != 0)
                }

                if (fInsert)
                    WinInsertLboxItem(hwndListBox,
                                      LIT_SORTASCENDING,
                                      pAssocThis);

                // go for next key
                pAssocThis += strlen(pAssocThis)+1;
            }

            free(pszAssocsList);

        } // end if (pszAssocsList)

        free(pszTypesWithFiltersList);
    }

    WinSetPointer(HWND_DESKTOP, hptrOld);
}

/*
 * fnwpImportWPSFilters:
 *      dialog procedure for the "Import WPS Filters" dialog,
 *      which gets loaded upon ID_XSMI_FILEFILTER_IMPORTWPS from
 *      ftypFileTypesItemChanged.
 *
 *      This needs the PFILETYPESPAGEDATA from the notebook
 *      page as a pCreateParam in WinLoadDlg (passed to
 *      WM_INITDLG here).
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.6 (2000-11-08) [umoeller]: fixed "Close" behavior
 *@@changed V0.9.9 (2001-02-06) [umoeller]: setting proper fonts now
 */

STATIC MRESULT EXPENTRY fnwpImportWPSFilters(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    PFILETYPESPAGEDATA pftpd = (PFILETYPESPAGEDATA)WinQueryWindowPtr(hwndDlg,
                                                                     QWL_USER);

    switch (msg)
    {
        /*
         * WM_INITDLG:
         *      mp2 has the PFILETYPESPAGEDATA
         */

        case WM_INITDLG:
        {
            pftpd = (PFILETYPESPAGEDATA)mp2;
            WinSetWindowPtr(hwndDlg, QWL_USER, pftpd);

            BEGIN_CNRINFO()
            {
                cnrhSetView(CV_TEXT | CA_ORDEREDTARGETEMPH);
            } END_CNRINFO(WinWindowFromID(hwndDlg,
                                          ID_XSDI_FT_ASSOCSCNR));

            cmnSetControlsFont(hwndDlg,
                               1,
                               3000);

            if (pftpd)
            {
                // 1) set the "Merge with" text to
                //    the selected record core

                WinSetDlgItemText(hwndDlg,
                                  ID_XSDI_FT_TYPE,
                                  pftpd->pftreccSelected->recc.recc.pszIcon);

                // 2) fill the left list box with the
                //    WPS-defined filters
                FillListboxWithWPSFilters(hwndDlg);

            } // end if (pftpd)

            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
        {
            USHORT usItemID = SHORT1FROMMP(mp1),
                   usNotifyCode = SHORT2FROMMP(mp1);

            switch (usItemID)
            {
                // "Filters" listbox
                case ID_XSDI_FT_FILTERLIST:
                {
                    if (usNotifyCode == LN_SELECT)
                    {
                        // selection changed on the left: fill right container

                        HWND    hwndFiltersListBox = WinWindowFromID(hwndDlg,
                                                              ID_XSDI_FT_FILTERLIST),
                                hwndAssocsCnr     =  WinWindowFromID(hwndDlg,
                                                              ID_XSDI_FT_ASSOCSCNR);
                        SHORT   sItemSelected = LIT_FIRST;
                        CHAR    szFilterText[200];
                        BOOL    fFirstRun = TRUE;

                        HPOINTER hptrOld = winhSetWaitPointer();

                        // OK, something has been selected in
                        // the "Filters" list box.
                        // Since we support multiple selections,
                        // we go thru all the selected items
                        // and add the corresponding filters
                        // to the right container.

                        // WinEnableWindowUpdate(hwndAssocsListBox, FALSE);

                        while (TRUE)
                        {
                            sItemSelected = (SHORT)WinSendMsg(hwndFiltersListBox,
                                       LM_QUERYSELECTION,
                                       (MPARAM)sItemSelected,  // initially LIT_FIRST
                                       MPNULL);

                            if (sItemSelected == LIT_NONE)
                                // no more selections:
                                break;
                            else
                            {
                                WinSendMsg(hwndFiltersListBox,
                                           LM_QUERYITEMTEXT,
                                           MPFROM2SHORT(sItemSelected,
                                                    sizeof(szFilterText)-1),
                                           (MPARAM)szFilterText);

                                UpdateAssocsCnr(hwndAssocsCnr,
                                                        // cnr to update
                                                szFilterText,
                                                        // file filter to search for
                                                (PSZ)WPINIAPP_ASSOCFILTER,
                                                        // "PMWP_ASSOC_FILTER"
                                                fFirstRun,
                                                        // empty container the first time
                                                TRUE);
                                                        // enable all records

                                // winhLboxSelectAll(hwndAssocsListBox, TRUE);

                                fFirstRun = FALSE;
                            }
                        } // end while (TRUE);

                        WinSetPointer(HWND_DESKTOP, hptrOld);
                    }
                }
                break;

                case ID_XSDI_FT_UNKNOWNONLY:
                    if (    (usNotifyCode == BN_CLICKED)
                         || (usNotifyCode == BN_DBLCLICKED) // added V0.9.0
                       )
                        // refresh the filters listbox
                        FillListboxWithWPSFilters(hwndDlg);
                break;
            } // end switch (usItemID)

        }
        break; // case case WM_CONTROL

        /*
         * WM_COMMAND:
         *      process buttons pressed
         */

        case WM_COMMAND:
        {
            SHORT usCmd = SHORT1FROMMP(mp1);

            switch (usCmd)
            {
                /*
                 * DID_OK:
                 *      "Merge" button
                 */

                case DID_OK:
                {
                    HWND    hwndFiltersListBox = WinWindowFromID(hwndDlg,
                                                          ID_XSDI_FT_FILTERLIST),
                            hwndAssocsCnr     =  WinWindowFromID(hwndDlg,
                                                          ID_XSDI_FT_ASSOCSCNR);
                    SHORT   sItemSelected = LIT_FIRST;
                    CHAR    szFilterText[200];
                    PASSOCRECORD preccThis = (PASSOCRECORD)CMA_FIRST;

                    // 1)  copy the selected filters in the
                    //     "Filters" listbox to the "Filters"
                    //     container on the notebook page

                    while (TRUE)
                    {
                        sItemSelected = (SHORT)WinSendMsg(hwndFiltersListBox,
                                   LM_QUERYSELECTION,
                                   (MPARAM)sItemSelected,  // initially LIT_FIRST
                                   MPNULL);

                        if (sItemSelected == LIT_NONE)
                            break;
                        else
                        {
                            // copy filter from list box to container
                            WinSendMsg(hwndFiltersListBox,
                                       LM_QUERYITEMTEXT,
                                       MPFROM2SHORT(sItemSelected,
                                                sizeof(szFilterText)-1),
                                       (MPARAM)szFilterText);
                            AddFilter2Cnr(pftpd, szFilterText);
                        }
                    } // end while (TRUE);

                    // write the new filters to OS2.INI
                    WriteXWPFilters2INI(pftpd);

                    // 2)  copy all the selected container items from
                    //     the "Associations" container in the dialog
                    //     to the one on the notebook page

                    while (TRUE)
                    {
                        // get first or next selected record
                        preccThis = (PASSOCRECORD)WinSendMsg(hwndAssocsCnr,
                                            CM_QUERYRECORDEMPHASIS,
                                            (MPARAM)preccThis,
                                            (MPARAM)CRA_SELECTED);

                        if ((preccThis == 0) || ((LONG)preccThis == -1))
                            // last or error
                            break;
                        else
                        {
                            // get object from handle
                            WPObject *pobjAssoc = objFindObjFromHandle(preccThis->hobj);
                                            // V0.9.9 (2001-04-02) [umoeller]

                            if (pobjAssoc)
                                // add the object to the notebook list box
                                AddAssocObject2Cnr(pftpd->hwndAssocsCnr,
                                                   pobjAssoc,
                                                   (PRECORDCORE)CMA_END,
                                                   TRUE);
                        }
                    } // end while (TRUE);

                    cnrhInvalidateAll(pftpd->hwndAssocsCnr);

                    // write the new associations to INI
                    WriteAssocs2INI((PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE",
                                    pftpd->hwndTypesCnr,
                                        // for selected record core
                                    pftpd->hwndAssocsCnr);
                                        // from updated list box
                }
                break; // case DID_OK

                case ID_XSDI_FT_SELALL:
                case ID_XSDI_FT_DESELALL:
                    cnrhSelectAll(WinWindowFromID(hwndDlg,
                                                  ID_XSDI_FT_ASSOCSCNR),
                                     (usCmd == ID_XSDI_FT_SELALL)); // TRUE = select
                break;

                case DID_CANCEL:
                    // "close" button:
                    WinPostMsg(hwndDlg,
                               WM_SYSCOMMAND,
                               (MPARAM)SC_CLOSE,
                               MPFROM2SHORT(CMDSRC_PUSHBUTTON,
                                            TRUE));  // pointer (who cares)
                break;

                /* default:
                    // this includes "Close"
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2); */
            } // end switch (usCmd)

        }
        break; // case WM_CONTROL

        case WM_HELP:
            // display help using the "Workplace Shell" SOM object
            // (in CREATENOTEBOOKPAGE structure)
            cmnDisplayHelp(pftpd->pnbp->inbp.somSelf,
                           pftpd->pnbp->inbp.ulDefaultHelpPanel + 1);
                            // help panel which follows the one on the main page
        break;

        case WM_SYSCOMMAND:
            switch ((ULONG)mp1)
            {
                case SC_CLOSE:
                    winhDestroyWindow(&hwndDlg);
                    mrc = 0;
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case WM_DESTROY:
            pftpd->hwndWPSImportDlg = NULLHANDLE;
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/* ******************************************************************
 *
 *   Shared code for XFldDataFile, XWPProgramFile, XWPProgram
 *   file-type notebook pages
 *
 ********************************************************************/

/*
 *@@ G_ampDatafileTypesPage:
 *      resizing information for data file "File types" page.
 *      Stored in CREATENOTEBOOKPAGE of the
 *      respective "add notebook page" method.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 */

static MPARAM G_ampDatafileTypesPage[] =
    {
        MPFROM2SHORT(ID_XSDI_DATAF_AVAILABLE_CNR, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XSDI_DATAF_GROUP, XAC_SIZEX | XAC_SIZEY)
    };

extern MPARAM *G_pampDatafileTypesPage = G_ampDatafileTypesPage;
extern ULONG G_cDatafileTypesPage = ARRAYITEMCOUNT(G_ampDatafileTypesPage);

/*
 *@@ INSTANCEFILETYPESPAGE:
 *      notebook page structure for all the
 *      instance file-type pages
 *      (XFldDataFile, XWPProgramFile, XWPProgram).
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 */

typedef struct _INSTANCEFILETYPESPAGE
{
    // linked list of file types (linklist.c);
    // this contains FILETYPELISTITEM structs
    LINKLIST llAvailableTypes;           // auto-free

    // controls which are used all the time
    HWND    hwndTypesCnr;

    // backup of explicit types (for Undo)
    PSZ     pszTypesBackup;

} INSTANCEFILETYPESPAGE, *PINSTANCEFILETYPESPAGE;

/*
 *@@ InitInstanceFileTypesPage:
 *      common code for CBI_INIT.
 *
 *@@added V0.9.9 (2001-04-02) [umoeller]
 */

STATIC VOID InitInstanceFileTypesPage(PNOTEBOOKPAGE pnbp,
                                      PINSTANCEFILETYPESPAGE *pp)  // out: new struct
{
    PINSTANCEFILETYPESPAGE pdftp;
    if (pdftp = (PINSTANCEFILETYPESPAGE)malloc(sizeof(INSTANCEFILETYPESPAGE)))
    {
        memset(pdftp, 0, sizeof(*pdftp));
        pnbp->pUser = pdftp;

        lstInit(&pdftp->llAvailableTypes, TRUE);     // auto-free

        pdftp->hwndTypesCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                              ID_XSDI_DATAF_AVAILABLE_CNR);

        ctlMakeCheckboxContainer(pnbp->hwndDlgPage,
                                 ID_XSDI_DATAF_AVAILABLE_CNR);
                // this switches to tree view etc., but
                // we need to override some settings
        BEGIN_CNRINFO()
        {
            cnrhSetSortFunc(fnCompareName);
        } END_CNRINFO(pdftp->hwndTypesCnr);
    }

    *pp = pdftp;
}

/*
 *@@ FillInstanceFileTypesPage:
 *      common code for CBI_SET.
 *
 *@@added V0.9.9 (2001-04-02) [umoeller]
 */

STATIC VOID FillInstanceFileTypesPage(PNOTEBOOKPAGE pnbp,
                                      PCSZ pcszCheck,
                                      CHAR cSeparator,
                                      PLINKLIST pllDisable)
{
    PINSTANCEFILETYPESPAGE pdftp;
    if (pdftp = (PINSTANCEFILETYPESPAGE)pnbp->pUser)
    {
        // build list of explicit types to be passed
        // to FillCnrWithAvailableTypes; all the types
        // on this list will be CHECKED
        PLINKLIST pllExplicitTypes = NULL;

        if (pcszCheck)
        {
            pllExplicitTypes = lstCreate(TRUE);
            ftypAppendTypesFromString(pcszCheck,
                                      cSeparator,
                                      pllExplicitTypes);
        }

        // clear container and memory
        ftypClearTypesList(pdftp->hwndTypesCnr,
                           &pdftp->llAvailableTypes);

        // insert all records and check/disable them in one flush
        FillCnrWithAvailableTypes(pdftp->hwndTypesCnr,
                                  &pdftp->llAvailableTypes,
                                  pllExplicitTypes,  // check list
                                  pllDisable);        // disable list

        if (pllExplicitTypes)
            lstFree(&pllExplicitTypes);
    }
}

/*
 *@@ DestroyInstanceFileTypesPage:
 *      common code for CBI_DESTROY.
 *
 *@@added V0.9.9 (2001-04-02) [umoeller]
 */

STATIC VOID DestroyInstanceFileTypesPage(PNOTEBOOKPAGE pnbp)
{
    PINSTANCEFILETYPESPAGE pdftp;
    if (pdftp = (PINSTANCEFILETYPESPAGE)pnbp->pUser)
    {
        ftypClearTypesList(pdftp->hwndTypesCnr,
                           &pdftp->llAvailableTypes);

        if (pdftp->pszTypesBackup)
        {
            free(pdftp->pszTypesBackup);
            pdftp->pszTypesBackup = NULL;
        }
    }
}

/*
 *@@ HandleRecordChecked:
 *
 *@@added V0.9.9 (2001-04-02) [umoeller]
 *@@changed V0.9.12 (2001-05-12) [umoeller]: fixed buggy type removal
 */

STATIC VOID HandleRecordChecked(ULONG ulExtra,         // from "item changed" callback
                                PXSTRING pstrTypes,
                                PCSZ pcszSeparator)
{
    PFILETYPERECORD precc = (PFILETYPERECORD)ulExtra;

    if (precc->recc.usCheckState)
    {
        // checked -> type added:
        if (pstrTypes->ulLength)
        {
            // explicit types exist already:
            // append a new one
            PSZ pszNew;
            if (pszNew = (PSZ)malloc(   pstrTypes->ulLength
                                      + 1       // for \n
                                      + strlen(precc->pliFileType->pszFileType)
                                      + 1)) // for \0
            {
                ULONG ul = sprintf(pszNew,
                                   "%s%s%s",
                                   pstrTypes->psz,
                                   pcszSeparator,
                                   precc->pliFileType->pszFileType);

                xstrset2(pstrTypes, pszNew, ul);
            }
        }
        else
            // no explicit types yet:
            // just set this new type as the only one
            xstrcpy(pstrTypes,
                    precc->pliFileType->pszFileType,
                    0);
            // _wpSetType(pnbp->inbp.somSelf, precc->pliFileType->pszFileType, 0);
    }
    else
    {
        // unchecked -> type removed:
        if (pstrTypes->ulLength)
        {
            if (strchr(pstrTypes->psz, *pcszSeparator))
            {
                // we have more than one type:
                // build a linked list of the types now
                PLINKLIST   pllExplicitTypes = lstCreate(TRUE);
                PLISTNODE   pNode;
                XSTRING     strNew;
                ftypAppendTypesFromString(pstrTypes->psz,
                                          *pcszSeparator,
                                          pllExplicitTypes);
                for (pNode = lstQueryFirstNode(pllExplicitTypes);
                     pNode;
                     pNode = pNode->pNext)
                {
                    PSZ pszTypeThis = (PSZ)pNode->pItemData;
                    if (!strcmp(pszTypeThis, precc->pliFileType->pszFileType))
                    {
                        // alright, remove this one
                        lstRemoveNode(pllExplicitTypes, pNode);
                        break;
                    }
                }

                // construct a new types string from the types list
                xstrInit(&strNew, pstrTypes->ulLength);
                for (pNode = lstQueryFirstNode(pllExplicitTypes);
                     pNode;
                     pNode = pNode->pNext)
                {
                   if (strNew.ulLength)
                        // not first one:
                        xstrcatc(&strNew, *pcszSeparator);
                    xstrcat(&strNew, (PSZ)pNode->pItemData, 0);
                }

                // replace original string
                xstrcpys(pstrTypes, &strNew);
                xstrClear(&strNew);

                lstFree(&pllExplicitTypes);
            }
            else
                // we had only one type:
                xstrClear(pstrTypes);
        }
    }
}

/* ******************************************************************
 *
 *   XFldDataFile notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ fncbDatafileTypesInitPage:
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

BOOL _Optlink fncbDatafileTypesInitPage(PCSZ pcszType,
                                        ULONG ulTypeLen,
                                        PVOID pllTypes)
{
    // store the type (not the filter) in the output list
    ftypAppendSingleTypeUnique((PLINKLIST)pllTypes,
                               pcszType,
                               // length:
                               ulTypeLen);

    return TRUE;
}

/*
 *@@ ftypDatafileTypesInitPage:
 *      notebook callback function (notebook.c) for the
 *      "File types" page in a data file instance settings notebook.
 *      Initializes and fills the page.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 */

VOID ftypDatafileTypesInitPage(PNOTEBOOKPAGE pnbp,
                               ULONG flFlags)
{
    if (flFlags & CBI_INIT)
    {
        PINSTANCEFILETYPESPAGE pdftp = NULL;

        InitInstanceFileTypesPage(pnbp,
                                  &pdftp);

        if (pdftp)
        {
            // backup existing types for "Undo"
            pdftp->pszTypesBackup = strhdup(_wpQueryType(pnbp->inbp.somSelf), NULL);
        }
    }

    if (flFlags & CBI_SET)
    {
        PLINKLIST pllAutomaticTypes = lstCreate(TRUE);
        PSZ pszTypes = _wpQueryType(pnbp->inbp.somSelf);

        // build list of automatic types;
        // all these records will be DISABLED
        // in the container
        ftypForEachAutoType(_wpQueryTitle(pnbp->inbp.somSelf),
                            fncbDatafileTypesInitPage,
                            pllAutomaticTypes);

        FillInstanceFileTypesPage(pnbp,
                                  pszTypes,    // string with types to check
                                  '\n',         // separator char
                                  pllAutomaticTypes);    // items to disable

        lstFree(&pllAutomaticTypes);
    }

    if (flFlags & CBI_DESTROY)
    {
        DestroyInstanceFileTypesPage(pnbp);
    }
}

/*
 *@@ ftypDatafileTypesItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "File types" page in a data file instance settings notebook.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@added V0.9.9 (2001-03-27) [umoeller]
 *@@changed V0.9.16 (2001-12-08) [umoeller]: now refreshing icon on changes
 */

MRESULT ftypDatafileTypesItemChanged(PNOTEBOOKPAGE pnbp,
                                     ULONG ulItemID,
                                     USHORT usNotifyCode,
                                     ULONG ulExtra)
{
    MRESULT mrc = 0;

    switch (ulItemID)
    {
        case ID_XSDI_DATAF_AVAILABLE_CNR:
            if (usNotifyCode == CN_RECORDCHECKED)
            {
                // get existing types
                PSZ pszTypes = _wpQueryType(pnbp->inbp.somSelf);
                XSTRING str;
                xstrInitCopy(&str, pszTypes, 0);

                // modify the types according to record click
                HandleRecordChecked(ulExtra,         // from "item changed" callback
                                    &str,
                                    "\n");      // types separator

                // set new types
                _wpSetType(pnbp->inbp.somSelf,
                           (str.ulLength)
                                ? str.psz
                                : NULL,         // remove
                           0);
                xstrClear(&str);

                // refresh the icon, it might have changed
                // V0.9.16 (2001-12-08) [umoeller]
                _wpSetAssociatedFileIcon(pnbp->inbp.somSelf);
            }
        break;

        case DID_UNDO:
        {
            // V1.0.5 (2006-05-15) [pr]
            PINSTANCEFILETYPESPAGE pdftp = (PINSTANCEFILETYPESPAGE) pnbp->pUser;
            if (pdftp)
            {
                // set type to what was saved on init
                _wpSetType(pnbp->inbp.somSelf, pdftp->pszTypesBackup, 0);
                // call "init" callback to reinitialize the page
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
                // refresh the icon, it might have changed
                // V0.9.16 (2001-12-08) [umoeller]
                _wpSetAssociatedFileIcon(pnbp->inbp.somSelf);
            }
        }
        break;

        case DID_DEFAULT:
            // kill all explicit types
            _wpSetType(pnbp->inbp.somSelf, NULL, 0);
            // call "init" callback to reinitialize the page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            // refresh the icon, it might have changed
            // V0.9.16 (2001-12-08) [umoeller]
            _wpSetAssociatedFileIcon(pnbp->inbp.somSelf);
        break;

    } // end switch (ulItemID)

    return mrc;
}

/* ******************************************************************
 *
 *   XWPProgram/XWPProgramFile notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ ftypInsertAssociationsPage:
 *      inserts the "Associations" page into the
 *      given notebook.
 *
 *      Shared code between XWPProgramFile and XWPProgram.
 *
 *@@added V0.9.9 (2001-04-02) [umoeller]
 */

ULONG ftypInsertAssociationsPage(WPObject *somSelf, // in: WPProgram or WPProgramFile
                                 HWND hwndNotebook)
{
    INSERTNOTEBOOKPAGE inbp;

    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndNotebook;
    inbp.hmod = cmnQueryNLSModuleHandle(FALSE);
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.pcszName = cmnGetString(ID_XSSI_PGM_ASSOCIATIONS);  // pszAssociationsPage
    inbp.ulDlgID = ID_XSD_DATAF_TYPES;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_PGM_ASSOCIATIONS;
    inbp.ulPageID = SP_PGMFILE_ASSOCS;
    inbp.pampControlFlags = G_pampDatafileTypesPage;
    inbp.cControlFlags = G_cDatafileTypesPage;
    inbp.pfncbInitPage    = ftypAssociationsInitPage;
    inbp.pfncbItemChanged    = ftypAssociationsItemChanged;
    return ntbInsertPage(&inbp);
}

/*
 *@@ ftypAssociationsInitPage:
 *      notebook callback function (notebook.c) for the
 *      "Associations" page in program settings notebooks.
 *      Sets the controls on the page according to the
 *      file types and instance settings.
 *
 *      Note that this is shared between XWPProgram and
 *      XWPProgramFile.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

VOID ftypAssociationsInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                              ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        PINSTANCEFILETYPESPAGE pdftp = NULL;

        InitInstanceFileTypesPage(pnbp,
                                  &pdftp);

        if (pdftp)
        {
            // backup existing types for "Undo"
            pdftp->pszTypesBackup = strhdup(_wpQueryAssociationType(pnbp->inbp.somSelf), NULL);
        }
    }

    if (flFlags & CBI_SET)
    {
        PSZ pszTypes = _wpQueryAssociationType(pnbp->inbp.somSelf);

        FillInstanceFileTypesPage(pnbp,
                                  pszTypes,     // string with types to check
                                  ',',          // separator char
                                  NULL);        // items to disable
    }

    if (flFlags & CBI_DESTROY)
    {
        DestroyInstanceFileTypesPage(pnbp);
    }
}

/*
 *@@ ftypAssociationsItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "Associations" page in program settings notebooks.
 *      Reacts to changes of any of the dialog controls.
 *
 *      Note that this is shared between XWPProgram and
 *      XWPProgramFile.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 *@@changed V1.0.5 (2006-05-15) [pr]: Fixed crashes in Undo and Default
 */

MRESULT ftypAssociationsItemChanged(PNOTEBOOKPAGE pnbp,
                                    ULONG ulItemID,
                                    USHORT usNotifyCode,
                                    ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = 0;

    switch (ulItemID)
    {
        case ID_XSDI_DATAF_AVAILABLE_CNR:
            if (usNotifyCode == CN_RECORDCHECKED)
            {
                // get existing types
                PSZ pszTypes = _wpQueryAssociationType(pnbp->inbp.somSelf);
                                    // this works for both WPProgram and
                                    // WPProgramFile; even though the two
                                    // methods are differently implemented,
                                    // they both call the same implementation
                                    // in the WPS, so this is safe
                                    // (famous last words)
                XSTRING str;
                xstrInitCopy(&str, pszTypes, 0);

                PMPF_ASSOCS(("pre: str.psz is %s (ulLength: %u)",
                            (str.psz) ? str.psz : "NULL",
                            str.ulLength));

                // modify the types according to record click
                HandleRecordChecked(ulExtra,         // from "item changed" callback
                                    &str,
                                    ",");      // types separator

                PMPF_ASSOCS(("post: str.psz is %s (ulLength: %u)",
                            (str.psz) ? str.psz : "NULL",
                            str.ulLength));

                // set new types
                _wpSetAssociationType(pnbp->inbp.somSelf,
                                      (str.ulLength)
                                           ? str.psz
                                           // : NULL);         // remove
                                           : "");         // fixed V0.9.12 (2001-05-12) [umoeller]

                    // ^^^ @@todo, this never works with the null string

                xstrClear(&str);
            }
        break;

        case DID_UNDO:
        {
            // V1.0.5 (2006-05-15) [pr]
            PINSTANCEFILETYPESPAGE pdftp = (PINSTANCEFILETYPESPAGE) pnbp->pUser;
            if (pdftp)
            {
                // set type to what was saved on init
                _wpSetAssociationType(pnbp->inbp.somSelf, pdftp->pszTypesBackup);
                // call "init" callback to reinitialize the page
                pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            }
        }
        break;

        case DID_DEFAULT:
            // kill all explicit types
            // V1.0.5 (2006-05-15) [pr]: This doesn't work either, but at least
            // it doesn't crash now. @@todo ha, ha
            _wpSetAssociationType(pnbp->inbp.somSelf, "");
            // call "init" callback to reinitialize the page
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;

    } // end switch (ulItemID)

    return mrc;
}

#endif
