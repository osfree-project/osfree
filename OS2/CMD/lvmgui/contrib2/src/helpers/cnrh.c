
/*
 *@@sourcefile cnrh.c:
 *      contains various PM container helper functions.
 *
 *      These functions used to be in winh.c, but they became
 *      too many finally, so these were moved to this file with
 *      V0.9.0.
 *
 *      Usage: All PM programs.
 *
 *      Function prefixes:
 *      --  cnrh*   container helper functions (was: winh* before V0.9.0)
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@added V0.9.0 [umoeller]
 *@@header "helpers\cnrh.h"
 */

/*
 *      Copyright (C) 1997-2000 Ulrich M”ller.
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

#define INCL_WINWINDOWMGR
#define INCL_WINRECTANGLES
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINSYS
#define INCL_WINSTDDRAG
#define INCL_WINSTDCNR

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIPRIMITIVES
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/winh.h"
#include "helpers/cnrh.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Container helpers
 *      See cnrh.c.
 */

/*
 *@@category: Helpers\PM helpers\Container helpers\Details view helpers
 *      these functions aid you in setting up the FIELDINFO structures
 *      for a container Details view. Use cnrhSetFieldInfos for that.
 */

/* ******************************************************************
 *
 *   Details view field infos
 *
 ********************************************************************/

/*
 *@@ cnrhClearFieldInfos:
 *      removes all existing FIELDINFO's from
 *      the given container. Returns the number
 *      of remaining FIELDINFO's, which should
 *      be 0, or -1 upon errors.
 *
 *@@added V0.9.1 (2000-02-13) [umoeller]
 */

ULONG cnrhClearFieldInfos(HWND hwndCnr,
                          BOOL fInvalidate) // in: if TRUE, invalidate container
{
    ULONG ulFlags = CMA_FREE;
    if (fInvalidate)
        ulFlags |= CMA_INVALIDATE;

    return (ULONG)WinSendMsg(hwndCnr,
                             CM_REMOVEDETAILFIELDINFO,
                             (MPARAM)NULL,
                             MPFROM2SHORT(0,      // all
                                          ulFlags));
}

/*
 *@@ cnrhSetFieldInfo:
 *      this sets a FIELDINFO structure to the given
 *      data. Note that ppFieldInfo is a double pointer
 *      to the actual FIELDINFO data.
 *
 *      This gets called from cnrhSetFieldInfos to set
 *      each individual field info.
 *      You can also use this function separately with
 *      the cnrhAllocFieldInfos macro defined in cnrh.h.
 *
 *      After setting the data, <B>*ppFieldInfo</B> is advanced
 *      to the next FIELDINFO structure so that you can
 *      call this function several times for all the
 *      FIELDINFOS that you have allocated. After the last
 *      column, this pointer will be NULL.
 *
 *      Since the pointer is modified, do not invoke this
 *      function on the original pointer returned from
 *      cnrhAllocFieldInfos, because you'll need that
 *      pointer for cnrhAllocFieldInfos later.
 *
 *      <B>Example usage</B>:
 *
 +          PFIELDINFO pFieldInfoFirst, pFieldInfo2;
 +          if (pFieldInfoFirst = cnrhAllocFieldInfos(hwndFilesCnr, NO_OF_COLUMNS))
 +                          // macro defined in cnrh.h
 +          {
 +              pFieldInfo2 = pFieldInfoFirst;
 +
 +              // "File name" column
 +              cnrhSetFieldInfo(&pFieldInfo2,
 +                                  FIELDOFFSET(RECORDCORE, pszIcon),
 +                                      // icon text offset in original RECORDCORE
 +                                  "File name",
 +                                  CFA_STRING,
 +                                  CFA_LEFT,
 +                                  FALSE);     // no draw lines
 +              // "Size" column
 +              cnrhSetFieldInfo(&pFieldInfo2,
 +                                  FIELDOFFSET(FILERECORDCORE, ulSize),
 +                                      // size data field in sample extended RECORDCORE
 +                                  "Size",
 +                                  CFA_ULONG,
 +                                  CFA_RIGHT,
 +                                  FALSE);     // no draw lines
 +              ... // set other field infos
 +          }
 +
 +          // insert field infos
 +          cnrhInsertFieldInfos(hwndFilesCnr,
 +                                  pFieldInfoFirst,
 +                                  NO_OF_COLUMNS);
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (99-12-18) [umoeller]: fixed memory leak
 */

VOID cnrhSetFieldInfo(PFIELDINFO *ppFieldInfo2,  // in/out: double ptr to FIELDINFO
                      ULONG ulFieldOffset,       // in: FIELDOFFSET(YOURRECORDCORE, yourField)
                      PSZ pszColumnTitle,        // in: column title; this must be a static string!!
                      ULONG ulDataType,          // in: column data type (CFA_* flags)
                      ULONG ulOrientation,       // in: vertical and horizontal orientation (CFA_* flags)
                      BOOL fDrawLines)           // in: if TRUE, we'll draw lines around the columns
{
    PFIELDINFO pfi;
    if (    (ppFieldInfo2)
         && (pfi = *ppFieldInfo2)
       )
    {
        ULONG flData = ulDataType | ulOrientation;
        if (fDrawLines)
            flData |= CFA_HORZSEPARATOR | CFA_SEPARATOR;

        pfi->cb = sizeof(FIELDINFO);
        pfi->flData = flData;
        pfi->flTitle = CFA_FITITLEREADONLY | ulOrientation;
        pfi->offStruct = ulFieldOffset;
        pfi->pTitleData = pszColumnTitle;   // strdup removed, V0.9.1 (99-12-18) [umoeller]
        pfi->pUserData   = NULL;
        pfi->cxWidth = 0;
        *ppFieldInfo2 = pfi->pNextFieldInfo;
    }
}

/*
 *@@ cnrhInsertFieldInfos:
 *      this inserts field infos for Details view
 *      into the specified container.
 *
 *      pFieldInfoFirst should be the PFIELDINFO
 *      returned by cnrhAllocFieldInfos.
 *
 *      This inserts the FIELDINFOs at the end,
 *      should any columns already exist in the container.
 *      Also, the container is invalidated.
 *
 *      Returns the return value of CM_INSERTDETAILFIELDINFO,
 *      which is the total no. of field infos in the container
 *      or null upon errors.
 *
 *@@added V0.9.0 [umoeller]
 */

ULONG cnrhInsertFieldInfos(HWND hwndCnr,                // in: cnr for Details view
                           PFIELDINFO pFieldInfoFirst,  // in: first field info as returned
                                                        // by cnrhAllocFieldInfos
                           ULONG ulFieldCount)          // in: no. of field infos
{
    FIELDINFOINSERT fii;
    fii.cb = sizeof(FIELDINFOINSERT);
    fii.pFieldInfoOrder = (PFIELDINFO)CMA_END;
    fii.fInvalidateFieldInfo = TRUE;
    fii.cFieldInfoInsert = ulFieldCount;

    return (ULONG)WinSendMsg(hwndCnr,
                             CM_INSERTDETAILFIELDINFO,
                             (MPARAM)pFieldInfoFirst,
                             (MPARAM)&fii);
}

/*
 *@@ cnrhSetFieldInfos:
 *      this combines cnrhAllocFieldInfos,
 *      cnrhSetFieldInfo, and cnrhInsertFieldInfos
 *      into a one-shot func for setting details view
 *      column field infos. This one has proven to
 *      be EXTREMELY useful.
 *
 *      To pass all the arguments normally passed to
 *      cnrhSetFieldInfo, we use an array of XFIELDINFO
 *      structures, which takes the same parameters.
 *
 *      <B>XFIELDINFO.ulDataType</B> specifies the data type of the record
 *      core field. This can be one of the following:
 *      --  CFA_BITMAPORICON: bit-map or icon data
 *      --  CFA_DATE: date format (CDATE structure)
 *      --  CFA_STRING: null-terminated string
 *      --  CFA_TIME: time format (CTIME structure)
 *      --  CFA_ULONG: unsigned number data
 *
 *      You can add the following optional flags to
 *      ulDataType:
 *      --  CFA_FIREADONLY (CFA_STRING only): disable editing
 *      --  CFA_INVISIBLE: make column invisible
 *      --  CFA_OWNER: enable owner draw for this column
 *
 *      If (fDrawLines == TRUE), we'll automatically add
 *      CFA_HORZSEPARATOR | CFA_SEPARATOR.
 *
 *      <B>XFIELDINFO.ulOrientation</B> specifies the vertical and
 *      horizontal orientation of both the column title
 *      and the column data. This should be OR'ed from
 *      the following
 *      --  CFA_CENTER, CFA_LEFT, CFA_RIGHT (horizontal);
 *          the default is CFA_LEFT
 *      --  CFA_BOTTOM, CFA_TOP, CFA_VCENTER (vertical);
 *          the default is CFA_VCENTER.
 *
 *      Note that the container automatically displays
 *      data according to the settings in the "Country" object.
 *
 *      The column title will always be set to string format
 *      and CFA_FITITLEREADONLY.
 *
 *      <B>XFIELDINFO.ulFieldOffset</B> should be set to the return value
 *      of the FIELDOFFSET macro, which is also redefined in
 *      cnrh.h (to work with C++).
 *
 *      The return value is the PFIELDINFO which corresponds
 *      to the column index specified in ulFieldReturn, or
 *      NULL upon errors. This is useful for setting the
 *      position of the split bar afterwards (using the
 *      cnrhSetSplitBarAfter macro).
 *
 *      Example usage:
 +          XFIELDINFO      xfi[3];
 +          PFIELDINFO      pfi;
 +
 +          xfi[0].ulFieldOffset = FIELDOFFSET(DATABASERECORD, pszApplication);
 +          xfi[0].pszColumnTitle = "Application";
 +          xfi[0].ulDataType = CFA_STRING;
 +          xfi[0].ulOrientation = CFA_LEFT;
 +
 +          xfi[1].ulFieldOffset = FIELDOFFSET(RECORDCORE, pszIcon);
 +          xfi[1].pszColumnTitle = "Package name";
 +          xfi[1].ulDataType = CFA_STRING;
 +          xfi[1].ulOrientation = CFA_LEFT;
 +
 +          xfi[2].ulFieldOffset = FIELDOFFSET(DATABASERECORD, pszAuthor);
 +          xfi[2].pszColumnTitle = "Author";
 +          xfi[2].ulDataType = CFA_STRING;
 +          xfi[2].ulOrientation = CFA_CENTER;
 +
 +          pfi = cnrhSetFieldInfos(hwndCnr,
 +                                  &xfi[0],
 +                                  (sizeof(xfi) / sizeof(XFIELDINFO)),
 +                                      // smart way of calculating the array item count
 +                                  FALSE,         // no draw lines
 +                                  0);            // return first column
 *
 *@@added V0.9.0 [umoeller]
 */

PFIELDINFO cnrhSetFieldInfos(HWND hwndCnr,            // in: container hwnd
                             PXFIELDINFO paxfi,       // in: pointer to an array of ulFieldCount XFIELDINFO structures
                             ULONG ulFieldCount,      // in: no. of items in paxfi array (> 0)
                             BOOL fDrawLines,         // in: if TRUE, we'll draw lines around the columns
                             ULONG ulFieldReturn)     // in: the column index to return as PFIELDINFO
{
    PFIELDINFO  pFieldInfoFirst,
                pFieldInfo2,
                pFieldInfoReturn = NULL;

    if ((pFieldInfoFirst = cnrhAllocFieldInfos(hwndCnr, ulFieldCount)))
    {
        ULONG ul = 0;
        PXFIELDINFO pxfi = NULL;

        pFieldInfo2 = pFieldInfoFirst;
        pxfi = paxfi;
        for (ul = 0; ul < ulFieldCount; ul++)
        {
            if (ul == ulFieldReturn)
                // set return value
                pFieldInfoReturn = pFieldInfo2;

            // set current field info;
            // this will modify pFieldInfo to point to the next
            cnrhSetFieldInfo(&pFieldInfo2,
                             pxfi->ulFieldOffset,
                             (PSZ)pxfi->pszColumnTitle,
                             pxfi->ulDataType,
                             pxfi->ulOrientation,
                             fDrawLines);
            pxfi++;
        }

        // insert field infos
        if (!cnrhInsertFieldInfos(hwndCnr,
                                  pFieldInfoFirst,
                                  ulFieldCount))
            pFieldInfoReturn = NULL;
    }

    return pFieldInfoReturn;
}

/*
 *@@category: Helpers\PM helpers\Container helpers\Record core helpers
 *      functions for allocating and inserting records more easily.
 *
 *      Use cnrhAllocRecords first to allocate, then cnrhInsertRecords
 *      to insert the records.
 */

/* ******************************************************************
 *
 *   Record core management
 *
 ********************************************************************/

/*
 *@@ cnrhAllocRecords:
 *      this is a shortcut to allocating memory for
 *      container record cores.
 *
 *      If (ulCount == 1), this returns the new record core.
 *      If (ulCount > 1), this returns the _first_ record
 *      core; the following record cores may be reached
 *      by following the RECORDCORE.preccNextRecord pointers
 *      (i.e. we have a linked list here).
 *
 *      Returns NULL on errors (what CM_ALLOCRECORD returns).
 *
 *      The record cores returned by the container are
 *      automatically zeroed out, and their "cb" field
 *      is automatically set to the size of the record core.
 *
 *      Note that this function presently does _not_ work
 *      with MINIRECORDCOREs.
 *
 *@@changed V0.9.0 [umoeller]: function prototype changed to allocate more than one record
 */

PRECORDCORE cnrhAllocRecords(HWND hwndCnr,    // in: cnr to allocate from
                             ULONG cbrecc,
                                 // in: total size of your record core.
                                 // If you're using the default recc's, this
                                 // must be sizeof(RECORDCORE).
                             ULONG ulCount)  // in: number of records to allocate (> 0)
{
    return (PRECORDCORE)WinSendMsg(hwndCnr,
                                   CM_ALLOCRECORD,
                                   (MPARAM)(cbrecc - sizeof(RECORDCORE)),
                                   (MPARAM)ulCount);
}

/*
 *@@ cnrhInsertRecords:
 *      shortcut to inserting record cores into a container
 *      which have been allocated using cnrhAllocRecords.
 *
 *      If (<B>ulCount</B> == 1), this inserts precc.
 *      If (ulCount > 1), this assumes precc to be the first
 *      record to be inserted, while precc->preccNextRecord
 *      must point to the next record in a linked list (as
 *      returned with cnrhAllocRecords). (changed V0.9.0)
 *
 *      Note that ulCount here must be exactly the same as
 *      specified with cnrhAllocRecords.
 *
 *      If (<B>pszText</B> != NULL), this will automatically set precc->pszIcon,
 *      precc->pszText, precc->pszName, precc->pszTree to pszText to have
 *      the same record titles in all views. If (pszText == NULL), those
 *      fields will be left alone. (changed V0.9.0)
 *
 *      <B>flRecordAttr</B> should have the record attributes as
 *      specified in the RECORDCORE structure.
 *
 *      Emphasis flags:
 *      --  CRA_SELECTED:      record is selected
 *                             (other records will be deselected in Tree views
 *                             and when single selection is enabled)
 *      --  CRA_CURSORED:      cursor (keyboard focus) is on the record
 *                             (other records will be de-cursored)
 *      --  CRA_SOURCE:        record has source emphasis (drag'n'drop,
 *                             open context menus)
 *      --  CRA_TARGET:        record has target emphasis (drag'n'drop)
 *      --  CRA_PICKED:        record picked (Lazy Drag)
 *      --  CRA_INUSE:         record has in-use emphasis (diagonal lines,
 *                             as with open objects with the WPS)
 *
 *      Miscellaneous flags:
 *      --  CRA_FILTERED:      record has been filtered (is invisible)
 *      --  CRA_DROPONABLE:    record can be dropped something upon
 *      --  CRA_RECORDREADONLY: record is read-only (no text edit with Alt+MB1)
 *      --  CRA_EXPANDED:      record is expanded (Tree view)
 *      --  CRA_COLLAPSED:     record is collapsed (Tree view)
 *
 *      plus the following half-documented from PMSTDDLG.H (haven't tested these):
 *      --  CRA_IGNORE         record is to be ignored
 *      --  CRA_DISABLED       has no visible effect, but can be used with
 *                             cnrhOwnerDrawRecord
 *      --  CRA_LOCKED         maybe "locked in place"?!?
 *      --  CRA_OWNERFREE      owner must free record
 *      --  CRA_OWNERDRAW      owner must draw record; I'd recommend using
 *                             CA_OWNERDRAW in CNRINFO.flWindowAttr instead.
 *
 *      This func returns the total number of records in the container
 *      or NULL upon errors (return value of CM_INSERTRECORD, changed V0.9.0).
 *
 *@@changed V0.9.0 [umoeller]: function prototype changed to insert more than one record
 *@@changed V0.9.0 [umoeller]: added error checking for pszText
 *@@changed V0.9.2 (2000-02-19) [umoeller]: added fInvalidate field
 */

ULONG cnrhInsertRecords(HWND hwndCnr,   // in: container to insert into
                        PRECORDCORE preccParent,
                            // in: record core below which precc should
                            // be inserted (tree view only). If NULL, precc
                            // is inserted at "root" level
                        PRECORDCORE precc,
                            // in: record core to insert (allocated using
                            // cnrhAllocRecords)
                        BOOL fInvalidate,
                        const char *pcszText,
                            // in: text for recc. in all views (or NULL)
                        ULONG flRecordAttr,
                            // in: CRA_* flags
                        ULONG ulCount)  // in: number of records to insert (> 0)
{
    RECORDINSERT    ri;

    if (precc)
    {
        // RECORDCORE stuff
        precc->flRecordAttr = flRecordAttr;
        // precc->preccNextRecord = NULL;

        if (pcszText) // V0.9.0
        {
            precc->pszIcon = (PSZ)pcszText;
            precc->pszText = (PSZ)pcszText;
            precc->pszName = (PSZ)pcszText;
            precc->pszTree = (PSZ)pcszText;
        }

        // setup RECORDINSERT struct
        ri.cb = sizeof(RECORDINSERT);
        ri.pRecordOrder = (PRECORDCORE)CMA_END;
        ri.pRecordParent = (PRECORDCORE)preccParent;
        ri.zOrder = CMA_TOP;
        ri.fInvalidateRecord = fInvalidate;  // V0.9.2 (2000-02-19) [umoeller]
        ri.cRecordsInsert = ulCount;        // V0.9.0

        return (ULONG)WinSendMsg(hwndCnr,
                                 CM_INSERTRECORD,
                                 (MPARAM)precc,
                                 (MPARAM)&ri);
    }

    return 0;
}

/*
 *@@ cnrhInsertRecordAfter:
 *      similar to cnrhInsertRecords, but this inserts
 *      a single record _after_ another one. Parent records
 *      and multiple records are not supported.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.4 (2000-06-14) [umoeller]: added fInvalidate
 */

ULONG cnrhInsertRecordAfter(HWND hwndCnr,
                            PRECORDCORE precc,
                            PSZ pszText,
                            ULONG flRecordAttr,
                            PRECORDCORE preccAfter,
                            BOOL fInvalidate)           // in: invalidate records?
{
    RECORDINSERT    ri;

    if (precc)
    {
        // RECORDCORE stuff
        precc->flRecordAttr = flRecordAttr;
        precc->preccNextRecord = NULL;

        if (pszText) // V0.9.0
        {
            precc->pszIcon = pszText;
            precc->pszText = pszText;
            precc->pszName = pszText;
            precc->pszTree = pszText;

            // setup RECORDINSERT struct
            ri.cb = sizeof(RECORDINSERT);
            ri.pRecordOrder = (PRECORDCORE)preccAfter;
            ri.pRecordParent = 0;   // no parent here
            ri.zOrder = CMA_TOP;
            ri.fInvalidateRecord = fInvalidate; // V0.9.4 (2000-06-14) [umoeller]
            ri.cRecordsInsert = 1;

            return (ULONG)WinSendMsg(hwndCnr,
                                     CM_INSERTRECORD,
                                     (MPARAM)precc,
                                     (MPARAM)&ri);
        }
    }

    return 0;
}

/*
 *@@ cnrhMoveRecord:
 *      this moves a single record within the same container
 *      window. This only makes sense in ordered views (i.e.
 *      Name, Text, and Details views).
 *
 *      For moving records with child records, use cnrhMoveTree
 *      instead.
 */

BOOL cnrhMoveRecord(HWND hwndCnr,
                    PRECORDCORE preccMove,        // in: record to move
                    PRECORDCORE preccInsertAfter)
                         // in: where to move the record to
                         // (can be CMA_FIRST or CMA_END)
{
    // rule out possible errors:
    if (    (preccMove)     // this better be valid
         && (preccInsertAfter)  // this also, because otherwise the record just disappears
         && (preccMove != preccInsertAfter)
                            // same here
       )
    {
        RECORDINSERT ri;
        ULONG        ulrc = 0;

        // remove record
        WinSendMsg(hwndCnr,
                   CM_REMOVERECORD,
                   (MPARAM)(&preccMove),
                   MPFROM2SHORT(1,
                                CMA_INVALIDATE)); // no free

        // and re-insert that record at the new position
        ri.cb = sizeof(RECORDINSERT);
        ri.pRecordOrder = (PRECORDCORE)preccInsertAfter;
        ri.pRecordParent = 0;   // no parent here
        ri.zOrder = CMA_TOP;
        ri.fInvalidateRecord = TRUE;
        ri.cRecordsInsert = 1;
        ulrc = (ULONG)WinSendMsg(hwndCnr,
                                 CM_INSERTRECORD,
                                 (MPARAM)preccMove,
                                 (MPARAM)&ri);

        return (ulrc != 0);
    }

    return FALSE;
}

/*
 *@@ cnrhMoveTree:
 *      this function moves a container record core and all its
 *      children from one tree to another.
 *      See the CM_MOVETREE message for more explanations.
 *
 *      The problem with that message is that automatic container
 *      sorting does not work in tree view. This function however
 *      is smart enough to maintain container sorting and insert
 *      the moved record core at the correct position below the
 *      target record core, if you specify a container sort function
 *      (pfnCnrSort, which works just in CNRINFO).
 *
 *      Otherwise, we'll just insert the item as the first child
 *      of preccNewParent.
 *
 *      This function also has a workaround for a nasty container
 *      bug which results in a crash in PMMERGE.DLL when a tree
 *      is moved to the last position in the target parent record
 *      (at least with Warp 4 FP 7).
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL cnrhMoveTree(HWND hwndCnr,          // in: container control
                  PRECORDCORE preccMove, // in: record core to move
                  PRECORDCORE preccNewParent, // in: new parent for preccMove
                                              // or NULL if move to root
                  PFNCNRSORT pfnCnrSort) // in: sort function to use or NULL
{
    TREEMOVE    tm;
    PRECORDCORE preccInsertAfter = (PRECORDCORE)CMA_FIRST;
    BOOL        fBugWorkAround = FALSE,
                brc = FALSE;

    if (pfnCnrSort)
    {
        // permanent sort activated:

        // since the automatic container sort does
        // not work when moving a record core tree in
        // Tree view, we must find the recc after
        // which we'll insert the tree ourselves
        PRECORDCORE preccSearch = preccNewParent;
        BOOL        fFirstRun = TRUE;
        ULONG       ulFirstCode;

        // set the code for first-loop query:
        if (preccNewParent)
            // if new parent is non-root
            ulFirstCode = CMA_FIRSTCHILD;
        else
            // if new parent is root
            ulFirstCode = CMA_FIRST;

        while (TRUE)
        {
            preccSearch =
                (PRECORDCORE)WinSendMsg(hwndCnr,
                                        CM_QUERYRECORD,
                                        // the following gets either the
                                        // first child recc of the target
                                        // record core or the next child
                                        // for consecutive loops
                                        (MPARAM)preccSearch,
                                        MPFROM2SHORT(
                                                ((fFirstRun)
                                                ? ulFirstCode  // first loop
                                                : CMA_NEXT  // works for next child too
                                            ),
                                            CMA_ITEMORDER)
                                        );
            fFirstRun = FALSE;

            if (    (preccSearch == NULL)
                 || ((ULONG)preccSearch == -1)
               )
            {
                // no more items found:
                // keep preccInsertAfter, which might be CMA_FIRST
                // or the preccSearch we have found previously.

                if (preccInsertAfter != (PRECORDCORE)CMA_FIRST)
                {
                    // Unfortunately, there is a bug in the container
                    // control which prohibits CM_MOVETREE from working
                    // if preccInsertAfter turns out to be the last
                    // record core in preccNewParent. This applies to
                    // preccInsertAfter == CMA_LAST also, and CMA_LASTCHILD
                    // doesn't work either.
                    // Duh.
                    // We'll fix this later.

                    fBugWorkAround = TRUE;
                }
                break;
            }

            if (((*pfnCnrSort)(preccSearch, preccMove, 0)) < 0)
            {
                // found record core is < our tree:
                // mark current as "insert after" for later and go on
                preccInsertAfter = preccSearch;
            }
            else
                break;
        }

        /* _Pmpf(("preccInsertAfter %s",
                (preccInsertAfter == (PRECORDCORE)CMA_FIRST) ? "CMA_FIRST"
                : (preccInsertAfter == (PRECORDCORE)CMA_LAST) ? "CMA_LAST"
                : (preccInsertAfter == NULL) ? "NULL"
                : preccInsertAfter->pszIcon
             )); */
    } // end if (CnrInfo.pSortRecord)

    if (fBugWorkAround)
        // this is TRUE only if preccInsertAfter has turned
        // out to be the last child of preccNewParent. This
        // will make the container crash, so we insert as
        // first and sort the whole damn container later.
        // Not terribly fast, but better than crashing. ;-)
        preccInsertAfter = (PRECORDCORE)CMA_FIRST;

    // set record to be moved
    tm.preccMove = preccMove;
    // set target record core
    tm.preccNewParent = preccNewParent;
    tm.pRecordOrder = preccInsertAfter;
    tm.flMoveSiblings = FALSE;
                // move only preccMove
    brc = (BOOL)WinSendMsg(hwndCnr,
                           CM_MOVETREE,
                           (MPARAM)&tm,
                           (MPARAM)NULL);

    if (brc)
        if (fBugWorkAround)
            WinSendMsg(hwndCnr, CM_SORTRECORD, (MPARAM)pfnCnrSort, (MPARAM)NULL);

    return brc;
}

/*
 *@@category: Helpers\PM helpers\Container helpers\View management
 *      functions for easily managing container views.
 *
 *      Most importantly, this has the BEGIN_CNRINFO and END_CNRINFO
 *      macros which make CNRINFO handling easier. Also see
 *      cnrhSetView for that.
 */

/* ******************************************************************
 *
 *   View management
 *
 ********************************************************************/

/*
 *@@ cnrhSelectAll:
 *      this selects or deselects all records in hwndCnr,
 *      depending on fSelect. Since multiple selections are
 *      not supported in Tree views in the first place, we
 *      only go for the "root" records (and no child records).
 *
 *      Invoking this function on a container in Tree view
 *      will result in not much but display flickering anyway.
 *
 *      This returns the number of records which were processed.
 */

ULONG cnrhSelectAll(HWND hwndCnr,
                    BOOL fSelect)
{
    ULONG ulrc = 0;
    PRECORDCORE precc2 = NULL;

    do {
        precc2 =
            (PRECORDCORE)WinSendMsg(hwndCnr,
                                    CM_QUERYRECORD,
                                    (MPARAM)precc2,
                                    MPFROM2SHORT(
                                        ((precc2 == NULL) ? CMA_FIRST : CMA_NEXT),
                                        CMA_ITEMORDER)
                                    );
        if ((precc2 == 0) || ((LONG)precc2 == -1))
            // last record or error:
            break;

        // select this one
        cnrhSelectRecord(hwndCnr, precc2, fSelect);
        ulrc++;

    } while (TRUE);

    return ulrc;
}

/*
 *@@ cnrhFindRecordFromPoint:
 *      this returns the record under the given
 *      point or NULL if none could be found
 *      (e.g. point is on whitespace).
 *
 *      The point must be specified in container
 *      coordinates, relative to the container
 *      origin (which might be invisible if the
 *      container viewport has been scrolled up).
 *
 *      With fsExtent, specify what part of the
 *      record should be checked. See
 *      cnrhScrollToRecord for more.
 *
 *      This is not terribly fast, because it
 *      will send at least one message for each
 *      record which is currently visible in the
 *      container.
 *
 *      With fl, specify any or none of the following:
 *
 *      --  FRFP_RIGHTSPLITWINDOW: test right split Details view
 *              instead of main cnr.
 *
 *      --  FRFP_SCREENCOORDS: *pptl specifies the point to
 *              check in _screen_ coordinates, instead of
 *              container ccordinates.
 *
 *@@added V0.9.1 (99-11-29) [umoeller]
 *@@changed V0.9.4 (2000-08-08) [umoeller]: fixed viewport/window confusion; now works after cnr resize
 */

PRECORDCORE cnrhFindRecordFromPoint(HWND hwndCnr,
                                    PPOINTL pptl,   // in: point to check for
                                    PRECTL prclFoundRecord,
                                            // out: if != NULL and record was
                                            // found, this receives the record rectl
                                            // (or subrectl, depending on fsExtent)
                                    ULONG fsExtent,
                                            // in: one or several of
                                            // CMA_ICON, CMA_TEXT, CMA_TREEICON
                                    ULONG fl)
{
    PRECORDCORE preccReturn = NULL;
    RECTL       rclViewport;
    HAB         habCnr = WinQueryAnchorBlock(hwndCnr);

    POINTL      ptlCheck;

    // _Pmpf(("Entering cnrhFindRecordFromPoint"));

    ptlCheck.x = pptl->x;
    ptlCheck.y = pptl->y;

    if (fl & FRFP_SCREENCOORDS)
    {
        // _Pmpf(("  mapping screen %d/%d to cnr", ptlCheck.x, ptlCheck.y));
        WinMapWindowPoints(HWND_DESKTOP,
                           hwndCnr,
                           &ptlCheck,
                           1);
        // _Pmpf(("  got %d/%d", ptlCheck.x, ptlCheck.y));
    }

    // get cnr viewport (currently visible client area,
    // relative to cnr origin)
    if (WinSendMsg(hwndCnr,
                   CM_QUERYVIEWPORTRECT,
                   (MPARAM)&rclViewport,
                   MPFROM2SHORT(CMA_WINDOW, // CMA_WORKSPACE, V0.9.4 (2000-08-08) [umoeller]
                                ((fl & FRFP_RIGHTSPLITWINDOW) != 0))))
                                        // right split window?
    {
        PRECORDCORE         preccFound = (PRECORDCORE)CMA_FIRST;
        QUERYRECFROMRECT    qrfr;
        QUERYRECORDRECT     qrr;

        // now enumerate all records which are visible in
        // the viewport
        do
        {
            qrfr.cb = sizeof(qrfr);
            qrfr.rect.xLeft = rclViewport.xLeft;
            qrfr.rect.yBottom = rclViewport.yBottom;
            qrfr.rect.xRight = rclViewport.xRight;
            qrfr.rect.yTop = rclViewport.yTop;
            qrfr.fsSearch = CMA_PARTIAL | CMA_ITEMORDER;

            // _Pmpf(("  Enumerating recs in viewport %d, %d, %d, %d",
            //             rclViewport.xLeft, rclViewport.yBottom, rclViewport.xRight, rclViewport.yTop));

            preccFound = (PRECORDCORE)WinSendMsg(hwndCnr,
                                                 CM_QUERYRECORDFROMRECT,
                                                 (MPARAM)preccFound, // CMA_FIRST on first loop
                                                 (MPARAM)&qrfr);
            // _Pmpf(("   CM_QUERYRECORDFROMRECT returned 0x%lX", preccFound));

            if (    (preccFound != NULL)
                 && (preccFound != (PRECORDCORE)-1)
               )
            {
                RECTL rclRecord;
                qrr.cb = sizeof(qrr);
                qrr.pRecord = preccFound;
                qrr.fRightSplitWindow = ((fl & FRFP_RIGHTSPLITWINDOW) != 0);
                qrr.fsExtent = fsExtent;
                if (WinSendMsg(hwndCnr,
                               CM_QUERYRECORDRECT,
                               (MPARAM)&rclRecord,
                               (MPARAM)&qrr))
                {
                    if (WinPtInRect(habCnr,
                                    &rclRecord,
                                    &ptlCheck))
                    {
                        // yes, it's in the record:
                        // return that
                        preccReturn = preccFound;

                        // if rectangle is queried,
                        // copy that
                        if (prclFoundRecord)
                            memcpy(prclFoundRecord, &rclRecord, sizeof(RECTL));
                        break;
                    }
                }
            }
            else
                // last one or error:
                break;
        } while (TRUE);
    }
    // else
    //     _Pmpf(("  CM_QUERYVIEWPORTRECT failed."));

    return preccReturn;
}

/*
 *@@ cnrhExpandFromRoot:
 *      expands prec and climbs up all parent
 *      records and expands them all as well.
 *
 *      Returns the no. of records expanded.
 *
 *@@added V0.9.9 (2001-03-13) [umoeller]
 */

ULONG cnrhExpandFromRoot(HWND hwndCnr,
                         PRECORDCORE prec)
{
    ULONG ul = 0;
    PRECORDCORE preccParent = prec;
    while (preccParent)
    {
        WinSendMsg(hwndCnr, CM_EXPANDTREE, (MPARAM)preccParent, MPNULL);
        ul++;

        preccParent = (PRECORDCORE)WinSendMsg(hwndCnr,
                                              CM_QUERYRECORD,
                                              (MPARAM)preccParent,
                                              MPFROM2SHORT(CMA_PARENT,
                                                           CMA_ITEMORDER));

        if (preccParent == (PRECORDCORE)-1)
            preccParent = NULL;
    }

    return ul;
}

/*
 *@@ cnrhScrollToRecord:
 *      scrolls a given container control to make a given
 *      record visible.
 *
 *      Returns:
 *      --  0:       OK, scrolled
 *      --  1:       record rectangle query failed (error)
 *      --  2:       cnr viewport query failed (error)
 *      --  3:       record is already visible (scrolling not necessary)
 *      --  4:       cnrinfo query failed (error)
 *
 *      Note: All messages are _sent_ to the container, not posted.
 *      Scrolling therefore occurs synchroneously before this
 *      function returns.
 *
 *      This function an improved version of the one (W)(C) Dan Libby, found at
 *      http://zebra.asta.fh-weingarten.de/os2/Snippets/Howt6364.HTML
 *      Improvements (C) 1998 Ulrich M”ller.
 *
 *@@changed V0.9.4 (2000-08-07) [umoeller]: now posting scroll messages to avoid sync errors
 *@@changed V0.9.9 (2001-03-12) [umoeller]: this never scrolled for root records in tree view if KeepParent == TRUE, fixed
 *@@changed V0.9.9 (2001-03-13) [umoeller]: largely rewritten; this now scrolls x properly too and is faster
 */

ULONG cnrhScrollToRecord(HWND hwndCnr,       // in: container window
                         PRECORDCORE pRec,   // in: record to scroll to
                         ULONG fsExtent,
                                 // in: this determines what parts of pRec
                                 // should be made visible. OR the following
                                 // flags:
                                 // -- CMA_ICON: the icon rectangle
                                 // -- CMA_TEXT: the record text
                                 // -- CMA_TREEICON: the "+" sign in tree view
                         BOOL fKeepParent)
                                 // for tree views only: whether to keep
                                 // the parent record of pRec visible when scrolling.
                                 // If scrolling to pRec would make the parent
                                 // record invisible, we instead scroll so that
                                 // the parent record appears at the top of the
                                 // container workspace (Win95 style).

{
    QUERYRECORDRECT qRect;
    RECTL           rclRecord,
                    rclCnr;
    LONG            lXOfs = 0,
                    lYOfs = 0;

    qRect.cb = sizeof(qRect);
    qRect.pRecord = (PRECORDCORE)pRec;
    qRect.fsExtent = fsExtent;

    if (fKeepParent)
    {
        CNRINFO         CnrInfo;
        // this is only valid in tree view, so check
        if (!WinSendMsg(hwndCnr,
                        CM_QUERYCNRINFO,
                        (MPARAM)&CnrInfo,
                        (MPARAM)sizeof(CnrInfo)))
            return 4;
        else
            // disable if not tree view
            fKeepParent = ((CnrInfo.flWindowAttr & CV_TREE) != 0);
    }

    // query record location and size of container
    if (!WinSendMsg(hwndCnr,
                    CM_QUERYRECORDRECT,
                    &rclRecord,
                    &qRect))
        return 1;

    if (!WinSendMsg(hwndCnr,
                    CM_QUERYVIEWPORTRECT,
                    &rclCnr,
                    MPFROM2SHORT(CMA_WINDOW,
                                    // returns the client area rectangle
                                    // in container window coordinates
                                 FALSE)) )
        return 2;

    // check if left bottom point of pRec is currently visible in container

    #define IS_BETWEEN(a, b, c) (((a) >= (b)) && ((a) <= (c)))

    // 1) set lXOfs if we need to scroll horizontally
    if (!IS_BETWEEN(rclRecord.xLeft, rclCnr.xLeft, rclCnr.xRight))
        // record xLeft is outside viewport:
        // scroll horizontally so that xLeft is exactly on left of viewport
        lXOfs = (rclRecord.xLeft - rclCnr.xLeft);
    else if (!IS_BETWEEN(rclRecord.xRight, rclCnr.xLeft, rclCnr.xRight))
        // record xRight is outside viewport:
        // scroll horizontally so that xRight is exactly on right of viewport
        lXOfs = (rclRecord.xRight - rclCnr.xRight);

    // 2) set lYOfs if we need to scroll vertically
    if (!IS_BETWEEN(rclRecord.yBottom, rclCnr.yBottom, rclCnr.yTop))
        // record yBottom is outside viewport:
        // scroll horizontally so that yBottom is exactly on bottom of viewport
        lYOfs =   (rclCnr.yBottom - rclRecord.yBottom)    // this would suffice
                + (rclRecord.yTop - rclRecord.yBottom);
                            // but we make the next rcl visible too
    else if (!IS_BETWEEN(rclRecord.yTop, rclCnr.yBottom, rclCnr.yTop))
        // record yTop is outside viewport:
        // scroll horizontally so that yTop is exactly on top of viewport
        lYOfs = (rclRecord.yTop - rclCnr.yTop);

    if (fKeepParent && (lXOfs || lYOfs))
    {
        // keep parent enabled, and we're scrolling:
        // find the parent record then
        qRect.cb = sizeof(qRect);
        qRect.pRecord = (PRECORDCORE)WinSendMsg(hwndCnr,
                                                CM_QUERYRECORD,
                                                (MPARAM)pRec,
                                                MPFROM2SHORT(CMA_PARENT,
                                                             CMA_ITEMORDER));
        if (qRect.pRecord)     // V0.9.9 (2001-03-12) [umoeller]
        {
            // parent exists:
            // get PARENT record rectangle then
            RECTL rclParentRecord;
            qRect.fsExtent = fsExtent;
            if (WinSendMsg(hwndCnr,
                           CM_QUERYRECORDRECT,
                           &rclParentRecord,
                           &qRect))
            {
                // check if parent record WOULD still be visible
                // if we scrolled what we calculated above
                RECTL rclCnr2;
                memcpy(&rclCnr2, &rclCnr, sizeof(rclCnr2));
                winhOffsetRect(&rclCnr2, lXOfs, -lYOfs);

                if (    lXOfs
                     && (!IS_BETWEEN(rclParentRecord.xLeft, rclCnr2.xLeft, rclCnr2.xRight))
                   )
                    // record xLeft is outside viewport:
                    // scroll horizontally so that xLeft is exactly on left of viewport
                    lXOfs = (rclParentRecord.xLeft - rclCnr.xLeft);

                if (    lYOfs
                     && (!IS_BETWEEN(rclParentRecord.yBottom, rclCnr2.yBottom, rclCnr2.yTop))
                   )
                    // record yBottom is outside viewport:
                    // recalculate y ofs so that we scroll so
                    // that parent record is on top of cnr viewport
                    lYOfs =   (rclCnr.yTop - rclParentRecord.yTop) // this would suffice
                            - (rclRecord.yTop - rclRecord.yBottom);  // but we make the previous rcl visible too
            }
        }
    }

    // V0.9.14 (2001-07-28) [umoeller]
    // tried WinSendMsg (instead of post) because
    // otherwise we can't get auto-edit on wpshCreateFromTemplate
    // to work... but this causes two problems:
    // --  for some reason, the WPS selects the recc under
    //     the mouse then, which is very wrong in most situations;
    // --  since the WPS expands the root record in tree views
    //     after the root has been populated, this can cause
    //     totally garbled display for complex trees.
    // So I'm back to WinPostMsg... // @@todo

    if (lXOfs)
        // scroll horizontally
        WinPostMsg(hwndCnr,
                   CM_SCROLLWINDOW,
                   (MPARAM)CMA_HORIZONTAL,
                   (MPARAM)lXOfs);

    // scroll vertically
    if (lYOfs)
        WinPostMsg(hwndCnr,
                   CM_SCROLLWINDOW,
                   (MPARAM)CMA_VERTICAL,
                   (MPARAM)lYOfs);

    return 0;
}

/*
 *@@ cnrhShowContextMenu:
 *      this function shows the given menu as a context
 *      menu for the given record core (using WinPopupMenu).
 *      This function may be used when receiving WM_CONTROL
 *      with CN_CONTEXTMENU from the container, which has
 *      the preccSource in mp2.
 *
 *      In detail, this function does the following:
 *
 *      1)  query the coordinates where to show the menu;
 *          if (preccSource), this will be next to the
 *          record core, otherwise (i.e. menu requested
 *          for whitespace) the mouse coordinates over
 *          the container;
 *
 *      2)  give preccSource (or, if NULL, the whole
 *          container) source emphasis;
 *
 *      3)  call WinPopupMenu.
 *
 *      Note: It is the responsibility of the caller to catch
 *      WM_MENUEND in the window procedure of hwndMenuOwner later
 *      to remove the source emphasis for preccSource again.
 *
 *      This function returns FALSE if an error occurred.
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL cnrhShowContextMenu(HWND hwndCnr,
                         PRECORDCORE preccSource, // in: mp2 of CN_CONTEXTMENU
                         HWND hMenu,              // in: menu to show
                         HWND hwndMenuOwner)      // in: menu owner (where the
                                                  // WM_COMMAND will go to)
{
    BOOL    brc = FALSE;
    if (hMenu)
    {
        BOOL        fQueried = FALSE;

        POINTL ptl;
        if (preccSource)
        {
            CNRINFO     CnrInfo;
            cnrhQueryCnrInfo(hwndCnr, &CnrInfo);

            if ((CnrInfo.flWindowAttr & CV_DETAIL) == 0)
            {
                // if we're not in Details view:
                // calculate the point where to show the context
                // menu; we use the lower-right corner of the
                // source record core
                QUERYRECORDRECT qRect;
                RECTL           rclRecc;
                qRect.cb = sizeof(qRect);
                qRect.pRecord = preccSource;
                qRect.fsExtent = CMA_TEXT;
                WinSendMsg(hwndCnr,
                           CM_QUERYRECORDRECT,
                           &rclRecc,
                           &qRect);
                ptl.x = rclRecc.xRight;
                ptl.y = rclRecc.yBottom;
                    // now we have the lower-right corner in cnr coords

                // clip if this is outside the container window
                WinQueryWindowRect(hwndCnr, &rclRecc);
                if (ptl.x > rclRecc.xRight)
                    ptl.x = rclRecc.xRight;
                if (ptl.y > rclRecc.yTop)
                    ptl.y = rclRecc.yTop;

                // convert this to screen coordinates
                WinMapWindowPoints(hwndCnr,
                                   HWND_DESKTOP,
                                   &ptl,
                                   1);
                fQueried = TRUE;
            }
        }

        if (!fQueried)
            // else: use mouse coordinates for context menu
            WinQueryPointerPos(HWND_DESKTOP, &ptl);

        // give preccSource source emphasis;
        // if preccSource == NULL, the whole container will be
        // given source emphasis
        WinSendMsg(hwndCnr,
                   CM_SETRECORDEMPHASIS,
                   (MPARAM)preccSource,     // might be NULL for whole container
                   MPFROM2SHORT(TRUE,  // set emphasis
                            CRA_SOURCE));

        // finally, show context menu
        brc = WinPopupMenu(HWND_DESKTOP,               // menu parent
                           hwndMenuOwner,              // menu owner
                           hMenu,
                           (SHORT)ptl.x,
                           (SHORT)ptl.y,
                           0,                          // ID
                           PU_NONE
                              | PU_MOUSEBUTTON1
                              | PU_KEYBOARD
                              | PU_HCONSTRAIN
                              | PU_VCONSTRAIN);
    }

    return brc;
}

/*
 *@@ cnrhQuerySourceRecord:
 *      this helper function evaluates a given container
 *      to find out which records have been selected while
 *      a context menu is open.
 *
 *      This is for implementing a WPS-like (probably CUA) behavior
 *      when invoking actions on container records. That is:
 *
 *      1)  If the user opens a context menu on a selected object,
 *          the selected action should be invoked on _all_ selected
 *          objects.
 *
 *      2)  If the user opens a context menu on an object which is
 *          _not_ selected, the action should be invoked on that
 *          object only (which should be given source emphasis),
 *          no matter what other objects are selected.
 *
 *      This function expects in preccSource the record core which
 *      currently has (or just had) source emphasis.
 *
 *      1)  In your own programs, you should have used cnrhShowContextMenu
 *          above, which sets record source emphasis correctly. Unfortunately,
 *          PM posts WM_MENUEND _before_ WM_COMMAND, so if you remove source
 *          emphasis in WM_MENUEND, it is unknown which record was selected at
 *          the time WM_COMMAND comes in... so you need to store that record
 *          and pass it to this function later. Sorry.
 *
 *      2)  With the WPS, this function works within the wpMenuItemSelected
 *          method, because that one is invoked during the processing of
 *          WM_COMMAND and the WPS automatically does the source emphasis
 *          stuff right.
 *
 *      The result of this evaluation is stored in *pulSelection,
 *      which can be:
 *
 *      --   SEL_WHITESPACE (1): the context menu was opened on the
 *                          whitespace of the container (preccSource
 *                          is NULL);
 *                          this func then returns NULL also.
 *      --   SEL_SINGLESEL (2): the context menu was opened for a
 *                          single selected object:
 *                          this func then returns that record core
 *                          (which is preccSource).
 *      --   SEL_MULTISEL (3): the context menu was opened on one
 *                          of a multitude of selected record;
 *                          this func then returns the first of the
 *                          selected records. Use
 *                          cnrhQueryNextSelectedRecord to get the others.
 *      --   SEL_SINGLEOTHER (4): the context menu was opened for a
 *                          single record _other_ than the selected
 *                          records:
 *                          this func then returns that record
 *                          (which is preccSource).
 *
 *@@added V0.9.0 [umoeller]
 */

PRECORDCORE cnrhQuerySourceRecord(HWND hwndCnr,          // in:  cnr
                                  PRECORDCORE preccSource, // in: record which had source emphasis
                                  PULONG pulSelection)   // out: selection flags
{
    PRECORDCORE preccReturn = NULL;

    if (preccSource == NULL)
    {
        // this probably means that the whole container has
        // source emphasis --> context menu on folder whitespace
        *pulSelection = SEL_WHITESPACE;
                // and return NULL
    }
    else if (((LONG)preccSource) != -1) // no error?
    {
        // check whether the source record is also selected
        if ((preccSource->flRecordAttr & CRA_SELECTED) == 0)
        {
            // no:
            // only one object, but not one of the selected ones
            // (if any at all)
            preccReturn = preccSource;
            *pulSelection = SEL_SINGLEOTHER;
        }
        else
        {
            // yes, source record _is_ selected:
            // check whether we have more than one selected record?

            // get first selected record
            PRECORDCORE preccSelected = (PRECORDCORE)WinSendMsg(hwndCnr,
                                                                CM_QUERYRECORDEMPHASIS,
                                                                (MPARAM)CMA_FIRST,
                                                                (MPARAM)CRA_SELECTED);
            // return that one
            preccReturn = preccSelected;

            // are several objects selected?
            preccSelected = (PRECORDCORE)WinSendMsg(hwndCnr,
                                                   CM_QUERYRECORDEMPHASIS,
                                                   (MPARAM)preccSelected,
                                                            // get next selected
                                                   (MPARAM)CRA_SELECTED);
            if (preccSelected)
                // several objects
                *pulSelection = SEL_MULTISEL;
            else
                // only one object
                *pulSelection = SEL_SINGLESEL;
        }
    }

    return preccReturn;
}

/*
 *@@ cnrhQueryNextSelectedRecord:
 *      if cnrhQuerySourceRecord above returns SEL_MULTISEL
 *      you can use this helper func to loop thru all the
 *      selected records. This will return the next record
 *      after preccCurrent which is selected or NULL if it's the last.
 *
 *      If you're not using cnrhQuerySourceRecord (because your
 *      records do not have source emphasis), you can also call
 *      this function with (preccCurrent == CMA_FIRST) to get the
 *      first selected record core.
 */

PRECORDCORE cnrhQueryNextSelectedRecord(HWND hwndCnr,
                                        PRECORDCORE preccCurrent)
{
    PRECORDCORE preccReturn = 0;
    if (preccCurrent)
    {
        PRECORDCORE preccNext = (PRECORDCORE)WinSendMsg(hwndCnr,
                                                        CM_QUERYRECORDEMPHASIS,
                                                        (MPARAM)preccCurrent,
                                                        (MPARAM)CRA_SELECTED);
        if ((preccNext) && ((LONG)preccNext != -1) )
            preccReturn = preccNext;
    }

    return preccReturn;
}

/*
 *@@category: Helpers\PM helpers\Container helpers\Record relations/iteration
 *      functions for querying the relation of records and
 *      iterating through records.
 */

/* ******************************************************************
 *
 *   Record relations/iteration
 *
 ********************************************************************/

/*
 *@@ cnrhQueryRecordIndex:
 *      returns the "index" of the specified record
 *      in the specified container. That is, if the
 *      specified record is the first record in the
 *      container, 0 is returned. If it's the second,
 *      1 is returned, and so on.
 *
 *      Returns -1 if precc could not be found.
 *
 *      Doesn't work in Tree view.
 *
 *@@added V0.9.3 (2000-04-19) [umoeller]
 */

LONG cnrhQueryRecordIndex(HWND hwndCnr,
                          PRECORDCORE precc)
{
    PRECORDCORE precc2 = (PRECORDCORE)CMA_FIRST;
    BOOL        fFirstCall = TRUE;
    LONG        lrc = -1,
                lCount = 0;

    while (TRUE)
    {
        precc2 =
            (PRECORDCORE)WinSendMsg(hwndCnr,
                                    CM_QUERYRECORD,
                                    (MPARAM)precc2,  // ignored on first call
                                    MPFROM2SHORT(
                                            (fFirstCall)
                                                ? CMA_FIRST
                                                : CMA_NEXT,
                                            CMA_ITEMORDER)
                                    );
        if (     (precc2 == NULL)
              || (precc2 == (PRECORDCORE)-1)
           )
            // error:
            // return -1
            break;
        else
            if (precc2 == precc)
            {
                // same as search record:
                lrc = lCount;
                break;
            }

        // else search on
        fFirstCall = FALSE;
        lCount++;
    }

    return lrc;
}

/*
 *@@ cnrhIsChildOf:
 *      returns TRUE only if precTest is a child record
 *      of precParent in a tree view.
 *
 *@@added V0.9.7 (2000-12-13) [umoeller]
 */

BOOL cnrhIsChildOf(HWND hwndCnr,
                   PRECORDCORE precTest,        // in: recc to test
                   PRECORDCORE precParent)      // in: parent to test
{
    BOOL brc = FALSE;
    if ((precTest) && (precParent))
    {
        PRECORDCORE precParentThis = precTest;

        while (precParentThis)
        {
            // first call: get parent of precTest;
            // subsequent calls: climb up
            precParentThis = (PRECORDCORE)WinSendMsg(hwndCnr,
                                                     CM_QUERYRECORD,
                                                     precParentThis,
                                                     MPFROM2SHORT(CMA_PARENT,
                                                                  CMA_ITEMORDER));
            if (precParentThis == (PRECORDCORE)-1)
                break;
            else
                if (precParentThis == precParent)
                {
                    brc = TRUE;
                    break;
                }
        }
    }

    return brc;
}

/*
 *@@ cnrhForAllRecords:
 *      this monster function calls the given callback
 *      function for really all the records in the
 *      container, including child records in tree view.
 *
 *      This is extremely useful for cleaning up
 *      all record cores before a container window
 *      gets destroyed.
 *
 *      This function recurses for child records.
 *      On the first call, precParent should be
 *      NULL; you may however specify a certain
 *      record, and this function will call the
 *      callback only for that record and children.
 *
 *      As a special argument, if precParent is -1,
 *      we do not recurse into child records. This
 *      is useful if you _know_ that your container
 *      does not contain child records and you want
 *      to speed up processing.
 *
 *      The callback function pfncb must be declared as
 *      follows:
 *
 +          ULONG XWPENTRY fncb(HWND hwndCnr,
 +                              PRECORDCORE precc,
 +                              ULONG ulUser)
 *
 *      It gets called for every record with the following
 *      parameters:
 *
 *      -- HWND hwndCnr, as passed to this function
 *
 *      -- PRECORDCORE precc: current record core
 *
 *      -- ULONG ulUser: what was given to this function.
 *
 *      If the callback returns anything != 0, this
 *      function stops even before all records have
 *      been processed. You can use ulUser for a
 *      pointer to a return value.
 *
 *      This always returns the no. of records which
 *      were processed.
 *
 *      If you use this function for deleting record
 *      cores, you can be sure that you can delete
 *      every record, because your callback gets called
 *      for the child records before the parent record.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V1.0.0 (2002-09-09) [umoeller]: rewritten to support deletion in callback
 *@@changed V1.0.0 (2002-09-09) [umoeller]: added support for precParent = -1
 *@@changed V1.0.0 (2002-09-09) [umoeller]: changed prototype and callback prototype
 */

ULONG cnrhForAllRecords(HWND hwndCnr,
                        PRECORDCORE precParent,     // in: parent rec to start with, or NULL, or -1
                        PFNCBRECC pfncb,            // in: callback function
                        ULONG ulUser)               // in: user argument for callback
{
    ULONG       ulrc = 0;

    // get first record or first child record
    PRECORDCORE prec2;

    BOOL        fRecurse = TRUE;

    if ((ULONG)precParent == -1)
    {
        fRecurse = FALSE;
        precParent = NULL;
    }

    prec2 = (PRECORDCORE)WinSendMsg(hwndCnr,
                                    CM_QUERYRECORD,
                                    (MPARAM)precParent,
                                            // ignored for CMA_FIRST
                                    MPFROM2SHORT(
                                            (precParent)
                                                    ? CMA_FIRSTCHILD
                                                    : CMA_FIRST,
                                            CMA_ITEMORDER)
                                    );

    // loop while we have records
    while (    (prec2)
            && ((ULONG)prec2 != -1)
          )
    {
        // record found:

        // get the next record BEFORE calling the callback
        // in case the callback removes the record
        // V1.0.0 (2002-09-09) [umoeller]
        PRECORDCORE precNext = (PRECORDCORE)WinSendMsg(hwndCnr,
                                                       CM_QUERYRECORD,
                                                       (MPARAM)prec2,
                                                       MPFROM2SHORT(CMA_NEXT,
                                                                    CMA_ITEMORDER));

        if (fRecurse)   // V1.0.0 (2002-09-09) [umoeller]
            // recurse for the record we found
            ulrc += cnrhForAllRecords(hwndCnr,
                                      prec2,        // new parent to search
                                      pfncb,
                                      ulUser);

        // call callback
        if (    (pfncb)
             && (pfncb(hwndCnr, prec2, ulUser))
           )
            // returns something != NULL:
            // stop
            break;

        ulrc++;

        prec2 = precNext;
    }

    return ulrc;
}

/*
 * cnrhForAllChildRecords:
 *      calls the specified fncbRecc callback for
 *      the specified recc and all its child records.
 *
 *@@added V0.9.0 [umoeller]
 */

/* VOID cnrhForAllChildRecords(HWND hwndCnr,
                            PRECORDCORE precc,
                            PFNCBRECC pfncbRecc,
                            ULONG ulp1,
                            ULONG ulp2)
{
    PRECORDCORE precc2 = precc;
    (*pfncbRecc)(precc, ulp1, ulp2);
    do {
        precc2 =
            (PRECORDCORE)WinSendMsg(hwndCnr,
                    CM_QUERYRECORD,
                    (MPARAM)precc2,
                    MPFROM2SHORT(
                        (precc2 == precc)
                            ? CMA_FIRSTCHILD : CMA_NEXT,
                        CMA_ITEMORDER)
                    );
        if ((LONG)precc2 == -1)
            precc2 = NULL;
        if (precc2)
            // recurse again
            cnrhForAllChildRecords(hwndCnr, precc2, pfncbRecc, ulp1, ulp2);
    } while (precc2);
} */

/*
 * cnrhForAllRecords2:
 *      this is a useful function which calls
 *      the specified callback function for
 *      really all records in the container of
 *      the main window, including child records.
 *
 *      xxx
 *
 *@@added V0.9.0 [umoeller]
 */

/* VOID cnrhForAllRecords2(HWND hwndCnr,
                        PFNCBRECC pfncbRecc,
                        ULONG ulp1,
                        ULONG ulp2)
{
    PRECORDCORE precc2 = NULL;
    do {
        precc2 =
            (PRECORDCORE)WinSendMsg(hwndCnr,
                    CM_QUERYRECORD,
                    (MPARAM)precc2,
                    MPFROM2SHORT(
                        ((precc2 == NULL) ? CMA_FIRST : CMA_NEXT),
                        CMA_ITEMORDER)
                    );
        if ((LONG)precc2 == -1)
            precc2 = NULL;
        if (precc2)
            // recurse again
            cnrhForAllChildRecords(hwndCnr, precc2, pfncbRecc, ulp1, ulp2);
    } while (precc2);
} */

/*
 * cnrhForAllParentRecords:
 *      just as above, but climbs up instead.
 *      The first call of the callback will be
 *      for the parent record of precc.
 *
 *@@added V0.9.0 [umoeller]
 */

/* VOID cnrhForAllParentRecords(HWND hwndCnr,
                             PRECORDCORE precc,
                             PFNCBRECC pfncbRecc,
                             ULONG ulp1,
                             ULONG ulp2)
{
    PRECORDCORE precc2 = precc;
    do {
        precc2 =
            (PRECORDCORE)WinSendMsg(hwndCnr,
                                    CM_QUERYRECORD,
                                    (MPARAM)precc2,
                                    MPFROM2SHORT(CMA_PARENT,
                                        CMA_ITEMORDER)
                                    );
        if ((LONG)precc2 == -1)
            precc2 = NULL;
        if (precc2)
            (*pfncbRecc)(precc2, ulp1, ulp2);
    } while (precc2);
} */

/*
 *@@category: Helpers\PM helpers\Container helpers\Miscellaneous
 */

/* ******************************************************************
 *
 *   Miscellaneous
 *
 ********************************************************************/

/*
 *@@ cnrhQueryCnrFromFrame:
 *      find the handle of a frame window's container; we do this
 *      by enumerating all the frame's child windows and comparing
 *      their window classes to the WC_CONTAINER code.
 *      This is not terribly fast, so use this func economically.
 *      Returns NULLHANDLE if not found.
 */

HWND cnrhQueryCnrFromFrame(HWND hwndFrame)
{
    HENUM               henum;
    CHAR                szClassName[256];
    HWND                hwndCnr = NULLHANDLE,
                        hwndTemp = NULLHANDLE;

    if (hwndFrame)
    {
        if (henum = WinBeginEnumWindows(hwndFrame))
        {
            do
            {
                if (hwndTemp = WinGetNextWindow(henum))
                {
                    if (WinQueryClassName(hwndTemp, 250, szClassName))
                        if (strcmp(szClassName, "#37") == 0)
                            // unbelievable, this is PM's code for WC_CONTAINER...
                            hwndCnr = hwndTemp;
                }
            } while (hwndTemp);
        }
        WinEndEnumWindows(henum);
    }

    return hwndCnr;
}

/*
 *@@ cnrhOpenEdit:
 *      begins direct text editing on the first
 *      record that is currently cursored in
 *      the container.
 *
 *      This is the recommended response when
 *      the almost documented WM_TEXTEDIT
 *      message comes into the container owner's
 *      window procedure. This is the case when
 *      the user presses the key combo that was
 *      set for direct text editing in the "Keyboard"
 *      object.
 *
 *      Returns TRUE if direct editing was
 *      successfully started. If FALSE is returned,
 *      there is either no record with CRA_CURSORED
 *      emphasis, or that record has the read-only
 *      flag set.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

BOOL cnrhOpenEdit(HWND hwndCnr)
{
    BOOL brc = FALSE;
    CNREDITDATA ced;
    memset(&ced, 0, sizeof(ced));
    ced.cb = sizeof(ced);
    ced.hwndCnr = hwndCnr;
    if (ced.pRecord = (PRECORDCORE)WinSendMsg(hwndCnr,
                                              CM_QUERYRECORDEMPHASIS,
                                              (MPARAM)CMA_FIRST,
                                              (MPARAM)CRA_CURSORED))
    {
        ced.id = WinQueryWindowUShort(hwndCnr, QWS_ID);
        brc = (BOOL)WinSendMsg(hwndCnr,
                               CM_OPENEDIT,
                               (MPARAM)&ced,
                               0);
    }

    return brc;
}

/*
 *@@ cnrhInitDrag:
 *      this sets up the necessary structures to begin dragging
 *      a record core from a container. This helper func should be
 *      called if your container should support being the source
 *      window of a direct manipulation.
 *
 *      This should get called in three situations:
 *
 *      --  the container sends CN_INITDRAG ("real" d'n'd desired by user);
 *
 *      --  the container sends us CN_PICKUP (Alt+MB2 pressed);
 *
 *      --  the user has selected "Pickup" from a record core's
 *          context menu (ID_XSMI_FILETYPES_PICKUP command).
 *          In that case, you can also call this function with
 *          usNotifyCode == CN_PICKUP.
 *
 *      In all three cases, preccDrag must be the record core
 *      which is to be dragged.
 *
 *      Depending on usNotifyCode, this function, after having allocated
 *      the necessary data, will do the following:
 *
 *      1)  If (usNotifyCode == CN_PICKUP), we will initiate a non-modal
 *          (lazy) drag (DrgLazyDrag). This function will then return
 *          after the records have been given "picked" emphasis.
 *
 *          Note: You must intercept CN_DROPNOTIFY in your window proc
 *          to clean up resources later. For this, call:
 *
 +              DrgDeleteDraginfoStrHandles(pdrgInfo);
 +              DrgFreeDraginfo(pdrgInfo);
 *
 *      2)  However, if (usNotifyCode == CN_INITDRAG), we will start
 *          a regular modal drag here by calling DrgDrag. This function
 *          will _not_ return until the object has been dropped or d'n'd
 *          has been cancelled. PM establishes another message loop
 *          internally for this. In this case, this function managed
 *          cleanup automatically.
 *
 *      This function supports one single record core only. The following
 *      information will be set in the DRAGITEM structure:
 *
 *      --  ulItemID will be set to the preccDrag so that the target
 *          window can access the dragged record.
 *
 *      --  hstrSourceName == hstrTargetName gets the RECORDCORE.pszIcon.
 *
 *      The drag icon will be a default system file icon.
 *
 *      Preconditions:
 *
 *      --  pszIcon must be set in the RECORDCORE. This is used for
 *          the item source name. This must not be NULL.
 */

PDRAGINFO cnrhInitDrag(HWND hwndCnr,
                                 // in: source container window
                       PRECORDCORE preccDrag,
                                 // in: record to be dragged (only one supported)
                       USHORT usNotifyCode,
                                 // in: CN_INITDRAG or CN_PICKUP
                       PSZ   pszRMF,
                                 // in: rendering mechanism and format,
                                 // e.g. "(DRM_MYFILETYPE)x(DRF_UNKNOWN)"
                       USHORT usSupportedOps)
                                 // stored in DRAGITEM.fsSupportedOps,
                                 // one or several of DO_COPYABLE, DO_MOVEABLE, DO_LINKABLE
{
    DRAGIMAGE    drgImage;
    PDRAGINFO    pdrgInfo = NULL;

    memset(&drgImage, 0, sizeof(drgImage));

    if (pdrgInfo = DrgAllocDraginfo(1)) // one item only
    {
        DRAGITEM drgItem;
        memset(&drgItem, 0, sizeof(drgItem));

        drgItem.hwndItem  = hwndCnr;
        drgItem.ulItemID  = (ULONG)preccDrag;
                    // we use this to store the container rec
        drgItem.hstrType  = DrgAddStrHandle(DRT_UNKNOWN);
                    // application defined
        drgItem.hstrRMF   = DrgAddStrHandle(pszRMF);
        drgItem.hstrContainerName = 0;
        drgItem.hstrSourceName = DrgAddStrHandle(preccDrag->pszIcon);
        drgItem.hstrTargetName = drgItem.hstrSourceName;
        drgItem.fsSupportedOps = usSupportedOps;

        // set the DRAGITEM struct into the memory
        // allocated by DrgAllocDraginfo()
        DrgSetDragitem(pdrgInfo,
                       &drgItem,
                       sizeof(DRAGITEM),
                       0);     // item index

        // fill in the DRAGIMAGE structure
        drgImage.cb       = sizeof(DRAGIMAGE);
        drgImage.hImage   = WinQuerySysPointer(HWND_DESKTOP,
                                               SPTR_FILE,
                                               FALSE);
                    // pointer used for dragging; we
                    // use a dull default file icon
        drgImage.fl       = DRG_ICON;             // hImage is an HPOINTER
        drgImage.cxOffset = 0; // 5 * iOffset;          // Image offset from mouse pointer
        drgImage.cyOffset = 0; // 5 * iOffset;          // Image offset from mouse pointer

        // set source emphasis for the container record
        WinSendMsg(hwndCnr,
                   CM_SETRECORDEMPHASIS,
                   (MPARAM)preccDrag,
                   MPFROM2SHORT(TRUE,
                       (usNotifyCode == CN_INITDRAG)
                           // for dragging: source emphasis
                           ? CRA_SOURCE
                           // for lazy drag: picked emphasis
                           : CRA_PICKED));

        if (usNotifyCode == CN_INITDRAG)
        {
            // "real" dragging:
            // call modal function for drag'n'drop.
            // This does not return until either
            // d'n'd has been cancelled or the
            // record core has been dropped.
            /* HWND hwndTarget = */ DrgDrag(hwndCnr,
                                      pdrgInfo,
                                      &drgImage,
                                      1,      // drag image count
                                      VK_ENDDRAG, // system drag button
                                      NULL);  // reserved

            // since it's the source which is responsible
            // for cleaning up the resources, this is
            // what we do now (we had a modal d'n'd)
            DrgDeleteDraginfoStrHandles(pdrgInfo);
            DrgFreeDraginfo(pdrgInfo);

            // remove source emphasis for the container record
            WinSendMsg(hwndCnr,
                       CM_SETRECORDEMPHASIS,
                       (MPARAM)preccDrag,
                       MPFROM2SHORT(FALSE,
                               CRA_SOURCE));
        } // end if (usNotifyCode == CN_INITDRAG)
        else
        {
            // "lazy drag" (CN_PICKUP):
            // initiate lazy drag. This function returns
            // immediately, since lazy drag is non-modal.
            // As a result, we cannot clean up the resources
            // here, but need to do this later (CN_DROPNOTIFY).
            DrgLazyDrag(hwndCnr,
                        pdrgInfo,
                        &drgImage,
                        1,      // drag image count
                        0);     // reserved
        }
    } // end if (pdrgInfo)

    return pdrgInfo;
}

/*
 *@@ cnrhOwnerDrawRecord:
 *      this helper func can be called upon receiving
 *      WM_DRAWITEM for owner-drawing container record
 *      cores.
 *
 *      What this draws depends on flFlags:
 *
 *      --  If CODFL_DISABLEDTEXT is set, this function supports
 *          drawing record cores in gray color if the record has
 *          the CRA_DISABLED style (which, AFAIK, PM ignores per
 *          default).
 *
 *      --  If CODFL_MINIICON is set, this function paints the
 *          record's mini-icon properly. So far I have failed
 *          to find out how to get a container to paint the
 *          correct mini-icon if the CV_MINI style is set...
 *          the container normally _does_ paint a small icon,
 *          but it won't use the "real" mini icon data if that
 *          is present.
 *
 *          For painting the mini-icon, WinDrawPointer is used
 *          with DP_MINI set properly and the RECORDCORE's
 *          hptrMiniIcon field.
 *
 *      This returns either TRUE or FALSE as an MPARAM, depending
 *      on whether we have drawn the item. This return value should
 *      also be the return value of your window procedure.
 *
 *@@changed V0.9.16 (2001-09-29) [umoeller]: added flFlags, icon draw support
 */

MRESULT cnrhOwnerDrawRecord(MPARAM mp2,     // in: mp2 of WM_DRAWITEM (POWNERITEM)
                            ULONG flFlags)  // in: CODFL_* flags
{
    MRESULT mrc = (MPARAM)FALSE; // tell cnr to draw the item

    // get generic DRAWITEM structure
    POWNERITEM poi;

    if (poi = (POWNERITEM)mp2)
    {
        // get container-specific draw-item struct
        PCNRDRAWITEMINFO pcdii = (PCNRDRAWITEMINFO)poi->hItem;

        // check if we're to draw the text
        // (and not the icon)
        if (    (poi->idItem == CMA_TEXT)
             && (flFlags & CODFL_DISABLEDTEXT)
           )
        {
            if (((pcdii->pRecord->flRecordAttr) & CRA_DISABLED) == 0)
            {
                /*
                // not disabled == valid WPS class
                if ((pcdii->pRecord->flRecordAttr) & CRA_SELECTED)
                {
                    // not disabled, but selected:
                    lBackground = winhQueryPresColor(hwndDlg, PP_HILITEBACKGROUNDCOLOR, SYSCLR_HILITEBACKGROUND);
                    lForeground = winhQueryPresColor(hwndDlg, PP_HILITEFOREGROUNDCOLOR, SYSCLR_HILITEFOREGROUND);
                }
                else
                {
                    // not disabled, not selected:
                    lBackground = winhQueryPresColor(hwndDlg, PP_BACKGROUNDCOLOR, SYSCLR_BACKGROUND);
                    lForeground = winhQueryPresColor(hwndDlg, PP_FOREGROUNDCOLOR, SYSCLR_WINDOWTEXT);
                } */
                mrc = FALSE;
                    // let cnr draw the thing
            }
            else
            {
                // CRA_DISABLED:

                ULONG flCmd = DT_LEFT | DT_TOP | DT_ERASERECT;
                RECTL rcl2;

                // set draw colors
                LONG  lBackground,
                      lForeground;

                // switch to RGB
                GpiCreateLogColorTable(poi->hps, 0, LCOLF_RGB, 0, 0, NULL);

                if ((pcdii->pRecord->flRecordAttr) & CRA_SELECTED)
                {
                    // disabled and selected:
                    lBackground = WinQuerySysColor(HWND_DESKTOP,
                                                   SYSCLR_SHADOWTEXT, 0);
                    lForeground = winhQueryPresColor(poi->hwnd,
                                                     PP_BACKGROUNDCOLOR,
                                                     FALSE, // no inherit
                                                     SYSCLR_WINDOW);
                }
                else
                {
                    // disabled and not selected:
                    lBackground = winhQueryPresColor(poi->hwnd,
                                                     PP_BACKGROUNDCOLOR,
                                                     FALSE,
                                                     SYSCLR_WINDOW);
                    lForeground = WinQuerySysColor(HWND_DESKTOP,
                                                   SYSCLR_SHADOWTEXT, 0);
                }

                memcpy(&rcl2, &(poi->rclItem), sizeof(rcl2));
                /* WinDrawText(poi->hps,
                            strlen(pcdii->pRecord->pszText),
                            pcdii->pRecord->pszText,
                            &rcl2,
                            lForeground,  // foreground
                            lBackground,
                            flCmd); */

                GpiSetBackColor(poi->hps, lBackground);
                GpiSetColor(poi->hps, lForeground);

                winhDrawFormattedText(poi->hps,
                                      &rcl2,
                                      pcdii->pRecord->pszText,
                                      flCmd);

                mrc = (MPARAM)TRUE;     // tell cnr that we've drawn the item
            }
        }
        else if (    (poi->idItem == CMA_ICON)
                  && (flFlags & CODFL_MINIICON)
                )
        {
            WinDrawPointer(poi->hps,
                           // center the icon in the rectangle
                           (   poi->rclItem.xLeft
                             + (poi->rclItem.xRight - poi->rclItem.xLeft
                                - G_cxIcon / 2
                               ) / 2
                           ),
                           (   poi->rclItem.yBottom
                             + (poi->rclItem.yTop - poi->rclItem.yBottom
                                - G_cyIcon / 2
                               ) / 2
                           ),
                           pcdii->pRecord->hptrMiniIcon,
                           DP_MINI);

            mrc = (MPARAM)TRUE;         // tell cnr that we've drawn the item
        }
    }

    return mrc;
}

/*
 *@@ cnrhDateTimeDos2Win:
 *      this converts the information in a Control Program
 *      DATETIME structure (as returned by DosGetDateTime)
 *      to the CDATE and CTIME structures used by containers.
 *
 *      If any of the target pointers is NULL, that data is
 *      not converted.
 */

BOOL cnrhDateTimeDos2Win(DATETIME* pdt,     // in: Dos format date and time
                         CDATE* pcdate,     // out: container format date
                         CTIME* pctime)     // out: container format time
{
    if (pdt)
    {
        if (pcdate)
        {
            pcdate->day = pdt->day;
            pcdate->month = pdt->month;
            pcdate->year = pdt->year;
        }
        if (pctime)
        {
            pctime->seconds = pdt->seconds;
            pctime->minutes = pdt->minutes;
            pctime->hours = pdt->hours;
        }
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ cnrhDateDos2Win:
 *      converts an FDATE structure (Control program)
 *      to a container CDATE structure.
 */

BOOL cnrhDateDos2Win(FDATE* pfd,    // in: DOS date
                     CDATE* pcd)    // out: container date
{
    if ((pfd) && (pcd))
    {
        pcd->day = pfd->day;
        pcd->month = pfd->month;
        pcd->year = pfd->year + 1980;
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ cnrhTimeDos2Win:
 *      converts an FTIME structure (Control program)
 *      to a container CTIME structure.
 */

BOOL cnrhTimeDos2Win(FTIME* pft,    // in: DOS time
                     CTIME* pct)    // out: container time
{
    if ((pft) && (pct))
    {
        pct->seconds = pft->twosecs * 2;
        pct->minutes = pft->minutes;
        pct->hours = pft->hours;
        pct->ucReserved = 0;
        return TRUE;
    }

    return FALSE;
}
