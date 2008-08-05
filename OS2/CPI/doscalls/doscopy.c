/*!

   @file doscopy.c

   @brief DosCopy API implementation

   (c) osFree Project 2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <prokushev@freemail.ru>

*/
#include <osfree.h>

/*!
 * Copies file from one location to another
 *      CopyFile
 * CALL
 *      CopyFile(src,dst)
 * PARAMETER
 *      src             name of source file
 *      dst             name of destination (may be existent)
 * RETURNS
 *      0               copy done
 *      /0              file exits, disk full, etc.
   API
     DosAlloc
     DosFree
     DosOpenL
     DosClose
     DosRead
     DosWrite

 * GLOBAL
 *      overwrite
 */

#define IOBUF_SIZ       32768U                  /* enough? (performance) */

USHORT CopyFile(char *src,char *dst)
{
    int    hSrc, hDst;
    char  *iobuf;
    int    cbTransfer;

    if( (iobuf=malloc(IOBUF_SIZ)) == NULL )
    {
        Verbose(1,"%s\tno more memory",src);
        return (USHORT)-1;
    }

    if( (hSrc=open(src, O_BINARY|O_RDONLY)) == -1 )
    {
        Verbose(1,"open(%s) - errno %u (%s)", src, errno, strerror(errno) );
        free( iobuf );
        return 0;
    }

    if( (hDst=open(dst,
                   O_BINARY|O_WRONLY|O_CREAT|(fOverwrite ? O_TRUNC : O_EXCL),
                   S_IREAD|S_IWRITE)) == -1 )
    {
        Verbose(1,"open(%s) - errno %u (%s)", dst, errno, strerror(errno) );
        free( iobuf );
        close( hSrc );
        return errno;
    }

    while( (cbTransfer=read(hSrc, iobuf, IOBUF_SIZ)) != (USHORT)-1
          &&  cbTransfer != 0 )
    {
        if( write(hDst, iobuf, cbTransfer) != cbTransfer )
        {
            Verbose(1,"write(%s) - errno %u (%s)", dst, errno, strerror(errno) );
            free( iobuf );
            close( hSrc );
            close( hDst );
            return errno;
        }
    }

    free( iobuf );
    close( hSrc );
    close( hDst );
    return 0;
}


/*!
   Copies file from one location to another

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

*/
APIRET APIENTRY Dos32Copy(PCSZ pszOld, PCSZ pszNew, ULONG ulOptions)
{
}

/*!
   Copies files from one location to another (16-bit wrapper of Dos32Copy function)
*/
APIRET16 APIENTRY16 DosCopy(PCSZ16 pszOld, PCSZ16 pszNew, USHORT ulOptions, ULONG ulReserved)
{
  // Check arguments
  if (ulReserved) return ERROR_INVALID_PARAMETER;

  // Other arguments will be checked in 32-bit version
  return Dos32Copy(MAKEFLATP(pszOld), MAKEFLATP(pszNew), ulOptions);
}

/*!
   Copies file from one location to another (16-bit wrapper).
   Note: Seems to be full copy of DosCopy
*/
APIRET16 APIENTRY16 DosICopy(PCSZ16 pszOld, PCSZ16 pszNew, USHORT ulOptions, ULONG ulReserved)
{
  return DosCopy(pszOld, pszNew, ulOptions);
}
