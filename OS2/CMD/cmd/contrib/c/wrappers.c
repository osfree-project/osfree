
/***********************************************************************

  $Id: wrappers.c 1398 2009-02-21 17:43:00Z gyoung $

  Wrappers with error checking

  Copyright (c) 2006, 2008 Steven H.Levine

  22 Jul 06 SHL Baseline
  29 Jul 06 SHL Add xgets_stripped
  18 Aug 06 SHL Correct Runtime_Error line number report
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  01 Sep 07 GKY Add xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry
  06 Oct 07 SHL Add xDos...() wrappers to support systems wo/large file support (Gregg, Steven)
  05 May 08 SHL Add FORTIFY support
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis

  20090609 AB taken from FM/2 project and adopted to DataSeeker / 4os2

***********************************************************************/

#define INCL_DOS
#define INCL_LONGLONG
#define INCL_DOSFILEMGR   /* File Manager values */
#define INCL_DOSERRORS    /* DOS error values */
#define INCL_DOSMODULEMGR
#define INCL_WINDIALOGS     // needed for sw.h

#include <os2.h>
#include <stdio.h>
#include <string.h>

// for x_.open
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>
#include "openmode.h"
#include "libdata.h"

// 200808xx AB add trace definitions for Dennis Bareis / AB pmprintf untility
#ifdef __DEBUG_PMPRINTF__
    #include "pmprintf.h"
    #include <STDARG.H>
    #define  __PMPRINTF__
    #define TRACE( ... )   PmpfF ( ( __VA_ARGS__ ) )
#else
    #define TRACE( ... )
#endif  // __DEBUG_PMPRINTF__
// end trace definitions

// Data definitions
static ULONG ulLastInfoLevel = FIL_STANDARDL;

#pragma data_seg(GLOBAL1)
int iLargeFileSupport = 0;
int iVerify;

static APIRET _System (* APIENTRY _DosOpenL)(       PCSZ  pszFileName,
                                                    PHFILE phf,
                                                    PULONG pulAction,
                                                    LONGLONG cbFile,
                                                    ULONG ulAttribute,
                                                    ULONG fsOpenFlags,
                                                    ULONG fsOpenMode,
                                                    PEAOP2 peaop2);

static APIRET _System (* APIENTRY _DosSetFilePtrL)( HFILE hFile,
                                                    LONGLONG ib,
                                                    ULONG method,
                                                    PLONGLONG ibActual );


int WrapperInit() {
    HMODULE hDoscalls;                                      /* Module handle                     */
    APIRET  rc              = ERROR_INVALID_FUNCTION;       /* Return code                       */

    _DosOpenL = NULL;
    _DosSetFilePtrL = NULL;

    if ( DosQueryModuleHandle("DOSCALLS", &hDoscalls) != NO_ERROR ) {
        TRACE("no DOSCALLS !!!!!!!");
    } else {
        rc = DosQueryProcAddr(hDoscalls,                // Handle to module
                              981,                      // ProcName specified
                              NULL,                     // ProcName (not specified)
                              (PFN *)&_DosOpenL);       // Address returned
        TRACE("981 rc=0x%04X (%d)", rc, rc);

        if ( rc == NO_ERROR ) { // large file support detected
            iLargeFileSupport = TRUE;
            TRACE("Congratulation, your system has large file support (Warp Server for e-business or better)");

            // get DosSetFilePtrL address
            rc = DosQueryProcAddr(hDoscalls,                // Handle to module
                                  988,                      // ProcName specified (Dos32SetFilePtrL)
                                  NULL,                     // ProcName (not specified)
                                  (PFN *)&_DosSetFilePtrL); // Address returned
        } else { // no LFS
            iLargeFileSupport = FALSE;
            TRACE("Sorry, your system has no large file support (pre Warp Server for e-business)");
        }
    }
    return rc;
}

APIRET xDosFindFirst(PSZ pszFileSpec,
                     PHDIR phdir,
                     ULONG flAttribute,
                     PVOID pfindbuf,
                     ULONG cbBuf,
                     PULONG pcFileNames,
                     ULONG ulInfoLevel) {
    APIRET rc;

    //TRACE("xDosFindFirst");
    if ( !iLargeFileSupport ) {
        ulLastInfoLevel = ulInfoLevel;
        switch ( ulInfoLevel ) {
            case FIL_STANDARDL:
                {
                    FILEFINDBUF3 ffb3;
                    ulInfoLevel = FIL_STANDARD;
                    *pcFileNames = 1;       // fixme to support larger counts
                    TRACE("xDosFindFirst.....");
                    rc = DosFindFirst(pszFileSpec, phdir, flAttribute, &ffb3, sizeof(ffb3),
                                      pcFileNames, ulInfoLevel);
                    if ( !rc ) {
                        *(PFILEFINDBUF3)pfindbuf = ffb3;    // Copy aligned data
                        ((PFILEFINDBUF3L)pfindbuf)->cbFile = ffb3.cbFile;   // Copy unaligned data
                        ((PFILEFINDBUF3L)pfindbuf)->cbFileAlloc = ffb3.cbFileAlloc;
                        ((PFILEFINDBUF3L)pfindbuf)->attrFile = ffb3.attrFile;
                        ((PFILEFINDBUF3L)pfindbuf)->cchName = ffb3.cchName;
                        memcpy(((PFILEFINDBUF3L)pfindbuf)->achName, ffb3.achName, ffb3.cchName + 1);
                    }
                }
                break;
            case FIL_QUERYEASIZEL:
                {
                    FILEFINDBUF4 ffb4;
                    *pcFileNames = 1;       // fixme to support larger counts
                    ulInfoLevel = FIL_QUERYEASIZE;
                    rc = DosFindFirst(pszFileSpec, phdir, flAttribute, &ffb4, sizeof(ffb4),
                                      pcFileNames, ulInfoLevel);
                    if ( !rc ) {
                        *(PFILEFINDBUF4)pfindbuf = ffb4;    // Copy aligned data
                        ((PFILEFINDBUF4L)pfindbuf)->cbFile = ffb4.cbFile;   // Copy unaligned data
                        ((PFILEFINDBUF4L)pfindbuf)->cbFileAlloc = ffb4.cbFileAlloc;
                        ((PFILEFINDBUF4L)pfindbuf)->attrFile = ffb4.attrFile;
                        ((PFILEFINDBUF4L)pfindbuf)->cbList = ffb4.cbList;
                        ((PFILEFINDBUF4L)pfindbuf)->cchName = ffb4.cchName;
                        memcpy(((PFILEFINDBUF4L)pfindbuf)->achName, ffb4.achName, ffb4.cchName + 1);
                    }
                }
                break;
            default:
                TRACE("ulInfoLevel=%u is unexpected", ulInfoLevel);
                rc = ERROR_INVALID_PARAMETER;
        } // switch
    } else rc = DosFindFirst(pszFileSpec, phdir, flAttribute, pfindbuf, cbBuf,
                             pcFileNames, ulInfoLevel);
    return rc;
}

APIRET xDosFindNext(HDIR hDir,      // ToDo: // 20090615 LFS     ulInfoLevel????
                    PVOID pfindbuf,
                    ULONG cbfindbuf,
                    PULONG pcFileNames) {
    APIRET rc;
    //TRACE("xDosFindNext");
    if ( !iLargeFileSupport ) {
        switch ( ulLastInfoLevel ) {
            case FIL_STANDARDL:
                {
                    FILEFINDBUF3 ffb3;
                    *pcFileNames = 1;       // fixme to support larger counts
                    rc = DosFindNext(hDir, &ffb3, sizeof(ffb3), pcFileNames);
                    if ( !rc ) {
                        *(PFILEFINDBUF3)pfindbuf = ffb3;    // Copy aligned data
                        ((PFILEFINDBUF3L)pfindbuf)->cbFile = ffb3.cbFile;   // Copy unaligned data
                        ((PFILEFINDBUF3L)pfindbuf)->cbFileAlloc = ffb3.cbFileAlloc;
                        ((PFILEFINDBUF3L)pfindbuf)->attrFile = ffb3.attrFile;
                        ((PFILEFINDBUF3L)pfindbuf)->cchName = ffb3.cchName;
                        memcpy(((PFILEFINDBUF3L)pfindbuf)->achName, ffb3.achName, ffb3.cchName + 1);
                    }
                }
                break;
            case FIL_QUERYEASIZEL:
                {
                    FILEFINDBUF4 ffb4;
                    *pcFileNames = 1;       // fixme to support larger counts
                    rc = DosFindNext(hDir, &ffb4, sizeof(ffb4), pcFileNames);
                    if ( !rc ) {
                        *(PFILEFINDBUF4)pfindbuf = ffb4;    // Copy aligned data
                        ((PFILEFINDBUF4L)pfindbuf)->cbFile = ffb4.cbFile;   // Copy unaligned data
                        ((PFILEFINDBUF4L)pfindbuf)->cbFileAlloc = ffb4.cbFileAlloc;
                        ((PFILEFINDBUF4L)pfindbuf)->attrFile = ffb4.attrFile;
                        ((PFILEFINDBUF4L)pfindbuf)->cbList = ffb4.cbList;
                        ((PFILEFINDBUF4L)pfindbuf)->cchName = ffb4.cchName;
                        memcpy(((PFILEFINDBUF4L)pfindbuf)->achName, ffb4.achName, ffb4.cchName + 1);
                    }
                }
                break;
            default:
                TRACE("ulLastInfoLevel=%u is unexpected", ulLastInfoLevel);
                rc = ERROR_INVALID_PARAMETER;
        } // switch
    } else
        rc = DosFindNext(hDir, pfindbuf, cbfindbuf, pcFileNames);
    TRACE("rc=%d, size: %lld", rc, ((PFILEFINDBUF3L)pfindbuf)->cbFile );
    return rc;
}

/**
 * DosQueryPathInfo wrapper
 * Translate request for systems without large file support
 */

APIRET xDosQueryPathInfo (PSZ pszPathName, ULONG ulInfoLevel, PVOID pInfoBuf, ULONG cbInfoBuf) {
    FILESTATUS3 fs3;
    FILESTATUS4 fs4;
    APIRET rc;

    TRACE("xDosQueryPathInfo");
    if ( !iLargeFileSupport ) {
        switch ( ulInfoLevel ) {
            case FIL_STANDARDL:
                rc = DosQueryPathInfo(pszPathName, ulInfoLevel, &fs3, sizeof(fs3));
                if ( !rc ) {
                    *(PFILESTATUS3)pInfoBuf = fs3;  // Copy aligned data
                    ((PFILESTATUS3L)pInfoBuf)->cbFile = fs3.cbFile; // Copy unaligned data
                    ((PFILESTATUS3L)pInfoBuf)->cbFileAlloc = fs3.cbFileAlloc;
                    ((PFILESTATUS3L)pInfoBuf)->attrFile = fs3.attrFile;
                }
                break;
            case FIL_QUERYEASIZEL:
                rc = DosQueryPathInfo(pszPathName, ulInfoLevel, &fs4, sizeof(fs4));
                if ( !rc ) {
                    *(PFILESTATUS4)pInfoBuf = fs4;  // Copy aligned data
                    ((PFILESTATUS4L)pInfoBuf)->cbFile = fs4.cbFile; // Copy unaligned data
                    ((PFILESTATUS4L)pInfoBuf)->cbFileAlloc = fs4.cbFileAlloc;
                    ((PFILESTATUS4L)pInfoBuf)->attrFile = fs4.attrFile;
                    ((PFILESTATUS4L)pInfoBuf)->cbList = fs4.cbList;
                }
                break;
            case FIL_QUERYFULLNAME:
                rc = DosQueryPathInfo (pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf);
                break;
            default:
                TRACE("ulInfoLevel=%u is unexpected", ulInfoLevel);
                rc = ERROR_INVALID_PARAMETER;
        } // switch
    } else {
        rc = DosQueryPathInfo (pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf);
        //TRACE1("DosQueryPathInfo rc=%d", rc);
    }

    return rc;
}

/**
 * Wrap DosSetPathInfo to avoid spurious ERROR_INVALID_NAME returns and
 * support systems without large file support
 *
 * Some kernels to do not correctly handle FILESTATUS3 and PEAOP2 buffers
 * that cross a 64K boundary.
 * When this occurs, they return ERROR_INVALID_NAME.
 * This code works around the problem because if the passed buffer crosses
 * the boundary the alternate buffer will not because both are on the stack
 * and we don't put enough additional data on the stack for this to occur.
 * It is caller's responsitibility to report errors
 * @param pInfoBuf pointer to FILESTATUS3(L) or EAOP2 buffer
 * @param ulInfoLevel FIL_STANDARD(L) or FIL_QUERYEASIZE
 * @returns Same as DosSetPathInfo
 */
/*
APIRET xDosSetPathInfo(PSZ pszPathName,
                       ULONG ulInfoLevel,
                       PVOID pInfoBuf,
                       ULONG cbInfoBuf,
                       ULONG flOptions)
    {
    FILESTATUS3 fs3;
    FILESTATUS3 fs3_a;
    FILESTATUS3L fs3l;
    EAOP2 eaop2;
    APIRET rc;
    BOOL crosses = ((ULONG)pInfoBuf ^
                    ((ULONG)pInfoBuf + cbInfoBuf - 1)) & ~0xffff;
    BOOL fResetVerify = FALSE;

    TRACE("xDosSetPathInfo");
    if ( iVerify && g.driveflags[toupper(*pszPathName) - 'A'] & DRIVE_WRITEVERIFYOFF )
        {
        DosSetVerify(FALSE);
        fResetVerify = TRUE;
        }
    switch ( ulInfoLevel )
        {
        case FIL_STANDARD:
            if ( crosses )
                {
                fs3 = *(PFILESTATUS3)pInfoBuf;    // Copy to buffer that does not cross 64K boundary
                rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3, cbInfoBuf, flOptions);
                } else
                rc = DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
            break;

        case FIL_STANDARDL:
            if ( !iLargeFileSupport )
                {
                ulInfoLevel = FIL_STANDARD;
                fs3 = *(PFILESTATUS3)pInfoBuf;    // Copy aligned data
                // Check size too big to handle
                if (((PFILESTATUS3L)pInfoBuf)->cbFile >= 1LL << 32 ||
                    ((PFILESTATUS3L)pInfoBuf)->cbFileAlloc >= 2LL << 32)
                    {
                    rc = ERROR_INVALID_PARAMETER;
                    } else
                    {
                        fs3.cbFile = ((PFILESTATUS3L)pInfoBuf)->cbFile; // Copy unaligned data
                        fs3.cbFileAlloc = ((PFILESTATUS3L)pInfoBuf)->cbFileAlloc;
                        fs3.attrFile = ((PFILESTATUS3L)pInfoBuf)->attrFile;
                        rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3, sizeof(fs3), flOptions);
                    }
                if ( rc == ERROR_INVALID_NAME )
                    {
                    // fixme to validate counts?
                    fs3_a = fs3;        // Copy to buffer that does not cross
                    rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3_a, sizeof(fs3_a), flOptions);
                    }
                } else
                {
                rc = DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
                if ( rc == ERROR_INVALID_NAME )
                    {
                    fs3l = *(PFILESTATUS3L)pInfoBuf;    // Copy to buffer that does not cross
                    rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3l, sizeof(fs3l), flOptions);
                    }
                }
            break;
        case FIL_QUERYEASIZE:
            rc = DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
            if ( rc == ERROR_INVALID_NAME )
                {
                // fixme to validate counts?
                eaop2 = *(PEAOP2)pInfoBuf;    // Copy to buffer that does not cross
                rc = DosSetPathInfo(pszPathName, ulInfoLevel, &eaop2, sizeof(eaop2), flOptions);
                }
            break;
        default:
            TRACE("ulInfoLevel=%u is unexpected", ulInfoLevel);
            rc = ERROR_INVALID_PARAMETER;
        } // switch
    if ( fResetVerify )
        {
        DosSetVerify(iVerify);
        fResetVerify = FALSE;
        }
    return rc;
    }
*/
/**
 * xDosQueryFileInfo wrapper
 * Translate request for systems without large file support
 */
// 20090615 LFS
APIRET xDosQueryFileInfo (  HFILE hf,
                            ULONG ulInfoLevel,
                            PVOID pInfoBuf,
                            ULONG cbInfoBuf) {

    FILESTATUS3 fs3;
    FILESTATUS4 fs4;
    APIRET rc;

    TRACE("xDosQueryFileInfo");
    if ( !iLargeFileSupport ) {
        switch ( ulInfoLevel ) {
            case FIL_STANDARDL:
                rc = DosQueryFileInfo(hf, ulInfoLevel, &fs3, sizeof(fs3));
                if ( !rc ) {
                    *(PFILESTATUS3)pInfoBuf = fs3;  // Copy aligned data
                    ((PFILESTATUS3L)pInfoBuf)->cbFile = fs3.cbFile; // Copy unaligned data
                    ((PFILESTATUS3L)pInfoBuf)->cbFileAlloc = fs3.cbFileAlloc;
                    ((PFILESTATUS3L)pInfoBuf)->attrFile = fs3.attrFile;
                }
                break;
            case FIL_QUERYEASIZEL:
                rc = DosQueryFileInfo(hf, ulInfoLevel, &fs4, sizeof(fs4));
                if ( !rc ) {
                    *(PFILESTATUS4)pInfoBuf = fs4;  // Copy aligned data
                    ((PFILESTATUS4L)pInfoBuf)->cbFile = fs4.cbFile; // Copy unaligned data
                    ((PFILESTATUS4L)pInfoBuf)->cbFileAlloc = fs4.cbFileAlloc;
                    ((PFILESTATUS4L)pInfoBuf)->attrFile = fs4.attrFile;
                    ((PFILESTATUS4L)pInfoBuf)->cbList = fs4.cbList;
                }
                break;
            default:
                TRACE("ulInfoLevel=%u is unexpected", ulInfoLevel);
                rc = ERROR_INVALID_PARAMETER;
        } // switch
    } else
        rc = DosQueryFileInfo (hf, ulInfoLevel, pInfoBuf, cbInfoBuf);

    return rc;
}

/**
 * xDosQueryFileInfo wrapper
 * Translate request for systems without large file support
 */
// 20090615 LFS
APIRET xDosOpen (PSZ pszFileName,
                 PHFILE pHf,
                 PULONG pulAction,
                 LONGLONG cbFile,
                 ULONG ulAttribute,
                 ULONG fsOpenFlags,
                 ULONG fsOpenMode,
                 PEAOP2 peaop2) {
    APIRET rc;

    //TRACE("xDosOpen");
    if ( iLargeFileSupport ) {
        rc = _DosOpenL (pszFileName,
                       pHf,
                       pulAction,
                       cbFile,
                       ulAttribute,
                       fsOpenFlags,
                       fsOpenMode,
                       peaop2);
        //TRACE("xDosOpen rc=%d", rc);
    } else {
        rc = DosOpen ( pszFileName,
                       pHf,
                       pulAction,
                       (ULONG) cbFile,
                       ulAttribute,
                       fsOpenFlags,
                       fsOpenMode,
                       peaop2);
    }

    return rc;
}

/**
 * xDosSetFilePtr wrapper
 * Translate request for systems without large file support
 */
// 20100126 AB
APIRET xDosSetFilePtr ( HFILE hFile,
                        LONGLONG ib,
                        ULONG method,
                        PLONGLONG ibActual ) {

    if ( iLargeFileSupport ) {
        return _DosSetFilePtrL ( hFile, ib, method, ibActual );
    } else {
        ULONG ulTemp;
        APIRET rc;
        rc = DosSetFilePtr ( hFile, ib, method, &ulTemp );
        *ibActual = (LONGLONG) ulTemp;

        return rc;
    }
}

/**
 * xlseek wrapper
 * Translate request for systems without large file support
 * _lseek64() in OW prior 1.9 have a bug for files >2GB so do it here
 */
// 20100126 AB
LONGLONG xlseek(    int iHandle,
                    LONGLONG llOffset,
                    int iOrigin ) {

    APIRET rc;
    LONGLONG llLocal;

    switch( iOrigin ) {
        case SEEK_CUR:
            rc = xDosSetFilePtr(iHandle, llOffset, FILE_CURRENT, &llLocal);
            break;
        case SEEK_SET:
            rc = xDosSetFilePtr(iHandle, llOffset, FILE_BEGIN, &llLocal);
            break;
        case SEEK_END:
            rc = xDosSetFilePtr(iHandle, llOffset, FILE_END, &llLocal);
            break;
        default:
            TRACE("(currently) not supported origin %d", iOrigin);
            errno = EINVAL;
            return( -1 );
        }



    if( rc != 0) {
        TRACE("xDosSetFilePtr returned error %d", rc);
        switch (rc) {
            case ERROR_INVALID_HANDLE:
                errno = EBADF;
                break;
        // ToDo: set errno
        // errno =  ;
            default:
                //errno = ;
                break;
        }
        return(-1);
    } else {
        //TRACE("actual position=%lld", llLocal);
    }

    return(llLocal);
}


/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*    Pulled from OW 1.8 source tree. Modified to work with large files.
*    Feb 2009 Michael Greene <mike@mgreene.org>
*
*    20100112 modified for use with 4os2 by Andreas Buchinger
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  OS/2 implementation of open64() and sopen64(). Source pulled
*               from OW tree and modified to use DosOpenL for > 2 gig.
*
****************************************************************************/

// os2file64.h
int x_sopen( const char *name, int mode, int shflag, ... );
int x_open( const char *name, int mode, ... );

static int __sopen( const char *name, int mode, int share, va_list args );
//static int __set_binary( int handle );


/* sopen64( )
 *
 * The sopen function opens a file at the operating system level
 * for shared access.
 *
 * takes param like regular sopen( ) and falls through to
 * __sopen( ) which uses DosOpenL.
 *
 * returns file handle if good or -1 if error
 */

_WCRTLINK int x_sopen( const char *name, int mode, int shflag, ... )
{
    va_list     args;

    va_start( args, shflag );

    return(__sopen( name, mode, shflag, args ) );
}


/* open64( )
 *
 * The open function opens a file at the operating system level.
 *
 * takes param like regular open( ) and falls through to
 * sopen64( ) and then to __sopen( ) which uses DosOpenL.
 *
 * returns file handle if good or -1 if error
 */

_WCRTLINK int x_open( const char *name, int mode, ... )
{
    int         permission;
    va_list     args;

    va_start( args, mode );
    permission = va_arg( args, int );
    va_end( args );

    return(x_sopen( name, mode, SH_COMPAT, permission ) );
}


/* _set_binary( )
 *
 * This seems to have no purpose.
 */

/*static int _set_binary( int handle )
{
    handle = handle;
    return( 0 );
}
*/

/* __sopen( )
 *
 * This function does all the heavy lifting.
 *
 * name   - filename to open
 * mode   - access mode
 * share  - sharing mode
 *
 */

static int __sopen( const char *name, int mode, int share, va_list args )
{
    int  rwmode;
    int  error;


    int  perm = S_IREAD | S_IWRITE;

    unsigned    iomode_flags;

    // DosOpenL parameters - file name received as function parameter

    HFILE     handle;               // handle returned
    ULONG     actiontaken;          // action taken - needed for function but not used
    LONGLONG  cbFile = 0LL;         // logical size
    int       fileattr;             // file attribute
    int       openflag;             // action to be taken
    int       openmode;             // mode of the open

    while( *name == ' ' ) ++name;   // remove leading spaces from filename

    if( mode & O_CREAT ) {

        perm = va_arg( args, int );

        va_end( args );

        if( mode & O_EXCL ) {
            openflag = OPENFLAG_FAIL_IF_EXISTS | OPENFLAG_CREATE_IF_NOT_EXISTS;
        } else if( mode & O_TRUNC ) {
            openflag = OPENFLAG_REPLACE_IF_EXISTS | OPENFLAG_CREATE_IF_NOT_EXISTS;
        } else {
            openflag = OPENFLAG_OPEN_IF_EXISTS | OPENFLAG_CREATE_IF_NOT_EXISTS;
        }

    } else if( mode & O_TRUNC ) {
        openflag = OPENFLAG_REPLACE_IF_EXISTS;
    } else {
        openflag = OPENFLAG_OPEN_IF_EXISTS;
    }

    rwmode = mode & OPENMODE_ACCESS_MASK;

    if( share == SH_COMPAT ) share = SH_DENYNO;

    openmode = share+rwmode;

    perm &= ~_RWD_umaskval;

    if( !(perm & S_IWRITE) ) {
        fileattr = _A_RDONLY;
    } else {
        fileattr = _A_NORMAL;
    }

    // 64 bit system level open function
    error = xDosOpen( (PSZ) name,
                      &handle,
                      &actiontaken,
                      cbFile,
                      fileattr,
                      openflag,
                      openmode,
                      0L );
    // TRACE("error=0x%X, handle=0x%X, '%s'", error, handle, name );
    if( error ) {
        return( __set_errno_dos( error ) );
    }

    if( handle >= __NFiles ) {
        DosClose( handle );
        __set_errno(ENOMEM);
        return( -1 );
    }

    if( rwmode == O_RDWR )              iomode_flags  = _READ | _WRITE;
    if( rwmode == O_RDONLY)             iomode_flags  = _READ;
    if( rwmode == O_WRONLY)             iomode_flags  = _WRITE;
    if( mode & O_APPEND )               iomode_flags |= _APPEND;
    if( mode & (O_BINARY|O_TEXT) ) {
        if( mode & O_BINARY )           iomode_flags |= _BINARY;
    } else {
        if( _RWD_fmode == O_BINARY )    iomode_flags |= _BINARY;
    }
    if( isatty( handle ) )              iomode_flags |= _ISTTY;

//    if( iomode_flags & _BINARY )        _set_binary( handle );    // does nothing

    __SetIOMode( handle, iomode_flags );

    // return good file handle
    return( handle );

} // end of __sopen






