
/***********************************************************************

  $Id: wrappers.h 1327 2008-12-09 04:02:12Z stevenhl $

  Wrappers with error checking

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  08 Dec 08 SHL Add missing OS2_INCLUDED check

  20090609 AB taken from FM/2 project and adopted to DataSeeker / 4os2
  20100126 AB added xlseek with DosSetFilePtrL cause a bug in OW1.8 _lseeki64

***********************************************************************/

#if !defined(WRAPPERS_H)
    #define WRAPPERS_H

    #if !defined(OS2_INCLUDED)
        #include <os2.h>
    #endif

// would need OW newer than 1.8
//#define xlseek(x, y, z)     _lseeki64(x, y, z)

// 20090609 AB added from fm3dll.h
    #define DRIVE_REMOVABLE     0x00000001
    #define DRIVE_NOTWRITEABLE  0x00000002
    #define DRIVE_IGNORE        0x00000004
    #define DRIVE_CDROM         0x00000008
    #define DRIVE_NOLONGNAMES   0x00000010
    #define DRIVE_REMOTE        0x00000020
    #define DRIVE_BOOT          0x00000040
    #define DRIVE_INVALID       0x00000080
    #define DRIVE_NOPRESCAN     0x00000100
    #define DRIVE_ZIPSTREAM     0x00000200
    #define DRIVE_NOLOADICONS   0x00000400
    #define DRIVE_NOLOADSUBJS   0x00000800
    #define DRIVE_NOLOADLONGS   0x00001000
    #define DRIVE_SLOW          0x00002000
    #define DRIVE_INCLUDEFILES  0x00004000
    #define DRIVE_VIRTUAL       0x00008000
    #define DRIVE_NOSTATS       0x00010000
    #define DRIVE_RAMDISK       0x00020000
    #define DRIVE_WRITEVERIFYOFF 0x00040000
    #define DRIVE_RSCANNED      0x00080000

int WrapperInit(void);

APIRET xDosFindFirst(PSZ pszFileSpec,
                     PHDIR phdir,
                     ULONG  flAttribute,
                     PVOID  pfindbuf,
                     ULONG  cbBuf,
                     PULONG pcFileNames,
                     ULONG  ulInfoLevel);
APIRET xDosFindNext(HDIR   hDir,
                    PVOID  pfindbuf,
                    ULONG  cbfindbuf,
                    PULONG pcFilenames);    // 06 Oct 07 SHL Added
                                            // 20090702 AB removed ulInfoLevel (now used from previous DosFindFirst)
APIRET xDosSetPathInfo(PSZ   pszPathName,
                       ULONG ulInfoLevel,
                       PVOID pInfoBuf,
                       ULONG cbInfoBuf,
                       ULONG flOptions);
APIRET xDosQueryPathInfo (PSZ pszPathName,  // 20090615 LFS
                          ULONG ulInfoLevel,
                          PVOID pInfoBuf,
                          ULONG cbInfoBuf);
APIRET xDosQueryFileInfo (HFILE hf,         // 20090615 LFS
                          ULONG ulInfoLevel,
                          PVOID pInfoBuf,
                          ULONG cbInfoBuf);
APIRET xDosOpen (PSZ pszFileName,           // 20090615 LFS
                 PHFILE pHf,
                 PULONG pulAction,
                 LONGLONG cbFile,
                 ULONG ulAttribute,
                 ULONG fsOpenFlags,
                 ULONG fsOpenMode,
                 PEAOP2 peaop2);


PSZ xfgets(PSZ pszBuf, size_t cMaxBytes, FILE * fp, PCSZ pszSrcFile,
           UINT uiLineNumber);
PSZ xfgets_bstripcr(PSZ pszBuf, size_t cMaxBytes, FILE * fp, PCSZ pszSrcFile,
                    UINT uiLineNumber);
FILE *xfopen(PCSZ pszFileName, PCSZ pszMode, PCSZ pszSrcFile,
             UINT uiLineNumber);
VOID xfree(PVOID pv, PCSZ pszSrcFile, UINT uiLineNumber);
FILE *xfsopen(PCSZ pszFileName, PCSZ pszMode, INT fSharemode, PCSZ pszSrcFile,
              UINT uiLineNumber);
PVOID xmalloc(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xmallocz(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xrealloc(PVOID pvIn, size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xstrdup(PCSZ pszIn, PCSZ pszSrcFile, UINT uiLineNumber);

_WCRTLINK int x_open( const char *name, int mode, ... );
_WCRTLINK int x_sopen( const char *name, int mode, int shflag, ... );

LONGLONG xlseek( int handle, LONGLONG offset, int origin );

// Data declarations
extern BOOL fNoLargeFileSupport;

#endif // WRAPPERS_H
