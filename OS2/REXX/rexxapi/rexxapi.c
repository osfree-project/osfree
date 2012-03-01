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
 * $Id: rexxapi.c,v 1.3 2005/09/01 10:23:31 mark Exp $
 */
#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif
#include "configur.h"

#include <stdio.h>

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

#include "rexxapi.h"

typedef HMODULE handle_type ;

#define FUNCTION_REXXREGISTEREXITEXE           0
#define FUNCTION_REXXREGISTEREXITDLL           1
#define FUNCTION_REXXDEREGISTEREXIT            2
#define FUNCTION_REXXQUERYEXIT                 3
#define FUNCTION_REXXREGISTERSUBCOMEXE         4
#define FUNCTION_REXXDEREGISTERSUBCOM          5
#define FUNCTION_REXXREGISTERSUBCOMDLL         6
#define FUNCTION_REXXQUERYSUBCOM               7
#define FUNCTION_REXXREGISTERFUNCTIONEXE       8
#define FUNCTION_REXXREGISTERFUNCTIONDLL       9
#define FUNCTION_REXXDEREGISTERFUNCTION       10
#define FUNCTION_REXXQUERYFUNCTION            11
#define FUNCTION_REXXCREATEQUEUE              12
#define FUNCTION_REXXDELETEQUEUE              13
#define FUNCTION_REXXADDQUEUE                 14
#define FUNCTION_REXXPULLQUEUE                15
#define FUNCTION_REXXQUERYQUEUE               16
#define FUNCTION_REXXADDMACRO                 17
#define FUNCTION_REXXDROPMACRO                18
#define FUNCTION_REXXSAVEMACROSPACE           19
#define FUNCTION_REXXLOADMACROSPACE           20
#define FUNCTION_REXXQUERYMACRO               21
#define FUNCTION_REXXREORDERMACRO             22
#define FUNCTION_REXXCLEARMACROSPACE          23

#define NUM_REXX_FUNCTIONS                    24

static char *MyFunctionName[ NUM_REXX_FUNCTIONS ] =
{
   /*  0 */  "RexxRegisterExitExe",
   /*  1 */  "RexxRegisterExitDll",
   /*  2 */  "RexxDeregisterExit",
   /*  3 */  "RexxQueryExit",
   /*  4 */  "RexxRegisterSubcomExe",
   /*  5 */  "RexxDeregisterSubcom",
   /*  6 */  "RexxRegisterSubcomDll",
   /*  7 */  "RexxQuerySubcom",
   /*  8 */  "RexxRegisterFunctionExe",
   /*  9 */  "RexxRegisterFunctionDll",
   /* 10 */  "RexxDeregisterFunction",
   /* 11 */  "RexxQueryFunction",
   /* 12 */  "RexxCreateQueue",
   /* 13 */  "RexxDeleteQueue",
   /* 14 */  "RexxQueryQueue",
   /* 15 */  "RexxAddQueue",
   /* 16 */  "RexxPullQueue",
   /* 17 */  "RexxAddMacro",
   /* 18 */  "RexxDropMacro",
   /* 19 */  "RexxSaveMacroSpace",
   /* 20 */  "RexxLoadMacroSpace",
   /* 21 */  "RexxQueryMacro",
   /* 22 */  "RexxReorderMacro",
   /* 23 */  "RexxClearMacroSpace",
};

/*
 * Typedefs for Regina
 */
typedef APIRET APIENTRY OREXXREGISTEREXITEXE(PSZ,PFN,PUCHAR ) ;
typedef APIRET APIENTRY OREXXREGISTEREXITDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG ) ;
typedef APIRET APIENTRY OREXXDEREGISTEREXIT(PSZ,PSZ ) ;
typedef APIRET APIENTRY OREXXQUERYEXIT(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET APIENTRY OREXXREGISTERSUBCOMEXE(PSZ,PFN,PUCHAR ) ;
typedef APIRET APIENTRY OREXXDEREGISTERSUBCOM(PSZ,PSZ ) ;
typedef APIRET APIENTRY OREXXREGISTERSUBCOMDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG ) ;
typedef APIRET APIENTRY OREXXQUERYSUBCOM(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET APIENTRY OREXXREGISTERFUNCTIONEXE(PSZ,PFN ) ;
typedef APIRET APIENTRY OREXXREGISTERFUNCTIONDLL(PSZ,PSZ,PSZ ) ;
typedef APIRET APIENTRY OREXXDEREGISTERFUNCTION(PSZ) ;
typedef APIRET APIENTRY OREXXQUERYFUNCTION(PSZ) ;
typedef APIRET APIENTRY OREXXCREATEQUEUE(PSZ,ULONG,PSZ,ULONG* ) ;
typedef APIRET APIENTRY OREXXDELETEQUEUE(PSZ ) ;
typedef APIRET APIENTRY OREXXQUERYQUEUE(PSZ,ULONG* ) ;
typedef APIRET APIENTRY OREXXADDQUEUE( PSZ,PRXSTRING,ULONG ) ;
typedef APIRET APIENTRY OREXXPULLQUEUE(PSZ,PRXSTRING,PDATETIME,ULONG );
typedef APIRET APIENTRY OREXXADDMACRO(PSZ,PSZ,ULONG );
typedef APIRET APIENTRY OREXXDROPMACRO(PSZ );
typedef APIRET APIENTRY OREXXSAVEMACROSPACE(ULONG,PSZ *,PSZ);
typedef APIRET APIENTRY OREXXLOADMACROSPACE(ULONG ,PSZ *,PSZ);
typedef APIRET APIENTRY OREXXQUERYMACRO(PSZ,PUSHORT );
typedef APIRET APIENTRY OREXXREORDERMACRO(PSZ,ULONG );
typedef APIRET APIENTRY OREXXCLEARMACROSPACE(VOID );
OREXXREGISTEREXITEXE     *ORexxRegisterExitExe=NULL;
OREXXREGISTEREXITDLL     *ORexxRegisterExitDll=NULL;
OREXXDEREGISTEREXIT      *ORexxDeregisterExit=NULL;
OREXXQUERYEXIT           *ORexxQueryExit=NULL;
OREXXREGISTERSUBCOMEXE   *ORexxRegisterSubcomExe=NULL;
OREXXDEREGISTERSUBCOM    *ORexxDeregisterSubcom=NULL;
OREXXREGISTERSUBCOMDLL   *ORexxRegisterSubcomDll=NULL;
OREXXQUERYSUBCOM         *ORexxQuerySubcom=NULL;
OREXXREGISTERFUNCTIONEXE *ORexxRegisterFunctionExe=NULL;
OREXXREGISTERFUNCTIONDLL *ORexxRegisterFunctionDll=NULL;
OREXXDEREGISTERFUNCTION  *ORexxDeregisterFunction=NULL;
OREXXQUERYFUNCTION       *ORexxQueryFunction=NULL;
OREXXCREATEQUEUE         *ORexxCreateQueue=NULL;
OREXXDELETEQUEUE         *ORexxDeleteQueue=NULL;
OREXXQUERYQUEUE          *ORexxQueryQueue=NULL;
OREXXADDQUEUE            *ORexxAddQueue=NULL;
OREXXPULLQUEUE           *ORexxPullQueue=NULL;
OREXXADDMACRO            *ORexxAddMacro=NULL;
OREXXDROPMACRO           *ORexxDropMacro=NULL;
OREXXSAVEMACROSPACE      *ORexxSaveMacroSpace=NULL;
OREXXLOADMACROSPACE      *ORexxLoadMacroSpace=NULL;
OREXXQUERYMACRO          *ORexxQueryMacro=NULL;
OREXXREORDERMACRO        *ORexxReorderMacro=NULL;
OREXXCLEARMACROSPACE     *ORexxClearMacroSpace=NULL;

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
            case FUNCTION_REXXREGISTERFUNCTIONEXE:  ORexxRegisterFunctionExe =   (OREXXREGISTERFUNCTIONEXE  *)addr; break;
            case FUNCTION_REXXREGISTERFUNCTIONDLL:  ORexxRegisterFunctionDll =   (OREXXREGISTERFUNCTIONDLL  *)addr; break;
            case FUNCTION_REXXDEREGISTERFUNCTION:   ORexxDeregisterFunction =    (OREXXDEREGISTERFUNCTION   *)addr; break;
            case FUNCTION_REXXREGISTEREXITEXE:      ORexxRegisterExitExe =       (OREXXREGISTEREXITEXE      *)addr; break;
            case FUNCTION_REXXREGISTEREXITDLL:      ORexxRegisterExitDll =       (OREXXREGISTEREXITDLL      *)addr; break;
            case FUNCTION_REXXDEREGISTEREXIT:       ORexxDeregisterExit =        (OREXXDEREGISTEREXIT       *)addr; break;
            case FUNCTION_REXXQUERYEXIT:            ORexxQueryExit =             (OREXXQUERYEXIT            *)addr; break;
            case FUNCTION_REXXREGISTERSUBCOMEXE:    ORexxRegisterSubcomExe =     (OREXXREGISTERSUBCOMEXE    *)addr; break;
            case FUNCTION_REXXDEREGISTERSUBCOM:     ORexxDeregisterSubcom =      (OREXXDEREGISTERSUBCOM     *)addr; break;
            case FUNCTION_REXXREGISTERSUBCOMDLL:    ORexxRegisterSubcomDll =     (OREXXREGISTERSUBCOMDLL    *)addr; break;
            case FUNCTION_REXXQUERYSUBCOM:          ORexxQuerySubcom =           (OREXXQUERYSUBCOM          *)addr; break;
            case FUNCTION_REXXQUERYFUNCTION:        ORexxQueryFunction =         (OREXXQUERYFUNCTION        *)addr; break;
            case FUNCTION_REXXCREATEQUEUE:          ORexxCreateQueue=            (OREXXCREATEQUEUE          *)addr; break;
            case FUNCTION_REXXDELETEQUEUE:          ORexxDeleteQueue=            (OREXXDELETEQUEUE          *)addr; break;
            case FUNCTION_REXXQUERYQUEUE:           ORexxQueryQueue=             (OREXXQUERYQUEUE           *)addr; break;
            case FUNCTION_REXXADDQUEUE:             ORexxAddQueue=               (OREXXADDQUEUE             *)addr; break;
            case FUNCTION_REXXPULLQUEUE:            ORexxPullQueue=              (OREXXPULLQUEUE            *)addr; break;
            case FUNCTION_REXXADDMACRO:             ORexxAddMacro=               (OREXXADDMACRO             *)addr; break;
            case FUNCTION_REXXDROPMACRO:            ORexxDropMacro=              (OREXXDROPMACRO            *)addr; break;
            case FUNCTION_REXXSAVEMACROSPACE:       ORexxSaveMacroSpace=         (OREXXSAVEMACROSPACE       *)addr; break;
            case FUNCTION_REXXLOADMACROSPACE:       ORexxLoadMacroSpace=         (OREXXLOADMACROSPACE       *)addr; break;
            case FUNCTION_REXXQUERYMACRO:           ORexxQueryMacro=             (OREXXQUERYMACRO           *)addr; break;
            case FUNCTION_REXXREORDERMACRO:         ORexxReorderMacro=           (OREXXREORDERMACRO         *)addr; break;
            case FUNCTION_REXXCLEARMACROSPACE:      ORexxClearMacroSpace=        (OREXXCLEARMACROSPACE      *)addr; break;
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

   if ( DosScanEnv( "REXXAPI_TRACEFILE", (PSZ *)&ptr ) )
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
 * System Exit functions
 */
APIRET APIENTRY RexxRegisterExitExe(
   PCSZ EnvName,
   RexxExitHandler *EntryPoint,
   PUCHAR UserArea )
{
   APIRET rc=RXEXIT_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: EnvName \"%s\" EntryPoint %x UserArea %x ",
         "RexxRegisterExitExe()",
         EnvName,
         EntryPoint,
         UserArea );
   }
   if (ORexxRegisterExitExe)
   {
      rc = (*ORexxRegisterExitExe)(
         (PSZ)       EnvName,
         (PFN)       EntryPoint,
         (PUCHAR)    UserArea );
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxRegisterExitDll(
   PCSZ EnvName,
   PCSZ ModuleName,
   PCSZ ProcedureName,
   PUCHAR UserArea,
   ULONG DropAuth )
{
   APIRET rc=RXEXIT_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: EnvName \"%s\" ModuleName \"%s\" ProcedureName \"%s\" UserArea %x DropAuth %d ",
         "RexxRegisterExitDll()",
         EnvName,
         ModuleName,
         ProcedureName,
         UserArea,
         DropAuth );
   }
   if (ORexxRegisterExitDll)
   {
      rc = (*ORexxRegisterExitDll)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName,
         (PSZ)       ProcedureName,
         (PUCHAR)    UserArea,
         (ULONG)     DropAuth );
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxDeregisterExit(
   PCSZ EnvName,
   PCSZ ModuleName )
{
   APIRET rc=RXEXIT_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: EnvName \"%s\" ModuleName \"%s\" ",
         "RexxDeregisterExit()",
         EnvName,
         (ModuleName == NULL) ? "NULL" : ModuleName );
   }
   if (ORexxDeregisterExit)
   {
      rc = (*ORexxDeregisterExit)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName );
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQueryExit(
   PCSZ EnvName,
   PCSZ ModuleName,
   PUSHORT Flag,
   PUCHAR UserArea )
{
   APIRET rc=RXEXIT_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: EnvName \"%s\" ModuleName \"%s\" Flag %d UserArea %x ",
         "RexxQueryExit()",
         EnvName,
         ModuleName,
         Flag,
         UserArea );
   }
   if (ORexxQueryExit)
   {
      rc = (*ORexxQueryExit)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName,
         (PUSHORT)   Flag,
         (PUCHAR)    UserArea ) ;
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/*
 * Subcommands
 */
APIRET APIENTRY RexxRegisterSubcomExe(
   PCSZ EnvName,
   RexxSubcomHandler *EntryPoint,
   PUCHAR UserArea )
{
   APIRET rc=RXSUBCOM_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: EnvName \"%s\" EntryPoint %x UserArea %x ",
         "RexxRegisterSubcomExe()",
         EnvName,
         EntryPoint,
         UserArea );
   }
   if (ORexxRegisterSubcomExe)
   {
      rc = (*ORexxRegisterSubcomExe)(
         (PSZ)       EnvName,
         (PFN)       EntryPoint,
         (PUCHAR)    UserArea );
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxDeregisterSubcom(
   PCSZ EnvName,
   PCSZ ModuleName )
{
   APIRET rc=RXSUBCOM_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: EnvName \"%s\" ModuleName \"%s\" ",
         "RexxDeregisterSubcom()",
         EnvName,
         (ModuleName == NULL) ? "NULL" : ModuleName );
   }
   if (ORexxDeregisterSubcom)
   {
      rc = (*ORexxDeregisterSubcom)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName );
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxRegisterSubcomDll(
   PCSZ EnvName,
   PCSZ ModuleName,
   PCSZ ProcedureName,
   PUCHAR UserArea,
   ULONG DropAuth )
{
   APIRET rc=RXSUBCOM_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: EnvName \"%s\" ModuleName \"%s\" ProcedureName \"%s\" UserArea %x DropAuth %d ",
         "RexxQuerySubcom()",
         EnvName,
         ModuleName,
         ProcedureName,
         UserArea,
         DropAuth );
   }
   if (ORexxRegisterSubcomDll)
   {
      rc = (*ORexxRegisterSubcomDll)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName,
         (PSZ)       ProcedureName,
         (PUCHAR)    UserArea,
         (ULONG)     DropAuth );
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQuerySubcom(
   PCSZ Envname,
   PCSZ ModuleName,
   PUSHORT Flag,
   PUCHAR UserArea )
{
   APIRET rc=RXSUBCOM_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: EnvName \"%s\" ModuleName \"%s\" Flag %d UserArea %x ",
         "RexxQuerySubcom()",
         Envname,
         ModuleName,
         Flag,
         UserArea );
   }
   if (ORexxQuerySubcom)
   {
      rc = (*ORexxQuerySubcom)(
         (PSZ)       Envname,
         (PSZ)       ModuleName,
         (PUSHORT)   Flag,
         (PUCHAR)    UserArea );
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/*
 * External functions
 */
APIRET APIENTRY RexxRegisterFunctionExe(
   PCSZ name,
   RexxFunctionHandler *EntryPoint )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: Name \"%s\" Entrypoint %x ",
         "RexxRegisterFunctionExe()",
         name,
         EntryPoint );

   }
   if (ORexxRegisterFunctionExe)
      rc = (*ORexxRegisterFunctionExe)(
         (PSZ)       name,
         (PFN)       EntryPoint );
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxRegisterFunctionDll(
   PCSZ ExternalName,
   PCSZ LibraryName,
   PCSZ InternalName )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: External %s Library %s Internal %s ",
         "RexxRegisterFunctionDll()",
         ExternalName,
         LibraryName,
         InternalName );

   }
   if (ORexxRegisterFunctionDll)
      rc = (*ORexxRegisterFunctionDll)(
         (PSZ)       ExternalName,
         (PSZ)       LibraryName,
         (PSZ)       InternalName );
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxDeregisterFunction(
   PCSZ name )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: %s ",
         "RexxDeregisterFunction()",
         name );

   }
   if (ORexxDeregisterFunction)
      rc = (*ORexxDeregisterFunction)(
         (PSZ)       name );
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQueryFunction(
   PCSZ name )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: %s ",
         "RexxQueryFunction()",
         name );

   }
   if (ORexxQueryFunction)
      rc = (*ORexxQueryFunction)(
         (PSZ)       name );
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/* ============================================================= */
/* Named queue interface */

APIRET APIENTRY RexxCreateQueue( PSZ Buffer,
                                 ULONG BuffLen,
                                 PSZ RequestedName,
                                 ULONG* DupFlag)
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: Buffer: %x BuffLen: %d RequestedName: \"%s\" DupFlag: %ld ",
         "RexxCreateQueue()",
         Buffer,
         BuffLen,
         RequestedName,
         DupFlag );

   }
   if (ORexxCreateQueue)
   {
      rc = (*ORexxCreateQueue)(
         (PSZ)       Buffer,
         (ULONG)     BuffLen,
         (PSZ)       RequestedName,
         (ULONG*)    DupFlag );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d ", rc );
      if ( rc == RXQUEUE_OK )
         TraceString( "Buffer: \"%s\" DupFlag: %d\n", rc );
      else
         TraceString( "\n" );
   }
   return rc;
}

APIRET APIENTRY RexxDeleteQueue( PSZ QueueName )
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: QueueName: \"%s\" ",
         "RexxDeleteQueue()",
         QueueName );

   }
   if (ORexxDeleteQueue)
   {
      rc = (*ORexxDeleteQueue)(
         (PSZ)       QueueName );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQueryQueue( PSZ QueueName,
                                ULONG* Count)
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: QueueName: \"%s\" Count: %x ",
         "RexxQueryQueue()",
         QueueName,
         Count );

   }
   if (ORexxQueryQueue)
   {
      rc = (*ORexxQueryQueue)(
         (PSZ)       QueueName,
         (ULONG*)    Count );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d ", rc );
      if ( rc == RXQUEUE_OK )
         TraceString( "Count: %ld\n", *Count );
      else
         TraceString( "\n" );
   }
   return rc;
}

APIRET APIENTRY RexxAddQueue( PSZ QueueName,
                              PRXSTRING EntryData,
                              ULONG AddFlag)
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: QueueName: \"%s\" AddFlag: %ld ",
         "RexxAddQueue()",
         QueueName,
         AddFlag );

   }
   if (ORexxAddQueue)
   {
      rc = (*ORexxAddQueue)(
         (PSZ)       QueueName,
         (PRXSTRING) EntryData,
         (ULONG)     AddFlag );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxPullQueue( PSZ QueueName,
                               PRXSTRING DataBuf,
                               PDATETIME TimeStamp,
                               ULONG WaitFlag)
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: QueueName: \"%s\" DataBuf: %x TimeStamp: %x WaitFlag: %ld ",
         "RexxPullQueue()",
         QueueName,
         DataBuf,
         TimeStamp,
         WaitFlag );

   }
   if (ORexxPullQueue)
   {
      rc = (*ORexxPullQueue)(
         (PSZ)       QueueName,
         (PRXSTRING) DataBuf,
         (PDATETIME) TimeStamp,
         (ULONG)     WaitFlag );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
      if ( rc == RXQUEUE_OK )
         TraceString( "DataBuf->strlength: %ld DataBuf->strptr: \"%s\"\n",
            DataBuf->strlength,
            DataBuf->strptr );
      else
         TraceString( "\n" );
   }
   return rc;
}

/* ============================================================= */
/* MacroSpace Rexx API interface */

APIRET APIENTRY RexxAddMacro( PSZ FuncName,
                              PSZ SourceFile,
                              ULONG Position )
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: FuncName: \"%s\" SourceFile: \"%s\" Position: %ld ",
         "RexxAddMacro()",
         FuncName,
         SourceFile,
         Position );

   }
   if (ORexxAddMacro)
   {
      rc = (*ORexxAddMacro)(
         (PSZ)       FuncName,
         (PSZ)       SourceFile,
         (ULONG)     Position );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxDropMacro( PSZ FuncName)
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: FuncName: \"%s\" ",
         "RexxDropMacro()",
         FuncName );

   }
   if (ORexxDropMacro)
   {
      rc = (*ORexxDropMacro)(
         (PSZ)       FuncName );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxSaveMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile)
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      unsigned int i;

      TraceString( "%s: FuncCount %d MacroLibFile \"%s\" ",
         "RexxSaveMacroSpace()",
         FuncCount, MacroLibFile );
      if ( FuncCount && FuncNames )
      {
         for ( i = 0; i < FuncCount; i++ )
         {
            TraceString( "%s: FuncName \"%s\" ",
               "RexxSaveMacroSpace()",
               FuncNames[i] );
         }
      }
   }
   if (ORexxSaveMacroSpace)
   {
      rc = (*ORexxSaveMacroSpace)(
         (ULONG)     FuncCount,
         (PSZ*)      FuncNames,
         (PSZ)       MacroLibFile
          );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxLoadMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile)
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      unsigned int i;

      TraceString( "%s: FuncCount %d MacroLibFile \"%s\" ",
         "RexxLoadMacroSpace()",
         FuncCount, MacroLibFile );
      if ( FuncCount && FuncNames )
      {
         for ( i = 0; i < FuncCount; i++ )
         {
            TraceString( "%s: FuncName \"%s\" ",
               "RexxLoadMacroSpace()",
               FuncNames[i] );
         }
      }
   }
   if (ORexxLoadMacroSpace)
   {
      rc = (*ORexxLoadMacroSpace)(
         (ULONG)     FuncCount,
         (PSZ*)      FuncNames,
         (PSZ)       MacroLibFile
          );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQueryMacro( PSZ FuncName,
                                PUSHORT Position )
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: FuncName: \"%s\" Position: %x ",
         "RexxQueryMacro()",
         FuncName,
         Position );

   }
   if (ORexxQueryMacro)
   {
      rc = (*ORexxQueryMacro)(
         (PSZ)       FuncName,
         (PUSHORT)   Position );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
      if ( rc == RXMACRO_OK )
         TraceString( "Position: %d\n",
            *Position );
      else
         TraceString( "\n" );
   }
   return rc;
}

APIRET APIENTRY RexxReorderMacro( PSZ FuncName,
                                  ULONG Position )
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s: FuncName: \"%s\" Position: %ld",
         "RexxReorderMacro()",
         FuncName,
         Position );

   }
   if (ORexxReorderMacro)
   {
      rc = (*ORexxReorderMacro)(
         (PSZ)       FuncName,
         (ULONG)     Position );
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxClearMacroSpace( VOID )
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s:",
         "RexxClearMacroSpace()" );

   }
   if (ORexxClearMacroSpace)
      rc = (*ORexxClearMacroSpace)(
           );
   if (Trace)
   {
      TraceString( ": Result: %x\n", rc );
   }
   return rc;
}
