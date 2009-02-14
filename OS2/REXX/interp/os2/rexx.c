/*
 *  Copyright (C) 2004  Mark Hessling   <M.Hessling@qut.com.au>
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

/*
 * $Id: rexx.c,v 1.2 2004/06/06 11:14:01 mark Exp $
 */
#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif
#include "configur.h"

#include <stdio.h>
#include <stdarg.h>

#define INCL_REXXSAA

#if defined(OS2)
# define INCL_DOSMODULEMGR
# define INCL_DOSMISC
# undef INCL_REXXSAA
# include <os2.h>
# define INCL_REXXSAA
# define DONT_TYPEDEF_PFN
# define DYNAMIC_OS2
#endif

#if defined(HAVE_ERRNO_H)
# include <errno.h>
#endif

#if defined(HAVE_MEMORY_H)
# include <memory.h>
#endif

#if defined(HAVE_STDARG_H)
# include <stdarg.h>
#endif

#if defined(HAVE_STDLIB_H)
# include <stdlib.h>
#endif

#undef __USE_BSD
#if defined(HAVE_STRING_H)
# include <string.h>
#endif

#if defined(HAVE_SYS_TYPES_H)
# include <sys/types.h>
#endif

#if defined(HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif

#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#include "rexxsaa.h"

typedef HMODULE handle_type ;

#define FUNCTION_REXXSTART                     0
#define FUNCTION_REXXVARIABLEPOOL              1
#define FUNCTION_REXXSETHALT                   2
#define FUNCTION_REXXSETTRACE                  3
#define FUNCTION_REXXRESETTRACE                4

#define NUM_REXX_FUNCTIONS                     5

static char *MyFunctionName[ NUM_REXX_FUNCTIONS ] =
{
   /*  0 */  "RexxStart",
   /*  1 */  "RexxVariablePool",
   /*  2 */  "RexxSetHalt",
   /*  3 */  "RexxSetTrace",
   /*  4 */  "RexxResetTrace",
};

/*
 * Typedefs for Regina
 */
typedef APIRET APIENTRY OREXXSTART(LONG,PRXSTRING,PSZ,PRXSTRING,PSZ,LONG,PRXSYSEXIT,PSHORT,PRXSTRING );
typedef APIRET APIENTRY OREXXVARIABLEPOOL(PSHVBLOCK );
typedef APIRET APIENTRY OREXXSETHALT(LONG,LONG );
typedef APIRET APIENTRY OREXXSETTRACE(LONG,LONG );
typedef APIRET APIENTRY OREXXRESETTRACE(LONG,LONG );
OREXXSTART               *ORexxStart=NULL;
OREXXVARIABLEPOOL        *ORexxVariablePool=NULL;
OREXXSETHALT             *ORexxSetHalt=NULL;
OREXXSETTRACE            *ORexxSetTrace=NULL;
OREXXRESETTRACE          *ORexxResetTrace=NULL;

static char TraceFileName[256];
static int Trace = 0;
static int InterpreterIdx = -1;

static void TraceString( char *fmt, ... )
{
   FILE *fp=NULL;
   int using_stderr = 0;
   va_list argptr;

   if ( strcmp( TraceFileName, "stderr" ) == 0 )
      using_stderr = 1;
   if ( using_stderr )
      fp = stderr;
   else
      fp = fopen( TraceFileName, "a" );
   if ( fp )
   {
      va_start( argptr, fmt );
      vfprintf( fp, fmt, argptr );
      va_end( argptr );
      if ( !using_stderr )
         fclose( fp );
   }
}

static handle_type FindInterpreter( char *library )
{
   handle_type handle=(handle_type)NULL ;
   CHAR LoadError[256];
   PFN addr;
   register int j=0;

   if ( Trace )
   {
      TraceString( "%s: Attempting to load \"%s\" using DosLoadModule()...",
                   "FindInterpreter()",
                   "REGINA");
   }
   if ( DosLoadModule( LoadError, sizeof(LoadError), library, &handle ) )
   {
      handle = (handle_type)NULL;
   }
   if ( handle != (handle_type)NULL )
   {
      if ( Trace )
      {
         TraceString( "found REGINA\n" );
      }
      for ( j = 0; j < NUM_REXX_FUNCTIONS; j++ )
      {
         if ( DosQueryProcAddr( handle, 0L, MyFunctionName[j], &addr) )
         {
            addr = NULL;
         }
         /*
          * Log the function and address. This is useful if the module
          * doesn't have an address for this procedure.
          */
         if (Trace)
         {
            TraceString( "%s: Address %x\n",
               MyFunctionName[j],
               (addr == NULL) ? 0 : addr );
         }
         /*
          * Even if the function being processed is not in the module, its
          * address is still stored.  In this case it will simply be set
          * again to NULL.
          */
         switch ( j )
         {
            case FUNCTION_REXXSTART:                ORexxStart =                 (OREXXSTART                *)addr; break;
            case FUNCTION_REXXVARIABLEPOOL:         ORexxVariablePool =          (OREXXVARIABLEPOOL         *)addr; break;
            case FUNCTION_REXXSETHALT:              ORexxSetHalt=                (OREXXSETHALT              *)addr; break;
            case FUNCTION_REXXSETTRACE:             ORexxSetTrace =              (OREXXSETTRACE             *)addr; break;
            case FUNCTION_REXXRESETTRACE:           ORexxResetTrace =            (OREXXRESETTRACE           *)addr; break;
         }
      }
   }
   else
   {
      if ( Trace )
      {
         TraceString( "not found: %s\n", LoadError );
      }
   }
   return handle;
}

static void LoadInterpreter( void )
{
   handle_type handle=(handle_type)NULL ;
   char *ptr;

   if ( DosScanEnv( "REXX_TRACEFILE", (PSZ *)&ptr ) )
      ptr = NULL;
   if ( ptr != NULL )
   {
      Trace = 1;
      strcpy( TraceFileName, ptr );
   }

   handle = (handle_type)FindInterpreter( "REGINA" );
   if ( handle == (handle_type)NULL )
   {
      fprintf( stderr, "Could not find Regina DLL. Cannot continue.\n" );
      exit( 11 );
   }
   if ( Trace )
   {
      TraceString( "----------- Initialisation Complete - Program Execution Begins -----------\n" );
   }
   InterpreterIdx = 0;
   return;
}


/*
 * RexxStart
 */
APIRET APIENTRY RexxStart(
   LONG ArgCount,
   PRXSTRING ArgList,
   PCSZ ProgramName,
   PRXSTRING Instore,
   PCSZ EnvName,
   LONG CallType,
   PRXSYSEXIT Exits,
   PSHORT ReturnCode,
   PRXSTRING Result )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: ArgCount: %d ArgList: %x ProgramName: \"%s\" Instore: %x EnvName: \"%s\" Calltype: %d Exits: %x ReturnCode: %x Result: %x",
         "RexxStart()",
         ArgCount,
         ArgList,
         ProgramName,
         Instore,
         EnvName,
         CallType,
         Exits,
         ReturnCode,
         Result );
   }
   if (ORexxStart)
      rc = (*ORexxStart)(
         (LONG)      ArgCount,
         (PRXSTRING) ArgList,
         (PSZ)       ProgramName,
         (PRXSTRING) Instore,
         (PSZ)       EnvName,
         (LONG)      CallType,
         (PRXSYSEXIT)Exits,
         (PSHORT)    ReturnCode,
         (PRXSTRING) Result ) ;
   if (Trace)
   {
      TraceString( "<=> ReturnCode %d ResultString \"%s\" Result: %d\n",
                   (Result && Result->strptr) ? Result->strptr : "",
                   ReturnCode,
                   rc );
   }
   return rc;
}

/*
 * Variable pool
 */
APIRET APIENTRY RexxVariablePool(
   PSHVBLOCK RequestBlockList )
{
   APIRET rc=RXSHV_NOAVL;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      PSHVBLOCK tmp=RequestBlockList;
      TraceString( "%s: RequestBlockList %x\n",
         "RexxVariablePool()",
         RequestBlockList );
      while(tmp)
      {
         switch( tmp->shvcode )
         {
            case RXSHV_SET:
               TraceString("in    RXSHV_SET: shvname: \"%s\" shvnamelen: %d shvvalue: \"%s\" shvvaluelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen, tmp->shvvalue.strptr, tmp->shvvaluelen );
               break;
            case RXSHV_SYSET:
               TraceString("in  RXSHV_SYSET: shvname: \"%s\" shvnamelen: %d shvvalue: \"%s\" shvvaluelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen, tmp->shvvalue.strptr, tmp->shvvaluelen );
               break;
            case RXSHV_FETCH:
               TraceString("in  RXSHV_FETCH: shvname: \"%s\" shvnamelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen );
               break;
            case RXSHV_DROPV:
               TraceString("in  RXSHV_DROPV: shvname: \"%s\" shvnamelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen );
               break;
            case RXSHV_SYDRO:
               TraceString("in  RXSHV_SYDRO: shvname: \"%s\" shvnamelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen );
               break;
            case RXSHV_NEXTV:
               TraceString("in  RXSHV_NEXTV: shvname: \"%s\" shvnamelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen );
               break;
            case RXSHV_PRIV:
               TraceString("in   RXSHV_PRIV\n" );
               break;
            case RXSHV_EXIT:
               TraceString("in   RXSHV_EXIT\n" );
               break;
            default:
               break;
         }
         tmp = tmp->shvnext;
      }
   }
   if (ORexxVariablePool)
      rc = (*ORexxVariablePool)(
         (PSHVBLOCK) RequestBlockList);
   if (Trace)
   {
      PSHVBLOCK tmp=RequestBlockList;
      while(tmp)
      {
         switch( tmp->shvcode )
         {
            case RXSHV_SET:
               TraceString("out   RXSHV_SET: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_SYSET:
               TraceString("out RXSHV_SYSET: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_FETCH:
               TraceString("out RXSHV_FETCH: shvret: %x shvvalue: \"%s\" shvvaluelen: %d\n",
                  tmp->shvret, tmp->shvvalue.strptr, tmp->shvvaluelen );
               break;
            case RXSHV_DROPV:
               TraceString("out RXSHV_DROPV: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_SYDRO:
               TraceString("out RXSHV_SYDRO: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_NEXTV:
               TraceString("out RXSHV_NEXTV: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_PRIV:
               TraceString("out  RXSHV_PRIV: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_EXIT:
               TraceString("out  RXSHV_EXIT: shvret: %x\n",
                  tmp->shvret );
               break;
            default:
               break;
         }
         tmp = tmp->shvnext;
      }
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/* ============================================================= */
/* Asynchronous Request Rexx API interface */

APIRET APIENTRY RexxSetHalt( LONG pid,
                             LONG tid)
{
   APIRET rc=RXARI_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: pid: %ld tid: %ld ",
         "RexxSetHalt()",
         pid,
         tid );

   }
   if (ORexxSetHalt)
   {
      rc = (*ORexxSetHalt)(
         (LONG)      pid,
         (LONG)      tid);
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxSetTrace( LONG pid,
                              LONG tid)
{
   APIRET rc=RXARI_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: pid: %ld tid: %ld ",
         "RexxSetTrace()",
         pid,
         tid );

   }
   if (ORexxSetTrace)
   {
      rc = (*ORexxSetTrace)(
         (LONG)      pid,
         (LONG)      tid);
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxResetTrace( LONG pid,
                                LONG tid)
{
   APIRET rc=RXARI_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: pid: %ld tid: %ld ",
         "RexxResetTrace()",
         pid,
         tid );

   }
   if (ORexxResetTrace)
   {
      rc = (*ORexxResetTrace)(
         (LONG)      pid,
         (LONG)      tid);
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/*
 * The following functions do nothing; they are here because they are exported in the DLL
 */
APIRET APIENTRY RxBreakCleanup(
                VOID              )
{
   APIRET rc=0;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: ",
         "RxBreakCleanup()" );

   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n",
                   rc );
   }
   return rc;
}

APIRET APIENTRY RexxSAA(
                VOID              )
{
   APIRET rc=0;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: ",
         "RexxSAA()" );

   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n",
                   rc );
   }
   return rc;
}
