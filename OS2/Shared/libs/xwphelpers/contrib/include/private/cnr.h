/*
 *@@sourcefile cnr.h.h:
 *      private header file for cctl_cnr.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 */

/*      Copyright (C) 2003 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 */

#if __cplusplus
extern "C" {
#endif

#ifndef PRIVATE_CNR_HEADER_INCLUDED
    #define PRIVATE_CNR_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Global stuff
     *
     ********************************************************************/

    #define WC_CCTL_CNR_DETAILS     "ComctlCnrDtls"

    extern const CCTLCOLOR G_scsCnr[];

    #define CNRCOL_HILITEBGND   2
    #define CNRCOL_HILITEFGND   3
    #define CNRCOL_BORDER       4
    #define CNRCOL_EDITBGND     5
    #define CNRCOL_EDITFGND     6
    #define CNRCOL_SIZINGBOX    7

    #define COLUMN_PADDING_X        7
    #define COLUMN_PADDING_Y        3
                // padding to apply between column border and column data

    #define DEFAULT_BORDER_WIDTH    1
                // default width of lines used for details and tree view lines

    /*
     *@@ DETAILCOLUMN:
     *      private wrapper data around FIELDINFO.
     */

    typedef struct _DETAILCOLUMN
    {
        const FIELDINFO *pfi;           // ptr to fieldinfo for this column

        LONG        cxContent;          // current width of the column (excluding padding);
                                        // -- if pfi->cxWidth == 0, this is the computed auto-size;
                                        // -- otherwise this is a copy of the pfi->cxWidth value

        LONG        cxWidestRecord;     // width of widest record data in this column (excluding padding)

        SIZEL       szlTitleData;       // dimensions of title data (excluding padding)

        LONG        xLeft;              // left border of column in cnr workspace coordinates;
                                        // this includes spacing, i.e. is 0 for the leftmost column

    } DETAILCOLUMN, *PDETAILCOLUMN;

    /*
     *@@ RECORDLISTITEM:
     *      private wrapper data around an app's RECORDCORE
     *      that is currently inserted somewhere.
     *
     *      We create both a list node and a tree node for
     *      each record that is currently inserted.
     */

    typedef struct _RECORDLISTITEM
    {
        const RECORDCORE
                    *precc,             // ptr to app's RECORDCORE buffer
                    *preccParent;       // parent of record or NULL if root

        ULONG       flRecordAttr;       // private copy of record's attributes

        ULONG       flInvalidate;       // CMA_TEXTCHANGED et al flags from
                                        // CM_INVALIDATERECORD or 0 if record is fresh

        POINTL      ptl;                // position of record as calculated by us (in
                                        // workspace coordinates)

        SIZEL       szlContent;         // space that records needs in current cnr view
                                        // (excluding padding)
        SIZEL       szlBox;             // space that records needs in current cnr view
                                        // (including padding)
        // LONG        yOfs;               // y offset of the top of this record's rectangle
                                        // from the top of the cnr workspace. Positive
                                        // values mean the record is further down.
                                        // In Details view, the topmost record has an
                                        // offset of 0.

    } RECORDLISTITEM, *PRECORDLISTITEM;

    /*
     *@@ RECORDTREEITEM:
     *      second private wrapper data around a
     *      RECORDCORE that is currently inserted somewhere.
     *
     *      We create both a list node and a tree node for
     *      each record that is currently inserted.
     */

    typedef struct _RECORDTREEITEM
    {
        TREE        Tree;               // ulKey is app's PRECORDCORE
        PLISTNODE   pListNode;          // points to the LISTNODE corresponding to record
    } RECORDTREEITEM, *PRECORDTREEITEM;

    /*
     *@@ CNRDATA:
     *      private container data, allocated on WM_CREATE and
     *      stored at QWL_USER + 1 in the container main and
     *      subwindows.
     */

    typedef struct _CNRDATA
    {
        DEFWINDATA  dwdMain,
                    dwdContent;

        BOOL        fMiniRecords;       // set to TRUE on WM_CREATE if we have the
                                        // CCS_MINIRECORDCORE style bit set

        CNRINFO     CnrInfo;            // private container info struct

        LINKLIST    llAllocatedFIs;     // contains PFIELDINFO's that were allocated, auto-free
        LINKLIST    llColumns;          // contains PDETAILCOLUMN's with FIELDINFO's that are
                                        // currently inserted, auto-free

        LINKLIST    llAllocatedRecs;    // contains PRECORDCORE's that were allocated, auto-free
        LINKLIST    llRootRecords;      // contains PRECORDLISTITEM's with RECORDCORE's that are
                                        // currently inserted at root level (i.e. have no parent
                                        // record), auto-free
        TREE        *RecordsTree;       // tree of _all_ currently inserted RECORDLISTITEM's;
                                        // we use CnrInfo.cRecords for the tree count

        // paint data

        COUNTRYSETTINGS2 cs;            // current NLS settings for painting CMA_DATE etc.

        FONTMETRICS fm;                 // font metrics of current font

        LONG        cyColTitlesContent, // if CnrInfo.flWindowAttr & CA_DETAILSVIEWTITLES, height of
                                        // column titles area (excluding padding and horz. separator)
                    cyColTitlesBox;     // ... and including those

        BOOL        fSettingPP;         // anti-recursion flag

        // scrolling data

        SCROLLABLEWINDOW scrw;

        // selection data

        PRECORDLISTITEM
                    prliCursored;       // currently cursored record (there can only be one ever)

        PRECORDLISTITEM
                    prliSwipingFirst;   // set to record on which swipe select started; NULL if
                                        // we're not currently swipe-selecting
        BOOL        fSwipeTurnOn;       // if TRUE, we select records while swiping; if FALSE,
                                        // we deselect

    } CNRDATA, *PCNRDATA;

    // bits for WM_SEM2
    #define DDFL_INVALIDATECOLUMNS      0x0001
                // no. of columns changed, recompute column data

    #define DDFL_INVALIDATERECORDS      0x0002
                // record data changed, recompute all records and adjust columns

    #define DDFL_INVALIDATESOME         0x0004
                // some records changed, recompute only those that have szlContent.x == -1

    #define DDFL_WINDOWSIZECHANGED      0x0008
                // subwindows need repositioning

    #define DDFL_WORKAREACHANGED        0x0010
                // cnr workarea changed, adjust scrollbars

    #define DDFL_ALL                    0xFFFF

    /* ******************************************************************
     *
     *   Cnr details view stuff
     *
     ********************************************************************/

    VOID ctnrInit(HWND hwnd,
                  MPARAM mp2,
                  ULONG flMainCnrStyle,
                  PDEFWINDATA pdwd);

    VOID ctnrDrawString(HPS hps,
                       PCSZ pcsz,
                       PRECTL prcl,
                       ULONG fl,
                       PFONTMETRICS pfm);

    VOID ctnrGetRecordRect(PCNRDATA pData,
                          PRECTL prcl,
                          const RECORDLISTITEM *prli);

    BOOL ctnrRepaintRecord(PCNRDATA pData,
                           const RECORDLISTITEM *prli);

    ULONG ctnrQuerySelMode(PCNRDATA pData);

    BOOL ctnrChangeEmphasis(PCNRDATA pData,
                            PRECORDLISTITEM prliSet,
                            BOOL fTurnOn,
                            ULONG fsEmphasis);

    VOID ctnrPresParamChanged(HWND hwnd,
                             ULONG ulpp);

    BOOL ctnrSetRecordEmphasis(PCNRDATA pData,
                               PRECORDCORE precc,
                               BOOL fTurnOn,
                               USHORT fsEmphasis,
                               BOOL fMouse);

    VOID ctnrRecordEnter(PCNRDATA pData,
                         const RECORDLISTITEM *prli,
                         BOOL fKeyboard);

    PLISTNODE ctnrFindListNodeForRecc(PCNRDATA pData,
                                      const RECORDCORE *precc);

    VOID cdtlRecalcDetails(PCNRDATA pData,
                           HPS hps,
                           PULONG pfl);

    MRESULT EXPENTRY fnwpCnrDetails(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#endif

#if __cplusplus
}
#endif

