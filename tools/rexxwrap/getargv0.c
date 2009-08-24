/*
 * Copyright (C) 1999-2004  Mark Hessling <M.Hessling@qut.edu.au>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Mark Hessling  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 *
 */
#if defined(HAVE_CONFIG_H)
# include "config.h"
#else
# include "defines.h"
#endif

#include <stdio.h>
#if defined(HAVE_STDLIB_H)
# include <stdlib.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
# include <sys/types.h>
#endif
#if defined(HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif
#if defined(HAVE_MALLOC_H)
# include <malloc.h>
#endif
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#if defined(HAVE_STRING_H)
# include <string.h>
#endif

#if defined(_MSC_VER)
# include <fcntl.h>
# include <io.h>
#endif

#if defined(__OS2__)
#  define INCL_DOSPROCESS
#  define INCL_DOSMODULEMGR
#  include <os2.h>
#  undef __OS2__
#  include "rxdefines.h"
#  define __OS2__
#else
#  include "rxdefines.h"
#endif

/* RexxWrapper_GetArgv0 tries to find the fully qualified filename of the current program.
 * It uses some ugly and system specific tricks and it may return NULL if
 * it can't find any useful value.
 * The argument should be argv[0] of main() and it may be returned.
 * This function must not be called from another as the sole one when starting
 * up.
 * Borrowed from Regina
 */
char *RexxWrapper_GetArgv0(char *argv0)
{
#ifdef WIN32
   char buf[512];

   if (GetModuleFileName(NULL, buf, sizeof(buf)) != 0)
      return(strdup(buf)); /* never freed up */
#elif defined(__QNX__) && defined(__WATCOMC__)
   char buffer[PATH_MAX];
   char *buf;
   if ( (buf = _cmdname(buffer) ) != NULL )
      return(strdup(buf)); /* never freed up */
#elif defined(OS2)
   char buf[512];
   PPIB ppib;

# ifdef __EMX__
   if (_osmode == OS2_MODE)
   {
# endif
      if (DosGetInfoBlocks(NULL, &ppib) == 0)
         if (DosQueryModuleName(ppib->pib_hmte, sizeof(buf), buf) == 0)
            return(strdup(buf));
# ifdef __EMX__
   }
# endif
#endif

#ifdef HAVE_READLINK
   {
      /*
       * will work on Linux 2.1+
       */
      char buf[1024];
      int result;
      result = readlink("/proc/self/exe", buf, sizeof( buf ) );
      if ( ( result > 0 ) && ( result < sizeof( buf ) ) && ( buf[0] != '[' ) )
      {
         buf[result] = '\0';
         return strdup( buf );
      }
   }
#endif
   /* No specific code has found the right file name. Maybe, it's coded
    * in argv0. Check it, if it is an absolute path. Be absolutely sure
    * to detect it safely!
    */
   if (argv0 == NULL)
      return(NULL);

   if (argv0[0] == '/') /* unix systems and some others */
      return(argv0);

   if ((argv0[0] == '\\') && (argv0[1] == '\\')) /* MS and OS/2 UNC names */
      return(argv0);

   if (isalpha(argv0[0]) && (argv0[1] == ':') && (argv0[2] == '\\'))
      return(argv0); /* MS and OS/2 drive letter with path */

   return(NULL); /* not a proven argv0 argument */
}
