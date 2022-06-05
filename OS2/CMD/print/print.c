#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INCL_BASE
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSMODULEMGR
#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_SPLERRORS
#define INCL_SPLFSE
#define INCL_WINERRORS
#include <os2.h>
#include <pmspl.h>


#define SZ_VERSION                  "0.5.0"

#define ENABLE_DIRECT       // define to enable "direct" port writing fallback
//#define ENABLE_QUEUED       // define to enable printing through a print queue

// ---------------------------------------------------------------------------
// Spooler definitions from the DDK

#define TYPE_CACHE_ONLY             0
#define TYPE_SHORT_WAIT             1
#define TYPE_LONG_WAIT              2
#define TYPE_CACHE_UPDATE           3
#define TYPE_WITHOUT_WRAPPER        4
#define BIDI_Q_SW                   0x0000800C
#define PRTSW_JOB_WRAPPER_REQUIRED  0x00000040

typedef struct _PRTSW {
    ULONG flJob;        /* job contol flags */
    ULONG flDevice;     /* printer flags */
} PRTSW;
typedef PRTSW FAR *PPRTSW;

typedef struct _PRTOPENSTRUCT0 {
    ULONG ulVersion;
    ULONG ulSpoolerJobID;
    PSZ   pszUserName;
    PSZ   pszFormName;
    PSZ   pszDriverName;
    PSZ   pszInterpreter;
    ULONG ulLogicalUnit;
    ULONG ulStartPage;
    ULONG ulEndPage;
} PRTOPENSTRUCT0;
typedef PRTOPENSTRUCT0 FAR *PPRTOPENSTRUCT0;


// ---------------------------------------------------------------------------
// Other (private) definitions

#define CCH_MAXPORT 256             // Maximum length of a port name
#define CB_BUFFER   0xFFFF          // Default DosRead/Write buffer size

// Type of device being printed to
#define US_TYPE_PORT    0           // Printer port (/D option)
#define US_TYPE_QUEUE   1           // Printer queue (/P option)


// ---------------------------------------------------------------------------
// Global variables

BOOL  fDebug   = FALSE;             // Show debugging output?
BOOL  fAppendF = FALSE;             // Append a FF byte?
BOOL  fAppendX = FALSE;             // Append an EOT or user-defined byte?
BYTE  bAppend  = 0x04;              // User-defined byte to append
ULONG cbBuffer = CB_BUFFER;         // DosRead/Write buffer size requested

// ---------------------------------------------------------------------------
// Functions

#ifdef ENABLE_DIRECT
// These are registered dynamically from the port driver
PFN SplPdOpen,
    SplPdClose,
    SplPdQuery,
    SplPdWrite,
    SplPdProtWrite;
#endif

// Internal function prototypes
void  ListPorts( void );
BOOL  ResolvePort( PSZ pszPort, PSZ pszDriver );
ULONG WritePort_Orthodox( HFILE hfFile, PSZ pszPort );

#ifdef ENABLE_DIRECT
ULONG WritePort_Direct( HFILE hfFile, PSZ pszPort, HMODULE hmPort );
#endif

#ifdef ENABLE_QUEUED
void  ListQueues( void );
ULONG PrintQueuedJob( HFILE hfFile, PSZ pszFile, PSZ pszDest );
#endif

/* ------------------------------------------------------------------------- *
 * Main program                                                              *
 * ------------------------------------------------------------------------- */
int main( int argc, char *argv[] )
{
    FILESTATUS3 fs3;                     // file info structure
    PSZ     psz,                         // used for string manipulation
            pszFile = NULL;              // input filename argument
    CHAR    szPort[ CCH_MAXPORT ] = {0}, // output printer port or device name
            szPdr[ CCHMAXPATH ]   = {0}; // FQN of port driver
    BOOL    fSyntax = FALSE,             // syntax error
            fList   = FALSE;             // user requested a list of devices
    HMODULE hPdr    = NULLHANDLE;        // module handle for port driver
    HFILE   hfFile  = NULLHANDLE;        // file handle for input file
    ULONG   a,                           // arg index counter
            ulAppend,                    // byte-append argument as parsed
            ulAction,                    // action reported by DosOpen/PrtOpen
            flOpFn,                      // function flags for DosOpen/PrtOpen
            flOpMd;                      // mode flags for DosOpen/PrtOpen
    APIRET  rc;                          // return code from Dos**
#ifdef ENABLE_QUEUED
    USHORT  usType  = US_TYPE_PORT;      // type of output device
#endif


    // Parse the input arguments
    for ( a = 1; a < argc; a++ ) {
        psz = argv[ a ];
        if ( *psz == '/' || *psz == '-') {
            psz++;
            if ( !(*psz) ) continue;
            switch ( *psz ) {
                case 'B':
                case 'b':
                    if ( sscanf( psz+1, ":%u", cbBuffer ) != 1 )
                        cbBuffer = CB_BUFFER;
                    break;
                case 'D':
                case 'd':
                    if ( sscanf( psz+1, ":%255s", szPort ) != 1 )
                        fSyntax = TRUE;
                    break;
                case 'F':
                case 'f':
                    fAppendF = TRUE;
                    break;
                case 'L':
                case 'l':
                    fList = TRUE;
                    break;
#ifdef ENABLE_QUEUED
                case 'P':
                case 'p':
                    usType = US_TYPE_QUEUE;
                    if ( sscanf( psz+1, ":%255s", szPort ) != 1 )
                        fSyntax = TRUE;
                    break;
                case 'Q':
                case 'q':
                    usType = US_TYPE_QUEUE;
                    fList = TRUE;
                    break;
#endif
                case 'V':
                case 'v':
                    fDebug = TRUE;
                    break;
                case 'X':
                case 'x':
                    if ( sscanf( psz+1, ":%x", ulAppend ) == 1 ) {
                        fAppendX = TRUE;
                        bAppend = (BYTE) ulAppend;
                    }
                    break;
                case 'Z':
                case 'z':
                    fAppendX = TRUE;
                    bAppend = 0x04;
                    break;
            }
        }
        else if ( !pszFile )
            pszFile = psz;
        else
            fSyntax = TRUE;
    }
    if ( fList ) {
#ifdef ENABLE_QUEUED
        if ( usType == US_TYPE_QUEUE )
            ListQueues();
        else
#endif
            ListPorts();
        return 0;
    }

    if ( !pszFile || !szPort[0] || fSyntax ) {
        printf("EPRINT - Enhanced PRINT version %s\n\n", SZ_VERSION );
        printf("Usage:  EPRINT <file> /D:<port> [ /B:<size> ] [ /F ] [ /V ] [ /X:<##> ] [ /Z ]\n");
#ifdef ENABLE_QUEUED
        printf("        EPRINT <file> /P:<queue> [ /B:<size> ] [ /F ] [ /V ] [ /X:<##> ] [ /Z ]\n");
#endif
        printf("        EPRINT /L\n");
#ifdef ENABLE_QUEUED
        printf("        EPRINT /Q\n");
#endif
        printf("\n  /B:<size>  Set maximum read/write buffer size (default: 64 KB).\n");
        printf("  /D:<port>  Attempt to print <file> to port <port>.\n");
        printf("  /F         Append a form feed control byte (^L) to the print job.\n");
        printf("  /L         Show a list of valid ports known to the system.\n");
#ifdef ENABLE_QUEUED
        printf("  /P:<queue> Attempt to print <filename> to printer queue <queue>.\n");
        printf("  /Q         Show list of printer queues defined on the system.\n");
#endif
        printf("  /V         Verbose mode, show extra diagnostic information.\n");
        printf("  /X:<##>    Append the hexadecimal byte value <##> to the end of the print\n");
        printf("             job. May not be used together with /Z.\n");
        printf("  /Z         Append an end-of-transmission byte (^D) to the end of the print\n");
        printf("             job; equivalent to \"/X:04\". May not be used together with /X.\n\n");
        return 1;
    }


#ifdef ENABLE_QUEUED
    if ( usType != US_TYPE_QUEUE )
#endif
    {
        // We don't support the FILE port (there's really no point)
        if ( !strcmpi( szPort, "FILE") )  {
            fprintf( stderr, "Use of the FILE port is not supported by this command.\n");
            return 1;
        }

        // Make sure the port exists, and get the path/name of its port driver
        if ( !ResolvePort( szPort, szPdr )) {
            fprintf( stderr, "The port %s was not found.\n", szPort );
            return 2;
        }
    }

    // Open the input file
    flOpFn = OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW;
    flOpMd = OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_SEQUENTIAL |
             OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE;
    rc = DosOpen( pszFile, &hfFile, &ulAction, 0L, 0L, flOpFn, flOpMd, 0L );
    if ( rc != NO_ERROR || !hfFile ) {
        fprintf( stderr, "Failed to open input file %s: RC=%u\n", pszFile, rc );
        return ( rc );
    }

    // If the file is smaller than our default buffer size, just use the file size
    rc = DosQueryFileInfo( hfFile, FIL_STANDARD, &fs3, sizeof( fs3 ));
    if (( rc == NO_ERROR ) && ( fs3.cbFile < cbBuffer ))
        cbBuffer = fs3.cbFile;


#ifdef ENABLE_QUEUED
    if ( usType == US_TYPE_QUEUE ) {
        rc = PrintQueuedJob( hfFile, pszFile, szPort );

    }
    else
#endif
    {

        // Now try writing the contents of the input file to the port
        rc = WritePort_Orthodox( hfFile, szPort );

    #ifdef ENABLE_DIRECT
        if ( rc == ERROR_OPEN_FAILED ) {

            /* The PrtOpen function failed (this seems to happen with some devices).
             * As a fallback, we will try to to register and call the port driver's
             * open/write/close functions directly.
             */
            if ( *szPdr ) {
                // Load the port driver DLL
                rc = DosLoadModule( NULL, 0, szPdr, &hPdr );
                if ( !hPdr || ( rc != NO_ERROR )) {
                    fprintf( stderr, "Failed to load port driver %s: DosLoadModule() returned error %u\n", szPdr, rc );
                    if ( hfFile ) DosClose( hfFile );
                    return ( rc );
                }
                // If there's an entrypoint for SplPdOpen, we can proceed
                rc = DosQueryProcAddr( hPdr, 0, "SPLPDOPEN", &SplPdOpen );
                if ( rc == NO_ERROR ) {
                    printf("Opening port %s in direct access mode.\n", szPort );
                    rc = WritePort_Direct( hfFile, szPort, hPdr );
                }
                else {
                    fprintf( stderr, "Failed to open output port %s: PrtOpen() returned error 110\n", szPort );
                    fprintf( stderr, "Port driver does not appear to support SplPdOpen: DosQueryProcAddr() returned %u\n", rc );
                }
                // Release the port driver module
                DosFreeModule( hPdr );
            }
        }
    #endif

    }

    // Close the input file
    if ( hfFile ) DosClose( hfFile );

    // And we're done
    return ( rc );
}


/* ------------------------------------------------------------------------- *
 * ListPorts                                                                 *
 *                                                                           *
 * Output a list of ports known to the system.                               *
 *                                                                           *
 * PARAMETERS: NONE                                                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ListPorts( void )
{
    PPRPORTINFO1 pInfo = NULL;
    PVOID        pbuf  = NULL;
    ULONG        cbBuf,
                 cTotal,
                 cReturned,
                 cbNeeded,
                 i;
    SPLERR       rc;


    // Query the amount of available data
    rc = SplEnumPort( NULL, 1, pbuf, 0L, &cReturned, &cTotal, &cbNeeded, NULL );
    if (( rc != ERROR_MORE_DATA ) && ( rc != NERR_BufTooSmall )) {
        fprintf( stderr, "Error getting port information: SplEnumPort() returned error %u\n", rc );
        return;
    }

    // Now get the actual data
    pbuf = malloc( cbNeeded );
    if ( !pbuf ) {
        fprintf( stderr, "Memory allocation error.\n", rc );
        return;
    }
    cbBuf = cbNeeded;
    rc = SplEnumPort( NULL, 1, pbuf, cbBuf,
                      &cReturned, &cTotal, &cbNeeded, NULL );
    if ( rc == NO_ERROR ) {
        pInfo = (PPRPORTINFO1) pbuf;
        printf("------------------------------------------------------------------------------\n");
        printf("Port Name                        Port Driver\n");
        printf("------------------------------------------------------------------------------\n");
        for ( i = 0; i < cReturned; i++ ) {
            if ( !pInfo->pszPortName ) continue;
            printf("%-33s", pInfo->pszPortName );
            if ( pInfo->pszPortDriverPathName )
                printf("%s", pInfo->pszPortDriverPathName );
            printf("\n");
            pInfo++;
        }
        printf("------------------------------------------------------------------------------\n");
    }
    free( pbuf );
}


/* ------------------------------------------------------------------------- *
 * ResolvePort                                                               *
 *                                                                           *
 * Confirm that the requested port name exists, and identify the port driver *
 * that owns it.  The port name is matched case-insensitively; on return     *
 * the contents of pszPort will be updated if necessary to reflect the       *
 * proper capitalization.                                                    *
 *                                                                           *
 * PARAMETERS:                                                               *
 *   PSZ pszPort  : The requested port name.  (IO)                           *
 *   PSZ pszDriver: The fully-qualified name of the port driver (O)          *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the port was found; pszDriver will contain the port driver FQN. *
 *   FALSE if the port was not found or an error occurred.                   *
 * ------------------------------------------------------------------------- */
BOOL ResolvePort( PSZ pszPort, PSZ pszDriver )
{
    PPRPORTINFO1 pInfo = NULL;
    PVOID        pbuf  = NULL;
    ULONG        cbBuf,
                 cTotal,
                 cReturned,
                 cbNeeded,
                 i;
    SPLERR       rc;
    BOOL         fFound = FALSE;


    if ( !pszPort || !pszDriver ) return FALSE;

    // Query the amount of available data
    rc = SplEnumPort( NULL, 1, pbuf, 0L, &cReturned, &cTotal, &cbNeeded, NULL );
    if (( rc != ERROR_MORE_DATA ) && ( rc != NERR_BufTooSmall )) {
        fprintf( stderr, "Error getting port information: SplEnumPort() returned error %u\n", rc );
        return FALSE;
    }

    // Now get the actual data
    pbuf = malloc( cbNeeded );
    if ( !pbuf ) {
        fprintf( stderr, "Memory allocation error.\n");
        return FALSE;
    }
    cbBuf = cbNeeded;
    rc = SplEnumPort( NULL, 1, pbuf, cbBuf,
                      &cReturned, &cTotal, &cbNeeded, NULL );
    if ( rc == NO_ERROR ) {
        pInfo = (PPRPORTINFO1) pbuf;
        for ( i = 0; i < cReturned; i++ ) {
            if ( pInfo->pszPortName &&
                 ( stricmp( pszPort, pInfo->pszPortName )) == 0 )
            {
                fFound = TRUE;
                strcpy( pszPort, pInfo->pszPortName );
                if ( pInfo->pszPortDriverPathName ) {
                    strcpy( pszDriver, pInfo->pszPortDriverPathName );
                    if ( fDebug ) printf("Found port %s --> %s\n", pszPort, pszDriver );
                }
                else if ( fDebug ) printf("Found port %s (unknown driver)\n", pszPort );
                break;
            }
            pInfo++;
        }
    }

    free( pbuf );
    return fFound;
}


/* ------------------------------------------------------------------------- *
 * WritePort_Orthodox                                                        *
 *                                                                           *
 * Write the contents of the input file to the output port, using the        *
 * Prt*** API from PMSPL.DLL (which is the 'proper' way of doing things).    *
 *                                                                           *
 * PARAMETERS:                                                               *
 *   HFILE hfFile: Handle of the input file (as returned from DosOpen). (I)  *
 *   PSZ pszPort : The requested port name.  (I)                             *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   0 on success, system error code otherwise.                              *
 * ------------------------------------------------------------------------- */
ULONG WritePort_Orthodox( HFILE hfFile, PSZ pszPort )
{
    HFILE  hfPort = NULLHANDLE;     // file handle for output port
    PBYTE  pbuf;                    // read/write buffer
    ULONG  ulAction,                // action reported by DosOpen/PrtOpen
           flOpFn,                  // function flags for DosOpen/PrtOpen
           flOpMd,                  // mode flags for DosOpen/PrtOpen
           cbRead,                  // number of bytes read from file
           cbWritten,               // number of bytes written to port
           ulrc;                    // return code from Prt**
    APIRET rc;                      // return code from Dos**


    // Open the port
    flOpFn = OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW;
    flOpMd = OPEN_FLAGS_NONSPOOLED | OPEN_FLAGS_FAIL_ON_ERROR |
             OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE;
    ulrc = PrtOpen( pszPort, &hfPort, &ulAction, 0L, FILE_NORMAL, flOpFn, flOpMd, 0L );
    if ( ulrc != NO_ERROR || !hfPort ) {
#ifdef ENABLE_DIRECT
        // (We will handle ERROR_OPEN_FAILED specially after returning)
        if ( ulrc != ERROR_OPEN_FAILED )
#endif
            fprintf( stderr, "Failed to open output port %s: PrtOpen() returned error %u\n", pszPort, ulrc );
        return ( ulrc );
    }

    // Allocate our read/write buffer
    pbuf = malloc( cbBuffer );
    if ( !pbuf ) {
        fprintf( stderr, "Memory allocation error.\n");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    // Read from the file and write to the port until there is no more data
    do {
        rc = DosRead( hfFile, pbuf, cbBuffer, &cbRead );
        if ( rc || !cbRead ) continue;
        if ( fDebug ) printf("Read %u/%u bytes from file [%u] : ", cbRead, cbBuffer, rc );

        ulrc = PrtWrite( hfPort, pbuf, cbRead, &cbWritten );
        if ( fDebug ) printf("Wrote %u bytes to port [%u]\n", cbWritten, ulrc );

        /* Currently we just retry once if the write was incomplete.
         * Eventually this should probably be made more robust.
         */
        if ( ulrc == NO_ERROR && ( cbWritten < cbRead )) {
            cbRead = cbRead - cbWritten;
            ulrc = PrtWrite( hfPort, (PBYTE)(pbuf+cbWritten), cbRead, &cbWritten );
            if ( fDebug ) printf("Wrote %u bytes to port [%u]\n", cbWritten, ulrc );
            if ( ulrc == NO_ERROR && ( cbWritten < cbRead ))
                ulrc = ERROR_MORE_DATA;
        }

    } while (( ulrc == NO_ERROR ) && ( rc == NO_ERROR ) && ( cbRead == cbBuffer ));

    if ( rc )
        fprintf( stderr, "Error reading input file: DosRead() returned error %u\n", rc );

    if ( !rc && !ulrc && fAppendF ) {
        *pbuf = 0x0C;
        ulrc = PrtWrite( hfPort, pbuf, 1, &cbWritten );
    }
    if ( !rc && !ulrc && fAppendX ) {
        *pbuf = bAppend;
        ulrc = PrtWrite( hfPort, pbuf, 1, &cbWritten );
    }

    if ( ulrc ) {
        fprintf( stderr, "Error writing to port: PrtWrite() returned error %u\n", ulrc );
        rc = ulrc;
    }

    // Close the port
    PrtClose( hfPort );

    free( pbuf );
    return ( rc );
}


#ifdef ENABLE_DIRECT
/* ------------------------------------------------------------------------- *
 * WritePort_Direct                                                          *
 *                                                                           *
 * Write the contents of the input file to the output port by directly       *
 * registering and using the SplPd*** entrypoints from the port driver.      *
 * This is needed by some ports for which the Prt*** functions don't seem    *
 * to work.                                                                  *
 *                                                                           *
 * PARAMETERS:                                                               *
 *   HFILE hfFile  : Handle of the input file (from DosOpen). (I)            *
 *   PSZ pszPort   : The requested port name.  (I)                           *
 *   HMODULE hmPort: Handle of the port driver (from DosLoadModule). (I)     *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   0 on success, system error code otherwise.                              *
 * ------------------------------------------------------------------------- */
ULONG WritePort_Direct( HFILE hfFile, PSZ pszPort, HMODULE hmPort )
{
    FILESTATUS3    fs3;                 // file info structure
    PRTOPENSTRUCT0 spOpen = {0};        // information returned by SplPdOpen
    PRTSW  prtsw = {0};                 // BIDI information returned by SplPdQuery
    BOOL   fProt = FALSE;               // use SplPdProtWrite instead of SplPdWrite
    HFILE  hfPort = NULLHANDLE;         // handle to port
    PBYTE  pbuf;                        // read/write buffer
    ULONG  flDevice,                    // device flags returned by SplPdOpen
           cbRead,                      // number of bytes read from file
           cbWritten,                   // number of bytes written to port
           ulrc;                        // return code from SplPd**
    APIRET rc;                          // return code from Dos**



    // If the file is smaller than our default buffer size, just use the file size
    rc = DosQueryFileInfo( hfFile, FIL_STANDARD, &fs3, sizeof( fs3 ));
    if (( rc == NO_ERROR ) && ( fs3.cbFile < cbBuffer ))
        cbBuffer = fs3.cbFile;

    // Call SplPdQuery to find out which write function we need to use
    rc = DosQueryProcAddr( hmPort, 0, "SPLPDQUERY", &SplPdQuery );
    if ( rc == NO_ERROR ) {
        rc = SplPdQuery( pszPort, TYPE_SHORT_WAIT, BIDI_Q_SW, NULL, 0L, &prtsw, &cbRead );
        if (( rc == NO_ERROR ) && ( prtsw.flJob & PRTSW_JOB_WRAPPER_REQUIRED )) {
            if ( fDebug ) printf("Port driver requested use of SplPdProtWrite()\n");
            rc = DosQueryProcAddr( hmPort, 0, "SPLPDPROTWRITE", &SplPdProtWrite );
            if (( rc != NO_ERROR ) || !SplPdProtWrite ) {
                fprintf( stderr, "Failed to load entrypoint for SplPdProtWrite: DosQueryProcAddr() returned %u\n", rc );
                return ( rc );
            }
            fProt = TRUE;
        }
    }

    // Load the remaining functions (note: SplPdOpen was registered earlier)
    rc = DosQueryProcAddr( hmPort, 0, "SPLPDWRITE", &SplPdWrite );
    if ( rc != NO_ERROR ) {
        fprintf( stderr, "Failed to load entrypoint for SplPdWrite: DosQueryProcAddr() returned %u\n", rc );
        return ( rc );
    }
    rc = DosQueryProcAddr( hmPort, 0, "SPLPDCLOSE", &SplPdClose );
    if ( rc != NO_ERROR ) {
        fprintf( stderr, "Failed to load entrypoint for SplPdClose: DosQueryProcAddr() returned %u\n", rc );
        return ( rc );
    }

    // Open the output port
    ulrc = SplPdOpen( pszPort, &hfPort, &flDevice, &spOpen );
    if ( ulrc != NO_ERROR || !hfPort ) {
        fprintf( stderr, "Failed to open output port %s: SplPdOpen() returned %u\n", pszPort, ulrc );
        return ( rc );
    }

    // Allocate our read/write buffer
    pbuf = malloc( cbBuffer );
    if ( !pbuf ) {
        fprintf( stderr, "Memory allocation error.\n");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    // Read from the file and write to the port
    do {
        rc = DosRead( hfFile, pbuf, cbBuffer, &cbRead );
        if ( rc || !cbRead ) continue;
        if ( fDebug ) printf("Read %u/%u bytes from file [%u] : ", cbRead, cbBuffer, rc );
        ulrc = fProt ?
                 SplPdProtWrite( hfPort, pszPort, SplPdWrite,
                                 NULL, pbuf, cbRead, &cbWritten ) :
                 SplPdWrite( hfPort, pbuf, cbRead, &cbWritten );
        if ( fDebug ) printf("Wrote %u/%u bytes to port [%u]\n", cbWritten, cbRead, ulrc );

        /* Currently we just retry once if the write was incomplete.
         * Eventually this should probably be made more robust.
         */
        if ( ulrc == NO_ERROR && ( cbWritten < cbRead )) {
            cbRead = cbRead - cbWritten;
            ulrc = fProt ?
                     SplPdProtWrite( hfPort, pszPort, SplPdWrite, NULL,
                                     (PBYTE)(pbuf+cbWritten), cbRead, &cbWritten ) :
                     SplPdWrite( hfPort, (PBYTE)(pbuf+cbWritten), cbRead, &cbWritten );
            if ( fDebug ) printf("Writing %u bytes to port [%u] : ", cbWritten, ulrc );
            if ( ulrc == NO_ERROR && ( cbWritten < cbRead ))
                ulrc = ERROR_MORE_DATA;
        }

    } while (( ulrc == NO_ERROR ) && ( rc == NO_ERROR ) && ( cbRead == cbBuffer ));

    if ( rc )
        fprintf( stderr, "Error reading input file: DosRead returned error %u\n", rc );

    if ( !rc && !ulrc && fAppendF ) {
        *pbuf = 0x0C;
        ulrc = fProt ? SplPdProtWrite( hfPort, pszPort, SplPdWrite,
                                       NULL, pbuf, 1, &cbWritten ) :
                       SplPdWrite( hfPort, pbuf, 1, &cbWritten );
    }
    if ( !rc && !ulrc && fAppendX ) {
        *pbuf = bAppend;
        ulrc = fProt ? SplPdProtWrite( hfPort, pszPort, SplPdWrite,
                                       NULL, pbuf, 1, &cbWritten ) :
                       SplPdWrite( hfPort, pbuf, 1, &cbWritten );
    }

    if ( ulrc ) {
        fprintf( stderr, "Error writing to port: %s() returned error %u\n",
                         fProt? "SplPdProtWrite": "SplPdWrite", ulrc );
        rc = ulrc;
    }

    // Close the port
    SplPdClose( hfPort );

    free( pbuf );
    return ( rc );
}
#endif


#ifdef ENABLE_QUEUED
/* ------------------------------------------------------------------------- *
 * ListQueues                                                                *
 *                                                                           *
 * Output a list of print queues known to the spooler.                       *
 *                                                                           *
 * PARAMETERS: NONE                                                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ListQueues( void )
{
    PPRQINFO6 pInfo = NULL;
    PVOID     pbuf  = NULL;
    PCHAR     c;
    ULONG     cbBuf,
              cTotal,
              cReturned,
              cbNeeded,
              i;
    SPLERR    rc;


    // Query the amount of available data
    rc = SplEnumQueue( NULL, 6, pbuf, 0L, &cReturned, &cTotal, &cbNeeded, NULL );
    if (( rc != ERROR_MORE_DATA ) && ( rc != NERR_BufTooSmall )) {
        fprintf( stderr, "Error getting port information: SplEnumQueue() returned error %u\n", rc );
        return;
    }

    // Now get the actual data
    pbuf = malloc( cbNeeded );
    if ( !pbuf ) {
        fprintf( stderr, "Memory allocation error.\n", rc );
        return;
    }
    cbBuf = cbNeeded;
    rc = SplEnumQueue( NULL, 6, pbuf, cbBuf,
                       &cReturned, &cTotal, &cbNeeded, NULL );
    if ( rc == NO_ERROR ) {
        pInfo = (PPRQINFO6) pbuf;
        printf("------------------------------------------------------------------------------\n");
        printf("Printer Queue Name          Description\n");
        printf("------------------------------------------------------------------------------\n");
        for ( i = 0; i < cReturned; i++ ) {
            if ( !pInfo->pszName ) continue;
            printf("%-26s", pInfo->pszName );
            if ( pInfo->pszComment ) {
                c = pInfo->pszComment;
                while (( c = strpbrk( c, "\r\n")) != NULL ) *c++ = ' ';
                printf("  %s", pInfo->pszComment );
            }
            printf("\n");
            pInfo++;
        }
        printf("------------------------------------------------------------------------------\n");
    }
    free( pbuf );
}


/* ------------------------------------------------------------------------- *
 * PrintQueuedJob                                                            *
 *                                                                           *
 * Prints the contents of the input file to the specified print queue by     *
 * sending it to the spooler.                                                *
 *                                                                           *
 * PARAMETERS:                                                               *
 *   HFILE hfFile: Handle of the input file (as returned from DosOpen). (I)  *
 *   PSZ pszFile : The fully-qualified name of the input file. (I)           *
 *   PSZ pszDest : The requested printer queue name. (I)                     *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   0 on success, system error code otherwise.                              *
 * ------------------------------------------------------------------------- */
ULONG PrintQueuedJob( HFILE hfFile, PSZ pszFile, PSZ pszDest )
{
    HAB          hab;               // system HAB
    HSPL         hspl;              // handle to spooler queue
    DEVOPENSTRUC qmo = {0};         // device open structure for spooler
    PBYTE        pbuf;              // read/write buffer
    ULONG        cbRead,            // number of bytes read from file
                 ulJob,             // job ID
                 ulrc;              // return code from SplQm*
    time_t       tt;                // job submission time (seconds since epoch)
    APIRET       rc;                // return code from Dos*


    hab = WinInitialize( 0 );

    // Open the spooler queue
    qmo.pszLogAddress = pszDest;
    qmo.pszDriverName = "IBMNULL";
    qmo.pszComment    = "EPRINT";
    //qmo.pszDataType   = "PM_Q_RAW";
    hspl = SplQmOpen("*", 4L, (PQMOPENDATA) &qmo );
    if ( hspl == SPL_ERROR ) {
        ulrc = ERRORIDERROR( WinGetLastError( hab ));
        if ( ulrc == PMERR_SPL_QUEUE_NOT_FOUND )
            fprintf( stderr, "The print queue \"%s\" was not found.\n", pszDest );
        else
            fprintf( stderr, "Failed to open print queue: SplQmOpen() returned error 0x%X\n", ulrc );
        goto finish;
    }

    // Signal the start of the print job
    if ( ! SplQmStartDoc( hspl, pszFile )) {
        ulrc = ERRORIDERROR( WinGetLastError( hab ));
        fprintf( stderr, "Failed to initialize job: SplQmStartDoc() returned error 0x%X\n", ulrc );
        goto close;
    }

    // Allocate our read/write buffer (there is a max of 64KB with SplQmWrite)
    if ( cbBuffer > 0xFFFF ) cbBuffer = 0xFFFF;
    pbuf = malloc( cbBuffer );
    if ( !pbuf ) {
        fprintf( stderr, "Memory allocation error.\n");
        ulrc = ERROR_NOT_ENOUGH_MEMORY;
        goto close;
    }

    // Now copy the contents of the input file to the queue
    do {
        rc = DosRead( hfFile, pbuf, cbBuffer, &cbRead );
        if ( rc || !cbRead ) continue;
        if ( fDebug ) printf("Read %u/%u bytes from file [%u] : ", cbRead, cbBuffer, rc );

        if ( SplQmWrite( hspl, cbRead, pbuf )) {
            if ( fDebug ) printf("Wrote %u bytes to spooler\n", cbRead );
        }
        else {
            ulrc = ERRORIDERROR( WinGetLastError( hab ));
            fprintf( stderr, "Failed to write to spooler: SplQmWrite() returned error 0x%X\n", ulrc );
            break;
        }

    } while (( rc == NO_ERROR ) && ( cbRead == cbBuffer ));
    free( pbuf );

    // Signal job completion
    ulJob = SplQmEndDoc( hspl );
    if ( ulJob == SPL_ERROR ) {
        ulrc = ERRORIDERROR( WinGetLastError( hab ));
        fprintf( stderr, "Failed to complete job: SplQmEndDoc() returned error 0x%X\n", ulrc );
        goto close;
    }

    printf("%s has been submitted to print queue \"%s\" as job %u.\n", pszFile, pszDest, ulJob );
    if ( fDebug ) {
        ulrc = SplQueryJob( NULL, pszDest, ulJob, 3, NULL, 0, &cbRead );
        if (( ulrc == NERR_BufTooSmall ) || ( ulrc == ERROR_MORE_DATA )) {
            PPRJINFO3 pInfo;
            pbuf = malloc( cbRead );
            ulrc = SplQueryJob( NULL, pszDest, ulJob, 3, pbuf, cbRead, &cbRead );
            if ( !ulrc ) {
                pInfo = (PPRJINFO3) pbuf;
                tt = (time_t) pInfo->ulSubmitted;
                printf("  Priority:  %u\n", pInfo->uPriority );
                printf("  Position:  %u\n", pInfo->uPosition );
                printf("  Status:    %u\n", pInfo->fsStatus & PRJ_QSTATUS );
                if ( pInfo->pszStatus && *(pInfo->pszStatus) )
                    printf("  Message:   %s\n", pInfo->pszStatus );
                printf("  Driver:    %s\n", pInfo->pszDriverName );
                printf("  Submitted: %s",   asctime( gmtime( &tt )));
                printf("  Size:      %u\n", pInfo->ulSize );
                printf("  Document:  %s\n", pInfo->pszDocument );
            }
            free( pbuf );
        }
        if ( ulrc )
            fprintf( stderr, "Failed to query job: SplQueryJob() returned %u\n", ulrc );
    }

close:
    // Close the queue
    if ( ! SplQmClose( hspl )) {
        ulrc = ERRORIDERROR( WinGetLastError( hab ));
        fprintf( stderr, "Failed to close spooler: SplQmClose() returned error 0x%X\n", ulrc );
    }

finish:
    // Clean up and return
    WinTerminate( hab );
    return ( ulrc );
}
#endif
