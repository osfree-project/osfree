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
#if defined(__OSFREE__)
#define OS2
#define HAVE_STRING_H
#define HAVE_STDLIB_H
#define HAVE_STDARG_H
#include <ctype.h>
#endif

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#ifndef __OSFREE__
#include "configur.h"
#endif

#include <stdio.h>

#if defined(OS2)
# define INCL_DOSSEMAPHORES
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

#define RXQUEUE_OK            0        /* Successful return           */
#define RXQUEUE_NOTREG        9        /* Queue does not exist        */

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

APIRET APIENTRY os2CreateQueue( PSZ Buffer,
                                ULONG BuffLen,
                                PSZ RequestedName,
                                ULONG* DupFlag );

APIRET APIENTRY os2DeleteQueue( PSZ QueueName );

APIRET APIENTRY os2QueryQueue( PSZ QueueName,
                               ULONG* Count );

APIRET APIENTRY os2AddQueue( PSZ QueueName,
                             PRXSTRING EntryData,
                             ULONG AddFlag );

APIRET APIENTRY os2PullQueue( PSZ QueueName,
                              PRXSTRING DataBuf,
                              PDATETIME TimeStamp,
                              ULONG WaitFlag );

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
   /* 14 */  "RexxAddQueue",
   /* 15 */  "RexxPullQueue",
   /* 16 */  "RexxQueryQueue",
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

int Trace = 0;
int InterpreterIdx = -1;

extern HMTX hmtx;

//static 
void TraceString( char *fmt, ... )
{
   FILE *fp=NULL;
   int using_stderr = 0;
   va_list argptr;

   if (! hmtx)
   {
      return;
   }
   
   DosRequestMutexSem(hmtx, SEM_INDEFINITE_WAIT);

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

   DosReleaseMutexSem(hmtx);
}

static handle_type FindInterpreter( char *library )
{
   handle_type handle=(handle_type)NULL ;
   CHAR LoadError[256];
   PFN addr;
   register int j=0;
   APIRET rc;
   
   log( "%s: Attempting to load \"%s\" using DosLoadModule()...",
                __FUNCTION__,
                library);

   if ( rc = DosLoadModule( LoadError, sizeof(LoadError), library, &handle ) )
   {
      handle = (handle_type)NULL;
   }

   if ( handle != (handle_type)NULL )
   {
      log( "found %s\n", library );
      
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
         log( "%s: Address %x\n",
                MyFunctionName[j],
                (addr == NULL) ? 0 : addr );

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
      log( "not found: %s\n", LoadError );
   }

   return handle;
}

void LoadInterpreter( void )
{
   handle_type handle=(handle_type)NULL ;
   char interpreter[9] = "REGINA";
   char *ptr;

   if ( DosScanEnv( "REXXAPI_TRACEFILE", (PSZ *)&ptr ) )
      ptr = NULL;

   if ( ptr != NULL )
   {
      Trace = 1;
      strcpy( TraceFileName, ptr );
   }

   /* REXX switcher feature */
   if ( DosScanEnv( "REXXAPI_DLL", (PSZ *)&ptr ) )
      ptr = NULL;
      
   if ( ptr && *ptr && strlen(ptr) < 9 )
      strcpy(interpreter, ptr);

   ptr = interpreter;

   while (*ptr)
   {
      *ptr++ = toupper(*ptr);
   }

   handle = (handle_type)FindInterpreter( interpreter );

   if ( handle == (handle_type)NULL )
   {
      fprintf( stderr, "Could not find %s DLL. Cannot continue.\n", interpreter );
      exit( 11 );
   }

   log( "----------- Initialisation Complete - Program Execution Begins -----------\n" );

   InterpreterIdx = 0;

   return;
}


/*
 * System Exit functions
 */
APIRET APIENTRY RexxRegisterExitExe(
   PCSZ EnvName,
   PFN EntryPoint,
   //RexxExitHandler *EntryPoint,
   PUCHAR UserArea )
{
   APIRET rc = RXEXIT_NOTREG;

   log( "%s: EnvName \"%s\" EntryPoint %x UserArea %x ",
          __FUNCTION__,
          EnvName,
          EntryPoint,
          UserArea );

   if (ORexxRegisterExitExe)
   {
      rc = (*ORexxRegisterExitExe)(
         (PSZ)       EnvName,
         (PFN)       EntryPoint,
         (PUCHAR)    UserArea );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxRegisterExitDll(
   PCSZ EnvName,
   PCSZ ModuleName,
   PCSZ ProcedureName,
   PUCHAR UserArea,
   ULONG DropAuth )
{
   APIRET rc = RXEXIT_NOTREG;

   log( "%s: EnvName \"%s\" ModuleName \"%s\" ProcedureName \"%s\" UserArea %x DropAuth %d ",
          __FUNCTION__,
          EnvName,
          ModuleName,
          ProcedureName,
          UserArea,
          DropAuth );

   if (ORexxRegisterExitDll)
   {
      rc = (*ORexxRegisterExitDll)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName,
         (PSZ)       ProcedureName,
         (PUCHAR)    UserArea,
         (ULONG)     DropAuth );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxDeregisterExit(
   PCSZ EnvName,
   PCSZ ModuleName )
{
   APIRET rc = RXEXIT_NOTREG;

   log( "%s: EnvName \"%s\" ModuleName \"%s\" ",
          __FUNCTION__,
          EnvName,
          (ModuleName == NULL) ? "NULL" : ModuleName );

   if (ORexxDeregisterExit)
   {
      rc = (*ORexxDeregisterExit)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxQueryExit(
   PCSZ EnvName,
   PCSZ ModuleName,
   PUSHORT Flag,
   PUCHAR UserArea )
{
   APIRET rc = RXEXIT_NOTREG;

   log( "%s: EnvName \"%s\" ModuleName \"%s\" Flag %d UserArea %x ",
          __FUNCTION__,
          EnvName,
          ModuleName,
          Flag,
          UserArea );

   if (ORexxQueryExit)
   {
      rc = (*ORexxQueryExit)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName,
         (PUSHORT)   Flag,
         (PUCHAR)    UserArea ) ;
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

/*
 * Subcommands
 */
APIRET APIENTRY RexxRegisterSubcomExe(
   PCSZ EnvName,
   PFN EntryPoint,
   //RexxSubcomHandler *EntryPoint,
   PUCHAR UserArea )
{
   APIRET rc = RXSUBCOM_NOTREG;

   log( "%s: EnvName \"%s\" EntryPoint %x UserArea %x ",
          __FUNCTION__,
          EnvName,
          EntryPoint,
          UserArea );

   if (ORexxRegisterSubcomExe)
   {
      rc = (*ORexxRegisterSubcomExe)(
         (PSZ)       EnvName,
         (PFN)       EntryPoint,
         (PUCHAR)    UserArea );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxDeregisterSubcom(
   PCSZ EnvName,
   PCSZ ModuleName )
{
   APIRET rc = RXSUBCOM_NOTREG;

   log( "%s: EnvName \"%s\" ModuleName \"%s\" ",
          __FUNCTION__,
          EnvName,
          (ModuleName == NULL) ? "NULL" : ModuleName );

   if (ORexxDeregisterSubcom)
   {
      rc = (*ORexxDeregisterSubcom)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxRegisterSubcomDll(
   PCSZ EnvName,
   PCSZ ModuleName,
   PCSZ ProcedureName,
   PUCHAR UserArea,
   ULONG DropAuth )
{
   APIRET rc = RXSUBCOM_NOTREG;

   log( "%s: EnvName \"%s\" ModuleName \"%s\" ProcedureName \"%s\" UserArea %x DropAuth %d ",
          __FUNCTION__,
          EnvName,
          ModuleName,
          ProcedureName,
          UserArea,
          DropAuth );

   if (ORexxRegisterSubcomDll)
   {
      rc = (*ORexxRegisterSubcomDll)(
         (PSZ)       EnvName,
         (PSZ)       ModuleName,
         (PSZ)       ProcedureName,
         (PUCHAR)    UserArea,
         (ULONG)     DropAuth );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxQuerySubcom(
   PCSZ Envname,
   PCSZ ModuleName,
   PUSHORT Flag,
   PUCHAR UserArea )
{
   APIRET rc = RXSUBCOM_NOTREG;

   log( "%s: EnvName \"%s\" ModuleName \"%s\" Flag %d UserArea %x ",
          __FUNCTION__,
          Envname,
          ModuleName,
          Flag,
          UserArea );

   if (ORexxQuerySubcom)
   {
      rc = (*ORexxQuerySubcom)(
         (PSZ)       Envname,
         (PSZ)       ModuleName,
         (PUSHORT)   Flag,
         (PUCHAR)    UserArea );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

/*
 * External functions
 */
APIRET APIENTRY RexxRegisterFunctionExe(
   PCSZ name,
   PFN EntryPoint )
   //RexxFunctionHandler *EntryPoint )
{
   APIRET rc = RXFUNC_NOTREG;

   log( "%s: Name \"%s\" Entrypoint %x ",
          __FUNCTION__,
          name,
          EntryPoint );

   if (ORexxRegisterFunctionExe)
   {
      rc = (*ORexxRegisterFunctionExe)(
              (PSZ)name,
              (PFN)EntryPoint );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxRegisterFunctionDll(
   PCSZ ExternalName,
   PCSZ LibraryName,
   PCSZ InternalName )
{
   APIRET rc = RXFUNC_NOTREG;

   log( "%s: External %s Library %s Internal %s ",
          __FUNCTION__,
          ExternalName,
          LibraryName,
          InternalName );

   if (ORexxRegisterFunctionDll)
   {
      rc = (*ORexxRegisterFunctionDll)(
              (PSZ)ExternalName,
              (PSZ)LibraryName,
              (PSZ)InternalName );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxDeregisterFunction(
   PCSZ name )
{
   APIRET rc = RXFUNC_NOTREG;

   log( "%s: %s ",
          __FUNCTION__,
          name );

   if (ORexxDeregisterFunction)
   {
      rc = (*ORexxDeregisterFunction)( (PSZ)name );
   }
     
   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxQueryFunction(
   PCSZ name )
{
   APIRET rc = RXFUNC_NOTREG;

   log( "%s: %s ",
          __FUNCTION__,
          name );

   if (ORexxQueryFunction)
   {
      rc = (*ORexxQueryFunction)( (PSZ)name );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

/* ============================================================= */
/* Named queue interface */

APIRET APIENTRY RexxCreateQueue( PSZ Buffer,
                                 ULONG BuffLen,
                                 PSZ RequestedName,
                                 ULONG* DupFlag)
{
   APIRET rc = RXQUEUE_NOTREG;

   log( "%s: Buffer: %x BuffLen: %d RequestedName: \"%s\" DupFlag: %ld ",
          __FUNCTION__,
          Buffer,
          BuffLen,
          RequestedName,
          DupFlag );

#if 1
   if (ORexxCreateQueue)
   {
      rc = (*ORexxCreateQueue)(
         (PSZ)       Buffer,
         (ULONG)     BuffLen,
         (PSZ)       RequestedName,
         (ULONG*)    DupFlag );
   }
#else
      rc = os2CreateQueue(
         (PSZ)       Buffer,
         (ULONG)     BuffLen,
         (PSZ)       RequestedName,
         (ULONG*)    DupFlag );
#endif

   log( "<=> Result: %d ", rc );

   if ( rc == RXQUEUE_OK )
      log( "Buffer: \"%s\" DupFlag: %d\n", rc );
   else
      log( "\n" );

   return rc;
}

APIRET APIENTRY RexxDeleteQueue( PSZ QueueName )
{
   APIRET rc = RXQUEUE_NOTREG;

   log( "%s: QueueName: \"%s\" ",
          __FUNCTION__,
          QueueName );

#if 1
   if (ORexxDeleteQueue)
   {
      rc = (*ORexxDeleteQueue)(
         (PSZ)       QueueName );
   }
#else
      rc = os2DeleteQueue(
         (PSZ)       QueueName );
#endif

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxQueryQueue( PSZ QueueName,
                                ULONG* Count)
{
   APIRET rc = RXQUEUE_NOTREG;

   log( "%s: QueueName: \"%s\" Count: %x ",
          __FUNCTION__,
          QueueName,
          Count );

#if 1
   if (ORexxQueryQueue)
   {
      rc = (*ORexxQueryQueue)(
         (PSZ)       QueueName,
         (ULONG*)    Count );
   }
#else
      rc = os2QueryQueue(
         (PSZ)       QueueName,
         (ULONG*)    Count );
#endif

   log( "<=> Result: %d ", rc );

   if ( rc == RXQUEUE_OK )
      log( "Count: %ld\n", *Count );
   else
      log( "\n" );

   return rc;
}

APIRET APIENTRY RexxAddQueue( PSZ QueueName,
                              PRXSTRING EntryData,
                              ULONG AddFlag)
{
   APIRET rc = RXQUEUE_NOTREG;

   log( "%s: QueueName: \"%s\" AddFlag: %ld ",
          __FUNCTION__,
          QueueName,
          AddFlag );

#if 1
   if (ORexxAddQueue)
   {
      rc = (*ORexxAddQueue)(
         (PSZ)       QueueName,
         (PRXSTRING) EntryData,
         (ULONG)     AddFlag );
   }
#else
      rc = os2AddQueue(
         (PSZ)       QueueName,
         (PRXSTRING) EntryData,
         (ULONG)     AddFlag );
#endif

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxPullQueue( PSZ QueueName,
                               PRXSTRING DataBuf,
                               PDATETIME TimeStamp,
                               ULONG WaitFlag)
{
   APIRET rc = RXQUEUE_NOTREG;

   log( "%s: QueueName: \"%s\" DataBuf: %x TimeStamp: %x WaitFlag: %ld ",
          __FUNCTION__,
          QueueName,
          DataBuf,
          TimeStamp,
          WaitFlag );

#if 1
   if (ORexxPullQueue)
   {
      rc = (*ORexxPullQueue)(
         (PSZ)       QueueName,
         (PRXSTRING) DataBuf,
         (PDATETIME) TimeStamp,
         (ULONG)     WaitFlag );
   }
#else
      rc = os2PullQueue(
         (PSZ)       QueueName,
         (PRXSTRING) DataBuf,
         (PDATETIME) TimeStamp,
         (ULONG)     WaitFlag );
#endif

   log( "<=> Result: %d\n", rc );

   if ( rc == RXQUEUE_OK )
      log( "DataBuf->strlength: %ld DataBuf->strptr: \"%s\"\n",
             DataBuf->strlength,
             DataBuf->strptr );
   else
      log( "\n" );

   return rc;
}

/* ============================================================= */
/* MacroSpace Rexx API interface */

APIRET APIENTRY RexxAddMacro( PSZ FuncName,
                              PSZ SourceFile,
                              ULONG Position )
{
   APIRET rc = RXMACRO_NOT_FOUND;

   if (FuncName && SourceFile)
   {
      log( "%s: FuncName: \"%s\" SourceFile: \"%s\" Position: %ld ",
           __FUNCTION__,
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

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxDropMacro( PSZ FuncName)
{
   APIRET rc = RXMACRO_NOT_FOUND;

   if (FuncName)
   {
      log( "%s: FuncName: \"%s\" ",
           __FUNCTION__,
           FuncName );
   }
   else
   {
      log( "%s: FuncName: <NULL> ",
           __FUNCTION__);
   }

   if (ORexxDropMacro)
   {
      rc = (*ORexxDropMacro)(
         (PSZ)       FuncName );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxSaveMacroSpace( ULONG FuncCount,
                                    //PSZ * FuncNames,
                                    PSZ const * FuncNames,
                                    PSZ MacroLibFile)
{
   APIRET rc = RXMACRO_NOT_FOUND;

   if (Trace)
   {
      unsigned int i;

      log( "%s: FuncCount %d MacroLibFile \"%s\" ",
             __FUNCTION__,
             FuncCount, MacroLibFile );

      if ( FuncCount && FuncNames )
      {
         for ( i = 0; i < FuncCount; i++ )
         {
            if (FuncNames[i])
            {
               log( "%s: FuncName \"%s\" ",
                    __FUNCTION__,
                    FuncNames[i] );
            }
            else
            {
               log( "%s: FuncName <NULL> ",
                    __FUNCTION__);
            }
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

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxLoadMacroSpace( ULONG FuncCount,
                                    //PSZ * FuncNames,
                                    PSZ const * FuncNames,
                                    PSZ MacroLibFile)
{
   APIRET rc = RXMACRO_NOT_FOUND;

   if (Trace)
   {
      unsigned int i;

      log( "%s: FuncCount %d MacroLibFile \"%s\" ",
             __FUNCTION__,
             FuncCount, MacroLibFile );

      if ( FuncCount && FuncNames )
      {
         for ( i = 0; i < FuncCount; i++ )
         {
            if (FuncNames[i])
            {
               log( "%s: FuncName \"%s\" ",
                    __FUNCTION__,
                    FuncNames[i] );
            }
            else
            {
               log( "%s: FuncName <NULL> ",
                    __FUNCTION__);
            }
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

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxQueryMacro( PSZ FuncName,
                                PUSHORT Position )
{
   APIRET rc = RXMACRO_NOT_FOUND;

   if (FuncName)
   {
      log( "%s: FuncName: \"%s\" Position: %x ",
           __FUNCTION__,
           FuncName,
           Position );
   }
   else
   {
      log( "%s: FuncName: <NULL> Position: %x ",
           __FUNCTION__,
           Position );
   }
   
   if (ORexxQueryMacro)
   {
      rc = (*ORexxQueryMacro)(
         (PSZ)       FuncName,
         (PUSHORT)   Position );
   }

   TraceString( "<=> Result: %d\n", rc );

   if ( rc == RXMACRO_OK )
      log( "Position: %d\n", *Position );
   else
      log( "\n" );

   return rc;
}

APIRET APIENTRY RexxReorderMacro( PSZ FuncName,
                                  ULONG Position )
{
   APIRET rc = RXMACRO_NOT_FOUND;

   if (FuncName)
   {
      log( "%s: FuncName: \"%s\" Position: %ld",
           __FUNCTION__,
           FuncName,
           Position );
   }
   else
   {
      log( "%s: FuncName: <NULL> Position: %ld",
           __FUNCTION__,
           Position );
   }

   if (ORexxReorderMacro)
   {
      rc = (*ORexxReorderMacro)(
         (PSZ)       FuncName,
         (ULONG)     Position );
   }

   log( "<=> Result: %d\n", rc );

   return rc;
}

APIRET APIENTRY RexxClearMacroSpace( VOID )
{
   APIRET rc = RXMACRO_NOT_FOUND;

   log( "%s:", __FUNCTION__ );

   if (ORexxClearMacroSpace)
   {
      rc = (*ORexxClearMacroSpace)( );
   }

   log( "<=> Result: %x\n", rc );

   return rc;
}

#if 1

APIRET unimplemented(char *func)
{
  log("%s is not yet implemented!\n", func);
  return 0;
}

APIRET APIENTRY RexxLoadSubcom( PSZ envp,
                                PSZ dllp )
{
   APIRET rc = RXSUBCOM_NOTREG;

   log( "%s:", __FUNCTION__ );

   unimplemented(__FUNCTION__);
   
   log( "<=> Result: %x\n", rc );

   return rc;
}

// ???
APIRET APIENTRY RexxCallFunction (
        PSZ fn,
        ULONG argc,
        PRXSTRING argv,
        PUSHORT ret,
        PRXSTRING stor,
        PSZ data)
{
   APIRET rc = RXFUNC_NOTREG;

   log( "%s:", __FUNCTION__ );

   unimplemented(__FUNCTION__);
   
   log( "<=> Result: %x\n", rc );

   return rc;
}

// ???
APIRET APIENTRY RexxCallSubcom(
         PSZ env,
         PSZ dll,
         PRXSTRING cmd,
         PUSHORT flag,
         PUSHORT ret,
         PRXSTRING stor )
{
   APIRET rc = RXSUBCOM_NOTREG;

   log( "%s:", __FUNCTION__ );

   unimplemented(__FUNCTION__);
   
   log( "<=> Result: %x\n", rc );

   return rc;
}

// ???
APIRET APIENTRY RexxCallExit(
         ULONG arg1,
         ULONG arg2,
         ULONG arg3,
         ULONG arg4,
         ULONG arg5 )
{
   APIRET rc = RXEXIT_NOTREG;

   log( "%s:", __FUNCTION__ );

   unimplemented(__FUNCTION__);
   
   log( "<=> Result: %x\n", rc );

   return rc;
}

// ???
APIRET APIENTRY RexxExecuteMacroFunction(
         ULONG arg1,
         ULONG arg2 )
{
   APIRET rc = RXMACRO_NOT_FOUND;

   log( "%s:", __FUNCTION__ );

   unimplemented(__FUNCTION__);
   
   log( "<=> Result: %x\n", rc );

   return rc;
}

#endif
