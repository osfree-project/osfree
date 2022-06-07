/*****************************************************************************
 * lvm_ctls.c                                                                *
 *                                                                           *
 * Copyright (C) 2011-2019 Alexander Taylor.                                 *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify it *
 *   under the terms of the GNU General Public License as published by the   *
 *   Free Software Foundation; either version 2 of the License, or (at your  *
 *   option) any later version.                                              *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License along *
 *   with this program; if not, write to the Free Software Foundation, Inc., *
 *   59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                 *
 *****************************************************************************/

#include "lvm_ctls.h"

#ifndef DebugBox
#define DebugBox( errmsg ) \
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, (PSZ)errmsg, "Debug", 0, MB_MOVEABLE | MB_OK | MB_ERROR )
#endif

// Convert ratio: determine x where a/b == x/y )
#define RATIO_CONV( a, b, y )  (( a * y ) / b )

// ----------------------------------------------------------------------------
// PRIVATE DEFINITIONS FOR MODIFYING CONTROL BEHAVIOUR
//

// Uncomment to use a solid-style selection border on partition-view controls.
//
#define SOLID_PV_BORDER


// Uncomment to indicate disk list focus by drawing a dotted border around the
// selected disk (and always drawing the current-partition selection border).
// If this is NOT enabled, focus will be indicated by drawing or not drawing
// the selection border around the current partition.
#define FOCUS_BORDER


// Uncomment to have the disk/partition controls automatically activate the
// context-menu border on context menu events (normally this is the
// responsibility of the owner, i.e. the application proper).
//
//#define AUTO_CONTEXT


// ----------------------------------------------------------------------------
// PRIVATE CONSTANTS
//

// Custom colour table indices
#define CLRIDX_PRIMARY      16      // primary partition colour
#define CLRIDX_LOGICAL      17      // logical partition colour
#define CLRIDX_LETTER       18      // drive-letter background colour
#define CLRIDX_BOOTMGR      19      // Boot Manager partition colour


// ----------------------------------------------------------------------------
// PRIVATE TYPEDEFS
//

// Internal state data for the LVMDiskList control
typedef struct _DiskList_Private {
    ULONG       id;                     // our control ID
    USHORT      usDisks,                // number of disk controls
                usDiskSel,              // currently-selected disk index
                usPartSel,              // currently-selected partition index
                fs;                     // current flags
    HWND        hwndScroll,             // scrollbar handle
                *pDV;                   // array of disk window handles
    RECTL       rclScroll;              // bounds of the scrollbar
    ULONG       ulDisplayable,          // number of disks that can be displayed
                ulScrollPos;            // # of topmost visible disk
    LONG        lDiskWidth,             // height of each disk control (in pels)
                lDiskHeight,            // width of each disk control (in pels)
                lDiskTop,               // top y of first visible disk (in pels)
                lDiskBottom;            // bottom y of last disk (in pels)
    PSZ         pszTitle;               // optional title string
    PDVCTLDATA  pDisks;                 // array of disk control data
} DISKLISTPRIVATE, *PDISKLISTPRIVATE;


// Internal state data for the LVMDiskView control
typedef struct _DiskView_Private {
    DVCTLDATA   ctldata;                   // control-data structure
    ULONG       id;                        // our control ID
    USHORT      usPartitions,              // number of partition controls
                usPartSel,                 // index of selected partition
                fsEmphasis,                // current emphasis flags
                fsStyle;                   // current style flags
    HPOINTER    hptrIcon;                  // handle of the disk icon
    HWND        *pPV;                      // array of partition window handles
    RECTL       rclIcon,                   // bounds of the disk icon
                rclText,                   // bounds of the disk name/size area
                rclGraphic,                // bounds of the disk graphic
                *prclPV;                   // array of partition view bounds
    PPVCTLDATA  pPartitions;               // array of partition control data
} DVPRIVATE, *PDVPRIVATE;


// Internal state data for the LvmPartitionView control
typedef struct _PartitionView_Private {
    PVCTLDATA   ctldata;                // control-data structure
    ULONG       id;                     // our control ID
    USHORT      fsEmphasis;             // current emphasis flags
    LONG        lDPI;                   // current font DPI
} PVPRIVATE, *PPVPRIVATE;


// Internal state data for the LVMVolumeInfo control
typedef struct _VolInfo_Private {
    VICTLDATA ctldata;                  // control-data structure
    ULONG     id;                       // our control ID
    LONG      lDPI,                     // current font DPI
              lIcon;                    // current icon resolution
    HPOINTER  hptrIcon;                 // handle of icon data
    RECTL     rclIcon,                  // position of the icon
              rclDivider;               // position of the divider line
    HWND      hwndLetter,               // drive letter text control
              hwndName,                 // volume name text control
              hwndType,                 // volume type text control
              hwndFSL,    hwndFSV,      // filesystem label and value controls
              hwndDevL,   hwndDevV,     // device label and value controls
              hwndSizeL,  hwndSizeV,    // size label and value controls
              hwndCnr;                  // partition container
} VOLINFOPRIVATE, *PVOLINFOPRIVATE;



// ----------------------------------------------------------------------------
// PRIVATE FUNCTION PROTOTYPES
//

void             DL_ArrangeDisks( PDISKLISTPRIVATE pCtl, LONG lTop );
void             DL_ClearDiskFlags_All( USHORT usFlags, PDISKLISTPRIVATE pCtl );
void             DL_ClearDiskFlags_Other( USHORT usIndex, USHORT usFlags, PDISKLISTPRIVATE pCtl );
USHORT           DL_GetDiskIndex( HWND hwndDisk, PDISKLISTPRIVATE pCtl );
void             DL_SetSelectedDisk( HWND hwnd, PDISKLISTPRIVATE pCtl, BOOL fFocus );
void             DL_Size( HPS hps, HWND hwnd, PDISKLISTPRIVATE pCtl );
MRESULT EXPENTRY DLDisplayProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

void             DV_ClearPartitionFlags_All( USHORT usFlags, PDVPRIVATE pCtl );
void             DV_ClearPartitionFlags_Other( USHORT ulIndex, USHORT usFlags, PDVPRIVATE pCtl );
USHORT           DV_GetPartitionIndex( HWND hwndPart, PDVPRIVATE pCtl );
void             DV_LayoutDisk( HPS hps, PDVPRIVATE pCtl );
void             DV_Size( HPS hps, HWND hwnd, PDVPRIVATE pCtl );
MRESULT EXPENTRY DVDisplayProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

void             PV_DrawText( HPS hps, HWND hwnd, RECTL rcl, PPVPRIVATE pCtl );
MRESULT EXPENTRY PVDisplayProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

void             VI_Size( HWND hwnd, PVOLINFOPRIVATE pCtl );
MRESULT EXPENTRY VIDisplayProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );


// ***************************************************************************
// PUBLIC FUNCTIONS
// ***************************************************************************


/* ------------------------------------------------------------------------- *
 * PVRegisterClass                                                           *
 *                                                                           *
 * Registers the LVMPartitionView control class.                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HAB hab: Application's anchor-block handle.                             *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   Return code from WinRegisterClass.                                      *
 * ------------------------------------------------------------------------- */
BOOL PVRegisterClass( HAB hab )
{
    return WinRegisterClass( hab, WC_PARTITIONVIEW, PVDisplayProc,
                             CS_SIZEREDRAW, sizeof( PVOID ));
}


/* ------------------------------------------------------------------------- *
 * DVRegisterClass                                                           *
 *                                                                           *
 * Registers the LVMDiskView control class.                                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HAB hab: Application's anchor-block handle.                             *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   Return code from WinRegisterClass.                                      *
 * ------------------------------------------------------------------------- */
BOOL DVRegisterClass( HAB hab )
{
    PVRegisterClass( hab );
    return WinRegisterClass( hab, WC_DISKVIEW, DVDisplayProc,
                             CS_SIZEREDRAW, sizeof( PVOID ));
}


/* ------------------------------------------------------------------------- *
 * DLRegisterClass                                                           *
 *                                                                           *
 * Registers the LVMDiskList control class.                                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HAB hab: Application's anchor-block handle.                             *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   Return code from WinRegisterClass.                                      *
 * ------------------------------------------------------------------------- */
BOOL DLRegisterClass( HAB hab )
{
    DVRegisterClass( hab );
    return WinRegisterClass( hab, WC_LVMDISKS,
                             DLDisplayProc, CS_SIZEREDRAW, sizeof( PVOID ));
}


/* ------------------------------------------------------------------------- *
 * VIRegisterClass                                                           *
 *                                                                           *
 * Registers the LVMVolumeInfo control class.                                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HAB hab: Application's anchor-block handle.                             *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   Return code from WinRegisterClass.                                      *
 * ------------------------------------------------------------------------- */
BOOL VIRegisterClass( HAB hab )
{
    return WinRegisterClass( hab, WC_VOLUMEINFO,
                             VIDisplayProc, CS_SIZEREDRAW, sizeof( PVOID ));
}


/* ------------------------------------------------------------------------- *
 * GetBoldFontPP                                                             *
 *                                                                           *
 * Given the name of a font in presentation-parameter format (e.g. "8.Helv"),*
 * replace it with the name of its bold version (e.g. "8.Helv Bold").  The   *
 * same buffer is used for input and output, and should be FACESIZE+4 bytes. *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS hps    : Handle of current presentation space.                      *
 *   PSZ pszFont: Font name buffer.  On input it contains the face name of   *
 *                the font to be emboldened, in "size.name" format.  On      *
 *                output it will contain the name of the bold font.          *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if a suitable bitmap font was found; FALSE otherwise.              *
 * ------------------------------------------------------------------------- */
BOOL GetBoldFontPP( HPS hps, PSZ pszFont )
{
    PFONTMETRICS pfm;               // array of FONTMETRICS objects
    LONG         i,                 // loop index
                 cFonts    = 0,     // number of fonts found
                 cCount    = 0;     // number of fonts to return
    BOOL         fFound    = FALSE; // did we find a suitable bitmap font?
    CHAR         szTest[ FACESIZE ] = {0};
    PSZ          pszFontName;
    ULONG        ulPts;

    // Separate the point size and face name
    pszFontName = strchr( pszFont, '.') + 1;
    if ( sscanf( pszFont, "%d.", &ulPts ) != 1 )
        ulPts = 10;

    if (( strstr( pszFontName, " Bold")) || ( strstr( pszFontName, " bold")))
        // Looks like font is already bold, so do nothing
        return TRUE;

    /* Now we append " Bold" to the font name and search for matching fonts.
     * (This is kind of quick-and-dirty, but it should be sufficient for our
     * purposes here.)
     */
    strcpy( szTest, pszFontName );
    strncat( szTest, " Bold", FACESIZE-1 );
    cFonts = GpiQueryFonts( hps, QF_PUBLIC, szTest,
                            &cCount, sizeof(FONTMETRICS), NULL );
    if ( cFonts < 1 ) return FALSE;
    if (( pfm = (PFONTMETRICS) calloc( cFonts, sizeof( FONTMETRICS ))) == NULL )
        return FALSE;
    GpiQueryFonts( hps, QF_PUBLIC, szTest,
                   &cFonts, sizeof(FONTMETRICS), pfm );

    /* We have a match if we find either (a) a bitmap font at the requested
     * point size, or (b) an outline font.
     */
    for ( i = 0; i < cFonts; i++ ) {
        if (( pfm[i].fsDefn & FM_DEFN_OUTLINE ) ||
            ( pfm[i].sNominalPointSize == ( ulPts * 10 )))
        {
            fFound = TRUE;
            break;
        }
    }
    free( pfm );

    if ( fFound )
        strncpy( pszFontName, szTest, FACESIZE-1 );

    return ( fFound );
}


/* ------------------------------------------------------------------------- *
 * GetImageFont                                                              *
 *                                                                           *
 * Look for a bitmap (a.k.a. image or raster) font under the specified font  *
 * name.  This is necessary because PM sees every distinct point size of a   *
 * bitmap font as a separate font (albeit with a shared family name).        *
 *                                                                           *
 * This function specifically looks for bitmap rather than outline (vector)  *
 * fonts; if no bitmap versions of the requested font name are found, we     *
 * return FALSE.  (The caller will presumably try to treat pszFontFace as an *
 * outline font in such a case; whether it wants to verify its existence or  *
 * not before doing so is its own problem.)                                  *
 *                                                                           *
 * If there are any matching bitmap fonts found, do the following:           *
 * - If possible, return the largest font/size pair which is equal to or     *
 *   smaller than the requested point size.  (An exact match should take     *
 *   precedence over all others.)                                            *
 * - If all the fonts found are larger than the requested point size, then   *
 *   return the smallest one available.                                      *
 *                                                                           *
 * If a match is found, the FATTRS structure pointed to by pfAttrs will be   *
 * populated with the appropriate font information, and the function will    *
 * return TRUE.  Otherwise, FALSE will be returned (and the contents of      *
 * pfAttrs will not be altered).                                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS     hps        : Handle of the current presentation space.          *
 *   PSZ     pszFontFace: Face name of the font being requested.             *
 *   PFATTRS pfAttrs    : Address of a FATTRS object to receive the results. *
 *   LONG    lCY        : The requested maximum point size (multiple of 10). *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if a suitable bitmap font was found; FALSE otherwise.              *
 * ------------------------------------------------------------------------- */
BOOL GetImageFont( HPS hps, PSZ pszFontFace, PFATTRS pfAttrs, LONG lCY )
{
    PFONTMETRICS pfm;               // array of FONTMETRICS objects
    LONG         i,                 // loop index
                 lSmIdx    = -1,    // index of smallest available font
                 lLargest  = 0,     // point size of largest font
                 cFonts    = 0,     // number of fonts found
                 cCount    = 0;     // number of fonts to return
    BOOL         fOutline  = FALSE, // does font include an outline version?
                 fFound    = FALSE; // did we find a suitable bitmap font?


    // Find the specific fonts which match the given face name
    cFonts = GpiQueryFonts( hps, QF_PUBLIC, pszFontFace,
                            &cCount, sizeof(FONTMETRICS), NULL );
    if ( cFonts < 1 ) return FALSE;
    if (( pfm = (PFONTMETRICS) calloc( cFonts, sizeof( FONTMETRICS ))) == NULL )
        return FALSE;
    GpiQueryFonts( hps, QF_PUBLIC, pszFontFace,
                   &cFonts, sizeof(FONTMETRICS), pfm );

    // Look for the largest bitmap font that fits within the requested height
    for ( i = 0; i < cFonts; i++ ) {

        if ( !( pfm[i].fsDefn & FM_DEFN_OUTLINE )) {
            if (( lSmIdx < 0 ) || ( pfm[i].sNominalPointSize < pfm[lSmIdx].sNominalPointSize ))
                lSmIdx = i;
            if (( pfm[i].sNominalPointSize <= lCY ) && ( pfm[i].sNominalPointSize >= lLargest )) {
                lLargest = pfm[i].sNominalPointSize;
                fFound   = TRUE;
                strcpy( pfAttrs->szFacename, pfm[i].szFacename );
                pfAttrs->lMatch          = pfm[i].lMatch;
                pfAttrs->idRegistry      = pfm[i].idRegistry;
                pfAttrs->lMaxBaselineExt = pfm[i].lMaxBaselineExt;
                pfAttrs->lAveCharWidth   = pfm[i].lAveCharWidth;
            }
        } else fOutline = TRUE;
    }

    // If nothing fits the requested size, use the smallest point size available
    if ( !fFound && !fOutline && lSmIdx >= 0 ) {
        fFound = TRUE;
        strcpy( pfAttrs->szFacename, pfm[lSmIdx].szFacename );
        pfAttrs->lMatch          = pfm[lSmIdx].lMatch;
        pfAttrs->idRegistry      = pfm[lSmIdx].idRegistry;
        pfAttrs->lMaxBaselineExt = pfm[lSmIdx].lMaxBaselineExt;
        pfAttrs->lAveCharWidth   = pfm[lSmIdx].lAveCharWidth;
    }

    free( pfm );
    return ( fFound );
}


/* ------------------------------------------------------------------------- *
 * SetFontFromPP                                                             *
 *                                                                           *
 * Sets the current GPI font from the current font presentation parameter.   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS  hps : Current presentation space handle.                           *
 *   HWND hwnd: Window handle of control.                                    *
 *   LONG lDPI: Current screen DPI setting.                                  *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void SetFontFromPP( HPS hps, HWND hwnd, LONG lDPI )
{
    FATTRS  fa;                     // required font attributes
    SIZEF   sfCell;                 // character cell size
    CHAR    szFont[ FACESIZE+4 ];   // current font presentation parameter
    PSZ     pszFontName;            // requested font family
    ULONG   ulPts;                  // requested font size
    double  dScale;                 // point scaling factor
    APIRET  rc;


    // Define the requested font attributes
    memset( &fa, 0, sizeof( FATTRS ));
    fa.usRecordLength = sizeof( FATTRS );
    fa.fsType         = FATTR_TYPE_MBCS;
    fa.fsFontUse      = FATTR_FONTUSE_NOMIX;
    rc = WinQueryPresParam( hwnd, PP_FONTNAMESIZE, 0, NULL,
                            sizeof( szFont ), szFont, QPF_NOINHERIT );
    if ( !rc )
        return;

    pszFontName = strchr( szFont, '.') + 1;
    strcpy( fa.szFacename, pszFontName );
    if ( sscanf( szFont, "%d.", &ulPts ) != 1 )
        ulPts = 10;

    /* If this is a bitmap font, look up the proper name+size; otherwise,
     * we will scale it to the requested point size according to the DPI.
     */
    if ( ! GetImageFont( hps, pszFontName, &fa, ulPts*10 )) {
        dScale = ( (double) lDPI / 72 );
        ulPts = ulPts * dScale;
        sfCell.cy = MAKEFIXED( ulPts, 0 );
        sfCell.cx = sfCell.cy;
        GpiSetCharBox( hps, &sfCell );
    }

    // Make the font active
    if (( GpiCreateLogFont( hps, NULL, 1L, &fa )) == GPI_ERROR )
        return;
    GpiSetCharSet( hps, 1L );
}


/* ------------------------------------------------------------------------- *
 * GetCurrentDPI                                                             *
 *                                                                           *
 * Queries the current vertical font resolution (a.k.a. DPI).                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: Window handle of control.                                    *
 *                                                                           *
 * RETURNS: LONG                                                             *
 *   Current screen DPI setting.                                             *
 * ------------------------------------------------------------------------- */
LONG GetCurrentDPI( HWND hwnd )
{
    HDC  hdc;           // device-context handle
    LONG lCap,          // value from DevQueryCaps
         lDPI;          // returned DPI value

    hdc = WinOpenWindowDC( hwnd );
    if ( DevQueryCaps( hdc, CAPS_VERTICAL_FONT_RES, 1, &lCap ))
        lDPI = lCap;
    if ( !lDPI )
        lDPI = 96;

    return lDPI;
}


// ***************************************************************************
// PRIVATE FUNCTIONS RELATED TO THE WC_PARTITIONVIEW CONTROL
// ***************************************************************************


/* ------------------------------------------------------------------------- *
 * Window procedure for the PartitionView control.                           *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY PVDisplayProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PWNDPARAMS    pwp;              // pointer to window parameters
    PPVPRIVATE    pPrivate;         // pointer to private control data
    HPS           hps;              // control's presentation space
    RECTL         rcl;              // control's window area
    POINTL        ptl;              // current drawing position
    LONG          alClrs[ 4 ],      // array of values to add to the color table
                  lFG, lBG, lBdr;   // current drawing colours
    HWND          hwndOwner;        // handle of our owner (presumably a DiskView control)
    USHORT        fsMask;           // options mask


    switch( msg ) {

        case WM_BUTTON1CLICK:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            pPrivate->fsEmphasis |= LPV_FS_SELECTED;
            pPrivate->fsEmphasis &= ~LPV_FS_CONTEXT;
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
            WinSendMsg( hwndOwner, WM_CONTROL,
                        MPFROM2SHORT( pPrivate->id, LPN_SELECT ),
                        MPFROMP( hwnd ));
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT) TRUE;


        case WM_CONTEXTMENU:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
#ifdef AUTO_CONTEXT
            pPrivate->fsEmphasis |= LPV_FS_CONTEXT;
#endif
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
            WinSendMsg( hwndOwner, WM_CONTROL,
                        MPFROM2SHORT( pPrivate->id, LPN_CONTEXTMENU ),
                        MPFROMP( hwnd ));
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT) TRUE;


        case WM_CREATE:
            // initialize the private control data structure
            if ( !( pPrivate = (PPVPRIVATE) calloc( 1, sizeof( PVPRIVATE ))))
                return (MRESULT) TRUE;
            WinSetWindowPtr( hwnd, 0, pPrivate );

            if ( mp1 ) {
                memcpy( &(pPrivate->ctldata), mp1, ((PPVCTLDATA)mp1)->cb );
            }
            pPrivate->id = ((PCREATESTRUCT)mp2)->id;

            pPrivate->lDPI = GetCurrentDPI( hwnd );
            return (MRESULT) FALSE;


        case WM_DESTROY:
            if (( pPrivate = WinQueryWindowPtr( hwnd, 0 )) != NULL ) {
                // free any allocated fields of pPrivate
                free( pPrivate );
            }
            break;


        case WM_PAINT:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            GpiSetLineType( hps, LINETYPE_DEFAULT );
            WinQueryWindowRect( hwnd, &rcl );

            // Define some new colours
            alClrs[ CLRIDX_PRIMARY - 16 ] = 0x00203080;     // medium blue
            alClrs[ CLRIDX_LOGICAL - 16 ] = 0x00307080;     // medium cyan
            alClrs[ CLRIDX_LETTER  - 16 ] = 0x00FFFFD0;     // cream/pale yellow
            alClrs[ CLRIDX_BOOTMGR - 16 ] = 0x00703080;     // dark purple
            GpiCreateLogColorTable( hps, 0, LCOLF_CONSECRGB, 16, 4, alClrs );

            // Set the colours according to the partition type
            if ( pPrivate->ctldata.fDisable ) {
                lBG  = CLR_PALEGRAY;
                lFG  = CLR_BLACK;
                lBdr = CLR_DARKGRAY;
            }
            else switch ( pPrivate->ctldata.bType ) {
                case LPV_TYPE_FREE:     // freespace (black-on-white)
                    lBG  = CLR_WHITE;
                    lFG  = CLR_BLACK;
                    lBdr = CLR_PALEGRAY;
                    break;
                case LPV_TYPE_PRIMARY:  // primary partition (white-on-blue)
                    lBG  = CLRIDX_PRIMARY;
                    lFG  = CLR_WHITE;
                    lBdr = CLR_DARKBLUE;
                    break;
                case LPV_TYPE_LOGICAL:  // logical partition (white-on-cyan)
                    lBG  = CLRIDX_LOGICAL;
                    lFG  = CLR_WHITE;
                    lBdr = CLR_DARKGRAY;
                    break;
                case LPV_TYPE_UNUSABLE: // unusable (light-on-dark-grey)
                    lBG  = CLR_DARKGRAY;
                    lFG  = CLR_PALEGRAY;
                    lBdr = CLR_BLACK;
                    break;
                case LPV_TYPE_BOOTMGR:  // Boot Manager primary (white-on-red)
                    lBG  = CLRIDX_BOOTMGR;
                    lFG  = CLR_WHITE;
                    lBdr = CLR_DARKPINK;
                    break;
                default:                // error (should not occur)
                    lBG  = CLR_BLACK;
                    lFG  = CLR_RED;
                    lBdr = CLR_DARKGRAY;
                    break;
            }
            GpiSetBackColor( hps, lBG );

            // Draw the outer border
            GpiSetColor( hps, lBdr );
            ptl.x = 0;
            ptl.y = 0;
            GpiMove( hps, &ptl );
            ptl.x = rcl.xRight - 1;
            ptl.y = rcl.yTop - 1;
            GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );

            // Paint the background according to the current state
            rcl.xLeft   += 1;
            rcl.yBottom += 1;
            rcl.xRight  -= 1;
            rcl.yTop    -= 1;
            WinFillRect( hps, &rcl, lBG );
            if ( pPrivate->fsEmphasis & LPV_FS_ACTIVE ) {
                HRGN hrgn = GpiCreateRegion( hps, 1, &rcl );
                GpiSetColor( hps, CLR_PALEGRAY );
                GpiSetPattern( hps, PATSYM_DIAG1 );
                GpiPaintRegion( hps, hrgn );
                GpiSetPattern( hps, PATSYM_SOLID );
                GpiDestroyRegion( hps, hrgn );
            }

            // Paint the text
            rcl.xLeft   += 1;
            rcl.yBottom += 1;
            rcl.xRight  -= 1;
            rcl.yTop    -= 1;
            GpiSetColor( hps, lFG );
            PV_DrawText( hps, hwnd, rcl, pPrivate );

            // Draw the context menu indicator, if applicable
            if ( pPrivate->fsEmphasis & LPV_FS_CONTEXT ) {
                GpiSetColor( hps, lFG );
                GpiSetLineType( hps, LINETYPE_DOT );
                ptl.x = rcl.xLeft + 1;
                ptl.y = rcl.yBottom + 1;
                GpiMove( hps, &ptl );
                ptl.x = rcl.xRight - 2;
                ptl.y = rcl.yTop - 2;
                GpiBox( hps, DRO_OUTLINE, &ptl, 20, 20 );
            }
            // Draw the selection border, if applicable
            if ( pPrivate->fsEmphasis & LPV_FS_SELECTED ) {
                GpiSetColor( hps, lFG );
#ifdef SOLID_PV_BORDER
                GpiSetLineType( hps, LINETYPE_DEFAULT );
                ptl.x = rcl.xLeft - 1;
                ptl.y = rcl.yBottom - 1;
                GpiMove( hps, &ptl );
                ptl.x = rcl.xRight;
                ptl.y = rcl.yTop;
                GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
#else
                GpiSetLineType( hps, LINETYPE_ALTERNATE );
                ptl.x = rcl.xLeft;
                ptl.y = rcl.yBottom;
                GpiMove( hps, &ptl );
                ptl.x = rcl.xRight - 1;
                ptl.y = rcl.yTop - 1;
                GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
#endif
            }

            // Finish up
            WinEndPaint( hps );
            return (MRESULT) 0;


        case WM_PRESPARAMCHANGED:
            // We only use PP_FONTNAMESIZE
            switch ( (ULONG) mp1 ) {
                case PP_FONTNAMESIZE:
                    WinInvalidateRect(hwnd, NULL, FALSE);
                    break;
                default: break;
            }
            break;


        case WM_QUERYWINDOWPARAMS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            pwp = (PWNDPARAMS) mp1;

            // Only support WPM_CTLDATA
            if ( pwp && ( pwp->fsStatus & WPM_CTLDATA )) {
                pwp->cbCtlData = sizeof( PVCTLDATA );
                memcpy( pwp->pCtlData, &(pPrivate->ctldata), pwp->cbCtlData );
                return (MRESULT) TRUE;
            }
            return (MRESULT) FALSE;


        case WM_SETWINDOWPARAMS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            pwp = (PWNDPARAMS) mp1;
            if ( !pwp ) return (MRESULT) 0;

            // Only support WPM_CTLDATA
            if (( pwp->fsStatus & WPM_CTLDATA ) &&
                ( pwp->cbCtlData == sizeof( PVCTLDATA )) && pwp->pCtlData )
            {
                memcpy( &(pPrivate->ctldata), pwp->pCtlData, pwp->cbCtlData );
            }
            return (MRESULT) 0;


        // --------------------------------------------------------------------
        // Custom messages defined for this control
        //

        /* .................................................................. *
         * LPM_SETEMPHASIS                                                    *
         *  - mp1 (BOOL)  : TRUE to set given emphasis type, FALSE to unset it*
         *  - mp2 (USHORT): Mask of emphasis types to set (LPV_FS_*)          *
         *  Returns (BOOL) TRUE on success, FALSE on failure                  *
         * .................................................................. */
        case LPM_SETEMPHASIS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) FALSE;

            fsMask = (USHORT) mp2;
            if ( (BOOL) mp1 == TRUE ) {
                pPrivate->fsEmphasis |= fsMask;
                // Tell our owner we were selected
                if ( fsMask & LPV_FS_SELECTED ) {
                    hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
                    WinSendMsg( hwndOwner, WM_CONTROL,
                                MPFROM2SHORT( pPrivate->id, LPN_SELECT ),
                                MPFROMP( hwnd ));
                }
            }
            else
                pPrivate->fsEmphasis &= ~fsMask;
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT) TRUE;


        /* .................................................................. *
         * LPM_GETEMPHASIS                                                    *
         * - mp1: Unused, should be 0.                                        *
         * - mp2: Unused, should be 0.                                        *
         * Returns (USHORT) the emphasis flags active for the partition.      *
         * .................................................................. */
        case LPM_GETEMPHASIS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) FALSE;
            return (MRESULT) pPrivate->fsEmphasis;


    }

    return ( WinDefWindowProc( hwnd, msg, mp1, mp2 ));
}


/* ------------------------------------------------------------------------- *
 * PV_DrawText                                                               *
 *                                                                           *
 * Draw the text contents of a PartitionView control.                        *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS        hps : Current presentation space handle.                     *
 *   HWND       hwnd: Window handle of control.                              *
 *   RECTL      rcl : Overall bounding rectangle for all text.               *
 *   PPVPRIVATE pCtl: Pointer to control's internal state data.              *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void PV_DrawText( HPS hps, HWND hwnd, RECTL rcl, PPVPRIVATE pCtl )
{
    FONTMETRICS   fm;                   // current font metrics
    CHAR          szLtr[ 3 ];           // drive letter string
    PCH           pchText;              // pointer to current output text
    ULONG         cb,                   // output text length
                  cchFit;               // number of displayable characters
    LONG          lEllipsis,            // width of the string "..."
                  lMaxWidth;            // maximum width of text in pels
    POINTL        ptl,                                  // drawing position
                  aptl[ PARTITION_NAME_SIZE+1 ] = {0};  // character positions
    RECTL         rclVol;                               // volume-letter area

    if ( !pCtl ) return;        // Oops, no control data - better give up now

    // Set the font according to the current pres-params
    SetFontFromPP( hps, hwnd, pCtl->lDPI );
    GpiSetTextAlignment( hps, TA_LEFT, TA_BASE );

    // Now get the font metrics so we can use them for positioning calculations
    GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm );

    // Draw the name string, or as much of it as will fit within our rectangle
    cb = strlen( pCtl->ctldata.szName );
    if ( cb ) {
        ptl.x = rcl.xLeft + ( fm.lAveCharWidth / 2 );
        ptl.y = rcl.yTop - fm.lMaxAscender - 1;

        // measure the ellipsis string in case we need it
        GpiQueryTextBox( hps, 3, (PCH) "...", TXTBOX_COUNT, aptl );
        lEllipsis = aptl[ TXTBOX_TOPRIGHT ].x - aptl[ TXTBOX_TOPLEFT ].x;

        // check the amount of space required for the whole text
        GpiQueryCharStringPosAt( hps, &ptl, 0, cb,
                                 (PCH) pCtl->ctldata.szName, NULL, aptl );

        // if the text is too long, truncate it and then append "..."
        if ( aptl[cb].x > rcl.xRight ) {
            lMaxWidth = rcl.xRight - ptl.x - lEllipsis - 1;
            cchFit = cb - 1;
            while ( cchFit && ( aptl[cchFit-1].x >= lMaxWidth )) cchFit--;
            cb = cchFit + 3;
            pchText = (PCH) calloc( cb + 1, 1 );
            if ( cchFit ) strncpy( pchText, pCtl->ctldata.szName, cchFit );
            strncat( pchText, "...", cb );
        }
        else
            pchText = (PCH) strdup( pCtl->ctldata.szName );
        if ( !pchText ) return;

        GpiCharStringPosAt( hps, &ptl, &rcl, CHS_CLIP, cb, pchText, NULL );
        free( pchText );
    }

    // If there's no volume drive letter, then we're done
    if ( !pCtl->ctldata.cLetter )
        return;

    // Draw a pretty box for the volume drive letter
    rclVol.xLeft   = rcl.xLeft + ( fm.lAveCharWidth / 2 );
    rclVol.yTop    = ptl.y - ( fm.lMaxDescender * 2 ) - fm.lExternalLeading - 1;
    rclVol.yBottom = rclVol.yTop - fm.lMaxBaselineExt - fm.lExternalLeading - 1;
    if ( rclVol.yBottom < rcl.yBottom ) return;
    rclVol.xRight = rclVol.xLeft + fm.lEmInc + fm.lAveCharWidth;
    if ( rclVol.xRight > rcl.xRight ) return;

    WinFillRect( hps, &rclVol, CLRIDX_LETTER );
    GpiSetColor( hps, CLR_BLACK );
    ptl.x = rclVol.xLeft;
    ptl.y = rclVol.yBottom;
    GpiMove( hps, &ptl );
    ptl.x = rclVol.xRight;
    ptl.y = rclVol.yTop;
    GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );

    // Now draw the drive letter string
    ptl.x = rclVol.xLeft + (( rclVol.xRight - rclVol.xLeft ) / 2 );
    ptl.y = rclVol.yBottom + 1 + fm.lMaxDescender;
    GpiSetTextAlignment( hps, TA_CENTER, TA_BASE );
    sprintf( szLtr, "%c:", pCtl->ctldata.cLetter );
    pchText = (PCH) szLtr;
    cb = strlen( szLtr );
    GpiCharStringPosAt( hps, &ptl, &rclVol, CHS_CLIP, cb, pchText, NULL );

}



// ***************************************************************************
// Private functions related to the WC_DISKVIEW control
// ***************************************************************************


/* ------------------------------------------------------------------------- *
 * Window procedure for the DiskView control.                                *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY DVDisplayProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PWNDPARAMS  pwp;            // pointer to window parameters
    WNDPARAMS   wp = {0};       // window-params for query partition data
    PDVPRIVATE  pPrivate;       // pointer to private control data
    PVCTLDATA   partdata = {0}; // partition-view control data structure
    DISKNOTIFY  notify;         // notification message structure
    FONTMETRICS fm;             // current font metrics
    HPS         hps;            // control's presentation space
    RECTL       rcl;            // control's window area
    POINTL      ptl;            // current drawing position
    ULONG       cb;
    USHORT      i;
    USHORT      usPartIdx,      // current partition index
                fsMask;         // flags mask (used for various purposes)
    BOOL        fSet;           // emphasis set/clear flag
    HWND        hwndOwner,      // handle of our owner
                hwndPart;       // handle of a partition control
    CHAR        chKey;          // WM_CHAR value

    switch( msg ) {

        case WM_BUTTON1CLICK:
            // The disk itself (not a child partition) has been selected
            pPrivate = WinQueryWindowPtr( hwnd, 0 );

            // Clear all disk and partition context-menu flags
            pPrivate->fsEmphasis &= ~LDV_FS_CONTEXT;
            DV_ClearPartitionFlags_All( LPV_FS_CONTEXT, pPrivate );

            /* Set selected emphasis on the disk and first partition, but only
             * if the disk was not already selected.
             */
            notify.hwndPartition = NULLHANDLE;
            notify.usPartition   = 0;
            if ( !( pPrivate->fsEmphasis & LDV_FS_SELECTED )) {
                pPrivate->fsEmphasis |= LDV_FS_SELECTED;
                if ( pPrivate->pPV ) {
                    WinSendMsg( pPrivate->pPV[ 0 ], LPM_SETEMPHASIS,
                                MPFROMSHORT( TRUE ),
                                MPFROMSHORT( LPV_FS_SELECTED ));
                    notify.hwndPartition = pPrivate->pPV[ 0 ];
                    notify.usPartition   = 0;
                }
            }
            else if ( pPrivate->pPV ) {
                notify.hwndPartition = pPrivate->pPV[ pPrivate->usPartSel ];
                notify.usPartition   = pPrivate->usPartSel;
            }

            // Get the selected partition's LVM handle
            if ( notify.hwndPartition ) {
                wp.fsStatus  = WPM_CTLDATA;
                wp.cbCtlData = sizeof( PVCTLDATA );
                wp.pCtlData  = &partdata;
                WinSendMsg( notify.hwndPartition, WM_QUERYWINDOWPARAMS,
                            MPFROMP( &wp ), 0 );
                notify.partition = partdata.handle;
            }
            else notify.partition = 0;

            // Notify our owner that we've been selected
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
            notify.hwndDisk      = hwnd;
            notify.disk          = pPrivate->ctldata.handle;
            notify.usDisk        = pPrivate->id;
            WinSendMsg( hwndOwner, WM_CONTROL,
                        MPFROM2SHORT( pPrivate->id, LDN_SELECT ),
                        MPFROMP( &notify ));

            WinInvalidateRect(hwnd, NULL, FALSE);
            return (MRESULT) TRUE;


        case WM_CHAR:
            fsMask = SHORT1FROMMP( mp1 );
            if ( !( fsMask & KC_KEYUP )) {
                if ( fsMask & KC_VIRTUALKEY ) {
                    pPrivate = WinQueryWindowPtr( hwnd, 0 );
                    if ( !pPrivate ) return (MRESULT) FALSE;
                    hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
                    chKey = SHORT2FROMMP( mp2 );
                    switch ( chKey ) {

                        case VK_LEFT:
                            // Select the previous partition
                            if ( ! pPrivate->pPV ) break;
                            if ( pPrivate->usPartSel > 0 ) {
                                pPrivate->usPartSel--;
                                WinSendMsg( pPrivate->pPV[ pPrivate->usPartSel ],
                                            LPM_SETEMPHASIS, MPFROMSHORT( TRUE ),
                                            MPFROMSHORT( LPV_FS_SELECTED ));
                                DV_ClearPartitionFlags_Other( pPrivate->usPartSel,
                                                              LPV_FS_SELECTED |
                                                              LPV_FS_CONTEXT, pPrivate );
                                // Notify our owner
                                notify.hwndDisk      = hwnd;
                                notify.hwndPartition = pPrivate->pPV[ pPrivate->usPartSel ];
                                notify.disk          = pPrivate->ctldata.handle;
                                notify.partition     = pPrivate->pPartitions[ pPrivate->usPartSel ].handle;
                                notify.usDisk        = pPrivate->id;
                                notify.usPartition   = pPrivate->usPartSel;
                                WinSendMsg( hwndOwner, WM_CONTROL,
                                            MPFROM2SHORT( pPrivate->id, LDN_SELECT ),
                                            MPFROMP( &notify ));
                            }
                            return (MRESULT) TRUE;

                        case VK_RIGHT:
                            // Select the next partition
                            if ( ! pPrivate->pPV ) break;
                            if ( pPrivate->usPartSel < ( pPrivate->usPartitions - 1 )) {
                                pPrivate->usPartSel++;
                                WinSendMsg( pPrivate->pPV[ pPrivate->usPartSel ],
                                            LPM_SETEMPHASIS, MPFROMSHORT( TRUE ),
                                            MPFROMSHORT( LPV_FS_SELECTED ));
                                DV_ClearPartitionFlags_Other( pPrivate->usPartSel,
                                                              LPV_FS_SELECTED |
                                                              LPV_FS_CONTEXT, pPrivate );
                                // Notify our owner
                                notify.hwndDisk      = hwnd;
                                notify.hwndPartition = pPrivate->pPV[ pPrivate->usPartSel ];
                                notify.disk          = pPrivate->ctldata.handle;
                                notify.partition     = pPrivate->pPartitions[ pPrivate->usPartSel ].handle;
                                notify.usDisk        = pPrivate->id;
                                notify.usPartition   = pPrivate->usPartSel;
                                WinSendMsg( hwndOwner, WM_CONTROL,
                                            MPFROM2SHORT( pPrivate->id, LDN_SELECT ),
                                            MPFROMP( &notify ));
                            }
                            return (MRESULT) TRUE;

                        case VK_HOME:
                            // Select the first partition
                            if ( ! pPrivate->pPV ) break;
                            pPrivate->usPartSel = 0;
                            WinSendMsg( pPrivate->pPV[ pPrivate->usPartSel ],
                                        LPM_SETEMPHASIS, MPFROMSHORT( TRUE ),
                                        MPFROMSHORT( LPV_FS_SELECTED ));
                            DV_ClearPartitionFlags_Other( pPrivate->usPartSel,
                                                          LPV_FS_SELECTED |
                                                          LPV_FS_CONTEXT, pPrivate );
                            // Notify our owner
                            notify.hwndDisk      = hwnd;
                            notify.hwndPartition = pPrivate->pPV[ pPrivate->usPartSel ];
                            notify.disk          = pPrivate->ctldata.handle;
                            notify.partition     = pPrivate->pPartitions[ pPrivate->usPartSel ].handle;
                            notify.usDisk        = pPrivate->id;
                            notify.usPartition   = pPrivate->usPartSel;
                            WinSendMsg( hwndOwner, WM_CONTROL,
                                        MPFROM2SHORT( pPrivate->id, LDN_SELECT ),
                                        MPFROMP( &notify ));
                            return (MRESULT) TRUE;

                        case VK_END:
                            // Select the last partition
                            if ( ! pPrivate->pPV || ! pPrivate->usPartitions ) break;
                            pPrivate->usPartSel = pPrivate->usPartitions - 1;
                            WinSendMsg( pPrivate->pPV[ pPrivate->usPartSel ],
                                        LPM_SETEMPHASIS, MPFROMSHORT( TRUE ),
                                        MPFROMSHORT( LPV_FS_SELECTED ));
                            DV_ClearPartitionFlags_Other( pPrivate->usPartSel,
                                                          LPV_FS_SELECTED |
                                                          LPV_FS_CONTEXT, pPrivate );
                            // Notify our owner
                            notify.hwndDisk      = hwnd;
                            notify.hwndPartition = pPrivate->pPV[ pPrivate->usPartSel ];
                            notify.disk          = pPrivate->ctldata.handle;
                            notify.partition     = pPrivate->pPartitions[ pPrivate->usPartSel ].handle;
                            notify.usDisk        = pPrivate->id;
                            notify.usPartition   = pPrivate->usPartSel;
                            WinSendMsg( hwndOwner, WM_CONTROL,
                                        MPFROM2SHORT( pPrivate->id, LDN_SELECT ),
                                        MPFROMP( &notify ));
                            return (MRESULT) TRUE;

                    }
                }
            }
            return (MRESULT) FALSE;


        case WM_CONTEXTMENU:
            // The disk itself (not a child partition) requested a context menu
            pPrivate = WinQueryWindowPtr( hwnd, 0 );

            // No change needed if this was already set
            if ( pPrivate->fsEmphasis & LDV_FS_CONTEXT )
                return (MRESULT) TRUE;

#ifdef AUTO_CONTEXT
            pPrivate->fsEmphasis |= LDV_FS_CONTEXT;
            // Clear all partition context-menu flags
            DV_ClearPartitionFlags_All( LPV_FS_CONTEXT, pPrivate );
#endif

            // Notify our owner
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
            notify.hwndDisk      = hwnd;
            notify.hwndPartition = NULLHANDLE;
            notify.disk          = pPrivate->ctldata.handle;
            notify.partition     = 0;
            notify.usDisk        = pPrivate->id;
            notify.usPartition   = 0;
            WinSendMsg( hwndOwner, WM_CONTROL,
                        MPFROM2SHORT( pPrivate->id, LDN_CONTEXTMENU ),
                        MPFROMP( &notify ));

            WinInvalidateRect(hwnd, NULL, FALSE);
            return (MRESULT) TRUE;


        case WM_CONTROL:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );

            switch ( SHORT2FROMMP( mp1 )) {

                // A partition control was selected
                case LPN_SELECT:

                    // The control ID is also the partition index, as per LDM_SETPARTITIONS
                    usPartIdx = SHORT1FROMMP( mp1 );
                    pPrivate->usPartSel = usPartIdx;

                    // Update our own emphasis styles
                    pPrivate->fsEmphasis |= LDV_FS_SELECTED;
                    pPrivate->fsEmphasis &= ~LDV_FS_CONTEXT;

                    // Now update all other partitions
                    DV_ClearPartitionFlags_Other( usPartIdx,
                                                  LPV_FS_SELECTED |
                                                  LPV_FS_CONTEXT, pPrivate );

                    // Notify our owner
                    notify.hwndDisk      = hwnd;
                    notify.hwndPartition = (HWND) mp2;
                    notify.disk          = pPrivate->ctldata.handle;
                    notify.partition     = pPrivate->pPartitions[ usPartIdx ].handle;
                    notify.usDisk        = pPrivate->id;
                    notify.usPartition   = usPartIdx;
                    WinSendMsg( hwndOwner, WM_CONTROL,
                                MPFROM2SHORT( pPrivate->id, LDN_SELECT ),
                                MPFROMP( &notify ));

                    WinInvalidateRect(hwnd, NULL, FALSE);
                    break;


                // A partition control requested a context-menu
                case LPN_CONTEXTMENU:

                    // The control ID is also the partition index, as per LDM_SETPARTITIONS
                    usPartIdx = SHORT1FROMMP( mp1 );

#ifdef AUTO_CONTEXT
                    // Update our own emphasis styles
                    pPrivate->fsEmphasis &= ~LDV_FS_CONTEXT;
                    // Now update all other partitions
                    DV_ClearPartitionFlags_Other( usPartIdx,
                                                  LPV_FS_CONTEXT, pPrivate );
#endif

                    // Notify our owner
                    notify.hwndDisk      = hwnd;
                    notify.hwndPartition = (HWND) mp2;
                    notify.disk          = pPrivate->ctldata.handle;
                    notify.partition     = pPrivate->pPartitions[ usPartIdx ].handle;
                    notify.usDisk        = pPrivate->id;
                    notify.usPartition   = usPartIdx;
                    WinSendMsg( hwndOwner, WM_CONTROL,
                                MPFROM2SHORT( pPrivate->id, LDN_CONTEXTMENU ),
                                MPFROMP( &notify ));

                    WinInvalidateRect(hwnd, NULL, FALSE);
                    break;

            }
            return (MRESULT) 0;


        case WM_CREATE:
            // initialize the private control data structure
            cb = sizeof( DVPRIVATE );
            if ( !( pPrivate = (PDVPRIVATE) calloc( 1, cb )))
                return (MRESULT) TRUE;
            if ( !mp1 ) return (MRESULT) FALSE;

            // populate the private control data
            memcpy( &(pPrivate->ctldata), mp1, ((PDVCTLDATA)mp1)->cb );
            pPrivate->id = ((PCREATESTRUCT)mp2)->id;
            //sprintf( pPrivate->szSize, "%u MB", pPrivate->ctldata.ulSize );

            // create the icon
            pPrivate->hptrIcon = NULLHANDLE;

            // position the various components
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            DV_Size( hps, hwnd, pPrivate );
            WinEndPaint( hps );

            WinSetWindowPtr( hwnd, 0, pPrivate );
            return (MRESULT) FALSE;


        case WM_DESTROY:
            if (( pPrivate = WinQueryWindowPtr( hwnd, 0 )) != NULL ) {
                if ( pPrivate->usPartitions ) {
                    for ( i = 0; pPrivate->pPV &&
                          ( i < pPrivate->usPartitions ); i++ )
                        WinDestroyWindow( pPrivate->pPV[ i ] );
                    if ( pPrivate->pPV )
                        free( pPrivate->pPV );
                    if ( pPrivate->prclPV )
                        free( pPrivate->prclPV );
                    if ( pPrivate->pPartitions )
                        free( pPrivate->pPartitions );
                }
                free( pPrivate );
            }
            break;


        case WM_PAINT:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            WinQueryWindowRect( hwnd, &rcl );

            // Paint the background according to the current state
            // (Exclude the graphic area because it gets painted below)
            GpiBeginPath( hps, 1L );
            ptl.x = rcl.xLeft;
            ptl.y = rcl.yBottom;
            GpiMove( hps, &ptl );
            ptl.x = rcl.xRight;
            ptl.y = rcl.yTop;
            GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
            ptl.x = pPrivate->rclGraphic.xLeft;
            ptl.y = pPrivate->rclGraphic.yBottom;
            GpiMove( hps, &ptl );
            ptl.x = pPrivate->rclGraphic.xRight;
            ptl.y = pPrivate->rclGraphic.yTop;
            GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
            GpiEndPath( hps );
            GpiSetColor( hps, (pPrivate->fsEmphasis & LDV_FS_SELECTED) ?
                              SYSCLR_HILITEBACKGROUND : SYSCLR_WINDOW );
            GpiFillPath( hps, 1L, FPATH_ALTERNATE );
            GpiSetColor( hps, SYSCLR_WINDOWTEXT );

            // Draw whatever borders (emphasis types) are appropriate
            if ( pPrivate->fsEmphasis & LDV_FS_SELECTED ) {
                GpiSetColor( hps, SYSCLR_HILITEFOREGROUND );
            }
            if ( pPrivate->fsEmphasis & LDV_FS_FOCUS ) {
                GpiSetLineType( hps, LINETYPE_ALTERNATE );
                ptl.x = rcl.xLeft + 1;
                ptl.y = rcl.yBottom + 1;
                GpiMove( hps, &ptl );
                ptl.x = rcl.xRight - 2;
                ptl.y = rcl.yTop - 2;
                GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
            }
            if ( pPrivate->fsEmphasis & LDV_FS_CONTEXT ) {
                GpiSetLineType( hps, LINETYPE_DOT );
                ptl.x = rcl.xLeft + 2;
                ptl.y = rcl.yBottom + 2;
                GpiMove( hps, &ptl );
                ptl.x = rcl.xRight - 3;
                ptl.y = rcl.yTop - 3;
                GpiBox( hps, DRO_OUTLINE, &ptl, 20, 20 );
            }

            // Draw the icon
            if ( pPrivate->hptrIcon != NULLHANDLE )
                WinDrawPointer( hps, pPrivate->rclIcon.xLeft,
                                pPrivate->rclIcon.yBottom, pPrivate->hptrIcon,
                                DP_NORMAL );

            // Draw the text
            SetFontFromPP( hps, hwnd, GetCurrentDPI( hwnd ));
            GpiSetTextAlignment( hps, TA_LEFT, TA_BOTTOM );
            GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm );

            ptl.x = pPrivate->rclText.xLeft + 1;
            ptl.y = pPrivate->rclText.yBottom + 1 + fm.lExternalLeading + ( fm.lEmHeight * 1.2 );
            GpiCharStringPosAt( hps, &ptl, &(pPrivate->rclText), CHS_CLIP,
                                strlen( pPrivate->ctldata.szName ),
                                (PCH) pPrivate->ctldata.szName, NULL );
            ptl.x = pPrivate->rclText.xLeft;
            ptl.y = pPrivate->rclText.yBottom + 1;
            GpiCharStringPosAt( hps, &ptl, &(pPrivate->rclText), CHS_CLIP,
                                strlen( pPrivate->ctldata.szSize ),
                                (PCH) pPrivate->ctldata.szSize, NULL );

            // Draw the outline of the disk graphic
            GpiSetColor( hps, CLR_BLACK );
            GpiSetLineType( hps, LINETYPE_DEFAULT );
            ptl.x = pPrivate->rclGraphic.xLeft;
            ptl.y = pPrivate->rclGraphic.yBottom;
            GpiMove( hps, &ptl );
            ptl.x = pPrivate->rclGraphic.xRight;
            ptl.y = pPrivate->rclGraphic.yTop;
            GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );

            // Fill the interior of the disk graphic
            GpiSetColor( hps, CLR_WHITE );
            ptl.x = pPrivate->rclGraphic.xLeft + 1;
            ptl.y = pPrivate->rclGraphic.yBottom + 1;
            GpiMove( hps, &ptl );
            ptl.x = pPrivate->rclGraphic.xRight - 1;
            ptl.y = pPrivate->rclGraphic.yTop - 1;
            GpiBox( hps, DRO_FILL, &ptl, 0, 0 );

            // Finish up
            WinEndPaint( hps );
            break;


        case WM_PRESPARAMCHANGED:
            switch ( (ULONG) mp1 ) {
                case PP_FONTNAMESIZE:
                    pPrivate = WinQueryWindowPtr( hwnd, 0 );
                    hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
                    DV_Size( hps, hwnd, pPrivate );
                    WinEndPaint( hps );
                    WinInvalidateRect( hwnd, NULL, TRUE );
                    break;

                default: break;
            }
            /*
            // Pass the message up to our owner
            // TODO use a custom notification for this
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
            WinSendMsg( hwndOwner, WM_PRESPARAMCHANGED, mp1, mp2 );
            */
            break;


        case WM_QUERYWINDOWPARAMS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            pwp = (PWNDPARAMS) mp1;

            // Only support WPM_CTLDATA
            if ( pwp && ( pwp->fsStatus & WPM_CTLDATA )) {
                pwp->cbCtlData = sizeof( DVCTLDATA );
                memcpy( pwp->pCtlData, &(pPrivate->ctldata), pwp->cbCtlData );
                return (MRESULT) TRUE;
            }
            return (MRESULT) FALSE;


        case WM_SETWINDOWPARAMS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            pwp = (PWNDPARAMS) mp1;
            if ( !pwp ) return (MRESULT) 0;

            // Only support WPM_CTLDATA
            if (( pwp->fsStatus & WPM_CTLDATA ) &&
                ( pwp->cbCtlData == sizeof( DVCTLDATA )) && pwp->pCtlData )
            {
                memcpy( &(pPrivate->ctldata), pwp->pCtlData, pwp->cbCtlData );
                hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
                DV_LayoutDisk( hps, pPrivate );
                WinEndPaint( hps );
                WinInvalidateRect( hwnd, NULL, TRUE );
            }
            return (MRESULT) 0;


        case WM_SIZE:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            DV_Size( hps, hwnd, pPrivate );
            WinEndPaint( hps );
            WinInvalidateRect( hwnd, NULL, TRUE );
            break;


        // --------------------------------------------------------------------
        // Custom messages defined for this control
        //

        /* .................................................................. *
         * LDM_SETPARTITIONS                                                  *
         *  - mp1 (ULONG)     : Number of partitions                          *
         *  - mp2 (PPVCTLDATA): Array of partition control data structures    *
         *  Returns (BOOL) TRUE on success, FALSE on failure                  *
         * .................................................................. */
        case LDM_SETPARTITIONS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );

            // Free the old array, if any
            if ( pPrivate->pPartitions ) {
                for ( i = 0; pPrivate->pPV &&
                      ( i < pPrivate->usPartitions ); i++ )
                    WinDestroyWindow( pPrivate->pPV[ i ] );
                free( pPrivate->pPartitions );
                free( pPrivate->pPV );
                free( pPrivate->prclPV );
            }
            pPrivate->pPartitions = NULL;
            pPrivate->pPV         = NULL;
            pPrivate->prclPV      = NULL;

            if ( !mp1 ) {
                // 0 partitions means remove all partitions
                pPrivate->usPartitions = 0;
                return (MRESULT) TRUE;
            }
            if ( !mp2 ) {
                // Treat non-zero mp1 with NULL mp2 as an error
                return (MRESULT) FALSE;
            }

            pPrivate->usPartitions = ((ULONG) mp1 > 0xFFFF) ?
                                     0xFFFF : (USHORT) mp1;

            // Allocate the arrays in our private data
            pPrivate->pPartitions = (PPVCTLDATA) calloc( pPrivate->usPartitions,
                                                         sizeof( PVCTLDATA ));
            if ( !pPrivate->pPartitions )
                return (MRESULT) FALSE;
            pPrivate->pPV = (PHWND) calloc( pPrivate->usPartitions,
                                            sizeof( HWND ));
            if ( !pPrivate->pPV ) {
                free( pPrivate->pPartitions );
                return (MRESULT) FALSE;
            }
            pPrivate->prclPV = (PRECTL) calloc( pPrivate->usPartitions,
                                                sizeof( RECTL ));
            if ( !pPrivate->prclPV ) {
                free( pPrivate->pPartitions );
                free( pPrivate->pPV );
                return (MRESULT) FALSE;
            }

            // Populate the partition information array
            memcpy( pPrivate->pPartitions, (PPVCTLDATA) mp2,
                    sizeof( PVCTLDATA ) * (ULONG) mp1 );

            // Create a partition view control for each partition
            for ( i = 0; pPrivate->pPV && ( i < pPrivate->usPartitions ); i++ )
            {
                // (Note: the partition array index is used as the window ID)
                pPrivate->pPV[ i ] = WinCreateWindow(
                    hwnd, WC_PARTITIONVIEW, "", CS_SIZEREDRAW, 0, 0, 0, 0, hwnd,
                    HWND_TOP, (USHORT) i, &(pPrivate->pPartitions[i]), NULL
                );
            }

            // Now calculate the size/position of each control
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            DV_LayoutDisk( hps, pPrivate );
            WinInvalidateRect( hwnd, NULL, TRUE );
            WinEndPaint( hps );

            return (MRESULT) TRUE;


        /* .................................................................. *
         * LDM_SETDISKICON                                                    *
         *  - mp1 (HPOINTER): Handle of the icon or bitmap to use             *
         *  - mp2 (ULONG)   : Unused, should be 0                             *
         *  Returns 0                                                         *
         * .................................................................. */
        case LDM_SETDISKICON:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) 0;
            pPrivate->hptrIcon = (HPOINTER) mp1;
            WinInvalidateRect(hwnd, &(pPrivate->rclIcon), FALSE);
            return (MRESULT) 0;


        /* .................................................................. *
         * LDM_SETEMPHASIS                                                    *
         * - mp1 (HWND)  : Handle of WC_PARTITIONVIEW if setting partition    *
         *                 emphasis; 0 to set first or clear all partitions   *
         * - mp2 (BOOL)  : TRUE to set given emphasis type, FALSE to unset it *
         *       (USHORT): Emphasis flag being set/unset                      *
         *  Returns (BOOL) TRUE on success, FALSE on failure                  *
         * .................................................................. */
        case LDM_SETEMPHASIS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) FALSE;

            fSet = (BOOL) SHORT1FROMMP( mp2 );
            fsMask = SHORT2FROMMP( mp2 );

            if ( fSet ) {

                // First, set the disk-specific flags
                pPrivate->fsEmphasis |= (fsMask & 0xFF00);

                if ( pPrivate->usPartitions && pPrivate->pPV ) {
                    /* Now handle the partitions, as necessary.  Besides setting
                     * the specified emphasis flags on the requested partition,
                     * it may be necessary to clear certain flags depending on what
                     * other flags were specified.
                     */
                    usPartIdx = DV_GetPartitionIndex( (HWND) mp1, pPrivate );
                    hwndPart = pPrivate->pPV[ usPartIdx ];

                    if ( fsMask & LPV_FS_SELECTED ) {
                        // A partition was explicitly selected
                        pPrivate->usPartSel = usPartIdx;
                        // Un-select all partitions except the selected one
                        DV_ClearPartitionFlags_Other( usPartIdx,
                                                      LPV_FS_SELECTED, pPrivate );
                    }
                    else if ( fsMask & LDV_FS_SELECTED ) {
                        // Make sure the first partition (only) will be selected
                        if ( usPartIdx == 0 ) {
                            // We're already operating on the first partition, so
                            // just add the selected flag to its mask...
                            fsMask |= LPV_FS_SELECTED;
                            pPrivate->usPartSel = usPartIdx;
                            // ...and clear selection flags on all other partitions
                            DV_ClearPartitionFlags_Other( usPartIdx,
                                                          LPV_FS_SELECTED,
                                                          pPrivate );
                        }
                        else {
                            // We have to send a separate selection command
                            WinSendMsg( pPrivate->pPV[0], LPM_SETEMPHASIS,
                                        MPFROMSHORT( TRUE ),
                                        MPFROMSHORT( LPV_FS_SELECTED ));
                            pPrivate->usPartSel = 0;
                            DV_ClearPartitionFlags_Other( 0, LPV_FS_SELECTED,
                                                          pPrivate );
                        }
                        /* Note that we shouldn't need to set an explicit
                         * LDN_SELECT notification to our owner at this point.
                         * This is because we'll be receiving an LPN_SELECT from
                         * the partition that we're selecting here, and our
                         * LPN_SELECT handler should take care of sending along
                         * the appropriate LDN_SELECT.
                         */
                    }

                    if ( fsMask & LDV_FS_CONTEXT ) {
                        // Disk has context-menu, clear it from all partitions
                        DV_ClearPartitionFlags_All( LPV_FS_CONTEXT, pPrivate );
                        // Just to make sure it doesn't get reactivated below...
                        fsMask &= ~LPV_FS_CONTEXT;
                    }
                    else if ( fsMask & LPV_FS_CONTEXT ) {
                        // Clear LPV_FS_CONTEXT on all other partitions
                        DV_ClearPartitionFlags_Other( usPartIdx,
                                                      LPV_FS_SELECTED, pPrivate );
                    }

                    // Now we can set the flag(s) for the requested partition
                    WinSendMsg( hwndPart, LPM_SETEMPHASIS,
                                MPFROMSHORT( TRUE ), MPFROMSHORT( fsMask & 0xFF ));

                }
                else {
                    /* No partitions on disk (e.g. empty PRM). There won't be a
                     * LPN_SELECT notification as a result, so tell our owner
                     * we've been selected ourselves.
                     */
                    notify.hwndDisk      = hwnd;
                    notify.hwndPartition = NULLHANDLE;
                    notify.disk          = pPrivate->ctldata.handle;
                    notify.partition     = NULLHANDLE;
                    notify.usDisk        = pPrivate->id;
                    notify.usPartition   = 0;
                    WinSendMsg( hwndOwner, WM_CONTROL,
                                MPFROM2SHORT( pPrivate->id, LDN_SELECT ),
                                MPFROMP( &notify ));
                }
            }
            else {
                hwndPart = (HWND) mp1;

                // Clear the disk flags
                pPrivate->fsEmphasis &= ~(fsMask & 0xFF00);

                if ( pPrivate->usPartitions && pPrivate->pPV ) {

                    // If the disk is de-selected, de-select all partitions as well
                    if ( fsMask & LDV_FS_SELECTED ) {
                        if ( !hwndPart )
                            fsMask |= LPV_FS_SELECTED;
                        else {
                            usPartIdx = DV_GetPartitionIndex( hwndPart, pPrivate );
                            DV_ClearPartitionFlags_Other( usPartIdx,
                                                          LPV_FS_SELECTED,
                                                          pPrivate );
                        }
                    }

                    // Now clear the flags from the specified partition(s)
                    if ( hwndPart )
                        WinSendMsg( hwndPart, LPM_SETEMPHASIS,
                                    MPFROMSHORT( FALSE ),
                                    MPFROMSHORT( fsMask & 0xFF ));
                    else
                        DV_ClearPartitionFlags_All( fsMask & 0xFF, pPrivate );
                }
            }
            // Now force a redraw of the disk control
            if ( fsMask & 0xFF00 )
                WinInvalidateRect( hwnd, NULL, TRUE );

            return (MRESULT) TRUE;


        /* .................................................................. *
         * LDM_GETEMPHASIS                                                    *
         *  - mp1 (HWND): Handle of the partition control being queried (or   *
         *                0 if the disk emphasis is being queried).           *
         *  - mp2:        Unused, should be 0.                                *
         * Returns (USHORT) the emphasis flags for the disk or partition.     *
         * .................................................................. */
        case LDM_GETEMPHASIS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) 0;
            hwndPart = (HWND) mp1;
            if ( hwndPart )
                fsMask = (USHORT) WinSendMsg( hwndPart, LPM_GETEMPHASIS, 0, 0 );
            else
                fsMask = pPrivate->fsEmphasis;
            return (MRESULT) fsMask;


        /* .................................................................. *
         * LDM_SETSTYLE                                                       *
         *  - mp1 (USHORT)    : New style mask                                *
         *  - mp2             : Not used                                      *
         * Returns 0                                                          *
         * .................................................................. */
        case LDM_SETSTYLE:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            fsMask = (USHORT) mp1;
            if ( pPrivate && ( pPrivate->fsStyle != fsMask )) {
                pPrivate->fsStyle = (ULONG) mp1;
                hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
                // Recalculate the layout and force a repaint
                DV_Size( hps, hwnd, pPrivate );
                WinEndPaint( hps );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT) 0;


        /* .................................................................. *
         * LDM_QUERYPARTITIONHWND                                             *
         *  - mp1 (CARDINAL32): Partition array index or LVM handle           *
         *  - mp2 (BOOL)      : TRUE if mp1 contains the partition index      *
         *                      FALSE if mp1 contains the LVM handle          *
         *  Returns (HWND) the window handle of the WC_PARTITIONVIEW          *
         * .................................................................. */
        case LDM_QUERYPARTITIONHWND:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( pPrivate && pPrivate->pPartitions && pPrivate->pPV ) {
                if ( !(BOOL) mp2 ) {
                    ADDRESS partid = (ADDRESS) mp1;
                    for ( i = 0; i < pPrivate->usPartitions; i++ ) {
                        if ( pPrivate->pPartitions[ i ].handle == partid )
                            return (MRESULT) pPrivate->pPV[ i ];
                    }
                }
                else if ( (USHORT) mp1 < pPrivate->usPartitions ) {
                    USHORT usIdx = (USHORT) mp1;
                    return (MRESULT) pPrivate->pPV[ usIdx ];
                }
            }
            return (MRESULT) NULLHANDLE;


        /* .................................................................. *
         * LDM_QUERYPARTITIONEMPHASIS                                         *
         *  - mp1 (HWND):   HWND of the WC_PARTITIONVIEW control to start     *
         *                    searching after, or 0 to search from the first  *
         *                    partition                                       *
         *  - mp2 (USHORT): Emphasis flags to search for                      *
         *  Returns the HWND of the first matching WC_PARTITIONVIEW           *
         *                                                                    *
         * Searches the disk control for a WC_PARTITIONVIEW child control     *
         * with the specified emphasis type(s).                               *
         * .................................................................. */
        case LDM_QUERYPARTITIONEMPHASIS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate || !pPrivate->pPV ) return (MRESULT) 0;

            hwndPart  = (HWND) mp1;
            fsMask    = (USHORT) mp2;
            usPartIdx = 0;
            if ( hwndPart ) {
                for ( i = 0; i < pPrivate->usPartitions; i++ ) {
                    if ( hwndPart == pPrivate->pPV[ i ] ) {
                        usPartIdx = i+1;
                        break;
                    }
                }
            }
            if ( usPartIdx >= pPrivate->usPartitions )
                return (MRESULT) 0;

            for ( i = usPartIdx; i < pPrivate->usPartitions; i++ ) {
                USHORT fsEmphasis;
                fsEmphasis = (USHORT) WinSendMsg( pPrivate->pPV[ i ],
                                                  LPM_GETEMPHASIS, 0, 0 );
                if ( fsEmphasis & fsMask )
                    return (MRESULT) pPrivate->pPV[ i ];
            }
            return (MRESULT) 0;


        /* .................................................................. *
         * LDM_QUERYLASTSELECTED                                              *
         *  - mp1 (BOOL): TRUE to return the partition's HWND                 *
         *                FALSE to return the partition number                *
         *  - mp2       : Unused, should be 0                                 *
         *  Returns either (HWND) the window handle or (USHORT) the number of *
         *  the most recently selected partition                              *
         * .................................................................. */
        case LDM_QUERYLASTSELECTED:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate || !pPrivate->pPV ) return (MRESULT) 0;
            if ( (BOOL) mp1 )
                return (MRESULT) pPrivate->pPV[ pPrivate->usPartSel ];
            else
                return (MRESULT) pPrivate->usPartSel;


        /* .................................................................. *
         * LDM_QUERYPARTITIONS                                                *
         *  - mp1 : Unused, should be 0                                       *
         *  - mp2 : Unused, should be 0                                       *
         * Returns (USHORT) the number of WC_PARTITIONVIEW controls.          *
         * .................................................................. */
        case LDM_QUERYPARTITIONS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate || !pPrivate->pPV ) return (MRESULT) 0;
            return (MRESULT) pPrivate->usPartitions;

    }

    return ( WinDefWindowProc( hwnd, msg, mp1, mp2 ));

}


/* ------------------------------------------------------------------------- *
 * DV_Size                                                                   *
 *                                                                           *
 * (Re)size the disk-view control and all its contents.  Note that this      *
 * simply updates the stored coordinates used by WM_PAINT; this function     *
 * does not itself actually repaint anything.  The changes will only become  *
 * visible the next time WM_PAINT gets processed.                            *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS        hps : Current presentation space handle.                     *
 *   HWND       hwnd: Window handle of control.                              *
 *   PDVPRIVATE pCtl: Pointer to control's internal state data.              *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void DV_Size( HPS hps, HWND hwnd, PDVPRIVATE pCtl )
{
    FONTMETRICS fm;                 // current font metrics
    RECTL rcl;                      // current window bounds
    LONG  lDPI,                     // current font DPI
          lIcon,                    // current icon size (32 or 40)
          lX, lY, lCX, lCY;         // working variables for position/size


    WinQueryWindowRect( hwnd, &rcl );
    lDPI  = GetCurrentDPI( hwnd );
    lIcon = WinQuerySysValue( HWND_DESKTOP, SV_CYICON );

    // Icon control
    lY = (( rcl.yTop - rcl.yBottom ) / 2 ) - ( lIcon / 2 );
    if ( lY < 1 ) lY = 1;
    lX = 10;
    pCtl->rclIcon.xLeft = lX;
    pCtl->rclIcon.yBottom = lY;
    pCtl->rclIcon.xRight = lX + lIcon;
    pCtl->rclIcon.yTop = lY + lIcon;

    // Outside dimensions of the text area
    SetFontFromPP( hps, hwnd, lDPI );
    GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm );
    lCX = ( fm.fsType & FM_TYPE_FIXED ) ?
          fm.lAveCharWidth * ( DISK_NAME_SIZE + 2 ) :
          ( fm.lAveCharWidth + 1 ) * ( DISK_NAME_SIZE + 4 );
    lX += lIcon + 10;
    lCY = 2 + ( fm.lEmHeight * 2.4 ) + ( fm.lExternalLeading * 2 );
    lY = (( rcl.yTop - rcl.yBottom ) / 2 ) - ( lCY / 2 );
    pCtl->rclText.xLeft = rcl.xLeft + lX;
    pCtl->rclText.xRight = pCtl->rclText.xLeft + lCX;
    pCtl->rclText.yBottom = rcl.yBottom + lY;
    pCtl->rclText.yTop = pCtl->rclText.yBottom + lCY;

    // Outside dimensions of the disk graphic
    pCtl->rclGraphic.xLeft = rcl.xLeft + lX + lCX + 8;
    pCtl->rclGraphic.xRight = rcl.xRight - 10;
    pCtl->rclGraphic.yBottom = rcl.yBottom + 10;
    pCtl->rclGraphic.yTop = rcl.yTop - 12;

    // Now resize the disk graphic's contents
    DV_LayoutDisk( hps, pCtl );
}


/* ------------------------------------------------------------------------- *
 * DV_LayoutDisk                                                             *
 *                                                                           *
 * Recalculate the size and contents of the disk graphic.                    *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS        hps : Current presentation space handle.                     *
 *   PPVPRIVATE pCtl: Pointer to control's internal state data.              *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void DV_LayoutDisk( HPS hps, PDVPRIVATE pCtl )
{
    FONTMETRICS fm;                 // current font metrics
    LONG        lBaseWidth,         // unadjusted width of the drawing area
                lWidth, lHeight,    // adjusted dimensions of the drawing area
                lMinCX,             // minimum width required for a partition
                lCurCX,             // calculated width of current partition
                lCurX,              // left offset of current partition
                lTSize;             // adjusted total disk size for calculation
    USHORT      i;                  // loop index
//FILE *f;

    // Just return if there are no partitions to lay out
    if ( !pCtl->usPartitions || !pCtl->pPartitions || !pCtl->pPV ||
         !pCtl->prclPV || !pCtl->ctldata.ulSize )
        return;

    GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm );

    // Get our basic dimensions
    lWidth = pCtl->rclGraphic.xRight - pCtl->rclGraphic.xLeft - 3;
    lWidth -= pCtl->usPartitions - 1; // extra pel for inter-partition dividers
    lHeight = pCtl->rclGraphic.yTop - pCtl->rclGraphic.yBottom - 3;
    if ( lHeight < 1 || lWidth < 1 ) return;
    lBaseWidth = lWidth;

//f = fopen("size.log", "a");

    lTSize = pCtl->ctldata.ulSize;

    if ( pCtl->fsStyle & LDS_FS_UNIFORM ) {                 // Uniform sizing
        lCurX = pCtl->rclGraphic.xLeft + 2;
        lMinCX = lWidth / pCtl->usPartitions;
        for ( i = 0; i < pCtl->usPartitions; i++ ) {
            pCtl->prclPV[ i ].xLeft   = lCurX;
            pCtl->prclPV[ i ].yBottom = pCtl->rclGraphic.yBottom + 2;
            pCtl->prclPV[ i ].yTop    = pCtl->prclPV[ i ].yBottom + lHeight;
            if ( i + 1 == pCtl->usPartitions ) {
                // This prevents any gap at the end due to rounding error
                lCurCX = pCtl->rclGraphic.xRight - 1 - lCurX;
            }
            else {
                lCurCX = lMinCX;
            }
            pCtl->prclPV[ i ].xRight  = lCurX + lCurCX;
            WinSetWindowPos( pCtl->pPV[ i ], HWND_TOP,
                             pCtl->prclPV[ i ].xLeft, pCtl->prclPV[ i ].yBottom,
                             lCurCX, lHeight, SWP_SIZE | SWP_MOVE | SWP_SHOW );
            lCurX = pCtl->prclPV[ i ].xRight + 1;

        }
    }
    else {                                                  // Proportional sizing
        /* Figure out the minimum width to allow for a partition regardless of how
         * small it is.  HOWEVER, if the result would be impossible to fit within
         * the total size of our disk graphic (i.e. it's been resized too small)
         * then don't use a minimum (basically making it the user's problem).
         */
        lMinCX = 4 + ( 3 * fm.lEmInc );
        if (( lMinCX * pCtl->usPartitions ) > lWidth )
            lMinCX = 0;

        /* Now we need to determine the width to allocate for each partition
         * widget.  First, we will give lMinCX pixels to every partition.
         * However much width is left over after that will be allocated
         * proportionally (below) to every partition large enough to merit
         * more than the minimum, based on how large a portion of the
         * remaining disk space it occupies.
         */
        if ( lMinCX ) {
            /* Calculate lTSize (the total size of all partitions which
             * will be larger than the minimum) and lWidth (the graphic
             * width left over after all partitions have been allocated
             * their minimum).
             */
            for ( i = 0; i < pCtl->usPartitions; i++ ) {
                if ( RATIO_CONV( pCtl->pPartitions[ i ].ulSize,
                                 pCtl->ctldata.ulSize, lWidth ) <= lMinCX )
                    lTSize -= pCtl->pPartitions[ i ].ulSize;
            }
            lWidth -= lMinCX * pCtl->usPartitions;
        }
//fprintf(f, "Partitions: %d\t Size: %d\t lTSize: %d\t lBaseWidth: %d\t lMinCX: %d\t lWidth: %d\n", pCtl->usPartitions, pCtl->ctldata.ulSize, lTSize, lBaseWidth, lMinCX, lWidth );

        lCurX = pCtl->rclGraphic.xLeft + 2;
        for ( i = 0; i < pCtl->usPartitions; i++ ) {
            pCtl->prclPV[ i ].xLeft   = lCurX;
            pCtl->prclPV[ i ].yBottom = pCtl->rclGraphic.yBottom + 2;
            pCtl->prclPV[ i ].yTop    = pCtl->prclPV[ i ].yBottom + lHeight;
            if ( i + 1 == pCtl->usPartitions ) {
                /* If we've done our calculations properly, the remaining
                 * space will be ~the correct amount due to the last partition.
                 * We just tell it to use all the remaining space, to allow for
                 * the inevitable difference due to rounding error.
                 */
                lCurCX = pCtl->rclGraphic.xRight - 1 - lCurX;
//fprintf(f, "P%02u (%d): \t%d\n", i, pCtl->pPartitions[ i ].ulSize, lCurCX );
            }
            else if ( lMinCX > 0 ) {
                lCurCX = lMinCX;
                if ( RATIO_CONV( pCtl->pPartitions[ i ].ulSize,
                                 pCtl->ctldata.ulSize, lBaseWidth ) > lCurCX )
                {
                    lCurCX += RATIO_CONV( pCtl->pPartitions[ i ].ulSize, lTSize, lWidth );
                }
//fprintf(f, "P%02u (%d): \t%d + %4d\t = %d\n", i, pCtl->pPartitions[ i ].ulSize, lMinCX, lCurCX - lMinCX, lCurCX );
            }
            /* If lMinCX is disabled (because there's not enough space to give
             * every partition even that much) then just divide them all equally.
             */
            else lCurCX = lWidth / pCtl->usPartitions;

            pCtl->prclPV[ i ].xRight  = lCurX + lCurCX;
            WinSetWindowPos( pCtl->pPV[ i ], HWND_TOP,
                             pCtl->prclPV[ i ].xLeft, pCtl->prclPV[ i ].yBottom,
                             lCurCX, lHeight, SWP_SIZE | SWP_MOVE | SWP_SHOW );
            lCurX = pCtl->prclPV[ i ].xRight + 1;
        }
    }
//fclose(f);
}


/* ------------------------------------------------------------------------- *
 * DV_GetPartitionIndex                                                      *
 *                                                                           *
 * Given the HWND of a WC_PARTITIONVIEW control belonging to the disk, find  *
 * the index of that partition's entry in the WC_DISKVIEW's internal array.  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwndPart: Window handle of WC_PARTITIONVIEW control.         *
 *   PDVPRIVATE pCtl: Pointer to control's internal state data.              *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   The index number of the partition in the internal array.                *
 * ------------------------------------------------------------------------- */
USHORT DV_GetPartitionIndex( HWND hwndPart, PDVPRIVATE pCtl )
{
    USHORT i;

    if ( !hwndPart )
        return 0;

    for ( i = 0; pCtl->pPV && ( i < pCtl->usPartitions ); i++ )
    {
        if ( hwndPart == pCtl->pPV[ i ] ) return i;
    }

    return 0;
}


/* ------------------------------------------------------------------------- *
 * DV_ClearPartitionFlags_All                                                *
 *                                                                           *
 * Clear the specified emphasis flags from all WC_PARTITIONVIEW controls     *
 * belonging to the current WC_DISKVIEW control.                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   USHORT     usFlags: The emphasis flags to be cleared.                   *
 *   PDVPRIVATE pCtl   : Pointer to control's internal state data.           *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DV_ClearPartitionFlags_All( USHORT usFlags, PDVPRIVATE pCtl )
{
    USHORT i;

    for ( i = 0; pCtl->pPV && ( i < pCtl->usPartitions ); i++ )
    {
        WinSendMsg( pCtl->pPV[ i ], LPM_SETEMPHASIS,
                    MPFROMSHORT( FALSE ), MPFROMSHORT( usFlags ));
    }
}


/* ------------------------------------------------------------------------- *
 * DV_ClearPartitionFlags_Other                                              *
 *                                                                           *
 * Clear the specified emphasis flags from all WC_PARTITIONVIEW controls     *
 * belonging to the current WC_DISKVIEW control, EXCEPT for the partition    *
 * whose index number is specified.                                          *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   ULONG      ulIndex: The array index number of the partition whose       *
 *                         flags are NOT to be cleared.                      *
 *   USHORT     usFlags: The emphasis flags to be cleared.                   *
 *   PDVPRIVATE pCtl   : Pointer to control's internal state data.           *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DV_ClearPartitionFlags_Other( USHORT ulIndex, USHORT usFlags, PDVPRIVATE pCtl )
{
    USHORT i;

    for ( i = 0; pCtl->pPV && ( i < pCtl->usPartitions ); i++ )
    {
        if ( i == ulIndex ) continue;
        WinSendMsg( pCtl->pPV[ i ], LPM_SETEMPHASIS,
                    MPFROMSHORT( FALSE ), MPFROMSHORT( usFlags ));
    }
}


// ***************************************************************************
// PRIVATE FUNCTIONS RELATED TO THE WC_LVMDISKS CONTROL
// ***************************************************************************


/* ------------------------------------------------------------------------- *
 * Window procedure for the LVMDiskList control.                             *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY DLDisplayProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PDISKLISTPRIVATE    pPrivate;    // pointer to private control data
    DISKNOTIFY          notify,      // sent notification message structure
                        *pNotified;  // received notification message structure
    HPS                 hps;         // control's presentation space
    RECTL               rcl;         // client bounds
    POINTL              ptl;
    HWND                hwndDisk,    // handle of child disk-view control
                        hwndPart,    // handle of partition-view control
                        hwndOwner;   // handle of our owner
    LONG                lDPI,        // default font DPI
                        lInc,        // requested scroll increment
                        lMaxInc;     // maximum allowable scroll increment
    ULONG               i;           // loop counter
    USHORT              usDiskIdx,   // disk index number in array
                        fsMask;      // options mask
    CHAR                chKey;       // WM_CHAR key value


    switch( msg ) {

        case WM_BUTTON1CLICK:
            // Claim the input focus
            WinSetFocus( HWND_DESKTOP, hwnd );
            return (MRESULT) TRUE;


        case WM_CHAR:
            fsMask = SHORT1FROMMP( mp1 );
            if ( !( fsMask & KC_KEYUP )) {
                if ( fsMask & KC_VIRTUALKEY ) {
                    pPrivate = WinQueryWindowPtr( hwnd, 0 );
                    if ( !pPrivate ) return (MRESULT) FALSE;
                    chKey = SHORT2FROMMP( mp2 );
                    switch ( chKey ) {

                        case VK_LEFT:       // Pass these through to the current disk
                        case VK_RIGHT:
                        case VK_HOME:
                        case VK_END:
                            if ( ! pPrivate->pDV ) break;
                            return WinSendMsg( pPrivate->pDV[ pPrivate->usDiskSel ],
                                               WM_CHAR, mp1, mp2 );

                        case VK_UP:
                            // Select the previous disk
                            if ( ! pPrivate->pDV ) break;
                            if ( pPrivate->usDiskSel > 0 ) {
                                pPrivate->usDiskSel--;
                                pPrivate->usPartSel = 0;
                                DL_SetSelectedDisk( hwnd, pPrivate, TRUE );
                                if ( pPrivate->usDiskSel < pPrivate->ulScrollPos )
                                    WinSendMsg( hwnd, WM_VSCROLL,
                                                MPFROMSHORT( 65535 ),
                                                MPFROM2SHORT( 0, SB_LINEUP ));
                            }
                            return (MRESULT) TRUE;

                        case VK_DOWN:
                            // Select the next disk
                            if ( ! pPrivate->pDV ) break;
                            if ( pPrivate->usDiskSel < ( pPrivate->usDisks - 1 )) {
                                pPrivate->usDiskSel++;
                                pPrivate->usPartSel = 0;
                                DL_SetSelectedDisk( hwnd, pPrivate, TRUE );
                                if (( pPrivate->usDiskSel + 1 ) >
                                     ( pPrivate->ulScrollPos + pPrivate->ulDisplayable ))
                                    WinSendMsg( hwnd, WM_VSCROLL,
                                                MPFROMSHORT( 65535 ),
                                                MPFROM2SHORT( 0, SB_LINEDOWN ));
                            }
                            return (MRESULT) TRUE;

                        case VK_PAGEUP:
                            // Go back by a whole page
                            if ( ! pPrivate->pDV ) break;
                            if ( pPrivate->usDiskSel > 0 ) {
                                pPrivate->usDiskSel =
                                    ( pPrivate->ulScrollPos >= pPrivate->ulDisplayable ) ?
                                    pPrivate->ulScrollPos - pPrivate->ulDisplayable : 0;
                                pPrivate->usPartSel = 0;
                                DL_SetSelectedDisk( hwnd, pPrivate, TRUE );
                                if ( pPrivate->usDiskSel < pPrivate->ulScrollPos )
                                    WinSendMsg( hwnd, WM_VSCROLL,
                                                MPFROMSHORT( 65535 ),
                                                MPFROM2SHORT( 0, SB_PAGEUP ));
                            }
                            return (MRESULT) TRUE;
                            break;

                        case VK_PAGEDOWN:
                            // Advance by a whole page
                            if ( ! pPrivate->pDV ) break;
                            if ( pPrivate->usDiskSel < ( pPrivate->usDisks - 1 )) {
                                pPrivate->usDiskSel = pPrivate->ulScrollPos + pPrivate->ulDisplayable;
                                if ( pPrivate->usDiskSel >= pPrivate->usDisks )
                                    pPrivate->usDiskSel = pPrivate->usDisks - 1;
                                pPrivate->usPartSel = 0;
                                DL_SetSelectedDisk( hwnd, pPrivate, TRUE );
                                if (( pPrivate->usDiskSel + 1 ) >
                                     ( pPrivate->ulScrollPos + pPrivate->ulDisplayable ))
                                    WinSendMsg( hwnd, WM_VSCROLL,
                                                MPFROMSHORT( 65535 ),
                                                MPFROM2SHORT( 0, SB_PAGEDOWN ));
                            }
                            return (MRESULT) TRUE;

                    }
                }
            }
            break;


        case WM_CONTEXTMENU:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) FALSE;

            // Clear all disk context-menu flags
            for ( i = 0; pPrivate->pDV && ( i < pPrivate->usDisks ); i++ ) {
                WinSendMsg( pPrivate->pDV[ i ], LDM_SETEMPHASIS, 0,
                            MPFROM2SHORT( FALSE,
                                          LDV_FS_CONTEXT | LPV_FS_CONTEXT ));
            }

            // Notify our owner
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
            notify.hwndDisk = NULLHANDLE;
            notify.hwndPartition = NULLHANDLE;
            notify.disk = 0;
            notify.partition = 0;
            notify.usDisk = 0;
            notify.usPartition = 0;
            WinSendMsg( hwndOwner, WM_CONTROL,
                        MPFROM2SHORT( pPrivate->id, LLN_CONTEXTMENU ),
                        MPFROMP( &notify ));
            return (MRESULT) TRUE;


        case WM_CONTROL:
            pPrivate  = WinQueryWindowPtr( hwnd, 0 );
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );

            switch ( SHORT2FROMMP( mp1 )) {

                case LDN_SELECT:
                    // A disk was selected
                    pNotified = (PDISKNOTIFY) mp2;
                    if ( !pNotified || !pPrivate ) break;

                    // Remember the currently-selected disk
                    pPrivate->usDiskSel = pNotified->usDisk;
                    // And the selected partition on that disk (this will be 0
                    // a.k.a. first partition if none was reported).
                    pPrivate->usPartSel = pNotified->usPartition;

#ifdef FOCUS_BORDER
                    WinSendMsg( pNotified->hwndDisk, LDM_SETEMPHASIS,
                                0, MPFROM2SHORT( TRUE, LDV_FS_FOCUS ));
#endif

                    // Clear selection and context-menu flags on all other disks
                    DL_ClearDiskFlags_Other( pNotified->usDisk,
                                             LDV_FS_SELECTED |
                                             LDV_FS_CONTEXT  |
#ifdef FOCUS_BORDER
                                             LDV_FS_FOCUS    |
#endif
                                             LPV_FS_CONTEXT, pPrivate );

                    // Grab the keyboard focus
                    WinSetFocus( HWND_DESKTOP, hwnd );

                    // Notify our owner
                    WinSendMsg( hwndOwner, WM_CONTROL,
                                MPFROM2SHORT( pPrivate->id, LLN_EMPHASIS ), mp2 );
                    break;


                case LDN_CONTEXTMENU:
                    // A disk generated a context-menu event
                    pNotified = (PDISKNOTIFY) mp2;
                    if ( !pNotified || !pPrivate ) break;

                    // Clear context-menu flags on all other disks
                    DL_ClearDiskFlags_Other( pNotified->usDisk,
                                             LDV_FS_CONTEXT |
                                             LPV_FS_CONTEXT, pPrivate );

                    // Notify our owner
                    WinSendMsg( hwndOwner, WM_CONTROL,
                                MPFROM2SHORT( pPrivate->id, LLN_CONTEXTMENU ), mp2 );
                    break;

            }
            return (MRESULT) 0;


        case WM_CREATE:
            // initialize the private control data structure
            pPrivate = (PDISKLISTPRIVATE) calloc( 1, sizeof( DISKLISTPRIVATE ));
            if ( !pPrivate )
                return (MRESULT) TRUE;

            pPrivate->id = ((PCREATESTRUCT)mp2)->id;

            // create the scrollbar
            pPrivate->hwndScroll = WinCreateWindow( hwnd, WC_SCROLLBAR, NULL,
                                                    SBS_VERT | SBS_AUTOTRACK | WS_VISIBLE,
                                                    0, 0, 0, 0, hwnd, HWND_TOP,
                                                    -1, NULL, NULL );
            // initialize the control layout
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            lDPI = GetCurrentDPI( hwnd );
            SetFontFromPP( hps, hwnd, lDPI );
            DL_Size( hps, hwnd, pPrivate );
            WinEndPaint( hps );

            WinSetWindowPtr( hwnd, 0, pPrivate );
            return (MRESULT) FALSE;


        case WM_DESTROY:
            if (( pPrivate = WinQueryWindowPtr( hwnd, 0 )) != NULL ) {
                if ( pPrivate->usDisks ) {
                    for ( i = 0; pPrivate->pDV &&
                          ( i < pPrivate->usDisks ); i++ )
                        WinDestroyWindow( pPrivate->pDV[ i ] );
                    if ( pPrivate->pDV )
                        free( pPrivate->pDV );
                    if ( pPrivate->pDisks )
                        free( pPrivate->pDisks );
                    if ( pPrivate->hwndScroll )
                        WinDestroyWindow( pPrivate->hwndScroll );
                }
                if ( pPrivate->pszTitle )
                    free( pPrivate->pszTitle );
                free( pPrivate );
            }
            break;


        case WM_PAINT:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate )
                return (MRESULT) TRUE;
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );

            WinQueryWindowRect( hwnd, &rcl );

            /* Paint the background, except for the scrollbar area and whatever
             * is already covered by a disk control.
             */
            if ( pPrivate->rclScroll.xLeft )
                rcl.xRight = pPrivate->rclScroll.xLeft;
            if ( pPrivate->lDiskBottom > 0 )
                rcl.yTop = pPrivate->lDiskBottom;
            WinFillRect( hps, &rcl, SYSCLR_WINDOW );

            // Paint the title area, if there is one
            if ( pPrivate->fs & LLD_FS_TITLE ) {
                WinQueryWindowRect( hwnd, &rcl );
                rcl.yBottom = pPrivate->lDiskTop;
                WinFillRect( hps, &rcl, SYSCLR_WINDOW );
                ptl.x = rcl.xLeft;
                ptl.y = rcl.yBottom;
                GpiMove( hps, &ptl );
                ptl.x = rcl.xRight;
                GpiSetColor( hps, CLR_DARKGRAY );
                GpiLine( hps, &ptl );
                // Now draw the actual title
                if ( pPrivate->pszTitle ) {
                    lDPI = GetCurrentDPI( hwnd );
                    SetFontFromPP( hps, hwnd, lDPI );
                    ptl.x = ( rcl.xRight - rcl.xLeft ) / 2;
                    ptl.y = rcl.yBottom + (( rcl.yTop - rcl.yBottom ) / 2 );
                    GpiSetColor( hps, SYSCLR_WINDOWTEXT );
                    GpiSetTextAlignment( hps, TA_CENTER, TA_HALF );
                    GpiCharStringPosAt( hps, &ptl, &rcl,
                                        CHS_LEAVEPOS | CHS_CLIP,
                                        strlen( pPrivate->pszTitle ),
                                        pPrivate->pszTitle, 0 );
                }
            }
            WinEndPaint( hps );
            break;


        case WM_PRESPARAMCHANGED:
            switch ( (ULONG) mp1 ) {
                case PP_FONTNAMESIZE:
                    pPrivate = WinQueryWindowPtr( hwnd, 0 );
                    hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );

                    // Set the font as requested
                    lDPI = GetCurrentDPI( hwnd );
                    SetFontFromPP( hps, hwnd, lDPI );

                    // Resize the list contents as needed
                    if ( pPrivate )
                        DL_Size( hps, hwnd, pPrivate );

                    // Finish up
                    WinEndPaint( hps );
                    WinInvalidateRect( hwnd, NULL, TRUE );
                    break;

                default: break;
            }
            break;


        case WM_SETFOCUS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) 0;
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );

            if ( (BOOL) mp2 ) {     // Receiving focus
                // Restore selection on the current disk/partition
                if ( pPrivate->usDisks && pPrivate->pDV &&
                     ( pPrivate->usDiskSel < pPrivate->usDisks ))
                {
                    DL_SetSelectedDisk( hwnd, pPrivate, TRUE );
                }
                // Tell our owner that we've received focus
                WinSendMsg( hwndOwner, WM_CONTROL,
                            MPFROM2SHORT( pPrivate->id, LLN_SETFOCUS ),
                            MPFROMP( hwnd ));
            }
            else {                  // Losing focus
                if ( pPrivate->usDisks && pPrivate->pDV &&
                     ( pPrivate->usDiskSel < pPrivate->usDisks ))
                {
#ifdef FOCUS_BORDER
                    // Clear the focus border from the current disk
                    WinSendMsg( pPrivate->pDV[ pPrivate->usDiskSel ],
                                LDM_SETEMPHASIS, 0,
                                MPFROM2SHORT( FALSE, LDV_FS_FOCUS ));
#else
                    // Clear selection from the current partition (only)
                    WinSendMsg( pPrivate->pDV[ pPrivate->usDiskSel ],
                                LDM_SETEMPHASIS, 0,
                                MPFROM2SHORT( FALSE, LPV_FS_SELECTED ));
#endif
                }
                // Tell our owner that we've lost focus
                WinSendMsg( hwndOwner, WM_CONTROL,
                            MPFROM2SHORT( pPrivate->id, LLN_KILLFOCUS ),
                            MPFROMP( hwnd ));
            }
            return (MRESULT) 0;


        case WM_SIZE:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) break;
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            lDPI = GetCurrentDPI( hwnd );
            SetFontFromPP( hps, hwnd, lDPI );
            DL_Size( hps, hwnd, pPrivate );
            WinEndPaint( hps );
            WinInvalidateRect( hwnd, NULL, TRUE );
            break;


        case WM_VSCROLL:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) 0;

            if ( pPrivate->usDisks <= pPrivate->ulDisplayable )
                return (MRESULT) 0;

            switch ( SHORT2FROMMP( mp2 )) {
                case SB_LINEUP:
                    lInc = -1;
                    break;
                case SB_LINEDOWN:
                    lInc = 1;
                    break;
                case SB_PAGEUP:
                    lInc = min( (LONG)(-(pPrivate->ulDisplayable)) + 1, -1 );
                    break;
                case SB_PAGEDOWN:
                    lInc = max( pPrivate->ulDisplayable - 1, 1 );
                    break;
                case SB_SLIDERPOSITION:
                case SB_SLIDERTRACK:
                    lInc = SHORT1FROMMP( mp2 ) - pPrivate->ulScrollPos;
                    break;
                default:
                    return (MRESULT) 0;
            }
            // Do some bounds checking on the increment
            lMaxInc = max( 0, (LONG)( pPrivate->usDisks - pPrivate->ulDisplayable ));
            if ((LONG)( pPrivate->ulScrollPos + lInc ) < 0 )
                lInc = -(pPrivate->ulScrollPos);
            else if (( pPrivate->ulScrollPos + lInc ) > lMaxInc )
                lInc = lMaxInc - pPrivate->ulScrollPos;

            // Now scroll ourselves by the indicated amount
            if ( lInc != 0 ) {
                pPrivate->ulScrollPos += lInc;
                DL_ArrangeDisks( pPrivate, pPrivate->lDiskTop );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            break;


        // --------------------------------------------------------------------
        // Custom messages defined for this control
        //

        /* .................................................................. *
         * LLM_SETDISKS                                                       *
         *  - mp1 (ULONG)     : Number of disks                               *
         *  - mp2 (PDVCTLDATA): Array of disk control data structures         *
         * Returns (BOOL) TRUE on success, FALSE on failure                   *
         * .................................................................. */
        case LLM_SETDISKS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) TRUE;

            // Free the old array, if any
            if ( pPrivate->pDisks ) {
                for ( i = 0; pPrivate->pDV &&
                      ( i < pPrivate->usDisks ); i++ )
                {
                    // destroying the disk controls also frees their partitions
                    WinDestroyWindow( pPrivate->pDV[ i ] );
                }
                free( pPrivate->pDisks );
                free( pPrivate->pDV );
            }
            pPrivate->pDisks = NULL;
            pPrivate->pDV = NULL;

            if ( !mp1 ) {
                // 0 disks means remove all disks
                pPrivate->usDisks = 0;
                return (MRESULT) TRUE;
            }
            if ( !mp2 ) {
                // Treat non-zero mp1 with NULL mp2 as an error
                return (MRESULT) FALSE;
            }

            pPrivate->usDisks = ((ULONG) mp1 > 0xFFFF) ? 0xFFFF : (USHORT) mp1;

            // Allocate the arrays in our private data
            pPrivate->pDisks = (PDVCTLDATA) calloc( pPrivate->usDisks,
                                                    sizeof( DVCTLDATA ));
            if ( !pPrivate->pDisks )
                return (MRESULT) FALSE;
            pPrivate->pDV = (PHWND) calloc( pPrivate->usDisks,
                                            sizeof( HWND ));
            if ( !pPrivate->pDV ) {
                free( pPrivate->pDisks );
                pPrivate->pDisks = NULL;
                return (MRESULT) FALSE;
            }

            // Populate the disk information array
            memcpy( pPrivate->pDisks, (PDVCTLDATA) mp2,
                    sizeof( DVCTLDATA ) * pPrivate->usDisks );

            // Create a disk view control for each partition
            for ( i = 0; pPrivate->pDV && ( i < pPrivate->usDisks ); i++ )
            {
                // (Note: the disk array index is used as the window ID)
                pPrivate->pDV[ i ] = WinCreateWindow(
                    hwnd, WC_DISKVIEW, "", CS_SIZEREDRAW | WS_CLIPCHILDREN,
                    0, 0, 0, 0, hwnd, HWND_TOP, (USHORT) i,
                    &(pPrivate->pDisks[i]), NULL
                );
            }
            // Recalculate the content layout and force a repaint
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            lDPI = GetCurrentDPI( hwnd );
            SetFontFromPP( hps, hwnd, lDPI );
            DL_Size( hps, hwnd, pPrivate );
            WinEndPaint( hps );
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT) TRUE;


        /* .................................................................. *
         * LLM_QUERYDISKHWND                                                  *
         *  - mp1 (varies): disk array index number or LVM disk handle        *
         *  - mp2 (BOOL)  : TRUE if mp1 contains the disk array index         *
         *                  FALSE if mp1 contains the LVM disk handle         *
         * Returns (HWND) the window handle of the corresponding WC_DISKVIEW  *
         * .................................................................. */
        case LLM_QUERYDISKHWND:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( pPrivate && pPrivate->pDisks && pPrivate->pDV ) {
                if ( (BOOL) mp2 ) {
                    USHORT usIdx = (USHORT) mp1;
                    if ( usIdx < pPrivate->usDisks )
                        return (MRESULT) pPrivate->pDV[ usIdx ];
                }
                else {
                    ADDRESS diskid = (ADDRESS) mp1;
                    for ( i = 0; i < pPrivate->usDisks; i++ ) {
                        if ( pPrivate->pDisks[ i ].handle == diskid )
                            return (MRESULT) pPrivate->pDV[ i ];
                   }
                }
            }
            return (MRESULT) NULLHANDLE;


        /* .................................................................. *
         * LLM_QUERYDISKEMPHASIS                                              *
         *  - mp1 (HWND):   HWND of the WC_DISKVIEW to start searching after, *
         *                    or 0 to search from the first disk              *
         *  - mp2 (USHORT): Emphasis flags to search for                      *
         * Returns the HWND of the first matching WC_DISKVIEW control         *
         * .................................................................. */
        case LLM_QUERYDISKEMPHASIS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate || !pPrivate->pDV ) return (MRESULT) 0;

            hwndDisk  = (HWND) mp1;
            fsMask    = (USHORT) mp2;
            usDiskIdx = 0;
            if ( hwndDisk ) {
                for ( i = 0; i < pPrivate->usDisks; i++ ) {
                    if ( hwndDisk == pPrivate->pDV[ i ] ) {
                        usDiskIdx = i+1;
                        break;
                    }
                }
            }
            if ( usDiskIdx >= pPrivate->usDisks )
                return (MRESULT) 0;

            for ( i = usDiskIdx; i < pPrivate->usDisks; i++ ) {
                USHORT fsEmphasis;
                fsEmphasis = (USHORT) WinSendMsg( pPrivate->pDV[ i ],
                                                  LDM_GETEMPHASIS, 0, 0 );
                if ( fsEmphasis & fsMask )
                    return (MRESULT) pPrivate->pDV[ i ];
            }
            return (MRESULT) 0;


        /* .................................................................. *
         * LLM_SETDISKEMPHASIS                                                *
         * - mp1 (HWND)  : Handle of the WC_DISKVIEW control whose emphasis   *
         *                   is to be set.                                    *
         * - mp2 (BOOL)  : If TRUE: set given emphasis type(s)                *
         *                 If FALSE: clear the given emphasis type(s)         *
         *       (USHORT): Emphasis type(s) to set; available values are:     *
         *                  LDV_FS_SELECTED  selection/focus emphasis         *
         *                  LDV_FS_CONTEXT   context-menu emphasis            *
         * Returns (BOOL) TRUE on success, FALSE on failure                   *
         * .................................................................. */
        case LLM_SETDISKEMPHASIS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate || !pPrivate->pDV ) return (MRESULT) FALSE;

            hwndDisk  = (HWND) mp1;
            fsMask    = SHORT2FROMMP( mp2 );
            fsMask   &= 0xFF00;         // we don't allow partition flags here
            usDiskIdx = DL_GetDiskIndex( hwndDisk, pPrivate );

            if ( (BOOL) SHORT1FROMMP( mp2 )) {
                WinSendMsg( hwndDisk, LDM_SETEMPHASIS, 0,
                            MPFROM2SHORT( TRUE, fsMask ));

                if ( fsMask & LDV_FS_SELECTED ) {
                    pPrivate->usDiskSel = usDiskIdx;
                    DL_ClearDiskFlags_Other( usDiskIdx,
                                             LDV_FS_SELECTED, pPrivate );
                    if ( !( fsMask & LDV_FS_CONTEXT ))
                        DL_ClearDiskFlags_All( LDV_FS_CONTEXT, pPrivate );
                }
                if ( fsMask & LDV_FS_FOCUS ) {
                    DL_ClearDiskFlags_Other( usDiskIdx,
                                             LDV_FS_FOCUS, pPrivate );
                }
                if ( fsMask & LDV_FS_CONTEXT ) {
                    DL_ClearDiskFlags_Other( usDiskIdx,
                                             LDV_FS_CONTEXT, pPrivate );
                }
            }
            else {
                WinSendMsg( hwndDisk, LDM_SETEMPHASIS, 0,
                            MPFROM2SHORT( FALSE, fsMask ));
            }
            return (MRESULT) TRUE;


        /* .................................................................. *
         * LLM_GETPARTITION                                                   *
         *  - mp1 (ADDRESS): LVM partition handle                             *
         *  - mp2 (ADDRESS): LVM disk handle (optional)                       *
         * Returns (HWND) the window handle of the corresponding partition    *
         * .................................................................. */
        case LLM_GETPARTITION:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate || !pPrivate->usDisks ||
                 !pPrivate->pDV || !pPrivate->pDisks )
                return (MRESULT) NULLHANDLE;

            for ( i = 0; i < pPrivate->usDisks; i++ ) {
                if ( mp2 && pPrivate->pDisks[ i ].handle == (ADDRESS) mp2 )
                {
                    return WinSendMsg( pPrivate->pDV[ i ],
                                       LDM_QUERYPARTITIONHWND,
                                       mp1, MPFROMSHORT( FALSE ));
                }
                else {
                    hwndPart = (HWND) WinSendMsg( pPrivate->pDV[ i ],
                                                  LDM_QUERYPARTITIONHWND,
                                                  mp1, MPFROMSHORT( FALSE ));
                    if ( hwndPart != NULLHANDLE )
                        return (MRESULT) hwndPart;
                }
            }
            return (MRESULT) NULLHANDLE;


        /* .................................................................. *
         * LLM_SETSTYLE                                                       *
         *  - mp1 (USHORT)    : New style mask                                *
         *  - mp2             : Not used                                      *
         * Returns 0                                                          *
         * .................................................................. */
        case LLM_SETSTYLE:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            fsMask = (USHORT) mp1;
            if ( pPrivate && ( pPrivate->fs != fsMask )) {
                pPrivate->fs = (ULONG) mp1;
                hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
                // Recalculate the layout and force a repaint
                SetFontFromPP( hps, hwnd, lDPI );
                DL_Size( hps, hwnd, pPrivate );
                WinEndPaint( hps );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT) 0;


        /* .................................................................. *
         * LLM_SETTITLE                                                       *
         *  - mp1 (PSZ) : Title string                                        *
         *  - mp2       : Unused, should be 0                                 *
         *  Returns 0.                                                        *
         * .................................................................. */
        case LLM_SETTITLE:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( pPrivate && mp1 ) {
                pPrivate->pszTitle = strdup( (PSZ) mp1 );
                WinQueryWindowRect( hwnd, &rcl );
                rcl.yBottom = pPrivate->lDiskTop;
                WinInvalidateRect( hwnd, &rcl, TRUE );
            }
            return (MRESULT) 0;


        /* .................................................................. *
         * LLM_QUERYDISKS                                                     *
         *  - mp1 : Unused, should be 0                                       *
         *  - mp2 : Unused, should be 0                                       *
         * Returns (USHORT) the number of WC_DISKVIEW controls.               *
         * .................................................................. */
        case LLM_QUERYDISKS:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( pPrivate ) {
                return (MRESULT) pPrivate->usDisks;
            }
            return (MRESULT) 0;

    }

    return ( WinDefWindowProc( hwnd, msg, mp1, mp2 ));
}


/* ------------------------------------------------------------------------- *
 * DL_Size                                                                   *
 *                                                                           *
 * (Re)size the disk list control and all its contents.                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS              hps : Current presentation space handle.               *
 *   HWND             hwnd: Window handle of control.                        *
 *   PDISKLISTPRIVATE pCtl: Pointer to control's internal state data.        *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void DL_Size( HPS hps, HWND hwnd, PDISKLISTPRIVATE pCtl )
{
    FONTMETRICS fm;             // current font metrics
    RECTL       rcl;            // window client area
    LONG        lSBWidth,       // scrollbar width
                lTHeight,       // height of title area
                lListHeight,    // height of scrollable list area
                lIcon;          // default icon size

    if ( !pCtl ) return;

    GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm );
    WinQueryWindowRect( hwnd, &rcl );

    // set the title area height
    lTHeight = ( pCtl->fs & LLD_FS_TITLE ) ? (2 * fm.lEmHeight) : 0;
    pCtl->lDiskTop = rcl.yTop - lTHeight;

    // set the scrollbar size
    lSBWidth = 16;
    pCtl->rclScroll.xLeft   = rcl.xRight - lSBWidth;
    pCtl->rclScroll.xRight  = rcl.xRight;
    pCtl->rclScroll.yBottom = rcl.yBottom;
    pCtl->rclScroll.yTop    = rcl.yTop - lTHeight;
    lListHeight = pCtl->rclScroll.yTop - pCtl->rclScroll.yBottom;
    if ( pCtl->hwndScroll )
        WinSetWindowPos( pCtl->hwndScroll, HWND_TOP,
                         pCtl->rclScroll.xLeft, pCtl->rclScroll.yBottom,
                         lSBWidth, lListHeight, SWP_SIZE | SWP_MOVE );

    // recalculate the disk control size (this is the same for all disks)
    pCtl->lDiskWidth = pCtl->rclScroll.xLeft;
    // the disk control height depends on both the current font and icon sizes
    lIcon = WinQuerySysValue( HWND_DESKTOP, SV_CYICON );
    pCtl->lDiskHeight = max( lIcon + 2, fm.lMaxBaselineExt * 4 ) + 8;

    if ( pCtl->usDisks ) {
        DL_ArrangeDisks( pCtl, pCtl->lDiskTop );
    }
    else {
        WinSendMsg( pCtl->hwndScroll, SBM_SETTHUMBSIZE,
                    MPFROM2SHORT( 1, 1 ), 0 );
        WinSendMsg( pCtl->hwndScroll, SBM_SETSCROLLBAR,
                    MPFROMSHORT( 0 ),
                    MPFROM2SHORT( 0, 1 ));
    }
}


/* ------------------------------------------------------------------------- *
 * DL_ArrangeDisks                                                           *
 *                                                                           *
 * (Re)position all visible disk controls.                                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDISKLISTPRIVATE pCtl: Pointer to control's internal state data.        *
 *   LONG             lTop: y-coordinate of the top of the list control.     *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void DL_ArrangeDisks( PDISKLISTPRIVATE pCtl, LONG lTop )
{
    LONG  lDiskPos;       // y offset of current disk control
    ULONG ulDocHeight,    // total conceptual height of contents
          i;


    // recalculate the viewport size
    pCtl->ulDisplayable = lTop / pCtl->lDiskHeight;

    // this avoids "losing" disks off the top of the viewport
    if ( pCtl->usDisks <= pCtl->ulDisplayable )
        pCtl->ulScrollPos = 0;

    // now position the disk controls
    lDiskPos = lTop;
    for ( i = 0; pCtl->pDV && ( i < pCtl->ulScrollPos ); i++ ) {
        WinShowWindow( pCtl->pDV[ i ], FALSE );
    }
    for ( i = pCtl->ulScrollPos; pCtl->pDV && ( i < pCtl->usDisks ); i++ ) {
        lDiskPos -= pCtl->lDiskHeight;
        WinSetWindowPos( pCtl->pDV[ i ], HWND_TOP, 0, lDiskPos,
                         pCtl->lDiskWidth, pCtl->lDiskHeight,
                         SWP_SIZE | SWP_MOVE | SWP_SHOW );
    }
    pCtl->lDiskBottom = lDiskPos;

    // update the scrollbar thumb position/size
    ulDocHeight = pCtl->usDisks;
    WinSendMsg( pCtl->hwndScroll, SBM_SETTHUMBSIZE,
                MPFROM2SHORT( pCtl->ulDisplayable, ulDocHeight ), 0 );
    WinSendMsg( pCtl->hwndScroll, SBM_SETSCROLLBAR,
                MPFROMSHORT( pCtl->ulScrollPos ),
                MPFROM2SHORT( 0, pCtl->usDisks - pCtl->ulDisplayable ));

}


/* ------------------------------------------------------------------------- *
 * DL_GetDiskIndex                                                           *
 *                                                                           *
 * Given the HWND of a WC_DISKVIEW control belonging to the disk list, find  *
 * the index of that disk's entry in the disk list's internal array.         *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND             hwndDisk: Window handle of WC_DISKVIEW control.        *
 *   PDISKLISTPRIVATE pCtl    : Pointer to control's internal state data.    *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   The index number of the partition in the internal array.                *
 * ------------------------------------------------------------------------- */
USHORT DL_GetDiskIndex( HWND hwndDisk, PDISKLISTPRIVATE pCtl )
{
    USHORT i;

    if ( !hwndDisk )
        return 0;

    for ( i = 0; pCtl->pDV && ( i < pCtl->usDisks ); i++ )
    {
        if ( hwndDisk == pCtl->pDV[ i ] ) return i;
    }

    return 0;
}


/* ------------------------------------------------------------------------- *
 * DL_ClearDiskFlags_All                                                     *
 *                                                                           *
 * Clear the specified emphasis flags from all WC_DISKVIEW controls in the   *
 * disk list.                                                                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   USHORT           usFlags: The emphasis flags to be cleared.             *
 *   PDISKLISTPRIVATE pCtl   : Pointer to control's internal state data.     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DL_ClearDiskFlags_All( USHORT usFlags, PDISKLISTPRIVATE pCtl )
{
    ULONG i;

    for ( i = 0; pCtl->pDV && ( i < pCtl->usDisks ); i++ )
    {
        WinSendMsg( pCtl->pDV[ i ], LDM_SETEMPHASIS,
                    0, MPFROM2SHORT( FALSE, usFlags ));
    }
}


/* ------------------------------------------------------------------------- *
 * DL_ClearDiskFlags_Other                                                   *
 *                                                                           *
 * Clear the specified emphasis flags from all WC_DISKVIEW controls in the   *
 * disk list EXCEPT for the disk whose index number is specified.            *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   USHORT           usIndex: The array index number of the disk whose      *
 *                               flags are NOT to be cleared.                *
 *   USHORT           usFlags: The emphasis flags to be cleared.             *
 *   PDISKLISTPRIVATE pCtl   : Pointer to control's internal state data.     *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DL_ClearDiskFlags_Other( USHORT usIndex, USHORT usFlags, PDISKLISTPRIVATE pCtl )
{
    ULONG i;

    for ( i = 0; pCtl->pDV && ( i < pCtl->usDisks ); i++ )
    {
        if ( i == usIndex ) continue;
        WinSendMsg( pCtl->pDV[ i ], LDM_SETEMPHASIS,
                    0, MPFROM2SHORT( FALSE, usFlags ));
    }
}


/* ------------------------------------------------------------------------- *
 * DL_SetSelectedDisk                                                        *
 *                                                                           *
 * Handles the necessary flag updates and notification messages when a new   *
 * disk in the disk list is selected.  pCtl must already indicate the new    *
 * disk and partition numbers.                                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND             hwnd  : Window handle of the disk list control.        *
 *   PDISKLISTPRIVATE pCtl  : Pointer to control's internal state data.      *
 *   BOOL             fFocus: Disk list has keyboard focus (TRUE/FALSE)      *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DL_SetSelectedDisk( HWND hwnd, PDISKLISTPRIVATE pCtl, BOOL fFocus )
{
    DISKNOTIFY notify = {0};
    WNDPARAMS  wp     = {0};
    PVCTLDATA  pvdata = {0};
    DVCTLDATA  dvdata = {0};


    // Get the disk and partition HWNDs
    notify.hwndDisk      = pCtl->pDV[ pCtl->usDiskSel ];
    notify.hwndPartition = (HWND) WinSendMsg( notify.hwndDisk,
                                              LDM_QUERYPARTITIONHWND,
                                              MPFROMLONG( pCtl->usPartSel ),
                                              MPFROMSHORT( TRUE ));

    // Update the flags on the newly-selected disk and partition
#ifdef FOCUS_BORDER
    if ( fFocus ) {
        WinSendMsg( notify.hwndDisk, LDM_SETEMPHASIS,
                    MPFROMP( notify.hwndPartition ),
                    MPFROM2SHORT( TRUE, LDV_FS_SELECTED | LDV_FS_FOCUS |
                                        LPV_FS_SELECTED ));
        DL_ClearDiskFlags_Other( pCtl->usDiskSel,
                                 LDV_FS_SELECTED | LDV_FS_CONTEXT | LDV_FS_FOCUS |
                                 LPV_FS_CONTEXT,
                                 pCtl );
    }
    else
#endif
    {
        WinSendMsg( notify.hwndDisk, LDM_SETEMPHASIS,
                    MPFROMP( notify.hwndPartition ),
                    MPFROM2SHORT( TRUE, LDV_FS_SELECTED | LPV_FS_SELECTED ));
        DL_ClearDiskFlags_Other( pCtl->usDiskSel,
                                 LDV_FS_SELECTED | LDV_FS_CONTEXT |
                                 LPV_FS_CONTEXT,
                                 pCtl );
    }

    // Finish populating the notification structure
    notify.usDisk      = pCtl->usDiskSel;
    notify.usPartition = pCtl->usPartSel;

    // (we have to jump through a few hoops to get the disk/partition handles)
    wp.fsStatus  = WPM_CTLDATA;
    wp.cbCtlData = sizeof( DVCTLDATA );
    wp.pCtlData  = (PDVCTLDATA) &dvdata;
    WinSendMsg( notify.hwndDisk, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 );
    notify.disk = dvdata.handle;

    wp.fsStatus  = WPM_CTLDATA;
    wp.cbCtlData = sizeof( PVCTLDATA );
    wp.pCtlData  = (PPVCTLDATA) &pvdata;
    WinSendMsg( notify.hwndPartition, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), 0 );
    notify.partition = pvdata.handle;

    /* OK, now we can send ourselves a "disk selected" notification.  This will
     * ensure that our owner receives the proper notification code.
     */
    WinSendMsg( hwnd, WM_CONTROL,
                MPFROM2SHORT( pCtl->id, LDN_SELECT ), MPFROMP( &notify ));
}



// ***************************************************************************
// PRIVATE FUNCTIONS RELATED TO THE WC_VOLUMEINFO CONTROL
// ***************************************************************************


/* ------------------------------------------------------------------------- *
 * Window procedure for the LVMVolumeInfo control.                           *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY VIDisplayProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PVOLINFOPRIVATE pPrivate;    // pointer to private control data
    PVICTLDATA      pPublic;     // pointer to public control data
    PVIVOLUMEINFO   pInfo;       // set volume info structure
    HWND            hwndOwner;   // handle of our owner
    HPS             hps;         // presentation space handle
    POINTL          ptl;         // current drawing position
    RECTL           rcl;         // window area
    LONG            clrBG;       // current background colour
    ULONG           ulID,        // found pres-param ID
                    rc;          // return code
    USHORT          fsMask;      // new mask


    switch( msg ) {

        case WM_CREATE:
            if ( !mp1 ) return (MRESULT) TRUE;
            pPublic = (PVICTLDATA) mp1;

            // initialize the private control data structure
            pPrivate = (PVOLINFOPRIVATE) calloc( 1, sizeof( VOLINFOPRIVATE ));
            if ( !pPrivate ) return (MRESULT) TRUE;

            pPrivate->ctldata.cb        = pPublic->cb;
            pPrivate->ctldata.ulCnrID   = pPublic->ulCnrID;
            pPrivate->ctldata.fsStyle   = pPublic->fsStyle;
#ifdef VICTLDATA_STRINGS
            pPrivate->ctldata.pszFSL     = strdup( pPublic->pszFSL );
            pPrivate->ctldata.pszSizeL   = strdup( pPublic->pszSizeL );
            pPrivate->ctldata.pszDeviceL = strdup( pPublic->pszDeviceL );
#endif
            pPrivate->lIcon = WinQuerySysValue( HWND_DESKTOP, SV_CYICON );
            pPrivate->lDPI  = GetCurrentDPI( hwnd );
            pPrivate->id    = ((PCREATESTRUCT)mp2)->id;
            WinSetWindowPtr( hwnd, 0, pPrivate );

            // create the child controls
            pPrivate->hwndLetter = WinCreateWindow(
                                     hwnd, WC_STATIC, "", SS_TEXT | WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndName   = WinCreateWindow(
                                     hwnd, WC_STATIC, "", SS_TEXT | WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndType   = WinCreateWindow(
                                     hwnd, WC_STATIC, "", SS_TEXT | WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndFSL    = WinCreateWindow(
                                     hwnd, WC_STATIC, "",
                                     SS_TEXT | WS_VISIBLE, 0, 0, 0, 0, hwnd,
                                     HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndFSV    = WinCreateWindow(
                                     hwnd, WC_STATIC, "", SS_TEXT | WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndDevL   = WinCreateWindow(
                                     hwnd, WC_STATIC, "",
                                     SS_TEXT | WS_VISIBLE, 0, 0, 0, 0, hwnd,
                                     HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndDevV   = WinCreateWindow(
                                     hwnd, WC_STATIC, "", SS_TEXT | WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndSizeL  = WinCreateWindow(
                                     hwnd, WC_STATIC, "",
                                     SS_TEXT | WS_VISIBLE, 0, 0, 0, 0, hwnd,
                                     HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndSizeV  = WinCreateWindow(
                                     hwnd, WC_STATIC, "", SS_TEXT | WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, HWND_TOP, -1, NULL, NULL
                                   );
            pPrivate->hwndCnr    = WinCreateWindow(
                                     hwnd, WC_CONTAINER, "", WS_VISIBLE |
                                     CCS_MINIRECORDCORE | CCS_READONLY |
                                     CCS_SINGLESEL, 0, 0, 0, 0, hwnd, HWND_TOP,
                                     pPublic->ulCnrID, NULL, NULL
                                   );

#ifdef VICTLDATA_STRINGS
            WinSetWindowText( pPrivate->hwndFSL,    pPrivate->ctldata.pszFSL );
            WinSetWindowText( pPrivate->hwndSizeL,  pPrivate->ctldata.pszSizeL );
            WinSetWindowText( pPrivate->hwndDevL,   pPrivate->ctldata.pszDeviceL );
#endif

            // lay out the various components
            VI_Size( hwnd, pPrivate );

            return (MRESULT) FALSE;


        case WM_CONTROL:
            hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
            pPrivate  = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) break;

            // simply pass notifications from the container up to our owner
            if ( SHORT1FROMMP( mp1 ) == pPrivate->ctldata.ulCnrID )
                WinSendMsg( hwndOwner, WM_CONTROL, mp1, mp2 );
            break;


        case WM_DESTROY:
            if (( pPrivate = WinQueryWindowPtr( hwnd, 0 )) != NULL ) {
                if ( pPrivate->hwndLetter ) WinDestroyWindow( pPrivate->hwndLetter );
                if ( pPrivate->hwndName   ) WinDestroyWindow( pPrivate->hwndName   );
                if ( pPrivate->hwndType   ) WinDestroyWindow( pPrivate->hwndType   );
                if ( pPrivate->hwndFSL    ) WinDestroyWindow( pPrivate->hwndFSL    );
                if ( pPrivate->hwndFSV    ) WinDestroyWindow( pPrivate->hwndFSV    );
                if ( pPrivate->hwndSizeL  ) WinDestroyWindow( pPrivate->hwndSizeL  );
                if ( pPrivate->hwndSizeV  ) WinDestroyWindow( pPrivate->hwndSizeV  );
                if ( pPrivate->hwndDevL   ) WinDestroyWindow( pPrivate->hwndSizeL  );
                if ( pPrivate->hwndDevV   ) WinDestroyWindow( pPrivate->hwndSizeV  );
                if ( pPrivate->hwndCnr    ) WinDestroyWindow( pPrivate->hwndCnr    );
#ifdef VICTLDATA_STRINGS
                free( pPrivate->ctldata.pszFSL     );
                free( pPrivate->ctldata.pszSizeL   );
                free( pPrivate->ctldata.pszDeviceL );
#endif
                free( pPrivate );
            }
            break;


        case WM_PAINT:
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            GpiCreateLogColorTable( hps, 0, LCOLF_RGB, 0, 0, NULL );
            rc = WinQueryPresParam( hwnd, PP_BACKGROUNDCOLORINDEX, PP_BACKGROUNDCOLOR,
                                    &ulID, sizeof(clrBG), &clrBG, QPF_ID1COLORINDEX );
            if ( !rc ) clrBG = SYSCLR_WINDOW;
            else if ( ulID == PP_BACKGROUNDCOLORINDEX )
                clrBG = GpiQueryRGBColor( hps, 0, clrBG );
            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, clrBG );
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( pPrivate ) {
                // paint the icon, if one is defined
                if ( pPrivate->hptrIcon )
                    WinDrawPointer( hps, pPrivate->rclIcon.xLeft,
                                    pPrivate->rclIcon.yBottom,
                                    pPrivate->hptrIcon, DP_NORMAL );

                // paint the divider
                GpiSetColor( hps, ( pPrivate->ctldata.fsStyle & VIV_FS_3DSEP ) ?
                                  SYSCLR_BUTTONDARK : SYSCLR_WINDOWSTATICTEXT );
                ptl.x = pPrivate->rclDivider.xLeft;
                ptl.y = pPrivate->rclDivider.yTop;
                GpiMove( hps, &ptl );
                ptl.x = pPrivate->rclDivider.xRight-1;
                ptl.y = pPrivate->rclDivider.yTop;
                GpiLine( hps, &ptl );
                GpiSetColor( hps, ( pPrivate->ctldata.fsStyle & VIV_FS_3DSEP ) ?
                                  SYSCLR_BUTTONLIGHT : SYSCLR_WINDOW );
                ptl.x = pPrivate->rclDivider.xLeft;
                ptl.y = pPrivate->rclDivider.yTop-1;
                GpiMove( hps, &ptl );
                ptl.x = pPrivate->rclDivider.xRight-1;
                ptl.y = pPrivate->rclDivider.yTop-1;
                GpiLine( hps, &ptl );
            }
            WinEndPaint( hps );
            return (MRESULT) 0;


        case WM_PRESPARAMCHANGED:
            switch ( (ULONG) mp1 ) {
                case PP_FONTNAMESIZE:
                    pPrivate = WinQueryWindowPtr( hwnd, 0 );
                    if ( !pPrivate ) break;
                    VI_Size( hwnd, pPrivate );
                    WinInvalidateRect( hwnd, NULL, TRUE );
                    break;

                case PP_BACKGROUNDCOLOR:
                case PP_BACKGROUNDCOLORINDEX:
                case PP_FOREGROUNDCOLOR:
                case PP_FOREGROUNDCOLORINDEX:
                    // Update the container with the new window colour
                    pPrivate = WinQueryWindowPtr( hwnd, 0 );
                    if ( pPrivate && pPrivate->hwndCnr ) {
                        rc = WinQueryPresParam( hwnd, (ULONG) mp1, 0, NULL,
                                                sizeof( clrBG ), &clrBG, 0 );
                        if ( rc )
                            WinSetPresParam( pPrivate->hwndCnr, (ULONG) mp1,
                                             sizeof( clrBG ), &clrBG );
                    }
                    WinInvalidateRect( hwnd, NULL, TRUE );
                    break;

                default: break;
            }
            break;


        case WM_SIZE:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) break;
            VI_Size( hwnd, pPrivate );
            WinInvalidateRect( hwnd, NULL, TRUE );
            return (MRESULT) 0;


        // --------------------------------------------------------------------
        // Custom messages defined for this control
        //

        /* .................................................................. *
         * VIM_SETINFO                                                        *
         * - mp1 (PVIVOLUMEINFO): Pointer to volume information               *
         * - mp2:                 Unused, should be 0.                        *
         * Returns (BOOL) TRUE on success, FALSE on failure                   *
         * .................................................................. */
        case VIM_SETINFO:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate ) return (MRESULT) FALSE;
            pInfo = (PVIVOLUMEINFO) mp1;
            if ( !pInfo ) return (MRESULT) FALSE;

            pPrivate->hptrIcon = pInfo->hIcon;
            WinSetWindowText( pPrivate->hwndLetter, pInfo->pszLetter );
            WinSetWindowText( pPrivate->hwndName,   pInfo->pszName );
            WinSetWindowText( pPrivate->hwndType,   pInfo->pszType );
            WinSetWindowText( pPrivate->hwndFSV,    pInfo->pszFS );
            WinSetWindowText( pPrivate->hwndSizeV,  pInfo->pszSize );
            WinSetWindowText( pPrivate->hwndDevV,   pInfo->pszDevice );
#ifndef VICTLDATA_STRINGS
            WinSetWindowText( pPrivate->hwndFSL,    pInfo->pszFSL );
            WinSetWindowText( pPrivate->hwndSizeL,  pInfo->pszSizeL );
            WinSetWindowText( pPrivate->hwndDevL,   pInfo->pszDeviceL );
#endif
            WinInvalidateRect( hwnd, NULL, TRUE );
            return (MRESULT) TRUE;


        /* .................................................................. *
         * VIM_SETSTYLE                                                       *
         *  - mp1 (USHORT) : New style flags                                  *
         *  - mp2          : Unused, should be 0                              *
         *  Returns 0.                                                        *
         * .................................................................. */
        case VIM_SETSTYLE:
            fsMask = (USHORT) mp1;
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( pPrivate && ( pPrivate->ctldata.fsStyle != fsMask )) {
                pPrivate->ctldata.fsStyle = (USHORT) mp1;
                WinInvalidateRect( hwnd, NULL, TRUE );
            }
            return (MRESULT) 0;


        /* .................................................................. *
         * VIM_GETCONTAINERHWND                                               *
         *  - mp1 : Unused, should be 0.                                      *
         *  - mp2 : Unused, should be 0.                                      *
         * Returns (HWND) the window handle of the partition container        *
         * .................................................................. */
        case VIM_GETCONTAINERHWND:
            pPrivate = WinQueryWindowPtr( hwnd, 0 );
            if ( !pPrivate )
                return (MRESULT) NULLHANDLE;
            return (MRESULT) pPrivate->hwndCnr;


        default: break;
    }

    return ( WinDefWindowProc( hwnd, msg, mp1, mp2 ));
}


/* ------------------------------------------------------------------------- *
 * VI_Size                                                                   *
 *                                                                           *
 * (Re)size the volume info panel and all its contents.                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND            hwnd: Window handle of control.                         *
 *   PVOLINFOPRIVATE pCtl: Pointer to control's internal state data.         *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void VI_Size( HWND hwnd, PVOLINFOPRIVATE pCtl )
{
    FONTMETRICS fm;                     // current font metrics
    HPS         hps;                    // control's presentation space
    RECTL       rcl;                    // client bounds
    LONG        x, y,                   // positioning coordinates
                cx, cy;                 // size coordinates
    SWP         aswp[ 10 ];             // array of window positions
    CHAR        szFont[ FACESIZE+4 ];   // current font pres-param


    hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
    WinQueryWindowRect( hwnd, &rcl );
    SetFontFromPP( hps, hwnd, pCtl->lDPI );

    if ( WinQueryPresParam( hwnd, PP_FONTNAMESIZE, 0, NULL,
                            sizeof( szFont ), szFont, 0 ) > 0 )
    {
        GetBoldFontPP( hps, szFont );
        WinSetPresParam( pCtl->hwndLetter, PP_FONTNAMESIZE,
                         strlen( szFont ) + 1, (PVOID) szFont );
        WinSetPresParam( pCtl->hwndName, PP_FONTNAMESIZE,
                         strlen( szFont ) + 1, (PVOID) szFont );
    }

    GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm );

    // icon (not a window but painted directly, so just update the rectangle)
    x = 6;
    y = rcl.yTop - 6 - pCtl->lIcon;
    pCtl->rclIcon.xLeft   = x;
    pCtl->rclIcon.xRight  = x + pCtl->lIcon;
    pCtl->rclIcon.yBottom = y;
    pCtl->rclIcon.yTop    = x + pCtl->lIcon;

    // letter field
    x += pCtl->lIcon + 10;
    cx = rcl.xRight - x - 4;
    cy = 1 + fm.lExternalLeading + fm.lInternalLeading + fm.lMaxBaselineExt;
    y = rcl.yTop - 6 - cy;
    aswp[ 0 ].hwnd = pCtl->hwndLetter;
    aswp[ 0 ].hwndInsertBehind = HWND_TOP;
    aswp[ 0 ].x = x;
    aswp[ 0 ].y = y;
    aswp[ 0 ].cx = cx;
    aswp[ 0 ].cy = cy;
    aswp[ 0 ].fl = SWP_SIZE | SWP_MOVE;

    // name field
    y -= cy;
    aswp[ 1 ].hwnd = pCtl->hwndName;
    aswp[ 1 ].hwndInsertBehind = HWND_TOP;
    aswp[ 1 ].x = x;
    aswp[ 1 ].y = y;
    aswp[ 1 ].cx = cx;
    aswp[ 1 ].cy = cy;
    aswp[ 1 ].fl = SWP_SIZE | SWP_MOVE;

    // divider (not a window but painted directly, so just update the rectangle)
    x  = 6;
    cx = rcl.xRight - ( 2 * x );
    y -= fm.lMaxBaselineExt / 2;
    pCtl->rclDivider.xLeft = x;
    pCtl->rclDivider.xRight = x + cx;
    pCtl->rclDivider.yTop = y;
    pCtl->rclDivider.yBottom = y - 2;

    // volume type field
    y = pCtl->rclDivider.yBottom - cy - ( fm.lMaxBaselineExt / 2 );
    aswp[ 2 ].hwnd = pCtl->hwndType;
    aswp[ 2 ].hwndInsertBehind = HWND_TOP;
    aswp[ 2 ].x = x;
    aswp[ 2 ].y = y;
    aswp[ 2 ].cx = cx;
    aswp[ 2 ].cy = cy;
    aswp[ 2 ].fl = SWP_SIZE | SWP_MOVE;

    // filesystem label field
    cx = cx / 3;
    y -= cy;
    aswp[ 3 ].hwnd = pCtl->hwndFSL;
    aswp[ 3 ].hwndInsertBehind = HWND_TOP;
    aswp[ 3 ].x = x;
    aswp[ 3 ].y = y;
    aswp[ 3 ].cx = cx;
    aswp[ 3 ].cy = cy;
    aswp[ 3 ].fl = SWP_SIZE | SWP_MOVE;

    // filesystem value field
    x += cx;
    aswp[ 4 ].hwnd = pCtl->hwndFSV;
    aswp[ 4 ].hwndInsertBehind = HWND_TOP;
    aswp[ 4 ].x = x;
    aswp[ 4 ].y = y;
    aswp[ 4 ].cx = cx * 2;
    aswp[ 4 ].cy = cy;
    aswp[ 4 ].fl = SWP_SIZE | SWP_MOVE;

    // device label field
    x  = 6;
    y -= cy;
    aswp[ 5 ].hwnd = pCtl->hwndDevL;
    aswp[ 5 ].hwndInsertBehind = HWND_TOP;
    aswp[ 5 ].x = x;
    aswp[ 5 ].y = y;
    aswp[ 5 ].cx = cx;
    aswp[ 5 ].cy = cy;
    aswp[ 5 ].fl = SWP_SIZE | SWP_MOVE;

    // filesystem value field
    x += cx;
    aswp[ 6 ].hwnd = pCtl->hwndDevV;
    aswp[ 6 ].hwndInsertBehind = HWND_TOP;
    aswp[ 6 ].x = x;
    aswp[ 6 ].y = y;
    aswp[ 6 ].cx = cx * 2;
    aswp[ 6 ].cy = cy;
    aswp[ 6 ].fl = SWP_SIZE | SWP_MOVE;

    // size label field
    x  = 6;
    y -= cy;
    aswp[ 7 ].hwnd = pCtl->hwndSizeL;
    aswp[ 7 ].hwndInsertBehind = HWND_TOP;
    aswp[ 7 ].x = x;
    aswp[ 7 ].y = y;
    aswp[ 7 ].cx = cx;
    aswp[ 7 ].cy = cy;
    aswp[ 7 ].fl = SWP_SIZE | SWP_MOVE;

    // size value field
    x += cx;
    aswp[ 8 ].hwnd = pCtl->hwndSizeV;
    aswp[ 8 ].hwndInsertBehind = HWND_TOP;
    aswp[ 8 ].x = x;
    aswp[ 8 ].y = y;
    aswp[ 8 ].cx = cx * 2;
    aswp[ 8 ].cy = cy;
    aswp[ 8 ].fl = SWP_SIZE | SWP_MOVE;

    // partition container
    x  = 6;
    cx = rcl.xRight - ( 2 * x );
    cy = y - 4 - fm.lMaxDescender;
    y  = 4;
    aswp[ 9 ].hwnd = pCtl->hwndCnr;
    aswp[ 9 ].hwndInsertBehind = HWND_TOP;
    aswp[ 9 ].x = x;
    aswp[ 9 ].y = y;
    aswp[ 9 ].cx = cx;
    aswp[ 9 ].cy = cy;
    aswp[ 9 ].fl = SWP_SIZE | SWP_MOVE;

    WinSetMultWindowPos( WinQueryAnchorBlock( hwnd ), aswp, 10 );
    WinEndPaint( hps );
}

