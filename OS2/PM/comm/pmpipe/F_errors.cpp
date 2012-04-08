/* F_errors.cpp */
/* Error codes and messages  */
/* ver 0.00 21.08.2002       */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#include "FreePM.hpp"

#include "FreePM_err.hpp"
#include "os2_ErrCodes.h"
#include "FreePM_ErrCodes.h"

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



