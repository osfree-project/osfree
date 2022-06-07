
/*
 *@@sourcefile dialog.c:
 *      contains PM helper functions to create and
 *      auto-format dialogs from control arrays in memory.
 *
 *      As a summary, this code allows you to get rid of
 *      PM dialog resources completely.
 *
 *      See dlghCreateDlg for an introduction.
 *
 *      See @dlg_algorithm for the gory details of the new
 *      algorithm used since V1.0.0. Even though much
 *      of this file was rewritten, the new dialog is
 *      backwards-compatible with all the hacks that existing
 *      dialogs might use for aligning things properly.
 *
 *      In addition, this has dlghMessageBox (a WinMessageBox
 *      replacement) and some helper functions for simulating
 *      dialog behavior in regular window procs (see
 *      dlghSetPrevFocus and others).
 *
 *      If you are out to find all workarounds to get certain
 *      buggy PM controls aligned right, this file is definitely
 *      the place.
 *
 *      Usage: All PM programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  dlg*   Dialog functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@added V0.9.9 (2001-04-01) [umoeller]
 *@@header "helpers\dialog.h"
 */

/*
 *      Copyright (C) 2001-2006 Ulrich M”ller.
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

// #define DEBUG_DIALOG_WINDOWS 1
                // blue frame is table
                // green frame is column
                // red frame is control

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WININPUT
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDCNR
#define INCL_WINSYS

#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#define INCL_GPILCIDS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\comctl.h"
#include "helpers\dialog.h"
#include "helpers\except.h"
#include "helpers\gpih.h"
#include "helpers\linklist.h"
#include "helpers\nlscache.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\textview.h"
#include "helpers\winh.h"
#include "helpers\xstring.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Dialog templates
 */

/* ******************************************************************
 *
 *   Glossary entries
 *
 ********************************************************************/

/*
 *@@gloss: dlg_using_macros Using the Dialog Formatter Macros
 *
 *      The DLGHITEM macros are:
 *
 *      --  START_TABLE starts a new table. The tables may nest,
 *          but must each be properly terminated with END_TABLE.
 *
 *          Note that as opposed to HTML tables, the columns
 *          in the rows of the table are NOT aligned under each
 *          other. If that is what you want, use START_TABLE_ALIGN
 *          instead.
 *
 *      --  START_GROUP_TABLE(pDef) starts a group. This
 *          behaves exacly like START_TABLE, but in addition,
 *          it produces a static group control around the table.
 *          Useful for group boxes. pDef must point to a
 *          CONTROLDEF describing the control to be used for
 *          the group (usually a WC_STATIC with SS_GROUP style),
 *          whose size parameter is ignored.
 *
 *          As with START_TABLE, START_GROUP_TABLE must be
 *          terminated with END_TABLE.
 *
 *          As with START_TABLE, columns in the rows of the table
 *          are NOT aligned under each other. If that is what you
 *          want, use START_GROUP_TABLE_ALIGN instead.
 *
 *      --  START_ROW(fl) starts a new row in a table (regular
 *          or group). This must also be the first item after
 *          the (group) table tag.
 *
 *          fl specifies formatting flags for the row. This
 *          can be one of ROW_VALIGN_BOTTOM, ROW_VALIGN_CENTER,
 *          ROW_VALIGN_TOP and affects all items in the row.
 *
 *      --  CONTROL_DEF(pDef) defines a control in a table row.
 *          pDef must point to a CONTROLDEF structure.
 *
 *          Again, there is is NO information in CONTROLDEF
 *          about a control's _position_. Instead, the structure
 *          only contains the _size_ of the control. All
 *          positions are computed by this function, depending
 *          on the sizes of the controls and their nesting within
 *          the various tables.
 *
 *          If you specify SZL_AUTOSIZE with either cx or cy
 *          or both, the size of the control is even computed
 *          automatically. Presently, this only works for statics
 *          with SS_TEXT, SS_ICON, and SS_BITMAP, push buttons,
 *          and radio and check boxes.
 *
 *          Unless separated with START_ROW items, subsequent
 *          control items will be considered to be in the same
 *          row (== positioned next to each other).
 *
 *          Columns will only be aligned horizontally if the
 *          container table was specified with START_TABLE_ALIGN
 *          or START_GROUP_TABLE_ALIGN.
 *
 *      There are a few rules, whose violation will produce
 *      an error:
 *
 *      --  The entire array must be enclosed in a table
 *          (the "root" table).
 *
 *      --  After START_TABLE or START_GROUP_TABLE, there must
 *          always be a START_ROW first.
 *
 *      While it is possible to set up the CONTROLDEFs manually
 *      as static structures, I recommend using the bunch of
 *      other macros that were defined in dialog.h for this.
 *      For example, you can use CONTROLDEF_PUSHBUTTON to create
 *      a push button, and many more.
 *
 *      To create a dialog, set up arrays like the following:
 *
 +          // control definitions referenced by DlgTemplate:
 +          CONTROLDEF
 +      (1)             GroupDef = CONTROLDEF_GROUP("Group",
 +                                                  -1,     // ID
 +                                                  SZL_AUTOSIZE,
 +                                                  SZL_AUTOSIZE),
 +      (2)             CnrDef = CONTROLDEF_CONTAINER(-1,   // ID,
 +                                                  50,
 +                                                  50),
 +      (3)             Static = CONTROLDEF_TEXT("Static below cnr",
 +                                                  -1,     // ID
 +                                                  SZL_AUTOSIZE,
 +                                                  SZL_AUTOSIZE),
 +      (4)             OKButton = CONTROLDEF_DEFPUSHBUTTON("~OK",
 +                                                  DID_OK,
 +                                                  SZL_AUTOSIZE,
 +                                                  SZL_AUTOSIZE),
 +      (5)             CancelButton = CONTROLDEF_PUSHBUTTON("~Cancel",
 +                                                  DID_CANCEL,
 +                                                  SZL_AUTOSIZE,
 +                                                  SZL_AUTOSIZE);
 +
 +          DLGHITEM DlgTemplate[] =
 +              {
 +                  START_TABLE,            // root table, required
 +                      START_ROW(0),       // row 1 in the root table, required
 +                          // create group on top
 +      (1)                 START_GROUP_TABLE(&Group),
 +                              START_ROW(0),
 +      (2)                         CONTROL_DEF(&CnrDef),
 +                              START_ROW(0),
 +      (3)                         CONTROL_DEF(&Static),
 +                          END_TABLE,      // end of group
 +                      START_ROW(0),       // row 2 in the root table
 +                          // two buttons next to each other
 +      (4)                 CONTROL_DEF(&OKButton),
 +      (5)                 CONTROL_DEF(&CancelButton),
 +                  END_TABLE
 +              }
 *
 *      This will produce a dlg like this:
 *
 +          ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 +          º                                   º
 +          º ÚÄ Group (1) ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿  º
 +          º ³                              ³  º
 +          º ³  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿  ³  º
 +          º ³  ³                        ³  ³  º
 +          º ³  ³  Cnr inside group (2)  ³  ³  º
 +          º ³  ³                        ³  ³  º
 +          º ³  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ  ³  º
 +          º ³                              ³  º
 +          º ³  Static below cnr (3)        ³  º
 +          º ³                              ³  º
 +          º ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ  º
 +          º                                   º
 +          º ÚÄÄÄÄÄÄÄÄÄÄÄ¿ ÚÄÄÄÄÄÄÄÄÄÄÄÄÄ¿     º
 +          º ³   OK (4)  ³ ³  Cancel (5) ³     º
 +          º ÀÄÄÄÄÄÄÄÄÄÄÄÙ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÙ     º
 +          º                                   º
 +          ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
 */

/*
 *@@gloss: dlg_boxmodel Dialog formatter box model
 *
 *      We now use the CSS box model for the dialog
 *      formatter (V1.0.0).
 *
 +      ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 +      ³  margin                  ³
 +      ³  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿  ³
 +      ³  ³# border ###########³  ³
 +      ³  ³##ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿##³  ³
 +      ³  ³##³  padding     ³##³  ³
 +      ³  ³##³  ÚÄÄÄÄÄÄÄÄÄ¿ ³##³  ³
 +      ³  ³##³  ³ content ³ ³##³  ³
 +      ³  ³##³  ÀÄÄÄÄÄÄÄÄÄÙ ³##³  ³
 +      ³  ³##³              ³##³  ³
 +      ³  ³##ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ##³  ³
 +      ³  ³####################³  ³
 +      ³  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ  ³
 +      ³                          ³
 +      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
 *
 *      See "http://www.w3.org/TR/REC-CSS1#formatting-model".
 *
 *      Remark on the box model for subtables:
 *
 *      We do not want to apply spacing twice for nested tables. This
 *      is especially important for group boxes (i.e. tables which
 *      have a PM group static control created around them). As a
 *      result, the definition is that ONLY COLUMNS HAVE SPACING,
 *      EVER. Neither tables nor rows do.
 */

/*
 *@@gloss: dlg_algorithm Dialog formatter algorithm
 *
 *      See @dlg_boxmodel for information about how the
 *      rectangles are defined.
 *
 *      The dialog formatter allows you to specify PM dialog layouts
 *      as nested tables, without having to specify a single control
 *      position. In fact, it is impossible to give the dialog
 *      formatter fixed positions. Instead, you give the formatter
 *      the control _sizes_, and everything is layed out automatically.
 *      You may even have the formatter compute the sizes automatically
 *      based on the control classes and values (strings); it is possible
 *      to create dialogs without specifying a single size also.
 *
 *      There are several tricks to how this works.
 *
 *      1)  The dialog is supposed to consist of one outer table,
 *          which in turn has rows. The rows in turn have columns.
 *          We process the outer table by iterating over its rows
 *          (see ProcessTable). For each row, we iterate over the
 *          columns (see ProcessRow). Most of the processing then
 *          actually occurs in ProcessColumn.
 *
 *      2)  Each column can contain a PM control, as specified in a
 *          CONTROLDEF structure on input. Alternatively, a column
 *          can be a nested subtable (with more rows and columns).
 *          If so, ProcessColumn recurses back into ProcessTable
 *          (which again goes into ProcessRow and ProcessColumn).
 *          There is no limit to how deep tables may nest, except
 *          for the stack size of the current thread, of course.
 *
 *      3)  This whole recursive iteration is performed several times.
 *          In fact, we run through the entire data up to seven times
 *          presently. Only in the last run, we actually create the
 *          controls. Details follow.
 *
 *      4)  We have sophisticated means for computing the size of
 *          certain controls automatically. See CalcAutoSize.
 *
 *      The algorithm used here has been greatly reworked with V1.0.0.
 *      Essentially, the new trick is that we first compute all widths
 *      (which requires several iterations since widths may now inherit
 *      from each other), then all heights (which, for many controls,
 *      depend on the widths), then all positions. The previous approach
 *      to calculate widths and heights at the same time has turned out
 *      to not work very well.
 *
 *      With each of the seven recursive iterations, ProcessTable,
 *      ProcessRow, and ProcessColumn receive a ProcessMode variable,
 *      which is just an integer that is raised with each run. These
 *      are the PROCESS_* constants all over the code.
 *
 *      In the seven iterations, the following happens:
 *
 *      1)  PROCESS_1_CALC_MIN_WIDTHS: calculates cxMinContent of all
 *          columns, rows, and tables from the inside to the outside
 *          recursively. cxMinContent specifies the minimum width of
 *          the column, row, and table, respectively. This value is
 *          required for all subsequent computations.
 *
 *          This works as follows:
 *
 *          --  From the root table, process all rows. [1]
 *
 *              --  From each row, process all columns.
 *
 *                  --  For each column, calculate the minimum size of
 *                      the content.
 *
 *                      (*) If the column is a control leaf node (i.e.
 *                          does not hold a subtable), get the minimum
 *                          size of the control.
 *
 *                          (*) If the CONTROLDEF specified an explicit
 *                              width, copy it to cxMinContent.
 *
 *                          (*) If the CONTROLDEF specified SZL_AUTOSIZE
 *                              for cx, evaluate the control type (icon,
 *                              bitmap, text etc.) and set the minimum size
 *                              to the result. See CalcAutoSize.
 *
 *                          (*) The control may specify its desired width
 *                              as a percentage of the parent table's
 *                              width. In that case, set the cxMinContent
 *                              to 0 for now because it is to be ignored
 *                              in the computation of the parent row and
 *                              table widths.
 *
 *                      (*) If the column is a subtable, recurse with
 *                          the subtable ([1] above) by calling ProcessTable
 *                          again. Then set the control's cxMinContent to
 *                          the table's cxMinBox, which was set by the
 *                          subtable's ProcessTable (see below).
 *
 *                      After this, we have set the column's cxMinContent
 *                      to either a minimum size (explicit or automatic )
 *                      or 0.
 *
 *                  --  Set the row's cxMinBox to the sum of the minimum
 *                      widths of the contained columns, with padding,
 *                      border, and margin applied to each column.
 *
 *              --  Set the table's cxMinBox to the cxMinBox of the table's
 *                  widest row.
 *
 *      2)  PROCESS_2_CALC_RELATIVE_WIDTHS: calculates the widths of
 *          those columns that have specified that their size should
 *          be a percentage of the parent table's width, and calculate
 *          the content rectangles of all columns and the box rectangles
 *          of the parent rows and tables. Again, do this recursively
 *          from the inside columns to the outside tables.
 *
 *          First, we inherit table sizes from parent tables in
 *          ProcessTable. If a column has a subtable that has the
 *          TABLE_INHERIT_SIZE table flag set, compute a new cxMinBox
 *          for that table. _After_ that, recurse into the table to allow
 *          subtables of that subtable to inherit that size as well.
 *
 *          In the columns, this then sets cpContent.cx in the columns,
 *          with the following processing in the control leaf columns:
 *
 *          (*) If the control has specified an explicit or automatic
 *              width, we have set cxMinContent in PROCESS_1_CALC_MIN_WIDTHS;
 *              copy that width to cpContent.cx.
 *
 *          (*) However, if the control had specified its desired
 *              width as a percentage of the parent table's width,
 *              calculate the cpContent.cx for the control from the
 *              table's cxMinBox (from PROCESS_1_CALC_MIN_WIDTHS).
 *
 *          In the rows and tables, cpBox.cx and cpTable.cx is set as
 *          above from the column cpContent.cx values, with spacing applied.
 *
 *      3)  PROCESS_3_ALIGN_COLUMNS: if the table has the TABLE_ALIGN_COLUMNS
 *          flag set, for each column in the table, find the widest
 *          instance in all rows, and set the cpContent.cx of all
 *          columns in that row to that width.
 *
 *      4)  PROCESS_4_REMAINDER_WIDTHS: for all controls that have
 *          SZL_REMAINDER set for their widths, fill those so that
 *          they can use up the remaining width in the row.
 *
 *      5)  PROCESS_5_CALC_HEIGHTS: Now that we know all widths,
 *          calculate the heights. We set cpContent.cy of all columns,
 *          cpRowBox.cy of all rows, and cpTableBox.cy of the tables,
 *          with the following processing in the control leaf columns:
 *
 *          (*) If the control has specified an explicit or automatic
 *              height, use or compute that. In the case of autosize
 *              height, we now know the final width of the control,
 *              which is important for computing the height of multi-line
 *              static controls (which depend on the width, obviously).
 *              The new approach finally works for the case where a
 *              multi-line static control specifies its width as a
 *              percentage of the parent table, but still wants an
 *              autosize height.
 *
 *      6)  PROCESS_6_CALC_POSITIONS: Now that we know all heights and
 *          widths, for each column, calculate cpContent.x and y.
 *
 *      7)  PROCESS_7_CREATE_CONTROLS: finally creates the controls
 *          at the cpContent.x and y position, with the cpContent.cx
 *          and cy size.
 */

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

/*
 *@@ DLGPRIVATE:
 *      private data to the dlg manager, allocated
 *      by dlghCreateDlg.
 *
 *      This only exists while the dialog is being
 *      created and is not stored with the new dialog.
 */

typedef struct _DLGPRIVATE
{
    HWND        hwndDlg;                // dialog frame

    // definition data
    LINKLIST    llTables;               // linked list of TABLEDEF structs

    HWND        hwndFirstFocus,
                hwndDefPushbutton;      // V0.9.14 (2001-08-21) [umoeller]

    POINTL      ptlTotalOfs;

    PLINKLIST   pllControls;            // linked list of HWNDs in the order
                                        // in which controls were created;
                                        // ptr can be NULL

    PCSZ        pcszControlsFont;       // from dlghCreateDlg

    // size of the client to be created
    SIZEL       szlClient;

    // various cached data V0.9.14 (2001-08-01) [umoeller]
    HPS         hps;
    PCSZ        pcszFontLast;
    LONG        lcidLast;
    FONTMETRICS fmLast;

    LONG        cxBorder,
                cyBorder;               // cached now V0.9.19 (2002-04-17) [umoeller]

    double      dFactorX,               // correlation factors for dialog units
                dFactorY;               // V0.9.19 (2002-04-24) [umoeller]

    ULONG       flNeedsProcessing;      // flags to avoid unnecessary recursions:
            #define FL_REMAINDER_WIDTHS       0x0001
                        // dialog needs PROCESS_4_REMAINDER_WIDTHS processing

} DLGPRIVATE, *PDLGPRIVATE;

// macros for the dlg units conversion;
#define FACTOR_X     (pDlgData->dFactorX)
#ifdef USE_SQUARE_CORRELATION
#define FACTOR_Y     (pDlgData->dFactorX)
#else
#define FACTOR_Y     (pDlgData->dFactorY)
#endif

typedef struct _COLUMNDEF *PCOLUMNDEF;
typedef struct _ROWDEF *PROWDEF;
typedef struct _TABLEDEF *PTABLEDEF;

/*
 *@@ CONTROLPOS:
 *      control position. We don't want to use SWP.
 *      We could have used RECTL, but the xRight
 *      etc. fields are always confusing with what
 *      we're doing... so we just define this.
 */

typedef struct _CONTROLPOS
{
    LONG        x,
                y,
                cx,
                cy;
} CONTROLPOS, *PCONTROLPOS;

/*
 *@@ enProcessMode:
 *      enumeration that specifies the current processing
 *      mode while recursing through the tables, rows,
 *      and columns.
 *
 *      For each processing mode, ProcessAll gets called
 *      while the dialog is being set up, which in turn
 *      calls ProcessTable for all tables, which in turn
 *      calls ProcessRow for all rows in that table,
 *      which in turn calls ProcessColumn for all rows
 *      in that column. ProcessColumn will recurse back
 *      into ProcessTable if a column represents a
 *      subtable.
 *
 *      See @dlg_algorithm for what processing takes
 *      place with each of the process modes.
 */

typedef enum _enProcessMode
{
    PROCESS_1_CALC_MIN_WIDTHS,
    PROCESS_2_CALC_RELATIVE_WIDTHS,
    PROCESS_3_ALIGN_COLUMNS,
    PROCESS_4_REMAINDER_WIDTHS,
    PROCESS_5_CALC_HEIGHTS,
    PROCESS_6_CALC_POSITIONS,
    PROCESS_7_CREATE_CONTROLS
} enProcessMode;

/*
 *@@ COLUMNDEF:
 *      representation of a column in a table row.
 *
 *      Each COLUMNDEF is stored in a linked list
 *      in ROWDEF.llColumns.
 *
 *      Such a column represents either a PM control
 *      window or another table, which may therefore
 *      be nested.
 *
 *      See @dlg_algorithm for the new algorithm
 *      used since V1.0.0.
 *
 *      With V1.0.0, we now follow the CSS box model
 *      to render these things better without all
 *      the hacks I am no longer able to understand
 *      myself. See @dlg_box_model for details.
 *
 *@@changed V1.0.0 (2002-08-18) [umoeller]: completely replaced
 */

typedef struct _COLUMNDEF
{
    // 1) input table set up by the DLGHITEM parser (CreateColumn):

    // a) column position in recursion hierarchy

    PROWDEF     pOwningRow;         // row whose linked list this column belongs to

    PTABLEDEF   pNestedTable;       // if != NULL, this column represents a nested table,
                                    // and this points to the contained table definition
                                    // with which we must recurse back into ProcessTable().
                                    // In that case, if (pcszClass != NULL) also, it
                                    // must represent the PM static group control to
                                    // create around the table.
                                    // If (pNestedTable == NULL), this column is a control
                                    // leaf node, and pcszClass and the following represent
                                    // the control to create for the column.

    ULONG       ulColumnIndex;      // zero-based column index in parent row
                                    // V1.0.0 (2002-08-18) [umoeller]

    // b) information for control window to be created:

    PCSZ        pcszClass;          // class of control to create; if NULL, create no
                                    // control; NULL is valid only for subtable columns
    PCSZ        pcszText;           // window text; if NULL, it is replaced with "" in
                                    // ColumnCreateControl
    ULONG       flStyle;            // window style (should have at least WS_VISIBLE)
    USHORT      usID;               // window ID or -1 for "don't care"
    PCSZ        pcszFont;           // font to use for this window; this has CTL_COMMON_FONT
                                    // resolved to the dialog default font already
    SIZEL       szlProposed;        // proposed size for the control; copied from the
                                    // CONTROLDEF with the dialog units already converted
                                    // to pixels. Can be
                                    // -- SZL_AUTOSIZE (-1) if the control wants to be autosized,
                                    // -- SZL_REMAINDER (0) if the control wants the remaining
                                    //    space in its column,
                                    // -- a negative number from [-100, -1] to specify the width
                                    //    as a percentage of the parent table _box_.
    PVOID       pvCtlData;          // control-specific data for WinCreateWindow

    // c) spacing:

    RECTL       rclPadding,         // spacing, in pixels, to apply to the four borders of
                rclMargin;          // the content to get the box; calculated from CONTROLDEF
                                    // V1.0.0 (2002-08-16) [umoeller]

    LONG        cxSpacingTotal;     // sum of rclPadding and rclMargin xLeft and
                                    // xRight, respectively

    // 2) data from CalcAutoSize:

    SIZEL       szlAuto;

    // 3) result data after recursion:

    LONG        cxMinContent;       // minimum width of the content box;
                                    // computed in PROCESS_1_CALC_MIN_WIDTHS

    CONTROLPOS  cpContent;          // cx computed in PROCESS_2_CALC_RELATIVE_WIDTHS,
                                    // cy computed in PROCESS_5_CALC_HEIGHTS,
                                    // x and y computed in PROCESS_6_CALC_POSITIONS;
                                    // this is where the control will be created

    CONTROLPOS  cpBox;              // cx and cy computed in PROCESS_5_CALC_HEIGHTS,
                                    // x and y computed in PROCESS_6_CALC_POSITIONS

    HWND        hwndControl;        // created control after PROCESS_7_CREATE_CONTROLS;
                                    // for tables, this is only set if we have created
                                    // a PM group around them. Note: For text box
                                    // controls, we create the control earlier,
                                    // in CalcAutoSizeTextView during PROCESS_5_CALC_HEIGHTS,
                                    // because only then we can ask the control for
                                    // its required height.

} COLUMNDEF;

/*
 *@@ ROWDEF:
 *      representation of a row in a table.
 *
 *      Each ROWDEF is stored in a linked list in
 *      TABLEDEF.llRows and in turn contains a
 *      linked list of the contained COLUMNDEF structs
 *      in its llColumns member.
 */

typedef struct _ROWDEF
{
    // input table set up by the DLGHITEM parser:

    PTABLEDEF   pOwningTable;       // table whose linked list this row belongs to

    LINKLIST    llColumns;          // contains COLUMNDEF structs, no auto-free

    ULONG       flRowFormat;        // one of:
                                    // -- ROW_VALIGN_BOTTOM           0x0000
                                    // -- ROW_VALIGN_CENTER           0x0001
                                    // -- ROW_VALIGN_TOP              0x0002

    ULONG       cColumns;           // no. of columns in this row
                                    // V1.0.0 (2002-08-18) [umoeller]

    // result data after recursion:

    LONG        cxMinBox;           // set by PROCESS_1_CALC_MIN_WIDTHS to the sum of
                                    // the minimum widths of the contained columns,
                                    // with padding, border, and margin applied to each
                                    // column

    CONTROLPOS  cpRowBox;           // cx computed in PROCESS_2_CALC_RELATIVE_WIDTHS,
                                    // cy computed in PROCESS_5_CALC_HEIGHTS,
                                    // x and y computed in PROCESS_6_CALC_POSITIONS

    ULONG       cWantRemainders;    // count of controls in the row that have
                                    // SZL_REMAINDER set; counted in ProcessColumn
                                    // with PROCESS_2_CALC_RELATIVE_WIDTHS
                                    // so that we can quickly process
                                    // PROCESS_4_REMAINDER_WIDTHS

    LONG        cxNonRemainder;     // sum of the widths of those controls that do
                                    // not have SZL_REMAINDER set; counted in
                                    // ProcessColumn with PROCESS_3_ALIGN_COLUMNS
                                    // so that we can quickly process
                                    // PROCESS_4_REMAINDER_WIDTHS

} ROWDEF;

/*
 *@@ TABLEDEF:
 *      representation of a table. If pOwningColumn
 *      is NULL, this is the dialog's root table.
 *      Otherwise this is a nested subtable in the
 *      specified column.
 *
 *      Each TABLEDEF has its contained rows
 *      in the llRows member.
 */

typedef struct _TABLEDEF
{
    // input table set up by the DLGHITEM parser:

    PCOLUMNDEF  pOwningColumn;      // != NULL if this is a nested table

    LINKLIST    llRows;             // contains ROWDEF structs, no auto-free

    ULONG       flTable;            // TABLE_* flags, copied from DLGHITEM

    // result data after recursion:

    LONG        cxMinBox;           // set by PROCESS_1_CALC_MIN_WIDTHS to the cxMinBox of
                                    // the widest row

    PROWDEF     pWidestRow;         // used during PROCESS_2_CALC_RELATIVE_WIDTHS to
                                    // calculate percentage widths

    CONTROLPOS  cpTableBox;         // cx computed in PROCESS_2_CALC_RELATIVE_WIDTHS,
                                    // cy computed in PROCESS_5_CALC_HEIGHTS,
                                    // x and y computed in PROCESS_6_CALC_POSITIONS

} TABLEDEF;

/* ******************************************************************
 *
 *   Debug frames
 *
 ********************************************************************/

#ifdef DEBUG_DIALOG_WINDOWS

STATIC PFNWP G_pfnwpDebugStaticOrig = NULL;

/*
 *@@ fnwpDebugFrame:
 *      window proc for the subclassed static that is used in
 *      debug mode to mark rectangles in the dialog. This
 *      was added to support painting dotted rectangles. The
 *      line type is expected to sit in QWL_USER. We use
 *      PP_FOREGROUNDCOLOR as the paint color for the rectangle.
 *
 *      This is only compiled if DEBUG_DIALOG_WINDOWS is
 *      set.
 *
 *@@added V1.0.0 (2002-08-16) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpDebugFrame(HWND hwndBox, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_PAINT:
        {
            LONG lcol = winhQueryPresColor2(hwndBox,
                                            PP_FOREGROUNDCOLOR,
                                            PP_FOREGROUNDCOLORINDEX,
                                            FALSE,
                                            -1);
            LONG lLineType = WinQueryWindowULong(hwndBox, QWL_USER);

            HPS hps;
            if (hps = WinBeginPaint(hwndBox, NULLHANDLE, NULL))
            {
                RECTL rcl;
                POINTL ptl;
                WinQueryWindowRect(hwndBox, &rcl);

                gpihSwitchToRGB(hps);

                GpiSetColor(hps, lcol);
                GpiSetLineType(hps, lLineType);
                ptl.x = 0;
                ptl.y = 0;
                GpiMove(hps, &ptl);
                ptl.x = rcl.xRight - 1;
                ptl.y = rcl.yTop - 1;
                GpiBox(hps,
                       DRO_OUTLINE,
                       &ptl,
                       0,
                       0);
                WinEndPaint(hps);
            }
        }
        break;

        default:
            mrc = G_pfnwpDebugStaticOrig(hwndBox, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ CreateDebugFrame:
 *      in debug mode, creates a subclassed static that
 *      shows the rectangles that were produced in the
 *      dialog. See fnwpDebugFrame.
 *
 *      This is only compiled if DEBUG_DIALOG_WINDOWS is
 *      set.
 *
 *@@added V1.0.0 (2002-08-16) [umoeller]
 */

HWND CreateDebugFrame(HWND hwndParent,
                      LONG x,
                      LONG y,
                      LONG cx,
                      LONG cy,
                      LONG lcol,
                      LONG lLineType)
{
    HWND hwndDebug;
    if (hwndDebug = WinCreateWindow(hwndParent,
                                    WC_STATIC,
                                    "",
                                    WS_VISIBLE,
                                    x,
                                    y,
                                    cx,
                                    cy,
                                    hwndParent,     // owner
                                    HWND_BOTTOM,
                                    -1,
                                    NULL,
                                    NULL))
    {
        G_pfnwpDebugStaticOrig = WinSubclassWindow(hwndDebug, fnwpDebugFrame);
        winhSetPresColor(hwndDebug, PP_FOREGROUNDCOLOR, lcol);
        WinSetWindowULong(hwndDebug, QWL_USER, lLineType);
    }

    return hwndDebug;
}

#endif // DEBUG_DIALOG_WINDOWS

/* ******************************************************************
 *
 *   Column processing
 *
 ********************************************************************/

// forward declaration so we can recurse into tables from columns

STATIC APIRET ProcessTable(PTABLEDEF pTableDef,
                           const CONTROLPOS *pcpTableBox,
                           enProcessMode ProcessMode,
                           PDLGPRIVATE pDlgData);

/*
 *@@ SetDlgFont:
 *      refreshes the cached font data in DLGPRIVATE
 *      for the given font. Used all the time with
 *      CalcAutoSize.
 *
 *      In DLGPRIVATE, this modifies hps, lcidLast,
 *      and pcszFontLast. After this, the FONTMETRICS
 *      in fmLast are set.
 *
 *@@added V0.9.16 (2001-10-11) [umoeller]
 */

STATIC VOID SetDlgFont(PCSZ pcszFontThis,
                       PDLGPRIVATE pDlgData)
{
    LONG lPointSize = 0;

    if (!pDlgData->hps)
        pDlgData->hps = WinGetPS(pDlgData->hwndDlg);

    // check if we can reuse font data from last time
    // V0.9.14 (2001-08-01) [umoeller]
    if (strhcmp(pcszFontThis,               // can be NULL!
                pDlgData->pcszFontLast))
    {
        // different font than last time:

        // delete old font?
        if (pDlgData->lcidLast)
        {
            GpiSetCharSet(pDlgData->hps, LCID_DEFAULT);     // LCID_DEFAULT == 0
            GpiDeleteSetId(pDlgData->hps, pDlgData->lcidLast);
        }

        if (pcszFontThis)
        {
            // create new font
            pDlgData->lcidLast = gpihFindPresFont(NULLHANDLE,        // no window yet
                                                  FALSE,
                                                  pDlgData->hps,
                                                  pcszFontThis,
                                                  &pDlgData->fmLast,
                                                  &lPointSize);

            GpiSetCharSet(pDlgData->hps, pDlgData->lcidLast);
            if (pDlgData->fmLast.fsDefn & FM_DEFN_OUTLINE)
                gpihSetPointSize(pDlgData->hps, lPointSize);
        }
        else
        {
            // use default font:
            // @@todo handle presparams, maybe inherited?
            GpiSetCharSet(pDlgData->hps, LCID_DEFAULT);
            GpiQueryFontMetrics(pDlgData->hps,
                                sizeof(pDlgData->fmLast),
                                &pDlgData->fmLast);
        }

        pDlgData->pcszFontLast = pcszFontThis;      // can be NULL
    }
}

/*
 *@@ CalcAutoSizeTextSingle:
 *      implementation for CalcAutoSize for single-line
 *      static text controls.
 *
 *@@changed V0.9.12 (2001-05-31) [umoeller]: fixed various things with statics
 *@@changed V0.9.12 (2001-05-31) [umoeller]: fixed broken fonts
 *@@changed V0.9.14 (2001-08-01) [umoeller]: now caching fonts, which is significantly faster
 *@@changed V0.9.16 (2001-10-15) [umoeller]: added APIRET
 *@@changed V0.9.16 (2002-02-02) [umoeller]: added ulWidth
 *@@changed V1.0.0 (2002-08-18) [umoeller]: renamed; moved multi-line processing to CalcAutoSizeTextMulti
 */

STATIC APIRET CalcAutoSizeTextSingle(PCOLUMNDEF pColumn,
                                     PDLGPRIVATE pDlgData)
{
    SetDlgFont(pColumn->pcszFont, pDlgData);

    pColumn->szlAuto.cy =   pDlgData->fmLast.lMaxBaselineExt
                          + pDlgData->fmLast.lExternalLeading;

    // get the control string and see how much space it needs
    if (pColumn->pcszText)
    {
        POINTL aptl[TXTBOX_COUNT];
        if (!GpiQueryTextBox(pDlgData->hps,
                             strlen(pColumn->pcszText),
                             (PCH)pColumn->pcszText,
                             TXTBOX_COUNT,
                             aptl))
            return DLGERR_GPIQUERYTEXTBOX;

        pColumn->szlAuto.cx = aptl[TXTBOX_TOPRIGHT].x - aptl[TXTBOX_BOTTOMLEFT].x;

        return NO_ERROR;
    }

    return DLGERR_INVALID_CONTROL_TITLE;
}

/*
 *@@ CalcAutoSizeTextMulti:
 *      implementation for CalcAutoSize for multi-line
 *      static text controls.
 *
 *@@added V1.0.0 (2002-08-18) [umoeller]
 */

STATIC APIRET CalcAutoSizeTextMulti(PCOLUMNDEF pColumn,
                                    ULONG ulWidth,            // in: proposed width of control (req.)
                                    PDLGPRIVATE pDlgData)
{
    SetDlgFont(pColumn->pcszFont, pDlgData);

    // get the control string and see how much space it needs
    if (pColumn->pcszText)
    {
        RECTL rcl = {0, 0, 0, 0};
        rcl.xRight = ulWidth;
        if (pColumn->szlProposed.cy > 0)
            rcl.yTop = pColumn->szlProposed.cy;
        else
            rcl.yTop = G_cyScreen * 2 / 3;

        winhDrawFormattedText(pDlgData->hps,
                              &rcl,
                              pColumn->pcszText,
                              DT_LEFT | DT_TOP | DT_WORDBREAK | DT_QUERYEXTENT);
        pColumn->szlAuto.cx = rcl.xRight - rcl.xLeft;
        pColumn->szlAuto.cy = rcl.yTop - rcl.yBottom;

        return NO_ERROR;
    }

    return DLGERR_INVALID_CONTROL_TITLE;
}

/*
 *@@ CalcAutoSizeTextView:
 *      implementation for CalcAutoSize for the XTextView
 *      control.
 *
 *      This is slightly sick. We create the control already
 *      here in order to be able to have it format the text
 *      and then send TXM_QUERYTEXTEXTENT to it, which was
 *      added to the control with V0.9.20 for this very
 *      purpose. The control that we create here will then be
 *      reused by ColumnCreateControl and not be recreated,
 *      which would be way too expensive.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 *@@changed V1.0.0 (2002-08-18) [umoeller]: removed temp windows list
 */

STATIC APIRET CalcAutoSizeTextView(PCOLUMNDEF pColumn,
                                   ULONG ulWidth,            // in: proposed width of control (req.)
                                   PDLGPRIVATE pDlgData)
{
    APIRET arc = NO_ERROR;

    PCSZ pcszTitle;

    if (    (pColumn->hwndControl)
         || (pColumn->hwndControl = WinCreateWindow(pDlgData->hwndDlg,   // parent
                                                    (PSZ)pColumn->pcszClass,
                                                    NULL,
                                                    pColumn->flStyle,
                                                    0,
                                                    0,
                                                    ulWidth,
                                                    2000,         // cy, for now
                                                    pDlgData->hwndDlg,   // owner
                                                    HWND_BOTTOM,
                                                    pColumn->usID,
                                                    pColumn->pvCtlData,
                                                    NULL))
       )
    {
        HWND hwnd = pColumn->hwndControl;
        SIZEL szlTemp;

        if (pColumn->pcszFont)
            winhSetWindowFont(hwnd,
                              pColumn->pcszFont);

        WinSetWindowText(hwnd,
                         (pcszTitle = pColumn->pcszText)
                             ? (PSZ)pcszTitle
                             : "");

        WinSendMsg(hwnd,
                   TXM_QUERYTEXTEXTENT,
                   (MPARAM)&szlTemp,
                   0);

        pColumn->szlAuto.cy = szlTemp.cy;
    }
    else
        arc = DLGERR_CANNOT_CREATE_CONTROL;

    return arc;
}

/*
 *@@ CalcAutoSize:
 *      helper func that gets called from ColumnCalcSizes for
 *      every control that has set SZL_AUTOSIZE for its size.
 *
 *      We try to be smart and set a correct size for the
 *      control, depending on its class and data.
 *
 *      This sets pColumn->szlAuto and possibly pre-creates
 *      a window for the control and stores the window
 *      in pColumn->hwndControl.
 *
 *      Presently this works for
 *
 *      --  static text, single and multiline
 *
 *      --  static icons and bitmaps
 *
 *      --  pushbuttons, radio buttons, and checkboxes
 *
 *      --  the XTextView control (yes! V0.9.20).
 *
 *@@changed V0.9.12 (2001-05-31) [umoeller]: fixed various things with statics
 *@@changed V0.9.16 (2001-10-15) [umoeller]: added APIRET
 *@@changed V0.9.20 (2002-08-10) [umoeller]: added support for textview
 */

STATIC APIRET CalcAutoSize(PCOLUMNDEF pColumn,
                           ULONG ulWidth,            // in: proposed width of control
                           PDLGPRIVATE pDlgData)
{
    APIRET arc = NO_ERROR;

    switch ((ULONG)pColumn->pcszClass)
    {
        case 0xffff0003L: // WC_BUTTON:
            if (!(arc = CalcAutoSizeTextSingle(pColumn,
                                               pDlgData)))
            {
                if (pColumn->flStyle & (   BS_AUTOCHECKBOX
                                         | BS_AUTORADIOBUTTON
                                         | BS_AUTO3STATE
                                         | BS_3STATE
                                         | BS_CHECKBOX
                                         | BS_RADIOBUTTON))
                {
                    // give a little extra width for the box bitmap
                    // V0.9.19 (2002-04-24) [umoeller]
                    pColumn->szlAuto.cx += ctlQueryCheckboxSize() + 4;
                    // and height
                    pColumn->szlAuto.cy += 2;
                }
                else if (pColumn->flStyle & BS_BITMAP)
                    ;       // @@todo
                else if (pColumn->flStyle & (BS_ICON | BS_MINIICON))
                    ;       // @@todo
                // we can't test for BS_PUSHBUTTON because that's 0x0000
                else if (!(pColumn->flStyle & BS_USERBUTTON))
                {
                    pColumn->szlAuto.cx += (2 * pDlgData->cxBorder + 15);
                    pColumn->szlAuto.cy += (2 * pDlgData->cyBorder + 15);
                }
            }
        break;

        case 0xffff0005L: // WC_STATIC:
            switch (pColumn->flStyle & 0x0F)
            {
                case SS_TEXT:
                    if (pColumn->flStyle & DT_WORDBREAK)
                        // multi-line:
                        arc = CalcAutoSizeTextMulti(pColumn,
                                                    ulWidth,
                                                    pDlgData);
                    else
                        // single-line:
                        arc = CalcAutoSizeTextSingle(pColumn,
                                                     pDlgData);
                break;

                case SS_BITMAP:
                {
                    HBITMAP hbm;
                    if (hbm = (HBITMAP)pColumn->pcszText)
                    {
                        BITMAPINFOHEADER2 bmih2;
                        ZERO(&bmih2);
                        bmih2.cbFix = sizeof(bmih2);
                        if (GpiQueryBitmapInfoHeader(hbm,
                                                     &bmih2))
                        {
                            pColumn->szlAuto.cx = bmih2.cx;
                            pColumn->szlAuto.cy = bmih2.cy;
                        }
                        else
                            arc = DLGERR_GPIQUERYBITMAPINFOHEADER;
                    }
                    else
                        arc = DLGERR_INVALID_STATIC_BITMAP;
                }
                break;

                case SS_ICON:
                    pColumn->szlAuto.cx = G_cxIcon;
                    pColumn->szlAuto.cy = G_cyIcon;
                break;
            }
        break;

        default:
            // added support for textview V0.9.20 (2002-08-10) [umoeller]
            if (    (((ULONG)pColumn->pcszClass & 0xFFFF0000) != 0xFFFF0000)
                            // don't run strcmp on the PM pseudo-strings
                 && (!strcmp(pColumn->pcszClass, WC_XTEXTVIEW))
               )
            {
                arc = CalcAutoSizeTextView(pColumn,
                                           ulWidth,
                                           pDlgData);
            }
            else
            {
                // any other control (just to be safe):
                SetDlgFont(pColumn->pcszFont, pDlgData);
                pColumn->szlAuto.cx = 50;
                pColumn->szlAuto.cy =   pDlgData->fmLast.lMaxBaselineExt
                                      + pDlgData->fmLast.lExternalLeading
                                      + 7;         // some space
            }
    }

    return arc;
}

/*
 *@@ ColumnCalcPositions:
 *      implementation for PROCESS_6_CALC_POSITIONS in
 *      ProcessColumn.
 *
 *@@added V0.9.15 (2001-08-26) [umoeller]
 *@@changed V0.9.16 (2001-10-15) [umoeller]: added APIRET
 *@@changed V0.9.19 (2002-04-24) [umoeller]: fixed PM groups alignment
 *@@changed V1.0.0 (2002-08-16) [umoeller]: adjusted for new algorithm
 */

STATIC APIRET ColumnCalcPositions(PCOLUMNDEF pColumn,
                                  PROWDEF pOwningRow,          // in: current row from ProcessRow
                                  PLONG plX,                   // in/out: PROCESS_6_CALC_POSITIONS only
                                  PDLGPRIVATE pDlgData)
{
    APIRET arc = NO_ERROR;

    // column box = *plX on ProcessRow stack
    pColumn->cpBox.x = *plX;
    pColumn->cpBox.y = pOwningRow->cpRowBox.y;

    // check vertical alignment of row;
    // we might need to increase column y
    switch (pOwningRow->flRowFormat & ROW_VALIGN_MASK)
    {
        // case ROW_VALIGN_BOTTOM:      // do nothing

        case ROW_VALIGN_CENTER:
            if (pColumn->cpBox.cy < pOwningRow->cpRowBox.cy)
                pColumn->cpBox.y += (   (pOwningRow->cpRowBox.cy - pColumn->cpBox.cy)
                                      / 2);
        break;

        case ROW_VALIGN_TOP:
            if (pColumn->cpBox.cy < pOwningRow->cpRowBox.cy)
                pColumn->cpBox.y += (pOwningRow->cpRowBox.cy - pColumn->cpBox.cy);
        break;
    }

    // increase plX by box width
    *plX += pColumn->cpBox.cx;

    // calculate CONTROL pos from COLUMN pos by applying spacing
    pColumn->cpContent.x =   pColumn->cpBox.x
                           + pColumn->rclPadding.xLeft
                           + pColumn->rclMargin.xLeft;
    pColumn->cpContent.y =   pColumn->cpBox.y
                           + pColumn->rclPadding.yBottom
                           + pColumn->rclMargin.yBottom;

    if (pColumn->pNestedTable)
    {
        // nested table:
        // recurse!! to create windows for the sub-table
        arc = ProcessTable(pColumn->pNestedTable,
                           &pColumn->cpContent,   // start pos for new table
                           PROCESS_6_CALC_POSITIONS,
                           pDlgData);
    }

    return arc;
}

/*
 *@@ ColumnCreateControl:
 *      implementation for PROCESS_7_CREATE_CONTROLS in
 *      ProcessColumn.
 *
 *@@added V0.9.15 (2001-08-26) [umoeller]
 *@@changed V0.9.16 (2001-10-15) [umoeller]: fixed ugly group table spacings
 *@@changed V0.9.16 (2001-12-08) [umoeller]: fixed entry field ES_MARGIN positioning
 *@@changed V0.9.19 (2002-04-17) [umoeller]: fixes for the STUPID drop-down comboboxes
 *@@changed V0.9.19 (2002-04-24) [umoeller]: fixed PM groups alignment
 *@@changed V1.0.0 (2002-08-16) [umoeller]: adjusted for new algorithm
 *@@changed V1.0.0 (2002-08-18) [umoeller]: setting entry field length to CCHMAXPATH per default now
 */

STATIC APIRET ColumnCreateControl(PCOLUMNDEF pColumn,
                                  PDLGPRIVATE pDlgData)
{
    APIRET      arc = NO_ERROR;

    volatile PCSZ        pcszClass = NULL; // XWP V1.0.4 (2005-10-09) [pr]
    volatile PCSZ        pcszText = NULL;
    volatile ULONG       flStyle = 0;
    volatile LHANDLE     lHandleSet = NULLHANDLE;
    volatile ULONG       flOld = 0;

    if (pColumn->pNestedTable)
    {
        // nested table:
        // recurse!!
        if (!(arc = ProcessTable(pColumn->pNestedTable,
                                 NULL,
                                 PROCESS_7_CREATE_CONTROLS,
                                 pDlgData)))
        {
            // should we create a PM control around the table?
            // (do this AFTER the other controls from recursing,
            // otherwise the stupid container doesn't show up)
            if (pcszClass = pColumn->pcszClass)
            {
                // yes:
                pcszText = pColumn->pcszText;
                flStyle = pColumn->flStyle;

                // V1.0.0 (2002-08-16) [umoeller]
                // Removed all the terrible hacks for the PM group
                // control. We now use rclPadding and rclMargin
                // in the CONTROLDEF for determining the group
                // box position and size. Those were set initially
                // by the DLGHITEM parser. See CreateColumn().
                // If the group box had an explicit size set,
                // that is used by ProcessTable for the table
                // box also.
            }

#ifdef DEBUG_DIALOG_WINDOWS
            // debug: create a frame with the exact size
            // of the table in blue
            CreateDebugFrame(pDlgData->hwndDlg,        // parent
                             pTableDef->cpTableBox.x + pDlgData->ptlTotalOfs.x,
                             pTableDef->cpTableBox.y + pDlgData->ptlTotalOfs.y,
                             pTableDef->cpTableBox.cx,
                             pTableDef->cpTableBox.cy,
                             RGBCOL_BLUE,
                             LINETYPE_DOT);
#endif
        }
    }
    else
    {
        // no nested table, but control:
        pcszClass = pColumn->pcszClass;
        pcszText = pColumn->pcszText;
        flStyle = pColumn->flStyle;
    }

    if (    (!arc)            // check error code V1.0.0 (2002-08-16) [umoeller]
         && (pcszClass)
       )
    {
        // create something:
        LONG        x, cx, y, cy;               // for control hacks
        HWND        hwndFound = NULLHANDLE;
        LONG        cxUse;
        PCSZ        pcszTextEF = NULL;          // for entry field

        // V1.0.0 (2002-08-16) [umoeller]
        // determine the position where to create the
        // control; this is the content box, to which
        // we add the absolute position on the dlgdata
        // stack, and padding. Padding is presently
        // only set for the group box around tables.

        // To create the control, we use the explicit
        // or automatic size of the control instead
        // of the column width. Otherwise all buttons
        // are expanded to the column width, for example.
        if (    (!(cxUse = pColumn->cxMinContent))
             || (pColumn->pNestedTable)
           )
            cxUse = pColumn->cpContent.cx;

        x  =   pColumn->cpContent.x
             - pColumn->rclPadding.xLeft
             + pDlgData->ptlTotalOfs.x;
        cx =   cxUse
             + pColumn->rclPadding.xLeft
             + pColumn->rclPadding.xRight;
        y  =   pColumn->cpContent.y
             - pColumn->rclPadding.yBottom
             + pDlgData->ptlTotalOfs.y;
        cy =   pColumn->cpContent.cy
             + pColumn->rclPadding.yBottom
             + pColumn->rclPadding.yTop;

        // now implement hacks for certain controls
        switch ((ULONG)pcszClass)
        {
            case 0xffff0005L: // WC_STATIC:
                // change the title if this is a static with SS_BITMAP;
                // we have used a HBITMAP in there!
                if (    (    ((flStyle & 0x0F) == SS_BITMAP)
                          || ((flStyle & 0x0F) == SS_ICON)
                        )
                   )
                {
                    // change style flag to not use SS_BITMAP nor SS_ICON;
                    // control creation fails otherwise (stupid, stupid PM)
                    flOld = flStyle;
                    flStyle = ((flStyle & ~0x0F) | SS_FGNDFRAME);
                    pcszText = "";
                    lHandleSet = (LHANDLE)pColumn->pcszText;
                }
            break;

            case 0xffff0002L:   // combobox
                if (flStyle & (CBS_DROPDOWN | CBS_DROPDOWNLIST))
                {
                    // while calculating the column height,
                    // we have set pColumn->cpContent.cy
                    // to the height of a single line to get
                    // the position calculations right...
                    LONG  cyDelta = pColumn->szlProposed.cy - pColumn->cpContent.cy;
                    y  -= cyDelta;
                    cy += cyDelta;
                }
            break;

            case 0xffff0006L:   // entry field
            case 0xffff000AL:   // MLE:
                // the stupid entry field resizes itself if it has
                // the ES_MARGIN style, so correlate that too... dammit
                // V0.9.16 (2001-12-08) [umoeller]
                if (flStyle & ES_MARGIN)
                {
                    LONG cxMargin = 3 * pDlgData->cxBorder;
                    LONG cyMargin = 3 * pDlgData->cyBorder;

                    x += cxMargin;
                    y += cyMargin;
                    cx -= 2 * cxMargin;
                    cy -= 2 * cyMargin;
                }

                if ((ULONG)pcszClass == 0xffff0006L)
                {
                    // entry field:
                    // defer setting the text because we should
                    // first set the entry field limit
                    // V1.0.0 (2002-08-18) [umoeller]
                    pcszTextEF = pcszText;
                    pcszText = "";          // for now
                }
            break;
        } // end switch ((ULONG)pControlDef->pcszClass)

        if (pColumn->hwndControl)
        {
            // control was already created by CalcAutoSize:
            // resize it to what we really need
            WinSetWindowPos(pColumn->hwndControl,
                            HWND_BOTTOM,
                            x,
                            y,
                            cx,
                            cy,
                            SWP_SIZE | SWP_MOVE | SWP_ZORDER);
        }
        else
        {
            TRY_QUIET(excpt1)
            {
                pColumn->hwndControl = WinCreateWindow(pDlgData->hwndDlg,   // parent
                                                       (PSZ)pcszClass,      // window class, hacked
                                                       (pcszText)           // control text, hacked
                                                             ? (PSZ)pcszText
                                                             : "",
                                                       flStyle,             // window style, hacked
                                                       x,
                                                       y,
                                                       cx,
                                                       cy,
                                                       pDlgData->hwndDlg,   // owner == parent
                                                       HWND_BOTTOM,
                                                       pColumn->usID,
                                                       pColumn->pvCtlData,
                                                       NULL);
            }
            CATCH(excpt1)
            {
                CHAR szClass2[100] = "Invalid mem",
                     szText2[100] = "Invalid mem";
                ULONG Size, Attr;
                if (!(arc = DosQueryMem((PVOID)pcszClass, &Size, &Attr)))
                    strhncpy0(szClass2, pcszClass, sizeof(szClass2));
                if (!(arc = DosQueryMem((PVOID)pcszClass, &Size, &Attr)))
                    strhncpy0(szText2, pcszText, sizeof(szText2));

                #ifdef DEBUG_DIALOG_WINDOWS
                _Pmpf(("Crash creating control of class 0x%lX (%s), title 0x%lX (%s)",
                        szClass2,
                        szText2));
                #endif

                arc = ERROR_PROTECTION_VIOLATION;
            } END_CATCH();
        }

        if (pColumn->hwndControl)
        {
            // control created, or reused above:

            if (lHandleSet)
            {
                // subclass the damn static
                if ((flOld & 0x0F) == SS_ICON)
                    // this was a static:
                    ctlPrepareStaticIcon(pColumn->hwndControl,
                                         1);
                else
                    // this was a bitmap:
                    ctlPrepareStretchedBitmap(pColumn->hwndControl,
                                              TRUE);

                WinSendMsg(pColumn->hwndControl,
                           SM_SETHANDLE,
                           (MPARAM)lHandleSet,
                           0);
            }
            else
            {
                if (pcszTextEF)
                {
                    // entry field:
                    // set text limit to CCHMAXPATH per default,
                    // and set the real text now
                    // V1.0.0 (2002-08-18) [umoeller]
                    winhSetEntryFieldLimit(pColumn->hwndControl, CCHMAXPATH);
                    WinSetWindowText(pColumn->hwndControl, (PSZ)pcszTextEF);
                }

                if (pColumn->pcszFont)
                    // we must set the font explicitly here...
                    // doesn't always work with WinCreateWindow
                    // presparams parameter, for some reason
                    // V0.9.12 (2001-05-31) [umoeller]
                    winhSetWindowFont(pColumn->hwndControl,
                                      pColumn->pcszFont);
            }

#ifdef DEBUG_DIALOG_WINDOWS
            // if (!pColumn->fIsNestedTable)
            {
                // debug: create a frame with the exact size
                // of the box (not the content), so this
                // includes spacing
                CreateDebugFrame(pDlgData->hwndDlg,        // parent
                                 pColumn->cpBox.x + pDlgData->ptlTotalOfs.x,
                                 pColumn->cpBox.y + pDlgData->ptlTotalOfs.y,
                                 pColumn->cpBox.cx,
                                 pColumn->cpBox.cy,
                                 RGBCOL_DARKGREEN,
                                 LINETYPE_SOLID);

                // and another one for the content (control size)
                CreateDebugFrame(pDlgData->hwndDlg,        // parent
                                 pColumn->cpContent.x + pDlgData->ptlTotalOfs.x,
                                 pColumn->cpContent.y + pDlgData->ptlTotalOfs.y,
                                 pColumn->cpContent.cx,
                                 pColumn->cpContent.cy,
                                 RGBCOL_RED,
                                 LINETYPE_ALTERNATE);
            }
#endif

            // append window that was created
            // V0.9.18 (2002-03-03) [umoeller]
            if (pDlgData->pllControls)
                lstAppendItem(pDlgData->pllControls,
                              (PVOID)pColumn->hwndControl);

            // if this is the first control with WS_TABSTOP,
            // we'll give it the focus later
            if (    (flStyle & WS_TABSTOP)
                 && (!pDlgData->hwndFirstFocus)
               )
                pDlgData->hwndFirstFocus = pColumn->hwndControl;

            // if this is the first default push button,
            // go store it too
            // V0.9.14 (2001-08-21) [umoeller]
            if (    (!pDlgData->hwndDefPushbutton)
                 && ((ULONG)pColumn->pcszClass == 0xffff0003L)
                 && (pColumn->flStyle & BS_DEFAULT)
               )
                pDlgData->hwndDefPushbutton = pColumn->hwndControl;
        }
        else
            // V0.9.14 (2001-08-03) [umoeller]
            arc = DLGERR_CANNOT_CREATE_CONTROL;
    }

    return arc;
}

/* ******************************************************************
 *
 *   Recursive workers for columns, rows, tables
 *
 ********************************************************************/

/*
 *@@ ProcessColumn:
 *      processes a column, which per definition is either
 *      a control or a nested subtable.
 *
 *      A column is part of a row, which in turn is part
 *      of a table. There can be several columns in a row,
 *      and several rows in a table.
 *
 *      Since tables may be specified as columns, it is
 *      possible to produce complex dialog layouts by
 *      nesting tables.
 *
 *      Preconditions:
 *
 *      -- PROCESS_6_CALC_POSITIONS: position of each column
 *         is taken from *plX, which is increased by the
 *         column width by this call.
 *
 *         Owning row must already have its y position properly
 *         set, or we can't compute ours. Besides, plX must
 *         point to the current X in the row and will be
 *         incremented by the column's size here.
 *
 *@@changed V0.9.12 (2001-05-31) [umoeller]: added control data
 *@@changed V0.9.12 (2001-05-31) [umoeller]: fixed font problems
 *@@changed V0.9.20 (2002-08-08) [umoeller]: added support for aligning columns horizontally
 *@@changed V1.0.0 (2002-08-16) [umoeller]: calc size rewritten for new algorithm
 */

STATIC APIRET ProcessColumn(PCOLUMNDEF pColumn,
                            PROWDEF pOwningRow,          // in: current row from ProcessRow
                            enProcessMode ProcessMode,     // in: processing mode (see ProcessAll)
                            PLONG plX,                   // in/out: PROCESS_6_CALC_POSITIONS only
                            PDLGPRIVATE pDlgData)
{
    APIRET arc = NO_ERROR;

    // pColumn->pOwningRow = pOwningRow;

    switch (ProcessMode)
    {
        /*
         * PROCESS_1_CALC_MIN_WIDTHS:
         *
         */

        case PROCESS_1_CALC_MIN_WIDTHS:
            // rewritten V1.0.0 (2002-08-16) [umoeller]

            if (!pColumn->pNestedTable)
            {
                // control leaf node, not subtable:

                // check if the control title is valid... we
                // crash later otherwise
                if ((ULONG)pColumn->pcszText > 0xFFFF0000)
                    arc = DLGERR_INVALID_CONTROL_TITLE;
                else
                {
                    if (pColumn->szlProposed.cx > 0)
                    {
                        // explicit size:
                        pColumn->cxMinContent = pColumn->szlProposed.cx;
                    }
                    else if (pColumn->szlProposed.cx == SZL_AUTOSIZE)   // -1
                    {
                        // autosize width:
                        // calc then
                        arc = CalcAutoSize(pColumn,
                                           1000,            // width, whatever...
                                                            // that's what we want to know!
                                           pDlgData);

                        pColumn->cxMinContent = pColumn->szlAuto.cx;
                    }
                    // else: -100 < width < -1: percentage of parent table,
                    // leave cxMinContent == 0 for now

                    #ifdef DEBUG_DIALOG_WINDOWS
                        if (pControlDef->pcszText)
                        {
                            CHAR szTemp[40];
                            strhncpy0(szTemp, pControlDef->pcszText, sizeof(szTemp));
                            strcpy(szTemp + 35, "...");
                            _PmpfF(("PROCESS_1_CALC_MIN_WIDTHS [%s] cxMinContent %d",
                                    szTemp,
                                    pColumn->cxMinContent));
                        }
                    #endif
                }
            }
            else
            {
                // column represents subtable:
                // recurse!
                if (!(arc = ProcessTable(pColumn->pNestedTable,
                                         NULL,
                                         ProcessMode,
                                         pDlgData)))
                {
                    // ProcessTable has set pTableDef->cxMinBox,
                    // that is the size of the entire table box...
                    // use that for our cxMinContent
                    pColumn->cxMinContent = pColumn->pNestedTable->cxMinBox;
                }
            }

        break; // PROCESS_1_CALC_MIN_WIDTHS

        /*
         * PROCESS_2_CALC_RELATIVE_WIDTHS:
         *
         */

        case PROCESS_2_CALC_RELATIVE_WIDTHS:
            // rewritten V1.0.0 (2002-08-16) [umoeller]

            if (!pColumn->pNestedTable)
            {
                // control leaf node, not subtable:
                if (    (pColumn->szlProposed.cx == SZL_AUTOSIZE)   // -1
                     || (pColumn->szlProposed.cx > 0)               // not SZL_REMAINDER
                   )
                {
                    // explicit size or autosize:
                    // that was computed with PROCESS_1_CALC_MIN_WIDTHS,
                    // so copy from cxMinContent
                    pColumn->cpContent.cx = pColumn->cxMinContent;
                }
                else if (pColumn->szlProposed.cx < 0)
                {
                    // control wants percentage of parent table width:
                    PTABLEDEF   pOwningTable = pOwningRow->pOwningTable;
                    PROWDEF     pWidestRow;

                    #ifdef DEBUG_DIALOG_WINDOWS
                        {
                            CHAR szTemp[40];
                            strhncpy0(szTemp,
                                      (pControlDef->pcszText) ? pControlDef->pcszText : "NULL",
                                      sizeof(szTemp));
                            strcpy(szTemp + 35, "...");
                            _PmpfF(("PROCESS_2_CALC_RELATIVE_WIDTHS [%s]"));
                        }
                    #endif

                    if (pWidestRow = pOwningTable->pWidestRow)
                    {
                        PLISTNODE   pNode;
                        PCOLUMNDEF  pSubColumn,
                                    pTablesColumn;
                        LONG        cxWidestRowsMinBox = pOwningTable->cxMinBox; // pWidestRow->cxMinBox;

                        #ifdef DEBUG_DIALOG_WINDOWS
                        _Pmpf(("  pWidestRow->cxMinBox %d", pWidestRow->cxMinBox));
                        _Pmpf(("  pOwningTable->cxMinBox %d", pOwningTable->cxMinBox));
                        #endif

                        // now, since we're supposed to set our own CONTENT
                        // rectangle, subtract our own padding and margin
                        cxWidestRowsMinBox -=   pColumn->cxSpacingTotal;

                        pColumn->cpContent.cx =   (double)cxWidestRowsMinBox
                                                   * -pColumn->szlProposed.cx
                                                   / 100;
                    }

                    #ifdef DEBUG_DIALOG_WINDOWS
                        {
                            _Pmpf(("   cpContent.cx %d",
                                    pColumn->cpContent.cx));
                            _Pmpf(("   pWidestRow was 0x%lX", pWidestRow));
                        }
                    #endif
                } // end if (pControlDef->szlDlgUnits.cx < 0)
                else if (pColumn->szlProposed.cx == SZL_REMAINDER) // 0
                {
                    // SZL_REMAINDER is calculated in PROCESS_4_REMAINDER_WIDTHS,
                    // just count the remainders in the ROWDEF for now so
                    // we can process them quickly later
                    ++(pOwningRow->cWantRemainders);

                    // set the flag so that Dlg2_CalcSizes knows we'll
                    // need PROCESS_4_REMAINDER_WIDTHS
                    pDlgData->flNeedsProcessing |= FL_REMAINDER_WIDTHS;
                }
            }
            else
            {
                // column represents subtable:
                // recurse!
                if (!(arc = ProcessTable(pColumn->pNestedTable,
                                         NULL,
                                         ProcessMode,
                                         pDlgData)))
                {
                    // ProcessTable has set pTableDef->cpTableBox.cx,
                    // that is the size of the entire table box...
                    // use that for our cpContent.cx
                    pColumn->cpContent.cx = pColumn->pNestedTable->cpTableBox.cx;
                }
            }
        break; // PROCESS_2_CALC_RELATIVE_WIDTHS

        /*
         * PROCESS_3_ALIGN_COLUMNS:
         *
         */

        case PROCESS_3_ALIGN_COLUMNS:
            // rewritten V1.0.0 (2002-08-16) [umoeller]

            if (!pColumn->pNestedTable)
            {
                // control leaf node, not subtable:

                LONG    cxWidest = 0;
                PTABLEDEF pOwningTable;

                // if our owning table has the TABLE_ALIGN_COLUMNS
                // flag set, set this columns width to the width
                // of the widest column in all rows of this table,
                // but align this column ONLY if it does not want
                // SZL_REMAINDER

                if (    (pColumn->szlProposed.cx != SZL_REMAINDER)       // 0
                     && (pOwningTable = pOwningRow->pOwningTable)
                     && (pOwningTable->flTable & TABLE_ALIGN_COLUMNS)
                   )
                {
                    // find the widest corresponding column in the
                    // surrounding table; pColumn->ulColumnIndex
                    // has our index, so we need to run through
                    // the rows and compare the widths of all
                    // columns with that index
                    PLISTNODE pRowNode;

                    FOR_ALL_NODES(&pOwningTable->llRows, pRowNode)
                    {
                        PROWDEF     pRowThis = (PROWDEF)pRowNode->pItemData;
                        PCOLUMNDEF  pCorrespondingColumn;
                        if (    (pCorrespondingColumn = (PCOLUMNDEF)lstItemFromIndex(&pRowThis->llColumns,
                                                                                     pColumn->ulColumnIndex))
                             && (pCorrespondingColumn->cxMinContent > cxWidest)
                           )
                        {
                            #ifdef DEBUG_DIALOG_WINDOWS
                            {
                                const CONTROLDEF *pControlDef = (const CONTROLDEF *)pCorrespondingColumn->pvDefinition;
                                if (pControlDef->pcszText)
                                {
                                    CHAR szTemp[40];
                                    strhncpy0(szTemp, pControlDef->pcszText, sizeof(szTemp));
                                    strcpy(szTemp + 35, "...");

                                    #ifdef DEBUG_DIALOG_WINDOWS
                                    _PmpfF(("   widest column is [%s] cxMinContent %d",
                                            szTemp,
                                            pCorrespondingColumn->cxMinContent));
                                    #endif
                                }
                            }
                            #endif

                            cxWidest = pCorrespondingColumn->cxMinContent;
                        }
                    }

                    pColumn->cpContent.cx = cxWidest;

                    #ifdef DEBUG_DIALOG_WINDOWS
                    {
                        if (pControlDef->pcszText)
                        {
                            CHAR szTemp[40];
                            strhncpy0(szTemp, pControlDef->pcszText, sizeof(szTemp));
                            strcpy(szTemp + 35, "...");
                            _PmpfF(("PROCESS_3_ALIGN_COLUMNS [%s] cpContent.cx %d",
                                    szTemp,
                                    pColumn->cpContent.cx));
                        }
                    }
                    #endif
                }
            }
            else
            {
                // column represents subtable:
                // recurse!
                arc = ProcessTable(pColumn->pNestedTable,
                                   NULL,
                                   ProcessMode,
                                   pDlgData);

                // the width of a table can NOT change from this
                // processing, so no need to adjust
                // (individual rows might have changed though)
                // @@todo wrong, if the first column is made
                // wider, the second column might now move over
                // the table width, fix that
            }

            // in any case (control or subtable), count the
            // widths of all columns that do not have SZL_REMAINDER
            // set for PROCESS_4_REMAINDER_WIDTHS in the next loop

            // NOTE: we add _all_ columns, including those that have
            // SZL_REMAINDER set (for which cpContent.cx is null
            // presently), because the "non remainder" space needs
            // to include the spacing of _all_ columns (see the
            // PROCESS_4_REMAINDER_WIDTHS processing below)
            pOwningRow->cxNonRemainder +=   pColumn->cpContent.cx
                                          + pColumn->cxSpacingTotal;

        break; // PROCESS_3_ALIGN_COLUMNS

        /*
         * PROCESS_4_REMAINDER_WIDTHS:
         *
         */

        case PROCESS_4_REMAINDER_WIDTHS:
            if (!pColumn->pNestedTable)
            {
                // control leaf node, not subtable:
                PTABLEDEF pOwningTable;

                if (    (pColumn->szlProposed.cx == SZL_REMAINDER)       // 0
                     && (pOwningTable = pOwningRow->pOwningTable)
                   )
                {
                    LONG    cxRemaining = pOwningTable->cpTableBox.cx - pOwningRow->cxNonRemainder;
                    // NOTE: pOwningRow->cxNonRemainder has
                    // --   the sum of the column content rectangle widths that did not
                    //      have SZL_REMAINDER set
                    // --   the sum of _all_ column spacings, even if the column had
                    //      SZL_REMAINDER set,
                    // so this really has the leftover space now.
                    // Divide that by the no. of columns in the row that have
                    // SZL_REMAINDER set so that we evenly distribute the remaining
                    // space among those columns.
                    pColumn->cpContent.cx = (   cxRemaining
                                              / pOwningRow->cWantRemainders
                                                    // this cannot be 0 since we have
                                                    // that flag and were thus counted
                                            );

                    #ifdef DEBUG_DIALOG_WINDOWS
                    {
                        CHAR szTemp[40] = "NULL";

                        if (pControlDef->pcszText)
                        {
                            strhncpy0(szTemp, pControlDef->pcszText, sizeof(szTemp));
                            strcpy(szTemp + 35, "...");
                        }

                        _PmpfF(("PROCESS_4_REMAINDER_WIDTHS [%s] pOwningTable->cpTableBox.cx %d",
                                szTemp,
                                pOwningTable->cpTableBox.cx));
                        _PmpfF(("  pOwningRow->cxNonRemainder %d",
                                pOwningRow->cxNonRemainder));
                        _PmpfF(("  pOwningRow->cWantRemainders %d",
                                pOwningRow->cWantRemainders));
                        _PmpfF(("  cpContent.cx %d",
                                pColumn->cpContent.cx));
                    }
                    #endif

                }
            }
            else
            {
                // column represents subtable:
                // recurse!
                arc = ProcessTable(pColumn->pNestedTable,
                                   NULL,
                                   ProcessMode,
                                   pDlgData);
            }
        break;

        /*
         * PROCESS_5_CALC_HEIGHTS:
         *
         */

        case PROCESS_5_CALC_HEIGHTS:
            if (!pColumn->pNestedTable)
            {
                // control leaf node, not subtable:
                if (pColumn->szlProposed.cy > 0)
                {
                    // explicit size:
                    pColumn->cpContent.cy = pColumn->szlProposed.cy;

                    if ((ULONG)pColumn->pcszClass == 0xffff0002L)
                    {
                        // hack the stupid drop-down combobox where the
                        // size of the drop-down is the full size of the
                        // control: when creating the control, we _do_
                        // specify the full size, but for the column,
                        // we must rather use a single line with
                        // the current font
                        // V0.9.19 (2002-04-17) [umoeller]
                        if (pColumn->flStyle & (CBS_DROPDOWN | CBS_DROPDOWNLIST))
                        {
                            SetDlgFont(pColumn->pcszFont, pDlgData);

                            pColumn->cpContent.cy
                                =   pDlgData->fmLast.lMaxBaselineExt
                                  + pDlgData->fmLast.lExternalLeading
                                  + 2 * 3 * pDlgData->cyBorder;
                        }
                    }
                }
                else if (pColumn->szlProposed.cy == SZL_AUTOSIZE)   // -1
                {
                    // autosize height:
                    // we MIGHT have run autosize already above
                    // (e.g. for the typical bitmap or icon statics),
                    // so check if szlAuto.cy is already set
                    if (!(pColumn->cpContent.cy = pColumn->szlAuto.cy))
                    {
                        // not yet determined:
                        // calc then
                        arc = CalcAutoSize(pColumn,
                                           // width: now we know
                                           pColumn->cpContent.cx,
                                           pDlgData);
                        pColumn->cpContent.cy = pColumn->szlAuto.cy;
                    }
                }
                // else: -100 < width < -1: @@todo
                // add another stage for using the percentage of
                // the table height
            }
            else
            {
                // column represents subtable:
                // recurse!
                if (!(arc = ProcessTable(pColumn->pNestedTable,
                                         NULL,
                                         ProcessMode,
                                         pDlgData)))
                {
                    // ProcessTable has set pTableDef->cpTableBox.cy,
                    // that is the size of the entire table box...
                    // use that for our cpContent.cy
                    pColumn->cpContent.cy = pColumn->pNestedTable->cpTableBox.cy;
                }
            }

            // we finally know the DEFINITE width and height
            // of the content, so we can set the box width
            // and height here now (ProcessRow relies on this
            // to determine the row height)
            pColumn->cpBox.cx =   pColumn->cpContent.cx
                                + pColumn->cxSpacingTotal;
            pColumn->cpBox.cy =   pColumn->cpContent.cy
                                + pColumn->rclPadding.yBottom
                                + pColumn->rclMargin.yBottom
                                + pColumn->rclPadding.yTop
                                + pColumn->rclMargin.yTop;
        break;

        /*
         * PROCESS_6_CALC_POSITIONS:
         *
         */

        case PROCESS_6_CALC_POSITIONS:
            arc = ColumnCalcPositions(pColumn,
                                      pOwningRow,
                                      plX,
                                      pDlgData);
        break;

        /*
         * PROCESS_7_CREATE_CONTROLS:
         *
         */

        case PROCESS_7_CREATE_CONTROLS:
            arc = ColumnCreateControl(pColumn,
                                      pDlgData);
        break;
    }

    return arc;
}

/*
 *@@ ProcessRow:
 *      level-3 procedure (called from ProcessTable),
 *      which in turn calls ProcessColumn for each column
 *      in the row.
 *
 *@@changed V1.0.0 (2002-08-16) [umoeller]: adjusted for new algorithm
 */

STATIC APIRET ProcessRow(PROWDEF pRowDef,
                         PTABLEDEF pOwningTable,     // in: current table from ProcessTable
                         enProcessMode ProcessMode,    // in: processing mode (see ProcessAll)
                         PLONG plY,                  // in/out: current y position (decremented)
                         PDLGPRIVATE pDlgData)
{
    APIRET  arc = NO_ERROR;
    LONG    lXRowBox;
    PLISTNODE pNode;

    pRowDef->pOwningTable = pOwningTable;

    switch (ProcessMode)
    {
        case PROCESS_3_ALIGN_COLUMNS:
            // we've computed pRowDef->cpRowBox.cx in the
            // previous run (PROCESS_2_CALC_RELATIVE_WIDTHS),
            // but since we keep adding to it, we must
            // reset it now... the row will probably become
            // larger!
            pRowDef->cpRowBox.cx = 0;
        break;

        case PROCESS_6_CALC_POSITIONS:
            // set up x and y so that the columns can
            // base on that
            pRowDef->cpRowBox.x = pOwningTable->cpTableBox.x;
            // decrease y by row height
            *plY -= pRowDef->cpRowBox.cy;
            // and use that for our bottom position
            pRowDef->cpRowBox.y = *plY;

            // set lX to left of row; used by column calls below
            lXRowBox = pRowDef->cpRowBox.x;
        break;
    }

    FOR_ALL_NODES(&pRowDef->llColumns, pNode)
    {
        PCOLUMNDEF  pColumnThis = (PCOLUMNDEF)pNode->pItemData;

        if (!(arc = ProcessColumn(pColumnThis,
                                  pRowDef,
                                  ProcessMode,
                                  &lXRowBox,
                                  pDlgData)))
        {
            switch (ProcessMode)
            {
                case PROCESS_1_CALC_MIN_WIDTHS:
                    // row width = sum of all columns
                    pRowDef->cxMinBox +=   pColumnThis->cxMinContent
                                         + pColumnThis->cxSpacingTotal;
                break;

                case PROCESS_2_CALC_RELATIVE_WIDTHS:
                case PROCESS_3_ALIGN_COLUMNS:       // reapply, was reset above
                    // row width = sum of all columns
                    pRowDef->cpRowBox.cx +=   pColumnThis->cpContent.cx
                                            + pColumnThis->cxSpacingTotal;
                break;

                case PROCESS_5_CALC_HEIGHTS:
                    // row height = maximum height of a column
                    if (pRowDef->cpRowBox.cy < pColumnThis->cpBox.cy)
                        pRowDef->cpRowBox.cy = pColumnThis->cpBox.cy;
            }
        }
        // we should stop on errors V0.9.20 (2002-08-10) [umoeller]
        else
            break;
    }

    return arc;
}

/*
 *@@ ProcessTable:
 *      level-2 procedure (called from ProcessAll),
 *      which in turn calls ProcessRow for each row
 *      in the table (which in turn calls ProcessColumn
 *      for each column in the row).
 *
 *      See ProcessAll for the meaning of ProcessMode.
 *
 *      This routine is a bit sick because it can even be
 *      called recursively from ProcessColumn (!) if a
 *      nested table is found in a COLUMNDEF.
 *
 *      With PROCESS_6_CALC_POSITIONS, pptl must specify
 *      the lower left corner of the table. For the
 *      root call, this will be {0, 0}; for nested calls,
 *      this must be the lower left corner of the column
 *      to which the nested table belongs.
 *
 *@@changed V1.0.0 (2002-08-16) [umoeller]: adjusted for new algorithm
 */

STATIC APIRET ProcessTable(PTABLEDEF pTableDef,
                           const CONTROLPOS *pcpTableBox,   // in: table position with PROCESS_6_CALC_POSITIONS
                           enProcessMode ProcessMode,         // in: processing mode (see ProcessAll)
                           PDLGPRIVATE pDlgData)
{
    APIRET  arc = NO_ERROR;
    LONG    lY;
    PLISTNODE pNode;

    switch (ProcessMode)
    {
        case PROCESS_2_CALC_RELATIVE_WIDTHS:
            // before calculating relative widths for
            // the table, inherit size from parent table
            // if the TABLE_INHERIT_SIZE table flag is
            // set; this was originally designed
            // as a separate step (PROCESS_2_INHERIT_TABLE_SIZES),
            // but we can save ourselves a loop here
            // V1.0.0 (2002-08-16) [umoeller]
            if (pTableDef->flTable & TABLE_INHERIT_SIZE)
            {
                PCOLUMNDEF  pOwningColumn;
                PROWDEF     pOwningRow;
                PTABLEDEF   pOwningTable;
                if (    (!(pOwningColumn = pTableDef->pOwningColumn))
                     || (!(pOwningRow = pOwningColumn->pOwningRow))
                     || (!(pOwningTable = pOwningRow->pOwningTable))
                   )
                    arc = DLGERR_ROOT_TABLE_INHERIT_SIZE;
                else
                {
                    PLISTNODE pRowNode;
                    LONG cxMax = 0;
                    FOR_ALL_NODES(&pOwningTable->llRows, pRowNode)
                    {
                        PROWDEF pRowThis = (PROWDEF)pRowNode->pItemData;
                        if (pRowThis->cxMinBox > cxMax)
                            cxMax = pRowThis->cxMinBox;
                    }

                    #ifdef DEBUG_DIALOG_WINDOWS
                    _Pmpf(("    cxMax found is %d", cxMax));
                    #endif

                    pTableDef->cxMinBox =   cxMax
                                          - pOwningColumn->cxSpacingTotal;
                }
            }
        break;

        case PROCESS_6_CALC_POSITIONS:
            pTableDef->cpTableBox.x = pcpTableBox->x;
            pTableDef->cpTableBox.y = pcpTableBox->y;

            // start the rows on top
            lY = pcpTableBox->y + pTableDef->cpTableBox.cy;
        break;
    }

    if (!arc)
    {
        FOR_ALL_NODES(&pTableDef->llRows, pNode)
        {
            PROWDEF pRowDefThis = (PROWDEF)pNode->pItemData;

            if (!(arc = ProcessRow(pRowDefThis, pTableDef, ProcessMode, &lY, pDlgData)))
            {
                switch (ProcessMode)
                {
                    case PROCESS_1_CALC_MIN_WIDTHS:
                        // table width = maximum width of a row
                        if (pTableDef->cxMinBox < pRowDefThis->cxMinBox)
                        {
                            pTableDef->cxMinBox = pRowDefThis->cxMinBox;

                            // remember the widest row so that
                            // PROCESS_2_CALC_RELATIVE_WIDTHS can calc relative
                            // widths from it

                            pTableDef->pWidestRow = pRowDefThis;
                        }
                    break;

                    case PROCESS_2_CALC_RELATIVE_WIDTHS:
                    case PROCESS_3_ALIGN_COLUMNS:
                    {
                        PCOLUMNDEF pOwningColumn;

                        // table width = maximum width of a row
                        if (pTableDef->cpTableBox.cx < pRowDefThis->cpRowBox.cx)
                            pTableDef->cpTableBox.cx = pRowDefThis->cpRowBox.cx;

                        // preserve the inherited width if it was larger
                        if (pTableDef->cxMinBox > pTableDef->cpTableBox.cx)
                            pTableDef->cpTableBox.cx = pTableDef->cxMinBox;

                        // if the table has a group box with an explicit
                        // size, use it if it's larger
                        if (    (pOwningColumn = pTableDef->pOwningColumn)
                             && (pOwningColumn->pcszClass)
                             && (pOwningColumn->szlProposed.cx > 0)
                             && (pOwningColumn->szlProposed.cx > pTableDef->cpTableBox.cx)
                           )
                            pTableDef->cpTableBox.cx = pOwningColumn->szlProposed.cx;
                    }
                    break;

                    case PROCESS_5_CALC_HEIGHTS:
                        // table height = sum of all rows
                        pTableDef->cpTableBox.cy += pRowDefThis->cpRowBox.cy;
                    break;
                }
            }
            else
                break;
        }
    }

    return arc;
}

/*
 *@@ ProcessAll:
 *      level-1 procedure, which in turn calls ProcessTable
 *      for each root-level table found (which in turn
 *      calls ProcessRow for each row in the table, which
 *      in turn calls ProcessColumn for each column in
 *      the row).
 */

STATIC APIRET ProcessAll(PDLGPRIVATE pDlgData,
                         enProcessMode ProcessMode)
{
    APIRET arc = NO_ERROR;
    PLISTNODE pNode;
    CONTROLPOS cpTableBox;
    ZERO(&cpTableBox);

    switch (ProcessMode)
    {
        case PROCESS_1_CALC_MIN_WIDTHS:
            pDlgData->szlClient.cx = 0;
            pDlgData->szlClient.cy = 0;
        break;

        case PROCESS_6_CALC_POSITIONS:
            // start with the table on top
            cpTableBox.y = pDlgData->szlClient.cy;
        break;
    }

    FOR_ALL_NODES(&pDlgData->llTables, pNode)
    {
        PTABLEDEF pTableDefThis = (PTABLEDEF)pNode->pItemData;

        if (ProcessMode == PROCESS_6_CALC_POSITIONS)
        {
            cpTableBox.x = 0;
            cpTableBox.y -= pTableDefThis->cpTableBox.cy;
        }

        if (!(arc = ProcessTable(pTableDefThis,
                                 &cpTableBox,      // start pos
                                 ProcessMode,
                                 pDlgData)))
        {
            switch (ProcessMode)
            {
                case PROCESS_5_CALC_HEIGHTS:
                {
                    // all sizes have now been computed:
                    pDlgData->szlClient.cx += pTableDefThis->cpTableBox.cx;
                    pDlgData->szlClient.cy += pTableDefThis->cpTableBox.cy;
                }
            }
        }
        // we should stop on errors V0.9.20 (2002-08-10) [umoeller]
        else
            break;
    }

    return arc;
}

/* ******************************************************************
 *
 *   DLGHITEM parser
 *
 ********************************************************************/

/*
 *@@ CreateColumn:
 *
 *@@changed V1.0.0 (2002-08-18) [umoeller]: mostly rewritten for new algorithm
 *@@changed V1.0.1 (2002-12-11) [umoeller]: added interface into nlsGetString
 */

STATIC APIRET CreateColumn(PDLGPRIVATE pDlgData,
                           PROWDEF pCurrentRow,
                           PTABLEDEF pNestedTable,
                           const CONTROLDEF *pControlDef,
                           PCOLUMNDEF *ppColumn)    // out: new COLUMNDEF
{
    APIRET arc = NO_ERROR;

    if (!pCurrentRow)
        arc = DLGERR_CONTROL_BEFORE_ROW;
    else
    {
        // create column and store ctl def
        PCOLUMNDEF pColumn;
        if (!(pColumn = NEW(COLUMNDEF)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            ZERO(pColumn);

            pColumn->pOwningRow = pCurrentRow;

            if (pColumn->pNestedTable = pNestedTable)
            {
                // should we create a PM control around the table?
                if (pControlDef)
                {
                    // yes:
                    pColumn->rclPadding.xLeft
                    = pColumn->rclPadding.xRight
                    = GROUP_INNER_SPACING_X * FACTOR_X;

                    pColumn->rclPadding.yBottom = GROUP_INNER_SPACING_BOTTOM * FACTOR_Y;

                    pColumn->rclPadding.yTop = GROUP_INNER_SPACING_TOP * FACTOR_Y;

                    pColumn->rclMargin.xLeft = GROUP_OUTER_SPACING_X * FACTOR_X;
                    pColumn->rclMargin.xRight = GROUP_OUTER_SPACING_X * FACTOR_X;
                    pColumn->rclMargin.yBottom = GROUP_OUTER_SPACING_BOTTOM * FACTOR_Y;
                    pColumn->rclMargin.yTop = GROUP_OUTER_SPACING_TOP * FACTOR_Y;
                }
            }
            else
                if (!pControlDef)
                    arc = DLGERR_NULL_CONTROLDEF;

            if (pControlDef)
            {
                // copy control fields
                pColumn->pcszClass = pControlDef->pcszClass;

                if (pControlDef->pcszText == LOAD_STRING)
                    pColumn->pcszText = nlsGetString(pControlDef->usID);
                            // V1.0.1 (2002-12-11) [umoeller]
                else
                    pColumn->pcszText = pControlDef->pcszText;

                pColumn->flStyle = pControlDef->flStyle;
                pColumn->usID = pControlDef->usID;

                // resolve font
                if (pControlDef->pcszFont == CTL_COMMON_FONT)
                    pColumn->pcszFont = pDlgData->pcszControlsFont;
                else
                    pColumn->pcszFont = pControlDef->pcszFont;
                                    // can be NULL

                // copy and convert proposed size
                if (pControlDef->szlDlgUnits.cx > 0)
                    // not SZL_AUTOSIZE, not SZL_REMAINDER, not percentage:
                    // convert from dlgunits to pixels
                    pColumn->szlProposed.cx = pControlDef->szlDlgUnits.cx * FACTOR_X;
                else
                    pColumn->szlProposed.cx = pControlDef->szlDlgUnits.cx;

                if (pControlDef->szlDlgUnits.cy > 0)
                    // not SZL_AUTOSIZE, not SZL_REMAINDER, not percentage:
                    // convert from dlgunits to pixels
                    pColumn->szlProposed.cy = pControlDef->szlDlgUnits.cy * FACTOR_Y;
                else
                    pColumn->szlProposed.cy = pControlDef->szlDlgUnits.cy;

                pColumn->pvCtlData = pControlDef->pvCtlData;

                // note: we increase the margin here... it was null
                // unless set above because we had a PM group control
                // around a table
                pColumn->rclMargin.xLeft += pControlDef->duSpacing * FACTOR_X;
                pColumn->rclMargin.xRight += pControlDef->duSpacing * FACTOR_X;

                pColumn->rclMargin.yBottom += pControlDef->duSpacing * FACTOR_Y;
                pColumn->rclMargin.yTop += pControlDef->duSpacing * FACTOR_Y;
            }

            // calculate the column's total spacing width
            // for speed
            pColumn->cxSpacingTotal =   pColumn->rclPadding.xLeft
                                      + pColumn->rclMargin.xLeft
                                      + pColumn->rclPadding.xRight
                                      + pColumn->rclMargin.xRight;

            // set the column index V1.0.0 (2002-08-18) [umoeller]
            pColumn->ulColumnIndex = (pCurrentRow->cColumns)++;

            *ppColumn = pColumn;
        }
    }

    return arc;
}

/*
 *@@ FreeTable:
 *      frees the specified table and recurses
 *      into nested tables, if necessary.
 *
 *      This was added with V0.9.14 to fix the
 *      bad memory leaks with nested tables.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

STATIC VOID FreeTable(PTABLEDEF pTable)
{
    // for each table, clean up the rows
    PLISTNODE pRowNode;
    FOR_ALL_NODES(&pTable->llRows, pRowNode)
    {
        PROWDEF pRow = (PROWDEF)pRowNode->pItemData;

        // for each row, clean up the columns
        PLISTNODE pColumnNode;
        FOR_ALL_NODES(&pRow->llColumns, pColumnNode)
        {
            PCOLUMNDEF pColumn = (PCOLUMNDEF)pColumnNode->pItemData;

            if (pColumn->pNestedTable)
                // nested table: recurse!
                FreeTable(pColumn->pNestedTable);

            free(pColumn);
        }
        lstClear(&pRow->llColumns);

        free(pRow);
    }
    lstClear(&pTable->llRows);

    free(pTable);
}

/*
 *@@ STACKITEM:
 *
 */

typedef struct _STACKITEM
{
    PTABLEDEF       pLastTable;
    PROWDEF         pLastRow;

} STACKITEM, *PSTACKITEM;

/*
 *@@ Dlg0_Init:
 *
 *@@added V0.9.15 (2001-08-26) [umoeller]
 *@@changed V0.9.18 (2002-03-03) [umoeller]: added pllWindows
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added resolution correlation
 */

STATIC APIRET Dlg0_Init(PDLGPRIVATE *ppDlgData,
                        PCSZ pcszControlsFont,
                        PLINKLIST pllControls)
{
    PDLGPRIVATE     pDlgData;
    POINTL          ptl = {100, 100};

    if (!(pDlgData = NEW(DLGPRIVATE)))
        return ERROR_NOT_ENOUGH_MEMORY;

    ZERO(pDlgData);
    lstInit(&pDlgData->llTables, FALSE);

    if (pllControls)
        pDlgData->pllControls = pllControls;

    pDlgData->pcszControlsFont = pcszControlsFont;

    // cache these now too V0.9.19 (2002-04-17) [umoeller]
    pDlgData->cxBorder = WinQuerySysValue(HWND_DESKTOP, SV_CXBORDER);
    pDlgData->cyBorder = WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);

    // check how many pixels we get out of the
    // dlgunits (100/100) for mapping all sizes
    // V0.9.19 (2002-04-24) [umoeller]
    if (WinMapDlgPoints(NULLHANDLE,
                        &ptl,
                        1,
                        TRUE))
    {
        // this worked:
        // for 1024x768, I get 200/250 out of the above,
        // so calculate a factor from that; we multiply
        // szlDlgUnits with this factor when calculating
        // the sizes
        pDlgData->dFactorX = (double)ptl.x / (double)100;       // 2   on 1024x768
        pDlgData->dFactorY = (double)ptl.y / (double)100;       // 2.5 on 1024x768
    }
    else
    {
        // didn't work:
        pDlgData->dFactorX = 2;
        pDlgData->dFactorY = 2.5;
    }

    *ppDlgData = pDlgData;

    return NO_ERROR;
}

/*
 *@@ Dlg1_ParseTables:
 *
 *@@added V0.9.15 (2001-08-26) [umoeller]
 */

STATIC APIRET Dlg1_ParseTables(PDLGPRIVATE pDlgData,
                               PCDLGHITEM paDlgItems,      // in: definition array
                               ULONG cDlgItems)           // in: array item count (NOT array size)
{
    APIRET      arc = NO_ERROR;

    LINKLIST    llStack;
    ULONG       ul;
    PTABLEDEF   pCurrentTable = NULL;
    PROWDEF     pCurrentRow = NULL;

    lstInit(&llStack, TRUE);      // this is our stack for nested table definitions

    for (ul = 0;
         ul < cDlgItems;
         ul++)
    {
        PCDLGHITEM   pItemThis = &paDlgItems[ul];

        switch (pItemThis->Type)
        {
            /*
             * TYPE_START_NEW_TABLE:
             *
             */

            case TYPE_START_NEW_TABLE:
            {
                // root table or nested?
                BOOL fIsRoot = (pCurrentTable == NULL);

                // push the current table on the stack
                PSTACKITEM pStackItem;
                if (!(pStackItem = NEW(STACKITEM)))
                {
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                else
                {
                    pStackItem->pLastTable = pCurrentTable;
                    pStackItem->pLastRow = pCurrentRow;
                    lstPush(&llStack, pStackItem);
                }

                // create new table
                if (!(pCurrentTable = NEW(TABLEDEF)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else
                {
                    const CONTROLDEF *pControlDef;
                    ZERO(pCurrentTable);

                    lstInit(&pCurrentTable->llRows, FALSE);

                    // if control specified: store it (this will become a PM group)
                    // pCurrentTable->pCtlDef = (const CONTROLDEF *)pItemThis->ul1; // pItemThis->pCtlDef;
                    pControlDef = (const CONTROLDEF *)pItemThis->ul1;
                                // can be NULL for plain table

                    pCurrentTable->flTable = pItemThis->ul2;         // V0.9.20 (2002-08-08) [umoeller]

                    if (fIsRoot)
                        // root table:
                        // append to dialog data list
                        lstAppendItem(&pDlgData->llTables, pCurrentTable);
                    else
                    {
                        // nested table:
                        // create "table" column for this in the current row
                        PCOLUMNDEF pColumn;
                        if (!(arc = CreateColumn(pDlgData,
                                                 pCurrentRow,
                                                 pCurrentTable,        // nested table
                                                 pControlDef,
                                                 &pColumn)))
                        {
                            pCurrentTable->pOwningColumn = pColumn;
                            lstAppendItem(&pCurrentRow->llColumns,
                                          pColumn);
                        }
                    }
                }

                // reset current row so we can detect
                // wrong DLGHITEM ordering
                pCurrentRow = NULL;
            }
            break;

            /*
             * TYPE_START_NEW_ROW:
             *
             */

            case TYPE_START_NEW_ROW:
                if (!pCurrentTable)
                    arc = DLGERR_ROW_BEFORE_TABLE;
                else
                {
                    // create new row
                    if (!(pCurrentRow = NEW(ROWDEF)))
                        arc = ERROR_NOT_ENOUGH_MEMORY;
                    else
                    {
                        ZERO(pCurrentRow);

                        pCurrentRow->pOwningTable = pCurrentTable;
                        lstInit(&pCurrentRow->llColumns, FALSE);

                        pCurrentRow->flRowFormat = pItemThis->ul2;

                        lstAppendItem(&pCurrentTable->llRows, pCurrentRow);
                    }
                }
            break;

            /*
             * TYPE_CONTROL_DEF:
             *
             */

            case TYPE_CONTROL_DEF:
            {
                PCOLUMNDEF pColumn;
                if (!(arc = CreateColumn(pDlgData,
                                         pCurrentRow,
                                         FALSE,        // no nested table
                                         (const CONTROLDEF *)pItemThis->ul1, // pCtlDef, V1.0.0 (2002-08-18) [umoeller]
                                         &pColumn)))
                    lstAppendItem(&pCurrentRow->llColumns,
                                  pColumn);
            }
            break;

            /*
             * TYPE_END_TABLE:
             *
             */

            case TYPE_END_TABLE:
            {
                PLISTNODE pNode;
                if (!(pNode = lstPop(&llStack)))
                    // nothing on the stack:
                    arc = DLGERR_TOO_MANY_TABLES_CLOSED;
                else
                {
                    PSTACKITEM pStackItem = (PSTACKITEM)pNode->pItemData;
                    pCurrentTable = pStackItem->pLastTable;
                    pCurrentRow = pStackItem->pLastRow;

                    lstRemoveNode(&llStack, pNode);
                }
            }
            break;

            default:
                arc = DLGERR_INVALID_CODE;
        }

        if (arc)
            break;
    }

    // all tables should be closed now
    if (    (!arc)
         && (lstCountItems(&llStack))
       )
        arc = DLGERR_TABLE_NOT_CLOSED;

    lstClear(&llStack);

    return arc;
}

/*
 *@@ Dlg2_CalcSizes:
 *      calls ProcessAll with the first four process modes
 *      for calculating the final sizes (but not yet the
 *      positions) of the controls.
 *
 *      See @dlg_algorithm for details.
 *
 *      After this, DLGPRIVATE.szlClient is valid.
 *
 *@@added V0.9.15 (2001-08-26) [umoeller]
 *@@changed V1.0.0 (2002-08-16) [umoeller]: adjusted for new algorithm
 */

STATIC APIRET Dlg2_CalcSizes(PDLGPRIVATE pDlgData)
{
    APIRET arc;

    // call process mode with the first for "calc size"
    // process modes
    // changed V1.0.0 (2002-08-16) [umoeller]
    if (!(arc = ProcessAll(pDlgData,
                           PROCESS_1_CALC_MIN_WIDTHS)))
        if (!(arc = ProcessAll(pDlgData,
                               PROCESS_2_CALC_RELATIVE_WIDTHS)))
            if (!(arc = ProcessAll(pDlgData,
                                   PROCESS_3_ALIGN_COLUMNS)))
                // skip PROCESS_4_REMAINDER_WIDTHS if the dlg doesn't need it
                if (    (!(pDlgData->flNeedsProcessing & FL_REMAINDER_WIDTHS))
                     || (!(arc = ProcessAll(pDlgData,
                                            PROCESS_4_REMAINDER_WIDTHS)))
                   )
                    arc = ProcessAll(pDlgData,
                                     PROCESS_5_CALC_HEIGHTS);

    // free the cached font resources that
    // might have been created here
    if (pDlgData->hps)
    {
        if (pDlgData->lcidLast)
        {
            GpiSetCharSet(pDlgData->hps, LCID_DEFAULT);
            GpiDeleteSetId(pDlgData->hps, pDlgData->lcidLast);
        }

        WinReleasePS(pDlgData->hps);
    }

    return arc;
}

/*
 *@@ Dlg3_PositionAndCreate:
 *
 *@@added V0.9.15 (2001-08-26) [umoeller]
 *@@changed V0.9.15 (2001-08-26) [umoeller]: BS_DEFAULT for other than first button was ignored, fixed
 *@@changed V0.9.20 (2002-08-10) [umoeller]: return code checking was missing, fixed
 */

STATIC APIRET Dlg3_PositionAndCreate(PDLGPRIVATE pDlgData,
                                     HWND *phwndFocusItem)       // out: item to give focus to
{
    APIRET arc = NO_ERROR;

    /*
     *  5) compute _positions_ of all controls
     *
     */

    // this was missing a return code, fixed V0.9.20 (2002-08-10) [umoeller]
    if (!(arc = ProcessAll(pDlgData,
                           PROCESS_6_CALC_POSITIONS)))
    {
        /*
         *  6) create control windows, finally
         *
         */

        pDlgData->ptlTotalOfs.x = DLG_OUTER_SPACING_X * FACTOR_X;
        pDlgData->ptlTotalOfs.y = DLG_OUTER_SPACING_Y * FACTOR_Y;

        // this was missing a return code, fixed V0.9.20 (2002-08-10) [umoeller]
        if (!(arc = ProcessAll(pDlgData,
                               PROCESS_7_CREATE_CONTROLS)))
        {
            if (pDlgData->hwndDefPushbutton)
            {
                // we had a default pushbutton:
                // go set it V0.9.14 (2001-08-21) [umoeller]
                WinSetWindowULong(pDlgData->hwndDlg,
                                  QWL_DEFBUTTON,
                                  pDlgData->hwndDefPushbutton);
                *phwndFocusItem = pDlgData->hwndDefPushbutton;
                        // V0.9.15 (2001-08-26) [umoeller]
            }
            else
                *phwndFocusItem = (pDlgData->hwndFirstFocus)
                                    ? pDlgData->hwndFirstFocus
                                    : pDlgData->hwndDlg;
        }
    }

    return arc;
}

/*
 *@@ Dlg9_Cleanup:
 *
 *@@added V0.9.15 (2001-08-26) [umoeller]
 */

STATIC VOID Dlg9_Cleanup(PDLGPRIVATE *ppDlgData)
{
    PDLGPRIVATE pDlgData;
    if (    (ppDlgData)
         && (pDlgData = *ppDlgData)
       )
    {
        PLISTNODE pTableNode;

        // clean up the tables
        FOR_ALL_NODES(&pDlgData->llTables, pTableNode)
        {
            PTABLEDEF pTable = (PTABLEDEF)pTableNode->pItemData;

            FreeTable(pTable);
                    // this may recurse for nested tables
        }

        lstClear(&pDlgData->llTables);

        free(pDlgData);
        *ppDlgData = NULL;
    }
}

/* ******************************************************************
 *
 *   Dialog formatter entry points
 *
 ********************************************************************/

/*
 *@@ dlghCreateDlg:
 *      replacement for WinCreateDlg/WinLoadDlg for creating a
 *      dialog from a settings array in memory, which is
 *      formatted automatically.
 *
 *      This does NOT use regular dialog templates from
 *      module resources. Instead, you pass in an array
 *      of DLGHITEM structures, which define the controls
 *      and how they are to be formatted.
 *
 *      The main advantage compared to dialog resources is
 *      that with this function, you will never have to
 *      define control _positions_. Instead, you only specify
 *      the control _sizes_, and all positions are computed
 *      automatically here. Even better, for many controls,
 *      auto-sizing is supported according to the control's
 *      text (e.g. for statics and checkboxes). This is
 *      quite similar to HTML tables.
 *
 *      A regular standard dialog would use something like
 *
 +          FCF_TITLEBAR | FCF_SYSMENU | FCF_DLGBORDER | FCF_NOBYTEALIGN | FCF_CLOSEBUTTON
 *
 *      for flCreateFlags. To make the dlg sizeable, specify
 *      FCF_SIZEBORDER instead of FCF_DLGBORDER.
 *
 *      dialog.h defines FCF_FIXED_DLG and FCF_SIZEABLE_DLG
 *      to make this more handy.
 *
 *      <B>Usage:</B>
 *
 *      Like WinLoadDlg, this creates a standard WC_FRAME and
 *      subclasses it with fnwpMyDlgProc. It then sends WM_INITDLG
 *      to the dialog with pCreateParams in mp2.
 *
 *      If this func returns no error, you can then use
 *      WinProcessDlg with the newly created dialog as usual. In
 *      your dlg proc, use WinDefDlgProc as usual.
 *
 *      There is NO run-time overhead for either code or memory
 *      after dialog creation; after this function returns, the
 *      dialog is a standard dialog as if loaded from WinLoadDlg.
 *      The array of DLGHITEM structures defines how the
 *      dialog is set up. All this is ONLY used by this function
 *      and NOT needed after the dialog has been created.
 *
 *      In DLGHITEM, the "Type" field determines what this
 *      structure defines. A number of handy macros have been
 *      defined to make this easier and to provide type-checking
 *      at compile time. See dialog.h for the complete (and
 *      ever-expanding) list of definitions.
 *
 *      See @dlg_using_macros for how to use these macros.
 *
 *      Essentially, such a dialog item operates similarly to
 *      HTML tables. There are rows and columns in the table,
 *      and each control which is specified must be a column
 *      in some table. Tables may also nest (see below).
 *
 *      See @dlg_algorithm for the gory details of the new
 *      algorithm used since V1.0.0.
 *
 *      See @dlg_boxmodel for information about how the
 *      rectangles are defined.
 *
 *      <B>Example:</B>
 *
 *      The typical calling sequence would be:
 *
 +          HWND hwndDlg = NULLHANDLE;
 +          if (NO_ERROR == dlghCreateDlg(&hwndDlg,
 +                                        hwndOwner,
 +                                        FCF_TITLEBAR | FCF_SYSMENU
 +                                           | FCF_DLGBORDER | FCF_NOBYTEALIGN,
 +                                        fnwpMyDlgProc,
 +                                        "My Dlg Title",
 +                                        dlgTemplate,      // DLGHITEM array
 +                                        ARRAYITEMCOUNT(dlgTemplate),
 +                                        NULL,             // mp2 for WM_INITDLG
 +                                        "9.WarpSans"))    // default font
 +          {
 +              ULONG idReturn = WinProcessDlg(hwndDlg);
 +              WinDestroyWindow(hwndDlg);
 +          }
 *
 *      <B>Errors:</B>
 *
 *      This does not return a HWND, but an APIRET. This will be
 *      one of the following:
 *
 *      --  NO_ERROR: only in that case, the phwndDlg ptr
 *          receives the HWND of the new dialog, which can
 *          then be given to WinProcessDlg. Don't forget
 *          WinDestroyWindow.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  DLGERR_ROW_BEFORE_TABLE: a row definition appeared
 *          outside a table definition.
 *
 *      --  DLGERR_CONTROL_BEFORE_ROW: a control definition
 *          appeared right after a table definition. You must
 *          specify a row first.
 *
 *      --  DLGERR_NULL_CTL_DEF: A required CONTROLDEF ptr
 *          was NULL.
 *
 *      --  DLGERR_CANNOT_CREATE_FRAME: unable to create the
 *          WC_FRAME window. Typically an invalid owner was
 *          specified.
 *
 *      --  DLGERR_INVALID_CODE: invalid "Type" field in
 *          DLGHITEM.
 *
 *      --  DLGERR_TABLE_NOT_CLOSED, DLGERR_TOO_MANY_TABLES_CLOSED:
 *          improper nesting of TYPE_START_NEW_TABLE and
 *          TYPE_END_TABLE fields.
 *
 *      --  DLGERR_CANNOT_CREATE_CONTROL: creation of some
 *          sub-control failed. Maybe an invalid window class
 *          was specified.
 *
 *      --  DLGERR_INVALID_CONTROL_TITLE: bad window title in
 *          control.
 *
 *      --  DLGERR_INVALID_STATIC_BITMAP: static bitmap contains
 *          an invalid bitmap handle.
 *
 *@@changed V0.9.14 (2001-07-07) [umoeller]: fixed disabled mouse with hwndOwner == HWND_DESKTOP
 *@@changed V0.9.14 (2001-08-01) [umoeller]: fixed major memory leaks with nested tables
 *@@changed V0.9.14 (2001-08-21) [umoeller]: fixed default push button problems
 *@@changed V0.9.16 (2001-12-06) [umoeller]: fixed bad owner if not direct desktop child
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added excpt handling
 +@@changed V1.0.0 (2002-08-21) [umoeller]: now setting wait pointer
 */

APIRET dlghCreateDlg(HWND *phwndDlg,            // out: new dialog
                     HWND hwndOwner,            // in: owner for dialog
                     ULONG flCreateFlags,       // in: standard FCF_* frame flags
                     PFNWP pfnwpDialogProc,     // in: dialog winproc or WinDefDlgProc
                     PCSZ pcszDlgTitle,         // in: title to set for dlg frame's titlebar
                     PCDLGHITEM paDlgItems,     // in: definition array with tables, rows, and columns
                     ULONG cDlgItems,           // in: ARRAYITEMCOUNT(paDlgItems)
                     PVOID pCreateParams,       // in: create param for mp2 of WM_INITDLG
                     PCSZ pcszControlsFont)     // in: font for ctls with CTL_COMMON_FONT
{
    APIRET      arc = NO_ERROR;

    volatile HPOINTER hptrOld = winhSetWaitPointer(); // XWP V1.0.4 (2005-10-09) [pr]

    TRY_LOUD(excpt1)
    {
        ULONG       ul;

        PDLGPRIVATE  pDlgData = NULL;

        HWND        hwndDesktop = WinQueryDesktopWindow(NULLHANDLE, NULLHANDLE);
                                            // works with a null HAB

        /*
         *  1) parse the table and create structures from it
         *
         */

        if (!(arc = Dlg0_Init(&pDlgData,
                              pcszControlsFont,
                              NULL)))
        {
            if (!(arc = Dlg1_ParseTables(pDlgData,
                                         paDlgItems,
                                         cDlgItems)))
            {
                /*
                 *  2) create empty dialog frame
                 *
                 */

                FRAMECDATA      fcData = {0};
                ULONG           flStyle = 0;
                HWND            hwndOwnersParent;

                fcData.cb = sizeof(FRAMECDATA);
                fcData.flCreateFlags = flCreateFlags | 0x40000000L;

                if (flCreateFlags & FCF_SIZEBORDER)
                    // dialog has size border:
                    // add "clip siblings" style
                    flStyle |= WS_CLIPSIBLINGS;

                if (hwndOwner == HWND_DESKTOP)
                    // there's some dumb XWorkplace code left
                    // which uses this, and this disables the
                    // mouse for some reason
                    // V0.9.14 (2001-07-07) [umoeller]
                    hwndOwner = NULLHANDLE;

                // now, make sure the owner window is child of
                // HWND_DESKTOP... if it is not, we'll only disable
                // some dumb child window, which is not sufficient
                // V0.9.16 (2001-12-06) [umoeller]
                while (    (hwndOwner)
                        && (hwndOwnersParent = WinQueryWindow(hwndOwner, QW_PARENT))
                        && (hwndOwnersParent != hwndDesktop)
                      )
                    hwndOwner = hwndOwnersParent;

                if (!(pDlgData->hwndDlg = WinCreateWindow(HWND_DESKTOP,
                                                          WC_FRAME,
                                                          (PSZ)pcszDlgTitle,
                                                          flStyle,        // style; invisible for now
                                                          0, 0, 0, 0,
                                                          hwndOwner,
                                                          HWND_TOP,
                                                          0,              // ID
                                                          &fcData,
                                                          NULL)))          // presparams
                    arc = DLGERR_CANNOT_CREATE_FRAME;
                else
                {
                    HWND    hwndDlg = pDlgData->hwndDlg;
                    HWND    hwndFocusItem = NULLHANDLE;
                    RECTL   rclClient;

                    /*
                     *  3) compute size of all controls
                     *
                     */

                    if (!(arc = Dlg2_CalcSizes(pDlgData)))
                    {
                        WinSubclassWindow(hwndDlg, pfnwpDialogProc);

                        /*
                         *  4) compute size of dialog client from total
                         *     size of all controls
                         */

                        // calculate the frame size from the client size
                        rclClient.xLeft = DLG_OUTER_SPACING_X * FACTOR_X; // 10;
                        rclClient.yBottom = DLG_OUTER_SPACING_Y * FACTOR_Y; // 10;
                        rclClient.xRight =   pDlgData->szlClient.cx
                                           + 2 * (DLG_OUTER_SPACING_X * FACTOR_X)
                                           - 1;
                        rclClient.yTop   =   pDlgData->szlClient.cy
                                           + 2 * (DLG_OUTER_SPACING_Y * FACTOR_Y)
                                           - 1;
                        WinCalcFrameRect(hwndDlg,
                                         &rclClient,
                                         FALSE);            // frame from client

                        WinSetWindowPos(hwndDlg,
                                        0,
                                        10,
                                        10,
                                        rclClient.xRight,
                                        rclClient.yTop,
                                        SWP_MOVE | SWP_SIZE | SWP_NOADJUST);

                        arc = Dlg3_PositionAndCreate(pDlgData,
                                                     &hwndFocusItem);

                        /*
                         *  7) WM_INITDLG, set focus
                         *
                         */

                        if (!WinSendMsg(pDlgData->hwndDlg,
                                        WM_INITDLG,
                                        (MPARAM)hwndFocusItem,
                                        (MPARAM)pCreateParams))
                        {
                            // if WM_INITDLG returns FALSE, this means
                            // the dlg proc has not changed the focus;
                            // we must then set the focus here
                            WinSetFocus(HWND_DESKTOP, hwndFocusItem);
                        }
                    }
                }
            }

            if (arc)
            {
                // error: clean up
                if (pDlgData->hwndDlg)
                {
                    WinDestroyWindow(pDlgData->hwndDlg);
                    pDlgData->hwndDlg = NULLHANDLE;
                }
            }
            else
                // no error: output dialog
                *phwndDlg = pDlgData->hwndDlg;

            Dlg9_Cleanup(&pDlgData);
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (arc)
    {
        CHAR szErr[300];
        sprintf(szErr, "Error %d occurred in " __FUNCTION__ ".", arc);
        winhDebugBox(hwndOwner,
                     "Error in Dialog Manager",
                     szErr);
    }

    WinSetPointer(HWND_DESKTOP, hptrOld); // V1.0.0 (2002-08-21) [umoeller]

    return arc;
}

/*
 *@@ dlghFormatDlg:
 *      similar to dlghCreateDlg in that this can
 *      dynamically format dialog items.
 *
 *      The differences however are the following:
 *
 *      --  This assumes that hwndDlg already points
 *          to a valid dialog frame and that this
 *          dialog should be modified according to
 *          flFlags.
 *
 *      This is what's used in XWorkplace for notebook
 *      settings pages since these always have to be
 *      based on a resource dialog (which is loaded
 *      empty).
 *
 *      flFlags can be any combination of the following:
 *
 *      --  DFFL_CREATECONTROLS: paDlgItems points to
 *          an array of cDlgItems DLGHITEM structures
 *          (see dlghCreateDlg) which is used for creating
 *          subwindows in hwndDlg. By using this flag, the
 *          function will essentially work like dlghCreateDlg,
 *          except that the frame is already created.
 *
 *      If pszlClient is specified, it receives the required
 *      size of the client to surround all controls properly.
 *      You can then use dlghResizeFrame to resize the frame
 *      with a bit of spacing, if desired.
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 *@@changed V0.9.18 (2002-03-03) [umoeller]: added pszlClient, fixed output
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added excpt handling
 */

APIRET dlghFormatDlg(HWND hwndDlg,              // in: dialog frame to work on
                     PCDLGHITEM paDlgItems,     // in: definition array
                     ULONG cDlgItems,           // in: array item count (NOT array size)
                     PCSZ pcszControlsFont,     // in: font for ctls with CTL_COMMON_FONT
                     ULONG flFlags,             // in: DFFL_* flags
                     PSIZEL pszlClient,         // out: size of all controls (ptr can be NULL)
                     PVOID *ppllControls)       // out: new LINKLIST receiving HWNDs of created controls (ptr can be NULL)
{
    APIRET      arc = NO_ERROR;

    TRY_LOUD(excpt1)
    {
        ULONG       ul;

        PDLGPRIVATE  pDlgData = NULL;
        PLINKLIST   pllControls = NULL;

        /*
         *  1) parse the table and create structures from it
         *
         */

        if (ppllControls)
            pllControls = *(PLINKLIST*)ppllControls = lstCreate(FALSE);

        if (!(arc = Dlg0_Init(&pDlgData,
                              pcszControlsFont,
                              pllControls)))
        {
            if (!(arc = Dlg1_ParseTables(pDlgData,
                                         paDlgItems,
                                         cDlgItems)))
            {
                HWND hwndFocusItem;

                /*
                 *  2) create empty dialog frame
                 *
                 */

                pDlgData->hwndDlg = hwndDlg;

                /*
                 *  3) compute size of all controls
                 *
                 */

                Dlg2_CalcSizes(pDlgData);

                if (pszlClient)
                {
                    pszlClient->cx =    pDlgData->szlClient.cx
                                      + 2 * (DLG_OUTER_SPACING_X * FACTOR_X);
                    pszlClient->cy =    pDlgData->szlClient.cy
                                      + 2 * (DLG_OUTER_SPACING_Y * FACTOR_Y);
                }

                if (flFlags & DFFL_CREATECONTROLS)
                {
                    if (!(arc = Dlg3_PositionAndCreate(pDlgData,
                                                       &hwndFocusItem)))
                        WinSetFocus(HWND_DESKTOP, hwndFocusItem);
                }
            }

            Dlg9_Cleanup(&pDlgData);
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (arc)
    {
        CHAR szErr[300];
        sprintf(szErr, "Error %d occurred in " __FUNCTION__ ".", arc);
        winhDebugBox(NULLHANDLE,
                     "Error in Dialog Manager",
                     szErr);
    }

    return arc;
}

/*
 *@@ dlghResizeFrame:
 *
 *@@added V0.9.18 (2002-03-03) [umoeller]
 */

VOID dlghResizeFrame(HWND hwndDlg,
                     PSIZEL pszlClient)
{
    // calculate the frame size from the client size
    RECTL   rclClient;
    rclClient.xLeft = 10;
    rclClient.yBottom = 10;
    rclClient.xRight = pszlClient->cx;
    rclClient.yTop = pszlClient->cy;
    WinCalcFrameRect(hwndDlg,
                     &rclClient,
                     FALSE);            // frame from client

    WinSetWindowPos(hwndDlg,
                    0,
                    10,
                    10,
                    rclClient.xRight,
                    rclClient.yTop,
                    SWP_MOVE | SWP_SIZE | SWP_NOADJUST);
}

/* ******************************************************************
 *
 *   Dialog arrays
 *
 ********************************************************************/

/*
 *@@ dlghCreateArray:
 *      creates a "dialog array" for dynamically
 *      building a dialog template in memory.
 *
 *      A dialog array is simply an array of
 *      DLGHITEM structures, as you would normally
 *      define them statically in the source.
 *      However, there are situations where you
 *      might want to leave out certain controls
 *      depending on certain conditions, which
 *      can be difficult with static arrays.
 *
 *      As a result, these "array" functions have
 *      been added to allow for adding static
 *      DLGHITEM subarrays to a dynamic array in
 *      memory, which can then be passed to the
 *      formatter.
 *
 *      Usage:
 *
 *      1)  Call this function with the maximum
 *          amount of DLGHITEM's that will need
 *          to be allocated in cMaxItems. Set this
 *          to the total sum of all DLGHITEM's
 *          in all the subarrays.
 *
 *      2)  For each of the subarrays, call
 *          dlghAppendToArray to have the subarray
 *          appended to the dialog array.
 *          After each call, DLGARRAY.cDlgItemsNow
 *          will contain the actual total count of
 *          DLGHITEM's that were added.
 *
 *      3)  Call dlghCreateDialog with the dialog
 *          array.
 *
 *      4)  Call dlghFreeArray.
 *
 *      Sort of like this (error checking omitted):
 *
 +      DLGHITEM    dlgSampleFront =  ...   // always included
 +      DLGHITEM    dlgSampleSometimes =  ...   // not always included
 +      DLGHITEM    dlgSampleTail =  ...   // always included
 +
 +      PDLGARRAY pArraySample = NULL;
 +      // create array with sufficient size
 +      dlghCreateArray(   ARRAYITEMCOUNT(dlgSampleFront)
 +                       + ARRAYITEMCOUNT(dlgSampleSometimes)
 +                       + ARRAYITEMCOUNT(dlgSampleTail),
 +                      &pArraySample);
 +
 +      // always include front
 +      dlghAppendToArray(pArraySample,
 +                        dlgSampleFront,
 +                        ARRAYITEMCOUNT(dlgSampleFront));
 +      // include "sometimes" conditionally
 +      if (...)
 +          dlghAppendToArray(pArraySample,
 +                            dlgSampleSometimes,
 +                            ARRAYITEMCOUNT(dlgSampleSometimes));
 +      // include tail always
 +      dlghAppendToArray(pArraySample,
 +                        dlgSampleTail,
 +                        ARRAYITEMCOUNT(dlgSampleTail));
 +
 +      // now create the dialog from the array
 +      dlghCreateDialog(&hwndDlg,
 +                       hwndOwner,
 +                       FCF_ ...
 +                       fnwpMyDialogProc,
 +                       "Title",
 +                       pArray->paDlgItems,        // dialog array!
 +                       pArray->cDlgItemsNow,      // real count of items!
 +                       NULL,
 +                       NULL);
 +
 +      dlghFreeArray(&pArraySample);
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 */

APIRET dlghCreateArray(ULONG cMaxItems,
                       PDLGARRAY *ppArray)       // out: DLGARRAY
{
    APIRET arc = NO_ERROR;
    PDLGARRAY pArray;

    if (pArray = NEW(DLGARRAY))
    {
        ULONG cb;

        ZERO(pArray);
        if (    (cb = cMaxItems * sizeof(DLGHITEM))
             && (pArray->paDlgItems = (DLGHITEM*)malloc(cb))
           )
        {
            memset(pArray->paDlgItems, 0, cb);
            pArray->cDlgItemsMax = cMaxItems;
            *ppArray = pArray;
        }
        else
            arc = ERROR_NOT_ENOUGH_MEMORY;

        if (arc)
            dlghFreeArray(&pArray);
    }
    else
        arc = ERROR_NOT_ENOUGH_MEMORY;

    return arc;
}

/*
 *@@ dlghFreeArray:
 *      frees a dialog array created by dlghCreateArray.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 */

APIRET dlghFreeArray(PDLGARRAY *ppArray)
{
    PDLGARRAY pArray;
    if (    (ppArray)
         && (pArray = *ppArray)
       )
    {
        if (pArray->paDlgItems)
            free(pArray->paDlgItems);
        free(pArray);
    }
    else
        return ERROR_INVALID_PARAMETER;

    return NO_ERROR;
}

/*
 *@@ dlghAppendToArray:
 *      appends a subarray of DLGHITEM's to the
 *      given DLGARRAY. See dlghCreateArray for
 *      usage.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  DLGERR_ARRAY_TOO_SMALL: pArray does not
 *          have enough memory to hold the new items.
 *          The cMaxItems parameter given to dlghCreateArray
 *          wasn't large enough.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 */

APIRET dlghAppendToArray(PDLGARRAY pArray,      // in: dialog array created by dlghCreateArray
                         PCDLGHITEM paItems,    // in: subarray to be appended
                         ULONG cItems)          // in: subarray item count (NOT array size)
{
    APIRET arc = NO_ERROR;
    if (pArray)
    {
        if (    (pArray->cDlgItemsMax >= cItems)
             && (pArray->cDlgItemsMax - pArray->cDlgItemsNow >= cItems)
           )
        {
            // enough space left in the array:
            memcpy(&pArray->paDlgItems[pArray->cDlgItemsNow],
                   paItems,     // source
                   cItems * sizeof(DLGHITEM));
            pArray->cDlgItemsNow += cItems;
        }
        else
            arc = DLGERR_ARRAY_TOO_SMALL;
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

/* ******************************************************************
 *
 *   Standard dialogs
 *
 ********************************************************************/

/*
 *@@ fnwpMessageBox:
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 */

MRESULT EXPENTRY fnwpMessageBox(HWND hwndBox, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_HELP:
        {
            PFNHELP pfnHelp;
            if (pfnHelp = (PFNHELP)WinQueryWindowPtr(hwndBox, QWL_USER))
                pfnHelp(hwndBox);

            return 0;
        }
    }

    return WinDefDlgProc(hwndBox, msg, mp1, mp2);
}

/*
 *@@ dlghCreateMessageBox:
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 *@@changed V0.9.14 (2001-07-26) [umoeller]: fixed missing focus on buttons
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added pfnHelp
 *@@changed V0.9.20 (2002-07-12) [umoeller]: made icon spacing wider
 *@@changed V0.9.20 (2002-08-10) [umoeller]: fixed missing close button
 *@@changed V1.0.0 (2002-08-16) [umoeller]: now using table alignment
 *@@changed WarpIN V1.0.11 (2006-05-22) [pr]: add to Tasklist
 */

APIRET dlghCreateMessageBox(HWND *phwndDlg,
                            HWND hwndOwner,
                            HPOINTER hptrIcon,
                            PCSZ pcszTitle,
                            PCSZ pcszMessage,
                            PFNHELP pfnHelp,           // in: help callback or NULL
                            ULONG flFlags,
                            PCSZ pcszFont,
                            const MSGBOXSTRINGS *pStrings,
                            PULONG pulAlarmFlag)      // out: alarm sound to be played
{
    APIRET arc;

    CONTROLDEF
        Icon = CONTROLDEF_ICON_WIDER(NULLHANDLE, 0),
                // made icon spacing wider V0.9.20 (2002-07-12) [umoeller]
        Spacing = CONTROLDEF_TEXT(NULL, 0, 1, 1),
        InfoText = CONTROLDEF_TEXT_WORDBREAK(NULL, 10, MSGBOX_TEXTWIDTH),
        Buttons[] =
        {
            CONTROLDEF_PUSHBUTTON(NULL, 1, STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
            CONTROLDEF_PUSHBUTTON(NULL, 2, STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
            CONTROLDEF_PUSHBUTTON(NULL, 3, STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
            CONTROLDEF_HELPPUSHBUTTON(NULL, 4, STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT)
        };

    DLGHITEM MessageBoxFront[] =
                {
                    START_TABLE_ALIGN,
                        START_ROW(ROW_VALIGN_CENTER),
                            CONTROL_DEF(&Icon),
                            START_TABLE,
                                START_ROW(ROW_VALIGN_CENTER),
                                    CONTROL_DEF(&Spacing),
                                START_ROW(ROW_VALIGN_CENTER),
                                    CONTROL_DEF(&InfoText),
                                START_ROW(ROW_VALIGN_CENTER),
                                    CONTROL_DEF(&Spacing),
                            END_TABLE,
                        START_ROW(ROW_VALIGN_CENTER),
                            CONTROL_DEF(&Spacing),
                            START_TABLE,
                                START_ROW(ROW_VALIGN_CENTER),
                                    CONTROL_DEF(&Buttons[0]),
                                    CONTROL_DEF(&Buttons[1]),
                                    CONTROL_DEF(&Buttons[2]),
                },
            MessageBoxHelp[] =
                {
                                    CONTROL_DEF(&Buttons[3]),
                },
            MessageBoxTail[] =
                {
                            END_TABLE,
                    END_TABLE
                };

    ULONG flButtons = flFlags & 0xF;        // low nibble contains MB_YESNO etc.
    PDLGARRAY pArrayBox;

    PCSZ        p0 = "Error",
                p1 = NULL,
                p2 = NULL;

    Icon.pcszText = (PCSZ)hptrIcon;
    InfoText.pcszText = pcszMessage;

    // now work on the three buttons of the dlg template:
    // give them proper titles or hide them
    if (flButtons == MB_OK)
    {
        p0 = pStrings->pcszOK;
    }
    else if (flButtons == MB_OKCANCEL)
    {
        p0 = pStrings->pcszOK;
        p1 = pStrings->pcszCancel;
    }
    else if (flButtons == MB_RETRYCANCEL)
    {
        p0 = pStrings->pcszRetry;
        p1 = pStrings->pcszCancel;
    }
    else if (flButtons == MB_ABORTRETRYIGNORE)
    {
        p0 = pStrings->pcszAbort;
        p1 = pStrings->pcszRetry;
        p2 = pStrings->pcszIgnore;
    }
    else if (flButtons == MB_YESNO)
    {
        p0 = pStrings->pcszYes;
        p1 = pStrings->pcszNo;
    }
    else if (flButtons == MB_YESNOCANCEL)
    {
        p0 = pStrings->pcszYes;
        p1 = pStrings->pcszNo;
        p2 = pStrings->pcszCancel;
    }
    else if (flButtons == MB_CANCEL)
    {
        p0 = pStrings->pcszCancel;
    }
    else if (flButtons == MB_ENTER)
    {
        p0 = pStrings->pcszEnter;
    }
    else if (flButtons == MB_ENTERCANCEL)
    {
        p0 = pStrings->pcszEnter;
        p1 = pStrings->pcszCancel;
    }
    else if (flButtons == MB_YES_YES2ALL_NO)
    {
        p0 = pStrings->pcszYes;
        p1 = pStrings->pcszYesToAll;
        p2 = pStrings->pcszNo;
    }

    // now set strings and hide empty buttons
    Buttons[0].pcszText = p0;

    if (p1)
        Buttons[1].pcszText = p1;
    else
        Buttons[1].flStyle &= ~WS_VISIBLE;

    if (p2)
        Buttons[2].pcszText = p2;
    else
        Buttons[2].flStyle &= ~WS_VISIBLE;

    // query default button IDs
    if (flFlags & MB_DEFBUTTON2)
        Buttons[1].flStyle |= BS_DEFAULT;
    else if (flFlags & MB_DEFBUTTON3)
        Buttons[2].flStyle |= BS_DEFAULT;
    else
        Buttons[0].flStyle |= BS_DEFAULT;

    *pulAlarmFlag = WA_NOTE;
    if (flFlags & (MB_ICONHAND | MB_ERROR))
        *pulAlarmFlag = WA_ERROR;
    else if (flFlags & (MB_ICONEXCLAMATION | MB_WARNING))
        *pulAlarmFlag = WA_WARNING;

    if (pfnHelp)
        Buttons[3].pcszText = pStrings->pcszHelp;

    if (!(arc = dlghCreateArray(   ARRAYITEMCOUNT(MessageBoxFront)
                                 + ARRAYITEMCOUNT(MessageBoxHelp)
                                 + ARRAYITEMCOUNT(MessageBoxTail),
                                &pArrayBox)))
    {
        if (    (!(arc = dlghAppendToArray(pArrayBox,
                                           MessageBoxFront,
                                           ARRAYITEMCOUNT(MessageBoxFront))))
             && (    (!pfnHelp)
                  || (!(arc = dlghAppendToArray(pArrayBox,
                                                MessageBoxHelp,
                                                ARRAYITEMCOUNT(MessageBoxHelp))))
                )
             && (!(arc = dlghAppendToArray(pArrayBox,
                                           MessageBoxTail,
                                           ARRAYITEMCOUNT(MessageBoxTail))))
           )
        {
            if (!(arc = dlghCreateDlg(phwndDlg,
                                      hwndOwner,
                                      FCF_TITLEBAR | FCF_SYSMENU | FCF_DLGBORDER | FCF_NOBYTEALIGN
                                            | FCF_CLOSEBUTTON,      // was missing V0.9.20 (2002-08-10) [umoeller]
                                      fnwpMessageBox,
                                      pcszTitle,
                                      pArrayBox->paDlgItems,
                                      pArrayBox->cDlgItemsNow,
                                      NULL,
                                      pcszFont)))
            {
                // WarpIN V1.0.11 (2006-05-22) [pr]
                winhAddToTasklist(*phwndDlg, NULLHANDLE);
                // added help callback V0.9.19 (2002-04-24) [umoeller]
                WinSetWindowPtr(*phwndDlg, QWL_USER, (PVOID)pfnHelp);
            }
        }

        dlghFreeArray(&pArrayBox);
    }

    return arc;
}

/*
 *@@ dlghProcessMessageBox:
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 */

ULONG dlghProcessMessageBox(HWND hwndDlg,
                            ULONG ulAlarmFlag,
                            ULONG flFlags)
{
    ULONG ulrcDlg;
    ULONG flButtons = flFlags & 0xF;        // low nibble contains MB_YESNO etc.

    winhCenterWindow(hwndDlg);

    if (flFlags & MB_SYSTEMMODAL)
        WinSetSysModalWindow(HWND_DESKTOP, hwndDlg);

    if (ulAlarmFlag)
        WinAlarm(HWND_DESKTOP, ulAlarmFlag);

    ulrcDlg = WinProcessDlg(hwndDlg);

    WinDestroyWindow(hwndDlg);

    if (flButtons == MB_OK)
        return MBID_OK;
    else if (flButtons == MB_OKCANCEL)
        switch (ulrcDlg)
        {
            case 1:     return MBID_OK;
            default:    return MBID_CANCEL;
        }
    else if (flButtons == MB_RETRYCANCEL)
        switch (ulrcDlg)
        {
            case 1:     return MBID_RETRY;
            default:    return MBID_CANCEL;
        }
    else if (flButtons == MB_ABORTRETRYIGNORE)
        switch (ulrcDlg)
        {
            case 2:     return MBID_RETRY;
            case 3:     return MBID_IGNORE;
            default:    return MBID_ABORT;
        }
    else if (flButtons == MB_YESNO)
        switch (ulrcDlg)
        {
            case 1:     return MBID_YES;
            default:    return MBID_NO;
        }
    else if (flButtons == MB_YESNOCANCEL)
        switch (ulrcDlg)
        {
            case 1:     return MBID_YES;
            case 2:     return MBID_NO;
            default:    return MBID_CANCEL;
        }
    else if (flButtons == MB_CANCEL)
        return MBID_CANCEL;
    else if (flButtons == MB_ENTER)
        return MBID_ENTER;
    else if (flButtons == MB_ENTERCANCEL)
        switch (ulrcDlg)
        {
            case 1:     return MBID_ENTER;
            default:    return MBID_CANCEL;
        }
    else if (flButtons == MB_YES_YES2ALL_NO)
        switch (ulrcDlg)
        {
            case 1:     return MBID_YES;
            case 2:     return MBID_YES2ALL;
            default:    return MBID_NO;
        }

    return MBID_CANCEL;
}

/*
 *@@ dlghMessageBox:
 *      WinMessageBox replacement, which uses dlghCreateDlg
 *      internally.
 *
 *      This has all the flags of the standard call,
 *      but looks much prettier. Besides, it allows
 *      you to specify any icon to be displayed.
 *
 *      Currently the following flStyle's are supported:
 *
 *      -- MB_OK                      0x0000
 *      -- MB_OKCANCEL                0x0001
 *      -- MB_RETRYCANCEL             0x0002
 *      -- MB_ABORTRETRYIGNORE        0x0003
 *      -- MB_YESNO                   0x0004
 *      -- MB_YESNOCANCEL             0x0005
 *      -- MB_CANCEL                  0x0006
 *      -- MB_ENTER                   0x0007 (not implemented yet)
 *      -- MB_ENTERCANCEL             0x0008 (not implemented yet)
 *
 *      -- MB_YES_YES2ALL_NO          0x0009
 *          This is new: this has three buttons called "Yes"
 *          (MBID_YES), "Yes to all" (MBID_YES2ALL), "No" (MBID_NO).
 *
 *      -- MB_DEFBUTTON2            (for two-button styles)
 *      -- MB_DEFBUTTON3            (for three-button styles)
 *
 *      -- MB_ICONHAND
 *      -- MB_ICONEXCLAMATION
 *
 *      If (pfnHelp != NULL), a "Help" button is also added and
 *      pfnHelp gets called when the user presses it or the F1
 *      key.
 *
 *      Returns MBID_* codes like WinMessageBox.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added pfnHelp
 */

ULONG dlghMessageBox(HWND hwndOwner,            // in: owner for msg box
                     HPOINTER hptrIcon,         // in: icon to display
                     PCSZ pcszTitle,            // in: title
                     PCSZ pcszMessage,          // in: message
                     PFNHELP pfnHelp,           // in: help callback or NULL
                     ULONG flFlags,             // in: standard message box flags
                     PCSZ pcszFont,             // in: font (e.g. "9.WarpSans")
                     const MSGBOXSTRINGS *pStrings) // in: strings array
{
    HWND    hwndDlg;
    ULONG   ulAlarmFlag;
    APIRET  arc;
    CHAR    szMsg[100];

    if (    (!(arc = dlghCreateMessageBox(&hwndDlg,
                                          hwndOwner,
                                          hptrIcon,
                                          pcszTitle,
                                          pcszMessage,
                                          pfnHelp,
                                          flFlags,
                                          pcszFont,
                                          pStrings,
                                          &ulAlarmFlag)))
         && (hwndDlg)
       )
    {
        // SHOW DIALOG
        return dlghProcessMessageBox(hwndDlg,
                                     ulAlarmFlag,
                                     flFlags);
    }

    sprintf(szMsg, "dlghCreateMessageBox reported error %u.", arc);
    WinMessageBox(HWND_DESKTOP,
                  NULLHANDLE,
                  "Error",
                  szMsg,
                  0,
                  MB_CANCEL | MB_MOVEABLE);

    return DID_CANCEL;
}

/*
 *@@ cmnTextEntryBox:
 *      common dialog for entering a text string.
 *      The dialog has a descriptive text on top
 *      with an entry field below and "OK" and "Cancel"
 *      buttons.
 *
 *      The string from the user is returned in a
 *      new buffer, which must be free'd by the caller.
 *      Returns NULL if the user pressed "Cancel".
 *
 *      fl can be any combination of the following
 *      flags:
 *
 *      --  TEBF_REMOVETILDE: tilde ("~") characters
 *          are removed from pcszTitle before setting
 *          the title. Useful for reusing menu item
 *          texts.
 *
 *      --  TEBF_REMOVEELLIPSE: ellipse ("...") strings
 *          are removed from pcszTitle before setting
 *          the title. Useful for reusing menu item
 *          texts.
 *
 *      --  TEBF_SELECTALL: the default text in the
 *          entry field is initially highlighted.
 *
 *@@added V0.9.15 (2001-09-14) [umoeller]
 */

PSZ dlghTextEntryBox(HWND hwndOwner,
                     PCSZ pcszTitle,          // in: dlg title
                     PCSZ pcszDescription,    // in: descriptive text above entry field
                     PCSZ pcszDefault,        // in: default text for entry field or NULL
                     PCSZ pcszOK,             // in: "OK" string
                     PCSZ pcszCancel,         // in: "Cancel" string
                     ULONG ulMaxLen,                 // in: maximum length for entry
                     ULONG fl,                       // in: TEBF_* flags
                     PCSZ pcszFont)           // in: font (e.g. "9.WarpSans")
{
    CONTROLDEF
        Static = CONTROLDEF_TEXT_WORDBREAK(
                            NULL,
                            -1,
                            150),
        Entry = CONTROLDEF_ENTRYFIELD(
                            NULL,
                            999,
                            150,
                            SZL_AUTOSIZE),
        OKButton = CONTROLDEF_DEFPUSHBUTTON(
                            NULL,
                            DID_OK,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
        CancelButton = CONTROLDEF_PUSHBUTTON(
                            NULL,
                            DID_CANCEL,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT);

    DLGHITEM DlgTemplate[] =
        {
            START_TABLE,
                START_ROW(0),
                    CONTROL_DEF(&Static),
                START_ROW(0),
                    CONTROL_DEF(&Entry),
                START_ROW(0),
                    CONTROL_DEF(&OKButton),
                    CONTROL_DEF(&CancelButton),
            END_TABLE
        };

    HWND hwndDlg = NULLHANDLE;
    PSZ  pszReturn = NULL;
    XSTRING strTitle;

    xstrInitCopy(&strTitle, pcszTitle, 0);

    if (fl & (TEBF_REMOVEELLIPSE | TEBF_REMOVETILDE))
    {
        ULONG ulOfs;
        if (fl & TEBF_REMOVEELLIPSE)
        {
            ulOfs = 0;
            while (xstrFindReplaceC(&strTitle,
                                    &ulOfs,
                                    "...",
                                    ""))
                ;
        }

        if (fl & TEBF_REMOVETILDE)
        {
            ulOfs = 0;
            while (xstrFindReplaceC(&strTitle,
                                    &ulOfs,
                                    "~",
                                    ""))
                ;
        }
    }

    Static.pcszText = pcszDescription;

    OKButton.pcszText = pcszOK;
    CancelButton.pcszText = pcszCancel;

    if (NO_ERROR == dlghCreateDlg(&hwndDlg,
                                  hwndOwner,
                                  FCF_FIXED_DLG,
                                  WinDefDlgProc,
                                  strTitle.psz,
                                  DlgTemplate,      // DLGHITEM array
                                  ARRAYITEMCOUNT(DlgTemplate),
                                  NULL,
                                  pcszFont))
    {
        HWND hwndEF = WinWindowFromID(hwndDlg, 999);
        winhCenterWindow(hwndDlg);
        winhSetEntryFieldLimit(hwndEF, ulMaxLen);
        if (pcszDefault)
        {
            WinSetWindowText(hwndEF, (PSZ)pcszDefault);
            if (fl & TEBF_SELECTALL)
                winhEntryFieldSelectAll(hwndEF);
        }
        WinSetFocus(HWND_DESKTOP, hwndEF);
        if (DID_OK == WinProcessDlg(hwndDlg))
            pszReturn = winhQueryWindowText(hwndEF);

        WinDestroyWindow(hwndDlg);
    }

    xstrClear(&strTitle);

    return pszReturn;
}

/* ******************************************************************
 *
 *   Dialog input handlers
 *
 ********************************************************************/

/*
 *@@ dlghSetPrevFocus:
 *      "backward" function for rotating the focus
 *      in a dialog when the "shift+tab" keys get
 *      pressed.
 *
 *      pllWindows must be a linked list with the
 *      plain HWND window handles of the focussable
 *      controls in the dialog.
 */

VOID dlghSetPrevFocus(PVOID pvllWindows)
{
    PLINKLIST   pllWindows = (PLINKLIST)pvllWindows;

    // check current focus
    HWND        hwndFocus = WinQueryFocus(HWND_DESKTOP);

    PLISTNODE   pNode = lstNodeFromItem(pllWindows, (PVOID)hwndFocus);

    BOOL fRestart = FALSE;

    while (pNode)
    {
        CHAR    szClass[100];

        // previos node
        pNode = pNode->pPrevious;

        if (    (!pNode)        // no next, or not found:
             && (!fRestart)     // avoid infinite looping if bad list
           )
        {
            pNode = lstQueryLastNode(pllWindows);
            fRestart = TRUE;
        }

        if (pNode)
        {
            // check if this is a focusable control
            if (WinQueryClassName((HWND)pNode->pItemData,
                                  sizeof(szClass),
                                  szClass))
            {
                if (    (strcmp(szClass, "#5"))    // not static
                   )
                    break;
                // else: go for next then
            }
        }
    }

    if (pNode)
    {
        WinSetFocus(HWND_DESKTOP,
                    (HWND)pNode->pItemData);
    }
}

/*
 *@@ dlghSetNextFocus:
 *      "forward" function for rotating the focus
 *      in a dialog when the "ab" key gets pressed.
 *
 *      pllWindows must be a linked list with the
 *      plain HWND window handles of the focussable
 *      controls in the dialog.
 */

VOID dlghSetNextFocus(PVOID pvllWindows)
{
    PLINKLIST   pllWindows = (PLINKLIST)pvllWindows;

    // check current focus
    HWND        hwndFocus = WinQueryFocus(HWND_DESKTOP);

    PLISTNODE   pNode = lstNodeFromItem(pllWindows, (PVOID)hwndFocus);

    BOOL fRestart = FALSE;

    while (pNode)
    {
        CHAR    szClass[100];

        // next focus in node
        pNode = pNode->pNext;

        if (    (!pNode)        // no next, or not found:
             && (!fRestart)     // avoid infinite looping if bad list
           )
        {
            pNode = lstQueryFirstNode(pllWindows);
            fRestart = TRUE;
        }

        if (pNode)
        {
            // check if this is a focusable control
            if (WinQueryClassName((HWND)pNode->pItemData,
                                  sizeof(szClass),
                                  szClass))
            {
                if (    (strcmp(szClass, "#5"))    // not static
                   )
                    break;
                // else: go for next then
            }
        }
    }

    if (pNode)
    {
        WinSetFocus(HWND_DESKTOP,
                    (HWND)pNode->pItemData);
    }
}

/*
 *@@ dlghProcessMnemonic:
 *      finds the control which matches usch
 *      and gives it the focus. If this is a
 *      static control, the next control in the
 *      list is given focus instead. (Standard
 *      dialog behavior.)
 *
 *      Pass in usch from WM_CHAR. It is assumed
 *      that the caller has already tested for
 *      the "alt" key to be depressed.
 *
 *@@added V0.9.9 (2001-03-17) [umoeller]
 */

HWND dlghProcessMnemonic(PVOID pvllWindows,
                         USHORT usch)
{
    PLINKLIST   pllWindows = (PLINKLIST)pvllWindows;

    HWND hwndFound = NULLHANDLE;
    PLISTNODE pNode = lstQueryFirstNode(pllWindows);
    CHAR szClass[100];

    while (pNode)
    {
        HWND hwnd = (HWND)pNode->pItemData;

        if (WinSendMsg(hwnd,
                       WM_MATCHMNEMONIC,
                       (MPARAM)usch,
                       0))
        {
            // according to the docs, only buttons and static
            // return TRUE to that msg;
            // if this is a static, give focus to the next
            // control

            // _Pmpf((__FUNCTION__ ": hwnd 0x%lX", hwnd));

            // check if this is a focusable control
            if (WinQueryClassName(hwnd,
                                  sizeof(szClass),
                                  szClass))
            {
                if (!strcmp(szClass, "#3"))
                    // it's a button: click it
                    WinSendMsg(hwnd, BM_CLICK, (MPARAM)TRUE, 0);
                else if (!strcmp(szClass, "#5"))
                {
                    // it's a static: give focus to following control
                    pNode = pNode->pNext;
                    if (pNode)
                        WinSetFocus(HWND_DESKTOP, (HWND)pNode->pItemData);
                }
            }
            else
                // any other control (are there any?): give them focus
                WinSetFocus(HWND_DESKTOP, hwnd);

            // in any case, stop
            hwndFound = hwnd;
            break;
        }

        pNode = pNode->pNext;
    }

    return hwndFound;
}

/*
 *@@ dlghEnter:
 *      presses the first button with BS_DEFAULT.
 */

BOOL dlghEnter(PVOID pvllWindows)
{
    PLINKLIST   pllWindows = (PLINKLIST)pvllWindows;

    PLISTNODE pNode = lstQueryFirstNode(pllWindows);
    CHAR szClass[100];
    while (pNode)
    {
        HWND hwnd = (HWND)pNode->pItemData;
        if (WinQueryClassName(hwnd,
                              sizeof(szClass),
                              szClass))
        {
            if (!strcmp(szClass, "#3"))    // button
            {
                // _Pmpf((__FUNCTION__ ": found button"));
                if (    (WinQueryWindowULong(hwnd, QWL_STYLE) & (BS_PUSHBUTTON | BS_DEFAULT))
                     == (BS_PUSHBUTTON | BS_DEFAULT)
                   )
                {
                    // _Pmpf(("   is default!"));
                    WinPostMsg(hwnd,
                               BM_CLICK,
                               (MPARAM)TRUE,        // upclick
                               0);
                    return TRUE;
                }
            }
        }

        pNode = pNode->pNext;
    }

    return FALSE;
}


