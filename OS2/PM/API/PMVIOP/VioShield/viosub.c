#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_VIO
#include <os2.h>

#include <string.h>
#include <stdlib.h>

#include "dosqss.h"

#include "viosub.h"
#include "kshell.h"

#ifdef DEBUG
#include <stdio.h>

#define dprintf( ... ) \
{\
    FILE *fp;\
    fp = fopen("viosub.log", "at");\
    fprintf( fp, __VA_ARGS__ );\
    fclose( fp );\
}
#else
#define dprintf( ... )
#endif

#define BUF_SIZE    10240

static CHAR     m_achSysState[ BUF_SIZE ];

static ULONG    m_ulSGID = ( ULONG )-1;
static CHAR     m_szPipeName[ PIPE_KSHELL_VIOSUB_LEN ];

static PCH      m_LVBPtr = NULL;
static USHORT   m_LVBLen = 0;

extern BOOL     SkipFlag;

extern APIRET APIENTRY DosQuerySysState (ULONG func,
                ULONG par1, ULONG pid, ULONG _reserved_,
                PVOID buf,
                ULONG bufsz);

ULONG APIENTRY getSGID( VOID )
{
    PQTOPLEVEL  pQTopLevel = ( PQTOPLEVEL )m_achSysState;
    PPIB        ppib;

    if( m_ulSGID != ( ULONG )-1 )
        return m_ulSGID;

    DosGetInfoBlocks( NULL, &ppib );

    DosQuerySysState( 0x01, 0, ppib->pib_ulpid, 1, pQTopLevel, BUF_SIZE );

    m_ulSGID = pQTopLevel->procdata->sessid;

    return m_ulSGID;
}

static VOID pipeOpen( HPIPE *phpipe )
{
    ULONG   ulAction;
    APIRET  rc;

    do
    {
        rc = DosOpen( m_szPipeName, phpipe, &ulAction, 0, 0,
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |
                      OPEN_FLAGS_FAIL_ON_ERROR,
                      NULL );
        if( rc == ERROR_PIPE_BUSY )
            while( DosWaitNPipe( m_szPipeName, -1 ) == ERROR_INTERRUPT );
        else if( rc )
            DosSleep( 1 );

    } while( rc );
}

static VOID pipeClose( HPIPE hpipe )
{
#if 0
    USHORT usIndex;
    ULONG  cbActual;

    // wait acknowledgement
    DosRead( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
#endif

    DosClose( hpipe );
}

#define CALL_VIO( viofn, ... ) \
{ \
    ULONG rc; \
\
    SkipFlag = TRUE; \
    rc = viofn( __VA_ARGS__ ); \
    SkipFlag = FALSE; \
\
    if( rc ) \
        return rc;\
}

#pragma pack( 2 )
typedef struct tagVIOGETBUFPARAM
{
    HVIO            hvio;
    USHORT * _Seg16 pusLen;
    ULONG  * _Seg16 pulLVBPtr;
} VIOGETBUFPARAM, *PVIOGETBUFPARAM;
#pragma pack()

static ULONG vioGetBuf( USHORT usIndex, PVOID pargs )
{
    PVIOGETBUFPARAM p = pargs;

    CALL_VIO( VioGetBuf, p->pulLVBPtr, p->pusLen, p->hvio );

    m_LVBPtr = ( PCH )*( PVOID16 * )p->pulLVBPtr;
    m_LVBLen = *p->pusLen;

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOSHOWBUFPARAM
{
    HVIO    hvio;
    USHORT  usLen;
    USHORT  usOfs;
} VIOSHOWBUFPARAM, *PVIOSHOWBUFPARAM;
#pragma pack()

static ULONG vioShowBuf( USHORT usIndex, PVOID pargs )
{
    PVIOSHOWBUFPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioShowBuf, p->usOfs, p->usLen, p->hvio );

    if( m_LVBPtr )
    {
        USHORT  usStart = p->usOfs & -2;
        USHORT  usEnd = ( p->usOfs + p->usLen + 1 ) & -2;
        USHORT  usLen;

        if( usEnd > m_LVBLen )
            usEnd = m_LVBLen;

        usLen = usEnd - usStart;

        pipeOpen( &hpipe );

        DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
        DosWrite( hpipe, &usStart, sizeof( USHORT ), &cbActual );
        DosWrite( hpipe, &usLen, sizeof( USHORT ), &cbActual );
        DosWrite( hpipe, m_LVBPtr + usStart, usLen, &cbActual );

        pipeClose( hpipe );
    }

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOSETCURPOSPARAM
{
    HVIO            hvio;
    USHORT          usCol;
    USHORT          usRow;
} VIOSETCURPOSPARAM, *PVIOSETCURPOSPARAM;
#pragma pack()

static ULONG vioSetCurPos( USHORT usIndex, PVOID pargs )
{
    PVIOSETCURPOSPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioSetCurPos, p->usRow, p->usCol, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRow, sizeof( USHORT ), &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOSETCURTYPEPARAM
{
    HVIO                    hvio;
    VIOCURSORINFO * _Seg16  pvci;
} VIOSETCURTYPEPARAM, *PVIOSETCURTYPEPARAM;
#pragma pack()

static ULONG vioSetCurType( USHORT usIndex, PVOID pargs )
{
    PVIOSETCURTYPEPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioSetCurType, p->pvci, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pvci, sizeof( VIOCURSORINFO ), &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOSETMODEPARAM
{
    HVIO                 hvio;
    VIOMODEINFO * _Seg16 pvmi;
} VIOSETMODEPARAM, *PVIOSETMODEPARAM;
#pragma pack()

static ULONG vioSetMode( USHORT usIndex, PVOID pargs )
{
    PVIOSETMODEPARAM p = pargs;
    VIOMODEINFO      vmi;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioSetMode, p->pvmi, p->hvio );

    vmi.cb = sizeof( VIOMODEINFO );
    CALL_VIO( VioGetMode, &vmi, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &vmi, sizeof( VIOMODEINFO ), &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOWRTNCHARPARAM
{
    HVIO            hvio;
    USHORT          usCol;
    USHORT          usRow;
    USHORT          usTimes;
    CHAR  * _Seg16  pch;
} VIOWRTNCHARPARAM, *PVIOWRTNCHARPARAM;
#pragma pack()

static ULONG vioWrtNChar( USHORT usIndex, PVOID pargs )
{
    PVIOWRTNCHARPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioWrtNChar, p->pch, p->usTimes, p->usRow, p->usCol, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usTimes, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pch, sizeof( CHAR ), &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOWRTNATTRPARAM
{
    HVIO            hvio;
    USHORT          usCol;
    USHORT          usRow;
    USHORT          usTimes;
    BYTE  * _Seg16  pbAttr;
} VIOWRTNATTRPARAM, *PVIOWRTNATTRPARAM;
#pragma pack()

static ULONG vioWrtNAttr( USHORT usIndex, PVOID pargs )
{
    PVIOWRTNATTRPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioWrtNAttr, p->pbAttr, p->usTimes, p->usRow, p->usCol, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usTimes, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pbAttr, sizeof( BYTE ), &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOWRTNCELLPARAM
{
    HVIO            hvio;
    USHORT          usCol;
    USHORT          usRow;
    USHORT          usTimes;
    BYTE  * _Seg16  pbCell;
} VIOWRTNCELLPARAM, *PVIOWRTNCELLPARAM;
#pragma pack()

static ULONG vioWrtNCell( USHORT usIndex, PVOID pargs )
{
    PVIOWRTNCELLPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioWrtNCell, p->pbCell, p->usTimes, p->usRow, p->usCol, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usTimes, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pbCell, sizeof( BYTE ) * VIO_CELLSIZE, &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOWRTCHARSTRPARAM
{
    HVIO            hvio;
    USHORT          usCol;
    USHORT          usRow;
    USHORT          usLen;
    CHAR  * _Seg16  pchCharStr;
} VIOWRTCHARSTRPARAM, *PVIOWRTCHARSTRPARAM;
#pragma pack()

static ULONG vioWrtCharStr( USHORT usIndex, PVOID pargs )
{
    PVIOWRTCHARSTRPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioWrtCharStr, p->pchCharStr, p->usLen, p->usRow, p->usCol, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usLen, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pchCharStr, p->usLen, &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOWRTCHARSTRATTPARAM
{
    HVIO            hvio;
    BYTE  * _Seg16  pbAttr;
    USHORT          usCol;
    USHORT          usRow;
    USHORT          usLen;
    CHAR  * _Seg16  pchCharStr;
} VIOWRTCHARSTRATTPARAM, *PVIOWRTCHARSTRATTPARAM;
#pragma pack()

static ULONG vioWrtCharStrAtt( USHORT usIndex, PVOID pargs )
{
    PVIOWRTCHARSTRATTPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioWrtCharStrAtt, p->pchCharStr, p->usLen, p->usRow, p->usCol, p->pbAttr, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pbAttr, sizeof( BYTE ), &cbActual );
    DosWrite( hpipe, &p->usCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usLen, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pchCharStr, p->usLen, &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOWRTCELLSTRPARAM
{
    HVIO            hvio;
    USHORT          usCol;
    USHORT          usRow;
    USHORT          usLen;
    CHAR  * _Seg16  pchCellStr;
} VIOWRTCELLSTRPARAM, *PVIOWRTCELLSTRPARAM;
#pragma pack()

static ULONG vioWrtCellStr( USHORT usIndex, PVOID pargs )
{
    PVIOWRTCELLSTRPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioWrtCellStr, p->pchCellStr, p->usLen, p->usRow, p->usCol, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usLen, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pchCellStr, p->usLen * sizeof( BYTE ) * VIO_CELLSIZE, &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOSCROLLUPPARAM
{
    HVIO            hvio;
    BYTE  * _Seg16  pbCell;
    USHORT          usLines;
    USHORT          usRightCol;
    USHORT          usBotRow;
    USHORT          usLeftCol;
    USHORT          usTopRow;
} VIOSCROLLUPPARAM, *PVIOSCROLLUPPARAM;
#pragma pack()

static ULONG vioScrollUp( USHORT usIndex, PVOID pargs )
{
    PVIOSCROLLUPPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioScrollUp, p->usTopRow, p->usLeftCol, p->usBotRow, p->usRightCol, p->usLines, p->pbCell, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pbCell, sizeof( BYTE ) * VIO_CELLSIZE, &cbActual );
    DosWrite( hpipe, &p->usLines, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRightCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usBotRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usLeftCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usTopRow, sizeof( USHORT ), &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOSCROLLDNPARAM
{
    HVIO            hvio;
    BYTE  * _Seg16  pbCell;
    USHORT          usLines;
    USHORT          usRightCol;
    USHORT          usBotRow;
    USHORT          usLeftCol;
    USHORT          usTopRow;
} VIOSCROLLDNPARAM, *PVIOSCROLLDNPARAM;
#pragma pack()

static ULONG vioScrollDn( USHORT usIndex, PVOID pargs )
{
    PVIOSCROLLDNPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioScrollDn, p->usTopRow, p->usLeftCol, p->usBotRow, p->usRightCol, p->usLines, p->pbCell, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pbCell, sizeof( BYTE ) * VIO_CELLSIZE, &cbActual );
    DosWrite( hpipe, &p->usLines, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRightCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usBotRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usLeftCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usTopRow, sizeof( USHORT ), &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOSCROLLLFPARAM
{
    HVIO            hvio;
    BYTE  * _Seg16  pbCell;
    USHORT          usLines;
    USHORT          usRightCol;
    USHORT          usBotRow;
    USHORT          usLeftCol;
    USHORT          usTopRow;
} VIOSCROLLLFPARAM, *PVIOSCROLLLFPARAM;
#pragma pack()

static ULONG vioScrollLf( USHORT usIndex, PVOID pargs )
{
    PVIOSCROLLLFPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioScrollLf, p->usTopRow, p->usLeftCol, p->usBotRow, p->usRightCol, p->usLines, p->pbCell, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pbCell, sizeof( BYTE ) * VIO_CELLSIZE, &cbActual );
    DosWrite( hpipe, &p->usLines, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRightCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usBotRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usLeftCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usTopRow, sizeof( USHORT ), &cbActual );

    pipeClose( hpipe );

    return 0;
}

#pragma pack( 2 )
typedef struct tagVIOSCROLLRTPARAM
{
    HVIO            hvio;
    BYTE  * _Seg16  pbCell;
    USHORT          usLines;
    USHORT          usRightCol;
    USHORT          usBotRow;
    USHORT          usLeftCol;
    USHORT          usTopRow;
} VIOSCROLLRTPARAM, *PVIOSCROLLRTPARAM;
#pragma pack()

static ULONG vioScrollRt( USHORT usIndex, PVOID pargs )
{
    PVIOSCROLLRTPARAM p = pargs;

    HPIPE   hpipe;
    ULONG   cbActual;

    CALL_VIO( VioScrollRt, p->usTopRow, p->usLeftCol, p->usBotRow, p->usRightCol, p->usLines, p->pbCell, p->hvio );

    pipeOpen( &hpipe );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, p->pbCell, sizeof( BYTE ) * VIO_CELLSIZE, &cbActual );
    DosWrite( hpipe, &p->usLines, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usRightCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usBotRow, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usLeftCol, sizeof( USHORT ), &cbActual );
    DosWrite( hpipe, &p->usTopRow, sizeof( USHORT ), &cbActual );

    pipeClose( hpipe );

    return 0;
}


#pragma pack( 2 )
typedef struct vioargs {
    USHORT es_reg;
    USHORT ds_reg;
    ULONG  think16_addr;
    USHORT CallerDS;
    USHORT NearAddr;
    USHORT Index;
    ULONG  CallerAddr;
    USHORT VioHandle;
} vioargs;
#pragma pack()

ULONG __cdecl Entry32Main( vioargs * args )
{
    switch( args->Index )
    {
        case VI_VIOGETBUF        : return vioGetBuf( args->Index, &args->VioHandle );
        case VI_VIOSHOWBUF       : return vioShowBuf( args->Index, &args->VioHandle );
        case VI_VIOSETCURPOS     : return vioSetCurPos( args->Index, &args->VioHandle);
        case VI_VIOSETCURTYPE    : return vioSetCurType( args->Index, &args->VioHandle );
        case VI_VIOSETMODE       : return vioSetMode( args->Index, &args->VioHandle );
        case VI_VIOWRTNCHAR      : return vioWrtNChar( args->Index, &args->VioHandle );
        case VI_VIOWRTNATTR      : return vioWrtNAttr( args->Index, &args->VioHandle );
        case VI_VIOWRTNCELL      : return vioWrtNCell( args->Index, &args->VioHandle );
        case VI_VIOWRTCHARSTR    : return vioWrtCharStr( args->Index, &args->VioHandle );
        case VI_VIOWRTCHARSTRATT : return vioWrtCharStrAtt( args->Index, &args->VioHandle );
        case VI_VIOWRTCELLSTR    : return vioWrtCellStr( args->Index, &args->VioHandle );
        case VI_VIOSCROLLUP      : return vioScrollUp( args->Index, &args->VioHandle );
        case VI_VIOSCROLLDN      : return vioScrollDn( args->Index, &args->VioHandle );
        case VI_VIOSCROLLLF      : return vioScrollLf( args->Index, &args->VioHandle );
        case VI_VIOSCROLLRT      : return vioScrollRt( args->Index, &args->VioHandle );
        //case VI_VIOPOPUP         : return vioPopUp( args->Index, &args->VioHandle );
        //case VI_VIOENDPOPUP      : return vioEndPopUp( args->Index, &args->VioHandle );
    }

    return ( ULONG )-1;
}

unsigned APIENTRY LibMain( unsigned hmod, unsigned termination )
{
    if( !termination )
    {
        getSGID();

        strcpy( m_szPipeName, PIPE_KSHELL_VIOSUB_BASE );
        _ultoa( m_ulSGID, m_szPipeName + strlen( m_szPipeName ), 16 );
    }

    return 1;
}

