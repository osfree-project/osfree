#ifndef lint
static char *RCSid = "$Id: client.c,v 1.51 2004/04/24 09:32:58 florian Exp $";
#endif

/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1993-1994  Anders Christensen <anders@pvv.unit.no>
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
/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */

#include "regina_c.h"

#if defined(WIN32) && defined(__IBMC__)
# include <windows.h>
# pragma warning(default: 4115 4201 4214)
#else
# if defined(WIN32) && (defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__MINGW32__))
#  undef APIENTRY
#  include <windows.h>
# else
#  if defined(__EMX__) && defined(OS2)
#   define DONT_TYPEDEF_PFN
#   undef APIENTRY
#   include <os2emx.h>
#  else
#   if defined(__WATCOMC__) && defined(OS2)
#    undef APIENTRY
#    define DONT_TYPEDEF_PFN
#    include <os2.h>
#   else
#    if defined(_MSC_VER) && !defined(__WINS__)
#     undef APIENTRY
#     if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#      pragma warning(disable: 4115 4201 4214 4514)
#     endif
#     include <windows.h>
#     if _MSC_VER >= 1100
#      pragma warning(default: 4115 4201 4214)
#     endif
#    else
#     define APIENTRY
#    endif
#   endif
#  endif
# endif
#endif

#ifndef RXLIB  /* need __regina_faked_main which is known with RXLIB only */
# define RXLIB
#endif
#include "rexx.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "rxiface.h"
#include <stdio.h>
#include <string.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

static int ReMapHook( const tsd_t *TSD, int NetHook ) ;

typedef struct { /* cli_tsd: static variables of this module (thread-safe) */
   char    count_params_buf[32]; /* handle_no_of_params() */

   /* Buffering of external parameters allows a delayed deletion in case of
    * errors.
    */
   char *  ExternalName ;
   int     StringsCount ;
   int *   Lengths ; /* StringsCount + 1 elements */
   char ** Strings ; /* StringsCount + 1 elements */
} cli_tsd_t; /* thread-specific but only needed by this module. see
              * init_client
              */

/* init_client initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_client( tsd_t *TSD )
{
   cli_tsd_t *ct;

   if (TSD->cli_tsd != NULL)
      return(1);

   if ((ct = TSD->cli_tsd = MallocTSD(sizeof(cli_tsd_t))) == NULL)
      return(0);
   memset(ct,0,sizeof(cli_tsd_t));
   ct->StringsCount = -1;
   return(1);
}


/* wrapstring makes a streng from external (application) data. If you get
 * an exception you may have a wild pointer in your application.
 * Returns NULL if string==NULL or length<0.
 */
static streng *wrapstring( const tsd_t *TSD, const char *string, int length )
{
   streng *result=NULL ;

   if (length>=0 && string)
   {
      result = Str_makeTSD( length ) ;
      memcpy( result->value, string, length ) ;
      result->len = length ;
   }
   else
      result = NULL ;

   return result ;
}


static int map_type( int in )
{
   if (in==RX_TYPE_COMMAND)
      return INVO_COMMAND ;
   else if (in==RX_TYPE_FUNCTION)
      return INVO_FUNCTION ;
   else
   {
      assert( in == RX_TYPE_SUBROUTINE ) ;
      return INVO_SUBROUTINE ;
   }
}


static paramboxptr parametrize( const tsd_t *TSD, int ArgCount, const int *ParLengths, const char **ParStrings )
{
   paramboxptr parms=NULL, root=NULL ;
   int count=0, i=0 ;

   count = ArgCount ;
   for (i=0; count--; i++ )
   {
      if (parms)
      {
         parms->next = MallocTSD( sizeof(parambox)) ;
         parms = parms->next ;
      }
      else
         parms = root = MallocTSD( sizeof(parambox )) ;

      parms->value = wrapstring( TSD, ParStrings[i], ParLengths[i] ) ;
   }

   if (parms)
      parms->next = NULL ;

   return root ;
}

/*
 * IfcPrepareReturnString will copy the content of src to the buffer described
 * by the pair (*targetlen, *targetbuf). The target is not checked, but a
 * value of RX_NO_STRING for *targetlen will be accepted.
 *
 * If src is a NULL pointer the targetlen will become 0 and targetbuf will
 * become NULL.
 * Otherwise the length of the target is checked if it can hold the entire
 * source string PLUS one string termination character. The terminator won't
 * be honoured in *targetlen.
 * A new buffer will be allocated using IfcAllocateString on insufficient size,
 * but the old one is not freed.
 *
 * This function is intended to be the opposite of wrapstring.
 */
static void IfcPrepareReturnString( const streng *src, int *targetlen,
                                    char **targetbuf )
{
   int len;

   /*
    * Check for an invalid source first.
    */
   if ( ( src == NULL ) || ( Str_val( src ) == NULL ) )
   {
      *targetlen = 0;
      *targetbuf = NULL;
      return;
   }
   len = Str_len( src );
   if ( *targetlen < len + 1 )
   {
      if ( ( *targetbuf = IfcAllocateMemory( len + 1 ) ) == NULL )
      {
         /*
          * Better idea?
          */
         *targetlen = 0;
         return;
      }
   }
   memcpy( *targetbuf, Str_val( src ), len );
   (*targetbuf)[len] = '\0';
   *targetlen = len;
}

/*
 * ScriptSetup does the setup step of IfcExecScript.
 * This function is part of the technique to bypass problems with
 * setjmp/longjmp.
 */
static void ScriptSetup( tsd_t *TSD,
                         void **instore_buf, unsigned long *instore_length,
                         streng **command, const char *Name, int NameLen,
                         paramboxptr *params, int ArgCount,
                              const int *ParLengths, const char **ParStrings,
                         int *ctype, int CallType,
                         int restricted,
                         int *hooks, int ExitFlags,
                         streng **environment, int EnvLen, const char *EnvName )
{
   int i;

   *instore_buf = NULL;
   *instore_length = 0;
   *command = wrapstring( TSD, Name, NameLen );
   TSD->systeminfo->input_file = *command;
   assert( *command );
   *params = parametrize( TSD, ArgCount, ParLengths, ParStrings );

   *ctype = map_type( CallType );

   TSD->restricted = restricted;

   *hooks = 0;
   for( i = 0; i < 30; i++ )
   {
      if ( ExitFlags & ( 1 << i ) )
         *hooks |= 1 << ReMapHook( TSD, i );
   }

   *environment = wrapstring( TSD, EnvName, EnvLen );
   /*
    * FIXME: "DEFAULT" is a bad idea! We have to use the extension of the file.
    */
   if ( *environment == NULL )
      *environment = Str_creTSD( "DEFAULT" );

   if ( !envir_exists( TSD, *environment ) )
      add_envir( TSD, Str_dupTSD( *environment ), ENVIR_PIPE, 0 );
}

/*
 * ScriptCleanup does the cleanup step of IfcExecScript.
 * 1) script_exit becomes the new old value for a jumped exit.
 * 2) environment is deregistered and freed.
 * 3) A fresh Ifc-allocated copy of result is put into RetLen/RetString,
 *    and result is freed. *RetString is either used or replaced.
 * 4) InterpreterStatus is reset to the given value.
 * This function is part of the technique to bypass problems with
 * setjmp/longjmp.
 */
static void ScriptCleanup(tsd_t *TSD, jmp_buf *script_exit, streng *environment,
                          streng *result, int *RetLen, char **RetString,
                          unsigned *InterpreterStatus)
{
   TSD->systeminfo->script_exit = script_exit;
   del_envir( TSD, environment );
   Free_stringTSD( environment );
   RestoreInterpreterStatus( TSD, InterpreterStatus );

   IfcPrepareReturnString( result, RetLen, RetString );

   if ( result )
      Free_stringTSD( result );
}

int IfcExecScript( tsd_t * volatile TSD, int NameLen, const char *Name,
                   int ArgCount, const int *ParLengths, const char **ParStrings,
                   int CallType, int ExitFlags, int EnvLen, const char *EnvName,
                   int SourceCode, int restricted,
                   const char *SourceString, unsigned long SourceStringLen,
                   const void *TinnedTree, unsigned long TinnedTreeLen,
                   int *RetLen, char **RetString, void **instore_buf,
                   unsigned long *instore_length )
{
   streng * volatile result=NULL;
   tsd_t * volatile saved_TSD;
   jmp_buf * volatile old_exit_addr;
   int volatile RetCode=0;
   streng * volatile environment=NULL;
   volatile int jumped=0;
   jmp_buf exit_addr;
   streng * command;
   paramboxptr params;
   int ctype,hooks,type;
   unsigned InterpreterStatus[IPRT_BUFSIZE];

   /*
    * NOTE:
    * setjmp and longjmp will clobber all variables not explicitely marked
    * as volatile. The reason is the unexpected jump from anywhere to the
    * code following the setjmp() call. Every assumption of the compiler
    * about valid register variables are wrong. We have to remove all
    * register variables set by the optimizer. This can be done either by
    * using volatile, or by not using variables before the setjmp() INCLUDING
    * initialization or by using non-inline functions for manipulating
    * values.
    * We use a combination here.
    * If you ever need to change ANY line before the setjmp() code,
    * double-check the compiler's chance to cache some variables in registers.
    * Florian
    */
   saved_TSD = TSD;
   SaveInterpreterStatus( TSD, InterpreterStatus );

   /* It may be that TSD->systeminfo->script_exit is not set. This may lead to
    * an exit() call, e.g. at Rexx "EXIT". We set TSD->systeminfo->script_exit
    * if it is not set. If a problem occurs we will be informed by a longjmp
    * and we will not completely been killed by an exit(). Remember: This
    * function is typically called if a program uses "us" as a subroutine.
    * Exiting is very harmful in this case.
    * Note: The memory allocated within called subsoutines will NOT be
    * freed. We must change many things to do this. Maybe, it's a good
    * idea to traverse all called subroutine trees and free the leaves.
    * FGC
    */
   if ( ( old_exit_addr = TSD->systeminfo->script_exit ) == NULL )
   {
      TSD->systeminfo->script_exit = &exit_addr;
      assert( !TSD->in_protected );
      if ( setjmp( *TSD->systeminfo->script_exit ) )
      {
         TSD = saved_TSD;
         result = TSD->systeminfo->result;
         TSD->systeminfo->result = NULL;
         if ( !RetCode )
         {
            if ( result )
               RetCode = atoi( result->value );
            else
               RetCode = -1;
         }
         result = NULL; /* result was a static buffer provided by exiterror */
         jumped++;
      }
   }
   if ( !jumped )
   {
      ScriptSetup( TSD,
                   instore_buf, instore_length,
                   &command, Name, NameLen,
                   &params, ArgCount, ParLengths, ParStrings,
                   &ctype, CallType,
                   restricted,
                   &hooks, ExitFlags,
                   (streng **) &environment, EnvLen, EnvName );

      type = SourceCode ;
      if ( type == RX_TYPE_EXTERNAL )
      {
         result = execute_external( TSD, command, params, environment,
                                    (int *) &RetCode, hooks, ctype );
      }
      else if ( type == RX_TYPE_INSTORE )
      {
         result = do_instore( TSD, command, params, environment,
                              (int *) &RetCode, hooks,
                              TinnedTree, TinnedTreeLen,
                              SourceString, SourceStringLen, NULL, ctype );
      }
      else if ( type == RX_TYPE_MACRO )
         result = nullstringptr();
      else if ( type == RX_TYPE_SOURCE )
      {
         streng *SrcStr = wrapstring( TSD, SourceString, SourceStringLen );
         internal_parser_type ipt;

         ipt = enter_macro( TSD, SrcStr, instore_buf, instore_length ) ;
         if ( ( CallType == RX_TYPE_COMMAND )
         &&   ArgCount
         &&   ( *ParStrings != NULL )
         &&   ( *ParLengths == 3 )
         &&   ( memcmp( "//T", *ParStrings, 3 ) == 0 ) )
            ; /* request for tokenisation only, don't execute */
         else
         {
            ipt.kill = SrcStr;
            result = do_instore( TSD, command, params, environment,
                                 (int *) &RetCode, hooks, NULL,0, NULL,0, &ipt,
                                 ctype);
            /* do_instore already has deleted the internal structure */
         }
      }
      else
      {
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" );
         return 0;
      }
   }

   ScriptCleanup( TSD, old_exit_addr, environment, result, RetLen, RetString,
                  InterpreterStatus );

   return RetCode;
}

int IfcExecCallBack( tsd_t * volatile TSD, int NameLen, const char *Name,
                     int ArgCount, const int *ParLengths, const char **ParStrings,
                     int *RetLen, char **RetString )
{
   nodeptr node;
   paramboxptr args;
   streng *result;
   streng *name;

   /*
    * If we can't find the label, no point in continuing.
    */
   name = Str_ncreTSD( Name, NameLen );
   if ( ( node = getlabel( TSD, name ) ) == NULL )
   {
      FreeTSD( name );
      return RX_CODE_NOSUCH;
   }
   FreeTSD( name );
   if ( node->u.trace_only )
   {
      FreeTSD( name );
      return RX_CODE_NOSUCH;
   }

   /*
    * Fixes bug 772199
    */
   args = initargs( TSD, ArgCount, ParLengths, ParStrings );
   result = CallInternalFunction( TSD, node, TSD->currentnode, args );

   IfcPrepareReturnString( result, RetLen, RetString );

   if ( result )
      Free_stringTSD( result );

   return 0;
}


static int handle_source( const tsd_t *TSD, int *Length, char **String,
                          int *allocated )
{
   int infile=0, total=0, invoked=0 ;
   const char *stype=NULL ;
   char *ctmp=NULL ;
   int sleng=0 ;

   stype = system_type() ;
   sleng = strlen( stype ) ;
   infile = TSD->systeminfo->input_file->len ;
   invoked = strlen(invo_strings[TSD->systeminfo->invoked]) ;
   total = sleng + 1 + invoked + 1 + infile ;

   ctmp = *String = MallocTSD( (*Length=total)+2 ) ;
   sprintf( ctmp, "%s %s ", stype, invo_strings[TSD->systeminfo->invoked]) ;
   strncat( ctmp, TSD->systeminfo->input_file->value, infile ) ;

   *allocated = 1;
   return RX_CODE_OK ;
}

static int handle_param( tsd_t *TSD, int *Length, char **String )
{
   paramboxptr ptr=NULL ;
   streng *value=NULL ;
   int number=0 ;

   ptr = TSD->systeminfo->currlevel0->args ;
   value = wrapstring( TSD, *String, *Length ) ;
   number = atopos( TSD, value, "internal", 1 ) ;
   Free_stringTSD( value ) ;
   value = get_parameter( ptr, number ) ;

   if (value)
   {
      *(Length+1) = value->len ;
      *(String+1) = value->value ;
   }
   else
      *(Length+1) = RX_NO_STRING ;

   return RX_CODE_OK ;
}

static int handle_no_of_params( const tsd_t *TSD, int *Length, char **String )
{
   paramboxptr ptr=NULL ;
   int count=0 ;
   cli_tsd_t *ct;

   ct = TSD->cli_tsd;
   ptr = TSD->systeminfo->currlevel0->args ;
   count = count_params( ptr, PARAM_TYPE_HARD ) ;

   sprintf( ct->count_params_buf, "%d", count ) ;
   *String = ct->count_params_buf;
   *Length = strlen(*String) ;

   return RX_CODE_OK ;
}



static int get_next_var( tsd_t *TSD, int *Lengths, char **Strings,
                         int *allocated )
{
   variableptr value=NULL, rval=NULL, rrval=NULL ;
   int l;

   *allocated = 0;
   if (TSD->var_indicator==0)
   {
      get_next_variable( TSD, 1 ) ;
      TSD->var_indicator = 1 ;
   }

   for (;;)
   {
      value = get_next_variable( TSD, 0 ) ;
      for (rval=value; rval && rval->realbox; rval=rval->realbox)
      {
        ;
      }
      if (rval && !(rval->flag & VFLAG_STR))
      {
         if (rval->flag & VFLAG_NUM)
         {
            expand_to_str( TSD, rval ) ;
            break ;
         }

         if (!rval->stem)
            continue ;

         for (rrval=rval->stem; rrval && rrval->realbox; rrval=rrval->realbox)
         {
           ;
         }
         if (rrval && !(rrval->flag & VFLAG_STR))
         {
            if (rval->flag & VFLAG_NUM)
            {
               expand_to_str( TSD, rval ) ;
               break ;
            }
            else
               continue ;
         }
      }
      break ;
   }

   if (rval)
   {
      if (rval->stem)
      {
         *allocated |= 1;
         l = Lengths[0] = rval->stem->name->len + rval->name->len ;
         Strings[0] = MallocTSD( (l < 1) ? 1 : l ) ;
         memcpy(Strings[0], rval->stem->name->value, rval->stem->name->len);
         memcpy(Strings[0]+rval->stem->name->len,
                                      rval->name->value, rval->name->len ) ;
      }
      else
      {
         Lengths[0] = rval->name->len ;
         Strings[0] = rval->name->value ;
      }

      if (rval->value)
      {
         Lengths[1] = rval->value->len ;
         Strings[1] = rval->value->value ;
      }
      else
      {
         *allocated |= 2;
         assert( rval->stem && rrval->value ) ;
         l = Lengths[1] = rval->stem->name->len + rval->name->len ;
         Strings[1] = MallocTSD( (l < 1) ? 1 : l ) ;
         memcpy( Strings[1], rval->stem->name->value, value->stem->name->len );
         memcpy( Strings[1]+value->stem->name->len,
                                        rval->name->value, rval->name->len ) ;
      }

      return 2 ;
   }
   else
   {
      /*
       * Once we have reached the end of all variables, reset var_indicator
       * so next time we can start from the beginning again.
       */
      TSD->var_indicator = 0 ;
      return 0 ;
   }
}


static int MapHook( const tsd_t *TSD, int RexxHook )
{
   switch ( RexxHook )
   {
      case HOOK_STDOUT: return RX_EXIT_STDOUT ;
      case HOOK_STDERR: return RX_EXIT_STDERR ;
      case HOOK_TRCIN:  return RX_EXIT_TRCIN ;
      case HOOK_PULL:   return RX_EXIT_PULL ;

      case HOOK_INIT:   return RX_EXIT_INIT ;
      case HOOK_TERMIN: return RX_EXIT_TERMIN ;
      case HOOK_SUBCOM: return RX_EXIT_SUBCOM ;

      case HOOK_GETENV: return RX_EXIT_GETENV ;
      case HOOK_SETENV: return RX_EXIT_SETENV ;

      case HOOK_GETCWD: return RX_EXIT_GETCWD ;
      case HOOK_SETCWD: return RX_EXIT_SETCWD ;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" );
   }

   return 0 ;
}


static int ReMapHook( const tsd_t *TSD, int NetHook )
{
   switch ( NetHook )
   {
      case RX_EXIT_STDOUT: return HOOK_STDOUT ;
      case RX_EXIT_STDERR: return HOOK_STDERR ;
      case RX_EXIT_TRCIN:  return HOOK_TRCIN ;
      case RX_EXIT_PULL:   return HOOK_PULL ;

      case RX_EXIT_TERMIN: return HOOK_TERMIN ;
      case RX_EXIT_INIT:   return HOOK_INIT ;
      case RX_EXIT_SUBCOM: return HOOK_SUBCOM ;
      case RX_EXIT_FUNC:   return HOOK_FUNC ;

      case RX_EXIT_GETENV: return HOOK_GETENV ;
      case RX_EXIT_SETENV: return HOOK_SETENV ;

      case RX_EXIT_GETCWD: return HOOK_GETCWD ;
      case RX_EXIT_SETCWD: return HOOK_SETCWD ;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" );
   }

   return 0 ;
}



int hookup( tsd_t *TSD, int hook )
{
   int rcode=0, code=0;

   code = MapHook ( TSD, hook );
   assert (code == RX_EXIT_INIT || code == RX_EXIT_TERMIN );
   rcode = IfcDoExit( TSD, code, 0, NULL, 0, NULL, NULL, NULL ) ;
   if (rcode==RX_HOOK_ERROR)
       exiterror( ERR_SYSTEM_FAILURE, 0 )  ;
   else if (rcode==RX_HOOK_GO_ON)
      rcode = HOOK_GO_ON ;
   else if (rcode==RX_HOOK_NOPE)
      rcode = HOOK_NOPE ;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
   return rcode ;
}

int hookup_output( tsd_t *TSD, int hook, const streng *outdata )
{
   int rcode=0, code=0 ;
   char *str;
   int len = 0;

   code = MapHook( TSD, hook );
/*   assert (code == RX_EXIT_STDOUT || code == RX_EXIT_STDERR ); */
   if (outdata)
   {
      str = str_of( TSD, outdata ) ;
      len = outdata->len ;
   }
   else
   {
      str = MallocTSD( 1 ) ;
      str[0] = '\0' ;
      len = 0 ;
   }

   rcode = IfcDoExit( TSD, code, len, str, 0, NULL, NULL, NULL) ;
   FreeTSD( str ) ;
   if (rcode==RX_HOOK_ERROR)
       exiterror( ERR_SYSTEM_FAILURE, 0 )  ;
   else if (rcode==RX_HOOK_GO_ON)
      rcode = HOOK_GO_ON ;
   else if (rcode==RX_HOOK_NOPE)
      rcode = HOOK_NOPE ;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   return rcode ;
}

int hookup_output2( tsd_t *TSD, int hook, const streng *outdata1, const streng *outdata2 )
{
   int rcode=0, code=0 ;
   char *str1, *str2;
   int len1 = 0, len2 = 0;

   code = MapHook( TSD, hook );
/*   assert (code == RX_EXIT_SETENV ); */
   if (outdata1)
   {
      str1 = str_of( TSD, outdata1 ) ;
      len1 = outdata1->len ;
   }
   else
   {
      str1 = MallocTSD( 1 ) ;
      str1[0] = '\0' ;
      len1 = 0 ;
   }
   if (outdata2)
   {
      str2 = str_of( TSD, outdata2 ) ;
      len2 = outdata2->len ;
   }
   else
   {
      str2 = MallocTSD( 1 ) ;
      str2[0] = '\0' ;
      len2 = 0 ;
   }

   rcode = IfcDoExit( TSD, code, len1, str1, len2, str2, NULL, NULL) ;
   FreeTSD( str1 ) ;
   FreeTSD( str2 ) ;
   if (rcode==RX_HOOK_ERROR)
       exiterror( ERR_SYSTEM_FAILURE, 0 )  ;
   else if (rcode==RX_HOOK_GO_ON)
      rcode = HOOK_GO_ON ;
   else if (rcode==RX_HOOK_NOPE)
      rcode = HOOK_NOPE ;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   return rcode ;
}

int hookup_input( tsd_t *TSD, int hook, streng **indata )
{
   int rcode=0, code=0 ;
/* The following value allows called programs to call "free" to the return
 * parameters without destroying our stack.
 */
#define ILLEGAL_USE_SIZE (8 * sizeof(void *))
   char RetBuf[ILLEGAL_USE_SIZE + 256] ;
   int retlen=256 ;
   char *retstr=RetBuf;

   code = MapHook( TSD, hook );
/*   assert (code == RX_EXIT_PULL || code == RX_EXIT_TRCIN ); */

   strcpy(RetBuf,"");
   /* FIXME, FGC: The documentation says that the input of a PULL or TRCIN is
    * undefined. Why do we provide a buffer?
    */
   rcode = IfcDoExit( TSD, code, 0, NULL, 0, NULL, &retlen, &retstr ) ;
   if (rcode==RX_HOOK_ERROR)
      exiterror( ERR_SYSTEM_FAILURE, 0 )  ;
   else if (rcode==RX_HOOK_GO_ON)
      rcode = HOOK_GO_ON ;
   else if (rcode==RX_HOOK_NOPE)
      rcode = HOOK_NOPE ;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   *indata = wrapstring( TSD, retstr, retlen ) ;
   FreeTSD( retstr ) ; /* retstr was always newly allocated */
#undef ILLEGAL_USE_SIZE
   return rcode ;
}

int hookup_input_output( tsd_t *TSD, int hook, const streng *outdata, streng **indata )
{
   char *str;
   int len = 0;
   int rcode=0, code=0 ;
/* The following value allows called programs to call "free" to the return
 * parameters without destroying our stack.
 */
#define ILLEGAL_USE_SIZE (8 * sizeof(void *))
   char RetBuf[ILLEGAL_USE_SIZE + 256] ;
   int retlen=256 ;
   char *retstr=RetBuf;

   code = MapHook( TSD, hook );
/*   assert (code == RX_EXIT_GETENV );*/

   if (outdata)
   {
      str = str_of( TSD, outdata ) ;
      len = outdata->len ;
   }
   else
   {
      str = MallocTSD( 1 ) ;
      str[0] = '\0' ;
      len = 0 ;
   }

   strcpy(RetBuf,"");
   /* FIXME, FGC: The documentation says that the input of a PULL or TRCIN is
    * undefined. Why do we provide a buffer?
    */
   rcode = IfcDoExit( TSD, code, len, str, 0, NULL, &retlen, &retstr ) ;
   FreeTSD( str ) ;
   if (rcode==RX_HOOK_ERROR)
      exiterror( ERR_SYSTEM_FAILURE, 0 )  ;
   else if (rcode==RX_HOOK_GO_ON)
      rcode = HOOK_GO_ON ;
   else if (rcode==RX_HOOK_NOPE)
      rcode = HOOK_NOPE ;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   *indata = wrapstring( TSD, retstr, retlen ) ;
   FreeTSD( retstr ) ; /* retstr was always newly allocated */
#undef ILLEGAL_USE_SIZE
   return rcode ;
}



streng *SubCom( tsd_t *TSD, const streng *command, const streng *envir, int *rc )
{
   int tmplen ;
   char *tmpptr ;
   streng *retval ;

   *rc = IfcSubCmd( TSD,
                    envir ? envir->len : RX_NO_STRING,
                    envir ? envir->value : NULL,
                    command->len, command->value, &tmplen, &tmpptr ) ;

   retval = wrapstring( TSD, tmpptr, tmplen ) ;
   FreeTSD( tmpptr ) ;
   return retval ;
}


/****************************************************************************
 *
 *  JH 13/12/1999 Trying to satisfy both BUG031 and BUG022.
 *
 *  BUG022  To make Direct setting of stems Direct and not Symbolic.
 *   - Added new parameter, "Code".  Assumed values are RX_GETSVAR, and
 *     RX_GETVAR.  They denote that the actiion is symbolic, or direct
 *     (respectively).
 *   - if direct call getdirvalue()
 *
 *  BUG031
 *   - if default value is being returned, uppercase the whole name for
 *     symbolics, and only the stem for directs.  This should be a moot
 *     issue, as there is code in variable.c to return default values.
 *
 ****************************************************************************/
static int GetVariable( tsd_t *TSD, int Code, int *Lengths, char *Strings[],
                        int *allocated)
{
   const streng *value;
   streng *varbl;
   char *retval;
   int i,len,state;

   varbl = wrapstring( TSD, Strings[0], Lengths[0] );

   if ( !varbl )
   {
      Lengths[1] = RX_NO_STRING;
      return RX_CODE_INVNAME;
   }

   if ( !valid_var_symbol( varbl ) )
   {
      Free_stringTSD( varbl );
      Lengths[1] = RX_NO_STRING;
      return RX_CODE_INVNAME;
   }

   state = variables_per_SAA( TSD );
   if ( Code == RX_GETSVAR )
      value = getvalue( TSD, varbl, -1 );
   else
      value = getdirvalue( TSD, varbl );
   restore_variable_state( TSD, state );

   /*
    * getvalue returns varbl or a temporary value on NOVALUE, don't free varbl
    * until the final use of value and varbl.
    */
   if ( var_was_found( TSD ) )
   {
      Lengths[1] = value->len;
      Strings[1] = (char *) value->value;
      *allocated = 0;
      Free_stringTSD( varbl );
      return RX_CODE_OK;
   }

   Lengths[1] = len = value->len;
   Strings[1] = retval = MallocTSD( len );
   memcpy( retval, value->value, len );
   *allocated = 2;
   Free_stringTSD( varbl );

   if ( Code == RX_GETSVAR )
   {
      mem_upper( retval, len );
   }
   else
   {
      /* JH 13-12-99 If this is a direct variable, only uppercase the
       * stem name.
       */
      /*
       * Bypass the dot test for the first character to allow reserved
       * variables.
       */
      if ( len )
         retval[0] = (unsigned char) rx_toupper( retval[0] );
      for( i = 1; ( i < len ) && ( '.' != retval[i] ); i++ )
         retval[i] = (unsigned char) rx_toupper( retval[i] );
   }
   return RX_CODE_NOVALUE;
}

/****************************************************************************
 *
 *  JH 13/12/1999 Trying to satisfy both BUG031 and BUG022.
 *
 *  BUG022  To make Direct setting of stems Direct and not Symbolic.
 *   - Added new parameter, "Code".  Assumed values are RX_SETSVAR, and
 *     RX_SETVAR.  They denote that the actiion is symbolic, or direct
 *     (respectively).
 *  Until this point setting and dropping variables have been treated as
 *  the same.  If there is a value, then it is a set, otherwise drop it.
 *   - if direct call setdirvalue()
 *   - if direct call drop_dirvar()
 *  BUG031
 *   - uppercase the whole name for symbolics, and only the stem for directs.
 *
 ****************************************************************************/
static int SetVariable( tsd_t *TSD, int Code, int *Lengths, char *Strings[] )
{
   streng *varbl,*varname,*value;
   int rcode,i,state;

   varbl = wrapstring( TSD, Strings[0], Lengths[0] );

   if ( varbl == NULL )
   {
      /*
       * If the variable name is empty, return an error
       */
      return RX_CODE_INVNAME;
   }

   if ( Code == RX_SETSVAR )
      varname = Str_upper( Str_dupTSD( varbl ) );
   else
   {
      varname = Str_dupTSD( varbl );
      /*
       * Bypass the dot test for the first character to allow reserved
       * variables.
       */
      if ( varname->len )
         varname->value[0] = (unsigned char) rx_toupper( varname->value[0] );
      for( i = 1; ( i < varname->len ) && ( '.' != varname->value[i] ); i++ )
      {
         varname->value[i] = (unsigned char) rx_toupper( varname->value[i] );
      }
   }
   Free_stringTSD( varbl ) ;
   varbl = NULL; /* For debugging purpose only */

   if ( !valid_var_symbol( varname ) )
   {
      Free_stringTSD( varname );
      return RX_CODE_INVNAME;
   }

   value = wrapstring( TSD, Strings[1], Lengths[1] );

   state = variables_per_SAA( TSD );
   rcode = RX_CODE_OK; /* default value */
   if ( value )
   {
      if ( Code == RX_SETSVAR )
         setvalue( TSD, varname, value, -1 );
      else
         setdirvalue( TSD, varname, value );
   }
   else
   {
      if ( Code == RX_SETSVAR )
         drop_var( TSD, varname );
      else
         drop_dirvar( TSD, varname );
   }
   restore_variable_state( TSD, state );

   if ( !var_was_found( TSD ) )
      rcode = RX_CODE_NOVALUE;

   Free_stringTSD( varname );
   return rcode;
}

static int handle_version( int *Length, char **String )
{
   *Length = strlen(PARSE_VERSION_STRING) ;
   *String = PARSE_VERSION_STRING ;
   return RX_CODE_OK ;
}


static int handle_queue( const tsd_t *TSD, int *Length, char **String )
{
   fill_queue_name( TSD, Length, String );
   return RX_CODE_OK ;
}


/****************************************************************************
 *
 *  JH 13/12/1999
 *
 *  BUG022  To make Direct setting of stems Direct and not Symbolic.
 *   - Added checks for the direct variable functions RX_GETVAR and RX_SETVAR.
 *
 *  *allocated will be set, if the returned value(s) is/are allocated and
 *  should be freed after the use.
 *
 ****************************************************************************/
int IfcVarPool( tsd_t *TSD, int Code, int *Lengths, char *Strings[],
                int *allocated )
{
   int rc=0 ;

   *allocated = 0;
   if (Code==RX_GETSVAR ||Code==RX_GETVAR )
      rc = GetVariable( TSD, Code, Lengths, Strings, allocated ) ;
   else if (Code==RX_SETSVAR ||Code==RX_SETVAR )
      rc = SetVariable( TSD, Code, Lengths, Strings ) ;
   else if (Code==RX_NEXTVAR)
      rc = get_next_var( TSD, Lengths, Strings, allocated ) ;
   else if (Code==RX_CODE_VERSION)
      rc = handle_version( Lengths, Strings ) ;
   else if (Code==RX_CODE_SOURCE)
      rc = handle_source( TSD, Lengths, Strings, allocated ) ;
   else if (Code==RX_CODE_QUEUE)
      rc = handle_queue( TSD, Lengths, Strings );
   else if (Code==RX_CODE_PARAMS)
      rc = handle_no_of_params( TSD, Lengths, Strings ) ;
   else if (Code==RX_CODE_PARAM)
      rc = handle_param( TSD, Lengths, Strings ) ;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   return rc ;
}

/* Removes ct->Strings and ct->Lengths AND ct->ExternalName. */
static void RemoveParams(const tsd_t *TSD)
{
   int i;
   cli_tsd_t *ct;

   ct = TSD->cli_tsd;

   if ( ct->Strings && ct->Lengths )
   {
      for (i = 0;i < ct->StringsCount;i++) /* The last one is always NULL */
      {
         if ((ct->Lengths[i] != RX_NO_STRING) &&
             (ct->Strings[i] != NULL))
            FreeTSD( ct->Strings[i] );
      }
   }

   if ( ct->Lengths )
      FreeTSD( ct->Lengths ) ;
   if ( ct->Strings )
      FreeTSD( ct->Strings ) ;
   if ( ct->ExternalName )
      FreeTSD( ct->ExternalName ) ;

   ct->StringsCount = -1;
   ct->Strings = NULL;
   ct->Lengths = NULL;
   ct->ExternalName = NULL;
}

/* Creates (possibly after a removal of old values) ct->Strings and ct->Lengths
 * from the given parms. One hidden NULL string is appended.
 */
static void MakeParams(const tsd_t *TSD, cparamboxptr parms)
{
   int i;
   cparamboxptr p=NULL ;
   cli_tsd_t *ct;

   ct = TSD->cli_tsd;

   /* Cleanup the old parameters before we set StringsCount */
   RemoveParams(TSD);

   ct->StringsCount = 0; /* This is the default in case of unused parameters */
   /* Detect the index of the last valid parameter */
   for (i=0,p=parms; p; p=p->next,i++)
   {
      if (p->value)
         ct->StringsCount=i+1 ;
   }

   /* add one NULL string at the end */
   ct->Lengths = MallocTSD( sizeof(int) * (ct->StringsCount+1) ) ;
   ct->Strings = MallocTSD( sizeof(char*) * (ct->StringsCount+1) ) ;

   for (i=0,p=parms; i < ct->StringsCount; p=p->next,i++)
   {
      if (p->value)
      {
         ct->Lengths[i] = Str_len( p->value ) ;
         ct->Strings[i] = str_of( TSD, p->value );
      }
      else
      {
         ct->Lengths[i] = RX_NO_STRING ;
         ct->Strings[i] = NULL ;
      }
   }

   /* Provide a hidden NULL string at the end */
   ct->Lengths[ct->StringsCount] = RX_NO_STRING ;
   ct->Strings[ct->StringsCount] = NULL ;
}

/* do_an_external calls IfcExecFunc with the appropriate parameters. Basically
 * it wraps the parameters. Either ExeName or box must be NULL.
 */
static streng *do_an_external( tsd_t *TSD,
                               const streng *ExeName,
                               const struct entry_point *box,
                               cparamboxptr parms,
                               char exitonly,
                               char called )
{
   int RetLength=0 ;
   char *RetString=NULL ;
   streng *retval ;
   streng *queue;
   int rc ;
   int RC ;
   PFN Func;
   cli_tsd_t *ct;
   void *gci_info;
   volatile char *tmpExternalName; /* used to save ct->ExternalName */
                                   /* when erroring                 */

   ct = TSD->cli_tsd;

   MakeParams( TSD, parms ) ;
   if (ExeName)
   {
      ct->ExternalName = str_of( TSD, ExeName );
      tmpExternalName = tmpstr_of( TSD, ExeName );
      Func = NULL;
      gci_info = NULL;
   }
   else
   {
      ct->ExternalName = str_of( TSD, box->name );
      tmpExternalName = tmpstr_of( TSD, box->name );
      Func = box->addr;
      gci_info = box->special.gci_info;
   }

   queue = get_queue( TSD );

   rc = IfcExecFunc( TSD, Func, ct->ExternalName,
                     ct->StringsCount, ct->Lengths, ct->Strings,
                     Str_len( queue ), Str_val( queue ),
                     &RetLength, &RetString, &RC, called, gci_info );

   Free_stringTSD( queue );
   RemoveParams( TSD ) ;

   if (RC)
   {
      switch( RC )
      {
         case ERR_ROUTINE_NOT_FOUND:
            exiterror( ERR_ROUTINE_NOT_FOUND, 1, tmpExternalName );
            break;
         default:
            exiterror( RC, 0) ;
            break;
      }
      retval = NULL ;
   }
   else if ( RetLength == RX_NO_STRING )
   {
      /*
       * ERR_NO_DATA_RETURNED is detected later in expr.c.
       */
      retval = NULL;
   }
   else
   {
      retval = Str_makeTSD( RetLength ) ;
      memcpy( retval->value, RetString, RetLength ) ;
      FreeTSD( RetString );
      retval->len = RetLength ;
   }
   return retval ;

}

streng *call_unknown_external( tsd_t *TSD, const streng *name, cparamboxptr parms, char called )
{
   assert(name);

   return( do_an_external( TSD, name, NULL, parms, 1, called ) ) ;
}

streng *call_known_external( tsd_t *TSD, const struct entry_point *vbox, cparamboxptr parms, char called )
{
   assert(vbox);

   return( do_an_external( TSD, NULL, vbox, parms, 0, called ) ) ;
}

int IfcCreateQueue( tsd_t *TSD, const char *qname, const int qlen, char *data, unsigned long *dupflag, unsigned long buflen )
{
   streng *queuename=NULL,*strdata;
   int rc;

   if ( qname )
   {
      queuename = Str_makeTSD( qlen ) ;
      memcpy( queuename->value, qname, qlen ) ;
      queuename->len = qlen;
   }
   rc = create_queue( TSD, queuename, &strdata );

   if ( rc == 0 || rc == 1 )
   {
      /*
       * Return the new queue name
       */
      memcpy( data, strdata->value, min( strdata->len, (int) buflen - 1 ) );
      data[min( strdata->len, (int) buflen - 1)] = '\0';
      /*
       * If the returned queue name is different to
       * the one we requested, set the dupflag
       */
      if ( queuename
      &&   rc == 1 )
         *dupflag = 1;
      else
         *dupflag = 0;
      FreeTSD( strdata );
      rc = 0;
   }
   if ( queuename) Free_stringTSD( queuename );
   return rc;
}

int IfcDeleteQueue( tsd_t *TSD, const char *qname, const int qlen )
{
   streng *queuename;
   int rc;

   queuename = Str_makeTSD( qlen ) ;
   memcpy( queuename->value, qname, qlen ) ;
   queuename->len = qlen;
   rc = delete_queue( TSD, queuename );
   Free_stringTSD( queuename );
   return rc;
}

int IfcQueryQueue( tsd_t *TSD, const char *qname, const int qlen, unsigned long *count )
{
   streng *queuename;
   int rc;

   queuename = Str_makeTSD( qlen ) ;
   memcpy( queuename->value, qname, qlen ) ;
   queuename->len = qlen;
   rc = lines_in_stack( TSD, queuename );
   if ( rc < 0 )
      rc = -rc;
   else
   {
      *count = rc;
      rc = 0;
   }
   Free_stringTSD( queuename );
   return rc;
}

int IfcAddQueue( tsd_t *TSD, const char *qname, const int qlen, const char *data, const int datalen, unsigned long addlifo )
{
   streng *queuename,*strdata;
   int rc;

   queuename = Str_makeTSD( qlen ) ;
   memcpy( queuename->value, qname, qlen ) ;
   queuename->len = qlen;

   strdata = Str_makeTSD( datalen ) ;
   memcpy( strdata->value, data, datalen ) ;
   strdata->len = datalen;

   if ( addlifo )
      rc = stack_lifo( TSD, strdata, queuename );
   else
      rc = stack_fifo( TSD, strdata, queuename );
   Free_stringTSD( queuename );
   return rc;
}

int IfcPullQueue( tsd_t *TSD, const char *qname, const int qlen, char **data, unsigned long *datalen, unsigned long waitforline )
{
   streng *queuename,*strdata;
   int rc, len;
   char *p;

   queuename = Str_makeTSD( qlen ) ;
   memcpy( queuename->value, qname, qlen ) ;
   queuename->len = qlen;

   strdata = popline( TSD, queuename, &rc, waitforline );

   if ( rc == 0 )
   {
      if ( strdata == NULL )
      {
         /*
          * Queue is empty
          */
         rc = 8; /* RXQUEUE_EMPTY */
      }
      else
      {
         len = strdata->len;

         if ( ( *data == NULL ) || ( *datalen <= (unsigned long) len ) )
            p = IfcAllocateMemory( len + 1 );
         else
            p = *data;
         if ( p == NULL )
            rc = 12; /* RXQUEUE_MEMFAIL */
         else
            {
               /*
                * Using the temporary p inhibits use of *data until it's sure
                * that we return 0.
                */
               *data = p;
               memcpy( *data, strdata->value, len );
               (*data)[len] = '\0';
               *datalen = len;
            }
         Free_stringTSD( strdata );
      }
   }
   Free_stringTSD( queuename );
   return rc;
}
