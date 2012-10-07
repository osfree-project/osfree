#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_VIO
#define INCL_KBD
#define INCL_WIN
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "kshell.h"
#include "viodmn.h"

#ifdef DEBUG
#define dprintf( ... ) \
{\
    FILE *fp;\
    fp = fopen("viodmn.log", "at");\
    fprintf( fp, __VA_ARGS__ );\
    fclose( fp );\
}
#else
#define dprintf( ... )
#endif

#ifndef fGETNMSHR
#define fGETNMSHR ( fPERM )
#endif

// ----- from os2emx.h
typedef struct tagMONIN
{
  USHORT    cb;
  BYTE      abReserved[18];
  BYTE      abBuffer[108];
} MONIN, *PMONIN;

typedef struct tagMONOUT
{
  USHORT    cb;
  UCHAR     buffer[18];
  BYTE      abBuf[108];
} MONOUT, *PMONOUT;

#define VR_VIOMODEWAIT          0x00000001
#define VR_VIOMODEUNDO          0x00000002
#define VR_VIOGETFONT           0x00000004
#define VR_VIOGETCONFIG         0x00000008
#define VR_VIOSETCP             0x00000010
#define VR_VIOGETCP             0x00000020
#define VR_VIOSETFONT           0x00000040
#define VR_VIOGETSTATE          0x00000080
#define VR_VIOSETSTATE          0x00000100
// -----

#define VR_MASK1 (  \
                    VR_VIOGETCURPOS         |\
                    VR_VIOGETCURTYPE        |\
                    VR_VIOGETMODE           |\
                    VR_VIOGETBUF            |\
                    VR_VIOGETPHYSBUF        |\
                    VR_VIOSETCURPOS         |\
                    VR_VIOSETCURTYPE        |\
                    VR_VIOSETMODE           |\
                    VR_VIOSHOWBUF           |\
                    VR_VIOREADCHARSTR       |\
                    VR_VIOREADCELLSTR       |\
                    VR_VIOWRTNCHAR          |\
                    VR_VIOWRTNATTR          |\
                    VR_VIOWRTNCELL          |\
                    VR_VIOWRTTTY            |\
                    VR_VIOWRTCHARSTR        |\
                    VR_VIOWRTCHARSTRATT     |\
                    VR_VIOWRTCELLSTR        |\
                    VR_VIOSCROLLUP          |\
                    VR_VIOSCROLLDN          |\
                    VR_VIOSCROLLLF          |\
                    VR_VIOSCROLLRT          |\
                    VR_VIOSETANSI           |\
                    VR_VIOGETANSI           |\
                    VR_VIOPRTSC             |\
                    VR_VIOSCRLOCK           |\
                    VR_VIOSCRUNLOCK         |\
                    VR_VIOSAVREDRAWWAIT     |\
                    VR_VIOSAVREDRAWUNDO     |\
                    VR_VIOPOPUP             |\
                    VR_VIOENDPOPUP          |\
                    VR_VIOPRTSCTOGGLE        \
                 )

#define VR_MASK2 ( \
                    VR_VIOMODEWAIT          |\
                    VR_VIOMODEUNDO          |\
                    VR_VIOGETFONT           |\
                    VR_VIOGETCONFIG         |\
                    VR_VIOSETCP             |\
                    VR_VIOGETCP             |\
                    VR_VIOSETFONT           |\
                    VR_VIOGETSTATE          |\
                    VR_VIOSETSTATE           \
                 )

#define MON_WAIT    0x00
#define MON_NOWAIT  0x01

#define SH_RSHIFT_DOWN      0x0001
#define SH_LSHIFT_DOWN      0x0002
#define SH_CTRL_DOWN        0x0004
#define SH_ALT_DOWN         0x0008
#define SH_SCRLOCK_ON       0x0010
#define SH_NUMLOCK_ON       0x0020
#define SH_CAPSLOCK_ON      0x0040
#define SH_INSERT_ON        0x0080
#define SH_LCTRL_DOWN       0x0100
#define SH_LALT_DOWN        0x0200
#define SH_RCTRL_DOWN       0x0400
#define SH_RALT_DOWN        0x0800
#define SH_SCRLOCK_DOWN     0x1000
#define SH_NUMLOCK_DOWN     0x2000
#define SH_CAPSLOCK_DOWN    0x4000
#define SH_SYSRQ_DOWN       0x8000

#define DDF_SHIFTKEY            0x0007
#define DDF_PAUSEKEY            0x0008
#define DDF_PSEUDOPAUSEKEY      0x0009
#define DDF_WAKEUPKEY           0x000A
#define DDF_ACCENTKEY           0x0010
#define DDF_BREAKKEY            0x0011
#define DDF_PSEUDOBREAKKEY      0x0012
#define DDF_PRINTSCREENKEY      0x0013
#define DDF_PRINTECHOKEY        0x0014
#define DDF_PSEUDOPRINTECHOKEY  0x0015
#define DDF_PRINTFLUSHKEY       0x0016
#define DDF_UNDEFKEY            0x003F

#define DDF_KEYBREAK            0x0040
#define DDF_SECONDARY           0x0080
#define DDF_MULTITAKE           0x0100
#define DDF_ACCENTED            0x0200

#define ST_SHIFT_WITHOUT_CHAR      0x01
#define ST_EXTENDED_KEY            0x02
#define ST_IMM_CONVERSION_REQ      0x20
#define ST_BIT6_7                  ( 0x80 | 0x40 )
#define ST_UNDEFINED               0x00
#define ST_FINAL_INTERIM_OFF       0x40
#define ST_INTERIM                 0x80
#define ST_FINAL_INTERIM_ON        ( 0x80 | 0x40 )

#define CAS_CTRL( f )   (( f ) & KC_CTRL )
#define CAS_ALT( f )    (( f ) & KC_ALT )
#define CAS_SHIFT( f )  (( f ) & KC_SHIFT )
#define CAS_CO( f )     ( CAS_CTRL( f ) && !CAS_ALT( f ) && !CAS_SHIFT( f ))
#define CAS_AO( f )     ( !CAS_CTRL( f ) && CAS_ALT( f ) && !CAS_SHIFT( f ))
#define CAS_SO( f )     ( !CAS_CTRL( f ) && !CAS_ALT( f ) && CAS_SHIFT( f ))
#define CAS_NONE( f )   ( !CAS_CTRL( f ) && !CAS_ALT( f ) && !CAS_SHIFT( f ))

#define FKC_CHAR( f )   (( f ) & KC_CHAR )
#define FKC_SCAN( f )   (( f ) & KC_SCANCODE )
#define FKC_VIRT( f )   (( f ) & KC_VIRTUALKEY )
#define FKC_KEYUP( f )  (( f ) & KC_KEYUP )

typedef struct tagKEYPACKET
{
   USHORT     monFlag;
   KBDKEYINFO cp;
   USHORT     ddFlag;
} KEYPACKET, *PKEYPACKET;

APIRET16 _Far16 _Pascal DosSMPause( USHORT );

static MONIN  m_monIn;
static MONOUT m_monOut;

static HPIPE m_hpipe = NULLHANDLE;
static char  m_szMemName[ MEM_KSHELL_VIOBUF_LEN ];

static char  m_szPipeName[ PIPE_VIODMN_LEN ];

static PID   m_pid_comspec;
static BOOL  volatile m_fQuit = FALSE;

static ULONG m_ulSGID = ( ULONG )-1;
static CHAR  m_szVioSubPipeName[ PIPE_KSHELL_VIOSUB_LEN ];

static HEV   m_hevKShell;

static HMONITOR  m_hmon;
static TID       m_tid_pipe;
static TID       m_tid_kbdmon;

static TID      m_tid_packet;
static HQUEUE   m_hqPacket;

static BOOL m_fPaused = FALSE;

static void init( PSZ pszPid );
static void done( void );
static void initSGID( void );
static void hideFromSwitchList( void );
static void kbdmonThread( void *arg );
static void pipeThread( void *arg );
static void getCurInfo( void );
static void getVioInfo( void );
static void makeKeyEvent( void );
static void sendAck( void );
static void getSGID( void );
static void waitVioSubPipeThread( void );
static void termVioSubPipeThread( void );
static void packetRead( PKEYPACKET pkp );
static void packetWrite( PKEYPACKET pkp );
static void packetThread( void *arg );

int main( int argc, char *argv[] )
{
    PSZ         pszComspec;

    if(( argc != 3 ) || ( strcmp( argv[ 2 ], VIODMN_MAGIC ) != 0 ))
    {
        fprintf( stderr, VIODMN_MAGIC );

        return 1;
    }

    init( argv[ 1 ] );

    waitVioSubPipeThread();

    if( VioRegister( "VIOSUB", "VioRouter", VR_MASK1, VR_MASK2 ) == 0 )
    {
        pszComspec = getenv( "KSHELL_COMSPEC" );
        if( pszComspec == NULL )
        {
            pszComspec = getenv( "COMSPEC" );
            if( pszComspec == NULL )
                pszComspec = "CMD.EXE";
        }

        m_pid_comspec = spawnlp( P_NOWAIT, pszComspec, pszComspec, NULL );
        cwait( NULL, m_pid_comspec, WAIT_CHILD );

        VioDeRegister();
    }

    termVioSubPipeThread();

    done();

    return 0;
}

void init( PSZ pszPid )
{
    CHAR    szSemKShell[ SEM_VIODMN_KSHELL_LEN ];
    CHAR    szQueueName[ 80 ];

    initSGID();

    hideFromSwitchList();

    strcpy( m_szMemName, MEM_KSHELL_VIOBUF_BASE );
    strcat( m_szMemName, pszPid );

    DosMonOpen( "KBD$", &m_hmon );

    m_monIn.cb = sizeof( MONIN );
    m_monOut.cb = sizeof( MONOUT );

    DosMonReg( m_hmon, ( PBYTE )&m_monIn, ( PBYTE )&m_monOut,
               2,   // 0 for Default, 1 for First, 2 for Last
               -1 );

    m_tid_kbdmon = _beginthread( kbdmonThread, NULL, 32768, NULL );

    strcpy( szQueueName, "\\QUEUES\\VIODMN\\" );
    strcat( szQueueName, pszPid );

    DosCreateQueue( &m_hqPacket, QUE_FIFO | QUE_CONVERT_ADDRESS, szQueueName );

    m_tid_packet = _beginthread( packetThread, NULL, 32768, NULL );

    strcpy( m_szPipeName, PIPE_VIODMN_BASE );
    strcat( m_szPipeName, pszPid );

    DosCreateNPipe( m_szPipeName,
                    &m_hpipe,
                    NP_ACCESS_DUPLEX,
                    NP_WAIT | NP_TYPE_MESSAGE | NP_READMODE_MESSAGE | 0x01,
                    MSG_PIPE_SIZE,
                    MSG_PIPE_SIZE,
                    0 );

    m_tid_pipe = _beginthread( pipeThread, NULL, 32768, pszPid );

    strcpy( szSemKShell, SEM_VIODMN_KSHELL_BASE );
    strcat( szSemKShell, pszPid );

    DosCreateEventSem( szSemKShell, &m_hevKShell, DC_SEM_SHARED, 0 );
}

void done( void )
{
    KEYPACKET keyPacket;

    DosCloseEventSem( m_hevKShell );

    while( DosWaitThread( &m_tid_pipe, DCWW_WAIT ) == ERROR_INTERRUPT );
    DosClose( m_hpipe );

    keyPacket.monFlag = -1;
    packetWrite( &keyPacket );
    while( DosWaitThread( &m_tid_packet, DCWW_WAIT ) == ERROR_INTERRUPT );
    DosCloseQueue( m_hqPacket );

    DosMonClose( m_hmon );
    while( DosWaitThread( &m_tid_kbdmon, DCWW_WAIT ) == ERROR_INTERRUPT );
}

void initSGID( void )
{
    static ULONG APIENTRY ( * pfn_getSGID )( VOID );

    CHAR    szFailName[ 256 ];
    HMODULE hmod;

    DosLoadModule( szFailName, sizeof( szFailName ), "VIOSUB", &hmod );
    DosQueryProcAddr( hmod, 0, "getSGID", ( PFN * )&pfn_getSGID );

    m_ulSGID = pfn_getSGID();

    strcpy( m_szVioSubPipeName, PIPE_KSHELL_VIOSUB_BASE );
    _ultoa( m_ulSGID, m_szVioSubPipeName + strlen( m_szVioSubPipeName ), 16 );

    DosFreeModule( hmod );
}

void hideFromSwitchList( void )
{
    HSWITCH hswitch;
    SWCNTRL swc;
    PPIB    ppib;

    DosGetInfoBlocks( NULL, &ppib );

    hswitch = WinQuerySwitchHandle( NULLHANDLE, ppib->pib_ulpid );
    WinQuerySwitchEntry( hswitch, &swc );
    swc.uchVisibility = SWL_INVISIBLE;
    WinChangeSwitchEntry( hswitch, &swc );
}

void kbdmonThread( void *arg )
{
    KEYPACKET   keyPacket;
    USHORT      usLen;

    DosSetPriority( PRTYS_THREAD, PRTYC_TIMECRITICAL, +31, 0 );

    while( !m_fQuit )
    {
        usLen = sizeof( keyPacket );

        if( DosMonRead(( PBYTE )&m_monIn, MON_WAIT, ( PBYTE )&keyPacket, &usLen ) == 0 )
            DosMonWrite(( PBYTE )&m_monOut, ( PBYTE )&keyPacket, usLen );
    }
}

void pipeThread( void *arg )
{
    CHAR    szSemVioDmn[ SEM_KSHELL_VIODMN_LEN ];
    HEV     hevVioDmn = 0;

    USHORT  usMsg;
    ULONG   cbActual;

    strcpy( szSemVioDmn, SEM_KSHELL_VIODMN_BASE );
    strcat( szSemVioDmn, arg );

    DosOpenEventSem( szSemVioDmn, &hevVioDmn );
    DosPostEventSem( hevVioDmn );
    DosCloseEventSem( hevVioDmn );

    do
    {
        DosConnectNPipe( m_hpipe );

        DosRead( m_hpipe, &usMsg, sizeof( usMsg ), &cbActual );

        switch( usMsg )
        {
            case MSG_CURINFO :
                getCurInfo();
                sendAck();
                break;

            case MSG_CHAR :
                makeKeyEvent();
                sendAck();
                break;

            case MSG_VIOINFO :
                getVioInfo();
                sendAck();
                break;

            case MSG_SGID :
                getSGID();
                sendAck();
                break;

            case MSG_QUIT :
                DosKillProcess( DKP_PROCESSTREE, m_pid_comspec );
                m_fQuit = TRUE;
                sendAck();
                break;
        }

        DosDisConnectNPipe( m_hpipe );
    } while( !m_fQuit );
}

void getCurInfo( void )
{
    USHORT  usRow;
    USHORT  usCol;
    VIOCURSORINFO   ci;
    PVOID   pKBufBase;
    PCHAR   pKBuf;

    DosGetNamedSharedMem( &pKBufBase, m_szMemName, fGETNMSHR );
    pKBuf = pKBufBase;

    VioGetCurPos( &usRow, &usCol, 0 );
    VioGetCurType( &ci, 0 );

    memcpy( pKBuf, &usCol, sizeof( usCol ));
    pKBuf += sizeof( usCol );

    memcpy( pKBuf, &usRow, sizeof( usRow ));
    pKBuf += sizeof( usRow );

    memcpy( pKBuf, &ci, sizeof( ci ));
    pKBuf += sizeof( ci );

    DosFreeMem( pKBufBase );
}

void getVioInfo( void )
{
    PUSHORT       pKBuf;

    DosGetNamedSharedMem(( PVOID )&pKBuf, m_szMemName, fGETNMSHR );

    *pKBuf = sizeof( VIOMODEINFO );
    VioGetMode(( PVIOMODEINFO )pKBuf, 0 );

    DosFreeMem( pKBuf );
}

void getSGID( void )
{
    PULONG  pKBuf;

    DosGetNamedSharedMem(( PVOID )&pKBuf, m_szMemName, fGETNMSHR );

    *pKBuf = m_ulSGID;

    DosFreeMem( pKBuf );
}

static BYTE m_abPMScanToVio[256][ 4 ] =
/*********************************************************************************************/
/* PM Scancode              *     Vio Scancode     with Ctrl       with Alt     Ctrl-Char    */
/*********************************************************************************************/
/* 0x00                     */ {{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x01 PMSCAN_ESC          */ ,{ 0x01           , 0x00          , 0x00         , 0x00 }
/* 0x02 PMSCAN_ONE          */ ,{ 0x02           , 0x00          , 0x78         , 0x00 }
/* 0x03 PMSCAN_TWO          */ ,{ 0x03           , 0x03          , 0x79         , 0x00 }
/* 0x04 PMSCAN_THREE        */ ,{ 0x04           , 0x00          , 0x7A         , 0x00 }
/* 0x05 PMSCAN_FOUR         */ ,{ 0x05           , 0x00          , 0x7B         , 0x00 }
/* 0x06 PMSCAN_FIVE         */ ,{ 0x06           , 0x00          , 0x7C         , 0x00 }
/* 0x07 PMSCAN_SIX          */ ,{ 0x07           , 0x07          , 0x7D         , 0x1E }
/* 0x08 PMSCAN_SEVEN        */ ,{ 0x08           , 0x00          , 0x7E         , 0x00 }
/* 0x09 PMSCAN_EIGHT        */ ,{ 0x09           , 0x00          , 0x7F         , 0x00 }
/* 0x0A PMSCAN_NINE         */ ,{ 0x0A           , 0x00          , 0x80         , 0x00 }
/* 0x0B PMSCAN_ZERO         */ ,{ 0x0B           , 0x00          , 0x81         , 0x00 }
/* 0x0C PMSCAN_HYPHEN       */ ,{ 0x0C           , 0x0C          , 0x82         , 0x1F }
/* 0x0D PMSCAN_EQUAL        */ ,{ 0x0D           , 0x00          , 0x83         , 0x00 }
/* 0x0E PMSCAN_BACKSPACE    */ ,{ 0x0E           , 0x0E          , 0x0E         , 0x7F }
/* 0x0F PMSCAN_TAB          */ ,{ 0x0F           , 0x94          , 0xA5         , 0x00 }
/* 0x10 PMSCAN_Q            */ ,{ 0x10           , 0x10          , 0x10         , 0x11 }
/* 0x11 PMSCAN_W            */ ,{ 0x11           , 0x11          , 0x11         , 0x17 }
/* 0x12 PMSCAN_E            */ ,{ 0x12           , 0x12          , 0x12         , 0x05 }
/* 0x13 PMSCAN_R            */ ,{ 0x13           , 0x13          , 0x13         , 0x12 }
/* 0x14 PMSCAN_T            */ ,{ 0x14           , 0x14          , 0x14         , 0x14 }
/* 0x15 PMSCAN_Y            */ ,{ 0x15           , 0x15          , 0x15         , 0x19 }
/* 0x16 PMSCAN_U            */ ,{ 0x16           , 0x16          , 0x16         , 0x15 }
/* 0x17 PMSCAN_I            */ ,{ 0x17           , 0x17          , 0x17         , 0x09 }
/* 0x18 PMSCAN_O            */ ,{ 0x18           , 0x18          , 0x18         , 0x0F }
/* 0x19 PMSCAN_P            */ ,{ 0x19           , 0x19          , 0x19         , 0x10 }
/* 0x1A PMSCAN_BRACKETLEFT  */ ,{ 0x1A           , 0x1A          , 0x1A         , 0x1B }
/* 0x1B PMSCAN_BRACKETRIGHT */ ,{ 0x1B           , 0x1B          , 0x1B         , 0x1D }
/* 0x1C PMSCAN_ENTER        */ ,{ 0x1C           , 0x1C          , 0x1C         , 0x0A }
/* 0x1D PMSCAN_CTRLLEFT     */ ,{ 0x1D           , 0x1D          , 0x1D         , 0x00 }
/* 0x1E PMSCAN_A            */ ,{ 0x1E           , 0x1E          , 0x1E         , 0x01 }
/* 0x1F PMSCAN_S            */ ,{ 0x1F           , 0x1F          , 0x1F         , 0x13 }
/* 0x20 PMSCAN_D            */ ,{ 0x20           , 0x20          , 0x20         , 0x04 }
/* 0x21 PMSCAN_F            */ ,{ 0x21           , 0x21          , 0x21         , 0x06 }
/* 0x22 PMSCAN_G            */ ,{ 0x22           , 0x22          , 0x22         , 0x07 }
/* 0x23 PMSCAN_H            */ ,{ 0x23           , 0x23          , 0x23         , 0x08 }
/* 0x24 PMSCAN_J            */ ,{ 0x24           , 0x24          , 0x24         , 0x0A }
/* 0x25 PMSCAN_K            */ ,{ 0x25           , 0x25          , 0x25         , 0x0B }
/* 0x26 PMSCAN_L            */ ,{ 0x26           , 0x26          , 0x26         , 0x0C }
/* 0x27 PMSCAN_SEMICOLON    */ ,{ 0x27           , 0x00          , 0x27         , 0x00 }
/* 0x28 PMSCAN_QUOTESINGLE  */ ,{ 0x28           , 0x00          , 0x28         , 0x00 }
/* 0x29 PMSCAN_GRAVE        */ ,{ 0x29           , 0x00          , 0x29         , 0x00 }
/* 0x2A PMSCAN_SHIFTLEFT    */ ,{ 0x2A           , 0x2A          , 0x2A         , 0x00 }
/* 0x2B PMSCAN_BACKSLASH    */ ,{ 0x2B           , 0x2B          , 0x2B         , 0x1C }
/* 0x2C PMSCAN_Z            */ ,{ 0x2C           , 0x2C          , 0x2C         , 0x1A }
/* 0x2D PMSCAN_X            */ ,{ 0x2D           , 0x2D          , 0x2D         , 0x18 }
/* 0x2E PMSCAN_C            */ ,{ 0x2E           , 0x2E          , 0x2E         , 0x03 }
/* 0x2F PMSCAN_V            */ ,{ 0x2F           , 0x2F          , 0x2F         , 0x16 }
/* 0x30 PMSCAN_B            */ ,{ 0x30           , 0x30          , 0x30         , 0x02 }
/* 0x31 PMSCAN_N            */ ,{ 0x31           , 0x31          , 0x31         , 0x0E }
/* 0x32 PMSCAN_M            */ ,{ 0x32           , 0x32          , 0x32         , 0x0D }
/* 0x33 PMSCAN_COMMA        */ ,{ 0x33           , 0x00          , 0x33         , 0x00 }
/* 0x34 PMSCAN_PERIOD       */ ,{ 0x34           , 0x00          , 0x34         , 0x00 }
/* 0x35 PMSCAN_SLASH        */ ,{ 0x35           , 0x00          , 0x35         , 0x00 }
/* 0x36 PMSCAN_SHIFTRIGHT   */ ,{ 0x36           , 0x36          , 0x36         , 0x00 }
/* 0x37 PMSCAN_PADASTERISK  */ ,{ 0x37           , 0x96          , 0x37         , 0x00 }
/* 0x38 PMSCAN_ALTLEFT      */ ,{ 0x38           , 0x38          , 0x38         , 0x00 }
/* 0x39 PMSCAN_SPACE        */ ,{ 0x39           , 0x39          , 0x39         , 0x00 }
/* 0x3A PMSCAN_CAPSLOCK     */ ,{ 0x3A           , 0x3A          , 0x3A         , 0x00 }
/* 0x3B PMSCAN_F1           */ ,{ 0x3B           , 0x5E          , 0x68         , 0x00 }
/* 0x3C PMSCAN_F2           */ ,{ 0x3C           , 0x5F          , 0x69         , 0x00 }
/* 0x3D PMSCAN_F3           */ ,{ 0x3D           , 0x60          , 0x6A         , 0x00 }
/* 0x3E PMSCAN_F4           */ ,{ 0x3E           , 0x61          , 0x6B         , 0x00 }
/* 0x3F PMSCAN_F5           */ ,{ 0x3F           , 0x62          , 0x6C         , 0x00 }
/* 0x40 PMSCAN_F6           */ ,{ 0x40           , 0x63          , 0x6D         , 0x00 }
/* 0x41 PMSCAN_F7           */ ,{ 0x41           , 0x64          , 0x6E         , 0x00 }
/* 0x42 PMSCAN_F8           */ ,{ 0x42           , 0x65          , 0x6F         , 0x00 }
/* 0x43 PMSCAN_F9           */ ,{ 0x43           , 0x66          , 0x70         , 0x00 }
/* 0x44 PMSCAN_F10          */ ,{ 0x44           , 0x67          , 0x71         , 0x00 }
/* 0x45 PMSCAN_NUMLOCK      */ ,{ 0x45           , 0x45          , 0x45         , 0x00 }
/* 0x46 PMSCAN_SCROLLLOCK   */ ,{ 0x46           , 0x46          , 0x46         , 0x00 }
/* 0x47 PMSCAN_PAD7         */ ,{ 0x47           , 0x77          , 0x00         , 0x00 }
/* 0x48 PMSCAN_PAD8         */ ,{ 0x48           , 0x8D          , 0x00         , 0x00 }
/* 0x49 PMSCAN_PAD9         */ ,{ 0x49           , 0x84          , 0x00         , 0x00 }
/* 0x4A PMSCAN_PADMINUS     */ ,{ 0x4A           , 0x8E          , 0x4A         , 0x00 }
/* 0x4B PMSCAN_PAD4         */ ,{ 0x4B           , 0x73          , 0x00         , 0x00 }
/* 0x4C PMSCAN_PAD5         */ ,{ 0x4C           , 0x8F          , 0x00         , 0x00 }
/* 0x4D PMSCAN_PAD6         */ ,{ 0x4D           , 0x74          , 0x00         , 0x00 }
/* 0x4E PMSCAN_PADPLUS      */ ,{ 0x4E           , 0x90          , 0x4E         , 0x00 }
/* 0x4F PMSCAN_PAD1         */ ,{ 0x4F           , 0x75          , 0x00         , 0x00 }
/* 0x50 PMSCAN_PAD2         */ ,{ 0x50           , 0x91          , 0x00         , 0x00 }
/* 0x51 PMSCAN_PAD3         */ ,{ 0x51           , 0x76          , 0x00         , 0x00 }
/* 0x52 PMSCAN_PAD0         */ ,{ 0x52           , 0x92          , 0x00         , 0x00 }
/* 0x53 PMSCAN_PADPERIOD    */ ,{ 0x53           , 0x93          , 0x00         , 0x00 }
/* 0x54 PMSCAN_SYSREQ       */ ,{ 0x54           , 0x54          , 0x00         , 0x00 }
/* 0x55 PMSCAN_RESET        */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x56 PMSCAN_EXTRA        */ ,{ 0x56           , 0x56          , 0x56         , 0x00 }
/* 0x57 PMSCAN_F11          */ ,{ 0x85           , 0x89          , 0x8B         , 0x00 }
/* 0x58 PMSCAN_F12          */ ,{ 0x86           , 0x8A          , 0x8C         , 0x00 }
/* 0x59 PMSCAN_BACKTAB      */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x5A PMSCAN_PADENTER     */ ,{ 0xE0           , 0xE0          , 0xA6         , 0x0A }
/* 0x5B PMSCAN_CTRLRIGHT    */ ,{ 0x5B           , 0x5B          , 0x5B         , 0x00 }
/* 0x5C PMSCAN_PADSLASH     */ ,{ 0xE0           , 0x95          , 0xA4         , 0x00 }
/* 0x5D PMSCAN_PRINT        */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x5E PMSCAN_ALTRIGHT     */ ,{ 0x5E           , 0x5E          , 0x5E         , 0x00 }
/* 0x5F PMSCAN_PAUSE        */ ,{ 0x45           , 0x45          , 0x00         , 0x00 }
/* 0x60 PMSCAN_HOME         */ ,{ 0x47           , 0x77          , 0x97         , 0xE0 }
/* 0x61 PMSCAN_UP           */ ,{ 0x48           , 0x8D          , 0x98         , 0xE0 }
/* 0x62 PMSCAN_PAGEUP       */ ,{ 0x49           , 0x84          , 0x99         , 0xE0 }
/* 0x63 PMSCAN_LEFT         */ ,{ 0x4B           , 0x73          , 0x9B         , 0xE0 }
/* 0x64 PMSCAN_RIGHT        */ ,{ 0x4D           , 0x74          , 0x9D         , 0xE0 }
/* 0x65 PMSCAN_END          */ ,{ 0x4F           , 0x75          , 0x9F         , 0xE0 }
/* 0x66 PMSCAN_DOWN         */ ,{ 0x50           , 0x91          , 0xA0         , 0xE0 }
/* 0x67 PMSCAN_PAGEDOWN     */ ,{ 0x51           , 0x76          , 0xA1         , 0xE0 }
/* 0x68 PMSCAN_INSERT       */ ,{ 0x52           , 0x92          , 0xA2         , 0xE0 }
/* 0x69 PMSCAN_DELETE       */ ,{ 0x53           , 0x93          , 0xA3         , 0xE0 }
/* 0x6A PMSCAN_F23          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x6B PMSCAN_F24          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x6C PMSCAN_SYSMEM       */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x6D PMSCAN_ERASEEOF     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x6E PMSCAN_BREAK        */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x6F PMSCAN_MOVEWIN      */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x70 PMSCAN_NLS3         */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x71 PMSCAN_HELP         */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x72 PMSCAN_TASKMAN      */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x73 PMSCAN_B11          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x74 PMSCAN_JUMP         */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x75 PMSCAN_MINWIN       */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x76 PMSCAN_CLEAR        */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x77 PMSCAN_77           */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x78 PMSCAN_78           */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x79 PMSCAN_NLS2         */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x7a PMSCAN_SIZE         */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x7b PMSCAN_NLS1         */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x7c PMSCAN_APPLICATION  */ ,{ 0xEE           , 0xEE          , 0xEE         , 0x00 }
/* 0x7d PMSCAN_E13          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x7e PMSCAN_WINLEFT      */ ,{ 0xEC           , 0xEC          , 0xEC         , 0x00 }
/* 0x7f PMSCAN_WINRIGHT     */ ,{ 0xED           , 0xED          , 0xED         , 0x00 }
/* 0x80 PMSCAN_PA1          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x81 PMSCAN_F13          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x82 PMSCAN_F14          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x83 PMSCAN_F15          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x84 PMSCAN_PA2          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x85 PMSCAN_PA3          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x86 PMSCAN_SPACEBREAK   */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x87 PMSCAN_TABRIGHT     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x88 PMSCAN_NOOP         */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x89 PMSCAN_F16          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x8A PMSCAN_F17          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x8B PMSCAN_F18          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x8C PMSCAN_F19          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x8D PMSCAN_F20          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x8E PMSCAN_F21          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x8F PMSCAN_F22          */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x90                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x91                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x92                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x93                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x94                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x95 PMSCAN_DBE_CONV     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x96 PMSCAN_DBE_NOCONV   */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x97                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x98                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x99                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x9A                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x9B                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x9C                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x9D                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x9E                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0x9F                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA0                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA1                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA2                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA3                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA4                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA5                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA6                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA7                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA8                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xA9                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xAA                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xAB                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xAC                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xAD                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xAE                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xAF                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB0                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB1                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB2                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB3                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB4                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB5                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB6                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB7                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB8                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xB9                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xBA                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xBB                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xBC                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xBD                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xBE                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xBF                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC0                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC1                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC2                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC3                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC4                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC5                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC6                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC7                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC8                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xC9                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xCA                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xCB                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xCC                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xCD                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xCE                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xCF                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD0                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD1                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD2                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD3                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD4                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD5                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD6                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD7                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD8                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xD9                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xDA                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xDB                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xDC                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xDD                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xDE                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xDF                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE0                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE1                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE2                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE3                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE4                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE5                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE6                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE7                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE8                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xE9                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xEA                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xEB                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xEC                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xED                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xEE                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xEF                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF0                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF1                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF2                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF3                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF4                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF5                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF6                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF7                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF8                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xF9                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xFA                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xFB                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xFC                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xFD                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
/* 0xFF                     */ ,{ 0x00           , 0x00          , 0x00         , 0x00 }
};

void makeKeyEvent( void )
{
    PVOID       pmpBase;
    PULONG      pmp;
    ULONG       mp1;
    ULONG       mp2;
    USHORT      fsFlags;
    UCHAR       uchRepeat;
    UCHAR       uchScan;
    USHORT      usChar;
    USHORT      usVk;
    BYTE        abKbdState[ 256 ];
    BYTE        abPhysKbdState[ 256 ];
    KEYPACKET   keyPacket;

    DosGetNamedSharedMem( &pmpBase, m_szMemName, fGETNMSHR );

    pmp = pmpBase;
    mp1 = *pmp++;
    mp2 = *pmp++;
    memcpy( abKbdState, pmp, sizeof( abKbdState ));
    memcpy( abPhysKbdState, (( PBYTE )pmp ) + sizeof( abKbdState ), sizeof( abPhysKbdState ));

    DosFreeMem( pmpBase );

    fsFlags = SHORT1FROMMP( mp1 );
    uchRepeat = CHAR3FROMMP( mp1 );
    uchScan = CHAR4FROMMP( mp1 );
    usChar = SHORT1FROMMP( mp2 );
    usVk = SHORT2FROMMP( mp2 );

    keyPacket.monFlag = 0;

    keyPacket.cp.bNlsShift = 0;

    keyPacket.cp.fsState = 0;

    if( abPhysKbdState[ PM_SC_LSHIFT ] & 0x80 )
        keyPacket.cp.fsState |= SH_LSHIFT_DOWN;

    if( abPhysKbdState[ PM_SC_RSHIFT ] & 0x80 )
        keyPacket.cp.fsState |= SH_RSHIFT_DOWN;

    if( abKbdState[ VK_CTRL ] & 0x80 )
        keyPacket.cp.fsState |= SH_CTRL_DOWN;

    if( abKbdState[ VK_ALT ] & 0x80 )
        keyPacket.cp.fsState |= SH_ALT_DOWN;

    if( abKbdState[ VK_SCRLLOCK ] & 0x01 )
        keyPacket.cp.fsState |= SH_SCRLOCK_ON;

    if( abKbdState[ VK_NUMLOCK ] & 0x01 )
        keyPacket.cp.fsState |= SH_NUMLOCK_ON;

    if( abKbdState[ VK_CAPSLOCK ] & 0x01 )
        keyPacket.cp.fsState |= SH_CAPSLOCK_ON;

    if( abKbdState[ VK_INSERT ] & 0x01 )
        keyPacket.cp.fsState |= SH_INSERT_ON;

    if( abPhysKbdState[ PM_SC_LCTRL ] & 0x80 )
        keyPacket.cp.fsState |= SH_LCTRL_DOWN;

    if( abPhysKbdState[ PM_SC_LALT ] & 0x80 )
        keyPacket.cp.fsState |= SH_LALT_DOWN;

    if( abPhysKbdState[ PM_SC_RCTRL ] & 0x80 )
        keyPacket.cp.fsState |= SH_RCTRL_DOWN;

    if( abPhysKbdState[ PM_SC_RALT ] & 0x80 )
        keyPacket.cp.fsState |= SH_RALT_DOWN;

    if( abKbdState[ VK_SCRLLOCK ] & 0x80 )
        keyPacket.cp.fsState |= SH_SCRLOCK_DOWN;

    if( abKbdState[ VK_NUMLOCK ] & 0x80 )
        keyPacket.cp.fsState |= SH_NUMLOCK_DOWN;

    if( abKbdState[ VK_CAPSLOCK ] & 0x80 )
        keyPacket.cp.fsState |= SH_CAPSLOCK_DOWN;

    if( abKbdState[ VK_SYSRQ ] & 0x80 )
        keyPacket.cp.fsState |= SH_SYSRQ_DOWN;

    keyPacket.cp.fbStatus = ST_FINAL_INTERIM_OFF;

    if( !FKC_CHAR( fsFlags ) && FKC_VIRT( fsFlags ) &&
        (( usVk == VK_ALT ) || ( usVk == VK_CTRL ) || ( usVk == VK_SHIFT ) ||
         ( usVk == VK_SCRLLOCK ) || ( usVk == VK_NUMLOCK ) || ( usVk == VK_CAPSLOCK )))
        keyPacket.cp.fbStatus |= ST_SHIFT_WITHOUT_CHAR;

    keyPacket.ddFlag = 0;

    if( !FKC_SCAN( fsFlags ) && FKC_CHAR( fsFlags )) // from Clipboard or DBCS ?
    {
        keyPacket.cp.chChar = LOUCHAR( usChar );
        keyPacket.cp.chScan = 0;

        DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &keyPacket.cp.time, sizeof( ULONG ));

        packetWrite( &keyPacket );

        if( HIUCHAR( usChar )) // DBCS char ?
        {
            keyPacket.cp.chChar = HIUCHAR( usChar );
            packetWrite( &keyPacket );
        }

        return;
    }

    if( keyPacket.cp.fbStatus & ST_SHIFT_WITHOUT_CHAR )
        keyPacket.ddFlag |= DDF_SHIFTKEY;

    if( FKC_KEYUP( fsFlags ))
        keyPacket.ddFlag |= DDF_KEYBREAK;
    else if( m_fPaused )
    {
        // Shift keys and Pause key do not wake up session
        if(( keyPacket.cp.fbStatus & ST_SHIFT_WITHOUT_CHAR ) ||
           ( CAS_NONE( fsFlags ) && ( usVk == VK_PAUSE )))
            return;

        DosSMPause( m_ulSGID );

        m_fPaused = FALSE;

        // not CTRL-C(0x2E) and CTRL-BREAK(0x5F) ?
        if( !CAS_CTRL( fsFlags ) || (( uchScan != 0x2E ) && ( uchScan != 0x5F)))
            return;
    }

    if( CAS_ALT( fsFlags ))
    {
        keyPacket.cp.chChar = 0;
        keyPacket.cp.chScan = m_abPMScanToVio[ uchScan ][ 2 ];

        if( keyPacket.cp.chScan == 0 )
            keyPacket.ddFlag |= DDF_UNDEFKEY;
        else
            keyPacket.cp.fbStatus |= ST_EXTENDED_KEY;
    }
    else if( CAS_CTRL( fsFlags ))
    {
        switch( uchScan )
        {
            case 0x2E : // CTRL-C
                if( !FKC_KEYUP( fsFlags ))
                    DosSendSignalException( m_pid_comspec, XCPT_SIGNAL_INTR );
                return;

            case 0x5F : // CTRL-BREAK
                if( !FKC_KEYUP( fsFlags ))
                    DosSendSignalException( m_pid_comspec, XCPT_SIGNAL_BREAK );
                return;
        }

        keyPacket.cp.chChar = m_abPMScanToVio[ uchScan ][ 3 ];
        keyPacket.cp.chScan = m_abPMScanToVio[ uchScan ][ 1 ];

        if( keyPacket.cp.chScan == 0 )
            keyPacket.ddFlag |= DDF_UNDEFKEY;
        else if(( keyPacket.cp.chChar == 0x00 ) ||
                ( keyPacket.cp.chChar == 0xE0 ) ||
                ( keyPacket.cp.chScan == 0xE0 ))      // PAD Enter
            keyPacket.cp.fbStatus |= ST_EXTENDED_KEY;
        else
        {
            // TODO : CTRL-P, CTRL-S, ...
        }
    }
    else
    {
        keyPacket.cp.chChar = LOUCHAR( usChar );

        if( FKC_VIRT( fsFlags ) && ( usVk == VK_BACKTAB ))
            keyPacket.cp.chChar = 0;

        keyPacket.cp.chScan = m_abPMScanToVio[ uchScan ][ 0 ];
        if( CAS_SHIFT( fsFlags ))
        {
            // SHIFT key modify a scancode of Fxx keys
            if(( keyPacket.cp.chScan >= 0x3B ) && ( keyPacket.cp.chScan <= 0x44 ))
                keyPacket.cp.chScan += 0x19;
            else if(( keyPacket.cp.chScan >= 0x85 ) && ( keyPacket.cp.chScan <= 0x86 ))
                keyPacket.cp.chScan += 0x02;
        }

        if( !((( uchScan >= 0x47 ) && ( uchScan <= 0x49 )) ||
              (( uchScan >= 0x4B ) && ( uchScan <= 0x4D )) ||
              (( uchScan >= 0x4F ) && ( uchScan <= 0x53 ))) &&  // PAD Number, Period
             (( keyPacket.cp.chChar == 0x00 ) ||
              ( keyPacket.cp.chScan == 0xE0 ) || // PAD Slash, PAD Enter
              ( !FKC_CHAR( fsFlags ) && FKC_VIRT( fsFlags ) && ( usVk != VK_ESC ) &&
               ( keyPacket.cp.chScan != 0x37 )))) // PAD Asterisk
            keyPacket.cp.fbStatus |= ST_EXTENDED_KEY;

        if( keyPacket.cp.chScan == 0x45 ) // PAUSE key
        {
            if( !FKC_KEYUP( fsFlags ))
            {
                DosSMPause( m_ulSGID );

                m_fPaused = TRUE;

                return;
            }

            keyPacket.ddFlag |= FKC_KEYUP( fsFlags ) ? DDF_UNDEFKEY : DDF_PAUSEKEY;
            keyPacket.cp.fbStatus &= ~ST_EXTENDED_KEY;
        }
    }

#if 0
    // ASCII code of extended keys set to 0x00.
    // Because if monitor is at first, extended keys its ascii code is 0xe0,
    // make additional key stroke, where chChar = 0x00, chScan = 0x00, in monitor
    // chains.
    if(( keyPacket.cp.fbStatus & ST_EXTENDED_KEY ) && ( keyPacket.cp.chChar == 0xE0 ))
        keyPacket.cp.chChar = 0x00;
#endif

    DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &keyPacket.cp.time, sizeof( ULONG ));
    packetWrite( &keyPacket );
}

void sendAck( void )
{
    USHORT  usAck;
    ULONG   cbActual;

    usAck = m_fQuit ? MSG_QUIT : MSG_DONE;

    DosWrite( m_hpipe, &usAck, sizeof( usAck ), &cbActual );
    DosResetBuffer( m_hpipe );
}

void waitVioSubPipeThread( void )
{
    while( DosWaitEventSem( m_hevKShell, SEM_INDEFINITE_WAIT ) == ERROR_INTERRUPT );
}

void termVioSubPipeThread( void )
{
    HFILE   hpipe;
    ULONG   ulAction;
    USHORT  usIndex = ( USHORT )-1;
    ULONG   cbActual;
    ULONG rc;

    do
    {
        rc = DosOpen( m_szVioSubPipeName, &hpipe, &ulAction, 0, 0,
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |
                      OPEN_FLAGS_FAIL_ON_ERROR,
                      NULL );
        if( rc == ERROR_PIPE_BUSY )
            while( DosWaitNPipe( m_szVioSubPipeName, -1 ) == ERROR_INTERRUPT );
        else if( rc )
            DosSleep( 1 );

    } while( rc );

    DosWrite( hpipe, &usIndex, sizeof( USHORT ), &cbActual );

    DosClose( hpipe );
}

void packetRead( PKEYPACKET pkp )
{
    PKEYPACKET  pKeyPacket;
    REQUESTDATA rd;
    ULONG       cbData;
    BYTE        bPriority;

    DosReadQueue( m_hqPacket, &rd, &cbData, &pKeyPacket, 0, DCWW_WAIT, &bPriority, 0 );

    memcpy( pkp, pKeyPacket, sizeof( KEYPACKET ));
    free( pKeyPacket );
}

void packetWrite( PKEYPACKET pkp )
{
    PKEYPACKET pKeyPacket;

    pKeyPacket = malloc( sizeof( KEYPACKET ));
    memcpy( pKeyPacket, pkp, sizeof( KEYPACKET ));

    DosWriteQueue( m_hqPacket, 0x1234, sizeof( KEYPACKET ), pKeyPacket, 0 );
}

void packetThread( void *arg )
{
    KEYPACKET keyPacket;

    for(;;)
    {
        packetRead( &keyPacket );
        if( keyPacket.monFlag == ( USHORT )-1 )
            break;

        DosMonWrite(( PBYTE )&m_monOut, ( PBYTE )&keyPacket, sizeof( KEYPACKET ));
    }
}

