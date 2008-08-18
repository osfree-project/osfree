/*!

   @file doscopy.c

   @brief DosCopy API implementation. Based on RCOPY.

   (c) osFree Project 2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <prokushev@freemail.ru>

   @todo Add support of EAs

*/
//#define INCL_OS2DEF
#include <os2def.h>
#define INCL_DOSFILEMGR
#include <osfree.h>

/*!
   @brief Copies file from one location to another

   @param pszSrc             name of source file
   @param pszDst             name of destination (may be existent)
   @param ulOptions          copy options

   @return
     NO_ERROR           - files were processed succesfully
     ERROR_WRITE_FAULT  - fault during file writing
     other              - according errors in used APIs

   API
     DosAllocMem
     DosFreeMem
     DosOpenL
     DosClose
     DosRead
     DosWrite
     DosSetFilePtrL

*/

#define IOBUF_SIZ       32768U                  /* enough? (performance)
                                                   Most probably here
                                                   we must automatically
                                                   detect size of buffer */

APIRET CopyFile(PSZ pszSrc, PSZ pszDst, ULONG ulOptions)
{
  APIRET rc;
  HFILE  hSrc;
  HFILE  hDst;
  ULONG  ulAction;
  //PCHAR
  void **pIOBuf;
  ULONG  ulTransfer;

  rc = DosAllocMem(pIOBuf,
                   IOBUF_SIZ,
                   fPERM|PAG_COMMIT);
  if (rc) return rc;

  rc = DosOpenL(pszSrc,             // Address of ASCIIZ with source path
                &hSrc,              // Handle
                &ulAction,          // Action was taken (not used)
                (LONGLONG) 0,       // Initial file size (not used)
                0,                  // File attributes (not used)
                OPEN_ACTION_FAIL_IF_NEW |
                OPEN_ACTION_OPEN_IF_EXISTS, // Open type
                OPEN_SHARE_DENYNONE |
                OPEN_ACCESS_READONLY, // Open mode
                NULL);
  if (rc)
  {
    DosClose(hSrc);
    DosFreeMem(pIOBuf);
    return rc;
  }

  if (!(ulOptions&DCPY_EXISTING))
  {
    // if no file exists then we fail
    ulOpenType = OPEN_ACTION_FAIL_IF_EXISTS;
  } else {
    if (ulOptions&DCPY_APPEND)
    {
      // else or append
      ulOpenType = OPEN_ACTION_OPEN_IF_EXISTS;
    } else {
      // or replace
      ulOpenType = OPEN_ACTION_REPLACE_IF_EXISTS;
    }
  }

  rc = DosOpenL(pszDst,             // Address of ASCIIZ with source path
                &hDst,              // Handle
                &ulAction,          // Action was taken
                (LONGLONG) 0,       // Initial file size (not used)
                FILE_ARCHIVED |
                FILE_NORMAL, // File attributes
                OPEN_ACTION_CREATE_IF_NEW |
                ulOpenType, // Open type
                OPEN_SHARE_DENYREADWRITE |
                OPEN_ACCESS_WRITEONLY, // Open mode
                NULL);
  if (rc)
  {
    DosClose(hDst);
    DosClose(hSrc);
    DosFreeMem(pIOBuf);
    return rc;
  }


  // If append mode move file pointer to the end
  // We can be here only if not DCY_EXISTING flag
  // set because DosOpenL will fail
  // In case of APPEND we need to move to end of file (because file open
  // with OPEN_ACCESS_OPEN_IF_EXISTS) otherwise
  if (ulOptions&DCPY_APPEND)
  {
    DosSetFilePtrL (hDst,
                    0,
                    FILE_END,
                    NULL);
  }

  rc = DosRead(hSrc, pIOBuf, IOBUF_SIZ, &ulTransfer);
  if (rc)
  {
    DosClose(hDst);
    DosClose(hSrc);
    DosFreeMem(pIOBuf);
    return rc;
  }

  while(ulTransfer)
  {
    rc = DosWrite(hDst, pIOBuf, ulTransfer, ulWritten);
    if (rc)
    {
      DosClose(hDst);
      DosClose(hSrc);
      DosFreeMem(pIOBuf);
      return rc;
    }

    if (ulTransfer!=ulWritten) return ERROR_WRITE_FAULT;

    rc = DosRead(hSrc, pIOBuf, IOBUF_SIZ, &ulTransfer);
    if (rc)
    {
      DosClose(hDst);
      DosClose(hSrc);
      DosFreeMem(pIOBuf);
      return rc;
    }
  }

  DosClose(hDst);
  DosClose(hSrc);
  DosFreeMem(pIOBuf);
  return NO_ERROR;
}

/*!
    @brief Copy directory tree

    @param src             Source Path (existing)
    @param dst             Destination Path (existing)
    @param ulOption        copy options

    @return
        NO_ERROR               OK

 */
APIRET CopyTree(PSZ pszSrc, PSZ pszDst, ULONG ulOptions)
{
    FILEFINDBUF3  findBuffer;
    HDIR         hSearch;
    ULONG        cFound;
    APIRET       rc;
    int          result = 0, i;
    char        *nsp, *ndp;
    struct dirlist_t {
        char              src[_MAX_PATH]; //@todo detect maximum path name!!!
        char              dst[_MAX_PATH]; //@todo detect maximum path name!!!
        struct dirlist_t *next;
    } *pDirListRoot=NULL, *pDirList=NULL, *pHelp;

    nsp = pszSrc + strlen(pszSrc);
    ndp = pszDst + strlen(pszDst);

    /* Search all subdirectories */

    strcpy( nsp, "*" );
    hSearch = HDIR_SYSTEM;                      /* use system handle */
    cFound = 1;                                 /* only one at a time */
    rc = DosFindFirst(pszSrc,
                      &hSearch,
                      MUST_HAVE_DIRECTORY|FILE_DIRECTORY,
                      &findBuffer,
                      sizeof(findBuffer),
                      &cFound,
                      FIL_STANDARD);
    if( !rc )
        do
        {
            if( !strcmp(findBuffer.achName, ".")
               ||  !strcmp(findBuffer.achName, "..") )
                continue;
            if( !(findBuffer.attrFile & FILE_DIRECTORY) )
                continue;

            strcpy( nsp, findBuffer.achName );
            strcpy( ndp, findBuffer.achName );

            strcat( pszSrc, "/" );
            strcat( pszDst, "/" );

            /* Tyv„r, vi kan inte kopiera nu */

            if( pDirList )
            {
                pDirList->next = malloc( sizeof(struct dirlist_t) );
                assert( pDirList->next != NULL );
                pDirList = pDirList->next;
            }
            else
            {
                pDirList = pDirListRoot = malloc( sizeof(struct dirlist_t) );
                assert( pDirList != NULL );
            }
            pDirList->next = NULL;
            strcpy( pDirList->src, pszSrc );
            strcpy( pDirList->dst, pszDst );
        }
        while( !(rc=DosFindNext(hSearch, &findBuffer,
                                sizeof(findBuffer), &cFound)) );
    DosFindClose( hSearch );

    for( pHelp = pDirList = pDirListRoot; pDirList ; pHelp = pDirList )
    {
        pDirList = pDirList->next;
        if( (i=CheckPath(pHelp->dst, 1)) )      /* create destination path */
        {
            result = i;
        }
        else
        {
            CopyTree( pHelp->src, pHelp->dst, ulOptions );
        }
        free( pHelp );
    }

    /* Copy the files in actual directory */

    strcpy( nsp, "*" );
    hSearch = HDIR_SYSTEM;                      /* use system handle */
    cFound = 1;                                 /* only one at a time */
    rc = DosFindFirst(pszSrc, &hSearch, FILE_NORMAL,
                      &findBuffer, sizeof(findBuffer), &cFound, FIL_STANDARD );
    if( !rc )
        do
        {
            if( findBuffer.attrFile & FILE_DIRECTORY )
                continue;

            strcpy( nsp, findBuffer.achName );
            strcpy( ndp, findBuffer.achName );
            i = CopyFile( pszSrc, pszDst, ulOptions );
            if( i != 0 )
                result = i;
        }
        while( !(rc=DosFindNext(hSearch, &findBuffer,
                                sizeof(findBuffer), &cFound)) );
    DosFindClose( hSearch );

    *nsp = '\0';
    *ndp = '\0';
    return result;
}

/*!
   @brief Copies file (directory) from one location to another

   @param pszOld     pointer to ASCIIZ filename, directory or character device
   @param pszNew     pointer to ASCIIZ target filename, directory or character device
   @param ulOptions  options to be used when processing files

Bit Description

2 DCPY_FAILEAS (0x00000004)
Discard the EAs if the source file contains EAs and the destination file system does not support EAs.

0 Discard the EAs (extended attributes) if the destination file system does not support EAs.

1 Fail the copy if the destination file system does not support EAs.

1 DCPY_APPEND (x00000002)
Append the source file to the target file's end of data.

0 Replace the target file with the source file.
1 Append the source file to the target file's end of data.

This is ignored when copying a directory, or if the target file does not exist.

0 DCPY_EXISTING (0x00000001)
Existing Target File Disposition.

0 Do not copy the source file to the target if the file name already exists within the target directory. If a single file is being copied and the target already exists, an error is returned.

1 Copy the source file to the target even if the file name already exists within the target directory.

Bit flag DCPY_FAILEAS can be used in combination with bit flag DCPY_APPEND or DCPY_EXISTING.

   @return
     NO_ERROR - files were processed succesfully
     ERROR_FILE_NOT_FOUND
     ERROR_PATH_NOT_FOUND
     ERROR_ACCESS_DENIED
     ERROR_NOT_DOS_DISK
     ERROR_SHARING_VIOLATION
     ERROR_SHARING_BUFFER_EXCEEDED
     ERROR_INVALID_PARAMETER
     ERROR_DRIVE_LOCKED
     ERROR_DISK_FULL
     ERROR_FILENAME_EXCED_RANGE
     ERROR_DIRECTORY
     ERROR_EAS_NOT_SUPPORTED
     ERROR_NEED_EAS_FOUND
     ERROR_NOT_ENOUGH_MEMORY

*/
APIRET APIENTRY Dos32Copy(PCSZ pszOld, PCSZ pszNew, ULONG ulOptions)
{
  FILESTATUS3 fileStatus;
  APIRET rc;

  #define DCPY_MASK ~(DCPY_EXISTING | DCPY_APPEND | DCPY_FAILEAS )

  //Check arguments
  if ((!pszOld) || (!pszNew)) return ERROR_INVALID_PARAMETER;
  // Also check for reserved options used
  if (ulOptions & DCPY_MASK) return ERROR_INVALID_PARAMETER;

  //Detect is source dir or file (also check is it exists)
  rc = DosQueryPathInfo(pszOld,               // Path
                        FIL_STANDARD,         // Level 1 information
                        &fileStatus,          // Address of return buffer
                        sizeof(FILESTATUS3)); // Size of buffer

  if (rc) return rc;

  // Perfom action based on source path type
  if ((fileStatus.attrFile&FILE_DIRECTORY)==FILE_DIRECTORY)
  {
    // DCPY_APPEND flag not valid in directory copy
    return CopyTree(pszOld, pszNew, ulOptions & ~DCPY_APPEND);
  } else {
    return CopyFile(pszOld, pszNew); //FIXME pass options
  };
}

/*!
   Copies files from one location to another (16-bit wrapper of Dos32Copy function)
*/
APIRET16 APIENTRY16 DosCopy(PCSZ16 pszOld, PCSZ16 pszNew, USHORT usOptions, ULONG ulReserved)
{
  // Check arguments
  if (ulReserved) return ERROR_INVALID_PARAMETER;

  // Other arguments will be checked in 32-bit version
  return Dos32Copy(MAKEFLATP(pszOld), MAKEFLATP(pszNew), usOptions);
}

/*!
   Copies file from one location to another (16-bit wrapper).
   Note: Seems to be full copy of DosCopy
*/
APIRET16 APIENTRY16 DosICopy(PCSZ16 pszOld, PCSZ16 pszNew, USHORT usOptions, ULONG ulReserved)
{
  return DosCopy(pszOld, pszNew, usOptions);
}
