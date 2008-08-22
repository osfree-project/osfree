#ifndef lint
static char *RCSid = "$Id: os2funcs.c,v 1.21 2004/01/17 07:36:50 mark Exp $";
#endif

/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef OS2
# define INCL_DOSPROCESS
#endif

#ifdef __EMX__
# define DONT_TYPEDEF_PFN
# include <io.h>
# include <os2emx.h>
# include <fcntl.h>
#endif

#if defined(__WATCOMC__) && defined(OS2)
# include <os2.h>
# define DONT_TYPEDEF_PFN
#endif

#if defined(__WATCOMC__) && !defined(__QNX__)
# include <i86.h>
# if defined(OS2)
#  define DONT_TYPEDEF_PFN
#  include <os2.h>
# endif
#endif

#include "rexx.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif
#include <time.h>

#if defined(VMS)
# include <stat.h>
#elif defined(OS2)
# include <sys/stat.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#elif defined(__WATCOMC__) || defined(_MSC_VER)
# include <sys/stat.h>
# include <fcntl.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
# if defined(_MSC_VER) && !defined(__WINS__)
#  include <direct.h>
#  include <io.h>
# endif
#elif defined(MAC)
# include "mac.h"
#else
# include <sys/stat.h>
# ifdef HAVE_PWD_H
#  include <pwd.h>
# endif
# ifdef HAVE_GRP_H
#  include <grp.h>
# endif
# include <fcntl.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif

#ifdef DJGPP
# include <pc.h>
# include <dir.h>
#endif

#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif

#ifdef WIN32
# if defined(__BORLANDC__)
#  include <dir.h>
# endif
# if defined(_MSC_VER)
#  if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#   pragma warning(disable: 4115 4201 4214 4514)
#  endif
# endif
# include <windows.h>
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
#   pragma warning(default: 4115 4201 4214)
#  endif
# endif
# if defined(__WATCOMC__)
#  include <io.h>
# endif
#endif

/*
 * Since development of Ultrix has ceased, and they never managed to
 * fix a few things, we want to define a few things, just in order
 * to kill a few warnings ...
 */
#if defined(FIX_PROTOS) && defined(FIX_ALL_PROTOS) && defined(ultrix)
   int fstat( int fd, struct stat *buf ) ;
   int stat( char *path, struct stat *buf ) ;
#endif

streng *os2_directory( tsd_t *TSD, cparamboxptr parms )
{
   streng *result=NULL ;
   int ok=HOOK_GO_ON ;
   char *path;

   checkparam(  parms,  0,  1 , "DIRECTORY" ) ;

   if (parms&&parms->value)
   {
      if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_SETCWD))
         ok = hookup_output( TSD, HOOK_SETCWD, parms->value ) ;

      if (ok==HOOK_GO_ON)
      {
         path = str_of( TSD, parms->value ) ;
         if (chdir( path ) )
         {
            FreeTSD( path ) ;
            return nullstringptr() ;
         }
         FreeTSD( path ) ;
      }
   }

   /*
    * The remainder of this is for obtaining the current working directory...
    */
   if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_GETCWD))
      ok = hookup_input( TSD, HOOK_GETCWD, &result ) ;

   if (ok==HOOK_GO_ON)
   {
      result = Str_makeTSD( REXX_PATH_MAX );
      my_fullpath( result->value, "." );
      result->len = strlen( result->value );
   }
   return result;
}

streng *os2_beep( tsd_t *TSD, cparamboxptr parms )
{
   int freq=0,dur=1;

   checkparam(  parms,  2,  1 , "BEEP" ) ;

   if (parms && parms->value)
   {
      freq = atopos( TSD, parms->value, "BEEP", 1 ) ;
      if (freq < 37 || freq > 32767)
         exiterror( ERR_INCORRECT_CALL, 0 );
   }
   if (parms->next && parms->next->value)
   {
      dur = atopos( TSD, parms->next->value, "BEEP", 2 ) ;
      if (dur < 1 || freq > 60000)
         exiterror( ERR_INCORRECT_CALL, 0 );
   }

#if defined(WIN32)
   Beep( (DWORD)freq, (DWORD)dur );
#elif defined (__EMX__)
   if (_osmode != DOS_MODE)
      DosBeep( freq, dur );
   else
   {
      int hdl;

      /* stdout and/or stderr may be redirected */
      if ((hdl = open("con", O_WRONLY)) != -1)
      {
         write(hdl, "\x07" /* ^G == bell */, 1);
         close(hdl);
      }
   }
#elif defined(DOS)
   putchar(7);
#elif defined(OS2)
   DosBeep( freq, dur );
#elif defined(__QNX__)
   printf("\a");
#elif defined(__WATCOMC__)
   sound( freq );
   delay( dur );
   nosound( );
#elif defined(__DJGPP__)
   sound( freq );
   delay( dur );
   nosound( );
#elif defined(__WINS__) || defined(__EPOC32__)
   beep( freq, dur );
#endif
   return nullstringptr();
}

streng *os2_filespec( tsd_t *TSD, cparamboxptr parms )
{
   streng *result=NULL ;
   streng *inpath=NULL;
   char format = '?' ;
#if defined(DJGPP)
   char fdrive[MAXDRIVE], fdir[MAXDIR], fname[MAXFILE], fext[MAXEXT];
#elif defined(__EMX__)
   char fdrive[_MAX_DRIVE], fdir[_MAX_DIR], fname[_MAX_FNAME], fext[_MAX_EXT];
#elif defined(HAVE__SPLITPATH2)
   char fpath[_MAX_PATH2], *fdrive=NULL, *fdir=NULL, *fname=NULL, *fext=NULL;
#elif defined(HAVE__SPLITPATH)
   char fdrive[_MAX_PATH], fdir[_MAX_PATH], fname[_MAX_PATH], fext[_MAX_PATH];
#else
   char fpath[REXX_PATH_MAX+5],*fdrive=NULL,*fdir=NULL,*fname=NULL,*fext=NULL;
#endif

   checkparam(  parms,  2,  2 , "FILESPEC" ) ;
   format = getoptionchar( TSD, parms->value, "FILESPEC", 1, "DNP", "?" ) ;
   inpath = Str_dupstrTSD( parms->next->value ) ;
#if defined(DJGPP)
   fnsplit( inpath->value, fdrive, fdir, fname, fext );
#elif defined(__EMX__)
   _splitpath( inpath->value, fdrive, fdir, fname, fext );
#elif defined(HAVE__SPLITPATH2)
   _splitpath2( inpath->value, fpath, &fdrive, &fdir, &fname, &fext );
#elif defined(HAVE__SPLITPATH)
   _splitpath( inpath->value, fdrive, fdir, fname, fext );
#else
   my_splitpath2( inpath->value, fpath, &fdrive, &fdir, &fname, &fext );
#endif
   switch( format )
   {
      case 'D':
         result = Str_creTSD( fdrive );
         break;
      case 'N':
         result = Str_makeTSD( strlen( fname) + strlen( fext ) );
         Str_catstrTSD( result, fname );
         Str_catstrTSD( result, fext );
         break;
      case 'P':
         result = Str_creTSD( fdir );
         break;
   }
   FreeTSD( inpath );
   return result;
}
