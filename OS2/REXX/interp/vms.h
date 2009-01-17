/* vms configuration - it was easier to separate it from configur.h */
#if !defined(_VMS_H_INCLUDED)
# define _VMS_H_INCLUDED

# include <unixlib.h>
# define fork() vfork()      /* necessary */
# include <processes.h>      /* should pull in vfork() */
# if defined(__DECC)
#   define HAVE_SETJMP_H
#   define HAVE_STRING_H
#   define HAVE_UNISTD_H
#   define HAVE_LIMITS_H
#   define HAVE_SYS_SOCKET_H
#   define HAVE_NETINET_IN_H
#   define HAVE_NETDB_H
#   if __CRTL_VER >= 70000000
#     include <fcntl.h>      /* necessary for posix_do_command */
#     include <sys/utsname.h>
#     define HAVE_PUTENV
#     define HAVE_FTRUNCATE
#     define HAVE_RANDOM
#     define HAVE_USLEEP
#     if !defined _VMS_V6_SOURCE
#       define HAVE_GETTIMEOFDAY
#     endif
#   else
#     include "utsname.h"    /* for unxfuncs */
#   endif
#   define HAVE_FTIME
#   define HAVE_MEMCPY
#   define HAVE_MEMMOVE
#   define HAVE_STRERROR
#   define HAVE_VFPRINTF
#   define TIME_WITH_SYS_TIME
#   define _XOPEN_SOURCE_EXTENDED
# endif
# ifndef _MAX_PATH
#   define _MAX_PATH PATH_MAX
# endif
# ifdef DYNAMIC
#   define DYNAMIC_VMS
# define HAVE_DIV
# define HAVE_RAISE
# endif

#endif
