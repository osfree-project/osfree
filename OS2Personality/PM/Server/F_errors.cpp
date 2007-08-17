/* 
 $Id: F_errors.cpp,v 1.2 2002/09/06 12:29:45 evgen2 Exp $ 
*/
/* F_errors.cpp */
/* Error codes and messages  */
/* ver 0.00 21.08.2002       */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include "F_def.hpp"
#include "F_config.hpp"
#include "F_globals.hpp"
/*
 #include <OS2WERR.H> // Warp ToolKit
 I don't have it, so uncomment. Only NO_ERROR seems missing. Trying with NO_ERROR from bseerr.h.
 Trying ERROR_SUCCESS from winerror.h, it's probably wrong but will do for now. 
 Can't find EBADMODE, EOS2ERR just put it in with something.
 Oh, a lot more of them is also needed...
 */
 #include <error.h> /* error.h is from an older toolkit, version 1.3*/
#include <bseerr.h> 
 
/* #define NO_ERROR          0 */

/*#define ERROR_INVALID_FUNCTION -3
#define EBADNAME               -4
#define ERROR_FILE_NOT_FOUND   -5
#define EISTEMPMEM             -6
#define ERROR_PATH_NOT_FOUND   -7
ERROR_TOO_MANY_OPEN_FILES
*/



/*
#define NO_ERROR          0
#define ERROR_SUCCESS     0L
#define EBADMODE          -1
#define EOS2ERR			  -2
#define ERROR_INVALID_FUNCTION -3
#define EBADNAME               -4
#define ERROR_FILE_NOT_FOUND   -5
#define EISTEMPMEM             -6
#define ERROR_PATH_NOT_FOUND   -7
#define ERROR_TOO_MANY_OPEN_FILES  -8
#define ERROR_ACCESS_DENIED        -9
#define EBADSHARE              -10
#define ERROR_INVALID_HANDLE   -11
#define EBUFMODE               -12
#define ERROR_ARENA_TRASHED    -13
#define ERROR_NOT_ENOUGH_MEMORY -14
#define EERRSET    				-15
#define ERROR_INVALID_BLOCK	    -16
#define EISOPEN                 -17
#define ERROR_BAD_ENVIRONMENT   -18
#define ENOTEXIST               -19
#define ENOTINIT                -20
#define ERROR_BAD_FORMAT        -21
*/







#include "os2_ErrCodes.h"

#include "FreePM_err.hpp"
#include "FreePM_ErrCodes.h"
/* #include "snprintf.h" */

char * GetOS2ErrorMessage(int ierr)
{ int i;
static char *Unknown = "Unknown";
  for(i=0; i< sizeof(OS2_ErrMessages) / sizeof(struct OS2_ErrMsg); i++)
  {  if(ierr == OS2_ErrMessages[i].ierrcode)
          return OS2_ErrMessages[i].msg;
  }
  return Unknown;
}

char * GetFreePMErrorMessage(int ierr)
{ int i;
static char *Unknown = "Unknown";
  for(i=0; i< sizeof(FreePM_ErrMessages) / sizeof(struct OS2_ErrMsg); i++)
  {  if(ierr == FreePM_ErrMessages[i].ierrcode)
          return FreePM_ErrMessages[i].msg;
  }
  return Unknown;
}

#if USE_SOCKETS

/*
 * xstrerror() - sock_errno/strerror-like wrapper
 */
const char *
xstrerror_a(void)

{   int ierr;
    ierr = sock_errno();
    return xbstrerror_i(ierr);
}
#endif
    /* USE_SOCKETS */

const char *
xstrerror_type(int type)
{
#if USE_SOCKETS
   if(type == FD_SOCKET)
      return xstrerror_a();
#endif
   return xstdio_strerror();
}

/*  xstdio_strerror() - strerror() wrapper */
const char *
xstdio_strerror(void)
{   static char xstrerror_buf[BUFSIZ];
    int fix_errno;
    fix_errno = errno;
    if (fix_errno < 0 || fix_errno >= EOS2ERR+1)
       snprintf(xstrerror_buf, BUFSIZ, "(%d) Unknown", fix_errno);
    else
    {
       if(EOS2ERR == fix_errno)
       {  int i,ierr;
          ierr = _doserrno;

          for(i=0; i< sizeof(OS2_ErrMessages) / sizeof(struct OS2_ErrMsg); i++)
           {  if(ierr == OS2_ErrMessages[i].ierrcode)
               {  snprintf(xstrerror_buf, BUFSIZ, "OS/2 error(%d) %s", ierr, OS2_ErrMessages[i].msg);
                  return xstrerror_buf;
               }
           }
           snprintf(xstrerror_buf, BUFSIZ, "Unknown errcode: %d OS/2 err=%d", fix_errno, ierr);
       } else
            snprintf(xstrerror_buf, BUFSIZ, "%s (%d)",  StdStrError(fix_errno),fix_errno);
    }
    return xstrerror_buf;
}

/* XXX strerror - work only with VAC installed */
const char *StdStrError(int ierr)
{  int i;
static  char buf[128];
   for(i=0; i< sizeof(OS2_StdErrMessages) / sizeof(struct OS2_ErrMsg); i++)
   {  if(ierr == OS2_StdErrMessages[i].ierrcode)
            return   OS2_StdErrMessages[i].msg;
   }
   snprintf(buf,128,"There is no message for error %i",ierr);
   return &buf[0];
}



