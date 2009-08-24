#define INCL_VIO
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <process.h>

#include <uconv.h>

#include "ft2lib.h"

#include "kshell.h"
#include "viodmn.h"
#include "viosub.h"

#ifdef DEBUG
#include <stdio.h>

#define dprintf( ... ) \
{\
    FILE *fp;\
    fp = fopen("kshell.log", "at");\
    fprintf( fp, __VA_ARGS__ );\
    fclose( fp );\
}
#else
#define dprintf( ... )
#endif

#define PRF_APP         "KSHELL"
#define PRF_KEY_CP      "CODEPAGE"
#define PRF_KEY_FONT    "FONT"
#define PRF_KEY_SIZE    "SIZE"
#define PRF_KEY_HEIGHT  "HEIGHT"
#define PRF_KEY_WIDTH   "WIDTH"
#define PRF_KEY_FT2LIB  "FT2LIB"

#define DEFAULT_CODEPAGE    0
#define DEFAULT_FONT_FACE   "GulimChe"
#define DEFAULT_CHAR_PTS    12
#define DEFAULT_CHAR_HEIGHT 16
#define DEFAULT_CHAR_WIDTH  8

#define BASE_TITLE      "KShell"

static VIOMODEINFO  m_vmi;

#define VIO_CISIZE      ( sizeof( USHORT ) * 2 + sizeof( VIOCURSORINFO ))

#define MAX_CP_NAME     12      // maximum length of a codepage name
#define MAX_CP_SPEC     64      // maximum length of a UconvObject codepage specifier

static ATOM     m_cfUnicode;    // atom for "text/unicode" clipboard format

static FATTRS   m_fat;
static FIXED    m_fxPointSize;
static LONG     m_lHoriFontRes;
static LONG     m_lVertFontRes;
static LONG     m_lCharHeight;
static LONG     m_lMaxDescender;

static PBYTE    m_pVioBuf = NULL;
static CHAR     m_szPipeName[ PIPE_VIODMN_LEN ];
static CHAR     m_szPid[ 20 ];

static HEV      m_hevVioDmn;

static PID      m_pidVioDmn;
static ULONG    m_sidVioDmn;

static BOOL     m_afDBCSLeadByte[ 256 ] = { FALSE, };
static BOOL     m_fDBCSEnv = FALSE;

#define MAX_XCHARS  132

static CHAR     m_achXChar[ MAX_XCHARS ];
static POINTL   m_aptlPos[ MAX_XCHARS + 1 ];
static LONG     m_alXInc[ MAX_XCHARS ];

#define X_Vio2Win( xVio ) ( m_aptlPos[ xVio ].x )
#define GetCharWidth( x ) ( m_alXInc[ x ])

#define isDBCSEnv() ( m_fDBCSEnv )

#ifndef min
#define min( a, b ) (( a ) < ( b ) ? ( a ) : ( b ))
#endif

#ifndef max
#define max( a, b ) (( a ) > ( b ) ? ( a ) : ( b ))
#endif

static ULONG    m_ulSGID = ( ULONG )-1;
static HPIPE    m_hpipeVioSub = NULLHANDLE;
static TID      m_tidPipeThread = 0;

static BOOL     m_fFt2LibLoaded = FALSE;
static BOOL     m_fUseFt2Lib = FALSE;

PFNENABLEFONTENGINE     ksEnableFontEngine;
PFNSETCOLOR             ksSetColor;
PFNSETBACKCOLOR         ksSetBackColor;
PFNCHARSTRINGPOSAT      ksCharStringPosAt;
PFNQUERYCHARSTRINGPOSAT ksQueryCharStringPosAt;
PFNCREATELOGFONT        ksCreateLogFont;
PFNSETCHARSET           ksSetCharSet;
PFNSETCHARBOX           ksSetCharBox;
PFNQUERYFONTMETRICS     ksQueryFontMetrics;
PFNBEGINPAINT           ksBeginPaint;
PFNENDPAINT             ksEndPaint;
PFNGETPS                ksGetPS;
PFNGETSCREENPS          ksGetScreenPS;
PFNRELEASEPS            ksReleasePS;

static BOOL init( VOID );
static VOID done( VOID );

static VOID initDBCSEnv( USHORT usCP );
static VOID initFrame( HWND hwndFrame );

#define     isDBCSLeadByte( uch ) ( m_afDBCSLeadByte[( BYTE )( uch )])

static BOOL callVioDmn( USHORT usMsg );

static BOOL startVioDmn( VOID );
static VOID waitVioDmn( VOID );

static VOID initPipeThreadForVioSub( HWND hwnd );
static VOID donePipeThreadForVioSub( VOID );

static VOID updateWindow( HWND hwnd, PRECTL prcl );

static VOID initScrollBackMode( HWND hwnd );
static VOID doneScrollBackMode( HWND hwnd );

static VOID initMarkingMode( HWND hwnd );
static VOID doneMarkingMode( HWND hwnd );

static VOID invertRect( HPS hps, PPOINTS pptsStart, PPOINTS pptsEnd, PPOINTS pptsEndNew );

static VOID insertCBText( HWND hwnd, PSZ pszCBText );
static VOID copyFromClipbrd( HWND hwnd );
static VOID copyToClipbrd( HWND hwnd, BOOL fAll );

static VOID setFontMode( VOID );

static MRESULT EXPENTRY windowProc( HWND, ULONG, MPARAM, MPARAM );

INT main( VOID )
{
    HAB      hab;
    HMQ      hmq;
    ULONG    flFrameFlags;
    HWND     hwndFrame;
    HWND     hwndClient;
    QMSG     qm;
    HATOMTBL hSATbl;        // handle to system atom table

    int     result = 0;

    init();

    if( !startVioDmn())
    {
        result = 1;
        goto main_exit;
    }

    waitVioDmn();

    if( callVioDmn( MSG_VIOINFO ))
    {
        result = 2;
        goto main_exit;
    }

    memcpy( &m_vmi, m_pVioBuf, sizeof( VIOMODEINFO ));

    if( callVioDmn( MSG_SGID ))
    {
        result = 3;
        goto main_exit;
    }

    memcpy( &m_ulSGID, m_pVioBuf, sizeof( ULONG ));

    hab = WinInitialize( 0 );

    hmq = WinCreateMsgQueue( hab, 0);

    WinRegisterClass(
        hab,
        WC_KSHELL,
        windowProc,
        CS_SIZEREDRAW,
        sizeof( PVOID )
    );

    flFrameFlags = FCF_SYSMENU | FCF_TITLEBAR | FCF_TASKLIST | FCF_DLGBORDER |
                   FCF_VERTSCROLL | FCF_ICON;

    hwndFrame = WinCreateStdWindow(
                HWND_DESKTOP,               // parent window handle
                WS_VISIBLE,                 // frame window style
                &flFrameFlags,              // window style
                WC_KSHELL,                  // class name
                BASE_TITLE,                 // window title
                0L,                         // default client style
                NULLHANDLE,                 // resource in exe file
                ID_KSHELL,                  // frame window id
                &hwndClient                 // client window handle
                );

    // assume not failing
    initPipeThreadForVioSub( hwndClient );

    initFrame( hwndFrame );

    // Register the Unicode clipboard format
    hSATbl      = WinQuerySystemAtomTable();
    m_cfUnicode = WinAddAtom( hSATbl, "text/unicode");

    while( WinGetMsg( hab, &qm, NULLHANDLE, 0, 0 ))
        WinDispatchMsg( hab, &qm );

    donePipeThreadForVioSub();

    // Deregister the Unicode clipboard format
    WinDeleteAtom( hSATbl, m_cfUnicode );

    WinDestroyWindow( hwndFrame );

    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );

main_exit:
    done();

    return result;
}

static VOID convertVio2Win( PRECTL prcl )
{
    prcl->xLeft = X_Vio2Win( prcl->xLeft );
    prcl->yBottom = ( m_vmi.row - prcl->yBottom - 1 ) * m_lCharHeight;
    prcl->xRight = X_Vio2Win( prcl->xRight + 1 );
    prcl->yTop = ( m_vmi.row - prcl->yTop ) * m_lCharHeight;
}

static int findXCol( int x )
{
    int min_key = 0;
    int max_key = m_vmi.col;
    int left = min_key;
    int right = max_key;
    int key = ( left + right ) / 2;

    while(( key > min_key && x < m_aptlPos[ key ].x ) ||
          ( key < max_key && x >= m_aptlPos[ key + 1 ].x ))
    {
        if( x < m_aptlPos[ key ].x )
            right = key - 1;
        else
            left = key + 1;

        key = ( left + right ) / 2;
    }

    return key;
}

#define X_Win2Vio( x ) findXCol( x )
#define Y_Win2Vio( y ) (( int )( m_vmi.row - (( y ) / m_lCharHeight ) - 1 ))

#define KSHELL_SCROLLBACK_LINES 200

#define KSM_NORMAL      0L
#define KSM_SCROLLBACK  1L
#define KSM_MARKING     2L

typedef struct tagKSHELLDATA
{
    USHORT  x;
    USHORT  y;
    VIOCURSORINFO ci;
    PVOID   pVioBuf;
    PVOID   pScrollBackBuf;
    PVOID   pMarkingBuf;
    USHORT  usBaseLineOfVioBuf;
    USHORT  usBaseLineOfScrollBackBuf;
    USHORT  usLastLineOfScrollBackBuf;
    ULONG   ulBufSize;
    ULONG   ulKShellMode;
    ULONG   ulKShellModePrev;
    BOOL    fMarking;
    BOOL    fUpdateInvertRect;
    POINTS  ptsStart;
    POINTS  ptsEnd;
    HWND    hwndPopup;
} KSHELLDATA, *PKSHELLDATA;

#define getPtrOfUpdateBuf( pKShellData ) \
    ( pKShellData->ulKShellMode == KSM_SCROLLBACK ? \
      (( PVOID )(( PUSHORT )( pKShellData->pScrollBackBuf ) + \
                            ( pKShellData->usBaseLineOfScrollBackBuf * m_vmi.col ))) : \
      pKShellData->ulKShellMode == KSM_MARKING ? \
      (( PVOID )( pKShellData->pMarkingBuf )) : \
      (( PVOID )(( PUSHORT )( pKShellData->pVioBuf ) + \
                            ( pKShellData->usBaseLineOfVioBuf * m_vmi.col ))))

#define getPtrOfVioBuf( pKShellData ) \
    (( PVOID )(( PUSHORT )( pKShellData->pVioBuf ) + \
                          ( pKShellData->usBaseLineOfVioBuf * m_vmi.col )))

static VOID moveBaseLineOfVioBuf( HWND hwnd, SHORT sLines )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );
    HWND        hwndVertScroll = WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_VERTSCROLL );

    pKShellData->usBaseLineOfVioBuf += sLines;

    if( pKShellData->ulKShellMode == KSM_NORMAL )
    {
        // use WinPostMsg() instead of WinSendMsg() because the latter cause system to hang on.
        WinPostMsg( hwndVertScroll, SBM_SETSCROLLBAR, MPFROMSHORT( pKShellData->usBaseLineOfVioBuf ), MPFROM2SHORT( 0, pKShellData->usBaseLineOfVioBuf ));
        WinPostMsg( hwndVertScroll, SBM_SETTHUMBSIZE, MPFROM2SHORT( m_vmi.row, pKShellData->usBaseLineOfVioBuf + m_vmi.row ), 0 );
    }
}

static VOID moveBaseLineOfVioBufTo( HWND hwnd, SHORT sTo )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );
    HWND        hwndVertScroll = WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_VERTSCROLL );

    pKShellData->usBaseLineOfVioBuf = sTo;
    if( pKShellData->ulKShellMode == KSM_NORMAL )
    {
        // use WinPostMsg() instead of WinSendMsg() because the latter cause system to hang on.
        WinPostMsg( hwndVertScroll, SBM_SETSCROLLBAR, MPFROMSHORT( pKShellData->usBaseLineOfVioBuf ), MPFROM2SHORT( 0, pKShellData->usBaseLineOfVioBuf ));
        WinPostMsg( hwndVertScroll, SBM_SETTHUMBSIZE, MPFROM2SHORT( m_vmi.row, pKShellData->usBaseLineOfVioBuf + m_vmi.row ), 0 );
    }
}

static VOID setCursor( HWND hwnd, BOOL fCreate )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );

    WinShowCursor( hwnd, FALSE );
    WinDestroyCursor( hwnd );

    if(( pKShellData->ci.attr != ( USHORT )-1 ) &&
       ( pKShellData->ulKShellMode == KSM_NORMAL ) &&
       ( WinQueryFocus( HWND_DESKTOP ) == hwnd ) &&
       fCreate )
    {
        USHORT usCellHeight = m_vmi.vres / m_vmi.row;
        USHORT usStart = m_lCharHeight * ( pKShellData->ci.yStart + 1 ) / usCellHeight;
        USHORT usEnd = m_lCharHeight * ( pKShellData->ci.cEnd + 1 ) / usCellHeight;

        WinCreateCursor( hwnd, X_Vio2Win( pKShellData->x ),
                               ( m_vmi.row - pKShellData->y - 1 ) * m_lCharHeight +
                               ( m_lCharHeight - usEnd ),
                               GetCharWidth( pKShellData->x ),
                               usEnd - usStart + 1,
                               CURSOR_FLASH,
                               NULL );
        WinShowCursor( hwnd, TRUE );
    }
}

static VOID setAttr( HPS hps, UCHAR uchAttr )
{
    static int aiColorTable[ 16 ] = {
            CLR_BLACK,
            CLR_DARKBLUE,
            CLR_DARKGREEN,
            CLR_DARKCYAN,
            CLR_DARKRED,
            CLR_DARKPINK,
            CLR_BROWN,
            CLR_PALEGRAY,
            CLR_DARKGRAY,
            CLR_BLUE,
            CLR_GREEN,
            CLR_CYAN,
            CLR_RED,
            CLR_PINK,
            CLR_YELLOW,
            CLR_WHITE
    };

    ksSetColor( hps, aiColorTable[ uchAttr & 0x0F ]);
    ksSetBackColor( hps, aiColorTable[ ( uchAttr & 0xF0 ) >> 4 ]);
}

static VOID drawCharStringPosAt( HAB hab, PKSHELLDATA pKShellData, HPS hps, USHORT usAttr, PPOINTL pptl, int xStart, int count, PCH pchBase )
{
    PCHAR   pch;
    int     xEnd = xStart + count;
    int     x;
    LONG    alXInc[ MAX_XCHARS ];
    PLONG   palXInc;
    RECTL   rcl;
    POINTS  ptsStart, ptsEnd;
    RECTL   rclMarking;

    setAttr( hps, usAttr );
    if( isDBCSEnv())
    {
        for( x = xStart, pch = pchBase; x < xEnd; x++, pch++ )
        {
            alXInc[ x ] = m_alXInc[ x ];
            if( isDBCSLeadByte( *pch ))
            {
                alXInc[ x ] += m_alXInc[ x + 1 ];
                x++, pch++;
                alXInc[ x ] = 0;
            }
        }

        palXInc = &alXInc[ xStart ];
    }
    else
        palXInc = &GetCharWidth( xStart );

    // Points on xRight and yTop are included.
    rcl.xLeft = pptl->x;
    rcl.yBottom = pptl->y - m_lMaxDescender;
    rcl.xRight = rcl.xLeft + ( X_Vio2Win( xEnd ) - X_Vio2Win( xStart )) - 1;
    rcl.yTop = rcl.yBottom + m_lCharHeight - 1;
    ksCharStringPosAt( hps, pptl, &rcl, CHS_OPAQUE | CHS_VECTOR, count, pchBase, palXInc );

    if( pKShellData->ulKShellMode == KSM_MARKING && pKShellData->fUpdateInvertRect )
    {
        rclMarking.xLeft = min( pKShellData->ptsStart.x, pKShellData->ptsEnd.x );
        rclMarking.yBottom = max( pKShellData->ptsStart.y, pKShellData->ptsEnd.y );
        rclMarking.xRight = max( pKShellData->ptsStart.x, pKShellData->ptsEnd.x );
        rclMarking.yTop = min( pKShellData->ptsStart.y, pKShellData->ptsEnd.y );
        convertVio2Win( &rclMarking );

        WinIntersectRect( hab, &rclMarking, &rcl, &rclMarking );

        if( !WinIsRectEmpty( hab, &rclMarking ))
        {
            ptsStart.x = X_Win2Vio( rclMarking.xLeft );
            ptsStart.y = Y_Win2Vio( rclMarking.yTop - 1 );
            ptsEnd.x = X_Win2Vio( rclMarking.xRight - 1 );
            ptsEnd.y = Y_Win2Vio( rclMarking.yBottom );
            invertRect( hps, &ptsStart, &ptsEnd, NULL );
        }
    }
}

VOID updateWindow( HWND hwnd, PRECTL prcl )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );

    HPS     hps;
    SIZEF   sizef;
    int     xStart, yStart;
    int     xEnd, yEnd;
    RECTL   rcl;

    if( !prcl )
    {
        prcl = &rcl;
        WinQueryWindowRect( hwnd, prcl );
    }

    if( WinIsRectEmpty( WinQueryAnchorBlock( hwnd ), prcl ))
        return;

    setCursor( hwnd, FALSE );

    hps = ksGetPS( hwnd );

    xStart = X_Win2Vio( prcl->xLeft );
    yStart = Y_Win2Vio( prcl->yTop - 1 );
    xEnd = X_Win2Vio( prcl->xRight - 1 );
    yEnd = Y_Win2Vio( prcl->yBottom );

    ksCreateLogFont( hps, NULL, 1L, &m_fat );

    ksSetCharSet( hps, 1L );

    sizef.cx = (( m_fxPointSize * m_lHoriFontRes / 72 ) + 0x10000L ) & -0x20000L; // nearest even size
    sizef.cy = m_fxPointSize * m_lVertFontRes / 72;

    ksSetCharBox( hps, &sizef );

    {
        int     x, y;
        int     xStart1;
        POINTL  ptl;
        PUSHORT pVioBufShell;
        PCH     pchBase, pch;
        USHORT  usAttr;
        USHORT  usLen;

        ptl.y = ( m_vmi.row - yStart - 1 ) * m_lCharHeight + m_lMaxDescender;
        for( y = yStart; y <= yEnd; y++ )
        {
            xStart1 = xStart;
            pVioBufShell = ( PUSHORT )getPtrOfUpdateBuf( pKShellData ) + y * m_vmi.col;
            if( isDBCSEnv())
            {

                for( x = 0; x < xStart1; x++, pVioBufShell++ )
                {
                    if( isDBCSLeadByte( LOUCHAR( *pVioBufShell )))
                    {
                        x++;
                        pVioBufShell++;
                    }
                }

                if( xStart1 < x ) // dbcs trail byte ?
                {
                    // to dbcs lead byte
                    xStart1--;
                    pVioBufShell -= 2;
                }
            }
            else
                pVioBufShell += xStart1;

            ptl.x = X_Vio2Win( xStart1 );

            pchBase = malloc(( xEnd - xStart1 + 1 ) + 1 + 1 ); // 1 for broken DBCS, 1 for null

            pch = pchBase;
            usAttr = HIUCHAR( *pVioBufShell );

            for( x = xStart1; x <= xEnd; x++ )
            {
                if( usAttr != HIUCHAR( *pVioBufShell ))
                {
                    usLen = pch - pchBase;
                    drawCharStringPosAt( WinQueryAnchorBlock( hwnd ), pKShellData,
                                         hps, usAttr, &ptl, x - usLen, usLen, pchBase );

                    ptl.x = X_Vio2Win( x );
                    pch = pchBase;
                    usAttr = HIUCHAR( *pVioBufShell );
                }

                if( isDBCSLeadByte( LOUCHAR( *pVioBufShell )))
                {
                    *pch++ = LOUCHAR( *pVioBufShell++ );
                    x++;
                }

                *pch = LOUCHAR( *pVioBufShell++ );
                if( *pch == 0 )
                    *pch = 0x20;
                pch++;
            }

            if( pch != pchBase )
                drawCharStringPosAt( WinQueryAnchorBlock( hwnd ), pKShellData,
                                     hps, usAttr, &ptl, xStart1, pch - pchBase, pchBase );

            free( pchBase );

            ptl.y -= m_lCharHeight;
        }
    }

    ksReleasePS( hps );

    setCursor( hwnd, TRUE );
}

static VOID scrollWindow( HWND hwnd, LONG lDx, LONG lDy, PRECTL prcl )
{
#if 1
    RECTL   rcl;

    WinScrollWindow( hwnd,
                     lDx,
                     lDy,
                     prcl,
                     prcl,
                     NULLHANDLE,
                     &rcl,
                     0 );

    updateWindow( hwnd, &rcl );
#else
    HPS     hps;
    POINTL  aptl[ 3 ];
    RECTL   rcl;

    if( !lDx && !lDy )
        return;

    if( !prcl )
    {
        prcl = &rcl;
        WinQueryWindowRect( hwnd, prcl );
    }

    // source
    aptl[ 2 ].x = prcl->xLeft;
    if( lDx < 0 )
        aptl[ 2 ].x -= lDx;

    aptl[ 2 ].y = prcl->yBottom;
    if( lDy < 0 )
        aptl[ 2 ].y -= lDy;

    // target
    aptl[ 0 ].x = prcl->xLeft;
    if( lDx > 0 )
        aptl[ 0 ].x += lDx;

    aptl[ 0 ].y = prcl->yBottom;
    if( lDy > 0 )
        aptl[ 0 ].y += lDy;

    aptl[ 1 ].x = prcl->xRight;
    if( lDx < 0 )
        aptl[ 1 ].x += lDx;

    aptl[ 1 ].y = prcl->yTop;
    if( lDy < 0 )
        aptl[ 1 ].y += lDy;

    hps = ksGetPS( hwnd );
    GpiBitBlt( hps, hps, 3, aptl, ROP_SRCCOPY, BBO_IGNORE );
    ksReleasePS( hps );

    if( lDx > 0 )
        prcl->xRight = prcl->xLeft + lDx;
    else if( lDx < 0 )
        prcl->xLeft = prcl->xRight + lDx;

    if( lDy > 0 )
        prcl->yTop = prcl->yBottom + lDy;
    else if( lDy < 0 )
        prcl->yBottom = prcl->yTop + lDy;

    updateWindow( hwnd, prcl );
#endif
}

VOID initScrollBackMode( HWND hwnd )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );
    CHAR        szTitle[ 50 ];

    strcpy( szTitle, "Scroll Back Mode : ");
    WinQueryWindowText( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_TITLEBAR ), sizeof( szTitle ), szTitle + strlen( szTitle ));
    WinSetWindowText( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_TITLEBAR ), szTitle );

    memcpy( pKShellData->pScrollBackBuf, pKShellData->pVioBuf, pKShellData->ulBufSize );
    pKShellData->usBaseLineOfScrollBackBuf = pKShellData->usBaseLineOfVioBuf;
    pKShellData->usLastLineOfScrollBackBuf = pKShellData->usBaseLineOfVioBuf;
    setCursor( hwnd, FALSE );

    pKShellData->ulKShellMode = KSM_SCROLLBACK;
}

VOID doneScrollBackMode( HWND hwnd )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );
    CHAR        szTitle[ 50 ];

    WinQueryWindowText( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_TITLEBAR ), sizeof( szTitle ), szTitle );
    WinSetWindowText( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_TITLEBAR ), strchr( szTitle, ':' ) + 2 );

    pKShellData->ulKShellMode = KSM_NORMAL;

    WinSendMsg( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_VERTSCROLL ),
                SBM_SETPOS,
                MPFROMSHORT( pKShellData->usBaseLineOfVioBuf ),
                0 );

    setCursor( hwnd, TRUE );

    updateWindow( hwnd, NULL );
}

VOID initMarkingMode( HWND hwnd )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );
    CHAR        szTitle[ 50 ];

    strcpy( szTitle, "Marking Mode : ");
    WinQueryWindowText( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_TITLEBAR ), sizeof( szTitle ), szTitle + strlen( szTitle ));
    WinSetWindowText( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_TITLEBAR ), szTitle );

    setCursor( hwnd, FALSE );

    memcpy( pKShellData->pMarkingBuf, getPtrOfUpdateBuf( pKShellData ), m_vmi.row * m_vmi.col * VIO_CELLSIZE );

    pKShellData->ulKShellModePrev = pKShellData->ulKShellMode;
    pKShellData->ulKShellMode = KSM_MARKING;
    pKShellData->fUpdateInvertRect = TRUE;
}

VOID doneMarkingMode( HWND hwnd )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );
    CHAR        szTitle[ 50 ];

    WinQueryWindowText( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_TITLEBAR ), sizeof( szTitle ), szTitle );
    WinSetWindowText( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_TITLEBAR ), strchr( szTitle, ':' ) + 2 );

    pKShellData->ulKShellMode = pKShellData->ulKShellModePrev;
    pKShellData->fUpdateInvertRect = FALSE;

    setCursor( hwnd, TRUE );

    updateWindow( hwnd, NULL );
}

VOID invertRect( HPS hps, PPOINTS pptsStart, PPOINTS pptsEnd, PPOINTS pptsEndNew )
{
    RECTL   rcl;

    rcl.xLeft = min( pptsStart->x, pptsEnd->x );
    rcl.yBottom = max( pptsStart->y, pptsEnd->y );
    rcl.xRight = max( pptsStart->x, pptsEnd->x );
    rcl.yTop = min( pptsStart->y, pptsEnd->y );
    convertVio2Win( &rcl );

    if( pptsEndNew )
    {
        HRGN    hrgn, hrgnNew;
        RECTL   rclNew;
        RGNRECT rgnrc;

        rclNew.xLeft = min( pptsStart->x, pptsEndNew->x );
        rclNew.yBottom = max( pptsStart->y, pptsEndNew->y );
        rclNew.xRight = max( pptsStart->x, pptsEndNew->x );
        rclNew.yTop = min( pptsStart->y, pptsEndNew->y );
        convertVio2Win( &rclNew );

        hrgn = GpiCreateRegion( hps, 1, &rcl );
        hrgnNew = GpiCreateRegion( hps, 1, &rclNew );

        GpiCombineRegion( hps, hrgn, hrgn, hrgnNew, CRGN_XOR );

        rgnrc.ircStart = 1;
        //rgnrc.crc = 0;
        rgnrc.ulDirection = RECTDIR_LFRT_TOPBOT;
        GpiQueryRegionRects( hps, hrgn, NULL, &rgnrc, NULL );

        if( rgnrc.crcReturned > 0 )
        {
            PRECTL  prcl;
            int     i;

            prcl = malloc( sizeof( RECTL ) * rgnrc.crcReturned );

            rgnrc.crc = rgnrc.crcReturned;

            GpiQueryRegionRects( hps, hrgn, NULL, &rgnrc, prcl );

            for( i = 0; i < rgnrc.crcReturned; i++ )
                WinInvertRect( hps, &prcl[ i ]);

            free( prcl );
        }

        GpiDestroyRegion( hps, hrgn );
        GpiDestroyRegion( hps, hrgnNew );
    }
    else
        WinInvertRect( hps, &rcl );
}

VOID insertCBText( HWND hwnd, PSZ pszCBText )
{
            USHORT  fsFlags;
            UCHAR   uchRepeat;
            UCHAR   uchScan;
            USHORT  usCh;
            USHORT  usVk;

            for( ; *pszCBText; pszCBText++ )
            {
                fsFlags = KC_CHAR;
                uchRepeat = 0;
                uchScan = 0;
                usVk = 0;

                usCh = *pszCBText;

                if( *pszCBText == '\r' && ( pszCBText[ 1 ] == '\n' ))
                {
                    // for Enter-key
                    fsFlags = KC_SCANCODE | KC_CHAR | KC_VIRTUALKEY;
                    uchRepeat = 1;
                    uchScan = 0x1C;
                    usVk = VK_ENTER;

                    pszCBText++;
                }
                else if( isDBCSLeadByte( *pszCBText ))
                    usCh |= MAKEUSHORT( usCh, *++pszCBText );

                WinPostMsg( hwnd, WM_CHAR,
                            MPFROMSH2CH( fsFlags, uchRepeat, uchScan ),
                            MPFROM2SHORT( usCh, usVk ));
            }
}


VOID copyFromClipbrd( HWND hwnd )
{
    HAB hab = WinQueryAnchorBlock( hwnd );
    PSZ pszCBText;

    UconvObject uconv;                      // conversion object
    UniChar     suCodepage[ MAX_CP_SPEC ],  // conversion specifier
                *psuCBText;                 // pointer into psuClipText
    ULONG       ulBufLen;
    APIRET      rc;

    if( WinOpenClipbrd( hab ))
    {

// Paste as Unicode text if available...
        if (( psuCBText = (UniChar *) WinQueryClipbrdData( hab, m_cfUnicode )) != NULL )
        {
            // Create the conversion object
            UniMapCpToUcsCp( m_fat.usCodePage, suCodepage, MAX_CP_NAME );
            UniStrcat( suCodepage, (UniChar *) L"@map=cdra,path=no");

            if (( rc = UniCreateUconvObject( suCodepage, &uconv )) == ULS_SUCCESS )
            {
                // Convert to the current codepage
                ulBufLen = ( UniStrlen(psuCBText) * 4 ) + 1;
                pszCBText = (PSZ) calloc( ulBufLen, sizeof(CHAR) );
                if (( rc = UniStrFromUcs( uconv, pszCBText, psuCBText, ulBufLen )) == ULS_SUCCESS )
                {
                    // Output the converted text
                    insertCBText( hwnd, pszCBText );
                }
#if DEBUG
                else dprintf("Failed to convert Unicode clipboard text: UniStrFromUcs() = %08X\n", rc );
#endif
                UniFreeUconvObject( uconv );
                free( pszCBText );

            }
#if DEBUG
            else dprintf("Failed to convert Unicode clipboard text: UniCreateUconvObject() = %08X\n", rc );
#endif
        }
// Done pasting Unicode

        else if(( pszCBText = ( PSZ )WinQueryClipbrdData( hab, CF_TEXT )) != 0 )
        {
            insertCBText( hwnd, pszCBText );
        }

        WinCloseClipbrd( hab );
    }
}

VOID copyToClipbrd( HWND hwnd, BOOL fAll )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );

    HAB     hab = WinQueryAnchorBlock( hwnd );

    if( WinOpenClipbrd( hab ))
    {
        int     xStart, yStart;
        int     xEnd, yEnd;
        int     x, y;
        int     xStart1;
        PUSHORT pVioBufShell;
        PCH     pchBase, pch;

        UconvObject uconv;                      // UCS-2 conversion object
        UniChar     suCodepage[ MAX_CP_SPEC ],  // conversion specifier
                    *psuCopyText,               // Unicode text to be copied
                    *psuShareMem,               // Unicode text in clipboard
                    *psuOffset;                 // pointer into psuCopyText
        PSZ         pszCopyText,                // plain text to be converted
                    pszOffset;                  // pointer into pszCopyText
        ULONG       ulBufLen;                   // length of copied string
        APIRET      rc;
        size_t      stIn,
                    stOut,
                    stSub;

        WinEmptyClipbrd( hab );

        if( fAll )
        {
            xStart = 0;
            yStart = 0;
            xEnd = m_vmi.col - 1;
            yEnd = m_vmi.row - 1;
        }
        else
        {
            xStart = min( pKShellData->ptsStart.x, pKShellData->ptsEnd.x );
            yStart = min( pKShellData->ptsStart.y, pKShellData->ptsEnd.y );
            xEnd = max( pKShellData->ptsStart.x, pKShellData->ptsEnd.x );
            yEnd = max(  pKShellData->ptsStart.y, pKShellData->ptsEnd.y );
        }

        DosAllocSharedMem(( PPVOID )&pchBase, NULL,
                          (( xEnd - xStart + 1 ) + 2 ) * ( yEnd - yStart + 1 ) + 1, // 2 for '\r' and '\n', 1 for null
                          fALLOCSHR );

        pch = pchBase;

        for( y = yStart; y <= yEnd; y++ )
        {
            xStart1 = xStart;
            pVioBufShell = ( PUSHORT )getPtrOfUpdateBuf( pKShellData ) + y * m_vmi.col;
            if( isDBCSEnv())
            {
                for( x = 0; x < xStart1; x++, pVioBufShell++ )
                {
                    if( isDBCSLeadByte( LOUCHAR( *pVioBufShell )))
                    {
                        x++;
                        pVioBufShell++;
                    }
                }

                if( xStart1 < x ) // dbcs trail byte ?
                {
                    *pch++ = 0x20;
                    xStart1 = x;
                }
            }
            else
                pVioBufShell += xStart1;

            for( x = xStart1; x <= xEnd; x++ )
            {
                if( isDBCSLeadByte( LOUCHAR( *pVioBufShell )))
                {
                    if( x < xEnd )
                        *pch++ = LOUCHAR( *pVioBufShell++ );
                    else
                    {
                        *pch++ = 0x20;
                        break;
                    }

                    x++;
                }

                *pch = LOUCHAR( *pVioBufShell++ );
                if( *pch == 0 )
                    *pch = 0x20;
                pch++;
            }

            // remove last contiguous spaces.
            for( ; pch > pchBase && *( pch - 1 ) == 0x20; pch-- );

            if( yStart != yEnd )
            {
                *pch++ = '\r';
                *pch++ = '\n';
            }
        }

        *pch = '\0';

        WinEmptyClipbrd( hab );

        ulBufLen = strlen( pchBase ) + 1;
        pszCopyText = (PSZ) calloc( ulBufLen, sizeof(CHAR) );
        strncpy( pszCopyText, pchBase, ulBufLen - 1 );

        WinSetClipbrdData( hab, ( ULONG )pchBase, CF_TEXT, CFI_POINTER );

// Now copy as "text/unicode" (UCS-2)
        if ( pszCopyText )
        {
            UniMapCpToUcsCp( m_fat.usCodePage, suCodepage, MAX_CP_NAME );
            UniStrcat( suCodepage, (UniChar *) L"@map=cdra,path=no");
            if (( rc = UniCreateUconvObject( suCodepage, &uconv )) == ULS_SUCCESS )
            {
                // Convert string to Unicode
                if (( psuCopyText = (UniChar *) calloc( ulBufLen, sizeof(UniChar) )) != NULL ) {
                    pszOffset = pszCopyText;
                    psuOffset = psuCopyText;
                    stIn  = ulBufLen - 1;
                    stOut = ulBufLen - 1;
                    stSub = 0;
                    if (( rc = UniUconvToUcs( uconv, (PPVOID) &pszOffset, &stIn, &psuOffset, &stOut, &stSub )) == ULS_SUCCESS )
                    {
                        // (CP850 erroneously maps U+0131 to the euro sign)
                        if ( m_fat.usCodePage == 850 )
                            while (( psuOffset = UniStrchr( psuCopyText, 0x0131 )) != NULL ) *psuOffset = 0xFFFD;

                        // Place the UCS-2 string on the clipboard as "text/unicode"
                        rc = DosAllocSharedMem( (PVOID) &psuShareMem, NULL, ulBufLen,
                                                PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE );
                        if ( rc == 0 ) {
                            UniStrncpy( psuShareMem, psuCopyText, ulBufLen - 1 );
                            if ( !WinSetClipbrdData( hab, (ULONG) psuShareMem, m_cfUnicode, CFI_POINTER )) {
#ifdef DEBUG
                                dprintf("[copyToClipbrd] Failed to copy Unicode text: error %08X from WinSetClipbrdData().\n",  WinGetLastError(hab) );
#endif
                            }
                        }
                    }
#ifdef DEBUG
                    else dprintf("[copyToClipbrd] Failed to convert text: UniStrToUcs() = %08X.\n", rc );
#endif
                    free( psuCopyText );
                }
                UniFreeUconvObject( uconv );
            }
#if DEBUG
            else dprintf("[copyToClipbrd] Failed to created conversion object %ls: UniCreateUconvObject() = %08X.\n", suCodepage, rc );
#endif
            free ( pszCopyText );
        }
// Done copying UCS-2

        WinCloseClipbrd( hab );
    }
}

VOID setFontMode( VOID )
{
    if( m_fFt2LibLoaded )
    {
        ksEnableFontEngine = getFt2EnableFontEngine();

        ksEnableFontEngine( m_fUseFt2Lib );
    }

    if( m_fFt2LibLoaded && m_fUseFt2Lib )
    {
        ksSetColor              = getFt2SetColor();
        ksSetBackColor          = getFt2SetBackColor();
        ksCharStringPosAt       = getFt2CharStringPosAt();
        ksQueryCharStringPosAt  = getFt2QueryCharStringPosAt();
        ksCreateLogFont         = getFt2CreateLogFont();
        ksSetCharSet            = getFt2SetCharSet();
        ksSetCharBox            = getFt2SetCharBox();
        ksQueryFontMetrics      = getFt2QueryFontMetrics();
        ksBeginPaint            = getFt2BeginPaint();
        ksEndPaint              = getFt2EndPaint();
        ksGetPS                 = getFt2GetPS();
        ksGetScreenPS           = getFt2GetScreenPS();
        ksReleasePS             = getFt2ReleasePS();
    }
    else
    {
        ksSetColor              = GpiSetColor;
        ksSetBackColor          = GpiSetBackColor;
        ksCharStringPosAt       = GpiCharStringPosAt;
        ksQueryCharStringPosAt  = GpiQueryCharStringPosAt;
        ksCreateLogFont         = GpiCreateLogFont;
        ksSetCharSet            = GpiSetCharSet;
        ksSetCharBox            = GpiSetCharBox;
        ksQueryFontMetrics      = GpiQueryFontMetrics;
        ksBeginPaint            = WinBeginPaint;
        ksEndPaint              = WinEndPaint;
        ksGetPS                 = WinGetPS;
        ksGetScreenPS           = WinGetScreenPS;
        ksReleasePS             = WinReleasePS;
    }
}

MRESULT EXPENTRY windowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );

    switch( msg )
    {
        case WM_CREATE :
        {
            PUSHORT pVioBuf;
            int     cells;
            int     i;

            pKShellData = ( PKSHELLDATA )malloc( sizeof( KSHELLDATA ));
            memset( pKShellData, 0, sizeof( KSHELLDATA ));

            WinSetWindowPtr( hwnd, 0, pKShellData );

            pKShellData->ulBufSize = ( KSHELL_SCROLLBACK_LINES + m_vmi.row ) * m_vmi.col * VIO_CELLSIZE;

            pKShellData->pVioBuf = malloc( pKShellData->ulBufSize );

            pVioBuf = pKShellData->pVioBuf;
            cells = pKShellData->ulBufSize / VIO_CELLSIZE;
            for( i = 0; i < cells; i++ )
                *pVioBuf++ = 0x720;

            pKShellData->pScrollBackBuf = malloc( pKShellData->ulBufSize );
            memset( pKShellData->pScrollBackBuf, 0, pKShellData->ulBufSize );

            pKShellData->pMarkingBuf = malloc( m_vmi.row * m_vmi.col * VIO_CELLSIZE );
            memset( pKShellData->pMarkingBuf, 0, m_vmi.row * m_vmi.col * VIO_CELLSIZE );

            if( callVioDmn( MSG_CURINFO ))
            {
                WinPostMsg( hwnd, WM_QUIT, 0, 0 );
                return 0;
            }

            memcpy( pKShellData, m_pVioBuf, VIO_CISIZE );

            pKShellData->hwndPopup = WinLoadMenu( hwnd, NULLHANDLE, ID_POPUP );
            return 0;
        }

        case WM_DESTROY :
        {
            free( pKShellData->pMarkingBuf );
            free( pKShellData->pScrollBackBuf );
            free( pKShellData->pVioBuf );
            free( pKShellData );

            WinSetWindowPtr( hwnd, 0, NULL );

            return 0;
        }

        case WM_CLOSE :
            WinPostMsg( hwnd, WM_QUIT, 0, 0 );

            return 0;

        case WM_SETFOCUS :
            setCursor( hwnd, ( BOOL )mp2 );
            break;
#if 0
        case WM_ERASEBACKGROUND :
        {
            RECTL   rcl;

            memcpy( &rcl, ( void * )mp2, sizeof( RECTL ));
            WinMapWindowPoints( WinQueryWindow( hwnd, QW_PARENT ), hwnd, ( PPOINTL )&rcl, 2 );
            //WinInvalidateRect( hwnd, &rcl, FALSE );
            updateWindow( hwnd, &rcl );
            return 0;
        }
#endif
        case WM_PAINT :
        {
            HPS     hps;
            RECTL   rcl;

            hps = ksBeginPaint( hwnd, NULLHANDLE, &rcl );
            ksEndPaint( hps );

            updateWindow( hwnd, &rcl );

            return 0;
        }

        case WM_TRANSLATEACCEL :
        {
            PQMSG pQmsg = ( PQMSG )mp1;

            if( pQmsg->msg == WM_CHAR )
                return 0;

            break;
        }

        case WM_CHAR :
        {
            PMPARAM pmp;
            BYTE    abKbdState[ 256 ];
            BYTE    abPhysKbdState[ 256 ];

            if( pKShellData->ulKShellMode == KSM_MARKING )
            {
                if(( SHORT1FROMMP( mp1 ) & KC_VIRTUALKEY) &&
                   !( SHORT1FROMMP( mp1 ) & ( KC_ALT | KC_CTRL | KC_SHIFT )) &&
                   !( SHORT1FROMMP( mp1 ) & KC_KEYUP ))
                {
                    switch( SHORT2FROMMP( mp2 ))
                    {
                        case VK_NEWLINE :
                            copyToClipbrd( hwnd, FALSE );

                        case VK_ESC :
                            doneMarkingMode( hwnd );
                            break;
                    }
                }

                return MRFROMLONG( TRUE );
            }

            if(( SHORT1FROMMP( mp1 ) & KC_VIRTUALKEY ) &&
               ( SHORT2FROMMP( mp2 ) >= 0x80 ) &&      // 0x80 for VK_DBE_FIRST
               ( SHORT2FROMMP( mp2 ) <= 0xFF ))        // 0xFF for VK_DBE_LAST
                return MRFROMLONG( TRUE );

            if(( pKShellData->ulKShellMode == KSM_SCROLLBACK ) &&
               !( SHORT1FROMMP( mp1 ) & KC_KEYUP ))
                doneScrollBackMode( hwnd );

            WinSetKeyboardStateTable( HWND_DESKTOP, abKbdState, FALSE );

            memset( abPhysKbdState, 0, sizeof( abPhysKbdState ));

            if( WinGetPhysKeyState( HWND_DESKTOP, PM_SC_LSHIFT ) & 0x8000 )
                abPhysKbdState[ PM_SC_LSHIFT ] = 0x80;

            if( WinGetPhysKeyState( HWND_DESKTOP, PM_SC_RSHIFT ) & 0x8000 )
                abPhysKbdState[ PM_SC_RSHIFT ] = 0x80;

            if( WinGetPhysKeyState( HWND_DESKTOP, PM_SC_LCTRL ) & 0x8000 )
                abPhysKbdState[ PM_SC_LCTRL ] = 0x80;

            if( WinGetPhysKeyState( HWND_DESKTOP, PM_SC_LALT ) & 0x8000 )
                abPhysKbdState[ PM_SC_LALT ] = 0x80;

            if( WinGetPhysKeyState( HWND_DESKTOP, PM_SC_RCTRL ) & 0x8000 )
                abPhysKbdState[ PM_SC_RCTRL ] = 0x80;

            if( WinGetPhysKeyState( HWND_DESKTOP, PM_SC_RALT ) & 0x8000 )
                abPhysKbdState[ PM_SC_RALT ] = 0x80;

            pmp = ( PMPARAM )m_pVioBuf;
            *pmp++ = mp1;
            *pmp++ = mp2;
            memcpy( pmp, abKbdState, sizeof( abKbdState ));
            memcpy((( PBYTE )pmp ) + sizeof( abKbdState ), abPhysKbdState, sizeof( abPhysKbdState ));

            if( callVioDmn( MSG_CHAR ))
                WinPostMsg( hwnd, WM_QUIT, 0, 0 );

            return MRFROMLONG( TRUE );
        }

        case WM_QUERYCONVERTPOS :
        {
            PRECTL  pCursorPos = ( PRECTL )mp1;

            pCursorPos->xRight = pCursorPos->xLeft = pKShellData->x;
            pCursorPos->yTop = pCursorPos->yBottom = pKShellData->y;

            convertVio2Win( pCursorPos );

            return MRFROMLONG( QCP_CONVERT );
        }

        case WM_COMMAND :
            switch( SHORT1FROMMP( mp1 ))
            {
                case IDM_CODEPAGE :
                {
                    HWND    hwndDlg;

                    hwndDlg = WinLoadDlg( HWND_DESKTOP, hwnd, WinDefDlgProc, NULLHANDLE, IDD_CODEPAGE, NULL );
                    if( hwndDlg )
                    {
                        HWND    hwndEF = WinWindowFromID( hwndDlg, IDEF_CODEPAGE );
                        CHAR    szCP[ 10 ];
                        ULONG   ulReply;

                        _itoa( m_fat.usCodePage, szCP, 10 );
                        WinSetWindowText( hwndEF, szCP );
                        WinSendMsg( hwndEF, EM_SETSEL, MPFROM2SHORT( 0, -1 ), 0 );

                        ulReply = WinProcessDlg( hwndDlg );

                        if( ulReply == DID_OK )
                        {

                            WinQueryWindowText( hwndEF, sizeof( szCP ), szCP );
                            m_fat.usCodePage = atoi( szCP );
                            initDBCSEnv( m_fat.usCodePage );

                            PrfWriteProfileString( HINI_USERPROFILE, PRF_APP, PRF_KEY_CP, szCP );

                            //WinInvalidateRect( hwnd, NULL, FALSE );
                            updateWindow( hwnd, NULL );
                        }

                        WinDestroyWindow( hwndDlg );
                    }

                    return 0;
                }

                case IDM_FONT :
                {
                    static FONTDLG fd;

                    HPS    hps;
                    HWND   hwndFontDlg;

                    memset( &fd, 0, sizeof( FONTDLG ));

                    hps = ksGetPS( hwnd );

                    fd.cbSize = sizeof(FONTDLG);
                    fd.hpsScreen = hps;

                    fd.pszFamilyname = m_fat.szFacename;
                    fd.usFamilyBufLen = sizeof( m_fat.szFacename );

                    fd.fxPointSize = m_fxPointSize;
                    fd.fl = FNTS_HELPBUTTON | FNTS_CENTER |
                            FNTS_FIXEDWIDTHONLY | FNTS_INITFROMFATTRS;
                    fd.clrFore = SYSCLR_WINDOWTEXT;
                    fd.clrBack = SYSCLR_WINDOW;

                    memcpy( &fd.fAttrs, &m_fat, sizeof( FATTRS ));

                    hwndFontDlg = WinFontDlg( HWND_DESKTOP, hwnd, &fd );

                    if( hwndFontDlg &&( fd.lReturn == DID_OK ))
                    {
                        CHAR    szNum[ 10 ];

                        memcpy( &m_fat, &fd.fAttrs, sizeof( FATTRS ));
                        m_fat.usCodePage = PrfQueryProfileInt( HINI_USERPROFILE, PRF_APP, PRF_KEY_CP, DEFAULT_CODEPAGE );

                        m_fxPointSize = fd.fxPointSize;

                        PrfWriteProfileString( HINI_USERPROFILE, PRF_APP, PRF_KEY_FONT, m_fat.szFacename );

                        _itoa( FIXEDINT( m_fxPointSize ), szNum, 10 );
                        PrfWriteProfileString( HINI_USERPROFILE, PRF_APP, PRF_KEY_SIZE, szNum );

                        _ltoa( m_fat.lMaxBaselineExt , szNum, 10 );
                        PrfWriteProfileString( HINI_USERPROFILE, PRF_APP, PRF_KEY_HEIGHT, szNum );

                        _ltoa( m_fat.lAveCharWidth, szNum, 10 );
                        PrfWriteProfileString( HINI_USERPROFILE, PRF_APP, PRF_KEY_WIDTH, szNum );

                        initFrame( WinQueryWindow( hwnd, QW_PARENT ));
                        //WinInvalidateRect( hwnd, NULL, FALSE );
                        //setCursor( hwnd, TRUE );
                        updateWindow( hwnd, NULL );
                    }

                    ksReleasePS( hps );

                    return 0;
                }

                case IDM_COPY :
                case IDM_COPYALL :
                    copyToClipbrd( hwnd, SHORT1FROMMP( mp1 ) == IDM_COPYALL  );
                    if( pKShellData->ulKShellMode == KSM_MARKING ) // 'Copy All' can be called not in Marking Mode
                        doneMarkingMode( hwnd );
                    return 0;

                case IDM_PASTE :
                    copyFromClipbrd( hwnd );
                    return 0;

                case IDM_FT2LIB :
                {
                    CHAR    szNum[ 10 ];

                    m_fUseFt2Lib = !m_fUseFt2Lib;

                    _ltoa( m_fUseFt2Lib, szNum, 10 );
                    PrfWriteProfileString( HINI_USERPROFILE, PRF_APP, PRF_KEY_FT2LIB, szNum );

                    initFrame( WinQueryWindow( hwnd, QW_PARENT ));
                    updateWindow( hwnd, NULL );

                    return 0;
                }
            }

        case WM_VSCROLL :
        {
            SHORT   sSlider = SHORT1FROMMP( mp2 );
            USHORT  uscmd = SHORT2FROMMP( mp2 );

            if( pKShellData->ulKShellMode == KSM_MARKING )
                return 0;

            switch( uscmd )
            {
                case SB_LINEUP :
                    if( pKShellData->usBaseLineOfVioBuf > 0 )
                    {
                        if( pKShellData->ulKShellMode == KSM_NORMAL )
                            initScrollBackMode( hwnd );

                        if( pKShellData->usBaseLineOfScrollBackBuf > 0 )
                        {
                            pKShellData->usBaseLineOfScrollBackBuf--;
                            WinSendMsg( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_VERTSCROLL ),
                                        SBM_SETPOS,
                                        MPFROMSHORT( pKShellData->usBaseLineOfScrollBackBuf ),
                                        0);

                            scrollWindow( hwnd, 0, -m_lCharHeight, NULL );
                        }
                    }
                    break;

                case SB_LINEDOWN :
                    if( pKShellData->ulKShellMode == KSM_SCROLLBACK )
                    {
                        if( pKShellData->usBaseLineOfScrollBackBuf + 1 == pKShellData->usLastLineOfScrollBackBuf )
                            doneScrollBackMode( hwnd );
                        else
                        {
                            pKShellData->usBaseLineOfScrollBackBuf++;
                            WinSendMsg( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_VERTSCROLL ),
                                        SBM_SETPOS,
                                        MPFROMSHORT( pKShellData->usBaseLineOfScrollBackBuf ),
                                        0);

                            scrollWindow( hwnd, 0, m_lCharHeight, NULL );
                        }
                    }
                    break;

                case SB_PAGEUP :
                    if( pKShellData->usBaseLineOfVioBuf > 0 )
                    {
                        if( pKShellData->ulKShellMode == KSM_NORMAL )
                            initScrollBackMode( hwnd );

                        if( pKShellData->usBaseLineOfScrollBackBuf > 0 )
                        {
                            if( pKShellData->usBaseLineOfScrollBackBuf < m_vmi.row )
                                pKShellData->usBaseLineOfScrollBackBuf = 0;
                            else
                                pKShellData->usBaseLineOfScrollBackBuf -= m_vmi.row;

                            WinSendMsg( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_VERTSCROLL ),
                                        SBM_SETPOS,
                                        MPFROMSHORT( pKShellData->usBaseLineOfScrollBackBuf ),
                                        0);

                            updateWindow( hwnd, NULL );
                        }
                    }
                    break;

                case SB_PAGEDOWN :
                    if( pKShellData->ulKShellMode == KSM_SCROLLBACK )
                    {
                        if( pKShellData->usBaseLineOfScrollBackBuf + m_vmi.row >= pKShellData->usLastLineOfScrollBackBuf )
                            doneScrollBackMode( hwnd );
                        else
                        {
                            pKShellData->usBaseLineOfScrollBackBuf  += m_vmi.row;
                            WinSendMsg( WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ), FID_VERTSCROLL ),
                                        SBM_SETPOS,
                                        MPFROMSHORT( pKShellData->usBaseLineOfScrollBackBuf ),
                                        0);
                            updateWindow( hwnd, NULL );
                        }
                    }
                    break;

                //case SB_SLIDERPOSITION :
                case SB_SLIDERTRACK :
                {
                    if( pKShellData->ulKShellMode == KSM_NORMAL )
                        initScrollBackMode( hwnd );

                    if( sSlider == pKShellData->usLastLineOfScrollBackBuf )
                        doneScrollBackMode( hwnd );
                    else
                    {
                        SHORT   sDelta = sSlider - pKShellData->usBaseLineOfScrollBackBuf;

                        if( sDelta != 0 )
                        {
                            pKShellData->usBaseLineOfScrollBackBuf += sDelta;

                            scrollWindow( hwnd, 0, sDelta * m_lCharHeight, NULL );
                        }
                    }
                    break;
                }
            }

            return 0;
        }

        case WM_BUTTON1DOWN :
        case WM_BUTTON2DOWN :
        {
            if(( WinGetKeyState( HWND_DESKTOP, VK_BUTTON1 ) & 0x8000 ) &&
               ( WinGetKeyState( HWND_DESKTOP, VK_BUTTON2 ) & 0x8000 ))
            {
                copyFromClipbrd( hwnd );

                return MRFROMLONG( TRUE );
            }
            break;
        }

        case WM_BUTTON2CLICK :
        {
            POINTS  pts;
            ULONG   fs = PU_NONE | PU_KEYBOARD | PU_MOUSEBUTTON1 |
                         PU_HCONSTRAIN | PU_VCONSTRAIN;

            WinEnableMenuItem( pKShellData->hwndPopup, IDM_COPY,
                               pKShellData->ulKShellMode == KSM_MARKING );

            WinEnableMenuItem( pKShellData->hwndPopup, IDM_FT2LIB, m_fFt2LibLoaded );
            WinCheckMenuItem( pKShellData->hwndPopup, IDM_FT2LIB, m_fUseFt2Lib );

            pts.x = SHORT1FROMMP( mp1 );
            pts.y = SHORT2FROMMP( mp1 );

            WinPopupMenu( hwnd, hwnd, pKShellData->hwndPopup, pts.x, pts.y, 0, fs );

            return MRFROMLONG( TRUE );
        }

        case WM_BUTTON1MOTIONSTART :
        {
            HPS hps;

            if( pKShellData->ulKShellMode == KSM_MARKING )
            {
                RECTL rcl;

                rcl.xLeft = min( pKShellData->ptsStart.x, pKShellData->ptsEnd.x );
                rcl.yBottom = max( pKShellData->ptsStart.y, pKShellData->ptsEnd.y );
                rcl.xRight = max( pKShellData->ptsStart.x, pKShellData->ptsEnd.x );
                rcl.yTop = min( pKShellData->ptsStart.y, pKShellData->ptsEnd.y );
                convertVio2Win( &rcl );

                pKShellData->fUpdateInvertRect = FALSE;
                updateWindow( hwnd, &rcl );
                pKShellData->fUpdateInvertRect = TRUE;
            }
            else
                initMarkingMode( hwnd );

            pKShellData->ptsEnd.x = pKShellData->ptsStart.x = X_Win2Vio( SHORT1FROMMP( mp1 ));
            pKShellData->ptsEnd.y = pKShellData->ptsStart.y = Y_Win2Vio( SHORT2FROMMP( mp1 ));

            pKShellData->fMarking = TRUE;

            hps = ksGetPS( hwnd );
            invertRect( hps, &pKShellData->ptsStart, &pKShellData->ptsEnd, NULL );
            ksReleasePS( hps );

            WinSetCapture( HWND_DESKTOP, hwnd );

            return MRFROMLONG( TRUE );
        }

        case WM_BUTTON1MOTIONEND :
        {
            WinSetCapture( HWND_DESKTOP, NULLHANDLE );

            pKShellData->fMarking = FALSE;

            return MRFROMLONG( TRUE );
        }

        case WM_MOUSEMOVE :
            if( pKShellData->fMarking )
            {
                HPS    hps;
                POINTS ptsEndNew;
                RECTL  rcl;

                ptsEndNew.x = SHORT1FROMMP( mp1 );
                ptsEndNew.y = SHORT2FROMMP( mp1 );

                WinQueryWindowRect( hwnd, &rcl );

                if( ptsEndNew.x < 0 )
                    ptsEndNew.x = 0;

                if( ptsEndNew.x >= rcl.xRight )
                    ptsEndNew.x = rcl.xRight - 1;

                if( ptsEndNew.y < 0 )
                    ptsEndNew.y = 0;

                if( ptsEndNew.y >= rcl.yTop )
                    ptsEndNew.y = rcl.yTop - 1;

                ptsEndNew.x = X_Win2Vio( ptsEndNew.x );
                ptsEndNew.y = Y_Win2Vio( ptsEndNew.y );

                hps = ksGetPS( hwnd );
                invertRect( hps, &pKShellData->ptsStart, &pKShellData->ptsEnd, &ptsEndNew );
                ksReleasePS( hps );

                pKShellData->ptsEnd.x = ptsEndNew.x;
                pKShellData->ptsEnd.y = ptsEndNew.y;
            }
            break;

        case KSHELLM_INITFRAME :
            initFrame( WinQueryWindow( hwnd, QW_PARENT ));
            updateWindow( hwnd, NULL );
            return 0;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

BOOL init( VOID )
{
    PPIB    ppib;
    CHAR    szSem[ SEM_KSHELL_VIODMN_LEN ];
    CHAR    szMem[ MEM_KSHELL_VIOBUF_LEN ];
    LONG    lResult;
    HPS     hps;
    HDC     hdc;

    DosGetInfoBlocks( NULL, &ppib );

    _ultoa( ppib->pib_ulpid, m_szPid, 16 );

    strcpy( szMem, MEM_KSHELL_VIOBUF_BASE );
    strcat( szMem, m_szPid );

    DosAllocSharedMem(( PPVOID )&m_pVioBuf, szMem, KSHELL_BUFSIZE, fALLOC );

    strcpy( m_szPipeName, PIPE_VIODMN_BASE );
    strcat( m_szPipeName, m_szPid );

    strcpy( szSem, SEM_KSHELL_VIODMN_BASE );
    strcat( szSem, m_szPid );

    DosCreateEventSem( szSem, &m_hevVioDmn, DC_SEM_SHARED, 0 );

    srand( time( NULL ));

    PrfQueryProfileString( HINI_USERPROFILE, PRF_APP, PRF_KEY_FONT, DEFAULT_FONT_FACE, m_fat.szFacename, FACESIZE );

    m_fat.usRecordLength = sizeof( FATTRS );
    m_fat.fsSelection = 0;
    m_fat.lMatch = 0L;
    m_fat.idRegistry = 0;
    m_fat.usCodePage = PrfQueryProfileInt( HINI_USERPROFILE, PRF_APP, PRF_KEY_CP, DEFAULT_CODEPAGE );

    lResult = PrfQueryProfileInt( HINI_USERPROFILE, PRF_APP, PRF_KEY_HEIGHT, DEFAULT_CHAR_HEIGHT );
    m_fat.lMaxBaselineExt = lResult ? lResult : DEFAULT_CHAR_HEIGHT;

    lResult = PrfQueryProfileInt( HINI_USERPROFILE, PRF_APP, PRF_KEY_WIDTH, DEFAULT_CHAR_WIDTH );
    m_fat.lAveCharWidth = lResult ? lResult : DEFAULT_CHAR_WIDTH;

    m_fat.fsType = FATTR_TYPE_MBCS | FATTR_TYPE_DBCS;
    m_fat.fsFontUse = FATTR_FONTUSE_NOMIX;

    lResult = PrfQueryProfileInt( HINI_USERPROFILE, PRF_APP, PRF_KEY_SIZE, DEFAULT_CHAR_PTS );
    m_fxPointSize = MAKEFIXED( lResult ? lResult : DEFAULT_CHAR_PTS, 0 );

    initDBCSEnv( m_fat.usCodePage );

    m_fFt2LibLoaded = loadFt2Lib();
    lResult = PrfQueryProfileInt( HINI_USERPROFILE, PRF_APP, PRF_KEY_FT2LIB, FALSE );
    m_fUseFt2Lib = lResult ? lResult : FALSE;

    setFontMode();

    hps = ksGetScreenPS( HWND_DESKTOP );
    hdc = GpiQueryDevice( hps );
    DevQueryCaps( hdc, CAPS_HORIZONTAL_FONT_RES, 1, &m_lHoriFontRes );
    DevQueryCaps( hdc, CAPS_VERTICAL_FONT_RES, 1, &m_lVertFontRes );
    ksReleasePS( hps );

    return TRUE;
}

VOID done( VOID )
{
    freeFt2Lib();

    DosCloseEventSem( m_hevVioDmn );

    DosFreeMem( m_pVioBuf );
}

VOID initDBCSEnv( USHORT usCP )
{
    COUNTRYCODE cc;
    UCHAR       uchDBCSInfo[ 12 ];
    int         i, j;

    m_fDBCSEnv = FALSE;
    memset( m_afDBCSLeadByte, 0, sizeof( m_afDBCSLeadByte ));

    cc.country = 0;
    cc.codepage = usCP ;

    if( DosQueryDBCSEnv( sizeof( uchDBCSInfo ), &cc, uchDBCSInfo ) == 0 )
    {
        for( i = 0; uchDBCSInfo[ i ] != 0 || uchDBCSInfo[ i + 1 ] != 0; i += 2 )
        {
            for( j = uchDBCSInfo[ i ]; j <= uchDBCSInfo[ i + 1 ]; j++ )
            {
                m_afDBCSLeadByte[ j ] = TRUE;
                m_fDBCSEnv = TRUE;
            }
        }
    }
}

static VOID randomizeRect( PRECTL prcl )
{
    LONG    cxScreen = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    LONG    cyScreen = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    LONG    newX, newY;

    newX = ( rand() % ( cxScreen - ( prcl->xRight - prcl->xLeft )));
    newY = ( rand() % ( cyScreen - ( prcl->yTop - prcl->yBottom )));

    prcl->xRight = newX + ( prcl->xRight - prcl->xLeft );
    prcl->yTop = newY + ( prcl->yTop - prcl->yBottom );

    prcl->xLeft = newX;
    prcl->yBottom = newY;

}

VOID initFrame( HWND hwndFrame )
{
    static BOOL fInitFirst = TRUE;

    HWND        hwndSysMenu;
    HWND        hwndClient;
    MENUITEM    mi;
    HPS         hps;
    SIZEF       sizef;
    POINTL      ptlStart = { 0, 0 };
    FONTMETRICS fm;
    RECTL       rcl;
    int         i;

    if( fInitFirst )
    {
        hwndSysMenu = WinWindowFromID( hwndFrame, FID_SYSMENU );
        WinSendMsg( hwndSysMenu, MM_QUERYITEM, MPFROM2SHORT( SC_SYSMENU, FALSE ), ( MPARAM )&mi );

        hwndSysMenu = mi.hwndSubMenu;
        WinSendMsg( hwndSysMenu, MM_QUERYITEM, MPFROM2SHORT( SC_CLOSE, FALSE ), ( MPARAM )&mi );

        mi.iPosition += 2;
        mi.afStyle = MIS_TEXT;
        mi.afAttribute = 0;
        mi.id = IDM_CODEPAGE;
        mi.hwndSubMenu = NULLHANDLE;
        mi.hItem = 0;
        WinSendMsg( hwndSysMenu, MM_INSERTITEM, ( MPARAM )&mi, ( MPARAM )"Codepage..." );

        mi.iPosition++;
        mi.afStyle = MIS_TEXT;
        mi.afAttribute = 0;
        mi.id = IDM_FONT;
        mi.hwndSubMenu = NULLHANDLE;
        mi.hItem = 0;
        WinSendMsg( hwndSysMenu, MM_INSERTITEM, ( MPARAM )&mi, ( MPARAM )"Font..." );

        mi.iPosition++;
        mi.afStyle = MIS_SEPARATOR;
        mi.afAttribute = 0;
        mi.id = 0;
        mi.hwndSubMenu = NULLHANDLE;
        mi.hItem = 0;
        WinSendMsg( hwndSysMenu, MM_INSERTITEM, ( MPARAM )&mi, 0 );

        WinEnableWindow( WinWindowFromID( hwndFrame, FID_VERTSCROLL ), FALSE );

        memset( m_achXChar, 'k', sizeof( m_achXChar ));
    }

    setFontMode();

    hwndClient = WinWindowFromID( hwndFrame, FID_CLIENT );

    hps = ksGetPS( hwndClient );

    ksCreateLogFont( hps, NULL, 1L, &m_fat );
    ksSetCharSet( hps, 1L );

    sizef.cx = (( m_fxPointSize * m_lHoriFontRes / 72 ) + 0x10000L ) & -0x20000L; // nearest even size
    sizef.cy = m_fxPointSize * m_lVertFontRes / 72;

    ksSetCharBox( hps, &sizef );

    ksQueryCharStringPosAt( hps, &ptlStart, 0, MAX_XCHARS, m_achXChar, NULL, m_aptlPos );
    // workaround for FT2LIB. First position can be non-zero, maybe 1
    if( m_fFt2LibLoaded && m_fUseFt2Lib )
        m_aptlPos[ 0 ].x = 0;
    for( i = 0; i < MAX_XCHARS; i++ )
        m_alXInc[ i ] = m_aptlPos[ i + 1 ].x - m_aptlPos[ i ].x;

    ksQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fm );

    m_lCharHeight = fm.lMaxBaselineExt + fm.lExternalLeading;
    m_lMaxDescender = fm.lMaxDescender;

    ksReleasePS( hps );

    WinQueryWindowRect( hwndClient, &rcl );
    WinMapWindowPoints( hwndClient, HWND_DESKTOP, ( PPOINTL )&rcl, 2 );
    rcl.xRight = rcl.xLeft + X_Vio2Win( m_vmi.col );
    rcl.yTop = rcl.yBottom + m_lCharHeight * m_vmi.row;
    WinCalcFrameRect( hwndFrame, &rcl, FALSE );

    if( fInitFirst )
    {
        randomizeRect( &rcl );
        fInitFirst = FALSE;
    }
    else
    {
        RECTL rclFrame;

        WinQueryWindowRect( hwndFrame, &rclFrame );
        WinMapWindowPoints( hwndFrame, HWND_DESKTOP, ( PPOINTL)&rclFrame, 2 );

        WinOffsetRect( WinQueryAnchorBlock( hwndFrame ), &rcl, 0, rclFrame.yTop - rcl.yTop );
    }

    WinSetWindowPos( hwndFrame, HWND_TOP,
                     rcl.xLeft, rcl.yBottom,
                     rcl.xRight - rcl.xLeft,
                     rcl.yTop - rcl.yBottom,
                     SWP_SIZE | SWP_MOVE | SWP_ZORDER | SWP_SHOW | SWP_ACTIVATE );
}

BOOL    callVioDmn( USHORT usMsg )
{
    static  BOOL fQuit = FALSE;

    USHORT  usAck;
    ULONG   cbActual;

    ULONG   rc;

    if( fQuit )
        return TRUE;

    do
    {
        rc = DosCallNPipe( m_szPipeName,
                           &usMsg, sizeof( usMsg ),
                           &usAck, sizeof( usAck ), &cbActual,
                           10000L );

        if( rc == ERROR_PIPE_BUSY )
            while( DosWaitNPipe( m_szPipeName, -1 ) == ERROR_INTERRUPT );

        if( rc )
            DosSleep( 1 );

    } while( rc );

    if( usAck == MSG_QUIT )
        fQuit = TRUE;

    return fQuit;
}

BOOL startVioDmn( VOID )
{
    STARTDATA   stdata;
    CHAR        szInput[ 256 ];
    CHAR        ObjectBuffer[ 256 ];

    strcpy( szInput, m_szPid );
    strcat( szInput, " " );
    strcat( szInput, VIODMN_MAGIC );

    stdata.Length = sizeof( stdata );
    stdata.Related = SSF_RELATED_INDEPENDENT;
    stdata.FgBg = SSF_FGBG_BACK;
    stdata.TraceOpt = SSF_TRACEOPT_NONE;
    stdata.PgmTitle = "VIO Daemon";
    stdata.PgmName = "VIODMN.EXE";
    stdata.PgmInputs = szInput;
    stdata.TermQ = 0;
    stdata.Environment = 0;
    stdata.InheritOpt = SSF_INHERTOPT_PARENT;
    stdata.SessionType = SSF_TYPE_FULLSCREEN;
    stdata.IconFile = 0;
    stdata.PgmHandle = 0;
    stdata.PgmControl = SSF_CONTROL_INVISIBLE;
    stdata.ObjectBuffer = ObjectBuffer;
    stdata.ObjectBuffLen = sizeof( ObjectBuffer );

    return ( DosStartSession( &stdata, &m_sidVioDmn, &m_pidVioDmn ) == 0 );
}

VOID waitVioDmn( VOID )
{
    while( DosWaitEventSem( m_hevVioDmn, SEM_INDEFINITE_WAIT ) == ERROR_INTERRUPT );
}

#define isFullRect( tr, lc, br, rc ) ((( tr ) == 0 ) && (( lc ) == 0 ) && \
                                      (( br ) == m_vmi.row - 1 ) && (( rc ) == m_vmi.col -1 ))

static VOID pipeThread( void *arg )
{
    HWND        hwnd = ( HWND )arg;
    PKSHELLDATA pKShellData = WinQueryWindowPtr( hwnd, 0 );

    CHAR szSem[ SEM_VIODMN_KSHELL_LEN ];
    HEV  hev = 0;

    USHORT usIndex;
    ULONG  cbActual;

    strcpy( szSem, SEM_VIODMN_KSHELL_BASE );
    strcat( szSem, m_szPid );

    DosOpenEventSem( szSem, &hev );
    DosPostEventSem( hev );
    DosCloseEventSem( hev );

    do
    {
        DosConnectNPipe( m_hpipeVioSub );

        DosRead( m_hpipeVioSub, &usIndex, sizeof( USHORT ), &cbActual );

        switch( usIndex )
        {
            case VI_VIOSHOWBUF :
            {
                USHORT  usOfs;
                USHORT  usLen;
                USHORT  usRow;
                USHORT  usCol;
                INT     y;
                RECTL   rcl;

                DosRead( m_hpipeVioSub, &usOfs, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usLen, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, ( PCHAR )getPtrOfVioBuf( pKShellData ) + usOfs, usLen, &cbActual );

                if( pKShellData->ulKShellMode == KSM_NORMAL )
                {
                    usOfs /= VIO_CELLSIZE;
                    usLen /= VIO_CELLSIZE;
                    usRow = usOfs / m_vmi.col;
                    usCol = usOfs % m_vmi.col;

                    for( y = usRow; ( y < m_vmi.row ) && ( usLen > 0 ); y++ )
                    {
                        rcl.xLeft = usCol;
                        rcl.yBottom = rcl.yTop = y;
                        rcl.xRight = min( usCol + usLen, m_vmi.col ) - 1;
                        usLen -= rcl.xRight - rcl.xLeft + 1;
                        convertVio2Win( &rcl );
                        updateWindow( hwnd, &rcl );

                        usCol = 0;
                    }
                }
                break;
            }

            case VI_VIOSETCURPOS :
            {
                DosRead( m_hpipeVioSub, &pKShellData->x, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &pKShellData->y, sizeof( USHORT ), &cbActual );

                if( pKShellData->ulKShellMode == KSM_NORMAL )
                    setCursor( hwnd, TRUE );
                break;
            }

            case VI_VIOSETCURTYPE :
            {
                VIOCURSORINFO vci;
                USHORT usCellHeight = m_vmi.vres / m_vmi.row;

                DosRead( m_hpipeVioSub, &vci, sizeof( VIOCURSORINFO ), &cbActual );

                if(( SHORT )vci.yStart < 0 )
                    vci.yStart = ( usCellHeight * ( -( SHORT )vci.yStart ) + 99 ) / 100 - 1;

                if(( SHORT )vci.cEnd < 0 )
                    vci.cEnd = ( usCellHeight * ( -( SHORT )vci.cEnd ) + 99 ) / 100 - 1;

                if( vci.yStart >= usCellHeight )
                    vci.yStart = usCellHeight - 1;

                if( vci.cEnd > 31 )
                    vci.cEnd = 31;

                memcpy( &pKShellData->ci, &vci, sizeof( VIOCURSORINFO ));

                if( pKShellData->ulKShellMode == KSM_NORMAL )
                    setCursor( hwnd, TRUE );
                break;
            }

            case VI_VIOSETMODE :
            {
                HWND    hwndPopup;
                PUSHORT pVioBuf;
                int     cells;
                int     i;

                DosRead( m_hpipeVioSub, &m_vmi, sizeof( VIOMODEINFO ), &cbActual );

                free( pKShellData->pVioBuf );
                free( pKShellData->pScrollBackBuf );
                free( pKShellData->pMarkingBuf );

                hwndPopup = pKShellData->hwndPopup;

                memset( pKShellData, 0, sizeof( KSHELLDATA ));

                pKShellData->hwndPopup = hwndPopup;

                pKShellData->ulBufSize = ( KSHELL_SCROLLBACK_LINES + m_vmi.row ) * m_vmi.col * VIO_CELLSIZE;

                pKShellData->pVioBuf = malloc( pKShellData->ulBufSize );

                pVioBuf = pKShellData->pVioBuf;
                cells = pKShellData->ulBufSize / VIO_CELLSIZE;
                for( i = 0; i < cells; i++ )
                    *pVioBuf++ = 0x720;

                pKShellData->pScrollBackBuf = malloc( pKShellData->ulBufSize );
                memset( pKShellData->pScrollBackBuf, 0, pKShellData->ulBufSize );

                pKShellData->pMarkingBuf = malloc( m_vmi.row * m_vmi.col * VIO_CELLSIZE );
                memset( pKShellData->pMarkingBuf, 0, m_vmi.row * m_vmi.col * VIO_CELLSIZE );

                WinPostMsg( hwnd, KSHELLM_INITFRAME, 0, 0 );
                break;
            }

            case VI_VIOWRTNCHAR :
            {
                USHORT  usCol;
                USHORT  usRow;
                USHORT  usTimes;
                CHAR    ch;
                PCH     pBuf;
                INT     x, y;
                RECTL   rcl;

                DosRead( m_hpipeVioSub, &usCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usTimes, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &ch, sizeof( CHAR ), &cbActual );

                pBuf = ( PCH )(( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( usRow * m_vmi.col ) + usCol );
                for( y = usRow; ( y < m_vmi.row ) && ( usTimes > 0 ); y++ )
                {
                    for( x = usCol; ( x < m_vmi.col ) && ( usTimes > 0 ); x++, usTimes--, pBuf += VIO_CELLSIZE )
                    {
                       *pBuf = ch;
                    }

                    if( pKShellData->ulKShellMode == KSM_NORMAL )
                    {
                        rcl.xLeft = usCol;
                        rcl.yBottom = rcl.yTop = y;
                        rcl.xRight = x - 1;
                        convertVio2Win( &rcl );
                        updateWindow( hwnd, &rcl );
                    }

                    usCol = 0;
                }
                break;
            }

            case VI_VIOWRTNATTR :
            {
                USHORT  usCol;
                USHORT  usRow;
                USHORT  usTimes;
                BYTE    bAttr;
                PBYTE   pBuf;
                INT     x, y;
                RECTL   rcl;

                DosRead( m_hpipeVioSub, &usCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usTimes, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &bAttr, sizeof( BYTE ), &cbActual );

                pBuf = ( PBYTE )(( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( usRow * m_vmi.col ) + usCol ) + 1;
                for( y = usRow; ( y < m_vmi.row ) && ( usTimes > 0 ); y++ )
                {
                    for( x = usCol; ( x < m_vmi.col ) && ( usTimes > 0 ); x++, usTimes--, pBuf += VIO_CELLSIZE )
                    {
                       *pBuf = bAttr;
                    }

                    if( pKShellData->ulKShellMode == KSM_NORMAL )
                    {
                        rcl.xLeft = usCol;
                        rcl.yBottom = rcl.yTop = y;
                        rcl.xRight = x - 1;
                        convertVio2Win( &rcl );
                        updateWindow( hwnd, &rcl );
                    }

                    usCol = 0;
                }
                break;
            }

            case VI_VIOWRTNCELL :
            {
                USHORT  usCol;
                USHORT  usRow;
                USHORT  usTimes;
                USHORT  usCell;
                PUSHORT pBuf;
                INT     x, y;
                RECTL   rcl;

                DosRead( m_hpipeVioSub, &usCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usTimes, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usCell, sizeof( BYTE ) * VIO_CELLSIZE, &cbActual );

                pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( usRow * m_vmi.col ) + usCol;
                for( y = usRow; ( y < m_vmi.row ) && ( usTimes > 0 ); y++ )
                {
                    for( x = usCol; ( x < m_vmi.col ) && ( usTimes > 0 ); x++, usTimes-- )
                    {
                       *pBuf++ = usCell;
                    }

                    if( pKShellData->ulKShellMode == KSM_NORMAL )
                    {
                        rcl.xLeft = usCol;
                        rcl.yBottom = rcl.yTop = y;
                        rcl.xRight = x - 1;
                        convertVio2Win( &rcl );
                        updateWindow( hwnd, &rcl );
                    }

                    usCol = 0;
                }
                break;
            }

            case VI_VIOWRTCHARSTR :
            {
                USHORT  usCol;
                USHORT  usRow;
                USHORT  usLen;
                PCH     pchCharStr, pch;
                PUSHORT pBuf;
                INT     x, y;
                RECTL   rcl;

                DosRead( m_hpipeVioSub, &usCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usLen, sizeof( USHORT ), &cbActual );

                pchCharStr = malloc( usLen );
                DosRead( m_hpipeVioSub, pchCharStr, usLen, &cbActual );

                pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( usRow * m_vmi.col ) + usCol;
                pch = pchCharStr;
                for( y = usRow; ( y < m_vmi.row ) && ( usLen > 0 ); y++ )
                {
                    for( x = usCol; ( x < m_vmi.col ) && ( usLen > 0 ); x++, usLen--, pBuf++ )
                    {
                       *pBuf = MAKEUSHORT( *pch++, HIUCHAR( *pBuf ));
                    }

                    if( pKShellData->ulKShellMode == KSM_NORMAL )
                    {
                        rcl.xLeft = usCol;
                        rcl.yBottom = rcl.yTop = y;
                        rcl.xRight = x - 1;
                        convertVio2Win( &rcl );
                        updateWindow( hwnd, &rcl );
                    }

                    usCol = 0;
                }
                free( pchCharStr );
                break;
            }

            case VI_VIOWRTCHARSTRATT :
            {
                BYTE    bAttr;
                USHORT  usCol;
                USHORT  usRow;
                USHORT  usLen;
                PCH     pchCharStr, pch;
                PUSHORT pBuf;
                INT     x, y;
                RECTL   rcl;

                DosRead( m_hpipeVioSub, &bAttr, sizeof( BYTE ), &cbActual );
                DosRead( m_hpipeVioSub, &usCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usLen, sizeof( USHORT ), &cbActual );

                pchCharStr = malloc( usLen );
                DosRead( m_hpipeVioSub, pchCharStr, usLen, &cbActual );

                pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( usRow * m_vmi.col ) + usCol;
                pch = pchCharStr;
                for( y = usRow; ( y < m_vmi.row ) && ( usLen > 0 ); y++ )
                {
                    for( x = usCol; ( x < m_vmi.col ) && ( usLen > 0 ); x++, usLen-- )
                    {
                       *pBuf++ = MAKEUSHORT( *pch++, bAttr );
                    }

                    if( pKShellData->ulKShellMode == KSM_NORMAL )
                    {
                        rcl.xLeft = usCol;
                        rcl.yBottom = rcl.yTop = y;
                        rcl.xRight = x - 1;
                        convertVio2Win( &rcl );
                        updateWindow( hwnd, &rcl );
                    }

                    usCol = 0;
                }
                free( pchCharStr );
                break;
            }

            case VI_VIOWRTCELLSTR :
            {
                USHORT  usCol;
                USHORT  usRow;
                USHORT  usLen;
                PUSHORT pusCellStr, pusCell;
                PUSHORT pBuf;
                INT     x, y;
                RECTL   rcl;

                DosRead( m_hpipeVioSub, &usCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usLen, sizeof( USHORT ), &cbActual );

                pusCellStr = malloc( usLen );
                DosRead( m_hpipeVioSub, pusCellStr, usLen, &cbActual );

                usLen /= VIO_CELLSIZE;
                pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( usRow * m_vmi.col ) + usCol;
                pusCell = pusCellStr;
                for( y = usRow; ( y < m_vmi.row ) && ( usLen > 0 ); y++ )
                {
                    for( x = usCol; ( x < m_vmi.col ) && ( usLen > 0 ); x++, usLen-- )
                    {
                       *pBuf++ = *pusCell++;
                    }

                    if( pKShellData->ulKShellMode == KSM_NORMAL )
                    {
                        rcl.xLeft = usCol;
                        rcl.yBottom = rcl.yTop = y;
                        rcl.xRight = x - 1;
                        convertVio2Win( &rcl );
                        updateWindow( hwnd, &rcl );
                    }

                    usCol = 0;
                }
                free( pusCellStr );
                break;
            }

            case VI_VIOSCROLLUP :
            {
                USHORT  usCell;
                USHORT  usLines;
                USHORT  usRightCol;
                USHORT  usBottomRow;
                USHORT  usLeftCol;
                USHORT  usTopRow;

                PUSHORT pBuf;
                INT     x, y;

                DosRead( m_hpipeVioSub, &usCell, sizeof( BYTE ) * 2, &cbActual );
                DosRead( m_hpipeVioSub, &usLines, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRightCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usBottomRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usLeftCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usTopRow, sizeof( USHORT ), &cbActual );

                if( usLeftCol >= m_vmi.col )
                    usLeftCol = m_vmi.col - 1;

                if( usRightCol >= m_vmi.col )
                    usRightCol = m_vmi.col - 1;

                if( usBottomRow >= m_vmi.row )
                    usBottomRow = m_vmi.row - 1;

                if( usTopRow >= m_vmi.row )
                    usTopRow = m_vmi.row - 1;

                if( usLines >= ( usBottomRow - usTopRow + 1 ))
                    usLines = usBottomRow - usTopRow + 1;

                if( isFullRect( usTopRow, usLeftCol, usBottomRow, usRightCol ))
                {
                    if( pKShellData->usBaseLineOfVioBuf + usLines > KSHELL_SCROLLBACK_LINES ) // Scroll buf full ?
                    {
                        pBuf = ( PUSHORT )pKShellData->pVioBuf + usLines * m_vmi.col;
                        memmove( pKShellData->pVioBuf, pBuf, ( KSHELL_SCROLLBACK_LINES + m_vmi.row - usLines ) * m_vmi.col * VIO_CELLSIZE );
                        moveBaseLineOfVioBufTo( hwnd, KSHELL_SCROLLBACK_LINES );
                    }
                    else
                        moveBaseLineOfVioBuf( hwnd, usLines );
                }
                else
                {
                    for( y = usTopRow; y <= usBottomRow - usLines; y++ )
                    {
                        pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( y * m_vmi.col ) + usLeftCol;
                        memmove( pBuf, pBuf + ( usLines * m_vmi.col ), ( usRightCol - usLeftCol + 1 ) * VIO_CELLSIZE );
                    }
                }

                for( y = usBottomRow - usLines + 1; y <= usBottomRow; y++ )
                {
                    pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( y * m_vmi.col ) + usLeftCol;
                    for( x = usLeftCol; x <= usRightCol; x++ )
                        *pBuf++ = usCell;
                }

                if( pKShellData->ulKShellMode == KSM_NORMAL )
                {
                    RECTL   rcl;

                    rcl.xLeft = usLeftCol;
                    rcl.yBottom = usBottomRow;
                    rcl.xRight = usRightCol;
                    rcl.yTop = usTopRow;
                    convertVio2Win( &rcl );

                    setCursor( hwnd, FALSE );

                    scrollWindow( hwnd, 0, usLines * m_lCharHeight, &rcl );

                    setCursor( hwnd, TRUE );
                }
                break;

            }

            case VI_VIOSCROLLDN :
            {
                USHORT  usCell;
                USHORT  usLines;
                USHORT  usRightCol;
                USHORT  usBottomRow;
                USHORT  usLeftCol;
                USHORT  usTopRow;

                PUSHORT pBuf;
                BOOL    fFullRect;
                BOOL    fClearScreen = FALSE;
                INT     x, y;

                DosRead( m_hpipeVioSub, &usCell, sizeof( BYTE ) * 2, &cbActual );
                DosRead( m_hpipeVioSub, &usLines, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRightCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usBottomRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usLeftCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usTopRow, sizeof( USHORT ), &cbActual );

                if( usLeftCol >= m_vmi.col )
                    usLeftCol = m_vmi.col - 1;

                if( usRightCol >= m_vmi.col )
                    usRightCol = m_vmi.col - 1;

                if( usBottomRow >= m_vmi.row )
                    usBottomRow = m_vmi.row - 1;

                if( usTopRow >= m_vmi.row )
                    usTopRow = m_vmi.row - 1;

                fFullRect = isFullRect( usTopRow, usLeftCol, usBottomRow, usRightCol );

                if( usLines >= ( usBottomRow - usTopRow + 1 ))
                {
                    usLines = usBottomRow - usTopRow + 1;
                    fClearScreen = fFullRect;
                }

                if( !fClearScreen )
                {
                    for( y = usBottomRow; y >= usTopRow + usLines; y-- )
                    {
                        pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( y * m_vmi.col ) + usLeftCol;
                        memmove( pBuf, pBuf - ( usLines * m_vmi.col ), ( usRightCol - usLeftCol + 1 ) * VIO_CELLSIZE );
                    }
                }
                else
                {
                    if( pKShellData->usBaseLineOfVioBuf + m_vmi.row > KSHELL_SCROLLBACK_LINES ) // Scroll buf full ?
                    {
                        pBuf = ( PUSHORT )pKShellData->pVioBuf + m_vmi.row * m_vmi.col;
                        memmove( pKShellData->pVioBuf, pBuf, ( KSHELL_SCROLLBACK_LINES + m_vmi.row - m_vmi.row ) * m_vmi.col * VIO_CELLSIZE );

                        moveBaseLineOfVioBufTo( hwnd, KSHELL_SCROLLBACK_LINES );
                    }
                    else
                        moveBaseLineOfVioBuf( hwnd, m_vmi.row );
                }

                for( y = usTopRow; y < usTopRow + usLines; y++ )
                {
                    pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( y * m_vmi.col ) + usLeftCol;
                    for( x = usLeftCol; x <= usRightCol; x++ )
                        *pBuf++ = usCell;
                }

                if( pKShellData->ulKShellMode == KSM_NORMAL )
                {
                    RECTL   rcl;

                    rcl.xLeft = usLeftCol;
                    rcl.yBottom = usBottomRow;
                    rcl.xRight = usRightCol;
                    rcl.yTop = usTopRow;
                    convertVio2Win( &rcl );

                    setCursor( hwnd, FALSE );

                    scrollWindow( hwnd, 0, -usLines * m_lCharHeight, &rcl );

                    setCursor( hwnd, TRUE );
                }
                break;
            }

            case VI_VIOSCROLLLF :
            {
                USHORT  usCell;
                USHORT  usCols;
                USHORT  usRightCol;
                USHORT  usBottomRow;
                USHORT  usLeftCol;
                USHORT  usTopRow;

                PUSHORT pBuf;
                BOOL    fFullRect;
                BOOL    fClearScreen = FALSE;
                INT     x, y;

                DosRead( m_hpipeVioSub, &usCell, sizeof( BYTE ) * 2, &cbActual );
                DosRead( m_hpipeVioSub, &usCols, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRightCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usBottomRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usLeftCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usTopRow, sizeof( USHORT ), &cbActual );

                if( usLeftCol >= m_vmi.col )
                    usLeftCol = m_vmi.col - 1;

                if( usRightCol >= m_vmi.col )
                    usRightCol = m_vmi.col - 1;

                if( usBottomRow >= m_vmi.row )
                    usBottomRow = m_vmi.row - 1;

                if( usTopRow >= m_vmi.row )
                    usTopRow = m_vmi.row - 1;

                fFullRect = isFullRect( usTopRow, usLeftCol, usBottomRow, usRightCol );

                if( usCols >= ( usBottomRow - usTopRow + 1 ))
                {
                    usCols = usBottomRow - usTopRow + 1;
                    fClearScreen = fFullRect;
                }

                if( !fClearScreen )
                {
                    for( y = usTopRow; y <= usBottomRow; y++ )
                    {
                        pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( y * m_vmi.col ) + usLeftCol;
                        memmove( pBuf, pBuf + usCols, ( usRightCol - usLeftCol + 1 - usCols ) * VIO_CELLSIZE );
                    }
                }
                else
                {
                    if( pKShellData->usBaseLineOfVioBuf + m_vmi.row > KSHELL_SCROLLBACK_LINES ) // Scroll buf full ?
                    {
                        pBuf = ( PUSHORT )pKShellData->pVioBuf + m_vmi.row * m_vmi.col;
                        memmove( pKShellData->pVioBuf, pBuf, ( KSHELL_SCROLLBACK_LINES + m_vmi.row - m_vmi.row ) * m_vmi.col * VIO_CELLSIZE );

                        moveBaseLineOfVioBufTo( hwnd, KSHELL_SCROLLBACK_LINES );
                    }
                    else
                        moveBaseLineOfVioBuf( hwnd, m_vmi.row );
                }

                for( y = usTopRow; y <= usBottomRow; y++ )
                {
                    pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( y * m_vmi.col ) + usRightCol - usCols + 1;
                    for( x = usRightCol - usCols + 1; x <= usRightCol; x++ )
                        *pBuf++ = usCell;
                }

                if( pKShellData->ulKShellMode == KSM_NORMAL )
                {
                    RECTL   rcl;

                    rcl.xLeft = usLeftCol;
                    rcl.yBottom = usBottomRow;
                    rcl.xRight = usRightCol;
                    rcl.yTop = usTopRow;
                    convertVio2Win( &rcl );

                    setCursor( hwnd, FALSE );

                    scrollWindow( hwnd, X_Vio2Win( usLeftCol ) - X_Vio2Win( usLeftCol + usCols ), 0, &rcl );

                    setCursor( hwnd, TRUE );
                }
            }

            case VI_VIOSCROLLRT :
            {
                USHORT  usCell;
                USHORT  usCols;
                USHORT  usRightCol;
                USHORT  usBottomRow;
                USHORT  usLeftCol;
                USHORT  usTopRow;

                PUSHORT pBuf;
                BOOL    fFullRect;
                BOOL    fClearScreen = FALSE;
                INT     x, y;

                DosRead( m_hpipeVioSub, &usCell, sizeof( BYTE ) * 2, &cbActual );
                DosRead( m_hpipeVioSub, &usCols, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usRightCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usBottomRow, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usLeftCol, sizeof( USHORT ), &cbActual );
                DosRead( m_hpipeVioSub, &usTopRow, sizeof( USHORT ), &cbActual );

                if( usLeftCol >= m_vmi.col )
                    usLeftCol = m_vmi.col - 1;

                if( usRightCol >= m_vmi.col )
                    usRightCol = m_vmi.col - 1;

                if( usBottomRow >= m_vmi.row )
                    usBottomRow = m_vmi.row - 1;

                if( usTopRow >= m_vmi.row )
                    usTopRow = m_vmi.row - 1;

                fFullRect = isFullRect( usTopRow, usLeftCol, usBottomRow, usRightCol );

                if( usCols >= ( usBottomRow - usTopRow + 1 ))
                {
                    usCols = usBottomRow - usTopRow + 1;
                    fClearScreen = fFullRect;
                }

                if( !fClearScreen )
                {
                    for( y = usTopRow; y <= usBottomRow; y++ )
                    {
                        pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( y * m_vmi.col ) + usLeftCol;
                        memmove( pBuf + usCols, pBuf, ( usRightCol - usLeftCol + 1 - usCols ) * VIO_CELLSIZE );
                    }
                }
                else
                {
                    if( pKShellData->usBaseLineOfVioBuf + m_vmi.row > KSHELL_SCROLLBACK_LINES ) // Scroll buf full ?
                    {
                        pBuf = ( PUSHORT )pKShellData->pVioBuf + m_vmi.row * m_vmi.col;
                        memmove( pKShellData->pVioBuf, pBuf, ( KSHELL_SCROLLBACK_LINES + m_vmi.row - m_vmi.row ) * m_vmi.col * VIO_CELLSIZE );

                        moveBaseLineOfVioBufTo( hwnd, KSHELL_SCROLLBACK_LINES );
                    }
                    else
                        moveBaseLineOfVioBuf( hwnd, m_vmi.row );
                }

                for( y = usTopRow; y <= usBottomRow; y++ )
                {
                    pBuf = ( PUSHORT )getPtrOfVioBuf( pKShellData ) + ( y * m_vmi.col ) + usLeftCol;
                    for( x = usLeftCol; x < usLeftCol + usCols; x++ )
                        *pBuf++ = usCell;
                }

                if( pKShellData->ulKShellMode == KSM_NORMAL )
                {
                    RECTL   rcl;

                    rcl.xLeft = usLeftCol;
                    rcl.yBottom = usBottomRow;
                    rcl.xRight = usRightCol;
                    rcl.yTop = usTopRow;
                    convertVio2Win( &rcl );

                    setCursor( hwnd, FALSE );

                    scrollWindow( hwnd, X_Vio2Win( usRightCol + 1 ) - X_Vio2Win( usRightCol + 1 - usCols ), 0, &rcl );

                    setCursor( hwnd, TRUE );
                }
                break;
            }
            //case VI_VIOPOPUP :
            //case VI_VIOENDPOPUP :

        }

        // send acknowledge
        //DosWrite( m_hpipeVioSub, &usIndex, sizeof( USHORT ), &cbActual );

        DosDisConnectNPipe( m_hpipeVioSub );
    } while( usIndex != ( USHORT )-1 );

    WinPostMsg( hwnd, WM_QUIT, 0, 0 );
}

VOID initPipeThreadForVioSub( HWND hwnd )
{
    CHAR szName[ PIPE_KSHELL_VIOSUB_LEN ];

    strcpy( szName, PIPE_KSHELL_VIOSUB_BASE );
    _ultoa( m_ulSGID, szName + strlen( szName ), 16 );

    DosCreateNPipe( szName,
                    &m_hpipeVioSub,
                    NP_ACCESS_DUPLEX,
                    NP_WAIT | NP_TYPE_MESSAGE | NP_READMODE_MESSAGE | 0x01,
                    32768,
                    32768,
                    0 );

    m_tidPipeThread = _beginthread( pipeThread, NULL, 32768, ( void * )hwnd );
}

VOID donePipeThreadForVioSub( VOID )
{
    callVioDmn( MSG_QUIT );

    while( DosWaitThread( &m_tidPipeThread, DCWW_WAIT ) == ERROR_INTERRUPT );

    DosClose( m_hpipeVioSub );
}


