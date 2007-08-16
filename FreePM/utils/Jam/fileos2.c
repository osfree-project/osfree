/*
 * Copyright 1993, 1995 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

# include "jam.h"
# include "filesys.h"

/* note that we use "fileunix.c" when compiling with EMX on OS/2 */
# if defined(OS_OS2) && !defined(__EMX__)

# include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
  #define INCL_DOSFILEMGR   /* File Manager values */
  #define INCL_DOSERRORS    /* DOS error values */
  #include <os2.h>

/*
 * fileos2.c - scan directories and archives on OS/2
 *
 * External routines:
 *
 *     file_dirscan() - scan a directory for files
 *     file_time() - get timestamp of file, if not done by file_dirscan()
 *     file_archscan() - scan an archive for files
 *
 * File_dirscan() and file_archscan() call back a caller provided function
 * for each file found.  A flag to this callback function lets file_dirscan()
 * and file_archscan() indicate that a timestamp is being provided with the
 * file.   If file_dirscan() or file_archscan() do not provide the file's
 * timestamp, interested parties may later call file_time().
 *
 * 07/10/95 (taylor)  Findfirst() returns the first file on NT.
 * 05/03/96 (seiwald) split apart into pathnt.c
 * 09/22/00 (seiwald) handle \ and c:\ specially: don't add extra /
 */

/*
 * file_dirscan() - scan a directory for files
 */

void
file_dirscan(
       char *dir,
       void (*func)( char *file, int status, time_t t ) )
{
       FILENAME f;
       char filespec[ MAXJPATH ];
       char filename[ MAXJPATH ];
       long handle;
       int ret;
//       struct _find_t finfo[1];

     HDIR          hdirFindHandle = HDIR_CREATE;
     FILEFINDBUF3  FindBuffer     = {0};      /* Returned from FindFirst/Next */
     ULONG         ulResultBufLen = sizeof(FILEFINDBUF3);
     ULONG         ulFindCount    = 1;        /* Look for 1 file at a time    */
     APIRET        rc             = NO_ERROR; /* Return code                  */


       /* First enter directory itself */

       memset( (char *)&f, '\0', sizeof( f ) );

       f.f_dir.ptr = dir;
       f.f_dir.len = strlen(dir);

       dir = *dir ? dir : ".";

       /* Special case \ or d:\ : enter it */

       strcpy( filespec, dir );

       if( f.f_dir.len == 1 && f.f_dir.ptr[0] == '\\' )
           (*func)( dir, 0 /* not stat()'ed */, (time_t)0 );
       else if( f.f_dir.len == 3 && f.f_dir.ptr[1] == ':' )
           (*func)( dir, 0 /* not stat()'ed */, (time_t)0 );
       else
           strcat( filespec, "/" );

       strcat( filespec, "*" );

       /* Now enter contents of directory */

       if( DEBUG_BINDSCAN )
           printf( "scan directory %s\n", filespec );

       /* Time info in dos find_t is not very useful.  It consists */
       /* of a separate date and time, and putting them together is */
       /* not easy.  So we leave that to a later stat() call. */

     rc = DosFindFirst( filespec,             /* File pattern     */
                        &hdirFindHandle,      /* Directory search handle      */
   FILE_DIRECTORY|FILE_NORMAL|FILE_READONLY,  /* Search attribute             */
                        &FindBuffer,          /* Result buffer                */
                        ulResultBufLen,       /* Result buffer length         */
                        &ulFindCount,         /* Number of entries to find    */
                        FIL_STANDARD);        /* Return Level 1 file info     */
    if(rc != NO_ERROR)
    {   int dl=0;
        if(rc == ERROR_FILE_NOT_FOUND || rc == ERROR_PATH_NOT_FOUND) dl = 1;
        if(!dl)  printf(" DosFindFirst error rc=%i\n",rc);
        return;
    }


//       if( !_dos_findfirst( filespec, _A_NORMAL|_A_RDONLY|_A_SUBDIR, finfo ) )
       {
           do
           {
               f.f_base.ptr = FindBuffer.achName;
               f.f_base.len = strlen( FindBuffer.achName );

               file_build( &f, filename, 0 );

               (*func)( filename, 0 /* not stat()'ed */, (time_t)0 );

    rc = DosFindNext(hdirFindHandle,      /* Directory handle             */
                     &FindBuffer,         /* Result buffer                */
                     ulResultBufLen,      /* Result buffer length         */
                     &ulFindCount);       /* Number of entries to find    */

           }
           while( rc == NO_ERROR );
       }

}

/*
 * file_time() - get timestamp of file, if not done by file_dirscan()
 */

int
file_time(
       char    *filename,
       time_t  *time )
{
       /* This is called on OS2, not NT.  */
       /* NT fills in the time in the dirscan. */

       struct stat statbuf;

       if( stat( filename, &statbuf ) < 0 )
           return -1;

       *time = statbuf.st_mtime;

       return 0;
}

void
file_archscan(
       char *archive,
       void (*func)( char *file, int status, time_t t ) )
{
}

# endif /* OS2 && !__EMX__ */

